/******************************************************************************
**
** FILE NAME    : ppa_api_hal_selector.c
** PROJECT      : PPA
** MODULES      : PPA HAL Selector APIs
**
** DATE         : 28 Feb 2014
** AUTHOR       : Kamal Eradath
** DESCRIPTION  : PPA HAL Selector layer APIs
** COPYRIGHT    :              Copyright (c) 2014
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 28 Feb 2014  Kamal Eradath          Initiate Version
*******************************************************************************/

/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/atmdev.h>
#include <net/sock.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 13)
#include <net/ipip.h>
#else
#include <net/ip_tunnels.h>
#endif
#include <linux/if_arp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <net/ipv6.h>
#include <net/ip6_tunnel.h>

/*
 *  Chip Specific Head File
 */
#include <net/ppa_api.h>
#if defined(CONFIG_LTQ_DATAPATH) && CONFIG_LTQ_DATAPATH
#include <net/datapath_api.h>
#endif
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_session.h"
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#include "ppa_api_session_limit.h"
#endif
#include "ppa_api_netif.h"
#include "ppe_drv_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_hal_selector.h"
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR


extern ppa_generic_hook_t ppa_drv_hal_hook[MAX_HAL];
extern PPA_ATOMIC g_hw_session_cnt;     /*declared in ppa_api_session.c */
extern PPA_HLIST_HEAD
    g_session_list_hash_table[SESSION_LIST_HASH_TABLE_SIZE];
extern PPA_HLIST_HEAD
    g_mc_group_list_hash_table[SESSION_LIST_MC_HASH_TABLE_SIZE];

extern ppa_tunnel_entry *g_tunnel_table[MAX_TUNNEL_ENTRIES];
extern uint32_t g_tunnel_counter[MAX_TUNNEL_ENTRIES];
extern PPE_LOCK g_tunnel_table_lock;

extern struct session_action * ppa_session_mc_construct_tmplbuf( void *p_item, uint32_t dest_list);
extern int32_t ppa_session_mc_destroy_tmplbuf(void *);
static void ppa_del_tunnel_tbl_entry(PPE_TUNNEL_INFO* tInfo)
{
  uint32_t  tunnel_idx = tInfo->tunnel_idx;

  if (tunnel_idx >= MAX_TUNNEL_ENTRIES)
    return;

  ppe_lock_get(&g_tunnel_table_lock);
  if (g_tunnel_counter[tunnel_idx] && !--g_tunnel_counter[tunnel_idx]) {
    /*
     * Mahipati:
     * ppa_hsel_del_tunnel_entry should be called only if no sessions are
     * active on the given tunnel.
     * Note: This table is maintained only for PAE_HAL
     */
    ppa_hsel_del_tunnel_entry(tInfo, 0, PAE_HAL);
    ppa_free(g_tunnel_table[tunnel_idx]);
  }
  ppe_lock_release(&g_tunnel_table_lock);
}

static inline
int compare_ip6_tunnel(PPA_IP6HDR* t1, struct ip6_tnl *t2)
{
   return ((ipv6_addr_cmp((struct in6_addr *)&(t1->saddr), &t2->parms.laddr) == 0) &&
           (ipv6_addr_cmp((struct in6_addr *)&(t1->daddr), &t2->parms.raddr) == 0));
}

static inline
int compare_ip4_tunnel(PPA_IP4HDR* t1, struct ip_tunnel *t2)
{  
  return (t1->saddr == t2->parms.iph.saddr && 
          t1->daddr == t2->parms.iph.daddr);
}

static inline
void init_ip6_tunnel(PPA_IP6HDR *ip6hdr, struct ip6_tnl* t6)
{
  ip6hdr->hop_limit = t6->parms.hop_limit;
  ipv6_addr_copy((struct in6_addr *) &ip6hdr->saddr, &t6->parms.laddr); 
  ipv6_addr_copy((struct in6_addr *) &ip6hdr->daddr, &t6->parms.raddr);
}

static inline
void init_ip4_tunnel(PPA_IP4HDR *ip4hdr, struct ip_tunnel* t)
{
  ip4hdr->saddr = t->parms.iph.saddr;
  ip4hdr->daddr = t->parms.iph.daddr;
}

static uint32_t ppa_add_tunnel_tbl_entry(PPE_TUNNEL_INFO * entry)
{
  uint32_t  index;
  uint32_t  ret = PPA_EAGAIN;
  uint32_t  empty_entry = MAX_TUNNEL_ENTRIES;
  struct ip_tunnel *tip = NULL;
  struct ip6_tnl *tip6 = NULL;
  ppa_tunnel_entry *t_entry = NULL;

  switch(entry->tunnel_type) {
    case TUNNEL_TYPE_DSLITE:
    case TUNNEL_TYPE_IP6OGRE:
    case TUNNEL_TYPE_6EOGRE:
      tip6 = (struct ip6_tnl *)netdev_priv(entry->tx_dev);
      break;
    case TUNNEL_TYPE_6RD:
    case TUNNEL_TYPE_IPOGRE:
    case TUNNEL_TYPE_EOGRE:
      tip = (struct ip_tunnel *)netdev_priv(entry->tx_dev);
      break;
    default:
      goto tunnel_err;
  }

  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {

    t_entry = g_tunnel_table[index];
    if ( !t_entry ) {
      if (empty_entry == MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else if( entry->tunnel_type == t_entry->tunnel_type ) {
      
      switch(entry->tunnel_type) {
        case TUNNEL_TYPE_DSLITE:
        case TUNNEL_TYPE_IP6OGRE:
        case TUNNEL_TYPE_6EOGRE:
          //compere the src and dst address       
          if( compare_ip6_tunnel(&t_entry->tunnel_info.ip6_hdr,tip6) ) {
            goto tunnel_found;
          }
          break;
        case TUNNEL_TYPE_6RD:
        case TUNNEL_TYPE_IPOGRE:
        case TUNNEL_TYPE_EOGRE:
          //compere the src and dst address       
          if( compare_ip4_tunnel(&t_entry->tunnel_info.ip4_hdr,tip) ) {
            goto tunnel_found;
          }
          break;
      }
    }
  }

  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) {
    goto tunnel_err;
  } else {
  
    t_entry = (ppa_tunnel_entry *) ppa_malloc(sizeof(ppa_tunnel_entry));
    if (t_entry) {
        
      t_entry->tunnel_type = entry->tunnel_type;
      t_entry->hal_buffer = NULL;
      switch(entry->tunnel_type) {
        case TUNNEL_TYPE_DSLITE:
        case TUNNEL_TYPE_IP6OGRE:
        case TUNNEL_TYPE_6EOGRE:
          init_ip6_tunnel(&t_entry->tunnel_info.ip6_hdr,tip6);
          break;
        case TUNNEL_TYPE_6RD:
        case TUNNEL_TYPE_IPOGRE:
        case TUNNEL_TYPE_EOGRE:
          init_ip4_tunnel(&t_entry->tunnel_info.ip4_hdr,tip);
          ASSERT(t_entry->tunnel_info.ip4_hdr.saddr != 0
                 || t_entry->tunnel_info.ip4_hdr.daddr != 0,
                 "iphdr src/dst address is zero");
          break;
      }
    
      if( entry->tunnel_type == TUNNEL_TYPE_DSLITE ||
          entry->tunnel_type == TUNNEL_TYPE_6RD ) { 
                  
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, 
                    "Adding tunnel type %d index%d\n",
                    entry->tunnel_type, index);

        entry->tunnel_idx = index;
    	g_tunnel_table[index] = t_entry;
        /* Required for DS-Lite and 6rd Tunnels */
        if (ppa_hsel_add_tunnel_entry(entry, 0, PAE_HAL) != PPA_SUCCESS) {
  
          ppa_debug(DBG_ENABLE_MASK_ERR, 
                    "Adding tunnel type %d failed\n",
                    entry->tunnel_type);
          ppa_free(t_entry);
    	  g_tunnel_table[index] = NULL;
          goto tunnel_err; 
        }
      }
    }
  }

tunnel_found:
  ret = PPA_SUCCESS;
  g_tunnel_counter[index]++;
  entry->tunnel_idx = index;

tunnel_err:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;
}


#if defined(CONFIG_LTQ_PPA_MPE_IP97)
uint32_t ppa_add_ipsec_tunnel_tbl_entry(PPA_XFRM_STATE * entry,sa_direction dir, uint32_t *tunnel_index )
{
  uint32_t  index;
  uint32_t  ret = PPA_EAGAIN;
  uint32_t  empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {

    t_entry = g_tunnel_table[index];
    if ( !t_entry ) {
      if (empty_entry == MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else {
          //compere the src and dst address       
	    if ( t_entry->tunnel_info.ipsec_hdr.inbound != NULL)
            {
            if ( ( (t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr.a4 == entry->props.saddr.a4) 
		&& (t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr.a4 == entry->id.daddr.a4) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->ealg->alg_name , entry->ealg->alg_name))
		&& (t_entry->tunnel_info.ipsec_hdr.outbound == NULL)
              	)
		) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n %s,Inbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\ninbound daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\ninbound saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry daddr=0x%x\n",entry->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry saddr=0x%x\n",entry->props.saddr.a4);
            		goto tunnel_found;
          	  }
             }
	    if ( t_entry->tunnel_info.ipsec_hdr.outbound != NULL)
            {
              	if ( (t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr.a4 == entry->props.saddr.a4) 
		&& (t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr.a4 == entry->id.daddr.a4) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->ealg->alg_name , entry->ealg->alg_name))
		&& (t_entry->tunnel_info.ipsec_hdr.inbound == NULL)
		)
	        {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n %s,Outbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\noutbound daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\noutbound saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry daddr=0x%x\n",entry->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry saddr=0x%x\n",entry->props.saddr.a4);
            		goto tunnel_found;
          	}
             }
    }
  }
  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) {
    goto tunnel_err;
  } else {
  
    t_entry = (ppa_tunnel_entry *) ppa_malloc(sizeof(ppa_tunnel_entry));
    if (t_entry) {
        
      t_entry->tunnel_type = TUNNEL_TYPE_IPSEC;
      t_entry->hal_buffer = NULL;
      t_entry->tunnel_info.ipsec_hdr.inbound = NULL;
      t_entry->tunnel_info.ipsec_hdr.outbound = NULL;
      g_tunnel_table[index] = t_entry;

      }
    }

tunnel_found:
  ret = PPA_SUCCESS;
  if(dir == INBOUND)  
	t_entry->tunnel_info.ipsec_hdr.inbound = entry;
  else
	t_entry->tunnel_info.ipsec_hdr.outbound = entry;

  t_entry->tunnel_info.ipsec_hdr.dir = dir;

  *tunnel_index = index;
  g_tunnel_counter[index]++;

tunnel_err:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;
}

uint32_t ppa_get_ipsec_tunnel_tbl_entry(PPA_XFRM_STATE * entry,sa_direction *dir, uint32_t *tunnel_index )
{
  uint32_t  index;
  uint32_t  ret = PPA_EAGAIN;
  uint32_t  empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {

    t_entry = g_tunnel_table[index];
    if ( !t_entry ) {
      if (empty_entry == MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else {
          //compere the src and dst address       

	    if ( t_entry->tunnel_info.ipsec_hdr.inbound != NULL)
            {

            if ( (t_entry->tunnel_info.ipsec_hdr.inbound->id.spi == entry->id.spi) 
                && ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr) ,&entry->id.daddr,entry->props.family)
		&& ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr), &entry->props.saddr,entry->props.family) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->ealg->alg_name , entry->ealg->alg_name))
               ) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,Inbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nINBOUND daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nINBOUND saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry daddr=0x%x\n",entry->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry saddr=0x%x\n",entry->props.saddr.a4);
                        *dir = INBOUND;
			t_entry->tunnel_info.ipsec_hdr.dir = INBOUND;
            		goto tunnel_found;
               }
             }
	     if ( t_entry->tunnel_info.ipsec_hdr.outbound != NULL) {

                  if ( (t_entry->tunnel_info.ipsec_hdr.outbound->id.spi == entry->id.spi) 
                && ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr),&entry->id.daddr,entry->props.family)
		&& ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr) ,&entry->props.saddr, entry->props.family) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->ealg->alg_name , entry->ealg->alg_name))
               ) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,Outbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nOUTBOUND daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nOUTBOUND saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry daddr=0x%x\n",entry->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nentry saddr=0x%x\n",entry->props.saddr.a4);
                        *dir = OUTBOUND;
			t_entry->tunnel_info.ipsec_hdr.dir = OUTBOUND;
            		goto tunnel_found;
               }
             }

    }
  }
//  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) 
    goto tunnel_err;

tunnel_found:
  ret = PPA_SUCCESS;
  *tunnel_index = index;

tunnel_err:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;
}

uint32_t ppa_add_ipsec_tunnel_tbl_update(sa_direction dir, uint32_t tunnel_index )
{
  uint32_t  index;
  uint32_t  ret = PPA_SUCCESS;
  uint32_t  empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  if (tunnel_index == MAX_TUNNEL_ENTRIES)
     return PPA_FAILURE;
  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry

  t_entry = g_tunnel_table[tunnel_index];
  if ( t_entry != NULL ) {

  if(dir == INBOUND)  
	t_entry->tunnel_info.ipsec_hdr.inbound = NULL;
  else
	t_entry->tunnel_info.ipsec_hdr.outbound = NULL;
  }

  if( (t_entry->tunnel_info.ipsec_hdr.inbound == NULL) && (t_entry->tunnel_info.ipsec_hdr.outbound == NULL))
  {
    ppa_free(g_tunnel_table[tunnel_index]);
    g_tunnel_counter[tunnel_index]--;
    g_tunnel_table[tunnel_index] = NULL;
  }
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;

}

