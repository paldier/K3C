
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
#include<linux/atmdev.h>

#endif

#include "vrx218_common.h"
#include "vrx_ppe.h"
#include "unified_qos_ds_be.h"
#include "vrx218_ppe_ptm_tc_ds.h"
#include "vrx218_ppe_atm_ds.h"
#include "vrx218_a1plus_addr_def.h"
#include "vrx218_ppe_fw_ds.h"
#include "vrx218_ppe_bonding_ds.h"
#include "vrx218_edma.h"
#include "vrx218_ppe_fw_const.h"
#include "vrx218_regs.h"
#include "vrx218_atm_common.h"



//////////////////////////////
//          wrapper         //
//////////////////////////////

#define VRX218_A1_DRIVER            1

//PDBRAM Packet Buffer -- 0xA1000000 to 0xA116FFFF
//  Allocate for each packet 1600 Bytes
//  Upstream    : 0x1E098000 - 0x1E09DDBF
//  Dowstream   : 0x1E09DDC0 - 0x1E0A3B7F
#define PDBRAM_TX_PKT_BUFFER_BASE   0x1E098000
#define PDBRAM_TX_PKT_BUFFER_END    0x1E09DDBF
#define PDBRAM_RX_PKT_BUFFER_BASE   0x1E09DDC0
#define PDBRAM_RX_PKT_BUFFER_END    0x1E0A3B7F

#define TOTAL_QOS_DES_NUM	    512
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


/**
 * Extern function
 */
extern int atm_txq_num(void);
extern int atm_pvc_num(void);


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

//Initialize eDMA Configuration Registers and PPE FW eDMA Context
//  Single line applcation:   MUST be called
//  Bonding line application: MUST be called for both VRX218
void vrx218_edma_init(unsigned long base, int lle_in_sb, int cdma_write_data_en)
{
    unsigned int pcie_ep_cfg_addr, *dst_addr;
    edma_ch_ctrl_t edma_ch_ctrl;
    edma_lle_link_t edma_lle_link;
    edma_ch_ctxt_t edma_ch_ctxt;

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
void vrx218_ds_soc_des_init(unsigned long base, int is_bonding, unsigned int vrx218_id)
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
            rx_descriptor.data_ptr = (unsigned int)CPHYSADDR(alloc_dataptr_fragment(DMA_PACKET_SIZE));
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_PKT_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
    }
#ifndef VRX218_A1_DRIVER
    else if ((is_bonding == 1) && (vrx218_id == VRX218_US_BONDING_MASTER)) {
        rx_descriptor.own = 1;
        rx_descriptor.byte_off = 2;
        rx_descriptor.data_len = 544;

        //Initialize DS Frag Des List Descriptors
        for (i=0; i<32; i++) {
            rx_descriptor.data_ptr = (unsigned int)alloc_dataptr_fragment(560);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
    } else if ((is_bonding == 1) && (vrx218_id == VRX218_DS_BONDING_MASTER)) {
        //Initialize DS Frag Des List Descriptors
        rx_descriptor.own = 1;
        rx_descriptor.byte_off = 2;
        rx_descriptor.data_len = 0;
        rx_descriptor.data_ptr = 0;
        for (i = 0; i < 32; i ++) {
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
        rx_descriptor.own = 1;
        rx_descriptor.byte_off = 2;
        rx_descriptor.data_len = 544;
        for (i=0; i<32; i++) {
            rx_descriptor.data_ptr = (unsigned int)alloc_dataptr_fragment(560);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_FRAGQ_DES_LIST_BASE + (48 * 2) + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }

        //Initialize DS GIF LL Descriptors
        for (i=0; i<256; i++) {
            * (unsigned int *) (&rx_descriptor) = ((__DS_BOND_GIF_LL_DES_BA  + 2 * ( (i + 1) & 0xFF )) << 16) | 544;
            rx_descriptor.data_ptr = (unsigned int)alloc_dataptr_fragment(560);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_BOND_GIF_LL_DES_BA + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }

        //Initialize DS Packet Des List Descriptors
        dword_mem_clear(&rx_descriptor, sizeof(rx_descriptor));
        rx_descriptor.own = 1;
        rx_descriptor.data_len = 544;
        for (i=0; i<SOC_DS_DES_NUM; i++) {
            rx_descriptor.data_ptr = (unsigned int)alloc_dataptr_fragment(560);
            dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_PKT_DES_LIST_BASE + (i * 2)), base);
            dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
        }
    }
