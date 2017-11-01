#!/bin/sh /etc/rc.common

START=10

. /lib/functions/common_utils.sh
. /lib/functions/dsl_fapi.sh
. /lib/functions/ethernet_fapi.sh
. /lib/functions/system_fapi.sh

## First bootup script
first_bootup()
{
	local fb

	fb=`get_uboot_env firstboot 1`
	[ $fb = 0 ] && { do_log "This is not first boot"; return 0; }

	do_log "First time booting..."

	## By default uhttpd https_redirection is on :(
	uci set uhttpd.main.redirect_https='0'; uci commit uhttpd

	dsl_if_action "ATM" "CONFIG"
	dsl_if_action "PTM" "CONFIG"

	set_uboot_env "firstboot" "0"
}

## Load extra modules manually
load_modules()
{
	insmod /lib/modules/*/vrx318_tc.ko
}

## Normal boot-up sequence
bootup()
{
	makedevs -d /etc/device_table.txt /
	load_modules
	init_sys_fapi
}

start()
{
	first_bootup
	bootup
}
