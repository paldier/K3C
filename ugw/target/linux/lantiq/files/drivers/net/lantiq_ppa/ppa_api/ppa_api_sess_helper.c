/*******************************************************************************
**
** FILE NAME    : ppa_api_sess_helper.c
** PROJECT      : PPA
** MODULES      : PPA API - Routing/Bridging(flow based) helper routines
**
** DATE         : 24 Feb 2015
** AUTHOR       : Mahipati Deshpande
** DESCRIPTION  : PPA Protocol Stack Hook API Session Operation Functions
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 24 Feb 2015  Mahipati               The helper functions are moved from 
**                                     ppa_api_session.c
*******************************************************************************/

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#include <linux/swap.h>
#include <net/ppa_api.h>
#if defined(CONFIG_LTQ_DATAPATH) && CONFIG_LTQ_DATAPATH
#include <net/datapath_api.h>
#endif
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#include "ppa_api_session_limit.h"
#endif
#include "ppa_api_mib.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_hal_selector.h"
#include "ppa_api_tools.h"
#include <net/ppa_stack_al.h>
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif
#include "ppa_api_sess_helper.h"

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
#include "ppa_sae_hal.h"
#endif

//  routing session list item operation
static INLINE void ppa_session_list_init_item(struct session_list_item *);
//static INLINE void ppa_session_list_free_item(struct session_list_item *);

static PPA_LOCK         g_session_list_lock;
PPA_HLIST_HEAD          g_session_list_hash_table[SESSION_LIST_HASH_TABLE_SIZE];
static PPA_MEM_CACHE    *g_session_item_cache = NULL;

#define PPA_SESSION_RCU_LIST

#ifdef PPA_SESSION_RCU_LIST
#define PPA_SESSION_LIST_ADD                  ppa_hlist_add_head_rcu 
#define PPA_SESSION_LIST_DEL                  ppa_hlist_del_rcu
#define PPA_SESSION_LIST_FOR_EACH_NODE        ppa_hlist_for_each_rcu
#define PPA_SESSION_LIST_FOR_EACH_ENTRY       ppa_hlist_for_each_entry_safe
#define PPA_SESSION_LIST_FOR_EACH_ENTRY_READ  ppa_hlist_for_each_entry_rcu
#define list_get_first_node(head)   (rcu_dereference(hlist_first_rcu((head)))) 
#define list_get_next_node(node)    (rcu_dereference(hlist_next_rcu((node))))
#else
#define PPA_SESSION_LIST_ADD                  ppa_hlist_add_head 
#define PPA_SESSION_LIST_DEL                  ppa_hlist_del
#define PPA_SESSION_LIST_FOR_EACH_NODE        ppa_hlist_for_each
#define PPA_SESSION_LIST_FOR_EACH_ENTRY       ppa_hlist_for_each_entry_safe
#define PPA_SESSION_LIST_FOR_EACH_ENTRY_READ  ppa_hlist_for_each_entry
#define list_get_first_node(head)   ((head)->first)
#define list_get_next_node(node)    ((node)->next)
#endif

#if 0
/*
 * Final intention is to create the list as defined below
 */
struct session_list {
  PPA_HLIST_NODE hlist;
  PPA_ATOMIC used;
  PPA_TIMER timer;
  session_list_item item; /* or object */
}
#endif


int ppa_session_list_lock_init(void)
{
  return  ppa_lock_init(&g_session_list_lock);
}

int ppa_session_list_lock_destroy(void)
{
  return  ppa_lock_init(&g_session_list_lock);
}

void ppa_session_list_lock(void)
{
  ppa_lock_get(&g_session_list_lock);
}
//EXPORT_SYMBOL(ppa_session_list_lock);

void ppa_session_list_unlock(void)
{
  ppa_lock_release(&g_session_list_lock);
}
//EXPORT_SYMBOL(ppa_session_list_unlock);


void ppa_session_list_read_lock(void)
{
  /*
   * If session list is RCU list, then call ppa_rcu_read_lock
   */
#ifdef PPA_SESSION_RCU_LIST
  ppa_rcu_read_lock();
#else
  ppa_lock_get(&g_session_list_lock);
#endif
}
//EXPORT_SYMBOL(ppa_session_list_read_lock);

void ppa_session_list_read_unlock(void)
{
  /*
   * If session list is RCU list, then call ppa_rcu_read_unlock
   */
#ifdef PPA_SESSION_RCU_LIST
  ppa_rcu_read_unlock();
#else
  ppa_lock_release(&g_session_list_lock);
#endif
}
//EXPORT_SYMBOL(ppa_session_list_read_unlock);

int32_t ppa_session_cache_shrink(void)
{
  return ppa_kmem_cache_shrink(g_session_item_cache);
}

