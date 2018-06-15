#!/bin/sh /etc/rc.common

START=18
USE_PROCD=1
QUIET=""

export LD_LIBRARY_PATH=/opt/lantiq/lib:/opt/lantiq/usr/lib:${LD_LIBRARY_PATH}
export PATH=/opt/lantiq/sbin:/opt/lantiq/usr/sbin:/opt/lantiq/bin:${PATH}

. /lib/lantiq.sh

board=$(lantiq_board_name)

##port numbers start from 0 according to UCI
##port numbers start from 1 as per XRX330/300 HW

wan_fid=1
lan_fid=0
cpu_fid=0
cpu_port_id=5	#CPU port ID is 6 for  GRX300/GRX330 platforms. CPU port number is 5 in UCI notation.

CONFIG_SWITCH_DEVICE_ID=0	#switch Dev ID=0 for all the platforms

## Function to create a VLAN id by calling IFX_ETHSW_VLAN_ID_CREATE 
# Arguements:
# $1 VLANid to be created <1-4095>
# $2 Fid to be sepcified. <1-256>
create_vlanid() {
	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_ID_CREATE nVId=$1 nFId=$2
}

## Function to delete a specified VLAN id by using IFX_ETHSW_VLAN_ID_DELETE
# Arguements:
# $1 vlan id to be deleted <1-4095>
delete_vlanid() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_ID_DELETE nVId=$1
}

## Function to set pvid of a specified port
# Arguements:
# $1 Portid <0-11>
# $2 VLANid <1-4095>
# $3 TVM (Transparent VLAN Mode) <0/1>
config_port_pvid() {
	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_SET nPortId=$1 nPortVId=$2 bVLAN_UnknownDrop=0 bVLAN_ReAssign=0 eVLAN_MemberViolation=$4 eAdmitMode=0 bTVM=$3
}

## Function to set add a port as member to a specified VLAN
# Arguements:
# $1 VLANid <1-4095>
# $2 Portid <0-11>
# $3 Egress tagging <0/1>
config_port_member() {
	local vid=$1
	local port=$2
	local egress_tag=$3
	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_ADD nVId=$vid nPortId=$port bVLAN_TagEgress=$egress_tag
}

## Function to set remove a port from a vlan membership
# Arguements:
# $1 VLANid <1-4096>
# $2 Portid <0-11>
reset_port_member() {

	local vid=$1
	local port=$2
	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE nVId=$vid nPortId=$port
}


## This function initializes switch buffers for GRX300 platform.
# Default configuration on this platform is different from other platforms where in,
# 512 switch buffers are available on GRX300 (GSWIP 2.2 and above) compared to 256 on the rest (GSWIP-2.1 and below)
# This configuration is required to address congestion problems due to bursty traffic.
# Buffers reserved for each queue can be tuned for different scenarios.
# Given configuration is based on test cases and scenarios executed by switch team.
init_sw_cfg_for_grx330_plat() {
	# Enable Flow Control eFlowCtrl=3
	i=0
	while [ $i -le 5 ]
	do
		switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=$i eEnable=1 eFlowCtrl=3
		switch_cli IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET nPortId=$i nFlowCtrl_Min=18 nFlowCtrl_Max=30
		i=$((i+1))
	done

	# Configure Buffer reservation of each queue to 24 for i 0 31
	i=0
	while [ $i -le 31 ]
	do
		switch_cli IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET nQueueId=$i nBufferReserved=24
		i=$((i+1))
	done
	# Configure Global buffer threshold
	switch_cli IFX_ETHSW_QOS_WRED_CFG_SET eProfile=0 nRed_Min=0x3ff nRed_Max=0x3ff nYellow_Min=0x3ff nYellow_Max=0x3ff nGreen_Min=0x180 nGreen_Max=0x180

	# Configure Global flowcontrol  threshold buffer
	switch_cli IFX_ETHSW_QOS_FLOWCTRL_CFG_SET nFlowCtrlNonConform_Min=0x3ff nFlowCtrlNonConform_Max=0x3ff nFlowCtrlConform_Min=0x3ff nFlowCtrlConform_Max=0x3ff
}

