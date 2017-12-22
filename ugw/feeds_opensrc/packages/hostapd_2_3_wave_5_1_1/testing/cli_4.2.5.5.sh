#!/bin/sh

interface_name=$1
own_mac=$2
own_mac_no_colon=${own_mac//:/}
sta_addr=$3
own_ssid=$7

echo ""
echo ""
echo ""
echo "Press Enter to execute test 4.2.5.5"
read -p "Execute 4.2.5.5: " exec

echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$own_ssid\" nr=${own_mac_no_colon}00000000732409030120"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$own_ssid\" nr=${own_mac_no_colon}00000000732409030120

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${own_mac},0,32,1,7,0 mbo=4:0:255"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${own_mac},0,32,1,7,0 mbo=4:0:255
