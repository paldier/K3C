/*******************************************************************************
**
** FILE NAME    : ppa_api_core.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Implementation
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**data
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 03 NOV 2008  Xu Liang               Initiate Version
** 10 DEC 2012  Manamohan Shetty       Added the support for RTP,MIB mode and CAPWAP 
**                                     Features 
*******************************************************************************/



/*
 * ####################################
 *              Version No.
 * ####################################
 */

#define VER_FAMILY      0x60        //  bit 0: res
                                    //      1: Danube
                                    //      2: Twinpass
                                    //      3: Amazon-SE
                                    //      4: res
                                    //      5: AR9
                                    //      6: GR9
#define VER_DRTYPE      0x20        //  bit 0: Normal Data Path driver
                                    //      1: Indirect-Fast Path driver
                                    //      2: HAL driver
                                    //      3: Hook driver
                                    //      4: Stack/System Adaption Layer driver
                                    //      5: PPA API driver
#define VER_INTERFACE   0x07        //  bit 0: MII 0
                                    //      1: MII 1
                                    //      2: ATM WAN
                                    //      3: PTM WAN
#define VER_ACCMODE     0x03        //  bit 0: Routing
                                    //      1: Bridging
#define VER_MAJOR       0
#define VER_MID         0
#define VER_MINOR       4

#define ETHSW_INVALID_PORT 0xFF

/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */

/*
 *  PPA Specific Head File
 */
#include "ppa_ss.h"
#include <net/ppa_api_common.h>
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#include "ppa_api_session_limit.h"
#endif
#include <net/ppa_hook.h>
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_hal_selector.h"
#include "ppa_api_core.h"
#include "ppa_api_tools.h"
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
#include "ppa_api_cpu_freq.h"
#endif
#else
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif
#endif

#ifdef CONFIG_PPA_PUMA7
#include <linux/inetdevice.h>
#endif

#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
#include "ppa_api_mfe.h"
#endif
#ifdef CONFIG_LTQ_PPA_QOS
#include "ppa_api_qos.h"
#endif
#include "ppa_api_mib.h"

#ifdef CONFIG_PPA_PUMA7
#include <net/switch-api/lantiq_ethsw_api.h>
#else
#ifndef CONFIG_LTQ_PPA_GRX500
//#ifdef LTQ_ETHSW_API
#include <switch-api/lantiq_ethsw_api.h>
#endif
#endif

/*
 * ####################################
 *              Definition
 * ####################################
 */

#define MIN_HITS                        2

#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
#ifndef CONFIG_LTQ_PPA_GRX500
static int g_snoop_set = 1;
#endif
#endif
/*
 * ####################################
 *              Data Type
 * ####################################
 */
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
typedef struct fidinfo {
	PPA_IFNAME ifname[PPA_IF_NAME_SIZE];
	uint32_t fid; 
	struct fidinfo *next;
} fid_info; 

//By default br0 will have fid 0
/*
static fid_info         def_br_fid = {"br0", 0, NULL};
static fid_info         *fid_list= &def_br_fid;
static uint32_t          g_next_fid=1;
*/
static fid_info         *fid_list=NULL;
static uint32_t          g_next_fid=0;
#else
static uint32_t g_broadcast_bridging_entry = ~0;
#endif


#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
  int32_t ppa_api_directpath_init(void);
  void ppa_api_directpath_exit(void);
#endif


/*
 * ####################################
 *           Global Variable
 * ####################################
 */

uint32_t g_ppa_min_hits = MIN_HITS;

#ifndef CONFIG_PPA_PUMA7
volatile uint8_t g_ppe_fastpath_enabled = 1;
#else
volatile uint8_t g_ppe_fastpath_enabled = 0;
#endif

uint32_t g_ppa_ppa_mtu=DEFAULT_MTU;  /*maximum frame size from ip header to end of the data payload, not including MAC header/pppoe header/vlan */


/*
 * ####################################
 *           Extern Variable
 * ####################################
 */

/*
 * ####################################
 *            Local Function
 * ####################################
 */
static uint8_t g_bridging_mac_learning = 1;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static uint8_t g_bridged_flow_learning = 0;
#else
static uint8_t g_bridged_flow_learning = 1;
#endif

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
void free_fid_list()
{
    fid_info *tmp;   
    while(fid_list) {
	tmp=fid_list;
	fid_list = fid_list->next;
	ppa_free(tmp);
    }
}

int32_t ppa_get_fid(PPA_IFNAME *ifname, uint16_t *fid) 
{
  int32_t ret = PPA_SUCCESS;
  fid_info *tmp;   

  if(ifname) {
   if(fid_list) {
    //searching in the fid list
    tmp=fid_list;
    while(tmp) {
      if(!strncmp(tmp->ifname,ifname, PPA_IF_NAME_SIZE)){
        *fid = tmp->fid;
        break;
      }
      tmp = tmp->next;
    }
    // search returned failure
    if(!tmp) {
      tmp = (fid_info*) ppa_malloc (sizeof(fid_info));
      if(tmp) {
        tmp->next = fid_list;
        ppa_memcpy(tmp->ifname, ifname, PPA_IF_NAME_SIZE);
        tmp->fid = g_next_fid++;
        *fid = tmp->fid;
        fid_list = tmp;
      } else {
        ret = PPA_FAILURE;
      }
    }  
  } else { // first entry in the fid list
    fid_list = (fid_info*) ppa_malloc (sizeof(fid_info));
    if(fid_list) {
      fid_list->next = NULL;
      ppa_memcpy(fid_list->ifname, ifname, PPA_IF_NAME_SIZE);
      fid_list->fid= g_next_fid++;
      *fid = fid_list->fid;
    } else {
      ret = PPA_FAILURE;
    }
  }
 }

  return ret;
} 
#endif

static int32_t ppa_init_new(PPA_INIT_INFO *p_info, uint32_t flags)
{
    int32_t ret;
    uint32_t i;
    PPA_MAX_ENTRY_INFO entry={0};
    PPE_WAN_VID_RANGE vlan_id;
    PPE_ACC_ENABLE acc_cfg={0};
#ifndef CONFIG_LTQ_PPA_GRX500
    uint8_t broadcast_mac[PPA_ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    PPE_FAST_MODE_CFG fast_mode={0};
    PPE_BR_MAC_INFO br_mac={0};
#endif
    if ( ppa_drv_hal_init(0) != PPA_SUCCESS )
    {
        ret = PPA_EIO;
        goto HAL_INIT_ERROR;
    }

    ppa_drv_get_max_entries(&entry, 0);

    ret = PPA_EINVAL;
    if ( p_info->max_lan_source_entries + p_info->max_wan_source_entries > (entry.max_lan_entries + entry.max_wan_entries ))
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Two many entries:%d > %d\n",  p_info->max_lan_source_entries + p_info->max_wan_source_entries , (entry.max_lan_entries + entry.max_wan_entries ) );
        goto MAX_SOURCE_ENTRIES_ERROR;
    }
    if ( p_info->max_mc_entries > entry.max_mc_entries)
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Two many multicast entries:%d > %d\n",   p_info->max_mc_entries , entry.max_mc_entries);
        goto MAX_MC_ENTRIES_ERROR;
    }
    if ( p_info->max_bridging_entries > entry.max_bridging_entries)
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Two many bridge entries:%d > %d\n",   p_info->max_bridging_entries , entry.max_bridging_entries);
        goto MAX_BRG_ENTRIES_ERROR;
    }

    //  disable accelation mode by default
    acc_cfg.f_is_lan = 1;
    acc_cfg.f_enable = PPA_ACC_MODE_NONE;
    if( ppa_drv_set_acc_mode( &acc_cfg, 0) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_acc_mode lan fail\n");
    }
    acc_cfg.f_is_lan = 0;
    acc_cfg.f_enable = PPA_ACC_MODE_NONE;
    if( ppa_drv_set_acc_mode( &acc_cfg, 0) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_acc_mode  wan fail\n");
    }

    if ( (entry.max_lan_entries + entry.max_wan_entries ) || entry.max_mc_entries )
    {
        PPE_ROUTING_CFG cfg;

        //set LAN acceleration
        ppa_memset( &cfg, 0, sizeof(cfg));
        cfg.f_is_lan = 1;
        cfg.entry_num = p_info->max_lan_source_entries;
        cfg.mc_entry_num = 0;
        cfg.f_ip_verify = p_info->lan_rx_checks.f_ip_verify;
        cfg.f_tcpudp_verify=p_info->lan_rx_checks.f_tcp_udp_verify;
        cfg.f_tcpudp_err_drop=p_info->lan_rx_checks.f_tcp_udp_err_drop;
        cfg.f_drop_on_no_hit = p_info->lan_rx_checks.f_drop_on_no_hit;
        cfg.f_mc_drop_on_no_hit = 0;
        cfg.flags = PPA_SET_ROUTE_CFG_ENTRY_NUM | PPA_SET_ROUTE_CFG_IP_VERIFY | PPA_SET_ROUTE_CFG_TCPUDP_VERIFY | PPA_SET_ROUTE_CFG_DROP_ON_NOT_HIT;
        if( ppa_drv_set_route_cfg(&cfg, 0) != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_route_cfg lan fail\n");

        //set WAN acceleration
        ppa_memset( &cfg, 0, sizeof(cfg));
        cfg.f_is_lan = 0;
        cfg.entry_num = p_info->max_wan_source_entries;
        cfg.mc_entry_num = p_info->max_mc_entries;
        cfg.f_ip_verify = p_info->wan_rx_checks.f_ip_verify;
        cfg.f_tcpudp_verify=p_info->wan_rx_checks.f_tcp_udp_verify;
        cfg.f_tcpudp_err_drop=p_info->wan_rx_checks.f_tcp_udp_err_drop;
        cfg.f_drop_on_no_hit = p_info->wan_rx_checks.f_drop_on_no_hit;
        cfg.f_mc_drop_on_no_hit =  p_info->wan_rx_checks.f_mc_drop_on_no_hit;
        cfg.flags = PPA_SET_ROUTE_CFG_ENTRY_NUM | PPA_SET_ROUTE_CFG_MC_ENTRY_NUM | PPA_SET_ROUTE_CFG_IP_VERIFY | PPA_SET_ROUTE_CFG_TCPUDP_VERIFY | PPA_SET_ROUTE_CFG_DROP_ON_NOT_HIT | PPA_SET_ROUTE_CFG_MC_DROP_ON_NOT_HIT;
        if( ppa_drv_set_route_cfg( &cfg, 0) != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_route_cfg wan fail\n");
    }

    if ( entry.max_bridging_entries )
    {
        PPE_BRDG_CFG br_cfg;
        PPE_COUNT_CFG count={0};
        if ( ppa_drv_get_number_of_phys_port( &count, 0) != PPA_SUCCESS )
        {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_get_number_of_phys_port fail\n");
        }

        ppa_memset( &br_cfg, 0, sizeof(br_cfg));
        br_cfg.entry_num = p_info->max_bridging_entries;
        br_cfg.br_to_src_port_mask = (1 << count.num) - 1;   //  br_to_src_port_mask
        br_cfg.flags = PPA_SET_BRIDGING_CFG_ENTRY_NUM | PPA_SET_BRIDGING_CFG_BR_TO_SRC_PORT_EN | PPA_SET_BRIDGING_CFG_DEST_VLAN_EN | PPA_SET_BRIDGING_CFG_SRC_VLAN_EN | PPA_SET_BRIDGING_CFG_MAC_CHANGE_DROP;

        if ( ppa_drv_set_bridging_cfg( &br_cfg, 0) != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_bridging_cfg  fail\n");
    }
//not able to find/map physical ports for PUMA and failing to init
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7    
if ( (ret = ppa_api_netif_manager_init()) != PPA_SUCCESS) {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_api_netif_manager_init  fail\n");
        goto PPA_API_NETIF_CREATE_INIT_FAIL;
}
#else
if ( (ret = ppa_api_netif_manager_init()) != PPA_SUCCESS || !PPA_IS_PORT_CPU0_AVAILABLE() )
    {
        if( ret != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_api_netif_manager_init  fail\n");
        else
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"CPU0 not available\n");
        goto PPA_API_NETIF_CREATE_INIT_FAIL;
    }
#endif

    if ( (ret = ppa_api_session_manager_init()) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_api_session_manager_init  fail\n");
        goto PPA_API_SESSION_MANAGER_INIT_FAIL;
    }
    for ( i = 0; i < p_info->num_lanifs; i++ )
        if ( p_info->p_lanifs[i].ifname != NULL && ppa_netif_add(p_info->p_lanifs[i].ifname, 1, NULL, NULL,0) != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in adding LAN side network interface - %s, reason could be no sufficient memory or LAN/WAN rule violation with physical network interface.\n", p_info->p_lanifs[i].ifname);
    for ( i = 0; i < p_info->num_wanifs; i++ )
        if ( p_info->p_wanifs[i].ifname != NULL && ppa_netif_add(p_info->p_wanifs[i].ifname, 0, NULL, NULL,0) != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in adding WAN side network interface - %s, reason could be no sufficient memory or LAN/WAN rule violation with physical network interface.\n", p_info->p_wanifs[i].ifname);

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
    if ( (ret = ppa_api_directpath_init()) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_api_directpath_init fail - %d\n", ret);
        goto PPA_API_DIRECTPATH_INIT_FAIL;
    }
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
	ppa_hook_directpath_reinject_from_imq_fn = ppa_directpath_reinject_from_imq;
	ppa_directpath_imq_en_flag = 0;
#endif
#endif

/***** network interface mode will not be set by PPA API
    it should be configured by low level driver (eth/atm driver)
    #ifdef CONFIG_LTQ_PPA_A4
        set_if_type(eth0_iftype, 0);
        set_wan_vlan_id((0x010 << 16) | 0x000);  //  high 16 bits is upper of WAN VLAN ID, low 16 bits is min WAN VLAN ID
    #else
        set_if_type(eth0_iftype, 0);
        set_if_type(eth1_iftype, 1);
    #endif
*/

// TBD:Not Applicable for GRX500
    //  this is default setting for LAN/WAN mix mode to use VLAN tag to differentiate LAN/WAN traffic
    vlan_id.vid =(0x010 << 16) | 0x000; //  high 16 bits is upper of WAN VLAN ID, low 16 bits is min WAN VLAN ID
    if( ppa_drv_set_mixed_wan_vlan_id( &vlan_id, 0) != PPA_SUCCESS )//  high 16 bits is upper of WAN VLAN ID, low 16 bits is min WAN VLAN ID
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_mixed_wan_vlan_id fail\n");

/*** WFQ is hidden and not to be changed after firmware running. ***
  set_if_wfq(ETX_WFQ_D4, 1);
  set_fastpath_wfq(FASTPATH_WFQ_D4);
  set_dplus_wfq(DPLUS_WFQ_D4);
*/
#ifndef CONFIG_LTQ_PPA_GRX500
//Not Applicable for GRX500
    for ( i = 0; i < 8; i++ )
    {
        PPE_DEST_LIST cfg;
        cfg.uc_dest_list = 1 << PPA_PORT_CPU0;
        cfg.mc_dest_list = 1 << PPA_PORT_CPU0;
        cfg.if_no = i;
        if( ppa_drv_set_default_dest_list( &cfg, 0 ) != PPA_SUCCESS )
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_default_dest_list fail\n");
    }
//Not Applicable for GRX500
    /*Note, FAST_CPU1_DIRECT is only used for Danube twinpass product */
    fast_mode.mode = PPA_SET_FAST_MODE_CPU1_INDIRECT | PPA_SET_FAST_MODE_ETH1_DIRECT;
    fast_mode.flags = PPA_SET_FAST_MODE_CPU1 | PPA_SET_FAST_MODE_ETH1;
    if( ppa_drv_set_fast_mode( &fast_mode, 0 ) != PPA_SUCCESS )
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_set_fast_mode fail\n");

// Adding switch table rule to forward all the broadcast packets to CPU port
    br_mac.port = PPA_PORT_CPU0;
    ppa_memcpy(br_mac.mac, broadcast_mac, sizeof(br_mac.mac));
    br_mac.f_src_mac_drop = 0;
    br_mac.dslwan_qid = 0;
    br_mac.dest_list = PPA_DEST_LIST_CPU0;
    br_mac.p_entry = g_broadcast_bridging_entry;

    if ( ppa_drv_add_bridging_entry(&br_mac, 0) != PPA_SUCCESS )
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_add_bridging_entry broadcast fail\n");
    g_broadcast_bridging_entry = br_mac.p_entry;
#endif

    if( p_info->add_requires_min_hits )
        g_ppa_min_hits = p_info->add_requires_min_hits;

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    ppa_init_session_limit_params(p_info,0);
#endif

#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
    if( ppa_drv_init_qos_rate(0) != PPA_SUCCESS ){
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_init_qos_rate  fail\n");
    }
#endif /* CONFIG_LTQ_PPA_QOS_RATE_SHAPING */

#ifdef CONFIG_LTQ_PPA_QOS_WFQ
    if( ppa_drv_init_qos_wfq(0) != PPA_SUCCESS ){
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_drv_init_qos_wfq  fail\n");
    }
#endif /* CONFIG_LTQ_PPA_QOS_WFQ */


#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
    ppa_api_cpufreq_init();
#endif
#else
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
    ppa_pwm_init();
#endif
#endif

#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE    
    if( ppa_multifield_control(0, 0) != PPA_SUCCESS ) //by default, disable it
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_multifield_control  fail\n");

#endif /* CONFIG_LTQ_PPA_MFE */

    ppa_set_init_status(PPA_INIT_STATE);

    return PPA_SUCCESS;

PPA_API_SESSION_MANAGER_INIT_FAIL:
    ppa_api_session_manager_exit();
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
PPA_API_DIRECTPATH_INIT_FAIL:
    ppa_api_directpath_exit();
#endif
PPA_API_NETIF_CREATE_INIT_FAIL:
    ppa_api_netif_manager_exit();
MAX_BRG_ENTRIES_ERROR:
MAX_MC_ENTRIES_ERROR:
MAX_SOURCE_ENTRIES_ERROR:
    ppa_drv_hal_exit(0);
HAL_INIT_ERROR:
    ppa_debug(DBG_ENABLE_MASK_ERR,"failed in PPA init\n");
    return ret;
}

static void ppa_exit_new(void)
{
#ifndef CONFIG_LTQ_PPA_GRX500
    PPE_BR_MAC_INFO br_mac={0};
#endif
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
   ppa_hook_directpath_reinject_from_imq_fn = NULL;
   ppa_directpath_imq_en_flag = 0;
#endif
    ppa_set_init_status(PPA_UNINIT_STATE);
    ppa_synchronize_rcu();
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
    ppa_api_cpufreq_exit();
#endif
#else
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
    ppa_pwm_exit();
#endif
#endif
#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
    ppa_multifield_control(0, 0); //by default, disable it
    ppa_quick_del_multifield_flow(-1, 0);
#endif //end of CONFIG_LTQ_PPA_MFE

    ppa_vlan_filter_del_all(0); //sgh add,

#ifndef CONFIG_LTQ_PPA_GRX500
    br_mac.p_entry = g_broadcast_bridging_entry;
    ppa_drv_del_bridging_entry(&br_mac, 0);
#endif

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
     ppa_api_directpath_exit();  //must put before ppa_api_netif_manager_exit since directpath exit will call netif related API
#endif

    ppa_api_session_manager_exit();

    ppa_api_netif_manager_exit();

    ppa_drv_hal_exit(0);

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    free_fid_list();
#endif
    
printk(KERN_INFO "Acceleration module exited!!!\n");    
}



/*
 * ####################################
 *           Global Function
 * ####################################
 */

/*
 *  Function for internal use
 */


/*
 *  PPA Initialization Functions
 */

void ppa_subsystem_id(uint32_t *p_family,
                          uint32_t *p_type,
                          uint32_t *p_if,
                          uint32_t *p_mode,
                          uint32_t *p_major,
                          uint32_t *p_mid,
                          uint32_t *p_minor,
                          uint32_t *p_tag)
{
    if ( p_family )
        *p_family = 0;

    if ( p_type )
        *p_type = 0;

    if ( p_if )
        *p_if = 0;

    if ( p_mode )
        *p_mode = 0;

    if ( p_major )
        *p_major = PPA_SUBSYSTEM_MAJOR;

    if ( p_mid )
        *p_mid = PPA_SUBSYSTEM_MID;

    if ( p_minor )
        *p_minor = PPA_SUBSYSTEM_MINOR;

    if( p_tag ) 
        *p_tag = PPA_SUBSYSTEM_TAG;
}

void ppa_get_api_id(uint32_t *p_family,
                        uint32_t *p_type,
                        uint32_t *p_if,
                        uint32_t *p_mode,
                        uint32_t *p_major,
                        uint32_t *p_mid,
                        uint32_t *p_minor)
{
    if ( p_family )
        *p_family = VER_FAMILY;

    if ( p_type )
        *p_type = VER_DRTYPE;

    if ( p_if )
        *p_if = VER_INTERFACE;

    if ( p_mode )
        *p_mode = VER_ACCMODE;

    if ( p_major )
        *p_major = VER_MAJOR;

    if ( p_mid )
        *p_mid = VER_MID;

    if ( p_minor )
        *p_minor = VER_MINOR;
}

int32_t ppa_init(PPA_INIT_INFO *p_info, uint32_t flags)
{
    int32_t ret;

    if ( !p_info )
        return PPA_EINVAL;

    if ( ppa_is_init() )
        ppa_exit();

    if ( (ret = ppa_init_new(p_info, flags)) == PPA_SUCCESS )
        printk("ppa_init - init succeeded\n");
    else
        printk("ppa_init - init failed (%d)\n", ret);

    return ret;
}

void ppa_exit(void)
{
    if ( ppa_is_init() )
        ppa_exit_new();
}

/*
 *  PPA Enable/Disable and Status Functions
 */

int32_t ppa_enable(uint32_t lan_rx_ppa_enable, uint32_t wan_rx_ppa_enable, uint32_t flags)
{
    u32 sys_flag;
    PPE_ACC_ENABLE acc_cfg;

    if ( ppa_is_init() )
    {
        lan_rx_ppa_enable = lan_rx_ppa_enable ? PPA_ACC_MODE_ROUTING : PPA_ACC_MODE_NONE;
        wan_rx_ppa_enable = wan_rx_ppa_enable ? PPA_ACC_MODE_ROUTING : PPA_ACC_MODE_NONE;
        sys_flag = ppa_disable_int();
        acc_cfg.f_is_lan =1;
        acc_cfg.f_enable = lan_rx_ppa_enable;
        ppa_drv_set_acc_mode(&acc_cfg, 0);

        acc_cfg.f_is_lan =0;
        acc_cfg.f_enable = wan_rx_ppa_enable;
        ppa_drv_set_acc_mode(&acc_cfg, 0);
        ppa_enable_int(sys_flag);
        return PPA_SUCCESS;
    }
    return PPA_FAILURE;
}

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE

/*
 *  PPA Set Unicast/multicast session mib mode configuration 
 */

int32_t ppa_set_mib_mode(uint8_t mib_mode)
{
    PPE_MIB_MODE_ENABLE mib_cfg;

    mib_cfg.session_mib_unit = mib_mode; 
    
    ppa_drv_set_mib_mode(&mib_cfg,0);

    return PPA_SUCCESS;
}

/*
 *  PPA Get Unicast/multicast session mib mode configuration 
 */

int32_t ppa_get_mib_mode(uint8_t *mib_mode)
{
    PPE_MIB_MODE_ENABLE mib_cfg;
    
    ppa_drv_get_mib_mode(&mib_cfg);

    *mib_mode =mib_cfg.session_mib_unit; 

    return PPA_SUCCESS;
}

#endif

int32_t ppa_get_status(uint32_t *lan_rx_ppa_enable, uint32_t *wan_rx_ppa_enable, uint32_t flags)
{
    if ( ppa_is_init() )
    {
        PPE_ACC_ENABLE cfg;

        cfg.f_is_lan = 1;
        ppa_drv_get_acc_mode(&cfg, 0);
		if( lan_rx_ppa_enable ) *lan_rx_ppa_enable = cfg.f_enable;

        cfg.f_is_lan = 0;
        ppa_drv_get_acc_mode(&cfg, 0);
		if( wan_rx_ppa_enable ) *wan_rx_ppa_enable = cfg.f_enable;
        return PPA_SUCCESS;
    }
    return PPA_FAILURE;
}

static INLINE int32_t ppa_pkt_filter(PPA_BUF *ppa_buf, uint32_t flags)
{
    //basic pkt filter
    PPA_NETIF *tx_if;
    PPA_IFNAME *ifname;
    struct netif_info *p_info;
    uint32_t flag=0;
    uint16_t sport,dport;
    uint8_t proto;

    //  ignore packets output by the device
    if ( ppa_is_pkt_host_output(ppa_buf) )
    {
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
	if( ( ppa_get_pkt_ip_proto(ppa_buf) != PPA_IPPROTO_TCP ) ) { 
	    ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"ppa_is_pkt_host_output\n");
	    return PPA_SESSION_FILTED;
	}
#else
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"ppa_is_pkt_host_output\n");
        return PPA_SESSION_FILTED;
#endif
    }

    //  ignore incoming broadcast
    if ( ppa_is_pkt_broadcast(ppa_buf) )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"ppa_is_pkt_broadcast\n");
        return PPA_SESSION_FILTED;
    }

    //  ignore multicast packet in unitcast routing but learn multicast source interface automatically
    if (  ppa_is_pkt_multicast(ppa_buf) )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT, "ppa_is_pkt_multicast\n");

        /*auto learn multicast source interface*/
        if ( (flags & PPA_F_BEFORE_NAT_TRANSFORM ) && ppa_hook_multicast_pkt_srcif_add_fn )
            ppa_hook_multicast_pkt_srcif_add_fn(ppa_buf, NULL);

        return PPA_SESSION_FILTED;
    }

    //  ignore loopback packet
    if ( ppa_is_pkt_loopback(ppa_buf) )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"ppa_is_pkt_loopback\n");
        return PPA_SESSION_FILTED;
    }

    //  ignore protocols other than TCP/UDP, since some of them (e.g. ICMP) can't be handled safe in this arch
    proto = ppa_get_pkt_ip_proto(ppa_buf); 
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
    if ( proto != PPA_IPPROTO_UDP && proto != PPA_IPPROTO_TCP && proto != 50)
