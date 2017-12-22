#!/bin/sh /etc/rc.common
# Copyright (C) 2015 Lantiq Beteiligungs-GmbH Co. KG

START=81
STOP=06

BOND_IF_NAME=eth0_0

start() {

	insmod /lib/modules/*/ltq_eth_bond_dp.ko

	sleep 1
	ifconfig $BOND_IF_NAME up -arp
}

stop () {

	ifconfig $BOND_IF_NAME down

	rmmod /lib/modules/*/ltq_eth_bond_dp.ko
}

