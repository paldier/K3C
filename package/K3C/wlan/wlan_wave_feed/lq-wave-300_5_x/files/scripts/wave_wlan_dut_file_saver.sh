#!/bin/sh

script_name="wave_wlan_dut_drvctrl.sh"

# Defines
[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh

# Define local parameters
local firmware_dir src_file dst_file ret command eeprom_partition eeprom_tar

firmware_dir=`grep FIRMWARE_DIR= /etc/hotplug/firmware.agent | sed 's/FIRMWARE_DIR=//'`
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

exit $ret
