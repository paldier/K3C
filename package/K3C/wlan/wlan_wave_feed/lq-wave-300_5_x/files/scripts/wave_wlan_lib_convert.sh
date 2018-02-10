#!/bin/sh
# Library script to convert from rc.conf values to hostapd/driver values.

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Read rc.conf AutoCoc value.
# If value is 0, read rc.conf numAntennas.
# If value is 0, set phy_enable=0
# Else, phy_enable=1
convert_phy_enable()
{
	# Define local parameters
	local ap_index
	local phy_enable auto_coc_ugw num_antennas

	phy_enable=1
	eval auto_coc_ugw=\${wlcoc_${ap_index}_autoCoC}

	if [ "$auto_coc_ugw" = "0" ]
	then
		eval num_antennas=\${wlcoc_${ap_index}_numAntennas}
		[ "$num_antennas" = "0" ] && phy_enable=0
	fi
	echo "$phy_enable"
}

# Preamble values in rc.conf are 1=short, 2=long
# Preamble values in hostapd are 1=short, 0=long
convert_preamble()
{
	# Define local parameters
	local ap_index
	local preamble

	ap_index=$1

	eval preamble=\${wlphy_${ap_index}_preamble}
	[ "$preamble" = "2" ] && preamble=0
	echo $preamble
}

# Get rc.conf network mode and frequency band and set hw_mode.
# If network mode is 11b, set hw_mode=b.
# If not 11b, set hw_mode=g for 2.4Ghz and hw_mode=a for 5Ghz.
convert_hw_mode()
{
	# Define local parameters
	local freq_band network_mode hw_mode

	freq_band=$1
	network_mode=$2

	if [ "$network_mode" = "$MODE_11B" ]
	then
		hw_mode=b
	elif [ "$freq_band" = "$FREQ_24G" ]
	then
		hw_mode=g
	else
		hw_mode=a
	fi
	echo $hw_mode
}

# Read rc.conf dtim period value of the radio to which the VAP belongs
convert_dtim_period()
{
	# Define local parameters
	local ap_index
	local radio_cpe_id pap_index

	ap_index=$1
	eval radio_cpe_id=\${wlmn_${ap_index}_radioCpeId}
	pap_index=$((radio_cpe_id-1))
	eval dtim_period=\${wlphy_${pap_index}_dtimInt}
	echo $dtim_period
}

# Read the network mode and if mode is ANAC, set value to 1
convert_opmode_notif()
{
	# Define local parameters
	local ap_index opmode_notif pap_name pap_index network_mode

	ap_index=$1
	opmode_notif=""

	# Find the ap index of the parent of the VAP
	pap_name=${interface_name%%.*}
	pap_index=`find_index_from_wave_if $pap_name`
	eval network_mode=\${wlphy_${pap_index}_standard}

	[ "$network_mode" = "$MODE_11ANAC" ] && opmode_notif=1
	echo "$opmode_notif"
}

# Get rc.conf network mode and if value is 4 and above, set ieee80211n enabled.
convert_ieee80211n()
{
	# Define local parameters
	local ieee80211n network_mode

	network_mode=$1
	ieee80211n=0

	[ $network_mode -gt 3 ] && ieee80211n=1
	echo $ieee80211n
}

# Get rc.conf network mode and if value is value of 802.11ANAC, set ieee80211ac enabled.
convert_ieee80211ac()
{
	# Define local parameters
	local ieee80211ac network_mode

	network_mode=$1
	ieee80211ac=""

	[ "$network_mode" = "$MODE_11ANAC" ] && ieee80211ac=1
	echo $ieee80211ac
}

# Read rc.conf rates and convert to hostapd values.
# rc.conf values are in the format of: 1.0,2.0,5.5,11.0
# hostapd values are in the format of: 10 20 55 110
convert_rates_list()
{
	# Define local parameters
	local ap_index
	local rates_param ugw_rates hostapd_rates

	ap_index=$1
	rates_param=$2

	eval ugw_rates=\${wlmn_${ap_index}_${rates_param}}
	# Remove dots
	hostapd_rates=${ugw_rates//./}
	# Replace commas with spaces
	hostapd_rates=${hostapd_rates//,/ }
	echo "$hostapd_rates"
}

# Read rc.conf power level value (in percentage) and convert to driver value (in db).
convert_power_level()
{
	# Define local parameters
	local ap_index
	local power_level

	ap_index=$1

	eval power_level=\${wlphy_${ap_index}_powerLvl}
	case $power_level in
	100)
		power_level=0
	;;
	50)
		power_level=3
	;;
	25)
		power_level=6
	;;
	12)
		power_level=9
	;;
	esac
	echo $power_level
}

# Read rc.conf numAntennas and AutoCoc.
# If numAntennas value is 0, read prevAutoCoC as AutoCoc and prevNumAntennas as numAntennas.
# If auto_coc value is 0, set num_antennas value for tx and rx.
# If auto_coc value is 1, set value 1 (auto).
convert_auto_coc()
{
	# Define local parameters
	local ap_index
	local auto_coc_ugw num_antennas auto_coc_driver

	ap_index=$1

	eval auto_coc_ugw=\${wlcoc_${ap_index}_autoCoC}
	eval num_antennas=\${wlcoc_${ap_index}_numAntennas}

	if [ "$num_antennas" = "0" ]
	then
		eval auto_coc_ugw=\${wlcoc_${ap_index}_prevAutoCoC}
		eval num_antennas=\${wlcoc_${ap_index}_prevNumAntennas}
	fi

	if [ "$auto_coc_ugw" = "0" ]
	then
		auto_coc_driver="$auto_coc_ugw $num_antennas $num_antennas"
	else
		auto_coc_driver=1
	fi
	echo $auto_coc_driver
}

