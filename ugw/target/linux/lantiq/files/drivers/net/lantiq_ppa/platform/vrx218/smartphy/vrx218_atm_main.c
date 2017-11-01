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
#include <linux/delay.h>
#include <linux/proc_fs.h>

/*
 *  Chip Specific Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pcie.h>
#else
//#include <ltq_regs.h>
//#include <common_routines.h>
//#include <ltq_pmu.h>
//#include <asm/mach-ltqcpe/ltq_pcie.h>
#include <linux/version.h>
#include <lantiq_dma.h>
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>
#include <lantiq_pcie.h>

#endif

#include <net/ppa_api.h>
#include <net/ppa_stack_al.h>
#include <net/ppa_api_directpath.h>
#include <net/ppa_ppe_hal.h>
#include "../../ppa_datapath.h"

#include "vrx218_common.h"
#include "vrx218_ppe_fw_ds.h"
#include "vrx218_ppe_atm_ds.h"
#include "vrx218_a1plus_addr_def.h"
#include "vrx218_ppe_fw_const.h"
#include "vrx218_a1plus.h"
#include "unified_qos_ds_be.h"
#include "vrx218_atm_common.h"



/*
 * ####################################
 *   Parameters to Configure PPE
 * ####################################
 */



/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  Firmware Registers
 */
#define FW_VER_ID                               ((volatile struct fw_ver_id *)          SOC_ACCESS_VRX218_SB(__FW_VER_ID, g_vrx218_dev.phy_membase))
#define CFG_WRX_HTUTS                           SOC_ACCESS_VRX218_SB(__CFG_WRX_HTUTS, g_vrx218_dev.phy_membase) /*  WAN RX HTU Table Size, must be configured before enable PPE firmware.   */

#define WRX_QUEUE_CONFIG(i)                     ((volatile wrx_queue_config_t *)        SOC_ACCESS_VRX218_SB(__WRX_QUEUE_CONFIG + (i) * 10, g_vrx218_dev.phy_membase))  /*  i < 16  */
#define WTX_QUEUE_CONFIG(i)                     ((volatile wtx_queue_config_t *)        SOC_ACCESS_VRX218_SB(__WTX_QUEUE_CONFIG + (i) * 25, g_vrx218_dev.phy_membase))  /*  i < 15  */

#define HTU_ENTRY(i)                            ((volatile struct htu_entry *)          SOC_ACCESS_VRX218_SB(__HTU_ENTRY_TABLE + (i), g_vrx218_dev.phy_membase))        /*  i < 32  */
#define HTU_MASK(i)                             ((volatile struct htu_mask *)           SOC_ACCESS_VRX218_SB(__HTU_MASK_TABLE + (i), g_vrx218_dev.phy_membase))         /*  i < 32  */
#define HTU_RESULT(i)                           ((volatile struct htu_result *)         SOC_ACCESS_VRX218_SB(__HTU_RESULT_TABLE + (i), g_vrx218_dev.phy_membase))       /*  i < 32  */

#define DSL_WAN_MIB_TABLE                       ((volatile struct dsl_wan_mib_table *)  SOC_ACCESS_VRX218_SB(0x4EF0, g_vrx218_dev.phy_membase))
//  DSL_QUEUE_RX_MIB_TABLE - each queue stands for one PVC
#define DSL_QUEUE_RX_MIB_TABLE(i)               ((volatile struct dsl_queue_mib *)      SOC_ACCESS_VRX218_SB(__WRX_VC_MIB_BASE + (i) * 2, g_vrx218_dev.phy_membase))    /*  i < 16  */
//  DSL_QUEUE_TX_MIB_TABLE, DSL_QUEUE_TX_DROP_MIB_TABLE - multiple queue are attached to one PVC according to WTX_QUEUE_CONFIG
#define DSL_QUEUE_TX_MIB_TABLE(i)               ((volatile struct dsl_queue_mib *)      SOC_ACCESS_VRX218_SB(__WTX_VC_MIB_BASE + (i) * 2, g_vrx218_dev.phy_membase))    /*  i < 16  */
//  Not available in VRX218 A1+
//#define DSL_QUEUE_TX_DROP_MIB_TABLE(i)        ((volatile struct dsl_queue_drop_mib *) SOC_ACCESS_VRX218_SB(0x2FD0 + (i), g_vrx218_dev.phy_membase))   /*  i < 16  */

/*
 *  RCU Registers
 */
#define RST_REQ(base)                           SOC_ACCESS_VRX218_ADDR(0x1E002010, base)
#define RST_STAT(base)                          SOC_ACCESS_VRX218_ADDR(0x1E002000, base)

/*
 *  PMU Registers
 */
#define PMU_PWDCR(base)                         SOC_ACCESS_VRX218_ADDR(0x1E00311C, base)
#define PMU_SR(base)                            SOC_ACCESS_VRX218_ADDR(0x1E003120, base)

/*
 *  RTHA/TTHA Registers
 */
#define RFBI_CFG(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0400), base)
#define RBA_CFG0(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0404), base)
#define RBA_CFG1(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0405), base)
#define RCA_CFG0(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0408), base)
#define RCA_CFG1(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0409), base)
#define RDES_CFG0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x040C), base)
#define RDES_CFG1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x040D), base)
#define SFSM_STATE0(base)                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0410), base)
#define SFSM_STATE1(base)                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0411), base)
#define SFSM_DBA0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0412), base)
#define SFSM_DBA1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0413), base)
#define SFSM_CBA0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0414), base)
#define SFSM_CBA1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0415), base)
#define SFSM_CFG0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0416), base)
#define SFSM_CFG1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0417), base)
#define SFSM_PGCNT0(base)                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x041C), base)
#define SFSM_PGCNT1(base)                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x041D), base)
#define FFSM_DBA0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0508), base)
#define FFSM_DBA1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0509), base)
#define FFSM_CFG0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x050A), base)
#define FFSM_CFG1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x050B), base)
#define FFSM_IDLE_HEAD_BC0(base)                SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x050E), base)
#define FFSM_IDLE_HEAD_BC1(base)                SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x050F), base)
#define FFSM_PGCNT0(base)                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0514), base)
#define FFSM_PGCNT1(base)                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0515), base)

/*
 *  PPE TC Logic Registers (partial)
 */
#define DREG_A_VERSION(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D00), base)
#define DREG_A_CFG(base)                        SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D01), base)
#define DREG_AT_CTRL(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D02), base)
#define DREG_AT_CB_CFG0(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D03), base)
#define DREG_AT_CB_CFG1(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D04), base)
#define DREG_AR_CTRL(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D08), base)
#define DREG_AR_CB_CFG0(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D09), base)
#define DREG_AR_CB_CFG1(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D0A), base)
#define DREG_A_UTPCFG(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D0E), base)
#define DREG_A_STATUS(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D0F), base)
#define DREG_AT_CFG0(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D20), base)
#define DREG_AT_CFG1(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D21), base)
#define DREG_AT_FB_SIZE0(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D22), base)
#define DREG_AT_FB_SIZE1(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D23), base)
#define DREG_AT_CELL0(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D24), base)
#define DREG_AT_CELL1(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D25), base)
#define DREG_AT_IDLE_CNT0(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D26), base)
#define DREG_AT_IDLE_CNT1(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D27), base)
#define DREG_AT_IDLE0(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D28), base)
#define DREG_AT_IDLE1(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D29), base)
#define DREG_AR_CFG0(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D60), base)
#define DREG_AR_CFG1(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D61), base)
#define DREG_AR_CELL0(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D68), base)
#define DREG_AR_CELL1(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D69), base)
#define DREG_AR_IDLE_CNT0(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D6A), base)
#define DREG_AR_IDLE_CNT1(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D6B), base)
#define DREG_AR_AIIDLE_CNT0(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D6C), base)
#define DREG_AR_AIIDLE_CNT1(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D6D), base)
#define DREG_AR_BE_CNT0(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D6E), base)
#define DREG_AR_BE_CNT1(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D6F), base)
#define DREG_AR_HEC_CNT0(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D70), base)
#define DREG_AR_HEC_CNT1(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D71), base)
#define DREG_AR_IDLE0(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D74), base)
#define DREG_AR_IDLE1(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D75), base)
#define DREG_AR_OVDROP_CNT0(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D98), base)
#define DREG_AR_OVDROP_CNT1(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0D99), base)
#define DREG_AR_CERRN_CNT0(base)                SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA0), base)
#define DREG_AR_CERRN_CNT1(base)                SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA1), base)
#define DREG_AR_CERRNP_CNT0(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA2), base)
#define DREG_AR_CERRNP_CNT1(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA3), base)
#define DREG_AR_CVN_CNT0(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA4), base)
#define DREG_AR_CVN_CNT1(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA5), base)
#define DREG_AR_CVNP_CNT0(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA6), base)
#define DREG_AR_CVNP_CNT1(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA7), base)
#define DREG_B0_LADR(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA8), base)
#define DREG_B1_LADR(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0DA9), base)

