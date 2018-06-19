#!/bin/sh

#find some small space in ramfs, in future it should be taken form exported variable.
tempfile="/tmp/logger$1.zip"
md5file="/tmp/logger$1.md5"

#create a temporary file (the name should be same as the final logfile of pad application)
#dd if=/dev/zero of=$tempdir$tempfile bs=1000 count=1 2>/dev/null

# for each ip address found on arp table (will correspond to LAN hosts)
for i in $(cat /proc/net/arp |cut -d " " -f 1); do
	ping -W2 -c 1 -q  $i 2>/dev/null

	if [ $? = "0" ]; then
		echo "$i responded to ping, Now trying tftp transfer..!!"
#		cd $tempdir
		tftp -p -l $tempfile -r logger$1.zip $i
		tftp -p -l $md5file -r logger$1.md5 $i
		
		if [ $? = "0" ]; then
			echo "Transfer Success .!! to tftp Server running @ $i"
		else
			echo "Transfer Failed..!! Since, tftp Server not running @ $i"
		fi
	fi
done

#if [ -f $tempdir$tempfile ]; then
#	rm $tempdir$tempfile
#fi



