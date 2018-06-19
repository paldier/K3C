#!/bin/sh

MIN_CONNTRACK=999
SM_DEFAULT_CONNTRACK="/tmp/.default_ip_conntrack_max"

CONFFILE=/opt/lantiq/etc/sess_mgmt.conf

source $CONFFILE

if  [ ! -f  $SM_DEFAULT_CONNTRACK ]; then
    cat /proc/sys/net/ipv4/netfilter/ip_conntrack_max > $SM_DEFAULT_CONNTRACK
fi

usage() {
	echo "Usage: ./sess_mgmt.sh"
	echo "	-e/--enable - used to enable session management"
	echo "	-d/--disable - used to disable session mangement"
	echo " Parameters are read from conf file $CONFFILE"
}

sess_enable() {
		if [ -z "$sessmgmt_ms" ]; then
			echo "sessmgmt_ms is not defined(max connection track count)"
			exit
		fi
		if [ $sessmgmt_ms -gt $MIN_CONNTRACK ]; then
        	echo $sessmgmt_ms > /proc/sys/net/ipv4/netfilter/ip_conntrack_max
    	fi
    	if [ $sessmgmt_ms -eq 0 ]; then
        	dval=$(cat $SM_DEFAULT_CONNTRACK)
        	if [ $dval -gt $MIN_CONNTRACK 2>/dev/null ]; then
            	echo ${dval} > /proc/sys/net/ipv4/netfilter/ip_conntrack_max
        	else
            	echo $MIN_CONNTRACK > /proc/sys/net/ipv4/netfilter/ip_conntrack_max
        	fi
    	fi

		if [ -n "$sessmgmt_dpm" ]; then 
			[ $sessmgmt_dpm -lt 100 ] && { 
				echo "sessmgmt_dpm cannot be less than 100"
				exit
			}
		else
			sessmgmt_dpm=0
		fi

		if [ -n "$sessmgmt_lpm" ]; then
			[ $sessmgmt_lpm -lt 100 ] && { 
				echo "sessmgmt_lpm cannot be less than 100"
				exit
			}
		else
			sessmgmt_lpm=0
		fi

		if [ -n "$sessmgmt_lpt" ]; then
			([ $sessmgmt_lpt -lt 0 ] || [ $sessmgmt_lpt -gt 7 ]) && {
				echo "sessmgmt_lpt should range in 0-7"
				exit 
			}
		else
			echo "sessmgmt_lpt cannot be null on enable" 
			exit
		fi
 
		if [ -n "$sessmgmt_dpt" ]; then
			([ $sessmgmt_dpt -lt 0 ] || [ $sessmgmt_dpt -gt 7 ]) && { 
				echo "sessmgmt_dpt should range in 0-7"
				exit
			}
		else
			echo "sessmgmt_dpt cannot be null on enable" 
			exit
		fi

		if [ -n "$sessmgmt_ldr" ]; then
			([ $sessmgmt_ldr -lt 0 ] || [ $sessmgmt_ldr -gt 1048576 ]) && {
				echo "Session rates can't be greater than 1Gbps"
				exit
			}
		else
			sessmgmt_ldr=0
		fi
 
		if [ -n "$sessmgmt_ddr" ]; then
			([ $sessmgmt_ddr -lt 0 ] || [ $sessmgmt_ddr -gt 1048576 ]) && {
				echo "Session rates can't be greater than 1Gbps"
				exit
			}
		else
			sessmgmt_ddr=0
		fi

		echo $sessmgmt_dpm > /proc/sys/net/netfilter/nf_conntrack_default_prio_max
        echo $sessmgmt_dpt > /proc/sys/net/netfilter/nf_conntrack_default_prio_thresh
        echo $sessmgmt_lpm > /proc/sys/net/netfilter/nf_conntrack_low_prio_max
        echo $sessmgmt_lpt > /proc/sys/net/netfilter/nf_conntrack_low_prio_thresh
        echo `expr $sessmgmt_ldr \* 1024 ` > /proc/sys/net/netfilter/nf_conntrack_low_prio_data_rate
        echo `expr $sessmgmt_ddr \* 1024 ` > /proc/sys/net/netfilter/nf_conntrack_default_prio_data_rate
        echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_initial_offset
        echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_steady_offset
        echo 5 > /proc/sys/net/netfilter/nf_conntrack_sessionmgmt_add_time
		echo 1 > /proc/sys/net/netfilter/nf_conntrack_session_limit_enable
		/opt/lantiq/etc/rc.d/flow-aware-qos.sh start auto
}

sess_disable() {
		echo 0 > /proc/sys/net/netfilter/nf_conntrack_session_limit_enable
        echo 0 > /proc/sys/net/netfilter/nf_conntrack_default_prio_max
        echo 0 > /proc/sys/net/netfilter/nf_conntrack_default_prio_thresh
        echo 0 > /proc/sys/net/netfilter/nf_conntrack_default_prio_data_rate
        echo 0 > /proc/sys/net/netfilter/nf_conntrack_low_prio_max
        echo 0 > /proc/sys/net/netfilter/nf_conntrack_low_prio_thresh
        echo 0 > /proc/sys/net/netfilter/nf_conntrack_low_prio_data_rate
        echo 5 > /proc/sys/net/netfilter/nf_conntrack_sessionmgmt_add_time
		/opt/lantiq/etc/rc.d/flow-aware-qos.sh stop auto
}

	case $1 in
		-e | --enable )
			sess_enable
			;;
		-d | --disable )
			sess_disable
			;;
		-h | --help )           
			usage
			exit
			;;
		* )
			usage
			exit 1
	esac
