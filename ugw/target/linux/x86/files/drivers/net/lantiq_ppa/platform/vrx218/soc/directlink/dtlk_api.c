/*****************************************************************************
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
 ******************************************************************************/

/*************************************************
 *  This file provide all the APIs that need export to DLRX_FW or 11AC driver
 *************************************************/

/*************************************************
 *          Head File
 *************************************************/
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
#include <linux/skbuff.h>
#include <linux/dma-mapping.h>
#include <linux/list.h>

#include <net/ppa_ppe_hal.h>
#include <net/ppa_stack_al.h>

/* FW header files */
#include "../11ac_acc_data_structure_tx_be.h"
#include "./include/dlrx_fw_data_structure_be.h"
#include "./include/dlrx_fw_def.h"
#include "./include/dlrx_fw_version.h"
/*DLRX driver header files */
#include "./include/dlrx_drv.h"
#include "./include/dlrx_memory_lyt.h"
#include "./include/dlrx_dre_api.h"
#include "./include/dlrx_wlan_api.h"

#include "../directlink_tx_cfg.h"
#include "../ltqmips_hal.h"



/*************************************************
 *          Definition 
 *************************************************/
#define PHY_ADDR_MASK 0x1FFFFFFF


/*************************************************
 *          Global Variable
 *************************************************/
uint32_t g_dma_skb_idx = 0;
uint32_t *g_dma_des_base;
uint32_t g_dlrx_max_inv_header_len = 0;
uint32_t g_dlrx_cfg_offset_atten = 0;
void *g_dlrx_handle = NULL;
PPA_QCA_DL_RX_CB g_dlrx_qca_cb = {0};
dre_regfn_set_t g_dre_fnset    = {0};
spinlock_t g_vap2int_tbl_lock;
spinlock_t g_prid2pr_tbl_lock;
spinlock_t g_pr2vap_tbl_lock;
spinlock_t g_pr2handler_tbl_lock;

LIST_HEAD(g_dlrx_skblist);



int set_peer_id_to_peer_table(uint32_t, uint32_t , uint32_t *, unsigned int, unsigned int);
int remove_peer_id_from_table( uint32_t,uint32_t);
int remove_peer_from_table( uint32_t ,uint32_t );
int get_handler_index( uint32_t );
int get_free_peer_number( unsigned int *);

/*************************************************
 *          Static functions
 *************************************************/
static struct sk_buff *alloc_skb_tx_aligned(struct sk_buff *skb, int len)
{
    if ( skb )
        dev_kfree_skb_any(skb);

    skb = dev_alloc_skb(len + DTLK_ALIGNMENT * 2);
    if ( skb )
    {
        skb_reserve(skb, (~((u32)skb->data + (DTLK_ALIGNMENT - 1)) & (DTLK_ALIGNMENT - 1)) + DTLK_ALIGNMENT);
        ASSERT(((u32)skb->data & (DTLK_ALIGNMENT - 1)) == 0, "skb->data (%#x) is not 8 DWORDS aligned", (u32)skb->data);
    }

    return skb;
}

static struct sk_buff *alloc_skb_tx(int len)
{
    struct sk_buff *skb = NULL;

    len = (len + DTLK_ALIGNMENT - 1) & ~(DTLK_ALIGNMENT - 1);

    skb = dev_alloc_skb(len);
    if ( skb )
    {
        if ( ((u32)skb->data & (DTLK_ALIGNMENT - 1)) != 0 && !(skb = alloc_skb_tx_aligned(skb, len)) )
            return NULL;
        *((u32 *)skb->data - 1) = (u32)skb;
        
        //Debug Clear Attention bit.
        *((uint32_t *)skb->data + 1) = 0;
#ifndef CONFIG_MIPS_UNCACHED
        dma_cache_wback((u32)skb->data - sizeof(u32), sizeof(u32) + 3); //+3 to invalidate attention config change as well.
#endif
    }

    return skb;
}

static struct sk_buff *__get_skb_pointer(uint32_t dataptr, const char *func_name, unsigned int line_num)
{
    unsigned int skb_dataptr;
    struct sk_buff *skb;

    if ( dataptr == 0 ) {
        dtlk_print("dataptr is 0, it's supposed to be invalid pointer");
        return NULL;
    }

    skb_dataptr = KSEG1ADDR(dataptr - 4);
    skb = *(struct sk_buff **)skb_dataptr;
    if((uint32_t)skb < KSEG0 || (uint32_t)skb >= 0xA0000000){
        printk("%s:%d: invalid skb - skb = %#08x, dataptr = %#08x\n", func_name, line_num, (unsigned int)skb, dataptr);
    }

    ASSERT((unsigned int)skb >= KSEG0, "%s:%d: invalid skb - skb = %#08x, dataptr = %#08x", func_name, line_num, (unsigned int)skb, dataptr);
    //ASSERT((((unsigned int)skb->data & (0x1FFFFFFF ^ (DTLK_ALIGNMENT - 1))) | KSEG1) == (dataptr | KSEG1), "%s:%d: invalid skb - skb = %#08x, skb->data = %#08x, dataptr = %#08x", func_name, line_num, (unsigned int)skb, (unsigned int)skb->data, dataptr);

    return skb;
}

static inline struct sk_buff *dlrx_get_skb_ptr(unsigned int dataptr)
{
    return __get_skb_pointer(dataptr, __FUNCTION__, __LINE__);    
}

/*
*  Get the DRE FW API pointer 
*/ 
static void *dlrx_get_dre_fn(unsigned int fntype)
{
    switch(fntype){
        case DRE_MAIN_FN:
            return g_dre_fnset.dre_dl_main_fn;
        case DRE_GET_VERSION_FN:
            return g_dre_fnset.dre_dl_getver_fn;
        case DRE_RESET_FN:
            return g_dre_fnset.dre_dl_reset_fn;
        case DRE_GET_MIB_FN:
            return g_dre_fnset.dre_dl_getmib_fn;
        case DRE_GET_CURMSDU_FN:
            return g_dre_fnset.dre_dl_getmsdu_fn;
        //case DRE_REPL_FN:
        //    return g_dre_fnset.dre_dl_replinsh_fn;
        case DRE_SET_MEMBASE_FN:
            return g_dre_fnset.dre_dl_set_membase_fn;
        case DRE_SET_RXPN_FN:
            return g_dre_fnset.dre_dl_set_rxpn_fn;
        case DRE_SET_DLRX_UNLOAD:
            return g_dre_fnset.dre_dl_set_dlrx_unload_t;
        default:
            return NULL;
    }

    return NULL;

}

/*
 *  Extract and Setup the skb structure
 */
static struct sk_buff *dlrx_skb_setup(unsigned int rxpb_ptr, unsigned int data_ptr, unsigned int data_len)
{
    struct sk_buff *skb;
    skb = dlrx_get_skb_ptr(rxpb_ptr);

    //dtlk_print("skb: 0x%x, rxpb_ptr: 0x%x, data_ptr: 0x%x, data_len: 0x%x\n",
    //    (uint32_t)skb, rxpb_ptr, data_ptr, data_len);

    if((uint32_t)skb < KSEG0 || (uint32_t)skb >= 0xA0000000){
        dtlk_err("skb address is not correct: skb: 0x%x, data_ptr: 0x%x, rxpb_ptr: 0x%x\n", 
            (uint32_t)skb, data_ptr, rxpb_ptr);
        return NULL;
    }

    //adjust skb
    skb->data = (unsigned char *)KSEG0ADDR(data_ptr);
    skb->len  = data_len;
    skb->tail = skb->data + data_len;
    
    return skb;
}

static void dump_skb(struct sk_buff *skb, uint32_t len, char * title)
{
    int i;
    
    if ( skb->len < len )
        len = skb->len;

    if ( len > DTLK_PACKET_SIZE )
    {
        dtlk_print("too big data length: skb = %08x, skb->data = %08x, skb->len = %d\n", (u32)skb, (u32)skb->data, skb->len);
        return;
    }

    dtlk_print("%s\n", title);
    dtlk_print(" skb = 0x%x, skb->data = %08X, skb->tail = %08X, skb->len = %d\n", (u32)skb, (u32)skb->data, (u32)skb->tail, (int)skb->len);
    for ( i = 1; i <= len; i++ )
    {
        if ( i % 16 == 1 )
            dtlk_print("  %4d:", i - 1);
        dtlk_print(" %02X", (int)(*((char*)skb->data + i - 1) & 0xFF));
        if ( i % 16 == 0 )
            dtlk_print("\n");
    }
    if ( (i - 1) % 16 != 0 )
        dtlk_print("\n");
}

