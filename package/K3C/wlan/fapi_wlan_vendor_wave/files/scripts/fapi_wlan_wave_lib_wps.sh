#!/bin/sh

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh

export HOSTAPD_PIN_REQ=/var/run/hostapd.pin-req
export WPS_PIN_TEMP_FILE=${CONF_DIR}/wps_current_pin
export WPS_MAC_TEMP_FILE=${CONF_DIR}/wps_current_mac

# Read uuid from DB. If no value is set, generate new uuid
# Return the uuid.
read_uuid()
{
	# Define local parameters
	local interface_index uuid

	interface_index=$1

	uuid=`db2fapi_convert regular UUID $interface_index`
	if [ -z "$uuid" ]
	then
		if [ -e /tmp/uuid_placeholder ]
		then
			uuid=`cat /tmp/uuid_placeholder`
		else
			which uuidgen > /dev/null
			if [ $? -eq 0 ]
			then
				uuid=`uuidgen`
			else
				uuid=`cat /proc/sys/kernel/random/uuid`
			fi
			[ -n $uuid ] && echo $uuid > /tmp/uuid_placeholder
		fi
	fi

	echo "$uuid"
}

set_ap_configured()
{
	local interface_name wps_vendor_index interface_index radio_index wps_state

	interface_name=$1
	wps_vendor_index=$2
	interface_index=$3
	radio_index=$4

	print2log $radio_index DEBUG "$script_name: Start set_ap_configured"

	wps_state=`convert_wps_state $interface_index $radio_index`

	# If it is a non-configured AP
	if [ "$wps_state" = "$WPS_ENABLED_NOT_CONFIGURED" ]
	then
		print2log $radio_index DEBUG "$script_name: set_ap_configured. AP is non-configured, setting to configured"
		update_conf_out "Object_${wps_vendor_index}" "$RADIO_WPS_VENDOR_OBJECT"
		update_conf_out "ConfigState_${wps_vendor_index}" "Configured"
		# TBD: need to add ubus call that will re-trigger the WPS PBC with AP as configured. concider: how much time will it all take?
	else
		print2log $radio_index DEBUG "$script_name: set_ap_configured. AP is in configured mode"
	fi
	print2log $radio_index DEBUG "$script_name: End set_ap_configured"
}

wps_connect_via_pbc()
{
	local interface_name radio_name wps_vendor_index interface_index radio_index

	interface_name=$1
	radio_name=$2
	wps_vendor_index=$3

	# Find the interface index and the radio index
	interface_index=`find_index_from_interface_name $interface_name`
	radio_index=`find_index_from_interface_name $radio_name`

	print2log $radio_index WPS "FAPI_WLAN_VENDOR_WAVE: The button 'Start PBC' was activated for $interface_name"

	# send WPS-SESSION-START event to the WLAN events script
	( . $HOSTAPD_EVENTS_SCRIPT $radio_name WPS-SESSION-START $interface_name )
	# Set AP to configured mode.
	set_ap_configured $interface_name $wps_vendor_index $interface_index $radio_index
	# Start the PBC session
	/tmp/hostapd_cli_${radio_name} -i $radio_name wps_pbc $interface_name
	#/tmp/hostapd_cli_${radio_name} -i $interface_name wps_pbc
}

