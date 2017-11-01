#!/bin/sh

###################################
#
# Port0 as Lan with VID 25
# Port1 as Lan with VID 35
# Port2 as Lan with VID 45
# Port4 as Wan with VID 5
# CPU is attached to Port6 in AR9
#
###################################

brctl delif br0 eth0
ifconfig br0 down
brctl delbr br0
vconfig add eth0 25
vconfig add eth0 35
vconfig add eth0 45
vconfig add eth0 5
ifconfig eth0 down

#turn switch off
mem -s 0x1e1080cc -w 0x000004e1 -u
sleep 1

#set speed for port4,5,6 as 1000Mbit/s
mem -s 0x1e1080cc -w 0x0bbb04f5 -u
sleep 1

#enable port4,6
#mem -s 0x1e1080cc -w 0x00040481 -u
#sleep 1
mem -s 0x1e1080cc -w 0x000404c1 -u
sleep 1

#CPU is attached to port6
mem -s 0x1e1080cc -w 0x00c004e2 -u
sleep 1

#grouping port0,1,2,4,6 individually
mem -s 0x1e1080cc -w 0x04410403 -u
sleep 1
mem -s 0x1e1080cc -w 0x04420423 -u
sleep 1
mem -s 0x1e1080cc -w 0x04440443 -u
sleep 1
mem -s 0x1e1080cc -w 0x04500483 -u
sleep 1
mem -s 0x1e1080cc -w 0x043f04c3 -u
sleep 1

#set vid for port0,1,2,4,6
mem -s 0x1e1080cc -w 0x00190404 -u
sleep 1
mem -s 0x1e1080cc -w 0x00230424 -u
sleep 1
mem -s 0x1e1080cc -w 0x002d0444 -u
sleep 1
mem -s 0x1e1080cc -w 0x00050484 -u
sleep 1
mem -s 0x1e1080cc -w 0x10cc04c4 -u
sleep 1

#turn switch on
mem -s 0x1e1080cc -w 0x800004e1 -u
sleep 1

ifconfig eth0 up
ifconfig eth0.25 10.10.10.254 netmask 255.255.255.0 up
ifconfig eth0.35 10.10.20.254 netmask 255.255.255.0 up
ifconfig eth0.45 10.10.30.254 netmask 255.255.255.0 up
ifconfig eth0.5 192.168.1.254 netmask 255.255.255.0 up
sleep 1

echo wan lo 0 /proc/eth/genconf
echo wan hi 10 /proc/eth/genconf
ppacmd addlan -i eth0.25
ppacmd addlan -i eth0.35
ppacmd addlan -i eth0.45
ppacmd addwan -i eth0.5
ppacmd control --enable-lan --enable-wan