# Read rc.conf CoC parameters and generate CoC auto config string.
convert_coc_auto_config()
{
	# Define local parameters
	local ap_index
	local limit_antennas ugw_value coc_config limit1_high limit2_low limit2_high limit3_low limit3_high limit4_low

	ap_index=$1

	limit_antennas="1 2 3 4"
	for antenna in $limit_antennas
	do
		eval ugw_value=\${wlcoc_${ap_index}_autoCoC${antenna}x${antenna}TimerInterval}
		coc_config="${coc_config} ${ugw_value}"
	done

	eval limit1_high=\${wlcoc_${ap_index}_autoCoC1x1HighLimit}
	eval limit2_low=\${wlcoc_${ap_index}_autoCoC2x2LowLimit}
	eval limit2_high=\${wlcoc_${ap_index}_autoCoC2x2HighLimit}
	eval limit3_low=\${wlcoc_${ap_index}_autoCoC3x3LowLimit}
	eval limit3_high=\${wlcoc_${ap_index}_autoCoC3x3HighLimit}
	eval limit4_low=\${wlcoc_${ap_index}_autoCoC4x4LowLimit}

	echo "${coc_config} ${limit1_high} ${limit2_low} ${limit2_high} ${limit3_low} ${limit3_high} ${limit4_low}"
}

# Read rc.conf Power CoC parameters and generate power CoC auto config string.
convert_power_coc_auto_config()
{
	# Define local parameters
	local ap_index
	local pcoc_low2high pcoc_high2low pcoc_lim_upper pcoc_lim_lower

	ap_index=$1

	# Return the interval thresholds, the low and high thresholds for the Power CoC(CPU frequecy OTF).
	eval pcoc_low2high=\${wlcoc_${ap_index}_pCocLow2High}
	eval pcoc_high2low=\${wlcoc_${ap_index}_pCocHigh2Low}
	eval pcoc_lim_upper=\${wlcoc_${ap_index}_pCocLimitUpper}
	eval pcoc_lim_lower=\${wlcoc_${ap_index}_pCocLimitLower}
	echo "${pcoc_low2high} ${pcoc_high2low} ${pcoc_lim_lower} ${pcoc_lim_upper}"
}

# Read from rc.conf the uapsdEna value and set uapsd_advertisement_enabled in hostapd.
# In the hostapd: 0 = manage uapsd in hostapd (enabled) 1 = manage uapsd outside hostapd (disabled).
# In rc.conf: 1 = uapsd enabled, 0 = uapsd disabled.
convert_uapsd_advertisement_enabled()
{
	# Define local parameters
	local ap_index
	local ugw_uapsd uapsd_advertisement_enabled

	ap_index=$1

	eval ugw_uapsd=\${wlmn_${ap_index}_uapsdEna}
	uapsd_advertisement_enabled=0
	[ "$ugw_uapsd" = "0" ] && uapsd_advertisement_enabled=1
	echo $uapsd_advertisement_enabled
}

# Read from rc.conf the wmm ecw value and calculate the cw value according to the formula:
# CW = (2^ECW)-1
convert_wmm_cw()
{
	# Define local parameters
	local ap_index
	local ac_index cw_type ecw_value cw_value

	ap_index=$1
	ac_index=$2
	cw_type=$3

	eval ecw_value=\${wlawmm${ap_index}_${ac_index}_ECW${cw_type}}
	cw_value=$((2**ecw_value-1))
	echo $cw_value
}

