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
#include "vrx_ppe.h"
#include "vrx218_ppe_ptm_tc_ds.h"
#include "vrx218_ppe_fw_ds.h"
#include "vrx218_ppe_bonding_ds.h"
#include "unified_qos_ds_be.h"
#include "vrx218_e1_addr_def.h"
#include "vrx218_ppe_fw_const.h"
#include "vrx218_e1.h"
#include "vrx218_pp32_1.h"
#include "vrx218_ptm_common.h"

/*
 * ####################################
 *   Parameters to Configure PPE
 * ####################################
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
  #define MODULE_PARM_ARRAY(a, b)   module_param_array(a, int, NULL, 0)
  #define MODULE_PARM(a, b)         module_param(a, int, 0)
#else
  #define MODULE_PARM_ARRAY(a, b)   MODULE_PARM(a, b)
#endif

static int queue_gamma_map[4] = {0x00FF, 0x0000, 0x0000, 0x0000};
static int qos_queue_len[8] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0}; 

MODULE_PARM_ARRAY(queue_gamma_map, "4-4i");
MODULE_PARM_DESC(queue_gamma_map, "TX QoS queues mapping to 4 TX Gamma interfaces.");

MODULE_PARM_ARRAY(qos_queue_len, "1-8i");
MODULE_PARM_DESC(qos_queue_len, "QoS queue's length for each QoS queue");

/*
 * ####################################
 *              Definition
 * ####################################
 */
#define DEBUG_BONDING_PROC                      1

/*
 *  Bonding Definition
 */
#define L1_PTM_TC_BOND_MODE                     0
#define L2_ETH_TRUNK_MODE                       1
#define LINE_NUMBER                             2
#define BEARER_CHANNEL_PER_LINE                 2

/*
 *  Firmware Registers
 */
#define FW_VER_ID(base)                         ((volatile struct fw_ver_id *)SOC_ACCESS_VRX218_SB(__FW_VER_ID, base))
#define STD_DES_CFG(base)                       ((volatile std_des_cfg_t *)SOC_ACCESS_VRX218_SB(__STD_DES_CFG, base))
#define RX_GAMMA_ITF_CFG(i, base)               ((volatile struct rx_gamma_itf_cfg *)SOC_ACCESS_VRX218_SB(__RX_GIF0_CFG_STATS_CFG + (i) * sizeof(struct rx_gamma_itf_cfg) / sizeof(unsigned int), base))    /*  i < 4   */
#define DS_BOND_LL_CTXT(i, base)                ((volatile ds_bond_gif_ll_ctxt_t *)SOC_ACCESS_VRX218_SB(__DS_BOND_LL_CTXT_BASE + (i) * sizeof(ds_bond_gif_ll_ctxt_t) / sizeof(unsigned int), base))         /*  i < 9   */

/*
 *  Bonding Registers
 */
#define BOND_CONF(base)                         ((volatile bond_conf_t *)SOC_ACCESS_VRX218_SB(__BOND_CONF, base))

/*
 *  RCU Registers
 */
#define RST_REQ(base)                           SOC_ACCESS_VRX218_ADDR(0x1E002010, base)
#define RST_STAT(base)                          SOC_ACCESS_VRX218_ADDR(0x1E002000, base)
#define RST_DBGR(base)                          SOC_ACCESS_VRX218_ADDR(0x1E002050, base)

/*
  * MEI Registers
*/
#define VRX218_DSL_MEI_BASE                     0x1E116000
#define VRX218_DSP_MGMT_REG_BASE                0x0000
#define VRX218_DSP_MGMT_REG(r)                  (VRX218_DSP_MGMT_REG_BASE + (r))
#define VRX218_DSL_MEI_REG(r)                   (VRX218_DSL_MEI_BASE + VRX218_DSP_MGMT_REG(r))
#define VRX218_MEI_DBG_MASTER(base)             SOC_ACCESS_VRX218_ADDR(VRX218_DSL_MEI_REG(0x0020),base)
#define VRX218_MEI_DBG_PORT_SEL(base)           SOC_ACCESS_VRX218_ADDR(VRX218_DSL_MEI_REG(0x0028),base)
#define VRX218_MEI_DBG_DECODE(base)             SOC_ACCESS_VRX218_ADDR(VRX218_DSL_MEI_REG(0x0024),base)
#define VRX218_MEI_XMEM_BAR(id,base)            SOC_ACCESS_VRX218_ADDR(VRX218_DSL_MEI_REG(0x0058 + id * 4),base)

/*
 *  PMU Registers
 */
#define PMU_PWDCR(base)                         SOC_ACCESS_VRX218_ADDR(0x1E00311C, base)
#define PMU_SR(base)                            SOC_ACCESS_VRX218_ADDR(0x1E003120, base)

/*
 *  CGU Registers
 */
#define CGU_CLKFSR(base)                        SOC_ACCESS_VRX218_ADDR(0x1E003010, base)

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

//  MIB counter
#define RECEIVE_NON_IDLE_CELL_CNT(i,base)            SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x34A0 + (i)), base)
#define RECEIVE_IDLE_CELL_CNT(i,base)                SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x34A2 + (i)), base)
#define TRANSMIT_CELL_CNT(i,base)                    SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x34A4 + (i)), base)

#define WAN_RX_MIB_TABLE(i, base)                    ((volatile struct wan_rx_mib_table *)       SOC_ACCESS_VRX218_ADDR(SB_BUFFER(__RX_GIF_MIB_BASE + (i) * 8), base)) /*  i < 4   */
#define WAN_QOS_MIB_TABLE(i, base)                   ((volatile qosq_mib_t *)                    SOC_ACCESS_VRX218_ADDR(SB_BUFFER(__QOSQ_MIB_BASE   + (i) * 8), base)) /*  i < 16   */
#define WAN_TX_GIF_CFG(i,base)                       ((volatile desq_cfg_ctxt_t *)               SOC_ACCESS_VRX218_ADDR(SB_BUFFER(__US_TC_LOCAL_Q_CFG_CTXT_BASE  + (i)* 8), base)) /* i < 4 */

//bonding MIB
#define US_BOND_GIF_MIB(i, base)                     SOC_ACCESS_VRX218_SB((__US_E1_FRAG_Q_FRAG_MIB + i), base)  /* i < 8 */
#define DS_BOND_GIF_MIB(i, base)                     ((volatile ds_bond_gif_mib_t *)              SOC_ACCESS_VRX218_SB((__DS_BOND_GIF_MIB + i * 16),base))  /* i < 8 */
#define DS_BG_MIB(i,base)                            ((volatile ds_bg_mib_t *)                    SOC_ACCESS_VRX218_SB((__DS_BG_MIB + i * 16), base))       /* i < 4 */

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

#define SFSM_CFG(i,base)                                ((volatile struct SFSM_cfg *)  SOC_ACCESS_VRX218_SB(0x0416 + (i),base))
#define SFSM_DBA(i,base)                                ((volatile struct SFSM_dba *)  SOC_ACCESS_VRX218_SB(0x0412 + (i),base))
#define SFSM_CBA(i,base)                                ((volatile struct SFSM_cba *)  SOC_ACCESS_VRX218_SB(0x0414 + (i),base))
#define FFSM_DBA(i,base)                                ((volatile struct FFSM_dba *)  SOC_ACCESS_VRX218_SB(0x0508 + (i),base))
#define FFSM_CFG(i,base)                                ((volatile struct FFSM_cfg *)  SOC_ACCESS_VRX218_SB(0x050A + (i),base))



#define VRX218_QOSQ_NUM                         8
#define VRX218_GIF_NUM                          8

/*Bonding Definition */
#define L1_PTM_TC_BOND_MODE                     0
#define L2_ETH_TRUNK_MODE                       1


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

/*
 *  DSL Status Script Notification
 */
static void switch_to_eth_trunk(void);
static void switch_to_ptmtc_bonding(void);
static void update_max_delay(int);
static int dsl_proc_read_status(struct seq_file *, void *);
static ssize_t dsl_proc_write_status(struct file *, const char __user *, size_t, loff_t *);

/*
 *  Proc File
 */
