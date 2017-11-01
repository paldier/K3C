#!/bin/sh
. /lib/functions/system.sh
lan_mac=`ifconfig br-lan|grep HWaddr|awk {'print $5'}`
ssid_mac=`echo $lan_mac|cut -d':' -f 6`

init_5g()
{
	wlan0_mac=$(macaddr_add "$lan_mac" 1)
	sed -i "s/bssid=00:C0:20:22:22:21/bssid=$wlan0_mac/g" ./wlan_wave/hostapd_wlan0.conf
	sed -i "s/ssid=@PHICOMM_20_5G/ssid=@PHICOMM_${ssid_mac}_5G/g" ./wlan_wave/hostapd_wlan0.conf
	./wlan_wave/runner_up_wlan0.sh
}

init_2g()
{
	wlan1_mac=$(macaddr_add "$lan_mac" 2)
	sed -i "s/bssid=00:C0:20:22:22:22/bssid=$wlan1_mac/g" ./wlan_wave/hostapd_wlan1.conf
	sed -i "s/ssid=@PHICOMM_20/ssid=@PHICOMM_${ssid_mac}/g" ./wlan_wave/hostapd_wlan1.conf
	./wlan_wave/runner_up_wlan1.sh
}

cp -f /etc/fastmode/wlan_wave.tar.gz /tmp/
cd /tmp
tar -xvf wlan_wave.tar.gz
./wlan_wave/runner_hw_init.sh
if [ $1 = "2g" ]; then
	init_2g
elif [ $1 = "5g" ]; then
	init_5g
elif [ $1 = 'wifi' ]; then
	init_5g
	init_2g
else
	exit 1
fi