# Read rc.conf txop value for the recieved AC and convert to hostapd value.
# Hostapd burst time is configured in units of 0.1 msec and UGW TXOP parameter in 32 usec.
# UGW value will be multiplied by 32 and divided by 1000 using string manipulations.
convert_txop()
{
	# Define local parameters
	local ap_index
	local ac_index ac_txop txop_length num dec num_length digit2_position digit2

	ap_index=$1
	ac_index=$2

	eval ac_txop=\${wlawmm${ap_index}_${ac_index}_TXOP}
	# If ugw value is 0, return 0.
	[ "$ac_txop" = "0" ] && echo 0 && return

	# Multiply ugw value by 32.
	ac_txop=$((ac_txop*32))
	# Check txop number length.
	txop_length=${#ac_txop}
	# When the number is less than 3 digits, dividing by 1000 will result in 0.0X result, on other cases, extract the number and the decimals.
	if [ $txop_length -lt 3 ]
	then
		num=0
		dec=0
	else
		num_length=$((txop_length-3))
		num=${ac_txop:0:${num_length}}
		[ -z "$num" ] && num=0
		dec=${ac_txop:${num_length}:1}
	fi
	# Check if need to round the number: read the 2nd digit from the end and if it's 5 or above, round up.
	digit2_position=$((txop_length-2))
	digit2=${ac_txop:${digit2_position}:1}
	if [ $digit2 -gt 4 ]
	then
		if [ $dec -eq 9 ]
		then
			dec=0
			num=$((num+1))
		else
			dec=$((dec+1))
		fi
	fi
	echo "${num}.${dec}"
}

# Read from rc.conf the macAddrCntrlType value and set macaddr_acl in hostapd.
# In the hostapd:
# 	0 = accept unless in deny list
#	1 = deny unless in accept list
#	2 = use external RADIUS server (accept/deny lists are searched first).
# In rc.conf:
#	0 = Allow
#	1 = Deny
#	2 = ACL Disabled
convert_macaddr_acl()
{
	# Define local parameters
	local ap_index
	local ugw_acl macaddr_acl

	ap_index=$1
	ugw_acl=$2

	macaddr_acl=0
	[ "$ugw_acl" = "0" ] && macaddr_acl=1
	echo "$macaddr_acl"
}

# Update the acl list with mac addresses from rc.conf.
update_acl_list()
{
	# Define local parameters
	local ap_index pid interface_name
	local list_type empty_list acl_file tmp_acl_file current_list_populated ap_cpeid new_list_populated ugw_pcpeId mac_exist mac_address tmp_mac i acl_mac in_rc_conf

	list_type=$1
	ap_index=$2
	pid=$3
	empty_list=$4

	eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
	pap_name=${interface_name%%.*}
	eval acl_file=${TEMP_CONF_DIR}/\${${list_type}_ACL_FILE}.${interface_name}.conf
	eval tmp_acl_file=${TEMP_CONF_DIR}/\${${list_type}_ACL_FILE}.${interface_name}_${pid}.conf

	# Get the current list of MACs and compare to new list to see if changes were made.
	# The sed command has 3 steps. Showing example for the MAC 00:11:22:33:44:55 in DENY list:
	# Step 1: Add list type value before all the MACs in the file: DENY_00:11:22:33:44:55
	# Step 2: Add ="1" at the end of each line: DENY_00:11:22:33:44:55="1"
	# Step 3: Replace colon with underscore to allow sourcing of parameters: DENY_00_11_22_33_44_55="1"
	if [ -e "$acl_file" ]
	then
		sed -e 's/^/'${list_type}'_/' -e 's/$/=\"1\"/' -e 's/:/_/g' $acl_file > ${TEMP_CONF_DIR}/acl_params_${interface_name}
		. ${TEMP_CONF_DIR}/acl_params_${interface_name}
	fi
	# Check if current list is populated
	current_list_populated=""
	[ -s "${TEMP_CONF_DIR}/acl_params_${interface_name}" ] && current_list_populated=1
	rm -f ${TEMP_CONF_DIR}/acl_params_${interface_name}

	# If acl is set to disabled, files will remain empty.
	# If current list has macs in it, set restart flag
	if [ "$empty_list" ]
	then
		[ "$current_list_populated" ] && echo "restart_${pap_name}=yes" >> /tmp/$RESTART_FLAG
		return
	fi

	# Go over list of MACs is rc.conf and write MACs to ACL file
	eval ap_cpeid=\${wlmn_${ap_index}_cpeId}
	i=0
	new_list_populated=""
	while [ $i -lt $wlan_mac_control_Count ]
	do
		eval ugw_pcpeId=\${wlmacctrl_${i}_pcpeId}
		if [ "$ugw_pcpeId" = "$ap_cpeid" ]
		then
			mac_exist=""
			new_list_populated=1
			eval mac_address=\${wlmacctrl_${i}_macAddr}
			tmp_mac=${mac_address//:/_}
			eval mac_exist=\${${list_type}_${tmp_mac}}
			[ -z "$mac_exist" ] && echo "restart_${pap_name}=yes" >> /tmp/$RESTART_FLAG
			echo "$mac_address" >> $tmp_acl_file
			# Write the new MAC to the list of rc.conf MACs
			echo "CONF_${tmp_mac}=1" >> ${TEMP_CONF_DIR}/conf_macs
		fi
		i=$((i+1))
	done

	# If no MACs appear in the rc.conf, clear the hostapd MAC list
	if [ -z "$new_list_populated" ]
	then
		cat /dev/null > $acl_file
		echo "restart_${pap_name}=yes" >> /tmp/$RESTART_FLAG
		rm -f ${TEMP_CONF_DIR}/conf_macs
		return
	fi
	. ${TEMP_CONF_DIR}/conf_macs
	rm -f ${TEMP_CONF_DIR}/conf_macs
	
	# Go over the list of MACs in the current list (if populated) and check if MAC in the list is also in the rc.conf
	if [ "$current_list_populated" ]
	then
			while read acl_mac
			do
				acl_mac=${acl_mac//:/_}
				eval in_rc_conf=\${CONF_${acl_mac}}
				[ "$in_rc_conf" != "1" ] && echo "restart_${pap_name}=yes" >> /tmp/$RESTART_FLAG && return
			done < $acl_file
	fi
}

# Read rc.conf values for ht capabilities: Channel bonding with secondary channel, HT-greenfield support, Short-GI, TX-STBC, LDPC, MAX-AMSDU and 40-INTOLERANT and create string for hostapd.
convert_ht_capab()
{
	# Define local parameters
	local ap_index interface_name
	local phy_name driver_ldpc driver_tx_stbc driver_rx_stbc driver_stbc \
	channel_width channel_bonding secondary_channel ht_greenfield short_gi ugw_stbc ugw_ldpc max_amsdu coex_enable intolerant_40 ht_capab_value auto_channel auto_coc

	ap_index=$1
	interface_name=$2
	channel_bonding=$3
	secondary_channel=$4
	auto_channel=$5

	# Read driver capabilities values
	phy_name=`find_phy_from_interface_name $interface_name`

	# LDPC
	driver_ldpc=""
	driver_ldpc=`iw phy $phy_name info | grep LDPC`
	[ -n "$driver_ldpc" ] && driver_ldpc="[LDPC]"
	
	# STBC
	driver_tx_stbc=""
	driver_rx_stbc=""
	driver_no_tx_stbc=`iw phy $phy_name info | grep "No TX STBC"`
	driver_no_rx_stbc=`iw phy $phy_name info | grep "No RX STBC"`
	[ -z "$driver_no_tx_stbc" ] && driver_tx_stbc="[TX-STBC]"
	[ -z "$driver_no_rx_stbc" ] && driver_rx_stbc="[RX-STBC1]"
	driver_stbc="${driver_tx_stbc}${driver_rx_stbc}"
	
	# Read rc.conf values for channel bonding and secondary channel and set:
	# [HT40-] = both 20 MHz and 40 MHz with secondary channel below the primary channel.
	# [HT40+] = both 20 MHz and 40 MHz with secondary channel above the primary channel.
	# When no value is set, 20 MHz is configured.
	# If HT40 needs to be set and ACS is enabled, only [HT40+] can be set.
	channel_width=""
	if [ "$channel_bonding" != "$CH_WIDTH_20" ]
	then
		if [ "$auto_channel" = "1" ]
		then
			channel_width="[HT40+]"
		else
			case "$secondary_channel" in
			$SECONDARY_CHANNEL_UPPER)
				channel_width="[HT40+]"
			;;
			$SECONDARY_CHANNEL_LOWER)
				channel_width="[HT40-]"
			;;
			esac
		fi
	fi
	# For short-GI parameters, in rc.conf: 0=enabled, 1=disabled, 2=auto (set as enabled).
	# The value in rc.conf sets short-GI for both 20Mhz and 40Mhz.
	eval short_gi=\${wlphy_${ap_index}_nGuardIntvl}
	[ "$short_gi" = "2" ] && short_gi=0

	# Check value for STBC. Parameter name in rc.conf is nSTBCrx but used to set value for TX-STBC and RX-STBC.
	eval ugw_stbc=\${wlphy_${ap_index}_nSTBCrx}

	# Check value for LDPC.
	eval ldpc=\${wlphy_${ap_index}_nLDPCen}

	# Check value for MAX-AMSDU
	eval max_amsdu=\${wlphy_${ap_index}_nAMSDUlen}

	# Check value for 40-INTOLERANT if 20/40 coexistence is enabled.
	eval coex_enable=\${wlphy_${ap_index}_n2040CoexEna}
	if [ "$coex_enable" = "1" ]
	then
		eval intolerant_40=\${wlphy_${ap_index}_nCoex40IntoleranceEna}
		[ "$intolerant_40" = "0" ] && intolerant_40=""
	fi

	# Check value for SMPS
	eval auto_coc=\${wlcoc_${ap_index}_autoCoC}
	
	# Build the value string
	ht_capab_value="$channel_width"
	if [ "$short_gi" = "0" ]
	then
		ht_capab_value="${ht_capab_value}[SHORT-GI-20]"
		[ "$channel_bonding" != "$CH_WIDTH_20" ] && ht_capab_value="${ht_capab_value}[SHORT-GI-40]"
	fi
	if [ "$ugw_stbc" -gt "0" ]
	then
		ht_capab_value="${ht_capab_value}${driver_stbc}"
	fi
	if [ "$intolerant_40" ]
	then
		ht_capab_value="${ht_capab_value}[40-INTOLERANT]"
	fi
	if [ "$ldpc" = "1" ]
	then
		ht_capab_value="${ht_capab_value}${driver_ldpc}"
	fi
	if [ "$auto_coc" = "1" ]
	then
		ht_capab_value="${ht_capab_value}[SMPS-STATIC]"
	fi
	ht_capab_value="${ht_capab_value}[MAX-AMSDU-${max_amsdu}]"

	echo "$ht_capab_value"
}

# Read from driver if beamforming is supported and from rc.conf if explicit beamforming is enabled.
# If so, set the ht_tx_bf_capab flags (flags are hardcoded and cannot be set from web/cli).
convert_ht_tx_bf_capab()
{
	# Define local parameters
	local ap_index interface_name \
	ht_tx_bf_capab_value driver_bf_support ugw_bf_enable comps_bf_ant

	ap_index=$1
	interface_name=$2

	ht_tx_bf_capab_value=""
	driver_bf_support=`iwpriv $interface_name gBfExplicitCap`
	driver_bf_support=`echo ${driver_bf_support##w*:}`

	eval ugw_bf_enable=\${wlphywave_${ap_index}_explicitBfEna}
	if [ "$driver_bf_support" = "1" ] && [ "$ugw_bf_enable" = "1" ]
	then
		# read number of antennas
		eval comps_bf_ant=\${wlcoc_${ap_index}_numAntennas}
		comps_bf_ant=$((comps_bf_ant-1))
		ht_tx_bf_capab_value="[IMPL-TXBF-RX][EXPL-COMPR-STEER][EXPL-COMPR-FB-FBACK-IMM][MIN-GROUP-124][CSI-BF-ANT-1][NONCOMPS-BF-ANT-1][COMPS-BF-ANT-${comps_bf_ant}][CSI-MAX-ROWS-BF-1][CHE-SPACE-TIME-STR-1]"
	fi
	echo "$ht_tx_bf_capab_value"
}

# Read rc.conf values for vht capabilities and create string for hostapd:
# MPDU max length
# RX-LDPC
# VHT Short-GI
# VHT TX STBC
# VHT RX STBC
# Single User beamformer
# Single User beamformee
# Beamformer antenna
# Sounding dimension
# VHT TXOP power save
# A-MPDU max length exponent

convert_vht_capab()
{
	# Define local parameters
	local ap_index interface_name
	local phy_name driver_ldpc driver_tx_stbc driver_rx_stbc driver_no_tx_stbc driver_no_rx_stbc num_tx_antennas \
	mpdu_len vht_capab_value rx_ldpc vht_short_gi vht_tx_stbc vht_rx_stbc \
	su_beamformer su_beamformee beamformer_antenna ugw_beamformer_antenna sounding_dimestion ugw_sounding_dimestion \
	vht_txop_ps ampdu_max_len_exp

	ap_index=$1
	interface_name=$2

	# Read driver capabilities values
	phy_name=`find_phy_from_interface_name $interface_name`
	
	# LDPC
	driver_ldpc=""
	driver_ldpc=`iw phy $phy_name info | grep LDPC`
	[ -n "$driver_ldpc" ] && driver_ldpc="[RXLDPC]"
	
	# TX-STBC
	driver_tx_stbc=""
	driver_rx_stbc=""
	driver_no_tx_stbc=`iw phy $phy_name info | grep "No TX STBC"`
	driver_no_rx_stbc=`iw phy $phy_name info | grep "No RX STBC"`
	[ -z "$driver_no_tx_stbc" ] && driver_tx_stbc="[TX-STBC-2BY1]"
	[ -z "$driver_no_rx_stbc" ] && driver_rx_stbc="[RX-STBC-1]"

	# Number of TX antennas
	num_tx_antennas=`iw phy $phy_name info | grep "Available Antennas"`
	num_tx_antennas=${num_tx_antennas##*TX 0x}
	num_tx_antennas=${num_tx_antennas:0:1}

	# Check value for MPDU max length
	eval mpdu_len=\${wlphy_${ap_index}_vhtMPDUlen}
	vht_capab_value="[MAX-MPDU-${mpdu_len}]"

	# Check value for RX-LDPC.
	eval rx_ldpc=\${wlphy_${ap_index}_vhtLDPCEn}
	[ "$rx_ldpc" = "1" ] && vht_capab_value="${vht_capab_value}${driver_ldpc}"

	# Check value for VHT Short-GI.
	eval vht_short_gi=\${wlphy_${ap_index}_vhtGuardIntvl}
	[ "$vht_short_gi" = "1" ] && vht_capab_value="$vht_capab_value[SHORT-GI-80]"

	# Check value for VHT TX STBC.
	eval vht_tx_stbc=\${wlphy_${ap_index}_vhtSTBCtx}
	[ "$vht_tx_stbc" = "1" ] && vht_capab_value="${vht_capab_value}${driver_tx_stbc}"

	# Check value for VHT RX STBC. Currently, support is for 1 spatial stream.
	eval vht_rx_stbc=\${wlphy_${ap_index}_vhtSTBCrx}
	[ "$vht_rx_stbc" = "1" ] && vht_capab_value="${vht_capab_value}${driver_rx_stbc}"

	# Check value for Single User beamformer.
	eval su_beamformer=\${wlphy_${ap_index}_vhtSUbeamformer}
	[ "$su_beamformer" = "1" ] && vht_capab_value="$vht_capab_value[SU-BEAMFORMER]"

	# Check value for Single User beamformee.
	eval su_beamformee=\${wlphy_${ap_index}_vhtSUbeamformee}
	[ "$su_beamformee" = "1" ] && vht_capab_value="$vht_capab_value[SU-BEAMFORMEE]"

	# Check value for Beamformer antenna if SU-beamformee is supported.
	# Compare between number of TX antennas and values set in rc.conf and set the lowest.
	if [ "$su_beamformee" = "1" ]
	then
		beamformer_antenna=$num_tx_antennas
		eval ugw_beamformer_antenna=\${wlphy_${ap_index}_vhtBfAntenna}
		[ "$ugw_beamformer_antenna" -lt "$beamformer_antenna" ] && beamformer_antenna=$ugw_beamformer_antenna
		vht_capab_value="$vht_capab_value[BF-ANTENNA-${beamformer_antenna}]"
	fi
	# Check value for Sounding dimension if SU-beamformer is supported.
	# Compare between number of TX antennas and values set in rc.conf and set the lowest.
	if [ "$su_beamformer" = "1" ]
	then
		sounding_dimestion=$num_tx_antennas
		eval ugw_sounding_dimestion=\${wlphy_${ap_index}_vhtSoundingDimension}
		[ "$ugw_sounding_dimestion" -lt "$sounding_dimestion" ] && sounding_dimestion=$ugw_sounding_dimestion
		vht_capab_value="$vht_capab_value[SOUNDING-DIMENSION-${sounding_dimestion}]"
	fi

	# Check value for VHT TXOP power save.
	eval vht_txop_ps=\${wlphy_${ap_index}_vhtTXOPpowerSave}
	[ "$su_beamformee" = "1" ] && vht_capab_value="$vht_capab_value[VHT-TXOP-PS]"

	# Check value for Multi User beamformer
	# If interface is Wave500B, set flag ON
	is_wave500=`check_wave500b $interface_name`
	[ "$is_wave500" = "yes" ] && vht_capab_value="$vht_capab_value[MU-BEAMFORMER]"

	# Check value for A-MPDU max length exponent.
	eval ampdu_max_len_exp=\${wlphy_${ap_index}_vhtAMPDUlenExp}
	vht_capab_value="${vht_capab_value}[MAX-A-MPDU-LEN-EXP${ampdu_max_len_exp}]"

	echo "$vht_capab_value"
}

# Get the channel number, channel width and secondary channel (if needed) and find the center channel for VHT.
# For 20MHz, return the channel.
# For 40MHz, check the secondary channel and return channel+2 for secondary upper or channel-2 for secondary lower.
# For 80MHz, return the center channel according to the list:
# 36,40,44,48 - return 42
# 52,56,60,64 - return 58
# 100,104,108,112 - return 106
# 116,120,124,128 - return 122
# 132,136,140,144 - return 138
# 149,153,157,161 - return 155
convert_center_freq()
{
	# Define local parameters
	local channel channel_width secondary_channel center_freq

	channel=$1
	channel_width=$2
	secondary_channel=$3

	case "$channel_width" in
	"$CH_WIDTH_20")
		center_freq="$channel"
	;;
	"$CH_WIDTH_40")
		[ "$secondary_channel" = "$SECONDARY_CHANNEL_UPPER" ] && center_freq=$((channel+2))
		[ "$secondary_channel" = "$SECONDARY_CHANNEL_LOWER" ] && center_freq=$((channel-2))
	;;
	"$CH_WIDTH_80")
		case "$channel" in
		36|40|44|48)
			center_freq=42
		;;
		52|56|60|64)
			center_freq=58
		;;
		100|104|108|112)
			center_freq=106
		;;
		116|120|124|128)
			center_freq=122
		;;
		132|136|140|144)
			center_freq=138
		;;
		149|153|157|161)
			center_freq=155
		;;
		"acs_numbss"|"0")
			center_freq=0
		;;
		esac
	;;
	esac
	echo "$center_freq"
}

