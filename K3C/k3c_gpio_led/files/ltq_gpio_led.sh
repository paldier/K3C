#!/bin/sh /etc/rc.common

START=03

start() {
   # Power
   if [ -d /sys/class/leds/power_led/ ]; then
       echo 1 > /sys/class/leds/power_led/brightness
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
   if [ -d /sys/class/leds/g1led0/ ]; then
       echo "netdev" > /sys/class/leds/g1led0/trigger
       echo "eth1" > /sys/class/leds/g1led0/device_name
       echo "link tx rx" > /sys/class/leds/g1led0/mode
   fi
   # Eth
   if [ -d /sys/class/leds/g2led0/ ]; then
       echo "netdev" > /sys/class/leds/g2led0/trigger
       echo "eth0_2" > /sys/class/leds/g2led0/device_name
       echo "link tx rx" > /sys/class/leds/g2led0/mode
   fi
   if [ -d /sys/class/leds/g3led0/ ]; then
       echo "netdev" > /sys/class/leds/g3led0/trigger
       echo "eth0_3" > /sys/class/leds/g3led0/device_name
       echo "link tx rx" > /sys/class/leds/g3led0/mode
   fi
   if [ -d /sys/class/leds/g4led0/ ]; then
       echo "netdev" > /sys/class/leds/g4led0/trigger
       echo "eth0_4" > /sys/class/leds/g4led0/device_name
       echo "link tx rx" > /sys/class/leds/g4led0/mode
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
}
