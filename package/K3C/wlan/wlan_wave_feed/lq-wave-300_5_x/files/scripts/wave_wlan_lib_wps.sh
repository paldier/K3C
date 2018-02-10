#!/bin/sh

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh

export HOSTAPD_PIN_REQ=/var/run/hostapd.pin-req
export WPS_PIN_TEMP_FILE=/tmp/wps_pin_params

# Read uuid from rc.conf. If no value is set, generate new uuid and save it to rc.conf
# Return the uuid.
read_uuid()
{
	# Define local parameters
	local cpeid uuid

	cpeid=$1

	eval uuid=\${wlwps${cpeid}_0_uuid}
	if [ -z "$uuid" ]
	then
		uuid=`uuidgen`
		status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_wps" "wlwps${cpeid}_0_uuid" "$uuid" > /dev/null
	fi
	echo "$uuid"
}

# Set AP to be WPS configured.
wps_set_ap_configured()
{
	# Define local parameters
	local ap_index
	local cpeid wps_config_state

	ap_index=$1
	eval cpeid=\${wlmn_${ap_index}_cpeId}

	# Read current WPS AP state (configured/un-configured)
	eval wps_config_state=\${wlwps${cpeid}_0_cfgState}
	# If AP is un-configured, set it to configured in the rc.conf and call script to reconfigure wps parameters.
	if [ "$wps_config_state" = "1" ]
	then
		status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_wps" "wlwps${cpeid}_0_cfgState" "2" > /dev/null
		$ETC_PATH/backup
		(. $ETC_PATH/wave_wlan_config_execute.sh wps_modify $ap_index)
	fi
}

# Start a WPS PIN session for the AP
wps_connect_via_pin()
{
	# Define local parameters
	local ap_index interface_name
	local enrollee_pin enrollee_mac enrollee_uuid wps_pin_timeout

	ap_index=$1
	interface_name=$2
	enrollee_pin=$3
	enrollee_mac=$4
	enrollee_uuid=$5

	# Using hard-coded timeout of 30 minutes
	wps_pin_timeout=1800

	# send WPS-SESSION-START event to the WLAN events script
	( . $HOSTAPD_EVENTS_SCRIPT $interface_name WPS-SESSION-START )
	# Set AP to configured mode.
	wps_set_ap_configured $ap_index

	# Check if web selection was to accept all incoming PIN connectios with correct PIN. In such case, no MAC and timeout are needed.
	if [ -z "$enrollee_mac" -o "$enrollee_mac" = "FF:FF:FF:FF:FF:FF" ]
	then
		enrollee_uuid=any
		enrollee_mac=""
		wps_pin_timeout=""
	fi

	# If no uuid was received, need to read from pin_req file or generate new random
	if [ -z "$enrollee_uuid" ]
	then
		# Get the MAC from the file and compare with requesting MAC
		# Get the MACs and uuids from the file
		while read line_timestamp line_uuid line_mac param4 param5 param6 param7 param8 param9
		do
			if [ "$enrollee_mac" = "$line_mac" ]
			then
				enrollee_uuid=$line_uuid
				break
			fi
		done < $HOSTAPD_PIN_REQ
		# If MAC wasn't found in list, generate random uuid
		[ -z "$enrollee_uuid" ] && enrollee_uuid=`uuidgen`
	fi
	$BINDIR/hostapd_cli -i$interface_name wps_pin $enrollee_uuid $enrollee_pin $wps_pin_timeout $enrollee_mac
}

# Read updated values in hostapd.conf after external registrar configured it.
# The updated values changed by the external registrar are saved in the hostapd conf file.
# These values are in a section starting with the comment "WPS configuration - START" and ending with the comment "WPS configuration - END".
# Read the updated parameters in the new section to a file and source this file.
read_new_hostapd_values()
{
	# Define local parameters
	local interface_name
	local section_start_text section_end_text section_start hostapd_new_values

	interface_name=$1
	section_start_text="# WPS configuration - START"
	section_end_text="# WPS configuration - END"
	section_start=""
	hostapd_new_values=${TEMP_CONF_DIR}/hostapd_new_values_${interface_name}

	# Go over the updated hostapd.conf file and find the needed parameters.
	while read line
	do
		# If found ending comment, exit the loop.
		[ "$line" = "$section_end_text" ] && break
		if [ "$section_start" ]
		then
			# Write the parameter to the temp file with the prefix "hostapd_"
			echo "hostapd_$line" >> $hostapd_new_values
		fi
		# If found starting comment, set the flag.
		[ "$line" = "$section_start_text" ] && section_start=1
	done < ${CONF_DIR}/hostapd_${interface_name}.conf

	# Source new hostapd values file and delete it.
	. $hostapd_new_values
	rm $hostapd_new_values
}

