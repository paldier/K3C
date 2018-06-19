#!/bin/sh

before_sync="$2"
after_sync="$3"

. /opt/lantiq/etc/ugw_notify_defs.sh

#check the first argument for NOTIFY_NTP_SYNC or NOTIFY_NTP_SYNC_ERROR

if [ "$1" = "NOTIFY_NTP_SYNC" ]; then
        ubus call servd notify '{
                "nid": '$NOTIFY_NTP_SYNC',"type": false,
                "pn1": "time_before_sync", "pv1": "'$before_sync'",
                "pn2": "time_after_sync", "pv2": "'$after_sync'"
        }'>/dev/null
elif [ "$1" = "NOTIFY_NTP_SYNC_ERROR" ]; then
        ubus call servd notify '{"nid": '$NOTIFY_NTP_SYNC_ERROR',"type": false}'>/dev/null
fi
 
