#!/bin/sh

. /lib/functions/common_utils.sh

## Initilize system fapi
init_sys_fapi()
{
	export LD_LIBRARY_PATH=/opt/lantiq/lib:/opt/lantiq/usr/lib:${LD_LIBRARY_PATH}
	export PATH=$PATH:/opt/lantiq/sbin:/opt/lantiq/usr/sbin:/opt/lantiq/bin

	## initialize system fapi modules
	sys_cli eth --sys_init
	sys_cli eth --ppa_init
}

config_sys_fapi()
{
	export LD_LIBRARY_PATH=/opt/lantiq/lib:/opt/lantiq/usr/lib:${LD_LIBRARY_PATH}
	export PATH=$PATH:/opt/lantiq/sbin:/opt/lantiq/usr/sbin:/opt/lantiq/bin

	local ifname

	## Add all LAN & WAN interfaces to system fapi
	for ifname in $LAN_INTERFACE; do
		do_log "sys_cli eth --ppa_add interface=$ifname hook=LAN"
		sys_cli eth --ppa_add interface=$ifname hook=LAN
	done
	sys_cli eth --ppa_add interface=br-lan hook=LAN

	for ifname in $WAN_INTERFACE; do
		do_log "sys_cli eth --ppa_add interface=$ifname hook=WAN"
		sys_cli eth --ppa_add interface=$ifname hook=WAN
	done

	## Enable system fapi for LAN & WAN
	sys_cli eth --ppa_enable LANstatus=ENABLE WANstatus=ENABLE
}
