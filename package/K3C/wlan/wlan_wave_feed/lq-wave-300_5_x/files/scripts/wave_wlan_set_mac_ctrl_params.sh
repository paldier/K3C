#!/bin/sh

script_name="wave_wlan_set_mac_ctrl_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name comment
local hostapd_params hostapd_vap_current_values
local hostapd_changed hostapd_vap_conf_name
local macaddr_acl accept_mac_file deny_mac_file
local ugw_acl

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
comment="___ACL_parameters___###"

hostapd_params="macaddr_acl
accept_mac_file
deny_mac_file"

# Read current values and initiate new values file.
hostapd_vap_current_values=${TEMP_CONF_DIR}/hostapd_vap_current_values_${interface_name}_${pid}

read_current_values $HOSTAPD_VAP_CONF_PREFIX $interface_name $hostapd_vap_current_values

### ACL parameters
# Read rc.conf ACL type value
eval ugw_acl=\${wlsec_${ap_index}_macAddrCntrlType}
macaddr_acl=`convert_macaddr_acl $ap_index $ugw_acl`

accept_mac_file="${TEMP_CONF_DIR}/${ACCEPT_ACL_FILE}.${interface_name}.conf"
deny_mac_file="${TEMP_CONF_DIR}/${DENY_ACL_FILE}.${interface_name}.conf"
# If ACL is disabled, don't set ACL files parameters
if [ "$ugw_acl" = "$ACL_DISABLED" ]
then
	accept_mac_file=""
	deny_mac_file=""
fi

# Check if hostapd parameter was changed.
hostapd_changed=`check_param_changed "$hostapd_params"`

# If a hostapd vap parameter was changed, remove mac_ctrl parameters from hostapd_vap temp conf file and write all of them with updated values.
if [ "$hostapd_changed" ]
then
	hostapd_vap_conf_name=${TEMP_CONF_DIR}/hostapd_vap_${interface_name}_${pid}.conf
	remove_params_from_conf "###$comment $hostapd_params" $hostapd_vap_conf_name $HOSTAPD_VAP_CONF_PREFIX

	# Write the hostapd vap mac_ctrl parameters.
	set_conf_param hostapd_vap comment otf $pid $interface_name comment "$comment"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name macaddr_acl "$macaddr_acl"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name accept_mac_file "$accept_mac_file"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name deny_mac_file "$deny_mac_file"
	[ -n "$accept_mac_file" ] && touch $accept_mac_file
	[ -n "$deny_mac_file" ] && touch $deny_mac_file
fi

# Write the MAC addresses to the deny/accept lists and clear the other list.
case $ugw_acl in
$ACL_ACCEPT)
	update_acl_list ACCEPT $ap_index $pid
	[ -e "$deny_mac_file" ] && cat /dev/null > $deny_mac_file
;;
$ACL_DENY)
	update_acl_list DENY $ap_index $pid
	[ -e "$accept_mac_file" ] && cat /dev/null > $accept_mac_file
;;
$ACL_DISABLED)
	update_acl_list DENY $ap_index $pid empty
	[ -e "$accept_mac_file" ] && cat /dev/null > $accept_mac_file
	[ -e "$deny_mac_file" ] && cat /dev/null > $deny_mac_file
;;
esac

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
