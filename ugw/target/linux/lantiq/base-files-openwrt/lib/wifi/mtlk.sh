#!/bin/sh
#
# Copyright (c) 2018 paldier <paldier@hotmail.com>
# lantiq 5xx 无线驱动detect脚本

append DRIVERS "mtlk"

. /lib/functions.sh
. /lib/lantiq.sh
. /lib/functions/system.sh

find_mtlk_mac() {
    phyx=$1
	[ "$phyx" = "wlan0" ] && phy_offset=0
	[ "$phyx" = "wlan1" ] && phy_offset=2
	[ "$phyx" = "wlan2" ] && phy_offset=4
	[ "$phyx" = "wlan3" ] && phy_offset=6
	board_mac=`upgrade mac_get 0`

	board_mac1=0x`echo $board_mac | cut -c 1-2`
	board_mac23=`echo $board_mac | cut -c 4-8`
	board_mac46=0x`echo $board_mac | sed s/://g | cut -c 7-12`

	board_mac46=$((board_mac46+phy_offset))
	vap_mac4=$((board_mac46/65536))
	board_mac46=$((board_mac46-vap_mac4*65536))
	vap_mac5=$((board_mac46/256))
	board_mac46=$((board_mac46-vap_mac5*256))
	vap_mac6=$board_mac46
    [ $vap_mac4 -ge 256 ] && vap_mac4=0
    mac_address=`printf '%02X:%s:%02X:%02X:%02X' $board_mac1 $board_mac23 $vap_mac4 $vap_mac5 $vap_mac6`
    echo "$mac_address"
}

detect_mtlk() {
	local macaddr

	for phyname in wlan0 wlan2; do
	{
		config_get type $phyname type
		[ "$type" == "mtlk" ] || {
			case $phyname in
				wlan2)
					hwmode=11g
					htmode=HT40+
					macaddr="$(find_mtlk_mac $phyname)"
					ssid="K3C-2.4G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
					;;
				wlan0)
					hwmode=11a
					htmode=VHT80
					macaddr="$(find_mtlk_mac $phyname)"
					ssid="K3C-5G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
					;;
			esac
			
		[ -n "$macaddr" ] && {
			dev_id="set wireless.${phyname}.macaddr=${macaddr}"
		}
		[ ! -e /etc/config/wireless ] && touch /etc/config/wireless
		uci -q batch <<EOF
			set wireless.${phyname}=wifi-device
			set wireless.${phyname}.type=mtlk
			${dev_id}
			set wireless.${phyname}.hwmode=$hwmode
			set wireless.${phyname}.channel=auto
			set wireless.${phyname}.htmode=$htmode
			set wireless.${phyname}.country=CN
			set wireless.${phyname}.beacon_int=100
			set wireless.${phyname}.disabled=0
			
			set wireless.default_${phyname}=wifi-iface
			set wireless.default_${phyname}.device=${phyname}
			set wireless.default_${phyname}.network=lan
			set wireless.default_${phyname}.mode=ap
			set wireless.default_${phyname}.ssid=${ssid}
			set wireless.default_${phyname}.encryption=none
EOF
		uci -q commit wireless
		}
	}
	done

}