#endif

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
//tc_mode - 0: PTM Mode; 1: A1+ Mode
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
    ds_desq_cfg_ctxt.des_num         = SOC_DS_DES_NUM;      //  max: 64 (for PTM)
    ds_desq_cfg_ctxt.des_base_addr   = __DS_PKT_DES_LIST_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_PKT_DESQ_CFG_CTXT, base);

#ifdef VRX218_A1_DRIVER
    if (tc_mode == 1) {
        ds_desq_cfg_ctxt.des_num         = SOC_DS_DES_NUM;  //  max: 32
        ds_desq_cfg_ctxt.des_base_addr   = __DS_PKT_DES_LIST_BASE;
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_PKT_DESQ_CFG_CTXT, base);
    }
#endif

    dword_mem_write(dst_addr, (unsigned int *)(&ds_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    //Initialize OWN bit of all the Descriptors in Shadow DS Descriptor List
    p_rx_descriptor = (rx_descriptor_t *)SOC_ACCESS_VRX218_SB(__DS_PKT_DES_LIST_BASE, base);
#ifdef VRX218_A1_DRIVER
    if (tc_mode == 1)
        p_rx_descriptor = (rx_descriptor_t *)SOC_ACCESS_VRX218_SB(__DS_PKT_DES_LIST_BASE, base);
#endif

    for (i=0; i<ds_desq_cfg_ctxt.des_num; i++, p_rx_descriptor++)
        p_rx_descriptor->own = ! ds_desq_cfg_ctxt.des_in_own_val;

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
    }

    if (is_bonding == 1 && vrx218_id == VRX218_US_BONDING_MASTER) {
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

    if (cdma_write_data_en == 1) {
        //Setup DMA Channel 6 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_TC_LOCAL_Q_DES_LIST_BASE)) | 0x20000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 14;

        //Setup DMA Channel 7 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR(VRX218_ACCESS_VRX218_SB(__DS_PKT_DES_LIST_BASE)) | 0x20000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = SOC_DS_DES_NUM * 2;
        //Enable P2PCPY ==> To reverse the OWN Bit definition (0 ==> DMA; 1 ==> PPE)
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CCTRL, base) |= 0x01000000;
    }
#endif

    //CFG CTXT Address Sequence: each des_sync_cfg_ctxt_t structure size is 32 dw
    //From: __DES_SYNC_CFG_CTXT: 
    // 0: Fastpath, 1: CPUPATH, 2: __DS_PKT_DESQ_CFG_CTXT, 3: __DS_OAM_DESQ_CFG_CTXT
#ifdef VRX218_A1_DRIVER
    if (tc_mode == 1) {
        //Setup DMA Channel 6 (RX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 10), base) = 0xF0000040;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 11), base) = CPHYSADDR((unsigned int)VRX218_ACCESS_VRX218_SB(__DES_SYNC_CFG_CTXT + 96 + 16));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x6;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 96) + 10));
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CDLEN, base) = 1;

        //Setup DMA Channel 7 (TX Channel) : 1 Descriptor
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 2), base) = 0x00000000;
        *SOC_ACCESS_VRX218_SB(((__DES_SYNC_CFG_CTXT + 96) + 3), base) = 0x00000000;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CS, base) = 0x7;
        *SOC_ACCESS_VRX218_ADDR(VRX218_DMA_CBA, base) = CPHYSADDR((unsigned int)VRX218_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + 96) + 2));
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

#ifdef VRX218_A1_DRIVER
/*******************************************
 * ATM TC - Initialization Support Functions
*******************************************/
void vrx218_atm_pdma_init(unsigned long base)
{
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_PDMA_CFG, base)                  = 0x00000008;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_PDMA_RX_CMDBUF_CFG, base)    = 0x00203580;
    *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SAR_PDMA_RX_FW_CMDBUF_CFG, base) = 0x004035A0;

    return;
}

