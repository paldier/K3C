#!/bin/sh /etc/rc.common

START=03

#Need to enable later on once utils are in place, remove it from rcS

start() {
	touch /ramdisk/flash/passwd

	echo "root:\$1\$\$CoERg7ynjYLsj2j4glJ34.:0:0:root:/root:/bin/sh" > /ramdisk/flash/passwd
}
