#!/bin/sh

interface_name=$1
sta_addr=$3
neighbor_ap_addr=$4
neighbor_ssid=$5

echo ""
echo ""
echo ""
echo "Press Enter to execute test 4.2.3"
read -p "Execute 4.2.3: " exec

echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 115 0 1000 20 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 req_elements=0,48,54,70,221"
hostapd_cli -i${interface_name} req_beacon $sta_addr 115 0 1000 20 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 req_elements=0,48,54,70,221
sleep 5
echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 81 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=1,6 req_elements=0,48,54,70,221"
hostapd_cli -i${interface_name} req_beacon $sta_addr 81 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=1,6 req_elements=0,48,54,70,221
sleep 5
echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 115 0 1000 20 table ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=0 rep_cond=0,0"
hostapd_cli -i${interface_name} req_beacon $sta_addr 115 0 1000 20 table ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=0 rep_cond=0,0
sleep 5
echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 115 48 1000 112 passive ff:ff:ff:ff:ff:ff rep_detail=1 rep_cond=0,0 req_elements=0,48,54,70,221"
hostapd_cli -i${interface_name} req_beacon $sta_addr 115 48 1000 112 passive ff:ff:ff:ff:ff:ff rep_detail=1 rep_cond=0,0 req_elements=0,48,54,70,221
sleep 5
echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 115 48 1000 20 active $neighbor_ap_addr rep_detail=1 rep_cond=0,0 req_elements=0,48,54,70,221"
hostapd_cli -i${interface_name} req_beacon $sta_addr 115 48 1000 20 active $neighbor_ap_addr rep_detail=1 rep_cond=0,0 req_elements=0,48,54,70,221
sleep 5
echo "Executing: hostapd_cli -i${interface_name} req_beacon $sta_addr 115 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=36,48 req_elements=0,48,54,70,221"
hostapd_cli -i${interface_name} req_beacon $sta_addr 115 255 1000 50 active ff:ff:ff:ff:ff:ff ssid=\"$neighbor_ssid\" rep_detail=1 rep_cond=0,0 ap_ch_report=36,48 req_elements=0,48,54,70,221