void vrx218_atm_fw_init(unsigned long base)
{
    unsigned int *dst_addr;
    qos_cfg_t qos_cfg;
    struct psave_cfg ps_cfg; 
    
#define DS_FLOW_CTRL_CFG    0x2026
    *SOC_ACCESS_VRX218_SB(DS_FLOW_CTRL_CFG, base) = 0x3C30;

    dword_mem_clear((unsigned int *)(&qos_cfg), sizeof(qos_cfg));
    //PPE FW use this timer to wake up in the sleep mode.  It's the only way to wake up FW in the ATM mode.
    qos_cfg.time_tick = 2304;   // cgu_get_pp32_clock() / 62500 / 3 
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__QOS_CFG, base);
    dword_mem_write(dst_addr, &qos_cfg, sizeof(qos_cfg));

    // to be changed to follow the local -> remote copy pattern
    dword_mem_clear((unsigned int *)(&ps_cfg),sizeof(ps_cfg));
    ps_cfg.start_state   = 0;
    ps_cfg.sleep_en      = 1;    //  Enable sleep mode by default
    dst_addr = (void *)PSAVE_CFG(base);
    dword_mem_write(dst_addr,&ps_cfg,sizeof(ps_cfg));

    *SOC_ACCESS_VRX218_SB(__CFG_WRX_HTUTS, base)        = 15 + 3;
    *SOC_ACCESS_VRX218_SB(__CFG_WRX_DMACH_ON, base)     = 0x03;
    *SOC_ACCESS_VRX218_SB(__CFG_WRX_HUNT_BITTH, base)   = 4;

    return;
}

void vrx218_atm_htu_init(unsigned long base)
{
    /* OAM_F4_SEG_HTU_ENTRY */
    *SOC_ACCESS_VRX218_SB(__HTU_ENTRY_TABLE, base)  = 0x00000031;
    *SOC_ACCESS_VRX218_SB(__HTU_MASK_TABLE, base)   = 0xfff0000e;
    *SOC_ACCESS_VRX218_SB(__HTU_RESULT_TABLE, base) = 0x00000600;

    /* OAM_F4_TOT_HTU_ENTRY */
    *SOC_ACCESS_VRX218_SB(__HTU_ENTRY_TABLE + 1, base) = 0x00000041;
    *SOC_ACCESS_VRX218_SB(__HTU_MASK_TABLE  + 1, base) = 0xfff0000e;
    *SOC_ACCESS_VRX218_SB(__HTU_RESULT_TABLE+ 1, base) = 0x00000600;

    /* OAM_F5_HTU_ENTRY */
    *SOC_ACCESS_VRX218_SB(__HTU_ENTRY_TABLE + 1, base) = 0x00000009;
    *SOC_ACCESS_VRX218_SB(__HTU_MASK_TABLE  + 1, base) = 0xfffffff2;
    *SOC_ACCESS_VRX218_SB(__HTU_RESULT_TABLE+ 1, base) = 0x00000600;

    return;
}

void vrx218_atm_wtx_port_cfg_init(unsigned long base, u8 qsb_en)
{
    unsigned int *dst_addr;
    wtx_port_config_t wtx_port_config = {0};

    wtx_port_config.qsben = qsb_en;
    //BC0: Use TX Queue 0 - if QSB is Disabled
    wtx_port_config.qid = 0;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__WTX_PORT_CONFIG0, base);
    dword_mem_write(dst_addr, &wtx_port_config, sizeof(wtx_port_config));
    //BC1: Use TX Queue 1 - if QSB is Disabled
    wtx_port_config.qid = 1;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__WTX_PORT_CONFIG1, base);
    dword_mem_write(dst_addr, &wtx_port_config, sizeof(wtx_port_config));

    return;
}

void vrx218_atm_wtx_queue_cfg_init(unsigned long base, u8 qsb_en)
{
    unsigned int *dst_addr, i;
    wtx_queue_config_t wtx_queue_config = {0};

    wtx_queue_config.same_vc_qmap = 0;
    wtx_queue_config.uu           = 0;
    wtx_queue_config.cpi          = 0;
    wtx_queue_config.sbid         = 0;
    wtx_queue_config.qsb_vcid     = 0; //  Which QSB queue (VCID) does this TX queue map to.
    wtx_queue_config.mpoa_mode    = 0; //  0: VCmux, 1: LLC
    wtx_queue_config.qsben        = qsb_en;
    wtx_queue_config.atm_header   = 0;

    for (i = 0; i < 16; i++) {
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__WTX_QUEUE_CONFIG + 25 * i, base);
        dword_mem_write(dst_addr, &wtx_queue_config, sizeof(wtx_queue_config_t));
    }

    return;
}