# Function to config GPHY LED
led_config_ethernet_330() {
	local i port

	for i in $lan_ports; do
		port=$((i+1))
		switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e2 nData=0x42
		switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e3 nData=0x10
		switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e4 nData=0x70
		switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e5 nData=0x03
	done

	switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e2 nData=0x70
	switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e3 nData=0x03
	switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e4 nData=0x42
	switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e5 nData=0x10
}


bringup_if() {
	local if_name="$1"
	local dev port pvid
	local switch_dev

	config_get dev "$if_name" device
	config_get pvid "$if_name" pvid
	config_get port "$if_name" port
	
	##for WAN port
	if [ $port -eq $WAN_port ]
	then
		ifconfig eth1 down 
		## Alias is used to find the base interface if the original interface is renamed.
		ip link set name $if_name eth1 alias eth1
		ifconfig $if_name up
		return
	fi

	## for LAN ports
	#TODO: match port with LAN ports..
	if [ $port -le 3 ]
	then
		if [ $dev == "switch0" ]
		then
			switch_dev="eth0"
		fi
		## port=port+1.. Since UCI ports strats from '0' where as GRX330 ports starts from '1'
		port=$((port+1))
		ip link add dev $if_name link $switch_dev type ethsw ports $port
		ifconfig $switch_dev up
		ifconfig $if_name up
	fi

}

bring_down_if(){
	local if_name="$1"
	local dev vlan ports

	config_get dev "$if_name" device
	config_get pvid "$if_name" pvid
	config_get port "$if_name" port
	##bring down the LAN interfaces only.
	if [ $port -le 3 ]
	then
		local sys_entry="/sys/class/net/$if_name"
		[ -n dev -a -d $sys_entry ] && {
			ifconfig $if_name down
			ip link delete $if_name
		}
	fi
}

bring_down_ifs_renamed(){

	## This will give LAN interfaces that are renamed
	local ifs_renamed=$(ip link show type ethsw|sed -e 's/@.*//' -e '/.*link.*/d' -e 's/^.*: //')
	
	for i in $ifs_renamed
	do
		ifconfig $i down
		ip link delete $i
	done
}

find_WAN_if() {
	local if_name="$1"
	local dev vlan ports

	config_get port "$if_name" port

	if [ $port -eq $WAN_port ]
	then 
		new_WAN_name=$1 #new_WAN_name=Used when WAN interface name is changed.
	fi
}


rename_wan_interface() {

	local WAN_if # obtain from sysfs
	local entry sys_entry
	local found=0
	local new_name # obtain from UCI file

	for entry in `ls /sys/class/net/`
	do 
		sys_entry=`cat /sys/class/net/$entry/ifalias`
		if [ "$sys_entry" == "eth1" ]
		then
			WAN_if=$entry
			found=1
			break
		fi	
	done
	config_foreach find_WAN_if switch_port $new_name

	if [ $found -eq 1 ] 
	then
		ifconfig $WAN_if down
		ip link set name $new_WAN_name $WAN_if alias eth1
	else
		ip link set name $new_WAN_name eth1 alias eth1
	fi
}

find_all_ports() {
	local vlan_section=$1

	switch_vlan_section_found=1
	config_get ports "$vlan_section" ports
	all_ports="$all_ports $ports" ##all_ports=lan+wan+cpu ports including tagging info
}

find_all_vlans() {
	local port 
	local pvid

	config_get port "$1" port
	config_get pvid "$1" pvid
	all_pvids="$all_pvids $pvid"
}

