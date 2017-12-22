#ifndef __XRX300_A5_COMMON_H__
#define __XRX300_A5_COMMON_H__



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <net/ppa_stack_al.h>
//#include "../../ppa_datapath.h"



/*
 * ####################################
 *           Compilation Flags
 * ####################################
 */
/*
extern int g_dbg_enable;

#define DBG_ENABLE_MASK_ERR                     (1 << 0)
#define DBG_ENABLE_MASK_DEBUG_PRINT             (1 << 1)
#define DBG_ENABLE_MASK_ASSERT                  (1 << 2)
#define DBG_ENABLE_MASK_DUMP_SKB_RX             (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX             (1 << 9)
#define DBG_ENABLE_MASK_DUMP_FLAG_HEADER        (1 << 10)
#define DBG_ENABLE_MASK_DUMP_INIT               (1 << 11)
#define DBG_ENABLE_MASK_DUMP_QOS                (1 << 12)
#define DBG_ENABLE_MASK_MAC_SWAP                (1 << 16)
#define DBG_ENABLE_MASK_ALL                     (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT \
                                                | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX                 \
                                                | DBG_ENABLE_MASK_DUMP_FLAG_HEADER | DBG_ENABLE_MASK_DUMP_INIT              \
                                                | DBG_ENABLE_MASK_DUMP_QOS | DBG_ENABLE_MASK_MAC_SWAP)

#define err(format, arg...)                     do { if ( (g_dbg_enable & DBG_ENABLE_MASK_ERR) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
*/
#if defined(CONFIG_DSL_MEI_CPE_DRV) && !defined(CONFIG_IFXMIPS_DSL_CPE_MEI)
  #define CONFIG_IFXMIPS_DSL_CPE_MEI            1
#endif

#ifndef NUM_ENTITY
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))
#endif

/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
struct ltq_mei_atm_showtime_info { 
    void *check_ptr; 
    void *enter_ptr; 
    void *exit_ptr; 
};
*/ 

extern int atm_callback_set(e_ltq_mei_cb_type type, void *func);
extern void* atm_callback_get(e_ltq_mei_cb_type type);

#endif  //  __XRX300_A5_COMMON_H__
