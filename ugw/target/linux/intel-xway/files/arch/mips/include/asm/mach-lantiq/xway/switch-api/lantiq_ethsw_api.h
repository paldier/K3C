/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
 
#ifndef _LTQ_ETHSW_KERNEL_API_H_
#define _LTQ_ETHSW_KERNEL_API_H_

#include "lantiq_ethsw.h"
#include "lantiq_ethsw_flow.h"
/* Group definitions for Doxygen */
/** \defgroup ETHSW_KERNELAPI Ethernet Switch Linux Kernel Interface
    This chapter describes the entire interface to access and
    configure the services of the Ethernet switch module
    within the Linux kernel space. */

/*@{*/

/** Definition of the device handle that is retrieved during
    the \ref ifx_ethsw_kopen call. This handle is used to access the switch
    device while calling \ref ifx_ethsw_kioctl. */
typedef unsigned int LTQ_ETHSW_API_HANDLE;

/**
   Request a device handle for a dedicated Ethernet switch device. The switch
   device is identified by the given device name (e.g. "/dev/switch/1").
   The device handle is the return value of this function. This handle is
   used to access the switch parameter and features while
   calling \ref ifx_ethsw_kioctl. Please call the function
   \ref ifx_ethsw_kclose to release a device handle that is not needed anymore.

   \param name Pointer to the device name of the requested Ethernet switch device.

   \remarks The client kernel module should check the function return value.
   A returned zero indicates that the resource allocation failed.

   \return Return the device handle in case the requested device is available.
   It returns a zero in case the device does not exist or is blocked
   by another application.
*/
LTQ_ETHSW_API_HANDLE ltq_ethsw_api_kopen(char *name);

/**
   Calls the switch API driver implementation with the given command and the
   parameter argument. The called Ethernet switch device is identified by the
   given device handle. This handle was previously requested by
   calling \ref ifx_ethsw_kopen.

   \param handle Ethernet switch device handle, given by \ref ifx_ethsw_kopen.
   \param command Switch API command to perform.
   \param arg Command arguments. This argument is basically a reference to
   the command parameter structure.

   \remarks The commands and arguments are the same as normally used over
   the Linux ioctl interface from user space.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurred.
*/
int ltq_ethsw_api_kioctl(LTQ_ETHSW_API_HANDLE handle, unsigned int command, unsigned int arg);

/**
   Releases an Ethernet switch device handle which was previously
   allocated by \ref ifx_ethsw_kopen.

   \param handle Ethernet switch device handle, given by \ref ifx_ethsw_kopen.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurred.
*/
int ltq_ethsw_api_kclose(LTQ_ETHSW_API_HANDLE handle);

