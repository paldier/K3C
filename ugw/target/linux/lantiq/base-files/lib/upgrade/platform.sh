PART_NAME=firmware
REQUIRE_IMAGE_METADATA=1

platform_check_image() {
	[ "$ARGC" -gt 1 ] && return 1

	case "$(get_magic_word "$1")" in
		# .trx files
		7379 |\
		2705) return 0;;
		*)
			echo "Invalid image type"
			return 1
		;;
	esac
}

# use default for platform_do_upgrade()

disable_watchdog() {
	killall watchdog
	( ps | grep -v 'grep' | grep '/dev/watchdog' ) && {
		echo 'Could not disable watchdog'
		return 1
	}
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	lantiq,grx500)
		nand_do_upgrade $1
		;;
	*)
		default_do_upgrade "$ARGV"
		;;
	esac
}

platform_nand_pre_upgrade() {
	local board=$(board_name)

	case "$board" in
	lantiq,grx500)
		platform_upgrade_grx500 "$ARGV"
		;;
	esac
}

do_upgrade_stage2() {
	v "Performing system upgrade..."
	platform_do_upgrade "$IMAGE"

	if [ "$SAVE_CONFIG" -eq 1 ] && type 'platform_copy_config' >/dev/null 2>/dev/null; then
		platform_copy_config
	fi

	v "Upgrade completed"
	sleep 1

	v "Rebooting system..."
	umount -a
	reboot -f
	sleep 5
	echo b 2>/dev/null >/proc/sysrq-trigger
}

append sysupgrade_pre_upgrade disable_watchdog
