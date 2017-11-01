#!/bin/sh

led_status()
{
        if [ $1 == red ]
        then
                echo 1 > /sys/class/gpio/gpio35/value
                echo 1 > /sys/class/gpio/gpio36/value
                if [ $2 == 1 ]
                then
                        echo 1 > /sys/class/gpio/gpio34/value
                else
                        echo 0 > /sys/class/gpio/gpio34/value
                fi
        elif [ $1 == blue ]
        then
                echo 0 > /sys/class/gpio/gpio34/value
                echo 1 > /sys/class/gpio/gpio36/value
                if [ $2 == 1 ]
                then
                        echo 0 > /sys/class/gpio/gpio35/value
                else
                        echo 1 > /sys/class/gpio/gpio35/value
                fi 
        elif [ $1 == yellow ]
        then
                echo 0 > /sys/class/gpio/gpio34/value
                echo 1 > /sys/class/gpio/gpio35/value
                if [ $2 == 1 ]
                then
                        echo 0 > /sys/class/gpio/gpio36/value
                else
                        echo 1 > /sys/class/gpio/gpio36/value
                fi
        fi 
}                                                            
 
if [ $2 == 1 ]
then
        while [ 1 ]
        do 
                led_status $1 1
                usleep 400000
                led_status $1 0
                usleep 400000
        done
elif [ $2 == 0 ]
then
        led_status $1 1
fi
