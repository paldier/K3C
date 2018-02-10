#!/bin/sh

# Common paths and files names
export CONFIGS_PATH=/ramdisk/flash
export ETC_PATH=/etc/rc.d
export BINDIR=/bin
export DRIVER_PATH=/lib/modules/$(uname -r)/net
export GENL_FAMILY_ID_FILE=/tmp/mtlk_genl_family_id_file
export IMAGES_PATH=/root/mtlk/images
export RESTART_FLAG=wave_wlan_need_restart
export HOSTAPD_EVENTS_SCRIPT=$ETC_PATH/wave_wlan_events_hostapd.sh

# Configuration files defines
export CONF_DIR=/tmp/wave_conf
export TEMP_CONF_DIR=/tmp/wave_temp
export HOSTAPD_PHY_CONF_PREFIX=hostapd_phy
export HOSTAPD_VAP_CONF_PREFIX=hostapd_vap
export DRIVER_PRE_UP_CONF_PREFIX=drv_config_pre_up
export DRIVER_POST_UP_CONF_PREFIX=drv_config_post_up
export DRIVER_SINGLE_CALL_CONFIG_FILE=drv_config_single_call
export OTF_CONFIG_FILE=wave_wlan_otf_config.conf
export WAVE_WLAN_RUNNNER=wave_wlan_runner.sh
export CRDA_FLAG=${CONF_DIR}/crda_executed

# List of features need to be configured for AP
export FULL_FEATURES_LIST_0="radio_modify main_modify sec_modify wps_modify wmm_modify wds_modify mac_ctrl_modify hs20_modify"
# List of features need to be configured for VAP
export FULL_FEATURES_LIST_1="main_modify sec_modify wmm_modify wds_modify mac_ctrl_modify hs20_modify"

# Common parameters defines
export AP=0
export VAP=1
export STA=2

export FREQ_24G=0
export FREQ_5G=1
export FREQ_BOTH=2

export MODE_11BG=0
export MODE_11A=1
export MODE_11B=2
export MODE_11G=3
export MODE_11N=4
export MODE_11BGN=5
export MODE_11GN=6
export MODE_11AN=7
export MODE_11AC=8
export MODE_11NAC=9
export MODE_11ANAC=10

export CH_WIDTH_20=0
export CH_WIDTH_40=1
export CH_WIDTH_AUTO=2
export CH_WIDTH_80=3

export SECONDARY_CHANNEL_UPPER=0
export SECONDARY_CHANNEL_LOWER=1

export ACL_ACCEPT=0
export ACL_DENY=1
export ACL_DISABLED=2
export ACCEPT_ACL_FILE=hostapd.accept
export DENY_ACL_FILE=hostapd.deny

export AUTH_OPEN=0
export AUTH_SHARED=1
export AUTH_RADIUS=2
export AUTH_PSK=3

export BEACON_BASIC=0
export BEACON_WPA=1
export BEACON_WPA2=2
export BEACON_WPA_WPA2_NOT_COMPLIANT=3
export BEACON_WPA_WPA2_COMPLIANT=4
export BEACON_OSEN=8

export ENCR_NONE=0
export ENCR_WEP=1
export ENCR_TKIP=2
export ENCR_CCMP=3
export ENCR_TKIP_CCMP=4

export HOSTAPD_WPA=1
export HOSTAPD_WPA2=2
export HOSTAPD_WPA_MIXED=3

export WEP_ASCII=0
export WEP_HEX=1

export PMF_DISABLED=0
export PMF_ENABLED=1
export PMF_REQUIRED=2

export WPS_DISABLED=0
export WPS_ENABLED_NOT_CONFIGURED=1
export WPS_ENABLED_CONFIGURED=2

export USE_WAVE_CONF=NO

# Logger parameters
export LOGGER_FW=0
export LOGGER_DRIVER=1
export LOGGER_CONFIGURATIONS=2
export LOGGER_HOSTAPD=3
#export LOGGER_PROC=/proc/net/rt_logger/rtlog
export LOGGER_PROC=/proc/net/mtlk_log/rtlog
export LOGGER_LAN=0
export LOGGER_WAN=1

# HS2.0 parameters:
export HS20_MODE_DISABLED=0
export HS20_MODE_ENABLED=1
export HS20_MODE_OSEN=2
export PARP_CTRL_SCRIPT="$ETC_PATH/wave_wifi_parp_ctrl.sh"
export DGAF_DISABLE_SCRIPT="$ETC_PATH/wave_wifi_dgaf_disable.sh"
export WMDCTRL_SCRIPT="$ETC_PATH/wmdctrl.sh"
export HAIRPIN_CONFIG_SCRIPT="$ETC_PATH/wave_wifi_hairpin_config.sh"
export L2F_CTRL_SCRIPT="$ETC_PATH/wave_wifi_l2f_ctrl.sh"


# Print scripts logs.
# Possible log types: ERROR, WARNNING, DEBUG, INFO or any other string.
# Output depends on the log level set in rc.conf:
# 	0 = print only errors and any string that is not ERROR, WARNNING, DEBUG or INFO.
#	1 = print errors, warnings and any string that is not ERROR, WARNNING, DEBUG or INFO.
#	2 = print errors, warnings, debug and any string that is not ERROR, WARNNING, DEBUG or INFO.
#	3 = print all outputs.
# This function can only be called after rc.conf was sourced by the calling script.
print2log()
{
	# Define local parameters
	local ap_index
	local log_type msg radio_cpe_id log_level log_output

	ap_index=$1
	log_type=$2
	msg=$3

	eval radio_cpe_id=\${wlmn_${ap_index}_radioCpeId}
	radio_cpe_id=$((radio_cpe_id-1))
	eval log_level=\${wlphywave_${radio_cpe_id}_scriptsDebugLevel}
	eval log_output=\${wlphywave_${radio_cpe_id}_scriptsDebugOutput}

	case $log_type in
	ERROR)
		[ $log_level -ge 0 ] && echo "$log_type $msg" > $log_output
	;;
	WARNNING)
		[ $log_level -ge 1 ] && echo "$log_type $msg" > $log_output
	;;
	DEBUG)
		[ $log_level -ge 2 ] && echo "$log_type $msg" > $log_output
	;;
	INFO)
		[ $log_level -ge 3 ] && echo "$log_type $msg" > $log_output
	;;
	*)
		echo "$log_type $msg" > /dev/console
	;;
	esac
}