uint32_t ppa_ipsec_tunnel_tbl_routeindex(uint32_t tunnel_index, uint32_t routing_entry)
{
  uint32_t  ret = PPA_SUCCESS;
  ppa_tunnel_entry *t_entry = NULL;

  if (tunnel_index == MAX_TUNNEL_ENTRIES)
     return PPA_FAILURE;
  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry

  t_entry = g_tunnel_table[tunnel_index];
  if ( t_entry != NULL ) {

	t_entry->tunnel_info.ipsec_hdr.routeindex =routing_entry;
  }

  ppe_lock_release(&g_tunnel_table_lock);

  return ret;

}

uint32_t ppa_del_tunnel_ipsec_entry(uint32_t  tunnel_idx)
{
  uint32_t  ret = PPA_SUCCESS;
  if (tunnel_idx >= MAX_TUNNEL_ENTRIES)
    return PPA_FAILURE;

  ppe_lock_get(&g_tunnel_table_lock);
  if (g_tunnel_counter[tunnel_idx] && !--g_tunnel_counter[tunnel_idx]) {
  //  ppa_hsel_del_tunnel_entry(tInfo, 0, PAE_HAL);
    ppa_free(g_tunnel_table[tunnel_idx]);
  }
  ppe_lock_release(&g_tunnel_table_lock);
}

int32_t __ppa_update_ipsec_tunnelindex(struct session_list_item *pp_item)
{

  uint32_t  index;
  uint32_t  ret = PPA_EAGAIN;
  uint32_t  empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  uint32_t  spi_id =0x0;
  spi_id = ((pp_item->src_port << 16) & 0xFFFF0000); 
  spi_id |= ((pp_item->dst_port) & 0xFFFF); 
  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {

    t_entry = g_tunnel_table[index];
    if ( !t_entry ) {
      if (empty_entry == MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else {
          //compere the src and dst address       

	    if ( t_entry->tunnel_info.ipsec_hdr.inbound != NULL)
            {

            if ( (t_entry->tunnel_info.ipsec_hdr.inbound->id.spi == spi_id) 
                && ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr) , &(pp_item->dst_ip),t_entry->tunnel_info.ipsec_hdr.inbound->props.family)
		&& ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr), &(pp_item->src_ip),t_entry->tunnel_info.ipsec_hdr.inbound->props.family) 
/*
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->ealg->alg_name , entry->ealg->alg_name)) */
               ) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,Inbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nINBOUND daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nINBOUND saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr.a4);
            		goto tunnel_found;
               }
             }
	     if ( t_entry->tunnel_info.ipsec_hdr.outbound != NULL) {

                  if ( (t_entry->tunnel_info.ipsec_hdr.outbound->id.spi == spi_id) 
                && ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr),&(pp_item->dst_ip),t_entry->tunnel_info.ipsec_hdr.outbound->props.family)
		&& ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr) , &(pp_item->src_ip),t_entry->tunnel_info.ipsec_hdr.outbound->props.family)
/* 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->ealg->alg_name , entry->ealg->alg_name)) */
               ) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,Outbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nOUTBOUND daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nOUTBOUND saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr.a4);
            		goto tunnel_found;
               }
             }

    }
  }
//  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) 
    goto tunnel_err;

tunnel_found:
  ret = PPA_SUCCESS;
  //*tunnel_index = index;
   pp_item->tunnel_idx = index;
   pp_item->routing_entry =t_entry->tunnel_info.ipsec_hdr.routeindex;
tunnel_err:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;


}

int32_t __ppa_search_ipsec_rtindex(struct session_list_item *pp_item)
{

  uint32_t  index;
  uint32_t  ret =PPA_EAGAIN;
  uint32_t  empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  uint32_t  spi_id =0x0;
  spi_id = ((pp_item->src_port << 16) & 0xFFFF0000); 
  spi_id |= ((pp_item->dst_port) & 0xFFFF); 
  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {

    t_entry = g_tunnel_table[index];
    if ( !t_entry ) {
      if (empty_entry == MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else {
          //compere the src and dst address       

	    if ( t_entry->tunnel_info.ipsec_hdr.inbound != NULL)
            {

            if ( (t_entry->tunnel_info.ipsec_hdr.inbound->id.spi == spi_id) 
                && ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr) , &(pp_item->dst_ip),t_entry->tunnel_info.ipsec_hdr.inbound->props.family)
		&& ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr), &(pp_item->src_ip),t_entry->tunnel_info.ipsec_hdr.inbound->props.family) 
/*
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.inbound->ealg->alg_name , entry->ealg->alg_name)) */
               ) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,Inbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nINBOUND daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nINBOUND saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.inbound->props.saddr.a4);
            		goto tunnel_found;
               }
             }
	     if ( t_entry->tunnel_info.ipsec_hdr.outbound != NULL) {

                  if ( (t_entry->tunnel_info.ipsec_hdr.outbound->id.spi == spi_id) 
                && ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr),&(pp_item->dst_ip),t_entry->tunnel_info.ipsec_hdr.outbound->props.family)
		&& ppa_ipsec_addr_equal(&(t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr) , &(pp_item->src_ip),t_entry->tunnel_info.ipsec_hdr.outbound->props.family)
/* 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->aalg->alg_name , entry->aalg->alg_name)) 
		&& (ppa_str_cmp(t_entry->tunnel_info.ipsec_hdr.outbound->ealg->alg_name , entry->ealg->alg_name)) */
               ) {
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,Outbound Tunnlel found index =%d\n",__FUNCTION__, index);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nOUTBOUND daddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->id.daddr.a4);
            		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\nOUTBOUND saddr=0x%x\n",t_entry->tunnel_info.ipsec_hdr.outbound->props.saddr.a4);
            		goto tunnel_found;
               }
             }

    }
  }
//  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) 
  {
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n%s,%d Tunnel Error\n",__FUNCTION__,__LINE__);
    goto tunnel_err;
  }

tunnel_found:
  ret = PPA_SUCCESS;
  //*tunnel_index = index;

   if(pp_item->routing_entry != t_entry->tunnel_info.ipsec_hdr.routeindex)
  	ret = PPA_FAILURE;
tunnel_err:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;


}






#endif





#if defined(L2TP_CONFIG) && L2TP_CONFIG
static uint32_t ppa_add_tunnel_tbll2tp_entry(PPA_L2TP_INFO * entry)
{
  uint32_t index, ret = PPA_SUCCESS;
  uint32_t empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {
    t_entry = g_tunnel_table[index];
    if (!t_entry) {
      if (empty_entry >= MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else {
      if( t_entry->tunnel_type == entry->tunnel_type && 
          entry->tunnel_id == t_entry->tunnel_info.l2tp_hdr.tunnel_id) {
        goto ADD_PPPOL2TP_ENTRY_GOON;
      }
    }
  }

  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) {
    ret = PPA_EAGAIN;
    goto ADD_TUNNEL_ENTRY_FAILURE;
  } else {
    t_entry = (ppa_tunnel_entry *) ppa_malloc(sizeof(ppa_tunnel_entry));
    if (t_entry) {
      t_entry->tunnel_type = TUNNEL_TYPE_L2TP;
      ppa_memcpy(&t_entry->tunnel_info.l2tp_hdr,entry,sizeof(PPA_L2TP_INFO));
      t_entry->hal_buffer = NULL;
      g_tunnel_table[index] = t_entry;
    }
  }

ADD_PPPOL2TP_ENTRY_GOON:
  g_tunnel_counter[index]++;
  entry->tunnel_idx = index;
ADD_TUNNEL_ENTRY_FAILURE:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;
}
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
static uint32_t ppa_add_tunnel_tblcapwap_entry(PPA_CMD_CAPWAP_INFO * entry)
{
  uint32_t index, ret = PPA_SUCCESS;
  uint32_t empty_entry = MAX_TUNNEL_ENTRIES;
  ppa_tunnel_entry *t_entry = NULL;

  ppe_lock_get(&g_tunnel_table_lock);
  // first lookup for a matching entry
  for (index = 0; index < MAX_TUNNEL_ENTRIES; index++) {
    if (!g_tunnel_table[index]) {
      if (empty_entry >= MAX_TUNNEL_ENTRIES)
        empty_entry = index;
    } else {
// revisit to adapt to grx500 ifid + subifid mechanism
      if (entry->directpath_portid ==
          g_tunnel_table[index]->tunnel_info.capwap_hdr.
          directpath_portid) {
        goto ADD_CAPWAP_ENTRY_GOON;
      }
    }
  }

  index = empty_entry;
  if (index >= MAX_TUNNEL_ENTRIES) {
    ret = PPA_EAGAIN;
    goto ADD_TUNNEL_ENTRY_FAILURE;
  } else {
    t_entry = (ppa_tunnel_entry *) ppa_malloc(sizeof(ppa_tunnel_entry));
    if (t_entry) {
      t_entry->tunnel_type = TUNNEL_TYPE_CAPWAP;
      ppa_memcpy(&t_entry->tunnel_info.capwap_hdr, entry,
                 sizeof(PPA_CMD_CAPWAP_INFO));

      t_entry->hal_buffer = NULL;
    }
    g_tunnel_table[index] = t_entry;
  }

ADD_CAPWAP_ENTRY_GOON:
  g_tunnel_counter[index]++;
  entry->tunnel_idx = index;
ADD_TUNNEL_ENTRY_FAILURE:
  ppe_lock_release(&g_tunnel_table_lock);

  return ret;
}
#endif

static uint32_t ppa_form_capability_list(struct session_list_item * p_item)
{
  uint32_t total_caps=0;

  //  session is routing
  if (!(p_item->flags & SESSION_IS_IPV6)) {
#ifdef CONFIG_LTQ_PPA_MPE_IP97
    if(!(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND_SA))
#endif
      p_item->caps_list[total_caps++].cap = SESS_IPV4;
  } else {
    p_item->caps_list[total_caps++].cap = SESS_IPV6;
  }

  //  session needs 6rd tunneling
  if ( ppa_is_6rdSession(p_item)) {
    p_item->caps_list[total_caps++].cap = TUNNEL_6RD;
  } else if ( ppa_is_DsLiteSession(p_item)) {
    //  session needs dslite tunneling
    p_item->caps_list[total_caps++].cap = TUNNEL_DSLITE;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
  } else if (p_item->flags & SESSION_VALID_PPPOL2TP) {
    // ppp over L2TP
    p_item->caps_list[total_caps++].cap = 
      ppa_is_LanSession(p_item)?TUNNEL_L2TP_US:TUNNEL_L2TP_DS;
#endif
  } else if (ppa_is_CapWapSession(p_item)) {
    p_item->caps_list[total_caps++].cap = 
      ppa_is_LanSession(p_item)?TUNNEL_CAPWAP_US:TUNNEL_CAPWAP_DS;
  } else if (ppa_is_GreSession(p_item)) {
    p_item->caps_list[total_caps++].cap = 
      ppa_is_LanSession(p_item)?TUNNEL_GRE_US:TUNNEL_GRE_DS;
  }
#ifdef CONFIG_LTQ_PPA_MPE_IP97
  if (p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND) {
    // ppp over L2TP
    p_item->caps_list[total_caps++].cap = TUNNEL_IPSEC_US;
  }
  if (p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_INBOUND) {
    // ppp over L2TP
    p_item->caps_list[total_caps++].cap = TUNNEL_IPSEC_DS;
  }
#endif
/* kamal: TBD
   Not supported in existing PPA
//  session needs tunneling + encryption
    if(p_item->flags & SESSION_TUNNEL_ENCRYPT ){
	p_item->caps_list[total_caps++].cap = TUNNEL_ENCRYPT;
    } else if(p_item->flags & SESSION_TUNNEL_DECRYPT ){
// session needs tunneling + decryption
	p_item->caps_list[total_caps++].cap = TUNNEL_DECRYPT;
    } 
*/

  return total_caps;
}
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
static int32_t ppa_get_baseif_for_flowid(const char *in_if,const char **out_baseif)
{
	PPA_NETIF *netif_1,*netif_2;
	PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE];
	int32_t ret = PPA_FAILURE;
	netif_1 = ppa_get_netif(in_if);
        if ( ppa_check_is_ppp_netif(netif_1) )
        {
            	if ( ppa_check_is_pppoe_netif(netif_1) && ppa_pppoe_get_physical_if(netif_1, NULL, underlying_ifname) == PPA_SUCCESS )
            	{
                	netif_2 = ppa_get_netif(underlying_ifname);
		
                	struct netif_info *tmp_ifinfo_1;
                	if ( __ppa_netif_lookup(netif_2->name, &tmp_ifinfo_1) == PPA_SUCCESS )
                	{
                    		if( tmp_ifinfo_1->manual_lower_ifname[0] )
                    		{
                        		PPA_NETIF *tmp_netif_1 = ppa_get_netif(tmp_ifinfo_1->manual_lower_ifname);
                        		if( tmp_netif_1 )
                        		{
						netif_2 = tmp_netif_1;
						*out_baseif = netif_2->name;
						ret = PPA_SUCCESS;
					}

		    		}
            			else if( netif_2 && ppa_if_is_vlan_if(netif_2, NULL) && 
                			ppa_vlan_get_underlying_if(netif_2, NULL, underlying_ifname) == PPA_SUCCESS )
            			{
					*out_baseif = underlying_ifname;
					ret = PPA_SUCCESS;
	    			}
			}
            	}
	}
	else if(ppa_if_is_vlan_if(netif_1, NULL) && ppa_vlan_get_underlying_if(netif_1, NULL, underlying_ifname) == PPA_SUCCESS)
	{
		*out_baseif = underlying_ifname;	
		ret = PPA_SUCCESS;
	}
	else
	{
                struct netif_info *tmp_ifinfo_2;
                if ( __ppa_netif_lookup(netif_1->name, &tmp_ifinfo_2) == PPA_SUCCESS )
                {
                	if( tmp_ifinfo_2->manual_lower_ifname[0] )
                	{
                       		PPA_NETIF *tmp_netif_2 = ppa_get_netif(tmp_ifinfo_2->manual_lower_ifname);
                       		if( tmp_netif_2 )
                       		{
					netif_1 = tmp_netif_2;
					*out_baseif = netif_1->name;
					ret = PPA_SUCCESS;
				}
	    		}
       			else if( netif_1 && ppa_if_is_vlan_if(netif_1, NULL) && 
               			ppa_vlan_get_underlying_if(netif_1, NULL, underlying_ifname) == PPA_SUCCESS )
       			{
				*out_baseif = underlying_ifname;
				ret = PPA_SUCCESS;
    			}
		}
	}
	return ret;
}
#endif