# Start a WPS PIN session for the AP
wps_connect_via_pin()
{
	local interface_index interface_name radio_name wps_vendor_index enrollee_mac enrollee_uuid \
	radio_index enrollee_pin enrollee_type wps_pin_timeout line_timestamp line_uuid line_mac param4 \
	line res need_uuid

	interface_index=$1
	interface_name=$2
	radio_name=$3
	wps_vendor_index=$4
	enrollee_mac=$5
	enrollee_uuid=$6

	# Find the radio index
	radio_index=`find_index_from_interface_name $radio_name`

	# Read enrollee PIN
	enrollee_pin=`db2fapi_convert regular EndpointPIN $interface_index`
	[ -z "$enrollee_pin" ] && . $WPS_PIN_TEMP_FILE

	# If enrollee MAC is missing, check if authorized MAC is set
	[ -z "$enrollee_mac" ] && enrollee_mac=`db2fapi_convert regular AuthorizedMac $interface_index`
	# Make enrollee_mac lower case as appears in the pin requests file
	enrollee_mac=`echo "$enrollee_mac" | tr '[:upper:]' '[:lower:]'`

	enrollee_type=2
	# Using hard-coded timeout of 30 minutes
	wps_pin_timeout=1800

	print2log $radio_index WPS "FAPI_WLAN_VENDOR_WAVE: The button 'Connect' for PIN connection was activated for $interface_name with PIN=$enrollee_pin and MAC=$enrollee_mac"

	# If PIN wasn't received, show warning and exit
	[ -z "$enrollee_pin" ] && print2log $radio_index WARNING "$script_name: PIN connection started but no PIN number received" && exit

	# Save PIN and MAC of STA in temp files
	echo "enrollee_pin=$enrollee_pin" > $WPS_PIN_TEMP_FILE
	echo "enrollee_mac=$enrollee_mac" > $WPS_MAC_TEMP_FILE

	# send WPS-SESSION-START event to the WLAN events script
	( . $HOSTAPD_EVENTS_SCRIPT $radio_name WPS-SESSION-START $interface_name )
	# Set AP to configured mode.
	set_ap_configured $interface_name $wps_vendor_index $interface_index $radio_index

	# Check if web selection was to accept all incoming PIN connectios with correct PIN. In such case, no MAC and timeout are needed.
	if [ -z "$enrollee_mac"  ]
	then
		enrollee_uuid=any
		enrollee_mac=""
		wps_pin_timeout=""
	fi

	# If no uuid was received, need to read from pin_req file or generate new random
	if [ -z "$enrollee_uuid" ]
	then
		if [ -e $HOSTAPD_PIN_REQ ]
		then
			# Get the MAC from the file and compare with requesting MAC
			# Get the MACs and uuids from the file
			while read line_timestamp line_uuid line_mac param4
			do
				if [ "$enrollee_mac" = "$line_mac" ]
				then
					enrollee_uuid=$line_uuid
					break
				fi
			done < $HOSTAPD_PIN_REQ
		fi
		# If file list doesn't exist or MAC wasn't found in list, generate random uuid
		[ -z "$enrollee_uuid" ] && enrollee_uuid=`uuidgen`
	fi
	enrollee_pin=`echo $enrollee_pin | sed -e 's/-//g' -e 's/ //g'`
	/tmp/hostapd_cli_${radio_name} -i $radio_name wps_pin $interface_name $enrollee_uuid $enrollee_pin $wps_pin_timeout $enrollee_mac
	#/tmp/hostapd_cli_${radio_name} -i$interface_name wps_pin $enrollee_uuid $enrollee_pin $wps_pin_timeout $enrollee_mac
}

# Generate a new PIN number for the AP and return new value to the fapi_wlan_wave_out.conf
wps_generate_pin()
{
	local interface_name radio_name wps_vendor_index new_pin

	interface_name=$1
	radio_name=$2
	wps_vendor_index=$3

	new_pin=`/tmp/hostapd_cli_${radio_name} -i $radio_name wps_ap_pin $radio_name random`
	#new_pin=`/tmp/hostapd_cli_${radio_name} -i $interface_name wps_ap_pin random`

	# Write the new AP PIN to out.conf
	update_conf_out "PIN_${wps_vendor_index}" "$new_pin"
}

# Cancel currently running WPS session
cancel_wps()
{
	# Define local parameters
	local interface_name radio_name

	interface_name=$1
	radio_name=$2

	/tmp/hostapd_cli_${radio_name} -i $radio_name wps_cancel $interface_name
	#/tmp/hostapd_cli_${radio_name} -i$interface_name wps_cancel
}

