#!/bin/sh /etc/rc.common

START=03

#Need to enable later on once utils are in place, remove it from rcS

start() {
	touch /ramdisk/flash/passwd

	echo "root:\$1\$\$CoERg7ynjYLsj2j4glJ34.:0:0:root:/root:/bin/sh" > /ramdisk/flash/passwd

	i=0
	while [ $i -lt $passFileLineCount ]
	do
		eval passVar='$passFileLineCount'$i
		echo $passVar >> /ramdisk/flash/passwd
		i=$(( $i + 1 ))
	done

	i=0
	echo user obj count $user_obj_Count
	while [ $i -lt $user_obj_Count ]
	do
		eval user_ena='$'user_${i}_fEnable
		eval user_name='$'user_${i}_username
		eval user_pass='$'user_${i}_password
		if [ "$user_ena" = "1" ]; then
    # create user with,
    # (-h) home directory as /tmp/username
    # (-G) group as root : with new group we see some commands could not be executed
    # (-g) group as root : with new group we see some commands could not be executed
    # (-s) shell as /bin/sh
    # (-D) don't assign password now
    			sed -i '/'${user_name}'/d' /flash/passwd
    			if [ "${user_name}" != "root" -a "${user_name}" != "admin" -a "${user_name}" != "ftp" -a "${user_name}" != "support_user" ]; then
    				echo "${user_name}:x:201:0:Linux User:/mnt/usb:/bin/sh" >> /flash/passwd
    			else
				echo "${user_name}:x:0:0:root:/tmp/${user_name}:/bin/sh" >> /flash/passwd
			fi
			if [ -x /usr/bin/yes ]; then
				/usr/bin/yes ${user_pass} 2>/dev/null|/usr/bin/passwd ${user_name} >/dev/null 2>/dev/null 
			else
				echo Unable to change the password please install yes command 
			fi 
			# set password of admin to root
			if [ "$user_name" = "admin" ]; then
				if [ -x /usr/bin/yes ]; then 
					/usr/bin/yes ${user_pass} 2>/dev/null|/usr/bin/passwd root >/dev/null 2>/dev/null 
				else
					echo Unable to change the password please install yes command 
				fi
			fi
		fi
		i=$(( $i + 1 ))
	done

	if [ -r /etc/init.d/vsftpd.filterusers.sh ]; then
		. /etc/init.d/vsftpd.filterusers.sh
	fi;

	#if ! [ -f /etc/rc.conf ]; then
	#	echo "root:\$1\$\$CoERg7ynjYLsj2j4glJ34.:0:0:root:/root:/bin/sh" > /ramdisk/flash/passwd
	#	echo "admin:\$1\$\$CoERg7ynjYLsj2j4glJ34.:0:0:root:/root:/bin/sh" >> /ramdisk/flash/passwd
	#fi
}
