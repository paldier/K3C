#!/bin/sh /etc/rc.common

START=99

echo "Env $ENVLOADED" > /dev/null
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


#values required for minissdpd 
eval TR64ENABLE='$'tr69_misc_tr64enable
eval UPNPENABLE='$'tr69_misc_upnpenable	
eval TR64PORT='$'tr69_misc_tr64port
eval UPNPPORT='$'tr69_misc_upnpport	

start (){
/usr/sbin/minissdpd $TR64ENABLE $UPNPENABLE $TR64PORT $UPNPPORT &
}

#Fix for VRX220 Endurance to pass
echo 300 > /proc/sys/dev/ltq_sflash/fifosize