/*@}*/
union ifx_sw_param
{
	/* Ethernet Bridging Functions*/
	IFX_ETHSW_MAC_tableRead_t	MAC_tableRead;
	IFX_ETHSW_MAC_tableQuery_t	mac_query;
	IFX_ETHSW_MAC_tableAdd_t	MAC_tableAdd;
	IFX_ETHSW_MAC_tableRemove_t		MAC_tableRemove;
	IFX_ETHSW_portCfg_t						portcfg;
	IFX_ETHSW_STP_portCfg_t				STP_portCfg;
	IFX_ETHSW_STP_BPDU_Rule_t			STP_BPDU_Rule;
	IFX_ETHSW_8021X_portCfg_t	PNAC_portCfg;
	IFX_ETHSW_8021X_EAPOL_Rule_t	PNAC_EAPOL_Rule;
	/* VLAN Functions */
	IFX_ETHSW_VLAN_IdCreate_t			vlan_IdCreate;
	IFX_ETHSW_VLAN_IdDelete_t			vlan_IdDelete;
	IFX_ETHSW_VLAN_IdGet_t				vlan_IdGet;
	IFX_ETHSW_VLAN_portCfg_t			vlan_portcfg;
	IFX_ETHSW_VLAN_portMemberAdd_t	vlan_portMemberAdd;
	IFX_ETHSW_VLAN_portMemberRead_t	vlan_portMemberRead;
	IFX_ETHSW_VLAN_portMemberRemove_t	vlan_portMemberRemove;
	IFX_ETHSW_VLAN_reserved_t			vlan_Reserved;
	IFX_ETHSW_VLAN_IdGet_t				vlan_VidFid;
	/* Operation, Administration, and Management Functions */
	IFX_ETHSW_cfg_t					cfg_Data;
	IFX_ETHSW_MDIO_cfg_t		mdio_cfg;
	IFX_ETHSW_MDIO_data_t		mdio_Data;
	IFX_ETHSW_portLinkCfg_t	portlinkcfgGet;
	IFX_ETHSW_portLinkCfg_t	portlinkcfgSet;
	IFX_ETHSW_portPHY_Addr_t	phy_addr;
	IFX_ETHSW_portRGMII_ClkCfg_t	portRGMII_clkcfg;
 	IFX_ETHSW_CPU_PortExtendCfg_t	portextendcfg;
	IFX_ETHSW_portRedirectCfg_t	portRedirectData;
	IFX_ETHSW_RMON_clear_t		RMON_clear;
	IFX_ETHSW_RMON_cnt_t		RMON_cnt;
	IFX_FLOW_RMON_extendGet_t	RMON_ExtendGet;
	IFX_ETHSW_monitorPortCfg_t	monitorPortCfg;
	IFX_ETHSW_cap_t	cap;
	IFX_ETHSW_portPHY_Query_t	phy_Query;
	IFX_ETHSW_CPU_PortCfg_t	CPU_PortCfg;
	IFX_ETHSW_version_t	Version;
	IFX_ETHSW_HW_Init_t	HW_Init;
	/* Multicast Functions */
	IFX_ETHSW_multicastRouter_t	multicast_RouterPortAdd;
	IFX_ETHSW_multicastRouter_t	multicast_RouterPortRemove;
	IFX_ETHSW_multicastRouterRead_t	multicast_RouterPortRead;
	IFX_ETHSW_multicastTable_t	multicast_TableEntryAdd;
	IFX_ETHSW_multicastTable_t	multicast_TableEntryRemove;
	IFX_ETHSW_multicastTableRead_t	multicast_TableEntryRead;
	IFX_ETHSW_multicastSnoopCfg_t	multicast_SnoopCfgSet;
	IFX_ETHSW_multicastSnoopCfg_t	multicast_SnoopCfgGet;
	/* Quality of Service Functions */
	IFX_ETHSW_QoS_portCfg_t		qos_portcfg;
	IFX_ETHSW_QoS_queuePort_t	qos_queueport;
	IFX_ETHSW_QoS_DSCP_ClassCfg_t	qos_dscpclasscfgget;
	IFX_ETHSW_QoS_DSCP_ClassCfg_t	qos_dscpclasscfgset;
	IFX_ETHSW_QoS_PCP_ClassCfg_t	qos_pcpclasscfgget;
	IFX_ETHSW_QoS_PCP_ClassCfg_t	qos_pcpclasscfgset;
	IFX_ETHSW_QoS_ClassDSCP_Cfg_t	qos_classdscpcfgget;
	IFX_ETHSW_QoS_ClassDSCP_Cfg_t	qos_classdscpcfgset;
	IFX_ETHSW_QoS_ClassPCP_Cfg_t	qos_classpcpcfgget;
	IFX_ETHSW_QoS_ClassPCP_Cfg_t	qos_classpcpcfgset;
	IFX_ETHSW_QoS_ShaperCfg_t		qos_shappercfg;
	IFX_ETHSW_QoS_ShaperQueue_t	qos_shapperqueue;
	IFX_ETHSW_QoS_stormCfg_t		qos_stormcfg;
	IFX_ETHSW_QoS_schedulerCfg_t	qos_schedulecfg;
	IFX_ETHSW_QoS_WRED_Cfg_t	qos_wredcfg;
	IFX_ETHSW_QoS_WRED_QueueCfg_t	qos_wredqueuecfg;
	IFX_ETHSW_QoS_meterCfg_t	qos_metercfg;
	IFX_ETHSW_QoS_meterPort_t	qos_meterport;
	IFX_ETHSW_QoS_meterPortGet_t	qos_meterportget;
	IFX_ETHSW_QoS_portRemarkingCfg_t	qos_portremarking;
	/* Packet Classification Engine */
	IFX_FLOW_PCE_rule_t				pce_rule;
	IFX_FLOW_PCE_ruleDelete_t	pce_ruledelete;
	IFX_FLOW_register_t		register_access;
};
#endif /* _LTQ_ETHSW_KERNEL_API_H_ */
