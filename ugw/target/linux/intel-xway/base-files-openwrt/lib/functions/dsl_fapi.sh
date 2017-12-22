#!/bin/sh

. /lib/functions/common_utils.sh

#switch_cli GSW_QOS_QUEUE_PORT_GET nPortId=5; switch_cli GSW_QOS_SHAPER_QUEUE_GET nQueueId=20; switch_cli GSW_QOS_SHAPER_CFG_GET nRateShaperId=5; switch_cli dev=0 GSW_PORT_LINK_CFG_GET nPortId=4

FOR_ALL_ATM_IN_XTM_CONF()
{
	local __atm; local __idx

	for __atm in `uci show xtm | sed -n 's/=atmdevice//p' | sed 's/xtm\.//g'`; do
		__idx=`echo ${__atm} | sed 's/atm//g'`
		$1 ${__atm} ${__idx}
	done
}

FOR_ALL_PTM_IN_XTM_CONF()
{
	local __ptm; local __idx

	for __ptm in `uci show xtm | sed -n 's/=ptmdevice//p' | sed 's/xtm\.//g'`; do
		__idx=`echo ${__ptm} | sed 's/ptm//g'`
		$1 ${__ptm} ${__idx}
	done
}

FOR_ALL_ATM_IN_NETWORK_CONF()
{
	local __atm; local __idx;

	for __atm in `uci show network | awk -F'[.=]' '{if(NF==3){print $2}}' | grep nas[0-9]*wan`; do
		__idx=`echo ${__ptm} | sed -e 's/nas//g' -e 's/wan//g'`
		$1 ${__atm} ${__idx}
	done
}

FOR_ALL_PTM_IN_NETWORK_CONF()
{
	local __ptm; local __idx;

	for __ptm in `uci show network | awk -F'[.=]' '{if(NF==3){print $2}}' | grep ptm[0-9]*wan`; do
		__idx=`echo ${__ptm} | sed -e 's/ptm//g' -e 's/wan//g'`
		$1 ${__ptm} ${__idx}
	done
}

do_br2684ctl()
{
	local idx=$2
	local vpi; local vci; local enc; local qos; local td
	local servicecat; local pcr; local scr; local mbs; local mcr
	local qos_str=""

	## ATM parameters
	vpi=`uci get xtm.atm${idx}.vpi`; [ -z $vpi ] && { do_log "ATM $idx vpi is not configured"; return 1; }
	vci=`uci get xtm.atm${idx}.vci`; [ -z $vci ] && { do_log "ATM $idx vci is not configured"; return 1; }
	enc=`uci get xtm.atm${idx}.enc`; [ -z $enc ] && { do_log "ATM $idx enc is not configured"; return 1; }
	td=`uci get xtm.atm${idx}.td`; [ -z $enc ] && { do_log "ATM $idx td is not configured"; return 1; }

	# 0 - LLC/SNAP (Default), 1 - VCMux
	if [ "$enc" = "vcmux" ]; then
		enc=1
	else
		enc=0
	fi

	## Traffic description parameters
	servicecat=`uci get xtm.${td}.servicecat`; [ -z $servicecat ] && { do_log "$td servicecat is not configured"; return 1; }
	if [ "$servicecat" = "ubr" ]; then
		qos_str="UBR,aal5"
	elif [ "$servicecat" = "ubr_pcr" ]; then
		pcr=`uci get xtm.${td}.pcr`; [ -z $pcr ] && { do_log "$td pcr is not configured"; return 1; }
		qos_str="UBR+,aal5:max_pcr=$pcr"
	elif [ "$servicecat" = "cbr" ]; then
		pcr=`uci get xtm.${td}.pcr`; [ -z $pcr ] && { do_log "$td pcr is not configured"; return 1; }
		qos_str="CBR,aal5:max_pcr=$pcr"
	elif [ "$servicecat" = "rtvbr" ]; then
		pcr=`uci get xtm.${td}.pcr`; [ -z $pcr ] && { do_log "$td pcr is not configured"; return 1; }
		scr=`uci get xtm.${td}.scr`; [ -z $scr ] && { do_log "$td scr is not configured"; return 1; }
		mbs=`uci get xtm.${td}.mbs`; [ -z $mbs ] && { do_log "$td mbs is not configured"; return 1; }
		qos_str="RT-VBR,aal5:max_pcr=$pcr,scr=$scr,mbs=$mbs"
	elif [ "$servicecat" = "nrtvbr" ]; then
		pcr=`uci get xtm.${td}.pcr`; [ -z $pcr ] && { do_log "$td pcr is not configured"; return 1; }
		scr=`uci get xtm.${td}.scr`; [ -z $scr ] && { do_log "$td scr is not configured"; return 1; }
		mbs=`uci get xtm.${td}.mbs`; [ -z $mbs ] && { do_log "$td mbs is not configured"; return 1; }
		qos_str="NRT-VBR,aal5:max_pcr=$pcr,scr=$scr,mbs=$mbs"
	else
		do_log "TD $idx invalid servicecat $servicecat"
		return 1
	fi

	# Possible ATM Classes include UBR (Default), CBR, RT-VBR, NRT-VBR & UBR+
	#ATM_QOS="UBR,aal5:"
	#ATM_QOS="UBR,aal5:max_pcr=1250,cdv=100"
	#ATM_QOS="CBR,aal5:min_pcr=1250,cdv=100"
	#ATM_QOS="NRT-VBR,aal5:max_pcr=1250,cdv=100,min_pcr=1000,scr=1100,mbs=1000"
	#ATM_QOS="RT-VBR,aal5:max_pcr=1250,cdv=100,scr=1100,mbs=1000"
	#ATM_QOS="UBR+,aal5:max_pcr=1250,cdv=100,min_pcr=1000"


	# b) Background
	# p) 0 - Routed, 1 - Bridged (Default)
	do_log "/usr/sbin/br2684ctl -b -p 1 -c $idx -e $enc -q ${qos_str} -a 0.${vpi}.${vci}"
	/usr/sbin/br2684ctl -b -p 1 -c $idx -e $enc -q ${qos_str} -a 0.${vpi}.${vci}
}