#else
    if ( proto != PPA_IPPROTO_UDP && proto != PPA_IPPROTO_TCP )
#endif
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"protocol: %u\n", (uint32_t)(proto));
        return PPA_SESSION_FILTED;
    }

    /*  
     * Ignore special sessions with broadcast addresses and protocols with
     * high probability of less packets being exchanged
     * 1. windows netbios
     * 2. DHCP
     * 3. DNS
     */
    sport = ppa_get_pkt_src_port(ppa_buf);
    dport = ppa_get_pkt_dst_port(ppa_buf);
    if ( sport == 137 || dport == 137
        || sport == 138 || dport == 138 
        || sport == 68 || sport == 67
        || dport == 68 || dport == 67
        || sport == 53 || dport == 53 
        || sport == 1701 || dport == 1701 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"src port = %d, dst port = %d\n", sport,dport);
        return PPA_SESSION_FILTED;
    }

    //  ignore fragment packet
    if ( ppa_is_pkt_fragment(ppa_buf) )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,"fragment\n");
        return PPA_SESSION_FILTED;
    }

    tx_if = ppa_get_pkt_dst_if(ppa_buf);
    if (tx_if)
    {
        ifname = ppa_get_netif_name(tx_if);
    }
    else
    {
        goto skip_lookup;
    }
    if( ppa_netif_lookup(ifname, &p_info) != PPA_SUCCESS )
    {
        return PPA_ENOTAVAIL;
    }

    flag = p_info->flags;

skip_lookup:

    if(!(flags & PPA_F_BEFORE_NAT_TRANSFORM) ){
        //  handle routed packet only
        if( !(flag & NETIF_PPPOL2TP) )
        {
            if ( !ppa_is_pkt_routing(ppa_buf) )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"not routing packet\n");
                return PPA_SESSION_FILTED;
            }
        }
    }

    return PPA_SESSION_NOT_FILTED; 

}

int32_t ppa_hook_set_ppe_fastpath_enable(uint32_t f_enable, uint32_t flags)
{
    g_ppe_fastpath_enabled = f_enable;
    return PPA_SUCCESS;
}

int32_t ppa_hook_get_ppe_fastpath_enable(uint32_t *f_enable, uint32_t flags)
{
    if( f_enable )
        *f_enable = g_ppe_fastpath_enabled;
    return PPA_SUCCESS;
}

/*
 *  PPA Routing Session Operation Functions
 */

#define  ppa_session_print(p_item) printk("<%s> %s: proto:%d %pI4:%d <-> %pI4:%d count=%u\n", \
                                     __FUNCTION__,  \
                                     ppa_is_BrSession((p_item)?"Bridged":"Routed"), \
                                     p_item->ip_proto, \
                                     &p_item->src_ip,p_item->src_port, \
                                     &p_item->dst_ip,p_item->dst_port, \
                                     p_item->num_adds);
int32_t ppa_session_add(PPA_BUF *ppa_buf, PPA_SESSION *p_session, uint32_t flags)
{
  int32_t ret = PPA_SESSION_NOT_ADDED;
  struct session_list_item* p_item=NULL;

  if(!ppa_buf){
    return PPA_SESSION_NOT_ADDED;
  }

  if( (flags & PPA_F_BRIDGED_SESSION) && !g_bridged_flow_learning )
    return PPA_SESSION_NOT_ADDED; /* No bridged flow learning */

  if( p_session ) {
    ret = ppa_session_find_by_tuple(p_session, 
                                    flags & PPA_F_SESSION_REPLY_DIR, 
                                    &p_item);
  } else {
    ret = ppa_find_session_from_skb(ppa_buf,0,&p_item);
  }

  if( PPA_SESSION_EXISTS != ret ) {

    if( unlikely(NULL == p_session && !(flags & PPA_F_BRIDGED_SESSION))) {
      return PPA_SESSION_NOT_ADDED;
    }

    if(likely(flags & PPA_F_BEFORE_NAT_TRANSFORM) ) {
      /* Filter packet */
      if((ret = ppa_pkt_filter(ppa_buf, flags)) == PPA_SESSION_FILTED) {
        return ret;
      }

      if ( ppa_add_session(ppa_buf, p_session, flags, &p_item) != PPA_SUCCESS ) {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_add_session failed\n");
        return PPA_SESSION_NOT_ADDED;
      }
    } else {
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
        if(ppa_get_pkt_ip_proto(ppa_buf) == 50) 
        {
                ret = ppa_find_session_for_ipsec(ppa_buf,0,&p_item);
                if(ret != PPA_SESSION_EXISTS)
                { 
      		  return PPA_SESSION_NOT_ADDED;
                }
        }
        else
      		return PPA_SESSION_NOT_ADDED;


#else
      return PPA_SESSION_NOT_ADDED;
#endif

    }
  } 

  //ppa_session_print(p_item);
  ppa_session_list_lock(); //Need to update session from here on
  if( p_session && ppa_is_BrSession(p_item) ) {
    /* When control comes here, it must be from PRE routing hook
       from routed path */
    ppa_session_not_bridged(p_item,p_session);
    goto done;/* Packet is already seen in bridged path */
  }

  if( ! ppa_is_BrSession(p_item) && (flags & PPA_F_BRIDGED_SESSION) && 
		(flags & PPA_F_BEFORE_NAT_TRANSFORM))
	goto done; //Routed session...but seen in bridged path

  /* Note: ppa_speed_handle_frame returns PPA_SESSION_NOT_FILTED only in POST
     routing */
  ret = ppa_speed_handle_frame(ppa_buf, p_item, flags);
  if(ret == PPA_SESSION_NOT_FILTED ) {
        //  in case compiler optimization problem
		PPA_SYNC();
		ret = ppa_update_session(ppa_buf, p_item, flags);
    }

done:
  __ppa_session_put(p_item);
  ppa_session_list_unlock();
  return ret;
}