int32_t ppa_session_cache_create(void)
{
    
  return ppa_mem_cache_create( "ppa_session_item", 
                               sizeof(struct session_list_item), 
                               &g_session_item_cache);
}

void ppa_session_cache_destroy(void)
{
  if( g_session_item_cache ) {
    ppa_mem_cache_destroy(g_session_item_cache);
    g_session_item_cache = NULL;
  }
}

int ppa_session_list_init(void)
{
  int i;

  for(i = 0; i < SESSION_LIST_HASH_TABLE_SIZE; i ++) {
    PPA_INIT_HLIST_HEAD(&g_session_list_hash_table[i]);
  }
  return PPA_SUCCESS;
}

void ppa_session_list_free(void)
{
  PPA_HLIST_NODE *tmp;
  struct session_list_item *p_item;
  uint32_t idx;

  ppa_session_list_lock();

  for(idx = 0; idx < SESSION_LIST_HASH_TABLE_SIZE; idx ++) {
    PPA_SESSION_LIST_FOR_EACH_ENTRY( p_item, tmp, &g_session_list_hash_table[idx], hlist)
    {
      __ppa_session_delete_item(p_item);
    }
  }
  
  ppa_session_list_unlock();
}

static INLINE void ppa_session_list_init_item(struct session_list_item *p_item)
{
  ppa_memset(p_item, 0, sizeof(*p_item));
  //ppa_lock_init(&p_item->uc_lock);
  PPA_INIT_HLIST_NODE(&p_item->hlist);
  p_item->mtu             = g_ppa_ppa_mtu;
  p_item->routing_entry   = ~0;
  p_item->pppoe_entry     = ~0;
  p_item->mtu_entry       = ~0;
  p_item->src_mac_entry   = ~0;
  p_item->out_vlan_entry  = ~0;
  p_item->tunnel_idx      = ~0;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
  p_item->l2tptunnel_idx  = ~0;
#endif
}

struct session_list_item* ppa_session_alloc_item(void)
{
  struct session_list_item *p_item;

  p_item = ppa_mem_cache_alloc(g_session_item_cache);
  if ( p_item )
      ppa_session_list_init_item(p_item);

  return p_item;
}

static void ppa_session_rcu_free(struct rcu_head *rp)
{
  struct session_list_item *p_item;

  p_item = container_of(rp, struct session_list_item, rcu);
  ppa_mem_cache_free(p_item, g_session_item_cache);
}

void ppa_session_list_free_item(struct session_list_item *p_item)
{
#if defined(SESSION_STATISTIC_DEBUG) && SESSION_STATISTIC_DEBUG 
  PPE_IPV6_INFO ip6_info;
  if( p_item->src_ip6_index )
  {
    ip6_info.ipv6_entry = p_item->src_ip6_index - 1;
    ppa_drv_del_ipv6_entry(&ip6_info, 0);
  }
  if( p_item->dst_ip6_index )
  {
    ip6_info.ipv6_entry = p_item->dst_ip6_index - 1;
    ppa_drv_del_ipv6_entry(&ip6_info, 0);
  }
#endif

#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO
    if(p_item->flag2 & SESSION_FLAG2_LRO) {
	if(ppa_del_lro_entry(p_item)!=PPA_SUCCESS) {
	    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"lro entry delete failed\n");
	}
	goto __DELETE_SUCCESS;
    }
#endif

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
  ppa_hsel_del_routing_session(p_item);
#else
  ppa_hw_del_session(p_item);
#endif

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
  if ( (p_item->flags & SESSION_ADDED_IN_SW) ) {
    sw_del_session_mgmt_stats(p_item);
  }
#endif
  ppa_sw_del_session(p_item);
#endif

#if defined CONFIG_LTQ_PPA_MPE_HAL || defined CONFIG_LTQ_PPA_MPE_HAL_MODULE
  ppa_session_destroy_tmplbuf(p_item);
#endif

__DELETE_SUCCESS:
#ifdef PPA_SESSION_RCU_LIST
  ppa_call_rcu(&p_item->rcu, ppa_session_rcu_free);
#else
  ppa_mem_cache_free(p_item, g_session_item_cache);
#endif
}

static INLINE uint32_t ppa_session_get_index(uint32_t key)
{
	return ((u64)key* (SESSION_LIST_HASH_TABLE_SIZE)) >> 32;
}

/* Must be inside write lock */
void __ppa_session_insert_item(struct session_list_item *p_item)
{
  uint32_t idx;

  idx = ppa_session_get_index(p_item->hash);
  //ppa_session_print(p_item);
  PPA_SESSION_LIST_ADD(&p_item->hlist, &g_session_list_hash_table[idx]);
}