void vrx218_atm_wrx_queue_cfg_init(unsigned long base)
{
    unsigned int *dst_addr, i;
    wrx_queue_config_t wrx_queue_config = {0};

    wrx_queue_config.new_vlan  = 0;
    wrx_queue_config.vlan_ins  = 0;
    wrx_queue_config.mpoa_type = 3; //0: EoA without FCS, 1: EoA with FCS, 2: PPPoA, 3:IPoA
    wrx_queue_config.ip_ver    = 0; //0: IPv4, 1: IPv6
    wrx_queue_config.mpoa_mode = 0; //0: VCmux, 1: LLC
    wrx_queue_config.oversize  = 1600;
    wrx_queue_config.undersize = 0;
    wrx_queue_config.mfs       = 1600;
    wrx_queue_config.uumask    = 0xFF;
    wrx_queue_config.cpimask   = 0xFF;
    wrx_queue_config.uuexp     = 0;
    wrx_queue_config.cpiexp    = 0;

    for (i = 0; i < 2; i++) {
        dst_addr = (void *)SOC_ACCESS_VRX218_SB(__WRX_QUEUE_CONFIG + 10 * i, base);
        dword_mem_write(dst_addr, &wrx_queue_config, sizeof(wrx_queue_config_t));
    }

    return;
}