static void proc_file_enable(char *, int);
static void proc_file_create(void);
static void proc_file_delete(void);
static int proc_read_ver(struct seq_file *, void *);
static int proc_read_qos(struct seq_file *, void *);
static ssize_t proc_write_qos(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_vector_prio(struct seq_file *, void *);
static ssize_t proc_write_vector_prio(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_dbg(struct seq_file *, void *);
static ssize_t proc_write_dbg(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_mem(struct seq_file *, void *);
static ssize_t proc_write_mem(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_pp32(struct seq_file *, void *);
static ssize_t proc_write_pp32(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_dev(struct seq_file *, void *);
static ssize_t proc_write_dev(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_wanmib(struct seq_file *, void *);
static ssize_t proc_write_wanmib(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_ps(struct seq_file *, void *);
static ssize_t proc_write_ps(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_bonding(struct seq_file *, void *);
static int proc_read_bndmib(struct seq_file *, void *);
static ssize_t proc_write_bndmib(struct file *, const char __user *, size_t, loff_t *);
extern int proc_read_pcie_rst(struct seq_file *, void *);


static void fwdbg_help(char **, int);
static void fw_dbg_start(char *);
static ssize_t proc_write_fwdbg(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_fwdbg(struct seq_file *, void *);


static int proc_read_ver_seq_open(struct inode *, struct file *);
static int proc_read_wanmib_seq_open(struct inode *, struct file *);
static int proc_read_vector_prio_seq_open(struct inode *, struct file *);
static int proc_read_dbg_seq_open(struct inode *, struct file *);
static int proc_read_mem_seq_open(struct inode *, struct file *);
static int proc_read_pp32_seq_open(struct inode *, struct file *);
static int proc_read_qos_seq_open(struct inode *, struct file *);
static int proc_read_fwdbg_seq_open(struct inode *, struct file *);
static int proc_read_dev_seq_open(struct inode *, struct file *);
static int proc_read_bonding_seq_open(struct inode *, struct file *);
static int proc_read_bndmib_seq_open(struct inode *, struct file *);
static int proc_read_pciereset_seq_open(struct inode *, struct file *);
static int proc_read_status_seq_open(struct inode *, struct file *);

extern void init_qos_queue(int* qos_queue_len);
extern void (*ltq_vectoring_priority_hook)(uint32_t priority);


static int g_vector_prio = 0;

/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static int g_dsl_bonding = 0;   //  default: single line
int g_queue_gamma_map[4] = {0};

extern int g_showtime;
static int g_line_showtime[LINE_NUMBER] = {0};
static unsigned int g_datarate_us[LINE_NUMBER * BEARER_CHANNEL_PER_LINE] = {0};
static unsigned int g_datarate_ds[LINE_NUMBER * BEARER_CHANNEL_PER_LINE] = {0};
static unsigned int g_max_delay[LINE_NUMBER] = {0};

//  single line: 0 - device,                    1 - reserved
//  bonding:     0 - VRX218_US_BONDING_MASTER,  1 - VRX218_DS_BONDING_MASTER
static ifx_pcie_ep_dev_t g_vrx218_dev[2] = {{0}};
static int g_vrx218_idx[2] = {-1, -1};
//  pointer to device in single line case and upstream bonding master in bonding case
ifx_pcie_ep_dev_t * const g_us_vrx218_dev = &g_vrx218_dev[VRX218_US_BONDING_MASTER];
//  pointer to device in single line case and downstream bonding master in bonding case
//  to be initialized after device enumateration
static ifx_pcie_ep_dev_t *g_ds_vrx218_dev = NULL;
//  pointer for random device access (usually via proc filesystem)
static ifx_pcie_ep_dev_t *g_cur_vrx218_dev = &g_vrx218_dev[VRX218_US_BONDING_MASTER];

static unsigned long g_bonding_sync_buf_page = 0;

/*  if no parent folder name provided, register function will take the nearest proc_dir_entry by default.
    if no proc_dir_entry found, by default, it's NULL. (e.g. create folder eth )
    Please make sure the file follow the right dir entry otherwise it would cause trouble to release it
 */

	static struct proc_entry_cfg g_proc_entry[] = {
     // Parent dir,    Name,    IS_DIR,   RD_fn,     WR_fn,     Enable
        {NULL,  "dsl_tc",		1, NULL,  NULL,		1 },
        {NULL,  "status",      		0, NULL,  NULL,		1 },
        {"proc",  "eth/vrx318",		1, NULL,  NULL,		1 },
        {NULL,  "ver",      		0, NULL,  NULL,		1 },
        {NULL,  "dbg",      		0, NULL,  NULL,       	1 },
        {NULL,  "mem",      		0, NULL,  NULL,       	1 },
        {NULL,  "pp32",     		0, NULL,  NULL,      	1 },
        {NULL,"qos",			0, NULL,  NULL, 	1 },
        {NULL,  "tfwdbg",   		0, NULL,  NULL,     	1 },
        {NULL,"dev",			0, NULL,  NULL, 	1 },
        {NULL,"mib",			0, NULL,  NULL, 	1 },
        {NULL,"vector_prio",	0, NULL,  NULL, 	1 },
#if defined(DEBUG_BONDING_PROC) && DEBUG_BONDING_PROC
        {NULL,"bonding",		0, NULL,  NULL, 	1 },
        {NULL,"bondingmib",		0, NULL,  NULL, 	1 },
#endif
        {NULL,"pciereset",		0, NULL,  NULL, 	1 },
	{NULL,"powersaving",		0, NULL,  NULL,		1 },
};

/*
    hardware register value or FW context value, cannot be cleared.
    Driver cached the original value when user want to clear them.
*/
static unsigned int last_wrx_total_pdu[2][4] = {{0}};
static unsigned int last_wrx_crc_error_total_pdu[2][4] = {{0}};
static unsigned int last_wrx_cv_cw_cnt[2][4] = {{0}};
static unsigned int last_wrx_bc_overdrop_cnt[2][2] = {{0}};
static unsigned int last_wtx_total_pdu[2][4] = {{0}};
static unsigned int last_wtx_total_bytes[2][4] = {{0}};


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
    int i, j;

    for ( i = 0; i < NUM_ENTITY(g_queue_gamma_map); i++ )
    {
        g_queue_gamma_map[i] = queue_gamma_map[i] & ((1 << /* g_wanqos_en */ 8) - 1);
        for ( j = 0; j < i; j++ )
            g_queue_gamma_map[i] &= ~g_queue_gamma_map[j];
    }

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

static void enable_vrx218_ppe_ema(unsigned int base)
{
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
    ret = pp32_download_code(dev, 0, vrx218_e1_fw_code, NUM_ENTITY(vrx218_e1_fw_code), vrx218_e1_fw_data, NUM_ENTITY(vrx218_e1_fw_data));
    if ( ret != PPA_SUCCESS )
        return ret;

    pp32_download_code(dev, 1, vrx218_pp32_1_fw_code, NUM_ENTITY(vrx218_pp32_1_fw_code), vrx218_pp32_1_fw_data, NUM_ENTITY(vrx218_pp32_1_fw_data));

    return PPA_SUCCESS;
}

static int pp32_start(const ifx_pcie_ep_dev_t *dev)
{
    unsigned int mask = 0x00010001;

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
//  vrx218_ppe_ptm_init.c
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
    set_dfe_data_rate(port, 4, 427, 427, 1);

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

//  vrx218_ppe_ptm_init.c
extern void vrx218_ppe_ptm_init(const ifx_pcie_ep_dev_t *dev, const ifx_pcie_ep_dev_t *peer_dev, int is_bonding, unsigned int vrx218_id, unsigned long sync_buf, int lle_in_sb, int cdma_write_data_en);
extern void __exit vrx218_ppe_ptm_fw_exit(const ifx_pcie_ep_dev_t *dev, int is_bonding, unsigned int vrx218_id);
extern void __exit vrx218_ppe_ptm_free_mem(const ifx_pcie_ep_dev_t *dev, int is_bonding, unsigned int vrx218_id);
//  vrx218_datapath.c
extern int vrx218_ptm_datapath_init(const ifx_pcie_ep_dev_t *p_vrx218_dev_us, const ifx_pcie_ep_dev_t *p_vrx218_dev_ds);
extern void vrx218_ptm_datapath_exit(void);

/*
*  US des sync: CDMA read and dmal write
*  US data copy: (DDR to PDBRAM) EDMA read
*  peer to peer write: dmal write
*  DS data copy: (PDBRAM to DDR) CDMA write
*  DS des sync: dmal write and CDMA read
*  EDMA no outbound base address, pcie will handle it.
*  DMAL and CDMA NEED outbound base address.
*/
static int __init init_vrx218_module(void)
{
    printk("port        = %d\n",     g_vrx218_dev->phy_membase == 0x18000000 ? 1 : 0);
    printk("irq         = %u\n",     g_vrx218_dev->irq);
    printk("membase     = 0x%08x\n", (unsigned int)g_vrx218_dev->membase);
    printk("phy_membase = 0x%08x\n", g_vrx218_dev->phy_membase);
    printk("peer_num    = %u\n",     g_vrx218_dev->peer_num);

    /*
     *  init hardware
     */
    reset_ppe(g_vrx218_dev->phy_membase);
    /*  activate VRX218 */
    enable_vrx218_ppe_ema(g_vrx218_dev->phy_membase);
    /*  set VRX218 PPE clock 432MHz */
    set_vrx218_ppe_clk(432, g_vrx218_dev->phy_membase);
    /*  clear and disable mailbox   */
    *MBOX_IGU0_ISRC(g_vrx218_dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU1_ISRC(g_vrx218_dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU0_IER(g_vrx218_dev->phy_membase) = 0x00;
    *MBOX_IGU1_IER(g_vrx218_dev->phy_membase) = 0x00;
    /*  freeze PP32 */
    pp32_stop(g_vrx218_dev);
    vrx218_ppe_ptm_init(g_vrx218_dev, NULL, 0 /* is_bonding */, 0 /* vrx218_id */, 0 /* sync_buf */, 0 /* lle_in_sb */, 1 /* cdma_write_data_en */);
    pp32_load(g_vrx218_dev);

    /*
     *  init datapath
     */
    vrx218_ptm_datapath_init(g_vrx218_dev, g_vrx218_dev);

    /*
     *  start hardware
     */
    pp32_start(g_vrx218_dev);
    setup_dfe_loopback(g_vrx218_dev->phy_membase);

    /*
     *  enable mailbox
     */
    // Enter overflow state in Qos_q[0-15]  MBOX_IGU0[15:0]
    // Exit overflow in           Qos_q[0-15]  MBOX_IGU0[31:16]
    *MBOX_IGU0_IER(g_vrx218_dev->phy_membase) = 0x00;   /* not ready to implement flow control yet */
    /*
       *  Bit 0 - Receive Pkt in Ds_pkt_des_list queue
       *  Bit 1 - Reserved for ATM: Receive Pkt in DS OAM  queue
       *  Bit 2 - Swap Queue pkt
       *  Bit 3 - EDMA Hang
       *  Bit 4 - Peer-to-peer-link-state  //single link don't need to support it
      */
    *MBOX_IGU1_IER(g_vrx218_dev->phy_membase) = 0x0D;   

    return PPA_SUCCESS;
}

static void __exit exit_vrx218_module(void)
{
    vrx218_ptm_datapath_exit();

    vrx218_ppe_ptm_fw_exit(g_vrx218_dev, 0 /* is_bonding */, 0 /* vrx218_id */);
    udelay(100);

    pp32_stop(g_vrx218_dev);

    vrx218_ppe_ptm_free_mem(g_vrx218_dev, 0 /* is_bonding */, 0 /* vrx218_id */);
}

static int __init init_vrx218_bonding_module(void)
{
    printk("VRX318_US_BONDING_MASTER\n");
    printk("port        = %d\n",     g_us_vrx218_dev->phy_membase == 0x18000000 ? 1 : 0);
    printk("irq         = %u\n",     g_us_vrx218_dev->irq);
    printk("membase     = 0x%08x\n", (unsigned int)g_us_vrx218_dev->membase);
    printk("phy_membase = 0x%08x\n", g_us_vrx218_dev->phy_membase);
    printk("peer_num    = %u\n",     g_us_vrx218_dev->peer_num);

    /*
     *  init hardware
     */
    reset_ppe(g_us_vrx218_dev->phy_membase);
    /*  activate VRX218 */
    enable_vrx218_ppe_ema(g_us_vrx218_dev->phy_membase);
    /*  set VRX218 PPE clock 432MHz */
    set_vrx218_ppe_clk(432, g_us_vrx218_dev->phy_membase);
    /*  clear and disable mailbox   */
    *MBOX_IGU0_ISRC(g_ds_vrx218_dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU1_ISRC(g_ds_vrx218_dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU0_IER(g_ds_vrx218_dev->phy_membase) = 0x00;
    *MBOX_IGU1_IER(g_ds_vrx218_dev->phy_membase) = 0x00;
    /*  freeze PP32 */
    pp32_stop(g_us_vrx218_dev);
    vrx218_ppe_ptm_init(g_us_vrx218_dev,
                        g_ds_vrx218_dev,
                        1 /* is_bonding */,
                        VRX218_US_BONDING_MASTER /* vrx218_id */,
                        g_bonding_sync_buf_page /* sync_buf */,
                        0 /* lle_in_sb */,
                        1 /* cdma_write_data_en */);
    pp32_load(g_us_vrx218_dev);

    printk("VRX318_DS_BONDING_MASTER\n");
    printk("port        = %d\n",     g_ds_vrx218_dev->phy_membase == 0x18000000 ? 1 : 0);
    printk("irq         = %u\n",     g_ds_vrx218_dev->irq);
    printk("membase     = 0x%08x\n", (unsigned int)g_ds_vrx218_dev->membase);
    printk("phy_membase = 0x%08x\n", g_ds_vrx218_dev->phy_membase);
    printk("peer_num    = %u\n",     g_ds_vrx218_dev->peer_num);

    /*
     *  init hardware
     */
    reset_ppe(g_ds_vrx218_dev->phy_membase);
    /*  activate VRX218 */
    enable_vrx218_ppe_ema(g_ds_vrx218_dev->phy_membase);
    /*  set VRX218 PPE clock 432MHz */
    set_vrx218_ppe_clk(432, g_ds_vrx218_dev->phy_membase);
    /*  clear and disable mailbox   */
    *MBOX_IGU0_ISRC(g_us_vrx218_dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU1_ISRC(g_us_vrx218_dev->phy_membase) = 0xFFFFFFFF;
    *MBOX_IGU0_IER(g_us_vrx218_dev->phy_membase) = 0x00;
    *MBOX_IGU1_IER(g_us_vrx218_dev->phy_membase) = 0x00;
    /*  freeze PP32 */
    pp32_stop(g_ds_vrx218_dev);
    vrx218_ppe_ptm_init(g_ds_vrx218_dev,
                        g_us_vrx218_dev,
                        1 /* is_bonding */,
                        VRX218_DS_BONDING_MASTER /* vrx218_id */,
                        g_bonding_sync_buf_page /* sync_buf */,
                        0 /* lle_in_sb */,
                        1 /* cdma_write_data_en */);
    pp32_load(g_ds_vrx218_dev);

    /*
     *  init datapath
     */
    vrx218_ptm_datapath_init(g_us_vrx218_dev, g_ds_vrx218_dev);

    /*
     *  start hardware
     */
    pp32_start(g_us_vrx218_dev);
    setup_dfe_loopback(g_us_vrx218_dev->phy_membase);
    pp32_start(g_ds_vrx218_dev);
    setup_dfe_loopback(g_ds_vrx218_dev->phy_membase);

    /*
     *  enable mailbox
     */
    /*
       *  Bit 0 - Receive Pkt in Ds_pkt_des_list queue
       *  Bit 1 - Reserved for ATM: Receive Pkt in DS OAM  queue
       *  Bit 2 - Swap Queue pkt
       *  Bit 3 - EDMA Hang
       *  Bit 4 - Peer-to-peer-link-state 
      */
    *MBOX_IGU0_IER(g_us_vrx218_dev->phy_membase) = 0x00;    /* not ready to implement flow control yet */
    *MBOX_IGU1_IER(g_us_vrx218_dev->phy_membase) = 0x1D;    /* not ready to implement swap queue, but enable DS */
    *MBOX_IGU0_IER(g_ds_vrx218_dev->phy_membase) = 0x00;    /* not ready to implement flow control yet */
    *MBOX_IGU1_IER(g_ds_vrx218_dev->phy_membase) = 0x1D;    /* not ready to implement swap queue, but enable DS */

    return PPA_SUCCESS;
}

static void __exit exit_vrx218_bonding_module(void)
{
    vrx218_ptm_datapath_exit();

    vrx218_ppe_ptm_fw_exit(g_us_vrx218_dev, 1 /* is_bonding */, VRX218_US_BONDING_MASTER /* vrx218_id */);
    vrx218_ppe_ptm_fw_exit(g_ds_vrx218_dev, 1 /* is_bonding */, VRX218_DS_BONDING_MASTER /* vrx218_id */);
    udelay(100);

    pp32_stop(g_us_vrx218_dev);
    pp32_stop(g_ds_vrx218_dev);

	vrx218_ppe_ptm_free_mem(g_us_vrx218_dev, 1 /* is_bonding */, VRX218_US_BONDING_MASTER /* vrx218_id */);
	vrx218_ppe_ptm_free_mem(g_ds_vrx218_dev, 1 /* is_bonding */, VRX218_DS_BONDING_MASTER /* vrx218_id */);
}

static void __switch_to_eth_thunk(unsigned long base)
{
    int i;
    struct rx_gamma_itf_cfg rx_gamma_itf_cfg;

    STD_DES_CFG(base)->byte_off     = 0;
#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
    BOND_CONF(base)->max_frag_size  = 1604;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base) = 0x02040660;
#else
    BOND_CONF(base)->max_frag_size  = 1536;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base) = 0x02040604;
#endif

    for ( i = 0; i < 4; i++ )
    {
        rx_gamma_itf_cfg = *RX_GAMMA_ITF_CFG(i, base);

        rx_gamma_itf_cfg.rx_eth_fcs_ver_dis = 0;
        rx_gamma_itf_cfg.rx_rm_eth_fcs      = 1;
        rx_gamma_itf_cfg.rx_edit_num1       = 4;
        rx_gamma_itf_cfg.rx_edit_type1      = 1;
        rx_gamma_itf_cfg.rx_edit_en1        = 1;

        *RX_GAMMA_ITF_CFG(i, base) = rx_gamma_itf_cfg;
    }

    // no need to change for tx gif config
    BOND_CONF(base)->bond_mode = L2_ETH_TRUNK_MODE;
}

static void switch_to_eth_trunk(void)
{
    // assume no traffic

    unsigned long sys_flag;

    local_irq_save(sys_flag);
    __switch_to_eth_thunk(g_us_vrx218_dev->phy_membase);
    __switch_to_eth_thunk(g_ds_vrx218_dev->phy_membase);
    local_irq_restore(sys_flag);
}

static void __switch_to_ptmtc_bonding(unsigned long base)
{
    int i;
    struct rx_gamma_itf_cfg rx_gamma_itf_cfg;

    STD_DES_CFG(base)->byte_off     = 2;
    BOND_CONF(base)->max_frag_size  = 0x200;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base) = 0x02040204;

    for ( i = 0; i < 4; i++ )
    {
        rx_gamma_itf_cfg = *RX_GAMMA_ITF_CFG(i, base);

        rx_gamma_itf_cfg.rx_eth_fcs_ver_dis = 1;
        rx_gamma_itf_cfg.rx_rm_eth_fcs      = 0;
        rx_gamma_itf_cfg.rx_edit_num1       = 0;
        rx_gamma_itf_cfg.rx_edit_type1      = 0;
        rx_gamma_itf_cfg.rx_edit_en1        = 0;

        *RX_GAMMA_ITF_CFG(i, base) = rx_gamma_itf_cfg;
    }

    // no need to change for tx gif config
    BOND_CONF(base)->bond_mode = L1_PTM_TC_BOND_MODE;
}

static void switch_to_ptmtc_bonding(void)
{
    // assume no traffic

    unsigned long sys_flag;

    local_irq_save(sys_flag);
    __switch_to_ptmtc_bonding(g_us_vrx218_dev->phy_membase);
    __switch_to_ptmtc_bonding(g_ds_vrx218_dev->phy_membase);
    local_irq_restore(sys_flag);
}

static void __update_max_delay(unsigned long base, int line)
{
    unsigned int user_max_delay;
    unsigned int linerate;
    int i, off;
    unsigned int delay_value;

    if ( g_dsl_bonding && line >= 0 && line < LINE_NUMBER )
    {
        delay_value = 1000;
        user_max_delay = (g_max_delay[line] + 8) / 16;
        for ( i = 0, linerate = 0; i < BEARER_CHANNEL_PER_LINE; i++ )
            linerate += g_datarate_ds[line * BEARER_CHANNEL_PER_LINE + i];
        linerate = (linerate + 50) / 100;
        if ( linerate != 0 )
        {
            //  delay_value = DS_BOND_LL_CTXT(0, base)->max_des_num * 512 * 8 / linerate / (16 / 1000000)
            delay_value = DS_BOND_LL_CTXT(0, base)->max_des_num * 256 * 10000 / linerate;
        }
        if ( user_max_delay != 0 && user_max_delay < delay_value )
            delay_value = user_max_delay;
        switch ( line )
        {
        case 0:
            off = 0;
            break;
        case 1:
        default:
            off = g_dsl_bonding == 1 ?  /* on-chip */ 2 : /* off-chip */ 4;
        }
        for ( i = 0; i < 2; i++ )
            DS_BOND_LL_CTXT(1 + off + i, base)->max_delay = delay_value;
    }
}

static void update_max_delay(int line)
{
    __update_max_delay(g_us_vrx218_dev->phy_membase, line);
    __update_max_delay(g_ds_vrx218_dev->phy_membase, line);
}

static int dsl_proc_read_status(struct seq_file *seq, void *v)
{
    int len = 0;
    int i;

    seq_printf(seq, "DSL Bonding Capability: %s\n", g_dsl_bonding ? "non-bonding" : "offchip bonding");
    if ( g_dsl_bonding)
        seq_printf(seq, BOND_CONF(g_vrx218_dev->phy_membase)->bond_mode == L1_PTM_TC_BOND_MODE ? "Bonding Mode: PTM TC\n" : "Bonding Mode: ETH Trunk\n");
    seq_printf(seq, "Upstream Data Rate:  ");
    for ( i = 0; i < NUM_ENTITY(g_datarate_us); i++ )
        seq_printf(seq, "\t%d", g_datarate_us[i]);
    seq_printf(seq, "\n");
    seq_printf(seq, "Downstream Data Rate:");
    for ( i = 0; i < NUM_ENTITY(g_datarate_ds); i++ )
        seq_printf(seq, "\t%d", g_datarate_ds[i]);
    seq_printf(seq, "\n");
    seq_printf(seq, "User Specified Max_Delay:");
    for ( i = 0; i < NUM_ENTITY(g_max_delay); i++ )
        seq_printf(seq, "\t%u", g_max_delay[i]);
    seq_printf(seq, "\n");


    return len;
}

static ssize_t dsl_proc_write_status(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char *p1, *p2;
    int len;
    int colon;
    char local_buf[128];

    char *env_var_str[] = {"help", "DSL_LINE_NUMBER", "DSL_TC_LAYER_STATUS", "DSL_EFM_TC_CONFIG_US", "DSL_BONDING_STATUS", "DSL_INTERFACE_STATUS", "DSL_DATARATE_US_BC", "DSL_DATARATE_DS_BC", "MAX_DELAY"};
    int line = -1, down = 0, upstr = 0, downstr = 0;
    int datarate_us[BEARER_CHANNEL_PER_LINE] = {0}, datarate_ds[BEARER_CHANNEL_PER_LINE] = {0};
    unsigned int num, bc;
    int i;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    p2 = NULL;
    colon = 1;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        for ( i = 0; i < NUM_ENTITY(env_var_str) && strncasecmp(p1, env_var_str[i], strlen(env_var_str[i])) != 0; i++ );
        switch ( i )
        {
        case 0: //  help
            printk("Usage:\n");
            printk("  echo <command> [parameters] > /proc/dsl_tc/status\n");
            printk("  commands:\n");
            printk("    DSL_LINE_NUMBER      - parameter: line number\n");
            printk("    DSL_TC_LAYER_STATUS  - parameter: ATM, EFM\n");
            printk("    DSL_EFM_TC_CONFIG_US - parameter: normal, preemption\n");
            printk("    DSL_BONDING_STATUS   - parameter: inactive, active\n");
            printk("    DSL_INTERFACE_STATUS - parameter: down, ready, handshake, training, up\n");
            printk("    DSL_DATARATE_US_BC?  - ?: bearer channel number 0/1, parameter: data rate in bps\n");
            printk("    DSL_DATARATE_DS_BC?  - ?: bearer channel number 0/1, parameter: data rate in bps\n");
            printk("    MAX_DELAY            - parameter: user specified max delay in micorsecond\n");
            break;
        case 1: //  DSL_LINE_NUMBER
            ignore_space(&p2, &len);
            num = get_number(&p2, &len, 0);
            if ( num < LINE_NUMBER )
                line = num;
            break;
        case 2: //  DSL_TC_LAYER_STATUS
            //  TODO
            break;
        case 3: //  DSL_EFM_TC_CONFIG_US
            //  TODO
            break;
        case 4: //  DSL_BONDING_STATUS
            if ( g_dsl_bonding )
            {
                p1 = p2;
                colon = 1;
                if ( get_token(&p1, &p2, &len, &colon) )
                {
                    if ( strcasecmp(p1, "inactive") == 0 )
                        switch_to_eth_trunk();
                    else if ( strcasecmp(p1, "active") == 0 )
                        switch_to_ptmtc_bonding();
                }
            }
            break;
        case 5: //  DSL_INTERFACE_STATUS
            if ( line >= 0 )
            {
                p1 = p2;
                colon = 1;
                if ( get_token(&p1, &p2, &len, &colon) && strcasecmp(p1, "down") == 0 && g_line_showtime[line] != 0 )
                {
                    g_line_showtime[line] = 0;
                    down = 1;
                }
            }
            break;
        case 6: //  DSL_DATARATE_US_BC
            bc = (unsigned int)(p1[strlen(env_var_str[i])] - '0');
            if ( line >= 0 && bc < BEARER_CHANNEL_PER_LINE )
            {
                ignore_space(&p2, &len);
                num = get_number(&p2, &len, 0);
                if ( num != 0 )
                {
                    datarate_us[bc] = num;
                    upstr = 1;
                }
            }
            break;
        case 7: //  DSL_DATARATE_DS_BC
            bc = (unsigned int)(p1[strlen(env_var_str[i])] - '0');
            if ( line >= 0 && bc < BEARER_CHANNEL_PER_LINE )
            {
                ignore_space(&p2, &len);
                num = get_number(&p2, &len, 0);
                if ( num != 0 )
                {
                    datarate_ds[bc] = num;
                    downstr = 1;
                }
            }
            break;
        case 8: //  MAX_DELAY
            if ( line >= 0 )
            {
                ignore_space(&p2, &len);
                num = get_number(&p2, &len, 0);
                g_max_delay[line] = num;
                if ( g_dsl_bonding )
                    update_max_delay(line);
            }
            break;
        }

        p1 = p2;
        colon = 1;
    }

    if ( line >= 0 )
    {
        if ( down )
        {
            for ( i = 0, num = 0; i < LINE_NUMBER; num += g_line_showtime[i], i++ );
            if ( num == 0 )
            {
                g_showtime = 0; //  equivalent to dsl_showtime_exit
                memset(g_datarate_us, 0, sizeof(g_datarate_us));
                memset(g_datarate_ds, 0, sizeof(g_datarate_us));
            }
        }
        else
        {
            if ( upstr )
            {
                for ( i = 0; i < BEARER_CHANNEL_PER_LINE; i++ )
                    if ( datarate_us[i] != 0 )
                        g_datarate_us[line * BEARER_CHANNEL_PER_LINE + i] = datarate_us[i];
                g_line_showtime[line] = 1;
                g_showtime = 1; //  equivalent to dsl_showtime_enter
            }
            if ( downstr )
            {
                for ( i = 0; i < BEARER_CHANNEL_PER_LINE; i++ )
                    if ( datarate_ds[i] != 0 )
                        g_datarate_ds[line * BEARER_CHANNEL_PER_LINE + i] = datarate_ds[i];
                if ( g_dsl_bonding )
                    update_max_delay(line);
            }
        }
    }

    return count;
}

static struct file_operations g_proc_file_status_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_status_seq_open,
    .read       = seq_read,
    .write      = dsl_proc_write_status,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_status_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, dsl_proc_read_status, NULL);
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


static struct file_operations g_proc_file_vector_prio_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_vector_prio_seq_open,
    .read       = seq_read,
    .write      = proc_write_vector_prio,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_vector_prio_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_vector_prio, NULL);
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

static struct file_operations g_proc_file_qos_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_qos_seq_open,
    .read       = seq_read,
    .write	= proc_write_qos,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_qos_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_qos, NULL);
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

static struct file_operations g_proc_file_dev_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dev_seq_open,
    .read       = seq_read,
    .write      = proc_write_dev,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_dev_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dev, NULL);
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

#if defined(DEBUG_BONDING_PROC) && DEBUG_BONDING_PROC
static struct file_operations g_proc_file_bondingmib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_bndmib_seq_open,
    .read       = seq_read,
    .write      = proc_write_bndmib,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_bndmib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_bndmib, NULL);
}

static struct file_operations g_proc_file_bonding_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_bonding_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_bonding_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_bonding, NULL);
}
#endif
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

static int proc_read_ps(struct seq_file *seq, void *v)
{
    int len = 0;
    u32 base = g_cur_vrx218_dev->phy_membase;
    volatile struct psave_cfg *ps_cfg = PSAVE_CFG(base);

    seq_printf(seq, "Power Saving: %s\n", ps_cfg->sleep_en == 1 ? "Enable" : "Disable");

    return len;
}

static ssize_t proc_write_ps(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[128];
    char *p;
    int len, rlen;
    u32 base = g_cur_vrx218_dev->phy_membase;  
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

static int proc_read_powersaving_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ps, NULL);
}

static struct file_operations g_proc_file_powersaving_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_powersaving_seq_open,
    .read       = seq_read,
    .write      = proc_write_ps,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static void proc_file_enable(char *entry_name, int is_enable)
{
    int i;

    for ( i = 0; i < ARRAY_SIZE(g_proc_entry); i++ ) {
        if ( strcmp(g_proc_entry[i].name, entry_name) == 0 )
            g_proc_entry[i].is_enable = is_enable;
    }
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
            if ( j == i )
                g_proc_entry[i].parent_proc_entry = NULL;
        }

        if ( g_proc_entry[i].is_dir ) {
            g_proc_entry[i].proc_entry = parent_proc_entry = proc_mkdir(g_proc_entry[i].name, g_proc_entry[i].parent_proc_entry);
            if ( g_proc_entry[i].proc_entry == NULL )
                printk("%s: Create proc directory \"%s\" failed!\n", __FUNCTION__, g_proc_entry[i].name);
        }
    }
    for ( i = 0; i < ARRAY_SIZE(g_proc_entry); i++ ) {
        if ( strcmp(g_proc_entry[i].name, "dsl_tc") == 0 )
    res  = proc_create("status",S_IRUGO|S_IWUSR, g_proc_entry[i].proc_entry, &g_proc_file_status_seq_fops);
    }
    res  = proc_create("ver",S_IRUGO, parent_proc_entry, &g_proc_file_ver_seq_fops);
    res  = proc_create("dbg",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_dbg_seq_fops);
    res  = proc_create("mem",S_IWUSR, parent_proc_entry, &g_proc_file_mem_seq_fops);
    res  = proc_create("pp32",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_pp32_seq_fops);
    res  = proc_create("qos",S_IRUGO, parent_proc_entry, &g_proc_file_qos_seq_fops);
    res  = proc_create("tfwdbg",S_IWUSR, parent_proc_entry, &g_proc_file_fwdbg_seq_fops);
    res  = proc_create("dev",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_dev_seq_fops);
    res  = proc_create("mib",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_wanmib_seq_fops);
    res  = proc_create("vector_prio",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_vector_prio_seq_fops);
#if defined(DEBUG_BONDING_PROC) && DEBUG_BONDING_PROC
    res  = proc_create("bonding",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_bonding_seq_fops);
    res  = proc_create("bondingmib",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_bondingmib_seq_fops);
#endif
    res  = proc_create("pciereset",S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_pciereset_seq_fops);
    res  = proc_create("powersaving", S_IRUGO|S_IWUSR, parent_proc_entry, &g_proc_file_powersaving_seq_fops);
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

    if ( g_dsl_bonding ) {
        //  bonding
        seq_printf(seq, "VRX318_US_BONDING_MASTER");
        print_fw_ver(seq, *FW_VER_ID(g_us_vrx218_dev->phy_membase));
        seq_printf(seq, "VRX318_DS_BONDING_MASTER");
        print_fw_ver(seq, *FW_VER_ID(g_ds_vrx218_dev->phy_membase));
    }
    else {
        //  single line
        print_fw_ver(seq, *FW_VER_ID(g_cur_vrx218_dev->phy_membase));
    }

    return len;
}

