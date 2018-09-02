#!/bin/sh /etc/rc.common

START=99

. /lib/functions/common_utils.sh

# After a sysupgrade with fullimage, mark current bank as active bank in case of a dual bank image.
sysupgrade_dualbank_update()
{
	local _cmdline active_bank update_chk;

	_cmdline=$(cat /proc/cmdline);active_bank=${_cmdline//*active_bank=}
	[ "$_cmdline" != "$active_bank" ] && active_bank=${active_bank// *} || active_bank="";

	_cmdline=$(cat /proc/cmdline);update_chk=${_cmdline//*update_chk=}
	[ "$_cmdline" != "$update_chk" ] && update_chk=${update_chk// *} || update_chk="";

	[ -n "$active_bank" -a -n "$update_chk" ] && {
		[ "$active_bank" = "A" ] && active_bank=A || active_bank=B

		# The current logic will get executed only on first boot after an image upgrade.
		if [ "$active_bank" = "B" -a "$update_chk" = "0" ]; then
			/usr/sbin/uboot_env --set --name update_chk --value  2
		elif [ "$active_bank" = "A" -a "$update_chk" != "0" ]; then
			/usr/sbin/uboot_env --set --name update_chk --value 0
		fi
	}
}

start() {
	sysupgrade_dualbank_update
}
