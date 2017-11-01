#!/bin/sh
# Author: Shivaji Roy
# This script configures the switch in such a way that most of the modes will work. Special scenarios may require further customization
# Revision History:
#  Date 22-Nov-2012:Added options from 0-8. Add new option after 8 and vefore "Help"
#                   Mutlicast snooping configuration for switch TBD
#  Date 

. /etc/rc.d/ltq_switch_functions.sh 2>/dev/null

show_menu() {
	#clear
	echo "============================="
	echo "Please select below options :"
	echo "============================="
	echo "0  - Wan Mode"
	echo "1  - Init"
	echo "2  - Enable Port separation"
	echo "3  - Disable Port separation"
	echo "4  - Enable multiwan without vlan"
	echo "5  - Disable multiwan without vlan"
        echo "6  - Enable switch trace mode"
        echo "7  - Disable switch trace mode"
        echo "8  - Multicast settings"
        echo "9  - Enable Port Isolation"
        echo "10 - Disable Port Isolation"
        echo "11 - Add WAN VLAN interface"
        echo "12 - Delete WAN VLAN interface"
        echo "13 - Enable Bridge acceleration"
        echo "14 - Disale Bridge acceleration"
	echo "15 - Add ATM routing acceleration"
	echo "16 - Remove ATM routing acceleration"
	echo "17 - Help"
	echo "18 - Debug dump"
	echo "19 - Exit"
	echo "============================="
}

usage() {
	echo "Choose  "
}

help_info() {

	local choice
	echo "=============="
        echo " Wan Mode                   :Ethernet or DSL or LTE(uses directpath switch port 7)." 
        echo " Init                       :Initialze switch to default settings." 
        echo " Enable Port separation     :Enables Switch LAN port separation based on VLAN."
        echo " Enable Port separation     :Disables Switch LAN port separation based on VLAN."
	echo " Enable multiwan without vlan: This option is required when multiple logical wan is created on top of same physical wan and they are not vlan separated for e.g. using macvlan. Especially for ethernet WAN mode when one of the wan is bridged and the other routed"
	echo " Disable multiwan without vlan: To disable the above configuration"
        echo " Enable switch trace mode   :Enables priniting of switch_utility or switch_cli commands, params on console. 
Requires writable Filesystem"
        echo " Disable switch trace mode:Disables priniting of switch_utility or switch_cli commands, params on console. 
Requires writable Filesystem"
        echo " Multicast settings         :Set multicast configurations "
        echo " Enable Port Isolation      :Isolate a port from other switch ports."
        echo " Disable Port Isolation     :Connect an isolated port with other switch ports."
	echo " Add WAN VLAN interface     :Add new WAN VLAN interface configuration to the switch."
	echo " Delete WAN VLAN interface  :Deletes WAN VLAN configuration fron switch."
	echo " Enable Bridge acceleration :Enables bridging acceleration between specified LAN & WAN interfaces."
	echo " Disale Bridge acceleration :Disables bridging acceleration between specified LAN & WAN interfaces."
	echo " Add ATM routing acceleration : Enables acceleration on a routed nas interface"
	echo " Remove ATM routing acceleration : Disables acceleration on a routed nas interface" 
        echo " Debug dump               :Dump for expert analysis. Disable "Switch trace mode" before selecting this "
        echo " Help                     :Print this help menu"
	echo "=============="
	read -p "Press 9 to select a help topic:" choice
        if [ $choice = "9" ]; then
           echo "===================== VLAN Topic============================="
           echo " In switch a VLAN group creates a broadcast domain. Packet will be transmitted between 
the VLAN group only. For e.g. Ports with same PVID can transmit packet among each other 
and hence creates a vlan group. Hence commands VLAN_PortCfgGet and VLAN_PortMemberAdd 
creates a VLAN group where ports can transmit packets with each other."
           echo "______________________________________________"
           echo "           egress /\  | ingress            "
           echo "                  |   \/                   "
           echo "                     --                    " 
           echo " -------------------|6 |-------------------"
           echo "|                    --                   |" 
           echo "|                                         |"
           echo "|    --      --       --                  |" 
           echo " ---|0 |----|1 |-----|2 |------------------"
           echo "     --      --       --                   " 
           echo "       egress|        /\ingress            "
           echo "             \/       |                    "
           echo "______________________________________________"

           echo " The command VLAN_PortCfgSet is evaluated at the ingress of any port 
whereas the command VLAN_PortMemberAdd command is evaluated at the egress of 
any port. The command VLAN_PortCfgSet creates Port based VLAN grouping. 
Ingress port corresponding to the same PVID belong to same VLAN group. Other 
important parameters of the commands are mentioned below."
           echo " TVM (transparent VLAN mode): The port is transparent (don't care) of 
the VLAN in ingress direction"
           echo " VLAN Violation Member: ingress/egress/both: means if the ingress/egress/both
(ingress and egress)  port is not a member of the VLAN group then don't forward."
           echo " The command VLAN_PortMember allows to create an tag or untag member 
of a VLAN group. For e.g. if the PVID of Port 2 is 500 and Port 6 is Tag member of 
500 (VLAN_PortMemberAdd 500 6 1) then egress packet from Port 6 will have VLAN tag of 
500 on the other had if it is an untag member of 500 (VLAN_PortMemberAdd 500 6 0) 
then egress packet from Port 6 will have no VLAN tag when ingress was from Port 2." 
           echo "                                                              "   
           echo "========================  Directpath Topic===================="
           echo "========================== Other Topics ======================"
        fi
}

