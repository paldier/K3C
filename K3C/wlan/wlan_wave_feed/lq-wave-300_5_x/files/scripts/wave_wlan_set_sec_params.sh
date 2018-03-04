#!/bin/sh
# This script updates the security parameters in the configuration files.

script_name="wave_wlan_set_sec_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name comment
local hostapd_params
local hostapd_current_values
local hostapd_changed
local hostapd_vap_conf_name
local auth_type beacon_type encr_type pmf_enabled pmf_required
local auth_algs eapol_key_index_workaround wpa eap_server wep_default_key wep_key0 wep_key1 wep_key2 wep_key3 wpa_key_mgmt wpa_passphrase wpa_psk wpa_group_rekey wpa_gmk_rekey wpa_pairwise rsn_pairwise osen disable_dgaf ieee8021x auth_server_addr auth_server_port auth_server_shared_secret acct_server_addr acct_server_port acct_server_shared_secret eap_reauth_period ieee80211w assoc_sa_query_max_timeout assoc_sa_query_retry_timeout
local cpeid

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
comment="___Security_parameters___###"

# Define list of sec parameters
hostapd_params="auth_algs
eapol_key_index_workaround
wpa
eap_server
wep_default_key
wep_key0
wep_key1
wep_key2
wep_key3
wpa_key_mgmt
wpa_passphrase
wpa_psk
wpa_group_rekey
wpa_gmk_rekey
wpa_pairwise
rsn_pairwise
osen
disable_dgaf
ieee8021x
auth_server_addr
auth_server_port
auth_server_shared_secret
acct_server_addr
acct_server_port
acct_server_shared_secret
eap_reauth_period
ieee80211w
assoc_sa_query_max_timeout
assoc_sa_query_retry_timeout"

# Read current values and initiate new values file.
hostapd_current_values=${TEMP_CONF_DIR}/hostapd_current_values_${interface_name}_${pid}

read_current_values $HOSTAPD_VAP_CONF_PREFIX $interface_name $hostapd_current_values

# Calculate new values.
# in rc.conf, using cpeid for wlpsk.
eval cpeid=\${wlmn_${ap_index}_cpeId}

# Read the rc.conf parameters that configure sec:
### authType:
# 0 = open
# 1 = shared
# 2 = radius
# 3 = psk
### encrType
# 0 = None
# 1 = wep
# 2 = tkip
# 3 = ccmp
# 4 = tkip-ccmp
### beaconType
# 0 = basic
# 1 = wpa
# 2 = wpa2
# 3 = wpa-wpa2 (not compliant)
# 4 = wpa/wpa2 (compliant)
# TODO: the values for compliant and non-compliant will be switched. change once switch is done.
eval auth_type=\${wlsec_${ap_index}_authType}
eval beacon_type=\${wlsec_${ap_index}_beaconType}
eval encr_type=\${wlsec_${ap_index}_encrType}
eval pmf_enabled=\${wlsec_${ap_index}_pmfEna}
eval pmf_required=\${wlsec_${ap_index}_pmfRequired}
# Convert the rc.conf values to the hostapd parameters that configure sec:
### wpa
# 0 = Open/WEP
# 1 = wpa
# 2 = wpa2
# 3 = wpa-wpa2
### wpa_key_mgmt
# WPA-PSK
# WPA-EAP
# WPA-PSK-SHA256
# WPA-EAP-SHA256
### auth_algs
# 1 = open
# 2 = shared
# 3 = both
### eap_server
# 0 = external authenticator (RADIUS).
# 1 = internal authenticator

wpa=`convert_wpa $beacon_type`
wpa_key_mgmt=""
auth_algs=`convert_auth_algs $auth_type`
eap_server=`convert_eap_server $auth_type`
eapol_key_index_workaround=0

### WEP parameters
wep_default_key=""
wep_key0=""
wep_key1=""
wep_key2=""
wep_key3=""
eval wep_key_type=\${wlsec_${ap_index}_wepKeyType}
# Convert the WEP keys only if security is WEP.
if [ "$encr_type" = "$ENCR_WEP" ]
then
	eval wep_default_key=\${wlsec_${ap_index}_wepKeyIndx}
	# Read the WEP keys values
	for wep_key_name in wep_key0 wep_key1 wep_key2 wep_key3
	do
		eval $wep_key_name=`convert_wep_key $ap_index ${wep_key_name##wep_key} $cpeid`
	done
fi

### WPA and WPA2 parameters
wpa_passphrase=""
wpa_psk=""
group_key_interval=""
wpa_group_rekey=""
wpa_gmk_rekey=""
wpa_pairwise=""
rsn_pairwise=""
if [ $beacon_type -gt 0 ] && [ $beacon_type -lt $BEACON_OSEN ]
then
	wpa_key_mgmt=`convert_wpa_key_mgmt $auth_type $beacon_type $encr_type $pmf_enabled $pmf_required`

	# For personal authentication, check which password type is used: passphrase or psk
	if [ "$auth_type" != "$AUTH_RADIUS" ]
	then
		eval psk_flag=\${wlpsk${cpeid}_0_pskFlag}
		if [ "$psk_flag" = "0" ]
		then
			# using passphrase
			eval wpa_passphrase=\${wlpsk${cpeid}_0_passPhrase}
			wpa_passphrase=$(printf "%b" "$wpa_passphrase")
		else
			# using psk
			eval wpa_psk=\${wlpsk${cpeid}_0_psk}
		fi
		eval group_key_interval=\${wl1x_${ap_index}_grpKeyIntvl}
		wpa_group_rekey=$group_key_interval
		wpa_gmk_rekey=$group_key_interval
	fi
	if [ "$beacon_type" = "$BEACON_WPA_WPA2_COMPLIANT" ]
	then
		wpa_pairwise="TKIP"
		rsn_pairwise="CCMP"
	else
		wpa_pairwise=`convert_wpa_pairwise $encr_type`
	fi