#define REP_SKB_NUM 256

void skb_list_replenish(struct list_head *head)
{
    int i;
    struct sk_buff *skb;
	
    for(i = 0; i < REP_SKB_NUM; i ++){
        skb = alloc_skb_rx();
        if(!skb){
            dtlk_err("Alloc SKB fail!!!\n");
            return;
        }

        dma_cache_inv((u32)skb->data, skb->end - skb->data);
        list_add((struct list_head *)skb, head);
    }
}

unsigned int skb_list_get_skb(unsigned int rxpb_ptr)
{
    struct list_head *pos, *q;
    list_for_each_safe(pos, q, &g_dlrx_skblist){
        if( pos )
        {
            struct sk_buff *buff = pos;
            if( buff->data == rxpb_ptr )
            {
                list_del(pos);
                return 1;
            }
        }
    }
    return 0;
}
/*
* Free all skb buffers in the recycle list
*/
void skb_list_exhaust(struct list_head *head)
{
    struct list_head *pos, *q;
    int count = 0;
    list_for_each_safe(pos, q, head){
        list_del(pos);
        if( pos )
        {
            struct sk_buff *buff = pos;
            dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s: %x shoudl be the same %x \n",__func__,buff,*((u32 *)buff->data - 1));
            //packet already free by this function, no need to free in the ring
            //*((u32 *)buff->data - 1) = (u32)0;
            dev_kfree_skb_any((struct skb_buff *)pos);
            count++;
        }
    }
    dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s: release %d in list \n",__func__,count);
}

void dlrx_skb_recycle(struct sk_buff *skb /*, int inv_type */)
{
    if(unlikely(!skb))
    {
        return;
    }
   
	// TODO: James test DMA 
#if 0
	if(inv_type == INVALIDATE_HEADER){
	    dma_cache_inv((u32)skb->data,g_dlrx_max_inv_header_len);
    }else{
        dma_cache_inv((u32)skb->data, skb->end - skb->data);
    }
#else
    dma_cache_inv((u32)skb->data,g_dlrx_max_inv_header_len);
#endif
    list_add((struct list_head *)skb, &g_dlrx_skblist);
}

/*************************************************
 *          Global Functions
 *************************************************/
struct sk_buff *alloc_skb_rx(void)
{
    return alloc_skb_tx(DTLK_PACKET_SIZE);
}

/*
* This function is called when DTLK driver is unloaded.
*/
void dtlk_rx_api_exit(void)
{
    skb_list_exhaust(&g_dlrx_skblist);
    *DLRX_SKB_POOL_CTXT = 0;
}


void dtlk_rx_api_init(void)
{
    spin_lock_init(&g_vap2int_tbl_lock);
    spin_lock_init(&g_prid2pr_tbl_lock);
    spin_lock_init(&g_pr2vap_tbl_lock);
	spin_lock_init(&g_prid2pr_tbl_lock);
    spin_lock_init(&g_pr2handler_tbl_lock);

    skb_list_replenish(&g_dlrx_skblist);
    *DLRX_SKB_POOL_CTXT = (unsigned int)&g_dlrx_skblist;
    return;
}

/*************************************************
 *          APIs for DLRX FW
 *************************************************/
/* 
 * Allocate a new rxpb ring pkt buffer
 */
unsigned int dlrx_dl_dre_rxpb_buf_alloc(void)
{
    struct sk_buff *skb;

#if 0    
    skb = alloc_skb_rx();
    if(!skb){
        dtlk_err("!!!Cannot Alloc buffer!!!\n");
        return 0;
    }
#else
    if(unlikely(list_empty(&g_dlrx_skblist))){
        skb_list_replenish(&g_dlrx_skblist);
    }

    skb = (struct sk_buff *)g_dlrx_skblist.next;
    list_del((struct list_head *)skb); 
#endif
    return (unsigned int)skb->data;    
}
EXPORT_SYMBOL(dlrx_dl_dre_rxpb_buf_alloc);


/* 
 * Free a  rxpb ring pkt buffer,called by DLRX firmware.
 */
int ppa_dl_dre_rxpb_buf_free(unsigned int rxpb_ptr)
{
    struct sk_buff *skb;
    static int count = 0;
    //sanity check
    if( !rxpb_ptr )
    {
        dtlk_err("%s: rxpb_ptr is NULL\n",__func__);
        return DTLK_FAILURE;
    }
    skb = dlrx_get_skb_ptr(rxpb_ptr);
    if(!skb)
    {
        dtlk_err("%s: rxpb_ptr[%x] skb is NULL\n",__func__,rxpb_ptr);
        return DTLK_FAILURE;

    }
    //printk("%s: %x [%d]\n",__func__,skb,count++);
    if( skb_list_get_skb(rxpb_ptr) == 1 )
    {
        //printk("%s: Inside replenish \n",__func__);
    }

    if( skb )
    {
        //printk("%s: [%x] [%x]\n",__func__,rxpb_ptr, skb);
        dev_kfree_skb_any(skb);
    }else
    {
        dtlk_err("%s: freed by replenish or invalid\n",__func__);
    }

    return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_dre_rxpb_buf_free);


int ppa_dl_dre_gswip_dma_send(unsigned int rxpb_ptr, unsigned int data_ptr, unsigned int data_len, unsigned int release_flag, unsigned int pmac_hdr_ptr,unsigned int unmap_type)
{
    struct sk_buff *skb, *skb2;
    int i;
    soc_dma_desc_t *dma_desc_pmac, *dma_desc_data;
    unsigned int pmac_addr, data_addr;

    //data_ptr     = CPHYSADDR(data_ptr);
    //pmac_hdr_ptr = CPHYSADDR(pmac_hdr_ptr);

    //dtlk_print("rxpb_ptr: 0x%x, data_ptr: 0x%x, data_len: 0x%x, release: %s, pmac_hdr: 0x%x\n",
    //    rxpb_ptr, data_ptr, data_len, release_flag == 1 ? "Yes":"NO", pmac_hdr_ptr);
    //dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"rxpb_ptr: 0x%x, data_ptr: 0x%x, data_len: 0x%x, release: %s, pmac_hdr: 0x%x\n",
    //    rxpb_ptr, data_ptr, data_len, release_flag == 1 ? "Yes":"NO", pmac_hdr_ptr);
    skb = dlrx_get_skb_ptr(rxpb_ptr);
    //check own bit
    i = g_dma_skb_idx * 2;
    dma_desc_pmac = (soc_dma_desc_t *)KSEG1ADDR(g_dma_des_base + (i << 1));
    dma_desc_data = (soc_dma_desc_t *)KSEG1ADDR(g_dma_des_base + ((i+1) << 1));
    if(dma_desc_data->own != DMA_CPU_OWNBIT){//Buffer full, cannot send, drop the packet
        dtlk_print("DMA buffer full, dma desc idx:%d !!!\n", i);
        if(release_flag){
            dlrx_skb_recycle(skb);
        }
        return DTLK_FAILURE;
    }
    if(!release_flag){     
		int header_len = data_ptr - rxpb_ptr;
        skb_put(skb, data_len + (data_ptr - rxpb_ptr) );
        skb2 = skb_copy(skb, GFP_ATOMIC); //DLRX FW invalidate cache of all the datas
        skb_trim(skb,0);
        if(!skb2)
            return DTLK_FAILURE;
        pmac_addr = CPHYSADDR(skb2->data + (pmac_hdr_ptr - rxpb_ptr));
        data_addr = CPHYSADDR(skb2->data + header_len);
        *((uint32_t *)skb2->data - 1) = (uint32_t)skb2;
#ifndef CONFIG_MIPS_UNCACHED
         //write back including the skb pointer 
        dma_cache_wback((u32)skb2->data - 1, skb2->tail - skb2->data + 2);  
#endif
        skb_trim(skb2,0);
    }else{
        skb2 = skb;  
#ifndef CONFIG_MIPS_UNCACHED
        dma_cache_wback((u32)skb2->data, g_dlrx_max_inv_header_len);  
#endif
        pmac_addr = CPHYSADDR(pmac_hdr_ptr);
        data_addr = CPHYSADDR(data_ptr);

    }
    //Release the original skb buffer in the array
    if(g_dma_skb_buf[g_dma_skb_idx] != 0){
        //ASSERT(g_dma_skb_buf[g_dma_skb_idx] >= 0x80000000 && g_dma_skb_buf[g_dma_skb_idx] < 0xA0000000 , "skb pointer is not correct: skb: 0x%x, skb idx: 0x%x\n", 
        //g_dma_skb_buf[g_dma_skb_idx], g_dma_skb_idx);  
        //dev_kfree_skb_any((struct sk_buff *)g_dma_skb_buf[g_dma_skb_idx]);
        skb = (struct sk_buff *)g_dma_skb_buf[g_dma_skb_idx];
		if (skb < 0x80000000)
		{
			skb = g_dma_skb_buf[g_dma_skb_idx] | 0x80000000;
			dev_kfree_skb_any(skb );
		}
		else
	        dlrx_skb_recycle(skb);            
    }
	if(!release_flag)
	    g_dma_skb_buf[g_dma_skb_idx] = (uint32_t)skb2 & (~0x80000000);
	else
		g_dma_skb_buf[g_dma_skb_idx] = (uint32_t)skb2;
    g_dma_skb_idx = (g_dma_skb_idx + 1) % (g_dma_des_len / 2);
    
    //prepare the desc setup for pmac and data and send
    dma_desc_pmac->sop              = 1;
    dma_desc_pmac->eop              = 0;
    dma_desc_pmac->byte_offset      = pmac_addr & 0x1f;
    dma_desc_pmac->data_len         = 4;
    dma_desc_pmac->dataptr          = (pmac_addr & (~0x1f)) >> 3;

    dma_desc_data->own              = !DMA_CPU_OWNBIT;
    dma_desc_data->sop              = 0;
    dma_desc_data->eop              = 1;
    dma_desc_data->byte_offset      = data_addr & 0x1f;
    dma_desc_data->data_len         = data_len;
    dma_desc_data->dataptr          = (data_addr & (~0x1f)) >> 3;

    //memory barrier
    wmb();
    dma_desc_pmac->own              = !DMA_CPU_OWNBIT;
    
    return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_dre_gswip_dma_send);