seperate_lan_wan_ports() {
	local switch_vlan=$1
	local vlan
	local ports 
	
	config_get vlan "$1" vlan 
	config_get ports "$1" ports

	if [ $vlan -eq 0 ]; then 
		#cpu_lan_ports=CPU+LAN ports: same as mentioned in uci section. It also contains Tagging info like '5t'
		cpu_lan_ports=$ports
	fi

	if [ $vlan -eq 1 ]; then 
		#CPU + WAN ports: same as mentioned in uci section. It also contains Tagging infi like '5t'
		cpu_wan_ports=$ports
	fi
}
find_dsl_ports() {
	local section=$1
	local port pvid
	
	config_get port $1 port
	config_get pvid $1 pvid 	
	#DSL names are hardcoded..
	if [ $section == "dsl1" -o $section == "dsl2" ]
	then
		DSL_ports="$DSL_ports $port"	
	fi
}

find_lan_VLAN() {
	local if_name="$1"
	local port pvid 

	config_get port "$if_name" port
	config_get pvid "$if_name" pvid
	if [ $port -eq $2 ]; then 
		lan_VLAN=$pvid
	fi
}

find_wan_VLAN() {
	local if_name="$1"
	local port pvid

	config_get port "$if_name" port
	config_get pvid "$if_name" pvid
	if [ $port -eq $2 ]; then
		wan_VLAN=$pvid
		WAN_if_name=$if_name ##WAN_if_name=(for eg. eth1)used to configure WAN VLAN
		WAN_port=$port ##(for eg. 5 for 350) used to configure WAN VLAN
	fi
}

find_cpu_VLAN() {
	local if_name="$1"
	local port pvid

	config_get port "$if_name" port
	config_get pvid "$if_name" pvid
	if [ $port -eq $2 ]; then 
		cpu_VLAN=$pvid
	fi
}

find_all_WAN_VLANs() {
	local section proto ifname base_if vlan_tag type
	section=$1
	
	config_get proto "$section" proto
	config_get ifname "$section" ifname
	config_get type "$section" type
	ifname=`uci get network.$section.ifname`
	
	#1. get all the DHCP, PPPoE and static VLAN tags
	[ -z $proto ] && [ -z $type ] && return
	if [ $proto == "dhcp" -o $proto == "pppoe" -o $proto == "static"  -o $proto == "dhcpv6" ]
	then
		base_if=${ifname/.*/}
		if [ "$base_if" == $WAN_if_name ]
		then
			vlan_tag=`echo $ifname  | cut -d . -f 2`
			if [ "X$vlan_tag" != "X"  -a "$base_if" != "$vlan_tag" ]
			then
				all_WAN_VLANs="$all_WAN_VLANs $vlan_tag"
			fi
		fi
	fi
	
	##TODO: Enable the below code when swconfig.sh is adapted for bridge acceleration.
	#2. get all the bridge WAN VLAN tags
	#if [ "$type" ==  "bridge" ]
	#then
	#	# in bridge WAN case ifname is not a single interface, it is a list of interfaces.
	#	for i in $ifname
	#	do
	#	base_if=${i/.*/}
	#		if [ $base_if == $WAN_if_name ]
	#		then
	#			#Enable the bridge acceleration if bridged WAN is created.
	#			vlan_tag=`echo $i  | cut -d . -f 2`
	#			if [ "X$vlan_tag" != "X"  -a "$base_if" != "$vlan_tag" ]
	#			then
	#				all_WAN_VLANs="$all_WAN_VLANs $vlan_tag"
	#			fi
	#		fi
	#	done
	#fi
}

