#!/bin/sh

. /lib/functions/common_utils.sh

switch_cli_set()
{
	do_log "switch_cli $@"
	switch_cli $@
}

init_ethrnet_switch()
{
#	switch_mii1_port="15"
#	switch_lan_ports="2 3 4 5"
	local pname=""; local enabled; local speed; local duplex; local wan; local td
	local port_dev=0; local port_id=0;

	for port in `uci show ethernet | sed -n 's/=port//p'`; do
		pname=`echo ${port} | awk -F'.' '{print $2}'`
		if [ $pname = "eth0_1" ]; then
			port_dev=0; port_id=2
		elif [ $pname = "eth0_2" ]; then
			port_dev=0; port_id=3
		elif [ $pname = "eth0_3" ]; then
			port_dev=0; port_id=4
		elif [ $pname = "eth0_4" ]; then
			port_dev=0; port_id=5
		elif [ $pname = "eth1" ]; then
			port_dev=1; port_id=15
		else
			do_log "ERROR: Invalid port name $pname"
			continue;
		fi

		enabled=`uci get ethernet.${pname}.enable`; [ -z $enabled ] && enabled=1
		speed=`uci get ethernet.${pname}.speed`; [ -z $speed ] && speed="auto"
		duplex=`uci get ethernet.${pname}.duplex`; [ -z $duplex ] && duplex="full"
		wan=`uci get ethernet.${pname}.wan`; [ -z $wan ] && wan=0
		td=`uci get ethernet.${pname}.td`
		softswitch=`uci get ethernet.${pname}.softswitch`; [ -z $softswitch ] && softswitch=0

		do_log "CONFIG: Port: $port Enabled: $enabled Speed: $speed Duplex: $duplex Wan: $wan TD: $td Softswitch: $softswitch"

		if [ $enabled -ne 1 ]; then
			## TODO: Disable port
			continue;
		fi

		if [ "$speed" != "auto" ]; then
			if [ "$duplex" = "half" ]; then duplex=1; else duplex=0; fi
			switch_cli_set dev=$port_dev GSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=1 eDuplex=$duplex bSpeedForce=1 eSpeed=$speed
		else
			## Set port to auto mode
			switch_cli_set dev=$port_dev GSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=0 bSpeedForce=0
		fi

		[ $wan == 0 ] && { switch_cli_set dev=$port_dev GSW_PORT_CFG_SET nPortId=$port_id bLearning=$softswitch; }

		if [ "$td" != "" ]; then
			local shaper_id=$port_id; local mbr; local mbs; local port_queue

			mbr=`uci get ethernet.${td}.mbr`
			mbs=`uci get ethernet.${td}.mbs`

			## Get current port QUEUE ID
			port_queue=`switch_cli GSW_QOS_QUEUE_PORT_GET nPortId=$port_id | awk -F'|' 'BEGIN { found=0; } { if (NF == 5) { if($3 == " Egress Queue ") { found=1; } else if (found == 1) {printf("%d\n", $3); found=0;}}}' | head -n 1`
			[ -z $port_queue ] && { do_log "ERROR: Can't find port_queue for $pname"; continue; }
			do_log "Port Queue: $port_queue"

			## Create SHAPER
			switch_cli_set GSW_QOS_SHAPER_CFG_SET nRateShaperId=$shaper_id bEnable=1 nCbs=$mbs nRate=$mbr

			## Assign SHAPER to QUEUE
			switch_cli_set GSW_QOS_SHAPER_QUEUE_ASSIGN nRateShaperId=$shaper_id nQueueId=$port_queue
		fi
	done
}