/*****************************************************************************************/
// ppa_api_session.c will call this when a session is getting added
/*****************************************************************************************/
int32_t ppa_hsel_add_routing_session(struct session_list_item * p_item,
                                     uint32_t f_test)
{
// while adding a new session do the following steps
// check all the flags and find the capabilities needed and form an list of capabilities needed by this session
// group the capabilities based on first regisrted HAL

  uint32_t num_caps = 0, i, j, k, l;
  uint8_t f_more_hals = 0;
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
  uint8_t l1,l2,l3,l4,m;
#endif
  PPE_ROUTING_INFO route;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
  PPA_L2TP_INFO l2tpinfo = { 0 };
#endif

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  struct session_list_item *old_item;
#endif

  int ret = PPA_SUCCESS;

  ppa_memset(&route, 0, sizeof(route));
#if defined(L2TP_CONFIG) && L2TP_CONFIG
  ppa_memset(&l2tpinfo, 0, sizeof(l2tpinfo));
#endif


  if (p_item->flags & SESSION_ADDED_IN_HW) {
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Session %p already in HW\n",
              p_item->session);
    return PPA_SUCCESS;
  }
////////////////////////////////////////////////////////////////    
//  find all the capabilities needed by this session
////////////////////////////////////////////////////////////////

  ppa_memset(p_item->caps_list, 0,
             sizeof(PPA_HSEL_CAP_NODE) * MAX_RT_SESS_CAPS);

  num_caps = ppa_form_capability_list(p_item);

////////////////////////////////////////////////////////////////
// for each capability find out the HAL  
////////////////////////////////////////////////////////////////
  if (ppa_select_hals_from_caplist(0, num_caps, p_item->caps_list) !=
      PPA_SUCCESS) {
    return PPA_FAILURE;
  }

  for (i = 0; i < num_caps;) {
    f_more_hals = 0;
    j = ppa_group_hals_in_capslist(i, num_caps, p_item->caps_list);

    // Based on the capability of first entry in the list we can decide the action
    switch (p_item->caps_list[i].cap) {
    case SESS_IPV4:
    case SESS_IPV6:

      // based on the flags fill the PPE_ROUTING_INFO structure
      if (p_item->caps_list[i].hal_id == PPE_HAL) {
        // reset the paramenters to default value -1
        route.src_mac.mac_ix = ~0;
        route.pppoe_info.pppoe_ix = ~0;
        route.out_vlan_info.vlan_entry = ~0;
        route.mtu_info.mtu_ix = ~0;
      }
#if defined(L2TP_CONFIG) && L2TP_CONFIG
      route.pppol2tp_info.pppol2tp_ix = ~0;
#endif
      // check whether the session is an ipv4/nat/napt session    
      route.route_type =
          (p_item->flags & SESSION_VALID_NAT_IP) ? 
          ((p_item->flags & SESSION_VALID_NAT_PORT) ?  PPA_ROUTE_TYPE_NAPT : PPA_ROUTE_TYPE_NAT) :
          PPA_ROUTE_TYPE_IPV4;
      if ((p_item->flags & SESSION_VALID_NAT_IP)) {
        route.new_ip.f_ipv6 = 0;
        memcpy(&route.new_ip.ip, &p_item->nat_ip, sizeof(route.new_ip.ip));     //since only IPv4 support NAT, translate it to IPv4 format
      }

      if ((p_item->flags & SESSION_VALID_NAT_PORT))
        route.new_port = p_item->nat_port;

      route.f_is_lan = (p_item->flags & SESSION_LAN_ENTRY) ? 1 : 0;

	if(!(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND_LAN))
      	route.pppoe_mode = p_item->flags & SESSION_VALID_PPPOE; 
      // check whether sesssion is pppoe
      if( route.f_is_lan && (p_item->flags & SESSION_VALID_PPPOE) )  {
        route.pppoe_info.pppoe_session_id = p_item->pppoe_session_id;
        // in case PPE_HAL we need to call HAL ppeoe entry add
        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_drv_add_pppoe_entry(&route.pppoe_info, 0) == PPA_SUCCESS)
            p_item->pppoe_entry = route.pppoe_info.pppoe_ix;
          else {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                      "add pppoe_entry error\n");
            SET_DBG_FLAG(p_item, SESSION_DBG_ADD_PPPOE_ENTRY_FAIL);
            goto SESSION_VALID_PPPOE_ERROR;
          }
        }
      }

      route.mtu_info.mtu = p_item->mtu;
      // in case PPE_HAL we need to call HAL ppeoe entry add
      if (p_item->caps_list[i].hal_id == PPE_HAL) {
        if (ppa_drv_add_mtu_entry(&route.mtu_info, 0) == PPA_SUCCESS) {
          p_item->mtu_entry = route.mtu_info.mtu_ix;
          p_item->flags |= SESSION_VALID_MTU;
        } else {
          SET_DBG_FLAG(p_item, SESSION_DBG_ADD_MTU_ENTRY_FAIL);
          goto MTU_ERROR;
        }
      }

      // handle tunnels
      /* ?? Better to store tunnel type in p_item ???  */
      route.tnnl_info.tunnel_type = TUNNEL_TYPE_MAX; 
      if ( ppa_is_6rdSession(p_item) ) {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                  "add 6RD entry to acceleration, tx dev: %s\n",
                  p_item->tx_if->name);
        route.tnnl_info.tunnel_type = TUNNEL_TYPE_6RD;
	if(route.f_is_lan) {
	    route.new_ip.f_ipv6 = 0;
	    route.new_ip.ip.ip = p_item->sixrd_daddr;
	}
      } else if ( ppa_is_DsLiteSession(p_item)) {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                  "add Dslite entry to acceleration, tx dev: %s\n",
                  p_item->tx_if->name);
        route.tnnl_info.tunnel_type = TUNNEL_TYPE_DSLITE;
      } else if(ppa_is_GreSession(p_item) ) {

        struct netif_info* pnetinfo; 
        char* ifName;
  
        if( ppa_is_gre_netif(p_item->tx_if))
          ifName = p_item->tx_if->name;
        else 
          ifName = p_item->rx_if->name;

        if( PPA_SUCCESS != ppa_netif_lookup(ifName, &pnetinfo)) {
          goto MTU_ERROR; // Should never happen
        }

        switch( pnetinfo->greInfo.flowId)
        {
          case FLOWID_IPV4GRE:
            route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPOGRE; break;
          case FLOWID_IPV4_EoGRE:
            route.tnnl_info.tunnel_type = TUNNEL_TYPE_EOGRE; break;
          case FLOWID_IPV6GRE:
            route.tnnl_info.tunnel_type = TUNNEL_TYPE_IP6OGRE; break;
          case FLOWID_IPV6_EoGRE:
            route.tnnl_info.tunnel_type = TUNNEL_TYPE_6EOGRE; break;
        }
        ppa_netif_put(pnetinfo); 
      }
#if defined(L2TP_CONFIG) && L2TP_CONFIG
      else if (p_item->flags & SESSION_VALID_PPPOL2TP) {

        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                  "add L2TP entry to acceleration, tx dev: %s\n",
                  p_item->tx_if->name);
        route.tnnl_info.tunnel_type = TUNNEL_TYPE_L2TP;  
      }
#endif
#ifdef CONFIG_LTQ_PPA_MPE_IP97

	else if (p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND) {
		printk("%d\n",__LINE__);
        	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;  
        	route.tnnl_info.tunnel_idx = p_item->tunnel_idx;
	}
	else if (p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_INBOUND) {
		printk("%d\n",__LINE__);
        	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;  
        	route.tnnl_info.tunnel_idx = p_item->tunnel_idx;
	}
#endif
	if(route.tnnl_info.tunnel_type != TUNNEL_TYPE_IPSEC)
		p_item->tunnel_idx = ~0;
      if ( ppa_is_LanSession(p_item) && 
          route.tnnl_info.tunnel_type != TUNNEL_TYPE_MAX) {
   
        // upstream 
        switch( route.tnnl_info.tunnel_type ) 
        {    
          case TUNNEL_TYPE_DSLITE:
          case TUNNEL_TYPE_6RD:
            {
              route.tnnl_info.tx_dev = p_item->tx_if;

              if (p_item->caps_list[i].hal_id == PPE_HAL) {
                if (ppa_hsel_add_tunnel_entry(&route.tnnl_info, 0, PPE_HAL) ==
                    PPA_SUCCESS) {
                  p_item->tunnel_idx = route.tnnl_info.tunnel_idx;
                  route.tnnl_info.tunnel_idx = route.tnnl_info.tunnel_idx << 1 | 1;
                } else {
                  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                            "add tunnel 6rd entry error\n");
                  goto MTU_ERROR;
                }
              } else { 
              	if (ppa_add_tunnel_tbl_entry(&route.tnnl_info) != PPA_SUCCESS) {
                    goto MTU_ERROR;
              	}	 
		p_item->tunnel_idx = route.tnnl_info.tunnel_idx;
	      }
              break;
            }
          case TUNNEL_TYPE_EOGRE:
          case TUNNEL_TYPE_6EOGRE:
          case TUNNEL_TYPE_IPOGRE:
          case TUNNEL_TYPE_IP6OGRE:
            {
              /*
               * Note: For PAE complimentary processing PPPoE mode should 
               * be reset for these tunnel
               */

              if( route.f_is_lan )
                route.pppoe_mode = 0; 
              route.tnnl_info.tx_dev = p_item->tx_if;
              if (ppa_add_tunnel_tbl_entry(&route.tnnl_info) != PPA_SUCCESS) {
                goto MTU_ERROR;
              } 
                
              p_item->tunnel_idx = route.tnnl_info.tunnel_idx;
              break;
            }
          case TUNNEL_TYPE_L2TP:
            {
              /*
               * Note: For PAE complimentary processing PPPoE mode should 
               * be reset for these tunnel
               */

              if( route.f_is_lan )
                route.pppoe_mode = 0; 
              route.pppol2tp_info.pppol2tp_session_id = p_item->pppol2tp_session_id;
              route.pppol2tp_info.pppol2tp_tunnel_id = p_item->pppol2tp_tunnel_id;
              route.l2tptnnl_info.tunnel_type = TUNNEL_TYPE_L2TP;
              route.l2tptnnl_info.tx_dev = p_item->tx_if;
              l2tpinfo.tunnel_id = p_item->pppol2tp_tunnel_id;
              l2tpinfo.session_id = p_item->pppol2tp_session_id;
              l2tpinfo.tunnel_type = TUNNEL_TYPE_L2TP;

              ppa_pppol2tp_get_l2tpinfo(p_item->tx_if, &l2tpinfo);

              if (p_item->caps_list[i].hal_id == PPE_HAL) {
                if (ppa_drv_add_l2tptunnel_entry(&l2tpinfo, 0) == PPA_SUCCESS) {
                  p_item->l2tptunnel_idx = l2tpinfo.tunnel_idx;
                    route.l2tptnnl_info.tunnel_idx =  l2tpinfo.tunnel_idx << 1 | 1;
                } else {
                  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                            "add tunnel l2tp entry error\n");
                  goto MTU_ERROR;
                }
              } else {
                if (ppa_add_tunnel_tbll2tp_entry(&l2tpinfo) == PPA_SUCCESS) {
                  p_item->l2tptunnel_idx = l2tpinfo.tunnel_idx;
                }
              }
              break;
            }
        }
      } //Up stream tunnel handling

      // source mac 
      if (!(p_item->flags & SESSION_TX_ITF_IPOA_PPPOA_MASK)) {

        if( route.tnnl_info.tunnel_type == TUNNEL_TYPE_EOGRE ||
            route.tnnl_info.tunnel_type == TUNNEL_TYPE_6EOGRE ) {
          ppa_memcpy(route.src_mac.mac,p_item->src_mac,6);
        } else {
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	  if((!(route.tnnl_info.tunnel_type == TUNNEL_TYPE_IPSEC) 
			&&
	  	!(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_INBOUND)))  {
			
	      if(!(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND_LAN))
#endif
             ppa_get_netif_hwaddr(p_item->tx_if, route.src_mac.mac, 1);
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	    }
#endif
        }

        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_drv_add_mac_entry(&route.src_mac, 0) == PPA_SUCCESS) {
            p_item->src_mac_entry = route.src_mac.mac_ix;
            p_item->flags |= SESSION_VALID_NEW_SRC_MAC;
          } else {
            SET_DBG_FLAG(p_item, SESSION_DBG_ADD_MAC_ENTRY_FAIL);
            goto NEW_SRC_MAC_ERROR;
          }
        } else {
          p_item->flags |= SESSION_VALID_NEW_SRC_MAC;
          // PAE HAL
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
          if (p_item->br_rx_if) {
            ppa_bridge_entry_add(p_item->s_mac, NULL, p_item->br_rx_if,
                                 PPA_F_STATIC_ENTRY);
          } else {
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	printk("%d\n",__LINE__);
	if((p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_INBOUND)
			||
		(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND))
	{
		printk("%d\n",__LINE__);
	} else
#endif
	  if(p_item->rx_if)	
            ppa_bridge_entry_add(p_item->s_mac, NULL, p_item->rx_if,
                                 PPA_F_STATIC_ENTRY);
          }
#endif
        }
      }
      //vlan insert/remove
      if ((p_item->flags & SESSION_VALID_OUT_VLAN_INS)) {
        route.out_vlan_info.vlan_id = p_item->out_vlan_tag;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        route.out_vlan_info.stag_ins = PPA_ENABLED;
#endif
        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_hsel_add_outer_vlan_entry
              (&route.out_vlan_info, 0, PPE_HAL) == PPA_SUCCESS) {
            p_item->out_vlan_entry = route.out_vlan_info.vlan_entry;
          } else {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                      "add out_vlan_ix error\n");
            SET_DBG_FLAG(p_item, SESSION_DBG_ADD_OUT_VLAN_ENTRY_FAIL);
            goto OUT_VLAN_ERROR;
          }
        }
      }

      // copy the remaining route information
      route.dest_list = 1 << p_item->dest_ifid;

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      route.dest_subif_id = p_item->dest_subifid;
#endif

      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "dest_list = %02X\n",
                route.dest_list);
      // dscp re-marking
      if ((p_item->flags & SESSION_VALID_NEW_DSCP))
        route.new_dscp = p_item->new_dscp;


      ppa_memcpy(route.new_dst_mac, p_item->dst_mac, PPA_ETH_ALEN);
      route.dst_port = p_item->dst_port;
      route.src_port = p_item->src_port;
      route.f_is_tcp = p_item->flags & SESSION_IS_TCP;
      if(p_item->flags & SESSION_VALID_NEW_DSCP)
      route.f_new_dscp_enable = 1;
      route.f_vlan_ins_enable = p_item->flags & SESSION_VALID_VLAN_INS;
      route.new_vci = p_item->new_vci;
      route.f_vlan_rm_enable = p_item->flags & SESSION_VALID_VLAN_RM;

      route.f_out_vlan_ins_enable =
          p_item->flags & SESSION_VALID_OUT_VLAN_INS;
      route.f_out_vlan_rm_enable =
          p_item->flags & SESSION_VALID_OUT_VLAN_RM;
      route.dslwan_qid = p_item->dslwan_qid;
      route.collision_flag = p_item->collision_flag;