int32_t ppa_session_modify(PPA_SESSION *p_session, PPA_SESSION_EXTRA *p_extra, uint32_t flags)
{
  struct session_list_item *p_item=NULL;
  int32_t ret = PPA_FAILURE;

  /* Since session parameters being updated, so take spin lock */
  ppa_session_list_lock();
  if ( PPA_SESSION_EXISTS == __ppa_session_find_by_ct(p_session, 
                                  flags & PPA_F_SESSION_REPLY_DIR, &p_item) ) {

    ppa_update_session_extra(p_extra, p_item, flags);
    if ( (p_item->flags & SESSION_ADDED_IN_HW) && (flags != 0)  ) {

      if( !(p_item->flags & SESSION_NOT_ACCEL_FOR_MGM) 
#ifdef CONFIG_LTQ_PPA_API_SW_FASTPATH
        && !(p_item->flags & SESSION_ADDED_IN_SW)
#endif  
        ) {
          
        if ( ppa_hw_update_session_extra(p_item, flags) != PPA_SUCCESS ) {
                    //  update failed
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
          ppa_hsel_del_routing_session(p_item);
#else
          ppa_hw_del_session(p_item);
#endif
#ifdef CONFIG_LTQ_PPA_API_SW_FASTPATH
          /* session was in hardware an the modification failed; so the 
             session is moved out of HW and put in SW fastpath */
          p_item->flags |= SESSION_ADDED_IN_SW;
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
		update_session_mgmt_stats(p_item, ADD);
#endif
#endif
          goto __MODIFY_DONE;
          
        }
    
      } else { 
        //just remove the accelerated session from PPE FW, no need to update other flags since PPA hook will rewrite them.
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
        ppa_hsel_del_routing_session(p_item);
#else
        ppa_hw_del_session(p_item);
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH) && defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
		update_session_mgmt_stats(p_item, ADD);
#endif
      }
    }
   
    ret = PPA_SUCCESS;
  } else{
    ret = PPA_FAILURE;
  }

__MODIFY_DONE:
  if( p_item ) __ppa_session_put(p_item);
  ppa_session_list_unlock();
    return ret;
}

int32_t ppa_session_get(PPA_SESSION ***pp_sessions, PPA_SESSION_EXTRA **pp_extra, int32_t *p_num_entries, uint32_t flags)
{
//#warning ppa_session_get is not implemented
    return PPA_ENOTIMPL;
}



/*
 *  PPA IPSec Session Operation Functions
 */

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
int32_t ppa_session_ipsec_add(PPA_XFRM_STATE *ppa_x, sa_direction dir)
{
  int32_t ret = PPA_SESSION_NOT_ADDED;
  uint32_t tunnel_index =0;

  if(!ppa_x){
    return PPA_SESSION_NOT_ADDED;
  }
  if(ppa_add_ipsec_tunnel_tbl_entry(ppa_x,dir,&tunnel_index) != PPA_SUCCESS)
      return PPA_SESSION_NOT_ADDED;

    ppa_ipsec_get_session_lock();

    if(dir == INBOUND) {
        ret =  ppa_ipsec_add_entry(tunnel_index);

    }
    else {
        ret =  ppa_ipsec_add_entry_outbound(tunnel_index);
    }
    
    ppa_ipsec_release_session_lock();

    return ret;

}

int32_t ppa_session_ipsec_delete(PPA_XFRM_STATE *ppa_x)
{
  int32_t ret = PPA_SESSION_NOT_DELETED;
  struct session_list_item *p_item;
  uint32_t tunnel_index =0;
  sa_direction dir;

  if(!ppa_x){
    return PPA_SESSION_NOT_DELETED;
  }
  if(ppa_get_ipsec_tunnel_tbl_entry(ppa_x,&dir,&tunnel_index) != PPA_SUCCESS)
      return PPA_SESSION_NOT_DELETED;

    ppa_ipsec_get_session_lock();

    if(dir == INBOUND) {
        ret = __ppa_lookup_ipsec_group(ppa_x, &p_item);
	if ( ret  == PPA_IPSEC_EXISTS ){
        	ret =  ppa_ipsec_del_entry(p_item);
    	}
    	else
            ret = PPA_SESSION_NOT_DELETED;
    }
    else {
        ret =  ppa_ipsec_del_entry_outbound(tunnel_index);
    }

    ppa_add_ipsec_tunnel_tbl_update(dir, tunnel_index );
    ppa_ipsec_release_session_lock();

    return ret;

} 
#endif


#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG

int32_t ppa_capwap_check_ip(PPA_CMD_CAPWAP_INFO *ppa_capwap_entry)
{
    if( is_ip_zero(&ppa_capwap_entry->source_ip) )
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "ppa_capwap_group_update not support zero ip address\n");
        return PPA_FAILURE;
    }

    if( is_ip_zero(&ppa_capwap_entry->dest_ip) )
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "ppa_capwap_group_update not support zero ip address\n");
        return PPA_FAILURE;
    }


    if(is_ip_allbit1(&ppa_capwap_entry->source_ip)){
        if(g_ppa_dbg_enable & DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP){
            ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "source ip not support all bit 1 ip address, except dbg enabled\n");
        }else{
            return PPA_FAILURE;
        }
    }

    if(is_ip_allbit1(&ppa_capwap_entry->dest_ip)){
        if(g_ppa_dbg_enable & DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP){
            ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "destination ip not support all bit 1 ip address, except dbg enabled\n");
        }else{
            return PPA_FAILURE;
        }
    }

    return PPA_SUCCESS;
}

int32_t ppa_capwap_add_entry(PPA_CMD_CAPWAP_INFO *ppa_capwap_entry)
{
    struct capwap_group_list_item *p_item;
    //  add new capwap groups
    if ( ppa_add_capwap_group(ppa_capwap_entry, &p_item) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "ppa_add_capwap_group fail\n");
        return PPA_CAPWAP_NOT_ADDED;
    }

    if ( ppa_drv_add_capwap_entry(ppa_capwap_entry,0 ) == PPA_SUCCESS )
    {
         p_item->p_entry = ppa_capwap_entry->p_entry;
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP,
                        "ppa_hw_add_capwap_group(%d.%d.%d.%d): fail",
                        ppa_capwap_entry->source_ip.ip.ip >> 24,
                        (ppa_capwap_entry->source_ip.ip.ip >> 16) & 0xFF,
                        (ppa_capwap_entry->source_ip.ip.ip >> 8) & 0xFF,
                        ppa_capwap_entry->source_ip.ip.ip & 0xFF);
        ppa_remove_capwap_group(p_item);
        return PPA_CAPWAP_NOT_ADDED;
    }
    ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "capwap hardware added\n");

    return PPA_CAPWAP_ADDED;
}

int32_t ppa_capwap_del_entry(PPA_CMD_CAPWAP_INFO *ppa_capwap_entry,struct
                capwap_group_list_item *p_item)
{
    
    ppa_capwap_entry->p_entry = p_item->p_entry;

    if ( ppa_drv_delete_capwap_entry(ppa_capwap_entry,0 ) != PPA_SUCCESS )
         return PPA_FAILURE;
    
    ppa_list_delete(&p_item->capwap_list);
    ppa_free_capwap_group_list_item(p_item); 
    
    return PPA_SUCCESS;
    
}


/*
 *  PPA CAPWAP configuration Functions
 */
int32_t ppa_capwap_update(PPA_CMD_CAPWAP_INFO *ppa_capwap_entry)
{
    struct capwap_group_list_item *p_item;
    int32_t ret;

    { //for print debug information only
            ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "ppa_capwap_update for source ip: %d.%d.%d.%d \n", NIPQUAD(ppa_capwap_entry->source_ip.ip.ip));
            ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "destination ip : %d.%d.%d.%d \n", NIPQUAD(ppa_capwap_entry->dest_ip.ip.ip));

    }

    if(ppa_capwap_check_ip(ppa_capwap_entry) != PPA_SUCCESS)
        return PPA_FAILURE;
    

    ppa_capwap_get_table_lock();
    ret = __ppa_lookup_capwap_group_add(&ppa_capwap_entry->source_ip, &ppa_capwap_entry->dest_ip,ppa_capwap_entry->directpath_portid,&p_item);
    if ( ret  == PPA_CAPWAP_NOT_ADDED ){
        ret =  ppa_capwap_add_entry(ppa_capwap_entry);
        if (ret == PPA_CAPWAP_NOT_ADDED)
                ret = PPA_FAILURE;
        else
                ret = PPA_SUCCESS;
    }
    else
    {
        ret = __ppa_capwap_group_update(ppa_capwap_entry, p_item);
    }

    ppa_capwap_release_table_lock();

    return ret;
}

int32_t ppa_capwap_delete(PPA_CMD_CAPWAP_INFO *ppa_capwap_entry)
{
    struct capwap_group_list_item *p_item;
    int32_t ret;

    { //for print debug information only
            ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "ppa_capwap_delete for source ip: %d.%d.%d.%d \n", NIPQUAD(ppa_capwap_entry->source_ip.ip.ip));
            ppa_debug(DBG_ENABLE_MASK_DUMP_CAPWAP_GROUP, "destination ip : %d.%d.%d.%d \n", NIPQUAD(ppa_capwap_entry->dest_ip.ip.ip));


    }

    if(ppa_capwap_check_ip(ppa_capwap_entry) != PPA_SUCCESS)
        return PPA_FAILURE;
    
    ppa_capwap_get_table_lock();
    
    ret = __ppa_lookup_capwap_group(&ppa_capwap_entry->source_ip, &ppa_capwap_entry->dest_ip,ppa_capwap_entry->directpath_portid, &p_item);
    if ( ret  == PPA_CAPWAP_EXISTS ){
        ret =  ppa_capwap_del_entry(ppa_capwap_entry,p_item);
    }
    else
            ret = PPA_FAILURE;

    ppa_capwap_release_table_lock();

    return ret;
}


#endif


int32_t ppa_mc_check_ip(PPA_MC_GROUP *ppa_mc_entry)
{
    if( is_ip_zero(&ppa_mc_entry->ip_mc_group) )
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_mc_group_update not support zero ip address\n");
        return PPA_FAILURE;
    }

    if(is_ip_allbit1(&ppa_mc_entry->source_ip)){
        if(g_ppa_dbg_enable & DBG_ENABLE_MASK_DUMP_MC_GROUP){
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "source ip not support all bit 1 ip address, except dbg enabled\n");
        }else{
            return PPA_FAILURE;
        }
    }
    if(is_ip_zero(&ppa_mc_entry->source_ip)){
		if ( ppa_mc_entry->SSM_flag == 1 ){//Must provide src ip if SSM_flag is set
		    ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "SMM flag set but no souce ip provided\n");
			return PPA_FAILURE;
		}
    }else if(ppa_mc_entry->ip_mc_group.f_ipv6 != ppa_mc_entry->source_ip.f_ipv6){ //mc group ip & source ip must both be ipv4 or ipv6
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "MC group IP and source ip not in same IPv4/IPv6 type\n");
		return PPA_FAILURE;
	}

    return PPA_SUCCESS;
}


int32_t ppa_mc_add_entry(PPA_MC_GROUP *ppa_mc_entry, uint32_t flags)
{
    struct mc_group_list_item *p_item;
    //  add new mc groups
    if ( ppa_add_mc_group(ppa_mc_entry, &p_item, flags) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_add_mc_group fail\n");
        return PPA_SESSION_NOT_ADDED;
    }

    if ( p_item->src_netif == NULL )    // only added in PPA level, not PPE FW level since source interface not get yet.
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "IGMP request no src_netif. No acceleration !\n");
        return PPA_SESSION_ADDED;
    }
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    if ( ppa_hsel_add_wan_mc_group(p_item) != PPA_SUCCESS )
#else
    if ( ppa_hw_add_mc_group(p_item) != PPA_SUCCESS )
#endif
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_hw_add_mc_group(%d.%d.%d.%d): fail", ppa_mc_entry->ip_mc_group.ip.ip >> 24, (ppa_mc_entry->ip_mc_group.ip.ip >> 16) & 0xFF, (ppa_mc_entry->ip_mc_group.ip.ip >> 8) & 0xFF, ppa_mc_entry->ip_mc_group.ip.ip & 0xFF);
        // keep p_item added in PPA level
        return PPA_SESSION_ADDED;
    }
    ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "hardware added\n");

    return PPA_SESSION_ADDED;
}

/*
 *  PPA Multicast Routing Session Operation Functions
 */

int32_t ppa_mc_group_update(PPA_MC_GROUP *ppa_mc_entry, uint32_t flags)
{
    struct mc_group_list_item *p_item;
    int32_t ret;

    { //for print debug information only
        if(ppa_mc_entry->ip_mc_group.f_ipv6 == 0){
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_mc_group_update for group: %d.%d.%d.%d \n", NIPQUAD(ppa_mc_entry->ip_mc_group.ip.ip));
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "source ip : %d.%d.%d.%d \n", NIPQUAD(ppa_mc_entry->source_ip.ip.ip));
        }else{
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "group ip: "NIP6_FMT"\n", NIP6(ppa_mc_entry->ip_mc_group.ip.ip6));
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "source ip: "NIP6_FMT"\n", NIP6(ppa_mc_entry->source_ip.ip.ip6));
        }
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "from %s ", ppa_mc_entry->src_ifname ? ppa_mc_entry->src_ifname: "NULL" );

        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "to ");
        if( ppa_mc_entry->num_ifs ==0 ) ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "NULL" );
        else
        {
            int i, bit;
            for(i=0, bit=1; i<ppa_mc_entry->num_ifs; i++ )
            {
                if ( ppa_mc_entry->if_mask & bit )
                    ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "%s ", ppa_mc_entry->array_mem_ifs[i].ifname? ppa_mc_entry->array_mem_ifs[i].ifname:"NULL");

                bit = bit<<1;
            }
        }
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "with ssm=%d flags=%x\n",  ppa_mc_entry->SSM_flag, flags );
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "lan itf num:%d, mask:%x\n",  ppa_mc_entry->num_ifs, ppa_mc_entry->if_mask);
    }

    if(ppa_mc_check_ip(ppa_mc_entry) != PPA_SUCCESS)
        return PPA_FAILURE;

    if(ppa_mc_entry->num_ifs == 0){//delete action: if SMM flag == 0, don't care src ip
        ppa_delete_mc_group(ppa_mc_entry);
        return PPA_SUCCESS;
    }
    

    ppa_mc_get_htable_lock();
    ret = __ppa_lookup_mc_group(&ppa_mc_entry->ip_mc_group, &ppa_mc_entry->source_ip, &p_item);
    if(ret == PPA_MC_SESSION_VIOLATION){//Cannot add or update
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "IGMP violation, cannot be added or updated\n");
        ret = PPA_FAILURE;
    }
    else if ( ret  != PPA_SESSION_EXISTS ){
        ret =  ppa_mc_add_entry(ppa_mc_entry, flags);
    }
    else
    {
        ret = __ppa_mc_group_update(ppa_mc_entry, p_item, flags);
    }

    ppa_mc_release_htable_lock();

    return ret;
}

int32_t ppa_mc_group_get(IP_ADDR_C  ip_mc_group, IP_ADDR_C src_ip, PPA_MC_GROUP *ppa_mc_entry, uint32_t flags)
{
    struct mc_group_list_item *p_item = NULL;
    PPA_IFNAME *ifname;
    uint32_t idx;
    uint32_t bit;
    uint32_t i;

    ASSERT(ppa_mc_entry != NULL, "ppa_mc_entry == NULL");

    ppa_mc_get_htable_lock();
    if ( __ppa_lookup_mc_group(&ip_mc_group, &src_ip,  &p_item) != PPA_SESSION_EXISTS ){
        ppa_mc_release_htable_lock();
        return PPA_ENOTAVAIL;
    }

    ppa_memcpy( &ppa_mc_entry->ip_mc_group, &p_item->ip_mc_group, sizeof( ppa_mc_entry->ip_mc_group  ) );
    ppa_memcpy( &ppa_mc_entry->source_ip, &p_item->source_ip, sizeof( ppa_mc_entry->ip_mc_group ));


    for ( i = 0, bit = 1, idx = 0; i < PPA_MAX_MC_IFS_NUM; i++, bit <<= 1 )
        if ( (p_item->if_mask & bit) )
        {
            ifname = ppa_get_netif_name(p_item->netif[i]);
            if ( ifname )
            {
                ppa_mc_entry->array_mem_ifs[idx].ifname = ifname;
                ppa_mc_entry->array_mem_ifs[idx].ttl    = p_item->ttl[i];
                ppa_mc_entry->if_mask |= (1 << idx);
                idx++;
            }
        }

    ppa_mc_entry->num_ifs = idx;
    //ppa_mc_entry->if_mask = (1 << idx) - 1;
    ppa_mc_release_htable_lock();

    return PPA_SUCCESS;
}

