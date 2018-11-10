#!/bin/sh /etc/rc.common

# Copyright (c) 2018 paldier <paldier@hotmail.com>

START=21

start()
{
#--------- init wifi driver   -----------
if [ ! -n "`lsmod | grep directconnect_datapath`" ]
then
insmod /lib/modules/3.10.104/directconnect_datapath.ko
insmod /lib/modules/3.10.104/dc_mode0-xrx500.ko
fi

cd /tmp
cp -s /opt/lantiq/lib/modules/3.10.104/net/mtlkroot.ko /tmp/
cp -s /lib/firmware/fw_scd_file.scd /tmp/
cp -s /lib/firmware/hw_scd_file.scd /tmp/
cp -s /opt/lantiq/lib/modules/3.10.104/net/mtlk.ko /tmp/

echo /opt/lantiq/sbin/hotplug > /proc/sys/kernel/hotplug
udevd_up=`ps | grep -c udevd`
[ $udevd_up -gt 1 ] || udevd --daemon
export COUNTRY=00
crda
mkdir /tmp/wlan_wave
touch /tmp/wlan_wave/crda_executed
insmod mtlkroot.ko cdebug=0 rdebug=0
cp -s /opt/lantiq/bin/logserver /tmp/
/tmp/logserver -f /tmp/dev/mtlkroot0 -s /tmp/fw_scd_file.scd &

insmod mtlk.ko ap=1,1 fastpath=1,1 ahb_off=1
}

