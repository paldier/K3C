#!/bin/sh /etc/rc.common
START=06
if [ -n "$CONFIG_TARGET_LANTIQ_VBG500_VBG500" ]; then exit; fi

# Include model information
if [ ! "$ENVLOADED" ]; then
	if [ -r /etc/rc.conf ]; then
	. /etc/rc.conf 2> /dev/null
		ENVLOADED="1"
	fi
fi

# Get the platform information.
platform=${CONFIG_IFX_MODEL_NAME%%_*}
target=`echo $platform | cut -c -4`
#echo "$target"

check_bonding_status() {
	bonding=`/usr/sbin/status_oper GET xDSL_Bonding Bonding_mode`
	if [ "$sub_platform" = "vrx318" ]; then
		bonding_mode="NA"
	else
		if [ "$bonding" = "" ]; then 
			if [ "$CONFIG_PACKAGE_KMOD_LTQCPE_PPE_E5_OFFCHIP_BONDING" = "1" ]; then
				# OFF Chip Bonding
				bonding_mode="2"
			else
				# ON Chip Bonding
				bonding_mode="1"
			fi
		else
			bonding_mode="$bonding"
		fi
	fi
	/usr/sbin/status_oper SET "xDSL_Bonding" "Bonding_mode" $bonding_mode "Bonding_status" "INACTIVE" "active_line" "-1" "Status_0" "DOWN" "Status_1" "DOWN" "US_0" "0" "DS_0" "0" "US_1" "0" "DS_1" "0"
}

if [ "$wanphy_phymode" = "5" -o "$wanphy_phymode" = "6" ]; then 
	#if 3G, use D5 as ethernet driver 
	echo "Wireless WAN Mode selected"
	wanphy_phymode="2"
	# if Dual WAN is enabled, use the driver for the alternate mode
	if [ "$CONFIG_FEATURE_DUAL_WAN_SUPPORT" = "1" -a "$dw_failover_state" = "1" ]; then
		if [ "$dw_pri_wanphy_phymode" = "5" -o  "$dw_pri_wanphy_phymode" = "6" ]; then
			wanphy_phymode=$dw_sec_wanphy_phymode
			wanphy_tc=$dw_sec_wanphy_tc
		elif [ "$dw_sec_wanphy_phymode" = "5" -o  "$dw_sec_wanphy_phymode" = "6" ]; then
			wanphy_phymode="$dw_pri_wanphy_phymode"
			wanphy_tc="$dw_pri_wanphy_tc"
		fi
	fi
fi