int32_t ppa_mc_entry_modify(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, PPA_MC_GROUP *ppa_mc_entry, PPA_SESSION_EXTRA *p_extra, uint32_t flags)
{
    struct mc_group_list_item *p_item;
    int32_t ret = PPA_FAILURE;

    ppa_mc_get_htable_lock();
    if ( __ppa_lookup_mc_group(&ip_mc_group, &src_ip, &p_item) == PPA_SESSION_EXISTS )
    {
        ppa_update_mc_group_extra(p_extra, p_item, flags);
        if ( (p_item->flags & SESSION_ADDED_IN_HW) )
        {
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	    if ( ppa_hsel_update_wan_mc_group(p_item, flags) != PPA_SUCCESS )
#else
            if ( ppa_hw_update_mc_group_extra(p_item, flags) != PPA_SUCCESS )
#endif
            {
                ppa_remove_mc_group(p_item);
                ret = PPA_FAILURE;
            }
        }
        ret = PPA_SUCCESS;
    }
    ppa_mc_release_htable_lock();

    return ret;
}

int32_t ppa_mc_entry_get(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, PPA_SESSION_EXTRA *p_extra, uint32_t flags)
{
    struct mc_group_list_item *p_item;
    int32_t ret = PPA_FAILURE;

    if ( !p_extra )
        return PPA_EINVAL;

    ppa_mc_get_htable_lock();
    if ( __ppa_lookup_mc_group(&ip_mc_group, &src_ip, &p_item) == PPA_SESSION_EXISTS )
    {
        ppa_memset(p_extra, 0, sizeof(*p_extra));

        p_extra->session_flags = flags;

        if ( (flags & PPA_F_SESSION_NEW_DSCP) )
        {
            if ( (p_item->flags & SESSION_VALID_NEW_DSCP) )
            {
                p_extra->dscp_remark = 1;
                p_extra->new_dscp = p_item->new_dscp;
            }
        }

        if ( (flags & PPA_F_SESSION_VLAN) )
        {
            if ( (p_item->flags & SESSION_VALID_VLAN_INS) )
            {
                p_extra->vlan_insert = 1;
                p_extra->vlan_prio   = p_item->new_vci >> 13;
                p_extra->vlan_cfi    = (p_item->new_vci >> 12) & 0x01;
                p_extra->vlan_id     = p_item->new_vci & ((1 << 12) - 1);
            }

            if ( (p_item->flags & SESSION_VALID_VLAN_RM) )
                p_extra->vlan_remove = 1;
        }

        if ( (flags & PPA_F_SESSION_OUT_VLAN) )
        {
            if ( (p_item->flags & SESSION_VALID_OUT_VLAN_INS) )
            {
                p_extra->out_vlan_insert = 1;
                p_extra->out_vlan_tag    = p_item->out_vlan_tag;
            }

            if ( (p_item->flags & SESSION_VALID_OUT_VLAN_RM) )
                p_extra->out_vlan_remove = 1;
        }

         p_extra->dslwan_qid_remark = 1;
         p_extra->dslwan_qid        = p_item->dslwan_qid;
         p_extra->out_vlan_tag      = p_item->out_vlan_tag;

        ret = PPA_SUCCESS;
    }
    
    ppa_mc_release_htable_lock();

    return ret;
}

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
int32_t ppa_mc_entry_rtp_get(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, uint8_t* p_RTP_flag)
{
    struct mc_group_list_item *p_item;
    int32_t ret = PPA_FAILURE;

    if ( !p_RTP_flag )
        return PPA_EINVAL;

    ppa_mc_get_htable_lock();
    if ( __ppa_lookup_mc_group(&ip_mc_group, &src_ip, &p_item) == PPA_SESSION_EXISTS )
    {
        *p_RTP_flag = p_item->RTP_flag;
        ret = PPA_SUCCESS;
    }
    
    ppa_mc_release_htable_lock();

    return ret;
}

#endif


/*
 *  PPA Unicast Session Timeout Functions
 */

int32_t ppa_inactivity_status(PPA_U_SESSION *p_session)
{
  int f_flag = 0;
  int f_timeout = 1;
  int32_t ret;
  struct session_list_item *p_item;
  uint32_t timeDiff;

  p_item = NULL;
          
  ret = ppa_session_find_by_ct((PPA_SESSION *)p_session, 0, &p_item);

  if( ret == PPA_SESSION_EXISTS ) {

    if( p_item->ip_proto == PPA_IPPROTO_TCP && 
        ! ppa_is_tcp_established(p_session) ) {

      ppa_session_put(p_item);
      return PPA_TIMEOUT;
    }
    if( p_item->flags & (SESSION_ADDED_IN_HW | SESSION_ADDED_IN_SW)) {

      f_flag = 1;
      timeDiff = ppa_get_time_in_sec() - p_item->last_hit_time;
      if ( p_item->timeout >= timeDiff)
        f_timeout = 0;
      ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,
                "session %p, timeout=%u Time since last hit=%u\n", 
                p_session, p_item->timeout, timeDiff);
    }
      
    ppa_session_put(p_item);
  }

  ret = ppa_session_find_by_ct((PPA_SESSION *)p_session, 1, &p_item);
    
  if( ret == PPA_SESSION_EXISTS ) {

    if(p_item->flags & (SESSION_ADDED_IN_HW | SESSION_ADDED_IN_SW)) {

      f_flag = 1;
      timeDiff = ppa_get_time_in_sec() - p_item->last_hit_time;
      if( p_item->timeout >= timeDiff)
        f_timeout = 0;
      ppa_debug(DBG_ENABLE_MASK_DEBUG2_PRINT,
                "session %p, timeout=%u Time since last hit=%u\n", 
                p_session, p_item->timeout, timeDiff);
    }

    ppa_session_put(p_item);
  }

  if(g_ppa_dbg_enable & DBG_ENABLE_MASK_SESSION){//if session dbg enable, keep it from timeout
      return PPA_HIT;
  }
  //  not added in hardware
  if ( !f_flag )
    return PPA_SESSION_NOT_ADDED; //Why it is required???

  return f_timeout ? PPA_TIMEOUT : PPA_HIT;
}

int32_t ppa_set_session_inactivity(PPA_U_SESSION *p_session, int32_t timeout)
{
  int32_t ret;
  struct session_list_item *p_item;

  if( p_session == NULL ) //for modifying ppa routing hook timer purpose
  {
    ppa_set_polling_timer(timeout, 1);
    return PPA_SUCCESS;
  }

  /* Since session parameters being updated, so take spin lock */
  ppa_session_list_lock(); 
  ret = __ppa_session_find_by_ct((PPA_SESSION *)p_session, 0, &p_item);
  if (ret == PPA_SESSION_EXISTS){
    p_item->timeout = timeout;
    __ppa_session_put(p_item);
  }
    
  ret = __ppa_session_find_by_ct((PPA_SESSION *)p_session, 1, &p_item);

  if (ret == PPA_SESSION_EXISTS){
    p_item->timeout = timeout;
    __ppa_session_put(p_item);
  }
  ppa_session_list_unlock();
    
  ppa_set_polling_timer(timeout, 0);

  return PPA_SUCCESS;
}

uint8_t ppa_session_bridged_flow_status(void)
{
  return g_bridged_flow_learning;
}
EXPORT_SYMBOL(ppa_session_bridged_flow_status);

void ppa_session_bridged_flow_set_status(uint8_t fEnable)
{
  if( fEnable ) {
	      
    ppa_reg_export_fn(PPA_SESSION_BRADD_FN, 
                      (uint32_t)ppa_session_add, 
                      "ppa_session_bradd",
                      (uint32_t *)&ppa_hook_session_bradd_fn, 
                      (uint32_t)ppa_session_bradd_rpfn );
  } else {
        
    /* Un-register the hooks */
    ppa_unreg_export_fn(PPA_SESSION_BRADD_FN, 
                        (uint32_t *)&ppa_hook_session_bradd_fn);
  }
  g_bridged_flow_learning = fEnable;
}
EXPORT_SYMBOL(ppa_session_bridged_flow_set_status);

/*
 *  PPA Bridge MAC learning Operation Functions
 */

int32_t ppa_bridge_entry_add(uint8_t *mac_addr, PPA_NETIF *brif, PPA_NETIF *netif, uint32_t flags)
{
  struct bridging_session_list_item *p_item;
  int32_t ret = PPA_SESSION_NOT_ADDED; 
  uint16_t fid=0;
  struct netif_info *ifinfo = NULL;
  
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  uint32_t cur_time = 0;
  if(ppa_get_fid(ppa_get_netif_name(brif), &fid)!=PPA_SUCCESS) {
    ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in getting fid of bridge %s\n", ppa_get_netif_name(brif));
  }
#else
  if(flags & PPA_F_BRIDGE_ACCEL_MODE) { // update operation not needed for legacy platforms
    return PPA_SUCCESS;
  }
#endif

  if( !g_bridging_mac_learning ) return PPA_FAILURE;

  if( ppa_netif_lookup(ppa_get_netif_name(netif), &ifinfo) != PPA_SUCCESS )
    return ret;

  /* - Mahipati -
     Exception: Don't add eogre interfaces 
     IPoGRE and EoGRE does not terminate on same MAC !!
   */
  if( (!(flags & PPA_F_STATIC_ENTRY))  && ifinfo->flags & NETIF_GRE_TUNNEL ) {
    ppa_netif_put(ifinfo);
    //ppa_debug(DBG_ENABLE_MASK_ERR,"%s::Entry not added: %pM\n",__FUNCTION__,mac_addr);
    return ret;
  }

  ppa_br_get_htable_lock();
  if ( __ppa_bridging_lookup_session( mac_addr, 
                                      fid, 
                                      netif, 
                                      &p_item) == PPA_SESSION_EXISTS ) {

    p_item->netif = netif;
    
    if ( ifinfo->flags & NETIF_PHYS_PORT_GOT) {
      p_item->dest_ifid = ifinfo->phys_port;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
      /* For directconnect destif, get destination SubifId={VapId, StationId} */
      if ( (ifinfo->flags & NETIF_DIRECTCONNECT) )
      {
        dp_subif_t dp_subif = {0};

        dp_subif.port_id = ifinfo->phys_port;
        dp_subif.subif = ifinfo->subif_id;

        if ((dp_get_netif_subifid(p_item->netif, NULL, NULL, mac_addr, &dp_subif, 0))==PPA_SUCCESS)
        {
          p_item->sub_ifid = dp_subif.subif;
        } else {
          //ret = PPA_SESSION_NOT_ADDED;
          goto __BR_SESSION_ADD_DONE;
        }
      } else
#endif
        p_item->sub_ifid = ifinfo->subif_id;
#endif
    }

    if( (p_item->flags & SESSION_ADDED_IN_HW) ) {  //  added in hardware/firmware
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      if(flags & PPA_F_STATIC_ENTRY) {
        p_item->ref_count++;
        if(!(p_item->flags & SESSION_STATIC)) {
          p_item->flags |= SESSION_STATIC;
          p_item->timeout   = ~0; //  max timeout
          goto __UPDATE_HW_SESSION;
        }
    } else if(p_item->flags & SESSION_LAN_ENTRY) { // dynamic bridge entry getting updated
	cur_time = ppa_get_time_in_sec(); 
	if( cur_time - p_item->last_hit_time > 60) { //1 hw update per minute
	    p_item->last_hit_time = cur_time;
	    goto __UPDATE_HW_SESSION;
 	}
    } else {
      p_item->ref_count++;
      p_item->flags |= SESSION_LAN_ENTRY; // dynamic entry learned by bridge learning 
    }
#endif
      ret = PPA_SESSION_ADDED;
      goto __BR_SESSION_ADD_DONE;
    }
  }
  else if ( ppa_bridging_add_session(mac_addr, fid, netif, &p_item, flags) != 0 ){
      //ret = PPA_SESSION_NOT_ADDED;
      goto __BR_SESSION_ADD_DONE;
  }

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
__UPDATE_HW_SESSION:
#endif
  if ( ppa_bridging_hw_add_session(p_item) != PPA_SUCCESS )
  {
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_bridging_hw_add_session(%02x:%02x:%02x:%02x:%02x:%02x): fail\n", (uint32_t)p_item->mac[0], (uint32_t)p_item->mac[1], (uint32_t)p_item->mac[2], (uint32_t)p_item->mac[3], (uint32_t)p_item->mac[4], (uint32_t)p_item->mac[5]);
    //ret =  PPA_SESSION_NOT_ADDED;
    goto __BR_SESSION_ADD_DONE;
  }
  ret =  PPA_SESSION_ADDED;

  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"hardware added\n");

__BR_SESSION_ADD_DONE:
  ppa_netif_put(ifinfo);
  ppa_br_release_htable_lock();
  return ret;
}

int32_t ppa_hook_bridge_enable(uint32_t f_enable, uint32_t flags)
{
    g_bridging_mac_learning = f_enable;
    return PPA_SUCCESS;
}

int32_t ppa_hook_get_bridge_status(uint32_t *f_enable, uint32_t flags)
{
    if( f_enable )
        *f_enable = g_bridging_mac_learning;
    return PPA_SUCCESS;
}

int32_t ppa_bridge_entry_delete(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t flags)
{
    struct bridging_session_list_item *p_item;
    int32_t ret = PPA_FAILURE;
    uint16_t fid=0;
    
    if( !g_bridging_mac_learning ) return PPA_FAILURE;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    if(ppa_get_fid(ppa_get_netif_name(brif), &fid)!=PPA_SUCCESS) {
            ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in getting fid of bridge %s\n", ppa_get_netif_name(brif));
    }
#endif

    ppa_br_get_htable_lock();
    if ( __ppa_bridging_lookup_session(mac_addr, fid, NULL, &p_item) != PPA_SESSION_EXISTS ){
        goto __BR_SESSION_DELETE_DONE;
    }

    dump_bridging_list_item(p_item, "ppa_bridge_entry_delete");
    
    if(!(p_item->flags & SESSION_STATIC)) { // bridge learned entry only delete 
#ifdef CONFIG_LTQ_PPA_GRX500
    ppa_bridging_hw_del_session(p_item);
#endif

    //  ppa_bridging_remove_session->ppa_bridging_free_session_list_item->ppa_bridging_hw_del_session will delete MAC entry from Firmware/Hardware
    ppa_bridging_remove_session(p_item);

#ifdef CONFIG_LTQ_PPA_GRX500
    } else {				
	if(p_item->ref_count==1) { 
	    ppa_bridging_hw_del_session(p_item);
	    ppa_bridging_remove_session(p_item);
	} else {
	    p_item->ref_count--;
	    if(flags & PPA_F_STATIC_ENTRY) { // routed mac learned entry delete
		if(p_item->ref_count==1 && (p_item->flags & SESSION_LAN_ENTRY)) {
		    p_item->flags &= ~SESSION_STATIC;
		    p_item->timeout = DEFAULT_BRIDGING_TIMEOUT_IN_SEC;
		    ppa_bridging_hw_add_session(p_item); // modify the session to dynamic
		}
	    } else { 
		p_item->flags &= ~SESSION_LAN_ENTRY;
	    }
	} 
#endif
    }
    ret = PPA_SUCCESS;

__BR_SESSION_DELETE_DONE:
    ppa_br_release_htable_lock();

    return ret;
}

