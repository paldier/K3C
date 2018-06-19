#!/bin/sh

event_name="$1"
mac="$2"
ip_addr="$3"
time_at="$DNSMASQ_LEASE_EXPIRES"
leasetime="$DNSMASQ_TIME_REMAINING"
if [ "$4" == "" ]; then
	hostname="Unknown"
else
	hostname="$4"
fi

. /opt/lantiq/etc/ugw_notify_defs.sh

if [ "$event_name" == "add" ] || [ "$event_name" == "old" ]; then
        ubus call servd notify '{
                "notify_id": '$NOTIFY_DHCP_CLIENT_UP', "type": false,
                "pn1": "ip_addr", "pv1": "'$ip_addr'",
                "pn2": "time_at", "pv2": "'$time_at'",
                "pn3": "mac", "pv3": "'$mac'" ,
                "pn4": "active", "pv4": "1" ,
                "pn5": "leasetime", "pv5": "'$leasetime'",
                "pn6": "hostname", "pv6": "'$hostname'"
        }'

elif [ "$event_name" == "del" ]; then
        ubus call servd notify '{
                "notify_id": '$NOTIFY_DHCP_CLIENT_DOWN', "type": false,
                "pn1": "ip_addr", "pv1": "'$ip_addr'",
                "pn2": "time_at", "pv2": "'$time_at'",
                "pn3": "mac", "pv3": "'$mac'" ,
                "pn4": "active", "pv4": "0" ,
                "pn5": "leasetime", "pv5": "'$leasetime'"
        }'
fi

# send DHCP event to Beerocks module for managing IRE's/STA's
# this would be moved to Beerocks App SL when it's available
ubus call dhcp_event dhcp_event '{
	"op":"'$event_name'",
	"mac":"'$mac'",
	"ip":"'$ip_addr'",
	"hostname":"'$hostname'"
}'