#if defined(CONFIG_NETWORK_EXTMARK) && defined(CONFIG_LTQ_PPA_GRX500)
    if(p_item->extmark & SESSION_FLAG_TC_REMARK)
    	route.f_tc_remark_enable = 1;
    else
    	route.f_tc_remark_enable = 0;
    /* skb->priority 0 - 7 refers to Low->High priority Queue & TC 0-7 also refers to Low->High priority Queue*/	    
    route.tc_remark = p_item->priority;
#endif
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
/*Start of : Set FlowId & TC for Vlan intercface */
    if(!ppa_is_TunneledSession(p_item))
    {
	struct netif_info *p_ifinfo;
	char *ifname,*ifname_base;
	ifname = p_item->tx_if->name;
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," %s:%d : calling ppa_get_baseif_for_flowid() with ifname = %s \n",__FUNCTION__,__LINE__,ifname);
	if(ppa_get_baseif_for_flowid(ifname,&ifname_base) == PPA_SUCCESS)
	{
        	if( PPA_SUCCESS != ppa_netif_lookup(ifname_base, &p_ifinfo)) {
        	  goto MTU_ERROR; // Should never happen
        	}
		route.nFlowId = (p_ifinfo->flowId & 0x3) << 6;
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," %s:%d : ifname_base = %s : p_ifinfo->flowId = %d \n",__FUNCTION__,__LINE__,ifname_base,p_ifinfo->flowId);
		l1 = (((p_ifinfo->flowId >> 2) & 0x1) << 3);
		l2 = (((p_ifinfo->flowId >> 2) & 0x2) << 1);
		l3 = (((p_ifinfo->flowId >> 2) & 0x4) >> 1);
		l4 = (((p_ifinfo->flowId >> 2) & 0x8) >> 3);
		m = l1 | l2 | l3 | l4;
		if(((p_ifinfo->flowId >> 2) & 0x0F) != 0)
		{
		    if(p_item->extmark & SESSION_FLAG_TC_REMARK)
			route.tc_remark |= m;
		    else
		    {
			route.tc_remark = m;
    			route.f_tc_remark_enable = 1;
		    }
		}
		else
		{
		    if(!(p_item->extmark & SESSION_FLAG_TC_REMARK))
			route.tc_remark = 0;
		}
	}
    }
/*End of :  Set FlowId & TC for Vlan intercface */
#endif      
      //ipv6 information
      if (p_item->flags & SESSION_IS_IPV6) {
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
        route.src_ip.f_ipv6 = 1;
        ppa_memcpy(route.src_ip.ip.ip6, p_item->src_ip.ip6,
                   sizeof(route.src_ip.ip.ip6));
        route.dst_ip.f_ipv6 = 1;
        ppa_memcpy(route.dst_ip.ip.ip6, p_item->dst_ip.ip6,
                   sizeof(route.dst_ip.ip.ip6));
#endif
      } else {
        route.src_ip.ip.ip = p_item->src_ip.ip;
        route.dst_ip.ip.ip = p_item->dst_ip.ip;
      }

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      route.sessionAction = p_item->sessionAction;
#endif 
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	if(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND_LAN)
		route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;
	//	route.tnnl_info.tunnel_idx = 0;
#endif
      // TBD: encrypt information to be filled in route in case of full processing by MPE

      // check whether more HALs needs to be configured to complete this session configuration
      if (i + j != num_caps) {  // more HALs are there in the caps_list
        f_more_hals = 1;
        // TBD: setup flow between the Acceleration blocks
      }

      if ((ret = ppa_hsel_add_routing_entry(&route, 0,
                            p_item->caps_list[i].hal_id)) == PPA_SUCCESS) {
        if (p_item->caps_list[i].hal_id == PAE_HAL) {
          // in this case we have to check the flags to see whether the PAE has done any swap 
          // route->flags == 1 // TBD: Obtain the  actual flags from PAE header
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
          if (route.flags & FLAG_SESSION_SWAPPED) {
            /*
             * Search in the session list to find out the session with 
             * routing_entry == route.entry (same index) 
             * clear the flag SESSION_ADDED_IN_HW
             * make session->routing_entry = ~0;
             * decrement g_hw_session_cnt
             */
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Session swapped!!!\n");
            old_item = __ppa_session_find_by_routing_entry(route.entry);
            if (old_item) {
              old_item->routing_entry = ~0;
              /*
               * Mahipati : //TODO
               * For session management reseting of this flag will create issues.
               * This need to be taken care.
               */
              old_item->flags &= ~SESSION_ADDED_IN_HW;

#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
              if (ppa_atomic_dec(&g_hw_session_cnt) == 0) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
                ppa_pwm_deactivate_module();
#endif
              }
#endif
              __ppa_session_put(old_item);
              goto SESSION_SWAP_CONTINUE;
            }
          }
#endif
        }

      SESSION_SWAP_CONTINUE:
		p_item->routing_entry = route.entry;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
		 p_item->sess_hash = p_item->routing_entry;
#endif
        p_item->flags |= SESSION_ADDED_IN_HW;
        p_item->collision_flag = route.collision_flag;

#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
        if (ppa_atomic_inc(&g_hw_session_cnt) == 1) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
          ppa_pwm_activate_module();
#endif
        }
#endif
        break;
      }
      //  fail in add_routing_entry
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                "fail in add_routing_entry\n");

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      if (route.flags & FLAG_SESSION_LOCK_FAIL) {
        p_item->flag2 |= SESSION_FLAG2_HW_LOCK_FAIL;
      }
#endif

      p_item->out_vlan_entry = ~0;
      if (p_item->caps_list[i].hal_id == PPE_HAL)
        ppa_hsel_del_outer_vlan_entry(&route.out_vlan_info, 0, PPE_HAL);
    OUT_VLAN_ERROR:
      p_item->src_mac_entry = ~0;
      if (p_item->caps_list[i].hal_id == PPE_HAL)
        ppa_drv_del_mac_entry(&route.src_mac, 0);
    NEW_SRC_MAC_ERROR:
      p_item->mtu_entry = ~0;
      if (p_item->caps_list[i].hal_id == PPE_HAL)
        ppa_drv_del_mtu_entry(&route.mtu_info, 0);
    MTU_ERROR:
      p_item->pppoe_entry = ~0;
      if (p_item->caps_list[i].hal_id == PPE_HAL)
        ppa_drv_del_pppoe_entry(&route.pppoe_info, 0);
    SESSION_VALID_PPPOE_ERROR:
      ret = PPA_FAILURE;
      break;
    case TUNNEL_L2TP_US:
    case TUNNEL_GRE_US:
    case TUNNEL_CAPWAP_US:
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
    case TUNNEL_IPSEC_DS:
#endif
	{
	printk("%d\n",__LINE__);
      if ((ret = ppa_hsel_add_complement(&route, 0, p_item->caps_list[i].hal_id)) != PPA_SUCCESS) {
        ret = PPA_FAILURE;
	}
      }
      // TBD: fill the datastructure and call the API
      break;
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	case TUNNEL_IPSEC_US:
	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;
        route.tnnl_info.tunnel_idx = p_item->tunnel_idx;
	route.f_is_lan = 1;
	route.entry = p_item->routing_entry;
        route.sessionAction = p_item->sessionAction;
      if ((ret = ppa_hsel_add_complement(&route, 0, p_item->caps_list[i].hal_id)) != PPA_SUCCESS) {
        ret = PPA_FAILURE;
      } else
		p_item->flags |= SESSION_ADDED_IN_HW;

      // TBD: fill the datastructure and call the API
      break;
#endif
#if 0 /*
       * Mahipati: Now single pass method
       * Two pass method is scrapped for GRE downstream, so following 
       * code is not required as of now.
       */
    case TUNNEL_GRE_DS:
      if (p_item->caps_list[0].hal_id == PAE_HAL) {
        struct netif_info* pnetinfo; 
        ret = PPA_FAILURE;
        if( PPA_SUCCESS == ppa_netif_lookup(p_item->rx_if->name, &pnetinfo)) {

          route.nFlowId = pnetinfo->greInfo.flowId << 6;
          ppa_memset(route.new_dst_mac, 0, PPA_ETH_ALEN);
          ppa_memset(route.src_mac.mac, 0, PPA_ETH_ALEN);
          route.route_type = PPA_ROUTE_TYPE_NULL;
          ret = ppa_hsel_add_routing_entry(&route, 0, PAE_HAL);
          p_item->cp_rt_index = route.entry;
          ppa_netif_put(pnetinfo);
        }
      }
      break;
#endif

    case TUNNEL_ENCRYPT:
    case TUNNEL_DECRYPT:
      // TBD: fill the datastructure and call the API
      break;
    default:
      break;
    }
    // if it returns failure 
    if (ret != PPA_SUCCESS) {
      // revert all the settings done from caps_list[0] to caps_list[i-1]
      for (k = 0; k < i; k++) {
        l = ppa_group_hals_in_capslist(k, i, p_item->caps_list);
        if (p_item->caps_list[k].cap == SESS_IPV4
            || p_item->caps_list[k].cap == SESS_IPV6) {
          ppa_hsel_del_routing_entry(&route, 0,
                                     p_item->caps_list[k].hal_id);
          p_item->routing_entry = ~0;
          p_item->flags &= ~SESSION_ADDED_IN_HW;

#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
          if (ppa_atomic_dec(&g_hw_session_cnt) == 0) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
            ppa_pwm_deactivate_module();
#endif
          }
#endif
        } else if (p_item->caps_list[k].cap == TUNNEL_L2TP_US
                   || p_item->caps_list[k].cap == TUNNEL_CAPWAP_US) {
          // TBD: call the fn to delete the tunnel configuration 
        }
        k += l;
      }
      //find the next HAL registred for this capability
      if (ppa_select_hals_from_caplist(i, j, p_item->caps_list) !=
          PPA_SUCCESS) {
        // this session cannot be added to the HW
        return PPA_FAILURE;
      } else {
        i = 0;
        continue;
      }
    }
    // returned success in step 1 proceed
    i += j;
  }

  p_item->num_caps = num_caps;
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
  update_session_mgmt_stats(p_item, ADD);
#endif

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
            "\n%s %s %d add entry success\n sip=%u\n dip=%u\n sp=%u\n dp=%u\n protocol_tcp=%d\n src_port=%0x\n dest_port=%0x\n\
dest_subifid=%0x\n p_item->sess_hash=%u\n session=%p\n mtu=%d num_caps=%d \ntimeout=%d \n", __FILE__,
            __FUNCTION__, __LINE__, p_item->src_ip.ip, p_item->dst_ip.ip, p_item->src_port, p_item->dst_port, p_item->flags & SESSION_IS_TCP, p_item->src_ifid, p_item->dest_ifid, p_item->dest_subifid, p_item->sess_hash,
            p_item->session, p_item->mtu, p_item->num_caps, p_item->timeout);
#endif

  return PPA_SUCCESS;
}

