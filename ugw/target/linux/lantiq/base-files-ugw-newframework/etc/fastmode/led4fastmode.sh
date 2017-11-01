#!/bin/sh

red_sw=/sys/class/gpio/gpio34/value
blue_sw=/sys/class/gpio/gpio35/value
yellow_sw=/sys/class/gpio/gpio36/value

lightup_all()
{
	echo 1 > $red_sw
	echo 0 > $blue_sw
	echo 0 > $yellow_sw
}

shutdown_all()
{
	echo 0 > $red_sw
	echo 1 > $blue_sw
	echo 1 > $yellow_sw
}

led_read()
{
	red=`cat $red_sw`
	blue=`cat $blue_sw`
	yellow=`cat $yellow_sw`
	if [ $red = "1" ] && [ $blue = "0" ] && [ $yellow = "0" ];then
		echo "Led=open"
		exit 0
	elif [ $red = "0" ] && [ $blue = "1" ] && [ $yellow = "1" ];then
		echo "Led=close"
		exit 0
	elif [ $red = "1" ] && [ $blue = "1" ] && [ $yellow = "1" ];then
		echo "Led=red"
		exit 0
	elif [ $red = "0" ] && [ $blue = "0" ] && [ $yellow = "1" ];then
		echo "Led=blue"
		exit 0
	elif [ $red = "0" ] && [ $blue = "1" ] && [ $yellow = "0" ];then
		echo "Led=yellow"
		exit 0
	else
		echo "Led=NG"
		exit 1
	fi
}

led_init()
{
	if [ ! -d "/sys/class/gpio/gpio34" ]
	then
		echo 34 > /sys/class/gpio/export;
		echo out > /sys/class/gpio/gpio34/direction;
		echo 35 > /sys/class/gpio/export;
		echo out > /sys/class/gpio/gpio35/direction;
		echo 36 > /sys/class/gpio/export;
		echo out > /sys/class/gpio/gpio36/direction;
	fi
}

led_init
if [ $1 = "open" ];then
	lightup_all
elif [ $1 = "close" ];then
	shutdown_all
elif [ $1 = "read" ];then
	led_read
else
	echo "value is invalid"
	exit 1
fi
