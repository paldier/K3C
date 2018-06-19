#!/bin/sh

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh

# Find the interface index of wlan0
interface_index=`find_index_from_interface_name wlan0`

##########################################################################
# Probe before burn file

image=`which upgrade`
status=$?
print2log $interface_index DEBUG "$script_name: status=$status"
# Set to cal_wlan mode, overrun otherwize (if not UGW750):
burn_mode="cal_wlan"

if [ $status -eq 0 ]
then
	print2log $interface_index DEBUG "$script_name: FOUND UGW350"
	burn_mode="upgrade" 
else
	print2log $interface_index DEBUG "$script_name: Non UGW350"
	haven_park=`cat /etc/config.sh | grep haven_park` #if not exist config.sh? 
	print2log $interface_index DEBUG "$script_name: haven_park=$haven_park"	
	# Overwrite if PUMA:
	[ -n "$haven_park" ] && burn_mode="eeprom_tar"
fi
print2log $interface_index DEBUG "$script_name: burn_mode=$burn_mode"

##########################################################################


# firmware_dir=`grep FIRMWARE_DIR= /etc/hotplug/firmware.agent | sed 's/FIRMWARE_DIR=//'`
firmware_dir=/tmp
eeprom_partition=wlanconfig
eeprom_tar=eeprom.tar.gz

src_file=$1
dst_file=$firmware_dir/$2

if [ ! -e $src_file ]
then
	echo "$script_name error: file '$src_file' do not exists" >&2
	exit -1
fi

cp "$src_file" "$dst_file"
ret=$?

if [ $ret = 0 ]
then
	echo "$script_name: file '$dst_file' saved."
else
	echo "$script_name error: failed to save file '$dst_file'" >&2
fi

# Write the new calibration file to the FLASH.
# Create tarball from eeprom bins
cd /tmp

if [ "$burn_mode" = "eeprom_tar" ]
then
	print2log $interface_index DEBUG "$script_name: copy eeprom.tar.gz /nvram/"
	tar czf eeprom.tar.gz cal_*.bin
	cp /tmp/eeprom.tar.gz /nvram/
	sync
elif [ "$burn_mode" = "cal_wlan" ]
then
	print2log $interface_index DEBUG "$script_name: copy cal_*.bin /nvram/etc/wave_cal/"
	cp /tmp/cal_*.bin /nvram/etc/wave_cal/
	sync
else
	print2log $interface_index DEBUG "$script_name: use upgrade to burn flash"
	# make sure that it is not tri-band, before renaming to cal_wlan1.bin
	if [ -e /tmp/cal_wlan2.bin ] && [ ! -e /tmp/cal_wlan4.bin ]
	then
		mv /tmp/cal_wlan2.bin /tmp/cal_wlan1.bin
	fi
	tar czf $eeprom_tar cal_*.bin
	if [ $? != 0 ]
	then
		echo "$script_name: failed to create $eeprom_tar" >&2
		exit -1
	fi
	cd - > /dev/null
	upgrade /tmp/$eeprom_tar $eeprom_partition 0 0
	if [ $? != 0 ]
	then
		echo "$script_name: the partition $eeprom_partition doesn't exist and cannot be created" >&2
		exit -1
	fi
fi

exit $ret