/* Must be inside write lock */
void __ppa_session_delete_item(struct session_list_item *p_item)
{
  PPA_SESSION_LIST_DEL(&p_item->hlist);
  //set dying bit...usefull for timer handling
  //ppa_session_print(p_item);
  if( ppa_timer_pending(&p_item->timer) ) {
    ppa_timer_del(&p_item->timer);
    ppa_atomic_dec_and_test(&p_item->used);
  }

  __ppa_session_put(p_item);
}

uint32_t ppa_session_get_routing_count(uint16_t bf_lan, uint32_t count_flag, uint32_t hash_index)
{
  struct session_list_item *p_item;
  uint32_t i;
  uint32_t count = 0, start_pos=0;
  uint32_t session_flag;

  if( hash_index ) 
  {
      start_pos = hash_index -1;        
  }
      
  if( bf_lan == 0 )        
  {
      session_flag = SESSION_WAN_ENTRY;
  }
  else if ( bf_lan == 1 )
  {
      session_flag = SESSION_LAN_ENTRY;
  }
  else if ( bf_lan == 2 )  /*non lan/wan, it means unknow session */
  {          
      ppa_session_list_read_lock();
      for(i = start_pos; i < SESSION_LIST_HASH_TABLE_SIZE; i ++)
      {
          ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"i=%d\n", i);
          PPA_SESSION_LIST_FOR_EACH_ENTRY_READ(p_item,&g_session_list_hash_table[i],hlist)
          {                
  
            if( !(p_item->flags & SESSION_LAN_ENTRY) && !(p_item->flags & SESSION_WAN_ENTRY) )
                  count++;               
               
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"p_item=%p with index=%u count=%u\n", p_item, i, count);
          }
          if( hash_index ) break;
      }
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_get_non_lan_wan_routing_session_count=%d\n", count);
      ppa_session_list_read_unlock();
      return count;
  }
  else
  {
    critial_err("wrong bf_flab value:%u\n", bf_lan);
    return 0;
  }
  
  ppa_session_list_read_lock();

  for(i = start_pos; i < SESSION_LIST_HASH_TABLE_SIZE; i ++) {

    PPA_SESSION_LIST_FOR_EACH_ENTRY_READ(p_item,&g_session_list_hash_table[i],hlist) {
    
      if(p_item->flags & session_flag){

        if( count_flag == 0 ||/* get all PPA sessions with acceleratted and non-accelearted  */
            ( (count_flag == SESSION_ADDED_IN_HW) && (p_item->flags & SESSION_ADDED_IN_HW) )  || /*get all accelerated sessions only */
            ( (count_flag == SESSION_ADDED_IN_SW) && (p_item->flags & SESSION_ADDED_IN_SW) )  || /*get all software accelerated sessions only */
            ( (count_flag == SESSION_NON_ACCE_MASK) && !(p_item->flags & SESSION_ADDED_IN_HW ) && !(p_item->flags & SESSION_ADDED_IN_SW )) ){
                  count++;
                  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_session_get_routing_count=%d\n", count);
            }

      }
    }
    if( hash_index ) break;
  }

  ppa_session_list_read_unlock();
  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_session_get_routing_count=%d with count_flag=%x\n", count, count_flag);

  return count;
}
EXPORT_SYMBOL(ppa_session_get_routing_count);

void ppa_session_delete_by_netif(PPA_IFNAME *ifname)
{
  uint32_t idx;
  struct session_list_item *p_item = NULL;
  PPA_HLIST_NODE *tmp;

  ppa_session_list_lock();
  for ( idx = 0; idx < SESSION_LIST_HASH_TABLE_SIZE; idx++ )
  {

    PPA_SESSION_LIST_FOR_EACH_ENTRY(p_item, tmp, &g_session_list_hash_table[idx], hlist) {

      if( ppa_is_netif_name(p_item->rx_if, ifname) || 
          ppa_is_netif_name(p_item->tx_if, ifname) ) {
              
        __ppa_session_delete_item(p_item);
      }
    }
  }
  ppa_session_list_unlock();
}

#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
void ppa_session_delete_by_subif(PPA_DP_SUBIF *subif)
{
    uint32_t idx;
    struct session_list_item *p_item = NULL;
    PPA_HLIST_NODE *tmp;

    ppa_session_list_lock();
    for ( idx = 0; idx < SESSION_LIST_HASH_TABLE_SIZE; idx++ )
    {
        PPA_SESSION_LIST_FOR_EACH_ENTRY(p_item, tmp, &g_session_list_hash_table[idx], hlist)
        {
            if ( (p_item->dest_ifid == subif->port_id) && (p_item->dest_subifid == subif->subif) ){
                __ppa_session_delete_item(p_item);
            }
        }
    }
    ppa_session_list_unlock();
    
}

