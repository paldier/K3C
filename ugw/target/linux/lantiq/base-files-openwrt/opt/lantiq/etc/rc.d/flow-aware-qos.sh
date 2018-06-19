#!/bin/sh
# Script name: flow-aware-qos.sh
# 
# 1. marks traffic into 3 categories HIGH, MEDIUM & LOW  based on configuration done below through environment variable 
#     L7HIGH, L7LOW, L7DEF, TCPHIGH, TCPLOW, TCPDEF, UDPHIGH, UDPLOW, UDPDEF
# 2. supports ingress queueing based on predefined marks FLOW_MARKLOW=0x00000400, FLOW_MARKMED=0x00000000, FLOW_MARKHIGH=0x00001C00
# 3. supports egress queueing (imq0 based)  on interface given in command line
# 4. uses port/protocol/layer7  based classification
# 5. takes upstream & downstream rates from command line argument
# 6. usage: "flow-aware-qos.sh <start/stop/restart> <interface name> <ds rate in kbps> <us rate in kbps>"


if [ ! "$ENVLOADED" ]; then
        if [ -r /etc/rc.conf ]; then
                 . /etc/rc.conf 2> /dev/null
		# To Read Remark VID & VPRIO From Tmp. It is tempurary.
		if [ -r /tmp/system_status ]; then
                	. /tmp/system_status 2> /dev/null
		fi
                ENVLOADED="1"
        fi
fi

if [ ! "$CONFIGLOADED" ]; then
        if [ -r /etc/rc.d/config.sh ]; then
                . /etc/rc.d/config.sh 2>/dev/null
        fi
        if [ -r /etc/rc.d/config_qos.sh ]; then
                . /etc/rc.d/config_qos.sh 2>/dev/null
        fi
        if [ -r /etc/extmark.sh ]; then
                . /etc/extmark.sh 2>/dev/null
        fi
                CONFIGLOADED="1"
