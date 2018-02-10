#!/bin/sh
# This script updates the WDS parameters in the configuration files.

script_name="wave_wlan_set_wds_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}

# Define list of wds parameters
driver_params="sBridgeMode
sPeerAPkeyIdx
key"

# Read current values and initiate new values file.
driver_current_values=${TEMP_CONF_DIR}/driver_current_values_${interface_name}

read_current_values $DRIVER_POST_UP_CONF_PREFIX $interface_name $driver_current_values

# Calculate new values.
sPeerAPkeyIdx=""
driver_peer_aps=""
rc_conf_peer_aps=""
eval sBridgeMode=\${wlwds_${ap_index}_enable}
# Configure WDS parameters only if WDS is enabled
if [ "$sBridgeMode" = "1" ]
then
	sPeerAPkeyIdx=`convert_peer_ap_key_index $ap_index`
	# Configure the wep keys if wep is set
	[ $sPeerAPkeyIdx -gt 0 ] && key=`convert_wds_wep_keys $ap_index`
	# Get the list of peer APs in driver and rc.conf
	driver_peer_aps=`get_wds_driver_peer_list $ap_index $interface_name`
	rc_conf_peer_aps=`get_wds_rc_conf_peer_list $ap_index`
fi

# Write the parameters to the configuration files.
# Check if a driver parameter was changed.
driver_changed=`check_param_changed "$driver_params"`
# Check if peer APs list was changed.
peers_changed=`check_wds_peer_list_changed $ap_index "$driver_peer_aps" "$rc_conf_peer_aps"`

# If a driver parameter was changed, or the peer APs list was changed, remove wds parameters from drv_config_post_up temp conf file and write all of them with updated values.
if [ "$driver_changed" -o "$peers_changed" ]
then
	drv_config_post_up_conf_name=${TEMP_CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf
	# Remove the wds driver parameters.
	remove_params_from_conf "sAddPeerAP sDelPeerAP $driver_params" $drv_config_post_up_conf_name $DRIVER_POST_UP_CONF_PREFIX

	# Write the driver wds parameters.
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sBridgeMode "$sBridgeMode"
	set_conf_param drv_config_post_up iwconfig otf $pid $interface_name key "$key"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sPeerAPkeyIdx "$sPeerAPkeyIdx"
	# Update the AP peers list
	update_wds_peer_ap_list $ap_index $pid $interface_name "$driver_peer_aps" "$rc_conf_peer_aps"
fi

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
