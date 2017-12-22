#!/bin/sh

single_line=$1

# Assist functions
read_own_mac()
{
	edit_conf=$1

	own_mac=`iwpriv ${interface_name} gMAC`
	own_mac=${own_mac#*:}
	own_mac=`echo $own_mac`
	if [ "$edit_conf" = "yes" ]
	then
		sed -i 's/^bssid=.*/bssid='$own_mac'/' /opt/lantiq/wave/scripts/mbo/${conf_file}_${security_mode}_${interface_name}.conf
	fi
}

get_own_ssid()
{
	own_ssid=`iwconfig $interface_name | grep ESSID`
	own_ssid=${own_ssid#*ESSID:}
	own_ssid=${own_ssid//\"/}
	own_ssid=`echo $own_ssid`
}

get_sta_mac()
{
	echo -e '\0033\0143'
	echo "Enter the STA MAC address and press Enter:"
	read -p "STA MAC Address: " sta_addr
	check_sta=`iwlist $interface_name peers | grep -ic $sta_addr`
	if [ -z $check_sta ] || [ $check_sta = 0 ]
	then
		echo "Illegal MAC address or STA not found"
		return -1
	fi
}

get_neighbor_mac()
{
	neighbor_id=$1

	echo -e '\0033\0143'
	echo "Enter the Neighbor AP MAC address and press Enter:"
	read -p "Neighbor AP MAC Address: " neighbor_ap_addr
	if [ "$neighbor_id" = "2" ]
	then
		echo -e '\0033\0143'
		echo "Enter the Neighbor AP MAC address of the 2.4GHz AP and press Enter:"
		read -p "Neighbor 2.4GHz AP MAC Address: " neighbor_ap_addr2
	fi
}

get_neighbor_ssid()
{
	echo -e '\0033\0143'
	echo "Enter the Neighbor ssid and press Enter:"
	read -p "Neighbor AP SSID: " neighbor_ssid
}

extract_single_line_params()
{
	test_section=$1
	interface_name=$2
	sta_addr=$3
	neighbor_ap_addr=$4
	neighbor_ssid=$5
	neighbor_ap_addr2=$6
	security_mode=$7
	hostapd_debug=$8
	read_own_mac

	[ "$test_section" = "OOB" ] && test_section="4.1.1"
	[ "$hostapd_debug" = "y" ] && hostapd_debug="-dd" || hostapd_debug=""
	cli_script="cli_${test_section}.sh"
}

get_conf_file()
{
	test_section=$1
	interface_name=$2

	case $test_section in
		"4.1.1")
			conf_file="hostapd_OOB"
			;;
		"4.2.3")
			conf_file="hostapd_WiFi1"
			;;
		"4.2.3_ap2")
			conf_file="hostapd_3VAPs_ap2"
			;;
		"4.2.4.1")
			conf_file="hostapd_4.2.4"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"4.2.5.1")
			conf_file="hostapd_OOB"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"4.2.5.1_ap2")
			conf_file="hostapd_4.2.5.1_ap2"
			;;
		"4.2.5.2")
			conf_file="hostapd_4.2.5.2"
			;;
		"4.2.5.3")
			conf_file="hostapd_OOB"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"4.2.5.3_ap2")
			conf_file="hostapd_4.2.5.1_ap2"
			;;
		"4.2.5.4")
			conf_file="hostapd_OOB"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"4.2.5.5")
			conf_file="hostapd_4.2.5.5"
			;;
		"4.2.7")
			conf_file="hostapd_OOB"
			;;
		"4.2.8.1")
			conf_file="hostapd_WiFi1"
			;;
		"4.2.8_ap2")
			conf_file="hostapd_4.2.8_ap2"
			;;
		"5.2.3")
			conf_file="hostapd_3VAPs"
			;;
		"5.2.3_ap2")
			conf_file="hostapd_3VAPs_ap2"
			;;
		"5.2.5.2")
			conf_file="hostapd_OOB"
			;;
		"5.2.5.2_ap2")
			conf_file="hostapd_OOB"
			;;
		"5.2.7")
			conf_file="hostapd_OOB"
			;;
		"5.2.7_ap2")
			conf_file="hostapd_OOB"
			;;
		"*")
			echo "Illegal test section entered"
			return -1
			;;
	esac
}
# If single line flag is set, show the list of parameters to set in a single line execution
if [ -n "$single_line" ]
then
	echo -e '\0033\0143'
	echo "For a single line execution of a script please set the following parameters:"
	echo "<Test section> <Interface Name> <STA MAC address> <Neighbor MAC address> <Neighbor SSID> <Neighbor 2.4GHz MAC address> <wpa2/open> <debug (y to enable)>"
	echo "If STA MAC address or Neighbor MAC address or Neighbor SSID or Neighbor 2.4GHz MAC address are not needed, type value of 0"
	echo "Test section possible values:"
	echo "	OOB"
	echo "	4.2.3"
	echo "	4.2.3_ap2"
	echo "	4.2.4.1"
	echo "	4.2.5.1"
	echo "	4.2.5.1_ap2"
	echo "	4.2.5.2"
	echo "	4.2.5.3"
	echo "	4.2.5.3_ap2"
	echo "	4.2.5.4"
	echo "	4.2.5.5"
	echo "	4.2.7"
	echo "	4.2.8.1"
	echo "	5.2.3"
	echo "	5.2.3_ap2"
	echo "	5.2.5.2"
	echo "	5.2.5.2_ap2"
	echo "	5.2.7"
	echo "	5.2.7_ap2"
	echo ""
	echo ""
	echo "Example: "
	echo "4.2.3 wlan1 11:22:33:44:55:66 66:55:44:33:22:11 WiFi1 0 wpa2 y"
	echo "<Test section> <Interface Name> <STA MAC address> <Neighbor MAC address> <Neighbor SSID> <Neighbor 2.4GHz MAC address> <wpa2/open> <debug (y to enable)>"
	read -p "Single line command: " single_line_command

	extract_single_line_params $single_line_command
	get_conf_file $test_section $interface_name
	[ "$test_section" != "5.2.3" ] && [ "$test_section" != "5.2.3_ap2" ] && [ "$test_section" != "4.2.5.1_ap2" ] && [ "$test_section" != "4.2.5.3_ap2" ] && [ "$test_section" != "4.2.8_ap2" ] && [ "$test_section" != "4.2.3_ap2" ] && read_own_mac "yes"
	echo "Restarting hostapd with the conf file: ${conf_file}_${security_mode}_${interface_name}.conf"
	killall hostapd_${interface_name}
	sleep 3
	/tmp/hostapd_${interface_name} $hostapd_debug /opt/lantiq/wave/scripts/mbo/${conf_file}_${security_mode}_${interface_name}.conf &
	sleep 4
	/tmp/hostapd_cli_${interface_name} -i${interface_name} -a/opt/lantiq/wave/scripts/fapi_wlan_wave_events_hostapd.sh -B

	if [ "$test_section" = "4.2.3_ap2" ] || [ "$test_section" = "5.2.3_ap2" ]
	then
		if [ "$interface_name" = "wlan0" ]
		then
			interface_name="wlan1"
		elif [ "$interface_name" = "wlan1" ]
		then
			interface_name="wlan0"
		fi
		echo "Restarting hostapd with the conf file: ${conf_file}_${security_mode}_${interface_name}.conf"
		killall hostapd_${interface_name}
		sleep 3
		/tmp/hostapd_${interface_name} $hostapd_debug /opt/lantiq/wave/scripts/mbo/${conf_file}_${security_mode}_${interface_name}.conf &
		sleep 4
		/tmp/hostapd_cli_${interface_name} -i${interface_name} -a/opt/lantiq/wave/scripts/fapi_wlan_wave_events_hostapd.sh -B
	fi
	if [ "$test_section" != "4.2.3" ] && [ "$test_section" != "4.2.4.1" ] && [ "${test_section:0:5}" != "4.2.5" ] && [ "$test_section" != "4.2.7" ] && [ "$test_section" != "4.2.8.1" ] && [ "$test_section" != "5.2.3" ] && [ "${test_section:0:7}" != "5.2.5.2" ] && [ "${test_section:0:5}" != "5.2.7" ]
	then
		return 0
	fi
	get_own_ssid
