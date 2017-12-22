
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pcie.h>
#else
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
#include "unified_qos_ds_be.h"
#include "vrx218_ppe_ptm_tc_ds.h"
#include "vrx218_e1_addr_def.h"
#include "vrx218_ppe_fw_ds.h"
#include "vrx218_ppe_bonding_ds.h"
#include "vrx218_edma.h"
#include "vrx218_ppe_fw_const.h"
#include "vrx218_regs.h"
#include "vrx218_ptm_common.h"



//////////////////////////////
//          wrapper         //
//////////////////////////////

#define QOSQ_NUM                    8   //  <= 16
#define QOSQ_ID_MASK                ((QOSQ_NUM - 1) | ((QOSQ_NUM - 1) >> 1) | ((QOSQ_NUM - 1) >> 2) | ((QOSQ_NUM - 1) >> 3))
#define QOSQ_PORT_SSID              16    //PORT SHAPER STARD ID
#define QOSQ_L3_SHAPER_ID           20    // All the outqss share one L3 shaper
//PDBRAM Packet Buffer -- 0xA1000000 to 0xA116FFFF
//  Allocate for each packet 1600 Bytes
//  Upstream    : 0x1E098000 - 0x1E09DDBF
//  Dowstream   : 0x1E09DDC0 - 0x1E0A3B7F
#define PDBRAM_TX_PKT_BUFFER_BASE   0x1E098000  // start of packet buffer, TX (upstream) followed by RX (downstream), PTM: 19 + 10, ATM: 16 + 13
#define PDBRAM_TX_PKT_BUFFER_END    0x1E09F6BF
#define PDBRAM_RX_PKT_BUFFER_BASE   0x1E09F6C0
#define PDBRAM_RX_PKT_BUFFER_END    0x1E0A3B7F

static unsigned long g_sync_buf_base = 0;
#define SOC_BOND_US_DES_SYNC_BUF_GIF4   g_sync_buf_base             //  512 DWORDs
#define SOC_BOND_DS_DES_SYNC_BUF        (g_sync_buf_base + 512 * 4) //  256 DWORDs

#define DMA_ALIGNMENT               32
#define DMA_ALIGNMENT_MASK          (DMA_ALIGNMENT - 1)
#define DMA_LENGTH_ALIGN(x)         (((x) + DMA_ALIGNMENT_MASK) & ~DMA_ALIGNMENT_MASK)
#ifdef NET_SKB_PAD_ALLOC
#define SKB_RESERVE_LENGTH          (DMA_LENGTH_ALIGN(NET_SKB_PAD_ALLOC) - NET_SKB_PAD_ALLOC)
#else
#define SKB_RESERVE_LENGTH          (DMA_LENGTH_ALIGN(NET_SKB_PAD) - NET_SKB_PAD)
#endif
static struct sk_buff *alloc_skb_tx(unsigned int len)
{
    struct sk_buff *skb;

    len = (len + DMA_ALIGNMENT - 1) & ~(DMA_ALIGNMENT - 1);

    skb = dev_alloc_skb(len);
    if ( skb )
    {
//#if SKB_RESERVE_LENGTH != 0
if( SKB_RESERVE_LENGTH != 0)
        skb_reserve(skb, SKB_RESERVE_LENGTH);
//#endif
        if ( ((u32)skb->data & (DMA_ALIGNMENT - 1)) != 0 ) {
            panic("%s:%d: unaligned address!", __FUNCTION__, __LINE__);
        }
        *((u32 *)skb->data - 1) = (u32)skb;
#ifndef CONFIG_MIPS_UNCACHED
        dma_cache_wback((u32)skb->data - sizeof(u32), sizeof(u32));
#endif
    }

    return skb;
}

static void *alloc_dataptr_skb(void)
{
    struct sk_buff *skb = alloc_skb_tx(DMA_PACKET_SIZE);

    if ( skb ) {
        put_skb_to_dbg_pool(skb);
        return skb->data;
    }
    else
        return NULL;
}

static struct sk_buff *g_skb_fragments[512] = {0};
static unsigned int g_skb_fragments_count = 0;

static void *alloc_dataptr_fragment(unsigned int bytes)
{
    struct sk_buff *skb = alloc_skb_tx(bytes);

    if ( skb ) {
        ASSERT(g_skb_fragments_count < ARRAY_SIZE(g_skb_fragments), "too many skb fragments");
        g_skb_fragments[g_skb_fragments_count++] = skb;
        return skb->data;
    }
    else
        return NULL;
}

static void __exit free_dataptr_fragments(void)
{
    unsigned int i;

    for ( i = 0; i < g_skb_fragments_count; i++ ) {
        dev_kfree_skb_any(g_skb_fragments[i]);
        g_skb_fragments[i] = NULL;
    }
    g_skb_fragments_count = 0;
}

extern int g_queue_gamma_map[4];

//////////////////////////////
//  start of chiptest code  //
//////////////////////////////

//---------------------------------
//MACROS for FW Address Definitions
//---------------------------------
#define TX_CTRL_K_TABLE(i)                      (__CTRL_K_TBL_BASE + i)
#define TEST_MODE(base)                         ((volatile struct test_mode *)              SOC_ACCESS_VRX218_SB(__TEST_MODE, base))

/*
 * TC Mode indicate to DSL, bit0-ATM,  bit1-PTM, set to 1 when module loaded , reset to zero when unloaded.
 */
#define xTM_MODE_TO_DSL(base)                     SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x7DC4), base)

#define xTM_BC0_SIZE_TO_DSL(base)                     SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x7DC5), base)

//--------------------
//Function Definitions
//--------------------
void dword_mem_clear(void *__addr, unsigned int no_of_bytes) {
    volatile unsigned int *addr = __addr;

    unsigned int i, dw_size;

    dw_size = (no_of_bytes+3)/4;

    for (i=0; i<dw_size; i++)
        *addr++ = 0;

    return;
}

void dword_mem_write(void *__dst_addr, void *__src_addr, unsigned int no_of_bytes) {
    unsigned int *dst_addr = __dst_addr;
    unsigned int *src_addr = __src_addr;

    unsigned int dw_num = (no_of_bytes + 3) >> 2;
    unsigned int i;

    if(no_of_bytes == 0)
        return;

    for (i = 0; i < dw_num; ++ i)
        *dst_addr++ = *src_addr++;

    return;
}

#define DSL_PAGE_SIZE 65
#define DSL_US_BC0 0x50
#define DSL_DS_BC0 0x70

void vrx218_tc_reg_init(unsigned long base)
{
	uint16_t us_bc0 = DSL_US_BC0 * DSL_PAGE_SIZE;
	uint16_t ds_bc0 = DSL_DS_BC0 * DSL_PAGE_SIZE;
	volatile unsigned int *ppe_tc_bc0_size = xTM_BC0_SIZE_TO_DSL(base);

    //*SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base)     = 0x00010040;
    //*SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base)     = 0x00010040;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_PGCNT0, base)   = 0x00020000;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_PGCNT1, base)   = 0x00020000;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AT_CFG0, base)  = 0x000001E0;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AT_CFG1, base)  = 0x000001E0;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AT_IDLE0, base) = 0x00000000;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AT_IDLE1, base) = 0x00000000;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AR_CFG0, base)  = 0x000001F0;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AR_CFG1, base)  = 0x000001F0;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AR_IDLE0, base) = 0x00000000;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_AR_IDLE1, base) = 0x00000000;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_B0_LADR, base)  = 0x0000080C;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_DREG_B1_LADR, base)  = 0x0000080C;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_DBA0, base)     = 0x00003000;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CBA0, base)     = 0x00003EE0;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_DBA1, base)     = 0x00003770;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CBA1, base)     = 0x00003F50;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_FFSM_DBA0, base)     = 0x00002000;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_FFSM_DBA1, base)     = 0x00002550;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base)     = 0x00030070;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base)     = 0x00030070;

    //KEEP IDLE (BC0 & BC1)
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base)     = (* SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base) | (1 << 15));
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base)     = (* SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base) | (1 << 15));

    //Enable SFSM (BC0 & BC1)
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base)     = (* SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base) | (1 << 14));
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base)     = (* SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base) | (1 << 14));

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_FFSM_IDLE_HEAD_BC0, base)    = 0xF0D10000;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_FFSM_IDLE_HEAD_BC1, base)    = 0xF0D10000;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_FFSM_CFG0, base)     = 0x00030050;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_FFSM_CFG1, base)     = 0x00030050;

	// Bit 15 - Bit 0 : US PPE Codeword Buffer Size for BC0
	// Bit 31 - Bit 16: DS PPE Codeword Buffer Size for BC0
	*ppe_tc_bc0_size = (ds_bc0 << 16) + us_bc0;

    return;
}

void vrx218_pdma_init(unsigned long base, int is_bonding)
{
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_CFG, base)                  = 0x00000001;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_CTX_CFG, base)           = 0x00082C00;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_TX_CTX_CFG, base)           = 0x00081B00;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_MAX_LEN_REG, base)       = is_bonding ? 0x02040204 : 0x02040660;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_RX_DELAY_CFG, base)         = 0x000F003F;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_MODE_CFG, base)              = 0x00000011;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_RX_CTX_CFG, base)            = 0x00081200;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_TX_CTX_CFG, base)            = 0x00082E00;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_POLY_CFG_SET0, base)         = 0x00001021;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_POLY_CFG_SET1, base)         = 0x1EDC6F41;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_POLY_CFG_SET2, base)         = 0x04C11DB7;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_CRC_SIZE_CFG, base)          = 0x00000F3E;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_PDMA_RX_CMDBUF_CFG, base)    = 0x01001900;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_PDMA_TX_CMDBUF_CFG, base)    = 0x01001A00;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_PDMA_RX_FW_CMDBUF_CFG, base) = 0x00203FC0;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_PDMA_TX_FW_CMDBUF_CFG, base) = 0x00203FE0;

    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_IER, base)                  = 0x00000007;

    return;
}

void vrx218_tc_hw_init(unsigned long base, int is_bonding)
{
    vrx218_tc_reg_init(base);

    vrx218_pdma_init(base, is_bonding);
}

void vrx218_tc_fw_init(unsigned long base, int is_bonding)
{
    std_des_cfg_t std_des_cfg;
    qos_cfg_t qos_cfg;
    qosq_flow_ctrl_cfg_t qosq_flow_ctrl_cfg;

    volatile struct psave_cfg *psave_cfg;
    volatile struct test_mode *test_mode;

    struct rx_bc_cfg rx_bc_cfg;
    struct tx_bc_cfg tx_bc_cfg;
    struct rx_gamma_itf_cfg rx_gamma_itf_cfg;
    struct tx_gamma_itf_cfg tx_gamma_itf_cfg;

    void *dst_addr;
    int i;

#define DS_FLOW_CTRL_CFG    0x2026
    *SOC_ACCESS_VRX218_SB(DS_FLOW_CTRL_CFG, base) = 0x3C30;

    dword_mem_clear((unsigned int *)(&std_des_cfg), sizeof(std_des_cfg));
    std_des_cfg.byte_off = is_bonding ? 2 : 0;  //  this field replaces byte_off in rx descriptor of VDSL ingress
    std_des_cfg.data_len = DMA_PACKET_SIZE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__STD_DES_CFG, base);
    dword_mem_write(dst_addr, &std_des_cfg, sizeof(std_des_cfg));

    dword_mem_clear((unsigned int *)(&qos_cfg), sizeof(qos_cfg));
    qos_cfg.time_tick = 432000000 / 62500;      //  16 * (cgu_get_pp32_clock() / 1000000), TODO real time clock
    qos_cfg.qosq_num = QOSQ_NUM;
    qos_cfg.qos_en = 0;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__QOS_CFG, base);
    dword_mem_write(dst_addr, &qos_cfg, sizeof(qos_cfg));

    // to be changed to follow the local -> remote copy pattern
    psave_cfg = PSAVE_CFG(base);
    psave_cfg->start_state   = 0;
    psave_cfg->sleep_en      = 1;    //  Enable sleep mode by default

    dword_mem_clear((unsigned int *)(&qosq_flow_ctrl_cfg), sizeof(qosq_flow_ctrl_cfg));
    qosq_flow_ctrl_cfg.large_frame_size = 128;
    qosq_flow_ctrl_cfg.large_frame_drop_th = 16;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__QOSQ_FLOW_CTRL_CFG, base);
    dword_mem_write(dst_addr, &qosq_flow_ctrl_cfg, sizeof(qosq_flow_ctrl_cfg));

    // to be changed to follow the local -> remote copy pattern
    test_mode = TEST_MODE(base);
    test_mode->mib_clear_mode    = 0;
    test_mode->test_mode         = 0;

