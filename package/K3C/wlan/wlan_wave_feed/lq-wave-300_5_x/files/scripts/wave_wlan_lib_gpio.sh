#!/bin/sh

# Framework for Wave LED/PBC controller

# Turn on a LED
# Args: duration (if blank stay on indefinitely)
led_on () {
	LED_NAME=$1
	DURATION=$2
	echo default-on > $LED_NAME/trigger

	# Control duration by stating a monitor script
	if [ ! -z "$DURATION" ]
	then
		LED_OFF_SCRIPT=/tmp/led_mon_$$.sh
		echo "sleep $DURATION; echo none > $LED_NAME/trigger; rm $LED_OFF_SCRIPT" > $LED_OFF_SCRIPT
		chmod +x $LED_OFF_SCRIPT
		/bin/sh $LED_OFF_SCRIPT &
	fi
}

# Turn off a LED
led_off () {
	LED_NAME=$1
	echo none > $LED_NAME/trigger
}

# Blink a LED for a specified period interval
# Args: Interval on, interval off, and duration (if blank blink indefinitely)
led_blink () {
	LED_NAME=$1
	INTERVAL_ON=$2
	INTERVAL_OFF=$3
	DURATION=$4

	echo timer > $LED_NAME/trigger
	echo $INTERVAL_ON > $LED_NAME/delay_on
	echo $INTERVAL_OFF > $LED_NAME/delay_off

	# Control duration by stating a monitor script
	if [ ! -z "$DURATION" ]
	then
		LED_OFF_SCRIPT=/tmp/led_mon_$$.sh
		echo "i=0; while [ \$i -lt $DURATION ]; do sleep 1; let i=i+1; done; echo none > $LED_NAME/trigger; rm $LED_OFF_SCRIPT" > $LED_OFF_SCRIPT
		chmod +x $LED_OFF_SCRIPT
		/bin/sh $LED_OFF_SCRIPT &
	fi
}

# run a LED sequence for a specified period
# Args: <led_name> <final_color> <repetitions> <color>,<duration> [<color>,<duration> [...]]
#       run infinite if <repetitions> is -1
led_sequence () {
	LED_NAME=$1
	shift
	FINAL_COLOR=$1
	shift
	REPEAT=$1
	shift
	LED_SEQUENCE=$*

	echo sequence > $LED_NAME/trigger
	echo $FINAL_COLOR > $LED_NAME/final_brightness
	echo $REPEAT > $LED_NAME/repeat
	echo $LED_SEQUENCE > $LED_NAME/sequence
}

# Connect a LED to a wlan interface rx/tx activity
led_netif_activity_trigger () {
	LED_NAME=$1
	WLAN_IF=$2
	echo netdev > $LED_NAME/trigger
	echo $WLAN_IF  > $LED_NAME/device_name
	echo "link rx tx" > $LED_NAME/mode
	echo 120 > $LED_NAME/interval
}

# Initialize a Pushbutton GPIO
pbc_init () {
	PBC_NAME=$1
	PBC_TRIGGER=$2

	# Insmod the wps_pbc driver for polling gpios
	if [ -e /usr/drivers/wps_pb.ko ] && [ `lsmod | grep wps_pb -c` -eq 0 ]
	then
		insmod /usr/drivers/wps_pb.ko
	fi
	echo $PBC_TRIGGER > $PBC_NAME
}

# Wait for pushbutton event
pbc_wait () {
	PBC_NAME=$1
	read < $PBC_NAME
}

usage () {
	echo USAGE:
	echo "wave_wlan_led_ctrl.sh <command> <led_name/pbc_name> <options>"
	echo Commands:
	echo -e "    led_on\n    led_off\n    led_blink\n    led_netif_activity_trigger\n    pbc_init\n    pbc_wait"
	echo Options:
	echo "    led_on:                     [<duration (secs)>]"
	echo "    led_off/pbc_wait:           none"
	echo "    led_blink:                  <interval_on (ms)>   <interval_off (ms)>  [<duration (secs)>]"
	echo "    led_netif_activity_trigger: <netif_name   e.g. wlan0>"
	echo "    pbc_init:                   <trigger   e.g. 1 for trigger on PBC release>"
	echo
}

# Define local parameters 
local command

command=$1
shift
case $command in
	led_on)
		led_on $@
	;;
	led_off)
		led_off $@
	;;
	led_blink)
		led_blink $@
	;;
	led_sequence)
		led_sequence $@
	;;
	led_netif_activity_trigger)
		led_netif_activity_trigger $@
	;;
	pbc_init)
		pbc_init $@
	;;
	pbc_wait)
		pbc_wait $@
	;;
	*)
		usage
	;;
esac