int32_t ppa_bridge_entry_hit_time(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t *p_hit_time)
{
    struct bridging_session_list_item *p_item;
    int32_t ret = PPA_SESSION_NOT_ADDED;
    uint16_t fid=0;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    PPE_BR_MAC_INFO br_mac={0};
#endif
    
    if( !g_bridging_mac_learning ) return PPA_SUCCESS;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    if(ppa_get_fid(ppa_get_netif_name(brif), &fid)!=PPA_SUCCESS) {
            ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in getting fid of bridge %s\n", ppa_get_netif_name(brif));
    }
#endif
   
    ppa_br_get_htable_lock();
    if ( __ppa_bridging_lookup_session(mac_addr, fid, NULL, &p_item) == PPA_SESSION_EXISTS ){
 
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
//  If the hardware has the MAC entry then the bridge entry should not timeout
//  If the mac entry is aged out by the hardware then bridge entry can be removed. 
    	ppa_memcpy(br_mac.mac, p_item->mac, PPA_ETH_ALEN);
    	br_mac.fid = p_item->fid;
    	ppa_drv_test_and_clear_bridging_hit_stat( &br_mac, 0);
	if(br_mac.f_hit) {
	    p_item->last_hit_time = ppa_get_time_in_sec() - ( p_item->timeout -  br_mac.age_timer);
	    *p_hit_time = p_item->last_hit_time;
	    ret = PPA_HIT;
	}
#else
       *p_hit_time = p_item->last_hit_time;
        ret = PPA_HIT;
#endif
    }

    ppa_br_release_htable_lock();
    return ret;
}

int32_t ppa_bridge_entry_inactivity_status(uint8_t *mac_addr, PPA_NETIF *brif)
{
    struct bridging_session_list_item *p_item;
    int32_t ret = PPA_HIT;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    PPE_BR_MAC_INFO br_mac={0};
#endif
    uint16_t fid=0;
    
    if( !g_bridging_mac_learning ) return PPA_SUCCESS;
 #if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    if(ppa_get_fid(ppa_get_netif_name(brif), &fid)!=PPA_SUCCESS) {
            ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in getting fid of bridge %s\n", ppa_get_netif_name(brif));
    }
#endif

   
    ppa_br_get_htable_lock();
    if ( __ppa_bridging_lookup_session(mac_addr, fid, NULL, &p_item) != PPA_SESSION_EXISTS ){
        ret = PPA_SESSION_NOT_ADDED;
        goto __BR_INACTIVITY_DONE;
    }

    //  not added in hardware
    if ( !(p_item->flags & SESSION_ADDED_IN_HW) ){
        ret = PPA_SESSION_NOT_ADDED;
        goto __BR_INACTIVITY_DONE;
    }

    if ( (p_item->flags & SESSION_STATIC) ){
        ret = PPA_HIT;
        goto __BR_INACTIVITY_DONE;
    }

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
//  If the hardware has the MAC entry then the bridge entry should not timeout
//  If the mac entry is aged out by the hardware then bridge entry can be removed. 
    ppa_memcpy(br_mac.mac, p_item->mac, PPA_ETH_ALEN);
    br_mac.fid = p_item->fid;
    ppa_drv_test_and_clear_bridging_hit_stat( &br_mac, 0);
    ret = br_mac.f_hit ? PPA_HIT : PPA_TIMEOUT;
    goto __BR_INACTIVITY_DONE;
#endif

    if ( p_item->timeout < ppa_get_time_in_sec() - p_item->last_hit_time ){  //  use < other than <= to avoid "false positives"
        ret = PPA_TIMEOUT;
        goto __BR_INACTIVITY_DONE;
    }

__BR_INACTIVITY_DONE:
    ppa_br_release_htable_lock();
    return ret;
}

int32_t ppa_set_bridge_entry_timeout(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t timeout)
{
    struct bridging_session_list_item *p_item;
    int32_t ret = PPA_SUCCESS;
    uint16_t fid=0;
    
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    if(ppa_get_fid(ppa_get_netif_name(brif), &fid)!=PPA_SUCCESS) {
            ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in getting fid of bridge %s\n", ppa_get_netif_name(brif));
    }
#endif
   
    ppa_br_get_htable_lock();
    if ( __ppa_bridging_lookup_session(mac_addr, fid,NULL, &p_item) != PPA_SESSION_EXISTS ){
        ret = PPA_FAILURE;
        goto __BR_TIMEOUT_DONE;
    }
    
    ppa_br_release_htable_lock();
    if ( !(p_item->flags & SESSION_STATIC) )
        p_item->timeout = timeout;

    ppa_bridging_set_polling_timer(timeout);

    return PPA_SUCCESS;

__BR_TIMEOUT_DONE:
    ppa_br_release_htable_lock();

    return PPA_SUCCESS;
}

/*
 *  PPA Bridge VLAN Config Functions
 */

int32_t ppa_set_bridge_if_vlan_config(PPA_NETIF *netif, PPA_VLAN_TAG_CTRL *vlan_tag_control, PPA_VLAN_CFG *vlan_cfg, uint32_t flags)
{
    struct netif_info *ifinfo;
    PPE_BRDG_VLAN_CFG cfg={0};

    if ( ppa_netif_update(netif, NULL) != PPA_SUCCESS )
        return PPA_FAILURE;

    if ( ppa_netif_lookup(ppa_get_netif_name(netif), &ifinfo) != PPA_SUCCESS )
        return PPA_FAILURE;

    if ( !(ifinfo->flags & NETIF_PHYS_PORT_GOT) )
    {
        ppa_netif_put(ifinfo);
        return PPA_FAILURE;
    }

    cfg.if_no = ifinfo->phys_port;
    cfg.f_eg_vlan_insert= vlan_tag_control->insertion | vlan_tag_control->replace;
    cfg.f_eg_vlan_remove= vlan_tag_control->remove | vlan_tag_control->replace;
    cfg.f_ig_vlan_aware = vlan_cfg->vlan_aware | (vlan_tag_control->unmodified ? 0 : 1) | vlan_tag_control->insertion | vlan_tag_control->remove | vlan_tag_control->replace |(vlan_tag_control->out_unmodified ? 0 : 1) | vlan_tag_control->out_insertion | vlan_tag_control->out_remove | vlan_tag_control->out_replace;
    cfg.f_ig_src_ip_based = vlan_cfg->src_ip_based_vlan,
    cfg.f_ig_eth_type_based= vlan_cfg->eth_type_based_vlan;
    cfg.f_ig_vlanid_based = vlan_cfg->vlanid_based_vlan;
    cfg.f_ig_port_based= vlan_cfg->port_based_vlan;
    cfg.f_eg_out_vlan_insert = vlan_tag_control->out_insertion | vlan_tag_control->out_replace;
    cfg.f_eg_out_vlan_remove = vlan_tag_control->out_remove | vlan_tag_control->out_replace;
    cfg.f_ig_out_vlan_aware = vlan_cfg->out_vlan_aware | (vlan_tag_control->out_unmodified ? 0 : 1) | vlan_tag_control->out_insertion | vlan_tag_control->out_remove | vlan_tag_control->out_replace;
    ppa_drv_set_bridge_if_vlan_config( &cfg, 0);

    ppa_netif_put(ifinfo);

    return PPA_SUCCESS;
}

int32_t ppa_get_bridge_if_vlan_config(PPA_NETIF *netif, PPA_VLAN_TAG_CTRL *vlan_tag_control, PPA_VLAN_CFG *vlan_cfg, uint32_t flags)
{
    struct netif_info *ifinfo;
    PPE_BRDG_VLAN_CFG cfg={0};

    if ( ppa_netif_update(netif, NULL) != PPA_SUCCESS )
        return PPA_FAILURE;

    if ( ppa_netif_lookup(ppa_get_netif_name(netif), &ifinfo) != PPA_SUCCESS )
        return PPA_FAILURE;

    if ( !(ifinfo->flags & NETIF_PHYS_PORT_GOT) )
    {
        ppa_netif_put(ifinfo);
        return PPA_FAILURE;
    }

    cfg.if_no = ifinfo->phys_port;
    ppa_drv_get_bridge_if_vlan_config( &cfg, 0);

    vlan_tag_control->unmodified    = cfg.f_ig_vlan_aware ? 0 : 1;
    vlan_tag_control->insertion     = cfg.f_eg_vlan_insert ? 1 : 0;
    vlan_tag_control->remove        = cfg.f_eg_vlan_remove ? 1 : 0;
    vlan_tag_control->replace       = cfg.f_eg_vlan_insert && cfg.f_eg_vlan_remove ? 1 : 0;
    vlan_cfg->vlan_aware            = cfg.f_ig_vlan_aware ? 1 : 0;
    vlan_cfg->src_ip_based_vlan     = cfg.f_ig_src_ip_based ? 1 : 0;
    vlan_cfg->eth_type_based_vlan   = cfg.f_ig_eth_type_based ? 1 : 0;
    vlan_cfg->vlanid_based_vlan     = cfg.f_ig_vlanid_based ? 1 : 0;
    vlan_cfg->port_based_vlan       = cfg.f_ig_port_based ? 1 : 0;
    vlan_tag_control->out_unmodified    = cfg.f_ig_out_vlan_aware ? 0 : 1;
    vlan_tag_control->out_insertion     = cfg.f_eg_out_vlan_insert ? 1 : 0;
    vlan_tag_control->out_remove        = cfg.f_eg_out_vlan_remove ? 1 : 0;
    vlan_tag_control->out_replace       = cfg.f_eg_out_vlan_insert && cfg.f_eg_out_vlan_remove ? 1 : 0;
    vlan_cfg->out_vlan_aware            = cfg.f_ig_out_vlan_aware ? 1 : 0;

    ppa_netif_put(ifinfo);

    return PPA_SUCCESS;
}

int32_t ppa_vlan_filter_add(PPA_VLAN_MATCH_FIELD *vlan_match_field, PPA_VLAN_INFO *vlan_info, uint32_t flags)
{
    struct netif_info *ifinfo;
    int i;
    PPE_BRDG_VLAN_FILTER_MAP vlan_filter={0};

    vlan_filter.out_vlan_info.vlan_id= vlan_info->out_vlan_id;
    if ( ppa_drv_add_outer_vlan_entry( &vlan_filter.out_vlan_info , 0) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"add out_vlan_ix error for out vlan id:%d\n", vlan_info->out_vlan_id);
        return PPA_FAILURE;
    }

    switch ( vlan_match_field->match_flags )
    {
    case PPA_F_VLAN_FILTER_IFNAME:
        if ( ppa_netif_update(NULL, vlan_match_field->match_field.ifname) != PPA_SUCCESS )
            return PPA_FAILURE;
        if ( ppa_netif_lookup(vlan_match_field->match_field.ifname, &ifinfo) != PPA_SUCCESS )
            return PPA_FAILURE;
        if ( !(ifinfo->flags & NETIF_PHYS_PORT_GOT) )
        {
            ppa_netif_put(ifinfo);
            return PPA_FAILURE;
        }
        vlan_filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_DEF;
        vlan_filter.ig_criteria         = ifinfo->phys_port;
        ppa_netif_put(ifinfo);
        break;
    case PPA_F_VLAN_FILTER_IP_SRC:
        vlan_filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_SRC_IP;
        vlan_filter.ig_criteria         = vlan_match_field->match_field.ip_src;
        break;
    case PPA_F_VLAN_FILTER_ETH_PROTO:
        vlan_filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_ETH_TYPE;
        vlan_filter.ig_criteria         = vlan_match_field->match_field.eth_protocol;
        break;
    case PPA_F_VLAN_FILTER_VLAN_TAG:
        vlan_filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_VLAN;
        vlan_filter.ig_criteria         = vlan_match_field->match_field.ingress_vlan_tag;
        break;
    default:
        return PPA_FAILURE;
    }

    vlan_filter.new_vci             = vlan_info->vlan_vci;

    vlan_filter.vlan_port_map = 0;
    for ( i = 0; i < vlan_info->num_ifs; i++ )
    {
        if ( ppa_netif_update(NULL, vlan_info->vlan_if_membership[i].ifname) != PPA_SUCCESS )
            continue;
        if ( ppa_netif_lookup(vlan_info->vlan_if_membership[i].ifname, &ifinfo) != PPA_SUCCESS )
            continue;
        if ( (ifinfo->flags & NETIF_PHYS_PORT_GOT) )
            vlan_filter.vlan_port_map |= 1 << ifinfo->phys_port;
        ppa_netif_put(ifinfo);
    }

    vlan_filter.dest_qos = vlan_info->qid;
    vlan_filter.in_out_etag_ctrl = vlan_info->inner_vlan_tag_ctrl | vlan_info->out_vlan_tag_ctrl;

    return ppa_drv_add_vlan_map( &vlan_filter, 0);
}

int32_t ppa_vlan_filter_del(PPA_VLAN_MATCH_FIELD *vlan_match_field, PPA_VLAN_INFO *vlan_info, uint32_t flags)
{
    PPE_BRDG_VLAN_FILTER_MAP filter={0};
    PPE_VFILTER_COUNT_CFG vfilter_count={0};
    struct netif_info *ifinfo;
    uint32_t i;
    uint8_t bfMatch=0;
    int32_t res;

    switch ( vlan_match_field->match_flags )
    {
    case PPA_F_VLAN_FILTER_IFNAME:
        if ( ppa_netif_update(NULL, vlan_match_field->match_field.ifname) != PPA_SUCCESS )
            return PPA_FAILURE;
        if ( ppa_netif_lookup(vlan_match_field->match_field.ifname, &ifinfo) != PPA_SUCCESS )
            return PPA_FAILURE;
        if ( !(ifinfo->flags & NETIF_PHYS_PORT_GOT) )
        {
            ppa_netif_put(ifinfo);
            return PPA_FAILURE;
        }
        filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_DEF;
        filter.ig_criteria         = ifinfo->phys_port;
        ppa_netif_put(ifinfo);
        break;
    case PPA_F_VLAN_FILTER_IP_SRC:
        filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_SRC_IP;
        filter.ig_criteria         = vlan_match_field->match_field.ip_src;
        break;
    case PPA_F_VLAN_FILTER_ETH_PROTO:
        filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_ETH_TYPE;
        filter.ig_criteria         = vlan_match_field->match_field.eth_protocol;
        break;
    case PPA_F_VLAN_FILTER_VLAN_TAG:
        filter.ig_criteria_type    = PPA_BRG_VLAN_IG_COND_TYPE_VLAN;
        filter.ig_criteria         = vlan_match_field->match_field.ingress_vlan_tag;
        break;
    default:
        return PPA_FAILURE;
    }

    //check whether there is such kind of vlan filter to delete
    vfilter_count.vfitler_type = filter.ig_criteria_type;
    ppa_drv_get_max_vfilter_entries( &vfilter_count, 0 );
    for ( i = 0; i < vfilter_count.num; i++ )
    {
        filter.entry = i;
        if ( (res = ppa_drv_get_vlan_map(&filter , 0) ) != -1 ) //get fail. break;
        {
            bfMatch = 1;
            break;
        }
    }

    if ( !bfMatch )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_vlan_filter_del: canot find such kinds of vlan filter\n");
        return PPA_FAILURE;
    }
    if ( res == 0 ) //blank item
        return PPA_SUCCESS;

    ppa_drv_del_vlan_map( &filter, 0 );

    ppa_drv_del_outer_vlan_entry( &filter.out_vlan_info, 0);

    return PPA_SUCCESS;
}

int32_t ppa_vlan_filter_get_all(int32_t *num_filters, PPA_VLAN_FILTER_CONFIG *vlan_filters, uint32_t flags)
{
//#warning ppa_vlan_filter_get_all is not implemented, too many memory allocation problem
    return PPA_ENOTIMPL;
}

int32_t ppa_vlan_filter_del_all(uint32_t flags)
{
    int32_t i, j;
    uint32_t vlan_filter_type[]={PPA_F_VLAN_FILTER_IFNAME, PPA_F_VLAN_FILTER_IP_SRC, PPA_F_VLAN_FILTER_ETH_PROTO, PPA_F_VLAN_FILTER_VLAN_TAG};
    PPE_BRDG_VLAN_FILTER_MAP filter={0};
    PPE_VFILTER_COUNT_CFG vfilter_count={0};

    for ( i = 0; i < NUM_ENTITY(vlan_filter_type); i++ )
    {
        vfilter_count.vfitler_type = vlan_filter_type[i];
        ppa_drv_get_max_vfilter_entries( &vfilter_count, 0);

        for ( j = 0; j < vfilter_count.num; j++ )
        {
            filter.ig_criteria_type = vlan_filter_type[i];
            filter.entry = j;
            if( ppa_drv_get_vlan_map( &filter, 0)  == 1 )
            {
                ppa_drv_del_outer_vlan_entry( &filter.out_vlan_info, 0);
            }
        }
    }

    ppa_drv_del_all_vlan_map( 0);
    return PPA_SUCCESS;
}

