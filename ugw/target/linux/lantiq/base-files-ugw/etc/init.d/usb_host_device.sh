#!/bin/sh /etc/rc.common
# Copyright (c) 2013 Lantiq Deutschland GMBH
# Start-up Script for On board USB, External USB 3.0 and External eSATA modules

START=70


if [ ! "$ENVLOADED" ]; then
	if [ -r /etc/rc.conf ]; then
		. /etc/rc.conf 2> /dev/null
		ENVLOADED="1"
	fi
fi

if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
fi

start() {
	local SATA_CLASS="0106"
	local USB30_CLASS="0c03"

	# Load USB Host mode driver
	if [ "`cat /proc/device-tree/fpi*/usb*/status`" = "okay" ]; then
		if [ -f /lib/modules/*/ltqusb_host.ko ]; then
			echo "Loading USB Host mode driver"
			insmod /lib/modules/*/usb-storage.ko
			insmod /lib/modules/*/ltqusb_host.ko
			insmod /lib/modules/*/usblp.ko
		fi
	fi

	# Load USB Device mode driver
	if [ $usb0Mode -eq 2 -o $usb1Mode -eq 2 ]; then
		echo "" > /tmp/cmd_output
		ifconfig eth0 > /tmp/cmd_output
		local MAC_ADDR_BASE=`/bin/sed -n 's,^.*HWaddr,,;1p' /tmp/cmd_output`
		local USB_MAC_ADDR=`echo $MAC_ADDR_BASE|/usr/sbin/next_macaddr -7`
		rm -f /tmp/cmd_output

		if [ -f /lib/modules/*/ifxusb_gadget.ko ] && [ -f /lib/modules/*/g_ether.ko ]; then
			echo "Loading USB Device mode driver"
			/sbin/insmod /lib/modules/*/ltqusb_gadget.ko
			local board_mac=`/usr/sbin/upgrade mac_get 0`
			/sbin/insmod /lib/modules/*/g_ether.ko dev_addr="$USB_MAC_ADDR"
			/usr/sbin/brctl addif $lan_main_0_interface usb0
			/sbin/ifconfig usb0 0.0.0.0 up
		fi
	fi

	# Load eSATA driver
	lspci 2>/dev/null | grep -qw "$SATA_CLASS:" && {
		echo "Found SATA module connected on PCI(e)"
		if [ -f /lib/modules/*/ahci.ko ]; then
			insmod /lib/modules/*/libahci.ko
			insmod /lib/modules/*/ahci.ko
		fi
	} || true

	# Load USB-3.0 driver
	lspci 2>/dev/null | grep -qw "$USB30_CLASS:" && {
		echo "Found USB module connected on PCI(e)"
		if [ -f /lib/modules/*/xhci*.ko ]; then
			insmod /lib/modules/*/usb-storage.ko
			insmod /lib/modules/*/xhci*.ko
		fi
	} || true

	# Load XHCI driver
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2> /dev/null
		plat_form=${CONFIG_BUILD_SUFFIX%%_*}
		platform=`echo $plat_form |tr '[:lower:]' '[:upper:]'`
	fi
	if [ "$platform" = "GRX350" -o "$platform" = "GRX550" ]; then
		if [ -f /lib/modules/*/xhci*.ko ]; then
			echo "Loading USB3 XHCI driver"
			insmod /lib/modules/*/xhci-*.ko
			#for LRO cantidates mark the packet
			#/usr/sbin/iptables -t mangle -I INPUT -d ${lan_main_0_ipAddr} -p tcp -m state --state ESTABLISHED -j EXTMARK --set-mark 0x80000000/0x80000000
			/usr/sbin/iptables -t mangle -I INPUT -p tcp -m state --state ESTABLISHED -j EXTMARK --set-mark 0x80000000/0x80000000
		fi
	fi
}

