#!/bin/sh

for i in $*; do
	version=`zcat $i | strings | grep "@(#)" | sed 's/\(.*\)@(#)\(.*\)/\2/g'`
	echo $version
done