# Timestamp function for profiling.
# Results added to: /tmp/wlanprofiling.log
# This function can only be called after rc.conf was sourced by the calling script.
timestamp()
{
	# Define local parameters
	local ap_index
	local prefix radio_cpe_id wlan_profiling seconds

	ap_index=$1
	prefix=$2

	eval radio_cpe_id=\${wlmn_${ap_index}_radioCpeId}
	radio_cpe_id=$((radio_cpe_id-1))
	eval wlan_profiling=\${wlphywave_${radio_cpe_id}_profilingDebug}

	[ "$wlan_profiling" = "0" ] && return

	seconds=`date +%s`
	echo ${prefix}${seconds} >> /tmp/wlanprofiling.log
}

# Create a copy of rc.conf and source it.
rc_conf_source()
{
	rc_conf_temp=$TEMP_CONF_DIR/rc.conf_wave_$$
	/usr/sbin/syscfg_lock $CONFIGS_PATH/rc.conf "
	`grep "^wl\|^gbc\|^lan_main\|^lan_dhcps\|^vlan_ch_cfg\|^auto_detect_cfg\|^wan_phy_cfg\|^default_wan_iface\|^qos_queuemgmt" $CONFIGS_PATH/rc.conf > $rc_conf_temp`
	"
	. $rc_conf_temp

	[ "$USE_WAVE_CONF" = "YES" ] && . $CONFIGS_PATH/wave.conf

	# Delete temporary copy of rc.conf.
	rm $rc_conf_temp
	# Save flag indicating rc.conf is sourced.
	RC_CONF_SOURCED="1"
}

# Return the supported frequencies by an interface
# Function gets the iw output file to parse for the supported frequencies
get_supported_frequencies()
{
	# Define local parameters
	local iw_output iw_frequencies freq_24 freq_5 f f1 supported_frequencies
	
	iw_output=$1
	
	iw_frequencies=`grep "MHz" $iw_output | sed '/MHz/s/ MHz.*//'`
	freq_24=""
	freq_5=""
	for f in $iw_frequencies
	do
		f1=${f:0:1}
		[ "$f1" = "2" ] && freq_24="yes"
		[ "$f1" = "5" ] && freq_5="yes"
	done
	
	if [ "$freq_24" ]
	then
		if [ "$freq_5" ]
		then
			supported_frequencies=$FREQ_BOTH
		else
			supported_frequencies=$FREQ_24G
		fi
	elif [ "$freq_5" ]
	then
		supported_frequencies=$FREQ_5G
	fi
	
	echo "$supported_frequencies"
}

# Convert the frequency index to the frequency string
freq_index_to_str()
{
	# Define local parameters
	local freq_index freq_str
	
	freq_index=$1
	freq_str=""
	
	if [ "$freq_index" = "$FREQ_24G" ]
	then
		freq_str="2.4GHz"
	elif [ "$freq_index" = "$FREQ_5G" ]
	then
		freq_str="5GHz"
	fi
	echo "$freq_str"
}

# Find the name of the physical interface to which the requested interface belongs.
# This function can only be called after rc.conf was sourced by the calling script.
find_pap_name_from_index()
{
	# Define local parameters
	local ap_index interface_name pap_name

	ap_index=$1
	eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
	pap_name=${interface_name%%.*}
	echo $pap_name
}

# Find the ap index of the requested interface.
find_index_from_wave_if()
{
	# Define local parameters
	local interface_name
	local cur_name i

	interface_name=$1

	# Go over the interfaces names, find the interface with the requested name and get the ap index of this interface.
	# If the interface name wasn't found, an error will be printed.
	i=0
	eval cur_name=\$wlmnwave_${i}_interfaceName
	while [ "$cur_name" ] && [ "$cur_name" != "$interface_name" ]
	do
		i=$((i+1))
		eval cur_name=\$wlmnwave_${i}_interfaceName
	done
	if [ -z "$cur_name" ]
	then
		print2log 0 ERROR "interface name $interface_name wasn't found"
		i=""
	fi
	echo $i
}