int ppa_dl_dre_ps_send(unsigned int rxpb_ptr, unsigned int data_ptr, unsigned int data_len, unsigned int vap_id)
{
    struct sk_buff *skb;
    struct net_device *dev;
    skb = dlrx_skb_setup(rxpb_ptr, data_ptr, data_len);
    
    if(!skb){
        return DTLK_FAILURE;
    }
    dev = dtlk_dev_from_vapid((uint32_t)vap_id);
    if(!dev){//no valid device
        dtlk_print("No valid device pointer!!!\n");
        dev_kfree_skb_any(skb);
        //dlrx_skb_recycle(skb);
        return DTLK_FAILURE;
    }
    skb->protocol = eth_type_trans(skb, dev);
    dtlk_print("%s, skb: 0x%x, dev_name: %s, rxpb_ptr: 0x%x, data_ptr: 0x%x, data_len: 0x%x, vap_id: 0x%x, protocol: %d\n",
        __func__, (uint32_t)skb, dev->name, rxpb_ptr, (uint32_t)skb->data, skb->len, vap_id, skb->protocol);  
	//dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s, skb: 0x%x, dev_name: %s, rxpb_ptr: 0x%x, data_ptr: 0x%x, data_len: 0x%x, vap_id: 0x%x, protocol: %d\n",
    //   __func__, (uint32_t)skb, dev->name, rxpb_ptr, (uint32_t)skb->data, skb->len, vap_id, skb->protocol);

    dump_skb(skb, skb->len, "Send To Protocl Stack");
    //dma_unmap_single(skb->dev, CPHYSADDR((unsigned long)skb->data), skb->end - skb->data, DMA_FROM_DEVICE);
//#ifndef CONFIG_MIPS_UNCACHED
//    dma_cache_inv((u32)skb->data, skb->end - skb->data);
//#endif

    if(netif_rx(skb) == NET_RX_DROP){
        return DTLK_FAILURE;
    }

    return DTLK_SUCCESS;
    
}
EXPORT_SYMBOL(ppa_dl_dre_ps_send);

/*
 *  DLRX FW send pkt to WLAN driver
 */
int ppa_dl_dre_wlan_pkt_send(unsigned int rxpb_ptr, unsigned int data_len, unsigned int pkt_status, unsigned int msg_ptr, unsigned int vap_id, unsigned int flags)
{
    struct sk_buff *skb;
    int ret = DTLK_FAILURE;

	// TODO: James add ( 23Jun)
    skb = dlrx_skb_setup(rxpb_ptr, rxpb_ptr, data_len+g_dlrx_cfg_offset_atten);
    //     skb = dlrx_skb_setup(rxpb_ptr, rxpb_ptr, data_len);
    if(!skb){
        return DTLK_FAILURE;
    }

//#ifndef CONFIG_MIPS_UNCACHED
//        dma_cache_inv((u32)skb->data, data_len);
//#endif
    dtlk_print("%s, rxpb_ptr: 0x%x, data_len: 0x%x, pkt_status: 0x%x, msg_ptr: 0x%x, flags: 0x%x\n",
        __func__, rxpb_ptr, data_len, pkt_status, msg_ptr, flags);    
	//dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s, rxpb_ptr: 0x%x, data_len: 0x%x, pkt_status: 0x%x, msg_ptr: 0x%x, flags: 0x%x\n",
	//		__func__, rxpb_ptr, data_len, pkt_status, msg_ptr, flags);	 

    if(likely(g_dlrx_qca_cb.rx_splpkt_fn)){
        ret = g_dlrx_qca_cb.rx_splpkt_fn(g_dlrx_handle, pkt_status, data_len, skb, (uint32_t *)msg_ptr, flags);
    }else{
        dtlk_err("No Special PKT fn handler!");
        dev_kfree_skb_any(skb);
        //dlrx_skb_recycle(skb);
    }
	/*
       * The QCA does not pass the packet to protocol stack.
       * DTLK will create new skb buffer for good packet and send it to protocol stack.
       */
    if (flags) {
        //Get the data pointer
        unsigned int data_ptr = rxpb_ptr + g_dlrx_cfg_offset_atten;
        struct net_device *dev;
		struct sk_buff *skb3 = dev_alloc_skb(data_len + 32);
        unsigned padding = (flags >> 16);
        data_ptr += padding;

        if(skb3 == NULL)
            dtlk_err("%s: Cannot alloc [%d]\n",__func__, data_len + 32);
        else {
			dev = dtlk_dev_from_vapid((uint32_t)vap_id);
            if (dev) {
				memcpy(skb_put(skb3, data_len), data_ptr, data_len);
				skb3->dev = dev;
				skb3->protocol = eth_type_trans(skb3, dev);
				skb3->ip_summed = CHECKSUM_UNNECESSARY;
		   		if(netif_rx(skb3) != NET_RX_DROP){
					dtlk_print("%s: Send to protocol stack successfully\n", __func__);
				} else {
					dev_kfree_skb_any(skb3);
					dtlk_err("%s: Cannot send to protocol stack\n", __func__);
				}
			}
		}
    }
    return ret;
}
EXPORT_SYMBOL(ppa_dl_dre_wlan_pkt_send);

/*
 *  DLRX FW send msg to WLAN driver
 */
int ppa_dl_dre_wlan_msg_send(unsigned int msg_type, unsigned int msg_ptr, unsigned int msg_len, unsigned int flags)
{       
    int ret = DTLK_FAILURE;
    if(likely(g_dlrx_qca_cb.rx_msg_fn)){
        dtlk_print("%s, msg_type: %d, msg_len: %d, msg_ptr: 0x%x, flags: 0x%x\n",
        __func__, msg_type, msg_len, msg_ptr, flags);  
		//dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s, msg_type: %d, msg_len: %d, msg_ptr: 0x%x, flags: 0x%x\n",
        //__func__, msg_type, msg_len, msg_ptr, flags);
        ret = g_dlrx_qca_cb.rx_msg_fn(g_dlrx_handle, msg_type, msg_len,(uint32_t *)msg_ptr, flags);
    }else{
        dtlk_err("No Message fn handler!");
    }
	
    return ret;    
}
EXPORT_SYMBOL(ppa_dl_dre_wlan_msg_send);

