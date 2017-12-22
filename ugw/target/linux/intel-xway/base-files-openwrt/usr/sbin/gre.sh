#!/bin/sh

tun_type=$1
oper=$2
tunnel_name=$3
wan_ipaddr=$4
peer_ipaddr=$5

help()
{
    echo "Invalid argument:"
    echo "    ./gre.sh EoGRE ADD <tunnel name> <wan ip addr> <peer ip addr> <lan bridge name>"
    echo "    ./gre.sh EoGRE DEL <tunnel name>"
    echo "    ./gre.sh IPoGRE ADD <tunnel name> <wan ip addr> <peer ip addr> <tunnel ip addr> <tunnel netmask>"
    echo "    ./gre.sh IPoGRE DEL <tunnel name>"
    echo ""
    exit
}

configure_eogre()
{
    lan_br_name=$1
    
    uci set network.$tunnel_name=interface
    uci set network.$tunnel_name.proto='gretap'
    uci set network.$tunnel_name.ipaddr=$wan_ipaddr
    uci set network.$tunnel_name.peeraddr=$peer_ipaddr
    uci set network.$tunnel_name.network=$lan_br_name
    uci commit

    ubus call network reload
}

destroy_eogre()
{
    uci delete network.$tunnel_name
    uci commit

    ubus call network reload
}

configure_ipogre()
{
    tun_ipaddr=$1
    tun_subnet_prefix=$2

    uci set network.$tunnel_name=interface
    uci set network.$tunnel_name.proto='gre'
    uci set network.$tunnel_name.ipaddr=$wan_ipaddr
    uci set network.$tunnel_name.peeraddr=$peer_ipaddr
    uci set network.$tunnel_name.tunipaddr=$tun_ipaddr
    uci set network.$tunnel_name.tunnetmask=$tun_subnet_prefix
    uci commit

    # Allow GRE Traffic from WAN
    val=$(uci add firewall rule)
    uci set firewall.$val.name='Allow-GRE'
    uci set firewall.$val.src='wan'
    uci set firewall.$val.proto='gre'
    uci set firewall.$val.family='ipv4'
    uci set firewall.$val.target='ACCEPT'
    uci commit firewall

    /etc/init.d/firewall restart 2>/dev/null

    ubus call network reload
}

destroy_ipogre()
{
    uci delete network.$tunnel_name
    uci commit

    ubus call network reload
}

[ $# -lt 3 ] && help
case $oper in
    ADD)
        if [ "$tun_type" == "EoGRE" ]; then
            [ $# -ne 6 ] && help
            configure_eogre $6
        elif [ "$tun_type" == "IPoGRE" ]; then
            [ $# -ne 7 ] && help
            configure_ipogre $6 $7
        else
            help
        fi
    ;;

    DEL)
        if [ "$tun_type" == "EoGRE" ]; then
            [ $# -ne 3 ] && help
            destroy_eogre
        elif [ "$tun_type" == "IPoGRE" ]; then
            [ $# -ne 3 ] && help
            destroy_ipogre
        else
            help
        fi
    ;;

    *)
        help
esac
