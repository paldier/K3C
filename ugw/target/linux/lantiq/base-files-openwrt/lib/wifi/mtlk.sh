#!/bin/sh
#
# Copyright (c) 2018 paldier <paldier@hotmail.com>

append DRIVERS "mtlk"

scan_mtlk() {
	local device="$1"
	local wlan_mode=disabled
	config_get vifs "$device" vifs
	for vif in $vifs; do
		config_get_bool disabled "$vif" disabled 0
		[ $disabled -eq 0 ] || continue
		config_get mode "$vif" mode
		case "$mode" in
			sta)
				wlan_mode=sta
			;;
			ap)
				wlan_mode=ap
			;;
			*) echo "$device($vif): Invalid mode";;
		esac
		break
	done

	radio=1
	ap=0
	sta=0
	[ $wlan_mode = disabled ] && radio=0
	[ $wlan_mode = ap  ] && ap=1
	[ $wlan_mode = sta ] && sta=1
}


disable_mtlk() {
	kill -9 $(ps | grep hostapd | cut -c1-5)
	fapi_wlan_cli unInit
	rmmod mtlk
	return 0
}

detect_mtlk() {

	[ -e /etc/config/wireless ] || {
        touch /etc/config/wireless
		uci batch <<EOF
			set wireless.wlan0=wifi-device
			set wireless.wlan0.type=mtlk
			set wireless.wlan0.hwmode=11g
			set wireless.wlan0.channel=auto
			set wireless.wlan0.txpower=100
			set wireless.wlan0.htmode=HT40
			set wireless.wlan0.country=CN
			set wireless.wlan0.txburst=1
			set wireless.wlan0.noscan=1

			set wireless.default_wlan0=wifi-iface
			set wireless.default_wlan0.device=wlan0
			set wireless.default_wlan0.network=lan
			set wireless.default_wlan0.mode=ap
			set wireless.default_wlan0.ssid=K3C
			set wireless.default_wlan0.encryption=none

			set wireless.wlan2=wifi-device
			set wireless.wlan2.type=mtlk
			set wireless.wlan2.hwmode=11ac
			set wireless.wlan2.channel=auto
			set wireless.wlan2.txpower=100
			set wireless.wlan2.htmode=VHT80
			set wireless.wlan2.country=CN
			set wireless.wlan2.txburst=1
			set wireless.wlan2.noscan=1

			set wireless.default_wlan2=wifi-iface
			set wireless.default_wlan2.device=wlan2
			set wireless.default_wlan2.network=lan
			set wireless.default_wlan2.mode=ap
			set wireless.default_wlan2.ssid=K3C_5G
			set wireless.default_wlan2.encryption=none
EOF
		uci -q commit wireless
		}

}

