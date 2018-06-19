#!/bin/sh


if [ $# -le 0 ]
then
echo "Script must get at least one parameter, exit"
exit 0
fi



cpu_num=$1

perf stat -C $cpu_num -o /tmp/test sleep 1 ; cat /tmp/test | grep GHz | awk '{print "CPU'$cpu_num' - "$4*100/2 "%" }'
rm /tmp/test
