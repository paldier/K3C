#!/bin/sh
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


if [ "$qm_enable" = "0" ]; then
	if [ "$wanphy_phymode" = "2" ]; then
		if [ "$CONFIG_IFX_CONFIG_CPU" = "XRX288" ]; then
			echo eth1 prio 0 queue 1 prio 1 queue 1 prio 2 queue 1 prio 3 queue 1 prio 4 queue 1 prio 5 queue 1 prio 6 queue 1 prio 7 queue 0 > /proc/eth/prio
		elif [ "$CONFIG_IFX_CONFIG_CPU" = "AMAZON_S" ]; then
			echo eth1 prio 0 queue 1 prio 1 queue 1 prio 2 queue 1 prio 3 queue 1 prio 4 queue 1 prio 5 queue 1 prio 6 queue 1 prio 7 queue 0 > /proc/eth/prio
		fi
	elif [ "$wanphy_tc" = "1" -a "$wanphy_phymode" = "3" ]; then
		if [ "$CONFIG_IFX_CONFIG_CPU" = "XRX288" ]; then
			echo ptm0 prio 0 queue 1 prio 1 queue 1 prio 2 queue 1 prio 3 queue 1 prio 4 queue 1 prio 5 queue 1 prio 6 queue 1 prio 7 queue 0 > /proc/eth/prio
		elif [ "$CONFIG_IFX_CONFIG_CPU" = "AMAZON_S" ]; then
			echo ptm0 prio 0 queue 1 prio 1 queue 1 prio 2 queue 1 prio 3 queue 1 prio 4 queue 1 prio 5 queue 1 prio 6 queue 1 prio 7 queue 0 > /proc/eth/prio
		fi
	fi
fi