void ppa_session_delete_by_macaddr(uint8_t *mac)
{
  uint32_t idx;
  struct session_list_item *p_item = NULL;
  PPA_HLIST_NODE *tmp;

  ppa_session_list_lock();
  for ( idx = 0; idx < SESSION_LIST_HASH_TABLE_SIZE; idx++ )
  {
    PPA_SESSION_LIST_FOR_EACH_ENTRY(p_item, tmp, &g_session_list_hash_table[idx], hlist) {
     
      if ( 0 == ppa_memcmp(p_item->dst_mac, mac, sizeof(p_item->dst_mac)) ||
           0 == ppa_memcmp(p_item->src_mac, mac, sizeof(p_item->src_mac)) ) {

        __ppa_session_delete_item(p_item);
      }
      
    }
  }
  ppa_session_list_unlock();
}
#endif

static INLINE int ppa_compare_with_tuple(const PPA_TUPLE *t1, 
                                          struct session_list_item *t2)
{
  unsigned short l3num;

  l3num = (t2->flags & SESSION_IS_IPV6)?AF_INET6:AF_INET;

  if( t1->src.l3num == l3num &&
      t1->dst.protonum == t2->ip_proto &&
      t1->src.u.all == t2->src_port &&
	  t1->dst.u.all == t2->dst_port ) {
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE 
      if (t1->src.u3.all[0] == t2->src_ip.ip6[0] &&
			t1->src.u3.all[1] == t2->src_ip.ip6[1] &&
			t1->src.u3.all[2] == t2->src_ip.ip6[2] &&
			t1->src.u3.all[3] == t2->src_ip.ip6[3] &&
			t1->dst.u3.all[0] == t2->dst_ip.ip6[0] &&
			t1->dst.u3.all[1] == t2->dst_ip.ip6[1] &&
			t1->dst.u3.all[2] == t2->dst_ip.ip6[2] &&
			t1->dst.u3.all[3] == t2->dst_ip.ip6[3] ) 
#else
	 if (t1->src.u3.all[0] == t2->src_ip.ip &&
			t1->dst.u3.all[0] == t2->dst_ip.ip )	
#endif
	{
      return 1;
    }
  }

  return 0;
}
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
static INLINE int ppa_compare_with_skb(PPA_BUF* skb, 
                                          struct session_list_item *t2)
{
  unsigned short l3num;

  l3num = (t2->flags & SESSION_IS_IPV6)?AF_INET6:AF_INET;

    	PPA_IPADDR src_addr,dst_addr;
        src_addr.ip6[0] =0x0;
        src_addr.ip6[1] =0x0;
        src_addr.ip6[2] =0x0;
        src_addr.ip6[3] =0x0;

        dst_addr.ip6[0] =0x0;
        dst_addr.ip6[1] =0x0;
        dst_addr.ip6[2] =0x0;
        dst_addr.ip6[3] =0x0;

    	src_addr =  ppa_get_pkt_src_ip(skb);
    	dst_addr =  ppa_get_pkt_dst_ip(skb);


  //if( t1->src.l3num == l3num &&
    if( ppa_get_pkt_ip_proto(skb)== t2->ip_proto &&
      ppa_get_pkt_src_port(skb) == t2->src_port &&
      (src_addr.ip6[0] == t2->src_ip.ip6[0])) { 
      /* (src_addr.ip6[0] == t2->src_ip.ip6[0] &&
      src_addr.ip6[1] == t2->src_ip.ip6[1] &&
      src_addr.ip6[2] == t2->src_ip.ip6[2] &&
      src_addr.ip6[3] == t2->src_ip.ip6[3]) ) { */

    if( ppa_get_pkt_dst_port(skb) == t2->dst_port && 
        dst_addr.ip6[0] == t2->dst_ip.ip6[0] ) {
        /* dst_addr.ip6[0] == t2->dst_ip.ip6[0] &&
        dst_addr.ip6[1] == t2->dst_ip.ip6[1] &&
        dst_addr.ip6[2] == t2->dst_ip.ip6[2] &&
        dst_addr.ip6[3] == t2->dst_ip.ip6[3] ) { */
      return 1;
    }
  }
  return 0;
}


/*
 * Find the session from skb. 
 */
int ppa_find_session_for_ipsec(PPA_BUF* skb, uint8_t pf, struct session_list_item **pp_item)
{
  uint32_t hash;
  PPA_TUPLE tuple;
  struct session_list_item *p_item;
  uint32_t index;
  int ret;
  
  if(ppa_get_hash_from_packet(skb,pf, &hash, &tuple))
    return PPA_SESSION_NOT_ADDED;

  ret = PPA_SESSION_NOT_ADDED;

  index = ppa_session_get_index(hash);
  
  ppa_session_list_read_lock();
  PPA_SESSION_LIST_FOR_EACH_ENTRY_READ(p_item, 
                                   (g_session_list_hash_table+index), hlist) {

    if(p_item->hash == hash && ppa_compare_with_skb(skb,p_item) ) {

    if ( p_item->routing_entry != 0xFFFFFFFF) { 
	if(__ppa_search_ipsec_rtindex(p_item) == PPA_SUCCESS) 	      {
    		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,%d\n",__FUNCTION__,__LINE__);
			   
      if( !ppa_atomic_inc_not_zero(&p_item->used) )
        break;

      ret = PPA_SESSION_EXISTS;
      *pp_item = p_item;
      break;
    }
   }
   else
   {
        if( !ppa_atomic_inc_not_zero(&p_item->used) )
        break;


      ret = PPA_SESSION_EXISTS;
      *pp_item = p_item;
      break;
   }

  }
  }
  ppa_session_list_read_unlock();

  return ret;
}
EXPORT_SYMBOL(ppa_find_session_for_ipsec);