#define GIF0_RX_CRC_ERR_CNT(base)               DREG_AR_CERRN_CNT0(base)
#define GIF1_RX_CRC_ERR_CNT(base)               DREG_AR_CERRNP_CNT0(base)
#define GIF2_RX_CRC_ERR_CNT(base)               DREG_AR_CERRN_CNT1(base)
#define GIF3_RX_CRC_ERR_CNT(base)               DREG_AR_CERRNP_CNT1(base)
#define GIF0_RX_CV_CNT(base)                    DREG_AR_CVN_CNT0(base)
#define GIF1_RX_CV_CNT(base)                    DREG_AR_CVNP_CNT0(base)
#define GIF2_RX_CV_CNT(base)                    DREG_AR_CVN_CNT1(base)
#define GIF3_RX_CV_CNT(base)                    DREG_AR_CVNP_CNT1(base)
#define DREG_B0_OVERDROP_CNT(base)              DREG_AR_OVDROP_CNT0(base)
#define DREG_B1_OVERDROP_CNT(base)              DREG_AR_OVDROP_CNT1(base)

/*
 *  SAR Registers
 */
#define SAR_MODE_CFG(base)                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x080A), base)
#define SAR_RX_CMD_CNT(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x080B), base)
#define SAR_TX_CMD_CNT(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x080C), base)
#define SAR_RX_CTX_CFG(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x080D), base)
#define SAR_TX_CTX_CFG(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x080E), base)
#define SAR_TX_CMD_DONE_CNT(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x080F), base)
#define SAR_POLY_CFG_SET0(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0812), base)
#define SAR_POLY_CFG_SET1(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0813), base)
#define SAR_POLY_CFG_SET2(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0814), base)
#define SAR_POLY_CFG_SET3(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0815), base)
#define SAR_CRC_SIZE_CFG(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0816), base)

/*
 *  PDMA/EMA Registers
 */
#define PDMA_CFG(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A00), base)
#define PDMA_RX_CMDCNT(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A01), base)
#define PDMA_TX_CMDCNT(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A02), base)
#define PDMA_RX_FWDATACNT(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A03), base)
#define PDMA_TX_FWDATACNT(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A04), base)
#define PDMA_RX_CTX_CFG(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A05), base)
#define PDMA_TX_CTX_CFG(base)                   SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A06), base)
#define PDMA_RX_MAX_LEN_REG(base)               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A07), base)
#define PDMA_RX_DELAY_CFG(base)                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A08), base)
#define PDMA_INT_FIFO_RD(base)                  SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A09), base)
#define PDMA_ISR(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A0A), base)
#define PDMA_IER(base)                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A0B), base)
#define PDMA_SUBID(base)                        SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A0C), base)
#define PDMA_BAR0(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A0D), base)
#define PDMA_BAR1(base)                         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0A0E), base)

#define SAR_PDMA_RX_CMDBUF_CFG(base)            SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0F00), base)
#define SAR_PDMA_TX_CMDBUF_CFG(base)            SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0F01), base)
#define SAR_PDMA_RX_FW_CMDBUF_CFG(base)         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0F02), base)
#define SAR_PDMA_TX_FW_CMDBUF_CFG(base)         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0F03), base)
#define SAR_PDMA_RX_CMDBUF_STATUS(base)         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0F04), base)
#define SAR_PDMA_TX_CMDBUF_STATUS(base)         SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0F05), base)

/*
 *  Mailbox IGU0 Registers
 */
#define MBOX_IGU0_ISRS(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0200), base)
#define MBOX_IGU0_ISRC(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0201), base)
#define MBOX_IGU0_ISR(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0202), base)
#define MBOX_IGU0_IER(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0203), base)

/*
 *  Mailbox IGU1 Registers
 */
#define MBOX_IGU1_ISRS(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0204), base)
#define MBOX_IGU1_ISRC(base)                    SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0205), base)
#define MBOX_IGU1_ISR(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0206), base)
#define MBOX_IGU1_IER(base)                     SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0207), base)

/*
 *    Code/Data Memory (CDM) Interface Configuration Register
 */
#define CDM_CFG(base)                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0100), base)

#define CDM_CFG_RAM1_SET(value)                 SET_BITS(0, 3, 2, value)
#define CDM_CFG_RAM0_SET(value)                 ((value) ? (1 << 1) : 0)

/*
 *  PP32 Debug Control Register
 */
#define PP32_FREEZE(base)                               SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0000), base)
#define PP32_SRST(base)                                 SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0020), base)

#define PP32_DBG_CTRL(base, n)                          SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0000), base)

#define DBG_CTRL_RESTART                                0
#define DBG_CTRL_STOP                                   1

#define PP32_CTRL_CMD(base, n)                          SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0B00), base)
  #define PP32_CTRL_CMD_RESTART                         (1 << 0)
  #define PP32_CTRL_CMD_STOP                            (1 << 1)
  #define PP32_CTRL_CMD_STEP                            (1 << 2)
  #define PP32_CTRL_CMD_BREAKOUT                        (1 << 3)

#define PP32_CTRL_OPT(base, n)                          SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0C00), base)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP_ON             (3 << 0)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP_OFF            (2 << 0)
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN_ON          (3 << 2)
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN_OFF         (2 << 2)
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN_ON              (3 << 4)
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN_OFF             (2 << 4)
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON           (3 << 6)
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF          (2 << 6)
  #define PP32_CTRL_OPT_BREAKOUT_ON_STOP(base,n)        (IFX_REG_R32(PP32_CTRL_OPT(base, n)) & (1 << 0))
  #define PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN(base, n)    (IFX_REG_R32(PP32_CTRL_OPT(base, n)) & (1 << 2))
  #define PP32_CTRL_OPT_STOP_ON_BREAKIN(base, n)        (IFX_REG_R32(PP32_CTRL_OPT(base, n)) & (1 << 4))
  #define PP32_CTRL_OPT_STOP_ON_BREAKPOINT(base, n)     (IFX_REG_R32(PP32_CTRL_OPT(base, n)) & (1 << 6))

#define PP32_BRK_PC(base, n, i)                         SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0900 + (i) * 2), base)
#define PP32_BRK_PC_MASK(base, n, i)                    SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0901 + (i) * 2), base)
#define PP32_BRK_DATA_ADDR(base, n, i)                  SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0904 + (i) * 2), base)
#define PP32_BRK_DATA_ADDR_MASK(base, n, i)             SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0905 + (i) * 2), base)
#define PP32_BRK_DATA_VALUE_RD(base, n, i)              SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0908 + (i) * 2), base)
#define PP32_BRK_DATA_VALUE_RD_MASK(base, n, i)         SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0909 + (i) * 2), base)
#define PP32_BRK_DATA_VALUE_WR(base, n, i)              SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x090C + (i) * 2), base)
#define PP32_BRK_DATA_VALUE_WR_MASK(base, n, i)         SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x090D + (i) * 2), base)
  #define PP32_BRK_CONTEXT_MASK(i)                      (1 << (i))
  #define PP32_BRK_CONTEXT_MASK_EN                      (1 << 4)
  #define PP32_BRK_COMPARE_GREATER_EQUAL                (1 << 5)    //  valid for break data value rd/wr only
  #define PP32_BRK_COMPARE_LOWER_EQUAL                  (1 << 6)
  #define PP32_BRK_COMPARE_EN                           (1 << 7)

#define PP32_BRK_TRIG(base, n)                          SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0F00), base)
  #define PP32_BRK_GRPi_PCn_ON(i, n)                    ((3 << ((n) * 2)) << ((i) * 16))
  #define PP32_BRK_GRPi_PCn_OFF(i, n)                   ((2 << ((n) * 2)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_ADDRn_ON(i, n)             ((3 << ((n) * 2 + 4)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_ADDRn_OFF(i, n)            ((2 << ((n) * 2 + 4)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn_ON(i, n)         ((3 << ((n) * 2 + 8)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn_OFF(i, n)        ((2 << ((n) * 2 + 8)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn_ON(i, n)         ((3 << ((n) * 2 + 12)) << ((i) * 16))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn_OFF(i, n)        ((2 << ((n) * 2 + 12)) << ((i) * 16))
  #define PP32_BRK_GRPi_PCn(base, k, i, n)              (IFX_REG_R32(PP32_BRK_TRIG(base, k)) & ((1 << ((n))) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_ADDRn(base, k, i, n)       (IFX_REG_R32(PP32_BRK_TRIG(base, k)) & ((1 << ((n) + 2)) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_VALUE_RDn(base, k, i, n)   (IFX_REG_R32(PP32_BRK_TRIG(base, k)) & ((1 << ((n) + 4)) << ((i) * 8)))
  #define PP32_BRK_GRPi_DATA_VALUE_WRn(base, k, i, n)   (IFX_REG_R32(PP32_BRK_TRIG(base, k)) & ((1 << ((n) + 6)) << ((i) * 8)))

#define PP32_CPU_STATUS(base, n)                        SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0D00), base)
#define PP32_HALT_STAT(base, n)                         SOC_ACCESS_VRX218_ADDR(PP32_CPU_STATUS(n), base)
#define PP32_DBG_CUR_PC(base, n)                        SOC_ACCESS_VRX218_ADDR(PP32_CPU_STATUS(n), base)
  #define PP32_CPU_USER_STOPPED(base, n)                (IFX_REG_R32(PP32_CPU_STATUS(base, n)) & (1 << 0))
  #define PP32_CPU_USER_BREAKIN_RCV(base, n)            (IFX_REG_R32(PP32_CPU_STATUS(base, n)) & (1 << 1))
  #define PP32_CPU_USER_BREAKPOINT_MET(base, n)         (IFX_REG_R32(PP32_CPU_STATUS(base, n)) & (1 << 2))
  #define PP32_CPU_CUR_PC(base, n)                      (IFX_REG_R32(PP32_CPU_STATUS(base, n)) >> 16)

