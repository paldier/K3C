#!/bin/sh

interface="eth0_LAN"
delay=5

usage()
{
        echo "usage: $0 [-i|--interface <interface>] [-d|--delay <delay>] [-h|--help]" 1>&2;
        echo "options -"
        echo " -i|--interface   - interface name"
        echo " -d|--delay       - execution time"
        echo " -h|--help        - show this help"
        exit 1;
}

get_status()
{
	case "$interface" in
        "eth0_1" | "eth0_2" | "eth0_3" | "eth0_4" )
                tmp=$(echo ${interface:5})
		int=$(($tmp-1))
                rx=$(switch_cli GSW_RMON_PORT_GET nPortId=$int | grep nRxGoodBytes | awk '{print $2}')
                tx=$(switch_cli GSW_RMON_PORT_GET nPortId=$int | grep nTxGoodBytes | awk '{print $2}')
                ;;
        * )
                str=$(ifconfig $interface | grep bytes | awk '{print $2}')
                rx=$(echo ${str:6})
                str=$(ifconfig $interface | grep bytes | awk '{print $6}')
                tx=$(echo ${str:6})
                ;;
	esac
	echo $rx $tx
}

while true; do
  case "$1" in
        -i | --interface ) interface=$2; shift 2 ;;
        -d | --delay ) delay=$2; shift 2 ;;
        -h | --help ) usage; shift ;;
        -- ) shift; break ;;
        * ) break ;;
  esac
done

str=$(ifconfig $interface 2< /dev/null)
if [ "$str" = "" ]; then
	echo "ERROR:interface doesn't exist"
        usage
fi

zero=0
if [ "$delay" -le "$zero" ]; then
        echo "ERROR:delay must be a positive integer"
        usage
fi                                                                                          
                                                                                            
echo "start test: interface=$interface, delay=$delay"                                       
                                                                                            
str=$(get_status) 
RX1=$(echo $str | awk '{print $1}')
TX1=$(echo $str | awk '{print $2}')                                                                     
                                                                                                       
sleep $delay                             
            
str=$(get_status)
RX2=$(echo $str | awk '{print $1}')
TX2=$(echo $str | awk '{print $2}')                 

if [ "$RX2" -lt "$RX1" ]  || [  "$TX2" -lt "$TX1" ]; then
	echo "ERROR:test failed"
	usage
fi
                                                                                                
RX=$(($RX2-$RX1))                                                                                      
TX=$(($TX2-$TX1))                                                                                      
div=$((1024*1024*$delay))                                                                              
                                                                                                       
echo "RX throughput:"                                                                                  
var1=$(($RX/$div))                                                                                     
var2=$((($RX*8)/$div))                                                                                 
printf "%d MB/sec = %d Mb/sec\n" $var1 $var2                                                           
                                                                                                       
echo "TX throughput:"                                                                                  
var1=$(($TX/$div))                                                        
var2=$((($TX*8)/$div))                                                    
printf "%d MB/sec = %d Mb/sec\n" $var1 $var2