/*
*  DLRX register function
*/
int ppa_dl_dre_fn_register(unsigned int fntype, void *func)
{
    //if(func){
        switch(fntype){
            case DRE_MAIN_FN:
                g_dre_fnset.dre_dl_main_fn = func;
                break;
                
            case DRE_GET_VERSION_FN:
                g_dre_fnset.dre_dl_getver_fn = func;
                break;
                
            case DRE_RESET_FN:
                g_dre_fnset.dre_dl_reset_fn = func;
                break;
                
            case DRE_GET_MIB_FN:
                g_dre_fnset.dre_dl_getmib_fn = func;
                break;

            case DRE_GET_CURMSDU_FN:
                g_dre_fnset.dre_dl_getmsdu_fn = func;
                break;
                
            //case DRE_REPL_FN:
            //    g_dre_fnset.dre_dl_replinsh_fn = func;
            //    break;
                
            case DRE_SET_MEMBASE_FN:
                g_dre_fnset.dre_dl_set_membase_fn = func;
                break;

            case DRE_SET_RXPN_FN:
                g_dre_fnset.dre_dl_set_rxpn_fn = func;
                break;
            
			case DRE_SET_DLRX_UNLOAD:
							g_dre_fnset.dre_dl_set_dlrx_unload_t = func;
							break;
							
            case DRE_MAX_FN:
            default:
                dtlk_err("Register NO is Not valid:%d", fntype);
                return DTLK_FAILURE;
        }
		/*
    }else{
        dtlk_err("Register func is NULL");
        return DTLK_FAILURE;
    }
*/
    return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_dre_fn_register);

/*
 * Get Peer value from PeerID  
 * Return Peer Valid
 */
int ppa_dl_dre_peer_from_peerid(unsigned int peerid,unsigned int *peer)
{
    volatile unsigned int *pid2p_tbl = DLRX_CFG_PEER_ID_TO_PEER_MAP_BASE(0);
    unsigned int idx = peerid >> 2;
    unsigned int offset = peerid % 4;
    unsigned int peer_val;
    int result = 0;
 
    if(peerid >= MAX_PEERID_NUM)
        return PEER_INVALID;

    spin_lock_bh(&g_prid2pr_tbl_lock);
    peer_val = *(pid2p_tbl + idx);
    spin_unlock_bh(&g_prid2pr_tbl_lock);

    peer_val = (peer_val >> (offset << 3));
    *peer = peer_val & 0x7F;
    result = ((peer_val >> 7) & 0x1);
    return ((peer_val >> 7) & 0x1);
}
EXPORT_SYMBOL(ppa_dl_dre_peer_from_peerid);


int32_t ppa_dl_qca_clear_stats (	uint32_t vapId,	uint32_t flags )
{
	dre_dl_reset_fn_t dre_dl_reset_fn;
	dre_dl_reset_fn = dlrx_get_dre_fn(DRE_RESET_FN);
       
	if( flags == 1 )
	{
		//volatile dlrx_cfg_mib_reset_t *resetMib =   (volatile dlrx_cfg_mib_reset_t* )DLRX_CFG_MIB_RESET_BASE;
		uint32_t* ptrTxMib = (void *)SB_BUFFER(__D6_PER_VAP_MIB_BASE );
		//resetMib->allreq = 1;
		if(likely(dre_dl_reset_fn)){
            dre_dl_reset_fn(DRE_RESET_MIB, 0xff);
        }else{
            dtlk_err("%s: Function DRE_RESET_MIB is not registered!",__FUNCTION__);
        }
		memset(ptrTxMib,0x0, sizeof(mib_table_t)*MAX_VAP_NUM);
		return DTLK_SUCCESS;
	}
	if( vapId < __D6_SUPPORT_VAP_NUM)
	{
		//uint32_t clearVap = (1 << 30 | (vapId & 0xf)) ;
		uint32_t* ptrTxMib = (void *)SB_BUFFER(__D6_PER_VAP_MIB_BASE ) + vapId*sizeof(mib_table_t);
		//volatile dlrx_cfg_mib_reset_t *resetMib =   (volatile dlrx_cfg_mib_reset_t* )DLRX_CFG_MIB_RESET_BASE;
		//clear this vap mib counter
		//*(uint32_t *)resetMib = clearVap;
		if(likely(dre_dl_reset_fn)){
            dre_dl_reset_fn(DRE_RESET_MIB, vapId);
        }else{
            dtlk_err("%s: Function DRE_RESET_MIB is not registered!",__FUNCTION__);
        }
		memset(ptrTxMib,0,sizeof(mib_table_t));
		return DTLK_SUCCESS;
	}
	return DTLK_FAILURE;
}

EXPORT_SYMBOL(ppa_dl_qca_clear_stats);

/*
 * Set VAP info
 * Return -1 if peer or vapid out of range
 */
int ppa_dl_dre_vapinfo_set(unsigned int peer, unsigned int vapid, unsigned int sec_type, unsigned int acc_dis)
{
    volatile unsigned int *vapinfo_tbl = DLRX_CFG_PEER_TO_VAP_PN_BASE(0);
    unsigned int vapinfo;

    if(vapid >= MAX_VAP_NUM || peer >= MAX_PEER_NUM)
        return DTLK_FAILURE;

    vapinfo = ((acc_dis & 0x1) << 6) | ((sec_type & 0x3) << 4) | (vapid & 0xF);
    spin_lock_bh(&g_pr2vap_tbl_lock);
    *(vapinfo_tbl + peer) = vapinfo;
    spin_unlock_bh(&g_pr2vap_tbl_lock);

    return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_dre_vapinfo_set);


/*
 *  Get VAP info from Peer
 *  Return -1 if peer or vap id out of range
 */
int ppa_dl_dre_vapinfo_from_peer(unsigned int peer, unsigned int *vapid, unsigned int *sec_type, unsigned int *acc_dis)
{
    volatile unsigned int *vapinfo_tbl = DLRX_CFG_PEER_TO_VAP_PN_BASE(0);
    unsigned int vapinfo;

    if(peer >= MAX_PEER_NUM)
        return DTLK_FAILURE;

    spin_lock_bh(&g_pr2vap_tbl_lock);
    vapinfo = *(vapinfo_tbl + peer);
    spin_unlock_bh(&g_pr2vap_tbl_lock);

    *vapid    = vapinfo & 0xf;
    *sec_type = (vapinfo >> 4) & 0x3;
    *acc_dis  = (vapinfo >> 6) & 0x1;
    return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_dre_vapinfo_from_peer);

/*
 * Get interface id from VAP id
 */
unsigned int ppa_dl_dre_itf_from_vapid(unsigned int vap_id)
{
    volatile unsigned int *itf_tbl;
    volatile unsigned int itf_id;

    if(vap_id >= MAX_VAP_NUM)
        return DTLK_INVALID_ITFID;

    if(vap_id <= 7){//Range is defined in the spec
        itf_tbl = DLRX_CFG_VAP2INT_MAP1_BASE;
    }else{
        itf_tbl = DLRX_CFG_VAP2INT_MAP2_BASE;
    }

    vap_id = vap_id % 8;
    spin_lock_bh(&g_vap2int_tbl_lock);
    itf_id = (*itf_tbl >> (vap_id<<2)) & 0xF;
    spin_unlock_bh(&g_vap2int_tbl_lock);

    return itf_id;
    
}
EXPORT_SYMBOL(ppa_dl_dre_itf_from_vapid);



/*************************************************
 *          APIs for 11AC Driver
 *************************************************/ 
void ppa_dl_qca_register(void *dl_rx_handle, PPA_QCA_DL_RX_CB *dl_qca_rxcb,uint32_t flags)
{
    ASSERT((dl_rx_handle != NULL &&  dl_qca_rxcb != NULL), "dl_rx_handle or dl_qca_rxcb is NULL");
	ASSERT((flags == PPA_F_REGISTER) || (flags == PPA_F_DEREGISTER), "flag is not expected: %d\n", flags);
    dtlk_print("%s, dl_rx_handle: 0x%x, dl_qca_rxcb: 0x%x, flags: 0x%d\n",
        __func__, (uint32_t)dl_rx_handle, (uint32_t)dl_qca_rxcb, flags);
	//dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s, dl_rx_handle: 0x%x, dl_qca_rxcb: 0x%x, flags: 0x%d\n",
    //    __func__, (uint32_t)dl_rx_handle, (uint32_t)dl_qca_rxcb, flags);
		
	g_dlrx_handle = dl_rx_handle;

	switch(flags)
    {
        case PPA_F_REGISTER:
			{
			dre_dl_set_dlrx_unload_t dre_dl_set_dlrx_unload_fn;
            dre_dl_set_dlrx_unload_fn = g_dre_fnset.dre_dl_set_dlrx_unload_t;
            ASSERT((dl_rx_handle != NULL &&  dl_qca_rxcb != NULL), "dl_rx_handle or dl_qca_rxcb is NULL");
            g_dlrx_handle = dl_rx_handle;
            g_dlrx_qca_cb.rx_msg_fn     = dl_qca_rxcb->rx_msg_fn;
            g_dlrx_qca_cb.rx_splpkt_fn  = dl_qca_rxcb->rx_splpkt_fn;
            g_dlrx_qca_cb.vap_stats_fn  = dl_qca_rxcb->vap_stats_fn;
        	}
            break;
        case PPA_F_DEREGISTER:
            {
                printk("%s: deregister from QCA\n",__func__);
                dre_dl_set_dlrx_unload_t dre_dl_set_dlrx_unload_fn;
                dre_dl_set_dlrx_unload_fn = g_dre_fnset.dre_dl_set_dlrx_unload_t;
                //we stop dlrx firmware here
                if(  likely(dre_dl_set_dlrx_unload_fn ) )
                {
                    printk("%s: inform DRE to stop \n",__func__);
                    dre_dl_set_dlrx_unload_fn();
                    
                }
            }
			break;
        default:
		    break;
		
	}
		
	return;
}
EXPORT_SYMBOL(ppa_dl_qca_register);

