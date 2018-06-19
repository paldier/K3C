#!/bin/sh
# This script updates the logger parameters in the configuration files.

script_name="fapi_wlan_wave_set_logger_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh
[ ! "$LIB_CONVERT_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_convert.sh
[ ! "$LIB_LOGGER_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_logger.sh

interface_name=$1
interface_index=$2
pid=$3

timestamp $interface_index "$script_name:$interface_name:begin"
print2log $interface_index DEBUG "$script_name $*"

# Map the objects indexes to the received objects in the in.conf file
fw_stream_index=`map_param_index Object $RADIO_VENDOR_FW_LOGGER_OBJECT`
driver_stream_index=`map_param_index Object $RADIO_VENDOR_DRIVER_LOGGER_OBJECT`
#configuration_stream_index=`map_param_index Object $RADIO_VENDOR_CONFIGURATION_LOGGER_OBJECT`
#hostapd_stream_index=`map_param_index Object $RADIO_VENDOR_HOSTAPD_LOGGER_OBJECT`

destination_conf=drv_config_post_up
# Remove logger commands from the conf file
drv_config_post_up_conf_name=${CONF_DIR}/drv_config_post_up_${interface_name}_${pid}.conf
remove_params_from_conf "/proc/net/mtlk_log/rtlog rdebug route" "$drv_config_post_up_conf_name" $DRIVER_POST_UP_CONF_PREFIX

# Calculate new values.
### General parameters
# Components log level
# TODO: remove comment once LogLevel is supported by driver
#fw_log_level=`convert_log_level $interface_name $interface_index $LOGGER_FW`
driver_log_level=`convert_driver_debug_level $interface_index rdebug`
# TODO: remove comment once configuration and hostapd streams are supported
#configurations_log_level=`convert_log_level $interface_name $interface_index $LOGGER_CONFIGURATIONS`
#hostapd_log_level=`convert_log_level $interface_name $interface_index $LOGGER_HOSTAPD`

# Halt on error
#halt_on_error=`db2fapi_convert regular WaveHaltOnErrorLog $interface_index`

# Write the above parameters to the configuration file.
# TODO: remove comment once LogLevel is supported by driver
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$fw_log_level"
set_conf_param $destination_conf proc otf $pid $interface_name "/proc/net/mtlk_log/debug" "$driver_log_level"
# TODO: remove comment once configuration and hostapd streams are supported
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$configurations_log_level"
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$hostapd_log_level"
# TODO: remove comment once HaltOnError is supported by driver
#set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "HaltOnError $interface_name $halt_on_error"

# For each component, save the input configuration parameters to a local DB used by Wave FAPI and set its related parameters
# FW logger parameters and streams
if [ -n "$fw_stream_index" ]
then
	save_db_params logger_set_fw $interface_name $fw_stream_index $interface_index
	local_db_source LOGGER_FW

	# Assign HW modules to the HW FIFOs
	hw_module_fifo=`assign_hw_module_fifo $interface_name $interface_index`
	set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$hw_module_fifo"

	# Remove the existing stream and add existing stream.
	remove_fw_stream0=`remove_stream_arguments $LOGGER_FW $interface_name 0`
	# Write the above configurations to the configuration file.
	set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$remove_fw_stream0"

	# Add the stream
	# Check if stream is enabled and if so, add it
	fw_stream_enabled=`db2fapi_convert boolean WaveFwStreamEnable $interface_index`
	if [ "$fw_stream_enabled" = "1" ]
	then
		add_fw_stream=`add_new_stream_arguments $interface_name $interface_index $LOGGER_FW $fw_stream_index`
		# For WAN connections, need to add route command to runner
		interface_type=`db2fapi_convert regular WaveFwRemoteInterface $interface_index`
		[ "$interface_type" = "WAN" ] && add_route_command "Fw" $destination_conf $interface_index $pid $interface_name
		set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$add_fw_stream"
	fi
fi
# Driver logger parameters and streams
if [ -n "$driver_stream_index" ]
then
	save_db_params logger_set_driver $interface_name $driver_stream_index $interface_index
	local_db_source LOGGER_DRIVER

	# Remove the existing stream and add existing stream.
	remove_driver_stream0=`remove_stream_arguments $LOGGER_DRIVER $interface_name 0`
	# Write the above configurations to the configuration file.
	set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$remove_driver_stream0"

	# Add the stream
	# Check if stream is enabled and if so, add it
	driver_stream_enabled=`db2fapi_convert boolean WaveDriverStreamEnable $interface_index`
	if [ "$driver_stream_enabled" = "1" ]
	then
		add_driver_stream=`add_new_stream_arguments $interface_name $interface_index $LOGGER_DRIVER $driver_stream_index`
		# For WAN connections, need to add route command to runner
		interface_type=`db2fapi_convert regular WaveDriverRemoteInterface $interface_index`
		[ "$interface_type" = "WAN" ] && add_route_command "Driver" $destination_conf $interface_index $pid $interface_name
		set_conf_param $destination_conf proc otf $pid $interface_name "$LOGGER_PROC" "$add_driver_stream"
	fi
fi

# TODO: add hostapd and configuration parameters and streams when support is added

# If the bridge of the logger was changed, bring the rtlog0 down and restart the radio
if test `grep WaveLoggerBridgeName_ ${IN_CONF}`
then
	ifconfig rtlog0 down
	echo "restart_${interface_name%%.*}=yes" >> ${CONF_DIR}/${RESTART_FLAG}_${interface_name}
fi

print2log $interface_index DEBUG "$script_name done"
timestamp $interface_index "$script_name:$interface_name:done"
