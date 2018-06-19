#!/bin/sh

interface_name=$1
own_mac=$2
own_mac_no_colon=${own_mac//:/}
neighbor_ap_addr=$4
neighbor_ap_addr_no_colon=${neighbor_ap_addr//:/}
neighbor_ssid=$5

echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $neighbor_ap_addr ssid=\"$neighbor_ssid\" nr=${neighbor_ap_addr_no_colon}00000000732809030110"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $neighbor_ap_addr ssid=\"$neighbor_ssid\" nr=${neighbor_ap_addr_no_colon}00000000732809030110
echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$neighbor_ssid\" nr=${own_mac_no_colon}00000000732409030120"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$neighbor_ssid\" nr=${own_mac_no_colon}00000000732409030120
