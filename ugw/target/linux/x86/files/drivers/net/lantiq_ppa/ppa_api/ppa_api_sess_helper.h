/*******************************************************************************
**
** FILE NAME    : ppa_api_sess_helper.h
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
#ifndef __PPA_API_SESS_HELPER__

#define __PPA_API_SESS_HELPER__

/* Session list lock init and destroy */
int ppa_session_list_lock_init(void);
int ppa_session_list_lock_destroy(void);

/* Session list init and destroy routines */
int ppa_session_list_init(void);
void ppa_session_list_free(void);

/* Session list lock and unlock routine */
void ppa_session_list_lock(void);
void ppa_session_list_unlock(void);
void ppa_session_list_read_lock(void);
void ppa_session_list_read_lock(void);

/* Session allocate, insert and delete routines */
struct session_list_item *ppa_session_alloc_item(void);
void __ppa_session_insert_item(struct session_list_item *);
void __ppa_session_delete_item(struct session_list_item *p_item);
void ppa_session_list_free_item(struct session_list_item *p_item);

void ppa_session_delete_by_netif(PPA_IFNAME *);

/* TODO: Can these functions be inline */
/* Bridged session flows timer handling routines */
int ppa_session_br_timer_init(struct session_list_item *p_item);
void  ppa_session_br_timer_add(struct session_list_item *p_item);
void  ppa_session_br_timer_del(struct session_list_item *p_item);
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
void ppa_session_delete_by_subif(PPA_DP_SUBIF *subif);
void ppa_session_delete_by_macaddr(uint8_t *mac);
#endif

/* Session cache related functions */
int32_t ppa_session_cache_create(void);
int32_t ppa_session_cache_shrink(void);
void ppa_session_cache_destroy(void);

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
extern int32_t __ppa_search_ipsec_rtindex(struct session_list_item *pp_item);
#endif

#if defined CONFIG_LTQ_PPA_MPE_HAL || defined CONFIG_LTQ_PPA_MPE_HAL_MODULE
/* Template buffer helper functions */
int32_t ppa_session_construct_tmplbuf(struct sk_buff *skb, 
                              struct session_list_item *p_item, 
                              struct netif_info *tx_ifinfo);

void ppa_session_destroy_tmplbuf(struct session_list_item *p_item);
#endif

/* Debug macro/routines */
#if 0
#define  ppa_session_print(p_item) printk("<%s> flags=0x%X flag2=0x%X proto:%d %pI4:%d <-> %pI4:%d ref=%u\n", \
                                     __FUNCTION__,  \
                                     p_item->flags,p_item->flag2, \
                                     p_item->ip_proto, \
                                     &p_item->src_ip,p_item->src_port, \
                                     &p_item->dst_ip,p_item->dst_port, \
                                     ppa_atomic_read(&p_item->used));
#else
#define  ppa_session_print  
#endif

#endif
