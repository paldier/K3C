/*******************************************************************************
**
** FILE NAME    : ppa_api_session_limit.c
** PROJECT      : PPA
** MODULES      : PPA API (Session Limiting APIs)
**
** DATE         : 20 May 2014
** AUTHOR       : Lantiq
** DESCRIPTION  : PPA Session Limiting APIs
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
**                                     Features
*******************************************************************************/


#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif


#include <linux/swap.h>
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#include "ppa_api_session_limit.h"
#include "ppa_api_mib.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_tools.h"
#include <net/ppa_stack_al.h>
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
#include "ppa_api_hal_selector.h"
#endif

uint32_t g_ppa_lan_reserve_collisions = LAN_RESERVE_COLLISIONS;
uint32_t g_ppa_wan_reserve_collisions = WAN_RESERVE_COLLISIONS;
uint32_t g_ppa_lan_current_collisions = 0;
uint32_t g_ppa_wan_current_collisions = 0;
uint32_t g_ppa_low_prio_data_rate = 0;
uint32_t g_ppa_def_prio_data_rate = 0;
uint32_t g_ppa_low_prio_thresh = 0;
uint32_t g_ppa_def_prio_thresh = 0;


unsigned int session_count[MAX_SESSION_TYPE][MAX_SESSION_PRIORITY][MAX_DATA_FLOW_ENGINES];
EXPORT_SYMBOL(session_count);

PPA_LIST_HEAD session_list_head[MAX_SESSION_TYPE][MAX_SESSION_PRIORITY][MAX_DATA_FLOW_ENGINES];

void ppa_session_mgmt_init(void) 
{
	int session_type,session_prio,engine;
	for(session_type=0; session_type<MAX_SESSION_TYPE; session_type++)
		for(session_prio=0; session_prio<MAX_SESSION_PRIORITY; session_prio++)
			for(engine=0; engine<MAX_DATA_FLOW_ENGINES; engine++)
				PPA_LIST_HEAD_INIT(&session_list_head[session_type][session_prio][engine]);
	return; 
}

void ppa_session_mgmt_exit(void) 
{
	int session_type,session_prio,engine;
	struct session_list_item *p_item, *start;

	for(session_type=0; session_type<MAX_SESSION_TYPE; session_type++)
		for(session_prio=0; session_prio<MAX_SESSION_PRIORITY; session_prio++)
			for(engine=0; engine<MAX_DATA_FLOW_ENGINES; engine++) {
					ppa_list_for_each_entry_safe_reverse(p_item, start, &session_list_head[session_type][engine][session_prio], priority_list) {
						ppa_list_del(&p_item->priority_list);
					}
					session_count[session_type][session_prio][engine]=0;
			}
	return; 
}

void session_mgmt_stats(struct session_list_item *p_item, enum ppa_oper_type oper) 
{
	int mark;
	int low_mark = ppa_get_low_prio_thresh(1);
	int med_mark = ppa_get_def_prio_thresh(1);

	enum ppa_engine_type engine;
	enum sess_type session_type;
	enum sess_class_type session_prio;

	if(p_item->flags & SESSION_ADDED_IN_HW) {
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
		if (p_item->num_caps == 0 || p_item->num_caps != 1) { //if number of selected hals is not set to 1 then its complementery prcoessing	
			return;
		}
		if(	p_item->caps_list[0].hal_id == PAE_HAL) {
			engine = PAE; 	
		} else if(p_item->caps_list[0].hal_id == MPE_HAL) {
			engine = MPE;
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
		} else if (p_item->caps_list[0].hal_id == SWAC_HAL) {
			engine = SWAC;
#endif
		} else {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update_session_mgmt_stats(p_item, %d)failed\n", oper);
			return;
		}
#else
		engine = PPE;
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
	} else if(p_item->flags & SESSION_ADDED_IN_SW) {
		engine = SWAC;	
#else
	} else {
		engine = STACK;
#endif
	} else {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update_session_mgmt_stats(p_item, %d)failed\n", oper);
		return;
	}

	if(p_item->flags & SESSION_LAN_ENTRY) {
		session_type = LAN;
	} else if(p_item->flags & SESSION_WAN_ENTRY) {
		session_type = WAN;
	} else {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update_session_mgmt_stats(p_item) failed to select LAN/WAN\n");
		return;
	}

	if(oper == DELETE) {
		if(p_item->session_class == 0) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update_session_mgmt_stats(p_item) delete failed \n");
			return;
		}

		ppa_list_del(&p_item->priority_list);
		session_count[session_type][p_item->session_class-1][engine]--;
		p_item->session_class=0;

	} else if(oper == ADD) {
		if(p_item->session_class > 0) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update_session_mgmt_stats(p_item) add failed \n");
			return;
		}

		mark = p_item->session_priority;

		if(mark > med_mark) {
			session_prio = HIGH;
		} else {
			if((mark > low_mark && mark <= med_mark)|| mark == 0) {	
				session_prio = DEFAULT;
			} else {
				session_prio = LOW;
			}
		}
		p_item->session_class=session_prio; 
		ppa_list_add_head(&p_item->priority_list, &session_list_head[session_type][session_prio-1][engine]);
		session_count[session_type][session_prio-1][engine]++;
	}
	return;
}

