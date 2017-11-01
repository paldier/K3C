#!/bin/sh
# *Authors: Shivaji Roy, Kamal Eradath
# *This script provides functions to configure the switch for any wan mode
# *Any subsystem which configures the switch ports that can affect the basic 
# switching behavior must use this scripts function for doing it.
# *Revision History:
#  Date 22-Nov-2012:Added options from 0-8. Add new option after 8 and vefore "Help"
#                   Mutlicast snooping configuration for switch TBD.
#  Date 22-Dec-2012:Added support for MIIO mode 
#  Date 15-Jan-2013:Extend various wan mode configuration modes based on the input to the select_wan_mode function
#  Date 31-Jan-2013:Added support for XRX100.
#  Date 06-Dec-2013:Added support for bridge acceleration port separated as well as non port separated LAN
#  Date 20-Jan-2014:Added support for accelerating VAPs on port 10 & 11 in ETH WAN mode 
#  Date 25-Mar-2014:Added support for switch buffer configuration on GRX300 platform

##############################################################################################################
# Sourcing the switch port config for the board
##############################################################################################################
if [ "$CONFIG_PACKAGE_QUANTENNA_TYPE_TWO_RGMII" = "1" -o "$CONFIG_PACKAGE_QUANTENNA_TYPE_SINGLE_RGMII" = "1" ]; then
. /etc/switchports_qtn.conf
else
. /etc/switchports.conf
fi

###############################################################################################################
# *Switch ports can vary of different platforms
# *When selecting the WAN using the select_wan_mode function port numbers also gets re-configured.
# *If any change in the switch port config is needed according to any new platform we need to make 
# changes here.
###############################################################################################################
cpu_port="6"
wan_port="5"
if [ "$switch_mii1_port" != "" ]; then
  wan_port_mii1=$switch_mii1_port
else
  wan_port_mii1="5"
fi
wan_port_mii0="4"
wan_port_xdsl="10 11"
wan_port_lte="7"
if [ "$switch_lan_ports" != "" ]; then
  lan_port=$switch_lan_ports
else
  lan_port="0 1 2 4"
fi
lan_port_vrx220="4 3 2"
lan_port_all="$lan_port $wan_port_mii1"
lan_port_mii0="0 1 2"
wlan_port="7 8 9"
wlan_port_all="7 8 9"
wlan_port_lte="8 9"
wlan_port_ext="10 11"
pce_rule_start="50"
lan_port_1=$(echo ${lan_port} | awk '{ print $1 }')
###############################################################################################################
# *VLAN IDs used for LAN / WAN separation inside switch.
# *501 = LAN side VLAN.
#  -> All LAN ports (1,2,3,4), WLAN ports (7,8,9)  will have this as PVID
#  -> LAN ports (1,2,3,4), WLAN ports (7,8,9) & CPU port (6) will be members of this VLAN
#  -> This ensures that # Enables isolation of a specified port from the rest of the switch
# Any packets from packets from LAN side is not flooded to wan side
# *500 = CPU port VLAN.
#  -> CPU port will have this as PVID
#  -> LAN ports(1,2,3,4),  WLAN ports (7,8,9), WAN ports (5,10,11) will be members of this VLAN
#  -> This ensures that packets from CPU reach LAN side as well as WAN side
#  *502 = WAN side VLAN
#  -> All WAN ports (5,10,11) will have this PVID
#  -> WAN ports (5,10,11) and CPU port (6) will be members of this VLAN
#  -> This ensures that packets from WAN side is not flooded to LAN side
###############################################################################################################
lan_vid_all=501
cpu_vid=500
wan_vid=502

###############################################################################################################
# *WAN and LAN VLAN needs to have different fids so that the swithc maintains two different set of 
# switch tables which helps in maintaining the LAN / WAN isolation.
###############################################################################################################
lan_fid=0
cpu_fid=0
wan_fid=1

###############################################################################################################
# *Value to be written to the 0xCCD register of switch to set it to ADSL WAN and ETH WAN modes. 
# 0x00 = port 5 will be disabled 0x20 = port 5 will be enabled
# default wan mode = ETHWAN MII1
# default secondary wan = none
###############################################################################################################
# wan_reg="0x00" 
config_wan="2"
sec_wan="-1"
###############################################################################################################
# *VLAN ids 2050-2053 will be used internally for LAN port separation 
###############################################################################################################
lan_vid_port_start=2050

###############################################################################################################
# *VLAN ids 2-5 will be used for creating virtual interfaces eth0.2-etho.5
###############################################################################################################
software_vid_port_start=`expr $lan_vid_port_start \- 2048`


###############################################################################################################
# *Sourcing the rc.conf and config.sh if they are already not sourced.
###############################################################################################################
if [ ! "$ENVLOADED" ]; then
        if [ -r /etc/rc.conf ]; then
                 . /etc/rc.conf 2> /dev/null
                ENVLOADED="1"
        fi
fi
if [ ! "$CONFIGLOADED" ]; then
        if [ -r /etc/rc.d/config.sh ]; then
                . /etc/rc.d/config.sh 2>/dev/null
                CONFIGLOADED="1"
        fi
fi

###############################################################################################################
# This function does the following
#  *Selects the WAN mode based on the inputs given
#  *Selects the WAN port and LAN ports to be used based on the WAN mode and platform
#  * If no inputs are given to this function it reads the primary and secondary wan modes from rc.conf  
#     mapping from the rc.conf parameter values
#    0 = ADSL
#     1 = MII0
#     2 = MII1
#     3 = VDSL
#     4 = AUTO; used only by MAPI/WEB for Autodetect
#     5 = 3G
#     6 = LTE
#  * If only one input is give we assume that there is no Dual WAN 
#  * If two inputs are given we take it as primary and secondary wan mode.  
###############################################################################################################
select_wan_mode() 
{
  local wan_reg_mii1=0
  local wan_reg=0
   
  wan_reg_mii1="$(( 1 << $wan_port_mii1))"
   
  if [ "$#" = "0" ]; then
     # No Parameters passed so reading WAN modes from rc.conf
     config_wan=$wanphy_phymode
    if [ "$CONFIG_FEATURE_DUAL_WAN_SUPPORT" = "1" -a "$dw_failover_state" = "1" ]; then
        config_wan=$dw_pri_wanphy_phymode
        sec_wan=$dw_sec_wanphy_phymode    
    fi
  elif [ "$#" = "1" ]; then
    # Only one parameter passed so no Dual WAN
    config_wan=$1
    sec_wan="-1"
  else
    # Dual WAN configurations passed
    config_wan=$1
    sec_wan=$2  
  fi

  case "$config_wan" in 
    "2")
      
      # Setting switch port configuration for Ethernet WAN MII1 mode
      wan_port="$wan_port_mii1"

      if [ -n "$CONFIG_TARGET_LANTIQ_XRX200_EASY220" ]; then
        lan_port_all=$lan_port_vrx220
        lan_port=$lan_port_vrx220
        lan_port_1=$(echo ${lan_port} | awk '{ print $1 }')
      else
        lan_port_all="$lan_port"
      fi
      if [ "$sec_wan" = "6" ]; then
         # Dual WAN (MII1 & LTE)
               wan_port="$wan_port_mii1 $wan_port_lte"
         wlan_port="$wlan_port_lte $wlan_port_ext"
      elif [ "$sec_wan" = "0" -o "$sec_wan" = "3" ]; then
         # Dual WAN (MII1 & xDSL)
               wan_port="$wan_port_mii1 $wan_port_xdsl"
      else
         wlan_port="$wlan_port_all $wlan_port_ext"
      fi
      wan_reg="$wan_reg_mii1"  
      ;;

    "1")
      #Setting switch port configuration for Ethernet WAN MII0 mode
      wan_port="$wan_port_mii0"
      lan_port="$lan_port_mii0"
      lan_port_all="$lan_port $wan_port_mii1"
      ;;

    "0"|"3")
      # Setting switch port configuration for DSL WAN
      # ARX188 external switch is currently not supporting the vlan config on virutal ports 10 & 11  
      if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
        wan_port="$wan_port_xdsl"
        if [ "$sec_wan" = "6" ]; then
          # Dual WAN (xDSL & LTE)
          wan_port="$wan_port_xdsl $wan_port_lte"
          wlan_port="$wlan_port_lte"
        elif [ "$sec_wan" = "2" ]; then
          # Dual WAN (xDSL & MII1)
          wan_port="$wan_port_xdsl $wan_port_mii1"
          wan_reg="$wan_reg_mii1"  
          lan_port_all="$lan_port"
        fi
        if [ "A$CONFIG_FEATURE_DSL_BONDING_SUPPORT" = "A1" ]; then
          bonding_wan="1"
        fi
      else
        wan_port="$wan_port_mii1"
      fi
      ;;
 
    "6")
      #Setting switch port configuration for LTE WAN mode
      wan_port="$wan_port_lte"
      wlan_port="$wlan_port_lte"
      #echo "Setting switch port configuration for LTE WAN"
      if [ "$sec_wan" = "0" -o "$sec_wan" = "3" ]; then
        # Dual WAN (LTE & xDSL)
        wan_port="$wan_port_lte $wan_port_xdsl"
        if [ "A$CONFIG_FEATURE_DSL_BONDING_SUPPORT" = "A1" ]; then
          bonding_wan="1"
        fi
      elif [ "$sec_wan" = "2" ]; then
        # Dual WAN (LTE & MII1)
        wan_port="$wan_port_lte $wan_port_mii1"
        lan_port_all="$lan_port"
        wan_reg="$wan_reg_mii1"  
      fi
      ;;
 
    *)
      ;;
   esac
   
   # ARX188 doesnt allow setting IFX_FLOW_REGISTER_SET to 0
   if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
     switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_FLOW_REGISTER_SET nRegAddr=0xCCD nData=$wan_reg  
   fi
}

