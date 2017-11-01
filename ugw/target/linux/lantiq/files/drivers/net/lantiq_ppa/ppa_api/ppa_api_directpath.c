/*******************************************************************************
**
** FILE NAME    : ppa_api_directpath.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 19 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Directpath Functions
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
** 19 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
//#include <linux/autoconf.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif


//#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/version.h>
//#include <linux/types.h>
//#include <linux/init.h>
//#include <linux/slab.h>
//#if defined(CONFIG_LTQ_PPA_API_PROC)
//#include <linux/proc_fs.h>
//#endif
//#include <linux/netdevice.h>
//#include <linux/in.h>
//#include <net/sock.h>
//#include <net/ip_vs.h>
//#include <asm/time.h>

/*
 *  PPA Specific Head File
 */
#include <net/ppa_api.h>
#include <net/ppa_api_directpath.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_misc.h"
#include "ppa_api_tools.h"
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#include <net/lantiq_cbm_api.h>
#endif
#include <net/ltq_mpe_hal.h>
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
#include "ppa_api_cpu_freq.h"
#endif
#else
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif
#endif
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
#include "lpdp_api.h"
#endif


/*
 * ####################################
 *              Definition
 * ####################################
 */

#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
/* This API are according to new lpdp structure */
	#define MAX_SUBIF       MAX_VPID
	#define DRV_REGISTER    ppa_drv_lpdp_directpath_register
	#define DRV_SEND        ppa_drv_lpdp_directpath_send
	#define DRV_RX_STOP     ppa_drv_lpdp_directpath_flowctrl
	#define DRV_RX_START    ppa_drv_lpdp_directpath_flowctrl
	#define DRV_ALLOC_SKB   ppa_drv_lpdp_directpath_alloc_skb
	#define DRV_RECYCLE_SKB ppa_drv_lpdp_directpath_recycle_skb
#else
	//#define MAX_SUBIF       16
	#define DRV_REGISTER    ppa_drv_directpath_register
	#define DRV_SEND        ppa_drv_directpath_send
	#define DRV_RX_STOP     ppa_drv_directpath_rx_stop
	#define DRV_RX_START    ppa_drv_directpath_rx_start
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

static void update_itf_info(void);



/*
 * ####################################
 *           Global Variable
 * ####################################
 */

static PPA_LOCK g_directpath_port_lock;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static uint32_t g_start_ifid = 0, g_end_ifid = 0;
#elif defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
static uint32_t g_start_ifid = 0, g_end_ifid = MAX_PID * MAX_VPID;
#else
static uint32_t g_start_ifid = ~0, g_end_ifid = ~0;
#endif

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
uint16_t 		g_ewma_num_adds=1;
uint64_t		g_ewma_bytes=0;    // EWMA in bytes 
uint64_t		g_ewma_time=0;    // EWMA in time 
uint64_t 		g_ewma_session_bytes=0;
uint64_t 		g_ppa_dp_thresh = (1500 * 2);
uint64_t 		g_ppa_dp_window = 2; //window is initialized to 2 sec
PPA_TIMESPEC		g_timespent = {0};// Time spent in EWMA window
#endif
#endif

/*
 * ####################################
 *           Extern Variable
 * ####################################
 */



/*
 * ####################################
 *            Local Function
 * ####################################
 */
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
uint32_t ppa_dp_coc_reset_ewma_stats( uint32_t flags)
{
        g_ewma_session_bytes = 0;
        g_ewma_num_adds = 1;
        return PPA_SUCCESS;
}

uint32_t ppa_dp_coc_store_ewma( PPA_BUF *ppa_buf, uint32_t flags)
{
	g_ewma_session_bytes   += ppa_buf->len + PPA_ETH_HLEN + PPA_ETH_CRCLEN;
	g_ewma_num_adds ++;
        return PPA_SUCCESS;
}

uint32_t  ppa_dp_coc_record_time(PPA_BUF *ppa_buf,uint32_t flags)
{
        PPA_TIMESPEC before;
        if ( g_ewma_num_adds == 1 ) { // checking for one cause for each session for first packet ppa_session_store_ewma() will get called 
                                                                                        //comes to this point and there we are increamenting count of ewma_num_adds. 
        	ppa_get_monotonic(&before);
                g_timespent.tv_nsec = before.tv_nsec;
                g_timespent.tv_sec = before.tv_sec;
        }
        g_ewma_num_adds++;
        g_ewma_session_bytes += ppa_buf->len + PPA_ETH_HLEN + PPA_ETH_CRCLEN;
        return PPA_SUCCESS;
}


uint32_t ppa_dp_coc_pass_criteria(PPA_SUBIF *subif, PPA_BUF *skb, uint32_t flags)
{
	uint64_t time_msec =0;
	uint64_t total_bytes =0;
	PPA_TIMESPEC wait_time;
	uint64_t wait_time_const;
	PPA_TIMESPEC after;
	PPA_TIMESPEC sleep_time;
	uint64_t new_wait_time_const;
	//wait_time.tv_sec = nf_conntrack_sessionmgmt_add_time; // This duration is required to correctly arrive data rate of session
	wait_time.tv_sec = g_ppa_dp_window; // This duration is required to correctly arrive data rate of session
	wait_time.tv_nsec =0;

	ppa_get_monotonic(&after);
	sleep_time = ppa_timespec_sub(after, g_timespent);
	time_msec = (uint64_t ) ppa_timespec_to_ns(&sleep_time);
	wait_time_const = (uint64_t) ppa_timespec_to_ns(&wait_time);
	// Convert bytes to bits = Multiply by 8 = Left shift by 3
	total_bytes = (uint64_t) (g_ewma_session_bytes << 3) * NSEC_PER_SEC;

	// EWMA calculation: We give 1/4 to current value and 3/4 to previous value. Such ratios allow shift operations
	if ( g_ewma_bytes == 0 ) {
		g_ewma_bytes = total_bytes;
		g_ewma_time = time_msec;
	} else {
		g_ewma_bytes = ( total_bytes + (g_ewma_bytes * 3)) >> 2;
		g_ewma_time = ( time_msec + (g_ewma_time * 3)) >> 2;
	}

	total_bytes = g_ewma_bytes;
	time_msec = g_ewma_time;

	new_wait_time_const =  wait_time_const;
	if( time_msec >  new_wait_time_const ) { // For each Min time interval check data rate of session

		//printk(" --> Before Session data rate=%llu \n",total_bytes);
		total_bytes = div64_u64( total_bytes,new_wait_time_const );           
		//printk("total_bytes: %llu\n",total_bytes);
		//printk("--> After Session data rate=%llu ewma bytes=%llu time=%llu \n",total_bytes, g_ewma_bytes, time_msec);
		//printk("--> Session data rate=%llu threshold=%llu  \n",total_bytes, g_ppa_dp_thresh);

		ppa_dp_coc_reset_ewma_stats(0);
		// Data rate is EWMA of bytes divided by EWMA of time

		// If total bytes does not meet its threshold data rate return failure.
		if ( total_bytes <=  g_ppa_dp_thresh  )
			return PPA_FAILURE;

	}
	else
	{
		return PPA_FAILURE;
	}
	//printk("--> exit <--\n");
	return PPA_SUCCESS;

}