/*
 *  PPA MIB Counters Operation Functions
 */

int32_t ppa_get_if_stats(PPA_IFNAME *ifname, PPA_IF_STATS *p_stats, uint32_t flags)
{
    struct netif_info *p_info;
    uint32_t port_flags;
    PPE_ITF_MIB_INFO itf_mib={0};

    if ( !ifname || !p_stats )
        return PPA_EINVAL;

    if ( ppa_netif_lookup(ifname, &p_info) != PPA_SUCCESS )
        return PPA_EIO;
    itf_mib.itf= p_info->phys_port;
    port_flags = p_info->flags;
    ppa_netif_put(p_info);

    if ( !(port_flags & NETIF_PHYS_PORT_GOT) )
        return PPA_EIO;

    ppa_drv_get_itf_mib(&itf_mib, 0);

    p_stats->rx_pkts            = itf_mib.mib.ig_cpu_pkts;
    p_stats->tx_discard_pkts    = itf_mib.mib.ig_drop_pkts;
    p_stats->rx_bytes           = itf_mib.mib.ig_cpu_bytes;

    return PPA_SUCCESS;
}

int32_t ppa_get_accel_stats(PPA_IFNAME *ifname, PPA_ACCEL_STATS *p_stats, uint32_t flags)
{
    struct netif_info *p_info;
    uint32_t port;
    uint32_t port_flags;
    PPE_ITF_MIB_INFO mib = {0};

    if ( !ifname || !p_stats )
        return PPA_EINVAL;

    if ( ppa_netif_lookup(ifname, &p_info) != PPA_SUCCESS )
        return PPA_EIO;
    port = p_info->phys_port;
    port_flags = p_info->flags;
    ppa_netif_put(p_info);

    if ( !(port_flags & NETIF_PHYS_PORT_GOT) )
        return PPA_EIO;

    mib.itf = p_info->phys_port;
    mib.flag = flags;
    ppa_drv_get_itf_mib(&mib, 0);

    p_stats->fast_routed_tcp_pkts       = mib.mib.ig_fast_rt_ipv4_tcp_pkts + mib.mib.ig_fast_rt_ipv6_tcp_pkts;
    p_stats->fast_routed_udp_pkts       = mib.mib.ig_fast_rt_ipv4_udp_pkts + mib.mib.ig_fast_rt_ipv6_udp_pkts;
    p_stats->fast_routed_udp_mcast_pkts = mib.mib.ig_fast_rt_ipv4_mc_pkts;
    p_stats->fast_drop_pkts             = mib.mib.ig_drop_pkts;
    p_stats->fast_drop_bytes            = mib.mib.ig_drop_bytes;
    p_stats->fast_ingress_cpu_pkts      = mib.mib.ig_cpu_pkts;
    p_stats->fast_ingress_cpu_bytes     = mib.mib.ig_cpu_bytes;
    p_stats->fast_bridged_ucast_pkts    = mib.mib.ig_fast_brg_pkts;
    p_stats->fast_bridged_bytes         = mib.mib.ig_fast_brg_bytes;

    return PPA_SUCCESS;
}

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
int32_t ppa_get_netif_accel_stats(PPA_IFNAME *ifname, PPA_NETIF_ACCEL_STATS *p_stats, uint32_t flags)
{
    struct netif_info *p_info;
    uint32_t port;
    uint32_t port_flags;
    PPE_ITF_MIB_INFO mib = {0};

    if ( !ifname || !p_stats )
        return PPA_EINVAL;

    if ( ppa_netif_lookup(ifname, &p_info) != PPA_SUCCESS )
        return PPA_EIO;

    if ( (flags & PPA_F_NETIF_PORT_MIB) ) {
        port = p_info->phys_port;
        port_flags = p_info->flags;

        if ( (port_flags & NETIF_PHYS_PORT_GOT) ) {
            mib.itf = p_info->phys_port;
            ppa_drv_get_itf_mib(&mib, 0);

            p_stats->port_mib_stats.ig_fast_brg_pkts = mib.mib.ig_fast_brg_pkts;
            p_stats->port_mib_stats.ig_fast_brg_bytes = mib.mib.ig_fast_brg_bytes;

            p_stats->port_mib_stats.ig_fast_rt_ipv4_udp_pkts = mib.mib.ig_fast_rt_ipv4_udp_pkts;
            p_stats->port_mib_stats.ig_fast_rt_ipv4_tcp_pkts = mib.mib.ig_fast_rt_ipv4_tcp_pkts;
            p_stats->port_mib_stats.ig_fast_rt_ipv4_mc_pkts = mib.mib.ig_fast_rt_ipv4_mc_pkts;
            p_stats->port_mib_stats.ig_fast_rt_ipv4_bytes = mib.mib.ig_fast_rt_ipv4_bytes;

            p_stats->port_mib_stats.ig_fast_rt_ipv6_udp_pkts = mib.mib.ig_fast_rt_ipv6_udp_pkts;
            p_stats->port_mib_stats.ig_fast_rt_ipv6_tcp_pkts = mib.mib.ig_fast_rt_ipv6_tcp_pkts;
            p_stats->port_mib_stats.ig_fast_rt_ipv6_bytes = mib.mib.ig_fast_rt_ipv6_bytes;

            p_stats->port_mib_stats.ig_cpu_pkts = mib.mib.ig_cpu_pkts;
            p_stats->port_mib_stats.ig_cpu_bytes = mib.mib.ig_cpu_bytes;

            p_stats->port_mib_stats.eg_fast_pkts = mib.mib.eg_fast_pkts;
        }
    }

    if ( (flags & PPA_F_NETIF_HW_ACCEL) ) {
        p_stats->hw_accel_stats.rx_bytes = p_info->hw_accel_stats.rx_bytes;
        p_stats->hw_accel_stats.tx_bytes = p_info->hw_accel_stats.tx_bytes;
    }

    if ( (flags & PPA_F_NETIF_SW_ACCEL) ) {
        p_stats->sw_accel_stats.rx_bytes = p_info->sw_accel_stats.rx_bytes;
        p_stats->sw_accel_stats.tx_bytes = p_info->sw_accel_stats.tx_bytes;
    }

    ppa_netif_put(p_info);

    return PPA_SUCCESS;
}
#endif

/*
 *  PPA Network Interface Operation Functions
 */

int32_t ppa_set_if_mac_address(PPA_IFNAME *ifname, uint8_t *mac_addr, uint32_t flags)
{
    struct netif_info *ifinfo;
    PPE_ROUTE_MAC_INFO mac_info={0};

    if ( !ifname || !mac_addr )
        return PPA_EINVAL;

    if ( ppa_netif_lookup(ifname, &ifinfo) != PPA_SUCCESS )
        return PPA_FAILURE;

    if ( (ifinfo->flags & NETIF_MAC_ENTRY_CREATED) )
    {
        mac_info.mac_ix= ifinfo->mac_entry;
        ppa_drv_del_mac_entry(&mac_info, 0);
        ifinfo->mac_entry = ~0;
        ifinfo->flags &= ~NETIF_MAC_ENTRY_CREATED;
    }

    ppa_memcpy(ifinfo->mac, mac_addr, PPA_ETH_ALEN);
    ifinfo->flags |= NETIF_MAC_AVAILABLE;

    ppa_memcpy(mac_info.mac, mac_addr, sizeof(mac_info.mac));
    if ( ppa_drv_add_mac_entry( &mac_info, 0) == PPA_SUCCESS )
    {
        ifinfo->mac_entry = mac_info.mac_ix;
        ifinfo->flags |= NETIF_MAC_ENTRY_CREATED;
    }

    ppa_netif_put(ifinfo);

    return PPA_SUCCESS;
}

int32_t ppa_get_if_mac_address(PPA_IFNAME *ifname, uint8_t *mac_addr, uint32_t flags)
{
    int32_t ret;
    struct netif_info *ifinfo;

    if ( !ifname || !mac_addr )
        return PPA_EINVAL;

    if ( ppa_netif_lookup(ifname, &ifinfo) != PPA_SUCCESS )
        return PPA_FAILURE;

    if ( (ifinfo->flags & NETIF_MAC_AVAILABLE) )
    {
        ppa_memcpy(mac_addr,ifinfo->mac, PPA_ETH_ALEN);
        ret = PPA_SUCCESS;
    }
    else
        ret = PPA_FAILURE;

    ppa_netif_put(ifinfo);

    return ret;
}

#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
#if !defined(CONFIG_LTQ_PPA_GRX500) && !(CONFIG_PPA_PUMA7)

int32_t ppa_ethsw_get_pktrcv_port_gen(LTQ_ETHSW_API_HANDLE handle, void *pktdata)
{
	int ret, i, count = 0, retval = 0xFF;/*ETHSW_INVALID_PORT */
	IFX_ETHSW_MAC_tableRead_t MAC_tableRead;
	unsigned char *tmac = pktdata;


	for (i = 0; i < 7; i++) {
		memset(&MAC_tableRead, 0x00, sizeof(MAC_tableRead));
		MAC_tableRead.bInitial = 1;
		MAC_tableRead.nPortId = i;
		ret =
		    ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MAC_TABLE_ENTRY_READ,
			  (u32)&MAC_tableRead);
		if (ret != PPA_SUCCESS) {
#ifdef PPA_MCAST_DEBUG
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"IFX_ETHSW_MAC_TABLE_ENTRY_READ ioctl Error \n");
#endif
			return retval;
		}
		if (MAC_tableRead.bLast == 0) {
			if (tmac[0] == MAC_tableRead.nMAC[0]
			    && tmac[1] == MAC_tableRead.nMAC[1]
			    && tmac[2] == MAC_tableRead.nMAC[2]
			    && tmac[3] == MAC_tableRead.nMAC[3]
			    && tmac[4] == MAC_tableRead.nMAC[4]
			    && tmac[5] == MAC_tableRead.nMAC[5]) {
//				ltq_ethsw_api_kclose(handle);
				return MAC_tableRead.nPortId;
			}

			count++;
			do {
				memset(&MAC_tableRead, 0x00,
				       sizeof(MAC_tableRead));
				MAC_tableRead.bInitial = 0;
				MAC_tableRead.nPortId = i;

				ret =
				    ltq_ethsw_api_kioctl(handle,
					  IFX_ETHSW_MAC_TABLE_ENTRY_READ,
					  (u32)&MAC_tableRead);
				if (ret != PPA_SUCCESS) {
#ifdef PPA_MCAST_DEBUG
					ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"IFX_ETHSW_MAC_TABLE_ENTRY_READ ioctl Error \n");
#endif
					return retval;
				}

				if (MAC_tableRead.bLast == 0) {
					if (tmac[0] == MAC_tableRead.nMAC[0]
					    && tmac[1] == MAC_tableRead.nMAC[1]
					    && tmac[2] == MAC_tableRead.nMAC[2]
					    && tmac[3] == MAC_tableRead.nMAC[3]
					    && tmac[4] == MAC_tableRead.nMAC[4]
					    && tmac[5] ==
					    MAC_tableRead.nMAC[5]) {
						return MAC_tableRead.nPortId;
					}
					count++;
				}
			} while (MAC_tableRead.bLast == 0);
		}
	}
	return ETHSW_INVALID_PORT;
}

int32_t ppa_set_snoop_switch_config(mcast_stream_t *mc_stream)
{
	LTQ_ETHSW_API_HANDLE handle;
	int ret = 0;

	handle = ltq_ethsw_api_kopen("/dev/switch_api/0");

		if ((mc_stream->src_ip.ipType == 0)|| (mc_stream->src_ip.ipType == 1)) {
		IFX_ETHSW_multicastSnoopCfg_t multicast_SnoopCfgSet;
//		common_config_t *cfg = COMMON_CONFIG_P(priv->type);
		memset(&multicast_SnoopCfgSet, 0,
		       sizeof(IFX_ETHSW_multicastSnoopCfg_t));

			/* Set CPU port configuration */
			multicast_SnoopCfgSet.bIGMPv3 = 1;
			multicast_SnoopCfgSet.eIGMP_Mode =
			    IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD;
			multicast_SnoopCfgSet.nRobust = 2;
			multicast_SnoopCfgSet.nQueryInterval = 125;
			multicast_SnoopCfgSet.eSuppressionAggregation = 2;
			multicast_SnoopCfgSet.bFastLeave = 0;
			multicast_SnoopCfgSet.eForwardPort = 3;
			multicast_SnoopCfgSet.nForwardPortId = 6;

		ret =
		    ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MULTICAST_SNOOP_CFG_SET,
			  (u32)&multicast_SnoopCfgSet);
#ifdef PPA_MCAST_DEBUG
		if (ret != 0) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"set_snoop_switch_config ioctl failed \n");
		}
#endif
	}
	ltq_ethsw_api_kclose(handle);

	return ret;
}

int32_t ppa_add_switch_config(mcast_stream_t *mc_stream, struct net_device *member, uint8_t portid)
{
	LTQ_ETHSW_API_HANDLE handle;
	IFX_ETHSW_multicastTable_t multicast_TableEntryAdd;
	uint8_t sw_portid;
	int32_t ret = PPA_FAILURE;

	
	memset(&multicast_TableEntryAdd, 0, sizeof(IFX_ETHSW_multicastTable_t));
	handle = ltq_ethsw_api_kopen("/dev/switch_api/0");
	
	if(mc_stream->src_ip.ipType == 0)
	{
		multicast_TableEntryAdd.eIPVersion = IFX_ETHSW_IP_SELECT_IPV4;
		multicast_TableEntryAdd.uIP_Gsa.nIPv4 = mc_stream->src_ip.ipA.ipAddr.s_addr; 
	}
	else
	{
		 multicast_TableEntryAdd.eIPVersion = IFX_ETHSW_IP_SELECT_IPV6;
		 memcpy(&(multicast_TableEntryAdd.uIP_Gsa.nIPv6[0]),&(mc_stream->src_ip.ipA.ip6Addr.s6_addr[0]), sizeof(struct in6_addr));
	}
	if(mc_stream->dst_ip.ipType == 0)
	{
		multicast_TableEntryAdd.eIPVersion = IFX_ETHSW_IP_SELECT_IPV4;
		multicast_TableEntryAdd.uIP_Gda.nIPv4 = mc_stream->dst_ip.ipA.ipAddr.s_addr;
	}
	else
	{
		multicast_TableEntryAdd.eIPVersion = IFX_ETHSW_IP_SELECT_IPV6;
		memcpy(&(multicast_TableEntryAdd.uIP_Gda.nIPv6[0]),&(mc_stream->dst_ip.ipA.ip6Addr.s6_addr[0]), sizeof(struct in6_addr));
	}
	if(portid != 6)
	{
			sw_portid = ppa_ethsw_get_pktrcv_port_gen(handle,mc_stream->macaddr);

			if((sw_portid == ETHSW_INVALID_PORT) || (sw_portid >= 6))
				multicast_TableEntryAdd.nPortId	= ETHSW_INVALID_PORT;
			else
				multicast_TableEntryAdd.nPortId	= sw_portid;
	
	}
	else
		multicast_TableEntryAdd.nPortId = 6;

	if(multicast_TableEntryAdd.nPortId != ETHSW_INVALID_PORT ) { 
        	ret =  ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD,(u32)&multicast_TableEntryAdd);
#ifdef PPA_MCAST_DEBUG
		if (ret != PPA_SUCCESS) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD ioctl Error \n");
		}
#endif
	}
	ltq_ethsw_api_kclose(handle);

	return ret;

}

