/******************************************************************************
**
** FILE NAME    : ppa_hal_ar10_e5.h
** PROJECT      : UEIP
** MODULES      : PTM + MII0/1 Acceleration Package (AR9 PPA E5)
**
** DATE         : 20 DEC 2010
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM + MII0/1 Driver with Acceleration Firmware (E5)
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 20 DEC 2010  Xu Liang        Initiate Version
*******************************************************************************/



#ifndef MIPS_PPA_HAL_AR10_E5_H
#define MIPS_PPA_HAL_AR10_E5_H



/*
 * ####################################
 *              Definition
 * ####################################
 */


/*
 *  Compilation Switch
 */

#define ENABLE_NEW_HASH_ALG                     1

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
#define MAX_IPV6_IP_ENTRIES_PER_BLOCK           64
#define MAX_IPV6_IP_ENTRIES_BLOCK               2
#define MAX_ROUTING_ENTRIES                     (MAX_WAN_ROUTING_ENTRIES + MAX_LAN_ROUTING_ENTRIES)
#define MAX_COLLISION_ROUTING_ENTRIES           (g_ipv6_acc_en ? 32 : 64)
#define MAX_HASH_BLOCK                          8
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
#define MAX_CLASSIFICATION_ENTRIES              32
#define MAX_6RD_TUNNEL_ENTRIES                  4
#define MAX_DSLITE_TUNNEL_ENTRIES               4
#define MAX_PTM_QOS_PORT_NUM                    2

/*
 *  FPI Configuration Bus Register and Memory Address Mapping
 */
#define IFX_PPE                                 KSEG1ADDR(0x1E180000)
#define PP32_DEBUG_REG_ADDR(x)                  ((volatile unsigned int *)(IFX_PPE + (((x) + 0x0000) << 2)))
#define PPM_INT_REG_ADDR(x)                     ((volatile unsigned int *)(IFX_PPE + (((x) + 0x0030) << 2)))
#define PP32_INTERNAL_RES_ADDR(x)               ((volatile unsigned int *)(IFX_PPE + (((x) + 0x0040) << 2)))
#define PPE_CLOCK_CONTROL_ADDR(x)               ((volatile unsigned int *)(IFX_PPE + (((x) + 0x0100) << 2)))
#define CDM_CODE_MEMORY_RAM0_ADDR(x)            ((volatile unsigned int *)(IFX_PPE + (((x) + 0x1000) << 2)))
#define CDM_CODE_MEMORY_RAM1_ADDR(x)            ((volatile unsigned int *)(IFX_PPE + (((x) + 0x2000) << 2)))
#define PPE_REG_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0x4000) << 2)))
#define PP32_DATA_MEMORY_RAM1_ADDR(x)           ((volatile unsigned int *)(IFX_PPE + (((x) + 0x5000) << 2)))
#define PPM_INT_UNIT_ADDR(x)                    ((volatile unsigned int *)(IFX_PPE + (((x) + 0x6000) << 2)))
#define PPM_TIMER0_ADDR(x)                      ((volatile unsigned int *)(IFX_PPE + (((x) + 0x6100) << 2)))
#define PPM_TASK_IND_REG_ADDR(x)                ((volatile unsigned int *)(IFX_PPE + (((x) + 0x6200) << 2)))
#define PPS_BRK_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0x6300) << 2)))
#define PPM_TIMER1_ADDR(x)                      ((volatile unsigned int *)(IFX_PPE + (((x) + 0x6400) << 2)))
#define SB_RAM0_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0x8000) << 2)))
#define SB_RAM1_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0x8800) << 2)))
#define SB_RAM2_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0x9000) << 2)))
#define SB_RAM3_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0x9800) << 2)))
#define SB_RAM4_ADDR(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0xA000) << 2)))
#define QSB_CONF_REG(x)                         ((volatile unsigned int *)(IFX_PPE + (((x) + 0xC000) << 2)))

/*
 *  DWORD-Length of Memory Blocks
 */
