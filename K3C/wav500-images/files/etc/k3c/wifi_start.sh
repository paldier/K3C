#!/bin/sh /etc/rc.common

# Copyright (c) 2018 paldier <paldier@hotmail.com>

START=21

start()
{
local k3cb1 k3cb2 devinfo ez ezf
#reset-gpio 5G/2.4G or 2.4G/5G 
ez=`cat /proc/device-tree/ssx3@18000000/pcie@900000/reset-gpio | grep ""`
#5G or 2.4G
ezf=`dd if=/lib/firmware/cal_wlan0.bin skip=40 bs=1 count=7 2>/dev/null | grep "UAA"`
#ec5e22ae5718e4209ca78a96668b5a2f=K3CB2
#783b5f7beaba3069be724ae1325a9033=K3CB1
#65e230b0e840d4dbd6eeae23ce62c554=K3CC1
#0df1cf08d70206570f880cc707dfdce2=K3CA1
#A1=B1=B1G B2=C1? or C1=B1G?
#null?lost mtd4 mtd6 mtd7
devinfo=`dd if=/dev/mtd7 bs=1 skip=144 count=48 2>/dev/null |grep ec5e22ae5718e4209ca78a96668b5a2f`

[ -n "$devinfo" ] && k3cb2=1 && k3cb1=0
[ -z "$devinfo" ] && k3cb2=0 && k3cb1=1
if [ "$k3cb1" = 1 ]; then
	if [ -e "/lib/firmware/cal_wlan0.bin.b1" ]; then
		mv /lib/firmware/cal_wlan0.bin /lib/firmware/cal_wlan0.bin.bk
		mv /lib/firmware/cal_wlan1.bin /lib/firmware/cal_wlan1.bin.bk
		mv /lib/firmware/PSD.bin /lib/firmware/PSD.bin.bk
		mv /lib/firmware/cal_wlan0.bin.b1 /lib/firmware/cal_wlan0.bin
		mv /lib/firmware/cal_wlan1.bin.b1 /lib/firmware/cal_wlan1.bin
		mv /lib/firmware/PSD.bin.b1 /lib/firmware/PSD.bin
	fi
elif  [ "$k3cb2" = 1 ]; then
	if [ -e "/lib/firmware/cal_wlan0.bin.bk" ]; then
		mv /lib/firmware/cal_wlan0.bin /lib/firmware/cal_wlan0.bin.b1
		mv /lib/firmware/cal_wlan1.bin /lib/firmware/cal_wlan1.bin.b1
		mv /lib/firmware/PSD.bin /lib/firmware/PSD.bin.b1
		mv /lib/firmware/cal_wlan0.bin.bk /lib/firmware/cal_wlan0.bin
		mv /lib/firmware/cal_wlan1.bin.bk /lib/firmware/cal_wlan1.bin
		mv /lib/firmware/PSD.bin.bk /lib/firmware/PSD.bin
	fi
fi
if [ -n "$ez" ]; then
	if [ -n "$ezf" ]; then
		mv /lib/firmware/cal_wlan0.bin /lib/firmware/cal_wlan2.bin
		mv /lib/firmware/cal_wlan1.bin /lib/firmware/cal_wlan0.bin
		mv /lib/firmware/cal_wlan2.bin /lib/firmware/cal_wlan1.bin
	fi
fi
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

export COUNTRY=00
crda
mkdir /tmp/wlan_wave
touch /tmp/wlan_wave/crda_executed
insmod mtlkroot.ko cdebug=0 rdebug=0
cp -s /opt/lantiq/bin/logserver /tmp/
/tmp/logserver -f /tmp/dev/mtlkroot0 -s /tmp/fw_scd_file.scd &

insmod mtlk.ko ap=1,1 fastpath=1,1 ahb_off=1
/usr/sbin/wifireload &
}

