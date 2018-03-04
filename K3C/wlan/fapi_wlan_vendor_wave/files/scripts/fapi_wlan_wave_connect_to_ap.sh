#!/bin/sh
# Configure Profile parameters to connect EndPoint to AP or to disconnect from AP
# Needed objects: EndPoint.Profile EndPoint.Profile.Security

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh

interface_name=$1
interface_index=$2
pid=$3
radio_index=$4
connection_flag=$5

print2log $radio_index DEBUG "$script_name $*"
timestamp $radio_index "$script_name:begin"

# DEBUG: set debug_save_conf in fapi_wlan_wave_lib_common.sh to save the in conf file
[ "$debug_save_conf" ] && cp ${IN_CONF} ${CONF_DIR}/connect_to_ap_in_conf_${interface_name}

connection_command="$ENDPOINT_RECONNECT"
[ "$connection_flag" = "$ENDPOINT_DISCONNECT" ] && connection_command="$ENDPOINT_DISCONNECT"

# Map the objects indexes to the received objects in the in.conf file
profile_index=`map_param_index Object $ENDPOINT_PROFILE_OBJECT`
profile_security_index=`map_param_index Object $ENDPOINT_PROFILE_SECURITY_OBJECT`

# Save the input configuration parameters to a local DB used by Wave FAPI
[ -n "$profile_index" ] && save_db_params connect_to_ap $interface_name $profile_index $interface_index
[ -n "$profile_security_index" ] && save_db_params connect_to_ap $interface_name $profile_security_index $interface_index

# Source needed DB
local_db_source PROFILE

# Read the profile parameters
profile_enabled=`db2fapi_convert boolean Enable $interface_index`
profile_status=`db2fapi_convert regular Status $interface_index`
if [ "$profile_enabled" != "1" ] || [ "$profile_status" != "Active" -a "$profile_status" != "Disabled" ]
then
	print2log $radio_index DEBUG "$script_name profile is not Active and not Disabled"
	print2log $radio_index DEBUG "$script_name done"
	timestamp $radio_index "$script_name:done"
	exit 0
fi

# Since only 1 connection is supported, clear the supplicant_profile file of this EndPoint
cat /dev/null > ${CONF_DIR}/${SUPPLICANT_PROFILE_CONF_PREFIX}_${interface_name}_${pid}.conf

key_mgmt="NONE"
proto=""
pairwise=""
group=""
psk=""
wep_key0=""
wep_tx_keyidx=""
ssid=`db2fapi_convert hex SSID $interface_index`
bssid=`db2fapi_convert regular X_LANTIQ_COM_Vendor_BSSID $interface_index`
security_mode=`db2fapi_convert regular ModeEnabled $interface_index`

if [ "$security_mode" = "$DB_WPA_TKIP_PERSONAL" ] || [ "$security_mode" = "$DB_WPA2_CCMP_PERSONAL" ] || [ "$security_mode" = "$DB_WPA_MIXED_PERSONAL" ]
then
	key_mgmt=WPA-PSK
	proto=WPA2
	pairwise=CCMP
	[ "$security_mode" = "$DB_WPA_TKIP_PERSONAL" ] && proto="WPA" && pairwise="TKIP"
	[ "$security_mode" = "$DB_WPA_MIXED_PERSONAL" ] && proto="WPA WPA2" && pairwise="TKIP CCMP"
	group="$pairwise"
	psk=`db2fapi_convert regular KeyPassphrase $interface_index`
elif [ "$security_mode" = "$DB_WEP64" ] || [ "$security_mode" = "$DB_WEP128" ]
then
	wep_key0=`db2fapi_convert regular WEPKey $interface_index`
	wep_tx_keyidx=0
fi

scan_ssid=`db2fapi_convert boolean X_LANTIQ_COM_Vendor_IsHiddenSsid $interface_index`
[ "$scan_ssid" = "0" ] && scan_ssid=""

# Write the connection parameters to the profile conf file
# Currently: only 1 profile is supported

set_conf_param supplicant_profile regular otf $pid $interface_name text "network={"
set_conf_param supplicant_profile regular otf $pid $interface_name scan_ssid "$scan_ssid"
set_conf_param supplicant_profile regular otf $pid $interface_name ssid "$ssid"
set_conf_param supplicant_profile regular otf $pid $interface_name bssid "$bssid"
set_conf_param supplicant_profile regular otf $pid $interface_name proto "$proto"
set_conf_param supplicant_profile regular otf $pid $interface_name key_mgmt "$key_mgmt"
set_conf_param supplicant_profile regular otf $pid $interface_name pairwise "$pairwise"
set_conf_param supplicant_profile regular otf $pid $interface_name group "$group"
if [ -n "$psk" ]
then
	set_conf_param supplicant_profile regular otf $pid $interface_name psk "\"$psk\""
else
	set_conf_param supplicant_profile regular otf $pid $interface_name psk "$psk"
fi
set_conf_param supplicant_profile regular otf $pid $interface_name wep_key0 "$wep_key0"
set_conf_param supplicant_profile regular otf $pid $interface_name wep_tx_keyidx "$wep_tx_keyidx"
set_conf_param supplicant_profile regular otf $pid $interface_name text "}"

drv_config_post_up_conf_name=${CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf

remove_params_from_conf "reconnect disconnect" $drv_config_post_up_conf_name $DRIVER_POST_UP_CONF_PREFIX
set_conf_param drv_config_post_up wpa_cli otf $pid $interface_name "$connection_command" "$connection_command"

[ "$connection_flag" != "$ENDPOINT_DISCONNECT" ] && touch ${CONNECT_FLAG}_${interface_name}

print2log $radio_index DEBUG "$script_name done"
timestamp $radio_index "$script_name:done"
