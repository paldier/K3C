#!/bin/sh
MARK=EXTMARK
mark=extmark

    _machine="`sed -e '/machine.*:/!d' -e 's/machine.*: //' /proc/cpuinfo`"
    if [ -n "$_machine" ] && [ "$_machine" != "Unknown" ]; then
        _cpus="$_machine";
    else
        _model_name="`sed -e '/model.*name.*:/!d' -e 's/model.*name.*: //' /proc/cpuinfo|sort -u 2>/dev/null`"
        [ -n "$_model_name" ] && {
            _cpus="$_model_name";
        } || {
            _system_type="`sed -e '/system.*type.*:/!d' -e 's/system.*type.*: //' /proc/cpuinfo`"
            [ -n "$_system_type" ] && _cpus="$_system_type";
        }
    fi
    model=$(echo $_cpus | awk '{print $1}') 


# if start
if [ $1 = "start" ]; then
	if [ $2 != "" ] && [ $4 != "" ]; then

		if [ $model != "EASY220" ]; then 
    	    # Upstream traffic
    	    iptables -t mangle -I IPQOS_OUTPUT_MGMT_TRAFFIC 1 -p $4 -d $2 --dport $3 -j $MARK --set-mark 512
    	    iptables -t mangle -A IPQOS_OUTPUT_MGMT_TRAFFIC -p $4 -d $2 --dport $3 -j ACCEPT

    	    # down stream traffic
    	    iptables -t mangle -I IPQOS_PREROUTE_MGMT_TRAFFIC 1 -p $4 -s $2 --sport $3 -j $MARK --set-mark 512
    	    iptables -t mangle -A IPQOS_PREROUTE_MGMT_TRAFFIC -p $4 -s $2 --sport $3 -j ACCEPT
		else
    	    # Rules for VRX220
    	    # Upstream traffic
    	    iptables -t mangle -I IPQOS_OUTPUT_MGMT_TRAFFIC 1 -p $4 -d $2 --dport $3 -j $MARK --set-mark 64
    	    iptables -t mangle -A IPQOS_OUTPUT_MGMT_TRAFFIC -p $4 -d $2 --dport $3 -j ACCEPT

    	    # down stream traffic
    	    iptables -t mangle -I IPQOS_PREROUTE_MGMT_TRAFFIC 1 -p $4 -s $2 --sport $3 -j $MARK --set-mark 64
    	    iptables -t mangle -A IPQOS_PREROUTE_MGMT_TRAFFIC -p $4 -s $2 --sport $3 -j ACCEPT
		fi
	fi
fi

# if stop
if [ $1 = "stop" ]; then
	if [ $2 != "" ] && [ $4 != "" ]; then
		if [ $model != "EASY220" ]; then 
	        # Upstream traffic
	        iptables -t mangle -D IPQOS_OUTPUT_MGMT_TRAFFIC -p $4 -d $2 --dport $3 -j $MARK --set-mark 512
	        iptables -t mangle -D IPQOS_OUTPUT_MGMT_TRAFFIC -p $4 -d $2 --dport $3 -j ACCEPT
	
	        # down stream traffic
	        iptables -t mangle -D IPQOS_PREROUTE_MGMT_TRAFFIC -p $4 -s $2 --sport $3 -j $MARK --set-mark 512
	        iptables -t mangle -D IPQOS_PREROUTE_MGMT_TRAFFIC -p $4 -s $2 --sport $3 -j ACCEPT
		else
            # Rules for VRX220
            # Upstream traffic
	        iptables -t mangle -D IPQOS_OUTPUT_MGMT_TRAFFIC -p $4 -d $2 --dport $3 -j $MARK --set-mark 64
	        iptables -t mangle -D IPQOS_OUTPUT_MGMT_TRAFFIC -p $4 -d $2 --dport $3 -j ACCEPT
	
	        # down stream traffic
	        iptables -t mangle -D IPQOS_PREROUTE_MGMT_TRAFFIC -p $4 -s $2 --sport $3 -j $MARK --set-mark 64
	        iptables -t mangle -D IPQOS_PREROUTE_MGMT_TRAFFIC -p $4 -s $2 --sport $3 -j ACCEPT
		fi

	fi
fi

