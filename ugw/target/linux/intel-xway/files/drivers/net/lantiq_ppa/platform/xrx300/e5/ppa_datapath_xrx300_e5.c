/******************************************************************************
**
** FILE NAME    : ppa_datapath_ar10_e5.c
** PROJECT      : UEIP
** MODULES      : PTM + MII0/1 Acceleration Package (AR10 PPA E5)
**
** DATE         : 20 DEC 2010
** AUTHOR       : Xu Liang
** DESCRIPTION  : PTM + MII0/1 Driver with Acceleration Firmware (E5)
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
** 20 DEC 2010  Xu Liang               Initiate Version
** 10 DEC 2012  Manamohan Shetty       Added the support for RTP,MIB mode and CAPWAP 
**                                     Features 
*******************************************************************************/



/*
 * ####################################
 *      Version No.
 * ####################################
 */
#define VER_FAMILY      0x80    //  bit 0: res
                                //      1: Danube
                                //      2: Twinpass
                                //      3: Amazon-SE
                                //      4: res
                                //      5: AR9
                                //      6: VR9
                                //      7: AR10
#define VER_DRTYPE      0x03    //  bit 0: Normal Data Path driver
                                //      1: Indirect-Fast Path driver
                                //      2: HAL driver
                                //      3: Hook driver
                                //      4: Stack/System Adaption Layer driver
                                //      5: PPA API driver
#define VER_INTERFACE   0x0B    //  bit 0: MII 0
                                //      1: MII 1
                                //      2: ATM WAN
                                //      3: PTM WAN
#define VER_ACCMODE     0x03    //  bit 0: Routing
                                //      1: Bridging
#define VER_MAJOR       0
#define VER_MID         0
#define VER_MINOR       2



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
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
#include <linux/ethtool.h>      /*  ethtool_cmd     */
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <net/xfrm.h>

/*
 *  Chip Specific Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)

#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pmu.h>
#include <asm/ifx/ifx_rcu.h>
#include <asm/ifx/ifx_gpio.h>
#include <asm/ifx/ifx_dma_core.h>
#include <asm/ifx/ifx_clk.h>
#include <switch_api/ifx_ethsw_kernel_api.h>
#include <switch_api/ifx_ethsw_api.h>
#ifdef CONFIG_LTQ_ETH_FRAMEWORK
  #include <asm/ifx/ifx_eth_framework.h>
#endif

#else
//#include <asm/mach-ltqcpe/ltq_regs.h>
//#include <ltq_regs.h>
//#include <common_routines.h>
//#include <ltq_pmu.h>
//#include <ltq_rcu.h>
//#include <ltq_gpio.h>
//#include <ltq_led.h>
//#include <ltq_dma.h>
//#include <ltq_clk.h>
#include <lantiq_dma.h>
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>

//#include <switch_api/ifx_ethsw_kernel_api.h>
//#include <switch_api/ifx_ethsw_api.h>
#include <switch-api/lantiq_ethsw_api.h>
#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
  #include <lantiq_eth_framework.h>
#endif
#endif

#include <net/ppa_api.h>
#include <net/ppa_stack_al.h>
#include <net/ppa_api_directpath.h>
#include <net/ppa_ppe_hal.h>
#include "../../ppa_datapath.h"
#include "ppa_datapath_fw_xrx300_e5.h"
#include "ppa_hal_xrx300_e5.h"
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
#include <net/ltq_eth_oam_handler.h>
#endif
#include "ppa_xrx300_e5_common.h"



/*
 * ####################################
 *   Parameters to Configure PPE
 * ####################################
 */
static int write_descriptor_delay  = 0x20;      /*  Write descriptor delay                          */

static int ethwan = 0;
static int wanitf = ~0;

static int ipv6_acc_en = 1;

static int wanqos_en = 8;
static int qos_queue_len[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; 
static uint32_t lan_port_seperate_enabled=0, wan_port_seperate_enabled=0;

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  static int directpath_tx_queue_size = 1524 * 50;
#endif
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  static int directpath_tx_queue_pkts = 50;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
  #define MODULE_PARM_ARRAY(a, b)   module_param_array(a, int, NULL, 0)
  #define MODULE_PARM(a, b)         module_param(a, int, 0)
#else
  #define MODULE_PARM_ARRAY(a, b)   MODULE_PARM(a, b)
#endif

MODULE_PARM(write_descriptor_delay, "i");
MODULE_PARM_DESC(write_descriptor_delay, "PPE core clock cycles between descriptor write and effectiveness in external RAM");

MODULE_PARM(ethwan, "i");
MODULE_PARM_DESC(ethwan, "WAN mode, 0 - DSL WAN.");

MODULE_PARM(wanitf, "i");
MODULE_PARM_DESC(wanitf, "WAN interfaces, bit 0 - ETH0, 1 - ETH1, 2 - reserved for CPU0, 3/4/5 - DirectPath, 6/7 - DSL");

MODULE_PARM(ipv6_acc_en, "i");
MODULE_PARM_DESC(ipv6_acc_en, "IPv6 support, 1 - enabled, 0 - disabled.");

MODULE_PARM(wanqos_en, "i");
MODULE_PARM_DESC(wanqos_en, "WAN QoS support, 1 - enabled, 0 - disabled.");

MODULE_PARM_ARRAY(qos_queue_len, "1-8i");
MODULE_PARM_DESC(qos_queue_len, "QoS queue's length for each QoS queue");

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  MODULE_PARM(directpath_tx_queue_size, "i");
  MODULE_PARM_DESC(directpath_tx_queue_size, "directpath TX queue size in total buffer size");
#endif
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  MODULE_PARM(directpath_tx_queue_pkts, "i");
  MODULE_PARM_DESC(directpath_tx_queue_pkts, "directpath TX queue size in total packet number");
#endif



/*
 * ####################################
 *              Board Type
 * ####################################
 */

#define BOARD_AR10_REFERENCE                     0x01



/*
 * ####################################
 *              Definition
 * ####################################
 */

#define BOARD_CONFIG                            BOARD_AR10_REFERENCE

#define DEBUG_SKB_SWAP                          0

#if defined(CONFIG_SUPPORT_PMAC_FLOWCONTROL)
    #define ENABLE_P2P_COPY_WITHOUT_DESC        0
#else
    #define ENABLE_P2P_COPY_WITHOUT_DESC        1
#endif

#define ENABLE_LED_FRAMEWORK                    1

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  #define ENABLE_DIRECTPATH_TX_QUEUE            1
  #define ENABLE_DIRECTPATH_TX_QUEUE_SIZE       directpath_tx_queue_size
#elif defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  #define ENABLE_DIRECTPATH_TX_QUEUE            1
  #define ENABLE_DIRECTPATH_TX_QUEUE_PKTS       directpath_tx_queue_pkts
#endif

#define DISABLE_LOCAL_MAC_ADD_TO_SW             0

#define ENABLE_MII0_BYPASS_DELAY_FIX            1

#define ENABLE_MII1_BYPASS_DELAY_FIX            1

#define ENABLE_SWITCH_FLOW_CONTROL              1

#define ENABLE_DFE_LOOPBACK_TEST                0   //  DSL firmware level loopback

#define ENABLE_MY_MEMCPY                        0

#define ENABLE_FW_MODULE_TEST                   0

#define INCLUDE_MPRINT_PROC                     0

#define ENABLE_DEBUG                            1

#define ENABLE_ASSERT                           1

#define DEBUG_DUMP_SKB                          1

#define DEBUG_DUMP_FLAG_HEADER                  1

#define DEBUG_DUMP_INIT                         0

#define DEBUG_FIRMWARE_TABLES_PROC              1

#define DEBUG_MEM_PROC                          1

#define DEBUG_PP32_PROC                         1

#define DEBUG_FW_PROC                           1

#define DEBUG_MIRROR_PROC                       1

#define PPE_MAILBOX_IGU0_INT                    INT_NUM_IM2_IRL23
#define PPE_MAILBOX_IGU1_INT                    AR10_MAILBOX_INT1

#define MY_ETH0_ADDR                            g_my_ethaddr

#if defined(CONFIG_IFXMIPS_DSL_CPE_MEI) && !defined(CONFIG_LTQMIPS_DSL_CPE_MEI)
  #define CONFIG_LTQMIPS_DSL_CPE_MEI            1
#endif

#ifdef CONFIG_USE_EMULATOR
#define AR10_EMULATION                          1
#endif

#define RCU_RST_REQ         0x0010
//  specific board related configuration
#if defined (BOARD_CONFIG)
  #if BOARD_CONFIG == BOARD_AR10_REFERENCE
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
#define DBG_ENABLE_MASK_MAC_SWAP                (1 << 12)
#define DBG_ENABLE_MASK_ALL                     (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT \
                                                | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX                 \
                                                | DBG_ENABLE_MASK_DUMP_FLAG_HEADER | DBG_ENABLE_MASK_DUMP_INIT | DBG_ENABLE_MASK_MAC_SWAP)

/*
 *  Mailbox Signal Bit
 */
#define DMA_TX_CH1_SIG                          (1 << 31)   //  IGU0
#define DMA_TX_CH0_SIG                          (1 << 30)   //  IGU0
#define CPU_TO_WAN_TX_SIG                       (1 << 17)
#define CPU_TO_WAN_SWAP_SIG                     (1 << 16)
#define WAN_RX_SIG(i)                           (1 << 0)    //  0: Downstream DSL Traffic


/*
 *  Constant Definition
 */
#define ETH_WATCHDOG_TIMEOUT                    (10 * HZ)
#define ETOP_MDIO_DELAY                         1
#define IDLE_CYCLE_NUMBER                       30000

#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
#define DMA_PACKET_SIZE                         1696    //  ((1604 + 8 <2 VLAN> + 62 <PPE FW> + 8 <SW Header>) + 31) & ~31 [1696]
#else
#define DMA_PACKET_SIZE                         1600    //  ((1518 + 8 <2 VLAN> + 62 <PPE FW> + 8 <SW Header>) + 31) & ~31
#endif

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

#define DEFAULT_RX_HUNT_BITTH                   4

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
#define WAN_TX_DESC_NUM(i)                      __WAN_TX_QUEUE_LEN  //  WAN QoS TX, i < __WAN_TX_QUEUE_NUM
#define WAN_TX_DESC_NUM_TOTAL                   448
#define CPU_TO_WAN_SWAP_DESC_NUM                32
#define WAN_RX_DESC_NUM                         32
#define DMA_TX_CH1_DESC_NUM                     (g_eth_wan_mode ? (g_eth_wan_mode == 3 /* ETH WAN */ && g_wanqos_en ? 32 : DMA_RX_CH1_DESC_NUM) : WAN_RX_DESC_NUM)
#define DMA_RX_CH1_DESC_NUM                     32  //  DSL/MII1 WAN mode fast path
#define DMA_RX_CH2_DESC_NUM                     32  //  Switch/MII0/MII1 LAN mode fast path
#define DMA_TX_CH0_DESC_NUM                     DMA_RX_CH2_DESC_NUM //  DMA_TX_CH0_DESC_NUM is used to replace DMA_TX_CH2_DESC_NUM
#define WAN_BCi_TX_DESC_NUM(i)                  2

#define PPA_PORT_NUM                        8

/*
 *  Bits Operation
 */
//#define GET_BITS(x, msb, lsb)                   (((x) & ((1 << ((msb) + 1)) - 1)) >> (lsb))
//#define SET_BITS(x, msb, lsb, value)            (((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1))) | (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))

/*
 *  Internal Tantos Switch Register
 */
#define AR10_SWIP_MACRO                          (KSEG1 | 0x1E108000)
#define AR10_SWIP_MACRO_REG(off)                 ((volatile u32*)(AR10_SWIP_MACRO + (off) * 4))
#define AR10_SWIP_TOP                            (AR10_SWIP_MACRO | (0x0C40 * 4))
#define AR10_SWIP_TOP_REG(off)                   ((volatile u32*)(AR10_SWIP_TOP + (off) * 4))
#define ETHSW_SWRES_REG                         AR10_SWIP_MACRO_REG(0x00)
#define ETHSW_CLK_REG                           AR10_SWIP_MACRO_REG(0x01)
#define ETHSW_BM_RAM_VAL_3_REG                  AR10_SWIP_MACRO_REG(0x40)
#define ETHSW_BM_RAM_VAL_2_REG                  AR10_SWIP_MACRO_REG(0x41)
#define ETHSW_BM_RAM_VAL_1_REG                  AR10_SWIP_MACRO_REG(0x42)
#define ETHSW_BM_RAM_VAL_0_REG                  AR10_SWIP_MACRO_REG(0x43)
#define ETHSW_BM_RAM_ADDR_REG                   AR10_SWIP_MACRO_REG(0x44)
#define ETHSW_BM_RAM_CTRL_REG                   AR10_SWIP_MACRO_REG(0x45)
//  Buffer manager per port registrs
#define ETHSW_BM_PCFG_REG(port)                 AR10_SWIP_MACRO_REG(0x80 + (port) * 2)   //  port < 7
#define ETHSW_BM_RMON_CTRL_REG(port)            AR10_SWIP_MACRO_REG(0x81 + (port) * 2)   //  port < 7
#define PCE_PMAP_REG(reg)                       AR10_SWIP_MACRO_REG(0x453 + (reg) - 1)   //  1 <= reg <= 3
//  Parser & Classification Engine
#define PCE_TBL_KEY(n)                          AR10_SWIP_MACRO_REG(0x440 + 7 - (n))                 //  n < 7
#define PCE_TBL_MASK                            AR10_SWIP_MACRO_REG(0x448)
#define PCE_TBL_VAL(n)                          AR10_SWIP_MACRO_REG(0x449 + 4 - (n))                 //  n < 4;
#define PCE_TBL_ADDR                            AR10_SWIP_MACRO_REG(0x44E)
#define PCE_TBL_CTRL                            AR10_SWIP_MACRO_REG(0x44F)
#define PCE_TBL_STAT                            AR10_SWIP_MACRO_REG(0x450)
#define PCE_GCTRL_REG(reg)                      AR10_SWIP_MACRO_REG(0x456 + (reg))
#define PCE_PCTRL_REG(port, reg)                AR10_SWIP_MACRO_REG(0x480 + (port) * 0xA + (reg))    //  port < 12, reg < 4
#define PCE_TCM_STAT(port)                      AR10_SWIP_MACRO_REG(0x581 + (port) * 7)
//  MAC Frame Length Register
#define MAC_FLEN_REG                            AR10_SWIP_MACRO_REG(0x8C5)
//  MAC Port Status Register
#define MAC_PSTAT_REG(port)                     AR10_SWIP_MACRO_REG(0x900 + (port) * 0xC)//  port < 7
//  MAC Control Register 0
#define MAC_CTRL_REG(port, reg)                 AR10_SWIP_MACRO_REG(0x903 + (port) * 0xC + (reg))    //  port < 7, reg < 7
//  Ethernet Switch Fetch DMA Port Control, Controls per-port functions of the Fetch DMA
#define FDMA_PCTRL_REG(port)                    AR10_SWIP_MACRO_REG(0xA80 + (port) * 6)  //  port < 7
//  Ethernet Switch Store DMA Port Control, Controls per-ingress-port functions of the Store DMA
#define SDMA_PCTRL_REG(port)                    AR10_SWIP_MACRO_REG(0xBC0 + (port) * 6)  //  port < 7
//  Global Control Register 0
#define GLOB_CTRL_REG                           AR10_SWIP_TOP_REG(0x00)
//  MDIO Control Register
#define MDIO_CTRL_REG                           AR10_SWIP_TOP_REG(0x08)
//  MDIO Read Data Register
#define MDIO_READ_REG                           AR10_SWIP_TOP_REG(0x09)
//  MDIO Write Data Register
#define MDIO_WRITE_REG                          AR10_SWIP_TOP_REG(0x0A)
//  MDC Clock Configuration Register 0
#define MDC_CFG_0_REG                           AR10_SWIP_TOP_REG(0x0B)
//  MDC Clock Configuration Register 1
#define MDC_CFG_1_REG                           AR10_SWIP_TOP_REG(0x0C)
//  PHY Address Register PORT 5~0
#define PHY_ADDR_REG(port)                      AR10_SWIP_TOP_REG(0x15 - (port))     //  port < 6
//  PHY MDIO Polling Status per PORT
#define MDIO_STAT_REG(port)                     AR10_SWIP_TOP_REG(0x16 + (port))     //  port < 6
//  xMII Control Registers
//  xMII Port 0 Configuration register
#define MII_CFG_REG(port)                       AR10_SWIP_TOP_REG(0x36 + (port) * 2) //  port < 6
//  Configuration of Clock Delay for Port 0 (used for RGMII mode only)
#define MII_PCDU_REG(port)                      AR10_SWIP_TOP_REG(0x37 + (port) * 2) //  port < 6
//  PMAC Header Control Register
#define PMAC_HD_CTL_REG                         AR10_SWIP_TOP_REG(0x82)
//  PMAC Type/Length register
#define PMAC_TL_REG                             AR10_SWIP_TOP_REG(0x83)
//  PMAC Source Address Register
#define PMAC_SA1_REG                            AR10_SWIP_TOP_REG(0x84)
#define PMAC_SA2_REG                            AR10_SWIP_TOP_REG(0x85)
#define PMAC_SA3_REG                            AR10_SWIP_TOP_REG(0x86)
//  PMAC Destination Address Register
#define PMAC_DA1_REG                            AR10_SWIP_TOP_REG(0x87)
#define PMAC_DA2_REG                            AR10_SWIP_TOP_REG(0x88)
#define PMAC_DA3_REG                            AR10_SWIP_TOP_REG(0x89)
//  PMAC VLAN register
#define PMAC_VLAN_REG                           AR10_SWIP_TOP_REG(0x8A)
//  PMAC Inter Packet Gap in RX Direction
#define PMAC_RX_IPG_REG                         AR10_SWIP_TOP_REG(0x8B)
//  PMAC Special Tag Ethertype
#define PMAC_ST_ETYPE_REG                       AR10_SWIP_TOP_REG(0x8C)
//  PMAC Ethernet WAN Group
#define PMAC_EWAN_REG                           AR10_SWIP_TOP_REG(0x8D)

/*
 *  Internal Tantos Switch Related
 */
//#define MDIO_OP_READ                            (2 << 10)
//#define MDIO_OP_WRITE                           (1 << 10)
//#define TANTOS_CHIP_ID                          0x2599


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
 *  DMA/EMA Descriptor Base Address
 */
#define CPU_TO_WAN_TX_DESC_BASE                 ((volatile struct tx_descriptor *)SB_BUFFER(0x3C00))                /*  32 each queue    */
#define __WAN_TX_QUEUE_NUM                      g_wanqos_en
#define __VIRTUAL_TX_QUEUE_NUM                  SWITCH_CLASS_NUM    //  LAN interface, there is no real firmware queue, but only virtual queue maintained by driver, so that skb->priority => firmware QId => switch class can be mapped.
#define SWITCH_CLASS_NUM                        8
#define __WAN_TX_QUEUE_LEN                      (WAN_TX_DESC_NUM_TOTAL / __WAN_TX_QUEUE_NUM < 256 ? (WAN_TX_DESC_NUM_TOTAL / __WAN_TX_QUEUE_NUM) : 255)
#define __WAN_TX_DESC_BASE(i)                   (0x3C80 + (i) * 2 * __WAN_TX_QUEUE_LEN)
#define WAN_QOS_DESC_BASE                       0x3C80
#define WAN_TX_DESC_BASE(i)                     ((volatile struct tx_descriptor *)SB_BUFFER(__WAN_TX_DESC_BASE(i))) /*  i < __WAN_TX_QUEUE_NUM, __WAN_TX_QUEUE_LEN each queue    */
#define CPU_TO_WAN_SWAP_DESC_BASE               ((volatile struct tx_descriptor *)SB_BUFFER(0x3C40))                /*  32 each queue    */
#define WAN_RX_DESC_BASE                        ((volatile struct rx_descriptor *)SB_BUFFER(0x3AC0))                /*  32 each queue    */
#define DMA_TX_CH1_DESC_BASE                    (g_eth_wan_mode ? (g_eth_wan_mode == 3 /* ETH WAN */ && g_wanqos_en ? ((volatile struct rx_descriptor *)SB_BUFFER(0x3AC0)): DMA_RX_CH1_DESC_BASE) : WAN_RX_DESC_BASE)
#define DMA_RX_CH1_DESC_BASE                    ((volatile struct rx_descriptor *)SB_BUFFER(0x3B40))                /*  32 each queue    */
#define DMA_RX_CH2_DESC_BASE                    ((volatile struct rx_descriptor *)SB_BUFFER(0x3B00))                /*  32 each queue    */
#define DMA_TX_CH0_DESC_BASE                    DMA_RX_CH2_DESC_BASE
#define WAN_BCi_TX_DESC_PPE(i)                  (0x3910 + (i) * 2 * 2)
#define WAN_BCi_TX_DESC_BASE(i)                 ((volatile struct tx_descriptor *)SB_BUFFER(WAN_BCi_TX_DESC_PPE(i)))/*  i < 2, 2 each queue    */

#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
/*
 *  Firmware Proc
 */
//  need check with hejun
  #define TIR(i)                                ((volatile u32 *)(0xBE198800) + (i))

  #define __CPU_TX_SWAPPER_DROP_PKTS            SB_BUFFER(0x32C0)
  #define __CPU_TX_SWAPPER_DROP_BYTES           SB_BUFFER(0x32C1)
  #define __DSLWAN_FP_SWAPPER_DROP_PKTS         SB_BUFFER(0x32C2)
  #define __DSLWAN_FP_SWAPPER_DROP_BYTES        SB_BUFFER(0x32C3)

  #define __CPU_TXDES_SWAP_RDPTR                SB_BUFFER(0x32C4)
  #define __DSLWAN_FP_RXDES_SWAP_RDPTR          SB_BUFFER(0x32C5)
  #define __DSLWAN_FP_RXDES_DPLUS_WRPTR         SB_BUFFER(0x32C6)

  #define __DSLWAN_TX_PKT_CNT(i)                SB_BUFFER(0x32D0+(i))   //  i < 8

  #define __DSLWAN_TXDES_SWAP_PTR(i)            SB_BUFFER(0x32E0+(i))   //  i < 8

#if 0
  // SB_BUFFER(0x29C0) - SB_BUFFER(0x29C7)
  #define __VLAN_PRI_TO_QID_MAPPING             SB_BUFFER(0x32E0)
#endif

  //
  // etx MIB: SB_BUFFER(0x32F0) - SB_BUFFER(0x32FF)
  #define __DSLWAN_TX_THRES_DROP_PKT_CNT(i)     SB_BUFFER(0x2D80+(i))   //  i < 8

  #define __CPU_TO_DSLWAN_TX_PKTS               SB_BUFFER(0x32C8)
  #define __CPU_TO_DSLWAN_TX_BYTES              SB_BUFFER(0x32C9)

  #define ACC_ERX_PID                           SB_BUFFER(0x2B00)
  #define ACC_ERX_PORT_TIMES                    SB_BUFFER(0x2B01)
  #define SLL_ISSUED                            SB_BUFFER(0x2B02)
  #define BMC_ISSUED                            SB_BUFFER(0x2B03)
  #define DPLUS_RX_ON                           SB_BUFFER(0x3283)
  #define ISR_IS                                SB_BUFFER(0x3284)

  #define __ENETS_PGNUM                         20
  #define __ERX_CBUF_BASE                       SB_BUFFER(0x343C)
  #define __ERX_DBUF_BASE                       SB_BUFFER(0x3450)

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

  #define PRE_DPLUS_PTR                         SB_BUFFER(0x3281)
  #define DPLUS_PTR                             SB_BUFFER(0x3282)
  #define DPLUS_CNT                             SB_BUFFER(0x3280)
#endif
#define xTM_MODE_TO_DSL                     	SB_BUFFER(0x0D78)

/*
 *  Share Buffer Registers
 */
#define SB_MST_PRI0                             PPE_REG_ADDR(0x0300)
#define SB_MST_PRI1                             PPE_REG_ADDR(0x0301)

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
 *  DSL registers
 */
#define AT_CTRL                                 PPE_REG_ADDR(0xD02)
#define AR_CTRL                                 PPE_REG_ADDR(0xD08)
#define AT_IDLE0                                PPE_REG_ADDR(0xD28)
#define AT_IDLE1                                PPE_REG_ADDR(0xD29)
#define AR_IDLE0                                PPE_REG_ADDR(0xD74)
#define AR_IDLE1                                PPE_REG_ADDR(0xD75)
#define RFBI_CFG                                PPE_REG_ADDR(0x400)
#define SFSM_DBA0                               PPE_REG_ADDR(0x412)
#define SFSM_DBA1                               PPE_REG_ADDR(0x413)
#define SFSM_CBA0                               PPE_REG_ADDR(0x414)
#define SFSM_CBA1                               PPE_REG_ADDR(0x415)
#define SFSM_CFG0                               PPE_REG_ADDR(0x416)
#define SFSM_CFG1                               PPE_REG_ADDR(0x417)
#define FFSM_DBA0                               PPE_REG_ADDR(0x508)
#define FFSM_DBA1                               PPE_REG_ADDR(0x509)
#define FFSM_CFG0                               PPE_REG_ADDR(0x50A)
#define FFSM_CFG1                               PPE_REG_ADDR(0x50B)
#define FFSM_IDLE_HEAD_BC0                      PPE_REG_ADDR(0x50E)
#define FFSM_IDLE_HEAD_BC1                      PPE_REG_ADDR(0x50F)

/*
 *  Reset Registers
 */
#define AMAZON_S_RCU_BASE_ADDR                  (KSEG1 | 0x1F203000)
#define AMAZON_S_RCU_RST_REQ                    ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x0010))
#define AMAZON_S_RCU_RST_STAT                   ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x0014))
#define AMAZON_S_USB_CFG                        ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x0018))
#define AMAZON_S_RCU_PPE_CONF                   ((volatile u32*)(AMAZON_S_RCU_BASE_ADDR + 0x002C))

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


#ifdef AR10_EMULATION
#define asc_puts                printk

#define REG32(addr)             (*((volatile u32*)(addr)))
#define BSP_REG32( addr )       (*(volatile u32 *)(addr))
#endif

/*
 *  macro for eth framework
 */
#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
  #define ifx_eth_fw_alloc_netdev(size, ifname, dummy)      alloc_netdev(size, ifname, ether_setup)
  #define ifx_eth_fw_free_netdev(netdev, dummy)             free_netdev(netdev)
  #define ifx_eth_fw_register_netdev(netdev)                register_netdev(netdev)
  #define ifx_eth_fw_unregister_netdev(netdev, dummy)       unregister_netdev(netdev)
  #define ifx_eth_fw_netdev_priv(netdev)                    netdev_priv(netdev)
#endif

/*
 * Chip ID Registers
 */
#define AR10_MIPS_CHIPID_BASE                   (KSEG1 | 0x1F107340)
#define AR10_MIPS_CHIPID                        (volatile u32*)(AR10_MIPS_CHIPID_BASE + 0x4)

/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *CHIP ID
 */
struct ar10_chipid{
    unsigned int   res0     :1;
    unsigned int   version  :3;
    unsigned int   pnum     :16;
    unsigned int   manid    :11;
    unsigned int   res1     :1;
};


/*
 *  Switch Header, Flag Header & Descriptor
 */
#if defined(__BIG_ENDIAN)
  struct sw_eg_pkt_header {
    //  byte 0
    unsigned int    res1                :5;
    unsigned int    spid                :3;
    //  byte 1
    unsigned int    crc_gen_dis         :1; //  0: enable CRC generation, 1: disable CRC generation
    unsigned int    res2                :4;
    unsigned int    dpid                :3;
    //  byte 2
    unsigned int    port_map_en         :1; //  0: ignore Dest Eth Port Map, 1: use Dest Eth Port Map (field port_map)
    unsigned int    port_map_sel        :1; //  0: field port_map is Dest Port Mask, 1: field port_map is Dest Port Map
    unsigned int    lrn_dis             :1; //  0/1: enable/disable source MAC address learning
    unsigned int    class_en            :1; //  0/1: disable/enable Target Traffic Class (field class)
    unsigned int    class               :4; //  Target Traffic Class
    //  byte 3
    unsigned int    res3                :1;
    unsigned int    port_map            :6;
    unsigned int    dpid_en             :1; //  0/1: disable/enable field dpid
  };

  struct flag_header {
    //  0 - 3h
    unsigned int    ipv4_rout_vld       :1;
    unsigned int    ipv4_mc_vld         :1;
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
    unsigned int    sppid               :3; //switch port id
    unsigned int    pkt_len             :13;//packet length
    unsigned int    pl_byteoff          :8; //bytes between flag header and fram payload
    unsigned int    mpoa_type           :2;
    unsigned int    ip_outer_offset     :6; //offset from the start of the Ethernet frame to the IP field

    // 12 - 15h
    unsigned int    tc                  :4; //switch traffic class
    unsigned int    res4                :28;
  };

  #define rx_descriptor                 tx_descriptor

#if 1
  struct tx_descriptor {
    //  0 - 3h
    unsigned int    own                 :1; //  0: MIPS, 1: PPE
    unsigned int    c                   :1;
    unsigned int    sop                 :1;
    unsigned int    eop                 :1;
    unsigned int    byteoff             :5;
    unsigned int    qid                 :4;
    unsigned int    res1                :2;
    unsigned int    small               :1;
    unsigned int    datalen             :16;
    //  4 - 7h
    unsigned int    dataptr             :32;
  };
#else

struct tx_descriptor {
  //  0 - 3h
  unsigned int    own                 :1; //  0: MIPS, 1: PPE
  unsigned int    c                   :1;
  unsigned int    sop                 :1;
  unsigned int    eop                 :1;
  unsigned int    byteoff             :5;
  unsigned int    qid                 :4;
  unsigned int    res1                :3;
  unsigned int    datalen             :16;
  //  4 - 7h

  unsigned int    small               :1;
  unsigned int    res2                :2;
  unsigned int    dataptr             :29;
};
#endif

#else
  #error no littel endian support
#endif

/*
 *  Internal Structure of Devices (ETH/ATM)
 */
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

    unsigned int                    dev_id;
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
 *  Network Operations (PTM)
 */
static void ptm_setup(struct net_device *);
static int ptm_ioctl(struct net_device *, struct ifreq *, int);
static void ptm_tx_timeout(struct net_device *);

/*
 *  Network Operations
 */
static void eth_setup(struct net_device *);
static struct net_device_stats *eth_get_stats(struct net_device *);
static int eth_open(struct net_device *);
static int eth_stop(struct net_device *);
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
static INLINE int ethtool_ioctl(struct net_device *, struct ifreq *);

/*
 *  DSL led flash function
 */
static INLINE void adsl_led_flash(void);

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
static int dma_int_handler(struct dma_device_info *, int);
static INLINE int dma_rx_int_handler(struct dma_device_info *);

/*
 *  Hardware init functions
 */
static INLINE void reset_ppe(void);
static INLINE void init_pmu(void);
static INLINE void start_cpu_port(void);
static INLINE void stop_datapath(void);
static INLINE void init_internal_tantos_qos_setting(void);
static INLINE void init_internal_tantos(void);
static INLINE void init_dplus(void);
static INLINE void init_ema(void);
static INLINE void init_mailbox(void);
static INLINE void clear_share_buffer(void);
static INLINE void clear_cdm(void);
static INLINE void board_init(void);
static INLINE void init_dsl_hw(void);
static INLINE void setup_ppe_conf(void);
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
#if defined(CONFIG_LTQ_PPA_DATAPATH)
  static INLINE void ethaddr_setup(unsigned int, char *);
#endif

/* DSL Showtime Function */
static int in_showtime(void);

/*
 *  DSL Data Led help function
 */
static void dsl_led_polling(unsigned long);

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

static int print_fw_ver_seq(struct seq_file *);