#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
    *SOC_ACCESS_VRX218_SB(__MAX_PKT_SIZE_CFG,base) = 1604;
#else
    *SOC_ACCESS_VRX218_SB(__MAX_PKT_SIZE_CFG,base) = 1536;
#endif

    dword_mem_clear((unsigned int *)(&rx_bc_cfg), sizeof(rx_bc_cfg));
    rx_bc_cfg.local_state   = 0;
    rx_bc_cfg.remote_state  = 0;
    rx_bc_cfg.to_false_th   = 7;
    rx_bc_cfg.to_looking_th = 3;
    for (i=0; i<2; i++) {
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__RX_BC0_CFG_STATS_CFG + (i*sizeof(rx_bc_cfg)/sizeof(unsigned int))), base);
        dword_mem_write(dst_addr, &rx_bc_cfg, sizeof(rx_bc_cfg));
    }

    dword_mem_clear((unsigned int *)(&tx_bc_cfg), sizeof(tx_bc_cfg));
    tx_bc_cfg.fill_wm = 2;
    tx_bc_cfg.uflw_wm = 2;
    for (i=0; i<2; i++) {
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__TX_BC0_CFG_STATS_CFG + (i*sizeof(tx_bc_cfg)/sizeof(unsigned int))), base);
        dword_mem_write(dst_addr, &tx_bc_cfg, sizeof(tx_bc_cfg));
    }

    dword_mem_clear((unsigned int *)(&rx_gamma_itf_cfg), sizeof(rx_gamma_itf_cfg));
    rx_gamma_itf_cfg.receive_state      = 0;
    rx_gamma_itf_cfg.rx_min_len         = 64;
    rx_gamma_itf_cfg.rx_pad_en          = 1;
    rx_gamma_itf_cfg.rx_eth_fcs_ver_dis = is_bonding ? 1 : 0;  //  disable Ethernet FCS verification during bonding
    rx_gamma_itf_cfg.rx_rm_eth_fcs      = is_bonding ? 0 : 1;  //  disable Ethernet FCS verification during bonding
    rx_gamma_itf_cfg.rx_tc_crc_ver_dis  = 0;
    rx_gamma_itf_cfg.rx_tc_crc_size     = 1;
    rx_gamma_itf_cfg.rx_eth_fcs_result  = 0xC704DD7B;
    rx_gamma_itf_cfg.rx_tc_crc_result   = 0x1D0F1D0F;
    rx_gamma_itf_cfg.rx_crc_cfg         = 0x2500;
    rx_gamma_itf_cfg.rx_eth_fcs_init_value = 0xFFFFFFFF;
    rx_gamma_itf_cfg.rx_tc_crc_init_value  = 0x0000FFFF;
    rx_gamma_itf_cfg.rx_max_len_sel     = 0;
    rx_gamma_itf_cfg.rx_edit_num2       = 0;
    rx_gamma_itf_cfg.rx_edit_pos2       = 0;
    rx_gamma_itf_cfg.rx_edit_type2      = 0;
    rx_gamma_itf_cfg.rx_edit_en2        = 0;
    rx_gamma_itf_cfg.rx_edit_num1       = is_bonding ? 0 : 4;  //  no PMAC header insertion during bonding
    rx_gamma_itf_cfg.rx_edit_pos1       = 0;
    rx_gamma_itf_cfg.rx_edit_type1      = is_bonding ? 0 : 1;  //  no PMAC header insertion during bonding
    rx_gamma_itf_cfg.rx_edit_en1        = is_bonding ? 0 : 1;  //  no PMAC header insertion during bonding
    rx_gamma_itf_cfg.rx_inserted_bytes_1l   = 0x00000007;      //  E5: byte swap of value 0x07000000
    rx_gamma_itf_cfg.rx_inserted_bytes_1h   = 0;
    rx_gamma_itf_cfg.rx_inserted_bytes_2l   = 0;
    rx_gamma_itf_cfg.rx_inserted_bytes_2h   = 0;
    rx_gamma_itf_cfg.rx_len_adj         = -2;
    for (i=0; i<4; i++) {
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__RX_GIF0_CFG_STATS_CFG + (i*sizeof(rx_gamma_itf_cfg)/sizeof(unsigned int))), base);
        dword_mem_write(dst_addr, &rx_gamma_itf_cfg, sizeof(rx_gamma_itf_cfg));
    }

    dword_mem_clear((unsigned int *)(&tx_gamma_itf_cfg), sizeof(tx_gamma_itf_cfg));
    tx_gamma_itf_cfg.tx_len_adj         = is_bonding ? 2 : 6;
    tx_gamma_itf_cfg.tx_crc_off_adj     = 6;
    tx_gamma_itf_cfg.tx_min_len         = 0;
    tx_gamma_itf_cfg.tx_eth_fcs_gen_dis = is_bonding ? 1 : 0;
    tx_gamma_itf_cfg.tx_tc_crc_size     = 1;
    tx_gamma_itf_cfg.tx_crc_cfg         = is_bonding ? 0x2F02 : 0x2F00;
    tx_gamma_itf_cfg.tx_eth_fcs_init_value  = 0xFFFFFFFF;
    tx_gamma_itf_cfg.tx_tc_crc_init_value   = 0x0000FFFF;
    for (i=0; i<4; i++) {
        /*  queue_mapping is reserved in VRX218
        tx_gamma_itf_cfg.queue_mapping = g_queue_gamma_map[i];
        */
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__TX_GIF0_CFG_STATS_CFG + (i*sizeof(tx_gamma_itf_cfg)/sizeof(unsigned int))), base);
        dword_mem_write(dst_addr, &tx_gamma_itf_cfg, sizeof(tx_gamma_itf_cfg));
    }

    /*
    //  TODO: real queue config
    for ( i = 0; i < 8; i++ ) {
        wtx_qos_q_desc_cfg.length = 64;
        wtx_qos_q_desc_cfg.addr   = __US_QOSQ_DES_LIST_BASE + 64 * i * 2;
        *WTX_QOS_Q_DESC_CFG(pcie_port, i) = wtx_qos_q_desc_cfg;
    }
    */

    //Initialize TX Ctrl K Table
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(0), base)  = 0x90111293;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(1), base)  = 0x14959617;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(2), base)  = 0x18999A1B;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(3), base)  = 0x9C1D1E9F;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(4), base)  = 0xA02122A3;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(5), base)  = 0x24A5A627;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(6), base)  = 0x28A9AA2B;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(7), base)  = 0xAC2D2EAF;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(8), base)  = 0x30B1B233;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(9), base)  = 0xB43536B7;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(10), base) = 0xB8393ABB;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(11), base) = 0x3CBDBE3F;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(12), base) = 0xC04142C3;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(13), base) = 0x44C5C647;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(14), base) = 0x48C9CA4B;
    *SOC_ACCESS_VRX218_SB(TX_CTRL_K_TABLE(15), base) = 0xCC4D4ECF;

    return;
}

//Initialize VRX218 PPE FW General Configuration
//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
void vrx218_gen_cfg_init(unsigned long base, int is_bonding, unsigned int vrx218_id, unsigned long peer_base, int cdma_write_data_en)
{
    unsigned int *dst_addr, peer_fw_access_base;
    task_cfg_t task_cfg[2];

    dword_mem_clear((unsigned int *)(&task_cfg), sizeof(task_cfg));

    //Default Task Configuration (Single-Line)
    task_cfg[0].pp32_core_id             = 0;   task_cfg[1].pp32_core_id             = 1;
    task_cfg[0].us_bonding_master        = 0;   task_cfg[1].us_bonding_master        = 0;
    task_cfg[0].us_segment_en            = 0;   task_cfg[1].us_segment_en            = 0;
    task_cfg[0].us_buf_release_en        = 0;   task_cfg[1].us_buf_release_en        = 0;

    task_cfg[0].ds_bonding_master        = 0;   task_cfg[1].ds_bonding_master        = 0;
    task_cfg[0].ds_pkt_dispatch_en       = 0;   task_cfg[1].ds_pkt_dispatch_en       = 0;
    task_cfg[0].ds_pkt_reconstruct_en    = 0;   task_cfg[1].ds_pkt_reconstruct_en    = 0;
    task_cfg[0].ds_pkt_flush_en          = 0;   task_cfg[1].ds_pkt_flush_en          = 0;

    task_cfg[0].tc_us_en                 = 1;   task_cfg[1].tc_us_en                 = 0;
    task_cfg[0].tc_ds_en                 = 1;   task_cfg[1].tc_ds_en                 = 0;

    task_cfg[0].des_sync_en              = 0;   task_cfg[1].des_sync_en              = 1;
    task_cfg[0].edma_write_lle_gen_en    = 0;   task_cfg[1].edma_write_lle_gen_en    = 1;
    task_cfg[0].edma_read_lle_gen_en     = 0;   task_cfg[1].edma_read_lle_gen_en     = 1;
    task_cfg[0].edma_post_proc_en        = 0;   task_cfg[1].edma_post_proc_en        = 1;
    task_cfg[0].qos_wfq_shaping_en       = 0;   task_cfg[1].qos_wfq_shaping_en       = 1;
    task_cfg[0].qos_dispatch_en          = 0;   task_cfg[1].qos_dispatch_en          = 1;
    task_cfg[0].qos_replenish_en         = 0;   task_cfg[1].qos_replenish_en         = 1;   //  enable for QoS

    if (is_bonding == 1) {

        bond_conf_t bond_conf;
        dword_mem_clear((unsigned int *)(&bond_conf), sizeof(bond_conf));

        bond_conf.dplus_fp_fcs_en   = 0x1;
        bond_conf.max_frag_size     = 0x200;
        bond_conf.bg_num            = 0x2;  //  TODO: 0x4 if two bearer channels are enabled
        bond_conf.bond_mode         = 0x0;
        bond_conf.e1_bond_en        = 0x1;
        bond_conf.d5_acc_dis        = 0x1;
        bond_conf.d5_b1_en          = 0x1;

        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__BOND_CONF, base);
        dword_mem_write(dst_addr, &bond_conf, sizeof(bond_conf));

        //Get Peer-Base Address
        //  address = ((IFX_PPE + 0x6000 * 4) + peer_base) | 0x20000000
        //PI concept required to use peer to peer write. pcie switch issue is retained.
        if(cdma_write_data_en){
            peer_fw_access_base = (unsigned int)SOC_ACCESS_VRX218_ADDR(0x1E218000, peer_base);
            *SOC_ACCESS_VRX218_SB(__BOND_PEER_SB_BASE, base) = (0x20000000 | peer_fw_access_base) & 0x3FFFFFFF;
            
        }else{
            //As FW cannot write to the peer due to pcie switch don't support it.
            //Driver will read it out and write to the peer. The peer address will be write to 0 to remind FW to use local address.
            *SOC_ACCESS_VRX218_SB(__BOND_PEER_SB_BASE, base) = 0;
        }
        *SOC_ACCESS_VRX218_SB(__DS_PKT_PMAC_HEADER, base) = 0x07800000;
    }

    if ((is_bonding == 1) && (vrx218_id == VRX218_US_BONDING_MASTER)) {

        *SOC_ACCESS_VRX218_SB(__US_BG_QMAP, base) = 0x0000FE01;
        *SOC_ACCESS_VRX218_SB(__US_BG_GMAP, base) = 0x00002211;

        task_cfg[1].us_bonding_master        = 1;
        task_cfg[1].us_segment_en            = 1;
        task_cfg[1].us_buf_release_en        = 1;

        task_cfg[0].us_bonding_des_sync      = 0;   task_cfg[1].us_bonding_des_sync      = 1;
        task_cfg[0].ds_bonding_des_sync      = 0;   task_cfg[1].ds_bonding_des_sync      = 1;

        task_cfg[0].des_sync_en              = 0;   task_cfg[1].des_sync_en              = 1;
        task_cfg[0].edma_write_lle_gen_en    = 0;   task_cfg[1].edma_write_lle_gen_en    = 1;
        task_cfg[0].edma_read_lle_gen_en     = 0;   task_cfg[1].edma_read_lle_gen_en     = 1;
        task_cfg[0].edma_post_proc_en        = 0;   task_cfg[1].edma_post_proc_en        = 1;

        task_cfg[0].qos_wfq_shaping_en       = 1;   task_cfg[1].qos_wfq_shaping_en       = 0;
        task_cfg[0].qos_dispatch_en          = 1;   task_cfg[1].qos_dispatch_en          = 0;
        task_cfg[0].qos_replenish_en         = 1;   task_cfg[1].qos_replenish_en         = 0;

    } else if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER)) {

        task_cfg[1].ds_bonding_master        = 1;
        task_cfg[1].ds_pkt_dispatch_en       = 1;
        task_cfg[1].ds_pkt_reconstruct_en    = 1;
        task_cfg[1].ds_pkt_flush_en          = 1;

        task_cfg[0].us_bonding_des_sync      = 0;   task_cfg[1].us_bonding_des_sync      = 1;
        task_cfg[0].ds_bonding_des_sync      = 0;   task_cfg[1].ds_bonding_des_sync      = 1;

        task_cfg[0].des_sync_en              = 1;   task_cfg[1].des_sync_en              = 0;
        task_cfg[0].edma_write_lle_gen_en    = 0;   task_cfg[1].edma_write_lle_gen_en    = 1;
        task_cfg[0].edma_read_lle_gen_en     = 0;   task_cfg[1].edma_read_lle_gen_en     = 1;
        task_cfg[0].edma_post_proc_en        = 0;   task_cfg[1].edma_post_proc_en        = 1;

        task_cfg[0].qos_wfq_shaping_en       = 0;   task_cfg[1].qos_wfq_shaping_en       = 0;
        task_cfg[0].qos_dispatch_en          = 0;   task_cfg[1].qos_dispatch_en          = 0;
        task_cfg[0].qos_replenish_en         = 0;   task_cfg[1].qos_replenish_en         = 0;

        *SOC_ACCESS_VRX218_SB(__DS_BG_GMAP, base) = 0x00002211;
    }

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__PP32_0_TASK_CFG, base);
    dword_mem_write(dst_addr, &task_cfg, sizeof(task_cfg));

    //    TODO:
    /*
    * VRX218_SB_BUFFER(__CFG_TX_QOSQ_BANDWIDTH_CTRL, p_vrx218_phy_base)
    * VRX218_SB_BUFFER(__SHAPING_CFG, p_vrx218_phy_base) = 0;
    * VRX218_SB_BUFFER(__WFQ_CFG, p_vrx218_phy_base)     = 0;
    */
}