#define PP32_DEBUG_REG_DWLEN                    0x0030
#define PPM_INT_REG_DWLEN                       0x0010
#define PP32_INTERNAL_RES_DWLEN                 0x00C0
#define PPE_CLOCK_CONTROL_DWLEN                 0x0F00
#define CDM_CODE_MEMORY_RAM0_DWLEN              0x1000
#define CDM_CODE_MEMORY_RAM1_DWLEN              0x1000
#define PPE_REG_DWLEN                           0x1000
#define PP32_DATA_MEMORY_RAM1_DWLEN             CDM_CODE_MEMORY_RAM1_DWLEN
#define PPM_INT_UNIT_DWLEN                      0x0100
#define PPM_TIMER0_DWLEN                        0x0100
#define PPM_TASK_IND_REG_DWLEN                  0x0100
#define PPS_BRK_DWLEN                           0x0100
#define PPM_TIMER1_DWLEN                        0x0100
#define SB_RAM0_DWLEN                           0x0800
#define SB_RAM1_DWLEN                           0x0800
#define SB_RAM2_DWLEN                           0x0800
#define SB_RAM3_DWLEN                           0x0800
#define SB_RAM4_DWLEN                           0x0C00
#define QSB_CONF_REG_DWLEN                      0x0100


/*
 *  Host-PPE Communication Data Address Mapping
 */
#define SB_BUFFER(__sb_addr)    ( (volatile unsigned int *) ( ( ( (__sb_addr) >= 0x0000 ) && ( (__sb_addr) <= 0x0FFF ) )  ?  PPE_REG_ADDR(__sb_addr) :                           \
                                                              ( ( (__sb_addr) >= 0x1000 ) && ( (__sb_addr) <= 0x1FFF ) )  ?  PP32_DATA_MEMORY_RAM1_ADDR((__sb_addr) - 0x1000) :  \
                                                              ( ( (__sb_addr) >= 0x2000 ) && ( (__sb_addr) <= 0x27FF ) )  ?  SB_RAM0_ADDR((__sb_addr) - 0x2000) :                \
                                                              ( ( (__sb_addr) >= 0x2800 ) && ( (__sb_addr) <= 0x2FFF ) )  ?  SB_RAM1_ADDR((__sb_addr) - 0x2800) :                \
                                                              ( ( (__sb_addr) >= 0x3000 ) && ( (__sb_addr) <= 0x37FF ) )  ?  SB_RAM2_ADDR((__sb_addr) - 0x3000) :                \
                                                              ( ( (__sb_addr) >= 0x3800 ) && ( (__sb_addr) <= 0x3FFF ) )  ?  SB_RAM3_ADDR((__sb_addr) - 0x3800) :                \
                                                              ( ( (__sb_addr) >= 0x4000 ) && ( (__sb_addr) <= 0x4BFF ) )  ?  SB_RAM4_ADDR((__sb_addr) - 0x4000) :                \
                                                          0 ) )

#define FW_VER_ID                               ((volatile struct fw_ver_id *)              SB_BUFFER(0x2000))
#define FW_VER_FEATURE                                                                      SB_BUFFER(0x2001)

#define PS_MC_GENCFG3                           ((volatile struct ps_mc_cfg *)              SB_BUFFER(0x2003))   //  power save and multicast gen config

#define CFG_STD_DATA_LEN                        ((volatile struct cfg_std_data_len *)       SB_BUFFER(0x2011))
#define TX_QOS_CFG                              ((volatile struct tx_qos_cfg *)             SB_BUFFER(0x2012))
#define EG_BWCTRL_CFG                           ((volatile struct eg_bwctrl_cfg *)          SB_BUFFER(0x2013))
#define CFG_WRDES_DELAY                         SB_BUFFER(0x2014)   /*  WAN Descriptor Write Delay, must be configured before enable PPE firmware.          */
#define WRX_EMACH_ON                            SB_BUFFER(0x2015)   /*  WAN RX EMA Channel Enable (0 - 1), must be configured before enable PPE firmware.   */
#define WTX_EMACH_ON                            SB_BUFFER(0x2016)   /*  WAN TX EMA Channel Enable (0 - 14), must be configured before enable PPE firmware.   */
#define WRX_HUNT_BITTH                          SB_BUFFER(0x2017)   /*  WAN RX HUNT Threshold, must be between 2 to 8.                                      */
#define PTM_CRC_CFG                             ((volatile struct ptm_crc_cfg *)            SB_BUFFER(0x2018))
#define CFG_WAN_PORTMAP                         SB_BUFFER(0x201A)
#define CFG_MIXED_PORTMAP                       SB_BUFFER(0x201B)
#define TX_QOS_WFQ_RELOAD_MAP                   SB_BUFFER(0x2020)
#define PSEUDO_IPv4_BASE_ADDR                   SB_BUFFER(0x2023)
#define MIXED_WAN_VLAN_CFG                      ((volatile struct mixed_wan_vlan_cfg *)     SB_BUFFER(0x2024))
#define LAN_ROUT_TBL_CFG                        ((volatile struct rout_tbl_cfg *)           SB_BUFFER(0x2026))
#define WAN_ROUT_TBL_CFG                        ((volatile struct rout_tbl_cfg *)           SB_BUFFER(0x2027))
#define GEN_MODE_CFG1                           ((volatile struct gen_mode_cfg1 *)          SB_BUFFER(0x2028))
#define GEN_MODE_CFG                            GEN_MODE_CFG1
#define GEN_MODE_CFG2                           ((volatile struct gen_mode_cfg2 *)          SB_BUFFER(0x2029))
#define ETH1_BRG_PMAC_HD                        SB_BUFFER(0x202A)
#define KEY_SEL_n(i)                            SB_BUFFER(0x202C + (i))