#define PP32_BREAKPOINT_REASONS(base, n)                SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0A00), base)
  #define PP32_BRK_PC_MET(base, n, i)                   (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << (i)))
  #define PP32_BRK_DATA_ADDR_MET(base, n, i)            (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) + 2)))
  #define PP32_BRK_DATA_VALUE_RD_MET(base, n, i)        (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) + 4)))
  #define PP32_BRK_DATA_VALUE_WR_MET(base, n, i)        (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) + 6)))
  #define PP32_BRK_DATA_VALUE_RD_LO_EQ(base, n, i)      (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) * 2 + 8)))
  #define PP32_BRK_DATA_VALUE_RD_GT_EQ(base, n, i)      (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) * 2 + 9)))
  #define PP32_BRK_DATA_VALUE_WR_LO_EQ(base, n, i)      (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) * 2 + 12)))
  #define PP32_BRK_DATA_VALUE_WR_GT_EQ(base, n, i)      (IFX_REG_R32(PP32_BREAKPOINT_REASONS(base, n)) & (1 << ((i) * 2 + 13)))
  #define PP32_BRK_CUR_CONTEXT(base, n)                 ((IFX_REG_R32(PP32_CPU_STATUS(base, n)) >> 8) & 0x03)

#define PP32_GP_REG_BASE(base, n)                       SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0E00), base)
#define PP32_GP_CONTEXTi_REGn(base, n, i, j)            SOC_ACCESS_VRX218_ADDR(PP32_DEBUG_REG_ADDR(n, 0x0E00 + (i) * 16 + (j)), base)

#define VRX218_QOSQ_NUM                         8
#define VRX218_GIF_NUM                          8

/*
 *  CGU Registers
 */
#define CGU_CLKFSR(base)                        SOC_ACCESS_VRX218_ADDR(0x1E003010, base)



/*
 * ####################################
 *              Data Type
 * ####################################
 */

struct proc_entry_cfg{
    char                    *parent_name;
    char                    *name;
    unsigned int            is_dir;
    int (*proc_r_fn)(char*, char **, off_t , int , int*, void*);
    int (*proc_w_fn)(struct file*, const char*, unsigned long, void*);
    int                     is_enable;
    struct proc_dir_entry   *parent_proc_entry;
    struct proc_dir_entry   *proc_entry;
  };

/*
  * Proc debug
*/

typedef struct fw_dbg {
    char *cmd;
    void (*pfunc)(char **tokens, int token_num);
}fw_dbg_t;

typedef struct fw_naddr{
    char *name;
    uint32_t addr;
    uint32_t num;
}fw_naddr_t;




/*
 * ####################################
 *             Declaration
 * ####################################
 */

