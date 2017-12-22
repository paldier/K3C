#!/bin/sh

interface_name=$1
sta_addr=$3
neighbor_ap_addr=$4
neighbor_ssid=$5
neighbor_ap_addr2=$6

echo ""
echo ""
echo ""
echo "Press Enter to execute step 3"
read -p "Execute step 3: " exec_step3

echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 81 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=1 req_elements=0,48,70"
hostapd_cli -i${interface_name} req_beacon $sta_addr 81 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=1 req_elements=0,48,70

echo ""
echo ""
echo ""
echo "Press Enter to execute step 5"
read -p "Execute step 5: " exec_step5

echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 115 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=44 req_elements=0,48,70"
hostapd_cli -i${interface_name} req_beacon $sta_addr 115 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=44 req_elements=0,48,70

echo ""
echo ""
echo ""
echo "Press Enter to execute step 7 only after the STA sets reject"
read -p "Execute step 7: " exec_step7

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${neighbor_ap_addr2},0,81,1,9,1 neighbor=${neighbor_ap_addr},0,115,44,9,0 mbo=4:0:0"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${neighbor_ap_addr2},0,81,1,9,1 neighbor=${neighbor_ap_addr},0,115,44,9,0 mbo=4:0:0

echo ""
echo ""
echo ""
echo "Press Enter to execute step 10 only after WNM sent from STA"
read -p "Execute step 10: " exec_step10

echo "Executing: hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${neighbor_ap_addr},0,115,44,9,0 mbo=4:0:0 disassoc_imminent=1"
hostapd_cli -i${interface_name} bss_tm_req $sta_addr pref=1 neighbor=${neighbor_ap_addr},0,115,44,9,1 mbo=4:0:0 disassoc_imminent=1
