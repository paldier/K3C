#!/bin/sh
#
# This script is called by dsl_cpe_control whenever there is a DSL event,
# we only actually care about the DSL_INTERFACE_STATUS events as these
# tell us the line has either come up or gone down.
#
# The rest of the code is basically the same at the atm hotplug code
#

[ "$DSL_NOTIFICATION_TYPE" = "DSL_INTERFACE_STATUS" ] || exit 0

. /usr/share/libubox/jshn.sh
. /lib/functions.sh
. /lib/functions/leds.sh

include /lib/network
scan_interfaces

local default
config_load system
config_get default led_adsl default
if [ "$default" != 1 ]; then
	case "$DSL_INTERFACE_STATUS" in
	  "HANDSHAKE")  led_timer dsl 500 500;;
	  "TRAINING")   led_timer dsl 200 200;;
	  "UP")		led_on dsl;;
	  *)		led_off dsl
	esac
fi

local cmp_device
case $DSL_XTU_STATUS in                                  
	VDSL)                                            
		cmp_device=ptm                           
	;;                                               
	ADSL)                                            
		cmp_device=nas                           
	;;                                               
	*)
		echo "Unknown DST_XTU_STATUS $DSL_XTU_STATUS"
		exit 1
	;;
esac

local interfaces=`ubus list network.interface.\* | cut -d"." -f3`
local ifc
for ifc in $interfaces; do

	local up
	json_load "$(ifstatus $ifc)"
	json_get_var up up

	local auto
	config_get_bool auto "$ifc" auto 1

	local proto
	json_get_var proto proto

	local device
	json_get_var device device

	## I will handle only interested interface (ATM/PTM)
	echo $device | grep "${cmp_device}" > /dev/null
	[ $? -ne 0 ] && { continue; }

	if [ "$DSL_INTERFACE_STATUS" = "UP" ]; then
		if [ "$up" != 1 ] && [ "$auto" = 1 ]; then
			[ "${cmp_device}" = "nas" ] && { . /lib/functions/dsl_fapi.sh; atm_init_interface; }
			( sleep 1; ifup "$ifc" ) &
		fi
	elif [ "$DSL_INTERFACE_STATUS" = "DOWN" ]; then
		if [ "$up" = 1 ] && [ "$auto" = 1 ]; then
			( sleep 1; ifdown "$ifc" ) &
		else
			json_get_var autostart autostart
			if [ "$proto" = "pppoa" ] && [ "$up" != 1 ] && [ "$autostart" = 1 ]; then
				( sleep 1; ifdown "$ifc" ) &
			fi
		fi
	else
		echo "DSL_INTERFACE_STATUS $DSL_INTERFACE_STATUS"
	fi
done