select_wan_mode_accel() 
{
  local wan_reg_mii1=0
  local wan_reg=0
   
  wan_reg_mii1="$(( 1 << $wan_port_mii1))"
   
  if [ "$#" = "0" ]; then
     # No Parameters passed so reading WAN modes from rc.conf
     config_wan=$wanphy_phymode
    if [ "$CONFIG_FEATURE_DUAL_WAN_SUPPORT" = "1" -a "$dw_failover_state" = "1" ]; then
        config_wan=$dw_pri_wanphy_phymode
        sec_wan=$dw_sec_wanphy_phymode    
    fi
  elif [ "$#" = "1" ]; then
    # Only one parameter passed so no Dual WAN
    config_wan=$1
    sec_wan="-1"
  else
    # Dual WAN configurations passed
    config_wan=$1
    sec_wan=$2  
  fi

  case "$config_wan" in 
    "2")
      
      # Setting switch port configuration for Ethernet WAN MII1 mode
      wan_port="$wan_port_mii1"

      if [ -n "$CONFIG_TARGET_LANTIQ_XRX200_EASY220" ]; then
        lan_port_all=$lan_port_vrx220
        lan_port=$lan_port_vrx220
        lan_port_1=$(echo ${lan_port} | awk '{ print $1 }')
      else
        lan_port_all="$lan_port"
      fi
      if [ "$sec_wan" = "6" ]; then
         # Dual WAN (MII1 & LTE)
               wan_port="$wan_port_mii1 $wan_port_lte"
         wlan_port="$wlan_port_lte $wlan_port_ext"
      elif [ "$sec_wan" = "0" -o "$sec_wan" = "3" ]; then
         # Dual WAN (MII1 & xDSL)
               wan_port="$wan_port_mii1 $wan_port_xdsl"
      else
         wlan_port="$wlan_port_all $wlan_port_ext"
      fi
      wan_reg="$wan_reg_mii1"  
      ;;

    "1")
      #Setting switch port configuration for Ethernet WAN MII0 mode
      wan_port="$wan_port_mii0"
      lan_port="$lan_port_mii0"
      lan_port_all="$lan_port $wan_port_mii1"
      ;;

    "0"|"3")
      # Setting switch port configuration for DSL WAN
      # ARX188 external switch is currently not supporting the vlan config on virutal ports 10 & 11  
      if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
        wan_port="$wan_port_xdsl"
        if [ "$sec_wan" = "6" ]; then
          # Dual WAN (xDSL & LTE)
          wan_port="$wan_port_xdsl $wan_port_lte"
          wlan_port="$wlan_port_lte"
        elif [ "$sec_wan" = "2" ]; then
          # Dual WAN (xDSL & MII1)
          wan_port="$wan_port_xdsl $wan_port_mii1"
          wan_reg="$wan_reg_mii1"  
          lan_port_all="$lan_port"
        fi
        if [ "A$CONFIG_FEATURE_DSL_BONDING_SUPPORT" = "A1" ]; then
          bonding_wan="1"
        fi
      else
        wan_port="$wan_port_mii1"
      fi
      ;;
 
    "6")
      #Setting switch port configuration for LTE WAN mode
      wan_port="$wan_port_lte"
      wlan_port="$wlan_port_lte"
      #echo "Setting switch port configuration for LTE WAN"
      if [ "$sec_wan" = "0" -o "$sec_wan" = "3" ]; then
        # Dual WAN (LTE & xDSL)
        wan_port="$wan_port_lte $wan_port_xdsl"
        if [ "A$CONFIG_FEATURE_DSL_BONDING_SUPPORT" = "A1" ]; then
          bonding_wan="1"
        fi
      elif [ "$sec_wan" = "2" ]; then
        # Dual WAN (LTE & MII1)
        wan_port="$wan_port_lte $wan_port_mii1"
        lan_port_all="$lan_port"
        wan_reg="$wan_reg_mii1"  
      fi
      ;;
 
    *)
      ;;
   esac
} 

###############################################################################################################
# Function to create a VLAN id by calling IFX_ETHSW_VLAN_ID_CREATE 
# Arguements:
# $1 VLANid to be created <1-4096>
# $2 Fid to be sepcified. <1-256>
###############################################################################################################
create_vlanid() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_ID_CREATE nVId=$1 nFId=$2
}

###############################################################################################################
# Function to delete a specified VLAN id by using IFX_ETHSW_VLAN_ID_DELETE
# Arguements:
# $1 vlan id to be deleted <1-4096>
###############################################################################################################
delete_vlanid() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_ID_DELETE nVId=$1
}
###############################################################################################################
# Function to set pvid of a specified port
# Arguements:
# $1 Portid <0-11>
# $2 VLANid <1-4096>
# $3 TVM (Transparent VLAN Mode) <0/1>
###############################################################################################################
config_port_pvid() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_SET nPortId=$1 nPortVId=$2 bVLAN_UnknownDrop=0 bVLAN_ReAssign=0 eVLAN_MemberViolation=$4 eAdmitMode=0 bTVM=$3
}

