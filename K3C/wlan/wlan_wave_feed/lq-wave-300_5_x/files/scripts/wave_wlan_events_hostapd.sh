#!/bin/sh

script_name="wave_wlan_events_hostapd.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_WPS_SOURCED" ] && . /tmp/wave_wlan_lib_wps.sh
# . /tmp/wave_wlan_lib_gpio.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source
[ -e $LED_VARS_FILE ] && . $LED_VARS_FILE

# Define local parameters
local ap_index interface_name
local name param3 param4

interface_name=$1
name=$2
param3=$3
param4=$4

# Get corresponding ap index for current interface name
ap_index=`find_index_from_wave_if $interface_name`

print2log $ap_index DEBUG "$script_name $*"

conf_via_external()
{
	# Define local parameters
	local ap_index

	print2log $ap_index DEBUG "$script_name: conf_via_external start"
	write_parameters_from_hostapd_to_ugw $ap_index
	# rc.conf needs to be sourced again after the changes.
	RC_CONF_SOURCED=""
	# Changes effect main,sec and wps parameters.
	(. $ETC_PATH/wave_wlan_config_execute.sh main_modify $ap_index sec_modify $ap_index wps_modify $ap_index)
	print2log $ap_index DEBUG "$script_name: conf_via_external done"
}

wps_pin_needed()
{
	# Define local parameters
	local ap_index interface_name sta_uuid sta_mac enrollee_mac

	# Read the current MAC and PIN in current existing session (if such exists).
	. $WPS_PIN_TEMP_FILE

	ap_index=$1
	interface_name=$2
	sta_uuid=$3
	sta_mac=$4

	# Change MAC to uppercase
	sta_mac=`echo $sta_mac | tr '[:lower:]' '[:upper:]'`

	# If no MAC was found in the WPS_PIN_TEMP_FILE, no PIN connection is expected.
	[ -z "$enrollee_mac" ] && print2log $ap_index DEBUG "$script_name: not expecting WPS-PIN session, no allowed MACs found" && exit

	# Compare between MAC in requesting sta and MAC listed as allowed in rc.conf (converted to uppercase).
	enrollee_mac=`echo $enrollee_mac | tr '[:lower:]' '[:upper:]'`

	# If requesting STA's MAC is allowed, perform pin connection
	if [ "$enrollee_mac" = "$sta_mac" ]
	then
		wps_connect_via_pin $ap_index $interface_name $enrollee_pin $sta_mac $sta_uuid
	else
		print2log $ap_index DEBUG "$script_name: STA $sta_mac tried PIN connection but wasn't allowed"
	fi
}

