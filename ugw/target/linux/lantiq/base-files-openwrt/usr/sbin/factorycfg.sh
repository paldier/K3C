#!/bin/sh
# Factory reset config files
#

rm -rf /overlay/etc/config
rm -rf /overlay/etc/uci-defaults
sync
reboot