# Get the phy name in iw for the interface
find_phy_from_interface_name()
{
	# Define local parameters
	local interface_name
	local phy_name
	
	interface_name=$1
	phy_name=`iw dev $interface_name info`
	phy_name=${phy_name##*wiphy }
	phy_name=phy${phy_name}
	echo $phy_name
}

# Check if need to write command to OTF file or mark the restart file.
check_and_write_to_otf_file()
{
	# Define local parameters
	local interface_name
	local otf command pap_name

	otf=$1
	command=$2
	interface_name=$3
	
	if [ "$otf" = "otf" ]
	then
		echo "$command" >> $TEMP_CONF_DIR/$OTF_CONFIG_FILE
	else
		# Parameter is not OTF, write the radio name to the restart flag
		pap_name=${interface_name%%.*}
		echo "restart_${pap_name}=yes" >> /tmp/$RESTART_FLAG
	fi
}

# Write configuration to hostapd conf files.
set_hostapd_param()
{
	# Define local parameters
	local interface_name
	local conf_file_path command_type otf param_name value

	conf_file_path=$1
	command_type=$2
	otf=$3
	interface_name=$4
	param_name=$5
	value=$6

	# If the value is empty, nothing needs to be set.
	[ -z "$value" ] && return

	# If it is a comment, write comment to the conf file.
	[ "$command_type" = "comment" ] && echo "###$value" >> $conf_file_path && return
	# Write parameter to conf file.
	echo "$param_name=$value" >> $conf_file_path

	# TODO: update the hostapd_cli command once hostapd OTF is available.
	check_and_write_to_otf_file $otf "#hostapd_cli $param_name $value" $interface_name
}

# Write wireless extensions API commands (iwconfig/iwpriv) to driver conf files.
set_drv_param()
{
	# Define local parameters
	local interface_name
	local conf_file_path command_type otf param_name value

	conf_file_path=$1
	command_type=$2
	otf=$3
	interface_name=$4
	param_name=$5
	value=$6

	# If the value is empty, nothing needs to be set.
	[ -z "$value" ] && return

	# Since driver configuration files are created per configuration, no check if line exists is needed
	# each line will be written at the end of the file.
	if [ "$command_type" = "proc" ]
	then
		echo "echo $value > $param_name" >> $conf_file_path
		check_and_write_to_otf_file $otf "echo $value > $param_name" $interface_name
	elif [ "$command_type" = "ppa" ]
	then
		# For PPA the value can be add to add the VAP to PPA or remove to delete VAP from PPA.
		# Call function to add the needed commands to the driver post_up conf file and OTF conf file.
		set_ppa_commands $interface_name $value $TEMP_CONF_DIR/$OTF_CONFIG_FILE
		set_ppa_commands $interface_name $value $conf_file_path
	else
		echo "$command_type $interface_name $param_name $value" >> $conf_file_path
		check_and_write_to_otf_file $otf "$command_type $interface_name $param_name $value" $interface_name
	fi
}

# Write configuration to drvhlpr conf file.
set_drvhlpr_param()
{
	# Define local parameters
	local interface_name
	local conf_file_path command_type otf param_name value

	conf_file_path=$1
	command_type=$2
	otf=$3
	interface_name=$4
	param_name=$5
	value=$6

	# If the value is empty, nothing needs to be set.
	[ -z "$value" ] && return

	# Write parameter to conf file.
	echo "$param_name = $value" >> $conf_file_path

	check_and_write_to_otf_file $otf "$param_name = $value" $interface_name
}

# Write configuration to the runner directly
set_runner_param()
{
	# Define local parameters
	local interface_name
	local conf_file_path command_type otf param_name value

	conf_file_path=$1
	command_type=$2
	otf=$3
	interface_name=$4
	param_name=$5
	value=$6

	# If the value is empty, nothing needs to be set.
	[ -z "$value" ] && return

	# each line will be written at the end of the file.
	if [ "$command_type" = "proc" ]
	then
		echo "echo $value > $param_name" >> $conf_file_path
	elif [ "$command_type" = "alumnus" ]
	then
		echo "$param_name $value" >> $conf_file_path
	else
		echo "$command_type $interface_name $param_name $value" >> $conf_file_path
	fi
}

# Write a line in the needed configuration files.
# Parameters:
# conf_file - the configuration file type
#		hostapd_phy
#		hostapd_vap
#		drv_config_pre_up
#		drv_config_post_up
#		drvhlpr
#		runner file name (according to the parameter WAVE_WLAN_RUNNNER)
# command_type -
# 		regular - use name=value or name = value format for the configuration file.
#		iwpriv - use iwpriv command in the driver configuration file.
# 		iwconfig - use iwconfig command in the driver configuration file.
#		proc - use proc command in the driver configuration file.
#		ppa - set the needed ppa commands in the driver configuration file.
#		comment - add new comment line to the hostapd configuration file.
#		alumnus - add execution line for alumnus script
# otf - flag if the parameter is on-the-fly.
# pid - the pid used in the temporary configuration file name.
# interface_name - name of the interface.
# param_name - parameter name to add/modify
# value - value of the parameter to write.
set_conf_param()
{
	# Define local parameters
	local pid interface_name
	local conf_file command_type otf param_name value conf_file_prefix conf_file_path

	conf_file=$1
	command_type=$2
	otf=$3
	pid=$4
	interface_name=$5
	param_name=$6
	# Using "echo" to clear leading spaces.
	value=`echo $7`

	# Get the prefix of name of the configuration before the underscore.
	conf_file_prefix=${conf_file%%_*}

	# Generate the full name of the configuration file to write to.
	conf_file_path=${TEMP_CONF_DIR}/${conf_file}_${interface_name}_${pid}.conf

	# Set the parameters when the conf_file is the runner
	if [ "$conf_file" = "$WAVE_WLAN_RUNNNER" ]
	then
		conf_file_prefix="runner"
		conf_file_path=${CONF_DIR}/${WAVE_WLAN_RUNNNER}
	fi
	# Call the function for the current conf file.
	set_${conf_file_prefix}_param $conf_file_path $command_type $otf $interface_name "$param_name" "$value"
}

# Lock a file and perform copy operation while locked.
lock_and_copy_conf_file()
{
	# Define local parameters
	local locking_file source_file destination_file

	locking_file=$1
	source_file=$2
	destination_file=$3

	# If the source file doesn't exist, create empty new destination file.
	if [ ! -e "$source_file" ]
	then
		touch $destination_file
	# If the locking file doesn't exist, copy files without lock.
	elif [ ! -e "$locking_file" ]
	then
		cp $source_file $destination_file
	else
	# Locking and source files exist, copy files with lock.
		/usr/sbin/syscfg_lock $locking_file "
		cp $source_file $destination_file
		"
	fi
}

# Calculate and update the MAC address of the interface.
update_mac_address()
{
	# Define local parameters
	local ap_index interface_name pap_name
	local ap_type vap_index phy_offset board_mac vap_increment mac_address

	ap_index=$1
	interface_name=$2
	ap_type=$3

	pap_name=${interface_name%%.*}
	if [ "$ap_type" = "$VAP" ]
	then
		vap_index=${interface_name##*.}
		vap_index=$((vap_index+1))
	else
		vap_index=0
	fi
	[ "$pap_name" = "wlan0" ] && phy_offset=12
	[ "$pap_name" = "wlan1" ] && phy_offset=28
	board_mac=`/usr/sbin/upgrade mac_get 0`
	vap_increment=$((phy_offset+vap_index))
	mac_address=`echo $board_mac | /usr/sbin/next_macaddr $vap_increment`
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_main" "wlmn_${ap_index}_bssid" "$mac_address" > /dev/null
	echo "$mac_address"
}

# Create empty temporary configuration files to be used by the requesting wave_wlan_prepare.sh script.
create_empty_temp_conf_files()
{
	# Define local parameters
	local interface_name pid
	local ap_type

	interface_name=$1
	ap_type=$2
	pid=$3

	# Create temporary configuration files for AP.
	if [ "$ap_type" = "$AP" ]
	then
		touch ${TEMP_CONF_DIR}/${HOSTAPD_PHY_CONF_PREFIX}_${interface_name}_${pid}.conf
		touch ${TEMP_CONF_DIR}/${DRIVER_PRE_UP_CONF_PREFIX}_${interface_name}_${pid}.conf
		touch ${TEMP_CONF_DIR}/drvhlpr_${interface_name}_${pid}.conf
	fi

	# Create temporary configuration files for AP and VAP.
	touch ${TEMP_CONF_DIR}/${HOSTAPD_VAP_CONF_PREFIX}_${interface_name}_${pid}.conf
	touch ${TEMP_CONF_DIR}/${DRIVER_POST_UP_CONF_PREFIX}_${interface_name}_${pid}.conf
	touch ${TEMP_CONF_DIR}/${DRIVER_SINGLE_CALL_CONFIG_FILE}_${interface_name}_${pid}.conf
}

# Write initial content to the temp conf files.
# First do safe copy of existing conf files. If hostapd conf files are empty, write initial values to temp conf files.
write_initial_content()
{
	# Define local parameters
	local interface_name pid ap_index
	local ap_type hostapd_phy_conf_temp hostapd_vap_conf_temp bssid

	interface_name=$1
	ap_type=$2
	pid=$3
	ap_index=$4

	if [ "$ap_type" = "$AP" ]
	then
		# Copy existing hostapd_phy conf file to the temp conf file.
		hostapd_phy_conf_temp=${TEMP_CONF_DIR}/${HOSTAPD_PHY_CONF_PREFIX}_${interface_name}_${pid}.conf
		lock_and_copy_conf_file ${TEMP_CONF_DIR}/${HOSTAPD_PHY_CONF_PREFIX}_${interface_name}.conf ${TEMP_CONF_DIR}/${HOSTAPD_PHY_CONF_PREFIX}_${interface_name}.conf $hostapd_phy_conf_temp

		# If the hostapd_phy conf file is empty, write initial values
		if [ ! -s $hostapd_phy_conf_temp ]
		then
			echo "################ Physical radio parameters ################" > $hostapd_phy_conf_temp
			set_conf_param hostapd_phy regular no_otf $pid $interface_name interface $interface_name
			set_conf_param hostapd_phy regular no_otf $pid $interface_name driver nl80211
			set_conf_param hostapd_phy regular no_otf $pid $interface_name logger_syslog_level 3
			set_conf_param hostapd_phy regular no_otf $pid $interface_name ctrl_interface /var/run/hostapd
			set_conf_param hostapd_phy regular no_otf $pid $interface_name ctrl_interface_group 0
		fi

		# Copy existing drvhlpr conf file to the temp conf file.
		lock_and_copy_conf_file ${TEMP_CONF_DIR}/drvhlpr_${interface_name}.conf ${TEMP_CONF_DIR}/drvhlpr_${interface_name}.conf ${TEMP_CONF_DIR}/drvhlpr_${interface_name}_${pid}.conf
	fi

	# Copy existing driver post-up conf file to the temp conf file.
	lock_and_copy_conf_file ${TEMP_CONF_DIR}/${DRIVER_POST_UP_CONF_PREFIX}_${interface_name}.conf ${TEMP_CONF_DIR}/${DRIVER_POST_UP_CONF_PREFIX}_${interface_name}.conf ${TEMP_CONF_DIR}/${DRIVER_POST_UP_CONF_PREFIX}_${interface_name}_${pid}.conf

	# Copy existing hostapd_vap conf file to the temp conf file.
	hostapd_vap_conf_temp=${TEMP_CONF_DIR}/${HOSTAPD_VAP_CONF_PREFIX}_${interface_name}_${pid}.conf
	lock_and_copy_conf_file ${TEMP_CONF_DIR}/${HOSTAPD_VAP_CONF_PREFIX}_${interface_name}.conf ${TEMP_CONF_DIR}/${HOSTAPD_VAP_CONF_PREFIX}_${interface_name}.conf $hostapd_vap_conf_temp

	# If the hostapd_vap conf file is empty, write initial values
	if [ ! -s $hostapd_vap_conf_temp ]
	then
		echo "############## $interface_name VAP parameters #############" > $hostapd_vap_conf_temp
		[ "$ap_type" = "$VAP" ] && set_conf_param hostapd_vap regular no_otf $pid $interface_name bss $interface_name
		# Read bssid from rc.conf and set if needed.
		eval bssid=\${wlmn_${ap_index}_bssid}
		if [ -z "$bssid" ] || [ "$bssid" = "00:00:00:00:00:00" ]
		then
			bssid=`update_mac_address $ap_index $interface_name $ap_type`
		fi
		set_conf_param hostapd_vap regular no_otf $pid $interface_name bssid "$bssid"
		# Write vendor element to advertise metalink/Lantiq AP.
		set_conf_param hostapd_vap regular no_otf $pid $interface_name vendor_elements "dd050009860100"
	fi
}

# Read country from calibration file and set it to rc.conf.
# If no country set in calibration file or if country in rc.conf is same as in calibration file, don't write to rc.conf.
set_country()
{
	# Define local parameters
	local ap_index
	local eeprom eeprom_country rc_conf_country

	ap_index=$1

	eeprom=`iwpriv wlan0 gEEPROM`
	eeprom_country=`echo "$eeprom" | sed -n '/EEPROM country:/{s/EEPROM country:.*\([A-Z?][A-Z?]\)/\1/p}'`
	eval rc_conf_country=\${wlphy_${ap_index}_country}
	if [ -n "$eeprom_country" ] && [ "$eeprom_country" != "??" ] && [ "$eeprom_country" != "$rc_conf_country" ]
	then
		status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_phy" "wlphy_${ap_index}_country" "$eeprom_country" > /dev/null
	fi
	if [ -z "$eeprom_country" ]
	then
		# TODO: add error handling for no eeprom/cal file
		echo "Country code is empty. EEPROM or calibration file might be missing"
	fi
}

# For the physical Wave interfaces:
# Read rc.conf value and driver value.
# If driver value is less than rc.conf value, the rc.conf will be updated with the driver value.
set_max_num_sta()
{
	# Define local parameters
	local interface_name
	local change_made i vendor ap_type max_sta driver_value

	change_made=""
	# Check indexes 0 and 1 to see if they are physical WAVE interfaces and if so, set maximum number of STAs for this interface.
	for i in 0 1
	do
		eval vendor=\${wlss_${i}_vendor}
		if [ "$vendor" = "LANTIQ" ]
		then
			eval ap_type=\${wlmn_${i}_apType}
			[ "$ap_type" = "$VAP" ] && continue
			eval interface_name=\${wlmnwave_${i}_interfaceName}
			eval max_sta=\${wlmn_${i}_maxSta}
			driver_value=`iwpriv $interface_name gAPCapsMaxSTAs`
			driver_value=${driver_value##w*:}
			driver_value=${driver_value%% *}
			[ "$driver_value" ] && [ $driver_value -lt $max_sta ] && status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_main" "wlmn_${i}_maxSta" "$driver_value" > /dev/null && change_made="yes"
		fi
	done
	# If rc.conf was modified, save changes and re-source it
	[ "$change_made" ] && /etc/rc.d/backup && rc_conf_source
}

# For the physical Wave interfaces:
# Read rc.conf value and driver value.
# If driver value is less than rc.conf value, the rc.conf will be updated with the driver value.
set_num_antennas()
{
	# Define local parameters
	local interface_name
	local change_made i vendor ap_type phy_name num_antennas driver_value

	change_made=""
	# Check indexes 0 and 1 to see if they are physical WAVE interfaces and if so, set number of antennas for this interface.
	for i in 0 1
	do
		eval vendor=\${wlss_${i}_vendor}
		if [ "$vendor" = "LANTIQ" ]
		then
			eval ap_type=\${wlmn_${i}_apType}
			[ "$ap_type" = "$VAP" ] && continue
			eval interface_name=\${wlmnwave_${i}_interfaceName}
			
			# Get the phy name in iw for the interface
			phy_name=`find_phy_from_interface_name $interface_name`
			eval num_antennas=\${wlcoc_${i}_numAntennas}
			# Read iw info for the interface to a file and remove tabs and asterisks
			iw $phy_name info > $TEMP_CONF_DIR/iw_info_${phy_name}
			sed -i -e 's/\t//g' -e 's/\* //' $TEMP_CONF_DIR/iw_info_${phy_name}
			driver_value=`grep "Available Antennas" $TEMP_CONF_DIR/iw_info_${phy_name}`
			driver_value=${driver_value##*TX 0x}
			driver_value=${driver_value:0:1}
			if [ "$driver_value" ] && [ $driver_value -lt $num_antennas ]
			then
				status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_coc" "wlcoc_${i}_numAntennas" "$driver_value" > /dev/null
				status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_coc" "wlcoc_${i}_prevNumAntennas" "$driver_value" > /dev/null
				change_made="yes"
			fi
			rm -f $TEMP_CONF_DIR/iw_info_${phy_name}
		fi
	done
	# If rc.conf was modified, save changes and re-source it
	[ "$change_made" ] && /etc/rc.d/backup && rc_conf_source
}

# Read rc.conf BF-ANTENNA and SOUNDING-DIMENSION values and driver TX antennas capability value.
# If driver value is less than rc.conf value, the rc.conf will be updated with the driver value.
set_vht_antennas()
{
	# Define local parameters
	local interface_name
	local change_made i vendor ap_type ugw_beamformer_antenna ugw_sounding_dimestion phy_name num_antennas driver_value

	change_made=""
	# Check indexes 0 and 1 to see if they are physical WAVE interfaces and if so, set BF-ANTENNA and SOUNDING-DIMENSION for this interface.
	for i in 0 1
	do
		eval vendor=\${wlss_${i}_vendor}
		if [ "$vendor" = "LANTIQ" ]
		then
			eval ap_type=\${wlmn_${i}_apType}
			[ "$ap_type" = "$VAP" ] && continue
			eval interface_name=\${wlmnwave_${i}_interfaceName}
			# Read rc.conf values
			eval ugw_beamformer_antenna=\${wlphy_${i}_vhtBfAntenna}
			eval ugw_sounding_dimestion=\${wlphy_${i}_vhtSoundingDimension}
			# Read driver capability
			# Get the phy name in iw for the interface
			phy_name=`find_phy_from_interface_name $interface_name`
			# Read iw info for the interface to a file and remove tabs and asterisks
			iw $phy_name info > $TEMP_CONF_DIR/iw_info_${phy_name}
			sed -i -e 's/\t//g' -e 's/\* //' $TEMP_CONF_DIR/iw_info_${phy_name}
			driver_value=`grep "Available Antennas" $TEMP_CONF_DIR/iw_info_${phy_name}`
			driver_value=${driver_value##*TX 0x}
			driver_value=${driver_value:0:1}
			if [ "$driver_value" ]
			then
				[ $driver_value -lt $ugw_beamformer_antenna ] && status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_phy" "wlphy_${i}_vhtBfAntenna" "$driver_value" > /dev/null && change_made="yes"
				[ $driver_value -lt $ugw_sounding_dimestion ] && status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_phy" "wlphy_${i}_vhtSoundingDimension" "$driver_value" > /dev/null && change_made="yes"
			fi
			rm -f $TEMP_CONF_DIR/iw_info_${phy_name}
		fi
	done
	# If rc.conf was modified, save changes and re-source it
	[ "$change_made" ] && /etc/rc.d/backup && rc_conf_source
}

# Get for each possible index the fastpath value and disable AutoCoC for iterfaces in fastpath.
set_auto_coc()
{
	# Define local parameters
	local ap_index
	local fastpath0 fastpath1 ap_index1
	
	ap_index=$1
	fastpath0=$2
	ap_index1=$3
	fastpath1=$4

	[ "$fastpath0" = "1" ] && status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_coc" "wlcoc_${ap_index}_autoCoC" 0
	[ "$fastpath1" = "1" ] && status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_coc" "wlcoc_${ap_index1}_autoCoC" 0
}

# Read driver capability for beamforming.
# If driver doesn't support beamforming, set ht beamforming, vht beamforming and implicit beamforming disabled in rc.conf.
set_beamforming()
{
	# Define local parameters
	local interface_name
	local change_made i vendor ap_type beamforming_support

	change_made=""
	# Check indexes 0 and 1 to see if they are physical WAVE interfaces and if so, check if beamforming is supported for this interface.
	# If beamforming is not supported, set the following parameters to 0: explicitBfEna, implicitBfEna and implicitBfEna.
	for i in 0 1
	do
		eval vendor=\${wlss_${i}_vendor}
		if [ "$vendor" = "LANTIQ" ]
		then
			eval ap_type=\${wlmn_${i}_apType}
			[ "$ap_type" = "$VAP" ] && continue
			eval interface_name=\${wlmnwave_${i}_interfaceName}
			# Read driver capability
			beamforming_support=`iwpriv $interface_name gBfExplicitCap`
			beamforming_support=`echo ${beamforming_support##w*:}`
			[ "$beamforming_support" = "1" ] && continue
			status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_phy_vendor_wave" "wlphywave_${i}_explicitBfEna" "0" > /dev/null
			status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_phy_vendor_wave" "wlphywave_${i}_implicitBfEna" "0" > /dev/null
			status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_phy" "wlphy_${i}_vhtSUbeamformer" "0" > /dev/null
			change_made="yes"
		fi
	done
	# If rc.conf was modified, save changes and re-source it
	[ "$change_made" ] && /etc/rc.d/backup && rc_conf_source
}

# Check if the wlan interface is ready (eeprom/calibration file exist and ifconfig is working)
# When cal_wlan is in flash it is also in /tmp folder, if in eeprom/eefuse then exist proc /proc/net/mtlk/wlanX/eeprom
# If the interface is up, return 0, else return 1.
check_interface_is_ready()
{
	# Define local parameters
	local interface_name radio_name ifconfig_status

	interface_name=$1
	radio_name=${interface_name%%.*}
	# Check if cal_wlan file exists for the radio
	if [ ! -e /tmp/cal_${radio_name}.bin ] && [ ! -e /proc/net/mtlk/${radio_name}/eeprom ]
	then
		echo "cal_${radio_name}.bin file is missing" > /dev/console
		echo 1
		return
	fi
	# Check if the wlan interface already exists.
	ifconfig_status=`ifconfig $interface_name`
	if [ $? -ne 0 ]
	then
		echo "$interface_name interface not ready." > /dev/console
		echo 1
		return
	fi
	echo 0
}

# Check if the interface is in PPA.
# If the PPA is set to "remove" and the interface is in PPA, add commands to remove it.
# If the PPA is set to "add" and the interface is not in the PPA, add the commands to add it.
# The commands are written to the conf file sent as an argument.
set_ppa_commands()
{
	# Define local parameters
	local interface_name
	local action conf_file in_ppa

	interface_name=$1
	action=$2
	conf_file=$3

	# Source config.sh
	. $ETC_PATH/config.sh
	# Check if the interface is in PPA
	in_ppa=`ppacmd getlan | grep "\<$interface_name with\>" -c`
	if [ "$action" = "remove" ] && [ $in_ppa -gt 0 ]
	then
		# Write commands to delete VAP from PPA to the conf file
		echo "ppacmd getportid -i $interface_name > /dev/null" >> $conf_file
		echo "if [ \$? -eq 0 ]" >> $conf_file
		echo "then" >> $conf_file
		if [ "$CONFIG_IFX_CONFIG_CPU" != "GRX500" ]
		then
			echo "	nPortId=\`ppacmd getportid -i $interface_name | sed 's/The.* is //'\`" >> $conf_file
			echo "	nPortId=\$((nPortId+4))" >> $conf_file
			echo "	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=\$nPortId bLearningMAC_PortLock=0" >> $conf_file
		fi
		echo "	ppacmd dellan -i $interface_name" >> $conf_file
		echo "fi" >> $conf_file
	elif [ "$action" = "add" ] && [ "$in_ppa" = "0" ]
	then
		# Write commands to add VAP to PPA to driver the conf file
		echo "ppacmd addlan -i $interface_name" >> $conf_file
		if [ "$CONFIG_IFX_CONFIG_CPU" != "GRX500" ]
		then
			echo "nPortId=\`ppacmd getportid -i $interface_name | sed 's/The.* is //'\`" >> $conf_file
			echo "nPortId=\$((nPortId+4))" >> $conf_file
			echo "switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=\$nPortId bLearningMAC_PortLock=1" >> $conf_file
		fi
	fi
}

# Create new file with current parameters values and source this file.
# Check if the original conf file exists.
# The parameters in this file will be in the format of: current_<parameter name>="<value>" (e.g: current_hw_mode="g")
# conf_file_prefix can be drv_config_post_up, hostapd_phy, hostapd_vap or drvhlpr.
# The sed command for hostapd and drvhlpr files has 5 steps:
# Step 1: Remove all comment lines from conf file (starting with #)
# Step 2: Replace all " = " with "=" (parameters in drvhlpr conf file have the format of parameter = value)
# Step 3: Replace all = with ="
# Step 4: Add " to the end of line (to create the format of parameter="value")
# Step 5: Add the string "current_" to the beginning of the line (to create the final format of current_<parameter name>="<value>")

# The sed command for drv_config_post_up has 4 steps:
# Step 1: Remove all iwpriv and iwconfig with interface name.
# Step 2: Replace the first space with ="
# Step 3: Add " to the end of line (to create the format of parameter="value")
# Step 4: Add the string "current_" to the beginning of the line (to create the final format of current_<parameter name>="<value>")
read_current_values()
{
	# Define local parameters
	local interface_name
	local conf_file_prefix current_values_file cur_ssid hex_ssid cur_osu_ssid hex_osu_ssid

	conf_file_prefix=$1
	interface_name=$2
	current_values_file=$3

	if [ -e ${TEMP_CONF_DIR}/${conf_file_prefix}_${interface_name}.conf ]
	then
		case "$conf_file_prefix" in
		${HOSTAPD_VAP_CONF_PREFIX}|${HOSTAPD_PHY_CONF_PREFIX}|drvhlpr)
			sed -e '/#/d' -e 's/ = /=/' -e 's/=/=\"/' -e 's/$/\"/' -e 's/^/current_/' ${TEMP_CONF_DIR}/${conf_file_prefix}_${interface_name}.conf > $current_values_file
			# Handle special charcters in ssid and osu_ssid, represent value in hex
			# Read current_ssid from conf file
			cur_ssid=`grep "current_ssid" $current_values_file`
			if [ ! -z "$cur_ssid" ]
			then
				# Remove current_ssid=" prefix and " suffix
				cur_ssid=${cur_ssid##current_ssid=\"}
				cur_ssid=${cur_ssid%\"}
				# Convert to hex value and update the current_values_file
				hex_ssid=`ascii2hex $cur_ssid`
				sed -i '/current_ssid=/d' $current_values_file && echo "current_ssid=\"$hex_ssid\"" >> $current_values_file
			fi

			cur_osu_ssid=`grep "current_osu_ssid" $current_values_file`
			if [ ! -z "$cur_osu_ssid" ]
			then
				# Remove current_osu_ssid=" prefix and " suffix
				cur_osu_ssid=${cur_osu_ssid##current_osu_ssid=\"}
				cur_osu_ssid=${cur_osu_ssid%\"}
				# Convert to hex value and update the current_values_file
				hex_osu_ssid=`ascii2hex $cur_osu_ssid`
				sed -i '/current_osu_ssid=/d' $current_values_file && echo "current_osu_ssid=\"$hex_osu_ssid\"" >> $current_values_file
			fi
		;;
		$DRIVER_POST_UP_CONF_PREFIX)
			sed -e 's/iwpriv '$interface_name' //' -e 's/iwconfig '$interface_name' //' -e 's/ /=\"/' -e 's/$/\"/' -e 's/^/current_/' ${TEMP_CONF_DIR}/${conf_file_prefix}_${interface_name}.conf > $current_values_file
		;;
		esac

		. $current_values_file
		rm $current_values_file
	fi
}

# Go over the list of requested parameters and see if new value is different than current value.
# If change was found, return "yes", if no change found, return empty value.
check_param_changed()
{
	# Define local parameters
	local params_list params_changed current_value new_value

	params_list=$1

	params_changed=""
	for parameter in $params_list
	do
		eval current_value=\${current_$parameter}
		eval new_value=\$$parameter
		new_value=`echo $new_value`
		[ "$new_value" != "$current_value" ] && params_changed="yes" && break
	done
	echo "$params_changed"
}

# Check if the WDS peers list was changed (MACs were added or removed).
check_wds_peer_list_changed()
{
	# Define local parameters
	local ap_index
	local driver_peer_aps rc_conf_macs wds_peers_changed driver_peer_list_length num_peers_driver rc_conf_peer_list_length num_peers_rc_conf

	ap_index=$1
	driver_peer_aps=$2
	rc_conf_macs=$3
	wds_peers_changed=""

	# Go over MACs in driver and see if each MAC appears in rc.conf. If not, a change was found, return "yes"
	for driver_mac in $driver_peer_aps
	do
		[ "$rc_conf_macs" = "${rc_conf_macs/$driver_mac/}" ] && wds_peers_changed="yes" && break
	done

	# If a change was not found yet, check if the number of MACs in driver and rc.conf is different, if so, a change was found, return yes.
	if [ -z "$wds_peers_changed" ]
	then
		# Calculate number of peers in driver by measuring the length and divide by 17 (length of a MAC address).
		driver_peer_list_length=${#driver_peer_aps}
		num_peers_driver=$((driver_peer_list_length/17))
		# Calculate number of peers in rc.conf by measuring the length and divide by 17 (length of a MAC address).
		rc_conf_peer_list_length=${#rc_conf_macs}
		num_peers_rc_conf=$((rc_conf_peer_list_length/17))
		[ $num_peers_driver -ne $num_peers_rc_conf ] && wds_peers_changed="yes"
	fi

	echo "$wds_peers_changed"
}

# Go over the requested driver parameters and see if new value is different than current value.
# If change was found, mark the parameter as changed.
# Write to the new values file the parameter and the new value, if the new value is null, write the current value.
# Replace the current driver values file with the new values file.
update_driver_params_file()
{
	# Define local parameters
	local params_list driver_new_values_file driver_current_values_file current_value new_value

	params_list=$1
	driver_new_values_file=$2
	driver_current_values_file=$3

	for parameter in $params_list
	do
		eval current_value=\${current_$parameter}
		eval new_value=\$$parameter
		new_value=`echo $new_value`
		eval ${parameter}_changed=""

		# If current value and new value are empty, nothing needs to be done
		[ -z "$current_value" ] && [ -z "$new_value" ] && continue
		# If current value and new value are not the same, mark parameter as changed.
		if [ "$new_value" != "$current_value" ]
		then
			eval ${parameter}_changed=yes
		fi
		# Write all the values to the new values file.
		# If new value is null, write the current value, else write the new value.
		if [ -z "$new_value" ]
		then
			echo "current_$parameter=\"$current_value\"" >> $driver_new_values_file
		else
			echo "current_$parameter=\"$new_value\"" >> $driver_new_values_file
		fi
	done

	# Replace current values file with the new values file
	mv $driver_new_values_file $driver_current_values_file
}

# Remove parameters from the conf file by creating a grep string command, execute it to write to a new temp file and overwrite the existing conf file with the new file.
# The created grep command removes all the parameters in the params_list from the conf file and writes the result to a temp file.
# This temp file overwrites the existing conf file.
# Example of the final grep command: grep -wv "param_1\|param_2\|param_3" conf_file_6385.conf > conf_file_6385.conf_tmp
remove_params_from_conf()
{
	# Define local parameters
	local params_list conf_file grep_cmd

	params_list=$1
	conf_file=$2

	grep_cmd="grep -wv \""
	for param in $params_list
	do
		grep_cmd=${grep_cmd}${param}\\\|
	done

	grep_cmd=${grep_cmd%\\\|}
	grep_cmd="${grep_cmd}\" $conf_file > ${conf_file}_tmp"
	eval $grep_cmd

	# Replace the conf file with the temp file that doesn't include the removed parameters.
	mv -f ${conf_file}_tmp $conf_file
}

# Converts ascii to hex
ascii2hex()
{
	# Define local parameters
	local ascii_x ascii_len i ascii_char

	ascii_x=$1
	ascii_len=${#ascii_x}
	i=0
	while [ $i -lt $ascii_len ]
	do
		ascii_char=${ascii_x:$i:1}
		printf '\\x%02x' "'$ascii_char" | sed 's/00/20/'
		i=$((i+1))
	done
}

# Check if the fastpath is enabled for the radio of a given interface
get_fastpath_enabled()
{
	# Define local parameters
	local interface_name
	local fastpath_enabled pap_name pap_index

	interface_name=$1

	fastpath_enabled="0"
	
	# Find the ap index of the parent of the VAP
	pap_name=${interface_name%%.*}
	pap_index=`find_index_from_wave_if $pap_name`
	
	# Read fastpath value from the rc.conf and return the value
	eval fastpath_enabled=\${wlphywave_${pap_index}_fastpath}
	echo $fastpath_enabled
}

# Check if an interface is Wave500B
# Wave500B PCIe cards are identified as 1bef:0810 in lspci output.
check_wave500b()
{
	# Define local parameters
	local interface_name
	local i param1 param2 param3 param4

	interface_name=$1
	i=0
	# Read information of physical Wlan interface from wlan_discover output
	. /tmp/wlan_discover.txt

	[ "$PCI_LTQ_COUNT" = "0" ] && echo "no" && return
	[ "$AHB_WLAN_COUNT" = "1" ] && i=1

	# Go over the lspci output saved by wlan_discover.sh
	while read param1 param2 param3 param4
	do
		[ "$param4" = "1bef:0810" ] && wave500b_interface="wlan${i}"
		[ "$wave500b_interface" = "$interface_name" ] && echo "yes" && return
		[ "${param4:0:7}" = "1bef:08" ] && i=$((i+1))
	done < /tmp/lspci.txt
	echo "no"
}
LIB_COMMON_SOURCED="1"
