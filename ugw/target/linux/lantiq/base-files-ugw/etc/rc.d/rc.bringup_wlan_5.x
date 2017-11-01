#!/bin/sh

#while true; do
#	bridge_status=`/usr/sbin/status_oper GET global_bridge_config bridge_status`
#	#echo "bridge_status=$bridge_status"
#	if [ "$bridge_status" != "UNCONFIGURED" ]; then   # i.e. empty or "CONFIGURED"
#		break
#	fi
#done

SCRIPT_DIR="/etc/rc.d"

# Safe source rc.conf by locking the file before sourcing.
if [ ! "$ENVLOADED" ]; then
	if [ -r /etc/rc.conf ]; then
		/usr/sbin/syscfg_lock /etc/rc.conf "
		cp /etc/rc.conf /tmp/rc.conf_$$
		"
		. /tmp/rc.conf_$$ 2> /dev/null
		rm /tmp/rc.conf_$$
		ENVLOADED="1"
	fi
fi

if [ ! "$CONFIGLOADED" ]; then
	if [ -r $SCRIPT_DIR/config.sh ]; then
		. $SCRIPT_DIR/config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
fi

ATH_MAP_FILE="/tmp/system.conf"
WLAN_VENDOR_NAME=""
WLAN_VENDOR_SCRIPT_PREFIX=""
ATH_IF=""

find_vendor_from_index() {
	eval radioCpeId='$'wlmn_$1'_radioCpeId'
	if [ "$radioCpeId" = "1" ]; then
		radioPrefix=0
	elif [ "$radioCpeId" = "2" ]; then
		radioPrefix=1
	fi
	eval WLAN_VENDOR_NAME='$'wlss_$radioPrefix'_prefixScript'
	eval WLAN_VENDOR_SCRIPT_PREFIX=$WLAN_VENDOR_NAME'_wlan'

	if [  "$wlan_cmd" = "start" -o "$wlan_cmd" = "stop" ]; then
		if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
			eval WLAN_VENDOR_SCRIPT_PREFIX=$WLAN_VENDOR_NAME'_wlan_ap'
		fi
	fi
	echo "prefix: $WLAN_VENDOR_SCRIPT_PREFIX"
}

if [ -z "$CONFIG_FEATURE_WIRELESS" ]; then
	echo "no wireless support"
	exit
fi

echo "rc.bringup_wlan_5_x $@" > /dev/console
wlan_cmd=$1
shift
nArgs=$#
if [ $nArgs -gt 0 ]; then
	vap_index=$1
	shift
	find_vendor_from_index $vap_index
	script=$SCRIPT_DIR/$WLAN_VENDOR_SCRIPT_PREFIX'_'$wlan_cmd