toUpper(){ echo "$1"|sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/';}
toLower(){ echo "$1"|sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/';}

wan_qos=0
if [ $qm_enable -eq 1 ]; then
  if [ "$CONFIG_NEW_FRAMEWORK" = "1"]; then
	wan_qos=8
  else
   # identify if fw queues are required based on current state of rc.conf (i.e., WAN mode, queue method, queue/classifier features)
   /usr/sbin/queuecfg -e 0/0
   if [ "A`/usr/sbin/status_oper GET q_req use_fw_q`" = "A1" ]; then
	  wan_qos=8
   fi
  fi
elif [ $sessmgmt_wanacc -eq 1 2>/dev/null ]; then
  if [ "$CONFIG_NEW_FRAMEWORK" = "1"]; then
  	wan_qos=2
  elif [ "$wanphy_phymode" = "1" -o "$wanphy_phymode" = "2" ]; then #Ethernet
		wan_qos=2
  fi
elif [ "$CONFIG_NEW_FRAMEWORK" = "1"]; then
	wan_qos=0
elif [ "A`/usr/sbin/status_oper GET qos_al fw_q`" = "A1" ]; then
	wan_qos=8
else
	wan_qos=0
fi

if [ "$CONFIG_FEATURE_WWAN_LTE_SUPPORT" = "1" ]; then
	wan_itf=8
else
	wan_itf=0
fi

if [ "$CONFIG_NEW_FRAMEWORK" != "1" ]; then
if [ "A${wan_qos}" != "A0" -o "A${wan_itf}" != "A0" ]; then
	/usr/sbin/status_oper SET fw_queues in_use 1
else
	/usr/sbin/status_oper SET fw_queues in_use 0
fi
fi

set_platform_config() {
	module=""
	ppe_module=""
	ppe_platform=""
	builtin_package=""
	loadable_package=""
	phy_mode=$2

	case $target in
		GRX5)
			platform="xrx500"
			if [ "$1" = "PTM" ]; then
				sub_platform="vrx318"
				loadable_package_val=1
				module="e1"
			elif [ "$1" = "ATM" ]; then
				sub_platform="vrx318"
				loadable_package_val=1
                                module="a1"
			fi
		;;
		VRX2|GRX2)
			platform="xrx200"
			module="a5"
			if [ "$1" = "ETHERNET" -o "$1" = "LTE" ]; then
				module="d5"
			elif [ "$1" = "PTM" ]; then
				module="e5"
			fi
		;;
		ARX3|GRX3|VRX3)
			platform="xrx300"
			[ "$CONFIG_TARGET_LANTIQ_XRX330" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX300_EASY388_VRX318" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX300_EASY388_BOND" = "1" ] && {
				sub_platform="vrx318"
				loadable_package_val=1
				if [ "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ]; then
					platform="xrx330"
				fi
				if [ "$1" = "ATM" ]; then
					module="a1"
				elif [ "$1" = "PTM" ]; then
					module="e1"
				elif [ "$1" = "ETHERNET" -o "$1" = "LTE" ]; then
					module="d5"	
				fi
			} || {
				module="a5"
				if [ "$1" = "ETHERNET" -o "$1" = "LTE" ]; then
					module="d5"
				 elif [ "$1" = "PTM" ]; then
					module="e5"
				fi
			}
		;;
	esac

# NOTE - The below macros needs to be present in config.sh. 
# So, defining them for use here. Please dont remove them
#
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_A5_BUILTIN
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_E5_BUILTIN
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_D5_BUILTIN
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_A5_MOD
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_E5_MOD
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_D5_MOD
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_VRX318_D5_MOD
# CONFIG_PACKAGE_KMOD-LANTIQ_PPA_VRX318_E1_MOD
# CONFIG_PACKAGE_KMOD-LANTIQ_PPA_VRX318_A1_MOD
# CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500

	ppe_module=`toUpper $module`
	ppe_platform=`toUpper $platform`
	if [ "$ppe_module" != "" -a "$sub_platform" = "" ]; then
		builtin_package=CONFIG_PACKAGE_KMOD_LANTIQ_PPA_${ppe_module}_BUILTIN
		loadable_package=CONFIG_PACKAGE_KMOD_LANTIQ_PPA_${ppe_module}_MOD
		eval builtin_package_val='$'$builtin_package
		eval loadable_package_val='$'$loadable_package
	fi
}

update_ppe_status() {
	if [ "$sub_platform" != "" ]; then
	/usr/sbin/status_oper SET "ppe_config_status" "ppe_platform" $ppe_platform "ppe_firmware" $ppe_module "ppe_subtarget" $sub_platform
	else
	/usr/sbin/status_oper SET "ppe_config_status" "ppe_platform" $ppe_platform "ppe_firmware" $ppe_module
	fi
}