wps_in_progress()
{
	# multicolor WPS LED: fast blink amber
	if [ -n "$led_wps_count" -a $led_wps_count -eq 1 -a -n "$wps_green" -a -n "$wps_amber" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_amber 0 400 $wps_amber,200 0,100 )
	# unicolor WPS LED: blink green
	elif [ -n "$led_wps_count" -a -n "$wps_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_blink $led_wps_green 500 500 )
	# no WPS LED, but multicolor WLAN0 LED: fast blink amber
	elif [ -n "$led_wlan0_count" -a $led_wlan0_count -eq 1 -a -n "$wlan0_green" -a -n "$wlan0_amber" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_amber 0 400 $wlan0_amber,200 0,100 )
	# no WPS LED, but unicolor WLAN0 LED: blink green
	elif [ -n "$led_wlan0_count" -a -n "$wlan0_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_blink $led_wlan0_green 500 500 )
	fi
}

wps_auth_started()
{
	# multicolor WPS LED: slow blink green and amber
	if [ -n "$led_wps_count" -a $led_wps_count -eq 1 -a -n "$wps_green" -a -n "$wps_amber" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green -1 $wps_green,1000 $wps_amber,1000 )
	# unicolor WPS LED: slow blink green
	elif [ -n "$led_wps_count" -a -n "$wps_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_blink $led_wps_green 1000 1000 )
	# no WPS LED, but multicolor WLAN0 LED: slow blink green and amber
	elif [ -n "$led_wlan0_count" -a $led_wlan0_count -eq 1 -a -n "$wlan0_green" -a -n "$wlan0_amber" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green -1 $wlan0_green,1000 $wlan0_amber,1000 )
	# no WPS LED, but unicolor WLAN0 LED: slow blink green
	elif [ -n "$led_wlan0_count" -a -n "$wlan0_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_blink $led_wlan0_green 1000 1000 )
	fi
}

wps_error()
{
	# multicolor WPS LED: fast blink red for 2 minutes, then stay green
	if [ -n "$led_wps_count" -a $led_wps_count -eq 1 -a -n "$wps_green" -a -n "$wps_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 600 $wps_red,100 0,100 )
	# unicolor WPS LEDs: fast blink red for 2 minutes, then stay green
	elif [ -n "$wps_green" -a -n "$wps_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 1 0,120000 )
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_blink $led_wps_red 100 100 120 )
	# unicolor WPS LED (green only): fast blink green for 2 minutes, then stay green
	elif [ -n "$wps_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 600 $wps_green,100 0,100 )
	# no WPS LED, but multicolor WLAN0 LED: fast blink red for 2 minutes, then stay green
	elif [ -n "$led_wlan0_count" -a $led_wlan0_count -eq 1 -a -n "$wlan0_green" -a -n "$wlan0_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 600 $wlan0_red,100 0,100 )
	# no WPS LED, but unicolor WLAN0 LEDs: fast blink red for 2 minutes, then stay green
	elif [ -n "$wlan0_green" -a -n "$wlan0_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 1 0,120000 )
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_blink $led_wlan0_red 100 100 120 )
	# no WPS LED, but unicolor WLAN0 LED (green only): fast blink green for 2 minutes, then stay green
	elif [ -n "$wlan0_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 600 $wlan0_green,100 0,100 )
	fi
}

wps_session_overlap()
{
	# multicolor WPS LED: blink red for 2 minutes, then stay green
	if [ -n "$led_wps_count" -a $led_wps_count -eq 1 -a -n "$wps_green" -a -n "$wps_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 80 $wps_red,100 0,100 $wps_red,100 0,100 $wps_red,100 0,100 $wps_red,100 0,100 $wps_red,100 0,100 0,500 )
	# unicolor WPS LEDs:  blink red for 2 minutes, then stay green
	elif [ -n "$wps_green" -a -n "$wps_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 1 0,120000 )
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_red 0 80 $wps_red,100 0,100 $wps_red,100 0,100 $wps_red,100 0,100 $wps_red,100 0,100 $wps_red,100 0,100 0,500 )
	# unicolor WPS LED (green only): fast blink green for 2 minutes, then stay green
	elif [ -n "$wps_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 600 $wps_green,100 0,100 )
	# no WPS LED, but multicolor WLAN0 LED: fast blink red for 2 minutes, then stay green
	elif [ -n "$led_wlan0_count" -a $led_wlan0_count -eq 1 -a -n "$wlan0_green" -a -n "$wlan0_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 80 $wlan0_red,100 0,100 $wlan0_red,100 0,100 $wlan0_red,100 0,100 $wlan0_red,100 0,100 $wlan0_red,100 0,100 0,500 )
	# no WPS LED, but unicolor WLAN0 LEDs: fast blink red for 2 minutes, then stay green
	elif [ -n "$wlan0_green" -a -n "$wlan0_red" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 1 0,120000 )
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_red 0 80 $wlan0_red,100 0,100 $wlan0_red,100 0,100 $wlan0_red,100 0,100 $wlan0_red,100 0,100 $wlan0_red,100 0,100 0,500 )
	# no WPS LED, but unicolor WLAN0 LED (green only): fast blink green for 2 minutes, then stay green
	elif [ -n "$wlan0_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 600 $wlan0_green,100 0,100 )
	fi
}

wps_success()
{
	# WPS LED: slow blink green 5 times, then stay green
	if [ -n "$led_wps_count" -a -n "$wps_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 5 $wps_green,1000 0,1000 )
	# no WPS LED, but WLAN0 LED: slow blink green 5 times, then stay green
	elif [ -n "$led_wlan0_count" -a -n "$wlan0_green" ]; then
		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wlan0_green $wlan0_green 5 $wlan0_green,1000 0,1000 )
	fi
}

timestamp=`date -Iseconds`
# Write to file only when CLI WPS session started.
# CLI WPS command create CLI_WPS_IN_PROCESS file when WPS session start, delete it when finish .
# It also mv wave_wlan_hostapd_events.log to wave_wlan_hostapd_events_prev.log for debug history.
if [ -e $CLI_WPS_IN_PROCESS ]
then
	echo "[$timestamp] $*" >> /tmp/wave_wlan_hostapd_events.log
fi

if [ -e /tmp/WPS_STATE ]; then
	source /tmp/WPS_STATE
else
	WPS_STATE=WPS_OFF
fi

# WPS state machine
case $name in
	"WPS-NEW-AP-SETTINGS")
		conf_via_external $ap_index
	;;
	"WPS-PIN-NEEDED")
		wps_pin_needed $ap_index $interface_name $param3 $param4
	;;
	#"WLAN-MODE-INIT")
		# WLAN LED: flashing green for WLAN network activity
	#	if [ "$interface_name" = "wlan0" -a -n "$led_wlan0_green" ]; then
	#		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_netif_activity_trigger $led_wlan0_green $interface_name )
	#	elif [ "$interface_name" = "wlan1" -a -n "$led_wlan1_green" ]; then
	#		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_netif_activity_trigger $led_wlan1_green $interface_name )
	#	fi
	#	# Initialize WPS and APMODE LEDs (only for WLAN-MODE-INIT of wlan0)
	#	if [ "$interface_name" = "wlan0" ]; then
	#		# check if WPS is enabled
	#		WPS_ON=`get_wps_on $ap_index $interface_name`
	#		if [ "$WPS_ON" = "$YES" ]; then
	#			WPS_STATE="WPS_IDLE"
 	#		else
	#			WPS_STATE="WPS_OFF"
 	#		fi
	#		if [ -n "$led_wps_green" ]; then
	#			if [ "$WPS_STATE" != "WPS_OFF" ]; then
	#				( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_sequence $led_wps_green $wps_green 0 )
	#			else
	#				( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_off $led_wps_green )
	#			fi
	#		fi
	#		if [ -n "$led_apmode_green" ]; then
	#			if [ "$param3" = "sta" ]; then
	#				( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_off $led_apmode_green )
	#			else
	#				( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_on $led_apmode_green )
	#			fi
	#		fi
	#	fi
	#;;
	#"WPS-SESSION-START")
	#	if [ "$WPS_STATE" != "WPS_OFF" ]; then
	#		WPS_STATE="WPS_STARTED"
	#		wps_in_progress
	#	fi
	#;;
	#"CTRL-EVENT-EAP-STARTED")
	#	if [ "$WPS_STATE" = "WPS_STARTED" ]; then
	#		WPS_STATE=$param3
	#		wps_auth_started
	#	fi
	#;;
	#"WPS-REG-SUCCESS")
	#	WPS_STATE=$param3
	#	wps_success
	#;;
	#"AP-STA-CONNECTED" | "CONNECTED")
	#	if [ "$WPS_STATE" = "$param3" ]; then
	#		WPS_STATE="WPS_IDLE"
	#	fi
	#	# WLAN0 LED: flashing green for WLAN0 network activity
	#	if [ -n "$led_wlan0_count" -a -n "$wlan0_green" ]; then
	#		( . $ETC_PATH/wave_wlan_gpio_ctrl.sh led_netif_activity_trigger $led_wlan0_green wlan0 )
	#	fi
	#;;
	#"WPS-TIMEOUT")
	#	WPS_STATE="WPS_IDLE"
	#	wps_error
	#;;
	#"AP-STA-DISCONNECTED")
	#	if [ "$WPS_STATE" = "$param3" ]; then
	#		WPS_STATE="WPS_IDLE"
	#		wps_error
	#	fi
	#;;
	#"WPS-OVERLAP-DETECTED")
	#	WPS_STATE="WPS_IDLE"
	#	wps_session_overlap
	#;;
	*)
		echo "wave_wlan_hostapd_events: $name"
	;;
esac
#echo "WPS_STATE=$WPS_STATE" > /tmp/WPS_STATE
