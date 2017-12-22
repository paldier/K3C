#ifndef __TMU_HAL_CONNECTIVITY_H__20081119_1144__
#define __TMU_HAL_CONNECTIVITY_H__20081119_1144__



/*******************************************************************************
**
** FILE NAME    : tmu_hal_connectivity.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 19 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Directpath Functions Header
**                File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 19 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/
/*! \file ppa_api_directpath.h
    \brief This file contains: PPA direct path api.
*/



#include <net/ppa_api_common.h>
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#include <net/datapath_api.h>
#endif


int tmu_hal_setup_dp_ingress_connectivity(
                        struct net_device *netdev,
                        uint32_t pmac_port);

int tmu_hal_setup_dp_egress_connectivity(
                        struct net_device *netdev,
                        uint32_t pmac_port);
int tmu_hal_remove_dp_egress_connectivity(
			struct net_device *netdev, 
			uint32_t pmac_port);
int tmu_hal_remove_dp_ingress_connectivity(
			struct net_device *netdev, 
			uint32_t pmac_port);
#endif  //  __TMU_HAL_CONNECTIVITY_H__20081119_1144__