config_switch_WAN_VLANs () {
	local section vlan ports
	local i cpu_port wan_port
	local port
		
	section=$1
	config_get vlan $section vlan
	config_get ports $section ports
	
	cpu_port=$(($cpu_port_id+1))
	wan_port=$(($WAN_port+1))
	
	if [ $vlan -eq 1 ] # WAN section
	then
		for port in $ports
		do
			case $port in
			$WAN_port)
				;;
			$WAN_port\t)
				# Create all the WAN VLANs in switch
				for i in $all_WAN_VLANs
				do
					create_vlanid $i $wan_fid
					# Add CPU and ETH as members of the WAN VLAN
					config_port_member $i $cpu_port 1
					config_port_member $i $wan_port 1
				done
				;;
			5)
				;;
			5t)
				#Add CPU port as a member of 502 VLAN
				config_port_member 502 $cpu_port 1
				;;
			*)
				echo " Unknown port: $port"
				;;
			esac
		done
	fi
}

find_bridge_WANs () {
	#1. loop through all the bridge interfaces which has WAN as a member
	#2. if found call enable_bridge_accel with args(wanintf lan brname)
	local section=$1
	local proto if_list if_type iface vlan_tag
	local lan_ifs wan_if br_name
	local found_bridge_wan=0

	config_get proto "$section" proto
	config_get if_type "$section" type
	if_list=`uci get network.$section.ifname`

	if [ "$if_type" ==  "bridge" ]
	then
		# in bridge WAN case ifname is not a single interface, it is a list of interfaces.
		for iface in $if_list
		do
			base_if=${iface/.*/}
			if [ $base_if == $WAN_if_name ]
			then
				#Enable the bridge acceleration if bridged WAN is created.
				found_bridge_wan=1
				br_name="$section"
				break
			fi
		done
		if [ $found_bridge_wan -eq 1 ]
		then
			## create a list of LAN ifs, WAN if, bridge name.
			for iface in $if_list
			do
				base_if=${iface/.*/}
				if [ $base_if == $WAN_if_name ]
				then
					vlan_tag=`echo $iface  | cut -d . -f 2`
					##enable_bridge_accel expects the wan name like eth1_109
					wan_if="$base_if"_"$vlan_tag"
				else
					lan_ifs="$lan_ifs, $iface"
				fi
			done
			iface=$(echo $wan_if | cut -d "_" -f1)
			if [ "${iface::3}" = "nas" ]; then
				. /opt/lantiq/etc/switch_functions "dsl" 2> /dev/null
			elif [ "$iface" = "ptm0" ]; then
				. /opt/lantiq/etc/switch_functions "dsl" 2> /dev/null
			elif [ "$iface" = "eth1" ]; then
				. /opt/lantiq/etc/switch_functions "eth" 2> /dev/null
			fi
			echo disable > /proc/ppa/api/bridged_flow_learning
			enable_bridge_accel "$wan_if" "$lan_ifs" "$br_name"
		fi
	fi
}

get_lan_wan_ifs() {
	local section=$1
	lan_wan_interfaces="$lan_wan_interfaces $section"
}

##This is function is for 350 platform...Here only interface renaming is needed.
config_switch_no_vlan() {

	local eth_ifs_on_boot="eth0_1 eth0_2 eth0_3 eth0_4 eth1"
	local i n
	local boot_if uci_if boot_ifs uci_ifs

	config_foreach get_lan_wan_ifs switch_port

	n=0
	for i in $eth_ifs_on_boot
	do
		eval boot_ifs$n=$i
		n=$((n+1))
	done
	n=0
	for i in $lan_wan_interfaces
	do
		eval uci_ifs$n=$i
		n=$((n+1))
	done

	for i in 0 1 2 3 4
	do
		eval uci_if='$'uci_ifs$i
		eval boot_if='$'boot_ifs$i
		##Rename the eth interface when UCI name is changed
		if [ ${uci_if} != ${boot_if} ]
		then
			ifconfig $boot_if down
			ip link set name $uci_if $boot_if
			ifconfig $uci_if up
		fi
	done
}