//Initialize eDMA Configuration Registers and PPE FW eDMA Context
//  Single line applcation:   MUST be called
//  Bonding line application: MUST be called for both VRX218
void vrx218_edma_init(unsigned long base, int lle_in_sb, int cdma_write_data_en)
{
    unsigned int pcie_ep_cfg_addr, *dst_addr;
    edma_ch_ctrl_t edma_ch_ctrl;
    edma_lle_link_t edma_lle_link;
    edma_ch_ctxt_t edma_ch_ctxt;

   //edma hang detection init James
   *SOC_ACCESS_VRX218_SB(0x2514,base)=10;

    pcie_ep_cfg_addr =  base;   //  config space offset is calculated within EDMA register wrappers

    //Initialize eDMA Channel Control Register - Data Structure
    dword_mem_clear(&edma_ch_ctrl, sizeof(edma_ch_ctrl));

    edma_ch_ctrl.at = 0;    // ??? Address Translation (AT)
    edma_ch_ctrl.tc = 0;    // Traffic Class (TC)
    edma_ch_ctrl.td = 0;    // Traffic Digest, the PCIe core adds the ECRC
                            // field and sets the TD bit in TLP header
    edma_ch_ctrl.ro = 1;    // Releaxed Ording
    edma_ch_ctrl.ns = 0;    // No Snoop
    edma_ch_ctrl.fn = 0;    // ??? Function Number (FN) for generated MRd/MWr DMA TLPs
                            // The core uses this when generating the RID for the
                            // MRd/MWr DMA TLP
    edma_ch_ctrl.ll_en = 1; // link list enable
    edma_ch_ctrl.ccs   = 1; // Consumer Cycles State (CCS).
                            // Used in Link List mode only. It is used to synchronize
                            // the Producer (Software) and the Consumer (DMA).
                            // *You must initialize this bit.* The DMA updates
                            // this bit during linked list operation
    edma_ch_ctrl.cs    = 3; // Channel Status (CS)
                            // The Channel Status bits identify the current operational
                            // state of the DMA write or read channel.
                            //  00: Reserved
                            //  01: Running, this channel is active and transferring data
                            //  10: Halted. An error condition has been detected.
                            //      and the DMA has stopped this channel
                            //  11: Stopped. The DMA has transferred all data for
                            //      this channel or you have prematurely stopped
                            //      this channel by writing to the Stop field of
                            //      of the DMA R/W doorbell register

    edma_ch_ctrl.rie  = 0;  // remote interrupt enable
    edma_ch_ctrl.lie  = 0;  // local interrupt enable
    edma_ch_ctrl.llp  = 0;  // load link pointer (LLP)
                            // Used in link list mode only. Indicates that
                            // this linked list element is a link element, and
                            // it's LL element pointer DWORDs are pointint to the next
                            // (non-contiguous) element
                            // the DMA loads this field with the LLP of the
                            // linked list element
    edma_ch_ctrl.tcb  = 0;  // Toggle Cycle Bit (TCB)
                            // Indicates to the DMA to toglle its intepreation of
                            // the CB. Used in linked list mode only. It is used
                            // to synchorize the Producer (Software) and the
                            // Consumer (DMA).
                            // The DMA loads this field with the TCB of the linked
                            // list element.
                            // Note: this field is not defined in a ata LL element
    edma_ch_ctrl.cb  = 0;   // Cycle Bit (CB)
                            // Unsed in Linked list mode only.  It is used
                            // to synchorize the Producer (Software) and the
                            // Consumer (DMA).
                            // The DMA loads this field with the CB of the linked
                            // list elment

    if (cdma_write_data_en == 0) {
        //----------------------------
        //Configure eDMA Write Channel
        //----------------------------

        //Disable eDMA Write Channel
        *EDMA_WCH_EN(pcie_ep_cfg_addr) = 0;

        //Select Write Channel
        *EDMA_CH_IDX(pcie_ep_cfg_addr) = EDMA_WRITE_CH;

        //Setup eDMA Channel Control Register
        dword_mem_write((unsigned int *)(EDMA_CH_CTRL(pcie_ep_cfg_addr)), (unsigned int *)(&edma_ch_ctrl), sizeof(edma_ch_ctrl));

        *EDMA_TRANSFER_SIZE(pcie_ep_cfg_addr)   = 0;
        *EDMA_SAR_LOW(pcie_ep_cfg_addr)         = 0;
        *EDMA_SAR_HIGH(pcie_ep_cfg_addr)        = 0;
        *EDMA_DAR_LOW(pcie_ep_cfg_addr)         = 0;
        *EDMA_DAR_HIGH(pcie_ep_cfg_addr)        = 0;
        if (lle_in_sb == 1)
            *EDMA_LL_PTR_LOW(pcie_ep_cfg_addr)      = VRX218_ACCESS_VRX218_SB(__DS_EDMA_LLE_BASE);
        else
            *EDMA_LL_PTR_LOW(pcie_ep_cfg_addr)      = __DS_EDMA_LLE_FPI_BASE;
        *EDMA_LL_PTR_HIGH(pcie_ep_cfg_addr)     = 0;

        //Setup Write Channel Link List Elements
        dword_mem_clear(&edma_lle_link, sizeof(edma_lle_link));
        edma_lle_link.tcb = 1;
        edma_lle_link.llp = 1;
        if (lle_in_sb == 1)
            edma_lle_link.lle_ptr_low = VRX218_ACCESS_VRX218_SB(__DS_EDMA_LLE_BASE);
        else
            edma_lle_link.lle_ptr_low = __DS_EDMA_LLE_FPI_BASE;

        if (lle_in_sb == 1)
            dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_EDMA_LLE_BASE, base);
        else
            dst_addr = (void *)SOC_ACCESS_VRX218_ADDR(__DS_EDMA_LLE_FPI_BASE, base);
        dword_mem_clear(dst_addr, EDMA_WCH_DATA_LLE_NUM * sizeof(edma_lle_data_t));
        dst_addr = (void *)((unsigned char *)(dst_addr) + (EDMA_WCH_DATA_LLE_NUM * sizeof(edma_lle_data_t)));
        dword_mem_write(dst_addr, &edma_lle_link, sizeof(edma_lle_link));

        //Setup EDMA Write Channel Context
        dword_mem_clear(&edma_ch_ctxt, sizeof(edma_ch_ctxt));
        edma_ch_ctxt.edma_ch_type         = EDMA_WRITE_CH;
        edma_ch_ctxt.edma_pcs             = 1;
        edma_ch_ctxt.edma_lle_num         = EDMA_WCH_DATA_LLE_NUM;
        edma_ch_ctxt.edma_lle_sb_size     = EDMA_WCH_DATA_LLE_NUM * 6;
        edma_ch_ctxt.edma_lle_sb_base     = __DS_EDMA_LLE_BASE;
        edma_ch_ctxt.edma_lle_ext_sb_base = __DS_EDMA_LLE_EXT_BASE;
        if (lle_in_sb == 1)
            edma_ch_ctxt.edma_lle_fpi_base    = VRX218_ACCESS_VRX218_SB(__DS_EDMA_LLE_BASE);
        else
            edma_ch_ctxt.edma_lle_fpi_base    = __DS_EDMA_LLE_FPI_BASE;
        edma_ch_ctxt.edma_ch_status = EDMA_STOPPED;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_EDMA_WRITE_CH_CFG_CTXT, base);
        dword_mem_write(dst_addr, &edma_ch_ctxt, sizeof(edma_ch_ctxt));

        //Enable eDMA Write Channel
        *EDMA_WCH_EN(pcie_ep_cfg_addr) = 1;
    }

    //---------------------------
    //Configure eDMA Read Channel
    //---------------------------

    //Disable eDMA Read Channel
    *EDMA_RCH_EN(pcie_ep_cfg_addr) = 0;

    //Select Read Channel
    *EDMA_CH_IDX(pcie_ep_cfg_addr) = EDMA_READ_CH;

    //Setup eDMA Channel Control Register
    dword_mem_write((unsigned int *)(EDMA_CH_CTRL(pcie_ep_cfg_addr)), (unsigned int *)(&edma_ch_ctrl), sizeof(edma_ch_ctrl));

    *EDMA_TRANSFER_SIZE(pcie_ep_cfg_addr)   = 0;
    *EDMA_SAR_LOW(pcie_ep_cfg_addr)         = 0;
    *EDMA_SAR_HIGH(pcie_ep_cfg_addr)        = 0;
    *EDMA_DAR_LOW(pcie_ep_cfg_addr)         = 0;
    *EDMA_DAR_HIGH(pcie_ep_cfg_addr)        = 0;
    if (lle_in_sb == 1)
        *EDMA_LL_PTR_LOW(pcie_ep_cfg_addr)      = VRX218_ACCESS_VRX218_SB(__US_EDMA_LLE_BASE);
    else
        *EDMA_LL_PTR_LOW(pcie_ep_cfg_addr)      = __US_EDMA_LLE_FPI_BASE;
    *EDMA_LL_PTR_HIGH(pcie_ep_cfg_addr)     = 0;

    //Setup Read Channel Link List Elements
    dword_mem_clear(&edma_lle_link, sizeof(edma_lle_link));
    edma_lle_link.tcb = 1;
    edma_lle_link.llp = 1;
    if (lle_in_sb == 1)
        edma_lle_link.lle_ptr_low = VRX218_ACCESS_VRX218_SB(__US_EDMA_LLE_BASE);
    else
        edma_lle_link.lle_ptr_low = __US_EDMA_LLE_FPI_BASE;

    if (lle_in_sb == 1)
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_EDMA_LLE_BASE, base);
    else
        dst_addr = (void *)SOC_ACCESS_VRX218_ADDR(__US_EDMA_LLE_FPI_BASE, base);
    dword_mem_clear(dst_addr, EDMA_RCH_DATA_LLE_NUM * sizeof(edma_lle_data_t));
    dst_addr = (void *)((unsigned char *)(dst_addr) + (EDMA_RCH_DATA_LLE_NUM * sizeof(edma_lle_data_t)));
    dword_mem_write(dst_addr, &edma_lle_link, sizeof(edma_lle_link));

    //Setup EDMA Read Channel Context
    dword_mem_clear(&edma_ch_ctxt, sizeof(edma_ch_ctxt));
    edma_ch_ctxt.edma_ch_type         = EDMA_READ_CH;
    edma_ch_ctxt.edma_pcs             = 1;
    edma_ch_ctxt.edma_lle_num         = EDMA_RCH_DATA_LLE_NUM;
    edma_ch_ctxt.edma_lle_sb_size     = EDMA_RCH_DATA_LLE_NUM * 6;
    edma_ch_ctxt.edma_lle_sb_base     = __US_EDMA_LLE_BASE;
    edma_ch_ctxt.edma_lle_ext_sb_base = __US_EDMA_LLE_EXT_BASE;
    if (lle_in_sb == 1)
        edma_ch_ctxt.edma_lle_fpi_base    = VRX218_ACCESS_VRX218_SB(__US_EDMA_LLE_BASE);
    else
        edma_ch_ctxt.edma_lle_fpi_base    = __US_EDMA_LLE_FPI_BASE;
    edma_ch_ctxt.edma_ch_status = EDMA_STOPPED;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_EDMA_READ_CH_CFG_CTXT, base);
    dword_mem_write(dst_addr, &edma_ch_ctxt, sizeof(edma_ch_ctxt));

    //Enable eDMA Read Channel
    *EDMA_RCH_EN(pcie_ep_cfg_addr) = 1;

    return;
}


