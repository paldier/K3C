#!/bin/sh

sys_dev=/sys/class/usbmisc/cdc-wdm*/dev

local mdls="";
local udv="";

lsmod | grep -q wdm || exit 1

ls $sys_dev 2>/dev/null

if [ 0 != $? ] ; then
	sleep 5
fi

for udv in $(ls $sys_dev); do
	rm -f /dev/`echo $udv|cut -d'/' -f5`
	echo mknod /dev/`echo $udv|cut -d'/' -f5` c $(sed 's/:/ /g' $udv)
	mknod /dev/`echo $udv|cut -d'/' -f5` c $(sed 's/:/ /g' $udv)
done; sync