load_modules() {
	set_platform_config $1 $2
	
	if [ "$CONFIG_FEATURE_DSL_BONDING_SUPPORT" = "1" ]; then
		check_bonding_status
	fi
	
	#GRX500 uses multiple acceleration engines 
	if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PPA_GRX500" = "1" ]; then
		#PAE hal driver
		pae_hal_driver=ltq_pae_hal.ko
		if [ -r /lib/modules/*/$pae_hal_driver ]; then
			echo "insmod /lib/modules/*/$pae_hal_driver"
			insmod /lib/modules/*/$pae_hal_driver
		fi
		#MPE HAL driver
		mpe_hal_driver=ltq_mpe_hal_drv.ko
		if [ -r /lib/modules/*/$mpe_hal_driver ]; then
		    echo "insmod /lib/modules/*/$mpe_hal_driver"
		    insmod /lib/modules/*/$mpe_hal_driver
		fi
		#add more hals as needed here	
		tmu_hal_driver=ltq_tmu_hal_drv.ko
		if [ -r /lib/modules/*/$tmu_hal_driver ]; then
			echo "insmod /lib/modules/*/$tmu_hal_driver"
			insmod /lib/modules/*/$tmu_hal_driver
		fi

		#Directconnect datapath driver
		directconnect_driver=ltq_directconnect_datapath.ko
		if [ -r /lib/modules/*/$directconnect_driver ]; then
			echo "insmod /lib/modules/*/$directconnect_driver"
			insmod /lib/modules/*/$directconnect_driver
		fi

		#Directpath datapath driver
		directpath_driver=ltq_directpath_datapath.ko
		if [ -r /lib/modules/*/$directpath_driver ]; then
			echo "insmod /lib/modules/*/$directpath_driver"
			insmod /lib/modules/*/$directpath_driver
		fi

		#xdsl HAL driver
		if [ "$loadable_package_val" = "1" -a "$sub_platform" = "vrx318" ]; then
		    xdsl_hal_driver=ltqmips_${sub_platform}_${module}.ko
		    if [ -r /lib/modules/*/$xdsl_hal_driver ]; then
			insmod /lib/modules/*/$xdsl_hal_driver
		    fi
		fi
	    
		insmod /lib/modules/*/ppa_api.ko
		insmod /lib/modules/*/ppa_api_proc.ko 
		insmod /lib/modules/*/swa_stack_al.ko
		insmod /lib/modules/*/ppa_api_sw_accel_mod.ko 
		if [ -r /lib/modules/*/ppa_api_tmplbuf.ko ]; then
			echo "insmod /lib/modules/*/ppa_api_tmplbuf.ko"
			insmod /lib/modules/*/ppa_api_tmplbuf.ko
		fi
		echo wlan1 c 0 p 0 c 1 p 1 c 2 p 2 c 3 p 3 c 4 p 4 c 5 p 5 c 6 p 6 c 7 p 7 c 8 p 7 c 9 p 7 c 10 p 7 c 11 p 7 c 12 p 7 c 13 p 7 c 14 p 7 c 15 p 7 > /proc/ppa/api/class2prio
		echo wlan0 c 0 p 0 c 1 p 1 c 2 p 2 c 3 p 3 c 4 p 4 c 5 p 5 c 6 p 6 c 7 p 7 c 8 p 7 c 9 p 7 c 10 p 7 c 11 p 7 c 12 p 7 c 13 p 7 c 14 p 7 c 15 p 7 > /proc/ppa/api/class2prio
		update_ppe_status

		#Directlink driver
	 dtlk_driver=ltqmips_dtlk.ko
		if [ -r /lib/modules/*/$dtlk_driver ]; then
			 echo "insmod /lib/modules/*/$dtlk_driver"
			 insmod /lib/modules/*/$dtlk_driver
		fi
		#DlRx
		dlrx_driver=dlrx_fw.ko
		if [ -r /lib/modules/*/$dlrx_driver ]; then
				echo "insmod /lib/modules/*/$dlrx_driver"
				insmod /lib/modules/*/$dlrx_driver
		fi

		case $1 in
			ATM)
				insmod /lib/modules/*/vrx318_tc.ko
				echo load atm 0 > /proc/driver/ltq_vrx318/tc_switch
			;;
			PTM)
				insmod /lib/modules/*/vrx318_tc.ko
				echo load ptm 0 > /proc/driver/ltq_vrx318/tc_switch
			;;
		esac
	else
	#for all platforms that use PPE based acceleration
		if [ "$builtin_package_val" = "1" ]; then
		    echo "package builtin - $builtin_package"
		    update_ppe_status
		elif [ "$loadable_package_val" = "1" ]; then
		    if [ "$sub_platform" = "vrx318" ]; then
			ethernet_driver=ltqmips_ppe_drv.ko
			if [ -r /lib/modules/*/$ethernet_driver ]; then