void update_session_mgmt_stats(struct session_list_item *p_item, enum ppa_oper_type oper)
{
	if ( ! ppa_get_session_limit_enable(0))
		return;
	session_mgmt_stats(p_item, oper);
} 
EXPORT_SYMBOL(update_session_mgmt_stats);

// To arrive at correct data rate for TCP session,we wait for few seconds to calculate data rate.
//wait_time.tv_sec = nf_conntrack_sessionmgmt_wait_time : this is period in seconds we want to wait before calculating data rate (will be kept as 5 sesonds by default).This can be modified using
//proc entry on the fly
// For this duration we keep adding session bytes cumulatively, Once session cross this duration we calculate data rate by session_bytes/duration and check if it
//satishfies data rate condition. If data rate is higher than pass criteria we add session to PPA/PPE  and return success. 
	

uint32_t ppa_session_pass_criteria(PPA_BUF *ppa_buf, struct session_list_item *p_item,uint32_t flags)
{
    uint64_t time_msec;
    uint64_t total_bytes;
   	PPA_TIMESPEC wait_time;
   	uint64_t wait_time_const;
    PPA_TIMESPEC after;
	uint64_t new_wait_time_const;
    int multiply_offset = 1;
	//wait_time.tv_sec = nf_conntrack_sessionmgmt_add_time; // This duration is required to correctly arrive data rate of session
	wait_time.tv_sec = 5; // This duration is required to correctly arrive data rate of session
															//as in case of TCP there is initial burst so that dats rate will be very high 
															//for first couple of seconds and then it will get settled around specified bandwidth.
															// and also it makes sure that no transient sessions are getting added to ppa/ppe.
	wait_time.tv_nsec =0 ;

    // This is the most basic critria which should be met for all sessions
    if ( p_item->num_adds < g_ppa_min_hits )
	  return PPA_FAILURE;

    // Return if session limit check is not enabled
    if ( ! ppa_get_session_limit_enable(0) )
	  return PPA_SUCCESS;
		
    p_item->session_priority = ppa_get_session_priority( ppa_buf );
    g_ppa_def_prio_thresh = ppa_get_def_prio_thresh(0);

    // High priority can directly be added to acceleration
    if ( p_item->session_priority  > g_ppa_def_prio_thresh)
	  return PPA_SUCCESS;

    g_ppa_low_prio_data_rate = ppa_get_low_prio_data_rate(0);
    g_ppa_def_prio_data_rate = ppa_get_def_prio_data_rate(0);

    // Do the math only if any one one of the data checks is enabled
    if ( ( g_ppa_low_prio_data_rate != 0) || (g_ppa_def_prio_data_rate != 0 ) ) {
      PPA_TIMESPEC sleep_time;  
      ppa_get_monotonic(&after);
      sleep_time = ppa_timespec_sub(after, p_item->timespent);
      time_msec = (uint64_t ) ppa_timespec_to_ns(&sleep_time); 
      wait_time_const = (uint64_t) ppa_timespec_to_ns(&wait_time);
      // Convert bytes to bits = Multiply by 8 = Left shift by 3
      total_bytes = (uint64_t) (p_item->ewma_session_bytes << 3) * NSEC_PER_SEC; 

      // EWMA calculation: We give 1/4 to current value and 3/4 to previous value. Such ratios allow shift operations
      if ( p_item->ewma_bytes == 0 ) {
        p_item->ewma_bytes = total_bytes; 
        p_item->ewma_time = time_msec;
        if ( ppa_get_pkt_ip_proto(ppa_buf) == PPA_IPPROTO_TCP )
          multiply_offset = ppa_get_tcp_initial_offset(0);
      } else {
        p_item->ewma_bytes = ( total_bytes + (p_item->ewma_bytes * 3)) >> 2;
        p_item->ewma_time = ( time_msec + (p_item->ewma_time * 3)) >> 2;
        if ( ppa_get_pkt_ip_proto(ppa_buf) == PPA_IPPROTO_TCP )
          multiply_offset = ppa_get_tcp_steady_offset(0);
      }

      total_bytes = p_item->ewma_bytes;
      time_msec = p_item->ewma_time;

      if ( flags == 1 ) // Updated moving average bytes and time. Return.
        return PPA_SUCCESS;
      if(!p_item->mult_factor) 
        p_item->mult_factor++;

      new_wait_time_const =  p_item->mult_factor*wait_time_const;
      if( time_msec >  new_wait_time_const ) { // For each Min time interval check data rate of session
        p_item->mult_factor++;
      
        total_bytes = div64_u64( total_bytes,new_wait_time_const ); //do_div() was failing intermittently so using div64_u64()		
        // Data rate is EWMA of bytes divided by EWMA of time
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                 "Session data rate=%llu bytes=%llu time=%llu prio=%d low_thresh=%d def_thresh=%d low_rate=%u def_rate=%u\n",
                 total_bytes,p_item->ewma_bytes,time_msec,
                 p_item->session_priority,g_ppa_low_prio_thresh,
                 g_ppa_def_prio_thresh,g_ppa_low_prio_data_rate,g_ppa_def_prio_data_rate);

        // We ignore first few packets to negate the bursty nature of traffic which wil give us wrong seed to start with for EWMA
        g_ppa_low_prio_thresh = ppa_get_low_prio_thresh(0);
        // If low priority session does not meet its threshold data rate return failure.
        if ( p_item->session_priority <= g_ppa_low_prio_thresh && total_bytes <= ( g_ppa_low_prio_data_rate * multiply_offset) )
        return PPA_FAILURE;

        // If default priority session does not meet its threshold data rate return failure.
        if ( p_item->session_priority > g_ppa_low_prio_thresh && total_bytes <= ( g_ppa_def_prio_data_rate * multiply_offset))
            return PPA_FAILURE;
      } 
      else 
      {
          return PPA_FAILURE;
      }

    }

    // Either data rate checks are not enabled or session has passed all criteria. Can be added to acceleration hence return success
    return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_session_pass_criteria);