#define __QOS_NEW_FEATURE                       SB_BUFFER(0x325A)
#define MAX_SHAPER_NUM                          8
#define WTX_EG_Q_DEF_SHAPER_ADDR                (0x3BD4)
#define WTX_EG_Q_SHAPER_ADDR(i)                 (0x3BD8 + ((i) << 2))      /* 0 <= i < 8  0x3BD8 - 0x3BF4 */
#define WTX_EG_Q_PORT_SHAPER_ADDR(i)            (0x3BD0 + ((i) << 2))      /*  i < 1 */
#define WTX_EG_Q_BC_PORT_SHAPING_ADDR(i)        (0x3BF8 + ((i) << 2))      /*  i < 2 */ 
#define WTX_EG_Q_SHAPER_ID(qid)                (int)((WTX_EG_Q_SHAPER_MAPPING_CFG(qid)->shaper_base_addr - WTX_EG_Q_SHAPER_ADDR(0)) >> 2)
#define WTX_QOS_Q_DESC_CFG(i)                   ((volatile struct wtx_qos_q_desc_cfg *)     SB_BUFFER(0x3420 + (i) * 2))    /*  i < 8   */
#define WTX_PTM_EG_Q_PORT_SHAPING_CFG(i)        ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(WTX_EG_Q_BC_PORT_SHAPING_ADDR(i)))    /*  i < 2   */
#define WTX_EG_Q_DEF_SHAPING_CFG                ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(WTX_EG_Q_DEF_SHAPER_ADDR))    /*  i < 1   */
#define WTX_EG_Q_SHAPING_CFG(i)                 ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(WTX_EG_Q_SHAPER_ADDR(i)))     /*  i < 8   */
#define WTX_EG_Q_PORT_SHAPING_CFG(i)            ((volatile struct wtx_eg_q_shaping_cfg *)   SB_BUFFER(WTX_EG_Q_PORT_SHAPER_ADDR(i)))    /*  i < 1   */

/* MBR Queue to Shaper Mapping */
#define WTX_EG_Q_SHAPER_MAPPING_CFG(i)          ((volatile struct wtx_q_shaper_mapping_cfg *) SB_BUFFER(0x2348 + (i)))

/* Minimum Bandwidth Reservation Negative BitMap */
#define WTX_EG_Q_SHAPER_WEIGHT_NEGATIVE_MAP     SB_BUFFER(0x3250)

/* MBR Shaper to Queue Mapping */
#define WTX_EG_Q_DEF_SHAPER_QUEUE_MAP           SB_BUFFER(0x3251)
#define WTX_EG_Q_SHAPER_QUEUE_MAP(i)            SB_BUFFER(0x3252 + (i))  /* 0 <= i < 8 */

#define WTX_QOS_Q_MIB_TABLE(i)                  ((volatile struct wtx_qos_q_mib_table *)    SB_BUFFER(0x3B90 + (i) * 8))    /*  i < 8   */

#define WRX_PORT_CONFIG(i)                      ((volatile struct wrx_port_cfg_status *)    SB_BUFFER(0x31E8 + (i) * 20))   /*  i < 2, each entry has 2 DWORDs  */
#define WRX_DMA_CHANNEL_CONFIG(i)               ((volatile struct wrx_dma_channel_cfg *)    SB_BUFFER(0x3A60 + (i) * 7))    /*  i < 2, each entry has 3 DWORDs  */
#define WTX_PORT_CONFIG(i)                      ((volatile struct wtx_port_cfg *)           SB_BUFFER(0x3210 + (i) * 31))   /*  i < 2, each entry has 1 DWORD   */
#define WTX_DMA_CHANNEL_CONFIG(i)               ((volatile struct wtx_dma_channel_cfg *)    SB_BUFFER(0x3211 + (i) * 31))   /*  i < 2, each entry has 3 DWORDs  */
#define PTM_MIB_TABLE(i)                        ((volatile struct ptm_mib_table *)          SB_BUFFER(0x2030 + (i) * 16))   /*  i < 2   */

