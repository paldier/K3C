#!/bin/sh

script_name="wave_wlan_debug.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh

command=$1
pc_ip=$2
param1=$3


interface_down()
{
	(. /etc/rc.d/wave_wlan_stop)
}

interface_up()
{
	(. /etc/rc.d/wave_wlan_start)
}

restart_wave()
{
	(. /etc/rc.d/wave_wlan_stop)
	(. /etc/rc.d/wave_wlan_uninit)
	(. /etc/rc.d/wave_wlan_init)
	(. /etc/rc.d/wave_wlan_start)
}

burn_cal_file()
{
	no_restart=$1
	[ -z "$pc_ip" ] && echo "Parameter missing: execute /etc/rc.d/wave_wlan_debug.sh burn_cal <your PC IP> <interface name>" && exit

	tftp_path=${param1%\/*}
	interface_name=${param1##*\/}
	[ "$tftp_path" = "$interface_name" ] && tftp_path=""
	tftp_path="$tftp_path/"

	[ -z $interface_name ] && interface_name="wlan0"
	cd /tmp/
	
	[ "$interface_name" = "both" ] && interface_name="wlan0" && burn_both="yes"
	
	cal_status=0
	tftp -gr "${tftp_path}cal_${interface_name}.bin" -l cal_${interface_name}.bin $pc_ip
	cal_status=$(( $cal_status + `echo $?` ))

	if [ "$burn_both" = "yes" ]
	then
		tftp -gr "${tftp_path}cal_wlan1.bin" -l cal_wlan1.bin $pc_ip
	fi

	cal_status=$(( $cal_status + `echo $?` ))
	tar czf eeprom.tar.gz cal_*.bin
	upgrade /tmp/eeprom.tar.gz wlanconfig 0 0
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
	[ -z "$pc_ip" ] && echo "Parameter missing: execute /etc/rc.d/wave_wlan_debug.sh driver <your PC IP>" && exit
	echo "Make sure your tftp is set to the folder with the mtlk.ko and mtlkroot.ko files"
	cd /lib/modules/3.10.12/net/
	tftp -gr mtlk.ko $pc_ip
	tftp -gr mtlkroot.ko $pc_ip
	restart_wave
	cd - > /dev/null
}

replace_fw()
{
	[ -z "$pc_ip" ] && echo "Parameter missing: execute /etc/rc.d/wave_wlan_debug.sh fw <your PC IP>" && exit
	echo "Make sure your tftp is set to the folder with the FW binary file"

	# Source config.sh
	. $ETC_PATH/config.sh

	cd /root/mtlk/images/
	[ -e ap_upper_ar10.bin ] && tftp -gr ap_upper_ar10.bin $pc_ip
	[ -e ap_upper_gen4.bin ] && tftp -gr ap_upper_gen4.bin $pc_ip

	# If platform is Gen5, get the gen5 FW binaries
	if [ "$CONFIG_IFX_CONFIG_CPU" = "GRX500" ]
	then
		tftp -gr ap_upper_gen5.bin $pc_ip
		tftp -gr ap_lower_gen5.bin $pc_ip
	fi
	restart_wave
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

		rcd_list=`ls /etc/rc.d`
		MINI_FO_MOUNTED="NO"

		for file in `cat ls.unix`
		do
			echo "echo \" +++ Downloading $file +++\"" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
			echo "rm -f $file" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
			echo "tftp -gr $file $serverip" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
			echo "chmod +x $file" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
			if [ `echo $rcd_list | grep $file` ]
			then
				if [ "$MINI_FO_MOUNTED" = "NO" ]
				then
					echo "mini_fo.sh mount /etc/rc.d/" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
					MINI_FO_MOUNTED=YES
				fi
				echo "mv $file /etc/rc.d/$file" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
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

set_ahb_state()
{
	ahb_state=$pc_ip

	[ -z "$ahb_state" ] && ahb_state=2
	case $ahb_state in
		"enable"|"1")
			set_value="1"
		;;
		"disable"|"0")
			set_value="0"
		;;
		"auto"|"2")
			set_value="2"
		;;
	esac
	status_oper -u -f /ramdisk/flash/rc.conf SET wlan_phy_vendor_wave wlphywave_0_internalWlanEna "$set_value"
	/etc/rc.d/backup
	echo "Make sure to reboot your device for changes to take effect" > /dev/console
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
	wave400)
		set_ahb_state
	;;
	*)
		echo "$script_name: Unknown command $command"
		echo "Usage: /etc/rc.d/$script_name COMMAND [Argument 1] [Argument 2]"
		echo ""
		echo "Commnads:"
		echo "if_down        Bring down all the interfaces."
		echo "if_up          Bring up all the interfaces."
		echo "restart        Restart the Wlan interfaces (including rmmod and insmod of Wlan driver)"
		echo "burn_cal       Burn the calibration files"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "  Argument 2:  The interface name to which calibration is burned: wlan0/wlan1/both"
		echo "               Default value if Argument 2 is not set: wlan0"
		echo "               This argument can contain also the path in the tftp server before the interface name: /path/wlan"
		echo "               Example: /etc/rc.d/$script_name burn_cal <PC IC> /private_folder/both"
		echo "burn_cal_dut   Burns calibration file without restart"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "  Argument 2:  The interface name to which calibration is burned: wlan0/wlan1/both"
		echo "               Default value if Argument 2 is not set: wlan0"
		echo "               This argument can contain also the path in the tftp server before the interface name: /path/wlan"
		echo "               Example: /etc/rc.d/$script_name burn_cal_dut <PC IC> /private_folder/both"
		echo "driver         Replace driver binaries (mtlk.ko and mtlkroot.ko)"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "               Make sure to have the mtlk.ko and mtlkroot.ko files in the tftp folder in your PC"
		echo "fw             Replace FW binary (ap_upper and ap_lower binaries)"
		echo "  Arguments:"
		echo "  Argument 1:  Your PC IP"
		echo "               Make sure to have the ap_upper and ap_lower files in the tftp folder in your PC"
		echo "wave400        Change the state of the internal Wlan (Wave400)"
		echo "  Arguments:"
		echo "  Argument 1:  The state of the internal Wlan to set enable/disable/auto"
	;;
esac