//These functions need to merge with DL TX
//QCA Name: "QCA-11AC"
void ppa_directlink_manage(char *name,uint32_t flags)
{
    //Update the global structure cfg_global
	dlrx_cfg_global_t *dlrx_global=(dlrx_cfg_global_t *)DLRX_CFG_GLOBAL_BASE;
    
	if(flags == PPA_F_INIT)
	{
		dlrx_global->dlrx_enable = TRUE;
		dlrx_global->dltx_enable = TRUE;
	}
	else if(flags == PPA_F_UNINIT)
	{
		dlrx_global->dlrx_enable = FALSE;
        dlrx_global->dltx_enable = FALSE;
	}

    ppa_directlink_enable(flags);
        
	return;
}
EXPORT_SYMBOL(ppa_directlink_manage);
#ifdef SUPPORT_11AC_MULTICARD

/*API to get 11AC wireless card type */
extern int ppa_dl_detect_11ac_card();
#endif

/*
    This function initializes Target-to-Host (t2h) CE-5 Destination Ring in Direct Link between Target WLAN and PPE.
*/
void ppa_dl_qca_t2h_ring_init(uint32_t *t2h_ring_sz,uint32_t *dst_ring_base, uint32_t pcie_baddr, uint32_t flags)
{
    dre_dl_set_membase_fn_t  dre_dl_set_mb_fn;
    volatile dlrx_cfg_ctxt_ce5des_t *dlrx_cfg_ctxt_ce5des_ptr;
    volatile dlrx_cfg_ctxt_ce5buf_t *dlrx_cfg_ctxt_ce5buf_ptr;
    dlrx_cfg_global_t *dlrx_cfg_global_ptr;
	#ifdef SUPPORT_11AC_MULTICARD	
	unsigned int board_type = 0;
	#endif
	uint32_t ce5_read_addr = 0;
	uint32_t ce5_write_addr = 0;

	dtlk_print("%s: ddr base: 0x%08x, cfg_ctxt_base: 0x%x, pcie base: 0x%08x\n", 
         __func__, (uint32_t)ddr_base,
		 (uint32_t)cfg_ctxt_base,(uint32_t)pcie_baddr);

    pcie_base = (unsigned int *)KSEG1ADDR(pcie_baddr);
    dtlk_print("pcie base(virtual): 0x%x\n", (unsigned int)pcie_base);
    dlrx_cfg_global_ptr = (dlrx_cfg_global_t *)DLRX_CFG_GLOBAL_BASE;
    dlrx_cfg_global_ptr->dlrx_pcie_base = (unsigned int)pcie_base;
    
    dlrx_cfg_ctxt_ce5des_ptr = (dlrx_cfg_ctxt_ce5des_t *)DLRX_CFG_CTXT_CE5DES_BASE;
    *dst_ring_base = CPHYSADDR(dlrx_cfg_ctxt_ce5des_ptr->cfg_badr_ce5des);
    *t2h_ring_sz   = dlrx_cfg_ctxt_ce5des_ptr->cfg_num_ce5des;

    //QCA will update the HW register
	//*DLRX_TARGET_CE5_READ_INDEX  = 0;
    //*DLRX_TARGET_CE5_WRITE_INDEX = dlrx_cfg_ctxt_ce5buf_ptr->cfg_num_ce5buf - 1;
    dlrx_cfg_ctxt_ce5buf_ptr = (dlrx_cfg_ctxt_ce5buf_t *)DLRX_CFG_CTXT_CE5BUF_BASE;
#ifdef SUPPORT_11AC_MULTICARD
	if (ppa_dl_detect_11ac_card() == PEREGRINE_BOARD)
	{
		printk("%s: PEREGRINE_BOARD\n",__func__);
		dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_read_index  = (unsigned int)DLRX_TARGET_CE5_READ_INDEX(DLRX_TARGET_CE5_PERIGRINE);
		dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_write_index = (unsigned int)DLRX_TARGET_CE5_WRITE_INDEX(DLRX_TARGET_CE5_PERIGRINE);
	}else
	{
		printk("%s: BEELINER_BOARD\n",__func__);
		dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_read_index  = (unsigned int)DLRX_TARGET_CE5_READ_INDEX(DLRX_TARGET_CE5_BEELINER);
		dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_write_index = (unsigned int)DLRX_TARGET_CE5_WRITE_INDEX(DLRX_TARGET_CE5_BEELINER);	
	}
	//printk("%s: ce5 read 0x%x\n",__func__,dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_read_index);
	//printk("%s: ce5 write 0x%x\n",__func__,dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_write_index);	
#else
	dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_read_index  = (unsigned int)DLRX_TARGET_CE5_READ_INDEX(DLRX_TARGET_CE5_PERIGRINE);
	dlrx_cfg_ctxt_ce5buf_ptr->cfg_badr_target_ce5_write_index = (unsigned int)DLRX_TARGET_CE5_WRITE_INDEX(DLRX_TARGET_CE5_PERIGRINE);	
#endif

    dre_dl_set_mb_fn = dlrx_get_dre_fn(DRE_SET_MEMBASE_FN);
    if(dre_dl_set_mb_fn){
        dre_dl_set_mb_fn((unsigned int)ddr_base, (unsigned int)cfg_ctxt_base, (unsigned int)pcie_base);
    }else{
        dtlk_err("Not register function: set membase!!!");
    }
	#if 0
	/* overwrite flow control DLRX FW */
	*(unsigned int *)(DLRX_CFG_GLOBAL_BASE + 11) = 0x4000;
	*(unsigned int *)(DLRX_CFG_GLOBAL_BASE + 12) = 0x40;
	#endif
	
    return;
}
EXPORT_SYMBOL(ppa_dl_qca_t2h_ring_init);


/*
This function gets called from QCA driver to initialize or free the Rx packet buffers pool.
*/
void ppa_dl_qca_t2h_pktbuf_pool_manage (uint32_t *alloc_idx_ptr,uint32_t *t2h_rxpb_ring_sz,uint32_t *rxpb_ring_base,uint32_t flags )
{
    volatile dlrx_cfg_ctxt_rxpb_ptr_ring_t *dlrx_cfg_rxpb_ring; 
    dlrx_cfg_rxpb_ring = (dlrx_cfg_ctxt_rxpb_ptr_ring_t *)DLRX_CFG_CTXT_RXPB_PTR_RING_BASE;

	if(flags == PPA_F_INIT){
        *alloc_idx_ptr    = (uint32_t)&dlrx_cfg_rxpb_ring->rxpb_ptr_write_index;
        *t2h_rxpb_ring_sz = dlrx_cfg_rxpb_ring->cfg_num_rxpb_ptr_ring;
		*rxpb_ring_base   = CPHYSADDR(dlrx_cfg_rxpb_ring->cfg_badr_rxpb_ptr_ring);
	}
	
    return;
}
EXPORT_SYMBOL(ppa_dl_qca_t2h_pktbuf_pool_manage);

