#!/bin/sh /etc/rc.common
# Cgroups Script to start the daemons at bootup
# config: /etc/config.conf

START=08
USE_PROCD=1

start_service_notifier()
{
	procd_open_instance
	procd_set_param command cgroup_notifier
	procd_set_param respawn
	procd_close_instance
}

start_service_daemon()
{
	procd_open_instance
	procd_set_param command cgroup_daemon
	procd_set_param file /etc/cgroups.conf
	procd_set_param respawn
	procd_close_instance
}

start_service()
{
	start_service_notifier
	start_service_daemon
}