uint32_t ppa_dp_ewma_coc(PPA_SUBIF *subif, PPA_BUF *skb, uint32_t flags)
{
	if(ppa_dp_coc_pass_criteria(subif, skb, flags) == PPA_SUCCESS)
	{
		//printk(" ----------------->  Requesting CoC for upscale !!! ----------> \n");
		ppa_api_cpufreq_activate_module();
	}
	return PPA_SUCCESS;
}
#endif
#endif

static void update_itf_info(void)
{
    uint32_t i;
    PPE_IFINFO if_info;
    uint32_t tmp_flags;

    ppa_memset(&if_info, 0, sizeof(if_info));

    for ( i = g_start_ifid; i < g_end_ifid; i++ )
    {
        if_info.port = i;
        if ( ppa_drv_get_phys_port_info(&if_info, 0) == PPA_SUCCESS
            && (if_info.if_flags & (PPA_PHYS_PORT_FLAGS_VALID | PPA_PHYS_PORT_FLAGS_TYPE_MASK)) == (PPA_PHYS_PORT_FLAGS_VALID | PPA_PHYS_PORT_FLAGS_TYPE_EXT)
            && (if_info.if_flags & (PPA_PHYS_PORT_FLAGS_EXT_CPU0 | PPA_PHYS_PORT_FLAGS_EXT_CPU1)) != 0 )
        {
            tmp_flags = ppa_drv_g_ppe_directpath_data[i - g_start_ifid].flags & ~PPE_DIRECTPATH_LANWAN_MASK;
            if ( (if_info.if_flags & PPA_PHYS_PORT_FLAGS_MODE_LAN) )
                tmp_flags |= PPE_DIRECTPATH_LAN;
            if ( (if_info.if_flags & PPA_PHYS_PORT_FLAGS_MODE_WAN) )
                tmp_flags |= PPE_DIRECTPATH_WAN;
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "%s: if_info.if_flags = 0x%08x, direct_flag[%d]=0x%08x, tmp_flags=0x%08x\n", __FUNCTION__, if_info.if_flags, i - g_start_ifid, ppa_drv_g_ppe_directpath_data[i - g_start_ifid].flags, tmp_flags);
            ppa_drv_g_ppe_directpath_data[i - g_start_ifid].flags = tmp_flags;
        }
    }
}