fi
if [ -r //etc/rc.d/layer7.sh ]; then
. /etc/rc.d/layer7.sh
layer7_init
fi


# specify protocols to classify using Layer 7
# supported protocols can be found at /etc/l7-protocols/protocols/

L7HIGH="youtube netflix telnet ssh dns"
L7LOW="bittorrent ftp"
L7DEF="smtp"

TCPHIGH=23,22,53
TCPLOW=445,21,20,139
TCPDEF=443

UDPHIGH=53
UDPLOW=
UDPDEF=


FLOW_IMQ=imq0
FLOW_EWAN=eth1
FLOW_IWAN=eth1


FLOW_DOWN_PORTRATE=50000
FLOW_UP_PORTRATE=50000

BURST1=20k
BURST2=5k
BURST3=1k
FLOW_RATE1=$FLOW_DOWN_PORTRATE
FLOW_RATE2=$FLOW_DOWN_PORTRATE
FLOW_RATE3=$FLOW_DOWN_PORTRATE
FLOW_MARKLOW=0x00000400
FLOW_MARKMED=0x00000000
FLOW_MARKHIGH=0x00001C00
FLOW_MARKMASK=0x00001C00
INGRESS_POLICING_FLAG=0

init_var() 
{
	FLOW_IMQ=imq0
	FLOW_EWAN=$1

	FLOW_DOWN_PORTRATE=$2
	FLOW_UP_PORTRATE=$3
	BURST1=20k
	BURST2=5k
	BURST3=1k
	FLOW_RATE1=$FLOW_DOWN_PORTRATE
	FLOW_RATE2=$FLOW_DOWN_PORTRATE
	FLOW_RATE3=$FLOW_DOWN_PORTRATE
	FLOW_MARKLOW=0x00000400
	FLOW_MARKMED=0x00000000
	FLOW_MARKHIGH=0x00001C00
	FLOW_MARKMASK=0x00001C00
	

}
tcp_init()
{
	iptables -t mangle -I PREROUTING  -i br+ -p tcp -m multiport --dports $TCPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	iptables -t mangle -I PREROUTING  -i br+ -p tcp -m multiport --dports $TCPLOW -j MARK --set-mark $FLOW_MARKLOW/${FLOW_MARKMASK}
	iptables -t mangle -I PREROUTING  -i br+ -p tcp -m multiport --dports $TCPDEF -j MARK --set-mark $FLOW_MARKMED/${FLOW_MARKMASK}

	iptables -t mangle -I PREROUTING  ! -i br+ -p tcp -m multiport --sports $TCPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	iptables -t mangle -I PREROUTING  ! -i br+ -p tcp -m multiport --sports $TCPLOW -j MARK --set-mark $FLOW_MARKLOW/${FLOW_MARKMASK}
	iptables -t mangle -I PREROUTING  ! -i br+ -p tcp -m multiport --sports $TCPDEF -j MARK --set-mark $FLOW_MARKMED/${FLOW_MARKMASK}
	
}
tcp_exit()
{
	iptables -t mangle -D PREROUTING  -i br+ -p tcp -m multiport --dports $TCPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	iptables -t mangle -D PREROUTING  -i br+ -p tcp -m multiport --dports $TCPLOW -j MARK --set-mark $FLOW_MARKLOW/${FLOW_MARKMASK}
	iptables -t mangle -D PREROUTING  -i br+ -p tcp -m multiport --dports $TCPDEF -j MARK --set-mark $FLOW_MARKMED/${FLOW_MARKMASK}

	iptables -t mangle -D PREROUTING  ! -i br+ -p tcp -m multiport --sports $TCPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	iptables -t mangle -D PREROUTING  ! -i br+ -p tcp -m multiport --sports $TCPLOW -j MARK --set-mark $FLOW_MARKLOW/${FLOW_MARKMASK}
	iptables -t mangle -D PREROUTING  ! -i br+ -p tcp -m multiport --sports $TCPDEF -j MARK --set-mark $FLOW_MARKMED/${FLOW_MARKMASK}
}

udp_init()
{
	iptables -t mangle -I PREROUTING  -i br+ -p tcp -m multiport --sports $UDPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	iptables -t mangle -I PREROUTING  ! -i br+ -p tcp -m multiport --dports $UDPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
}
udp_exit()
{
	iptables -t mangle -D PREROUTING  -i br+ -p tcp -m multiport --sports $UDPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	iptables -t mangle -D PREROUTING  ! -i br+ -p tcp -m multiport --dports $UDPHIGH -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
}

l7_init()
{
	 insmod /lib/modules/*/xt_layer7.ko 2>/dev/null
         echo 1 > /proc/sys/net/netfilter/nf_conntrack_acct
         
	for i in $L7LOW 
	do
		iptables -t mangle -I PREROUTING -m layer7 --l7proto $i -j MARK --set-mark $FLOW_MARKLOW/${FLOW_MARKMASK}
	done

	for i in $L7DEF 
	do
		iptables -t mangle -I PREROUTING -m layer7 --l7proto $i -j MARK --set-mark $FLOW_MARKMED/${FLOW_MARKMASK}
	done
	for i in $L7HIGH 
	do
		iptables -t mangle -I PREROUTING -m layer7 --l7proto $i -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	done
}

l7_exit()
{
	for i in $L7LOW 
	do
		iptables -t mangle -D PREROUTING -m layer7 --l7proto $i -j MARK --set-mark $FLOW_MARKLOW/${FLOW_MARKMASK}
	done

	for i in $L7DEF 
	do
		iptables -t mangle -D PREROUTING -m layer7 --l7proto $i -j MARK --set-mark $FLOW_MARKMED/${FLOW_MARKMASK}
	done
	for i in $L7HIGH 
	do
		iptables -t mangle -D PREROUTING -m layer7 --l7proto $i -j MARK --set-mark $FLOW_MARKHIGH/${FLOW_MARKMASK}
	done
}

delete_ipt_rules()
{
	tcp_exit
	udp_exit
	l7_exit
}

init_ipt_rules()
{
	delete_ipt_rules 2>/dev/null
	l7_init
	udp_init
	tcp_init
}


imq_exit()
{

	iptables -t mangle -D POSTROUTING -o $FLOW_EWAN -j IMQ --todev 0 2>/dev/null
	ifconfig imq0 down
}
imq_init()
{
	ifconfig imq0 up
	iptables -t mangle -I POSTROUTING -o $FLOW_EWAN -j IMQ --todev 0
}

down_qdisc_exit()
{
	tc qdisc del dev $FLOW_IMQ root 2>/dev/null
}

down_qdisc_init()
{
	down_qdisc_exit
	ifconfig $FLOW_IMQ up

	tc qdisc add dev $FLOW_IMQ root handle 1: htb default 12 r2q 40
	tc class add dev $FLOW_IMQ parent 1:0 classid 1:10 htb rate ${FLOW_DOWN_PORTRATE}kbit ceil ${FLOW_DOWN_PORTRATE}kbit prio 1 mtu 1500

	tc class add dev $FLOW_IMQ parent 1:10 classid 1:11 htb rate $(( $FLOW_RATE1 * 8/10 ))kbit ceil ${FLOW_DOWN_PORTRATE}kbit burst $BURST1 prio 0 mtu 1500 
	tc qdisc add dev $FLOW_IMQ parent 1:11 handle 11: fq_codel 
#	tc-latest qdisc add dev $FLOW_IMQ parent 1:11 handle 11: pfifo limit 100 

	tc class add dev $FLOW_IMQ parent 1:10 classid 1:12 htb rate $(( $FLOW_RATE2 * 1/10 ))kbit ceil ${FLOW_DOWN_PORTRATE}kbit burst $BURST2 prio 1 mtu 1500
#	tc qdisc add dev $FLOW_IMQ parent 1:12 handle 12: pfifo limit 4 
	tc-latest qdisc add dev $FLOW_IMQ parent 1:12 handle 12: fq_codel 

	tc class add dev $FLOW_IMQ parent 1:10 classid 1:13 htb rate 100kbit ceil ${FLOW_DOWN_PORTRATE}kbit burst $BURST3 prio 2 mtu 1500
#	tc qdisc add dev $FLOW_IMQ parent 1:13 handle 13: pfifo limit 2 
	tc-latest qdisc add dev $FLOW_IMQ parent 1:13 handle 13: fq_codel  


	tc filter add dev $FLOW_IMQ parent 1:0 protocol ip u32 match mark $FLOW_MARKHIGH 0xffffffff flowid 1:11
	tc filter add dev $FLOW_IMQ parent 1:0 protocol ip u32 match mark $FLOW_MARKMED 0xffffffff flowid 1:12
	tc filter add dev $FLOW_IMQ parent 1:0 protocol ip u32 match mark $FLOW_MARKLOW 0xffffffff flowid 1:13
}

up_qdisc_exit()
{
	WINTF="eth1_0"
	tc qdisc del dev $WINTF root 2>/dev/null
}
up_qdisc_init()
{
	WINTF="eth1_0"
	ifconfig $WINTF up

	tc qdisc add dev $WINTF root handle 1: htb default 12 r2q 40
	tc class add dev $WINTF parent 1:0 classid 1:10 htb rate $(( $FLOW_DOWN_PORTRATE * 1/23))kbit ceil ${FLOW_DOWN_PORTRATE}kbit prio 1 mtu 1500

	tc class add dev $WINTF parent 1:10 classid 1:11 htb rate $(( $FLOW_RATE1 * 8/10 *1/23 ))kbit ceil ${FLOW_DOWN_PORTRATE}kbit burst $BURST1 prio 0 mtu 1500 
#	tc qdisc add dev $WINTF parent 1:11 handle 11: fq_codel 
	tc-latest qdisc add dev $WINTF parent 1:11 handle 11: pfifo limit 100 

	tc class add dev $WINTF parent 1:10 classid 1:12 htb rate $(( $FLOW_RATE2 * 1/10 *1/23 ))kbit ceil ${FLOW_DOWN_PORTRATE}kbit burst $BURST2 prio 1 mtu 1500
	tc qdisc add dev $WINTF parent 1:12 handle 12: pfifo limit 4 
#	tc-latest qdisc add dev $WINTF parent 1:12 handle 12: fq_codel 

	tc class add dev $WINTF parent 1:10 classid 1:13 htb rate 100kbit ceil ${FLOW_DOWN_PORTRATE}kbit burst $BURST3 prio 2 mtu 1500
	tc qdisc add dev $WINTF parent 1:13 handle 13: pfifo limit 2 
#	tc-latest qdisc add dev $WINTF parent 1:13 handle 13: fq_codel  


	tc filter add dev $WINTF parent 1:0 protocol ip u32 match mark $FLOW_MARKHIGH 0xffffffff flowid 1:11
	tc filter add dev $WINTF parent 1:0 protocol ip u32 match mark $FLOW_MARKMED 0xffffffff flowid 1:12
	tc filter add dev $WINTF parent 1:0 protocol ip u32 match mark $FLOW_MARKLOW 0xffffffff flowid 1:13
}


ingress_queue_init()
{
	BURST=300k
	insmod /lib/modules/*/act_police.ko 2>/dev/null
	insmod /lib/modules/*/sch_ingress.ko 2>/dev/null
	tc qdisc add dev $FLOW_IWAN handle ffff: ingress
	tc filter add dev ${FLOW_IWAN} parent ffff: protocol ip prio 10 u32 match extmark ${FLOW_MARKHIGH} ${FLOW_MARKMASK}  police rate ${FLOW_DOWN_PORTRATE}kbit burst $BURST mtu 1500 drop  flowid :1
	#tc filter add dev ${FLOW_IWAN} parent ffff: protocol ip prio 20 u32 match extmark ${FLOW_MARKMED} ${FLOW_MARKMASK}  police rate $(( $FLOW_DOWN_PORTRATE * 8/10 ))kbit burst     $(( $FLOW_DOWN_PORTRATE * 8/10 * 1/10))kbit mtu 1500 drop  flowid :1
	tc filter add dev ${FLOW_IWAN} parent ffff: protocol ip prio 20 u32 match extmark ${FLOW_MARKMED} ${FLOW_MARKMASK}  police rate $(( $FLOW_DOWN_PORTRATE * 8/10 ))kbit burst $BURST  mtu 1500 drop  flowid :1
	tc filter add dev ${FLOW_IWAN} parent ffff: protocol ip prio 30 u32 match extmark ${FLOW_MARKLOW} ${FLOW_MARKMASK}  police rate $(( $FLOW_DOWN_PORTRATE * 7/10 ))kbit burst $BURST  mtu 1500 drop  flowid :1
	#tc filter add dev ${FLOW_IWAN} parent ffff: protocol ip prio 30 u32 match extmark ${FLOW_MARKLOW} ${FLOW_MARKMASK}  police rate $(( $FLOW_DOWN_PORTRATE * 7/10 ))kbit burst     $(( $FLOW_DOWN_PORTRATE * 7/10 * 1/10))kbit mtu 1500 drop  flowid :1
}
ingress_queue_exit()
{
	tc qdisc del dev $FLOW_IWAN handle ffff: ingress
}


ingress_queue_show()
{
	tc -s filter show dev $FLOW_IWAN parent ffff: 
}

egress_queue_init()
{


	insmod /lib/modules/*/sch_sfq.ko 2>/dev/null
	
	# clean existing down- and uplink qdiscs, hide errors
	tc qdisc del dev $FLOW_IMQ root    2> /dev/null > /dev/null
	tc qdisc del dev $FLOW_IMQ ingress 2> /dev/null > /dev/null

	###### uplink

	# install root HTB, point default traffic to 1:20:
	tc qdisc add dev $FLOW_IMQ root handle 1: htb default 20

	# shape everything at $FLOW_UP_PORTRATE speed - this prevents huge queues in your
	# DSL modem which destroy latency:

	tc class add dev $FLOW_IMQ parent 1: classid 1:1 htb rate ${FLOW_UP_PORTRATE}kbit burst 6k

	# high prio class 1:10:
	tc class add dev $FLOW_IMQ parent 1:1 classid 1:10 htb rate ${FLOW_UP_PORTRATE}kbit burst 6k prio 1

	# bulk & default class 1:20 - gets slightly less traffic, 
	# and a lower priority:

	tc class add dev $FLOW_IMQ parent 1:1 classid 1:20 htb rate $(( $FLOW_UP_PORTRATE * 9/10 ))kbit burst 6k prio 2

	tc class add dev $FLOW_IMQ parent 1:1 classid 1:30 htb rate $(( $FLOW_UP_PORTRATE * 8/10 ))kbit burst 6k prio 2

	# all get Stochastic Fairness:
	tc qdisc add dev $FLOW_IMQ parent 1:10 handle 10: sfq perturb 10
	tc qdisc add dev $FLOW_IMQ parent 1:20 handle 20: sfq perturb 10
	tc qdisc add dev $FLOW_IMQ parent 1:30 handle 30: sfq perturb 10

	# TOS Minimum Delay (ssh, NOT scp) in 1:10:

	tc filter add dev $FLOW_IMQ parent 1:0 protocol ip prio 10 u32 match ip tos 0x10 0xff  flowid 1:10

	# ICMP (ip protocol 1) in the interactive class 1:10 so we 
	# can do measurements & impress our friends:
	tc filter add dev $FLOW_IMQ parent 1:0 protocol ip prio 10 u32 match ip protocol 1 0xff flowid 1:10

	# To speed up downloads while an upload is going on, put ACK packets in
	# the interactive class:

	tc filter add dev $FLOW_IMQ parent 1: protocol ip prio 10 u32 \
	   match ip protocol 6 0xff \
	   match u8 0x05 0x0f at 0 \
	   match u16 0x0000 0xffc0 at 2 \
	   match u8 0x10 0xff at 33 \
	   flowid 1:10

	tc filter add dev $FLOW_IMQ parent 1: protocol ip prio 18 u32 \
	   match ip dst 0.0.0.0/0 flowid 1:20

	   imq_init
}

egress_queue_exit()
{
	# clean existing down- and uplink qdiscs, hide errors
	tc qdisc del dev $FLOW_IMQ root    2> /dev/null > /dev/null
	tc qdisc del dev $FLOW_IMQ ingress 2> /dev/null > /dev/null
	imq_exit
}

if [ "z$2" = "z" ]; then
	INGRESS_POLICING_FLAG=0
fi


if [ "z$3" != "z" -a "$3" != "0" ]; then
	FLOW_DOWN_PORTRATE=$3
	INGRESS_POLICING_FLAG=1
fi

if [ $2 == "auto" ]; then

case $wanphy_phymode in
2)
	FLOW_EWAN=eth1
	FLOW_IWAN=eth1
	;;
3)
	FLOW_EWAN=ptm0
	FLOW_IWAN=ptm0
	;;
4)
	FLOW_EWAN=ptm0
	FLOW_IWAN=ptm0
	;;
5)
	FLOW_EWAN=nas0
	FLOW_IWAN=nas0
	;;
*)
	INGRESS_POLICING_FLAG=0
	;;
esac

fi
	

init_var $2 $3 $4

#start function. 
flow_start()
{
	#down_qdisc_init
	#imq_init
	#egress_queue_init
	if [ $INGRESS_POLICING_FLAG -eq 1 ]; then
		ingress_queue_init
	fi
	init_ipt_rules
	#up_qdisc_init
}
#stop function. 
flow_stop()
{
	#down_qdisc_exit
	#imq_exit
	#egress_queue_init
	ingress_queue_exit
	delete_ipt_rules
	#up_qdisc_exit
}
case $1 in

start)
		
	flow_start
	;;
stop)
	flow_stop
	;;
restart)
	flow_stop
	flow_start
	ingress_queue_show
	;;
*)
	echo "usage: $0 <start/stop/restart> <interface name> <rate in kbps>"
	;;
esac
	

#ppacmd control --disable-lan
