#!/bin/sh /etc/rc.common

# Copyright (c) 2018 zhaowei xu <paldier@hotmail.com>

START=21

start()
{
#--------- create wifi config  -----------
if [ ! -e /opt/lantiq/wave/db/default ]; then
fapi_wlan_cli createConfig
fi
#--------- init wifi driver   -----------
if [ ! -n "`lsmod | grep directconnect_datapath`" ]
then
insmod /lib/modules/3.10.104/directconnect_datapath.ko
insmod /lib/modules/3.10.104/dc_mode0-xrx500.ko
fi

cd /tmp
cp -s /opt/lantiq/lib/modules/3.10.104/net/mtlkroot.ko /tmp/
cp -s /opt/lantiq/wave/images/fw_scd_file.scd /tmp/
cp -s /opt/lantiq/wave/images/hw_scd_file.scd /tmp/
cp -s /opt/lantiq/lib/modules/3.10.104/net/mtlk.ko /tmp/
cp -s /opt/lantiq/wave/images/* /lib/firmware/
cp -s /opt/lantiq/bin/hostapd /tmp/hostapd_wlan0
cp -s /opt/lantiq/bin/hostapd /tmp/hostapd_wlan2
cp -s /opt/lantiq/bin/hostapd_cli /tmp/hostapd_cli_wlan0
cp -s /opt/lantiq/bin/hostapd_cli /tmp/hostapd_cli_wlan2

echo /opt/lantiq/sbin/hotplug > /proc/sys/kernel/hotplug
udevd_up=`ps | grep -c udevd`
[ $udevd_up -gt 1 ] || udevd --daemon
export COUNTRY=00
crda
touch /tmp/wlan_wave/crda_executed
insmod mtlkroot.ko cdebug=0 rdebug=0
cp -s /opt/lantiq/bin/logserver /tmp/
/tmp/logserver -f /tmp/dev/mtlkroot0 -s /tmp/fw_scd_file.scd &
#if [ ! -e /tmp/cal_wlan0.bin ]
#then
#	read_img wlanconfig /tmp/eeprom.tar.gz
#	tar xzf /tmp/eeprom.tar.gz -C /tmp/
#cp -s /tmp/cal_wlan*.bin /lib/firmware/
#fi
insmod mtlk.ko ap=1,1 fastpath=1,1 ahb_off=1
}

