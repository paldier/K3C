#!/bin/sh


	echo "*** Disabling Port mirror ***, device goes for reboot...."
        # re-set switch mirror register
	switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x453 nData=0x3FF

	# re-set previous monitor port

	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=0 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=1 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=2 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=4 eEnable=1 ePortMonitor=0
	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=5 eEnable=1 ePortMonitor=0

	switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=6 eEnable=1 ePortMonitor=0

	# re-set previous mirror port

	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=0 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=1 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=2 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=4 bMonitorPort=0
	switch_cli IFX_ETHSW_MONITOR_PORT_CFG_SET nPortId=5 bMonitorPort=0

	echo none > /proc/mirror

	reboot -f