# Read rc.conf value for radarSimulationEna.
# If radarSimulationEna is 1, read and set value in radarSimulationChannel
# If radarSimulationEna is 0, set empty value
convert_radar_simulation_debug_channel()
{
	# Define local parameters
	local ap_index
	local radio_simulation_enabled radio_simulation_channel
	
	radio_simulation_channel=""

	eval radio_simulation_enabled=\${wlphywave_${ap_index}_radarSimulationEna}
	if [ "$radio_simulation_enabled" = "1" ]
	then
		eval radio_simulation_channel=\${wlphywave_${ap_index}_radarSimulationChannel}
	fi

	echo "$radio_simulation_channel"
}

# Read rc.conf value for driver debug level and create string to set in proc.
# Driver debug level is set for console (cdebug) and for remote (rdebug)
convert_driver_debug_level()
{
	# Define local parameters
	local ap_index component driver_debug_value driver_debug_level

	ap_index=$1
	component=$2
	driver_debug_value=""

	if [ "$component" = "cdebug" ]
	then
		eval driver_debug_level=\${wlphywave_${ap_index}_driverDebugLevel}
	elif [ "$component" = "rdebug" ]
	then
		eval driver_debug_level=\${wllogwave_${ap_index}_logLevelDriver}
	fi
	[ -n "$driver_debug_level" ] && driver_debug_value="0 $component=$driver_debug_level"
	echo "$driver_debug_value"
}