#define ITF_MIB_TBL(i)                          ((volatile struct itf_mib *)                SB_BUFFER(0x3300 + (i) * 16))   /*  i < 8   */

#define PPPOE_CFG_TBL(i)                        SB_BUFFER(0x3AB0 + (i))         /*  i < 8   */
#define MTU_CFG_TBL(i)                          SB_BUFFER(0x3AB8 + (i))         /*  i < 8   */
#define ROUT_MAC_CFG_TBL(i)                     SB_BUFFER(0x3A90 + (i) * 2)     /*  i < 16  */
#define TUNNEL_6RD_TBL(i)                       SB_BUFFER(0x2088 + (i) * 5)     /*  i < 4    */
#define TUNNEL_DSLITE_TBL(i)                    SB_BUFFER(0x20A0 + (i) * 10)    /*  i < 4    */
#define TUNNEL_MAX_ID                           SB_BUFFER(0x2025)               /* IPv4 Header Identification value */

#define IPv6_IP_IDX_TBL(x, i)                   SB_BUFFER(((x) == 0 ? 0x2500 : 0x2C40) + (i) * 4)   /*  i < 64 */
#define OVERSIZE_HEADER_LEN                     24

#define  CFG_CLASS2QID_MAP(i)                   SB_BUFFER(0x3430 + (i))         /*  i < 4   */
#define  CFG_QID2CLASS_MAP(i)                   SB_BUFFER(0x3434 + (i))         /*  i < 4   */

//-------------------------------------
// Hit Status
//-------------------------------------
#define __IPV4_WAN_HIT_STATUS_HASH_TABLE_BASE       0x3A70
#define __IPV4_WAN_HIT_STATUS_COLLISION_TABLE_BASE  0x3A5C
#define __IPV4_LAN_HIT_STATUS_HASH_TABLE_BASE       0x3A80
#define __IPV4_LAN_HIT_STATUS_COLLISION_TABLE_BASE  0x3A5E
#define __IPV4_WAN_HIT_STATUS_MC_TABLE_BASE         0x3A58
#define __IPV6_HIT_STATUS_TABLE_BASE                0x3A5A

#define ROUT_LAN_HASH_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_LAN_HIT_STATUS_HASH_TABLE_BASE + (i))
#define ROUT_LAN_COLL_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_LAN_HIT_STATUS_COLLISION_TABLE_BASE + (i))
#define ROUT_WAN_HASH_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_WAN_HIT_STATUS_HASH_TABLE_BASE + (i))
#define ROUT_WAN_COLL_HIT_STAT_TBL(i)               SB_BUFFER(__IPV4_WAN_HIT_STATUS_COLLISION_TABLE_BASE + (i))
#define ROUT_WAN_MC_HIT_STAT_TBL(i)                 SB_BUFFER(__IPV4_WAN_HIT_STATUS_MC_TABLE_BASE + (i))

//-------------------------------------
// Compare and Action table
//-------------------------------------
#define __IPV4_WAN_HASH_ROUT_FWDA_TABLE_BASE        0x2DE0  //  16 x 8 = 128
#define __IPV4_LAN_HASH_ROUT_FWDA_TABLE_BASE        0x26A0  //  16 x 8 = 128

#define __IPV4_WAN_COLLISION_ROUT_FWDA_TABLE_BASE   (g_ipv6_acc_en ? 0x2B80 : 0x2BE0)   //  32 with IPv6, 2B80 - 2C3F, 64 without IPv6, 2BE0 - 2D5F
#define __IPV4_LAN_COLLISION_ROUT_FWDA_TABLE_BASE   0x23E0  //  32 with IPv6, 23E0 - 249F, 64 without IPv6, 23E0 - 255F

#define __IPV4_WAN_HASH_ROUT_FWDC_TABLE_BASE        0x2100
#define __IPV4_LAN_HASH_ROUT_FWDC_TABLE_BASE        0x29A0

#define __IPV4_WAN_COLLISION_ROUT_FWDC_TABLE_BASE   0x2B20  //2B20-2B7F
#define __IPV4_LAN_COLLISION_ROUT_FWDC_TABLE_BASE   (g_ipv6_acc_en ? 0x24A0 : 0x2560)   //  32 with IPv6, 24A0 - 24FF, 64 without IPv6, 2560 - 261F

