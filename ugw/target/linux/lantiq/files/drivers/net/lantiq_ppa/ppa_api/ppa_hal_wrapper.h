/******************************************************************************
**
** FILE NAME    : ppe_hal_wrapper.h
** PROJECT      : PPA
** MODULES      : PPA Wrapper for various HAL drivers
**
** DATE         : 27 Feb 2014
** AUTHOR       : Kamal Eradath
** DESCRIPTION  : PPA Wrapper for HAL Driver API
** COPYRIGHT    :              Copyright (c) 2014
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 27 Feb 2014  Kamal Eradath   Initiate Version
*******************************************************************************/
#ifndef PPA_HAL_WRAPPER_2014_02_27
#define PPA_HAL_WRAPPER_2014_02_27

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
// HAL selector table node;
#ifndef MAX_TUNNEL_ENTRIES
#define MAX_TUNNEL_ENTRIES                  	16
#endif


extern uint8_t ppa_drv_get_num_tunnel_entries(void);
extern uint8_t ppa_drv_get_num_registred_hals(void);
extern uint32_t ppa_drv_generic_hal_register(uint32_t hal_id, ppa_generic_hook_t generic_hook);
extern void ppa_drv_generic_hal_deregister(uint32_t hal_id); 

// All the HAL layers initialized must get registered with the HAL selectors regarding the "capabilities" it supports
// This funcion must be invoked by the all HALs while hal_init is called to register all the supported capabilities
extern uint32_t ppa_drv_register_cap(PPA_API_CAPS cap, uint8_t wt, PPA_HAL_ID hal_id);
extern uint32_t ppa_drv_deregister_cap(PPA_API_CAPS cap, PPA_HAL_ID hal_id);
// helper functions
extern int32_t ppa_select_hals_from_caplist(uint8_t start, uint8_t num_entries, PPA_HSEL_CAP_NODE *caps_list);
extern int32_t ppa_group_hals_in_capslist(uint8_t start, uint8_t num_entries, PPA_HSEL_CAP_NODE *caps_list);
// **1** Functions that need to be invoked on all the HALs registered
extern uint32_t ppa_drv_get_hal_id(PPA_VERSION *v, uint32_t flag);
// Function to be called for adding additional switch port configuration for bridge acceleration
extern uint32_t ppa_drv_add_br_port(PPA_BR_PORT_INFO *entry, uint32_t flag);
extern uint32_t ppa_drv_del_br_port(PPA_BR_PORT_INFO *entry, uint32_t flag);
extern uint32_t ppa_drv_set_hal_dbg(PPA_CMD_GENERAL_ENABLE_INFO *cfg, uint32_t flag);
/******************************************************************************************************/
// functions to be used in ppa_api_hal_selector.c
/******************************************************************************************************/
extern uint32_t ppa_hsel_add_complement(PPE_ROUTING_INFO *entry , uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_complement(PPE_ROUTING_INFO *entry , uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_add_routing_entry(PPE_ROUTING_INFO *entry , uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_routing_entry(PPE_ROUTING_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_update_routing_entry(PPE_ROUTING_INFO *entry , uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_add_wan_mc_entry( PPE_MC_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_wan_mc_entry( PPE_MC_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_update_wan_mc_entry( PPE_MC_INFO *entry, uint32_t flag, uint32_t hal_id);

extern uint32_t ppa_hsel_add_tunnel_entry(PPE_TUNNEL_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_tunnel_entry(PPE_TUNNEL_INFO *entry, uint32_t flag, uint32_t hal_id);
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO
extern uint32_t ppa_hsel_add_lro_entry(PPA_LRO_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_lro_entry(PPA_LRO_INFO *entry, uint32_t flag, uint32_t hal_id);
#endif // defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
extern uint32_t ppa_hsel_get_ipsec_tunnel_mib(IPSEC_TUNNEL_MIB_INFO *entry, uint32_t flag, uint32_t hal_id);
#endif
extern uint32_t ppa_hsel_get_routing_entry_bytes(PPE_ROUTING_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_get_mc_entry_bytes(PPE_MC_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_test_and_clear_hit_stat(PPE_ROUTING_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_test_and_clear_mc_hit_stat(PPE_MC_INFO *entry, uint32_t flag, uint32_t hal_id);

extern uint32_t ppa_hsel_add_outer_vlan_entry( PPE_OUT_VLAN_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_outer_vlan_entry(PPE_OUT_VLAN_INFO *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_get_outer_vlan_entry(PPE_OUT_VLAN_INFO *entry, uint32_t flag, uint32_t hal_id);

extern uint32_t ppa_hsel_init_qos_cfg(uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_uninit_qos_cfg(uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_add_qos_queue_entry(QOS_Q_ADD_CFG *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_modify_qos_queue_entry(QOS_Q_MOD_CFG *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_delete_qos_queue_entry(QOS_Q_DEL_CFG *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_set_qos_rate_entry(QOS_RATE_SHAPING_CFG *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_reset_qos_rate_entry(QOS_RATE_SHAPING_CFG *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_set_qos_shaper_entry(QOS_RATE_SHAPING_CFG *entry, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_mod_subif_port_cfg(QOS_MOD_SUBIF_PORT_CFG *entry, uint32_t flag, uint32_t hal_id);

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
extern uint32_t ppa_hsel_add_class_rule(PPA_CLASS_RULE *rule, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_mod_class_rule(PPA_CLASS_RULE *rule, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_del_class_rule(PPA_CLASS_RULE *rule, uint32_t flag, uint32_t hal_id);
extern uint32_t ppa_hsel_get_class_rule(PPA_CLASS_RULE *rule, uint32_t flag, uint32_t hal_id);
#endif

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
extern uint32_t ppa_hsel_set_wan_mc_rtp( PPE_MC_INFO *entry, uint32_t hal_id);
extern uint32_t ppa_hsel_get_mc_rtp_sampling_cnt( PPE_MC_INFO *entry, uint32_t hal_id);
#endif
/******************************************************************************************************/
#endif //defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR

#endif //end of PPA_HAL_WRAPPER_2014_02_27