static int print_driver_ver_seq(struct seq_file *, char *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

static INLINE void proc_file_create(void);
static INLINE void proc_file_delete(void);

static int proc_read_ver(struct seq_file *, void *);

static int proc_read_mib(struct seq_file *, void *);
static ssize_t proc_write_mib_seq(struct file *, const char __user *, size_t, loff_t *);
static void clear_mib_counter(unsigned int );


#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC

  static void *proc_read_route_seq_start(struct seq_file *, loff_t *);
  static void *proc_read_route_seq_next(struct seq_file *, void *, loff_t *);
  static void proc_read_route_seq_stop(struct seq_file *, void *);
  static int proc_read_route_seq_show(struct seq_file *, void *);
  static int proc_read_route_seq_open(struct inode *, struct file *);
  int32_t ppa_route_start_iteration(uint32_t *ppos, struct rout_forward_compare_tbl **pcompare, uint32_t type);
  int32_t ppa_route_iterate_next(uint32_t *ppos, struct rout_forward_compare_tbl **pcompare, uint32_t type);
  void ppa_route_stop_iteration(void);
  static ssize_t proc_write_route_seq(struct file *, const char __user *, size_t, loff_t *);


  static int proc_read_mc(struct seq_file *, void *);

  static int proc_read_genconf(struct seq_file *, void *);
  static ssize_t proc_write_genconf_seq(struct file *, const char __user *, size_t, loff_t *);

  static int proc_read_pppoe(struct seq_file *, void *);

  static int proc_read_mtu(struct seq_file *, void *);

  static int proc_read_hit(struct seq_file *, void *);
  static ssize_t proc_write_hit_seq(struct file *, const char __user *, size_t, loff_t *);

  static int proc_read_mac(struct seq_file *, void *);

  static int proc_read_out_vlan(struct seq_file *, void *);

  static int proc_read_ipv6_ip(struct seq_file *, void *);

#endif

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static int proc_read_dbg(struct seq_file *, void *);
  static ssize_t proc_write_dbg_seq(struct file *, const char __user *, size_t, loff_t *);
#endif

#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
  static int proc_read_mem(struct seq_file *, void *);
  static inline unsigned long sb_addr_to_fpi_addr_convert(unsigned long);
  static ssize_t proc_write_mem_seq(struct file *, const char __user *, size_t, loff_t *);
#endif

#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
  static int proc_read_pp32(struct seq_file *, void *);
  static ssize_t proc_write_pp32_seq(struct file *, const char __user *, size_t, loff_t *);
#endif

static int proc_read_burstlen(struct seq_file *, void *);
static ssize_t proc_write_burstlen_seq(struct file *, const char __user *, size_t, loff_t *);

#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
  static int proc_read_fw(struct seq_file *, void *);
  static ssize_t proc_write_fw_seq(struct file *, const char __user *, size_t, loff_t *);

  static int proc_read_fwdbg(struct seq_file *, void *);
  static ssize_t proc_write_fwdbg_seq(struct file *, const char __user *, size_t, loff_t *);

  static void fw_dbg_start(char *cmdbuf);
  static void fwdbg_help(char **tokens, int token_num);
  static void dump_ipv4_hdr(struct iphdr * iph);
  static void dump_ipv6_hdr(struct ipv6hdr *iph6);

#endif


static INLINE unsigned int sw_get_rmon_counter(int, int);
static INLINE void sw_clr_rmon_counter(int);
static INLINE int read_port_counters(int, struct seq_file  *);

static int port0_counters_proc_read(struct seq_file *, void *);
static int port1_counters_proc_read(struct seq_file *, void *);
static int port2_counters_proc_read(struct seq_file *, void *);
static int port3_counters_proc_read(struct seq_file *, void *);
static int port4_counters_proc_read(struct seq_file *, void *);
static int port5_counters_proc_read(struct seq_file *, void *);
static int port6_counters_proc_read(struct seq_file *, void *);
static ssize_t port_counters_proc_write_seq(struct file *, const char __user *, size_t, loff_t *);

static int proc_read_directforwarding(struct seq_file *, void *);
static ssize_t proc_write_directforwarding_seq(struct file *, const char __user *, size_t, loff_t *);

static int proc_read_clk(struct seq_file *, void *);
static ssize_t proc_write_clk_seq(struct file *, const char __user *, size_t, loff_t *);

#if defined(ENABLE_NAPI) && ENABLE_NAPI
static int proc_read_napi(struct seq_file *, void *);
static ssize_t proc_write_napi_seq(struct file *, const char __user *, size_t, loff_t *);
#endif

static int proc_read_prio(struct seq_file *, void *);
static ssize_t proc_write_prio_seq(struct file *, const char __user *, size_t, loff_t *);

static int proc_read_ewan(struct seq_file *, void *);
static ssize_t proc_write_ewan_seq(struct file *, const char __user *, size_t, loff_t *);

#ifdef CONFIG_LTQ_PPA_QOS
  static int proc_read_qos(struct seq_file *, void *);
#endif

static int32_t ppa_datapath_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);

#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
  static int proc_read_mirror(struct seq_file *, void *);
static ssize_t proc_write_mirror_seq(struct file *, const char __user *, size_t, loff_t *);
#endif

static int proc_read_class(struct seq_file *, void *);
static ssize_t proc_write_class_seq(struct file *, const char __user *, size_t, loff_t *);

static int proc_read_mccfg(struct seq_file *, void *);
static ssize_t proc_write_mccfg_seq(struct file *, const char __user *, size_t, loff_t *);



static int proc_read_ver_seq_open(struct inode *, struct file *);
static int proc_read_mib_seq_open(struct inode *, struct file *);
//static int proc_read_capwap_seq_open(struct inode *, struct file *);
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
static int proc_read_mc_seq_open(struct inode *, struct file *);
static int proc_read_genconf_seq_open(struct inode *, struct file *);
static int proc_read_pppoe_seq_open(struct inode *, struct file *);
static int proc_read_mtu_seq_open(struct inode *, struct file *);
static int proc_read_hit_seq_open(struct inode *, struct file *);
static int proc_read_mac_seq_open(struct inode *, struct file *);
static int proc_read_out_vlan_seq_open(struct inode *, struct file *);
static int proc_read_ipv6_ip_seq_open(struct inode *, struct file *);
#endif
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
static int proc_read_dbg_seq_open(struct inode *, struct file *);
#endif
#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
static int proc_read_mem_seq_open(struct inode *, struct file *);
#endif
#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
static int proc_read_pp32_seq_open(struct inode *, struct file *);
#endif
static int proc_read_burstlen_seq_open(struct inode *, struct file *);
#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
static int proc_read_fw_seq_open(struct inode *, struct file *);
static int proc_read_fwdbg_seq_open(struct inode *, struct file *);
#endif
static int proc_read_port0_counters_seq_open(struct inode *, struct file *);
static int proc_read_port1_counters_seq_open(struct inode *, struct file *);
static int proc_read_port2_counters_seq_open(struct inode *, struct file *);
static int proc_read_port3_counters_seq_open(struct inode *, struct file *);
static int proc_read_port4_counters_seq_open(struct inode *, struct file *);
static int proc_read_port5_counters_seq_open(struct inode *, struct file *);
static int proc_read_port6_counters_seq_open(struct inode *, struct file *);
static int proc_read_directforwarding_seq_open(struct inode *, struct file *);
static int proc_read_clk_seq_open(struct inode *, struct file *);
static int proc_read_napi_seq_open(struct inode *, struct file *);
static int proc_read_prio_seq_open(struct inode *, struct file *);
static int proc_read_ewan_seq_open(struct inode *, struct file *);
#ifdef CONFIG_LTQ_PPA_QOS
static int proc_read_qos_seq_open(struct inode *, struct file *);
#endif
#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
static int proc_read_mirror_seq_open(struct inode *, struct file *);
#endif
static int proc_read_class_seq_open(struct inode *, struct file *);
static int proc_read_mccfg_seq_open(struct inode *, struct file *);



#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
static INLINE int print_mc_seq(struct seq_file *, int i, struct wan_rout_multicast_cmp_tbl *, struct wan_rout_multicast_act_tbl *, struct rtp_sampling_cnt *,uint32_t, uint32_t);
#else
static INLINE int print_mc_seq(struct seq_file *, int i, struct wan_rout_multicast_cmp_tbl *, struct wan_rout_multicast_act_tbl *, uint32_t , uint32_t );
#endif


#endif

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
static INLINE int print_mc(char *, int i, struct wan_rout_multicast_cmp_tbl *, struct wan_rout_multicast_act_tbl *, struct rtp_sampling_cnt *,uint32_t, uint32_t);
#else
static INLINE int print_mc(char *, int i, struct wan_rout_multicast_cmp_tbl *, struct wan_rout_multicast_act_tbl *, uint32_t , uint32_t );
#endif

#endif

/*Get Mailbox1 interrupt NO. */
static uint32_t get_mailbox1_irq_num(void);

/*
 *  Debug functions
 */
#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  static INLINE void dump_skb(struct sk_buff *, u32, char *, int, int, int, int);
#else
  #define dump_skb(a, b, c, d, e, f)
#endif
#if defined(DEBUG_DUMP_FLAG_HEADER) && DEBUG_DUMP_FLAG_HEADER
  static INLINE void dump_flag_header(int, struct flag_header *, const char *, int);
#else
  #define dump_flag_header(a, b, c, d)
#endif
#if defined(DEBUG_DUMP_INIT) && DEBUG_DUMP_INIT
  static INLINE void dump_init(int);
#else
  #define dump_init(i)
#endif
#ifdef CONFIG_LTQ_PORT_MIRROR
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
 *  Local MAC Address Tracking Functions
 */
static INLINE void register_netdev_event_handler(void);
static INLINE void unregister_netdev_event_handler(void);
static int netdev_event_handler(struct notifier_block *, unsigned long, void *);

/*
*  directpath tasklet
*/
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
static void directpath_task_send(unsigned long unused);
static int get_next_txq_skb(struct sk_buff **skb, int qid, int flag);
#endif


/*
 *  External Functions
 */
extern void (*ppa_hook_mpoa_setup)(struct atm_vcc *, int, int);
#if defined(CONFIG_LTQ_OAM) || defined(CONFIG_LTQ_OAM_MODULE)
  extern void ifx_push_oam(struct atm_vcc *atmvcc, unsigned char *);
#else
  static inline void ifx_push_oam(unsigned char *dummy) {}
#endif

/*
 *  External Variables
 */
extern struct sk_buff* (*ifx_atm_alloc_tx)(struct atm_vcc *, unsigned int);

/*
  * ATM Emulation functions
  */



/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static int g_fwcode = FWCODE_ROUTING_ACC_A5;

static struct semaphore g_sem;  //  lock used by open/close function

static int g_stop_datapath = 0;

static int g_eth_wan_mode = 0;
static unsigned int g_wan_itf = 1 << 7;

static int g_ipv6_acc_en = 1;
static unsigned int g_ipv4_lan_collision_rout_fwdc_table_base;
static unsigned int g_ipv4_wan_collision_rout_fwda_table_base;

static int g_wanqos_en = 0;

static int g_cpu_to_wan_tx_desc_pos = 0;
static DEFINE_SPINLOCK(g_cpu_to_wan_tx_spinlock);
static int g_cpu_to_wan_swap_desc_pos = 0;

static u32 g_mailbox_signal_mask;
static DEFINE_SPINLOCK(g_mailbox_signal_spinlock);

static int                      g_f_dma_opened = 0;
static struct dma_device_info  *g_dma_device = NULL;

static struct net_device *g_ptm_net_dev[1] = {0};
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
static struct net_device *g_ethoam_net_dev = NULL;
#endif

#if !defined(CONFIG_LANTIQ_ETH_FRAMEWORK) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
  static struct net_device_ops g_ptm_netdev_ops[1] = {
    {
    .ndo_open               = eth_open,
    .ndo_stop               = eth_stop,
    .ndo_get_stats          = eth_get_stats,
    .ndo_set_mac_address    = eth_set_mac_address,
    .ndo_start_xmit         = eth_qos_hard_start_xmit,
    .ndo_tx_timeout         = ptm_tx_timeout,
    .ndo_do_ioctl           = ptm_ioctl,
    },
  };
#endif

static int g_ptm_prio_queue_map[1][8];

#define LINE_NUMBER         1
#ifdef CONFIG_LTQMIPS_MEI_FW_LOOPBACK
  static int g_showtime[LINE_NUMBER] = {1};
#else
  static int g_showtime[LINE_NUMBER] = {0};
#endif
static void *g_xdata_addr = NULL;

static u32 g_dsl_wrx_correct_pdu[2] = {0};
static u32 g_dsl_wtx_total_pdu[2] = {0};
static struct timer_list g_dsl_led_polling_timer;
#if defined(ENABLE_LED_FRAMEWORK) && ENABLE_LED_FRAMEWORK
  static void *g_data_led_trigger = NULL;
#endif

static struct net_device *g_eth_net_dev[2] = {0};
static DEFINE_SPINLOCK(g_eth_tx_spinlock);

#if !defined(CONFIG_LANTIQ_ETH_FRAMEWORK) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
  static struct net_device_ops g_eth_netdev_ops[2] = {
    {
    .ndo_open               = eth_open,
    .ndo_stop               = eth_stop,
    .ndo_get_stats          = eth_get_stats,
    .ndo_set_mac_address    = eth_set_mac_address,
    .ndo_tx_timeout         = eth_tx_timeout,
    .ndo_do_ioctl           = eth_ioctl,
    },
    {
    .ndo_open               = eth_open,
    .ndo_stop               = eth_stop,
    .ndo_get_stats          = eth_get_stats,
    .ndo_set_mac_address    = eth_set_mac_address,
    .ndo_tx_timeout         = eth_tx_timeout,
    .ndo_do_ioctl           = eth_ioctl,
    },
  };
#endif

static int g_eth_prio_queue_map[2][8];
static int g_wan_queue_class_map[8];    //  So far, this table is shared by all interfaces.

static u8 g_my_ethaddr[MAX_ADDR_LEN * 2] = {0};

static struct proc_entry_cfg *g_eth_proc_entry = NULL;

static int g_dbg_enable = 0;

static unsigned int g_mib_itf = 0x83;

//  1:switch forward packets between port0/1 and cpu port directly w/o processing
//  0:pkts go through normal path and are processed by switch central function
static int g_ingress_direct_forward_enable = 0;
static int g_egress_direct_forward_enable = 1;

static int g_pmac_ewan = 0;    //  port0

/*
 *  variable for directpath
 */
static int g_directpath_dma_full = 0;
static DEFINE_SPINLOCK(g_directpath_tx_spinlock);
#if !defined(DISABLE_LOCAL_MAC_ADD_TO_SW) || !DISABLE_LOCAL_MAC_ADD_TO_SW
  static struct notifier_block g_netdev_event_handler_nb = {0};
#endif

#if defined(DEBUG_SKB_SWAP) && DEBUG_SKB_SWAP
  static struct sk_buff *g_dbg_skb_swap_pool[1024] = {0};
#endif

#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
  static struct net_device *g_mirror_netdev = NULL;
#endif

static int g_dump_cnt = -1;
static uint32_t AR10_MAILBOX_INT1 = INT_NUM_IM1_IRL29;
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
static int g_max_txq_num = 10;
DECLARE_TASKLET(g_directpath_task,directpath_task_send,0);
#endif
/*
 * ####################################
 *           Global Variable
 * ####################################
 */

/*
 *  variable for directpath
 */
#define     MAX_DIRECTPATH_NUM      3
struct ppe_directpath_data g_ppe_directpath_data[MAX_DIRECTPATH_NUM];    //  port 0 - eth0, 1 - eth1, 2 - cpu, 6 - dsl (PPPoA/IPoA), 7 - dsl (EoA)




/*
 * ####################################
 *            Local Function
 * ####################################
 */

#if !defined(ENABLE_LED_FRAMEWORK) || !ENABLE_LED_FRAMEWORK
 extern int ifx_mei_atm_led_blink(void);
#endif

static void ptm_setup(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    int val;
    int i;

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dev->get_stats       = eth_get_stats;
    dev->open            = eth_open;
    dev->stop            = eth_stop;
    dev->hard_start_xmit = eth_qos_hard_start_xmit;
    dev->set_mac_address = eth_set_mac_address;
    dev->do_ioctl        = ptm_ioctl;
    dev->tx_timeout      = ptm_tx_timeout;
  #else
    dev->netdev_ops      = &g_ptm_netdev_ops[0];
  #endif
    dev->watchdog_timeo  = ETH_WATCHDOG_TIMEOUT;
#endif
    priv->dev_id = 7;   //  DSL

    for ( i = 0, val = 0; i < 6; i++ )
        val += dev->dev_addr[i];
    if ( val == 0 )
    {
        dev->dev_addr[0] = 0x00;
        dev->dev_addr[1] = 0x20;
        dev->dev_addr[2] = 0xda;
        dev->dev_addr[3] = 0x86;
        dev->dev_addr[4] = 0x23;
        dev->dev_addr[5] = 0xee;
    }
}

static int ptm_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    switch ( cmd )
    {
    case IFX_PTM_MIB_CW_GET:
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxNoIdleCodewords   = PTM_MIB_TABLE(0)->wrx_nonidle_cw + PTM_MIB_TABLE(1)->wrx_nonidle_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxIdleCodewords     = PTM_MIB_TABLE(0)->wrx_idle_cw + PTM_MIB_TABLE(1)->wrx_idle_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxCodingViolation   = PTM_MIB_TABLE(0)->wrx_err_cw + PTM_MIB_TABLE(1)->wrx_err_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxNoIdleCodewords   = 0;    //  not available
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxIdleCodewords     = 0;    //  not available
        break;
    case IFX_PTM_MIB_FRAME_GET:
        {
            PTM_FRAME_MIB_T data = {0};

            data.RxCorrect   = PTM_MIB_TABLE(0)->wrx_correct_pdu + PTM_MIB_TABLE(1)->wrx_correct_pdu;
            data.RxDropped   = PTM_MIB_TABLE(0)->wrx_nodesc_drop_pdu + PTM_MIB_TABLE(0)->wrx_len_violation_drop_pdu + PTM_MIB_TABLE(1)->wrx_nodesc_drop_pdu + PTM_MIB_TABLE(1)->wrx_len_violation_drop_pdu;
            data.TxSend      = PTM_MIB_TABLE(0)->wtx_total_pdu + PTM_MIB_TABLE(1)->wtx_total_pdu;
            data.TC_CrcError = 0;   //  not available

            *((PTM_FRAME_MIB_T *)ifr->ifr_data) = data;
        }
        break;
    case IFX_PTM_CFG_GET:
        //  use bear channel 0 preemption gamma interface settings
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent = PTM_CRC_CFG->wrx_fcs_crc_vld;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck   = PTM_CRC_CFG->wrx_fcs_crc_chk;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck    = PTM_CRC_CFG->wrx_tc_crc_chk;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen      = PTM_CRC_CFG->wrx_tc_crc_len;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen     = PTM_CRC_CFG->wtx_fcs_crc_gen;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen      = PTM_CRC_CFG->wtx_tc_crc_gen;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen      = PTM_CRC_CFG->wtx_tc_crc_len;
        break;
    case IFX_PTM_CFG_SET:
        PTM_CRC_CFG->wrx_fcs_crc_vld = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent;
        PTM_CRC_CFG->wrx_fcs_crc_chk = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck;
        PTM_CRC_CFG->wrx_tc_crc_chk  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck;
        PTM_CRC_CFG->wrx_tc_crc_len  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen;
        PTM_CRC_CFG->wtx_fcs_crc_gen = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen;
        PTM_CRC_CFG->wtx_tc_crc_gen  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen;
        PTM_CRC_CFG->wtx_tc_crc_len  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen;
        break;
    case IFX_PTM_MAP_PKT_PRIO_TO_Q:
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static void ptm_tx_timeout(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    priv->tx_errors++;

    netif_wake_queue(dev);

    return;
}

static void eth_setup(struct net_device *dev)
{
    int port;
    u8 *ethaddr;
    u32 val;
    int i;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    for ( port = 0; port < NUM_ENTITY(g_eth_net_dev) && g_eth_net_dev[port] != dev; port++ );
    if ( port == NUM_ENTITY(g_eth_net_dev) )
        err("can not find net device (0x%08X) in local ethernet list", (unsigned int)dev);

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
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
    priv->dev_id = port;

    ethaddr = MY_ETH0_ADDR + port * MAX_ADDR_LEN;

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
}

static struct net_device_stats *eth_get_stats(struct net_device *dev)
{
    int port;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    port = get_eth_port(dev);
    if ( port < 0 )
        return NULL;

    priv->stats.rx_packets  = priv->rx_packets
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

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
    netif_start_queue(dev);
#endif

    if ( port == 7 )
    {
        g_dsl_wrx_correct_pdu[0] = PTM_MIB_TABLE(0)->wrx_correct_pdu;
        g_dsl_wtx_total_pdu[0]   = PTM_MIB_TABLE(0)->wtx_total_pdu;
        g_dsl_wrx_correct_pdu[1] = PTM_MIB_TABLE(1)->wrx_correct_pdu;
        g_dsl_wtx_total_pdu[1]   = PTM_MIB_TABLE(1)->wtx_total_pdu;
        g_dsl_led_polling_timer.expires = jiffies + HZ / 10;    //  100ms
        add_timer(&g_dsl_led_polling_timer);
    }

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

    if ( port == 7 )
        del_timer(&g_dsl_led_polling_timer);

    turn_off_dma_rx(port);

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
    netif_stop_queue(dev);
#endif

    up(&g_sem);
    return 0;
}

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
    volatile struct tx_descriptor *desc;
    struct tx_descriptor reg_desc;
    struct sk_buff *skb_to_free;
    int byteoff;
    int len;
    int snd_len;
    int padding = 0;
    struct sw_eg_pkt_header pkth = {0};
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    if ( g_stop_datapath != 0 )
        goto ETH_XMIT_DROP;

    port = get_eth_port(dev);
    if ( port < 0 )
        return -ENODEV;

    if( skb->len < ETH_MIN_TX_PACKET_LENGTH )
   {
       len = ETH_MIN_TX_PACKET_LENGTH;
       padding = 1;
   }else
   {
       len =  skb->len;
   }
  
    if ( port < NUM_ENTITY(g_eth_net_dev) )
    {
        if ( skb->cb[13] == 0x5A )  //  magic number indicating forcing QId (e.g. directpath)
            qid = skb->cb[15];
        else
        {
            if ( skb->priority >= NUM_ENTITY(g_eth_prio_queue_map[port]) )
                qid = g_eth_prio_queue_map[port][NUM_ENTITY(g_eth_prio_queue_map[port]) - 1];
            else
                qid = g_eth_prio_queue_map[port][skb->priority];
        }

        pkth.spid           = 2;    //  CPU
        pkth.dpid           = port; //  eth0/eth1
        pkth.lrn_dis        = 0;
        pkth.class_en       = PS_MC_GENCFG3->class_en;
        pkth.class          = g_wan_queue_class_map[qid];       //  0 is lowest priority, 3 is highest priority
        if ( pkth.dpid < 2 )
            pkth.dpid_en    = g_egress_direct_forward_enable;

        snd_len = len + sizeof(pkth);

        byteoff = (((unsigned int)skb->data - sizeof(pkth)) & (DMA_ALIGNMENT - 1)) + sizeof(pkth);
    }
    else
    {
        if ( !in_showtime() )
            goto ETH_XMIT_DROP;

        if ( skb->cb[13] == 0x5A )  //  magic number indicating forcing QId (e.g. directpath)
            qid = skb->cb[15];
        else
        {
            if ( skb->priority >= NUM_ENTITY(g_ptm_prio_queue_map[0]) )
                qid = g_ptm_prio_queue_map[0][NUM_ENTITY(g_ptm_prio_queue_map[0]) - 1];
            else
                qid = g_ptm_prio_queue_map[0][skb->priority];
        }

        snd_len = len;

        byteoff = (unsigned int)skb->data & (DMA_ALIGNMENT - 1);

        #if (defined(ENABLE_DFE_LOOPBACK_TEST) && ENABLE_DFE_LOOPBACK_TEST)
        do{
            unsigned char mac[6];
            unsigned char mac_broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            unsigned char ip[4];
            int off = 0;

            dump_skb(skb, DUMP_SKB_LEN, "ppe_send - before swap", 7, 0, 1, 0);

            if ( memcmp(skb->data + off, mac_broadcast, 6) == 0 )
                break;
            memcpy(mac, skb->data + off, 6);
            memcpy(skb->data + off, skb->data + off + 6, 6);
            memcpy(skb->data + off + 6, mac, 6);

            off += 12; //ethernet protocol
            while ( skb->data[off] == 0x81 && skb->data[off + 1] == 0x00 )  //  VLAN
                off += 4;

            if ( skb->data[off] == 0x08 && skb->data[off + 1] == 0x00 ) //  IP
            {
                off += 14; //ip src ipaddr offset
                memcpy(ip, skb->data + off, 4);
                memcpy(skb->data + off, skb->data + off + 4, 4);
                memcpy(skb->data + off + 4, ip, 4);
            }
        }while(0);
        #endif //ENABLE_DFE_LOOPBACK_TEST
    }

    dump_skb(skb, DUMP_SKB_LEN, "eth_qos_hard_start_xmit", port, qid, 1, 0);

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
        if ( port < NUM_ENTITY(g_eth_net_dev) )
        {
            skb_put(new_skb, sizeof(pkth) + skb->len);
            my_memcpy(new_skb->data, &pkth, sizeof(pkth));
            my_memcpy(new_skb->data + sizeof(pkth), skb->data, skb->len);
        }
        else
        {
            skb_put(new_skb, skb->len);
            my_memcpy(new_skb->data, skb->data, skb->len);
        }
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
        if ( port < NUM_ENTITY(g_eth_net_dev) )
        {
            skb_push(skb, sizeof(pkth));
            byteoff -= sizeof(pkth);
            my_memcpy(skb->data, &pkth, sizeof(pkth));
        }
        *(struct sk_buff **)((u32)skb->data - byteoff - sizeof(struct sk_buff *)) = skb;
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((u32)skb->data - byteoff - sizeof(struct sk_buff *), skb->len + byteoff + sizeof(struct sk_buff *));
#endif
    }
    //fix padding
    if( padding )
    {
        if( skb_padto(skb,snd_len) == 0 )
        {
            dma_cache_wback((u32)skb->data, snd_len);
        }
    }

    /*  allocate descriptor */
    spin_lock_irqsave(&g_cpu_to_wan_tx_spinlock, sys_flag);
    desc = (volatile struct tx_descriptor *)(CPU_TO_WAN_TX_DESC_BASE + g_cpu_to_wan_tx_desc_pos);
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

#ifdef CONFIG_LTQ_PORT_MIRROR
    if ( (g_wan_itf & (1 << port)) && ifx_get_mirror_netdev && ifx_get_mirror_netdev() != NULL )
	{
		mirror_pkt_eth(skb, port < NUM_ENTITY(g_eth_net_dev) ? sizeof(pkth) : 0);
	}
#endif


#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
    if ( g_mirror_netdev != NULL )
    {
        struct sk_buff *new_skb = skb_copy(skb, GFP_ATOMIC);

        if ( new_skb != NULL )
        {
            if ( port < NUM_ENTITY(g_eth_net_dev) )
                skb_pull(new_skb, sizeof(pkth));
            new_skb->dev = g_mirror_netdev;
            dev_queue_xmit(new_skb);
        }
    }
#endif

    /*  update descriptor   */
    reg_desc.small      = (unsigned int)skb->end - (unsigned int)skb->data < DMA_PACKET_SIZE ? 1 : 0;
    reg_desc.dataptr    = (u32)skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1));  //  byte address
    reg_desc.byteoff    = byteoff;
    reg_desc.datalen    = snd_len;
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
        return ethtool_ioctl(dev, ifr);
    case SET_VLAN_COS:
        {
            struct vlan_cos_req vlan_cos_req;

            if ( copy_from_user(&vlan_cos_req, ifr->ifr_data, sizeof(struct vlan_cos_req)) )
                return -EFAULT;
        }
        break;
    case SET_DSCP_COS:
        {
            struct dscp_cos_req dscp_cos_req;

            if ( copy_from_user(&dscp_cos_req, ifr->ifr_data, sizeof(struct dscp_cos_req)) )
                return -EFAULT;
        }
        break;
    case ETH_MAP_PKT_PRIO_TO_Q:
        {
            struct ppe_prio_q_map cmd;

            if ( copy_from_user(&cmd, ifr->ifr_data, sizeof(cmd)) )
                return -EFAULT;

            if ( cmd.pkt_prio < 0 || cmd.pkt_prio >= NUM_ENTITY(g_eth_prio_queue_map[port]) )
                return -EINVAL;

            if ( cmd.qid < 0 )
                return -EINVAL;
            if ( cmd.qid >= ((g_wan_itf & (1 << port)) && (g_eth_wan_mode == 3) && g_wanqos_en ? __WAN_TX_QUEUE_NUM : __VIRTUAL_TX_QUEUE_NUM) )
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
    return dev == g_eth_net_dev[0] ? 0 : (dev == g_eth_net_dev[1] ? 1 : (dev == g_ptm_net_dev[0] ? 7 : -1));
}