###############################################################################################################
# Function to reset the port to initial state
# Arguements:
# $1 portid <0-11>
###############################################################################################################
reset_port_pvid() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_SET nPortId=$1 nPortVId=0 bVLAN_UnknownDrop=0 bVLAN_ReAssign=0 eVLAN_MemberViolation=0 eAdmitMode=0 bTVM=0
}

###############################################################################################################
# Function to set add a port as member to a specified VLAN
# Arguements:
# $1 VLANid <1-4096>
# $2 Portid <0-11>
# $3 Egress tagging <0/1>
###############################################################################################################
config_port_member() {
      local vid=$1
      local port=$2
      local egress_tag=$3
      switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_ADD nVId=$vid nPortId=$port bVLAN_TagEgress=$egress_tag
}

###############################################################################################################
# Function to set remove a port from a vlan membership
# Arguements:
# $1 VLANid <1-4096>
# $2 Portid <0-11>
###############################################################################################################
reset_port_member() {

      local vid=$1
      local port=$2
      switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE nVId=$vid nPortId=$port
}


##############################################################################################################
# Function to separate LAN and WAN interrupts (DMA channels) i.e LAN : 129 and WAN : 64
# Applicable to SMP models ONLY
# By default all the ports are served by DMA 129. Here we set the traffic class of WAN and WLAN ports to 3,
# this enabled the traffic to be served using DMA 64
##############################################################################################################
split_lan_wan_dma_channel() {
if [ -n $1 -a $1 -eq 1 ]; then
  if [ -n "$CONFIG_PACKAGE_KMOD_SMVP" -a "$CONFIG_PACKAGE_KMOD_SMVP" = "1" ]; then
    port_idx=58
    for i in $wlan_port; do
      switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=$port_idx pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$i action.eTrafficClassAction=2 action.nTrafficClassAlternate=3
      port_idx=`expr $port_idx + 1`
    done
  
    port_idx=61
    for i in $wan_port; do
      switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=$port_idx pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$i action.eTrafficClassAction=2 action.nTrafficClassAlternate=3
      port_idx=`expr $port_idx + 1`
    done
  fi
else
  if [ -n "$CONFIG_PACKAGE_KMOD_SMVP" -a "$CONFIG_PACKAGE_KMOD_SMVP" = "1" ]; then
    port_idx=58
    for i in $wlan_port; do
      switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=$port_idx
      port_idx=`expr $port_idx + 1`
    done
  
    port_idx=61
    for i in $wan_port; do
      switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=$port_idx
      port_idx=`expr $port_idx + 1`
    done
  fi
fi
}

###############################################################################################################
# Function to config GPHY LED
###############################################################################################################
config_led()
{

  [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ] && {
    # AR10 and EASY330 platfrom

    for port in $lan_port; do
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e2 nData=0x42
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e3 nData=0x10
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e4 nData=0x70
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=$port nAddressReg=0x1F01e5 nData=0x03
    done
    [ "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ] && {
      # EASY330 platfrom
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e2 nData=0x70
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e3 nData=0x03
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e4 nData=0x42
      switch_cli  IFX_ETHSW_MMD_DATA_WRITE nAddressDev=0x2 nAddressReg=0x1F01e5 nData=0x10
    }
  }
}

