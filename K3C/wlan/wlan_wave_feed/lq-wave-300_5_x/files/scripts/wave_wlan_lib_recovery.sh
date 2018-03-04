#!/bin/sh
# Library script for recovery functions.

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Get free mem, including memory freeable from buffers
get_total_free_mem()
{
	sync
	cat /proc/meminfo | awk 'BEGIN {total=0} { \
	if ($1 == "MemFree:") { total += $2}; if ($1 == "Buffers:") {total += $2}  } \
	END {print total}'
}

# Return 0 if enough free memory available to save dump
validate_memory()
{
	# Define local parameters
	local min_free_mem_kb ret free_mem free_mem_sync
	
	min_free_mem_kb=$1
		
	ret=0
	free_mem=`cat /proc/meminfo | grep MemFree | awk '{print $2}'`
	if [ $free_mem -lt $min_free_mem_kb ]
	then
		# Validate also after sync:
		free_mem_sync=`get_total_free_mem`
		print2log $ap_index NOTIFY "memory after sync = $free_mem_sync (before=$free_mem)"
		[ $free_mem_sync -lt $min_free_mem_kb ] && ret=1
	fi
	echo $ret
}

# $1 = folder (/tmp/wave_tmp or /root/mtlk)
# $2 = max dumps to save
# Check if max number of dumps reached and if so, delete the oldest dump file.
prepare_folder_for_dumps()
{
	# Define local parameters
	local folder max_dumps exist_num_dumps oldest_dump

	folder=$1
	max_dumps=$2

	exist_num_dumps=`ls $folder/ | grep -c dump.tar`
	while [ $exist_num_dumps -ge $max_dumps ]
	do
		oldest_dump=`ls -tr $folder/*dump.tar | sed -n '1 p'`
		rm $oldest_dump
		exist_num_dumps=`ls $folder/ | grep -c dump.tar`
	done
}
LIB_RECOVERY_SOURCED="1"