void add_pitem_node(struct session_list_item *p_item, enum ppa_engine_type engine)
{
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	if(engine == PAE || engine == MPE) {
    	if (ppa_hsel_add_routing_session(p_item, 0) != PPA_SUCCESS ) {
			p_item->flag2 |= SESSION_FLAG2_ADD_HW_FAIL;  //PPE hash full in this hash index, or IPV6 table full ,..
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_hw_add_session(p_item) fail\n");
        }
#else
	if(engine == PPE) {
		if(ppa_hw_add_session(p_item, 0) != PPA_SUCCESS ) {
                p_item->flag2 |= SESSION_FLAG2_ADD_HW_FAIL;
		}
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
	} else if (engine == SWAC) {
//		ppa_sw_add_session(ppa_buf, p_item);
#else
	} else { 
		return;
#endif
	}
}	

void delete_pitem_node(struct session_list_item *p_item, enum ppa_engine_type engine)
{
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	if(engine == PAE || engine == MPE) {
		ppa_hsel_del_routing_session(p_item);
#else
	if(engine == PPE ) {
		ppa_hw_del_session(p_item);
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
	} else if (engine == SWAC) {
  		update_session_mgmt_stats(p_item, DELETE);
		p_item->flags &= ~SESSION_ADDED_IN_SW;
#else
	} else { 
		return;
#endif
	}
}

static void do_switch(void) 
{
	struct session_list_item *p_item, *start;
	int index;
	int session_type,session_prio,engine;
	int other_count=0, high_count=0, swap_count;
	
	ppa_session_list_lock();

	for(session_type=0; session_type<MAX_SESSION_TYPE; session_type++) {
		for(engine=0;engine<MAX_DATA_FLOW_ENGINES;engine++){
			other_count = 0;
			high_count = 0;
			for(session_prio=1;session_prio<MAX_SESSION_PRIORITY;session_prio++)
				other_count+=session_count[session_type][session_prio][engine];
			for(session_prio=engine+1;session_prio<MAX_DATA_FLOW_ENGINES;session_prio++)
				high_count+=session_count[session_type][HIGH-1][session_prio];

			if(other_count > high_count)
				swap_count = high_count;
			else
				swap_count = other_count;
		
			if(swap_count > 0) {
				index = 1;
				for(session_prio=MAX_SESSION_PRIORITY-1;session_prio>0;session_prio--) {
			        ppa_list_for_each_entry_safe_reverse(p_item, start, &session_list_head[session_type][session_prio][engine], priority_list) {
            			if(index++ > swap_count)
		                	break;
						delete_pitem_node(p_item,engine);	
			        }
				}
				index = 1;
				for(session_prio=MAX_DATA_FLOW_ENGINES-1;session_prio>engine;session_prio--) {
					ppa_list_for_each_entry_safe_reverse(p_item, start, &session_list_head[session_type][engine][session_prio], priority_list) {
						if(index++ > swap_count)
							break;
						delete_pitem_node(p_item,session_prio);
						add_pitem_node(p_item,engine);	
					}
				}
			}
		}
	}

	ppa_session_list_unlock();

}

uint32_t ppa_reset_ewma_stats( struct session_list_item *p_item,uint32_t flags)
{
	p_item->ewma_session_bytes = 0;
	p_item->ewma_num_adds = 1;
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_reset_ewma_stats);

uint32_t ppa_update_ewma(PPA_BUF *ppa_buf,struct session_list_item *p_item,uint32_t flags)
{
		if ( ppa_get_session_limit_enable(0) ) {
			ppa_session_pass_criteria(ppa_buf,p_item,1); // Just update moving average bytes and time
			ppa_reset_ewma_stats(p_item,0);
		}
		return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_update_ewma);
uint32_t ppa_session_update_collisions(uint32_t flags)
{
    g_ppa_lan_current_collisions =  ppa_num_collision_sessions(0);
    g_ppa_wan_current_collisions =  ppa_num_collision_sessions(1);
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_session_update_collisions);

uint32_t ppa_swap_sessions(uint32_t flags)
{
	if ( ! ppa_get_session_limit_enable(0) )
		return PPA_SUCCESS;
	do_switch();
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_swap_sessions);

uint32_t ppa_session_store_ewma( PPA_BUF *ppa_buf, struct session_list_item *p_item,uint32_t flags)
{
    p_item->ewma_session_bytes   += ppa_buf->len + PPA_ETH_HLEN + PPA_ETH_CRCLEN;
    p_item->ewma_num_adds ++;
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_session_store_ewma);

uint32_t ppa_decide_collision(struct session_list_item *p_item) {
	if ( ! ppa_get_session_limit_enable(0) ) // Just return in case session limit is not enabled
		return PPA_SUCCESS;
	if ( p_item->flags & SESSION_LAN_ENTRY ) {
		PPA_MAX_ENTRY_INFO entry={0}; /*!< max entry info*/
		if (ppa_drv_get_max_entries(&entry, 0) != PPA_FAILURE ) {
		    	if ( p_item->session_priority <= g_ppa_def_prio_thresh  && ((entry.max_lan_collision_entries - g_ppa_lan_current_collisions) <= g_ppa_lan_reserve_collisions)) 
				p_item->collision_flag = 2; // This session should not be added to collision table in case hash table is already full
        			ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"set collision to 2, p_item = 0x%08x\n", (u32)p_item);
				return PPA_SUCCESS;
		} else {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"could not get max entries for lan entry!!\n");
			return PPA_FAILURE;
		}
	} else {
		PPA_MAX_ENTRY_INFO entry={0}; /*!< max entry info*/
		if (ppa_drv_get_max_entries(&entry, 0) != PPA_FAILURE ) {
		    	if ( p_item->session_priority <= g_ppa_def_prio_thresh && ((entry.max_wan_collision_entries - g_ppa_wan_current_collisions) <= g_ppa_wan_reserve_collisions)) 
				p_item->collision_flag = 2; // This session should not be added to collision table in case hash table is already full
        			ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"set collision to 2, p_item = 0x%08x\n", (u32)p_item);
				return PPA_SUCCESS;
		} else {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"could not get max entries for wan entry!!\n");
			return PPA_FAILURE;
		}
	}
}
EXPORT_SYMBOL(ppa_decide_collision);

