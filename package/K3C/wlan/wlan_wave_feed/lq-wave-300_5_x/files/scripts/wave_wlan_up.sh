#!/bin/sh

script_name="wave_wlan_up.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pap_name
local rtlog0_count in_runner bridge_mac ap_index0 driver_debug_level vaps_post_up vaps_single_execute

pap_name=$1
up_mode=$2

ap_index=`find_index_from_wave_if $pap_name`
timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

# TODO: remove the following lines once logger will be configured on init
# Set MAC address to the logger interface and add it to the bridge. The MAC used is the bridge MAC
# Handle logger interface only if it doesn't exist yet
rtlog0_count=`ifconfig | grep rtlog0 -c`
if [ $rtlog0_count -eq 0 ]
then
	# Verify that the commands are not already in the runner
	in_runner=0
	[ -e $CONF_DIR/$WAVE_WLAN_RUNNNER ] && in_runner=`grep "ifconfig rtlog0 hw ether" -c $CONF_DIR/$WAVE_WLAN_RUNNNER`
	if [ "$in_runner" = "0" ]
	then
		bridge_mac=`uboot_env --get --name ethaddr`
		# Workaround to ignore un-needed output from uboot_env application
		bridge_mac=${bridge_mac##*complete }
		bridge_mac=${bridge_mac%%param*}
		bridge_mac=`echo $bridge_mac`
		echo "ifconfig rtlog0 hw ether $bridge_mac" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
		echo "brctl addif br0 rtlog0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
		echo "ifconfig rtlog0 up" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
		# Write switch_cli command to add the logger to the switch MAC table
		echo "switch_cli dev=1 GSW_MAC_TABLE_ENTRY_ADD nFId=0 nPortId=9 nSubIfId=128 bStaticEntry=1 nMAC=00:00:00:00:00:10" >> ${CONF_DIR}/${WAVE_WLAN_RUNNNER}
	fi
	# Create the logger streams, currently only for wlan0.
	(. $ETC_PATH/wave_wlan_logger_modify $ap_index init)
fi

# Copy the pre-up driver commands to the runner only if not in recovery mode (no need to reconfigure parameters since no change is done)
[ "$up_mode" != "IN_FW_RECOVERY" ] && cat $TEMP_CONF_DIR/${DRIVER_PRE_UP_CONF_PREFIX}_${pap_name}.conf >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Request to always change adaptive sensitivity threshold to this value:
echo "iwpriv $pap_name sSetRxTH -82" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Merge hostapd configuration files
echo "cat ${TEMP_CONF_DIR}/${HOSTAPD_PHY_CONF_PREFIX}_${pap_name}.conf $TEMP_CONF_DIR/${HOSTAPD_VAP_CONF_PREFIX}_${pap_name}.conf > ${CONF_DIR}/hostapd_${pap_name}.conf" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "cat ${TEMP_CONF_DIR}/${HOSTAPD_VAP_CONF_PREFIX}_${pap_name}.*.conf >> ${CONF_DIR}/hostapd_${pap_name}.conf 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Change the default maximum size of nl received buffer in the kernel
echo "echo 262144 > /proc/sys/net/core/rmem_max" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Start hostapd
echo "cp -s $BINDIR/hostapd /tmp/hostapd_$pap_name" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
# Read the driver debug level set in wlan0, if driver is in debug level higher than 0, start hostapd also with debug prints.
ap_index0=$ap_index
[ "$interface_name" != "wlan0" ] && ap_index0=`find_index_from_wave_if wlan0`
eval driver_debug_level=\${wlphywave_${ap_index0}_driverDebugLevel}
if [ $driver_debug_level -gt 0 ]
then
	echo "/tmp/hostapd_$pap_name -ddt $CONF_DIR/hostapd_$pap_name.conf -e /tmp/hostapd_ent_$pap_name &" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "sleep 4" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
else
	echo "/tmp/hostapd_$pap_name $CONF_DIR/hostapd_$pap_name.conf -e /tmp/hostapd_ent_$pap_name -B" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
fi

# Wait for all the interface to be up.
# Count number of bss= in hostapd conf file (+1 for physical interface) and cound interfaces in ifconfig -a command.
echo "num_vaps=\`grep \"^bss=\" -c $CONF_DIR/hostapd_$pap_name.conf\`" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "num_vaps=\$((num_vaps+1))" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "up_timeout=0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "while [ \`ifconfig -a | grep '^$pap_name' -c\` -lt \$num_vaps ] && [ \$up_timeout -lt 30 ]" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "do sleep 1; up_timeout=\$((up_timeout+1)); done" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Start hostapd_cli to listen to events
echo "cp -s $BINDIR/hostapd_cli /tmp/hostapd_cli_$pap_name" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "/tmp/hostapd_cli_$pap_name -i$pap_name -a$HOSTAPD_EVENTS_SCRIPT -B" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Restart drvhlpr. It is for stop+start case, to make sure that the latest drvhlpr conf is taken.
if [ "$up_mode" != "IN_FW_RECOVERY" ]
then
	echo "cp -s $BINDIR/drvhlpr /tmp/drvhlpr_$pap_name" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "cp $TEMP_CONF_DIR/drvhlpr_$pap_name.conf $CONF_DIR/drvhlpr_$pap_name.conf" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "killall drvhlpr_$pap_name 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "drvhlpr_count=\`ps | grep drvhlpr_$pap_name -c\`" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "while [ "\$drvhlpr_count" -gt 1 ]; do sleep 1; drvhlpr_count=\`ps | grep drvhlpr_$pap_name -c\`; done" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	echo "/tmp/drvhlpr_$pap_name -p $CONF_DIR/drvhlpr_$pap_name.conf </dev/console 1>/dev/console 2>&1 &" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
fi

# Copy post-up driver commands to the runner (otf commands).
vaps_post_up=`ls $TEMP_CONF_DIR/${DRIVER_POST_UP_CONF_PREFIX}_${pap_name}*`
for post_up in $vaps_post_up
do
	cat $post_up >> $CONF_DIR/$WAVE_WLAN_RUNNNER
done

# Copy commands that need to be executed only once to the runner and delete the conf files for these commands
vaps_single_execute=`ls $TEMP_CONF_DIR/${DRIVER_SINGLE_CALL_CONFIG_FILE}_${pap_name}* 2>/dev/null`
for single_execute in $vaps_single_execute
do
	cat $single_execute >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	rm $single_execute
done

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