static void proc_file_create(void);
static void proc_file_delete(void);
static int proc_read_ver(struct seq_file *, void *);
static int proc_read_dbg(struct seq_file *, void *);
static ssize_t proc_write_dbg(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_mem(struct seq_file *, void *);
static ssize_t proc_write_mem(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_pp32(struct seq_file *, void *);
static ssize_t proc_write_pp32(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_wanmib(struct seq_file *, void *);
static ssize_t proc_write_wanmib(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_queue(struct seq_file *, void *);
static int proc_read_htu(struct seq_file *, void *);
extern int proc_read_prio(struct seq_file *, void *);
extern ssize_t proc_write_prio(struct file *, const char __user *, size_t, loff_t *);
extern int proc_read_dsl_vlan(struct seq_file *, void *);
extern ssize_t proc_write_dsl_vlan(struct file *, const char __user *, size_t, loff_t *);
extern ssize_t proc_write_cell(struct file *, const char __user *, size_t, loff_t *);
extern int proc_read_cell(struct seq_file *, void *);
static int proc_read_ps(struct seq_file *, void *);
static ssize_t proc_write_ps(struct file *, const char __user *, size_t, loff_t *);
extern int proc_read_pcie_rst(struct seq_file *, void *);

static void fwdbg_help(char **, int);
static void fw_dbg_start(char *);
static int proc_read_fwdbg(struct seq_file *, void *);
static ssize_t proc_write_fwdbg(struct file *, const char __user *, size_t, loff_t *);

static int proc_read_ver_seq_open(struct inode *, struct file *);
static int proc_read_dbg_seq_open(struct inode *, struct file *);
static int proc_read_mem_seq_open(struct inode *, struct file *);
static int proc_read_pp32_seq_open(struct inode *, struct file *);
static int proc_read_wanmib_seq_open(struct inode *, struct file *);
static int proc_read_queue_seq_open(struct inode *, struct file *);
static int proc_read_htu_seq_open(struct inode *, struct file *);
static int proc_read_prio_seq_open(struct inode *, struct file *);
static int proc_read_dsl_vlan_seq_open(struct inode *, struct file *);
static int proc_read_cell_seq_open(struct inode *, struct file *);
static int proc_read_fwdbg_seq_open(struct inode *, struct file *);
static int proc_read_powersaving_seq_open(struct inode *, struct file *);
static int proc_read_pciereset_seq_open(struct inode *, struct file *);


/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static ifx_pcie_ep_dev_t g_vrx218_dev;
static int g_vrx218_idx = -1;

/*  if no parent folder name provided, register function will take the nearest proc_dir_entry by default.
    if no proc_dir_entry found, by default, it's NULL. (e.g. create folder eth )
    Please make sure the file follow the right dir entry otherwise it would cause trouble to release it
 */

	static struct proc_entry_cfg g_proc_entry[] = {
     // Parent dir,    Name,    IS_DIR,   RD_fn,     WR_fn,     Enable
        {NULL,  "eth/vrx318",		1, NULL,  NULL,		1 },
        {NULL,  "ver",      		0, NULL,  NULL,		1 },
        {NULL,  "dbg",      		0, NULL,  NULL,       	1 },
        {NULL,  "mem",      		0, NULL,  NULL,       	1 },
        {NULL,  "pp32",     		0, NULL,  NULL,      	1 },
        {NULL,  "wanmib",    		0, NULL,  NULL,     	1 },
        {NULL,  "queue",    		0, NULL,  NULL,     	1 },
        {NULL,  "htu",      		0, NULL,  NULL,         1 },
        {NULL,  "prio",     		0, NULL,  NULL,      	1 },
        {NULL,  "dsl_vlan", 		0, NULL,  NULL,  	1 },
        {NULL,  "cell",   		0, NULL,  NULL,     	1 },
        {NULL,  "tfwdbg",   		0, NULL,  NULL,     	1 },
        {NULL,"powersaving",		0, NULL,  NULL, 	1 },
        {NULL,"pciereset",		0, NULL,  NULL, 	1 },

    };

/*
 * ####################################
 *            Local Function
 * ####################################
 */

/*
 *  Init & clean-up functions
 */
static int init_local_variables(void)
{
    return PPA_SUCCESS;
}

/*
 *  Hardware init & clean-up functions
 */
static void reset_ppe(unsigned int base)
{
    int max_loop = 10000;
#if !defined(ENABLE_PARTIAL_RESET_PPE) || !ENABLE_PARTIAL_RESET_PPE
    unsigned int reset_mask = (1 << 23) | (1 << 14) | (1 << 9) | (1 << 8) | (1 << 7) | (1 << 3);
#else
    unsigned int reset_mask = (1 << 23) | (1 << 9) | (1 << 7) | (1 << 3);
#endif

    IFX_REG_W32_MASK(0, reset_mask, RST_REQ(base));
    udelay(1000);
    while ( max_loop-- > 0 && (IFX_REG_R32(RST_REQ(base)) & reset_mask) );

    if ( (IFX_REG_R32(RST_REQ(base)) & reset_mask) )
        printk("failed in reset PPE\n");

#if defined(ENABLE_PARTIAL_RESET_PPE) && ENABLE_PARTIAL_RESET_PPE
    IFX_REG_W32_MASK(0x303CF, 0, PP32_SRST(base));
    udelay(1000);
    IFX_REG_W32_MASK(0, 0x303CF, PP32_SRST(base));
    udelay(1000);
#endif
}

static void enable_vrx218_ppe_ema(unsigned int base) {

    unsigned int rd_val;

    rd_val = *PMU_PWDCR(base);
    *PMU_PWDCR(base) = rd_val & ~(0x1 << 22);
    while (1) {
        rd_val = *PMU_SR(base);
        if ((rd_val & (0x1 << 22)) == 0)
            break;
    }
    dbg("[VRX318] PMU_PWDCR: 0x%08X", *PMU_PWDCR(base));
    dbg("[VRX318] PMU_SR   : 0x%08X", *PMU_SR(base));

    return;
}

static int inline is_char_in_set(char ch, char * set)
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


static void fwdbg_help(char **tokens, int token_num)
{
    const char *proc_file = "/proc/eth/vrx318/tfwdbg";
    const char *cmds[] = {
            "help",
            "dessync",
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

   if(strncasecmp(tokens[0],"dessync", strlen(tokens[0])) == 0){
        printk("echo dessync [CPU | FASTPATH | DS | OUTQ | LOCALQ | USEDMARD | DSEDMAWR | DSOAMQ ] [qidx] > %s \n", proc_file);
        printk("     :print vrx318 descriptor sync structure \n");
        printk("      CPU      for CPU PATH US_PATH_DES_SYNC\n");
        printk("      FASTPATH for FASTPATH US_PASH_DES_SYNC\n");
        printk("      DS       for DOWNSTREAM DS_PATH_DES_SYNC\n");
        printk("      OUTQ     for US_PATH_DES_SYNC Q0-Q3\n");
        printk("      LOCALQ   for US_PATH_DES_SYNC Q0-Q3\n");
        printk("      USEDMARD for Upstream EDMA Read CTXT   \n");
        printk("      DSEDMAWR for Downstream EDMA Write CTXT \n");
        printk("      DSOAMQ   for Downstream OAM DES SYNC \n");

    }
    else{
        fwdbg_help((char **)NULL, 0);
    }

}



static void print_des_sync(char *name, uint32_t addr)
{
    volatile desq_cfg_ctxt_t *des_sync_ctxt = (volatile desq_cfg_ctxt_t *)SOC_ACCESS_VRX218_ADDR(SB_BUFFER(addr),g_vrx218_dev.phy_membase);

    printk("%s Descriptor Sync Context:\n", name);
    printk("       desc own val:      %d\n",    des_sync_ctxt->des_in_own_val);
    printk("       fast path:         %d\n",    des_sync_ctxt->fast_path);
    printk("       mbox_int_en:       %d\n",    des_sync_ctxt->mbox_int_en);
    printk("       des_sync_need:     %d\n",    des_sync_ctxt->des_sync_needed);
    printk("       gif_id:            %d\n",    des_sync_ctxt->gif_id);
    printk("       des_num:           %d\n",    des_sync_ctxt->des_num);
    printk("       des_base_addr:     0x%x\n",  des_sync_ctxt->des_base_addr);
    printk("       mbox_int_cfg_ptr:  0x%x\n",  des_sync_ctxt->mbox_int_cfg_ptr);
    printk("       bp_des_base_addr:  0x%x\n",  des_sync_ctxt->bp_des_base_addr);
    printk("       deq_idx:           %d\n",    des_sync_ctxt->deq_idx);
    printk("       enq_idx:           %d\n",    des_sync_ctxt->enq_idx);
    printk("       enq_pkt_cnt:       %d\n",    des_sync_ctxt->enq_pkt_cnt);
    printk("       enq_byte_cnt:      %d\n",    des_sync_ctxt->enq_byte_cnt);
    printk("       deq_pkt_cnt:       %d\n",    des_sync_ctxt->deq_pkt_cnt);
    printk("       deq_byte_cnt:      %d\n",    des_sync_ctxt->deq_byte_cnt);

}

static void fwdbg_des_sync(char **tokens, int token_num)
{
    int idx = -1;
    int i = 0;
    int j = 0;
    char ctxt_name[32];
    struct fw_naddr des_sync_path[] =
                      {{"CPU",      __US_CPU_INQ_DES_CFG_CTXT,      1  },
                       {"FASTPATH", __US_FP_INQ_DES_CFG_CTXT,       1  },
                       {"DS",       __DS_PKT_DESQ_CFG_CTXT,         1  },
                       {"OUTQ",     __QOSQ_PSEUDO_DES_CFG_BASE,     16 },
                       {"LOCALQ",   __US_TC_LOCAL_Q_CFG_CTXT_BASE,  16 },
                       {"USEDMARD", __US_EDMA_READ_CH_CFG_CTXT,     1  },
                       {"DSEDMAWR", __DS_EDMA_WRITE_CH_CFG_CTXT,    1  },
                       {"DSOAMQ",   __DS_OAM_DESQ_CFG_CTXT,         1  },

                       {NULL, 0x0, 0}};

    if(token_num > 0){
        for(i = 0; des_sync_path[i].name != NULL; i ++){
            if(strncasecmp(tokens[0], des_sync_path[i].name, strlen(tokens[0])) == 0){
                idx = i;
                break;
            }
        }
    }
    j++;

    i = -1;
    if(idx != -1){
        if(token_num > j){
            i = simple_strtoul(tokens[j],NULL, 0);
        }

        if(i < 0 || i >= des_sync_path[idx].num){
            for(i = 0; i < des_sync_path[idx].num; i ++){
                sprintf(ctxt_name,"%s%d",des_sync_path[idx].name,i);
                print_des_sync(ctxt_name,(des_sync_path[idx].addr + i * 8));
            }
        }else{
            sprintf(ctxt_name,"%s%d",des_sync_path[idx].name,i);
            print_des_sync(ctxt_name,(des_sync_path[idx].addr + i * 8));
        }

    }else{
        fw_dbg_start("help dessync");
    }

    return;
}

static void fw_dbg_start(char *cmdbuf)
{
    char * tokens[32];
    int finished, token_num;
    int i;

    fw_dbg_t cmds[]={{"help",           fwdbg_help},
                     {"dessync",        fwdbg_des_sync},
                     {NULL, NULL}};


    token_num = tokenize(cmdbuf, " \t", tokens, 32, &finished);
    if (token_num <= 0) {
        return;
    }

    for(i = 0; cmds[i].cmd != NULL; i ++){
        if(strncasecmp(tokens[0], cmds[i].cmd, strlen(tokens[0])) == 0){
            cmds[i].pfunc(&tokens[1],token_num - 1);
            break;
        }
    }

    if(cmds[i].cmd == NULL){
        fw_dbg_start("help");
    }

    return;
}

static int proc_write_fwdbg(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[1024];
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

    ASSERT(p < str + 2048, "data point out of range");
    fw_dbg_start(p);

    return count;
}

static void set_vrx218_ppe_clk(unsigned int clk, unsigned int base)
{
    switch (clk) {
    case 288:
        *CGU_CLKFSR(base) = (*CGU_CLKFSR(base) & ~0x00070000) | 0x00020000;
        break;
    case 432:
        *CGU_CLKFSR(base) = (*CGU_CLKFSR(base) & ~0x00070000) | 0x00010000;
        break;
    }
}


/*
 *  PP32 specific functions
 */
static int pp32_download_code(const ifx_pcie_ep_dev_t *dev, int pp32, const u32 *code_src, unsigned int code_dword_len, const u32 *data_src, unsigned int data_dword_len)
{
    unsigned int clr, set;
    volatile unsigned int *dest;
    int i;

    if ( code_src == 0 || ((unsigned long)code_src & 0x03) != 0
        || data_src == 0 || ((unsigned long)data_src & 0x03) != 0 )
        return PPA_FAILURE;

    clr = pp32 ? 0xF0 : 0x0F;
    if ( code_dword_len <= CDM_CODE_MEMORYn_DWLEN(0) )
        set = pp32 ? (3 << 6): (2 << 2);
    else
        set = 0x00;
    dbg("%s: addr(CDM_CFG) = 0x%08x, clr = 0x%02x, set 0x%02x", __FUNCTION__, (unsigned int)CDM_CFG(dev->phy_membase), clr, set);
    IFX_REG_W32_MASK(clr, set, CDM_CFG(dev->phy_membase));

    /*  clear code memory   */
    dest = (volatile unsigned int *)SOC_ACCESS_VRX218_ADDR(CDM_CODE_MEMORY(pp32, 0), dev->phy_membase);
    for ( i = 0; i < CDM_CODE_MEMORYn_DWLEN(0); i++ )
        IFX_REG_W32(0, dest++);
    if ( code_dword_len > CDM_CODE_MEMORYn_DWLEN(0) )
        for ( i = 0; i < CDM_CODE_MEMORYn_DWLEN(1); i++ )
            IFX_REG_W32(0, dest++);

    /*  copy code   */
    dest = (volatile unsigned int *)SOC_ACCESS_VRX218_ADDR(CDM_CODE_MEMORY(pp32, 0), dev->phy_membase);
    while ( code_dword_len-- > 0 )
        IFX_REG_W32(*code_src++, dest++);

    /*  copy data   */
    dest = (volatile unsigned int *)SOC_ACCESS_VRX218_ADDR(CDM_DATA_MEMORY(pp32, 0), dev->phy_membase);
    while ( data_dword_len-- > 0 )
        IFX_REG_W32(*data_src++, dest++);

    return PPA_SUCCESS;
}

static int pp32_load(const ifx_pcie_ep_dev_t *dev)
{
    int ret;

    /*  download firmware   */
    ret = pp32_download_code(dev, 0, vrx218_a1plus_fw_code, NUM_ENTITY(vrx218_a1plus_fw_code), vrx218_a1plus_fw_data, NUM_ENTITY(vrx218_a1plus_fw_data));
    if ( ret != PPA_SUCCESS )
        return ret;

    return PPA_SUCCESS;
}

static int pp32_start(const ifx_pcie_ep_dev_t *dev)
{
    unsigned int mask = 0x00000001;

    /*  run PP32    */
    IFX_REG_W32_MASK(mask, 0, PP32_FREEZE(dev->phy_membase));

    /*  idle for a while to let PP32 init itself    */
    mdelay(1);

    return PPA_SUCCESS;
}

static void pp32_stop(const ifx_pcie_ep_dev_t *dev)
{
    unsigned int mask = 0x00010001;

    /*  halt PP32   */
    IFX_REG_W32_MASK(0, mask, PP32_FREEZE(dev->phy_membase));
}

static void setup_dfe_loopback(unsigned long base)
{
#ifdef CONFIG_VRX218_DSL_DFE_LOOPBACK
//  vrx218_dfe.c
extern unsigned long dfe_pcie_base_addr;
extern void dfe_zephyr_lb_init(u8 pcie_port, u8 arc_mode);
extern void set_dfe_data_rate(u8 pcie_port, u32 nBC_switches, u32 nBC0Bytes,  u32 nBC1Bytes, u32 numTimeSlots);
//  vrx218_ppe_atm_init.c
extern unsigned int in_sync(unsigned long base, u8 bc);

    int port = base == 0x18000000 ? 1 : 0;
    int i;

    printk("enable DFE loopback ...\n");

    dfe_pcie_base_addr = base;

    //set to link up mode
    *SOC_ACCESS_VRX218_SB(__DSL_LINK_DOWN,base) = 0;

    //Set DFE to Zephyr loopback mode
    dfe_zephyr_lb_init(port, 0);    // 0 - ARC core disable ; 1 - ARC core enable

    //Function to change DFE data rate
    // Bit_rate (MBps) = num_bc_switch * (bc0_payld + bc1_payld)  / ((num_time_slot + 1) * 28 us);
    // Bit_rate (Mbps) = 8 * num_bc_switch * (bc0_payld + bc1_payld)  / ((num_time_slot + 1) * 28 us);
    // set_dfe_data_rate(u8 pcie_port, UINT32 num_bc_switch, UINT32 bc0_payld, UINT32 bc1_payld, UINT32 num_time_slot)
    set_dfe_data_rate(port, 4, 178, 178, 1);

    for ( i = 0; i < 1000 && !in_sync(base, 0); i++ )
        udelay(10);
    printk(in_sync(base, 0) ? "BC0 is in sync\n" : "BC0 is not in sync\n");
    for ( i = 0; i < 1000 && !in_sync(base, 1); i++ )
        udelay(10);
    printk(in_sync(base, 1) ? "BC1 is in sync\n" : "BC1 is not in sync\n");

    //Don't keep idle for emulation, but must keep idle for real case.
    *SFSM_CFG0(base) &= ~(1 << 15);
    *SFSM_CFG1(base) &= ~(1 << 15);

    g_dbg_enable |= DBG_ENABLE_MASK_MAC_SWAP;
#endif
}

//  vrx218_ppe_atm_init.c
extern void vrx218_ppe_atm_init(const ifx_pcie_ep_dev_t *dev, int qsb_en);
extern void __exit vrx218_ppe_atm_fw_exit(const ifx_pcie_ep_dev_t *dev);
extern void __exit vrx218_ppe_atm_free_mem(const ifx_pcie_ep_dev_t *dev);
//  vrx218_datapath.c
extern int vrx218_atm_datapath_init(const ifx_pcie_ep_dev_t *p_vrx218_dev);
extern void vrx218_atm_datapath_exit(void);

static int __init init_vrx218_module(const ifx_pcie_ep_dev_t *dev)
{
    printk("port        = %d\n",     dev->phy_membase == 0x18000000 ? 1 : 0);
    printk("irq         = %u\n",     dev->irq);
    printk("membase     = 0x%08x\n", (unsigned int)dev->membase);
    printk("phy_membase = 0x%08x\n", dev->phy_membase);
    printk("peer_num    = %u\n",     dev->peer_num);

    /*
     *  init hardware
     */
    reset_ppe(dev->phy_membase);
    /*  activate VRX218 */
    enable_vrx218_ppe_ema(dev->phy_membase);
    /*  set VRX218 PPE clock 288MHz */
    set_vrx218_ppe_clk(288, dev->phy_membase);
    /* clear mailbox */
    *MBOX_IGU0_ISRC(dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU1_ISRC(dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU0_IER(dev->phy_membase) = 0x00;
    *MBOX_IGU1_IER(dev->phy_membase) = 0x00;
    /*  freeze PP32 */
    pp32_stop(dev);
    vrx218_ppe_atm_init(dev, 1);
    pp32_load(dev);

    /*
     *  update OAM base address
     */
    g_host_desc_base.ds_oam_des_base    = CPHYSADDR(SOC_ACCESS_VRX218_SB(__DS_OAM_DES_LIST_BASE, dev->phy_membase));
    g_host_desc_base.ds_oam_des_num     = SOC_DS_OAM_DES_NUM;

    /*
     *  init datapath
     */
    vrx218_atm_datapath_init(&g_vrx218_dev);

    /*
     *  start hardware
     */
    pp32_start(dev);
    setup_dfe_loopback(dev->phy_membase);

    /*
     *  enable mailbox
     */
    *MBOX_IGU0_IER(g_vrx218_dev.phy_membase) = 0x00;    /* not ready to implement flow control yet */
    *MBOX_IGU1_IER(g_vrx218_dev.phy_membase) = 0x0B;    /* not ready to implement swap queue, but enable DS and OAM */

    return PPA_SUCCESS;
}

static void __exit exit_vrx218_module(const ifx_pcie_ep_dev_t *dev)
{
    vrx218_atm_datapath_exit();

    vrx218_ppe_atm_fw_exit(dev);
    udelay(100);
    pp32_stop(dev);
    vrx218_ppe_atm_free_mem(dev);
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

static struct file_operations g_proc_file_dbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dbg_seq_open,
    .read       = seq_read,
    .write      = proc_write_dbg,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dbg, NULL);
}

static struct file_operations g_proc_file_mem_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_mem_seq_open,
    .read       = seq_read,
    .write      = proc_write_mem,
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
static struct file_operations g_proc_file_pp32_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_pp32_seq_open,
    .read       = seq_read,
    .write      = proc_write_pp32,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_pp32_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_pp32, NULL);
}

static struct file_operations g_proc_file_wanmib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_wanmib_seq_open,
    .read       = seq_read,
    .write      = proc_write_wanmib,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_wanmib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_wanmib, NULL);
}

static struct file_operations g_proc_file_queue_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_queue_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_queue_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_queue, NULL);
}
static struct file_operations g_proc_file_htu_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_htu_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_htu_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_htu, NULL);
}

