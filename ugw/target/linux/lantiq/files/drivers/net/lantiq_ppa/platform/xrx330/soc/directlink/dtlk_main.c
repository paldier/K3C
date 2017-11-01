/*
* Copyright (c) 2014 Lantiq Corporation
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
* SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
* OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
* CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/


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
#include <asm/addrspace.h>
#include <asm/io.h>
#include <linux/netdevice.h>
#include <net/ppa_ppe_hal.h>
#include <linux/list.h>

#include "../11ac_acc_data_structure_tx_be.h"
#include "./include/dlrx_drv.h"
#include <net/ppa_stack_al.h>
#include "./include/dlrx_fw_data_structure_be.h"

#include "./include/dlrx_fw_def.h"
#include "./include/dlrx_wlan_api.h"
#include "./include/dlrx_dre_api.h"
#include "./include/dlrx_memory_lyt.h"


#include "./include/dlrx_fw_version.h"
#include "../directlink_tx_cfg.h"
#include "../ltqmips_hal.h"


/*****************************************************
 *   Macro definition
 *****************************************************/
#define MOD_AUTHOR          "Zhu YiXin, Manickavel Karthikeyan"
#define MOD_DESCRIPTION     "Accelerate QCA 11AC RX Traffic"
#define MOD_LICENCE         "GPL"
#define DLRX_DRV_MAJOR      0
#define DLRX_DRV_MID        0
#define DLRX_DRV_MINOR      20
#define DLRX_DRV_VERSION    ((DLRX_DRV_MAJOR << 24) | (DLRX_DRV_MID << 16) | DLRX_DRV_MINOR)

#define INLINE                                inline
#define DIRECTLINK_PROC_DIR "dl"

#define SWITCH_BURST_LOW_THRESHOLD_ADDR 0x1E10B098
#define SWITCH_BURST_HIGH_THRESHOLD_ADDR 0x1E10B09C


/*****************************************************
 *  Global Parameter
 *****************************************************/
uint32_t g_dma_des_len          = GSWIP_DESC_NUM;
uint32_t g_ce5_desc_ring_num    = CE5_DEST_DESC_RING_NUM;
uint32_t g_ce5_buf_ptr_ring_num = RX_PKT_BUF_PTR_RING_NUM;
uint32_t *g_dma_skb_buf         = NULL;
uint32_t g_dtlk_memsize         = DTLK_TX_RSV_MEM_SIZE + DTLK_RX_RSV_MEM_SIZE;

/**************************************/

/* !!! These three base address must be initialized before access any FW structure !!! */
unsigned int *ddr_base       = 0;
unsigned int *cfg_ctxt_base  = 0;
unsigned int *pcie_base      = 0;

extern dre_regfn_set_t g_dre_fnset;
extern uint32_t g_dlrx_max_inv_header_len;
extern uint32_t g_dlrx_cfg_offset_atten;

#ifdef SUPPORT_UNLOAD_DTLK

extern 	void ppa_dp_dlrx_init(void);
extern 	void ppa_dp_dlrx_exit(void);
#endif

#ifdef SUPPORT_11AC_MULTICARD

/*API to get 11AC wireless card type */
extern int ppa_dl_detect_11ac_card();
#endif

/*****************************************************
 *  Internal Variables
 *****************************************************/
static struct proc_dir_entry *g_dtlk_proc_dir = NULL;

struct dtlk_dgb_info
{
    char *cmd;
    char *description;
    uint32_t flag;
};

static struct dtlk_dgb_info dtlk_dbg_enable_mask_str[] = {
    {"err",      "error print",                     DTLK_DBG_ENABLE_MASK_ERR },
    {"assert",    "assert",                         DTLK_DBG_ENABLE_MASK_ASSERT},
	{"dbg", 	  "debug print",					DTLK_DBG_ENABLE_MASK_DEBUG_PRINT},
    /*the last one */
    {"all",       "enable all debug",                -1}
};
uint32_t g_dtlk_dbg_enable = DTLK_DBG_ENABLE_MASK_ERR;


/*****************************************************
 *  Internal functions
 *****************************************************/
static void dma_txch_init(uint32_t ppe_txch_no, uint32_t dma_desc_base, uint32_t dma_desc_len)
{
    uint32_t dma_ch_no;

    dtlk_print("ppe tx ch no: %d, desc base: 0x%x, desc len: %d\n",
        ppe_txch_no, dma_desc_base, dma_desc_len);
        
    dma_ch_no = ppe_txch_no*2 + 1 ;
    *DMA_CS     = dma_ch_no;
    *DMA_CIE    = 0;
    *DMA_CIS    = 0x3F;
    *DMA_CDBA   = CPHYSADDR(dma_desc_base);
    *DMA_CDLEN  = dma_desc_len;
    *DMA_CCTRL  = 0x30100;
    *DMA_CCTRL  = *DMA_CCTRL | 1;  //Enable the tx channel

    dtlk_print("DMA CS:0x%x, DMA_CIE:0x%x, DMA_CIS:0x%x, DMA_CDBA:0x%x, DMA_CDLEN:0x%x, DMA_CCTRL:0x%x\n",
        *DMA_CS,*DMA_CIE, *DMA_CIS, *DMA_CDBA, *DMA_CDLEN, *DMA_CCTRL);
        
    return;
}

static void dlrx_init_buf_size(dlrx_bufsize_t *dlrx_bufnum)
{
	//Initializing the Length of data structure
    dlrx_bufnum->gswip_desc_num           = g_dma_des_len;
    dlrx_bufnum->wlan_desc_num            = WLAN_DESC_NUM;
    dlrx_bufnum->proto_desc_num           = PROTO_DESC_NUM;
    dlrx_bufnum->cpu_ce5_desc_ring_num    = CPU_CE5_DESC_RING_NUM;
    dlrx_bufnum->rx_pkt_buf_rel_msg_num   = RX_PKT_BUF_REL_MSG_NUM;
    dlrx_bufnum->ce5_dest_desc_ring_num   = g_ce5_desc_ring_num;
    dlrx_bufnum->ce5_dest_msg_buf_num     = g_ce5_desc_ring_num;
    dlrx_bufnum->rx_pkt_buf_ptr_ring_num  = g_ce5_buf_ptr_ring_num;
    dlrx_bufnum->rx_reorder_main_num      = RX_REORDER_MAIN_NUM;
    dlrx_bufnum->rx_reorder_desc_link_num = RX_REORDER_DESC_LINK_NUM;

    return;
}

extern unsigned int skb_list_get_skb(unsigned int rxpb_ptr);
void dlrx_data_structure_free(void)
{
    dlrx_rxpb_ptr_ring_t *dlrx_rxpb_ring_ptr,*dlrx_rxpb_ring_ptr_org;
    unsigned int index,j;
	unsigned int currentV;
    dlrx_rxpb_ring_ptr=(dlrx_rxpb_ptr_ring_t *)DLRX_DDR_RX_PKT_BUF_RING_BASE;
	dlrx_rxpb_ring_ptr_org=(dlrx_rxpb_ptr_ring_t *)DLRX_DDR_RX_PKT_BUF_RING_BASE;
    dlrx_bufsize_t dlrx_bufnum;
    dlrx_init_buf_size(&dlrx_bufnum);

	//looking for duplicate
	for(index = 0; index < dlrx_bufnum.rx_pkt_buf_ptr_ring_num -1; index ++) //initial 1024 -1
    {
        currentV = (dlrx_rxpb_ring_ptr_org+index)->rxpb_ptr;
    	j = index+1;
		for( j ; j < dlrx_bufnum.rx_pkt_buf_ptr_ring_num -1;j++)
		{
			if(currentV != 0 && currentV == (dlrx_rxpb_ring_ptr_org+j)->rxpb_ptr)
			{
			    //printk("Duplicate: %x index [%d]\n",currentV,j);
				(dlrx_rxpb_ring_ptr_org+j)->rxpb_ptr = 0;
			}
		}
	}
}

