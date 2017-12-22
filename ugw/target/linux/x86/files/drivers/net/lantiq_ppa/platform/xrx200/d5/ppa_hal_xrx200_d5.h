/******************************************************************************
**
** FILE NAME    : ppa_hal_vr9_d5.h
** PROJECT      : UEIP
** MODULES      : MII0/1 Acceleration Package (VR9 PPA D5)
**
** DATE         : 19 OCT 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : MII0/1 Driver with Acceleration Firmware (D5)
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 19 OCT 2009  Xu Liang        Initiate Version
*******************************************************************************/
#ifndef PPA_HAL_VR9_D5_H
#define PPA_HAL_VR9_D5_H

/*
 *  Platform and Mode
 */
#define PLATFM_VR9                              1
#define MODE_VR9_D5                             1
#define DEF_PPA_MODE_D5


/*
 *  hash calculation
 */

#define BRIDGING_SESSION_LIST_HASH_BIT_LENGTH   8
#define BRIDGING_SESSION_LIST_HASH_MASK         ((1 << BRIDGING_SESSION_LIST_HASH_BIT_LENGTH) - 1)
#define BRIDGING_SESSION_LIST_HASH_TABLE_SIZE   (1 << BRIDGING_SESSION_LIST_HASH_BIT_LENGTH)
#define BRIDGING_SESSION_LIST_HASH_VALUE(x)     ((x) & BRIDGING_SESSION_LIST_HASH_MASK)
/*
 *  Firmware Constant
 */

#define MAX_IPV6_IP_ENTRIES                     (MAX_IPV6_IP_ENTRIES_PER_BLOCK * MAX_IPV6_IP_ENTRIES_BLOCK)
#define MAX_IPV6_IP_ENTRIES_PER_BLOCK           128
#define MAX_IPV6_IP_ENTRIES_BLOCK               2
#define MAX_ROUTING_ENTRIES                     (MAX_WAN_ROUTING_ENTRIES + MAX_LAN_ROUTING_ENTRIES)
#define MAX_COLLISION_ROUTING_ENTRIES           64
#define MAX_HASH_BLOCK                          32
#define MAX_ROUTING_ENTRIES_PER_HASH_BLOCK      16
#define MAX_WAN_ROUTING_ENTRIES                 (MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK + MAX_COLLISION_ROUTING_ENTRIES)
#define MAX_LAN_ROUTING_ENTRIES                 (MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK + MAX_COLLISION_ROUTING_ENTRIES)
#define MAX_WAN_MC_ENTRIES                      32

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
#define MAX_CAPWAP_ENTRIES                      5 

#define  ETHER_TYPE                             0x0800
#define  IP_VERSION                             0x4
#define  IP_HEADER_LEN                          0x5
#define  IP_TOTAL_LEN                           0x0
#define  IP_IDENTIFIER                          0x0
#define  IP_FLAG                                0x0
#define  IP_FRAG_OFF                            0x0
#define  IP_PROTO_UDP                           0x11 
#define  UDP_TOTAL_LENGTH                       0x0
#define  CAPWAP_PREAMBLE                        0x0 
#define  CAPWAP_HDLEN                           0x2 
#define  CAPWAP_F_FLAG                          0x0 
#define  CAPWAP_L_FLAG                          0x0
#define  CAPWAP_W_FLAG                          0x0
#define  CAPWAP_M_FLAG                          0x0
#define  CAPWAP_K_FLAG                          0x0
#define  CAPWAP_FLAGS                           0x0
#define  CAPWAP_FRAG_ID                         0x0
#define  CAPWAP_FRAG_OFF                        0x0

#endif


#define MAX_PPPOE_ENTRIES                       8
#define MAX_MTU_ENTRIES                         8
#define MAX_MAC_ENTRIES                         16
#define MAX_OUTER_VLAN_ENTRIES                  32
#define MAX_CLASSIFICATION_ENTRIES              (64 - 1)
#define MAX_6RD_TUNNEL_ENTRIES                  4
#define MAX_DSLITE_TUNNEL_ENTRIES               4
#if defined(L2TP_CONFIG) && L2TP_CONFIG
#define MAX_PPPOL2TP_ENTRIES                    8
#endif

/*
 *  FPI Configuration Bus Register and Memory Address Mapping
 */
#define PPE_BASE                                (KSEG1 | 0x1E200000)
#define PP32_DEBUG_REG_ADDR(i, x)               ((volatile unsigned int*)(PPE_BASE + (((x) + 0x000000 + (i) * 0x00010000) << 2)))
#define CDM_CODE_MEMORY(i, x)                   ((volatile unsigned int*)(PPE_BASE + (((x) + 0x001000 + (i) * 0x00010000) << 2)))
#define CDM_DATA_MEMORY(i, x)                   ((volatile unsigned int*)(PPE_BASE + (((x) + 0x004000 + (i) * 0x00010000) << 2)))
#define SB_RAM0_ADDR(x)                         ((volatile unsigned int*)(PPE_BASE + (((x) + 0x008000) << 2)))
#define SB_RAM1_ADDR(x)                         ((volatile unsigned int*)(PPE_BASE + (((x) + 0x009000) << 2)))
#define SB_RAM2_ADDR(x)                         ((volatile unsigned int*)(PPE_BASE + (((x) + 0x00A000) << 2)))
#define SB_RAM3_ADDR(x)                         ((volatile unsigned int*)(PPE_BASE + (((x) + 0x00B000) << 2)))
#define PPE_REG_ADDR(x)                         ((volatile unsigned int*)(PPE_BASE + (((x) + 0x00D000) << 2)))
#define QSB_CONF_REG_ADDR(x)                    ((volatile unsigned int*)(PPE_BASE + (((x) + 0x00E000) << 2)))
#define SB_RAM6_ADDR(x)                         ((volatile unsigned int*)(PPE_BASE + (((x) + 0x018000) << 2)))

