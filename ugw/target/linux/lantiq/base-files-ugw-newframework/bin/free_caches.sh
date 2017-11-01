#!/bin/sh

while true
do
	sleep 3600
	sync
	echo 3 > /proc/sys/vm/drop_caches

done

