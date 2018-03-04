#!/bin/sh
# This script updates the hotspot parameters in the configuration files.

script_name="wave_wlan_set_hotspot_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_HOTSPOT_SOURCED" ] && . /tmp/wave_wlan_lib_hotspot.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name comment
local hostapd_params
local hostapd_current_values
local driver_changed hostapd_changed
local hostapd_vap_conf_name
local hs20 disable_dgaf anqp_domain_id hs20_deauth_req_timeout hs20_oper_friendly_name hs20_conn_capab hs20_operating_class hs20_icon osu_ssid osu_server_uri osu_friendly_name osu_nai osu_method_list osu_icon osu_service_desc gas_comeback_delay manage_p2p allow_cross_connection tdls_prohibit interworking access_network_type internet venue_group venue_type hessid roaming_consortium venue_name network_auth_type ipaddr_type_availability domain_name anqp_3gpp_cell_net nai_realm
local l2_fw_ena wan_metrics_duration wan_port ap_isolate parap_cmd pcpeid oper_friendly_name_indexes conn_capab_indexes hs20_icon_indexes osu_providers_indexes roaming_consortium_indexes venue_name_indexes domain_name_indexes nai_realm_indexes

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
comment="___Hotspot_parameters___###"

hostapd_params="hs20
proxy_arp
disable_dgaf
anqp_domain_id
hs20_deauth_req_timeout
hs20_oper_friendly_name
hs20_conn_capab
hs20_operating_class
hs20_icon
osu_ssid
osu_server_uri
osu_friendly_name
osu_nai
osu_method_list
osu_icon
osu_service_desc
gas_comeback_delay
manage_p2p
allow_cross_connection
tdls_prohibit
interworking
access_network_type
internet
venue_group
venue_type
hessid
roaming_consortium
venue_name
network_auth_type
ipaddr_type_availability
domain_name
anqp_3gpp_cell_net
nai_realm"

# Assumption is that this script is called only if a change is needed in hotspot parameters in hostapd.
# Due to that, all the hotspot parameters will be removed from conf file and re-written.

# Read current driver value and initiate new values file.
driver_current_values=${TEMP_CONF_DIR}/driver_current_values_${interface_name}
driver_new_values=${TEMP_CONF_DIR}/driver_new_values_${interface_name}_${pid}

[ -e "$driver_current_values" ] && . $driver_current_values
touch $driver_new_values

# Calculate new values.
hs20=""
proxy_arp=""
disable_dgaf=""
anqp_domain_id=""
hs20_deauth_req_timeout=""
hs20_oper_friendly_name=""
hs20_conn_capab=""
hs20_operating_class=""
hs20_icon=""
osu_ssid=""
osu_server_uri=""
osu_friendly_name=""
osu_nai=""
osu_method_list=""
osu_icon=""
osu_service_desc=""
gas_comeback_delay=""
manage_p2p=""
allow_cross_connection=""
tdls_prohibit=""
interworking=""
access_network_type=""
internet=""
venue_group=""
venue_type=""
hessid=""
roaming_consortium=""
venue_name=""
network_auth_type=""
ipaddr_type_availability=""
domain_name=""
anqp_3gpp_cell_net=""
nai_realm=""

# Read hotspot status and other parameters
eval hs20=\${wlhs2_${ap_index}_hs20Mode}
eval proxy_arp=\${wlhs2_${ap_index}_proxyArp}
eval l2_fw_ena=\${wlhs2_${ap_index}_l2FwEna}
eval disable_dgaf=\${wlhs2_${ap_index}_dgafDisabled}
eval wan_metrics_duration=\${wlhs2_${ap_index}_wanMetricsDuration}
eval wan_port=\${wlhs2_${ap_index}_wanPort}
eval ap_isolate=\${wlmn_${ap_index}_apIsolationEna}

# Start/stop/configure other hotspot components
if [ $hs20 -gt $HS20_MODE_DISABLED ]
then
	parap_cmd="disable"

	[ "$proxy_arp" = "1" ] && parap_cmd="enable"
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $PARP_CTRL_SCRIPT" "$parap_cmd $interface_name)"
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $DGAF_DISABLE_SCRIPT" "$interface_name $disable_dgaf)"
fi