#	echo ". $script $vap_index $@"
fi
case "$wlan_cmd" in
	init)
		eval ap_count='$'wlan_phy_Count
		i=0
		wlan_vendor_temp_prefix="NOTHING"
		while [ $i -lt $ap_count ]
		do
			eval wlan_vendor_prefix='$'wlss_$i'_prefixScript'
			if [ "$wlan_vendor_prefix" = $wlan_vendor_temp_prefix ]; then
				echo "$wlan_vendor_prefix module is already Initialized "
			else
				if [ "$wlan_vendor_prefix" != "UNKNOWN" ]; then
					echo "Initializing $wlan_vendor_prefix "
					. $SCRIPT_DIR/$wlan_vendor_prefix'_wlan_init' 
				fi
			fi
			eval wlan_vendor_temp_prefix='$'wlan_vendor_prefix
			i=`expr $i + 1`
		done
		;;
	uninit)
		if [ $nArgs = 0 ]; then
			eval ap_count='$'wlan_phy_Count
			wave_uninit=0
			i=0
			while [ $i -lt $ap_count ]
			do
				find_vendor_from_index $i
				if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
					if [ $wave_uninit -eq 0 ]; then
						echo "Unintializing $WLAN_VENDOR_NAME "
						. $SCRIPT_DIR/wave_wlan_uninit
						wave_uninit=1
					fi
				else
					echo "uninit not supported by $WLAN_VENDOR_NAME"
				fi
				i=`expr $i + 1`
			done
		else
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				echo "Unintializing $WLAN_VENDOR_NAME "
				. $SCRIPT_DIR/wave_wlan_uninit
			else
				echo "uninit not supported by $WLAN_VENDOR_NAME"
			fi
		fi
		;;
	capability)
		if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	start)
		if [ $nArgs = 0 ]; then
			# No Index is passed. That means we have to start all the AP/VAP
			# for that WLAN module. Iterate through all the wlan_main instances
			# to find out how many of AP/VAPs are created for that WLAN module.
			eval ap_count='$'wlan_main_Count
			j=0
			while [ $j -lt $ap_count ]
			do
				find_vendor_from_index $j
				script=$SCRIPT_DIR/$WLAN_VENDOR_SCRIPT_PREFIX'_'$wlan_cmd
				# All VAPs of WAVE can be started by calling the script without parameter
				# If one AP/VAP is found for WAVE, then start all and break the loop
				if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
					echo ". $script"
					. $script
					break
				fi
				j=`expr $j + 1`
			done

			ath_index=0
			j=0
			while [ $j -lt $ap_count ]
			do
				find_vendor_from_index $j
				if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
					eval CPEID='$'wlmn_$j'_cpeId'
					eval ATHIF=ath${ath_index}
					echo "Starting QCA interface $ATH_IF"
					/usr/sbin/status_oper -a SET "ATH_MAP" "$CPEID" "$ATHIF"
					. $SCRIPT_DIR/ath_wlan_ap_start $j
					ath_index=`expr $ath_index + 1`
				fi
				j=`expr $j + 1`
			done
		else
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		fi
		;;
	stop)
		if [ $nArgs = 0 ]; then
			# No Index is passed. That means we have to stop all the AP/VAP
			# for that WLAN module. Iterate through all the wlan_main instances
			# to find out how many of AP/VAPs are created for that WLAN module.
			eval ap_count='$'wlan_main_Count
			j=`expr $ap_count - 1`
			while [ $j -ge 0 ]
			do
				echo "stop: j = $j, ap_num = $ap_count"
				find_vendor_from_index $j
				script=$SCRIPT_DIR/$WLAN_VENDOR_SCRIPT_PREFIX'_'$wlan_cmd
				if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
					echo ". $script"
					. $script
					break
				fi
				j=`expr $j - 1`
			done
			# For Atheros the stopping interfaces will be AP and then all VAP's
			# which is the way opposite than the WAVE uses. So it is stopped 
			# and modules are removed seperately
			eval ap_count='$'wlan_phy_Count
			i=0
			while [ $i -lt $ap_count ]
			do
				find_vendor_from_index $i
				if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
					echo "Stopping and uninitializing Atheros"
					. $SCRIPT_DIR/ath_wlan_uninit
				fi
				i=`expr $i + 1`
			done
		else
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				echo "Stopping $WLAN_VENDOR_NAME $vap_index"
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		fi
		;;
	main_modify)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	vap_enable)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan vap_enable vap_index"
		fi
		;;
	vap_disable)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan vap_disable vap_index"
		fi
		;;
	radio_modify)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	vap_add)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			elif [ "$WLAN_VENDOR_NAME" = "ath" ]; then
				echo "Ath add vap"
				eval CPEID='$'wlmn_${1}'_cpeId'
				eval ath_index=$CPEID
				eval ATHIF=ath${ath_index}
				echo " $vap_index $ath_index $CPEID $ATHIF "
				/usr/sbin/status_oper -a SET "ATH_MAP" "$CPEID" "$ATHIF"
				. $SCRIPT_DIR/ath_wlan_ap_start $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan vap_add vap_index"
		fi
		;;
	vap_remove)
		if [ $nArgs = 0 ]; then
			eval ap_count='$'wlan_main_Count
			j=`expr $ap_count - 1`
			while [ $j -ge 0 ]
			do
				find_vendor_from_index $j
				script=$SCRIPT_DIR/$WLAN_VENDOR_SCRIPT_PREFIX'_'$wlan_cmd
				if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
					echo "Stoping Wave interface $j, ap_num = $ap_count"
					. $script $j
				fi
				j=`expr $j - 1`
			done
		elif [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
				echo "QCA remove vap"
				. $SCRIPT_DIR/ath_wlan_remove_vap $vap_index
			elif [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan vap_remove vap_index"
		fi
		;;
	sec_modify)
		if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
			if [ $nArgs -eq 1 ]; then
				. $script $vap_index
			else
				echo "Usage: $SCRIPT_DIR/rc.bringup_wlan sec_modify vap_index"
			fi
		elif [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	wmm_modify)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan wmm_modify vap_index"
		fi
		;;
	wps_modify)
		if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
			if [ $nArgs -eq 1 ]; then
				echo "QCA wps conf"
				. $SCRIPT_DIR/ath_wlan_wps_config $vap_index
			else
				echo "Usage: $SCRIPT_DIR/rc.bringup_wlan wps_modify vap_index"
			fi
		elif [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	wps_trigger_pbc)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
				echo "QCA wps trigger pbc pairing"
				. $SCRIPT_DIR/ath_wlan_wps_trigger_pbc_pairing $@
			elif [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan wps_trigger_pbc vap_index"
		fi
		;;
	wps_trigger_pin)
		if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
			if [ $nArgs -eq 2 ]; then
				echo "QCA wps trigger pin pairing index $vap_index pin $1"
				. $SCRIPT_DIR/ath_wlan_wps_trigger_pin_pairing $vap_index $1
			else
				echo "Usage: $SCRIPT_DIR/rc.bringup_wlan wps_trigger_pin vap_index pin"
			fi
		elif [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	wps_get_pin)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan wps_get_pin vap_index"
		fi
		;;
	wps_get_profile)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
				echo "QCA get wps profile"
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan wps_get_profile vap_index"
		fi
		;;
	wps_reset_pin)
		if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
			if [ $nArgs -eq 1 ]; then
				. $script $vap_index
			else
				echo "Usage: $SCRIPT_DIR/ath_wlan_wps_reset_pin vap_index"
			fi
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	wps_reset)
		if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
			echo "QCA reset wps"
			if [ $nArgs -eq 1 -o $nArgs -eq 3 ]; then
				. $script $vap_index $@
			else
				echo "Usage: $SCRIPT_DIR/ath_wlan_wps_reset vap_index [new_ssid new_passphrase] "
			fi
		elif [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	wps_generate_pin)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	mac_ctrl_modify)
		if [ "$WLAN_VENDOR_NAME" = "wave" -o "$WLAN_VENDOR_NAME" = "ath" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	wds_modify)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	hs20_modify)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	hs20_l2fw_modify)
		if [ $nArgs -eq 3 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index $@
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage:$SCRIPT_DIR/rc.bringup_wlan hs20_l2fw_modify vap_index cpeid action"
		fi
		;;
	send_qos_map_conf)
		if [ $nArgs -eq 3 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index $@
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage:$SCRIPT_DIR/rc.bringup_wlan send_qos_map_conf vap_index qos_map_frame"
		fi
		;;
	get_wan_dyn_info)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	set_wan_dyn_info)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	update_wan_metrics)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			hostapd_cli update_wan_metrics $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	logger_modify)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	get_logger_info)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	get_logger_stream_info)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	logger_flush_buffers)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	fw_trigger_recovery)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			if [ $nArgs -eq 3 ]; then
				. $script $vap_index $@
			else
				echo "Usage: $SCRIPT_DIR/rc.bringup_wlan fw_trigger_recovery radio_index type time"
			fi
		else
			echo "FW recovery not supported by $WLAN_VENDOR_NAME"
		fi
		;;
	get_stats)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan get_stats vap_index"
		fi
		;;
	get_advanced_stats)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			. $script $vap_index $@
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	get_sta_advanced_stats)
		if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
			nMacs=$#
			if [ $nMacs -gt 0 ]; then
				. $script $vap_index $@
			else
				. $script
			fi
		else
			echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
		fi
		;;
	get_radio_advanced_stats)
		if [ $nArgs = 0 ]; then
			eval radio_count='$'wlan_phy_Count
			wave_found=0
			j=0
			while [ $j -lt $radio_count ]
			do
				find_vendor_from_index $j
				script=$SCRIPT_DIR/$WLAN_VENDOR_SCRIPT_PREFIX'_'$wlan_cmd
				if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
					wave_found=1
					. $script
					break
				fi
				j=`expr $j + 1`
			done
			if [ $wave_found -eq 0 ]; then
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		fi
		;;
	get_vap_advanced_stats)
		if [ $nArgs = 0 ]; then
			eval vap_count='$'wlan_main_Count
			wave_found=0
			j=0
			while [ $j -lt $vap_count ]
			do
				find_vendor_from_index $j
				script=$SCRIPT_DIR/$WLAN_VENDOR_SCRIPT_PREFIX'_'$wlan_cmd
				if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
					wave_found=1
					. $script
					break
				fi
				j=`expr $j + 1`
			done
			if [ $wave_found -eq 0 ]; then
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		fi
		;;
	get_ap_dyn_info)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan get_ap_dyn_info vap_index"
		fi
		;;
	get_assoc)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan get_assoc vap_index"
		fi
		;;
	get_radio_dyn_info)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan get_radio_dyn_info index"
		fi
		;;
	get_wps_dyn_info)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" -o "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan get_wps_dyn_info vap_index"
		fi
		;;
	get_wps_regs_dyn_info)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "ath" ]; then
				echo "QCA get wps regs dyn info"
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan get_wps_regs_dyn_info vap_index"
		fi
		;;
	ap_scan)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan ap_scan radio_index"
		fi
		;;
	ap_scan_get_last)
		if [ $nArgs -eq 1 ]; then
			if [ "$WLAN_VENDOR_NAME" = "wave" ]; then
				. $script $vap_index
			else
				echo "Command $wlan_cmd not supported by vendor $WLAN_VENDOR_NAME!!!!"
			fi
		else
			echo "Usage: $SCRIPT_DIR/rc.bringup_wlan ap_scan_get_last radio_index"
		fi
		;;
