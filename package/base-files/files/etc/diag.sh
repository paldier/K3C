#!/bin/sh
# Copyright (C) 2006-2009 OpenWrt.org

set_state() { :; }

if [ -e /etc/diag.arch.sh ]; then
	. /etc/diag.arch.sh
fi
