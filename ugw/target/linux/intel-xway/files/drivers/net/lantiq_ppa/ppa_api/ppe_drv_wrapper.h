/******************************************************************************
**
** FILE NAME    : ppe_drv_wrapper.h
** PROJECT      : PPA
** MODULES      : PPA Wrapper for PPE Driver API
**
** DATE         : 14 Mar 2011
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Wrapper for PPE Driver API
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 14 MAR 2011  Shao Guohua       Initiate Version
*******************************************************************************/
#ifndef PPA_PPE_DRV_WRAPPER_2011_03_14
#define PPA_PPE_DRV_WRAPPER_2011_03_14

extern uint32_t ppa_drv_dp_sb_addr_to_fpi_addr_convert(PPA_FPI_ADDR*a, uint32_t flag);

extern uint32_t ppa_drv_get_ppe_hal_id(PPA_VERSION *v, uint32_t flag);

extern uint32_t ppa_drv_get_firmware_id(PPA_VERSION *v, uint32_t flag);

extern uint32_t ppa_drv_get_number_of_phys_port(PPE_COUNT_CFG *count, uint32_t flag);

extern uint32_t ppa_drv_get_phys_port_info(PPE_IFINFO *info, uint32_t flag);

extern uint32_t ppa_drv_get_max_entries(PPA_MAX_ENTRY_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_set_mixed_wan_vlan_id(PPE_WAN_VID_RANGE *vlan_id, uint32_t flag);
extern uint32_t ppa_drv_get_mixed_wan_vlan_id(PPE_WAN_VID_RANGE *vlan_id, uint32_t flag);

extern uint32_t ppa_drv_set_route_cfg( PPE_ROUTING_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_set_bridging_cfg( PPE_BRDG_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_set_fast_mode(PPE_FAST_MODE_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_get_acc_mode(PPE_ACC_ENABLE *acc_enable, uint32_t flag);

extern uint32_t ppa_drv_set_acc_mode(PPE_ACC_ENABLE *acc_enable, uint32_t flag);

extern uint32_t ppa_drv_set_default_dest_list( PPE_DEST_LIST *cfg, uint32_t flag);

extern uint32_t ppa_drv_set_bridge_if_vlan_config(PPE_BRDG_VLAN_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_get_bridge_if_vlan_config(PPE_BRDG_VLAN_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_add_vlan_map( PPE_BRDG_VLAN_FILTER_MAP *filter, uint32_t flag);
extern uint32_t ppa_drv_del_vlan_map(PPE_BRDG_VLAN_FILTER_MAP *filter, uint32_t flag);

extern uint32_t ppa_drv_get_vlan_map(PPE_BRDG_VLAN_FILTER_MAP *filter , uint32_t flag);

extern uint32_t ppa_drv_del_all_vlan_map(uint32_t flag);
extern uint32_t ppa_drv_get_max_vfilter_entries(PPE_VFILTER_COUNT_CFG *count, uint32_t flag);

extern uint32_t ppa_drv_is_ipv6_enabled(uint32_t flag);

extern uint32_t ppa_drv_add_routing_entry(PPE_ROUTING_INFO *entry , uint32_t flag);
extern uint32_t ppa_drv_del_routing_entry(PPE_ROUTING_INFO *entry, uint32_t flag);
extern uint32_t ppa_drv_update_routing_entry(PPE_ROUTING_INFO *entry , uint32_t flag);

extern uint32_t ppa_drv_add_wan_mc_entry( PPE_MC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_wan_mc_entry( PPE_MC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_update_wan_mc_entry( PPE_MC_INFO *entry, uint32_t flag);

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
extern uint32_t ppa_drv_set_wan_mc_rtp( PPE_MC_INFO *entry);
extern uint32_t ppa_drv_get_mc_rtp_sampling_cnt( PPE_MC_INFO *entry);
#endif

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
extern uint32_t ppa_drv_set_mib_mode(PPE_MIB_MODE_ENABLE *cfg, uint32_t );
extern uint32_t ppa_drv_get_mib_mode(PPE_MIB_MODE_ENABLE *cfg);
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
extern uint32_t ppa_drv_add_capwap_entry(PPA_CMD_CAPWAP_INFO *entry , uint32_t flag);
extern uint32_t ppa_drv_delete_capwap_entry(PPA_CMD_CAPWAP_INFO *entry , uint32_t flag);
extern uint32_t ppa_drv_get_capwap_mib(PPA_CMD_CAPWAP_INFO *entry,uint32_t flag); 
#endif

//extern uint32_t ppa_drv_get_dest_ip_from_wan_mc_entry(PPE_MC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_add_bridging_entry(PPE_BR_MAC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_bridging_entry(PPE_BR_MAC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_add_tunnel_entry(PPE_TUNNEL_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_tunnel_entry(PPE_TUNNEL_INFO *entry, uint32_t flag);
#if defined(L2TP_CONFIG) && L2TP_CONFIG
extern uint32_t ppa_drv_add_l2tptunnel_entry(PPA_L2TP_INFO *entry, uint32_t flag);
extern uint32_t ppa_drv_del_l2tptunnel_entry(PPA_L2TP_INFO *entry, uint32_t flag);
#endif

extern uint32_t ppa_drv_add_pppoe_entry(PPE_PPPOE_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_pppoe_entry(PPE_PPPOE_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_get_pppoe_entry(PPE_PPPOE_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_add_mtu_entry(PPE_MTU_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_mtu_entry(PPE_MTU_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_get_mtu_entry(PPE_MTU_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_get_routing_entry_bytes(PPE_ROUTING_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_get_mc_entry_bytes(PPE_MC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_add_mac_entry(PPE_ROUTE_MAC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_mac_entry(PPE_ROUTE_MAC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_get_mac_entry(PPE_ROUTE_MAC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_add_outer_vlan_entry( PPE_OUT_VLAN_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_outer_vlan_entry(PPE_OUT_VLAN_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_get_outer_vlan_entry(PPE_OUT_VLAN_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_add_ipv6_entry(PPE_IPV6_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_del_ipv6_entry(PPE_IPV6_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_multifield_control( PPE_ENABLE_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_get_multifield_status(PPE_ENABLE_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_get_multifield_max_entry( PPE_COUNT_CFG *count, uint32_t flag);

extern uint32_t ppa_drv_add_multifield_entry(PPE_MULTIFILED_FLOW *flow, uint32_t flag);

extern uint32_t ppa_drv_get_multifield_entry( PPE_MULTIFILED_FLOW *flow, uint32_t flag);

extern uint32_t ppa_drv_del_multifield_entry(PPE_MULTIFILED_FLOW *flow, uint32_t flag);

extern uint32_t ppa_drv_del_multifield_entry_via_index(PPE_MULTIFILED_FLOW *flow, uint32_t flag);

#ifdef CONFIG_LTQ_PPA_QOS
extern uint32_t ppa_drv_get_qos_status( PPA_QOS_STATUS *status, uint32_t flag);
extern uint32_t ppa_drv_get_qos_qnum( PPE_QOS_COUNT_CFG *count, uint32_t flag);
extern uint32_t ppa_drv_get_qos_mib( PPE_QOS_MIB_INFO *mib, uint32_t flag);
#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
extern uint32_t ppa_drv_set_ctrl_qos_rate(PPE_QOS_ENABLE_CFG *enable_cfg, uint32_t flag);
extern uint32_t ppa_drv_get_ctrl_qos_rate(PPE_QOS_ENABLE_CFG *enable_cfg, uint32_t flag);
extern uint32_t ppa_drv_set_qos_rate( PPE_QOS_RATE_SHAPING_CFG *cfg , uint32_t flag);
extern uint32_t ppa_drv_get_qos_rate( PPE_QOS_RATE_SHAPING_CFG *cfg , uint32_t flag);
extern uint32_t ppa_drv_reset_qos_rate( PPE_QOS_RATE_SHAPING_CFG *cfg , uint32_t flag);
extern uint32_t ppa_drv_init_qos_rate(uint32_t flag);
#endif
#ifdef CONFIG_LTQ_PPA_QOS_WFQ
extern uint32_t ppa_drv_set_ctrl_qos_wfq(PPE_QOS_ENABLE_CFG *enable_cfg, uint32_t flag);
extern uint32_t ppa_drv_get_ctrl_qos_wfq(PPE_QOS_ENABLE_CFG *enable_cfg, uint32_t flag);
extern uint32_t ppa_drv_set_qos_wfq( PPE_QOS_WFQ_CFG *cfg, uint32_t flag);
extern uint32_t ppa_drv_get_qos_wfq( PPE_QOS_WFQ_CFG *cfg, uint32_t flag);
extern uint32_t ppa_drv_reset_qos_wfq( PPE_QOS_WFQ_CFG *cfg, uint32_t flag);
extern uint32_t ppa_drv_init_qos_wfq( uint32_t flag);
#endif
#endif

extern uint32_t ppa_drv_get_dsl_mib(PPA_DSL_QUEUE_MIB *mib, uint32_t flag);
extern uint32_t ppa_drv_get_ports_mib(PPA_PORT_MIB *mib, uint32_t flag);
extern uint32_t ppa_drv_get_itf_mib( PPE_ITF_MIB_INFO *mib, uint32_t flag);

extern uint32_t ppa_drv_test_and_clear_hit_stat(PPE_ROUTING_INFO *entry, uint32_t flag);
extern uint32_t ppa_drv_test_and_clear_mc_hit_stat(PPE_MC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_test_and_clear_bridging_hit_stat(PPE_BR_MAC_INFO *entry, uint32_t flag);

extern uint32_t ppa_drv_set_value( PPA_CMD_VARIABLE_VALUE_INFO *v, uint32_t flag);
extern uint32_t ppa_drv_get_value( PPA_CMD_VARIABLE_VALUE_INFO *v, uint32_t flag);

extern uint32_t ppa_set_wan_itf( PPE_WANITF_CFG *cfg, uint32_t flag);

extern uint32_t ppa_drv_hal_init(uint32_t flag);
extern uint32_t ppa_drv_hal_exit(uint32_t flag);
extern uint32_t ppa_get_session_hash( PPE_SESSION_HASH *cfg, uint32_t flag);

#if defined(MBR_CONFIG) && MBR_CONFIG
extern uint32_t ppa_drv_set_qos_shaper( PPE_QOS_RATE_SHAPING_CFG *cfg , uint32_t flag);
extern uint32_t ppa_drv_get_qos_shaper( PPE_QOS_RATE_SHAPING_CFG *cfg , uint32_t flag);
#endif
#endif //end of PPA_PPE_DRV_WRAPPER_2011_03_14


