/******************************************************************************
**
** FILE NAME    : ifxmips_ppa_datapath_ar9_d5.c
** PROJECT      : UEIP
** MODULES      : MII0/1 Acceleration Package (AR9 PPA D5)
**
** DATE         : 23 MAR 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : MII0/1 Driver with Acceleration Firmware (D5)
** COPYRIGHT    :   Copyright (c) 2006
**          Infineon Technologies AG
**          Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author                $Comment
** 23 MAR 2008  Xu Liang               Initiate Version
** 23 MAY 2011  A. Schmidt             PHY probing for eth0 and eth1
**                                     basic "ethtool" support
** 10 DEC 2012  Manamohan Shetty       Added the support for RTP and MIB mode Features 
*******************************************************************************/



/*
 * ####################################
 *              Version No.
 * ####################################
 */
#define VER_FAMILY      0x20        //  bit 0: res
                                    //      1: Danube
                                    //      2: Twinpass
                                    //      3: Amazon-SE
                                    //      4: res
                                    //      5: AR9
                                    //      6: VR9
                                    //      7: AR10
#define VER_DRTYPE      0x03        //  bit 0: Normal Data Path driver
                                    //      1: Indirect-Fast Path driver
                                    //      2: HAL driver
                                    //      3: Hook driver
                                    //      4: Stack/System Adaption Layer driver
                                    //      5: PPA API driver
#define VER_INTERFACE   0x03        //  bit 0: MII 0
                                    //      1: MII 1
                                    //      2: ATM WAN
                                    //      3: PTM WAN
#define VER_ACCMODE     0x01        //  bit 0: Routing
                                    //      1: Bridging
#define VER_MAJOR       0
#define VER_MID         1
#define VER_MINOR       2



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/etherdevice.h>  /*  eth_type_trans  */
#include <linux/mii.h>
#include <linux/ethtool.h>      /*  ethtool_cmd     */
#include <linux/if_ether.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <net/xfrm.h>
#include <linux/if_vlan.h>

/*
 *  Chip Specific Head File
 */
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pmu.h>
#include <asm/ifx/ifx_rcu.h>
#include <asm/ifx/ifx_gpio.h>
#include <asm/ifx/ifx_dma_core.h>
#include <asm/ifx/ifx_clk.h>
#ifdef CONFIG_IFX_ETH_FRAMEWORK
  #include <asm/ifx/ifx_eth_framework.h>
#endif
#include <net/ifx_ppa_api.h>
#include <net/ifx_ppa_stack_al.h>
#include <net/ifx_ppa_api_directpath.h>
#include <net/ifx_ppa_ppe_hal.h>
#include "../../ifx_ppa_datapath.h"
#include "ifxmips_ppa_datapath_fw_ar9_d5.h"
#include "ifxmips_ppa_hal_ar9_d5.h"

#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
#include <net/ltq_eth_oam_handler.h>
#endif


/*
 * ####################################
 *   Parameters to Configure PPE
 * ####################################
 */

static int ethwan = 2;
static int wanitf = ~0;

static int ipv6_acc_en = 1;

static int wanqos_en = 8;
static uint32_t lan_port_seperate_enabled=0, wan_port_seperate_enabled=0;

#if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  static int directpath_tx_queue_size = 1524 * 50;
#endif
#if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  static int directpath_tx_queue_pkts = 50;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
  #define MODULE_PARM_ARRAY(a, b)   module_param_array(a, int, NULL, 0)
  #define MODULE_PARM(a, b)         module_param(a, int, 0)
#else
  #define MODULE_PARM_ARRAY(a, b)   MODULE_PARM(a, b)
#endif

MODULE_PARM(ethwan, "i");
MODULE_PARM_DESC(ethwan, "WAN mode, 2 - ETH1 WAN, 1 - ETH0 mixed.");
MODULE_PARM(wanitf, "i");
MODULE_PARM_DESC(wanitf, "WAN interfaces, bit 0 - ETH0, 1 - ETH1, 2 - reserved for CPU0, 3/4/5 - DirectPath, 6/7 - DSL");

MODULE_PARM(ipv6_acc_en, "i");
MODULE_PARM_DESC(ipv6_acc_en, "IPv6 support, 1 - enabled, 0 - disabled.");

MODULE_PARM(wanqos_en, "i");
MODULE_PARM_DESC(wanqos_en, "WAN QoS support, 1 - enabled, 0 - disabled.");

#if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  MODULE_PARM(directpath_tx_queue_size, "i");
  MODULE_PARM_DESC(directpath_tx_queue_size, "directpath TX queue size in total buffer size");
#endif
#if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  MODULE_PARM(directpath_tx_queue_pkts, "i");
  MODULE_PARM_DESC(directpath_tx_queue_pkts, "directpath TX queue size in total packet number");
#endif



/*
 * ####################################
 *              Board Type
 * ####################################
 */

#define BOARD_AR9_REFERENCE                     0x01



/*
 * ####################################
 *              Definition
 * ####################################
 */

#define BOARD_CONFIG                            BOARD_AR9_REFERENCE

#define DEBUG_SKB_SWAP                          0

#if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  #define ENABLE_DIRECTPATH_TX_QUEUE            1
  #define ENABLE_DIRECTPATH_TX_QUEUE_SIZE       directpath_tx_queue_size
#elif defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  #define ENABLE_DIRECTPATH_TX_QUEUE            1
  #define ENABLE_DIRECTPATH_TX_QUEUE_PKTS       directpath_tx_queue_pkts
#endif

#if defined(CONFIG_IFX_PPA_NAPI_ENABLE) && defined(CONFIG_IFX_ETH_FRAMEWORK)
  #define ENABLE_NAPI                           1
#endif

#define ENABLE_MIXED_QOS                        1

#define ENABLE_MII0_BYPASS_DELAY_FIX            1

#define ENABLE_MII1_BYPASS_DELAY_FIX            1

#define ENABLE_SWITCH_FLOW_CONTROL              0

#define ENABLE_STATS_ON_VCC_BASIS               1

#define ENABLE_MY_MEMCPY                        0

#define ENABLE_FW_MODULE_TEST                   0

#define ENABLE_DEBUG                            1

#define ENABLE_ASSERT                           1

#define DEBUG_DUMP_SKB                          1

#define DEBUG_DUMP_FLAG_HEADER                  1

#define DEBUG_DUMP_INIT                         0

#define DEBUG_FIRMWARE_TABLES_PROC              1

#define DEBUG_MEM_PROC                          1

#define DEBUG_PP32_PROC                         1

#define DEBUG_FW_PROC                           1

#define DEBUG_REDIRECT_FASTPATH_TO_CPU          0

#define PPE_MAILBOX_IGU0_INT                    INT_NUM_IM2_IRL23

#define PPE_MAILBOX_IGU1_INT                    INT_NUM_IM2_IRL24

#define MY_ETH0_ADDR                            g_my_ethaddr

//  default board related configuration
//  TODO:
#define MII0_MODE_SETUP                         RGMII_MODE
#if defined(CONFIG_MII1_RGMII_MODE_100MB)
  #define MII1_MODE_SETUP                       RGMII_MODE_100MB
#elif defined(CONFIG_MII1_RGMII_MODE)
  #define MII1_MODE_SETUP                       RGMII_MODE
#elif defined(CONFIG_MII1_AUTONEG)
  #define MII1_MODE_SETUP                       RGMII_MODE_100MB  //  default setting before auto-negotiate
 #ifdef CONFIG_LTQ_VB300
  #define MII1_GPHY_INT_EXT_INT_NO              3
  #define PHY11G_ADDR                           1
 #else
  #define MII1_GPHY_INT_EXT_INT_NO              5
  #define PHY11G_ADDR                           17
 #endif

  #define GPHY_VSC                              0
  #define GPHY_LANTIQ_11G                       1

  #if MII1_GPHY_INT_EXT_INT_NO == 0
    #define MII1_GPHY_INT                       INT_NUM_IM4_IRL30
  #elif MII1_GPHY_INT_EXT_INT_NO == 1
    #define MII1_GPHY_INT                       INT_NUM_IM3_IRL31
  #elif MII1_GPHY_INT_EXT_INT_NO == 2
    #define MII1_GPHY_INT                       INT_NUM_IM1_IRL26
  #elif MII1_GPHY_INT_EXT_INT_NO == 3
    #define MII1_GPHY_INT                       INT_NUM_IM1_IRL0
  #elif MII1_GPHY_INT_EXT_INT_NO == 4
    #define MII1_GPHY_INT                       INT_NUM_IM1_IRL1
  #elif MII1_GPHY_INT_EXT_INT_NO == 5
    #define MII1_GPHY_INT                       INT_NUM_IM1_IRL2
  #endif
#else
  #error you must specify MII1 link state in current stage
#endif

//  specific board related configuration
#if defined (BOARD_CONFIG)
  #if BOARD_CONFIG == BOARD_AR9_REFERENCE
  #endif
#endif

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG
  #define ENABLE_DEBUG_PRINT                    1
  #define DISABLE_INLINE                        1
#else
  #define ENABLE_DEBUG_PRINT                    0
  #define DISABLE_INLINE                        0
#endif

#if !defined(DISABLE_INLINE) || !DISABLE_INLINE
  #define INLINE                                inline
#else
  #define INLINE
#endif

#define err(format, arg...)                     do { if ( (g_dbg_enable & DBG_ENABLE_MASK_ERR) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )

#if defined(ENABLE_DEBUG_PRINT) && ENABLE_DEBUG_PRINT
  #undef  dbg
  #define dbg(format, arg...)                   do { if ( (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT) ) printk(KERN_WARNING __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #if !defined(dbg)
    #define dbg(format, arg...)
  #endif
#endif

#if defined(ENABLE_ASSERT) && ENABLE_ASSERT
  #define ASSERT(cond, format, arg...)          do { if ( (g_dbg_enable & DBG_ENABLE_MASK_ASSERT) && !(cond) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define ASSERT(cond, format, arg...)
#endif

#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  #define DUMP_SKB_LEN                          ~0
#endif

#if (defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB)                     \
    || (defined(DEBUG_DUMP_FLAG_HEADER) && DEBUG_DUMP_FLAG_HEADER)  \
    || (defined(DEBUG_DUMP_INIT) && DEBUG_DUMP_INIT)                \
    || (defined(ENABLE_DEBUG_PRINT) && ENABLE_DEBUG_PRINT)          \
    || (defined(ENABLE_ASSERT) && ENABLE_ASSERT)
  #define ENABLE_DBG_PROC                       1
#else
  #define ENABLE_DBG_PROC                       0
#endif

/*
 *  Debug Print Mask
 */
#define DBG_ENABLE_MASK_ERR                     (1 << 0)
#define DBG_ENABLE_MASK_DEBUG_PRINT             (1 << 1)
#define DBG_ENABLE_MASK_ASSERT                  (1 << 2)
#define DBG_ENABLE_MASK_DUMP_SKB_RX             (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX             (1 << 9)
#define DBG_ENABLE_MASK_DUMP_FLAG_HEADER        (1 << 10)
#define DBG_ENABLE_MASK_DUMP_INIT               (1 << 11)
#define DBG_ENABLE_MASK_ALL                     (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT \
                                                | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX                 \
                                                | DBG_ENABLE_MASK_DUMP_FLAG_HEADER | DBG_ENABLE_MASK_DUMP_INIT)

/*
 *  Mailbox Signal Bit
 */
#define DMA_TX_CH1_SIG                          (1 << 31)   //  IGU0
#define DMA_TX_CH0_SIG                          (1 << 30)   //  IGU0
#define CPU_TO_WAN_TX_SIG                       (1 << 17)   //  IGU1
#define CPU_TO_WAN_SWAP_SIG                     (1 << 16)   //  IGU1

/*
 *  Eth Mode
 */
#define RGMII_MODE                              0
#define MII_MODE                                1
#define REV_MII_MODE                            2
#define RED_MII_MODE_IC                         3   //  Input clock
#define RGMII_MODE_100MB                        4
#define TURBO_REV_MII_MODE                      6   //  Turbo Rev Mii mode
#define RED_MII_MODE_OC                         7   //  Output clock
#define RGMII_MODE_10MB                         8

/*
 *  GPIO Related
 */
#define IFX_PPA_GPIO_OP(op, pin)                op((pin), IFX_GPIO_MODULE_PPA)
#define IFX_SWITCH_PIN_RESERVE(pin)             IFX_PPA_GPIO_OP(ifx_gpio_pin_reserve, pin)
#define IFX_SWITCH_PIN_FREE(pin)                IFX_PPA_GPIO_OP(ifx_gpio_pin_free, pin)
#define IFX_SWITCH_DIR_OUT(pin)                 IFX_PPA_GPIO_OP(ifx_gpio_dir_out_set, pin)
#define IFX_SWITCH_DIR_IN(pin)                  IFX_PPA_GPIO_OP(ifx_gpio_dir_in_set, pin)
#define IFX_SWITCH_OUTPUT_SET(pin)              IFX_PPA_GPIO_OP(ifx_gpio_output_set, pin)
#define IFX_SWITCH_OUTPUT_CLR(pin)              IFX_PPA_GPIO_OP(ifx_gpio_output_clear, pin)
#define IFX_SWITCH_ALTSEL0_SET(pin)             IFX_PPA_GPIO_OP(ifx_gpio_altsel0_set, pin)
#define IFX_SWITCH_ALTSEL0_CLR(pin)             IFX_PPA_GPIO_OP(ifx_gpio_altsel0_clear, pin)
#define IFX_SWITCH_OD_SET(pin)                  IFX_PPA_GPIO_OP(ifx_gpio_open_drain_set, pin)
#define IFX_SWITCH_OD_CLR(pin)                  IFX_PPA_GPIO_OP(ifx_gpio_open_drain_clear, pin)
#define IFX_SWITCH_ALTSEL1_SET(pin)             IFX_PPA_GPIO_OP(ifx_gpio_altsel1_set, pin)
#define IFX_SWITCH_ALTSEL1_CLR(pin)             IFX_PPA_GPIO_OP(ifx_gpio_altsel1_clear, pin)
#define IFX_SWITCH_PUDSEL_SET(pin)              IFX_PPA_GPIO_OP(ifx_gpio_pudsel_set, pin)
#define IFX_SWITCH_PUDSEL_CLEAR(pin)            IFX_PPA_GPIO_OP(ifx_gpio_pudsel_clear, pin)
#define IFX_SWITCH_PUDEN_SET(pin)               IFX_PPA_GPIO_OP(ifx_gpio_puden_set, pin)
#define SWITCH_MDIO                             42  //  P2.10
#define SWITCH_MDC                              43  //  P2.11
#define MII0_COL                                32  //  P2.0
#define MII0_CRS                                33  //  P2.1
#define MII0_TXERR                              40  //  P2.8
#define MII0_RXERR                              41  //  P2.9
#define MII1_COL                                44  //  P2.12
#define MII1_CRS                                47  //  P2.15
#define MII1_TXERR                              45  //  P2.13
#define MII1_RXERR                              46  //  P2.14
#define CLOCK_OUT2                              3   //  P0.3
#define RESET_GPHY                              32  //  P2.0

/*
 *  Constant Definition
 */
#define ETH_WATCHDOG_TIMEOUT                    (10 * HZ)
#define ETOP_MDIO_DELAY                         1
#define IDLE_CYCLE_NUMBER                       30000

#define DMA_PACKET_SIZE                         1600    //  ((1518 + 8 <2 VLAN> + 62 <PPE FW> + 8 <SW Header>) + 31) & ~31
#define DMA_ALIGNMENT                           32

#define FWCODE_ROUTING_ACC_D2                   0x02
#define FWCODE_BRIDGING_ACC_D3                  0x03
#define FWCODE_ROUTING_BRIDGING_ACC_D4          0x04
#define FWCODE_ROUTING_BRIDGING_ACC_A4          0x14
#define FWCODE_ROUTING_ACC_D5                   0x05
#define FWCODE_ROUTING_ACC_A5                   0x15

/*
 *  Ethernet Frame Definitions
 */
#define ETH_CRC_LENGTH                          4
#define ETH_MAX_DATA_LENGTH                     ETH_DATA_LEN
#define ETH_MIN_TX_PACKET_LENGTH                ETH_ZLEN

/*
 *  EMA Settings
 */
#define EMA_CMD_BUF_LEN                         0x0010
#define EMA_CMD_BASE_ADDR                       (0x1710 << 2)
#define EMA_DATA_BUF_LEN                        0x0040
#define EMA_DATA_BASE_ADDR                      (0x16d0 << 2)
#define EMA_WRITE_BURST                         0x02
#define EMA_READ_BURST                          0x02

/*
 *  Firmware Settings
 */

#define CPU_TO_WAN_TX_DESC_NUM                  32  //  WAN CPU TX
#define ETH_WAN_TX_DESC_NUM(i)                  __ETH_WAN_TX_QUEUE_LEN  //  ETH WAN QoS TX, i < __ETH_WAN_TX_QUEUE_NUM
#define CPU_TO_WAN_SWAP_DESC_NUM                32
#define WAN_TX_DESC_NUM_TOTAL                   448
#define DMA_TX_CH1_DESC_NUM                     (g_wanqos_en ? 32 : DMA_RX_CH1_DESC_NUM)
#define DMA_RX_CH1_DESC_NUM                     32
#define DMA_RX_CH2_DESC_NUM                     32
#define DMA_TX_CH0_DESC_NUM                     DMA_RX_CH2_DESC_NUM //  DMA_TX_CH0_DESC_NUM is used to replace DMA_TX_CH2_DESC_NUM

#define IFX_PPA_PORT_NUM                        8

#define MAX_DEFAULT_ITF_VLAN_MAP_ENTRIES        8
#define MAX_SRC_IP_VLAN_MAP_ENTRIES             4
#define MAX_ETHTYPE_VLAN_MAP_ENTRIES            4
#define MAX_VLAN_VLAN_MAP_ENTRIES               8

/*
 *  Bits Operation
 */
//#define GET_BITS(x, msb, lsb)                   (((x) & ((1 << ((msb) + 1)) - 1)) >> (lsb))
//#define SET_BITS(x, msb, lsb, value)            (((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1))) | (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))

/*
 *  Internal Tantos Switch Register
 */
#define AMAZON_S_SW                             (KSEG1 | 0x1E108000)
#define AMAZON_S_SW_REG(off)                    ((volatile u32*)(AMAZON_S_SW + (off)))
#define AMAZON_S_SW_PS                          AMAZON_S_SW_REG(0x000)
#define AMAZON_S_SW_P0_CTL                      AMAZON_S_SW_REG(0x004)
#define AMAZON_S_SW_P1_CTL                      AMAZON_S_SW_REG(0x008)
#define AMAZON_S_SW_P2_CTL                      AMAZON_S_SW_REG(0x00C)
#define AMAZON_S_SW_P0_VLAN                     AMAZON_S_SW_REG(0x010)
#define AMAZON_S_SW_P1_VLAN                     AMAZON_S_SW_REG(0x014)
#define AMAZON_S_SW_P2_VLAN                     AMAZON_S_SW_REG(0x018)
#define AMAZON_S_SW_P0_INCTL                    AMAZON_S_SW_REG(0x020)
#define AMAZON_S_SW_P1_INCTL                    AMAZON_S_SW_REG(0x024)
#define AMAZON_S_SW_P2_INCTL                    AMAZON_S_SW_REG(0x028)
#define AMAZON_S_SW_DF_PORTMAP                  AMAZON_S_SW_REG(0x02C)
#define AMAZON_S_SW_P0_ECS_Q32                  AMAZON_S_SW_REG(0x030)
#define AMAZON_S_SW_P0_ECS_Q10                  AMAZON_S_SW_REG(0x034)
#define AMAZON_S_SW_P0_ECW_Q32                  AMAZON_S_SW_REG(0x038)
#define AMAZON_S_SW_P0_ECW_Q10                  AMAZON_S_SW_REG(0x03C)
#define AMAZON_S_SW_P1_ECS_Q32                  AMAZON_S_SW_REG(0x040)
#define AMAZON_S_SW_P1_ECS_Q10                  AMAZON_S_SW_REG(0x044)
#define AMAZON_S_SW_P1_ECW_Q32                  AMAZON_S_SW_REG(0x048)
#define AMAZON_S_SW_P1_ECW_Q10                  AMAZON_S_SW_REG(0x04C)
#define AMAZON_S_SW_P2_ECS_Q32                  AMAZON_S_SW_REG(0x050)
#define AMAZON_S_SW_P2_ECS_Q10                  AMAZON_S_SW_REG(0x054)
#define AMAZON_S_SW_P2_ECW_Q32                  AMAZON_S_SW_REG(0x058)
#define AMAZON_S_SW_P2_ECW_Q10                  AMAZON_S_SW_REG(0x05C)
#define AMAZON_S_SW_INT_ENA                     AMAZON_S_SW_REG(0x060)
#define AMAZON_S_SW_INT_ST                      AMAZON_S_SW_REG(0x064)
#define AMAZON_S_SW_GCTL0                       AMAZON_S_SW_REG(0x068)
#define AMAZON_S_SW_GCTL1                       AMAZON_S_SW_REG(0x06C)
#define AMAZON_S_SW_ARP                         AMAZON_S_SW_REG(0x070)
#define AMAZON_S_SW_STRM_CTL                    AMAZON_S_SW_REG(0x074)
#define AMAZON_S_SW_RGMII_CTL                   AMAZON_S_SW_REG(0x078)
#define AMAZON_S_SW_1P_PRT                      AMAZON_S_SW_REG(0x07C)
#define AMAZON_S_SW_GBKT_SZBS                   AMAZON_S_SW_REG(0x080)
#define AMAZON_S_SW_GBKT_SZEBS                  AMAZON_S_SW_REG(0x084)
#define AMAZON_S_SW_BF_TH                       AMAZON_S_SW_REG(0x088)
#define AMAZON_S_SW_PMAC_HD_CTL                 AMAZON_S_SW_REG(0x08C)
#define AMAZON_S_SW_PMAC_SA1                    AMAZON_S_SW_REG(0x090)
#define AMAZON_S_SW_PMAC_SA2                    AMAZON_S_SW_REG(0x094)
#define AMAZON_S_SW_PMAC_DA1                    AMAZON_S_SW_REG(0x098)
#define AMAZON_S_SW_PMAC_DA2                    AMAZON_S_SW_REG(0x09C)
#define AMAZON_S_SW_PMAC_VLAN                   AMAZON_S_SW_REG(0x0A0)
#define AMAZON_S_SW_PMAC_TX_IPG                 AMAZON_S_SW_REG(0x0A4)
#define AMAZON_S_SW_PMAC_RX_IPG                 AMAZON_S_SW_REG(0x0A8)
#define AMAZON_S_SW_ADR_TB_CTL0                 AMAZON_S_SW_REG(0x0AC)
#define AMAZON_S_SW_ADR_TB_CTL1                 AMAZON_S_SW_REG(0x0B0)
#define AMAZON_S_SW_ADR_TB_CTL2                 AMAZON_S_SW_REG(0x0B4)
#define AMAZON_S_SW_ADR_TB_ST0                  AMAZON_S_SW_REG(0x0B8)
#define AMAZON_S_SW_ADR_TB_ST1                  AMAZON_S_SW_REG(0x0BC)
#define AMAZON_S_SW_ADR_TB_ST2                  AMAZON_S_SW_REG(0x0C0)
#define AMAZON_S_SW_RMON_CTL                    AMAZON_S_SW_REG(0x0C4)
#define AMAZON_S_SW_RMON_ST                     AMAZON_S_SW_REG(0x0C8)
#define AMAZON_S_SW_MDIO_CTL                    AMAZON_S_SW_REG(0x0CC)
#define AMAZON_S_SW_MDIO_DATA                   AMAZON_S_SW_REG(0x0D0)
#define AMAZON_S_SW_TP_FLT_ACT                  AMAZON_S_SW_REG(0x0D4)
#define AMAZON_S_SW_PRTCL_FLT_ACT               AMAZON_S_SW_REG(0x0D8)
#define AMAZON_S_SW_VLAN_FLT0                   AMAZON_S_SW_REG(0x100)
#define AMAZON_S_SW_VLAN_FLT1                   AMAZON_S_SW_REG(0x104)
#define AMAZON_S_SW_VLAN_FLT2                   AMAZON_S_SW_REG(0x108)
#define AMAZON_S_SW_VLAN_FLT3                   AMAZON_S_SW_REG(0x10C)
#define AMAZON_S_SW_VLAN_FLT4                   AMAZON_S_SW_REG(0x110)
#define AMAZON_S_SW_VLAN_FLT5                   AMAZON_S_SW_REG(0x114)
#define AMAZON_S_SW_VLAN_FLT6                   AMAZON_S_SW_REG(0x118)
#define AMAZON_S_SW_VLAN_FLT7                   AMAZON_S_SW_REG(0x11C)
#define AMAZON_S_SW_VLAN_FLT8                   AMAZON_S_SW_REG(0x120)
#define AMAZON_S_SW_VLAN_FLT9                   AMAZON_S_SW_REG(0x124)
#define AMAZON_S_SW_VLAN_FLT10                  AMAZON_S_SW_REG(0x128)
#define AMAZON_S_SW_VLAN_FLT11                  AMAZON_S_SW_REG(0x12C)
#define AMAZON_S_SW_VLAN_FLT12                  AMAZON_S_SW_REG(0x130)
#define AMAZON_S_SW_VLAN_FLT13                  AMAZON_S_SW_REG(0x134)
#define AMAZON_S_SW_VLAN_FLT14                  AMAZON_S_SW_REG(0x138)
#define AMAZON_S_SW_VLAN_FLT15                  AMAZON_S_SW_REG(0x13C)
#define AMAZON_S_SW_TP_FLT10                    AMAZON_S_SW_REG(0x140)
#define AMAZON_S_SW_TP_FLT32                    AMAZON_S_SW_REG(0x144)
#define AMAZON_S_SW_TP_FLT54                    AMAZON_S_SW_REG(0x148)
#define AMAZON_S_SW_TP_FLT76                    AMAZON_S_SW_REG(0x14C)
#define AMAZON_S_SW_DFSRV_MAP0                  AMAZON_S_SW_REG(0x150)
#define AMAZON_S_SW_DFSRV_MAP1                  AMAZON_S_SW_REG(0x154)
#define AMAZON_S_SW_DFSRV_MAP2                  AMAZON_S_SW_REG(0x158)
#define AMAZON_S_SW_DFSRV_MAP3                  AMAZON_S_SW_REG(0x15C)
#define AMAZON_S_SW_TCP_PF0                     AMAZON_S_SW_REG(0x160)
#define AMAZON_S_SW_TCP_PF1                     AMAZON_S_SW_REG(0x164)
#define AMAZON_S_SW_TCP_PF2                     AMAZON_S_SW_REG(0x168)
#define AMAZON_S_SW_TCP_PF3                     AMAZON_S_SW_REG(0x16C)
#define AMAZON_S_SW_TCP_PF4                     AMAZON_S_SW_REG(0x170)
#define AMAZON_S_SW_TCP_PF5                     AMAZON_S_SW_REG(0x174)
#define AMAZON_S_SW_TCP_PF6                     AMAZON_S_SW_REG(0x178)
#define AMAZON_S_SW_TCP_PF7                     AMAZON_S_SW_REG(0x17C)
#define AMAZON_S_SW_RA_03_00                    AMAZON_S_SW_REG(0x180)
#define AMAZON_S_SW_RA_07_04                    AMAZON_S_SW_REG(0x184)
#define AMAZON_S_SW_RA_0B_08                    AMAZON_S_SW_REG(0x188)
#define AMAZON_S_SW_RA_0F_0C                    AMAZON_S_SW_REG(0x18C)
#define AMAZON_S_SW_RA_13_10                    AMAZON_S_SW_REG(0x190)
#define AMAZON_S_SW_RA_17_14                    AMAZON_S_SW_REG(0x194)
#define AMAZON_S_SW_RA_1B_18                    AMAZON_S_SW_REG(0x198)
#define AMAZON_S_SW_RA_1F_1C                    AMAZON_S_SW_REG(0x19C)
#define AMAZON_S_SW_RA_23_20                    AMAZON_S_SW_REG(0x1A0)
#define AMAZON_S_SW_RA_27_24                    AMAZON_S_SW_REG(0x1A4)
#define AMAZON_S_SW_RA_2B_28                    AMAZON_S_SW_REG(0x1A8)
#define AMAZON_S_SW_RA_2F_2C                    AMAZON_S_SW_REG(0x1AC)
#define AMAZON_S_SW_F0                          AMAZON_S_SW_REG(0x1B0)
#define AMAZON_S_SW_F1                          AMAZON_S_SW_REG(0x1B4)

/*
 *  Internal Tantos Switch Related
 */
#define MDIO_OP_READ                            (2 << 10)
#define MDIO_OP_WRITE                           (1 << 10)
#define TANTOS_CHIP_ID                          0x2599

#define PHYID_IFX_PHY11G                        0x030260D1
#define PHYID_LANTIQ_PHY11G                     0xD565A400
#define PHYID_ICPLUS_IP101A                     0x02430C54
#define PHYID_VSC_SIMPLIPHY                     0x00070421

#define MII_IMASK                               0x19
#define MII_ISTAT                               0x1A
/*
 *  DMA/EMA Descriptor Base Address
 */
#define CPU_TO_WAN_TX_DESC_BASE                 ((volatile struct tx_descriptor *)SB_BUFFER(0x3C00))                /*         32 each queue    */
#define __ETH_WAN_TX_QUEUE_NUM                  g_wanqos_en
#define __ETH_VIRTUAL_TX_QUEUE_NUM              8   //  LAN interface, there is no real firmware queue, but only virtual queue maintained by driver, so that skb->priority => firmware QId => switch class can be mapped.
#define SWITCH_CLASS_NUM                        4   //  AR9 switch has 4 classes.
#define __ETH_WAN_TX_QUEUE_LEN                  (WAN_TX_DESC_NUM_TOTAL / __ETH_WAN_TX_QUEUE_NUM)
#define __ETH_WAN_TX_DESC_BASE(i)               (0x3C80 + (i) * 2 * __ETH_WAN_TX_QUEUE_LEN)
#define ETH_WAN_TX_DESC_BASE(i)                 ((volatile struct tx_descriptor *)SB_BUFFER(__ETH_WAN_TX_DESC_BASE(i))) /*  i < __ETH_WAN_TX_QUEUE_NUM, __ETH_WAN_TX_QUEUE_LEN each queue    */
#define CPU_TO_WAN_SWAP_DESC_BASE               ((volatile struct tx_descriptor *)SB_BUFFER(0x3C40))                /*         32 each queue    */
#define DMA_TX_CH1_DESC_BASE                    (g_wanqos_en ? ((volatile struct rx_descriptor *)SB_BUFFER(0x3AC0)) : DMA_RX_CH1_DESC_BASE)
#define DMA_RX_CH1_DESC_BASE                    ((volatile struct rx_descriptor *)SB_BUFFER(0x3B40))                /*         32 each queue    */
#define DMA_RX_CH2_DESC_BASE                    ((volatile struct rx_descriptor *)SB_BUFFER(0x3B00))                /*         32 each queue    */
#define DMA_TX_CH0_DESC_BASE                    DMA_RX_CH2_DESC_BASE

#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
/*
 *  Firmware Proc
 */
//  need check with hejun
  #define TIR(i)                                ((volatile u32 *)(0xBE198800) + (i))

  #define __CPU_TX_SWAPPER_DROP_PKTS            SB_BUFFER(0x29A8)
  #define __CPU_TX_SWAPPER_DROP_BYTES           SB_BUFFER(0x29A9)
  #define __DSLWAN_FP_SWAPPER_DROP_PKTS         SB_BUFFER(0x29AA)
  #define __DSLWAN_FP_SWAPPER_DROP_BYTES        SB_BUFFER(0x29AB)

  #define __CPU_TXDES_SWAP_RDPTR                SB_BUFFER(0x29AC)
  #define __DSLWAN_FP_RXDES_SWAP_RDPTR          SB_BUFFER(0x29AD)
  #define __DSLWAN_FP_RXDES_DPLUS_WRPTR         SB_BUFFER(0x29AE)

  #define __DSLWAN_TX_PKT_CNT0                  SB_BUFFER(0x29B0)
  #define __DSLWAN_TX_PKT_CNT1                  SB_BUFFER(0x29B1)
  #define __DSLWAN_TX_PKT_CNT2                  SB_BUFFER(0x29B2)
  #define __DSLWAN_TX_PKT_CNT3                  SB_BUFFER(0x29B3)
  #define __DSLWAN_TX_PKT_CNT4                  SB_BUFFER(0x29B4)
  #define __DSLWAN_TX_PKT_CNT5                  SB_BUFFER(0x29B5)
  #define __DSLWAN_TX_PKT_CNT6                  SB_BUFFER(0x29B6)
  #define __DSLWAN_TX_PKT_CNT7                  SB_BUFFER(0x29B7)

  #define __DSLWAN_TXDES_SWAP_PTR0              SB_BUFFER(0x29B8)
  #define __DSLWAN_TXDES_SWAP_PTR1              SB_BUFFER(0x29B9)
  #define __DSLWAN_TXDES_SWAP_PTR2              SB_BUFFER(0x29BA)
  #define __DSLWAN_TXDES_SWAP_PTR3              SB_BUFFER(0x29BB)
  #define __DSLWAN_TXDES_SWAP_PTR4              SB_BUFFER(0x29BC)
  #define __DSLWAN_TXDES_SWAP_PTR5              SB_BUFFER(0x29BD)
  #define __DSLWAN_TXDES_SWAP_PTR6              SB_BUFFER(0x29BE)
  #define __DSLWAN_TXDES_SWAP_PTR7              SB_BUFFER(0x29BF)

  // SB_BUFFER(0x29C0) - SB_BUFFER(0x29C7)
  #define __VLAN_PRI_TO_QID_MAPPING             SB_BUFFER(0x29C0)

  //
  // etx MIB: SB_BUFFER(0x29C8) - SB_BUFFER(0x29CF)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT0       SB_BUFFER(0x29C8)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT1       SB_BUFFER(0x29C9)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT2       SB_BUFFER(0x29CA)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT3       SB_BUFFER(0x29CB)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT4       SB_BUFFER(0x29CC)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT5       SB_BUFFER(0x29CD)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT6       SB_BUFFER(0x29CE)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT7       SB_BUFFER(0x29CF)

  #define __CPU_TO_DSLWAN_TX_PKTS               SB_BUFFER(0x29D0)
  #define __CPU_TO_DSLWAN_TX_BYTES              SB_BUFFER(0x29D1)

  #define ACC_ERX_PID                           SB_BUFFER(0x2B00)
  #define ACC_ERX_PORT_TIMES                    SB_BUFFER(0x2B01)
  #define SLL_ISSUED                            SB_BUFFER(0x2B02)
  #define BMC_ISSUED                            SB_BUFFER(0x2B03)
  #define DPLUS_RX_ON                           SB_BUFFER(0x3283)
  #define ISR_IS                                SB_BUFFER(0x3284)

  #define PRESEARCH_RDPTR                       SB_BUFFER(0x2B06)

  #define SLL_ERX_PID                           SB_BUFFER(0x2B04)
  #define SLL_PKT_CNT                           SB_BUFFER(0x2B08)
  #define SLL_RDPTR                             SB_BUFFER(0x2B0A)

  #define EDIT_PKT_CNT                          SB_BUFFER(0x2B0C)
  #define EDIT_RDPTR                            SB_BUFFER(0x2B0E)

  #define DPLUSRX_PKT_CNT                       SB_BUFFER(0x2B10)
  #define DPLUS_RDPTR                           SB_BUFFER(0x2B12)

  #define SLL_STATE_NULL                        0
  #define SLL_STATE_DA                          1
  #define SLL_STATE_SA                          2
  #define SLL_STATE_DA_BC                       3
  #define SLL_STATE_ROUTER                      4

   #define PRE_DPLUS_PTR                        SB_BUFFER(0x3281)
   #define DPLUS_PTR                            SB_BUFFER(0x3282)
   #define DPLUS_CNT                            SB_BUFFER(0x3280)
#endif

/*
 *  Share Buffer Registers
 */
#define SB_MST_PRI0                             PPE_REG_ADDR(0x0300)
#define SB_MST_PRI1                             PPE_REG_ADDR(0x0301)
//#define SB_MST_SEL                              PPE_REG_ADDR(0x0304)

/*
 *    ETOP MDIO Registers
 */
//#define ETOP_MDIO_CFG                           PPE_REG_ADDR(0x0600)
//#define ETOP_MDIO_ACC                           PPE_REG_ADDR(0x0601)
//#define ETOP_CFG                                PPE_REG_ADDR(0x0602)
//#define ETOP_IG_VLAN_COS                        PPE_REG_ADDR(0x0603)
//#define ETOP_IG_DSCP_COSx(x)                    PPE_REG_ADDR(0x0607 - ((x) & 0x03))
//#define ETOP_IG_PLEN_CTRL0                      PPE_REG_ADDR(0x0608)
//#define ETOP_ISR                                PPE_REG_ADDR(0x060A)
//#define ETOP_IER                                PPE_REG_ADDR(0x060B)
//#define ETOP_VPID                               PPE_REG_ADDR(0x060C)
//#define ENET_MAC_CFG(i)                         PPE_REG_ADDR(0x0610 + ((i) ? 0x40 : 0x00))
//#define ENETS_DBA(i)                            PPE_REG_ADDR(0x0612 + ((i) ? 0x40 : 0x00))
//#define ENETS_CBA(i)                            PPE_REG_ADDR(0x0613 + ((i) ? 0x40 : 0x00))
//#define ENETS_CFG(i)                            PPE_REG_ADDR(0x0614 + ((i) ? 0x40 : 0x00))
//#define ENETS_PGCNT(i)                          PPE_REG_ADDR(0x0615 + ((i) ? 0x40 : 0x00))
//#define ENETS_PKTCNT(i)                         PPE_REG_ADDR(0x0616 + ((i) ? 0x40 : 0x00))
//#define ENETS_BUF_CTRL(i)                       PPE_REG_ADDR(0x0617 + ((i) ? 0x40 : 0x00))
//#define ENETS_COS_CFG(i)                        PPE_REG_ADDR(0x0618 + ((i) ? 0x40 : 0x00))
//#define ENETS_IGDROP(i)                         PPE_REG_ADDR(0x0619 + ((i) ? 0x40 : 0x00))
//#define ENETS_IGERR(i)                          PPE_REG_ADDR(0x061A + ((i) ? 0x40 : 0x00))
//#define ENETS_MAC_DA0(i)                        PPE_REG_ADDR(0x061B + ((i) ? 0x40 : 0x00))
//#define ENETS_MAC_DA1(i)                        PPE_REG_ADDR(0x061C + ((i) ? 0x40 : 0x00))
//#define ENETF_DBA(i)                            PPE_REG_ADDR(0x0630 + ((i) ? 0x40 : 0x00))
//#define ENETF_CBA(i)                            PPE_REG_ADDR(0x0631 + ((i) ? 0x40 : 0x00))
//#define ENETF_CFG(i)                            PPE_REG_ADDR(0x0632 + ((i) ? 0x40 : 0x00))
//#define ENETF_PGCNT(i)                          PPE_REG_ADDR(0x0633 + ((i) ? 0x40 : 0x00))
//#define ENETF_PKTCNT(i)                         PPE_REG_ADDR(0x0634 + ((i) ? 0x40 : 0x00))
//#define ENETF_HFCTRL(i)                         PPE_REG_ADDR(0x0635 + ((i) ? 0x40 : 0x00))
//#define ENETF_TXCTRL(i)                         PPE_REG_ADDR(0x0636 + ((i) ? 0x40 : 0x00))
//#define ENETF_VLCOS0(i)                         PPE_REG_ADDR(0x0638 + ((i) ? 0x40 : 0x00))
//#define ENETF_VLCOS1(i)                         PPE_REG_ADDR(0x0639 + ((i) ? 0x40 : 0x00))
//#define ENETF_VLCOS2(i)                         PPE_REG_ADDR(0x063A + ((i) ? 0x40 : 0x00))
//#define ENETF_VLCOS3(i)                         PPE_REG_ADDR(0x063B + ((i) ? 0x40 : 0x00))
//#define ENETF_EGCOL(i)                          PPE_REG_ADDR(0x063C + ((i) ? 0x40 : 0x00))
//#define ENETF_EGDROP(i)                         PPE_REG_ADDR(0x063D + ((i) ? 0x40 : 0x00))

/*
 *  DPlus Registers
 */
#define DM_RXDB                                 PPE_REG_ADDR(0x0612)
#define DM_RXCB                                 PPE_REG_ADDR(0x0613)
#define DM_RXCFG                                PPE_REG_ADDR(0x0614)
#define DM_RXPGCNT                              PPE_REG_ADDR(0x0615)
#define DM_RXPKTCNT                             PPE_REG_ADDR(0x0616)
#define DS_RXDB                                 PPE_REG_ADDR(0x0710)
#define DS_RXCB                                 PPE_REG_ADDR(0x0711)
#define DS_RXCFG                                PPE_REG_ADDR(0x0712)
#define DS_RXPGCNT                              PPE_REG_ADDR(0x0713)

/*
 *  EMA Registers
 */
#define EMA_CMDCFG                              PPE_REG_ADDR(0x0A00)
#define EMA_DATACFG                             PPE_REG_ADDR(0x0A01)
#define EMA_CMDCNT                              PPE_REG_ADDR(0x0A02)
#define EMA_DATACNT                             PPE_REG_ADDR(0x0A03)
#define EMA_ISR                                 PPE_REG_ADDR(0x0A04)
#define EMA_IER                                 PPE_REG_ADDR(0x0A05)
#define EMA_CFG                                 PPE_REG_ADDR(0x0A06)
#define EMA_SUBID                               PPE_REG_ADDR(0x0A07)

/*
  *  SLL Registers
  */
#define SLL_CMD1                                PPE_REG_ADDR(0x900)
#define SLL_CMD0                                PPE_REG_ADDR(0x901)
#define SLL_KEY(x)                              PPE_REG_ADDR(0x910+x)
#define SLL_RESULT                              PPE_REG_ADDR(0x920)

/*
 *  PP32 Debug Control Register
 */
#define PP32_SRST                               PPE_REG_ADDR(0x0020)

#define PP32_DBG_CTRL                           PP32_DEBUG_REG_ADDR(0x0000)

#define DBG_CTRL_RESTART                        0
#define DBG_CTRL_STOP                           1

/*
 *  PP32 Registers
 */

// Amazon-S

#define PP32_CTRL_CMD                           PP32_DEBUG_REG_ADDR(0x0B00)
  #define PP32_CTRL_CMD_RESTART                 (1 << 0)
  #define PP32_CTRL_CMD_STOP                    (1 << 1)
  #define PP32_CTRL_CMD_STEP                    (1 << 2)
  #define PP32_CTRL_CMD_BREAKOUT                (1 << 3)

#define PP32_CTRL_OPT                           PP32_DEBUG_REG_ADDR(0x0C00)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP_ON     (3 << 0)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP_OFF    (2 << 0)
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN_ON  (3 << 2)
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN_OFF (2 << 2)
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN_ON      (3 << 4)
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN_OFF     (2 << 4)
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON   (3 << 6)
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF  (2 << 6)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP        (*PP32_CTRL_OPT & (1 << 0))
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN     (*PP32_CTRL_OPT & (1 << 2))
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN         (*PP32_CTRL_OPT & (1 << 4))
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT      (*PP32_CTRL_OPT & (1 << 6))

#define PP32_BRK_PC(i)                          PP32_DEBUG_REG_ADDR(0x0900 + (i) * 2)
#define PP32_BRK_PC_MASK(i)                     PP32_DEBUG_REG_ADDR(0x0901 + (i) * 2)
#define PP32_BRK_DATA_ADDR(i)                   PP32_DEBUG_REG_ADDR(0x0904 + (i) * 2)
#define PP32_BRK_DATA_ADDR_MASK(i)              PP32_DEBUG_REG_ADDR(0x0905 + (i) * 2)
#define PP32_BRK_DATA_VALUE_RD(i)               PP32_DEBUG_REG_ADDR(0x0908 + (i) * 2)
#define PP32_BRK_DATA_VALUE_RD_MASK(i)          PP32_DEBUG_REG_ADDR(0x0909 + (i) * 2)
#define PP32_BRK_DATA_VALUE_WR(i)               PP32_DEBUG_REG_ADDR(0x090C + (i) * 2)
#define PP32_BRK_DATA_VALUE_WR_MASK(i)          PP32_DEBUG_REG_ADDR(0x090D + (i) * 2)
  #define PP32_BRK_CONTEXT_MASK(i)              (1 << (i))
  #define PP32_BRK_CONTEXT_MASK_EN              (1 << 4)
  #define PP32_BRK_COMPARE_GREATER_EQUAL        (1 << 5)    //  valid for break data value rd/wr only
  #define PP32_BRK_COMPARE_LOWER_EQUAL          (1 << 6)
  #define PP32_BRK_COMPARE_EN                   (1 << 7)

#define PP32_BRK_TRIG                           PP32_DEBUG_REG_ADDR(0x0F00)
  #define PP32_BRK_GRPi_PCn_ON(i, n)            ((3 << ((n) * 2)) << ((i) * 16))
  #define PP32_BRK_GRPi_PCn_OFF(i, n)           ((2 << ((n) * 2)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_ADDRn_ON(i, n)     ((3 << ((n) * 2 + 4)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_ADDRn_OFF(i, n)    ((2 << ((n) * 2 + 4)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn_ON(i, n) ((3 << ((n) * 2 + 8)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn_OFF(i, n)((2 << ((n) * 2 + 8)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn_ON(i, n) ((3 << ((n) * 2 + 12)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn_OFF(i, n)((2 << ((n) * 2 + 12)) << ((i) * 16))
  #define PP32_BRK_GRPi_PCn(i, n)               (*PP32_BRK_TRIG & ((1 << ((n))) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_ADDRn(i, n)        (*PP32_BRK_TRIG & ((1 << ((n) + 2)) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn(i, n)    (*PP32_BRK_TRIG & ((1 << ((n) + 4)) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn(i, n)    (*PP32_BRK_TRIG & ((1 << ((n) + 6)) << ((i) * 8)))

#define PP32_CPU_STATUS                         PP32_DEBUG_REG_ADDR(0x0D00)
#define PP32_HALT_STAT                          PP32_CPU_STATUS
#define PP32_DBG_CUR_PC                         PP32_CPU_STATUS
  #define PP32_CPU_USER_STOPPED                 (*PP32_CPU_STATUS & (1 << 0))
  #define PP32_CPU_USER_BREAKIN_RCV             (*PP32_CPU_STATUS & (1 << 1))
  #define PP32_CPU_USER_BREAKPOINT_MET          (*PP32_CPU_STATUS & (1 << 2))
  #define PP32_CPU_CUR_PC                       (*PP32_CPU_STATUS >> 16)

#define PP32_BREAKPOINT_REASONS                 PP32_DEBUG_REG_ADDR(0x0A00)
  #define PP32_BRK_PC_MET(i)                    (*PP32_BREAKPOINT_REASONS & (1 << (i)))
  #define PP32_BRK_DATA_ADDR_MET(i)             (*PP32_BREAKPOINT_REASONS & (1 << ((i) + 2)))
  #define PP32_BRK_DATA_VALUE_RD_MET(i)         (*PP32_BREAKPOINT_REASONS & (1 << ((i) + 4)))
  #define PP32_BRK_DATA_VALUE_WR_MET(i)         (*PP32_BREAKPOINT_REASONS & (1 << ((i) + 6)))
  #define PP32_BRK_DATA_VALUE_RD_LO_EQ(i)       (*PP32_BREAKPOINT_REASONS & (1 << ((i) * 2 + 8)))
  #define PP32_BRK_DATA_VALUE_RD_GT_EQ(i)       (*PP32_BREAKPOINT_REASONS & (1 << ((i) * 2 + 9)))
  #define PP32_BRK_DATA_VALUE_WR_LO_EQ(i)       (*PP32_BREAKPOINT_REASONS & (1 << ((i) * 2 + 12)))
  #define PP32_BRK_DATA_VALUE_WR_GT_EQ(i)       (*PP32_BREAKPOINT_REASONS & (1 << ((i) * 2 + 13)))
  #define PP32_BRK_CUR_CONTEXT                  ((*PP32_BREAKPOINT_REASONS >> 16) & 0x03)

#define PP32_GP_REG_BASE                        PP32_DEBUG_REG_ADDR(0x0E00)
#define PP32_GP_CONTEXTi_REGn(i, n)             PP32_DEBUG_REG_ADDR(0x0E00 + (i) * 16 + (n))

/*
 *    Code/Data Memory (CDM) Interface Configuration Register
 */
#define CDM_CFG                                 PPE_REG_ADDR(0x0100)

#define CDM_CFG_RAM1_SET(value)                 SET_BITS(0, 3, 2, value)
#define CDM_CFG_RAM0_SET(value)                 ((value) ? (1 << 1) : 0)

/*
 *  Mailbox IGU0 Registers
 */
#define MBOX_IGU0_ISRS                          PPE_REG_ADDR(0x0200)
#define MBOX_IGU0_ISRC                          PPE_REG_ADDR(0x0201)
#define MBOX_IGU0_ISR                           PPE_REG_ADDR(0x0202)
#define MBOX_IGU0_IER                           PPE_REG_ADDR(0x0203)

/*
 *  Mailbox IGU1 Registers
 */
#define MBOX_IGU1_ISRS                          PPE_REG_ADDR(0x0204)
#define MBOX_IGU1_ISRC                          PPE_REG_ADDR(0x0205)
#define MBOX_IGU1_ISR                           PPE_REG_ADDR(0x0206)
#define MBOX_IGU1_IER                           PPE_REG_ADDR(0x0207)

/*
 *  Reset Registers
 */
#define AMAZON_S_RCU_BASE_ADDR                  (KSEG1 | 0x1F203000)
#define AMAZON_S_RCU_RST_REQ                    ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x0010))
#define AMAZON_S_RCU_RST_STAT                   ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x0014))
#define AMAZON_S_USB_CFG                        ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x0018))
#define AMAZON_S_RCU_PPE_CONF                   ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x002C))

/*
 *  Power Management Unit Registers
 */
//#define AMAZON_S_PMU                            (KSEG1 + 0x1F102000)
//#define PMU_PWDCR                               ((volatile u32 *)(AMAZON_S_PMU + 0x001C))
//#define PMU_SR                                  ((volatile u32 *)(AMAZON_S_PMU + 0x0020))

/*
 *  GPIO Registers
 */
//#define AMAZON_S_GPIO                           (KSEG1 + 0x1E100B00)
//#define GPIO_Pi_OUT(i)                          ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0010))
//#define GPIO_Pi_IN(i)                           ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0014))
//#define GPIO_Pi_DIR(i)                          ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0018))
//#define GPIO_Pi_ALTSEL0(i)                      ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x001C))
//#define GPIO_Pi_ALTSEL1(i)                      ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0020))
//#define GPIO_Pi_OD(i)                           ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0024))
//#define GPIO_Pi_STOFF(i)                        ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0028))
//#define GPIO_Pi_PUDSEL(i)                       ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x002C))
//#define GPIO_Pi_PUDEN(i)                        ((volatile u32 *)(AMAZON_S_GPIO + (i) * 0x0030 + 0x0030))

/*
 *  DMA Registers
 */
#define AMAZON_S_DMA                            (KSEG1 | 0x1E104100)
#define AMAZON_S_DMA_BASE                       AMAZON_S_DMA
#define AMAZON_S_DMA_CLC                        (volatile u32*)(AMAZON_S_DMA_BASE + 0x00)
#define AMAZON_S_DMA_ID                         (volatile u32*)(AMAZON_S_DMA_BASE + 0x08)
#define AMAZON_S_DMA_CTRL                       (volatile u32*)(AMAZON_S_DMA_BASE + 0x10)
#define AMAZON_S_DMA_CPOLL                      (volatile u32*)(AMAZON_S_DMA_BASE + 0x14)
#define AMAZON_S_DMA_CS(i)                      (volatile u32*)(AMAZON_S_DMA_BASE + 0x18 + 0x38 * (i))
#define AMAZON_S_DMA_CCTRL(i)                   (volatile u32*)(AMAZON_S_DMA_BASE + 0x1C + 0x38 * (i))
#define AMAZON_S_DMA_CDBA(i)                    (volatile u32*)(AMAZON_S_DMA_BASE + 0x20 + 0x38 * (i))
#define AMAZON_S_DMA_CDLEN(i)                   (volatile u32*)(AMAZON_S_DMA_BASE + 0x24 + 0x38 * (i))
#define AMAZON_S_DMA_CIS(i)                     (volatile u32*)(AMAZON_S_DMA_BASE + 0x28 + 0x38 * (i))
#define AMAZON_S_DMA_CIE(i)                     (volatile u32*)(AMAZON_S_DMA_BASE + 0x2C + 0x38 * (i))
#define AMAZON_S_CGBL                           (volatile u32*)(AMAZON_S_DMA_BASE + 0x30)
#define AMAZON_S_DMA_PS(i)                      (volatile u32*)(AMAZON_S_DMA_BASE + 0x40 + 0x30 * (i))
#define AMAZON_S_DMA_PCTRL(i)                   (volatile u32*)(AMAZON_S_DMA_BASE + 0x44 + 0x30 * (i))
#define AMAZON_S_DMA_IRNEN                      (volatile u32*)(AMAZON_S_DMA_BASE + 0xf4)
#define AMAZON_S_DMA_IRNCR                      (volatile u32*)(AMAZON_S_DMA_BASE + 0xf8)
#define AMAZON_S_DMA_IRNICR                     (volatile u32*)(AMAZON_S_DMA_BASE + 0xfc)

/*
 *  External Interrupt Registers
 */
#define AMAZON_S_EIU_BASE                       (KSEG1 | 0x1F101000)
#define AMAZON_S_EIU_EXIN_C                     (volatile u32*)(AMAZON_S_EIU_BASE + 0x00)
#define AMAZON_S_EIU_INIC                       (volatile u32*)(AMAZON_S_EIU_BASE + 0x04)
#define AMAZON_S_EIU_INC                        (volatile u32*)(AMAZON_S_EIU_BASE + 0x08)
#define AMAZON_S_EIU_INEN                       (volatile u32*)(AMAZON_S_EIU_BASE + 0x0C)
#define AMAZON_S_EIU_YIELDEN0                   (volatile u32*)(AMAZON_S_EIU_BASE + 0x10)
#define AMAZON_S_EIU_YIELDEN1                   (volatile u32*)(AMAZON_S_EIU_BASE + 0x14)
#define AMAZON_S_EIU_YIELDEN2                   (volatile u32*)(AMAZON_S_EIU_BASE + 0x18)
#define AMAZON_S_EIU_NMI_CR                     (volatile u32*)(AMAZON_S_EIU_BASE + 0xF0)
#define AMAZON_S_EIU_NMI_SR                     (volatile u32*)(AMAZON_S_EIU_BASE + 0xF4)

/*
 *  macro for eth framework
 */
#ifndef CONFIG_IFX_ETH_FRAMEWORK
  #define ifx_eth_fw_alloc_netdev(size, ifname, dummy)      alloc_netdev(size, ifname, ether_setup)
  #define ifx_eth_fw_free_netdev(netdev, dummy)             free_netdev(netdev)
  #define ifx_eth_fw_register_netdev(netdev)                register_netdev(netdev)
  #define ifx_eth_fw_unregister_netdev(netdev, dummy)       unregister_netdev(netdev)
  #define ifx_eth_fw_netdev_priv(netdev)                    netdev_priv(netdev)
#endif



/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *  Switch Header, Flag Header & Descriptor
 */
#if defined(__BIG_ENDIAN)
  struct sw_eg_pkt_header {
    unsigned int    spid                :8; //  bit 26-24
    unsigned int    dpid                :8; //  bit 17-16
    unsigned int    qid                 :8; //  bit 9-8
    unsigned int    direct              :8; //  bit 0
  };

  struct flag_header {
    //  0 - 3h
    unsigned int    ipv4_mc_pkt         :1; // IPv4 multicast packet?
    unsigned int    res0                :1;
    unsigned int    proc_type           :1; // 0: routing, 1: bridging
    unsigned int    res1                :1;
    unsigned int    tcpudp_err          :1; //  reserved in A4
    unsigned int    tcpudp_chk          :1; //  reserved in A4
    unsigned int    is_udp              :1;
    unsigned int    is_tcp              :1;
    unsigned int    res2                :1;
    unsigned int    ip_inner_offset     :7; //offset from the start of the Ethernet frame to the IP field(if there's more than one IP/IPv6 header, it's inner one)
    unsigned int    is_pppoes           :1; //  2h
    unsigned int    is_inner_ipv6       :1;
    unsigned int    is_inner_ipv4       :1;
    unsigned int    is_vlan             :2; //  0: nil, 1: single tag, 2: double tag, 3: reserved
    unsigned int    rout_index          :11;

    //  4 - 7h
    unsigned int    dest_list           :8;
    unsigned int    src_itf             :3; //  7h
    unsigned int    tcp_rstfin          :1; //  7h
    unsigned int    qid                 :4; //  7h
    unsigned int    temp_dest_list      :8; //  only for firmware use
    unsigned int    src_dir             :1; //  0: LAN, 1: WAN
    unsigned int    acc_done            :1;
    unsigned int    res3                :2;
    unsigned int    is_outer_ipv6       :1; //if normal ipv6 packet, only is_inner_ipv6 is set
    unsigned int    is_outer_ipv4       :1;
    unsigned int    is_tunnel           :2; //0-1 reserved, 2: 6RD, 3: Ds-lite

    // 8 - 11h
	unsigned int    sppid               :3; //not used in ar9;
    unsigned int    pkt_len             :13;//packet length
    unsigned int    pl_byteoff          :8; //bytes between flag header and fram payload
    unsigned int    mpoa_type           :2;
    unsigned int    ip_outer_offset     :6; //offset from the start of the Ethernet frame to the IP field

    // 12 - 15h
    unsigned int    tc                  :4; //switch traffic class
    unsigned int    res4                :28;
  };

  struct wlan_flag_header {
    unsigned int    res1                :1;
    unsigned int    aal5_raw            :1; //  0: ETH frame with FCS, 1: AAL5 CPCS-SDU
    unsigned int    mpoa_type           :2; //  0: EoA without FCS, 1: EoA with FCS, 2: PPPoA, 3:IPoA
    unsigned int    res2                :1;
    unsigned int    dsl_qid             :3; //  not valid for WLAN
    unsigned int    res3                :5;
    unsigned int    src_itf             :3;
    unsigned int    payload_overlap     :16;//  This flag header is 16 bits only. This field overlapps with payload, and is for 32-bit align purpose.
  };

  struct rx_descriptor {
    //  0 - 3h
    unsigned int    own                 :1; //  0: PPE, 1: MIPS, this value set is for DSL PKT RX and DSL OAM RX, it's special case.
    unsigned int    c                   :1;
    unsigned int    sop                 :1;
    unsigned int    eop                 :1;
    unsigned int    res1                :3;
    unsigned int    byteoff             :2;
    unsigned int    res2                :2;
    unsigned int    qid                 :4;
    unsigned int    err                 :1; //  0: no error, 1: error (RA, IL, CRC, USZ, OVZ, MFL, CPI, CPCS-UU)
    unsigned int    datalen             :16;
    //  4 - 7h
    unsigned int    res3                :3;
    unsigned int    dataptr             :29;
  };

  struct tx_descriptor {
    //  0 - 3h
    unsigned int    own                 :1; //  0: MIPS, 1: PPE, for CPU to WAN TX, it's inverse, 0: PPE, 1: MIPS
    unsigned int    c                   :1;
    unsigned int    sop                 :1;
    unsigned int    eop                 :1;
    unsigned int    byteoff             :5;
    unsigned int    mpoa_pt             :1; //  not for WLAN TX, 0: MPoA is dterminated in FW, 1: MPoA is transparent to FW.
    unsigned int    mpoa_type           :2; //  not for WLAN TX, 0: EoA without FCS, 1: reserved, 2: PPPoA, 3: IPoA
    //unsigned int    pdu_type            :1; //  not for WLAN TX, 0: AAL5, 1: Non-AAL5 cell
    unsigned int    qid                 :4; //  not for WLAN TX,
    unsigned int    datalen             :16;
    //  4 - 7h
    unsigned int    res1                :4;
    unsigned int    dataptr             :28;
  };

  struct eth_tx_descriptor {
    //  0 - 3h
    unsigned int    own                 :1; //  0: MIPS, 1: PPE
    unsigned int    c                   :1;
    unsigned int    sop                 :1;
    unsigned int    eop                 :1;
    unsigned int    byteoff             :5;
    unsigned int    qid                 :4;
    unsigned int    res1                :2;
	unsigned int    small				:1;
    unsigned int    datalen             :16;
    //  4 - 7h
    unsigned int    dataptr             :32;
  };
#else
#endif

/*
 *  Internal Structure of Devices (ETH/ATM)
 */
struct mii_port {
    unsigned int  port_num;
    unsigned int  phy_id;
    unsigned char phy_addr;
    unsigned char irq_line;
    unsigned char mode;
};

struct eth_priv_data {
    struct  net_device_stats        stats;

    unsigned int                    rx_packets;
    unsigned int                    rx_bytes;
    unsigned int                    rx_errors;
    unsigned int                    rx_dropped;
    unsigned int                    tx_packets;
    unsigned int                    tx_bytes;
    unsigned int                    tx_errors;
    unsigned int                    tx_dropped;

/*Please add new member below, the above is only for statistic, and may be cleared on the run time.  */
    unsigned int                    dev_id;
    struct  mii_if_info             mii;
    struct  mii_port               *mii_port;
};

typedef struct fw_dbg {
    char *cmd;
    void (*pfunc)(char **tokens, int token_num);
}fw_dbg_t;

/*
 * ####################################
 *             Declaration
 * ####################################
 */
/*
 *  Network Operations
 */
static void eth_setup(struct net_device *);
static struct net_device_stats *eth_get_stats(struct net_device *);
static int eth_open(struct net_device *);
static int eth_stop(struct net_device *);
#if defined(ENABLE_NAPI) && ENABLE_NAPI
  static ifx_eth_fw_poll_ret_t eth_poll(struct net_device *, int, int *);
#endif
static int eth_hard_start_xmit(struct sk_buff *, struct net_device *);
static int eth_qos_hard_start_xmit(struct sk_buff *, struct net_device *);
static int eth_set_mac_address(struct net_device *, void *);
static int eth_ioctl(struct net_device *, struct ifreq *, int);
static void eth_tx_timeout(struct net_device *);

/*
 *  Network operations help functions
 */
static INLINE int get_eth_port(struct net_device *);
static INLINE int eth_xmit(struct sk_buff *, unsigned int, int, int, int);

/*
 *  ioctl help functions
 */
static int ethtool_ioctl(struct net_device *, struct ifreq *, int);
static void xway_get_drvinfo(struct net_device *, struct ethtool_drvinfo *);
static int xway_get_settings(struct net_device *, struct ethtool_cmd *);
static int xway_set_settings(struct net_device *, struct ethtool_cmd *);
static void xway_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause);
static int xway_set_pauseparam(struct net_device *netdev,
            struct ethtool_pauseparam *epause);
static int xway_nway_reset(struct net_device *);
static u32 xway_get_link(struct net_device *);

/*
 *  Buffer manage functions
 */
static INLINE struct sk_buff *alloc_skb_rx(void);
static struct sk_buff *alloc_skb_tx_aligned(struct sk_buff *, int);
static INLINE struct sk_buff *alloc_skb_tx(int);
static struct sk_buff* skb_break_away_from_protocol(struct sk_buff *);
static INLINE void __get_skb_from_dbg_pool(struct sk_buff *, const char *, unsigned int);
#define get_skb_from_dbg_pool(skb)  __get_skb_from_dbg_pool(skb, __FUNCTION__, __LINE__)
static INLINE struct sk_buff *__get_skb_pointer(unsigned int, const char *, unsigned int);
#define get_skb_pointer(dataptr)    __get_skb_pointer(dataptr, __FUNCTION__, __LINE__)
static INLINE void __put_skb_to_dbg_pool(struct sk_buff *, const char *, unsigned int);
#define put_skb_to_dbg_pool(skb)    __put_skb_to_dbg_pool(skb, __FUNCTION__, __LINE__)

/*
 *  Mailbox handler
 */
static irqreturn_t mailbox0_irq_handler(int, void *);
static irqreturn_t mailbox_irq_handler(int, void *);

/*
 *  Turn On/Off Dma
 */
static INLINE void turn_on_dma_rx(int);
static INLINE void turn_off_dma_rx(int);

/*
 *  DMA interface functions
 */
static u8 *dma_buffer_alloc(int, int *, void **);
static int dma_buffer_free(u8 *, void *);
#if defined(ENABLE_NAPI) && ENABLE_NAPI
  static void dma_activate_poll(struct dma_device_info *);
  static void dma_inactivate_poll(struct dma_device_info *);
#endif
static int dma_int_handler(struct dma_device_info *, int);
static INLINE int dma_rx_int_handler(struct dma_device_info *);

/*
 *  MII/Switch help functions
 */
static int mdio_read(struct net_device *, int, int);
static void mdio_write(struct net_device *, int, int, int);
static INLINE u16 smi_reg_read(u16 reg);
static INLINE void smi_reg_write(u16 reg, u16 data);
static INLINE void init_mdio(void);
static void probe_mii_ports(void);
static INLINE void init_external_tantos(void);

/*
 *  GPHY Function (MII1)
 */
static INLINE void init_gphy_v14(unsigned int);
static INLINE void mii1_gphy_config_gpio(void);
#ifdef CONFIG_MII1_AUTONEG
  static INLINE void mii1_gphy_config_ext_int(void);
  static irqreturn_t mii1_gphy_11G_int_handler(int, void *);
  static irqreturn_t mii1_gphy_VSC_int_handler(int, void *);
#endif

/*
 *  Hardware init functions
 */
static INLINE void reset_ppe(void);
static INLINE void reset_gphy(void);
static INLINE void init_pmu(void);
static INLINE void init_mii0(int);
static INLINE void init_mii1(int);
static INLINE void init_mii2(void);
static INLINE void start_mii2(void);
static INLINE void stop_datapath(void);
static INLINE void init_internal_tantos_qos_setting(void);
static INLINE void init_internal_tantos(void);
static INLINE void init_dplus(void);
static INLINE void init_ema(void);
static INLINE void init_mailbox(void);
static INLINE void clear_share_buffer(void);
static INLINE void clear_cdm(void);
static INLINE void board_init(void);
static INLINE void setup_ppe_conf(void);
static INLINE void restore_ppe_conf(void);
static INLINE void init_hw(void);

/*
 *  PP32 specific functions
 */
static INLINE int pp32_download_code(const u32 *, unsigned int, const u32 *, unsigned int);
static INLINE int pp32_specific_init(int, void *);
static INLINE int pp32_start(int);
static INLINE void pp32_stop(void);

/*
 *  Init & clean-up functions
 */
static INLINE int init_local_variables(void);
static INLINE void clear_local_variables(void);
static INLINE void init_communication_data_structures(int);
static INLINE int alloc_dma(void);
static INLINE void free_dma(void);
#ifdef CONFIG_IFX_PPA_DATAPATH
  static INLINE void ethaddr_setup(unsigned int, char *);
#endif

/*
 *  local implement memcpy
 */
#if defined(ENABLE_MY_MEMCPY) && ENABLE_MY_MEMCPY
  static INLINE void *my_memcpy(unsigned char *, const unsigned char *, unsigned int);
#else
  #define my_memcpy             memcpy
#endif

/*
 *  Print Firmware Version ID
 */
static int print_fw_ver(char *, int);
static int print_driver_ver(char *, int, char *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

/*
 *  Proc File
 */
static INLINE void proc_file_create(void);
static INLINE void proc_file_delete(void);
static int proc_read_ver(char *, char **, off_t, int, int *, void *);
static int proc_read_mib(char *, char **, off_t, int, int *, void *);
static int proc_write_mib(struct file *, const char *, unsigned long, void *);
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
  static int proc_read_route(char *, char **, off_t, int, int *, void *);
  static int proc_write_route(struct file *, const char *, unsigned long, void *);
  static int proc_read_mc(char *, char **, off_t, int, int *, void *);
  static int proc_read_genconf(char *, char **, off_t, int, int *, void *);
  static int proc_write_genconf(struct file *, const char *, unsigned long, void *);
  static int proc_read_pppoe(char *, char **, off_t, int, int *, void *);
  static int proc_read_mtu(char *, char **, off_t, int, int *, void *);
  static int proc_read_hit(char *, char **, off_t, int, int *, void *);
  static int proc_write_hit(struct file *, const char *, unsigned long, void *);
  static int proc_read_mac(char *, char **, off_t, int, int *, void *);
  static int proc_read_out_vlan(char *, char **, off_t, int, int *, void *);
  static int proc_read_ipv6_ip(char *, char **, off_t, int, int *, void *);
#endif
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static int proc_read_dbg(char *, char **, off_t, int, int *, void *);
  static int proc_write_dbg(struct file *, const char *, unsigned long, void *);
#endif
#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
  static int proc_write_mem(struct file *, const char *, unsigned long, void *);
#endif
#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
  static int proc_read_pp32(char *, char **, off_t, int, int *, void *);
  static int proc_write_pp32(struct file *, const char *, unsigned long, void *);
#endif
static int proc_read_burstlen(char *, char **, off_t, int, int *, void *);
static int proc_write_burstlen(struct file *, const char *, unsigned long, void *);
#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
  static int proc_read_fw(char *, char **, off_t, int, int *, void *);
  static int proc_write_fw(struct file *, const char *, unsigned long, void *);
  static int proc_write_fwdbg(struct file *, const char *, unsigned long, void *);
  static void fw_dbg_start(char *cmdbuf);
  static void fwdbg_help(char **tokens, int token_num);
  static void dump_ipv4_hdr(struct iphdr * iph);
  static void dump_ipv6_hdr(struct ipv6hdr *iph6);
#endif
static int physical_port_counters_proc_read(char *, char **, off_t, int, int *, void *);
static int physical_port_counters_proc_write(struct file *, const char *, unsigned long, void *);
static int port0_counters_proc_read(char *, char **, off_t, int, int *, void *);
static int port0_counters_proc_write(struct file *, const char *, unsigned long, void *);
static int port1_counters_proc_read(char *, char **, off_t, int, int *, void *);
static int port1_counters_proc_write(struct file *, const char *, unsigned long, void *);
static int port2_counters_proc_read(char *, char **, off_t, int, int *, void *);
static int port2_counters_proc_write(struct file *, const char *, unsigned long, void *);
static int proc_read_mactable(char *, char **, off_t, int, int *, void *);
static int proc_write_mactable(struct file *, const char *, unsigned long, void *);
static int proc_read_directforwarding(char *, char **, off_t, int, int *, void *);
static int proc_write_directforwarding(struct file *, const char *, unsigned long, void *);
static int proc_read_clk(char *, char **, off_t, int, int *, void *);
static int proc_write_clk(struct file *, const char *, unsigned long, void *);
static int proc_read_flowcontrol(char *, char **, off_t, int, int *, void *);
static int proc_write_flowcontrol(struct file *, const char *, unsigned long, void *);
static int proc_read_gphy(char *, char **, off_t, int, int *, void *);
static int proc_write_gphy(struct file *, const char *, unsigned long, void *);
#if defined(ENABLE_NAPI) && ENABLE_NAPI
  static int proc_read_napi(char *, char **, off_t, int, int *, void *);
  static int proc_write_napi(struct file *, const char *, unsigned long, void *);
#endif
static int proc_read_prio(char *, char **, off_t, int, int *, void *);
static int proc_write_prio(struct file *, const char *, unsigned long, void *);
#ifdef CONFIG_IFX_PPA_QOS
  static int proc_read_qos(char *, char **, off_t, int, int *, void *);
#endif
static int proc_read_class(char *, char **, off_t, int, int *, void *);
static int proc_write_class(struct file *, const char *, unsigned long, void *);
static int proc_read_mccfg(char *, char **, off_t, int, int *, void *);
static int proc_write_mccfg(struct file *, const char *, unsigned long, void *);

/*
 *  Proc File help functions
 */
static INLINE int proc_buf_copy(char **, int, off_t, int *, const char *, int);
static INLINE int stricmp(const char *, const char *);
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
  static INLINE int strincmp(const char *, const char *, int);
#endif
static INLINE int get_token(char **, char **, int *, int *);
static INLINE unsigned int get_number(char **, int *, int);
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
  static INLINE void get_ip_port(char **, int *, unsigned int *);
  static INLINE void get_mac(char **, int *, unsigned int *);
#endif
static INLINE void ignore_space(char **, int *);
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
  static INLINE char *get_wanitf(int);
  static INLINE int print_route(char *, int, int, struct rout_forward_compare_tbl *, struct rout_forward_action_tbl *);

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
static INLINE int print_mc(char *, int i, struct wan_rout_multicast_cmp_tbl *, struct wan_rout_multicast_act_tbl *, struct rtp_sampling_cnt *,uint32_t rt_mc_cnt);
#else
static INLINE int print_mc(char *, int i, struct wan_rout_multicast_cmp_tbl *, struct wan_rout_multicast_act_tbl *, uint32_t);
#endif

#endif

/*
 *  Debug functions
 */
#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  static INLINE void dump_skb(struct sk_buff *, u32, char *, int, int, int, int);
#else
  #define dump_skb(a, b, c, d, e, f, g)
#endif
#if defined(DEBUG_DUMP_FLAG_HEADER) && DEBUG_DUMP_FLAG_HEADER
  static INLINE void dump_flag_header(int, struct flag_header *, const char *, int, int);
#else
  #define dump_flag_header(a, b, c, d, e)
#endif
#if defined(DEBUG_DUMP_INIT) && DEBUG_DUMP_INIT
  static INLINE void dump_init(void);
#else
  #define dump_init()
#endif

#ifdef CONFIG_IFX_PORT_MIRROR
  extern struct net_device * (*ifx_get_mirror_netdev)(void);
  extern uint32_t (*ifx_is_device_type_wireless)(void);
  static void mirror_pkt_eth(struct sk_buff *, int);
#endif

/*
 *  Directpath Help Functions
 */
#if defined(ENABLE_DIRECTPATH_TX_QUEUE) && ENABLE_DIRECTPATH_TX_QUEUE
  static int send_directpath_tx_queue(void);
#endif

/*
 *  External Functions
 */

/*
 *  External Variables
 */



/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static int g_fwcode = FWCODE_ROUTING_ACC_D5;

static struct semaphore g_sem;  //  lock used by open/close function

static int g_stop_datapath = 0;

static int g_eth_wan_mode = 3;
static unsigned int g_wan_itf = 1 << 1;

static int g_ipv6_acc_en = 1;

static int g_wanqos_en = 0;

static int g_cpu_to_wan_tx_desc_pos = 0;
static DEFINE_SPINLOCK(g_cpu_to_wan_tx_spinlock);
static int g_cpu_to_wan_swap_desc_pos = 0;

static u32 g_mailbox_signal_mask = CPU_TO_WAN_SWAP_SIG;

static int                      g_f_dma_opened = 0;
static struct dma_device_info  *g_dma_device = NULL;

static struct net_device *g_eth_net_dev[2] = {0};
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
static struct net_device *g_ethoam_net_dev = NULL;
#endif

static DEFINE_SPINLOCK(g_eth_tx_spinlock);

#if !defined(CONFIG_IFX_ETH_FRAMEWORK) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static struct net_device_ops g_eth_netdev_ops[2] = {
    {
    .ndo_open = eth_open,
    .ndo_stop = eth_stop,
    .ndo_set_mac_address = eth_set_mac_address,
    .ndo_do_ioctl = eth_ioctl,
    .ndo_tx_timeout =  eth_tx_timeout,
    .ndo_get_stats = eth_get_stats,
    },
    {
    .ndo_open = eth_open,
    .ndo_stop = eth_stop,
    .ndo_set_mac_address = eth_set_mac_address,
    .ndo_do_ioctl = eth_ioctl,
    .ndo_tx_timeout =  eth_tx_timeout,
    .ndo_get_stats = eth_get_stats,
    },
};
#endif
static const struct ethtool_ops g_xway_ethtool_ops = {
    .get_drvinfo    = xway_get_drvinfo,
    .get_settings   = xway_get_settings,
    .set_settings   = xway_set_settings,
    .get_pauseparam = xway_get_pauseparam,
    .set_pauseparam = xway_set_pauseparam,
    .nway_reset     = xway_nway_reset,
    .get_link       = xway_get_link,
};

static int g_eth_prio_queue_map[2][8];
static int g_wan_queue_class_map[8];    //  So far, this table is shared by all interfaces.

static u8 g_my_ethaddr[MAX_ADDR_LEN * 2] = {0};

static struct proc_entry_cfg *g_eth_proc_entry = NULL;

static int g_dbg_enable = 0;

static unsigned int g_mib_itf = 0x03;

//  1:switch forward packets between port0/1 and cpu port directly w/o processing
//  0:pkts go through normal path and are processed by switch central function
static int g_ingress_direct_forward_enable = 0;
static int g_egress_direct_forward_enable = 1;

#ifdef CONFIG_MII1_AUTONEG
  static int g_link_speed = 1000;   //  link speed:     10/100/1000 Mbps
#endif
static struct mii_port g_mii_port[2] = {
    {.port_num = 0, .phy_id = 0, .mode = 0xFF},
    {.port_num = 1, .phy_id = 0, .mode = 0xFF},
};
static spinlock_t g_mdio_lock;

#if defined(ENABLE_NAPI) && ENABLE_NAPI
  static int g_napi_enable = 1;
#endif

static int g_directpath_dma_full = 0;
static DEFINE_SPINLOCK(g_directpath_tx_spinlock);

/*
 *  variable for test
 */
#if defined(DEBUG_SKB_SWAP) && DEBUG_SKB_SWAP
  static struct sk_buff *g_dbg_skb_swap_pool[1024] = {0};
#endif

static int g_dump_cnt = -1;


/*
 * ####################################
 *           Global Variable
 * ####################################
 */

/*
 *  variable for directpath
 */
#define     MAX_DIRECTPATH_NUM      5
struct ppe_directpath_data g_ppe_directpath_data[MAX_DIRECTPATH_NUM];



/*
 * ####################################
 *            Local Function
 * ####################################
 */

static void eth_setup(struct net_device *dev)
{
    int port;
    u8 *ethaddr;
    u32 val;
    int i;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    for ( port = 0; port < NUM_ENTITY(g_eth_net_dev) && g_eth_net_dev[port] != dev; port++ );
    if ( port == NUM_ENTITY(g_eth_net_dev) )
        err("Can't find available slot for device %s", dev->name);

#ifndef CONFIG_IFX_ETH_FRAMEWORK
    /*  hook network operations */
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dev->get_stats           = eth_get_stats;
    dev->open                = eth_open;
    dev->stop                = eth_stop;
    if ( g_eth_wan_mode == 3 && g_wanqos_en && (g_wan_itf & (1 << port)) )
        dev->hard_start_xmit = eth_qos_hard_start_xmit;
    else
        dev->hard_start_xmit = eth_hard_start_xmit;
    dev->set_mac_address     = eth_set_mac_address;
    dev->do_ioctl            = eth_ioctl;
    dev->tx_timeout          = eth_tx_timeout;
  #else
    if ( g_eth_wan_mode == 3 && g_wanqos_en && (g_wan_itf & (1 << port)) )
        g_eth_netdev_ops[port].ndo_start_xmit = eth_qos_hard_start_xmit;
    else
        g_eth_netdev_ops[port].ndo_start_xmit = eth_hard_start_xmit;
    dev->netdev_ops          = &g_eth_netdev_ops[port];
  #endif
    dev->watchdog_timeo      = ETH_WATCHDOG_TIMEOUT;
#endif
    SET_ETHTOOL_OPS(dev, &g_xway_ethtool_ops);
    priv->dev_id = port;

    ethaddr = MY_ETH0_ADDR + (port ? MAX_ADDR_LEN : 0);

    /*  read MAC address from the MAC table and put them into device    */
    for ( i = 0, val = 0; i < 6; i++ )
        val += dev->dev_addr[i];
    if ( val == 0 )
    {
        for ( i = 0, val = 0; i < 6; i++ )
            val += ethaddr[i];
        if ( val == 0 )
        {
            /*  ethaddr not set in u-boot   */
            dev->dev_addr[0] = 0x00;
            dev->dev_addr[1] = 0x20;
            dev->dev_addr[2] = 0xda;
            dev->dev_addr[3] = 0x86;
            dev->dev_addr[4] = 0x23;
            dev->dev_addr[5] = 0x74 + port;
        }
        else
        {
            for ( i = 0; i < 6; i++ )
                dev->dev_addr[i] = ethaddr[i];
        }
    }

#if defined(ENABLE_IPv6_DEMO_SUPPORT) && ENABLE_IPv6_DEMO_SUPPORT
    {
        u32 mac_dword0, mac_dword1;

        mac_dword0 = (((u32)dev->dev_addr[0] & 0xFF) << 24) | (((u32)dev->dev_addr[1] & 0xFF) << 16) | (((u32)dev->dev_addr[2] & 0xFF) << 8) | ((u32)dev->dev_addr[3] & 0xFF);
        mac_dword1 = (((u32)dev->dev_addr[4] & 0xFF) << 24) | (((u32)dev->dev_addr[5] & 0xFF) << 16);
        ROUT_MAC_CFG_TBL(port)[0] = mac_dword0;
        ROUT_MAC_CFG_TBL(port)[1] = mac_dword1;
    }
#endif
}

static struct net_device_stats *eth_get_stats(struct net_device *dev)
{
    int port;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    port = priv->dev_id;
    if ( port >= NUM_ENTITY(g_eth_net_dev) )
        return NULL;

    priv->stats.rx_packets = priv->rx_packets
                            + ITF_MIB_TBL(port)->ig_fast_brg_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_udp_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_tcp_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_mc_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv6_udp_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv6_tcp_pkts;
    priv->stats.rx_bytes    = priv->rx_bytes
                            + ITF_MIB_TBL(port)->ig_fast_brg_bytes
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_bytes
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv6_bytes;
    priv->stats.rx_errors   = 0;
    priv->stats.rx_dropped  = priv->rx_dropped
                            + ITF_MIB_TBL(port)->ig_drop_pkts;

    priv->stats.tx_packets  = priv->tx_packets + ITF_MIB_TBL(port)->eg_fast_pkts;
    priv->stats.tx_bytes    = priv->tx_bytes;
    priv->stats.tx_errors   = priv->tx_errors;
    priv->stats.tx_dropped  = priv->tx_dropped;

    return &priv->stats;
}

static int eth_open(struct net_device *dev)
{
    int port;

    port = get_eth_port(dev);
    if ( port < 0 )
        return -ENODEV;

    down(&g_sem);

    dbg("%s", dev->name);

    turn_on_dma_rx(port);

#ifndef CONFIG_IFX_ETH_FRAMEWORK
    netif_start_queue(dev);
#endif

    up(&g_sem);
    return 0;
}

static int eth_stop(struct net_device *dev)
{
    int port;

    port = get_eth_port(dev);
    if ( port < 0 )
        return -ENODEV;

    down(&g_sem);

    turn_off_dma_rx(port);

#ifndef CONFIG_IFX_ETH_FRAMEWORK
    netif_stop_queue(dev);
#endif

    up(&g_sem);
    return 0;
}

#define PPE_ETH_DATA_LEN   1572  /*1600 - 14( 12(Flag_header) + 2(space)) - 14  ( MAC header) */
static int eth_change_mtu(struct net_device *dev, int new_mtu)
{
	if (new_mtu < 68 || new_mtu > PPE_ETH_DATA_LEN)
		return -EINVAL;
	dev->mtu = new_mtu;
	return 0;
}

#if defined(ENABLE_NAPI) && ENABLE_NAPI
static ifx_eth_fw_poll_ret_t eth_poll(struct net_device *poll_dev, int work_to_do, int *work_done)
{
    int ret = dma_device_poll(g_dma_device, work_to_do, work_done);

    return ret == 0 ? IFX_ETH_FW_POLL_COMPLETE : IFX_ETH_FW_POLL_CONTINUE;
}
#endif

static int eth_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int port;
    int qid;

    port = get_eth_port(dev);
    if ( port < 0 )
        return -ENODEV;

    if ( skb->priority >= NUM_ENTITY(g_eth_prio_queue_map[port]) )
        qid = g_eth_prio_queue_map[port][NUM_ENTITY(g_eth_prio_queue_map[port]) - 1];
    else
        qid = g_eth_prio_queue_map[port][skb->priority];

    eth_xmit(skb, port, 2, 2, g_wan_queue_class_map[qid]);  //  spid - CPU, class - 2 layer map, 1st: taken from prio_queue_map, 2nd: taken from wan_queue_class_map

    return 0;
}

static int eth_qos_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int port;
    int qid;
    unsigned long sys_flag;
    volatile struct eth_tx_descriptor *desc;
    struct eth_tx_descriptor reg_desc;
    struct sk_buff *skb_to_free;
    int byteoff;
    int len;
    struct sw_eg_pkt_header pkth = {0};
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    if ( g_stop_datapath != 0 )
        goto ETH_XMIT_DROP;

    port = get_eth_port(dev);
    if ( port < 0 )
        return -ENODEV;

    if ( skb->cb[13] == 0x5A )  //  magic number indicating directpath
        qid = skb->cb[15];
    else
    {
        if ( skb->priority >= NUM_ENTITY(g_eth_prio_queue_map[port]) )
            qid = g_eth_prio_queue_map[port][NUM_ENTITY(g_eth_prio_queue_map[port]) - 1];
        else
            qid = g_eth_prio_queue_map[port][skb->priority];
    }

    pkth.spid   = 2;    //  CPU
    pkth.dpid   = port; //  eth0/eth1
    pkth.qid    = g_wan_queue_class_map[qid];       //  0 is lowest priority, 3 is highest priority
    pkth.direct = g_egress_direct_forward_enable;   //  enable/disable direct forward according to flag, need re-consideration

    len = skb->len <= ETH_MIN_TX_PACKET_LENGTH ? ETH_MIN_TX_PACKET_LENGTH : skb->len;

    dump_skb(skb, DUMP_SKB_LEN, "eth_qos_hard_start_xmit", port, qid, 1, 0);

    /*  reserve space to put pointer in skb */
    byteoff = (((unsigned int)skb->data - sizeof(pkth)) & (DMA_ALIGNMENT - 1)) + sizeof(pkth);
    //if ( skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff || skb->end - skb->data < 1564 /* 1518 (max ETH frame) + 22 (reserved for firmware) + 10 (AAL5) + 6 (PPPoE) + 4 x 2 (2 VLAN) */ || skb_shared(skb) || skb_cloned(skb) )
    if ( skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff || skb_shared(skb) || skb_cloned(skb) ||
          (unsigned int)skb->end - (unsigned int)skb->data < DMA_PACKET_SIZE  )
    {
        struct sk_buff *new_skb;

        ASSERT(skb_headroom(skb) >= sizeof(struct sk_buff *) + byteoff, "skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff");
        new_skb = alloc_skb_tx(sizeof(pkth) + skb->len < DMA_PACKET_SIZE ? DMA_PACKET_SIZE : sizeof(pkth) + skb->len);  //  may be used by RX fastpath later
        if ( !new_skb )
        {
            err("no memory");
            goto ALLOC_SKB_TX_FAIL;
        }
        skb_put(new_skb, sizeof(pkth) + skb->len);
        my_memcpy(new_skb->data, &pkth, sizeof(pkth));
        my_memcpy(new_skb->data + sizeof(pkth), skb->data, skb->len);
        dev_kfree_skb_any(skb);
        skb = new_skb;
        byteoff = (u32)skb->data & (DMA_ALIGNMENT - 1);
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((u32)skb->data, skb->len);
#endif
    }
    else
    {
        skb_push(skb, sizeof(pkth));
        byteoff -= sizeof(pkth);
        my_memcpy(skb->data, &pkth, sizeof(pkth));
        *(struct sk_buff **)((u32)skb->data - byteoff - sizeof(struct sk_buff *)) = skb;
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((u32)skb->data - byteoff - sizeof(struct sk_buff *), skb->len + byteoff + sizeof(struct sk_buff *));
#endif
    }

    /*  allocate descriptor */
    spin_lock_irqsave(&g_cpu_to_wan_tx_spinlock, sys_flag);
    desc = (volatile struct eth_tx_descriptor *)(CPU_TO_WAN_TX_DESC_BASE + g_cpu_to_wan_tx_desc_pos);
    if ( desc->own )    //  PPE hold
    {
        spin_unlock_irqrestore(&g_cpu_to_wan_tx_spinlock, sys_flag);
        err("PPE hold");
        goto NO_FREE_DESC;
    }
    if ( ++g_cpu_to_wan_tx_desc_pos == CPU_TO_WAN_TX_DESC_NUM )
        g_cpu_to_wan_tx_desc_pos = 0;
    spin_unlock_irqrestore(&g_cpu_to_wan_tx_spinlock, sys_flag);

    /*  load descriptor from memory */
    reg_desc = *desc;

    /*  free previous skb   */
    ASSERT((reg_desc.dataptr & (DMA_ALIGNMENT - 1)) == 0, "reg_desc.dataptr (0x%#x) must be 8 DWORDS aligned", reg_desc.dataptr);
    skb_to_free = get_skb_pointer(reg_desc.dataptr);
    dev_kfree_skb_any(skb_to_free);

    /*  detach from protocol    */
    skb_to_free = skb;
    skb = skb_break_away_from_protocol(skb);
    dev_kfree_skb_any(skb_to_free);

    put_skb_to_dbg_pool(skb);

#ifdef CONFIG_IFX_PORT_MIRROR
    if ( ifx_get_mirror_netdev && ifx_get_mirror_netdev() != NULL )
        mirror_pkt_eth(skb, sizeof(pkth));
#endif

    /*  update descriptor   */
    reg_desc.small      = (unsigned int)skb->end - (unsigned int)skb->data < DMA_PACKET_SIZE ? 1 : 0;
    reg_desc.dataptr    = (u32)skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1));  //  byte address
    reg_desc.byteoff    = byteoff;
    reg_desc.datalen    = len + sizeof(pkth);
    reg_desc.qid        = qid;
    reg_desc.own        = 1;
    reg_desc.c          = 0;
    reg_desc.sop = reg_desc.eop = 1;

    /*  update MIB  */
    dev->trans_start = jiffies;
    priv->tx_packets++;
    priv->tx_bytes += len;

    /*  write discriptor to memory and write back cache */
    *((volatile u32 *)desc + 1) = *((u32 *)&reg_desc + 1);
    *(volatile u32 *)desc = *(u32 *)&reg_desc;

    return 0;

NO_FREE_DESC:
ALLOC_SKB_TX_FAIL:
ETH_XMIT_DROP:
    dev_kfree_skb_any(skb);
    priv->tx_dropped++;
    return 0;
}

#if defined(ENABLE_MIXED_QOS) && ENABLE_MIXED_QOS
static int eth_mix_qos_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    struct vlan_ethhdr *eth_dst;
    volatile struct eth_ports_cfg *eth_p_cfg;

    eth_dst = (struct vlan_ethhdr *)(skb->data);
    if(eth_dst->h_vlan_proto != htons(ETH_P_8021Q)){
        return eth_hard_start_xmit(skb, dev);
    }

    eth_p_cfg = ETH_PORTS_CFG;
    if(eth_dst->h_vlan_TCI < eth_p_cfg->wan_vlanid_lo
        || eth_dst->h_vlan_TCI > eth_p_cfg->wan_vlanid_hi){
        return eth_hard_start_xmit(skb,dev);
    }

    return eth_qos_hard_start_xmit(skb,dev);
}
#endif

static int eth_set_mac_address(struct net_device *dev, void *p)
{
    struct sockaddr *addr = (struct sockaddr *)p;
#ifdef ROUT_MAC_CFG_TBL
    u32 addr1, addr2;
    int i;
#endif

    dbg("%s: change MAC from %02X:%02X:%02X:%02X:%02X:%02X to %02X:%02X:%02X:%02X:%02X:%02X", dev->name,
        (u32)dev->dev_addr[0] & 0xFF, (u32)dev->dev_addr[1] & 0xFF, (u32)dev->dev_addr[2] & 0xFF, (u32)dev->dev_addr[3] & 0xFF, (u32)dev->dev_addr[4] & 0xFF, (u32)dev->dev_addr[5] & 0xFF,
        (u32)addr->sa_data[0] & 0xFF, (u32)addr->sa_data[1] & 0xFF, (u32)addr->sa_data[2] & 0xFF, (u32)addr->sa_data[3] & 0xFF, (u32)addr->sa_data[4] & 0xFF, (u32)addr->sa_data[5] & 0xFF);

#ifdef ROUT_MAC_CFG_TBL
    addr1 = (((u32)dev->dev_addr[0] & 0xFF) << 24) | (((u32)dev->dev_addr[1] & 0xFF) << 16) | (((u32)dev->dev_addr[2] & 0xFF) << 8) | ((u32)dev->dev_addr[3] & 0xFF);
    addr2 = (((u32)dev->dev_addr[4] & 0xFF) << 24) | (((u32)dev->dev_addr[5] & 0xFF) << 16);
    for ( i = 0; i < 16; i++ )
        if ( ROUT_MAC_CFG_TBL(i)[0] == addr1 && ROUT_MAC_CFG_TBL(i)[1] == addr2 )
        {
            ROUT_MAC_CFG_TBL(i)[0] = (((u32)addr->sa_data[0] & 0xFF) << 24) | (((u32)addr->sa_data[1] & 0xFF) << 16) | (((u32)addr->sa_data[2] & 0xFF) << 8) | ((u32)addr->sa_data[3] & 0xFF);
            ROUT_MAC_CFG_TBL(i)[1] = (((u32)addr->sa_data[4] & 0xFF) << 24) | (((u32)addr->sa_data[5] & 0xFF) << 16);
            break;
        }
#endif

    my_memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    return 0;
}

static int eth_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    int port;

    port = get_eth_port(dev);
    if ( port < 0 )
        return -ENODEV;

    switch ( cmd )
    {
    case SIOCETHTOOL:
        return ethtool_ioctl(dev, ifr, cmd);
    case SET_VLAN_COS:
        {
            struct vlan_cos_req vlan_cos_req;

            if ( copy_from_user(&vlan_cos_req, ifr->ifr_data, sizeof(struct vlan_cos_req)) )
                return -EFAULT;
#if 0   //  TODO
            set_vlan_cos(&vlan_cos_req);
#endif
        }
        break;
    case SET_DSCP_COS:
        {
            struct dscp_cos_req dscp_cos_req;

            if ( copy_from_user(&dscp_cos_req, ifr->ifr_data, sizeof(struct dscp_cos_req)) )
                return -EFAULT;
#if 0   //  TODO
            set_dscp_cos(&dscp_cos_req);
#endif
        }
        break;
#if 0   //  TODO
    case ENABLE_VLAN_CLASSIFICATION:
        *ENETS_COS_CFG(port) |= ENETS_COS_CFG_VLAN_SET;    break;
    case DISABLE_VLAN_CLASSIFICATION:
        *ENETS_COS_CFG(port) &= ENETS_COS_CFG_VLAN_CLEAR;  break;
    case ENABLE_DSCP_CLASSIFICATION:
        *ENETS_COS_CFG(port) |= ENETS_COS_CFG_DSCP_SET;    break;
    case DISABLE_DSCP_CLASSIFICATION:
        *ENETS_COS_CFG(port) &= ENETS_COS_CFG_DSCP_CLEAR;  break;
    case VLAN_CLASS_FIRST:
        *ENETS_CFG(port) &= ENETS_CFG_FTUC_CLEAR;          break;
    case VLAN_CLASS_SECOND:
        *ENETS_CFG(port) |= ENETS_CFG_VL2_SET;             break;
    case PASS_UNICAST_PACKETS:
        *ENETS_CFG(port) &= ENETS_CFG_FTUC_CLEAR;          break;
    case FILTER_UNICAST_PACKETS:
        *ENETS_CFG(port) |= ENETS_CFG_FTUC_SET;            break;
    case KEEP_BROADCAST_PACKETS:
        *ENETS_CFG(port) &= ENETS_CFG_DPBC_CLEAR;          break;
    case DROP_BROADCAST_PACKETS:
        *ENETS_CFG(port) |= ENETS_CFG_DPBC_SET;            break;
    case KEEP_MULTICAST_PACKETS:
        *ENETS_CFG(port) &= ENETS_CFG_DPMC_CLEAR;          break;
    case DROP_MULTICAST_PACKETS:
        *ENETS_CFG(port) |= ENETS_CFG_DPMC_SET;            break;
#endif
    case ETH_MAP_PKT_PRIO_TO_Q:
        {
            struct ppe_prio_q_map cmd;

            if ( copy_from_user(&cmd, ifr->ifr_data, sizeof(cmd)) )
                return -EFAULT;

            if ( cmd.pkt_prio < 0 || cmd.pkt_prio >= NUM_ENTITY(g_eth_prio_queue_map[port]) )
                return -EINVAL;

            if ( cmd.qid < 0 )
                return -EINVAL;
            if ( cmd.qid >= ((g_wan_itf & (1 << port)) && (g_eth_wan_mode == 3) && g_wanqos_en ? __ETH_WAN_TX_QUEUE_NUM : __ETH_VIRTUAL_TX_QUEUE_NUM) )
                return -EINVAL;

            g_eth_prio_queue_map[port][cmd.pkt_prio] = cmd.qid;
        }
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static void eth_tx_timeout(struct net_device *dev)
{
    unsigned long sys_flag;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    priv->tx_errors++;

    spin_lock_irqsave(&g_eth_tx_spinlock, sys_flag);
    g_dma_device->tx_chan[2]->disable_irq(g_dma_device->tx_chan[2]);
    netif_wake_queue(g_eth_net_dev[0]);
    if ( !g_wanqos_en )
        netif_wake_queue(g_eth_net_dev[1]);
    spin_unlock_irqrestore(&g_eth_tx_spinlock, sys_flag);

    return;
}

static INLINE int get_eth_port(struct net_device *dev)
{
    return dev == g_eth_net_dev[0] ? 0 : (dev == g_eth_net_dev[1] ? 1 : -1);
}

static INLINE int eth_xmit(struct sk_buff *skb, unsigned int port, int ch, int spid, int class)
{
    struct sw_eg_pkt_header pkth = {spid:   spid,   //  CPU/app0/app1/app2
                                    dpid:   port,   //  eth0/eth1
                                    qid:    class,  //  0 is lowest priority, 3 is highest priority
                                    direct: port < 2 && spid <= 2 ? g_egress_direct_forward_enable : 0}; //  enable/disable direct forward according to flag, need re-consideration
    int len;
    struct eth_priv_data *priv = NULL;

    if ( g_stop_datapath != 0 )
        goto ETH_XMIT_DROP;

    len = skb->len <= ETH_MIN_TX_PACKET_LENGTH ? ETH_MIN_TX_PACKET_LENGTH : skb->len;

#ifdef CONFIG_IFX_PORT_MIRROR
    if ( (g_wan_itf & (1 << port)) && ifx_get_mirror_netdev && ifx_get_mirror_netdev() != NULL )
        mirror_pkt_eth(skb, 0);
#endif

    g_dma_device->current_tx_chan = ch;

    dump_skb(skb, DUMP_SKB_LEN, "eth_xmit - raw data", port, ch, 1, 0);

    if ( skb_headroom(skb) < sizeof(struct sw_eg_pkt_header) )
    {
        struct sk_buff *new_skb;

        new_skb = alloc_skb_tx(skb->len + sizeof(struct sw_eg_pkt_header));
        if ( !new_skb )
            goto ETH_XMIT_DROP;
        else
        {
            skb_put(new_skb, skb->len + sizeof(struct sw_eg_pkt_header));
            memcpy(new_skb->data + sizeof(struct sw_eg_pkt_header), skb->data, skb->len);
	    PPA_PORT_SEPARATION_COPY_MARK(skb,new_skb);	
            dev_kfree_skb_any(skb);
            skb = new_skb;
        }
    }
    else
        skb_push(skb, sizeof(struct sw_eg_pkt_header));
    len += sizeof(struct sw_eg_pkt_header);
    
    PPA_PORT_SEPARATION_TX(skb, pkth, lan_port_seperate_enabled, wan_port_seperate_enabled, port );

    memcpy(skb->data, &pkth, sizeof(struct sw_eg_pkt_header));

//  xuliang: test code for Aprotech
#if 0
    {
        int busy_count = 0, free_count = 0, i;

        for ( i = 0; i < DMA_TX_CH1_DESC_NUM; i++ ) {
            if ( DMA_TX_CH1_DESC_BASE[i].own )
                free_count++;
            else
                busy_count++;
            if ( busy_count > DMA_TX_CH1_DESC_NUM / 2 )
                goto ETH_XMIT_DROP;
            else if ( free_count >= DMA_TX_CH1_DESC_NUM / 2 )
                break;
        }
    }
#endif
//  xuliang: test code end

    if ( port < NUM_ENTITY(g_eth_net_dev) )
    {
        g_eth_net_dev[port]->trans_start = jiffies;
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[port]);
        priv->tx_packets++;
        priv->tx_bytes += len;
    }

    put_skb_to_dbg_pool(skb);
    if ( dma_device_write(g_dma_device,
                          skb->data,
                          len,
                          skb)
         != len )
        goto ETH_XMIT_DROP;

    return 0;

ETH_XMIT_DROP:
    dev_kfree_skb_any(skb);
    if ( priv )
        priv->tx_dropped++;
    return -1;
}

/*
 *  Description:
 *    Handle ioctl command SIOCETHTOOL.
 *  Input:
 *    dev --- struct net_device *, device responsing to the command.
 *    ifr --- struct ifreq *, interface request structure to pass parameters
 *            or result.
 *  Output:
 *    int --- 0:    Success
 *            else: Error Code (-EFAULT, -EOPNOTSUPP)
 */
static INLINE int ethtool_ioctl(struct net_device *dev, struct ifreq *req, int cmd)
{
    int ret;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    unsigned long flags;

    if ( !netif_running(dev) )
        return -EINVAL;

    spin_lock_irqsave(&g_mdio_lock, flags);
    ret = generic_mii_ioctl(&priv->mii, if_mii(req), cmd, NULL);
    spin_unlock_irqrestore(&g_mdio_lock, flags);

    return ret;
}

static void xway_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
    sprintf(info->driver, "PPE datapath driver info");
    sprintf(info->version, "%d.%d.%d", VER_MAJOR, VER_MID, VER_MINOR);
    strncpy(info->bus_info, dev->name, IFNAMSIZ);
}

static int xway_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    unsigned long flags;

    spin_lock_irqsave(&g_mdio_lock, flags);
    mii_ethtool_gset(&priv->mii, cmd);
    spin_unlock_irqrestore(&g_mdio_lock, flags);

    return 0;
}

static int xway_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
{
    int ret;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    unsigned long flags;

    spin_lock_irqsave(&g_mdio_lock, flags);
    ret = mii_ethtool_sset(&priv->mii, cmd);
    spin_unlock_irqrestore(&g_mdio_lock, flags);

    return ret;
}

static void xway_get_pauseparam(struct net_device *dev, struct ethtool_pauseparam *epause)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    u32 val;
    unsigned long flags;

    epause->tx_pause = epause->rx_pause = !!(*AMAZON_S_SW_RGMII_CTL & (1 << (priv->mii_port->port_num * 10)));

    spin_lock_irqsave(&g_mdio_lock, flags);
    val = mdio_read(dev, priv->mii.phy_id, MII_ADVERTISE);
    spin_unlock_irqrestore(&g_mdio_lock, flags);

    epause->autoneg =
        (val & (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM)) != 0;
}

static int xway_set_pauseparam(struct net_device *dev,
            struct ethtool_pauseparam *epause)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    u32 val;
    unsigned long flags;

    if (((*AMAZON_S_SW_RGMII_CTL & (1 << (priv->mii_port->port_num * 10))) != 0) ^
        (epause->tx_pause != 0)) {

        if (epause->tx_pause)
            *AMAZON_S_SW_RGMII_CTL |= (1 << (priv->mii_port->port_num * 10));
        else
            *AMAZON_S_SW_RGMII_CTL &= ~(1 << (priv->mii_port->port_num * 10));
    }

#if 0 /* we only support symmetric pause !? */
    if (((*AMAZON_S_SW_RGMII_CTL & (1 << (priv->mii_port->port_num * 10))) != 0) ^
        (epause->rx_pause != 0)) {

        if (epause->rx_pause)
            *AMAZON_S_SW_RGMII_CTL |= (1 << (priv->mii_port->port_num * 10));
        else
            *AMAZON_S_SW_RGMII_CTL &= ~(1 << (priv->mii_port->port_num * 10));
    }
#endif

    spin_lock_irqsave(&g_mdio_lock, flags);
    val = mdio_read(dev, priv->mii.phy_id, MII_ADVERTISE);
    if (((val & (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM)) != 0) ^
        (epause->autoneg != 0)) {

        if (epause->autoneg)
            val |= (ADVERTISE_PAUSE_CAP /*| ADVERTISE_PAUSE_ASYM*/);
        else
            val &= ~(ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

        mdio_write(dev, priv->mii.phy_id,
                MII_ADVERTISE, val);
    }
    spin_unlock_irqrestore(&g_mdio_lock, flags);

    return 0;
}

static int xway_nway_reset(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    return mii_nway_restart(&priv->mii);
}

static u32 xway_get_link(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    return mii_link_ok(&priv->mii);
}

/*
 *  Description:
 *    Allocate a sk_buff for RX path using. The size is maximum packet size
 *    plus maximum overhead size.
 *  Input:
 *    none
 *  Output:
 *    sk_buff* --- 0:    Failed
 *                 else: Pointer to sk_buff
 */
static INLINE struct sk_buff *alloc_skb_rx(void)
{
    return alloc_skb_tx(DMA_PACKET_SIZE);
}

static struct sk_buff *alloc_skb_tx_aligned(struct sk_buff *skb, int len)
{
    if ( skb )
        dev_kfree_skb_any(skb);

    skb = dev_alloc_skb(len + DMA_ALIGNMENT * 2);
    if ( skb )
    {
        skb_reserve(skb, (~((u32)skb->data + (DMA_ALIGNMENT - 1)) & (DMA_ALIGNMENT - 1)) + DMA_ALIGNMENT);
        ASSERT(((u32)skb->data & (DMA_ALIGNMENT - 1)) == 0, "skb->data (%#x) is not 8 DWORDS aligned", (u32)skb->data);
    }

    return skb;
}

static INLINE struct sk_buff *alloc_skb_tx(int len)
{
    struct sk_buff *skb;

    len = (len + DMA_ALIGNMENT - 1) & ~(DMA_ALIGNMENT - 1);

    skb = dev_alloc_skb(len);
    if ( skb )
    {
        if ( ((u32)skb->data & (DMA_ALIGNMENT - 1)) != 0 && !(skb = alloc_skb_tx_aligned(skb, len)) )
            return NULL;
        *((u32 *)skb->data - 1) = (u32)skb;
#ifndef CONFIG_MIPS_UNCACHED
        dma_cache_wback((u32)skb->data - sizeof(u32), sizeof(u32));
#endif
    }

    return skb;
}

static struct sk_buff* skb_break_away_from_protocol(struct sk_buff *skb)
{
    struct sk_buff *new_skb;

    if ( skb_shared(skb) ) {
        new_skb = skb_clone(skb, GFP_ATOMIC);
        if ( new_skb == NULL )
            return NULL;
    }
    else
        new_skb = skb_get(skb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dst_release(new_skb->dst);
    new_skb->dst = NULL;
#else
    skb_dst_drop(new_skb);
#endif
#ifdef CONFIG_XFRM
    secpath_put(new_skb->sp);
    new_skb->sp = NULL;
#endif
    if ( new_skb->destructor )
    {
        WARN_ON(in_irq());
        new_skb->destructor(new_skb);
        new_skb->destructor = NULL;
    }
#if defined(CONFIG_NETFILTER) || LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    nf_conntrack_put(new_skb->nfct);
    new_skb->nfct = NULL;
  #if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
    nf_conntrack_put_reasm(new_skb->nfct_reasm);
    new_skb->nfct_reasm = NULL;
  #endif
  #ifdef CONFIG_BRIDGE_NETFILTER
    nf_bridge_put(new_skb->nf_bridge);
    new_skb->nf_bridge = NULL;
  #endif
#endif

    return new_skb;
}

static INLINE void __get_skb_from_dbg_pool(struct sk_buff *skb, const char *func_name, unsigned int line_num)
{
#if defined(DEBUG_SKB_SWAP) && DEBUG_SKB_SWAP
    int i;

    for ( i = 0; i < NUM_ENTITY(g_dbg_skb_swap_pool) && g_dbg_skb_swap_pool[i] != skb; i++ );
    if ( i == NUM_ENTITY(g_dbg_skb_swap_pool) || (unsigned int)skb < KSEG0 )
    {
        err("%s:%d: skb (0x%08x) not in g_dbg_skb_swap_pool", func_name, line_num, (unsigned int)skb);
    }
    else
        g_dbg_skb_swap_pool[i] = NULL;
#endif
}

static INLINE struct sk_buff *__get_skb_pointer(unsigned int dataptr, const char *func_name, unsigned int line_num)
{
    unsigned int skb_dataptr;
    struct sk_buff *skb;

    //  usually, CPE memory is less than 256M bytes
    //  so NULL means invalid pointer
    if ( dataptr == 0 ) {
        dbg("dataptr is 0, it's supposed to be invalid pointer");
        return NULL;
    }

    skb_dataptr = (dataptr - 4) | KSEG1;
    skb = *(struct sk_buff **)skb_dataptr;

    __get_skb_from_dbg_pool(skb, func_name, line_num);

    ASSERT((unsigned int)skb >= KSEG0, "%s:%d: invalid skb - skb = %#08x, dataptr = %#08x", func_name, line_num, (unsigned int)skb, dataptr);
    ASSERT((((unsigned int)skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1))) | KSEG1) == (dataptr | KSEG1), "%s:%d: invalid skb - skb = %#08x, skb->data = %#08x, dataptr = %#08x", func_name, line_num, (unsigned int)skb, (unsigned int)skb->data, dataptr);

    return skb;
}

static INLINE void __put_skb_to_dbg_pool(struct sk_buff *skb, const char *func_name, unsigned int line_num)
{
#if defined(DEBUG_SKB_SWAP) && DEBUG_SKB_SWAP
    int i;

    for ( i = 0; i < NUM_ENTITY(g_dbg_skb_swap_pool) && g_dbg_skb_swap_pool[i] != NULL; i++ );
    if ( i == NUM_ENTITY(g_dbg_skb_swap_pool) )
    {
        err("%s:%d: g_dbg_skb_swap_pool overrun", func_name, line_num);
    }
    else
        g_dbg_skb_swap_pool[i] = skb;
#endif
}

static irqreturn_t mailbox0_irq_handler(int irq, void *dev_id)
{
    u32 mailbox_signal = 0;

    mailbox_signal = *MBOX_IGU0_ISR & *MBOX_IGU0_IER;
    if ( !mailbox_signal )
        return IRQ_HANDLED;
    *MBOX_IGU0_ISRC = mailbox_signal;

    if ( (mailbox_signal & DMA_TX_CH0_SIG) )
    {
        g_dma_device->tx_chan[0]->open(g_dma_device->tx_chan[0]);
        *MBOX_IGU0_IER &= DMA_TX_CH1_SIG;
        dbg("DMA_TX_CH0_SIG");
    }

    if ( (mailbox_signal & DMA_TX_CH1_SIG) )
    {
        g_dma_device->tx_chan[1]->open(g_dma_device->tx_chan[1]);
        *MBOX_IGU0_IER &= DMA_TX_CH0_SIG;
        dbg("DMA_TX_CH1_SIG");
    }

    return IRQ_HANDLED;
}

/*
 *  Description:
 *    Handle IRQ of mailbox and despatch to relative handler.
 *  Input:
 *    irq    --- int, IRQ number
 *    dev_id --- void *, argument passed when registering IRQ handler
 *    regs   --- struct pt_regs *, registers' value before jumping into handler
 *  Output:
 *    none
 */
static irqreturn_t mailbox_irq_handler(int irq, void *dev_id)
{
    u32 mailbox_signal = 0;

    mailbox_signal = *MBOX_IGU1_ISR & g_mailbox_signal_mask;
    if ( !mailbox_signal )
        return IRQ_HANDLED;
    *MBOX_IGU1_ISRC = mailbox_signal;

    if ( (mailbox_signal & CPU_TO_WAN_SWAP_SIG) )
    {
        struct sk_buff *new_skb;
        volatile struct tx_descriptor *desc = &CPU_TO_WAN_SWAP_DESC_BASE[g_cpu_to_wan_swap_desc_pos];
        struct tx_descriptor reg_desc = {0};
        struct sk_buff *skb_to_free;

        while ( desc->own == 0 )
        {
            new_skb = alloc_skb_rx();
            if ( new_skb == NULL )
                break;

#ifndef CONFIG_MIPS_UNCACHED
            /*  invalidate cache    */
            dma_cache_inv((unsigned long)new_skb->data, DMA_PACKET_SIZE);
#endif

            if ( ++g_cpu_to_wan_swap_desc_pos == CPU_TO_WAN_SWAP_DESC_NUM )
                g_cpu_to_wan_swap_desc_pos = 0;

            /*  free previous skb   */
            skb_to_free = get_skb_pointer(desc->dataptr);
            if ( skb_to_free != NULL )
                dev_kfree_skb_any(skb_to_free);
            put_skb_to_dbg_pool(new_skb);

            /*  update descriptor   */
            reg_desc.dataptr = (unsigned int)new_skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1));
            reg_desc.own     = 1;

            /*  write discriptor to memory  */
            *((volatile unsigned int *)desc + 1) = *((unsigned int *)&reg_desc + 1);
            wmb();
            *(volatile unsigned int *)desc = *(unsigned int *)&reg_desc;

            desc = &CPU_TO_WAN_SWAP_DESC_BASE[g_cpu_to_wan_swap_desc_pos];
        }
    }

    return IRQ_HANDLED;
}

static INLINE void turn_on_dma_rx(int mask)
{
    int i;

    if ( !g_f_dma_opened )
    {
        ASSERT((u32)g_dma_device >= 0x80000000, "g_dma_device = 0x%08X", (u32)g_dma_device);

        for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
        {
            ASSERT((u32)g_dma_device->rx_chan[i] >= 0x80000000, "g_dma_device->rx_chan[%d] = 0x%08X", i, (u32)g_dma_device->rx_chan[i]);
            ASSERT(g_dma_device->rx_chan[i]->control == IFX_DMA_CH_ON, "g_dma_device->rx_chan[i]->control = %d", g_dma_device->rx_chan[i]->control);

            if ( g_dma_device->rx_chan[i]->control == IFX_DMA_CH_ON )
            {
                ASSERT((u32)g_dma_device->rx_chan[i]->open >= 0x80000000, "g_dma_device->rx_chan[%d]->open = 0x%08X", i, (u32)g_dma_device->rx_chan[i]->open);

                //  channel 1, 2 is for fast path
                if ( i == 1 || i == 2 )
                    g_dma_device->rx_chan[i]->dir = 1;
                g_dma_device->rx_chan[i]->open(g_dma_device->rx_chan[i]);
                if ( i == 1 || i == 2 )
                    g_dma_device->rx_chan[i]->dir = 0;
            }
        }
    }
    g_f_dma_opened |= 1 << mask;
}

static INLINE void turn_off_dma_rx(int mask)
{
    int i;

    g_f_dma_opened &= ~(1 << mask);
    if ( !g_f_dma_opened )
        for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
            g_dma_device->rx_chan[i]->close(g_dma_device->rx_chan[i]);
}

static u8 *dma_buffer_alloc(int len, int *byte_offset, void **opt)
{
    u8 *buf;
    struct sk_buff *skb;

    skb = alloc_skb_rx();
    if ( !skb )
        return NULL;

    put_skb_to_dbg_pool(skb);

    buf = (u8 *)skb->data;
    *(u32 *)opt = (u32)skb;
    *byte_offset = 0;
    return buf;
}

static int dma_buffer_free(u8 *dataptr, void *opt)
{
    //  ! be careful
    //    this method makes kernel crash when free this DMA device
    //    since the pointers used by fastpast which is supposed not visible to CPU
    //    mix up the pointers so that "opt" is not align with "dataptr".

//    ASSERT(*(void **)((((u32)dataptr | KSEG1) & ~0x0F) - 4) == opt, "address is not match: dataptr = %08X, opt = %08X, *(void **)((((u32)dataptr | KSEG1) & ~0x0F) - 4) = %08X", (u32)dataptr, (u32)opt, *(u32 *)((((u32)dataptr | KSEG1) & ~0x0F) - 4));

    if ( opt )
    {
        get_skb_from_dbg_pool((struct sk_buff *)opt);
        dev_kfree_skb_any((struct sk_buff *)opt);
    }

    return 0;
}

#if defined(ENABLE_NAPI) && ENABLE_NAPI
static void dma_activate_poll(struct dma_device_info *dma_dev)
{
    ifx_eth_fw_poll_schedule(g_eth_net_dev[0]);
    ifx_eth_fw_poll_schedule(g_eth_net_dev[1]);
}

static void dma_inactivate_poll(struct dma_device_info *dma_dev)
{
    ifx_eth_fw_poll_complete(g_eth_net_dev[0]);
    ifx_eth_fw_poll_complete(g_eth_net_dev[1]);
}
#endif

static int dma_int_handler(struct dma_device_info *dma_dev, int status)
{
    int ret = 0;
    unsigned long sys_flag;
#if !defined(ENABLE_DIRECTPATH_TX_QUEUE) || !ENABLE_DIRECTPATH_TX_QUEUE
    int i;
#endif

    switch ( status )
    {
    case RCV_INT:
        ret = dma_rx_int_handler(dma_dev);
        break;
    case TX_BUF_FULL_INT:
        if ( g_dma_device->current_tx_chan == 2 )
        {
            dbg("eth0/1 TX buffer full!");
            spin_lock_irqsave(&g_eth_tx_spinlock, sys_flag);
            if ( !g_wanqos_en )
            {
                if ( g_eth_net_dev[0]->trans_start < g_eth_net_dev[1]->trans_start )
                {
                    g_eth_net_dev[1]->trans_start = jiffies;
                    g_eth_net_dev[0]->trans_start = jiffies + 1;
                }
                else
                {
                    g_eth_net_dev[0]->trans_start = jiffies;
                    g_eth_net_dev[1]->trans_start = jiffies + 1;
                }
                netif_stop_queue(g_eth_net_dev[1]);
            }
            netif_stop_queue(g_eth_net_dev[0]);
            if ( g_dma_device->tx_chan[2]->control == IFX_DMA_CH_ON )
                g_dma_device->tx_chan[2]->enable_irq(g_dma_device->tx_chan[2]);
            spin_unlock_irqrestore(&g_eth_tx_spinlock, sys_flag);
        }
        else if ( g_dma_device->current_tx_chan == 3 )
        {
            dbg("directpath TX (CPU->PPE) buffer full!");
            if ( !g_directpath_dma_full )
            {
                g_directpath_dma_full = 1;
                if ( g_dma_device->tx_chan[3]->control == IFX_DMA_CH_ON )
                    g_dma_device->tx_chan[3]->enable_irq(g_dma_device->tx_chan[3]);
#if !defined(ENABLE_DIRECTPATH_TX_QUEUE) || !ENABLE_DIRECTPATH_TX_QUEUE
                for ( i = 0; i < NUM_ENTITY(g_ppe_directpath_data); i++ )
                    if ( (g_ppe_directpath_data[i].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) && g_ppe_directpath_data[i].callback.stop_tx_fn )
                        g_ppe_directpath_data[i].callback.stop_tx_fn(g_ppe_directpath_data[i].netif);
#endif
            }
        }
        else
        {
            err("incorrect DMA TX channel: %d (0 - 1 is reserved for fast path)", g_dma_device->current_tx_chan);
        }
        break;
    case TRANSMIT_CPT_INT:
        if ( g_dma_device->current_tx_chan == 2 )
        {
            dbg("eth0/1 TX buffer released!");
            spin_lock_irqsave(&g_eth_tx_spinlock, sys_flag);
            g_dma_device->tx_chan[2]->disable_irq(g_dma_device->tx_chan[2]);
            netif_wake_queue(g_eth_net_dev[0]);
            if ( !g_wanqos_en )
                netif_wake_queue(g_eth_net_dev[1]);
            spin_unlock_irqrestore(&g_eth_tx_spinlock, sys_flag);
        }
        else if ( g_dma_device->current_tx_chan == 3 )
        {
            dbg("directpath TX (CPU->PPE) buffer released");
            spin_lock_irqsave(&g_directpath_tx_spinlock, sys_flag);
            if ( g_directpath_dma_full )
            {
                g_directpath_dma_full = 0;
                g_dma_device->tx_chan[3]->disable_irq(g_dma_device->tx_chan[3]);
#if !defined(ENABLE_DIRECTPATH_TX_QUEUE) || !ENABLE_DIRECTPATH_TX_QUEUE
                for ( i = 0; i < NUM_ENTITY(g_ppe_directpath_data); i++ )
                    if ( (g_ppe_directpath_data[i].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) && g_ppe_directpath_data[i].callback.start_tx_fn )
                        g_ppe_directpath_data[i].callback.start_tx_fn(g_ppe_directpath_data[i].netif);
#endif
            }
#if defined(ENABLE_DIRECTPATH_TX_QUEUE) && ENABLE_DIRECTPATH_TX_QUEUE
            send_directpath_tx_queue();
#endif
            spin_unlock_irqrestore(&g_directpath_tx_spinlock, sys_flag);
        }
        else
        {
            err("incorrect DMA TX channel: %d (0 - 1 is reserved for fast path)", g_dma_device->current_tx_chan);
        }
        break;
    default:
        err("unkown DMA interrupt event - %d", status);
    }

    return ret;
}

static INLINE int dma_rx_int_handler(struct dma_device_info *dma_dev)
{
    struct sk_buff *skb = NULL;
    u8 *buf;
    int len;
    struct flag_header *header;
    u32 off;
    int netif_rx_ret;
    struct eth_priv_data *priv;
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
    struct eth_priv_data *priv_oam=NULL;
#endif

    len = dma_device_read(dma_dev, &buf, (void **)&skb);
    if ( !skb )
        return 0;
    get_skb_from_dbg_pool(skb);

    ASSERT(buf != NULL, "skb = %08x, buf = %08x", (unsigned int)skb, (unsigned int)buf);
    ASSERT((u32)skb >= 0x80000000, "can not restore skb pointer (ch %d) --- skb = 0x%08X", dma_dev->current_rx_chan, (u32)skb);
    ASSERT(skb->data == buf && skb->tail == buf, "skb->data (%08X) or skb->tail (%08X) is corrupted", (u32)skb->data, (u32)skb->tail);
    ASSERT(len <= (u32)skb->end - (u32)skb->data, "pakcet is too large: %d", len);
    ASSERT(*(struct sk_buff **)((((u32)buf | KSEG1) & ~0x1F) - 4) == skb, "address is not match: skb = %08X, *(struct sk_buff **)((((u32)buf | KSEG1) & ~0x0F) - 4) = %08X", (u32)skb, *(u32 *)((((u32)buf | KSEG1) & ~0x1F) - 4));

    dump_flag_header(g_fwcode, (struct flag_header *)skb->data, __FUNCTION__, dma_dev->current_rx_chan, 0);

    header = (struct flag_header *)skb->data;

    //  pl_byteoff
    off = header->pl_byteoff;

    skb->tail = skb->data + len;
    skb->len =  len;
    dump_skb(skb, DUMP_SKB_LEN, "dma_rx_int_handler raw data", (header->src_itf) & 0x07, dma_dev->current_rx_chan, 0, 0);
   // printk("The payload offset: %d:(0x%x)\n",  sizeof(struct flag_header) + off, sizeof(struct flag_header) + off);

    len -=  off + 8; //8 is 2 leading dword, FW used this to back compatible to old version. whose flag header size is 2 dwords
    ASSERT(len >= 60, "pakcet is too small: %d", len);

    skb->data +=  off + 8;
    skb->tail =  skb->data + len;
    skb->len  =  len;
    
    PPA_PORT_SEPARATION_RX(header,skb,lan_port_seperate_enabled,wan_port_seperate_enabled);
    
    skb->priority = header->tc;

    dump_skb(skb, DUMP_SKB_LEN, "dma_rx_int_handler packet data", (header->src_itf) & 0x07, dma_dev->current_rx_chan, 0, 0);

    //  implement indirect fast path
    if ( header->acc_done && header->dest_list )    //  acc_done == 1 && dest_list != 0
    {
        //  Current implementation send packet to highest priority dest only (0 -> 7, 0 is highest)
        //  2 is CPU, so we ignore it here
        if ( (header->dest_list & (3 << 0)) )           //  0 - eth0, 1 - eth1
        {
            int itf = (header->dest_list & (1 << 1)) ? 1 : 0;

            if ( (header->dest_list & g_wan_itf) )
            {
                skb->cb[13] = 0x5A; //  magic number indicating directpath
                skb->cb[15] = header->qid & 0x07;
                eth_qos_hard_start_xmit(skb, g_eth_net_dev[itf]);
            }
            else
                eth_xmit(skb, itf, 2, 2, g_wan_queue_class_map[header->qid & 0x07]);
            return 0;
        }
        else
        {
            for ( off = 3; off < 3 + MAX_DIRECTPATH_NUM; off++ )
            {
                if ( (header->dest_list & (1 << off)) ) //  3...7
                {
                    int if_id = off - 3;

                    if ( (g_ppe_directpath_data[if_id].flags & PPE_DIRECTPATH_DATA_RX_ENABLE) )
                    {
                        //  fill MAC info into skb, in case the driver hooked to direct path need such info
                        skb->dev = g_ppe_directpath_data[if_id].netif;
                        skb->protocol = eth_type_trans(skb, skb->dev);
                        skb_push(skb, ETH_HLEN);

                        //  no RX interface information, indirect xmit
                        g_ppe_directpath_data[if_id].rx_fn_txif_pkt++;
                        g_ppe_directpath_data[if_id].callback.rx_fn(NULL, g_ppe_directpath_data[if_id].netif, skb, skb->len);
                        return 0;
                    }
                }
            }
            dev_kfree_skb_any(skb);
            return 0;
        }
    }

    off = header->src_itf;
    switch ( off )
    {
    case 0: //  MII0
    case 1: //  MII1
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[off]);
        if ( netif_running(g_eth_net_dev[off]) )
        {
            //  do not need to remove CRC
            //skb->tail -= ETH_CRC_LENGTH;
            //skb->len  -= ETH_CRC_LENGTH;

            {
                skb->dev = g_eth_net_dev[off];
                skb->protocol = eth_type_trans(skb, g_eth_net_dev[off]);
            }
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
            if (skb->protocol == 0x8902) {
                   if(fp_ltq_eth_oam_dev != NULL) {
                     g_ethoam_net_dev = fp_ltq_eth_oam_dev();
                     skb->dev = g_ethoam_net_dev;
                     priv_oam = ifx_eth_fw_netdev_priv(skb->dev);
                   }
             }
#endif

#if defined(ENABLE_NAPI) && ENABLE_NAPI
            netif_rx_ret = g_napi_enable ? netif_receive_skb(skb) : netif_rx(skb);
#else
            netif_rx_ret = netif_rx(skb);
#endif
            if ( netif_rx_ret == NET_RX_DROP )
            {
                priv->rx_dropped++;
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
            if (skb->protocol == 0x8902) {
                priv_oam->stats.rx_dropped++;
            }
#endif
            }
            else
            {
                priv->rx_packets++;
                priv->rx_bytes += skb->len;
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
            if (skb->protocol == 0x8902) {
                priv_oam->stats.rx_packets++;
                priv_oam->stats.rx_bytes += skb->len;
            }
#endif
            }

            return 0;
        }
        else
            priv->rx_dropped++;
        break;
    default:    //  other interface receive
        if(off < 3 || off >= 3 + MAX_DIRECTPATH_NUM){
            break;
        }
        if ( (g_ppe_directpath_data[off - 3].flags & PPE_DIRECTPATH_DATA_RX_ENABLE) )
        {
            skb->dev = g_ppe_directpath_data[off - 3].netif;
            //  no TX interface information, device push
            g_ppe_directpath_data[off - 3].rx_fn_rxif_pkt++;
            g_ppe_directpath_data[off - 3].callback.rx_fn(g_ppe_directpath_data[off - 3].netif, NULL, skb, skb->len);
            return 0;
        }
    }

    dev_kfree_skb_any(skb);
    return 0;
}

#define MDIOADDR2PHYID(__x)     (((__x) >> 5)& 0x1F)
#define MDIOADDR2PHYREG(__x)    ((__x) & 0x1F)
static int mdio_read(struct net_device *dev, int phy_id, int location)
{
    int result;

    while ( (*AMAZON_S_SW_MDIO_CTL & 0x8000) );
    *AMAZON_S_SW_MDIO_CTL = 0x8000 | MDIO_OP_READ | ((phy_id & 0x1F) << 5) | (location & 0x1F);
    while ( (*AMAZON_S_SW_MDIO_CTL & 0x8000) );

    /* should not be needed, but without this, sometimes old data is read!? */
    udelay(1);

    result = (u16)*AMAZON_S_SW_MDIO_DATA;
    *AMAZON_S_SW_MDIO_DATA = 0xFFFF;

    return result;
}

static void mdio_write(struct net_device *dev, int phy_id, int location, int val)
{
    while ( (*AMAZON_S_SW_MDIO_CTL & 0x8000) );

    /* should not be needed, but without this, sometimes write fails!? */
    udelay(1);

    *AMAZON_S_SW_MDIO_CTL = 0x8000 | (((u32)val) << 16) | MDIO_OP_WRITE | ((phy_id & 0x1F) << 5) | (location & 0x1F);
    __sync();

    return;
}

static INLINE u16 smi_reg_read(u16 reg)
//  reg is ((phy_addr & 0x1F) << 5) | (reg_addr & 0x1F)
{
    return (u16)mdio_read(NULL, MDIOADDR2PHYID(reg), MDIOADDR2PHYREG(reg));
}

static INLINE void smi_reg_write(u16 reg, u16 data)
//  reg is ((phy_addr & 0x1F) << 5) | (reg_addr & 0x1F)
{
    mdio_write(NULL, MDIOADDR2PHYID(reg), MDIOADDR2PHYREG(reg), data);
}

static INLINE void init_mdio(void)
{
    IFX_SWITCH_PIN_RESERVE(SWITCH_MDIO);
    IFX_SWITCH_ALTSEL0_SET(SWITCH_MDIO);
    IFX_SWITCH_ALTSEL1_CLR(SWITCH_MDIO);
    IFX_SWITCH_DIR_OUT(SWITCH_MDIO);
    IFX_SWITCH_OD_SET(SWITCH_MDIO);

    IFX_SWITCH_PIN_RESERVE(SWITCH_MDC);
    IFX_SWITCH_ALTSEL0_SET(SWITCH_MDC);
    IFX_SWITCH_ALTSEL1_CLR(SWITCH_MDC);
    IFX_SWITCH_DIR_OUT(SWITCH_MDC);
    IFX_SWITCH_OD_SET(SWITCH_MDC);
}

static void probe_mii_ports(void)
{
#ifndef CONFIG_MII1_AUTONEG /*  fixed (RG)MII configuration */
    g_mii_port[0].phy_addr = 0;
    g_mii_port[0].phy_id   = TANTOS_CHIP_ID;
    g_mii_port[0].mode     = MII0_MODE_SETUP;
    g_mii_port[1].phy_addr = 0;
    g_mii_port[1].phy_id   = 0xFFFFFFFF;
    g_mii_port[1].mode     = MII1_MODE_SETUP;
#else
    unsigned short mdio_value;
    int probe_phyad = 0;
    unsigned int phy_id;
    unsigned char mode;

    /*  probe for TANTOS chip ID register   */
    mdio_value = smi_reg_read(0x0101);

    if ( mdio_value == TANTOS_CHIP_ID )
    {
        /*  TANTOS per definition connected to Port 0 in RGMII mode */
        printk("TANTOS_CHIP_ID@0-f, ");

        g_mii_port[0].phy_addr = 0;
        g_mii_port[0].phy_id   = TANTOS_CHIP_ID;
        g_mii_port[0].mode     = RGMII_MODE;

        probe_phyad = 16;
    }

    /*  Note: Even PHY address get assigned to port 0,
        odd PHY address gets assigned to port 1 !       */
    for ( ; probe_phyad < 32; probe_phyad++ )
    {
        mdio_value = mdio_read(NULL, probe_phyad, 2);
        phy_id = mdio_value << 16;
        mdio_value = mdio_read(NULL, probe_phyad, 3);
        phy_id |= mdio_value;

        switch ( phy_id )
        {
        case PHYID_LANTIQ_PHY11G:
            printk("PHYID_LANTIQ_PHY11G@%x, ", probe_phyad);
            mode = RGMII_MODE;
            break;

        case PHYID_IFX_PHY11G:
            printk("PHYID_IFX_PHY11G@%x, ", probe_phyad);
            mode = RGMII_MODE;
            break;

        case PHYID_VSC_SIMPLIPHY:
            printk("PHYID_VSC_SIMPLIPHY@%x, ", probe_phyad);
            mode = RGMII_MODE;
            break;

        default:
            if ( probe_phyad == 0x11 || phy_id != 0xFFFFFFFF )
                printk("unknown(%08x)@%x, ", phy_id, probe_phyad);
            mode = 0xFF;
        }

        if ( mode != 0xFF )
        {
            g_mii_port[probe_phyad & 0x01].phy_addr = probe_phyad;
            g_mii_port[probe_phyad & 0x01].phy_id   = phy_id;
            g_mii_port[probe_phyad & 0x01].mode     = mode;
        }
    }

    /*  hack for SuperVB board  */
    if ( g_mii_port[0].phy_id == 0 && g_mii_port[1].phy_id == 0 )
    {
        printk("MII1 fixed to MII_MODE, ");
        g_mii_port[1].phy_id = 0xFFFFFFFF;
        g_mii_port[1].mode = MII_MODE;
    }
#endif
}

static INLINE void init_external_tantos(void)
{
    u16 value;

    smi_reg_write(0x00a1, 0x0004);  //  PORT 5
    udelay(500);    //  fix me
    value = smi_reg_read(0x00a1);
    if ( value != 0x0004 )
        err("error: port 5 force link up failed");

    smi_reg_write(0x00c1, 0x0004);  //  PORT 6
    udelay(500);    //  fix me
    value = smi_reg_read(0x00c1);
    if ( value != 0x0004 )
        err("error: port 6 force link up failed");

#if defined(ENABLE_SWITCH_FLOW_CONTROL) && ENABLE_SWITCH_FLOW_CONTROL
    smi_reg_write(0x00f5, 0x0bbb);  //  enable flow control
    udelay(500);    //  fix me
    value = smi_reg_read(0x00f5);
    if ( value != 0x0bbb )
        err("error: enable flow control failed");
#else
    smi_reg_write(0x00f5, 0x0aaa);  //  disable flow control
    udelay(500);
    value = smi_reg_read(0x00f5);
    if ( value != 0x0aaa )
        err("error: disable flow control failed");
#endif

    //  Set Bypass mode
    //    P0 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0x03));
    smi_reg_write(0x03, (value| 0x80));
    //    P1 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0x23));
    smi_reg_write(0x23, (value| 0x80));
    //    P2 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0x43));
    smi_reg_write(0x43, (value| 0x80));
    //    P3 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0x63));
    smi_reg_write(0x63, (value| 0x80));
    //    P4 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0x83));
    smi_reg_write(0x83, (value| 0x80));
    //    P5 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0xa3));
    smi_reg_write(0xa3, (value| 0x80));
    //    P6 portbase vlan map reg
    value = (unsigned short)(smi_reg_read(0xc3));
    smi_reg_write(0xc3, (value| 0x80));
}

static INLINE void init_gphy_v14(unsigned int mii_port)
{
    u16 fw_version, gctrl_reg;

    if (mii_port >= sizeof(g_mii_port)/sizeof(struct mii_port)) {
        return;
    }

    /* workaround for v14ES */
    fw_version = (u16)mdio_read(NULL, g_mii_port[mii_port].phy_addr, 0x1E /*MII_ISTAT*/);
    printk("GPHY v1.4 FW Version: 0x%x\n", fw_version);
    if (fw_version == 0x8301) {
        gctrl_reg = (u16)mdio_read(NULL, g_mii_port[mii_port].phy_addr, 0x09 /*GCTRL*/);
        gctrl_reg |= (1<<10) /*MSPT*/;
        mdio_write(NULL, g_mii_port[mii_port].phy_addr, 0x09 /*GCTRL*/, gctrl_reg);
    }

    if ( fw_version == 0x8301 || fw_version == 0x8304 ) {
        /* set up an internal register access */
        mdio_write(NULL, g_mii_port[mii_port].phy_addr, 0x0D /*MMDCTRL */, 0x001F);
        /* specify the internal register address */
        mdio_write(NULL, g_mii_port[mii_port].phy_addr, 0x0E /*MMDDATA */, 0x0FC6);
        /* specify a data access to the previously specified internal register address */
        mdio_write(NULL, g_mii_port[mii_port].phy_addr, 0x0D /*MMDCTRL */, 0x401F);
        /* correct an internal setting */
        mdio_write(NULL, g_mii_port[mii_port].phy_addr, 0x0E /*MMDDATA */, 0x0052);
        /* finish this MMD access */
        mdio_write(NULL, g_mii_port[mii_port].phy_addr, 0x0D /*MMDCTRL */, 0x0000);
    }
}

static INLINE void mii1_gphy_config_gpio(void)
{
#ifdef CONFIG_MII1_PHY_CLOCK_GPIO3
    //  Select Clock_Out2 @25MHz
    *AMAZON_S_CGU_IFCCR &= ~(3 << 10);
    //  Select GPIO P0.3 for CLOCK OUT 2
    IFX_SWITCH_PIN_RESERVE(CLOCK_OUT2);
    IFX_SWITCH_ALTSEL0_SET(CLOCK_OUT2);
    IFX_SWITCH_ALTSEL1_CLR(CLOCK_OUT2);
    IFX_SWITCH_DIR_OUT(CLOCK_OUT2);
    IFX_SWITCH_OD_SET(CLOCK_OUT2);
#endif

#ifdef CONFIG_MII1_AUTONEG
  #if MII1_GPHY_INT_EXT_INT_NO == 0
    IFX_SWITCH_PIN_RESERVE(0);
    IFX_SWITCH_ALTSEL0_SET(0);
    IFX_SWITCH_ALTSEL1_CLR(0);
    IFX_SWITCH_DIR_IN(0);
  #elif MII1_GPHY_INT_EXT_INT_NO == 1
    IFX_SWITCH_PIN_RESERVE(1);
    IFX_SWITCH_ALTSEL0_SET(1);
    IFX_SWITCH_ALTSEL1_CLR(1);
    IFX_SWITCH_DIR_IN(1);
  #elif MII1_GPHY_INT_EXT_INT_NO == 2
    IFX_SWITCH_PIN_RESERVE(2);
    IFX_SWITCH_ALTSEL0_CLR(2);
    IFX_SWITCH_ALTSEL1_SET(2);
    IFX_SWITCH_DIR_IN(2);
  #elif MII1_GPHY_INT_EXT_INT_NO == 3
    IFX_SWITCH_PIN_RESERVE(39);
    IFX_SWITCH_ALTSEL0_CLR(39);
    IFX_SWITCH_ALTSEL1_SET(39);
    IFX_SWITCH_DIR_IN(39);
  #elif MII1_GPHY_INT_EXT_INT_NO == 4
    IFX_SWITCH_PIN_RESERVE(10);
    IFX_SWITCH_ALTSEL0_SET(10);
    IFX_SWITCH_ALTSEL1_SET(10);
    IFX_SWITCH_DIR_IN(10);
  #elif MII1_GPHY_INT_EXT_INT_NO == 5
    IFX_SWITCH_PIN_RESERVE(9);
    IFX_SWITCH_ALTSEL0_SET(9);
    IFX_SWITCH_ALTSEL1_SET(9);
    IFX_SWITCH_DIR_IN(9);
  #else
    #error ERROR EXT INT NO
  #endif
#endif

    IFX_SWITCH_PIN_RESERVE(RESET_GPHY);
    IFX_SWITCH_ALTSEL0_CLR(RESET_GPHY);
    IFX_SWITCH_ALTSEL1_CLR(RESET_GPHY);
    IFX_SWITCH_DIR_OUT(RESET_GPHY);
    IFX_SWITCH_OD_SET(RESET_GPHY);
}

#ifdef CONFIG_MII1_AUTONEG

static INLINE void mii1_gphy_config_ext_int(void)
{
    *AMAZON_S_EIU_EXIN_C |= 0x6 << (MII1_GPHY_INT_EXT_INT_NO * 4);  //  Low Level (GPHY_VSC uses Falling Edge <0x02> which could be covered by Low Level <0x06> trigger)
    *AMAZON_S_EIU_INEN   |= 1 << MII1_GPHY_INT_EXT_INT_NO;          //  Enable External interrupt
}

static irqreturn_t mii1_gphy_11G_int_handler(int irq, void *dev_id)
{
    u16 status, linkReg, speedReg;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv((struct net_device *)dev_id);

    status = mdio_read(NULL, priv->mii.phy_id, MII_ISTAT);  //  read data from phy register to clear phy interrupts
    if ( (status & 0x0007) )  //  interrupt due to link status, speed, full duplex
    {
        linkReg = mdio_read(NULL, priv->mii.phy_id, MII_BMSR);
        if ( (linkReg & 0x04) == 0 )    //  link down
        {
            dbg("MII%d - link down", priv->mii_port->port_num);
            if ( priv->mii_port->port_num == 0 )
                *AMAZON_S_SW_P0_CTL = (*AMAZON_S_SW_P0_CTL & ~(1 << 18)) | (1 << 17);   //  force link down
            else
                *AMAZON_S_SW_P1_CTL = (*AMAZON_S_SW_P1_CTL & ~(1 << 18)) | (1 << 17);   //  force link down
        }
        else
        {
            speedReg = mdio_read(NULL, priv->mii.phy_id, 0x18);
            dbg("MII%d - link up, speedReg = %04x", priv->mii_port->port_num, (u32)speedReg);

            if ( (speedReg & 0x08) )
            {
                dbg("MII%d - full duplex", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL |= 1 << (priv->mii_port->port_num * 10 + 1);
            }
            else
            {
                dbg("MII%d - half duplex", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL &= ~(1 << (priv->mii_port->port_num * 10 + 1));
            }

            *AMAZON_S_SW_RGMII_CTL &= ~(0x0C << (priv->mii_port->port_num * 10));
            switch ( (speedReg  & 0x03) )
            {
            case 0:
                dbg("MII%d - 10BASE-T", priv->mii_port->port_num);
                g_link_speed = 10;
                break;
            case 1:
                dbg("MII%d - 100BASE-TX", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL |= 1 << (priv->mii_port->port_num * 10 + 2);
                g_link_speed = 100;
                break;
            case 2:
                dbg("MII%d - 1000BASE-T", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL |= 2 << (priv->mii_port->port_num * 10 + 2);
                g_link_speed = 1000;
                break;
            case 3:
                err("MII%d - unknown link speed", priv->mii_port->port_num);
                g_link_speed = 0;
                break;
            }

            if ( g_link_speed )
            {
                if ( priv->mii_port->port_num == 0 )
                {
                    *AMAZON_S_SW_P0_CTL &= ~(1 << 17);  //  disable force link down
                    *AMAZON_S_SW_P0_CTL |= 0x00040000;  //  force link up
                }
                else
                {
                    *AMAZON_S_SW_P1_CTL &= ~(1 << 17);  //  disable force link down
                    *AMAZON_S_SW_P1_CTL |= 0x00040000;  //  force link up
                }
            }
        }
    }
    else
        return IRQ_NONE;

    return IRQ_HANDLED;
}

static irqreturn_t mii1_gphy_VSC_int_handler(int irq, void *dev_id)
{
    u16 status, linkReg, speedReg;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv((struct net_device *)dev_id);

    status = mdio_read(NULL, priv->mii.phy_id, MII_ISTAT);  //  read data from phy register to clear phy interrupts
    if ( (status & 0x8000)      //  interrupt status bit
        && (status & 0x7000) )  //  interrupt due to link status, speed, full duplex
    {
        linkReg = mdio_read(NULL, priv->mii.phy_id, MII_BMSR);
        if ( (linkReg & 0x04) == 0 )    //  link down
        {
            dbg("MII%d - link down", priv->mii_port->port_num);
            if ( priv->mii_port->port_num == 0 )
                *AMAZON_S_SW_P0_CTL = (*AMAZON_S_SW_P0_CTL & ~(1 << 18)) | (1 << 17);   //  force link down
            else
                *AMAZON_S_SW_P1_CTL = (*AMAZON_S_SW_P2_CTL & ~(1 << 18)) | (1 << 17);   //  force link down
        }
        else
        {
            speedReg = mdio_read(NULL, priv->mii.phy_id, 0x1C);
            dbg("MII%d - link up, speedReg = %04x", priv->mii_port->port_num, (u32)speedReg);

            if ( (speedReg & 0x20) )
            {
                dbg("MII%d - full duplex", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL |= 1 << (priv->mii_port->port_num * 10 + 1);
            }
            else
            {
                dbg("MII%d - half duplex", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL &= ~(1 << (priv->mii_port->port_num * 10 + 1));
            }

            *AMAZON_S_SW_RGMII_CTL &= ~(0x0C << (priv->mii_port->port_num * 10));
            switch ( ((speedReg >> 3)  & 0x03) )
            {
            case 0:
                dbg("MII%d - 10BASE-T", priv->mii_port->port_num);
                g_link_speed = 10;
                break;
            case 1:
                dbg("MII%d - 100BASE-TX", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL |= 1 << (priv->mii_port->port_num * 10 + 2);
                g_link_speed = 100;
                break;
            case 2:
                dbg("MII%d - 1000BASE-T", priv->mii_port->port_num);
                *AMAZON_S_SW_RGMII_CTL |= 2 << (priv->mii_port->port_num * 10 + 2);
                g_link_speed = 1000;
                break;
            case 3:
                err("MII%d - unknown link speed", priv->mii_port->port_num);
                g_link_speed = 0;
                break;
            }

            if ( g_link_speed )
            {
                if ( priv->mii_port->port_num == 0 )
                {
                    *AMAZON_S_SW_P0_CTL &= ~(1 << 17);  //  disable force link down
                    *AMAZON_S_SW_P0_CTL |= 0x00040000;  //  force link up
                }
                else
                {
                    *AMAZON_S_SW_P1_CTL &= ~(1 << 17);  //  disable force link down
                    *AMAZON_S_SW_P1_CTL |= 0x00040000;  //  force link up
                }
            }
        }
    }
    else
        return IRQ_NONE;

    return IRQ_HANDLED;
}

#endif

static INLINE void reset_ppe(void)
{
#ifdef MODULE
    if ( (*DM_RXCFG & 0xFF) == 0x30 )
        return;

    //  reset PPE
    ifx_rcu_rst(IFX_RCU_DOMAIN_PPE, IFX_RCU_MODULE_PPA);
    udelay(1000);
#endif
}

static INLINE void reset_gphy(void)
{
    //  reset GPHY - P2.0
    IFX_SWITCH_OUTPUT_SET(RESET_GPHY);
    mdelay(50);
    IFX_SWITCH_OUTPUT_CLR(RESET_GPHY);
    mdelay(50);
    IFX_SWITCH_OUTPUT_SET(RESET_GPHY);
    mdelay(300);
}

static INLINE void init_pmu(void)
{
    //   5 - DMA, should be enabled in DMA driver
    //   9 - DSL DFE/AFE
    //  15 - AHB
    //  21 - PPE TC
    //  22 - PPE EMA
    //  23 - PPE DPLUS
    //  28 - SWITCH

    //  code as reference
    //*PMU_PWDCR &= ~((1 << 28) | (1 << 23) | (1 << 22) | (1 << 21) | (1 << 15) | (1 << 9) | (1 << 5));

    PPE_TOP_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_SLL01_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_EMA_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_TPE_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_DPLUS_PMU_SETUP(IFX_PMU_ENABLE);
    SWITCH_PMU_SETUP(IFX_PMU_ENABLE);
}

static INLINE void init_mii0(int mii0_mode)
{
    *AMAZON_S_SW_P0_CTL = (*AMAZON_S_SW_P2_CTL & ~(1 << 18)) | (1 << 17);   //  force link down

    if ( mii0_mode == REV_MII_MODE || mii0_mode == TURBO_REV_MII_MODE || mii0_mode == MII_MODE )
    {
        IFX_SWITCH_PIN_RESERVE(MII0_COL);
        IFX_SWITCH_ALTSEL0_SET(MII0_COL);
        IFX_SWITCH_ALTSEL1_CLR(MII0_COL);
        if ( mii0_mode == MII_MODE )
            IFX_SWITCH_DIR_IN(MII0_COL);
        else
        {
            IFX_SWITCH_DIR_OUT(MII0_COL);
            IFX_SWITCH_OD_SET(MII0_COL);
        }

        IFX_SWITCH_PIN_RESERVE(MII0_CRS);
        IFX_SWITCH_ALTSEL0_SET(MII0_CRS);
        IFX_SWITCH_ALTSEL1_CLR(MII0_CRS);
        if ( mii0_mode == MII_MODE )
        {
            IFX_SWITCH_DIR_IN(MII0_CRS);
            /* Pull-Down for Rx_ERR pin since it is probably not connected */
            IFX_SWITCH_PUDSEL_CLEAR(MII0_RXERR);
            IFX_SWITCH_PUDEN_SET(MII0_RXERR);
        }
        else
        {
            IFX_SWITCH_DIR_OUT(MII0_CRS);
            IFX_SWITCH_OD_SET(MII0_CRS);
        }

        IFX_SWITCH_PIN_RESERVE(MII0_TXERR);
        IFX_SWITCH_ALTSEL0_SET(MII0_TXERR);
        IFX_SWITCH_ALTSEL1_CLR(MII0_TXERR);
        if ( mii0_mode == MII_MODE )
        {
            IFX_SWITCH_DIR_OUT(MII0_TXERR);
            IFX_SWITCH_OD_SET(MII0_TXERR);
        }
        else
            IFX_SWITCH_DIR_IN(MII0_TXERR);

        IFX_SWITCH_PIN_RESERVE(MII0_RXERR);
        IFX_SWITCH_ALTSEL0_SET(MII0_RXERR);
        IFX_SWITCH_ALTSEL1_CLR(MII0_RXERR);
        if ( mii0_mode == MII_MODE )
            IFX_SWITCH_DIR_IN(MII0_RXERR);
        else
        {
            IFX_SWITCH_DIR_OUT(MII0_RXERR);
            IFX_SWITCH_OD_SET(MII0_RXERR);
        }
    }

    if ( g_mii_port[0].phy_id == TANTOS_CHIP_ID )
        init_external_tantos();

    if ( g_mii_port[0].phy_id == PHYID_LANTIQ_PHY11G )
        init_gphy_v14(0);

    *AMAZON_S_SW_RGMII_CTL &= 0xffcffc0f;

#if defined(ENABLE_MII0_BYPASS_DELAY_FIX) && ENABLE_MII0_BYPASS_DELAY_FIX
    if ( g_mii_port[0].phy_id == TANTOS_CHIP_ID )
    {
        *AMAZON_S_SW_RGMII_CTL |= 2 << 6;   //  set RX delay, SWITCH Port 0 Rx DLL 1.75ns
        *AMAZON_S_RCU_PPE_CONF = (*AMAZON_S_RCU_PPE_CONF & 0xFFFF81FF) | (5 << 9);  //  Bypass of DLL of SWITCH Port 0 Tx 1.75ns delay
    }
    else
        *AMAZON_S_RCU_PPE_CONF = (*AMAZON_S_RCU_PPE_CONF & 0xFFFF81FF) | (0x20 << 9);   //  Bypass of DLL of SWITCH Port 0 Tx 1.75ns delay
#endif

    *AMAZON_S_SW_RGMII_CTL |= (mii0_mode & 0x03) << 8;

    switch ( mii0_mode )
    {
    case RGMII_MODE:
        *AMAZON_S_SW_RGMII_CTL = (*AMAZON_S_SW_RGMII_CTL & 0xfffffff3) | 0x08; break;
    case RGMII_MODE_100MB:
        *AMAZON_S_SW_RGMII_CTL = (*AMAZON_S_SW_RGMII_CTL & 0xfffffff3) | 0x04; break;
    case RGMII_MODE_10MB:
        *AMAZON_S_SW_RGMII_CTL = *AMAZON_S_SW_RGMII_CTL & 0xfffffff3; break;
    case TURBO_REV_MII_MODE:
        *AMAZON_S_SW_RGMII_CTL |= 1 << 20; break;
    case RED_MII_MODE_OC:
        *AMAZON_S_SW_RGMII_CTL |= 1 << 21; break;
    }

#if defined(ENABLE_SWITCH_FLOW_CONTROL) && ENABLE_SWITCH_FLOW_CONTROL
    *AMAZON_S_SW_RGMII_CTL |= 1;
#else
    *AMAZON_S_SW_RGMII_CTL &= ~1;
#endif

    if ( g_ingress_direct_forward_enable )
        *AMAZON_S_SW_P0_CTL |= (1 << 19);   //  enable port ingress direct forwarding
    else
        *AMAZON_S_SW_P0_CTL &= ~(1 << 19);  //  disable port ingress direct forwarding
    *AMAZON_S_SW_P0_CTL &= ~(1 << 13);      //  disable port redirect
    *AMAZON_S_SW_P0_CTL |= 0x00000001;      //  the output packet keep the same format as receive
    if ( g_mii_port[0].phy_id == TANTOS_CHIP_ID )
    {
        *AMAZON_S_SW_P0_CTL &= ~(1 << 17);  //  disable force link down
        *AMAZON_S_SW_P0_CTL |= 1 << 18;     //  force link up
    }
}

static INLINE void init_mii1(int mii1_mode)
{
    *AMAZON_S_SW_P1_CTL = (*AMAZON_S_SW_P2_CTL & ~(1 << 18)) | (1 << 17);   //  force link down

    if ( mii1_mode == REV_MII_MODE || mii1_mode ==TURBO_REV_MII_MODE || mii1_mode == MII_MODE )
    {
        IFX_SWITCH_PIN_RESERVE(MII1_COL);
        IFX_SWITCH_ALTSEL0_SET(MII1_COL);
        IFX_SWITCH_ALTSEL1_CLR(MII1_COL);
        if ( mii1_mode == MII_MODE )
            IFX_SWITCH_DIR_IN(MII1_COL);
        else
        {
            IFX_SWITCH_DIR_OUT(MII1_COL);
            IFX_SWITCH_OD_SET(MII1_COL);
        }

        IFX_SWITCH_PIN_RESERVE(MII1_CRS);
        IFX_SWITCH_ALTSEL0_SET(MII1_CRS);
        IFX_SWITCH_ALTSEL1_CLR(MII1_CRS);
        if ( mii1_mode == MII_MODE )
            IFX_SWITCH_DIR_IN(MII1_CRS);
        else
        {
            IFX_SWITCH_DIR_OUT(MII1_CRS);
            IFX_SWITCH_OD_SET(MII1_CRS);
        }

        IFX_SWITCH_PIN_RESERVE(MII1_TXERR);
        IFX_SWITCH_ALTSEL0_SET(MII1_TXERR);
        IFX_SWITCH_ALTSEL1_CLR(MII1_TXERR);
        if ( mii1_mode == MII_MODE )
        {
            IFX_SWITCH_DIR_OUT(MII1_TXERR);
            IFX_SWITCH_OD_SET(MII1_TXERR);
        }
        else
            IFX_SWITCH_DIR_IN(MII1_TXERR);

        IFX_SWITCH_PIN_RESERVE(MII1_RXERR);
        IFX_SWITCH_ALTSEL0_SET(MII1_RXERR);
        IFX_SWITCH_ALTSEL1_CLR(MII1_RXERR);
        if ( mii1_mode == MII_MODE )
        {
            IFX_SWITCH_DIR_IN(MII1_RXERR);
            /* Pull-Down for Rx_ERR pin since it is probably not connected */
            IFX_SWITCH_PUDSEL_CLEAR(MII1_RXERR);
            IFX_SWITCH_PUDEN_SET(MII1_RXERR);
        }
        else
        {
            IFX_SWITCH_DIR_OUT(MII1_RXERR);
            IFX_SWITCH_OD_SET(MII1_RXERR);
        }
    }

    if ( g_mii_port[1].phy_id == PHYID_LANTIQ_PHY11G )
        init_gphy_v14(1);

    *AMAZON_S_SW_RGMII_CTL &= 0xff303fff;

#if defined(ENABLE_MII1_BYPASS_DELAY_FIX) && ENABLE_MII1_BYPASS_DELAY_FIX
    *AMAZON_S_RCU_PPE_CONF = (*AMAZON_S_RCU_PPE_CONF & 0xFFE07FFF) | (0x20 << 15); //  Bypass of DLL of SWITCH Port 1 Tx 1.5ns, SWITCH Port 1 Rx DLL 0ns
#endif

    *AMAZON_S_SW_RGMII_CTL |= (mii1_mode & 0x03) << 18;

    switch ( mii1_mode )
    {
    case RGMII_MODE:
        *AMAZON_S_SW_RGMII_CTL = (*AMAZON_S_SW_RGMII_CTL & 0xffffcfff) | 0x2000; break;
    case RGMII_MODE_100MB:
        *AMAZON_S_SW_RGMII_CTL = (*AMAZON_S_SW_RGMII_CTL & 0xffffcfff) | 0x1000; break;
    case RGMII_MODE_10MB:
        *AMAZON_S_SW_RGMII_CTL = *AMAZON_S_SW_RGMII_CTL & 0xffffcfff; break;
    case TURBO_REV_MII_MODE:
        *AMAZON_S_SW_RGMII_CTL |= 1 << 22; break;
    case RED_MII_MODE_OC:
        *AMAZON_S_SW_RGMII_CTL |= 1 << 23; break;
    }

#if defined(ENABLE_SWITCH_FLOW_CONTROL) && ENABLE_SWITCH_FLOW_CONTROL
    *AMAZON_S_SW_RGMII_CTL |= 1 << 10;
#else
    *AMAZON_S_SW_RGMII_CTL &= ~(1 << 10);
#endif

    if ( g_ingress_direct_forward_enable )
        *AMAZON_S_SW_P1_CTL |= (1 << 19);   //  enable port ingress direct forwarding
    else
        *AMAZON_S_SW_P1_CTL &= ~(1 << 19);  //  disable port ingress direct forwarding
    *AMAZON_S_SW_P1_CTL &= ~(1 << 13);      //  disable port redirect
    *AMAZON_S_SW_P1_CTL |= 0x00000001;      //  the output packet keep the same format as receive
    if ( g_mii_port[1].phy_id == 0xFFFFFFFF )   //  hack for SuperVB board and for static configuration of MII1 (CONFIG_MII1_RGMII_MODE_100MB, CONFIG_MII1_RGMII_MODE)
    {
        *AMAZON_S_SW_P1_CTL &= ~(1 << 17);  //  disable force link down
        *AMAZON_S_SW_P1_CTL |= 1 << 18;     //  force link up
    }
}

static INLINE void init_mii2(void)
{
    *AMAZON_S_SW_P2_CTL = (*AMAZON_S_SW_P2_CTL & ~(1 << 18)) | (1 << 17);   //  force link down

    *AMAZON_S_SW_GCTL0 &= ~0x0300;          //  max packet length: 1152 bytes
//    *AMAZON_S_SW_PMAC_HD_CTL |= 0x00040000; //  enable CRC
    *AMAZON_S_SW_PMAC_HD_CTL |= (1 << 20);  //  remove CRC of packets going from PMAC to DMA
    *AMAZON_S_SW_P2_CTL |= 0x00000001;      //  the output packet keep the same format as receive

    *AMAZON_S_SW_GCTL0 |= 1 << 29;          //  enable logical port
    *AMAZON_S_SW_PMAC_HD_CTL |= 1 << 19;    //  enable status header (PMAC -> DMA)
    *AMAZON_S_SW_PMAC_HD_CTL |= 1 << 22;    //  enable status header (DMA -> PMAC)

    *AMAZON_S_SW_GCTL1 &= ~ ( (1 << 16) | (1 << 17) | (1 << 19));         //  identify IP in PPPoE and IPv6 in PPPoE

    *AMAZON_S_SW_P2_CTL = (*AMAZON_S_SW_P2_CTL & ~((3 << 30) | (1 << 17))) | (1 << 18) | (1 << 30); //  force link up, discard
    udelay(10);
}

static INLINE void start_mii2(void)
{
    *DS_RXCFG |= 0x00008000;    //  enable DSRX
    *DM_RXCFG |= 0x80000000;    //  enable DMRX

    *AMAZON_S_SW_P2_CTL = (*AMAZON_S_SW_P2_CTL & ~((3 << 30) | (3 << 17))) | (1 << 18); //  force link up, normal forwarding
}

static INLINE void stop_datapath(void)
{
    unsigned long org_jiffies;
    int i;

    g_stop_datapath = 1;    //  going to be unloaded

    //  disable switch external port
#ifdef CONFIG_MII1_AUTONEG
    disable_irq(MII1_GPHY_INT);
#endif
    *AMAZON_S_SW_P0_CTL = (*AMAZON_S_SW_P0_CTL & ~(1 << 18)) | (1 << 17);   //  force MII0 link down
    *AMAZON_S_SW_P1_CTL = (*AMAZON_S_SW_P1_CTL & ~(1 << 18)) | (1 << 17);   //  force MII1 link down

    //  turn on DMA RX channels
    turn_on_dma_rx(31);

    //  clean ingress datapath
    org_jiffies = jiffies;
    while ( ((IFX_REG_R32(DM_RXPGCNT) & 0x00FF) != 0 || (IFX_REG_R32(DM_RXPKTCNT) & 0x00FF) != 0) && jiffies - org_jiffies < HZ / 10 + 1 )
        schedule();
    if ( jiffies - org_jiffies > HZ / 10 )
        err("Can not clear DM_RXPGCNT/DM_RXPKTCNT");

    //  turn off DMA RX channels (including loopback RX channels)
    while ( (i = clz(g_f_dma_opened)) >= 0 )
        turn_off_dma_rx(i);

    //  turn off DMA Loopback TX channels
    g_dma_device->tx_chan[0]->close(g_dma_device->tx_chan[0]);
    g_dma_device->tx_chan[1]->close(g_dma_device->tx_chan[1]);

    //  free GPIO pins
    //    MDIO Bus (shared by MII0/Tantos and MII1 GPHY)
    IFX_SWITCH_PIN_FREE(SWITCH_MDIO);
    IFX_SWITCH_PIN_FREE(SWITCH_MDC);
    //    MII0
    if ( g_mii_port[0].mode == REV_MII_MODE || g_mii_port[0].mode == TURBO_REV_MII_MODE || g_mii_port[0].mode == MII_MODE )
    {
        IFX_SWITCH_PIN_FREE(MII0_COL);
        IFX_SWITCH_PIN_FREE(MII0_CRS);
        IFX_SWITCH_PIN_FREE(MII0_TXERR);
        IFX_SWITCH_PIN_FREE(MII0_RXERR);
    }
    //    MII1
#ifdef CONFIG_MII1_PHY_CLOCK_GPIO3
    IFX_SWITCH_PIN_FREE(CLOCK_OUT2);
#endif
    if ( g_mii_port[1].mode == REV_MII_MODE || g_mii_port[1].mode == TURBO_REV_MII_MODE || g_mii_port[1].mode == MII_MODE )
    {
        IFX_SWITCH_PIN_FREE(MII1_COL);
        IFX_SWITCH_PIN_FREE(MII1_CRS);
        IFX_SWITCH_PIN_FREE(MII1_TXERR);
        IFX_SWITCH_PIN_FREE(MII1_RXERR);
    }
#ifdef CONFIG_MII1_AUTONEG
  #if MII1_GPHY_INT_EXT_INT_NO == 0
    IFX_SWITCH_PIN_FREE(0);
  #elif MII1_GPHY_INT_EXT_INT_NO == 1
    IFX_SWITCH_PIN_FREE(1);
  #elif MII1_GPHY_INT_EXT_INT_NO == 2
    IFX_SWITCH_PIN_FREE(2);
  #elif MII1_GPHY_INT_EXT_INT_NO == 3
    IFX_SWITCH_PIN_FREE(39);
  #elif MII1_GPHY_INT_EXT_INT_NO == 4
    IFX_SWITCH_PIN_FREE(10);
  #elif MII1_GPHY_INT_EXT_INT_NO == 5
    IFX_SWITCH_PIN_FREE(9);
  #else
    #error ERROR EXT INT NO
  #endif
#endif
    //    MII2
    IFX_SWITCH_PIN_FREE(RESET_GPHY);
}

static INLINE void init_internal_tantos_qos_setting(void)
{
    /*
     *  Set internal switch configurations is,
     *  Classification: 802.1q & DSCP
     *  Scheduling for all internal ports: Strict Priority and Rate limit is Disabled
     */

    /*
     *  classification scheme:
     *      if packet is IP packet, then classification is based on DSCP.
     *      if packet is inot IP packet, then classification is based on 802.1q.
     */

    //  Enable VLAN, DSCP & IP Over VLAN priority on all ports (0/1/2)
    *AMAZON_S_SW_P0_CTL |= 7 << 25;
    *AMAZON_S_SW_P1_CTL |= 7 << 25;
    *AMAZON_S_SW_P2_CTL |= 7 << 25;

    /*
     *  Queue Mapping
     */

    *AMAZON_S_SW_1P_PRT = 0xFA41;

    *AMAZON_S_SW_DFSRV_MAP0 = 0x11100000;
    *AMAZON_S_SW_DFSRV_MAP1 = 0x22211111;
    *AMAZON_S_SW_DFSRV_MAP2 = 0x30222222;
    *AMAZON_S_SW_DFSRV_MAP3 = 0x00030003;

    /*
     *  Scheduling Scheme
     *      Strict Priority scheduling is used for all ports and Rate limit is disabled
     */

    *AMAZON_S_SW_P0_ECS_Q32 = 0x03E803E8;
    *AMAZON_S_SW_P0_ECS_Q10 = 0x03E803E8;
    *AMAZON_S_SW_P0_ECW_Q32 = 0x00000000;
    *AMAZON_S_SW_P0_ECW_Q10 = 0x00000000;
    *AMAZON_S_SW_P1_ECS_Q32 = 0x03E803E8;
    *AMAZON_S_SW_P1_ECS_Q10 = 0x03E803E8;
    *AMAZON_S_SW_P1_ECW_Q32 = 0x00000000;
    *AMAZON_S_SW_P1_ECW_Q10 = 0x00000000;
    *AMAZON_S_SW_P2_ECS_Q32 = 0x03E803E8;
    *AMAZON_S_SW_P2_ECS_Q10 = 0x03E803E8;
    *AMAZON_S_SW_P2_ECW_Q32 = 0x00000000;
    *AMAZON_S_SW_P2_ECW_Q10 = 0x00000000;
}

static INLINE void init_internal_tantos(void)
{
    //  forward all unknown unicast, unknown multicast, broadcast, reserve packet to CPU port only
    *AMAZON_S_SW_DF_PORTMAP = 0x04040404;

    //  turn on all virtual port as bridge destination
    *AMAZON_S_SW_P0_VLAN |= 0xFF << 16;
    *AMAZON_S_SW_P1_VLAN |= 0xFF << 16;
    *AMAZON_S_SW_P2_VLAN |= 0xFF << 16;

    init_internal_tantos_qos_setting();
}

static INLINE void init_dplus(void)
{
    *DM_RXCFG &= ~0x80000000;   //  disable DMRX
    *DS_RXCFG &= ~0x00008000;   //  disable DSRX
    *DM_RXPGCNT = 0x00020000;   //  clear page pointer & counter
    *DS_RXPGCNT = 0x00040000;   //  clear page pointer & counter

    *DM_RXDB    = 0x1450;
    *DM_RXCB    = 0x143C;
    *DM_RXPGCNT = 0x00020000;
    *DM_RXPKTCNT= 0x00000200;
    //HEADLEN: 12(Flag_header)+4(outervlan)+4(innervlan)+8(pppoe_header)+40(IPv6 header)+2(space)-8(switch header)=62
    *DM_RXCFG   = 0x00F87014;
}

static INLINE void init_ema(void)
{
    *EMA_CMDCFG  = (EMA_CMD_BUF_LEN << 16) | (EMA_CMD_BASE_ADDR >> 2);
    *EMA_DATACFG = (EMA_DATA_BUF_LEN << 16) | (EMA_DATA_BASE_ADDR >> 2);
    *EMA_IER     = 0x000000FF;
    *EMA_CFG     = EMA_READ_BURST | (EMA_WRITE_BURST << 2);
}

static INLINE void init_mailbox(void)
{
    *MBOX_IGU0_ISRC = 0xFFFFFFFF;
    *MBOX_IGU0_IER  = 0x00000000;
    *MBOX_IGU1_ISRC = 0xFFFFFFFF;
    *MBOX_IGU1_IER  = 0x00000000;   //  Don't need to enable RX interrupt, EMA driver handle RX path.
    dbg("MBOX_IGU1_IER = 0x%08X", *MBOX_IGU1_IER);
}

static INLINE void clear_share_buffer(void)
{
    volatile u32 *p = SB_RAM0_ADDR(0);
    unsigned int i;

    //  write all zeros only
    for ( i = 0; i < SB_RAM0_DWLEN + SB_RAM1_DWLEN + SB_RAM2_DWLEN + SB_RAM3_DWLEN + SB_RAM4_DWLEN; i++ )
        *p++ = 0;

    //  Configure share buffer master selection
    *SB_MST_PRI0 = 1;
    *SB_MST_PRI1 = 1;
}

static INLINE void clear_cdm(void)
{
    volatile u32 *dest;
    int i;

    //  Both blocks are set to code memory
    *CDM_CFG = CDM_CFG_RAM1_SET(0x01) | CDM_CFG_RAM0_SET(0x00);
    for ( i = 0; i < 1000; i++ );

    dest = CDM_CODE_MEMORY_RAM0_ADDR(0);
    for ( i = 0; i < CDM_CODE_MEMORY_RAM0_DWLEN + CDM_CODE_MEMORY_RAM1_DWLEN; i++ )
        *dest++ = 0;
}

static INLINE void board_init(void)
{
}

static INLINE void setup_ppe_conf(void)
{
    // enable PPE and MIPS access to DFE memory
    *AMAZON_S_RCU_PPE_CONF = *AMAZON_S_RCU_PPE_CONF | 0xC0000000;
}

static INLINE void restore_ppe_conf(void)
{
    // disable PPE and MIPS access to DFE memory
    *AMAZON_S_RCU_PPE_CONF = *AMAZON_S_RCU_PPE_CONF & ~0xC0000000;
}

static INLINE void init_hw(void)
{
    setup_ppe_conf();

    init_pmu();

    init_mdio();

    mii1_gphy_config_gpio();

    reset_gphy();

    probe_mii_ports();

    if ( g_mii_port[0].mode != 0xFF )
        init_mii0(g_mii_port[0].mode);

    if ( g_mii_port[1].mode != 0xFF )
        init_mii1(g_mii_port[1].mode);

    init_mii2();

    init_internal_tantos();

    init_dplus();

    init_ema();

    init_mailbox();

    clear_share_buffer();

    clear_cdm();

    board_init();
}

/*
 *  Description:
 *    Download PPE firmware binary code.
 *  Input:
 *    src       --- u32 *, binary code buffer
 *    dword_len --- unsigned int, binary code length in DWORD (32-bit)
 *  Output:
 *    int       --- 0:    Success
 *                  else: Error Code
 */
static INLINE int pp32_download_code(const u32 *code_src, unsigned int code_dword_len, const u32 *data_src, unsigned int data_dword_len)
{
    volatile u32 *dest;
    int i;

    if ( code_src == 0 || ((unsigned long)code_src & 0x03) != 0
        || data_src == 0 || ((unsigned long)data_src & 0x03) != 0
        || (code_dword_len > 0x1000 && data_dword_len > 0) )
        return -EINVAL;

    /*  set PPE code memory to FPI bus access mode  */
    if ( code_dword_len <= 0x1000 )
        *CDM_CFG = CDM_CFG_RAM1_SET(0x00) | CDM_CFG_RAM0_SET(0x00);
    else
        *CDM_CFG = CDM_CFG_RAM1_SET(0x01) | CDM_CFG_RAM0_SET(0x00);
    for ( i = 0; i < 1000; i++ );

    dbg("code_dword_len = 0x%X, data_dword_len = 0x%X", code_dword_len, data_dword_len);

    /*  copy code   */
    dest = CDM_CODE_MEMORY_RAM0_ADDR(0);
    while ( code_dword_len-- > 0 )
        *dest++ = *code_src++;

    /*  copy data   */
    dest = PP32_DATA_MEMORY_RAM1_ADDR(0);
    while ( data_dword_len-- > 0 )
        *dest++ = *data_src++;

    return 0;
}

/*
 *  Description:
 *    Do PP32 specific initialization.
 *  Input:
 *    data --- void *, specific parameter passed in.
 *  Output:
 *    int  --- 0:    Success
 *             else: Error Code
 */
static INLINE int pp32_specific_init(int fwcode, void *data)
{
    return 0;
}

/*
 *  Description:
 *    Initialize and start up PP32.
 *  Input:
 *    none
 *  Output:
 *    int  --- 0:    Success
 *             else: Error Code
 */
static INLINE int pp32_start(int fwcode)
{
    int ret;

    /*  download firmware   */
    ret = pp32_download_code(ar9_d5_fw_code, sizeof(ar9_d5_fw_code) / sizeof(*ar9_d5_fw_code), ar9_d5_fw_data, sizeof(ar9_d5_fw_data) / sizeof(*ar9_d5_fw_data));
    if ( ret )
        return ret;

    /*  firmware specific initialization    */
    ret = pp32_specific_init(fwcode, NULL);
    if ( ret )
        return ret;

    /*  run PP32    */
    *PP32_DBG_CTRL = DBG_CTRL_RESTART;
    /*  idle for a while to let PP32 init itself    */
    udelay(200);

    return 0;
}

/*
 *  Description:
 *    Halt PP32.
 *  Input:
 *    none
 *  Output:
 *    none
 */
static INLINE void pp32_stop(void)
{
    /*  halt PP32   */
    *PP32_DBG_CTRL = DBG_CTRL_STOP;
}

static INLINE int init_local_variables(void)
{
    int i, j;

    /*  initialize semaphore used by open and close */
    sema_init(&g_sem, 1);

    if ( ethwan == 1 )
        g_eth_wan_mode = 2;
    else if ( ethwan == 2 )
        g_eth_wan_mode = 3;

    if ( wanitf == ~0 || wanitf == 0)
    {
        switch ( g_eth_wan_mode )
        {
        case 2: /*  Mixed Mode  */  g_wan_itf = 1 << 0; break;
        case 3: /*  ETH WAN     */  g_wan_itf = 1 << 1; break;
        }
    }
    else
    {
        g_wan_itf = wanitf;
        switch ( g_eth_wan_mode )
        {
        case 2: /*  Mixed Mode  */
                g_wan_itf &= 0x03; //  only ETH0/1 support mixed mode
                if(g_wan_itf == 0x03 || g_wan_itf == 0){ //both ETH0/1 in mixed mode, reset to eth0 mixed mode
                    g_wan_itf = 1;
                }
                break;
        case 3: /*  ETH WAN     */
                g_wan_itf &= 0x7B; //  DSL disabled in ETH WAN mode, DSL port 7 is reserved in D5
                if((g_wan_itf & 0x03) == 0x03){ //both ETH0/1 in WAN mode, remove eth0 wan setting
                    g_wan_itf &= ~0x01;
                }
                if((g_wan_itf & 0x03) == 0){
                    g_wan_itf |= 1 << 1;
                }
                break;
        }
    }

    g_ipv6_acc_en = ipv6_acc_en ? 1 : 0;

#if defined(ENABLE_MIXED_QOS) && ENABLE_MIXED_QOS
    g_wanqos_en = wanqos_en;
#else
    g_wanqos_en = g_eth_wan_mode == 3 && wanqos_en ? wanqos_en : 0;
#endif

    for ( i = 0; i < 2; i++ )
    {
        int max_packet_priority = NUM_ENTITY(g_eth_prio_queue_map[i]);  //  assume only 8 levels are used in Linux
        int tx_num_q;
        int q_step, q_accum, p_step;

        tx_num_q = (g_wan_itf & (1 << i)) && g_wanqos_en ? __ETH_WAN_TX_QUEUE_NUM : __ETH_VIRTUAL_TX_QUEUE_NUM;
        q_step = tx_num_q - 1;
        p_step = max_packet_priority - 1;
        for ( j = 0, q_accum = 0; j < max_packet_priority; j++, q_accum += q_step )
            g_eth_prio_queue_map[i][j] = q_step - (q_accum + (p_step >> 1)) / p_step;
    }

    for ( i = 0; i < NUM_ENTITY(g_wan_queue_class_map); i++ )
        g_wan_queue_class_map[i] = (NUM_ENTITY(g_wan_queue_class_map) - 1 - i) % SWITCH_CLASS_NUM;

    return 0;
}

static INLINE void clear_local_variables(void)
{
}

#if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST

#include "cfg_arrays_d5.h"
#include "cfg_arrays_d5_ipv6.h"

unsigned int ppe_fw_mode        = 0; // 0: normal mode, 1: mix mode
unsigned int ppe_wan_hash_algo  = 0; // 0: using algo 0, 1: using algo 1
unsigned int acc_proto          = 0; // 0: UDP, 1:TCP
unsigned int ipv6_en            = 0; // 0: IPv6 disabled, 1: IPv6 enabled

void setup_acc_action_tbl(void)
{
    unsigned int idx;

    if (acc_proto == 0) {
        // clear bit16 of dw3 of each action entry
        unsigned long udp_mask = ~ (1 << 16);

        // setup for Hash entry
        idx = 3;
        if (ipv6_en == 0) {
            while (idx < sizeof(lan_uc_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
                lan_uc_act_tbl_normal_mode_cfg[idx] &= udp_mask;
                lan_uc_act_tbl_mixed_mode_cfg[idx] &= udp_mask;

                wan_uc_act_tbl_alo_0_cfg[idx] &= udp_mask;
                wan_uc_act_tbl_alo_1_cfg[idx] &= udp_mask;
                idx += 6;
            }
        } else {
            while (idx < sizeof(lan_uc_act_tbl_normal_mode_cfg_ipv6)/sizeof(unsigned long)) {
                lan_uc_act_tbl_normal_mode_cfg_ipv6[idx] &= udp_mask;
                lan_uc_act_tbl_mixed_mode_cfg_ipv6[idx] &= udp_mask;

                wan_uc_act_tbl_alo_0_cfg_ipv6[idx] &= udp_mask;
                wan_uc_act_tbl_alo_1_cfg_ipv6[idx] &= udp_mask;
                idx += 6;
            }
        }

        // setup for Collsion entry
        idx = 3;
        if (ipv6_en == 0) {
            while (idx < sizeof(lan_uc_col_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
                lan_uc_col_act_tbl_normal_mode_cfg[idx] &= udp_mask;
                lan_uc_col_act_tbl_mixed_mode_cfg[idx] &= udp_mask;
                wan_uc_col_act_tbl_cfg[idx] &= udp_mask;
                idx += 6;
            }
        } else {
            while (idx < sizeof(lan_uc_col_act_tbl_normal_mode_cfg_ipv6)/sizeof(unsigned long)) {
                lan_uc_col_act_tbl_normal_mode_cfg_ipv6[idx] &= udp_mask;
                lan_uc_col_act_tbl_mixed_mode_cfg_ipv6[idx] &= udp_mask;
                wan_uc_col_act_tbl_cfg_ipv6[idx] &= udp_mask;
                idx += 6;
            }
        }
    }
    else {
        // set bit16 of dw3 of each action entry
        unsigned long tcp_mask =  (1 << 16);

        // setup for Hash entry
        idx = 3;
        if (ipv6_en == 0) {
            while (idx < sizeof(lan_uc_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
                lan_uc_act_tbl_normal_mode_cfg[idx] |= tcp_mask;
                lan_uc_act_tbl_mixed_mode_cfg[idx] |= tcp_mask;

                wan_uc_act_tbl_alo_0_cfg[idx] |= tcp_mask;
                wan_uc_act_tbl_alo_1_cfg[idx] |= tcp_mask;
                idx += 6;
            }
        } else {
            while (idx < sizeof(lan_uc_act_tbl_normal_mode_cfg_ipv6)/sizeof(unsigned long)) {
                lan_uc_act_tbl_normal_mode_cfg_ipv6[idx] |= tcp_mask;
                lan_uc_act_tbl_mixed_mode_cfg_ipv6[idx] |= tcp_mask;

                wan_uc_act_tbl_alo_0_cfg_ipv6[idx] |= tcp_mask;
                wan_uc_act_tbl_alo_1_cfg_ipv6[idx] |= tcp_mask;
                idx += 6;
            }
        }

        // setup for Collsion entry
        idx = 3;
        if (ipv6_en == 0) {
            while (idx < sizeof(lan_uc_col_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
                lan_uc_col_act_tbl_normal_mode_cfg[idx] |= tcp_mask;
                lan_uc_col_act_tbl_mixed_mode_cfg[idx] |= tcp_mask;
                wan_uc_col_act_tbl_cfg[idx] |= tcp_mask;
                idx += 6;
            }
        } else {
            while (idx < sizeof(lan_uc_col_act_tbl_normal_mode_cfg_ipv6)/sizeof(unsigned long)) {
                lan_uc_col_act_tbl_normal_mode_cfg_ipv6[idx] |= tcp_mask;
                lan_uc_col_act_tbl_mixed_mode_cfg_ipv6[idx] |= tcp_mask;
                wan_uc_col_act_tbl_cfg_ipv6[idx] |= tcp_mask;
                idx += 6;
            }
        }
    }
}

void init_acc_tables(void)
{
    setup_acc_action_tbl();

    // init MAC table
    memcpy((void *)ROUT_MAC_CFG_TBL(0), rout_mac_cfg, sizeof(rout_mac_cfg));

    // PPPoE table
    memcpy((void *)PPPOE_CFG_TBL(0), pppoe_cfg, sizeof(pppoe_cfg));

    // Outer VLAN Config
    memcpy((void *)OUTER_VLAN_TBL(0), outer_vlan_cfg, sizeof(outer_vlan_cfg));

    if (ipv6_en == 0) {
        //Use Original Data Structures

        // lan uc_cmp_tbl_cfg (Hash) and collision
        memcpy((void *)ROUT_LAN_HASH_CMP_TBL(0), lan_uc_cmp_tbl_cfg, sizeof(lan_uc_cmp_tbl_cfg));
        memcpy((void *)ROUT_LAN_COLL_CMP_TBL(0), lan_uc_col_cmp_tbl_cfg, sizeof(lan_uc_col_cmp_tbl_cfg));

        // lan action
        if(ppe_fw_mode == 0) {
            // normal mode
            memcpy((void *)ROUT_LAN_HASH_ACT_TBL(0), lan_uc_act_tbl_normal_mode_cfg, sizeof(lan_uc_act_tbl_normal_mode_cfg));
            memcpy((void *)ROUT_LAN_COLL_ACT_TBL(0), lan_uc_col_act_tbl_normal_mode_cfg, sizeof(lan_uc_col_act_tbl_normal_mode_cfg));
        } else {
            // mix mode
            memcpy((void *)ROUT_LAN_HASH_ACT_TBL(0), lan_uc_act_tbl_mixed_mode_cfg, sizeof(lan_uc_act_tbl_mixed_mode_cfg));
            memcpy((void *)ROUT_LAN_COLL_ACT_TBL(0), lan_uc_col_act_tbl_mixed_mode_cfg, sizeof(lan_uc_col_act_tbl_mixed_mode_cfg));
        }

        // wan hash cmp anc act table
        if ( ppe_wan_hash_algo == 0) {
            // WAN algo 0
            memcpy((void *)ROUT_WAN_HASH_CMP_TBL(0), wan_uc_cmp_tbl_alo_0_cfg, sizeof(wan_uc_cmp_tbl_alo_0_cfg));
            memcpy((void *)ROUT_WAN_HASH_ACT_TBL(0), wan_uc_act_tbl_alo_0_cfg, sizeof(wan_uc_act_tbl_alo_0_cfg));
        } else {
            // WAN algo 1
            memcpy((void *)ROUT_WAN_HASH_CMP_TBL(0), wan_uc_cmp_tbl_alo_1_cfg, sizeof(wan_uc_cmp_tbl_alo_1_cfg));
            memcpy((void *)ROUT_WAN_HASH_ACT_TBL(0), wan_uc_act_tbl_alo_1_cfg, sizeof(wan_uc_act_tbl_alo_1_cfg));
        }

        // wan collision cmp and act table
        memcpy((void *)ROUT_WAN_COLL_CMP_TBL(0), wan_uc_col_cmp_tbl_cfg, sizeof(wan_uc_col_cmp_tbl_cfg));
        memcpy((void *)ROUT_WAN_COLL_ACT_TBL(0), wan_uc_col_act_tbl_cfg, sizeof(wan_uc_col_act_tbl_cfg));

        // wan multicast cmp and act table
        memcpy((void *)ROUT_WAN_MC_CMP_TBL(0), wan_mc_cmp_tbl_cfg, sizeof(wan_mc_cmp_tbl_cfg));
        memcpy((void *)ROUT_WAN_MC_ACT_TBL(0), wan_mc_act_tbl_cfg, sizeof(wan_mc_act_tbl_cfg));
    } else {
        //Use IPv6 Data Structures

        // lan uc_cmp_tbl_cfg (Hash) and collision
        memcpy((void *)ROUT_LAN_HASH_CMP_TBL(0), lan_uc_cmp_tbl_cfg_ipv6, sizeof(lan_uc_cmp_tbl_cfg_ipv6));
        memcpy((void *)ROUT_LAN_COLL_CMP_TBL(0), lan_uc_col_cmp_tbl_cfg_ipv6, sizeof(lan_uc_col_cmp_tbl_cfg_ipv6));

        // lan action
        if(ppe_fw_mode == 0) {
            // normal mode
            memcpy((void *)ROUT_LAN_HASH_ACT_TBL(0), lan_uc_act_tbl_normal_mode_cfg_ipv6, sizeof(lan_uc_act_tbl_normal_mode_cfg_ipv6));
            memcpy((void *)ROUT_LAN_COLL_ACT_TBL(0), lan_uc_col_act_tbl_normal_mode_cfg_ipv6, sizeof(lan_uc_col_act_tbl_normal_mode_cfg_ipv6));
        } else {
            // mix mode
            memcpy((void *)ROUT_LAN_HASH_ACT_TBL(0), lan_uc_act_tbl_mixed_mode_cfg_ipv6, sizeof(lan_uc_act_tbl_mixed_mode_cfg_ipv6));
            memcpy((void *)ROUT_LAN_COLL_ACT_TBL(0), lan_uc_col_act_tbl_mixed_mode_cfg_ipv6, sizeof(lan_uc_col_act_tbl_mixed_mode_cfg_ipv6));
        }

        // wan hash cmp anc act table
        if ( ppe_wan_hash_algo == 0) {
            // WAN algo 0
            memcpy((void *)ROUT_WAN_HASH_CMP_TBL(0), wan_uc_cmp_tbl_alo_0_cfg_ipv6, sizeof(wan_uc_cmp_tbl_alo_0_cfg_ipv6));
            memcpy((void *)ROUT_WAN_HASH_ACT_TBL(0), wan_uc_act_tbl_alo_0_cfg_ipv6, sizeof(wan_uc_act_tbl_alo_0_cfg_ipv6));
        } else {
            // WAN algo 1
            memcpy((void *)ROUT_WAN_HASH_CMP_TBL(0), wan_uc_cmp_tbl_alo_1_cfg_ipv6, sizeof(wan_uc_cmp_tbl_alo_1_cfg_ipv6));
            memcpy((void *)ROUT_WAN_HASH_ACT_TBL(0), wan_uc_act_tbl_alo_1_cfg_ipv6, sizeof(wan_uc_act_tbl_alo_1_cfg_ipv6));
        }

        // wan collision cmp and act table
        memcpy((void *)ROUT_WAN_COLL_CMP_TBL(0), wan_uc_col_cmp_tbl_cfg_ipv6, sizeof(wan_uc_col_cmp_tbl_cfg_ipv6));
        memcpy((void *)ROUT_WAN_COLL_ACT_TBL(0), wan_uc_col_act_tbl_cfg_ipv6, sizeof(wan_uc_col_act_tbl_cfg_ipv6));

        // wan multicast cmp and act table
        memcpy((void *)ROUT_WAN_MC_CMP_TBL(0), wan_mc_cmp_tbl_cfg_ipv6, sizeof(wan_mc_cmp_tbl_cfg_ipv6));
        memcpy((void *)ROUT_WAN_MC_ACT_TBL(0), wan_mc_act_tbl_cfg_ipv6, sizeof(wan_mc_act_tbl_cfg_ipv6));
    }
}

#endif  //  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST

static INLINE void init_communication_data_structures(int fwcode)
{
    struct eth_ports_cfg eth_ports_cfg      = {0};
    struct rout_tbl_cfg lan_rout_tbl_cfg    = {0};
    struct rout_tbl_cfg wan_rout_tbl_cfg    = {0};
    struct gen_mode_cfg1 gen_mode_cfg1      = {0};
    struct ps_mc_cfg ps_mc_gencfg3          = {0};
    int i, j;

    *CDM_CFG = CDM_CFG_RAM1_SET(0x00) | CDM_CFG_RAM0_SET(0x00);

    for ( i = 0; i < 1000; i++ );

    *PSEUDO_IPv4_BASE_ADDR = 0xFFFFFF00;

    eth_ports_cfg.wan_vlanid_hi = 0;
    eth_ports_cfg.wan_vlanid_lo = 0;
    eth_ports_cfg.eth0_type = 0;    //  lan
    *ETH_PORTS_CFG = eth_ports_cfg;

    lan_rout_tbl_cfg.rout_num   = MAX_COLLISION_ROUTING_ENTRIES;
    lan_rout_tbl_cfg.ttl_en     = 1;
    lan_rout_tbl_cfg.rout_drop  = 0;
    *LAN_ROUT_TBL_CFG = lan_rout_tbl_cfg;

    wan_rout_tbl_cfg.rout_num           = MAX_COLLISION_ROUTING_ENTRIES;
    wan_rout_tbl_cfg.wan_rout_mc_num    = 64;
    wan_rout_tbl_cfg.ttl_en             = 1;
    wan_rout_tbl_cfg.wan_rout_mc_drop   = 0;
    wan_rout_tbl_cfg.rout_drop          = 0;
    *WAN_ROUT_TBL_CFG = wan_rout_tbl_cfg;

    gen_mode_cfg1.app2_indirect         = 1;   //  0: means DMA RX ch 3 is dedicated for CPU1 process
    gen_mode_cfg1.us_indirect           = 0;
    gen_mode_cfg1.us_early_discard_en   = 0;
#if defined(CONFIG_IFX_PPA_MFE) && CONFIG_IFX_PPA_MFE
    gen_mode_cfg1.classification_num    = MAX_CLASSIFICATION_ENTRIES;
#else
    gen_mode_cfg1.classification_num    = 0;
#endif
    gen_mode_cfg1.ipv6_rout_num         = g_ipv6_acc_en ? 2 : 0;
    gen_mode_cfg1.session_ctl_en        = 0;
    gen_mode_cfg1.wan_hash_alg          = 0;
#if defined(CONFIG_IFX_PPA_MFE) && CONFIG_IFX_PPA_MFE
    gen_mode_cfg1.brg_class_en          = 1;
#else
    gen_mode_cfg1.brg_class_en          = 0;
#endif
    gen_mode_cfg1.ipv4_mc_acc_mode      = 0;
    gen_mode_cfg1.ipv6_acc_en           = g_ipv6_acc_en;
    gen_mode_cfg1.wan_acc_en            = 1;
    gen_mode_cfg1.lan_acc_en            = 1;
    gen_mode_cfg1.sw_iso_mode           = 1;
    gen_mode_cfg1.sys_cfg               = g_eth_wan_mode;   //  3 - ETH1 WAN, 2 - ETH0 mixed
                                                            // default - ETH WAN Separate, Ethernet WAN Mode.
                                                            // Ethernet 0 carry LAN traffic, Ethernet 1 carry WAN traffic.
    *GEN_MODE_CFG1 = gen_mode_cfg1;
    GEN_MODE_CFG2->itf_outer_vlan_vld   = 0xFF; //  Turn on outer VLAN for all interfaces;

    ps_mc_gencfg3.ssc_mode = 1;
    ps_mc_gencfg3.asc_mode = 0;
    ps_mc_gencfg3.psave_en = 0; //default disable clock gating power save function
    *PS_MC_GENCFG3         = ps_mc_gencfg3;

    *CFG_CLASS2QID_MAP(0) = 0;
    *CFG_CLASS2QID_MAP(1) = 0;
    *CFG_QID2CLASS_MAP(0) = 0;
    *CFG_QID2CLASS_MAP(1) = 0;
    i = (g_wan_itf & 0x01) ? 0 : 1;
    for ( j = 0; j < NUM_ENTITY(g_eth_prio_queue_map[i]); j++ )
    {
        *CFG_CLASS2QID_MAP(j >> 3) |= g_eth_prio_queue_map[i][j] << ((j & 0x07) * 4);
        *CFG_CLASS2QID_MAP((j + 8) >> 3) |= (g_eth_prio_queue_map[i][j] | 8) << ((j & 0x07) * 4);
    }
    for ( j = 0; j < NUM_ENTITY(g_wan_queue_class_map); j++ )
        *CFG_QID2CLASS_MAP(j >> 3) |= g_wan_queue_class_map[j] << ((j & 0x07) * 4);

    //*MTU_CFG_TBL(0) = DMA_PACKET_SIZE;          //  for ATM
    *MTU_CFG_TBL(0) = ETH_MAX_DATA_LENGTH;
    for ( i = 1; i < 8; i++ )
        *MTU_CFG_TBL(i) = ETH_MAX_DATA_LENGTH;  //  for ETH

    if ( g_wanqos_en )
    {
        struct wtx_qos_q_desc_cfg wtx_qos_q_desc_cfg = {0};
        struct tx_qos_cfg tx_qos_cfg = {0};
        struct cfg_std_data_len cfg_std_data_len = {0};

        for ( i = 0; i < __ETH_WAN_TX_QUEUE_NUM; i++ )
        {
            wtx_qos_q_desc_cfg.length = ETH_WAN_TX_DESC_NUM(i);
            wtx_qos_q_desc_cfg.addr   = __ETH_WAN_TX_DESC_BASE(i);
            *WTX_QOS_Q_DESC_CFG(i) = wtx_qos_q_desc_cfg;
        }

        tx_qos_cfg.time_tick    = cgu_get_pp32_clock() / 62500; //  16 * (cgu_get_pp32_clock() / 1000000)
        tx_qos_cfg.eth1_eg_qnum = __ETH_WAN_TX_QUEUE_NUM;
        tx_qos_cfg.eth1_qss     = 1;
        tx_qos_cfg.eth1_burst_chk = 1;
        *TX_QOS_CFG = tx_qos_cfg;

        cfg_std_data_len.byte_off = 0;
        cfg_std_data_len.data_len = DMA_PACKET_SIZE;

        *CFG_STD_DATA_LEN = cfg_std_data_len;
    }

    *CFG_WAN_PORTMAP    = g_wan_itf;
    *CFG_MIXED_PORTMAP  = g_eth_wan_mode == 2 /* Mixed Mode */ ? g_wan_itf : 0;
    *TUNNEL_MAX_ID = -1;
#if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST
    init_acc_tables();
#endif
}

static INLINE int alloc_dma(void)
{
    int ret;
    unsigned int pre_alloc_desc_total_num;
    struct sk_buff **skb_pool;
    struct sk_buff **pskb;
    volatile u32 *p;
    int i;

    pre_alloc_desc_total_num = DMA_RX_CH1_DESC_NUM + DMA_RX_CH2_DESC_NUM;
    if ( g_wanqos_en )
        pre_alloc_desc_total_num += CPU_TO_WAN_TX_DESC_NUM + WAN_TX_DESC_NUM_TOTAL + CPU_TO_WAN_SWAP_DESC_NUM + DMA_TX_CH1_DESC_NUM;

    skb_pool = (struct sk_buff **)kmalloc(pre_alloc_desc_total_num * sizeof(*skb_pool), GFP_KERNEL);
    if ( skb_pool == NULL )
    {
        ret = -ENOMEM;
        goto ALLOC_SKB_POOL_FAIL;
    }

    for ( i = 0; i < pre_alloc_desc_total_num; i++ )
    {
        skb_pool[i] = alloc_skb_rx();
        if ( !skb_pool[i] )
        {
            ret = -ENOMEM;
            goto ALLOC_SKB_FAIL;
        }
#ifndef CONFIG_MIPS_UNCACHED
        /*  invalidate cache    */
        dma_cache_inv((unsigned long)skb_pool[i]->data, DMA_PACKET_SIZE);
#endif
#if defined(DEBUG_SKB_SWAP) && DEBUG_SKB_SWAP
        g_dbg_skb_swap_pool[i] = skb_pool[i];
#endif
    }

    g_dma_device = dma_device_reserve("PPE");
    if ( !g_dma_device )
    {
        ret = -EIO;
        goto RESERVE_DMA_FAIL;
    }

    g_dma_device->buffer_alloc    = dma_buffer_alloc;
    g_dma_device->buffer_free     = dma_buffer_free;
#if defined(ENABLE_NAPI) && ENABLE_NAPI
    g_dma_device->activate_poll   = g_napi_enable ? dma_activate_poll : NULL;
    g_dma_device->inactivate_poll = dma_inactivate_poll;
#endif
    g_dma_device->intr_handler    = dma_int_handler;
    g_dma_device->max_rx_chan_num = 4;
    g_dma_device->max_tx_chan_num = 4;
    g_dma_device->tx_burst_len    = 8;
    g_dma_device->rx_burst_len    = 8;
    *AMAZON_S_SW_PMAC_RX_IPG = (*AMAZON_S_SW_PMAC_RX_IPG & ~0xF0) | 0x0130; //  increase RX FIFO Request Watermark, 0x01?0 means maximum watermark

    for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
    {
        g_dma_device->rx_chan[i]->packet_size = DMA_PACKET_SIZE;
        g_dma_device->rx_chan[i]->control     = IFX_DMA_CH_ON;
        //g_dma_device->rx_chan[i]->channel_packet_drop_enable    = 1;
    }
#if !defined(DEBUG_REDIRECT_FASTPATH_TO_CPU) || !DEBUG_REDIRECT_FASTPATH_TO_CPU
    g_dma_device->rx_chan[1]->desc_base = (int)DMA_RX_CH1_DESC_BASE;
    g_dma_device->rx_chan[1]->desc_len  = DMA_RX_CH1_DESC_NUM;
    g_dma_device->rx_chan[2]->desc_base = (int)DMA_RX_CH2_DESC_BASE;
    g_dma_device->rx_chan[2]->desc_len  = DMA_RX_CH2_DESC_NUM;
    g_dma_device->rx_chan[1]->req_irq_to_free   = g_dma_device->rx_chan[1]->irq;
    g_dma_device->rx_chan[2]->req_irq_to_free   = g_dma_device->rx_chan[2]->irq;
#endif


    for ( i = 0; i < g_dma_device->max_tx_chan_num; i++ )
    {
        g_dma_device->tx_chan[i]->control     = IFX_DMA_CH_ON;
    }
#if !defined(DEBUG_REDIRECT_FASTPATH_TO_CPU) || !DEBUG_REDIRECT_FASTPATH_TO_CPU
    g_dma_device->tx_chan[0]->desc_base = (int)DMA_TX_CH0_DESC_BASE;
    g_dma_device->tx_chan[0]->desc_len  = DMA_TX_CH0_DESC_NUM;
    g_dma_device->tx_chan[1]->desc_base = (int)DMA_TX_CH1_DESC_BASE;
    g_dma_device->tx_chan[1]->desc_len  = DMA_TX_CH1_DESC_NUM;
    g_dma_device->tx_chan[0]->global_buffer_len = DMA_PACKET_SIZE;
    g_dma_device->tx_chan[0]->peri_to_peri      = 1;
    g_dma_device->tx_chan[1]->global_buffer_len = DMA_PACKET_SIZE;
    g_dma_device->tx_chan[1]->peri_to_peri      = 1;
#endif

    g_dma_device->port_packet_drop_enable = 1;

    if ( dma_device_register(g_dma_device) != IFX_SUCCESS )
    {
        err("failed in \"dma_device_register\"");
        ret = -EIO;
        goto DMA_DEVICE_REGISTER_FAIL;
    }

    pskb = skb_pool;

    if ( g_wanqos_en )
    {
        p = (volatile u32 *)CPU_TO_WAN_TX_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_TX_DESC_NUM; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "CPU to WAN TX data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = 0x30000000;
            *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
        }

        p = (volatile u32 *)ETH_WAN_TX_DESC_BASE(0);
        for ( i = 0; i < WAN_TX_DESC_NUM_TOTAL; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "WAN (ETH1) QoS TX data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = 0x30000000;
            *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
        }

        p = (volatile u32 *)CPU_TO_WAN_SWAP_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_SWAP_DESC_NUM; i++ )
        {
            *p++ = 0xB0000000;
            *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
        }
    }

#if !defined(DEBUG_REDIRECT_FASTPATH_TO_CPU) || !DEBUG_REDIRECT_FASTPATH_TO_CPU
    p = (volatile u32 *)DMA_RX_CH1_DESC_BASE;
    for ( i = 0; i < DMA_RX_CH1_DESC_NUM; i++ )
    {
        ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "DMA RX channel 1 data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
        *p++ = 0x80000000 | DMA_PACKET_SIZE;
        *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
    }

    p = (volatile u32 *)DMA_RX_CH2_DESC_BASE;
    for ( i = 0; i < DMA_RX_CH2_DESC_NUM; i++ )
    {
        ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "DMA RX channel 2 data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
        *p++ = 0xB0000000 | DMA_PACKET_SIZE;
        *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
    }
#endif

    if ( g_wanqos_en )
    {
        p = (volatile u32 *)DMA_TX_CH1_DESC_BASE;
        for ( i = 0; i < DMA_TX_CH1_DESC_NUM; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "DMA TX channel 1 data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = 0xB0000000;
            *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
        }
    }

    g_f_dma_opened = 0;

    kfree(skb_pool);

    return 0;

DMA_DEVICE_REGISTER_FAIL:
    dma_device_release(g_dma_device);
    g_dma_device = NULL;
RESERVE_DMA_FAIL:
    i = pre_alloc_desc_total_num;
ALLOC_SKB_FAIL:
    while ( i-- )
        dev_kfree_skb_any(skb_pool[i]);
    kfree(skb_pool);
ALLOC_SKB_POOL_FAIL:
    return ret;
}

static INLINE void free_dma(void)
{
    volatile struct tx_descriptor *p;
    struct sk_buff *skb_to_free;
    int i;

    if ( g_wanqos_en )
    {
        p = (volatile struct tx_descriptor *)CPU_TO_WAN_TX_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_TX_DESC_NUM; i++ )
        {
            skb_to_free = get_skb_pointer(p->dataptr);
            dev_kfree_skb_any(skb_to_free);
            p++;
        }

        p = (volatile struct tx_descriptor *)ETH_WAN_TX_DESC_BASE(0);
        for ( i = 0; i < WAN_TX_DESC_NUM_TOTAL; i++ )
        {
            skb_to_free = get_skb_pointer(p->dataptr);
            dev_kfree_skb_any(skb_to_free);
            p++;
        }

        p = (volatile struct tx_descriptor *)CPU_TO_WAN_SWAP_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_SWAP_DESC_NUM; i++ )
        {
            skb_to_free = get_skb_pointer(p->dataptr);
            dev_kfree_skb_any(skb_to_free);
            p++;
        }
    }



#if !defined(DEBUG_REDIRECT_FASTPATH_TO_CPU) || !DEBUG_REDIRECT_FASTPATH_TO_CPU
    p = (volatile struct tx_descriptor *)DMA_RX_CH1_DESC_BASE;
    for ( i = 0; i < DMA_RX_CH1_DESC_NUM; i++ )
    {
        skb_to_free = get_skb_pointer(p->dataptr);
        dev_kfree_skb_any(skb_to_free);
        p++;
    }

    p = (volatile struct tx_descriptor *)DMA_RX_CH2_DESC_BASE;
    for ( i = 0; i < DMA_RX_CH2_DESC_NUM; i++ )
    {
        skb_to_free = get_skb_pointer(p->dataptr);
        dev_kfree_skb_any(skb_to_free);
        p++;
    }
#endif

    if ( g_wanqos_en )
    {
        p = (volatile struct tx_descriptor *)DMA_TX_CH1_DESC_BASE;
        for ( i = 0; i < DMA_TX_CH1_DESC_NUM; i++ )
        {
            skb_to_free = get_skb_pointer(p->dataptr);
            dev_kfree_skb_any(skb_to_free);
            p++;
        }
    }

    dma_device_unregister(g_dma_device);
    dma_device_release(g_dma_device);
    g_dma_device = NULL;

#if defined(DEBUG_SKB_SWAP) && DEBUG_SKB_SWAP
    for ( i = 0; i < NUM_ENTITY(g_dbg_skb_swap_pool); i++ )
        if ( g_dbg_skb_swap_pool[i] != NULL )
        {
            err("skb swap pool is not clean: %d - %08x\n", i, (unsigned int)g_dbg_skb_swap_pool[i]);
        }
#endif
}

#ifdef CONFIG_IFX_PPA_DATAPATH
static INLINE void ethaddr_setup(unsigned int port, char *line)
{
    u8 *p;
    char *ep;
    int i;

    p = MY_ETH0_ADDR + (port ? MAX_ADDR_LEN : 0);
    memset(p, 0, MAX_ADDR_LEN * sizeof(*p));
    for ( i = 0; i < 6; i++ )
    {
        p[i] = line ? simple_strtoul(line, &ep, 16) : 0;
        if ( line )
            line = *ep ? ep + 1 : ep;
    }
    dbg("eth%d mac address %02X-%02X-%02X-%02X-%02X-%02X",
        port ? 1 : 0,
        (u32)p[0] & 0xFF, (u32)p[1] & 0xFF, (u32)p[2] & 0xFF,
        (u32)p[3] & 0xFF, (u32)p[4] & 0xFF, (u32)p[5] & 0xFF);
}
#endif

#if defined(ENABLE_MY_MEMCPY) && ENABLE_MY_MEMCPY
static INLINE void *my_memcpy(unsigned char *dest, const unsigned char *src, unsigned int count)
{
    char *d = (char *)dest, *s = (char *)src;

    if (count >= 32) {
        int i = 8 - (((unsigned long) d) & 0x7);

        if (i != 8)
            while (i-- && count--) {
                *d++ = *s++;
            }

        if (((((unsigned long) d) & 0x7) == 0) &&
                ((((unsigned long) s) & 0x7) == 0)) {
            while (count >= 32) {
                unsigned long long t1, t2, t3, t4;
                t1 = *(unsigned long long *) (s);
                t2 = *(unsigned long long *) (s + 8);
                t3 = *(unsigned long long *) (s + 16);
                t4 = *(unsigned long long *) (s + 24);
                *(unsigned long long *) (d) = t1;
                *(unsigned long long *) (d + 8) = t2;
                *(unsigned long long *) (d + 16) = t3;
                *(unsigned long long *) (d + 24) = t4;
                d += 32;
                s += 32;
                count -= 32;
            }
            while (count >= 8) {
                *(unsigned long long *) d =
                                            *(unsigned long long *) s;
                d += 8;
                s += 8;
                count -= 8;
            }
        }

        if (((((unsigned long) d) & 0x3) == 0) &&
                ((((unsigned long) s) & 0x3) == 0)) {
            while (count >= 4) {
                *(unsigned long *) d = *(unsigned long *) s;
                d += 4;
                s += 4;
                count -= 4;
            }
        }

        if (((((unsigned long) d) & 0x1) == 0) &&
                ((((unsigned long) s) & 0x1) == 0)) {
            while (count >= 2) {
                *(unsigned short *) d = *(unsigned short *) s;
                d += 2;
                s += 2;
                count -= 2;
            }
        }
    }

    while (count--) {
        *d++ = *s++;
    }

    return d;
}
#endif

static int print_fw_ver(char *buf, int buf_len)
{
    static char * fw_ver_family_str[] = {
        "reserved - 0",
        "Danube",
        "Twinpass",
        "Amazon-SE",
        "reserved - 4",
        "AR9",
        "GR9",
        "VR9",
        "AR10"
    };

    static char * fw_ver_package_str[] = {
        "Reserved - 0",
		"A1",
		"B1 - PTM_BONDING",
		"E1",
		"A5",
		"D5",
		"D5v2",
		"E5",
    };

    static char *fw_feature[] = {
		"  ATM/PTM TC-Layer                Support",
		"  ATM/PTM TC-Layer Retransmission Support",
		"  ATM/PTM TC-Layer Bonding        Support",
		"  L2 Trunking                     Support",
		"  Packet Acceleration             Support",
		"  IPv4                            Support",
		"  IPv6                            Support",
		"  6RD                             Support",
		"  DS-Lite                         Support",
		"  Unified FW QoS                  Support",

	};


    int len = 0;
    int i;

    len += snprintf(buf + len, buf_len - len, "PPE firmware info:\n");
    len += snprintf(buf + len, buf_len - len,     "  Version ID: %d.%d.%d.%d.%d\n", (int)FW_VER_ID->family, (int)FW_VER_ID->package, (int)FW_VER_ID->major, (int)FW_VER_ID->middle, (int)FW_VER_ID->minor);
    if ( FW_VER_ID->family >= NUM_ENTITY(fw_ver_family_str) )
        len += snprintf(buf + len, buf_len - len, "  Family    : reserved - %d\n", (int)FW_VER_ID->family);
    else
        len += snprintf(buf + len, buf_len - len, "  Family    : %s\n", fw_ver_family_str[FW_VER_ID->family]);

    if ( FW_VER_ID->package >= NUM_ENTITY(fw_ver_package_str) )
        len += snprintf(buf + len, buf_len - len, "  FW Package: reserved - %d\n", (int)FW_VER_ID->package);
    else
        len += snprintf(buf + len, buf_len - len, "  FW Package: %s\n", fw_ver_package_str[FW_VER_ID->package]);

    len += snprintf(buf + len, buf_len - len,     "  Release   : %u.%u.%u\n", (int)FW_VER_ID->major, (int)FW_VER_ID->middle, (int)FW_VER_ID->minor);

    len += snprintf(buf + len, buf_len - len, "PPE firmware feature:\n");

    for(i = 0; i < NUM_ENTITY(fw_feature); i ++){
        if(*FW_VER_FEATURE & (1 << (31-i))){
            len += snprintf(buf + len, buf_len - len, "%s\n", fw_feature[i]);
        }
    }

    return len;

}

static int print_driver_ver(char *buf, int buf_len, char *title, unsigned int family, unsigned int type, unsigned int itf, unsigned int mode, unsigned int major, unsigned int mid, unsigned int minor)
{
    static char * dr_ver_family_str[] = {
        NULL,
        "Danube",
        "Twinpass",
        "Amazon-SE",
        NULL,
        "AR9",
        "VR9",
        "AR10",
        NULL,
    };
    static char * dr_ver_type_str[] = {
        "Normal Data Path",
        "Indirect-Fast Path",
        "HAL",
        "Hook",
        "OS Adatpion Layer",
        "PPA API"
    };
    static char * dr_ver_interface_str[] = {
        "MII0",
        "MII1",
        "ATM",
        "PTM"
    };
    static char * dr_ver_accmode_str[] = {
        "Routing",
        "Bridging",
    };

    int len = 0;
    unsigned int bit;
    int i, j;

    len += snprintf(buf + len, buf_len - len, "%s:\n", title);
    len += snprintf(buf + len, buf_len - len, "  Version ID: %d.%d.%d.%d.%d.%d.%d\n", family, type, itf, mode, major, mid, minor);
    len += snprintf(buf + len, buf_len - len, "  Family    : ");
    for ( bit = family, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_family_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_family_str[i] != NULL )
        {
            if ( j )
                len += snprintf(buf + len, buf_len - len, " | %s", dr_ver_family_str[i]);
            else
                len += snprintf(buf + len, buf_len - len, dr_ver_family_str[i]);
            j++;
        }
    if ( j )
        len += snprintf(buf + len, buf_len - len, "\n");
    else
        len += snprintf(buf + len, buf_len - len, "N/A\n");
    len += snprintf(buf + len, buf_len - len, "  DR Type   : ");
    for ( bit = type, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_type_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_type_str[i] != NULL )
        {
            if ( j )
                len += snprintf(buf + len, buf_len - len, " | %s", dr_ver_type_str[i]);
            else
                len += snprintf(buf + len, buf_len - len, dr_ver_type_str[i]);
            j++;
        }
    if ( j )
        len += snprintf(buf + len, buf_len - len, "\n");
    else
        len += snprintf(buf + len, buf_len - len, "N/A\n");
    len += snprintf(buf + len, buf_len - len, "  Interface : ");
    for ( bit = itf, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_interface_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_interface_str[i] != NULL )
        {
            if ( j )
                len += snprintf(buf + len, buf_len - len, " | %s", dr_ver_interface_str[i]);
            else
                len += snprintf(buf + len, buf_len - len, dr_ver_interface_str[i]);
            j++;
        }
    if ( j )
        len += snprintf(buf + len, buf_len - len, "\n");
    else
        len += snprintf(buf + len, buf_len - len, "N/A\n");
    len += snprintf(buf + len, buf_len - len, "  Mode      : ");
    for ( bit = mode, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_accmode_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_accmode_str[i] != NULL )
        {
            if ( j )
                len += snprintf(buf + len, buf_len - len, " | %s", dr_ver_accmode_str[i]);
            else
                len += snprintf(buf + len, buf_len - len, dr_ver_accmode_str[i]);
            j++;
        }
    if ( j )
        len += snprintf(buf + len, buf_len - len, "\n");
    else
        len += snprintf(buf + len, buf_len - len, "N/A\n");
    len += snprintf(buf + len, buf_len - len, "  Release   : %d.%d.%d\n", major, mid, minor);

    return len;
}


static struct proc_dir_entry *ppa_get_proc_dir(struct proc_entry_cfg *proc_entry, char *base_dir)
{
    int i;

    for(i = 0; proc_entry[i].name != NULL; i ++){
        if(proc_entry[i].is_dir && strcmp(proc_entry[i].name,base_dir) == 0){
            return proc_entry[i].proc_dir;
        }
    }

    return NULL;
}

static void ppa_proc_reg(struct proc_entry_cfg *proc_entry)
{
    int i;
    struct proc_dir_entry *proc_dir = NULL;
    struct proc_dir_entry *res = NULL;

    for(i = 0; proc_entry[i].name != NULL; i ++){
        if(!proc_entry[i].is_enable)
            continue;

        if(proc_entry[i].parent_dir != NULL){
            printk("searching parent_dir\n");
            proc_dir = ppa_get_proc_dir(proc_entry, proc_entry[i].parent_dir);
        }

        if(proc_entry[i].is_dir){//create folder
            proc_entry[i].proc_dir = proc_mkdir(proc_entry[i].name, proc_dir);
            if(!proc_entry[i].proc_dir){
                printk("Create proc dir [%s] failed \n", proc_entry[i].name);
                return;
            }
            proc_dir = proc_entry[i].proc_dir;
        }
        else{//create file
            if(!proc_entry[i].proc_w_fn){//create readonly entry
                create_proc_read_entry(proc_entry[i].name,
                           0,
                           proc_dir,
                           proc_entry[i].proc_r_fn,
                           NULL);
            }
            else{
                res  = create_proc_entry(proc_entry[i].name,0,proc_dir);
                if(res){
                    res->read_proc  = proc_entry[i].proc_r_fn;
                    res->write_proc = proc_entry[i].proc_w_fn;
                }
            }
            proc_entry[i].proc_dir = proc_dir;
        }
    }
}

static INLINE void proc_file_create(void)
{
    /*if no parent folder name provided, register function will take the nearest proc_dir_entry by default.
          if no proc_dir_entry found, by default, it's NULL. (e.g. create folder eth )
          Please make sure the file follow the right dir entry otherwise it would cause trouble to release it
      */
     static struct proc_entry_cfg ppa_proc_entry[] = {
     // Parent dir,    Name,    IS_DIR,   RD_fn,     WR_fn,     Enable
        {NULL,  "eth",      1, NULL,                        NULL,                 1 },
        {NULL,  "ver",      0, proc_read_ver,               NULL,                 1 },
        {NULL,  "mib",      0, proc_read_mib,               proc_write_mib,       1 },
        {NULL,  "route",    0, proc_read_route,             proc_write_route,     1 },
        {NULL,  "mc",       0, proc_read_mc,                NULL,                 1 },
        {NULL,  "genconf",  0, proc_read_genconf,            proc_write_genconf,  1 },
        {NULL,  "pppoe",    0, proc_read_pppoe,             NULL,                 1 },
        {NULL,  "mtu",      0, proc_read_mtu,               NULL,                 1 },
        {NULL,  "hit",      0, proc_read_hit,               proc_write_hit,       1 },
        {NULL,  "mac",      0, proc_read_mac,               NULL,                 1 },
        {NULL,  "out_vlan", 0, proc_read_out_vlan,          NULL,                 1 },
        {NULL,  "ipv6_ip",  0, proc_read_ipv6_ip,           NULL,                 1 },
        {NULL,  "dbg",      0, proc_read_dbg,               proc_write_dbg,       1 },
        {NULL,  "mem",      0, NULL,                        proc_write_mem,       1 },
        {NULL,  "pp32",     0, proc_read_pp32,              proc_write_pp32,      1 },
        {NULL,  "burstlen", 0, proc_read_burstlen,          proc_write_burstlen,  1 },
        {NULL,  "fw",       0, proc_read_fw,                proc_write_fw,        1 },
        {NULL,  "tfwdbg",   0, NULL,                        proc_write_fwdbg,     1 },
        {NULL,  "portmib",  0, physical_port_counters_proc_read,    physical_port_counters_proc_write, 1 },
        {NULL,  "port0mib", 0, port0_counters_proc_read,    port0_counters_proc_write, 1 },
        {NULL,  "port1mib", 0, port1_counters_proc_read,    port1_counters_proc_write, 1 },
        {NULL,  "port2mib", 0, port2_counters_proc_read,    port2_counters_proc_write, 1 },
        {NULL,  "mactable", 0, proc_read_mactable,          proc_write_mactable,       1 },
        {NULL,  "direct_forwarding", 0,  proc_read_directforwarding,  proc_write_directforwarding, 1 },
        {NULL,  "clk",      0, proc_read_clk,               proc_write_clk,       1 },
        {NULL,"flowcontrol",0, proc_read_flowcontrol,       proc_write_flowcontrol, 1 },
        {NULL,  "gphy",     0, proc_read_gphy,              proc_write_gphy,        1 },
#if defined(ENABLE_NAPI) && ENABLE_NAPI
        {NULL,  "napi",     0, proc_read_napi,              proc_write_napi,      1 },
#endif
        {NULL,  "prio",     0, proc_read_prio,              proc_write_prio,      1 },
        {NULL,  "qos",      0, proc_read_qos,               NULL,                 1 },
        {NULL,  "class",    0, proc_read_class,             proc_write_class,     1 },
        {NULL,  "mccfg",    0, proc_read_mccfg,             proc_write_mccfg,     1 },
        {NULL,  NULL,       0, NULL,                        NULL,                 1 },

    };
    int i;

    //Enable/Disable proc
    for(i = 0; ppa_proc_entry[i].name != NULL; i ++){
        if(!g_wanqos_en && strcmp(ppa_proc_entry[i].name, "qos") == 0){
            ppa_proc_entry[i].is_enable = 0;
        }
        else if(!g_ipv6_acc_en && strcmp(ppa_proc_entry[i].name, "ipv6_ip") == 0){
            ppa_proc_entry[i].is_enable = 0;
        }
    }

    g_eth_proc_entry = ppa_proc_entry;

    ppa_proc_reg(g_eth_proc_entry);
}

static INLINE void proc_file_delete(void)
{
    int i = 0;

    if(!g_eth_proc_entry)
        return;

    for(i = 0; g_eth_proc_entry[i].name != NULL; i ++);

    for(i = i - 1; i >= 0; i --){
        if(!g_eth_proc_entry[i].is_dir){
            remove_proc_entry(g_eth_proc_entry[i].name, g_eth_proc_entry[i].proc_dir);
        }else{
            remove_proc_entry(g_eth_proc_entry[i].name, NULL);
        }
    }
}

static int proc_read_ver(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += print_driver_ver(page + len, count - len, "PPE datapath driver info", VER_FAMILY, VER_DRTYPE, VER_INTERFACE, VER_ACCMODE, VER_MAJOR, VER_MID, VER_MINOR);
    len += print_fw_ver(page + len, count - len);

    *eof = 1;

    return len;
}

static int proc_read_mib(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    struct eth_priv_data *priv0, *priv1;
    char *itf_name[] = {"eth0", "eth1", "cpu", "ext_int1", "ext_int2", "ext_int3", "ext_int4", "ptm0"};
    char *row_name[] = {"ig_fast_brg_pkts", "ig_fast_brg_bytes", "ig_fast_rt_ipv4_udp_pkts",
                        "ig_fast_rt_ipv4_tcp_pkts", "ig_fast_rt_ipv4_mc_pkts", "ig_fast_rt_ipv4_bytes",
                        "ig_fast_rt_ipv6_udp_pkts", "ig_fast_rt_ipv6_tcp_pkts", "ig_fast_rt_ipv6_mc_pkts",
                        "ig_fast_rt_ipv6_bytes", NULL, "ig_cpu_pkts", "ig_cpu_bytes", "ig_drop_pkts",
                        "ig_drop_bytes", "eg_fast_pkts"};
    int i, j, k, h;

    eth_get_stats(g_eth_net_dev[0]);
    eth_get_stats(g_eth_net_dev[1]);
    priv0 = ifx_eth_fw_netdev_priv(g_eth_net_dev[0]);
    priv1 = ifx_eth_fw_netdev_priv(g_eth_net_dev[1]);

    len += sprintf(page + len, "Ethernet\n");

    len += sprintf(page + len, "  Firmware  (");
    for ( i = h = 0; i < NUM_ENTITY(itf_name); i++ )
        if ( (g_mib_itf & (1 << i)) )
            len += sprintf(page + len, h++ == 0 ? "%s" : ", %s", itf_name[i]);
    len += sprintf(page + len, ")\n");
    for ( i = k = 0; i < NUM_ENTITY(row_name); i++ )
        if ( row_name[i] != NULL && k < (j = strlen(row_name[i])) )
            k = j;
    k += 6;
    for ( i = 0; i < NUM_ENTITY(row_name); i++ )
    {
        if ( row_name[i] == NULL )
            continue;

        len += (j = sprintf(page + len, "    %s: ", row_name[i]));
        for ( j = k - j; j > 0; j-- )
            page[len++] = ' ';
        for ( j = h = 0; j < 8; j++ )
            if ( (g_mib_itf & (1 << j)) )
                len += sprintf(page + len, h++ == 0 ? "%10u" : ", %10u", ((volatile unsigned int *)ITF_MIB_TBL(j))[i]);
        len += sprintf(page + len, "\n");
    }

    len += sprintf(page + len, "  Driver  (eth0, eth1)\n");
    len += sprintf(page + len, "    rx_packets: %10u, %10u\n", priv0->rx_packets,  priv1->rx_packets);
    len += sprintf(page + len, "    rx_bytes:   %10u, %10u\n", priv0->rx_bytes,    priv1->rx_bytes);
    len += sprintf(page + len, "    rx_errors:  %10u, %10u\n", priv0->rx_errors,   priv1->rx_errors);
    len += sprintf(page + len, "    rx_dropped: %10u, %10u\n", priv0->rx_dropped,  priv1->rx_dropped);
    len += sprintf(page + len, "    tx_packets: %10u, %10u\n", priv0->tx_packets,  priv1->tx_packets);
    len += sprintf(page + len, "    tx_bytes:   %10u, %10u\n", priv0->tx_bytes,    priv1->tx_bytes);
    len += sprintf(page + len, "    tx_errors:  %10u, %10u\n", priv0->tx_errors,   priv1->tx_errors);
    len += sprintf(page + len, "    tx_dropped: %10u, %10u\n", priv0->tx_dropped,  priv1->tx_dropped);

    *eof = 1;

    ASSERT(len <= 4096, "proc read buffer overflow");
    return len;
}

static void clear_mib_counter(unsigned int mib_itf )
{
    struct eth_priv_data *priv;
    int i;
    if ( mib_itf == 0x80000000 )
        mib_itf = 0x800000FF;
    if ( (mib_itf & 0x01) )
    {
        eth_get_stats(g_eth_net_dev[0]);
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[0]);
        memset(priv, 0, (u32)&priv->tx_dropped - (u32)priv + sizeof(u32));
    }
    if ( (mib_itf & 0x02) )
    {
        eth_get_stats(g_eth_net_dev[1]);
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[1]);
        memset(priv, 0, (u32)&priv->tx_dropped - (u32)priv + sizeof(u32));
    }
    for ( i = 0; i < 8; i++ )
	{
        if ( (mib_itf & (1 << i)) )
        {
            void *ptmp = (void *)ITF_MIB_TBL(i);
            memset(ptmp, 0, sizeof(struct itf_mib));
        }
	}
    if ( (mib_itf & g_wan_itf) )
    {
        for ( i = 0; i < 8; i++ )
        {
            void *ptmp = (void *)ETH_WAN_TX_MIB_TABLE(i);
            memset(ptmp, 0, sizeof(struct eth_wan_mib_table));
        }
    }

}


static int proc_write_mib(struct file *file, const char *buf, unsigned long count, void *data)
{
    char *p1, *p2;
    int len;
    int colon;
    char local_buf[1024];
    unsigned int mib_itf = 0;
    char *itf_name[] = {"eth0", "eth1", "cpu", "ext_int1", "ext_int2", "ext_int3", "ext_int4", "ext_int5"};
    int i;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    p2 = NULL;
    colon = 1;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "on") == 0 || stricmp(p1, "enable") == 0 )
            mib_itf = 0x40000000;
        else if ( stricmp(p1, "off") == 0 || stricmp(p1, "disable") == 0 )
            mib_itf = 0x20000000;
        else if ( stricmp(p1, "clear") == 0 || stricmp(p1, "clean") == 0 )
            mib_itf = 0x80000000;
        else if ( (mib_itf & 0xE0000000) )
        {
            if ( stricmp(p1, "all") == 0 )
                mib_itf |= 0xFF;
            else if ( strlen(p1) == 1 && *p1 >= '0' && *p1 <= '7' )
                mib_itf |= 1 << (*p1 - '0');
            else
            {
                for ( i = 0; i < NUM_ENTITY(itf_name); i++ )
                    if ( stricmp(p1, itf_name[i]) == 0 )
                    {
                        mib_itf |= 1 << i;
                        break;
                    }
            }
        }

        p1 = p2;
        colon = 1;
    }

    if ( (mib_itf & 0x40000000) )
        g_mib_itf |= mib_itf & 0xFF;
    else if ( (mib_itf & 0x20000000) )
        g_mib_itf &= ~(mib_itf & 0xFF);
    else if ( (mib_itf & 0x80000000) )
    {        
        clear_mib_counter(mib_itf);	
    }
    else
        printk("echo <on|off|clear> <all|eth0|eth1|cpu|ext_int?> > /proc/eth/mib\n");

    return count;
}

#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC

static int proc_read_route(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    struct rout_forward_compare_tbl *pcompare;
    struct rout_forward_action_tbl *paction;
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;

    pstr = *start = page;

    __sync();

    llen = sprintf(str, "Wan Routing Table\n");
    proc_buf_copy(&pstr, count, off, &len, str, llen);

    pcompare = (struct rout_forward_compare_tbl *)ROUT_WAN_HASH_CMP_TBL(0);
    paction = (struct rout_forward_action_tbl *)ROUT_WAN_HASH_ACT_TBL(0);
    for ( i = 0; i < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK; i++ )
    {
        if ( *(u32*)pcompare && *((u32*)pcompare + 1) )
        {
            llen = print_route(str, i, 0, pcompare, paction);
            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_ROUTE_OVERRUN_END;
        }

        pcompare++;
        paction++;
    }

    pcompare = (struct rout_forward_compare_tbl *)ROUT_WAN_COLL_CMP_TBL(0);
    paction = (struct rout_forward_action_tbl *)ROUT_WAN_COLL_ACT_TBL(0);
    for ( i = 0; i < WAN_ROUT_TBL_CFG->rout_num; i++ )
    {
        if ( *(u32*)pcompare && *((u32*)pcompare + 1) )
        {
            llen = print_route(str, i + MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK, 0, pcompare, paction);
            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_ROUTE_OVERRUN_END;
        }

        pcompare++;
        paction++;
    }

    llen = sprintf(str, "Lan Routing Table\n");
    if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
        goto PROC_READ_ROUTE_OVERRUN_END;

    pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_HASH_CMP_TBL(0);
    paction = (struct rout_forward_action_tbl *)ROUT_LAN_HASH_ACT_TBL(0);
    for ( i = 0; i < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK; i++ )
    {
        if ( *(u32*)pcompare && *((u32*)pcompare + 1) )
        {
            llen = print_route(str, i, 1, pcompare, paction);
            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_ROUTE_OVERRUN_END;
        }

        pcompare++;
        paction++;
    }

    pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_COLL_CMP_TBL(0);
    paction = (struct rout_forward_action_tbl *)ROUT_LAN_COLL_ACT_TBL(0);
    for ( i = 0; i < LAN_ROUT_TBL_CFG->rout_num; i++ )
    {
        if ( *(u32*)pcompare && *((u32*)pcompare + 1) )
        {
            llen = print_route(str, i + MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK, 1, pcompare, paction);
            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_ROUTE_OVERRUN_END;
        }

        pcompare++;
        paction++;
    }

    *eof = 1;

PROC_READ_ROUTE_OVERRUN_END:
    return len - off;
}

static int proc_write_route(struct file *file, const char *buf, unsigned long count, void *data)
{
    static const char *command[] = {
        "add",      //  0
        "del",      //  1
        "LAN",      //  2
        "WAN",      //  3
        "new",      //  4
        "src",      //  5
        "dest",     //  6
        "MAC",      //  7
        "route",    //  8
        "type",     //  9
        "DSCP",     //  10
        "MTU",      //  11
        "index",    //  12
        "VLAN",     //  13
        "insert",   //  14
        "remove",   //  15
        "list",     //  16
        "PPPoE",    //  17
        "mode",     //  18
        "ch",       //  19
        "id",       //  20
        "delete",   //  21
        "disable",  //  22
        "enable",   //  23
        "transparent",  //  24
        "termination",  //  25
        "NULL",     //  26
        "IPv4",     //  27
        "NAT",      //  28
        "NAPT",     //  29
        "entry",    //  30
        "tcp",      //  31
        "help",     //  32
        "vci",      //  33
        "yes",      //  34
        "no",       //  35
        "qid",      //  36
        "outer",    //  37
        "IP",       //  38
        "IPv6",     //  39
    };

    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "EXT_INT4", "EXT_INT5"};
    static const int dest_list_strlen[] = {4, 4, 4, 8, 8, 8, 8, 8};

    int state;              //  1: new,
    int prev_cmd;
    int operation;          //  1: add, 2: delete
    int type;               //  1: LAN, 2: WAN, 0: auto detect
    int entry;
    struct rout_forward_compare_tbl compare_tbl;
    struct rout_forward_action_tbl action_tbl;
    uint32_t ipv6_src_ip[4] = {0}, ipv6_dst_ip[4] = {0};
    int is_ipv6 = 0;
    unsigned int val[20];
    char local_buf[1024];
    int len;
    char *p1, *p2;
    int colon;
    int i, j;
    u32 mask;
    u32 bit;
    u32 *pu1, *pu2, *pu3;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    state = 0;
    prev_cmd = 0;
    operation = 0;
    type = 0;
    entry = -1;

    memset(&compare_tbl, 0, sizeof(compare_tbl));
    memset(&action_tbl, 0, sizeof(action_tbl));

    p1 = local_buf;
    colon = 1;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        for ( i = 0; i < sizeof(command) / sizeof(*command); i++ )
            if ( stricmp(p1, command[i]) == 0 )
            {
                switch ( i )
                {
                case 0:
                    if ( !state && !operation )
                    {
                        operation = 1;
//                      printk("add\n");
                    }
                    state = prev_cmd = 0;
                    break;
                case 1:
                case 21:
                    if ( !state && !operation )
                    {
                        operation = 2;
//                      printk("delete\n");
                    }
                    state = prev_cmd = 0;
                    break;
                case 2:
                    if ( !state && !type )
                    {
                        type = 1;
//                      printk("lan\n");
                    }
                    state = prev_cmd = 0;
                    break;
                case 3:
                    if ( !state && !type )
                    {
                        type = 2;
//                      printk("wan\n");
                    }
                    state = prev_cmd = 0;
                    break;
                case 4:
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 4;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 5:
                    if ( state == 1 )
                    {
                        if ( prev_cmd == 4 )
                        {
                            //  new src
                            if ( !type )
                                type = 1;

                            //  check for "new src mac index"
                            ignore_space(&p2, &len);
                            if ( strincmp(p2, "mac ", 4) == 0 )
                            {
                                state = 2;
                                prev_cmd = 5;
                                break;
                            }
                            else
                            {
                                get_ip_port(&p2, &len, val);
//                              printk("new src: %d.%d.%d.%d:%d\n", val[0], val[1], val[2], val[3], val[4]);
                                action_tbl.new_ip = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                                action_tbl.new_port = val[4];
                            }
                        }
                        else
                            state = 0;
                    }
                    if ( state == 0 )
                    {
                        //  src
                        get_ip_port(&p2, &len, val);
//                      printk("src: %d.%d.%d.%d:%d\n", val[0], val[1], val[2], val[3], val[4]);
                        if ( val[5] == 4 )
                        {
                            if ( is_ipv6 )
                                ipv6_src_ip[0] = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                            else
                                compare_tbl.src_ip = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                        }
                        else
                        {
                            is_ipv6 = 1;
                            ipv6_src_ip[0] = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                            ipv6_src_ip[1] = (val[6] << 24) | (val[7] << 16) | (val[8] << 8) | val[9];
                            ipv6_src_ip[2] = (val[10] << 24) | (val[11] << 16) | (val[12] << 8) | val[13];
                            ipv6_src_ip[3] = (val[14] << 24) | (val[15] << 16) | (val[16] << 8) | val[17];
                        }
                        compare_tbl.src_port = val[4];
                    }
                    state = prev_cmd = 0;
                    break;
                case 6:
                    if ( state == 1 )
                    {
                        if ( prev_cmd == 4 )
                        {
                            //  new dest
                            if ( !type )
                                type = 2;

                            get_ip_port(&p2, &len, val);
//                          printk("new dest: %d.%d.%d.%d:%d\n", val[0], val[1], val[2], val[3], val[4]);
                            action_tbl.new_ip = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                            action_tbl.new_port = val[4];
                        }
                        else
                            state = 0;
                    }
                    if ( state == 0 )
                    {
                        if ( !colon )
                        {
                            int llen;

                            llen = len;
                            p1 = p2;
                            while ( llen && *p1 <= ' ' )
                            {
                                llen--;
                                p1++;
                            }
                            if ( llen && (*p1 == ':' || (*p1 >= '0' && *p1 <= '9')) )
                                colon = 1;
                        }
                        if ( colon )
                        {
                            //  dest
                            get_ip_port(&p2, &len, val);
//                          printk("dest: %d.%d.%d.%d:%d\n", val[0], val[1], val[2], val[3], val[4]);
                            if ( val[5] == 4 )
                            {
                                if ( is_ipv6 )
                                    ipv6_dst_ip[0] = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                                else
                                    compare_tbl.dest_ip = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                            }
                            else
                            {
                                is_ipv6 = 1;
                                ipv6_dst_ip[0] = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
                                ipv6_dst_ip[1] = (val[6] << 24) | (val[7] << 16) | (val[8] << 8) | val[9];
                                ipv6_dst_ip[2] = (val[10] << 24) | (val[11] << 16) | (val[12] << 8) | val[13];
                                ipv6_dst_ip[3] = (val[14] << 24) | (val[15] << 16) | (val[16] << 8) | val[17];
                            }
                            compare_tbl.dest_port = val[4];
                        }
                        else
                        {
                            state = 1;
                            prev_cmd = 6;
                            break;
                        }
                    }
                    state = prev_cmd = 0;
                    break;
                case 7:
                    if ( state == 1 && prev_cmd == 4 )
                    {
                        //  new MAC
                        get_mac(&p2, &len, val);
//                      printk("new MAC: %02X.%02X.%02X.%02X:%02X:%02X\n", val[0], val[1], val[2], val[3], val[4], val[5]);
                        action_tbl.new_dest_mac54 = (val[0] << 8) | val[1];
                        action_tbl.new_dest_mac30 = (val[2] << 24) | (val[3] << 16) | (val[4] << 8) | val[5];
                    }
                    else if ( state == 2 && prev_cmd == 5 )
                    {
                        state = 3;
                        prev_cmd = 7;
                        break;
                    }
                    state = prev_cmd = 0;
                    break;
                case 8:
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 8;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 9:
                    if ( state == 1 && prev_cmd == 8 )
                    {
                        state = 2;
                        prev_cmd = 9;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 10:
                    if ( state == 1 && prev_cmd == 4 )
                    {
                        ignore_space(&p2, &len);
                        if ( len && *p2 >= '0' && *p2 <= '9' )
                        {
                            //  new DSCP
                            val[0] = get_number(&p2, &len, 0);
//                          printk("new DSCP: %d\n", val[0]);
                            if ( !action_tbl.new_dscp_en )
                            {
                                action_tbl.new_dscp_en = 1;
                                action_tbl.new_dscp = val[0];
                            }
                        }
                        else if ( (len == 8 || (len > 8 && (p2[8] <= ' ' || p2[8] == ','))) && strincmp(p2, "original", 8) == 0 )
                        {
                            p2 += 8;
                            len -= 8;
                            //  new DSCP original
//                          printk("new DSCP: original\n");
                            //  the reset value is 0, so don't need to do anything
                        }
                    }
                    state = prev_cmd = 0;
                    break;
                case 11:
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 11;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 12:
                    if ( state == 1 )
                    {
                        if ( prev_cmd == 11 )
                        {
                            //  MTU index
                            ignore_space(&p2, &len);
                            val[0] = get_number(&p2, &len, 0);
//                          printk("MTU index: %d\n", val[0]);
                            action_tbl.mtu_ix = val[0];
                        }
                        else if ( prev_cmd == 13 )
                        {
                            //  VLAN insert enable
                            //  VLAN index
                            ignore_space(&p2, &len);
                            val[0] = get_number(&p2, &len, 0);
//                          printk("VLAN insert: enable, index %d\n", val[0]);
                            if ( !action_tbl.in_vlan_ins )
                            {
                                action_tbl.in_vlan_ins = 1;
                                action_tbl.new_in_vci = val[0];
                            }
                        }
                        else if ( prev_cmd == 17 )
                        {
                            //  PPPoE index
                            ignore_space(&p2, &len);
                            val[0] = get_number(&p2, &len, 0);
//                          printk("PPPoE index: %d\n", val[0]);
                            action_tbl.pppoe_ix = val[0];
                        }
                    }
                    else if ( state == 3 && prev_cmd == 7 )
                    {
                        //  new src mac index
                        ignore_space(&p2, &len);
                        val[0] = get_number(&p2, &len, 0);
                        action_tbl.new_src_mac_ix = val[0];
                    }
                    else if ( state == 2 && prev_cmd == 13 )
                    {
                        //  outer VLAN enable
                        //  outer VLAN index
                        ignore_space(&p2, &len);
                        val[0] = get_number(&p2, &len, 0);
//                      printk("outer VLAN insert: enable, index %d\n", val[0]);
                        if ( !action_tbl.out_vlan_ins )
                        {
                            action_tbl.out_vlan_ins = 1;
                            action_tbl.out_vlan_ix = val[0];
                        }
                    }
                    state = prev_cmd = 0;
                    break;
                case 13:
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 13;
                    }
                    else if ( state == 1 && prev_cmd == 37 )
                    {
                        state = 2;
                        prev_cmd = 13;
                        printk("outer vlan\n");
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 14:
                    if ( (state == 1 || state == 2) && prev_cmd == 13 )
                    {
                        state++;
                        prev_cmd = 14;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 15:
                    if ( (state == 1 || state == 2) && prev_cmd == 13 )
                    {
                        state++;
                        prev_cmd = 15;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 16:
                    if ( state == 1 && prev_cmd == 6 )
                    {
                        mask = 0;
                        do
                        {
                            ignore_space(&p2, &len);
                            if ( !len )
                                break;
                            for ( j = 0, bit = 1; j < sizeof(dest_list) / sizeof(*dest_list); j++, bit <<= 1 )
                                if ( (len == dest_list_strlen[j] || (len > dest_list_strlen[j] && (p2[dest_list_strlen[j]] <= ' ' || p2[dest_list_strlen[j]] == ','))) && strincmp(p2, dest_list[j], dest_list_strlen[j]) == 0 )
                                {
                                    p2 += dest_list_strlen[j];
                                    len -= dest_list_strlen[j];
                                    mask |= bit;
                                    break;
                                }
                        } while ( j < sizeof(dest_list) / sizeof(*dest_list) );
//                      if ( mask )
//                      {
//                          //  dest list
//                          printk("dest list:");
//                          for ( j = 0, bit = 1; j < sizeof(dest_list) / sizeof(*dest_list); j++, bit <<= 1 )
//                              if ( (mask & bit) )
//                              {
//                                  printk(" %s", dest_list[j]);
//                              }
//                          printk("\n");
//                      }
//                      else
//                          printk("dest list: none\n");
                        action_tbl.dest_list = mask;
                    }
                    state = prev_cmd = 0;
                    break;
                case 17:
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 17;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 18:
                    if ( state == 1 && prev_cmd == 17 )
                    {
                        state = 2;
                        prev_cmd = 18;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 19:
                    if ( state == 1 && prev_cmd == 6 )
                    {
                        state = 2;
                        prev_cmd = 19;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 20:
                    if ( state == 2 && prev_cmd == 19 )
                    {
                        //  dest ch id
                        ignore_space(&p2, &len);
                        val[0] = get_number(&p2, &len, 0);
//                      printk("dest ch id: %d\n", val[0]);
                        //action_tbl.dest_chid = val[0];
                    }
                    state = prev_cmd = 0;
                    break;
                case 22:
                case 23:
                    if ( state == 2 )
                    {
                        if ( prev_cmd == 14 )
                        {
                            //  VLAN insert
//                          printk("VLAN insert: %s (%d)", command[i], i - 22);
                            if ( (i - 22) )
                            {
                                ignore_space(&p2, &len);
                                if ( len > 5 && (p2[5] <= ' ' || p2[5] == ':') && strincmp(p2, "index", 5) == 0 )
                                {
                                    p2 += 6;
                                    len -= 6;
                                    //  VLAN index
                                    ignore_space(&p2, &len);
                                    val[0] = get_number(&p2, &len, 0);
//                                  printk(", index %d", val[0]);
                                    if ( !action_tbl.in_vlan_ins )
                                    {
                                        action_tbl.in_vlan_ins = 1;
                                        //action_tbl.vlan_ix = val[0];
                                        action_tbl.new_in_vci = val[0];
                                    }
                                }
                                else if ( len > 3 && (p2[3] <= ' ' || p2[3] == ':') && strincmp(p2, "vci", 3) == 0 )
                                {
                                    p2 += 4;
                                    len -= 4;
                                    //  New VCI
                                    ignore_space(&p2, &len);
                                    val[0] = get_number(&p2, &len, 1);
//                                  printk(", vci 0x%04X", val[0]);
                                    if ( !action_tbl.in_vlan_ins )
                                    {
                                        action_tbl.in_vlan_ins = 1;
                                        action_tbl.new_in_vci = val[0];
                                    }
                                }
                            }
                            else
                            {
                                action_tbl.in_vlan_ins = 0;
                                action_tbl.new_in_vci = 0;
                            }
//                          printk("\n");
                        }
                        else if ( prev_cmd == 15 )
                        {
                            //  VLAN remove
//                          printk("VLAN remove: %s (%d)\n", command[i], i - 22);
                            action_tbl.in_vlan_rm = i - 22;
                        }
                    }
                    else if ( state == 3 )
                    {
                        if ( prev_cmd == 14 )
                        {
                            //  outer vlan insert
//                          printk("outer VLAN insert: %s (%d)", command[i], i - 22);
                            if ( (i - 22) )
                            {
                                ignore_space(&p2, &len);
                                if ( len > 5 && (p2[5] <= ' ' || p2[5] == ':') && strincmp(p2, "index", 5) == 0 )
                                {
                                    p2 += 6;
                                    len -= 6;
                                    //  outer VLAN index
                                    ignore_space(&p2, &len);
                                    val[0] = get_number(&p2, &len, 0);
//                                  printk(", index %d", val[0]);
                                    if ( !action_tbl.out_vlan_ins )
                                    {
                                        action_tbl.out_vlan_ins = 1;
                                        action_tbl.out_vlan_ix = val[0];
                                    }
                                }
                            }
                            else
                            {
                                action_tbl.out_vlan_ins = 0;
                                action_tbl.out_vlan_ix = 0;
                            }
//                          printk("\n");
                        }
                        else if ( prev_cmd == 15 )
                        {
                            //  outer VLAN remove
//                          printk("outer VLAN remove: %s (%d)\n", command[i], i - 22);
                            action_tbl.out_vlan_rm = i - 22;
                        }
                    }
                    state = prev_cmd = 0;
                    break;
                case 24:
                case 25:
                    if ( state == 2 && prev_cmd == 18 )
                    {
                        //  PPPoE mode
//                      printk("PPPoE mode: %s (%d)\n", command[i], i - 24);
                        action_tbl.pppoe_mode = i - 24;
                    }
                    state = prev_cmd = 0;
                    break;
                case 38:
                case 39:
                    i = 27;
                case 26:
                case 27:
                case 28:
                case 29:
                    if ( state == 2 && prev_cmd == 9 )
                    {
                        //  route type
//                      printk("route type: %s (%d)\n", command[i], i - 26);
                        action_tbl.rout_type = i - 26;
                    }
                    state = prev_cmd = 0;
                    break;
                case 30:
                    if ( !state )
                    {
                        if ( entry < 0 )
                        {
                            ignore_space(&p2, &len);
                            if ( len && *p2 >= '0' && *p2 <= '9' )
                            {
                                entry = get_number(&p2, &len, 0);
                                //  entry
//                              printk("entry: %d\n", entry);
                            }
                        }
                    }
                    state = prev_cmd = 0;
                    break;
                case 31:  // if this flag is not presented, it's UDP by default
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 31;
                        action_tbl.protocol = 1;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                case 32:
                    printk("add\n");
                    printk("  LAN/WAN entry ???\n");
                    printk("    compare\n");
                    printk("      src:  ???.???.???.???:????\n");
                    printk("      dest: ???.???.???.???:????\n");
                    printk("    action\n");
                    printk("      new src/dest:      ???.???.???.???:????\n");
                    printk("      new MAC:           ??:??:??:??:??:?? (HEX)\n");
                    printk("      route type:        NULL/IP/NAT/NAPT\n");
                    printk("      new DSCP:          original/??\n");
                    printk("      MTU index:         ??\n");
                    printk("      VLAN insert:       disable/enable, VCI ???? (HEX)\n");
                    printk("      VLAN remove:       disable/enable\n");
                    printk("      dest list:         ETH0/ETH1/CPU0/EXT_INT?\n");
                    printk("      PPPoE mode:        transparent/termination\n");
                    printk("      PPPoE index:       ??\n");
                    printk("      new src MAC index: ??\n");
                    printk("      outer VLAN insert: disable/enable, index ??\n");
                    printk("      outer VLAN remove: disable/enable\n");
                    printk("      tcp:               yes/no\n");
                    printk("      dest qid:          ??\n");
                    printk("\n");
                    printk("delete\n");
                    printk("  LAN/WAN entry ???\n");
                    printk("    compare\n");
                    printk("      src:  ???.???.???.???:????\n");
                    printk("      dest: ???.???.???.???:????\n");
                    printk("\n");
                    state = prev_cmd = 0;
                    break;
                case 33:
                    if ( state == 1 && prev_cmd == 13 )
                    {
                        //  vlan vci
                        ignore_space(&p2, &len);
                        val[0] = get_number(&p2, &len, 1);
                        if ( !action_tbl.in_vlan_ins )
                        {
                            action_tbl.in_vlan_ins = 1;
                            action_tbl.new_in_vci = val[0];
                        }
                    }
                    state = prev_cmd = 0;
                    break;
                case 34:
                    if ( state == 1 && prev_cmd == 31 )
                        //  tcp yes
                        action_tbl.protocol = 1;
                    state = prev_cmd = 0;
                    break;
                case 35:
                    if ( state == 1 && prev_cmd == 31 )
                        //  tcp no
                        action_tbl.protocol = 0;
                    state = prev_cmd = 0;
                    break;
                case 36:
                    if ( state == 1 && prev_cmd == 6 )
                    {
                        //  dest qid
                        ignore_space(&p2, &len);
                        val[0] = get_number(&p2, &len, 0);
                        action_tbl.dest_qid = val[0];
                    }
                    state = prev_cmd = 0;
                    break;
                case 37:
                    if ( !state )
                    {
                        state = 1;
                        prev_cmd = 37;
                    }
                    else
                        state = prev_cmd = 0;
                    break;
                default:
                    state = prev_cmd = 0;
                }

                break;
            }

        if ( i == sizeof(command) / sizeof(*command) )
            state = prev_cmd = 0;

        p1 = p2;
        colon = 1;
    }

    if ( operation == 2 )
    {
        u32 is_lan = 0;

        //  delete
        pu1 = (u32*)&compare_tbl;
        pu2 = NULL;
        pu3 = NULL;
        if ( entry < 0 )
        {
            //  search the entry number
            if ( *pu1 && pu1[1] )
            {
                if ( (!type || type == 1) )
                {
                    //  LAN
                    for ( entry = 0; entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK; entry++ )
                    {
                        pu2 = (u32*)ROUT_LAN_HASH_CMP_TBL(entry);
                        if ( *pu2 == *pu1 && pu2[1] == pu1[1] && pu2[2] == pu1[2] )
                        {
                            pu3 = (u32*)ROUT_LAN_HASH_ACT_TBL(entry);
                            break;
                        }
                    }
                    if ( entry == MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK )
                    {
                        for ( entry = 0; entry < LAN_ROUT_TBL_CFG->rout_num; entry++ )
                        {
                            pu2 = (u32*)ROUT_LAN_COLL_CMP_TBL(entry);
                            if ( *pu2 == *pu1 && pu2[1] == pu1[1] && pu2[2] == pu1[2] )
                            {
                                pu3 = (u32*)ROUT_LAN_COLL_ACT_TBL(entry);
                                break;
                            }
                        }
                        if ( entry == LAN_ROUT_TBL_CFG->rout_num )
                            pu2 = NULL;
                        else
                            entry += MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK;
                    }
                    if ( pu3 != NULL )
                        is_lan = 1;
                }
                if ( (!type && !pu2) || type == 2 )
                {
                    //  WAN
                    for ( entry = 0; entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK; entry++ )
                    {
                        pu2 = (u32*)ROUT_WAN_HASH_CMP_TBL(entry);
                        if ( *pu2 == *pu1 && pu2[1] == pu1[1] && pu2[2] == pu1[2] )
                        {
                            pu3 = (u32*)ROUT_WAN_HASH_ACT_TBL(entry);
                            break;
                        }
                    }
                    if ( entry == MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK )
                    {
                        for ( entry = 0; entry < WAN_ROUT_TBL_CFG->rout_num; entry++ )
                        {
                            pu2 = (u32*)ROUT_WAN_COLL_CMP_TBL(entry);
                            if ( *pu2 == *pu1 && pu2[1] == pu1[1] && pu2[2] == pu1[2] )
                            {
                                pu3 = (u32*)ROUT_WAN_COLL_ACT_TBL(entry);
                                break;
                            }
                        }
                        if ( entry == WAN_ROUT_TBL_CFG->rout_num )
                            pu2 = NULL;
                        else
                            entry += MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK;
                    }
                    if ( pu3 != NULL )
                        is_lan = 0;
                }
            }
        }
        else
        {
            if ( *pu1 && pu1[1] )
            {
                pu3 = NULL;
                //  check compare
                if ( !type || type == 1 )
                {
                    //  LAN
                    if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK )
                    {
                        pu2 = (u32*)ROUT_LAN_HASH_CMP_TBL(entry);
                        if ( *pu2 != *pu1 || pu2[1] != pu1[1] || pu2[2] != pu1[2] )
                            pu2 = NULL;
                        else
                            pu3 = (u32*)ROUT_LAN_HASH_ACT_TBL(entry);
                    }
                    else if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK + LAN_ROUT_TBL_CFG->rout_num )
                    {
                        pu2 = (u32*)ROUT_LAN_COLL_CMP_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                        if ( *pu2 != *pu1 || pu2[1] != pu1[1] || pu2[2] != pu1[2] )
                            pu2 = NULL;
                        else
                            pu3 = (u32*)ROUT_LAN_COLL_ACT_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                    }
                    if ( pu3 != NULL )
                        is_lan = 1;
                }
                if ( !type || type == 2 )
                {
                    //  WAN
                    if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK )
                    {
                        pu2 = (u32*)ROUT_WAN_HASH_CMP_TBL(entry);
                        if ( *pu2 != *pu1 || pu2[1] != pu1[1] || pu2[2] != pu1[2] )
                            pu2 = NULL;
                        else
                            pu3 = (u32*)ROUT_WAN_HASH_ACT_TBL(entry);
                    }
                    else if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK + WAN_ROUT_TBL_CFG->rout_num )
                    {
                        pu2 = (u32*)ROUT_WAN_COLL_CMP_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                        if ( *pu2 != *pu1 || pu2[1] != pu1[1] || pu2[2] != pu1[2] )
                            pu2 = NULL;
                        else
                            pu3 = (u32*)ROUT_WAN_COLL_ACT_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                    }
                    if ( pu3 != NULL )
                        is_lan = 0;
                }
            }
            else if ( !*pu1 && !pu1[1] )
            {
                if ( type == 1 )
                {
                    if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK )
                    {
                        pu2 = (u32*)ROUT_LAN_HASH_CMP_TBL(entry);
                        pu3 = (u32*)ROUT_LAN_HASH_ACT_TBL(entry);
                    }
                    else if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK + LAN_ROUT_TBL_CFG->rout_num )
                    {
                        pu2 = (u32*)ROUT_LAN_COLL_CMP_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                        pu3 = (u32*)ROUT_LAN_COLL_ACT_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                    }
                    if ( pu3 != NULL )
                        is_lan = 1;
                }
                else if ( type == 2 )
                {
                    if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK )
                    {
                        pu2 = (u32*)ROUT_WAN_HASH_CMP_TBL(entry);
                        pu3 = (u32*)ROUT_WAN_HASH_ACT_TBL(entry);
                    }
                    else if ( entry < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK + WAN_ROUT_TBL_CFG->rout_num )
                    {
                        pu2 = (u32*)ROUT_WAN_COLL_CMP_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                        pu3 = (u32*)ROUT_WAN_COLL_ACT_TBL(entry - MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK);
                    }
                    if ( pu3 != NULL )
                        is_lan = 0;
                }
            }
        }
        if ( pu2 && pu3 )
        {
            is_lan <<= 31;

            if ( ifx_ppa_drv_hal_generic_hook == NULL )
            {
                printk("HAL is not loaded - verify only, no firmware table operation\n");
                printk("del_routing_entry(%d - %08x)\n", entry, (u32)entry | is_lan);
            }
            else
            {
                PPE_ROUTING_INFO route = {0};
                int ret;

                route.entry = (u32)entry | is_lan;
                ret = ifx_ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_DEL_ROUTE_ENTRY, &route, 0);
                printk("%s entry %d deleted %s\n", is_lan ? "LAN" : "WAN", entry, ret == IFX_SUCCESS ? "successfully" : "fail");
            }
        }
    }
    else if ( operation == 1 && type && ((!is_ipv6 && *(u32*)&compare_tbl && *((u32*)&compare_tbl + 1)) || (is_ipv6 && *((u32*)&compare_tbl + 2))) )
    {
        if ( ifx_ppa_drv_hal_generic_hook == NULL )
        {
            char str_to_print[1024];

            printk("HAL is not loaded - verify only, no firmware table operation\n");

            if ( is_ipv6 )
            {
                printk("ipv6 src ip:  %d.%d.%d.%d", ipv6_src_ip[0] >> 24, (ipv6_src_ip[0] >> 16) & 0xFF, (ipv6_src_ip[0] >> 8) & 0xFF, ipv6_src_ip[0] & 0xFF);
                printk(".%d.%d.%d.%d", ipv6_src_ip[1] >> 24, (ipv6_src_ip[1] >> 16) & 0xFF, (ipv6_src_ip[1] >> 8) & 0xFF, ipv6_src_ip[1] & 0xFF);
                printk(".%d.%d.%d.%d", ipv6_src_ip[2] >> 24, (ipv6_src_ip[2] >> 16) & 0xFF, (ipv6_src_ip[2] >> 8) & 0xFF, ipv6_src_ip[2] & 0xFF);
                printk(".%d.%d.%d.%d", ipv6_src_ip[3] >> 24, (ipv6_src_ip[3] >> 16) & 0xFF, (ipv6_src_ip[3] >> 8) & 0xFF, ipv6_src_ip[3] & 0xFF);
                printk("\n");
                printk("ipv6 dest ip: %d.%d.%d.%d", ipv6_dst_ip[0] >> 24, (ipv6_dst_ip[0] >> 16) & 0xFF, (ipv6_dst_ip[0] >> 8) & 0xFF, ipv6_dst_ip[0] & 0xFF);
                printk(".%d.%d.%d.%d", ipv6_dst_ip[1] >> 24, (ipv6_dst_ip[1] >> 16) & 0xFF, (ipv6_dst_ip[1] >> 8) & 0xFF, ipv6_dst_ip[1] & 0xFF);
                printk(".%d.%d.%d.%d", ipv6_dst_ip[2] >> 24, (ipv6_dst_ip[2] >> 16) & 0xFF, (ipv6_dst_ip[2] >> 8) & 0xFF, ipv6_dst_ip[2] & 0xFF);
                printk(".%d.%d.%d.%d", ipv6_dst_ip[3] >> 24, (ipv6_dst_ip[3] >> 16) & 0xFF, (ipv6_dst_ip[3] >> 8) & 0xFF, ipv6_dst_ip[3] & 0xFF);
                printk("\n");
            }
            print_route(str_to_print, 0, type == 1 ? 1 : 0, &compare_tbl, &action_tbl);
            printk(str_to_print);
        }
        else
        {
            PPE_ROUTING_INFO route = {0};
            u8 test_mac[PPA_ETH_ALEN] = {0};

            test_mac[0] = (action_tbl.new_dest_mac54 >> 8) & 0xFF;
            test_mac[1] = action_tbl.new_dest_mac54 & 0xFF;
            test_mac[2] = (action_tbl.new_dest_mac30 >> 24) & 0xFF;
            test_mac[3] = (action_tbl.new_dest_mac30 >> 16) & 0xFF;
            test_mac[4] = (action_tbl.new_dest_mac30 >> 8) & 0xFF;
            test_mac[5] = action_tbl.new_dest_mac30 & 0xFF;

            route.f_is_lan = type == 1 ? 1 : 0;
            if ( is_ipv6 )
            {
                route.src_ip.f_ipv6 = 1;
                memcpy(route.src_ip.ip.ip6, ipv6_src_ip, 16);
                route.dst_ip.f_ipv6 = 1;
                memcpy(route.dst_ip.ip.ip6, ipv6_dst_ip, 16);
            }
            else
            {
                route.src_ip.f_ipv6 = 0;
                route.src_ip.ip.ip = compare_tbl.src_ip;
                route.dst_ip.f_ipv6 = 0;
                route.dst_ip.ip.ip = compare_tbl.dest_ip;
                route.new_ip.f_ipv6 = 0;
                route.new_ip.ip.ip = action_tbl.new_ip;
            }
            route.src_port              = compare_tbl.src_port;
            route.dst_port              = compare_tbl.dest_port;
            route.new_port              = action_tbl.new_port;
            route.f_is_tcp              = action_tbl.protocol;
            route.route_type            = action_tbl.rout_type;
            memcpy(route.new_dst_mac, test_mac, 6);
            route.src_mac.mac_ix        = action_tbl.new_src_mac_ix;
            route.mtu_info.mtu_ix       = action_tbl.mtu_ix;
            route.pppoe_mode            = action_tbl.pppoe_mode;
            route.pppoe_info.pppoe_ix   = action_tbl.pppoe_ix;
            route.f_new_dscp_enable     = action_tbl.new_dscp_en;
            route.new_dscp              = action_tbl.new_dscp;
            route.f_vlan_ins_enable     = action_tbl.in_vlan_ins;
            route.new_vci               = action_tbl.new_in_vci;
            route.f_vlan_rm_enable      = action_tbl.in_vlan_rm;
            route.f_out_vlan_ins_enable = action_tbl.out_vlan_ins;
            route.out_vlan_info.vlan_entry = action_tbl.out_vlan_ix;
            route.f_out_vlan_rm_enable  = action_tbl.out_vlan_rm;
            route.dslwan_qid            = action_tbl.dest_qid;
            route.dest_list             = action_tbl.dest_list;

            if ( ifx_ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_ADD_ROUTE_ENTRY, &route, 0) == IFX_SUCCESS )
                printk("%s entry add successfully: entry = %d\n", (route.entry & (1 << 31)) ? "LAN" : "WAN", route.entry & ~(1 << 31));
            else
                printk("%s entry add fail\n", type == 1 ? "LAN" : "WAN");
        }
    }
    else
        printk("No operation: operation = %d, type = %d, is_ipv6 = %d\n", operation, type, is_ipv6);

    return count;
}

static int proc_read_mc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    volatile struct wan_rout_multicast_cmp_tbl *pcompare;
    volatile struct wan_rout_multicast_act_tbl *paction;
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    volatile struct rtp_sampling_cnt *rtp_mib;   
#endif
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;

    pstr = *start = page;

    __sync();

    llen = sprintf(str, "Multicast Table\n");
    proc_buf_copy(&pstr, count, off, &len, str, llen);

    pcompare = ROUT_WAN_MC_CMP_TBL(0);
    paction = ROUT_WAN_MC_ACT_TBL(0);
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    rtp_mib= MULTICAST_RTP_MIB_TBL(0);
#endif
    for ( i = 0; i < MAX_WAN_MC_ENTRIES; i++ )
    {
        if ( pcompare->wan_dest_ip )
        {
            llen = print_mc(
                           str, i,
                           (struct wan_rout_multicast_cmp_tbl *)pcompare,
                           (struct wan_rout_multicast_act_tbl *)paction,
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
                           (struct rtp_sampling_cnt *)rtp_mib,
#endif
                           (uint32_t)*ROUT_WAN_MC_CNT(i)
                           );
            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_MC_OVERRUN_END;
        }

        pcompare++;
        paction++;
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
        rtp_mib++;
#endif
    }

    *eof = 1;

PROC_READ_MC_OVERRUN_END:
    return len - off;
}

static int proc_read_genconf(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    char *str;
    int llen = 0;
    int i;
    unsigned long bit;

    str = *start = page;

    __sync();


    llen += sprintf(str + llen, "ETH_PORTS_CFG    (0x%08X): WAN VLAN ID Hi - %d, WAN VLAN ID Lo - %d\n", (u32)ETH_PORTS_CFG, ETH_PORTS_CFG->wan_vlanid_hi, ETH_PORTS_CFG->wan_vlanid_lo);

    llen += sprintf(str + llen, "LAN_ROUT_TBL_CFG (0x%08X): num - %d, TTL check - %s, no hit drop - %s\n",
                                                            (u32)LAN_ROUT_TBL_CFG,
                                                            LAN_ROUT_TBL_CFG->rout_num,
                                                            LAN_ROUT_TBL_CFG->ttl_en ? "on" : "off",
                                                            LAN_ROUT_TBL_CFG->rout_drop  ? "on" : "off"
                                                            );
    llen += sprintf(str + llen, "                               IP checksum - %s, TCP/UDP checksum - %s, checksum error drop - %s\n",
                                                            LAN_ROUT_TBL_CFG->ip_ver_en ? "on" : "off",
                                                            LAN_ROUT_TBL_CFG->tcpdup_ver_en ? "on" : "off",
                                                            LAN_ROUT_TBL_CFG->iptcpudperr_drop ? "on" : "off"
                                                            );

    llen += sprintf(str + llen, "WAN_ROUT_TBL_CFG (0x%08X): num - %d, TTL check - %s, no hit drop - %s\n",
                                                            (u32)WAN_ROUT_TBL_CFG,
                                                            WAN_ROUT_TBL_CFG->rout_num,
                                                            WAN_ROUT_TBL_CFG->ttl_en ? "on" : "off",
                                                            WAN_ROUT_TBL_CFG->rout_drop  ? "on" : "off"
                                                            );
    llen += sprintf(str + llen, "                               MC num - %d, MC drop - %s\n",
                                                            WAN_ROUT_TBL_CFG->wan_rout_mc_num,
                                                            WAN_ROUT_TBL_CFG->wan_rout_mc_drop ? "on" : "off"
                                                            );
    llen += sprintf(str + llen, "                               IP checksum - %s, TCP/UDP checksum - %s, checksum error drop - %s\n",
                                                            WAN_ROUT_TBL_CFG->ip_ver_en ? "on" : "off",
                                                            WAN_ROUT_TBL_CFG->tcpdup_ver_en ? "on" : "off",
                                                            WAN_ROUT_TBL_CFG->iptcpudperr_drop ? "on" : "off"
                                                            );

    llen += sprintf(str + llen, "GEN_MODE_CFG1    (0x%08X): App2 - %s, U/S - %s, U/S early discard - %s\n"
                                "                               classification table entry - %d, IPv6 route entry - %d\n"
                                "                               session based rate control - %s, IPv4 WAN ingress hash alg - %s\n"
                                "                               multiple field based classification and VLAN assignment for bridging - %s\n"
                                "                               D/S IPv4 multicast acceleration - %s, IPv6 acceleration - %s\n"
                                "                               WAN acceleration - %s, LAN acceleration - %s, switch isolation - %s\n",
                                                            (u32)GEN_MODE_CFG1,
                                                            GEN_MODE_CFG1->app2_indirect ? "indirect" : "direct",
                                                            GEN_MODE_CFG1->us_indirect ? "indirect" : "direct",
                                                            GEN_MODE_CFG1->us_early_discard_en ? "enable" : "disable",
                                                            GEN_MODE_CFG1->classification_num,
                                                            GEN_MODE_CFG1->ipv6_rout_num,
                                                            GEN_MODE_CFG1->session_ctl_en ? "enable" : "disable",
                                                            GEN_MODE_CFG1->wan_hash_alg ? "src IP" : "dest port",
                                                            GEN_MODE_CFG1->brg_class_en ? "enable" : "disable",
                                                            GEN_MODE_CFG1->ipv4_mc_acc_mode ? "IP/port pairs" : "dst IP only",
                                                            GEN_MODE_CFG1->ipv6_acc_en ? "enable" : "disable",
                                                            GEN_MODE_CFG1->wan_acc_en ? "enable" : "disable",
                                                            GEN_MODE_CFG1->lan_acc_en ? "enable" : "disable",
                                                            GEN_MODE_CFG1->sw_iso_mode ? "isolated" : "not isolated"
                                                            );
    llen += sprintf(str + llen, "WAN Interfaces:                %s \n", get_wanitf(1));
    llen += sprintf(str + llen, "LAN Interfaces:                %s \n", get_wanitf(0));
    llen += sprintf(str + llen, "GEN_MODE_CFG2    (0x%08X):", (u32)GEN_MODE_CFG2);
    for ( i = 0, bit = 1; i < 8; i++, bit <<= 1 )
    {
        llen += sprintf(str + llen, " %d - %s", i, (GEN_MODE_CFG2->itf_outer_vlan_vld & bit) ? "on " : "off");
        if ( i == 3 )
            llen += sprintf(str + llen, "\n                              ");
    }
    llen += sprintf(str + llen, "\n");

    llen += sprintf(str + llen, "TX_QOS_CFG       (0x%08X): time_tick - %d, overhd_bytes - %d, eth1_eg_qnum - %d\n"
                                "                               eth1_burst_chk - %s, eth1 - %s, rate shaping - %s, WFQ - %s\n",
                                                            (u32)TX_QOS_CFG,
                                                            TX_QOS_CFG->time_tick,
                                                            TX_QOS_CFG->overhd_bytes,
                                                            TX_QOS_CFG->eth1_eg_qnum,
                                                            TX_QOS_CFG->eth1_burst_chk ? "on" : "off",
                                                            TX_QOS_CFG->eth1_qss ? "FW QoS" : "HW QoS",
                                                            TX_QOS_CFG->shape_en ? "on" : "off",
                                                            TX_QOS_CFG->wfq_en ? "on" : "off");

    llen += sprintf(str + llen, "KEY_SEL_n        (0x%08X): %08x %08x %08x %08x\n",
                                                            (u32)KEY_SEL_n(0),
                                                            *KEY_SEL_n(0), *KEY_SEL_n(1), *KEY_SEL_n(2), *KEY_SEL_n(3));

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    llen += sprintf(str + llen, "MIB Mode - %s\n",
                                                         PS_MC_GENCFG3->session_mib_unit
                                                         ? "Packet" : "Byte"
                                                            );
#endif

    *eof = 1;

    return llen - off;
}

static int proc_write_genconf(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int f_wan_hi = 0;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( strincmp(p, "wan hi ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        f_wan_hi = 1;
    }
    else if ( strincmp(p, "wan high ", 9) == 0 )
    {
        p += 9;
        rlen -= 9;
        f_wan_hi = 1;
    }
    else if ( strincmp(p, "wan lo ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        f_wan_hi = -1;
    }
    else if ( strincmp(p, "wan low ", 8) == 0 )
    {
        p += 8;
        rlen -= 8;
        f_wan_hi = -1;
    }
    else if ( strincmp(p, "eth0 type ", 10) == 0 )
    {
        p += 10;
        rlen -= 10;
        if ( stricmp(p, "lan") == 0 )
            ETH_PORTS_CFG->eth0_type = 0;
        else if ( stricmp(p, "wan") == 0 )
            ETH_PORTS_CFG->eth0_type = 1;
        else if ( stricmp(p, "mix") == 0 )
            ETH_PORTS_CFG->eth0_type = 2;
    }

    if ( f_wan_hi )
    {
        int num;

        num = get_number(&p, &rlen, 0);
        if ( f_wan_hi > 0 )
            ETH_PORTS_CFG->wan_vlanid_hi = num;
        else
            ETH_PORTS_CFG->wan_vlanid_lo = num;
    }

    return count;
}

static int proc_read_pppoe(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;

    pstr = *start = page;

    __sync();

    llen = sprintf(pstr, "PPPoE Table (0x%08X) - Session ID:\n", (u32)PPPOE_CFG_TBL(0));
    pstr += llen;
    len += llen;

    for ( i = 0; i < 8; i++ )
    {
        llen = sprintf(str, "  %d: %d\n", i, *PPPOE_CFG_TBL(i));
        if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
            goto PROC_READ_PPPOE_OVERRUN_END;
    }

    *eof = 1;

PROC_READ_PPPOE_OVERRUN_END:
    return len - off;
}

static int proc_read_mtu(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;

    pstr = *start = page;

    __sync();

    llen = sprintf(pstr, "MTU Table (0x%08X):\n", (u32)MTU_CFG_TBL(0));
    pstr += llen;
    len += llen;

    for ( i = 0; i < 8; i++ )
    {
        llen = sprintf(str, "  %d: %d\n", i, *MTU_CFG_TBL(i));
        if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
            goto PROC_READ_MTU_OVERRUN_END;
    }

    *eof = 1;

PROC_READ_MTU_OVERRUN_END:
    return len - off;
}

static int proc_read_hit(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;
    int n;
    unsigned long bit;

    pstr = *start = page;

    __sync();

    llen = sprintf(pstr, "Unicast Routing Hit Table (0x%08X):\n", (u32)ROUT_LAN_HASH_HIT_STAT_TBL(0));
    pstr += llen;
    len += llen;

    llen = sprintf(pstr, "             1 2 3 4 5 6 7 8 9 10\n");
    pstr += llen;
    len += llen;

    n = 1;
    for ( i = 0; i < 8; i++ )
        for ( bit = 0x80000000; bit; bit >>= 1 )
        {
            if ( n % 10 == 1 )
                llen = sprintf(str, "  %3d - %3d:", n, n + 9);

            llen += sprintf(str + llen, " %d", (*ROUT_LAN_HASH_HIT_STAT_TBL(i) & bit) ? 1 : 0);

            if ( n++ % 10 == 0 )
            {
                llen += sprintf(str + llen, "\n");

                if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                    goto PROC_READ_HIT_OVERRUN_END;
            }
        }

    if ( n % 10 != 0 )
    {
        llen += sprintf(str + llen, "\n");

        if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
            goto PROC_READ_HIT_OVERRUN_END;
    }

    llen = sprintf(str, "Multicast Routing Hit Table (0x%08X):\n", (u32)ROUT_WAN_MC_HIT_STAT_TBL(0));
    llen += sprintf(str + llen, "             1 2 3 4 5 6 7 8 9 10\n");
    if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
        goto PROC_READ_HIT_OVERRUN_END;

    n = 1;
    for ( i = 0; i < 1; i++ )
        for ( bit = 0x80000000; bit; bit >>= 1 )
        {
            if ( n % 10 == 1 )
                llen = sprintf(str, "  %3d - %3d:", n, n + 9);

            llen += sprintf(str + llen, " %d", (*ROUT_WAN_MC_HIT_STAT_TBL(i) & bit) ? 1 : 0);

            if ( n++ % 10 == 0 )
            {
                llen += sprintf(str + llen, "\n");

                if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                    goto PROC_READ_HIT_OVERRUN_END;
            }
        }

    if ( n % 10 != 0 )
    {
        llen += sprintf(str + llen, "\n");

        if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
            goto PROC_READ_HIT_OVERRUN_END;
    }

    *eof = 1;

PROC_READ_HIT_OVERRUN_END:
    return len - off;
}

static int proc_write_hit(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "clear") == 0 || stricmp(p, "clean") == 0 )
    {
        ppa_memset((void*)ROUT_LAN_HASH_HIT_STAT_TBL(0), 0, 64);
        ppa_memset((void*)ROUT_LAN_COLL_HIT_STAT_TBL(0), 0, 2);
        ppa_memset((void*)ROUT_WAN_HASH_HIT_STAT_TBL(0), 0, 64);
        ppa_memset((void*)ROUT_WAN_COLL_HIT_STAT_TBL(0), 0, 2);
        ppa_memset((void*)ROUT_WAN_MC_HIT_STAT_TBL(0), 0, 2);
    }

    return count;
}

static int proc_read_mac(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;
    unsigned int mac52, mac10;

    pstr = *start = page;

    __sync();

    llen = sprintf(pstr, "MAC Table:\n");
    pstr += llen;
    len += llen;

    llen = sprintf(pstr, "  ROUT_MAC_CFG_TBL (0x%08X)\n", (u32)ROUT_MAC_CFG_TBL(0));
    pstr += llen;
    len += llen;

    for ( i = 0; i < 16; i++ )
    {
        mac52 = *ROUT_MAC_CFG_TBL(i);
        mac10 = *(ROUT_MAC_CFG_TBL(i) + 1);

        llen = sprintf(str, "    %2d: %02X:%02X:%02X:%02X:%02X:%02X\n", i + 1, mac52 >> 24, (mac52 >> 16) & 0xFF, (mac52 >> 8) & 0xFF, mac52 & 0xFF, mac10 >> 24, (mac10 >> 16) & 0xFF);
        if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
            goto PROC_READ_MAC_OVERRUN_END;
    }

    *eof = 1;

PROC_READ_MAC_OVERRUN_END:
    return len - off;
}

static int proc_read_out_vlan(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;
    int i;

    pstr = *start = page;

    __sync();

    llen = sprintf(pstr, "Outer VLAN Table (0x%08X):\n", (u32)OUTER_VLAN_TBL(0));
    pstr += llen;
    len += llen;

    for ( i = 0; i < 32; i++ )
    {
        llen = sprintf(str, "  %d: 0x%08X\n", i, *OUTER_VLAN_TBL(i));
        if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
            goto PROC_READ_MAC_OVERRUN_END;
    }

    *eof = 1;

PROC_READ_MAC_OVERRUN_END:
    return len - off;
}

static int proc_read_ipv6_ip(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[2048];
    int llen;

    int i, j, x;
    volatile char *p;

    pstr = *start = page;

    __sync();

    llen = sprintf(str, "IPv6 IP Table:\n");
    proc_buf_copy(&pstr, count, off, &len, str, llen);

    for ( x = 0; x < MAX_IPV6_IP_ENTRIES_BLOCK; x++ )
        for ( i = 0; i < MAX_IPV6_IP_ENTRIES_PER_BLOCK; i++ )
            if ( IPv6_IP_IDX_TBL(x, i)[0] != 0 || IPv6_IP_IDX_TBL(x, i)[1] != 0 || IPv6_IP_IDX_TBL(x, i)[2] != 0 || IPv6_IP_IDX_TBL(x, i)[3] != 0 )
            {
                p = (volatile char *)IPv6_IP_IDX_TBL(x, i);
                llen = sprintf(str, "%3d - %u", x * MAX_IPV6_IP_ENTRIES_PER_BLOCK + i, (unsigned int)p[0]);
                for ( j = 1; j < 16; j++ )
                    llen += sprintf(str + llen, ".%u", (unsigned int)p[j]);
                llen += sprintf(str + llen, "\n");

                if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                    goto PROC_READ_IPv6_IP_OVERRUN_END;
            }

    *eof = 1;

PROC_READ_IPv6_IP_OVERRUN_END:
    return len - off;
}

#endif

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
static int proc_read_dbg(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + len, "error print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ERR)              ? "enabled" : "disabled");
    len += sprintf(page + len, "debug print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT)      ? "enabled" : "disabled");
    len += sprintf(page + len, "assert           - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ASSERT)           ? "enabled" : "disabled");
    len += sprintf(page + len, "dump rx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_RX)      ? "enabled" : "disabled");
    len += sprintf(page + len, "dump tx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_TX)      ? "enabled" : "disabled");
    len += sprintf(page + len, "dump flag header - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_FLAG_HEADER) ? "enabled" : "disabled");
    len += sprintf(page + len, "dump init        - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_INIT)        ? "enabled" : "disabled");
  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST
    len += sprintf(page + len, "ipv6             - %s\n", (ipv6_en)             ? "enabled"  : "disabled");
    len += sprintf(page + len, "fw_mode          - %s\n", (ppe_fw_mode)        ? "mix_mode" : "normal_mode");
    len += sprintf(page + len, "wan_hash_algo    - %s\n", (ppe_wan_hash_algo)   ? "Algo 1"   : "Algo 0");
    len += sprintf(page + len, "acc_protocol     - %s\n", (acc_proto)        ? "TCP" : "UDP");
  #endif

    *eof = 1;

    return len;
}

static int proc_write_dbg(struct file *file, const char *buf, unsigned long count, void *data)
{
    static const char *dbg_enable_mask_str[] = {
        " error print",
        " err",
        " debug print",
        " dbg",
        " assert",
        " assert",
        " dump rx skb",
        " rx",
        " dump tx skb",
        " tx",
        " dump flag header",
        " header",
        " dump init",
        " init",
        " all"
    };
    static const int dbg_enable_mask_str_len[] = {
        12, 4,
        12, 4,
        7,  7,
        12, 3,
        12, 3,
        17, 7,
        10, 5,
        4
    };
    u32 dbg_enable_mask[] = {
        DBG_ENABLE_MASK_ERR,
        DBG_ENABLE_MASK_DEBUG_PRINT,
        DBG_ENABLE_MASK_ASSERT,
        DBG_ENABLE_MASK_DUMP_SKB_RX,
        DBG_ENABLE_MASK_DUMP_SKB_TX,
        DBG_ENABLE_MASK_DUMP_FLAG_HEADER,
        DBG_ENABLE_MASK_DUMP_INIT,
        DBG_ENABLE_MASK_ALL
    };

    char str[2048];
    char *p;

    int len, rlen;

    int f_enable = 0;
    int i;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST
    if ( strincmp(p, "enable_ipv6", 11) == 0 ) {
        ipv6_en = 1;
        ppe_fw_mode = 0;
        acc_proto = 0;
        ppe_wan_hash_algo = 0;
        printk("Mode          : Normal Node\n");
        printk("Protocol      : UDP\n");
        printk("WAN HASH Algo : Algorithm 0\n");
        init_acc_tables();
    } else if ( strincmp(p, "disable_ipv6", 12) == 0 ) {
        ipv6_en = 0;
        ppe_fw_mode = 0;
        acc_proto = 0;
        ppe_wan_hash_algo = 0;
        printk("Mode          : Normal Node\n");
        printk("Protocol      : UDP\n");
        printk("WAN HASH Algo : Algorithm 0\n");
        init_acc_tables();
    } else if ( strincmp(p, "set_acc_udp", 11) == 0 ) {
        if (acc_proto != 0) {
            acc_proto = 0;
            init_acc_tables();
        }
    } else if ( strincmp(p, "set_acc_tcp", 11) == 0 ) {
        if (acc_proto != 1) {
            acc_proto = 1;
            init_acc_tables();
        }
    } else if ( strincmp(p, "set_mix_mode", 12) == 0 ) {
        if (ppe_fw_mode != 1) {
            ppe_fw_mode = 1;
            init_acc_tables();
        }
    } else if ( strincmp(p, "set_normal_mode", 15) == 0 ) {
        if (ppe_fw_mode != 0) {
            ppe_fw_mode = 0;
            init_acc_tables();
        }
    }
    else if ( strincmp(p, "set_algo_0", 10) == 0 ) {
        if (ppe_wan_hash_algo != 0) {
            ppe_wan_hash_algo = 0;
            init_acc_tables();
        }
    } else if ( strincmp(p, "set_algo_1", 10) == 0 ) {
        if (ppe_wan_hash_algo != 1) {
            ppe_wan_hash_algo = 1;
            init_acc_tables();
        }
    } else
  #endif
    if ( strincmp(p, "enable", 6) == 0 )
    {
        p += 6;
        f_enable = 1;
    }
    else if ( strincmp(p, "disable", 7) == 0 )
    {
        p += 7;
        f_enable = -1;
    }
    else if ( strincmp(p, "help", 4) == 0 || *p == '?' )
    {
        printk("echo <enable/disable> [err/dbg/assert/rx/tx/header/init/all] > /proc/eth/dbg\n");
  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST
        printk("echo enable_ipv6/disable_ipv6\n");
        printk("echo set_mix_mode/set_normal_mode/set_algo_0/set_algo_1\n");
        printk("echo set_acc_udp/set_acc_tcp\n");
  #endif
    }

    if ( f_enable )
    {
        if ( *p == 0 )
        {
            if ( f_enable > 0 )
                g_dbg_enable |= DBG_ENABLE_MASK_ALL;
            else
                g_dbg_enable &= ~DBG_ENABLE_MASK_ALL;
        }
        else
        {
            do
            {
                for ( i = 0; i < NUM_ENTITY(dbg_enable_mask_str); i++ )
                    if ( strincmp(p, dbg_enable_mask_str[i], dbg_enable_mask_str_len[i]) == 0 )
                    {
                        if ( f_enable > 0 )
                            g_dbg_enable |= dbg_enable_mask[i >> 1];
                        else
                            g_dbg_enable &= ~dbg_enable_mask[i >> 1];
                        p += dbg_enable_mask_str_len[i];

                        if(f_enable > 0 && (strincmp(dbg_enable_mask_str[i], " rx", 3) == 0 ||
						                    strincmp(dbg_enable_mask_str[i], " tx", 3) == 0)){
							rlen -= 9; //enable + rx/tx
							ignore_space(&p, &rlen);
							g_dump_cnt = get_number(&p, &rlen, 0);
							if(g_dump_cnt == 0)
							   g_dump_cnt = -1;
						}
                        break;
                    }
            } while ( i < NUM_ENTITY(dbg_enable_mask_str) );
        }
    }

    return count;
}
#endif

#define PP32_REG_ADDR_BEGIN     0x0
#define PP32_REG_ADDR_END       0x1FFF
#define PP32_SB_ADDR_BEGIN      0x2000
#define PP32_SB_ADDR_END        0x4BFF

static inline unsigned long sb_addr_to_fpi_addr_convert(unsigned long sb_addr)
{
    if ( sb_addr <= PP32_SB_ADDR_END) {
        return (unsigned long ) SB_BUFFER(sb_addr);
    }
    else {
        return sb_addr;
    }
}

#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
static int proc_write_mem(struct file *file, const char *buf, unsigned long count, void *data)
{
    char *p1, *p2;
    int len;
    int colon;
    unsigned long *p;
    unsigned long dword;
    char local_buf[1024];
    int i, n, l;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    p2 = NULL;
    colon = 1;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "w") == 0 || stricmp(p1, "write") == 0 || stricmp(p1, "r") == 0 || stricmp(p1, "read") == 0 )
            break;

        p1 = p2;
        colon = 1;
    }

    if ( *p1 == 'w' )
    {
        ignore_space(&p2, &len);
        p = (unsigned long *)get_number(&p2, &len, 1);
        p = (unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);

        if ( (u32)p >= KSEG0 )
            while ( 1 )
            {
                ignore_space(&p2, &len);
                if ( !len || !((*p2 >= '0' && *p2 <= '9') || (*p2 >= 'a' && *p2 <= 'f') || (*p2 >= 'A' && *p2 <= 'F')) )
                    break;

                *p++ = (u32)get_number(&p2, &len, 1);
            }
    }
    else if ( *p1 == 'r' )
    {
        ignore_space(&p2, &len);
        p = (unsigned long *)get_number(&p2, &len, 1);
        p = (unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);

        if ( (u32)p >= KSEG0 )
        {
            ignore_space(&p2, &len);
            n = (int)get_number(&p2, &len, 0);
            if ( n )
            {
                char str[32] = {0};
                char *pch = str;
                int k;
                char c;

                n += (l = ((int)p >> 2) & 0x03);
                p = (unsigned long *)((u32)p & ~0x0F);
                for ( i = 0; i < n; i++ )
                {
                    if ( (i & 0x03) == 0 )
                    {
                        printk("%08X:", (u32)p);
                        pch = str;
                    }
                    if ( i < l )
                    {
                        printk("         ");
                        sprintf(pch, "    ");
                    }
                    else
                    {
                        dword = *p;
                        printk(" %08X", (u32)dword);
                        for ( k = 0; k < 4; k++ )
                        {
                            c = ((char*)&dword)[k];
                            pch[k] = c < ' ' ? '.' : c;
                        }
                    }
                    p++;
                    pch += 4;
                    if ( (i & 0x03) == 0x03 )
                    {
                        pch[0] = 0;
                        printk(" ; %s\n", str);
                    }
                }
                if ( (n & 0x03) != 0x00 )
                {
                    for ( k = 4 - (n & 0x03); k > 0; k-- )
                        printk("         ");
                    pch[0] = 0;
                    printk(" ; %s\n", str);
                }
            }
        }
    }

    return count;
}
#endif

#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
static int proc_read_pp32(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    static const char *stron = " on";
    static const char *stroff = "off";

    int len = 0;
    int cur_context;
    int f_stopped = 0;
    char str[256];
    char strlength;
    int i, j;

    if ( PP32_CPU_USER_STOPPED || PP32_CPU_USER_BREAKIN_RCV || PP32_CPU_USER_BREAKPOINT_MET )
    {
        strlength = 0;
        if ( PP32_CPU_USER_STOPPED )
            strlength += sprintf(str + strlength, "stopped");
        if ( PP32_CPU_USER_BREAKPOINT_MET )
            strlength += sprintf(str + strlength, strlength ? " | breakpoint" : "breakpoint");
        if ( PP32_CPU_USER_BREAKIN_RCV )
            strlength += sprintf(str + strlength, strlength ? " | breakin" : "breakin");
        f_stopped = 1;
    }
    else if ( PP32_CPU_CUR_PC == PP32_CPU_CUR_PC )
    {
        sprintf(str, "stopped");
        f_stopped = 1;
    }
    else
        sprintf(str, "running");
    cur_context = PP32_BRK_CUR_CONTEXT;
    len += sprintf(page + len, "Context: %d, PC: 0x%04x, %s\n", cur_context, PP32_CPU_CUR_PC, str);

    if ( PP32_CPU_USER_BREAKPOINT_MET )
    {
        strlength = 0;
        if ( PP32_BRK_PC_MET(0) )
            strlength += sprintf(str + strlength, "pc0");
        if ( PP32_BRK_PC_MET(1) )
            strlength += sprintf(str + strlength, strlength ? " | pc1" : "pc1");
        if ( PP32_BRK_DATA_ADDR_MET(0) )
            strlength += sprintf(str + strlength, strlength ? " | daddr0" : "daddr0");
        if ( PP32_BRK_DATA_ADDR_MET(1) )
            strlength += sprintf(str + strlength, strlength ? " | daddr1" : "daddr1");
        if ( PP32_BRK_DATA_VALUE_RD_MET(0) )
        {
            strlength += sprintf(str + strlength, strlength ? " | rdval0" : "rdval0");
            if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(0) )
            {
                if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(0) )
                    strlength += sprintf(str + strlength, " ==");
                else
                    strlength += sprintf(str + strlength, " <=");
            }
            else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(0) )
                strlength += sprintf(str + strlength, " >=");
        }
        if ( PP32_BRK_DATA_VALUE_RD_MET(1) )
        {
            strlength += sprintf(str + strlength, strlength ? " | rdval1" : "rdval1");
            if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(1) )
            {
                if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(1) )
                    strlength += sprintf(str + strlength, " ==");
                else
                    strlength += sprintf(str + strlength, " <=");
            }
            else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(1) )
                strlength += sprintf(str + strlength, " >=");
        }
        if ( PP32_BRK_DATA_VALUE_WR_MET(0) )
        {
            strlength += sprintf(str + strlength, strlength ? " | wtval0" : "wtval0");
            if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(0) )
            {
                if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(0) )
                    strlength += sprintf(str + strlength, " ==");
                else
                    strlength += sprintf(str + strlength, " <=");
            }
            else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(0) )
                strlength += sprintf(str + strlength, " >=");
        }
        if ( PP32_BRK_DATA_VALUE_WR_MET(1) )
        {
            strlength += sprintf(str + strlength, strlength ? " | wtval1" : "wtval1");
            if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(1) )
            {
                if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(1) )
                    strlength += sprintf(str + strlength, " ==");
                else
                    strlength += sprintf(str + strlength, " <=");
            }
            else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(1) )
                strlength += sprintf(str + strlength, " >=");
        }
        len += sprintf(page + len, "break reason: %s\n", str);
    }

    if ( f_stopped )
    {
        len += sprintf(page + len, "General Purpose Register (Context %d):\n", cur_context);
        for ( i = 0; i < 4; i++ )
        {
            for ( j = 0; j < 4; j++ )
                len += sprintf(page + len, "   %2d: %08x", i + j * 4, *PP32_GP_CONTEXTi_REGn(cur_context, i + j * 4));
            len += sprintf(page + len, "\n");
        }
    }

    len += sprintf(page + len, "break out on: break in - %s, stop - %s\n",
                                        PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN ? stron : stroff,
                                        PP32_CTRL_OPT_BREAKOUT_ON_STOP ? stron : stroff);
    len += sprintf(page + len, "     stop on: break in - %s, break point - %s\n",
                                        PP32_CTRL_OPT_STOP_ON_BREAKIN ? stron : stroff,
                                        PP32_CTRL_OPT_STOP_ON_BREAKPOINT ? stron : stroff);
    len += sprintf(page + len, "breakpoint:\n");
    len += sprintf(page + len, "     pc0: 0x%08x, %s\n", *PP32_BRK_PC(0), PP32_BRK_GRPi_PCn(0, 0) ? "group 0" : "off");
    len += sprintf(page + len, "     pc1: 0x%08x, %s\n", *PP32_BRK_PC(1), PP32_BRK_GRPi_PCn(1, 1) ? "group 1" : "off");
    len += sprintf(page + len, "  daddr0: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(0), PP32_BRK_GRPi_DATA_ADDRn(0, 0) ? "group 0" : "off");
    len += sprintf(page + len, "  daddr1: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(1), PP32_BRK_GRPi_DATA_ADDRn(1, 1) ? "group 1" : "off");
    len += sprintf(page + len, "  rdval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(0));
    len += sprintf(page + len, "  rdval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(1));
    len += sprintf(page + len, "  wrval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(0));
    len += sprintf(page + len, "  wrval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(1));

    *eof = 1;

    ASSERT(len <= 4096, "proc read buffer overflow");
    return len;
}

static int proc_write_pp32(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    u32 addr;

    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "start") == 0 )
        *PP32_CTRL_CMD = PP32_CTRL_CMD_RESTART;
    else if ( stricmp(p, "stop") == 0 )
        *PP32_CTRL_CMD = PP32_CTRL_CMD_STOP;
    else if ( stricmp(p, "step") == 0 )
        *PP32_CTRL_CMD = PP32_CTRL_CMD_STEP;
    else if ( strincmp(p, "pc0 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG = PP32_BRK_GRPi_PCn_OFF(0, 0);
            *PP32_BRK_PC_MASK(0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(0) = addr;
            *PP32_BRK_PC_MASK(0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG = PP32_BRK_GRPi_PCn_ON(0, 0);
        }
    }
    else if ( strincmp(p, "pc1 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG = PP32_BRK_GRPi_PCn_OFF(1, 1);
            *PP32_BRK_PC_MASK(1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(1) = addr;
            *PP32_BRK_PC_MASK(1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG = PP32_BRK_GRPi_PCn_ON(1, 1);
        }
    }
    else if ( strincmp(p, "daddr0 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG = PP32_BRK_GRPi_DATA_ADDRn_OFF(0, 0);
            *PP32_BRK_DATA_ADDR_MASK(0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(0) = addr;
            *PP32_BRK_DATA_ADDR_MASK(0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG = PP32_BRK_GRPi_DATA_ADDRn_ON(0, 0);
        }
    }
    else if ( strincmp(p, "daddr1 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( stricmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG = PP32_BRK_GRPi_DATA_ADDRn_OFF(1, 1);
            *PP32_BRK_DATA_ADDR_MASK(1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(1) = addr;
            *PP32_BRK_DATA_ADDR_MASK(1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG = PP32_BRK_GRPi_DATA_ADDRn_ON(1, 1);
        }
    }
    else
    {

        printk("echo \"<command>\" > /proc/eth/pp32\n");
        printk("  command:\n");
        printk("    start  - run pp32\n");
        printk("    stop   - stop pp32\n");
        printk("    step   - run pp32 with one step only\n");
        printk("    pc0    - pc0 <addr>/off, set break point PC0\n");
        printk("    pc1    - pc1 <addr>/off, set break point PC1\n");
        printk("    daddr0 - daddr0 <addr>/off, set break point data address 0\n");
        printk("    daddr0 - daddr1 <addr>/off, set break point data address 1\n");
        printk("    help   - print this screen\n");
    }

    if ( *PP32_BRK_TRIG )
        *PP32_CTRL_OPT = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON;
    else
        *PP32_CTRL_OPT = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF;

    return count;
}
#endif

static int proc_read_burstlen(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    u32 dma_ps;
    u32 dma_pctrl;
    unsigned long sys_flag;

    local_irq_save(sys_flag);
    dma_ps = *(volatile u32 *)0xBE104140;
    *(volatile u32 *)0xBE104140 = 0;
    dma_pctrl = *(volatile u32 *)0xBE104144;
    *(volatile u32 *)0xBE104140 = dma_ps;
    local_irq_restore(sys_flag);

    len += sprintf(page + len, "DMA-PPE burst length: Rx %d, Tx %d\n", 1 << ((dma_pctrl >> 2) & 0x03), 1 << ((dma_pctrl >> 4) & 0x03));

    *eof = 1;

    return len;
}

static int proc_write_burstlen(struct file *file, const char *buf, unsigned long count, void *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 0;
    int mask = 0x3C;    //  rx: 0x0C, tx: 0x30, all: 0x3C
    int burstlen = 0;
    int burstlen_mask = 0;
    int f_help = 0;
    u32 dma_ps;
    unsigned long sys_flag;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "rx") == 0 )
            burstlen_mask |= (mask = 0x0C);
        else if ( stricmp(p1, "tx") == 0 )
            burstlen_mask |= (mask = 0x30);
        else if ( stricmp(p1, "all") == 0 )
            burstlen_mask |= (mask = 0x3C);
        else if ( strcmp(p1, "2") == 0 )
            burstlen = 0x14 & mask;
        else if ( strcmp(p1, "4") == 0 )
            burstlen = 0x28 & mask;
        else if ( strcmp(p1, "8") == 0 )
            burstlen = 0x3C & mask;
        else
        {
            f_help = 1;
            break;
        }

        p1 = p2;
    }

    if ( !burstlen_mask && burstlen )
        burstlen_mask = 0x3C;

    if ( !burstlen_mask || !burstlen )
        f_help = 1;

    if ( !f_help )
    {
        local_irq_save(sys_flag);
        dma_ps = *(volatile u32 *)0xBE104140;
        *(volatile u32 *)0xBE104140 = 0;
        *(volatile u32 *)0xBE104144 = (*(volatile u32 *)0xBE104144 & ~burstlen_mask) | burstlen;
        *(volatile u32 *)0xBE104140 = dma_ps;
        local_irq_restore(sys_flag);
    }
    else
    {
        printk("echo [rx/tx/all] <2/4/8> > /proc/eth/burstlen\n");
    }

    return count;
}

#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
static int proc_read_fw(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

//    u32 sll_state;
    u32 tir4;
    u32 tir4_sll_state;
    u32 tir4_dplus_rx_on;

    len += sprintf(page + len, "Firmware\n");
//    len += sprintf(page + len, "  ACC_ERX_PID        = %08X\n", *ACC_ERX_PID);
//    len += sprintf(page + len, "  ACC_ERX_PORT_TIMES = %08X\n", *ACC_ERX_PORT_TIMES);

//    sll_state = *SLL_ISSUED;
    tir4 = *TIR(4);
    tir4_sll_state = (tir4 >> 21) & 7;
    tir4_dplus_rx_on = (tir4 >> 20) & 1;

    /*
    len += sprintf(page + len, "  SLL_ISSUED         = %d [%s], tir4.sll_state = %d [%s]\n",
                        sll_state,
                        sll_state == SLL_STATE_NULL     ? "NULL"   :
                        sll_state == SLL_STATE_DA       ? "BRG_DA" :
                        sll_state == SLL_STATE_SA       ? "BRG_SA" :
                        sll_state == SLL_STATE_ROUTER   ? "ROUT" :
                                                          "Error!",
                        tir4_sll_state,
                        tir4_sll_state == SLL_STATE_NULL    ? "NULL"   :
                        tir4_sll_state == SLL_STATE_DA      ? "BRG_DA" :
                        tir4_sll_state == SLL_STATE_SA      ? "BRG_SA" :
                        tir4_sll_state == SLL_STATE_ROUTER  ? "ROUT" :
                                                              "Error!"  );

    len += sprintf(page + len, "  DPLUS_RX_ON        = %d, tir4.dplus_rx_on = %d\n",
            *DPLUS_RX_ON, tir4_dplus_rx_on);


    len += sprintf(page + len, "  BMC_ISSUED         = %08X\n", *BMC_ISSUED);

    len += sprintf(page + len, "  PRESEARCH_RDPTR    = %08X, %08X\n",
            *PRESEARCH_RDPTR, *(PRESEARCH_RDPTR + 1));

    len += sprintf(page + len, "  SLL_ERX_PID        = %08X\n", *SLL_ERX_PID);

    len += sprintf(page + len, "  SLL_PKT_CNT        = %08X, %08X\n",
            *SLL_PKT_CNT, *(SLL_PKT_CNT + 1));

    len += sprintf(page + len, "  SLL_RDPTR          = %08X, %08X\n",
            *SLL_RDPTR, *(SLL_RDPTR + 1));


    len += sprintf(page + len, "  EDIT_PKT_CNT       = %08X, %08X\n",
            *EDIT_PKT_CNT, *(EDIT_PKT_CNT + 1));
    len += sprintf(page + len, "  EDIT_RDPTR         = %08X, %08X\n",
            *EDIT_RDPTR, *(EDIT_RDPTR + 1));

    len += sprintf(page + len, "  DPLUSRX_PKT_CNT    = %08X, %08X\n",
            *DPLUSRX_PKT_CNT, *(DPLUSRX_PKT_CNT + 1));
    len += sprintf(page + len, "  DPLUS_RDPTR        = %08X, %08X\n",
            *DPLUS_RDPTR, *(DPLUS_RDPTR + 1));

    //len += sprintf(page + len, "  POSTSEARCH_RDPTR   = %08X, %08X\n",

    len += sprintf(page + len, "\nQoS:\n");
    len += sprintf(page + len, "  DSLWAN_TX_SWAP_RDPTR = %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X\n",
                                            * __DSLWAN_TXDES_SWAP_PTR0, * __DSLWAN_TXDES_SWAP_PTR1,
                                            * __DSLWAN_TXDES_SWAP_PTR2, * __DSLWAN_TXDES_SWAP_PTR3,
                                            * __DSLWAN_TXDES_SWAP_PTR4, * __DSLWAN_TXDES_SWAP_PTR5,
                                            * __DSLWAN_TXDES_SWAP_PTR6, * __DSLWAN_TXDES_SWAP_PTR7 );

    len += sprintf(page + len, "  DSLWAN_TX_PKT_CNT    = %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X\n",
                                            * __DSLWAN_TX_PKT_CNT0, * __DSLWAN_TX_PKT_CNT1,
                                            * __DSLWAN_TX_PKT_CNT2, * __DSLWAN_TX_PKT_CNT3,
                                            * __DSLWAN_TX_PKT_CNT4, * __DSLWAN_TX_PKT_CNT5,
                                            * __DSLWAN_TX_PKT_CNT6, * __DSLWAN_TX_PKT_CNT7);

    len += sprintf(page + len, "  QOSD_DPLUS_RDPTR     = %04X\n",
                                            * __DSLWAN_FP_RXDES_SWAP_RDPTR);

    len += sprintf(page + len, "  QOSD_CPUTX_RDPTR     = %04X\n",
                                            * __CPU_TXDES_SWAP_RDPTR);

    len += sprintf(page + len, "  DPLUS_RXDES_RDPTR    = %04X\n",
                                            * __DSLWAN_FP_RXDES_DPLUS_WRPTR);
    */

    len += sprintf(page + len, "  pre_dplus_ptr    = %04X\n", *PRE_DPLUS_PTR);

    len += sprintf(page + len, "  pre_dplus_cnt    = %04X\n", (*DM_RXPKTCNT) & 0xff);

    len += sprintf(page + len, "  dplus_ptr        = %04X\n", *DPLUS_PTR);

    len += sprintf(page + len, "  dplus_cnt        = %04X\n", *DPLUS_CNT);

    len += sprintf(page + len, "  DPLUS_RX_ON      = %d\n",  *DPLUS_RX_ON);

    len += sprintf(page + len, "  ISR_IS           = %08X\n ",  *ISR_IS);

    /*
    len += sprintf(page + len, "\nQoS Mib:\n");
    len += sprintf(page + len, "  cputx_pkts:             %u\n",   *__CPU_TO_DSLWAN_TX_PKTS);
    len += sprintf(page + len, "  cputx_bytes:            %u\n",   *__CPU_TO_DSLWAN_TX_BYTES);
    len += sprintf(page + len, "  cputx_drop_pkts:        %u\n",   *__CPU_TX_SWAPPER_DROP_PKTS);
    len += sprintf(page + len, "  cputx_drop_bytess:      %u\n",   *__CPU_TX_SWAPPER_DROP_BYTES);

    len += sprintf(page + len, "  dslwan_fp_drop_pkts:    %u\n",   *__DSLWAN_FP_SWAPPER_DROP_PKTS );
    len += sprintf(page + len, "  dslwan_fp_drop_bytes:   %u\n",   *__DSLWAN_FP_SWAPPER_DROP_BYTES );

    len += sprintf(page + len, "  dslwan_tx_qf_drop_pkts: %u (%u, %u, %u, %u, %u, %u, %u, %u)\n",
                 *__DSLWAN_TX_THRES_DROP_PKT_CNT0 +  *__DSLWAN_TX_THRES_DROP_PKT_CNT1 +  *__DSLWAN_TX_THRES_DROP_PKT_CNT2 +  *__DSLWAN_TX_THRES_DROP_PKT_CNT3 +
                 *__DSLWAN_TX_THRES_DROP_PKT_CNT4 +  *__DSLWAN_TX_THRES_DROP_PKT_CNT5 +  *__DSLWAN_TX_THRES_DROP_PKT_CNT6 +  *__DSLWAN_TX_THRES_DROP_PKT_CNT7,
                 * __DSLWAN_TX_THRES_DROP_PKT_CNT0,  *__DSLWAN_TX_THRES_DROP_PKT_CNT1,  *__DSLWAN_TX_THRES_DROP_PKT_CNT2,  *__DSLWAN_TX_THRES_DROP_PKT_CNT3,
                 * __DSLWAN_TX_THRES_DROP_PKT_CNT4,  *__DSLWAN_TX_THRES_DROP_PKT_CNT5,  *__DSLWAN_TX_THRES_DROP_PKT_CNT6,  *__DSLWAN_TX_THRES_DROP_PKT_CNT7);
    */
    *eof = 1;

    return len;
}

static int proc_write_fw(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "clear") == 0 || stricmp(p, "clean") == 0 )
    {
        * __DSLWAN_TX_THRES_DROP_PKT_CNT0 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT1 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT2 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT3 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT4 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT5 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT6 = 0;
        * __DSLWAN_TX_THRES_DROP_PKT_CNT7 = 0;

        * __CPU_TO_DSLWAN_TX_PKTS  = 0;
        * __CPU_TO_DSLWAN_TX_BYTES = 0;


        * __CPU_TX_SWAPPER_DROP_PKTS    = 0;
        * __CPU_TX_SWAPPER_DROP_BYTES   = 0;
        * __DSLWAN_FP_SWAPPER_DROP_PKTS = 0;
        * __DSLWAN_FP_SWAPPER_DROP_PKTS = 0;
    }

    return count;
}

static int INLINE is_char_in_set(char ch, char * set)
{

    while(*set) {
        if (ch == *set)
            return 1;
        set ++;
    }

    return 0;
}

static int tokenize(char * in, char * sep, char **tokens, int max_token_num, int * finished)
{
    int token_num = 0;

    * finished = 0;

    while (*in && token_num < max_token_num) {

        // escape all seperators
        while (*in && is_char_in_set(*in, sep)) {
            in ++;
        }

        // break if no more chars left
        if(! *in) {
            break;
        }

        // found a new token, remember start position
        tokens[token_num ++] = in;

        // search end of token
        in ++;
        while (*in && ! is_char_in_set(*in, sep)) {
            in ++;
        }

        if(! *in) {
            // break if no more chars left
            break;
        }
        else {
            // tokenize
            *in = '\0';
            in ++;
        }

    }

    if ( ! *in )
        * finished = 1;

    return token_num;
}

static void fwdbg_read_ethbuf(char **tokens, int token_num)
{
    unsigned int start_pg = 0;
    unsigned int print_pg_num;
    unsigned int num;
    unsigned int pg_size;
    volatile unsigned int *dbase, *cbase;
    unsigned int pnum, i;
    unsigned int *cw;


    dbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_dba * )DM_RXDB)->dbase + 0x2000 );
    cbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_cba * )DM_RXCB)->cbase + 0x2000);
    pnum = ( ( struct dmrx_cfg * )DM_RXCFG)->pnum;

    i = 0;
    start_pg = i < token_num  ? simple_strtoul(tokens[i ++ ], NULL, 0) : start_pg;
    print_pg_num = i < token_num ? simple_strtoul(tokens[i ++ ], NULL, 0) : pnum;

    start_pg %= pnum;
    print_pg_num = print_pg_num > pnum ? pnum : print_pg_num;

#if 0
    for(i = 0; i < token_num - 1; i ++){
        if(strincmp(tokens[i], "start_pg", strlen(tokens[i])) == 0){
            start_pg = simple_strtoul(tokens[i + 1], NULL, 0);
            start_pg = start_pg % pnum;
            i ++;

        }else if(strincmp(tokens[i], "pg_num", strlen(tokens[i])) == 0){
            print_pg_num = simple_strtoul(tokens[i + 1], NULL, 0);
            print_pg_num = print_pg_num <= pnum ? print_pg_num : pnum;
            i ++;
        }
    }
#endif

    pg_size =  ((( struct  dmrx_cfg * )DM_RXCFG)->psize + 1) * 32 / 4;


    printk("Share buffer data/ctrl buffer:\n\n");
    for(i = start_pg, num = 0 ; num < print_pg_num ; num ++, i = (i + 1) % pnum ) {

        struct ctrl_dmrx_2_fw * p_ctrl_dmrx;
        struct ctrl_fw_2_dsrx * p_ctrl_dsrx;
        int j = 0;

        cw = (unsigned int *)dbase + i * pg_size;

        p_ctrl_dmrx = (struct ctrl_dmrx_2_fw *) ( &cbase[i]);
        p_ctrl_dsrx = (struct ctrl_fw_2_dsrx *) ( &cbase[i]);


        for(j =0; j < pg_size; j=j+4) {

            if(j==0) {

                printk("Pg_id %2d: %08x %08x %08x %08x ", i, cw[0], cw[1], cw[2], cw[3]);
                printk("pg_val: %x, byte_off: %x, cos: %x \n", p_ctrl_dmrx->pg_val, p_ctrl_dmrx->byte_off, p_ctrl_dmrx->cos);
            }


            else if(j==4) {
                printk("          %08x %08x %08x %08x ",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                printk("bytes_cnt: %x, eof: %x \n", p_ctrl_dmrx->bytes_cnt, p_ctrl_dmrx->eof);
            }

            else if(j==12) {
                printk("          %08x %08x %08x %08x ",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                printk("pg_val: %x, byte_off: %x, acc_sel: %x\n", p_ctrl_dsrx->pg_val, p_ctrl_dsrx->byte_off,p_ctrl_dsrx->acc_sel );
            }

            else if(j==16) {
                printk("          %08x %08x %08x %08x ",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                printk("cos: %x, bytes_cnt: %x, eof: %x\n", p_ctrl_dsrx->cos, p_ctrl_dsrx->bytes_cnt,p_ctrl_dsrx->eof );
            }

            else {
                printk("          %08x %08x %08x %08x \n",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
            }

        }

            printk("\n");

    }

    return;
}

#if 0
static void fwdbg_read_cb(char **tokens, int token_num)
{
    unsigned int start_pg;
    unsigned int print_pg_num;
    unsigned int num;
    unsigned int i, is_rx = 1;
    unsigned int chan_no = 0;
    volatile unsigned int *dbase0;
    unsigned int pnum0;
    unsigned int * cw;

    for(i = 0; i < token_num; i ++){
        if(strincmp(tokens[i], "tx", strlen(tokens[i])) == 0){
            is_rx = 0;
        }else if(strincmp(tokens[i], "chan_no", strlen(tokens[i])) == 0 && i < token_num - 1){
            chan_no = simple_strtoul(tokens[i + 1], NULL, 0);
            if(chan_no >= 1){ //bare channel only 0 or 1
                chan_no = 1;
            }
            i ++;

        }else if(strincmp(tokens[i], "start_pg", strlen(tokens[i])) == 0 && i < token_num - 1){
            start_pg = simple_strtoul(tokens[i + 1], NULL, 0);
            i ++;

        }else if(strincmp(tokens[i], "pg_num", strlen(tokens[i])) == 0 && i < token_num - 1){
            print_pg_num = simple_strtoul(tokens[i + 1], NULL, 0);
            i ++;

        }
    }

    dbase0 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(0)->dbase + 0x2000);
    pnum0 = FFSM_CFG(0)->pnum;

}
#endif


static void fwdbg_clear_ethbuf(char **tokens, int token_num)
{
    uint32_t i = 0;
    uint32_t pnum, pg_num, pg_size, start_pg = 0;
    volatile unsigned int *dbase, *cbase;

    dbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_dba * )DM_RXDB)->dbase + 0x2000 );
    cbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_cba * )DM_RXCB)->cbase + 0x2000);
    pnum = ((struct dmrx_cfg * )DM_RXCFG)->pnum;
    pg_num = pnum;
    pg_size =  ((( struct  dmrx_cfg * )DM_RXCFG)->psize + 1) * 32 / 4;

    start_pg = i < token_num ? simple_strtoul(tokens[i++], NULL, 0) : start_pg;
    pg_num = i < token_num ? simple_strtoul(tokens[i++], NULL, 0) : pg_num;
    start_pg %= pnum;
    pg_num = pg_num > pnum ? pnum : pg_num;


    dbase = dbase + start_pg * pg_size;
    cbase = cbase + start_pg * pg_size;

    ppa_memset((void*)dbase, 0, pg_num * pg_size * sizeof(uint32_t));
    ppa_memset((void*)cbase, 0, pg_num * sizeof(uint32_t));

    return;

}

static void fwdbg_dump_sthdr(char **tokens, int token_num)
{
    //uint32_t i = 0;
    uint32_t pnum,pg_size,pg_idx;
    volatile unsigned int *dbase;
    struct flag_header *pfhdr;
    unsigned char *paddr;

    dbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_dba * )DM_RXDB)->dbase + 0x2000 );

    if(token_num < 1){
        fw_dbg_start("help dump_st_hdr");
        return;
    }

    pnum = ((struct dmrx_cfg * )DM_RXCFG)->pnum;
    pg_size = ((( struct  dmrx_cfg * )DM_RXCFG)->psize + 1) * 32 / 4;
    pg_idx = simple_strtoul(tokens[0], NULL, 0);
    if(pg_idx >= pnum){
        printk("Error : page NO %d is equal or bigger than total page number: %d\n", pg_idx, pnum);
        return;
    }

    pfhdr = (struct flag_header *)(dbase + pg_idx * pg_size);
    dump_flag_header(g_fwcode, pfhdr, __FUNCTION__, -1, 0);

    if(token_num > 1 && strincmp(tokens[1], "iphdr", strlen(tokens[1])) == 0){
        if(pfhdr->is_inner_ipv4){
            printk("First header (IPv4)\n");
            paddr = (unsigned char *)pfhdr;
            paddr += 8 + pfhdr->pl_byteoff + pfhdr->ip_inner_offset;
            dump_ipv4_hdr((struct iphdr *)paddr);

        }else if(pfhdr->is_inner_ipv6){
            printk("First header (IPv6)\n");
            paddr = (unsigned char *)pfhdr;
            paddr += 8 + pfhdr->pl_byteoff + pfhdr->ip_inner_offset;
            dump_ipv6_hdr((struct ipv6hdr *)paddr);
        }

        printk("------------------------------------\n");
        if(pfhdr->is_outer_ipv4){
            printk("Second header (IPv4)\n");
            paddr = (unsigned char *)pfhdr;
            paddr += 8 + pfhdr->pl_byteoff + pfhdr->ip_outer_offset;
            dump_ipv4_hdr((struct iphdr *)paddr);
        }else if(pfhdr->is_outer_ipv6){
            printk("Second header (IPv6)\n");
            paddr = (unsigned char *)pfhdr;
            paddr += 8 + pfhdr->pl_byteoff + pfhdr->ip_outer_offset;
            dump_ipv6_hdr((struct ipv6hdr *)paddr);
        }

        printk("------------------------------------\n");
    }
    return;

}

//echo dump_rg [dmrx | dsrx | SLL]
static void fwdbg_dump_rg(char **tokens, int token_num)
{
    uint32_t i;

    if(token_num < 1){
        fw_dbg_start("help dump_rg");
        return;
    }

    if(strincmp(tokens[0],"dmrx", strlen(tokens[0])) == 0){

        printk("DMRX_DBA(0x612): 0x%x\n", ((struct dmrx_dba * )DM_RXDB)->dbase);
        printk("DMRX_CBA(0x613): 0x%x\n", (( struct dmrx_cba * )DM_RXCB)->cbase);
        printk("DMRX_CFG(0x614): \n");
            printk("\tSEN: \t0x%x\n", ((struct dmrx_cfg * )DM_RXCFG)->sen);
            printk("\tTRLPG: \t0x%x\n", ((struct dmrx_cfg * )DM_RXCFG)->trlpg);
            printk("\tHDLEN:\t0x%x\n", ((struct dmrx_cfg * )DM_RXCFG)->hdlen);
            printk("\tENDIAN:\t0x%x\n", ((struct dmrx_cfg * )DM_RXCFG)->endian);
            printk("\tPSIZE:\t0x%x\n", ((struct dmrx_cfg * )DM_RXCFG)->psize);
            printk("\tPNUM:\t0x%x\n", ((struct dmrx_cfg * )DM_RXCFG)->pnum);
        printk("DMRX_PGCNT(0x615): \n");
            printk("\tPGPTR: \t0x%x\n", ((struct dmrx_pgcnt * )DM_RXPGCNT)->pgptr);
            printk("\tDSRC: \t0x%x\n", ((struct dmrx_pgcnt * )DM_RXPGCNT)->dsrc);
            printk("\tDVAL: \t0x%x\n", ((struct dmrx_pgcnt * )DM_RXPGCNT)->dval);
            printk("\tDCMD: \t0x%x\n", ((struct dmrx_pgcnt * )DM_RXPGCNT)->dcmd);
            printk("\tUPAGE: \t0x%x\n", ((struct dmrx_pgcnt * )DM_RXPGCNT)->upage);
        printk("DMRX_PKTCNT(0x616): \n");
            printk("\tDSRC: \t0x%x\n", ((struct dmrx_pktcnt * )DM_RXPKTCNT)->dsrc);
            printk("\tDCMD: \t0x%x\n", ((struct dmrx_pktcnt * )DM_RXPKTCNT)->dcmd);
            printk("\tUPKT: \t0x%x\n", ((struct dmrx_pktcnt * )DM_RXPKTCNT)->upkt);

    }else if(strincmp(tokens[0],"dsrx", strlen(tokens[0])) == 0){
        printk("DSRX_DB(0x710): 0x%x\n", ((struct dsrx_dba * )DS_RXDB)->dbase);
        printk("DSRX_DB(0x711): 0x%x\n", ((struct dsrx_cba * )DS_RXCB)->cbase);
        printk("DSRX_CFG(0x712): \n");
            printk("\tDREN: \t0x%x\n", ((struct dsrx_cfg * )DS_RXCFG)->dren);
            printk("\tENDIAN:\t0x%x\n", ((struct dsrx_cfg * )DS_RXCFG)->endian);
            printk("\tPSIZE:\t0x%x\n", ((struct dsrx_cfg * )DS_RXCFG)->psize);
            printk("\tPNUM:\t0x%x\n", ((struct dsrx_cfg * )DS_RXCFG)->pnum);
        printk("DSRX_PGCNT(0x713): \n");
            printk("\tPGPTR: \t0x%x\n", ((struct dsrx_pgcnt* )DS_RXPGCNT)->pgptr);
            printk("\tISRC: \t0x%x\n", ((struct dsrx_pgcnt* )DS_RXPGCNT)->isrc);
            printk("\tIVAL: \t0x%x\n", ((struct dsrx_pgcnt* )DS_RXPGCNT)->ival);
            printk("\tICMD: \t0x%x\n", ((struct dsrx_pgcnt* )DS_RXPGCNT)->icmd);
            printk("\tVPAGE: \t0x%x\n", ((struct dsrx_pgcnt* )DS_RXPGCNT)->upage);

    }else if(strincmp(tokens[0],"sll", strlen(tokens[0])) == 0){
        unsigned int keynum = ((struct sll_cmd1 *)SLL_CMD1)->ksize;

        printk("SLL_CMD0(0x901):\n");
            printk("\tCMD: \t0x%x\n", ((struct sll_cmd0* )SLL_CMD0)->cmd);
            printk("\tENUM: \t0x%x\n", ((struct sll_cmd0* )SLL_CMD0)->eynum);
            printk("\tEYBASE: \t0x%x\n", ((struct sll_cmd0* )SLL_CMD0)->eybase);
        printk("Key size: 0x%x\n", keynum);
        for(i = 0; i < keynum; i ++){
            printk("Key[%d]:    0x%x\n", i, *(uint32_t*)SLL_KEY(i));
        }
        printk("SLL_RESULT(0x920) \n");
            printk("\tVLD: \t0x%x\n", ((struct sll_result* )SLL_RESULT)->vld);
            printk("\tFOUND: \t0x%x\n", ((struct sll_result* )SLL_RESULT)->fo);
            printk("\tINDEX: \t0x%x\n", ((struct sll_result* )SLL_RESULT)->index);
    }else{
        fw_dbg_start("help dump_rg");
    }

    return;
}

static void fwdbg_route(char **tokens, int token_num)
{
    unsigned long addr, *paddr;
    int i;
    volatile struct rout_forward_action_tbl *action_tbl = NULL;
    int val[16], len;

    if(! token_num ){
        goto print_help;
    }

    if(strincmp(tokens[0], "help", strlen(tokens[0])) == 0){
            goto print_help;;
    }else{
            addr = simple_strtoul(tokens[0], NULL, 16);
            if(!addr){
                goto print_help;
            }
            paddr = (unsigned long *)sb_addr_to_fpi_addr_convert(addr);
            action_tbl = (struct rout_forward_action_tbl*)paddr;
    }

    for(i = 1; i < token_num - 1; i += 2){
        if(strincmp(tokens[i], "newMAC", strlen(tokens[i])) == 0){
            len = strlen(tokens[i + 1]);
            get_mac(&tokens[i + 1], &len, val);
//                      printk("new MAC: %02X.%02X.%02X.%02X:%02X:%02X\n", val[0], val[1], val[2], val[3], val[4], val[5]);
            action_tbl->new_dest_mac54 = (val[0] << 8) | val[1];
            action_tbl->new_dest_mac30 = (val[2] << 24) | (val[3] << 16) | (val[4] << 8) | val[5];

        }else if(strincmp(tokens[i], "newip", strlen(tokens[i])) == 0){
            len = strlen(tokens[i + 1]);
            get_ip_port(&tokens[i + 1], &len, val);
//                          printk("new dest: %d.%d.%d.%d:%d\n", val[0], val[1], val[2], val[3], val[4]);
            action_tbl->new_ip = (val[0] << 24) | (val[1] << 16) | (val[2] << 8) | val[3];
            action_tbl->new_port = val[4];
        }else if(strincmp(tokens[i], "routetype", strlen(tokens[i])) == 0){
            if(strincmp(tokens[i + 1], "NULL", strlen(tokens[i + 1])) == 0){
                action_tbl->rout_type = 0;

            }else if(strincmp(tokens[i + 1], "IP", strlen(tokens[i + 1])) == 0
                || strincmp(tokens[i + 1], "ROUTE", strlen(tokens[i + 1])) == 0){
                action_tbl->rout_type = 1;

            }if(strincmp(tokens[i + 1], "NAT", strlen(tokens[i + 1])) == 0){
                action_tbl->rout_type = 2;
            }if(strincmp(tokens[i + 1], "NAPT", strlen(tokens[i + 1])) == 0){
                action_tbl->rout_type = 3;
            }
        }else if(strincmp(tokens[i], "newDSCP", strlen(tokens[i])) == 0){
            action_tbl->new_dscp = simple_strtoul(tokens[i + 1], NULL, 0);
            //action_tbl->new_dscp_en = 1;

        }else if(strincmp(tokens[i], "MTUidx", strlen(tokens[i])) == 0){
            action_tbl->mtu_ix = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "VLANins", strlen(tokens[i])) == 0){
            action_tbl->in_vlan_ins = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "VLANrm", strlen(tokens[i])) == 0){
            action_tbl->in_vlan_rm = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "newDSCPen", strlen(tokens[i])) == 0){
            action_tbl->new_dscp_en = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "entry_vld", strlen(tokens[i])) == 0){
            action_tbl->entry_vld = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "TCP", strlen(tokens[i])) == 0){
            action_tbl->protocol = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "destlist", strlen(tokens[i])) == 0){
            if(strincmp(tokens[i + 1], "ETH0", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 0;
            }else if(strincmp(tokens[i + 1], "ETH1", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 1;
            }else if(strincmp(tokens[i + 1], "CPU0", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 2;
            }else if(strincmp(tokens[i + 1], "EXT_INT1", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 3;
            }else if(strincmp(tokens[i + 1], "EXT_INT2", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 4;
            }else if(strincmp(tokens[i + 1], "EXT_INT3", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 5;
            }else if(strincmp(tokens[i + 1], "DSL_WAN", strlen(tokens[i + 1])) == 0){
                action_tbl->dest_list |= 1 << 7;
            }
        }else if(strincmp(tokens[i], "PPPoEmode", strlen(tokens[i])) == 0){
            action_tbl->pppoe_mode = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "PPPoEidx", strlen(tokens[i])) == 0){
            action_tbl->pppoe_ix = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "newMACidx", strlen(tokens[i])) == 0){
            action_tbl->new_src_mac_ix = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "VCI", strlen(tokens[i])) == 0){
            action_tbl->new_in_vci = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "outvlanins", strlen(tokens[i])) == 0){
            action_tbl->out_vlan_ins = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "outvlanrm", strlen(tokens[i])) == 0){
            action_tbl->out_vlan_rm = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "outvlanidx", strlen(tokens[i])) == 0){
            action_tbl->out_vlan_ix = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "mpoatype", strlen(tokens[i])) == 0){
            action_tbl->mpoa_type = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "destqid", strlen(tokens[i])) == 0){
            action_tbl->dest_qid = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "tunnelidx", strlen(tokens[i])) == 0){
            action_tbl->tnnl_hdr_idx = simple_strtoul(tokens[i + 1], NULL, 0);

        }else if(strincmp(tokens[i], "tunnelen", strlen(tokens[i])) == 0){
            action_tbl->encap_tunnel = simple_strtoul(tokens[i + 1], NULL, 0);

        }
    }

    return;

print_help:

    printk("route address [name value] ... \n");
    printk("      newIP(PORT)       ??:??:??:??:????\n");
    printk("      newMAC:           ??:??:??:??:??:?? (HEX)\n");
    printk("      routetype:        NULL/IP/NAT/NAPT\n");
    printk("      newDSCP:          ??\n");
    printk("      newDSCPen:        0|1(disable|enable)\n");
    printk("      MTUidx:           ??\n");
    printk("      VLANins:          0|1(disable/enable)\n");
    printk("      VCI:              ???? (HEX)\n");
    printk("      VLANrm:           0|1(disable/enable)\n");
    printk("      outvlanins:       0|1(disable/enable)\n");
    printk("      outvlanidx:       ??\n");
    printk("      outvlanrm:        0|1(disable/enable)\n");
    printk("      destlist:         ETH0/ETH1/CPU0/EXT_INT/DSL_WAN?\n");
    printk("      PPPoEmode:        0|1(transparent/termination)\n");
    printk("      PPPoEidx:         ??\n");
    printk("      newMACidx:        ??\n");
    printk("      tcp:              0|1(no(UDP)/yes)\n");
    printk("      destqid:          ??\n");
    printk("      entryvld:         0|1(invalid/valid)\n");
    printk("      mpoatype:         ??\n");
    printk("      tunnelidx         ??\n");
    printk("      tunnelen          0|1(disable | enable)\n");

    return;
}

static void dump_ipv4_hdr(struct iphdr * iph)
{
    ASSERT(iph != NULL, "ipv4 hdr point is NULL");

    printk("Version         :%d\n", iph->version);
    printk("Header len      :%d\n", iph->ihl);
    printk("TOS             :%d\n", iph->tos);
    printk("Total len       :%d\n", iph->tot_len);
    printk("Id              :%d\n", iph->id);
    printk("Flags              \n");
    printk("  Don't Fragment:%d\n", iph->frag_off & (1 << 14));
    printk("  More Fragments:%d\n", iph->frag_off & (1 << 13));
    printk("Fragment offset :%d\n", iph->frag_off & ~0x0700);
    printk("TTL             :%d\n", iph->ttl);
    printk("Protocol        :%d\n", iph->protocol);
    printk("Header checksum :%d\n", iph->check);
    printk("Src IP address  :%d.%d.%d.%d\n", iph->saddr >> 24,iph->saddr >> 16 & 0xFF,
                                             iph->saddr >> 8 & 0xFF, iph->saddr & 0xFF);
    printk("Dst IP address  :%d.%d.%d.%d\n", iph->daddr >> 24,iph->daddr >> 16 & 0xFF,
                                             iph->daddr >> 8 & 0xFF, iph->daddr & 0xFF);

    return;
}

static void dump_ipv6_hdr(struct ipv6hdr *iph6)
{
    int i;
    ASSERT(iph6 != NULL, "ipv6 hdr point is NULL");

    printk("Version         :%d\n", iph6->version);
    printk("Traffic clase   :%d\n", iph6->priority << 4 | (iph6->flow_lbl[0] & 0xFF00));
    printk("Flow label      :%d\n", (iph6->flow_lbl[0] & 0xFF)<<16 | (iph6->flow_lbl[1] << 8) | iph6->flow_lbl[2]);
    printk("Payload Len     :%d\n", iph6->payload_len);
    printk("Next Header     :%d\n", iph6->nexthdr);
    printk("Hop limit       :%d\n", iph6->hop_limit);
    printk("Src Address     \n");
    for( i  = 0; i < 16; i ++){
        printk("%s%x", i == 0 ? "    " : i % 4 == 0 ? "." : "", (unsigned int)iph6->saddr.s6_addr + i);
    }
    printk("\n");
    printk("Dst Address     \n");
    for( i  = 0; i < 16; i ++){
        printk("%s%x", i == 0 ? "    " : i % 4 == 0 ? "." : "", (unsigned int)iph6->daddr.s6_addr + i);
    }
    printk("\n");

    return;
}

static void fwdbg_dump_iphdr(char **tokens, int token_num)
{
    unsigned long addr;
    unsigned long *paddr;

    if(token_num < 2){
        goto print_help;
    }

    addr = simple_strtoul(tokens[1], NULL, 16);
    if(!addr){
        goto print_help;
    }
    paddr = (unsigned long *)sb_addr_to_fpi_addr_convert(addr);

    if(strincmp(tokens[0], "ipv4", strlen(tokens[0])) == 0){
        dump_ipv4_hdr((struct iphdr *)paddr);
    }else if(strincmp(tokens[0], "ipv6", strlen(tokens[0])) == 0){
        dump_ipv6_hdr((struct ipv6hdr *)paddr);
    }else{
        goto print_help;
    }

    return;
print_help:
    fw_dbg_start("help dump_ip_hdr");
    return;
}

static void fwdbg_help(char **tokens, int token_num)
{
    const char *proc_file = "/proc/eth/tfwdbg";
    const char *cmds[] = {
            "help","route",
            "read_ethbuf", "read_cb",
            "clear_cb", "clear_ethbuf",
            "dump_st_hdr", "dump_ip_hdr","dump_rg",
            NULL,
    };

    int i;

    if(!token_num){//print commands only
        for(i = 0; cmds[i] != NULL; i ++){
            printk(cmds[i]);
            printk("\t");
            if(i % 3 == 0)
                printk("\n");
        }

        printk("\n\n");
        printk("echo help cmd > %s for details\n", proc_file);
        return;
    }

    if(strincmp(tokens[0], "read_ethbuf", strlen(tokens[0])) == 0){
        printk("echo read_ethbuf [start_pg] [print_pg_num] > %s \n", proc_file);
        printk("   :clear eth rx data and control buffer  \n");

    }else if(strincmp(tokens[0], "read_cb", strlen(tokens[0])) == 0){
        printk("echo read_rx_cb [rx|tx] [bc0|bc1] [start_pg] [print_pg_num] > %s \n", proc_file);
        printk("   :read rx or tx (default is both rx & tx, bare channel 0 & 1) cell/codeword buffer for bc0 or bc1  \n");

    }else if(strincmp(tokens[0], "clear_cb", strlen(tokens[0])) == 0){
        printk("echo clear_cb [rx|tx] [bc0|bc1] [start_pg] [clear_pg_num] > %s \n", proc_file);
        printk("   :clear rx or tx (or both) cell/codeword buffer for bc0 or bc1  \n");

    }else if(strincmp(tokens[0], "clear_ethbuf", strlen(tokens[0])) == 0){
        printk("echo clear_ethbuf [start_pg] [pg_num] > %s \n", proc_file);
        printk("   :clear eth rx data and control buffer  \n");

    }else if(strincmp(tokens[0], "dump_st_hdr", strlen(tokens[0])) == 0){
        printk("echo dump_st_hdr start_pg(Hex) [iphdr]> %s \n", proc_file);
        printk("   :dump status header of the packet in the provided page and dump ipv4/ipv6 header indicated by flag header  \n");

    }else if(strincmp(tokens[0], "dump_ip_hdr", strlen(tokens[0])) == 0){
        printk("echo dump_ip_hdr [ipv4 | ipv6] [share_buffer_address] [idx] > %s \n", proc_file);
        printk("   :dump ipv4 or ipv6 header in the given share buffer  \n");

    }else if(strincmp(tokens[0], "dump_rg", strlen(tokens[0])) == 0){
        printk("echo dump_rg [dmrx | dsrx | sll] %s \n", proc_file);
        printk("   :dump given registers  \n");

    }else if(strincmp(tokens[0], "route", strlen(tokens[0])) == 0){
        printk("echo route address [name value] %s \n",proc_file);
        printk("   :change route entry's contents \n");
        printk("   :echo route help for details \n");
    }else{
        fwdbg_help((char **)NULL, 0);
    }

}

static void fw_dbg_start(char *cmdbuf)
{
    char * tokens[32];
    int finished, token_num;
    int i;

    fw_dbg_t cmds[]={{"help",fwdbg_help},
                     {"read_ethbuf", fwdbg_read_ethbuf},
                     {"clear_ethbuf",fwdbg_clear_ethbuf},
                     {"dump_st_hdr", fwdbg_dump_sthdr},
                     {"dump_rg",fwdbg_dump_rg},
                     {"route", fwdbg_route},
                     {"dump_ip_hdr", fwdbg_dump_iphdr},
                     {NULL, NULL}};


    token_num = tokenize(cmdbuf, " \t", tokens, 32, &finished);
    if (token_num <= 0) {
        return;
    }

    for(i = 0; cmds[i].cmd != NULL; i ++){
        if(strincmp(tokens[0], cmds[i].cmd, strlen(tokens[0])) == 0){
            cmds[i].pfunc(&tokens[1],token_num - 1);
            break;
        }
    }

    if(cmds[i].cmd == NULL){
        fw_dbg_start("help");
    }

    return;
}

static int proc_write_fwdbg(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        // MOD_DEC_USE_COUNT;
        return 0;
    }

    ASSERT(p < str + 2048, "data point out of range");
    fw_dbg_start(p);


#if 0
    if ( strincmp(tokens[0], "help", 4) == 0) {
















        printk("echo read_tx_cb[0|1] [start_pg] [print_pg_num] > /proc/eth/tdbg \n");
        printk("   :read tx cell/codeword buffer for bc0 or bc1  \n");

        printk("echo us_bg_ctxt > /proc/eth/tdbg \n");
        printk("   : US - display us_bg_ctxt\n");

        printk("echo ds_bg_ctxt > /proc/eth/tdbg \n");
        printk("   : DS - display ds_bg_ctxt\n");

        printk("echo ds_bg_mib > /proc/eth/tdbg \n");
        printk("   : DS - display ds_bg_mib\n");

        printk("echo ds_link_list > /proc/eth/tdbg \n");
        printk("   : DS - check link list\n");

        printk("echo rx_bc_cfg > /proc/eth/tdbg \n");
        printk("   : read RX bear channel config\n");

        printk("echo tx_bc_cfg > /proc/eth/tdbg \n");
        printk("   : read TX bear channel config\n");

        printk("echo rx_gif_cfg [0|1|2|3] > /proc/eth/tdbg \n");
        printk("   : read RX gamma interface config\n");

        printk("echo tx_bc_cfg [0|1|2|3] > /proc/eth/tdbg \n");
        printk("   : read TX gamma interface config\n");

        printk("echo idlekeep > /proc/eth/tdbg \n");
        printk("   : read idle keep of BC0 and BC1\n");

        printk("echo set idlekeep[0|1] > /proc/eth/tdbg \n");
        printk("   : set idle keep of BC0 or BC1\n");

        printk("echo clear idlekeep[0|1] > /proc/eth/tdbg \n");
        printk("   : clear idle keep of BC0 or BC1\n");

    }
    else if ( strincmp(tokens[0], "read_ethbuf", 10) == 0 ) {

        unsigned int start_pg;
        unsigned int print_pg_num;
        unsigned int num;
        unsigned int pg_size;
        volatile unsigned int *dbase, *cbase;
        unsigned int pnum, i;
        unsigned int *cw;


        dbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_dba * )DM_RXDB)->dbase + 0x2000 );
        cbase = (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_cba * )DM_RXCB)->cbase + 0x2000);

        pnum = ( ( struct dmrx_cfg * )DM_RXCFG)->pnum;
        pg_size =  ( ( ( struct  dmrx_cfg * )DM_RXCFG)->psize + 1) * 32 / 4;

        start_pg = 0;
        print_pg_num = pnum;

        if (token_num >= 2)
            start_pg =  simple_strtoul(tokens[1], NULL, 0);
        if (token_num >= 3)
            print_pg_num =  simple_strtoul(tokens[2], NULL, 0);

        start_pg = start_pg % pnum;
        if(print_pg_num > pnum)
            print_pg_num = pnum;

        printk("PTM TX BC 0 CELL data/ctrl buffer:\n\n");
        for(i = start_pg, num = 0 ; num < print_pg_num ; num ++, i = (i + 1) % pnum ) {

            struct ctrl_dmrx_2_fw * p_ctrl_dmrx;
            struct ctrl_fw_2_dsrx * p_ctrl_dsrx;
            int j = 0;

            cw = dbase + i * pg_size;

            p_ctrl_dmrx = (struct ctrl_dmrx_2_fw *) ( &cbase[i]);
            p_ctrl_dsrx = (struct ctrl_fw_2_dsrx *) ( &cbase[i]);


            for(j =0; j < pg_size; j=j+4) {

                if(j==0) {

                    printk("Pg_id %2d: %08x %08x %08x %08x ", i, cw[0], cw[1], cw[2], cw[3]);
                    printk("pg_val: %x, byte_off: %x, cos: %x \n", p_ctrl_dmrx->pg_val, p_ctrl_dmrx->byte_off, p_ctrl_dmrx->cos);
                }


                else if(j==4) {
                    printk("          %08x %08x %08x %08x ",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                    printk("bytes_cnt: %x, eof: %x \n", p_ctrl_dmrx->bytes_cnt, p_ctrl_dmrx->eof);
                }

                else if(j==12) {
                    printk("          %08x %08x %08x %08x ",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                    printk("pkt_status: %x, flag_len: %x, byte_off: %x, acc_sel: %x\n", p_ctrl_dsrx->pkt_status, p_ctrl_dsrx->flag_len, p_ctrl_dsrx->byte_off,p_ctrl_dsrx->acc_sel );
                }

                else if(j==16) {
                    printk("          %08x %08x %08x %08x ",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                    printk("fcs_en: %x, cos: %x, bytes_cnt: %x, eof: %x\n", p_ctrl_dsrx->fcs_en, p_ctrl_dsrx->cos, p_ctrl_dsrx->bytes_cnt,p_ctrl_dsrx->eof );
                }

                else {
                    printk("          %08x %08x %08x %08x \n",  cw[j], cw[j+1], cw[j+2], cw[j+3]);
                }

                }

                printk("\n");

           }

    }

    else if ( strincmp(tokens[0], "read_tx_cb", 10) == 0 ) {

        unsigned int start_pg;
        unsigned int print_pg_num;
        unsigned int num;

        if(tokens[0][10] == '0' || tokens[0][10] == '\0') {

            volatile unsigned int *dbase0;
            unsigned int pnum0, i;
            unsigned int * cw;

            dbase0 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(0)->dbase + 0x2000);
            pnum0 = FFSM_CFG(0)->pnum;

            start_pg = 0;
            print_pg_num = pnum0;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                print_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum0;
            if(print_pg_num > pnum0)
                print_pg_num = pnum0;

            printk("PTM TX BC 0 CELL data/ctrl buffer:\n\n");
            for(i = start_pg, num = 0 ; num < print_pg_num ; num ++, i = (i + 1) % pnum0 ) {

                cw = dbase0 + i * cw_pg_size;
                printk("CW %2d: %08x \n"          , i, cw[0]);
                printk("       %08x %08x %08x %08x\n",  cw[1], cw[2], cw[3], cw[4]);
                printk("       %08x %08x %08x %08x\n",  cw[5], cw[6], cw[7], cw[8]);
                printk("       %08x %08x %08x %08x\n",  cw[9], cw[10], cw[11], cw[12]);
                printk("       %08x %08x %08x %08x\n",  cw[13], cw[14], cw[15], cw[16]);
            }

        }

        if (tokens[0][10] == '1' || tokens[0][10] == '\0'){

            volatile unsigned int *dbase1;
            unsigned int pnum1, i;
            unsigned int * cw;

            dbase1 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(1)->dbase + 0x2000);
            pnum1 = FFSM_CFG(1)->pnum;

            start_pg = 0;
            print_pg_num = pnum1;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                print_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum1;
            if(print_pg_num > pnum1)
                print_pg_num = pnum1;

            printk("PTM TX BC 1 CELL data/ctrl buffer:\n\n");
            for(i = start_pg, num = 0 ; num < print_pg_num ; num ++, i = (i + 1) % pnum1 ) {

                cw = dbase1 + i * cw_pg_size;
                printk("CW %2d: %08x \n"          , i, cw[0]);
                printk("       %08x %08x %08x %08x\n",  cw[1], cw[2], cw[3], cw[4]);
                printk("       %08x %08x %08x %08x\n",  cw[5], cw[6], cw[7], cw[8]);
                printk("       %08x %08x %08x %08x\n",  cw[9], cw[10], cw[11], cw[12]);
                printk("       %08x %08x %08x %08x\n\n",  cw[13], cw[14], cw[15], cw[16]);
            }

        }

    }

    else if ( strincmp(tokens[0], "read_rx_cb", 10) == 0 ) {

        unsigned int start_pg;
        unsigned int print_pg_num;
        unsigned int num;

        unsigned char * cwid_txt[8] = {
            "All_data",
            "End_of_frame",
            "Start_while_tx",
            "All_idle",
            "Start_while_idle",
            "All_idle_nosync",
            "Error",
            "Res"
         };

        if(tokens[0][10] == '0' || tokens[0][10] == '\0') {

            volatile unsigned int *dbase0, *cbase0;
            unsigned int pnum0, i;
            unsigned int * cw;

            dbase0 = (volatile unsigned int *)SB_BUFFER( SFSM_DBA(0)->dbase + 0x2000);
            cbase0 = (volatile unsigned int *)SB_BUFFER( SFSM_CBA(0)->cbase + 0x2000);

            pnum0 = SFSM_CFG(0)->pnum;

            start_pg = 0;
            print_pg_num = pnum0;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                print_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum0;
            if(print_pg_num > pnum0)
                print_pg_num = pnum0;

            printk("PTM RX BC 0 CELL data/ctrl buffer (pnum0 = %d):\n\n", pnum0);
            for(i = start_pg, num = 0 ; num < print_pg_num ; num ++, i = (i + 1) % pnum0 ) {

                struct PTM_CW_CTRL * p_ctrl;

                cw = dbase0 + i * cw_pg_size;
                p_ctrl = (struct PTM_CW_CTRL *) ( &cbase0[i]);

                printk("CW %2d: %08x                            ", i, cw[0]);
                printk("cwid=%d [%-16s], cwer=%d, preempt=%d, short=%d\n",
                        p_ctrl->cwid, cwid_txt[p_ctrl->cwid], p_ctrl->cwer, p_ctrl->preempt, p_ctrl->shrt);

                printk("       %08x %08x %08x %08x ", cw[1], cw[2], cw[3], cw[4]);
                printk("state=%d, bad=%d, ber=%-3d, spos=%-3d, ffbn=%-3d\n",
                        p_ctrl->state, p_ctrl->bad, p_ctrl->ber, p_ctrl->spos, p_ctrl->ffbn);

                printk("       %08x %08x %08x %08x\n",  cw[5], cw[6], cw[7], cw[8]);
                printk("       %08x %08x %08x %08x\n",  cw[9], cw[10], cw[11], cw[12]);
                printk("       %08x %08x %08x %08x\n",  cw[13], cw[14], cw[15], cw[16]);

            }

        }

        if (tokens[0][10] == '1' || tokens[0][10] == '\0'){

            volatile unsigned int *dbase1, *cbase1;
            unsigned int pnum1, i;
            unsigned int * cw;

            dbase1 = (volatile unsigned int *)SB_BUFFER( SFSM_DBA(1)->dbase + 0x2000);
            cbase1 = (volatile unsigned int *)SB_BUFFER( SFSM_CBA(1)->cbase + 0x2000);

            pnum1 = SFSM_CFG(1)->pnum;

            start_pg = 0;
            print_pg_num = pnum1;


            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                print_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum1;
            if(print_pg_num > pnum1)
                print_pg_num = pnum1;

            printk("PTM RX BC 1 CELL data/ctrl buffer:\n\n");
            for(i = start_pg, num = 0 ; num < print_pg_num ; num ++, i = (i + 1) % pnum1 ) {

                struct PTM_CW_CTRL * p_ctrl;

                cw = dbase1 + i * cw_pg_size;
                p_ctrl = (struct PTM_CW_CTRL *) ( &cbase1[i]);

                printk("CW %2d: %08x                            ", i, cw[0]);
                printk("cwid=%d [%16s], cwer=%d, preempt=%d, short=%d\n",
                        p_ctrl->cwid, cwid_txt[p_ctrl->cwid], p_ctrl->cwer, p_ctrl->preempt, p_ctrl->shrt);

                printk("       %08x %08x %08x %08x ", cw[1], cw[2], cw[3], cw[4]);
                printk("state=%d, bad=%d, ber=%-3d, spos=%-3d, ffbn=%-3d\n",
                        p_ctrl->state, p_ctrl->bad, p_ctrl->ber, p_ctrl->spos, p_ctrl->ffbn);

                printk("       %08x %08x %08x %08x\n",  cw[5], cw[6], cw[7], cw[8]);
                printk("       %08x %08x %08x %08x\n",  cw[9], cw[10], cw[11], cw[12]);
                printk("       %08x %08x %08x %08x\n",  cw[13], cw[14], cw[15], cw[16]);

            }

        }
    }
     else if ( strincmp(tokens[0], "clear_tx_cb", 11) == 0 ) {

        unsigned int start_pg;
        unsigned int clear_pg_num;
        unsigned int num;

        if(tokens[0][11] == '0' || tokens[0][11] == '\0') {
            volatile unsigned int *dbase0;
            unsigned int pnum0;

            dbase0 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(0)->dbase + 0x2000);
            pnum0 = FFSM_CFG(0)->pnum;

            start_pg = 0;
            clear_pg_num = pnum0;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum0;
            if(clear_pg_num > pnum0)
                clear_pg_num = pnum0;

            dbase0 = dbase0 + start_pg * cw_pg_size;

            memset(dbase0, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);

        }

        if (tokens[0][11] == '1' || tokens[0][11] == '\0'){

            volatile unsigned int *dbase1;
            unsigned int pnum1;

            dbase1 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(1)->dbase + 0x2000);
            pnum1 = FFSM_CFG(1)->pnum;

            start_pg = 0;
            clear_pg_num = pnum1;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum1;
            if(clear_pg_num > pnum1)
                clear_pg_num = pnum1;

            dbase1 = dbase1 + start_pg * cw_pg_size;

            memset(dbase1, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);

        }
    }

    else if ( strincmp(tokens[0], "clear_rx_cb", 11) == 0 ) {

        unsigned int start_pg;
        unsigned int clear_pg_num;

        if(tokens[0][11] == '0' || tokens[0][11] == '\0'){

            volatile unsigned int *dbase0, *cbase0;
            unsigned int pnum0;

            dbase0 = (volatile unsigned int *)SB_BUFFER( SFSM_DBA(0)->dbase + 0x2000);
            cbase0 = (volatile unsigned int *)SB_BUFFER( SFSM_CBA(0)->cbase + 0x2000);

            pnum0 = SFSM_CFG(0)->pnum;

            start_pg = 0;
            clear_pg_num = pnum0;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum0;
            if(clear_pg_num > pnum0)
                clear_pg_num = pnum0;

            dbase0 = dbase0 + start_pg * cw_pg_size;
            cbase0 = &cbase0[start_pg];

            memset(dbase0, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);
            memset(cbase0, 0,  sizeof(unsigned int ) * clear_pg_num);

        }

        if (tokens[0][11] == '1' || tokens[0][11] == '\0'){

            volatile unsigned int *dbase1, *cbase1;
            unsigned int pnum1;

            dbase1 = (volatile unsigned int *)SB_BUFFER( SFSM_DBA(1)->dbase + 0x2000);
            cbase1 = (volatile unsigned int *)SB_BUFFER( SFSM_CBA(1)->cbase + 0x2000);
            pnum1 = SFSM_CFG(1)->pnum;

            start_pg = 0;
            clear_pg_num = pnum1;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % pnum1;
            if(clear_pg_num > pnum1)
                clear_pg_num = pnum1;

            dbase1 = dbase1 + start_pg * cw_pg_size;
            cbase1 = &cbase1[start_pg];

            memset(dbase1, 0,  cw_pg_size * sizeof(unsigned int )* clear_pg_num);
            memset(cbase1, 0,  sizeof(unsigned int ) * clear_pg_num);
        }
    }

    else if ( strincmp(tokens[0], "clear_cb", 8) == 0 ) {

        unsigned int start_pg;
        unsigned int clear_pg_num;

        if(tokens[0][8] == '0' || tokens[0][8] == '\0'){

            //Clear tx_cb0
            volatile unsigned int *tx_dbase0;
            unsigned int tx_pnum0;
            volatile unsigned int *rx_dbase0, *rx_cbase0;
            unsigned int rx_pnum0;


            tx_dbase0 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(0)->dbase + 0x2000);
            tx_pnum0 = FFSM_CFG(0)->pnum;

            start_pg = 0;
            clear_pg_num = tx_pnum0;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % tx_pnum0;

            if(clear_pg_num > tx_pnum0)
                clear_pg_num = tx_pnum0;

            tx_dbase0 = tx_dbase0 + start_pg * cw_pg_size;

            memset(tx_dbase0, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);

            //Clear rx_cb0

            rx_dbase0 = (volatile unsigned int *)SB_BUFFER( SFSM_DBA(0)->dbase + 0x2000);
            rx_cbase0 = (volatile unsigned int *)SB_BUFFER( SFSM_CBA(0)->cbase + 0x2000);

            rx_pnum0 = SFSM_CFG(0)->pnum;

            start_pg = 0;
            clear_pg_num = rx_pnum0;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % rx_pnum0;
            if(clear_pg_num > rx_pnum0)
                clear_pg_num = rx_pnum0;

            rx_dbase0 = rx_dbase0 + start_pg * cw_pg_size;
            rx_cbase0 = &rx_cbase0[start_pg];

            memset(rx_dbase0, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);
            memset(rx_cbase0, 0,  sizeof(unsigned int ) * clear_pg_num);

        }

        if(tokens[0][8] == '1' || tokens[0][8] == '\0'){

            //Clear tx_cb1
            volatile unsigned int *tx_dbase1;
            unsigned int tx_pnum1;
            volatile unsigned int *rx_dbase1, *rx_cbase1;
            unsigned int rx_pnum1;


            tx_dbase1 = (volatile unsigned int *)SB_BUFFER( FFSM_DBA(1)->dbase + 0x2000);
            tx_pnum1 = FFSM_CFG(1)->pnum;

            start_pg = 0;
            clear_pg_num = tx_pnum1;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % tx_pnum1;

            if(clear_pg_num > tx_pnum1)
                clear_pg_num = tx_pnum1;

            tx_dbase1 = tx_dbase1 + start_pg * cw_pg_size;

            memset(tx_dbase1, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);

            rx_dbase1 = (volatile unsigned int *)SB_BUFFER( SFSM_DBA(1)->dbase + 0x2000);
            rx_cbase1 = (volatile unsigned int *)SB_BUFFER( SFSM_CBA(1)->cbase + 0x2000);
            rx_pnum1 = SFSM_CFG(1)->pnum;

            start_pg = 0;
            clear_pg_num = rx_pnum1;

            if (token_num >= 2)
                start_pg =  simple_strtoul(tokens[1], NULL, 0);
            if (token_num >= 3)
                clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

            start_pg = start_pg % rx_pnum1;
            if(clear_pg_num > rx_pnum1)
                clear_pg_num = rx_pnum1;

            rx_dbase1 = rx_dbase1 + start_pg * cw_pg_size;
            rx_cbase1 = &rx_cbase1[start_pg];

            memset(rx_dbase1, 0,  cw_pg_size * sizeof(unsigned int ) * clear_pg_num);
            memset(rx_cbase1, 0,  sizeof(unsigned int )* clear_pg_num);

        }

    }

    else if ( strincmp(tokens[0], "clear_eth_dbuf", 14) == 0) {
        volatile u32 *p_dbase;
        unsigned int pg_size = 0;
        unsigned int pg_num  = 0;
        unsigned int start_pg;
        unsigned int clear_pg_num;

        pg_num  =  ( ( struct dmrx_cfg * )DM_RXCFG)->pnum;
        pg_size =  ( ( ( struct  dmrx_cfg * )DM_RXCFG)->psize + 1) * 32 / 4;

        p_dbase =  (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_dba * )DM_RXDB)->dbase + 0x2000 );

        start_pg = 0;
        clear_pg_num = pg_num;

        if (token_num >= 2)
            start_pg =  simple_strtoul(tokens[1], NULL, 0);
        if (token_num >= 3)
            clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

        start_pg = start_pg % pg_num;
        if(clear_pg_num > pg_num)
            clear_pg_num = pg_num;

        p_dbase = p_dbase + start_pg * pg_size;

        memset(p_dbase, 0, pg_size * sizeof(unsigned int) * clear_pg_num);

    }
    else if ( strincmp(tokens[0], "clear_eth_cbuf", 14) == 0) {
        volatile u32 *p_cbase;
        unsigned int pg_num  = 0;
        unsigned int start_pg;
        unsigned int clear_pg_num;

        pg_num  =  ( ( struct  dmrx_cfg * )DM_RXCFG )->pnum;
        p_cbase =  (volatile unsigned int *)SB_BUFFER( ( ( struct  dmrx_cba * )DM_RXCB)->cbase + 0x2000);

        start_pg = 0;
        clear_pg_num = pg_num;

        if (token_num >= 2)
            start_pg =  simple_strtoul(tokens[1], NULL, 0);
        if (token_num >= 3)
            clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

        start_pg = start_pg % pg_num;
        if(clear_pg_num > pg_num)
            clear_pg_num = pg_num;

        p_cbase = &p_cbase[start_pg];


        memset(p_cbase, 0, sizeof(unsigned int) * clear_pg_num );
    }

    else if ( strincmp(tokens[0], "clear_ethbuf", 12) == 0)
    {
        volatile u32 *p_dbase, *p_cbase;
        unsigned int pg_size = 0;
        unsigned int pg_num  = 0;
        unsigned int start_pg;
        unsigned int clear_pg_num;

        //Clear ethernet data buffer
        pg_num  =  ( ( struct dmrx_cfg * )DM_RXCFG)->pnum;
        pg_size =  ( ( ( struct  dmrx_cfg * )DM_RXCFG)->psize + 1) * 32 / 4;

        p_dbase =  (volatile unsigned int *)SB_BUFFER( ( ( struct dmrx_dba * )DM_RXDB)->dbase +0x2000);

        start_pg = 0;
        clear_pg_num = pg_num;

        if (token_num >= 2)
            start_pg =  simple_strtoul(tokens[1], NULL, 0);
        if (token_num >= 3)
            clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

        start_pg = start_pg % pg_num;
        if(clear_pg_num > pg_num)
            clear_pg_num = pg_num;

        p_dbase = p_dbase + start_pg * pg_size;

        memset(p_dbase, 0, pg_size * sizeof(unsigned int) * clear_pg_num);

        //Clear ethernet control buffer
        pg_num  =  ( ( struct  dmrx_cfg * )DM_RXCFG )->pnum;
        p_cbase =  (volatile unsigned int *)SB_BUFFER( ( ( struct  dmrx_cba * )DM_RXCB)->cbase + 0x2000 );

        start_pg = 0;
        clear_pg_num = pg_num;

        if (token_num >= 2)
            start_pg =  simple_strtoul(tokens[1], NULL, 0);
        if (token_num >= 3)
            clear_pg_num =  simple_strtoul(tokens[2], NULL, 0);

        start_pg = start_pg % pg_num;
        if(clear_pg_num > pg_num)
            clear_pg_num = pg_num;

        p_cbase = &p_cbase[start_pg];

        memset(p_cbase, 0, sizeof(unsigned int) * clear_pg_num );

    }

    else if ( strincmp(tokens[0], "us_bg_ctxt", 10) == 0 )  {
        int k = 0;
        int pkt_status = 0;
        int des_addr = 0;
        int des_index = 0;
        int q_no = 0;

        for(; k<4; k++) {
            printk( "US_BG_CTXT[%d]\n", k);
            printk( "         sid: 0x%04x     sop: %x      eop: %x  pkt_status: %x    des_addr: 0x%04x \n",
                US_BG_CTXT_BASE(k)->sid, US_BG_CTXT_BASE(k)->sop, US_BG_CTXT_BASE(k)->eop,
                US_BG_CTXT_BASE(k)->pkt_status, US_BG_CTXT_BASE(k)->des_addr);
            printk( "    data_ptr: 0x%08x qid: %x   gif_id: %x     rem_len: %-4u \n\n",
                US_BG_CTXT_BASE(k)->data_ptr, US_BG_CTXT_BASE(k)->qid, US_BG_CTXT_BASE(k)->gif_id,
                US_BG_CTXT_BASE(k)->rem_len);

            pkt_status = US_BG_CTXT_BASE(k)->pkt_status;

            if(pkt_status == 1) {
                des_addr = US_BG_CTXT_BASE(k)->des_addr;
                des_index = des_addr - 0x5c00;

                while((des_index-0x80)>=0)
                {
                    des_index-=0x80;
                    q_no ++;
                }

                printk("    ==>US_BOND_PKT_DES_BASE[%d]\n", q_no);
                printk("              own: %x         c: %x        sop: %x           eop: %x           byte_off: %d \n",
                    US_BOND_PKT_DES_BASE(q_no)->own,US_BOND_PKT_DES_BASE(q_no)->c,
                    US_BOND_PKT_DES_BASE(q_no)->sop,US_BOND_PKT_DES_BASE(q_no)->eop,
                    US_BOND_PKT_DES_BASE(q_no)->byte_off);
                printk("        last_frag: %x  frag_num: %-2d  data_len: %-4d   data_ptr: 0x%08x\n\n",
                    US_BOND_PKT_DES_BASE(q_no)->last_frag,US_BOND_PKT_DES_BASE(q_no)->frag_num,
                    US_BOND_PKT_DES_BASE(q_no)->data_len,US_BOND_PKT_DES_BASE(q_no)->data_ptr);

           }

        }
    }
    else if ( strincmp(tokens[0], "ds_bg_ctxt", 10) == 0 )   {
        int k = 0;
        for(; k<4; k++) {
            printk( "DS_BG_CTXT[%d]\n", k);
            printk( "       link_state_chg: %u        expected_sid: 0x%04x     last_sop: %u      last_eop: %u\t  bg_pkt_state: %u\n",
                DS_BG_CTXT_BASE(k)->link_state_chg, DS_BG_CTXT_BASE(k)->expected_sid, DS_BG_CTXT_BASE(k)->last_sop,
                DS_BG_CTXT_BASE(k)->last_eop, DS_BG_CTXT_BASE(k)->bg_pkt_state);
            printk( "        overfrag_flag: %u       oversize_flag: %u       no_eop_flag: %u   no_sop_flag: %u\t   no_err_flag: %u \n",
                DS_BG_CTXT_BASE(k)->overfrag_flag, DS_BG_CTXT_BASE(k)->oversize_flag, DS_BG_CTXT_BASE(k)->no_eop_flag,
                DS_BG_CTXT_BASE(k)->no_sop_flag, DS_BG_CTXT_BASE(k)->no_err_flag);
            printk( "    curr_pkt_frag_cnt: %-3d curr_pkt_byte_cnt: %-4u \n\n",
                DS_BG_CTXT_BASE(k)->curr_pkt_frag_cnt, DS_BG_CTXT_BASE(k)->curr_pkt_byte_cnt);
        }
    }
    else if ( strincmp(tokens[0], "ds_bond_gif_mib", 15) == 0 ) {
        int k = 0;
        for(; k<8; k++) {
            printk( "DS_BOND_GIF_MIB[%d]\n", k);
            printk( "       total_rx_frag_cnt: %-10u total_rx_byte_cnt: %-10u overflow_frag_cnt: %-10u overflow_byte_cnt: %-10u\n",
                DS_BOND_GIF_MIB_BASE(k)->total_rx_frag_cnt, DS_BOND_GIF_MIB_BASE(k)->total_rx_byte_cnt,
                DS_BOND_GIF_MIB_BASE(k)->overflow_frag_cnt, DS_BOND_GIF_MIB_BASE(k)->overflow_byte_cnt);
            printk( "   out_of_range_frag_cnt: %-10u  missing_frag_cnt: %-10u  timeout_frag_cnt: %-10u\n\n",
                DS_BOND_GIF_MIB_BASE(k)->out_of_range_frag_cnt, DS_BOND_GIF_MIB_BASE(k)->missing_frag_cnt,
                DS_BOND_GIF_MIB_BASE(k)->timeout_frag_cnt);
        }
    }
    else if ( strincmp(tokens[0], "ds_bg_mib", 9) == 0 ) {
        int k = 0;
        for(; k<4; k++) {
            printk( "DS_BG_MIB[%d]\n", k);
            printk( "     conform_pkt_cnt: %-10u  conform_frag_cnt: %-10u conform_byte_cnt: %-10u    no_sop_pkt_cnt: %-10u   no_sop_frag_cnt: %-10u\n",
                DS_BG_MIB_BASE(k)->conform_pkt_cnt, DS_BG_MIB_BASE(k)->conform_frag_cnt,
                DS_BG_MIB_BASE(k)->conform_byte_cnt, DS_BG_MIB_BASE(k)->no_sop_pkt_cnt, DS_BG_MIB_BASE(k)->no_sop_frag_cnt);
            printk( "     no_sop_byte_cnt: %-10u    no_eop_pkt_cnt: %-10u  no_eop_frag_cnt: %-10u   no_eop_byte_cnt: %-10u  oversize_pkt_cnt: %-10u\n",
                DS_BG_MIB_BASE(k)->no_sop_byte_cnt, DS_BG_MIB_BASE(k)->no_eop_pkt_cnt, DS_BG_MIB_BASE(k)->no_eop_frag_cnt,
                DS_BG_MIB_BASE(k)->no_eop_byte_cnt, DS_BG_MIB_BASE(k)->oversize_pkt_cnt);
            printk( "   oversize_frag_cnt: %-10u oversize_byte_cnt: %-10u overfrag_pkt_cnt: %-10u overfrag_frag_cnt: %-10u overfrag_byte_cnt: %-10u\n\n",
                DS_BG_MIB_BASE(k)->oversize_frag_cnt, DS_BG_MIB_BASE(k)->oversize_byte_cnt, DS_BG_MIB_BASE(k)->overfrag_pkt_cnt,
                DS_BG_MIB_BASE(k)-> overfrag_frag_cnt, DS_BG_MIB_BASE(k)->overfrag_byte_cnt);
        }
    }
    else if ( strincmp(tokens[0], "ds_link_list", 12) == 0 ) {
        unsigned int i;
        unsigned char buf[8];
        unsigned int total_des_num = 0;

        check_ds_ll_list(DS_BOND_FREE_LL_CTXT_BASE,    "Free: ");
        for( i = 0; i < 8; i ++ ) {
            sprintf(buf, "Gif%d: ", i);
            check_ds_ll_list(DS_BOND_GIF_LL_CTXT_BASE(i), buf);
        }

        for(i = 0; i < 4; i ++) {
            sprintf(buf, "BgR%d: ", i);
            check_ds_ll_list( (volatile struct DS_BOND_GIF_LL_CTXT *)
                                    ( (unsigned int *) (DS_BG_CTXT_BASE(i))  + 4 ), buf);
        }

        total_des_num = DS_BOND_FREE_LL_CTXT_BASE->des_num;
        for( i = 0; i < 8; i ++ ) {
            total_des_num += DS_BOND_GIF_LL_CTXT_BASE(i)->des_num;
        }
        for(i = 0; i < 4; i ++) {
            total_des_num +=   ( (volatile struct DS_BOND_GIF_LL_CTXT *)
                                 ( (unsigned int *) (DS_BG_CTXT_BASE(i))  + 4 )) ->des_num;
        }

        if (total_des_num != 256) {
            printk("[X] ");
        }
        else {
            printk("[Y] ");
        }
        printk("total_des_num = %d\n\n", total_des_num);
    }
    else if ( strincmp(tokens[0], "rx_bc_cfg", 9) == 0 )  {

        unsigned int bc_s = 0;
        unsigned int bc_e = 1;
        int i = 0;
        unsigned char * ls_txt[] = {
            "Looking",
            "Freewheel Sync False",
            "Synced",
            "Freewheel Sync True"
        };
        unsigned char * rs_txt[] = {
            "Out-of-Sync",
            "Synced"
        };

        if(tokens[0][10] == '\0' && token_num == 1) {
            bc_s = 0;
            bc_e = 1;
        }
        else if ( tokens[0][10] <= '1' && tokens[0][10] >= '0'  ) {
            bc_s = bc_e = tokens[0][10] - '0';
        }
        else if ( token_num >= 2 && tokens[1][0] <= '1' && tokens[1][0] >= '0'  ) {
            bc_s = bc_e = tokens[1][0] - '0';
        }

        for ( i = bc_s; i <= bc_e; i ++ ) {
            printk("TX_GIF_CFG[%d]\n", i);
            printk("    local_state: %d [ %-20s ] remote_state: %d [ %-12s ]  rx_cw_rdptr:%d \n\n",
                   RX_BC_CFG(i)->local_state, ls_txt[RX_BC_CFG(i)->local_state],
                   RX_BC_CFG(i)->remote_state, rs_txt[RX_BC_CFG(i)->remote_state],
                   RX_BC_CFG(i)->rx_cw_rdptr);
        }

    }
    else if ( strincmp(tokens[0], "tx_bc_cfg", 9) == 0 )  {
    }
    else if ( strincmp(tokens[0], "rx_gif_cfg", 10) == 0 )  {
    }
    else if ( strincmp(tokens[0], "tx_gif_cfg", 10) == 0 )  {
        unsigned int gif_s = 0;
        unsigned int gif_e = 0;
        int i = 0;
        if(tokens[0][10] == '\0' && token_num == 1) {
            gif_s = 0;
            gif_e = 3;
        }
        else if ( tokens[0][10] <= '3' && tokens[0][10] >= '0'  ) {
            gif_s = gif_e = tokens[0][10] - '0';
        }
        else if ( token_num >= 2 && tokens[1][0] <= '3' && tokens[1][0] >= '0'  ) {
            gif_s = gif_e = tokens[1][0] - '0';
        }

        for ( i = gif_s; i <= gif_e; i ++ ) {
            printk("TX_GIF_CFG[%d]\n", i);
            printk("    curr_qid: %d     fill_pkt_state: %d      post_pkt_state: %d  curr_pdma_context_ptr: 0x%04x\n"
                   "     des_qid: %d           des_addr: 0x%04x                     curr_sar_context_ptr: 0x%04x\n"
                   "    rem_data: %-4d         rem_crc: %d          rem_fh_len: %d\n"
                   "     des_dw0: 0x%08x   des_dw1: 0x%08x  des_bp_dw: 0x%08x\n\n",
                   TX_GAMMA_ITF_CFG(i)->curr_qid, TX_GAMMA_ITF_CFG(i)->fill_pkt_state, TX_GAMMA_ITF_CFG(i)->post_pkt_state, TX_GAMMA_ITF_CFG(i)->curr_pdma_context_ptr,
                   TX_GAMMA_ITF_CFG(i)->des_qid,  TX_GAMMA_ITF_CFG(i)->des_addr, TX_GAMMA_ITF_CFG(i)->curr_sar_context_ptr,
                   TX_GAMMA_ITF_CFG(i)->rem_data, TX_GAMMA_ITF_CFG(i)->rem_crc, TX_GAMMA_ITF_CFG(i)->rem_fh_len,
                   TX_GAMMA_ITF_CFG(i)->des_dw0, TX_GAMMA_ITF_CFG(i)->des_dw1, TX_GAMMA_ITF_CFG(i)->des_bp_dw);

        }

    }
    else if ( strincmp(tokens[0], "idlekeep", 8) == 0 ) {
        printk("BC0.idlekp = %d, BC1.idlekp = %d\n",
            (*SFSM_CFG0 >> 15) & 1, (*SFSM_CFG1 >> 15) & 1);
    }
    else if ( strincmp(tokens[0], "clear", 5) == 0 ) {
        if(token_num >= 2) {
            if(strincmp(tokens[1], "idlekeep", 8) == 0) {
                if(tokens[1][8] == '\0' || tokens[1][8] == '0')
                    *SFSM_CFG0 = ( *SFSM_CFG0 ) & ( ~ ( 1 << 15) );
                if(tokens[1][8] == '\0' || tokens[1][8] == '1')
                    *SFSM_CFG1 = ( *SFSM_CFG1 ) & ( ~ ( 1 << 15) );
            }
        }
    }
    else if ( strincmp(tokens[0], "set", 3) == 0 ) {
    }


    // MOD_DEC_USE_COUNT;
#endif

    return count;
}

#endif

#define RX_UNICAST_PCKNT                0x00
#define RX_BROADCAST_PCKNT              0x01
#define RX_MULTICAST_PCKNT              0x02
#define RX_CRC_ERR_PCKNT                0x03
#define RX_UNDERSIZE_GOOD_PCKNT         0x04
#define RX_OVER_SIZE_GOOD_PCKNT         0x05
#define RX_UNDERSIZE_ERR_PCKNT          0x06
#define RX_GOOD_PAUSE_PCKNT             0x07
#define RX_OVER_SIZE_ERR_PCKNT          0x08
#define RX_ALLIGN_ERR_PCKNT             0x09
#define RX_FILTERED_PCKNT               0x0A

#define RX_SIZE_64B_PCKNT               0x0B
#define RX_SIZE_65_127B_PCKNT           0x0C
#define RX_SIZE_128_255B_PCKNT          0x0D
#define RX_SIZE_256_511B_PCKNT          0x0E
#define RX_SIZE_512_1023B_PCKNT         0x0F
#define RX_SIZE_1024B_MAX_PCKNT         0x10

#define TX_UNICAST_PCKNT                0x11
#define TX_BROADCAST_PCKNT              0x12
#define TX_MULTICAST_PCKNT              0x13

#define TX_SINGLE_COLLISION_CNT         0x14
#define TX_MULTIPLE_COLLISION_CNT       0x15
#define TX_LATE_COLLISION_CNT           0x16
#define TX_EXCESSIVE_COLLISION_CNT      0x17
#define TX_COLLISION_CNT                0x18
#define TX_PAUSE_PCKNT                  0x19

#define TX_SIZE_64B_PCKNT               0x1A
#define TX_SIZE_65_127B_PCKNT           0x1B
#define TX_SIZE_128_255B_PCKNT          0x1C
#define TX_SIZE_256_511B_PCKNT          0x1D
#define TX_SIZE_512_1023B_PCKNT         0x1E
#define TX_SIZE_1024B_MAX_PCKNT         0x1F


#define TX_DROP_PCKNT                   0x20
#define RX_DROP_PCKNT                   0x21

#define RX_GOOD_BYTE_CNT_LOW            0x23
#define RX_GOOD_BYTE_CNT_HIGH           0x22

#define RX_BAD_BYTE_CNT_LOW             0x25
#define RX_BAD_BYTE_CNT_HIGH            0x24

#define TX_GOOD_BYTE_CNT_LOW            0x27
#define TX_GOOD_BYTE_CNT_HIGH           0x26

static INLINE unsigned long amazon_s_switch_get_rmon_counter_value(int port, int off)
{
    unsigned int val = 0;
    unsigned long counter;

    if ( port < 0 || port >=  3 )
    {
        err("Error !!!  Port:%d is not in limit\n", port);
        return -1;
    }
    while ( (*AMAZON_S_SW_RMON_CTL & 0x800) );
    /* read the specific port+offset counter from RMON counter */
    val = (port << 6) | 0x800 | off;
    *AMAZON_S_SW_RMON_CTL = val;
    while ( (*AMAZON_S_SW_RMON_CTL & 0x800) );
    /* get counters ! */
    counter = *AMAZON_S_SW_RMON_ST;

    return counter;
}

static INLINE void amazon_s_switch_clr_rmon_counter_value(int port)
{
    u32 val;

    while ( (*AMAZON_S_SW_RMON_CTL & 0x800) );
    /* renew the specific port RMON counter */
    if ( port >= 0 )
        val = (port << 6) | 0x800 | (0x2 << 9);
    else
        val = 0x800 | (0x03 << 9);
    *AMAZON_S_SW_RMON_CTL = val;
    while ( (*AMAZON_S_SW_RMON_CTL & 0x800) );
}

struct mac_table_entry
{
    int     fReset;
    int     nFid;
    int     fStatic;
    int     nAgeTimer;
    u32     uPortMap;
    u8      uMac[8];
};

static INLINE int amazon_s_switch_get_mac_table_entry (struct mac_table_entry *pEntry)
{
    unsigned int reg_value;

    ASSERT(pEntry != NULL, "pEntry is NULL");

    /* check if access engine is available */
    while ( (*AMAZON_S_SW_ADR_TB_ST2 & (1 << 31)) );

    if ( pEntry->fReset )
    {
        *AMAZON_S_SW_ADR_TB_CTL2 = (3 << 20) | (0 << 16);   //  Init search pointer to first address

        //  wait for command completion and check result
        do
        {
            udelay(1);
        } while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );

        if ( (reg_value & (7 << 28)) )
            return -1;
    }

    do
    {
        *AMAZON_S_SW_ADR_TB_CTL1 = (pEntry->nFid & 0x03) << 16;
        *AMAZON_S_SW_ADR_TB_CTL2 = (2 << 20) | (0x0A << 16);  //  search by forwarding group

        while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );

        if ( (reg_value & (7 << 28)) || (reg_value & (1 << 14)) )
            return -1;

    } while ( !(reg_value & (1 << 13)) );

    pEntry->fStatic     = (reg_value >> 12) & 0x01;
    pEntry->nAgeTimer   = !pEntry->fStatic ? (reg_value & 0x07FF) : 0;
    reg_value = *AMAZON_S_SW_ADR_TB_ST1;
    pEntry->nFid        = (reg_value >> 16) & 0x03;
    pEntry->uPortMap    = (reg_value >> 20) & 0xFF;
    pEntry->uMac[0]     = (reg_value >> 8) & 0xFF;
    pEntry->uMac[1]     = reg_value & 0xFF;
    reg_value = *AMAZON_S_SW_ADR_TB_ST0;
    pEntry->uMac[2]     = (reg_value >> 24) & 0xFF;
    pEntry->uMac[3]     = (reg_value >> 16) & 0xFF;
    pEntry->uMac[4]     = (reg_value >> 8) & 0xFF;
    pEntry->uMac[5]     = reg_value & 0xFF;

    return 0;
}

static INLINE int read_port_counters(int i, char *buf)
{
    int len = 0;
    unsigned int counter;
    unsigned long long  byteCnt;

    len += sprintf(buf+len, "\n\tPort [%d] counters\n\n", i);

    len += sprintf(buf+len, "Rx Unicast PckCnt   :");
    counter = amazon_s_switch_get_rmon_counter_value(i,RX_UNICAST_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Rx Broadcast PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_BROADCAST_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Rx Multicast PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_MULTICAST_PCKNT );
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Tx Unicase PckCnt   :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_UNICAST_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Tx Broadcast PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_BROADCAST_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Tx Multicase PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_MULTICAST_PCKNT );
    len += sprintf(buf+len, "0x%08x\n",counter );


    len += sprintf(buf+len, "Rx 64B PckCnt       :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_SIZE_64B_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Rx [65-127B] PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_SIZE_65_127B_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Rx [128~255B] PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_SIZE_128_255B_PCKNT );
    len += sprintf(buf+len, "0x%08x\n",counter );
    len += sprintf(buf+len, "Rx [256~511B] PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_SIZE_256_511B_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Rx [512~1023B]PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_SIZE_512_1023B_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Rx [ >1024B] PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_SIZE_1024B_MAX_PCKNT );
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Tx [64B] PckCnt     :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SIZE_64B_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Tx [65~127B] PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SIZE_65_127B_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Tx [128~255B] PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SIZE_128_255B_PCKNT);
    len += sprintf(buf+len, "0x%08x\n",counter );
    len += sprintf(buf+len, "Tx [256~511B] PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SIZE_256_511B_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Tx [512~1023B]PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SIZE_512_1023B_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );
    len += sprintf(buf+len, "Tx [>1024B] PckCnt  :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SIZE_1024B_MAX_PCKNT);
    len += sprintf(buf+len, "0x%08x\n",counter );


    len += sprintf(buf+len, "Rx CRC err PckCnt   :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_CRC_ERR_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Rx Unsize err PCkCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_UNDERSIZE_ERR_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Rx Ovsize err PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_OVER_SIZE_ERR_PCKNT);
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Rx UnsizeGood PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_UNDERSIZE_GOOD_PCKNT );
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Rx OvsizeGood PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_OVER_SIZE_GOOD_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Rx Good Pause PckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_GOOD_PAUSE_PCKNT);
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Rx Align err PckCnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_ALLIGN_ERR_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Rx filterd errPckCnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_FILTERED_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Tx Single col Cnt   :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_SINGLE_COLLISION_CNT);
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Tx Multiple col Cnt :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_MULTIPLE_COLLISION_CNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Tx Late col  Cnt    :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_LATE_COLLISION_CNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Tx Excessive col Cnt:");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_EXCESSIVE_COLLISION_CNT);
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Tx  collision Cnt   :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_COLLISION_CNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Tx  Pause Cnt       :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_PAUSE_PCKNT);
    len += sprintf(buf+len, "0x%08x\n",counter );


    len += sprintf(buf+len, "Tx Drop PckCnt      :");
    counter =amazon_s_switch_get_rmon_counter_value (i,TX_DROP_PCKNT);
    len += sprintf(buf+len, "0x%08x\t",counter );

    len += sprintf(buf+len, "Rx Drop PckCnt      :");
    counter =amazon_s_switch_get_rmon_counter_value (i,RX_DROP_PCKNT);
    len += sprintf(buf+len, "0x%08x\n",counter );

    len += sprintf(buf+len, "Rx Good Byte Cnt    :");
    byteCnt =amazon_s_switch_get_rmon_counter_value (i,RX_GOOD_BYTE_CNT_HIGH);
    byteCnt <<=32;
    byteCnt +=amazon_s_switch_get_rmon_counter_value (i,RX_GOOD_BYTE_CNT_LOW);
    len += sprintf(buf+len, "0x%llx \t",byteCnt);

    len += sprintf(buf+len, "Tx Good Byte Cnt    :");
    byteCnt =amazon_s_switch_get_rmon_counter_value (i,TX_GOOD_BYTE_CNT_HIGH);
    byteCnt <<=32;
    byteCnt +=amazon_s_switch_get_rmon_counter_value (i,TX_GOOD_BYTE_CNT_LOW);
    len += sprintf(buf+len, "0x%llx \n",byteCnt);

    len += sprintf(buf+len, "Rx Bad Byte Cnt     :");
    byteCnt =amazon_s_switch_get_rmon_counter_value (i,RX_BAD_BYTE_CNT_HIGH);
    byteCnt <<=32;
    byteCnt +=amazon_s_switch_get_rmon_counter_value (i,RX_BAD_BYTE_CNT_LOW);
    len += sprintf(buf+len, "0x%llx \n",byteCnt);

    return len;
}

static INLINE int print_mac_table_entry(char *buf, int i, struct mac_table_entry *entry)
{
    int len = 0;
    int portmap_tmp;
    int portmap[8];
    int j;

    portmap_tmp = entry->uPortMap;
    for ( j = 0; j < 8; j++ )
    {
        portmap[7 - j] = portmap_tmp & 0x01;
        portmap_tmp >>= 1;
    }

    len += sprintf(buf + len, "%3d. ", i);
    len += sprintf(buf + len, "mac %02x:%02x:%02x:%02x:%02x:%02x, ", entry->uMac[0], entry->uMac[1], entry->uMac[2], entry->uMac[3], entry->uMac[4], entry->uMac[5]);
    len += sprintf(buf + len, "fid %d, ", entry->nFid);
    len += sprintf(buf + len, "portmap %d%d%d%d%d%d%d%d, ", portmap[0], portmap[1], portmap[2], portmap[3], portmap[4], portmap[5], portmap[6], portmap[7]);
    if ( entry->fStatic )
        len += sprintf(buf + len, "static\n");
    else
        len += sprintf(buf + len, "age %d\n", entry->nAgeTimer);

    return len;
}

static int physical_port_counters_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;
    int i;

    for ( i = 0; i < 3; i++ )
        len += read_port_counters(i, buf + len);

    *eof = 1;

    return len;
}

static int physical_port_counters_proc_write(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "clean") == 0 )
        amazon_s_switch_clr_rmon_counter_value(-1);

    return count;
}

static int port0_counters_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    len += read_port_counters(0, buf + len);

    *eof = 1;

    return len;
}

static int port0_counters_proc_write(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "clean") == 0 )
        amazon_s_switch_clr_rmon_counter_value(0);

    return count;
}

static int port1_counters_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    len += read_port_counters(1, buf + len);

    *eof = 1;

    return len;
}

static int port1_counters_proc_write(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "clean") == 0 )
        amazon_s_switch_clr_rmon_counter_value(1);

    return count;
}

static int port2_counters_proc_read(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
    int len = 0;

    len += read_port_counters(2, buf + len);

    *eof = 1;

    return len;
}

static int port2_counters_proc_write(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "clean") == 0 )
        amazon_s_switch_clr_rmon_counter_value(2);

    return count;
}

static int proc_read_mactable(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[1024];
    int llen;

    struct mac_table_entry entry = {0};
    int i;

    pstr = *start = page;

    __sync();

    i = 0;
    //for ( entry.nFid = 0; entry.nFid < 4; entry.nFid++ )
    {
        entry.fReset = 1;
        while ( !amazon_s_switch_get_mac_table_entry(&entry) )
        {
            llen = print_mac_table_entry(str, i, &entry);
            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_MACTABLE_OVERRUN_END;

            entry.fReset = 0;
            i++;
        }
    }

    *eof = 1;

PROC_READ_MACTABLE_OVERRUN_END:
    return len - off;
}

static int proc_write_mactable(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int dbg = 0;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return count;

    if ( strincmp(p, "dbg ", 4) == 0 )
    {
        dbg = 1;
        p += 4;
        while ( *p && *p <= ' ' )
            p++;
        if ( !*p )
            return count;
    }

    if ( stricmp(p, "total") == 0 )
    {
        u32 reg_value;
        u32 flag = 0;
        u32 count = 0;
        u32 max_loop = 0;
        u32 loop;
        u32 start = 0, end = 0;
        u32 start_init, end_init;
        unsigned long sys_flag;

        local_irq_save(sys_flag);
        do
        {
            start_init = read_c0_count();

            while ( (*AMAZON_S_SW_ADR_TB_ST2 & (1 << 31)) );

            *AMAZON_S_SW_ADR_TB_CTL2 = (3 << 20) | (0 << 16);

            do
            {
                udelay(1);
            } while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );

            end_init = read_c0_count();

            if ( (reg_value & (7 << 28)) )
            {
                flag = 1;
                break;
            }

            start = read_c0_count();

            do
            {
                loop = 0;

                *AMAZON_S_SW_ADR_TB_CTL1 = 0;
                *AMAZON_S_SW_ADR_TB_CTL2 = (2 << 20) | (0x0A << 16);

                while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) )
                    loop++;
                if ( max_loop < loop )
                    max_loop = loop;

                if ( (reg_value & (7 << 28)) || (reg_value & (1 << 14)) )
                {
                    if ( (reg_value & (7 << 28)) != 2 )
                        flag = 2;
                    break;
                }

                count++;
            } while ( (reg_value & (1 << 13)) );

            end = read_c0_count();

        } while ( 0 );
        local_irq_restore(sys_flag);
        printk("flag = %u, count = %u, max_loop = %u, access time = %u (init), %u (iteration)\n", flag, count, max_loop, end_init - start_init, end - start);
    }
    else if ( strincmp(p, "check ", 6) == 0 )
    {
        unsigned char mac[8] __attribute__((__aligned__(sizeof(long)))) = {0};
        int i;
        unsigned long sys_flag;
        u32 reg_value;
        u32 st0, st1;
        u32 start = 0, end = 0;
        u32 portmap_tmp, portmap[8];

        p += 6;
        while ( *p && *p <= ' ' )
            p++;

        for ( i = 2; *p && i < 8; p++ )
        {
            if ( *p >= '0' && *p <= '9' )
                mac[i] = (mac[i] << 4) | (*p - '0');
            else if ( *p >= 'a' && *p <= 'f' )
                mac[i] = (mac[i] << 4) | (*p + 10 - 'a');
            else if ( *p >= 'A' && *p <= 'F' )
                mac[i] = (mac[i] << 4) | (*p + 10 - 'A');
            else if ( *p == ':' )
                i++;
            else
                break;
        }

        local_irq_save(sys_flag);
        start = read_c0_count();
        while ( (*AMAZON_S_SW_ADR_TB_ST2 & (1 << 31)) );
        *AMAZON_S_SW_ADR_TB_CTL2 = (3 << 20) | (0 << 16);       //  init to start
        while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );
        *AMAZON_S_SW_ADR_TB_CTL0 = ((u32 *)mac)[1];
        *AMAZON_S_SW_ADR_TB_CTL1 = ((u32 *)mac)[0];
        *AMAZON_S_SW_ADR_TB_CTL2 = (2 << 20) | (0x0C << 16);    //  search by address
        while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );
        st0 = *AMAZON_S_SW_ADR_TB_ST0;
        st1 = *AMAZON_S_SW_ADR_TB_ST1;
        end = read_c0_count();
        local_irq_restore(sys_flag);
        if ( dbg )
        {
            printk("ct0 = %08x, ct1 = %08x, ct2 = %08x\n", *AMAZON_S_SW_ADR_TB_CTL0, *AMAZON_S_SW_ADR_TB_CTL1, *AMAZON_S_SW_ADR_TB_CTL2);
            printk("st0 = %08x, st1 = %08x, st2 = %08x, time = %u\n", st0, st1, reg_value, end - start);
        }
        switch ( reg_value & (7 << 28) )
        {
        case 0 << 28:   //  OK
            portmap_tmp = st1 >> 20;
            for ( i = 0; i < 8; i++ )
            {
                portmap[7 - i] = portmap_tmp & 0x01;
                portmap_tmp >>= 1;
            }
            printk("mac %02x:%02x:%02x:%02x:%02x:%02x, fid %u, portmap %u%u%u%u%u%u%u%u, ",
                    (st1 >> 8) & 0xFF, st1 & 0xFF, st0 >> 24, (st0 >> 16) & 0xFF, (st0 >> 8) & 0xFF, st0 & 0xFF,
                    (st1 >> 16) & 3,
                    portmap[0], portmap[1], portmap[2], portmap[3], portmap[4], portmap[5], portmap[6], portmap[7]);
            if ( (reg_value & (1 << 12)) )
                printk("static");
            else
                printk("age %u", reg_value & 0x07FF);
            printk(", access time = %u\n", end - start);
            break;
        case 2 << 28:   //  Entry not found
            printk("mac (%02x:%02x:%02x:%02x:%02x:%02x) not found\n", (u32)mac[2], (u32)mac[3], (u32)mac[4], (u32)mac[5], (u32)mac[6], (u32)mac[7]);
            break;
        case 3 << 28:   //  Try again
            printk("try again (got temporary state\n");
            break;
        default:
            printk("error (%d) occured\n", (reg_value >> 24) & 7);
        }
    }
    else if ( strincmp(p, "add ", 4) == 0 )
    {
        unsigned char mac[8] __attribute__((__aligned__(sizeof(long)))) = {0};
        int i;
        unsigned long sys_flag;
        u32 reg_value;
        u32 st0, st1;
        u32 start = 0, end = 0;

        p += 6;
        while ( *p && *p <= ' ' )
            p++;

        for ( i = 2; *p && i < 8; p++ )
        {
            if ( *p >= '0' && *p <= '9' )
                mac[i] = (mac[i] << 4) | (*p - '0');
            else if ( *p >= 'a' && *p <= 'f' )
                mac[i] = (mac[i] << 4) | (*p + 10 - 'a');
            else if ( *p >= 'A' && *p <= 'F' )
                mac[i] = (mac[i] << 4) | (*p + 10 - 'A');
            else if ( *p == ':' )
                i++;
            else
                break;
        }

        local_irq_save(sys_flag);
        start = read_c0_count();
        while ( (*AMAZON_S_SW_ADR_TB_ST2 & (1 << 31)) );
        *AMAZON_S_SW_ADR_TB_CTL0 = ((u32 *)mac)[1];
        *AMAZON_S_SW_ADR_TB_CTL1 = ((u32 *)mac)[0] | (1 << 20);
        *AMAZON_S_SW_ADR_TB_CTL2 = (0 << 20) | (7 << 16);       //  create new entry
        while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );
        st0 = *AMAZON_S_SW_ADR_TB_ST0;
        st1 = *AMAZON_S_SW_ADR_TB_ST1;
        end = read_c0_count();
        local_irq_restore(sys_flag);
        if ( dbg )
        {
            printk("ct0 = %08x, ct1 = %08x, ct2 = %08x\n", *AMAZON_S_SW_ADR_TB_CTL0, *AMAZON_S_SW_ADR_TB_CTL1, *AMAZON_S_SW_ADR_TB_CTL2);
            printk("st0 = %08x, st1 = %08x, st2 = %08x, time = %u\n", st0, st1, reg_value, end - start);
        }
        switch ( reg_value & (7 << 28) )
        {
        case 0 << 28:   //  OK
            printk("mac (%02x:%02x:%02x:%02x:%02x:%02x) created, access time = %u\n", (u32)mac[2], (u32)mac[3], (u32)mac[4], (u32)mac[5], (u32)mac[6], (u32)mac[7], end - start);
            break;
        case 2 << 28:   //  Entry not found
            printk("mac (%02x:%02x:%02x:%02x:%02x:%02x) not found\n", (u32)mac[2], (u32)mac[3], (u32)mac[4], (u32)mac[5], (u32)mac[6], (u32)mac[7]);
            break;
        case 3 << 28:   //  Try again
            printk("try again (got temporary state\n");
            break;
        default:
            printk("error (%d) occured\n", (reg_value >> 24) & 7);
        }
    }
    else if ( strincmp(p, "del ", 4) == 0 )
    {
        unsigned char mac[8] __attribute__((__aligned__(sizeof(long)))) = {0};
        int i;
        unsigned long sys_flag;
        u32 reg_value;
        u32 st0, st1;
        u32 start = 0, end = 0;

        p += 6;
        while ( *p && *p <= ' ' )
            p++;

        for ( i = 2; *p && i < 8; p++ )
        {
            if ( *p >= '0' && *p <= '9' )
                mac[i] = (mac[i] << 4) | (*p - '0');
            else if ( *p >= 'a' && *p <= 'f' )
                mac[i] = (mac[i] << 4) | (*p + 10 - 'a');
            else if ( *p >= 'A' && *p <= 'F' )
                mac[i] = (mac[i] << 4) | (*p + 10 - 'A');
            else if ( *p == ':' )
                i++;
            else
                break;
        }

        local_irq_save(sys_flag);
        start = read_c0_count();
        while ( (*AMAZON_S_SW_ADR_TB_ST2 & (1 << 31)) );
        *AMAZON_S_SW_ADR_TB_CTL0 = ((u32 *)mac)[1];
        *AMAZON_S_SW_ADR_TB_CTL1 = ((u32 *)mac)[0];
        *AMAZON_S_SW_ADR_TB_CTL2 = (1 << 20) | (0x0F << 16);    //  remove entry
        while ( ((reg_value = *AMAZON_S_SW_ADR_TB_ST2) & (1 << 31)) );
        st0 = *AMAZON_S_SW_ADR_TB_ST0;
        st1 = *AMAZON_S_SW_ADR_TB_ST1;
        end = read_c0_count();
        local_irq_restore(sys_flag);
        if ( dbg )
        {
            printk("ct0 = %08x, ct1 = %08x, ct2 = %08x\n", *AMAZON_S_SW_ADR_TB_CTL0, *AMAZON_S_SW_ADR_TB_CTL1, *AMAZON_S_SW_ADR_TB_CTL2);
            printk("st0 = %08x, st1 = %08x, st2 = %08x, time = %u\n", st0, st1, reg_value, end - start);
        }
        switch ( reg_value & (7 << 28) )
        {
        case 0 << 28:   //  OK
            printk("mac (%02x:%02x:%02x:%02x:%02x:%02x) deleted, access time = %u\n", (u32)mac[2], (u32)mac[3], (u32)mac[4], (u32)mac[5], (u32)mac[6], (u32)mac[7], end - start);
            break;
        case 2 << 28:   //  Entry not found
            printk("mac (%02x:%02x:%02x:%02x:%02x:%02x) not found\n", (u32)mac[2], (u32)mac[3], (u32)mac[4], (u32)mac[5], (u32)mac[6], (u32)mac[7]);
            break;
        case 3 << 28:   //  Try again
            printk("try again (got temporary state\n");
            break;
        default:
            printk("error (%d) occured\n", (reg_value >> 24) & 7);
        }
    }

    return count;
}

static int proc_read_directforwarding(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + len, "ingress direct forwarding - %s\n", g_ingress_direct_forward_enable ? "enable" : "disable");
    len += sprintf(page + len, "egress direct forwarding  - %s\n", g_egress_direct_forward_enable ? "enable" : "disable");

    *eof = 1;

    return len;
}

static int proc_write_directforwarding(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "ingress enable") == 0 )
    {
        if ( !g_ingress_direct_forward_enable )
        {
            g_ingress_direct_forward_enable = 1;
            *AMAZON_S_SW_P0_CTL |= (1 << 19);  //  enable port ingress direct forwarding
            *AMAZON_S_SW_P1_CTL |= (1 << 19);  //  enable port ingress direct forwarding
            //egress is enabled in eth_xmit
        }
    }
    else if ( stricmp(p, "egress enable") == 0 )
    {
        if ( !g_egress_direct_forward_enable )
        {
            g_egress_direct_forward_enable = 1;
            //egress is enabled in eth_xmit
        }
    }
    else if ( stricmp(p, "ingress disable") == 0 )
    {
        if ( g_ingress_direct_forward_enable )
        {
            g_ingress_direct_forward_enable = 0;
            *AMAZON_S_SW_P0_CTL &= ~(1 << 19);  //  disable port ingress direct forwarding
            *AMAZON_S_SW_P1_CTL &= ~(1 << 19);  //  disable port ingress direct forwarding
        }
    }
    else if ( stricmp(p, "egress disable") == 0 )
    {
        if ( g_egress_direct_forward_enable )
        {
            g_egress_direct_forward_enable = 0;
            //egress is enabled in eth_xmit
        }
    }
    else
    {
        printk("echo <ingress/egress> <enable/disable> > /proc/eth/direct_forwarding\n");
        printk("enable  : pure routing configuration, switch forward packets between port0/1 and cpu port directly w/o learning\n");
        printk("disable : bridging/mix configuration, switch learn MAC address and make decision on which port packet forward to\n");
    }

    return count;
}

static int proc_read_clk(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + len, (*AMAZON_S_CGU_SYS & (1 << 7)) ? "PPE clock - 250M\n" : "PPE clock - 300M\n");

    *eof = 1;

    return len;
}

static int proc_write_clk(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "250M") == 0 || stricmp(p, "250") == 0 )
    {
        *AMAZON_S_CGU_SYS |= 1 << 7;
        *AMAZON_S_CGU_UPDATE = 1;
    }
    else if ( stricmp(p, "300M") == 0 || stricmp(p, "300") == 0 )
    {
        *AMAZON_S_CGU_SYS &= ~(1 << 7);
        *AMAZON_S_CGU_UPDATE = 1;
    }
    else
    {
        printk("echo <250M/300M> > /proc/eth/pure_routing\n");
        printk("  250M - PPE clock 250MHz\n");
        printk("  300M - PPE clock 300MHz\n");
    }

    return count;
}

static int proc_read_flowcontrol(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    u16 value;

    value = smi_reg_read(0x00f5);
    if ( value == 0x0BBB )
        len += sprintf(page + len, "external tantos flow control: enable\n");
    else if ( value == 0x0AAA )
        len += sprintf(page + len, "external tantos flow control: disable\n");
    else
        len += sprintf(page + len, "external tantos flow control: value = 0x%04x\n", (u32)value);

    len += sprintf(page + len, (*AMAZON_S_SW_RGMII_CTL & 1) ? "MII0 flow control: enable\n" : "MII0 flow control: disable\n");
    len += sprintf(page + len, (*AMAZON_S_SW_RGMII_CTL & (1 << 10)) ? "MII1 flow control: enable\n" : "MII0 flow control: disable\n");

    *eof = 1;

    return len;
}

static int proc_write_flowcontrol(struct file *file, const char *buf, unsigned long count, void *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 0;
    int f_enable = 0;
    int flag = 0;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "enable") == 0 )
            f_enable = 1;
        else if ( stricmp(p1, "disable") == 0 )
            f_enable = -1;
        else if ( stricmp(p1, "help") == 0 || strcmp(p1, "?") == 0 )
        {
            f_enable = 0;
            flag = 0;
            break;
        }
        else if ( f_enable )
        {
            if ( stricmp(p1, "sw") == 0 || stricmp(p1, "tantos") == 0 || stricmp(p1, "external") == 0 || stricmp(p1, "ext") == 0 )
                flag = (flag & ~3) | (f_enable > 0 ? 3 : 2);
            else if ( stricmp(p1, "eth0") == 0 || stricmp(p1, "mii0") == 0 )
                flag = (flag & ~(3 << 2)) | ((f_enable > 0 ? 3 : 2) << 2);
            else if ( stricmp(p1, "eth1") == 0 || stricmp(p1, "mii1") == 0 )
                flag = (flag & ~(3 << 4)) | ((f_enable > 0 ? 3 : 2) << 4);
        }

        p1 = p2;
    }

    if ( !flag && f_enable )
        flag = f_enable > 0 ? 0x3F : 0x2A;

    if ( (flag & 3) )
    {
        if ( (flag & 1) )
            smi_reg_write(0x00f5, 0x0bbb);
        else
            smi_reg_write(0x00f5, 0x0aaa);
    }
    if ( (flag & (3 << 2)) )
    {
        if ( (flag & (1 << 2)) )
            *AMAZON_S_SW_RGMII_CTL |= 1;
        else
            *AMAZON_S_SW_RGMII_CTL &= ~1;
    }
    if ( (flag & (3 << 4)) )
    {
        if ( (flag & (1 << 4)) )
            *AMAZON_S_SW_RGMII_CTL |= 1 << 10;
        else
            *AMAZON_S_SW_RGMII_CTL &= ~(1 << 10);
    }

    if ( !flag )
        printk("echo <enable/disable> [sw/mii0/mii1] > /proc/eth/flowcontrol\n");

    return count;
}

static int proc_read_gphy(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    unsigned int phy_id[] = {TANTOS_CHIP_ID, PHYID_IFX_PHY11G, PHYID_LANTIQ_PHY11G, PHYID_ICPLUS_IP101A, PHYID_VSC_SIMPLIPHY};
    char *phy_name[] = {"TANTOS", "IFX_PHY11G", "LTQ_PHY11G", "IP101A", "VSC"};
    unsigned int mode_id[] = {RGMII_MODE, MII_MODE, REV_MII_MODE, RED_MII_MODE_IC, RGMII_MODE_100MB, TURBO_REV_MII_MODE, RED_MII_MODE_OC, RGMII_MODE_10MB};
    char *mode_name[] = {"RGMII", "MII", "REV_MII", "RED_MII_IC", "RGMII_100MB", "TURBO_REV_MII", "RED_MII_OC", "RGMII_10MB"};
    int linkrate[] = {10, 100, 1000, 10000};

    int len = 0;
    int i, j;
    unsigned int mdio_value;

    len += sprintf(page + len, "MII Port:\n");
    for ( i = 0; i < NUM_ENTITY(g_mii_port); i++ )
    {
        len += sprintf(page + len, "  %d: ", g_mii_port[i].port_num);
        for ( j = 0; j < NUM_ENTITY(phy_id); j++ )
            if ( g_mii_port[i].phy_id == phy_id[j] )
            {
                len += sprintf(page + len, "%s - ", phy_name[j]);
                break;
            }
        if ( j == NUM_ENTITY(phy_id) )
            len += sprintf(page + len, "%08x - ", g_mii_port[i].phy_id);
        for ( j = 0; j < NUM_ENTITY(mode_id); j++ )
            if ( g_mii_port[i].mode == mode_id[j] )
            {
                len += sprintf(page + len, "%s\n", mode_name[j]);
                break;
            }
        if ( j == NUM_ENTITY(mode_id) )
            len += sprintf(page + len, "%d\n", g_mii_port[i].mode);
    }

    len += sprintf(page + len, "MII1:\n");
    mdio_value = ((unsigned int)mdio_read(NULL, g_mii_port[1].phy_addr, 2) << 16) | mdio_read(NULL, g_mii_port[1].phy_addr, 3);
    len += sprintf(page + len, "  ID:       ");
    for ( j = 0; j < NUM_ENTITY(phy_id); j++ )
        if ( mdio_value == phy_id[j] )
        {
            len += sprintf(page + len, "%s\n", phy_name[j]);
            break;
        }
    if ( j == NUM_ENTITY(phy_id) )
        len += sprintf(page + len, "%08x\n", mdio_value);
    mdio_value = (unsigned int)mdio_read(NULL, g_mii_port[1].phy_addr, MII_BMSR);
    len += sprintf(page + len, "  link:     %s\n", (mdio_value & 0x04) ? "up" : "down");
    mdio_value = (unsigned int)mdio_read(NULL, g_mii_port[1].phy_addr, 0x18);
    len += sprintf(page + len, "  speedReg: %04x\n", mdio_value);
    len += sprintf(page + len, "  duplex:   %s\n", (mdio_value & 0x08) ? "full" : "half");
    len += sprintf(page + len, "  linkrate: %d\n", linkrate[mdio_value & 0x03]);
    mdio_value = (unsigned int)mdio_read(NULL, g_mii_port[1].phy_addr, 0x15);
    len += sprintf(page + len, "  count:    %04x\n", mdio_value);

    *eof = 1;

    return len;
}

static int proc_write_gphy(struct file *file, const char *buf, unsigned long count, void *data)
{
    char *cmd_str[] = {"rxerr", "rxact", "esderr", "ssderr", "txerr", "txact", "col", NULL, "linkdown", "linkads", "probe", "reset", "count"};
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 0;
    int cmd = NUM_ENTITY(cmd_str);

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        for ( cmd = 0; cmd < NUM_ENTITY(cmd_str); cmd++ )
            if ( cmd_str[cmd] != NULL && stricmp(p1, cmd_str[cmd]) == 0 )
            {
                if ( cmd < 10 )
                    mdio_write(NULL, g_mii_port[1].phy_addr, 0x15, cmd << 8);
                else
                    switch ( cmd )
                    {
                    case 10:
                        probe_mii_ports();
                        printk("\n");
                        break;
                    case 11:
                        reset_gphy();
                        break;
                    case 12:
                        printk("count: %04x\n", mdio_read(NULL, g_mii_port[1].phy_addr, 0x15));
                        break;
                    }
                break;
            }

        if ( cmd == NUM_ENTITY(cmd_str) )
            break;

        p1 = p2;
    }

    if ( cmd == NUM_ENTITY(cmd_str) )
        printk("echo <probe/reset/count/rxerr/rxact/esderr/ssderr/txerr/txact/col/linkdown/linkads> > /proc/eth/gphy\n");

    return count;
}

#if defined(ENABLE_NAPI) && ENABLE_NAPI
static int proc_read_napi(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + len, g_napi_enable ? "NAPI enabled\n" : "NAPI disabled\n");

    *eof = 1;

    return len;
}

static int proc_write_napi(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
    {
        return 0;
    }

    if ( stricmp(p, "enable") == 0 )
    {
        if ( !g_napi_enable )
        {
            g_dma_device->activate_poll = dma_activate_poll;
            g_napi_enable = 1;
        }
    }
    else if ( stricmp(p, "disable") == 0 )
    {
        if ( g_napi_enable )
        {
            g_dma_device->activate_poll = NULL;
            g_napi_enable = 0;
        }
    }
    else
    {
        printk("echo <enable/disable> > /proc/eth/napi\n");
    }

    return count;
}
#endif

static int proc_read_prio(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    int i, j;

    //  skb->priority to firmware queue map (for LAN interface, QId is virtual one maitained by driver)
    len += sprintf(page + len,        "Priority to Queue Map:\n");
    len += sprintf(page + len,        "  prio     :  0  1  2  3  4  5  6  7\n");
    for ( i = 0; i < 2; i++ )
    {
        len += sprintf(page + len,    "  eth%d     :", i);
        for ( j = 0; j < NUM_ENTITY(g_eth_prio_queue_map[i]); j++ )
            len += sprintf(page + len," %2d", g_eth_prio_queue_map[i][j]);
        len += sprintf(page + len,    "\n");
    }
    //  firmware queue to switch class map (this is shared by both LAN/WAN interfaces)
    len += sprintf(page + len,        "Queue to Class Map:\n");
    len += sprintf(page + len,        "  queue    :  0  1  2  3  4  5  6  7\n");
    len += sprintf(page + len,        "  eth0/1   :");
    for ( j = 0; j < NUM_ENTITY(g_wan_queue_class_map); j++ )
        len += sprintf(page + len,    " %2d", g_wan_queue_class_map[j]);
    len += sprintf(page + len,        "\n");
    //  skb->priority to switch class map (this is calculated based on aboved two tables)
    len += sprintf(page + len,        "Priority to Class Map (derived from two tables above):\n");
    len += sprintf(page + len,        "  prio     :  0  1  2  3  4  5  6  7\n");
    for ( i = 0; i < 2; i++ )
    {
        len += sprintf(page + len,    "  eth%d     :", i);
        for ( j = 0; j < NUM_ENTITY(g_eth_prio_queue_map[i]); j++ )
            len += sprintf(page + len," %2d", g_wan_queue_class_map[g_eth_prio_queue_map[i][j]]);
        len += sprintf(page + len,    "\n");
    }

    *eof = 1;

    return len;
}

static int proc_write_prio(struct file *file, const char *buf, unsigned long count, void *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 1;
    unsigned int port = ~0;
    int prio = -1;
    int queue = -1;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "help") == 0 )
        {
            printk("echo <eth0/eth1> prio xx queue xx [prio xx queue xx] > /proc/eth/prio\n");
            break;
        }
        else if ( stricmp(p1, "eth0") == 0 )
        {
            port = 0;
            prio = queue = -1;
            dbg("port = 0");
        }
        else if ( stricmp(p1, "eth1") == 0 )
        {
            port = 1;
            prio = queue = -1;
            dbg("port = 1");
        }
        else if ( stricmp(p1, "pvc") == 0 )
        {
            err("atm is not valid");
            prio = queue = -1;
        }
        else if ( port != ~0 )
        {
            if ( stricmp(p1, "p") == 0 || stricmp(p1, "prio") == 0 )
            {
                ignore_space(&p2, &len);
                prio = get_number(&p2, &len, 0);
                dbg("prio = %d", prio);
                if ( port >= 0 && port <= 1 && prio >= 0 && prio < NUM_ENTITY(g_eth_prio_queue_map[port]) )
                {
                    if ( queue >= 0 )
                        g_eth_prio_queue_map[port][prio] = queue;
                }
                else
                {
                    err("prio (%d) is out of range 0 - %d", prio, NUM_ENTITY(g_eth_prio_queue_map[port]) - 1);
                }
            }
            else if ( stricmp(p1, "q") == 0 || stricmp(p1, "queue") == 0 )
            {
                ignore_space(&p2, &len);
                queue = get_number(&p2, &len, 0);
                dbg("queue = %d", queue);
                if ( port >= 0 && port <= 1 && queue >= 0 && queue < ((g_wan_itf & (1 << port)) && g_wanqos_en ? __ETH_WAN_TX_QUEUE_NUM : __ETH_VIRTUAL_TX_QUEUE_NUM) )
                {
                    if ( prio >= 0 )
                        g_eth_prio_queue_map[port][prio] = queue;
                }
                else
                {
                    err("queue (%d) is out of range 0 - %d", queue, port == 0 ? 3 : (g_wanqos_en ? __ETH_WAN_TX_QUEUE_NUM - 1 : __ETH_VIRTUAL_TX_QUEUE_NUM - 1));
                }
            }
            else
            {
                err("unknown command (%s)", p1);
            }
        }
        else
        {
            err("unknown command (%s)", p1);
        }

        p1 = p2;
        colon = 1;
    }

    return count;
}

#ifdef CONFIG_IFX_PPA_QOS
static int proc_read_qos(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    char *pstr;
    char str[200];
    int llen;
    int i;
    struct wtx_eg_q_shaping_cfg qos_cfg;
    struct eth_wan_mib_table qos_queue_mib;
    volatile struct tx_qos_cfg tx_qos_cfg = *TX_QOS_CFG;
    volatile struct wtx_qos_q_desc_cfg qos_q_desc_cfg;
    unsigned int portid;

    pstr = *start = page;

    __sync();

    llen = sprintf(pstr, "\n  qos         : %s\n  wfq         : %s\n  Rate shaping: %s\n\n",
                    tx_qos_cfg.eth1_qss ?"enabled":"disabled",
                    tx_qos_cfg.wfq_en?"enabled":"disabled",
                    tx_qos_cfg.shape_en ?"enabled":"disabled");
    pstr += llen;
    len += llen;

    llen = sprintf(pstr, "  Ticks  =%u,    overhd    =%u,       qnum=%u  @%p\n", (u32)tx_qos_cfg.time_tick, (u32)tx_qos_cfg.overhd_bytes, (u32)tx_qos_cfg.eth1_eg_qnum, TX_QOS_CFG );
    pstr += llen;
    len += llen;

    llen = sprintf(pstr, "  PPE clk=%u MHz, basic tick=%u\n", (u32)cgu_get_pp32_clock() / 1000000, TX_QOS_CFG->time_tick / (cgu_get_pp32_clock() / 1000000));
    pstr += llen;
    len += llen;

    if ( tx_qos_cfg.eth1_eg_qnum )
    {
        if ( ifx_ppa_drv_hal_generic_hook != NULL )
        {
            PPE_QOS_RATE_SHAPING_CFG rate_cfg = {0};
            PPE_QOS_WFQ_CFG wfq_cfg = {0};

            if ( g_eth_wan_mode == 0 )
                portid = 7;
            else if ( (g_wan_itf & 0x02) )
                portid = 1;
            else if ( (g_wan_itf & 0x01) )
                portid = 0;
            else
                portid = ~0;

            llen = sprintf(pstr, "\n  Cfg :    T     R     S -->  Bit-rate(kbps)      Weight --> Level       Address       d/w      tick_cnt   b/S\n");
            pstr += llen;
            len += llen;
            for ( i = 0; i < 8; i++ )
            {
  #ifdef CONFIG_IFX_PPA_QOS_RATE_SHAPING
                rate_cfg.portid  = portid;
                rate_cfg.queueid = i;
                ifx_ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG, &rate_cfg, 0);
  #endif
  #ifdef CONFIG_IFX_PPA_QOS_WFQ
                wfq_cfg.portid  = portid;
                wfq_cfg.queueid = i;
                ifx_ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_WFQ_CFG, &wfq_cfg, 0);
  #endif
                qos_cfg = *WTX_EG_Q_SHAPING_CFG(i);

                llen = sprintf(str, "\n      %2u:  %03u  %05u  %05u   %07u            %08u   %03u        @0x%p   %08u    %03u     %05u\n", i, qos_cfg.t, qos_cfg.r, qos_cfg.s, rate_cfg.rate_in_kbps, qos_cfg.w, wfq_cfg.weight_level, WTX_EG_Q_SHAPING_CFG(i), qos_cfg.d, qos_cfg.tick_cnt, qos_cfg.b);
                if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                    goto PROC_READ_QOS_OVERRUN_END;
            }

            if ( portid < 2 )
            {
                //  PTM WAN mode does not have port based rate shaping, only ETH WAN mode has.

  #ifdef CONFIG_IFX_PPA_QOS_RATE_SHAPING
                rate_cfg.portid  = portid;
                rate_cfg.queueid = i;
                ifx_ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG, &rate_cfg, 0);
  #endif
                qos_cfg = *WTX_EG_Q_PORT_SHAPING_CFG(0);

                llen = sprintf(str, "\n    port:  %03u  %05u  %05u   %07u                                  @0x%p   %08u    %03u     %05u\n", qos_cfg.t, qos_cfg.r, qos_cfg.s, rate_cfg.rate_in_kbps, WTX_EG_Q_PORT_SHAPING_CFG(0), qos_cfg.d, qos_cfg.tick_cnt, qos_cfg.b);
                if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                    goto PROC_READ_QOS_OVERRUN_END;
            }
        }

        llen = sprintf(pstr, "\n  MIB : rx_pkt/rx_bytes         tx_pkt/tx_bytes       cpu_small_drop/cpu_drop  fast_small_drop/fast_drop_cnt\n");
        pstr += llen;
        len += llen;
        for ( i = 0; i < 8; i++ )
        {
            qos_queue_mib = *ETH_WAN_TX_MIB_TABLE(i);

            llen = sprintf(str, "    %2u: %010u/%010u  %010u/%010u  %010u/%010u  %010u/%010u  @0x%p\n", i,
                qos_queue_mib.wrx_total_pdu, qos_queue_mib.wrx_total_bytes,
                qos_queue_mib.wtx_total_pdu, qos_queue_mib.wtx_total_bytes,
                qos_queue_mib.wtx_cpu_drop_small_pdu, qos_queue_mib.wtx_cpu_drop_pdu,
                qos_queue_mib.wtx_fast_drop_small_pdu, qos_queue_mib.wtx_fast_drop_pdu,
                ETH_WAN_TX_MIB_TABLE(i));

            if ( proc_buf_copy(&pstr, count, off, &len, str, llen) )
                goto PROC_READ_QOS_OVERRUN_END;
        }

        //QOS queue descriptor
        llen = sprintf(pstr, "\n  Desc: threshold  num    base_addr  rd_ptr   wr_ptr\n");
        pstr += llen;
        len += llen;
        for ( i = 0; i < 8; i++ )
        {
            qos_q_desc_cfg = *WTX_QOS_Q_DESC_CFG(i);

            llen = sprintf(pstr, "    %2u: 0x%02x       0x%02x   0x%04x     0x%04x   0x%04x  @0x%p\n", i,
                qos_q_desc_cfg.threshold,
                qos_q_desc_cfg.length,
                qos_q_desc_cfg.addr,
                qos_q_desc_cfg.rd_ptr,
                qos_q_desc_cfg.wr_ptr,
                WTX_QOS_Q_DESC_CFG(i) );

            pstr += llen;
            len += llen;
        }
    }

    *eof = 1;

PROC_READ_QOS_OVERRUN_END:
    return len - off;
}
#endif

static int proc_read_class(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + len, "CFG_CLASS2QID_MAP (0x%08x): %08x %08x %08x %08x\n", (unsigned int)CFG_CLASS2QID_MAP(0), *CFG_CLASS2QID_MAP(0), *CFG_CLASS2QID_MAP(1), *CFG_CLASS2QID_MAP(2), *CFG_CLASS2QID_MAP(3));
    len += sprintf(page + len, "CFG_QID2CLASS_MAP (0x%08x): %08x %08x %08x %08x\n", (unsigned int)CFG_QID2CLASS_MAP(0), *CFG_QID2CLASS_MAP(0), *CFG_QID2CLASS_MAP(1), *CFG_QID2CLASS_MAP(2), *CFG_QID2CLASS_MAP(3));

    *eof = 1;

    return len;
}

static int proc_write_class(struct file *file, const char *buf, unsigned long count, void *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 1;

    int i, j;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "CFG_CLASS2QID_MAP") == 0 || stricmp(p1, "CLASS2QID") == 0 )
        {
            for ( i = 0; i < 4; i++ )
            {
                ignore_space(&p2, &len);
                if ( !len )
                    break;
                *CFG_CLASS2QID_MAP(i) = get_number(&p2, &len, 1);
            }
            break;
        }
        else if ( stricmp(p1, "CFG_QID2CLASS_MAP") == 0 || stricmp(p1, "QID2CLASS") == 0 )
        {
            for ( i = 0; i < 4; i++ )
            {
                ignore_space(&p2, &len);
                if ( !len )
                    break;
                *CFG_QID2CLASS_MAP(i) = get_number(&p2, &len, 1);
                if ( i == 0 )
                {
                    for ( j = 0; j < 8; j++ )
                        g_wan_queue_class_map[j] = (*CFG_QID2CLASS_MAP(i) >> (j * 4)) & 0x0F;
                }
            }
            break;
        }
        else
        {
            printk("echo <CLASS2QID | QID2CLASS> [hex numbers] > /proc/eth/class\n");
            break;
        }

        p1 = p2;
        colon = 1;
    }

    return count;
}

static int proc_read_mccfg(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + len, "SSC: %s, ASC: %s \n",
                  PS_MC_GENCFG3->ssc_mode ? "Enable" : "Disable",
                  PS_MC_GENCFG3->asc_mode ? "Enable" : "Disable");

    *eof = 1;

    return len;
}

static int proc_write_mccfg(struct file *file, const char *buf, unsigned long count, void *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 0;
    int val_en = -1;
    int is_ssc = -1;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    get_token(&p1, &p2, &len, &colon);
    p2 ++;
    len --;

    if ( stricmp(p1, "Enable") == 0 || stricmp(p1, "Disable") == 0 )
    {
        val_en = (stricmp(p1, "Enable") == 0) ? 1 : 0;

        p1 = p2;
        get_token(&p1, &p2, &len, &colon);

        if(len && (stricmp(p1, "asc") == 0 || stricmp(p1, "ssc") == 0)){
            is_ssc = (stricmp(p1, "ssc") == 0) ? 1 : 0;
        }

        if(is_ssc >= 0 && val_en >= 0){
            if(is_ssc){
                PS_MC_GENCFG3->ssc_mode = val_en;
            }else{
                PS_MC_GENCFG3->asc_mode = val_en;
            }
            return count;
        }
    }

    printk("echo <Enable | Disable> [ssc | asc] > /proc/eth/mccfg\n");
    return count;

}


static INLINE int proc_buf_copy(char **pbuf, int size, off_t off, int *ppos, const char *str, int len)
{
    if ( *ppos <= off && *ppos + len > off )
    {
        my_memcpy(*pbuf, str + off - *ppos, *ppos + len - off);
        *pbuf += *ppos + len - off;
    }
    else if ( *ppos > off )
    {
        my_memcpy(*pbuf, str, len);
        *pbuf += len;
    }
    *ppos += len;
    return *ppos >= off + size;
}

static INLINE int stricmp(const char *p1, const char *p2)
{
    int c1, c2;

    while ( *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
static INLINE int strincmp(const char *p1, const char *p2, int n)
{
    int c1 = 0, c2;

    while ( n && *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
        n--;
    }

    return n ? *p1 - *p2 : c1;
}
#endif

static INLINE int get_token(char **p1, char **p2, int *len, int *colon)
{
    int tlen = 0;

    while ( *len && !((**p1 >= 'A' && **p1 <= 'Z') || (**p1 >= 'a' && **p1<= 'z') || (**p1 >= '0' && **p1<= '9')) )
    {
        (*p1)++;
        (*len)--;
    }
    if ( !*len )
        return 0;

    if ( *colon )
    {
        *colon = 0;
        *p2 = *p1;
        while ( *len && **p2 > ' ' && **p2 != ',' )
        {
            if ( **p2 == ':' )
            {
                *colon = 1;
                break;
            }
            (*p2)++;
            (*len)--;
            tlen++;
        }
        **p2 = 0;
    }
    else
    {
        *p2 = *p1;
        while ( *len && **p2 > ' ' && **p2 != ',' )
        {
            (*p2)++;
            (*len)--;
            tlen++;
        }
        **p2 = 0;
    }

    return tlen;
}

static INLINE unsigned int get_number(char **p, int *len, int is_hex)
{
    unsigned int ret = 0;
    int n = 0;

    if ( (*p)[0] == '0' && (*p)[1] == 'x' )
    {
        is_hex = 1;
        (*p) += 2;
        (*len) -= 2;
    }

    if ( is_hex )
    {
        while ( *len && ((**p >= '0' && **p <= '9') || (**p >= 'a' && **p <= 'f') || (**p >= 'A' && **p <= 'F')) )
        {
            if ( **p >= '0' && **p <= '9' )
                n = **p - '0';
            else if ( **p >= 'a' && **p <= 'f' )
               n = **p - 'a' + 10;
            else if ( **p >= 'A' && **p <= 'F' )
                n = **p - 'A' + 10;
            ret = (ret << 4) | n;
            (*p)++;
            (*len)--;
        }
    }
    else
    {
        while ( *len && **p >= '0' && **p <= '9' )
        {
            n = **p - '0';
            ret = ret * 10 + n;
            (*p)++;
            (*len)--;
        }
    }

    return ret;
}

#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
static INLINE void get_ip_port(char **p, int *len, unsigned int *val)
{
    int i;
    unsigned int tmp_val[17] = {0};
    int is_ipv6 = 0;

    memset(val, 0, sizeof(*val) * 6);

    for ( i = 0; i < 4; i++ )
    {
        ignore_space(p, len);
        if ( !*len )
            break;
        val[i] = get_number(p, len, 0);
    }

    if ( **p == '.' )
    {
        is_ipv6 = 1;
        for ( i = 0; i < 16 - 4; i++ )
        {
            ignore_space(p, len);
            if ( !*len )
                break;
            tmp_val[i] = get_number(p, len, 0);
        }
    }

    ignore_space(p, len);
    if ( *len )
        val[4] = get_number(p, len, 0);

    if ( is_ipv6 )
    {
        val[5] = 6;
        for ( i = 0; i < 16 - 4; i++ )
            val[i + 6] = tmp_val[i];
    }
    else
        val[5] = 4;
}

static INLINE void get_mac(char **p, int *len, unsigned int *val)
{
    int i;

    memset(val, 0, sizeof(*val) * 6);

    for ( i = 0; i < 6; i++ )
    {
        ignore_space(p, len);
        if ( !*len )
            break;
        val[i] = get_number(p, len, 1);
    }
}
#endif

static INLINE void ignore_space(char **p, int *len)
{
    while ( *len && (**p <= ' ' || **p == ':' || **p == '.' || **p == ',') )
    {
        (*p)++;
        (*len)--;
    }
}

#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
static INLINE char *get_wanitf(int iswan)
{
    static char itfs[64];
    char *allitfs[] = {
        "ETH0",
        "ETH1",
        "", //reserve for CPU
        "EXT1",
        "EXT2",
        "EXT3",
        "EXT4",
        "EXT5",
        NULL
    };

    int wan_itf = *CFG_WAN_PORTMAP;
    int wan_mixmap = *CFG_MIXED_PORTMAP;
    int i, len = 0;

    int max_exts_idx = 5 + 2;

    memset(itfs, 0, sizeof(itfs));
    if(wan_mixmap != 0 && !iswan){ //mix mode dont have lan
        return itfs;
    }

    for(i = 0; allitfs[i] != NULL ; i ++){
        if(i == 2) continue; //skip CPU port

        if((iswan && (wan_itf & (1 << i))) || (!iswan && !(wan_itf & (1 << i)))){
            if(i > max_exts_idx){
                if(!iswan){
                    break; //DSL cannot be lan
                }
                else{
                    len += sprintf(itfs + len, "%s ", "DSL");
                    break; //DSL only listed once although in A5 we have 2 ports support DSL
                }
            }

            len += sprintf(itfs + len, "%s ", allitfs[i]);

            if(iswan && (wan_mixmap &  (1 << i))){
                len += sprintf(itfs + len, "(Mixed) ");
            }
        }
    }

    return itfs;
}

static INLINE int print_route(char *buf, int i, int f_is_lan, struct rout_forward_compare_tbl *pcompare, struct rout_forward_action_tbl *pwaction)
{
    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "EXT_INT4", "EXT_INT5"};

    int len = 0;
    u32 bit;
    int j, k;

    len += sprintf(buf + len,          "  entry %d\n", i);
    len += sprintf(buf + len,          "    compare (0x%08X)\n", (u32)pcompare);
    len += sprintf(buf + len,          "      src:  %d.%d.%d.%d:%d\n", pcompare->src_ip >> 24,  (pcompare->src_ip >> 16) & 0xFF,  (pcompare->src_ip >> 8) & 0xFF,  pcompare->src_ip & 0xFF, pcompare->src_port);
    len += sprintf(buf + len,          "      dest: %d.%d.%d.%d:%d\n", pcompare->dest_ip >> 24, (pcompare->dest_ip >> 16) & 0xFF, (pcompare->dest_ip >> 8) & 0xFF, pcompare->dest_ip & 0xFF, pcompare->dest_port);
    len += sprintf(buf + len,          "    action  (0x%08X)\n", (u32)pwaction);
    len += sprintf(buf + len,          "      new %s    %d.%d.%d.%d:%d\n", f_is_lan ? "src :" : "dest:", pwaction->new_ip >> 24, (pwaction->new_ip >> 16) & 0xFF, (pwaction->new_ip >> 8) & 0xFF, pwaction->new_ip & 0xFF, pwaction->new_port);
    len += sprintf(buf + len,          "      new MAC :    %02X:%02X:%02X:%02X:%02X:%02X\n", (pwaction->new_dest_mac54 >> 8) & 0xFF, pwaction->new_dest_mac54 & 0xFF, pwaction->new_dest_mac30 >> 24, (pwaction->new_dest_mac30 >> 16) & 0xFF, (pwaction->new_dest_mac30 >> 8) & 0xFF, pwaction->new_dest_mac30 & 0xFF);
    switch ( pwaction->rout_type )
    {
    case 1:  len += sprintf(buf + len, "      route type:  IP\n");   break;
    case 2:  len += sprintf(buf + len, "      route type:  NAT\n");  break;
    case 3:  len += sprintf(buf + len, "      route type:  NAPT\n"); break;
    default: len += sprintf(buf + len, "      route type:  NULL\n");
    }
    if ( pwaction->new_dscp_en )
        len += sprintf(buf + len,      "      new DSCP:    %d\n", pwaction->new_dscp);
    else
        len += sprintf(buf + len,      "      new DSCP:    original (not modified)\n");
    len += sprintf(buf + len,          "      MTU index:   %d\n", pwaction->mtu_ix);
    if ( pwaction->in_vlan_ins )
        len += sprintf(buf + len,      "      VLAN insert: enable, VCI 0x%04x\n", pwaction->new_in_vci);
    else
        len += sprintf(buf + len,      "      VLAN insert: disable\n");
    len += sprintf(buf + len,          "      VLAN remove: %s\n", pwaction->in_vlan_rm ? "enable" : "disable");
    if(f_is_lan){
        len += sprintf(buf + len,      "      Tunnel insert: %s\n", pwaction->encap_tunnel ? "Yes" : "No");
        if(pwaction->encap_tunnel){
            len += sprintf(buf + len,  "      Tunnel idx:    %d\n", pwaction->tnnl_hdr_idx);
        }
    }
    else
        len += sprintf(buf + len,      "      Tunnel remove: %s\n", pwaction->encap_tunnel ? "Yes" : "No");

    if ( !pwaction->dest_list )
        len += sprintf(buf + len,      "      dest list:   none\n");
    else
    {
        len += sprintf(buf + len,      "      dest list:   ");
        for ( bit = 1, j = k = 0; bit < 1 << 8; bit <<= 1, j++ )
            if ( (pwaction->dest_list & bit) )
            {
                if ( k )
                    len += sprintf(buf + len, ", ");
                len += sprintf(buf + len, dest_list[j]);
                k = 1;
            }
        len += sprintf(buf + len, "\n");
    }
    if ( pwaction->pppoe_mode )
    {
        len += sprintf(buf + len,      "      PPPoE mode:  termination\n");
        if ( f_is_lan )
            len += sprintf(buf + len,  "      PPPoE index: %d\n", pwaction->pppoe_ix);
    }
    else
        len += sprintf(buf + len,      "      PPPoE mode:  transparent\n");
    len += sprintf(buf + len,          "      new src MAC index: %d\n", pwaction->new_src_mac_ix);
    if ( pwaction->out_vlan_ins )
        len += sprintf(buf + len,      "      outer VLAN insert: enable, index %d\n", pwaction->out_vlan_ix);
    else
        len += sprintf(buf + len,      "      outer VLAN insert: disable\n");
    len += sprintf(buf + len,          "      outer VLAN remove: %s\n", pwaction->out_vlan_rm ? "enable" : "disable");
    len += sprintf(buf + len,          "      tcp:         %s\n", pwaction->protocol ? "yes" : "no (UDP)");
    len += sprintf(buf + len,          "      entry valid: %s\n", pwaction->entry_vld ? "yes" : "no");
    len += sprintf(buf + len,          "      dest qid (dslwan qid): %d\n", pwaction->dest_qid);
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    {
         if(PS_MC_GENCFG3->session_mib_unit == 1)
            len += sprintf(buf + len,          "      accl packets: %u\n", pwaction->bytes);
         else
            len += sprintf(buf + len,          "      accl bytes: %u\n", pwaction->bytes * 32);
    }

#else
    len += sprintf(buf + len,          "      accl bytes: %u\n", pwaction->bytes);
#endif
    return len;
}

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
static INLINE int print_mc(char *buf, int i, struct wan_rout_multicast_cmp_tbl *pcompare, struct wan_rout_multicast_act_tbl *paction, struct rtp_sampling_cnt *rtp_mib,uint32_t rt_mc_cnt)
#else
static INLINE int print_mc(char *buf, int i, struct wan_rout_multicast_cmp_tbl *pcompare, struct wan_rout_multicast_act_tbl *paction, uint32_t rt_mc_cnt)
#endif
{
    static const char *rout_type[] = {"no IP level editing", "TTL editing", "reserved - 2", "reserved - 3"};
    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "res", "ATM"};

    int len = 0;
    u32 bit;
    int j, k;

    len += sprintf(buf + len,          "  entry %d - %s\n", i, paction->entry_vld ? "valid" : "invalid");
    len += sprintf(buf + len,          "    compare (0x%08X)\n", (u32)pcompare);
    len += sprintf(buf + len,          "      wan_dest_ip:  %d.%d.%d.%d\n", (pcompare->wan_dest_ip >> 24) & 0xFF,  (pcompare->wan_dest_ip >> 16) & 0xFF,  (pcompare->wan_dest_ip >> 8) & 0xFF,  pcompare->wan_dest_ip & 0xFF);
    len += sprintf(buf + len,          "      wan_src_ip:   %d.%d.%d.%d\n", (pcompare->wan_src_ip >> 24) & 0xFF,  (pcompare->wan_src_ip >> 16) & 0xFF,  (pcompare->wan_src_ip >> 8) & 0xFF,  pcompare->wan_src_ip & 0xFF);
    len += sprintf(buf + len,          "    action  (0x%08X)\n", (u32)paction);
    len += sprintf(buf + len,          "      rout_type:    %s\n", rout_type[paction->rout_type]);
    if ( paction->new_dscp_en )
        len += sprintf(buf + len,      "      new DSCP:     %d\n", paction->new_dscp);
    else
        len += sprintf(buf + len,      "      new DSCP:     original (not modified)\n");
    if ( paction->in_vlan_ins )
        len += sprintf(buf + len,      "      VLAN insert:  enable, VCI 0x%04x\n", paction->new_in_vci);
    else
        len += sprintf(buf + len,      "      VLAN insert:  disable\n");
    len += sprintf(buf + len,          "      VLAN remove:  %s\n", paction->in_vlan_rm ? "enable" : "disable");
    if ( !paction->dest_list )
        len += sprintf(buf + len,      "      dest list:    none\n");
    else
    {
        len += sprintf(buf + len,      "      dest list:   ");
        for ( bit = 1, j = k = 0; bit < 1 << 8; bit <<= 1, j++ )
            if ( (paction->dest_list & bit) )
            {
                if ( k )
                    len += sprintf(buf + len, ", ");
                len += sprintf(buf + len, dest_list[j]);
                k = 1;
            }
        len += sprintf(buf + len, "\n");
    }
    len += sprintf(buf + len,          "      PPPoE mode:   %s\n", paction->pppoe_mode ? "termination" : "transparent");

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    len += sprintf(buf + len,          "      RTP sampling    : %s\n", paction->sample_en ? "enable" : "disable");
   if(paction->sample_en) 
   {
    len += sprintf(buf + len,          "      RTP packet count: %d\n", rtp_mib->pkt_cnt);
    len += sprintf(buf + len,          "      RTP seq number  : %d\n", rtp_mib->seq_no);
   }
#endif

    if ( paction->new_src_mac_en )
        len += sprintf(buf + len,      "      new src MAC index: %d\n", paction->new_src_mac_ix);
    else
        len += sprintf(buf + len,      "      new src MAC index: disabled\n");
    if ( paction->out_vlan_ins )
        len += sprintf(buf + len,      "      outer VLAN insert: enable, index %d\n", paction->out_vlan_ix);
    else
        len += sprintf(buf + len,      "      outer VLAN insert: disable\n");
    len += sprintf(buf + len,          "      outer VLAN remove: %s\n", paction->out_vlan_rm ? "enable" : "disable");
    len += sprintf(buf + len,          "      dest_qid:          %d\n", paction->dest_qid);
    len += sprintf(buf + len,          "      Tunnel Remove    : %s\n", paction->tunnel_rm ? "enable" : "disable");
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    {
         if(PS_MC_GENCFG3->session_mib_unit == 1)
            len += sprintf(buf + len,          "      acc packets      : %u\n", rt_mc_cnt);
         else
            len += sprintf(buf + len,          "      acc bytes        : %u\n", rt_mc_cnt);
    }

#else
    len += sprintf(buf + len,          "      acc bytes        : %u\n", rt_mc_cnt);
#endif
    return len;
}
#endif

#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
static INLINE void dump_skb(struct sk_buff *skb, u32 len, char *title, int port, int ch, int is_tx, int enforce)
{
    int i;

    if ( !g_dump_cnt ||( !enforce && !(g_dbg_enable & (is_tx ? DBG_ENABLE_MASK_DUMP_SKB_TX : DBG_ENABLE_MASK_DUMP_SKB_RX))) )
        return;

    if ( g_dump_cnt > 0 )
	    g_dump_cnt--;

    if ( skb->len < len )
        len = skb->len;

    if ( len > DMA_PACKET_SIZE )
    {
        printk("too big data length: skb = %08x, skb->data = %08x, skb->len = %d\n", (u32)skb, (u32)skb->data, skb->len);
        return;
    }

    if ( ch >= 0 )
        printk("%s (port %d, ch %d)\n", title, port, ch);
    else
        printk("%s\n", title);
    printk("  skb->data = %08X, skb->tail = %08X, skb->len = %d\n", (u32)skb->data, (u32)skb->tail, (int)skb->len);
    for ( i = 1; i <= len; i++ )
    {
        if ( i % 16 == 1 )
            printk("  %4d:", i - 1);
        printk(" %02X", (int)(*((char*)skb->data + i - 1) & 0xFF));
        if ( i % 16 == 0 )
            printk("\n");
    }
    if ( (i - 1) % 16 != 0 )
        printk("\n");
}
#endif

#if defined(DEBUG_DUMP_FLAG_HEADER) && DEBUG_DUMP_FLAG_HEADER
static INLINE void dump_flag_header(int fwcode, struct flag_header *header, const char *title, int ch, int is_wlan)
{
#if 0
    static char * mpoa_type_str[] = {
        "EoA w/o FCS",
        "EoA w FCS",
        "PPPoA",
        "IPoA"
    };
#endif
    static char * is_vlan_str[] = {
        "nil",
        "single tag",
        "double tag",
        "reserved"
    };

    if ( !(g_dbg_enable & DBG_ENABLE_MASK_DUMP_FLAG_HEADER)  && ch >= 0)
        return;

    if ( ch >= 0 )
        printk("%s (ch %d)\n", title, ch);
    else
        printk("%s\n", title);

    switch ( fwcode )
    {
    case FWCODE_ROUTING_BRIDGING_ACC_A4:
    case FWCODE_ROUTING_ACC_D5:
        if ( is_wlan )
        {
#if 0
            struct wlan_flag_header * p = (struct wlan_flag_header *)header;

            printk("  aal5_raw          = %Xh (%s)\n", (u32)p->aal5_raw, p->aal5_raw ? "AAL5" : "ETH");
            printk("  mpoa_type         = %Xh (%s)\n", (u32)p->mpoa_type, mpoa_type_str[p->mpoa_type]);
            printk("  dsl_qid           = %Xh\n", (u32)p->dsl_qid);
            printk("  src_itf           = %Xh\n", (u32)header->src_itf);
#endif
        }
        else
        {
            printk("  ipv4_mc_pkt       = %Xh (%s)\n", (u32)header->ipv4_mc_pkt, header->ipv4_mc_pkt ? "multicast" : "uni-cast");
            printk("  proc_type         = %Xh (%s)\n", (u32)header->proc_type, header->proc_type ? "bridging" : "routing");
            printk("  tcpudp_err        = %Xh\n", (u32)header->tcpudp_err);
            printk("  tcpudp_chk        = %Xh\n", (u32)header->tcpudp_chk);
            printk("  is_udp            = %Xh\n", (u32)header->is_udp);
            printk("  is_tcp            = %Xh\n", (u32)header->is_tcp);
            printk("  ip_inner_offset   = %Xh\n", (u32)header->ip_inner_offset);
            printk("  is_pppoes         = %Xh\n", (u32)header->is_pppoes);
            printk("  is_inner_ipv6     = %Xh\n", (u32)header->is_inner_ipv6);
            printk("  is_inner_ipv4     = %Xh\n", (u32)header->is_inner_ipv4);
            printk("  is_vlan           = %Xh (%s)\n", (u32)header->is_vlan, is_vlan_str[header->is_vlan]);
            printk("  rout_index        = %Xh\n", (u32)header->rout_index);

            printk("  dest_list         = %Xh\n", (u32)header->dest_list);
            printk("  src_itf           = %Xh\n", (u32)header->src_itf);
            printk("  tcp_rstfin        = %Xh\n", (u32)header->tcp_rstfin);
            printk("  qid               = %Xh\n", (u32)header->qid);
            printk("  temp_dest_list    = %Xh\n", (u32)header->temp_dest_list);
            printk("  src_dir           = %Xh (from %s side)\n", (u32)header->src_dir, header->src_dir ? "WAN" : "LAN");
            printk("  acc_done          = %Xh\n", (u32)header->acc_done);
            printk("  is_outer_ipv6     = %Xh\n", (u32)header->is_outer_ipv6);
            printk("  is_outer_ipv4     = %Xh\n", (u32)header->is_outer_ipv4);
            printk("  is_tunnelled      = %Xh\n", (u32)header->is_tunnel);

            printk("  packet length     = %Xh\n", (u32)header->pkt_len);
            printk("  pl_byteoff        = %Xh\n", (u32)header->pl_byteoff);
            printk("  mpoa type         = %Xh\n", (u32)header->mpoa_type);
            printk("  ip_outer_offset   = %Xh\n", (u32)header->ip_outer_offset);

            printk("  traffic class     = %Xh\n", (u32)header->tc);
        }
        break;
    }
}
#endif

#if defined(DEBUG_DUMP_INIT) && DEBUG_DUMP_INIT
static INLINE void dump_init(void)
{
    if ( !(g_dbg_enable & DBG_ENABLE_MASK_DUMP_INIT) )
        return;

    printk("Share Buffer Conf:\n");
    printk("  SB_MST_SEL0(%08X) = 0x%08X, SB_MST_PRI1(%08X) = 0x%08X\n", (u32)SB_MST_PRI0, *SB_MST_PRI0, (u32)SB_MST_PRI1, *SB_MST_PRI1);

    printk("DPLUS:\n");
    printk("  DM_RXDB     = 0x%08X\n", *DM_RXDB);
    printk("  DM_RXCB     = 0x%08X\n", *DM_RXCB);
    printk("  DM_RXCFG    = 0x%08X\n", *DM_RXCFG);
    printk("  DM_RXPGCNT  = 0x%08X\n", *DM_RXPGCNT);
    printk("  DM_RXPKTCNT = 0x%08X\n", *DM_RXPKTCNT);
    printk("  DS_RXDB     = 0x%08X\n", *DS_RXDB);
    printk("  DS_RXCB     = 0x%08X\n", *DS_RXCB);
    printk("  DS_RXCFG    = 0x%08X\n", *DS_RXCFG);
    printk("  DS_RXPGCNT  = 0x%08X\n", *DS_RXPGCNT);

    printk("EMA:\n");
    printk("  EMA_CMDCFG  = 0x%08X\n", *EMA_CMDCFG);
    printk("  EMA_DATACFG = 0x%08X\n", *EMA_DATACFG);
    printk("  EMA_CMDCNT  = 0x%08X\n", *EMA_CMDCNT);
    printk("  EMA_DATACNT = 0x%08X\n", *EMA_DATACNT);
    printk("  EMA_ISR     = 0x%08X\n", *EMA_ISR);
    printk("  EMA_IER     = 0x%08X\n", *EMA_IER);
    printk("  EMA_CFG     = 0x%08X\n", *EMA_CFG);
    printk("  EMA_SUBID   = 0x%08X\n", *EMA_SUBID);

    printk("Communication:\n");
    printk("  FW_VER_ID(%08X)  = 0x%08X\n", (u32)FW_VER_ID, *(u32*)FW_VER_ID);
}
#endif

#ifdef CONFIG_IFX_PORT_MIRROR
static void mirror_pkt_eth(struct sk_buff *skb, int offset)
{
    struct sk_buff *new_skb = skb_copy(skb, GFP_ATOMIC);
    struct ethhdr *eth;
    int i;

    if ( new_skb != NULL )
    {
        if ( offset > 0 )
            skb_pull(new_skb, offset);
        new_skb->dev = ifx_get_mirror_netdev();
        eth = (struct ethhdr *)new_skb->data;
        memset(eth->h_source, 0x00, ETH_ALEN);
        if ( ifx_is_device_type_wireless() )
        {
            for ( i = 0; i < 6; i++ )
                eth->h_dest[i] = 0xFF;
        }
        dev_queue_xmit(new_skb);
    }
}
#endif

#if defined(ENABLE_DIRECTPATH_TX_QUEUE) && ENABLE_DIRECTPATH_TX_QUEUE
static int send_directpath_tx_queue(void)
{
    struct sk_buff *skb;
    int len;
    int i;

    for ( i = 0; !g_directpath_dma_full && i < NUM_ENTITY(g_ppe_directpath_data); i++ )
    {
        while ( !g_directpath_dma_full && g_ppe_directpath_data[i].skb_list )
        {
            skb = skb_get(g_ppe_directpath_data[i].skb_list);

            len = skb->len;
            if ( len < ETH_MIN_TX_PACKET_LENGTH + sizeof(struct sw_eg_pkt_header) )
                len = ETH_MIN_TX_PACKET_LENGTH + sizeof(struct sw_eg_pkt_header);

            put_skb_to_dbg_pool(skb);
            if ( dma_device_write(g_dma_device, skb->data, len, skb) == len )
            {
                //  remove skb from list
                if ( skb->prev == skb )
                    skb->prev = skb->next = g_ppe_directpath_data[i].skb_list = NULL;
                else
                {
                    g_ppe_directpath_data[i].skb_list = skb->next;
                    g_ppe_directpath_data[i].skb_list->prev = skb->prev;
                    skb->prev->next = skb->next;
                    skb->prev = skb->next = NULL;
                }
  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
                g_ppe_directpath_data[i].skb_list_size -= skb->len;
  #else
                g_ppe_directpath_data[i].skb_list_len--;
  #endif
                dev_kfree_skb_any(skb);
            }
            else
            {
                dev_kfree_skb_any(skb);
                return -1;
            }

            if ( g_ppe_directpath_data[i].callback.start_tx_fn
  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
                 && g_ppe_directpath_data[i].skb_list_size <= (ENABLE_DIRECTPATH_TX_QUEUE_SIZE * 3 / 4)
  #else
                 && g_ppe_directpath_data[i].skb_list_len <= (ENABLE_DIRECTPATH_TX_QUEUE_PKTS * 3 / 4)
  #endif
               )
                g_ppe_directpath_data[i].callback.start_tx_fn(g_ppe_directpath_data[i].netif);
        }
    }

    return 0;
}
#endif



/*
 * ####################################
 *           Global Function
 * ####################################
 */

static int32_t ppa_datapath_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    dbg("ppa_datapath_generic_hook cmd 0x%x\n", cmd );
    switch (cmd)  {
    case PPA_GENERIC_DATAPATH_TSET:
        return IFX_SUCCESS;
    case PPA_GENERIC_DATAPATH_ADDR_TO_FPI_ADDR:
        {
            PPA_FPI_ADDR *addr = (PPA_FPI_ADDR *) buffer;
            addr->addr_fpi = sb_addr_to_fpi_addr_convert(addr->addr_orig);
            return IFX_SUCCESS;
        }
    case PPA_GENERIC_DATAPATH_CLEAR_MIB:
        {
            clear_mib_counter(0x80000000);

            return IFX_SUCCESS;
        }
    case PPA_GENERIC_DATAPATH_SET_LAN_SEPARATE_FLAG:
        {
            lan_port_seperate_enabled = flag;
        }
    case PPA_GENERIC_DATAPATH_GET_LAN_SEPARATE_FLAG:
        {
            return lan_port_seperate_enabled;
        }
    case PPA_GENERIC_DATAPATH_SET_WAN_SEPARATE_FLAG:
        {
            wan_port_seperate_enabled = flag;
        }
    case PPA_GENERIC_DATAPATH_GET_WAN_SEPARATE_FLAG:
        {
            return wan_port_seperate_enabled;
        }
    default:
        dbg("ppa_datapath_generic_hook not support cmd 0x%x\n", cmd );
        return IFX_FAILURE;
    }

    return IFX_FAILURE;

}

int get_dslwan_qid_with_vcc(struct atm_vcc *vcc)
{
    return -1;
}

int get_netif_qid_with_pkt(struct sk_buff *skb, void *arg, int is_atm_vcc)
{
    if ( is_atm_vcc )
        return -1;
    else
    {
        int port = get_eth_port((struct net_device *)arg);
        if ( port >= 0 )
            return g_eth_prio_queue_map[port][skb->priority >= NUM_ENTITY(g_eth_prio_queue_map[port]) ? NUM_ENTITY(g_eth_prio_queue_map[port]) - 1 : skb->priority];
    }

    return 0;
}

int ppe_directpath_send(uint32_t if_id, struct sk_buff *skb, int32_t len, uint32_t flags)
{
    int ret;
    unsigned long sys_flag;
    uint32_t off;

    //  Careful, no any safety check here.
    //  Parameters must be correct.

    off = if_id - 3;

    g_ppe_directpath_data[off].tx_pkt++;

    if ( skb_headroom(skb) < sizeof(struct sw_eg_pkt_header) )
    {
        struct sk_buff *new_skb;

        new_skb = alloc_skb_tx(skb->len + sizeof(struct sw_eg_pkt_header));
        if ( !new_skb )
        {
            dev_kfree_skb_any(skb);
            g_ppe_directpath_data[off].tx_pkt_dropped++;
            return -1;
        }
        else
        {
            skb_reserve(new_skb, sizeof(struct sw_eg_pkt_header));
            skb_put(new_skb, skb->len);
            memcpy(new_skb->data, skb->data, skb->len);
            dev_kfree_skb_any(skb);
            skb = new_skb;
        }
    }

    ret = 0;
    spin_lock_irqsave(&g_directpath_tx_spinlock, sys_flag);
#if !defined(ENABLE_DIRECTPATH_TX_QUEUE) || !ENABLE_DIRECTPATH_TX_QUEUE
    if ( eth_xmit(skb, if_id /* dummy */, 3, if_id /* spid */, 0 /* dummy */) )
    {
        g_ppe_directpath_data[off].tx_pkt_dropped++;
        ret = -1;
    }
#else
    if ( !g_directpath_dma_full && !g_ppe_directpath_data[if_id - 3].skb_list )
    {
        if ( eth_xmit(skb, if_id /* dummy */, 3, if_id /* spid */, 0 /* dummy */) )
        {
            g_ppe_directpath_data[off].tx_pkt_dropped++;
            ret = -1;
        }
    }
    else
    {
        if (
  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
             g_ppe_directpath_data[off].skb_list_size + skb->len + sizeof(struct sw_eg_pkt_header) <= ENABLE_DIRECTPATH_TX_QUEUE_SIZE
  #else
             g_ppe_directpath_data[off].skb_list_len  < ENABLE_DIRECTPATH_TX_QUEUE_PKTS
  #endif
           )
        {
            struct sw_eg_pkt_header pkth = {spid:   if_id,  //  CPU/app0/app1/app2
                                            dpid:   2,      //  eth0/eth1/app0/app1/app2
                                            qid:    0,      //  0 is lowest priority, 3 is highest priority
                                            direct: 0};     //  disable direct forward

            skb_push(skb, sizeof(struct sw_eg_pkt_header));
            memcpy(skb->data, &pkth, sizeof(struct sw_eg_pkt_header));

  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
            g_ppe_directpath_data[off].skb_list_size += skb->len;
  #else
            g_ppe_directpath_data[off].skb_list_len++;
  #endif
            if ( g_ppe_directpath_data[off].skb_list != NULL )
            {
                g_ppe_directpath_data[off].skb_list->prev->next = skb;
                skb->next = g_ppe_directpath_data[off].skb_list;
                skb->prev = g_ppe_directpath_data[off].skb_list->prev;
                g_ppe_directpath_data[off].skb_list->prev = skb;
            }
            else
            {
                g_ppe_directpath_data[off].skb_list = skb;
                skb->prev = skb->next = skb;
            }

            g_ppe_directpath_data[off].tx_pkt_queued++;
        }
        else
        {
            if ( g_ppe_directpath_data[off].callback.stop_tx_fn )
                g_ppe_directpath_data[off].callback.stop_tx_fn(g_ppe_directpath_data[off].netif);
            dev_kfree_skb_any(skb);
            g_ppe_directpath_data[off].tx_pkt_dropped++;
            ret = -1;
        }
    }
#endif
    spin_unlock_irqrestore(&g_directpath_tx_spinlock, sys_flag);

    return ret;
}

int ppe_directpath_rx_stop(uint32_t if_id, uint32_t flags)
{
    return 0;
}

int ppe_directpath_rx_start(uint32_t if_id, uint32_t flags)
{
    return 0;
}

int ppe_clk_change(unsigned int arg, unsigned int flags)
{
    unsigned long sys_flag;
    unsigned int ppe_cfg_bit = arg == IFX_PPA_PWM_LEVEL_D0 ? 0 : (1 << 7);

    local_irq_save(sys_flag);
    if ( (*AMAZON_S_CGU_SYS & (1 << 7)) != ppe_cfg_bit )
    {
        *AMAZON_S_CGU_SYS ^= 1 << 7;
        *AMAZON_S_CGU_UPDATE = 1;
    }
    local_irq_restore(sys_flag);

    TX_QOS_CFG->time_tick = cgu_get_pp32_clock() / 62500;

    return 0;
}



/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

static int __init ppe_eth_init(void)
{
    int ret;
    int port;
    char buf[512];
#ifdef CONFIG_IFX_ETH_FRAMEWORK
    struct ifx_eth_fw_netdev_ops eth_ops = {
        .get_stats      = eth_get_stats,
        .open           = eth_open,
        .stop           = eth_stop,
        .change_mtu     = eth_change_mtu,
        .start_xmit     = NULL,
        .set_mac_address= eth_set_mac_address,
        .do_ioctl       = eth_ioctl,
        .tx_timeout     = eth_tx_timeout,
        .watchdog_timeo = ETH_WATCHDOG_TIMEOUT,
  #if defined(ENABLE_NAPI) && ENABLE_NAPI
        .poll           = eth_poll,
        .weight         = 64,
  #endif
    };
#endif
    struct eth_priv_data *priv;

    printk("Loading D5 (MII0/1) driver ...... ");

#if IFX_PPA_DP_DBG_PARAM_ENABLE
    if( ifx_ppa_drv_dp_dbg_param_enable == 1 )
    {
        ethwan = ifx_ppa_drv_dp_dbg_param_ethwan;
        wanitf = ifx_ppa_drv_dp_dbg_param_wanitf;
        ipv6_acc_en= ifx_ppa_drv_dp_dbg_param_ipv6_acc_en;
        wanqos_en= ifx_ppa_drv_dp_dbg_param_wanqos_en;
    }
#endif  //IFX_PPA_DP_DBG_PARAM_ENABLE

    reset_ppe();

    spin_lock_init(&g_mdio_lock);

    init_local_variables();

    init_hw();

    init_communication_data_structures(g_fwcode);

    ret = alloc_dma();
    if ( ret )
        goto ALLOC_DMA_FAIL;

#ifdef CONFIG_MII1_AUTONEG
    mii1_gphy_config_ext_int();
#endif

    for ( port = 0; port < 2; port++ )
    {
#ifdef CONFIG_IFX_ETH_FRAMEWORK
        if ( g_wanqos_en && (g_wan_itf & (1 << port)) )
            eth_ops.start_xmit = eth_qos_hard_start_xmit;
        else
            eth_ops.start_xmit = eth_hard_start_xmit;
#endif

#if defined(ENABLE_MIXED_QOS) && ENABLE_MIXED_QOS
        if(g_wanqos_en && (g_eth_wan_mode == 2) && (g_wan_itf & (1 << port)) != 0){
            eth_ops.start_xmit = eth_mix_qos_start_xmit;
        }
#endif

        sprintf(buf, "eth%d", port);
        g_eth_net_dev[port] = ifx_eth_fw_alloc_netdev(sizeof(struct eth_priv_data), buf, &eth_ops);
        if ( g_eth_net_dev[port] == NULL )
        {
            ret = -ENOMEM;
            goto ALLOC_NETDEV_FAIL;
        }
        if ( g_mii_port[port].phy_id == 0 )
            continue;
        eth_setup(g_eth_net_dev[port]);
        ret = ifx_eth_fw_register_netdev(g_eth_net_dev[port]);
        if ( ret )
        {
            ifx_eth_fw_free_netdev(g_eth_net_dev[port], 0);
            g_eth_net_dev[port] = NULL;
            goto ALLOC_NETDEV_FAIL;
        }
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[port]);
        priv->mii_port = &g_mii_port[port];
        priv->mii.phy_id_mask   = 0x1F;
        priv->mii.reg_num_mask  = 0x1F;
        priv->mii.phy_id        = priv->mii_port->phy_addr;
        priv->mii.force_media   = 0;
        priv->mii.full_duplex   = 0;
        priv->mii.supports_gmii = priv->mii_port->mode == RGMII_MODE ? 1 : 0;
        priv->mii.mdio_read     = mdio_read;
        priv->mii.mdio_write    = mdio_write;
#ifdef CONFIG_IFX_ETH_FRAMEWORK
        priv->rx_packets = priv->stats.rx_packets;
        priv->rx_bytes   = priv->stats.rx_bytes;
        priv->rx_dropped = priv->stats.rx_dropped;
        priv->tx_packets = priv->stats.tx_packets;
        priv->tx_bytes   = priv->stats.tx_bytes;
        priv->tx_errors  = priv->stats.tx_errors;
        priv->tx_dropped = priv->stats.tx_dropped;
#endif
#ifdef CONFIG_MII1_AUTONEG
        ret = 0;
        switch ( priv->mii_port->phy_id )
        {
        case PHYID_LANTIQ_PHY11G:
        case PHYID_IFX_PHY11G:
            mdio_write(NULL, priv->mii.phy_id, MII_IMASK, 0x0007);
            ret = request_irq(MII1_GPHY_INT, mii1_gphy_11G_int_handler, IRQF_SHARED, "mii_phy11g_isr", g_eth_net_dev[port]);
            break;

        case PHYID_VSC_SIMPLIPHY:
            mdio_write(NULL, priv->mii.phy_id, MII_IMASK, 0xF000);
            ret = request_irq(MII1_GPHY_INT, mii1_gphy_VSC_int_handler, IRQF_SHARED, "mii_vscgphy_isr", g_eth_net_dev[port]);
            break;

        case PHYID_ICPLUS_IP101A:
        default:
            break;
        }
        if ( ret )
        {
            if ( g_mii_port[port].phy_id != 0 )
                ifx_eth_fw_unregister_netdev(g_eth_net_dev[port], 0);
            ifx_eth_fw_free_netdev(g_eth_net_dev[port], 0);
            g_eth_net_dev[port] = NULL;
            goto ALLOC_NETDEV_FAIL;
        }
#endif
    }

    ret = request_irq(PPE_MAILBOX_IGU0_INT, mailbox0_irq_handler, IRQF_DISABLED, "d5_mailbox0_isr", NULL);
    if ( ret )
    {
        if ( ret == -EBUSY )
            err("IRQ may be occupied by other PPE driver, please reconfig to disable it.\n");
        goto REQUEST_IGU0_IRQ_FAIL;
    }

    ret = request_irq(PPE_MAILBOX_IGU1_INT, mailbox_irq_handler, IRQF_DISABLED, "d5_mailbox_isr", NULL);
    if ( ret )
    {
        if ( ret == -EBUSY )
            err("IRQ may be occupied by other PPE driver, please reconfig to disable it.\n");
        goto REQUEST_IRQ_FAIL;
    }

    /*
     *  init variable for directpath
     */
    memset(g_ppe_directpath_data, 0, sizeof(g_ppe_directpath_data));

    dump_init();

    ret = pp32_start(g_fwcode);
    if ( ret )
        goto PP32_START_FAIL;

    *MBOX_IGU0_IER = DMA_TX_CH0_SIG | DMA_TX_CH1_SIG;
    *MBOX_IGU1_IER = g_mailbox_signal_mask;

    start_mii2();

    /*  create proc file    */
    proc_file_create();

    //Fix warning message ---start
    ifx_ppa_drv_get_dslwan_qid_with_vcc_hook = get_dslwan_qid_with_vcc;
    ifx_ppa_drv_get_netif_qid_with_pkt_hook = get_netif_qid_with_pkt;
    ifx_ppa_drv_ppe_clk_change_hook = ppe_clk_change;

#ifdef CONFIG_IFX_PPA_API_DIRECTPATH
    ifx_ppa_drv_g_ppe_directpath_data = g_ppe_directpath_data;
    ifx_ppa_drv_directpath_send_hook = ppe_directpath_send;
    ifx_ppa_drv_directpath_rx_start_hook = ppe_directpath_rx_start;
    ifx_ppa_drv_directpath_rx_stop_hook = ppe_directpath_rx_stop;
#endif //CONFIG_IFX_PPA_API_DIRECTPATH
    ifx_ppa_drv_datapath_generic_hook = ppa_datapath_generic_hook;
    //Fix warning message ---end

    printk("Succeeded!\n");
    print_driver_ver(buf, sizeof(buf), "PPE datapath driver info", VER_FAMILY, VER_DRTYPE, VER_INTERFACE, VER_ACCMODE, VER_MAJOR, VER_MID, VER_MINOR);
    printk(buf);
    print_fw_ver(buf, sizeof(buf));
    printk(buf);

    return 0;

PP32_START_FAIL:
    free_irq(PPE_MAILBOX_IGU1_INT, NULL);
REQUEST_IRQ_FAIL:
    free_irq(PPE_MAILBOX_IGU0_INT, NULL);
REQUEST_IGU0_IRQ_FAIL:
ALLOC_NETDEV_FAIL:
    while ( port-- )
    {
#ifdef CONFIG_MII1_AUTONEG
        free_irq(MII1_GPHY_INT, g_eth_net_dev[port]);
#endif
        if ( g_mii_port[port].phy_id != 0 )
            ifx_eth_fw_unregister_netdev(g_eth_net_dev[port], 0);
        ifx_eth_fw_free_netdev(g_eth_net_dev[port], 0);
        g_eth_net_dev[port] = NULL;
    }
    free_dma();
ALLOC_DMA_FAIL:
    clear_local_variables();
    return ret;
}

static void __exit ppe_eth_exit(void)
{
    int i;

    //Fix warning message ---start
    ifx_ppa_drv_get_dslwan_qid_with_vcc_hook = NULL;
    ifx_ppa_drv_get_netif_qid_with_pkt_hook = NULL;
    ifx_ppa_drv_ppe_clk_change_hook = NULL;

#ifdef CONFIG_IFX_PPA_API_DIRECTPATH
    ifx_ppa_drv_g_ppe_directpath_data = NULL;
    ifx_ppa_drv_directpath_send_hook = NULL;
    ifx_ppa_drv_directpath_rx_start_hook = NULL;
    ifx_ppa_drv_directpath_rx_stop_hook = NULL;
#endif //CONFIG_IFX_PPA_API_DIRECTPATH
    ifx_ppa_drv_datapath_generic_hook = NULL;
    //Fix warning message ---end

    proc_file_delete();

    stop_datapath();

    pp32_stop();

    free_irq(PPE_MAILBOX_IGU1_INT, NULL);

    free_irq(PPE_MAILBOX_IGU0_INT, NULL);

    for ( i = 0; i < NUM_ENTITY(g_eth_net_dev); i++ )
    {
#ifdef CONFIG_MII1_AUTONEG
        switch ( g_mii_port[i].phy_id )
        {
        case PHYID_LANTIQ_PHY11G:
        case PHYID_IFX_PHY11G:
        case PHYID_VSC_SIMPLIPHY:
            free_irq(MII1_GPHY_INT, g_eth_net_dev[i]);
        }
#endif
        if ( g_mii_port[i].phy_id != 0 )
            ifx_eth_fw_unregister_netdev(g_eth_net_dev[i], 0);
        ifx_eth_fw_free_netdev(g_eth_net_dev[i], 0);
        g_eth_net_dev[i] = NULL;
    }

    free_dma();

    clear_local_variables();

    restore_ppe_conf();
}

#if defined(CONFIG_IFX_PPA_DATAPATH)
static int __init eth0addr_setup(char *line)
{
    ethaddr_setup(0, line);

    return 0;
}

static int __init eth1addr_setup(char *line)
{
    ethaddr_setup(1, line);

    return 0;
}

static int __init wan_mode_setup(char *line)
{
    if ( strcmp(line, "1") == 0 )
        ethwan = 1;
    else if ( strcmp(line, "2") == 0 )
        ethwan = 2;

    return 0;
}

static int __init wanitf_setup(char *line)
{
    wanitf = simple_strtoul(line, NULL, 0);

    if ( wanitf > 0xFF )
        wanitf = ~0;

    return 0;
}

static int __init ipv6_acc_en_setup(char *line)
{
    if ( strcmp(line, "0") == 0 )
        ipv6_acc_en = 0;
    else
        ipv6_acc_en = 1;

    return 0;
}

static int __init wanqos_en_setup(char *line)
{
    int tmp_wanqos_en = simple_strtoul(line, NULL, 0);

    if ( isdigit(*line) && tmp_wanqos_en >= 0 && tmp_wanqos_en <= 8 )
        wanqos_en = tmp_wanqos_en;

    return 0;
}

 #if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_SIZE)
static int __init directpath_tx_queue_size_setup(char *line)
{
    unsigned long i;

    i = simple_strtoul(line, NULL, 0);
    directpath_tx_queue_size = (int)i;

    return 0;
}
 #endif

 #if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_PKTS)
static int __init directpath_tx_queue_pkts_setup(char *line)
{
    unsigned long i;

    i = simple_strtoul(line, NULL, 0);
    directpath_tx_queue_pkts = (int)i;

    return 0;
}
 #endif
#endif



module_init(ppe_eth_init);
module_exit(ppe_eth_exit);
#if defined(CONFIG_IFX_PPA_DATAPATH)
  __setup("ethaddr=", eth0addr_setup);
  __setup("eth1addr=", eth1addr_setup);
  __setup("ethwan=", wan_mode_setup);
  __setup("wanitf=", wanitf_setup);
  __setup("ipv6_acc_en=", ipv6_acc_en_setup);
  __setup("wanqos_en=", wanqos_en_setup);
 #if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  __setup("directpath_tx_queue_size=", directpath_tx_queue_size_setup);
 #endif
 #if defined(CONFIG_IFX_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  __setup("directpath_tx_queue_pkts=", directpath_tx_queue_pkts_setup);
 #endif
#endif



MODULE_LICENSE("GPL");