/*
 *  DWORD-Length of Memory Blocks
 */
#define PP32_DEBUG_REG_DWLEN                    0x0030
#define CDM_CODE_MEMORYn_DWLEN(n)               ((n) == 0 ? 0x1000 : 0x0800)
#define CDM_DATA_MEMORY_DWLEN                   CDM_CODE_MEMORYn_DWLEN(1)
#define SB_RAM0_DWLEN                           0x1000
#define SB_RAM1_DWLEN                           0x1000
#define SB_RAM2_DWLEN                           0x1000
#define SB_RAM3_DWLEN                           0x1000
#define SB_RAM6_DWLEN                           0x8000
#define QSB_CONF_REG_DWLEN                      0x0100

/*
 *  Host-PPE Communication Data Address Mapping
 */
#define SB_BUFFER(__sb_addr)            ((volatile unsigned int *)((((__sb_addr) >= 0x0000) && ((__sb_addr) <= 0x1FFF)) ? CDM_DATA_MEMORY(0,(__sb_addr)) :    \
                                                                   (((__sb_addr) >= 0x2000) && ((__sb_addr) <= 0x2FFF)) ? SB_RAM0_ADDR((__sb_addr) - 0x2000) :  \
                                                                   (((__sb_addr) >= 0x3000) && ((__sb_addr) <= 0x3FFF)) ? SB_RAM1_ADDR((__sb_addr) - 0x3000) :  \
                                                                   (((__sb_addr) >= 0x4000) && ((__sb_addr) <= 0x4FFF)) ? SB_RAM2_ADDR((__sb_addr) - 0x4000) :  \
                                                                   (((__sb_addr) >= 0x5000) && ((__sb_addr) <= 0x5FFF)) ? SB_RAM3_ADDR((__sb_addr) - 0x5000) :  \
                                                                   (((__sb_addr) >= 0x7000) && ((__sb_addr) <= 0x7FFF)) ? PPE_REG_ADDR((__sb_addr) - 0x7000) :  \
                                                                   (((__sb_addr) >= 0x8000) && ((__sb_addr) <= 0xFFFF)) ? SB_RAM6_ADDR((__sb_addr) - 0x8000) :  \
                                                                   (((__sb_addr) >= 0x10000) &&((__sb_addr) <= 0x11FFF)) ? CDM_DATA_MEMORY(1, (__sb_addr))   :  \
                                                                0))

#define CDM_CFG                                 ((volatile struct cdm_mem_cfg *)            PPE_REG_ADDR(0x0100))
#define FW_VER_ID                               ((volatile struct fw_ver_id *)              SB_BUFFER(0x2000))
#define FW_VER_FEATURE                                                                      SB_BUFFER(0x2001)

#define PS_MC_GENCFG3                           ((volatile struct ps_mc_cfg *)              SB_BUFFER(0x2003))   //  power save and multicast gen config

#define CFG_STD_DATA_LEN                        ((volatile struct cfg_std_data_len *)       SB_BUFFER(0x2011))
#define TX_QOS_CFG                              ((volatile struct tx_qos_cfg *)             SB_BUFFER(0x2012))
#define EG_BWCTRL_CFG                           ((volatile struct eg_bwctrl_cfg *)          SB_BUFFER(0x2013))
#define CFG_WAN_PORTMAP                         SB_BUFFER(0x201A)
#define CFG_MIXED_PORTMAP                       SB_BUFFER(0x201B)
#define WRX_DMACH_ON                            SB_BUFFER(0x2015)
#define WTX_DMACH_ON                            SB_BUFFER(0x2016)
#define PSEUDO_IPv4_BASE_ADDR                   SB_BUFFER(0x2023)
#define TX_QOS_WFQ_RELOAD_MAP                   SB_BUFFER(0x2020)
#define ETH_PORTS_CFG                           ((volatile struct eth_ports_cfg *)          SB_BUFFER(0x2024))
#define LAN_ROUT_TBL_CFG                        ((volatile struct rout_tbl_cfg *)           SB_BUFFER(0x2026))
#define WAN_ROUT_TBL_CFG                        ((volatile struct rout_tbl_cfg *)           SB_BUFFER(0x2027))
#define GEN_MODE_CFG1                           ((volatile struct gen_mode_cfg1 *)          SB_BUFFER(0x2028))
#define GEN_MODE_CFG                            GEN_MODE_CFG1
#define GEN_MODE_CFG2                           ((volatile struct gen_mode_cfg2 *)          SB_BUFFER(0x2029))
#define KEY_SEL_n(i)                            SB_BUFFER(0x202C + (i))

#define WTX_QOS_Q_DESC_CFG(i)                   ((volatile struct wtx_qos_q_desc_cfg *)     SB_BUFFER(0x3F00 + (i) * 2))    /*  i < 8  */
#define WTX_EG_Q_PORT_SHAPING_CFG(i)            ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(0x3F60 + (i) * 4))    /*  i < 1  */
#define WTX_EG_Q_SHAPING_CFG(i)                 WTX_EG_Q_PORT_SHAPING_CFG((i+2))  /* i <  8 */

#if defined(MBR_CONFIG) && MBR_CONFIG
#define __QOS_NEW_FEATURE                        SB_BUFFER(0x2FEA)
#define DEFAULT_SHAPING_CFG_BASE                 0x2FD0
#define SHAPING_CFG_BASE                         0x3F64
#define WTX_EG_SHAPING_CFG(i)                    (0x3F64 + (i) * 4)  
#define WTX_Q_SHAPER_MAPPING_CFG(i)              ((volatile struct wtx_q_shaper_mapping_cfg *)   SB_BUFFER(0x2FD8 + (i) * 1))        
#endif

