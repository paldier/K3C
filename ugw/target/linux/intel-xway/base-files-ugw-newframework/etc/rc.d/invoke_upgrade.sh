#!/bin/sh

#/etc/rc.d/invoke_upgrade.sh image image_type expand saveenv reboot &

if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
fi

if [ "$5" = "reboot" ]; then
	/etc/rc.d/free_memory.sh
fi

if [ -n "$CONFIG_PACKAGE_KMOD_UBI_MTD" ]; then
	cd /mnt/data/ && {
		/usr/sbin/upgrade $@ &
	}
else
	mkdir -p /tmp/newroot/bin
	mkdir -p /tmp/newroot/lib
	mkdir -p /tmp/newroot/dev
	mkdir -p /tmp/newroot/tmp
	mv $1 /tmp/newroot/image

	if [ -n "$CONFIG_UBOOT_CONFIG_BOOT_FROM_NAND" ]; then
		cp /lib/ld-uClibc.so.0 /tmp/newroot/lib
		cp /lib/libc.so.0 /tmp/newroot/lib
		cp /lib/libdl.so.0 /tmp/newroot/lib
		cp /lib/libgcc_s.so.1 /tmp/newroot/lib
		cp /bin/busybox2 /tmp/newroot/bin/
		cd /tmp/newroot/bin; ln -s busybox2 sh; cd -
		cp /usr/sbin/nandwrite /tmp/newroot/bin/
		cp /usr/sbin/nanddump  /tmp/newroot/bin/
		cp /usr/sbin/flash_erase /tmp/newroot/bin/
		cp -a /dev/mtd* /tmp/newroot/dev/
	else
		cp -a /dev/mtd /tmp/newroot/dev/
		cp /lib/libifx_common.so /tmp/newroot/lib
	fi

	cp /usr/sbin/upgrade /tmp/newroot/bin
	cp /tmp/u_lock /tmp/newroot/tmp/.
	shift
#Process are not killed gracefully, hence killing them forcefully.
	killall -9 br2684ctld
	killall stunnel
        if [ -f /var/run/ssl.pid ] ; then
                pid=`cat /var/run/ssl.pid`
                kill $pid
		echo "stunnel process id--$pid"
        fi
	killall -9 stunnel
#ps for debug purpose to know whether any process running or not before upgrade operation.
	ps
	chroot /tmp/newroot upgrade /image $@ &
fi

