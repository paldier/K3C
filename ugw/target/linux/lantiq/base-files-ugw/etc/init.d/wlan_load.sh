#!/bin/sh /etc/rc.common
START=15
start() {
	if [ "$CONFIG_FEATURE_ATHEROS_WLAN_TYPE_USB" != "1" ]; then
		if [ "$CONFIG_FEATURE_WIRELESS" = "1" ]; then
			/etc/rc.d/wlan_discover
			if [ "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ] || [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" ]; then
				if [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" = "1" ]; then
					/etc/rc.d/rc.bringup_wlan_5.x init
				else
					/etc/rc.d/rc.bringup_wlan init
				fi
			fi
		fi
	fi
}

stop() {
	if [ "$CONFIG_FEATURE_ATHEROS_WLAN_TYPE_USB" != "1" ]; then
	    if [ "$CONFIG_FEATURE_WIRELESS" = "1" ]; then
			if [ "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ] || [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" ]; then
				if [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" = "1" ]; then
					/etc/rc.d/rc.bringup_wlan_5.x uninit
				else
					/etc/rc.d/rc.bringup_wlan uninit
				fi
			fi
		fi
	fi
}