##This function will parse UCI switch section and update the global variables.
parse_uci_file() {

	#TODO: if switch_vlan is not found, treat it as 350/550.
	switch_vlan_section_found=0
	config_foreach find_all_ports switch_vlan

	if [ $switch_vlan_section_found -eq 1 ]
	then

	config_foreach find_all_vlans switch_port

	config_foreach seperate_lan_wan_ports switch_vlan

	config_foreach find_dsl_ports switch_port

	#extract only LAN ports without tagging info
	for port in $cpu_lan_ports 
	do 
		case $port in
		5)
			#ignore the CPU port
			;;
		5\t)
			#ignore the CPU TGGED port
			;;
		[0-9]|1[0-1]) #for range of ports 0,1,2... 11. 
			  #This range is usefull when swconfig.sh is adapted to Luci switch config for more tha 5 ports
			lan_ports="$lan_ports $port" ##lan_ports=LAN ports only, no cpu port and no tagging info.
			;;
		[0-9]\t|1[0-1]\t)
			p=${port::1}
			lan_ports="$lan_ports $p"
			;;
		*)
			echo "Unknown Port:$port"
			;;
		esac
	done

	#extract only WAN ports without tagging info
	for port in $cpu_wan_ports 
	do 
		case $port in 
		5)
			#ignore the CPU port
			;;
		5\t)
			#ignore the CPU TAGGED port
			;;
		[0-9]|1[0-1])
			wan_ports="$wan_ports $port" #wan_ports=#WAN ports only, no cpu port and no tagging info.
			;;
		[0-9]\t|1[0-1]\t)
			p=${port::1}
			wan_ports="$wan_ports $p"
			;;
		*)
			echo "Unknown Port:$port"
			;;
		esac
	done

	lan_port_1=${cpu_lan_ports::1}
	wan_port_1=${cpu_wan_ports::1}


	config_foreach find_lan_VLAN switch_port $lan_port_1
	config_foreach find_wan_VLAN switch_port $wan_port_1
	config_foreach find_cpu_VLAN switch_port $cpu_port_id
	else
		return
	fi
}
led_config_ethernet_350() {

		#LAN Switch Port 2 GPHY ( eth0_1 )
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=2 nAddressReg=0x1f01e2 nData=0x70
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=2 nAddressReg=0x1f01e3 nData=0x0B
		#LAN Switch Port 3 GPHY ( eth0_2 )
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=3 nAddressReg=0x1f01e2 nData=0x70
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=3 nAddressReg=0x1f01e3 nData=0x0B
		#LAN Switch Port 4 GPHY ( eth0_3 )
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=4 nAddressReg=0x1f01e2 nData=0x70
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=4 nAddressReg=0x1f01e3 nData=0x0B
		#LAN Switch Port 5 GPHY ( eeth0_4 )
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=5 nAddressReg=0x1f01e2 nData=0x70
		switch_cli dev=0 GSW_MMD_DATA_WRITE nAddressDev=5 nAddressReg=0x1f01e3 nData=0x0B
		#WAN PAE  Port 15 ( eth1 )
		switch_cli dev=1 GSW_MMD_DATA_WRITE nAddressDev=1 nAddressReg=0x1f01e2 nData=0x70
		switch_cli dev=1 GSW_MMD_DATA_WRITE nAddressDev=1 nAddressReg=0x1f01e3 nData=0x0B


	echo GPT > /sys/devices/16000000.ssx4/16d00000.sso/update_clock_source
	echo 255 > /sys/class/leds/sys/brightness

}

