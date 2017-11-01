#!/bin/sh /etc/rc.common

if [ ! "$ENVLOADED" ]; then
	if [ -r /etc/rc.conf ]; then
		. /etc/rc.conf 2> /dev/null
		ENVLOADED="1"
	fi
fi

if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
fi

samba_opt_ena()
{
    # for xrx500 where LRO is accelerating Samba traffic no need of this option
    if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
	if [ ${samba_opt_flag} = 1 ]; then
		# play safe : disable first
		samba_opt_dis

		# it is assumed that dependant kernel configuration
		# is taken care of at compile time
		# for eg. raw table, CT target etc
		KERNEL_VER="`uname -r`"
		IPT_RAW_MOD="iptable_raw"
		SAMBA_PORT=445

		if [ -f /lib/modules/${KERNEL_VER}/${IPT_RAW_MOD}.ko -a -z "`grep ${IPT_RAW_MOD} /proc/modules`" ]; then
			/sbin/insmod /lib/modules/${KERNEL_VER}/${IPT_RAW_MOD}.ko
		fi
		
		if [ -n "`echo ${KERNEL_VER} | grep "^2.6."`" ]; then
			NOTRACK_MOD="xt_NOTRACK"
			NOTRACK_TARGET="-j NOTRACK"
		else
			NOTRACK_MOD="xt_CT"
			NOTRACK_TARGET="-j CT --notrack"
		fi

		if [ -f /lib/modules/${KERNEL_VER}/${NOTRACK_MOD}.ko -a -z "`grep ${NOTRACK_MOD} /proc/modules`" ]; then
			/sbin/insmod /lib/modules/${KERNEL_VER}/${NOTRACK_MOD}.ko
		fi

		/usr/sbin/iptables -t raw -I PREROUTING -p tcp --dport ${SAMBA_PORT} ${NOTRACK_TARGET}
		/usr/sbin/iptables -t raw -I PREROUTING -p tcp --sport ${SAMBA_PORT} ${NOTRACK_TARGET}
		/usr/sbin/iptables -t raw -I OUTPUT -p tcp --sport ${SAMBA_PORT} ${NOTRACK_TARGET}
		/usr/sbin/iptables -t raw -I OUTPUT -p tcp --dport ${SAMBA_PORT} ${NOTRACK_TARGET}
	fi
	# take care to restart samba process if required
    fi
}

samba_opt_dis()
{
    # for xrx500 where LRO is accelerating Samba traffic no need of this option
    if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
	# it is assumed that dependant kernel configuration
	# is taken care of at compile time
	# for eg. raw table, CT target etc
	KERNEL_VER="`uname -r`"
	IPT_RAW_MOD="iptable_raw"
	SAMBA_PORT=445

	if [ -n "`echo ${KERNEL_VER} | grep "^2.6."`" ]; then
		NOTRACK_MOD="xt_NOTRACK"
		NOTRACK_TARGET="-j NOTRACK"
	else
		NOTRACK_MOD="xt_CT"
		NOTRACK_TARGET="-j CT --notrack"
	fi

	/usr/sbin/iptables -t raw -D OUTPUT -p tcp --dport ${SAMBA_PORT} ${NOTRACK_TARGET} 2> /dev/null
	/usr/sbin/iptables -t raw -D OUTPUT -p tcp --sport ${SAMBA_PORT} ${NOTRACK_TARGET} 2> /dev/null
	/usr/sbin/iptables -t raw -D PREROUTING -p tcp --sport ${SAMBA_PORT} ${NOTRACK_TARGET} 2> /dev/null
	/usr/sbin/iptables -t raw -D PREROUTING -p tcp --dport ${SAMBA_PORT} ${NOTRACK_TARGET} 2> /dev/null

	if [ -n "`grep ${NOTRACK_MOD} /proc/modules`" ]; then
		/sbin/rmmod ${NOTRACK_MOD} 2> /dev/null
	fi

	if [ -n "`grep ${IPT_RAW_MOD} /proc/modules`" ]; then
		/sbin/rmmod ${IPT_RAW_MOD} 2> /dev/null
	fi
    fi
}
