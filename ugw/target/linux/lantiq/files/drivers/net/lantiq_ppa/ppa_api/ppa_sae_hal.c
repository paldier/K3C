/*******************************************************************************
**
** FILE NAME    : ppa_sae_hal.c
** PROJECT      : PPA
** MODULES      : PPA API - Software Acceleration Engine HAL 
**
** DATE         : 10 Aug 2015
** AUTHOR       : Mahipati Deshpande
** DESCRIPTION  : PPA Protocol Stack Hook API Session Operation Functions
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY      :
** $Date        $Author                $Comment
** 24 Feb 2015  Mahipati     This file defines the APIs for software 
**                           acceleration. 
**                           Note: This file is named as SAE HAL - In fufure 
**                           Software Acceleration Engine functionality shall
**                           be defined in this file. Now contains the wrapper 
**                           for hooks defined by Software acceleration.
*******************************************************************************/

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif
#include <linux/swap.h>
#include <net/ppa_api.h>

#include "ppa_api_session.h"
#include "ppa_api_netif.h"
#include "ppa_sae_hal.h"


int32_t ppa_sw_add_session(struct sk_buff *skb, 
                           struct session_list_item *p_item)
{
    if( !ppa_sw_add_session_hook ) return PPA_FAILURE;
    return ppa_sw_add_session_hook(skb, p_item); 
}

int32_t ppa_sw_update_session(struct sk_buff *skb, 
                              struct session_list_item *p_item, 
                              struct netif_info *tx_ifinfo)
{
  if( !ppa_sw_update_session_hook ) 
    return PPA_EINVAL;

  return ppa_sw_update_session_hook(skb, p_item, tx_ifinfo); 
}

int ppa_sw_del_session(struct session_list_item *p_item)
{
  if( !ppa_sw_del_session_hook ) 
    return PPA_FAILURE;

  ppa_sw_del_session_hook(p_item); 

  return PPA_SUCCESS;
}

int32_t ppa_sw_fastpath_enable(uint32_t f_enable, 
                               uint32_t flags)
{
	if( !ppa_sw_fastpath_enable_hook ) {
		return PPA_FAILURE;
	}
	return ppa_sw_fastpath_enable_hook(f_enable, flags);
}

int32_t ppa_get_sw_fastpath_status( uint32_t *f_enable, 
                                    uint32_t flags)
{
	if ( !ppa_get_sw_fastpath_status_hook ) 
    return PPA_FAILURE;
	
  return ppa_get_sw_fastpath_status_hook(f_enable, flags);
}

int32_t ppa_sw_session_enable(struct session_list_item *p_item, 
                              uint32_t f_enable, 
                              uint32_t flags)
{
	if( !ppa_sw_session_enable_hook ) {
		return PPA_FAILURE;
	}

	return ppa_sw_session_enable_hook(p_item, f_enable, flags);
}

int32_t ppa_get_sw_session_status(struct session_list_item *p_item, 
                                  uint32_t *f_enable, 
                                  uint32_t flags)
{
	if ( !ppa_get_sw_session_status_hook ) 
    return PPA_FAILURE;
	
  return ppa_get_sw_session_status_hook(p_item, f_enable, flags);
}

int32_t ppa_sw_fastpath_send(void *skb)
{
	if ( !ppa_sw_fastpath_send_hook ) 
    return PPA_FAILURE;
	
  return ppa_sw_fastpath_send_hook(skb);
}