ethernet_port_config() {
	local section=$1
	local enable
	local speed softswitch
	local duplex port td
	local port_dev=0

	config_get enable $section enable
	config_get speed $section speed
	config_get td $section td
	config_get duplex $section duplex
	config_get softswitch $section softswitch

	port_id=`uci get network.$1.port`
	port_id=$((port_id+1))
	[ -z $enabled ] && enabled=1
	[ -z $speed ] && speed="auto"
	[ -z $duplex ] && duplex="full"
	[ -z $wan ] && wan=0
	[ -z $softswitch ] && softswitch=0

	if [ "$speed" != "auto" ]
	then
		if [ "$duplex" = "half" ]
		then
			duplex=1
		else
			duplex=0
		fi
		##TODO: For 300/330 port_dev=0 for LAN port_dev=0 for WAN
		##	For 350 port_dev=0 for LAN port_dev=1 for WAN
		case $board in
			EASY330\ VDSL\ BOND | EASY300\ AC1200 )
				switch_cli dev=0 IFX_ETHSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=1 \
							eDuplex=$duplex bSpeedForce=1 eSpeed=$speed
				;;
			EASY350\ ANYWAN\ \(GRX350\)\ *)
			if [ $port_id -eq 15 ]
			then
				##For WAN port switch dev=1
				switch_cli dev=1 GSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=1 \
							eDuplex=$duplex bSpeedForce=1 eSpeed=$speed
			else
				##For LAN ports switch dev=0
				switch_cli dev=0 GSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=1 \
							eDuplex=$duplex bSpeedForce=1 eSpeed=$speed
			fi
				;;
		esac
	else
		## Set port to auto mode
		case $board in
			EASY330\ VDSL\ BOND | EASY300\ AC1200 )
				switch_cli dev=0 IFX_ETHSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=0 \
												bSpeedForce=0
				;;
			EASY350\ ANYWAN\ \(GRX350\)\ *)
			if [ $port_id -eq 15 ]
			then
				switch_cli dev=1 GSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=0 \
												bSpeedForce= 0
			else
				switch_cli dev=0 GSW_PORT_LINK_CFG_SET nPortId=$port_id bDuplexForce=0 \
												bSpeedForce= 0
			fi
				;;
		esac
	fi
	##TODO: For 350 only...
	#This is giving error... Check
	#[ $wan == 0 ] && { switch_cli dev=0 GSW_PORT_CFG_SET nPortId=$port_id bLearning=$softswitch; }

	if [ "$td" != "" ]; then
		local shaper_id=$port_id; local mbr; local mbs; local port_queue

		mbr=`uci get ethernet.${td}.mbr`
		mbs=`uci get ethernet.${td}.mbs`

		case $board in
			EASY330\ VDSL\ BOND | EASY300\ AC1200 )
				port_queue=`switch_cli IFX_ETHSW_QOS_QUEUE_PORT_GET nPortId=$port_id | awk -F'|' 'BEGIN { found=0; } { if (NF == 3) { if($3 == " Egress Queue") { found=1; } else if(found == 1) {printf("%d\n", $3); found=0;}}}'`
				[ -z $port_queue ] && { echo "ERROR: Can't find port_queue for $port_id"; }

				## Create SHAPER
				switch_cli IFX_ETHSW_QOS_SHAPER_CFG_SET nRateShaperId=$shaper_id bEnable=1 \
											nCbs=$mbs nRate=$mbr
				## Assign SHAPER to QUEUE
				switch_cli IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN nRateShaperId=$shaper_id \
											nQueueId=$port_queue
				;;
			EASY350\ ANYWAN\ \(GRX350\)\ *)
				port_queue=`switch_cli GSW_QOS_QUEUE_PORT_GET nPortId=$port_id | awk -F'|' 'BEGIN { found=0; } { if (NF == 5) { if($3 == " Egress Queue ") { found=1; } else if (found == 1) {printf("%d\n", $3); found=0;}}}' | head -n 1`
				[ -z $port_queue ] && { echo "ERROR: Can't find port_queue for $port_id"; }
				## Create SHAPER
				switch_cli GSW_QOS_SHAPER_CFG_SET nRateShaperId=$shaper_id bEnable=1 \
											nCbs=$mbs nRate=$mbr
				## Assign SHAPER to QUEUE
				switch_cli GSW_QOS_SHAPER_QUEUE_ASSIGN nRateShaperId=$shaper_id \
											nQueueId=$port_queue
				;;
		esac
	fi
}

