#!/bin/sh

if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
		CONFIGLOADED="1"
		plat_form=${CONFIG_BUILD_SUFFIX%%_*}
		platform=`echo $plat_form |tr '[:lower:]' '[:upper:]'`
	fi
fi

[ -n "$CONFIG_NEW_FRAMEWORK" ] && . /etc/ugw_notify_defs.sh

DSL_STATUS_UP=1
DSL_STATUS_DOWN=0

TC_MODE=$1
STATUS=$2
NEW_TC=$3

echo "network interface config called with status : $STATUS for tc_mode = $TC_MODE with new_tc = $NEW_TC"

if [ "$STATUS" = "$DSL_STATUS_UP" ]; then
	[ -n "$CONFIG_NEW_FRAMEWORK" ] && {
		([ -n "$CONFIG_PACKAGE_KMOD_VRX518_DP_MOD" ] || 
			[ -n "$CONFIG_PACKAGE_KMOD_VRX518_TC_DRV" ] || 
			[ -n "$CONFIG_PACKAGE_KMOD_VRX318_DP_MOD" ]) && {
				echo -n
		} || {
			([ -n "$TC_MODE" ] && [ "$TC_MODE" = "ptm" ]) && {
				ubus call servd notify '{"notify_id" : '$NOTIFY_WANMODE_CHANGE', "type" : false, "pn1" : "Type", "pv1" : "WAN", "pn2" : "Mode", "pv2" : "PTM", "pn3" : "LinkEnable", "pv3" : "true" }'
				ifconfig ptm0 up
				for __ii in `grep -E 'ptm' /etc/config/network|grep -w name|cut -d\' -f2|cut -d_ -f1`; do
					ifconfig $__ii up;
				done
				for __ii in `grep -E 'ptm' /etc/config/network|grep -w name|cut -d\' -f2|rev|cut -d_ -f1|rev`; do
					ifup $__ii;
				done
			}
		}	

		([ -n "$TC_MODE" ] && [ "$TC_MODE" = "atm" ]) && {
			ubus call servd notify '{ "notify_id": '$NOTIFY_L2_IFACE_ADD', "type": true, "pn1": "iface", "pv1": "atm" }' > /dev/null
			ubus call servd notify '{"notify_id" : '$NOTIFY_WANMODE_CHANGE', "type" : false, "pn1" : "Type", "pv1" : "WAN", "pn2" : "Mode", "pv2" : "ATM", "pn3" : "LinkEnable", "pv3" : "true" }'
			for __ii in `grep -E 'nas' /etc/config/network|grep -w name|cut -d\' -f2|cut -d_ -f1`; do
				ifconfig $__ii up;
			done
			for __ii in `grep -E 'nas' /etc/config/network|grep -w name|cut -d\' -f2|rev|cut -d_ -f1|rev`; do
				ifup $__ii;
			done
			oamd &
		}
	}
elif [ "$STATUS" = "$DSL_STATUS_DOWN" ]; then
	[ -n "$CONFIG_NEW_FRAMEWORK" ] && {
		([ -n "$TC_MODE" ] && [ "$TC_MODE" = "atm" ]) && {
			killall oamd &
			for __ii in `grep -E 'nas' /etc/config/network|grep -w name| cut -d\' -f2`; do
				ucisection=`echo $__ii | sed -n 's/.*_//;1p'`
				proto=`uci get network.${ucisection}.proto`
				[ "${proto::4}" = "pppo" ] &&
					ubus call servd notify '{ "notify_id": '$NOTIFY_IFACE_DELETE', "type": true, "pn1": "conn_ifname", "pv1": "'${proto}'-'${ucisection}'" }' > /dev/null
				
				ubus call servd notify '{ "notify_id": '$NOTIFY_IFACE_DELETE', "type": true, "pn1": "conn_ifname", "pv1": "'${__ii}'" }' > /dev/null
			done
			for __ii in `grep -E 'nas' /etc/config/network|grep -w name|cut -d\' -f2|cut -d_ -f1`; do
				ifconfig $__ii down;
			done
			for __ii in `grep -E 'nas' /etc/config/network|grep -w name|cut -d\' -f2|rev|cut -d_ -f1|rev`; do
				ifdown $__ii;
			done
			ubus call servd notify '{"notify_id" : '$NOTIFY_WANMODE_CHANGE', "type" : false, "pn1" : "Type", "pv1" : "WAN", "pn2" : "Mode", "pv2" : "ATM", "pn3" : "LinkEnable", "pv3" : "false" }'
			ubus call servd notify '{ "notify_id": '$NOTIFY_L2_IFACE_REM', "type": true, "pn1": "iface", "pv1": "atm" }' > /dev/null
		}
		([ -n "$CONFIG_PACKAGE_KMOD_VRX518_DP_MOD" ] || 
			[ -n "$CONFIG_PACKAGE_KMOD_VRX518_TC_DRV" ] || 
			[ -n "$CONFIG_PACKAGE_KMOD_VRX318_DP_MOD" ]) && {
				echo -n
		} || {
			([ -n "$TC_MODE" ] && [ "$TC_MODE" = "ptm" ]) && {
				for __ii in `grep -E 'ptm' /etc/config/network|grep -w name| cut -d\' -f2`; do
					ucisection=`echo $__ii | sed -n 's/.*_//;1p'`
					proto=`uci get network.${ucisection}.proto`
					[ "${proto::4}" = "pppo" ] &&
						ubus call servd notify '{ "notify_id": '$NOTIFY_IFACE_DELETE', "type": true, "pn1": "conn_ifname", "pv1": "'${proto}'-'${ucisection}'" }' > /dev/null
				
					ubus call servd notify '{ "notify_id": '$NOTIFY_IFACE_DELETE', "type": true, "pn1": "conn_ifname", "pv1": "'$__ii'" }' > /dev/null
				done
				for __ii in `grep -E 'ptm' /etc/config/network|grep -w name|cut -d\' -f2|cut -d_ -f1`; do
					ifconfig $__ii down;
				done
				ifconfig ptm0 down
				for __ii in `grep -E 'ptm' /etc/config/network|grep -w name|cut -d\' -f2|rev|cut -d_ -f1|rev`; do
					ifdown $__ii;
				done
				ubus call servd notify '{"notify_id" : '$NOTIFY_WANMODE_CHANGE', "type" : false, "pn1" : "Type", "pv1" : "WAN", "pn2" : "Mode", "pv2" : "PTM", "pn3" : "LinkEnable", "pv3" : "false" }'
			}
		}
	}
fi

([ -n "$TC_MODE" ] ) && {
        ([ -n "$NEW_TC" ] && ([ "$NEW_TC" = "ptm" ] || [ "$NEW_TC" = "atm" ] )) && {
		echo "sending switchover event to servd with new_tc = $NEW_TC"
		ubus call servd notify '{ "notify_id": '$NOTIFY_DSL_SWITCHOVER', "type": true, "pn1": "newtc", "pv1": "'$NEW_TC'", "pn2": "oldtc", "pv2": "'$TC_MODE'" }' > /dev/null
        }
}

