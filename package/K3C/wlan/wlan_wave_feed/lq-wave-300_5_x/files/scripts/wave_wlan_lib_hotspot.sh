#!/bin/sh
# Library script to convert from rc.conf values to hostapd/driver values.

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Get the list of indexes in a rc.conf section for a specified pcpeid
get_section_indexes()
{
	# Define local parameters
	local count prefix pcpeid i current_pcpeid indexes_list

	count=$1
	prefix=$2
	pcpeid=$3

	indexes_list=""
	i=0
	while [ $i -lt $count ]
	do
		eval current_pcpeid=\${${prefix}_${i}_pcpeId}
		if [ "$current_pcpeid" = "$pcpeid" ]
		then
			if [ -n "$indexes_list" ]
			then
				indexes_list="$indexes_list $i"
			else
				indexes_list=$i
			fi
		fi
		i=$((i+1))
	done
	echo "$indexes_list"
}

# Read rc.conf hessid value.
# If value is empty, set value to rc.conf value of bssid.
convert_hessid()
{
	# Define local parameters
	local ap_index
	local hessid

	ap_index=$1
	eval hessid=\${wlhs2_${ap_index}_hessid}
	[ -z "$hessid" ] && eval hessid=\${wlmn_${ap_index}_bssid}

	echo "$hessid"
}

# Read rc.conf parameters in wlhs2 section:
# ipv4AddrType and ipv6AddrType.
# Create a single value to present both values.
convert_ipaddr_type_availability()
{
	# Define local parameters
	local ap_index
	local ip_v4_type ip_v6_type masked shifted masked_6 ipaddr_type_availability

	ap_index=$1

	eval ip_v4_type=\${wlhs2_${ap_index}_ipv4AddrType}
	eval ip_v6_type=\${wlhs2_${ap_index}_ipv6AddrType}

	masked=$((ip_v4_type&63))
	shifted=$((masked<<2))
	masked_6=$((ip_v6_type&3))
	ipaddr_type_availability=$((shifted|masked_6))
	ipaddr_type_availability=`echo $ipaddr_type_availability | awk '{printf "0%-X\n", $0}'`

	echo "$ipaddr_type_availability"
}

# Write "regular" parameters to hostapd configuration file.
# Parameters:
#	current_index – parameter index in the rc.conf section
#	prefix – the prefix of the rc.conf section parameters.
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	interface_name
#	conf_file – configuration file to write.
#	pid
# Read the value from rc.conf of: ${prefix}_${current_index}_${rc_conf_name}
# Handle special hostapd parameters: venue_name and hs20_oper_friendly_name
# Write the parameter to the hostapd configuration file
hs20_write_param()
{
	# Define local parameters
	local ap_index
	local current_index prefix rc_conf_name hostapd_name interface_name conf_file pid value

	current_index=$1
	prefix=$2
	rc_conf_name=$3
	hostapd_name=$4
	interface_name=$5
	conf_file=$6
	pid=$7

	eval value=\${${prefix}_${current_index}_${rc_conf_name}}
	if [ -n "$value" ]
	then
		if [ "$hostapd_name" = "venue_name" ] || [ "$hostapd_name" = "hs20_oper_friendly_name" ]
		then
			value=$(printf "%b" "$value")
		fi
	fi

	# Write parameter to conf file
	set_conf_param $conf_file regular no_otf $pid $interface_name $hostapd_name "$value"
}

# Write "special" parameter from wlhsoi rc.conf section to hostapd configuration file.
# Parameters:
#	current_index – parameter index in the rc.conf section
#	prefix – the prefix of the rc.conf section parameters.
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	interface_name
#	conf_file – configuration file to write.
#	pid
# Read the values from rc.conf section wlhsoi of: osuIconWidth, osuIconHeight, osuIconCode, osuIconType, osuIconName and osuIconFile
# Save all the read parameters to a single parameter: {icon_width}:${icon_height}:${icon_code}:${icon_type}:${icon_name}:${icon_file}
# Write the parameter to the hostapd configuration file
hs20_write_param_wlhsoi()
{
	# Define local parameters
	local current_index prefix rc_conf_name hostapd_name interface_name conf_file pid value
	local icon_width icon_height icon_code icon_type icon_name icon_file

	current_index=$1
	prefix=$2
	rc_conf_name=$3
	hostapd_name=$4
	interface_name=$5
	conf_file=$6
	pid=$7

	eval icon_width=\${${prefix}_${current_index}_osuIconWidth}
	eval icon_height=\${${prefix}_${current_index}_osuIconHeight}
	eval icon_code=\${${prefix}_${current_index}_osuIconCode}
	eval icon_type=\${${prefix}_${current_index}_osuIconType}
	icon_type=$(printf "%b" "$icon_type")
	eval icon_name=\${${prefix}_${current_index}_osuIconName}
	eval icon_file=\${${prefix}_${current_index}_osuIconFile}
	icon_file=$(printf "%b" "$icon_file")

	value="${icon_width}:${icon_height}:${icon_code}:${icon_type}:${icon_name}:${icon_file}"

	# Write parameter to conf file
	set_conf_param $conf_file regular no_otf $pid $interface_name hs20_icon "$value"
}

