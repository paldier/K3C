#!/bin/sh
# This script updates the logger parameters in the configuration files.

script_name="wave_wlan_set_logger_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$LIB_LOGGER_SOURCED" ] && . /tmp/wave_wlan_lib_logger.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/wave_wlan_lib_convert.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Define local parameters
local ap_index pid interface_name call_on_init destination_conf drv_config_post_up_conf_name \
fw_log_level driver_log_level configurations_log_level hostapd_log_level halt_on_error i \
remove_fw_stream0 remove_driver_stream0 remove_configurations_stream0 remove_hostapd_stream0 cur_pcpeid stream_enabled pcpeid add_stream hw_module_fifo

ap_index=$1
pid=$2
call_on_init=$3

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}

# If script was called on init, write commands to the runner
if [ "$call_on_init" ]
then
	destination_conf=$WAVE_WLAN_RUNNNER
else
	destination_conf=drv_config_post_up
	# Remove logger commands from the conf file
	drv_config_post_up_conf_name=${TEMP_CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf
	remove_params_from_conf "/proc/net/mtlk_log/rtlog" "$drv_config_post_up_conf_name"
fi
# Calculate new values.

### General parameters
# Components log level
# TODO: remove comment once LogLevel is supported by driver
#fw_log_level=`convert_log_level $ap_index $interface_name $LOGGER_FW`
driver_log_level=`convert_driver_debug_level $ap_index rdebug`
# TODO: remove comment once configuration and hostapd streams are supported
#configurations_log_level=`convert_log_level $ap_index $interface_name $LOGGER_CONFIGURATIONS`
#hostapd_log_level=`convert_log_level $ap_index $interface_name $LOGGER_HOSTAPD`

# Halt on error
eval halt_on_error=\${wllogwave_${ap_index}_haltOnError}

# Write the above parameters to the configuration file.
# TODO: remove comment once LogLevel is supported by driver
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$fw_log_level"
set_conf_param $destination_conf proc otf $pid $interface_name  "/proc/net/mtlk_log/debug" "$driver_log_level"
# TODO: remove comment once configuration and hostapd streams are supported
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$configurations_log_level"
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$hostapd_log_level"
# TODO: remove comment once HaltOnError is supported by driver
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "HaltOnError $interface_name $halt_on_error"

# Assign HW modules to the HW FIFOs
hw_module_fifo=`assign_hw_module_fifo $interface_name`
set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$hw_module_fifo"

# Add the streams
# If script is not called on init, remove the existing streams and add all the streams.
# If script is called on init, no need to remove streams.
if [ -z "$call_on_init" ]
then
	# Remove all the streams from the driver
	remove_fw_stream0=`remove_stream_arguments $LOGGER_FW $interface_name 0`
	remove_driver_stream0=`remove_stream_arguments $LOGGER_DRIVER $interface_name 0`
	# TODO: remove comment once configuration and hostapd streams are supported
	#remove_configurations_stream0=`remove_stream_arguments $LOGGER_CONFIGURATIONS $interface_name 0`
	#remove_hostapd_stream0=`remove_stream_arguments $LOGGER_HOSTAPD $interface_name 0`

	# Write the above configurations to the configuration file.
	[ "$remove_fw_stream0" ] && set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$remove_fw_stream0"
	[ "$remove_driver_stream0" ] && set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$remove_driver_stream0"
	# TODO: remove comment once configuration and hostapd streams are supported
	#[ "$remove_configurations_stream0" ] && set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$remove_configurations_stream0"
	#[ "$remove_hostapd_stream0" ] && set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$remove_hostapd_stream0"
fi
# Add the streams according to rc.conf
# Go over the stream_index of the streams from 0 to wlan_logger_streams_vendor_wave_nextCpeId-1 and add the enabled streams that belong to the current radio.
# The pcpeid of the current radio is the suffix of the interface name incremented by 1.
cur_pcpeid=$((${interface_name##wlan}+1))
i=0
while [ $i -lt $((wlan_logger_streams_vendor_wave_nextCpeId-1)) ]
do
	eval stream_enabled=\${wlstrmwave_${i}_enable}
	eval pcpeid=\${wlstrmwave_${i}_pcpeId}
	if [ "$stream_enabled" = "1" ] && [ $pcpeid -eq $cur_pcpeid ]
	then
		add_stream=`add_new_stream_arguments $ap_index $i $interface_name`
		# Write the above configuration to the configuration file.
		set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$add_stream"
	fi
	i=$((i+1))
done

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