static INLINE int eth_xmit(struct sk_buff *skb, unsigned int port, int ch, int spid, int class)
{
    struct sw_eg_pkt_header pkth = {0};
    int len;
    struct eth_priv_data *priv = NULL;

    if ( g_stop_datapath != 0 )
        goto ETH_XMIT_DROP;

    //len = skb->len <= ETH_MIN_TX_PACKET_LENGTH ? ETH_MIN_TX_PACKET_LENGTH : skb->len;
    //fix the padding problem: switch will do the padding
    len = skb->len;
#ifdef CONFIG_LTQ_PORT_MIRROR
    if ( ifx_get_mirror_netdev && ifx_get_mirror_netdev() != NULL )
	{
        	mirror_pkt_eth(skb, 4);
	}
#endif

#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
    if ( g_mirror_netdev != NULL && (g_wan_itf & (1 << port)) )
    {
        struct sk_buff *new_skb = skb_copy(skb, GFP_ATOMIC);

        if ( new_skb != NULL )
        {
            new_skb->dev = g_mirror_netdev;
            dev_queue_xmit(new_skb);
        }
    }
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

    pkth.spid           = spid;
    pkth.dpid           = port;
    pkth.lrn_dis        = 0;
    
    PPA_PORT_SEPARATION_TX(skb, pkth, lan_port_seperate_enabled, wan_port_seperate_enabled, port );
    if ( class >= 0 )
    {
        pkth.class_en   = PS_MC_GENCFG3->class_en;
        pkth.class      = class;
    }
    if ( pkth.dpid < 2 )
        pkth.dpid_en    = g_egress_direct_forward_enable;
    else if ( pkth.dpid == 2 )
        pkth.dpid_en    = 1;

    memcpy(skb->data, &pkth, sizeof(struct sw_eg_pkt_header));

    if ( port < NUM_ENTITY(g_eth_net_dev))
    {
        g_eth_net_dev[port]->trans_start = jiffies;
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[port]);
        priv->tx_packets++;
        priv->tx_bytes += len;
    }
    else if ( port == 7 )
    {
        g_ptm_net_dev[0]->trans_start = jiffies;
        priv = ifx_eth_fw_netdev_priv(g_ptm_net_dev[0]);
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
static INLINE int ethtool_ioctl(struct net_device *dev, struct ifreq *ifr)
{
    int port;
    struct ethtool_cmd cmd;

    if ( copy_from_user(&cmd, ifr->ifr_data, sizeof(cmd)) )
        return -EFAULT;

    port = get_eth_port(dev);

    switch ( cmd.cmd )
    {
    case ETHTOOL_GSET:      /*  get hardware information        */
        {
            memset(&cmd, 0, sizeof(cmd));
            if ( copy_to_user(ifr->ifr_data, &cmd, sizeof(cmd)) )
                return -EFAULT;
        }
        break;
    case ETHTOOL_SSET:      /*  force the speed and duplex mode */
        {
            if ( !capable(CAP_NET_ADMIN) )
                return -EPERM;

            if ( cmd.autoneg == AUTONEG_ENABLE )
            {
                /*  set property and start autonegotiation                                  */
                /*  have to set mdio advertisement register and restart autonegotiation     */
                /*  which is a very rare case, put it to future development if necessary.   */
            }
            else
            {
            }
        }
        break;
    case ETHTOOL_GDRVINFO:  /*  get driver information          */
        {
            struct ethtool_drvinfo info;
            char str[32];

            memset(&info, 0, sizeof(info));
            strncpy(info.driver, "Danube Eth Driver (A4)", sizeof(info.driver) - 1);
            sprintf(str, "%d.%d.%d.%d.%d", (int)FW_VER_ID->family, (int)FW_VER_ID->package, (int)FW_VER_ID->major, (int)FW_VER_ID->middle, (int)FW_VER_ID->minor);
            strncpy(info.fw_version, str, sizeof(info.fw_version) - 1);
            strncpy(info.bus_info, "N/A", sizeof(info.bus_info) - 1);
            info.regdump_len = 0;
            info.eedump_len = 0;
            info.testinfo_len = 0;
            if ( copy_to_user(ifr->ifr_data, &info, sizeof(info)) )
                return -EFAULT;
        }
        break;
    case ETHTOOL_NWAY_RST:  /*  restart auto negotiation        */
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static INLINE void adsl_led_flash(void)
{
#if defined(ENABLE_LED_FRAMEWORK) && ENABLE_LED_FRAMEWORK
#ifdef CONFIG_LTQ_LED
    if ( g_data_led_trigger != NULL )
        ifx_led_trigger_activate(g_data_led_trigger);
#endif
#else
    ifx_mei_atm_led_blink();
#endif
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

    spin_lock(&g_mailbox_signal_spinlock);

    mailbox_signal = *MBOX_IGU1_ISR & g_mailbox_signal_mask;
    if ( !mailbox_signal )
        goto EXIT_MAILBOX_IRQ_HANDLER;
    *MBOX_IGU1_ISRC = mailbox_signal;

    if ( (mailbox_signal & WAN_RX_SIG(0)) ) //  PTM Packet
    {
        g_mailbox_signal_mask &= ~WAN_RX_SIG(0);
        g_dma_device->tx_chan[1]->open(g_dma_device->tx_chan[1]);
        *MBOX_IGU1_IER = g_mailbox_signal_mask;
        dbg("WAN_RX_SIG(0)");
    }

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

EXIT_MAILBOX_IRQ_HANDLER:
    spin_unlock(&g_mailbox_signal_spinlock);
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
                    g_dma_device->rx_chan[i]->dir = 1;  //  IFX_DMA_TX_CH
                g_dma_device->rx_chan[i]->open(g_dma_device->rx_chan[i]);
                if ( i == 1 || i == 2 )
                    g_dma_device->rx_chan[i]->dir = 0;  //  IFX_DMA_RX_CH
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

    dump_flag_header(g_fwcode, (struct flag_header *)skb->data, __FUNCTION__, dma_dev->current_rx_chan);

    header = (struct flag_header *)skb->data;

    //  pl_byteoff
    off = header->pl_byteoff;

    skb->tail = skb->data + len;
    skb->len =  len;
    dump_skb(skb, DUMP_SKB_LEN, "dma_rx_int_handler raw data", header->src_itf, dma_dev->current_rx_chan, 0, 0);
   // printk("The payload offset: %d:(0x%x)\n",  sizeof(struct flag_header) + off, sizeof(struct flag_header) + off);

    len -=  off + 8; //8 is 2 leading dword
    ASSERT(len >= 60, "pakcet is too small: %d", len);

    skb->data +=  off + 8;
    skb->tail =  skb->data + len;
    skb->len  =  len;
    
    PPA_PORT_SEPARATION_RX(header,skb,lan_port_seperate_enabled,wan_port_seperate_enabled);
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
    // To be used by sw acceleration to understand the source of packet
    // this needs to be replaced by the new mark field
    skb->mark |= FLG_PPA_PROCESSED;   
#endif
    
    skb->priority = header->tc;

    dump_skb(skb, DUMP_SKB_LEN, "dma_rx_int_handler raw data", header->src_itf, dma_dev->current_rx_chan, 0, 0);

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
                skb->cb[13] = 0x5A; //  magic number indicating forcing QId (e.g. directpath)
                skb->cb[15] = header->qid & 0x07;
                eth_qos_hard_start_xmit(skb, g_eth_net_dev[itf]);
            }
            else
                eth_xmit(skb, itf, 2, 2, g_wan_queue_class_map[header->qid & 0x07]);
            return 0;
        }
        else if ( (header->dest_list & (1 << 7)) )      //  7 - ptm0
        {
            skb->cb[13] = 0x5A; //  magic number indicating forcing QId (e.g. directpath)
            skb->cb[15] = header->qid & 0x07;
            eth_qos_hard_start_xmit(skb, g_ptm_net_dev[0]);
            return 0;
        }
        else
        {
            for ( off = 3; off < 3 + MAX_DIRECTPATH_NUM; off++ )
            {
                if ( (header->dest_list & (1 << off)) ) //  3...6
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

            if ( netif_rx(skb) == NET_RX_DROP )
            {
                priv->rx_dropped++;
            }
            else
            {
                priv->rx_packets++;
                priv->rx_bytes += skb->len;
            }

            return 0;
        }
        else
            priv->rx_dropped++;
        break;
    case 7: //  PTM
        if ( !g_ptm_net_dev[0] )    //  Ethernet WAN mode, PTM device is not initialized, but it might get ingress packet, just drop the packet.
            break;
        priv = ifx_eth_fw_netdev_priv(g_ptm_net_dev[0]);
        if ( netif_running(g_ptm_net_dev[0]) )
        {
            // PPA firmware will remove CRC
            //skb->tail -= ETH_CRC_LENGTH;
            //skb->len  -= ETH_CRC_LENGTH;

            skb->dev = g_ptm_net_dev[0];
            skb->protocol = eth_type_trans(skb, skb->dev);
#if defined(CONFIG_LTQ_ETH_OAM) || defined(CONFIG_LTQ_ETH_OAM_MODULE)
            if (skb->protocol == 0x8902) {
                   if(fp_ltq_eth_oam_dev != NULL) {
                     g_ethoam_net_dev = fp_ltq_eth_oam_dev();
                     skb->dev = g_ethoam_net_dev;
                     priv_oam = ifx_eth_fw_netdev_priv(skb->dev);
                   }
             }
#endif

            if ( netif_rx(skb) == NET_RX_DROP )
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

            adsl_led_flash();

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

#ifdef MODULE
static int pseudo_dma_int_handler(struct dma_device_info *dma_dev, int status)
{
    int ret = 0;
    struct sk_buff *skb = NULL;
    u8 *buf;

    switch ( status )
    {
    case RCV_INT:
        dma_device_read(dma_dev, &buf, (void **)&skb);
        if ( skb )
            dev_kfree_skb_any(skb);
    }

    return ret;
}
#endif

static INLINE void reset_ppe(void)
{
#ifdef MODULE
    unsigned long org_jiffies;
    int i;

    //if ( (*DM_RXCFG & 0xFF) == 0x30 )
    //    return;

    g_dma_device = dma_device_reserve("PPE");
    if ( !g_dma_device )
    {
        err("Reserve DMA for PPE failed");
        return;
    }

    g_dma_device->buffer_alloc    = dma_buffer_alloc;
    g_dma_device->buffer_free     = dma_buffer_free;
    g_dma_device->intr_handler    = pseudo_dma_int_handler;
    g_dma_device->tx_burst_len    = 8;
    g_dma_device->rx_burst_len    = 8;
    g_dma_device->port_packet_drop_enable = 1;

    for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
    {
        g_dma_device->rx_chan[i]->packet_size = DMA_PACKET_SIZE;
        g_dma_device->rx_chan[i]->control     = IFX_DMA_CH_ON;
    }
    for ( i = 0; i < g_dma_device->max_tx_chan_num; i++ )
        g_dma_device->tx_chan[i]->control     = IFX_DMA_CH_ON;

    dma_device_register(g_dma_device);

    for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
        g_dma_device->rx_chan[i]->open(g_dma_device->rx_chan[i]);

    org_jiffies = jiffies;
    while ( (IFX_REG_R32(DM_RXPKTCNT) & 0x0FF) != 0 && jiffies - org_jiffies < HZ / 10 + 1 )
        schedule();

    //  reset PPE
   // ifx_rcu_rst(IFX_RCU_DOMAIN_DSLDFE, IFX_RCU_MODULE_PPA);
   ltq_rcu_w32_mask(0, BIT(7), RCU_RST_REQ);
    udelay(100);
   // ifx_rcu_rst(IFX_RCU_DOMAIN_DSLTC, IFX_RCU_MODULE_PPA);
   ltq_rcu_w32_mask(0, BIT(23), RCU_RST_REQ);
    udelay(100);
   ltq_rcu_w32_mask(0, BIT(8), RCU_RST_REQ);
   // ifx_rcu_rst(IFX_RCU_DOMAIN_PPE, IFX_RCU_MODULE_PPA);
    udelay(100);

    for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
        g_dma_device->rx_chan[i]->reset(g_dma_device->rx_chan[i]);

    dma_device_unregister(g_dma_device);
    dma_device_release(g_dma_device);
    g_dma_device = NULL;
#endif
}

static INLINE void init_pmu(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)

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
    PPE_TC_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_EMA_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_QSB_PMU_SETUP(IFX_PMU_ENABLE);
    //PPE_TPE_PMU_SETUP(IFX_PMU_ENABLE);
    DSL_DFE_PMU_SETUP(IFX_PMU_ENABLE);
    PPE_DPLUS_PMU_SETUP(IFX_PMU_ENABLE);
    SWITCH_PMU_SETUP(IFX_PMU_ENABLE);
#else
    struct clk *clk;

    /*Enable Switch Power  */
   clk = clk_get_sys("1e108000.eth", NULL);
   clk_enable(clk);

#endif
}

#ifdef AR10_EMULATION

enum{
    MII_PHY_MODE = 0,
    MII_MAC_MODE,
    RMII_PHY_MODE,
    RMII_MAC_MODE,
    RGMII_ONLY_MODE,
};

static u8 xMii_interface_mode[6] = {
                    RGMII_ONLY_MODE,
                    RGMII_ONLY_MODE,
                    RGMII_ONLY_MODE,
                    MII_MAC_MODE,
                    RGMII_ONLY_MODE,
                    RGMII_ONLY_MODE,
};

//for emulation only: in venus, there is not real phy connection. It's mac to mac connection.
//So must force link up and set the link rate
static INLINE void arx_ethsw_port_enable(int port)
{
    if(port < 0 || port > 5)
        return;

    *PHY_ADDR_REG(port) &= ~(0x1f<<11);
    if (xMii_interface_mode[port] == RGMII_ONLY_MODE)
        *PHY_ADDR_REG(port) |= (0x2<<11); // 1G data rate
    *PHY_ADDR_REG(port) |= (1<<13);// Link Up

    return;
}
#endif
static INLINE void start_cpu_port(void)
{
    int i;

    *DS_RXCFG |= 0x00008000;    //  enable DSRX
    *DM_RXCFG |= 0x80000000;    //  enable DMRX

    for ( i = 0; i < 7; i++ ) {
        IFX_REG_W32_MASK(0, 1, FDMA_PCTRL_REG(i));  //  start port 0 - 6
        IFX_REG_W32_MASK(0, 1, SDMA_PCTRL_REG(i));  //  start port 0 - 6
    }
    *PCE_PCTRL_REG(6, 0) |= 0x07;   //  clear_dplus
}

static INLINE void stop_datapath(void)
{
    unsigned long org_jiffies;
    int i;

    g_stop_datapath = 1;    //  going to be unloaded

    //  disable switch external port
    for ( i = 0; i < 6; i++ ) {
        IFX_REG_W32_MASK(1, 0, SDMA_PCTRL_REG(i));  //  stop port 0 - 5
        IFX_REG_W32_MASK(0, 1, FDMA_PCTRL_REG(i));
    }

    //  turn on DMA RX channels
    turn_on_dma_rx(31);

    //  clean ingress datapath
    org_jiffies = jiffies;
    while ( ((IFX_REG_R32(DM_RXPGCNT) & 0x00FF) != 0 || (IFX_REG_R32(DM_RXPKTCNT) & 0x00FF) != 0 || (IFX_REG_R32(DS_RXPGCNT) & 0x00FF) != 0) && jiffies - org_jiffies < HZ / 10 + 1 )
        schedule();
    if ( jiffies - org_jiffies > HZ / 10 )
        err("Can not clear DM_RXPGCNT/DM_RXPKTCNT/DS_RXPGCNT");

    //  turn off DMA RX channels (including loopback RX channels)
    while ( (i = clz(g_f_dma_opened)) >= 0 )
        turn_off_dma_rx(i);

    //  turn off DMA Loopback TX channels
    g_dma_device->tx_chan[0]->close(g_dma_device->tx_chan[0]);
    g_dma_device->tx_chan[1]->close(g_dma_device->tx_chan[1]);

    *PMAC_HD_CTL_REG = 0x0000074C;  //  recover to default value
    *FDMA_PCTRL_REG(6) &= ~0x02;
}

static INLINE void init_internal_tantos_qos_setting(void)
{
}

static INLINE void init_internal_tantos(void)
{
    int i;

    *GLOB_CTRL_REG = 1 << 15;   //  enable Switch

    for ( i = 0; i < 7; i++ )
        sw_clr_rmon_counter(i); /*Reset and Enable Rom counter*/

    *FDMA_PCTRL_REG(6) = (*FDMA_PCTRL_REG(6) | 0x02);   //  insert special tag

    /*
              PMAC-->DMA :       Remove Special Header and Add Status Header / Remove CRC
              DMA--->PMAC:      With Status Header
              PMAC-->SWITCH:  Remove Status Header and Add Special Header
       */
#if defined(CONFIG_SUPPORT_PMAC_FLOWCONTROL)
       *PMAC_HD_CTL_REG = 0x05DC;  //  PMAC Head  + enable Flow Control
#else
       *PMAC_HD_CTL_REG = 0x01DC;  //  PMAC Head
#endif


    for ( i = 6; i < 12; i++ )
        *PCE_PCTRL_REG(i, 0) |= 1 << 11;    //  ingress special tag

    /*Max frame length */
#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
    *MAC_FLEN_REG = 1620; //1604 + 8 + 4 * 2;   //  MAC frame + 8-byte special tag + 4-byte VLAN tag * 2
#else
    *MAC_FLEN_REG = 1534; //1518 + 8 + 4 * 2;   //  MAC frame + 8-byte special tag + 4-byte VLAN tag * 2
#endif

    *MAC_CTRL_REG(6, 2) |= 1 << 3;      //  enable jumbo frame

    for(i = 0; i < 7; i ++){
        *MAC_CTRL_REG(i,0) |= 1 << 7;   //set  FCS generation Enable for each port
    }

    //*PCE_PMAP_REG(2) = 0x7F & ~g_pmac_ewan; //  broadcast and unknown multicast
    *PCE_PMAP_REG(2) = 0x7F;                //  broadcast and unknown multicast
    //*PCE_PMAP_REG(3) = 0x7F & ~g_pmac_ewan; //  unknown uni-cast
    *PCE_PMAP_REG(3) = 0x7F;                //  unknown uni-cast

    /*WAN group, port0~port5 */
    *PMAC_EWAN_REG = g_pmac_ewan;

    *PMAC_RX_IPG_REG = 0x8F;

#ifdef AR10_EMULATION
    //venus emulation code, disable phy automatic polling function
    *MDC_CFG_0_REG = 0;
#endif

    init_internal_tantos_qos_setting();
}

static INLINE void init_dplus(void)
{
    *DM_RXCFG &= ~0x80000000;   //  disable DMRX
    *DS_RXCFG &= ~0x00008000;   //  disable DSRX
    *DM_RXPGCNT = 0x00020000;   //  clear page pointer & counter
    *DS_RXPGCNT = 0x00040000;   //  clear page pointer & counter

    *DM_RXDB    = 0x1450;       //  Set Dplus Master Data base to 0x1450 which would be mapped to 0x3450
    *DM_RXCB    = 0x143C;       //  Set Dplus Master Control Base address to 0x143C
    *DM_RXPKTCNT= 0x00000200;   //  Set Dplus Master Packet counter in FW mode
    //HEADLEN: 12(Flag_header)+4(outervlan)+4(innervlan)+8(pppoe_header)+40(IPv6 header)+2(space)-8(switch header)=62
    *DM_RXCFG   = 0x00F87014;
    *DS_RXCFG   = 0x7014;
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

static INLINE void init_dsl_hw(void)
{
    IFX_REG_W32(0x0,        RFBI_CFG);
    IFX_REG_W32(0x1800,     SFSM_DBA0);
    IFX_REG_W32(0x192C,     SFSM_DBA1);
    IFX_REG_W32(0x1918,     SFSM_CBA0);
    IFX_REG_W32(0x1A44,     SFSM_CBA1);
    IFX_REG_W32(0x14010,    SFSM_CFG0);
    IFX_REG_W32(0x14010,    SFSM_CFG1);
    IFX_REG_W32(0x1750,     FFSM_DBA0);
    IFX_REG_W32(0x10E0,     FFSM_DBA1);
    IFX_REG_W32(0x3000A,    FFSM_CFG0);
    IFX_REG_W32(0x3000A,    FFSM_CFG1);
    IFX_REG_W32(0xF0D10000, FFSM_IDLE_HEAD_BC0);
    IFX_REG_W32(0xF0D10000, FFSM_IDLE_HEAD_BC1);
}


static INLINE void setup_ppe_conf(void)
{
    // Disable PPE and MIPS access to DFE memory
/*
    PPE_CONF setup as required by DSL
    Bit#1 of PPE_CONF  to 0  '  This implies dsl_afe_rst_eco_s(0) to 0
    Bit#2 of PPE_CONF  to 0  '  This implies dsl_afe_rst_eco_s(1) to 0
    Bit#3 of PPE_CONF  to 1  '  This implies dsl_afe_rst_eco_s(2) to 1
    Bit#4 of PPE_CONF  to 1  '  This implies dsl_afe_rst_eco_s(3) to 1
*/

  *AMAZON_S_RCU_PPE_CONF = (*AMAZON_S_RCU_PPE_CONF & 0x7FFFFFE1) | 0x18;
}

static inline void set_ppe_250(void)
{
    *AMAZON_S_CGU_SYS = ((*AMAZON_S_CGU_SYS & ~(0x70000)) | 0x10000);
}

static INLINE void init_hw(void)
{
    set_ppe_250();

    init_pmu();

    reset_ppe();

    setup_ppe_conf();

    init_internal_tantos();

    init_dplus();

    init_dsl_hw();

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
    ret = pp32_download_code(ar10_e5_fw_code, sizeof(ar10_e5_fw_code) / sizeof(*ar10_e5_fw_code), ar10_e5_fw_data, sizeof(ar10_e5_fw_data) / sizeof(*ar10_e5_fw_data));
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

    //  support DSL WAN only
    g_eth_wan_mode = 0;

    if ( wanitf == ~0 || wanitf == 0)
    {
        switch ( g_eth_wan_mode )
        {
        case 0: /*  DSL WAN     */  g_wan_itf = 1 << 7; break;
        case 2: /*  Mixed Mode  */  g_wan_itf = 1 << 0; break;
        case 3: /*  ETH WAN     */  g_wan_itf = 1 << 1; break;
        }
    }
    else
    {
        g_wan_itf = wanitf;
        switch ( g_eth_wan_mode )
        {
        case 0: /*  DSL WAN     */
                g_wan_itf &= 0xF8; //  ETH0/1 can not be WAN
                g_wan_itf |= 1 << 7; //set DSL to WAN
                break;
        case 2: /*  Mixed Mode  */
                g_wan_itf &= 0x03; //  only ETH0/1 support mixed mode
                if(g_wan_itf == 0x03 || g_wan_itf == 0){ //both ETH0/1 in mixed mode or improper setting, reset to eth0 mixed mode
                    g_wan_itf = 1;
                }
                break;
        case 3: /*  ETH WAN     */
                g_wan_itf &= 0x3B; //  DSL disabled in ETH WAN mode
                if((g_wan_itf & 0x03) == 0x03){ //both ETH0/1 in WAN mode, remove eth0 wan setting
                    g_wan_itf &= ~0x01;
                }
                if((g_wan_itf & 0x03) == 0){ //if improper setting, set eth1 to WAN
                    g_wan_itf |= 1 << 1;
                }
                break;
        }


    }

    g_ipv6_acc_en = ipv6_acc_en ? 1 : 0;
    g_ipv4_lan_collision_rout_fwdc_table_base = __IPV4_LAN_COLLISION_ROUT_FWDC_TABLE_BASE;
    g_ipv4_wan_collision_rout_fwda_table_base = __IPV4_WAN_COLLISION_ROUT_FWDA_TABLE_BASE;

    if ( g_eth_wan_mode == 0 )
        g_wanqos_en = wanqos_en ? wanqos_en : 8;    //  always enable QoS in PTM WAN mode
    else
        g_wanqos_en = g_eth_wan_mode == 3 && (g_wan_itf & 0x03) != 0x03 && wanqos_en ? wanqos_en : 0;

    g_mailbox_signal_mask = g_wanqos_en ? CPU_TO_WAN_SWAP_SIG : 0;
    if ( g_eth_wan_mode == 0 )
        g_mailbox_signal_mask |= WAN_RX_SIG(0);

    //memset(g_eth_priv_data, 0, sizeof(g_eth_priv_data));

    for ( i = 0; i < 2; i++ )
    {
        int max_packet_priority = NUM_ENTITY(g_eth_prio_queue_map[i]);  //  assume only 8 levels are used in Linux
        int tx_num_q;
        int q_step, q_accum, p_step;

        tx_num_q = (g_wan_itf & (1 << i)) && g_wanqos_en ? __WAN_TX_QUEUE_NUM : __VIRTUAL_TX_QUEUE_NUM;
        q_step = tx_num_q - 1;
        p_step = max_packet_priority - 1;
        for ( j = 0, q_accum = 0; j < max_packet_priority; j++, q_accum += q_step )
            g_eth_prio_queue_map[i][j] = q_step - (q_accum + (p_step >> 1)) / p_step;
    }

    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
    {
        int max_packet_priority = NUM_ENTITY(g_ptm_prio_queue_map[0]);  //  assume only 8 levels are used in Linux
        int tx_num_q;
        int q_step, q_accum, p_step;

        tx_num_q = __WAN_TX_QUEUE_NUM;
        q_step = tx_num_q - 1;
        p_step = max_packet_priority - 1;
        for ( j = 0, q_accum = 0; j < max_packet_priority; j++, q_accum += q_step )
            g_ptm_prio_queue_map[0][j] = (q_accum + (p_step >> 1)) / p_step;

        for ( j = 0; j < max_packet_priority; j++ )
            g_ptm_prio_queue_map[0][j] = q_step - g_ptm_prio_queue_map[0][j];
    }

    for ( i = 0; i < NUM_ENTITY(g_wan_queue_class_map); i++ )
        g_wan_queue_class_map[i] = (NUM_ENTITY(g_wan_queue_class_map) - 1 - i) % SWITCH_CLASS_NUM;

    return 0;
}

static INLINE void clear_local_variables(void)
{
}

#if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST

#include "cfg_arrays_a5.h"

unsigned int ppe_fw_mode        = 0; // 0: normal mode, 1: mix mode
unsigned int ppe_wan_hash_algo  = 0; // 0: using algo 0, 1: using algo 1
unsigned int acc_proto      = 0; // 0: UDP, 1:TCP

void setup_acc_action_tbl(void)
{
    unsigned int idx;

    if (acc_proto == 0) {
        // clear bit16 of dw3 of each action entry
        unsigned long udp_mask = ~ (1 << 16);

        // setup for Hash entry
        idx = 3;
        while (idx < sizeof(lan_uc_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
            lan_uc_act_tbl_normal_mode_cfg[idx] &= udp_mask;
            lan_uc_act_tbl_mixed_mode_cfg[idx] &= udp_mask;

            wan_uc_act_tbl_alo_0_cfg[idx] &= udp_mask;
            wan_uc_act_tbl_alo_1_cfg[idx] &= udp_mask;
            idx += 6;
        }

        // setup for Collsion entry
        idx = 3;
        while (idx < sizeof(lan_uc_col_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
            lan_uc_col_act_tbl_normal_mode_cfg[idx] &= udp_mask;
            lan_uc_col_act_tbl_mixed_mode_cfg[idx] &= udp_mask;
            wan_uc_col_act_tbl_cfg[idx] &= udp_mask;
            idx += 6;
        }
    }
    else {
        // set bit16 of dw3 of each action entry
        unsigned long tcp_mask =  (1 << 16);

        // setup for Hash entry
        idx = 3;
        while (idx < sizeof(lan_uc_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
            lan_uc_act_tbl_normal_mode_cfg[idx] |= tcp_mask;
            lan_uc_act_tbl_mixed_mode_cfg[idx] |= tcp_mask;

            wan_uc_act_tbl_alo_0_cfg[idx] |= tcp_mask;
            wan_uc_act_tbl_alo_1_cfg[idx] |= tcp_mask;
            idx += 6;
        }

        // setup for Collsion entry
        idx = 3;
        while (idx < sizeof(lan_uc_col_act_tbl_normal_mode_cfg)/sizeof(unsigned long)) {
            lan_uc_col_act_tbl_normal_mode_cfg[idx] |= tcp_mask;
            lan_uc_col_act_tbl_mixed_mode_cfg[idx] |= tcp_mask;
            wan_uc_col_act_tbl_cfg[idx] |= tcp_mask;
            idx += 6;
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

    // lan uc_cmp_tbl_cfg (Hash) and collision
    memcpy((void *)ROUT_LAN_HASH_CMP_TBL(0), lan_uc_cmp_tbl_cfg, sizeof(lan_uc_cmp_tbl_cfg));
    memcpy((void *)ROUT_LAN_COLL_CMP_TBL(0), lan_uc_col_cmp_tbl_cfg, sizeof(lan_uc_col_cmp_tbl_cfg));

    // lan action
    if(ppe_fw_mode == 0) {
        // normal mode
        memcpy((void *)ROUT_LAN_HASH_ACT_TBL(0), lan_uc_act_tbl_normal_mode_cfg, sizeof(lan_uc_act_tbl_normal_mode_cfg));
        memcpy((void *)ROUT_LAN_COLL_ACT_TBL(0), lan_uc_col_act_tbl_normal_mode_cfg, sizeof(lan_uc_col_act_tbl_normal_mode_cfg));
    }
    else {
        // mix mode
        memcpy((void *)ROUT_LAN_HASH_ACT_TBL(0), lan_uc_act_tbl_mixed_mode_cfg, sizeof(lan_uc_act_tbl_mixed_mode_cfg));
        memcpy((void *)ROUT_LAN_COLL_ACT_TBL(0), lan_uc_col_act_tbl_mixed_mode_cfg, sizeof(lan_uc_col_act_tbl_mixed_mode_cfg));
    }

    // wan hash cmp anc act table
    if ( ppe_wan_hash_algo == 0) {
        // WAN algo 0
        memcpy((void *)ROUT_WAN_HASH_CMP_TBL(0), wan_uc_cmp_tbl_alo_0_cfg, sizeof(wan_uc_cmp_tbl_alo_0_cfg));
        memcpy((void *)ROUT_WAN_HASH_ACT_TBL(0), wan_uc_act_tbl_alo_0_cfg, sizeof(wan_uc_act_tbl_alo_0_cfg));
    }
    else {
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
}

#endif  //  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST

static INLINE void init_qos_queue(void)
{
    struct wtx_qos_q_desc_cfg wtx_qos_q_desc_cfg = {0};
    int i;
    int des_num = WAN_TX_DESC_NUM_TOTAL;
    int q_len = WAN_TX_DESC_NUM_TOTAL;
    int equal_div = 0;
    
    for(i = 0; i < __WAN_TX_QUEUE_NUM; i ++){
        if(q_len < qos_queue_len[i] || (q_len - qos_queue_len[i]) < (__WAN_TX_QUEUE_NUM - i - 1)){
            q_len = -1;
            err("total qos queue descriptor number is large than max number(%d)", des_num);
            err("q_len: %d, queue len: %d, qid: %d", q_len, qos_queue_len[i], i);
            break;
        }
        q_len -= qos_queue_len[i];
    }
    
    if(q_len == -1){
        for ( i = 0; i < __WAN_TX_QUEUE_NUM; i++ )
        {
            wtx_qos_q_desc_cfg.length = WAN_TX_DESC_NUM(i);
            wtx_qos_q_desc_cfg.addr   = __WAN_TX_DESC_BASE(i);
            *WTX_QOS_Q_DESC_CFG(i) = wtx_qos_q_desc_cfg;
        }
    }else{      
        q_len = 0;
        for ( i = 0; i < __WAN_TX_QUEUE_NUM; i ++){
            if(!equal_div && qos_queue_len[i] == 0){
                equal_div = (des_num - q_len)/(__WAN_TX_QUEUE_NUM - i) < 256 ? (des_num - q_len)/(__WAN_TX_QUEUE_NUM - i) : 255; 
            }

            if(!equal_div){
                wtx_qos_q_desc_cfg.length = qos_queue_len[i] < 256 ? qos_queue_len[i] : 255;
            }else{
                wtx_qos_q_desc_cfg.length = equal_div;
            }
            wtx_qos_q_desc_cfg.addr   = WAN_QOS_DESC_BASE + ((q_len << 1));
            q_len += wtx_qos_q_desc_cfg.length;
            *WTX_QOS_Q_DESC_CFG(i) = wtx_qos_q_desc_cfg;
        }
    }

    return;
}

static INLINE void init_communication_data_structures(int fwcode)
{
    struct ptm_crc_cfg ptm_crc_cfg                      = {0};
    struct mixed_wan_vlan_cfg mixed_wan_vlan_cfg        = {0};
    struct rout_tbl_cfg lan_rout_tbl_cfg                = {0};
    struct rout_tbl_cfg wan_rout_tbl_cfg                = {0};
    struct gen_mode_cfg1 gen_mode_cfg1                  = {0};
    struct wrx_port_cfg_status wrx_port_config          = {0};
    struct wrx_dma_channel_cfg wrx_dma_channel_config   = {0};
    struct wtx_port_cfg wtx_port_config                 = {0};
    struct wtx_dma_channel_cfg wtx_dma_channel_config   = {0};
    struct ps_mc_cfg ps_mc_gencfg3                      = {0};
    int i, j;

    *CDM_CFG = CDM_CFG_RAM1_SET(0x00) | CDM_CFG_RAM0_SET(0x00);

    for ( i = 0; i < 1000; i++ );

    *PSEUDO_IPv4_BASE_ADDR = 0xFFFFFF00;

    *CFG_WRDES_DELAY    = write_descriptor_delay;
    *WRX_EMACH_ON       = 0x03;
    *WTX_EMACH_ON       = 0x7FFF;
    *WRX_HUNT_BITTH     = DEFAULT_RX_HUNT_BITTH;

    ptm_crc_cfg.wtx_fcs_crc_gen = 1;
    ptm_crc_cfg.wtx_tc_crc_gen  = 1;
    ptm_crc_cfg.wtx_tc_crc_len  = 16;
    ptm_crc_cfg.wrx_fcs_crc_vld = 1;
    ptm_crc_cfg.wrx_fcs_crc_chk = 1;
    ptm_crc_cfg.wrx_tc_crc_chk  = 1;
    ptm_crc_cfg.wrx_tc_crc_len  = 16;
    *PTM_CRC_CFG = ptm_crc_cfg;

    mixed_wan_vlan_cfg.wan_vlanid_hi = 0;
    mixed_wan_vlan_cfg.wan_vlanid_lo = 0;
    *MIXED_WAN_VLAN_CFG = mixed_wan_vlan_cfg;

    lan_rout_tbl_cfg.rout_num   = MAX_COLLISION_ROUTING_ENTRIES;
    lan_rout_tbl_cfg.ttl_en     = 1;
    lan_rout_tbl_cfg.rout_drop  = 0;
    *LAN_ROUT_TBL_CFG = lan_rout_tbl_cfg;

    wan_rout_tbl_cfg.rout_num           = MAX_COLLISION_ROUTING_ENTRIES;
    wan_rout_tbl_cfg.wan_rout_mc_num    = MAX_WAN_MC_ENTRIES;
    wan_rout_tbl_cfg.ttl_en             = 1;
    wan_rout_tbl_cfg.wan_rout_mc_drop   = 0;
    wan_rout_tbl_cfg.rout_drop          = 0;
    *WAN_ROUT_TBL_CFG = wan_rout_tbl_cfg;

    gen_mode_cfg1.app2_indirect         = 1;    //  0: means DMA RX ch 3 is dedicated for CPU1 process
    gen_mode_cfg1.us_indirect           = 0;
    gen_mode_cfg1.us_early_discard_en   = 1;
#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
    gen_mode_cfg1.classification_num    = MAX_CLASSIFICATION_ENTRIES;
#else
    gen_mode_cfg1.classification_num    = 0;
#endif
    gen_mode_cfg1.ipv6_rout_num         = g_ipv6_acc_en ? 2 : 2;
    gen_mode_cfg1.session_ctl_en        = 0;
    gen_mode_cfg1.wan_hash_alg          = 0;
#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
    gen_mode_cfg1.brg_class_en          = 1;
#else
    gen_mode_cfg1.brg_class_en          = 0;
#endif
    gen_mode_cfg1.ipv4_mc_acc_mode      = 0;
    gen_mode_cfg1.ipv6_acc_en           = g_ipv6_acc_en;
    gen_mode_cfg1.wan_acc_en            = 1;
    gen_mode_cfg1.lan_acc_en            = 1;
    gen_mode_cfg1.sw_iso_mode           = 1;
    gen_mode_cfg1.sys_cfg               = g_eth_wan_mode;   //  3 - ETH1 WAN, 2 - ETH0 mixed, 0 - DSL WAN
    *GEN_MODE_CFG1 = gen_mode_cfg1;
    GEN_MODE_CFG2->itf_outer_vlan_vld  |= 0xFF; //  Turn on outer VLAN for all interfaces;

    ps_mc_gencfg3.class_en = 0;
    ps_mc_gencfg3.ssc_mode = 1;
    ps_mc_gencfg3.asc_mode = 1;
    ps_mc_gencfg3.psave_en = 0; //default disable clock gating power save function
    *PS_MC_GENCFG3         = ps_mc_gencfg3;

    *__DPLUS_QID_CONF_PTR  = 2; //Qid configured by FW 

    *CFG_CLASS2QID_MAP(0) = 0;
    *CFG_CLASS2QID_MAP(1) = 0;
    *CFG_QID2CLASS_MAP(0) = 0;
    *CFG_QID2CLASS_MAP(1) = 0;
    if ( (g_wan_itf & 0x80) )
    {
        for ( j = 0; j < NUM_ENTITY(g_ptm_prio_queue_map[0]); j++ )
        {
            *CFG_CLASS2QID_MAP(j >> 3) |= g_ptm_prio_queue_map[0][j] << ((j & 0x07) * 4);
            *CFG_CLASS2QID_MAP((j + 8) >> 3) |= (g_ptm_prio_queue_map[0][j] | 8) << ((j & 0x07) * 4);
        }
    }
    else
    {
        i = (g_wan_itf & 0x01) ? 0 : 1;
        for ( j = 0; j < NUM_ENTITY(g_eth_prio_queue_map[i]); j++ )
        {
            *CFG_CLASS2QID_MAP(j >> 3) |= g_eth_prio_queue_map[i][j] << ((j & 0x07) * 4);
            *CFG_CLASS2QID_MAP((j + 8) >> 3) |= (g_eth_prio_queue_map[i][j] | 8) << ((j & 0x07) * 4);
        }
    }
    for ( j = 0; j < NUM_ENTITY(g_wan_queue_class_map); j++ )
        *CFG_QID2CLASS_MAP(j >> 3) |= g_wan_queue_class_map[j] << ((j & 0x07) * 4);

#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
    wrx_port_config.mfs		   = 1604 + 4 * 2;
#else
    wrx_port_config.mfs		   = 1518 + 4 * 2;
#endif

    wrx_port_config.dmach         = 0;
    wrx_port_config.local_state   = 0;
    wrx_port_config.partner_state = 0;
    for ( i = 0; i < 2; i++ )
    {
        *WRX_PORT_CONFIG(i) = wrx_port_config;

        wrx_dma_channel_config.desba  = ((unsigned int)WAN_RX_DESC_BASE & 0x1FFFFFFF) >> 2;
        wrx_dma_channel_config.deslen = WAN_RX_DESC_NUM;
        *WRX_DMA_CHANNEL_CONFIG(i) = wrx_dma_channel_config;
    }

    wtx_port_config.tx_cwth2 = 4;
    wtx_port_config.tx_cwth1 = 5;
    for ( i = 0; i < 2; i++ )
    {
        wtx_port_config.queue_map = i == 0 ? 0xFF : 0;
        *WTX_PORT_CONFIG(i) = wtx_port_config;

        wtx_dma_channel_config.desba  = WAN_BCi_TX_DESC_PPE(i);
        wtx_dma_channel_config.deslen = WAN_BCi_TX_DESC_NUM(i);
        *WTX_DMA_CHANNEL_CONFIG(i) = wtx_dma_channel_config;
    }

    for ( i = 0; i < 8; i++ )
        *MTU_CFG_TBL(i) = ETH_MAX_DATA_LENGTH;  //  for ETH

    if ( g_wanqos_en )
    {
        struct wtx_qos_q_desc_cfg wtx_qos_q_desc_cfg = {0};
        struct tx_qos_cfg tx_qos_cfg = {0};
        struct cfg_std_data_len cfg_std_data_len = {0};

        init_qos_queue();
		
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
        tx_qos_cfg.time_tick    = cgu_get_pp32_clock() / 62500; //  16 * (cgu_get_pp32_clock() / 1000000)
#else
   {
    struct clk * clk;
    clk = clk_get_ppe();
    //ps_mc_gencfg3.time_tick = clk_get_rate (clk) / 125000;    //  8 * (cgu_get_pp32_clock() / 1000000), 8 microsecond, half of QoS time tick
    tx_qos_cfg.time_tick    = clk_get_rate (clk) / 62500;
   }
#endif
        tx_qos_cfg.eth1_eg_qnum = __WAN_TX_QUEUE_NUM;
        tx_qos_cfg.eth1_qss     = 1;
        tx_qos_cfg.eth1_burst_chk = 1;
        tx_qos_cfg.overhd_bytes = OVERSIZE_HEADER_LEN;
        *TX_QOS_CFG = tx_qos_cfg;

        cfg_std_data_len.byte_off = 0;
        cfg_std_data_len.data_len = DMA_PACKET_SIZE;

        *CFG_STD_DATA_LEN = cfg_std_data_len;
		 *__QOS_NEW_FEATURE = 0; /* don't enable MBR by default */
        /* by default, all queues(q0-q7) map to default shaper */
        for( i = 0; i < __WAN_TX_QUEUE_NUM; i ++){
            WTX_EG_Q_SHAPER_MAPPING_CFG(i)->shaper_base_addr = WTX_EG_Q_DEF_SHAPER_ADDR;
            /* default shaper bitmap map to all queues */
            *WTX_EG_Q_DEF_SHAPER_QUEUE_MAP |= 1 << i;
        }
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
    int i, j;

#if defined(ENABLE_P2P_COPY_WITHOUT_DESC) && ENABLE_P2P_COPY_WITHOUT_DESC
    pre_alloc_desc_total_num = DMA_RX_CH1_DESC_NUM;
#else
    pre_alloc_desc_total_num = DMA_RX_CH1_DESC_NUM + DMA_RX_CH2_DESC_NUM;
#endif

    if ( g_eth_wan_mode == 0 )
        pre_alloc_desc_total_num += CPU_TO_WAN_TX_DESC_NUM + WAN_TX_DESC_NUM_TOTAL + CPU_TO_WAN_SWAP_DESC_NUM + WAN_RX_DESC_NUM + WAN_BCi_TX_DESC_NUM(0) * 2;
    else if ( g_wanqos_en )
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
    g_dma_device->intr_handler    = dma_int_handler;
    g_dma_device->tx_burst_len    = 8;
    g_dma_device->rx_burst_len    = 8;

    for ( i = 0; i < g_dma_device->max_rx_chan_num; i++ )
    {
        g_dma_device->rx_chan[i]->packet_size = DMA_PACKET_SIZE;
        g_dma_device->rx_chan[i]->control     = IFX_DMA_CH_ON;
    }
    g_dma_device->rx_chan[1]->desc_base = (int)DMA_RX_CH1_DESC_BASE;
    g_dma_device->rx_chan[1]->desc_len  = DMA_RX_CH1_DESC_NUM;
    g_dma_device->rx_chan[2]->desc_base = (int)DMA_RX_CH2_DESC_BASE;
    g_dma_device->rx_chan[2]->desc_len  = DMA_RX_CH2_DESC_NUM;
    g_dma_device->rx_chan[1]->req_irq_to_free   = g_dma_device->rx_chan[1]->irq;
    g_dma_device->rx_chan[2]->req_irq_to_free   = g_dma_device->rx_chan[2]->irq;
 #if defined(ENABLE_P2P_COPY_WITHOUT_DESC) && ENABLE_P2P_COPY_WITHOUT_DESC
    if ( g_eth_wan_mode != 0 /* ETH WAN */ && !g_wanqos_en /* QoS Disabled */ )
    {
        g_dma_device->rx_chan[1]->loopback_enable           = 1;
        g_dma_device->rx_chan[1]->loopback_channel_number   = 1 * 2 + 1;
    }
    g_dma_device->rx_chan[2]->loopback_enable           = 1;
    g_dma_device->rx_chan[2]->loopback_channel_number   = 0 * 2 + 1;
 #endif

    for ( i = 0; i < g_dma_device->max_tx_chan_num; i++ )
    {
        g_dma_device->tx_chan[i]->control     = IFX_DMA_CH_ON;
    }
    g_dma_device->tx_chan[0]->desc_base = (int)DMA_TX_CH0_DESC_BASE;
    g_dma_device->tx_chan[0]->desc_len  = DMA_TX_CH0_DESC_NUM;
    g_dma_device->tx_chan[1]->desc_base = (int)DMA_TX_CH1_DESC_BASE;
    g_dma_device->tx_chan[1]->desc_len  = DMA_TX_CH1_DESC_NUM;
 #if !defined(ENABLE_P2P_COPY_WITHOUT_DESC) || !ENABLE_P2P_COPY_WITHOUT_DESC
    g_dma_device->tx_chan[0]->global_buffer_len = DMA_PACKET_SIZE;
    g_dma_device->tx_chan[0]->peri_to_peri      = 1;
    g_dma_device->tx_chan[1]->global_buffer_len = DMA_PACKET_SIZE;
    g_dma_device->tx_chan[1]->peri_to_peri      = 1;
 #else
    g_dma_device->tx_chan[0]->loopback_enable   = 1;
    if ( g_eth_wan_mode == 0 /* DSL WAN */ || g_wanqos_en /* QoS Enabled */ )
    {
        g_dma_device->tx_chan[1]->global_buffer_len = DMA_PACKET_SIZE;
        g_dma_device->tx_chan[1]->peri_to_peri      = 1;
    }
    else
        g_dma_device->tx_chan[1]->loopback_enable   = 1;
 #endif

    g_dma_device->port_packet_drop_enable = 1;

    if ( dma_device_register(g_dma_device) != PPA_SUCCESS )
    {
        err("failed in \"dma_device_register\"");
        ret = -EIO;
        goto DMA_DEVICE_REGISTER_FAIL;
    }

    pskb = skb_pool;

    if ( g_eth_wan_mode == 0 || g_wanqos_en )
    {
        p = (volatile u32 *)CPU_TO_WAN_TX_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_TX_DESC_NUM; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "CPU to WAN TX data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = g_wanqos_en ? 0x30000000 : 0xB0000000;
            if ( g_eth_wan_mode == 0 || g_wanqos_en )
                *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
            else
                *p++ = 0;
        }

        p = (volatile u32 *)WAN_TX_DESC_BASE(0);
        for ( i = 0; i < WAN_TX_DESC_NUM_TOTAL; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "WAN (ATM/ETH) TX data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = 0x30000000;
            *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
        }

        p = (volatile u32 *)CPU_TO_WAN_SWAP_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_SWAP_DESC_NUM; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "WAN (PTM) Swap data pointer (0x%08x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = 0xB0000000;
            *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
        }
    }

    if ( g_eth_wan_mode == 0 )
    {
        p = (volatile u32 *)WAN_RX_DESC_BASE;
        for ( i = 0; i < WAN_RX_DESC_NUM; i++ )
        {
            ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "WAN (ATM) RX channel 0 (AAL) data pointer (0x%#x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
            *p++ = 0xB1000000 | DMA_PACKET_SIZE;        //  byteoff 2
            *(unsigned int *)((*pskb)->data) = 0x0700;  //  SPID    7
#ifndef CONFIG_MIPS_UNCACHED
            dma_cache_wback((unsigned int)(*pskb)->data, sizeof(unsigned int));
#endif
            *p++ = ((u32)(*pskb++)->data & 0x1FFFFFE0);
        }

        for ( j = 0; j < 2; j++ )
        {
            p = (volatile u32 *)WAN_BCi_TX_DESC_BASE(j);
            for ( i = 0; i < WAN_BCi_TX_DESC_NUM(j); i++ )
            {
                *p++ = 0x30000000;
                *p++ = ((u32)(*pskb++)->data & 0x1FFFFFE0) >> 2;
            }
        }
    }

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
#if !defined(ENABLE_P2P_COPY_WITHOUT_DESC) || !ENABLE_P2P_COPY_WITHOUT_DESC
        ASSERT(((u32)(*pskb)->data & (DMA_ALIGNMENT - 1)) == 0, "DMA RX channel 2 data pointer (0x%08x) must be 8 DWORDS aligned", (u32)(*pskb)->data);
#endif
        *p++ = 0xB0000000 | DMA_PACKET_SIZE;
#if !defined(ENABLE_P2P_COPY_WITHOUT_DESC) || !ENABLE_P2P_COPY_WITHOUT_DESC
        *p++ = (u32)(*pskb++)->data & 0x1FFFFFE0;
#else
        *p++ = 0x00000000;
#endif
    }

    if ( g_eth_wan_mode != 0 && g_wanqos_en )
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
    int i, j;

    if ( g_eth_wan_mode == 0 || g_wanqos_en )
    {
        p = (volatile struct tx_descriptor *)CPU_TO_WAN_TX_DESC_BASE;
        for ( i = 0; i < CPU_TO_WAN_TX_DESC_NUM; i++ )
        {
            skb_to_free = get_skb_pointer(p->dataptr);
            dev_kfree_skb_any(skb_to_free);
            p++;
        }

        p = (volatile struct tx_descriptor *)WAN_TX_DESC_BASE(0);
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

    if ( g_eth_wan_mode == 0 )
    {
        p = (volatile struct tx_descriptor *)WAN_RX_DESC_BASE;
        for ( i = 0; i < WAN_RX_DESC_NUM; i++ )
        {
            skb_to_free = get_skb_pointer(p->dataptr & ~(DMA_ALIGNMENT - 1));
            dev_kfree_skb_any(skb_to_free);
            p++;
        }

        for ( j = 0; j < 2; j++ )
        {
            p = (volatile struct tx_descriptor *)WAN_BCi_TX_DESC_BASE(j);
            for ( i = 0; i < WAN_BCi_TX_DESC_NUM(j); i++ )
            {
                skb_to_free = get_skb_pointer(p->dataptr << 2);
                dev_kfree_skb_any(skb_to_free);
                p++;
            }
        }
    }

    p = (volatile struct tx_descriptor *)DMA_RX_CH1_DESC_BASE;
    for ( i = 0; i < DMA_RX_CH1_DESC_NUM; i++ )
    {
        skb_to_free = get_skb_pointer(p->dataptr);
        dev_kfree_skb_any(skb_to_free);
        p++;
    }

#if !defined(ENABLE_P2P_COPY_WITHOUT_DESC) || !ENABLE_P2P_COPY_WITHOUT_DESC
    p = (volatile struct tx_descriptor *)DMA_RX_CH2_DESC_BASE;
    for ( i = 0; i < DMA_RX_CH2_DESC_NUM; i++ )
    {
        skb_to_free = get_skb_pointer(p->dataptr);
        dev_kfree_skb_any(skb_to_free);
        p++;
    }
#endif

    if ( g_eth_wan_mode != 0 && g_wanqos_en )
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

#if defined(CONFIG_LTQ_PPA_DATAPATH)
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

static void dsl_led_polling(unsigned long arg)
{
    if ( PTM_MIB_TABLE(0)->wrx_correct_pdu != g_dsl_wrx_correct_pdu[0] || PTM_MIB_TABLE(0)->wtx_total_pdu != g_dsl_wtx_total_pdu[0]
        || PTM_MIB_TABLE(1)->wrx_correct_pdu != g_dsl_wrx_correct_pdu[1] || PTM_MIB_TABLE(1)->wtx_total_pdu != g_dsl_wtx_total_pdu[1] )
    {
        g_dsl_wrx_correct_pdu[0] = PTM_MIB_TABLE(0)->wrx_correct_pdu;
        g_dsl_wtx_total_pdu[0] = PTM_MIB_TABLE(0)->wtx_total_pdu;
        g_dsl_wrx_correct_pdu[1] = PTM_MIB_TABLE(1)->wrx_correct_pdu;
        g_dsl_wtx_total_pdu[1] = PTM_MIB_TABLE(1)->wtx_total_pdu;

        adsl_led_flash();
    }

    if ( g_ptm_net_dev[0] != NULL && netif_running(g_ptm_net_dev[0]) )
    {
        g_dsl_led_polling_timer.expires = jiffies + HZ / 10;    //  100ms
        add_timer(&g_dsl_led_polling_timer);
    }
}

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
        "AR10",
        "VRX218",
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


static struct file_operations g_proc_file_ver_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_ver_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_ver_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ver, NULL);
}


static struct file_operations g_proc_file_mib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mib_seq_open,
    .read       = seq_read,
    .write      = proc_write_mib_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mib, NULL);
}

#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
static struct seq_operations g_proc_read_route_seq_ops = {
    .start      = proc_read_route_seq_start,
    .next       = proc_read_route_seq_next,
    .stop       = proc_read_route_seq_stop,
    .show       = proc_read_route_seq_show,
};

static struct file_operations g_proc_file_route_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_route_seq_open,
    .read       = seq_read,
    .write	= proc_write_route_seq,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static uint32_t g_proc_read_route_pos = 0;
static uint32_t g_proc_read_route_pos_prev = ~0;

static int proc_read_route_seq_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &g_proc_read_route_seq_ops);
}

static struct file_operations g_proc_file_mc_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mc_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mc_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mc, NULL);
}

static struct file_operations g_proc_file_genconf_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_genconf_seq_open,
    .read       = seq_read,
    .write      = proc_write_genconf_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_genconf_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_genconf, NULL);
}

