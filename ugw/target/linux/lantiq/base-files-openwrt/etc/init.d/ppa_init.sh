#!/bin/sh /etc/rc.common
# Add Physical Interfaces to PPA via System FAPI CLI.
#

START=20

export PATH=/opt/lantiq/sbin:/opt/lantiq/usr/sbin:/opt/lantiq/bin:${PATH}
export LD_LIBRARY_PATH=/opt/lantiq/lib:/opt/lantiq/usr/lib:${LD_LIBRARY_PATH}

sys_cli_ppa_file=/tmp/ppa_cfg.conf

# Get ethsw separated LAN interfaces and add with lower layer.
add_ethsw_alias()
{
	local _iface _ival;
	_iface=$(ip link|grep @$1:|cut -d: -f2)
	[ -n "$_iface" ] && {
		for _ival in ${_iface//@$1}; do
			echo "add_lan:$_ival $1" >> $sys_cli_ppa_file
		done
	}
}

prepare_ppa_config() {
	local section="$1"
	local section_type
	local i

	config_get section_type $section type
	config_get ifname $section ifname

	if [ $section_type == "lan" ]
	then
		for i in $ifname
		do
			echo "add_base:$i" >> $sys_cli_ppa_file
			add_ethsw_alias $ifname
		done
	elif [ $section_type == "wan" ]
	then
		for i in $ifname
		do
			echo "add_wan:$i" >> $sys_cli_ppa_file
		done
	fi
}

start() {
	config_load ppa
	> $sys_cli_ppa_file
	config_foreach prepare_ppa_config ppa
	sys_cli eth -F $sys_cli_ppa_file
}

stop() {
	sys_cli eth -P 1
}
