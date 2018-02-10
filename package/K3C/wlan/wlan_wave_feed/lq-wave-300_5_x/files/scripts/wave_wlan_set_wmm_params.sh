#!/bin/sh
# This script updates the WMM parameters in the configuration files.

script_name="wave_wlan_set_wmm_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

local ap_index pid interface_name comment
local driver_params hostapd_vap_params hostapd_phy_params
local driver_current_values driver_new_values hostapd_phy_current_values hostapd_vap_current_values
local sUAPSDMaxSP
local uapsd_advertisement_enabled
local wmm_enabled tx_queue_data0_aifs tx_queue_data0_cwmin tx_queue_data0_cwmax tx_queue_data0_burst tx_queue_data1_aifs tx_queue_data1_cwmin tx_queue_data1_cwmax tx_queue_data1_burst tx_queue_data2_aifs tx_queue_data2_cwmin tx_queue_data2_cwmax tx_queue_data2_burst tx_queue_data3_aifs tx_queue_data3_cwmin tx_queue_data3_cwmax tx_queue_data3_burst wmm_ac_be_aifs wmm_ac_be_cwmin wmm_ac_be_cwmax wmm_ac_be_txop_limit wmm_ac_bk_aifs wmm_ac_bk_cwmin wmm_ac_bk_cwmax wmm_ac_bk_txop_limit wmm_ac_vi_aifs wmm_ac_vi_cwmin wmm_ac_vi_cwmax wmm_ac_vi_txop_limit wmm_ac_vo_aifs wmm_ac_vo_cwmin wmm_ac_vo_cwmax wmm_ac_vo_txop_limit
local hostapd_phy_changed hostapd_vap_changed
local hostapd_phy_conf_name hostapd_vap_conf_name drv_config_post_up_conf_name
local ap_type cpeid driver_changed

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
eval ap_type=\${wlmn_${ap_index}_apType}
comment="___WMM_parameters___###"

# Define list of wmm parameters
driver_params="sUAPSDMaxSP"

hostapd_vap_params="uapsd_advertisement_enabled"

hostapd_phy_params="wmm_enabled
tx_queue_data0_aifs
tx_queue_data0_cwmin
tx_queue_data0_cwmax
tx_queue_data0_burst
tx_queue_data1_aifs
tx_queue_data1_cwmin
tx_queue_data1_cwmax
tx_queue_data1_burst
tx_queue_data2_aifs
tx_queue_data2_cwmin
tx_queue_data2_cwmax
tx_queue_data2_burst
tx_queue_data3_aifs
tx_queue_data3_cwmin
tx_queue_data3_cwmax
tx_queue_data3_burst
wmm_ac_be_aifs
wmm_ac_be_cwmin
wmm_ac_be_cwmax
wmm_ac_be_txop_limit
wmm_ac_bk_aifs
wmm_ac_bk_cwmin
wmm_ac_bk_cwmax
wmm_ac_bk_txop_limit
wmm_ac_vi_aifs
wmm_ac_vi_cwmin
wmm_ac_vi_cwmax
wmm_ac_vi_txop_limit
wmm_ac_vo_aifs
wmm_ac_vo_cwmin
wmm_ac_vo_cwmax
wmm_ac_vo_txop_limit"

# Read current values and initiate new values file.
if [ "$ap_type" = "$AP" ]
then
	driver_current_values=${TEMP_CONF_DIR}/driver_current_values_${interface_name}
	driver_new_values=${TEMP_CONF_DIR}/driver_new_values_${interface_name}_${pid}
	hostapd_phy_current_values=${TEMP_CONF_DIR}/hostapd_phy_current_values_${interface_name}_${pid}

	[ -e "$driver_current_values" ] && . $driver_current_values
	touch $driver_new_values
	read_current_values $HOSTAPD_PHY_CONF_PREFIX $interface_name $hostapd_phy_current_values
fi

hostapd_vap_current_values=${TEMP_CONF_DIR}/hostapd_vap_current_values_${interface_name}

read_current_values $HOSTAPD_VAP_CONF_PREFIX $interface_name $hostapd_vap_current_values

# Calculate new values.
# Most WMM parameters are for physical interfaces. Only UAPSD (WMM-PS) parameters are for VAPs.
eval uapsd_advertisement_enabled=\${wlmn_${ap_index}_uapsdEna}

