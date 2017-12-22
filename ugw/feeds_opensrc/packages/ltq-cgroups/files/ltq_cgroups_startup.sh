#!/bin/sh /etc/rc.common
# Copyright (C) 2007 OpenWrt.org
# Cgroups Script to start the daemons at bootup
# config: /etc/config.conf

START=08

start() {
        echo Cgroups related daemons started
        /usr/bin/cgroup_notifier &
	sleep 1 # Before creation of FIFO, FIFO open was getting called in cgroup_daemon.
        /usr/bin/cgroup_daemon &
}

stop() {
killall cgroup_notifier
killall cgroup_daemon
}

