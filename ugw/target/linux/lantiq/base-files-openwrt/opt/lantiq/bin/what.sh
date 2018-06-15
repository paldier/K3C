#!/bin/sh

for i in $*; do
	version=`strings $i | grep "@(#)" | sed 's/\(.*\)@(#)\(.*\)/\2/g'`
	echo $version
done