setup_switch() {
	local maxPacketLen=1536
	local platform=`echo $(grep "system" /proc/cpuinfo|cut -d: -f2)`
	local port
	local mii1_reg mii1
	config_load network

	parse_uci_file
	if [ $switch_vlan_section_found -eq 0 ]
	then
		config_switch_no_vlan
		config_foreach led_config_ethernet_350 switch_port
		##Ethernet FAPI functionality...
		config_load ethernet
		config_foreach ethernet_port_config port
		return
	fi

	##Bringing the interface UP
	config_foreach bringup_if switch_port

	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_DISABLE

	###TODO: If MINI JUMBO FRAMES needs to be supported ..... ENable the below flag
	#if [ "$CONFIG_FEATURE_MINI_JUMBO_FRAMES" == "1" ] ; then
	#   
	# When mini jumbo frames feature is enabled the larger size packets should
	# pass through switch. Default packet len configured in switch is 1536. In
	# case of mini jumbo frames switch shall support packets upto 1600 bytes.
	#   
	#	maxPacketLen=1600
	#fi

	## Making the switch VLAN aware is required for all the platforms.
	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_CFG_SET eMAC_TableAgeTimer=3 bVLAN_Aware=1 \
		nMaxPacketLen=$maxPacketLen bPauseMAC_ModeSrc=0 nPauseMAC_Src=00:00:00:00:00:00


	##create VLANS in switch
	create_vlanid $lan_VLAN $lan_fid
	create_vlanid $wan_VLAN $wan_fid
	create_vlanid $cpu_VLAN $cpu_fid

	##Addsetup  PVIDS
	# lan_ports contains only LAN ports, does not include CPU
	for port in $lan_ports
	do
		port=$((port+1))
		config_port_pvid $port $lan_VLAN 1 3
	done

	# wan_ports contains only WAN ports, does not include CPU
	
	for port in $wan_ports
	do
		port=$((port+1))
		config_port_pvid $port $wan_VLAN 0 3
	done
	
	#DSL port configuration	
	for port in $DSL_ports
	do 
		port=$((port+1))
		config_port_pvid $port $wan_VLAN 1 3
	done

	#cpu PVID config
	cpu_port=$((cpu_port_id+1))
	config_port_pvid $cpu_port $cpu_VLAN 0 2
	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_ENABLE

	## VLAN port membership
	for port in $lan_ports $wan_ports $cpu_port_id
	do 
		port=$((port+1))
		config_port_member $cpu_VLAN $port 0
	done

	for port in $cpu_lan_ports
	do
		#To find tag... and pass it as egress tagging
		case $port in 
			[0-9])
				port=$((port+1))
				config_port_member $lan_VLAN $port 0
				;;
			[0-9]\t)
				p=${port::1}
				p=$((p+1))
				## LAN port does not need a egress tagging.
				config_port_member $lan_VLAN $p 0
				;;
		esac
	done 

	for port in $cpu_wan_ports
	do 
		#To find tag... and pass it as egress tagging
		case $port in 
			[0-9]|1[0-1])
				port=$((port+1))
				config_port_member $wan_VLAN $port 0
				;;
			[0-9]\t|1[0-1]\t)
				p=${port::1}
				p=$((p+1))
				## Egress tagging done in config_switch_WAN_VLANs...
				config_port_member $wan_VLAN $p 0
				;;
		esac
	done

	#DSL configuration
	for port in $DSL_ports 
	do 
		port=$((port+1))
		config_port_member $wan_VLAN $port 0
		config_port_member $cpu_VLAN $port 0
	done

	switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_ENABLE

	#if  [ "$CONFIG_FEATURE_EEE" = "1" ]; then
	#for i in $lan_port_all; do
	#      switch_cli IFX_ETHSW_PORT_LINK_CFG_SET nPortId=$i  bLPI=1
	#done
	#fi
	# spilt the lan and wan interrupts (DMA channels)
	#  split_lan_wan_dma_channel 1
	
	mii1=$wan_port_1
	mii1=$((mii1+1))
	mii1_reg="$((1 << mii1))"
	# Disable pause frame for cpu port
	switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x94b nData=0x1c0
	switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0xccd nData=$mii1_reg
	echo 0 > /proc/sys/net/bridge/bridge-nf-call-iptables
	echo 0 > /proc/sys/net/bridge/bridge-nf-call-ip6tables

	# Configure reserved buffers and global WRED configuration for GRX330 platform
	case "$platform" in
               "xRX330 rev 1.1")
		##The platform is same for 300 & 330 platforms
		init_sw_cfg_for_grx330_plat
		led_config_ethernet_330
			;;
		##TODO: Add for 220 Platform
		*)
			;;
	esac

	## Read the switch_vlan "vlan '1'"
	# 1. <WAN_port>t is there....(WAN tagged), read all the WAN VLANs
	#    	Create all the WAN VLANs in switch
	#	Add CPU, ETH WAN ports to  WAN VLANs with egress tag enabled 
	# 2. <CPU_port>t is there.. (CPU tagged), Add CPU to 502 VLAN with egress tag enabled
	config_foreach find_all_WAN_VLANs interface
	
	#configure the switch for WAN VLANs.
	config_foreach config_switch_WAN_VLANs switch_vlan 

	#switch config for bridge mode, enable the bridge accel.
	config_foreach find_bridge_WANs interface

	##Ethernet FAPI functionality...
	config_load ethernet
	config_foreach ethernet_port_config port
}

