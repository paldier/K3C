#!/bin/sh
# This script updates the main parameters in the configuration files.

script_name="wave_wlan_set_main_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name comment
local driver_params hostapd_params
local driver_current_values hostapd_current_values
local driver_changed hostapd_changed
local drv_config_post_up_conf_name hostapd_vap_conf_name
local sAPforwarding sReliableMcast sIpxPpaEnabled sEnableAMSDU
local bridge ssid ignore_broadcast_ssid ap_isolate dtim_period ap_max_inactivity max_num_sta num_res_sta opmode_notif qos_map_set \
wave500_found field1 class_txt class id vendor_id pci_device fastpath_enabled hairpin_mode

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
comment="___Main_parameters___###"

# Define list of main parameters
driver_params="sAPforwarding
sReliableMcast
sIpxPpaEnabled
sEnableAMSDU"

hostapd_params="bridge
ssid
ignore_broadcast_ssid
ap_isolate
dtim_period
ap_max_inactivity
max_num_sta
num_res_sta
opmode_notif
qos_map_set"

# Read current values and initiate new values file.
driver_current_values=${TEMP_CONF_DIR}/driver_current_values_${interface_name}
hostapd_current_values=${TEMP_CONF_DIR}/hostapd_current_values_${interface_name}_${pid}

read_current_values $HOSTAPD_VAP_CONF_PREFIX $interface_name $hostapd_current_values
read_current_values $DRIVER_POST_UP_CONF_PREFIX $interface_name $driver_current_values

# Calculate new values.

### Multiple BSSID parameters
bridge="br0"

### 802.11 parameters
eval ssid=\${wlmn_${ap_index}_ssid}
eval ignore_broadcast_ssid=\${wlmn_${ap_index}_ssidMode}
eval ap_isolate=\${wlmn_${ap_index}_apIsolationEna}
# When ap_isolate is enabled, sAPforwarding is disabled and vice versa
sAPforwarding=$((ap_isolate^1))
dtim_period=`convert_dtim_period $ap_index`

### Station inactivity parameters
eval ap_max_inactivity=\${wlmn_${ap_index}_apMaxInactivity}

### VAP limits
eval max_num_sta=\${wlmn_${ap_index}_maxSta}
eval num_res_sta=\${wlmnwave_${ap_index}_minSta}

### 11ac parameters
opmode_notif=`convert_opmode_notif $ap_index`
### Multicast parameters
eval sReliableMcast=\${wlmnwave_${ap_index}_mctoUcEna}

### PPA parameters
# Configure PPA only if PPA exists by checking if /sbin/ppacmd exists
sIpxPpaEnabled=""
# sIpxPpaEnabled is set on interfaces that are not fastpath enabled
fastpath_enabled=`get_fastpath_enabled $interface_name`
[ "$fastpath_enabled" != "1" ] && [ -e "/sbin/ppacmd" ] && eval sIpxPpaEnabled=\${wlmnwave_${ap_index}_ppaEna}

### AMSDU
eval sEnableAMSDU=\${wlmnwave_${ap_index}_amsduEna}

### QOS map parameter
qos_map_set=""
eval qos_map_set=\${wlhs2_${ap_index}_qosMap}

# Write the parameters to the configuration files.
# Check if a hostapd parameter was changed.
hostapd_changed=`check_param_changed "$hostapd_params"`

# If a hostapd parameter was changed, remove main parameters from hostapd_vap temp conf file and write all of them with updated values.
if [ "$hostapd_changed" ]
then
	hostapd_vap_conf_name=${TEMP_CONF_DIR}/hostapd_vap_${interface_name}_${pid}.conf
	# Remove the hostapd main parameters.
	remove_params_from_conf "###$comment $hostapd_params" $hostapd_vap_conf_name $HOSTAPD_VAP_CONF_PREFIX

	# Write the hostapd main parameters.
	set_conf_param hostapd_vap comment otf $pid $interface_name comment "$comment"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name bridge "$bridge"
	ssid=$(printf "%b" "$ssid") && set_conf_param hostapd_vap regular no_otf $pid $interface_name ssid "$ssid"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ignore_broadcast_ssid "$ignore_broadcast_ssid"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ap_isolate "$ap_isolate"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name dtim_period "$dtim_period"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ap_max_inactivity "$ap_max_inactivity"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name max_num_sta "$max_num_sta"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name num_res_sta "$num_res_sta"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name opmode_notif "$opmode_notif"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name qos_map_set "$qos_map_set"
fi

# Check if a driver parameter was changed.
driver_changed=`check_param_changed "$driver_params"`

# If a driver parameter was changed, remove main parameters from drv_config_post_up temp conf file and write all of them with updated values.
if [ "$driver_changed" ]
then
	drv_config_post_up_conf_name=${TEMP_CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf
	# Remove the driver main parameters.
	remove_params_from_conf "$driver_params" $drv_config_post_up_conf_name $DRIVER_POST_UP_CONF_PREFIX

	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sAPforwarding "$sAPforwarding"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sReliableMcast "$sReliableMcast"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sIpxPpaEnabled "$sIpxPpaEnabled"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sEnableAMSDU "$sEnableAMSDU"
fi

# If PPA exists, check if the PPA is enabled for this interface and if fastpath is enabled for the radio of this interface.
# If ppa enabled or fastpath is enabled for the radio, write the PPA commands. If both disabled, remove the interface from the PPA.
if [ -e /sbin/ppacmd ]
then
	if [ "$sIpxPpaEnabled" = "1" ] || [ "$fastpath_enabled" = "1" ]
	then
		set_conf_param drv_config_post_up ppa otf $pid $interface_name ppacmd "add"
	else
		set_conf_param drv_config_post_up ppa otf $pid $interface_name ppacmd "remove"
	fi
fi

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
