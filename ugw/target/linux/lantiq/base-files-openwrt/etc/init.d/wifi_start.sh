#!/bin/sh /etc/rc.common

# Copyright (c) 2018 paldier <paldier@hotmail.com>

START=99

start()
{
if [ ! -e /opt/lantiq/wave/fapi_rpc_mode ]; then
    touch /opt/lantiq/wave/fapi_rpc_mode
fi
if [ ! -e /opt/lantiq/wave/db/default ]; then
    /opt/lantiq/wave/scripts/fapi_wlan_wave_createDB.sh
fi

#--------- fapi_wlan_cli init . -----------
if [ ! -n "`lsmod | grep directconnect_datapath`" ]
then
insmod /lib/modules/3.10.104/directconnect_datapath.ko
insmod /lib/modules/3.10.104/dc_mode0-xrx500.ko
fi
#bad idea
if [ -e /opt/lantiq/wave/db/instance/wlan_fapi_mapping ]; then
fapi_wlan_cli factory
else
fapi_wlan_cli init
fi
#--------- fapi_wlan_cli init End. -----------
}

