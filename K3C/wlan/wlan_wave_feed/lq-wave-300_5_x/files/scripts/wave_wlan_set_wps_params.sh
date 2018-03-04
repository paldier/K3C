#!/bin/sh
# This script updates the WPS parameters in the configuration files.

script_name="wave_wlan_set_wps_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_WPS_SOURCED" ] && . /tmp/wave_wlan_lib_wps.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name comment
local hostapd_params
local hostapd_vap_current_values
local wps_state ap_setup_locked uuid wps_pin_requests device_name manufacturer model_name model_number serial_number device_type os_version config_methods ap_pin wps_cred_processing wps_rf_bands pbc_in_m1 upnp_iface friendly_name manufacturer_url model_description
local hostapd_changed
local hostapd_vap_conf_name
local cpeid

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
comment="___WPS_parameters___###"

hostapd_params="wps_state
ap_setup_locked
uuid
wps_pin_requests
device_name
manufacturer
model_name
model_number
serial_number
device_type
os_version
config_methods
ap_pin
wps_cred_processing
wps_rf_bands
pbc_in_m1
upnp_iface
friendly_name
manufacturer_url
model_description"

# Read current values and initiate new values file.
hostapd_vap_current_values=${TEMP_CONF_DIR}/hostapd_vap_current_values_${interface_name}_${pid}

read_current_values $HOSTAPD_VAP_CONF_PREFIX $interface_name $hostapd_vap_current_values

# in rc.conf, using cpeid for WPS
eval cpeid=\${wlmn_${ap_index}_cpeId}

wps_state=`convert_wps_state $cpeid`

ap_setup_locked=""
uuid=""
wps_pin_requests=""
device_name=""
manufacturer=""
model_name=""
model_number=""
serial_number=""
device_type=""
os_version=""
config_methods=""
ap_pin=""
wps_cred_processing=""
wps_rf_bands=""
pbc_in_m1=""
upnp_iface=""
friendly_name=""
manufacturer_url=""
model_description=""
# Configure WPS parameters only if WPS is enabled
if [ "$wps_state" -gt "0" ]
then
	ap_setup_locked=`convert_ap_setup_locked $cpeid`
	uuid=`read_uuid $cpeid`
	wps_pin_requests="$HOSTAPD_PIN_REQ"
	eval device_name=\${wlwps${cpeid}_0_deviceName}
	eval manufacturer=\${wlwps${cpeid}_0_manufacturer}
	eval model_name=\${wlwps${cpeid}_0_modelName}
	eval model_number=\${wlwps${cpeid}_0_modelNumber}
	eval serial_number=\${wlwps${cpeid}_0_serialNumber}
	device_type="6-0050F204-1"
	os_version="01020300"
	config_methods="label virtual_display push_button virtual_push_button physical_push_button keypad"
	wps_cred_processing=2
	wps_rf_bands=`convert_wps_rf_bands $ap_index`

	# External registrar parameters
	if [ "$ap_setup_locked" = "0" ]
	then
		eval ap_pin=\${wlwps${cpeid}_0_PIN}
		pbc_in_m1=1
		upnp_iface=br0
		eval friendly_name=\${wlwps${cpeid}_0_friendlyName}
		eval manufacturer_url=\${wlwps${cpeid}_0_manufacturerUrl}
		eval model_description=\${wlwps${cpeid}_0_modelDescription}
	fi
fi

# Check if a hostapd parameter was changed.
hostapd_changed=`check_param_changed "$hostapd_params"`

# If a hostapd vap parameter was changed, remove wps parameters from hostapd_vap temp conf file and write all of them with updated values.
if [ "$hostapd_changed" ]
then
	hostapd_vap_conf_name=${TEMP_CONF_DIR}/hostapd_vap_${interface_name}_${pid}.conf
	remove_params_from_conf "###$comment $hostapd_params" $hostapd_vap_conf_name $HOSTAPD_VAP_CONF_PREFIX

	# Write the hostapd vap wps parameters.
	set_conf_param hostapd_vap comment otf $pid $interface_name comment "$comment"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wps_state "$wps_state"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ap_setup_locked "$ap_setup_locked"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name uuid "$uuid"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wps_pin_requests "$wps_pin_requests"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name device_name "$device_name"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name manufacturer "$manufacturer"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name model_name "$model_name"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name model_number "$model_number"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name serial_number "$serial_number"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name device_type "$device_type"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name os_version "$os_version"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name config_methods "$config_methods"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ap_pin "$ap_pin"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wps_cred_processing "$wps_cred_processing"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wps_rf_bands "$wps_rf_bands"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name pbc_in_m1 "$pbc_in_m1"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name upnp_iface "$upnp_iface"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name friendly_name "$friendly_name"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name manufacturer_url "$manufacturer_url"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name model_description "$model_description"
fi

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
