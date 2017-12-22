#!/bin/sh

interface_name=$1
own_mac=$2

echo "Executing: hostapd_cli -i${interface_name} mbo_bss_assoc_disallow $own_mac 1"
hostapd_cli -i${interface_name} mbo_bss_assoc_disallow $own_mac 1