/*
This hook function is invoked by QCA WLAN Driver for addition or deletion of peer.
*/
int32_t ppa_dl_qca_set_peer_cfg (uint32_t *dlrx_peer_reg_handle, uint16_t peer_id, 
                                         uint16_t vap_id, PPA_WLAN_PN_CHECK_Type_t pn_chk_type, 
                                         uint32_t *rxpn,uint32_t flags)
{
	//Each Peer_id can have 8 peer values
	unsigned int peer;
    unsigned int temp_acc_dis;
    unsigned int temp_vap_id;
    unsigned int temp_sec_type;
    dre_dl_set_rxpn_fn_t dre_dl_set_rxpn_fn;

    
	switch(flags)
	{
		case PPA_WLAN_ADD_PEER_ID:
			//Set peer ID to peer
	        if( vap_id >= MAX_VAP_NUM )
            {   
                return DTLK_FAILURE;
            }
			if( set_peer_id_to_peer_table((uint32_t)dlrx_peer_reg_handle, peer_id, &peer, (unsigned int)vap_id, (unsigned int)pn_chk_type)!= DTLK_SUCCESS )
            {         
				return DTLK_FAILURE;
            }
            dtlk_print("peer: %d, peer_id: %d, vap_id %d, pn_chk_type: %d\n",
                peer, peer_id, vap_id, pn_chk_type);
		break;
            
		case PPA_WLAN_REMOVE_PEER://FOR DELETE PEER
			if( remove_peer_from_table( (uint32_t)dlrx_peer_reg_handle, (uint32_t)peer_id )!= DTLK_SUCCESS )
            {         
				return DTLK_FAILURE;
            }
		break;

        case PPA_WLAN_REMOVE_PEER_ID://For Delete PEER_ID
			if( remove_peer_id_from_table( (uint32_t)dlrx_peer_reg_handle, (uint32_t)peer_id ) != DTLK_SUCCESS )
            {         
			    return DTLK_FAILURE;
            }
		break;

        case PPA_WLAN_SET_PN_CHECK:// To Update PN SEC TYPE
		    peer = get_handler_index((uint32_t)dlrx_peer_reg_handle );
            if( peer == HANDLER_NOT_FOUND )
            {
                return DTLK_FAILURE;
            }
            ppa_dl_dre_vapinfo_from_peer( peer, &temp_vap_id, &temp_sec_type, &temp_acc_dis );
            if (ppa_dl_dre_vapinfo_set( peer, temp_vap_id, (unsigned int)pn_chk_type, temp_acc_dis ) == DTLK_SUCCESS)
			{
				/*
				* Peer may still store old information from previous connection.
				* Reset Peer information as well as PNs value.
				*/
				dlrx_ro_mainlist_t *dlrx_ro_mainlist_ptr;
				int seqid = 0;
				int tid = 0;
			    dlrx_ro_mainlist_ptr=(dlrx_ro_mainlist_t *)DLRX_DDR_RO_MAINLIST_BASE;
				dlrx_ro_mainlist_ptr += (peer*NUM_TID);	
				for(tid=0 ; tid < NUM_TID; tid ++ )
				{
					dlrx_ro_mainlist_ptr->last_pn_dw0 = 0;
					dlrx_ro_mainlist_ptr->last_pn_dw1 = 0;
					dlrx_ro_mainlist_ptr->last_pn_dw2 = 0;
					dlrx_ro_mainlist_ptr->last_pn_dw3 = 0;
					dlrx_ro_mainlist_ptr->first_ptr = NULL_PTR;
					for(seqid = 1; seqid < 64; seqid ++)
						dlrx_ro_mainlist_ptr->_dw_res0[seqid-1] = NULL_PTR;
					dlrx_ro_mainlist_ptr++;
				}
			}
            dtlk_print("SET PN CHECK: peer: %d, peer_id: %d, vap_id: %d, sec_type: %d, acc_dis:%d\n",
                peer, peer_id, temp_vap_id, pn_chk_type, temp_acc_dis);
		break;

        case PPA_WLAN_SET_PN_CHECK_WITH_RXPN:
            peer = get_handler_index((uint32_t)dlrx_peer_reg_handle );
            if( peer == HANDLER_NOT_FOUND )
            {
                return DTLK_FAILURE;
            }
            ppa_dl_dre_vapinfo_from_peer( peer, &temp_vap_id, &temp_sec_type, &temp_acc_dis );
            ppa_dl_dre_vapinfo_set( peer, temp_vap_id, (unsigned int)pn_chk_type, temp_acc_dis );
            dtlk_print("SET PN CHECK WITH RXPN: peer: %d, peer_id: %d, vap_id: %d, sec_type: %d, acc_dis:%d, rxpn: 0x%x\n",
                peer, peer_id, temp_vap_id, pn_chk_type, temp_acc_dis, (uint32_t)rxpn);
            
            dre_dl_set_rxpn_fn = dlrx_get_dre_fn(DRE_SET_RXPN_FN);
            if(likely(dre_dl_set_rxpn_fn)){
                dre_dl_set_rxpn_fn(peer, rxpn);
            }else{
                dtlk_err("Function set_rxpn is not registered!");
            }
        break;
	}
	
	return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_qca_set_peer_cfg);

int set_peer_id_to_peer_table(uint32_t dlrx_peer_reg_handle,uint32_t peer_id, uint32_t *peer, unsigned int vap_id, unsigned int pn_chk_type)
{
    unsigned int vld;
    unsigned int peer_val;
    unsigned int peer_index;
    unsigned int peer_offset;
    unsigned int mask_value;
    unsigned int peerinfo;
    unsigned int temp_peerinfo;
    int handler_index;

    volatile unsigned int *pid2p_tbl = DLRX_CFG_PEER_ID_TO_PEER_MAP_BASE(0);
    volatile dlrx_cfg_ctxt_peer_handler_t *peer_handler_tbl = NULL;
	
	
    handler_index = get_handler_index( dlrx_peer_reg_handle );
	//dtlk_debug(DTLK_DBG_ENABLE_MASK_DEBUG_PRINT,"%s: peer: %d, peer_id:%d\n", __func__, handler_index, peer_id);

    if( handler_index == HANDLER_NOT_FOUND )
    {
        if( get_free_peer_number( &peer_val ) == DTLK_FAILURE )
        {
            return DTLK_FAILURE;
        }

        //Get the handler index address by using the base address
        peer_handler_tbl=(dlrx_cfg_ctxt_peer_handler_t *)DLRX_CFG_CTXT_PEER_HANDLER_BASE( peer_val );
        //Send the handler table data 
        peer_handler_tbl->cfg_peer_count++;
        peer_handler_tbl->cfg_peer_handler = dlrx_peer_reg_handle;

  	}	
    else 
    {
        //Get the handler index address by using the base address
        peer_handler_tbl=(dlrx_cfg_ctxt_peer_handler_t *)DLRX_CFG_CTXT_PEER_HANDLER_BASE( handler_index ); 
        peer_handler_tbl->cfg_peer_count++;

        peer_val = ( unsigned int )handler_index;
    }

    ppa_dl_dre_vapinfo_set( peer_val, vap_id, pn_chk_type, 0 );

    vld = VALID;
    peerinfo = ( ( vld << 7 ) | ( peer_val & 0x7F) );

    peer_index = peer_id >> 2;
	peer_offset = peer_id % 4;

    mask_value = ~( 0xFF << ( peer_offset * 8 ) );

	spin_lock_bh(&g_prid2pr_tbl_lock);
    temp_peerinfo = *(pid2p_tbl + peer_index);
    temp_peerinfo &= mask_value;
    *(pid2p_tbl + peer_index) = ( temp_peerinfo | ( peerinfo << ( peer_offset << 3 ) ) );
	spin_unlock_bh(&g_prid2pr_tbl_lock);

    *peer = peer_val;
    return DTLK_SUCCESS;
}

