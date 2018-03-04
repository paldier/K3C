#!/bin/sh

script_name="wave_wlan_set_remove_params.sh"

[ ! "$LIB_COMMON_SOURCED" ] && . /tmp/wave_wlan_lib_common.sh
[ ! "$RC_CONF_SOURCED" ] && rc_conf_source

# Removing a VAP by removing its configuration files.
# TBD: do we need to lock the files to delete?

# Define local parameters
local ap_index pid interface_name
local pap_name

ap_index=$1
pid=$2

timestamp $ap_index "$script_name:$ap_index:begin"
print2log $ap_index DEBUG "$script_name $*"

eval interface_name=\${wlmnwave_${ap_index}_interfaceName}
rm -f ${TEMP_CONF_DIR}/*${interface_name} ${TEMP_CONF_DIR}/*${interface_name}.conf ${CONF_DIR}/*${interface_name}.txt ${CONF_DIR}/*${interface_name}.conf ${TEMP_CONF_DIR}/*${interface_name}_${pid}* ${CONF_DIR}/*${interface_name}_${pid}*

# Set the restart flag for the radio to which the interface belongs
pap_name=${interface_name%%.*}
echo "restart_${pap_name}=yes" >> /tmp/$RESTART_FLAG

print2log $ap_index DEBUG "$script_name done"
timestamp $ap_index "$script_name:$ap_index:done"