int dlrx_data_structure_init(void)
{
    unsigned int index;
    struct sk_buff *new_skb;
    unsigned int seqid;
    unsigned int temp_ce5_buf_size;
    unsigned int shift_size = 0;
    dlrx_bufsize_t dlrx_bufnum;
    /**************************************/
    //Pointer to various data structures with respect to config context
    //dlrx_cfg_ctxt_dma_t *dlrx_cfg_ctxt_dma_des_wlan_ptr;
    dlrx_cfg_ctxt_dma_t *dlrx_cfg_ctxt_dma_des_gswip_ptr;
    dlrx_cfg_ctxt_dma_t *dlrx_cfg_ctxt_dma_des_prot_ptr;
    soc_dma_desc_t *dma_desc;  

    dlrx_cfg_ctxt_cpu_ce5des_t *dlrx_cfg_ctxt_cpu_ce5des_ptr;
    dlrx_ce5des_format_t *dlrx_ce5des_format_ptr;

    dlrx_cfg_ctxt_ce5des_t *dlrx_cfg_ctxt_ce5des_ptr;
    dlrx_cfg_ctxt_ce5buf_t *dlrx_cfg_ctxt_ce5buf_ptr;

    dlrx_cfg_ctxt_rxpb_ptr_ring_t *dlrx_cfg_ctxt_rxpb_ptr_ring_ptr;

    dlrx_cfg_ctxt_ro_mainlist_t *dlrx_cfg_ctxt_ro_mainlist_ptr;
    dlrx_cfg_ctxt_ro_linklist_t *dlrx_cfg_ctxt_ro_linklist_ptr;

    dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_t *dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_ptr;
    dlrx_cfg_global_t *dlrx_cfg_global_ptr;

    //This is the reorder main list in DDR memory (The starting of this  base address is stored in "dlrx_cfg_ctxt_ro_mainlist_t" structure).
    dlrx_ro_mainlist_t *dlrx_ro_mainlist_ptr;
    dlrx_ro_linklist_t *dlrx_ro_linklist_ptr;

    dlrx_rxpb_ptr_ring_t *dlrx_rxpb_ring_ptr;
    dlrx_cfg_vap2int_map1_t *dlrx_cfg_vap2int_map1_ptr;
    dlrx_cfg_vap2int_map2_t *dlrx_cfg_vap2int_map2_ptr;
    dlrx_cfg_ctxt_rxpb_t *dlrx_cfg_ctxt_rxpb_ptr;

    uint32_t *msg_buf_base;

    //The below base addresses will be used by FW
     dtlk_mem_base_get(NULL, (uint32_t *)&ddr_base);
    cfg_ctxt_base = (unsigned int *)KSEG0ADDR(DLRX_SRAM_PHY_ADDR);
    ddr_base = (unsigned int *)KSEG0ADDR(ddr_base);

    // Set the memory range to 0
    #if 0 
    memset(ddr_base, 0, DTLK_RX_RSV_MEM_SIZE);  // 4MB
    memset(cfg_ctxt_base, 0, DLRX_CFG_CTXT_MAX_SIZE); // 4KB
	#else
	memset((unsigned int *)KSEG1ADDR(ddr_base), 0, DTLK_RX_RSV_MEM_SIZE);	// 4MB
	memset((unsigned int *)KSEG1ADDR(DLRX_SRAM_PHY_ADDR), 0, DLRX_CFG_CTXT_MAX_SIZE); // 4KB
	#endif

#ifdef SUPPORT_11AC_MULTICARD
    if (ppa_dl_detect_11ac_card() ==  PEREGRINE_BOARD) {
        g_dlrx_max_inv_header_len = MAX_INV_PEREGRINE_HEADER_LEN;
		g_dlrx_cfg_offset_atten = QCA_PEREGRINE_11AC_CFG_OFFSET_ATTEN;
    } else {
        g_dlrx_max_inv_header_len = MAX_INV_BEELINER_HEADER_LEN;
		g_dlrx_cfg_offset_atten = QCA_BEELINER_11AC_CFG_OFFSET_ATTEN;
    }
#else
    g_dlrx_max_inv_header_len = MAX_INV_PEREGRINE_HEADER_LEN;
	g_dlrx_cfg_offset_atten = QCA_PEREGRINE_11AC_CFG_OFFSET_ATTEN;
#endif

    //Initializing the Length of data structure
    dlrx_init_buf_size(&dlrx_bufnum);

    // NOT used in current implementation( Wlan, protocol stack and cpu ring). For future use
    //dlrx_cfg_ctxt_dma_des_wlan_ptr  = (dlrx_cfg_ctxt_dma_t *)DLRX_CFG_CTXT_WLAN_DMA_BASE;
    //Initializing WLAN Ptr ==> 255 * (1 * 4) DWORDS 
    //dlrx_cfg_ctxt_dma_des_wlan_ptr->cfg_badr_dma = (unsigned int)DLRX_DDR_WLAN_DMA_DESC_BASE;
    //dlrx_cfg_ctxt_dma_des_wlan_ptr->cfg_num_dma  =  dlrx_bufnum.wlan_desc_num;
    //dlrx_cfg_ctxt_dma_des_wlan_ptr->txdes_index  = 0;

    dlrx_cfg_ctxt_dma_des_gswip_ptr = (dlrx_cfg_ctxt_dma_t *)DLRX_CFG_CTXT_GSWIP_DMA_BASE;
    //Initializing GSWIP Ptr ==> 255 * (1 * 4) DWORDS
    dlrx_cfg_ctxt_dma_des_gswip_ptr->cfg_badr_dma = (unsigned int)DLRX_DDR_GSWIP_DMA_DESC_BASE;
    dlrx_cfg_ctxt_dma_des_gswip_ptr->cfg_num_dma  = dlrx_bufnum.gswip_desc_num;
    dlrx_cfg_ctxt_dma_des_gswip_ptr->txdes_index  = 0;
    //SETUP THE OWNBIT OF THE DMA
    for(index = 0, dma_desc = (soc_dma_desc_t *)dlrx_cfg_ctxt_dma_des_gswip_ptr->cfg_badr_dma; 
        index < dlrx_bufnum.gswip_desc_num; 
        index ++, dma_desc ++){
        dma_desc->own = DMA_CPU_OWNBIT;
    }
    dma_txch_init(PPE_TX_CH_NO,dlrx_cfg_ctxt_dma_des_gswip_ptr->cfg_badr_dma, dlrx_cfg_ctxt_dma_des_gswip_ptr->cfg_num_dma);
    g_dma_des_base = (uint32_t *)dlrx_cfg_ctxt_dma_des_gswip_ptr->cfg_badr_dma; 

    dlrx_cfg_ctxt_dma_des_prot_ptr  = (dlrx_cfg_ctxt_dma_t *)DLRX_CFG_CTXT_PROT_DMA_BASE;
	//Initializing Protocol Stack Ptr ==> 255 * (1 * 4) DWORDS
	dlrx_cfg_ctxt_dma_des_prot_ptr->cfg_badr_dma = (unsigned int)DLRX_DDR_PROTO_DMA_DESC_BASE;
	dlrx_cfg_ctxt_dma_des_prot_ptr->cfg_num_dma  = dlrx_bufnum.proto_desc_num;
	dlrx_cfg_ctxt_dma_des_prot_ptr->txdes_index  = 0;


	dlrx_cfg_ctxt_cpu_ce5des_ptr = (dlrx_cfg_ctxt_cpu_ce5des_t *)DLRX_CFG_CTXT_CPU_CE5DES_BASE;
	//Initializing CPU CE5 Destination Ring Pointers ==> 2 * (1 * 8) DWORDS ( this CPU_CE5DES is in SB )
	dlrx_cfg_ctxt_cpu_ce5des_ptr->cfg_badr_cpu_ce5    = (unsigned int)DLRX_DDR_CPU_CE5_DESC_BASE;
	dlrx_cfg_ctxt_cpu_ce5des_ptr->cfg_num_cpu_ce5     = dlrx_bufnum.cpu_ce5_desc_ring_num;
	dlrx_cfg_ctxt_cpu_ce5des_ptr->cpu_ce5_read_index  = 0;
	dlrx_cfg_ctxt_cpu_ce5des_ptr->cpu_ce5_write_index = 1;
	dlrx_cfg_ctxt_cpu_ce5des_ptr->cpu_ce5_msg_done    = 1;
	
	dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_ptr = (dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_t *)DLRX_CFG_CTXT_RXPB_PTR_REL_MSGBUF_BASE;
	//Initializing RX PB Release Message Buffer ==> 2 * (2 * 128) DWORDS
	dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_ptr->cfg_badr_rel_msgbuf = (unsigned int)DLRX_DDR_RX_PKT_BUF_REL_MSG_BASE;
	dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_ptr->cfg_num_rel_msgbuf  = dlrx_bufnum.rx_pkt_buf_rel_msg_num;

    dlrx_cfg_ctxt_rxpb_ptr = (dlrx_cfg_ctxt_rxpb_t *)DLRX_CFG_CTXT_RXPB_BASE;
    dlrx_cfg_ctxt_rxpb_ptr->cfg_offset_atten  = 4;
    dlrx_cfg_ctxt_rxpb_ptr->cfg_size_rxpktdes = g_dlrx_cfg_offset_atten;

	dlrx_cfg_ctxt_rxpb_ptr_ring_ptr = (dlrx_cfg_ctxt_rxpb_ptr_ring_t *)DLRX_CFG_CTXT_RXPB_PTR_RING_BASE;
	//Initializing RX PB Ring Ptr ===> 256 * (1 * 8) DWORDS 
	dlrx_cfg_ctxt_rxpb_ptr_ring_ptr->cfg_badr_rxpb_ptr_ring = (unsigned int)KSEG1ADDR(DLRX_DDR_RX_PKT_BUF_RING_BASE);
	dlrx_cfg_ctxt_rxpb_ptr_ring_ptr->cfg_num_rxpb_ptr_ring  = dlrx_bufnum.rx_pkt_buf_ptr_ring_num;
    dlrx_cfg_ctxt_rxpb_ptr_ring_ptr->rxpb_ptr_read_index    = 0;
    dlrx_cfg_ctxt_rxpb_ptr_ring_ptr->rxpb_ptr_write_index   = dlrx_bufnum.rx_pkt_buf_ptr_ring_num - 1;
	dlrx_rxpb_ring_ptr=(dlrx_rxpb_ptr_ring_t *)DLRX_DDR_RX_PKT_BUF_RING_BASE;
    for(index = 0; index < dlrx_bufnum.rx_pkt_buf_ptr_ring_num -1; index ++) //initial 1024 -1
    {
        //Initializing RX PB PTR ==> 4096 * (1 * 4) DWORDS  
        new_skb = alloc_skb_rx();
        if( new_skb==NULL )
        {
            return DTLK_FAILURE;
        }
        else
        {
            
            //list_add((struct list_head *)new_skb, &g_rxbp_skblist);
            dma_cache_inv((u32)new_skb->data, new_skb->end - new_skb->data);
            dlrx_rxpb_ring_ptr->rxpb_ptr = CPHYSADDR((uint32_t)new_skb->data);
            //printk("%s: %x\n",__func__,new_skb);
            dlrx_rxpb_ring_ptr ++;
        }
    }
    
    // This is message ring initialization
	dlrx_cfg_ctxt_ce5buf_ptr = (dlrx_cfg_ctxt_ce5buf_t *)DLRX_CFG_CTXT_CE5BUF_BASE;
	//Initializing CE5 Buffers and Buffer format pointers  ====> 4096 * 512 Bytes
	dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_ce5buf = (unsigned int)DLRX_DDR_CE5BUF_BASE;
	dlrx_cfg_ctxt_ce5buf_ptr->cfg_num_ce5buf  = dlrx_bufnum.ce5_dest_msg_buf_num;
	dlrx_cfg_ctxt_ce5buf_ptr->cfg_size_ce5buf = DLRX_CE5_DEST_BUF_SIZE;//  * dlrx_bufnum.ce5_dest_msg_buf_num;
    //*DLRX_TARGET_CE5_READ_INDEX  = 0;
    //*DLRX_TARGET_CE5_WRITE_INDEX = dlrx_bufnum.ce5_dest_desc_ring_num - 1;
	//dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_read_index  = (unsigned int)DLRX_TARGET_CE5_READ_INDEX; 
	//dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_write_index = (unsigned int)DLRX_TARGET_CE5_WRITE_INDEX;
    temp_ce5_buf_size = DLRX_CE5_DEST_BUF_SIZE;
    while( temp_ce5_buf_size != 1 )
    {
        shift_size++;
        temp_ce5_buf_size >>= 1;
    }
    dlrx_cfg_ctxt_ce5buf_ptr->cfg_size_shift_ce5buf = shift_size;

    //initial msg buf type to 0xFF
    msg_buf_base = (uint32_t *)DLRX_DDR_CE5BUF_BASE;
    for(index = 0; index < dlrx_bufnum.ce5_dest_msg_buf_num; index ++){
        *(msg_buf_base + index * 512 / 4 + 2) = 0xFF;
    }
    
    //Initializing Actual CE5 Descriptors format ====> 4096 * 2 DWORDS
    // This is ce5 descriptor initialization
	dlrx_ce5des_format_ptr = (dlrx_ce5des_format_t *)DLRX_DDR_CE5DESC_BASE;
	for(index=0; index < dlrx_bufnum.ce5_dest_msg_buf_num; index++)
	{
		dlrx_ce5des_format_ptr->dest_ptr  = 
             CPHYSADDR(dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_ce5buf + (index << dlrx_cfg_ctxt_ce5buf_ptr->cfg_size_shift_ce5buf));
		dlrx_ce5des_format_ptr->meta_data = 0;
		//dlrx_ce5des_format_ptr->nbytes    = ( 1 <<  dlrx_cfg_ctxt_ce5buf_ptr->cfg_size_shift_ce5buf );
		dlrx_ce5des_format_ptr->nbytes  = 0;
		dlrx_ce5des_format_ptr ++;
	}

    dlrx_cfg_ctxt_ce5des_ptr = (dlrx_cfg_ctxt_ce5des_t *)DLRX_CFG_CTXT_CE5DES_BASE;
	//Initializing General Configuration of CE5 Destination Descriptors ===>  1 * 4 DWORDS (only used by driver)
	dlrx_cfg_ctxt_ce5des_ptr->cfg_badr_ce5des = (unsigned int)KSEG1ADDR(DLRX_DDR_CE5DESC_BASE);
	dlrx_cfg_ctxt_ce5des_ptr->cfg_num_ce5des  = dlrx_bufnum.ce5_dest_desc_ring_num;

	dlrx_cfg_ctxt_ro_mainlist_ptr = (dlrx_cfg_ctxt_ro_mainlist_t *)DLRX_CFG_CTXT_RO_MAINLIST_BASE;
	//Initializing RX REORDER Mainlist Ptr ===> 128 * 16 = 2048 domains ==> Each domain == 64 + 4 DWORDS
	dlrx_cfg_ctxt_ro_mainlist_ptr->cfg_badr_ro_mainlist = (unsigned int)DLRX_DDR_RO_MAINLIST_BASE;
	dlrx_cfg_ctxt_ro_mainlist_ptr->cfg_num_ro_mainlist  = dlrx_bufnum.rx_reorder_main_num;
	dlrx_cfg_ctxt_ro_mainlist_ptr->ro_mainlist_ptr      = NULL_PTR;
    
	dlrx_ro_mainlist_ptr=(dlrx_ro_mainlist_t *)DLRX_DDR_RO_MAINLIST_BASE;
	for(index = 0; index < dlrx_bufnum.rx_reorder_main_num; index++)
	{
		dlrx_ro_mainlist_ptr->first_ptr = NULL_PTR;
		for(seqid = 1; seqid < 64; seqid ++)
			dlrx_ro_mainlist_ptr->_dw_res0[seqid-1] = NULL_PTR;
		dlrx_ro_mainlist_ptr ++;
	}


	dlrx_cfg_ctxt_ro_linklist_ptr = (dlrx_cfg_ctxt_ro_linklist_t *) DLRX_CFG_CTXT_RO_LINKLIST_BASE;
	//Initializing RX REORDER Linklist Ptr  ===> 4095 Desc ==> Each Desc == 6 DWORDS
	dlrx_cfg_ctxt_ro_linklist_ptr->cfg_badr_ro_linklist   = (unsigned int)DLRX_DDR_RO_LINKLIST_BASE;
	dlrx_cfg_ctxt_ro_linklist_ptr->cfg_num_ro_linklist    = dlrx_bufnum.rx_reorder_desc_link_num;
	dlrx_cfg_ctxt_ro_linklist_ptr->free_num_ro_linklist   = dlrx_cfg_ctxt_ro_linklist_ptr->cfg_num_ro_linklist - 1;
	dlrx_cfg_ctxt_ro_linklist_ptr->ro_des_free_head_index = 0;
	dlrx_cfg_ctxt_ro_linklist_ptr->ro_des_free_tail_index = dlrx_cfg_ctxt_ro_linklist_ptr->cfg_num_ro_linklist - 1;

    dlrx_ro_linklist_ptr = (dlrx_ro_linklist_t *)DLRX_DDR_RO_LINKLIST_BASE;
	for(index = 0; index < dlrx_bufnum.rx_reorder_desc_link_num; index ++)
	{
		dlrx_ro_linklist_ptr->next_ptr = index + 1;
		dlrx_ro_linklist_ptr ++;
	}


	dlrx_cfg_global_ptr = (dlrx_cfg_global_t *)DLRX_CFG_GLOBAL_BASE;
	//Initializing GLOBAL CONFIGURATION STRUCTURE
	dlrx_cfg_global_ptr->dltx_enable            = 0; 
    dlrx_cfg_global_ptr->dlrx_enable            = 0;
	dlrx_cfg_global_ptr->dlrx_pcie_base         = (unsigned int)pcie_base;//This Driver will provide later, not now
	dlrx_cfg_global_ptr->dlrx_ddr_base          = (unsigned int)ddr_base;
	dlrx_cfg_global_ptr->dlrx_cfg_ctxt_base     = (unsigned int)cfg_ctxt_base;
	dlrx_cfg_global_ptr->dlrx_cfg_ctxt_max_size = DLRX_CFG_CTXT_MAX_SIZE;//Maximum size allocated in SRAM for DLRX data structure.
									//This value must need by other modules to utilize the SRAM
    dlrx_cfg_global_ptr->dlrx_cfg_unload = 0;
#ifdef SUPPORT_11AC_MULTICARD
    dlrx_cfg_global_ptr->dlrx_qca_hw = ppa_dl_detect_11ac_card();
#else
    dlrx_cfg_global_ptr->dlrx_qca_hw = 0;
#endif
    dlrx_cfg_vap2int_map1_ptr = (dlrx_cfg_vap2int_map1_t *) DLRX_CFG_VAP2INT_MAP1_BASE;
	dlrx_cfg_vap2int_map2_ptr = (dlrx_cfg_vap2int_map2_t *) DLRX_CFG_VAP2INT_MAP2_BASE;                      
    dlrx_cfg_vap2int_map1_ptr->vap0 = 0xF;
    dlrx_cfg_vap2int_map1_ptr->vap1 = 0xF;                                    
    dlrx_cfg_vap2int_map1_ptr->vap2 = 0xF;
    dlrx_cfg_vap2int_map1_ptr->vap3 = 0xF;
    dlrx_cfg_vap2int_map1_ptr->vap4 = 0xF;
    dlrx_cfg_vap2int_map1_ptr->vap5 = 0xF;
    dlrx_cfg_vap2int_map1_ptr->vap6 = 0xF;
    dlrx_cfg_vap2int_map1_ptr->vap7 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap8 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap9 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap10 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap11 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap12 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap13 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap14 = 0xF;
    dlrx_cfg_vap2int_map2_ptr->vap15 = 0xF;
    
    return DTLK_SUCCESS;	
}	