int remove_peer_from_table(uint32_t dlrx_peer_reg_handle,uint32_t peer_id)
{
	int handler_index;
    unsigned int peer;
    unsigned int loop_index_1;
    unsigned int loop_index_2;
    unsigned int peer_id_loop_num;
    unsigned int temp_peerinfo;
    unsigned int peerinfo;
    unsigned int temp_peer;
    unsigned int temp_peer_vld;
    unsigned int mask_value;
    volatile dlrx_cfg_ctxt_peer_handler_t *peer_handler_tbl = NULL;
    volatile unsigned int *pid2p_tbl = DLRX_CFG_PEER_ID_TO_PEER_MAP_BASE(0);   
    dre_dl_reset_fn_t dre_dl_reset_fn;
    unsigned int *peer_bit_field;
    unsigned int peer_bit_field_offset;
    unsigned int peer_bit_field_index;
    peer_bit_field = (unsigned int *)DLRX_CFG_CTXT_PEER_BITMAP_BASE(0);
 //have 4 element

    handler_index = get_handler_index( dlrx_peer_reg_handle );

	if( handler_index == HANDLER_NOT_FOUND )
	{
		return DTLK_FAILURE;
	}
	else
	{
	    peer = ( unsigned int )handler_index;
	    //Get the handler index address by using the base address
        peer_handler_tbl=(dlrx_cfg_ctxt_peer_handler_t *)DLRX_CFG_CTXT_PEER_HANDLER_BASE( peer );
        //Send the handler table data 
        peer_handler_tbl->cfg_peer_handler = 0;

        peer_id_loop_num = MAX_PEERID_NUM >> 2;

        if(peer_handler_tbl->cfg_peer_count){
            spin_lock_bh(&g_prid2pr_tbl_lock);
            for( loop_index_1 = 0; loop_index_1 < peer_id_loop_num; loop_index_1++ )
            {
                
                temp_peerinfo = *(pid2p_tbl + loop_index_1);
                
                for( loop_index_2 = 0; loop_index_2 < 4; loop_index_2++ )
                {
                    mask_value = ( 0xFF << ( loop_index_2 << 3 ) );
                    peerinfo = ( temp_peerinfo & mask_value ) >> ( loop_index_2 << 3 );
                    temp_peer = peerinfo & 0x7F;
                    temp_peer_vld = ( peerinfo & 0x80 ) >> 7;
                    if( ( temp_peer_vld == 1 ) && ( temp_peer == peer ) )
                    {
                        peerinfo = 0;
                        mask_value = ~mask_value;
                        temp_peerinfo &= mask_value;
                        *(pid2p_tbl + loop_index_1) = temp_peerinfo;
    	            }
                }       
            }
            spin_unlock_bh(&g_prid2pr_tbl_lock);
        }
        peer_handler_tbl->cfg_peer_count = 0;

        dre_dl_reset_fn = dlrx_get_dre_fn(DRE_RESET_FN);
        if(likely(dre_dl_reset_fn)){
            dre_dl_reset_fn(DRE_RESET_PEER, peer);
        }else{
            dtlk_err("Function DRE_RESET_PEER is not registered!");
        }
       
        ppa_dl_dre_vapinfo_set( peer, 0, 0, 0 );

        // Set the corresponding peer bit field value to 0
        peer_bit_field_offset = peer >> 5;  // Divide by 32
        peer_bit_field_index = peer % 32;
        mask_value = ~( 1 << peer_bit_field_index ); // Calculate the mask value to set the corresponding peer bit to zero
        peer_bit_field[peer_bit_field_offset] &= mask_value;        
	}
	return DTLK_SUCCESS;
}


//Remove peer ID PEER table
int remove_peer_id_from_table( uint32_t dlrx_peer_reg_handle,uint32_t peer_id )
{
    unsigned int handler_index;
    unsigned int peer_index;
    unsigned int peer_offset;
    unsigned int peer;
    unsigned int temp_peerinfo;
    unsigned int mask_value;
    volatile dlrx_cfg_ctxt_peer_handler_t *peer_handler_tbl = NULL;
    volatile unsigned int *pid2p_tbl = DLRX_CFG_PEER_ID_TO_PEER_MAP_BASE(0); 

    handler_index = get_handler_index( dlrx_peer_reg_handle );

    if( handler_index == HANDLER_NOT_FOUND )
    {
        return DTLK_FAILURE;
    }
    else
    {
        peer_index = peer_id >> 2;
	    peer_offset = peer_id % 4;

        peer = ( unsigned int )handler_index;
	    //Get the handler index address by using the base address
        peer_handler_tbl=(dlrx_cfg_ctxt_peer_handler_t *)DLRX_CFG_CTXT_PEER_HANDLER_BASE( peer );
        //Send the handler table data 
        peer_handler_tbl->cfg_peer_count--;

        mask_value = ~( 0xFF << ( peer_offset * 8 ) );

        spin_lock_bh(&g_prid2pr_tbl_lock);
        temp_peerinfo = *(pid2p_tbl + peer_index);
        temp_peerinfo &= mask_value;
        *(pid2p_tbl + peer_index) = temp_peerinfo;
        spin_unlock_bh(&g_prid2pr_tbl_lock);
    }

	return DTLK_SUCCESS;
}

int get_handler_index( uint32_t dlrx_peer_reg_handle )
{
	int index;
	int handler_index = HANDLER_NOT_FOUND;


	volatile dlrx_cfg_ctxt_peer_handler_t *peer_handler_tbl = NULL;
	
	for( index=0; index < MAX_PEER_NUM; index++ )
	{
		peer_handler_tbl=(dlrx_cfg_ctxt_peer_handler_t *)DLRX_CFG_CTXT_PEER_HANDLER_BASE(index);
		if(peer_handler_tbl->cfg_peer_handler == dlrx_peer_reg_handle)
		{
			return index;
		}
	}
    return handler_index;	
}

int get_free_peer_number( unsigned int *peer_val )
{
    unsigned int index;
    unsigned int temp_bit_field;
    unsigned int free_peer_num = 0;
    unsigned int *peer_bit_field;

    //To store handler value and number of peer count for each handler
    //NOTE:MAXIMUM SUPPORTED HANDLER VALUE IS 128
    peer_bit_field = (unsigned int *)DLRX_CFG_CTXT_PEER_BITMAP_BASE(0); //have 4 element
  
    for( index = 0; index < 4; index ++ )
    {
        temp_bit_field = peer_bit_field[index];
        if( temp_bit_field == 0xFFFFFFFF )  // No free peer in this Dword
        {
            free_peer_num += 32;
            continue;
        }
        while( temp_bit_field & 1 )
        {
            temp_bit_field >>= 1;
            free_peer_num++;
        }
        break;
    }
    if(index >= 4)
    {
        return DTLK_FAILURE;
    }
    *peer_val = free_peer_num;
    peer_bit_field[index] = (peer_bit_field[index] | ( 1 << (*peer_val % 32) ) );
    return DTLK_SUCCESS;
}


/*
	This function reads Statistics of a given VAP. The function combines both DRE Rx and PPE FW Tx Stats counters.
*/
// FW_COMMENT: Disabling this function as PPA_WLAN_VAP_Stats_t is defined under #if 0 in Ltqmips_hal.h
#if 0
int32_t ppa_dl_qca_get_vap_stats (uint32_t vap_id,PPA_WLAN_VAP_Stats_t * vap_stats,uint32_t flags )
{
	
    volatile vap_data_mib_t *vap_mib = (vap_data_mib_t *)DLRX_DATA_MIB_BASE;//need to take base address from DLRX


    if( (vap_id == 0xF))
       	return DTLK_FAILURE;

	//We need to count the MESSAGE MIB
    vap_stats->tx_pkts  = vap_mib[vap_id].txpdu;
    vap_stats->tx_bytes = vap_mib[vap_id].txbytes;
    vap_stats->rx_pkts = vap_mib[vap_id].rx_rcv_pdu;
    vap_stats->rx_bytes = vap_mib[vap_id].rx_rcv_bytes;
    vap_stats->rx_disc_pkts =vap_mib[vap_id].rx_discard_pdu;
    vap_stats->rx_disc_bytes =vap_mib[vap_id].rx_discard_bytes;

    vap_stats->rx_fwd_pkts =vap_mib[vap_id].rx_fwd_pdu;
    vap_stats->rx_fwd_bytes =vap_mib[vap_id].rx_fwd_bytes;
    vap_stats->rx_insp_pkts =vap_mib[vap_id].rx_inspect_pdu;
    vap_stats->rx_insp_bytes =vap_mib[vap_id].rx_inspect_bytes;
    vap_stats->rx_pn_pkts=vap_mib[vap_id].rx_pn_pdu;
    vap_stats->rx_pn_bytes=vap_mib[vap_id].rx_pn_bytes;
    vap_stats->rx_drop_pkts = vap_mib[vap_id].rx_drop_pdu;
    vap_stats->rx_drop_bytes=vap_mib[vap_id].rx_drop_bytes;

    return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_qca_get_vap_stats);
#endif

//This function transfers control for DirectLink in CE-5 Rx in system. 
//The function gets called by QCA WLAN driver as part of handling legacy interrupt when CE-5 handling is made.
//Flags -- This is used for future use
int32_t ppa_hook_dl_qca_rx_offload (uint32_t flags)
{     
    int ret = DTLK_FAILURE;
    dre_dl_main_fn_t dre_dl_main_fn;
    dre_dl_main_fn = dlrx_get_dre_fn(DRE_MAIN_FN);
    if(likely(dre_dl_main_fn)){
        ret = dre_dl_main_fn();
    }else{
        dtlk_err("FW Callback FN: dre_main is not registered");
    }
    
	return (int32_t)ret;
}
EXPORT_SYMBOL(ppa_hook_dl_qca_rx_offload);

