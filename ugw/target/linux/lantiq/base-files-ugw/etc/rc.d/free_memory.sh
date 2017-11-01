#!/bin/sh
# Free up some memory by stopping services, killing daemons and remove modules

if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
fi

_dbg () { echo $@; $@; };

stop_daemons ()
{
	#Format: <binary name>[:number of sleeps after stop]
	local stop_bins="\
		oamd dnrd:2 check_dsl syslogd klogd \
		snmpd udhcpc udhcpd:2 udhcpr atmarpd \
		ripd cli_be cli_fe ifxsip pppoe-relay \
		minidlna smbd mountd mcastd ftpd minissdpd inetd swreset br2684ctld \
	"
	local _daemons;
	for _daemons in $stop_bins; do
		eval $(echo $_daemons|sed 's/:/ /g' \
		|awk '{ print "_dbg killall "$1"; sleep "$2" 2>&-" }')
	done
}

stop_coc ()
{
        /opt/lantiq/bin/pm_util -yoff
}

stop_stunnel ()
{
	_dbg killall stunnel
	if [ -f /var/run/ssl.pid ] ; then
		pid=`cat /var/run/ssl.pid`
		_dbg kill $pid
		echo "stunnel process id--$pid"
	fi
	_dbg killall -9 stunnel
}

stop_tr69 ()
{
	_dbg killall devm
	_dbg killall devmapp
}

stop_web ()
{
	_dbg killall dwatch
	_dbg killall mini_httpd
}

stop_tftp ()
{
	_dbg killall tftpd
}

stop_wlan ()
{
	_dbg /etc/rc.d/rc.bringup_wlan stop
	_dbg /etc/rc.d/rc.bringup_wlan uninit

	grep rt3062ap /proc/modules >&- 2>&-
	if [ $? -eq 0 ]; then
		_dbg ifconfig ra0 down
		_dbg rmmod rt3062ap
	fi
}

stop_usb ()
{
	grep ifxusb_host /proc/modules >&- 2>&-
	if [ $? -eq 0 ]; then
		for drs in `ls -d /mnt/usb/*/*/ 2>&-`; do _dbg umount $drs; done
		_dbg umount /ramdisk/usb/.run/mountd 2>&-
		grep -q fuse /proc/modules 2>&-
		[ $? -eq 0 ] && _dbg rmmod fuse
		grep -q tntfs /proc/modules 2>&-
		[ $? -eq 0 ] && _dbg rmmod tntfs
		echo 0 > /proc/ifxusb_hcd/buspower
		echo 1 > /proc/ifxusb_hcd/suspend_host
		grep -q autofs4 /proc/modules 2>&-
		[ $? -eq 0 ] && _dbg rmmod autofs4
		_dbg rmmod ifxusb_host;
	fi

	grep ifxusb_gadget /proc/modules >&- 2>&-
	if [ $? -eq 0 ]; then
		_dbg brctl delif br0 usb0
		_dbg ifconfig usb0 down; sleep 1;
		_dbg rmmod g_ether.ko; sleep 1;
		_dbg rmmod ifxusb_gadget.ko;
	fi
}

stop_voip ()
{
	killall -9 el_handler
	grep drv_ifxos /proc/modules >&- 2>&-
	if [ $? -eq 0 ]; then
		_dbg killall ifxsip
		_dbg rmmod danube_paging
		_dbg rmmod cosic
		_dbg rmmod drv_timer
		_dbg rmmod ifx_voip_timer_driver
		_dbg rmmod drv_kpi2udp
		_dbg rmmod drv_vmmc
		_dbg rmmod drv_tapi; sleep 1
		_dbg rmmod drv_ifxos
		# _dbg rmmod drv_ter1x66; sleep 1
	fi
}

stop_dsl ()
{
	_dbg killall pppd
	([ -z "$CONFIG_UBOOT_CONFIG_BOOT_FROM_NAND" ] || [ -n "$CONFIG_TARGET_LTQCPE_PLATFORM_AR9" ]) && {
		grep drv_dsl_cpe_api /proc/modules >&- 2>&-
		if [ $? -eq 0 ]; then
			/etc/init.d/ltq_cpe_control_init.sh stop
			_dbg killall -9 oamd
			sleep 2

			grep CONFIG_PACKAGE_KMOD_LTQCPE_PPE_E5_OFFCHIP_BONDING /etc/config.sh &>/dev/null
			if [ $? -ne 0 ]; then
				_dbg rmmod drv_dsl_cpe_api
				rm -rf /tmp/pipe/*
				grep drv_ifxos /proc/modules >&- 2>&-
				[ $? -eq 0 ] && _dbg rmmod drv_ifxos
			fi
		fi
	}
}

stop_multicast ()
{
	grep iproxyd /proc/modules >&- 2>&-
	[ $? -eq 0 ] && _dbg rmmod iproxyd
}

stop_qos ()
{
	_dbg killall qos_rate_update
	grep nf_conntrack_proto_esp /proc/modules >&- 2>&-
	[ $? -eq 0 ] && _dbg rmmod nf_conntrack_proto_esp
}

stop_ppa ()
{
if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
	_dbg /etc/init.d/unload_ppa_modules.sh
fi
}

stop_misc ()
{
	grep pecostat_lkm /proc/modules >&- 2>&-
	[ $? -eq 0 ] && _dbg rmmod pecostat_lkm
	grep pecostat /proc/modules >&- 2>&-
	[ $? -eq 0 ] && _dbg rmmod pecostat
}

free_ramdisk ()
{
	_dbg rm -rf /ramdisk/var /ramdisk/tftp_upload/* /flash/env.sh /tmp/ubootconfig /tmp/dyn_info /tmp/WS* /flash/print*
	echo 256 > /proc/sys/vm/min_free_kbytes; sleep 1
}

[ -n "$1" ] && {
	case "$1" in
		tr69)
			echo "free_memory.sh: Free-up memory for tr69";
			stop_daemons; stop_coc; stop_web; stop_tftp; stop_wlan; stop_usb; stop_voip;
			stop_multicast; stop_qos; stop_misc; free_ramdisk;;
		http)
			echo "free_memory.sh: Free-up memory for http";
			stop_daemons; stop_coc; stop_tr69; stop_tftp; stop_usb; stop_voip; stop_multicast;
			stop_qos; stop_dsl; stop_misc; free_ramdisk;;
		tftp)
			echo "free_memory.sh: Free-up memory for tftp";
			stop_daemons; stop_coc; stop_web; stop_tr69; stop_stunnel; stop_wlan; stop_usb; stop_voip;
			stop_multicast; stop_qos; stop_dsl; stop_misc; free_ramdisk;;
		tr69_postdl)
			echo "free_memory.sh: Free-up memory for tr69_postdl";
			stop_dsl; free_ramdisk;;
		diag)
			echo "free_memory.sh: Free-up memory for diag";
			stop_daemons;;
	esac || true
} || {
	echo "free_memory.sh: Free-up all memory";
	stop_daemons; stop_coc; stop_web; stop_tr69; stop_stunnel; stop_tftp; stop_wlan; stop_usb; stop_voip;
	stop_multicast; stop_qos; stop_dsl; stop_misc; stop_ppa; stop_stunnel; free_ramdisk;
	_dbg killall -9 stunnel
}

echo 1 > /proc/sys/vm/drop_caches; sleep 1

