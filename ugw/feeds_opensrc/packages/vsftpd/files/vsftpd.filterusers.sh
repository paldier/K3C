#!/bin/sh
# Configure vsftp deny user list based on user accounts management.
#

if [ ! "$ENVLOADED" ]; then
        if [ -r /etc/rc.conf ]; then
                 . /etc/rc.conf 2> /dev/null
                ENVLOADED="1"
        fi
fi

local idx=0;
if [ -n "$user_obj_Count" ]; then
	if [ -f /etc/vsftpd.users ]; then
		echo root > /etc/vsftpd.users;
		while [ $idx -lt $user_obj_Count ]; do
			eval facc='$user_'$idx'_fileShareAccess';
			if [ -n "$facc" ]; then
				if ! [ "$facc" = "1" -o "$facc" = "3" ]; then
					eval fuser='$user_'$idx'_username';
					echo "$fuser" >> /etc/vsftpd.users;
				fi;
			fi;
			idx=$((idx + 1))
		done
	fi
fi

