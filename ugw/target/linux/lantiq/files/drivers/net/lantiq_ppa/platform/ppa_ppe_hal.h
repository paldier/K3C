#ifndef __PPA_PPE_HAL_H__20081104_1318__
#define __PPA_PPE_HAL_H__20081104_1318__



/*******************************************************************************
**
** FILE NAME    : ppa_ppe_hal.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 4 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA PPE Firmware Hardware/Firmware Adaption Layer Header File
*** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 04 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/



/*
 * ####################################
 *              Definition
 * ####################################
 */







#define PPA_IF_NOT_FOUND                            0
#define PPA_IF_TYPE_LAN                             1
#define PPA_IF_TYPE_WAN                             2
#define PPA_IF_TYPE_MIX                             3

#define PPA_ACC_MODE_NONE                           0
#define PPA_ACC_MODE_BRIDGING                       1
#define PPA_ACC_MODE_ROUTING                        2
#define PPA_ACC_MODE_HYBRID                         3

#define PPA_SET_ROUTE_CFG_ENTRY_NUM                 0x01
#define PPA_SET_ROUTE_CFG_MC_ENTRY_NUM              0x02
#define PPA_SET_ROUTE_CFG_IP_VERIFY                 0x04
#define PPA_SET_ROUTE_CFG_TCPUDP_VERIFY             0x08
#define PPA_SET_ROUTE_CFG_TCPUDP_ERR_DROP           0x10
#define PPA_SET_ROUTE_CFG_DROP_ON_NOT_HIT           0x20
#define PPA_SET_ROUTE_CFG_MC_DROP_ON_NOT_HIT        0x40

#define PPA_SET_BRIDGING_CFG_ENTRY_NUM              0x01
#define PPA_SET_BRIDGING_CFG_BR_TO_SRC_PORT_EN      0x02
#define PPA_SET_BRIDGING_CFG_DEST_VLAN_EN           0x04
#define PPA_SET_BRIDGING_CFG_SRC_VLAN_EN            0x08
#define PPA_SET_BRIDGING_CFG_MAC_CHANGE_DROP        0x10

#define PPA_ROUTE_TYPE_NULL                         0
#define PPA_ROUTE_TYPE_IPV4                         1
#define PPA_ROUTE_TYPE_NAT                          2
#define PPA_ROUTE_TYPE_NAPT                         3

//#define PPA_DEST_LIST_ETH0                          0x01
//#define PPA_DEST_LIST_ETH1                          0x02
//#define PPA_DEST_LIST_CPU0                          0x04
//#define PPA_DEST_LIST_EXT_INT1                      0x08
//#define PPA_DEST_LIST_EXT_INT2                      0x10
//#define PPA_DEST_LIST_EXT_INT3                      0x20
//#define PPA_DEST_LIST_EXT_INT4                      0x40
//#define PPA_DEST_LIST_EXT_INT5                      0x80
//#define PPA_DEST_LIST_ATM                           PPA_DEST_LIST_EXT_INT5  //  EoA

//#define PPA_DEST_LIST_ETH0                          0x0001
//#define PPA_DEST_LIST_ETH1                          0x0002
//#define PPA_DEST_LIST_CPU0                          0x0004
//#define PPA_DEST_LIST_EXT_INT1                      0x0008
//#define PPA_DEST_LIST_EXT_INT2                      0x0010
//#define PPA_DEST_LIST_EXT_INT3                      0x0020
//#define PPA_DEST_LIST_EXT_INT4                      0x0040
//#define PPA_DEST_LIST_EXT_INT5                      0x0080
//#define PPA_DEST_LIST_ATM                           0x0100
//#define PPA_DEST_LIST_NO_REMAP                      (1 << 31)

#define PPA_PHYS_PORT_FLAGS_VALID                   0x0001
#define PPA_PHYS_PORT_FLAGS_TYPE_CPU                0x0000
#define PPA_PHYS_PORT_FLAGS_TYPE_ATM                0x0010
#define PPA_PHYS_PORT_FLAGS_TYPE_ETH                0x0020
#define PPA_PHYS_PORT_FLAGS_TYPE_EXT                0x0030
#define PPA_PHYS_PORT_FLAGS_TYPE_MASK               0x0030
#define PPA_PHYS_PORT_FLAGS_MODE_LAN                0x0100
#define PPA_PHYS_PORT_FLAGS_MODE_WAN                0x0200
#define PPA_PHYS_PORT_FLAGS_MODE_MIX                (PPA_PHYS_PORT_FLAGS_MODE_LAN | PPA_PHYS_PORT_FLAGS_MODE_WAN)
#define PPA_PHYS_PORT_FLAGS_MODE_MASK               0x0300
#define PPA_PHYS_PORT_FLAGS_OUTER_VLAN              0x1000
#define PPA_PHYS_PORT_FLAGS_EXT_CPU0                0x4000
#define PPA_PHYS_PORT_FLAGS_EXT_CPU1                0x8000