uint32_t ppa_session_limit_update_flags(struct session_list_item *p_item,uint32_t flags)
{
		/* Criteria for setting SESSION_NOT_ACCEL_FOR_MGM:
		(1) If session limit is not enabled
		(2) Extra info flags has already flag set ( coming from ppacmd modifysession )
		if we set this flag, packet will go through linux stack and hit ppa_speed_handle_frame and return at postrouting stage.
		Note: The ppasessmgmtd will never set this flag though extra info flags*/
		if ( ! ppa_get_session_limit_enable(0) || ( flags & SESSION_NOT_ACCEL_FOR_MGM) ) 
	            p_item->flags |= SESSION_NOT_ACCEL_FOR_MGM;
		return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_session_limit_update_flags);

uint32_t  ppa_session_record_time(PPA_BUF *ppa_buf,struct session_list_item *p_item,uint32_t flags)
{
	PPA_TIMESPEC before;
	if ( ppa_get_session_limit_enable(0) ) {
		if ( p_item->ewma_num_adds == 1 ) { // checking for one cause for each session for first packet ppa_session_store_ewma() will get called 
											//comes to this point and there we are increamenting count of ewma_num_adds. 
		    ppa_get_monotonic(&before);
		    p_item->timespent.tv_nsec = before.tv_nsec;
		    p_item->timespent.tv_sec = before.tv_sec;
		}
        	p_item->ewma_num_adds++;
	        p_item->ewma_session_bytes += ppa_buf->len + PPA_ETH_HLEN + PPA_ETH_CRCLEN;
	} else { // reset all ewma parameters
		    p_item->timespent.tv_nsec = 0;
		    p_item->timespent.tv_sec = 0;
        	    p_item->ewma_num_adds = 0;
	            p_item->ewma_session_bytes = 0;
	}
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_session_record_time);

