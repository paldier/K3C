#ifndef __LTQ_MPE_HAL_H__20081119_1144__
#define __LTQ_MPE_HAL_H__20081119_1144__



/*******************************************************************************
**
** FILE NAME    : ltq_mpe_hal.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 1 APRIL 2015
** AUTHOR       : Purnendu Ghosh
** DESCRIPTION  : MPE HAL API's to reserve memory and start feature
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
** 1 APRIL 2015  Purnendu Ghosh     Initiate Version
*******************************************************************************/
/*! \file ltq_mpe_hal.h
    \brief This file contains: MPE HAL API's to reserve memory and start feature.
*/



#include <net/ppa_api_common.h>
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#include <net/datapath_api.h>
#endif

#define F_FEATURE_START 	1
#define F_FEATURE_STOP   	0

//#define F_FEATURE_START 	(1 << 25)
//#define F_FEATURE_STOP   	(1 << 26)
enum MPE_Feature_Type {
	DL_TX_1 =0,
	DL_TX_2
};

struct device * mpe_hal_dl_get_dev(void);

void mpe_hal_dl_enable_gic(int irq_no);

int mpe_hal_dl_alloc_resource(
	uint32_t memSize,
	uint32_t *memAddr,
	uint32_t flags);

int mpe_hal_feature_start(
	enum MPE_Feature_Type mpeFeature,
	uint32_t port_id,
	uint32_t * featureCfgBase,
	uint32_t flags);
#if 0
int (*mpe_hal_feature_start_fn)(
                enum MPE_Feature_Type mpeFeature,
                uint32_t port_id,
                uint32_t * featureCfgBase,
                uint32_t flags);
#endif

#if defined(CONFIG_LTQ_PPA_TMU_MIB_SUPPORT)
struct mpe_hal_if_stats {
        uint32_t txPkts; /* Transmitted packets Count */
        uint32_t txBytes; /* Transmitted Bytes Count */
        uint32_t dropPkts; /* Dropped packets count */
        uint32_t dropBytes; /* Dropped packets bytes size count */
        uint32_t rxPkts; /* Received packets Count - UNUSED FOR NOW */
        uint32_t rxBytes; /* Received Bytes Count - UNUSED FOR NOW */
};

extern int32_t (* mpe_hal_get_netif_mib_hook_fn) (struct net_device *dev,
	dp_subif_t *subif_id, struct mpe_hal_if_stats *mpe_mib,
	uint32_t flag);

extern int32_t (* mpe_hal_clear_if_mib_hook_fn) (struct net_device *dev,
	dp_subif_t *subif_id, uint32_t flag);
#endif
#endif  //  __LTQ_MPE_HAL_H__20081119_1144__