static int proc_read_vector_prio(struct seq_file *seq, void *v)
{
    int len = 0;

    seq_printf(seq, "Current vector priority value      - %d\n",g_vector_prio);
    return len;
}

int str2uint(char * p, unsigned int* res, int is_hex)
{
    int i = 0;
    unsigned int uint = 0;

    // [0x][0-9A-Fa-f]+
        if( p[0] == '0' && (  p[1]  == 'x' ||  p[1] == 'X')) {
            is_hex = 1;
            p += 2;
        }

    // check syntax
    while(p[i]) {
        if ( p[i] >= '0' && p[i] <= '9' ) {

            if (is_hex) {
                uint = (uint << 4) + p[i] - '0';
            }
            else {
                uint = uint * 10 + p[i] - '0';
            }

        }
        else {

            if (! is_hex)
                return 0;

            if ( p[i] >= 'a' && p[i] <= 'f' )
                uint = (uint << 4) + p[i] - 'a' + 10;
            else if ( p[i] >= 'A' && p[i] <= 'F' )
                uint = (uint << 4) + p[i] - 'A' + 10;
            else
                return 0;
        }

        ++i;
    }

    * res = uint;
    return 1;
}

static ssize_t proc_write_vector_prio(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	char str[32];
	char *p;
	unsigned int temp;

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
	if (str2uint(p,&temp,0) == 1) {
		if (temp >= 0) {
			g_vector_prio = temp;
			if (ltq_vectoring_priority_hook != NULL)
				ltq_vectoring_priority_hook(g_vector_prio);
		}
	}
	return count;
}