# TODO: enable sUAPSDMaxSP once WMM-PS is supported.
#eval sUAPSDMaxSP=\${wlmnwave_${ap_index}_uapsdMaxSpLength}
sUAPSDMaxSP=""

if [ "$ap_type" = "$AP" ]
then
	# When working with WMM, the rc.conf uses cpeid
	eval cpeid=\${wlmn_${ap_index}_cpeId}

	### WMM parameters
	eval wmm_enabled=\${wlmn_${ap_index}_wmmEna}

	### WMM AP parameters
	# rc.conf indexes:
	# BK: 1
	# BE: 0
	# VI: 2
	# VO: 3
	# hostapd indexes:
	# BK: data3
	# BE: data2
	# VI: data1
	# VO: data0

	tx_queue_data0_aifs=""
	tx_queue_data0_cwmin=""
	tx_queue_data0_cwmax=""
	tx_queue_data0_burst=""
	tx_queue_data1_aifs=""
	tx_queue_data1_cwmin=""
	tx_queue_data1_cwmax=""
	tx_queue_data1_burst=""
	tx_queue_data2_aifs=""
	tx_queue_data2_cwmin=""
	tx_queue_data2_cwmax=""
	tx_queue_data2_burst=""
	tx_queue_data3_aifs=""
	tx_queue_data3_cwmin=""
	tx_queue_data3_cwmax=""
	tx_queue_data3_burst=""

	eval tx_queue_data0_aifs=\${wlawmm${cpeid}_3_AIFSN}
	tx_queue_data0_cwmin=`convert_wmm_cw ${cpeid} 3 min`
	tx_queue_data0_cwmax=`convert_wmm_cw ${cpeid} 3 max`
	tx_queue_data0_burst=`convert_txop ${cpeid} 3`
	eval tx_queue_data1_aifs=\${wlawmm${cpeid}_2_AIFSN}
	tx_queue_data1_cwmin=`convert_wmm_cw ${cpeid} 2 min`
	tx_queue_data1_cwmax=`convert_wmm_cw ${cpeid} 2 max`
	tx_queue_data1_burst=`convert_txop ${cpeid} 2`
	eval tx_queue_data2_aifs=\${wlawmm${cpeid}_0_AIFSN}
	tx_queue_data2_cwmin=`convert_wmm_cw ${cpeid} 0 min`
	tx_queue_data2_cwmax=`convert_wmm_cw ${cpeid} 0 max`
	tx_queue_data2_burst=`convert_txop ${cpeid} 0`
	eval tx_queue_data3_aifs=\${wlawmm${cpeid}_1_AIFSN}
	tx_queue_data3_cwmin=`convert_wmm_cw ${cpeid} 1 min`
	tx_queue_data3_cwmax=`convert_wmm_cw ${cpeid} 1 max`
	tx_queue_data3_burst=`convert_txop ${cpeid} 1`

	### WMM STA parameters
	eval wmm_ac_be_aifs=\${wlswmm${cpeid}_0_AIFSN}
	eval wmm_ac_be_cwmin=\${wlswmm${cpeid}_0_ECWmin}
	eval wmm_ac_be_cwmax=\${wlswmm${cpeid}_0_ECWmax}
	eval wmm_ac_be_txop_limit=\${wlswmm${cpeid}_0_TXOP}
	eval wmm_ac_bk_aifs=\${wlswmm${cpeid}_1_AIFSN}
	eval wmm_ac_bk_cwmin=\${wlswmm${cpeid}_1_ECWmin}
	eval wmm_ac_bk_cwmax=\${wlswmm${cpeid}_1_ECWmax}
	eval wmm_ac_bk_txop_limit=\${wlswmm${cpeid}_1_TXOP}
	eval wmm_ac_vi_aifs=\${wlswmm${cpeid}_2_AIFSN}
	eval wmm_ac_vi_cwmin=\${wlswmm${cpeid}_2_ECWmin}
	eval wmm_ac_vi_cwmax=\${wlswmm${cpeid}_2_ECWmax}
	eval wmm_ac_vi_txop_limit=\${wlswmm${cpeid}_2_TXOP}
	eval wmm_ac_vo_aifs=\${wlswmm${cpeid}_3_AIFSN}
	eval wmm_ac_vo_cwmin=\${wlswmm${cpeid}_3_ECWmin}
	eval wmm_ac_vo_cwmax=\${wlswmm${cpeid}_3_ECWmax}
	eval wmm_ac_vo_txop_limit=\${wlswmm${cpeid}_3_TXOP}

	# Write the parameters to the configuration files.
	# Check if a hostapd parameter was changed.
	hostapd_phy_changed=`check_param_changed "$hostapd_phy_params"`

	# If a hostapd phy parameter was changed, remove wmm parameters from hostapd_phy temp conf file and write all of them with updated values.
	if [ "$hostapd_phy_changed" ]
	then
		hostapd_phy_conf_name=${TEMP_CONF_DIR}/hostapd_phy_${interface_name}_${pid}.conf
		remove_params_from_conf "###$comment $hostapd_phy_params" $hostapd_phy_conf_name $HOSTAPD_PHY_CONF_PREFIX

		# Write the hostapd phy wmm parameters.
		set_conf_param hostapd_phy comment otf $pid $interface_name comment "$comment"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_enabled "$wmm_enabled"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data0_aifs "$tx_queue_data0_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data0_cwmin "$tx_queue_data0_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data0_cwmax "$tx_queue_data0_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data0_burst "$tx_queue_data0_burst"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data1_aifs "$tx_queue_data1_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data1_cwmin "$tx_queue_data1_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data1_cwmax "$tx_queue_data1_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data1_burst "$tx_queue_data1_burst"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data2_aifs "$tx_queue_data2_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data2_cwmin "$tx_queue_data2_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data2_cwmax "$tx_queue_data2_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data2_burst "$tx_queue_data2_burst"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data3_aifs "$tx_queue_data3_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data3_cwmin "$tx_queue_data3_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data3_cwmax "$tx_queue_data3_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name tx_queue_data3_burst "$tx_queue_data3_burst"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_be_aifs "$wmm_ac_be_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_be_cwmin "$wmm_ac_be_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_be_cwmax "$wmm_ac_be_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_be_txop_limit "$wmm_ac_be_txop_limit"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_bk_aifs "$wmm_ac_bk_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_bk_cwmin "$wmm_ac_bk_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_bk_cwmax "$wmm_ac_bk_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_bk_txop_limit "$wmm_ac_bk_txop_limit"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vi_aifs "$wmm_ac_vi_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vi_cwmin "$wmm_ac_vi_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vi_cwmax "$wmm_ac_vi_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vi_txop_limit "$wmm_ac_vi_txop_limit"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vo_aifs "$wmm_ac_vo_aifs"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vo_cwmin "$wmm_ac_vo_cwmin"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vo_cwmax "$wmm_ac_vo_cwmax"
		set_conf_param hostapd_phy regular no_otf $pid $interface_name wmm_ac_vo_txop_limit "$wmm_ac_vo_txop_limit"
	fi
