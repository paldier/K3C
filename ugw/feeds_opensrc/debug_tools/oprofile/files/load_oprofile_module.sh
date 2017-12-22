#!/bin/sh /etc/rc.common

START=99

start(){

if [ -f "/lib/modules/2.6.32.32/oprofile.ko" ]; then
        insmod /lib/modules/2.6.32.32/oprofile.ko > /dev/null

cat /proc/mounts | grep nfs > /dev/null
if [ $? -eq 1 ]; then
        mini_fo.sh mount /root
fi

fi
}