static struct file_operations g_proc_file_pppoe_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_pppoe_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_pppoe_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_pppoe, NULL);
}

static struct file_operations g_proc_file_mtu_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mtu_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mtu_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mtu, NULL);
}

static struct file_operations g_proc_file_hit_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_hit_seq_open,
    .read       = seq_read,
    .write      = proc_write_hit_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_hit_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_hit, NULL);
}

static struct file_operations g_proc_file_mac_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mac_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mac_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mac, NULL);
}

static struct file_operations g_proc_file_out_vlan_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_out_vlan_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_out_vlan_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_out_vlan, NULL);
}

static struct file_operations g_proc_file_ipv6_ip_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_ipv6_ip_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_ipv6_ip_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ipv6_ip, NULL);
}
#endif

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC

static struct file_operations g_proc_file_dbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dbg_seq_open,
    .read       = seq_read,
    .write      = proc_write_dbg_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dbg, NULL);
}
#endif

#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
static struct file_operations g_proc_file_mem_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mem_seq_open,
    .read       = seq_read,
    .write      = proc_write_mem_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mem_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mem, NULL);
}

static int proc_read_mem(struct seq_file *seq, void *v)
{
    return 0;
}
#endif

#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
static struct file_operations g_proc_file_pp32_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_pp32_seq_open,
    .read       = seq_read,
    .write      = proc_write_pp32_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_pp32_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_pp32, NULL);
}

#endif

static struct file_operations g_proc_file_burstlen_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_burstlen_seq_open,
    .read       = seq_read,
    .write      = proc_write_burstlen_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_burstlen_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_burstlen, NULL);
}


#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
static struct file_operations g_proc_file_fw_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_fw_seq_open,
    .read       = seq_read,
    .write      = proc_write_fw_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_fw_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_fw, NULL);
}

static struct file_operations g_proc_file_fwdbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_fwdbg_seq_open,
    .read       = seq_read,
    .write      = proc_write_fwdbg_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_fwdbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_fwdbg, NULL);
}

static int proc_read_fwdbg(struct seq_file *seq, void *v)
{
    return 0;
}
#endif

static struct file_operations g_proc_file_port0_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port0_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port0_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port0_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_port1_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port1_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port1_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port1_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_port2_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port2_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port2_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port2_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_port3_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port3_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port3_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port3_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_port4_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port4_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port4_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port4_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_port5_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port5_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port5_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port5_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_port6_counters_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port6_counters_seq_open,
    .read       = seq_read,
    .write      = port_counters_proc_write_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port6_counters_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, port6_counters_proc_read, NULL);
}

static struct file_operations g_proc_file_directforwarding_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_directforwarding_seq_open,
    .read       = seq_read,
    .write      = proc_write_directforwarding_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_directforwarding_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_directforwarding, NULL);
}

static struct file_operations g_proc_file_clk_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_clk_seq_open,
    .read       = seq_read,
    .write      = proc_write_clk_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_clk_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_clk, NULL);
}

#if defined(ENABLE_NAPI) && ENABLE_NAPI
static struct file_operations g_proc_file_napi_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_napi_seq_open,
    .read       = seq_read,
    .write      = proc_write_napi_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_napi_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_napi, NULL);
}
#endif

static struct file_operations g_proc_file_prio_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_prio_seq_open,
    .read       = seq_read,
    .write      = proc_write_prio_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_prio_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_prio, NULL);
}

static struct file_operations g_proc_file_ewan_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_ewan_seq_open,
    .read       = seq_read,
    .write      = proc_write_ewan_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_ewan_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ewan, NULL);
}

#ifdef CONFIG_LTQ_PPA_QOS
static struct file_operations g_proc_file_qos_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_qos_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_qos_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_qos, NULL);
}
#endif


#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
static struct file_operations g_proc_file_mirror_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mirror_seq_open,
    .read       = seq_read,
    .write      = proc_write_mirror_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mirror_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mirror, NULL);
}
#endif

static struct file_operations g_proc_file_class_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_class_seq_open,
    .read       = seq_read,
    .write      = proc_write_class_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_class_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_class, NULL);
}

