#!/bin/sh /etc/rc.common
#START=48
START=21

	if [ ! "$ENVLOADED" ]; then
		if [ -r /etc/rc.conf ]; then
			. /etc/rc.conf 2> /dev/null
			ENVLOADED="1"
		fi
	fi

	if [ ! "$CONFIGLOADED" ]; then
	if [ -r /etc/rc.d/config.sh ]; then
		. /etc/rc.d/config.sh 2>/dev/null
#		. /etc/rc.d/model_config.sh 2>/dev/null
		CONFIGLOADED="1"
	fi
	fi

#        if [ -r /etc/rc.d/model_config.sh ]; then
#                . /etc/rc.d/model_config.sh 2>/dev/null
#        fi

check_mac ()
{
	local old_mac=$(upgrade mac_get 0 2>/dev/null);
	[ -n "$CONFIG_UBOOT_CONFIG_ETHERNET_ADDRESS" -a -n "$old_mac" ] && \
	  [ "$old_mac" = "$CONFIG_UBOOT_CONFIG_ETHERNET_ADDRESS" ] && {
		local i=0;
		while [ $i -lt 5 ]; do
			echo -en "\033[J"; usleep 150000;
			echo -en "#######################################################\n";
			echo -en "#     DEVICE CONFIGURED WITH DEFAULT MAC ADDRESS!!    #\n";
			echo -en "# This may conflict with other devices. Please change #\n";
			echo -en "#     the MAC address for un-interrupted services.    #\n";
			echo -en "#######################################################\n";
			echo -en "\033[5A\033G"; usleep 300000;
			let i++
		done; echo -en "\n\n\n\n\n";
	} || true
}

