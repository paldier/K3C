#!/bin/sh /etc/rc.common
START=45
start() {
	if [ "$CONFIG_FEATURE_ATHEROS_WLAN_TYPE_USB" != "1" ]; then
		if [ "$CONFIG_FEATURE_WIRELESS" = "1" ]; then
			if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ] && [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" != "1" ]; then
				if [ ! "$MTLK_INIT_PLATFORM" ]; then
					. /etc/rc.d/mtlk_init_platform.sh
				fi
			fi
			if [ "$CONFIG_TARGET_LANTIQ_XRX330" != "1" ] && [ "$CONFIG_TARGET_LANTIQ_XRX300" != "1" ]; then
				if [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" = "1" ]; then
					/etc/rc.d/rc.bringup_wlan_5.x init
				else
					/etc/rc.d/rc.bringup_wlan init
				fi
			else
				echo "Bring up QCA init"
				/etc/rc.d/rc.wlan up
				sleep 3 
			fi
			if [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" = "1" ]; then
				/etc/rc.d/rc.bringup_wlan_5.x start
			else
				/etc/rc.d/rc.bringup_wlan start
			fi
			if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
				if [ ! -e $wave_init_failure ] && [ ! -e $wave_start_failure ]; then
					echo "S45: wlan bringup success"  >> $wave_init_success
				fi
				if [ -e $wave_start_failure ] && [ `cat $wave_start_failure | grep -v "non-existing physical wlan1" | grep -v "wlan1 failed:" -c` -eq 0 ]; then
					echo "S45: wlan bringup success"  >> $wave_init_success
					echo "S45: Platform is dual concurrent, but wlan1 isn't connected" >> $wave_init_success
				fi

			fi
		fi
	fi
	if  [ "$CONFIG_FEATURE_RIP" = "1" ]; then
		if ps ax | grep -v grep | grep ripd  > /dev/null
		then
			echo "ripd service running"                                 
		else
			if [ "$route_dynamic_fEnable" = "1" ]; then
				if [ -r /etc/ripd.conf ]; then
					echo "hostname $hostname" > /etc/zebra.conf
					. /etc/rc.d/init.d/ripd start
				fi
			fi
		fi
	fi

}

stop() {
	if [ "$CONFIG_FEATURE_ATHEROS_WLAN_TYPE_USB" != "1" ]; then
		if [ "$CONFIG_FEATURE_WIRELESS" = "1" ]; then
			if [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" = "1" ]; then
				. /etc/rc.d/rc.bringup_wlan_5.x stop
			else
				. /etc/rc.d/rc.bringup_wlan stop
			fi
			if [ "$CONFIG_TARGET_LANTIQ_XRX330" != "1" ] && [ "$CONFIG_TARGET_LANTIQ_XRX300" != "1" ]; then
				if [ "$CONFIG_FEATURE_WIRELESS_WAVE_5_X" = "1" ]; then
					. /etc/rc.d/rc.bringup_wlan_5.x uninit
				else
					. /etc/rc.d/rc.bringup_wlan uninit
				fi
			fi
		fi
	fi
}