static PPA_BUF *__remove_directpath_dev_from_datapath(int if_id)
{
    //unsigned long sys_flag;
    uint32_t tmp_flags;
    
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
    PPA_BUF *skb_list = NULL;
#endif
    
    if ( (ppa_drv_g_ppe_directpath_data[if_id].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
    {
        ppa_phys_port_remove(if_id + g_start_ifid);
        //sys_flag = ppa_disable_int();
        tmp_flags = ppa_drv_g_ppe_directpath_data[if_id].flags & PPE_DIRECTPATH_MASK;
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
        ppe_lock_get(&ppa_drv_g_ppe_directpath_data[if_id].txq_lock);
        skb_list = ppa_drv_g_ppe_directpath_data[if_id].skb_list;
        ppa_drv_g_ppe_directpath_data[if_id].skb_list = NULL;
        ppe_lock_release(&ppa_drv_g_ppe_directpath_data[if_id].txq_lock);
#endif  
        ppa_memset(&ppa_drv_g_ppe_directpath_data[if_id], 0, sizeof(ppa_drv_g_ppe_directpath_data[if_id]));
        ppa_drv_g_ppe_directpath_data[if_id].flags = tmp_flags;
        //ppa_enable_int(sys_flag);
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"directp unregiter ok and restore direct_flag[%d]=%x\n", if_id, ppa_drv_g_ppe_directpath_data[if_id].flags );
    }

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
    return skb_list;
#endif
    return NULL;
}

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
static int remove_directpath_queue(PPA_BUF *skb_list)
{
    PPA_BUF *skb, *skb_list_bak;

    if ( skb_list )
    {
      skb_list_bak = skb_list;
      do
      {
          skb = skb_list;
          skb_list = ppa_buf_get_next(skb_list);
          ppa_buf_free(skb);
      } while ( skb_list != NULL && skb_list != skb_list_bak );
    }
    return 0;
}
#endif

/*
 * ####################################
 *           Global Function
 * ####################################
 */

/*
 *  PPA DirectPath Functions
 */
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
#ifdef CONFIG_LTQ_PPA_GRX500
int32_t g_DL_Init=0;
int (*datapath_dtlk_register_fn)(PPA_SUBIF *subIf, PPA_DTLK_T *dtlk) = NULL;
EXPORT_SYMBOL(datapath_dtlk_register_fn);
extern  int (*mpe_hal_feature_start_fn)(
                enum MPE_Feature_Type mpeFeature,
                uint32_t port_id,
                uint32_t * featureCfgBase,
                uint32_t flags);
//EXPORT_SYMBOL(mpe_hal_feature_start_fn);
int32_t ppa_directpath_register_dev_ex(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);
int32_t ppa_directlink_register_dev(
		PPA_SUBIF *subIf,
		PPA_DTLK_T *dtlk,
		PPA_DIRECTPATH_CB *pDirectpathCb,
		uint32_t flags
		)
{
	int32_t ret = PPA_SUCCESS;
	uint32_t if_id = ~0;
	dp_cb_t 	cb;
	int port_id;
	int f_wifi_register = 0;

	if (!ppa_is_init())
		return PPA_EINVAL;

	/* sanity check */
	if (!subIf || !dtlk || !dtlk->dev)
		return PPA_EINVAL;

	/* Register to DirectLink */
	if (flags & PPA_F_DIRECTPATH_REGISTER) {
		/* register directlink */
		pr_err("%s: register directlink subIf->port_id[%d]\n", __func__, subIf->port_id);
		/* Get DirectPath ID, the flag need is subject to change */
		port_id = -1;
		if (subIf->port_id == -1) {
			/* register wifi device */
			f_wifi_register = 1;
			if (ppa_directpath_ex_register_dev(subIf, //&port_id,
						dtlk->dev,
						pDirectpathCb,
						PPA_F_DIRECTPATH_ETH_IF | PPA_F_DIRECTPATH_REGISTER | DP_F_DIRECTLINK)
					!= PPA_SUCCESS) {
				pr_err("%s: register directpath fail!\n",
						__func__
				      );
				return PPA_ENOMEM;
			}
			//subIf->port_id = port_id;
			subIf->subif = 0;
			pr_err("%s: register wifi[%d][%d]\n", __func__,subIf->port_id, g_DL_Init);
			if(mpe_hal_feature_start_fn) {
				if(g_DL_Init == 0) {
					if(mpe_hal_feature_start_fn(DL_TX_1, subIf->port_id, NULL, F_FEATURE_START) != PPA_SUCCESS) {
						printk("%s: Feature start failed !\n");
						goto reg_err1;
					}
					g_DL_Init++;
				}
			}
		} else {
			pr_err("%s: register ath subif[%d] dtlk->dev[0x%x]\n", __func__,subIf->subif, dtlk->dev);
			if (subIf->port_id == 0){
				pr_err("%s: register port id[%d] use default 7\n", __func__,subIf->port_id);
				subIf->port_id = 7;
			}
			subIf->subif = -1;
			/* register VAP */
			if (ppa_directpath_ex_register_dev(subIf,
						dtlk->dev,
						pDirectpathCb,
						(flags | DP_F_DIRECTLINK)) != PPA_SUCCESS)
			{
				pr_err("%s: register VAP fail!\n",
						__func__
				      );
				return PPA_ENOMEM;
			}
			pr_err("%s: register vap port_id[%d] sub_if[%d]\n", __func__,
					subIf->port_id,
					subIf->subif
			      );
		}
		/*
		 * only register to DirectLink if this is VAP registration
		 * register to DirectLink */
		if (!f_wifi_register) {
			if (datapath_dtlk_register_fn) {
				if (datapath_dtlk_register_fn(subIf, dtlk) != PPA_SUCCESS) {
					pr_err("Failed to register to DirectLink\n");
					goto reg_err1;
				}
			}
		}

		return ret;
	} else {
		printk("%s: port %d  1 subif %d\n", __func__, subIf->port_id, subIf->subif);
		g_DL_Init--;
		if (g_DL_Init < 0)
			g_DL_Init = 0;
		printk("%s: Fix deregister for QCA %d\n", __func__, g_DL_Init);
		if (subIf->subif != -1) {
			/* unregister wifi device */
			//if (ppa_directpath_register_dev(&port_id,
			if (ppa_directpath_ex_register_dev(subIf,
						dtlk->dev,
						pDirectpathCb,
						(flags | DP_F_DIRECTLINK))
					!= PPA_SUCCESS) {
				pr_err("%s: deregister directpath failed!\n",
						__func__
				      );
				return PPA_ENOMEM;
			}
			if(mpe_hal_feature_start_fn) {
				if(g_DL_Init == 0) {
					if(mpe_hal_feature_start_fn(DL_TX_1, subIf->port_id, NULL, F_FEATURE_STOP) != PPA_SUCCESS) {
						printk("%s: Feature stop failed !\n");
						goto reg_err1;
					}
				}
			}
		} else {
			f_wifi_register = 1;
			if (ppa_directpath_ex_register_dev(subIf,
						dtlk->dev,
						pDirectpathCb,
						(flags | DP_F_DIRECTLINK)) != PPA_SUCCESS) {
				pr_err("%s: deregister VAP failed!\n",
						__func__
				      );

				return PPA_ENOMEM;
			}
			if(mpe_hal_feature_start_fn) {
				if(g_DL_Init == 0) {
					if(mpe_hal_feature_start_fn(DL_TX_1, subIf->port_id, NULL, F_FEATURE_STOP) != PPA_SUCCESS) {
						printk("%s: Feature stop failed !\n");
						goto reg_err1;
					}
				}
			}
		}
		/*
		 * only deregister to DirectLink if this is VAP */
		if (!f_wifi_register) {
			if (datapath_dtlk_register_fn) {
				if (datapath_dtlk_register_fn(subIf, dtlk) != PPA_SUCCESS) {
					pr_err("Failed to deregister to DirectLink\n");
					return PPA_ENOMEM;
				}
			}
		}
	}
	return ret;
reg_err1:
	if (ppa_directpath_register_dev_ex(&subIf,
				dtlk->dev,
				pDirectpathCb,
				PPA_F_DIRECTPATH_ETH_IF) != PPA_SUCCESS) {
		pr_err("%s: deregister fail!\n",
				__func__
		      );
		return PPA_ENOMEM;
	}
	return PPA_EFAULT;
}
#else
int32_t ppa_directlink_register_dev(int32_t *p_if_id, PPA_DTLK_T *dtlk, PPA_DIRECTPATH_CB *pDirectpathCb,uint32_t flags)
{
   int32_t ret = PPA_SUCCESS;
   uint32_t if_id = ~0;

   if(!ppa_is_init()) 
      return PPA_EINVAL;
   
   if(!p_if_id || !dtlk || !dtlk->dev)
      return PPA_EINVAL;

   if(dtlk->flags & PPE_F_DTLK_REGISTER){//register directlink
       printk("%s: register directlink\n", __func__); 
      if((ret = datapath_dtlk_register(p_if_id, dtlk)) != PPA_SUCCESS){
		return ret;
	}
   }
   
   if(dtlk->flags & PPE_F_DTLK_DP_REGISTER){//register to directpath
       if((ret=ppa_directpath_register_dev(&if_id,dtlk->dev,pDirectpathCb,flags)) == PPA_SUCCESS){
            printk("%s: register directpath\n", __func__); 
            if((ret=datapath_dtlk_update(*p_if_id,dtlk, &if_id)) != PPA_SUCCESS){
		return ret;
		} 
       }else{
		return ret;
       }
   }else if(dtlk->flags & PPE_F_DTLK_DP_DEREGISTER){//deregister from directpath
        printk("%s: de-register directpath\n", __func__); 
        datapath_dtlk_update(*p_if_id,dtlk, &if_id);
        if((ret=ppa_directpath_register_dev(&if_id,dtlk->dev,pDirectpathCb,flags)) != PPA_SUCCESS){
		return ret;
	}
   }

   if(dtlk->flags & PPE_F_DTLK_DEREGISTER){//deregister from directlink
        printk("%s: de-register directlink\n", __func__); 
        ret = datapath_dtlk_register(p_if_id, dtlk);
   }
   
   return ret;      
}
#endif
#endif

/* =====================  */

int32_t ppa_directpath_register_dev_legacy(uint32_t *p_if_id, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags)
{
    int32_t ret;
    uint32_t if_id;
    uint32_t tmp_flags;
    PPA_BUF *skb_list = NULL;

    if( !ppa_is_init() ) return PPA_EINVAL;

    if ( p_if_id == NULL || !ppa_drv_g_ppe_directpath_data  )
        return PPA_EINVAL;

    if ( (flags & PPA_F_DIRECTPATH_REGISTER) )
    {
        if ( !(flags & PPA_F_DIRECTPATH_ETH_IF) )
            //  currently, only ethernet interface is supported
            return PPA_EINVAL;

        if ( !netif || !pDirectpathCb
            || !pDirectpathCb->rx_fn )
            return PPA_EINVAL;

        if ( ppa_get_netif_name(netif) == NULL )
            return PPA_EINVAL;

        tmp_flags = (flags & PPA_F_DIRECTPATH_WAN) ? PPE_DIRECTPATH_WAN : PPE_DIRECTPATH_LAN;
        if ( (flags & PPA_F_DIRECTPATH_CORE1) )
            tmp_flags |= PPE_DIRECTPATH_CORE1;
        else
            tmp_flags |= PPE_DIRECTPATH_CORE0;
        if ( (flags & PPA_F_DIRECTPATH_ETH_IF) )
            tmp_flags |= PPE_DIRECTPATH_ETH;

        //  update flags which could be changed on the fly, e.g. AR9/VR9/AR10 - LAN/WAN could be changed
        update_itf_info();

        ppa_lock_get(&g_directpath_port_lock);

        //first check whether the interface already added into PPA directpath or not
        for ( if_id = 0; if_id < g_end_ifid; if_id ++ )
        {
            if( ppa_drv_g_ppe_directpath_data[if_id].netif && ( ppa_drv_g_ppe_directpath_data[if_id].netif == netif ) )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "The interface already in PPA directpath already. Its netif pointer is :%p\n", netif);
                if(p_if_id ) *p_if_id = ppa_drv_g_ppe_directpath_data[if_id].ifid ;
                ppa_lock_release(&g_directpath_port_lock);
                return PPA_SUCCESS;
            }
        }

        for ( if_id = 0;
              if_id < g_end_ifid - g_start_ifid && (ppa_drv_g_ppe_directpath_data[if_id].flags & (PPE_DIRECTPATH_DATA_ENTRY_VALID | PPE_DIRECTPATH_ITF_TYPE_MASK | tmp_flags)) != tmp_flags;
              if_id++ )
        {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "%s: direct_flag[%d]=0x%08x, tmp_flags=0x%08x\n", __FUNCTION__, if_id, ppa_drv_g_ppe_directpath_data[if_id].flags, tmp_flags);
        }
        if ( if_id < g_end_ifid - g_start_ifid
            && ppa_phys_port_add(ppa_get_netif_name(netif), if_id + g_start_ifid) == PPA_SUCCESS )
        {
            tmp_flags = (ppa_drv_g_ppe_directpath_data[if_id].flags & PPE_DIRECTPATH_MASK) | PPE_DIRECTPATH_DATA_ENTRY_VALID | PPE_DIRECTPATH_DATA_RX_ENABLE;
            ppa_memset(&ppa_drv_g_ppe_directpath_data[if_id], 0, sizeof(ppa_drv_g_ppe_directpath_data[if_id]));
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
            ppe_lock_init(&ppa_drv_g_ppe_directpath_data[if_id].txq_lock);
#endif
            ppa_drv_g_ppe_directpath_data[if_id].callback   = *pDirectpathCb;
            ppa_drv_g_ppe_directpath_data[if_id].netif      = netif;
            ppa_drv_g_ppe_directpath_data[if_id].ifid       = if_id + g_start_ifid;
            ppa_drv_g_ppe_directpath_data[if_id].flags      = tmp_flags;
            *p_if_id = if_id + g_start_ifid;
            ret = PPA_SUCCESS;
        }
        else
        {
            ret = PPA_FAILURE;
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "direct register fail\n" );
        }
        ppa_lock_release(&g_directpath_port_lock);
	printk("<%s> Exit - DP Register\n",__FUNCTION__);

        return ret;
    }
    else
    {
        if ( *p_if_id < g_start_ifid || *p_if_id >= g_end_ifid )
        {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"directp unregister wrong id: %d\n", *p_if_id);
            return PPA_EINVAL;
        }

        if_id = *p_if_id - g_start_ifid;
        ppa_lock_get(&g_directpath_port_lock);
        skb_list=__remove_directpath_dev_from_datapath(if_id);       
        ppa_lock_release(&g_directpath_port_lock);

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
        remove_directpath_queue(skb_list);