#endif


/*
 * Search the session from sbk. Should be called when lock is held
 */
int __ppa_find_session_from_skb(PPA_BUF* skb, uint8_t pf, struct session_list_item **pp_item)
{
  uint32_t hash;
  PPA_TUPLE tuple;
  struct session_list_item *p_item;
  uint32_t index;
  int ret;
  PPA_HLIST_NODE *tmp;
  
  if(ppa_get_hash_from_packet(skb,pf, &hash, &tuple))
    return PPA_SESSION_NOT_ADDED;

  ret = PPA_SESSION_NOT_ADDED;

  index = ppa_session_get_index(hash);
  
  PPA_SESSION_LIST_FOR_EACH_ENTRY(p_item,tmp, (g_session_list_hash_table+index), hlist) {

    if(p_item->hash == hash && ppa_compare_with_tuple(&tuple,p_item) ) {
			   
      if( !ppa_atomic_inc_not_zero(&p_item->used) )
        break;

      ret = PPA_SESSION_EXISTS;
      *pp_item = p_item;
      break;
    }
  }

  return ret;
}

/*
 * Find the session from skb. 
 */
int ppa_find_session_from_skb(PPA_BUF* skb, uint8_t pf, struct session_list_item **pp_item)
{
  uint32_t hash;
  PPA_TUPLE tuple;
  struct session_list_item *p_item;
  uint32_t index;
  int ret;
  
  if(ppa_get_hash_from_packet(skb,pf, &hash, &tuple))
    return PPA_SESSION_NOT_ADDED;

  ret = PPA_SESSION_NOT_ADDED;

  index = ppa_session_get_index(hash);
  
  ppa_session_list_read_lock();
  PPA_SESSION_LIST_FOR_EACH_ENTRY_READ(p_item, 
                                   (g_session_list_hash_table+index), hlist) {

    if(p_item->hash == hash && ppa_compare_with_tuple(&tuple,p_item) ) {
			   
      if( !ppa_atomic_inc_not_zero(&p_item->used) )
        break;

      ret = PPA_SESSION_EXISTS;
      *pp_item = p_item;
      break;
    }
  }
  ppa_session_list_read_unlock();

  return ret;
}
EXPORT_SYMBOL(ppa_find_session_from_skb);

/*
 * Search the session by ct when hash is known. Must be called within lock
 */
int __ppa_session_find_ct_hash(const PPA_SESSION *p_session, 
                               uint32_t hash, 
                               struct session_list_item **pp_item)
{
  struct session_list_item *p_item;
  PPA_HLIST_NODE *tmp;
  uint32_t index;
  int ret = PPA_SESSION_NOT_ADDED;

  index = ppa_session_get_index(hash);

  *pp_item = NULL;
  
  PPA_SESSION_LIST_FOR_EACH_ENTRY(p_item,tmp, (g_session_list_hash_table+index), hlist) {

    if(p_item->hash == hash && p_item->session == p_session ) {
			   
      if( !ppa_atomic_inc_not_zero(&p_item->used) )
        break;

      ret = PPA_SESSION_EXISTS;
      *pp_item = p_item;
      break;
    }
  }

  return ret;
}

/*
 * This function searches session using hash and connection track pointer.
 * Call this function outside the PRE/POST routing hooks. 
 * LOCK should be held.
 * The hash is computed using connection track's tuple
 */
int32_t __ppa_session_find_by_ct(PPA_SESSION *p_session, 
                             uint32_t is_reply, 
                             struct session_list_item **pp_item)
{
  uint32_t hash;
  PPA_TUPLE tuple;

  if(p_session) {
    hash = ppa_get_hash_from_ct(p_session, is_reply?1:0,&tuple);
    return __ppa_session_find_ct_hash(p_session, hash, pp_item);
  }
  return PPA_SESSION_NOT_ADDED; 
}

/*
 * This function searches session using hash and connection track pointer.
 * Call this function outside the PRE/POST routing hooks.
 */
int32_t ppa_session_find_by_ct(PPA_SESSION *p_session, 
                             uint32_t is_reply, 
                             struct session_list_item **pp_item)
{
  uint32_t ret ;

