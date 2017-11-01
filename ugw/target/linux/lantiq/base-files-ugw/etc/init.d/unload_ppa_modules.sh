#!/bin/sh

echo "Unloading PPA Modules"
toLower(){ echo "$1"|sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/';}

firmware=`/usr/sbin/status_oper GET ppe_config_status ppe_firmware`
ppe_fw=`toLower $firmware`

platform=`/usr/sbin/status_oper GET ppe_config_status ppe_platform`
ppe_platform=`toLower $platform`

sub_target=`/usr/sbin/status_oper GET ppe_config_status ppe_subtarget`

# If Dual WAN is enabled, if wired and wireless combination of wan modes is configured,
# dont remove the modules, use the same modules.
if [ "$CONFIG_FEATURE_DUAL_WAN_SUPPORT" = "1" -a "$dw_failover_state" = "1" ]; then
	pri_wired_wan="0"
	sec_wired_wan="0"
	if [ "$dw_pri_wanphy_phymode" = "5" -o "$dw_sec_wanphy_phymode" = "5" ]; then
		echo "Got Wired - Wireless Combination"
		echo "Dont remove the drivers"
		return
	fi
fi	

echo "ppe_fw to be unloaded - $ppe_fw"
if [ -n "`grep ppa /proc/modules`" ] || [ -n "`grep ppe /proc/modules`" ]; then
  rmmod ppa_api_sw_accel_mod
  rmmod swa_stack_al
  rmmod ppa_api_proc
  rmmod ppa_api
  if [ "$sub_target" = "vrx318" ]; then
    rmmod dlrx_fw
    rmmod ltqmips_dtlk
    rmmod ltqmips_${sub_target}_${ppe_fw}
    rmmod ltqmips_ppe_drv    
  else 
    rmmod ppa_hal_${ppe_platform}_${ppe_fw}
    usleep 100000
    if [ "$1" != "tr69" ]; then
      rmmod ppa_datapath_${ppe_platform}_${ppe_fw}
    fi
  fi
else
  rmmod ltqmips_eth_drv.ko 2>/dev/null
  rmmod ltqmips_atm.ko 2>/dev/null
  rmmod ltqmips_ptm.ko 2>/dev/null
fi

