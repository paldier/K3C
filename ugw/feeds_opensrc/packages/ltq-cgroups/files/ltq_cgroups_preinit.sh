#!/bin/sh

# Add Process ID 1 to cgroup 'default' group
[ -d /sys/fs/cgroup ] && {
	# Assuming '/sys/fs/cgroup' already mounted by preinit/procd
	mkdir -p /sys/fs/cgroup/cpu/ /sys/fs/cgroup/memory/
	cd /sys/fs/cgroup/cpu/ && {
		mkdir -p high low default
		echo $$ > default/tasks
		echo 1 > default/tasks
	}
	cd /sys/fs/cgroup/memory/ && {
		mkdir -p high low default
		cd - > /dev/null
	}
}

