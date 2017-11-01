#!/bin/sh
# Usage:ethwan_rate_update.sh 

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
#Assign variables coming from command line
PortId=5
DataWt=70
AckWt=30
WANInt="eth1"
oldRate=0
oldStat=0
oldPPAStat=0
IMQDev=2
plat=xrx300_d5

htbimq_create()
{
  # qdisc creation Eg: htbimq_create <IMQIdx> <TotalRate> <Calss1Rate> <class2Rate> <WanName>	
	IMQIdx=$1
	TotalRate=$2
	Class1Rate=$3
	Class2Rate=$4
	WanName=$5
	tc qdisc add dev imq$IMQIdx root handle 1: htb default 2
    tc class add dev imq$IMQIdx parent 1: classid 1:1 htb rate ${Class1Rate}kbit quantum 1500
    tc class add dev imq$IMQIdx parent 1: classid 1:2 htb rate ${Class2Rate}kbit quantum 1500
    tc filter add dev imq$IMQIdx parent 1: protocol ip u32 match ip protocol 6 0xff match u16 0x0000 0xff00 at 2 flowid 1:1
    iptables -t mangle -A POSTROUTING -o $WanName -j IMQ --todev $IMQIdx
}

htbimq_delete()
{
    # Function to delete the qdisc. Eg: htbimq_delete 2
	IMQIdx=$1
	WanName=$2
	ip link set imq$IMQIdx up
	tc qdisc del root dev imq$IMQIdx
	iptables -t mangle -D POSTROUTING -o $WanName -j IMQ --todev $IMQIdx
}

ppa_unload()
{
	# Function to unload PPA modules
	rmmod ppa_api_proc ppa_api ppa_hal_$plat ppa_datapath_$plat
}

ppa_load()
{
	# Function to load the PPA modules
	NumQs=$1
	insmod /lib/modules/`uname -r`/ppa_datapath_$plat.ko  wanqos_en=$NumQs
	insmod /lib/modules/`uname -r`/ppa_hal_$plat.ko
	insmod /lib/modules/`uname -r`/ppa_api.ko
	insmod /lib/modules/`uname -r`/ppa_api_proc.ko	
}

ppasfq_create()
{
   	# PPA WFQ, Rate-limit Eg: ppasfq_create <TotalRate> <Calss1Rate> <class2Rate> <WanName>	
	TotalRate=$1
	Class1Rate=$2
	Class2Rate=$3
	WanName=$4
    ppacmd setctrlrate -i $WanName -c enable
	ppacmd setrate -i $WanName -r $TotalRate
	ppacmd setctrlwfq -i $WanName -c enable
	ppacmd setwfq -i $WanName -q 1 -w $Class1Rate
	ppacmd setwfq -i $WanName -q 0 -w $Class2Rate
	echo "$WanName prio0 queue 1 prio 1 queue 1 prio 2 queue 1 prio 3 queue 1 prio 4 queue 1 prio 5 queue 1 prio 6 queue 1 prio 7 queue 1" > /proc/eth/prio
if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
	switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=10 pattern.bEnable=1 pattern.bProtocolEnable=1 pattern.nProtocol=0x6 pattern.nProtocolMask=0 pattern.bPktLngEnable=1 pattern.nPktLng=52 pattern.nPktLngRange=0x0 action.eTrafficClassAction=2 action.nTrafficClassAlternate=7
	echo "CLASS2QID 0x00000008 0x00000000" > /proc/eth/class
fi
}

ppasfq_delete()
{
    # restore to default settings
	WanName=$1
if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
	switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=10 pattern.bEnable=0	
fi
	ppacmd setctrlrate -i $WanName -c disable
	ppacmd setctrlwfq -i $WanName -c disable
}

EthWan_GetLinkStatus()
{
if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
	return `switch_cli IFX_ETHSW_PORT_LINK_CFG_GET nPortId=$PortId | grep eLink | awk '{ print $2 }'`
fi
}

EthWan_GetPPAStatus()
{
	return `ppacmd status | grep "WAN Acceleration: enabled" | wc -l`
}



EthWan_RateUpdate()
{
	#Check if Link is Up
	EthWan_GetLinkStatus
	LinkStatus=$?
	#if link is up get the link speed
	if [ $LinkStatus -eq 0 ]
	then
		oldStatus=$LinkStatus
if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
		LinkRate=`switch_cli IFX_ETHSW_PORT_LINK_CFG_GET nPortId=$PortId | grep eSpeed | awk '{ print $2 }'`
fi
		EthWan_GetPPAStatus
		PPAStatus=$?
		if [ $oldRate -eq $LinkRate -a $oldPPAStat -eq $PPAStatus ]
		then
		  return
		else
		  oldRate=$LinkRate
		  oldPPAStat=$PPAStatus
		fi		
		if [ $PPAStatus -eq 1 ]
		then
			#for PPA enabled scenarios
		   TotalRate=`expr $LinkRate \* 1000`
           AckRate=`expr $LinkRate \* 10 \* $AckWt`
           DataRate=`expr $LinkRate \* 10 \* $DataWt`
           ppasfq_create $TotalRate $AckRate $DataRate $WANInt
		else
			#else if CPU path use the IMQ infrastructure
			htbimq_delete $IMQDev $default_wan_conn_iface
		    AckRate=`expr $LinkRate \* 10 \* $AckWt`
		    DataRate=`expr $LinkRate \* 10 \* $DataWt`
			htbimq_create $IMQDev $LinkRate $AckRate $DataRate $default_wan_conn_iface
		fi
	else
		#Link is down so disable earlier configs
		EthWan_GetPPAStatus
		PPAStatus=$?
		if [ $oldStatus -eq $LinkStatus ]
		then
		  echo "Ignoring as state did not change"
		  return
		else
		  oldStatus=$LinkStatus
		  oldRate=0
		fi
		if [ $PPAStatus -eq 1 ]
		then
			#for PPA enabled scenarios
			ppasfq_delete $WANInt
		else
			#else if CPU path use the IMQ infrastructure
			htbimq_delete $IMQDev $default_wan_conn_iface
		fi
	fi
}

start()
{
  #Fill the variables based on platform
  
  #create IMQ
  ip link set imq$IMQDev up
  # Run a loop every 10 seconds
  while true 
  do
    EthWan_RateUpdate
    sleep 10
  done
}


# call arguments verbatim:
$@
