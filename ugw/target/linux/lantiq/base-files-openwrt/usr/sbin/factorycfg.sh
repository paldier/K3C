#!/bin/sh
# Factory reset config files
#

rm -rf /overlay/*
sync; sleep 2;
reboot
