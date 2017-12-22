#!/bin/sh

interface_name=$1
own_mac=$2

echo ""
echo ""
echo ""
echo "Press Enter to execute step 2 for AP2"
read -p "Execute step 2 for AP2: " exec_step2

echo "Executing: hostapd_cli -i${interface_name} mbo_bss_assoc_disallow $own_mac 0"
hostapd_cli -i${interface_name} mbo_bss_assoc_disallow $own_mac 0

echo ""
echo ""
echo ""
echo "Press Enter to execute step 7 for AP2"
read -p "Execute step 7 for AP2: " exec_step7

echo "Executing: hostapd_cli -i${interface_name} mbo_bss_assoc_disallow $own_mac 1"
hostapd_cli -i${interface_name} mbo_bss_assoc_disallow $own_mac 1
