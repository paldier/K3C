#!/bin/sh
. /etc/ugw_notify_defs.sh

_help()
{
	echo "Resets system configuration and applications.";
	echo "Default call is to reset only the system configuration."
	echo "Usage: $0 [ options ]";
	echo "Selective factory reset config:-"
	echo "     For now supported -wifi,-qos,-firewall";
	echo "     Example to reset wifi releated config"
	echo "     $0 -wifi "
	echo "Supported options:-";
	echo "     -a   Reset all applications";
	exit $1;
}

[ -n "$1" ] && {
	case "$1" in
		-h) _help 0;
		;;
		-a) OPT_ALL=1;
		;;
		-wifi) WiFi=1;
		;;
		-qos) QoS=1;
		;;
		-network) network=1;
		;;
		-firewall) firewall=1;
		;;
		*) _help 1;
        esac
}

#selective factory reset examples.
[ -n "$WiFi" ] && {
	echo " WiFi releated config getting reset ..."
	ubus call csd factoryreset '{ "object":"Device.WiFi."}'
	exit;
}

[ -n "$QoS" ] && {
	echo " QoS releated config getting reset ..."
	ubus call csd factoryreset '{ "object":"Device.QoS."}'
	exit;
}

[ -n "$firewall" ] && {
	echo " Firewall releated config getting reset ..."
	ubus call servd notify '{"nid":'$NOTIFY_FACTORY_RESET',"type":false}';
	ubus call csd factoryreset '{ "object":"Device.Firewall."}'
	exit;
}

[ -n "$network" ] && {
	## multiple objects to reset example ','
	echo " Network releated config getting reset ..."
	ubus call csd factoryreset '{ "object":"Device.IP.,Device.PPP."}'
	exit;
}

[ -n "$OPT_ALL" ] && {
	rm -f /opt/lantiq/servd/etc/servd.conf
} || {
	sed -i '/objcrc/d' /opt/lantiq/servd/etc/servd.conf
}

if [ -f /opt/lantiq/etc/uci_to_ipsec_config.sh ]; then
	. /opt/lantiq/etc/uci_to_ipsec_config.sh reset
	sync; sleep 1;
fi

if [ -f /etc/config/samba ]; then
	rm -f /overlay/etc/config/samba
	sync; sleep 1;
fi

if [ -f /opt/lantiq/etc/nextmac.conf ] ; then 
	rm -f /opt/lantiq/etc/nextmac.conf
	sync
fi

if [ -f /opt/lantiq/etc/.bootchk] ; then 
	rm -f /opt/lantiq/etc/.bootchk
	sync
fi

ubus call csd factoryreset
sync; sleep 2;

ubus call servd notify '{"nid":'$NOTIFY_FACTORY_RESET',"type":false}';
sync; sleep 2;

reboot