  ppa_session_list_lock();
  ret = __ppa_session_find_by_ct(p_session,is_reply,pp_item);
  ppa_session_list_unlock();

  return ret;
}

/*
 * This function searches the session using tuple from connection track 
 * The tuple is taken from ct.
 */
int32_t ppa_session_find_by_tuple( PPA_SESSION *p_session, 
                                   uint32_t is_reply, 
                                   struct session_list_item **pp_item)
{
  uint32_t hash;
  PPA_TUPLE tuple;
  struct session_list_item *p_item;
  uint32_t index;
  int ret;
  
  if(!p_session) {
    return PPA_SESSION_NOT_ADDED;
  }

  hash = ppa_get_hash_from_ct(p_session, is_reply?1:0,&tuple);
  ret = PPA_SESSION_NOT_ADDED;
  index = ppa_session_get_index(hash);
  
  ppa_session_list_read_lock();
  PPA_SESSION_LIST_FOR_EACH_ENTRY_READ(p_item, 
                                  (g_session_list_hash_table+index), hlist) {

    if(p_item->hash == hash && ppa_compare_with_tuple(&tuple,p_item) ) {
			   
      if( !ppa_atomic_inc_not_zero(&p_item->used) )
        break;

      ret = PPA_SESSION_EXISTS;
      *pp_item = p_item;
#if 0 
      printk("%s session::prot=%d src:%pI4 p=%d dst:%pI4 p=%d\n",
           __FUNCTION__,
           p_item->ip_proto,
           &p_item->src_ip,p_item->src_port,
           &p_item->dst_ip,p_item->dst_port);
#endif
      break;
    }
  }
  ppa_session_list_read_unlock();
  return ret; 
}
//EXPORT_SYMBOL(ppa_session_find_by_tuple);

struct session_list_item* __ppa_session_find_by_routing_entry( uint32_t routingEntry)
{
  uint32_t idx;
  struct session_list_item *p_item = NULL;
  PPA_HLIST_NODE *tmp;

  for(idx = 0; idx < SESSION_LIST_HASH_TABLE_SIZE; idx ++) {
    PPA_SESSION_LIST_FOR_EACH_ENTRY( p_item, tmp, &g_session_list_hash_table[idx], hlist)
    {
      if(p_item->routing_entry == routingEntry) {
        if( !ppa_atomic_inc_not_zero(&p_item->used) )
          return NULL;

        return p_item;
      }
    }
  }
          
  return NULL;
}

/*
 * Put the session back. If reference count is zero, then session is freed.
 * LOCK should be held
 */
void __ppa_session_put(struct session_list_item* p_item)
{
  //ppa_session_print(p_item);
  if(p_item && ppa_atomic_dec_and_test(&p_item->used) ) {
    //printk("__put_session: freeing p_item\n");
    ppa_session_list_free_item(p_item);
  }
}

/*
 * Put the session back. If reference count is zero, then session is freed.
 */
void ppa_session_put(struct session_list_item* p_item)
{
  //ppa_session_print(p_item);
  if(p_item && ppa_atomic_dec_and_test(&p_item->used) ) {
    //printk("put_session: freeing p_item\n");
    ppa_session_list_lock();
    ppa_session_list_free_item(p_item);
    ppa_session_list_unlock();
  }
}
EXPORT_SYMBOL(ppa_session_put);

/*
 * Find the session that is in use in a given hash(bucket). 
 */
static INLINE 
struct session_list_item * ppa_session_itr_next(PPA_HLIST_NODE* node)
{
  struct session_list_item *p_item;

  while( node != NULL ) {

    p_item = ppa_hlist_entry(node, struct session_list_item, hlist);
    if( ppa_atomic_read(&p_item->used ) )
      return p_item;
    node = list_get_next_node(node);
  }

  return NULL;
}

/*
 * Iterate through the session list. 
 * Call this function to start the iteration through list.
 * Call ppa_session_iterate_next to get next item
 * Call ppa_session_stop_iteration to stop interation.
 * Note: Once this function is ivoked, the iteration must be stopped by calling
 * ppa_session_stop_iteration
 *
 *              ***** NOTE *****
 * There could be need to increment the reference count in this function and also
 * in ppa_session_iterate_next. If reference count is increameted, then caller
 * should put back the session after use. OR other approach :-
 * In ppa_session_iterate_next put session(decrement the ref count) and 
 * ppa_session_stop_iteration should take session_list_item ptr and decrement ref
 * count if ptr is not null(this is required in case user breaks iteration)
 */