###############################################################################################################
# This function is used to show menu option for different WAN modes
###############################################################################################################
select_wan() {

   config_wan="2"
   local choice
   read -p "Enter: [1-xDSL 2-Ethernet-MII0 3-Ethernet-MII1 4-LTE] :" choice
   case $choice in
       1) config_wan="0" ;;
       2) config_wan="1" ;;
       3) config_wan="2" ;;
       4) config_wan="6"
   esac

   select_wan_mode $config_wan  
}

add_wan_vif() {
	local vif
	read -p "Enter the WAN vlan interface:" vif

	add_wanvlan_if $vif
}

del_wan_vif() {
	local vif
	read -p "Enter the WAN vlan interface:" vif

	del_wanvlan_if $vif
}

enable_br_acceleration() {
	local lanif
	local wanif
	local bridge
	read -p "Enter the LAN interfaces [comma separated without blank space] :" lanif
	read -p "Enter the WAN vlan interface:" wanif
	read -p "Enter the Bridge:" bridge

	enable_bridge_accel $wanif $lanif $bridge	 
}

disable_br_acceleration() {
	local lanif
	local wanif
	read -p "Enter the LAN interfaces [comma separated without blank space] :" lanif
        read -p "Enter the WAN vlan interface:" wanif
	disable_bridge_accel $wanif $lanif	
}

add_nasif_vconfig() {
	local wanif
	read -p "Enter the nas interface:" wanif
	add_atm_routing_acceleration  $wanif
}

del_nasif_vconfig() {
	local wanif
	read -p "Enter the nas interface:" wanif
	del_atm_routing_acceleration  $wanif
}

enable_isolation() {
	local lanif
	read -p "Enter the lan interface to be isolated:" lanif
	enable_port_isolation $lanif
}
disable_isolation() {
	local lanif
	read -p "Enter the lan interface:" lanif
	disable_port_isolation $lanif
}

read_options() {

	local choice
	read -p "Enter choice [0 to 19] :" choice
	case $choice in

		0) select_wan ;;
		1) switch_init ;;
		2) enable_separation ;;
		3) disable_separation ;;
		4) enable_multiwan_conf ;;
		5) disable_multiwan_conf ;;
                6) enable_trace ;;
                7) disable_trace ;;
                8) multicast_setting ;;
                9) enable_isolation ;;
                10) disable_isolation ;;
		11) add_wan_vif ;;
		12) del_wan_vif ;;
		13) enable_br_acceleration ;;
		14) disable_br_acceleration ;;
		15) add_nasif_vconfig ;;
		16) del_nasif_vconfig ;;
		17) help_info ;;
                18) debug_dump ;; 
	        19) exit 0
	esac
}

while true
do
	show_menu
        read_options
done