static int proc_read_dbg(struct seq_file *seq, void *v)
{
    int len = 0;

    seq_printf(seq, "error print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ERR)              ? "enabled" : "disabled");
    seq_printf(seq, "debug print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT)      ? "enabled" : "disabled");
    seq_printf(seq, "assert           - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ASSERT)           ? "enabled" : "disabled");
    seq_printf(seq, "dump rx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_RX)      ? "enabled" : "disabled");
    seq_printf(seq, "dump tx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_TX)      ? "enabled" : "disabled");
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
        9,  5,
        4
    };
    u32 dbg_enable_mask[] = {
        DBG_ENABLE_MASK_ERR,
        DBG_ENABLE_MASK_DEBUG_PRINT,
        DBG_ENABLE_MASK_ASSERT,
        DBG_ENABLE_MASK_DUMP_SKB_RX,
        DBG_ENABLE_MASK_DUMP_SKB_TX,
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
            "qosqctxt",
            "dessync",
            "bdmdswitch",
            "rx_bc_cfg",
            "read_rx_cb",
            "read_tx_cb",
            "clr_cb",
            "arcjtag",
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

    if(strncasecmp(tokens[0], "qosqctxt", strlen(tokens[0])) == 0){
        printk("echo qosqctxt [NO.] > %s \n",proc_file);
        printk("    :print QoS queue context \n");
    }
    else if(strncasecmp(tokens[0],"dessync", strlen(tokens[0])) == 0){
        printk("echo dessync [CPU | FASTPATH | DS | OUTQ | LOCALQ | DSFRAGQ | USFRQGQ | USEDMARD | DSEDMAWR ] [qidx] > %s \n", proc_file);
        printk("     :print vrx318 descriptor sync structure \n");
        printk("      CPU      for CPU PATH US_PATH_DES_SYNC\n");
        printk("      FASTPATH for FASTPATH US_PASH_DES_SYNC\n");
        printk("      DS       for DOWNSTREAM DS_PATH_DES_SYNC\n");
        printk("      OUTQ     for US_PATH_DES_SYNC Q0-Q3\n");
        printk("      LOCALQ   for US_PATH_DES_SYNC Q0-Q3\n");
        printk("      DSFRAGQ  for Bonding Downstream Fragment Q0-Q1\n");
        printk("      USFRQGQ  for Bonding Upstream Fragment Q0-Q7\n");
        printk("      USEDMARD for Upstream EDMA Read CTXT   \n");
        printk("      DSEDMAWR for Downstream EDMA Write CTXT \n");

    }
    else if(strncasecmp(tokens[0], "bdmdswitch", strlen(tokens[0])) == 0){
        printk("echo bdmdswitch [L1 | L2] > %s \n", proc_file);
        printk("     :switch to L1(PTM-TC bonding)mode or L2(Ethernet Trunk) mode\n");
    }
    else if(strncasecmp(tokens[0], "rx_bc_cfg", strlen(tokens[0])) == 0){
        printk("echo rx_bc_cfg > %s\n", proc_file);
        printk("     :print rx bear channel status \n");
    }
    else if(strncasecmp(tokens[0], "read_rx_cb", strlen(tokens[0])) == 0){
        printk("echo read_rx_cb [0|1] [start_page] [print_page_num] > %s\n", proc_file);
        printk("     :print RX codeword buffer for Bearer Channel 0/1 \n");
    }
    else if(strncasecmp(tokens[0], "read_tx_cb", strlen(tokens[0])) == 0){
        printk("echo read_tx_cb [0|1] [start_page] [print_page_num] > %s\n", proc_file);
        printk("     :print TX codeword buffer for Bearer Channel 0/1 \n");
    }
    else if(strncasecmp(tokens[0], "clr_cb", strlen(tokens[0])) == 0){
        printk("echo clr_cb [rx|tx] [0|1] [start_page] [print_page_num] > %s\n", proc_file);
        printk("     :Clear RX or TX codeword buffer for Bearer Channel 0/1 \n");
    }
    else if(strncasecmp(tokens[0],"arcjtag",strlen(tokens[0])) == 0){
        printk("echo arcjtag [mapping_address] [bar_id]> %s\n", proc_file);
        printk("     :Mapping arc jtag bar to given mapping address\n");
        printk("     :Print Current Mapping if no mapping address provided\n");
    }
    else{
        fwdbg_help((char **)NULL, 0);
    }

}

static void dump_qosqctxt(int qid)
{
    volatile qosq_mib_t *qmib;
    if(qid < 0 || qid >= VRX218_QOSQ_NUM){
        printk("QoSQ id is out of range: %d\n", qid);
        return;
    }

    qmib = WAN_QOS_MIB_TABLE(qid,g_cur_vrx218_dev->phy_membase);
    printk("Queue id[%d]:      rx_pkt_cnt/rx_byte_cnt       tx_pkt_cnt/tx_byte_cnt      small_drop[cnt/bytes]       large_drop[cnt/bytes]\n", qid);
    printk("                  0x%08x/0x%08x        0x%08x/0x%08x       0x%08x/0x%08x       0x%08x/0x%08x\n",
        qmib->rx_pkt_cnt, qmib->rx_byte_cnt, qmib->tx_pkt_cnt, qmib->tx_byte_cnt,
        qmib->small_pkt_drop_cnt, qmib->small_pkt_drop_byte_cnt, qmib->large_pkt_drop_cnt, qmib->large_pkt_drop_byte_cnt);

    return;
}

static void fwdbg_qosqctxt(char **tokens, int token_num)
{
    int i = 0;
    int qid = -1;

    if(token_num > 0){
        qid = simple_strtoul(tokens[0],NULL, 0);
    }

    if(qid < 0 || qid >= VRX218_QOSQ_NUM){
        qid = -1;
    }

    if(qid >= 0){
        dump_qosqctxt(qid);
    }else{
        for(i = 0; i < VRX218_QOSQ_NUM; i ++){
            dump_qosqctxt(i);
        }
    }

    return;

}

static void print_des_sync(char *name, uint32_t addr)
{
    volatile desq_cfg_ctxt_t *des_sync_ctxt = (volatile desq_cfg_ctxt_t *)SOC_ACCESS_VRX218_ADDR(SB_BUFFER(addr),g_cur_vrx218_dev->phy_membase);

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
                      {{"CPU",      __US_CPU_INQ_DES_CFG_CTXT,      1 },
                       {"FASTPATH", __US_FP_INQ_DES_CFG_CTXT,       1 },
                       {"DS",       __DS_PKT_DESQ_CFG_CTXT,         1 },
                       {"OUTQ",     __US_QOS_OUTQ_DES_CFG_CTXT_BASE,4 },
                       {"LOCALQ",   __US_TC_LOCAL_Q_CFG_CTXT_BASE,  4 },
                       {"DSFRAGQ",  __DS_FRAGQ_CFG_CTXT_BASE,       2 },
                       {"USFRAGQ",  __US_FRAGQ_CFG_CTXT_BASE,       8 },
                       {"USEDMARD", __US_EDMA_READ_CH_CFG_CTXT,     1 },
                       {"DSEDMAWR", __DS_EDMA_WRITE_CH_CFG_CTXT,    1 },

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

static void switch_to_ptmtc_bonding_mode(unsigned int base)
{
    bond_conf_t bond_cfg;
    std_des_cfg_t std_des_cfg;
    struct rx_gamma_itf_cfg rx_gitf_cfg;
    void *dst_addr;
    int i;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__BOND_CONF, base);
    bond_cfg = *(volatile bond_conf_t *)dst_addr;
    if(bond_cfg.e1_bond_en != 1 || bond_cfg.bond_mode == L1_PTM_TC_BOND_MODE){
        return;
    }

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base) = 0x02040204;
    bond_cfg.max_frag_size = 0x200;
    bond_cfg.bond_mode = L1_PTM_TC_BOND_MODE;
    dword_mem_write(dst_addr, &bond_cfg, sizeof(bond_conf_t));

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__STD_DES_CFG,base);
    std_des_cfg = *(volatile std_des_cfg_t *)dst_addr;
    std_des_cfg.byte_off = 2;
    dword_mem_write(dst_addr, &std_des_cfg, sizeof(std_des_cfg_t));

    for(i = 0; i < 4; i ++){
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__RX_GIF0_CFG_STATS_CFG + (i*sizeof(struct rx_gamma_itf_cfg)/sizeof(unsigned int))), base);
        rx_gitf_cfg = *(struct rx_gamma_itf_cfg *)dst_addr;
        rx_gitf_cfg.rx_eth_fcs_ver_dis = 1;
        rx_gitf_cfg.rx_rm_eth_fcs      = 0;
        rx_gitf_cfg.rx_edit_num1       = 0;
        rx_gitf_cfg.rx_edit_type1      = 0;
        rx_gitf_cfg.rx_edit_en1        = 0;
        dword_mem_write(dst_addr, &rx_gitf_cfg, sizeof(struct rx_gamma_itf_cfg));
    }

    return;

}

static void switch_to_eth_trunk_mode(unsigned int base)
{
    bond_conf_t bond_cfg;
    std_des_cfg_t std_des_cfg;
    struct rx_gamma_itf_cfg rx_gitf_cfg;
    void *dst_addr;
    int i;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__BOND_CONF, base);
    bond_cfg = *(volatile bond_conf_t *)dst_addr;
    if(bond_cfg.e1_bond_en != 1 || bond_cfg.bond_mode == L2_ETH_TRUNK_MODE){
        return;
    }

#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
    bond_cfg.max_frag_size = 1604;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base) = 0x02040660;
#else
    bond_cfg.max_frag_size = 1536;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base) = 0x02040604;
