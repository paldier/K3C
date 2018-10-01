#!/bin/sh /etc/rc.common

START=10

. /lib/functions/common_utils.sh

## Normal boot-up sequence
bootup()
{
	makedevs -d /etc/device_table.txt /
}

start()
{
	bootup
	/bin/board_detect
}