# Get values from hostapd and configure rc.conf.
# Read parameters:
# wps_state
# ssid
# wpa
# auth_algs
# wpa_key_mgmt
# wpa_pairwise
# rsn_pairwise (if needed)
# psk/passphrase (if needed)
######
# Write parameters:
# ssid
# beaconType
# authType
# encrType
# psk/passphrase
# pskFlag
write_parameters_from_hostapd_to_ugw()
{
	# Define local parameters
	local ap_index interface_name
	local sec_cpeid hostapd_ssid rc_conf_beacon_type rc_conf_auth_type rc_conf_encr_type rc_conf_psk_flag hostapd_wpa_passphrase

	ap_index=$1

	# Get the interface name
	eval interface_name=\${wlmnwave_${ap_index}_interfaceName}

	# Read the updated parameters
	read_new_hostapd_values $interface_name

	# in rc.conf, using cpeid for wlpsk.
	eval sec_cpeid=\${wlmn_${ap_index}_cpeId}

	# Set ssid
	hostapd_ssid=`ascii2hex $hostapd_ssid`
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_main" "wlmn_${ap_index}_ssid" "$hostapd_ssid" > /dev/null

	# Set security
	# beaconType is same as wpa, only difference is when using wpa mixed-mode, need to decide which type (mixed restricted or not restricted).
	rc_conf_beacon_type=$hostapd_wpa
	if [ "$hostapd_wpa" = "$HOSTAPD_WPA_MIXED" ]
	then
		if [ "$hostapd_rsn_pairwise" = "TKIP" ]
		then
			rc_conf_beacon_type=$BEACON_WPA_WPA2_COMPLIANT
		else
			rc_conf_beacon_type=$BEACON_WPA_WPA2_NOT_COMPLIANT
		fi
	fi
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_security" "wlsec_${ap_index}_beaconType" "$rc_conf_beacon_type" > /dev/null

	# Decide between radius and personal.
	rc_conf_auth_type=0
	if [ "$hostapd_wpa_key_mgmt" = "WPA-PSK" ]
	then
		rc_conf_auth_type=$AUTH_PSK
	elif [ "$hostapd_wpa_key_mgmt" = "WPA-EAP" ]
	then
		rc_conf_auth_type=$AUTH_RADIUS
	fi
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_security" "wlsec_${ap_index}_authType" "$rc_conf_auth_type" > /dev/null

	# Set the encryption type
	case $hostapd_wpa_pairwise in
	"TKIP")
		rc_conf_encr_type=$ENCR_TKIP
		;;
	"CCMP")
		rc_conf_encr_type=$ENCR_CCMP
		[ "$hostapd_rsn_pairwise" = "TKIP" ] && rc_conf_encr_type=$ENCR_TKIP_CCMP
		;;
	"TKIP CCMP")
		rc_conf_encr_type=$ENCR_TKIP_CCMP
		;;
	"CCMP TKIP")
		rc_conf_encr_type=$ENCR_TKIP_CCMP
		;;
	esac
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_security" "wlsec_${ap_index}_encrType" "$rc_conf_encr_type" > /dev/null

	# Set passphrase/psk if needed
	if [ "$hostapd_wpa_key_mgmt" = "WPA-PSK" ]
	then
		rc_conf_psk_flag=0
		# If no passphrase was found, read psk value and set psk flag.
		[ -z "$hostapd_wpa_passphrase" ] && hostapd_wpa_passphrase=$hostapd_wpa_psk && rc_conf_psk_flag=1
		# If using passphrase, convert to hex
		[ "$rc_conf_psk_flag" = "0" ] && hostapd_wpa_passphrase=`ascii2hex $hostapd_wpa_passphrase`

		status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_psk" "wlpsk${sec_cpeid}_0_pskFlag" "$rc_conf_psk_flag" > /dev/null
		status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_psk" "wlpsk${sec_cpeid}_0_passPhrase" "$hostapd_wpa_passphrase" > /dev/null
	fi
	# Set the AP to configured state.
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_wps" "wlwps${sec_cpeid}_0_cfgState" "2" > /dev/null

	# Save changes to rc.conf.
	$ETC_PATH/backup
}

LIB_WPS_SOURCED="1"