#endif
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"unregiter directpath ok\n");
	printk("<%s> Exit - DP UnRegister\n",__FUNCTION__);
        return PPA_SUCCESS;
    }
}


#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
int32_t ppa_directpath_register_dev_ex(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags)
{
    int32_t ret;
    int32_t if_id;
    uint32_t tmp_flags;
    PPA_BUF *skb_list = NULL;

    if( !ppa_is_init() ) return PPA_EINVAL;

    if ( subif == NULL || !ppa_drv_g_ppe_directpath_data  )
        return PPA_EINVAL;

    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Entry Port Id :%d\n", subif->port_id);
    if ( (flags & PPA_F_DIRECTPATH_REGISTER) )
    {
        if ( !(flags & PPA_F_DIRECTPATH_ETH_IF) )
            //  currently, only ethernet interface is supported
            return PPA_EINVAL;

        if ( !netif || !pDirectpathCb
            || !pDirectpathCb->rx_fn )
            return PPA_EINVAL;

        if ( ppa_get_netif_name(netif) == NULL )
            return PPA_EINVAL;

        tmp_flags = (flags & PPA_F_DIRECTPATH_WAN) ? PPE_DIRECTPATH_WAN : PPE_DIRECTPATH_LAN;
        if ( (flags & PPA_F_DIRECTPATH_CORE1) )
            tmp_flags |= PPE_DIRECTPATH_CORE1;
        else
            tmp_flags |= PPE_DIRECTPATH_CORE0;
        if ( (flags & PPA_F_DIRECTPATH_ETH_IF) )
            tmp_flags |= PPE_DIRECTPATH_ETH;


        //  update flags which could be changed on the fly, e.g. AR9/VR9/AR10 - LAN/WAN could be changed
        update_itf_info();

        ppa_lock_get(&g_directpath_port_lock);

        //first check whether the interface already added into PPA directpath or not
        for ( if_id = 0; if_id < g_end_ifid; if_id ++ )
        {
            if( ppa_drv_g_ppe_directpath_data[if_id].netif && ( ppa_drv_g_ppe_directpath_data[if_id].netif == netif ) )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "The interface already in PPA directpath already. Its netif pointer is :%p\n", netif);
                if(subif ){ 
#ifndef CONFIG_PPA_PUMA7
			if(flags & PPE_DIRECTPATH_LEGACY) {
				subif->port_id = ppa_drv_g_ppe_directpath_data[if_id].dp_subif.port_id ;
			} else {
				subif->port_id = ppa_drv_g_ppe_directpath_data[if_id].dp_subif.port_id ;
				subif->subif = ppa_drv_g_ppe_directpath_data[if_id].dp_subif.subif;
			}
#endif
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
			subif->port_id = DP_PORT_ID(if_id);
			subif->subif   = DP_SUBIF(if_id);
#endif
		}

                ppa_lock_release(&g_directpath_port_lock);
                return PPA_SUCCESS;
            }
        }
//#if !defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
	if(flags & PPE_DIRECTPATH_LEGACY) {
		subif->port_id = -1;
		subif->subif = -1;
	}
