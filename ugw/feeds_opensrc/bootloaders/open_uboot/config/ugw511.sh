#!/bin/sh

#convert the config directory to UGW511 compatible format

find ./ -name Kconfig |xargs sed -i 's/ltq_feeds_uboot/ifx_feeds_uboot/g'


