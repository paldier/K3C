#ifndef __PPA_API_CORE_H__20081103_1920__
#define __PPA_API_CORE_H__20081103_1920__



/*******************************************************************************
**
** FILE NAME    : ppa_api_core.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Implementation Header File
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
/*! \file ppa_api_core.h
    \brief This file contains es.
           provide PPA API.
*/

/** \addtogroup PPA_CORE_API PPA Core API
    \brief  PPA Core API provide PPA core accleration logic and API
            The API is defined in the following two source files
            - ppa_api_core.h: Header file for PPA API
            - ppa_api_core.c: C Implementation file for PPA API
*/
/* @{ */


/*
 * ####################################
 *              Definition
 * ####################################
 */

#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
#define MC_F_ADD 0x01
#define MC_F_DEL 0x02

#define MC_F_REGISTER   0x01
#define MC_F_DEREGISTER 0x02
#define MC_F_DIRECTPATH 0x04
#define PPA_MCAST_DEBUG 1
/* mcast stream data structure */

typedef enum {
       IPV4 = 0,
       IPV6 = 1,
       INVALID,
} ptype_t;

typedef struct _ip_addr_t{
       ptype_t  ipType ;/* IPv4 or IPv6 */
       union {
               struct in_addr ipAddr;
               struct in6_addr ip6Addr;
       } ipA;
} ip_addr_t;

typedef struct _mcast_stream_t{

struct net_device *rx_dev;/* Rx Netdevice */
ip_addr_t src_ip;/* Source ip : can be ipv4 or ipv6 */
ip_addr_t dst_ip;/* Destination ip - GA : can be ipv4 of ipv6 */
uint32_t proto;/* Protocol type : Mostly UDP for Multicast */
uint32_t src_port;/* Source port */
uint32_t dst_port;/* Destination port */
unsigned char macaddr[ETH_ALEN];/* Mac address */
unsigned char src_mac[ETH_ALEN];/* source Mac address for grx5xx */
}mcast_stream_t;


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
extern int mcast_helper_register_module(PPA_NETIF *mem_dev,struct module *mod_name,char *addl_name,int32_t *fn_cb, uint32_t flags);

#endif

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
extern uint32_t ppa_add_ipsec_tunnel_tbl_entry(PPA_XFRM_STATE * entry,sa_direction dir,uint32_t *tunnel_index );
extern uint32_t ppa_get_ipsec_tunnel_tbl_entry(PPA_XFRM_STATE * entry,sa_direction *dir, uint32_t *tunnel_index );
extern uint32_t ppa_add_ipsec_tunnel_tbl_update(sa_direction dir, uint32_t tunnel_index );
#endif

#endif  //  __PPA_API_CORE_H__20081103_1920__
/* @} */