/* MBR Queue to Shaper Mapping */
#define __ETH1_QoS_DEFAULT_SHAPING_SCHEDULING_CFG	0x3F64
#define __ETH1_QoS_Q_SHAPING_BASE			0x3F68
#define __ETH1_QoS_Q_SHAPING_SCHEDULING_CFG(i)		(0x3F68 + (i) * 4)
#define __ETH1_QoS_DEF_SHAPING_SCH_CFG_SB               SB_BUFFER(__ETH1_QoS_DEFAULT_SHAPING_SCHEDULING_CFG)
#define __ETH1_QoS_Q_SHAPING_SCH_CFG_SB(i)		SB_BUFFER(__ETH1_QoS_Q_SHAPING_SCHEDULING_CFG(i))
#define __ETH1_QoS_DEF_SHAPING_SCH_CFG_SB_STRUCT        ((volatile struct wtx_eg_q_shaping_cfg *) SB_BUFFER(__ETH1_QoS_DEFAULT_SHAPING_SCHEDULING_CFG))
#define __ETH1_QoS_Q_SHAPING_SCH_CFG_SB_STRUCT(i)	((volatile struct wtx_eg_q_shaping_cfg *) SB_BUFFER(__ETH1_QoS_Q_SHAPING_SCHEDULING_CFG(i)))
#define __ETH1_QoS_Q_SHAPER_MAPPING_CFG(i)            	SB_BUFFER(0x2FD8 + (i) * 1)
#define __ETH1_QoS_Q_SHAPER_MAPPING_CFG_SB(i)           ((volatile struct wtx_q_shaper_mapping_cfg *) SB_BUFFER(0x2FD8 + (i) * 1)) 
#define __ETH1_QoS_Q_SHAPER_WEIGHT_NEGATIVE_MAP         SB_BUFFER(0x2FE0)

/* MBR Shaper to Queue Mapping */
#define __ETH1_QoS_Q_DEFAULT_SHAPER_QUEUE_MAP           SB_BUFFER(0x2FE1)
#define __ETH1_QoS_Q_SHAPER_QUEUE_MAP(i)              	SB_BUFFER(0x2FE2 + (i))


#define DROPPED_PAUSE_FRAME_COUNTER(i)          SB_BUFFER(0x3F10 + (i) * 2)     /*  i < 8   */
#define ETH_WAN_TX_MIB_TABLE(i)                 ((volatile struct eth_wan_mib_table *)      SB_BUFFER(0x3F20 + (i) * 8))    /*  i < 8   */

#define ITF_MIB_TBL(i)                          ((volatile struct itf_mib *)                SB_BUFFER(0x2030 + (i) * 16))   /*  i < 8   */

#define PPPOE_CFG_TBL(i)                        SB_BUFFER(0x20B0 + (i))         /*  i < 8   */
#define MTU_CFG_TBL(i)                          SB_BUFFER(0x20B8 + (i))         /*  i < 8   */
#define ROUT_MAC_CFG_TBL(i)                     SB_BUFFER(0x20C0 + (i) * 2)     /*  i < 16  */
#define TUNNEL_6RD_TBL(i)                       SB_BUFFER(0x5F94  + (i) * 5)     /*  i < 4    */
#define TUNNEL_DSLITE_TBL(i)                    SB_BUFFER(0x2FA8 + (i) * 10)    /*  i < 4    */
#define TUNNEL_MAX_ID                           SB_BUFFER(0x2025)               /* IPv4 Header Identification value */

#if defined(L2TP_CONFIG) && L2TP_CONFIG
#define __L2TPTUNNLEL_CONFIG_TABLE_BASE         0x27C0
#define L2TPTUNNEL_CONFIG_TBL(i)                ((volatile struct l2tptunnel_config_tbl *) SB_BUFFER(__L2TPTUNNLEL_CONFIG_TABLE_BASE + (i) * 12))
#endif

#define IPv6_IP_IDX_TBL(x, i)                   SB_BUFFER(((x == 0) ? 0x2D00 : 0x3700) + (i) * 4)   /*  i < 128 */

#define CFG_CLASS2QID_MAP(i)                    SB_BUFFER(0x5FA8 + (i))         /*  i < 4   */
#define CFG_QID2CLASS_MAP(i)                    SB_BUFFER(0x5FAC + (i))         /*  i < 4   */

#define POWERSAVING_PROFILE(i)                  ((volatile struct powersaving_profile *)    SB_BUFFER(0x3F90 + (i) * 4))    /*  i < 4, TIMER0 (0), DPLUS_IN (1), SFSM (2), RUNNING (3)    */

#define DPLUS_RX_DBA                            0x3D00
#define DPLUS_RX_CBA                            0x3F80
//-------------------------------------
// Hit Status
//-------------------------------------
#define __IPV4_WAN_HIT_STATUS_HASH_TABLE_BASE       0x20E0
#define __IPV4_WAN_HIT_STATUS_COLLISION_TABLE_BASE  0x2FA0
#define __IPV4_LAN_HIT_STATUS_HASH_TABLE_BASE       0x20F0
#define __IPV4_LAN_HIT_STATUS_COLLISION_TABLE_BASE  0x2FA2
#define __IPV4_WAN_HIT_STATUS_MC_TABLE_BASE         0x2FA4
#define __IPV6_HIT_STATUS_TABLE_BASE                0x2FA6  //  reserved

#define ROUT_LAN_HASH_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_LAN_HIT_STATUS_HASH_TABLE_BASE + (i))
#define ROUT_LAN_COLL_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_LAN_HIT_STATUS_COLLISION_TABLE_BASE + (i))
#define ROUT_WAN_HASH_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_WAN_HIT_STATUS_HASH_TABLE_BASE + (i))
#define ROUT_WAN_COLL_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_WAN_HIT_STATUS_COLLISION_TABLE_BASE + (i))
#define ROUT_WAN_MC_HIT_STAT_TBL(i)                 SB_BUFFER(__IPV4_WAN_HIT_STATUS_MC_TABLE_BASE + (i))