void vrx218_ds_oam_desq_cfg_ctxt_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    desq_cfg_ctxt_t ds_desq_cfg_ctxt;
    rx_descriptor_t *p_rx_descriptor;
    rx_descriptor_t rx_descriptor = {0};

    dword_mem_clear((unsigned int *)(&ds_desq_cfg_ctxt), sizeof(ds_desq_cfg_ctxt));

    //Initialize Downstream OAM Descriptor Queue Config/Context
    ds_desq_cfg_ctxt.des_in_own_val  = 1;
    ds_desq_cfg_ctxt.fast_path       = 0;
    ds_desq_cfg_ctxt.mbox_int_en     = 0;
    ds_desq_cfg_ctxt.des_sync_needed = 0;   //  not copied to host
    ds_desq_cfg_ctxt.des_num         = SOC_DS_OAM_DES_NUM;
    ds_desq_cfg_ctxt.des_base_addr   = (unsigned int)__DS_OAM_DES_LIST_BASE;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_OAM_DESQ_CFG_CTXT, base);
    dword_mem_write(dst_addr, (unsigned int *)(&ds_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    //Initialize OWN bit of all the Descriptors in Shadow DS Descriptor List
    p_rx_descriptor = (rx_descriptor_t *)SOC_ACCESS_VRX218_SB(__DS_OAM_DES_LIST_BASE, base);

    rx_descriptor.own = ! ds_desq_cfg_ctxt.des_in_own_val;
    rx_descriptor.sop = 1;
    rx_descriptor.eop = 1;
    rx_descriptor.data_len = 0;
    for (i=0; i<ds_desq_cfg_ctxt.des_num; i++, p_rx_descriptor++) {
        //no OAM buffer, this queue is not synced with local queue
        //rx_descriptor.data_ptr = (unsigned int)alloc_dataptr_fragment(64);
        rx_descriptor.data_ptr = (unsigned int)CPHYSADDR(alloc_dataptr_fragment(64));
        *p_rx_descriptor = rx_descriptor;
    }

    return;
}

void vrx218_atm_us_qos_des_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    tx_descriptor_t tx_descriptor;

    dword_mem_clear(&tx_descriptor, sizeof(tx_descriptor));
    tx_descriptor.own = __QOS_DISPATCH_OWN;
    tx_descriptor.data_len = DMA_PACKET_SIZE - 32;

    //Initialize QoSQ Descriptors
    for (i=0; i<512; i++) {
        tx_descriptor.data_ptr = (unsigned int)CPHYSADDR(alloc_dataptr_skb());
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_QOSQ_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    return;
}

//Initialize QoS related configuration for VRX218
//  Initializes the below PPE FW Data Structures
//      1. INQ_QoS_CFG
//      2. QoSQ_CFG_CTXT
//      4. SHAPING_WFQ_CFG
//      5. QOSQ_MIB
//      6. QOSQ_FLOW_CTRL_CFG
//      7. STD_DES_CFG
void vrx218_atm_us_qos_cfg_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    qosq_flow_ctrl_cfg_t qosq_flow_ctrl_cfg;
    std_des_cfg_t std_des_cfg;
    inq_qos_cfg_t fp_qos_cfg, cpu_qos_cfg;
    qosq_cfg_ctxt_t qosq_cfg_ctxt;
//    outq_qos_cfg_ctxt_t outq_qos_cfg_ctxt;
    desq_cfg_ctxt_t desq_cfg_ctxt;

    //Initialize QOSQ_FLOW_CTRL_CFG
    dword_mem_clear((unsigned int *)(&qosq_flow_ctrl_cfg), sizeof(qosq_flow_ctrl_cfg));
    qosq_flow_ctrl_cfg.large_frame_size = 1024;
    qosq_flow_ctrl_cfg.large_frame_drop_th = 28;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__QOSQ_FLOW_CTRL_CFG, base);
    dword_mem_write(dst_addr, (unsigned int *)(&qosq_flow_ctrl_cfg), sizeof(qosq_flow_ctrl_cfg));

    //Initialize STD_DES_CFG
    dword_mem_clear((unsigned int *)(&std_des_cfg), sizeof(std_des_cfg));
    std_des_cfg.byte_off = 0;
    std_des_cfg.data_len = DMA_PACKET_SIZE - 32;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__STD_DES_CFG, base);
    dword_mem_write(dst_addr, (unsigned int *)(&std_des_cfg), sizeof(std_des_cfg));

    //--------------------------------------
    //Setup INQ_QoS_CFG for Fast-Path & CPU-Path
    //--------------------------------------
    dword_mem_clear((unsigned int *)(&fp_qos_cfg), sizeof(fp_qos_cfg));
    dword_mem_clear((unsigned int *)(&cpu_qos_cfg), sizeof(cpu_qos_cfg));

    //By default, support 8 queues only
    fp_qos_cfg.qos_en         = 1;
    fp_qos_cfg.qid_mask       = 0xF;
    fp_qos_cfg.qosq_base_qid  = 0;
    fp_qos_cfg.desq_cfg_ctxt  = __US_FP_INQ_DES_CFG_CTXT;

    cpu_qos_cfg.qos_en        = 1;
    cpu_qos_cfg.qid_mask      = 0xF;
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
    qosq_cfg_ctxt.des_num   = TOTAL_QOS_DES_NUM / atm_txq_num();

    for( i = 0; i < atm_txq_num(); i ++) {
        qosq_cfg_ctxt.des_base_addr = __US_QOSQ_DES_LIST_BASE + (i * qosq_cfg_ctxt.des_num * 2);
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__QOSQ_CFG_CTXT_BASE + (i * __QOSQ_CFG_CTXT_SIZE)), base);
        dword_mem_write(dst_addr, (unsigned int *)(&qosq_cfg_ctxt), sizeof(qosq_cfg_ctxt));
    }

    //------------------------------
    //Setup QoSQ PSEUDO DES_CFG_CTXT
    //------------------------------
    dword_mem_clear((unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    desq_cfg_ctxt.des_in_own_val    = 1;
    desq_cfg_ctxt.mbox_int_en       = 0;
    desq_cfg_ctxt.des_sync_needed   = 0;
    desq_cfg_ctxt.des_num           = TOTAL_QOS_DES_NUM / atm_txq_num();

    desq_cfg_ctxt.des_base_addr     = __US_QOSQ_DES_LIST_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__QOSQ_PSEUDO_DES_CFG_BASE, base);
    dword_mem_write(dst_addr, (unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));

    for (i = 0; i < atm_txq_num(); i++) {
        desq_cfg_ctxt.des_base_addr     += desq_cfg_ctxt.des_num * 2;
        dst_addr                        += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
        dword_mem_write(dst_addr, (unsigned int *)(&desq_cfg_ctxt), sizeof(desq_cfg_ctxt));
    }

    return;
}