/****************************************************************************
// SHOULD BE INIT BY SOC.D5 driver
//Initialize UpStream INQ Descriptor List (SoC)
//  Performs below actions
//      (1) Allocate and Initialize US CPU Path Descriptors
//      (2) Allocate and Initialize US Fast Path Descriptors
//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
void vrx218_us_inq_des_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int i, *dst_addr;
    tx_descriptor_t tx_descriptor;

    if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER))
        return;

    dword_mem_clear(&tx_descriptor, sizeof(tx_descriptor));

    tx_descriptor.own = 0;
    tx_descriptor.data_len = 1568;

    //Initialize CPU Path Descriptors
    for (i=0; i<64; i++) {
        tx_descriptor.data_ptr = (unsigned int)alloc_dataptr_skb();
        dst_addr = KSEG1ADDR(SOC_US_CPUPATH_DES_BASE + (i * 8));
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    tx_descriptor.own = 1;

    //Initialize Fast Path Descriptors
    for (i=0; i<64; i++) {
        tx_descriptor.data_ptr = (unsigned int)alloc_dataptr_skb();
        dst_addr = KSEG1ADDR(SOC_US_FASTPATH_DES_BASE + (i * 8));
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    return;
}
*********************************************************************************/
//Initialize DownStream Descriptor List (SoC)
//  Performs below actions
//      (1) Allocate and Initialize DownStream Descriptors
//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
void vrx218_ds_soc_des_init(unsigned long base, int is_bonding, unsigned int vrx218_id, int cdma_write_data_en)
{
    unsigned int i, *dst_addr;
    rx_descriptor_t rx_descriptor;

    dword_mem_clear(&rx_descriptor, sizeof(rx_descriptor));

    if (is_bonding == 0) {

        rx_descriptor.own = 1;
        rx_descriptor.sop = 1;
        rx_descriptor.eop = 1;
        rx_descriptor.data_len = DMA_PACKET_SIZE - 32;

        //Initialize DS Descriptors
        for (i=0; i<SOC_DS_DES_NUM; i++) {
            if(cdma_write_data_en){
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)) + VRX218_OUTBOUND_ADDR_BASE;
            }else{
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE));
            }
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_PKT_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
    }
#ifndef VRX218_A1_DRIVER
    else if ((is_bonding == 1) && (vrx218_id == VRX218_US_BONDING_MASTER)) {
        rx_descriptor.own = 1;
        rx_descriptor.byte_off = cdma_write_data_en ? 0 : 2; //CDMA doesn't support byte offset
        rx_descriptor.data_len = DMA_PACKET_SIZE - 32; //used to be 544 for bonding.  now we increase the size in order to support L2 truncking mode

        //Initialize DS Frag Des List Descriptors
        for (i = 0; i < DS_FRAG_DES_LIST1_LEN; i++) {
            if(cdma_write_data_en){
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)) + VRX218_OUTBOUND_ADDR_BASE;//used to 560
            }else{
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE));//used to 560
            }
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
    } else if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER)) {
        //Initialize DS Frag Des List Descriptors
        rx_descriptor.own = 1;
        rx_descriptor.byte_off = cdma_write_data_en ? 0 : 2;
        rx_descriptor.data_len = 0;
        rx_descriptor.data_ptr = 0;
        for (i = 0; i < DS_FRAG_DES_LIST1_LEN; i ++) {
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
        rx_descriptor.own = 1;
        rx_descriptor.byte_off = cdma_write_data_en ? 0 : 2;
        rx_descriptor.data_len = DMA_PACKET_SIZE - 32; //used to be 544 for bonding
        for (i = 0; i < DS_FRAG_DES_LIST2_LEN; i++) {
            if(cdma_write_data_en){
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)) + VRX218_OUTBOUND_ADDR_BASE; //used to be 560
            }else{
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)); //used to be 560
            }
            
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (DS_FRAG_DES_LIST1_LEN_MAX * 2) + (i * 2)), base); //ds_frag_des_list2 0x2730
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }

        //Initialize DS GIF LL Descriptors
        for (i = 0; i < DS_BOND_GIF_LL_DES_LEN; i++) {
            *(unsigned int *) (&rx_descriptor) = ((__DS_BOND_GIF_LL_DES_BA  + 2 * ( (i + 1) & 0xFF )) << 16) | 544;
            if(cdma_write_data_en){
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)) + VRX218_OUTBOUND_ADDR_BASE;; //used to be 560
            }else{
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)); //used to be 560
            }
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_BOND_GIF_LL_DES_BA + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }

        //Initialize DS Packet Des List Descriptors
        dword_mem_clear(&rx_descriptor, sizeof(rx_descriptor));
        rx_descriptor.own = 1;
        rx_descriptor.data_len = DMA_PACKET_SIZE - 32;
        for (i = 0; i < SOC_DS_DES_NUM; i++) {
            if(cdma_write_data_en){ //Note: FW need to addjust the data pointer to remove the VRX218_OUTBOUND_BASE when do the desc sync  
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE)) + VRX218_OUTBOUND_ADDR_BASE;//used to be 560
            }else{
                rx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE));//used to be 560
            }
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_PKT_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
    }
#endif

    return;
}

void vrx218_des_sync_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int *dst_addr;
    des_sync_cfg_ctxt_t des_sync_cfg_ctxt;

    dword_mem_clear((unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));

    if ((is_bonding == 0) || ((is_bonding == 1) && (vrx218_id == VRX218_US_BONDING_MASTER))) {

        //Fast-Path Sync Config/Context
        des_sync_cfg_ctxt.sync_type          = US_READ_WRITE_SYNC;
        //des_sync_cfg_ctxt.us_des_polling_needed = 0;
        des_sync_cfg_ctxt.max_polling_intv   = 4;
        des_sync_cfg_ctxt.desq_cfg_ctxt      = __US_FP_INQ_DES_CFG_CTXT;
        //NOTE: Only for CDMA add 0x20000000
        des_sync_cfg_ctxt.ext_desc_base_addr = 0x20000000 + SOC_US_FASTPATH_DES_BASE;

        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT, base);
        dword_mem_write(dst_addr, (unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));

        //CPU-Path Sync Config/Context
        des_sync_cfg_ctxt.sync_type          = US_READ_WRITE_SYNC;
        //des_sync_cfg_ctxt.us_des_polling_needed = 0;
        des_sync_cfg_ctxt.max_polling_intv   = 10;
        des_sync_cfg_ctxt.desq_cfg_ctxt      = __US_CPU_INQ_DES_CFG_CTXT;
        //NOTE: Only for CDMA add 0x20000000
        des_sync_cfg_ctxt.ext_desc_base_addr = 0x20000000 + SOC_US_CPUPATH_DES_BASE;
		//GRX500 support, set des_sync_cfg_ctxt.bit16 for SOC own bit
		des_sync_cfg_ctxt.soc_des_own_val = 1;

        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 32), base);
        dword_mem_write(dst_addr, (unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));
    }

    if ((is_bonding == 0) || ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER))) {

        //Downstream Sync Config/Context
        des_sync_cfg_ctxt.sync_type          = DS_WRITE_READ_SYNC;
        //des_sync_cfg_ctxt.us_des_polling_needed = 0;
        des_sync_cfg_ctxt.max_polling_intv   = 1;
        des_sync_cfg_ctxt.desq_cfg_ctxt      = __DS_PKT_DESQ_CFG_CTXT;
        //NOTE: Only for CDMA add 0x20000000
        des_sync_cfg_ctxt.ext_desc_base_addr = 0x20000000 + SOC_DS_DES_BASE;
		des_sync_cfg_ctxt.soc_des_own_val = 0;

        if((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER)) {
            dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT, base);
        } else {
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + (2*32)), base);
        }

        dword_mem_write(dst_addr, (unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));
    }

    return;
}