if [ "$hs20" = "$HS20_MODE_ENABLED" ]
then
	ap_isolate_cmd="disable"
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $WMDCTRL_SCRIPT" "set duration $wan_metrics_duration)"
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $WMDCTRL_SCRIPT" "set wlan name=$interface_name w_active=1 wans=$wan_port)"
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $WMDCTRL_SCRIPT" "enable $interface_name)"
	[ "$ap_isolate" = "1" ] && ap_isolate_cmd="enable"
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $HAIRPIN_CONFIG_SCRIPT" "$ap_isolate_cmd $interface_name)"
else
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $WMDCTRL_SCRIPT" "disable $interface_name)"
fi
if [ "$l2_fw_ena" = "1" ]
then
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $L2F_CTRL_SCRIPT" "enable $interface_name)"
else
	set_conf_param $WAVE_WLAN_RUNNNER alumnus otf $pid $interface_name "(. $L2F_CTRL_SCRIPT" "disable $interface_name)"
fi

# If hotspot is not enabled, don't set any other parameter
if [ $hs20 -gt $HS20_MODE_DISABLED ]
then
	pcpeid=$((ap_index+1))
	eval anqp_domain_id=\${wlhs2_${ap_index}_anqpDomainId}
	hs20_deauth_req_timeout=60
	# oper_friendly_name can have multiple values, read the indexes of values from rc.conf to update all values to hostapd conf.
	oper_friendly_name_indexes=`get_section_indexes $wlan_hs2_oper_name_Count wlhson $pcpeid`
	# hs20_conn_capab can have multiple values, read the indexes of values from rc.conf to update all values to hostapd conf.
	conn_capab_indexes=`get_section_indexes $wlan_hs2_connect_cap_Count wlhscc $pcpeid`
	eval hs20_operating_class=\${wlhs2_${ap_index}_operatingClass}
	# hs20_icon can have multiple values, read the indexes of values from rc.conf to update all values to hostapd conf.
	hs20_icon_indexes=`get_section_indexes $wlan_hs2_osu_icon_Count wlhsoi $pcpeid`
	eval osu_ssid=\${wlhs2_${ap_index}_osuSsid}
	osu_ssid=\""$osu_ssid"\"
	# There can be multiple osu provides and for each osu provider there is a set of parameters, read the indexes of values from rc.conf to update all values to hostapd conf.
	osu_providers_indexes=`get_section_indexes $wlan_hs2_osu_provider_Count wlhsop $pcpeid`
	eval gas_comeback_delay=\${wlhs2_${ap_index}_gasComebackDelay}

	### P2P parameters
	manage_p2p=1
	allow_cross_connection=0

	### TDLS parameters
	tdls_prohibit=1

	### Interworking parameters
	interworking=1
	eval access_network_type=\${wlhs2_${ap_index}_accessNetType}
	eval internet=\${wlhs2_${ap_index}_internetConnectivity}
	eval venue_group=\${wlhs2_${ap_index}_venueGroup}
	eval venue_type=\${wlhs2_${ap_index}_venueType}
	hessid=`convert_hessid $ap_index`
	# roaming_consortium can have multiple values, read the indexes of values from rc.conf to update all values to hostapd conf.
	roaming_consortium_indexes=`get_section_indexes $wlan_hs2_roam_consort_Count wlhsrc $pcpeid`
	# venue_name can have multiple values, read the indexes of values from rc.conf to update all values to hostapd conf.
	venue_name_indexes=`get_section_indexes $wlan_hs2_venue_Count wlhsvn $pcpeid`
	eval network_auth_type=\${wlhs2_${ap_index}_netAuthType}
	ipaddr_type_availability=`convert_ipaddr_type_availability $ap_index`
	eval anqp_3gpp_cell_net=\${wlhs2_${ap_index}_threeGpp}
	# nai_realm can have multiple values, read the indexes of values from rc.conf to update all values to hostapd conf.
	nai_realm_indexes=`get_section_indexes $wlan_hs2_nai_realm_Count wlhsnr $pcpeid`