# Read rc.conf value to check if 20/40 coexistence is enabled and if so, read obss_scan_interval
convert_obss_scan_interval()
{
	# Define local parameters
	local ap_index
	local coex_enable obss_interval

	ap_index=$1

	# Read 20/40 coexistence mode
	eval coex_enable=\${wlphy_${ap_index}_n2040CoexEna}
	if [ "$coex_enable" = "1" ]
	then
		eval obss_interval=\${wlphy_${ap_index}_nCoexObssScanInt}
	else
		obss_interval=0
	fi

	echo "$obss_interval"
}

# Read rc.conf values for the scan parameters and generate the config string.
convert_scan_params()
{
	# Define local parameters
	local ap_index
	local passiveScanTime activeScanTime numProbeReqs probeReqInterval passiveScanValidTime activeScanValidTime

	eval passiveScanTime=\${wlphywave_${ap_index}_passiveScanTime}
	eval activeScanTime=\${wlphywave_${ap_index}_activeScanTime}
	eval numProbeReqs=\${wlphywave_${ap_index}_numProbeReqs}
	eval probeReqInterval=\${wlphywave_${ap_index}_probeReqInterval}
	eval passiveScanValidTime=\${wlphywave_${ap_index}_passiveScanValidTime}
	eval activeScanValidTime=\${wlphywave_${ap_index}_activeScanValidTime}

	echo "$passiveScanTime $activeScanTime $numProbeReqs $probeReqInterval $passiveScanValidTime $activeScanValidTime"
}