# Test handler, take place only if test_only is set in wps_external_done()
create_test_file()
{
	hostapd_conf_file_file=$1
	
	echo "create_test_file: create test in file: $hostapd_conf_file_file" > /dev/console
	
	echo "############## wlan0 VAP parameters #############" > $hostapd_conf_file_file
	echo "vendor_elements=dd050009860100" >> $hostapd_conf_file_file
	echo "bssid=AC:9A:96:F4:85:10" >> $hostapd_conf_file_file
	echo "###___WPS_parameters___###" >> $hostapd_conf_file_file
	echo "wps_state=0" >> $hostapd_conf_file_file
	echo "############## wlan0.0 VAP parameters #############" >> $hostapd_conf_file_file
	echo "bss=wlan0.0" >> $hostapd_conf_file_file
	echo "vendor_elements=dd050009860100" >> $hostapd_conf_file_file
	echo "bssid=AC:9A:96:F4:85:12" >> $hostapd_conf_file_file
	echo "###___SSID_parameters___###" >> $hostapd_conf_file_file
	echo "bridge=br-lan" >> $hostapd_conf_file_file
	echo "# WPS configuration - START" >> $hostapd_conf_file_file
	echo "wps_state=2" >> $hostapd_conf_file_file
	echo "ssid=WLAN-TEST-160_Network" >> $hostapd_conf_file_file
	echo "wpa=2" >> $hostapd_conf_file_file
	echo "wpa_key_mgmt=WPA-PSK" >> $hostapd_conf_file_file
	echo "wpa_pairwise=CCMP" >> $hostapd_conf_file_file
	echo "wpa_passphrase=h3dz-gt48-voc5" >> $hostapd_conf_file_file
	echo "auth_algs=1" >> $hostapd_conf_file_file
	echo "# WPS configuration - END" >> $hostapd_conf_file_file
	echo "#WPS# ssid=2g_165.0" >> $hostapd_conf_file_file
	echo "###___AccessPoint_parameters___###" >> $hostapd_conf_file_file
	echo "ignore_broadcast_ssid=0" >> $hostapd_conf_file_file
	echo "############## wlan0.1 VAP parameters #############" >> $hostapd_conf_file_file
	echo "bss=wlan0.1" >> $hostapd_conf_file_file

	echo "create_test_file: dump $hostapd_conf_file_file:" > /dev/console
	cat  $hostapd_conf_file_file > /dev/console
}