//-------------------------------------
// Compare and Action table
//-------------------------------------
#define __IPV4_LAN_HASH_ROUT_FWDC_TABLE_BASE        0x3000
#define __IPV4_LAN_HASH_ROUT_FWDA_TABLE_BASE        0x5100

#define __IPV4_LAN_COLLISION_ROUT_FWDC_TABLE_BASE   0x2700
#define __IPV4_LAN_COLLISION_ROUT_FWDA_TABLE_BASE   0x2B80

#define __IPV4_WAN_HASH_ROUT_FWDC_TABLE_BASE        0x2100
#define __IPV4_WAN_HASH_ROUT_FWDA_TABLE_BASE        0x4400

#define __IPV4_WAN_COLLISION_ROUT_FWDC_TABLE_BASE   0x3600
#define __IPV4_WAN_COLLISION_ROUT_FWDA_TABLE_BASE   0x4000

#define __IPV4_ROUT_MULTICAST_FWDC_TABLE_BASE       0x36C0
#define __IPV4_ROUT_MULTICAST_FWDA_TABLE_BASE       0x2B00

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
#define __MULTICAST_RTP_MIB_BASE                    0x2B60
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
#define __CAPWAP_CONFIG_TABLE_BASE                  0x3800
#endif

#define ROUT_LAN_HASH_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_LAN_HASH_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_LAN_HASH_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_LAN_HASH_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_LAN_COLL_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_LAN_COLLISION_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_LAN_COLL_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_LAN_COLLISION_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_WAN_HASH_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_WAN_HASH_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_WAN_HASH_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_WAN_HASH_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_WAN_COLL_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_WAN_COLLISION_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_WAN_COLL_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_WAN_COLLISION_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_WAN_MC_CMP_TBL(i)                  ((volatile struct wan_rout_multicast_cmp_tbl *) SB_BUFFER(__IPV4_ROUT_MULTICAST_FWDC_TABLE_BASE + (i) * 2))
#define ROUT_WAN_MC_ACT_TBL(i)                  ((volatile struct wan_rout_multicast_act_tbl *) SB_BUFFER(__IPV4_ROUT_MULTICAST_FWDA_TABLE_BASE + (i) * 2))


#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG

#define CAPWAP_CONFIG_TBL(i)                  ((volatile struct capwap_config_tbl *) SB_BUFFER(__CAPWAP_CONFIG_TABLE_BASE + (i) * 18))

#define CAPWAP_CONFIG                       SB_BUFFER(0x201F)        

#endif

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE

#define MULTICAST_RTP_MIB_TBL(i)                  ((volatile struct rtp_sampling_cnt *) SB_BUFFER( __MULTICAST_RTP_MIB_BASE + (i)))

#endif


#define OUTER_VLAN_TBL(i)                       SB_BUFFER(0x2F80 + (i))         /*  i < 32  */
#define ROUT_WAN_MC_CNT(i)                      SB_BUFFER(0x2B40 + (i))         /*  i < 32  */

#define __CLASSIFICATION_CMP_TBL_BASE           0x28C0  //  64 * 4 dwords
#define __CLASSIFICATION_MSK_TBL_BASE           0x29C0  //  64 * 4 dwords
#define __CLASSIFICATION_ACT_TBL_BASE           0x2AC0  //  64 * 1 dwords

#define CLASSIFICATION_CMP_TBL(i)               SB_BUFFER(__CLASSIFICATION_CMP_TBL_BASE + (i) * 4)  //  i < 64
#define CLASSIFICATION_MSK_TBL(i)               SB_BUFFER(__CLASSIFICATION_MSK_TBL_BASE + (i) * 4)  //  i < 64
#define CLASSIFICATION_ACT_TBL(i)               ((volatile struct classification_act_tbl *)SB_BUFFER(__CLASSIFICATION_ACT_TBL_BASE + (i)))  //  i < 64

/*
 *  Qid configuration (How qid copied from dplus slave to CPU via dma)
 *  0   -   qid configured by HW
 *  1   -   wait until previous packet is sent out before processing new one
 *  2   -   qid configured by FW
*/

#define __DPLUS_QID_CONF_PTR                    SB_BUFFER(0x2FEC)



/*
 *  destlist
 */
#define PPA_DEST_LIST_ETH0                          0x0001
#define PPA_DEST_LIST_ETH1                          0x0002
#define PPA_DEST_LIST_CPU0                          0x0004
//#define PPA_DEST_LIST_EXT_INT1                      0x0008
//#define PPA_DEST_LIST_EXT_INT2                      0x0010
//#define PPA_DEST_LIST_EXT_INT3                      0x0020
//#define PPA_DEST_LIST_EXT_INT4                      0x0040
//#define PPA_DEST_LIST_EXT_INT5                      0x0080
#define PPA_DEST_LIST_ATM                           0x080

/*
 *  Clock Generation Unit Registers
 */
#define VR9_CGU                                 (KSEG1 | 0x1F103000)
#define VR9_CGU_CLKFSR                          ((volatile u32*)(VR9_CGU + 0x0010))

/*  Helper Macro
 */
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))
#define BITSIZEOF_UINT32                        (sizeof(uint32_t) * 8)
#define BITSIZEOF_UINT8                         (sizeof(uint8_t) * 8)
#define BITSIZEOF_UINT16                        (sizeof(uint16_t) * 8)
#define BITSIZEOF_UINT64                        (sizeof(uint64_t) * 8)

#define MAX_BRIDGING_ENTRIES                    2048

