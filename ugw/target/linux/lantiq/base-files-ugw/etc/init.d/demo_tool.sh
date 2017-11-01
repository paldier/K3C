#!/bin/sh /etc/rc.common
# Copyright (c) 2013 Lantiq Deutschland GMBH
# Start-up Script for Demo tool

START=71

start() {
	if [ "$CONFIG_FEATURE_DEMO_TOOL" = "1" ]; then
		echo "Starting Demo Tool"
	                /sbin/insmod /lib/modules/*/pecostat_noIRQ.ko
                        /sbin/insmod /lib/modules/*/demo_server.ko
        
        	file="/proc/driver/tapi/"
        	if [ -e "$file" ];then
                	echo ""\""Voip"\"":"\""up"\"","> "/proc/proc_entry_peco"
        	else
                	echo ""\""Voip"\"":"""down"""," >"/proc/proc_entry_peco"
        	fi

        	check=`cat /proc/net/wireless | awk 'END {print NR}'`
        	if [ "$check" -eq "2" ] ;then

                	echo ""\""wlan0"\"": { "\""status"\"":"\""down"\"","\""Freq"\"":"\""0"\""}," > "/proc/proc_entry_peco"
        	fi

        	check1=`cat /proc/net/wireless | awk '{if (NR>2) val=$1; print val}'`

        	fl=`echo $check1 | awk '{print NF}'`
        	if [ "$fl" -gt "1" ];then
                	cnt=`expr $fl - 1`

        	else
                	cnt=0
        	fi
        	i=0
        	j=$cnt

        	while [ $i -le $j ]
        	do
        	arr=$(echo $check1 | awk -v i="$i" '{print $(NF-i)}'| cut -d":" -f1)

        	a1="wlan0"
        	a2="ath0"
        	if [ "$arr" = "$a1" ];then
                	req=`/usr/sbin/iwconfig $arr  |sed -n 2p|awk '{split($2,frr,"=")}END {print frr[2]}'`
                	echo "\""$arr"\""": {"\""Freq"\"":""\""$req"\"""," > "/proc/proc_entry_peco"
                	req1=$(/usr/sbin/iwconfig $arr |sed -n 3p|awk '{split($0,brr,"=")} END {print brr[2]}' | awk '{print $1}')
                	req2="off"
                	if [ $req1 = $req2 ];then
				
                        	echo ""\""status"\"":"\""down"\""}," >"/proc/proc_entry_peco"
                	else
				
                        	echo ""\""status"\"":"\""up"\""},"  >"/proc/proc_entry_peco"
                	fi
        	fi
        	if [ "$arr" = "$a2" ];then
                	req=`/usr/sbin/iwconfig $arr |sed -n 2p|awk '{split($2,frr,":")}END {print frr[2]}'`
                	echo "\""wlan1"\""":{"\""Freq"\"":""\""$req"\"""," > "/proc/proc_entry_peco"
                	req2="0"
                	req1=`/usr/sbin/iwconfig $arr | sed -n 3p| awk '{split($2,brr,":")}END{print brr[2]}'`
                	if [ $req1 == $req2 ];then
				
                        	echo ""\""status"\"":"\""down"\""}," >"/proc/proc_entry_peco"
                	else
				
                        	echo ""\""status"\"":"\""up"\""},"  >"/proc/proc_entry_peco"
                	fi
        	fi
        	i=$((i + 1))
        	done
		line_no=$(cat /proc/driver/vmmc/power | awk 'END {print NR}')
		echo $line_no
		if [ "$line_no" -lt "7" ];then
			
			echo ""\""channel_Active"\"":"\""0"\""" >"/proc/proc_entry_peco"
		fi
	
	fi
}

                  
	