else
	# Set of questions to the user
	# Set interface name
	echo -e '\0033\0143'
	echo "Select the interface name and press Enter:"
	echo "[0] wlan0"
	echo "[1] wlan1"
	read -p "Interface: " interface_name

	interface_name="wlan${interface_name}"
	if [ "$interface_name" != "wlan0" ] && [ "$interface_name" != "wlan1" ]
	then
		echo "Illegal value entered"
		return -1
	fi

	# Set the test section
	echo -e '\0033\0143'
	echo "Select the test section and press Enter:"
	echo "[a] 4.1.1"
	echo "[b] 4.2.1"
	echo "[c] 4.2.2"
	echo "[d] 4.2.3"
	echo "[d2] 4.2.3_ap2"
	echo "[e] 4.2.4.1"
	echo "[f1] 4.2.5.1"
	echo "[f12] 4.2.5.1_ap2"
	echo "[f2] 4.2.5.2"
	echo "[f3] 4.2.5.3"
	echo "[f32] 4.2.5.3_ap2"
	echo "[f4] 4.2.5.4"
	echo "[f5] 4.2.5.5"
	echo "[g] 4.2.7"
	echo "[h] 4.2.8.1"
	echo "[h2] 4.2.8_ap2"
	echo "[i] 5.2.3"
	echo "[i2] 5.2.3_ap2"
	echo "[j] 5.2.5.2"
	echo "[j2] 5.2.5.2_ap2"
	echo "[k] 5.2.7"
	echo "[k2] 5.2.7_ap2"
	read  -p "Test Section: " test_section

	case $test_section in
		"a")
			test_section="4.1.1"
			conf_file="hostapd_OOB"
			;;
		"b")
			test_section="4.2.1"
			conf_file="hostapd_OOB"
			;;
		"c")
			test_section="4.2.2"
			conf_file="hostapd_OOB"
			;;
		"d")
			test_section="4.2.3"
			conf_file="hostapd_WiFi1"
			;;
		"d2")
			test_section="4.2.3_ap2"
			conf_file="hostapd_3VAPs_ap2"
			;;
		"e")
			test_section="4.2.4.1"
			conf_file="hostapd_4.2.4"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"f1")
			test_section="4.2.5.1"
			conf_file="hostapd_OOB"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"f12")
			test_section="4.2.5.1_ap2"
			conf_file="hostapd_4.2.5.1_ap2"
			;;
		"f2")
			test_section="4.2.5.2"
			conf_file="hostapd_4.2.5.2"
			;;
		"f3")
			test_section="4.2.5.3"
			conf_file="hostapd_OOB"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"f32")
			test_section="4.2.5.3_ap2"
			conf_file="hostapd_4.2.5.1_ap2"
			;;
		"f4")
			test_section="4.2.5.4"
			conf_file="hostapd_OOB"
			[ "$interface_name" = "wlan0" ] && conf_file="hostapd_ch1"
			;;
		"f5")
			test_section="4.2.5.5"
			conf_file="hostapd_4.2.5.5"
			;;
		"g")
			test_section="4.2.7"
			conf_file="hostapd_OOB"
			;;
		"h")
			test_section="4.2.8.1"
			conf_file="hostapd_WiFi1"
			;;
		"h2")
			test_section="4.2.8_ap2"
			conf_file="hostapd_4.2.8_ap2"
			;;
		"i")
			test_section="5.2.3"
			conf_file="hostapd_3VAPs"
			;;
		"i2")
			test_section="5.2.3_ap2"
			conf_file="hostapd_3VAPs_ap2"
			;;
		"j")
			test_section="5.2.5.2"
			conf_file="hostapd_OOB"
			;;
		"j2")
			test_section="5.2.5.2_ap2"
			conf_file="hostapd_OOB"
			;;
		"k")
			test_section="5.2.7"
			conf_file="hostapd_OOB"
			;;
		"k2")
			test_section="5.2.7_ap2"
			conf_file="hostapd_OOB"
			;;
		"*")
			echo "Illegal value entered"
			return -1
			;;
	esac

	cli_script="cli_${test_section}"

	# Set the action
	echo -e '\0033\0143'
	echo "Select the action and press Enter:"
	if [ "$test_section" != "4.2.5.1_ap2" ] && [ "$test_section" != "4.2.5.3_ap2" ]
	then
		echo "[1] Replace conf file"
	fi
	if [ "$test_section" = "4.2.3" ] || [ "$test_section" = "4.2.4.1" ] || [ "${test_section:0:5}" = "4.2.5" ] || [ "$test_section" = "4.2.7" ] || [ "$test_section" = "4.2.8.1" ] || [ "$test_section" = "5.2.3" ] || [ "${test_section:0:7}" = "5.2.5.2" ] || [ "${test_section:0:5}" = "5.2.7" ]
	then
		echo "[2] Execute cli script"
	fi
	read -p "Action: " action

	if [ "$action" != "1" ] && [ "$action" != "2" ]
	then
		echo "Illegal value entered"
		return -1
	fi

	# Perform the action
	if [ "$action" = "1" ]
	then
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
		[ "$test_section" != "5.2.3" ] && [ "$test_section" != "5.2.3_ap2" ] && [ "$test_section" != "4.2.5.1_ap2" ] && [ "$test_section" != "4.2.5.3_ap2" ] && [ "$test_section" != "4.2.8_ap2" ] && [ "$test_section" != "4.2.3_ap2" ] && read_own_mac "yes"
		echo "Restarting hostapd with the conf file: ${conf_file}_${security_mode}_${interface_name}.conf"
		killall hostapd_${interface_name}
		sleep 3
		/tmp/hostapd_${interface_name} $hostapd_debug /opt/lantiq/wave/scripts/mbo/${conf_file}_${security_mode}_${interface_name}.conf &
		sleep 4
		/tmp/hostapd_cli_${interface_name} -i${interface_name} -a/opt/lantiq/wave/scripts/fapi_wlan_wave_events_hostapd.sh -B

		# Set whether to execute the test itself after configuring the conf
		if [ "$test_section" = "4.2.3" ] || [ "$test_section" = "4.2.4.1" ] || [ "${test_section:0:5}" = "4.2.5" ] || [ "$test_section" = "4.2.7" ] || [ "$test_section" = "4.2.8.1" ] || [ "$test_section" = "5.2.3" ] || [ "${test_section:0:7}" = "5.2.5.2" ] || [ "${test_section:0:5}" = "5.2.7" ]
		then
			echo ""
			echo ""
			echo ""
			echo "Execute the cli script?"
			echo "[y/n]"
			read -p "Execute cli script: " exec_script
			[ "$exec_script" != "y" ] && return 0
		else
			return 0
		fi
	fi
	read_own_mac
	case "$test_section" in
		"4.2.3")
			get_sta_mac
			get_neighbor_mac
			get_neighbor_ssid
			;;
		"4.2.4.1")
			get_neighbor_mac
			get_neighbor_ssid
			;;
		"4.2.5.1")
			get_neighbor_mac
			get_neighbor_ssid
			;;
		"4.2.5.2")
			get_own_ssid
			;;
		"4.2.5.3")
			get_sta_mac
			get_neighbor_mac
			get_neighbor_ssid
			;;
		"4.2.5.4")
			get_sta_mac
			;;
		"4.2.5.5")
			get_sta_mac
			;;
		"4.2.8.1")
			get_sta_mac
			get_neighbor_mac 2
			get_neighbor_ssid
			;;
		"5.2.3")
			get_sta_mac
			get_neighbor_mac
			get_neighbor_ssid
			;;
		"5.2.5.2")
			get_sta_mac
			get_neighbor_mac
			get_neighbor_ssid
			;;
		"5.2.5.2_ap2")
			get_sta_mac
			get_neighbor_mac
			get_neighbor_ssid
			;;
	esac

	cli_script="${cli_script}.sh"
fi

# Execute the cli script
[ -z "$sta_addr" ] && sta_addr="Not Needed"
[ -z "$neighbor_ap_addr" ] && neighbor_ap_addr="Not Needed"
[ -z "$neighbor_ssid" ] && neighbor_ssid="Not Needed"
[ -z "$neighbor_ap_addr2" ] && neighbor_ap_addr2="Not Needed"
[ -z "$own_ssid" ] && own_ssid="Not Needed"

echo -e '\0033\0143'
echo "Executing /opt/lantiq/wave/scripts/mbo/${cli_script} with:"
echo "interface_name=$interface_name"
echo "Own MAC address=$own_mac"
echo "Own ssid=$own_ssid"
echo "STA MAC address=$sta_addr"
echo "Neighbor MAC address=$neighbor_ap_addr"
echo "Neighbor ssid=$neighbor_ssid"
echo "Neighbor 2.4GHz MAC address=$neighbor_ap_addr2"
/opt/lantiq/wave/scripts/mbo/${cli_script} $interface_name $own_mac "$sta_addr" "$neighbor_ap_addr" "$neighbor_ssid" "$neighbor_ap_addr2" "$own_ssid"