/*
 * Mac table manipulation action
*/
enum{
    MAC_TABLE_ENTRY_ADD = 1,
    MAC_TABLE_ENTRY_REMOVE,
};



/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *  Host-PPE Communication Data Structure
 */
#if defined(__BIG_ENDIAN)
  struct fw_ver_id {//@2000
    //DWORD 0
    unsigned int    family              :4;
    unsigned int    package             :4;
    unsigned int    major               :8;
    unsigned int    middle              :8;
    unsigned int    minor               :8;

    //DWORD 1
    unsigned int    features            :32;
  };

  struct ps_mc_cfg{//@2003  powersave & multicast config
    unsigned int    time_tick           :16;    //  max number of PP32 cycles, the PP32 can sleep
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    unsigned int    res1                :11;
    unsigned int    session_mib_unit    :1;
#else
    unsigned int    res1                :12;
#endif
    unsigned int    class_en            :1;     //  switch class enable
    unsigned int    ssc_mode            :1;     //  source specific comparison (multicast)
    unsigned int    asc_mode            :1;     //  any source comparison (multicast)
    unsigned int    psave_en            :1;     //  enable sleep mode for PPE1 (A5/D5v2) or PPE0 (D5v1)
  };

  struct proc_entry_cfg{
    char                    *parent_dir;
    char                    *name;
    unsigned int            is_dir;
    int (*proc_r_fn)(char*, char **, off_t , int , int*, void*);
    int (*proc_w_fn)(struct file*, const char*, unsigned long, void*);
    int                     is_enable;
    struct proc_dir_entry   *proc_dir;
  };

  struct cdm_mem_cfg {
    unsigned int    res                 :24;
    unsigned int    ram3                :2;
    unsigned int    ram2                :2;
    unsigned int    ram1                :2;
    unsigned int    ram0                :2;
  };

  struct cfg_std_data_len {
    unsigned int res1                   :14;
    unsigned int byte_off               :2;     //  byte offset in RX DMA channel
    unsigned int data_len               :16;    //  data length for standard size packet buffer
  };

  struct tx_qos_cfg {
    unsigned int time_tick              :16;    //  number of PP32 cycles per basic time tick
    unsigned int overhd_bytes           :8;     //  number of overhead bytes per packet in rate shaping
    unsigned int eth1_eg_qnum           :4;     //  number of egress QoS queues (< 8);
    unsigned int eth1_burst_chk         :1;     //  always 1, more accurate WFQ
    unsigned int eth1_qss               :1;     //  1: FW QoS, 0: HW QoS
    unsigned int shape_en               :1;     //  1: enable rate shaping, 0: disable
    unsigned int wfq_en                 :1;     //  1: WFQ enabled, 0: strict priority enabled
  };

  struct eg_bwctrl_cfg {
    unsigned int fdesc_wm               :16;    //  if free descriptors in QoS/Swap channel is less than this watermark, large size packets are discarded
    unsigned int class_len              :16;    //  if packet length is not less than this value, the packet is recognized as large packet
  };

  struct eth_ports_cfg {
    unsigned int    wan_vlanid_hi       :12;
    unsigned int    wan_vlanid_lo       :12;
    unsigned int    res1                :4;
    unsigned int    eth1_type           :2; //  not used
    unsigned int    eth0_type           :2; //  not used
  };

  struct rout_tbl_cfg {
    unsigned int    rout_num            :9;
    unsigned int    wan_rout_mc_num     :7; //  reserved in LAN route table config
    unsigned int    res1                :3;
    unsigned int    wan_rout_mc_ttl_en  :1; //  reserved in LAN route table config
    unsigned int    wan_rout_mc_drop    :1; //  reserved in LAN route table config
    unsigned int    ttl_en              :1;
    unsigned int    tcpdup_ver_en       :1;
    unsigned int    ip_ver_en           :1;
    unsigned int    iptcpudperr_drop    :1;
    unsigned int    rout_drop           :1;
    unsigned int    res2                :6;
  };

  struct gen_mode_cfg1 {
    unsigned int    app2_indirect       :1; //  0: direct, 1: indirect
    unsigned int    us_indirect         :1; //  0: direct, 1: indirect
    unsigned int    us_early_discard_en :1; //  0: disable, 1: enable
    unsigned int    classification_num  :6; //  classification table entry number
    unsigned int    ipv6_rout_num       :8;
    unsigned int    res1                :2;
    unsigned int    session_ctl_en      :1; //  session based rate control enable, 0: disable, 1: enable
    unsigned int    wan_hash_alg        :1; //  Hash Algorithm for IPv4 WAN ingress traffic, 0: dest port, 1: dest port + dest IP
    unsigned int    brg_class_en        :1; //  Multiple Field Based Classification and VLAN Assignment Enable for Bridging Traffic, 0: disable, 1: enable
    unsigned int    ipv4_mc_acc_mode    :1; //  Downstream IPv4 Multicast Acceleration Mode, 0: dst IP only, 1: IP pairs plus port pairs
    unsigned int    ipv6_acc_en         :1; //  IPv6 Traffic Acceleration Enable, 0: disable, 1: enable
    unsigned int    wan_acc_en          :1; //  WAN Ingress Acceleration Enable, 0: disable, 1: enable
    unsigned int    lan_acc_en          :1; //  LAN Ingress Acceleration Enable, 0: disable, 1: enable
    unsigned int    res2                :3;
    unsigned int    sw_iso_mode         :1; //  Switch Isolation Mode, 0: not isolated - ETH0/1 treated as single eth interface, 1: isolated - ETH0/1 treated as two eth interfaces
    unsigned int    sys_cfg             :2; //  System Mode, 0: DSL WAN ETH0/1 LAN, 1: res, 2: ETH0 WAN/LAN ETH1 not used, 3: ETH0 LAN ETH1 WAN
  };

  struct gen_mode_cfg2 {
    unsigned int    res1                :24;
    unsigned int    itf_outer_vlan_vld  :8; //  one bit for one interface, 0: no outer VLAN supported, 1: outer VLAN supported
  };

  struct wtx_qos_q_desc_cfg {
    unsigned int    threshold           :8;
    unsigned int    length              :8;
    unsigned int    addr                :16;
    unsigned int    rd_ptr              :16;
    unsigned int    wr_ptr              :16;
  };

  struct wtx_eg_q_shaping_cfg {
    unsigned int    t                   :8;
    unsigned int    w                   :24;
    unsigned int    s                   :16;
    unsigned int    r                   :16;
    unsigned int    res1                :8;
    unsigned int    d                   :24;  //ppe internal variable
    unsigned int    res2                :8;
    unsigned int    tick_cnt            :8;   //ppe internal variable
    unsigned int    b                   :16;  //ppe internal variable
  };

