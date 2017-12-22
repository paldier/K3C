#ifndef __PPA_API_MISC_H__20081103_1952__
#define __PPA_API_MISC_H__20081103_1952__



/*******************************************************************************
**
** FILE NAME    : ppa_api_misc.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Miscellaneous Functions Header
**                File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 03 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/

/*! \file ppa_api_misc.h
    \brief This file contains es.
           provide PPA API.
*/

/** \addtogroup PPA_MISC_API PPA Miscellaneous API
    \brief  PPA Miscellaneous API provide PPA Miscellaneous api and IOCTL API
            The API is defined in the following two source files
            - ppa_api_misc.h: Header file for PPA API
            - ppa_api_misc.c: C Implementation file for PPA API
*/
/* @{ */


/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  Compilation Switch
 */

#define ENABLE_DEBUG                            1

#define ENABLE_ASSERT                           1

#define ENABLE_ERROR                            1

#define ENABLE_SESSION_DEBUG_FLAGS              1

#define DEBUG_DUMP_LIST_ITEM                    1

#define ENABLE_DYNAMIC_NETIF_DETECTION          0

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG
  #define ENABLE_DEBUG_PRINT                    1
  #define DISABLE_INLINE                        1
#endif

#if defined(DISABLE_INLINE) && DISABLE_INLINE
  #define INLINE
#else
  #define INLINE                                inline
#endif


#if defined(ENABLE_ASSERT) && ENABLE_ASSERT
  #undef  ASSERT
  #define ASSERT(cond, format, arg...)          do { if ( (g_ppa_dbg_enable & DBG_ENABLE_MASK_ASSERT) && !(cond) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define ASSERT(cond, format, arg...)
#endif

#if defined(ENABLE_ERROR) && ENABLE_ERROR
  #undef  err
  #define err(format, arg...)                   do { if ( (g_ppa_dbg_enable & DBG_ENABLE_MASK_ERR) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define err(format, arg...)
#endif


#if defined(ENABLE_ERROR) && ENABLE_ERROR
  #undef  critial_err
  #define critial_err(format, arg...)           do { printk(KERN_EMERG __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define critial_err(format, arg...)
#endif


/*
 *  Debug Print Mask
 *  Note, once you add a new DBG_ macro, don't forget to add it in DBG_ENABLE_MASK_ALL also !!!!
 */
#define DBG_ENABLE_MASK_ERR                     (1 << 0)
#define DBG_ENABLE_MASK_ASSERT                  (1 << 1)
#define DBG_ENABLE_MASK_DEBUG_PRINT             (1 << 2)
#define DBG_ENABLE_MASK_DUMP_ROUTING_SESSION    (1 << 4)
#define DBG_ENABLE_MASK_DUMP_MC_GROUP           (1 << 5)
#define DBG_ENABLE_MASK_DUMP_BRIDGING_SESSION   (1 << 6)
#define DBG_ENABLE_MASK_DUMP_SKB_RX             (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX             (1 << 9)
#define DBG_ENABLE_MASK_DUMP_FLAG_HEADER        (1 << 10)
#define DBG_ENABLE_MASK_DUMP_INIT               (1 << 11)
#define DBG_ENABLE_MASK_QOS                     (1 << 12)
#define DBG_ENABLE_MASK_PWM                     (1 << 13)
#define DBG_ENABLE_MASK_MFE                     (1 << 14)
#define DBG_ENABLE_MASK_PRI_TEST                (1 << 15)
#define DBG_ENABLE_MASK_SESSION                 (1 << 16)
#define DBG_ENABLE_MASK_DEBUG2_PRINT            (1 << 17)
#define DBG_ENABLE_MASK_MARK_TEST               (1 << 18)
#define DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP       (1 << 19)

#define DBG_ENABLE_MASK_ALL                     (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT \
                                                | DBG_ENABLE_MASK_DUMP_ROUTING_SESSION | DBG_ENABLE_MASK_DUMP_MC_GROUP      \
                                                | DBG_ENABLE_MASK_DUMP_BRIDGING_SESSION                                     \
                                                | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX                 \
                                                | DBG_ENABLE_MASK_DUMP_FLAG_HEADER | DBG_ENABLE_MASK_DUMP_INIT              \
                                                | DBG_ENABLE_MASK_QOS | DBG_ENABLE_MASK_PWM | DBG_ENABLE_MASK_MFE           \
												| DBG_ENABLE_MASK_PRI_TEST | DBG_ENABLE_MASK_SESSION | DBG_ENABLE_MASK_DEBUG2_PRINT)

/*
 *  helper macro
 */
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))



/*ppa_debug currently supported flags:
        DBG_ENABLE_MASK_ERR,
        DBG_ENABLE_MASK_DEBUG_PRINT,
        DBG_ENABLE_MASK_ASSERT,
        DBG_ENABLE_MASK_DUMP_ROUTING_SESSION,
        DBG_ENABLE_MASK_DUMP_MC_GROUP,
        DBG_ENABLE_MASK_DUMP_BRIDGING_SESSION,
        DBG_ENABLE_MASK_DUMP_INIT,
        DBG_ENABLE_MASK_QOS,
        DBG_ENABLE_MASK_PWM,
		DBG_ENABLE_MASK_SESSION,
        DBG_ENABLE_MASK_ALL*/
#if defined(ENABLE_DEBUG_PRINT) && ENABLE_DEBUG_PRINT        
    #undef ppa_debug    
    //#define ppa_debug(flag, fmt, arg...)  do { if ( (g_ppa_dbg_enable & flag) && max_print_num ) { ppa_printk(/*__FILE__*/ ":%d:%s: " fmt, __LINE__, __FUNCTION__, ##arg); if(max_print_num) max_print_num--; } } while ( 0 )
    #define ppa_debug(flag, fmt, arg...)  do { if ( (g_ppa_dbg_enable & flag) && max_print_num ) { printk(KERN_ERR fmt, ##arg); if( max_print_num != ~0) max_print_num--; } } while ( 0 )
#else
    #undef ppa_debug
    #define ppa_debug(flag, fmt, arg...) 
#endif


/*
 * ####################################
 *              Data Type
 * ####################################
 */



/*
 * ####################################
 *             Declaration
 * ####################################
 */

extern uint32_t g_ppa_dbg_enable;
extern uint32_t max_print_num;

/* @} */


#endif  //  __PPA_API_MISC_H__20081103_1952__
