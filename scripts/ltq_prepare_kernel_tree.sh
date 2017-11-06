#!/bin/bash

if [ -f include/ltq-prepare-kernel-tree.mk ]; then
	make -f include/ltq-prepare-kernel-tree.mk OPTS=$1
else
	echo "ERROR: File include/ltq-prepare-kernel-tree.mk not found!!"
	exit 1
fi