#define PPA_PHYS_PORT_FLAGS_MODE_ETH_LAN_VALID      (PPA_PHYS_PORT_FLAGS_TYPE_ETH | PPA_PHYS_PORT_FLAGS_MODE_LAN | PPA_PHYS_PORT_FLAGS_VALID)
#define PPA_PHYS_PORT_FLAGS_MODE_ETH_WAN_VALID      (PPA_PHYS_PORT_FLAGS_TYPE_ETH | PPA_PHYS_PORT_FLAGS_MODE_WAN | PPA_PHYS_PORT_FLAGS_VALID)
#define PPA_PHYS_PORT_FLAGS_MODE_ETH_MIX_VALID      (PPA_PHYS_PORT_FLAGS_TYPE_ETH | PPA_PHYS_PORT_FLAGS_MODE_MIX | PPA_PHYS_PORT_FLAGS_VALID)
#define PPA_PHYS_PORT_FLAGS_MODE_CPU_VALID          (PPA_PHYS_PORT_FLAGS_TYPE_CPU | PPA_PHYS_PORT_FLAGS_VALID)
#define PPA_PHYS_PORT_FLAGS_MODE_ATM_WAN_VALID      (PPA_PHYS_PORT_FLAGS_TYPE_ATM | PPA_PHYS_PORT_FLAGS_MODE_WAN | PPA_PHYS_PORT_FLAGS_VALID)
#define PPA_PHYS_PORT_FLAGS_MODE_EXT_LAN_VALID      (PPA_PHYS_PORT_FLAGS_TYPE_EXT | PPA_PHYS_PORT_FLAGS_MODE_LAN | PPA_PHYS_PORT_FLAGS_VALID)
#define PPA_PHYS_PORT_FLAGS_MODE_EXT_WAN_VALID		(PPA_PHYS_PORT_FLAGS_TYPE_EXT | PPA_PHYS_PORT_FLAGS_MODE_WAN | PPA_PHYS_PORT_FLAGS_VALID)

#define PPA_PPPOE_MODE_TRANSPARENT                  0
#define PPA_PPPOE_MODE_TERMINATION                  1

#define PPA_UPDATE_ROUTING_ENTRY_ROUTE_TYPE         0x0001
#define PPA_UPDATE_ROUTING_ENTRY_NEW_IP             0x0002
#define PPA_UPDATE_ROUTING_ENTRY_NEW_PORT           0x0004
#define PPA_UPDATE_ROUTING_ENTRY_NEW_MAC            0x0008
#define PPA_UPDATE_ROUTING_ENTRY_NEW_SRC_MAC_IX     0x0010
#define PPA_UPDATE_ROUTING_ENTRY_MTU_IX             0x0020
#define PPA_UPDATE_ROUTING_ENTRY_NEW_DSCP_EN        0x0040
#define PPA_UPDATE_ROUTING_ENTRY_NEW_DSCP           0x0080
#define PPA_UPDATE_ROUTING_ENTRY_VLAN_INS_EN        0x0100
#define PPA_UPDATE_ROUTING_ENTRY_NEW_VCI            0x0200
#define PPA_UPDATE_ROUTING_ENTRY_VLAN_RM_EN         0x0400
#define PPA_UPDATE_ROUTING_ENTRY_PPPOE_MODE         0x0800
#define PPA_UPDATE_ROUTING_ENTRY_PPPOE_IX           0x1000
#define PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_INS_EN    0x0100
#define PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_IX        0x0200
#define PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_RM_EN     0x0400
#define PPA_UPDATE_ROUTING_ENTRY_DEST_LIST          0x2000
#define PPA_UPDATE_ROUTING_ENTRY_DEST_QID           0x4000