/*****************************************************************************************/
// this function will be called from ppa_api_session.c when a session is getting deleted 
/*****************************************************************************************/
void ppa_hsel_del_routing_session(struct session_list_item *p_item)
{
  PPE_ROUTING_INFO route;
  uint32_t i, j;

  uint32_t num_caps = 0;


	//printk("<%s> %d\n",__FUNCTION__,__LINE__);
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
  if(!(p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND_SA)) {
#endif
    if ( !((p_item->flags & SESSION_ADDED_IN_HW) && p_item->num_caps)) {
      return;
    }
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
  }
#endif

  ppa_memset(&route,0,sizeof(route));

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
              "\ndel entry \nsip=%u \ndip=%u \nsp=%u \ndp=%u \nprotocol_tcp=%d \ndest_port=%0x \ndest_subifid=%0x \nsess_hash=%u \nsession=%p num_caps=%d\n",
              p_item->src_ip.ip, p_item->dst_ip.ip, p_item->src_port,
              p_item->dst_port, p_item->flags & SESSION_IS_TCP,
              p_item->dest_ifid, p_item->dest_subifid, p_item->sess_hash,
              p_item->session, p_item->num_caps);
#endif

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
		  update_session_mgmt_stats(p_item, DELETE);
#endif

  //  when init, these entry values are ~0, the max the number which can be detected by these functions
  route.bytes = 0;
  route.f_hit = 0;
  // read the hit status from the HAL which handles the routing for this session

  if (p_item->caps_list[0].hal_id == PAE_HAL) {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    route.entry = p_item->sess_hash;
#else
    route.entry = p_item->routing_entry;
#endif
  } else {
    route.entry = p_item->routing_entry;
  }
  
  if (p_item->caps_list[0].hal_id == MPE_HAL) {
 
    if (p_item->flags & SESSION_IS_IPV6) {
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
      route.src_ip.f_ipv6 = 1;
      ppa_memcpy(route.src_ip.ip.ip6, p_item->src_ip.ip6,
                 sizeof(route.src_ip.ip.ip6));
      route.dst_ip.f_ipv6 = 1;
      ppa_memcpy(route.dst_ip.ip.ip6, p_item->dst_ip.ip6,
                 sizeof(route.dst_ip.ip.ip6));
#endif
    } else {
      route.src_ip.ip.ip = p_item->src_ip.ip;
      route.dst_ip.ip.ip = p_item->dst_ip.ip;
    }

    route.f_is_lan = (p_item->flags & SESSION_LAN_ENTRY) ? 1 : 0;
    ppa_memcpy(route.new_dst_mac, p_item->dst_mac, PPA_ETH_ALEN);
    route.dst_port = p_item->dst_port;
    route.src_port = p_item->src_port;
    route.f_is_tcp = p_item->flags & SESSION_IS_TCP;
  }

  ppa_hsel_get_routing_entry_bytes(&route, 0, p_item->caps_list[0].hal_id);

  if (route.f_hit) {
    uint64_t tmp;

    // read the statistics from HAL
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    tmp = route.bytes;
#else

    if ((uint32_t) route.bytes >= (uint32_t) p_item->last_bytes)
      tmp = (route.bytes - (uint64_t) p_item->last_bytes);
    else
      tmp =
          (route.bytes + (uint64_t) WRAPROUND_SESSION_MIB -
           (uint64_t) p_item->last_bytes);
#endif

    /* reset the accelerated counters, as it is going to be deleted */
    p_item->acc_bytes = 0;
    p_item->last_bytes = 0;

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
    //update mib interfaces
    update_netif_hw_mib(ppa_get_netif_name(p_item->rx_if), tmp, 1);
    if (p_item->br_rx_if)
      update_netif_hw_mib(ppa_get_netif_name(p_item->br_rx_if), tmp,
                          1);
    update_netif_hw_mib(ppa_get_netif_name(p_item->tx_if), tmp, 0);
    if (p_item->br_tx_if)
      update_netif_hw_mib(ppa_get_netif_name(p_item->br_tx_if), tmp,
                          0);
#endif
  }

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
  if((p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_INBOUND)) 
    	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC; 

  if((p_item->flag2 & SESSION_FLAG2_VALID_IPSEC_OUTBOUND_SA)) 
  {
	ppa_memset(p_item->caps_list, 0,
             sizeof(PPA_HSEL_CAP_NODE) * MAX_RT_SESS_CAPS);

  	num_caps = ppa_form_capability_list(p_item);

////////////////////////////////////////////////////////////////
// for each capability find out the HAL  
////////////////////////////////////////////////////////////////
  	if (ppa_select_hals_from_caplist(0, num_caps, p_item->caps_list) !=
      		PPA_SUCCESS) {
    	return PPA_FAILURE;
  	}
  	p_item->num_caps = num_caps;
  }
#endif


  for (i = 0; i < p_item->num_caps;) {

    j = ppa_group_hals_in_capslist(i, p_item->num_caps, p_item->caps_list);

    switch(p_item->caps_list[i].cap)
    {
      case SESS_IPV4:
      case SESS_IPV6:
        {

          ppa_hsel_del_routing_entry(&route, 0, p_item->caps_list[i].hal_id);
#if 0     /*
           * Mahipati: Now single pass method
           */
          if(p_item->cp_rt_index) {
            route.entry = p_item->cp_rt_index;
            printk("Deleting route entry %d for complimentary\n",p_item->cp_rt_index);
            ppa_hsel_del_routing_entry(&route, 0, p_item->caps_list[i].hal_id);
            p_item->cp_rt_index = 0;
          }
#endif
          route.tnnl_info.tunnel_idx = p_item->tunnel_idx;
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	  if(route.tnnl_info.tunnel_type != TUNNEL_TYPE_IPSEC)
          	p_item->tunnel_idx =  ~0;
#endif
          if ( ppa_is_6rdSession(p_item) ) {
            route.tnnl_info.tunnel_type = TUNNEL_TYPE_6RD;
          } else if (ppa_is_DsLiteSession(p_item)) {
            route.tnnl_info.tunnel_type = TUNNEL_TYPE_DSLITE;
          }
   
          if (p_item->caps_list[i].hal_id == PPE_HAL) {

            route.out_vlan_info.vlan_entry = p_item->out_vlan_entry;
            ppa_hsel_del_outer_vlan_entry(&route.out_vlan_info, 0, PPE_HAL);
            p_item->out_vlan_entry = ~0;

            route.pppoe_info.pppoe_ix = p_item->pppoe_entry;
            ppa_drv_del_pppoe_entry(&route.pppoe_info, 0);
            p_item->pppoe_entry = ~0;

            route.mtu_info.mtu_ix = p_item->mtu_entry;
            ppa_drv_del_mtu_entry(&route.mtu_info, 0);
            p_item->mtu_entry = ~0;

            route.src_mac.mac_ix = p_item->src_mac_entry;
            ppa_drv_del_mac_entry(&route.src_mac, 0);
            p_item->src_mac_entry = ~0;

            if( route.tnnl_info.tunnel_idx != ~0 )
              ppa_hsel_del_tunnel_entry(&route.tnnl_info, 0, PPE_HAL);
          } else {

            if( route.tnnl_info.tunnel_idx != ~0 )
              ppa_del_tunnel_tbl_entry(&route.tnnl_info);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
            ppa_bridge_entry_delete(p_item->s_mac, NULL, PPA_F_STATIC_ENTRY);
#endif
          }

          p_item->routing_entry = ~0;
          p_item->flags &= ~SESSION_ADDED_IN_HW;

#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
          if (ppa_atomic_dec(&g_hw_session_cnt) == 0) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
            ppa_pwm_deactivate_module();
#endif
          }
#endif
          break;
        }
      case TUNNEL_L2TP_US:
      case TUNNEL_GRE_US:
      case TUNNEL_CAPWAP_US:
        {
          //ppa_hsel_test_and_clear_hit_stat( &route, 0, caps_list[0].hal_id);
          // TBD: call the fn to delete the tunnel configuration 
          if (ppa_hsel_del_complement(&route, 0, p_item->caps_list[i].hal_id) != PPA_SUCCESS) {
            printk("Failed to delete Complementary entry!!!\n");
          }

          break;
        }

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
	case TUNNEL_IPSEC_DS:
	{
	printk("%d\n",__LINE__);
	route.f_is_lan = 0;
	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;
        route.tnnl_info.tunnel_idx = p_item->tunnel_idx;
      if ((ppa_hsel_del_complement(&route, 0, p_item->caps_list[i].hal_id)) != PPA_SUCCESS) {
            printk("Failed to delete Complementary entry!!!\n");
	}
	break;
      }

	case TUNNEL_IPSEC_US:
	{
	printk("<%s> %d\n",__FUNCTION__,__LINE__);
	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;
        route.tnnl_info.tunnel_idx = p_item->tunnel_idx;
	route.entry = p_item->routing_entry;
	route.f_is_lan = 1;
      if ((ppa_hsel_del_complement(&route, 0, p_item->caps_list[i].hal_id)) != PPA_SUCCESS) {
            printk("Failed to delete Complementary entry!!!\n");
      }
	break;
	}
#endif
      case TUNNEL_ENCRYPT:
      case TUNNEL_DECRYPT:
        {
          // TBD: call the fn to delete the tunnel configuration 
          break;
        }
      default:
        break;
    }
    i += j;
  }
  p_item->num_caps = 0;
}

/*****************************************************************************************/
// this function will be called from ppa_modify_session fn
// only the following feilds can be modified on an on going session
// DSCP 
// mtu 
// out VLAN handling
// inner vlan handling
// pppoe header
// This needs to be supported by PAE and MPE which has the capability SESS_IPV4 or SESS_IPV6
/*****************************************************************************************/
int32_t ppa_hsel_update_routing_session(struct session_list_item *p_item,
                                        uint32_t flags)
{
  PPE_ROUTING_INFO route = { 0 };

  route.mtu_info.mtu_ix = ~0;
  route.pppoe_info.pppoe_ix = ~0;
  route.out_vlan_info.vlan_entry = ~0;

  if ((flags & PPA_F_SESSION_NEW_DSCP))
    route.update_flags |=
        PPA_UPDATE_ROUTING_ENTRY_NEW_DSCP_EN |
        PPA_UPDATE_ROUTING_ENTRY_NEW_DSCP;

  if ((flags & PPA_F_SESSION_VLAN))
    route.update_flags |=
        PPA_UPDATE_ROUTING_ENTRY_VLAN_INS_EN |
        PPA_UPDATE_ROUTING_ENTRY_NEW_VCI |
        PPA_UPDATE_ROUTING_ENTRY_VLAN_RM_EN;

  if ((flags & PPA_F_PPPOE)) {
    uint8_t f_new_pppoe = 0;
    if (p_item->pppoe_session_id == 0) {        //need to disable pppoe flag, ie, change to PPPOE transparent
      if (p_item->pppoe_entry != ~0) {
        if (p_item->caps_list[0].hal_id == PPE_HAL)
          ppa_drv_del_pppoe_entry(&route.pppoe_info, 0);
        p_item->pppoe_entry = ~0;
      }

      route.pppoe_mode = 0;
      route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_PPPOE_MODE;
      route.pppoe_info.pppoe_ix = 0;
      route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_PPPOE_IX;
    } else {                    //need to add or replace old pppoe flag
      if (p_item->caps_list[0].hal_id == PPE_HAL) {
        if (p_item->pppoe_entry != ~0) {        //already have pppoe entry. so check whether need to replace or not
          route.pppoe_info.pppoe_ix = p_item->pppoe_entry;
          if (ppa_drv_get_pppoe_entry(&route.pppoe_info, 0) == PPA_SUCCESS) {
            if (route.pppoe_info.pppoe_session_id !=
                p_item->pppoe_session_id) {
              ppa_drv_del_pppoe_entry(&route.pppoe_info, 0);
              p_item->pppoe_entry = ~0;
              f_new_pppoe = 1;
            }
          }
        } else {
          f_new_pppoe = 1;
        }
        if (f_new_pppoe) {
          //  create new PPPOE entry
          route.pppoe_info.pppoe_session_id = p_item->pppoe_session_id;
          if (ppa_drv_add_pppoe_entry(&route.pppoe_info, 0) == PPA_SUCCESS) {
            //  success
            p_item->pppoe_entry = route.pppoe_info.pppoe_ix;
            route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_PPPOE_IX;

            route.pppoe_mode = 1;
            route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_PPPOE_MODE;
          } else
            return PPA_EAGAIN;
        }
      } else {
        route.pppoe_info.pppoe_session_id = p_item->pppoe_session_id;
        route.pppoe_mode = 1;
        route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_PPPOE_MODE;
        route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_PPPOE_IX;
      }
    }
  }

  if ((flags & PPA_F_MTU)) {
    if (p_item->caps_list[0].hal_id == PPE_HAL) {
      route.mtu_info.mtu_ix = p_item->mtu_entry;
      if (ppa_drv_get_mtu_entry(&route.mtu_info, 0) == PPA_SUCCESS) {
        if (route.mtu_info.mtu == p_item->mtu) {
          //  entry not changed
          route.mtu_info.mtu_ix = p_item->mtu_entry;
          goto PPA_HW_UPDATE_SESSION_EXTRA_MTU_GOON;
        } else {
          //  entry changed, so delete old first and create new one later
          ppa_drv_del_mtu_entry(&route.mtu_info, 0);
          p_item->mtu_entry = ~0;
        }
      }
      //  create new MTU entry
      route.mtu_info.mtu = p_item->mtu;
      if (ppa_drv_add_mtu_entry(&route.mtu_info, 0) == PPA_SUCCESS) {
        //  success
        p_item->mtu_entry = route.mtu_info.mtu_ix;
        route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_MTU_IX;
      } else
        return PPA_EAGAIN;
    } else {
      route.mtu_info.mtu = p_item->mtu;
      route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_MTU_IX;
    }
  }
PPA_HW_UPDATE_SESSION_EXTRA_MTU_GOON:

  if ((flags & PPA_F_SESSION_OUT_VLAN)) {
    if (p_item->caps_list[0].hal_id == PPE_HAL) {
      route.out_vlan_info.vlan_entry = p_item->out_vlan_entry;
      if (ppa_hsel_get_outer_vlan_entry(&route.out_vlan_info, 0, PPE_HAL)
          == PPA_SUCCESS) {
        if (route.out_vlan_info.vlan_id == p_item->out_vlan_tag) {
          //  entry not changed
          goto PPA_HW_UPDATE_SESSION_EXTRA_OUT_VLAN_GOON;
        } else {
          //  entry changed, so delete old first and create new one later                
          ppa_hsel_del_outer_vlan_entry(&route.out_vlan_info, 0, PPE_HAL);
          p_item->out_vlan_entry = ~0;
        }
      }
      //  create new OUT VLAN entry
      route.out_vlan_info.vlan_id = p_item->out_vlan_tag;
      if (ppa_hsel_add_outer_vlan_entry(&route.out_vlan_info, 0, PPE_HAL)
          == PPA_SUCCESS) {
        p_item->out_vlan_entry = route.out_vlan_info.vlan_entry;
        route.update_flags |= PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_IX;
      } else
        return PPA_EAGAIN;

      route.update_flags |=
          PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_INS_EN |
          PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_RM_EN;
    } else {
      route.out_vlan_info.vlan_id = p_item->out_vlan_tag;
      route.update_flags |=
          PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_INS_EN |
          PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_RM_EN;
    }
  }
PPA_HW_UPDATE_SESSION_EXTRA_OUT_VLAN_GOON:

  route.entry = p_item->routing_entry;
//  route.f_new_dscp_enable = p_item->flags & SESSION_VALID_NEW_DSCP;
  if ((p_item->flags & SESSION_VALID_NEW_DSCP))
  {
    route.f_new_dscp_enable = 1;
    route.new_dscp = p_item->new_dscp;
  }

  route.f_vlan_ins_enable = p_item->flags & SESSION_VALID_VLAN_INS;
  route.new_vci = p_item->new_vci;

  route.f_vlan_rm_enable = p_item->flags & SESSION_VALID_VLAN_RM;

  route.f_out_vlan_ins_enable = p_item->flags & SESSION_VALID_OUT_VLAN_INS;
  route.out_vlan_info.vlan_entry = p_item->out_vlan_entry,
      route.f_out_vlan_rm_enable =
      p_item->flags & SESSION_VALID_OUT_VLAN_RM;

  return ppa_hsel_update_routing_entry(&route, 0,
                                       p_item->caps_list[0].hal_id);
}

