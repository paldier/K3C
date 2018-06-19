
#!/bin/sh

tempfile="/tmp/logger$1.zip"
md5file="/tmp/logger$1.md5"

find_space_infilesys () {
	local avail_space=0
#	df $1 | awk '{ x = $4/1000 } ; END { if(x > 1) print x }'
	df $1 | awk '{ x = $4 } ; END { if(x > 1) print x }'
}

get_filesystem_type () {
	for i in "$(sed -n 's/^.* \([^ ]*\) '${1}' .*$/\1/p' /proc/mounts)"; do
		if [ "A${i}A" != "AA" ]; then
#			echo $1
			pathfound=$(find_space_infilesys $i)
#			echo "--------------"
#			echo "${i}:$pathfound Mbytes" 
			if [ "A${pathfound}A" != "AA" ]; then
				echo "${i}" 
				cp $tempfile ${i}
				cp $md5file ${i}
			fi
#			echo "--------------" 
		fi 
	done
}

get_storage_location () {
	local fstype_usb="ext3 ext4 fat vfat tfat ubifs ramfs tntfs" 
	for j in $fstype_usb; do
		get_filesystem_type $j 
	done
}


get_storage_location


