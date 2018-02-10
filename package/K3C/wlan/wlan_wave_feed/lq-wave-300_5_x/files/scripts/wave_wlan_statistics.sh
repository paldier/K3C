#!/bin/sh

contains() {
    string="$1"
    substring="$2"
    if test "${string#*$substring}" != "$string"
    then
        return 0    # $substring is in $string
    else
        return 1    # $substring is not in $string
    fi
}

statistics() {
	echo -e '\n'
	cmd="mtdump w $STATISTICS -r /proc/net/mtlk/$wlan/mhi_stats -f /root/mtlk/images/mtdump_$wave.mtidlc"
	if [ $ID != "NULL" ]
	then
					cmd="$cmd | grep \" \[$ID\]\""
	fi
	date -u
	echo "Statistics of \"$STATISTICS\""
	eval $cmd
}


ID="NULL"
wlan=$1
TIME=0
shift 1

OUTPUT="$(cat /proc/net/mtlk/$wlan/version)"

if  contains "$OUTPUT" "gen5_wrx_500.bin"  ||  contains "$OUTPUT" "_gen5.bin" 
then
	wave="wave500"
elif contains "$OUTPUT" "gen5b_wrx_500"
	then
		wave="wave500b"
elif contains "$OUTPUT" "gen4.bin"
	then
		wave="wave400"
fi


while [ $# -ne 0 ]
do
key="$1"

case $key in
    -id|--stationID/vapID)
		ID="$2"
		shift # past argument
    ;;
    -t|--periodic)
		TIME="$2"
		shift # past argument
    ;;
    --default)
    ;;
    *)
	while true; do
		STATISTICS="$1"
		statistics
	if [ $TIME == 0 ]
	then
		break
	fi
	sleep $TIME
	done
    ;;
esac
shift 1
done