# Returns interface name that was configured by external registrar.
# Read updated values in hostapd.conf after external registrar configured it.
# It parse hostapd.conf and save the VAP name that has its WPS active.
# These values are in a section starting with the comment 
#   "############## wlanX VAP parameters #############" (X=0, 0.0 etc.) and under it:
#   "WPS configuration - START" and ending with the comment "WPS configuration - END".
# Save last wlanX if found the WPS comments.
read_wps_vap_name_from_hostapd()
{
	# Define local parameters
	local interface_name radio_name section_start_text section_end_text section_start hostapd_get_name line enable_log
	
	# For logs set enable_log=1
	enable_log=0

	interface_name=$1
	radio_name=$2
	wps_state=$3
	hostapd_conf_file_file=$4
	[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: interface_name=$interface_name, radio_name=$radio_name, wps_state=$wps_state, hostapd_conf_file_file=$hostapd_conf_file_file" > /dev/console

	if [ -z "$hostapd_conf_file_file" ]
	then
		#hostapd_conf_file_file=${CONF_DIR}/hostapd_${radio_name}.conf
		hostapd_conf_file_file=${CONF_DIR}/hostapd_${interface_name}.conf
	fi
	
	[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: wps_state=$wps_state" > /dev/console

	# Options: unConfigured and Configured hostapd files
	# #___WPS_parameters___#
	# # WPS configuration - START
	if [ "$wps_state" = "wps_state=1" ]
	then
		[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: look for name of Unconfigured VAP" > /dev/console
		section_start_text="#___WPS_parameters___#"
	else
		[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: look for name of Configured VAP" > /dev/console
		section_start_text="# WPS configuration - START"
	fi

	section_start=""
	hostapd_get_name=${CONF_DIR}/hostapd_get_name_${interface_name}
	start_vap=" VAP parameters #"

	rm -f $hostapd_get_name
	
	while read line
	do
		[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: found section_start, line=$line" > /dev/console
		start_vap=`echo $line | grep ' VAP parameters #' -c`
		if [ "$start_vap" = "1" ]
		then
			[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: found VAP Start, get interface name" > /dev/console
			tmp1=${line################ }
			vap_name=`echo $tmp1 | awk '{print $1}'`
			[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: vap_name=$vap_name" > /dev/console
			# Overrun with last name
			echo "hostapd_vap_name=$vap_name" > $hostapd_get_name
		fi
		# If this section has WPS enable, return the VAP name
		if [ "$section_start" = "1" ]
		then
			[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: section_start=1, compare line ($line) to $wps_state" > /dev/console
			if [ "$line" = "wps_state=$wps_state" ]
			then
				[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: wps supported interface name already saved, can return" > /dev/console
				# Source new hostapd values file and delete it.
				[ -e "$hostapd_get_name" ] && . $hostapd_get_name
				return
			fi
		fi
		# If found starting comment, set the flag.
		if [ "$line" = "$section_start_text" ]
		then
			[ "$enable_log" = "1" ] && echo "read_wps_vap_name_from_hostapd: set section_start ($line)" > /dev/console
			section_start=1
		fi
	done < $hostapd_conf_file_file

	# Source new hostapd values file and delete it.
	[ -e "$hostapd_get_name" ] && . $hostapd_get_name
	# For debug savefile:
	[ -e "$hostapd_get_name" ] && cp $hostapd_get_name ${hostapd_get_name}_${interface_name}
}

# Returns external registrar configuration.
# Read updated values in hostapd.conf after external registrar configured it.
# The updated values changed by the external registrar are saved in the hostapd conf file.
# These values are in a section starting with the comment "WPS configuration - START" and ending with the comment "WPS configuration - END".
# Read the updated parameters in the new section to a file and source this file.
read_new_hostapd_values()
{
	# Define local parameters
	local interface_name radio_name section_start_text section_end_text section_start hostapd_new_values line enable_log
	
	# For logs set enable_log=1
	enable_log=0

	interface_name=$1
	radio_name=$2
	hostapd_conf_file_file=$3
	[ "$enable_log" = "1" ] && echo "read_new_hostapd_values: interface_name=$interface_name, radio_name=$radio_name, hostapd_conf_file_file=$hostapd_conf_file_file" > /dev/console

	if [ -z "$hostapd_conf_file_file" ]
	then
		hostapd_conf_file_file=${CONF_DIR}/hostapd_${radio_name}.conf
	fi
	section_start_text="# WPS configuration - START"
	section_end_text="# WPS configuration - END"
	section_start=""
	hostapd_new_values=${CONF_DIR}/hostapd_new_values_${interface_name}
	start_vap=" VAP parameters #"
	
	rm -f $hostapd_new_values
	
	# Go over the updated hostapd.conf file and find the needed parameters.
	while read line
	do
		start_vap=`echo $line | grep ' VAP parameters #' -c`
		if [ "$start_vap" = "1" ]
		then
			[ "$enable_log" = "1" ] && echo "read_new_hostapd_values: found VAP Start, get interface name" > /dev/console
			tmp1=${line################ }
			vap_name=`echo $tmp1 | awk '{print $1}'`
			[ "$enable_log" = "1" ] && echo "read_new_hostapd_values: vap_name=$vap_name" > /dev/console
			echo "hostapd_vap_name=$vap_name" >> $hostapd_new_values
		fi
		
		# If found ending comment, exit the loop.
		if [ "$line" = "$section_end_text" ]; then break; fi
		if [ "$section_start" = "1" ]
		then
			# Write the parameter to the temp file with the prefix "hostapd_"
			echo "hostapd_$line" >> $hostapd_new_values
		fi
		# If found starting comment, set the flag.
		if [ "$line" = "$section_start_text" ]
		then
			[ "$enable_log" = "1" ] && echo "read_new_hostapd_values: set section_start :-) !!!!!!!!!" > /dev/console
			section_start=1
		fi
	done < $hostapd_conf_file_file

	# Wrap hostapd_wpa_pairwise with parenthesis as there are spaces in value
	sed -i -e 's/hostapd_wpa_pairwise=/hostapd_wpa_pairwise=\"/' -e '/hostapd_wpa_pairwise=/s/$/\"/' /tmp/wlan_wave/hostapd_new_values_wlan0

	# Source new hostapd values file and delete it.
	[ -e "$hostapd_new_values" ] && . $hostapd_new_values
	[ -e "$hostapd_new_values" ] && cp $hostapd_new_values ${hostapd_new_values}_${interface_name}
}

# Save the settings after WPS external registrar session from the hostapd conf file to the database.
# Notify SL that an update of the DB after external registrar is needed using ubus call with the parameters to update
# For test set test_only=1
wps_external_done()
{
	local interface_name interface_index radio_name radio_index nid \
	param_index ubus_call db_security_mode wep_key_length wpa_type key_management enable_log
	############################################################################
	# TEST MODE:                                                               #
	# For test set test_only=1                                                 #
	# For logs set enable_log=1                                                 #
	############################################################################
	test_only=0
	enable_log=0

	interface_name=$1
	
	# Find the interface index and the radio index
	interface_index=`find_index_from_interface_name $interface_name`
	radio_name=${interface_name%%.*}
	radio_index=`find_index_from_interface_name $radio_name`

	echo "wps_external_done: interface_name=$interface_name, radio_name=$radio_name" > /dev/console

	[ "$enable_log" = "1" ] && echo "wps_external_done: interface_index=$interface_index, radio_name=$radio_name" > /dev/console
	print2log $radio_index DEBUG "$script_name: Start wps_external_done"

	# Check hostapd configuration file exists.
	[ ! -e ${CONF_DIR}/hostapd_${radio_name}.conf ] && print2log $radio_index ERROR "$script_name: Aborted. hostapd_${radio_name}.conf file doesn't exist" && exit 1

	# Find number of interfaces and config those support WPS
	#ifname_list=`ifconfig | grep wlan | awk -F " " '{print $1}'`
	#ifname_count=`ifconfig | grep wlan -c`
	ifname_list=
	db_radio_name_list=`cat /tmp/wlan_wave/fapi_wlan_wave_radio_conf | grep ^Name`
	for n in $db_radio_name_list
	do
		ifname_list_tmp=`echo ${n#*=\"}`
		ifname_list_tmp=`echo ${ifname_list_tmp%\"}`
		ifname_list_tmp=$(printf "%b" "$ifname_list_tmp")
		ifname_list="$ifname_list $ifname_list_tmp"
	done
	
	[ "$enable_log" = "1" ] && echo "wps_external_done: ifname_list=$ifname_list" > /dev/console

	# Find the radio that is configured
	radio_configured=""
	
	for interface_name_tmp in $ifname_list
	do
		[ "$test_only" = "0" ] && [ "$enable_log" = "1" ] && echo "wps_external_done: dump ${CONF_DIR}/hostapd_${interface_name_tmp}.conf" > /dev/console
		[ "$test_only" = "0" ] && [ "$enable_log" = "1" ] && cat ${CONF_DIR}/hostapd_${interface_name_tmp}.conf > /dev/console
		if [ "$test_only" = "0" ]
		then
			file=${CONF_DIR}/hostapd_${interface_name_tmp}.conf
		else
			file=/tmp/fapi_test_external_registrar
		fi
		tmp_name=`cat $file | grep wps_state=2 -c`
		# Save the configured radio
		if [ "$tmp_name" = "1" ]
		then
			radio_configured=$interface_name_tmp
		fi
	done
	[ "$enable_log" = "1" ] && echo "wps_external_done: radio_configured=$radio_configured" > /dev/console

	
	# Read the updated parameters from hostapd conf file
	if [ "$test_only" = "0" ]
	then
		#read_new_hostapd_values $interface_name $radio_name
		read_new_hostapd_values $radio_configured $radio_name
	else
		echo "wps_external_done: ############################################################################" > /dev/console
		echo "wps_external_done: # TEST MODE                                                                #" > /dev/console
		echo "wps_external_done: ############################################################################" > /dev/console
		create_test_file /tmp/fapi_test_external_registrar
		#read_new_hostapd_values $interface_name $radio_name /tmp/fapi_test_external_registrar
		read_new_hostapd_values $radio_configured $radio_name /tmp/fapi_test_external_registrar
	fi

	# Convert security hostapd values to DB values
	db_security_mode=""
	
	# If wpa is 0, security is wep or open.
	[ "$enable_log" = "1" ] && echo "wps_external_done: hostapd_wpa=$hostapd_wpa" > /dev/console
	if [ -z "$hostapd_wpa" ] || [ "$hostapd_wpa" = "0" ]
	then
		[ "$enable_log" = "1" ] && echo "wps_external_done: OPEN/WEP" > /dev/console
		db_security_mode="None"
		# Check if WEP by checking if wep_default_key was set.
		if [ "$hostapd_wep_default_key" ]
		then
			db_security_mode="WEP-64"
			# Check if key is 128
			wep_key_length=`echo $hostapd_wep_key | wc -L`
			if [ $wep_key_length = 13 ] || [ $wep_key_length = 26 ]
			then
				db_security_mode="WEP-128"
			fi
		fi
	else # Security is WPA 
		[ "$enable_log" = "1" ] && echo "wps_external_done: WPA" > /dev/console
		tkip_exist=`echo $hostapd_wpa_pairwise | grep TKIP -c`
		[ "$enable_log" = "1" ] && echo "wps_external_done: tkip_exist=$tkip_exist" > /dev/console
		if [ "$hostapd_wpa_pairwise" ] && [ "$hostapd_wpa_pairwise" = "CCMP" ]
		then
			wpa_type="WPA2"
		else
			wpa_type="WPA"
			# hostapd_rsn_pairwise - no such parameter. Add fix (05/04/17)
			[ "$hostapd_wpa_pairwise" ] && [ "$tkip_exist" = "1" ] && wpa_type="WPA-WPA2"
		fi
		# Check wpa_key_mgmt to see if in WPA-personal or WPA-enterparise security mode.
		key_management="Personal"
		[ "$hostapd_wpa_key_mgmt" ] && [ "$hostapd_wpa_key_mgmt" = "WPA-EAP" ] && key_management="Enterprise"
		db_security_mode="${wpa_type}-${key_management}"
		[ "$enable_log" = "1" ] && echo "wps_external_done: db_security_mode=$db_security_mode" > /dev/console
	fi
	
	# Create ubus command for SL to update DB with new values
	# Source ugw_notify_defs.sh
	. /etc/ugw_notify_defs.sh
	nid=$NOTIFY_WIFI_WPS_NEW_AP_SETTINGS
	param_index=1

	ubus_call=${CONF_DIR}/ubus_call.sh
	
	ssid=$hostapd_ssid
	index=0
	
	# Build radio list, configured radio first in list
	ifname_list_new=$radio_configured
	for interface_name_tmp in $ifname_list
	do
		if [ "$interface_name_tmp" != "$radio_configured" ]
		then
			ifname_list_new="$ifname_list_new $interface_name_tmp"
		fi
	done
	[ "$enable_log" = "1" ] && echo "wps_external_done: ifname_list_new=$ifname_list_new" > /dev/console
	
	# Loop over radios:
	#for interface_name_tmp in $ifname_list
	for interface_name_tmp in $ifname_list_new
	do
		if [ "$test_only" = "0" ]
		then
			file=${CONF_DIR}/hostapd_${interface_name_tmp}.conf
		else
			file=/tmp/fapi_test_external_registrar
		fi


		#wps_state=`cat ${CONF_DIR}/hostapd_${interface_name_tmp}.conf | grep wps_state | grep =2` # Configured
		wps_state=`cat $file | grep wps_state | grep =2` # Configured
		[ "$enable_log" = "1" ] && echo "wps_external_done: (1) wps_state=$wps_state in $file" > /dev/console
		if [ "$wps_state" != "wps_state=2" ]
		then
			[ "$enable_log" = "1" ] && echo "wps_external_done: READ AGAIN wps_state" > /dev/console
			#wps_state=`cat ${CONF_DIR}/hostapd_${interface_name_tmp}.conf | grep wps_state | grep =1` # Unconfigured
			wps_state=`cat $file | grep wps_state | grep =1` # Unconfigured
		fi
		[ "$enable_log" = "1" ] && echo "wps_external_done: (2) wps_state=$wps_state in $file" > /dev/console
		
		if [ "$wps_state" = "wps_state=2" ] || [ "$wps_state" = "wps_state=1" ]
		then
			[ "$enable_log" = "1" ] && echo "wps_external_done: $interface_name_tmp has WPS enabled" > /dev/console

			# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			# Open comment - to give different name for each interface:
			#if [ "$interface_name_tmp" != "radio_name" ]
			#then
			#	eval hostapd_ssid_${index}=${hostapd_ssid}_${index}
			#	ssid=${hostapd_ssid}_${index}
			#	[ "$enable_log" = "1" ] && echo "wps_external_done: ssid=$ssid" > /dev/console
			#fi

			ubus_command="ubus call servd notify '{\"nid\":$nid,\"type\":false,\"pn${param_index}\":\"ObjectName\",\"pv${param_index}\":\"$SSID_OBJECT\","
			param_index=$((param_index+1))
			ubus_command="${ubus_command}\"pn${param_index}\":\"SSID\",\"pv${param_index}\":\"$ssid\","
			param_index=$((param_index+1))
			
			# If the radio is of current VAP, send to VAP (hostapd_vap_name)
			if [ "$test_only" = "0" ]
			then
				#read_wps_vap_name_from_hostapd $interface_name $radio_name $wps_state
				read_wps_vap_name_from_hostapd $interface_name_tmp $radio_name $wps_state
			else
				############################################################################
				# TEST MODE                                                                #
				############################################################################
				#read_wps_vap_name_from_hostapd $interface_name $radio_name $wps_state /tmp/fapi_test_external_registrar
				read_wps_vap_name_from_hostapd $interface_name_tmp $radio_name $wps_state /tmp/fapi_test_external_registrar
			fi
			[ "$enable_log" = "1" ] && echo "wps_external_done: after read_wps_vap_name_from_hostapd: hostapd_vap_name=$hostapd_vap_name" > /dev/console
			
			ubus_command="${ubus_command}\"pn${param_index}\":\"Name\",\"pv${param_index}\":\"$hostapd_vap_name\","
			param_index=$((param_index+1))
			ubus_command="${ubus_command}\"pn${param_index}\":\"ObjectName\",\"pv${param_index}\":\"$ACCESSPOINT_SECURITY_OBJECT\","
			param_index=$((param_index+1))
			ubus_command="${ubus_command}\"pn${param_index}\":\"ModeEnabled\",\"pv${param_index}\":\"$db_security_mode\""
			param_index=$((param_index+1))
			# If security is WPA/WPA2/mixed, set paddphrase
			if [ ! -z "$hostapd_wpa_passphrase" ]
			then
				ubus_command="${ubus_command},\"pn${param_index}\":\"KeyPassphrase\",\"pv${param_index}\":\"$hostapd_wpa_passphrase\""
				param_index=$((param_index+1))
			fi
			# If security is WEP, set WEP key
			if [ ! -z "$hostapd_wep_key" ]
			then
				ubus_command="${ubus_command},\"pn${param_index}\":\"WEPKey\",\"pv${param_index}\":\"$hostapd_wep_key\""
				param_index=$((param_index+1))
			fi
			ubus_command="${ubus_command}}'"
		
			print2log $radio_index DEBUG_WPS "fapi_wlan_wave_wps.sh: build_wlan_notification: Name:$hostapd_vap_name Object:${RADIO_WPS_VENDOR_OBJECT} ConfigState:Configured"
			[ "$test_only" = "0" ] && build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:$hostapd_vap_name Object:${RADIO_WPS_VENDOR_OBJECT} ConfigState:Configured"
			print2log $radio_index DEBUG_WPS "fapi_wlan_wave_wps.sh: build_wlan_notification: Name:$interface_name_tmp Object:${RADIO_WPS_VENDOR_OBJECT} ConfigState:Configured"
			[ "$test_only" = "0" ] && build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:$interface_name_tmp Object:${RADIO_WPS_VENDOR_OBJECT} ConfigState:Configured"
			
						
			# Make the script calling the ubus executable and execute it
			echo "$ubus_command" > $ubus_call
			chmod +x $ubus_call
			[ "$enable_log" = "1" ] && echo "wps_external_done: cat $ubus_call:" > /dev/console
			[ "$enable_log" = "1" ] && cat $ubus_call > /dev/console
			# TEST:
			[ "$test_only" = "1" ] && exit
			# TEST END
			$ubus_call
			rm -f $ubus_call
			
			index=$((index+1))

		else
			[ "$enable_log" = "1" ] && echo "wps_external_done: $interface_name_tmp has WPS disabled" > /dev/console
		fi
	done
	
	# Remove TEST file if exist
	rm -rf /tmp/fapi_test_external_registrar
	
	print2log $radio_index DEBUG "fapi_wlan_wave_wps.sh: End wps_external_done"
}
LIB_WPS_SOURCED="1"
