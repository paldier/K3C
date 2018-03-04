#!/bin/sh
# Return the EndPoint.Profile objects from scan results
# Needed objects: None or EndPoint.Profile with partial list of parameters

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh
[ ! "$IN_CONF_SOURCED" ] && in_conf_source

interface_name=$1
endpoint_index=$2

interface_index=`find_index_from_interface_name $interface_name`
scanning_str="wpa_state=SCANNING"
# Find the radio index
radio_name=`get_radio_name_from_endpoint $interface_name`
radio_index=`find_index_from_interface_name $radio_name`

print2log $radio_index DEBUG "$script_name $*"
timestamp $radio_index "$script_name:begin"

# If interface name is not supplied, exit script
[ -z "$interface_name" ] && print2log $radio_index ALERT "$script_name: interface_name is missing. exit execution" && exit 1

# Verify interface is up and ready. If not, exit
[ `check_interface_is_ready $interface_name` = "1" ] && exit

endpoint_profile_index=1
endpoint_profile_security_index=2
next_object_index=3

# Security modes in scan results and the value to set in DB:
# Scan value   DB value
#
# No value                                       None
# [ESS]                                          None
# [WPS][ESS]                                     None
# [WPA2-PSK-CCMP][ESS]                           WPA2-Personal
# [WPA2-PSK-CCMP][WPS][ESS]                      WPA2-Personal
# [WPA2-PSK-CCMP][WPS-AUTH][ESS]                 WPA2-Personal
# [WPA-PSK-TKIP][ESS]                            WPA-Personal
# [WPA-PSK-CCMP+TKIP]                            WPA-Personal
# [WPA-PSK-CCMP+TKIP][WPS]                       WPA-Personal
# [WPA-PSK-TKIP][WPA2-PSK-CCMP][ESS]             WPA-WPA2-Personal
# [WPA-PSK-TKIP][WPA2-PSK-CCMP][WPS][ESS]        WPA-WPA2-Personal
# [WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP][ESS]   WPA-WPA2-Personal
# [WEP][ESS]                                     WEP-64
# [WPA2-EAP-CCMP][ESS]                           WPA2-Enterprise
# [WPA2-EAP-CCMP][WPS][ESS]                      WPA2-Enterprise
# [WPA-EAP-TKIP]                                 WPA-Enterprise
# [WPA-EAP-TKIP][WPA2-EAP-CCMP][ESS]             WPA-WPA2-Enterprise
# [WPA-EAP-CCMP+TKIP][WPA2-EAP-CCMP+TKIP][ESS]   WPA-WPA2-Enterprise
convert_ap_security()
{
	# Remove [ESS]
	ap_security=${sec%%\[ESS]}
	# Remove [WPS]
	ap_security=${ap_security%%\[WPS]}
	# Remove [WPS-AUTH]
	ap_security=${ap_security%%\[WPS-AUTH]}
	if [ -z "ssid" ]
	then
		ssid="$sec"
		ap_security="$DB_OPEN"
	else
		case "$ap_security" in
			"[WPA2-PSK-CCMP]"|"[WPA2-PSK-CCMP-preauth]")
				ap_security="$DB_WPA2_CCMP_PERSONAL"
				;;
			"[WPA-PSK-TKIP]"|"[WPA-PSK-CCMP+TKIP]")
				ap_security="$DB_WPA_TKIP_PERSONAL"
				;;
			"[WPA-PSK-TKIP][WPA2-PSK-CCMP]"|"[WPA-PSK-CCMP+TKIP][WPA2-PSK-CCMP+TKIP]")
				ap_security="$DB_WPA_MIXED_PERSONAL"
				;;
			"[WEP]")
				ap_security="$DB_WEP64"
				;;
			"[WPA2-EAP-CCMP]")
				ap_security="$DB_WPA2_CCMP_ENTERPRISE"
				;;
			"[WPA-EAP-TKIP]")
				ap_security="$DB_WPA_TKIP_ENTERPRISE"
				;;
			"[WPA-EAP-TKIP][WPA2-EAP-CCMP]"|"[WPA-EAP-CCMP+TKIP][WPA2-EAP-CCMP+TKIP]")
				ap_security="$DB_WPA_MIXED_ENTERPRISE"
				;;
			"")
				ap_security="$DB_OPEN"
				;;
			*)
				if [ "${ap_security/WPA2/}" != "$ap_security" ]
				then
					no_wpa2=${ap_security/WPA2/}
					if [ "${no_wpa2/WPA/}" != "$no_wpa2" ]
					then
						print2log $radio_index ENDPOINT "$script_name unknown security found in scan: $ap_security. saving AP as WPA2-CCMP WPA-TKIP"
						ap_security="$DB_WPA_MIXED_PERSONAL"
					else
						print2log $radio_index ENDPOINT "$script_name unknown security found in scan: $ap_security. saving AP as WPA2-CCMP"
						ap_security="$DB_WPA2_CCMP_PERSONAL"
					fi
				elif [ "${ap_security/WPA/}" != "$ap_security" ]
				then
					print2log $radio_index ENDPOINT "$script_name unknown security found in scan: $ap_security. saving AP as WPA-TKIP"
					ap_security="$DB_WPA_TKIP_PERSONAL"
				elif [ "${ap_security/WEP/}" != "$ap_security" ]
				then
					print2log $radio_index ENDPOINT "$script_name unknown security found in scan: $ap_security. saving AP as WEP64"
					ap_security="$DB_WEP64"
				else
					print2log $radio_index ERROR "$script_name unknown security found in scan: $ap_security. displaying AP as Open"
					ap_security="$DB_OPEN"
				fi
				;;
		esac
	fi
}

