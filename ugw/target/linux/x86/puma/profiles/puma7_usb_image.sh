#!/bin/bash

becho()
{
    echo -e "\e[1m$@\e[0m"
}

recho()
{
	echo -e "\e[1m\e[31m$@\e[0m"
}

gecho()
{
	echo -e "\e[1;42m$@\e[0m"
}

error()
{
	recho $@
	exit 1
}

device=$1
[ $# -ne 1 ] && { error "$0 <USB device, Ex: /dev/sdb>"; }

[ "`id -u`" -ne 0 ] && { error "Please run script with root user OR with sudo"; }

cd `dirname $0`

[ -b ${device} ] || { error "Device ${device} doesn't exist !!"; }

becho "Unmounting USB..."
for disk in `ls ${device}*`; do udisksctl unmount -b $disk; done

becho "Farmating USB disk..."
dd if=/dev/zero of=${device}  bs=1024  count=1024
[ $? -ne 0 ] && { error "Device initilization failed"; }

becho "Creating USB partitions..."
echo -e "n\n\n\n2048\n+9M\nt\nef\nn\n\n\n20480\n+120M\np\nw\n" | fdisk ${device}
udisksctl power-off -b ${device}
becho "Unplug & plug USB device....and then press enter to continue..."
read enter

becho "checking USB device attached !!"
while [ 1 ];
do
	sleep 2
	[ -b ${device}1 ] || { recho "USB not attached properly, trying again"; continue; }
	[ -b ${device}2 ] || { recho "USB not attached properly, trying again"; continue; }
	break
done

for disk in `ls ${device}*`; do udisksctl unmount -b $disk > /dev/null 2>&1; done

becho "Copying kernel & rootfs to USB..."
dd if=appcpu_kernel of=${device}1 conv=notrunc
[ $? -ne 0 ] && { error "Failed to copy kernel image"; }
dd if=appcpu_rootfs of=${device}2 conv=notrunc
[ $? -ne 0 ] && { error "Failed to copy rootfs image"; }

sync

becho "Ejecting USB device..."
udisksctl power-off -b ${device}
gecho "You USB is ready to use :)"
