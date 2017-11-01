#!/bin/sh
# UGW specific calibration update script.
#

__help()
{
	echo
	[ -n "$1" ] && {
		echo "$1"; echo;
	}
	echo "Please copy or download a calibration file under a writable folder"
	echo "and run this script with proper calibration name."
	echo
	echo "Example for 'wlan' calibration:-"
	echo "    cd /tmp"
	echo "    tftp -g -r cal_wlan0.bin 192.168.1.2"
	echo "    $0 wlan"
	echo
	exit 1;
}

[ -z "$1" ] && __help;

case "$1" in
	"wlan") [ -f ./cal_wlan0.bin ] || __help "Could not find 'cal_wlan0.bin' file!!"
		tar -czf eeprom.tar.gz cal_wlan0.bin || {
			__help "Unable to write in this folder!!"
		}
		vol_mgmt update_calibration wlanconfig eeprom.tar.gz
		sync; rm -f eeprom.tar.gz;
	;;
	*)
		__help "This calibration is not supported!!"
esac