# Write "special" parameter from wlhsop rc.conf section to hostapd configuration file.
# Parameters:
#	current_index – parameter index in the rc.conf section
#	prefix – the prefix of the rc.conf section parameters.
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	interface_name
#	conf_file – configuration file to write.
#	pid
# Read the values from rc.conf section wlhsoi of:
# osuProvServerUri, osuProvFriendName, osuProvFriendName2, osuProvNai, osuProvMethod, osuProvIcon, osuProvIcon2, osuProvServiceDesc and osuProvServiceDesc2
# Write all theabove  parameters to the hostapd configuration file
hs20_write_param_wlhsop()
{
	# Define local parameters
	local current_index prefix rc_conf_name hostapd_name interface_name conf_file pid
	local server_uri friendly_name friendly_name2 osu_nai method_list osu_icon osu_icon2 service_desc service_desc2

	current_index=$1
	prefix=$2
	rc_conf_name=$3
	hostapd_name=$4
	interface_name=$5
	conf_file=$6
	pid=$7

	eval server_uri=\${${prefix}_${current_index}_osuProvServerUri}
	server_uri=$(printf "%b" "$server_uri")
	eval friendly_name=\${${prefix}_${current_index}_osuProvFriendName}
	friendly_name=$(printf "%b" "$friendly_name")
	eval friendly_name2=\${${prefix}_${current_index}_osuProvFriendName2}
	[ -n "$friendly_name2" ] && friendly_name2=$(printf "%b" "$friendly_name2")
	eval osu_nai=\${${prefix}_${current_index}_osuProvNai}
	osu_nai=$(printf "%b" "$osu_nai")
	eval method_list=\${${prefix}_${current_index}_osuProvMethod}
	[ -n "$method_list" ] && [ "$method_list" = "2" ] && method_list="1 0"
	eval osu_icon=\${${prefix}_${current_index}_osuProvIcon}
	eval osu_icon2=\${${prefix}_${current_index}_osuProvIcon2}
	eval service_desc=\${${prefix}_${current_index}_osuProvServiceDesc}
	service_desc=$(printf "%b" "$service_desc")
	eval service_desc2=\${${prefix}_${current_index}_osuProvServiceDesc2}
	[ -n "$service_desc2" ] && service_desc2=$(printf "%b" "$service_desc2")

	# Write parameter to conf file
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_server_uri "$server_uri"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_friendly_name "$friendly_name"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_friendly_name "$friendly_name2"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_nai "$osu_nai"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_method_list "$method_list"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_icon "$osu_icon"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_icon "$osu_icon2"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_service_desc "$service_desc"
	set_conf_param $conf_file regular no_otf $pid $interface_name osu_service_desc "$service_desc2"
}