#if defined(MBR_CONFIG) && MBR_CONFIG
    struct wtx_q_shaper_mapping_cfg {
      unsigned int	  resv				  :16;
      unsigned int	  shaper_base_addr	  :16;
    };
#endif


  struct rout_forward_compare_tbl {
    /*  0h  */
    unsigned int    src_ip              :32;
    /*  1h  */
    unsigned int    dest_ip             :32;
    /*  2h  */
    unsigned int    src_port            :16;
    unsigned int    dest_port           :16;
  };

  struct rout_forward_action_tbl {
    /*  0h  */
    unsigned int    new_port            :16;
    unsigned int    new_dest_mac54      :16;
    /*  1h  */
    unsigned int    new_dest_mac30      :32;
    /*  2h  */
    unsigned int    new_ip              :32;
    /*  3h  */
    unsigned int    rout_type           :2;
    unsigned int    new_dscp            :6;
    unsigned int    mtu_ix              :3;
    unsigned int    in_vlan_ins         :1; //  Inner VLAN Insertion Enable
    unsigned int    in_vlan_rm          :1; //  Inner VLAN Remove Enable
    unsigned int    new_dscp_en         :1;
    unsigned int    entry_vld           :1;
    unsigned int    protocol            :1;
    unsigned int    dest_list           :8;
    unsigned int    pppoe_mode          :1;
    unsigned int    pppoe_ix            :3; //  not valid for WAN entry
    unsigned int    new_src_mac_ix      :4;
    /*  4h  */
    unsigned int    new_in_vci          :16;//  New Inner VLAN Tag to be inserted
    unsigned int    encap_tunnel        :1; //  encapsulate tunnel
    unsigned int    out_vlan_ix         :5; //  New Outer VLAN Tag pointed by this field to be inserted
    unsigned int    out_vlan_ins        :1; //  Outer VLAN Insertion Enable
    unsigned int    out_vlan_rm         :1; //  Outer VLAN Remove Enable
    unsigned int    tnnl_hdr_idx        :2; //  tunnel header index
    unsigned int    mpoa_type           :2; //  not valid for WAN entry, reserved in D5
    unsigned int    dest_qid            :4; //  in D5, dest_qid for both sides, in A5 DSL WAN Qid (PVC) for LAN side, dest_qid for WAN side
    /*  5h  */
    unsigned int    l2tp_encap_en       :1; //  L2TP encasulation Enable
    unsigned int    l2tp_hdr_idx        :3; //  L2TP tunnel header index
    unsigned int    reserved            :4;
    unsigned int    bytes               :24;
  };

  struct wan_rout_multicast_cmp_tbl {
    /*  0h  */
    unsigned int    wan_dest_ip         :32;
    /*  1h  */
    unsigned int    wan_src_ip          :32;
  };


#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
  
   struct rtp_sampling_cnt {
    unsigned int    pkt_cnt            :16;
    unsigned int    seq_no             :16;
  };

#endif


#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
  struct capwap_config_tbl {
    /*  0h  */
    unsigned int    us_max_frag_size    :16;
    unsigned int    us_dest_list        :8;
    unsigned int    qid                 :4;
    unsigned int    rsvd                :2;              
    unsigned int    is_ipv4header       :1;
    unsigned int    acc_en              :1;
    /*  1h  */
    unsigned int    rsvd_1;              
    /*  2h  */
    unsigned int    rsvd_2;              
    /*  3h  */
    unsigned int    ds_mib;              
    /*  4h  */
    unsigned int    us_mib;              
    /*  5h  */
    unsigned int    da_mac_hi;           
    /*  6h  */
    unsigned int    da_mac_lo           :16; 
    unsigned int    sa_mac_hi           :16; 
    /*  7h  */
    unsigned int    sa_mac_lo; 

    /*  8h  */
    unsigned int    eth_type            :16; 
    unsigned int    ver                 :4;
    unsigned int    header_len          :4;
    unsigned int    tos                 :8;
    /*  9h  */
    unsigned int    total_len           :16; 
    unsigned int    identifier          :16;
    /*  10h  */
    unsigned int    ip_flags            :3; 
    unsigned int    ip_frag_off         :13;
    unsigned int    ttl                 :8;
    unsigned int    protocol            :8;
    /*  11h  */
    unsigned int    ip_checksum         :16; 
    unsigned int    src_ip_hi           :16; 
    /*  12h  */
    unsigned int    src_ip_lo           :16; 
    unsigned int    dst_ip_hi           :16; 
    /*  13h  */
    unsigned int    dst_ip_lo           :16; 
    unsigned int    src_port            :16; 
    /*  14h  */
    unsigned int    dst_port            :16; 
    unsigned int    udp_ttl_len         :16; 
    /*  15h  */
    unsigned int    udp_checksum        :16; 
    unsigned int    preamble            :8; 
    unsigned int    hlen                :5; 
    unsigned int    rid_hi              :3; 
    /*  16h  */
    unsigned int    rid_lo              :2; 
    unsigned int    wbid                :5; 
    unsigned int    t_flag              :1; 
    unsigned int    f_flag              :1; 
    unsigned int    l_flag              :1; 
    unsigned int    w_flag              :1; 
    unsigned int    m_flag              :1; 
    unsigned int    k_flag              :1; 
    unsigned int    flags               :3; 
    unsigned int    frag_id             :16; 
    /*  17h  */
    unsigned int    frag_off            :13; 
    unsigned int    capwap_rsw          :3; 
    unsigned int    payload             :16; 
  
  };


