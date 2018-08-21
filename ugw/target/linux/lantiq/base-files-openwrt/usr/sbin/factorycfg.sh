#!/bin/sh
# Factory reset config files
#

rm -rf /overlay/*
echo "Resets system configuration and applications.";
sync; sleep 2;
reboot