#define PPA_UPDATE_WAN_MC_ENTRY_VLAN_INS_EN         0x0001
#define PPA_UPDATE_WAN_MC_ENTRY_NEW_VCI             0x0002
#define PPA_UPDATE_WAN_MC_ENTRY_VLAN_RM_EN          0x0004
#define PPA_UPDATE_WAN_MC_ENTRY_SRC_MAC_EN          0x0008
#define PPA_UPDATE_WAN_MC_ENTRY_SRC_MAC_IX          0x0010
#define PPA_UPDATE_WAN_MC_ENTRY_DEST_LIST           0x0020
#define PPA_UPDATE_WAN_MC_ENTRY_DEST_CHID           0x0040
#define PPA_UPDATE_WAN_MC_ENTRY_PPPOE_MODE          0x0080
#define PPA_UPDATE_WAN_MC_ENTRY_OUT_VLAN_INS_EN     0x0100
#define PPA_UPDATE_WAN_MC_ENTRY_OUT_VLAN_IX         0x0200
#define PPA_UPDATE_WAN_MC_ENTRY_OUT_VLAN_RM_EN      0x0400
#define PPA_UPDATE_WAN_MC_ENTRY_NEW_DSCP_EN         0x0800
#define PPA_UPDATE_WAN_MC_ENTRY_NEW_DSCP            0x1000
#define PPA_UPDATE_WAN_MC_ENTRY_DEST_QID            PPA_UPDATE_WAN_MC_ENTRY_DEST_CHID

#define PPA_ADD_MAC_ENTRY_PPPOE                     0x01
#define PPA_ADD_MAC_ENTRY_LAN                       0x02
#define PPA_ADD_MAC_ENTRY_WAN                       0x00

#define PPA_SET_FAST_MODE_CPU1                      0x01
#define PPA_SET_FAST_MODE_APP2                      PPA_SET_FAST_MODE_CPU1
#define PPA_SET_FAST_MODE_ETH1                      0x02
#define PPA_SET_FAST_MODE_ATM                       PPA_SET_FAST_MODE_ETH1

#define PPA_SET_FAST_MODE_CPU1_DIRECT               PPA_SET_FAST_MODE_CPU1
#define PPA_SET_FAST_MODE_CPU1_INDIRECT             0
#define PPA_SET_FAST_MODE_APP2_DIRECT               PPA_SET_FAST_MODE_CPU1_DIRECT
#define PPA_SET_FAST_MODE_APP2_INDIRECT             PPA_SET_FAST_MODE_CPU1_INDIRECT
#define PPA_SET_FAST_MODE_ETH1_DIRECT               PPA_SET_FAST_MODE_ETH1
#define PPA_SET_FAST_MODE_ETH1_INDIRECT             0
#define PPA_SET_FAST_MODE_ATM_DIRECT                PPA_SET_FAST_MODE_ETH1_DIRECT
#define PPA_SET_FAST_MODE_ATM_INDIRECT              PPA_SET_FAST_MODE_ETH1_INDIRECT

//#define PPA_PORT_ETH0                               0x00
//#define PPA_PORT_ETH1                               0x01
//#define PPA_PORT_CPU0                               0x02
//#define PPA_PORT_CPU1_EXT_IF0                       0x05
//#define PPA_PORT_ATM                                0x07
//#define PPA_PORT_ANY                                PPA_PORT_CPU0
//#define PPA_PORT_NUM                                0x08

//  Obsolete, for bridging only
//#define PPA_PORT_ETH0                               0x00
//#define PPA_PORT_ETH1                               0x01
//#define PPA_PORT_CPU0                               0x02
//#define PPA_PORT_CPU1_EXT_IF0                       0x05
//#define PPA_PORT_ATM                                0x08
//#define PPA_PORT_ANY                                PPA_PORT_CPU0
//#define PPA_PORT_NUM                                0x09

#define PPA_BRG_VLAN_IG_COND_TYPE_DEF               0
#define PPA_BRG_VLAN_IG_COND_TYPE_SRC_IP            1
#define PPA_BRG_VLAN_IG_COND_TYPE_ETH_TYPE          2
#define PPA_BRG_VLAN_IG_COND_TYPE_VLAN              3

