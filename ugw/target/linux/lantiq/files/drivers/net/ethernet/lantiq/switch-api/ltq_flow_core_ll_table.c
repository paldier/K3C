/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "ltq_ethsw_init.h"

#ifdef IFX_ETHSW_SW_FKT
 #undef IFX_ETHSW_SW_FKT
#endif /* IFX_ETHSW_SW_FKT */

#define IFX_ETHSW_SW_FKT(x, y) x? (LTQ_ll_fkt)y : NULL

#ifdef CONFIG_LTQ_8021X
   #undef CONFIG_LTQ_8021X
	#define CONFIG_LTQ_8021X		1
#else
	#define CONFIG_LTQ_8021X		0
#endif
#ifdef CONFIG_LTQ_MULTICAST
   #undef CONFIG_LTQ_MULTICAST
	#define CONFIG_LTQ_MULTICAST	1
#else
	#define CONFIG_LTQ_MULTICAST	0
#endif
#ifdef CONFIG_LTQ_QOS
   #undef CONFIG_LTQ_QOS
	#define CONFIG_LTQ_QOS			1
#else
	#define CONFIG_LTQ_QOS			0
#endif
#ifdef CONFIG_LTQ_STP
   #undef CONFIG_LTQ_STP
	#define CONFIG_LTQ_STP			1
#else
	#define CONFIG_LTQ_STP			0
#endif
#ifdef CONFIG_LTQ_VLAN
   #undef CONFIG_LTQ_VLAN
	#define CONFIG_LTQ_VLAN			1
#else
	#define CONFIG_LTQ_VLAN			0
#endif
#ifdef CONFIG_LTQ_WOL
   #undef CONFIG_LTQ_WOL
	#define CONFIG_LTQ_WOL			1
#else
	#define CONFIG_LTQ_WOL			0
#endif

