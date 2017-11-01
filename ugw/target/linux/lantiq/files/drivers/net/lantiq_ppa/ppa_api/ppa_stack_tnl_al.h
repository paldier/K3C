
/*******************************************************************************
 **
 ** FILE NAME    : ppa_stack_tnl_al.h
 ** PROJECT      : PPA 
 ** MODULES      : PPA Stack Adaptation Layer
 **
 ** DATE         : 12 May 2014
 ** AUTHOR       : Mahipati Deshpande
 ** DESCRIPTION  : Stack Adaptation for tunneled interfaces and sessions.
 ** NOTE         : This file is internal to PPA Stack Adaptation layer, so
 **                file should be included(used) only in PPA stack adaptation
 **                layer
 ** COPYRIGHT    :              Copyright (c) 2009
 **                          Lantiq Deutschland GmbH
 **                   Am Campeon 3; 85579 Neubiberg, Germany
 **   For licensing information, see the file 'LICENSE' in the root folder of
 **   this software module.
 **
 ** HISTORY
 ** $Date        $Author                $Comment
 ** 12 May 2014   Mahipati Deshpande    Moved tunneled routines to this file
 **                                     Added support for GRE
 *******************************************************************************/
#ifndef __PPA_STACK_TNL_AL__
#define __PPA_STACK_TNL_AL__

/* Returns base interface of GRE tunnel */
struct net_device* ppa_get_gre_phyif(struct net_device* dev);

/* Returns destination MAC address of the GRE tunnel */
int32_t ppa_get_gre_dmac(uint8_t *mac,
                     struct net_device* dev,
                     struct sk_buff *skb);
/* Returns destination MAX for the 6RD tunnel */
int32_t ppa_get_6rd_dst_mac(struct net_device *dev, 
                            PPA_BUF *ppa_buf,
                            uint8_t *mac,
                            uint32_t daddr);

/* Returns destination MAC of DS-Lite tunnel */
int32_t ppa_get_dslite_dst_mac(struct net_device *dev,PPA_BUF* buf, uint8_t *mac);

/*
 * Supporting routines that are used by other modules or inside PPA
 * Stack Adaptation Layer
 */
int32_t ppa_get_dmac_from_dst_entry( uint8_t* mac, 
                                 PPA_BUF* skb, 
                                 struct dst_entry *dst);

void ppa_neigh_update_hhs(struct neighbour *neigh);

void ppa_neigh_hh_init(struct neighbour *n, struct dst_entry *dst);

#endif