typedef enum {
  PPA_GENERIC_HAL_GET_DSL_MIB= 0,   //Get dsl mib
  PPA_GENERIC_HAL_CLEAR_DSL_MIB,   //clear dsl mib
  PPA_GENERIC_WAN_INFO,   //get wan information
  PPA_GENERIC_HAL_GET_PORT_MIB,  //get alls ports mib
  PPA_GENERIC_HAL_SET_DEBUG,    // turn on/off hal debug information
  PPA_GENERIC_HAL_GET_FEATURE_LIST, //get featuer list

//Fix warning message when exports API from different PPE FW Driver--begin
  PPA_GENERIC_HAL_GET_MAX_ENTRIES,  //get maximum ipv4 routing entry number
  PPA_GENERIC_HAL_GET_HAL_VERSION,  //get hal version number
  PPA_GENERIC_HAL_GET_PPE_FW_VERSION,  //get ppe fw version number
  PPA_GENERIC_HAL_GET_PHYS_PORT_NUM,  //get maixum physical port number
  PPA_GENERIC_HAL_GET_PHYS_PORT_INFO,  //get physical port information
  PPA_GENERIC_HAL_SET_MIX_WAN_VLAN_ID,  //set WAN interface's vlan range for mixed mode
  PPA_GENERIC_HAL_GET_MIX_WAN_VLAN_ID,  //get WAN interface's vlan range for mixed mode
  PPA_GENERIC_HAL_SET_ROUT_CFG,   //set routing configuration
  PPA_GENERIC_HAL_SET_BRDG_CFG,  //set bridge confgiration
  PPA_GENERIC_HAL_SET_FAST_MODE_CFG,  //set fact mode configuration
  PPA_GENERIC_HAL_SET_DEST_LIST,  // set destion list
  PPA_GENERIC_HAL_SET_ACC_ENABLE,  // eanble/disable Lan/wan routing acceleration respectively
  PPA_GENERIC_HAL_GET_ACC_ENABLE,  //get lan/wan routing eanble flag
  PPA_GENERIC_HAL_SET_BRDG_VLAN_CFG,  //set vlan briding configuration for a4/d4/e4 only
  PPA_GENERIC_HAL_GET_BRDG_VLAN_CFG,  //get vlan briding configuration for a4/d4/e4 only
  PPA_GENERIC_HAL_ADD_BRDG_VLAN_FITLER, //set vlan briding flow  for a4/d4/e4 only
  PPA_GENERIC_HAL_DEL_BRDG_VLAN_FITLER,  //del vlan briding flow  for a4/d4/e4 only
  PPA_GENERIC_HAL_GET_BRDG_VLAN_FITLER,  //get vlan briding flow  for a4/d4/e4 only
  PPA_GENERIC_HAL_DEL_BRDG_VLAN_ALL_FITLER_MAP, //delete all vlan briding filter's mapping
  PPA_GENERIC_HAL_GET_MAX_VFILTER_ENTRY_NUM, //get the maxumum entry for vlan filter
  PPA_GENERIC_HAL_GET_IPV6_FLAG,  //get ipv6 status : enabled or disabled
  PPA_GENERIC_HAL_ADD_COMPLEMENT_ENTRY, //add an ipv4 routing entry - complimentary processing
  PPA_GENERIC_HAL_DEL_COMPLEMENT_ENTRY, //del an ipv4 routing entry - complimentary processing
  PPA_GENERIC_HAL_ADD_ROUTE_ENTRY, //add an ipv4 routing entry
  PPA_GENERIC_HAL_DEL_ROUTE_ENTRY,  //del an ipv4 routing entry
  PPA_GENERIC_HAL_UPDATE_ROUTE_ENTRY,    //modify an ipv4 routing entry
  PPA_GENERIC_HAL_ADD_MC_ENTRY,   //add a multicast entry
  PPA_GENERIC_HAL_DEL_MC_ENTRY,  //del a multicast entry
  PPA_GENERIC_HAL_UPDATE_MC_ENTRY,   //modify a multicast entry
  PPA_GENERIC_HAL_ADD_BR_MAC_BRIDGING_ENTRY,  //add an mac address from brdging learning
  PPA_GENERIC_HAL_DEL_BR_MAC_BRIDGING_ENTRY,  //del an mac address from brdging learning
  PPA_GENERIC_HAL_ADD_BR_PORT,	//add port under a bridge
  PPA_GENERIC_HAL_DEL_BR_PORT,  //delete port from bridge
  PPA_GENERIC_HAL_ADD_PPPOE_ENTRY,   //add a pppoe entry
  PPA_GENERIC_HAL_DEL_PPPOE_ENTRY,  //del a pppoe entry
  PPA_GENERIC_HAL_GET_PPPOE_ENTRY,   //get a ppoe entry
  PPA_GENERIC_HAL_ADD_6RD_TUNNEL_ENTRY,  //add a 6rd tunnel entry
  PPA_GENERIC_HAL_DEL_6RD_TUNNEL_ENTRY,  //del a 6rd tunnel entry
  PPA_GENERIC_HAL_GET_6RD_TUNNEL_ENTRY,  //get a 6rd tunnel entry
  PPA_GENERIC_HAL_ADD_DSLITE_TUNNEL_ENTRY,  //add a dslite tunnel entry
  PPA_GENERIC_HAL_DEL_DSLITE_TUNNEL_ENTRY,  //del a dslite tunnel entry
  PPA_GENERIC_HAL_GET_DSLITE_TUNNEL_ENTRY,  //get a dslite tunnel entry
  PPA_GENERIC_HAL_ADD_MTU_ENTRY,   //and a MTU entry
  PPA_GENERIC_HAL_DEL_MTU_ENTRY,    //del a MTU entry
  PPA_GENERIC_HAL_GET_MTU_ENTRY,    //get a MUT entry
  PPA_GENERIC_HAL_GET_ROUTE_ACC_BYTES, //get the acclerated bytes counter for a specified acceleration routing entry
  PPA_GENERIC_HAL_GET_IPSEC_TUNNEL_MIB, //get the acclerated bytes counter for a specified acceleration tunnel entry
  PPA_GENERIC_HAL_GET_MC_ACC_BYTES, //get the acclerated bytes counter for a specified acceleration multicast entry
  PPA_GENERIC_HAL_ADD_MAC_ENTRY,   //add a routing mac address
  PPA_GENERIC_HAL_DEL_MAC_ENTRY,  //del a routing mac address
  PPA_GENERIC_HAL_GET_MAC_ENTRY,    //get a routing mac address
  PPA_GENERIC_HAL_ADD_OUT_VLAN_ENTRY, //add a out vlan entry
  PPA_GENERIC_HAL_DEL_OUT_VLAN_ENTRY,  //del a out vlan entry
  PPA_GENERIC_HAL_GET_OUT_VLAN_ENTRY,  //get a out vlan entry info
  PPA_GENERIC_HAL_ADD_IPV6_ENTRY, //add a IPv6 entry
  PPA_GENERIC_HAL_DEL_IPV6_ENTRY,  //del a IPv6 entry
  PPA_GENERIC_HAL_GET_IPV6_ENTRY,  //get a IPv6 entry info
  PPA_GENERIC_HAL_GET_ITF_MIB,   /*get one port's mib counter. I think it should be merged with PPA_GENERIC_HAL_GET_PORT_MIB */
  PPA_GENERIC_HAL_MFE_CONTROL,   //enable/disable multiple field vlan editing feature
  PPA_GENERIC_HAL_MFE_STATUS,  // get a multiple field vlan editing feature status: enabled/disabled
  PPA_GENERIC_HAL_MFE_GET_FLOW_MAX_ENTRY, // get muaximum entry number for multiple field vlan editing
  PPA_GENERIC_HAL_MFE_ADD_FLOW,   //add a multiple field vlan editing entry
  PPA_GENERIC_HAL_MFE_DEL_FLOW,   //del a multiple field vlan editing entry
  PPA_GENERIC_HAL_MFE_DEL_FLOW_VIA_ENTRY,  //del a multiple field vlan editing entry
  PPA_GENERIC_HAL_MFE_GET_FLOW,  //get a multiple field vlan editing entry
  PPA_GENERIC_HAL_TEST_CLEAR_ROUTE_HIT_STAT, //check whether a routing session is hit or not
  PPA_GENERIC_HAL_TEST_CLEAR_BR_HIT_STAT, //check whether a bridge session is hit or not
  PPA_GENERIC_HAL_TEST_CLEAR_MC_HIT_STAT, //check whether a multicast session is hit or not
  PPA_GENERIC_HAL_INIT, //init HAL
  PPA_GENERIC_HAL_EXIT, //exit HAL
  PPA_GENERIC_HAL_ADD_CLASS_RULE, // add classification rule
  PPA_GENERIC_HAL_MOD_CLASS_RULE, // modify classification rule
  PPA_GENERIC_HAL_DEL_CLASS_RULE, // delete classification rule
  PPA_GENERIC_HAL_GET_CLASS_RULE, // get classification rule
  PPA_GENERIC_HAL_QOS_INIT_CFG,
  PPA_GENERIC_HAL_QOS_UNINIT_CFG,
  PPA_GENERIC_HAL_GET_QOS_QUEUE_NUM,  //get maximum QOS queue number
  PPA_GENERIC_HAL_GET_QOS_MIB,  //get maximum QOS queue number
  PPA_GENERIC_HAL_SET_QOS_WFQ_CTRL,  //enable/disable WFQ
  PPA_GENERIC_HAL_GET_QOS_WFQ_CTRL,  //get  WFQ status: enabeld/disabled
  PPA_GENERIC_HAL_SET_QOS_WFQ_CFG,  //set WFQ cfg
  PPA_GENERIC_HAL_RESET_QOS_WFQ_CFG,  //reset WFQ cfg
  PPA_GENERIC_HAL_GET_QOS_WFQ_CFG,  //get WFQ cfg
  PPA_GENERIC_HAL_INIT_QOS_WFQ, // init QOS Rateshapping
  PPA_GENERIC_HAL_SET_QOS_RATE_SHAPING_CTRL,  //enable/disable Rate shaping
  PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CTRL,  //get  Rateshaping status: enabeld/disabled
  PPA_GENERIC_HAL_SET_QOS_RATE_SHAPING_CFG,  //set rate shaping
  PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG,  //get rate shaping cfg
  PPA_GENERIC_HAL_RESET_QOS_RATE_SHAPING_CFG,  //reset rate shaping cfg
  PPA_GENERIC_HAL_INIT_QOS_RATE_SHAPING, // init QOS Rateshapping
#if defined(MBR_CONFIG) && MBR_CONFIG
  PPA_GENERIC_HAL_SET_QOS_SHAPER_CFG,  //set shaper 
  PPA_GENERIC_HAL_GET_QOS_SHAPER_CFG,  //get shaper 
  PPA_GENERIC_HAL_RESET_QOS_SHAPER_CFG, //reset shaper
#endif
  PPA_GENERIC_HAL_SET_WANITF, // Set HAL WANITF
  PPA_GENERIC_HAL_GET_SESSION_HASH, // Get Hash Index

  //Fix warning message when exports API from different PPE FW Driver--End
  PPA_GENERIC_HAL_GET_QOS_STATUS, // get QOS tatus
  PPA_GENERIC_HAL_SET_VALUE, // set value
  PPA_GENERIC_HAL_GET_VALUE, // get value



  //Below macro is used for PPE datapath
  PPA_GENERIC_DATAPATH_GET_PPE_VERION,  //for VR9 E5 only so far for there is two PP32 core
  /*make sure it is the last one */
  PPA_GENERIC_DATAPATH_TSET,   //test only
  PPA_GENERIC_DATAPATH_ADDR_TO_FPI_ADDR,  //change to FPI address
  PPA_GENERIC_DATAPATH_CLEAR_MIB,  //change to FPI address
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
  PPA_GENERIC_HAL_SET_MC_RTP,  //Set rtp sampling for a multicast entry
  PPA_GENERIC_HAL_GET_MC_RTP_SAMPLING_CNT, //RTP sampling count
#endif

#if defined(L2TP_CONFIG) && L2TP_CONFIG
  PPA_GENERIC_HAL_ADD_L2TP_TUNNEL_ENTRY,  //add a L2TP tunnel entry
  PPA_GENERIC_HAL_DEL_L2TP_TUNNEL_ENTRY,  //del a L2TP tunnel entry
  PPA_GENERIC_HAL_GET_L2TP_TUNNEL_ENTRY,  //get a L2TP tunnel entry
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
  PPA_GENERIC_HAL_ADD_CAPWAP_ENTRY,   //add a CAPWAP entry
  PPA_GENERIC_HAL_DEL_CAPWAP_ENTRY,   //delete a CAPWAP entry
  PPA_GENERIC_HAL_GET_CAPWAP_MIB,     //CAWAP mib
#endif

  PPA_GENERIC_HAL_QOS_MODQUE_CFG,
  PPA_GENERIC_HAL_QOS_ADDQUE_CFG,
  PPA_GENERIC_HAL_QOS_DELQUE_CFG,

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
  PPA_GENERIC_HAL_SET_MIB_MODE_ENABLE, //configure the mib mode
  PPA_GENERIC_HAL_GET_MIB_MODE_ENABLE,
#endif

  PPA_GENERIC_HAL_ADD_LRO_ENTRY,
  PPA_GENERIC_HAL_DEL_LRO_ENTRY,

  PPA_GENERIC_DATAPATH_SET_LAN_SEPARATE_FLAG,
  PPA_GENERIC_DATAPATH_GET_LAN_SEPARATE_FLAG,
  PPA_GENERIC_DATAPATH_SET_WAN_SEPARATE_FLAG,
  PPA_GENERIC_DATAPATH_GET_WAN_SEPARATE_FLAG,
  PPA_GENERIC_HAL_MOD_SUBIF_PORT_CFG,
  /*make sure it is the last one */
  PPA_GENERIC_HAL_MAX_FLAG
}PPA_GENERIC_HOOK_CMD;

