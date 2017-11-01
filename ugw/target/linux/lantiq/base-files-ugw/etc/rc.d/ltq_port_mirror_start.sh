#!/bin/sh

if [ ! "$CONFIGLOADED" ]; then
        if [ -r /etc/rc.d/config.sh ]; then
                . /etc/rc.d/config.sh 2>/dev/null
                CONFIGLOADED="1"
        fi
fi

cpu_port=6
max_ports=5
mirror_mode=1

display_switch_layout() {

	if [ "$CONFIG_IFX_CONFIG_CPU" = "XRX288" ]; then

		echo "Switch port numbers for xRX200"
		echo "|-+ +-| +---+ +---+ +----+----+----+----+----+"
		echo "|       | |+  | |+  | || | || | || | || | || |"
		echo "|       | ||  | ||  |    |    |    |    |    |"
		echo "|-|||-| +---+ +---+ +----+----+----+----+----+"
		echo "  DSL    USB#1 USB#0   4    2    1    0    5  "
	
	elif [ "$CONFIG_IFX_CONFIG_CPU" = "XRX3XX" ]; then

		echo "Switch port numbers for xRX300"
		echo "|-+ +-| +----+----+----+----+----+ +---+ +---+"
		echo "|       | || | || | || | || | || | | |+  | |+ "
		echo "|       |    |    |    |    |    | | ||  | || "
		echo "|-|||-| +----+----+----+----+----+ +---+ +---+"
		echo "  DSL      1    3    2    0    5   USB#1 USB#0"
	fi
}

show_menu() {
	#clear
	echo "============================="
	echo "Please select below options :"
	echo "============================="
	echo "1 - Enable Port mirror"
	echo "2 - Display Switch layout"
	echo "3 - Disable Port mirror (reboots device!)"
	echo "4 - Help"
	echo "5 - Exit"
	echo "============================="
}

usage() {
	echo "Usage: $0 <interface type> <mirror port>"
	echo "Choose mirror logic: 1-Switch, 2-Software: "
}

help_info() {

	echo "===================================="
	echo "Switch Port mirror :" 
	echo "			     Supports to configure and choose monitor ports & mirror port among the LAN switch"
	echo "			     Supports mirroring with acceleration enabled"
	echo "			     Supports to configure switch port only (no WLAN)"
	echo "===================================="
}

read_options() {

	local choice
	read -p "Enter choice [1 to 5] :" choice
	case $choice in

		1) configure_mirror ;;
		2) display_switch_layout ;;
		3) disable_mirror ;;
		4) help_info ;;
		5) exit 0
	esac
}

display_config() {
	echo "======================"
	echo "Configuration details :"
	echo "monitor ports : $g_p1 $g_p2 $g_p3 $g_p4 $g_p5"
	echo "mirror port   : $g_mp"
	echo "======================"
}

reset_config() {

        # re-set switch mirror register
	switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x453 nData=0x0

	# re-set previous monitor port

	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=0 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=1 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=2 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=4 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=5 eEnable=1 ePortMonitor=0

	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=6 eEnable=1 ePortMonitor=0

	# re-set previous mirror port

	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=0 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=1 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=2 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=4 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=5 bMonitorPort=0
}

disable_mirror() {
	reset_config
	switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x453 nData=0x3FF

	if [ "$mirror_mode" = "2" ]; then
	    echo none > /proc/mirror

            switch_cli IFX_FLOW_PCE_RULE_DELETE pattern.nIndex=63 pattern.bEnable=1 pattern.bMAC_SrcEnable=1 pattern.nMAC_Src=00:00:00:00:00:00 action.ePortMapAction=4 action.nForwardPortMap=0x10 pattern.bPortIdEnable=1 pattern.nPortId=6
	fi

	reboot -f
}

configure_mirror() {

	local logic_type
	local intf_type
	local monitor_port
	local mirror_port
	local recheck
	local p1
	local p2
	local p3
	local p4
	local p5

	read -p "Choose mirror logic: 1-Switch, 2-Software :" logic_type

	mirror_mode=$logic_type
	
	if [ "$logic_type" = "1" ]; then

			
		read -p "Enter ports to be monitored [add space between ports: 0 1 2 4 5] :" p1 p2 p3 p4 p5


		# re-set previous configuration, if any
		reset_config

		# set ports-0,1,2,4,5 and CPU port 6 as monitored ports

		if [ ! -z "$p1" ]; then
		    	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=${p1} eEnable=1 ePortMonitor=3
		fi    
		if [ ! -z "$p2" ]; then
		    	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=${p2} eEnable=1 ePortMonitor=3
		fi
		if [ ! -z "$p3" ]; then
		    	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=${p3} eEnable=1 ePortMonitor=3
		fi
		if [ ! -z "$p4" ]; then
		    	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=${p4} eEnable=1 ePortMonitor=3
		fi
		if [ ! -z "$p5" ]; then
		    	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=${p5} eEnable=1 ePortMonitor=3
		fi

		# by default enabled, cpu port-6
		switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=${cpu_port} eEnable=1 ePortMonitor=3

                # re-set switch mirror register
	        switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x453 nData=0x0
		
		read -p "Enter choice [mirror port: choose one of 0 1 2 4 5] :" mirror_port
		# set mirror port
		case "$mirror_port" in
			0)
				switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=${mirror_port} bMonitorPort=1
				;;
			1)
				switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=${mirror_port} bMonitorPort=1
				;;
			2)
				switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=${mirror_port} bMonitorPort=1
				;;
			4)
				switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=${mirror_port} bMonitorPort=1
				;;
			5)
				switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=${mirror_port} bMonitorPort=1
		esac

		# copy local var
		g_p1=$p1
		g_p2=$p2
		g_p3=$p3
		g_p4=$p4
		g_p5=$p5
		g_mp=$mirror_port
		
		# common func to display
		#display_config
		
	elif [ "$logic_type" = "2" ]; then

        	# re-set switch mirror register to default value at system bringup
		switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x453 nData=0x3FF

		echo "Software based port mirroring..."
		read -p "Choose one port mirror interface : [eth0/wlan0/...]" intf_type

		echo "***** Disabling acceleration *****"
		ppacmd exit

		echo $intf_type > /proc/mirror
		echo "Configured mirror port: $intf_type"
		
		# redirect traffic with src mac all 0's (mirrored packets) to CPU port
		# FIXME: may affect original traffic which contain src mac all 0's
switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=63 pattern.bEnable=1 pattern.bMAC_SrcEnable=1 pattern.nMAC_Src=00:00:00:00:00:00 action.ePortMapAction=4 action.nForwardPortMap=0x10 pattern.bPortIdEnable=1 pattern.nPortId=6

	fi
}

while true
do
	show_menu
	read_options
done

