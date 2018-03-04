#!/bin/sh

script_name="wave_wlan_down.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pap_name
local radio_cpeid i current_radio_cpeid vap_name vaps_ppa current_vap in_l2f in_dgaf hs2_mode

pap_name=$1
down_mode=$2

ap_index=`find_index_from_wave_if $pap_name`
timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"
# Stop HS2.0 related components
# For each VAP of the radio to stop, check if in l2f and dgaf procs
# Get the cpeid of the radio to stop
eval radio_cpeid=\${wlmn_${ap_index}_radioCpeId}

# Go over all the interfaces and check for each VAP of the current radio to stop
i=0
while [ $i -lt $wlan_main_Count ]
do
	eval current_radio_cpeid=\${wlmn_${i}_radioCpeId}
	# Don't do anything for VAPs not belonging to the current radio
	[ $current_radio_cpeid -ne $radio_cpeid ] && i=$((i+1)) && continue

	# Don't do anything if Hotspot is disabled
	eval hs2_mode=\${wlhs2_${i}_hs20Mode}
	[ "$hs2_mode" = "$HS20_MODE_DISABLED" ] && i=$((i+1)) && continue

	eval vap_name=\${wlmnwave_${i}_interfaceName}

	# Unconditionaly disable of the parp - no API to know the status:
	echo "(. $ETC_PATH/wave_wifi_parp_ctrl.sh disable $vap_name)" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

	# Disable dgaf and l2f if they are enabled
	in_l2f=`cat /proc/net/wave_wifi_l2f/l2f | grep $vap_name -w`
	in_dgaf=`cat /proc/net/wave_wifi_l2f/dgaf | grep $vap_name -w`
	[ -n "$in_dgaf" ] && echo "( .$ETC_PATH/wave_wifi_dgaf_disable.sh $vap_name 0)" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	[ -n "$in_l2f" ] && echo "(. $ETC_PATH/wave_wifi_l2f_ctrl.sh disable $vap_name)" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

	# Disable wmd and hairpin
	if [ "$hs2_mode" = "$HS20_MODE_ENABLED" ]
	then
		echo "(. $ETC_PATH/wmdctrl.sh disable $vap_name)" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
		echo "(. $ETC_PATH/wave_wifi_hairpin_config.sh disable $vap_name)" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
	fi
	i=$((i+1))
done
# Remove all related VAPs from PPA if PPA exists, check if /sbin/ppacmd exists.
# The set_ppa_commands function writes the ppa commands for removal to the runner script.
vaps_ppa=""
[ -e /sbin/ppacmd ] && vaps_ppa=`ppacmd getlan | grep "\<$pap_name\>" | awk '{print $3}'`
current_vap=""
for current_vap in $vaps_ppa
do
	set_ppa_commands $current_vap "remove" $CONF_DIR/$WAVE_WLAN_RUNNNER
done

# Stop hostapd_cli and hostapd
echo "killall hostapd_cli_$pap_name 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "hostapd_cli_count=\`ps | grep hostapd_cli_$pap_name -c\`" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "hostapd_cli_down_timeout=0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "while [ "\$hostapd_cli_count" -gt 1 ] && [ "\$hostapd_cli_down_timeout" -lt 15 ]; do sleep 1; hostapd_cli_count=\`ps | grep hostapd_cli_$pap_name -c\`; hostapd_cli_down_timeout=\$((hostapd_cli_down_timeout+1)); done" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "[ "\$hostapd_cli_down_timeout" -eq 15 ] && echo wave_wlan_down.sh ERROR: HOSTAPD_CLI WAS KILLED BUT DID NOT DIE ON TIME > /dev/console && exit 1" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

echo "killall hostapd_$pap_name 2>/dev/null" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "hostapd_count=\`ps | grep hostapd_$pap_name -c\`" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "down_timeout=0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "while [ "\$hostapd_count" -gt 1 ] && [ "\$down_timeout" -lt 15 ]; do sleep 1; hostapd_count=\`ps | grep hostapd_$pap_name -c\`; down_timeout=\$((down_timeout+1)); done" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "[ "\$down_timeout" -eq 15 ] && echo wave_wlan_down.sh ERROR: HOSTAPD WAS KILLED BUT DID NOT DIE ON TIME > /dev/console && exit 1" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

# Verify all VAPs of the radio are down
echo "if_timeout=0" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "if_count=\`ifconfig | grep $pap_name -c\`" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "while [ "\$if_count" -gt 0 ] && [ "\$if_timeout" -lt 30 ]; do sleep 1; echo wave_wlan_down.sh WARNING: INTERFACES ARE UP AFTER HOSTAPD WAS KILLED > /dev/console; if_count=\`ifconfig | grep $pap_name -c\`; if_timeout=\$((if_timeout+1)); done" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "[ "\$if_timeout" -eq 30 ] && echo wave_wlan_down.sh ERROR: INTERFACES ARE UP AFTER HOSTAPD WAS KILLED > /dev/console && exit 1" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
#URGENT TODO - add support for apllication to trigger recovery

# Remove interface related hostapd softlinks
echo "rm -f /tmp/hostapd_$pap_name" >> $CONF_DIR/$WAVE_WLAN_RUNNNER
echo "rm -f /tmp/hostapd_cli_$pap_name" >> $CONF_DIR/$WAVE_WLAN_RUNNNER

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
