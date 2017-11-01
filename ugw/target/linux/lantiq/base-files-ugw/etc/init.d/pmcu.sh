#!/bin/sh /etc/rc.common
#START=76

start() {
	if [ A"$CONFIG_PACKAGE_LQ_COC_APP_PM" = "A1" ]; then
		/usr/sbin/mknod_util ifx_pmcu /dev/ifx_pmcu
		/usr/sbin/mknod_util ifx_cgu /dev/ifx_cgu
	fi
}
