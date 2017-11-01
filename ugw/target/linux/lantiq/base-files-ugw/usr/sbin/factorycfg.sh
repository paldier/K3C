#!/bin/sh


#tmp will be removed later.
. /etc/config.sh
if [ "$CONFIG_IFX_MODEL_NAME" = "GRX350_GW_HE_VDSL_LTE_NEW_FRAMEWORK" ] ; then 
	ubus call csd factoryreset
fi

/usr/sbin/syscfg_lock /flash/rc.conf '
	/usr/sbin/upgrade /etc/rc.conf.gz sysconfig 0 0
	sync; sleep 2;
	/etc/init.d/rebootsync.sh stop
	reboot -f
'