start() {

	check_mac;

## Handle IPv6
# Merge from S49ipv6.sh

	if [ "$CONFIG_PACKAGE_KMOD_IPV6" = "1"  -a  "$ipv6_status" = "1" ]; then
		echo 0 > /proc/sys/net/ipv6/conf/all/disable_ipv6
		echo 1 > /proc/sys/net/ipv6/conf/all/forwarding
	elif [ "$CONFIG_PACKAGE_KMOD_IPV6" = "1"  -a  "$ipv6_status" = "0" ]; then
		 echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6
		 echo 1 > /proc/sys/net/ipv6/conf/all/forwarding
	elif [ "$CONFIG_FEATURE_IPv6" = "1" ]; then
		echo 1 > /proc/sys/net/ipv6/conf/all/forwarding
	fi

	if [ -n "$CONFIG_TARGET_LTQCPE_PLATFORM_AR9_VB" ]; then
		echo 0 > /proc/sys/net/ipv6/conf/all/forwarding
	fi

### Bringup SYSLOG Daemon
# Merge from S41syslogd.sh
	# Start System log
	if [ -f /sbin/syslogd ]; then
		killall syslogd 2>/dev/null
#		echo "Bringing up syslog"
		if [ "$system_log_mode" = "1" -o "$system_log_mode" = "2" ]; then
			if [ -n "$system_log_IP" -a "$system_log_IP" != "0.0.0.0" ]; then
				if [ -n "$system_log_port" -a "$system_log_port" != "0" ]; then
					if [ "$system_log_mode" = "2" ]; then
						/sbin/syslogd -L -s	$CONFIG_FEATURE_SYSTEM_LOG_BUFFER_SIZE -b $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_COUNT -R $system_log_IP:$system_log_port -l $system_log_log_level
					else
						/sbin/syslogd -s $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_SIZE -b $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_COUNT -R $system_log_IP:$system_log_port -l $system_log_log_level
					fi
				else
					if [ "$system_log_mode" = "2" ]; then
						/sbin/syslogd -L -s $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_SIZE -b $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_COUNT 0 -R $system_log_IP -l $system_log_log_level
					else
						/sbin/syslogd -s $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_SIZE -b $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_COUNT 0 -R $system_log_IP -l $system_log_log_level
					fi
				fi
			fi
		else
			/sbin/syslogd -s $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_SIZE -b $CONFIG_FEATURE_SYSTEM_LOG_BUFFER_COUNT -l $system_log_log_level
		fi
	else
		if [ -f /usr/sbin/syslog-ng ]; then
			killall -9 syslog-ng 2>/dev/null
			ifx_event_util "SYSLOG_NG" "START"
			if [ -f /tmp/syslog-ng.conf ]; then
				syslog-ng -f /tmp/syslog-ng.conf
			else
				syslog-ng -f /etc/syslog-ng/syslog-ng.conf
			fi
		fi
	fi

### Setup HostName for Loopback Interface ###
	# Setup lo Interface Addresses
	/sbin/ifconfig lo 127.0.0.1 netmask 255.0.0.0
	if [ -n "$CONFIG_TARGET_LTQCPE_PLATFORM_AR9_VB" ]; then
		echo 0 > /proc/sys/net/ipv4/ip_forward
	else
		echo 1 > /proc/sys/net/ipv4/ip_forward
	fi
	
	# Setup Hostname
	echo "127.0.0.1 localhost.localdomain localhost" > /etc/hosts
	if [ -f /bin/hostname ]; then
		/bin/hostname $hostname
	fi
	
	i=0
	while [ $i -lt $lan_main_Count ]
	do
		eval ip='$lan_main_'$i'_ipAddr'
		shorthost=${hostname%%.*}
		echo "$ip ${hostname} $shorthost" >> /etc/hosts
		let i++
	done

	if [ "$ipv6_status" = "1" ]; then
		eval hn=`uname -n`'6'
		i=0
		while [ $i -lt $lan_main_Count ]
		do
			eval ip='$lan_main_ipv6_'$i'_ip6Addr'
			echo "$ip $hn.$lan_dhcpv6_dName $hn" >> /etc/hosts
			let i++
		done
	fi

#Set Active Wan Pvc's parameter in /tmp/system/status for LCP Prioritization
	/usr/sbin/status_oper SET "LCP_Wan_Info" "active_wan_pvc" "0"
### Initialize the PPA hooks for use in LAN Start ###
### Merge from PPA.sh script in S17PPA.sh ###

	if [ "1$CONFIG_FEATURE_PPA_SUPPORT" = "11" ]; then
		# initialize PPA hooks
		if [ "1$CONFIG_FEATURE_SESSION_LIMIT" = "11" ]; then
			/sbin/ppacmd init -n 10 -j ${sessmgmt_ncol} -k ${sessmgmt_ncol} 2> /dev/null
		else
			/sbin/ppacmd init -n 10 2> /dev/null
		fi
		
		/usr/sbin/ppasessmgmt -s -i 30 -n 128 -w 128

		# enable wan / lan ingress
		/sbin/ppacmd control --enable-lan --enable-wan 2> /dev/null

		# set WAN vlan range 3 to 4095		
		/sbin/ppacmd addvlanrange -s 3 -e 0xfff 2> /dev/null

		# In case of A4 or D4 and if ppe is loaded as module, in case of danube/tp-ve, 
		# then reinitialize TURBO MII mode since it is reset during module load.
		platform=${CONFIG_IFX_MODEL_NAME%%_*}
		if [ "$platform" = "DANUBE" -o "$platform" = "TP-VE" ]; then
                	if [ "1$CONFIG_PACKAGE_KMOD_LTQCPE_PPA_A4_MOD" = "11" ]; then
	                        echo w 0xbe191808 0000096E > /proc/eth/mem
        	        fi

                	if [ "1$CONFIG_PACKAGE_KMOD_LTQCPE_PPA_D4_MOD" = "11" ]; then
	                        echo w 0xbe191808 000003F6 > /proc/eth/mem
	                fi
		fi

		#For PPA, the ack/sync/fin packet go through the MIPS and normal data packet go through PP32 firmware.
		#The order of packets could be broken due to different processing time.
		#The flag nf_conntrack_tcp_be_liberal gives less restriction on packets and 
		# if the packet is in window it's accepted, do not care about the order

		echo 1 > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal
		if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" != "1" ]; then
			#if PPA is enabled, enable hardware based QoS to be used later
			/usr/sbin/status_oper SET "IPQoS_Config" "ppe_ipqos" "1"
			. /etc/init.d/ipqos_qprio_cfg.sh
		fi
	fi

	#devm	
	if [ "$CONFIG_FEATURE_NAPT" = "1" ]; then
		if [ -f /usr/sbin/naptcfg ]; then
			/usr/sbin/naptcfg --FWinit > /dev/null
			/usr/sbin/naptcfg --NAPTinit > /dev/null
			/usr/sbin/naptcfg --MACFilterinit > /dev/null
			/usr/sbin/naptcfg --PacketFilterinit > /dev/null
			/usr/sbin/naptcfg --ServicesACLinit > /dev/null
		fi
	fi

	BR_MAC_ADDR="`/usr/sbin/upgrade mac_get 0`"
		
	echo "BR_MAC_ADDR=" $BR_MAC_ADDR
	
	i=0
	while [ $i -lt $lan_main_Count ]
		do
			eval iface='$lan_main_'$i'_interface'
			eval ip='$'lan_main_${i}_ipAddr
			eval netmask='$'lan_main_${i}_netmask
			if [ "$iface" = "br0" ]; then
				/usr/sbin/brctl addbr $iface
				/sbin/ifconfig $iface hw ether $BR_MAC_ADDR up
				/usr/sbin/brctl setfd $iface 1
				[ `mount|grep -q nfs;echo $?` -eq  0 ] && /sbin/ifconfig $iface $ip netmask $netmask up
				if [ -n "$CONFIG_TARGET_LTQCPE_PLATFORM_AR9_VB" ]; then
                	                  /usr/sbin/brctl addif $iface eth1
				elif [ -n "$CONFIG_TARGET_LANTIQ_XRX500" ]; then
					#in anywan board port number 2,3,4,5 are used as lan ports
					/sbin/ifconfig eth0_1 hw ether $BR_MAC_ADDR 
					/usr/sbin/brctl addif $iface eth0_1
					local LAN_MAC_ADDR=`echo $BR_MAC_ADDR | /usr/sbin/next_macaddr -3`
					/sbin/ifconfig eth0_2 hw ether $LAN_MAC_ADDR
					/usr/sbin/brctl addif $iface eth0_2
					local LAN_MAC_ADDR=`echo $BR_MAC_ADDR | /usr/sbin/next_macaddr -2`
					/sbin/ifconfig eth0_3 hw ether $LAN_MAC_ADDR
					/usr/sbin/brctl addif $iface eth0_3 
					LAN_MAC_ADDR=`echo $BR_MAC_ADDR | /usr/sbin/next_macaddr -1`
                                        /sbin/ifconfig eth0_4 hw ether $LAN_MAC_ADDR
					/usr/sbin/brctl addif $iface eth0_4 
				else
					/usr/sbin/brctl addif $iface eth0
                        	fi
				/usr/sbin/brctl stp $iface off
			fi
			let i++
		done
	#806131:<IFTW-leon> Let board accessing by eth0 before ppa ready
	if [ -n "$CONFIG_TARGET_LANTIQ_XRX500" ]; then
		/sbin/ifconfig eth0_1 0.0.0.0 up
		/sbin/ifconfig eth0_2 0.0.0.0 up
		/sbin/ifconfig eth0_3 0.0.0.0 up
		/sbin/ifconfig eth0_4 0.0.0.0 up
	else
		/sbin/ifconfig eth0 0.0.0.0 up
	fi
  if [ "$CONFIG_FEATURE_MINI_JUMBO_FRAMES" == "1" ] ; then
    # When mini jumbo is enabled MTU is set to 1586. Reset NTU to 1500 on eth0
    # Note: remove mini jumbo changes if driver/PPA handles default MTU value
    #       to 1500 on LAN interface
    /sbin/ifconfig eth0 mtu 1500
  fi


	/etc/rc.d/rc.bringup_lan start

	# Start DevM
	/etc/rc.d/rc.bringup_services start_devm

	if [ "$CONFIG_FEATURE_IFX_A4" = "1" ]; then
		#806131:<IFTW-leon> Let board accessing by eth0 before ppa ready start
		/usr/sbin/brctl delif br0 eth0
		/sbin/ifconfig br0 0.0.0.0 down
		/usr/sbin/status_oper SET Lan1_IF_Info STATUS DOWN
			
		/sbin/ifconfig eth0 $lan_main_0_ipAddr netmask $lan_main_0_netmask up
		/usr/sbin/status_oper SET Lan1_IF_Info STATUS "UP" IP "$lan_main_0_ipAddr" MASK "$lan_main_0_netmask"
		/usr/sbin/naptcfg --ADDLAN eth0
		/usr/sbin/naptcfg --Servicesinit
		#806131:<IFTW-leon> Let board accessing by eth0 before  ppa ready end
	fi

	if [ "$CONFIG_FEATURE_NAPT" = "1" ]; then
		/etc/rc.d/rc.firewall start
		/usr/sbin/naptcfg --VSinit > /dev/null
		/usr/sbin/naptcfg --PMinit > /dev/null
		/usr/sbin/naptcfg --DMZinit > /dev/null
		/usr/sbin/naptcfg --VS 1 > /dev/null
		/usr/sbin/naptcfg --PM 1 > /dev/null
			
		if [ "$firewall_dmz_enable" = "1" ]; then
			/usr/sbin/naptcfg --DMZ 1 > /dev/null
		fi
		if [ "$ipnat_enable" = "1" ]; then
			/usr/sbin/naptcfg --NAPT 1 > /dev/null
			if [ -n "$CONFIG_TARGET_LANTIQ_XRX500" ]; then
				[ `mount|grep -q nfs;echo $?` -eq  0 ] && \
					iptables -t nat -A IFX_NAPT_PREROUTING_LAN --in-interface eth0_1 --jump ACCEPT && \
					iptables -t nat -A IFX_NAPT_PREROUTING_LAN --in-interface eth0_2 --jump ACCEPT && \
					iptables -t nat -A IFX_NAPT_PREROUTING_LAN --in-interface eth0_3 --jump ACCEPT && \
					iptables -t nat -A IFX_NAPT_PREROUTING_LAN --in-interface eth0_4 --jump ACCEPT
			else
				[ `mount|grep -q nfs;echo $?` -eq  0 ] && \
					iptables -t nat -A IFX_NAPT_PREROUTING_LAN --in-interface eth0 --jump ACCEPT
			fi
		else
			/usr/sbin/naptcfg --NAPT 0 > /dev/null
		fi
	fi

	# Run some deamons likes http, telnetd
	/etc/rc.d/rc.bringup_services except_devm

	#
	# Setup QOS
	#
	if  [ "$CONFIG_FEATURE_QOS" = "1" ]; then
			/etc/rc.d/init.d/qos init
	fi

	if [ -z "$CONFIG_LTQ_BRIDGE_MODEM" ]; then
	# accept everything on loopback interface
	/usr/sbin/iptables -I INPUT -i lo -j ACCEPT
	/usr/sbin/iptables -I OUTPUT -o lo -j ACCEPT
	fi

	if [ -n "$CONFIG_TARGET_LANTIQ_XRX500" ]; then
		echo 1 > /proc/sys/net/ipv4/conf/all/arp_filter
	fi
}
