#!/bin/sh

script_name="wave_wlan_check_overlay.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh

# Define local parameters
local ap_index
	
ap_index=$1

verify_versions()
{
	# Define local parameters
	local driver_drv_version driver_fw_version driver_prog_gen4_version driver_prog_gen5_version ugw_drv_version ugw_fw_version ugw_prog_gen4_version ugw_prog_gen5_version
	local var1 var2 var3 var4 var5 var6 var7 var8 component progmodel_name different_versions
	
	# Read information of physical Wlan interface from wlan_discover output
	. /tmp/wlan_discover.txt
	
	# Source wave_components.ver to get the UGW versions
	. /etc/wave_components.ver
	
	# Read the values from the /proc
	while read var1 var2 var3 var4 var5 var6 var7 var8
	do
		case "$var1" in
			"Driver")
				component="driver"
				continue
				;;
			"MAC/PHY")
				component="fw"
				continue
				;;
			"ProgModel:")
				# Check if it's Gen5 or Gen4 progmodel
				progmodel_name=$var2
				progmodel_name=${progmodel_name:10:4}
				if [ "$progmodel_name" = "gen4" ]
				then
					component="progmodel-gen4"
				elif [ "$progmodel_name" = "gen5" ]
				then
					component="progmodel-gen5"
				fi
				;;
		esac
		
		case "$component" in
			"driver")
				# Driver veriosn in proc is in the format of: A.B.C.D.E.exported.HASH. Comparing the hash value.
				driver_drv_version="$var1"
				driver_drv_version=${driver_drv_version##*exported.}
				driver_drv_version=${driver_drv_version%%.*}
				component=""
				;;
			"fw")
				driver_fw_version="$var8"
				driver_fw_version=${driver_fw_version%% MIPS*}
				component=""
				;;
			"progmodel-gen4")
				driver_prog_gen4_version="$var4"
				component=""
				;;
			"progmodel-gen5")
				driver_prog_gen5_version="$var4"
				component=""
				;;
		esac
	done < /proc/net/mtlk/version
	
	# Read the values from /etc/wave_components.ver
	ugw_drv_version=${wave_driver_ver##*.}
	ugw_fw_version=$wave_mac_ver
	ugw_prog_gen4_version=$wave_ar10_progmodel_ver
	ugw_prog_gen5_version=$wave500_progmodel_ver
	# If AHB disabled, Gen4 progmodel won't appear in driver output, so nothing to compare
	[ "$AHB_WLAN_COUNT" = "0" ] && ugw_prog_gen4_version=""
	# If no Wave500 detected, Gen5 progmodel won't appear in driver output, so nothing to compare
	[ "$PCI_WLAN_COUNT" = "0" ] && ugw_prog_gen5_version=""
	
	# If an interface wasn't started well, the version in the driver is empty. Don't compare versions in such case.
	[ -z "$driver_drv_version" ] && ugw_drv_version=""
	[ -z "$driver_fw_version" ] && ugw_fw_version=""
	[ -z "$driver_prog_gen4_version" ] && ugw_prog_gen4_version=""
	[ -z "$driver_prog_gen5_version" ] && ugw_prog_gen5_version=""
	
	# Compare the versions numbers
	different_versions=""
	[ "$driver_drv_version" != "$ugw_drv_version" ] && different_versions="driver"
	[ "$driver_fw_version" != "$ugw_fw_version" ] && different_versions="${different_versions} fw"
	[ "$driver_prog_gen4_version" != "$ugw_prog_gen4_version" ] && different_versions="${different_versions} AR10-Progmodels"
	# TODO: remove comment once Gen5 progmodel version is displayed correctly in driver output
	#[ "$driver_prog_gen5_version" != "$ugw_prog_gen5_version" ] && different_versions="${different_versions} Wave500-Progmodels"
	different_versions=`echo $different_versions`
	
	if [ -n "$different_versions" ]
	then
		print2log $ap_index ATTENTION ""
		print2log $ap_index ATTENTION "#############################################################################################"
		print2log $ap_index ATTENTION "######### $script_name: binaries version is different from expected version ###"
		print2log $ap_index ATTENTION "#############################################################################################"
		print2log $ap_index ATTENTION "#### difference was found for: $different_versions"
		print2log $ap_index ATTENTION "#############################################################################################"
	fi
}

# Check if the overlay folder has files in a Wave related folders:
# /root/mtlk
# /lib
# /etc/rc.d
# If files exist in any of these overlay folders, show warnning.
check_overlay()
{
	# Define local parameters
	local overlay_files file

	overlay_files=`find /overlay/root/mtlk /overlay/lib /overlay/etc/rc.d 2>/dev/null`
	if [ -n "$overlay_files" ]
	then
		print2log $ap_index ATTENTION ""
		print2log $ap_index ATTENTION "########################################################################################"
		print2log $ap_index ATTENTION "######### $script_name: Overlay folder has leftover Wave related files ###"
		print2log $ap_index ATTENTION "########################################################################################"
		print2log $ap_index ATTENTION "##### Files found in Overlay folder are:"
		for file in $overlay_files
		do
			print2log $ap_index ATTENTION "##### $file"
		done
		print2log $ap_index ATTENTION "########################################################################################"
	fi
}

# Add sleep to allow prints in console to finish
sleep 7

verify_versions
check_overlay