const LTQ_ll_fkt ltq_fkt_ptr_tbl [] =
{
	/* 0x00 */
	(LTQ_ll_fkt) NULL,
	/* Command: IFX_ETHSW_MAC_TABLE_ENTRY_READ ; Index: 0x01 */
	(LTQ_ll_fkt) IFX_FLOW_MAC_TableEntryRead,
	/* Command: IFX_ETHSW_MAC_TABLE_ENTRY_QUERY ; Index: 0x02 */
	(LTQ_ll_fkt) IFX_FLOW_MAC_TableEntryQuery,
	/* Command: IFX_ETHSW_MAC_TABLE_ENTRY_ADD ; Index: 0x03 */
	(LTQ_ll_fkt) IFX_FLOW_MAC_TableEntryAdd,
	/* Command: IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE ; Index: 0x04 */
	(LTQ_ll_fkt) IFX_FLOW_MAC_TableEntryRemove,
	/* Command: IFX_ETHSW_MAC_TABLE_CLEAR ; Index: 0x05 */
	(LTQ_ll_fkt) IFX_FLOW_MAC_TableClear,
	/* Command: IFX_ETHSW_STP_PORT_CFG_SET ; Index: 0x06 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_STP , IFX_FLOW_STP_PortCfgSet),
	/* Command: IFX_ETHSW_STP_PORT_CFG_GET ; Index: 0x07 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_STP , IFX_FLOW_STP_PortCfgGet),
	/* Command: IFX_ETHSW_STP_BPDU_RULE_SET ; Index: 0x08 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_STP , IFX_FLOW_STP_BPDU_RuleSet),
	/* Command: IFX_ETHSW_STP_BPDU_RULE_GET ; Index: 0x09 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_STP , IFX_FLOW_STP_BPDU_RuleGet),
	/* Command: IFX_ETHSW_8021X_EAPOL_RULE_GET ; Index: 0x0A */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_8021X , IFX_FLOW_8021X_EAPOL_RuleGet),
	/* Command: IFX_ETHSW_8021X_EAPOL_RULE_SET ; Index: 0x0B */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_8021X , IFX_FLOW_8021X_EAPOL_RuleSet),
	/* Command: IFX_ETHSW_8021X_PORT_CFG_GET ; Index: 0x0C */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_8021X , IFX_FLOW_8021X_PortCfgGet),
	/* Command: IFX_ETHSW_8021X_PORT_CFG_SET ; Index: 0x0D */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_8021X , IFX_FLOW_8021X_PortCfgSet),
	/* Command: IFX_ETHSW_VLAN_RESERVED_ADD ; Index: 0x0E */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_ReservedAdd),
	/* Command: IFX_ETHSW_VLAN_RESERVED_REMOVE ; Index: 0x0F */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_ReservedRemove),
	/* Command: IFX_ETHSW_VLAN_PORT_CFG_GET ; Index: 0x10 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_PortCfgGet),
	/* Command: IFX_ETHSW_VLAN_PORT_CFG_SET ; Index: 0x11 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_PortCfgSet),
	/* Command: IFX_ETHSW_VLAN_ID_CREATE ; Index: 0x12 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_IdCreate),
	/* Command: IFX_ETHSW_VLAN_ID_DELETE ; Index: 0x13 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_IdDelete),
	/* Command: IFX_ETHSW_VLAN_PORT_MEMBER_ADD ; Index: 0x14 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_PortMemberAdd),
	/* Command: IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE ; Index: 0x15 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_PortMemberRemove),
	/* Command: IFX_ETHSW_VLAN_PORT_MEMBER_READ ; Index: 0x16 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_PortMemberRead),
	/* Command: IFX_ETHSW_VLAN_ID_GET ; Index: 0x17 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_VLAN_IdGet),
	/* Command: IFX_ETHSW_QOS_PORT_CFG_SET ; Index: 0x18 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_PortCfgSet),
	/* Command: IFX_ETHSW_QOS_PORT_CFG_GET ; Index: 0x19 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_PortCfgGet),
	/* Command: IFX_ETHSW_QOS_DSCP_CLASS_SET ; Index: 0x1A */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_DSCP_ClassSet),
	/* Command: IFX_ETHSW_QOS_DSCP_CLASS_GET ; Index: 0x1B */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_DSCP_ClassGet),
	/* Command: IFX_ETHSW_QOS_PCP_CLASS_SET ; Index: 0x1C */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_PCP_ClassSet),
	/* Command: IFX_ETHSW_QOS_PCP_CLASS_GET ; Index: 0x1D */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_PCP_ClassGet),
	/* Command: IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_SET ; Index: 0x1E */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_DSCP_DropPrecedenceCfgSet),
	/* Command: IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_GET ; Index: 0x1F */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_DSCP_DropPrecedenceCfgGet),
	/* Command: IFX_ETHSW_QOS_PORT_REMARKING_CFG_SET ; Index: 0x20 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_PortRemarkingCfgSet),
	/* Command: IFX_ETHSW_QOS_PORT_REMARKING_CFG_GET ; Index: 0x21 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_PortRemarkingCfgGet),
	/* Command: IFX_ETHSW_QOS_CLASS_DSCP_SET ; Index: 0x22 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ClassDSCP_Set),
	/* Command: IFX_ETHSW_QOS_CLASS_DSCP_GET ; Index: 0x23 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ClassDSCP_Get),
	/* Command: IFX_ETHSW_QOS_CLASS_PCP_SET ; Index: 0x24 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ClassPCP_Set),
	/* Command: IFX_ETHSW_QOS_CLASS_PCP_GET ; Index: 0x25 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ClassPCP_Get),
	/* Command: IFX_ETHSW_QOS_SHAPER_CFG_SET ; Index: 0x26 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ShaperCfgSet),
	/* Command: IFX_ETHSW_QOS_SHAPER_CFG_GET ; Index: 0x27 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ShaperCfgGet),
	/* Command: IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN ; Index: 0x28 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ShaperQueueAssign),
	/* Command: IFX_ETHSW_QOS_SHAPER_QUEUE_DEASSIGN ; Index: 0x29 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ShaperQueueDeassign),
	/* Command: IFX_ETHSW_QOS_SHAPER_QUEUE_GET ; Index: 0x2A */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_ShaperQueueGet),
	/* Command: IFX_ETHSW_QOS_WRED_CFG_SET ; Index: 0x2B */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_WredCfgSet),
	/* Command: IFX_ETHSW_QOS_WRED_CFG_GET ; Index: 0x2C */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_WredCfgGet),
	/* Command: IFX_ETHSW_QOS_WRED_QUEUE_CFG_SET ; Index: 0x2D */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_WredQueueCfgSet),
	/* Command: IFX_ETHSW_QOS_WRED_QUEUE_CFG_GET ; Index: 0x2E */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_WredQueueCfgGet),
	/* Command: IFX_ETHSW_QOS_METER_CFG_SET ; Index: 0x2F */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_MeterCfgSet),
	/* Command: IFX_ETHSW_QOS_METER_CFG_GET ; Index: 0x30 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_MeterCfgGet),
	/* Command: IFX_ETHSW_QOS_METER_PORT_ASSIGN ; Index: 0x31 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_MeterPortAssign),
	/* Command: IFX_ETHSW_QOS_METER_PORT_DEASSIGN ; Index: 0x32 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_MeterPortDeassign),
	/* Command: IFX_ETHSW_QOS_METER_PORT_GET ; Index: 0x33 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_MeterPortGet),
	/* Command: IFX_ETHSW_QOS_STORM_CFG_SET ; Index: 0x34 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_StormCfgSet),
	/* Command: IFX_ETHSW_QOS_STORM_CFG_GET ; Index: 0x35 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_StormCfgGet),
	/* Command: IFX_ETHSW_QOS_SCHEDULER_CFG_SET ; Index: 0x36 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_SchedulerCfgSet),
	/* Command: IFX_ETHSW_QOS_SCHEDULER_CFG_GET ; Index: 0x37 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_SchedulerCfgGet),
	/* Command: IFX_ETHSW_QOS_QUEUE_PORT_SET ; Index: 0x38 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_QueuePortSet),
	/* Command: IFX_ETHSW_QOS_QUEUE_PORT_GET ; Index: 0x39 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_QueuePortGet),
	/* Command: IFX_ETHSW_MULTICAST_SNOOP_CFG_SET ; Index: 0x3A */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastSnoopCfgSet),
	/* Command: IFX_ETHSW_MULTICAST_SNOOP_CFG_GET ; Index: 0x3B */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastSnoopCfgGet),
	/* Command: IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD ; Index: 0x3C */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastRouterPortAdd),
	/* Command: IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE ; Index: 0x3D */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastRouterPortRemove),
	/* Command: IFX_ETHSW_MULTICAST_ROUTER_PORT_READ ; Index: 0x3E */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastRouterPortRead),
	/* Command: IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD ; Index: 0x3F */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastTableEntryAdd),
	/* Command: IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE ; Index: 0x40 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastTableEntryRemove),
	/* Command: IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ ; Index: 0x41 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_MULTICAST , IFX_FLOW_MulticastTableEntryRead),
	/* Command: IFX_ETHSW_HW_INIT ; Index: 0x42 */
	(LTQ_ll_fkt) IFX_FLOW_HW_Init,
	/* Command: IFX_ETHSW_VERSION_GET ; Index: 0x43 */
	(LTQ_ll_fkt) IFX_FLOW_VersionGet,
	/* Command: IFX_ETHSW_CAP_GET ; Index: 0x44 */
	(LTQ_ll_fkt) IFX_FLOW_CapGet,
	/* Command: IFX_ETHSW_CFG_SET ; Index: 0x45 */
	(LTQ_ll_fkt) IFX_FLOW_CfgSet,
	/* Command: IFX_ETHSW_CFG_GET ; Index: 0x46 */
	(LTQ_ll_fkt) IFX_FLOW_CfgGet,
	/* Command: IFX_ETHSW_ENABLE ; Index: 0x47 */
	(LTQ_ll_fkt) IFX_FLOW_Enable,
	/* Command: IFX_ETHSW_DISABLE ; Index: 0x48 */
	(LTQ_ll_fkt) IFX_FLOW_Disable,
	/* Command: IFX_ETHSW_PORT_CFG_GET ; Index: 0x49 */
	(LTQ_ll_fkt) IFX_FLOW_PortCfgGet,
	/* Command: IFX_ETHSW_PORT_CFG_SET ; Index: 0x4A */
	(LTQ_ll_fkt) IFX_FLOW_PortCfgSet,
	/* Command: IFX_ETHSW_CPU_PORT_CFG_SET ; Index: 0x4B */
	(LTQ_ll_fkt) IFX_FLOW_CPU_PortCfgSet,
	/* Command: IFX_ETHSW_CPU_PORT_CFG_GET ; Index: 0x4C */
	(LTQ_ll_fkt) IFX_FLOW_CPU_PortCfgGet,
	/* Command: IFX_ETHSW_CPU_PORT_EXTEND_CFG_SET ; Index: 0x4D */
	(LTQ_ll_fkt) IFX_FLOW_CPU_PortExtendCfgSet,
	/* Command: IFX_ETHSW_CPU_PORT_EXTEND_CFG_GET ; Index: 0x4E */
	(LTQ_ll_fkt) IFX_FLOW_CPU_PortExtendCfgGet,
	/* Command: IFX_ETHSW_PORT_LINK_CFG_GET ; Index: 0x4F */
	(LTQ_ll_fkt) IFX_FLOW_PortLinkCfgGet,
	/* Command: IFX_ETHSW_PORT_LINK_CFG_SET ; Index: 0x50 */
	(LTQ_ll_fkt) IFX_FLOW_PortLinkCfgSet,
	/* Command: IFX_ETHSW_PORT_RGMII_CLK_CFG_SET ; Index: 0x51 */
	(LTQ_ll_fkt) IFX_FLOW_PortRGMII_ClkCfgSet,
	/* Command: IFX_ETHSW_PORT_RGMII_CLK_CFG_GET ; Index: 0x52 */
	(LTQ_ll_fkt) IFX_FLOW_PortRGMII_ClkCfgGet,
	/* Command: IFX_ETHSW_PORT_PHY_QUERY ; Index: 0x53 */
	(LTQ_ll_fkt) IFX_FLOW_PortPHY_Query,
	/* Command: IFX_ETHSW_PORT_PHY_ADDR_GET ; Index: 0x54 */
	(LTQ_ll_fkt) IFX_FLOW_PortPHY_AddrGet,
	/* Command: IFX_ETHSW_PORT_REDIRECT_GET ; Index: 0x55 */
	(LTQ_ll_fkt) IFX_FLOW_PortRedirectGet,
	/* Command: IFX_ETHSW_PORT_REDIRECT_SET ; Index: 0x56 */
	(LTQ_ll_fkt) IFX_FLOW_PortRedirectSet,
	/* Command: IFX_ETHSW_MONITOR_PORT_CFG_GET ; Index: 0x57 */
	(LTQ_ll_fkt) IFX_FLOW_MonitorPortCfgGet,
	/* Command: IFX_ETHSW_MONITOR_PORT_CFG_SET ; Index: 0x58 */
	(LTQ_ll_fkt) IFX_FLOW_MonitorPortCfgSet,
	/* Command: IFX_ETHSW_RMON_GET ; Index: 0x59 */
	(LTQ_ll_fkt) IFX_FLOW_RMON_Get,
	/* Command: IFX_ETHSW_RMON_CLEAR ; Index: 0x5A */
	(LTQ_ll_fkt) IFX_FLOW_RMON_Clear,
	/* Command: IFX_ETHSW_MDIO_CFG_GET ; Index: 0x5B */
	(LTQ_ll_fkt) IFX_FLOW_MDIO_CfgGet,
	/* Command: IFX_ETHSW_MDIO_CFG_SET ; Index: 0x5C */
	(LTQ_ll_fkt) IFX_FLOW_MDIO_CfgSet,
	/* Command: IFX_ETHSW_MDIO_DATA_READ ; Index: 0x5D */
	(LTQ_ll_fkt) IFX_FLOW_MDIO_DataRead,
	/* Command: IFX_ETHSW_MDIO_DATA_WRITE ; Index: 0x5E */
	(LTQ_ll_fkt) IFX_FLOW_MDIO_DataWrite,
   /* Command: IFX_ETHSW_MMD_DATA_READ ; Index: 0x5F */
	(LTQ_ll_fkt) IFX_FLOW_MmdDataRead,
	/* Command: IFX_ETHSW_MMD_DATA_WRITE ; Index: 0x60 */
	(LTQ_ll_fkt) IFX_FLOW_MmdDataWrite,
	/* Command: IFX_ETHSW_WOL_CFG_SET ; Index: 0x61 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_WOL , IFX_FLOW_WoL_CfgSet),
	/* Command: IFX_ETHSW_WOL_CFG_GET ; Index: 0x62 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_WOL , IFX_FLOW_WoL_CfgGet),
	/* Command: IFX_ETHSW_WOL_PORT_CFG_SET ; Index: 0x63 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_WOL , IFX_FLOW_WoL_PortCfgSet),
	/* Command: IFX_ETHSW_WOL_PORT_CFG_GET ; Index: 0x64 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_WOL , IFX_FLOW_WoL_PortCfgGet),
	/* Command: IFX_ETHSW_TRUNKING_CFG_GET ; Index: 0x65 */
	(LTQ_ll_fkt) IFX_FLOW_TrunkingCfgGet,
	/* Command: IFX_ETHSW_TRUNKING_CFG_SET ; Index: 0x66 */
	(LTQ_ll_fkt) IFX_FLOW_TrunkingCfgSet,
	/* Command: IFX_ETHSW_TRUNKING_PORT_CFG_GET ; Index: 0x67 */
	(LTQ_ll_fkt) IFX_FLOW_TrunkingPortCfgGet,
	/* Command: IFX_ETHSW_TRUNKING_PORT_CFG_SET ; Index: 0x68 */
	(LTQ_ll_fkt) IFX_FLOW_TrunkingPortCfgSet,
	/* Command: IFX_ETHSW_QOS_WRED_PORT_CFG_SET ; Index: 0x69 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_WredPortCfgSet),
	/* Command: IFX_ETHSW_QOS_WRED_PORT_CFG_GET ; Index: 0x6a */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_WredPortCfgGet),
	/* Command: IFX_ETHSW_QOS_FLOWCTRL_CFG_SET ; Index: 0x6b */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_FlowctrlCfgSet),
	/* Command: IFX_ETHSW_QOS_FLOWCTRL_CFG_GET ; Index: 0x6c */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_FlowctrlCfgGet),
	/* Command: IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET ; Index: 0x6d */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_FlowctrlPortCfgSet),
	/* Command: IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_GET ; Index: 0x6e */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_FlowctrlPortCfgGet),
	/* Command: IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET ; Index: 0x6f */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_QueueBufferReserveCfgSet),
	/* Command: IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET ; Index: 0x70 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_QOS , IFX_FLOW_QoS_QueueBufferReserveCfgGet),
	/* Command: IFX_ETHSW_SVLAN_CFG_GET ; Index: 0x71 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_SVLAN_CfgGet),
	/* Command: IFX_ETHSW_SVLAN_CFG_SET ; Index: 0x72 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_SVLAN_CfgSet),
	/* Command: IFX_ETHSW_SVLAN_PORT_CFG_GET ; Index: 0x73 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_SVLAN_PortCfgGet),
	/* Command: IFX_ETHSW_SVLAN_PORT_CFG_SET ; Index: 0x74 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_SVLAN_PortCfgSet),
	/* Command: IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_SET ; Index: 0x75 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_QoS_SVLAN_ClassPCP_PortSet),
	/* Command: IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_GET ; Index: 0x76 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_QoS_SVLAN_ClassPCP_PortGet),
	/* Command: IFX_ETHSW_QOS_SVLAN_PCP_CLASS_SET ; Index: 0x77 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_QoS_SVLAN_PCP_ClassSet),
	/* Command: IFX_ETHSW_QOS_SVLAN_PCP_CLASS_GET ; Index: 0x78 */
	(LTQ_ll_fkt) IFX_ETHSW_SW_FKT( CONFIG_LTQ_VLAN , IFX_FLOW_QoS_SVLAN_PCP_ClassGet),
};