/*
 * Function: get_next_argument.
 * Description: Get the next valid argument from given string, ignore space
 * Argument: pSrc [IN]: source pointer
 * Return: pSrc[OUT]: new pointer points to the starting of valid argument
 *            len[OUT]: the len of ignorance space.
*/
static char* get_next_argument(char *pSrc,int *len)
{
	char* pTemp = pSrc;
	if ( pTemp == NULL ){
		*len = 0;
		return NULL;
	}

	while( pTemp != NULL && *pTemp == ' ' )
	{
		pTemp++;
	}

	return pTemp;
	
}
/*
 * Function: Compare strings follow by given number of length, ignore case senstive, .
 * Description: Compare two strings, ignore the case sensitive
 * Argument: p1 [IN]: source pointer 1
 *                 p2 [IN]: source pointer 2
 *                 n [IN]: length of string to compare.
 * Return:0: identical
 *           other: not match
*/
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

/*
 *  Proc struct def  
 */
/* Functions and structure support proc/dl/peer_to_vap */
static int proc_read_dtlk_peer_to_vap(struct seq_file *seq, void *v)
{
    int i = 0;
    seq_printf(seq, "\n");
    seq_printf(seq, "Peer to VAP and PN Configuration table: \n");
    seq_printf(seq, "PN type: 0-no PN check,1-48 bit PB,2-128 bit PB Even,3-128 bit PB Odd\n");
    seq_printf(seq, "\n");
    seq_printf(seq, "[ ID:Acc:PN:Vap] [ ID:Acc:PN:Vap]\n");
    for(i = 0; i < MAX_PEER_NUM ; i+=2)
    {
        uint32_t peer0=0,peer1=0;
        peer0 = *(volatile uint32_t *)DLRX_CFG_PEER_TO_VAP_PN_BASE(i);
        peer1 = *(volatile uint32_t *)DLRX_CFG_PEER_TO_VAP_PN_BASE(i+1);
        seq_printf(seq, "[%3d:%3d:%2d:%3d] [%3d:%3d:%2d:%3d]\n",i,(peer0 & 0x40),(peer0 & 0x30),(peer0 & 0xF)
            ,i+1,(peer1 & 0x40),(peer1 & 0x30),(peer1 & 0xF));
    }
    seq_printf(seq, "\n");

    return 0;
}
 /* Functions and structure support proc/dl/peer_id */