void vrx218_us_fastpath_desq_cfg_ctxt_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    desq_cfg_ctxt_t us_fp_desq_cfg_ctxt;
    tx_descriptor_t *p_tx_descriptor;

    dword_mem_clear((unsigned int *)(&us_fp_desq_cfg_ctxt), sizeof(us_fp_desq_cfg_ctxt));

    //Initialize Up-Stream Fast-Path Descriptor Queue Config/Context
    us_fp_desq_cfg_ctxt.des_in_own_val  = 0;    //  1 - owned by SoC CDMA, 0 - owned by VRX218
    us_fp_desq_cfg_ctxt.fast_path       = 1;
    us_fp_desq_cfg_ctxt.mbox_int_en     = 0;
    us_fp_desq_cfg_ctxt.des_sync_needed = 1;
    us_fp_desq_cfg_ctxt.des_num         = SOC_US_FASTPATH_DES_NUM;  //  max: 64
    us_fp_desq_cfg_ctxt.des_base_addr   = __US_FAST_PATH_DES_LIST_BASE;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_FP_INQ_DES_CFG_CTXT, base);
    dword_mem_write(dst_addr, (unsigned int *)(&us_fp_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    //Initialize OWN bit of all the Descriptors in Shadow Fast Path Descriptor List
    p_tx_descriptor = (tx_descriptor_t *)SOC_ACCESS_VRX218_SB(__US_FAST_PATH_DES_LIST_BASE, base);

    for (i=0; i<us_fp_desq_cfg_ctxt.des_num; i++, p_tx_descriptor++)
        p_tx_descriptor->own = ! us_fp_desq_cfg_ctxt.des_in_own_val;

    return;
}

void vrx218_us_cpupath_desq_cfg_ctxt_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    desq_cfg_ctxt_t us_cp_desq_cfg_ctxt;
    tx_descriptor_t *p_tx_descriptor;

    dword_mem_clear((unsigned int *)(&us_cp_desq_cfg_ctxt), sizeof(us_cp_desq_cfg_ctxt));

    //Initialize Up-Stream CPU-Path Descriptor Queue Config/Context
    us_cp_desq_cfg_ctxt.des_in_own_val  = 1;    //  0 - owned by SoC, 1 - owned by VRX218
    us_cp_desq_cfg_ctxt.fast_path       = 0;
    us_cp_desq_cfg_ctxt.mbox_int_en     = 0;
    us_cp_desq_cfg_ctxt.des_sync_needed = 1;
    us_cp_desq_cfg_ctxt.des_num         = SOC_US_CPUPATH_DES_NUM;   //  max: 64
    us_cp_desq_cfg_ctxt.des_base_addr   = __US_CPU_PATH_DEST_LIST_BASE;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_CPU_INQ_DES_CFG_CTXT, base);
    dword_mem_write(dst_addr, (unsigned int *)(&us_cp_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    //Initialize OWN bit of all the Descriptors in Shadow CPU Path Descriptor List
    p_tx_descriptor = (tx_descriptor_t *)SOC_ACCESS_VRX218_SB(__US_CPU_PATH_DEST_LIST_BASE, base);

    for (i=0; i<us_cp_desq_cfg_ctxt.des_num; i++, p_tx_descriptor++)
        p_tx_descriptor->own = ! us_cp_desq_cfg_ctxt.des_in_own_val;

    return;
}

//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
void vrx218_bonding_des_sync_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int i, *dst_addr;
    bond_des_sync_cfg_ctxt_t bond_des_sync_cfg_ctxt;
    unsigned int j, addr;

    dword_mem_clear((unsigned int *)(&bond_des_sync_cfg_ctxt), sizeof(bond_des_sync_cfg_ctxt));

    if (is_bonding == 0)
        return;

    //Initialize Upstream Bonding Descriptor Synchronization Data Structures
    for (i=0; i<4; i++) {
        bond_des_sync_cfg_ctxt.des_idx = 0;
        bond_des_sync_cfg_ctxt.dir = UPSTREAM;
        bond_des_sync_cfg_ctxt.state = __BOND_DES_SYNC_IDLE_STATE;
        if (vrx218_id == VRX218_US_BONDING_MASTER)
            bond_des_sync_cfg_ctxt.sync_type = WRITE_READ_SYNC;
        else
            bond_des_sync_cfg_ctxt.sync_type = READ_WRITE_SYNC;
        bond_des_sync_cfg_ctxt.desq_cfg_ctxt = (__US_FRAGQ_CFG_CTXT_BASE + (4 *  8)) + (i * 8);

        bond_des_sync_cfg_ctxt.soc_sync_addr = 0x20000000 | CPHYSADDR(SOC_BOND_US_DES_SYNC_BUF_GIF4 + (i * 56 * 4));

        bond_des_sync_cfg_ctxt.cdma_rx_des_dw1 = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_BUF_BASE));

        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_CFG_CTXT + (i * 8), base);
        dword_mem_write(dst_addr, (unsigned int *)(&bond_des_sync_cfg_ctxt), sizeof(bond_des_sync_cfg_ctxt));

        *SOC_ACCESS_VRX218_SB(__US_BOND_SOC_SYNC_ADDR_GIF4 + i, base)         = 0x20000000 | CPHYSADDR(SOC_BOND_US_DES_SYNC_BUF_GIF4 + (i * 56 * 4));
        //*SOC_ACCESS_VRX218_SB(__US_BOND_SOC_SYNC_ENQ_CNT_ADDR_GIF4 + i, base) = 0x20000000 | CPHYSADDR(SOC_BOND_US_DES_SYNC_BUF_GIF4 + (i * 56 * 4) + 0);
        //*SOC_ACCESS_VRX218_SB(__US_BOND_SOC_SYNC_DEQ_CNT_ADDR_GIF4 + i, base) = 0x20000000 | CPHYSADDR(SOC_BOND_US_DES_SYNC_BUF_GIF4 + (i * 56 * 4) + 4);

        for (j=0; j<128; j++) {
            addr = KSEG1ADDR(SOC_BOND_US_DES_SYNC_BUF_GIF4 + (i * 128 * 4) + (j*4));
            *(volatile unsigned int *)addr = 0x00000000;
        }

        //Only Initialize 1 US Bonding Descriptor Synchornization Config/Context
        i = 3;
    }
    *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW1, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_BUF_BASE));

    //Initialize Downstream Bonding Descriptor Synchronization Data Structures
    bond_des_sync_cfg_ctxt.des_idx = 0;
    bond_des_sync_cfg_ctxt.dir = DOWNSTREAM;
    bond_des_sync_cfg_ctxt.state = __BOND_DES_SYNC_IDLE_STATE;
    if (vrx218_id == VRX218_US_BONDING_MASTER)
        bond_des_sync_cfg_ctxt.sync_type = WRITE_READ_SYNC;
    else
        bond_des_sync_cfg_ctxt.sync_type = READ_WRITE_SYNC;
    bond_des_sync_cfg_ctxt.desq_cfg_ctxt = __DS_FRAGQ_CFG_CTXT_BASE;

    bond_des_sync_cfg_ctxt.soc_sync_addr = 0x20000000 | CPHYSADDR(SOC_BOND_DS_DES_SYNC_BUF);

    bond_des_sync_cfg_ctxt.cdma_rx_des_dw1 = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_BUF_BASE));

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT, base);
    dword_mem_write(dst_addr, (unsigned int *)(&bond_des_sync_cfg_ctxt), sizeof(bond_des_sync_cfg_ctxt));

    *SOC_ACCESS_VRX218_SB(__DS_BOND_SOC_SYNC_ADDR, base)         = 0x20000000 | CPHYSADDR(SOC_BOND_DS_DES_SYNC_BUF);
    *SOC_ACCESS_VRX218_SB(__DS_BOND_SOC_SYNC_ENQ_CNT_ADDR, base) = 0x20000000 | CPHYSADDR(SOC_BOND_DS_DES_SYNC_BUF + 0);
    *SOC_ACCESS_VRX218_SB(__DS_BOND_SOC_SYNC_DEQ_CNT_ADDR, base) = 0x20000000 | CPHYSADDR(SOC_BOND_DS_DES_SYNC_BUF + 4);

    for (i=0; i<256; i++) {
        addr = KSEG1ADDR(SOC_BOND_DS_DES_SYNC_BUF + (i*4));
        *(volatile unsigned int *)addr = 0x00000000;
    }

    return;
}

//Initialize QoS related configuration for VRX218
//  Initializes the below PPE FW Data Structures
//      1. INQ_QoS_CFG
//      2. QoSQ_CFG_CTXT
//      3. OUTQ_QoS_CFG_CTXT
//      4. SHAPING_WFQ_CFG
//      5. QOSQ_MIB
//      6. QOSQ_FLOW_CTRL_CFG
//      7. STD_DES_CFG
//      Single line applicaiton : MUST be called
//      Bonding application     : MUST be called for US_BONDING_MASTER
//                                MUST NOT be called for DS_BONDING_MASTER
void vrx218_us_qos_cfg_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int i, *dst_addr;
    inq_qos_cfg_t fp_qos_cfg, cpu_qos_cfg;
    qosq_cfg_ctxt_t qosq_cfg_ctxt;
    outq_qos_cfg_ctxt_t outq_qos_cfg_ctxt;
    desq_cfg_ctxt_t desq_cfg_ctxt;

    if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER))
        return;

    //--------------------------------------
    //Setup INQ_QoS_CFG for Fast-Path & CPU-Path
    //--------------------------------------
    dword_mem_clear((unsigned int *)(&fp_qos_cfg), sizeof(fp_qos_cfg));
    dword_mem_clear((unsigned int *)(&cpu_qos_cfg), sizeof(cpu_qos_cfg));

    //By default, support 8 queues only
    fp_qos_cfg.qos_en         = 1;
    fp_qos_cfg.qid_mask       = QOSQ_ID_MASK;
    fp_qos_cfg.qosq_base_qid  = 0;
    fp_qos_cfg.desq_cfg_ctxt  = __US_FP_INQ_DES_CFG_CTXT;

    cpu_qos_cfg.qos_en        = 1;
    cpu_qos_cfg.qid_mask      = QOSQ_ID_MASK;
    cpu_qos_cfg.qosq_base_qid = 0;
    cpu_qos_cfg.desq_cfg_ctxt = __US_CPU_INQ_DES_CFG_CTXT;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__INQ_QOS_CFG_BASE, base);
    dword_mem_write(dst_addr, (unsigned int *)(&fp_qos_cfg), sizeof(fp_qos_cfg));

    dst_addr = (void *)SOC_ACCESS_VRX218_SB((__INQ_QOS_CFG_BASE + __INQ_QOS_CFG_SIZE), base);
    dword_mem_write(dst_addr, (unsigned int *)(&cpu_qos_cfg), sizeof(cpu_qos_cfg));

    //-----------------------------
    //Setup Fast-Path DESQ_CFG_CTXT
    //-----------------------------
    vrx218_us_fastpath_desq_cfg_ctxt_init(base);

    //----------------------------
    //Setup CPU-Path DESQ_CFG_CTXT
    //----------------------------
    vrx218_us_cpupath_desq_cfg_ctxt_init(base);

    //-------------------
    //Setup QoSQ_CFG_CTXT
    //-------------------
    dword_mem_clear((unsigned int *)(&qosq_cfg_ctxt), sizeof(qosq_cfg_ctxt));

    qosq_cfg_ctxt.threshold = 8;
    qosq_cfg_ctxt.des_num   = 512 / QOSQ_NUM;

    for( i = 0; i < QOSQ_NUM; i ++) {
        qosq_cfg_ctxt.des_base_addr = __US_QOSQ_DES_LIST_BASE + (i * qosq_cfg_ctxt.des_num * 2);
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__QOSQ_CFG_CTXT_BASE + (i * __QOSQ_CFG_CTXT_SIZE)), base);
        dword_mem_write(dst_addr, (unsigned int *)(&qosq_cfg_ctxt), sizeof(qosq_cfg_ctxt));
    }

    //-----------------------
    //Setup OUTQ_QoS_CFG_CTXT
    //-----------------------
    //NOTE: By default, Shaping & WFQ both are DISABLED!!
    dword_mem_clear((unsigned int *)(&outq_qos_cfg_ctxt), sizeof(outq_qos_cfg_ctxt));

    outq_qos_cfg_ctxt.overhd_bytes  = is_bonding ? 20 : 24;

    //Output Queue 0  --reserve for pre-emption queue
    outq_qos_cfg_ctxt.qmap                  = g_queue_gamma_map[3];
    outq_qos_cfg_ctxt.l2_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l2_shaping_cfg_idx    = QOSQ_PORT_SSID;
    outq_qos_cfg_ctxt.l3_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l3_shaping_cfg_idx    = QOSQ_L3_SHAPER_ID;
    outq_qos_cfg_ctxt.desq_cfg_ctxt         = __US_QOS_OUTQ_DES_CFG_CTXT_BASE  + sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__OUTQ_QOS_CFG_CTXT_BASE, base);
    dword_mem_write(dst_addr, (unsigned int *)(&outq_qos_cfg_ctxt), sizeof(outq_qos_cfg_ctxt));

    //Output Queue 1  
    outq_qos_cfg_ctxt.qmap                  = g_queue_gamma_map[2];
    outq_qos_cfg_ctxt.l2_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l2_shaping_cfg_idx    = QOSQ_PORT_SSID + 1;
    outq_qos_cfg_ctxt.l3_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l3_shaping_cfg_idx    = QOSQ_L3_SHAPER_ID;
    outq_qos_cfg_ctxt.desq_cfg_ctxt         = __US_QOS_OUTQ_DES_CFG_CTXT_BASE;
    dst_addr                                += sizeof(outq_qos_cfg_ctxt_t)/sizeof(unsigned int);
    dword_mem_write(dst_addr, (unsigned int *)(&outq_qos_cfg_ctxt), sizeof(outq_qos_cfg_ctxt));

    //Output Queue 2
    outq_qos_cfg_ctxt.qmap                  = g_queue_gamma_map[1];
    outq_qos_cfg_ctxt.l2_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l2_shaping_cfg_idx    = QOSQ_PORT_SSID + 2;
    outq_qos_cfg_ctxt.l3_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l3_shaping_cfg_idx    = QOSQ_L3_SHAPER_ID;
    //outq_qos_cfg_ctxt.desq_cfg_ctxt         += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
    dst_addr                                += sizeof(outq_qos_cfg_ctxt_t)/sizeof(unsigned int);
    dword_mem_write(dst_addr, (unsigned int *)(&outq_qos_cfg_ctxt), sizeof(outq_qos_cfg_ctxt));

    //Output Queue 3
    outq_qos_cfg_ctxt.qmap                  = g_queue_gamma_map[0];
    outq_qos_cfg_ctxt.l2_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l2_shaping_cfg_idx    = QOSQ_PORT_SSID + 3;
    outq_qos_cfg_ctxt.l3_shaping_cfg_ptr    = 0;
    outq_qos_cfg_ctxt.l3_shaping_cfg_idx    = QOSQ_L3_SHAPER_ID;
    //outq_qos_cfg_ctxt.desq_cfg_ctxt         += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
    dst_addr                                += sizeof(outq_qos_cfg_ctxt_t)/sizeof(unsigned int);
    dword_mem_write(dst_addr, (unsigned int *)(&outq_qos_cfg_ctxt), sizeof(outq_qos_cfg_ctxt));

    //-------------------------------------
    //Setup DESQ_CFG_CTXT for Output Queues
    //-------------------------------------
    dword_mem_clear((unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    desq_cfg_ctxt.des_in_own_val    = 1;
    desq_cfg_ctxt.mbox_int_en       = 0;
    desq_cfg_ctxt.des_sync_needed   = 0;
    desq_cfg_ctxt.des_num           = 32; //original 16, temp set to 32 in order to improve performance for 129-byte frame size; Please note, the BC1's memory space is overwritten by BC0
                                          //Can be reduced to reduce performance impact of QoS

    desq_cfg_ctxt.des_base_addr     = __US_OUTQ_DES_LIST_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_QOS_OUTQ_DES_CFG_CTXT_BASE, base);
    dword_mem_write(dst_addr, (unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    desq_cfg_ctxt.des_base_addr     += desq_cfg_ctxt.des_num * 2;
    dst_addr                        += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
    dword_mem_write(dst_addr, (unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    desq_cfg_ctxt.des_base_addr     += desq_cfg_ctxt.des_num * 2;
    dst_addr                        += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
    dword_mem_write(dst_addr, (unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    desq_cfg_ctxt.des_base_addr     += desq_cfg_ctxt.des_num * 2;
    dst_addr                        += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
    dword_mem_write(dst_addr, (unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    return;
}

//Initialize QoS Descriptor List
//  Performs below actions
//      (1) Allocate and Initialize QoS Queue Descriptors
//      (2) Allocate and Initialize Output Queue (OUTQ) Descriptors
void vrx218_us_qos_des_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int i, *dst_addr;
    tx_descriptor_t tx_descriptor;

    if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER))
        return;

    dword_mem_clear(&tx_descriptor, sizeof(tx_descriptor));
    tx_descriptor.own = __QOS_DISPATCH_OWN;
    tx_descriptor.data_len = DMA_PACKET_SIZE - 32;

    //Initialize QoSQ Descriptors
    for (i=0; i<512; i++) {
        tx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_skb());
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_QOSQ_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    tx_descriptor.own = 0;
    //Initialize OUTQ Descriptors
    for (i=0; i<64; i++) {
        tx_descriptor.data_ptr = CPHYSADDR(alloc_dataptr_skb());
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_OUTQ_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    return;
}

void vrx218_us_bg_ctxt_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    us_bg_ctxt_t us_bg_ctxt;

    dword_mem_clear((unsigned int *)(&us_bg_ctxt), sizeof(us_bg_ctxt));
    for (i=0; i<4; i++) {
        us_bg_ctxt.desq_cfg_ctxt_ptr = __US_QOS_OUTQ_DES_CFG_CTXT_BASE + (i*(sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)));
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_BG_CTXT_BASE + (i*(sizeof(us_bg_ctxt_t)/sizeof(unsigned int)))), base);
        dword_mem_write(dst_addr, (unsigned int *)(&us_bg_ctxt), sizeof(us_bg_ctxt_t));
    }

    return;
}

//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
void vrx218_us_fragq_desq_cfg_ctxt_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int i, j, *dst_addr;
    unsigned int start_fragq_id = 0;
    desq_cfg_ctxt_t fragq_desq_cfg_ctxt;
    tx_descriptor_t *p_tx_descriptor;

    if (is_bonding == 0)
        return;

    dword_mem_clear((unsigned int *)(&fragq_desq_cfg_ctxt), sizeof(fragq_desq_cfg_ctxt));

    if (vrx218_id == VRX218_DS_BONDING_MASTER)
        start_fragq_id = 4;

    for (i=start_fragq_id; i<8; i++) {
        //Initialize Upstream Descriptor Queue Config/Context
        fragq_desq_cfg_ctxt.des_in_own_val  = 1;
        fragq_desq_cfg_ctxt.fast_path       = 0;
        fragq_desq_cfg_ctxt.mbox_int_en     = 0;
        fragq_desq_cfg_ctxt.des_sync_needed = 0;
        fragq_desq_cfg_ctxt.gif_id          = i;
        fragq_desq_cfg_ctxt.des_num         = 16;
        fragq_desq_cfg_ctxt.des_base_addr   = (__US_FRAGQ_DES_LIST_BASE + (i*48));
        fragq_desq_cfg_ctxt.bp_des_base_addr = (__US_FRAGQ_DES_LIST_BASE + (i*48) + (16*2));

        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_FRAGQ_CFG_CTXT_BASE + (i*(sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)))), base);
        dword_mem_write(dst_addr, (unsigned int *)(&fragq_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

        //Initialize OWN bit of all the Descriptors in FragmentQ Descriptor List
        p_tx_descriptor = (tx_descriptor_t *)SOC_ACCESS_VRX218_SB((__US_FRAGQ_DES_LIST_BASE + (i*48)), base);

        for (j=0; j<fragq_desq_cfg_ctxt.des_num; j++, p_tx_descriptor++)
            p_tx_descriptor->own = !fragq_desq_cfg_ctxt.des_in_own_val;

        if (vrx218_id == VRX218_DS_BONDING_MASTER) {
            //Initialize the Upstream Shadow Descriptors in SoC
            for (j=0; j<fragq_desq_cfg_ctxt.des_num; j++, p_tx_descriptor++) {
                if (j%4 == 0){
                    p_tx_descriptor = (tx_descriptor_t *)KSEG1ADDR(SOC_BOND_US_DES_SYNC_BUF_GIF4 + ((i-4) * (128 * 4)) + ((j/4)*64) + 8);
                    p_tx_descriptor->own = !fragq_desq_cfg_ctxt.des_in_own_val;
                }
            }
        }
    }

    return;
}

