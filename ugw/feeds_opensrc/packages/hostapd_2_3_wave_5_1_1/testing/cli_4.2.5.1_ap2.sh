#!/bin/sh

interface_name=$1


killall hostapd_${interface_name}
echo ""
echo ""
echo ""
echo "Press Enter to bring up AP2"
read -p "Bring up AP2 : " bring_up

echo -e '\0033\0143'
echo "Select the security mode and press Enter:"
echo "[1] Open"
echo "[2] WPA2"
read -p "Security mode: " security_mode
if [ "$security_mode" = "1" ]
then
    security_mode="open"
elif [ "$security_mode" = "2" ]
then
    security_mode="wpa2"
else
    echo "Illegal value entered"
    return -1
fi

echo -e '\0033\0143'
echo "Set hostapd in debug mode?"
echo "[y/n]"
read -p "Set hostapd debug: " hostapd_debug

[ "$hostapd_debug" = "y" ] && hostapd_debug="-dd" || hostapd_debug=""
echo "Starting hostapd with the conf file: hostapd_4.2.5.1_ap2_${security_mode}_${interface_name}.conf"
killall hostapd_${interface_name}
sleep 3
/tmp/hostapd_${interface_name} $hostapd_debug /opt/lantiq/wave/scripts/mbo/hostapd_4.2.5.1_ap2_${security_mode}_${interface_name}.conf &
sleep 4
/tmp/hostapd_cli_${interface_name} -i${interface_name} -a/opt/lantiq/wave/scripts/fapi_wlan_wave_events_hostapd.sh -B
