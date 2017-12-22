#!/bin/sh
# Script to activate of deactivate host mode USB LED's based on disk mount
# This script designed to work only with mountd application

led_f ()
{
	echo $2 > /sys/class/leds/usb$1_led/brightness
}

[ -d /sys/module/usb_storage/drivers/usb\:usb-storage/1-*/ ] && led_f 1 1 || led_f 1 0
[ -d /sys/module/usb_storage/drivers/usb\:usb-storage/2-*/ ] && led_f 2 1 || led_f 2 0

