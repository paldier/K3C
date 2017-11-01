#!/bin/sh
automation_log()
{
	# Do not add more unless it is really necessary
	echo " ====================== cat /var/log/messagews ======================="
	cat /var/log/messages*
	echo " ====================== ps ======================="
	ps
	echo " ====================== DSL Status ======================="
	cat /tmp/adsl_status
	echo " ====================== ifconfig ======================="
	ifconfig
	echo " ====================== cat /tmp/system_status ======================="
	cat /tmp/system_status
	echo " ====================== route -n  ======================="
	route -n
}

case $1 in
	"mini")
		automation_log
	;;
	*)
		echo " ====================== version.sh ======================="
		version.sh
		automation_log
		echo " ====================== cat /flash/rc.conf ======================="
		cat /flash/rc.conf
		echo " ====================== ls -l /tmp/ ======================="
		ls -l /tmp/
		echo " ====================== cat /proc/modules ======================="
		cat /proc/modules
		echo " ====================== cat /proc/meminfo  ======================="
		cat /proc/meminfo
		echo "================== cat /proc/slabinfo ======================"
		cat /proc/slabinfo
		echo " ====================== brctl show  ======================="
		brctl show
		echo "================== cat /proc/eth/genconf =================="
		cat /proc/eth/genconf
		echo "================== cat /proc/ppa/api/version =================="
		/sbin/ppacmd getversion
		echo "================== ppa QoS configuration =================="
		cat /proc/ppa/hal/*/qos
		echo "================== cat /proc/ppa/api/netif =================="
		cat /proc/ppa/api/netif
		echo "================== cat /proc/eth/mib =================="
		cat /proc/eth/mib
		echo "================== cat /proc/eth/route =================="
		cat /proc/eth/route
;;
	*)
;;
esac