#endif

#if defined(L2TP_CONFIG) && L2TP_CONFIG
  struct l2tptunnel_config_tbl {
  
    /* Ipv4 Header */
    
    /*  0h  */   
    unsigned int    ip_ver              :4;
    unsigned int    ip_hl               :4;
    unsigned int    ip_tos              :8;
    unsigned int    ip_len              :16;
    /*  1h  */
    unsigned int    ip_id               :16;
    unsigned int    ip_flag             :3;
    unsigned int    ip_frag_off         :13;
    /*  2h  */
    unsigned int    ip_ttl              :8;
    unsigned int    ip_proto            :8;
    unsigned int    ip_cksum            :16;
    /*  3h  */
    unsigned int    ip_sadd             :32;
    /*  4h  */
    unsigned int    ip_dadd             :32;
    
    /* UDP Header */

    /*  5h  */
    unsigned int    udp_s_port          :16;
    unsigned int    udp_d_port          :16;
    /*  6h  */
    unsigned int    udp_tot_len         :16;
    unsigned int    udp_cksum           :16;

    /* L2TP Header */

    /*  7h  */
    unsigned int    l2tp_flag           :12;
    unsigned int    l2tp_ver            :4;
    unsigned int    l2tp_tunnel_id      :16;
    /*  8h  */
    unsigned int    l2tp_session_id     :16;
    unsigned int    l2tp_off_size       :16;
    /*  9h  */
//    unsigned int    l2tp_nr           :16;
//    unsigned int    l2tp_ns           :16;
    /*  10h  */
//    unsigned int    l2tp_off_size     :16;
//    unsigned int    l2tp_off_pad      :16;

    /* PPP Header */

    /*  11h  */
    unsigned int    ppp_addr            :8;
    unsigned int    ppp_ctrl            :8;
    unsigned int    ppp_proto           :16;

    unsigned int    us_mib;
    unsigned int    ds_mib;

  };
#endif 
 
   struct wan_rout_multicast_act_tbl {
    /*  0h  */
    unsigned int    rout_type           :2; //  0: no IP level editing, 1: IP level editing (TTL)
    unsigned int    new_dscp            :6;
    unsigned int    res2                :3;
    unsigned int    in_vlan_ins         :1; //  Inner VLAN Insertion Enable
    unsigned int    in_vlan_rm          :1; //  Inner VLAN Remove Enable
    unsigned int    new_dscp_en         :1;
    unsigned int    entry_vld           :1;
    unsigned int    new_src_mac_en      :1;
    unsigned int    dest_list           :8;
    unsigned int    pppoe_mode          :1;
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    unsigned int    res3                :2;
    unsigned int    sample_en           :1;
#else
    unsigned int    res3                :3;
#endif
    unsigned int    new_src_mac_ix      :4;
    /*  1h  */
    unsigned int    new_in_vci          :16;
    unsigned int    tunnel_rm           :1;
    unsigned int    out_vlan_ix         :5;
    unsigned int    out_vlan_ins        :1;
    unsigned int    out_vlan_rm         :1;
    unsigned int    res5                :4;
    unsigned int    dest_qid            :4;
  };

  struct classification_act_tbl {
    unsigned int    new_in_vci          :16;
    unsigned int    fw_cpu              :1; //  0: forward to original destination, 1: forward to CPU without modification
    unsigned int    out_vlan_ix         :5;
    unsigned int    out_vlan_ins        :1;
    unsigned int    out_vlan_rm         :1;
    unsigned int    res1                :2;
    unsigned int    in_vlan_ins         :1;
    unsigned int    in_vlan_rm          :1;
    unsigned int    dest_qid            :4;
  };
#else
#endif

struct eth_wan_mib_table {
    unsigned int    wrx_total_pdu;
    unsigned int    wrx_total_bytes;
    unsigned int    wtx_total_pdu;
    unsigned int    wtx_total_bytes;

    unsigned int    wtx_cpu_drop_small_pdu;
    unsigned int    wtx_cpu_drop_pdu;
    unsigned int    wtx_fast_drop_small_pdu;
    unsigned int    wtx_fast_drop_pdu;
};

struct itf_mib {
    unsigned int    ig_fast_brg_pkts;           // 0 bridge ?
    unsigned int    ig_fast_brg_bytes;          // 1 ?

    unsigned int    ig_fast_rt_ipv4_udp_pkts;   // 2 IPV4 routing
    unsigned int    ig_fast_rt_ipv4_tcp_pkts;   // 3
    unsigned int    ig_fast_rt_ipv4_mc_pkts;    // 4
    unsigned int    ig_fast_rt_ipv4_bytes;      // 5

    unsigned int    ig_fast_rt_ipv6_udp_pkts;   // 6 IPV6 routing
    unsigned int    ig_fast_rt_ipv6_tcp_pkts;   // 7
    unsigned int    ig_fast_rt_ipv6_mc_pkts;    // 8
    unsigned int    ig_fast_rt_ipv6_bytes;      // 9

    unsigned int    res1;                       // A
    unsigned int    ig_cpu_pkts;                // B
    unsigned int    ig_cpu_bytes;               // C

