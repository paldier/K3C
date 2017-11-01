#ifndef __TMU_HAL_API_H__20081119_1007__
#define __TMU_HAL_API_H__20160621_1007__



/*******************************************************************************
**
** FILE NAME    : tmu_hal_api.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 21 JUN 2016
** AUTHOR       : Purnendu Ghosh
** DESCRIPTION  : PPA Protocol Stack Hook for TMU HAL API Functions Header
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

#if defined(CONFIG_LTQ_PPA_TMU_MIB_SUPPORT)
struct tmu_hal_qos_stats {
	uint32_t enqPkts; /* Enqueued packets Count */
	uint32_t enqBytes; /* Enqueued Bytes Count */
	uint32_t deqPkts; /* Dequeued packets Count */
	uint32_t deqBytes; /* Dequeued Bytes Count */
	uint32_t dropPkts; /* Dropped Packets Count */
	uint32_t dropBytes; /* Dropped Bytes Count - UNUSED for now */
	uint32_t qOccPkts; /* Queue Occupancy Packets Count - Only at Queue level */
};

extern int32_t (*tmu_hal_get_csum_ol_mib_hook_fn)(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag);
extern int32_t (*tmu_hal_clear_csum_ol_mib_hook_fn)(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag);

extern int32_t(*tmu_hal_get_qos_mib_hook_fn)(
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                struct tmu_hal_qos_stats *qos_mib,
                uint32_t flag);
extern int32_t (*tmu_hal_clear_qos_mib_hook_fn)(
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                uint32_t flag);

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



#endif  //  __TMU_HAL_API_H__20160621_1007__