int32_t ppa_session_start_iteration(uint32_t *ppos, struct session_list_item **pp_item)
{
  PPA_HLIST_NODE *node = NULL;
  int index;
  uint32_t l;

  l = *ppos + 1;

  ppa_session_list_read_lock();
  
  *pp_item = NULL;
  if( !ppa_is_init() ) {
    return PPA_FAILURE; 
  }
  
  for ( index = 0; l && index < SESSION_LIST_HASH_TABLE_SIZE; index++ ) {
      
    PPA_SESSION_LIST_FOR_EACH_NODE(node, &g_session_list_hash_table[index]) {
      if( !--l ) break;
    }
  }

  if ( l == 0 && node ) {
    *pp_item = ppa_session_itr_next(node);    
    while(((*pp_item) == NULL ) && index <  SESSION_LIST_HASH_TABLE_SIZE) {
      index++;
      node = list_get_first_node(g_session_list_hash_table + index);
      *pp_item = ppa_session_itr_next(node);
    }
  }
  
  if( *pp_item ) {
    (*ppos)++;
    return PPA_SUCCESS;
  } 
    
  return PPA_FAILURE;
}
EXPORT_SYMBOL(ppa_session_start_iteration);

/* 
 * Get next item from session list
 */
int32_t ppa_session_iterate_next(uint32_t *ppos, struct session_list_item **pp_item)
{
  uint32_t index;
  struct session_list_item *p_item=NULL;
  PPA_HLIST_NODE *node;

  if(likely(*pp_item != NULL)) {

    node = list_get_next_node(&((*pp_item)->hlist)); 
    p_item = ppa_session_itr_next(node);

    if( p_item == NULL ) {
      index = ppa_session_get_index((*pp_item)->hash) + 1;
      for( ;p_item == NULL && index < SESSION_LIST_HASH_TABLE_SIZE; index++ ) { 
        node = list_get_first_node(g_session_list_hash_table+index);
        p_item = ppa_session_itr_next(node);
      }
    }
  }

  if( ((*pp_item) = p_item) ) {
    (*ppos)++;
    return PPA_SUCCESS;
  }
    
  return PPA_FAILURE;
}
EXPORT_SYMBOL(ppa_session_iterate_next);

/*
 * Call this function to stop the iteration.
 */
void ppa_session_stop_iteration(void)
{
  ppa_session_list_read_unlock();
}
EXPORT_SYMBOL(ppa_session_stop_iteration);

/*
 * This function retrieves the session in a given hash.
 */
int32_t ppa_session_get_items_in_hash(uint32_t index, 
                                        struct session_list_item **pp_item, 
                                        uint32_t maxToCopy, 
                                        uint32_t *copiedItems, 
                                        uint32_t flag)
{
  //note, pp_item will allocate memory for pp_item 
  struct session_list_item *p, *p_tmp = NULL;
  uint32_t nCopiedItems;
  uint32_t count;
//#define MAX_ITEM_PER_HASH  12
   
  if( !copiedItems) {
    critial_err("copiedItems is NULL\n");
    return PPA_FAILURE;
  }
  *copiedItems = 0;
    
  if( index >= SESSION_LIST_HASH_TABLE_SIZE) {
    return PPA_INDEX_OVERFLOW;
  }
#if defined(MAX_ITEM_PER_HASH)           
  count=MAX_ITEM_PER_HASH;
#else    
  count = ppa_sesssion_get_count_in_hash(index);
#endif    
  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Session[%d] has %d items\n", index, count );    
    
  if( count == 0 ) return PPA_SUCCESS;    
  
  if( !*pp_item ) { 
    //If the buffer is not allocated yet, then allocate it  
    p_tmp = ppa_malloc( sizeof(struct session_list_item) * ( count + 1 ) );
    if( p_tmp == NULL ) {
      err("ppa_malloc failed to get %d bytes memory\n", sizeof(struct session_list_item) * count  );
      return PPA_ENOMEM;
    }
    ppa_memset( (void *)p_tmp, 0, sizeof(struct session_list_item) * count);
    *pp_item = p_tmp;
    //maxToCopy= count;
  } else {
    /*buffer is preallocated already */
    p_tmp = *pp_item ;
    if( count > maxToCopy) 
      count = maxToCopy;
  }

  if( count > 100 ) {
    err("Why counter=%d in one single hash index\n", count);
    count = 100;
  }

  nCopiedItems = 0;
  ppa_session_list_read_lock();
  PPA_SESSION_LIST_FOR_EACH_ENTRY_READ(p,&g_session_list_hash_table[index],hlist) {      
    
    if( ppa_atomic_read(&p->used) ) {

      ppa_memcpy( &p_tmp[nCopiedItems], p, sizeof(struct session_list_item) );

      /*add below codes for session management purpose from shivaji --start*/
      if( (flag & SESSION_BYTE_STAMPING) && (p->flags & SESSION_ADDED_IN_HW) ) {
        p->prev_sess_bytes = p->acc_bytes - p->prev_clear_acc_bytes;
      } else if((flag & SESSION_BYTE_STAMPING) && !(p->flags & SESSION_ADDED_IN_HW)) {
        p->prev_sess_bytes = p->mips_bytes;
      }
          
      nCopiedItems++;
      if(nCopiedItems >= count ) break;
    }
  } 
  ppa_session_list_read_unlock();
  
  *copiedItems = nCopiedItems;    
  return PPA_SUCCESS;
}
//EXPORT_SYMBOL(ppa_session_get_items_in_hash);

