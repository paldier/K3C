#!/bin/sh
# Library script to convert from rc.conf values to hostapd/driver values.

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Get the assigned FIFOs for a stream and return hexadecimal representation of it.
get_assigned_fifos()
{
	# Define local parameters
	local stream_id if_index assigned_fifos i assigned_stream

	stream_id=$1
	if_index=$2

	assigned_fifos=0
	# Go over the FIFOs and for each FIFO check if it is assigned to the stream_id
	for i in 0 1 2 3 4 5 6 7 8 9 10 11 12
	do
		eval assigned_stream=\${wlfifowave${if_index}_${i}_fifoStream}
		if [ $assigned_stream ] && [ $assigned_stream = $stream_id ]
		then
			shift_res=$((1<<i))
			assigned_fifos=$((assigned_fifos|shift_res))
		fi
	done

	assigned_fifos=`printf '0x%04x' "$assigned_fifos"`
	echo "$assigned_fifos"
}

# Get the assigned module for each HW FIFO and return the command for LogHwModuleFifo
assign_hw_module_fifo()
{
	# Define local parameters
	local interface_name if_index hw_module assigned_hw i

	interface_name=$1

	# Set the index for the fifo parameter: 0 for wlan0, 1 for wlan1
	if_index=${interface_name##wlan}

	# Generate the string of the assigned modules
	eval hw_module=\${wlfifowave${if_index}_7_fifoHwModule}
	assigned_hw="$hw_module"
	for i in 8 9 10 11 12
	do
		eval hw_module=\${wlfifowave${if_index}_${i}_fifoHwModule}
		assigned_hw="$assigned_hw $hw_module"
	done
	echo "LogHwModuleFifo $interface_name $assigned_hw"
}

# Get the source MAC of a stream
# If component is FW: the MAC address is the radio MAC address
# For othe components: MAC address is the bridge MAC address
get_source_mac()
{
	# Define local parameters
	local ap_index
	local component_id source_mac

	ap_index=$1
	component_id=$2

	if [ "$component_id" = "$LOGGER_FW" ]
	then
		eval source_mac=\${wlmn_${ap_index}_bssid}
	else
		source_mac=`ifconfig br0 | grep HWaddr | awk '{print $5}'`
	fi
	echo "$source_mac"
}

# get the source IP address of a stream
# If component is FW: the IP is selected according to DHCP pool+1, if this value is illegal, use highest available in pool.
# For other components: the IP address is the bridge IP address
get_source_ip()
{
	# Define local parameters
	local ap_index
	local component_id dhcp_end last_byte first_bytes new_last_byte dumplease source_ip

	ap_index=$1
	component_id=$2

	if [ "$component_id" = "$LOGGER_FW" ]
	then
		# Get the DHCP pool end, the last IP byte value and the first 2 bytes values.
		eval dhcp_end=\${lan_dhcps_endIp}
		last_byte=${dhcp_end##*\.}
		first_bytes=${dhcp_end%%\.${last_byte}}

		# Find the first value of last byte+1 that is not assigned by dhcp.
		new_last_byte=$((last_byte+1))
		dumplease=`/usr/bin/dumpleases | grep "${first_bytes}.${new_last_byte}"`
		while [ -n "$dumplease" ]
		do
			new_last_byte=$((new_last_byte+1))
			dumplease=`/usr/bin/dumpleases | grep "${first_bytes}.${new_last_byte}"`
		done

		# If found value is legal, use it. If value is not legal, try to take the highest available IP in the DHCP pool.
		if [ $new_last_byte -gt 253 ]
		then
			new_last_byte=$last_byte
			# Don't use 254 value
			[ $new_last_byte -eq 254 ] && new_last_byte=253
			dumplease=`/usr/bin/dumpleases | grep "${first_bytes}.${new_last_byte}"`
			while [ -n "$dumplease" ]
			do
				new_last_byte=$((new_last_byte-1))
				dumplease=`/usr/bin/dumpleases | grep "${first_bytes}.${new_last_byte}"`
			done
		fi
		source_ip=${first_bytes}.${new_last_byte}
	else
		eval source_ip=\${lan_main_0_ipAddr}
	fi
	echo "$source_ip"
}

# Find the destination MAC of a given IP and set the value to rc.conf.
# Return the MAC found.
set_destination_mac()
{
	# Define local parameters
	local stream_index dest_ping interface_type dest_mac

	stream_index=$1
	dest_ping=$2

	# For LAN the destination ping is the destination IP.
	# For WAN the destination ping is the gateway IP.
	# Read the interface type
	eval interface_type=\${wlstrmwave_${stream_index}_interfaceType}
	[ "$interface_type" = "$LOGGER_WAN" ] && eval dest_ping=\${lan_main_0_gw}

	# Ping for 2 seconds to the requested IP address
	ping $dest_ping -w 2 > /dev/null

	# Get the MAC address from the arp table
	dest_mac=`arp | grep $dest_ping | awk '{print $4}'`

	# Save the MAC to the rc.conf
	status_oper -f $CONFIGS_PATH/rc.conf -u SET "wlan_logger_streams_vendor_wave" "wlstrmwave_${stream_index}_destMac" "$dest_mac" > /dev/null
	/etc/rc.d/backup
	# Return the destination MAC
	echo "$dest_mac"
}

# Read rc.conf log level for a component and return the string to configure it.
# String structure is: LogLevel <component> <log level>
convert_log_level()
{
	# Define local parameters
	local ap_index interface_name
	local component component_name log_level

	ap_index=$1
	interface_name=$2
	component=$3

	case $component in
	0)
		component_name="Fw"
	;;
	1)
		component_name="Driver"
	;;
	2)
		component_name="Config"
	;;
	3)
		component_name="Hostapd"
	;;
	esac
	eval log_level=\${wllogwave_${ap_index}_logLevel${component_name}}
	echo "LogLevel $component $interface_name $log_level"
}

