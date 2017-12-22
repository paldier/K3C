#!/bin/sh

interface_name=$1
own_mac=$2
own_mac_no_colon=${own_mac//:/}
own_ssid=$7

echo "Executing: hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$own_ssid\" nr=${own_mac_no_colon}00000000732409030120"
hostapd_cli -i${interface_name} set_neighbor_per_vap $interface_name $own_mac ssid=\"$own_ssid\" nr=${own_mac_no_colon}00000000732409030120
echo "Executing: hostapd_cli -i${interface_name} cellular_pref_set $interface_name 1"
hostapd_cli -i${interface_name} cellular_pref_set $interface_name 1