#endif
    bond_cfg.bond_mode = L2_ETH_TRUNK_MODE;
    dword_mem_write(dst_addr, &bond_cfg, sizeof(bond_conf_t));

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__STD_DES_CFG,base);
    std_des_cfg = *(volatile std_des_cfg_t *)dst_addr;
    std_des_cfg.byte_off = 0;
    dword_mem_write(dst_addr, &std_des_cfg, sizeof(std_des_cfg_t));

    for(i = 0; i < 4; i ++){
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__RX_GIF0_CFG_STATS_CFG + (i*sizeof(struct rx_gamma_itf_cfg)/sizeof(unsigned int))), base);
        rx_gitf_cfg = *(struct rx_gamma_itf_cfg *)dst_addr;
        rx_gitf_cfg.rx_eth_fcs_ver_dis = 0;
        rx_gitf_cfg.rx_rm_eth_fcs      = 1;
        rx_gitf_cfg.rx_edit_num1       = 4;
        rx_gitf_cfg.rx_edit_type1      = 1;
        rx_gitf_cfg.rx_edit_en1        = 1;
        dword_mem_write(dst_addr, &rx_gitf_cfg, sizeof(struct rx_gamma_itf_cfg));
    }

    return;
}

static void fwdbg_bonding_mode_switch(char **tokens, int token_num)
{
    if(g_ds_vrx218_dev == g_us_vrx218_dev){
        printk("Non Bonding mode, quit switch\n");
        return;
    }

    if(token_num < 1){
        fw_dbg_start("help bdmdswitch");
        return;
    }

    if(strncasecmp(tokens[0],"L1",strlen(tokens[0])) == 0){//switch to TC-Bonding mode
        switch_to_ptmtc_bonding_mode(g_us_vrx218_dev->phy_membase);
        switch_to_ptmtc_bonding_mode(g_ds_vrx218_dev->phy_membase);
    }
    else if(strncasecmp(tokens[0],"L2",strlen(tokens[0])) == 0){
        switch_to_eth_trunk_mode(g_us_vrx218_dev->phy_membase);
        switch_to_eth_trunk_mode(g_ds_vrx218_dev->phy_membase);
    }
    else{
        fw_dbg_start("help bdmdswitch");
    }

    return;
}

static void fwdbg_rx_bc_cfg(char **tokens, int token_num)
{

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

    struct rx_bc_cfg rx_bc_stat;
    int i;

    for(i = 0; i < 2; i ++){
        rx_bc_stat = *(struct rx_bc_cfg *)SOC_ACCESS_VRX218_SB((__RX_BC0_CFG_STATS_CFG + (i*sizeof(struct rx_bc_cfg)/sizeof(unsigned int))), g_cur_vrx218_dev->phy_membase);
        printk("RX Bearer Channel[%d]:\n", i);
        printk("    local_state: %d [ %-20s ] remote_state: %d [ %-12s ]  rx_cw_rdptr:%d \n\n",
                   rx_bc_stat.local_state, ls_txt[rx_bc_stat.local_state],
                   rx_bc_stat.remote_state, rs_txt[rx_bc_stat.remote_state],
                   rx_bc_stat.rx_cw_rdptr);
    }
}