void vrx218_atm_des_sync_init(unsigned long base)
{
    unsigned int *dst_addr;
    des_sync_cfg_ctxt_t des_sync_cfg_ctxt;

    dword_mem_clear((unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));

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

    //Downstream AAL5Q Sync Config/Context
    des_sync_cfg_ctxt.sync_type          = DS_WRITE_READ_SYNC;
    //des_sync_cfg_ctxt.us_des_polling_needed = 0;
    des_sync_cfg_ctxt.max_polling_intv   = 1;
    des_sync_cfg_ctxt.desq_cfg_ctxt      = __DS_PKT_DESQ_CFG_CTXT;
    //NOTE: Only for CDMA add 0x20000000
    des_sync_cfg_ctxt.ext_desc_base_addr = 0x20000000 + SOC_DS_DES_BASE;
	des_sync_cfg_ctxt.soc_des_own_val = 0;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + (2*32)), base);
    dword_mem_write(dst_addr, (unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));

    //Downstream OAM Sync Config/Context
    des_sync_cfg_ctxt.sync_type          = DS_WRITE_READ_SYNC;
    //des_sync_cfg_ctxt.us_des_polling_needed = 0;
    des_sync_cfg_ctxt.max_polling_intv   = 1;
    des_sync_cfg_ctxt.desq_cfg_ctxt      = __DS_OAM_DESQ_CFG_CTXT;
    //NOTE: Only for CDMA add 0x20000000
    des_sync_cfg_ctxt.ext_desc_base_addr = 0x20000000 + SOC_DS_OAM_DES_BASE;

    dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DES_SYNC_CFG_CTXT + (3*32)), base);
    dword_mem_write(dst_addr, (unsigned int *)(&des_sync_cfg_ctxt), sizeof(des_sync_cfg_ctxt));

    return;
}

//Initialize VRX218 ATM-TC Local Descriptor List
//  Performs below actions
//      (1) Allocate and Initialize TC UpStream Local Descriptors
//      (2) Allocate and Initialize TC DownStream Local Descriptors
void vrx218_atm_local_des_init(unsigned long base)
{
    unsigned int i, *dst_addr;
    tx_descriptor_t tx_descriptor;
    rx_descriptor_t rx_descriptor;

//#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)  
    unsigned int all5_sb_addr[ATM_SB_DS_BUF_LEN] = 
                { __DS_SB_PKT_DATA_PTR_0,  __DS_SB_PKT_DATA_PTR_1, __DS_SB_PKT_DATA_PTR_2, __DS_SB_PKT_DATA_PTR_3,
                  __DS_SB_PKT_DATA_PTR_4,  __DS_SB_PKT_DATA_PTR_5, __DS_SB_PKT_DATA_PTR_6, __DS_SB_PKT_DATA_PTR_7,
                  __DS_SB_PKT_DATA_PTR_8,  __DS_SB_PKT_DATA_PTR_9, __DS_SB_PKT_DATA_PTR_10, __DS_SB_PKT_DATA_PTR_11};
//#else
//    unsigned int all5_sb_addr[ATM_SB_DS_BUF_LEN] = { 0x2100, 0x2C00, 0x3000, 0x3190, 0x3D80, 0x44E0, 0x4670, 0x4800, 0x4990, 0x4D40, 0x5B00, 0x5C90, 0x5E20 };
//#endif
    unsigned int oam_sb_addr[10] = { 0x3BC0, 0x3BE0, 0x3C00, 0x3C20, 0x3C40, 0x3C60, 0x3C80, 0x3CA0, 0x3CC0, 0x3CE0 };

    dword_mem_clear(&tx_descriptor, sizeof(tx_descriptor));
    tx_descriptor.own = 0;
    tx_descriptor.data_len = PDBRAM_PKT_SIZE - 32;

    //Initialize UpStream Descriptors
    for (i=0; i<ATM_PDBRAM_US_BUF_LEN; i++) {
        tx_descriptor.data_ptr = ((PDBRAM_TX_PKT_BUFFER_BASE + (i * PDBRAM_PKT_SIZE))/4);
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_TC_LOCAL_Q_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &tx_descriptor, sizeof(tx_descriptor));
    }

    dword_mem_clear(&rx_descriptor, sizeof(rx_descriptor));
    rx_descriptor.own = 1;
    rx_descriptor.data_len = PDBRAM_PKT_SIZE - 32;

    //Initialize DownStream AAL5 Descriptors
    for (i=0; i<ATM_PDBRAM_DS_BUF_LEN; i++) {
        rx_descriptor.data_ptr = (PDBRAM_TX_PKT_BUFFER_BASE + (ATM_PDBRAM_US_BUF_LEN * PDBRAM_PKT_SIZE) + (i * PDBRAM_PKT_SIZE));
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_TC_LOCAL_AAL5Q_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
    }
    for (i=0; i<NUM_ENTITY(all5_sb_addr); i++) {
        rx_descriptor.data_ptr = CPHYSADDR(VRX_SB_MEMORY(all5_sb_addr[i] - 0x2000));
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_TC_LOCAL_AAL5Q_DES_LIST_BASE + (ATM_PDBRAM_DS_BUF_LEN * 2) + (i * 2)), base);
        dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
    }

    dword_mem_clear(&rx_descriptor, sizeof(rx_descriptor));
    rx_descriptor.own = 1;
    rx_descriptor.data_len = 128;

    //Initialize DownStream OAM Descriptors
    for (i=0; i<10; i++) {
        //rx_descriptor.data_ptr = (VRX_SB_MEMORY(oam_sb_addr[i] - 0x2000)) >> 2; //  OAM take DWORD address
        rx_descriptor.data_ptr = CPHYSADDR(VRX_SB_MEMORY(oam_sb_addr[i] - 0x2000));
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__DS_TC_LOCAL_OAMQ_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(dst_addr, &rx_descriptor, sizeof(rx_descriptor));
    }

    return;
}

