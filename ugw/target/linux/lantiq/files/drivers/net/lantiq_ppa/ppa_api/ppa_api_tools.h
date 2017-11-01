#ifndef __PPA_API_TOOLS_H__20100318_1920__
#define __PPA_API_TOOLS_H__20100318_1920__

/*******************************************************************************
**
** FILE NAME    : ppa_api_toos.h
** PROJECT      : PPA
** MODULES      : PPA API (Tools APIs)
**
** DATE         : 18 March 2010
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack Tools API Implementation Header File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 18 March 2010 Shao Guohua        Initiate Version
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



/*
 * ####################################
 *              Data Type
 * ####################################
 */
typedef enum{
    PPA_UNINIT_STATE = 0,
    PPA_INIT_STATE   = 1, 
}PPA_INIT_STATUS_t;

typedef enum{
    PPA_ENABLE_FN = 0,
    PPA_GET_STATUS_FN,
    PPA_SESSION_ADD_FN,
    PPA_SESSION_DEL_FN,
    PPA_SESSION_MODIFY_FN,
    PPA_SESSION_GET_FN,
    PPA_SESSION_BRADD_FN,
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    PPA_SESSION_PRIO_FN,
#endif
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
    PPA_SESSION_IPSEC_ADD_FN,
    PPA_SESSION_IPSEC_DEL_FN,
#endif
    PPA_MC_GROUP_UPDATE_FN,
    PPA_MC_GROUP_GET_FN,
    PPA_MC_ENTRY_MODIFY_FN,
    PPA_MC_ENTRY_GET_FN,
    PPA_MC_PKT_SRCIF_ADD_FN,
    PPA_INACTIVITY_STATUS_FN,
    PPA_SET_INACTIVITY_FN,
    PPA_BRIDGE_ENTRY_ADD_FN,
    PPA_BRIDGE_ENTRY_DELETE_FN,
    PPA_BRIDGE_ENTRY_HIT_TIME_FN,
    PPA_BRIDGE_ENTRY_INACTIVITY_STATUS_FN,
    PPA_SET_BRIDGE_ENTRY_TIMEOUT_FN,
    PPA_BRIDGE_ENABLE_FN,
    PPA_SET_BRIDGE_IF_VLAN_CONFIG_FN,
    PPA_GET_BRIDGE_IF_VLAN_CONFIG_FN,
    PPA_VLAN_FILTER_ADD_FN,
    PPA_VLAN_FILTER_DEL_FN,
    PPA_VLAN_FILTER_GET_ALL_FN,
    PPA_VLAN_FILTER_DEL_ALL_FN,
    PPA_GET_IF_STATS_FN,
    PPA_GET_ACCEL_STATS_FN,
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
    PPA_GET_NETIF_ACCEL_STATS_FN,
#endif
    PPA_SET_IF_MAC_ADDRESS_FN,
    PPA_GET_IF_MAC_ADDRESS_FN,
    PPA_ADD_IF_FN,
    PPA_DEL_IF_FN,
    PPA_GET_IF_FN,
    PPA_DISCONN_IF_FN,
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
    PPA_REG_CLASS2PRIO_FN,
#endif
    PPA_DIRECTPATH_REGISTER_DEV_FN,
    PPA_DIRECTPATH_SEND_FN,
    PPA_DIRECTPATH_RX_STOP_FN,
    PPA_DIRECTPATH_EX_RX_STOP_FN,
    PPA_DIRECTPATH_RX_RESTART_FN,
    PPA_DIRECTPATH_EX_RX_RESTART_FN,
    PPA_GET_NETIF_FOR_PPA_IFID_FN,
    PPA_GET_IFID_FOR_NETIF_FN,
    PPA_WAN_MII0_VLAN_RANGE_ADD_FN,
    PPA_WAN_MII0_VLAN_RANGE_DEL_FN,
    PPA_WAN_MII0_VLAN_RANGES_GET_FN,
    PPA_GET_QOS_QNUM_FN,
    PPA_GET_QOS_MIB_FN,
    PPA_SET_CTRL_QOS_RATE_FN,
    PPA_GET_CTRL_QOS_RATE_FN,
    PPA_SET_QOS_RATE_FN,
    PPA_GET_QOS_RATE_FN,
    PPA_RESET_QOS_RATE_FN,
    PPA_SET_CTRL_QOS_WFQ_FN,
    PPA_GET_CTRL_QOS_WFQ_FN,
    PPA_SET_QOS_WFQ_FN,
    PPA_GET_QOS_WFQ_FN,
    PPA_RESET_QOS_WFQ_FN,
    PPA_MULTIFIELD_CONTROL_FN,
    PPA_GET_MULTIFIELD_STATUS_FN,
    PPA_GET_MULTIFIELD_MAX_FLOW_FN,
    PPA_ADD_MULTIFIELD_FLOW_FN,
    PPA_DEL_MULTIFIELD_FLOW_FN,
    PPA_QUICK_DEL_MULTIFIELD_FLOW_FN,
    PPA_GET_DSL_MIB_FN,
    PPA_GET_PORT_MIB_FN,
    PPA_SET_LAN_SEPARATE_FLAG_FN, 
    PPA_GET_LAN_SEPARATE_FLAG_FN, 
    PPA_SET_WAN_SEPARATE_FLAG_FN, 
    PPA_GET_WAN_SEPARATE_FLAG_FN,
    PPA_ENABLE_SW_FASTPATH_FN,   // Software Acceleration 
    PPA_GET_SW_FASTPATH_STATUS_FN,
    PPA_SW_FASTPATH_SEND_FN,
	PPA_DTLK_REGISTER_DEV_FN, 
    PPA_DIRECTPATH_EX_REGISTER_DEV_FN,
    PPA_DIRECTPATH_EX_SEND_FN,
    PPA_DIRECTPATH_ALLOC_SKB_FN,
    PPA_DIRECTPATH_RECYCLE_SKB_FN,
    PPA_HOOK_FN_MAX,
}PPA_EXPORT_FN_NO;

