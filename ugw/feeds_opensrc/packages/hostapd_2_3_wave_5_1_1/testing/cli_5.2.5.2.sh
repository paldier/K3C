#!/bin/sh

interface_name=$1
own_mac=$2
own_mac_no_colon=${own_mac//:/}
sta_addr=$3
neighbor_ap_addr=$4
neighbor_ap_addr_no_colon=${neighbor_ap_addr//:/}
neighbor_ssid=$5

echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $neighbor_ap_addr ssid=\"$neighbor_ssid\" nr=${neighbor_ap_addr_no_colon}00000000732409030110"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $neighbor_ap_addr ssid=\"$neighbor_ssid\" nr=${neighbor_ap_addr_no_colon}00000000732409030110
echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$neighbor_ssid\" nr=${own_mac_no_colon}00000000732409030120"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$neighbor_ssid\" nr=${own_mac_no_colon}00000000732409030120

echo ""
echo ""
echo ""
echo "Press Enter to execute step 7 for AP1"
read -p "Execute step 7 for AP1: " exec_step7

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${neighbor_ap_addr},10,81,36,9,10 mbo=4:50:-1 disassoc_imminent=1 disassoc_timer=100"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${neighbor_ap_addr},10,81,36,9,10 mbo=4:50:-1 disassoc_imminent=1 disassoc_timer=100
