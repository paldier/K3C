#!/bin/sh
#
# Copyright (c) 2018-2019 paldier <paldier@hotmail.com>
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

lookup_phy() {
	[ -n "$phy" ] && {
		[ -d /sys/class/ieee80211/$phy ] && return
	}

	local devpath
	config_get devpath "$device" path
	[ -n "$devpath" ] && {
		for phy in $(ls /sys/class/ieee80211 2>/dev/null); do
			case "$(readlink -f /sys/class/ieee80211/$phy/device)" in
				*$devpath) return;;
			esac
		done
	}

	local device="$(config_get "$device" device)"
	[ -n "$device" ] && {
		for _phy in /sys/class/ieee80211/*; do
			[ -e "$_phy" ] || continue

			[ "$device" = "wlan0" ] && phy="phy0" && return
			[ "$device" = "wlan1" ] && phy="phy1" && return
			[ "$device" = "wlan2" ] && phy="phy2" && return
			[ "$device" = "wlan3" ] && phy="phy3" && return
			
		done
	}
	phy=
	return
}

find_mtlk_phy() {
	local device="$1"

	config_get phy "$device" phy
	lookup_phy
	[ -n "$phy" -a -d "/sys/class/ieee80211/$phy" ] || {
		echo "PHY for wifi device $1 not found"
		return 1
	}
	config_set "$device" phy "$phy"

	return 0
}

check_mtlk_device() {
	config_get phy "$1" phy
	[ -z "$phy" ] && {
		find_mtlk_phy "$1" >/dev/null || return 0
		config_get phy "$1" phy
	}
}

detect_mtlk() {
	local macaddr ez

	for phyname in wlan0 wlan2; do
	{

		config_get type $phyname type
		[ "$type" == "mtlk" ] || {

			mode=ap
			disabled=0
			network=lan
			case $phyname in
				wlan0)
					[ "$(lantiq_board_name)" == "BlueCave" ] && {
						hwmode=11g
						htmode=HT40+
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="OpenWrt-2.4G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
					}
					[ "$(lantiq_board_name)" == "Phicomm K3C" ] && {
						hwmode=11a
						htmode=VHT80
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="K3C-5G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
					}
					;;
				wlan2)
					[ "$(lantiq_board_name)" == "BlueCave" ] && {
						hwmode=11a
						htmode=VHT80
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="OpenWrt-5G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
					}
					[ "$(lantiq_board_name)" == "Phicomm K3C" ] && {
						hwmode=11g
						htmode=HT40+
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="K3C-2.4G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
					}
					;;
				wlan1)
					[ "$(lantiq_board_name)" == "BlueCave" ] && {
						hwmode=11g
						htmode=HT40+
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="OpenWrt-2.4G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
						mode=sta
						disabled=1
					}
					[ "$(lantiq_board_name)" == "Phicomm K3C" ] && {
						hwmode=11a
						htmode=VHT80
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="K3C-5G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
						mode=sta
						disabled=1
					}
					;;
				wlan3)
					[ "$(lantiq_board_name)" == "BlueCave" ] && {
						hwmode=11a
						htmode=VHT80
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="OpenWrt-5G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
						mode=sta
						disabled=1
					}
					[ "$(lantiq_board_name)" == "Phicomm K3C" ] && {
						hwmode=11g
						htmode=HT40+
						macaddr="$(find_mtlk_mac $phyname)"
						ssid="K3C-2.4G-$(echo $macaddr | awk -F ":" '{print $4""$5""$6 }'| tr a-z A-Z)"
						mode=sta
						disabled=1
					}
					;;
			esac

		[ -n "$macaddr" ] && {
			dev_id="set wireless.${phyname}.macaddr=${macaddr}"
		}
		[ "$mode" = "sta" ] && {
			wds="set wireless.default_${phyname}.wds=1"
			network=wifi
		}
		config_foreach check_mtlk_device wifi-device

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
			set wireless.${phyname}.disabled=$disabled
			
			set wireless.default_${phyname}=wifi-iface
			set wireless.default_${phyname}.device=${phyname}
			set wireless.default_${phyname}.network=${network}
			set wireless.default_${phyname}.mode=$mode
			${wds}
			set wireless.default_${phyname}.ssid=${ssid}
			set wireless.default_${phyname}.encryption=none
EOF
		uci -q commit wireless
		}
	}
	done

}

