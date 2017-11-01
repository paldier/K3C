#ifndef __PPA_API_HAL_SELECTOR_H_2014_02_28
#define __PPA_API_HAL_SELECTOR_H_2014_02_28
/*******************************************************************************
**
** FILE NAME    : ppa_api_hal_selector.h
** PROJECT      : PPA
** MODULES      : PPA API ( PPA HAL SELECTOR  APIs)
**
** DATE         : 28 Feb 2014
** AUTHOR       : Kamal Eradath
** DESCRIPTION  : PPA HAL SELECTOR APIs
**                File
** COPYRIGHT    :              Copyright (c) 2014
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date                $Author         $Comment
** 28-FEB-2014  	Kamal Eradath   Initiate Version
*******************************************************************************/

/*! \file ppa_api_hal_selector.h
    \brief This file contains es.
           provide PPA APIs for HAL selector.
*/

/** \addtogroup  PPA_API_HAL_SELECTOR */
/*@{*/


#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR

/*
 * ####################################
 *              Definition
 * ####################################
 */

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


// unicast routing entries
extern int32_t ppa_hsel_add_routing_session(struct session_list_item *p_item, uint32_t f_test);
extern void ppa_hsel_del_routing_session(struct session_list_item *p_item);
extern int32_t ppa_hsel_update_routing_session(struct session_list_item *p_item, uint32_t flags);
extern void ppa_hsel_check_hit_stat_clear_mib(int32_t flag);

// multicast routing entries
extern int32_t ppa_hsel_add_wan_mc_group(struct mc_group_list_item *p_item);
extern void ppa_hsel_del_wan_mc_group(struct mc_group_list_item *p_item);
extern int32_t ppa_hsel_update_wan_mc_group(struct mc_group_list_item *p_item, uint32_t flags);

#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO
extern int32_t ppa_add_lro_entry(struct session_list_item *p_item, uint32_t f_test);
extern int32_t ppa_del_lro_entry(struct session_list_item *p_item);
extern int32_t ppa_lro_entry_criteria(struct session_list_item *p_item, PPA_BUF *ppa_buf, uint32_t flags);
#endif

#endif //CONFIG_LTQ_PPA_HAL_SELECTOR

#endif //__PPA_API_HAL_SELECTOR_H_2014_02_28