static struct file_operations g_proc_file_mccfg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mccfg_seq_open,
    .read       = seq_read,
    .write      = proc_write_mccfg_seq,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_mccfg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_mccfg, NULL);
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
    }
    res  = proc_create("ver",S_IRUGO, proc_dir, &g_proc_file_ver_seq_fops);
    res  = proc_create("mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_mib_seq_fops);
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
    res  = proc_create("route",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_route_seq_fops);
    res  = proc_create("mc",S_IRUGO, proc_dir, &g_proc_file_mc_seq_fops);
    res  = proc_create("genconf",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_genconf_seq_fops);

    res  = proc_create("pppoe",S_IRUGO, proc_dir, &g_proc_file_pppoe_seq_fops);
    res  = proc_create("mtu",S_IRUGO, proc_dir, &g_proc_file_mtu_seq_fops);
    res  = proc_create("hit",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_hit_seq_fops);
    res  = proc_create("mac",S_IRUGO, proc_dir, &g_proc_file_mac_seq_fops);
    res  = proc_create("out_vlan",S_IRUGO, proc_dir, &g_proc_file_out_vlan_seq_fops);
    if(g_ipv6_acc_en)
    {
    	res  = proc_create("ipv6_ip",S_IRUGO, proc_dir, &g_proc_file_ipv6_ip_seq_fops);
    }
#endif
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
    res  = proc_create("dbg",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_dbg_seq_fops);
#endif
#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
    res  = proc_create("mem",S_IWUSR, proc_dir, &g_proc_file_mem_seq_fops);
#endif
#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
    res  = proc_create("pp32",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_pp32_seq_fops);
#endif
    res  = proc_create("burstlen",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_burstlen_seq_fops);
#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
    res  = proc_create("fw",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_fw_seq_fops);
    res  = proc_create("tfwdbg",S_IWUSR, proc_dir, &g_proc_file_fwdbg_seq_fops);
#endif
    res  = proc_create("port0mib",S_IRUGO|S_IWUSR, proc_dir,&g_proc_file_port0_counters_seq_fops );
    res  = proc_create("port1mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_port1_counters_seq_fops);
    res  = proc_create("port2mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_port2_counters_seq_fops);
    res  = proc_create("port3mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_port3_counters_seq_fops);
    res  = proc_create("port4mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_port4_counters_seq_fops);
    res  = proc_create("port5mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_port5_counters_seq_fops);
    res  = proc_create("port6mib",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_port6_counters_seq_fops);
    res  = proc_create("direct_forwarding",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_directforwarding_seq_fops);
    res  = proc_create("clk",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_clk_seq_fops);
#if defined(ENABLE_NAPI) && ENABLE_NAPI
    res  = proc_create("napi",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_napi_seq_fops);
#endif
    res  = proc_create("prio",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_prio_seq_fops);
    res  = proc_create("ewan",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_ewan_seq_fops);
#ifdef CONFIG_LTQ_PPA_QOS
    if(g_wanqos_en && !g_eth_wan_mode)
    {
    	res  = proc_create("qos",S_IRUGO, proc_dir, &g_proc_file_qos_seq_fops);
    }
#endif
#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
    res  = proc_create("mirror",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_mirror_seq_fops);
#endif
    res  = proc_create("class",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_class_seq_fops);
    res  = proc_create("mccfg",S_IRUGO|S_IWUSR, proc_dir, &g_proc_file_mccfg_seq_fops);

}

static INLINE void proc_file_create(void)
{
    /*if no parent folder name provided, register function will take the nearest proc_dir_entry by default.
          if no proc_dir_entry found, by default, it's NULL. (e.g. create folder eth )
          Please make sure the file follow the right dir entry otherwise it would cause trouble to release it
      */

     static struct proc_entry_cfg ppa_proc_entry[] = {
     // Parent dir,    Name,    IS_DIR,   RD_fn,     WR_fn,     Enable
        {NULL,  "eth",      		1, 		NULL,   NULL,1 },
        {NULL,  "ver",      		0, 		NULL,   NULL,1 },
        {NULL,  "mib",      		0, 		NULL,   NULL,1 },
#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC
        {NULL,  "route",    		0, 		NULL,   NULL,1 },
        {NULL,  "mc",       		0, 		NULL,   NULL,1 },
        {NULL,  "genconf",  		0, 		NULL,   NULL,1 },
        {NULL,  "pppoe",    		0, 		NULL,   NULL,1 },
        {NULL,  "mtu",      		0, 		NULL,   NULL,1 },
        {NULL,  "hit",      		0, 		NULL,   NULL,1 },
        {NULL,  "mac",      		0, 		NULL,   NULL,1 },
        {NULL,  "out_vlan", 		0, 		NULL,   NULL,1 },
        {NULL,  "ipv6_ip",  		0, 		NULL,   NULL,1 },
#endif
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
        {NULL,  "dbg",      		0, 		NULL,   NULL,1 },
#endif
#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
        {NULL,  "mem",      		0, 		NULL,   NULL,1 },
#endif
#if defined(DEBUG_PP32_PROC) && DEBUG_PP32_PROC
        {NULL,  "pp32",     		0, 		NULL,   NULL,1 },
#endif
        {NULL,  "burstlen", 		0, 		NULL,   NULL,1 },
#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
        {NULL,  "fw",       		0, 		NULL,   NULL,1 },
        {NULL,  "tfwdbg",   		0, 		NULL,   NULL,1 },
#endif
        {NULL,  "port0mib", 		0, 		NULL,   NULL,1 },
        {NULL,  "port1mib", 		0, 		NULL,   NULL,1 },
        {NULL,  "port2mib", 		0, 		NULL,   NULL,1 },
        {NULL,  "port3mib", 		0, 		NULL,   NULL,1 },
        {NULL,  "port4mib", 		0, 		NULL,   NULL,1 },
        {NULL,  "port5mib", 		0, 		NULL,   NULL,1 },
        {NULL,  "port6mib", 		0, 		NULL,   NULL,1 },
        {NULL,"direct_forwarding", 	0,  		NULL,  	NULL,1 },
        {NULL,  "clk",      		0, 		NULL,   NULL,1 },
#if defined(ENABLE_NAPI) && ENABLE_NAPI	
        {NULL,  "napi",      		0, 		NULL,   NULL,1 },
#endif
        {NULL,  "prio",     		0, 		NULL,   NULL,1 },
        {NULL,  "ewan",     		0, 		NULL,   NULL,1 },
#ifdef CONFIG_LTQ_PPA_QOS
        {NULL,  "qos",      		0, 		NULL,   NULL,1 },
#endif
#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
        {NULL,  "mirror",   		0, 		NULL,   NULL,1 },
#endif		
        {NULL,  "class",    		0, 		NULL,   NULL,1 },
        {NULL,  "mccfg",    		0, 		NULL,   NULL,1 },
        {NULL,  NULL,       		0, 		NULL,   NULL,1 },

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
	if( ! g_eth_proc_entry[i].is_enable ) continue;
        if(g_eth_proc_entry[i].is_dir){
	    proc_remove(g_eth_proc_entry[i].proc_dir);
        }
    }
}


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

#if defined(DEBUG_FW_PROC) && DEBUG_FW_PROC
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
    dump_flag_header(g_fwcode, pfhdr, __FUNCTION__, -1);

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

#endif


#define RX_TOTAL_PCKNT                  0x1F
#define RX_UNICAST_PCKNT                0x23
#define RX_MULTICAST_PCKNT              0x22
#define RX_CRC_ERR_PCKNT                0x21
#define RX_UNDERSIZE_GOOD_PCKNT         0x1D
#define RX_OVER_SIZE_GOOD_PCKNT         0x1B
#define RX_UNDERSIZE_ERR_PCKNT          0x1E
#define RX_GOOD_PAUSE_PCKNT             0x20
#define RX_OVER_SIZE_ERR_PCKNT          0x1C
#define RX_ALLIGN_ERR_PCKNT             0x1A
#define RX_FILTERED_PCKNT               0x19
#define RX_DISCARD_CONGESTION_PCKNT     0x11

#define RX_SIZE_64B_PCKNT               0x12
#define RX_SIZE_65_127B_PCKNT           0x13
#define RX_SIZE_128_255B_PCKNT          0x14
#define RX_SIZE_256_511B_PCKNT          0x15
#define RX_SIZE_512_1023B_PCKNT         0x16
#define RX_SIZE_1024B_MAX_PCKNT         0x17

#define TX_TOTAL_PCKNT                  0x0C
#define TX_UNICAST_PCKNT                0x06
#define TX_MULTICAST_PCKNT              0x07

#define TX_SINGLE_COLLISION_CNT         0x08
#define TX_MULTIPLE_COLLISION_CNT       0x09
#define TX_LATE_COLLISION_CNT           0x0A
#define TX_EXCESSIVE_COLLISION_CNT      0x0B
#define TX_PAUSE_PCKNT                  0x0D

#define TX_SIZE_64B_PCKNT               0x00
#define TX_SIZE_65_127B_PCKNT           0x01
#define TX_SIZE_128_255B_PCKNT          0x02
#define TX_SIZE_256_511B_PCKNT          0x03
#define TX_SIZE_512_1023B_PCKNT         0x04
#define TX_SIZE_1024B_MAX_PCKNT         0x05


#define TX_DROP_PCKNT                   0x10
#define RX_DROP_PCKNT                   0x18

#define RX_GOOD_BYTE_CNT_LOW            0x24
#define RX_GOOD_BYTE_CNT_HIGH           0x25

#define RX_BAD_BYTE_CNT_LOW             0x26
#define RX_BAD_BYTE_CNT_HIGH            0x27

#define TX_GOOD_BYTE_CNT_LOW            0x0E
#define TX_GOOD_BYTE_CNT_HIGH           0x0F

static INLINE unsigned int sw_get_rmon_counter(int port, int addr)
{
    *ETHSW_BM_RAM_ADDR_REG = addr;
    *ETHSW_BM_RAM_CTRL_REG  = 0x8000 | port;
    while ( (*ETHSW_BM_RAM_CTRL_REG & 0x8000) );

    return (*ETHSW_BM_RAM_VAL_1_REG << 16) | (*ETHSW_BM_RAM_VAL_0_REG & 0xFFFF);
}

static INLINE void sw_clr_rmon_counter(int port)
{
    int i;

    if ( port >= 0 && port < 7 )
    {
        *ETHSW_BM_PCFG_REG(port) = 0; /*BM_PCFG RMON COUNTER Disable */
        *ETHSW_BM_RMON_CTRL_REG(port) = 3; /*BM_RMON_CTRL Software Reset */
        for ( i = 1000; (*ETHSW_BM_RMON_CTRL_REG(port) & 3) != 0 && i > 0; i-- );
        if ( i == 0 )
            *ETHSW_BM_RMON_CTRL_REG(port) = 0;
        *ETHSW_BM_PCFG_REG(port) = 1;
    }
}

static int proc_read_ver(struct seq_file *seq, void *v)
{
    int len = 0;

    print_driver_ver_seq(seq, "PPE datapath driver info", VER_FAMILY, VER_DRTYPE, VER_INTERFACE, VER_ACCMODE, VER_MAJOR, VER_MID, VER_MINOR);
    print_fw_ver_seq(seq);


    return len;
}


static int print_fw_ver_seq(struct seq_file *seq)
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
        "AR10",
        "VRX218",
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

    seq_printf(seq, "PPE firmware info:\n");
    seq_printf(seq,     "  Version ID: %d.%d.%d.%d.%d\n", (int)FW_VER_ID->family, (int)FW_VER_ID->package, (int)FW_VER_ID->major, (int)FW_VER_ID->middle, (int)FW_VER_ID->minor);
    if ( FW_VER_ID->family >= NUM_ENTITY(fw_ver_family_str) )
        seq_printf(seq, "  Family    : reserved - %d\n", (int)FW_VER_ID->family);
    else
        seq_printf(seq, "  Family    : %s\n", fw_ver_family_str[FW_VER_ID->family]);

    if ( FW_VER_ID->package >= NUM_ENTITY(fw_ver_package_str) )
        seq_printf(seq, "  FW Package: reserved - %d\n", (int)FW_VER_ID->package);
    else
        seq_printf(seq, "  FW Package: %s\n", fw_ver_package_str[FW_VER_ID->package]);

    seq_printf(seq,     "  Release   : %u.%u.%u\n", (int)FW_VER_ID->major, (int)FW_VER_ID->middle, (int)FW_VER_ID->minor);

    seq_printf(seq, "PPE firmware feature:\n");

    for(i = 0; i < NUM_ENTITY(fw_feature); i ++){
        if(*FW_VER_FEATURE & (1 << (31-i))){
            seq_printf(seq, "%s\n", fw_feature[i]);
        }
    }

    return len;
}

static int print_driver_ver_seq(struct seq_file *seq, char *title, unsigned int family, unsigned int type, unsigned int itf, unsigned int mode, unsigned int major, unsigned int mid, unsigned int minor)
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

    seq_printf(seq, "%s:\n", title);
    seq_printf(seq, "  Version ID: %d.%d.%d.%d.%d.%d.%d\n", family, type, itf, mode, major, mid, minor);
    seq_printf(seq, "  Family    : ");
    for ( bit = family, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_family_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_family_str[i] != NULL )
        {
            if ( j )
                seq_printf(seq, " | %s", dr_ver_family_str[i]);
            else
                seq_printf(seq, dr_ver_family_str[i]);
            j++;
        }
    if ( j )
        seq_printf(seq, "\n");
    else
        seq_printf(seq, "N/A\n");
    seq_printf(seq, "  DR Type   : ");
    for ( bit = type, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_type_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_type_str[i] != NULL )
        {
            if ( j )
                seq_printf(seq, " | %s", dr_ver_type_str[i]);
            else
                seq_printf(seq, dr_ver_type_str[i]);
            j++;
        }
    if ( j )
        seq_printf(seq, "\n");
    else
        seq_printf(seq, "N/A\n");
    seq_printf(seq, "  Interface : ");
    for ( bit = itf, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_interface_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_interface_str[i] != NULL )
        {
            if ( j )
                seq_printf(seq, " | %s", dr_ver_interface_str[i]);
            else
                seq_printf(seq, dr_ver_interface_str[i]);
            j++;
        }
    if ( j )
        seq_printf(seq, "\n");
    else
        seq_printf(seq, "N/A\n");
    seq_printf(seq, "  Mode      : ");
    for ( bit = mode, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_accmode_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_accmode_str[i] != NULL )
        {
            if ( j )
                seq_printf(seq, " | %s", dr_ver_accmode_str[i]);
            else
                seq_printf(seq, dr_ver_accmode_str[i]);
            j++;
        }
    if ( j )
        seq_printf(seq, "\n");
    else
        seq_printf(seq, "N/A\n");
    seq_printf(seq, "  Release   : %d.%d.%d\n", major, mid, minor);

    return len;
}


static int proc_read_mib(struct seq_file *seq, void *v)
{
    struct eth_priv_data *priv0, *priv1, *priv7 = NULL;
    char *itf_name[] = {"eth0", "eth1", "cpu", "ext_int1", "ext_int2", "ext_int3", "ext_int4", "ptm0"};
    char *row_name[] = {"ig_fast_brg_pkts", "ig_fast_brg_bytes", "ig_fast_rt_ipv4_udp_pkts",
                        "ig_fast_rt_ipv4_tcp_pkts", "ig_fast_rt_ipv4_mc_pkts", "ig_fast_rt_ipv4_bytes",
                        "ig_fast_rt_ipv6_udp_pkts", "ig_fast_rt_ipv6_tcp_pkts", "ig_fast_rt_ipv6_mc_pkts",
                        "ig_fast_rt_ipv6_bytes", NULL, "ig_cpu_pkts", "ig_cpu_bytes", "ig_drop_pkts",
                        "ig_drop_bytes", "eg_fast_pkts"};
    int i, j, k, h;

    eth_get_stats(g_eth_net_dev[0]);
    eth_get_stats(g_eth_net_dev[1]);
    eth_get_stats(g_ptm_net_dev[0]);
    priv0 = netdev_priv(g_eth_net_dev[0]);
    priv1 = netdev_priv(g_eth_net_dev[1]);
    priv7 = netdev_priv(g_ptm_net_dev[0]);


    seq_printf(seq, "Firmware  (");
    for ( i = h = 0; i < NUM_ENTITY(itf_name); i++ )
        if ( (g_mib_itf & (1 << i)) )
            seq_printf(seq, h++ == 0 ? "%s" : ", %s", itf_name[i]);
    seq_printf(seq, ")\n");
    for ( i = k = 0; i < NUM_ENTITY(row_name); i++ )
        if ( row_name[i] != NULL && k < (j = strlen(row_name[i])) )
            k = j;
    k += 6;
    for ( i = 0; i < NUM_ENTITY(row_name); i++ )
    {
        if ( row_name[i] == NULL )
            continue;

        seq_printf(seq, "    %s: ", row_name[i]);
	j = strlen(row_name[i]);
        for ( j = k - j; j > 0; j-- )
            seq_printf(seq, " ");
        for ( j = h = 0; j < 8; j++ )
            if ( (g_mib_itf & (1 << j)) )
                seq_printf(seq, h++ == 0 ? "%10u" : ", %10u", ((volatile unsigned int *)ITF_MIB_TBL(j))[i]);
        seq_printf(seq, "\n");
    }

    seq_printf(seq, "Driver (ETH0, ETH1, PTM)\n");
    seq_printf(seq, "  rx_packets: %10u, %10u, %10u\n", priv0->rx_packets,  priv1->rx_packets,  priv7 ? priv7->rx_packets : 0);
    seq_printf(seq, "  rx_bytes:   %10u, %10u, %10u\n", priv0->rx_bytes,    priv1->rx_bytes,    priv7 ? priv7->rx_bytes   : 0);
    seq_printf(seq, "  rx_errors:  %10u, %10u, %10u\n", priv0->rx_errors,   priv1->rx_errors,   priv7 ? priv7->rx_errors  : 0);
    seq_printf(seq, "  rx_dropped: %10u, %10u, %10u\n", priv0->rx_dropped,  priv1->rx_dropped,  priv7 ? priv7->rx_dropped : 0);
    seq_printf(seq, "  tx_packets: %10u, %10u, %10u\n", priv0->tx_packets,  priv1->tx_packets,  priv7 ? priv7->tx_packets : 0);
    seq_printf(seq, "  tx_bytes:   %10u, %10u, %10u\n", priv0->tx_bytes,    priv1->tx_bytes,    priv7 ? priv7->tx_bytes   : 0);
    seq_printf(seq, "  tx_errors:  %10u, %10u, %10u\n", priv0->tx_errors,   priv1->tx_errors,   priv7 ? priv7->tx_errors  : 0);
    seq_printf(seq, "  tx_dropped: %10u, %10u, %10u\n", priv0->tx_dropped,  priv1->tx_dropped,  priv7 ? priv7->tx_dropped : 0);

    if ( priv7 )
    {
        seq_printf(seq, "DSL WAN MIB\n");
        seq_printf(seq, "  wrx_correct_pdu:             %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_correct_pdu,               PTM_MIB_TABLE(1)->wrx_correct_pdu);
        seq_printf(seq, "  wrx_correct_pdu_bytes:       %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_correct_pdu_bytes,         PTM_MIB_TABLE(1)->wrx_correct_pdu_bytes);
        seq_printf(seq, "  wrx_tccrc_err_pdu:           %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_tccrc_err_pdu,             PTM_MIB_TABLE(1)->wrx_tccrc_err_pdu);
        seq_printf(seq, "  wrx_tccrc_err_pdu_bytes:     %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_tccrc_err_pdu_bytes,       PTM_MIB_TABLE(1)->wrx_tccrc_err_pdu_bytes);
        seq_printf(seq, "  wrx_ethcrc_err_pdu:          %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_ethcrc_err_pdu,            PTM_MIB_TABLE(1)->wrx_ethcrc_err_pdu);
        seq_printf(seq, "  wrx_ethcrc_err_pdu_bytes:    %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_ethcrc_err_pdu_bytes,      PTM_MIB_TABLE(1)->wrx_ethcrc_err_pdu_bytes);
        seq_printf(seq, "  wrx_nodesc_drop_pdu:         %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_nodesc_drop_pdu,           PTM_MIB_TABLE(1)->wrx_nodesc_drop_pdu);
        seq_printf(seq, "  wrx_len_violation_drop_pdu:  %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_len_violation_drop_pdu,    PTM_MIB_TABLE(1)->wrx_len_violation_drop_pdu);
        seq_printf(seq, "  wrx_idle_bytes:              %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_idle_bytes,                PTM_MIB_TABLE(1)->wrx_idle_bytes);
        seq_printf(seq, "  wrx_nonidle_cw:              %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_nonidle_cw,                PTM_MIB_TABLE(1)->wrx_nonidle_cw);
        seq_printf(seq, "  wrx_idle_cw:                 %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_idle_cw,                   PTM_MIB_TABLE(1)->wrx_idle_cw);
        seq_printf(seq, "  wrx_err_cw:                  %10u, %10u\n", PTM_MIB_TABLE(0)->wrx_err_cw,                    PTM_MIB_TABLE(1)->wrx_err_cw);
        seq_printf(seq, "  wtx_total_pdu:               %10u, %10u\n", PTM_MIB_TABLE(0)->wtx_total_pdu,                 PTM_MIB_TABLE(1)->wtx_total_pdu);
        seq_printf(seq, "  wtx_total_bytes:             %10u, %10u\n", PTM_MIB_TABLE(0)->wtx_total_bytes,               PTM_MIB_TABLE(1)->wtx_total_bytes);
    }

    return 0;
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
    if ( (mib_itf & 0x80) && (g_ptm_net_dev[0] != NULL) )
    {
        //  for PTM
        eth_get_stats(g_ptm_net_dev[0]);
        priv = ifx_eth_fw_netdev_priv(g_ptm_net_dev[0]);
        memset(priv, 0, (u32)&priv->tx_dropped - (u32)priv + sizeof(u32));
        memset((void *)PTM_MIB_TABLE(0), 0, sizeof(struct ptm_mib_table));
        memset((void *)PTM_MIB_TABLE(1), 0, sizeof(struct ptm_mib_table));
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
            void *ptmp = (void *)WTX_QOS_Q_MIB_TABLE(i);
            memset(ptmp, 0, sizeof(struct wtx_qos_q_mib_table));
        }
    }
}


static ssize_t proc_write_mib_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{

    char *p1, *p2;
    int len;
    int colon;
    char local_buf[1024];
    unsigned int mib_itf = 0;
    char *itf_name[] = {"eth0", "eth1", "cpu", "ext_int1", "ext_int2", "ext_int3", "ext_int4", "ptm0"};
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
        printk("echo <on|off|clear> <all|eth0|eth1|ptm0|cpu|ext_int?> > /proc/eth/mib\n");

    return count;

}

#if defined(DEBUG_FIRMWARE_TABLES_PROC) && DEBUG_FIRMWARE_TABLES_PROC

int32_t ppa_route_start_iteration(uint32_t *ppos, struct rout_forward_compare_tbl **pcompare, uint32_t type)
{
	struct rout_forward_compare_tbl *pcomp;
	uint32_t l;

	switch (type)
	{
		case 0x00000000:
			pcomp = (struct rout_forward_compare_tbl *)ROUT_WAN_HASH_CMP_TBL(0);
			g_proc_read_route_pos |= 0x00000000;
			break;
		case 0x10000000:
			pcomp = (struct rout_forward_compare_tbl *)ROUT_WAN_COLL_CMP_TBL(0);
//			g_proc_read_route_pos |= 0x10000000;
			break;
		case 0x20000000:
			pcomp = (struct rout_forward_compare_tbl *)ROUT_LAN_HASH_CMP_TBL(0);
//			g_proc_read_route_pos |= 0x20000000;
			break;
		case 0x30000000:
			pcomp = (struct rout_forward_compare_tbl *)ROUT_LAN_COLL_CMP_TBL(0);
//			g_proc_read_route_pos |= 0x30000000;
			break;
	}

	l = *ppos;	
	if( l == 0 && pcomp )
	{
		++*ppos;
		*pcompare = pcomp;
		return PPA_SUCCESS;
	}
	else
	{
		*pcompare = NULL;
		return PPA_FAILURE;
	}
}

static void *proc_read_route_seq_start(struct seq_file *seq, loff_t *ppos)
{
    struct rout_forward_compare_tbl *pcompare;
    uint32_t type;

    if ( *ppos == 0 )
    {
        g_proc_read_route_pos = 0;
        g_proc_read_route_pos_prev = ~0;
    }
    else
        g_proc_read_route_pos = g_proc_read_route_pos_prev;
    
    type = g_proc_read_route_pos & 0x30000000;
    g_proc_read_route_pos &= 0x0FFFFFFF;
   
    if ( ppa_route_start_iteration(&g_proc_read_route_pos, &pcompare, type) == PPA_SUCCESS )
    {
        *ppos = g_proc_read_route_pos;
//	i = 0;
        return (pcompare);
    }
    else
    {
        ppa_route_stop_iteration();
        g_proc_read_route_pos = 0;
    }
    return NULL;
}

int32_t ppa_route_iterate_next(uint32_t *ppos, struct rout_forward_compare_tbl **pcompare, uint32_t type)
{

    g_proc_read_route_pos = (uint32_t)*ppos;
    
    switch (type)
    {
	case 0x00000000:
    		if( *ppos < (MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK) ) 
    		{
				++*ppos;
			g_proc_read_route_pos |= 0x00000000;
				return PPA_SUCCESS;
    		}
    		else
		{
    			return PPA_FAILURE;
		}
		break;
	case 0x10000000:
    		if( *ppos < WAN_ROUT_TBL_CFG->rout_num ) 
    		{
				++*ppos;
			g_proc_read_route_pos |= 0x10000000;
				return PPA_SUCCESS;
    		}
    		else
		{
			g_proc_read_route_pos |= 0x10000000;
    			return PPA_FAILURE;
		}
	case 0x20000000:
    		if( *ppos < MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK ) 
    		{
				++*ppos;
			g_proc_read_route_pos |= 0x20000000;
				return PPA_SUCCESS;
    		}
    		else
		{
			g_proc_read_route_pos |= 0x20000000;
    			return PPA_FAILURE;
		}
	case 0x30000000:
    		if( *ppos < LAN_ROUT_TBL_CFG->rout_num ) 
    		{
				++*ppos;
			g_proc_read_route_pos |= 0x30000000;
				return PPA_SUCCESS;
    		}
    		else
		{
			g_proc_read_route_pos |= 0x30000000;
    			return PPA_FAILURE;
		}
    }
}

static void *proc_read_route_seq_next(struct seq_file *seq, void *v, loff_t *ppos)
{

	struct rout_forward_compare_tbl *pcompare = (struct rout_forward_compare_tbl *)v;
	uint32_t type;
	g_proc_read_route_pos_prev = (uint32_t)*ppos;
	g_proc_read_route_pos = (uint32_t)*ppos;
	type = g_proc_read_route_pos & 0x30000000;
	g_proc_read_route_pos &= 0x0FFFFFFF;

	if ( ppa_route_iterate_next(&g_proc_read_route_pos, &pcompare, type) == PPA_SUCCESS )
    	{
		*ppos = g_proc_read_route_pos;
		pcompare++;
        	return (pcompare);
    	}
	else
	{
		ppa_route_stop_iteration();
		type = g_proc_read_route_pos & 0x30000000;
		switch (type)
		{
			case 0x00000000:
				pcompare = (struct rout_forward_compare_tbl *)ROUT_WAN_COLL_CMP_TBL(0);
				type = 0x10000000;
				break;
			case 0x10000000:
				pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_HASH_CMP_TBL(0);
				type = 0x20000000;
				break;
			case 0x20000000:
				pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_COLL_CMP_TBL(0);
				type = 0x30000000;
				break;
			case 0x30000000:
				return NULL;
		}
		g_proc_read_route_pos = 0;
		if ( ppa_route_start_iteration(&g_proc_read_route_pos, &pcompare, type) == PPA_SUCCESS )
		{
			switch (type)
			{
				case 0x10000000:
					g_proc_read_route_pos |= 0x10000000;
					*ppos = g_proc_read_route_pos;
					pcompare = (struct rout_forward_compare_tbl *)ROUT_WAN_COLL_CMP_TBL(0);
					break;
				case 0x20000000:
					g_proc_read_route_pos |= 0x20000000;
					*ppos = g_proc_read_route_pos;
					pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_HASH_CMP_TBL(0);
					break;
				case 0x30000000:
					g_proc_read_route_pos |= 0x30000000;
					*ppos = g_proc_read_route_pos;
					pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_COLL_CMP_TBL(0);
					break;
				case 0x00000000:
					g_proc_read_route_pos |= 0x00000000;
					*ppos = g_proc_read_route_pos;
					return NULL;
			}
			*ppos = g_proc_read_route_pos;
        		return (pcompare);
                }
	}
	return NULL;
}

static void proc_read_route_seq_stop(struct seq_file *seq, void *v)
{
	printk(" ");
//	return 0;
}

void ppa_route_stop_iteration(void)
{
	printk(" ");
//	return 0;
}


static int proc_read_route_seq_show(struct seq_file *seq, void *v)
{

    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "EXT_INT4", "PTM"};
    struct rout_forward_compare_tbl *pcompare;
    struct rout_forward_action_tbl *pwaction;

    int f_is_lan,type;
    u32 bit;
    int i,j, k;
    type = g_proc_read_route_pos & 0x30000000;
    g_proc_read_route_pos &= 0x0FFFFFFF;
    switch( type )
    {
	case 0x00000000:
		if(g_proc_read_route_pos == 1)
			seq_printf(seq, "Wan Routing Table\n");	
    		pcompare = (struct rout_forward_compare_tbl *)ROUT_WAN_HASH_CMP_TBL((uint32_t)g_proc_read_route_pos - 1);
    		pwaction = (struct rout_forward_action_tbl *)ROUT_WAN_HASH_ACT_TBL((uint32_t)g_proc_read_route_pos - 1);
		f_is_lan = 0;
		i = ((uint32_t)g_proc_read_route_pos - 1);
		break;
	case 0x10000000:
    		pcompare = (struct rout_forward_compare_tbl *)ROUT_WAN_COLL_CMP_TBL((uint32_t)g_proc_read_route_pos - 1);
    		pwaction = (struct rout_forward_action_tbl *)ROUT_WAN_COLL_ACT_TBL((uint32_t)g_proc_read_route_pos - 1);
		f_is_lan = 0;
		i = ((uint32_t)g_proc_read_route_pos - 1 + (MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK));
		break;
	case 0x20000000:
		if(g_proc_read_route_pos == 1)
			seq_printf(seq, "Lan Routing Table\n");	
    		pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_HASH_CMP_TBL((uint32_t)g_proc_read_route_pos - 1);
    		pwaction = (struct rout_forward_action_tbl *)ROUT_LAN_HASH_ACT_TBL((uint32_t)g_proc_read_route_pos - 1);
		f_is_lan = 1;
		i = ((uint32_t)g_proc_read_route_pos - 1);
		break;
	case 0x30000000:
    		pcompare = (struct rout_forward_compare_tbl *)ROUT_LAN_COLL_CMP_TBL((uint32_t)g_proc_read_route_pos - 1);
    		pwaction = (struct rout_forward_action_tbl *)ROUT_LAN_COLL_ACT_TBL((uint32_t)g_proc_read_route_pos - 1);
		f_is_lan = 1;
		i = ((uint32_t)g_proc_read_route_pos - 1 + (MAX_ROUTING_ENTRIES_PER_HASH_BLOCK * MAX_HASH_BLOCK));
		break;
	default: f_is_lan = 1;
    }
    
    if ( *(u32*)pcompare && *((u32*)pcompare + 1) )
    {
    seq_printf(seq,          "  entry %d\n", i);
    seq_printf(seq,          "    compare (0x%08X)\n", (u32)pcompare);
    seq_printf(seq,          "      src:  %d.%d.%d.%d:%d\n", pcompare->src_ip >> 24,  (pcompare->src_ip >> 16) & 0xFF,  (pcompare->src_ip >> 8) & 0xFF,  pcompare->src_ip & 0xFF, pcompare->src_port);
    seq_printf(seq,          "      dest: %d.%d.%d.%d:%d\n", pcompare->dest_ip >> 24, (pcompare->dest_ip >> 16) & 0xFF, (pcompare->dest_ip >> 8) & 0xFF, pcompare->dest_ip & 0xFF, pcompare->dest_port);
    seq_printf(seq,          "    action  (0x%08X)\n", (u32)pwaction);
    seq_printf(seq,          "      new %s    %d.%d.%d.%d:%d\n", f_is_lan ? "src :" : "dest:", pwaction->new_ip >> 24, (pwaction->new_ip >> 16) & 0xFF, (pwaction->new_ip >> 8) & 0xFF, pwaction->new_ip & 0xFF, pwaction->new_port);
    seq_printf(seq,          "      new MAC :    %02X:%02X:%02X:%02X:%02X:%02X\n", (pwaction->new_dest_mac54 >> 8) & 0xFF, pwaction->new_dest_mac54 & 0xFF, pwaction->new_dest_mac30 >> 24, (pwaction->new_dest_mac30 >> 16) & 0xFF, (pwaction->new_dest_mac30 >> 8) & 0xFF, pwaction->new_dest_mac30 & 0xFF);
    switch ( pwaction->rout_type )
    {
    case 1:  seq_printf(seq, "      route type:  IP\n");   break;
    case 2:  seq_printf(seq, "      route type:  NAT\n");  break;
    case 3:  seq_printf(seq, "      route type:  NAPT\n"); break;
    default: seq_printf(seq, "      route type:  NULL\n");
    }
    if ( pwaction->new_dscp_en )
        seq_printf(seq,      "      new DSCP:    %d\n", pwaction->new_dscp);
    else
        seq_printf(seq,      "      new DSCP:    original (not modified)\n");
    seq_printf(seq,          "      MTU index:   %d\n", pwaction->mtu_ix);
    if ( pwaction->in_vlan_ins )
        seq_printf(seq,      "      VLAN insert: enable, VCI 0x%04x\n", pwaction->new_in_vci);
    else
        seq_printf(seq,      "      VLAN insert: disable\n");
    seq_printf(seq,          "      VLAN remove: %s\n", pwaction->in_vlan_rm ? "enable" : "disable");
    if ( !pwaction->dest_list )
        seq_printf(seq,      "      dest list:   none\n");
    else
    {
        seq_printf(seq,      "      dest list:   ");
        for ( bit = 1, j = k = 0; bit < 1 << 8; bit <<= 1, j++ )
            if ( (pwaction->dest_list & bit) )
            {
                if ( k )
                    seq_printf(seq, ", ");
                seq_printf(seq, dest_list[j]);
                k = 1;
            }
        seq_printf(seq, "\n");
    }
    if ( pwaction->pppoe_mode )
    {
        seq_printf(seq,      "      PPPoE mode:  termination\n");
        if ( f_is_lan )
            seq_printf(seq,  "      PPPoE index: %d\n", pwaction->pppoe_ix);
    }
    else
        seq_printf(seq,      "      PPPoE mode:  transparent\n");
    seq_printf(seq,          "      new src MAC index: %d\n", pwaction->new_src_mac_ix);
    if(f_is_lan){
        seq_printf(seq,      "      encap tunnel: %s\n", pwaction->encap_tunnel ? "yes":"no");
        seq_printf(seq,      "      tunnel iphdr idx: %d\n", pwaction->tnnl_hdr_idx);
    }else{
        seq_printf(seq,      "      decap tunnel: %s\n", pwaction->encap_tunnel ? "yes":"no");
    }
    if ( pwaction->out_vlan_ins )
        seq_printf(seq,      "      outer VLAN insert: enable, index %d\n", pwaction->out_vlan_ix);
    else
        seq_printf(seq,      "      outer VLAN insert: disable\n");
    seq_printf(seq,          "      outer VLAN remove: %s\n", pwaction->out_vlan_rm ? "enable" : "disable");
    seq_printf(seq,          "      tcp:         %s\n", pwaction->protocol ? "yes" : "no (UDP)");
    seq_printf(seq,          "      entry valid: %s\n", pwaction->entry_vld ? "yes" : "no");
    seq_printf(seq,          "      dest qid (dslwan qid): %d\n", pwaction->dest_qid);
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    {
         if(PS_MC_GENCFG3->session_mib_unit == 1)
            seq_printf(seq,          "      accl packets: %u\n", pwaction->bytes);
         else
            seq_printf(seq,          "      accl bytes: %u\n", pwaction->bytes * 32);
    }

#else
    seq_printf(seq,          "      accl bytes: %u\n", pwaction->bytes);
#endif
    }
	return 0;

}

static ssize_t proc_write_route_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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


    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "EXT_INT4", "PTM"};
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

            if ( ppa_drv_hal_generic_hook == NULL )
            {
                printk("HAL is not loaded - verify only, no firmware table operation\n");
                printk("del_routing_entry(%d - %08x)\n", entry, (u32)entry | is_lan);
            }
            else
            {
                PPE_ROUTING_INFO route = {0};
                int ret;

                route.entry = (u32)entry | is_lan;
                ret = ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_DEL_ROUTE_ENTRY, &route, 0);
                printk("%s entry %d deleted %s\n", is_lan ? "LAN" : "WAN", entry, ret == PPA_SUCCESS ? "successfully" : "fail");
            }
        }
    }
    else if ( operation == 1 && type && ((!is_ipv6 && *(u32*)&compare_tbl && *((u32*)&compare_tbl + 1)) || (is_ipv6 && *((u32*)&compare_tbl + 2))) )
    {
        if ( ppa_drv_hal_generic_hook == NULL )
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

            if ( ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_ADD_ROUTE_ENTRY, &route, 0) == PPA_SUCCESS )
                printk("%s entry add successfully: entry = %d\n", (route.entry & (1 << 31)) ? "LAN" : "WAN", route.entry & ~(1 << 31));
            else
                printk("%s entry add fail\n", type == 1 ? "LAN" : "WAN");
        }
    }
    else
        printk("No operation: operation = %d, type = %d, is_ipv6 = %d\n", operation, type, is_ipv6);

    return count;
}


static int proc_read_mc(struct seq_file *seq, void *v)
{
    volatile struct wan_rout_multicast_cmp_tbl *pcompare;
    volatile struct wan_rout_multicast_act_tbl *paction;
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    volatile struct rtp_sampling_cnt *rtp_mib;   
#endif
    int i;


    seq_printf(seq, "Multicast Table\n");

    pcompare = ROUT_WAN_MC_CMP_TBL(0);
    paction = ROUT_WAN_MC_ACT_TBL(0);
 #if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    rtp_mib= MULTICAST_RTP_MIB_TBL(0);
#endif
    for ( i = 0; i < MAX_WAN_MC_ENTRIES; i++ )
    {
        if ( pcompare->wan_dest_ip )
        {
            print_mc_seq(
                             seq, i,
                             (struct wan_rout_multicast_cmp_tbl *)pcompare,
                             (struct wan_rout_multicast_act_tbl *)paction,
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
                           (struct rtp_sampling_cnt *)rtp_mib,
#endif
                             (uint32_t) ROUT_WAN_MC_CNT(i),
                             (uint32_t) *ROUT_WAN_MC_CNT(i)
                           );
        }

        pcompare++;
        paction++;
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
        rtp_mib++;
#endif
    }

    return 0;
}


#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
static INLINE int print_mc_seq(struct seq_file *seq, int i, struct wan_rout_multicast_cmp_tbl *pcompare, struct wan_rout_multicast_act_tbl *paction, struct rtp_sampling_cnt *rtp_mib,uint32_t mc_mib_addr, uint32_t rt_mc_cnt)
#else
static INLINE int print_mc_seq(struct seq_file *seq, int i, struct wan_rout_multicast_cmp_tbl *pcompare, struct wan_rout_multicast_act_tbl *paction, uint32_t mc_mib_addr, uint32_t rt_mc_cnt)
#endif
{
    static const char *rout_type[] = {"no IP level editing", "TTL editing", "reserved - 2", "reserved - 3"};
    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "res", "ATM"};

    u32 bit;
    int j, k;

    seq_printf(seq,          "  entry %d - %s\n", i, paction->entry_vld ? "valid" : "invalid");
    seq_printf(seq,          "    compare (0x%08X)\n", (u32)pcompare);
    seq_printf(seq,          "      wan_dest_ip:  %d.%d.%d.%d\n", (pcompare->wan_dest_ip >> 24) & 0xFF,  (pcompare->wan_dest_ip >> 16) & 0xFF,  (pcompare->wan_dest_ip >> 8) & 0xFF,  pcompare->wan_dest_ip & 0xFF);
    seq_printf(seq,          "      wan_src_ip:   %d.%d.%d.%d\n", (pcompare->wan_src_ip >> 24) & 0xFF,  (pcompare->wan_src_ip >> 16) & 0xFF,  (pcompare->wan_src_ip >> 8) & 0xFF,  pcompare->wan_src_ip & 0xFF);
    seq_printf(seq,          "    action  (0x%08X)\n", (u32)paction);
    seq_printf(seq,          "      rout_type:    %s\n", rout_type[paction->rout_type]);
    if ( paction->new_dscp_en )
        seq_printf(seq,      "      new DSCP:     %d\n", paction->new_dscp);
    else
        seq_printf(seq,      "      new DSCP:     original (not modified)\n");
    if ( paction->in_vlan_ins )
        seq_printf(seq,      "      VLAN insert:  enable, VCI 0x%04x\n", paction->new_in_vci);
    else
        seq_printf(seq,      "      VLAN insert:  disable\n");
    seq_printf(seq,          "      VLAN remove:  %s\n", paction->in_vlan_rm ? "enable" : "disable");
    if ( !paction->dest_list )
        seq_printf(seq,      "      dest list:    none\n");
    else
    {
        seq_printf(seq,      "      dest list:   ");
        for ( bit = 1, j = k = 0; bit < 1 << 8; bit <<= 1, j++ )
            if ( (paction->dest_list & bit) )
            {
                if ( k )
                    seq_printf(seq, ", ");
                seq_printf(seq, dest_list[j]);
                k = 1;
            }
        seq_printf(seq, "\n");
    }
    seq_printf(seq,          "      PPPoE mode:   %s\n", paction->pppoe_mode ? "termination" : "transparent");

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    seq_printf(seq,          "      RTP sampling    : %s\n", paction->sample_en ? "enable" : "disable");
   if(paction->sample_en) 
   {
    seq_printf(seq,          "      RTP packet count: %d\n", rtp_mib->pkt_cnt);
    seq_printf(seq,          "      RTP seq number  : %d\n", rtp_mib->seq_no);
   }
#endif

    if ( paction->new_src_mac_en )
        seq_printf(seq,      "      new src MAC index: %d\n", paction->new_src_mac_ix);
    else
        seq_printf(seq,      "      new src MAC index: disabled\n");
    if ( paction->out_vlan_ins )
        seq_printf(seq,      "      outer VLAN insert: enable, index %d\n", paction->out_vlan_ix);
    else
        seq_printf(seq,      "      outer VLAN insert: disable\n");
    seq_printf(seq,          "      outer VLAN remove: %s\n", paction->out_vlan_rm ? "enable" : "disable");
    seq_printf(seq,          "      dest_qid:          %d\n", paction->dest_qid);
    seq_printf(seq,          "      Tunnel Remove    : %s\n", paction->tunnel_rm ? "enable" : "disable");
    seq_printf(seq,          "    MIB: (0x%08x)\n", mc_mib_addr);
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    {
         if(PS_MC_GENCFG3->session_mib_unit == 1)
            seq_printf(seq,          "      acc packets      : %u\n", rt_mc_cnt);
         else
            seq_printf(seq,          "      acc bytes        : %u\n", rt_mc_cnt);
    }

#else

    seq_printf(seq,          "      acc bytes        : %u\n", rt_mc_cnt);
#endif

    return 0;
}

static int proc_read_genconf(struct seq_file *seq, void *v)
{
    int i;
    unsigned long bit;
#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
    int num = 0;
#endif


    seq_printf(seq, "CFG_WRDES_DELAY  (0x%08X): %d\n", (u32)CFG_WRDES_DELAY, *CFG_WRDES_DELAY);
    seq_printf(seq, "WRX_EMACH_ON     (0x%08X): AAL - %s, OAM - %s\n", (u32)WRX_EMACH_ON, (*WRX_EMACH_ON & 0x01) ? "on" : "off",
                                                                                                      (*WRX_EMACH_ON & 0x02) ? "on" : "off");
    seq_printf(seq, "WTX_EMACH_ON     (0x%08X):", (u32)WTX_EMACH_ON);
    for ( i = 0, bit = 1; i < 15; i++, bit <<= 1 )
    {
        seq_printf(seq, " %d - %s", i, (*WTX_EMACH_ON & bit) ? "on " : "off");
        if ( i == 3 || i == 7 || i == 11 )
            seq_printf(seq, "\n                               ");
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "WRX_HUNT_BITTH   (0x%08X): %d\n", (u32)WRX_HUNT_BITTH, *WRX_HUNT_BITTH);

    seq_printf(seq, "MIXED_WAN_VLAN   (0x%08X): WAN VLAN ID Hi - %d, WAN VLAN ID Lo - %d\n", (u32)MIXED_WAN_VLAN_CFG, MIXED_WAN_VLAN_CFG->wan_vlanid_hi, MIXED_WAN_VLAN_CFG->wan_vlanid_lo);

    seq_printf(seq, "LAN_ROUT_TBL_CFG (0x%08X): num - %d, TTL check - %s, no hit drop - %s\n",
                                                            (u32)LAN_ROUT_TBL_CFG,
                                                            LAN_ROUT_TBL_CFG->rout_num,
                                                            LAN_ROUT_TBL_CFG->ttl_en ? "on" : "off",
                                                            LAN_ROUT_TBL_CFG->rout_drop  ? "on" : "off"
                                                            );
    seq_printf(seq, "                               IP checksum - %s, TCP/UDP checksum - %s, checksum error drop - %s\n",
                                                            LAN_ROUT_TBL_CFG->ip_ver_en ? "on" : "off",
                                                            LAN_ROUT_TBL_CFG->tcpdup_ver_en ? "on" : "off",
                                                            LAN_ROUT_TBL_CFG->iptcpudperr_drop ? "on" : "off"
                                                            );

    seq_printf(seq, "WAN_ROUT_TBL_CFG (0x%08X): num - %d, TTL check - %s, no hit drop - %s\n",
                                                            (u32)WAN_ROUT_TBL_CFG,
                                                            WAN_ROUT_TBL_CFG->rout_num,
                                                            WAN_ROUT_TBL_CFG->ttl_en ? "on" : "off",
                                                            WAN_ROUT_TBL_CFG->rout_drop  ? "on" : "off"
                                                            );
    seq_printf(seq, "                               MC num - %d, MC drop - %s\n",
                                                            WAN_ROUT_TBL_CFG->wan_rout_mc_num,
                                                            WAN_ROUT_TBL_CFG->wan_rout_mc_drop ? "on" : "off"
                                                            );
    seq_printf(seq, "                               IP checksum - %s, TCP/UDP checksum - %s, checksum error drop - %s\n",
                                                            WAN_ROUT_TBL_CFG->ip_ver_en ? "on" : "off",
                                                            WAN_ROUT_TBL_CFG->tcpdup_ver_en ? "on" : "off",
                                                            WAN_ROUT_TBL_CFG->iptcpudperr_drop ? "on" : "off"
                                                            );

    seq_printf(seq, "GEN_MODE_CFG1    (0x%08X): App2 - %s, U/S - %s, U/S early discard - %s\n"
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
    seq_printf(seq, "WAN Interfaces:                %s \n", get_wanitf(1));
    seq_printf(seq, "LAN Interfaces:                %s \n", get_wanitf(0));
    seq_printf(seq, "GEN_MODE_CFG2    (0x%08X):", (u32)GEN_MODE_CFG2);
    for ( i = 0, bit = 1; i < 8; i++, bit <<= 1 )
    {
        seq_printf(seq, " %d - %s", i, (GEN_MODE_CFG2->itf_outer_vlan_vld & bit) ? "on " : "off");
        if ( i == 3 )
            seq_printf(seq, "\n                              ");
    }
    seq_printf(seq, "\n");

    seq_printf(seq, "TX_QOS_CFG       (0x%08X): time_tick - %d, overhd_bytes - %d, eth1_eg_qnum - %d\n"
                                "                               eth1_burst_chk - %s, eth1 - %s, rate shaping - %s, WFQ - %s\n",
                                                            (u32)TX_QOS_CFG,
                                                            TX_QOS_CFG->time_tick,
                                                            TX_QOS_CFG->overhd_bytes,
                                                            TX_QOS_CFG->eth1_eg_qnum,
                                                            TX_QOS_CFG->eth1_burst_chk ? "on" : "off",
                                                            TX_QOS_CFG->eth1_qss ? "FW QoS" : "HW QoS",
                                                            TX_QOS_CFG->shape_en ? "on" : "off",
                                                            TX_QOS_CFG->wfq_en ? "on" : "off");

    seq_printf(seq, "KEY_SEL_n        (0x%08X): %08x %08x %08x %08x\n",
                                                            (u32)KEY_SEL_n(0),
                                                            *KEY_SEL_n(0), *KEY_SEL_n(1), *KEY_SEL_n(2), *KEY_SEL_n(3));

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    seq_printf(seq, "MIB Mode - %s\n",
                                                         PS_MC_GENCFG3->session_mib_unit
                                                         ? "Packet" : "Byte"
                                                            );
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
    for ( i = 0; i < MAX_CAPWAP_ENTRIES; i++ )
    {
         if( CAPWAP_CONFIG_TBL(i)->acc_en == 1 )
            num +=1;
    }
    
    seq_printf(seq, "Maximum CAPWAP tunnel- %d\n", num);
#endif


    return 0;
}


static ssize_t proc_write_genconf_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
            MIXED_WAN_VLAN_CFG->eth0_type = 0;
        else if ( stricmp(p, "wan") == 0 )
            MIXED_WAN_VLAN_CFG->eth0_type = 1;
        else if ( stricmp(p, "mix") == 0 )
            MIXED_WAN_VLAN_CFG->eth0_type = 2;
    }

    if ( f_wan_hi )
    {
        int num;

        num = get_number(&p, &rlen, 0);
        if ( f_wan_hi > 0 )
            MIXED_WAN_VLAN_CFG->wan_vlanid_hi = num;
        else
            MIXED_WAN_VLAN_CFG->wan_vlanid_lo = num;
    }

    return count;
}


static int proc_read_pppoe(struct seq_file *seq, void *v)
{
    int i;

    seq_printf(seq, "PPPoE Table (0x%08X) - Session ID:\n", (u32)PPPOE_CFG_TBL(0));
    for ( i = 0; i < 8; i++ )
    {
        seq_printf(seq, "  %d: %d\n", i, *PPPOE_CFG_TBL(i));
    }

      return 0;
}


static int proc_read_mtu(struct seq_file *seq, void *v)
{
    int i;

    seq_printf(seq, "MTU Table (0x%08X):\n", (u32)MTU_CFG_TBL(0));

    for ( i = 0; i < 8; i++ )
    {
        seq_printf(seq, "  %d: %d\n", i, *MTU_CFG_TBL(i));
    }

      return 0;
}

static int proc_read_hit(struct seq_file *seq, void *v)
{
    int i;
    int n;
    unsigned long bit;

    seq_printf(seq, "Unicast Routing Hit Table (0x%08X):\n", (u32)ROUT_LAN_HASH_HIT_STAT_TBL(0));

    seq_printf(seq, "             1 2 3 4 5 6 7 8 9 10\n");

    n = 1;
    for ( i = 0; i < 8; i++ )
        for ( bit = 0x80000000; bit; bit >>= 1 )
        {
            if ( n % 10 == 1 )
                seq_printf(seq, "  %3d - %3d:", n, n + 9);

            seq_printf(seq, " %d", (*ROUT_LAN_HASH_HIT_STAT_TBL(i) & bit) ? 1 : 0);

            if ( n++ % 10 == 0 )
            {
                seq_printf(seq, "\n");

            }
        }

    if ( n % 10 != 0 )
    {
        seq_printf(seq, "\n");

    }

    seq_printf(seq, "Multicast Routing Hit Table (0x%08X):\n", (u32)ROUT_WAN_MC_HIT_STAT_TBL(0));
    seq_printf(seq, "             1 2 3 4 5 6 7 8 9 10\n");

    n = 1;
    for ( i = 0; i < 1; i++ )
        for ( bit = 0x80000000; bit; bit >>= 1 )
        {
            if ( n % 10 == 1 )
                seq_printf(seq, "  %3d - %3d:", n, n + 9);

            seq_printf(seq, " %d", (*ROUT_WAN_MC_HIT_STAT_TBL(i) & bit) ? 1 : 0);

            if ( n++ % 10 == 0 )
            {
                seq_printf(seq, "\n");

            }
        }

    if ( n % 10 != 0 )
    {
        seq_printf(seq, "\n");

    }

      return 0;
}

static ssize_t proc_write_hit_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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

static int proc_read_mac(struct seq_file *seq, void *v)
{
    int i;
    unsigned int mac52, mac10;

    seq_printf(seq, "MAC Table:\n");

    seq_printf(seq, "  ROUT_MAC_CFG_TBL (0x%08X)\n", (u32)ROUT_MAC_CFG_TBL(0));

    for ( i = 0; i < 16; i++ )
    {
        mac52 = *ROUT_MAC_CFG_TBL(i);
        mac10 = *(ROUT_MAC_CFG_TBL(i) + 1);

        seq_printf(seq, "    %2d: %02X:%02X:%02X:%02X:%02X:%02X\n", i + 1, mac52 >> 24, (mac52 >> 16) & 0xFF, (mac52 >> 8) & 0xFF, mac52 & 0xFF, mac10 >> 24, (mac10 >> 16) & 0xFF);
    }

      return 0;
}

static int proc_read_out_vlan(struct seq_file *seq, void *v)
{
    int i;

    seq_printf(seq, "Outer VLAN Table (0x%08X):\n", (u32)OUTER_VLAN_TBL(0));
    
    for ( i = 0; i < 32; i++ )
    {
        seq_printf(seq, "  %d: 0x%08X\n", i, *OUTER_VLAN_TBL(i));
    }

    return 0;
}

static int proc_read_ipv6_ip(struct seq_file *seq, void *v)
{

    int i, j, x;
    volatile char *p;

    seq_printf(seq, "IPv6 IP Table:\n");

    for ( x = 0; x < MAX_IPV6_IP_ENTRIES_BLOCK; x++ )
        for ( i = 0; i < MAX_IPV6_IP_ENTRIES_PER_BLOCK; i++ )
            if ( IPv6_IP_IDX_TBL(x, i)[0] != 0 || IPv6_IP_IDX_TBL(x, i)[1] != 0 || IPv6_IP_IDX_TBL(x, i)[2] != 0 || IPv6_IP_IDX_TBL(x, i)[3] != 0 )
            {
                p = (volatile char *)IPv6_IP_IDX_TBL(x, i);
                seq_printf(seq, "%3d - %u", x * MAX_IPV6_IP_ENTRIES_PER_BLOCK + i, (unsigned int)p[0]);
                for ( j = 1; j < 16; j++ )
                    seq_printf(seq, ".%u", (unsigned int)p[j]);
                seq_printf(seq, "\n");

            }

    return 0;
}
#endif

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
static int proc_read_dbg(struct seq_file *seq, void *v)
{

    seq_printf(seq, "error print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ERR)              ? "enabled" : "disabled");
    seq_printf(seq, "debug print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT)      ? "enabled" : "disabled");
    seq_printf(seq, "assert           - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ASSERT)           ? "enabled" : "disabled");
    seq_printf(seq, "dump rx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_RX)      ? "enabled" : "disabled");
    seq_printf(seq, "dump tx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_TX)      ? "enabled" : "disabled");
    seq_printf(seq, "dump flag header - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_FLAG_HEADER) ? "enabled" : "disabled");
    seq_printf(seq, "dump init        - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_INIT)        ? "enabled" : "disabled");
    seq_printf(seq, "mac swap         - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_MAC_SWAP)         ? "enabled" : "disabled");
  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST
    seq_printf(seq, "FW Mode          - DSL WAN\n");
    seq_printf(seq, "WAN Hash Algo    - %s\n", (ppe_wan_hash_algo)   ? "Algo 1"   : "Algo 0");
    seq_printf(seq, "ACC Protocol     - %s\n", (acc_proto)        ? "TCP" : "UDP");
    seq_printf(seq, "MPoA Type        - EoA1\n");
  #endif

    return 0;
}

static ssize_t proc_write_dbg_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
        " mac swap",
        " swap",
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
        9,  5,
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
        DBG_ENABLE_MASK_MAC_SWAP,
        DBG_ENABLE_MASK_ALL & ~DBG_ENABLE_MASK_MAC_SWAP
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
    if ( strincmp(p, "set_acc_udp", 11) == 0 ) {
    if (acc_proto != 0) {
        acc_proto = 0;
        init_acc_tables();
    }
    }
    else if ( strincmp(p, "set_acc_tcp", 11) == 0 ) {
    if (acc_proto != 1) {
        acc_proto = 1;
        init_acc_tables();
    }
    }
    else if ( strincmp(p, "set_mix_mode", 12) == 0 ) {
    if (ppe_fw_mode != 1) {
        ppe_fw_mode = 1;
        init_acc_tables();
    }
    }
    else if ( strincmp(p, "set_normal_mode", 15) == 0 ) {
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
    }
    else if ( strincmp(p, "set_algo_1", 10) == 0 ) {
    if (ppe_wan_hash_algo != 1) {
        ppe_wan_hash_algo = 1;
        init_acc_tables();
    }
    }
    else
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
        printk("echo <enable/disable> [err/dbg/assert/rx/tx/header/init/swap/all] > /proc/eth/dbg\n");
  #if defined(ENABLE_FW_MODULE_TEST) && ENABLE_FW_MODULE_TEST
        printk("echo set_mix_mode/set_normal_mode/set_algo_0/set_algo_1\n");
        printk("echo set_acc_udp/set_acc_tcp\n");
  #endif
    }

    if ( f_enable )
    {
        if ( *p == 0 )
        {
            if ( f_enable > 0 )
                g_dbg_enable |= DBG_ENABLE_MASK_ALL & ~DBG_ENABLE_MASK_MAC_SWAP;
            else
                g_dbg_enable &= ~DBG_ENABLE_MASK_ALL | DBG_ENABLE_MASK_MAC_SWAP;
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


#if defined(DEBUG_MEM_PROC) && DEBUG_MEM_PROC
static ssize_t proc_write_mem_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char *p1, *p2;
    int len;
    int colon;
    unsigned long *p;
    char local_buf[1024];
    int i, n, l;
    int is_switch = 0;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
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
        if ( p2[0] == 's' && p2[1] == 'w' && (p2[2] == ' ' || p2[2] == '\t') )
        {
            unsigned long temp;

            is_switch = 1;
            p2 += 3;
            len -= 3;
            ignore_space(&p2, &len);
            temp = get_number(&p2, &len, 1);
            p = (unsigned long *)AR10_SWIP_MACRO_REG(temp);
        }
        else
        {
            p = (unsigned long *)get_number(&p2, &len, 1);
            p = (unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);
        }

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
        if ( p2[0] == 's' && p2[1] == 'w' && (p2[2] == ' ' || p2[2] == '\t') )
        {
            unsigned long temp;

            is_switch = 1;
            p2 += 3;
            len -= 3;
            ignore_space(&p2, &len);
            temp = get_number(&p2, &len, 1);
            p = (unsigned long *)AR10_SWIP_MACRO_REG(temp);
        }
        else
        {
            p = (unsigned long *)get_number(&p2, &len, 1);
            p = (unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);
        }

        if ( (u32)p >= KSEG0 )
        {
            ignore_space(&p2, &len);
            n = (int)get_number(&p2, &len, 0);
            if ( n )
            {
                char str[32] = {0};
                char *pch = str;
                int k;
                u32 data;
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
                        data = (u32)*p;
                        printk(" %08X", data);
                        for ( k = 0; k < 4; k++ )
                        {
                            c = ((char*)&data)[k];
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
static int proc_read_pp32(struct seq_file *seq, void *v)
{
    static const char *stron = " on";
    static const char *stroff = "off";

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
    seq_printf(seq, "Context: %d, PC: 0x%04x, %s\n", cur_context, PP32_CPU_CUR_PC, str);

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
        seq_printf(seq, "break reason: %s\n", str);
    }

    if ( f_stopped )
    {
        seq_printf(seq, "General Purpose Register (Context %d):\n", cur_context);
        for ( i = 0; i < 4; i++ )
        {
            for ( j = 0; j < 4; j++ )
                seq_printf(seq, "   %2d: %08x", i + j * 4, *PP32_GP_CONTEXTi_REGn(cur_context, i + j * 4));
            seq_printf(seq, "\n");
        }
    }

    seq_printf(seq, "break out on: break in - %s, stop - %s\n",
                                        PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN ? stron : stroff,
                                        PP32_CTRL_OPT_BREAKOUT_ON_STOP ? stron : stroff);
    seq_printf(seq, "     stop on: break in - %s, break point - %s\n",
                                        PP32_CTRL_OPT_STOP_ON_BREAKIN ? stron : stroff,
                                        PP32_CTRL_OPT_STOP_ON_BREAKPOINT ? stron : stroff);
    seq_printf(seq, "breakpoint:\n");
    seq_printf(seq, "     pc0: 0x%08x, %s\n", *PP32_BRK_PC(0), PP32_BRK_GRPi_PCn(0, 0) ? "group 0" : "off");
    seq_printf(seq, "     pc1: 0x%08x, %s\n", *PP32_BRK_PC(1), PP32_BRK_GRPi_PCn(1, 1) ? "group 1" : "off");
    seq_printf(seq, "  daddr0: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(0), PP32_BRK_GRPi_DATA_ADDRn(0, 0) ? "group 0" : "off");
    seq_printf(seq, "  daddr1: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(1), PP32_BRK_GRPi_DATA_ADDRn(1, 1) ? "group 1" : "off");
    seq_printf(seq, "  rdval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(0));
    seq_printf(seq, "  rdval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(1));
    seq_printf(seq, "  wrval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(0));
    seq_printf(seq, "  wrval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(1));

    return 0;
}

static ssize_t proc_write_pp32_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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


static int proc_read_burstlen(struct seq_file *seq, void *v)
{
    u32 dma_ps;
    u32 dma_pctrl;
    unsigned long sys_flag;

    local_irq_save(sys_flag);
    dma_ps = *(volatile u32 *)0xBE104140;
    *(volatile u32 *)0xBE104140 = 0;
    dma_pctrl = *(volatile u32 *)0xBE104144;
    *(volatile u32 *)0xBE104140 = dma_ps;
    local_irq_restore(sys_flag);

    seq_printf(seq, "DMA-PPE burst length: Rx %d, Tx %d\n", 1 << ((dma_pctrl >> 2) & 0x03), 1 << ((dma_pctrl >> 4) & 0x03));

    return 0;
}

static ssize_t proc_write_burstlen_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
static int proc_read_fw(struct seq_file *seq, void *v)
{
//    u32 sll_state;
    u32 tir4;
    u32 tir4_sll_state;
    u32 tir4_dplus_rx_on;

    seq_printf(seq, "Firmware\n");
//    seq_printf(seq, "  ACC_ERX_PID        = %08X\n", *ACC_ERX_PID);
//    seq_printf(seq, "  ACC_ERX_PORT_TIMES = %08X\n", *ACC_ERX_PORT_TIMES);

//    sll_state = *SLL_ISSUED;
    tir4 = *TIR(4);
    tir4_sll_state = (tir4 >> 21) & 7;
    tir4_dplus_rx_on = (tir4 >> 20) & 1;

    /*
    seq_printf(seq, "  SLL_ISSUED         = %d [%s], tir4.sll_state = %d [%s]\n",
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

    seq_printf(seq, "  DPLUS_RX_ON        = %d, tir4.dplus_rx_on = %d\n",
            *DPLUS_RX_ON, tir4_dplus_rx_on);


    seq_printf(seq, "  BMC_ISSUED         = %08X\n", *BMC_ISSUED);

    seq_printf(seq, "  PRESEARCH_RDPTR    = %08X, %08X\n",
            *PRESEARCH_RDPTR, *(PRESEARCH_RDPTR + 1));

    seq_printf(seq, "  SLL_ERX_PID        = %08X\n", *SLL_ERX_PID);

    seq_printf(seq, "  SLL_PKT_CNT        = %08X, %08X\n",
            *SLL_PKT_CNT, *(SLL_PKT_CNT + 1));

    seq_printf(seq, "  SLL_RDPTR          = %08X, %08X\n",
            *SLL_RDPTR, *(SLL_RDPTR + 1));


    seq_printf(seq, "  EDIT_PKT_CNT       = %08X, %08X\n",
            *EDIT_PKT_CNT, *(EDIT_PKT_CNT + 1));
    seq_printf(seq, "  EDIT_RDPTR         = %08X, %08X\n",
            *EDIT_RDPTR, *(EDIT_RDPTR + 1));

    seq_printf(seq, "  DPLUSRX_PKT_CNT    = %08X, %08X\n",
            *DPLUSRX_PKT_CNT, *(DPLUSRX_PKT_CNT + 1));
    seq_printf(seq, "  DPLUS_RDPTR        = %08X, %08X\n",
            *DPLUS_RDPTR, *(DPLUS_RDPTR + 1));

    seq_printf(seq, "  POSTSEARCH_RDPTR   = %08X, %08X\n", */

    seq_printf(seq, "\nQoS:\n");
    seq_printf(seq, "  DSLWAN_TX_SWAP_RDPTR = %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X\n %04X, %04X, %04X, %04X, %04X, %04X, %04X\n",
                                            * __DSLWAN_TXDES_SWAP_PTR(0), * __DSLWAN_TXDES_SWAP_PTR(1),
                                            * __DSLWAN_TXDES_SWAP_PTR(2), * __DSLWAN_TXDES_SWAP_PTR(3),
                                            * __DSLWAN_TXDES_SWAP_PTR(4), * __DSLWAN_TXDES_SWAP_PTR(5),
                                            * __DSLWAN_TXDES_SWAP_PTR(6), * __DSLWAN_TXDES_SWAP_PTR(7),
                                           * __DSLWAN_TXDES_SWAP_PTR(8), * __DSLWAN_TXDES_SWAP_PTR(9),
                                            * __DSLWAN_TXDES_SWAP_PTR(10), * __DSLWAN_TXDES_SWAP_PTR(11),
                                            * __DSLWAN_TXDES_SWAP_PTR(12), * __DSLWAN_TXDES_SWAP_PTR(13),
                                            * __DSLWAN_TXDES_SWAP_PTR(14));

    seq_printf(seq, "  DSLWAN_TX_PKT_CNT    = %04X, %04X, %04X, %04X, %04X, %04X, %04X, %04X\n %04X, %04X, %04X, %04X, %04X, %04X, %04X\n ",
                                            * __DSLWAN_TX_PKT_CNT(0), * __DSLWAN_TX_PKT_CNT(1),
                                            * __DSLWAN_TX_PKT_CNT(2), * __DSLWAN_TX_PKT_CNT(3),
                                            * __DSLWAN_TX_PKT_CNT(4), * __DSLWAN_TX_PKT_CNT(5),
                                            * __DSLWAN_TX_PKT_CNT(6), * __DSLWAN_TX_PKT_CNT(7),
                                            * __DSLWAN_TX_PKT_CNT(8), * __DSLWAN_TX_PKT_CNT(9),
                                            * __DSLWAN_TX_PKT_CNT(10), * __DSLWAN_TX_PKT_CNT(11),
                                            * __DSLWAN_TX_PKT_CNT(12), * __DSLWAN_TX_PKT_CNT(13),
                                            * __DSLWAN_TX_PKT_CNT(14));


    seq_printf(seq, "  ENETS PGNUM             = %d\n",  __ENETS_PGNUM);
    seq_printf(seq, "  ENETS DBUF BASE ADDR     = %08X\n",  (unsigned int)__ERX_DBUF_BASE);
    seq_printf(seq, "  ENETS CBUF BASE ADDR     = %08X\n",  (unsigned int)__ERX_CBUF_BASE);

    seq_printf(seq, "  QOSD_DPLUS_RDPTR     = %04X\n",
                                            * __DSLWAN_FP_RXDES_SWAP_RDPTR);

    seq_printf(seq, "  QOSD_CPUTX_RDPTR     = %04X\n",
                                            * __CPU_TXDES_SWAP_RDPTR);

    seq_printf(seq, "  DPLUS_RXDES_RDPTR    = %04X\n",
                                            * __DSLWAN_FP_RXDES_DPLUS_WRPTR);


    seq_printf(seq, "  pre_dplus_ptr    = %04X\n", *PRE_DPLUS_PTR);

    seq_printf(seq, "  pre_dplus_cnt    = %04X\n", (*DM_RXPKTCNT) & 0xff);

    seq_printf(seq, "  dplus_ptr        = %04X\n", *DPLUS_PTR);

    seq_printf(seq, "  dplus_cnt        = %04X\n", *DPLUS_CNT);

    seq_printf(seq, "  DPLUS_RX_ON      = %d\n",  *DPLUS_RX_ON);

    seq_printf(seq, "  ISR_IS           = %08X\n ",  *ISR_IS);

    seq_printf(seq, "\nQoS Mib:\n");
    seq_printf(seq, "  cputx_pkts:             %u\n",   *__CPU_TO_DSLWAN_TX_PKTS);
    seq_printf(seq, "  cputx_bytes:            %u\n",   *__CPU_TO_DSLWAN_TX_BYTES);
    seq_printf(seq, "  cputx_drop_pkts:        %u\n",   *__CPU_TX_SWAPPER_DROP_PKTS);
    seq_printf(seq, "  cputx_drop_bytess:      %u\n",   *__CPU_TX_SWAPPER_DROP_BYTES);

    seq_printf(seq, "  dslwan_fp_drop_pkts:    %u\n",   *__DSLWAN_FP_SWAPPER_DROP_PKTS );
    seq_printf(seq, "  dslwan_fp_drop_bytes:   %u\n",   *__DSLWAN_FP_SWAPPER_DROP_BYTES );

    seq_printf(seq, "  dslwan_tx_qf_drop_pkts:  (%u, %u, %u, %u, %u, %u, %u, %u ,%u, %u, %u, %u, %u, %u, %u)\n",
                 *__DSLWAN_TX_THRES_DROP_PKT_CNT(0),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(1),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(2) ,  *__DSLWAN_TX_THRES_DROP_PKT_CNT(3) ,
                 *__DSLWAN_TX_THRES_DROP_PKT_CNT(4),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(5),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(6) ,  *__DSLWAN_TX_THRES_DROP_PKT_CNT(7),
                 *__DSLWAN_TX_THRES_DROP_PKT_CNT(8),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(9),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(10),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(11),
                 *__DSLWAN_TX_THRES_DROP_PKT_CNT(12),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(13),  *__DSLWAN_TX_THRES_DROP_PKT_CNT(14));


    return 0;
}

static ssize_t proc_write_fw_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[2048];
    char *p;
    int len, rlen, i;

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
       for( i=0;i<15;i++)
               * __DSLWAN_TX_THRES_DROP_PKT_CNT(i) = 0;

        * __CPU_TO_DSLWAN_TX_PKTS  = 0;
        * __CPU_TO_DSLWAN_TX_BYTES = 0;


        * __CPU_TX_SWAPPER_DROP_PKTS    = 0;
        * __CPU_TX_SWAPPER_DROP_BYTES   = 0;
        * __DSLWAN_FP_SWAPPER_DROP_PKTS = 0;
        * __DSLWAN_FP_SWAPPER_DROP_PKTS = 0;
    }

    return count;
}

static ssize_t proc_write_fwdbg_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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

    return count;
}
#endif


static INLINE int read_port_counters(int port, struct seq_file *seq)
{
    unsigned int counter;
    unsigned long long  byteCnt;

    seq_printf(seq, "\n\tPort [%d] counters\n\n", port);

    seq_printf(seq, "Rx Total PckCnt     :");
    counter = sw_get_rmon_counter(port, RX_TOTAL_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Rx Unicast PckCnt   :");
    counter = sw_get_rmon_counter(port, RX_UNICAST_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Rx Multicast PckCnt :");
    counter = sw_get_rmon_counter(port, RX_MULTICAST_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Tx Total PckCnt     :");
    counter = sw_get_rmon_counter(port, TX_TOTAL_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Tx Unicase PckCnt   :");
    counter = sw_get_rmon_counter(port, TX_UNICAST_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Tx Multicase PckCnt :");
    counter = sw_get_rmon_counter(port, TX_MULTICAST_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Rx 64B PckCnt       :");
    counter = sw_get_rmon_counter(port, RX_SIZE_64B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Rx [65-127B] PckCnt :");
    counter = sw_get_rmon_counter(port, RX_SIZE_65_127B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Rx [128~255B] PckCnt:");
    counter = sw_get_rmon_counter(port, RX_SIZE_128_255B_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);
    seq_printf(seq, "Rx [256~511B] PckCnt:");
    counter = sw_get_rmon_counter(port, RX_SIZE_256_511B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Rx [512~1023B]PckCnt:");
    counter = sw_get_rmon_counter(port, RX_SIZE_512_1023B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Rx [ >1024B] PckCnt :");
    counter = sw_get_rmon_counter(port, RX_SIZE_1024B_MAX_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Tx [64B] PckCnt     :");
    counter = sw_get_rmon_counter(port, TX_SIZE_64B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Tx [65~127B] PckCnt :");
    counter = sw_get_rmon_counter(port, TX_SIZE_65_127B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Tx [128~255B] PckCnt:");
    counter = sw_get_rmon_counter(port, TX_SIZE_128_255B_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);
    seq_printf(seq, "Tx [256~511B] PckCnt:");
    counter = sw_get_rmon_counter(port, TX_SIZE_256_511B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Tx [512~1023B]PckCnt:");
    counter = sw_get_rmon_counter(port, TX_SIZE_512_1023B_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);
    seq_printf(seq, "Tx [>1024B] PckCnt  :");
    counter = sw_get_rmon_counter(port, TX_SIZE_1024B_MAX_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Rx CRC err PckCnt   :");
    counter = sw_get_rmon_counter(port, RX_CRC_ERR_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Rx Unsize err PCkCnt:");
    counter = sw_get_rmon_counter(port, RX_UNDERSIZE_ERR_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Rx Ovsize err PckCnt:");
    counter = sw_get_rmon_counter(port, RX_OVER_SIZE_ERR_PCKNT);
    seq_printf(seq, "0x%08x\n",counter );

    seq_printf(seq, "Rx UnsizeGood PckCnt:");
    counter = sw_get_rmon_counter(port, RX_UNDERSIZE_GOOD_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Rx OvsizeGood PckCnt:");
    counter = sw_get_rmon_counter(port, RX_OVER_SIZE_GOOD_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Rx Good Pause PckCnt:");
    counter = sw_get_rmon_counter(port, RX_GOOD_PAUSE_PCKNT);
    seq_printf(seq, "0x%08x\n",counter );

    seq_printf(seq, "Rx Align err PckCnt :");
    counter = sw_get_rmon_counter(port, RX_ALLIGN_ERR_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Rx filterd errPckCnt:");
    counter = sw_get_rmon_counter(port, RX_FILTERED_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Tx Single col Cnt   :");
    counter = sw_get_rmon_counter(port, TX_SINGLE_COLLISION_CNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Tx Multiple col Cnt :");
    counter = sw_get_rmon_counter(port, TX_MULTIPLE_COLLISION_CNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Tx Late col  Cnt    :");
    counter = sw_get_rmon_counter(port,TX_LATE_COLLISION_CNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Tx Excessive col Cnt:");
    counter = sw_get_rmon_counter(port, TX_EXCESSIVE_COLLISION_CNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Tx  Pause Cnt       :");
    counter = sw_get_rmon_counter(port, TX_PAUSE_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);


    seq_printf(seq, "Tx Drop PckCnt      :");
    counter =sw_get_rmon_counter(port, TX_DROP_PCKNT);
    seq_printf(seq, "0x%08x\t", counter);

    seq_printf(seq, "Rx Drop PckCnt      :");
    counter = sw_get_rmon_counter(port, RX_DROP_PCKNT);
    seq_printf(seq, "0x%08x\n", counter);

    seq_printf(seq, "Rx Good Byte Cnt    :");
    byteCnt = sw_get_rmon_counter(port, RX_GOOD_BYTE_CNT_HIGH);
    byteCnt <<= 32;
    byteCnt += sw_get_rmon_counter(port, RX_GOOD_BYTE_CNT_LOW);
    seq_printf(seq, "0x%llx \t", byteCnt);

    seq_printf(seq, "Tx Good Byte Cnt    :");
    byteCnt = sw_get_rmon_counter(port, TX_GOOD_BYTE_CNT_HIGH);
    byteCnt <<= 32;
    byteCnt += sw_get_rmon_counter(port, TX_GOOD_BYTE_CNT_LOW);
    seq_printf(seq, "0x%llx \n", byteCnt);

    seq_printf(seq, "Rx Bad Byte Cnt     :");
    byteCnt = sw_get_rmon_counter(port, RX_BAD_BYTE_CNT_HIGH);
    byteCnt <<= 32;
    byteCnt += sw_get_rmon_counter(port, RX_BAD_BYTE_CNT_LOW);
    seq_printf(seq, "0x%llx \t", byteCnt);

    seq_printf(seq, "Rx Discard Cong Cnt :");
    byteCnt = sw_get_rmon_counter(port, RX_DISCARD_CONGESTION_PCKNT);
    seq_printf(seq, "0x%llx \n", byteCnt);

    return 0;
}


static int port0_counters_proc_read(struct seq_file *seq, void *v)
{

    read_port_counters(0, seq);

    return 0;
}

static int port1_counters_proc_read(struct seq_file *seq, void *v)
{

    read_port_counters(1, seq);

    return 0;
}

static int port2_counters_proc_read(struct seq_file *seq, void *v)
{
    read_port_counters(2, seq);

    return 0;
}

static int port3_counters_proc_read(struct seq_file *seq, void *v)
{
    read_port_counters(3, seq);

    return 0;
}

static int port4_counters_proc_read(struct seq_file *seq, void *v)
{

    read_port_counters(4, seq);

    return 0;
}

static int port5_counters_proc_read(struct seq_file *seq, void *v)
{

    read_port_counters(5, seq);

    return 0;
}

static int port6_counters_proc_read(struct seq_file *seq, void *v)
{

    read_port_counters(6, seq);

    return 0;
}

static ssize_t port_counters_proc_write_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int port;

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

    if ( stricmp(p, "clean") == 0 || stricmp(p, "clear") == 0 )
    {
        strcpy(str, file->f_dentry->d_iname);
        port = str[4] - '0';
        str[4] = ' ';
        if ( stricmp(str, "port mib") == 0 && port >= 0 && port < 7 )
            sw_clr_rmon_counter(port);
        else
            printk("unknown entry - %s\n", file->f_dentry->d_iname);
    }
    else if ( stricmp(p, "clean all") == 0 || stricmp(p, "clear all") == 0 )
    {
        for ( port = 0; port < 7; port++ )
            sw_clr_rmon_counter(port);
    }

    return count;
}


static int proc_read_directforwarding(struct seq_file *seq, void *v)
{

    seq_printf(seq, "ingress direct forwarding - %s\n", g_ingress_direct_forward_enable ? "enable" : "disable");
    seq_printf(seq, "egress direct forwarding  - %s\n", g_egress_direct_forward_enable ? "enable" : "disable");

    return 0;
}

static ssize_t proc_write_directforwarding_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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


static int proc_read_clk(struct seq_file *seq, void *v)
{
    unsigned int clk;
    int valid = 0;

    switch ( (*AMAZON_S_CGU_SYS >> 16) & 0x07 )
    {
    case 1:  clk = 250; valid = 1; break;
    case 4:  clk = 400; valid = 1; break;
    default: clk = 0;
    }


    if ( valid )
        seq_printf(seq, "PPE clock - %uM\n", clk);
    else
        seq_printf(seq, "PPE clock - %uM (unknown option %u)\n", clk, (*AMAZON_S_CGU_SYS >> 16) & 0x07);


    return 0;
}

static ssize_t proc_write_clk_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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

    if ( stricmp(p, "400M") == 0 || stricmp(p, "400") == 0 )
    {
        *AMAZON_S_CGU_SYS = (*AMAZON_S_CGU_SYS & ~(0x07 << 16)) | (0x04 << 16);
    }
    else if ( stricmp(p, "250M") == 0 || stricmp(p, "250") == 0 )
    {
        *AMAZON_S_CGU_SYS = (*AMAZON_S_CGU_SYS & ~(0x07 << 16)) | (0x01 << 16);
    }
    else
    {
        printk("echo <400/250M> > /proc/eth/clk\n");
        printk("  400M - PPE clock 400MHz\n");
        printk("  250M - PPE clock 250MHz\n");
    }

    return count;
}


static int proc_read_prio(struct seq_file *seq, void *v)
{
    int len = 0;
    int i, j;

    //  skb->priority to firmware queue map (for LAN interface, QId is virtual one maitained by driver)
    seq_printf(seq,        "Priority to Queue Map:\n");
    seq_printf(seq,        "  prio\t\t:  0  1  2  3  4  5  6  7\n");
    for ( i = 0; i < 2; i++ )
    {
        seq_printf(seq,    "  eth%d\t\t:", i);
        for ( j = 0; j < NUM_ENTITY(g_eth_prio_queue_map[i]); j++ )
            seq_printf(seq," %2d", g_eth_prio_queue_map[i][j]);
        seq_printf(seq,    "\n");
    }
    if ( g_eth_wan_mode == 0 )
    {
        for ( i = 0; i < NUM_ENTITY(g_ptm_net_dev); i++ )
        {
            seq_printf(seq,"  ptm%d\t\t:", i);
            for ( j = 0; j < NUM_ENTITY(g_ptm_prio_queue_map[i]); j++ )
                seq_printf(seq," %2d", g_ptm_prio_queue_map[i][j]);
            seq_printf(seq,"\n");
        }
    }
    //  firmware queue to switch class map (this is shared by both LAN/WAN interfaces)
    seq_printf(seq,        "Queue to Class Map:\n");
    seq_printf(seq,        "  queue    :  0  1  2  3  4  5  6  7\n");
    seq_printf(seq,        "  eth0/1   :");
    for ( j = 0; j < NUM_ENTITY(g_wan_queue_class_map); j++ )
        seq_printf(seq,    " %2d", g_wan_queue_class_map[j]);
    seq_printf(seq,        "\n");
    //  skb->priority to switch class map (this is calculated based on aboved two tables)
    seq_printf(seq,        "Priority to Class Map (derived from two tables above):\n");
    seq_printf(seq,        "  prio     :  0  1  2  3  4  5  6  7\n");
    for ( i = 0; i < 2; i++ )
    {
        seq_printf(seq,    "  eth%d     :", i);
        for ( j = 0; j < NUM_ENTITY(g_eth_prio_queue_map[i]); j++ )
            seq_printf(seq," %2d", g_wan_queue_class_map[g_eth_prio_queue_map[i][j]]);
        seq_printf(seq,    "\n");
    }

    return 0;
}

static ssize_t proc_write_prio_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 1;
    unsigned int port = ~0;
    int conn = -1;
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
            printk("echo <eth0/eth1/ptm0> prio xx queue xx [prio xx queue xx] > /proc/eth/prio\n");
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
        else if ( stricmp(p1, "ptm0") == 0 )
        {
            if ( g_eth_wan_mode == 0 )
            {
                port = 7;
                prio = queue = -1;
                dbg("port = 7");
            }
        }
        else if ( port != ~0 || conn >= 0 )
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
                else if ( port == 7 && prio < NUM_ENTITY(g_ptm_prio_queue_map[0]) )
                {
                    if ( queue >= 0 )
                        g_ptm_prio_queue_map[port][prio] = queue;
                }
                else
                {
                    err("prio (%d) is out of range 0 - %d", prio, conn >= 0 ? 7 : (port == 7 ? NUM_ENTITY(g_ptm_prio_queue_map[0]) - 1 : NUM_ENTITY(g_eth_prio_queue_map[port]) - 1));
                }
            }
            else if ( stricmp(p1, "q") == 0 || stricmp(p1, "queue") == 0 )
            {
                ignore_space(&p2, &len);
                queue = get_number(&p2, &len, 0);
                dbg("queue = %d", queue);
                if ( port >= 0 && port <= 1 )
                {
                    if ( queue >= 0 && queue < ((g_wan_itf & (1 << port)) && g_wanqos_en ? __WAN_TX_QUEUE_NUM : __VIRTUAL_TX_QUEUE_NUM) )
                    {
                        if ( prio >= 0 )
                            g_eth_prio_queue_map[port][prio] = queue;
                    }
                    else
                    {
                        err("queue (%d) is out of range 0 - %d", queue, (g_wan_itf & (1 << port)) && g_wanqos_en ? __WAN_TX_QUEUE_NUM - 1 : __VIRTUAL_TX_QUEUE_NUM - 1);
                    }
                }
                else if ( port == 7 )
                {
                    if ( queue >= 0 && queue < g_wanqos_en )
                    {
                        if ( prio >= 0 )
                            g_ptm_prio_queue_map[0][prio] = queue;
                    }
                    else
                    {
                        err("queue (%d) is out of range 0 - %d", queue, g_wanqos_en - 1);
                    }
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


static int proc_read_ewan(struct seq_file *seq, void *v)
{
    int i, j;
    unsigned int bit;

    g_pmac_ewan = *PMAC_EWAN_REG;

    seq_printf(seq, "EWAN: ");
    for ( i = j = 0, bit = 1; i < 7; i++, bit <<= 1 )
        if ( (g_pmac_ewan & bit) ) {
            seq_printf(seq, (const char *)(j ? " | P%d" : "P%d"), i);
            j++;
        }
    seq_printf(seq, (const char *)(j ? "\n" : "null\n"));

    return 0;
}


static ssize_t proc_write_ewan_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char local_buf[2048];
    int len;
    char *p1, *p2;
    int colon = 1;

    unsigned int got_ports = 0;
    unsigned int ports = 0;
    char *ports_name[] = {"p0", "p1", "p2", "p3", "p4", "p5", "null"};
    int i;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( stricmp(p1, "help") == 0 )
        {
            printk("echo <p0|p1|p2|p3|p4|p5> > /proc/eth/ewan\n");
            break;
        }
        else
        {
            for ( i = 0; i < NUM_ENTITY(ports_name); i++ )
                if ( stricmp(p1, ports_name[i]) == 0 )
                {
                    if ( i == 6 )
                    {
                        *PMAC_EWAN_REG = g_pmac_ewan = 0;
                        return count;
                    }

                    ports |= 1 << i;
                    got_ports++;
                    break;
                }
        }

        p1 = p2;
        colon = 1;
    }

    if ( got_ports )
        *PMAC_EWAN_REG = g_pmac_ewan = ports;

    return count;
}


#ifdef CONFIG_LTQ_PPA_QOS
static int proc_read_qos(struct seq_file *seq, void *v)
{
    int i;
    struct wtx_eg_q_shaping_cfg qos_cfg;
    struct wtx_qos_q_mib_table qos_queue_mib;
    volatile struct tx_qos_cfg tx_qos_cfg = *TX_QOS_CFG;
    volatile struct wtx_qos_q_desc_cfg qos_q_desc_cfg;
    unsigned int portid;


    seq_printf(seq, "\n  qos         : %s\n  wfq         : %s\n  Rate shaping: %s\n\n",
                    tx_qos_cfg.eth1_qss ?"enabled":"disabled",
                    tx_qos_cfg.wfq_en?"enabled":"disabled",
                    tx_qos_cfg.shape_en ?"enabled":"disabled");

    seq_printf(seq, "  Ticks  =%u,    overhd    =%u,       qnum=%u  @%p\n", (u32)tx_qos_cfg.time_tick, (u32)tx_qos_cfg.overhd_bytes, (u32)tx_qos_cfg.eth1_eg_qnum, TX_QOS_CFG );


#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
    seq_printf(seq, "  PPE clk=%u MHz, basic tick=%u\n", (u32)cgu_get_pp32_clock() / 1000000, TX_QOS_CFG->time_tick / (cgu_get_pp32_clock() / 1000000));
#else
    struct clk * clk ;
    clk = clk_get_ppe();
    seq_printf(seq, "  PPE clk=%u MHz, basic tick=%u\n", (clk_get_rate(clk) / 1000000), TX_QOS_CFG->time_tick / (clk_get_rate(clk) / 1000000));
#endif

    if ( tx_qos_cfg.eth1_eg_qnum )
    {
        if ( ppa_drv_hal_generic_hook != NULL )
        {
            PPE_QOS_RATE_SHAPING_CFG rate_cfg = {0};
            PPE_QOS_WFQ_CFG wfq_cfg = {0};

            if ( g_eth_wan_mode == 0 )
                portid = PPA_WAN_QOS_PTM0;
            else if ( (g_wan_itf & 0x02) )
                portid = PPA_WAN_QOS_ETH1;
            else if ( (g_wan_itf & 0x01) )
                portid = PPA_WAN_QOS_ETH0;
            else
                portid = ~0;

            seq_printf(seq, "\n  Cfg :    T     R     S -->  Bit-rate(kbps)      Weight --> Level       Address       d/w      tick_cnt   b/S\n");
            for ( i = 0; i < 8; i++ )
            {
  #ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
                rate_cfg.portid  = portid;
                rate_cfg.queueid = i;
                ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG, &rate_cfg, 0);
  #endif
  #ifdef CONFIG_LTQ_PPA_QOS_WFQ
                wfq_cfg.portid  = portid;
                wfq_cfg.queueid = i;
                ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_WFQ_CFG, &wfq_cfg, 0);
  #endif
                qos_cfg = *WTX_EG_Q_SHAPING_CFG(i);

                seq_printf(seq, "\n      %2u:  %03u  %05u  %05u   %07u            %08u   %03u        @0x%p   %08u    %03u     %05u\n", i, qos_cfg.t, qos_cfg.r, qos_cfg.s, rate_cfg.rate_in_kbps, qos_cfg.w, wfq_cfg.weight_level, WTX_EG_Q_SHAPING_CFG(i), qos_cfg.d, qos_cfg.tick_cnt, qos_cfg.b);
            }

            if ( portid < 2 )
            {
                //  PTM WAN mode does not have port based rate shaping, only ETH WAN mode has.

  #ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
                rate_cfg.portid  = portid;
                rate_cfg.queueid = i;
                ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG, &rate_cfg, 0);
  #endif
                qos_cfg = *WTX_EG_Q_PORT_SHAPING_CFG(0);

                seq_printf(seq, "\n    port:  %03u  %05u  %05u   %07u                                  @0x%p   %08u    %03u     %05u\n", qos_cfg.t, qos_cfg.r, qos_cfg.s, rate_cfg.rate_in_kbps, WTX_EG_Q_PORT_SHAPING_CFG(0), qos_cfg.d, qos_cfg.tick_cnt, qos_cfg.b);
            }
            else if(portid == PPA_WAN_QOS_PTM0){
                for(i = 0; i < MAX_PTM_QOS_PORT_NUM; i ++){
  #ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
                rate_cfg.portid  = portid;
                rate_cfg.queueid = ~i;
                ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG, &rate_cfg, 0);
  #endif
                qos_cfg = *WTX_PTM_EG_Q_PORT_SHAPING_CFG(i);

                seq_printf(seq, "\n    p[%d]:  %03u  %05u  %05u   %07u                             @0x%p   %08u   %03u    %05u\n",
                                             i, qos_cfg.t, qos_cfg.r, qos_cfg.s, rate_cfg.rate_in_kbps, WTX_PTM_EG_Q_PORT_SHAPING_CFG(i), qos_cfg.d, qos_cfg.tick_cnt, qos_cfg.b);
                }
            }
        }

        seq_printf(seq, "\n  MIB : rx_pkt/rx_bytes         tx_pkt/tx_bytes       cpu_small_drop/cpu_drop  fast_small_drop/fast_drop_cnt\n");
        for ( i = 0; i < 8; i++ )
        {
            qos_queue_mib = *WTX_QOS_Q_MIB_TABLE(i);

            seq_printf(seq, "    %2u: %010u/%010u  %010u/%010u  %010u/%010u  %010u/%010u  @0x%p\n", i,
                qos_queue_mib.wrx_total_pdu, qos_queue_mib.wrx_total_bytes,
                qos_queue_mib.wtx_total_pdu, qos_queue_mib.wtx_total_bytes,
                qos_queue_mib.wtx_cpu_drop_small_pdu, qos_queue_mib.wtx_cpu_drop_pdu,
                qos_queue_mib.wtx_fast_drop_small_pdu, qos_queue_mib.wtx_fast_drop_pdu,
                WTX_QOS_Q_MIB_TABLE(i));

        }

        //QOS queue descriptor
        seq_printf(seq, "\n  Desc: threshold  num    base_addr  rd_ptr   wr_ptr\n");
        for ( i = 0; i < 8; i++ )
        {
            qos_q_desc_cfg = *WTX_QOS_Q_DESC_CFG(i);

            seq_printf(seq, "    %2u: 0x%02x       0x%02x   0x%04x     0x%04x   0x%04x  @0x%p\n", i,
                qos_q_desc_cfg.threshold,
                qos_q_desc_cfg.length,
                qos_q_desc_cfg.addr,
                qos_q_desc_cfg.rd_ptr,
                qos_q_desc_cfg.wr_ptr,
                WTX_QOS_Q_DESC_CFG(i) );

        }
    }

    return 0;

}
#endif


#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
static int proc_read_mirror(struct seq_file *seq, void *v)
{
    char devname[IFNAMSIZ + 1] = {0};

    if ( g_mirror_netdev == NULL )
        strcpy(devname, "NONE");
    else
        strncpy(devname, g_mirror_netdev->name, IFNAMSIZ);

    seq_printf(seq, "mirror: %s\n", devname);

    return 0;
}

static ssize_t proc_write_mirror_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
        return count;

    if ( g_mirror_netdev != NULL )
    {
        dev_put(g_mirror_netdev);
        g_mirror_netdev = NULL;
    }
    if ( stricmp(p, "none") == 0 )
        printk("disable mirror\n");
    else
    {
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
        g_mirror_netdev = dev_get_by_name(p);
  #else
        g_mirror_netdev = dev_get_by_name(&init_net, p);
  #endif
        if ( g_mirror_netdev != NULL )
            printk("mirror: %s\n", p);
        else
            printk("mirror: none, can't find device \"%s\"\n", p);
    }

    return count;
}
#endif


static int proc_read_class(struct seq_file *seq, void *v)
{
    int len = 0;

    seq_printf(seq, "Switch Class %s\n", PS_MC_GENCFG3->class_en ?  "Enable" : "Disable");
    seq_printf(seq, "CFG_CLASS2QID_MAP (0x%08x): %08x %08x %08x %08x\n", (unsigned int)CFG_CLASS2QID_MAP(0), *CFG_CLASS2QID_MAP(0), *CFG_CLASS2QID_MAP(1), *CFG_CLASS2QID_MAP(2), *CFG_CLASS2QID_MAP(3));
    seq_printf(seq, "CFG_QID2CLASS_MAP (0x%08x): %08x %08x %08x %08x\n", (unsigned int)CFG_QID2CLASS_MAP(0), *CFG_QID2CLASS_MAP(0), *CFG_QID2CLASS_MAP(1), *CFG_QID2CLASS_MAP(2), *CFG_QID2CLASS_MAP(3));


    return len;
}

static ssize_t proc_write_class_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
        else if ( stricmp(p1, "Enable") == 0 || stricmp(p1, "Disable") == 0){
            PS_MC_GENCFG3->class_en = stricmp(p1, "Enable") == 0 ? 1 : 0;
            break;
        }
        else
        {
            printk("echo <CLASS2QID | QID2CLASS> [hex numbers] > /proc/eth/class\n");
            printk("echo <Enable | Disable > /proc/eth/class\n");
            break;
        }

        p1 = p2;
        colon = 1;
    }

    return count;
}


static int proc_read_mccfg(struct seq_file *seq, void *v)
{

    seq_printf(seq, "SSC: %s, ASC: %s \n",
                  PS_MC_GENCFG3->ssc_mode ? "Enable" : "Disable",
                  PS_MC_GENCFG3->asc_mode ? "Enable" : "Disable");

    return 0;
}

static ssize_t proc_write_mccfg_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
        "PTM0",
        NULL
    };

    int wan_itf = *CFG_WAN_PORTMAP;
    int wan_mixmap = *CFG_MIXED_PORTMAP;
    int i, len = 0;

    int max_exts_idx = 4 + 2;

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
    static const char *dest_list[] = {"ETH0", "ETH1", "CPU0", "EXT_INT1", "EXT_INT2", "EXT_INT3", "EXT_INT4", "PTM"};

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
    if(f_is_lan){
        len += sprintf(buf + len,      "      encap tunnel: %s\n", pwaction->encap_tunnel ? "yes":"no");
        len += sprintf(buf + len,      "      tunnel iphdr idx: %d\n", pwaction->tnnl_hdr_idx);
    }else{
        len += sprintf(buf + len,      "      decap tunnel: %s\n", pwaction->encap_tunnel ? "yes":"no");
    }
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
static INLINE int print_mc(char *buf, int i, struct wan_rout_multicast_cmp_tbl *pcompare, struct wan_rout_multicast_act_tbl *paction, struct rtp_sampling_cnt *rtp_mib,uint32_t mc_mib_addr, uint32_t rt_mc_cnt)
#else
static INLINE int print_mc(char *buf, int i, struct wan_rout_multicast_cmp_tbl *pcompare, struct wan_rout_multicast_act_tbl *paction, uint32_t mc_mib_addr, uint32_t rt_mc_cnt)
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
    len += sprintf(buf + len,          "    MIB: (0x%08x)\n", mc_mib_addr);
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
static INLINE void dump_skb(struct sk_buff *skb, u32 len, char *title, int port, int ch, int is_tx, int is_force)
{
    int i;

    if ( !g_dump_cnt || (!is_force && !(g_dbg_enable & (is_tx ? DBG_ENABLE_MASK_DUMP_SKB_TX : DBG_ENABLE_MASK_DUMP_SKB_RX))) )
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
static INLINE void dump_flag_header(int fwcode, struct flag_header *header, const char *title, int ch)
{
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
    case FWCODE_ROUTING_ACC_A5:
        {
            printk("  ipv4_mc_pkt    = %Xh (%s)\n", (u32)header->ipv4_mc_vld, header->ipv4_mc_vld ? "multicast" : "uni-cast");
            printk("  proc_type      = %Xh (%s)\n", (u32)header->proc_type, header->proc_type ? "bridging" : "routing");
            printk("  tcpudp_err     = %Xh\n", (u32)header->tcpudp_err);
            printk("  tcpudp_chk     = %Xh\n", (u32)header->tcpudp_chk);
            printk("  is_udp         = %Xh\n", (u32)header->is_udp);
            printk("  is_tcp         = %Xh\n", (u32)header->is_tcp);
            printk("  ip_inner_offset= %Xh\n", (u32)header->ip_inner_offset);
            printk("  is_pppoes      = %Xh\n", (u32)header->is_pppoes);
            printk("  is_inner_ipv6  = %Xh\n", (u32)header->is_inner_ipv6);
            printk("  is_inner_ipv4  = %Xh\n", (u32)header->is_inner_ipv4);
            printk("  is_vlan        = %Xh (%s)\n", (u32)header->is_vlan, is_vlan_str[header->is_vlan]);
            printk("  rout_index     = %Xh\n", (u32)header->rout_index);

            printk("  dest_list      = %Xh\n", (u32)header->dest_list);
            printk("  src_itf        = %Xh\n", (u32)header->src_itf);
            printk("  tcp_rstfin     = %Xh\n", (u32)header->tcp_rstfin);
            printk("  qid            = %Xh\n", (u32)header->qid);
            printk("  temp_dest_list = %Xh\n", (u32)header->temp_dest_list);
            printk("  src_dir        = %Xh (from %s side)\n", (u32)header->src_dir, header->src_dir ? "WAN" : "LAN");
            printk("  acc_done       = %Xh\n", (u32)header->acc_done);
            printk("  is_outer_ipv6  = %Xh\n", (u32)header->is_outer_ipv6);
            printk("  is_outer_ipv4  = %Xh\n", (u32)header->is_outer_ipv4);
            printk("  is_tunnelled   = %Xh\n", (u32)header->is_tunnel);

            printk("  switch portid  = %Xh\n", (u32)header->sppid);
            printk("  packet length  = %Xh\n", (u32)header->pkt_len);
            printk("  pl_byteoff     = %Xh\n", (u32)header->pl_byteoff);
            printk("  mpoa type      = %Xh\n", (u32)header->mpoa_type);
            printk("  ip_outer_offset= %Xh\n", (u32)header->ip_outer_offset);

            printk("  traffic class  = %Xh\n", (u32)header->tc);
        }
        break;
    }
}
#endif

#if defined(DEBUG_DUMP_INIT) && DEBUG_DUMP_INIT
static INLINE void dump_init(int before_fw)
{
    if ( !(g_dbg_enable & DBG_ENABLE_MASK_DUMP_INIT) )
        return;

    printk("+++ %s firmware running +++\n", before_fw ? "before" : "after");
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

    printk("SFSM/FFSM:\n");
    printk("  SFSM_DBA    = 0x%08X, 0x%08X\n", *SFSM_DBA0, *SFSM_DBA1);
    printk("  SFSM_CBA    = 0x%08X, 0x%08X\n", *SFSM_CBA0, *SFSM_CBA1);
    printk("  SFSM_CFG    = 0x%08X, 0x%08X\n", *SFSM_CFG0, *SFSM_CFG1);
    printk("  FFSM_DBA    = 0x%08X, 0x%08X\n", *FFSM_DBA0, *FFSM_DBA1);
    printk("  FFSM_CFG    = 0x%08X, 0x%08X\n", *FFSM_CFG0, *FFSM_CFG1);
    printk("  FFSM_IDLE_HEAD_BC = 0x%08X, 0x%08X\n", *FFSM_IDLE_HEAD_BC0, *FFSM_IDLE_HEAD_BC1);

    printk("Communication:\n");
    printk("  FW_VER_ID(%08X)  = 0x%08X\n", (u32)FW_VER_ID, *(u32*)FW_VER_ID);
}
#endif

#ifdef CONFIG_LTQ_PORT_MIRROR
static void mirror_pkt_eth(struct sk_buff *skb, int offset)
{
    struct sk_buff *new_skb = skb_copy(skb, GFP_ATOMIC);
    struct ethhdr *eth;
    int i;
    eth = (struct ethhdr *)new_skb->data;
    
    unsigned char MAC_SRC[ETH_ALEN] = {0};
    for (i=0; i<6; i++)
    {
	    MAC_SRC[i]=eth->h_source[i];
    }
	
    if ((new_skb != NULL) && (memcmp(MAC_SRC, "\0\0\0\0\0\0", ETH_ALEN) != NULL))
    {
        if ( offset > 0 )
	{
            skb_pull(new_skb, offset);
		memmove(new_skb->data,new_skb->data-4,12);
		new_skb->mac_header+=4;
	}
    	eth = (struct ethhdr *)new_skb->data;
        new_skb->dev = ifx_get_mirror_netdev();
        memset(eth->h_source, 0x00, ETH_ALEN);
    
	//todo-
	//for mirrored DS packets, destnation macaddress has to be changed to MAC of mirrored destination
	//With out this, packets will not go out mirrored interface.
	//Set MSB bit on destination mac
	eth->h_dest[0]=eth->h_dest[0] | 0x80;

        if ( ifx_is_device_type_wireless() )
        {
            for ( i = 0; i < 6; i++ )
                eth->h_dest[i] = 0xFF;
        }
        dev_queue_xmit(new_skb);
    }
   else
   {
	//free skb for all traffic not matching src mac 0
        dev_kfree_skb_any(new_skb);
   }

}
#endif

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
/*
  * by default, we use straight priority queue
  * flag:    
  *       0 -- Get next skb
  *       1 -- Remove the skb from directpath[qid]'s txq (remove from head)
  * return current queue NO. if flag means get one skb
*/
static int get_next_txq_skb(struct sk_buff **skb, int qid, int flag)
{
    int idx = -1;
    int i;
    struct sk_buff *tx_skb;

    if(flag == 0){//Get next skb
        if(skb == NULL){
            return -1;
        }

        for(i = 0; i < NUM_ENTITY(g_ppe_directpath_data); i ++){
            spin_lock(&g_ppe_directpath_data[i].txq_lock);
            if(g_ppe_directpath_data[i].skb_list != NULL){
                skb_get(g_ppe_directpath_data[i].skb_list);
                idx  = i;
                *skb = g_ppe_directpath_data[i].skb_list;
                spin_unlock(&g_ppe_directpath_data[i].txq_lock);
                break;
            }
            spin_unlock(&g_ppe_directpath_data[i].txq_lock);
        }
        
        return idx;
    }
    else if(flag == 1){ //release the skb indicated by qid
        if(qid < 0 || qid >= NUM_ENTITY(g_ppe_directpath_data)){
            return -1;
        }

        spin_lock(&g_ppe_directpath_data[qid].txq_lock);
        tx_skb = g_ppe_directpath_data[qid].skb_list;
        if(tx_skb == NULL){
            //should not be NULL unless the device is unregistered.
            (*skb)->prev = (*skb)->next = NULL; 
        }
        else if(tx_skb->prev == tx_skb){//last one
            tx_skb->prev = tx_skb->next = g_ppe_directpath_data[qid].skb_list = NULL;
            //try to pull from directpath skb_list
  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
            g_ppe_directpath_data[qid].skb_list_size -= tx_skb->len;
            ASSERT(g_ppe_directpath_data[qid].skb_list_size == 0, "Error:%s: size should be zero\n", __FUNCTION__);
  #else
            g_ppe_directpath_data[qid].skb_list_len    -= 1;
            ASSERT(g_ppe_directpath_data[qid].skb_list_len == 0, "Error:%s: len should be zero\n", __FUNCTION__);
  #endif

        }
        else{
            g_ppe_directpath_data[qid].skb_list = tx_skb->next;
            tx_skb->next->prev = tx_skb->prev;
            tx_skb->prev->next = tx_skb->next;
            tx_skb->prev       = tx_skb->next  = NULL;

            //try to pull from directpath skb_list
  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
            g_ppe_directpath_data[qid].skb_list_size -= tx_skb->len;
  #else
            g_ppe_directpath_data[qid].skb_list_len  -= 1;
  #endif
        }

        spin_unlock(&g_ppe_directpath_data[qid].txq_lock);
        return qid;
    }
    
    return -1; 
}

static void directpath_task_send(unsigned long unused)
{
    struct sk_buff *skb;
    int tx_q_num = 0;
    int len;
    int qid = -1;
    uint32_t stat;

    for( tx_q_num = 0; tx_q_num < g_max_txq_num || !g_directpath_dma_full; tx_q_num ++){

        //switch metering control
        stat = *PCE_TCM_STAT(0) & 0x3;
        if(stat){
            qid = 1; //just for restart the tasklet
            *PCE_TCM_STAT(0) = stat;
            break;
        }

        skb = NULL;
        qid = get_next_txq_skb(&skb, -1, 0); 

        if(qid < 0 || skb == NULL){
            break;
        }

        len = skb->len;
        if(len < ETH_MIN_TX_PACKET_LENGTH + sizeof(struct sw_eg_pkt_header)){
            len = ETH_MIN_TX_PACKET_LENGTH + sizeof(struct sw_eg_pkt_header);
			skb_padto(skb,len);
        }

        put_skb_to_dbg_pool(skb);
        if(dma_device_write(g_dma_device, skb->data, len, skb) == len){//send successfully
            get_next_txq_skb(&skb, qid, 1); //remove from the list
            dev_kfree_skb_any(skb);
        }else{
            get_skb_from_dbg_pool(skb);
            dev_kfree_skb_any(skb); //decrease skb the reference as DMA don't free it if failed.
            break;                  //DMA fail, try next time
        }
                
    }

    if(qid >= 0){
        tasklet_schedule(&g_directpath_task);
    }

    return;
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
            if ( len < ETH_MIN_TX_PACKET_LENGTH + sizeof(struct sw_eg_pkt_header) ){
                len = ETH_MIN_TX_PACKET_LENGTH + sizeof(struct sw_eg_pkt_header);
                skb_padto(skb,len);
            }

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

static INLINE void register_netdev_event_handler(void)
{
#if !defined(DISABLE_LOCAL_MAC_ADD_TO_SW) || !DISABLE_LOCAL_MAC_ADD_TO_SW
    g_netdev_event_handler_nb.notifier_call = netdev_event_handler;
    register_netdevice_notifier(&g_netdev_event_handler_nb);
#endif
}

static INLINE void unregister_netdev_event_handler(void)
{
#if !defined(DISABLE_LOCAL_MAC_ADD_TO_SW) || !DISABLE_LOCAL_MAC_ADD_TO_SW
    unregister_netdevice_notifier(&g_netdev_event_handler_nb);
#endif
}

#if !defined(DISABLE_LOCAL_MAC_ADD_TO_SW) || !DISABLE_LOCAL_MAC_ADD_TO_SW
  #ifdef CONFIG_LTQ_ETHSW_API
static int netdev_event_handler(struct notifier_block *nb, unsigned long event, void *netdev)
{
    struct net_device *netif = (struct net_device *)netdev;
    LTQ_ETHSW_API_HANDLE handle;
    union ifx_sw_param x;

    if ( netif->type != ARPHRD_ETHER )
        return NOTIFY_DONE;

    if ( (netif->flags & IFF_POINTOPOINT) ) //  ppp interface
        return NOTIFY_DONE;

    switch ( event )
    {
    case NETDEV_UP:
        handle = ltq_ethsw_api_kopen("/dev/switch_api/0");
        memset(&x.MAC_tableAdd, 0x00, sizeof(x.MAC_tableAdd));
        x.MAC_tableAdd.nFId = 0;
        x.MAC_tableAdd.nPortId = 6; //  CPU port
        //if ( netif->name[0] == 'p' && netif->name[1] == 't' && netif->name[2] == 'm' )  //  ptm
        //    x.MAC_tableAdd.nPortId = 11;
        //else if ( netif->name[0] == 'n' && netif->name[1] == 'a' && netif->name[2] == 's' ) //  atm
        //    x.MAC_tableAdd.nPortId = 11;
        //else if ( netif->name[0] == 'e' && netif->name[1] == 't' && netif->name[2] == 'h' && netif->name[3] == '1' )    //  eth1
        //    x.MAC_tableAdd.nPortId = ;
        //else
        //{
        //    int i;
        //
        //    for ( i = 0; i < NUM_ENTITY(g_ppe_directpath_data); i++ )
        //    {
        //        if ( netif == g_ppe_directpath_data[i].netif )
        //            x.MAC_tableAdd.nPortId = 7 + i;
        //    }
        //}
        x.MAC_tableAdd.nAgeTimer = 3;
        x.MAC_tableAdd.bStaticEntry = 1;
        x.MAC_tableAdd.nMAC[0] = netif->dev_addr[0];
        x.MAC_tableAdd.nMAC[1] = netif->dev_addr[1];
        x.MAC_tableAdd.nMAC[2] = netif->dev_addr[2];
        x.MAC_tableAdd.nMAC[3] = netif->dev_addr[3];
        x.MAC_tableAdd.nMAC[4] = netif->dev_addr[4];
        x.MAC_tableAdd.nMAC[5] = netif->dev_addr[5];
        ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MAC_TABLE_ENTRY_ADD, (unsigned int)&x.MAC_tableAdd);
        ltq_ethsw_api_kclose(handle);
        return NOTIFY_OK;
    case NETDEV_DOWN:
        handle = ltq_ethsw_api_kopen("/dev/switch_api/0");
        memset(&x.MAC_tableRemove, 0x00, sizeof(x.MAC_tableRemove));
        x.MAC_tableRemove.nFId = 0;
        x.MAC_tableRemove.nMAC[0] = netif->dev_addr[0];
        x.MAC_tableRemove.nMAC[1] = netif->dev_addr[1];
        x.MAC_tableRemove.nMAC[2] = netif->dev_addr[2];
        x.MAC_tableRemove.nMAC[3] = netif->dev_addr[3];
        x.MAC_tableRemove.nMAC[4] = netif->dev_addr[4];
        x.MAC_tableRemove.nMAC[5] = netif->dev_addr[5];
        ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE, (unsigned int)&x.MAC_tableRemove);
        ltq_ethsw_api_kclose(handle);
        return NOTIFY_OK;
    }

    return NOTIFY_DONE;
}
  #else
static int netdev_event_handler(struct notifier_block *nb, unsigned long event, void *netdev)
{
    return NOTIFY_DONE;
}
  #endif
#endif
/*
 * ####################################
 *           Global Function
 * ####################################
 */
#ifdef CONFIG_LTQ_ETHSW_API
static int mac_entry_setting(unsigned char *mac, uint32_t fid, uint32_t portid, uint32_t agetime, uint32_t st_entry , uint32_t action)
{
    LTQ_ETHSW_API_HANDLE handle;
    union ifx_sw_param x;
    int ret = PPA_SUCCESS;

    if(!mac)
        return PPA_FAILURE;

    switch(action)
    {
        case IFX_ETHSW_MAC_TABLE_ENTRY_ADD:
            handle = ltq_ethsw_api_kopen("/dev/switch_api/0");
            memset(&x.MAC_tableAdd, 0x00, sizeof(x.MAC_tableAdd));
            x.MAC_tableAdd.nFId = fid;
            x.MAC_tableAdd.nPortId = portid;
            x.MAC_tableAdd.nAgeTimer = agetime;
            x.MAC_tableAdd.bStaticEntry = st_entry;
            x.MAC_tableAdd.nMAC[0] = mac[0];
            x.MAC_tableAdd.nMAC[1] = mac[1];
            x.MAC_tableAdd.nMAC[2] = mac[2];
            x.MAC_tableAdd.nMAC[3] = mac[3];
            x.MAC_tableAdd.nMAC[4] = mac[4];
            x.MAC_tableAdd.nMAC[5] = mac[5];
            ret = ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MAC_TABLE_ENTRY_ADD, (unsigned int)&x.MAC_tableAdd);
            ltq_ethsw_api_kclose(handle);
            break;

        case IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE:
            handle = ltq_ethsw_api_kopen("/dev/switch_api/0");
            memset(&x.MAC_tableRemove, 0x00, sizeof(x.MAC_tableRemove));
            x.MAC_tableRemove.nFId = fid;
            x.MAC_tableRemove.nMAC[0] = mac[0];
            x.MAC_tableRemove.nMAC[1] = mac[1];
            x.MAC_tableRemove.nMAC[2] = mac[2];
            x.MAC_tableRemove.nMAC[3] = mac[3];
            x.MAC_tableRemove.nMAC[4] = mac[4];
            x.MAC_tableRemove.nMAC[5] = mac[5];
            ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE, (unsigned int)&x.MAC_tableRemove);
            ltq_ethsw_api_kclose(handle);
            break;

        default:
            break;
    }

    return ret;
}
#else
static int mac_entry_setting(unsigned char *mac, uint32_t fid, uint32_t portid, uint32_t agetime, uint32_t st_entry , uint32_t action)
{
    return PPA_SUCCESS;
}
#endif

static int in_showtime(void)
{
    int i;

    for(i = 0; i < LINE_NUMBER; i ++){
        if(g_showtime[i]){
            return 1;
        }
    }

    return 0;
}

static int ptm_showtime_enter(const unsigned char line_idx,struct port_cell_info *port_cell, void *xdata_addr)
{
    ASSERT(port_cell != NULL, "port_cell is NULL");
    ASSERT(xdata_addr != NULL, "xdata_addr is NULL");
    ASSERT(line_idx < LINE_NUMBER, "line_idx: %d large than max line num: %d", line_idx, LINE_NUMBER);

    //  TODO: ReTX set xdata_addr
    g_xdata_addr = xdata_addr;

    g_showtime[line_idx] = 1;

    dbg("line[%d]:enter showtime, xdata addr: 0x%08x", line_idx, (unsigned int)g_xdata_addr);

    return PPA_SUCCESS;
}

static int ptm_showtime_exit(const unsigned char line_idx)
{
    ASSERT(line_idx < LINE_NUMBER, "line_idx: %d large than max line num: %d", line_idx, LINE_NUMBER);
    
    if ( !g_showtime[line_idx] )
        return PPA_FAILURE;

    g_showtime[line_idx] = 0;

    //  TODO: ReTX clean state
    g_xdata_addr = NULL;

    dbg("line[%d]:leave showtime", line_idx);

    return PPA_SUCCESS;
}

static void check_showtime(void)
{
    int i;
    struct port_cell_info port_cell = {0};
    int showtime = 0;
    void *xdata_addr = NULL;
    int ret = -1;
    ltq_mei_atm_showtime_check_t ltq_mei_atm_showtime_check = NULL;

    ltq_mei_atm_showtime_check = (ltq_mei_atm_showtime_check_t)atm_callback_get(LTQ_MEI_SHOWTIME_CHECK);
    if (!ltq_mei_atm_showtime_check) {
        return;
    }

    for(i = 0; i < LINE_NUMBER; i++){
        ret = ltq_mei_atm_showtime_check(i, &showtime, &port_cell, &xdata_addr);
        if(!ret && showtime){
            ptm_showtime_enter(i,&port_cell, &xdata_addr);
        }
    }

    return;
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
        switch ( port )
        {
        case 0:
        case 1: return g_eth_prio_queue_map[port][skb->priority >= NUM_ENTITY(g_eth_prio_queue_map[port]) ? NUM_ENTITY(g_eth_prio_queue_map[port]) - 1 : skb->priority];
        case 7: return g_ptm_prio_queue_map[0][skb->priority >= NUM_ENTITY(g_ptm_prio_queue_map[0]) ? NUM_ENTITY(g_ptm_prio_queue_map[0]) - 1 : skb->priority];
        default: return 0;
        }
    }
}

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
static int ppe_directpath_enqueue(uint32_t if_id, struct sk_buff *skb)
{
    uint32_t off = if_id - 3;
    int ret = 0;
    
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
    int restart_task = 0;


    spin_lock_bh(&g_ppe_directpath_data[off].txq_lock);
    if(g_ppe_directpath_data[off].skb_list == NULL){
        restart_task = 1;
    }
#endif
    
    if (
  #if defined(ENABLE_DIRECTPATH_TX_QUEUE_SIZE)
             g_ppe_directpath_data[off].skb_list_size + skb->len + sizeof(struct sw_eg_pkt_header) <= ENABLE_DIRECTPATH_TX_QUEUE_SIZE
  #else
             g_ppe_directpath_data[off].skb_list_len  < ENABLE_DIRECTPATH_TX_QUEUE_PKTS
  #endif
           )
        {
            struct sw_eg_pkt_header pkth = {0};

            skb_push(skb, sizeof(struct sw_eg_pkt_header));
            pkth.spid = if_id;  //  CPU/app0/app1/app2
            if ( (g_wan_itf & (1 << if_id)) )   //  if source port is WAN interface (LTE)
            {
                pkth.dpid_en = 1;
                pkth.dpid = 2;
            }
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
        
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
        spin_unlock_bh(&g_ppe_directpath_data[off].txq_lock);
        if(restart_task){
            tasklet_schedule(&g_directpath_task); 
        }
#endif

        return ret;
}
#endif

int ppe_directpath_send(uint32_t if_id, struct sk_buff *skb, int32_t len, uint32_t flags)
{
    int ret;
    unsigned long sys_flag;
    uint32_t off;
    unsigned int dpid = (g_wan_itf & (1 << if_id)) ? 2 : if_id;

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
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)
    return ppe_directpath_enqueue(if_id, skb);
#endif

    spin_lock_irqsave(&g_directpath_tx_spinlock, sys_flag);
#if !defined(ENABLE_DIRECTPATH_TX_QUEUE) || !ENABLE_DIRECTPATH_TX_QUEUE
    if ( eth_xmit(skb, dpid /* CPU or dummy */, 3, if_id /* spid */, -1 /* SW classification */) )
    {
        g_ppe_directpath_data[off].tx_pkt_dropped++;
        ret = -1;
    }
#else
    if ( !g_directpath_dma_full && !g_ppe_directpath_data[if_id - 3].skb_list )
    {
        if ( eth_xmit(skb, dpid /* CPU or dummy */, 3, if_id /* spid */, -1 /* SW classification */) )
        {
            g_ppe_directpath_data[off].tx_pkt_dropped++;
            ret = -1;
        }
    }
    else
    {
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH_TASKLET)    
        ret = ppe_directpath_enqueue(if_id, skb);
#endif
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
    TX_QOS_CFG->time_tick = cgu_get_pp32_clock() / 62500;
#else
    struct clk * clk;
    clk = clk_get_ppe();
    TX_QOS_CFG->time_tick = clk_get_rate(clk) / 62500;


#endif

    return 0;
}

static uint32_t get_mailbox1_irq_num(void)
{
    volatile struct ar10_chipid *chipid = (volatile struct ar10_chipid *)AR10_MIPS_CHIPID;
    printk("CHIPID: %d, chipid address: 0x%x\n", chipid->version, (uint32_t)chipid);
    if(chipid->version > 1){
        return INT_NUM_IM0_IRL24;
    }

    return INT_NUM_IM1_IRL29;
}


/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */
#if (INCLUDE_MPRINT_PROC == 1)
    #include "proc_dbg.c"
#endif

static int __init ppe_eth_init(void)
{
    int ret;
    int port;
    char buf[512];
    volatile unsigned int *ppe_tc_sync = xTM_MODE_TO_DSL;

#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
    struct ifx_eth_fw_netdev_ops ptm_ops = {
        .get_stats      = eth_get_stats,
        .open           = eth_open,
        .stop           = eth_stop,
        .start_xmit     = eth_qos_hard_start_xmit,
        .set_mac_address= eth_set_mac_address,
        .do_ioctl       = ptm_ioctl,
        .tx_timeout     = ptm_tx_timeout,
        .watchdog_timeo = ETH_WATCHDOG_TIMEOUT,
    };
    struct ifx_eth_fw_netdev_ops eth_ops = {
        .get_stats      = eth_get_stats,
        .open           = eth_open,
        .stop           = eth_stop,
        .start_xmit     = NULL,
        .set_mac_address= eth_set_mac_address,
        .do_ioctl       = eth_ioctl,
        .tx_timeout     = eth_tx_timeout,
        .watchdog_timeo = ETH_WATCHDOG_TIMEOUT,
    };
    struct eth_priv_data *priv;
#endif

    printk("Loading E5 (MII0/1 + PTM) driver ...... ");
#if PPA_DP_DBG_PARAM_ENABLE
    if( ppa_drv_dp_dbg_param_enable == 1 )
    {
        ethwan = ppa_drv_dp_dbg_param_ethwan;
        wanitf = ppa_drv_dp_dbg_param_wanitf;
        ipv6_acc_en= ppa_drv_dp_dbg_param_ipv6_acc_en;
        wanqos_en= ppa_drv_dp_dbg_param_wanqos_en;
    }
#endif  //PPA_DP_DBG_PARAM_ENABLE

    //  init timer for Data Led
    setup_timer(&g_dsl_led_polling_timer, dsl_led_polling, 0);
	printk("Loading E5 (MII0/1 + PTM) driver ...... \n");

    init_local_variables();

    init_hw();

    init_communication_data_structures(g_fwcode);

    ret = alloc_dma();
    if ( ret )
        goto ALLOC_DMA_FAIL;

    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
    {
        g_ptm_net_dev[0] = ifx_eth_fw_alloc_netdev(sizeof(struct eth_priv_data), "ptm0", &ptm_ops);
        if ( g_ptm_net_dev[0] == NULL )
        {
            ret = -ENOMEM;
            goto ALLOC_NETDEV_PTM_FAIL;
        }
        ptm_setup(g_ptm_net_dev[0]);
        ret = ifx_eth_fw_register_netdev(g_ptm_net_dev[0]);
        if ( ret != 0 )
            goto RETISTER_NETDEV_PTM_FAIL;
    }

    for ( port = 0; port < NUM_ENTITY(g_eth_net_dev); port++ )
    {
#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
        if ( g_eth_wan_mode == 3 /* ETH WAN */ && g_wanqos_en && (g_wan_itf & (1 << port))  )
            eth_ops.start_xmit = eth_qos_hard_start_xmit;
        else
            eth_ops.start_xmit = eth_hard_start_xmit;
#endif
        sprintf(buf, "eth%d", port);
        g_eth_net_dev[port] = ifx_eth_fw_alloc_netdev(sizeof(struct eth_priv_data), buf, &eth_ops);
        if ( g_eth_net_dev[port] == NULL )
        {
            ret = -ENOMEM;
            goto ALLOC_NETDEV_ETH_FAIL;
        }
        eth_setup(g_eth_net_dev[port]);
    }
    for ( port = 0; port < NUM_ENTITY(g_eth_net_dev); port++ )
    {
        ret = ifx_eth_fw_register_netdev(g_eth_net_dev[port]);
        if ( ret != 0 )
            goto RETISTER_NETDEV_ETH_FAIL;
#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
        priv = ifx_eth_fw_netdev_priv(g_eth_net_dev[port]);
        priv->rx_packets = priv->stats.rx_packets;
        priv->rx_bytes   = priv->stats.rx_bytes;
        priv->rx_dropped = priv->stats.rx_dropped;
        priv->tx_packets = priv->stats.tx_packets;
        priv->tx_bytes   = priv->stats.tx_bytes;
        priv->tx_errors  = priv->stats.tx_errors;
        priv->tx_dropped = priv->stats.tx_dropped;
#endif
    }

    ret = request_irq(PPE_MAILBOX_IGU0_INT, mailbox0_irq_handler, IRQF_DISABLED, "e5_mailbox0_isr", NULL);
    if ( ret )
    {
        if ( ret == -EBUSY )
            err("IRQ may be occupied by other PPE driver, please reconfig to disable it.\n");
        goto REQUEST_IGU0_IRQ_FAIL;
    }

    PPE_MAILBOX_IGU1_INT = get_mailbox1_irq_num();
    ret = request_irq(PPE_MAILBOX_IGU1_INT, mailbox_irq_handler, IRQF_DISABLED, "e5_mailbox_isr", NULL);
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

    dump_init(1);

    ret = pp32_start(g_fwcode);
    if ( ret )
        goto PP32_START_FAIL;

    *MBOX_IGU0_IER = g_eth_wan_mode ? DMA_TX_CH0_SIG | DMA_TX_CH1_SIG : DMA_TX_CH0_SIG;
    *MBOX_IGU1_IER = g_mailbox_signal_mask;

	/*
	* DSL PTM
	*/
	*ppe_tc_sync = ((*ppe_tc_sync) & ~0x01) | 0x02;

    start_cpu_port();

#ifdef AR10_EMULATION
    //emulation, enable port0-port5, port0 is connected to company network, port 1,2,4,5 mapped to
    //smartbit port 6,7,8,9
    for(port = 0; port <= 5; port ++){
        arx_ethsw_port_enable(port);
    }
#endif
    //  turn on DMA TX channels (0, 1) for fast path
    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
    {
        atm_callback_set(LTQ_MEI_SHOWTIME_ENTER, (void *)ptm_showtime_enter);
        atm_callback_set(LTQ_MEI_SHOWTIME_EXIT, (void *)ptm_showtime_exit);

        check_showtime();

#if defined(ENABLE_LED_FRAMEWORK) && ENABLE_LED_FRAMEWORK && defined(CONFIG_LTQ_LED)
        ifx_led_trigger_register("dsl_data", &g_data_led_trigger);
#endif
    }

    register_netdev_event_handler();

    /*  create proc file    */
    proc_file_create();

#if (INCLUDE_MPRINT_PROC == 1)
    /* add dbg proc */
    ppa_procdbg_init();
#endif

    //dump_init(0);
     //Fix warning message ---start
    ppa_drv_get_dslwan_qid_with_vcc_hook = get_dslwan_qid_with_vcc;
    ppa_drv_get_netif_qid_with_pkt_hook = get_netif_qid_with_pkt;
    ppa_drv_ppe_clk_change_hook = ppe_clk_change;


#ifdef CONFIG_LTQ_PPA_API_DIRECTPATH
    ppa_drv_g_ppe_directpath_data = g_ppe_directpath_data;
    ppa_drv_directpath_send_hook = ppe_directpath_send;
    ppa_drv_directpath_rx_start_hook = ppe_directpath_rx_start;
    ppa_drv_directpath_rx_stop_hook = ppe_directpath_rx_stop;
#endif //CONFIG_LTQ_PPA_API_DIRECTPATH
    ppa_drv_datapath_generic_hook = ppa_datapath_generic_hook;
    ppa_drv_datapath_mac_entry_setting = mac_entry_setting;
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
    port = NUM_ENTITY(g_eth_net_dev);
RETISTER_NETDEV_ETH_FAIL:
    while ( port-- )
        ifx_eth_fw_unregister_netdev(g_eth_net_dev[port], 0);
    port = NUM_ENTITY(g_eth_net_dev);
ALLOC_NETDEV_ETH_FAIL:
    while ( port-- )
    {
        ifx_eth_fw_free_netdev(g_eth_net_dev[port], 0);
        g_eth_net_dev[port] = NULL;
    }
    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
        ifx_eth_fw_unregister_netdev(g_ptm_net_dev[0], 0);
RETISTER_NETDEV_PTM_FAIL:
    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
    {
        ifx_eth_fw_free_netdev(g_ptm_net_dev[0], 0);
        g_ptm_net_dev[0] = NULL;
    }
ALLOC_NETDEV_PTM_FAIL:
    free_dma();
ALLOC_DMA_FAIL:
    clear_local_variables();
    return ret;
}

static void __exit ppe_eth_exit(void)
{
    int i;
    //Fix warning message ---start
    ppa_drv_get_dslwan_qid_with_vcc_hook = NULL;
    ppa_drv_get_netif_qid_with_pkt_hook = NULL;
    ppa_drv_ppe_clk_change_hook = NULL;


#ifdef CONFIG_LTQ_PPA_API_DIRECTPATH
    ppa_drv_g_ppe_directpath_data = NULL;
    ppa_drv_directpath_send_hook = NULL;
    ppa_drv_directpath_rx_start_hook = NULL;
    ppa_drv_directpath_rx_stop_hook = NULL;
#endif //CONFIG_LTQ_PPA_API_DIRECTPATH
   ppa_drv_datapath_generic_hook = NULL;
   ppa_drv_datapath_mac_entry_setting = NULL;
    //Fix warning message ---end

#if (INCLUDE_MPRINT_PROC == 1)
    /* remove dbg proc */
    ppa_procdbg_exit();
#endif

    proc_file_delete();

    unregister_netdev_event_handler();

    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
    {
    atm_callback_set(LTQ_MEI_SHOWTIME_ENTER, (void *)NULL);
    atm_callback_set(LTQ_MEI_SHOWTIME_EXIT, (void *)NULL);

        del_timer(&g_dsl_led_polling_timer);
#if defined(ENABLE_LED_FRAMEWORK) && ENABLE_LED_FRAMEWORK && defined(CONFIG_LTQ_LED)
        ifx_led_trigger_deregister(g_data_led_trigger);
        g_data_led_trigger = NULL;
#endif
    }

    stop_datapath();

    pp32_stop();


    free_irq(PPE_MAILBOX_IGU1_INT, NULL);

    free_irq(PPE_MAILBOX_IGU0_INT, NULL);

    for ( i = 0; i < NUM_ENTITY(g_eth_net_dev); i++ )
    {
        ifx_eth_fw_unregister_netdev(g_eth_net_dev[i], 0);
        ifx_eth_fw_free_netdev(g_eth_net_dev[i], 0);
        g_eth_net_dev[i] = NULL;
    }

    if ( g_eth_wan_mode == 0 /* DSL WAN */ )
    {
        ifx_eth_fw_unregister_netdev(g_ptm_net_dev[0], 0);
        ifx_eth_fw_free_netdev(g_ptm_net_dev[0], 0);
        g_ptm_net_dev[0] = NULL;
    }

    free_dma();

    clear_local_variables();
}

#if defined(CONFIG_LTQ_PPA_DATAPATH)
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
    //  support DSL WAN only

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

    if ( tmp_wanqos_en >= 0 && tmp_wanqos_en <= 8 )
        wanqos_en = tmp_wanqos_en;

    return 0;
}
 static int __init qos_queue_len_setup(char *line)
  {
	  char *p;
	  int i;
  
	  for ( i = 0, p = line; i < NUM_ENTITY(qos_queue_len) && isxdigit(*p); i++ )
	  {
		  qos_queue_len[i] = simple_strtoul(p, &p, 0);
		  if ( *p == ',' || *p == ';' || *p == ':' )
			  p++;
	  }
  
	  return 0;
  }

 #if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE)
static int __init directpath_tx_queue_size_setup(char *line)
{
    unsigned long i;

    i = simple_strtoul(line, NULL, 0);
    directpath_tx_queue_size = (int)i;

    return 0;
}
 #endif

 #if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
static int __init directpath_tx_queue_pkts_setup(char *line)
{
    unsigned long i;

    i = simple_strtoul(line, NULL, 0);
    directpath_tx_queue_pkts = (int)i;

    return 0;
}
 #endif
#endif

int32_t ppa_datapath_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    dbg("ppa_datapath_generic_hook cmd 0x%x\n", cmd );
    switch (cmd)  {
    case PPA_GENERIC_DATAPATH_TSET:

        return PPA_SUCCESS;
    case PPA_GENERIC_DATAPATH_ADDR_TO_FPI_ADDR:
        {
            PPA_FPI_ADDR *addr = (PPA_FPI_ADDR *) buffer;
            addr->addr_fpi = sb_addr_to_fpi_addr_convert(addr->addr_orig);
            return PPA_SUCCESS;
        }
    case PPA_GENERIC_DATAPATH_CLEAR_MIB:
        {
            clear_mib_counter(0x80000000);

            return PPA_SUCCESS;
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
        return PPA_FAILURE;
    }

    return PPA_FAILURE;

}
module_init(ppe_eth_init);
module_exit(ppe_eth_exit);
#if defined(CONFIG_LTQ_PPA_DATAPATH)
  __setup("ethaddr=", eth0addr_setup);
  __setup("eth1addr=", eth1addr_setup);
  __setup("ethwan=", wan_mode_setup);
  __setup("wanitf=", wanitf_setup);
  __setup("ipv6_acc_en=", ipv6_acc_en_setup);
  __setup("wanqos_en=", wanqos_en_setup);
  __setup("qos_queue_len=", qos_queue_len_setup);
 #if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE)
  __setup("directpath_tx_queue_size=", directpath_tx_queue_size_setup);
 #endif
 #if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
  __setup("directpath_tx_queue_pkts=", directpath_tx_queue_pkts_setup);
 #endif
#endif
MODULE_LICENSE("GPL");
