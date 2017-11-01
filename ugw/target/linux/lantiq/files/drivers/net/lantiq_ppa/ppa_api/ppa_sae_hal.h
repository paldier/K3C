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
#ifndef __PPA_SAE_HAL_H__

#define __PPA_SAE_HAL_H__ 

extern int32_t (*ppa_sw_add_session_hook)(PPA_BUF *skb, 
                                          struct session_list_item *p_item);

extern int32_t (*ppa_sw_update_session_hook)(PPA_BUF *skb, 
                                             struct session_list_item *p_item,
                                             struct netif_info *tx_ifinfo);

extern void (*ppa_sw_del_session_hook)(struct session_list_item *p_item);

extern int32_t (*ppa_sw_fastpath_enable_hook)(uint32_t f_enable, 
                                              uint32_t flags);

extern int32_t (*ppa_get_sw_fastpath_status_hook)(uint32_t *f_enable, 
                                                  uint32_t flags);

extern int32_t (*ppa_sw_session_enable_hook)(struct session_list_item *p_item, 
                                             uint32_t f_enable, 
                                             uint32_t flags);

extern int32_t (*ppa_get_sw_session_status_hook)(struct session_list_item *p_item, 
                                                 uint32_t *f_enable, 
                                                 uint32_t flags);

extern int32_t (*ppa_sw_fastpath_send_hook)(struct sk_buff *skb); 

int32_t ppa_sw_add_session(struct sk_buff *skb, 
                           struct session_list_item *p_item); 

int32_t ppa_sw_update_session(struct sk_buff *skb, 
                              struct session_list_item *p_item,
                              struct netif_info *tx_ifinfo);

int32_t ppa_sw_del_session(struct session_list_item *p_item);
int32_t ppa_sw_fastpath_send(void *skb); 

int32_t ppa_sw_fastpath_enable(uint32_t f_enable, 
                               uint32_t flags);

int32_t ppa_get_sw_fastpath_status(uint32_t *f_enable, 
                                   uint32_t flags);

int32_t ppa_sw_session_enable(struct session_list_item *p_item, 
                              uint32_t f_enable, 
                              uint32_t flags);

int32_t ppa_get_sw_session_status(struct session_list_item *p_item, 
                                  uint32_t *f_enable, 
                                  uint32_t flags);

#endif