fi

### OSEN parameters
osen=""
eval disable_dgaf=\${wlhs2_${ap_index}_dgafDisabled}
if [ "$beacon_type" = "$BEACON_OSEN" ]
then
	osen=1
	disable_dgaf=1
fi

### RADIUS parameters
ieee8021x=""
auth_server_addr=""
auth_server_port=""
auth_server_shared_secret=""
acct_server_addr=""
acct_server_port=""
acct_server_shared_secret=""
eap_reauth_period=""
if [ "$auth_type" = "$AUTH_RADIUS" ]
then
	ieee8021x=1
	eval auth_server_addr=\${wl1x_${ap_index}_radiusIP}
	eval auth_server_port=\${wl1x_${ap_index}_radiusPort}
	acct_server_addr=$auth_server_addr
	acct_server_port=$((auth_server_port+1))
	eval auth_server_shared_secret=\${wl1x_${ap_index}_radiusSecret}
	auth_server_shared_secret=$(printf "%b" "$auth_server_shared_secret")
	acct_server_shared_secret="$auth_server_shared_secret"
	eval eap_reauth_period=\${wl1x_${ap_index}_reAuthIntvl}
fi

### 802.11w - PMF parameters
assoc_sa_query_max_timeout=""
assoc_sa_query_retry_timeout=""
if [ "$pmf_enabled" = "$PMF_DISABLED" ]
then
	ieee80211w=0
else
	ieee80211w=$((pmf_required+1))
	eval assoc_sa_query_max_timeout=\${wlsec_${ap_index}_pmfSaQMaxTimeout}
	eval assoc_sa_query_retry_timeout=\${wlsec_${ap_index}_pmfSaQRetryTimeout}
fi

# Check if a hostapd parameter was changed.
hostapd_changed=`check_param_changed "$hostapd_params"`

# If a hostapd parameter was changed, remove sec parameters from hostapd_vap temp conf file and write all of them with updated values.
if [ "$hostapd_changed" ]
then
	hostapd_vap_conf_name=${TEMP_CONF_DIR}/hostapd_vap_${interface_name}_${pid}.conf
	# Remove the hostapd sec parameters.
	remove_params_from_conf "###$comment $hostapd_params" $hostapd_vap_conf_name $HOSTAPD_VAP_CONF_PREFIX

	# Write the hostapd sec parameters.
	set_conf_param hostapd_vap comment otf $pid $interface_name comment "$comment"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name auth_algs "$auth_algs"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name eapol_key_index_workaround "$eapol_key_index_workaround"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa "$wpa"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name eap_server "$eap_server"

	# Write WEP parameters.
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wep_default_key "$wep_default_key"
	# If key is ASCII, add quotes
	for wep_key_name in wep_key0 wep_key1 wep_key2 wep_key3
	do
		eval wep_key=\$$wep_key_name
		[ -z "$wep_key" ] && break
		if [ "$wep_key_type" = "$WEP_ASCII" ]
		then
			set_conf_param hostapd_vap regular no_otf $pid $interface_name $wep_key_name "\"$wep_key\""
		else
			set_conf_param hostapd_vap regular no_otf $pid $interface_name $wep_key_name "$wep_key"
		fi
	done

	# Write wpa parameters
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa_key_mgmt "$wpa_key_mgmt"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa_passphrase "$wpa_passphrase"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa_psk "$wpa_psk"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa_group_rekey "$wpa_group_rekey"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa_gmk_rekey "$wpa_gmk_rekey"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name wpa_pairwise "$wpa_pairwise"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name rsn_pairwise "$rsn_pairwise"

	# Write OSEN parameters
	set_conf_param hostapd_vap regular no_otf $pid $interface_name osen "$osen"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name disable_dgaf "$disable_dgaf"

	# Write radius parameters
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ieee8021x "$ieee8021x"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name auth_server_addr "$auth_server_addr"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name auth_server_port "$auth_server_port"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name auth_server_shared_secret "$auth_server_shared_secret"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name acct_server_addr "$acct_server_addr"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name acct_server_port "$acct_server_port"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name acct_server_shared_secret "$acct_server_shared_secret"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name eap_reauth_period "$eap_reauth_period"

	# Write 802.11w - PMF parameters
	set_conf_param hostapd_vap regular no_otf $pid $interface_name ieee80211w "$ieee80211w"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name assoc_sa_query_max_timeout "$assoc_sa_query_max_timeout"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name assoc_sa_query_retry_timeout "$assoc_sa_query_retry_timeout"
fi

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