void vrx218_atm_edma_copy_ch_init(unsigned long base)
{
    edma_copy_ch_cfg_t copy_ch_cfg;
    desq_cfg_ctxt_t local_desq_cfg_ctxt;
    unsigned int *dst_addr, i;
    unsigned int des_cnt;

    //Setup 16 UpStream eDMA Copy Channel
    copy_ch_cfg.srcq_ctxt_ptr = __QOSQ_PSEUDO_DES_CFG_BASE;
    copy_ch_cfg.dstq_ctxt_ptr = __US_TC_LOCAL_Q_CFG_CTXT_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__US_EDMA_COPY_CH_CFG, base);
    dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));

    for (i = 0; i < atm_txq_num(); i ++ ) {
        copy_ch_cfg.srcq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
        copy_ch_cfg.dstq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
        dst_addr                    += sizeof(edma_copy_ch_cfg_t)/sizeof(unsigned int);
        dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
    }

    //Setup the Local DESQ Configuration/Context for 16 UpStream Queues
    dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
    local_desq_cfg_ctxt.des_in_own_val = 1;
    des_cnt = 0;
    for (i = 0; i < atm_txq_num(); i++) {

        local_desq_cfg_ctxt.des_num = 16 / atm_txq_num();

        local_desq_cfg_ctxt.des_base_addr = __US_TC_LOCAL_Q_DES_LIST_BASE + (des_cnt * 2);
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_TC_LOCAL_Q_CFG_CTXT_BASE + (i * (sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int)))), base);
        dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));
        des_cnt = des_cnt + local_desq_cfg_ctxt.des_num;
    }

    //Setup 2 DownStream eDMA Copy Channel
    copy_ch_cfg.srcq_ctxt_ptr = __DS_TC_AAL5_LOCAL_Q_CFG_CTXT;
    copy_ch_cfg.dstq_ctxt_ptr = __DS_PKT_DESQ_CFG_CTXT;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_EDMA_COPY_CH_CFG, base);
    dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));

    for (i = 0; i < 1; i ++ ) {
        copy_ch_cfg.srcq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
        copy_ch_cfg.dstq_ctxt_ptr   += sizeof(desq_cfg_ctxt_t)/sizeof(unsigned int);
        dst_addr                    += sizeof(edma_copy_ch_cfg_t)/sizeof(unsigned int);
        dword_mem_write(dst_addr, (unsigned int *)(&copy_ch_cfg), sizeof(copy_ch_cfg));
    }

    //Setup the Local DESQ Configuration/Context for 2 DownStream Queues

    //Setup the Local DESQ Configuration/Context for AAL5 DownStream Queue
    dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
    local_desq_cfg_ctxt.des_in_own_val = 0;
    local_desq_cfg_ctxt.des_num = ATM_SB_DS_BUF_LEN + ATM_PDBRAM_DS_BUF_LEN;
    local_desq_cfg_ctxt.des_base_addr = __DS_TC_LOCAL_AAL5Q_DES_LIST_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_TC_AAL5_LOCAL_Q_CFG_CTXT, base);
    dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    //Setup the Local DESQ Configuration/Context for OAM DownStream Queue
    dword_mem_clear((unsigned int *)(&local_desq_cfg_ctxt), sizeof(local_desq_cfg_ctxt));
    local_desq_cfg_ctxt.des_in_own_val = 0;
    local_desq_cfg_ctxt.des_num = 10;
    local_desq_cfg_ctxt.des_base_addr = __DS_TC_LOCAL_OAMQ_DES_LIST_BASE;
    dst_addr = (void *)SOC_ACCESS_VRX218_SB(__DS_TC_OAM_LOCAL_Q_CFG_CTXT, base);
    dword_mem_write(dst_addr, (unsigned int *)(&local_desq_cfg_ctxt), sizeof(desq_cfg_ctxt_t));

    return;
}