int32_t ppa_del_switch_config(mcast_stream_t *mc_stream, PPA_NETIF *member, uint8_t portid,uint8_t filter_mode)
{
	LTQ_ETHSW_API_HANDLE handle;
	IFX_ETHSW_multicastTable_t multicast_TableEntryRemove;
	int ret = PPA_FAILURE;
	uint8_t sw_portid;
	handle = ltq_ethsw_api_kopen("/dev/switch_api/0");

	memset(&multicast_TableEntryRemove, 0,
	       sizeof(IFX_ETHSW_multicastTable_t));

	if(portid != 6)
	{
			sw_portid = ppa_ethsw_get_pktrcv_port_gen(handle,mc_stream->macaddr);
			if((sw_portid == ETHSW_INVALID_PORT) || (sw_portid >= 6))
				multicast_TableEntryRemove.nPortId	= ETHSW_INVALID_PORT;
			else
				multicast_TableEntryRemove.nPortId	= sw_portid;

	}
	else
		multicast_TableEntryRemove.nPortId = 6;
	
	if(mc_stream->src_ip.ipType == 0)
	{
		multicast_TableEntryRemove.eIPVersion = IFX_ETHSW_IP_SELECT_IPV4;
		multicast_TableEntryRemove.uIP_Gsa.nIPv4 = mc_stream->src_ip.ipA.ipAddr.s_addr; 
	}
	else
	{
		 multicast_TableEntryRemove.eIPVersion = IFX_ETHSW_IP_SELECT_IPV6;
		 memcpy(&(multicast_TableEntryRemove.uIP_Gsa.nIPv6[0]),&(mc_stream->src_ip.ipA.ip6Addr.s6_addr[0]), sizeof(struct in6_addr));
	}
	if(mc_stream->dst_ip.ipType == 0)
	{
		multicast_TableEntryRemove.eIPVersion = IFX_ETHSW_IP_SELECT_IPV4;
		multicast_TableEntryRemove.uIP_Gda.nIPv4 = mc_stream->dst_ip.ipA.ipAddr.s_addr;
	}
	else
	{
		multicast_TableEntryRemove.eIPVersion = IFX_ETHSW_IP_SELECT_IPV6;
		memcpy(&(multicast_TableEntryRemove.uIP_Gda.nIPv6[0]),&(mc_stream->dst_ip.ipA.ip6Addr.s6_addr[0]), sizeof(struct in6_addr));
	}

	multicast_TableEntryRemove.eModeMember = filter_mode;

	ret =
	    ltq_ethsw_api_kioctl(handle, IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE,
		  (u32)&multicast_TableEntryRemove);
#ifdef PPA_MCAST_DEBUG
	if (ret != PPA_SUCCESS) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE ioctl Error \n");
	}
#endif
	ltq_ethsw_api_kclose(handle);

	return ret;
}
#endif /* CONFIG_LTQ_PPA_GRX500 */

/* Multicast callback function*/

int32_t ppa_ip_compare( IP_ADDR ip1, IP_ADDR ip2, uint32_t flag )
{
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
    if( flag & SESSION_IS_IPV6 )
    {
        return ppa_memcmp(ip1.ip6, ip2.ip6, sizeof(ip1.ip6) );
    }
    else
#endif
    {
         return ppa_memcmp(&ip1.ip, &ip2.ip, sizeof(ip1.ip) );
    }
}

/* Multicast callback function*/

int32_t mcast_module_config(uint32_t grp_idx, struct net_device *member, mcast_stream_t *mc_stream, uint32_t flags)
{
	/* call ppacmd addmc group */
	/* grp_idx for future purpose */
	PPA_MC_GROUP mc_group;
	struct mc_group_list_item *pp_item;
    	struct netif_info *rx_ifinfo=NULL; 
	uint32_t pos = 0,itf_num=0;
        int32_t count=0, i, mc_group_count,flag=0,res=0,j;
	uint8_t idx,mcast_match=0;
#ifndef CONFIG_LTQ_PPA_GRX500
	uint8_t filter_mode; 
	uint32_t portid;
#endif
	int ret = PPA_FAILURE;
	char *mem_name,*pp_memname = NULL;
        ppa_memset( &mc_group, 0, sizeof(mc_group));

/* Fix for the multicast acceleration on ADSL ATM wan mode on VRX318 */
        flag |= PPA_F_SESSION_VLAN;
        flag |= PPA_F_SESSION_OUT_VLAN ;
        mc_group.out_vlan_remove = 1;

/* FILL ALL GIVEN INFO */
	
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Received Mcast ADD/DEL Trigger \n");
	if(member != NULL)
	{
	mem_name = ppa_get_netif_name(member);
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Member name is %s \n",mem_name);
	}
	else
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Member is NULL \n");
		goto EXIT_EOI;
	}
	if(mc_stream == NULL)
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"mc_stream is NULL \n");
		goto EXIT_EOI;
	}

	/* Get total number of Mcast Entries */
	mc_group_count = ppa_get_mc_group_count(flag);
	/* Set SSM flag for Source specific forwarding: If not set source ip will be ignored while DEL MC operation*/
	mc_group.SSM_flag = 1;

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500 
    	/* Get Mcast Group Idx */	
    	mc_group.group_id = grp_idx;
	ppa_memcpy( mc_group.src_mac, mc_stream->src_mac, PPA_ETH_ALEN); 
#endif

	/* Get Mcast Group IP */	
	if(mc_stream->dst_ip.ipType == 0)
	{
		mc_group.ip_mc_group.ip.ip = mc_stream->dst_ip.ipA.ipAddr.s_addr;
		mc_group.ip_mc_group.f_ipv6 = 0;
	}
	else
	{
		ppa_memcpy( &(mc_group.ip_mc_group.ip.ip6[0]), &(mc_stream->dst_ip.ipA.ip6Addr.s6_addr[0]), sizeof(struct in6_addr) ) ;
		mc_group.ip_mc_group.f_ipv6 = 1;
	}
	/* Fill rx device name */	
	mc_group.src_ifname = mc_stream->rx_dev->name;

	/* Get Mcast Source IP */
	if(mc_stream->src_ip.ipType == 0)
	{
		mc_group.source_ip.ip.ip = mc_stream->src_ip.ipA.ipAddr.s_addr;
		mc_group.source_ip.f_ipv6 = 0;
	}
	else
	{
		ppa_memcpy(&(mc_group.source_ip.ip.ip6[0]),&(mc_stream->src_ip.ipA.ip6Addr.s6_addr[0]), sizeof(struct in6_addr));
		mc_group.source_ip.f_ipv6 = 1;
	}
	/* If the member added is bridged, set bridging flag */
	if ( ppa_if_is_br_if(mc_stream->rx_dev, NULL) )
	{
		mc_group.bridging_flag = 1;
	}
/* DONE FILLING ALL GIVEN INFO */

#if 1	
	if ( ppa_mc_group_start_iteration(&pos, &pp_item) == PPA_SUCCESS )
	{
		/* itf_num Gives number of interfaces signed in for a given Mcast Group */
		itf_num = pp_item->num_ifs;
		/* Count will give the total number of entries in the Hash table */
		count = 0;
		do
		{
			/* Check if received gaddr & source ip matches the list, and extract the ifname in that gaddr and fill the mc_group data structure*/
			if (((pp_item->ip_mc_group.ip.ip == mc_group.ip_mc_group.ip.ip) && (pp_item->source_ip.ip.ip == mc_group.source_ip.ip.ip)) || 
								((!ppa_ip_compare((pp_item->ip_mc_group.ip),(mc_group.ip_mc_group.ip),SESSION_IS_IPV6)) && 
											(!ppa_ip_compare((pp_item->source_ip.ip),(mc_group.source_ip.ip),SESSION_IS_IPV6))))
			{
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Grp Ip & Src Ip Match found \n");
				/* itf_num is Number of interface signed in for the matched Group address */
				mcast_match = 1;
				itf_num = pp_item->num_ifs;
				if(flags & MC_F_DEL)
				{
					for(i=0, idx=0, j=0; i < itf_num ; i++)
					{
						if( pp_item->netif[i] )
						{
							pp_memname = ppa_get_netif_name(pp_item->netif[i]);
							if( ppa_is_netif_name(pp_item->netif[i],member->name ) != 1 )
							{
								mc_group.array_mem_ifs[j].ifname = pp_memname;
								j++;
							}
						}
						mc_group.if_mask |= 1 << idx;
						idx++;
					}
					ppa_mc_group_stop_iteration();
					goto PPA_DEL_MC; /* When DEL flag is set : Check required to find if this is the last itf in the gaddr and delete CPU switch rule for that gaddr */
				}
				for(i=0, idx=0; i < itf_num ; i++)
				{
					if( pp_item->netif[i] )
					{
						pp_memname = ppa_get_netif_name(pp_item->netif[i]);
						mc_group.array_mem_ifs[i].ifname = pp_memname;
					}
					mc_group.if_mask |= 1 << idx;
					idx++;
				}
				mc_group.array_mem_ifs[itf_num].ifname = member->name;
				itf_num++;
				idx++;
				mc_group.if_mask |= 1 << idx;
				break;
			}
			count++;
		}while ( (ppa_mc_group_iterate_next(&pos, &pp_item) == PPA_SUCCESS ) && (count < mc_group_count) );
	}
	ppa_mc_group_stop_iteration();
#endif	
	mc_group.num_ifs = itf_num; /* No of interfaces in the list + 1 */
	if(mcast_match == 0 && (flags & MC_F_ADD))
	{
		itf_num = 1;
		mc_group.num_ifs = itf_num; /* No of interfaces in the list + 1 */
		mc_group.array_mem_ifs[0].ifname = member->name;
	}
	else
	mcast_match = 0;
	
	res = ppa_mc_group_update( &mc_group, flag);
	
PPA_DEL_MC:
	if(flags & MC_F_DEL)
	{
		/* Call Del Mc only when there is atleast one entry that matches the input grp_addr else need to exit*/
		if(mcast_match != 0)
		{
			mc_group.num_ifs = j;/* Fill number of remaining interfaces that are filled in array_mem_ifs[].ifname. If this is zero del_mc is called else update_mc is called */
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," Invoking ppa_mc_group_update with MC_F_DEL \n");
			if ( ppa_netif_lookup(mc_group.src_ifname, &rx_ifinfo) != PPA_SUCCESS )
		        {
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," Netif lookup for %s Failed : Deleting all Entries\n",mc_group.src_ifname);
				mc_group.num_ifs = 0;
			}
			else
			ppa_netif_put(rx_ifinfo);

			res = ppa_mc_group_update( &mc_group, flag);
		}
		else
			goto EXIT_EOI;
	}		

	if ( res != PPA_SUCCESS )
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_mc_group_update fail\n");
		goto EXIT_EOI;
	}
	else
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_mc_group_update Success\n");
		res = 0;
	}

	/* Call switch ioclt for multicast configuration */
#if !defined(CONFIG_LTQ_PPA_GRX500) && !(CONFIG_PPA_PUMA7)
	if(g_snoop_set != 0)
	{
		ppa_set_snoop_switch_config(mc_stream);/* Set snoop config in switch only once based on global variable g_snoop_set */
		g_snoop_set = 0;/* Reset global snoop variable */
	}
	/* Setting Router port Config is done in Mcast script, as it can vary for platforms */
	if(flags & MC_F_ADD)
	{
		portid = 0;/* portid input is zero for any port except cpu port */
		ret = ppa_add_switch_config(mc_stream,member,portid);/* Do switch config for rest ports*/
		if(ret == PPA_SUCCESS){
			ppa_add_switch_config(mc_stream,member,6); /* Do switch config for CPU port */
		}
	}
	else
	{
		filter_mode = 0;
		portid = 0;
		ppa_del_switch_config(mc_stream,member,portid,filter_mode);/* Del switch config for other ports */
		if(mc_group.num_ifs == 0) /* Check if its the last itf in the gaddr */
			ppa_del_switch_config(mc_stream,member,6,filter_mode);/* Del switch config for cpu port */
	}
#endif

EXIT_EOI:

	return res;

}
#endif

#if defined(CONFIG_PPA_PUMA7) && defined(CONFIG_TI_HIL_PROFILE_INTRUSIVE_P7)
static void ppa_netdev_pp_prepare_vpid(PPA_IFINFO *ifinfo, uint32_t flags)
{
	struct netif_info *p_ifinfo;
	PPA_NETIF *netif, *lnetif;

	struct in_device *in_dev;
	struct in_ifaddr *ifap;

	uint16_t setup_flag = 0;
	uint32_t ip = 0;

	PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE];

	lnetif = netif = NULL;
	ifap = NULL;
	in_dev = NULL;
	underlying_ifname[0] = '\0';

	if (ppa_netif_lookup(ifinfo->ifname, &p_ifinfo) == PPA_SUCCESS) {
		if ((netif = ppa_get_netif(ifinfo->ifname)) != NULL) {
			in_dev = netif->ip_ptr;
			for (ifap = in_dev->ifa_list; ifap != NULL;	ifap = ifap->ifa_next) {
				if (!(strcmp(ifap->ifa_label, ifinfo->ifname))
									&& ifap->ifa_address > 0) {
// for NETIF_PPPOE, create VPID on actual interafce 
					if( p_ifinfo->flags & NETIF_PPPOE &&
						(ppa_pppoe_get_physical_if(netif, NULL, underlying_ifname) == PPA_SUCCESS) ) {
						if( ppa_netif_lookup(underlying_ifname, &p_ifinfo) != PPA_SUCCESS ||
							(netif = ppa_get_netif(p_ifinfo->name)) == NULL) {
							ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"PPP added to PPA without base interface\n");
							return;
						}
					}
					setup_flag = 1;
					break;	
				}
			}
			if ( p_ifinfo->flags & NETIF_PHY_TUNNEL)
				setup_flag = 0;
// check if this is a bonded slave interface
			if (ppa_is_bond_slave(NULL, netif))
				setup_flag = 1;
// check if interface is in bridge and bridge is assigned with ip
			if( ppa_is_netif_bridged(NULL, netif) && 
			  (lnetif = ppa_get_br_dev(netif)) != NULL) {
				if(ppa_get_netif_ip(&ip, lnetif) == PPA_SUCCESS)
					setup_flag = 1;
			}

			if ( setup_flag ) {
// for virtual interface copy pid_handle and do netdev cloning
				lnetif = ppa_get_netif(p_ifinfo->phys_netif_name);
				if(lnetif != NULL && netif != NULL) {
					netif->vpid_block.parent_pid_handle = lnetif->pid_handle;
					netif->pid_handle = lnetif->pid_handle;
					ti_hil_clone_netdev_pp_info(netif, lnetif);
				}

// for VLAN interface set vpid block type as AVALANCHE_PP_VPID_VLAN
				if( p_ifinfo->flags & NETIF_VLAN) {
					netif->vpid_block.type = AVALANCHE_PP_VPID_VLAN;
					if(p_ifinfo->manual_lower_ifname != NULL &&
						((lnetif = ppa_get_netif(p_ifinfo->manual_lower_ifname))!= NULL)) {
						netif->vpid_block.vlan_identifier = ppa_get_vlan_id(lnetif);
					} else {
						netif->vpid_block.vlan_identifier = ppa_get_vlan_id(netif);
					}
				} else {
					netif->vpid_block.type = AVALANCHE_PP_VPID_ETHERNET;
				}
				
				ti_hil_pp_event (TI_PP_ADD_VPID, netif);

			}
		}
	}
	return;
}
#endif

int32_t ppa_add_if(PPA_IFINFO *ifinfo, uint32_t flags)
{
    uint32_t ret;
#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
	uint32_t mcast_flag;
    PPA_NETIF *netif;
    char *addl_name = NULL;
#endif

    if ( !ifinfo )
        return PPA_EINVAL;

    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_add_if with force_wanitf_flag=%d in ppa_add_if\n", ifinfo->force_wanitf_flag );
    ret = ppa_netif_add(ifinfo->ifname, ifinfo->if_flags & PPA_F_LAN_IF, NULL, ifinfo->ifname_lower, ifinfo->force_wanitf_flag);

#if defined(CONFIG_PPA_PUMA7) && defined(CONFIG_TI_HIL_PROFILE_INTRUSIVE_P7)
	ppa_netdev_pp_prepare_vpid(ifinfo, flags);
#endif

#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
#ifndef CONFIG_PPA_PUMA7
    if ( ret == PPA_SUCCESS && (ifinfo->if_flags & PPA_F_LAN_IF))
    {
	netif = ppa_get_netif(ifinfo->ifname);
	mcast_flag = MC_F_REGISTER | MC_F_DIRECTPATH;
	mcast_helper_register_module(netif,THIS_MODULE,addl_name,(int32_t *)mcast_module_config,mcast_flag);	
    }
#endif
#endif
    return ret;
}