#	vb_config_mod)
#		if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
#			. $SCRIPT_DIR/wlan_sta_config 0
#		fi
#		;;
#	vb_wmm_config)
#		if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
#			. $SCRIPT_DIR/wave_wlan_wmm_modify $@
#		fi
#		;;
#	vb_trigger_connect)
#		if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
#			. $SCRIPT_DIR/wlan_connect
#		fi
#		;;
#	vb_trigger_disconnect)
#		if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
#			. $SCRIPT_DIR/wlan_disconnect
#		fi
#		;;
#	vb_get_wlan_link_status)
#		if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
#			. $SCRIPT_DIR/wlan_get_link_status 0
#		fi
#		;;
#	vb_get_wlan_scan_results)
#		if [ "$CONFIG_FEATURE_WIRELESS_WAVE300" = "1" ]; then
#			. $SCRIPT_DIR/wave_wlan_scan $@
#		fi
#		;;
	*)
#		echo $"Usage $0 {init|uninit|capability|start|stop|main_modify|radio_modify|radio_enable|vap_add|vap_remove|sec_modify|wmm_modify|wps_modify|wps_trigger_pbc|wps_trigger_pin|wps_get_pin|wps_reset_pin|wps_reset|wps_generate_pin|wps_get_profile|mac_ctrl_modify|wds_modify|hs20_modify|hs20_l2fw_modify|send_qos_map_conf|get_wan_dyn_info|set_wan_dyn_info|update_wan_metrics|logger_modify|get_logger_info|get_logger_stream_info|logger_flush_buffers|fw_trigger_recovery|get_stats|get_advanced_stats|get_ap_dyn_info|get_assoc|get_radio_dyn_info|get_wps_dyn_info|get_wps_regs_dyn_info|ap_scan|ap_scan_get_last|get_sta_advanced_stats|get_radio_advanced_stats|get_vap_advanced_stats|vb_config_mod|vb_wmm_config|vb_trigger_connect|vb_trigger_connect|vb_get_wlan_link_status|vb_get_wlan_scan_results}"
		echo $"Usage $0 {init|uninit|capability|start|stop|main_modify|radio_modify|radio_enable|vap_add|vap_remove|sec_modify|wmm_modify|wps_modify|wps_trigger_pbc|wps_trigger_pin|wps_get_pin|wps_reset_pin|wps_reset|wps_generate_pin|wps_get_profile|mac_ctrl_modify|wds_modify|hs20_modify|hs20_l2fw_modify|send_qos_map_conf|get_wan_dyn_info|set_wan_dyn_info|update_wan_metrics|logger_modify|get_logger_info|get_logger_stream_info|logger_flush_buffers|fw_trigger_recovery|get_stats|get_advanced_stats|get_ap_dyn_info|get_assoc|get_radio_dyn_info|get_wps_dyn_info|get_wps_regs_dyn_info|ap_scan|ap_scan_get_last|get_sta_advanced_stats|get_radio_advanced_stats|get_vap_advanced_stats}"
esac