/*****************************************************************************************/
// The statistics needs to be updated to p_item periodically by a timer thread
// This function is called from the timer thread periodically
// for a routed session statics needs to be read from the HAL that does the routing fn
// The for each session p_item->caps_list[0] will always give us the HAL id which does the routing 
/*****************************************************************************************/
#if 0                           /* Unused, use ppa_check_hit_stat_clear_mib() */
void ppa_hsel_check_hit_stat_clear_mib(int32_t flag)
{
  struct session_list_item *p_item = NULL;
  struct mc_group_list_item *p_mc_item = NULL;

  uint32_t i;
  PPE_ROUTING_INFO route = { 0 };
  PPE_MC_INFO mc = { 0 };
  uint64_t tmp = 0;

  ppa_uc_get_htable_lock();
  for (i = 0; i < NUM_ENTITY(g_session_list_hash_table); i++) {
    ppa_hlist_for_each_entry(p_item, &g_session_list_hash_table[i], hlist) {

      tmp = 0;
      route.entry = p_item->sess_hash;

      if (flag & PPA_CMD_CLEAR_PORT_MIB) {
        p_item->acc_bytes = 0;
        p_item->mips_bytes = 0;
        p_item->last_bytes = 0;
        p_item->prev_sess_bytes = 0;
        p_item->prev_clear_acc_bytes = 0;
        p_item->prev_clear_mips_bytes = 0;
        if (p_item->flags & SESSION_ADDED_IN_HW) {
          route.bytes = 0;
          route.f_hit = 0;
          // TBD: we have to read the routing entry bytes from the HAL that does capability SESS_IPV4 or SESS_IPV6 for this session
          // the first entry in the p_item->caps_list will give this information 
          // ppa_hsel_test_and_clear_hit_stat(&route, 0, caps_list[0].hal_id);  /* clear hit */
          // above fn call is not needed as the hit status is also updated byt the below fn
          ppa_hsel_get_routing_entry_bytes(&route, flag, p_item->caps_list[0].hal_id);  /* clear session mib */
        }
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
        if (p_item->rx_if)
          update_netif_mib(ppa_get_netif_name(p_item->rx_if), tmp, 1, 1);
        if (p_item->br_rx_if)
          update_netif_mib(ppa_get_netif_name(p_item->br_rx_if), tmp, 1,
                           1);
        if (p_item->tx_if)
          update_netif_mib(ppa_get_netif_name(p_item->tx_if), tmp, 0, 1);
        if (p_item->br_tx_if)
          update_netif_mib(ppa_get_netif_name(p_item->br_tx_if), tmp, 0,
                           1);
#endif

        continue;
      }

      if (!(p_item->flags & SESSION_ADDED_IN_HW))
        continue;

      route.bytes = 0;
      route.f_hit = 0;
      ppa_hsel_get_routing_entry_bytes(&route, flag,
                                       p_item->caps_list[0].hal_id);
      if (route.bytes > WRAPROUND_SESSION_MIB) {
        err("why route.bytes(%llu) > WRAPROUND_SESSION_MIB(%llu)\n",
            route.bytes, (uint64_t) WRAPROUND_SESSION_MIB);
        printk("why route.bytes(%llu) > WRAPROUND_SESSION_MIB(%llu)\n",
               route.bytes, (uint64_t) WRAPROUND_SESSION_MIB);
      }

      if (route.f_hit) {
        p_item->last_hit_time = ppa_get_time_in_sec();
        if ((uint32_t) (route.bytes) >= p_item->last_bytes)
          tmp = route.bytes - (uint64_t) p_item->last_bytes;
        else
          tmp =
              route.bytes + (uint64_t) WRAPROUND_SESSION_MIB -
              (uint64_t) p_item->last_bytes;
        if (tmp >= (uint64_t) WRAPROUND_SESSION_MIB)
          err("The handling of Session bytes wrappround wrongly \n");
        p_item->acc_bytes += tmp;
        p_item->last_bytes = route.bytes;

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
        if (p_item->rx_if)
          update_netif_mib(ppa_get_netif_name(p_item->rx_if), tmp, 1, 0);
        if (p_item->br_rx_if)
          update_netif_mib(ppa_get_netif_name(p_item->br_rx_if), tmp, 1,
                           0);
        if (p_item->tx_if)
          update_netif_mib(ppa_get_netif_name(p_item->tx_if), tmp, 0, 0);
        if (p_item->br_tx_if)
          update_netif_mib(ppa_get_netif_name(p_item->br_tx_if), tmp, 0,
                           0);
#endif
      }
    }
  }
  ppa_uc_release_htable_lock();


  ppa_mc_get_htable_lock();
  for (i = 0; i < NUM_ENTITY(g_mc_group_list_hash_table); i++) {

    ppa_hlist_for_each_entry(p_mc_item, &g_mc_group_list_hash_table[i],
                             mc_hlist) {

      tmp = 0;
      mc.p_entry = p_mc_item->mc_entry;

      if (flag & PPA_CMD_CLEAR_PORT_MIB) {
        if (p_mc_item->flags & SESSION_ADDED_IN_HW) {
          ppa_hsel_get_mc_entry_bytes(&mc, flag, p_mc_item->caps_list[0].hal_id);       /* flag means clear mib counter or not */
          p_mc_item->acc_bytes = 0;
          p_mc_item->last_bytes = 0;
          mc.f_hit = 0;
          mc.bytes = 0;
        }
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
        if (p_mc_item->src_netif)
          update_netif_mib(ppa_get_netif_name(p_mc_item->src_netif), tmp,
                           1, 1);

        for (i = 0; i < p_mc_item->num_ifs; i++)
          if (p_mc_item->netif[i])
            update_netif_mib(ppa_get_netif_name(p_mc_item->netif[i]), tmp,
                             0, 1);
#endif

        continue;
      }

      if (!(p_mc_item->flags & SESSION_ADDED_IN_HW))
        continue;

      mc.f_hit = 0;
      mc.bytes = 0;
      // above fn call is not needed as the hit status is also updated byt the below fn
      ppa_hsel_get_mc_entry_bytes(&mc, flag,
                                  p_mc_item->caps_list[0].hal_id);
      if (mc.f_hit) {
        p_mc_item->last_hit_time = ppa_get_time_in_sec();

        if ((uint32_t) mc.bytes >= p_mc_item->last_bytes) {
          tmp = mc.bytes - (uint64_t) p_mc_item->last_bytes;
        } else {
          tmp =
              mc.bytes + (uint64_t) WRAPROUND_32BITS -
              (uint64_t) p_mc_item->last_bytes;
        }
        p_mc_item->acc_bytes += tmp;

        p_mc_item->last_bytes = mc.bytes;

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
        if (p_mc_item->src_netif)
          update_netif_mib(ppa_get_netif_name(p_mc_item->src_netif), tmp,
                           1, 0);

        for (i = 0; i < p_mc_item->num_ifs; i++)
          if (p_mc_item->netif[i])
            update_netif_mib(ppa_get_netif_name(p_mc_item->netif[i]), tmp,
                             0, 0);
#endif
      }
    }
  }
  ppa_mc_release_htable_lock();

}
#endif                          /* #if 0 */

static int32_t ppa_mc_itf_get(char *itf, struct netif_info **pp_netif_info)
{
    if(!itf){
        return PPA_FAILURE;
    }

    if( ppa_netif_update(NULL, itf) != PPA_SUCCESS )
        return PPA_FAILURE;

    return ppa_netif_lookup(itf, pp_netif_info);

}


