/******************************************************************************
**
** FILE NAME    : ppe_datapath_wrapper.c
** PROJECT      : PPA
** MODULES      : PPA Wrapper for Datapath driver APIs
**
** DATE         : 27 Feb 2014
** AUTHOR       : Kamal Eradath
** DESCRIPTION  : PPA Wrapper for Datapath Driver API
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 27 FEB 2014  Kamal Eradath          Initiate Version
*******************************************************************************/

/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/netdevice.h>
#include <linux/atmdev.h>
#include <net/sock.h>

/*
 *  Chip Specific Head File
 */
#include <net/ppa_api.h>
#include "ppa_api_session.h"
#include "ppa_datapath_wrapper.h"
#include "../platform/ppa_datapath.h"

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#include <net/datapath_api.h>
#include <net/ltq_mpe_hal.h>
#endif

/*Hook API for PPE Driver's Datapath layer: these hook will be set in PPE datapath driver*/
/* First part is for direct path */
struct ppe_directpath_data *ppa_drv_g_ppe_directpath_data = NULL;
int32_t (*ppa_drv_directpath_send_hook)(uint32_t, PPA_BUF *, int32_t, uint32_t) = NULL;
int32_t (*ppa_drv_directpath_rx_stop_hook)(uint32_t, uint32_t) = NULL;
int32_t (*ppa_drv_directpath_rx_start_hook)(uint32_t, uint32_t) = NULL;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
int32_t (*ppa_drv_directpath_register_hook)(PPA_SUBIF *, PPA_NETIF *, PPA_DIRECTPATH_CB*, int32_t*, uint32_t) = NULL;
int32_t (*mpe_hal_feature_start_fn)(
                enum MPE_Feature_Type mpeFeature,
                uint32_t port_id,
                uint32_t * featureCfgBase,
                uint32_t flags) = NULL;
EXPORT_SYMBOL(mpe_hal_feature_start_fn);
int32_t (* mpe_hal_get_netif_mib_hook_fn) (struct net_device *dev,
	dp_subif_t *subif_id, struct mpe_hal_if_stats *mpe_mib,
	uint32_t flag) = NULL;
EXPORT_SYMBOL(mpe_hal_get_netif_mib_hook_fn);

int32_t (* mpe_hal_clear_if_mib_hook_fn) (struct net_device *dev,
	dp_subif_t *subif_id, uint32_t flag) = NULL;
EXPORT_SYMBOL(mpe_hal_clear_if_mib_hook_fn);

#if defined(CONFIG_LTQ_PPA_TMU_MIB_SUPPORT)
int32_t (*tmu_hal_get_csum_ol_mib_hook_fn)(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag) = NULL;
EXPORT_SYMBOL(tmu_hal_get_csum_ol_mib_hook_fn);
int32_t (*tmu_hal_clear_csum_ol_mib_hook_fn)(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag) = NULL;
EXPORT_SYMBOL(tmu_hal_clear_csum_ol_mib_hook_fn);
int32_t(*tmu_hal_get_qos_mib_hook_fn)(
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                struct tmu_hal_qos_stats *qos_mib,
                uint32_t flag) = NULL;
EXPORT_SYMBOL(tmu_hal_get_qos_mib_hook_fn);
int32_t (*tmu_hal_clear_qos_mib_hook_fn)(
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                uint32_t flag) = NULL;
EXPORT_SYMBOL(tmu_hal_clear_qos_mib_hook_fn);
#endif
#endif
/*    others:: these hook will be set in PPE datapath driver  */
int (*ppa_drv_get_dslwan_qid_with_vcc_hook)(struct atm_vcc *vcc)= NULL;
int (*ppa_drv_get_netif_qid_with_pkt_hook)(struct sk_buff *skb, void *arg, int is_atm_vcc)= NULL;
int (*ppa_drv_get_atm_qid_with_pkt_hook)(struct sk_buff *skb, void *arg, int is_atm_vcc)= NULL;
int (*ppa_drv_ppe_clk_change_hook)(unsigned int arg, unsigned int flags)= NULL;
int (*ppa_drv_ppe_pwm_change_hook)(unsigned int arg, unsigned int flags)= NULL;
int32_t (*ppa_drv_datapath_generic_hook)(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)=NULL;
//below hook will be exposed from datapath driver and called by its hal driver.
int32_t (*ppa_drv_datapath_mac_entry_setting)(uint8_t  *mac, uint32_t fid, uint32_t portid, uint32_t agetime, uint32_t st_entry , uint32_t action) = NULL;