/*
This function picks a corresponding network packet buffer for previous handed over message in callback. The 
function gets called by QCA WLAN driver after its PPA_QCA_DL_RX_MSG_FN callback gets a message of type 
RX_IND or RX_FRAG_IND to it. This network buffer should be getting freed inside QCA driver or somewhere in 
the path of protocol stack. 
*/
int32_t ppa_dl_qca_get_rx_net_buf(struct sk_buff **rx_skb,uint32_t flags)
{	
    int ret = DTLK_SUCCESS;
    dre_dl_getmsdu_fn_t dre_dl_getmsdu_fn;
    unsigned int rx_pb, data_len;

    *rx_skb = NULL;
    dre_dl_getmsdu_fn = dlrx_get_dre_fn(DRE_GET_CURMSDU_FN);
    if(likely(dre_dl_getmsdu_fn)){
        ret = dre_dl_getmsdu_fn(&rx_pb, &data_len);
		
        if(ret != DTLK_SUCCESS){
            return (int32_t)ret;
        }
    }else{
        dtlk_err("FW CallBack FN: get_curmsdu is not registered!");
        return (int32_t)ret;
    }
	
	*rx_skb = dlrx_skb_setup(rx_pb,rx_pb,data_len+g_dlrx_cfg_offset_atten);
	
	return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_qca_get_rx_net_buf);

// TODO:  Need discuss the mib functions
int32_t ppa_dl_qca_get_msg_mdu_stats(PPA_DL_WLAN_MSG_STATS_t *msg_stats,PPA_DL_WLAN_RX_MPDU_MSDU_STATS_t *mdu_stats,uint32_t flags )
{
	volatile dlrx_data_mib_t *wlan_mdu_stats=(dlrx_data_mib_t *)DLRX_DATA_MIB_BASE;

	volatile dlrx_msg_mib_t *wlan_msg_stats=(dlrx_msg_mib_t *)DLRX_MSG_MIB_BASE;
	
	msg_stats->ce4_cpu_msgs=wlan_msg_stats->total_ce4_cpu_msg;
	msg_stats->ce5_cpu_msgs=wlan_msg_stats->total_ce5_cpu_msg;
	msg_stats->rx_ind_msgs=wlan_msg_stats->total_rx_ind_msg;
	msg_stats->rx_flush_msgs=wlan_msg_stats->total_rx_flush_msg;
	msg_stats->tx_comp_msgs=wlan_msg_stats->total_tx_cmp_msg;
	msg_stats->rx_ind_wl_msgs=wlan_msg_stats->total_rx_ind_wlan_msg;
	msg_stats->rx_flush_wl_msgs=wlan_msg_stats->total_rx_flush_wlan_msg;
	msg_stats->rx_frag_msgs=wlan_msg_stats->total_rx_frag_ind_msg;

	mdu_stats->rx_mpdu_ok=wlan_mdu_stats->rx_success_mpdu;
	mdu_stats->rx_msdu_ok=wlan_mdu_stats->rx_success_msdu;
	mdu_stats->rx_mpdu_err2=wlan_mdu_stats->rx_error2_mpdu;
	mdu_stats->rx_msdu_err2=wlan_mdu_stats->rx_error2_msdu;
	mdu_stats->rx_mpdu_err3=wlan_mdu_stats->rx_error3_mpdu;
	mdu_stats->rx_msdu_err3=wlan_mdu_stats->rx_error3_msdu;
	mdu_stats->rx_mpdu_err4=wlan_mdu_stats->rx_error4_mpdu;
	mdu_stats->rx_msdu_err4=wlan_mdu_stats->rx_error4_msdu;
	mdu_stats->rx_mpdu_err5=wlan_mdu_stats->rx_error5_mpdu;
	mdu_stats->rx_msdu_err5=wlan_mdu_stats->rx_error5_msdu;
	mdu_stats->rx_mpdu_err6=wlan_mdu_stats->rx_error6_mpdu;
	mdu_stats->rx_msdu_err6=wlan_mdu_stats->rx_error6_msdu;
	mdu_stats->rx_mpdu_err7=wlan_mdu_stats->rx_error7_mpdu;
	mdu_stats->rx_msdu_err7=wlan_mdu_stats->rx_error7_msdu;
	mdu_stats->rx_mpdu_err8=wlan_mdu_stats->rx_error8_mpdu;
	mdu_stats->rx_msdu_err8=wlan_mdu_stats->rx_error8_msdu;
	mdu_stats->rx_mpdu_err9=wlan_mdu_stats->rx_error9_mpdu;
	mdu_stats->rx_msdu_err9=wlan_mdu_stats->rx_error9_msdu;
	mdu_stats->rx_mpdu_errA=wlan_mdu_stats->rx_errora_mpdu;
	mdu_stats->rx_msdu_errA=wlan_mdu_stats->rx_errora_msdu;
	 
	return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_qca_get_msg_mdu_stats);

//Need to add one more parameter to clear particular VAP mibs
// FW_COMMENT: Disabling this function as PPA_WLAN_DL_STATS_Clear_t is not defined
// TODO: 
#if 0
uint32_t ppa_dl_qca_clear_stats(PPA_WLAN_DL_STATS_Clear_t clear_type,uint32_t flags )
{
	switch(clear_type)
	{
		case PPA_DL_WLAN_ALL_STATS_CLEAR: 
			g_dre_fnset.dre_dl_reset_fn(clear_type,RESET_ALLVAP);
		break;
		case PPA_DL_WLAN_PDU_STATS_CLEAR:
			//For this from FW side, Need to add option for clear DATA MIBS
			//Right now there is two options are available in FW
			// 1. Clear all MIBS 2. To clear particular MIBS
			//g_dre_fnset.dre_dl_reset_fn(DRE_DATA_MIB);
		break;
		case PPA_DL_WLAN_MSG_STATS_CLEAR:
			//The above said comments
			//g_dre_fnset.dre_dl_reset_fn(DRE_MSG_MIB);
		break;
		case PPA_DL_WLAN_VAP_STATS_CLEAR:
			//For this function we need to ask QCA to add one more parameter for VAP_ID
			// OR ask them to pass the VAP_ID in flags
			g_dre_fnset.dre_dl_reset_fn(DRE_VAP_MIB,flags);
		break;
		default:
			return DTLK_FAILURE;
		break;
	}
	return DTLK_SUCCESS;
}
EXPORT_SYMBOL(ppa_dl_qca_clear_stats);
#endif

int32_t ppa_dl_qca_set_seq_mask (uint32_t *dlrx_peer_reg_handle, uint32_t ex_tid, uint32_t seq_mask , uint32_t flags)
{
    int peer;
    uint32_t *seq_mask_base = DLRX_DDR_SEQ_MASK_BASE;
    
    if(unlikely(!dlrx_peer_reg_handle)){
        return DTLK_FAILURE;
    }

    peer = get_handler_index((uint32_t)dlrx_peer_reg_handle);
    if(unlikely(peer == HANDLER_NOT_FOUND)){
        return DTLK_FAILURE;
    }

    seq_mask_base[peer * 16 + ex_tid] = seq_mask;

    return DTLK_SUCCESS;
    
}
EXPORT_SYMBOL(ppa_dl_qca_set_seq_mask);


/************************************************* 
 *         Functions called by datapath driver
 *************************************************/
void set_vap_itf_tbl(uint32_t vap_id, uint32_t fid)
{
    
    volatile unsigned int *itf_tbl;
    volatile unsigned int itf_id;

    if(vap_id >= MAX_VAP_NUM)
        return;

    if(vap_id <= 7){//Range is defined in the spec
        itf_tbl = DLRX_CFG_VAP2INT_MAP1_BASE;
    }else{
        itf_tbl = DLRX_CFG_VAP2INT_MAP2_BASE;
    }

    vap_id = vap_id % 8;
    spin_lock_bh(&g_vap2int_tbl_lock);
    itf_id = *(itf_tbl);
    *(itf_tbl) = (itf_id & ~(0xF << (vap_id * 4))) | ((fid & 0xF) << (vap_id * 4));
    spin_unlock_bh(&g_vap2int_tbl_lock);

    return;
}


/************************************************* 
 *          Export Functions
 *************************************************/

