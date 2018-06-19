#!/bin/sh
#
# Copyright (c) 2018 paldier <paldier@hotmail.com>

append DRIVERS "mtlk"

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

			set wireless.def_wlan0=wifi-iface
			set wireless.def_wlan0.device=wlan0
			set wireless.def_wlan0.network=lan
			set wireless.def_wlan0.mode=ap
			set wireless.def_wlan0.ssid=K3C
			set wireless.def_wlan0.encryption=none

			set wireless.wlan2=wifi-device
			set wireless.wlan2.type=mtlk
			set wireless.wlan2.hwmode=11ac
			set wireless.wlan2.channel=auto
			set wireless.wlan2.txpower=100
			set wireless.wlan2.htmode=VHT80
			set wireless.wlan2.country=CN
			set wireless.wlan2.txburst=1
			set wireless.wlan2.noscan=1

			set wireless.def_wlan2=wifi-iface
			set wireless.def_wlan2.device=wlan2
			set wireless.def_wlan2.network=lan
			set wireless.def_wlan2.mode=ap
			set wireless.def_wlan2.ssid=K3C_5G
			set wireless.def_wlan2.encryption=none
EOF
		uci -q commit wireless
		}

}

