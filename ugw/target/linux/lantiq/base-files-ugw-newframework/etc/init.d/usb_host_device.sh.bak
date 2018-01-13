#!/bin/sh /etc/rc.common
# Copyright (c) 2013 Lantiq Deutschland GMBH
# Start-up Script for On board USB, External USB 3.0 and External eSATA modules

START=70

start() {
	local SATA_CLASS="0106"
	local USB30_CLASS="0c03"

	# Load XHCI driver
	plat_form=${CONFIG_BUILD_SUFFIX%%_*}
	platform=`echo $plat_form |tr '[:lower:]' '[:upper:]'`
	if [ "$platform" = "GRX350" -o "$platform" = "GRX550" -o "$platform" = "IRX200" ]; then
		if [ -f /lib/modules/*/xhci*.ko ]; then
			echo "Loading USB3 XHCI driver"
			insmod /lib/modules/*/xhci-*.ko
		fi
	fi

	# Load USB Host mode driver
    if [ -f /lib/modules/*/ltqusb_host.ko ]; then
        echo "Loading USB Host mode driver"
        insmod /lib/modules/*/usb-storage.ko
        insmod /lib/modules/*/ltqusb_host.ko
        insmod /lib/modules/*/usblp.ko
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
}