#define __IPV4_ROUT_MULTICAST_FWDC_TABLE_BASE       0x2DA0
#define __IPV4_ROUT_MULTICAST_FWDA_TABLE_BASE       0x2620

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
#define __MULTICAST_RTP_MIB_BASE                    0x2680
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
#define __CAPWAP_CONFIG_TABLE_BASE                  0x3800
#endif


#define ROUT_LAN_HASH_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_LAN_HASH_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_LAN_HASH_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_LAN_HASH_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_LAN_COLL_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(g_ipv4_lan_collision_rout_fwdc_table_base + (i) * 3))
#define ROUT_LAN_COLL_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_LAN_COLLISION_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_WAN_HASH_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_WAN_HASH_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_WAN_HASH_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(__IPV4_WAN_HASH_ROUT_FWDA_TABLE_BASE + (i) * 6))

#define ROUT_WAN_COLL_CMP_TBL(i)                ((volatile struct rout_forward_compare_tbl *)   SB_BUFFER(__IPV4_WAN_COLLISION_ROUT_FWDC_TABLE_BASE + (i) * 3))
#define ROUT_WAN_COLL_ACT_TBL(i)                ((volatile struct rout_forward_action_tbl *)    SB_BUFFER(g_ipv4_wan_collision_rout_fwda_table_base + (i) * 6))

#define ROUT_WAN_MC_CMP_TBL(i)                  ((volatile struct wan_rout_multicast_cmp_tbl *) SB_BUFFER(__IPV4_ROUT_MULTICAST_FWDC_TABLE_BASE + (i) * 2))
#define ROUT_WAN_MC_ACT_TBL(i)                  ((volatile struct wan_rout_multicast_act_tbl *) SB_BUFFER(__IPV4_ROUT_MULTICAST_FWDA_TABLE_BASE + (i) * 2))

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG

#define CAPWAP_CONFIG_TBL(i)                  ((volatile struct capwap_config_tbl *) SB_BUFFER(__CAPWAP_CONFIG_TABLE_BASE + (i) * 18))

#define CAPWAP_CONFIG                         SB_BUFFER(0x201F)        

#endif

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE

#define MULTICAST_RTP_MIB_TBL(i)                  ((volatile struct rtp_sampling_cnt *) SB_BUFFER(__MULTICAST_RTP_MIB_BASE + (i)))

#endif

#define OUTER_VLAN_TBL(i)                       SB_BUFFER(0x3260 + (i))         /*  i < 32  */
#define ROUT_WAN_MC_CNT(i)                      SB_BUFFER(0x2660 + (i))         /*  i < 32  */

#define __CLASSIFICATION_CMP_TBL_BASE           0x22E0  //  32 * 4 dwords
#define __CLASSIFICATION_MSK_TBL_BASE           0x2360  //  32 * 4 dwords
#define __CLASSIFICATION_ACT_TBL_BASE           0x3400  //  32 * 1 dwords

#define CLASSIFICATION_CMP_TBL(i)               SB_BUFFER(__CLASSIFICATION_CMP_TBL_BASE + (i) * 4)  //  i < 32
#define CLASSIFICATION_MSK_TBL(i)               SB_BUFFER(__CLASSIFICATION_MSK_TBL_BASE + (i) * 4)  //  i < 32
#define CLASSIFICATION_ACT_TBL(i)               ((volatile struct classification_act_tbl *)SB_BUFFER(__CLASSIFICATION_ACT_TBL_BASE + (i)))  //  i < 32

/*
 *  Qid configuration (How qid copied from dplus slave to CPU via dma)
 *  0   -   qid configured by HW
 *  1   -   wait until previous packet is sent out before processing new one
 *  2   -   qid configured by FW
*/

#define __DPLUS_QID_CONF_PTR                    SB_BUFFER(0x3438)


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
#define PPA_DEST_LIST_PTM                           0x080

/*
  * SWITCH PORT DEFINITION
  */
#define PPA_PORT_ETH0                   0
#define PPA_PORT_ETH1                   1
#define PPA_PORT_CPU                    3
#define PPA_PORT_EXT1                   4
#define PPA_PORT_EXT2                   5
#define PPA_PORT_DSL0                   6
#define PPA_PORT_DSL1                   7
#define PPA_PORT_PTM                    7
/*
 *  Clock Generation Unit Registers
 */
#define AMAZON_S_CGU                            KSEG1ADDR(0x1F103000)
#define AMAZON_S_CGU_SYS                        ((volatile unsigned int*)(AMAZON_S_CGU + 0x0010))
#define AMAZON_S_CGU_UPDATE                     ((volatile unsigned int*)(AMAZON_S_CGU + 0x0014))
#define AMAZON_S_CGU_IFCCR                      ((volatile unsigned int*)(AMAZON_S_CGU + 0x0018))