#				echo "insmod /lib/modules/*/$ethernet_driver ethwan=$phy_mode wanitf=$wan_itf"
				insmod /lib/modules/*/$ethernet_driver ethwan=$phy_mode wanitf=$wan_itf wanqos_en=$wan_qos
			fi
			if [ "$ppe_module" != "" ]; then
				datapath_driver=ltqmips_${sub_platform}_${module}.ko
				insmod /lib/modules/*/$datapath_driver
			fi
		    else
			datapath_driver=ppa_datapath_${platform}_${module}.ko
			hal_driver=ppa_hal_${platform}_${module}.ko
			if [ -r /lib/modules/*/$datapath_driver ]; then
				if [ "$ppe_module" = "D5" -a "$target" = "GRX2" -a "$CONFIG_FEATURE_WWAN_LTE_SUPPORT" = "1" ]; then
					insmod /lib/modules/*/$datapath_driver ethwan=$phy_mode wanqos_en=0 wanitf=$wan_itf
				elif [ "$ppe_module" = "D5" -a "$target" = "GRX2" ]; then
					insmod /lib/modules/*/$datapath_driver ethwan=$phy_mode wanqos_en=0
				elif [ "$ppe_module" = "E5" -a "$CONFIG_FEATURE_DSL_BONDING_SUPPORT" = "1" ]; then
					insmod /lib/modules/*/$datapath_driver ethwan=$phy_mode wanqos_en=$wan_qos dsl_bonding=$bonding_mode
				else
					insmod /lib/modules/*/$datapath_driver ethwan=$phy_mode wanqos_en=$wan_qos wanitf=$wan_itf
				fi
				insmod /lib/modules/*/$hal_driver
			fi
		    fi
                   
		    insmod /lib/modules/*/ltqmips_dtlk.ko
                    insmod /lib/modules/*/dlrx_fw.ko
		    insmod /lib/modules/*/ppa_api.ko
		    insmod /lib/modules/*/ppa_api_proc.ko 
		    insmod /lib/modules/*/swa_stack_al.ko
		    insmod /lib/modules/*/ppa_api_sw_accel_mod.ko 

		    update_ppe_status
		    # Disable flow control if PPA is enabled.
		    if [ "$CONFIG_IFX_CONFIG_CPU" = "XRX288" ]; then
			switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=6 eFlowCtrl=4
		    fi
		else
		    if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_ETH_DRV_MOD" = "1" ]; then
			grep mips_eth_drv /proc/modules
			if [ "$?" != "0" ]; then
				insmod /lib/modules/*/*mips_eth_drv.ko
			fi
		    elif [ "$CONFIG_PACKAGE_KMOD_LANTIQ_ETH_DRV_BUILTIN" = "1" ]; then
			echo -n
		    else
			echo "Ethernet Driver not available. Exit Show"
		    fi
		    case $1 in 
			ATM)
				if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_ATM_MOD" = "1" ]; then
					# This is Non-Accelerated Mode, so insert stand-alone
					# Ethernet/Switch driver along with A1 ATM Driver
					insmod /lib/modules/*/*mips_atm.ko
					ppe_module="A1"
					update_ppe_status
				fi
			;;	
			PTM)
				if [ "$CONFIG_PACKAGE_KMOD_LANTIQ_PTM_MOD" = "1" ]; then	
					# This is Non-Accelerated Mode, so insert stand-alone
					# Ethernet/Switch driver along with E1 PTM Driver
					insmod /lib/modules/*/*mips_ptm.ko
					ppe_module="E1"
					update_ppe_status
				fi
			;;
			ETHERNET)
				switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0xccd nData=0x20
			;;
		    esac
		fi
	fi
}

start() {
	
	if [ "$wanphy_phymode" = "1" -o "$wanphy_phymode" = "2" ]; then #Ethernet
		load_modules "ETHERNET" $wanphy_phymode
	elif [ "$wanphy_phymode" = "6" -a "$CONFIG_FEATURE_DUAL_WAN_SUPPORT" != "1" ]; then
		load_modules "LTE" 2
	else
		if [ "$wanphy_tc" = "0" ]; then 
			load_modules ATM 0 
		elif [ "$wanphy_tc" = "1" ]; then
			load_modules PTM 0 
		else
			echo "UNSUPPORTED TC MODE : $wanphy_tc"
			if [ ! -f /etc/rc.conf.gz ]; then
				#Running in evaluation mode. Loading ethernet driver..
				cat /proc/modules | grep mips_eth_drv
				[ $? -ne 0 ] && insmod /lib/modules/*/*mips_eth_drv.ko && [ `mount|grep -q nfs;echo $?` -eq  0 ] || ifconfig eth0 up
			fi
		fi
	fi

}