# Create the string to remove a stream.
# String structure is: LogRemStream <component> <interface_name> <stream_id>
remove_stream_arguments()
{
	# Define local parameters
	local component_id interface_name stream_id

	component_id=$1
	interface_name=$2
	stream_id=$3

	echo "LogRemStream $component_id $interface_name $stream_id"
}

# Create the string to add a new stream.
# String structure is: LogAddStream 0 wlan0 0 192.168.1.1 01:02:03:04:05:06 2222 192.168.1.222 A1:A2:A3:A4:A5:A6 3333 1024 0x1234
add_new_stream_arguments()
{
	# Define local parameters
	local ap_index interface_name
	local stream_index if_index component stream_id source_ip source_mac source_port dest_ip dest_mac dest_port assigned_fifos hw_module assigned_hw i res buffer_threshold

	ap_index=$1
	stream_index=$2
	interface_name=$3

	# Set the index for the fifo parameter: 0 for wlan0, 1 for wlan1
	if_index=${interface_name##wlan}

	# Read the needed parameters
	eval component=\${wlstrmwave_${stream_index}_componentId}
	eval stream_id=\${wlstrmwave_${stream_index}_streamId}

	# For FW, the MAC is the MAC of the radio interface wlan0 and the IP is an IP from the DHCP pool.
	# For other components, the MAC is the MAC of the bridge (br0) and the IP is the bridge IP.

	### Currently: the source MAC is a MAC address not known to the bridge and the source IP is the bridge IP

	#eval source_mac=\${wlstrmwave_${stream_index}_sourceMac}
	#eval source_ip=\${wlstrmwave_${stream_index}_sourceIp}
	# If MAC wasn't set yet, get the needed MAC
	#[ "$source_mac" = "00:00:00:00:00:00" ] && source_mac=`get_source_mac $ap_index $component`
	# If IP wasn't set yet, get the needed IP
	#[ -z "$source_ip" ] && `get_source_ip $ap_index $component`
	source_mac="00:00:00:00:00:10"
	eval source_ip=\${lan_main_0_ipAddr}
	
	# If component is FW, read the assigned FIFOs for the stream.
	[ "$component" = "$LOGGER_FW" ] && assigned_fifos=`get_assigned_fifos $stream_id $if_index`

	eval source_port=\${wlstrmwave_${stream_index}_sourcePort}
	eval dest_ip=\${wlstrmwave_${stream_index}_destIp}

	dest_mac=`set_destination_mac $stream_index $dest_ip`
	eval dest_port=\${wlstrmwave_${stream_index}_destPort}

	buffer_threshold=1024
	echo "LogAddStream $component $interface_name $stream_id $source_ip $source_mac $source_port $dest_ip $dest_mac $dest_port $buffer_threshold $assigned_fifos"
}
LIB_LOGGER_SOURCED="1"