## This will add/prepare atm (nas) interfaces configured in /etc/config/xtm
atm_init_interface()
{
	## Delete all nas*
	do_log "/usr/sbin/br2684ctl -K"
	/usr/sbin/br2684ctl -K
	rm -rf /var/run/br2684ctld.pid

	FOR_ALL_ATM_IN_XTM_CONF do_br2684ctl
}

## This will set the dsl interface configuration in /etc/config/network
set_dsl_network_config()
{
	local name="`echo $1 | sed 's/atm/nas/g'`wan";
	local ifname="`echo $1 | sed 's/atm/nas/g'`";
	local macaddr; local oldif

	oldif=`uci show network | grep -w ifname=${name} | awk -F'.' '{print $2}'`
	[ ! -z $oldif ] && macaddr=`uci get network.${oldif}.macaddr`
	[ -z $macaddr ] && { macaddr=`gen_new_mac`; }

	do_log "setting dsl config for ${name} ifname: ${ifname} macaddr: ${macaddr}"

	uci set network.${name}=interface
	uci set network.${name}.proto='dhcp'
	uci set network.${name}.ifname=${ifname}
	uci set network.${name}.macaddr=${macaddr}
	uci commit network
}

## This sets the MAC address to dsl inteface
dsl_ifconfig_up()
{
	local name=$1; local idx=$2; local macaddr; local ifname;

	macaddr=`uci get network.${name}.macaddr`
	ifname=`uci get network.${name}.ifname`
	do_log "/sbin/ifconfig ${ifname} down hw ether ${macaddr}"
	/sbin/ifconfig ${ifname} down hw ether ${macaddr}
	/sbin/ifconfig ${ifname} up
}

## This makes the dsl interface up
dsl_ifup()
{
	local name=$1; local idx=$2;

	do_log "ifup ${name}"
	ifup "${name}"
}

## This makes the dsl interface down
dsl_ifdown()
{
	local name=$1; local idx=$2;

	do_log "ifdown ${name}"
	ifdown "${name}"
}

## This function will
##	1) Set the dsl configuration in network
##	2) Initialize the dsl interfaces
##	3) Reload network with new configuration
dsl_reload_network()
{
	FOR_ALL_ATM_IN_XTM_CONF set_dsl_network_config
	FOR_ALL_PTM_IN_XTM_CONF set_dsl_network_config
	atm_init_interface
	/etc/rc.d/S20network reload
}

## This function will help to modify dsl interface stat
##	ARG1		ARG2	Meaning
##	ATM/PTM		UP		It will configure the mac address to related interface(s) & make it up (not ifup)
##	ATM/PTM		IFUP	It will do "ifup" for related interface(s)
##	ATM/PTM		IFDOWN	It will do "ifdown" for related interface(s)
##	ATM/PTM		CONFIG	It will re-configure dsl configuration in /etc/config/network
dsl_if_action()
{
	local proto="$1"

	case "$2" in
		"UP")
			[ "$proto" = "ATM" ] && { FOR_ALL_ATM_IN_NETWORK_CONF dsl_ifconfig_up; return 0; }
			[ "$proto" = "PTM" ] && { FOR_ALL_PTM_IN_NETWORK_CONF dsl_ifconfig_up; return 0; }
			return 1
		;;
		"IFUP")
			[ "$proto" = "ATM" ] && { FOR_ALL_ATM_IN_NETWORK_CONF dsl_ifup; return 0; }
			[ "$proto" = "PTM" ] && { FOR_ALL_PTM_IN_NETWORK_CONF dsl_ifup; return 0; }
			return 1
		;;
		"IFDOWN")
			[ "$proto" = "ATM" ] && { FOR_ALL_ATM_IN_NETWORK_CONF dsl_ifdown; return 0; }
			[ "$proto" = "PTM" ] && { FOR_ALL_PTM_IN_NETWORK_CONF dsl_ifdown; return 0; }
			return 1
		;;
		"CONFIG")
			[ "$proto" = "ATM" ] && { FOR_ALL_ATM_IN_XTM_CONF set_dsl_network_config; return 0; }
			[ "$proto" = "PTM" ] && { FOR_ALL_PTM_IN_XTM_CONF set_dsl_network_config; return 0; }
			return 1
		;;
	esac

	return 1
}

## This will start/stop dsp fapi
dsl_fapi()
{
	do_log "/opt/lantiq/etc/init.d/ltq_cpe_control_init.sh $@"

	case "$1" in
		"start")
			/opt/lantiq/etc/init.d/ltq_load_cpe_mei_drv.sh $@
			/opt/lantiq/etc/init.d/ltq_load_dsl_cpe_api.sh $@
			/opt/lantiq/etc/init.d/ltq_cpe_control_init.sh $@
		;;
		*)
			/opt/lantiq/etc/init.d/ltq_cpe_control_init.sh $@
		;;
	esac
}
