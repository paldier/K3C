#!/bin/sh

script_name="$0"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/fapi_wlan_wave_lib_common.sh

# input examples:
#   EndPoint connected:
#   /opt/lantiq/wave/scripts/fapi_wlan_wave_events_supplicant.sh wlan
interface_name=$1
name=$2
init=$3
type=$4
reg_domain=$5

# Find the interface index and the radio index
interface_index=`find_index_from_interface_name $interface_name`
local_db_source SSID
ssid_type=`db2fapi_convert regular X_LANTIQ_COM_Vendor_SsidType $interface_index`
if [ "$ssid_type" = "EndPoint" ]
then
	radio_name=`get_radio_name_from_endpoint $interface_name`
else
	radio_name=${interface_name%%.*}
fi
radio_index=`find_index_from_interface_name $radio_name`

print2log $radio_index DEBUG "$script_name $*"

endpoint_connected()
{
	# Define local parameters
	local interface_name

	interface_name=$1

	# When an endpoint is connected:
	# 1. Update the BSSID of the remote AP in SSID object
	# 2. Update the status of the EndPoint to Connected

	# Read the BSSID using wpa_cli
	bssid=`wpa_cli -i${interface_name} status | grep ^bssid=`
	bssid=${bssid##bssid=}
	build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:${interface_name} Object:${SSID_OBJECT} BSSID:${bssid}"
	build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:${interface_name} Object:${ENDPOINT_OBJECT} X_LANTIQ_COM_Vendor_ConnectionStatus:Connected"
}

endpoint_disconnected()
{
	# Define local parameters
	local interface_name

	interface_name=$1

	# When an endpoint is disconnected:
	# 1. Remove the BSSID of the remote AP in SSID object (set value of NULL in the notification, SL converts it to empty string)
	# 2. Update the status of the EndPoint to Disconnected

	build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:${interface_name} Object:${SSID_OBJECT} BSSID:NULL"
	build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:${interface_name} Object:${ENDPOINT_OBJECT} X_LANTIQ_COM_Vendor_ConnectionStatus:Disonnected"
}

regulatory_domain_changed()
{
	# Define local parameters
	local interface_name country radio_name radio

	interface_name=$1
	country=$2

	country=${country#*=}

	radios_list=`ls ${INTERFACES_STATUS}*`
	for radio in $radios_list
	do
		radio=${radio#${INTERFACES_STATUS}_}
		build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:${radio} Object:${RADIO_OBJECT} RegulatoryDomain:${country} "
	done
}

wps_success()
{
	# Define local parameters
	local interface_name

	interface_name=$1

	# Notify the web and the DB that status is "Success"
	build_wlan_notification "wsd" "NOTIFY_WPS_STATUS" "message:Success"
	#build_wlan_notification "servd" "NOTIFY_WIFI_WPS_STATUS" "Name:${interface_name} Status:Success"
	build_wlan_notification "servd" "NOTIFY_WIFI_UPDATE_PARAM" "Name:$interface_name Object:${ENDPOINT_WPS_OBJECT} X_LANTIQ_COM_Vendor_WPSStatus:Success"
}
case $name in
	"CONNECTED")
		endpoint_connected $interface_name
	;;
	"DISCONNECTED")
		endpoint_disconnected $interface_name
	;;
	"CTRL-EVENT-REGDOM-CHANGE")
		[ -n "$reg_domain" ] && regulatory_domain_changed $interface_name $reg_domain
	;;
	"WPS-SUCCESS")
		wps_success $interface_name
	;;
	*)
		echo "$script_name: $name"
	;;
esac