# Read rc.conf values for the background scan parameters and generate the config string.
convert_bg_scan_params()
{
	# Define local parameters
	local ap_index
	local passiveScanTimeBG activeScanTimeBG numProbeReqsBG probeReqIntervalBG numChansInChunkBG chanChunkIntervalBG

	eval passiveScanTimeBG=\${wlphywave_${ap_index}_passiveScanTimeBG}
	eval activeScanTimeBG=\${wlphywave_${ap_index}_activeScanTimeBG}
	eval numProbeReqsBG=\${wlphywave_${ap_index}_numProbeReqsBG}
	eval probeReqIntervalBG=\${wlphywave_${ap_index}_probeReqIntervalBG}
	eval numChansInChunkBG=\${wlphywave_${ap_index}_numChansInChunkBG}
	eval chanChunkIntervalBG=\${wlphywave_${ap_index}_chanChunkIntervalBG}

	echo "$passiveScanTimeBG $activeScanTimeBG $numProbeReqsBG $probeReqIntervalBG $numChansInChunkBG $chanChunkIntervalBG"
}

# Read rc.conf values for the calibration channel width for scan.
convert_calibration_chan_width_for_scan()
{
	# Define local parameters
	local ap_index
	local chanWidthMask2 chanWidthMask5

	eval chanWidthMask2=\${wlphywave_${ap_index}_chanWidthMask2}
	eval chanWidthMask5=\${wlphywave_${ap_index}_chanWidthMask5}

	echo "$chanWidthMask2 $chanWidthMask5"
}

# Read rc.conf FW recovery parameters and generate FW recovery setting.
convert_fw_recovery()
{
	# Define local parameters
	local ap_index
	local fast_recovery_enabled fast_recovery_limit full_recovery_enabled full_recovery_limit complete_recovery_enabled recovery_timer auto_recover_dumps_enabled auto_recovery_dumps_limit fw_recovery

	ap_index=$1

	eval fast_recovery_enabled=\${wlphywave_${ap_index}_fwFastRecoverEna}
	if [ "$fast_recovery_enabled" = "0" ]
	then
		fw_recovery="$fast_recovery_enabled"
	else
		eval fast_recovery_limit=\${wlphywave_${ap_index}_fwFastRecoverLim}
		fw_recovery="$fast_recovery_limit"
	fi
	eval full_recovery_enabled=\${wlphywave_${ap_index}_fwFullRecoverEna}
	if [ "$full_recovery_enabled" = "0" ]
	then
		fw_recovery="${fw_recovery} ${full_recovery_enabled}"
	else
		eval full_recovery_limit=\${wlphywave_${ap_index}_fwFullRecoverLim}
		fw_recovery="${fw_recovery} ${full_recovery_limit}"
	fi
	eval complete_recovery_enabled=\${wlphywave_${ap_index}_fwCompleteRecoverEna}
	eval recovery_timer=\${wlphywave_${ap_index}_fwRecoverTimer}
	eval auto_recover_dumps_enabled=\${wlphywave_${ap_index}_fwAutoRecoverDumpEna}
	if [ "$auto_recover_dumps_enabled" = "0" ]
	then
		auto_recovery_dumps_limit=0
	else
		eval auto_recovery_dumps_limit=\${wlphywave_${ap_index}_fwAutoRecoverDumpLim}
	fi

	fw_recovery="${fw_recovery} ${complete_recovery_enabled} ${recovery_timer} ${auto_recovery_dumps_limit}"
	echo "$fw_recovery"
}

convert_num_msdu_in_amsdu()
{
	# Define local parameters
	local ap_index num_msdu_in_amsdu ht_num_msdu vht_num_msdu
	
	ap_index=$1

	num_msdu_in_amsdu=""
	eval ht_num_msdu=\${wlphywave_${ap_index}_htNumMsdusInAmsdu}
	eval vht_num_msdu=\${wlphywave_${ap_index}_vhtNumMsdusInAmsdu}

	num_msdu_in_amsdu="$ht_num_msdu $vht_num_msdu"

	echo "$num_msdu_in_amsdu"
}

