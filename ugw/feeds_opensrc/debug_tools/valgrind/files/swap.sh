#!/bin/sh

set -e
#dd if=/dev/zero of=/swapfile bs=1M count=100

case "$1" in
start)
	echo -n "Starting swap: "
	mkswap /swapfile
	losetup /dev/loop0 /swapfile
	mkswap /dev/loop0
	swapon /dev/loop0
	echo "done"
	;;
stop)
	echo -n "Stopping swap: "
	swapoff /dev/loop0
	losetup -d /dev/loop0
	echo "done"
	;;
restart)
	swapoff /dev/loop0
	swapon /dev/loop0
	;;
*)

echo "Usage: swap { start | stop | restart }" >&2 exit 1 ;; esac

exit 0