static int proc_read_dtlk_peer_id(struct seq_file *seq, void *v)
{
    int i = 0;
    seq_printf(seq, "\n");
    seq_printf(seq, "Peer ID to peer table address: 0x%x\n",DLRX_CFG_PEER_ID_TO_PEER_MAP_BASE(0));
    seq_printf(seq, "\n");
    seq_printf(seq, "Valid Peer ID table:\n");
    seq_printf(seq, "  [ ID:   Peer]\n");
    for(i = 0; i < MAX_PEERID_NUM/4 ; i++)
    {
        uint32_t regs = 0;
        uint8_t peerid0=0,peerid1=0,peerid2=0,peerid3=0;
        regs = *(volatile uint32_t *)DLRX_CFG_PEER_ID_TO_PEER_MAP_BASE(i);
        peerid0 = ((regs)         & 0xff);
        peerid1 = (( regs >>  8 ) & 0xff);
        peerid2 = (( regs >> 16 ) & 0xff);
        peerid3 = (( regs >> 32 ) & 0xff);
        if( ((peerid0 >> 7 ) & 1) )
        {
            seq_printf(seq, "  [%3d:%7d]\n",i*4,(peerid0 &0x7f));
        }
        if( ((peerid1 >> 7 ) & 1) )
        {
            seq_printf(seq, "  [%3d:%7d]\n",i*4+1,(peerid1 &0x7f));
        }
        if( ((peerid2 >> 7 ) & 1) )
        {
            seq_printf(seq, "  [%3d:%7d]\n",i*4+2,(peerid2 &0x7f));
        }
        if( ((peerid3 >> 7 ) & 1) )
        {
            seq_printf(seq, "  [%3d:%7d]\n",i*4+3,(peerid3 &0x7f));
        }
    }
    seq_printf(seq, "\n");

    return 0;
}
/* Functions and structure support proc/dl/wifi_port */
 static int proc_read_dtlk_wifi_port(struct seq_file *seq, void *v)
{
	int i = 0;
	for(i = 0; i < MAX_VAP_NUM; i++)
	{
		struct net_device* vapDev = dtlk_dev_from_vapid(i);
		if( vapDev )
		{
			int directpathID = 0;
			uint32_t vap2int;
			if( i < MAX_VAP_NUM / 2 )
			{
			    vap2int = *(volatile uint32_t *)DLRX_CFG_VAP2INT_MAP1_BASE;
			}else
			{
				vap2int = *(volatile uint32_t *)DLRX_CFG_VAP2INT_MAP2_BASE;
			}
			directpathID = (vap2int >> i*4) & 0xf;
			if( directpathID == 0xf)
				directpathID = -1;
			seq_printf(seq, "[%d]: Directpath: %d, VAP id: %d, wifi dev: %s\n",i,directpathID,i,vapDev->name);
		}else
		{
			seq_printf(seq, "[%d]: Invalid\n",i);
		}
	}
	

    return 0;
}