fi
# Go over the wmm hostapd vap parameters and see if any was changed or new.
# If change/new was found, set the flag.
hostapd_vap_changed=`check_param_changed "$hostapd_vap_params"`

# If a hostapd vap parameter was changed, remove wmm parameters from hostapd_vap temp conf file and write all of them with updated values.
if [ "$hostapd_vap_changed" ]
then
	hostapd_vap_conf_name=${TEMP_CONF_DIR}/hostapd_vap_${interface_name}_${pid}.conf
	remove_params_from_conf "###$comment $hostapd_vap_params" $hostapd_vap_conf_name $HOSTAPD_VAP_CONF_PREFIX

	# Write the hostapd vap wmm parameters.
	set_conf_param hostapd_vap comment otf $pid $interface_name comment "$comment"
	set_conf_param hostapd_vap regular no_otf $pid $interface_name uapsd_advertisement_enabled "$uapsd_advertisement_enabled"
fi

# Check if a driver parameter was changed.
driver_changed=`check_param_changed "$driver_params"`

# If a driver parameter was changed, remove WMM parameters from drv_config_post_up temp conf file and write all of them with updated values.
if [ "$driver_changed" ]
then
	drv_config_post_up_conf_name=${TEMP_CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf
	# Remove the driver WMM parameters.
	remove_params_from_conf "$driver_params" $drv_config_post_up_conf_name $DRIVER_POST_UP_CONF_PREFIX

	set_conf_param drv_config_post_up iwpriv no_otf $pid $interface_name sUAPSDMaxSP "$sUAPSDMaxSP"
fi

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
