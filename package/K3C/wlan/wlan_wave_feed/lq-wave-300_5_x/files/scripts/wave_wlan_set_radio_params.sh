#!/bin/sh
# This script updates the radio parameters in the configuration files.

script_name="wave_wlan_set_radio_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name comment
local driver_params driver_otf_params hostapd_params drvhlpr_params
local driver_current_values hostapd_current_values drvhlpr_current_values driver_new_values
local hostapd_changed drvhlpr_changed
local hostapd_phy_conf_name drvhlpr_conf_name
local sBeaconPeriod sDTIMPeriod sAocsRestrictCh sShortRetryLim sLongRetryLimit \
sMSDULifetime sPowerSelection sTxPowerLimOpt s11nProtection sCoCAutoCfg \
sPCoCPower sPCoCAutoCfg sCoexMode sCoexThreshold sCoexIntolMode \
sCoexDelayFactor sCoexScanIntrvl sAlgoCalibrMask sOnlineACM driver_debug \
sScanParams sScanParamsBG sScanModifFlags sScanCalCwMasks sFWRecovery sNumMsduInAmsdu
local sCoCPower sEnableRadio sDoSimpleCLI
local country_code hw_mode ieee80211d channel preamble \
beacon_int rts_threshold ieee80211n ht_capab ht_tx_bf_capab ht_rifs \
ieee80211h ieee80211ac vht_oper_chwidth vht_capab \
vht_oper_centr_freq_seg0_idx ap_max_num_sta acs_num_scans \
obss_interval scan_passive_dwell scan_active_dwell \
scan_passive_total_per_channel scan_active_total_per_channel \
channel_transition_delay_factor scan_activity_threshold \
obss_beacon_rssi_threshold acs_numbss_info_file \
acs_numbss_coeflist dfs_debug_chan chanlist
local Debug_SoftwareWatchdogEnable Interface arp_iface0 arp_iface1 recovery_script_path
local freq_band network_mode channel_bonding secondary_channel

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
comment="___Radio_parameters___###"

# Define list of radio parameters
driver_params="sBeaconPeriod
sDTIMPeriod
sAocsRestrictCh
sShortRetryLim
sLongRetryLimit
sMSDULifetime
sPowerSelection
sTxPowerLimOpt
s11nProtection
sCoCAutoCfg
sCoexMode
sCoexThreshold
sCoexIntolMode
sCoexDelayFactor
sCoexScanIntrvl
sAlgoCalibrMask
sOnlineACM
driver_debug
sScanParams
sScanParamsBG
sScanModifFlags
sScanCalCwMasks
sFWRecovery
sDoSimpleCLI
sNumMsduInAmsdu"

driver_otf_params="sCoCPower
sEnableRadio
sPCoCPower
sPCoCAutoCfg"

hostapd_params="country_code
hw_mode
ieee80211d
channel
preamble
beacon_int
rts_threshold
ieee80211n
ht_capab
ht_tx_bf_capab
ht_rifs
ieee80211h
ieee80211ac
vht_oper_chwidth
vht_capab
vht_oper_centr_freq_seg0_idx
ap_max_num_sta
acs_num_scans
obss_interval
scan_passive_dwell
scan_active_dwell
scan_passive_total_per_channel
scan_active_total_per_channel
channel_transition_delay_factor
scan_activity_threshold
obss_beacon_rssi_threshold
acs_numbss_info_file
acs_numbss_coeflist
dfs_debug_chan
chanlist"

drvhlpr_params="Debug_SoftwareWatchdogEnable
Interface
arp_iface0
arp_iface1
recovery_script_path"

# Read current values and initiate new values file.
driver_current_values=${TEMP_CONF_DIR}/driver_current_values_${interface_name}
driver_new_values=${TEMP_CONF_DIR}/driver_new_values_${interface_name}_${pid}
hostapd_current_values=${TEMP_CONF_DIR}/hostapd_current_values_${interface_name}_${pid}
drvhlpr_current_values=${TEMP_CONF_DIR}/drvhlpr_current_values_${interface_name}_${pid}

[ -e "$driver_current_values" ] && . $driver_current_values
touch $driver_new_values
read_current_values $HOSTAPD_PHY_CONF_PREFIX $interface_name $hostapd_current_values
read_current_values drvhlpr $interface_name $drvhlpr_current_values
read_current_values $DRIVER_POST_UP_CONF_PREFIX $interface_name $driver_current_values

# Calculate new values.
# Read the frequency value, as some features are for 2.4Ghz or 5Ghz only.
eval freq_band=\${wlphy_${ap_index}_freqBand}
# Read the network mode value, as some parameters depend on this value
eval network_mode=\${wlphy_${ap_index}_standard}

