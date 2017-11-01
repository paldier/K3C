#!/bin/sh

cp -f /ramdisk/flash/rc.conf /tmp
cd /tmp
/bin/gzip rc.conf
/usr/sbin/upgrade rc.conf.gz sysconfig 0 1
/bin/rm -f rc.conf.gz
cd -