/*
 * This function retunrs the number of sessions in a hash
 */
uint32_t ppa_sesssion_get_count_in_hash(uint32_t hashIndex)
{
  uint32_t num = 0;
  PPA_HLIST_NODE *node;

  if( hashIndex >= SESSION_LIST_HASH_TABLE_SIZE ) 
    return 0;
  
  ppa_session_list_read_lock();
  PPA_SESSION_LIST_FOR_EACH_NODE(node,g_session_list_hash_table+hashIndex) {
     num ++;
     if( num > 100 ) {
       err("Why num=%d in one single hash index\n", num);
       break;
     }
  }
  ppa_session_list_read_unlock();

  return num;    
}

/***** Bridged session related routines *****/

/* Bridged timout handler */
static void ppa_session_br_timeout(unsigned long item)
{
  struct session_list_item *p_item = (struct session_list_item *)item;
  uint32_t timeout;

  timeout = ppa_get_time_in_sec() - p_item->last_hit_time;
  /*
  printk("Timeout:%s %u (%u)\n",
      ((p_item->flag2 & SESSION_BRIDGED_SESSION)?"Bridged":"Routed"),
      timeout,p_item->timeout); */

  /* check dying bit ?? */
  if( timeout < p_item->timeout ) {
   //restart timer
   ppa_timer_add(&p_item->timer, p_item->timeout);
   return ; 
  }
  
  ppa_session_list_lock();
  __ppa_session_delete_item(p_item);
  ppa_session_list_unlock();
  ppa_session_put(p_item);
}

/* Birdged session timer init */
int ppa_session_br_timer_init(struct session_list_item *p_item)
{
  /*
   * Since states are not tracked for bridged flows, so set default 
   * timeout (of bridge timeout) ***This will be changed later***
   */
  p_item->timeout = DEFAULT_BRIDGING_TIMEOUT_IN_SEC; 
  ppa_timer_setup(&p_item->timer, ppa_session_br_timeout, (unsigned long)p_item);
  //ppa_session_print(p_item);
  return 1;
}

/* Bridged session timer add */
void  ppa_session_br_timer_add(struct session_list_item *p_item)
{
  ppa_atomic_inc(&p_item->used);/* used by timer */
  ppa_timer_add(&p_item->timer, p_item->timeout);
  //ppa_session_print(p_item);
}

/* Bridged session timer del */
void  ppa_session_br_timer_del(struct session_list_item *p_item)
{
  if( ppa_timer_pending(&p_item->timer) ) {
    ppa_timer_del(&p_item->timer);
    //ppa_session_print(p_item);
    ppa_session_put(p_item);
  } 
}

#if defined CONFIG_LTQ_PPA_MPE_HAL || defined CONFIG_LTQ_PPA_MPE_HAL_MODULE
/* Template buffer APIs */
extern int32_t (*ppa_construct_template_buf_hook)( PPA_BUF *skb, 
                                            struct session_list_item *p_item,
                                            struct netif_info *tx_ifinfo);
extern void (*ppa_destroy_template_buf_hook)(void* tmpl_buf);

extern void (*ppa_session_mc_destroy_tmplbuf_hook)(void* sessionAction);
extern struct session_action * (*ppa_construct_mc_template_buf_hook) (void *p_item, uint32_t dest_list);

int32_t ppa_session_construct_tmplbuf(struct sk_buff *skb, 
                              struct session_list_item *p_item, 
                              struct netif_info *tx_ifinfo)
{
  if( ppa_construct_template_buf_hook ) {
    return ppa_construct_template_buf_hook(skb, p_item, tx_ifinfo);
  }

  return PPA_FAILURE;
}

struct session_action *ppa_session_mc_construct_tmplbuf(void *p_item, uint32_t dest_list)
{
  if( ppa_construct_mc_template_buf_hook ) {
    return ppa_construct_mc_template_buf_hook(p_item, dest_list);
  }

  return PPA_FAILURE;

}

void ppa_session_destroy_tmplbuf(struct session_list_item *p_item)
{
  if( p_item->sessionAction && ppa_destroy_template_buf_hook ) {
    ppa_destroy_template_buf_hook(p_item);
  } 
}

void ppa_session_mc_destroy_tmplbuf( void  *sessionAction)
{
  if( sessionAction && ppa_session_mc_destroy_tmplbuf_hook ) {
    ppa_session_mc_destroy_tmplbuf_hook(sessionAction);
  }
}

#endif
