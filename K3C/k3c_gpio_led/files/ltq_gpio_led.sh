#!/bin/sh /etc/rc.common

START=03

start() {
   # Power
   if [ -d /sys/class/leds/power/ ]; then
       echo 1 > /sys/class/leds/power/brightness
   fi

   # USB
   #if [ -d /sys/class/leds/usb1_led/ ]; then
   #    echo "usbdev" > /sys/class/leds/usb1_led/trigger
   #    echo "1-1" > /sys/class/leds/usb1_led/device_name
   #fi
   #if [ -d /sys/class/leds/usb2_led/ ]; then
   #    echo "usbdev" > /sys/class/leds/usb2_led/trigger
   #    echo "3-1" > /sys/class/leds/usb2_led/device_name
   #fi

   # WLAN
   #if [ -d /sys/class/leds/wifi2g_led/ ]; then
   #    echo "netdev" > /sys/class/leds/wifi2g_led/trigger
   #    echo "wlan0" > /sys/class/leds/wifi2g_led/device_name
   #    echo "link tx rx" > /sys/class/leds/wifi2g_led/mode
   #fi
   #if [ -d /sys/class/leds/wifi5g_led/ ]; then
   #    echo "netdev" > /sys/class/leds/wifi5g_led/trigger
   #    echo "wlan1" > /sys/class/leds/wifi5g_led/device_name
   #    echo "link tx rx" > /sys/class/leds/wifi5g_led/mode
   #fi

   # WPS
   # TODO: FIXME!!
   # if [ -d /sys/class/leds/wps_led/ ]; then
   # echo 1 > /sys/class/leds/power_led/brightness
   # fi

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


   #if [ -d /sys/class/leds/internet_led/ ]; then
   #    echo "netdev" > /sys/class/leds/internet_led/trigger
   #    echo "eth1_wan1" > /sys/class/leds/internet_led/device_name
   #    echo "link tx rx" > /sys/class/leds/internet_led/mode
   #fi

   # SYS
   #if [ -d /sys/class/leds/sys_led/ ]; then
   #    echo "heartbeat" > /sys/class/leds/sys_led/trigger
   #fi
   /usr/sbin/k3cled &
}
