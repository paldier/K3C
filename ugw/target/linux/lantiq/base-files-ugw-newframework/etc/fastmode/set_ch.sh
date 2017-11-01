#!/bin/sh
conf_2g=/tmp/wlan_wave/hostapd_wlan1.conf
conf_5g=/tmp/wlan_wave/hostapd_wlan0.conf
current_ch_2g=`cat /tmp/wlan_wave/hostapd_wlan1.conf |grep ^channel=|cut -d"=" -f 2`
current_ch_5g=`cat /tmp/wlan_wave/hostapd_wlan0.conf |grep ^channel=|cut -d"=" -f 2`

ch_2g()
{
	if [ ${current_ch_2g} -eq ${1} ];then
		echo "2g: current ch is the same as aspired one, abandoned."
		exit 1
	fi
	sed -i "/^channel=/d" $conf_2g
	echo "channel=$1">>$conf_2g
	if [ ${1} -ge 1 ] && [ ${1} -le 7 ];then
		sed -i 's/ht_capab=\[HT40-/ht_capab=\[HT40+/g' $conf_2g
	fi
	if [ ${1} -ge 8 ] && [ ${1} -le 13 ];then
		sed -i 's/ht_capab=\[HT40+/ht_capab=\[HT40-/g' $conf_2g
	fi
}

ch_5g()
{
	if [ ${current_ch_5g} -eq ${1} ];then
		echo "5g: current ch is the same as aspired one, abandoned."
		exit 1
	fi
	if [ ${1} -eq 165 ];then
		cp /tmp/wlan_wave/hostapd_wlan0.conf /tmp/.hostapd_wlan0.conf
	fi
	if [ ${current_ch_5g} -eq 165 ];then
		cp /tmp/.hostapd_wlan0.conf /tmp/wlan_wave/hostapd_wlan0.conf
	fi
	sed -i "/^channel=/d" $conf_5g
	echo "channel=$1">>$conf_5g
	if [ $1 = "36" ] || [ $1 = "40" ] || [ $1 = "44" ] || [ $1 = "149" ] || [ $1 = "157" ];then
		sed -i 's/ht_capab=\[HT40-/ht_capab=\[HT40+/g' $conf_5g
	fi
	if [ $1 = "153" ] || [ $1 = "161" ] || [ $1 = "48" ];then
		sed -i 's/ht_capab=\[HT40+/ht_capab=\[HT40-/g' $conf_5g
	fi
	if [ ${1} -ge 36 ] && [ ${1} -le 48 ];then
		sed -i "/^vht_oper_centr_freq_seg0_idx=/d" $conf_5g
		echo "vht_oper_centr_freq_seg0_idx=42">>$conf_5g
	fi
	if [ ${1} -ge 149 ] && [ ${1} -le 161 ];then
		sed -i "/^vht_oper_centr_freq_seg0_idx=/d" $conf_5g
		echo "vht_oper_centr_freq_seg0_idx=155">>$conf_5g
	fi
	if [ $1 = "165" ];then
		sed -i "s/^ht_capab=\[HT40+\]\[SHORT-GI-20\]\[SHORT-GI-40\]/ht_capab=\[SHORT-GI-20\]/g" $conf_5g
		sed -i "s/^ht_capab=\[HT40-\]\[SHORT-GI-20\]\[SHORT-GI-40\]/ht_capab=\[SHORT-GI-20\]/g" $conf_5g
		sed -i 's/vht_oper_chwidth=1/vht_oper_chwidth=0/g' $conf_5g
		sed -i "/^vht_oper_centr_freq_seg0_idx=/d" $conf_5g
		echo "vht_oper_centr_freq_seg0_idx=165">>$conf_5g
		sed -i "/^ignore_40_mhz_intolerant=0/d" $conf_5g
	fi
}

if [ $# -ne 2 ]; then
	echo "param is invalid"
	exit 1
fi

region=`cat /tmp/wlan_wave/hostapd_wlan1.conf|grep country_code|cut -d"=" -f 2`
if [ $region = "CN" ];then
	if [ $1 = "2g" ];then
		if [ ${2} -ge 1 ] && [ ${2} -le 13 ];then :
		else
			exit 1
		fi
	fi

	if [ $1 = "5g" ];then
		if [[ "$2" = "36" ]] || [[ "$2" = "40" ]] || [[ "$2" = "44" ]] || [[ "$2" = "48" ]] || [[ "$2" = "149" ]] || [[ "$2" = "153" ]] || [[ "$2" = "157" ]] || [[ "$2" = "161" ]] || [[ "$2" = "165" ]]; then :
		else
			exit 1
		fi
	fi
elif [ $region = "US" ];then
	if [ $1 = "2g" ];then
		if [ ${2} -ge 1 ] && [ ${2} -le 11 ];then :
		else
			exit 1
		fi
	fi

	if [ $1 = "5g" ];then
		if [[ "$2" = "36" ]] || [[ "$2" = "40" ]] || [[ "$2" = "44" ]] || [[ "$2" = "48" ]] || [[ "$2" = "149" ]] || [[ "$2" = "153" ]] || [[ "$2" = "157" ]] || [[ "$2" = "161" ]] || [[ "$2" = "165" ]]; then :
		else
			exit 1
		fi
	fi
elif [ $region = "DE" ];then
	if [ $1 = "2g" ];then
		if [ ${2} -ge 1 ] && [ ${2} -le 13 ];then :
		else
			exit 1
		fi
	fi

	if [ $1 = "5g" ];then
		if [[ "$2" = "36" ]] || [[ "$2" = "40" ]] || [[ "$2" = "44" ]] || [[ "$2" = "48" ]]; then :
		else
			exit 1
		fi
	fi

fi

if [ $1 = "2g" ];then
	ch_2g $2
	sh /tmp/wlan_wave/runner_up_wlan1.sh
	echo "Test=OK"
elif [ $1 = "5g" ];then
	ch_5g $2
	sh /tmp/wlan_wave/runner_up_wlan0.sh
	echo "Test=OK"
fi