convert_txop_enbale()
{
	# Define local parameters
	local ap_index do_simple_cli

	ap_index=$1

	eval do_simple_cli=\${wlphywave_${ap_index}_txopEna}

	echo "33 $do_simple_cli"
}

# Read rc.conf WEP key and convert it from hexdump to ascii.
convert_wep_key()
{
	# Define local parameters
	local ap_index
	local key_index cpeid wep_key

	ap_index=$1
	key_index=$2
	cpeid=$3

	eval wep_key=\${wlwep${cpeid}_${key_index}_key}
	wep_key=$(printf "%b" "$wep_key")
	echo "$wep_key"
}

# Read rc.conf beacon type and set hostapd wpa value.
# rc.conf values: 0=basic, 1=wpa, 2=wpa2, 3=wpa-wpa2 (not compliant), 4=wpa/wpa2 (compliant).
# hostapd values: 0=Open/WEP, 1=wpa, 2=wpa2, 3=wpa-wpa2.
convert_wpa()
{
	# Define local parameters
	local beacon_type wpa

	beacon_type=$1

	wpa=$beacon_type
	# Convert beacon_type 4 to mixed mode (3).
	[ "$beacon_type" = "$BEACON_WPA_WPA2_COMPLIANT" ] && wpa=3
	echo "$wpa"
}

# Get values from rc.conf of auth_type, beacon_type, ecnr_type, pmf_enabled and pmf_required.
# The wpa_key_mgmt is set to WPA-PSK for personal and WPA-EAP for Radius.
# If security is WPA2-CCMP and PMF is enabled and required, the suffix -SHA256 is added to the wpa_key_mgmt.
convert_wpa_key_mgmt()
{
	# Define local parameters
	local auth_type beacon_type	encr_type pmf_enabled pmf_required wpa_key_mgmt

	auth_type=$1
	beacon_type=$2
	encr_type=$3
	pmf_enabled=$4
	pmf_required=$5

	# Init parameter as personal WPA
	wpa_key_mgmt=WPA-PSK
	# RADIUS is set
	[ "$auth_type" = "$AUTH_RADIUS" ] && wpa_key_mgmt=WPA-EAP
	# PMF is set as required
	[ "$beacon_type" = "$BEACON_WPA2" ] && [ "$encr_type" = "$ENCR_CCMP" ] && [ "$pmf_enabled" = "1" ] && [ "$pmf_required" = "1" ] && wpa_key_mgmt=$wpa_key_mgmt-SHA256
	echo "$wpa_key_mgmt"
}

# Convert auth from rc.conf values to hostapd values
# authType values in rc.conf: 0=open, 1=shared, 2=radius, 3=psk
# auth_algs values in hostapd: 1=open, 2=shared. (all is "open", except for "shared").
convert_auth_algs()
{
	# Define local parameters
	local auth_type auth_algs

	auth_type=$1
	auth_algs=1

	[ "$auth_type" = "$AUTH_SHARED" ] && auth_algs=2
	echo "$auth_algs"
}

# Read rc.conf value for authType and set eap_server:. eap_server value 1 is for all auth types, value 0 is for Radius.
# hostapd value 0 = external authenticator (RADIUS).
# hostapd value 1 = internal authenticator
convert_eap_server()
{
	# Define local parameters
	local auth_type eap_server

	auth_type=$1
	eap_server=1

	[ "$auth_type" = "$AUTH_RADIUS" ] && eap_server=0
	echo "$eap_server"
}

# Convert the encryption type from rc.conf value to hostapd wpa_pairwise
convert_wpa_pairwise()
{
	# Define local parameters
	local encr_type wpa_pairwise

	encr_type=$1

	case $encr_type in
	$ENCR_TKIP)
		wpa_pairwise="TKIP"
	;;
	$ENCR_CCMP)
		wpa_pairwise="CCMP"
	;;
	$ENCR_TKIP_CCMP)
		wpa_pairwise="TKIP CCMP"
	;;
	esac
	echo "$wpa_pairwise"
}

# Check in rc.conf if WPS is enabled.
# if WPS is enabled, check if AP is configured or not
# hostapd values: 0=disabled, 1=enabled, AP un-configured, 2=enabled, AP configured.
convert_wps_state()
{
	# Define local parameters
	local cpeid wps_enable wps_config_state

	cpeid=$1

	eval wps_enable=\${wlwps${cpeid}_0_enable}
	# WPS is disabled
	[ "$wps_enable" = "0" ] && echo 0 && return
	# WPS is enabled, set hostapd value according to AP state (configured/un-configured).
	if [ "$wps_enable" = "1" ]
	then
		eval wps_config_state=\${wlwps${cpeid}_0_cfgState}
		echo $wps_config_state
	fi
}

# Read rc.conf value for ap_setup_locked and convert to hostapd value.
# ap_setup_locked values in rc.conf and hostapd are 0->1 and 1->0
convert_ap_setup_locked()
{
	# Define local parameters
	local cpeid ap_setup_locked

	cpeid=$1

	eval ap_setup_locked=\${wlwps${cpeid}_0_proxyEna}
	ap_setup_locked=$((ap_setup_locked^1))
	echo $ap_setup_locked
}

# Read rc.conf frequency and decide wps_rf_bands.
convert_wps_rf_bands()
{
	# Define local parameters
	local ap_index
	local freq_band

	ap_index=$1

	eval freq_band=\${wlphy_${ap_index}_freqBand}
	if [ "$freq_band" = "0" ]
	then
		echo "g"
	else
		echo "a"
	fi
}

