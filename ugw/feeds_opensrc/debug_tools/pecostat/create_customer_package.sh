#!/bin/bash

[ -n "$1" ] || {
	echo "Usage: $0 <build dir path of pecostat>"
	exit 0;
}

rm -f pecostat_tool.c
install -m 777  $1/pecostat_tool.o .
install -m 777 $1/pecostat .

if [ -n "$FEEDDIR" ] && [ -d "$FEEDDIR/" ]; then
	rm -f "$FEEDDIR/patches/101-ltq-cpu-utilization-print-app.patch"
	rm -f "$FEEDDIR/patches/103-ltq-counter-accumualtion.patch"
	rm -f "$FEEDDIR/create_customer_package.sh"
fi