/*
 *  Helper Macro
 */
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))
#define BITSIZEOF_UINT32                        (sizeof(uint32_t) * 8)
#define BITSIZEOF_UINT16                        (sizeof(uint16_t) * 8)

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
#define BITSIZEOF_UINT8                         (sizeof(uint8_t) * 8)
#endif



#define MAX_BRIDGING_ENTRIES                    2048


/*
 * Mac table manipulation action
*/
enum{
    MAC_TABLE_ENTRY_ADD = 1,
    MAC_TABLE_ENTRY_REMOVE,
};

/*
 *  WAN QoS MODE List
 */
enum{
   PPA_WAN_QOS_ETH0 = 0,
   PPA_WAN_QOS_ETH1 = 1,
   PPA_WAN_QOS_PTM0 = 7,
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
    unsigned int time_tick               :16;
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    unsigned int    res1                :11;
    unsigned int    session_mib_unit    :1;
#else
    unsigned int    res1                :12;
#endif
    unsigned int class_en                :1; //  switch class enable
    unsigned int ssc_mode                :1;
    unsigned int asc_mode                :1;
    unsigned int psave_en                :1; //only for vrx200
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

  struct cfg_std_data_len {
    unsigned int res1                   :14;
    unsigned int byte_off               :2;     //  byte offset in RX DMA channel
    unsigned int data_len               :16;    //  data length for standard size packet buffer
  };

  struct tx_qos_cfg {
    unsigned int time_tick              :16;    //  number of PP32 cycles per basic time tick
    unsigned int overhd_bytes           :8;     //  number of overhead bytes per packet in rate shaping
    unsigned int eth1_eg_qnum           :4;     //  number of egress QoS queues (< 8);
    unsigned int eth1_burst_chk         :1;     //  always 1, FW use for port rate shaping
    unsigned int eth1_qss               :1;     //  1: FW QoS, 0: HW QoS
    unsigned int shape_en               :1;     //  1: enable rate shaping, 0: disable
    unsigned int wfq_en                 :1;     //  1: WFQ enabled, 0: strict priority enabled
  };

  struct eg_bwctrl_cfg {
    unsigned int fdesc_wm               :16;    //  if free descriptors in QoS/Swap channel is less than this watermark, large size packets are discarded
    unsigned int class_len              :16;    //  if packet length is not less than this value, the packet is recognized as large packet
  };

  struct ptm_crc_cfg {
    /*  0h  */
    unsigned int res0                   :6;
    unsigned int wtx_fcs_crc_gen        :1;
    unsigned int wtx_tc_crc_gen         :1;
    unsigned int wtx_tc_crc_len         :8;
    unsigned int res1                   :5;
    unsigned int wrx_fcs_crc_vld        :1;
    unsigned int wrx_fcs_crc_chk        :1;
    unsigned int wrx_tc_crc_chk         :1;
    unsigned int wrx_tc_crc_len         :8;
  };

