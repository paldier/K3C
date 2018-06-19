#!/bin/sh

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh

command=$1
pc_ip=$2
param1=$3


interface_down()
{
	local wave_count

	rm -f ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}

	. ${CONF_DIR}/fapi_wlan_wave_discover.txt
	wave_count=$((PCI_LTQ_COUNT+AHB_WLAN_COUNT))

	if [ "$wave_count" = "2" ]
	then
		(. $ETC_PATH/fapi_wlan_wave_down wlan2)
		touch ${CONF_IN_PROGRESS}_wlan2}
	fi

	(. $ETC_PATH/fapi_wlan_wave_down wlan0)
	touch ${CONF_IN_PROGRESS}_wlan0}
}

restart_wave()
{
	(. $ETC_PATH/fapi_wlan_wave_complete_recovery wlan0)
}

burn_cal_file()
{
	local no_restart tftp_path interfaces_list interface_name burn_both cal_status

	no_restart=$1
	[ -z "$pc_ip" ] && echo "The PC IP parameter is missing." && exit

	tftp_path=${param1%\/*}
	interfaces_list=${param1##*\/}
	if [ "$tftp_path" = "$interfaces_list" ]
	then
		tftp_path=""
	else
		tftp_path="$tftp_path/"
	fi

	cd /tmp/
	cal_status=0
	interface_name=${interfaces_list%%,*}
	while [ -n "$interface_name" ]
	do
		if [ "$interface_name" = "all" ]
		then
			tftp -gr "${tftp_path}cal_wlan0.bin" -l cal_wlan0.bin $pc_ip
			cal_status=$(( $cal_status + `echo $?` ))
			tftp -gr "${tftp_path}cal_wlan2.bin" -l cal_wlan2.bin $pc_ip
			cal_status=$(( $cal_status + `echo $?` ))
			tftp -gr "${tftp_path}cal_wlan4.bin" -l cal_wlan4.bin $pc_ip
			cal_status=$(( $cal_status + `echo $?` ))
		else
			tftp -gr "${tftp_path}cal_${interface_name}.bin" -l cal_${interface_name}.bin $pc_ip
			cal_status=$(( $cal_status + `echo $?` ))
		fi
		interfaces_list=${interfaces_list#$interface_name}
		interfaces_list=${interfaces_list#,}
		interface_name=${interfaces_list%%,*}
	done

	tar czf eeprom.tar.gz cal_*.bin
	if [ -d /nvram ]
	then
		cp /tmp/eeprom.tar.gz /nvram/
	else
		upgrade /tmp/eeprom.tar.gz wlanconfig 0 0
	fi
	cal_status=$(( $cal_status + `echo $?` ))

	if [ "$cal_status" -eq 0 ]
	then
		if [ -z "$no_restart" ]
		then
			sync
			reboot
		fi
	else
		echo "***********************************"
		echo "ERROR - calibration file not burned!"
		echo "***********************************"
	fi

	cd - > /dev/null
}

replace_driver()
{
	[ -z "$pc_ip" ] && echo "Parameter missing: execute $script_name driver <your PC IP>" && exit
	echo "Make sure your tftp is set to the folder with the mtlk.ko and mtlkroot.ko files"
	cd $DRIVER_PATH
	tftp -gr mtlk.ko $pc_ip
	sleep 5
	tftp -gr mtlkroot.ko $pc_ip
	sleep 1
	sync
	#restart_wave
	reboot
}

replace_fw()
{
	[ -z "$pc_ip" ] && echo "Parameter missing: execute $script_name fw <your PC IP>" && exit
	echo "Make sure your tftp is set to the folder with the FW binary file"

	# Source config.sh
	. $ETC_PATH/config.sh

	cd $IMAGES_PATH
	[ -e ap_upper_ar10.bin ] && tftp -gr ap_upper_ar10.bin $pc_ip
	[ -e ap_upper_gen4.bin ] && tftp -gr ap_upper_gen4.bin $pc_ip

	# If platform is Gen5, get the gen5 FW binaries
	if [ "$CONFIG_IFX_CONFIG_CPU" = "GRX500" ]
	then
		tftp -gr ap_upper_gen5.bin $pc_ip
		tftp -gr ap_lower_gen5.bin $pc_ip
	fi
	#restart_wave
	reboot
	cd - > /dev/null
}

tftp_bins()
{
	cd /tmp
	serverip=`uboot_env --get --name serverip`

	rm -f /tmp/ls.txt /tmp/ls.unix

	tftp -gr ls.txt $serverip

	if [ $? -eq 0 ]
	then
		# dos2unix by using awk
		awk '{ sub("\r$", ""); print }' ls.txt > ls.unix

		rcd_list=`ls $ETC_PATH`
		MINI_FO_MOUNTED="NO"

		for file in `cat ls.unix`
		do
			echo "echo \" +++ Downloading $file +++\"" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
			echo "rm -f $file" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
			echo "tftp -gr $file $serverip" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
			echo "chmod +x $file" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
			if [ `echo $rcd_list | grep $file` ]
			then
				if [ "$MINI_FO_MOUNTED" = "NO" ]
				then
					echo "mini_fo.sh mount $ETC_PATH" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
					MINI_FO_MOUNTED=YES
				fi
				echo "mv $file ${ETC_PATH}/${file}" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
			fi
		done
	else
			echo -e "*******************************************************"
			for i in 3 5 9
			do
				echo -e "\033[3${i}m ***Download of ls.txt FAILED, Loading image from flash***"
				usleep 50000
			done
			echo -e "*******************************************************"
	fi
	cd - > /dev/null
}

internal_radio()
{
	internal_state=$pc_ip
	# Create notification to modify WaveDisableAHB state accoring to user's input
	if [ "$internal_state" = "0" ] || [ "$internal_state" = "1" ]
	then
		set_value="true"
		[ "$internal_state" = "1" ] && set_value="false"
		build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:wlan0 Object:${RADIO_VENDOR_OBJECT} WaveDisableAHB:${set_value}"
		[ "$set_value" = "true" ] && build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:wlan0 Object:${RADIO_VENDOR_OBJECT} WaveFastpathEnabled:true"
	else
		echo "Illegal value set in WaveDisableAHB ($internal_state)" > /dev/console
	fi
}

yocto_debug()
{
	mkdir /nvram/wave_scripts/
	cp /etc/wave/scripts/* /nvram/wave_scripts/
}

check_overlay()
{
	(. ${ETC_PATH}/fapi_wlan_wave_check_overlay.sh no_sleep)
}

dump_fapi()
{
	cd /tmp/
	ts=`date +%s`
	which csdutil > /dev/null
	[ $? -eq 0 ] && csdutil decrypt /opt/lantiq/config/.run-data.xml ${CONF_DIR}/db_dump.xml
	echo "##### iwconfig output #####" > ${CONF_DIR}/commands_output.txt
	iwconfig >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### ifconfig output #####" >> ${CONF_DIR}/commands_output.txt
	ifconfig >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### ifconfig -a output #####" >> ${CONF_DIR}/commands_output.txt
	ifconfig -a >> ${CONF_DIR}/commands_output.txt
	echo "##### iwlist peers output #####" >> ${CONF_DIR}/commands_output.txt
	iwlist peers >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### hostapd wlan0 status output #####" >> ${CONF_DIR}/commands_output.txt
	hostapd_cli -iwlan0 stat >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### hostapd wlan2 status output #####" >> ${CONF_DIR}/commands_output.txt
	hostapd_cli -iwlan2 stat >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### hostapd wlan4 status output #####" >> ${CONF_DIR}/commands_output.txt
	hostapd_cli -iwlan4 stat >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### supplicant wlan1 status output #####" >> ${CONF_DIR}/commands_output.txt
	wpa_cli -iwlan1 stat >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### supplicant wlan3 status output #####" >> ${CONF_DIR}/commands_output.txt
	wpa_cli -iwlan3 stat >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "##### supplicant wlan5 status output #####" >> ${CONF_DIR}/commands_output.txt
	wpa_cli -iwlan5 stat >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	echo "" >> ${CONF_DIR}/commands_output.txt
	tar czf fapi_dump_${ts}.tar.gz wlan_wave
	echo "FAPI dump file created: /tmp/fapi_dump_${ts}.tar.gz" > /dev/console
}

case $command in
	if_down)
		interface_down
	;;
	if_up)
		interface_up
	;;
	restart)
		restart_wave
	;;
	burn_cal)
		burn_cal_file
	;;
	burn_cal_dut)
		burn_cal_file no_restart
	;;
	driver)
		replace_driver
	;;
	fw)
		replace_fw
	;;
	tftp_bins)
		tftp_bins
	;;
	set_internal)
		internal_radio
	;;
	yocto_debug)
		yocto_debug
	;;
	check_overlay)
		check_overlay
	;;
	"dump_fapi"|"df")
		dump_fapi
	;;
	*)
		echo "$script_name: Unknown command $command"
		echo "Usage: $script_name COMMAND [Argument 1] [Argument 2]"
		echo ""
		echo "Commnads:"
		echo "if_down        Bring down all the interfaces."
		echo "if_up          Bring up all the interfaces."
		echo "restart        Restart the Wlan interfaces (including rmmod and insmod of Wlan driver)"
		echo "burn_cal       Burn the calibration files"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "  Argument 2:  The interface name or names to which calibration is burned: wlan0/wlan2/wlan4/all"
		echo "               Names can be specified in a comma-separated list: wlan0,wlan2"
		echo "               This argument can contain also the path in the tftp server before the interface name: /path/wlan"
		echo "               Example: $script_name burn_cal <PC IC> /private_folder/wlan0,wlan2,wlan4"
		echo "burn_cal_dut   Burns calibration file without restart"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "  Argument 2:  The interface name to which calibration is burned: wlan0/wlan2/both"
		echo "               Default value if Argument 2 is not set: wlan0"
		echo "               This argument can contain also the path in the tftp server before the interface name: /path/wlan"
		echo "               Example: $script_name burn_cal_dut <PC IC> /private_folder/both"
		echo "driver         Replace driver binaries (mtlk.ko and mtlkroot.ko)"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "               Make sure to have the mtlk.ko and mtlkroot.ko files in the tftp folder in your PC"
		echo "fw             Replace FW binary (ap_upper and ap_lower binaries)"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "               Make sure to have the ap_upper and ap_lower files in the tftp folder in your PC"
		echo "set_internal   Enable/disable internal on-board WLAN"
		echo "  Arguments:"
		echo "  Argument 1:  Mode to set: 0/1"
		echo "yocto_debug    Add ability to edit FAPI scripts on yocto based platforms. Edit the scritps in the folder /nvram/wave_scripts/"
		echo "check_overlay  Compare between the driver, FW, progmodels version in version.sh and in driver /proc"
		echo "               List files in /overlay folder"
		echo "dump_fapi      Create a tarball FAPI and SL debug (df also can be used)"
	;;
esac
