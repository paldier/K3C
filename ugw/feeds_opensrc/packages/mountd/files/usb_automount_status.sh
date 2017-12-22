#!/bin/sh
# mountd Addon Script to display Disc statistics in WebPage
sync_disc_info ()
{
	ls /mnt/usb/*/*/* >/dev/null 2>/dev/null
	mountd_state=`cat /var/state/mountd`;
	df_state=`df`;
	disk_list=`echo "$mountd_state" | grep "\.disc" | cut -d. -f2`
	count_disk_list=`echo $disk_list | wc -w`
}

print_disk_part_info ()
{
	j=0;
	while read infoline; do
	  info_dev=`echo $infoline|awk '{ print $1 }'|cut -d'/' -f3`
	  info_dev_no=`echo $info_dev|sed 's/[a-z]//g'`
	  info_mounted_on="/$2/`echo "$mountd_state"|grep -w "$2\.name$info_dev_no"|cut -d= -f2`"
	  info_fs="`echo "$mountd_state"|grep -w "$2\.fs$info_dev_no"|cut -d= -f2`"
	  info_disk_stats="`echo $infoline|awk '{ print $2" "$3" "$4" "$5 }'`"
	  info_str="USB_$1_DISK_INFO_$j $info_mounted_on $info_fs $info_disk_stats"
	  echo $info_str
	  j=`expr $j + 1`
        done
}

print_disk_info ()
{
  echo USB_DISK_TOTAL $count_disk_list
  if [ $count_disk_list -gt 0 ]; then
    i=0
    for dsk in $disk_list; do
        disk_vendor=`echo "$mountd_state"|grep "$dsk\.vendor"|cut -d= -f2`
	disk_model=`echo "$mountd_state"|grep "$dsk\.model"|cut -d= -f2`
	disk_rev=`echo "$mountd_state"|grep "$dsk\.rev"|cut -d= -f2`
        eval echo "USB_$i'_DISK_NAME' $dsk $disk_model\(rev:$disk_rev\) $disk_vendor"
        disk_nod=`echo "$mountd_state"|sed "s/'//g"|grep "$dsk\.disc"|cut -d= -f2`
	count_disk_part=`echo "$df_state" | grep $disk_nod | wc -l`
	eval echo USB_$i'_DISK_PART' $count_disk_part
        echo "$df_state" | grep $disk_nod | print_disk_part_info $i $dsk
        i=`expr $i + 1`
    done
  fi
}

disc_umount ()
{
	mountd_state=`cat /var/state/mountd`;
	disk_vendor=`echo "$mountd_state"|grep "$1\.vendor"|cut -d= -f2`
	disk_model=`echo "$mountd_state"|grep "$1\.model"|cut -d= -f2`
	disk_rev=`echo "$mountd_state"|grep "$1\.rev"|cut -d= -f2`
	info_mounted_dirs="`echo "$mountd_state"|grep "$1\.name"|cut -d= -f2`"
	#count_mounted_dirs=`echo $info_mounted_dirs | wc -w`
	for mnt_dir in $info_mounted_dirs; do
		info_update=1
		umount "/mnt/usb/$1/$mnt_dir" >/dev/null 2>/dev/null
		if [ $? -ne 0 ]; then
			ls "/mnt/usb/$1/$mnt_dir/" >/dev/null 2>/dev/null
			if [ $? -ne 0 ]; then
				umnt_success="$umnt_success, $mnt_dir"
			else umnt_busy="$umnt_busy, $mnt_dir"; fi
		else
			rm -f "/mnt/usb/$1/$mnt_dir" 2>/dev/null;
			umnt_success="$umnt_success, $mnt_dir";
		fi
	done
	[ -z "$info_update" ] && exit 0
	if [ -z "$umnt_success" ]; then
		umnt_success="none";
	fi
	if [ -z "$umnt_busy" ]; then
		umnt_busy="none";
		echo "You can now safely remove the disk '"$disk_model"(rev:"$disk_rev")"$disk_vendor" - "$1"'"
	else
		echo "One or more mounted partitions are busy for the disk '"$disk_model"(rev:"$disk_rev")"$disk_vendor" - "$1"'!!"
		echo "Please stop accessing these partitions and try again."
	fi
	umnt_success=`echo $umnt_success | cut -d',' -f2-`
	umnt_busy=`echo $umnt_busy | cut -d',' -f2-`
	echo "Safely un-mounted partitions: $umnt_success"
	echo "Busy partitions: $umnt_busy"
}

[ -z "$1" ] && echo "Usage: $0 <status/umount> [give disc serial id for umount]" && exit 0

if [ "$1" = "status" ]; then
	sync_disc_info;
	print_disk_info > /tmp/usb_info.txt
	exit 0;
elif [ "$1" = "umount" ]; then
	[ -z "$2" ] && exit 0
	disc_umount "$2"
	exit 0
fi