static int proc_read_dtlk_peer_to_vap_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dtlk_peer_to_vap, NULL);
}

static int proc_read_dtlk_peer_id_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dtlk_peer_id, NULL);
}

static int proc_read_dtlk_wifi_port_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dtlk_wifi_port, NULL);
}

static struct file_operations g_proc_file_peer_to_vap_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_peer_to_vap_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release
};

static struct file_operations g_proc_file_peer_id_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_peer_id_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release
};

static struct file_operations g_proc_file_wifi_port_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_wifi_port_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release
};
 /* Functions and structure support proc/dtlk/reg */
static int proc_read_dtlk_reg(struct seq_file *seq, void *v)
{
	
	volatile dlrx_cfg_vap2int_map1_t *vap2int_map1 = (dlrx_cfg_vap2int_map1_t *)DLRX_CFG_VAP2INT_MAP1_BASE;
	volatile dlrx_cfg_vap2int_map2_t *vap2int_map2 = (dlrx_cfg_vap2int_map2_t *)DLRX_CFG_VAP2INT_MAP2_BASE;
	seq_printf(seq, "Firmware registers information\n");
	seq_printf(seq, "			CFG_VAP2INT_MAP1: 0x%08x\n",*vap2int_map1);
	seq_printf(seq, "	VAP0: %d\n",vap2int_map1->vap0);
	seq_printf(seq, "	VAP1: %d\n",vap2int_map1->vap1);
	seq_printf(seq, "	VAP2: %d\n",vap2int_map1->vap2);
	seq_printf(seq, "	VAP3: %d\n",vap2int_map1->vap3);
	seq_printf(seq, "	VAP4: %d\n",vap2int_map1->vap4);
	seq_printf(seq, "	VAP5: %d\n",vap2int_map1->vap5);
	seq_printf(seq, "	VAP6: %d\n",vap2int_map1->vap6);
	seq_printf(seq, "	VAP7: %d\n",vap2int_map1->vap7);
	seq_printf(seq, "			CFG_VAP2INT_MAP2: 0x%08x\n",*vap2int_map2);
	seq_printf(seq, "	VAP8: %d\n",vap2int_map2->vap8);
	seq_printf(seq, "	VAP9: %d\n",vap2int_map2->vap9);
	seq_printf(seq, "	VAP10: %d\n",vap2int_map2->vap10);
	seq_printf(seq, "	VAP11: %d\n",vap2int_map2->vap11);
	seq_printf(seq, "	VAP12: %d\n",vap2int_map2->vap12);
	seq_printf(seq, "	VAP13: %d\n",vap2int_map2->vap13);
	seq_printf(seq, "	VAP14: %d\n",vap2int_map2->vap14);
	seq_printf(seq, "	VAP15: %d\n",vap2int_map2->vap15);

    return 0;
}

