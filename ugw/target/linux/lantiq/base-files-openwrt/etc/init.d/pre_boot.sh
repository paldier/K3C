#!/bin/sh /etc/rc.common

START=10

. /lib/functions/common_utils.sh
. /lib/functions/dsl_fapi.sh

## First bootup script
first_bootup()
{
	local fb

	fb=`get_uboot_env firstboot 1`
	[ $fb = 0 ] && { do_log "This is not first boot"; return 0; }

	do_log "First time booting..."

	dsl_if_action "ATM" "CONFIG"
	dsl_if_action "PTM" "CONFIG"

	set_uboot_env "firstboot" "0"
}

## Normal boot-up sequence
bootup()
{
	makedevs -d /etc/device_table.txt /
}

start()
{
	first_bootup
	bootup
}
