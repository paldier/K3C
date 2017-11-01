#!/bin/sh

_help()
{
	echo "Resets system configuration and applications.";
	echo "Default call is to reset only the system configuration."
	echo "Usage: $0 [ options ]";
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
		*) _help 1;
        esac
}

[ -n "$2" ] && {
        case "$2" in
                notreboot) OPT_REBOOT=0;
                ;;
                *) OPT_REBOOT=1;
        esac
}||{
        OPT_REBOOT=1;
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

if [ -f /opt/lantiq/etc/nextmac.conf ] ; then 
	rm -f /opt/lantiq/etc/nextmac.conf
	sync
fi

if [ -f /etc/wlan-default-ssid ] ; then
	rm -f /etc/wlan-default-ssid
	sync;
fi

ubus call csd factoryreset
sync; sleep 2;

ubus call servd notify '{"nid":125,"type":false}';
sync; sleep 2;

rm -rf /overlay/*
sync; sleep 2;

if [ $OPT_REBOOT == 1 ]; then
	reboot
fi