static int proc_read_dtlk_reg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dtlk_reg, NULL);
}



static struct file_operations g_proc_file_reg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_reg_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release
};

 /* Functions and structure support proc/dtlk/ver */
static int proc_read_dtlk_ver(struct seq_file *seq, void *v)
{
	

	seq_printf(seq, "DirectLink driver information: \n");
	seq_printf(seq, "    Version: %d.%d.%d\n",DLRX_DRV_MAJOR,DLRX_DRV_MID,DLRX_DRV_MINOR);
	seq_printf(seq, "Firmware information: \n");
	seq_printf(seq, "    Version:%08x\n",DRE_FW_VERSION);
	seq_printf(seq, "    Feature:%08x\n",DRE_FW_FEATURE);

    return 0;
}

static int proc_read_dtlk_ver_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dtlk_ver, NULL);
}



static struct file_operations g_proc_file_ver_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_ver_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release
};
/* Functions and structure support proc/dtlk/mib */
static int proc_read_dtlk_mib(struct seq_file *seq, void *v)
{
	int i = 0;
	uint64_t generalRXDrops = 0;
	uint64_t generalRXErrors = 0;
	uint64_t generalTXDrops = 0;

	for( i = 0; i < 16; i++)
	{
		struct net_device* vapDev = dtlk_dev_from_vapid(i);
		if( vapDev )
		{
			// TODO: implement function to get mib from vap
			volatile vap_data_mib_t *vap_mib_rx =  (vap_data_mib_t* )DLRX_VAP_MIB_BASE(i);
			volatile mib_table_t *vap_mib_tx = (mib_table_t *)SB_BUFFER(__D6_PER_VAP_MIB_BASE);
			uint64_t rxpdu = (uint64_t)vap_mib_rx->rx_rcv_pdu_low + (((uint64_t)vap_mib_rx->rx_rcv_pdu_high) << 32) ;
			uint64_t rxbytes = (uint64_t)vap_mib_rx->rx_rcv_bytes_low + (((uint64_t)vap_mib_rx->rx_rcv_bytes_high) << 32);
			uint64_t txdrops = (uint64_t)vap_mib_rx->txdrop_low + (((uint64_t)vap_mib_rx->txdrop_high) << 32);
			uint64_t rxerros = ((uint64_t)vap_mib_rx->rx_pn_bytes_low + (((uint64_t)vap_mib_rx->rx_pn_bytes_high) << 32)) 
				+ ((uint64_t)vap_mib_rx->rx_discard_pdu_low + (((uint64_t)vap_mib_rx->rx_discard_pdu_high) << 32));
			uint64_t rxdrops = (uint64_t)vap_mib_rx->rx_drop_pdu_low + (((uint64_t)vap_mib_rx->rx_drop_pdu_high) << 32);
			uint64_t txpdu = (uint64_t)vap_mib_tx[i].txpdu + (((uint64_t)vap_mib_tx[i].txpdu_high) << 32);
			uint64_t txbytes = (uint64_t)vap_mib_tx[i].txbytes + (((uint64_t)vap_mib_tx[i].txbytes_high) << 32);
			seq_printf(seq,"VAP-Id = %d\n",i);
			seq_printf(seq,"  VAP-Name = %s\n",vapDev->name);
			seq_printf(seq,"    tx_pkts    = %llu\n", txpdu);
			seq_printf(seq,"    tx_bytes   = %llu\n", txbytes);
			seq_printf(seq,"    tx_drops   = %llu\n",txdrops);
			seq_printf(seq,"    rx_pkts    = %llu\n",rxpdu);
			seq_printf(seq,"    rx_bytes   = %llu\n",rxbytes);
			seq_printf(seq,"    rx_error_pkts  = %llu\n",rxerros);
			seq_printf(seq,"    rx_drops_pkts  = %llu\n",rxdrops);
			seq_printf(seq,"\n");
			generalRXDrops += rxdrops;
			generalRXErrors += rxerros;
		}else
		{
			seq_printf(seq,"VAP-Id = %d\n",i);
			seq_printf(seq,"  VAP-Name = Unassigned\n");
		}
	}
	seq_printf(seq,"\n");
	seq_printf(seq,"General RX Drops = %10llu General RX Errors = %10llu\n",generalRXDrops,generalRXErrors);
	seq_printf(seq,"General TX Drops = %10llu\n",generalTXDrops);
    return 0;
}

static int proc_read_dtlk_mib_seq_open(struct inode *inode, struct file *file)
{

    return single_open(file, proc_read_dtlk_mib, NULL);
}

static ssize_t proc_write_dtlk_mib_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	int len;
    char str[64];
    char *p;

    int spaceLen = 0;
    int i;
	uint32_t vapMask = 0; //VAP mask to clear mib counter

    len = min(count, (unsigned long)sizeof(str) - 1);
    len -= copy_from_user(str, buf, len);
    while ( len && str[len - 1] <= ' ' )
        len--;
    str[len] = 0;
    for ( p = str; *p && *p <= ' '; p++, len-- );
    if ( !*p )
        return count;
	//first, looking for VAP from user input 
	
	p = get_next_argument(p,&spaceLen);
	len -= spaceLen;
	//get vap id
	if( strincmp(p,"reset",strlen("reset") ) == 0)
	{
		p+=strlen("reset");
		len -= strlen("reset");
		
		while(p && *p)
		{
			int vapID = 0;
			p = get_next_argument(p,&spaceLen);
			//sanity check
			if( !p )
				break;
			
			len -= spaceLen;
			if( strincmp(p,"all",strlen("all")) == 0 )
			{
				//mask all
				vapMask = 0xffffffff;
				break;
			}else
			{
				if( *p >= '0' && *p <= '9' )
				{
					sscanf(p,"%d",&vapID);
					if( vapID >= MAX_VAP_NUM )
					{
						dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s: wrong user input argument - invalid VAP id [%d]\n",__FUNCTION__,vapID);
						break;//while
					}
					vapMask |= 1 << vapID;
					if( vapID > 9 ){
						len -= 2;
						p += 2;
					}
					else{
						len -= 1;
						p += 1;
					}
				}else
					break;
			}
			if( !p )
				break;
			p+=1;
			len -=1;
		}
	}
	else if ( strincmp(p, "help", 4) == 0 || *p == '?' )
    {
         printk("echo reset <0 1 2 ...> > /proc/%s/wifi_mib\n",DIRECTLINK_PROC_DIR);
         printk("echo reset all > /proc/%s/wifi_mib\n",DIRECTLINK_PROC_DIR);
		 return count;
    }
		
	
	dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s: Going to clear: 0x%08x\n",__FUNCTION__,vapMask);
	//secondly, clear vap mib counter
	if( vapMask == 0xffffffff )
	{
		//volatile dlrx_cfg_mib_reset_t *resetMib =   (volatile dlrx_cfg_mib_reset_t* )DLRX_CFG_MIB_RESET_BASE;
		dre_dl_reset_fn_t dre_dl_reset_fn;
		uint32_t* ptrTxMib = (void *)SB_BUFFER(__D6_PER_VAP_MIB_BASE );
		//clear rx mib counter
		//resetMib->allreq = 1;
		//clear tx mib counter
		memset(ptrTxMib,0x0, sizeof(mib_table_t)*MAX_VAP_NUM);
		dre_dl_reset_fn = g_dre_fnset.dre_dl_reset_fn;
        if(likely(dre_dl_reset_fn)){
			//printk("Clear all\n");
            dre_dl_reset_fn(DRE_RESET_MIB, 0xff);
        }else{
            printk("%s: Function DRE_RESET_MIB is not registered!\n",__FUNCTION__);
        }
        
	}else
	{
		//volatile dlrx_cfg_mib_reset_t *resetMib =   (volatile dlrx_cfg_mib_reset_t* )DLRX_CFG_MIB_RESET_BASE;
		dre_dl_reset_fn_t dre_dl_reset_fn;
		dre_dl_reset_fn = g_dre_fnset.dre_dl_reset_fn;
		for( i = 0; i < MAX_VAP_NUM;i++)
		{
			if( (1 << i) & vapMask )
			{
				//uint32_t clearVap = (1 << 30 | (i & 0xf)) ;
				uint32_t* ptrTxMib = (void *)SB_BUFFER(__D6_PER_VAP_MIB_BASE ) + i*sizeof(mib_table_t);
				//clear this vap mib counter
				//*(uint32_t *)resetMib = clearVap;
				//clear tx mib counter
				memset(ptrTxMib,0x0, sizeof(mib_table_t));
		        if(likely(dre_dl_reset_fn)){
					//printk("Clear %d\n",i);
		            dre_dl_reset_fn(DRE_RESET_MIB, i);
		        }else{
		            printk("%s: Function DRE_RESET_MIB is not registered!n",__FUNCTION__);
		        }
			}
		}
	}
	
    return count;

}


