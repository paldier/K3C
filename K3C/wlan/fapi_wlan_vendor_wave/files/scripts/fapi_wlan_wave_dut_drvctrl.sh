#!/bin/sh

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh

drvhlpr_dut="drvhlpr_dut"

command=$1

# Find the interface index of wlan0
interface_index=`find_index_from_interface_name wlan0`

################################################################
# Probe to get file to use for burn cal_wlan files:

drvhlpr_app="$BINDIR/drvhlpr"
dut_file_saver_file=${ETC_PATH}/fapi_wlan_wave_dut_file_saver.sh

if [ -e /tmp/dut_aps_and_scripts.sh ]
then
	print2log $interface_index DEBUG "/tmp/dut_aps_and_scripts.sh exist, use it"
	. /tmp/dut_aps_and_scripts.sh > /dev/null
	print2log $interface_index DEBUG "dut_file_saver_file=$dut_file_saver_file"
	print2log $interface_index DEBUG "drvhlpr_app=$drvhlpr_app"
fi

################################################################

start_dut_drvctrl()
{
	# Delete the runner file.
	rm -f ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	# Start drvhlpr for dut
	print2log $interface_index DEBUG "Use drvhlpr_app=$drvhlpr_app"
	echo "cp -s $drvhlpr_app /tmp/$drvhlpr_dut" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	echo "echo \"f_saver = $dut_file_saver_file\" > /tmp/${drvhlpr_dut}.config" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}	
	echo "/tmp/$drvhlpr_dut --dut -p /tmp/${drvhlpr_dut}.config &" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	# Execute the runner
	chmod +x ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
}

stop_drvhlpr()
{
	print2log $interface_index DEBUG "$script_name: Stop DUT Helper"
	# Delete the runner file.
	rm -f ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	# kill dutserver
	echo "killall $drvhlpr_dut 2>/dev/null" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	echo "killall drvhlpr_wlan0 2>/dev/null" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	echo "killall drvhlpr_wlan2 2>/dev/null" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	echo "drvhlpr_count=\`ps | grep drvhlpr_wlan'[0-1]\{1\}' -c\`" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	echo "while [ "\$drvhlpr_count" -gt 0 ]; do echo wait_drvhlpr_close > /dev/console; sleep 1; drvhlpr_count=\`ps | grep drvhlpr_wlan'[0-1]\{1\}' -c\`; done" >> ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}	
	print2log $interface_index DEBUG "$script_name: Stop DUT Helper Done"
	# Execute the runner
	chmod +x ${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
	${CONF_DIR}/${FAPI_WLAN_WAVE_RUNNNER}
}

#Arad: Fix when restart works on wave500:
case $command in
	start)
		start_dut_drvctrl
	;;
	stop)
		stop_drvhlpr
	;;
	*)
		echo "$script_name: Unknown command=$command" > /dev/console
	;;
esac
