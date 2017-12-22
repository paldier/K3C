/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _LTQ_ETHSW_FLOW_LL_H_
#define _LTQ_ETHSW_FLOW_LL_H_

/* Group definitions for Doxygen */
/** \defgroup FLOW_LL Ethernet Switch Application Kernel Interface
    This chapter describes the entire interface to access and
    configure the services of the switch module in OS kernel space.*/
/*@{*/
/** \defgroup FLOW_LL_BRIDGE Ethernet Bridging Functions
    Ethernet bridging (or switching) is the basic task of the device. It
    provides individual configurations per port and standard global
    switch features.
*/
/** \defgroup FLOW_LL_CLASSUNIT Packet Classification Engine
    Configures and controls the classification unit of the XWAY VRX200
    and XWAY GRX200 Family hardware.
*/
/** \defgroup FLOW_LL_DEBUG Debug Features
    XWAY VRX200 and XWAY GRX200 Family specific features for system
    integration and debug sessions.
*/
/** \defgroup FLOW_LL_IRQ Interrupt Handling
    Configure XWAY VRX200 and XWAY GRX200 Family specific hardware
    support to generate interrupts
    and read out the interrupt sources.
*/
/** \defgroup FLOW_LL_MULTICAST Multicast Functions
    IGMP/MLD snooping configuration and support for IGMPv1, IGMPv2, IGMPv3,
    MLDv1, and MLDv2.
*/
/** \defgroup FLOW_LL_OAM Operation, Administration, and Management Functions
    This chapter summarizes the functions that are provided to monitor the
    data traffic passing through the device.
*/
/** \defgroup FLOW_LL_QOS Quality of Service Functions
    Switch and port configuration for Quality of Service (QoS).
*/
/** \defgroup FLOW_LL_VLAN VLAN Functions
    This chapter describes VLAN bridging functionality. This includes support for
    Customer VLAN Tags (CTAG VLAN) and also Service VLAN Tags (STAG VLAN/SVLAN).
*/
/** \defgroup GSWIP_ROUTE Operation, Administration, and Management Functions
    This chapter summarizes the functions that are provided to monitor the
    data traffic passing through the device.
*/
/*@}*/

/* ------------------------------------------------------------------------- */
/*                       Function Declaration                                */
/* ------------------------------------------------------------------------- */