service_triggers() {
	## This is to configure the switch Egress tagging when a Routed WAN or Bridged WAN are created.
	procd_add_reload_trigger network
}
start_service() {
	setup_switch
}

stop_service() {
	local port
	local vlan
	config_load network

	parse_uci_file

	if [ $switch_vlan_section_found -eq 0 ]
	then
		return
	fi

	#clean up VLAN related configs
	for port in $lan_ports $wan_ports $cpu_port_id
	do 
		port=$((port+1))
		reset_port_member $cpu_VLAN $port
	done

	for port in $cpu_lan_ports
	do
		#To find tag... and pass it as egress tagging
		case $port in 
			[0-9])
				port=$((port+1))
				reset_port_member $lan_VLAN $port
				;;
			[0-9]\t)
				p=${port::1}
				p=$((p+1))
				reset_port_member $lan_VLAN $p
				;;
		esac
	done 

	for port in $cpu_wan_ports
	do 
		#To find tag... and pass it as egress tagging
		case $port in 
			[0-9])
				port=$((port+1))
				reset_port_member $wan_VLAN $port
				;;
			[0-9]\t)
				p=${port::1}
				p=$((p+1))
				reset_port_member $wan_VLAN $p
				;;
		esac
	done

	#Bring down the interfaces
	config_foreach bring_down_if switch_port

	bring_down_ifs_renamed

	rename_wan_interface

	# delete all the routed/bridged WAN VLANs
	config_foreach find_all_WAN_VLANs interface
	for vlan in $all_WAN_VLANs 
	do 
		delete_vlanid $vlan
	done

	#delete VLAN config in switch
	delete_vlanid $lan_VLAN
	delete_vlanid $wan_VLAN
	delete_vlanid $cpu_VLAN
}

reload_service() {
	# 1. Find out newly added WANs with VLAN
	# 2. configure the switch with newly added VLANs
	config_load network
	parse_uci_file

	if [ $switch_vlan_section_found -eq 1 ]
	then
		config_foreach find_all_WAN_VLANs interface
		config_foreach config_switch_WAN_VLANs switch_vlan
	fi

	#switch config for bridge mode, enable the bridge accel.
	config_foreach find_bridge_WANs interface
}
