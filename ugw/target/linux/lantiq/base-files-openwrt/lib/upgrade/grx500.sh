#
# Copyright (C) 2015 OpenWrt.org
# Copyright (C) 2018 paldier <paldier@hotmail.com>
#

. /lib/functions.sh
#Note: for grx500 only


grx500_get_target_kernel() {
	local current_kernel_index=$(sed -e 's/.*active_bank=\(.*\)/\1/' -e 's/\(\([^ ]*\) \)\{1\}.*/\2/' /proc/cmdline)

	if [ $current_kernel_index == "A" ]; then
		echo 'kernelA'
	elif [ $current_kernel_index == "B" ]; then
		echo 'kernelB'
	fi
}

grx500_update_target_kernel() {
	local kernel_part=$1

	if [ $kernel_part == "kernelA" ]; then
		/usr/sbin/uboot_env --set --name active_bank --value A 1>/dev/null
		/usr/sbin/uboot_env --set --name update_chk --value 3 1>/dev/null
	elif [ $kernel_part == "kernelB" ]; then
		/usr/sbin/uboot_env --set --name active_bank --value B 1>/dev/null
		/usr/sbin/uboot_env --set --name update_chk --value 1 1>/dev/null
	else
		echo 'Failed to update kernel bootup index' >&2
		return 1
	fi
}

platform_upgrade_grx500() {

	local kernel_part="$(grx500_get_target_kernel)"
	if [ -z "$kernel_part" ]; then
		echo "cannot find factory partition" >&2
		exit 1
	fi
	local kernel_flash rootfs_flash
	if [ $kernel_part == "kernelA" ]; then
		kernel_flash="kernelB"
		rootfs_flash="rootfsB"
	else
		kernel_flash="kernelA"
		rootfs_flash="rootfsA"
	fi
	# This is a global defined in nand.sh, sets partition kernel will be flashed into
	CI_KERNPART=${kernel_flash}
	CI_KERNPART=${rootfs_flash}
	#Remove volume possibly left over from stock firmware
	local ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	if [ -z "$ubidev" ]; then
		local mtdnum="$( find_mtd_index "$CI_UBIPART" )"
		if [ -z "$mtdnum" ]; then
			echo "cannot find ubi mtd partition $CI_UBIPART" >&2
			exit 1
		fi
		ubiattach -m "$mtdnum"
		sync
		ubidev="$( nand_find_ubi "$CI_UBIPART" )"
	fi

	if [ -n "$ubidev" ]; then
		local troot_ubivol="$( nand_find_volume $ubidev $rootfs_flash )"
		[ -n "$troot_ubivol" ] && ubirmvol /dev/$ubidev -N $rootfs_flash || true
	fi

	grx500_update_target_kernel ${kernel_part} || exit 1
}

