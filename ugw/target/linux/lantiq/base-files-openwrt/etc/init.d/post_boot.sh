#!/bin/sh /etc/rc.common

START=99

. /lib/functions/common_utils.sh
. /lib/functions/dsl_fapi.sh
. /lib/functions/ethernet_fapi.sh
. /lib/functions/system_fapi.sh

## Post initialization part
post_init()
{
	init_ethrnet_switch
	config_sys_fapi
	dsl_fapi "start"
}

start() {
	post_init
}