int32_t update_subif_port_info(struct mc_group_list_item *p_item, uint32_t *dest_list, uint32_t *dest_list_vap, uint16_t *subif)
{
	int32_t i;
	struct netif_info *p_netif_info = NULL;

	for(i = 0; i < PPA_MAX_MC_IFS_NUM; i ++){
        	if(ppa_mc_itf_get(p_item->netif[i]->name, &p_netif_info) != PPA_SUCCESS
        	    || !(p_netif_info->flags & NETIF_PHYS_PORT_GOT) ){
        	    return PPA_FAILURE;
        	}
		//printk("port=%d p_netif_info->subif_id=%d\n", p_netif_info->phys_port, p_netif_info->subif_id);	
		if ((p_netif_info->flags & NETIF_DIRECTPATH) || (p_netif_info->flags & NETIF_DIRECTCONNECT)) {
			*dest_list_vap |= (1 << p_netif_info->phys_port);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
			subif[p_netif_info->phys_port] |= (1 << (p_netif_info->subif_id >> 8));
#endif
		} else {
			*dest_list |= 1 << p_netif_info->phys_port;
		}

	}
}
/*****************************************************************************************/
// This function will be called when a new group is added 
// replaces ppa_hw_add_mc_group
/*****************************************************************************************/
int32_t ppa_hsel_add_wan_mc_group(struct mc_group_list_item *p_item)
{
//  find the capabilities needed for this session
//  call the HAL apis as needed
  PPE_MC_INFO mc = { 0 };
  uint32_t num_caps = 0, i = 0, j = 0, idx, l_dest_list=0, l_dest_list_vap=0;
  uint8_t f_more_hals = 0;
  int ret = PPA_SUCCESS;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  struct mc_group_list_item *old_item;
#endif

  ppa_memset(&mc, 0, sizeof(mc));
  mc.out_vlan_info.vlan_entry = ~0;
  mc.route_type =
      p_item->bridging_flag ? PPA_ROUTE_TYPE_NULL : PPA_ROUTE_TYPE_IPV4;

  // Must be LAN port
  // TBD: we have to prepare the proper destination mask based on the destination interfaces
  mc.dest_list = p_item->dest_ifid;

  ppa_memset(p_item->caps_list, 0,
             MAX_MC_SESS_CAPS * sizeof(PPA_HSEL_CAP_NODE));
////////////////////////////////////////////////////////////////    
//  find all the capabilities needed by this session
////////////////////////////////////////////////////////////////

  // must be multicast downstream session 
  p_item->caps_list[num_caps++].cap = SESS_MC_DS;
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
  // check the number of directconnect VAP
  if (p_item->num_vap > 1) {

    update_subif_port_info(p_item, &l_dest_list, &l_dest_list_vap, mc.dest_subif);
    // for additional capability requirement
    p_item->caps_list[num_caps++].cap = SESS_MC_DS_VAP;
#if defined CONFIG_LTQ_PPA_MPE_HAL || defined CONFIG_LTQ_PPA_MPE_HAL_MODULE
      mc.sessionAction = (struct session_action *) ppa_session_mc_construct_tmplbuf(p_item, l_dest_list_vap);
#endif

  }
#endif
////////////////////////////////////////////////////////////////
// for each capability find out the HAL  
////////////////////////////////////////////////////////////////
  if (ppa_select_hals_from_caplist(0, num_caps, p_item->caps_list) !=
      PPA_SUCCESS) {
    return PPA_FAILURE;
  }

  for (i = 0; i < num_caps;) {
    f_more_hals = 0;
    j = ppa_group_hals_in_capslist(i, num_caps, p_item->caps_list);

    // Based on the capability of first entry in the list we can decide the action
    switch (p_item->caps_list[i].cap) {
    case SESS_MC_DS:

#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
  	if (p_item->num_vap > 1) {

       	  mc.dest_list = 1;
       	  mc.dest_list |= l_dest_list;
	}
#endif
      //copy the session hash

      if ((p_item->flags & SESSION_VALID_OUT_VLAN_INS)) {
        mc.out_vlan_info.vlan_id = p_item->out_vlan_tag;
        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_hsel_add_outer_vlan_entry(&mc.out_vlan_info, 0, PPE_HAL)
              == 0) {
            p_item->out_vlan_entry = mc.out_vlan_info.vlan_entry;
          } else {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                      "add out_vlan_ix error\n");
            goto OUT_VLAN_ERROR;
          }
        }
      }

      if (p_item->caps_list[i].hal_id == PAE_HAL) {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        mc.group_id = p_item->group_id;
        mc.subif_id = p_item->dest_subifid;
        mc.src_port = p_item->src_port;
        mc.dst_port = p_item->dst_port;
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
        mc.num_vap = p_item->num_vap;
#endif
        if(p_item->src_netif)
	    ppa_bridge_entry_add(p_item->s_mac, NULL, p_item->src_netif, PPA_F_STATIC_ENTRY);
#endif
      }

      if (p_item->ip_mc_group.f_ipv6) { //ipv6
        ppa_memcpy(mc.dst_ipv6_info.ip.ip6, p_item->ip_mc_group.ip.ip6,
                   sizeof(IP_ADDR));
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && defined(CONFIG_LTQ_PPA_GRX500)
        mc.f_ipv6 = 1;
#endif
        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_drv_add_ipv6_entry(&mc.dst_ipv6_info, 0) == PPA_SUCCESS) {
            p_item->dst_ipv6_entry = mc.dst_ipv6_info.ipv6_entry;
            mc.dest_ip_compare = mc.dst_ipv6_info.psuedo_ip;
          } else {
            goto DST_IPV6_ERROR;
          }
        }
      } else {                  //ipv4
        mc.dest_ip_compare = p_item->ip_mc_group.ip.ip;
      }

      if (is_ip_zero(&p_item->source_ip)) {
        mc.src_ip_compare = PSEUDO_MC_ANY_SRC;
      } else if (p_item->source_ip.f_ipv6) {    //ipv6
        ppa_memcpy(mc.src_ipv6_info.ip.ip6, p_item->source_ip.ip.ip6,
                   sizeof(IP_ADDR));
        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_drv_add_ipv6_entry(&mc.src_ipv6_info, 0) == PPA_SUCCESS) {
            p_item->src_ipv6_entry = mc.src_ipv6_info.ipv6_entry;
            mc.src_ip_compare = mc.src_ipv6_info.psuedo_ip;
          } else {
            goto SRC_IPV6_ERROR;
          }
        }
      } else {
        mc.src_ip_compare = p_item->source_ip.ip.ip;
      }

      ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "mc group ip:%u.%u.%u.%u\n",
                NIPQUAD(mc.dest_ip_compare));
      ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "mc src ip:%u.%u.%u.%u\n",
                NIPQUAD(mc.src_ip_compare));

      mc.f_vlan_ins_enable = p_item->flags & SESSION_VALID_VLAN_INS;
      mc.new_vci = p_item->new_vci;
      mc.f_vlan_rm_enable = p_item->flags & SESSION_VALID_VLAN_RM;
      mc.f_src_mac_enable = p_item->flags & SESSION_VALID_SRC_MAC;
      mc.pppoe_mode = p_item->flags & SESSION_VALID_PPPOE;
      mc.f_out_vlan_ins_enable =
          p_item->flags & SESSION_VALID_OUT_VLAN_INS;
      mc.out_vlan_info.vlan_entry = p_item->out_vlan_entry;
      mc.f_out_vlan_rm_enable = p_item->flags & SESSION_VALID_OUT_VLAN_RM;
      mc.f_new_dscp_enable = p_item->flags & SESSION_VALID_NEW_DSCP;
      mc.f_tunnel_rm_enable = p_item->flags & (SESSION_TUNNEL_6RD | SESSION_TUNNEL_DSLITE);     //for only downstream multicast acceleration
      mc.new_dscp = p_item->new_dscp;
      mc.dest_qid = p_item->dslwan_qid;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      mc.flags |= FLAG_SESSION_HI_PRIO;
#endif

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
      mc.sample_en = p_item->RTP_flag;        /*!< rtp flag */
#endif
      // check whether more HALs needs to be configured to complete this session configuration
      if (i + j != num_caps) {  // more HALs are there in the caps_list
        f_more_hals = 1;
        // TBD: setup flow between the Acceleration blocks
      }

     /* printk("<%s>mc.num_vap=%d mc.dest_list=%d p_item->dest_ifid=%d\n", __FUNCTION__,mc.num_vap, 
	mc.dest_list, p_item->dest_ifid);*/
      if ((ret =
           ppa_hsel_add_wan_mc_entry(&mc, 0,
                                     p_item->caps_list[i].hal_id)) ==
          PPA_SUCCESS) {
        if (p_item->caps_list[i].hal_id == PAE_HAL) {
          // in this case we have to check the flags to see whether the PAE has done any swap
          // mc->flags == 1 // TBD: Obtain the  actual flags from PAE header
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
          if (mc.flags & FLAG_SESSION_SWAPPED) {
            // TBD: search in the session list to find out the session with routing_entry == route.entry (same index) 
            // clear the flag SESSION_ADDED_IN_HW
            // make session->routing_entry = ~0;
            // decrement g_hw_session_cnt
            for (idx = 0; idx < NUM_ENTITY(g_mc_group_list_hash_table);
                 idx++) {
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,8,13)
              ppa_hlist_for_each_entry(old_item, node,
                                       &g_mc_group_list_hash_table[idx],
                                       mc_hlist)
#else
              ppa_hlist_for_each_entry(old_item,
                                       &g_mc_group_list_hash_table[idx],
                                       mc_hlist)
#endif
              {
                if (old_item->mc_entry == mc.p_entry) {
                  old_item->mc_entry = 0;
                  old_item->flags &= ~SESSION_ADDED_IN_HW;
#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
                  if (ppa_atomic_dec(&g_hw_session_cnt) == 0) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
                    ppa_pwm_deactivate_module();
#endif
                  }
#endif
                  goto SESSION_SWAP_CONTINUE;
                }
              }
            }
          }
#endif
        }
      SESSION_SWAP_CONTINUE:
		p_item->mc_entry = mc.p_entry;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        p_item->sess_hash = p_item->mc_entry;
#endif
        p_item->flags |= SESSION_ADDED_IN_HW;

#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
        if (ppa_atomic_inc(&g_hw_session_cnt) == 1) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
          ppa_pwm_activate_module();
#endif
        }
#endif
        break;
      }

      if (p_item->caps_list[i].hal_id == PPE_HAL) {
        ppa_drv_del_ipv6_entry(&mc.src_ipv6_info, 0);
        p_item->src_ipv6_entry = ~0;
      }
    SRC_IPV6_ERROR:
      if (p_item->caps_list[i].hal_id == PPE_HAL) {
        ppa_drv_del_ipv6_entry(&mc.dst_ipv6_info, 0);
        p_item->dst_ipv6_entry = ~0;
      }
    DST_IPV6_ERROR:
      p_item->out_vlan_entry = ~0;
      if (p_item->caps_list[i].hal_id == PPE_HAL)
        ppa_hsel_del_outer_vlan_entry(&mc.out_vlan_info, 0, PPE_HAL);
    OUT_VLAN_ERROR:
      ret = PPA_EAGAIN;

      break;
    case SESS_MC_DS_VAP:
      // TBD: complementary processing needs to be added
      //printk("Capability SESS_MC_DS_VAP\n");


      mc.dest_list = l_dest_list_vap;
      //printk("<%s> num_vap=%d dest_list=%d dest_ifid=%d group_id=%d\n", __FUNCTION__,mc.num_vap, mc.dest_list, p_item->dest_ifid, mc.group_id);
	if ((ret =
           ppa_hsel_add_wan_mc_entry(&mc, 0,
                                     p_item->caps_list[i].hal_id)) ==
          PPA_SUCCESS) {

	}

      break;
    default:
      break;
    }
    if (ret != PPA_SUCCESS) {
      // if the capability is SESS_MC_DS then we can check for any more HALs registred
      // TBD: if the capability needed is SESS_MC_DS_VAP we have to call a different API to forward the packet to CPU port / clear the MPE flag in the PAE configuration
      //find the next HAL registred for this capability
      if (ppa_select_hals_from_caplist(i, j, p_item->caps_list) !=
          PPA_SUCCESS) {
        // this session cannot be added to the HW
        return PPA_FAILURE;
      } else {
        continue;
      }
    }
// returned success in step 1 proceed
    i += j;
  }

  p_item->num_caps = num_caps;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
            "\n%s %s %d add mc entry success\n sip=%u\n dip=%u\n sp=%u\n dp=%u\n src_netif=%s\n dest_ifid=%0x\n\
dest_subifid=%0x\n group_id=%u\n p_item->sess_hash=%u\n num_caps=%d \nlast_hit_time=%d \n", __FILE__,
            __FUNCTION__, __LINE__, p_item->source_ip.ip.ip, p_item->ip_mc_group.ip.ip, p_item->src_port, p_item->dst_port, p_item->src_netif->name, p_item->dest_ifid, p_item->dest_subifid, p_item->group_id,
            p_item->sess_hash, p_item->num_caps, p_item->last_hit_time);
#endif
  return ret;
}

/*****************************************************************************************/
// called when the multicast group is deleted
// replaces ppa_hw_del_mc_group
/*TBD*******************re-visit**********************************/
void ppa_hsel_del_wan_mc_group(struct mc_group_list_item *p_item)
{
  PPE_MC_INFO mc = { 0 };
  PPE_OUT_VLAN_INFO out_vlan = { 0 };
  uint64_t tmp;
  uint32_t i, j, l_dest_list=0, l_dest_list_vap=0;

#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
  if (p_item->num_vap > 1) {
	update_subif_port_info(p_item, &l_dest_list, &l_dest_list_vap, mc.dest_subif);

    // for additional capability requirement
    //p_item->caps_list[num_caps++].cap = SESS_MC_DS_VAP;
 #if defined CONFIG_LTQ_PPA_MPE_HAL || defined CONFIG_LTQ_PPA_MPE_HAL_MODULE
	ppa_session_mc_destroy_tmplbuf(mc.sessionAction);

#endif

  }
#endif



  if ((p_item->flags & SESSION_ADDED_IN_HW)) {

    mc.p_entry = p_item->mc_entry;
    if (p_item->num_caps) {

      // only first HAL in the cap_list is needded tpo get the stats
      //ppa_hsel_test_and_clear_mc_hit_stat(&mc, 0, caps_list[0].hal_id);
      ppa_hsel_get_mc_entry_bytes(&mc, 0, p_item->caps_list[0].hal_id);
      if (mc.f_hit) {

        if ((uint32_t) mc.bytes >= p_item->last_bytes) {
          tmp = mc.bytes - (uint64_t) p_item->last_bytes;
        } else {
          tmp =
              mc.bytes + (uint64_t) WRAPROUND_32BITS -
              (uint64_t) p_item->last_bytes;
        }

        /* reset the accelerated counters, as it is going to be deleted */
        p_item->acc_bytes = 0;
        p_item->last_bytes = 0;

        if (p_item->src_netif)
          update_netif_hw_mib(ppa_get_netif_name(p_item->src_netif), tmp,
                              1);

        for (i = 0; i < p_item->num_ifs; i++)
          if (p_item->netif[i])
            update_netif_hw_mib(ppa_get_netif_name(p_item->netif[i]), tmp,
                                0);
      }
    } else {
      return;
    }

    //  when init, these entry values are ~0, the max the number which can be detected by these functions
    ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP,
              "ppa_hw_del_mc_group:  remove %d from HW\n",
              p_item->mc_entry);
    for (i = 0; i < p_item->num_caps;) {

      j = ppa_group_hals_in_capslist(i, p_item->num_caps,
                                     p_item->caps_list);

      if (p_item->caps_list[i].cap == SESS_MC_DS) {

        ppa_hsel_del_wan_mc_entry(&mc, 0, p_item->caps_list[i].hal_id);

        p_item->mc_entry = ~0;

        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (p_item->dst_ipv6_entry != ~0) {
            mc.dst_ipv6_info.ipv6_entry = p_item->dst_ipv6_entry;
            ppa_drv_del_ipv6_entry(&mc.dst_ipv6_info, 0);
            p_item->dst_ipv6_entry = ~0;
          }

          if (p_item->src_ipv6_entry != ~0) {
            mc.src_ipv6_info.ipv6_entry = p_item->src_ipv6_entry;
            ppa_drv_del_ipv6_entry(&mc.src_ipv6_info, 0);
            p_item->src_ipv6_entry = ~0;
          }
          //  taken from netif_info, so don't need to be removed from MAC table

          out_vlan.vlan_entry = p_item->out_vlan_entry;
          ppa_drv_del_outer_vlan_entry(&out_vlan, 0);
          p_item->out_vlan_entry = ~0;
        } else if (p_item->caps_list[i].hal_id == PAE_HAL) {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
          ppa_bridge_entry_delete(p_item->s_mac, NULL, PPA_F_STATIC_ENTRY);
#endif
        }

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        p_item->src_mac_entry = ~0;
#endif
        p_item->flags &= ~SESSION_ADDED_IN_HW;

#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
        if (ppa_atomic_dec(&g_hw_session_cnt) == 0) {
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
          ppa_pwm_deactivate_module();
#endif
        }
#endif
      } else if (p_item->caps_list[i].cap == SESS_MC_DS_VAP) {
        //TBD: undo the mpe  config

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        mc.group_id = p_item->group_id;
#endif
    	mc.p_entry = p_item->mc_entry;
        mc.dest_list = l_dest_list_vap;
	ppa_hsel_del_wan_mc_entry(&mc, 0, p_item->caps_list[i].hal_id);

      }

      i += j;
    }
  }
}

