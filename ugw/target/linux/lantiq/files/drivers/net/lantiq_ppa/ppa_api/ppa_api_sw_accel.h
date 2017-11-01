#ifndef __PPA_API_SW_ACCEL_H__20130913
#define __PPA_API_SW_ACCEL_H__20130913
/*******************************************************************************
**
** FILE NAME    : ppa_api_sw_accel.h
** PROJECT      : PPA
** MODULES      : PPA API (Software Fastpath Implementation)
**
** DATE         : 12 Sep 2013
** AUTHOR       : Lantiq
** DESCRIPTION  : Function to bypass the linux stack for packets belonging to the PPA sessions which are not in PPE firmware.
** COPYRIGHT    :              Copyright (c) 2013
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 12 Sep 2013  Kamal Eradath          Initiate Version
*******************************************************************************/

/*! \file ppa_api_sw_accel.h
    \brief This file contains es.
           software fastpath function declarations
*/

/*@{*/

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
extern volatile u_int8_t g_sw_fastpath_enabled;
/*
 * ####################################
 *             Declaration
 * ####################################
 */
 /* @} */
/*
int32_t ppa_sw_fastpath_enable(uint32_t f_enable, uint32_t flags);
int32_t ppa_get_sw_fastpath_status(uint32_t *f_enable, uint32_t flags);
int32_t ppa_sw_fastpath_send(struct sk_buff *skb);
*/
extern int32_t (*ppa_sw_fastpath_enable_hook)(uint32_t, uint32_t);
extern int32_t (*ppa_get_sw_fastpath_status_hook)(uint32_t *, uint32_t);
extern int32_t (*ppa_sw_fastpath_send_hook)(PPA_BUF *);

extern int32_t (*ppa_sw_add_session_hook)(PPA_BUF *skb, struct session_list_item *p_item);
extern int32_t (*ppa_sw_update_session_hook)(PPA_BUF *skb, struct session_list_item *p_item,struct netif_info *tx_ifinfo);
extern void (*ppa_sw_del_session_hook)(struct session_list_item *p_item);

signed long sw_update_session(void *skb, void *p_item, void *tx_ifinfo);
signed long sw_add_session(void *skb, void *p_item);
void sw_del_session(void *p_item);

signed long sw_fastpath_send(void *skb);
signed long get_sw_fastpath_status(unsigned long *f_enable, unsigned long flags);
signed long sw_fastpath_enable(unsigned long f_enable, unsigned long flags);
#endif  // __PPA_API_SW_ACCEL_H__20130913
