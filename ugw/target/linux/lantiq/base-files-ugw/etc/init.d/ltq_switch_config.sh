#!/bin/sh
# 
# Author: Kamal Eradath 
# This script reads the current wan mode and sets the swith confgurations accordingly.
# It call functions defined switch_unified.sh for switch configuration.
# Revision History:
# Date: 19-12-2012 initial version
 
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

if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
. /etc/rc.d/ltq_switch_functions.sh 2>/dev/null
fi

# need to move this to multicast
platform=${CONFIG_IFX_MODEL_NAME%%_*}
if [ "$platform" = "DANUBE" -o "$platform" = "AMAZON" -o "$platform" = "TP-VE" -o "$platform" = "GW188" ]; then
	target=$platform
else
	target=`echo $platform | cut -c -4`
fi

if [ "$CONFIG_WAN" = "0" -o "$SEC_WAN" = "0" ] && [ "$target" = "VRX2" ]; then
	config_vrx_pppoa
fi
#end

select_wan_mode 

case $1 in
"do_vlan_separation")
	enable_separation	
;;
"undo_vlan_separation")
	disable_separation
;;
"do_driver_separation")
	enable_driver_separation
;;
"undo_driver_separation")
	disable_driver_separation
;;
"do_switch_config")
 	switch_init
	
	if [ "$lan_port_sep_enable" = "1" ]; then
		enable_separation
       	elif  [ "$lan_port_sep_drv" = "1" ];then
		if [ `lsmod |grep -q ppa;echo $?` = 0 ];then
			enable_driver_separation
		else
			echo "No PPA modules found hence no lan port separation"
		fi
	fi
	
	if [ "$wanphy_phymode" = "1" ]; then
	    enable_mii0_conf
	    . /etc/rc.d/ppa_config.sh addlan eth0.$lan_vid_all
	fi	
#pramod to confirm
#	/usr/sbin/iptables -t nat -I IFX_NAPT_PREROUTING_LAN -i eth0 -j ACCEPT

;;
"undo_switch_config")
	
	if [ "$wanphy_phymode" = "1" ]; then
	    . /etc/rc.d/ppa_config.sh dellan eth0.$lan_vid_all
	    disable_mii0_conf
	fi	
	
	if [ "$lan_port_sep_enable" = "1" ]; then
		disable_separation
       	elif  [ "$lan_port_sep_drv" = "1" ];then
		disable_driver_separation
	fi
	switch_uninit
	;;
*)
	;;
esac
