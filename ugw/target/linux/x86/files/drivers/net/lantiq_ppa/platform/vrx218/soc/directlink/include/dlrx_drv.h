#ifndef __DLRX_DRV_H__
#define __DLRX_DRV_H__

/****************************************************
 *  Extern Variable Declaration
 ****************************************************/
extern uint32_t g_dma_des_len;
extern uint32_t *g_dma_skb_buf;
extern uint32_t *g_dma_des_base;
extern uint32_t g_dtlk_dbg_enable;

/****************************************************
 *  Extern Variable Declaration
 ****************************************************/
extern struct sk_buff *alloc_skb_rx(void);
extern struct net_device *dtlk_dev_from_vapid(uint32_t);
extern void dtlk_rx_api_init(void);
extern void dtlk_rx_api_exit(void);
extern void set_vap_itf_tbl(uint32_t vap_id, uint32_t itf_id);

/****************************************************
 *  Macro Definition
 ****************************************************/
#define DTLK_PACKET_SIZE                         2048    
#define DTLK_ALIGNMENT                           32
#define DMA_CPU_OWNBIT                           0

#define DTLK_SUCCESS                             0
#define DTLK_FAILURE                             -1
#define DTLK_ERROR                               -2

#define HANDLER_FOUND							0
#define HANDLER_NOT_FOUND						-1
#define PEER_FIRST								1

#define DTLK_DBG                                 0
#define ENABLE_DTLK_DBG                                 1

#define SUPPORT_UNLOAD_DTLK 1
#define SUPPORT_11AC_MULTICARD 1



#if defined(DTLK_DBG) && DTLK_DBG
#define dtlk_print(fmt, arg...)      do { printk(KERN_WARNING  fmt, ##arg); } while (0)
#define dtlk_err(fmt, arg...)        do { printk(KERN_ERR     "%s:%d:%s: " fmt "\n", __FILE__,__LINE__, __FUNCTION__, ##arg); } while (0)
#define ASSERT(cond, fmt, arg...)    do { if (!(cond) ) printk(KERN_ERR  ":%d:%s: " fmt "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
#define dtlk_print(fmt, arg...)      
#define dtlk_err(fmt, arg...)        do { printk(KERN_ERR     "%s:%d:%s: " fmt "\n",__FILE__, __LINE__, __FUNCTION__, ##arg); } while (0)
#define ASSERT(cond, fmt, arg...)    do { if (!(cond) ) printk(KERN_ERR  "%s:%d:%s: " fmt "\n", __FILE__,__LINE__, __FUNCTION__, ##arg); } while ( 0 )
#endif

/*
 *  Debug Print Mask
 *  Note, once you add a new DBG_ macro, don't forget to add it in DBG_ENABLE_MASK_ALL also !!!!
 */
#define DTLK_DBG_ENABLE_MASK_ERR                     (1 << 0)
#define DTLK_DBG_ENABLE_MASK_ASSERT                  (1 << 1)
#define DTLK_DBG_ENABLE_MASK_DEBUG_PRINT             (1 << 2)

#define DTLK_DBG_ENABLE_MASK_ALL                     (DTLK_DBG_ENABLE_MASK_ERR | DTLK_DBG_ENABLE_MASK_ASSERT)


#if defined(ENABLE_DTLK_DBG) && ENABLE_DTLK_DBG        
    #undef dtlk_debug    
    //#define ppa_debug(flag, fmt, arg...)  do { if ( (g_ppa_dbg_enable & flag) && max_print_num ) { ppa_printk(/*__FILE__*/ ":%d:%s: " fmt, __LINE__, __FUNCTION__, ##arg); if(max_print_num) max_print_num--; } } while ( 0 )
    #define dtlk_debug(flag, fmt, arg...)  do { if (g_dtlk_dbg_enable & flag) { printk(KERN_DEBUG fmt, ##arg); } }while ( 0 );
#else
    #undef dtlk_debug
    #define dtlk_debug(flag, fmt, arg...) 
#endif




#endif /* __DLRX_DRV_H__ */
