#!/bin/sh

xl2tpd_options_file="/etc/ppp/options.xl2tpd"
xl2tpd_conf_file="/etc/xl2tpd/xl2tpd.conf"

oper=$1
lsn_ip=$2
server_name=$3

help()
{
	echo "Invalid argument"
	echo "	./l2tp.sh start <lsn ipaddr> <server name>"
	echo "	./l2tp.sh stop"
	echo ""
	exit
}

create_l2tp_options()
{
	echo > $xl2tpd_options_file
	echo "noauth" >> $xl2tpd_options_file
	echo "crtscts" >> $xl2tpd_options_file
	echo "idle 1800" >> $xl2tpd_options_file
	echo "mtu 1410" >> $xl2tpd_options_file
	echo "mru 1410" >> $xl2tpd_options_file
	echo "nodefaultroute" >> $xl2tpd_options_file
	echo "debug" >> $xl2tpd_options_file
	echo "lock" >> $xl2tpd_options_file
}

create_l2tp_conf()
{
	echo > $xl2tpd_conf_file
	echo "[global]" >> $xl2tpd_conf_file
	echo "[lac $server_name]" >> $xl2tpd_conf_file
	echo "lns = $lsn_ip" >> $xl2tpd_conf_file
	echo "ppp debug = yes" >> $xl2tpd_conf_file
	echo "pppoptfile = /etc/ppp/options.xl2tpd" >> $xl2tpd_conf_file
}

start_l2tp()
{
	## Stop l2tp service
	echo "Stop l2tp ..." > /dev/console
	killall -9 xl2tpd 2>/dev/null
	
	## Start l2tp service
	echo "Start l2tp ..." > /dev/console
	
	# Create l2tp configuration files
	create_l2tp_options
	create_l2tp_conf

	# Start xl2tpd
	xl2tpd -D &
	sleep 1
	echo "c $server_name" > /var/run/xl2tpd/l2tp-control

	# Add Firewall Rules	
	sleep 2
	iptables -I delegate_forward -i ppp1 -j ACCEPT
	iptables -A delegate_input -i ppp1 -j zone_wan_input
	iptables -A delegate_output -o ppp1 -j zone_wan_output
	iptables -A zone_wan_dest_ACCEPT -o ppp1 -j ACCEPT
}

stop_l2tp()
{
	## Stop l2tp service
	echo "Stop l2tp ..." > /dev/console
	killall -9 xl2tpd 2>/dev/null
}

[ $# -lt 1 ] && help
case $oper in
	start)
		[ $# -ne 3 ] && help
		start_l2tp
	;;
	
	stop)
		stop_l2tp
	;;
	
	*)
		help
esac
