#!/bin/sh

gpio_init()
{
        ps | grep phi_led.sh | grep -v grep > /dev/null
        if [ $? -eq 0 ]
        then
                ps | grep phi_led.sh | grep -v grep | awk '{print $1}' | xargs kill -9
        fi

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

usage()
{
        echo "gpio.sh <red|blue|yellow> <1|0>" > /dev/console
        echo "red:     red led" > /dev/console
        echo "blue:    blue led" > /dev/console
        echo "yellow:  yellow led" > /dev/console
        echo "1:       ficker" > /dev/console
        echo "0:       always bright" > /dev/console
}

if [ $# != 2 ]
then
        usage
        exit 1
fi

gpio_init
phi_led.sh $1 $2 &

