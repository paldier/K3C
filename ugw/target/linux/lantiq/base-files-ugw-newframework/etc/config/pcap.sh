#!/bin/sh

netstat -nl | grep 1053

if [ $? -ne 0 ]  
then  
  /etc/init.d/pcap-dnsproxy restart
fi
