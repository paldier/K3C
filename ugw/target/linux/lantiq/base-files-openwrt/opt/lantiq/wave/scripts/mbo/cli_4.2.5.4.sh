#!/bin/sh

interface_name=$1
own_mac=$2
own_mac_no_colon=${own_mac//:/}
sta_addr=$3

echo ""
echo ""
echo ""
echo "Press Enter to execute test 4.2.5.4"
read -p "Execute 4.2.5.4: " exec

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr bss_term=36000,5 pref=1 neighbor=${own_mac},10,81,36,9,0 mbo=9:0:0 disassoc_timer=45"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr bss_term=36000,5 pref=1 neighbor=${own_mac},10,81,36,9,0 mbo=9:0:0 disassoc_timer=45
echo "sleep 5"
sleep 5
echo "killall hostapd_${interface_name}"
killall hostapd_${interface_name}