const ltq_lowlevel_fkts_t ifx_ethsw_FLOW_fkt_tbl =
{
	NULL , /* pNext */
	(u16) IFX_ETHSW_MAGIC , /* nType */
	121 , /* nNumFkts */
	ltq_fkt_ptr_tbl /* pFkts */
};

const LTQ_ll_fkt gsw_flow_fkt_ptr_tbl [] =
{
	/* 0x00 */
	(LTQ_ll_fkt) NULL,
	/* Command: IFX_FLOW_REGISTER_SET ; Index: 0x01 */
	(LTQ_ll_fkt) IFX_FLOW_RegisterSet,
	/* Command: IFX_FLOW_REGISTER_GET ; Index: 0x02 */
	(LTQ_ll_fkt) IFX_FLOW_RegisterGet,
	/* Command: IFX_FLOW_IRQ_MASK_GET ; Index: 0x03 */
	(LTQ_ll_fkt) IFX_FLOW_IrqMaskGet,
	/* Command: IFX_FLOW_IRQ_MASK_SET ; Index: 0x04 */
	(LTQ_ll_fkt) IFX_FLOW_IrqMaskSet,
	/* Command: IFX_FLOW_IRQ_GET ; Index: 0x05 */
	(LTQ_ll_fkt) IFX_FLOW_IrqGet,
	/* Command: IFX_FLOW_IRQ_STATUS_CLEAR ; Index: 0x06 */
	(LTQ_ll_fkt) IFX_FLOW_IrqStatusClear,
	/* Command: IFX_FLOW_PCE_RULE_WRITE ; Index: 0x07 */
	(LTQ_ll_fkt) IFX_FLOW_PceRuleWrite,
	/* Command: IFX_FLOW_PCE_RULE_READ ; Index: 0x08 */
	(LTQ_ll_fkt) IFX_FLOW_PceRuleRead,
	/* Command: IFX_FLOW_PCE_RULE_DELETE ; Index: 0x09 */
	(LTQ_ll_fkt) IFX_FLOW_PceRuleDelete,
	/* Command: IFX_FLOW_RESET ; Index: 0x0A */
	(LTQ_ll_fkt) IFX_FLOW_Reset,
	/* Command: IFX_FLOW_RMON_EXTEND_GET ; Index: 0x0B */
	(LTQ_ll_fkt) IFX_FLOW_RMON_ExtendGet,
   /* Command: IFX_FLOW_TIMESTAMP_TIMER_SET ; Index: 0x0C */
   (LTQ_ll_fkt) IFX_FLOW_TimestampTimerSet,
   /* Command: IFX_FLOW_TIMESTAMP_TIMER_GET ; Index: 0x0D */
   (LTQ_ll_fkt) IFX_FLOW_TimestampTimerGet,
   /* Command: IFX_FLOW_TIMESTAMP_PORT_READ ; Index: 0x0E */
   (LTQ_ll_fkt) IFX_FLOW_TimestampPortRead,
};

const ltq_lowlevel_fkts_t ifx_flow_fkt_tbl =
{
	&ifx_ethsw_FLOW_fkt_tbl , /* pNext */
	(u16) IFX_FLOW_MAGIC , /* nType */
	15 , /* nNumFkts */
	gsw_flow_fkt_ptr_tbl /* pFkts */
};

