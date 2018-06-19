#!/bin/sh

local unload_modules="\
	cdc_mbim \
	cdc-acm \
	cdc_ncm \
	cdc-wdm \
	cdc_ether \
	usbnet_lte \
"

local mdls="";

ifconfig wwan0 down
ifconfig wwan1 down
ifconfig wwan2 down

sleep 1

#for mdls in $unload_modules; do
#	rmmod $mdls
#done

rm -f /dev/ttyACM0
rm -f /dev/ttyACM1
rm -f /dev/ttyACM2