/* Hook API for datapath A1 to get MPoA type */
int32_t (*ppa_drv_hal_get_mpoa_type_hook)(uint32_t dslwan_qid, uint32_t *mpoa_type) = NULL;

int ppa_drv_directpath_send(uint32_t if_id, struct sk_buff *skb, int32_t len, uint32_t flags)
{
    if( !ppa_drv_directpath_send_hook ) return PPA_EINVAL;
    return ppa_drv_directpath_send_hook(if_id, skb, len, flags);
}

int ppa_drv_directpath_rx_stop(uint32_t if_id, uint32_t flags)
{
     if( !ppa_drv_directpath_rx_stop_hook ) return PPA_EINVAL;
     return ppa_drv_directpath_rx_stop_hook(if_id, flags);
}

int ppa_drv_directpath_rx_start(uint32_t if_id, uint32_t flags)
{
     if( !ppa_drv_directpath_rx_start_hook ) return PPA_EINVAL;
     return ppa_drv_directpath_rx_start_hook(if_id, flags);
}

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
int32_t ppa_drv_directpath_register(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, int32_t *index, uint32_t flags)
{
    if( !ppa_drv_directpath_register_hook ) return PPA_EINVAL;
    return ppa_drv_directpath_register_hook(subif, netif, pDirectpathCb, index, flags);
}

#if defined(CONFIG_LTQ_PPA_TMU_MIB_SUPPORT)
int32_t ppa_drv_get_csum_ol_mib(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag)
{
    if( !tmu_hal_get_csum_ol_mib_hook_fn ) return PPA_EINVAL;
    return tmu_hal_get_csum_ol_mib_hook_fn (csum_mib, flag);

}
EXPORT_SYMBOL(ppa_drv_get_csum_ol_mib);

int32_t ppa_drv_clear_csum_ol_mib(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag)
{
    if( !tmu_hal_clear_csum_ol_mib_hook_fn ) return PPA_EINVAL;
    return tmu_hal_clear_csum_ol_mib_hook_fn (csum_mib, flag);

}
EXPORT_SYMBOL(ppa_drv_clear_csum_ol_mib);

int32_t ppa_drv_get_tmu_qos_mib (
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                struct tmu_hal_qos_stats *qos_mib,
                uint32_t flag)
{
    if( !tmu_hal_get_qos_mib_hook_fn ) return PPA_EINVAL;
    return tmu_hal_get_qos_mib_hook_fn (netdev, subif_id, queueid, qos_mib, flag);

}
EXPORT_SYMBOL(ppa_drv_get_tmu_qos_mib);
int32_t ppa_drv_reset_tmu_qos_mib (
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                uint32_t flag)
{
    if( !tmu_hal_clear_qos_mib_hook_fn) return PPA_EINVAL;
    return tmu_hal_clear_qos_mib_hook_fn (netdev, subif_id, queueid, flag);
}
EXPORT_SYMBOL(ppa_drv_reset_tmu_qos_mib);
#endif
#endif

int ppa_drv_get_dslwan_qid_with_vcc(struct atm_vcc *vcc)
{
    if( !ppa_drv_get_dslwan_qid_with_vcc_hook ) return 0;
    else return ppa_drv_get_dslwan_qid_with_vcc_hook(vcc);
}

int ppa_drv_get_netif_qid_with_pkt(struct sk_buff *skb, void *arg, int is_atm_vcc)
{
    if( !ppa_drv_get_netif_qid_with_pkt_hook ) return 0;
    else return ppa_drv_get_netif_qid_with_pkt_hook(skb, arg, is_atm_vcc);

}

int ppa_drv_ppe_clk_change(unsigned int arg, unsigned int flags)
{
    if( !ppa_drv_ppe_clk_change_hook ) return PPA_FAILURE;
    else return ppa_drv_ppe_clk_change_hook(arg, flags);
}