/*
 * ####################################
 *             Declaration
 * ####################################
 */
extern int32_t  get_ppa_hook_list_count(void);
//extern int32_t  ppa_enable_hook( int8_t *name, uint32_t enable, uint32_t flag);
extern uint32_t ppa_is_init(void);
extern int32_t  ppa_ioctl_set_hook(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t  ppa_ioctl_get_hook_list(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t  ppa_ioctl_read_mem(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t  ppa_ioctl_set_mem(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t  ppa_ioctl_dbg_tool_test(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);


extern void     ppa_set_init_status(PPA_INIT_STATUS_t);
extern void     ppa_hook_fn_uninit(void);
extern void     ppa_reg_export_fn(PPA_EXPORT_FN_NO fn_no, uint32_t in_fn_addr, uint8_t *name,uint32_t *out_fn_addr, uint32_t mid_fn_addr );
extern int32_t  ppa_unreg_export_fn(PPA_EXPORT_FN_NO fn_no, uint32_t *out_fn_addr);
extern void     ppa_export_fn_manager_exit(void);


#if defined (PPA_WRPFN_DEFINITION)
extern PPA_HOOK_INFO g_expfn_table[PPA_HOOK_FN_MAX];
#endif

#if !defined (PPA_WRPFN_DEFINITION)
#define WRPFN_1(RT,FNAME,T1, P1, IDX)                   \
    extern RT FNAME(T1 P1) ;
#else
#define WRPFN_1(RT,FNAME,T1, P1, IDX)                   \
    RT FNAME(T1 P1)                                     \
    {                                                   \
        RT ret = PPA_FAILURE;                           \
        RT (*pfn)(T1) = NULL;                           \
        ppa_rcu_read_lock();                            \
        if(ppa_is_init()&&g_expfn_table[IDX].used_flag && g_expfn_table[IDX].hook_flag ){ \
            if( in_irq() )        { /* critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/ }           \
            else if( irqs_disabled() ) {/*  critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() ); */ } \
            else {                                              \
                pfn = (RT (*)(T1))g_expfn_table[IDX].hook_addr; \
                if(pfn){                                    \
                    ret = pfn(P1);                          \
                }                                           \
            }                                               \
        }                                                   \
        ppa_rcu_read_unlock();                              \
        return ret;                                         \
    }                                                       \
    EXPORT_SYMBOL(FNAME);
#endif

#if !defined (PPA_WRPFN_DEFINITION)
#define WRPFN_2(RT,FNAME,T1, P1, T2, P2, IDX)           \
    extern RT FNAME(T1 P1, T2 P2);
#else
#define WRPFN_2(RT,FNAME,T1, P1, T2, P2, IDX)           \
    RT FNAME(T1 P1, T2 P2)                              \
    {                                                   \
        RT ret = PPA_FAILURE;                           \
        RT (*pfn)(T1, T2) = NULL;                       \
        ppa_rcu_read_lock();                             \
        if(ppa_is_init()&&g_expfn_table[IDX].used_flag && g_expfn_table[IDX].hook_flag ){ \
            if( in_irq() )        { /* critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/ }           \
            else if( irqs_disabled() ) { /* critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() ); */ } \
            else {                                                    \
                pfn = (RT (*)(T1, T2))g_expfn_table[IDX].hook_addr;   \
                if(pfn){                               \
                    ret = pfn(P1,P2);                  \
                }                                      \
            }                                          \
        }                                              \
        ppa_rcu_read_unlock();                         \
        return ret;                                    \
    }                                                  \
    EXPORT_SYMBOL(FNAME);
#endif

#if !defined (PPA_WRPFN_DEFINITION)
#define WRPFN_3(RT,FNAME,T1, P1, T2, P2, T3, P3, IDX)   \
    extern RT FNAME(T1 P1, T2 P2, T3 P3);
#else
#define WRPFN_3(RT,FNAME,T1, P1, T2, P2, T3, P3, IDX)   \
    RT FNAME(T1 P1, T2 P2, T3 P3)                       \
    {                                                   \
        RT ret = PPA_FAILURE;                           \
        RT (*pfn)(T1, T2, T3) = NULL;                   \
        ppa_rcu_read_lock();                            \
        if(ppa_is_init()&&g_expfn_table[IDX].used_flag && g_expfn_table[IDX].hook_flag ){ \
            if( in_irq() )        { /* critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/ }           \
            else if( irqs_disabled() ) { /* critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/  } \
            else  {                                                     \
                pfn = (RT (*)(T1, T2, T3))g_expfn_table[IDX].hook_addr;\
                if(pfn){                                 \
                    ret = pfn(P1,P2,P3);                 \
                }                                        \
            }                                           \
        }                                               \
        ppa_rcu_read_unlock();                          \
        return ret;                                     \
    }                                                   \
    EXPORT_SYMBOL(FNAME);
#endif

#if !defined (PPA_WRPFN_DEFINITION)
#define WRPFN_4(RT,FNAME,T1, P1, T2, P2, T3, P3, T4, P4, IDX)       \
    extern RT FNAME(T1 P1, T2 P2, T3 P3, T4 P4);
#else
#define WRPFN_4(RT,FNAME,T1, P1, T2, P2, T3, P3, T4, P4, IDX)       \
    RT FNAME(T1 P1, T2 P2, T3 P3, T4 P4)                            \
    {                                                               \
        RT ret = PPA_FAILURE;                                       \
        RT (*pfn)(T1, T2, T3, T4) = NULL;                           \
        ppa_rcu_read_lock();                                        \
        if(ppa_is_init()&&g_expfn_table[IDX].used_flag && g_expfn_table[IDX].hook_flag ){   \
            if( in_irq() )        {  /*critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/ }            \
            else if( irqs_disabled() ) { /* critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/  } \
            else  {                                                               \
                pfn = (RT (*)(T1, T2, T3, T4))g_expfn_table[IDX].hook_addr;       \
                if(pfn){                                             \
                    ret = pfn(P1,P2,P3,P4);                          \
                }                                                    \
            }                                                        \
        }                                                            \
        ppa_rcu_read_unlock();                                       \
        return ret;                                                  \
    }                                                                \
    EXPORT_SYMBOL(FNAME);
#endif

#if !defined (PPA_WRPFN_DEFINITION)
#define WRPFN_5(RT,FNAME,T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, IDX)       \
    extern RT FNAME(T1 P1, T2 P2, T3 P3, T4 P4, T5 P5);
#else
#define WRPFN_5(RT,FNAME,T1, P1, T2, P2, T3, P3, T4, P4, T5, P5, IDX)       \
    RT FNAME(T1 P1, T2 P2, T3 P3, T4 P4, T5 P5)                             \
    {                                                                       \
        RT ret = PPA_FAILURE;                                               \
        RT (*pfn)(T1, T2, T3, T4, T5) = NULL;                               \
        ppa_rcu_read_lock();                                                \
        if(ppa_is_init()&&g_expfn_table[IDX].used_flag && g_expfn_table[IDX].hook_flag ){   \
            if( in_irq() )        { /* critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/ }           \
            else if( irqs_disabled() ) {/*  critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[IDX].hookname, preempt_count() );*/  } \
            else {                                                              \
                pfn = (RT (*)(T1, T2, T3, T4, T5))g_expfn_table[IDX].hook_addr; \
                if(pfn){                                                        \
                    ret = pfn(P1,P2,P3,P4,P5);                                  \
                }                                                               \
            }                                                                   \
        }                                                                       \
        ppa_rcu_read_unlock();                                                  \
        return ret;                                                             \
    }                                                                           \
    EXPORT_SYMBOL(FNAME);
#endif

/* wrapper functions stay in kernel to handle smp without requring hook interface change */
/* 
    wrapper functions support:
    1. smp in function call and ppa exit
    2. smp in function call and module unload
*/

WRPFN_3(int32_t, ppa_enable_rpfn ,     uint32_t,   lan_rx_ppa_enable, uint32_t,  wan_rx_ppa_enable, uint32_t, flags, PPA_ENABLE_FN )
WRPFN_3(int32_t, ppa_get_status_rpfn,  uint32_t*,  lan_rx_ppa_enable, uint32_t*, wan_rx_ppa_enable, uint32_t, flags, PPA_GET_STATUS_FN)

WRPFN_3(int32_t, ppa_session_add_rpfn,      PPA_BUF*,       ppa_buf,        PPA_SESSION*,       p_session,  uint32_t, flags, PPA_SESSION_ADD_FN)
WRPFN_2(int32_t, ppa_session_del_rpfn,      PPA_SESSION*,   p_session,      uint32_t,           flags, PPA_SESSION_DEL_FN)
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
WRPFN_2(int32_t, ppa_session_ipsec_add_rpfn,     PPA_XFRM_STATE *,       ppa_x,        sa_direction,       dir,  PPA_SESSION_IPSEC_ADD_FN)
WRPFN_1(int32_t, ppa_session_ipsec_del_rpfn,      PPA_XFRM_STATE*,   ppa_x,  PPA_SESSION_IPSEC_DEL_FN)

#endif
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
WRPFN_2(int32_t, ppa_session_prio_rpfn,      PPA_SESSION*,   p_session,      uint32_t,           flags, PPA_SESSION_PRIO_FN)
#endif
WRPFN_3(int32_t, ppa_session_modify_rpfn,   PPA_SESSION*,   p_session,      PPA_SESSION_EXTRA*, p_extra,    uint32_t, flags, PPA_SESSION_MODIFY_FN)
WRPFN_4(int32_t, ppa_session_get_rpfn,      PPA_SESSION ***, pp_sessions,   PPA_SESSION_EXTRA**,pp_extra,   int32_t*, p_num_entries, uint32_t, flags, PPA_SESSION_GET_FN)
WRPFN_3(int32_t, ppa_session_bradd_rpfn,      PPA_BUF*,       ppa_buf,        PPA_SESSION*,       p_session,  uint32_t, flags, PPA_SESSION_BRADD_FN)

WRPFN_2(int32_t, ppa_mc_group_update_rpfn,  PPA_MC_GROUP*,  ppa_mc_entry,   uint32_t,   flags,  PPA_MC_GROUP_UPDATE_FN)
WRPFN_4(int32_t, ppa_mc_group_get_rpfn,     IP_ADDR_C,      ip_mc_group,    IP_ADDR_C,  src_ip, PPA_MC_GROUP*,  ppa_mc_entry,   uint32_t,   flags, PPA_MC_GROUP_GET_FN)
WRPFN_5(int32_t, ppa_mc_entry_modify_rpfn,  IP_ADDR_C,      ip_mc_group,    IP_ADDR_C,  src_ip, PPA_MC_GROUP*,  ppa_mc_entry,   PPA_SESSION_EXTRA*, p_extra, uint32_t,  flags, PPA_MC_ENTRY_MODIFY_FN)
WRPFN_4(int32_t, ppa_mc_entry_get_rpfn,     IP_ADDR_C,      ip_mc_group,    IP_ADDR_C,  src_ip, PPA_SESSION_EXTRA*, p_extra,    uint32_t,   flags, PPA_MC_ENTRY_GET_FN)
WRPFN_2(int32_t, ppa_mc_pkt_srcif_add_rpfn, PPA_BUF*,       pkt_buf,        PPA_NETIF*, rx_if,  PPA_MC_PKT_SRCIF_ADD_FN)

WRPFN_1(int32_t, ppa_inactivity_status_rpfn,PPA_U_SESSION*, p_session,      PPA_INACTIVITY_STATUS_FN)
WRPFN_2(int32_t, ppa_set_inactivity_rpfn,   PPA_U_SESSION*, p_session,      int32_t,    timeout,PPA_SET_INACTIVITY_FN)

WRPFN_4(int32_t, ppa_bridge_entry_add_rpfn,                 uint8_t*,   mac_addr, PPA_NETIF*, 	brif,  	PPA_NETIF*,   netif, uint32_t,flags, PPA_BRIDGE_ENTRY_ADD_FN)
WRPFN_3(int32_t, ppa_bridge_entry_delete_rpfn,              uint8_t*,   mac_addr, PPA_NETIF*,   brif, 	uint32_t,     flags, PPA_BRIDGE_ENTRY_DELETE_FN)
WRPFN_3(int32_t, ppa_bridge_entry_hit_time_rpfn,            uint8_t*,   mac_addr, PPA_NETIF*,   brif, 	uint32_t*,    p_hit_time, PPA_BRIDGE_ENTRY_HIT_TIME_FN)
WRPFN_2(int32_t, ppa_bridge_entry_inactivity_status_rpfn,   uint8_t*,   mac_addr, PPA_NETIF*,   brif, 	PPA_BRIDGE_ENTRY_INACTIVITY_STATUS_FN)
WRPFN_3(int32_t, ppa_set_bridge_entry_timeout_rpfn,         uint8_t*,   mac_addr, PPA_NETIF*,   brif, 	uint32_t,     timeout, PPA_SET_BRIDGE_ENTRY_TIMEOUT_FN)
WRPFN_2(int32_t, ppa_bridge_enable_rpfn,                    uint32_t,   f_enable, uint32_t,     flags, 	PPA_BRIDGE_ENABLE_FN)

WRPFN_4(int32_t, ppa_set_bridge_if_vlan_config_rpfn,    PPA_NETIF*,             netif,              PPA_VLAN_TAG_CTRL*, vlan_tag_control,   PPA_VLAN_CFG*,  vlan_cfg, uint32_t, flags, PPA_SET_BRIDGE_IF_VLAN_CONFIG_FN)
WRPFN_4(int32_t, ppa_get_bridge_if_vlan_config_rpfn,    PPA_NETIF*,             netif,              PPA_VLAN_TAG_CTRL*, vlan_tag_control,   PPA_VLAN_CFG*,  vlan_cfg, uint32_t, flags, PPA_GET_BRIDGE_IF_VLAN_CONFIG_FN)
WRPFN_3(int32_t, ppa_vlan_filter_add_rpfn,              PPA_VLAN_MATCH_FIELD*,  vlan_match_field,   PPA_VLAN_INFO*,     vlan_info,          uint32_t,       flags, PPA_VLAN_FILTER_ADD_FN)
WRPFN_3(int32_t, ppa_vlan_filter_del_rpfn,              PPA_VLAN_MATCH_FIELD*,  vlan_match_field,   PPA_VLAN_INFO*,     vlan_info,          uint32_t,       flags, PPA_VLAN_FILTER_DEL_FN)
WRPFN_3(int32_t, ppa_vlan_filter_get_all_rpfn,          int32_t*,               num_filters,        PPA_VLAN_FILTER_CONFIG*,vlan_filters,   uint32_t,       flags, PPA_VLAN_FILTER_GET_ALL_FN)
WRPFN_1(int32_t, ppa_vlan_filter_del_all_rpfn,          uint32_t,               flags,              PPA_VLAN_FILTER_DEL_ALL_FN)

WRPFN_3(int32_t, ppa_get_if_stats_rpfn,         PPA_IFNAME*,    ifname,     PPA_IF_STATS*,      p_stats,    uint32_t,   flags, PPA_GET_IF_STATS_FN)
WRPFN_3(int32_t, ppa_get_accel_stats_rpfn,      PPA_IFNAME*,    ifname,     PPA_ACCEL_STATS*,   p_stats,    uint32_t,   flags, PPA_GET_ACCEL_STATS_FN)
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
WRPFN_3(int32_t, ppa_get_netif_accel_stats_rpfn,      PPA_IFNAME*,    ifname,     PPA_NETIF_ACCEL_STATS*,   p_stats,
uint32_t,   flags, PPA_GET_NETIF_ACCEL_STATS_FN)
#endif
WRPFN_3(int32_t, ppa_set_if_mac_address_rpfn,   PPA_IFNAME*,    ifname,     uint8_t*,           mac_addr,   uint32_t,   flags, PPA_SET_IF_MAC_ADDRESS_FN)
WRPFN_3(int32_t, ppa_get_if_mac_address_rpfn,   PPA_IFNAME*,    ifname,     uint8_t*,           mac_addr,   uint32_t,   flags, PPA_GET_IF_MAC_ADDRESS_FN)
WRPFN_2(int32_t, ppa_add_if_rpfn,               PPA_IFINFO*,    ifinfo,     uint32_t,           flags,      PPA_ADD_IF_FN)
WRPFN_2(int32_t, ppa_del_if_rpfn,               PPA_IFINFO*,    ifinfo,     uint32_t,           flags,      PPA_DEL_IF_FN)
WRPFN_3(int32_t, ppa_get_if_rpfn,               int32_t*,       num_ifs,    PPA_IFINFO**,       ifinfo,     uint32_t,   flags, PPA_GET_IF_FN)
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
WRPFN_4(int32_t, ppa_disconn_if_rpfn, 		PPA_NETIF*, 	netif,	    PPA_DP_SUBIF*,	subif,	    uint8_t*,	mac,	uint32_t,   flags,  PPA_DISCONN_IF_FN)	
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
WRPFN_4(int32_t, ppa_register_for_qos_class2prio_rpfn,	int32_t,	port_id,	PPA_NETIF*, 	netif,	PPA_QOS_CLASS2PRIO_CB,	qos_class2prio_cb,	uint32_t,	flags,  PPA_REG_CLASS2PRIO_FN)	
#endif
#endif    
#ifdef CONFIG_LTQ_PPA_API_DIRECTPATH
WRPFN_4(int32_t, ppa_directpath_register_dev_rpfn,  uint32_t*,  p_if_id,    PPA_NETIF*, netif,  PPA_DIRECTPATH_CB*,pDirectpathCb,   uint32_t,   flags, PPA_DIRECTPATH_REGISTER_DEV_FN)
WRPFN_4(int32_t, ppa_directpath_ex_register_dev_rpfn,  PPA_SUBIF*,  subif,    PPA_NETIF*, netif,  PPA_DIRECTPATH_CB*,pDirectpathCb,   uint32_t,   flags, PPA_DIRECTPATH_EX_REGISTER_DEV_FN)
WRPFN_4(int32_t, ppa_directpath_ex_send_rpfn,          PPA_SUBIF*,   subif,   PPA_BUF*,   skb,    int32_t,            len,            uint32_t,   flags, PPA_DIRECTPATH_EX_SEND_FN)
WRPFN_2(int32_t, ppa_directpath_rx_stop_rpfn,       uint32_t,   if_id,      uint32_t,   flags,  PPA_DIRECTPATH_RX_STOP_FN)
WRPFN_2(int32_t, ppa_directpath_ex_rx_stop_rpfn,       PPA_SUBIF*,   subif,      uint32_t,   flags,  PPA_DIRECTPATH_EX_RX_STOP_FN)
WRPFN_2(int32_t, ppa_directpath_rx_restart_rpfn,    uint32_t,   if_id,      uint32_t,   flags,  PPA_DIRECTPATH_RX_RESTART_FN)
WRPFN_2(int32_t, ppa_directpath_ex_rx_restart_rpfn,    PPA_SUBIF*,   subif,      uint32_t,   flags,  PPA_DIRECTPATH_EX_RX_RESTART_FN)
WRPFN_1(int32_t, ppa_get_ifid_for_netif_rpfn,       PPA_NETIF*, netif,      PPA_GET_IFID_FOR_NETIF_FN)
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
WRPFN_4(int32_t, ppa_directlink_register_dev_rpfn, int32_t*, p_if_id, PPA_DTLK_T*, dtlk, PPA_DIRECTPATH_CB*, pDirectpathCb, uint32_t,  flags, PPA_DTLK_REGISTER_DEV_FN)
#endif /*CONFIG_ACCL_11AC*/

WRPFN_3(PPA_BUF*, ppa_directpath_alloc_skb_rpfn, PPA_SUBIF*, psubif, int32_t,  len, uint32_t, flags, PPA_DIRECTPATH_ALLOC_SKB_FN);
WRPFN_3(int32_t, ppa_directpath_recycle_skb_rpfn, PPA_SUBIF*, psubif, PPA_BUF*, skb, uint32_t, flags, PPA_DIRECTPATH_RECYCLE_SKB_FN);

#endif /* CONFIG_LTQ_PPA_API_DIRECTPATH */

WRPFN_2(int32_t, ppa_wan_mii0_vlan_range_add_rpfn,  PPA_VLAN_RANGE*,vlan_range, uint32_t,flags, PPA_WAN_MII0_VLAN_RANGE_ADD_FN)
WRPFN_2(int32_t, ppa_wan_mii0_vlan_range_del_rpfn,  PPA_VLAN_RANGE*,vlan_range, int32_t,flags,  PPA_WAN_MII0_VLAN_RANGE_DEL_FN)
WRPFN_3(int32_t, ppa_wan_mii0_vlan_ranges_get_rpfn, int32_t*,       num_ranges, PPA_VLAN_RANGE*, vlan_ranges, uint32_t, flags, PPA_WAN_MII0_VLAN_RANGES_GET_FN)
    
WRPFN_2(int32_t, ppa_get_dsl_mib_rpfn,  PPA_DSL_QUEUE_MIB*,mib, uint32_t,flag, PPA_GET_DSL_MIB_FN)
WRPFN_3(int32_t, ppa_get_port_mib_rpfn, PPA_PORT_MIB*,mib,      uint32_t,rate_flag, uint32_t,flag, PPA_GET_PORT_MIB_FN)

WRPFN_1(int32_t, ppa_hook_set_lan_seperate_flag_rpfn, uint32_t, flags,      PPA_SET_LAN_SEPARATE_FLAG_FN)
WRPFN_1(uint32_t, ppa_hook_get_lan_seperate_flag_rpfn,uint32_t, flags,      PPA_GET_LAN_SEPARATE_FLAG_FN)
WRPFN_1(int32_t, ppa_hook_set_wan_seperate_flag_rpfn, uint32_t, flags,      PPA_SET_WAN_SEPARATE_FLAG_FN)
WRPFN_1(uint32_t, ppa_hook_get_wan_seperate_flag_rpfn,uint32_t, flags,      PPA_GET_WAN_SEPARATE_FLAG_FN)

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
WRPFN_1(int32_t, ppa_hook_set_sw_fastpath_enable_rpfn, uint32_t, flags, PPA_ENABLE_SW_FASTPATH_FN)
WRPFN_1(int32_t, ppa_hook_get_sw_fastpath_status_rpfn, uint32_t, flags, PPA_GET_SW_FASTPATH_STATUS_FN)
WRPFN_1(int32_t, ppa_hook_sw_fastpath_send_rpfn, PPA_BUF*, skb, PPA_SW_FASTPATH_SEND_FN) 
#endif

extern PPA_NETIF* ppa_get_netif_for_ppa_ifid_rpfn(uint32_t if_id);
extern int32_t ppa_directpath_send_rpfn(uint32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags);


int32_t ppa_ioctl_set_value(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_get_value(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
    
#endif  //  __PPA_API_CORE_H__20081103_1920__
/* @} */