### 802.11 parameters
# Enable/disable the phy radio
sEnableRadio=`convert_phy_enable $ap_index`

# In hostapd, hw_mode with ieee80211n and ieee80211ac define the network mode:
# Network mode   hw_mode   ieee80211n   ieee80211ac
# 11b only          b         0            0
# 11bg              g         0            0
# 11bgn             g         1            0
# 11a only          a         0            0
# 11an              a         1            0
# 11anac            a         1            1
# Other network modes are currently not supported.
hw_mode=`convert_hw_mode $freq_band $network_mode`

# Beacon interval and DTIM period are set both in driver and hostapd (dtim is set as "main" parameter).
preamble=`convert_preamble $ap_index`
eval beacon_int=\${wlphy_${ap_index}_beaconInt}
sBeaconPeriod=$beacon_int
eval sDTIMPeriod=\${wlphy_${ap_index}_dtimInt}
eval rts_threshold=\${wlphy_${ap_index}_rts}

eval country_code=\${wlphy_${ap_index}_country}
eval ieee80211d=\${wlphy_${ap_index}_dot11dEnable}
# Radar detection is only for 5Ghz band.
[ "$freq_band" = "$FREQ_5G" ] && eval ieee80211h=\${wlphy_${ap_index}_hRadarEna}

eval sAocsRestrictCh=\${wlphywave_${ap_index}_aocsRestrictCh}
eval auto_channel=\${wlphy_${ap_index}_autoChanEna}
if [ "$auto_channel" = "1" ]
then
	channel="acs_numbss"
else
	eval channel=\${wlphy_${ap_index}_channelNo}
fi
eval sShortRetryLim=\${wlphy_${ap_index}_shortRetryLimit}
eval sLongRetryLimit=\${wlphy_${ap_index}_longRetryLimit}
eval sMSDULifetime=\${wlphy_${ap_index}_txRetryLifetime}
sPowerSelection=""
sPowerSelection=`convert_power_level $ap_index`
eval s11nProtection=\${wlphy_${ap_index}_nProtection}

### 802.11n parameters
# Set 802.11n parameters if HT network mode is set.
ieee80211n=`convert_ieee80211n $network_mode`
eval channel_bonding=\${wlphy_${ap_index}_nChanWidth}
eval secondary_channel=\${wlphy_${ap_index}_nExtChanPos}
sTxPowerLimOpt=""
ht_capab=""
ht_tx_bf_capab=""
ht_rifs=""


# For HT mode, set HT parameters.
if [ "$ieee80211n" = "1" ]
then
	# TODO: add boost parameters once 3.4.2 is merged.
	eval sTxPowerLimOpt=\${wlphywave_${ap_index}_boostMode}
	ht_capab=`convert_ht_capab $ap_index $interface_name $channel_bonding $secondary_channel $auto_channel`
	ht_tx_bf_capab=`convert_ht_tx_bf_capab $ap_index $interface_name`
	ht_rifs=1
	eval chanlist=\${wlphy_${ap_index}_n11hAllowedChannels}
fi

### 802.11ac parameters
# Set 802.11ac parameters if VHT network mode is set.
ieee80211ac=`convert_ieee80211ac $network_mode`

vht_oper_chwidth=""
vht_capab=""
vht_oper_centr_freq_seg0_idx=""

# For VHT mode, set VHT parameters.
if [ "$ieee80211ac" = "1" ]
then
	vht_oper_chwidth=0
	[ "$channel_bonding" = "$CH_WIDTH_80" ] && vht_oper_chwidth=1
	vht_capab=`convert_vht_capab $ap_index $interface_name`
	vht_oper_centr_freq_seg0_idx=`convert_center_freq $channel $channel_bonding $secondary_channel`
fi

### Auto CoC parameters
sCoCPower=`convert_auto_coc $ap_index`
sCoCAutoCfg=`convert_coc_auto_config $ap_index`

### Power CoC parameters
# Only for 2.4Ghz.
sPCoCPower=""
sPCoCAutoCfg=""
eval sPCoCPower=\${wlcoc_${ap_index}_pCocEna}
if [ "$sPCoCPower" = "1" ]
then
	sPCoCAutoCfg=`convert_power_coc_auto_config $ap_index`
fi

### 20/40 coexistence parameters
# Only for 2.4Ghz.
obss_interval="0"
scan_passive_dwell=""
scan_active_dwell=""
scan_passive_total_per_channel=""
scan_active_total_per_channel=""
channel_transition_delay_factor=""
scan_activity_threshold=""
obss_beacon_rssi_threshold=""

