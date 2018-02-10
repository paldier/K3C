#!/bin/sh

# Function declarations -START
usage ()
{
  echo -e '\n'
  echo ' -- Usage --'
  echo 'OPTION1: Script wlanIF -desc'
  echo 'OPTION2: Script wlanIF -all -id <vapID/StationID> -t <time_to_wait_after_each_get_statistics> -c <many_times_to_get_statistics>'
  echo 'OPTION3: Script wlanIF -t <time_to_wait_after_each_get_statistics> -c <many_times_to_get_statistics> -id <vapID/StationID> <Subtype0> <Subtype1> <SubtypeN>'
  echo 'OPTION4: Script wlanIF -type <start_with_statistics_type> -t <time_to_wait_after_each_get_statistics> -c <many_times_to_get_statistics> -id <vapID/StationID>'
  echo -e '\n'
  echo ' -- Examples --'
  echo 'Get Statistcs(RX_COUNTERS_RDCOUNT and HOSTIF_COUNTERS_TX_IN_UNICAST_NUM_OF_BYTES) with id 0 every 5 second 10 times:'
  echo './wstatistics wlan1 -id 0 -t 5 -c 10 RX_COUNTERS_RDCOUNT HOSTIF_COUNTERS_TX_IN_UNICAST_NUM_OF_BYTES'
  echo -e '\n'
  echo 'Get all Statistcs with id 0 every 5 second 10 times:'
  echo './wstatistics wlan1 -all -id 0 -t 5 -c 10 \n'
  echo -e '\n'
  echo 'Get all Statistcs with id 0 every 5 second 10 times and output to file sion_statistics.txt:'
  echo './wstatistics wlan1 -all -id 0 -t 5 -c 10 > /tmp/sion_statistics.txt'
  echo -e '\n'
  echo 'Get Statistcs by type'
  echo './wstatistics wlan1 -type LINK_ADAPTATION'
  echo './wstatistics wlan1 -type BAA_COUNTERS'
  echo -e '\n'
  exit
}

# check if a string contain another
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
# Run the statistics command
statistics() {
	cmd="mtdump w $STATISTICS -r /tmp/mhi_stats_temp -f $mtidlcpath/mtdump_$wave.mtidlc"
	#echo $cmd
	if [ $ID != "NULL" ]
	then
					cmd="$cmd | grep \" \[$ID\]\""
	fi
	echo "Statistics of \"$STATISTICS\""
	eval $cmd
}
# Run the statistics command
get_all_friendly_names_and_desc() {
	iii=0
	description=""
	#friendly_name=""
	mtidlc_file="$mtidlcpath/mtdump_$wave.mtidlc"
	unix_converted_mtidlc_file="/tmp/mtdump_converted.txt"
	cp $mtidlc_file $unix_converted_mtidlc_file
	eval dos2unix $unix_converted_mtidlc_file
	while read line
	do 
		aaa=${line##friendly_name}
		if [ "$aaa" != "$line" ]
		then
			#eval friendly_name$iii="${line:14}"
			eval friendly_name$iii=${line##friendly_name=}
			
			if [ $print_desc_or_all == 1 ]
			then
				eval echo \$friendly_name$iii
				eval echo \$description$iii
				echo -e '\n'
			elif [ $print_desc_or_all == 2 ]
			then
				eval temp=\${friendly_name${iii}}
				params="${params} ${temp}"
			elif [ $print_desc_or_all == 3 ]
			then
				eval temp=\${friendly_name${iii}}
				bbb=${temp##$type_to_extract} # bbb is equal to statistics subtype if not starting with $type_to_extract
				if [ "$bbb" != "$temp" ]
				then
					params="${params} ${temp}"
				fi
			fi
			
			iii=$((iii+1))
		elif test "${line#*"field_0_description="*}" != "$line"
		then
			eval description$iii="${line:8}"
		fi
	done < $unix_converted_mtidlc_file
}
# Function declarations -END

# init-START
ID="NULL"
wlan=$1
TIME=0
params=""
count_max=100
count=1
type_to_extract=""
mtidlcpath="/opt/lantiq/wave/images"
print_desc_or_all="0"
shift 1
# init-END

# Get wave version -START
OUTPUT="$(cat /proc/net/mtlk/$wlan/version)"

if  contains "$OUTPUT" "gen5_"  ||  contains "$OUTPUT" "_gen5.bin" 
then
	wave="wave500"
elif contains "$OUTPUT" "gen5b"
	then
		wave="wave500b"
elif contains "$OUTPUT" "gen4"
	then
		wave="wave400"
fi
# Get wave version -END

# Parse received arguments -START
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
		-c|--count_max)
			count_max="$2"
			shift # past argument		
		;;
		-desc|--description)
			print_desc_or_all=1
			get_all_friendly_names_and_desc	
		;;
		-all|--all_statistics)
			print_desc_or_all=2
			get_all_friendly_names_and_desc
		;;
		-type|--all_type)
			type_to_extract="$2"
			print_desc_or_all=3	
			get_all_friendly_names_and_desc
			shift # past argument
		;;
		-h|-help)
			usage
		;;		
		--default)
		;;
		*)
			params="$params $1"
		;;
	esac
	shift 1
done

# Parse received arguments -END
echo -e '\n'
# run over given statistics params -START
while true; do
	#save the mhi_stats file to local temp for data coherency and speed
	cmd="cp -f /proc/net/mtlk/$wlan/mhi_stats /tmp/mhi_stats_temp; chmod 777 /tmp/mhi_stats_temp;"
	eval $cmd
	
	echo $count
	date -u
	for param in $params
	do
		STATISTICS=$param
		statistics
	done
	
	if [ $TIME != 0 ]
	then
		sleep $TIME
		if [ $count == $count_max ]
		then
			break
		else
			count=$((count+1))
		fi
	else
		break
	fi
	
	echo -e '\n'

done
# run over given statistics params -END
