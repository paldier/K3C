#!/bin/sh /etc/rc.common

START=03

#Need to enable later on once utils are in place, remove it from rcS

start() {
	touch /ramdisk/flash/passwd

	echo "root:\$1\$\$CoERg7ynjYLsj2j4glJ34.:0:0:root:/root:/bin/sh" > /ramdisk/flash/passwd
	echo "ftp:\$1\$\$CoERg7ynjYLsj2j4glJ34.:100:100:ftp:/tmp:/dev/null" >> /ramdisk/flash/passwd
	echo "admin:\$1\$\$CoERg7ynjYLsj2j4glJ34.:0:0:root:/root:/bin/ash" >> /ramdisk/flash/passwd
	echo "support_user:\$1\$iuIXI3we\$egJmTeRFF8WkNrgPVxt18.:0:0:root:/tmp/support_user:/bin/sh" >> /ramdisk/flash/passwd
	echo "ugw:\$1\$VwzDmRae\$nsCk5rA8MuO3afxCvrYLH0:201:0:Linux User:/mnt/usb:/bin/sh" >> /ramdisk/flash/passwd
}