if [ "$freq_band" = "$FREQ_24G" ]
then
	obss_interval=`convert_obss_scan_interval $ap_index`
	if [ $obss_interval -gt 0 ]
	then
		eval scan_passive_dwell=\${wlphywave_${ap_index}_nCoexPassiveDwell}
		eval scan_active_dwell=\${wlphywave_${ap_index}_nCoexActiveDwell}
		eval scan_passive_total_per_channel=\${wlphywave_${ap_index}_nCoexPassivePerCh}
		eval scan_active_total_per_channel=\${wlphywave_${ap_index}_nCoexActivePerCh}
		eval channel_transition_delay_factor=\${wlphy_${ap_index}_nCoexTransDelay}
		eval scan_activity_threshold=\${wlphywave_${ap_index}_nCoexActivityThreshold}
		eval obss_beacon_rssi_threshold=\${wlphy_${ap_index}_nCoexRssiThreshold}
	fi
fi

### VAP limits
ap_max_num_sta=`iwpriv $interface_name gAPCapsMaxSTAs`
ap_max_num_sta=${ap_max_num_sta##*:}

### AP scan parameters
eval acs_num_scans=\${wlphywave_${ap_index}_acsNumScans}
sScanParams=`convert_scan_params $ap_index`
sScanParamsBG=`convert_bg_scan_params $ap_index`
eval sScanModifFlags=\${wlphywave_${ap_index}_scanModifier}
sScanCalCwMasks=`convert_calibration_chan_width_for_scan $ap_index`

### WAVE specific features parameters
eval sAlgoCalibrMask=\${wlphywave_${ap_index}_offlineAlgoCalibrationMask}
eval sOnlineACM=\${wlphywave_${ap_index}_onlineAlgoCalibrationMask}
eval Debug_SoftwareWatchdogEnable=\${wlphywave_${ap_index}_fwCompleteRecoverEna}
Interface=$interface_name
arp_iface0="eth0"
arp_iface1="eth1"
recovery_script_path="${ETC_PATH}/wave_wlan_fw_recovery_notify"
sFWRecovery=`convert_fw_recovery $ap_index`
sNumMsduInAmsdu=`convert_num_msdu_in_amsdu $ap_index`
sDoSimpleCLI=`convert_txop_enbale $ap_index`

### Numbss for ACS parameters
acs_numbss_info_file=/tmp/wave_conf/acs_numbss_info_${interface_name}.txt
eval acs_numbss_coeflist=\${wlphywave_${ap_index}_acsNumbssCoef}

### Radar simulation debug
dfs_debug_chan=`convert_radar_simulation_debug_channel $ap_index`

# Add the console driver debug level command to the drv_config_pre_up file only for wlan0 to configure the whole system.
[ "$interface_name" = "wlan0" ] && driver_debug=`convert_driver_debug_level $ap_index cdebug`

# Write the parameters to the configuration files.
# Update current driver values with new values.
update_driver_params_file "$driver_params" $driver_new_values $driver_current_values

# Check if a hostapd parameter was changed.
hostapd_changed=`check_param_changed "$hostapd_params"`

# Check if a drvhlpr parameter was changed.
drvhlpr_changed=`check_param_changed "$drvhlpr_params"`

# If a hostapd parameter was changed, remove radio parameters from hostapd_phy temp conf file and write all of them with updated values.
if [ "$hostapd_changed" ]
then
	hostapd_phy_conf_name=${TEMP_CONF_DIR}/hostapd_phy_${interface_name}_${pid}.conf
	# Remove the hostapd radio parameters.
	remove_params_from_conf "###$comment $hostapd_params" $hostapd_phy_conf_name $HOSTAPD_PHY_CONF_PREFIX

	# Write the hostapd radio parameters.
	set_conf_param hostapd_phy comment otf $pid $interface_name comment "$comment"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name country_code "$country_code"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name hw_mode "$hw_mode"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ieee80211d "$ieee80211d"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name channel "$channel"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name preamble "$preamble"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name beacon_int "$beacon_int"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name rts_threshold "$rts_threshold"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ieee80211n "$ieee80211n"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ht_capab "$ht_capab"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ht_tx_bf_capab "$ht_tx_bf_capab"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ht_rifs "$ht_rifs"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ieee80211ac "$ieee80211ac"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name vht_oper_chwidth "$vht_oper_chwidth"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name vht_capab "$vht_capab"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name vht_oper_centr_freq_seg0_idx "$vht_oper_centr_freq_seg0_idx"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ap_max_num_sta "$ap_max_num_sta"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name acs_num_scans "$acs_num_scans"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name ieee80211h "$ieee80211h"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name obss_interval "$obss_interval"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name scan_passive_dwell "$scan_passive_dwell"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name scan_active_dwell "$scan_active_dwell"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name scan_passive_total_per_channel "$scan_passive_total_per_channel"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name scan_active_total_per_channel "$scan_active_total_per_channel"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name channel_transition_delay_factor "$channel_transition_delay_factor"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name scan_activity_threshold "$scan_activity_threshold"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name obss_beacon_rssi_threshold "$obss_beacon_rssi_threshold"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name acs_numbss_info_file "$acs_numbss_info_file"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name acs_numbss_coeflist "$acs_numbss_coeflist"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name dfs_debug_chan "$dfs_debug_chan"
	set_conf_param hostapd_phy regular no_otf $pid $interface_name chanlist "$chanlist"
fi

# If a drvhlpr parameter was changed, re-write the drvhlpr conf file.
if [ "$drvhlpr_changed" ]
then
	drvhlpr_conf_name=${TEMP_CONF_DIR}/drvhlpr_${interface_name}_${pid}.conf
	# Delete the drvhlpr conf file and re-write it.
	rm -f $drvhlpr_conf_name

	set_conf_param drvhlpr regular no_otf $pid $interface_name Debug_SoftwareWatchdogEnable "$Debug_SoftwareWatchdogEnable"
	set_conf_param drvhlpr regular no_otf $pid $interface_name Interface "$Interface"
	set_conf_param drvhlpr regular no_otf $pid $interface_name arp_iface0 "$arp_iface0"
	set_conf_param drvhlpr regular no_otf $pid $interface_name arp_iface1 "$arp_iface1"
	set_conf_param drvhlpr regular no_otf $pid $interface_name recovery_script_path "$recovery_script_path"
fi

# Check if a driver OTF parameter was changed.
driver_changed=`check_param_changed "$driver_otf_params"`

# If a driver OTF parameter was changed, remove radio OTF parameters from drv_config_post_up temp conf file and write all of them with updated values.
if [ "$driver_changed" ]
then
	drv_config_post_up_conf_name=${TEMP_CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf
	# Remove the driver radio OTF parameters.
	remove_params_from_conf "$driver_otf_params" $drv_config_post_up_conf_name $DRIVER_POST_UP_CONF_PREFIX

	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sCoCPower "$sCoCPower"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sEnableRadio "$sEnableRadio"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sPCoCPower "$sPCoCPower"
	set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sPCoCAutoCfg "$sPCoCAutoCfg"
fi

# Write the driver radio parameters that were changed.
[ "$sBeaconPeriod_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sBeaconPeriod "$sBeaconPeriod"
[ "$sDTIMPeriod_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sDTIMPeriod "$sDTIMPeriod"
[ "$sAocsRestrictCh_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sAocsRestrictCh "$sAocsRestrictCh"
[ "$sShortRetryLim_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sShortRetryLim "$sShortRetryLim"
[ "$sLongRetryLimit_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sLongRetryLimit "$sLongRetryLimit"
[ "$sMSDULifetime_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sMSDULifetime "$sMSDULifetime"
[ "$sPowerSelection_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sPowerSelection "$sPowerSelection"
#[ "$s11dActive_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name s11dActive "$s11dActive"
# TODO: enable once sTxPowerLimOpt will be supported
#[ "$sTxPowerLimOpt_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sTxPowerLimOpt "$sTxPowerLimOpt"
[ "$s11nProtection_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name s11nProtection "$s11nProtection"
[ "$sCoCAutoCfg_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sCoCAutoCfg "$sCoCAutoCfg"
[ "$sCoexMode_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sCoexMode "$sCoexMode"
[ "$sCoexThreshold_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sCoexThreshold "$sCoexThreshold"
[ "$sCoexIntolMode_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sCoexIntolMode "$sCoexIntolMode"
[ "$sCoexDelayFactor_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sCoexDelayFactor "$sCoexDelayFactor"
[ "$sCoexScanIntrvl_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sCoexScanIntrvl "$sCoexScanIntrvl"
[ "$sAlgoCalibrMask_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sAlgoCalibrMask "$sAlgoCalibrMask"
[ "$sOnlineACM_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sOnlineACM "$sOnlineACM"
[ "$driver_debug_changed" ] && set_conf_param drv_config_pre_up proc no_otf $pid $interface_name  "/proc/net/mtlk_log/debug" "$driver_debug"
[ "$sScanParams_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sScanParams "$sScanParams"
[ "$sScanParamsBG_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sScanParamsBG "$sScanParamsBG"
[ "$sScanModifFlags_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sScanModifFlags "$sScanModifFlags"
[ "$sScanCalCwMasks_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sScanCalCwMasks "$sScanCalCwMasks"
[ "$sFWRecovery_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sFWRecovery "$sFWRecovery"
[ "$sDoSimpleCLI_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sDoSimpleCLI "$sDoSimpleCLI"
[ "$sNumMsduInAmsdu_changed" ] && set_conf_param drv_config_pre_up iwpriv no_otf $pid $interface_name sNumMsduInAmsdu "$sNumMsduInAmsdu"

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