static struct file_operations g_proc_file_prio_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_prio_seq_open,
    .read       = seq_read,
    .write      = proc_write_prio,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_prio_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_prio, NULL);
}

static struct file_operations g_proc_file_dsl_vlan_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dsl_vlan_seq_open,
    .read       = seq_read,
    .write      = proc_write_dsl_vlan,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_dsl_vlan_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dsl_vlan, NULL);
}

static struct file_operations g_proc_file_cell_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_cell_seq_open,
    .read       = seq_read,
    .write      = proc_write_cell,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_cell_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_cell, NULL);
}

extern int proc_read_cell(struct seq_file *seq, void *v)
{
    return 0;
}
static struct file_operations g_proc_file_fwdbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_fwdbg_seq_open,
    .read       = seq_read,
    .write      = proc_write_fwdbg,
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

static struct file_operations g_proc_file_powersaving_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_powersaving_seq_open,
    .read       = seq_read,
    .write      = proc_write_ps,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_powersaving_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ps, NULL);
}

static struct file_operations g_proc_file_pciereset_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_pciereset_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_pciereset_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_pcie_rst, NULL);
}

static void proc_file_create(void)
{
    struct proc_dir_entry *parent_proc_entry = NULL;
    struct proc_dir_entry *res = NULL;
    int i, j;

    for ( i = 0; i < NUM_ENTITY(g_proc_entry); i++ ) {
        if ( !g_proc_entry[i].is_enable )
            continue;

        g_proc_entry[i].parent_proc_entry = parent_proc_entry;
        if ( g_proc_entry[i].parent_name != NULL ) {
            for ( j = 0; j < i; j++ )
                if ( strcmp(g_proc_entry[j].name, g_proc_entry[i].parent_name) == 0 ) {
                    g_proc_entry[i].parent_proc_entry = g_proc_entry[j].proc_entry;
                    break;
                }
        }

        if ( g_proc_entry[i].is_dir ) {
            g_proc_entry[i].proc_entry = parent_proc_entry = proc_mkdir(g_proc_entry[i].name, g_proc_entry[i].parent_proc_entry);
            if ( g_proc_entry[i].proc_entry == NULL )
                printk("%s: Create proc directory \"%s\" failed!\n", __FUNCTION__, g_proc_entry[i].name);
        }
    }
    res  = proc_create("ver",S_IRUGO, parent_proc_entry, &g_proc_file_ver_seq_fops);
    res  = proc_create("dbg",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_dbg_seq_fops);
    res  = proc_create("mem",S_IWUSR, parent_proc_entry, &g_proc_file_mem_seq_fops);
    res  = proc_create("pp32",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_pp32_seq_fops);
    res  = proc_create("wanmib",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_wanmib_seq_fops);
    res  = proc_create("queue",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_queue_seq_fops);
    res  = proc_create("htu",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_htu_seq_fops);
    res  = proc_create("prio",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_prio_seq_fops);
    res  = proc_create("dsl_vlan",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_dsl_vlan_seq_fops);
    res  = proc_create("cell",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_cell_seq_fops);
    res  = proc_create("tfwdbg",S_IWUSR, parent_proc_entry, &g_proc_file_fwdbg_seq_fops);
    res  = proc_create("powersaving",S_IWUSR|S_IWUSR, parent_proc_entry, &g_proc_file_powersaving_seq_fops);
    res  = proc_create("pciereset",S_IWUSR, parent_proc_entry, &g_proc_file_pciereset_seq_fops);
}

static void proc_file_delete(void)
{
    int i;

    for ( i = NUM_ENTITY(g_proc_entry) - 1; i >= 0; i-- ) {
        remove_proc_entry(g_proc_entry[i].name, g_proc_entry[i].parent_proc_entry);
    }
}