int32_t ppa_del_if(PPA_IFINFO *ifinfo, uint32_t flags)
{
    PPA_NETIF *netif;
    PPA_NETIF *lnetif;
	struct netif_info *p_ifinfo;
	PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE];
#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
    uint32_t mcast_flag;
    char *addl_name = NULL;
#endif
    netif = ppa_get_netif(ifinfo->ifname);
#if defined(CONFIG_PPA_PUMA7) && defined(CONFIG_TI_HIL_PROFILE_INTRUSIVE_P7)
	if(netif != NULL) {
		if (ppa_netif_lookup(ifinfo->ifname, &p_ifinfo) == PPA_SUCCESS) {
// for NETIF_PPPOE, delete VPID of actual interafce
			if( p_ifinfo->flags & NETIF_PPPOE &&
				(ppa_pppoe_get_physical_if(netif, NULL, underlying_ifname) == PPA_SUCCESS) ) {
				if( (lnetif = ppa_get_netif(underlying_ifname)) != NULL) {
					if ( lnetif->qos_shutdown_hook != NULL)
						lnetif->qos_shutdown_hook( lnetif );
					ti_hil_pp_event (TI_PP_REMOVE_VPID, lnetif);
				}
			} else {
				if ( netif->qos_shutdown_hook != NULL)
					netif->qos_shutdown_hook( netif );
				ti_hil_pp_event (TI_PP_REMOVE_VPID, netif);
			}
		}
	}
	/* If hw_disable flag is set then delete the ifname only from HW, SW acceleration for ifname will be still functional */
	if (ifinfo->hw_disable) {
		printk("ifinfo.hw_disable is set\n");
		return PPA_SUCCESS;
	}
#endif

    ppa_netif_remove(ifinfo->ifname, ifinfo->if_flags & PPA_F_LAN_IF);

    ppa_remove_sessions_on_netif(ifinfo->ifname);
#if defined(CONFIG_LANTIQ_MCAST_HELPER_MODULE) || defined(CONFIG_LANTIQ_MCAST_HELPER)
#ifndef CONFIG_PPA_PUMA7
    if(ifinfo->if_flags & PPA_F_LAN_IF)
    {
		mcast_flag = MC_F_DEREGISTER | MC_F_DIRECTPATH;
    	mcast_helper_register_module(netif,THIS_MODULE,addl_name,(int32_t *)mcast_module_config,mcast_flag);
    }	
#endif
#endif
    return PPA_SUCCESS;
}

int32_t ppa_get_if(int32_t *num_ifs, PPA_IFINFO **ifinfo, uint32_t flags)
{
    uint32_t pos = 0;
    struct netif_info *info;
    int32_t num = 0;
    PPA_IFINFO *p_ifinfo;

    if ( !num_ifs || !ifinfo )
        return PPA_EINVAL;

    p_ifinfo = (PPA_IFINFO *)ppa_malloc(100 * sizeof(PPA_IFINFO));  //  assume max 100 netif
    if(!p_ifinfo){
        return PPA_ENOMEM;
    }

    if ( ppa_netif_start_iteration(&pos, &info) != PPA_SUCCESS ){
        ppa_free(p_ifinfo);
        return PPA_FAILURE;
    }

    do
    {
        if ( (info->flags & NETIF_LAN_IF) )
        {
            p_ifinfo[num].ifname = info->name;
            p_ifinfo[num].if_flags = PPA_F_LAN_IF;
            num++;
        }
        if ( (info->flags & NETIF_WAN_IF) )
        {
            p_ifinfo[num].ifname = info->name;
            p_ifinfo[num].if_flags = 0;
            num++;
        }
    } while ( ppa_netif_iterate_next(&pos, &info) == PPA_SUCCESS );

    ppa_netif_stop_iteration();

    *num_ifs = num;
    *ifinfo = p_ifinfo;

    return PPA_SUCCESS;
}

#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
int32_t ppa_disconn_if(PPA_NETIF *netif, PPA_DP_SUBIF *subif, uint8_t *mac, uint32_t flags)
{
    if (!netif && !subif && !mac)
        return PPA_FAILURE;

    if (netif)
    {
        ppa_remove_sessions_on_netif(ppa_get_netif_name(netif));
    }
    else if (subif)
    {
        ppa_remove_sessions_on_subif(subif);
    }
    else if (mac)
    {
        ppa_remove_sessions_on_macaddr(mac);
    }

    return PPA_SUCCESS;
}
#endif

int32_t ppa_multicast_pkt_srcif_add(PPA_BUF *pkt_buf, PPA_NETIF *rx_if)
{
    IP_ADDR_C ip={0};
    IP_ADDR_C src_ip={0};
    struct mc_group_list_item *p_item;
    struct netif_info *p_netif_info;
    int32_t res = PPA_SESSION_NOT_ADDED;
    int32_t ret;

    if( !rx_if )
    {
        rx_if = ppa_get_pkt_src_if( pkt_buf);
    }

    if(ppa_get_multicast_pkt_ip( pkt_buf, &ip, &src_ip) != PPA_SUCCESS){
        return res;
    }

    if(is_ip_zero(&ip))
    {
        return res;
    }

    ppa_mc_get_htable_lock();
    ret = __ppa_lookup_mc_group(&ip, &src_ip, &p_item);

    if (ret == PPA_MC_SESSION_VIOLATION ){//if violation, there is a item with src ip all zero, so search again with src ip zero
        ppa_memset(&src_ip, 0, sizeof(src_ip));
        ret = __ppa_lookup_mc_group(&ip, &src_ip, &p_item);
    }

    if ( ret == PPA_SESSION_EXISTS )
    {
        if( p_item->src_netif &&  p_item->src_netif != rx_if )
        { /*at present, we don't allowed to change multicast src_if */
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "Not matched src if: original srcif is %s, but new srcif is %s: %d.%d.%d.%d\n", ppa_get_netif_name(p_item->src_netif), ppa_get_netif_name(rx_if), ip.ip.ip >> 24, (ip.ip.ip >> 16) & 0xFF, (ip.ip.ip >> 8) & 0xFF, ip.ip.ip & 0xFF);
            goto ENTRY_ADD_EXIT;
        }
        if( p_item->flags & SESSION_ADDED_IN_HW )
        { //already added into HW. no change here
            res = PPA_SESSION_ADDED;
            goto ENTRY_ADD_EXIT;
        }

        if( ppa_is_netif_bridged(NULL, rx_if) )
            p_item->bridging_flag =1; //If the receive interface is in bridge, then regard it as bridge mode
        else
            p_item->bridging_flag =0;

        //  add to HW if possible
        if ( ppa_netif_update(rx_if, NULL) != PPA_SUCCESS ||  ppa_netif_lookup( ppa_get_netif_name( rx_if), &p_netif_info) != PPA_SUCCESS )
        {
            ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_multicast_pkt_srcif_add cannot get interface %s for  multicast session info: %d.%d.%d.%d\n", ppa_get_netif_name(rx_if), ip.ip.ip >> 24, (ip.ip.ip >> 16) & 0xFF, (ip.ip.ip >> 8) & 0xFF, ip.ip.ip & 0xFF) ;
            res = PPA_SESSION_NOT_ADDED;
            goto ENTRY_ADD_EXIT;
        }

        ppa_netif_put(p_netif_info);

        if ( p_netif_info->flags & NETIF_PHYS_PORT_GOT )
        {
            //  PPPoE and source mac
            if ( !p_item->bridging_flag )
            {
                if( p_netif_info->flags & NETIF_PPPOE )
                    p_item->flags |= SESSION_VALID_PPPOE;
            }

            //  VLAN
            if(p_netif_info->flags & NETIF_VLAN_CANT_SUPPORT ) 
                ppa_debug(DBG_ENABLE_MASK_ASSERT,"MC processing can support two layers of VLAN only\n");

            if ( (p_netif_info->flags & NETIF_VLAN_OUTER) )
                p_item->flags |= SESSION_VALID_OUT_VLAN_RM;
            if ( (p_netif_info->flags & NETIF_VLAN_INNER) )
                p_item->flags |= SESSION_VALID_VLAN_RM;

            p_item->src_netif = p_netif_info->netif;

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	    if ( ppa_hsel_add_wan_mc_group(p_item) != PPA_SUCCESS )
#else
            if( ppa_hw_add_mc_group(p_item) != PPA_SUCCESS )
#endif
            {
                ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_hw_add_mc_group(%d.%d.%d.%d): fail ???\n", ip.ip.ip >> 24, (ip.ip.ip >> 16) & 0xFF, (ip.ip.ip >> 8) & 0xFF, ip.ip.ip& 0xFF);
            }
            else
            {
                res = PPA_SESSION_ADDED;
                ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "ppa_hw_add_mc_group(%d.%d.%d.%d): sucessfully\n", ip.ip.ip >> 24, (ip.ip.ip >> 16) & 0xFF, (ip.ip.ip >> 8) & 0xFF, ip.ip.ip & 0xFF);
                ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "%s: update src interface:(%s)\n", __FUNCTION__, p_netif_info->netif->name );
            }
        }
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_DUMP_MC_GROUP, "Not found the multicast group in existing list:%u.%u.%u.%u\n", NIPQUAD((ip.ip.ip)) );
    }

ENTRY_ADD_EXIT:
    ppa_mc_release_htable_lock();

    return res;
}

int32_t ppa_hook_wan_mii0_vlan_range_add(PPA_VLAN_RANGE *vlan_range, uint32_t flags)
{
    if ( vlan_range )
    {
        PPE_WAN_VID_RANGE vid={0};
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"vlanrange: from %x to %x\n", vlan_range->start_vlan_range, vlan_range->end_vlan_range);
        vid.vid = (vlan_range->start_vlan_range & 0xFFF ) | ( (vlan_range->end_vlan_range & 0xFFF ) << 16) ;
        ppa_drv_set_mixed_wan_vlan_id( &vid, 0 );

        return PPA_SUCCESS;
    }
    return PPA_FAILURE;
}

int32_t ppa_hook_wan_mii0_vlan_range_del(PPA_VLAN_RANGE *vlan_range, int32_t flags)
{
    if ( vlan_range )
    {
        PPE_WAN_VID_RANGE vid = {0};
        vid.vid = (vlan_range->start_vlan_range & 0xFFF ) | ( (vlan_range->end_vlan_range & 0xFFF ) << 16) ;
        ppa_drv_set_mixed_wan_vlan_id(&vid, 0);
        return PPA_SUCCESS;
    }

    return PPA_FAILURE;
}

int32_t ppa_hook_wan_mii0_vlan_ranges_get(int32_t *num_ranges, PPA_VLAN_RANGE *vlan_ranges, uint32_t flags)
{
    if ( vlan_ranges && num_ranges )
    {
        PPE_WAN_VID_RANGE vlanid = {0};

        ppa_drv_get_mixed_wan_vlan_id(&vlanid, 0);

        vlan_ranges->start_vlan_range = vlanid.vid& 0xFFF;
        vlan_ranges->end_vlan_range = ( vlanid.vid>> 12 ) & 0xFFF;

        *num_ranges = 1;
        return PPA_SUCCESS;
    }

    return PPA_FAILURE;
}

int32_t ppa_get_max_entries(PPA_MAX_ENTRY_INFO *max_entry, uint32_t flags)
{
    if( !max_entry ) return PPA_FAILURE;

    ppa_drv_get_max_entries(max_entry, 0);

    return PPA_SUCCESS;
}

int32_t ppa_ip_sprintf( char *buf, PPA_IPADDR ip, uint32_t flag)
{
    int32_t len=0;
    if( buf)  {
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
        if( flag & SESSION_IS_IPV6 ) {
            len = ppa_sprintf(buf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", NIP6(ip.ip6) );
        }
        else
#endif
            len = ppa_sprintf(buf, "%u.%u.%u.%u", NIPQUAD(ip.ip) );

    }

    return len;
}

int32_t ppa_ip_comare( PPA_IPADDR ip1, PPA_IPADDR ip2, uint32_t flag )
{
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
    if( flag & SESSION_IS_IPV6 )
    {
        return ppa_memcmp(ip1.ip6, ip2.ip6, sizeof(ip1.ip6) );
    }
    else
#endif
    {
         return ppa_memcmp(&ip1.ip, &ip2.ip, sizeof(ip1.ip) );
    }
}


int32_t ppa_zero_ip( PPA_IPADDR ip)
{
    PPA_IPADDR zero_ip={0};

    return ( ppa_ip_comare(ip, zero_ip, 0 )==0 ) ? 1:0;
}

#ifdef CONFIG_PPA_PUMA7
void ppa_update_pp_add_fn(PPA_BUF *ppa_buf)
{
	int32_t ret = PPA_SESSION_NOT_ADDED;
	struct session_list_item* p_item;

	if ((ppa_buf->pp_packet_info->pp_session.session_handle < AVALANCHE_PP_MAX_ACCELERATED_SESSIONS)) {
		PPA_SESSION *p_session;
		enum ip_conntrack_info ctinfo;
		uint32_t flags = 0;

		p_item = NULL;
		p_session = NULL;

		p_session = nf_ct_get(ppa_buf, &ctinfo);

		if( p_session ) {
			flags |= CTINFO2DIR(ctinfo) == IP_CT_DIR_ORIGINAL ? PPA_F_SESSION_ORG_DIR : PPA_F_SESSION_REPLY_DIR;
			ret = ppa_session_find_by_tuple(p_session, flags & PPA_F_SESSION_REPLY_DIR, &p_item);

		} else {
			ret = ppa_find_session_from_skb(ppa_buf, 0, &p_item);
		}

		if ( ret == PPA_SESSION_EXISTS && p_item != NULL) {
			if ( p_item->ip_proto != PPA_IPPROTO_UDP ) {
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"p_item[%p] added to hardware\n", p_item);
				p_item->flags |= SESSION_ADDED_IN_HW;
				p_item->flags &= ~SESSION_ADDED_IN_SW;
			}
			ppa_session_put(p_item);
		}
	}	
	return;
}

void ppa_update_pp_del(PPA_SESSION *p_session)
{
	struct session_list_item* p_item;
	int32_t ret = PPA_SESSION_NOT_ADDED;

	if( p_session == NULL )
		return;

	p_item = NULL;
	ret = ppa_session_find_by_tuple(p_session, 0, &p_item);
	if (ret == PPA_SESSION_EXISTS && p_item != NULL){
		if (p_item->ip_proto == PPA_IPPROTO_UDP)
			p_item->flags &= ~SESSION_ADDED_IN_SW;
		ppa_session_put(p_item);
	}

	p_item = NULL;
	ret = ppa_session_find_by_tuple(p_session, 1, &p_item);
	if (ret == PPA_SESSION_EXISTS && p_item != NULL){
		if (p_item->ip_proto == PPA_IPPROTO_UDP)
			p_item->flags &= ~SESSION_ADDED_IN_SW;
		ppa_session_put(p_item);
	}

	return;
}

EXPORT_SYMBOL(ppa_update_pp_del);
EXPORT_SYMBOL(ppa_update_pp_add_fn);
#endif
EXPORT_SYMBOL(ppa_init);
EXPORT_SYMBOL(ppa_exit);
EXPORT_SYMBOL(ppa_enable);
EXPORT_SYMBOL(ppa_add_if);
EXPORT_SYMBOL(ppa_del_if);
EXPORT_SYMBOL(ppa_get_max_entries);
EXPORT_SYMBOL( ppa_ip_sprintf);
EXPORT_SYMBOL(ppa_ip_comare);
//EXPORT_SYMBOL(g_ppa_ppa_mtu);
EXPORT_SYMBOL(g_ppa_min_hits);
EXPORT_SYMBOL(g_ppe_fastpath_enabled);
EXPORT_SYMBOL(ppa_zero_ip);