# Set the WDS ap key index.
# When no WDS security is set, key index is 0
# When WDS security is set, read the key index from rc.conf and set value+1 to driver.
convert_peer_ap_key_index()
{
	# Define local parameters
	local ap_index
	local wds_encrType key_index

	ap_index=$1

	eval wds_encrType=\${wlwds_${ap_index}_encrType}
	if [ "$wds_encrType" == "0" ]
	then
		key_index="0"
	else
		eval key_index=\${wlwds_${ap_index}_peerAPKeyIdx}
		key_index=$((key_index+1))
	fi
	echo "$key_index"
}

# Set the WEP keys when configuring WDS.
# Read the WEP keys and the wep key type (ascii/hex) from rc.conf.
# If the key is ascii, it will be set as "iwconfig key [key index] s:"<KEY VALUE>"
# If the key is hex, it will be set "iwconfig key [key index] <KEY VALUE>
# The function creates a single string to set all the keys: "[01] <KEY1 VALUE> key [02] <KEY2 VALUE> key [03] <KEY3 VALUE> key [04] <KEY4 VALUE>"
convert_wds_wep_keys()
{
	# Define local parameters
	local ap_index
	local key_type wep_key add_wep_key iwconfig_key_index cpeid

	ap_index=$1

	eval key_type=\${wlsec_${ap_index}_wepKeyType}
	eval cpeid=\${wlmn_${ap_index}_cpeId}
	wep_key="[1]"
	add_wep_key=`convert_wep_key $ap_index 0 $cpeid`
	if [ "$key_type" = "$WEP_ASCII" ]
	then
		wep_key="$wep_key s:\"$add_wep_key\""
	else
		wep_key="$wep_key $add_wep_key"
	fi
	for key_index in 1 2 3
	do
		add_wep_key=`convert_wep_key $ap_index $key_index $cpeid`
		iwconfig_key_index=$((key_index+1))
		if [ "$key_type" = "$WEP_ASCII" ]
		then
			wep_key="$wep_key key [$iwconfig_key_index] s:\"$add_wep_key\""
		else
			wep_key="$wep_key key [$iwconfig_key_index] $add_wep_key"
		fi
	done
	echo "$wep_key"
}

# Update the WDS AP peer APs list.
# Generate 2 new parameters: list of MACs in rc.conf and list of MACs in driver.
# Go over the list of MACs from driver:
# For each MAC, try to "remove" it from rc.conf list. If rc.conf list after "removal" is same as before, the MAC doesn't exist in rc.conf and needs to be deleted from driver (using iwpriv sDelPeerAP).
##########
# Example:
# rc.conf list: 11:11:11:11:11:11 22:22:22:22:22:22
# driver list: 11:11:11:11:11:11 33:33:33:33:33:33
# When we try to "remove" 11:11:11:11:11:11 from rc.conf list, we get new list and nothing needs to be done.
# When we try to "remove" 33:33:33:33:33:33 from rc.conf list, we get the same list, hence this MAC doesn't appear in rc.conf and needs to be removed from driver.
##########
# Go over the list of MACs from rc.conf and add them to driver (using iwpriv sAddPeerAP, when adding an existing MAC, nothing changes).
update_wds_peer_ap_list()
{
	# Define local parameters
	local ap_index pid interface_name
	local driver_peer_aps rc_conf_peer_aps

	ap_index=$1
	pid=$2
	interface_name=$3
	driver_peer_aps=$4
	rc_conf_peer_aps=$5

	# Go over MACs in driver and see if a MAC appears in rc.conf. If not, delete this MAC from driver
	for driver_mac in $driver_peer_aps
	do
		[ "$rc_conf_peer_aps" = "${rc_conf_peer_aps/$driver_mac/}" ] && set_conf_param $DRIVER_SINGLE_CALL_CONFIG_FILE iwpriv otf $pid $interface_name sDelPeerAP "$driver_mac"
	done

	# Go over list of MACs in rc.conf and add them to driver
	for rc_conf_mac in $rc_conf_peer_aps
	do
		set_conf_param drv_config_post_up iwpriv otf $pid $interface_name sAddPeerAP "$rc_conf_mac"
	done
}

# Generate the list of WDS MACs from rc.conf.
# Go over all the MACs and add only those of the current interface.
get_wds_rc_conf_peer_list()
{
	# Define local parameters
	local ap_index
	local i rc_conf_macs pcpeid cur_pcpeid peer_ap_mac

	ap_index=$1
	i=0
	rc_conf_macs=""

	eval pcpeid=\${wlwds_${ap_index}_pcpeId}
	while [ $i -lt $wlan_wds_macs_Count ]
	do
		eval cur_pcpeid=\${wlwdsmac_${i}_pcpeId}
		if [ "$cur_pcpeid" = "$pcpeid" ]
		then
			eval peer_ap_mac=\${wlwdsmac_${i}_peerAP}
			rc_conf_macs="$rc_conf_macs $peer_ap_mac"
		fi
		i=$((i+1))
	done

	# Remove leading spaces from list.
	rc_conf_macs=`echo $rc_conf_macs`
	echo "$rc_conf_macs"
}

# Generate list of MACs from driver.
get_wds_driver_peer_list()
{
	# Define local parameters
	local ap_index interface_name
	local driver_peer_aps driver_wds_status

	ap_index=$1
	interface_name=$2
	driver_peer_aps=""

	# Read the peers list only if WDS is enabled in the driver.
	# Extract the BridgeMode value from the output
	driver_wds_status=`iwpriv $interface_name gBridgeMode 2>/dev/null`
	driver_wds_status=${driver_wds_status##*:}
	[ -n "$driver_wds_status" ] && [ $driver_wds_status -eq 1 ] && driver_peer_aps=`iwpriv $interface_name gPeerAPs`

	# Remove prefix from MACs list (list is in the format of "wlan0 gPeerAPs:" followed by the MACs) and remove leading spaces from list.
	driver_peer_aps=${driver_peer_aps#*:}
	driver_peer_aps=`echo $driver_peer_aps`
	# If the list is empty, the text "No any address" appears.
	[ "$driver_peer_aps" = "No any address" ] && driver_peer_aps=""
	echo "$driver_peer_aps"
}
LIB_CONVERT_SOURCED="1"