/** \addtogroup FLOW_LL_BRIDGE */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_8021X_EAPOL_RULE_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_8021X_EAPOL_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_8021X_EAPOL_RuleGet(void *cdev,
	GSW_8021X_EAPOL_Rule_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_8021X_EAPOL_RULE_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_8021X_EAPOL_Rule_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_8021X_EAPOL_RuleSet(void *cdev,
	GSW_8021X_EAPOL_Rule_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_8021X_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      802.1x port authorized state port
      configuration \ref GSW_8021X_portCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_8021X_PortCfgGet(void *cdev,
	GSW_8021X_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_8021X_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      802.1x port authorized state port
      configuration \ref GSW_8021X_portCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_8021X_PortCfgSet(void *cdev,
	GSW_8021X_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MAC_TABLE_CLEAR command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableClear(void *cdev);

/**
   This is the switch API low-level function for
   the \ref GSW_MAC_TABLE_ENTRY_ADD command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableAdd_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryAdd(void *cdev,
	GSW_MAC_tableAdd_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MAC_TABLE_ENTRY_QUERY command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableQuery_t structure that is filled out by the switch
   implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryQuery(void *cdev,
	GSW_MAC_tableQuery_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MAC_TABLE_ENTRY_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableRead_t structure that is filled out by the switch
   implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryRead(void *cdev,
	GSW_MAC_tableRead_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MAC_TABLE_ENTRY_REMOVE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a MAC table entry
   \ref GSW_MAC_tableRemove_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MAC_TableEntryRemove(void *cdev,
	GSW_MAC_tableRemove_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_STP_BPDU_RULE_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_STP_BPDU_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_BPDU_RuleGet(void *cdev,
	GSW_STP_BPDU_Rule_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_STP_BPDU_RULE_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_STP_BPDU_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_BPDU_RuleSet(void *cdev,
	GSW_STP_BPDU_Rule_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_STP_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_STP_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_PortCfgGet(void *cdev,
	GSW_STP_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_STP_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_STP_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_STP_PortCfgSet(void *cdev,
	GSW_STP_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TRUNKING_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      configuration \ref GSW_trunkingCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TrunkingCfgGet(void *cdev,
	GSW_trunkingCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TRUNKING_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      configuration \ref GSW_trunkingCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TrunkingCfgSet(void *cdev,
	GSW_trunkingCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TRUNKING_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      configuration \ref GSW_trunkingPortCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TrunkingPortCfgGet(void *cdev,
	GSW_trunkingPortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TRUNKING_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      configuration \ref GSW_trunkingPortCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TrunkingPortCfgSet(void *cdev,
	GSW_trunkingPortCfg_t *parm);

/*@}*/ /* FLOW_LL_BRIDGE */
/** \addtogroup FLOW_LL_VLAN */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_SVLAN_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_SVLAN_cfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_SVLAN_CfgGet(void *cdev,
	GSW_SVLAN_cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_SVLAN_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_SVLAN_cfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_SVLAN_CfgSet(void *cdev,
	GSW_SVLAN_cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_SVLAN_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an
      \ref GSW_SVLAN_portCfg_t structure element. Based on the parameter
      'nPortId', the switch API implementation fills out the remaining structure
      elements.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_SVLAN_PortCfgGet(void *cdev,
	GSW_SVLAN_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_SVLAN_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_SVLAN_portCfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_SVLAN_PortCfgSet(void *cdev,
	GSW_SVLAN_portCfg_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_MEMBER_INIT command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_memberInit_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_Member_Init(void *cdev,
	GSW_VLAN_memberInit_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_ID_CREATE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_IdCreate_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_IdCreate(void *cdev,
	GSW_VLAN_IdCreate_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_ID_DELETE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_VLAN_IdDelete_t structure element.

	\remarks A VLAN ID can only be removed in case it was created by
		\ref GSW_VLAN_ID_CREATE and is currently not assigned
		to any Ethernet port (done using \ref GSW_VLAN_PORT_MEMBER_ADD).

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_IdDelete(void *cdev,
	GSW_VLAN_IdDelete_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_ID_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_VLAN_IdGet_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_IdGet(void *cdev,
	GSW_VLAN_IdGet_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an
      \ref GSW_VLAN_portCfg_t structure element. Based on the parameter
      'nPortId', the switch API implementation fills out the remaining structure
      elements.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_PortCfgGet(void *cdev,
	GSW_VLAN_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_VLAN_portCfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_PortCfgSet(void *cdev,
	GSW_VLAN_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_PORT_MEMBER_ADD command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_portMemberAdd_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_PortMemberAdd(void *cdev,
	GSW_VLAN_portMemberAdd_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_PORT_MEMBER_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_portMemberRead_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_PortMemberRead(void *cdev,
	GSW_VLAN_portMemberRead_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_PORT_MEMBER_REMOVE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_portMemberRemove_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_PortMemberRemove(void *cdev,
	GSW_VLAN_portMemberRemove_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_RESERVED_ADD command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_reserved_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_ReservedAdd(void *cdev,
	GSW_VLAN_reserved_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VLAN_RESERVED_REMOVE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_VLAN_reserved_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_VLAN_ReservedRemove(void *cdev,
	GSW_VLAN_reserved_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PCE_EG_VLAN_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_PCE_EgVLAN_Cfg_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PCE_EG_VLAN_CfgSet(void *cdev,
	GSW_PCE_EgVLAN_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PCE_EG_VLAN_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_PCE_EgVLAN_Cfg_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PCE_EG_VLAN_CfgGet(void *cdev,
	GSW_PCE_EgVLAN_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PCE_EG_VLAN_ENTRY_WRITE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_PCE_EgVLAN_Entry_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PCE_EG_VLAN_EntryWrite(void *cdev,
	GSW_PCE_EgVLAN_Entry_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PCE_EG_VLAN_ENTRY_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_PCE_EgVLAN_Entry_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PCE_EG_VLAN_EntryRead(void *cdev,
	GSW_PCE_EgVLAN_Entry_t *parm);

/*@}*/ /* FLOW_LL_VLAN */
/** \addtogroup FLOW_LL_QOS */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_QOS_CLASS_DSCP_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the DSCP mapping parameter
		\ref GSW_QoS_ClassDSCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ClassDSCP_Get(void *cdev,
	GSW_QoS_ClassDSCP_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_CLASS_DSCP_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the DSCP mapping parameter
   \ref GSW_QoS_ClassDSCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ClassDSCP_Set(void *cdev,
	GSW_QoS_ClassDSCP_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_CLASS_PCP_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the PCP priority mapping parameter
   \ref GSW_QoS_ClassPCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ClassPCP_Get(void *cdev,
	GSW_QoS_ClassPCP_Cfg_t *parm);

/**
	This is the switch API low-level function for
	the \ref GSW_QOS_CLASS_PCP_SET command.

	\param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the PCP priority mapping parameter
   \ref GSW_QoS_ClassPCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ClassPCP_Set(void *cdev,
	GSW_QoS_ClassPCP_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_DSCP_CLASS_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_DSCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_ClassGet(void *cdev,
	GSW_QoS_DSCP_ClassCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_DSCP_CLASS_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_DSCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_ClassSet(void *cdev,
	GSW_QoS_DSCP_ClassCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_DSCP_DROP_PRECEDENCE_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS
   DSCP drop precedence parameters
   \ref GSW_QoS_DSCP_DropPrecedenceCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgGet(void *cdev,
	GSW_QoS_DSCP_DropPrecedenceCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_DSCP_DROP_PRECEDENCE_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS
   DSCP drop precedence parameters
   \ref GSW_QoS_DSCP_DropPrecedenceCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgSet(void *cdev,
	GSW_QoS_DSCP_DropPrecedenceCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_FLOWCTRL_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlCfgGet(void *cdev,
	GSW_QoS_FlowCtrlCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_FLOWCTRL_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlCfgSet(void *cdev,
	GSW_QoS_FlowCtrlCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_FLOWCTRL_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlPortCfgGet(void *cdev,
	GSW_QoS_FlowCtrlPortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_FLOWCTRL_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_FlowctrlPortCfgSet(void *cdev,
	GSW_QoS_FlowCtrlPortCfg_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_QOS_METER_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterCfgGet(void *cdev,
	GSW_QoS_meterCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_METER_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterCfgSet(void *cdev,
	GSW_QoS_meterCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_METER_PORT_ASSIGN command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterPort_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterPortAssign(void *cdev,
	GSW_QoS_meterPort_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_METER_PORT_DEASSIGN command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterPort_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterPortDeassign(void *cdev,
	GSW_QoS_meterPort_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_METER_PORT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_meterPortGet_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_MeterPortGet(void *cdev,
	GSW_QoS_meterPortGet_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_PCP_CLASS_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PCP_ClassGet(void *cdev,
	GSW_QoS_PCP_ClassCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_PCP_CLASS_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PCP_ClassSet(void *cdev,
	GSW_QoS_PCP_ClassCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      QOS port priority control configuration \ref GSW_QoS_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortCfgGet(void *cdev,
	GSW_QoS_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a
      QOS port priority control configuration \ref GSW_QoS_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortCfgSet(void *cdev,
	GSW_QoS_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_PORT_REMARKING_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_portRemarkingCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortRemarkingCfgGet(void *cdev,
	GSW_QoS_portRemarkingCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_PORT_REMARKING_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the remarking configuration
   \ref GSW_QoS_portRemarkingCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_PortRemarkingCfgSet(void *cdev,
	GSW_QoS_portRemarkingCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueueBufferReserveCfgGet(void *cdev,
	GSW_QoS_QueueBufferReserveCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueueBufferReserveCfgSet(void *cdev,
	GSW_QoS_QueueBufferReserveCfg_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_QOS_QUEUE_PORT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_queuePort_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueuePortGet(void *cdev,
	GSW_QoS_queuePort_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_QUEUE_PORT_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_queuePort_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_QueuePortSet(void *cdev,
	GSW_QoS_queuePort_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SVLAN_CLASS_PCP_PORT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the PCP priority mapping parameter
   \ref GSW_QoS_SVLAN_ClassPCP_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SVLAN_ClassPCP_PortGet(void *cdev,
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SVLAN_CLASS_PCP_PORT_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the PCP priority mapping parameter
   \ref GSW_QoS_SVLAN_ClassPCP_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SVLAN_ClassPCP_PortSet(void *cdev,
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SVLAN_PCP_CLASS_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_SVLAN_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SVLAN_PCP_ClassGet(void *cdev,
	GSW_QoS_SVLAN_PCP_ClassCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SVLAN_PCP_CLASS_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the QoS filter parameters
   \ref GSW_QoS_SVLAN_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SVLAN_PCP_ClassSet(void *cdev,
	GSW_QoS_SVLAN_PCP_ClassCfg_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SCHEDULER_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SchedulerCfgGet(void *cdev,
	GSW_QoS_schedulerCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SCHEDULER_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_SchedulerCfgSet(void *cdev,
	GSW_QoS_schedulerCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SHAPER_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperCfgGet(void *cdev,
	GSW_QoS_ShaperCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SHAPER_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperCfgSet(void *cdev,
	GSW_QoS_ShaperCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SHAPER_QUEUE_ASSIGN command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperQueueAssign(void *cdev,
	GSW_QoS_ShaperQueue_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SHAPER_QUEUE_DEASSIGN command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperQueueDeassign(void *cdev,
	GSW_QoS_ShaperQueue_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_SHAPER_QUEUE_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_ShaperQueueGet_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_ShaperQueueGet(void *cdev,
	GSW_QoS_ShaperQueueGet_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_STORM_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_stormCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_StormCfgGet(void *cdev, GSW_QoS_stormCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_STORM_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_stormCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_StormCfgSet(void *cdev, GSW_QoS_stormCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_WRED_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredCfgGet(void *cdev, GSW_QoS_WRED_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_WRED_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredCfgSet(void *cdev, GSW_QoS_WRED_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_WRED_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredPortCfgGet(void *cdev,
	GSW_QoS_WRED_PortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_WRED_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredPortCfgSet(void *cdev,
	GSW_QoS_WRED_PortCfg_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_QOS_WRED_QUEUE_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredQueueCfgGet(void *cdev,
	GSW_QoS_WRED_QueueCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_WRED_QUEUE_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the parameters
   structure \ref GSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_WredQueueCfgSet(void *cdev,
	GSW_QoS_WRED_QueueCfg_t *parm);

/*@}*/ /* FLOW_LL_QOS */
/** \addtogroup FLOW_LL_MULTICAST */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_ROUTER_PORT_ADD command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_multicastRouter_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastRouterPortAdd(void *cdev,
	GSW_multicastRouter_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_ROUTER_PORT_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_multicastRouterRead_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
   (e.g. Ethernet port parameter out of range)
*/
GSW_return_t GSW_MulticastRouterPortRead(void *cdev,
	GSW_multicastRouterRead_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_ROUTER_PORT_REMOVE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_multicastRouter_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
   (e.g. Ethernet port parameter out of range)
*/
GSW_return_t GSW_MulticastRouterPortRemove(void *cdev,
	GSW_multicastRouter_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_SNOOP_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the
   multicast configuration \ref GSW_multicastSnoopCfg_t.

   \remarks IGMP/MLD snooping is disabled when
   'eIGMP_Mode = GSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'.
   Then all other structure parameters are unused.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastSnoopCfgGet(void *cdev,
	GSW_multicastSnoopCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_SNOOP_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to the
   multicast configuration \ref GSW_multicastSnoopCfg_t.

   \remarks IGMP/MLD snooping is disabled when
   'eIGMP_Mode = GSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'.
   Then all other structure parameters are unused.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastSnoopCfgSet(void *cdev,
	GSW_multicastSnoopCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_TABLE_ENTRY_ADD command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer
      to \ref GSW_multicastTable_t.

	\remarks The Source IP parameter is ignored in case IGMPv3 support is
		not enabled in the hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryAdd(void *cdev,
	GSW_multicastTable_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_TABLE_ENTRY_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer
      to \ref GSW_multicastTableRead_t.

	\remarks The 'bInitial' parameter is reset during the read operation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryRead(void *cdev,
	GSW_multicastTableRead_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MULTICAST_TABLE_ENTRY_REMOVE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer
      to \ref GSW_multicastTable_t.

	\remarks The Source IP parameter is ignored in case
	IGMPv3 support is not enabled in the hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MulticastTableEntryRemove(void *cdev,
	GSW_multicastTable_t *parm);

/*@}*/ /* FLOW_LL_MULTICAST */
/** \addtogroup FLOW_LL_OAM */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_CPU_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this specialinstance of the device.
   \param parm Pointer to
      an \ref GSW_CPU_PortCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortCfgGet(void *cdev, GSW_CPU_PortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_CPU_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_CPU_PortCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortCfgSet(void *cdev, GSW_CPU_PortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_CPU_PORT_EXTEND_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_CPU_PortExtendCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_CPU_PortExtendCfgGet(void *cdev,
	GSW_CPU_PortExtendCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_CPU_PORT_EXTEND_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_CPU_PortExtendCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CPU_PortExtendCfgSet(void *cdev,
	GSW_CPU_PortExtendCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_CAP_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to pre-allocated capability
      list structure \ref GSW_cap_t.
      The switch API implementation fills out the structure with the supported
      features, based on the provided 'nCapType' parameter.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_CapGet(void *cdev, GSW_cap_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_cfg_t structure.
      The structure is filled out by the switch implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CfgGet(void *cdev, GSW_cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_CfgSet(void *cdev, GSW_cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_DISABLE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Disable(void *cdev);

/**
   This is the switch API low-level function for
   the \ref GSW_ENABLE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_Enable(void *cdev);

/**
   This is the switch API low-level function for
   the \ref GSW_HW_INIT command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to pre-allocated initialization structure
   \ref GSW_HW_Init_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_HW_Init(void *cdev, GSW_HW_Init_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MDIO_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_MDIO_cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MDIO_CfgGet(void *cdev, GSW_MDIO_cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MDIO_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_MDIO_cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MDIO_CfgSet(void *cdev, GSW_MDIO_cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MDIO_DATA_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_MDIO_data_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MDIO_DataRead(void *cdev, GSW_MDIO_data_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MDIO_DATA_WRITE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_MDIO_data_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_MDIO_DataWrite(void *cdev, GSW_MDIO_data_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MMD_DATA_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_MMD_data_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MmdDataRead(void *cdev, GSW_MMD_data_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MMD_DATA_WRITE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_MMD_data_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MmdDataWrite(void *cdev, GSW_MMD_data_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MONITOR_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_monitorPortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MonitorPortCfgGet(void *cdev, GSW_monitorPortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_MONITOR_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_monitorPortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_MonitorPortCfgSet(void *cdev, GSW_monitorPortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to a port configuration
   \ref GSW_portCfg_t structure to fill out by the driver.
   The parameter 'nPortId' tells the driver which port parameter is requested.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortCfgGet(void *cdev, GSW_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_portCfg_t structure
   to configure the switch port hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortCfgSet(void *cdev, GSW_portCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_LINK_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_portLinkCfg_t structure to read out the port status.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortLinkCfgGet(void *cdev, GSW_portLinkCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_LINK_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_portLinkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortLinkCfgSet(void *cdev, GSW_portLinkCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_PHY_ADDR_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_portPHY_Addr_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortPHY_AddrGet(void *cdev, GSW_portPHY_Addr_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_PHY_QUERY command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_portPHY_Query_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortPHY_Query(void *cdev, GSW_portPHY_Query_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_RGMII_CLK_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_portRGMII_ClkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortRGMII_ClkCfgGet(void *cdev, GSW_portRGMII_ClkCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_RGMII_CLK_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_portRGMII_ClkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortRGMII_ClkCfgSet(void *cdev, GSW_portRGMII_ClkCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_REDIRECT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_portRedirectCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.
	\remarks Not all hardware platforms support this feature. The function
		returns an error if this feature is not supported.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortRedirectGet(void *cdev, GSW_portRedirectCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PORT_REDIRECT_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_portRedirectCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.
	\remarks Not all hardware platforms support this feature. The function
		returns an error if this feature is not supported.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PortRedirectSet(void *cdev, GSW_portRedirectCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_CLEAR command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_clear_t structure. The structure element 'nPortId' is
   an input parameter stating on which port to clear all RMON counters.

   \remarks The function returns an error in case the given 'nPortId' is
   out of range.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Clear(void *cdev, GSW_RMON_clear_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_PORT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm  Pointer to pre-allocated
   \ref GSW_RMON_Port_cnt_t structure. The structure element 'nPortId' is
   an input parameter that describes from which port to read the RMON counter.
   All remaining structure elements are filled with the counter values.

   \remarks The function returns an error in case the given 'nPortId' is
   out of range.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Port_Get(void *cdev, GSW_RMON_Port_cnt_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_VERSION_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm* The parameter points to a
   \ref GSW_version_t structure.

   \return Returns value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_VersionGet(void *cdev, GSW_version_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_WOL_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_WoL_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_WoL_CfgGet(void *cdev, GSW_WoL_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_WOL_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_WoL_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_WoL_CfgSet(void *cdev, GSW_WoL_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_WOL_PORT_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_WoL_PortCfg_t.

   \remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_WoL_PortCfgGet(void *cdev, GSW_WoL_PortCfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_WOL_PORT_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_WoL_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_WoL_PortCfgSet(void *cdev, GSW_WoL_PortCfg_t *parm);

/*@}*/ /* FLOW_LL_OAM */
/** \addtogroup FLOW_LL_DEBUG */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_REGISTER_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_register_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RegisterGet(void *cdev, GSW_register_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_REGISTER_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_register_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RegisterSet(void *cdev, GSW_register_t *parm);

/*@}*/ /* FLOW_LL_DEBUG */
/** \addtogroup FLOW_LL_IRQ */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_IRQ_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_IrqGet(void *cdev, GSW_irq_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_IRQ_MASK_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_IrqMaskGet(void *cdev, GSW_irq_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_IRQ_MASK_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_IrqMaskSet(void *cdev, GSW_irq_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_IRQ_STATUS_CLEAR command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

*/
GSW_return_t GSW_IrqStatusClear(void *cdev, GSW_irq_t *parm);

/*@}*/ /* FLOW_LL_IRQ */
/** \addtogroup FLOW_LL_CLASSUNIT */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_PCE_RULE_DELETE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_PCE_ruleDelete_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PceRuleDelete(void *cdev, GSW_PCE_ruleDelete_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PCE_RULE_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_PCE_rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PceRuleRead(void *cdev, GSW_PCE_rule_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PCE_RULE_WRITE command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to \ref GSW_PCE_rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PceRuleWrite(void *cdev, GSW_PCE_rule_t *parm);

/*@}*/ /* FLOW_LL_CLASSUNIT */
/** \addtogroup FLOW_LL_OAM */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref GSW_RMON_EXTEND_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm  Pointer to a pre-allocated
   \ref GSW_RMON_extendGet_t structure. The structure element 'nPortId' is
   an input parameter that describes from which port to read the RMON counter.
   All remaining structure elements are filled with the counter values.
   The counter assignment needs to be done during the flow definition,
   for example in \ref GSW_PCE_RULE_WRITE.

	\remarks The function returns an error in case the given 'nPortId' is
		out of range.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_ExtendGet(void *cdev, GSW_RMON_extendGet_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RESET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_reset_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs

   \remarks Not supported for all devices
*/
GSW_return_t GSW_Reset(void *cdev, GSW_reset_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TIMESTAMP_PORT_READ command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to
      an \ref GSW_TIMESTAMP_PortRead_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TimestampPortRead(void *cdev, GSW_TIMESTAMP_PortRead_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TIMESTAMP_TIMER_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_TIMESTAMP_Timer_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TimestampTimerGet(void *cdev, GSW_TIMESTAMP_Timer_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_TIMESTAMP_TIMER_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_TIMESTAMP_Timer_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_TimestampTimerSet(void *cdev, GSW_TIMESTAMP_Timer_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_QOS_METER_ACT command (GSWIP-3.0 only).

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_QoS_mtrAction_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_QoS_Meter_Act(void *cdev, GSW_QoS_mtrAction_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_MODE_SET command (GSWIP-3.0 only)

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_RMON_mode_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Mode_Set(void *cdev, GSW_RMON_mode_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_METER_GET command (GSWIP-3.0 only)

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_RMON_Meter_cnt_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Meter_Get(void *cdev, GSW_RMON_Meter_cnt_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_REDIRECT_GET command (GSWIP-3.0 only)

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_RMON_Redirect_cnt_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Redirect_Get(void *cdev, GSW_RMON_Redirect_cnt_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_IF_GET command (GSWIP-3.0 only)

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_RMON_If_cnt_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_If_Get(void *cdev, GSW_RMON_If_cnt_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_RMON_ROUTE_GET command (GSWIP-3.0 only)

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_RMON_Route_cnt_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_RMON_Route_Get(void *cdev, GSW_RMON_Route_cnt_t *parm);
/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_IG_COUNT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Ig_Cnt_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_CountGet(void *cdev, GSW_PMAC_Cnt_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_EG_COUNT_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Eg_Cnt_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
/*GSW_return_t GSW_PMAC_EG_CountGet(void *cdev, GSW_PMAC_Eg_Cnt_t *parm);*/
/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_BM_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_BM_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_BM_CfgSet(void *cdev, GSW_PMAC_BM_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_BM_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_BM_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_BM_CfgGet(void *cdev, GSW_PMAC_BM_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_IG_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Ig_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_IG_CfgSet(void *cdev, GSW_PMAC_Ig_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_EG_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Ig_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_IG_CfgGet(void *cdev, GSW_PMAC_Ig_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_EG_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Eg_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_EG_CfgSet(void *cdev, GSW_PMAC_Eg_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_EG_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Eg_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_EG_CfgGet(void *cdev, GSW_PMAC_Eg_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_GLBL_CFG_SET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Glbl_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_GLBL_CfgSet(void *cdev, GSW_PMAC_Glbl_Cfg_t *parm);

/**
   This is the switch API low-level function for
   the \ref GSW_PMAC_GLBL_CFG_GET command.

   \param cdev This parameter is a pointer to the device context
   which contains all information related to this special
   instance of the device.
   \param parm Pointer to an \ref GSW_PMAC_Glbl_Cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref GSW_status_t.

   \return Return value as follows:
   - GSW_statusOk: if successful
   - An error code in case an error occurs
*/
GSW_return_t GSW_PMAC_GLBL_CfgGet(void *cdev, GSW_PMAC_Glbl_Cfg_t *parm);

/*@}*/ /* FLOW_LL_OAM */

/** \addtogroup GSWIP_ROUTE */
/*@{*/
/**
   \brief This function creates a Routing Session entry
   in the Routing-Session table.
   The pattern part describes the five tuple serving
   as input key on which hash computation should be
   done on an incoming packet to which the dedicated
   actions should be applied.
   A rule can be deleted using the command
   \ref GSW_ROUTE_SessionEntryDel or read using
   the command \ref GSW_ROUTE_SessionEntryRead.
   \param[in] cdev device context
   \param[in, out] pRtEntry Pointer to Routing Entry
   structure \ref GSW_ROUTE_Entry_t. The nHashVal is optional
   and can be supplied (-1). The nPrio field is carrying the
   priority information - 1 (Priority session), 0  (Normal session).
   Upon return of API, nHashVal and nRtIndex carries back
   computed hash value and Index location. In case of swap
   of an existent session by a high priority new session,
   the existing session that got removed from acceleration
   is returned back in routeEntry struct member. The swap
   done is informed through nFag member carrying special value(1).
   \return Return value as follows:
   - Routing session index number >=0 : if successful
   - An error code < 0 in case an error occurs. There has
   to be detailed error codes covering the maximum reasons :
   - E.g. Collision List is full, RT_Table full, PPPoE_table
   full, MTU table full etc.
*/
int GSW_ROUTE_SessionEntryAdd(void *cdev, GSW_ROUTE_Entry_t *pRtEntry);

/**
   \brief This function deletes a Routing Session
   entry at specififed index in the Routing-Session table.
   A rule can be created using the command
   \ref GSW_ROUTE_SessionEntryAdd
   \param[in] cdev device context
   \param[in] pRtEntry Routing Session Entry carrying
   mandatory nRtIndex value.
   \return Return value as follows:
   - GSW_SUCCESS : if successful
   - An error code < 0 in case an error occurs.
*/
int GSW_ROUTE_SessionEntryDel(void *cdev, GSW_ROUTE_Entry_t *pRtEntry);

/**
   \brief This function reads a session entry in the
   Routing session table at an specified index. The index
   must be valid entry of the routing index table.
   \param[in] cdev device context.
   \param[in,out] pRtEntry  Pointer to Routing Session
   Entry structure \ref GSW_ROUTE_Entry_t.
   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code < 0, in case an error occurs
*/
int GSW_ROUTE_SessionEntryRead(void *cdev, GSW_ROUTE_Entry_t *pRtEntry);

/**
   \brief This function creates a tunnel entry in the
   Tunnel table. For complete configuration of tunnel,
   it is a multi-step config. Besides tunnel entry creation
   in tunnel table, it should also be programmed in
   RoutingSession table.
   A configured tunnel entry can be read using the command
   \ref GSW_ROUTE_TunnelEntryRead
   \param[in] cdev device context
   \param[in] pTunnel Pointer to Tunnel structure
   \ref GSW_ROUTE_Tunnel_Entry_t.
   \return Return value as follows:
   - Tunnel Entry index number >=0 : if successful
   (Number of Tunnels supported are 16 so return value is 0..15 range)
   - An error code < 0 in case an error occurs
*/
int GSW_ROUTE_TunnelEntryAdd(void *cdev, GSW_ROUTE_Tunnel_Entry_t *pTunnel);

/**
   \brief This function deletes a tunnel entry in the
   Tunnel table.
   \param[in] cdev device context
   \param[in] nTunIdx number, where the Tunnel entry is
   stored in the Tunnel Table.
   \return Return value as follows:
   - Tunnel Entry index number >=0 : if successful
   - An error code < 0 in case an error occurs
*/
int GSW_ROUTE_TunnelEntryDel(void *cdev, GSW_ROUTE_Tunnel_Entry_t *pTunnel);

/**
   \brief This function reads a tunnel entry in the Tunnel
   table at an specified index. The index must be valid
   entry of the Tunnel index table.
   \param[in] cdev device context.
   \param[out] pTunnel Pointer to Tunnel structure
   \ref GSW_ROUTE_Tunnel_Entry_t.
   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code in case an error occurs
*/
int GSW_ROUTE_TunnelEntryRead(void *cdev, GSW_ROUTE_Tunnel_Entry_t *pTunnel);

/**
   \brief This function configures a Source L2NAT on an egress port.
   A configured tunnel entry can be read using the command
   \ref GSW_ROUTE_TunnelEntryRead
   \param[in] cdev device context
   \param[in] pL2NatCfg Pointer to Tunnel structure
   \ref GSW_ROUTE_EgPort_L2NAT_Cfg_t.

   \return Return value as follows:
   - Tunnel Entry index number >=0 : if successful
   - An error code < 0 in case an error occurs
*/
int GSW_ROUTE_L2NATCfgWrite(void *cdev,
	GSW_ROUTE_EgPort_L2NAT_Cfg_t *pL2NatCfg);

/**
   \brief This function reads currently configured L2NAT entry in
   the Tunnel table for the specified port. The port
   number must be a valid number.
   \param[in] cdev device context.
   \param[in,out] pL2NatCfg Pointer to L2NAT Config structure
   \ref GSW_ROUTE_EgPort_L2NAT_Cfg_t. The port number must
   be filled in this structure.
   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code in case an error occurs
*/
int GSW_ROUTE_L2NATCfgRead(void *cdev,
	GSW_ROUTE_EgPort_L2NAT_Cfg_t *pL2NatCfg);

/**
   \brief This function reads or reads-n-clears Session Hit
   Sttaus for the specified index.
   \param[in] cdev device context.
   \param[in,out] pHitOp Pointer to Session-Hit structure
   \ref GSW_ROUTE_Session_Hit_t. The index number must be
   filled in this structure.
   \return Return value as follows:
   - 0: Showing Session is not Hit.
   - 1: Showing Session is Hit.
   - (-1): In case of any error in reading or writing Session Hit Status.
   - An error code in case an error occurs
*/
int GSW_ROUTE_SessHitOp(void *cdev, GSW_ROUTE_Session_Hit_t *pHitOp);

/**
   \brief This function modifies the destination ports of Routing Session.
   \param[in] cdev device context.
   \param[in,out] pDestCfg Pointer to destination structure
   \ref GSW_ROUTE_Session_Dest_t.
   \return Return value as follows:
   - GSW_SUCCESS : if successful.
   - An error code in case an error occurs
*/
int GSW_ROUTE_SessDestModify(void *cdev, GSW_ROUTE_Session_Dest_t *pDestCfg);

/*@}*/ /* GSWIP_ROUTE */

#endif /* _LTQ_ETHSW_FLOW_LL_H_ */