    unsigned int    ig_drop_pkts;               // D
    unsigned int    ig_drop_bytes;              // E

    unsigned int    eg_fast_pkts;               // F
};

struct powersaving_profile {
    unsigned int    wakeup;                     //  number of wakeup from TIMER0 (0), DPLUS_IN (1), SFSM (2), reserved for RUNNING (3)
    unsigned int    cycles_lo;                  //  lower DWORD of sleep cycle before wakeup from TIMER0 (0), DPLUS_IN (1), SFSM (2), lower DWORD of running cycle for RUNNING (3)
    unsigned int    cycles_hi;                  //  higher DWORD of sleep cycle before wakeup from TIMER0 (0), DPLUS_IN (1), SFSM (2), higher DWORD of running cycle for RUNNING (3)
    unsigned int    res1;
};

struct mac_tbl_item {
    struct mac_tbl_item     *next;
    int             ref;

    uint8_t         mac[PPA_ETH_ALEN];
    u32             mac0;
    u32             mac1;
    u32             age;
    u32             timestamp;
};

struct dmrx_dba {
    unsigned int    res                 :16;
    unsigned int    dbase               :16;
};

struct dmrx_cba {
    unsigned int    res                 :16;
    unsigned int    cbase               :16;
};

struct dmrx_cfg {
    unsigned int    sen                 :1;
    unsigned int    res0                :5;
    unsigned int    trlpg               :1;
    unsigned int    hdlen               :7;
    unsigned int    res1                :3;
    unsigned int    endian              :1;
    unsigned int    psize               :2;
    unsigned int    pnum                :12;
};

struct dmrx_pgcnt {
    unsigned int    pgptr               :12;
    unsigned int    dval                :5;
    unsigned int    dsrc                :2;
    unsigned int    dcmd                :1;
    unsigned int    upage               :12;
};


struct dmrx_pktcnt {
    unsigned int    res                 :17;
    unsigned int    dsrc                :2;
    unsigned int    dcmd                :1;
    unsigned int    upkt                :12;
};

struct dsrx_dba {
    unsigned int    res                 :16;
    unsigned int    dbase               :16;
  };

struct dsrx_cba {
    unsigned int    res                 :16;
    unsigned int    cbase               :16;
  };

struct dsrx_cfg {
    unsigned int    res0                :16;
    unsigned int    dren                :1;
    unsigned int    endian              :1;
    unsigned int    psize               :2;
    unsigned int    pnum                :12;
  };

  struct dsrx_pgcnt {
    unsigned int    pgptr               :12;
    unsigned int    ival                :5;
    unsigned int    isrc                :2;
    unsigned int    icmd                :1;
    unsigned int    upage               :12;
  };
  struct ctrl_dmrx_2_fw {
    unsigned int    pg_val              :8;
    unsigned int    byte_off            :8;
    unsigned int    res                 :5;
    unsigned int    cos                 :2;
    unsigned int    bytes_cnt           :8;
    unsigned int    eof                 :1;
};

    struct ctrl_fw_2_dsrx {
      unsigned int    pg_val              :8;
      unsigned int    byte_off            :8;
      unsigned int    acc_sel             :2;
      unsigned int    res                 :3;
      unsigned int    cos                 :2;
      unsigned int    bytes_cnt           :8;
      unsigned int    eof                 :1;
  };


  struct SFSM_dba  {
    unsigned int    res     :17;
    unsigned int    dbase   :15;
} ;

 struct SFSM_cba {
    unsigned int    res     :15;
    unsigned int    cbase   :17;
} ;

  struct SFSM_cfg {
    unsigned int    res     :14;
    unsigned int    rlsync  :1;
    unsigned int    endian  :1;
    unsigned int    idlekeep:1;
    unsigned int    sen     :1;
    unsigned int    res1    :6;
    unsigned int    pnum    :8;
} ;

  struct SFSM_pgcnt {
    unsigned int    res     :14;
    unsigned int    dsrc    :1;
    unsigned int    pptr    :8;
    unsigned int    dcmd    :1;
    unsigned int    upage   :8;
} ;

  struct FFSM_dba {
    unsigned int    res     :17;
    unsigned int    dbase   :15;
} ;

 struct FFSM_cfg  {
    unsigned int    res     :12;
    unsigned int    rstptr  :1;
    unsigned int    clvpage :1;
    unsigned int    fidle   :1;
    unsigned int    endian  :1;
    unsigned int    res1    :8;
    unsigned int    pnum    :8;
} ;

  struct FFSM_pgcnt {
    unsigned int    res     :1;
    unsigned int    bptr    :7;
    unsigned int    pptr    :8;
    unsigned int    res0    :1;
    unsigned int    ival    :6;
    unsigned int    icmd    :1;
    unsigned int    vpage   :8;
} ;

  struct PTM_CW_CTRL {
    unsigned int    state   :1;
    unsigned int    bad     :1;
    unsigned int    ber     :9;
    unsigned int    spos    :7;
    unsigned int    ffbn    :7;
    unsigned int    shrt    :1;
    unsigned int    preempt :1;
    unsigned int    cwer    :2;
    unsigned int    cwid    :3;
};

  struct sll_cmd1 { //0x900
    unsigned int res0       :8;
    unsigned int mtype      :1;
    unsigned int esize      :4;
    unsigned int ksize      :4;
    unsigned int res1       :2;
    unsigned int embase     :13;
  };

  struct sll_cmd0 {
    unsigned int res0       :6;
    unsigned int cmd        :1;
    unsigned int eynum      :9;
    unsigned int res1       :3;
    unsigned int eybase     :13;
  };

  struct sll_result{
    unsigned int res0       :21;
    unsigned int vld        :1;
    unsigned int fo         :1;
    unsigned int index      :9;
  };

#endif /* PPA_HAL_VR9_D5_H */


