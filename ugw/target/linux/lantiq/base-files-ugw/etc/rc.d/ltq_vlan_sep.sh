#!/bin/sh

echo "Env $ENVLOADED" > /dev/null
if [ ! "$ENVLOADED" ]; then
	if [ -r /etc/rc.conf ]; then
		 . /etc/rc.conf 2> /dev/null
		ENVLOADED="1"
	fi
fi

if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
fi

case $1 in
	"start")
		if [ "$lan_port_sep_enable" = "1" ]; then
			. /etc/init.d/ltq_switch_config.sh do_vlan_separation
        	elif  [ "$lan_port_sep_drv" = "1" ];then
			if [ `lsmod |grep -q ppa;echo $?` = 0 ];then
				. /etc/init.d/ltq_switch_config.sh do_driver_separation
			else
				echo "No PPA modules found hence no lan port separation"
			fi
		fi

	;;
	"stop")
		# Stop IPQoS if already enabled
		if [ $qm_enable -eq 1 ]; then
		#echo " DISABLING IPQOS "
			/etc/rc.d/ipqos_disable
		fi

		#Stop the WAN
		. /etc/rc.d/rc.bringup_wan stop

		. /etc/rc.d/rc.bringup_l2if stop

		#echo "sleep until all wan services are stopped"
			sleep 3
	
		if [ "$lan_port_sep_enable" = "1" ]; then
			. /etc/init.d/ltq_switch_config.sh undo_vlan_separation
        	elif  [ "$lan_port_sep_drv" = "1" ];then
			. /etc/init.d/ltq_switch_config.sh undo_driver_separation
		fi

		#stop the qos rate update script.
		killall qos_rate_update

		sleep 2
		
		#if PPA is enabled, enable hardware based QoS to be used later
		. /etc/init.d/ipqos_qprio_cfg.sh
	
		. /etc/rc.d/rc.bringup_l2if start

		#echo "Starting wan services"
		. /etc/rc.d/rc.bringup_wan start

		#echo "Starting IPQoS"
		/etc/rc.d/S56init_ipqos.sh start
	;;
	*)
	;;
esac

	
	