static int proc_read_ver(struct seq_file *seq, void *v)
{
    int len = 0;

    print_fw_ver(seq, *FW_VER_ID);

    return len;
}

static int proc_read_dbg(struct seq_file *seq, void *v)
{
    int len = 0;

    seq_printf(seq, "error print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ERR)              ? "enabled" : "disabled");
    seq_printf(seq, "debug print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT)      ? "enabled" : "disabled");
    seq_printf(seq, "assert           - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ASSERT)           ? "enabled" : "disabled");
    seq_printf(seq, "dump rx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_RX)      ? "enabled" : "disabled");
    seq_printf(seq, "dump tx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_TX)      ? "enabled" : "disabled");
    seq_printf(seq, "dump qos         - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS)         ? "enabled" : "disabled");
    seq_printf(seq, "mac swap         - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_MAC_SWAP)         ? "enabled" : "disabled");

    return len;
}

static ssize_t proc_write_dbg(struct file *file, const char __user *buf, size_t count, loff_t *data)
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
        " dump qos",
        " qos",
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
        9,  4,
        9,  5,
        4
    };
    u32 dbg_enable_mask[] = {
        DBG_ENABLE_MASK_ERR,
        DBG_ENABLE_MASK_DEBUG_PRINT,
        DBG_ENABLE_MASK_ASSERT,
        DBG_ENABLE_MASK_DUMP_SKB_RX,
        DBG_ENABLE_MASK_DUMP_SKB_TX,
        DBG_ENABLE_MASK_DUMP_QOS,
        DBG_ENABLE_MASK_MAC_SWAP,
        DBG_ENABLE_MASK_ALL & ~DBG_ENABLE_MASK_MAC_SWAP
    };

    char str[128];
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

    if ( strncasecmp(p, "enable", 6) == 0 )
    {
        p += 6;
        f_enable = 1;
    }
    else if ( strncasecmp(p, "disable", 7) == 0 )
    {
        p += 7;
        f_enable = -1;
    }
    else if ( strncasecmp(p, "help", 4) == 0 || *p == '?' )
    {
        printk("echo <enable/disable> [err/dbg/assert/rx/tx/swap/all] > /proc/eth/vrx318/dbg\n");
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
                    if ( strncasecmp(p, dbg_enable_mask_str[i], dbg_enable_mask_str_len[i]) == 0 )
                    {
                        if ( f_enable > 0 )
                            g_dbg_enable |= dbg_enable_mask[i >> 1];
                        else
                            g_dbg_enable &= ~dbg_enable_mask[i >> 1];
                        p += dbg_enable_mask_str_len[i];

                        if(f_enable > 0 && (strncasecmp(dbg_enable_mask_str[i], " rx", 3) == 0 ||
                                            strncasecmp(dbg_enable_mask_str[i], " tx", 3) == 0)){

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

#define PP32_REG_ADDR_BEGIN     0x0
#define PP32_REG_ADDR_END       0x1FFF
#define PP32_SB_ADDR_BEGIN      0x2000
#define PP32_SB_ADDR_END        0xFFFF

static inline unsigned long sb_addr_to_fpi_addr_convert(unsigned long sb_addr)
{
    if ( sb_addr <= PP32_SB_ADDR_END ) {
        return (unsigned long )SB_BUFFER(sb_addr);
    }
    else {
        return sb_addr;
    }
}

static ssize_t proc_write_mem(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char *p1, *p2;
    int len;
    int colon;
    volatile unsigned long *p;
    unsigned long dword;
    char local_buf[128];
    int i, n, l;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    p2 = NULL;
    colon = 1;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( strcasecmp(p1, "w") == 0 || strcasecmp(p1, "write") == 0 || strcasecmp(p1, "r") == 0 || strcasecmp(p1, "read") == 0 )
            break;

        p1 = p2;
        colon = 1;
    }

    if ( *p1 == 'w' )
    {
        ignore_space(&p2, &len);
        p = (volatile unsigned long *)get_number(&p2, &len, 1);
        p = (volatile unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);
        p = (volatile unsigned long *)SOC_ACCESS_VRX218_ADDR(p, g_vrx218_dev.phy_membase);

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
        p = (volatile unsigned long *)get_number(&p2, &len, 1);
        p = (volatile unsigned long *)sb_addr_to_fpi_addr_convert( (unsigned long) p);
        p = (volatile unsigned long *)SOC_ACCESS_VRX218_ADDR(p, g_vrx218_dev.phy_membase);

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

static int proc_read_pp32(struct seq_file *seq, void *v)
{
    static const char *stron = " on";
    static const char *stroff = "off";

    int len = 0;
    int cur_context;
    int f_stopped;
    char str[256];
    char strlength;
    int i, j;

    int pp32;

    for ( pp32 = 0; pp32 < 2; pp32++ )
    {
        f_stopped = 0;

        seq_printf(seq, "===== pp32 core %d =====\n", pp32);

        if ( (*PP32_FREEZE(g_vrx218_dev.phy_membase) & (1 << (pp32 << 4))) != 0 )
        {
            sprintf(str, "freezed");
            f_stopped = 1;
        }
        else if ( PP32_CPU_USER_STOPPED(g_vrx218_dev.phy_membase, pp32) || PP32_CPU_USER_BREAKIN_RCV(g_vrx218_dev.phy_membase, pp32) || PP32_CPU_USER_BREAKPOINT_MET(g_vrx218_dev.phy_membase, pp32) )
        {
            strlength = 0;
            if ( PP32_CPU_USER_STOPPED(g_vrx218_dev.phy_membase, pp32) )
                strlength += sprintf(str + strlength, "stopped");
            if ( PP32_CPU_USER_BREAKPOINT_MET(g_vrx218_dev.phy_membase, pp32) )
                strlength += sprintf(str + strlength, strlength ? " | breakpoint" : "breakpoint");
            if ( PP32_CPU_USER_BREAKIN_RCV(g_vrx218_dev.phy_membase, pp32) )
                strlength += sprintf(str + strlength, strlength ? " | breakin" : "breakin");
            f_stopped = 1;
        }
        else if ( PP32_CPU_CUR_PC(g_vrx218_dev.phy_membase, pp32) == PP32_CPU_CUR_PC(g_vrx218_dev.phy_membase, pp32) )
        {
            unsigned int pc_value[64] = {0};

            f_stopped = 1;
            for ( i = 0; f_stopped && i < NUM_ENTITY(pc_value); i++ )
            {
                pc_value[i] = PP32_CPU_CUR_PC(g_vrx218_dev.phy_membase, pp32);
                for ( j = 0; j < i; j++ )
                    if ( pc_value[j] != pc_value[i] )
                    {
                        f_stopped = 0;
                        break;
                    }
            }
            if ( f_stopped )
                sprintf(str, "hang");
        }
        if ( !f_stopped )
            sprintf(str, "running");
        cur_context = PP32_BRK_CUR_CONTEXT(g_vrx218_dev.phy_membase, pp32);
        seq_printf(seq, "Context: %d, PC: 0x%04x, %s\n", cur_context, PP32_CPU_CUR_PC(g_vrx218_dev.phy_membase, pp32), str);

        if ( PP32_CPU_USER_BREAKPOINT_MET(g_vrx218_dev.phy_membase, pp32) )
        {
            strlength = 0;
            if ( PP32_BRK_PC_MET(g_vrx218_dev.phy_membase, pp32, 0) )
                strlength += sprintf(str + strlength, "pc0");
            if ( PP32_BRK_PC_MET(g_vrx218_dev.phy_membase, pp32, 1) )
                strlength += sprintf(str + strlength, strlength ? " | pc1" : "pc1");
            if ( PP32_BRK_DATA_ADDR_MET(g_vrx218_dev.phy_membase, pp32, 0) )
                strlength += sprintf(str + strlength, strlength ? " | daddr0" : "daddr0");
            if ( PP32_BRK_DATA_ADDR_MET(g_vrx218_dev.phy_membase, pp32, 1) )
                strlength += sprintf(str + strlength, strlength ? " | daddr1" : "daddr1");
            if ( PP32_BRK_DATA_VALUE_RD_MET(g_vrx218_dev.phy_membase, pp32, 0) )
            {
                strlength += sprintf(str + strlength, strlength ? " | rdval0" : "rdval0");
                if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(g_vrx218_dev.phy_membase, pp32, 0) )
                {
                    if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_vrx218_dev.phy_membase, pp32, 0) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_vrx218_dev.phy_membase, pp32, 0) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_RD_MET(g_vrx218_dev.phy_membase, pp32, 1) )
            {
                strlength += sprintf(str + strlength, strlength ? " | rdval1" : "rdval1");
                if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(g_vrx218_dev.phy_membase, pp32, 1) )
                {
                    if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_vrx218_dev.phy_membase, pp32, 1) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_vrx218_dev.phy_membase, pp32, 1) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_WR_MET(g_vrx218_dev.phy_membase, pp32, 0) )
            {
                strlength += sprintf(str + strlength, strlength ? " | wtval0" : "wtval0");
                if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(g_vrx218_dev.phy_membase, pp32, 0) )
                {
                    if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_vrx218_dev.phy_membase, pp32, 0) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_vrx218_dev.phy_membase, pp32, 0) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_WR_MET(g_vrx218_dev.phy_membase, pp32, 1) )
            {
                strlength += sprintf(str + strlength, strlength ? " | wtval1" : "wtval1");
                if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(g_vrx218_dev.phy_membase, pp32, 1) )
                {
                    if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_vrx218_dev.phy_membase, pp32, 1) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_vrx218_dev.phy_membase, pp32, 1) )
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
                    seq_printf(seq, "   %2d: %08x", i + j * 4, *PP32_GP_CONTEXTi_REGn(g_vrx218_dev.phy_membase, pp32, cur_context, i + j * 4));
                seq_printf(seq, "\n");
            }
        }

        seq_printf(seq, "break out on: break in - %s, stop - %s\n",
                                            PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN(g_vrx218_dev.phy_membase, pp32) ? stron : stroff,
                                            PP32_CTRL_OPT_BREAKOUT_ON_STOP(g_vrx218_dev.phy_membase, pp32) ? stron : stroff);
        seq_printf(seq, "     stop on: break in - %s, break point - %s\n",
                                            PP32_CTRL_OPT_STOP_ON_BREAKIN(g_vrx218_dev.phy_membase, pp32) ? stron : stroff,
                                            PP32_CTRL_OPT_STOP_ON_BREAKPOINT(g_vrx218_dev.phy_membase, pp32) ? stron : stroff);
        seq_printf(seq, "breakpoint:\n");
        seq_printf(seq, "     pc0: 0x%08x, %s\n", *PP32_BRK_PC(g_vrx218_dev.phy_membase, pp32, 0), PP32_BRK_GRPi_PCn(g_vrx218_dev.phy_membase, pp32, 0, 0) ? "group 0" : "off");
        seq_printf(seq, "     pc1: 0x%08x, %s\n", *PP32_BRK_PC(g_vrx218_dev.phy_membase, pp32, 1), PP32_BRK_GRPi_PCn(g_vrx218_dev.phy_membase, pp32, 1, 1) ? "group 1" : "off");
        seq_printf(seq, "  daddr0: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(g_vrx218_dev.phy_membase, pp32, 0), PP32_BRK_GRPi_DATA_ADDRn(g_vrx218_dev.phy_membase, pp32, 0, 0) ? "group 0" : "off");
        seq_printf(seq, "  daddr1: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(g_vrx218_dev.phy_membase, pp32, 1), PP32_BRK_GRPi_DATA_ADDRn(g_vrx218_dev.phy_membase, pp32, 1, 1) ? "group 1" : "off");
        seq_printf(seq, "  rdval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(g_vrx218_dev.phy_membase, pp32, 0));
        seq_printf(seq, "  rdval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(g_vrx218_dev.phy_membase, pp32, 1));
        seq_printf(seq, "  wrval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(g_vrx218_dev.phy_membase, pp32, 0));
        seq_printf(seq, "  wrval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(g_vrx218_dev.phy_membase, pp32, 1));
    }


    return len;
}

static ssize_t proc_write_pp32(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[128];
    char *p;
    u32 addr;

    int len, rlen;

    int pp32 = 0;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    if ( strncasecmp(p, "pp32 ", 5) == 0 )
    {
        p += 5;
        rlen -= 5;

        while ( rlen > 0 && *p >= '0' && *p <= '9' )
        {
            pp32 += *p - '0';
            p++;
            rlen--;
        }
        while ( rlen > 0 && *p && *p <= ' ' )
        {
            p++;
            rlen--;
        }
    }

    if ( strcasecmp(p, "restart") == 0 )
        *PP32_FREEZE(g_vrx218_dev.phy_membase) &= ~(1 << (pp32 << 4));
    else if ( strcasecmp(p, "freeze") == 0 )
        *PP32_FREEZE(g_vrx218_dev.phy_membase) |= 1 << (pp32 << 4);
    else if ( strcasecmp(p, "start") == 0 )
        *PP32_CTRL_CMD(g_vrx218_dev.phy_membase, pp32) = PP32_CTRL_CMD_RESTART;
    else if ( strcasecmp(p, "stop") == 0 )
        *PP32_CTRL_CMD(g_vrx218_dev.phy_membase, pp32) = PP32_CTRL_CMD_STOP;
    else if ( strcasecmp(p, "step") == 0 )
        *PP32_CTRL_CMD(g_vrx218_dev.phy_membase, pp32) = PP32_CTRL_CMD_STEP;
    else if ( strncasecmp(p, "pc0 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_PCn_OFF(0, 0);
            *PP32_BRK_PC_MASK(g_vrx218_dev.phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(g_vrx218_dev.phy_membase, pp32, 0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(g_vrx218_dev.phy_membase, pp32, 0) = addr;
            *PP32_BRK_PC_MASK(g_vrx218_dev.phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_PCn_ON(0, 0);
        }
    }
    else if ( strncasecmp(p, "pc1 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_PCn_OFF(1, 1);
            *PP32_BRK_PC_MASK(g_vrx218_dev.phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(g_vrx218_dev.phy_membase, pp32, 1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(g_vrx218_dev.phy_membase, pp32, 1) = addr;
            *PP32_BRK_PC_MASK(g_vrx218_dev.phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_PCn_ON(1, 1);
        }
    }
    else if ( strncasecmp(p, "daddr0 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_OFF(0, 0);
            *PP32_BRK_DATA_ADDR_MASK(g_vrx218_dev.phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(g_vrx218_dev.phy_membase, pp32, 0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(g_vrx218_dev.phy_membase, pp32, 0) = addr;
            *PP32_BRK_DATA_ADDR_MASK(g_vrx218_dev.phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_ON(0, 0);
        }
    }
    else if ( strncasecmp(p, "daddr1 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_OFF(1, 1);
            *PP32_BRK_DATA_ADDR_MASK(g_vrx218_dev.phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(g_vrx218_dev.phy_membase, pp32, 1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(g_vrx218_dev.phy_membase, pp32, 1) = addr;
            *PP32_BRK_DATA_ADDR_MASK(g_vrx218_dev.phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_ON(1, 1);
        }
    }
    else
    {

        printk("echo \"<command>\" > /proc/eth/vrx318/pp32\n");
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

    if ( *PP32_BRK_TRIG(g_vrx218_dev.phy_membase, pp32) )
        *PP32_CTRL_OPT(g_vrx218_dev.phy_membase, pp32) = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON;
    else
        *PP32_CTRL_OPT(g_vrx218_dev.phy_membase, pp32) = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF;

    return count;
}

static int proc_read_wanmib(struct seq_file *seq, void *v)
{
    int len = 0;
    int i;

    seq_printf(seq, "DSL WAN MIB:\n");
    seq_printf(seq, "  wrx_drophtu_cell: %u\n", DSL_WAN_MIB_TABLE->wrx_drophtu_cell);
    seq_printf(seq, "  wrx_dropdes_pdu:  %u\n", DSL_WAN_MIB_TABLE->wrx_dropdes_pdu);
    seq_printf(seq, "  wrx_correct_pdu:  %u\n", DSL_WAN_MIB_TABLE->wrx_correct_pdu);
    seq_printf(seq, "  wrx_err_pdu:      %u\n", DSL_WAN_MIB_TABLE->wrx_err_pdu);
    seq_printf(seq, "  wrx_dropdes_cell: %u\n", DSL_WAN_MIB_TABLE->wrx_dropdes_cell);
    seq_printf(seq, "  wrx_correct_cell: %u\n", DSL_WAN_MIB_TABLE->wrx_correct_cell);
    seq_printf(seq, "  wrx_err_cell:     %u\n", DSL_WAN_MIB_TABLE->wrx_err_cell);
    seq_printf(seq, "  wrx_total_byte:   %u\n", DSL_WAN_MIB_TABLE->wrx_total_byte);
    seq_printf(seq, "  wtx_total_pdu:    %u\n", DSL_WAN_MIB_TABLE->wtx_total_pdu);
    seq_printf(seq, "  wtx_total_cell:   %u\n", DSL_WAN_MIB_TABLE->wtx_total_cell);
    seq_printf(seq, "  wtx_total_byte:   %u\n", DSL_WAN_MIB_TABLE->wtx_total_byte);
    seq_printf(seq, "DSL RX QUEUE MIB:\n");
    seq_printf(seq, "  idx     pdu       bytes\n");
    for ( i = 0; i < 16; i++ )
        seq_printf(seq, "   %2d %10u %10u\n", i, DSL_QUEUE_RX_MIB_TABLE(i)->pdu, DSL_QUEUE_RX_MIB_TABLE(i)->bytes);
    seq_printf(seq, "DSL TX QUEUE MIB:\n");
    seq_printf(seq, "  idx     pdu       bytes\n");
    for ( i = 0; i < 16; i++ )
        seq_printf(seq, "   %2d %10u %10u\n", i, DSL_QUEUE_TX_MIB_TABLE(i)->pdu, DSL_QUEUE_TX_MIB_TABLE(i)->bytes);


    return len;
}

static ssize_t proc_write_wanmib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[32];
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

    if ( strcasecmp(p, "clear") == 0 || strcasecmp(p, "clean") == 0 ) {
        memset((void *)DSL_WAN_MIB_TABLE, 0, sizeof(struct dsl_wan_mib_table));
        memset((void *)DSL_QUEUE_RX_MIB_TABLE(0), 0, sizeof(struct dsl_queue_mib) * 16);
        memset((void *)DSL_QUEUE_TX_MIB_TABLE(0), 0, sizeof(struct dsl_queue_mib) * 16);
    }

    return count;
}

static int proc_read_ps(struct seq_file *seq, void *v)
{
    int len = 0;
    u32 base = g_vrx218_dev.phy_membase;
    volatile struct psave_cfg *ps_cfg = PSAVE_CFG(base);

    seq_printf(seq, "Power Saving: %s\n", ps_cfg->sleep_en == 1 ? "Enable" : "Disable");

    return len;
}

static ssize_t proc_write_ps(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[128];
    char *p;
    int len, rlen;
    u32 base = g_vrx218_dev.phy_membase;
    volatile struct psave_cfg *ps_cfg = PSAVE_CFG(base);

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return count;

    if ( strcasecmp(p, "disable") == 0 ){
        ps_cfg->sleep_en = 0;
    }
    else if( strcasecmp(p, "enable") == 0 ){
        ps_cfg->sleep_en = 1;
    }

    return count;
}

static int proc_read_queue(struct seq_file *seq, void *v)
{
    static const char *mpoa_type_str[] = {"EoA w/o FCS", "EoA w FCS", "PPPoA", "IPoA"};

    wrx_queue_config_t rx;
    wtx_queue_config_t tx;
    char qmap_str[64];
    char qmap_flag;
    int qmap_str_len;
    int i, k;
    unsigned int bit;


    seq_printf(seq, "RX Queue Config (0x%08X):\n", (u32)WRX_QUEUE_CONFIG(0));

    for ( i = 0; i < 16; i++ )
    {
        rx = *WRX_QUEUE_CONFIG(i);
        seq_printf(seq       ,  "  %d: MPoA type - %s, MPoA mode - %s, IP version %d\n", i, mpoa_type_str[rx.mpoa_type], rx.mpoa_mode ? "LLC" : "VC mux", rx.ip_ver ? 6 : 4);
        seq_printf(seq,  "     Oversize - %d, Undersize - %d, Max Frame size - %d\n", rx.oversize, rx.undersize, rx.mfs);
        seq_printf(seq,  "     uu mask - 0x%02X, cpi mask - 0x%02X, uu exp - 0x%02X, cpi exp - 0x%02X\n", rx.uumask, rx.cpimask, rx.uuexp, rx.cpiexp);
        if ( rx.vlan_ins )
            seq_printf(seq, "     new_vlan = 0x%08X\n", rx.new_vlan);

    }

    seq_printf(seq, "TX Queue Config (0x%08X):\n", (u32)WTX_QUEUE_CONFIG(0));

    for ( i = 0; i < 15; i++ )
    {
        tx = *WTX_QUEUE_CONFIG(i);
        qmap_flag = 0;
        qmap_str_len = 0;
        for ( k = 0, bit = 1; k < 15; k++, bit <<= 1 )
            if ( (tx.same_vc_qmap & bit) )
            {
                if ( qmap_flag++ )
                    qmap_str_len += sprintf(qmap_str + qmap_str_len, ", ");
                qmap_str_len += sprintf(qmap_str + qmap_str_len, "%d", k);
            }
        seq_printf(seq       , "  %d: uu - 0x%02X, cpi - 0x%02X, same VC queue map - %s\n", i, tx.uu, tx.cpi, qmap_flag ? qmap_str : "null");
        seq_printf(seq,  "     bearer channel - %d, QSB ID - %d, MPoA mode - %s\n", tx.sbid, tx.qsb_vcid, tx.mpoa_mode ? "LLC" : "VC mux");
        seq_printf(seq,  "     ATM header - 0x%08X\n", tx.atm_header);

    }

    return 0;
}

static int print_htu(struct seq_file *seq, int i)
{
    int len = 0;

    if ( HTU_ENTRY(i)->vld )
    {
        seq_printf(seq, "%2d. valid\n", i);
        seq_printf(seq,  "    entry  0x%08x - pid %01x vpi %02x vci %04x pti %01x\n", *(u32*)HTU_ENTRY(i), HTU_ENTRY(i)->pid, HTU_ENTRY(i)->vpi, HTU_ENTRY(i)->vci, HTU_ENTRY(i)->pti);
        seq_printf(seq,  "    mask   0x%08x - pid %01x vpi %02x vci %04x pti %01x\n", *(u32*)HTU_MASK(i), HTU_MASK(i)->pid_mask, HTU_MASK(i)->vpi_mask, HTU_MASK(i)->vci_mask, HTU_MASK(i)->pti_mask);
        seq_printf(seq,  "    result 0x%08x - type: %s, qid: %d", *(u32*)HTU_RESULT(i), HTU_RESULT(i)->type ? "cell" : "AAL5", HTU_RESULT(i)->qid);
        if ( HTU_RESULT(i)->type )
            seq_printf(seq,  ", cell id: %d, verification: %s", HTU_RESULT(i)->cellid, HTU_RESULT(i)->ven ? "on" : "off");
        seq_printf(seq,  "\n");
    }
    else
        seq_printf(seq, "%2d. invalid\n", i);

    return len;
}

static int proc_read_htu(struct seq_file *seq, void *v)
{
    int htuts = *CFG_WRX_HTUTS;
    int i;


    for ( i = 0; i < htuts; i++ )
    {
        print_htu(seq, i);
    }

    return 0;
}

/*
 *  Datapath/HAL Hook Function
 */
static int32_t ppe_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    switch ( cmd )
    {
    case PPA_GENERIC_DATAPATH_GET_PPE_VERION:
    case PPA_GENERIC_HAL_GET_PPE_FW_VERSION:
        {
            PPA_VERSION *version = (PPA_VERSION *)buffer;
            struct fw_ver_id ver;

            if ( version->index > 0 )
                return PPA_FAILURE;

            ver = *FW_VER_ID;
            version->family = ver.family;
            version->type   = ver.package;
            version->major  = ver.major;
            version->mid    = ver.middle;
            version->minor  = ver.minor;
            return PPA_SUCCESS;
        }
    case PPA_GENERIC_HAL_GET_DSL_MIB:
    case PPA_GENERIC_HAL_CLEAR_DSL_MIB:
        //  DSL_QUEUE_TX_DROP_MIB_TABLE is not defined in VRX218 A1+
        return PPA_FAILURE;
    default:
        return PPA_FAILURE;
    }
}



/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

static void __init init_host_desc(void)
{
    volatile unsigned long *p;
    unsigned int i;

    p = (volatile unsigned long *)KSEG1ADDR(SOC_US_FASTPATH_DES_BASE);
    for ( i = 0; i < SOC_US_FASTPATH_DES_NUM; i++ ) {
        *p = (*p & ~0xF0000000) | 0x80000000;
        p += 2;
    }

    p = (volatile unsigned long *)KSEG1ADDR(SOC_US_CPUPATH_DES_BASE);
    for ( i = 0; i < SOC_US_CPUPATH_DES_NUM; i++ ) {
        *p = 0x30000000;
        p += 2;
    }

    p = (volatile unsigned long *)KSEG1ADDR(SOC_DS_DES_BASE);
    for ( i = 0; i < SOC_DS_DES_NUM; i++ ) {
        *p++ = 0xB0000000;
        *p++ = 0x00000000;
    }
}

static int __init ppe_init(void)
{
    extern int32_t register_ppe_ext_generic_hook(ppe_generic_hook_t datapath_hook, ppe_generic_hook_t hal_hook);

    int dev_num;
    int i;

    printk("VRX318: loading PPE driver and firmware (ATM)\n");

    g_host_desc_base.us_fastpath_des_num = SOC_US_FASTPATH_DES_NUM;
    g_host_desc_base.us_cpupath_des_num  = SOC_US_CPUPATH_DES_NUM;
    g_host_desc_base.ds_des_num          = SOC_DS_DES_NUM;
    g_host_desc_base.ds_oam_des_num      = 0;
    if ( vrx218_get_desc_mem_base(&g_host_desc_base) ) {
        printk("%s failed to get host descriptor base\n", __func__);
        return -ENOMEM;
    }
    init_host_desc();

    if ( ifx_pcie_ep_dev_num_get(&dev_num) ) {
        printk("%s failed to get total device number\n", __func__);
        return -EIO;
    }

    init_local_variables();

    for ( i = 0; i < dev_num; i++ ) {
        if ( ifx_pcie_ep_dev_info_req(i, IFX_PCIE_EP_INT_PPE, &g_vrx218_dev) ) {
            printk("%s failed to get pcie ep %d information\n", __func__, i);
            continue;
        }

        g_vrx218_idx = i;

        /* init hardware */
        init_vrx218_module(&g_vrx218_dev);

        break;
    }

    register_ppe_ext_generic_hook(ppe_generic_hook, ppe_generic_hook);
    xet_phy_wan_port(7,NULL,0,1);

    proc_file_create();

    printk("  %d device(s) are detected!\n", dev_num);

    return 0;
}

static void __exit ppe_exit(void)
{
    extern int32_t register_ppe_ext_generic_hook(ppe_generic_hook_t datapath_hook, ppe_generic_hook_t hal_hook);

    proc_file_delete();

    register_ppe_ext_generic_hook(NULL, NULL);

    if ( g_vrx218_idx >= 0 ) {
        exit_vrx218_module(&g_vrx218_dev);

        ifx_pcie_ep_dev_info_release(g_vrx218_idx);
        g_vrx218_idx = -1;
    }
}

module_init(ppe_init);
module_exit(ppe_exit);

MODULE_LICENSE("GPL");
