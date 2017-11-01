#!/bin/sh

LAN_INTERFACE="eth0_1 eth0_2 eth0_3 eth0_4"
WAN_INTERFACE="eth1"

get_uboot_env()
{
	local env=$1; local def=$2; local val

	return $val;
	## For this uboot_env is required
	val=`/opt/lantiq/usr/sbin/uboot_env --get --name $env 2>/dev/null`
	([ $? -ne 0 ] || [ -z $val ]) && { echo $def; return 1; }

	echo $val
	return 0
}

set_uboot_env()
{
	local env=$1; local val=$2; local ret

	return 0
	## For this uboot_env is required
	/opt/lantiq/usr/sbin/uboot_env --set --name $env --value $val
	ret=$?
	[ $ret -ne 0 ] && { /opt/lantiq/usr/sbin/uboot_env --add --name $env --value $val; ret=$?; }
	return $ret
}

start_daemon()
{
	export LD_LIBRARY_PATH=/opt/lantiq/lib:/opt/lantiq/usr/lib:${LD_LIBRARY_PATH}
	export PATH=$PATH:/opt/lantiq/sbin:/opt/lantiq/usr/sbin:/opt/lantiq/bin
	$@ &
}

gen_new_mac()
{
	local MAC_ADDR=""

	if [ ! -f /tmp/basemacaddr ]; then
		MAC_ADDR=`cat /proc/cmdline | grep -E -o '([a-fA-F0-9]{2}\:){5}[a-fA-F0-9]{2}'`
		MAC_ADDR="${MAC_ADDR:0:15}`printf "%02X" $((0x${MAC_ADDR:15:2} + 0x6))`"
	else
		MAC_ADDR=`cat /tmp/basemacaddr`
		MAC_ADDR="${MAC_ADDR:0:15}`printf "%02X" $((0x${MAC_ADDR:15:2} + 0x1))`"
	fi
	echo $MAC_ADDR > /tmp/basemacaddr
	echo $MAC_ADDR
}

do_log()
{
	echo "`date` : $0 : $@" >> /tmp/fapi.log
	echo "`date` : $0 : $@"
}