void vrx218_cdma_copy_ch_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    cdma_copy_ch_cfg_t cdma_ch_cfg;
    unsigned int *dst_addr;

    dst_addr = (unsigned int *)SOC_ACCESS_VRX218_SB(__DS_CDMA_COPY_CH_CFG, base);
    if(is_bonding == 0){ //single line
        cdma_ch_cfg.srcq_ctxt_ptr = __DS_TC_LOCAL_Q_CFG_CTXT;
        cdma_ch_cfg.dstq_ctxt_ptr = __DS_PKT_DESQ_CFG_CTXT;
        
    }else if(is_bonding == 1 && vrx218_id == VRX218_US_BONDING_MASTER){
        cdma_ch_cfg.srcq_ctxt_ptr = __DS_TC_LOCAL_Q_CFG_CTXT;
        cdma_ch_cfg.dstq_ctxt_ptr = __DS_FRAGQ_CFG_CTXT_BASE; 
        
    }else if(is_bonding == 1 && vrx218_id == VRX218_DS_BONDING_MASTER){
        cdma_ch_cfg.srcq_ctxt_ptr = __DS_TC_LOCAL_Q_CFG_CTXT;
        cdma_ch_cfg.dstq_ctxt_ptr = __DS_FRAGQ_CFG_CTXT_BASE + 8;
    }

    dword_mem_write(dst_addr,&cdma_ch_cfg,sizeof(cdma_ch_cfg));

    return;
}

void vrx218_edma_copy_ch_init(unsigned long base, int is_bonding, unsigned int vrx218_id, int cdma_write_data_en)
{
    edma_copy_ch_cfg_t copy_ch_cfg;
    desq_cfg_ctxt_t local_desq_cfg_ctxt;
    unsigned int *dst_addr, i;
    unsigned int des_cnt;
#if defined(TEST_SOC_DMA_WRITE) && TEST_SOC_DMA_WRITE
    unsigned int us_des_alloc[] = {15, 0, 0, 0}; 
#else
    unsigned int us_des_alloc[] = {15, 3, 0, 0};
#endif

    if (is_bonding == 0) {

        //Setup 4 UpStream eDMA Copy Channel
        //  1 for each GIF
        copy_ch_cfg.srcq_ctxt_ptr = __US_QOS_OUTQ_DES_CFG_CTXT_BASE;
        copy_ch_cfg.dstq_ctxt_ptr = __US_TC_LOCAL_Q_CFG_CTXT_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_EDMA_COPY_CH_CFG, base);
        dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));

        //Uncomment the below code if you have GIF 1 active!
        for ( i = 0; i < 1; i ++ ) {
            copy_ch_cfg.srcq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            copy_ch_cfg.dstq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            dst_addr                    += sizeof(edma_copy_ch_cfg_t)/sizeof(unsigned int);
            dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
        }
        /*
        //Uncomment the below code if you have GIFs 1 to 3 active!
        for ( i = 0; i < 3; i ++ ) {
            copy_ch_cfg.srcq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            copy_ch_cfg.dstq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            dst_addr                    += sizeof(edma_copy_ch_cfg_t)/sizeof(unsigned int);
            dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
        }
        */

        //Setup the Local DESQ Configuration/Context for 4 UpStream Queues
        dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
        local_desq_cfg_ctxt.des_in_own_val = 1;
        des_cnt = 0;
        for (i=0; i<4; i++) {
            local_desq_cfg_ctxt.des_num = us_des_alloc[i];
            local_desq_cfg_ctxt.des_base_addr = __US_TC_LOCAL_Q_DES_LIST_BASE + (des_cnt * 2);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_TC_LOCAL_Q_CFG_CTXT_BASE + (i * (sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)))), base);
            dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));
            des_cnt = des_cnt + us_des_alloc[i];
        }

        if(cdma_write_data_en==0) {
            //Setup 1 DownStream eDMA Copy Channel
            copy_ch_cfg.srcq_ctxt_ptr = __DS_TC_LOCAL_Q_CFG_CTXT;
            copy_ch_cfg.dstq_ctxt_ptr = __DS_PKT_DESQ_CFG_CTXT;
            dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_EDMA_COPY_CH_CFG, base);
            dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
        }

        //Setup the Local DESQ Configuration/Context for DownStream Queues
        dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
        local_desq_cfg_ctxt.des_in_own_val = cdma_write_data_en ? 1 : 0;
        local_desq_cfg_ctxt.des_num = __DS_TC_LOCAL_Q_DES_LIST_NUM;
        local_desq_cfg_ctxt.des_base_addr = __DS_TC_LOCAL_Q_DES_LIST_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_CFG_CTXT, base);
        dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    } else if ((is_bonding == 1) && (vrx218_id == VRX218_US_BONDING_MASTER)) {

        //Setup 4 UpStream eDMA Copy Channel
        //  1 for each GIF
        copy_ch_cfg.srcq_ctxt_ptr = __US_FRAGQ_CFG_CTXT_BASE;
        copy_ch_cfg.dstq_ctxt_ptr = __US_TC_LOCAL_Q_CFG_CTXT_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_EDMA_COPY_CH_CFG, base);
        dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));

        //Uncomment the below code if you have GIFs 1 to 3 active!
        for ( i = 0; i < 3; i ++ ) {
            copy_ch_cfg.srcq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            copy_ch_cfg.dstq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            dst_addr                    += sizeof(edma_copy_ch_cfg_t)/sizeof(unsigned int);
            dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
        }

        //Setup the Local DESQ Configuration/Context for 4 UpStream Queues
        dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
        local_desq_cfg_ctxt.des_in_own_val = 1;
        des_cnt = 0;
        for (i=0; i<4; i++) {
            local_desq_cfg_ctxt.des_num = us_des_alloc[i];
            local_desq_cfg_ctxt.des_base_addr = __US_TC_LOCAL_Q_DES_LIST_BASE + (des_cnt * 2);
            local_desq_cfg_ctxt.bp_des_base_addr = __US_BP_TC_LOCAL_Q_CFG_CTXT_BASE + (des_cnt * 1);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_TC_LOCAL_Q_CFG_CTXT_BASE + (i * (sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)))), base);
            dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));
            des_cnt = des_cnt + us_des_alloc[i];
        }

        //Setup 1 DownStream eDMA Copy Channel
        copy_ch_cfg.srcq_ctxt_ptr = __DS_TC_LOCAL_Q_CFG_CTXT;
        copy_ch_cfg.dstq_ctxt_ptr = __DS_FRAGQ_CFG_CTXT_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_EDMA_COPY_CH_CFG, base);
        dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));

        //Setup the Local DESQ Configuration/Context for DownStream Queues
        dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
        local_desq_cfg_ctxt.des_in_own_val = cdma_write_data_en ? 1 : 0;
        local_desq_cfg_ctxt.des_num = __DS_TC_LOCAL_Q_DES_LIST_NUM;
        local_desq_cfg_ctxt.des_base_addr = __DS_TC_LOCAL_Q_DES_LIST_BASE;
        local_desq_cfg_ctxt.bp_des_base_addr = __DS_TC_LOCAL_Q_DES_LIST_BASE + (16 * 2);
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_CFG_CTXT, base);
        dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    } else if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER)) {

        //Setup 4 UpStream eDMA Copy Channel
        //  1 for each GIF
        copy_ch_cfg.srcq_ctxt_ptr = __US_FRAGQ_CFG_CTXT_BASE + 4*(sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int));
        copy_ch_cfg.dstq_ctxt_ptr = __US_TC_LOCAL_Q_CFG_CTXT_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_EDMA_COPY_CH_CFG, base);
        dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));

        //Uncomment the below code if you have GIFs 1 to 3 active!
        for ( i = 0; i < 3; i ++ ) {
            copy_ch_cfg.srcq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            copy_ch_cfg.dstq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            dst_addr                    += sizeof(edma_copy_ch_cfg_t)/sizeof(unsigned int);
            dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
        }

        //Setup the Local DESQ Configuration/Context for 4 UpStream Queues
        dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
        local_desq_cfg_ctxt.des_in_own_val = 1;
        des_cnt = 0;
        for (i=0; i<4; i++) {
            local_desq_cfg_ctxt.des_num = us_des_alloc[i];
            local_desq_cfg_ctxt.des_base_addr = __US_TC_LOCAL_Q_DES_LIST_BASE + (des_cnt * 2);
            local_desq_cfg_ctxt.bp_des_base_addr = __US_BP_TC_LOCAL_Q_CFG_CTXT_BASE + (des_cnt * 1);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_TC_LOCAL_Q_CFG_CTXT_BASE + (i * (sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)))), base);
            dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));
            des_cnt = des_cnt + us_des_alloc[i];
        }

        if(cdma_write_data_en==0) {
            //Setup 1 DownStream eDMA Copy Channel
            copy_ch_cfg.srcq_ctxt_ptr = __DS_TC_LOCAL_Q_CFG_CTXT;
            copy_ch_cfg.dstq_ctxt_ptr = __DS_FRAGQ_CFG_CTXT_BASE + sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_EDMA_COPY_CH_CFG, base);
            dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
        }

        //Setup the Local DESQ Configuration/Context for DownStream Queues
        dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
        local_desq_cfg_ctxt.des_in_own_val = cdma_write_data_en ? 1 : 0;
        local_desq_cfg_ctxt.des_num = __DS_TC_LOCAL_Q_DES_LIST_NUM;
        local_desq_cfg_ctxt.des_base_addr = __DS_TC_LOCAL_Q_DES_LIST_BASE;
        local_desq_cfg_ctxt.bp_des_base_addr = __DS_TC_LOCAL_Q_DES_LIST_BASE + (16 * 2);
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_CFG_CTXT, base);
        dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));
    }

    return;
}

