#!/bin/sh

CFG_FILE="/nvram/appcpu.cfg"
ACT=$1
shift

nvram_usage()
{
	echo "$0 <get> <key> [<default value>]"
	echo "$0 <set> <key> <value>"
	echo "$0 <del> <key>"
	echo "$0 <show>"
	exit 1
}

cfg_get()
{
	local key=$1; local def=$2; local val=""
	local ret=0

	#echo "GET $@"
	[ -z $key ] && { echo "key should not be null"; return 1; }

	val=`grep -wi $key $CFG_FILE | awk -F'=' '{print $2}'`
	[ "$val" = "" ] && { val=$def; ret=1; }
	[ "$val" != "" ] && { echo $val; }
	return $ret
}

cfg_set()
{
	local key=$1; local val=$2

	#echo "SET $@"
	([ -z $key ] || [ -z $val ]) && { echo "key/value should not be null"; return 1; }
	echo $key | grep -q '='
	[ $? -eq 0 ] && { echo "key/value should not contain special chars"; return 1; }
	echo $val | grep -q '='
	[ $? -eq 0 ] && { echo "key/value should not contain special chars"; return 1; }

	touch $CFG_FILE
	grep -viw $key $CFG_FILE > $CFG_FILE.tmp
	echo "$key=$val" >> $CFG_FILE.tmp
	mv $CFG_FILE.tmp $CFG_FILE
	sync
	return 0
}

cfg_delete()
{
	local key=$1;

	[ -z $key ] && { echo "key should not be null"; return 1; }
	[ ! -f $CFG_FILE ] && { echo "key not found"; return 1; }

	grep -qiw $key $CFG_FILE
	[ $? -ne 0 ] && { echo "key not found"; return 1; }

	grep -viw $key $CFG_FILE > $CFG_FILE.tmp
	mv $CFG_FILE.tmp $CFG_FILE
	sync
	return 0
}

cfg_show()
{
	echo "KEY=VALUE"
	echo ""

	cat $CFG_FILE
}

if [ "$ACT" = "get" ]; then
	cfg_get $@
elif [ "$ACT" = "set" ]; then
	cfg_set $@
elif [ "$ACT" = "del" ]; then
	cfg_delete $@
elif [ "$ACT" = "show" ]; then
	cfg_show
else
	nvram_usage;
fi

exit $?
