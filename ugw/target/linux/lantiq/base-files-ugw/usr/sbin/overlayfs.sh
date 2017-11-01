#!/bin/sh
# Script to do RAM overlay on specified directories.

_help ()
{
	echo "$0: Script to do RAM overlay on specified directories (make specified directories writable)."
	echo "Usage: $0 <mount/umount> <dir1> <dir2> ... <dirN>"
	exit 1;
}

[ -z "$1" -a -z "$2" ] && _help
([ "$1" = "mount" ] || [ "$1" = "umount" ]) || \
{
	echo "$0: wrong argument '$1'" && _help
}

_CMD=$1
overlay_type="`grep -q overlayfs /proc/filesystems && echo overlayfs || echo minifo`"
overlay_path="/tmp/my_overlay"

shift

for i in $@; do
	if [ -d "$i" ]; then
		if [ "$_CMD" = "mount" ]; then
			if [ "$overlay_type" = "overlayfs" ]; then
				mkdir -p $overlay_path/"$i"
				mount -t overlayfs -o lowerdir="$i",upperdir=$overlay_path/"$i" "overlayfs:$i" "$i" || \
				{
					echo "$0: Error in mounting $i !!";
				} && {
					grep "overlayfs:$i" /proc/mounts
				}
			else
				mkdir -p $overlay_path/"$i"
				mount -t mini_fo -o base="$i",sto=$overlay_path/"$i" "$i" "$i" || \
				{
					echo "$0: Error in mounting $i !!"
				} && true
			fi
		fi
		if [ "$_CMD" = "umount" ]; then
			umount "$i" && {
				cd $overlay_path/; rmdir -p ./"$i"; cd - >/dev/null;
			}
		fi
	else
		echo "$0: No such directory named '$i' found !!"
	fi
done