//Initialize VRX218 TC Local Descriptor List
//  Performs below actions
//      (1) Allocate and Initialize TC UpStream Local Descriptors
//      (2) Allocate and Initialize TC DownStream Local Descriptors

//Before PPA2.16: US: EDMA read, DS: EDMA write
//PPA2.16: US: EDMA read, DS: CDMA write
void vrx218_local_des_init(unsigned long base, int cdma_write_data_en)
{
    unsigned int i, *dst_addr, ds_local_addr;
    tx_descriptor_t tx_descriptor;
    rx_descriptor_t rx_descriptor;

    dword_mem_clear(&tx_descriptor, sizeof(tx_descriptor));
    tx_descriptor.own = 0;
    tx_descriptor.data_len = DMA_PACKET_SIZE - 32;

    //Initialize UpStream Descriptors
    for (i=0; i<__US_TC_LOCAL_Q_DES_LIST_NUM; i++) {
        tx_descriptor.data_ptr = (PDBRAM_TX_PKT_BUFFER_BASE + (i * DMA_PACKET_SIZE));
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_TC_LOCAL_Q_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    dword_mem_clear(&rx_descriptor, sizeof(rx_descriptor));
    //CDMA: own: 0--->CPU, 1--->DMA
    if(cdma_write_data_en){
        rx_descriptor.own = 0;
    }else{
        rx_descriptor.own = 1;
    }
    rx_descriptor.data_len = DMA_PACKET_SIZE - 32;

    //Initialize DownStream Descriptors
    ds_local_addr = PDBRAM_TX_PKT_BUFFER_BASE + __US_TC_LOCAL_Q_DES_LIST_NUM * DMA_PACKET_SIZE;
    for (i=0; i<__DS_TC_LOCAL_Q_DES_LIST_NUM; i++) {
        rx_descriptor.data_ptr = (ds_local_addr + (i * DMA_PACKET_SIZE));
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_TC_LOCAL_Q_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
    }

    return;
}

//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
//  tc_mode - 0: PTM Mode; 1: A1+ Mode
void vrx218_ds_desq_cfg_ctxt_init(unsigned long base, int is_bonding, unsigned int vrx218_id, int tc_mode)
{
    unsigned int i, *dst_addr;
    desq_cfg_ctxt_t ds_desq_cfg_ctxt;
    rx_descriptor_t *p_rx_descriptor;


    if ((is_bonding == 1) && (vrx218_id == VRX218_US_BONDING_MASTER))
        return;

    dword_mem_clear((unsigned int *)(&ds_desq_cfg_ctxt), sizeof(ds_desq_cfg_ctxt));

    //Initialize Downstream Descriptor Queue Config/Context
    if (is_bonding == 0)
        ds_desq_cfg_ctxt.des_in_own_val  = 0;   //  SoC CDMA TX channel 1 is configured in P2P copy mode, 1 - owned by VRX218, 0 - owned by SoC DMA
    else
        ds_desq_cfg_ctxt.des_in_own_val  = 0;
    ds_desq_cfg_ctxt.fast_path       = 0;
    ds_desq_cfg_ctxt.mbox_int_en     = 0;
    ds_desq_cfg_ctxt.des_sync_needed = 1;
    ds_desq_cfg_ctxt.des_num         = SOC_DS_DES_NUM;  //  max: 64
    ds_desq_cfg_ctxt.des_base_addr   = __DS_PKT_DES_LIST_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_PKT_DESQ_CFG_CTXT, base);

#ifdef VRX218_A1_DRIVER
    if (tc_mode == 1) {
        ds_desq_cfg_ctxt.des_num         = 32;
        ds_desq_cfg_ctxt.des_base_addr   = __DS_AAL5_DES_LIST_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_ALL5_DESQ_CFG_CTXT, base);
    }
#endif

    dword_mem_write(dst_addr, (unsigned int *)(&ds_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    //Initialize OWN bit of all the Descriptors in Shadow DS Descriptor List
    p_rx_descriptor = (rx_descriptor_t *)SOC_ACCESS_VRX218_SB(__DS_PKT_DES_LIST_BASE, base);
#ifdef VRX218_A1_DRIVER
    if (tc_mode == 1)
        p_rx_descriptor = (rx_descriptor_t *)SOC_ACCESS_VRX218_SB(__DS_AAL5_DES_LIST_BASE, base);
#endif

    for (i=0; i<ds_desq_cfg_ctxt.des_num; i++, p_rx_descriptor++)
        p_rx_descriptor->own = ! ds_desq_cfg_ctxt.des_in_own_val;

    return;
}

void vrx218_ds_bg_ctxt_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    ds_bg_ctxt_t ds_bg_ctxt;

    dword_mem_clear((unsigned int *)(&ds_bg_ctxt), sizeof(ds_bg_ctxt_t));
    ds_bg_ctxt.last_eop = 1;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_BG_CTXT_BASE, base);
    for (i = 0; i < 4; i++) {
        dword_mem_write(dst_addr, (unsigned int *)(&ds_bg_ctxt), sizeof(ds_bg_ctxt_t));
        dst_addr = (unsigned int *)dst_addr + sizeof(ds_bg_ctxt_t)/sizeof(unsigned int);
    }

    return;
}

void vrx218_ds_ll_ctxt_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    ds_bond_gif_ll_ctxt_t ds_bond_gif_ll_ctxt, *ds_bond_gif_ll_ctxt_ptr;

    dword_mem_clear((unsigned int *)(&ds_bond_gif_ll_ctxt), sizeof(ds_bond_gif_ll_ctxt_t));

    ds_bond_gif_ll_ctxt.max_des_num     = 128;
    ds_bond_gif_ll_ctxt.to_buff_thres   = 16; // when free_des_num <= 16, timeout any frag
    ds_bond_gif_ll_ctxt.max_delay       = 1000;  //  TODO: fix the number

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_BOND_LL_CTXT_BASE, base);
    for ( i = 0; i < 9; i++ ) {
        dword_mem_write(dst_addr, (unsigned int *)(&ds_bond_gif_ll_ctxt), sizeof(ds_bond_gif_ll_ctxt_t));
        dst_addr = (unsigned int *)dst_addr + sizeof(ds_bond_gif_ll_ctxt_t)/sizeof(unsigned int);
    }

    ds_bond_gif_ll_ctxt_ptr = (void *)SOC_ACCESS_VRX218_SB(__DS_BOND_LL_CTXT_BASE, base);
    ds_bond_gif_ll_ctxt_ptr->head_ptr = __DS_BOND_GIF_LL_DES_BA;
    ds_bond_gif_ll_ctxt_ptr->tail_ptr = __DS_BOND_GIF_LL_DES_BA + (255*2);
    ds_bond_gif_ll_ctxt_ptr->des_num = 256;

    return;
}

//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
void vrx218_ds_fragq_desq_cfg_ctxt_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
{
    unsigned int i, j, *dst_addr;
    unsigned int end_fragq_id = 1;
    desq_cfg_ctxt_t fragq_desq_cfg_ctxt;
    rx_descriptor_t *p_rx_descriptor;

    if (is_bonding == 0)
        return;

    dword_mem_clear((unsigned int *)(&fragq_desq_cfg_ctxt), sizeof(fragq_desq_cfg_ctxt));

    if (vrx218_id == VRX218_DS_BONDING_MASTER)
        end_fragq_id = 2;

    for (i=0; i<end_fragq_id; i++) {
        //Initialize Downstream Descriptor Queue Config/Context
        fragq_desq_cfg_ctxt.des_in_own_val  = 0;
        fragq_desq_cfg_ctxt.fast_path       = 0;
        fragq_desq_cfg_ctxt.mbox_int_en     = 0;
        fragq_desq_cfg_ctxt.des_sync_needed = 0;
        fragq_desq_cfg_ctxt.gif_id          = 0 + ( (i == 1) ? 4 : 0);
        fragq_desq_cfg_ctxt.des_num         = 32;
        fragq_desq_cfg_ctxt.des_base_addr   = (__DS_FRAGQ_DES_LIST_BASE + (i*96));
        fragq_desq_cfg_ctxt.bp_des_base_addr = (__DS_FRAGQ_DES_LIST_BASE + (i*96) + (32*2));

        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_CFG_CTXT_BASE + (i*(sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)))), base);
        dword_mem_write(dst_addr, (unsigned int *)(&fragq_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

        //Initialize OWN bit of all the Descriptors in FragmentQ Descriptor List
        p_rx_descriptor = (rx_descriptor_t *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (i*96)), base);

        for (j=0; j<fragq_desq_cfg_ctxt.des_num; j++, p_rx_descriptor++)
            p_rx_descriptor->own = !fragq_desq_cfg_ctxt.des_in_own_val;

        if (vrx218_id == VRX218_US_BONDING_MASTER) {
            //Initialize the Downstream Shadow Descriptors in SoC
            for (j=0; j<fragq_desq_cfg_ctxt.des_num; j++, p_rx_descriptor++) {
                if (j%4 == 0)
                    p_rx_descriptor = (rx_descriptor_t *)KSEG1ADDR(SOC_BOND_DS_DES_SYNC_BUF + ((j/4) * 64) + 8);
                p_rx_descriptor->own = !fragq_desq_cfg_ctxt.des_in_own_val;
            }
        }
    }

    return;
}

//Initialize CDMA Configuration Registers and PPE FW "Des-Sync Cfg/Ctxt"
//  is_bonding = 0               : single line application
//  is_bonding = 1, vrx218_id = 0: bonding application, us_bonding_master
//  is_bonding = 1, vrx218_id = 1: bonding application, ds_bonding_master
//  tc_mode - 0: PTM Mode; 1: A1+ Mode
void vrx218_cdma_init(unsigned long base, int is_bonding, unsigned int vrx218_id, int cdma_write_data_en, int tc_mode)
{
#ifdef VRX218_A1_DRIVER
    //Ensuring that cdma_write_data_en is '0' when a1plus_en is '1'
    if (tc_mode == 1)
        cdma_write_data_en = 0;
#endif

    //Enable POWER to CDMA
    *SOC_ACCESS_VRX218_ADDR(VRX218_PMU_PWDCR, base) = (*SOC_ACCESS_VRX218_ADDR(VRX218_PMU_PWDCR, base) & (~0x00000004));

    //*SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CTRL, base) = (*SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CTRL, base) | 0x80000000);
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CPOLL, base) = 0x80000040;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_PS, base) = 0x00000004;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_PCTRL, base) = ((*SOC_ACCESS_VRX218_ADDR(VRX218_DMA_PCTRL, base) & ~(0x0000003C)) | (0x0000003C));

    //Setup DMA Channel 0 (RX Channel) : 1 Descriptor
    *SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 10), base) = 0xF0000040;
    *SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 16) & 0x1FFFFFFF);
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x0;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 10) & 0x1FFFFFFF);
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

    //Setup DMA Channel 1 (TX Channel) : 1 Descriptor
    *SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 2), base) = 0x00000000;
    *SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 3), base) = 0x00000000;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x1;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 2) & 0x1FFFFFFF);
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

    //Enable DMA Channel 0 & 1
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x0;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) = *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) | 0x00000001;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x1;
    *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) = *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) | 0x00000001;

    if ((is_bonding == 0) || (is_bonding == 1 && vrx218_id == VRX218_US_BONDING_MASTER)) {
        //Setup DMA Channel 2 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 32) + 10), base) = 0xF0000040;;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 32) + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 32 + 16) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x2;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 32) + 10) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 3 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 32) + 2), base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 32) + 3), base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x3;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 32) + 2) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Enable DMA Channel 2 & 3
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x2;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) = *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) | 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x3;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) = *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) | 0x00000001;
    }

