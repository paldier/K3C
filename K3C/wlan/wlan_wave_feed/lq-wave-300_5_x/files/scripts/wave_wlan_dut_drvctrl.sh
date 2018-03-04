#!/bin/sh

script_name="wave_wlan_dut_drvctrl.sh"

# Define local parameters
local drvhlpr_dut command

drvhlpr_dut="drvhlpr_dut"

command=$1

stop_dut_helper()
{
	print2log 0 DEBUG "$script_name: Stop DUT Helper"
	# Delete the runner file.
	rm -f $CONF_DIR/$WAVE_WLAN_RUNNNER
	# kill dutserver
	echo "killall $drvhlpr_dut" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "killall drvhlpr_wlan0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Execute the runner
	chmod +x $CONF_DIR/$WAVE_WLAN_RUNNNER
	$CONF_DIR/$WAVE_WLAN_RUNNNER
	print2log 0 DEBUG "$script_name: Stop DUT Helper Done"
}

stop_dut_drvhlpr_only()
{
	print2log 0 DEBUG "$script_name: Stop DUT Helper"
	# Delete the runner file.
	rm -f $CONF_DIR/$WAVE_WLAN_RUNNNER
	# kill dutserver
	#echo "killall $drvhlpr_dut" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "killall drvhlpr_wlan0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Execute the runner
	chmod +x $CONF_DIR/$WAVE_WLAN_RUNNNER
	$CONF_DIR/$WAVE_WLAN_RUNNNER
	print2log 0 DEBUG "$script_name: Stop DUT Helper Done"
}

stop_dut_drvctrl_only()
{
	stop_dut_helper
}

stop_dut_drvctrl()
{
	[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
	# rmmod mtlk driver
	(. $ETC_PATH/wave_wlan_uninit)
	stop_dut_helper
}

first_stop_dut_drvctrl()
{
	# We have to stop all the wave AP/VAPs
	(. $ETC_PATH/wave_wlan_stop)
	# rmmod mtlk driver
	(. $ETC_PATH/wave_wlan_uninit)
}

start_dut_drvhlpr_only()
{
	# Delete the runner file.
	[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
	rm -f $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Start drvhlpr for dut
	echo "cp -s $BINDIR/drvhlpr /tmp/$drvhlpr_dut" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "echo \"f_saver = $ETC_PATH/wave_wlan_dut_file_saver.sh\" > /tmp/${drvhlpr_dut}.config" >> $CONF_DIR/$WAVE_WLAN_RUNNNER 
	echo "/tmp/$drvhlpr_dut --dut -p /tmp/${drvhlpr_dut}.config &" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Execute the runner
	chmod +x $CONF_DIR/$WAVE_WLAN_RUNNNER
	$CONF_DIR/$WAVE_WLAN_RUNNNER
	touch /tmp/dut_mode_on
}

start_dut_drvctrl()
{
	# insmod the driver
	(. /etc/rc.d/wave_wlan_init dut)
	# Delete the runner file.
	[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
	rm -f $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Start drvhlpr for dut
	echo "cp -s $BINDIR/drvhlpr /tmp/$drvhlpr_dut" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "echo \"f_saver = $ETC_PATH/wave_wlan_dut_file_saver.sh\" > /tmp/${drvhlpr_dut}.config" >> $CONF_DIR/$WAVE_WLAN_RUNNNER 
	echo "/tmp/$drvhlpr_dut --dut -p /tmp/${drvhlpr_dut}.config &" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Execute the runner
	chmod +x $CONF_DIR/$WAVE_WLAN_RUNNNER
	$CONF_DIR/$WAVE_WLAN_RUNNNER

}


###############################################################################


_start_dut_drvctrl()
{
	[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
	# Delete the runner file.
	rm -f $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Start drvhlpr for dut
	echo "cp -s $BINDIR/drvhlpr /tmp/$drvhlpr_dut" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "echo \"f_saver = $ETC_PATH/wave_wlan_dut_file_saver.sh\" > /tmp/${drvhlpr_dut}.config" >> $CONF_DIR/$WAVE_WLAN_RUNNNER 
	echo "/tmp/$drvhlpr_dut --dut -p /tmp/${drvhlpr_dut}.config &" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	# Execute the runner
	chmod +x $CONF_DIR/$WAVE_WLAN_RUNNNER
	$CONF_DIR/$WAVE_WLAN_RUNNNER
}

stop_drvhlpr()
{
	[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
	print2log 0 DEBUG "wave_wlan_dut_drvctrl.sh: Stop DUT Helper"
	# Delete the runner file.
	rm -f $CONF_DIR/$WAVE_WLAN_RUNNNER
	# kill dutserver
	echo "killall $drvhlpr_dut 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "killall drvhlpr_wlan0 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "killall drvhlpr_wlan1 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "drvhlpr_count=\`ps | grep drvhlpr_wlan'[0-1]\{1\}' -c\`" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "while [ "\$drvhlpr_count" -gt 0 ]; do echo wait_drvhlpr_close > /dev/console; sleep 1; drvhlpr_count=\`ps | grep drvhlpr_wlan'[0-1]\{1\}' -c\`; done" >> $CONF_DIR/$WAVE_WLAN_RUNNNER	
	print2log 0 DEBUG "wave_wlan_dut_drvctrl.sh: Stop DUT Helper Done"
	# Execute the runner
	chmod +x $CONF_DIR/$WAVE_WLAN_RUNNNER
	$CONF_DIR/$WAVE_WLAN_RUNNNER
}

#Arad: TODO when restart works on wave500:
# 1- replace 'start' with 'start_drvhlpr_only' and 'stop' 'with stop_drvhlpr_only'
# 2- change '_start' to 'start' and '_stop' to 'stop'
case $command in
	_start)
		start_dut_drvctrl
	;;
	#start_drvhlpr_only)
	start)
		#start_dut_drvhlpr_only
		_start_dut_drvctrl
	;;
	#stop_drvhlpr_only)
	stop)
		[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
	#	if [ -e /tmp/dut_mode_on ]
	#	then
	#		stop_dut_drvctrl_only
	#	else
	#		stop_dut_drvhlpr_only
	#	fi
		stop_drvhlpr
	;;
	_stop)
		[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
		if [ -e /tmp/dut_mode_on ]
		then
			stop_dut_drvctrl
		else
			first_stop_dut_drvctrl
		fi
	;;
	*)
		echo "wave_wlan_dut_drvctrl.sh: Unknown command=$command" > /dev/console
    ;;
esac
