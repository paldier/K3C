#!/bin/sh

interface_name=$1
own_mac=$2
own_mac_no_colon=${own_mac//:/}
sta_addr=$3
neighbor_ap_addr=$4
neighbor_ap_addr_no_colon=${neighbor_ap_addr//:/}
neighbor_ssid=$5

echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $neighbor_ap_addr ssid=\"$neighbor_ssid\" nr=${neighbor_ap_addr_no_colon}00000000732809030110"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $neighbor_ap_addr ssid=\"$neighbor_ssid\" nr=${neighbor_ap_addr_no_colon}00000000732809030110
echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$neighbor_ssid\" nr=${own_mac_no_colon}00000000732409030120"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$neighbor_ssid\" nr=${own_mac_no_colon}00000000732409030120

echo ""
echo ""
echo ""
echo "Press Enter to execute step 6"
read -p "Execute step 6: " exec_step6

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${own_mac},10,115,36,9,10 neighbor=${neighbor_ap_addr},10,115,40,9,10 mbo=4:0:-1 disassoc_imminent=0"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${own_mac},10,115,36,9,10 neighbor=${neighbor_ap_addr},10,115,40,9,10 mbo=4:0:-1 disassoc_imminent=0

echo ""
echo ""
echo ""
echo "Press Enter to execute step 8 only after the STA disables the reject"
read -p "Execute step 8: " exec_step8

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${own_mac},10,115,36,9,10 neighbor=${neighbor_ap_addr},10,115,40,9,10 mbo=4:100:-1 disassoc_imminent=1 disassoc_timer=20"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${own_mac},10,115,36,9,10 neighbor=${neighbor_ap_addr},10,115,40,9,10 mbo=4:100:-1 disassoc_imminent=1 disassoc_timer=20