static struct file_operations g_proc_file_mib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_mib_seq_open,
    .read       = seq_read,
    .write      = proc_write_dtlk_mib_seq,
    .llseek     = seq_lseek,
    .release    = single_release
};
/*
 * ####################################
 *            Local Function
 * ####################################
 */

/* Functions and structure support proc/dtlk/dbg*/
static int proc_read_dtlk_dbg(struct seq_file *seq, void *v)
{
	int i;

    for ( i = 0; i < NUM_ENTITY(dtlk_dbg_enable_mask_str) -1; i++ )  //skip -1
    {
        seq_printf(seq, "%-10s(%-40s):        %-5s\n", dtlk_dbg_enable_mask_str[i].cmd, dtlk_dbg_enable_mask_str[i].description, (g_dtlk_dbg_enable & dtlk_dbg_enable_mask_str[i].flag)  ? "enabled" : "disabled");
    }

    return 0;

}

static int proc_read_dtlk_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dtlk_dbg, NULL);
}

static ssize_t proc_write_dtlk_dbg_seq(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	int len;
    char str[64];
    char *p;
    int f_enable = 0;
    int i;
	int spaceLen = 0;

    len = min(count, (unsigned long)sizeof(str) - 1);
    len -= copy_from_user(str, buf, len);
    while ( len && str[len - 1] <= ' ' )
        len--;
    str[len] = 0;
    for ( p = str; *p && *p <= ' '; p++, len-- );
    if ( !*p )
        return count;

    if ( strincmp(p, "enable", 6) == 0 )
    {
        p += 6 ;  
        len -= 6 ;  
        f_enable = 1;
    }
    else if ( strincmp(p, "disable", 7) == 0 )
    {
        p += 7 ;  
        len -= 7 ; 
        f_enable = -1;
		
    }
    else if ( strincmp(p, "help", 4) == 0 || *p == '?' )
    {
         printk("echo <enable/disable> [");
         for ( i = 0; i < NUM_ENTITY(dtlk_dbg_enable_mask_str); i++ ) printk("%s/", dtlk_dbg_enable_mask_str[i].cmd );
         printk("] > /proc/%s/dbg\n",DIRECTLINK_PROC_DIR);
		 return count;
    }
	
	p = get_next_argument(p,&spaceLen);
	len -= spaceLen;

    if ( f_enable )
    {
        if ( (len <= 0) || ( p[0] >= '0' && p[1] <= '9') )
        {
            if ( f_enable > 0 )
                g_dtlk_dbg_enable |= DTLK_DBG_ENABLE_MASK_ALL;
            else
                g_dtlk_dbg_enable &= ~DTLK_DBG_ENABLE_MASK_ALL;
        }
        else
        {
        	do
            {

           	 	for ( i = 0; i < NUM_ENTITY(dtlk_dbg_enable_mask_str); i++ )
	                if ( strincmp(p, dtlk_dbg_enable_mask_str[i].cmd, strlen(dtlk_dbg_enable_mask_str[i].cmd) ) == 0 )
	                {
	                    if ( f_enable > 0 )
	                        g_dtlk_dbg_enable |= dtlk_dbg_enable_mask_str[i].flag;
	                    else
	                        g_dtlk_dbg_enable &= ~dtlk_dbg_enable_mask_str[i].flag;
	                    p += strlen(dtlk_dbg_enable_mask_str[i].cmd); //skip one blank
	                    p = get_next_argument(p,&spaceLen);
	                    len -= strlen(dtlk_dbg_enable_mask_str[i].cmd) + spaceLen; //skip one blank. len maybe negative now if there is no other parameters
	                    break;
	                }
			} while ( i < NUM_ENTITY(dtlk_dbg_enable_mask_str) );
        }
    }

    return count;

}


static struct file_operations g_proc_file_dbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dtlk_dbg_seq_open,
    .read       = seq_read,
    .write      = proc_write_dtlk_dbg_seq,
    .llseek     = seq_lseek,
    .release    = single_release
};

/**
  * proc_file_create - Create proc files for driver
  *
  *
  * All proc files will be located in /proc/dtlk. Called by
  * dlrx_drv_init
  * Return: 0 : success. Others: fail
  */
static INLINE int proc_file_create(void)
{
	struct proc_dir_entry *res;
	int ret = 0;

	if (g_dtlk_proc_dir ) {
	 	dtlk_err("More than one DirectLink device found!\n");
	 	return -EEXIST;
	}

	//create parent proc directory
   	g_dtlk_proc_dir = proc_mkdir(DIRECTLINK_PROC_DIR, NULL);
	//create mib 
	res  = proc_create("wifi_mib",S_IRUGO|S_IWUSR, g_dtlk_proc_dir, &g_proc_file_mib_seq_fops);

	if ( !res ) {
		dtlk_err("Failed to create mib\n");
		return -ENODEV;
	}

	//create wifi_port 
	res  = proc_create("wifi_port",S_IRUGO, g_dtlk_proc_dir, &g_proc_file_wifi_port_seq_fops);

	if ( !res ) {
		dtlk_err("Failed to create wifi_port\n");
		return -ENODEV;
	}

	//create peer_id
	res  = proc_create("peer_id",S_IRUGO, g_dtlk_proc_dir, &g_proc_file_peer_id_seq_fops);

	if ( !res ) {
		dtlk_err("Failed to create peer_id\n");
		return -ENODEV;
	}

	//create peer_to_vap
	res  = proc_create("peer_to_vap",S_IRUGO, g_dtlk_proc_dir, &g_proc_file_peer_to_vap_seq_fops);

	if ( !res ) {
		dtlk_err("Failed to create peer_to_vap\n");
		return -ENODEV;
	}


	//create dbg 
	res  = proc_create("dbg",S_IRUGO|S_IWUSR, g_dtlk_proc_dir, &g_proc_file_dbg_seq_fops);

	if ( !res ) {
		dtlk_err("Failed to create dbg\n");
		ret = -ENODEV;
	}

	//create ver
	res  = proc_create("ver",S_IRUGO, g_dtlk_proc_dir, &g_proc_file_ver_seq_fops);
	
	if ( !res ) {
		dtlk_err("Failed to create version\n");
		ret = -ENODEV;
	}

	//create reg
	res  = proc_create("reg",S_IRUGO, g_dtlk_proc_dir, &g_proc_file_reg_seq_fops);
	
	if ( !res ) {
		dtlk_err("Failed to create register\n");
		ret = -ENODEV;
	}

	

	return ret;
}