static void fwdbg_rd_rx_cb(char **tokens, int token_num)
{
    int i = 0;
    int bc_id = 0;
    int start_pg = 0;
    int page_max_num = 0;
    int page_num = 1;
    int num;
    const int cw_pg_size = 17;
    volatile struct SFSM_cfg *sfsm_cfg;
    volatile unsigned int *dbase;
    volatile unsigned int *cbase;
    unsigned int addr_base = g_cur_vrx218_dev->phy_membase;
    struct PTM_CW_CTRL * p_ctrl;
    volatile unsigned int *cw;

    unsigned char * cwid_txt[] = {
            "All_data",
            "End_of_frame",
            "Start_while_tx",
            "All_idle",
            "Start_while_idle",
            "All_idle_nosync",
            "Error",
            "Res",
            NULL
    };

    while(token_num > i){
        switch(i){
            case 0: //get bearer channel id 0 - 1
                bc_id = simple_strtoul(tokens[i],NULL, 0);
                if(bc_id < 0 || bc_id >= 2){
                    bc_id = 0;
                }
                sfsm_cfg = SFSM_CFG(bc_id, addr_base);
                page_max_num = sfsm_cfg->pnum;
                break;

            case 1: // get start page no.
                start_pg = simple_strtoul(tokens[i], NULL, 0);
                if(start_pg < 0 || start_pg >= page_max_num){
                    start_pg = 0;
                }
                break;

            case 2: // get print page num
                page_num = simple_strtoul(tokens[i], NULL, 0);
                if(page_num <= 0 || page_num >= page_max_num){
                    page_num = page_max_num;
                }
                break;

            default:
                break;
        }
        i ++;
    }

    if(page_max_num == 0){
        sfsm_cfg = SFSM_CFG(bc_id, addr_base);
        page_max_num = sfsm_cfg->pnum;
    }

    if(page_num == 0){
           page_num = page_max_num;
    }

    dbase = (volatile unsigned int *)SOC_ACCESS_VRX218_SB(SFSM_DBA(bc_id, addr_base)->dbase + 0x2000, addr_base);
    cbase = (volatile unsigned int *)SOC_ACCESS_VRX218_SB(SFSM_CBA(bc_id, addr_base)->cbase + 0x2000, addr_base);

    printk("PTM RX BC[%d] CELL data/ctrl buffer (Total Page Num = %d):\n\n", bc_id, page_max_num);
    for(i = start_pg, num = 0 ; num < page_num ; num ++, i = (i + 1) % page_max_num ) {

        cw = dbase + i * cw_pg_size;
        p_ctrl = (struct PTM_CW_CTRL *) ( &cbase[i]);

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

    return;
}

static void fwdbg_rd_tx_cb(char **tokens, int token_num)
{
    int i = 0;
    int bc_id = 0;
    int start_pg = 0;
    int page_max_num = 0;
    int page_num = 0;
    int num;
    const int cw_pg_size = 17;
    volatile struct FFSM_cfg *ffsm_cfg;
    volatile unsigned int *dbase;
    unsigned int addr_base = g_cur_vrx218_dev->phy_membase;
    volatile unsigned int *cw;

    while(token_num > i){
        switch(i){
            case 0: //get bearer channel id 0 - 1
                bc_id = simple_strtoul(tokens[i],NULL, 0);
                if(bc_id < 0 || bc_id >= 2){
                    bc_id = 0;
                }
                ffsm_cfg = FFSM_CFG(bc_id, addr_base);
                page_max_num = ffsm_cfg->pnum;
                break;

            case 1: // get start page no.
                start_pg = simple_strtoul(tokens[i], NULL, 0);
                if(start_pg < 0 || start_pg >= page_max_num){
                    start_pg = 0;
                }
                break;

            case 2: // get print page num
                page_num = simple_strtoul(tokens[i], NULL, 0);
                if(page_num <= 0 || page_num >= page_max_num){
                    page_num = page_max_num;
                }
                break;

            default:
                break;
        }
        i ++;
    }

    if(page_max_num == 0){
        ffsm_cfg = FFSM_CFG(bc_id, addr_base);
        page_max_num = ffsm_cfg->pnum;
    }

    if(page_num == 0){
        page_num = page_max_num;
    }

    dbase = (volatile unsigned int *)SOC_ACCESS_VRX218_SB(FFSM_DBA(bc_id, addr_base)->dbase + 0x2000, addr_base);
    printk("PTM TX BC[%d] CELL data buffer(Total Page Num:%d):\n\n", bc_id, page_max_num);
    for(i = start_pg, num = 0 ; num < page_num ; num ++, i = (i + 1) % page_max_num ) {

        cw = dbase + i * cw_pg_size;
        printk("CW %2d: %08x \n"          , i, cw[0]);
        printk("       %08x %08x %08x %08x\n",  cw[1], cw[2], cw[3], cw[4]);
        printk("       %08x %08x %08x %08x\n",  cw[5], cw[6], cw[7], cw[8]);
        printk("       %08x %08x %08x %08x\n",  cw[9], cw[10], cw[11], cw[12]);
        printk("       %08x %08x %08x %08x\n",  cw[13], cw[14], cw[15], cw[16]);
    }

    return;

}

static void fwdbg_clr_cb(char **tokens, int token_num)
{
    int i = 0;
    int is_rx = 1; // rx - 1, tx -2
    int bc_id = 0;
    int start_pg = 0;
    int page_max_num = 0;
    int page_num = 0;
    int num;
    const int cw_pg_size = 17;
    volatile struct FFSM_cfg *ffsm_cfg;
    volatile struct SFSM_cfg *sfsm_cfg;
    volatile unsigned int *dbase;
    volatile unsigned int *cbase;
    unsigned int addr_base = g_cur_vrx218_dev->phy_membase;

    while(token_num > i){
        switch(i){
            case 0: //get rx or tx
                if(strncasecmp(tokens[i],"tx",strlen(tokens[i])) == 0){
                    is_rx = 2;
                }else{
                    is_rx = 1;
                }
            case 1: //get bearer channel id 0 - 1
                bc_id = simple_strtoul(tokens[i],NULL, 0);
                if(bc_id < 0 || bc_id >= 2){
                    bc_id = 0;
                }
                if(is_rx & 1){//rx
                    sfsm_cfg = SFSM_CFG(bc_id, addr_base);
                    page_max_num = sfsm_cfg->pnum;
                }else{//tx
                    ffsm_cfg = FFSM_CFG(bc_id, addr_base);
                    page_max_num = ffsm_cfg->pnum;
                }

                break;

            case 2: // get start page no.
                start_pg = simple_strtoul(tokens[i], NULL, 0);
                if(start_pg < 0 || start_pg >= page_max_num){
                    start_pg = 0;
                }
                break;

            case 3: // get print page num
                page_num = simple_strtoul(tokens[i], NULL, 0);
                if(page_num <= 0 || page_num >= page_max_num){
                    page_num = page_max_num;
                }
                break;

            default:
                break;
        }
        i ++;
    }

    if(page_max_num == 0){
        sfsm_cfg = SFSM_CFG(bc_id, addr_base);
        page_max_num = sfsm_cfg->pnum;
    }

    if(page_num == 0){
        page_num = page_max_num;
    }

    if(is_rx & 1){
        dbase = (volatile unsigned int *)SOC_ACCESS_VRX218_SB(SFSM_DBA(bc_id,addr_base)->dbase + 0x2000, addr_base);
        cbase = (volatile unsigned int *)SOC_ACCESS_VRX218_SB(SFSM_CBA(bc_id,addr_base)->cbase + 0x2000, addr_base);

        for(i = 0, num = 0; num < page_num; i = (i + 1) % page_max_num){
            memset((void *)(dbase + i * cw_pg_size), 0 , cw_pg_size * sizeof(unsigned int));
            memset((void *)(cbase + i), 0, sizeof(unsigned int));
        }
    }
    else if(is_rx & 2){
        dbase = (volatile unsigned int *)SOC_ACCESS_VRX218_SB(FFSM_DBA(bc_id,addr_base)->dbase + 0x2000, addr_base);

        for(i = 0, num = 0; num < page_num; i = (i + 1) % page_max_num){
            memset((void *)(dbase + i * cw_pg_size), 0 , cw_pg_size * sizeof(unsigned int));
        }
    }

    return;
}

static void fwdbg_arcjtag(char **tokens, int token_num)
{
    unsigned int map_addr;
    int i = 0;
    int bar_id = 5; //by default, we use bar 5 for VDSL PTM as DSL driver don't use it
    unsigned int base = 0;
    unsigned int bar_base;

    if(token_num == 0){//print address mapping
        printk("Mapping Address: \n");
        bar_base = 0x080000;
        for(i = 0; i <= 16; i ++){
            printk("BAR[%d]: mapping  0x%8x ------------> 0x%8x\n",
                i, bar_base + i * 0x010000,  *VRX218_MEI_XMEM_BAR(i,base));
        }
        return;
    }else{
        map_addr = simple_strtoul(tokens[i ++], NULL, 0);
        if(token_num > i){
            bar_id = simple_strtoul(tokens[i ++], NULL, 0);
        }
    }


    base = g_cur_vrx218_dev->phy_membase;
    *RST_REQ(base) |= 0x00100000;   //enable arc debug in RST
    *RST_DBGR(base) = 0;            //Set debugger: 1. FPI is not used to control LD/ST bus.  2. not suspend FPI_BUS peripheral


    //Set LDST
    *VRX218_MEI_DBG_MASTER(base)    = 1;
    *VRX218_MEI_DBG_PORT_SEL(base)  = 0;
    *VRX218_MEI_DBG_DECODE(base)    = 1;

    //Config BAR Addresses
    *VRX218_MEI_XMEM_BAR(bar_id,base) = map_addr;


    //Set LDST
    *VRX218_MEI_DBG_MASTER(base)    = 0;
    *VRX218_MEI_DBG_PORT_SEL(base)  = 0;
    *VRX218_MEI_DBG_DECODE(base)    = 1;

    return;
}

static void fw_dbg_start(char *cmdbuf)
{
    char * tokens[32];
    int finished, token_num;
    int i;

    fw_dbg_t cmds[]={{"help",           fwdbg_help},
                     {"qosqctxt",       fwdbg_qosqctxt},
                     {"dessync",        fwdbg_des_sync},
                     {"bdmdswitch",     fwdbg_bonding_mode_switch},
                     {"rx_bc_cfg",      fwdbg_rx_bc_cfg},
                     {"read_rx_cb",     fwdbg_rd_rx_cb},
                     {"read_tx_cb",     fwdbg_rd_tx_cb},
                     {"clr_cb",         fwdbg_clr_cb},
                     {"arcjtag",        fwdbg_arcjtag},
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

static ssize_t proc_write_fwdbg(struct file *file, const char __user *buf, size_t count, loff_t *data)
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


static int proc_read_qos(struct seq_file *seq, void *v)
{
    int i, len;
    volatile qosq_mib_t *qmib;
    volatile uint32_t *p;

    len = 0;
    seq_printf(seq,    "PTM QoS MIB: \n\t rx_pkt/rx_byts           tx_pkt/tx_bytes             small_drop[cnt/bytes]        large_drop[cnt/bytes]     adress\n");

    for(i = 0; i < VRX218_QOSQ_NUM; i ++){
       p = (volatile uint32_t *)sb_addr_to_fpi_addr_convert( (unsigned long)( __QOSQ_MIB_BASE + i * (sizeof(qosq_mib_t)/sizeof(uint32_t))));
       qmib = (qosq_mib_t *)SOC_ACCESS_VRX218_ADDR(p, g_us_vrx218_dev->phy_membase);
       seq_printf(seq, "  [%d]:  %010u/%010u     %010u/%010u       %010u/%010u        %010u/%010u     @0x%x\n",
                                      i, qmib->rx_pkt_cnt, qmib->rx_byte_cnt, qmib->tx_pkt_cnt, qmib->tx_byte_cnt,
                                      qmib->small_pkt_drop_cnt, qmib->small_pkt_drop_byte_cnt,
                                      qmib->large_pkt_drop_cnt, qmib->large_pkt_drop_byte_cnt,
                                      (uint32_t)qmib);
    }

    return len;
}

static ssize_t proc_write_qos(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char local_buf[64];
    int len;
    char *p1,*p2;
    int colon = 0;

    volatile qosq_mib_t *qmib;
    int i;
    volatile unsigned long *p;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len -= copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;

    get_token(&p1, &p2, &len, &colon);

    if(strcasecmp(p1, "clear") == 0){
        for(i = 0; i < VRX218_QOSQ_NUM; i ++){
            p = (volatile unsigned long *)sb_addr_to_fpi_addr_convert((unsigned long)( __QOSQ_MIB_BASE + i * (sizeof(qosq_mib_t)/sizeof(uint32_t))));
            qmib = (qosq_mib_t *)SOC_ACCESS_VRX218_ADDR(p, g_us_vrx218_dev->phy_membase);
            memset((void *)qmib, 0, sizeof(*qmib));
        }
    }

    return count;
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
        p = (volatile unsigned long *)SOC_ACCESS_VRX218_ADDR(p, g_cur_vrx218_dev->phy_membase);

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
        p = (volatile unsigned long *)SOC_ACCESS_VRX218_ADDR(p, g_cur_vrx218_dev->phy_membase);

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

        if ( (*PP32_FREEZE(g_cur_vrx218_dev->phy_membase) & (1 << (pp32 << 4))) != 0 )
        {
            sprintf(str, "freezed");
            f_stopped = 1;
        }
        else if ( PP32_CPU_USER_STOPPED(g_cur_vrx218_dev->phy_membase, pp32) || PP32_CPU_USER_BREAKIN_RCV(g_cur_vrx218_dev->phy_membase, pp32) || PP32_CPU_USER_BREAKPOINT_MET(g_cur_vrx218_dev->phy_membase, pp32) )
        {
            strlength = 0;
            if ( PP32_CPU_USER_STOPPED(g_cur_vrx218_dev->phy_membase, pp32) )
                strlength += sprintf(str + strlength, "stopped");
            if ( PP32_CPU_USER_BREAKPOINT_MET(g_cur_vrx218_dev->phy_membase, pp32) )
                strlength += sprintf(str + strlength, strlength ? " | breakpoint" : "breakpoint");
            if ( PP32_CPU_USER_BREAKIN_RCV(g_cur_vrx218_dev->phy_membase, pp32) )
                strlength += sprintf(str + strlength, strlength ? " | breakin" : "breakin");
            f_stopped = 1;
        }
        else if ( PP32_CPU_CUR_PC(g_cur_vrx218_dev->phy_membase, pp32) == PP32_CPU_CUR_PC(g_cur_vrx218_dev->phy_membase, pp32) )
        {
            unsigned int pc_value[64] = {0};

            f_stopped = 1;
            for ( i = 0; f_stopped && i < NUM_ENTITY(pc_value); i++ )
            {
                pc_value[i] = PP32_CPU_CUR_PC(g_cur_vrx218_dev->phy_membase, pp32);
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
        cur_context = PP32_BRK_CUR_CONTEXT(g_cur_vrx218_dev->phy_membase, pp32);
        seq_printf(seq, "Context: %d, PC: 0x%04x, %s\n", cur_context, PP32_CPU_CUR_PC(g_cur_vrx218_dev->phy_membase, pp32), str);

        if ( PP32_CPU_USER_BREAKPOINT_MET(g_cur_vrx218_dev->phy_membase, pp32) )
        {
            strlength = 0;
            if ( PP32_BRK_PC_MET(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                strlength += sprintf(str + strlength, "pc0");
            if ( PP32_BRK_PC_MET(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                strlength += sprintf(str + strlength, strlength ? " | pc1" : "pc1");
            if ( PP32_BRK_DATA_ADDR_MET(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                strlength += sprintf(str + strlength, strlength ? " | daddr0" : "daddr0");
            if ( PP32_BRK_DATA_ADDR_MET(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                strlength += sprintf(str + strlength, strlength ? " | daddr1" : "daddr1");
            if ( PP32_BRK_DATA_VALUE_RD_MET(g_cur_vrx218_dev->phy_membase, pp32, 0) )
            {
                strlength += sprintf(str + strlength, strlength ? " | rdval0" : "rdval0");
                if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                {
                    if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_RD_MET(g_cur_vrx218_dev->phy_membase, pp32, 1) )
            {
                strlength += sprintf(str + strlength, strlength ? " | rdval1" : "rdval1");
                if ( PP32_BRK_DATA_VALUE_RD_LO_EQ(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                {
                    if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_RD_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_WR_MET(g_cur_vrx218_dev->phy_membase, pp32, 0) )
            {
                strlength += sprintf(str + strlength, strlength ? " | wtval0" : "wtval0");
                if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                {
                    if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 0) )
                    strlength += sprintf(str + strlength, " >=");
            }
            if ( PP32_BRK_DATA_VALUE_WR_MET(g_cur_vrx218_dev->phy_membase, pp32, 1) )
            {
                strlength += sprintf(str + strlength, strlength ? " | wtval1" : "wtval1");
                if ( PP32_BRK_DATA_VALUE_WR_LO_EQ(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                {
                    if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 1) )
                        strlength += sprintf(str + strlength, " ==");
                    else
                        strlength += sprintf(str + strlength, " <=");
                }
                else if ( PP32_BRK_DATA_VALUE_WR_GT_EQ(g_cur_vrx218_dev->phy_membase, pp32, 1) )
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
                    seq_printf(seq, "   %2d: %08x", i + j * 4, *PP32_GP_CONTEXTi_REGn(g_cur_vrx218_dev->phy_membase, pp32, cur_context, i + j * 4));
                seq_printf(seq, "\n");
            }
        }

        seq_printf(seq, "break out on: break in - %s, stop - %s\n",
                                            PP32_CTRL_OPT_BREAKOUT_ON_BREAKIN(g_cur_vrx218_dev->phy_membase, pp32) ? stron : stroff,
                                            PP32_CTRL_OPT_BREAKOUT_ON_STOP(g_cur_vrx218_dev->phy_membase, pp32) ? stron : stroff);
        seq_printf(seq, "     stop on: break in - %s, break point - %s\n",
                                            PP32_CTRL_OPT_STOP_ON_BREAKIN(g_cur_vrx218_dev->phy_membase, pp32) ? stron : stroff,
                                            PP32_CTRL_OPT_STOP_ON_BREAKPOINT(g_cur_vrx218_dev->phy_membase, pp32) ? stron : stroff);
        seq_printf(seq, "breakpoint:\n");
        seq_printf(seq, "     pc0: 0x%08x, %s\n", *PP32_BRK_PC(g_cur_vrx218_dev->phy_membase, pp32, 0), PP32_BRK_GRPi_PCn(g_cur_vrx218_dev->phy_membase, pp32, 0, 0) ? "group 0" : "off");
        seq_printf(seq, "     pc1: 0x%08x, %s\n", *PP32_BRK_PC(g_cur_vrx218_dev->phy_membase, pp32, 1), PP32_BRK_GRPi_PCn(g_cur_vrx218_dev->phy_membase, pp32, 1, 1) ? "group 1" : "off");
        seq_printf(seq, "  daddr0: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(g_cur_vrx218_dev->phy_membase, pp32, 0), PP32_BRK_GRPi_DATA_ADDRn(g_cur_vrx218_dev->phy_membase, pp32, 0, 0) ? "group 0" : "off");
        seq_printf(seq, "  daddr1: 0x%08x, %s\n", *PP32_BRK_DATA_ADDR(g_cur_vrx218_dev->phy_membase, pp32, 1), PP32_BRK_GRPi_DATA_ADDRn(g_cur_vrx218_dev->phy_membase, pp32, 1, 1) ? "group 1" : "off");
        seq_printf(seq, "  rdval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(g_cur_vrx218_dev->phy_membase, pp32, 0));
        seq_printf(seq, "  rdval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_RD(g_cur_vrx218_dev->phy_membase, pp32, 1));
        seq_printf(seq, "  wrval0: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(g_cur_vrx218_dev->phy_membase, pp32, 0));
        seq_printf(seq, "  wrval1: 0x%08x\n", *PP32_BRK_DATA_VALUE_WR(g_cur_vrx218_dev->phy_membase, pp32, 1));
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
        *PP32_FREEZE(g_cur_vrx218_dev->phy_membase) &= ~(1 << (pp32 << 4));
    else if ( strcasecmp(p, "freeze") == 0 )
        *PP32_FREEZE(g_cur_vrx218_dev->phy_membase) |= 1 << (pp32 << 4);
    else if ( strcasecmp(p, "start") == 0 )
        *PP32_CTRL_CMD(g_cur_vrx218_dev->phy_membase, pp32) = PP32_CTRL_CMD_RESTART;
    else if ( strcasecmp(p, "stop") == 0 )
        *PP32_CTRL_CMD(g_cur_vrx218_dev->phy_membase, pp32) = PP32_CTRL_CMD_STOP;
    else if ( strcasecmp(p, "step") == 0 )
        *PP32_CTRL_CMD(g_cur_vrx218_dev->phy_membase, pp32) = PP32_CTRL_CMD_STEP;
    else if ( strncasecmp(p, "pc0 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_PCn_OFF(0, 0);
            *PP32_BRK_PC_MASK(g_cur_vrx218_dev->phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(g_cur_vrx218_dev->phy_membase, pp32, 0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(g_cur_vrx218_dev->phy_membase, pp32, 0) = addr;
            *PP32_BRK_PC_MASK(g_cur_vrx218_dev->phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_PCn_ON(0, 0);
        }
    }
    else if ( strncasecmp(p, "pc1 ", 4) == 0 )
    {
        p += 4;
        rlen -= 4;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_PCn_OFF(1, 1);
            *PP32_BRK_PC_MASK(g_cur_vrx218_dev->phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_PC(g_cur_vrx218_dev->phy_membase, pp32, 1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_PC(g_cur_vrx218_dev->phy_membase, pp32, 1) = addr;
            *PP32_BRK_PC_MASK(g_cur_vrx218_dev->phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_PCn_ON(1, 1);
        }
    }
    else if ( strncasecmp(p, "daddr0 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_OFF(0, 0);
            *PP32_BRK_DATA_ADDR_MASK(g_cur_vrx218_dev->phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(g_cur_vrx218_dev->phy_membase, pp32, 0) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(g_cur_vrx218_dev->phy_membase, pp32, 0) = addr;
            *PP32_BRK_DATA_ADDR_MASK(g_cur_vrx218_dev->phy_membase, pp32, 0) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_ON(0, 0);
        }
    }
    else if ( strncasecmp(p, "daddr1 ", 7) == 0 )
    {
        p += 7;
        rlen -= 7;
        if ( strcasecmp(p, "off") == 0 )
        {
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_OFF(1, 1);
            *PP32_BRK_DATA_ADDR_MASK(g_cur_vrx218_dev->phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN;
            *PP32_BRK_DATA_ADDR(g_cur_vrx218_dev->phy_membase, pp32, 1) = 0;
        }
        else
        {
            addr = get_number(&p, &rlen, 1);
            *PP32_BRK_DATA_ADDR(g_cur_vrx218_dev->phy_membase, pp32, 1) = addr;
            *PP32_BRK_DATA_ADDR_MASK(g_cur_vrx218_dev->phy_membase, pp32, 1) = PP32_BRK_CONTEXT_MASK_EN | PP32_BRK_CONTEXT_MASK(0) | PP32_BRK_CONTEXT_MASK(1) | PP32_BRK_CONTEXT_MASK(2) | PP32_BRK_CONTEXT_MASK(3);
            *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) = PP32_BRK_GRPi_DATA_ADDRn_ON(1, 1);
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

    if ( *PP32_BRK_TRIG(g_cur_vrx218_dev->phy_membase, pp32) )
        *PP32_CTRL_OPT(g_cur_vrx218_dev->phy_membase, pp32) = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_ON;
    else
        *PP32_CTRL_OPT(g_cur_vrx218_dev->phy_membase, pp32) = PP32_CTRL_OPT_STOP_ON_BREAKPOINT_OFF;

    return count;
}

static int proc_read_wanmib(struct seq_file *seq, void *v)
{
    int len = 0;
    u32 base = g_cur_vrx218_dev->phy_membase;
    volatile unsigned int *wrx_total_pdu[4]         = {DREG_AR_AIIDLE_CNT0(base),  DREG_AR_HEC_CNT0(base),    DREG_AR_AIIDLE_CNT1(base), DREG_AR_HEC_CNT1(base)};
    volatile unsigned int *wrx_crc_err_pdu[4]       = {GIF0_RX_CRC_ERR_CNT(base),  GIF1_RX_CRC_ERR_CNT(base), GIF2_RX_CRC_ERR_CNT(base), GIF3_RX_CRC_ERR_CNT(base)};
    volatile unsigned int *wrx_cv_cw_cnt[4]         = {GIF0_RX_CV_CNT(base),       GIF1_RX_CV_CNT(base),      GIF2_RX_CV_CNT(base),      GIF3_RX_CV_CNT(base)};
    volatile unsigned int *wrx_bc_overdrop_cnt[2]   = {DREG_B0_OVERDROP_CNT(base), DREG_B1_OVERDROP_CNT(base)};
    int i;
    int idx = (g_cur_vrx218_dev == g_us_vrx218_dev ? 0 : 1);

    seq_printf(seq, "RX (Bearer Channels[0-1]):\n");
    seq_printf(seq, "   wrx_bc_overdrop:");
    for ( i = 0; i < 2; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u",
            *(wrx_bc_overdrop_cnt[i]) >= last_wrx_bc_overdrop_cnt[idx][i]
              ? *(wrx_bc_overdrop_cnt[i]) - last_wrx_bc_overdrop_cnt[idx][i]
              : *(wrx_bc_overdrop_cnt[i]) + ((unsigned int)(-1) - last_wrx_bc_overdrop_cnt[idx][i]));
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "   wrx_bc_user_cw: ");
    for ( i = 0; i < 2; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u", *RECEIVE_NON_IDLE_CELL_CNT((i), base));
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "   wrx_bc_idle_cw: ");
    for ( i = 0; i < 2; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u", *RECEIVE_IDLE_CELL_CNT((i), base));
    }
    seq_printf(seq, "\n");

    seq_printf(seq, "RX (Gamma Interfaces[0-3]):\n");
    seq_printf(seq, "  wrx_total_pdu:   ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u",
            *(wrx_total_pdu[i]) >= last_wrx_total_pdu[idx][i]
            ? *(wrx_total_pdu[i]) - last_wrx_total_pdu[idx][i]
            : *(wrx_total_pdu[i]) + ((unsigned int)(-1) - last_wrx_total_pdu[idx][i]));
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "  wrx_dropdes_pdu: ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u", WAN_RX_MIB_TABLE(i, base)->wrx_dropdes_pdu);
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "  wrx_violated_cw: ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u",
            *(wrx_cv_cw_cnt[i]) >= last_wrx_cv_cw_cnt[idx][i]
            ? *(wrx_cv_cw_cnt[i]) - last_wrx_cv_cw_cnt[idx][i]
            : *(wrx_cv_cw_cnt[i]) + ((unsigned int)(-1) - last_wrx_cv_cw_cnt[idx][i]));
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "  wrx_crc_err_pdu: ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u",
            *(wrx_crc_err_pdu[i]) >= last_wrx_crc_error_total_pdu[idx][i]
            ? *(wrx_crc_err_pdu[i]) - last_wrx_crc_error_total_pdu[idx][i]
            : *(wrx_crc_err_pdu[i]) + ((unsigned int)(-1) - last_wrx_crc_error_total_pdu[idx][i]));
    }
    seq_printf(seq, "\n");
    seq_printf(seq, "  wrx_total_bytes: ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u", WAN_RX_MIB_TABLE(i, base)->wrx_total_bytes);
    }
    seq_printf(seq, "\n");

    seq_printf(seq, "TX (Bearer Channels[0-1]):\n");
    seq_printf(seq, "  total_tx_cw:     %10u, %10u\n",
                    *TRANSMIT_CELL_CNT((0), base),
                    *TRANSMIT_CELL_CNT((1), base));

    seq_printf(seq, "TX (Gamma Interfaces[0-3]):\n");
    seq_printf(seq, "  tx_total_pdu:    ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u",
            WAN_TX_GIF_CFG(i, base)->deq_pkt_cnt >= last_wtx_total_pdu[idx][i]
            ? WAN_TX_GIF_CFG(i, base)->deq_pkt_cnt - last_wtx_total_pdu[idx][i]
            : WAN_TX_GIF_CFG(i, base)->deq_pkt_cnt + ((unsigned int)(-1) - last_wtx_total_pdu[idx][i]));
    }
    seq_printf(seq, "\n");

    seq_printf(seq, "  tx_total_bytes:  ");
    for ( i = 0; i < 4; i++ )
    {
        if ( i != 0 )
            seq_printf(seq, ", ");
        seq_printf(seq, "%10u",
            WAN_TX_GIF_CFG(i, base)->deq_byte_cnt >= last_wtx_total_bytes[idx][i]
            ? WAN_TX_GIF_CFG(i, base)->deq_byte_cnt - last_wtx_total_bytes[idx][i]
            : WAN_TX_GIF_CFG(i, base)->deq_byte_cnt + ((unsigned int)(-1) - last_wtx_total_bytes[idx][i]));
    }
    seq_printf(seq, "\n");

    return len;
}

static ssize_t proc_write_wanmib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[128];
    char *p;
    int len, rlen;

    u32 base = g_cur_vrx218_dev->phy_membase;
    volatile unsigned int *wrx_total_pdu[4]         = {DREG_AR_AIIDLE_CNT0(base),  DREG_AR_HEC_CNT0(base),    DREG_AR_AIIDLE_CNT1(base), DREG_AR_HEC_CNT1(base)};
    volatile unsigned int *wrx_crc_err_pdu[4]       = {GIF0_RX_CRC_ERR_CNT(base),  GIF1_RX_CRC_ERR_CNT(base), GIF2_RX_CRC_ERR_CNT(base), GIF3_RX_CRC_ERR_CNT(base)};
    volatile unsigned int *wrx_cv_cw_cnt[4]         = {GIF0_RX_CV_CNT(base),       GIF1_RX_CV_CNT(base),      GIF2_RX_CV_CNT(base),      GIF3_RX_CV_CNT(base)};
    volatile unsigned int *wrx_bc_overdrop_cnt[2]   = {DREG_B0_OVERDROP_CNT(base), DREG_B1_OVERDROP_CNT(base)};
    int i;
    int idx = g_cur_vrx218_dev == g_us_vrx218_dev ? 0 : 1;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return count;

    if ( strcasecmp(p, "clear") == 0 || strcasecmp(p, "clean") == 0 )
    {
        for ( i = 0; i < 4; i++ )
        {
            last_wrx_total_pdu[idx][i]           = *(wrx_total_pdu[i]);
            last_wrx_crc_error_total_pdu[idx][i] = *(wrx_crc_err_pdu[i]);
            last_wrx_cv_cw_cnt[idx][i]           = *(wrx_cv_cw_cnt[i]);
            last_wtx_total_pdu[idx][i]           = WAN_TX_GIF_CFG(i, base)->deq_pkt_cnt;
            last_wtx_total_bytes[idx][i]         = WAN_TX_GIF_CFG(i, base)->deq_byte_cnt;

            if ( i < 2 )
            {
                last_wrx_bc_overdrop_cnt[idx][i] = *(wrx_bc_overdrop_cnt[i]);
                *RECEIVE_NON_IDLE_CELL_CNT(i,base)       = 0;
                *RECEIVE_IDLE_CELL_CNT(i,base)           = 0;
                *TRANSMIT_CELL_CNT(i,base)               = 0;
            }
            memset((void* )WAN_RX_MIB_TABLE(i,base), 0, sizeof(*WAN_RX_MIB_TABLE(i,base)));
        }
        for ( i = 0; i < VRX218_QOSQ_NUM; i++ )
            memset((void*)WAN_QOS_MIB_TABLE(i,base), 0, sizeof(*WAN_QOS_MIB_TABLE(i,base)));
    }

    return count;
}

static int proc_read_dev(struct seq_file *seq, void *v)
{
    int len = 0;
    int i;

    for ( i = 0; i < ARRAY_SIZE(g_vrx218_dev) && g_cur_vrx218_dev != &g_vrx218_dev[i]; i++ );
    if ( i == ARRAY_SIZE(g_vrx218_dev) )
        seq_printf(seq, "unknown device\n");
    else
        seq_printf(seq, "PCIe device %d\n", g_vrx218_idx[i]);

    return len;
}

static ssize_t proc_write_dev(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[64];
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

    if ( strcasecmp(p, "VRX318_US_BONDING_MASTER") == 0 )
        g_cur_vrx218_dev = g_us_vrx218_dev;
    else if ( strcasecmp(p, "VRX318_DS_BONDING_MASTER") == 0 )
        g_cur_vrx218_dev = g_ds_vrx218_dev;
    else if ( *p >= '0' && *p <= '9' && p[1] == 0 ) {
        int i;

        for ( i = 0; i < ARRAY_SIZE(g_vrx218_idx) && g_vrx218_idx[i] != (int)(*p - '0'); i++ );
        if ( i < ARRAY_SIZE(g_vrx218_idx) )
            g_cur_vrx218_dev = &g_vrx218_dev[i];
        else
            printk("Unknown PCIe device no. - %c\n", *p);
    }
    else
        printk("echo <VRX318_US_BONDING_MASTER | VRX318_DS_BONDING_MASTER | PCIe Device No.> > /proc/eth/vrx318/dev\n");

    return count;
}

#if defined(DEBUG_BONDING_PROC) && DEBUG_BONDING_PROC

static int proc_read_bonding(struct seq_file *seq, void *v)
{
    int len = 0;
    unsigned int base = g_cur_vrx218_dev->phy_membase;
    volatile bond_conf_t  *bconf        = (volatile bond_conf_t  *)SOC_ACCESS_VRX218_SB(__BOND_CONF,  base);
    volatile us_bg_qmap_t *us_bg_qmap   = (volatile us_bg_qmap_t *)SOC_ACCESS_VRX218_SB(__US_BG_QMAP, base);
    volatile us_bg_gmap_t *us_bg_gmap   = (volatile us_bg_gmap_t *)SOC_ACCESS_VRX218_SB(__US_BG_GMAP, base);
    volatile ds_bg_gmap_t *ds_bg_gmap   = (volatile ds_bg_gmap_t *)SOC_ACCESS_VRX218_SB(__DS_BG_GMAP, base);


    seq_printf(seq, "Bonding Config:\n");

    seq_printf(seq, "  BOND_CONF [0x2008]       = 0x%08x, max_frag_size=%u, pctrl_cnt= %u, dplus_fp_fcs_en=%u\n"
                                     "                                     bg_num=%u, bond_mode=%u (%s)\n",
        *(volatile u32 *)bconf, (u32)bconf->max_frag_size, (u32)bconf->polling_ctrl_cnt, (u32)bconf->dplus_fp_fcs_en,
        (u32)bconf->bg_num, (u32)bconf->bond_mode, (u32)bconf->bond_mode ? "L2 Trunking" : "L1 TC");
    seq_printf(seq, "                           =             e1_bond_en=%u, d5_acc_dis=%u, d5_b1_en=%u\n",
        (u32)bconf->e1_bond_en, (u32)bconf->d5_acc_dis, (u32)bconf->d5_b1_en);

    seq_printf(seq, "  US_BG_QMAP[0x2009]       = 0x%08x, qmap0=0x%02x, qmap1=0x%02x, qmap2=0x%02x, qmap3=0x%02x\n",
        *(volatile u32 *)us_bg_qmap, us_bg_qmap->queue_map0, us_bg_qmap->queue_map1, us_bg_qmap->queue_map2, us_bg_qmap->queue_map3);

    seq_printf(seq, "  US_BG_GMAP[0x200A]       = 0x%08x, gmap0=0x%02x, gmap1=0x%02x, gmap2=0x%02x, qmap3=0x%02x\n",
        *(volatile u32 *)us_bg_gmap, us_bg_gmap->gif_map0, us_bg_gmap->gif_map1, us_bg_gmap->gif_map2, us_bg_gmap->gif_map3);

    seq_printf(seq, "  DS_BG_GMAP[0x200B]       = 0x%08x, gmap0=0x%02x, gmap1=0x%02x, gmap2=0x%02x, qmap3=0x%02x\n",
        *(volatile u32 *)ds_bg_gmap, ds_bg_gmap->gif_map0, ds_bg_gmap->gif_map1, ds_bg_gmap->gif_map2, ds_bg_gmap->gif_map3);


    seq_printf(seq, "Cross Pci Debug Info --- Read                  Write \n");
    seq_printf(seq, "    min clock cycle:     %-20u, %u\n",
                                *SOC_ACCESS_VRX218_SB(__READ_MIN_CYCLES, base), *SOC_ACCESS_VRX218_SB(__WRITE_MIN_CYCLES,base));
    seq_printf(seq, "    max clock cycle:     %-20u, %u\n",
                                *SOC_ACCESS_VRX218_SB(__READ_MAX_CYCLES, base), *SOC_ACCESS_VRX218_SB(__WRITE_MAX_CYCLES,base));
    seq_printf(seq, "   total access num:     %-20u, %u\n",
                                *SOC_ACCESS_VRX218_SB(__READ_NUM,base), *SOC_ACCESS_VRX218_SB(__WRITE_NUM,base));
    seq_printf(seq, "     total_cycle_lo:     %-20u, %u\n",
                                *SOC_ACCESS_VRX218_SB(__TOTAL_READ_CYCLES_LO, base),  *SOC_ACCESS_VRX218_SB(__TOTAL_WRITE_CYCLES_LO,base));
    seq_printf(seq, " *total_lo/acc_num*:     %-20u, %u\n",
        *SOC_ACCESS_VRX218_SB(__READ_NUM ,base) ?  *SOC_ACCESS_VRX218_SB(__TOTAL_READ_CYCLES_LO,base)  / *SOC_ACCESS_VRX218_SB(__READ_NUM,base) : 0,
        *SOC_ACCESS_VRX218_SB(__WRITE_NUM,base) ?  *SOC_ACCESS_VRX218_SB(__TOTAL_WRITE_CYCLES_LO,base) / *SOC_ACCESS_VRX218_SB(__WRITE_NUM,base) : 0);
    seq_printf(seq, "  total cycle_hi_lo:     0x%08x%08x,   0x%08x%08x\n",
        *SOC_ACCESS_VRX218_SB(__TOTAL_READ_CYCLES_HI, base), *SOC_ACCESS_VRX218_SB(__TOTAL_READ_CYCLES_LO,base),
        *SOC_ACCESS_VRX218_SB(__TOTAL_WRITE_CYCLES_HI,base), *SOC_ACCESS_VRX218_SB(__TOTAL_WRITE_CYCLES_LO,base));

    return len;
}

static int proc_read_bndmib(struct seq_file *seq, void *v)
{
    int i;
    int len = 0;
    unsigned int base;

    seq_printf(seq, "US_BONDING_GIF_MIB:\n");
    for(i = 0; i < VRX218_GIF_NUM; i ++){
        seq_printf(seq, "   Gif[%d]:   %-u\n",
            i, *US_BOND_GIF_MIB(i, g_us_vrx218_dev->phy_membase));
    }

    base = g_ds_vrx218_dev->phy_membase;
    for(i = 0; i < VRX218_GIF_NUM; i ++){
        seq_printf(seq, "DS_BONDING_GIF_MIB[%d]:\n", i);
        seq_printf(seq, "       Total_rx_frag_cnt:      %10u      Total_rx_byte_cnt: %10u    Overflow_frag_cnt: %10u  Overflow_byte_cnt: %10u\n",
                DS_BOND_GIF_MIB(i,base)->total_rx_frag_cnt, DS_BOND_GIF_MIB(i,base)->total_rx_byte_cnt,
                DS_BOND_GIF_MIB(i,base)->overflow_frag_cnt, DS_BOND_GIF_MIB(i,base)->overflow_byte_cnt);
        seq_printf(seq, "       Out_of_range_frag_cnt:  %10u      Missing_frag_cnt:  %10u    Timeout_frag_cnt:  %10u\n\n",
                DS_BOND_GIF_MIB(i,base)->out_of_range_frag_cnt, DS_BOND_GIF_MIB(i,base)->missing_frag_cnt,
                DS_BOND_GIF_MIB(i,base)->timeout_frag_cnt);
    }

    for(i = 0; i < 4; i ++){
        seq_printf(seq, "DS_BG_MIB[%d]:\n", i);
        seq_printf(seq, "     conform_pkt_cnt:   %10u    conform_frag_cnt:  %10u   conform_byte_cnt: %10u   no_sop_pkt_cnt:    %10u   no_sop_frag_cnt:   %10u\n",
                DS_BG_MIB(i,base)->conform_pkt_cnt, DS_BG_MIB(i,base)->conform_frag_cnt,
                DS_BG_MIB(i,base)->conform_byte_cnt, DS_BG_MIB(i,base)->no_sop_pkt_cnt, DS_BG_MIB(i,base)->no_sop_frag_cnt);
        seq_printf(seq, "     no_sop_byte_cnt:   %10u    no_eop_pkt_cnt:    %10u   no_eop_frag_cnt:  %10u   no_eop_byte_cnt:   %10u   oversize_pkt_cnt:  %10u\n",
                DS_BG_MIB(i,base)->no_sop_byte_cnt, DS_BG_MIB(i,base)->no_eop_pkt_cnt, DS_BG_MIB(i,base)->no_eop_frag_cnt,
                DS_BG_MIB(i,base)->no_eop_byte_cnt, DS_BG_MIB(i,base)->oversize_pkt_cnt);
        seq_printf(seq, "     oversize_frag_cnt: %10u    oversize_byte_cnt: %10u   noncosec_pkt_cnt: %10u   noncosec_frag_cnt: %10u   noncosec_byte_cnt: %10u\n\n",
                DS_BG_MIB(i,base)->oversize_frag_cnt, DS_BG_MIB(i,base)->oversize_byte_cnt, DS_BG_MIB(i,base)->noncosec_pkt_cnt,
                DS_BG_MIB(i,base)->noncosec_frag_cnt, DS_BG_MIB(i,base)->noncosec_byte_cnt);
    }


    return len;
}
static ssize_t proc_write_bndmib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[128];
    char *p;
    int len, rlen;
    int i;
    void *ptr;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return count;

    if ( strcasecmp(p, "clear") == 0 || strcasecmp(p, "clean") == 0 )
    {
        for(i = 0; i < VRX218_GIF_NUM; i ++){
            ptr = (void *)US_BOND_GIF_MIB(i, g_us_vrx218_dev->phy_membase);
            memset(ptr, 0, sizeof(unsigned int));

            ptr = (void *)DS_BOND_GIF_MIB(i, g_ds_vrx218_dev->phy_membase);
            memset(ptr, 0, sizeof(ds_bond_gif_mib_t));

            ptr = (void *)DS_BG_MIB(i, g_ds_vrx218_dev->phy_membase);
            memset(ptr, 0, sizeof(ds_bg_mib_t));
        }
    }else{
        printk("echo [clear|clean] > /proc/eth/vrx318/bondmib\n");
    }

    return count;
}

#endif

/*
 *  Datapath/HAL Hook Function
 */
static int32_t ppe_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    extern int32_t vrx218_qos_ppe_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);

    switch ( cmd )
    {
    case PPA_GENERIC_DATAPATH_GET_PPE_VERION:
    case PPA_GENERIC_HAL_GET_PPE_FW_VERSION:
        {
            PPA_VERSION *version = (PPA_VERSION *)buffer;
            struct fw_ver_id ver;

            if ( version->index >= ARRAY_SIZE(g_vrx218_idx)
                || g_vrx218_idx[version->index] < 0 )
                return PPA_FAILURE;

            ver = *FW_VER_ID(g_vrx218_dev[version->index].phy_membase);
            version->family = ver.family;
            version->type   = ver.package;
            version->major  = ver.major;
            version->mid    = ver.middle;
            version->minor  = ver.minor;
            return PPA_SUCCESS;
        }
#ifdef CONFIG_LTQ_PPA_QOS
        case PPA_GENERIC_HAL_GET_QOS_STATUS:    //get QOS status
        case PPA_GENERIC_HAL_GET_QOS_QUEUE_NUM: //get maximum QOS queue number
        case PPA_GENERIC_HAL_GET_QOS_MIB:       //get maximum QOS queue number
 #ifdef CONFIG_LTQ_PPA_QOS_WFQ
        case PPA_GENERIC_HAL_SET_QOS_WFQ_CTRL:  //enable/disable WFQ
        case PPA_GENERIC_HAL_GET_QOS_WFQ_CTRL:  //get  WFQ status: enabeld/disabled
        case PPA_GENERIC_HAL_SET_QOS_WFQ_CFG:   //set WFQ cfg
        case PPA_GENERIC_HAL_RESET_QOS_WFQ_CFG: //reset WFQ cfg
        case PPA_GENERIC_HAL_GET_QOS_WFQ_CFG:   //get WFQ cfg
        case PPA_GENERIC_HAL_INIT_QOS_WFQ:      // init QOS Rateshapping
 #endif
 #ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
        case PPA_GENERIC_HAL_SET_QOS_RATE_SHAPING_CTRL: //enable/disable Rate shaping
        case PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CTRL: //get Rateshaping status: enabeld/disabled
        case PPA_GENERIC_HAL_SET_QOS_RATE_SHAPING_CFG:  //set rate shaping
        case PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG:  //get rate shaping cfg
        case PPA_GENERIC_HAL_RESET_QOS_RATE_SHAPING_CFG://reset rate shaping cfg
        case PPA_GENERIC_HAL_INIT_QOS_RATE_SHAPING:     //init QOS Rateshapping
 #endif
  #if defined(MBR_CONFIG) && MBR_CONFIG
        case PPA_GENERIC_HAL_SET_QOS_SHAPER_CFG:        //set shaper
        case PPA_GENERIC_HAL_GET_QOS_SHAPER_CFG:        //get shaper
        case PPA_GENERIC_HAL_RESET_QOS_SHAPER_CFG:      //reset shaper
 #endif
            return vrx218_qos_ppe_generic_hook(cmd, buffer, flag);
#endif
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
    int valid_dev_num;
    int i;

    printk("VRX318: loading PPE driver and firmware (PTM)\n");

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

    for ( i = 0, valid_dev_num = 0; i < dev_num; i++ ) {
        if ( ifx_pcie_ep_dev_info_req(i, IFX_PCIE_EP_INT_PPE, &g_vrx218_dev[valid_dev_num]) ) {
            printk("%s failed to get pcie ep %d information\n", __func__, i);
            continue;
        }

        g_vrx218_idx[valid_dev_num] = i;
        if ( ++valid_dev_num == 2 )
            break;
    }

    if ( valid_dev_num == 1 ) {
        g_dsl_bonding = 0;
        g_ds_vrx218_dev = g_us_vrx218_dev;
        init_vrx218_module();
    }
    else if ( valid_dev_num == 2 ) {
        g_bonding_sync_buf_page = get_zeroed_page(GFP_KERNEL);
        if ( g_bonding_sync_buf_page == 0 ) {
            for ( i = 0; i < ARRAY_SIZE(g_vrx218_idx); i++ )
                if ( g_vrx218_idx[i] >= 0 )
                    ifx_pcie_ep_dev_info_release(g_vrx218_idx[i]);
            return -ENOMEM;
        }

        g_dsl_bonding = 2;
        g_ds_vrx218_dev = &g_vrx218_dev[1];
        init_vrx218_bonding_module();
        proc_file_enable("dev", 1);
    }
	init_qos_queue(qos_queue_len);

    register_ppe_ext_generic_hook(ppe_generic_hook, ppe_generic_hook);
    xet_phy_wan_port(7,NULL,1,1);
    proc_file_create();

    printk(KERN_INFO "  total %d device(s) are detected!\n", dev_num);
    if ( valid_dev_num ) {
        if ( g_dsl_bonding )
            printk(KERN_INFO "  %d device(s) are initialized to run in bonding mode!\n", valid_dev_num);
        else
            printk(KERN_INFO "  %d device is initialized to run in single line mode!\n", valid_dev_num);
    }

    return 0;
}

static void __exit ppe_exit(void)
{
    extern int32_t register_ppe_ext_generic_hook(ppe_generic_hook_t datapath_hook, ppe_generic_hook_t hal_hook);

    int i;

    proc_file_delete();

    register_ppe_ext_generic_hook(NULL, NULL);

    if ( g_dsl_bonding ) {
        exit_vrx218_bonding_module();
        free_page(g_bonding_sync_buf_page);
    }
    else
        exit_vrx218_module();

    for ( i = 0; i < ARRAY_SIZE(g_vrx218_idx); i++ )
        if ( g_vrx218_idx[i] >= 0 )
            ifx_pcie_ep_dev_info_release(g_vrx218_idx[i]);
}

#ifndef MODULE
static int __init queue_gamma_map_setup(char *line)
{
    char *p;
    int i;

    for ( i = 0, p = line; i < NUM_ENTITY(queue_gamma_map) && isxdigit(*p); i++ ) {
        queue_gamma_map[i] = simple_strtoul(p, &p, 0);
        if ( *p == ',' || *p == ';' || *p == ':' )
            p++;
    }

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

#endif

module_init(ppe_init);
module_exit(ppe_exit);
#ifndef MODULE
  __setup("queue_gamma_map=", queue_gamma_map_setup);
  __setup("qos_queue_len=", qos_queue_len_setup);

#endif

MODULE_LICENSE("GPL");
