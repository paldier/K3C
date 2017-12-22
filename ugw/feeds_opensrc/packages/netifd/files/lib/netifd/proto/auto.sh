#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_auto_init_config() {
	renew_handler=1

	proto_config_add_string 'ipaddr:ipaddr'
	proto_config_add_string 'hostname:hostname'
	proto_config_add_string clientid
	proto_config_add_string vendorid
	proto_config_add_boolean 'broadcast:bool'
	proto_config_add_string 'reqopts:list(string)'
	proto_config_add_string iface6rd
	proto_config_add_string sendopts
	proto_config_add_boolean delegate
	proto_config_add_string zone6rd
	proto_config_add_string zone
	proto_config_add_string mtu6rd
	proto_config_add_string customroutes
}

proto_auto_setup() {
	local config="$1"
	local iface="$2"

	local ipaddr hostname clientid vendorid broadcast reqopts iface6rd sendopts delegate zone6rd zone mtu6rd customroutes
	json_get_vars ipaddr hostname clientid vendorid broadcast reqopts iface6rd sendopts delegate zone6rd zone mtu6rd customroutes

	local opt dhcpopts
	for opt in $reqopts; do
		append dhcpopts "-O $opt"
	done

	for opt in $sendopts; do
		append dhcpopts "-x $opt"
	done

	[ "$broadcast" = 1 ] && broadcast="-B" || broadcast=
	[ -n "$clientid" ] && clientid="-x 0x3d:${clientid//:/}" || clientid="-C"
	[ -n "$iface6rd" ] && proto_export "IFACE6RD=$iface6rd"
	[ "$iface6rd" != 0 -a -f /lib/netifd/proto/6rd.sh ] && append dhcpopts "-O 212"
	[ -n "$zone6rd" ] && proto_export "ZONE6RD=$zone6rd"
	[ -n "$zone" ] && proto_export "ZONE=$zone"
	[ -n "$mtu6rd" ] && proto_export "MTU6RD=$mtu6rd"
	[ -n "$customroutes" ] && proto_export "CUSTOMROUTES=$customroutes"
	[ "$delegate" = "0" ] && proto_export "IFACE6RD_DELEGATE=0"
	
	proto_export "INTERFACE=$config"
	
	source /lib/netifd/proto/get_ltewan_ip.sh
	# get APN name from opt/lantiq/etc/cell_wan_dev_intel_at.conf
	lte_wan_ip="0.0.0.0"
	get_lte_wan_ip
	if [ $? -eq 0 ]; then
 	  proto_init_update "$ifname" 1
	  proto_add_ipv4_address "$lte_wan_ip" "${subnet:-255.255.255.255}"
	  # target, mask, source, ip
	  #proto_add_ipv4_route "$lte_wan_ip" 32 "" "$lte_wan_ip"
	  proto_add_ipv4_route "0.0.0.0" 0 "$lte_wan_ip" "$lte_wan_ip"
	  echo "$interface=> $ifname - $lte_wan_ip up dns1: $lte_wan_dns1 dns2: $lte_wan_dns2" > /dev/console

	  if [ "$lte_wan_dns1" = "0.0.0.0" ] || [ -z "$lte_wan_dns1" ]; then
	  	echo "zero DNS1"
	  else
	  	 proto_add_dns_server "$lte_wan_dns1"
	  fi

	  if [ "$lte_wan_dns2" = "0.0.0.0" ] || [ -z "$lte_wan_dns2" ]; then
	  	echo "zero DNS2"
	  else
	  	proto_add_dns_server "$lte_wan_dns2"
	  fi
	
	  proto_send_update "$interface"
	fi
}

proto_auto_renew() {
	local interface="$1"
	# SIGUSR1 forces udhcpc to renew its lease
	local sigusr1="$(kill -l SIGUSR1)"
	[ -n "$sigusr1" ] && proto_kill_command "$interface" $sigusr1
}

proto_auto_teardown() {
	local interface="$1"
	proto_kill_command "$interface"
}

add_protocol auto
