#!/bin/sh /etc/rc.common
. /lib/functions/system.sh

START=01

start() {
	rm -rf /etc/init.d/csd /etc/init.d/servd /etc/init.d/ltq_cgroups_startup.sh /etc/init.d/polld 
	rm -rf /etc/init.d/ltq_hwmon.sh	/etc/init.d/usb_host_device.sh /etc/init.d/usb_led.sh /etc/init.d/usb_automount_status.sh 
	rm -rf /etc/init.d/ltq_pmcu.sh /etc/init.d/ltq_temp.sh /etc/init.d/pm_util.sh /etc/init.d/ltq_regulator.sh
	rm -rf /etc/init.d/led_status

	insmod /lib/modules/3.10.102/ltq_pae_hal.ko;
	insmod /lib/modules/3.10.102/ltq_mpe_hal_drv.ko;
	insmod /lib/modules/3.10.102/ltq_tmu_hal_drv.ko;
	insmod /lib/modules/3.10.102/ltq_directpath_datapath.ko;
	insmod /lib/modules/3.10.102/ltq_directconnect_datapath.ko;
	insmod /lib/modules/3.10.102/macvlan.ko;

	insmod /lib/modules/3.10.102/ppa_api.ko;
	insmod /lib/modules/3.10.102/ppa_api_proc.ko;
	insmod /lib/modules/3.10.102/swa_stack_al.ko;
	insmod /lib/modules/3.10.102/ppa_api_sw_accel_mod.ko;
	insmod /lib/modules/3.10.102/ltqmips_dtlk.ko;
	insmod /lib/modules/3.10.102/dlrx_fw.ko;
	insmod /lib/modules/3.10.102/gpio-button-hotplug.ko

	ppacmd init
	lan_mac=`phic_fac -g mac|cut -d'=' -f 2`
	local i=1
	while [ $i -le 4 ]
	do
		j=`expr $i -1`
		ifconfig eth0_$i hw ether $(macaddr_add "$lan_mac" $j)
		i=`expr $i + 1`
	done
	ifconfig eth1 hw ether $lan_mac
	ifconfig eth0_1 up
	ifconfig eth0_2 up
	ifconfig eth0_3 up
	ifconfig eth0_4 up
	ifconfig eth1 up
	brctl addbr br-lan
	ip link set br-lan up
	brctl addif br-lan eth0_1
	brctl addif br-lan eth0_2
	brctl addif br-lan eth0_3
	brctl addif br-lan eth0_4
	brctl addif br-lan eth1
	ifconfig br-lan 192.168.2.1 netmask 255.255.255.0

	ppacmd addlan -i br-lan
	ppacmd addlan -i eth0_1
	ppacmd addlan -i eth0_2
	ppacmd addlan -i eth0_3
	ppacmd addlan -i eth0_4
	ppacmd addwan -i eth1
	/usr/bin/telnetd_startup&
	/etc/fastmode/led4fastmode.sh close
}