  struct mixed_wan_vlan_cfg {
    unsigned int    wan_vlanid_hi       :12;
    unsigned int    wan_vlanid_lo       :12;
    unsigned int    res1                :4;
    unsigned int    eth1_type           :2; //  reserved in AR9
    unsigned int    eth0_type           :2; //  reserved in AR9
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
    unsigned int    app2_indirect       :1; //  0: direct, 1: indirect (default)
    unsigned int    us_indirect         :1; //  0: direct, 1: indirect
    unsigned int    us_early_discard_en :1; //  0: disable, 1: enable
    unsigned int    classification_num  :6; //  classification table entry number
    unsigned int    ipv6_rout_num       :8;
    unsigned int    res1                :2;
    unsigned int    session_ctl_en      :1; //  session based rate control enable, 0: disable, 1: enable
    unsigned int    wan_hash_alg        :1; //  Hash Algorithm for IPv4 WAN ingress traffic, 0: dest port, 1: dest port + dest IP
    unsigned int    brg_class_en        :1; //  Multiple Field Based Classification and VLAN Assignment Enable for Bridging Traffic, 0: disable, 1: enable
    unsigned int    ipv4_mc_acc_mode    :1; //  Downstream IPv4 Multicast Acceleration Mode, 0: dst IP only, 1: Five-Tuple (IP pairs plus port pairs)
    unsigned int    ipv6_acc_en         :1; //  IPv6 Traffic Acceleration Enable, 0: disable, 1: enable
    unsigned int    wan_acc_en          :1; //  WAN Ingress Acceleration Enable, 0: disable, 1: enable
    unsigned int    lan_acc_en          :1; //  LAN Ingress Acceleration Enable, 0: disable, 1: enable
    unsigned int    eth1_rout_class_rul :1; //  Traffic Class Selection for ETH0 Routing Egress Traffic: 0: CPU, 1: Switch
    unsigned int    eth0_rout_class_rul :1; //  Traffic Class Selection for ETH1 Routing Egress Traffic: 0: CPU, 1: Switch
    unsigned int    max_class           :1; //  Maximum Number of Traffic Class, 0: 4 classes, 1: 8 classes
    unsigned int    sw_iso_mode         :1; //  Switch Isolation Mode, 0: not isolated - ETH0/1 treated as single eth interface, 1: isolated - ETH0/1 treated as two eth interfaces
    unsigned int    sys_cfg             :2; //  System Mode, 0: DSL WAN ETH0/1 LAN, 1: res, 2: ETH0 WAN/LAN ETH1 not used, 3: ETH0 LAN ETH1 WAN, replace by CFG_WAN_PORTMAP/CFG_MIXED_PORTMAP
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
    unsigned int    t                   :8;   //Time Tick
    unsigned int    w                   :24;  //Weight
    unsigned int    s                   :16;  //Burst
    unsigned int    r                   :16;  //Replenish
    unsigned int    res1                :8;
    unsigned int    d                   :24;  //ppe internal variable
    unsigned int    res2                :8;
    unsigned int    tick_cnt            :8;   //ppe internal variable
    unsigned int    b                   :16;  //ppe internal variable
  };

  struct wrx_port_cfg_status {
    /* 0h */
    unsigned int mfs                    :16;
    unsigned int res0                   :12;
    unsigned int dmach                  :3;
    unsigned int res1                   :1;
    /* 1h */
    unsigned int res2                   :14;
    unsigned int local_state            :2;     //  init with 0, written by firmware only
    unsigned int res3                   :15;
    unsigned int partner_state          :1;     //  init with 0, written by firmware only
  };

  struct wrx_dma_channel_cfg {
    /*  0h  */
    unsigned int res3                   :1;
    unsigned int res4                   :2;
    unsigned int res5                   :1;
    unsigned int desba                  :28;
    /*  1h  */
    unsigned int res1                   :16;
    unsigned int res2                   :16;
    /*  2h  */
    unsigned int deslen                 :16;
    unsigned int vlddes                 :16;    //  reserved
  };

  struct wtx_port_cfg {
    /* 0h */
    unsigned int tx_cwth2               :8;     //  default value: 4
    unsigned int tx_cwth1               :8;     //  default value: 5
    unsigned int res0                   :8;
    unsigned int queue_map              :8;
  };

  struct wtx_dma_channel_cfg {
    /*  0h  */
    unsigned int res3                   :1;
    unsigned int res4                   :2;
    unsigned int res5                   :1;
    unsigned int desba                  :28;
    /*  1h  */
    unsigned int res1                   :16;
    unsigned int res2                   :16;
    /*  2h  */
    unsigned int deslen                 :16;
    unsigned int vlddes                 :16;//  reserved
  };

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
    unsigned int    mpoa_type           :2; //  not valid for WAN entry, reserved in D5/E5
    unsigned int    dest_qid            :4; //  in D5/E5, dest_qid for both sides, in A5 DSL WAN Qid (PVC) for LAN side, dest_qid for WAN side
    /*  5h  */
    unsigned int    reserved            :8;
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
    unsigned int    dest_qid            :4; //  not implement yet
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

struct wtx_q_shaper_mapping_cfg {
    unsigned int    resv                :16;
    unsigned int    shaper_base_addr    :16;
};

struct wtx_qos_q_mib_table {
    unsigned int    wrx_total_pdu;
    unsigned int    wrx_total_bytes;
    unsigned int    wtx_total_pdu;
    unsigned int    wtx_total_bytes;

