#!/bin/sh /etc/rc.common

START=01

#
# This routine configures memory tuning parameters
#
tune_memory_parameters()
{
	if [ -n "$CONFIG_TARGET_LANTIQ_XRX500" ]; then
		sysctl -w vm.min_free_kbytes="8192"
	elif [ -n "$CONFIG_TARGET_LANTIQ_XRX200" ]; then
		sysctl -w vm.min_free_kbytes="1024"
	else
		sysctl -w vm.min_free_kbytes="4096"
	fi

	sysctl -w vm.lowmem_reserve_ratio="250"
	sysctl -w vm.dirty_background_ratio="2"
	sysctl -w vm.dirty_writeback_centisecs="250"
	sysctl -w vm.dirty_ratio="10"
	sysctl -w vm.max_map_count="16384"

	#
	# Following parameters ensure that user space applications are not allowed to
	# consume more than 95% of the system RAM and when system is heavily used &
	# under low memory, instead of OOM, user space application requesting for more
	# momory will suffer.
	#
	sysctl -w vm.scan_unevictable_pages="1"
	sysctl -w vm.overcommit_memory="0"
}

#
# This routine configures the network parameters
#
tune_network_parameters()
{
	sysctl -w net.ipv4.route.max_size="4096"

	if [ -f "/proc/sys/net/ipv4/tcp_min_tso_segs" ]; then
		sysctl -w net.ipv4.tcp_min_tso_segs="44"
	fi

	# Optimize Routing & Conntrack cache - needs individual tuning based on model
	if [ -n "$CONFIG_FEATURE_LOW_FOOTPRINT" ]; then
		sysctl -w net.netfilter.nf_conntrack_max="512"
		sysctl -w net.netfilter.nf_conntrack_expect_max="50"
	else
		if [ -n "$CONFIG_TARGET_X86_PUMA" ]; then
			sysctl -w net.netfilter.nf_conntrack_max="25000"
		else
			sysctl -w net.netfilter.nf_conntrack_max="4096"
		fi
		sysctl -w net.netfilter.nf_conntrack_expect_max="100"
	fi

	if [ -n "$CONFIG_TARGET_LANTIQ_XRX500" ]; then
		sysctl -w net.netfilter.nf_conntrack_max="6144"
		sysctl -w net.netfilter.nf_conntrack_expect_max="100"
	fi
	
	if [ -n "$CONFIG_TARGET_LANTIQ_XRX200" ]; then
		sysctl -w net.netfilter.nf_conntrack_tcp_be_liberal="1"
	fi
}

set_local_session_learning_for_lro()
{
	# -m comment --comment "local session learning for TCP litepath and LRO"

	local iRule="INPUT -p tcp -m state --state ESTABLISHED -j EXTMARK --set-mark 0x80000000/0x80000000"
	iptables -t mangle -C $iRule 2>/dev/null || {
		iptables -t mangle -I $iRule
	}
}

tune_watchdog_timer()
{
	if [ -n "$CONFIG_TARGET_LANTIQ_XRX200" ]; then
		ubus call system watchdog '{ "timeout": 180 }';
	else
		ubus call system watchdog '{ "timeout": 90 }';
	fi
}
start()
{
	tune_memory_parameters
	tune_network_parameters
	set_local_session_learning_for_lro
	tune_watchdog_timer
}