//#endif
	ret= DRV_REGISTER(subif, netif, pDirectpathCb, &if_id, flags);
	if(ret == PPA_SUCCESS) {
    		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Port Id :[%d] subif :[%d] if_id :[%d] \n", subif->port_id, subif->subif, if_id);
//#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if 0
		if(flags & PPE_DIRECTPATH_LEGACY) {
			subif->port_id = ppa_drv_g_ppe_directpath_data[if_id].dp_subif.port_id ;
		} else {
			subif->port_id = ppa_drv_g_ppe_directpath_data[if_id].dp_subif.port_id ;
			subif->subif = ppa_drv_g_ppe_directpath_data[if_id].dp_subif.subif;
		}
#endif
		if(subif->subif == -1) { // required only for the physical port
			if(ppa_phys_port_add(ppa_get_netif_name(netif), subif->port_id) == PPA_SUCCESS)
			{
            			tmp_flags = (ppa_drv_g_ppe_directpath_data[if_id].flags & 
							PPE_DIRECTPATH_MASK) | PPE_DIRECTPATH_DATA_ENTRY_VALID | PPE_DIRECTPATH_DATA_RX_ENABLE;
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
				ppe_lock_init(&ppa_drv_g_ppe_directpath_data[if_id].txq_lock);
#endif
				ppa_drv_g_ppe_directpath_data[if_id].flags      = tmp_flags;
			}
			else
        		{
            			ret = PPA_FAILURE;
            			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "direct register fail\n" );
        		}

		} else {
    			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Subif :%d\n", subif->subif);
            		tmp_flags = (ppa_drv_g_ppe_directpath_data[if_id].flags & 
						PPE_DIRECTPATH_MASK) | PPE_DIRECTPATH_DATA_ENTRY_VALID | PPE_DIRECTPATH_DATA_RX_ENABLE;
			ppa_drv_g_ppe_directpath_data[if_id].flags      = tmp_flags;
		}
	}
        ppa_lock_release(&g_directpath_port_lock);

        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Exit - DP Register \n");
        return ret;
    }
    else  // directpath unregister
    {
    	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Entry - DP UnRegister for port id :%d\n", subif->port_id);
        if ( subif->port_id < 0 || subif->port_id >= g_end_ifid )
        {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"directp unregister wrong id: %d\n", subif->port_id);
            return PPA_EINVAL;
        }

        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Entry - DP UnRegister \n");
        ret = DRV_REGISTER(subif, netif, pDirectpathCb, &if_id, flags);
        ppa_lock_get(&g_directpath_port_lock);
    	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Remove directpath device index :%d\n", if_id);
        skb_list=__remove_directpath_dev_from_datapath(if_id);       
        ppa_lock_release(&g_directpath_port_lock);

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
        remove_directpath_queue(skb_list);
#endif
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Unregiter Directpath ok\n");
        return PPA_SUCCESS;
    }
}
#endif


int32_t ppa_directpath_ex_register_dev(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags)
{
    	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Entry - Port id :%d\n", subif->port_id);
	if(flags & PPE_DIRECTPATH_LEGACY)
	{
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)

        	return ppa_directpath_register_dev_ex(subif, netif, pDirectpathCb, flags);        
#else
		int32_t ret, if_id;
		if_id = subif->port_id;
		ret = ppa_directpath_register_dev_legacy(&if_id, netif, pDirectpathCb, flags);
		if(ret != PPA_FAILURE)
			subif->port_id = if_id;
		return ret;
#endif
	} else if(!(flags & PPE_DIRECTPATH_LEGACY)) {
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
    		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "For grx500 platform - Port id :%d\n", subif->port_id);
        	return ppa_directpath_register_dev_ex(subif, netif, pDirectpathCb, flags);        

#endif
	}
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Exit \n");
	//printk("<%s> Exit\n",__FUNCTION__);

        return PPA_SUCCESS;
}

int32_t ppa_directpath_register_dev(uint32_t *p_if_id, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags)
{
	int32_t ret;
	PPA_SUBIF sub_if;

	//printk("<%s> Entry port id =%d\n",__FUNCTION__, *p_if_id);
    	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Entry - Port id :%d\n", *p_if_id);
	if ( (flags & PPA_F_DIRECTPATH_REGISTER) ) {
		sub_if.port_id = -1;
		sub_if.subif = 0;
	} else {
		sub_if.port_id = *p_if_id;
		sub_if.subif = -1;
	}
	ret = ppa_directpath_ex_register_dev(&sub_if, netif, pDirectpathCb, flags | PPE_DIRECTPATH_LEGACY);
	if(ret == PPA_SUCCESS){
		*p_if_id = sub_if.port_id;
	}
	return ret;
	
}
/* =====================  */