# Write "special" parameter from wlhsrc rc.conf section to hostapd configuration file.
# Parameters:
#	current_index – parameter index in the rc.conf section
#	prefix – the prefix of the rc.conf section parameters.
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	interface_name
#	conf_file – configuration file to write.
#	pid
# Read the values from rc.conf section wlhsrc of ${prefix}_${current_index}_${rc_conf_name}
# If value is not empty, remove dashes (that may or may not exist) from value
# Write the parameter to the hostapd configuration file
hs20_write_param_wlhsrc()
{
	# Define local parameters
	local current_index prefix rc_conf_name hostapd_name interface_name conf_file pid value

	current_index=$1
	prefix=$2
	rc_conf_name=$3
	hostapd_name=$4
	interface_name=$5
	conf_file=$6
	pid=$7

	eval value=\${${prefix}_${current_index}_${rc_conf_name}}
	# Remove dashes from value
	[ -n "$value" ] && value=${value//-/}

	# Write parameter to conf file
	set_conf_param $conf_file regular no_otf $pid $interface_name $hostapd_name "$value"
}

# Write "special" parameter from wlhsdn rc.conf section to hostapd configuration file.
# Parameters:
#	current_index – parameter index in the rc.conf section
#	prefix – the prefix of the rc.conf section parameters.
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	interface_name
#	conf_file – configuration file to write.
#	pid
# Call get_section_indexes function with the arguments: <wlan_hs2_domain_name_Count>, wlhsdn, <pcpeid> and save result to domain_name_indexes
# Loop on all the indexes in domain_name_indexes and for each index:
# Read the value from rc.conf of: ${prefix}_${index}_domainName
#	If domain_name is empty, save the above value into the parameter domain_name
#	If domain_name is not empty, save the above value into the parameter domain_name as domain_name,domainName
# Write the parameter to the hostapd configuration file
hs20_write_param_wlhsdn()
{
	# Define local parameters
	local ap_index interface_name conf_file pid
	local hs20 current_index prefix rc_conf_name hostapd_name pcpeid domain_name_indexes domain_name index value

	current_index=$1
	prefix=$2
	rc_conf_name=$3
	hostapd_name=$4
	interface_name=$5
	conf_file=$6
	pid=$7

	ap_index=`find_index_from_wave_if $interface_name`
	eval hs20=\${wlhs2_${ap_index}_hs20Mode}
	domain_name=""

	if [ $hs20 -gt $HS20_MODE_DISABLED ]
	then
		pcpeid=$((ap_index+1))
		domain_name_indexes=`get_section_indexes $wlan_hs2_domain_name_Count wlhsdn $pcpeid`

		for index in $domain_name_indexes
		do
			eval value=\${${prefix}_${index}_domainName}
			value=$(printf "%b" "$value")
			if [ -n "$domain_name" ]
			then
				domain_name="${domain_name},${value}"
			else
				domain_name="$value"
			fi
		done
	fi
	# Write parameter to conf file
	set_conf_param $conf_file regular no_otf $pid $interface_name domain_name "$domain_name"
}

# Write "special" parameter from wlhsnr rc.conf section to hostapd configuration file.
# Parameters:
#	current_index – parameter index in the rc.conf section
#	prefix – the prefix of the rc.conf section parameters.
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	interface_name
#	conf_file – configuration file to write.
#	pid
# Call get_section_indexes function with the arguments: <wlan_hs2_domain_name_Count>, wlhsdn, <pcpeid> and save result to domain_name_indexes
# Loop on all the indexes in domain_name_indexes and for each index:
# Read the value from rc.conf of: ${prefix}_${index}_domainName
#	If domain_name is empty, save the above value into the parameter domain_name
#	If domain_name is not empty, save the above value into the parameter domain_name as domain_name,domainName
# Write the parameter to the hostapd configuration file
hs20_write_param_wlhsnr()
{
	# Define local parameters
	local interface_name conf_file pid
	local current_index prefix rc_conf_name hostapd_name nai_realm_name eap1 eap2 nai_realm

	current_index=$1
	prefix=$2
	rc_conf_name=$3
	hostapd_name=$4
	interface_name=$5
	conf_file=$6
	pid=$7

	nai_realm=""
	eval nai_realm_name=\${${prefix}_${current_index}_naiRealmName}
	eval eap1=\${${prefix}_${current_index}_naiRealmEap1}
	eval eap2=\${${prefix}_${current_index}_naiRealmEap2}

	if [ -n "$nai_realm_name" ]
	then
		nai_realm_name=$(printf "%b" "$nai_realm_name")
		if [ "$eap1" = "0" ]
		then
			eap1=",13[5:6]"
		elif [ "$eap1" = "1" ]
		then
			eap1=",21[2:4][5:7]"
		else
			eap1=""
		fi
		if [ "$eap2" = "0" ]
		then
			eap2=",13[5:6]"
		elif [ "$eap2" = "1" ]
		then
			eap2=",21[2:4][5:7]"
		else
			eap2=""
		fi
		nai_realm="0,${nai_realm_name}${eap1}${eap2}"
	fi

	# Write parameter to conf file
	set_conf_param $conf_file regular no_otf $pid $interface_name $hostapd_name "$nai_realm"
}

# This function will call the needed function to write a parameter in the hostapd conf.
# Parameters:
# 	prefix - the prefix of the rc.conf section parameters
#	rc_conf_name – parameter name in rc.conf
#	hostapd_name – parameter name in hostapd configuration file
#	indexes_list – list of indexes to read
#	special – special parameter to distinguish between functions for the different rc.conf parameters value of “regular” means no special.
#	interface_name
#	conf_file – configuration file to write.
#	pid
# The function will Go over all the indexes in the indexes_list and for each index:
#	Call hs20_write_param${special} (if special parameter is empty, hs20_write_param is called) with the parameters:
#	current_index, prefix, rc_conf_name, hostapd_name, interface_name, conf_file and pid
hs20_write_conf()
{
	# Define local parameters
	local prefix rc_conf_name hostapd_name indexes_list special interface_name conf_file pid index

	prefix=$1
	rc_conf_name=$2
	hostapd_name=$3
	indexes_list=$4
	special="$5"
	interface_name=$6
	conf_file=$7
	pid=$8

	[ "$special" = "regular" ] && special=""
	for index in $indexes_list
	do
		hs20_write_param${special} $index $prefix $rc_conf_name $hostapd_name $interface_name $conf_file $pid
	done
}
LIB_HOTSPOT_SOURCED="1"
