#!/bin/sh /etc/rc.common

#START=17

start() {
	if [ "1$CONFIG_FEATURE_PPA_SUPPORT" = "11" ]; then
		if [ "1$CONFIG_FEATURE_SESSION_LIMIT" = "11" ]; then
			/sbin/ppacmd init -n 10 -j ${sessmgmt_ncol} -k ${sessmgmt_ncol} 2> /dev/null
		else
			/sbin/ppacmd init -n 10 2> /dev/null
		fi
		# enable wan / lan ingress
		/sbin/ppacmd control --enable-lan --enable-wan 2> /dev/null

		# set WAN vlan range 3 to 4095		
		/sbin/ppacmd addvlanrange -s 3 -e 0xfff 2> /dev/null

		# In PPE firmware is A4 or D4 and ppe is loaded as module then reinitialize TURBO MII mode since it is reset during module load.
                if [ "1$CONFIG_PACKAGE_KMOD_LTQCPE_PPA_A4_MOD" = "11" ]; then
                        echo w 0xbe191808 0000096E > /proc/eth/mem
                fi

                if [ "1$CONFIG_PACKAGE_KMOD_LTQCPE_PPA_D4_MOD" = "11" ]; then
                        echo w 0xbe191808 000003F6 > /proc/eth/mem
                fi

		#For PPA, the ack/sync/fin packet go through the MIPS and normal data packet go through PP32 firmware.
		#The order of packets could be broken due to different processing time.
		#The flag nf_conntrack_tcp_be_liberal gives less restriction on packets and 
		# if the packet is in window it's accepted, do not care about the order

		echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal
		#if PPA is enabled, enable hardware based QoS to be used later
		/usr/sbin/status_oper SET "IPQoS_Config" "ppe_ipqos" "1"
	fi
}

stop() {

	if [ "1$CONFIG_FEATURE_PPA_SUPPORT" = "11" ]; then
		/sbin/ppacmd exit 2> /dev/null

		/sbin/ppacmd control --disable-lan --disable-wan 2> /dev/null

		# reset to defaul WAN vlan range 0 to 16		
		/sbin/ppacmd addvlanrange -s 0 -e 10 2> /dev/null

		#if PPA is stopped, disable hardware based QoS to be used later
		/usr/sbin/status_oper SET "IPQoS_Config" "ppe_ipqos" "0"
	fi
}