else
	disable_dgaf=""
fi

# Remove hotspot parameters from hostapd_vap temp conf file and write all of them with updated values.
hostapd_vap_conf_name=${TEMP_CONF_DIR}/hostapd_vap_${interface_name}_${pid}.conf
# Remove the hostapd hotspot parameters.
remove_params_from_conf "###$comment $hostapd_params" $hostapd_vap_conf_name $HOSTAPD_VAP_CONF_PREFIX

# Write the hostapd hotspot parameters. Some parameters are written in the library functions.
set_conf_param hostapd_vap comment otf $pid $interface_name comment "$comment"
set_conf_param hostapd_vap regular no_otf $pid $interface_name hs20 "$hs20"
set_conf_param hostapd_vap regular no_otf $pid $interface_name proxy_arp "$proxy_arp"
set_conf_param hostapd_vap regular no_otf $pid $interface_name disable_dgaf "$disable_dgaf"
set_conf_param hostapd_vap regular no_otf $pid $interface_name anqp_domain_id "$anqp_domain_id"
set_conf_param hostapd_vap regular no_otf $pid $interface_name hs20_deauth_req_timeout "$hs20_deauth_req_timeout"
hs20_write_conf wlhson opFriendlyName hs20_oper_friendly_name "$oper_friendly_name_indexes" "regular" $interface_name "hostapd_vap" $pid
hs20_write_conf wlhscc connectionCap hs20_conn_capab "$conn_capab_indexes" "regular" $interface_name "hostapd_vap" $pid
set_conf_param hostapd_vap regular no_otf $pid $interface_name hs20_operating_class "$hs20_operating_class"
hs20_write_conf wlhsoi dummy dummy "$hs20_icon_indexes" "_wlhsoi" $interface_name "hostapd_vap" $pid
set_conf_param hostapd_vap regular no_otf $pid $interface_name osu_ssid "$osu_ssid"
hs20_write_conf wlhsop dummy dummy "$osu_providers_indexes" "_wlhsop" $interface_name "hostapd_vap" $pid
set_conf_param hostapd_vap regular no_otf $pid $interface_name gas_comeback_delay "$gas_comeback_delay"
set_conf_param hostapd_vap regular no_otf $pid $interface_name manage_p2p "$manage_p2p"
set_conf_param hostapd_vap regular no_otf $pid $interface_name allow_cross_connection "$allow_cross_connection"
set_conf_param hostapd_vap regular no_otf $pid $interface_name tdls_prohibit "$tdls_prohibit"
set_conf_param hostapd_vap regular no_otf $pid $interface_name interworking "$interworking"
set_conf_param hostapd_vap regular no_otf $pid $interface_name access_network_type "$access_network_type"
set_conf_param hostapd_vap regular no_otf $pid $interface_name internet "$internet"
set_conf_param hostapd_vap regular no_otf $pid $interface_name venue_group "$venue_group"
set_conf_param hostapd_vap regular no_otf $pid $interface_name venue_type "$venue_type"
set_conf_param hostapd_vap regular no_otf $pid $interface_name hessid "$hessid"
hs20_write_conf wlhsrc roamConsort roaming_consortium "$roaming_consortium_indexes" "_wlhsrc" $interface_name "hostapd_vap" $pid
hs20_write_conf wlhsvn venueName venue_name "$venue_name_indexes" "regular" $interface_name "hostapd_vap" $pid
set_conf_param hostapd_vap regular no_otf $pid $interface_name network_auth_type "$network_auth_type"
set_conf_param hostapd_vap regular no_otf $pid $interface_name ipaddr_type_availability "$ipaddr_type_availability"
hs20_write_conf wlhsdn domainName domain_name "1" "_wlhsdn" $interface_name "hostapd_vap" $pid
set_conf_param hostapd_vap regular no_otf $pid $interface_name anqp_3gpp_cell_net "$anqp_3gpp_cell_net"
hs20_write_conf wlhsnr naiRealmName nai_realm "$nai_realm_indexes" "_wlhsnr" $interface_name "hostapd_vap" $pid

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