int ppa_drv_ppe_pwm_change(unsigned int arg, unsigned int flags)
{
    if( !ppa_drv_ppe_pwm_change_hook ) return PPA_FAILURE;
    else return ppa_drv_ppe_pwm_change_hook(arg, flags);
}

uint32_t ppa_drv_dp_sb_addr_to_fpi_addr_convert(PPA_FPI_ADDR*a, uint32_t flag)
{
    if( !ppa_drv_datapath_generic_hook ) return PPA_FAILURE;
    return ppa_drv_datapath_generic_hook(PPA_GENERIC_DATAPATH_ADDR_TO_FPI_ADDR, (void *)a, flag );

}

int32_t ppa_hook_set_lan_seperate_flag( uint32_t flag) 
{
    if( !ppa_drv_datapath_generic_hook ) return PPA_FAILURE;
    return ppa_drv_datapath_generic_hook(PPA_GENERIC_DATAPATH_SET_LAN_SEPARATE_FLAG, NULL, flag );
}
int32_t ppa_hook_get_lan_seperate_flag( uint32_t flag) 
{
    if( !ppa_drv_datapath_generic_hook ) return PPA_FAILURE;
    return ppa_drv_datapath_generic_hook(PPA_GENERIC_DATAPATH_GET_LAN_SEPARATE_FLAG, NULL, flag );
}
uint32_t ppa_hook_set_wan_seperate_flag( uint32_t flag) 
{
    if( !ppa_drv_datapath_generic_hook ) return PPA_FAILURE;
    return ppa_drv_datapath_generic_hook(PPA_GENERIC_DATAPATH_SET_WAN_SEPARATE_FLAG, NULL, flag );
}
uint32_t ppa_hook_get_wan_seperate_flag( uint32_t flag) 
{
    if( !ppa_drv_datapath_generic_hook ) return PPA_FAILURE;
    return ppa_drv_datapath_generic_hook(PPA_GENERIC_DATAPATH_GET_WAN_SEPARATE_FLAG, NULL, flag );
}

//for PPE driver's datapath APIs
EXPORT_SYMBOL(ppa_drv_g_ppe_directpath_data);
EXPORT_SYMBOL(ppa_drv_directpath_send_hook);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
EXPORT_SYMBOL(ppa_drv_directpath_register);
EXPORT_SYMBOL(ppa_drv_directpath_register_hook);
#endif
EXPORT_SYMBOL(ppa_drv_directpath_rx_stop_hook);
EXPORT_SYMBOL(ppa_drv_directpath_rx_start_hook);
EXPORT_SYMBOL(ppa_drv_directpath_send);
EXPORT_SYMBOL(ppa_drv_directpath_rx_stop);
EXPORT_SYMBOL(ppa_drv_directpath_rx_start);

EXPORT_SYMBOL(ppa_drv_get_dslwan_qid_with_vcc_hook);
EXPORT_SYMBOL(ppa_drv_get_netif_qid_with_pkt_hook);
EXPORT_SYMBOL(ppa_drv_get_atm_qid_with_pkt_hook);
EXPORT_SYMBOL(ppa_drv_hal_get_mpoa_type_hook);
EXPORT_SYMBOL(ppa_drv_ppe_clk_change_hook);
EXPORT_SYMBOL(ppa_drv_ppe_pwm_change_hook);
EXPORT_SYMBOL(ppa_drv_get_dslwan_qid_with_vcc);
EXPORT_SYMBOL(ppa_drv_get_netif_qid_with_pkt);
EXPORT_SYMBOL(ppa_drv_ppe_clk_change);
EXPORT_SYMBOL(ppa_drv_ppe_pwm_change);
EXPORT_SYMBOL(ppa_drv_datapath_generic_hook);
EXPORT_SYMBOL(ppa_drv_datapath_mac_entry_setting);
EXPORT_SYMBOL(ppa_drv_dp_sb_addr_to_fpi_addr_convert);
EXPORT_SYMBOL(ppa_hook_get_lan_seperate_flag);
EXPORT_SYMBOL(ppa_hook_set_lan_seperate_flag);
EXPORT_SYMBOL(ppa_hook_get_wan_seperate_flag);
EXPORT_SYMBOL(ppa_hook_set_wan_seperate_flag);




