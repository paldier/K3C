#!/bin/sh
	if [ -e /etc/patches.list ];then
		cat /etc/patches.list
	else
		echo "No patches are applied"
	fi