int32_t ppa_directpath_send_legacy(uint32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags)
{
#if !defined(CONFIG_LTQ_PPA_API_DIRECTPATH_BRIDGING)
    uint8_t mac[PPA_ETH_ALEN] = {0};
    uint8_t netif_mac[PPA_ETH_ALEN] = {0};
#endif
    int32_t ret = PPA_SUCCESS;

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
    int32_t (*tmp_imq_queue)(PPA_BUF *skb, uint32_t portID) = ppa_hook_directpath_enqueue_to_imq_fn;
#endif        
    if( skb == NULL )
    {
        return PPA_EINVAL;
    }
    
    if( !ppa_is_init() ) 
    {
        PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }
    if(flags & 3) { //directlink function call
        return ppa_drv_directpath_send(rx_if_id, skb, len, flags);
    }
    if( !ppa_drv_g_ppe_directpath_data ) 
    {
        PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }

    ppa_lock_get(&g_directpath_port_lock);
    
    if ( rx_if_id < g_start_ifid || rx_if_id >= g_end_ifid )
    {
        ret = PPA_EINVAL;
        goto __DIRETPATH_TX_EXIT;
    }

    if ( !(ppa_drv_g_ppe_directpath_data[rx_if_id - g_start_ifid].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
    {
        ret = PPA_EPERM;
        goto __DIRETPATH_TX_EXIT;
    }

#if !defined(CONFIG_LTQ_PPA_API_DIRECTPATH_BRIDGING)
    ppa_get_pkt_rx_dst_mac_addr(skb, mac);
    ppa_get_netif_hwaddr(ppa_drv_g_ppe_directpath_data[rx_if_id - g_start_ifid].netif, netif_mac, 0);
    if ( ppa_memcmp(mac, /* ppa_drv_g_ppe_directpath_data[rx_if_id - g_start_ifid].mac */ netif_mac, PPA_ETH_ALEN) != 0 )
    {
        //  bridge
        rx_if_id -= g_start_ifid;
        if ( (ppa_drv_g_ppe_directpath_data[rx_if_id].flags & PPE_DIRECTPATH_DATA_RX_ENABLE) )
        {
            ppa_drv_g_ppe_directpath_data[rx_if_id].callback.rx_fn(ppa_drv_g_ppe_directpath_data[rx_if_id].netif, NULL, skb, len);
            ret = LTQ_SUCCESS;
            goto __DIRETPATH_TX_EXIT;
        }
        else
        {
            ret = PPA_EINVAL;
            goto __DIRETPATH_TX_EXIT;
        }
    }
#endif


#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
    if( ppa_directpath_imq_en_flag && tmp_imq_queue )
    {
         if( tmp_imq_queue(skb, rx_if_id) == 0 )
        { 
            ppa_lock_release(&g_directpath_port_lock);
            return 0;
        }
   }
#endif
   ret= ppa_drv_directpath_send(rx_if_id, skb, len, flags);
   ppa_lock_release(&g_directpath_port_lock);
   return ret;
   

__DIRETPATH_TX_EXIT:
   if(ret != PPA_SUCCESS && skb){
        PPA_SKB_FREE(skb);
   }

   ppa_lock_release(&g_directpath_port_lock);
   return ret;

}


#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
int (*datapath_dtlk_switch_parser)(void) = NULL;
EXPORT_SYMBOL(datapath_dtlk_switch_parser);
#endif
int32_t ppa_directpath_send_ex(PPA_SUBIF *subif, PPA_BUF *skb, int32_t len, uint32_t flags)
{
#if !defined(CONFIG_LTQ_PPA_API_DIRECTPATH_BRIDGING)
    uint8_t mac[PPA_ETH_ALEN] = {0};
    uint8_t netif_mac[PPA_ETH_ALEN] = {0};
#endif
    int32_t ret = PPA_SUCCESS;

#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
    int32_t (*tmp_imq_queue)(PPA_BUF *skb, uint32_t portID) = ppa_hook_directpath_enqueue_to_imq_fn;
#endif        
	//pr_err("%s: skb[0x%x] skb->data[0x%x]\n", __func__, skb, skb->data);
    if( skb == NULL )
    {
        return PPA_EINVAL;
    }
    
    if( !ppa_is_init() ) 
    {
        PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }
    if(flags & 3) { //directlink function call
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
		/*
		* Special for DirectLink: DirectLink needs extra 48 bytes before read data.
		*/
		int bSwitchParserIsDisable = 1;
		PPA_BUF *skb2 = NULL;
		int newLen = len;
		if (datapath_dtlk_switch_parser)
			bSwitchParserIsDisable = datapath_dtlk_switch_parser();

		if (bSwitchParserIsDisable) {
			skb2 = alloc_skb(skb->len + 48, GFP_KERNEL); //previous 2048
			newLen = len + 48;
		} else {
			skb2 = skb;
			newLen = len;
		}
		if (!skb2) {
			pr_err("<%s>: cannot allocate [%d] bytes\n", __func__ );
			ret = PPA_ENOMEM;
			goto __DIRETPATH_TX_EXIT;
		}
		/* Copy */
		if (bSwitchParserIsDisable) {
			if (skb->dev) {
				skb2->ip_summed = CHECKSUM_UNNECESSARY;
				unsigned char *data = skb_put(skb2, len + 48);
				memset(data, 0, len + 48);
				memcpy(data + 48, skb->data, len);
				skb2->dev = skb->dev;
				//pr_err("DIRECTLINK send: orig[0x%x] skb2->data[0x%x] len+48[%d]\n", skb->data,skb2->data, len + 48);
				/* Free old skb */
				ppa_buf_free(skb);
			} else {
				ret = PPA_EINVAL;
				ppa_buf_free(skb2);
				pr_err("DIRECTLINK send: no skb->dev\n");
				goto __DIRETPATH_TX_EXIT;
			}
		}
		return DRV_SEND(subif, skb2, newLen, DP_TX_TO_DL_MPEFW);
#endif
    }
    if( !ppa_drv_g_ppe_directpath_data ) 
    {
        PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
	ppa_dp_coc_record_time(skb, flags);
#endif
#endif

    ppa_lock_get(&g_directpath_port_lock);
    
    if ( subif->port_id < g_start_ifid || subif->port_id >= g_end_ifid )
    {
        ret = PPA_EINVAL;
        goto __DIRETPATH_TX_EXIT;
    }
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
    if ( !(ppa_drv_g_ppe_directpath_data[subif->port_id * (MAX_SUBIF + 1)].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
#else
    if ( !(ppa_drv_g_ppe_directpath_data[subif->port_id - g_start_ifid].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
#endif
    {
        ret = PPA_EPERM;
        goto __DIRETPATH_TX_EXIT;
    }
#if !defined(CONFIG_LTQ_PPA_API_DIRECTPATH_BRIDGING)
    ppa_get_pkt_rx_dst_mac_addr(skb, mac);
    ppa_get_netif_hwaddr(ppa_drv_g_ppe_directpath_data[subif->port_id - g_start_ifid].netif, netif_mac, 0);
    if ( ppa_memcmp(mac, /* ppa_drv_g_ppe_directpath_data[rx_if_id - g_start_ifid].mac */ netif_mac, PPA_ETH_ALEN) != 0 )
    {
        //  bridge
        subif->port_id -= g_start_ifid;
        if ( (ppa_drv_g_ppe_directpath_data[subif->port_id].flags & PPE_DIRECTPATH_DATA_RX_ENABLE) )
        {
            ppa_drv_g_ppe_directpath_data[subif->port_id].callback.rx_fn(ppa_drv_g_ppe_directpath_data[subif->port_id].netif, NULL, skb, len);
            ret = LTQ_SUCCESS;
            goto __DIRETPATH_TX_EXIT;
        }
        else
        {
            ret = PPA_EINVAL;
            goto __DIRETPATH_TX_EXIT;
        }
    }
#endif


#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
    if( ppa_directpath_imq_en_flag && tmp_imq_queue )
    {
         if( tmp_imq_queue(skb, subif->port_id) == 0 )
        { 
            ppa_lock_release(&g_directpath_port_lock);
            return 0;
        }
   }
#endif

//#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
	ppa_dp_ewma_coc(subif, skb, flags);
#endif
#endif

   ret= DRV_SEND(subif, skb, len, flags);
   ppa_lock_release(&g_directpath_port_lock);
   return ret;
   

__DIRETPATH_TX_EXIT:
   if(ret != PPA_SUCCESS && skb){
        PPA_SKB_FREE(skb);
   }

   ppa_lock_release(&g_directpath_port_lock);
   return ret;

}
#endif


int32_t ppa_directpath_ex_send(PPA_SUBIF *subif, PPA_BUF *skb, int32_t len, uint32_t flags)
{
	//pr_err("%s\n", __func__);
	if(flags & PPE_DIRECTPATH_LEGACY)
	{
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)

        	return ppa_directpath_send_ex(subif, skb, len, flags);        
#else
		int32_t ret;
		ret = ppa_directpath_send_legacy(subif->port_id, skb, len, flags);
		//if(ret != PPA_FAILURE)
		//	subif->port_id = if_id;
		return ret;
#endif
	} else if(!(flags & PPE_DIRECTPATH_LEGACY)) {
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)

        	return ppa_directpath_send_ex(subif, skb, len, flags);        

#endif

	}	
	return PPA_FAILURE;
}
int32_t ppa_directpath_send(uint32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags)
{
	int32_t ret;
	PPA_SUBIF sub_if;

	sub_if.port_id = rx_if_id;
	sub_if.subif = -1;
	ret = ppa_directpath_ex_send(&sub_if, skb, len, flags | PPE_DIRECTPATH_LEGACY);
	//if(ret == PPA_SUCCESS){
	//	*p_if_id = sub_if.port_id;
	//}
	return ret;
	


}

/** ========================== */


#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
int32_t ppa_directpath_reinject_from_imq(int32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags)
{
#if !defined(CONFIG_LTQ_PPA_API_DIRECTPATH_BRIDGING)
    uint8_t mac[PPA_ETH_ALEN] = {0};
    uint8_t netif_mac[PPA_ETH_ALEN] = {0};
#endif
    int32_t ret = PPA_SUCCESS;

    if( skb == NULL )
    {
        return PPA_EINVAL;
    }
    
    if( !ppa_is_init() ) 
    {
        PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }
    if( !ppa_drv_g_ppe_directpath_data ) 
    {
        PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }

    ppa_lock_get(&g_directpath_port_lock);
    
    if ( rx_if_id < g_start_ifid || rx_if_id >= g_end_ifid )
    {
        ret = PPA_EINVAL;
        goto __DIRETPATH_TX_EXIT;
    }

    if ( !(ppa_drv_g_ppe_directpath_data[rx_if_id - g_start_ifid].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
    {
        ret = PPA_EPERM;
        goto __DIRETPATH_TX_EXIT;
    }

    ret = DRV_SEND(rx_if_id, skb, len, flags);
    ppa_lock_release(&g_directpath_port_lock);
    return ret;
    

__DIRETPATH_TX_EXIT:
   if(ret != PPA_SUCCESS && skb){
        PPA_SKB_FREE(skb);
   }

   ppa_lock_release(&g_directpath_port_lock);
   return ret;

}
#endif

int32_t ppa_directpath_rx_stop(uint32_t if_id, uint32_t flags)
{
    int32_t ret;
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
    PPA_SUBIF subif;
#endif


    if( !ppa_is_init() ) return PPA_EINVAL;
    if( !ppa_drv_g_ppe_directpath_data ) return PPA_EINVAL;

    ppa_lock_get(&g_directpath_port_lock);
    if ( if_id >= g_start_ifid && if_id < g_end_ifid
        && (ppa_drv_g_ppe_directpath_data[if_id - g_start_ifid].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
    {
        ppa_drv_g_ppe_directpath_data[if_id - g_start_ifid].flags &= ~PPE_DIRECTPATH_DATA_RX_ENABLE;
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
        memset(&subif, 0, sizeof(subif));
        subif.port_id = if_id;
        ret = DRV_RX_STOP(&subif, flags) == 0 ? PPA_SUCCESS : PPA_FAILURE;
#else
        ret = DRV_RX_STOP(if_id, flags) == 0 ? PPA_SUCCESS : PPA_FAILURE;
#endif
    }
    else
        ret = PPA_EINVAL;
    ppa_lock_release(&g_directpath_port_lock);

    return ret;
}

int32_t ppa_directpath_ex_rx_stop(PPA_SUBIF *subif, uint32_t flags)
{
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
    int32_t ret;

    if( !ppa_is_init() ) return PPA_EINVAL;
    if( !ppa_drv_g_ppe_directpath_data ) return PPA_EINVAL;

    ppa_lock_get(&g_directpath_port_lock);
    /* TODO: subif validation */
    if ((ppa_drv_g_ppe_directpath_data[DP_DATA_INDEX(subif)].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID))
    {
        ppa_drv_g_ppe_directpath_data[DP_DATA_INDEX(subif)].flags &= ~PPE_DIRECTPATH_DATA_RX_ENABLE;
       ret = DRV_RX_STOP(subif, flags) == 0 ? PPA_SUCCESS : PPA_FAILURE;
    }
    else
        ret = PPA_EINVAL;
    ppa_lock_release(&g_directpath_port_lock);

    return ret;
#else
    return ppa_directpath_rx_stop(subif->port_id, flags);
#endif
}

int32_t ppa_directpath_rx_restart(uint32_t if_id, uint32_t flags)
{
    int32_t ret;
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
    PPA_SUBIF subif;
#endif

    if( !ppa_is_init() ) return PPA_EINVAL;
    if( !ppa_drv_g_ppe_directpath_data ) return PPA_EINVAL;

    ppa_lock_get(&g_directpath_port_lock);
    if ( if_id >= g_start_ifid && if_id < g_end_ifid
        && (ppa_drv_g_ppe_directpath_data[if_id - g_start_ifid].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
    {
        ppa_drv_g_ppe_directpath_data[if_id - g_start_ifid].flags |= PPE_DIRECTPATH_DATA_RX_ENABLE;
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
        memset(&subif, 0, sizeof(subif));
        subif.port_id = if_id;
        ret = DRV_RX_START(&subif, flags) == 0 ? PPA_SUCCESS : PPA_FAILURE;
#else
        ret = DRV_RX_START(if_id, flags) == 0 ? PPA_SUCCESS : PPA_FAILURE;
#endif

    }
    else
        ret = PPA_EINVAL;
    ppa_lock_release(&g_directpath_port_lock);

    return ret;
}

int32_t ppa_directpath_ex_rx_restart(PPA_SUBIF *subif, uint32_t flags)
{
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
    int32_t ret;

    ppa_lock_get(&g_directpath_port_lock);
    /* TODO: subif validation */
    if (ppa_drv_g_ppe_directpath_data[DP_DATA_INDEX(subif)].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID)
    {
        ppa_drv_g_ppe_directpath_data[DP_DATA_INDEX(subif)].flags |= PPE_DIRECTPATH_DATA_RX_ENABLE;
        ret = DRV_RX_START(subif, flags) == 0 ? PPA_SUCCESS : PPA_FAILURE;

    }
    else
        ret = PPA_EINVAL;
    ppa_lock_release(&g_directpath_port_lock);

    return ret;
#else
    return ppa_directpath_rx_restart(subif->port_id, flags);
#endif
}

PPA_BUF* ppa_directpath_alloc_skb(PPA_SUBIF *subif, int32_t len, uint32_t flags)
{
    PPA_BUF *ret = (void *)PPA_EINVAL;

#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
    ppa_lock_get(&g_directpath_port_lock);
    /* TODO: subif validation */
    if (ppa_drv_g_ppe_directpath_data[DP_DATA_INDEX(subif)].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID)
    {
        ret = DRV_ALLOC_SKB(subif, len, flags);
    }
    ppa_lock_release(&g_directpath_port_lock);
#else
    printk("ppa_directpath_alloc_skb not defined!!\n");
#endif

    return ret;
}

int32_t ppa_directpath_recycle_skb(PPA_SUBIF *subif, PPA_BUF *skb, uint32_t flags)
{
    int32_t ret = PPA_EINVAL;

#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
    ppa_lock_get(&g_directpath_port_lock);
    /* TODO: subif validation */
    if (ppa_drv_g_ppe_directpath_data[DP_DATA_INDEX(subif)].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID)
    {
        ret = DRV_RECYCLE_SKB(subif, skb, flags);
    }
    ppa_lock_release(&g_directpath_port_lock);
#else
    printk("ppa_directpath_recycle_skb not defined!!\n");
#endif

    return ret;
}

PPA_NETIF *ppa_get_netif_for_ppa_ifid(uint32_t if_id)
{
    PPA_NETIF *ret = NULL;

    if( !ppa_is_init() ) return NULL;
    if( !ppa_drv_g_ppe_directpath_data ) return NULL;

    ppa_lock_get(&g_directpath_port_lock);
    if ( if_id >= g_start_ifid && if_id < g_end_ifid )
    {
        if_id -= g_start_ifid;
        if ( (ppa_drv_g_ppe_directpath_data[if_id].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) )
            ret = ppa_drv_g_ppe_directpath_data[if_id].netif;
    }
    ppa_lock_release(&g_directpath_port_lock);

    return ret;
}

int32_t ppa_get_ifid_for_netif(PPA_NETIF *netif)
{
    uint32_t if_id;

    if( !ppa_is_init() ) return PPA_EINVAL;
    if( !ppa_drv_g_ppe_directpath_data ) return PPA_EINVAL;

    if ( netif == NULL )
        return PPA_EINVAL;

    ppa_lock_get(&g_directpath_port_lock);
    for ( if_id = 0; if_id < g_end_ifid - g_start_ifid; if_id++ )
        if ( (ppa_drv_g_ppe_directpath_data[if_id].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID)
            && ppa_is_netif_equal(netif, ppa_drv_g_ppe_directpath_data[if_id].netif) )
        {
            ppa_lock_release(&g_directpath_port_lock);
            return if_id + g_start_ifid;
        }
    ppa_lock_release(&g_directpath_port_lock);

    return PPA_FAILURE;
}
EXPORT_SYMBOL(ppa_get_ifid_for_netif);

int32_t ppa_directpath_data_start_iteration(uint32_t *ppos, struct ppe_directpath_data **info)
{
    uint32_t l;

    ppa_lock_get(&g_directpath_port_lock);
	
    if( !ppa_is_init() ) return PPA_EINVAL;
    
    if ( g_start_ifid == ~0 )
    {
        *info = NULL;
        return PPA_FAILURE;
    }

    if( !ppa_drv_g_ppe_directpath_data ) return PPA_EINVAL;

    l = *ppos;
    if ( l + g_start_ifid < g_end_ifid )
    {
        ++*ppos;
        *info = ppa_drv_g_ppe_directpath_data + l;
        return PPA_SUCCESS;
    }
    else
    {
        *info = NULL;
        return PPA_FAILURE;
    }
}

int32_t ppa_directpath_data_iterate_next(uint32_t *ppos, struct ppe_directpath_data **info)
{
    if( !ppa_is_init() ) return PPA_EINVAL;
    if( !ppa_drv_g_ppe_directpath_data ) return PPA_EINVAL;

    if ( *ppos + g_start_ifid < g_end_ifid )
    {
        *info = ppa_drv_g_ppe_directpath_data + *ppos;
        ++*ppos;
        return PPA_SUCCESS;
    }
    else
    {
        *info = NULL;
        return PPA_FAILURE;
    }
}

void ppa_directpath_data_stop_iteration(void)
{
    ppa_lock_release(&g_directpath_port_lock);
}

void ppa_directpath_get_ifid_range(uint32_t *p_start_ifid, uint32_t *p_end_ifid)
{
    if ( p_start_ifid )
        *p_start_ifid = g_start_ifid;
    if ( p_end_ifid )
        *p_end_ifid = g_end_ifid;
}



/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

int32_t ppa_api_directpath_init(void)
{
    uint32_t i;
    uint32_t last_valid_ifid;
    uint32_t tmp_flags;
    PPE_IFINFO if_info;
    PPE_COUNT_CFG count={0};

    if ( !ppa_drv_g_ppe_directpath_data )
        return PPA_EINVAL;

    ppa_drv_get_number_of_phys_port(&count, 0);
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
    last_valid_ifid = count.num * (MAX_SUBIF+1) -1;
#else
    last_valid_ifid = count.num - 1;
#endif
    ppa_memset( &if_info, 0, sizeof(if_info) );
    for ( i = 0; i < count.num; i++ )
    {
        if_info.port = i;
        ppa_drv_get_phys_port_info(&if_info, 0);
        if ( (if_info.if_flags & (PPA_PHYS_PORT_FLAGS_VALID | PPA_PHYS_PORT_FLAGS_TYPE_MASK)) == (PPA_PHYS_PORT_FLAGS_VALID | PPA_PHYS_PORT_FLAGS_TYPE_EXT)
            && (if_info.if_flags  & (PPA_PHYS_PORT_FLAGS_EXT_CPU0 | PPA_PHYS_PORT_FLAGS_EXT_CPU1)) != 0 )
        {
            if ( g_start_ifid == ~0 )
                g_start_ifid = i;
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
            last_valid_ifid += i * (MAX_SUBIF + 1);
#else
            last_valid_ifid = i;
#endif
            tmp_flags = PPE_DIRECTPATH_ETH;
            if ( (if_info.if_flags  & PPA_PHYS_PORT_FLAGS_EXT_CPU0) )
                tmp_flags |= PPE_DIRECTPATH_CORE0;
            if ( (if_info.if_flags  & PPA_PHYS_PORT_FLAGS_EXT_CPU1) )
                tmp_flags |= PPE_DIRECTPATH_CORE1;
            if ( (if_info.if_flags & PPA_PHYS_PORT_FLAGS_MODE_LAN) )
                tmp_flags |= PPE_DIRECTPATH_LAN;
            if ( (if_info.if_flags & PPA_PHYS_PORT_FLAGS_MODE_WAN) )
                tmp_flags |= PPE_DIRECTPATH_WAN;
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "%s: if_info.if_flags = 0x%08x, direct_flag[%d]=0x%08x, tmp_flags=0x%08x\n", __FUNCTION__, if_info.if_flags, i - g_start_ifid, ppa_drv_g_ppe_directpath_data[i - g_start_ifid].flags, tmp_flags);
#if (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500) || (defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7)
            ppa_drv_g_ppe_directpath_data[i * (MAX_SUBIF + 1)].flags = tmp_flags;
#else
            ppa_drv_g_ppe_directpath_data[i - g_start_ifid].flags = tmp_flags;
#endif
        }
    }
    g_end_ifid = last_valid_ifid + 1;

    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "directpath start id: %d, end id: %d\n", g_start_ifid, g_end_ifid);
    printk( "directpath start id: %d, end id: %d\n", g_start_ifid, g_end_ifid);

    return PPA_SUCCESS;
}

void ppa_api_directpath_exit(void)
{
    uint32_t i;  
    PPA_BUF *skb_list = NULL;



    if ( g_end_ifid != ~0 )
    {
        if( ppa_drv_g_ppe_directpath_data )
        {
            ppa_lock_get(&g_directpath_port_lock);
            for ( i = 0; i < g_end_ifid - g_start_ifid; i++ )
            {
                if( ppa_drv_g_ppe_directpath_data[i].flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) 
                {
                    skb_list=__remove_directpath_dev_from_datapath(i);      
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_SIZE) || defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_QUEUE_PKTS)
                    remove_directpath_queue(skb_list);                    
#endif                    
                }
            }
            g_start_ifid = g_end_ifid = ~0;
            ppa_lock_release(&g_directpath_port_lock);
        }
    }

}

int32_t ppa_api_directpath_create(void)
{
    if ( ppa_lock_init(&g_directpath_port_lock) )
    {
        err("Failed in creating lock for directpath port.");
        return PPA_EIO;
    }

    return PPA_SUCCESS;
}

void ppa_api_directpath_destroy(void)
{
    ppa_lock_destroy(&g_directpath_port_lock);
}

EXPORT_SYMBOL(ppa_directpath_data_start_iteration);
EXPORT_SYMBOL(ppa_directpath_data_iterate_next);
EXPORT_SYMBOL(ppa_directpath_data_stop_iteration);
EXPORT_SYMBOL(ppa_directpath_get_ifid_range);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
EXPORT_SYMBOL(g_ppa_dp_window);
EXPORT_SYMBOL(g_ppa_dp_thresh);
#endif
#endif
#if defined(LTQ_PPA_DIRECTPATH_TX_IMQ)
    EXPORT_SYMBOL(ppa_directpath_reinject_from_imq);
#endif