###############################################################################################################
# Initializes the Switch ports for the selected WAN mode
# ALL LAN ports will be set with PVID 501
# ALL LAN ports, WLAN ports and CPU port will be member of VLAN 501
# ALL WAN ports will be set with PVID 502
# ALL WAN ports and CPU port will be member of VLAN 502
# CPU Port will be set with PVID 500
# ALL LAN ports, WLAN Ports, WAN ports & CPU port will be member of VLAN 500
###############################################################################################################
switch_init() {

  local maxPacketLen=1536
  output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_GET nPortId=$lan_port_1| grep -w nPortVId | awk '{ print $2 }'`
  
  if [ $output = $lan_vid_all -o $output = $lan_vid_port_start ]; then
    switch_uninit
  fi
  
  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_DISABLE

  if [ "$CONFIG_FEATURE_MINI_JUMBO_FRAMES" == "1" ] ; then 
    #
    # When mini jumbo frames feature is enabled the larger size packets should
    # pass through switch. Default packet len configured in switch is 1536. In
    # case of mini jumbo frames switch shall support packets upto 1600 bytes.
    #
    maxPacketLen=1600
  fi

  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_CFG_SET eMAC_TableAgeTimer=3 bVLAN_Aware=1 \
             nMaxPacketLen=$maxPacketLen bPauseMAC_ModeSrc=0 nPauseMAC_Src=00:00:00:00:00:00        
  
  #create the VLAN id
  create_vlanid $wan_vid $wan_fid
  create_vlanid $cpu_vid $cpu_fid
  create_vlanid $lan_vid_all $lan_fid

  # Setting the PVID 502 for WAN ports
  # TVM is set to 0 in case of ETH WAN port and 1 in case of xDSL WAN port; because ATM and PTM will have some VLANs which is used internally by PPA
  # That means if a tagged packet comes from wan side we must have corresponding VLAN created in Switch which has both WAN port and CPU port as member and 
  # egress tagging enabled on CPU port for that VLAN.
        for i in $wan_port; do
     if [ $i = $wan_port_mii1 -o $i = $wan_port_mii0 ]; then
       #for ethwan we need to set TVM of wanport to 0
  if  [ "$CONFIG_FEATURE_EEE" = "1" ]; then
       switch_cli IFX_ETHSW_PORT_LINK_CFG_SET nPortId=$i  bLPI=1       
  fi
    config_port_pvid $i $wan_vid 0 3
     else
             config_port_pvid $i $wan_vid 1 3
     fi
        done

   # Setting PVID of CPU port as 500  
        config_port_pvid $cpu_port $cpu_vid 0 2

  # Setting PVID of LAN ports as 501
  for i in $lan_port_all; do
          config_port_pvid $i $lan_vid_all 1 3
        done  

  # ALL LAN ports, WLAN Ports, WAN ports & CPU port will be member of CPU VLAN 500
  for i in $lan_port_all $cpu_port $wan_port; do
    config_port_member $cpu_vid $i 0
  done
  
  # ALL LAN ports, WLAN ports and CPU port will be member of VLAN 501
  for i in $lan_port_all $cpu_port; do
    config_port_member $lan_vid_all $i 0
  done

  # ARX188 external switch is currently not supporting the vlan config on virutal ports 7 & 8  
  if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
    for i in $wlan_port; do
      config_port_pvid $i $lan_vid_all 1 3
      config_port_member $cpu_vid $i 0
      config_port_member $lan_vid_all $i 0
    done
  fi
  
  # ALL WAN ports and CPU port will be member of VLAN 502
  for i in $wan_port $cpu_port; do
    config_port_member $wan_vid $i 0
  done
  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_ENABLE
  if  [ "$CONFIG_FEATURE_EEE" = "1" ]; then
  for i in $lan_port_all; do 
        switch_cli IFX_ETHSW_PORT_LINK_CFG_SET nPortId=$i  bLPI=1       
  done     
  fi
  # spilt the lan and wan interrupts (DMA channels)
  split_lan_wan_dma_channel 1
	
  # Disable pause frame for cpu port
  switch_cli IFX_FLOW_REGISTER_SET nRegAddr=0x94b nData=0x1c0

  # Configure reserved buffers and global WRED configuration for GRX330 platform
  init_sw_cfg_for_grx330_plat

  config_led
  echo "Switch Init done "
  #switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_READ 
}

###############################################################################################################
# Function to reset the switch configurations done using switch_init
###############################################################################################################
switch_uninit() {

  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_DISABLE
  
  #reset the switch to re-configure
  if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
#    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_HW_INIT
       delete_vlanid $lan_vid_all
        delete_vlanid $cpu_vid
        delete_vlanid $wan_vid
  else
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_HW_INIT eInitMode=0
  fi  
  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_ENABLE
}

###############################################################################################################
# Creates Vconfig interface on WAN (eth0.502) for MII0 untagged packets. 
# Creates Vconfig interface for LAN side packets (eth0.501)
# Enables egress tagging on CPU port for VLAN 502 and 501
# All Packets from the WAN side without VLAN tag will be received on eth0.502
# All Packets from LAN side will be received on eth0.501
###############################################################################################################
enable_mii0_conf() {

  # If LAN side Port Separation is enabled no need to create eth0.501 as the LAN Packets are already isolated
  if [ "$lan_port_sep_enable" != "1" ]; then
          vconfig add eth0 $lan_vid_all;
          ifconfig eth0.$lan_vid_all up
    config_port_member $lan_vid_all $cpu_port 1
          brctl addif br0 eth0.$lan_vid_all
    # the below line needs to be added just after the call to this function for registering the interface with PPA
    #. /etc/rc.d/ppa_config.sh addlan eth0.$lan_vid_all
  fi
        vconfig add eth0 $wan_vid;
        ifconfig eth0.$wan_vid up
  #add wan interface to ppa?
  config_port_member $wan_vid $cpu_port 1

}

###############################################################################################################
# Deletes the Vconfig interface for MII0 untagged packets 
# Disables egress tagging on CPU port for VLAN 502 and 501
###############################################################################################################
disable_mii0_conf() {

  if [ "$lan_port_sep_enable" != "1" ]; then
    # the below line needs to be added just before the call to this function for de-registering the interface with PPA
    # . /etc/rc.d/ppa_config.sh dellan eth0.$lan_vid_all
    vconfig rem eth0.$lan_vid_all; 
    config_port_member $lan_vid_all $cpu_port 0
  fi
  vconfig rem eth0.$wan_vid;
  config_port_member $wan_vid $cpu_port 0

}


###############################################################################################################
# configures MII1 as lan port
# since we are setting bLearningMAC_PortLock on mii1 port the mac address learned on mii1 port will not be re-learned on any other port 
# so if you wan to move one lan pc from this port to any other lan port we need to wait 3 minutes until the MAC table entry times out or give the following command
###############################################################################################################
add_mii1_to_lan() {
  
  select_wan_mode
  if [ config_wan != "2" -a sec_wan != "2" ]; then
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_FLOW_REGISTER_SET nRegAddr=0xCCD nData=$wan_reg_mii1
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_PORT_CFG_SET nPortId=$wan_port_mii1 eEnable=1 bLearningMAC_PortLock=1 
    ifconfig eth1 up   
    brctl addif br0 eth1
    # the below line needs to be added just after the call to this function for de-registering the interface with PPA
    #. /etc/rc.d/ppa_config.sh addlan eth1
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MAC_TABLE_CLEAR
  else
    echo "mii1 is currently in WAN mode!!!"
  fi

}
###############################################################################################################
# removes mii1 from lan
# removes the eth1 interface from bridge
###############################################################################################################
del_mii1_frm_lan() {
  select_wan_mode
  if [ config_wan != "2" -a sec_wan != "2" ]; then
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_PORT_CFG_SET nPortId=$wan_port_mii1 eEnable=1 bLearningMAC_PortLock=0    
    brctl delif br0 eth1
    ifconfig eth1 down
    # the below line needs to be added just before the call to this function for de-registering the interface with PPA
    #. /etc/rc.d/ppa_config.sh dellan eth1
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MAC_TABLE_CLEAR
  else
    echo "mii1 is currently in WAN mode!!!"
  fi

}
###############################################################################################################
# Creates vconfig interface on WAN port (eq: eth1.502)
# Enables egress tagging on CPU port for VLAN 502
# All Packets from the WAN side without VLAN tag will be received on wan.502
# Enables Simultainious routing and software bridging in MII1 mode
# It takes base wan interface name as input
###############################################################################################################
enable_multiwan_conf() {

        #we need to create a wan vlan interface, 
  #the user need to create the logical interfaces like macvlan interface on top of wan.
        vconfig add eth1 $wan_vid;
        ifconfig eth1.$wan_vid up
  config_port_member $wan_vid $cpu_port 1

}

###############################################################################################################
# Disable the multiwan configuration
# Deletes the vconfig interface and disable egress tagging on CPU port for internal WAN VLAN (502)
# It takes base wan interface name as input
###############################################################################################################
disable_multiwan_conf() {

      vconfig rem eth1.$wan_vid;
      config_port_member $wan_vid $cpu_port 0

}

###############################################################################################################
# Create a wan vlan interface and adds the required port membership
# Takes a wan vlan interface name as input and creates the vconfig interface and required switch configurations
# eg: eth1.601 or ptm0.801
###############################################################################################################
add_wanvlan_if() {
  local ifname=$(echo $1 | cut -d "." -f1)
  local vlan=$(echo $1 | cut -d "." -f2)
  select_wan_mode_accel
  
  if [ "A${ifname}A" = "AA" -o "A${vlan}A" = "AA" ]; then
    echo "WAN interface is not valid!!!"
  else
    if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" != "11" ]; then
    	vconfig add $ifname $vlan;
	ifconfig $ifname.$vlan 0.0.0.0 up
    fi
    create_vlanid $vlan $wan_fid  
    for i in $wan_port $cpu_port; do
          config_port_member $vlan $i 1
    done
  fi
}
###############################################################################################################
# Deletes the wan vlan interface and switch configurations for the same
# takes the wan vlan interface name as input and deletes the vconfig interface & switch configuration
# eg: eth1.601 or ptm0.801
###############################################################################################################
del_wanvlan_if() {
  local vlan=$(echo $1 | cut -d "." -f2)
  if [ "A${vlan}A" = "AA" ]; then
    echo "WAN interface is not valid!!!"
  else
    if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" != "11" ]; then
    	vconfig rem $1;
    fi
    delete_vlanid $vlan
  fi
}
################################################################################################################
# ppa uses some vlans to send downstream data from virtual interfaces
# This function adds vlan configuration for a nas interface
# arguement: nas interface name to be added
###############################################################################################################
add_atm_routing_acceleration() {
  select_wan_mode_accel 
 
  local vpivci=$(sed -n '/'$1'/,/vcc/p' /proc/net/atm/br2684|grep vcc|sed -e 's/.*vcc 0./\1/' -e 's/:.*/\1/')
  if [ "A${vpivci}A" != "AA" ]; then
  	if [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ]; then
    		wanvlan=$(grep $vpivci /proc/eth/vrx318/dsl_vlan | awk '{print $5}') 
  	else
 
    		wanvlan=$(grep $vpivci /proc/eth/dsl_vlan | awk '{print $5}') 
	fi
  fi

  create_vlanid $wanvlan $wan_fid 
      for i in $wan_port; do
    config_port_member $wanvlan $i 1
      done
  config_port_member $wanvlan $cpu_port 1
}

##############################################################################################################
# ppa uses some vlans to send downstream data from virtual interfaces
# This function adds vlan configuration for a nas interface
# arguement: nas interface name to be added
###############################################################################################################
del_atm_routing_acceleration() {

  local vpivci=$(sed -n '/'$1'/,/vcc/p' /proc/net/atm/br2684|grep vcc|sed -e 's/.*vcc 0./\1/' -e 's/:.*/\1/')
  if [ "A${vpivci}A" != "AA" ]; then
  	if [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ]; then
    		wanvlan=$(grep $vpivci /proc/eth/vrx318/dsl_vlan | awk '{print $5}') 
  	else
    		wanvlan=$(grep $vpivci /proc/eth/dsl_vlan | awk '{print $5}') 
  	fi
  fi
  delete_vlanid $wanvlan 
}

###############################################################################################################
# Enables Bridging acceleration between LAN & WAN 
# The user needs to specify a LAN interacce & bridged WAN interface name to enable the bridging of packets 
# between specified LAN port and WAN port
# IMPORTANT NOTE: ATM WAN can be plain nasX
# input arguements
# $1 = wan interface name (eg: ptm0.800 or eth1.100 or nas2 )
# $2 = lan interface names (eg: eth0.2,eth0.3)
# $3 = bridge name (eg: br0 or br1)
###############################################################################################################
enable_bridge_accel() {
  local waniface
  local wanvlan
  select_wan_mode_accel

  if [ "${1::3}" = "nas" ]; then 
    local vpivci=$(sed -n '/'$1'/,/vcc/p' /proc/net/atm/br2684|grep vcc|sed -e 's/.*vcc 0./\1/' -e 's/:.*/\1/')
    if [ "A${vpivci}A" != "AA" ]; then
      waniface=$1
	if [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ]; then
    	   wanvlan=$(grep $vpivci /proc/eth/vrx318/dsl_vlan | awk '{print $5}') 
  	else

           wanvlan=$(grep $vpivci /proc/eth/dsl_vlan | awk '{print $5}') 
	fi
    fi
  else  
      waniface=$(echo $1 | cut -d "." -f1)
      wanvlan=$(echo $1 | cut -d "." -f2)
  fi

  local bridgemac=$(/sbin/ifconfig $3 | sed -e's/^.*HWaddr \([^ ]*\) .*$/\1/;t;d')

  #for handling non vlan wan interface ptm0 & eth1  
  if [ "$wanvlan" = "$waniface" ] ; then
    if [ "${waniface}" = "ptm0" -o "${waniface}" = "eth1" ]; then
      wanvlan=$wan_vid
    fi
  fi
  
  if [ "A${waniface}A" = "AA" -o "A${wanvlan}A" = "AA" ]; then
    echo "WAN interface is not valid!!!"
  elif [ "A${bridgemac}A" = "AA" ]; then
                echo "Invalid bridge name!!!"
  else
      if [ "${waniface::3}" = "ptm" ]; then
      # once the below statement is called all the routed wan connections will on ptmX.* will stop working
      # please ensure that all the routed wans calls add_wan_vlanif() while wan creation
      # disable tvm on wan if
      if [ "$wanvlan" -ne "$wan_vid" ]; then
            create_vlanid $wanvlan $wan_fid 
      fi
            for i in $wan_port; do
                config_port_pvid $i $wan_vid 0 3
        if [ "$wanvlan" -ne "$wan_vid" ]; then
          config_port_member $wanvlan $i 1
        else
          config_port_member $wanvlan $i 0
        fi
            done
      elif [ "${waniface::3}" = "nas" ]; then
          create_vlanid $wanvlan $wan_fid 
          #once the below statement is called all the routed wan connections will on nas* will stop working
           #we need following vlan configrations to be made for all the routed wans to make it work
          #1. call 'add_nasif_vlan' all the routed nas interfaces
          for i in $wan_port; do
            config_port_pvid $i $wan_vid 0 3
        config_port_member $wanvlan $i 1
          done
    fi

    k=0
    m=0
    for lan_if in $( echo $2 | sed -n 1'p' | tr ',' '\n'); do
      if [ "${lan_if::4}" = "eth0" ]; then
              output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_GET nPortId=$lan_port_1| grep -w nPortVId | awk '{ print $2 }'`
        if [ "${lan_if:4:1}" = "." ]; then
          #vlan based port separation
	  if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" = "11" ]; then
	    	local lanvlan=$(echo $lan_if | cut -d "." -f2)
                  j=0

                for i in $lan_port; do
                  tvlan=`expr $software_vid_port_start \+ $j`
                  if [ $tvlan = $lanvlan ]; then
                        nvlan_id=`expr $lan_vid_port_start \- $software_vid_port_start \+ $lanvlan`
                        nport_id=$i
                        break
                  fi
                  j=`expr $j \+ 1`
                  k=`expr $k \+ 3`
                done
	   else
                 if [ "$output" != "$lan_vid_port_start" ]; then
		       echo "Ports not separated, can't proceed"
		       break
                 else
                       local lanvlan=$(echo $lan_if | cut -d "." -f2)
                       j=0
        
                for i in $lan_port; do
                      tvlan=`expr $software_vid_port_start \+ $j`
                      if [ $tvlan = $lanvlan ]; then
                nvlan_id=`expr $lan_vid_port_start \- $software_vid_port_start \+ $lanvlan`
                nport_id=$i
                break
                 fi
                  j=`expr $j \+ 1`
                  k=`expr $k \+ 3`
                  done
              fi
           fi
        elif [ "${lan_if:4:1}" = "_" ]; then
          #driver based lan port separation
	     if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" = "11" ]; then
		local lanvlan=$(echo $lan_if | cut -d "_" -f2)
                j=1

                for i in $lan_port; do
                  if [ $j = $lanvlan ]; then
                    nvlan_id=$lan_vid_all
                    nport_id=$i
                break
                  fi
                  j=`expr $j \+ 1`
                  k=`expr $k \+ 3`
                done
	    else
              if [ "$output" = "$lan_vid_all" ]; then
                local lanvlan=$(echo $lan_if | cut -d "_" -f2)
                j=1

                for i in $lan_port; do
                if [ $j = $lanvlan ]; then
                    nvlan_id=$lan_vid_all
                    nport_id=$i
                break
                  fi
                  j=`expr $j \+ 1`
                  k=`expr $k \+ 3`
                done
              else
              	echo "Ports are separated, cannot proceed"
                break
              fi
	    fi
        else
          #no lan port separation
              if [ "$output" = "$lan_vid_all" ]; then
                nvlan_id=$lan_vid_all
                nport_id=$lan_port
              else
            echo "Ports are separated, cannot proceed"
                                          break
              fi
        fi
      elif [ "${lan_if::4}" = "wlan" -o "${lan_if::3}" = "ath" ]; then
        #wireless ports needs to be read from ppa/api/directpath
        #ifid + 4 = portid
        nvlan_id=$lan_vid_all
        nport_id=$(echo `sed -n '/'${lan_if::5}'/,/rx_fn_rxif_pkt/p' /proc/ppa/api/directpath` | awk '{print $7}')
        if [ $nport_id -gt 0 ]; then
          nport_id=`expr $nport_id \+ 4`
        else
          echo "Invalid wireless interface!!!"
          continue
        fi

      fi

      if [ "A${nvlan_id}A" = "AA" -o "A${nport_id}A" = "AA" ]; then
        echo "Invalid lan port!!!" $lan_if
        break
      else
        if [ "${waniface::3}" = "nas" ]; then
            #for ATM WAN ipstream traffic still goes through CPU path.
            for i in $nport_id; do
              config_port_member $wanvlan $i 0
            done
        else
            reset_port_member $wanvlan $cpu_port
          
            for i in $nport_id; do
          config_port_pvid $i $wanvlan 0 3
          config_port_member $wanvlan $i 0
	  if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" != "11" ]; then
          switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ $k` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$i pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=FF:FF:FF:FF:FF:FF action.eVLAN_Action=2 action.nVLAN_Id=$nvlan_id
              switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ $k \+ 1` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$i pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=$bridgemac action.eVLAN_Action=2 action.nVLAN_Id=$nvlan_id
              switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ $k \+ 2` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$i pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=01:00:5E:00:00:00 pattern.nMAC_DstMask=0x03F action.eVLAN_Action=2 action.nVLAN_Id=$nvlan_id
          k=`expr $k + 3`
	  fi
            done

	if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" = "11" ]; then
		eval pce_rule_start=$bridge_accel_Order

          	config_port_member $wanvlan $cpu_port 1
      		
		if [ "${waniface::3}" = "ptm" ]; then
			switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start` pattern.bEnable=1 pattern.bVid=1 pattern.nVid=$wanvlan pattern.bPortIdEnable=1 pattern.nPortId=11 pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=01:00:5E:00:00:00 pattern.nMAC_DstMask=0x03F action.ePortMapAction=4 action.nForwardPortMap=0x40 
			switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 1` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=11 pattern.bVid=1 pattern.nVid=$wanvlan action.ePortMapAction=4 action.nForwardPortMap=0x1F action.eVLAN_Action=2 action.nVLAN_Id=$wanvlan
      		else
			if [ "${waniface::3}" = "nas" ]; then
				switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start` pattern.bEnable=1 pattern.bVid=1 pattern.nVid=$wanvlan pattern.bPortIdEnable=1 pattern.nPortId=10 pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=01:00:5E:00:00:00 pattern.nMAC_DstMask=0x03F action.ePortMapAction=4 action.nForwardPortMap=0x40 
				switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 1` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=10 pattern.bVid=1 pattern.nVid=$wanvlan action.ePortMapAction=4 action.nForwardPortMap=0x1F action.eVLAN_Action=2 action.nVLAN_Id=$wanvlan
			else
				switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start` pattern.bEnable=1 pattern.bVid=1 pattern.nVid=$wanvlan pattern.bPortIdEnable=1 pattern.nPortId=$wan_port pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=01:00:5E:00:00:00 pattern.nMAC_DstMask=0x03F action.ePortMapAction=4 action.nForwardPortMap=0x40 
				switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 1` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$wan_port pattern.bVid=1 pattern.nVid=$wanvlan action.ePortMapAction=4 action.nForwardPortMap=0x1F action.eVLAN_Action=2 action.nVLAN_Id=$wanvlan
      			fi
		fi

		switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 2` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$cpu_port pattern.bVid=1 pattern.nVid=$wanvlan action.ePortMapAction=4 action.nForwardPortMap=0xFF action.eVLAN_Action=2 action.nVLAN_Id=$wanvlan

		switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 3` pattern.bEnable=1 pattern.bVid=0 pattern.nVid=$wanvlan pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=FF:FF:FF:FF:FF:FF action.eVLAN_Action=2 action.nVLAN_Id=$nvlan_id
		switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 4` pattern.bEnable=1 pattern.bVid=0 pattern.nVid=$wanvlan pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=$bridgemac action.eVLAN_Action=2 action.nVLAN_Id=$nvlan_id
		switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ 5` pattern.bEnable=1 pattern.bVid=0 pattern.nVid=$wanvlan pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=01:00:5E:00:00:00 pattern.nMAC_DstMask=0x03F action.eVLAN_Action=2 action.nVLAN_Id=$nvlan_id
	fi
        fi
          fi
    done
       switch_cli IFX_ETHSW_MAC_TABLE_CLEAR
    # tbd: if we need to access the dut from bridged wan we have to add the below rule on the wan port
    # switch_cli IFX_FLOW_PCE_RULE_WRITE pattern.nIndex=`expr $pce_rule_start \+ $k` pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=$wan_port pattern.bMAC_DstEnable=1 pattern.nMAC_Dst=$bridgemac action.eVLAN_Action=2 action.nVLAN_Id=$wanvlan
  fi
}

###############################################################################################################
# Disables Bridging acceleration between LAN & WAN 
# The user needs to specify a LAN port & bridged WAN interface name disable bridging acceleration between 
# specified LAN port and WAN port
# input arguements
# $1 = wan interface name (eg: ptm0.800 or eth1.100 or nas2 )
# $2 = lan interface names (eg: eth0.2,eth0.3)
###############################################################################################################
disable_bridge_accel() {
  local waniface
  local wanvlan
  
  if [ "${1::3}" = "nas" ]; then 
    local vpivci=$(sed -n '/'$1'/,/vcc/p' /proc/net/atm/br2684|grep vcc|sed -e 's/.*vcc 0./\1/' -e 's/:.*/\1/')
    if [ "A${vpivci}A" != "AA" ]; then
        waniface=$1
   	if [ "$CONFIG_TARGET_LANTIQ_XRX300" = "1" -o "$CONFIG_TARGET_LANTIQ_XRX330" = "1" ]; then
    		wanvlan=$(grep $vpivci /proc/eth/vrx318/dsl_vlan | awk '{print $5}') 
  	else
 
        	wanvlan=$(grep $vpivci /proc/eth/dsl_vlan | awk '{print $5}') 
	fi
    fi
  else  
    waniface=$(echo $1 | cut -d "." -f1)
    wanvlan=$(echo $1 | cut -d "." -f2)
  fi

  #for handling non vlan wan interface ptm0 & eth1  
  if [ "$wanvlan" = "$waniface" ] ; then
    if [ "${waniface}" = "ptm0" -o "${waniface}" = "eth1" ]; then
      wanvlan=$wan_vid
    fi
  fi


  if [ "A${waniface}A" = "AA" -o "A${wanvlan}A" = "AA" ]; then
    echo "WAN interface is not valid!!!"
  else 
    j=0
       if [ "${waniface::3}" = "ptm" ]; then
      for i in $wan_port; do
            config_port_pvid $i $wan_vid 1 3
          done
          if [ "$wanvlan" -ne "$wan_vid" ]; then
              delete_vlanid $wanvlan
          fi
      elif [ "${waniface::3}" = "nas" ]; then
       delete_vlanid $wanvlan
          for i in $wan_port; do
            config_port_pvid $i $wan_vid 1 3
          done
    fi

    k=0
    m=0
    for lan_if in $( echo $2 | sed -n 1'p' | tr ',' '\n'); do
      if [ "${lan_if::4}" = "eth0" ]; then
        if [ "${lan_if:4:1}" = "." ]; then
          #vlan based port separation
                      local lanvlan=$(echo $lan_if | cut -d "." -f2)
                j=0
        
            for i in $lan_port; do
                    tvlan=`expr $software_vid_port_start \+ $j`
                    if [ $tvlan = $lanvlan ]; then
              nvlan_id=`expr $lan_vid_port_start \- $software_vid_port_start \+ $lanvlan`
              nport_id=$i
              break
                fi
                j=`expr $j \+ 1`
                k=`expr $k \+ 3`
                 done
        elif [ "${lan_if:4:1}" = "_" ]; then
          #driver based lan port separation
              local lanvlan=$(echo $lan_if | cut -d "_" -f2)
              j=1

                for i in $lan_port; do
                if [ $j = $lanvlan ]; then
                  nvlan_id=$lan_vid_all
                  nport_id=$i
              break
                fi
                j=`expr $j \+ 1`
               k=`expr $k \+ 3`
             done
        else
          #no lan port separation
             nvlan_id=$lan_vid_all
              nport_id=$lan_port
        fi
      elif [ "${lan_if::4}" = "wlan" -o "${lan_if::3}" = "ath" ]; then
        #wireless ports needs to be read from ppa/api/directpath
        #ifid + 4 = portid
        nvlan_id=$lan_vid_all
        nport_id=$(echo `sed -n '/'${lan_if::5}'/,/rx_fn_rxif_pkt/p' /proc/ppa/api/directpath` | awk '{print $7}')
        if [ $nport_id -gt 0 ]; then
          nport_id=`expr $nport_id \+ 4`
        else
          echo "Invalid wireless interface!!!"
          continue
        fi
      fi

      if [ "A${nvlan_id}A" = "AA" -o "A${nport_id}A" = "AA" ]; then
        echo "Invalid lan port!!!" $lan_if
        break
      else
        if [ "$wanvlan" -ne "$wan_vid" ]; then
             config_port_member $wanvlan $cpu_port 1
        else
          config_port_member $wanvlan $cpu_port 0
        fi
          
            for i in $nport_id; do
                reset_port_member $wanvlan $i
          config_port_pvid $i $nvlan_id 1 3
	  if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" != "11" ]; then
          switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ $k`
          switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ $k \+ 1`
          switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ $k \+ 2`
          k=`expr $k + 3`
	  fi
            done
	
	  if [ "1$CONFIG_FEATURE_BR_ACCEL_SUPPORT" = "11" ]; then
		eval pce_rule_start=$bridge_accel_Order

	  switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start`
	  switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ 1`
          switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ 2`
          switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ 3`
          switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ 4`
	  switch_cli IFX_FLOW_PCE_RULE_DELETE nIndex=`expr $pce_rule_start \+ 5`
	  fi

          fi
    done
        switch_cli IFX_ETHSW_MAC_TABLE_CLEAR
  fi
}

###############################################################################################################
# Enable multicast config on all ports
###############################################################################################################
multicast_setting() {

     #remove any currnt port settings
     for i in $lan_port $wlan_port $cpu_port $wan_port; do
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE nPortId=$i
     done
     echo "Setting multicast router port= $wan_port"
     for i in $wan_port; do
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD nPortId=$i
     done

}

###############################################################################################################
# workaround for pppoa to work in VRX adsl atm
###############################################################################################################
enable_pppoa_conf() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MAC_TABLE_ENTRY_ADD nFId=$wan_fid nPortId=6 nAgeTimer=0 bStaticEntry=1 nMAC=00:00:00:00:00:00
}

disable_pppoa_conf() {
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE nFId=$wan_fid nMAC=00:00:00:00:00:00
}

###############################################################################################################
# This Function Enables spearaton of switch ports into 4 different Vconfig interfaces eth0.2 to eth0.5
# This enables any applications running on the DUT to to see four different interfaces on LAN side instead of eth0
# This function needs to be called only after calling the switch_init as it addresses only the LAN side config
###############################################################################################################
enable_separation() {
        
      output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_GET nPortId=$lan_port_1| grep -w nPortVId | awk '{ print $2 }'`
      if [ $output != $lan_vid_port_start ]; then

        switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_DISABLE

  # Creating internal VLAN ids 2050 to 2053
  j=0
  for i in $lan_port; do
      create_vlanid `expr $lan_vid_port_start \+ $j` $lan_fid
      j=`expr $j \+ 1`
        done 
  
        # Creating software VLAN ids 2 to 5
  j=0
        for i in $lan_port; do
      create_vlanid `expr $software_vid_port_start \+ $j` $lan_fid
        j=`expr $j \+ 1`
        done   
        
  # Setting PVIDs for LAN ports with internal vids 2050 to 2053
  j=0
        for i in $lan_port; do
          config_port_pvid $i `expr $lan_vid_port_start \+ $j` 1 3
      j=`expr $j \+ 1`
        done   

  # Making all the LAN Ports and WLAN Ports member of each of the internal VLANs (2050 - 2053)
  if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
      # ARX188 external switch is currently not supporting the vlan config on virutal ports 7 & 8  
      for i in $lan_port_all $wlan_port; do
    j=0
      for k in $lan_port; do
            config_port_member `expr $lan_vid_port_start \+ $j` $i 0
        j=`expr $j \+ 1`
     done
            done
  else
      for i in $lan_port_all; do
    j=0
     for k in $lan_port; do
                 config_port_member `expr $lan_vid_port_start \+ $j` $i 0
        j=`expr $j \+ 1`
     done
            done
  fi
        
  # Making CPU port member of each of the internal VLANs (2050 - 2053)
  j=0
  for i in $lan_port; do 
            config_port_member `expr $lan_vid_port_start \+ $j` $cpu_port 1
      j=`expr $j \+ 1`
        done 

  # Making CPU port and ONE LAN port as member of each SW VLANs 
  j=0 
  for i in $lan_port; do
            config_port_member `expr $software_vid_port_start \+ $j` $i 0
            config_port_member `expr $software_vid_port_start \+ $j` $cpu_port 0
      j=`expr $j \+ 1`
        done 

        # Creating the Vconfig interfaces and adding them to bridge and PPA 
  j=0
        for i in $lan_port; do
             vconfig add eth0 `expr $software_vid_port_start \+ $j`;
             ifconfig eth0.`expr $software_vid_port_start \+ $j` up
             brctl addif br0 eth0.`expr $software_vid_port_start \+ $j`
      . /etc/rc.d/ppa_config.sh addlan eth0.`expr $software_vid_port_start \+ $j`
      j=`expr $j \+ 1`
        done 
  # adding the base interface to ppa
  . /etc/rc.d/ppa_config.sh addlan eth0
  # removing eth0 from bridge
        brctl delif br0 eth0
  
  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_ENABLE
        #switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_READ 
        echo "Port Separation Enabled !"
     else
        echo "++++++++++++++ Port separateion already enabled +++++++++++++"
     fi

}

