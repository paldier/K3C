#!/bin/sh 

if [ ! "$ENVLOADED" ]; then
	if [ -r /etc/rc.conf ]; then
		. /etc/rc.conf 2> /dev/null
		ENVLOADED="1"
	fi
fi

if [ -r /etc/rc.d/config.sh ]; then
	. /etc/rc.d/config.sh 2>/dev/null
fi



if [ "1$CONFIG_FEATURE_PPA_SUPPORT" = "11" ]; then
	/sbin/ppacmd $1 -i $2 2> /dev/null
fi