/*****************************************************************************************/
// called for each join/leave messages
// replaces ppa_hw_update_mc_group_extra
/*TBD*******************re-visit**********************************/
int32_t ppa_hsel_update_wan_mc_group(struct mc_group_list_item *p_item,
                                     uint32_t flags)
{
  PPE_MC_INFO mc;
  uint32_t update_flags = 0;
  uint32_t i = 0, j = 0;
  uint8_t f_more_hals = 0;
  int ret = PPA_SUCCESS;

  ppa_memset(&mc, 0, sizeof(mc));


  p_item->caps_list[0].cap = SESS_MC_DS;
  p_item->num_caps = 1;

#if 0
  // check the destination interfaces for VAPs to see the capability SESS_MC_DS_VAP is needed
  for (i = 0; i < p_item->num_ifs; i++) {
    //TBD: THIS FUNCTION TO BE DEFINED
    if (ppa_get_if_type(p_item->netif[i]) == PPA_SUBIF) {
      // chance for additional capability requirement
      p_item->caps_list[1].cap = SESS_MC_DS_VAP;
      p_item->num_caps = 2;
      break;
    }
  }
#endif

///////////////////////////////////////////////////////////////
// for each capability find out the HAL  
////////////////////////////////////////////////////////////////
  if (ppa_select_hals_from_caplist(0, p_item->num_caps, p_item->caps_list)
      != PPA_SUCCESS) {
    return PPA_FAILURE;
  }

  for (i = 0; i < p_item->num_caps;) {
    f_more_hals = 0;
    j = ppa_group_hals_in_capslist(i, p_item->num_caps, p_item->caps_list);

    // Based on the capability of first entry in the list we can decide the action
    switch (p_item->caps_list[i].cap) {
    case SESS_MC_DS:

      if ((flags & PPA_F_SESSION_NEW_DSCP))
        update_flags |=
            PPA_UPDATE_WAN_MC_ENTRY_NEW_DSCP_EN |
            PPA_UPDATE_WAN_MC_ENTRY_NEW_DSCP;

      if ((flags & PPA_F_SESSION_VLAN))
        update_flags |=
            PPA_UPDATE_WAN_MC_ENTRY_VLAN_INS_EN |
            PPA_UPDATE_WAN_MC_ENTRY_NEW_VCI |
            PPA_UPDATE_WAN_MC_ENTRY_VLAN_RM_EN;

      if ((flags & PPA_F_SESSION_OUT_VLAN)) {
        mc.out_vlan_info.vlan_entry = p_item->out_vlan_entry;
        if (p_item->caps_list[i].hal_id == PPE_HAL) {
          if (ppa_hsel_get_outer_vlan_entry(&mc.out_vlan_info, 0, PPE_HAL)
              == PPA_SUCCESS) {
            if (mc.out_vlan_info.vlan_id == p_item->out_vlan_tag) {
              //  entry not changed
              goto PPA_HW_UPDATE_MC_GROUP_EXTRA_OUT_VLAN_GOON;
            } else {
              //  entry changed, so delete old first and create new one later
              ppa_hsel_del_outer_vlan_entry(&mc.out_vlan_info, 0, PPE_HAL);
              p_item->out_vlan_entry = ~0;
            }
          }
          mc.out_vlan_info.vlan_id = p_item->out_vlan_tag;
          if (ppa_hsel_add_outer_vlan_entry(&mc.out_vlan_info, 0, PPE_HAL)
              == 0) {
            p_item->out_vlan_entry = mc.out_vlan_info.vlan_entry;
            update_flags |= PPA_UPDATE_WAN_MC_ENTRY_OUT_VLAN_IX;
          } else {
            ppa_debug(DBG_ENABLE_MASK_ERR, "add_outer_vlan_entry fail\n");
            goto OUT_VLAN_ERROR;
          }
          update_flags |= PPA_UPDATE_WAN_MC_ENTRY_OUT_VLAN_INS_EN | PPA_UPDATE_WAN_MC_ENTRY_OUT_VLAN_RM_EN;     //PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_INS_EN | PPA_UPDATE_ROUTING_ENTRY_OUT_VLAN_RM_EN;
        }
      }

    PPA_HW_UPDATE_MC_GROUP_EXTRA_OUT_VLAN_GOON:
      update_flags |= PPA_UPDATE_WAN_MC_ENTRY_DEST_QID; //sgh chnage to update qid, since there is no such flag defined at present

      mc.p_entry = p_item->mc_entry;
      mc.f_vlan_ins_enable = p_item->flags & SESSION_VALID_VLAN_INS;
      mc.new_vci = p_item->new_vci;
      mc.f_vlan_rm_enable = p_item->flags & SESSION_VALID_VLAN_RM;
      mc.f_src_mac_enable = p_item->flags & SESSION_VALID_SRC_MAC;
      mc.pppoe_mode = p_item->flags & SESSION_VALID_PPPOE;
      mc.f_out_vlan_ins_enable =
          p_item->flags & SESSION_VALID_OUT_VLAN_INS;
      mc.out_vlan_info.vlan_entry = p_item->out_vlan_entry;
      mc.f_out_vlan_rm_enable = p_item->flags & SESSION_VALID_OUT_VLAN_RM;
      mc.f_new_dscp_enable = p_item->flags & SESSION_VALID_NEW_DSCP;
      mc.f_tunnel_rm_enable = p_item->flags & (SESSION_TUNNEL_6RD | SESSION_TUNNEL_DSLITE);     //for only downstream multicast acceleration
      mc.new_dscp = p_item->new_dscp;
      mc.dest_qid = p_item->dslwan_qid;
      mc.dest_list = 0;
      mc.update_flags = update_flags;

      if (p_item->caps_list[i].hal_id == PAE_HAL) {
        mc.dest_list = p_item->dest_ifid;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        mc.subif_id = p_item->dest_subifid;
        mc.group_id = p_item->group_id;
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
        mc.num_vap = p_item->num_vap;
#endif
#endif
      }
      // check whether more HALs needs to be configured to complete this session configuration
      if (i + j != p_item->num_caps) {  // more HALs are there in the caps_list
        f_more_hals = 1;
        // TBD: setup flow between the Acceleration blocks
      }

      if ((ret =
           ppa_hsel_update_wan_mc_entry(&mc, 0,
                                        p_item->caps_list[i].hal_id)) ==
          PPA_SUCCESS) {
        break;
      }

    OUT_VLAN_ERROR:
      ret = PPA_EAGAIN;

      break;
    case SESS_MC_DS_VAP:
      // TBD: complementary processing needs to be added
      break;
    default:
      break;
    }
    if (ret != PPA_SUCCESS) {
      // if the capability is SESS_MC_DS then we can check for any more HALs registred
      // TBD: if the capability needed is SESS_MC_DS_VAP we have to call a different API to forward the packet to CPU port / clear the MPE flag in the PAE configuration
      //find the next HAL registred for this capability
      if (ppa_select_hals_from_caplist(i, j, p_item->caps_list) !=
          PPA_SUCCESS) {
        // this session cannot be added to the HW
        return PPA_FAILURE;
      } else {
        continue;
      }
    }
// returned success in step 1 proceed
    i += j;
  }
  return PPA_SUCCESS;
}
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
int32_t ppa_add_class_rule(PPA_CLASS_RULE *rule)
{
	int32_t ret=PPA_SUCCESS;
	uint32_t flag=0;
	if ( ( ret=ppa_hsel_add_class_rule(rule, flag, PAE_HAL) ) == PPA_SUCCESS )
	{
		 ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n",__FILE__,__FUNCTION__,__LINE__);
	}
	return ret;
}
EXPORT_SYMBOL(ppa_add_class_rule);
		
int32_t ppa_mod_class_rule(PPA_CLASS_RULE *rule)
{
	int32_t ret=PPA_SUCCESS;
	uint32_t flag=0;
	if ( ( ret=ppa_hsel_mod_class_rule(rule, flag, PAE_HAL) ) == PPA_SUCCESS )
        {
                 ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n",__FILE__,__FUNCTION__,__LINE__);
        }
        return ret;
}
EXPORT_SYMBOL(ppa_mod_class_rule);

int32_t ppa_del_class_rule(PPA_CLASS_RULE *rule)                                 
{
	int32_t ret=PPA_SUCCESS;
	uint32_t flag=0;
	if ( ( ret=ppa_hsel_del_class_rule(rule, flag, PAE_HAL) ) == PPA_SUCCESS )
        {
                 ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n",__FILE__,__FUNCTION__,__LINE__);
        }
        return ret;
}
EXPORT_SYMBOL(ppa_del_class_rule);

int32_t ppa_get_class_rule(PPA_CLASS_RULE *rule)
{
	int32_t ret=PPA_SUCCESS;
	uint32_t flag=0;
	if ( ( ret=ppa_hsel_get_class_rule (rule, flag, PAE_HAL) ) == PPA_SUCCESS )
        {
                 ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n",__FILE__,__FUNCTION__,__LINE__);
        }
        return ret;
}
EXPORT_SYMBOL(ppa_get_class_rule);
#endif //defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500

#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO
int32_t ppa_add_lro_entry(struct session_list_item * p_item,
                                     uint32_t f_test)
{
    int32_t ret=PPA_SUCCESS;
    PPA_LRO_INFO lro_entry={0};

    /***********************************************/
    /*LRO not currently working with directpath interfaces 
      remove this section once this issue is fixed*/
    struct netif_info *rx_ifinfo;

    if(p_item->rx_if) {
        if( PPA_SUCCESS !=
            ppa_netif_lookup(ppa_get_netif_name(p_item->rx_if), &rx_ifinfo) ) {
   
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                        "failed in getting info structure of rx_if (%s)\n",
                                ppa_get_netif_name(p_item->rx_if));

            return PPA_FAILURE;
        } else {
            if(rx_ifinfo->flags & NETIF_BRIDGE) {
                if(p_item->br_rx_if) {
                    if( PPA_SUCCESS != ppa_netif_lookup(ppa_get_netif_name(p_item->br_rx_if), &rx_ifinfo) ) {
                        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                        "failed in getting info structure of rx_if (%s)\n",
                                ppa_get_netif_name(p_item->br_rx_if));
                    } else {
			if (( rx_ifinfo->flags & NETIF_DIRECTPATH) || ( rx_ifinfo->flags & NETIF_DIRECTCONNECT)) {
                            return PPA_FAILURE;
                        }
                    }
                } else {
                    return PPA_FAILURE;
                }
            }
        }
    } else {
        return PPA_FAILURE;
    }

    /*********************************************/


    if (p_item->flags & SESSION_IS_IPV6) {
	lro_entry.f_ipv6 = 1;
	ppa_memcpy(lro_entry.src_ip.ip6, p_item->src_ip.ip6,
                   sizeof(lro_entry.src_ip.ip6));
        ppa_memcpy(lro_entry.dst_ip.ip6, p_item->dst_ip.ip6,
                   sizeof(lro_entry.dst_ip.ip6));
    } else {
	lro_entry.src_ip.ip = p_item->src_ip.ip;
        lro_entry.dst_ip.ip = p_item->dst_ip.ip;
    }

    lro_entry.src_port = p_item->src_port;
    lro_entry.dst_port = p_item->dst_port;

    ret = ppa_hsel_add_lro_entry(&lro_entry, 0, PAE_HAL);
    if(ret == PPA_SUCCESS) {
	p_item->lro_sessid = lro_entry.session_id; 	
    }
    p_item->flags |= SESSION_NOT_ACCELABLE;
    return ret;
}
EXPORT_SYMBOL(ppa_add_lro_entry);

int32_t ppa_del_lro_entry(struct session_list_item * p_item)
{
    PPA_LRO_INFO lro_entry={0};
    lro_entry.session_id = p_item->lro_sessid;
    return ppa_hsel_del_lro_entry(&lro_entry, 0, PAE_HAL);
}
EXPORT_SYMBOL(ppa_del_lro_entry);

int32_t ppa_lro_entry_criteria(struct session_list_item * p_item, PPA_BUF *ppa_buf, uint32_t flags)
{
    // 1. Session gets added to lro only after 64k bytes are transmitted (TCP segment size) 
    // 2. Session gets added to lro only if packet size is large (to avoid tcp ack)
    // have to add proper session entry criteria based on data rate here. 
//    ppa_get_pkt_rx_src_mac_addr(ppa_buf, p_item->src_mac);
//    if(ppa_check_is_lro_enabled_netif(p_item->rx_if, p_item->src_mac)!=PPA_SUCCESS)
//	return PPA_FAILURE;    

    if ( p_item->mips_bytes > 64000 && ppa_buf->len > (p_item->mtu - 128) ) {
	return PPA_SUCCESS; 
    }
    return PPA_FAILURE;
}
EXPORT_SYMBOL(ppa_lro_entry_criteria);

#endif // defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO

EXPORT_SYMBOL(ppa_hsel_add_routing_session);
EXPORT_SYMBOL(ppa_hsel_del_routing_session);
EXPORT_SYMBOL(ppa_hsel_update_routing_session);
EXPORT_SYMBOL(ppa_hsel_add_wan_mc_group);
EXPORT_SYMBOL(ppa_hsel_del_wan_mc_group);
EXPORT_SYMBOL(ppa_hsel_update_wan_mc_group);

#endif                          //defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