###############################################################################################################
# This Function Enables spearaton of switch ports into 4 different virtual interfaces eth0_1 to eth0_4
# This enables any applications running on the DUT to to see four different interfaces on LAN side instead of eth0
# This function needs to be called only after calling the switch_init as it addresses only the LAN side config
###############################################################################################################
enable_driver_separation() {
if [ "$CONFIG_PACKAGE_QUANTENNA_TYPE_TWO_RGMII" = "1" ]; then
  # Switch Trunking must be enabled on port 0 and 5 for Quantenna 802.11ac. In this case, driver separation must be disable 
  echo "!!! LAN port driver separation can't be supported when switch trunking is needed !!!"
else
  insmod /lib/modules/`uname -r`/lantiq_ethsw.ko
  j=0
  brctl delif br0 eth0
        for i in $lan_port; do
    eval interface_name='$'lan_port_${j}_interface
    ip link add dev $interface_name link eth0 type ethsw ports $i
    ifconfig $interface_name 0.0.0.0 up
    brctl addif br0 $interface_name
    . /etc/rc.d/ppa_config.sh addlan "$interface_name -l eth0" 
    j=`expr $j \+ 1`
        done 
fi
} 
###############################################################################################################
# This Function Disables the LAN side Port separation and re-initializes the switch.
###############################################################################################################
disable_separation() {

      output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_GET nPortId=$lan_port_1| grep -w nPortVId | awk '{ print $2 }'`
      if [ $output = $lan_vid_port_start ]; then
        switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_DISABLE

        switch_init
  
  #ARX188 deletion will be handled in switch_init    
  if [ "$CONFIG_IFX_CONFIG_CPU" != "AMAZON_S" ]; then
    # delete lan port vids separation
    j=0
          for i in $lan_port; do
              delete_vlanid `expr $lan_vid_port_start \+ $j`
        j=`expr $j \+ 1`
          done   
        
    # delete software vids
    j=0
          for i in $lan_port; do
               delete_vlanid `expr $software_vid_port_start \+ $j`
        j=`expr $j \+ 1`
          done 
  fi  
        

  j=0
  for i in $lan_port; do
       . /etc/rc.d/ppa_config.sh  dellan eth0.`expr $software_vid_port_start \+ $j`
             vconfig rem eth0.`expr $software_vid_port_start \+ $j`;
       j=`expr $j \+ 1`
        done
 
        brctl addif br0 eth0

  switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_ENABLE
        #switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_READ 
        echo "Port Separation Disabled !"
      else
         echo "++++++++++++ Port separation already disabled++++++++++++++"
      fi

}

###############################################################################################################
# This Function Disables driver based LAN side Port separation.
###############################################################################################################
disable_driver_separation() {
  j=0
        for i in $lan_port; do
    eval interface_name='$'lan_port_${j}_interface
    brctl delif br0 $interface_name
    ip link delete $interface_name
    . /etc/rc.d/ppa_config.sh dellan $interface_name 
    j=`expr $j \+ 1`
        done 
  brctl addif br0 eth0
  rmmod /lib/modules/`uname -r`/lantiq_ethsw.ko
}


###############################################################################################################
# Enables isolation of a specified port from the rest of the switch
# Any packets from isolated interface will not reach other ports
# it will work only if Lan side port Separation is enabled
###############################################################################################################
enable_port_isolation() {

      output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_GET nPortId=$lan_port_1| grep -w nPortVId | awk '{ print $2 }'`
      if [ $output != $lan_vid_port_start ]; then
         echo "Ports not separated, can't isolate"
      else  
         for i in $lan_port; do
           if [ $i = $1 ]; then
             echo "Isolating port $i"
             for i in $lan_port; do
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE nVId=`expr $lan_vid_port_start \+ $1` nPortId=$i
             done
           fi     
         done
      fi

}

