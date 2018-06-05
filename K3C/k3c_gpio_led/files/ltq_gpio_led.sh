#!/bin/sh /etc/rc.common

START=03

start() {
   # Power
   if [ -d /sys/class/leds/power/ ]; then
       echo 1 > /sys/class/leds/power/brightness
   fi

   # WAN 
   if [ -d /sys/class/leds/wan/ ]; then
       echo "netdev" > /sys/class/leds/wan/trigger
       echo "eth1" > /sys/class/leds/wan/device_name
       echo "link tx rx" > /sys/class/leds/wan/mode
   fi
   # Eth
   if [ -d /sys/class/leds/lan1/ ]; then
       echo "netdev" > /sys/class/leds/lan1/trigger
       echo "eth0_3" > /sys/class/leds/lan1/device_name
       echo "link tx rx" > /sys/class/leds/lan1/mode
   fi
   if [ -d /sys/class/leds/lan2/ ]; then
       echo "netdev" > /sys/class/leds/lan2/trigger
       echo "eth0_2" > /sys/class/leds/lan2/device_name
       echo "link tx rx" > /sys/class/leds/lan2/mode
   fi
   if [ -d /sys/class/leds/lan3/ ]; then
       echo "netdev" > /sys/class/leds/lan3/trigger
       echo "eth0_4" > /sys/class/leds/lan3/device_name
       echo "link tx rx" > /sys/class/leds/lan3/mode
   fi

   /usr/sbin/k3cled &
}
