#!/bin/sh

script_name="wave_wlan_prepare.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name
local ap_type index configured_vaps current_feature current_index remove_index prepared_i temp_files orig_file

ap_index=$2
timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

pid=$$

# configured_vaps holds the list of ap indexes which were configured.
configured_vaps=""
rm -f $TEMP_CONF_DIR/prepare_commands.sh

# Read the first feature and its index
current_feature=$1
shift
current_index=$1
shift

# Loop over all the input and for each feature add the call to set_feature_params script in the prepare_commands.sh
# When the "remove" feature is found for a specific interface, a flag is set for this interface to prevent from trying to configure this interface.
while [ "$current_feature" ]
do
	# Delete the suffix "_modify" from the feature name. For "remove_vap", set feature name to "remove".
	if [ "$current_feature" = "vap_remove" ]
	then
		current_feature=remove
	else
		current_feature=${current_feature%%_modify}
	fi
	# Verify remove flag is not set for current_index
	eval remove_index=\$remove_index$current_index
	if [ -z "$remove_index" ]
	then
		if [ "$current_feature" = "remove" ]
		then
			eval remove_index$current_index=1
		fi
		echo "(. $ETC_PATH/wave_wlan_set_${current_feature}_params.sh $current_index $pid)" >> $TEMP_CONF_DIR/prepare_commands.sh
		configured_vaps="$configured_vaps $current_index"
	fi
	current_feature=$1
	shift
	current_index=$1
	shift
done

# Create new temp configuration files for all the VAPs (since repetitions may exist, only create if wasn't created before)
# Create new hostapd configuration files for new VAPs which currently have empty hostapd configuration files.
# Temp conf files are created with pid in the name to avoid working on the current conf files and only after the new temp conf files are done they will be used instead of the current files.
for index in $configured_vaps
do
	# Don't do anything for index that already have conf files prepared.
	eval prepared_i=\$prepared$index
	[ ! -z "$prepared_i" ] && continue

	eval interface_name=\${wlmnwave_${index}_interfaceName}
	eval ap_type=\${wlmn_${index}_apType}

	# Create empty conf files with pid in the file name
	create_empty_temp_conf_files $interface_name $ap_type $pid
	# Write initial content to the temp conf files
	write_initial_content $interface_name $ap_type $pid $index

	# Mark that the index has conf files prepared.
	eval prepared$index=yes
done

# Execute the script with the calls to configure the files.
chmod +x $TEMP_CONF_DIR/prepare_commands.sh
(. $TEMP_CONF_DIR/prepare_commands.sh)

# Copy the temporary conf files instead of the existing files.
# ls command sends errors to /dev/null to avoid errors when "remove" already deleted all files.
temp_files=`ls ${TEMP_CONF_DIR}/*${pid}.conf 2>/dev/null`
for file in $temp_files
do
	orig_file=${file%%_${pid}*}.conf
	lock_and_copy_conf_file $orig_file $file $orig_file
done

# Delete all temporary files
rm -f ${TEMP_CONF_DIR}/*_${pid}*

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