uint32_t ppa_init_session_limit_params(PPA_INIT_INFO *p_info,uint32_t flags)
{
    if( p_info->add_requires_lan_collisions ) 
        g_ppa_lan_reserve_collisions = p_info->add_requires_lan_collisions;

    if( p_info->add_requires_wan_collisions )
        g_ppa_wan_reserve_collisions = p_info->add_requires_wan_collisions;
    
    g_ppa_low_prio_data_rate = ppa_get_low_prio_data_rate(0);
    g_ppa_def_prio_data_rate = ppa_get_def_prio_data_rate(0);
    g_ppa_low_prio_thresh = ppa_get_low_prio_thresh(0);
    g_ppa_def_prio_thresh = ppa_get_def_prio_thresh(0);
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_init_session_limit_params);

uint32_t ppa_num_collision_sessions(uint32_t flag)
{
    struct session_list_item *pp_item;
    uint32_t pos = 0;
    uint32_t num_collision_sessions = 0;

    if ( ppa_session_start_iteration(&pos, &pp_item) == PPA_SUCCESS )
    {
        do
        {   
	    if (pp_item->collision_flag == 1 && ( pp_item->flags & ( flag ? SESSION_WAN_ENTRY: SESSION_LAN_ENTRY) ) )
		num_collision_sessions++;
        } while ( ppa_session_iterate_next(&pos, &pp_item) == PPA_SUCCESS  );
    }
    
    ppa_session_stop_iteration();

    return (num_collision_sessions);
}
EXPORT_SYMBOL(ppa_num_collision_sessions);

int32_t ppa_session_prio(PPA_SESSION *p_session, uint32_t flags)
{
    uint32_t flag_template[2] = {PPA_F_SESSION_ORG_DIR, PPA_F_SESSION_REPLY_DIR};
    struct session_list_item *p_item;
    uint32_t index;
    uint32_t priority = 0;

    ppa_session_list_lock();

    for ( index = 0; index < NUM_ENTITY(flag_template); index++ )
    {
     
        if ( !(flags & flag_template[index]) )
            continue;

        //  index = 0, org dir, index = 1, reply dir
      if ( __ppa_session_find_by_ct(p_session, index, &p_item) != PPA_SESSION_EXISTS )
            continue;
      priority = p_item->session_priority;
      __ppa_session_put(p_item);
      break;
    }
    ppa_session_list_unlock();
    return priority;
}