#ifndef VRX218_A1_DRIVER
    if (is_bonding == 1 && vrx218_id == VRX218_DS_BONDING_MASTER) {
        //Setup DMA Channel 2 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW0, base) = 0x00000000;
        //*SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW1, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x2;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW0));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 3 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_TX_DES_DW0, base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_TX_DES_DW1, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x3;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_TX_DES_DW0));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Enable DMA Channel 2 & 3
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x2;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x3;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
    }
#endif

    if (is_bonding == 0) {
        //Setup DMA Channel 4 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 10), base) = 0xF0000040;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 64 + 16) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x4;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 64) + 10) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 5 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 2), base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 3), base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x5;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 64) + 2) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Enable DMA Channel 4 & 5
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x4;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) = *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) | 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x5;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) = *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) | 0x00000001;

        if(cdma_write_data_en){
            //CDMA write ds_tc_local_q_des_list to ds_pkt_des_list(TX CH to RX CH)
            //Setup DMA Channel 6 (RX Channel) : 
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 10), base) = 0xF0000040;
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 64 + 16) & 0x1FFFFFFF);
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base)    = 0x6;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base)   = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_PKT_DES_LIST_BASE));
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = SOC_DS_DES_NUM;

            //Setup DMA Channel 7 (TX Channel) : 
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 2), base) = 0x00000000;
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 3), base) = 0x00000000;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base)    = 0x7;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base)   = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_DES_LIST_BASE));
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = __DS_TC_LOCAL_Q_DES_LIST_NUM;
        }
        
    }

#ifndef VRX218_A1_DRIVER
    if (is_bonding == 1 && vrx218_id == VRX218_US_BONDING_MASTER) {
        //Setup DMA Channel 4 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW0, base) = 0x00000000;
        //*SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW1, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x4;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_RX_DES_DW0));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 5 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_TX_DES_DW0, base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_TX_DES_DW1, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x5;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_US_DES_SYNC_TX_DES_DW0));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Enable DMA Channel 4 & 5
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x4;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x5;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;

         if(cdma_write_data_en){
            //CDMA write ds_tc_local_q_des_list to ds_frag_des_list1(TX CH to RX CH)
            //Setup DMA Channel 6 (RX Channel) : 
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 10), base) = 0xF0000040;
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 64 + 16) & 0x1FFFFFFF);
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base)    = 0x6;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base)   = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_FRAGQ_DES_LIST_BASE));
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = DS_FRAG_DES_LIST1_LEN;

            //Setup DMA Channel 7 (TX Channel) : 
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 2), base) = 0x00000000;
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 3), base) = 0x00000000;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base)    = 0x7;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base)   = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_DES_LIST_BASE));
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = __DS_TC_LOCAL_Q_DES_LIST_NUM;
        }
    }

    if (is_bonding == 1 && vrx218_id == VRX218_DS_BONDING_MASTER) {
        //Setup DMA Channel 4 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 6, base) = 0x00000000;
        //*SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 7, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x4;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 6));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 5 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 4, base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 5, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x5;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 4));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Enable DMA Channel 4 & 5
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x4;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x5;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;

        if(cdma_write_data_en){
            //CDMA write ds_tc_local_q_des_list to ds_frag_des_list2(TX CH to RX CH)
            //Setup DMA Channel 6 (RX Channel) : 
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 10), base) = 0xF0000040;
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 64 + 16) & 0x1FFFFFFF);
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base)    = 0x6;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base)   = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_FRAGQ_DES_LIST_BASE + (DS_FRAG_DES_LIST1_LEN_MAX * 2)));
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = DS_FRAG_DES_LIST2_LEN;

            //Setup DMA Channel 7 (TX Channel) : 
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 2), base) = 0x00000000;
            //*SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 64) + 3), base) = 0x00000000;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base)    = 0x7;
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base)   = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_DES_LIST_BASE));
            *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = __DS_TC_LOCAL_Q_DES_LIST_NUM;
        }
    }

    //if peer to peer is not enabled, then the DMA CH 6 & 7 are used to sync the bonding fragment descriptors
    if (is_bonding == 1 && vrx218_id == VRX218_US_BONDING_MASTER && (cdma_write_data_en == 0)) {
        //Setup DMA Channel 6 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 6, base) = 0x00000000;
        //*SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 7, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 6));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 7 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 4, base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 5, base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__BOND_DS_DES_SYNC_CFG_CTXT + 4));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Enable DMA Channel 6 & 7
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
    }

#if 0
    if (cdma_write_data_en == 1) {
        //Setup DMA Channel 6 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_DES_LIST_BASE)) | 0x20000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 14;

        //Setup DMA Channel 7 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_PKT_DES_LIST_BASE)) | 0x20000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 64;
        //Enable P2PCPY ==> To reverse the OWN Bit definition (0 ==> DMA; 1 ==> PPE)
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x01000000;
    }
#endif // 0 
#endif //not VRX218_A1_DRIVER

#ifdef VRX218_A1_DRIVER
    if (tc_mode == 1) {
        //Setup DMA Channel 6 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 10), base) = 0xF0000040;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 11), base) = ((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 64 + 16) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 96) + 10) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 7 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 2), base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 3), base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = ((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 96) + 2) & 0x1FFFFFFF);
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;
    }
#endif

    if (cdma_write_data_en == 1 || tc_mode == 1) {
        //Enable DMA Channel 6 & 7
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x00000001;
    }

    return;
}

void vrx218_sb_clear(unsigned long base)
{
    dword_mem_clear((unsigned int *)SOC_ACCESS_VRX218_SB(0x2000, base), 0x5000 * 4);
    dword_mem_clear((unsigned int *)SOC_ACCESS_VRX218_SB(0x8000, base), 0x8000 * 4);
}

/*************************************************************************
 *  PTM TC - Initialization : Entry Point
 *  dev         - PCIe endpoint device
 *  peer_dev    - PCIe endpoint device of peer bonding device
 *  is_bonding  - bool, is bonding enabled
 *  vrx218_id   - 0: upstream bonding master, 1: downstream bonding master
 *  sync_buf    - memory base address of bonding US/DS descriptor sync buffer
 *  lle_in_sb   - 0: LLE in PDBRAM, 1: LLE in SB
*************************************************************************/
void __init vrx218_ppe_ptm_init(const ifx_pcie_ep_dev_t *dev, const ifx_pcie_ep_dev_t *peer_dev, int is_bonding, unsigned int vrx218_id, unsigned long sync_buf, int lle_in_sb, int cdma_write_data_en)
{
    unsigned long base = dev->phy_membase;
    unsigned long peer_base;
    volatile unsigned int *ppe_tc_sync = xTM_MODE_TO_DSL(base);

    if ( is_bonding && peer_dev && sync_buf )
    {
        g_sync_buf_base = sync_buf;
        peer_base = peer_dev->phy_membase;
    }
    else
    {
        is_bonding = 0;
        peer_base = 0;
    }

    vrx218_sb_clear(base);

    vrx218_tc_hw_init(base, is_bonding);
    vrx218_tc_fw_init(base, is_bonding);

    vrx218_gen_cfg_init(base, is_bonding, vrx218_id, peer_base,cdma_write_data_en);

    vrx218_edma_init(base, lle_in_sb, cdma_write_data_en);

    //NOTE: To be commented out in Driver
    //          if SoC Driver does the initialization for the US INQ!!
    // vrx218_us_inq_des_init(base, is_bonding, vrx218_id);

    vrx218_ds_soc_des_init(base, is_bonding, vrx218_id, cdma_write_data_en);

    //des sync cfg ctxt init
    vrx218_des_sync_init(base, is_bonding, vrx218_id);

    vrx218_bonding_des_sync_init(base, is_bonding, vrx218_id);

    vrx218_us_qos_cfg_init(base, is_bonding, vrx218_id);
    vrx218_us_qos_des_init(base, is_bonding, vrx218_id);

    if (is_bonding && vrx218_id == VRX218_US_BONDING_MASTER)
        vrx218_us_bg_ctxt_init(base);

    vrx218_us_fragq_desq_cfg_ctxt_init(base, is_bonding, vrx218_id);

    vrx218_edma_copy_ch_init(base, is_bonding, vrx218_id, cdma_write_data_en);

    vrx218_local_des_init(base, cdma_write_data_en);

    vrx218_ds_desq_cfg_ctxt_init(base, is_bonding, vrx218_id, 0);

    if ( is_bonding ) {
        vrx218_ds_bg_ctxt_init(base);
        vrx218_ds_ll_ctxt_init(base);
    }

    vrx218_ds_fragq_desq_cfg_ctxt_init(base, is_bonding, vrx218_id);

#ifdef CONFIG_VRX218_DSL_DFE_LOOPBACK
    //NOTE: Always pretend LINK is UP!!
    //  Needed in DSL-Loopback Mode!!
    //  Will be updated by DSL FW in real-scenario!!
    *SOC_ACCESS_VRX218_SB(0x7DC0, base) = 0x00000003;
    *SOC_ACCESS_VRX218_SB(0x7DD0, base) = 0x00000003;
#endif

    /*indicate current PPE is PTM TC-SYNC to DSL FW to short the DSL negociation*/
    *ppe_tc_sync = (*ppe_tc_sync & ~0x01) | 0x02;

    vrx218_cdma_copy_ch_init(base, is_bonding, vrx218_id);
    vrx218_cdma_init(base, is_bonding, vrx218_id, cdma_write_data_en, 0);
}

unsigned int in_sync(unsigned long base, u8 bc)
{
    unsigned int state = 0;

    if (bc == 0)
        state = *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_STATE0, base) & 0x01;
    else if (bc == 1)
        state = *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_STATE1, base) & 0x01;

    return state;
}

void __exit vrx218_ppe_ptm_fw_exit(const ifx_pcie_ep_dev_t *dev, int is_bonding, unsigned int vrx218_id)
{
    unsigned int base = dev->phy_membase;
    volatile unsigned int *ppe_tc_sync = xTM_MODE_TO_DSL(base);

    /*stop downstream traffic */
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG0, base) &= ~(1 << 14);
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_CFG1, base) &= ~(1 << 14);

    /*clear the indication bit to DSL FW */
    *ppe_tc_sync &= ~0x3;
}

void __exit vrx218_ppe_ptm_free_mem(const ifx_pcie_ep_dev_t *dev, int is_bonding, unsigned int vrx218_id)
{
    unsigned int base = dev->phy_membase;
    unsigned int i, *dst_addr;
    tx_descriptor_t tx_descriptor;
    struct sk_buff *skb_to_free;

    if ( is_bonding == 0 || vrx218_id == VRX218_US_BONDING_MASTER ) {
        //Destroy QoSQ Descriptors
        for (i=0; i<512; i++) {
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_QOSQ_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(&tx_descriptor, dst_addr, sizeof(tx_descriptor));
            skb_to_free = get_skb_pointer(tx_descriptor.data_ptr);
            if ( skb_to_free != NULL )
                dev_kfree_skb_any(skb_to_free);
        }

        //Destroy OUTQ Descriptors
        for (i=0; i<64; i++) {
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_OUTQ_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(&tx_descriptor, dst_addr, sizeof(tx_descriptor));
            skb_to_free = get_skb_pointer(tx_descriptor.data_ptr);
            if ( skb_to_free != NULL )
                dev_kfree_skb_any(skb_to_free);
        }
    }

    free_dataptr_fragments();
}
