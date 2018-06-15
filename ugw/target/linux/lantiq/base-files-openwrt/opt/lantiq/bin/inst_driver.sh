#! /bin/sh
#
# Install device driver
#
# para 1 ($1): debug level (0 = use local debug level)
# para 2 ($2): device driver name
# para 3 ($3): binary name (optional)
# para 4 ($4): device node name (optional)
# para 5 ($5): number of devices (optional)
# 

if [ $# -lt 2 ]; then
	echo "Missing parameter(s)!"
	exit 1
fi

# check for linux 2.6.x
uname -r | grep -q "^2\.6\." && MODEXT=.ko

debug_level=$1
drv_dev_base_name=$2
drv_obj_file_name=${3-$2}$MODEXT
drv_dev_node_name=${4-$2}
devices=${5-0}

bindir=/opt/lantiq/bin

# debug_level: 1=low, 2=normal, 3=high, 4=off
# set local debug_level, if 0
if [ $debug_level -eq 0 ]; then
	debug_level=2
fi

# enable debugging outputs, if necessary
if [ "$debug_level" != 4 ]; then
	echo 8 > /proc/sys/kernel/printk
fi

cd $bindir

name=`grep -i "$drv_dev_base_name" /proc/devices | cut -d' ' -f2`
if [ -z $name ]; then
	strings $drv_obj_file_name | grep -q __module_parm_debug_level
	if [ $? = 0 ]; then
		insmod $drv_obj_file_name debug_level=$debug_level
	else
		insmod $drv_obj_file_name
	fi

	if [ $? -ne 0 ]; then
		echo "ERROR: loading driver failed."
		exit 1
	fi

fi

major_no=`grep -i "$drv_dev_base_name" /proc/devices | cut -d' ' -f1`

# exit if major number not found (e.g. in case of devfs)
if [ -z $major_no ]; then
	exit 0
fi

if [ $devices = 0 ]; then
	basedir=`dirname /dev/$drv_dev_node_name`
	test ! -d  $basedir && mkdir -p $basedir
	test ! -e /dev/$drv_dev_node_name && mknod /dev/$drv_dev_node_name c $major_no 0
else
	I=0
	basedir=`dirname /dev/$drv_dev_node_name/0`
	test ! -d  $basedir && mkdir -p $basedir
	while test $I -lt $devices; do 
		test ! -e /dev/$drv_dev_node_name/$I && mknod /dev/$drv_dev_node_name/$I c $major_no $I
		I=`expr $I + 1`
	done
fi