    unsigned int    wtx_cpu_drop_small_pdu;
    unsigned int    wtx_cpu_drop_pdu;
    unsigned int    wtx_fast_drop_small_pdu;
    unsigned int    wtx_fast_drop_pdu;
};

struct ptm_mib_table {
    unsigned int    wrx_correct_pdu;            /* 0 */
    unsigned int    wrx_correct_pdu_bytes;      /* 1 */
    unsigned int    wrx_tccrc_err_pdu;          /* 2 */
    unsigned int    wrx_tccrc_err_pdu_bytes;    /* 3 */
    unsigned int    wrx_ethcrc_err_pdu;         /* 4 */
    unsigned int    wrx_ethcrc_err_pdu_bytes;   /* 5 */
    unsigned int    wrx_nodesc_drop_pdu;        /* 6 */
    unsigned int    wrx_len_violation_drop_pdu; /* 7 */
    unsigned int    wrx_idle_bytes;             /* 8 */
    unsigned int    wrx_nonidle_cw;             /* 9 */
    unsigned int    wrx_idle_cw;                /* A */
    unsigned int    wrx_err_cw;                 /* B */
    unsigned int    wtx_total_pdu;              /* C */
    unsigned int    wtx_total_bytes;            /* D */
    unsigned int    res0;                       /* E */
    unsigned int    res1;                       /* F */
};

struct itf_mib {
    unsigned int    ig_fast_brg_pkts;           // 0 bridge (multifield editing)
    unsigned int    ig_fast_brg_bytes;          // 1

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

struct mac_tbl_item {
    struct mac_tbl_item     *next;
    int             ref;

    unsigned char           mac[PPA_ETH_ALEN];
    unsigned int             mac0;
    unsigned int             mac1;
    unsigned int             age;
    unsigned int             timestamp;
};

struct dmrx_dba { //@612
    unsigned int    res                 :18;
    unsigned int    dbase               :14;
};

struct dmrx_cba { //@613
    unsigned int    res                 :18;
    unsigned int    cbase               :14;
};

struct dmrx_cfg { //@614
    unsigned int    sen                 :1;   //bit 31
    unsigned int    res0                :5;
    unsigned int    trlpg               :1;   //bit 25
    unsigned int    hdlen               :7;   //bit 24:18
    unsigned int    res1                :3;
    unsigned int    endian              :1;   //bit 14
    unsigned int    psize               :2;   //bit 13:12
    unsigned int    res2                :4;
    unsigned int    pnum                :8;   //bit 7:0
};

struct dmrx_pgcnt {//@615
    unsigned int    res0                :5;
    unsigned int    pgptr               :8;  //bit 26:19
    unsigned int    dsrc                :2;   //bit 18:17
    unsigned int    dval                :8;   //bit 16:9
    unsigned int    dcmd                :1;   //bit 8
    unsigned int    upage               :8;   //bit 7:0
};


struct dmrx_pktcnt { //@616
    unsigned int    res                 :21;
    unsigned int    dsrc                :2;   //bit 10:9
    unsigned int    dcmd                :1;   //bit 8
    unsigned int    upkt                :8;   //bit 7:0
};

struct dsrx_dba { //@710
    unsigned int    res                 :18;
    unsigned int    dbase               :14;  //bit 13:0
  };

struct dsrx_cba { // @711
    unsigned int    res                 :18;
    unsigned int    cbase               :14;  //bit 13:0
  };

struct dsrx_cfg { // @712
    unsigned int    res0                :16;
    unsigned int    dren                :1;   //bit 15
    unsigned int    endian              :1;   //bit 14
    unsigned int    psize               :2;   //bit 13:12
    unsigned int    res1                :4;
    unsigned int    pnum                :8;   //bit 7:0
  };

  struct dsrx_pgcnt { //@713
    unsigned int    res0                :5;
    unsigned int    pgptr               :8;  //bit 26:19
    unsigned int    isrc                :2;   //bit 18:17
    unsigned int    ival                :8;   //bit 16:9
    unsigned int    icmd                :1;   //bit  8
    unsigned int    upage               :8;   //bit 7:0
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
  struct sll_cmd1 { //@0x900
    unsigned int res0       :8;
    unsigned int mtype      :1;     //    bit 23
    unsigned int esize      :4;     //    bit 22:19
    unsigned int ksize      :4;     //    bit 18:15
    unsigned int res1       :2;
    unsigned int embase     :13;    //    bit 12:0
  };

  struct sll_cmd0 { //@901
    unsigned int res0       :6;
    unsigned int cmd        :1;     //    bit 25
    unsigned int eynum      :9;     //    bit 24:16
    unsigned int res1       :3;
    unsigned int eybase     :13;    //    bit 12:0
  };

  struct sll_result{ //@910
    unsigned int res0       :21;    //
    unsigned int vld        :1;     //bit 10
    unsigned int fo         :1;     //bit 9
    unsigned int index      :9;     //bit 8:0
  };
#endif /* MIPS_PPA_HAL_AR10_E5_H */