#define PPA_PWM_LEVEL_D0                            0
#define PPA_PWM_LEVEL_D1                            1
#define PPA_PWM_LEVEL_D2                            2
#define PPA_PWM_LEVEL_D3                            3



/*
 * ####################################
 *              Data Type
 * ####################################
 */

typedef int32_t (*ppe_generic_hook_t)(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
typedef int32_t (*ppa_generic_hook_t)(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);
#endif

/*
 * ####################################
 *             Declaration
 * ####################################
 */

#ifdef __KERNEL__



  /*
   *    implemented in datapath driver
   */

extern int32_t (*ppa_drv_hal_generic_hook)(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);
extern int32_t (*ppa_drv_hal_get_mpoa_type_hook)(uint32_t dslwan_qid, uint32_t *mpoa_type);

#endif  //  __KERNEL__


#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
#define PPA_F_INIT          1
#define PPA_F_UNINIT        2

#define DTLK_TX_RSV_MEM_SIZE 0x800000
#define DTLK_RX_RSV_MEM_SIZE 0x400000

typedef struct
{
	int64_t txpdu;
	int64_t txbytes;
	unsigned int txdrop;
	unsigned int rx_fwd_pdu;
	unsigned int rx_fwd_bytes;
	unsigned int rx_inspect_pdu;
	unsigned int rx_inspect_bytes;
	unsigned int rx_discard_pdu;
	unsigned int rx_discard_bytes;
	unsigned int rx_pn_pdu;
	unsigned int rx_pn_bytes;
	unsigned int rx_drop_pdu;
	unsigned int rx_drop_bytes;
	int64_t rx_rcv_pdu;
	int64_t rx_rcv_bytes;
	unsigned int _dw_res0;
}PPA_WLAN_VAP_Stats_t; 

extern void ppa_directlink_enable (uint32_t flags);

extern int32_t ppa_directlink_get_status (uint32_t flag);

extern uint32_t ppa_dl_qca_h2t_ring_init ( uint32_t h2tRingSize, uint32_t entrySize,uint32_t src_ring_base,uint32_t pcie_base, uint32_t flags );

extern uint32_t ppa_dl_qca_cpu_h2t_ring_init (uint32_t h2tCpuMsgRingSize, uint32_t entrySize, uint32_t flags);

 extern int32_t ppa_dl_qca_cpu_h2t_ring_get_write_idx (uint32_t flags);

extern int32_t ppa_dl_qca_cpu_h2t_ring_write_msg ( uint32_t writeIndex, uint32_t * msgPtr, uint32_t msgLen, uint32_t next_writeIndex, uint32_t flags );

extern int32_t ppa_dl_dre_txpkt_buf_release ( uint32_t num_msdus,uint32_t * msg, uint32_t flags );

extern uint32_t ppa_dl_qca_get_vap_stats ( uint32_t vapId, PPA_WLAN_VAP_Stats_t * vapStats, uint32_t flags );

extern void dtlk_mem_base_get(uint32_t *dltx_base, uint32_t *dlrx_base);
extern void (*set_vap_itf_tbl_fn)(uint32_t, uint32_t);

#endif


#endif  //  __PPA_PPE_HAL_H__20081104_1318__