###############################################################################################################
# Disables Isolation which is enabled using enable_port_isolation
###############################################################################################################
disable_port_isolation() {

      output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_CFG_GET nPortId=$lan_port_1| grep -w nPortVId | awk '{ print $2 }'`
      if [ $output != $lan_vid_port_start ]; then
         echo "Ports not separated, can't join"
      else  
         for i in $lan_port; do
           if [ $i = $1 ]; then
             echo "Joining port $i"
             for i in $lan_port; do
    switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_ADD nVId=`expr $lan_vid_port_start \+ $1` nPortId=$i bVLAN_TagEgress=0
             done
           fi     
         done
      fi
}

###############################################################################################################
# Enables tracing of all the switch_cli calls
# Once this is enabled any calls to switch_cli will be printed on the console with arguements passed
###############################################################################################################
enable_trace() {
   if [ -f /usr/bin/switch_cli_bak ]; then
      echo "switch trace mode already enabled"
   else
      touch /usr/bin/switchdbg 2> /dev/null  
      if [ ! -f /usr/bin/switchdbg ]; then
        mini_fo.sh mount /usr/bin/
      fi
      mv /usr/bin/switch_cli /usr/bin/switch_cli_bak
      echo "echo switch_cli \$@" >> /usr/bin/switch_cli
      echo "switch_cli_bak \$@" >> /usr/bin/switch_cli
      chmod +x /usr/bin/switch_cli
   fi
}

###############################################################################################################
# Disable the logging functionality of switch_cli
###############################################################################################################
disable_trace() {
   if [ -f /usr/bin/switch_cli_bak ]; then
      mv /usr/bin/switch_cli_bak /usr/bin/switch_cli
      chmod +x /usr/bin/switch_cli
   else
      echo "switch trace mode already disabled"
   fi
}


###############################################################################################################
# Prints current switch config details for debuging purpose
###############################################################################################################
debug_dump() {
       echo "===========================" 
       echo -e "\n"
       echo "WAN Ports=             $wan_port"
       echo "LAN Ports=             $lan_port" 
       echo "WLAN Ports=            $wlan_port"
       echo -n "CCD          " 
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_FLOW_REGISTER_GET nRegAddr=0xCCD|grep nData
       echo -n "Multicast = " 
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MULTICAST_ROUTER_PORT_READ
       echo "========MAC Table====================="
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MAC_TABLE_ENTRY_READ
       sleep 1
       echo -e "\n"
       echo "========Multicast Table==============="
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ
       echo "========VLAN Membership==============="
       switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_VLAN_PORT_MEMBER_READ 
       echo "============= RMON Counts============="
       for i in $lan_port $cpu_port 5; do
         echo  "+++++++++++++++++++++ Port $i +++++++++++++++++++++"
   switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_ETHSW_RMON_GET nPortId=$i|grep "xGood\|xError\|xDrop\|Filter"
       done
       echo "============= PCE Table============="
       i=0
       while [ $i -le 63 ]
       do
   output=`switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_FLOW_PCE_RULE_READ pattern.nIndex=$i| grep -w pattern.bEnable | awk '{ print $2 }'`
         if [  $output != "FALSE" ]; then
            echo  "+++++++++++++++++++++ Port $i +++++++++++++++++++++"  
      switch_cli dev=$CONFIG_SWITCH_DEVICE_ID IFX_FLOW_PCE_RULE_READ pattern.nIndex=$i
         fi
         i=`expr $i + 1`
       done
}

####################################################################################################################
# This function initializes switch buffers for GRX300 platform.
# Default configuration on this platform is different from other platforms where in,
# 512 switch buffers are available on GRX300 (GSWIP 2.2 and above) compared to 256 on the rest (GSWIP-2.1 and below)
# This configuration is required to address congestion problems due to bursty traffic.
# Buffers reserved for each queue can be tuned for different scenarios.
# Given configuration is based on test cases and scenarios executed by switch team.
####################################################################################################################
init_sw_cfg_for_grx330_plat() {
  if [ "A$CONFIG_TARGET_LANTIQ_XRX330" = "A1" ]; then # [ GRX330 config starts
    # Enable Flow Control eFlowCtrl=3
    i=0
    while [ $i -le 5 ]
    do
      switch_cli IFX_ETHSW_PORT_CFG_SET nPortId=$i eEnable=1 eFlowCtrl=3
      switch_cli IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET nPortId=$i nFlowCtrl_Min=18 nFlowCtrl_Max=30
      i=`expr $i + 1`
    done
  
    # Configure Buffer reservation of each queue to 24 for i 0 31
    i=0
    while [ $i -le 31 ]
    do
      switch_cli IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET nQueueId=$i nBufferReserved=24
      i=`expr $i + 1`
    done
  
    # Configure Global buffer threshold
    switch_cli IFX_ETHSW_QOS_WRED_CFG_SET eProfile=0 nRed_Min=0x3ff nRed_Max=0x3ff nYellow_Min=0x3ff nYellow_Max=0x3ff nGreen_Min=0x100 nGreen_Max=0x100
  
    # Configure Global flowcontrol  threshold buffer
    switch_cli IFX_ETHSW_QOS_FLOWCTRL_CFG_SET nFlowCtrlNonConform_Min=0x3ff nFlowCtrlNonConform_Max=0x3ff nFlowCtrlConform_Min=0x3ff nFlowCtrlConform_Max=0x3ff
  fi # GRX330 config ends ]
}