read_ssid()
{
	# SSID length is up to 32 chars, using only first 32 chars
	ssid=${ssid:0:32}
}

convert_frequency_to_channel()
{
	if [ ${freq#24} != $freq ]
	then
		# 2.4Ghz Channels
		channel=$((freq-2407))
		channel=$((channel/5))
		# Channel 14 is special
		[ $channel -gt 13 ] && channel=14
	else
		# 5GHz Channels
		channel=$((freq-5000))
		channel=$((channel/5))
	fi
}

perform_scan()
{
	# Read status until it isn't SCANNING
	wpa_cli -i${interface_name} scan
	t=0
	while [ `wpa_cli -i${interface_name} status | grep "$scanning_str" -c` -gt 0 ] && [ $t -lt 40 ]
	do
		sleep 1
		t=$((t+1))
	done
}
# Perform 3 scans to get more results and save results to file
for i in 1 2 3
do
	perform_scan
done
wpa_cli -i${interface_name} scan_res > ${CONF_DIR}/scan_res_${interface_name}

# Update the scan status to ScanDone
update_conf_out "Object_${endpoint_index}" "$ENDPOINT_OBJECT"
update_conf_out "X_LANTIQ_COM_Vendor_ScanStatus_${endpoint_index}" "$ENDPOINT_SCAN_DONE"

maxAliasId=0
[ -e $ALIAS_ID_FILE ] && . $ALIAS_ID_FILE
maxAliasId=$((maxAliasId+1))
# Go over the scan results line by line and generate the output conf
# Scan results example:
# bssid=00:e0:92:01:03:88 frequency=2412 signal_level=-36 flags=[WPA2-PSK-CCMP][WPS][ESS] ssid=test_ssid

i=1
while read bssid freq signal sec ssid
do
	# Remove prefix for each element of scan results (bssid=, frequency=, etc.)
	bssid=${bssid#bssid=}
	freq=${freq#frequency=}
	signal=${signal#signal_level=}
	sec=${sec#flags=}
	ssid=${ssid#ssid=}
	# Create the Profile objects
	update_conf_out "Object_${endpoint_profile_index}" "$ENDPOINT_PROFILE_OBJECT"
	update_conf_out "Enable_${endpoint_profile_index}" "true"
	update_conf_out "Status_${endpoint_profile_index}" "Available"
	update_conf_out "Alias_${endpoint_profile_index}" "CPE-Profile-${maxAliasId}"
	read_ssid
	update_conf_out "SSID_${endpoint_profile_index}" "$ssid"
	update_conf_out "Priority_${endpoint_profile_index}" "255"
	update_conf_out "X_LANTIQ_COM_Vendor_BSSID_${endpoint_profile_index}" "$bssid"

	#convert_frequency_to_channel
	#update_conf_out "X_LANTIQ_COM_Vendor_Channel_${endpoint_profile_index}" "$channel"
	update_conf_out "X_LANTIQ_COM_Vendor_SignalStrength_${endpoint_profile_index}" "$signal"
	# Create the Profile.Security object
	update_conf_out "Object_${endpoint_profile_security_index}" "$ENDPOINT_PROFILE_SECURITY_OBJECT"
	convert_ap_security
	update_conf_out "ModeEnabled_${endpoint_profile_security_index}" "$ap_security"

	i=$((i+1))
	endpoint_profile_index=$next_object_index && next_object_index=$((next_object_index+1))
	endpoint_profile_security_index=$next_object_index && next_object_index=$((next_object_index+1))
	maxAliasId=$((maxAliasId+1))
done < ${CONF_DIR}/scan_res_${interface_name}

# Remove the scan status parameter from the local DB
sed -i '/X_LANTIQ_COM_Vendor_ScanStatus_'$interface_index'/d' $ENDPOINT_CONF

rm -rf ${CONF_DIR}/scan_res_${interface_name}

print2log $radio_index DEBUG "$script_name done"
timestamp $radio_index "$script_name:$interface_name:done"
