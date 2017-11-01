#!/bin/sh

print_cmd()
{
        local t_name="$1";
        shift
        echo "<< $t_name START (`date`) >>"
        $@
        echo "<< $t_name END >>"
        echo;echo
}

print_cmd "version" version.sh

print_cmd "qos sllog" slloglevel sl_qos 7 2

print_cmd "qos fapilog" qoscfg -l 7 -t 2

print_cmd "meminfo" cat /proc/meminfo

print_cmd "MTD info" cat /proc/mtd

print_cmd "disk free df" df

print_cmd "mounts" cat /proc/mounts

print_cmd "modules" cat /proc/modules

print_cmd "Running process ps" ps

print_cmd "No. of SL libs" ls -al /opt/lantiq/servd/lib/

print_cmd "XML configs" ls -al /opt/lantiq/config/

print_cmd "SLs start order" cat /opt/lantiq/servd/etc/servd.conf

print_cmd "Available start-up scripts" ls -al /etc/rc.d/S*

print_cmd "Network config" cat /etc/config/network

print_cmd "ifconfig" ifconfig

print_cmd "brctl" brctl show

print_cmd "netstat" netstat -ltun

print_cmd "route" route -n

print_cmd "ppa api netif" cat /proc/ppa/api/netif

[ -f /etc/switchports.conf ] && {
	. /etc/switchports.conf
	for i in $switch_lan_ports; do print_cmd "switch port '$i' status" switch_cli GSW_PORT_LINK_CFG_GET nPortId=$i; done
	for i in $switch_mii1_port; do print_cmd "switch port '$i' status" switch_cli GSW_PORT_LINK_CFG_GET nPortId=$i dev=1; done
}

print_cmd "dmesg" dmesg

print_cmd "ubus methods list" ubus list -v