/**
  * proc_file_create - Delete all proc files created by this driver
  *
  *
  * Delete all proc files in /proc/dtlk.
  * Called by dlrx_drv_exit.
  */
static INLINE void proc_file_delete(void)
{
	//delete reg  entry
	remove_proc_entry("reg", g_dtlk_proc_dir);
	//delete ver  entry
	remove_proc_entry("ver", g_dtlk_proc_dir);
	//delete mib entry
    remove_proc_entry("wifi_mib", g_dtlk_proc_dir);
	//delete dbg
	remove_proc_entry("dbg", g_dtlk_proc_dir);
	//delete wifi_port
	remove_proc_entry("wifi_port", g_dtlk_proc_dir);

	//delete peer_id
	remove_proc_entry("peer_id", g_dtlk_proc_dir);

	//delete peer_id
	remove_proc_entry("peer_to_vap", g_dtlk_proc_dir);

	//delete parent proc directory
	remove_proc_entry(DIRECTLINK_PROC_DIR, NULL);

	g_dtlk_proc_dir = NULL;
}



static int __init dlrx_drv_init(void)
{
#ifdef SUPPORT_UNLOAD_DTLK
    ppa_dp_dlrx_init();
#endif

    if(g_dtlk_memsize < DTLK_TX_RSV_MEM_SIZE + DTLK_RX_RSV_MEM_SIZE){
        goto __no_resv_err;
    }
    if(g_dma_des_len > GSWIP_DESC_NUM || g_dma_des_len < 2){
        g_dma_des_len = GSWIP_DESC_NUM;
    }
    if(g_dma_des_len % 2 != 0){
        g_dma_des_len += 1;
    }

    g_dma_skb_buf = kmalloc((g_dma_des_len / 2) * sizeof(uint32_t), GFP_KERNEL);
    if(!g_dma_skb_buf){
        goto __alloc_fail;
    }
    memset(g_dma_skb_buf, 0, (g_dma_des_len / 2) * sizeof(uint32_t)); 
    if(g_ce5_desc_ring_num >= CE5_DEST_DESC_RING_NUM){
        g_ce5_desc_ring_num = CE5_DEST_DESC_RING_NUM;
    }
    if(g_ce5_buf_ptr_ring_num < g_ce5_desc_ring_num){
       g_ce5_buf_ptr_ring_num = g_ce5_desc_ring_num;
    }
    
    dlrx_data_structure_init();
    dtlk_rx_api_init();
    set_vap_itf_tbl_fn = set_vap_itf_tbl;

	proc_file_create();
	#if 0
	/* Switch setting to support burst situation */
	*(unsigned int *)KSEG1ADDR(SWITCH_BURST_LOW_THRESHOLD_ADDR) = 0x30;
	*(unsigned int *)KSEG1ADDR(SWITCH_BURST_HIGH_THRESHOLD_ADDR) = 0x50;
    #endif
    printk("DLRX driver init successfully!\n");
    printk("DLRX driver version: 0x%x\n", DLRX_DRV_VERSION);
    
    return 0;

__alloc_fail:
    dtlk_err("Fail to alloc the skb buf, len:%d", g_dma_des_len);
	return -1;

__no_resv_err:
    dtlk_err("No dtlk reserved memory!!");
    
    return -1;
}

static void __exit dlrx_drv_exit(void)
{
    int i = 0;
	struct sk_buff *skb;
	//firstly: check for all activities and stop them
	//firstly: stop RX
#ifdef SUPPORT_UNLOAD_DTLK
	//ppa_dp_dlrx_exit(); will not disable DMA 7, it may cause the mismatch between firmware pointer
	dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s:Not disable DMA 7\n",__func__);
#endif

	//secondly: stop firmware

	//clean proc

    proc_file_delete();
    
    dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s: Try to free all buffer in replenish list\n",__func__);
    dtlk_rx_api_exit();
    //free buffer that sent to switch
    for(i = 0; i < (g_dma_des_len / 2);i++)
    {
        if( g_dma_skb_buf[i] != 0)
        {
            if(g_dma_skb_buf[i] >= 0x80000000 && g_dma_skb_buf[i])
            {
                skb = (struct sk_buff *)g_dma_skb_buf[i];
                dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"Try to free in switch: skb buffer %x at index [%d]\n",g_dma_skb_buf[i],i);
                dev_kfree_skb_any(skb);
            }else
            {
                if (g_dma_skb_buf[i])
                {
                    skb = (struct sk_buff *)(g_dma_skb_buf[i] | 0x80000000);
                    dev_kfree_skb_any(skb);
                } else
                    dtlk_err("Try to free an invalid skb buffer %x at index [%d]\n",g_dma_skb_buf[i],i);
            }
  
        }
    }
    //free resources
    if ( g_dma_skb_buf )
    {
    	kfree(g_dma_skb_buf);
    }
    return;
}		

static int __init dtlk_mem_setup(char *str)
{
    char *dummy;
    g_dtlk_memsize = (uint32_t)memparse(str, &dummy);
	printk("dtlk mem size: 0x%x\n", g_dtlk_memsize);

    return 0;    
}

__setup("dtlkm=", dtlk_mem_setup);
/* Module Definitions */
module_init(dlrx_drv_init);
module_exit(dlrx_drv_exit);

/* Module parameters */
MODULE_PARM_DESC(g_dma_des_len, "DMA TX descripto number to switch");
module_param(g_dma_des_len, uint, S_IRUGO);
MODULE_PARM_DESC(g_ce5_desc_ring_num, "CE5 Dest Ring number");
module_param(g_ce5_desc_ring_num, uint, S_IRUGO);
MODULE_PARM_DESC(g_ce5_buf_ptr_ring_num, "CE5 Dest Buffer Pointer Ring number, Must larger or equal to CE5 Dest Ring Number");
module_param(g_dtlk_memsize, uint, S_IRUGO);
MODULE_PARM_DESC(g_dtlk_memsize, "Directlink Reserved memsize");
module_param(g_ce5_buf_ptr_ring_num, uint, S_IRUGO);

MODULE_AUTHOR(MOD_AUTHOR);
MODULE_DESCRIPTION(MOD_DESCRIPTION);
MODULE_LICENSE(MOD_LICENCE);
