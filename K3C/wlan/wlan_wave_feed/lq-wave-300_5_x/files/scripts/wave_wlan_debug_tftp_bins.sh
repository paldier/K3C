#!/bin/sh

script_name="wave_wlan_debug_tftp_bins.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

(. $ETC_PATH/wave_wlan_debug.sh tftp_bins)