/*****************************************************************
 *  ATM TC - Initialization : Entry Point
 *  dev     - PCIe Endpoint Device
 *  qsb_en  - 0: QSB Disabled, 1: QSB Enabled
*****************************************************************/
void vrx218_ppe_atm_init(const ifx_pcie_ep_dev_t *dev, int qsb_en)
{
    unsigned long base = dev->phy_membase;
    volatile unsigned int *ppe_tc_sync = xTM_MODE_TO_DSL(base);

    vrx218_sb_clear(base);

    vrx218_atm_pdma_init(base);

    vrx218_atm_fw_init(base);
    vrx218_atm_htu_init(base);
    vrx218_atm_wtx_queue_cfg_init(base, qsb_en);
    vrx218_atm_wtx_port_cfg_init(base, qsb_en);
    vrx218_atm_wrx_queue_cfg_init(base);

    //Initialize Upstream Descriptors
    //A1+ ==> Single Line; no Bonding
    //vrx218_us_inq_des_init(base, 0, 0);

    //Initialize Downstream Descriptors
    //A1+ ==> Single Line; no Bonding
    vrx218_ds_soc_des_init(base, 0, 0);

    //Setup Downstream Packet/AAL5 DESQ_CFG_CTXT
    //A1+ ==> Single Line; no Bonding
    //tc_mode = 1
    vrx218_ds_desq_cfg_ctxt_init(base, 0, 0, 1);

    //Setup Downstream OAM DESQ_CFG_CTXT
    vrx218_ds_oam_desq_cfg_ctxt_init(base);

    vrx218_atm_us_qos_des_init(base);
    vrx218_atm_us_qos_cfg_init(base);

    vrx218_atm_des_sync_init(base);
    vrx218_atm_local_des_init(base);
    vrx218_atm_edma_copy_ch_init(base);

    //eDMA LLE in PDBRAM
    vrx218_edma_init(base, 0, 0);

#ifdef CONFIG_VRX218_DSL_DFE_LOOPBACK
    //NOTE: Always pretend LINK is UP!!
    //  Needed in DSL-Loopback Mode!!
    //  Will be updated by DSL FW in real-scenario!!
    *SOC_ACCESS_VRX218_SB(0x7DC0, base) = 0x00000007;
    *SOC_ACCESS_VRX218_SB(0x7DD0, base) = 0x00000007;
#endif

    //Init CDMA for ATM TC-Mode
    vrx218_cdma_init(base, 0, 0, 0, 1);

    //Indicate to DSL, we are in ATM mode
    *ppe_tc_sync = ((*ppe_tc_sync) & ~0x03) | 0x1;

    return;
}
#endif

unsigned int in_sync(unsigned long base, u8 bc)
{
    unsigned int state = 0;

    if (bc == 0)
        state = *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_STATE0, base) & 0x01;
    else if (bc == 1)
        state = *SOC_ACCESS_VRX218_ADDR(VRX_PPE_SFSM_STATE1, base) & 0x01;

    return state;
}

void __exit vrx218_ppe_atm_fw_exit(const ifx_pcie_ep_dev_t *dev)
{
    unsigned int base = dev->phy_membase;
    volatile unsigned int *ppe_tc_sync = xTM_MODE_TO_DSL(base);

    /*clear the indication bit to DSL FW */
    *ppe_tc_sync &= ~0x3;
}

void __exit vrx218_ppe_atm_free_mem(const ifx_pcie_ep_dev_t *dev)
{
    unsigned int base = dev->phy_membase;
    unsigned int i, *dst_addr;
    tx_descriptor_t tx_descriptor;
    struct sk_buff *skb_to_free;

    //Destroy QoSQ Descriptors
    for (i=0; i<512; i++) {
        dst_addr = (void *)SOC_ACCESS_VRX218_SB((__US_QOSQ_DES_LIST_BASE + (i * 2)), base);
        dword_mem_write(&tx_descriptor, dst_addr, sizeof(tx_descriptor));
        skb_to_free = get_skb_pointer(tx_descriptor.data_ptr);
        if ( skb_to_free != NULL )
            dev_kfree_skb_any(skb_to_free);
    }

    free_dataptr_fragments();
}
