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
    configure the services of the switch module in OS kernel space. */
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
/*@}*/

/* ------------------------------------------------------------------------- */
/*                       Function Declaration                                */
/* ------------------------------------------------------------------------- */

/** \addtogroup FLOW_LL_BRIDGE */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_8021X_EAPOL_RULE_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_8021X_EAPOL_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_8021X_EAPOL_RuleGet(void *pDevCtx, IFX_ETHSW_8021X_EAPOL_Rule_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_8021X_EAPOL_RULE_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_8021X_EAPOL_Rule_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_8021X_EAPOL_RuleSet(void *pDevCtx, IFX_ETHSW_8021X_EAPOL_Rule_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_8021X_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      802.1x port authorized state port
      configuration \ref IFX_ETHSW_8021X_portCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_8021X_PortCfgGet(void *pDevCtx, IFX_ETHSW_8021X_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_8021X_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      802.1x port authorized state port
      configuration \ref IFX_ETHSW_8021X_portCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_8021X_PortCfgSet(void *pDevCtx, IFX_ETHSW_8021X_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MAC_TABLE_CLEAR command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MAC_TableClear(void *pDevCtx);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MAC_TABLE_ENTRY_ADD command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableAdd_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MAC_TableEntryAdd(void *pDevCtx, IFX_ETHSW_MAC_tableAdd_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MAC_TABLE_ENTRY_QUERY command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableQuery_t structure that is filled out by the switch
   implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MAC_TableEntryQuery(void *pDevCtx, IFX_ETHSW_MAC_tableQuery_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MAC_TABLE_ENTRY_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableRead_t structure that is filled out by the switch
   implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MAC_TableEntryRead(void *pDevCtx, IFX_ETHSW_MAC_tableRead_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableRemove_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MAC_TableEntryRemove(void *pDevCtx, IFX_ETHSW_MAC_tableRemove_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_STP_BPDU_RULE_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_STP_BPDU_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_STP_BPDU_RuleGet(void *pDevCtx, IFX_ETHSW_STP_BPDU_Rule_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_STP_BPDU_RULE_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_STP_BPDU_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_STP_BPDU_RuleSet(void *pDevCtx, IFX_ETHSW_STP_BPDU_Rule_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_STP_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_STP_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_STP_PortCfgGet(void *pDevCtx, IFX_ETHSW_STP_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_STP_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_STP_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_STP_PortCfgSet(void *pDevCtx, IFX_ETHSW_STP_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_TRUNKING_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      configuration \ref IFX_ETHSW_trunkingCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TrunkingCfgGet(void *pDevCtx, IFX_ETHSW_trunkingCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_TRUNKING_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      configuration \ref IFX_ETHSW_trunkingCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TrunkingCfgSet(void *pDevCtx, IFX_ETHSW_trunkingCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_TRUNKING_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      configuration \ref IFX_ETHSW_trunkingPortCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TrunkingPortCfgGet(void *pDevCtx, IFX_ETHSW_trunkingPortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_TRUNKING_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      configuration \ref IFX_ETHSW_trunkingPortCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TrunkingPortCfgSet(void *pDevCtx, IFX_ETHSW_trunkingPortCfg_t *pPar);

/*@}*/ /* FLOW_LL_BRIDGE */
/** \addtogroup FLOW_LL_VLAN */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_SVLAN_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_SVLAN_cfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_SVLAN_CfgGet(void *pDevCtx, IFX_ETHSW_SVLAN_cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_SVLAN_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_SVLAN_cfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_SVLAN_CfgSet(void *pDevCtx, IFX_ETHSW_SVLAN_cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_SVLAN_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an
      \ref IFX_ETHSW_SVLAN_portCfg_t structure element. Based on the parameter
      'nPortId', the switch API implementation fills out the remaining structure
      elements.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_SVLAN_PortCfgGet(void *pDevCtx, IFX_ETHSW_SVLAN_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_SVLAN_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_SVLAN_portCfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_SVLAN_PortCfgSet(void *pDevCtx, IFX_ETHSW_SVLAN_portCfg_t *pPar);
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_ID_CREATE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_VLAN_IdCreate_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_IdCreate(void *pDevCtx, IFX_ETHSW_VLAN_IdCreate_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_ID_DELETE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_VLAN_IdDelete_t structure element.

	\remarks A VLAN ID can only be removed in case it was created by
		\ref IFX_ETHSW_VLAN_ID_CREATE and is currently not assigned
		to any Ethernet port (done using \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD).

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_IdDelete(void *pDevCtx, IFX_ETHSW_VLAN_IdDelete_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_ID_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_VLAN_IdGet_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_IdGet(void *pDevCtx, IFX_ETHSW_VLAN_IdGet_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an
      \ref IFX_ETHSW_VLAN_portCfg_t structure element. Based on the parameter
      'nPortId', the switch API implementation fills out the remaining structure
      elements.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_PortCfgGet(void *pDevCtx, IFX_ETHSW_VLAN_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_VLAN_portCfg_t
      structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_PortCfgSet(void *pDevCtx, IFX_ETHSW_VLAN_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_VLAN_portMemberAdd_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_PortMemberAdd(void *pDevCtx, IFX_ETHSW_VLAN_portMemberAdd_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_PORT_MEMBER_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_VLAN_portMemberRead_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_PortMemberRead(void *pDevCtx, IFX_ETHSW_VLAN_portMemberRead_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_VLAN_portMemberRemove_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_PortMemberRemove(void *pDevCtx, IFX_ETHSW_VLAN_portMemberRemove_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_RESERVED_ADD command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_VLAN_reserved_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_ReservedAdd(void *pDevCtx, IFX_ETHSW_VLAN_reserved_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VLAN_RESERVED_REMOVE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_VLAN_reserved_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_VLAN_ReservedRemove(void *pDevCtx, IFX_ETHSW_VLAN_reserved_t *pPar);

/*@}*/ /* FLOW_LL_VLAN */
/** \addtogroup FLOW_LL_QOS */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_CLASS_DSCP_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the DSCP mapping parameter
		\ref IFX_ETHSW_QoS_ClassDSCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ClassDSCP_Get(void *pDevCtx, IFX_ETHSW_QoS_ClassDSCP_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_CLASS_DSCP_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the DSCP mapping parameter
   \ref IFX_ETHSW_QoS_ClassDSCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ClassDSCP_Set(void *pDevCtx, IFX_ETHSW_QoS_ClassDSCP_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_CLASS_PCP_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_ClassPCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ClassPCP_Get(void *pDevCtx, IFX_ETHSW_QoS_ClassPCP_Cfg_t *pPar);

/**
	This is the switch API low-level function for
	the \ref IFX_ETHSW_QOS_CLASS_PCP_SET command.

	\param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_ClassPCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ClassPCP_Set(void *pDevCtx, IFX_ETHSW_QoS_ClassPCP_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_DSCP_CLASS_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_DSCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_DSCP_ClassGet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_ClassCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_DSCP_CLASS_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_DSCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_DSCP_ClassSet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_ClassCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS
   DSCP drop precedence parameters
   \ref IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_DSCP_DropPrecedenceCfgGet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS
   DSCP drop precedence parameters
   \ref IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_DSCP_DropPrecedenceCfgSet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_FLOWCTRL_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_FlowctrlCfgGet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_FLOWCTRL_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_FlowctrlCfgSet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_FlowctrlPortCfgGet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlPortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_FlowctrlPortCfgSet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlPortCfg_t *pPar);
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_METER_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_MeterCfgGet(void *pDevCtx, IFX_ETHSW_QoS_meterCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_METER_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_MeterCfgSet(void *pDevCtx, IFX_ETHSW_QoS_meterCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_METER_PORT_ASSIGN command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterPort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_MeterPortAssign(void *pDevCtx, IFX_ETHSW_QoS_meterPort_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_METER_PORT_DEASSIGN command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterPort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_MeterPortDeassign(void *pDevCtx, IFX_ETHSW_QoS_meterPort_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_METER_PORT_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterPortGet_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_MeterPortGet(void *pDevCtx, IFX_ETHSW_QoS_meterPortGet_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_PCP_CLASS_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_PCP_ClassGet(void *pDevCtx, IFX_ETHSW_QoS_PCP_ClassCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_PCP_CLASS_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_PCP_ClassSet(void *pDevCtx, IFX_ETHSW_QoS_PCP_ClassCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      QOS port priority control configuration \ref IFX_ETHSW_QoS_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_PortCfgGet(void *pDevCtx, IFX_ETHSW_QoS_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a
      QOS port priority control configuration \ref IFX_ETHSW_QoS_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_PortCfgSet(void *pDevCtx, IFX_ETHSW_QoS_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_PORT_REMARKING_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_portRemarkingCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_PortRemarkingCfgGet(void *pDevCtx, IFX_ETHSW_QoS_portRemarkingCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_PORT_REMARKING_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the remarking configuration
   \ref IFX_ETHSW_QoS_portRemarkingCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_PortRemarkingCfgSet(void *pDevCtx, IFX_ETHSW_QoS_portRemarkingCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_QueueBufferReserveCfgGet(void *pDevCtx, IFX_ETHSW_QoS_QueueBufferReserveCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_QueueBufferReserveCfgSet(void *pDevCtx, IFX_ETHSW_QoS_QueueBufferReserveCfg_t *pPar);
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_QUEUE_PORT_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_queuePort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_QueuePortGet(void *pDevCtx, IFX_ETHSW_QoS_queuePort_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_QUEUE_PORT_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_queuePort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_QueuePortSet(void *pDevCtx, IFX_ETHSW_QoS_queuePort_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_SVLAN_ClassPCP_PortGet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_SVLAN_ClassPCP_PortSet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SVLAN_PCP_CLASS_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_SVLAN_PCP_ClassGet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SVLAN_PCP_CLASS_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_SVLAN_PCP_ClassSet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t *pPar);
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SCHEDULER_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_SchedulerCfgGet(void *pDevCtx, IFX_ETHSW_QoS_schedulerCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SCHEDULER_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_SchedulerCfgSet(void *pDevCtx, IFX_ETHSW_QoS_schedulerCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SHAPER_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ShaperCfgGet(void *pDevCtx, IFX_ETHSW_QoS_ShaperCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SHAPER_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ShaperCfgSet(void *pDevCtx, IFX_ETHSW_QoS_ShaperCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ShaperQueueAssign(void *pDevCtx, IFX_ETHSW_QoS_ShaperQueue_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SHAPER_QUEUE_DEASSIGN command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ShaperQueueDeassign(void *pDevCtx, IFX_ETHSW_QoS_ShaperQueue_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_SHAPER_QUEUE_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperQueueGet_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_ShaperQueueGet(void *pDevCtx, IFX_ETHSW_QoS_ShaperQueueGet_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_STORM_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_stormCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_StormCfgGet(void *pDevCtx, IFX_ETHSW_QoS_stormCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_STORM_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_stormCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_StormCfgSet(void *pDevCtx, IFX_ETHSW_QoS_stormCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_WRED_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_WredCfgGet(void *pDevCtx, IFX_ETHSW_QoS_WRED_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_WRED_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_WredCfgSet(void *pDevCtx, IFX_ETHSW_QoS_WRED_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_WRED_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_WredPortCfgGet(void *pDevCtx, IFX_ETHSW_QoS_WRED_PortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_WRED_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_WredPortCfgSet(void *pDevCtx, IFX_ETHSW_QoS_WRED_PortCfg_t *pPar);
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_WRED_QUEUE_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_WredQueueCfgGet(void *pDevCtx, IFX_ETHSW_QoS_WRED_QueueCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_QOS_WRED_QUEUE_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_QoS_WredQueueCfgSet(void *pDevCtx, IFX_ETHSW_QoS_WRED_QueueCfg_t *pPar);

/*@}*/ /* FLOW_LL_QOS */
/** \addtogroup FLOW_LL_MULTICAST */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_multicastRouter_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MulticastRouterPortAdd(void *pDevCtx, IFX_ETHSW_multicastRouter_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_multicastRouterRead_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs (e.g. Ethernet port parameter out of range)
*/
ethsw_ret_t IFX_FLOW_MulticastRouterPortRead(void *pDevCtx, IFX_ETHSW_multicastRouterRead_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_multicastRouter_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs (e.g. Ethernet port parameter out of range)
*/
ethsw_ret_t IFX_FLOW_MulticastRouterPortRemove(void *pDevCtx, IFX_ETHSW_multicastRouter_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the
   multicast configuration \ref IFX_ETHSW_multicastSnoopCfg_t.

   \remarks IGMP/MLD snooping is disabled when
   'eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'.
   Then all other structure parameters are unused.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MulticastSnoopCfgGet(void *pDevCtx, IFX_ETHSW_multicastSnoopCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to the
   multicast configuration \ref IFX_ETHSW_multicastSnoopCfg_t.

   \remarks IGMP/MLD snooping is disabled when
   'eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'.
   Then all other structure parameters are unused.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MulticastSnoopCfgSet(void *pDevCtx, IFX_ETHSW_multicastSnoopCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer
      to \ref IFX_ETHSW_multicastTable_t.

	\remarks The Source IP parameter is ignored in case IGMPv3 support is
		not enabled in the hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MulticastTableEntryAdd(void *pDevCtx, IFX_ETHSW_multicastTable_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer
      to \ref IFX_ETHSW_multicastTableRead_t.

	\remarks The 'bInitial' parameter is reset during the read operation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MulticastTableEntryRead(void *pDevCtx, IFX_ETHSW_multicastTableRead_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer
      to \ref IFX_ETHSW_multicastTable_t.

	\remarks The Source IP parameter is ignored in case IGMPv3 support is
		not enabled in the hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MulticastTableEntryRemove(void *pDevCtx, IFX_ETHSW_multicastTable_t *pPar);

/*@}*/ /* FLOW_LL_MULTICAST */
/** \addtogroup FLOW_LL_OAM */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CPU_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_CPU_PortCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_CPU_PortCfgGet(void *pDevCtx, IFX_ETHSW_CPU_PortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CPU_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_CPU_PortCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_CPU_PortCfgSet(void *pDevCtx, IFX_ETHSW_CPU_PortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CPU_PORT_EXTEND_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_CPU_PortExtendCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_CPU_PortExtendCfgGet(void *pDevCtx, IFX_ETHSW_CPU_PortExtendCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CPU_PORT_EXTEND_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_CPU_PortExtendCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_CPU_PortExtendCfgSet(void *pDevCtx, IFX_ETHSW_CPU_PortExtendCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CAP_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to pre-allocated capability
      list structure \ref IFX_ETHSW_cap_t.
      The switch API implementation fills out the structure with the supported
      features, based on the provided 'nCapType' parameter.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_CapGet(void *pDevCtx, IFX_ETHSW_cap_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_cfg_t structure.
      The structure is filled out by the switch implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_CfgGet(void *pDevCtx, IFX_ETHSW_cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_CfgSet(void *pDevCtx, IFX_ETHSW_cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_DISABLE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_Disable(void *pDevCtx);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_ENABLE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_Enable(void *pDevCtx);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_HW_INIT command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to pre-allocated initialization structure
   \ref IFX_ETHSW_HW_Init_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_HW_Init(void *pDevCtx, IFX_ETHSW_HW_Init_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MDIO_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_MDIO_cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MDIO_CfgGet(void *pDevCtx, IFX_ETHSW_MDIO_cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MDIO_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_MDIO_cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MDIO_CfgSet(void *pDevCtx, IFX_ETHSW_MDIO_cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MDIO_DATA_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_MDIO_data_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MDIO_DataRead(void *pDevCtx, IFX_ETHSW_MDIO_data_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MDIO_DATA_WRITE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_MDIO_data_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_MDIO_DataWrite(void *pDevCtx, IFX_ETHSW_MDIO_data_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MMD_DATA_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_MMD_data_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MmdDataRead(void *pDevCtx, IFX_ETHSW_MMD_data_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MMD_DATA_WRITE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_MMD_data_t.

	\remarks The function returns an error code in case an error occurs.
			The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MmdDataWrite(void *pDevCtx, IFX_ETHSW_MMD_data_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MONITOR_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_monitorPortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MonitorPortCfgGet(void *pDevCtx, IFX_ETHSW_monitorPortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_MONITOR_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_monitorPortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_MonitorPortCfgSet(void *pDevCtx, IFX_ETHSW_monitorPortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to a port configuration
   \ref IFX_ETHSW_portCfg_t structure to fill out by the driver.
   The parameter 'nPortId' tells the driver which port parameter is requested.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortCfgGet(void *pDevCtx, IFX_ETHSW_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_ETHSW_portCfg_t structure
   to configure the switch port hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortCfgSet(void *pDevCtx, IFX_ETHSW_portCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_LINK_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_portLinkCfg_t structure to read out the port status.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortLinkCfgGet(void *pDevCtx, IFX_ETHSW_portLinkCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_LINK_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_portLinkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortLinkCfgSet(void *pDevCtx, IFX_ETHSW_portLinkCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_PHY_ADDR_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_portPHY_Addr_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortPHY_AddrGet(void *pDevCtx, IFX_ETHSW_portPHY_Addr_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_PHY_QUERY command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_portPHY_Query_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortPHY_Query(void *pDevCtx, IFX_ETHSW_portPHY_Query_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_RGMII_CLK_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_portRGMII_ClkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortRGMII_ClkCfgGet(void *pDevCtx, IFX_ETHSW_portRGMII_ClkCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_RGMII_CLK_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_ETHSW_portRGMII_ClkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortRGMII_ClkCfgSet(void *pDevCtx, IFX_ETHSW_portRGMII_ClkCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_REDIRECT_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_portRedirectCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.
	\remarks Not all hardware platforms support this feature. The function
		returns an error if this feature is not supported.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortRedirectGet(void *pDevCtx, IFX_ETHSW_portRedirectCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_PORT_REDIRECT_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_portRedirectCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.
	\remarks Not all hardware platforms support this feature. The function
		returns an error if this feature is not supported.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PortRedirectSet(void *pDevCtx, IFX_ETHSW_portRedirectCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_RMON_CLEAR command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar  Pointer to a pre-allocated
   \ref IFX_ETHSW_RMON_clear_t structure. The structure element 'nPortId' is
   an input parameter stating on which port to clear all RMON counters.

   \remarks The function returns an error in case the given 'nPortId' is
   out of range.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_RMON_Clear(void *pDevCtx, IFX_ETHSW_RMON_clear_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_RMON_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar  Pointer to pre-allocated
   \ref IFX_ETHSW_RMON_cnt_t structure. The structure element 'nPortId' is
   an input parameter that describes from which port to read the RMON counter.
   All remaining structure elements are filled with the counter values.

   \remarks The function returns an error in case the given 'nPortId' is
   out of range.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_RMON_Get(void *pDevCtx, IFX_ETHSW_RMON_cnt_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_VERSION_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar* The parameter points to a
   \ref IFX_ETHSW_version_t structure.

   \return Returns value as follows:
   - LTQ_SUCCESS: if successful
   - LTQ_ERROR: in case of an error

*/
ethsw_ret_t IFX_FLOW_VersionGet(void *pDevCtx, IFX_ETHSW_version_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_WOL_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_WoL_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_WoL_CfgGet(void *pDevCtx, IFX_ETHSW_WoL_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_WOL_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_WoL_Cfg_t.

   \remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_WoL_CfgSet(void *pDevCtx, IFX_ETHSW_WoL_Cfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_WOL_PORT_CFG_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_WoL_PortCfg_t.

   \remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_WoL_PortCfgGet(void *pDevCtx, IFX_ETHSW_WoL_PortCfg_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_ETHSW_WOL_PORT_CFG_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_ETHSW_WoL_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_WoL_PortCfgSet(void *pDevCtx, IFX_ETHSW_WoL_PortCfg_t *pPar);

/*@}*/ /* FLOW_LL_OAM */
/** \addtogroup FLOW_LL_DEBUG */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_REGISTER_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_FLOW_register_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_RegisterGet(void *pDevCtx, IFX_FLOW_register_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_REGISTER_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_FLOW_register_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_RegisterSet(void *pDevCtx, IFX_FLOW_register_t *pPar);

/*@}*/ /* FLOW_LL_DEBUG */
/** \addtogroup FLOW_LL_IRQ */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_IRQ_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_FLOW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_IrqGet(void *pDevCtx, IFX_FLOW_irq_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_IRQ_MASK_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_FLOW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_IrqMaskGet(void *pDevCtx, IFX_FLOW_irq_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_IRQ_MASK_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_FLOW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_IrqMaskSet(void *pDevCtx, IFX_FLOW_irq_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_IRQ_STATUS_CLEAR command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_FLOW_irq_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
ethsw_ret_t IFX_FLOW_IrqStatusClear(void *pDevCtx, IFX_FLOW_irq_t *pPar);

/*@}*/ /* FLOW_LL_IRQ */
/** \addtogroup FLOW_LL_CLASSUNIT */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_PCE_RULE_DELETE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_FLOW_PCE_ruleDelete_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PceRuleDelete(void *pDevCtx, IFX_FLOW_PCE_ruleDelete_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_PCE_RULE_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_FLOW_PCE_rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PceRuleRead(void *pDevCtx, IFX_FLOW_PCE_rule_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_PCE_RULE_WRITE command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to \ref IFX_FLOW_PCE_rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_PceRuleWrite(void *pDevCtx, IFX_FLOW_PCE_rule_t *pPar);

/*@}*/ /* FLOW_LL_CLASSUNIT */
/** \addtogroup FLOW_LL_OAM */
/*@{*/
/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_RMON_EXTEND_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar  Pointer to a pre-allocated
   \ref IFX_FLOW_RMON_extendGet_t structure. The structure element 'nPortId' is
   an input parameter that describes from which port to read the RMON counter.
   All remaining structure elements are filled with the counter values.
   The counter assignment needs to be done during the flow definition,
   for example in \ref IFX_FLOW_PCE_RULE_WRITE.

	\remarks The function returns an error in case the given 'nPortId' is
		out of range.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_RMON_ExtendGet(void *pDevCtx, IFX_FLOW_RMON_extendGet_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_RESET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_FLOW_reset_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

   \remarks Not supported for all devices
*/
ethsw_ret_t IFX_FLOW_Reset(void *pDevCtx, IFX_FLOW_reset_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_TIMESTAMP_PORT_READ command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to
      an \ref IFX_FLOW_TIMESTAMP_PortRead_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TimestampPortRead(void *pDevCtx, IFX_FLOW_TIMESTAMP_PortRead_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_TIMESTAMP_TIMER_GET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_FLOW_TIMESTAMP_Timer_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TimestampTimerGet(void *pDevCtx, IFX_FLOW_TIMESTAMP_Timer_t *pPar);

/**
   This is the switch API low-level function for
   the \ref IFX_FLOW_TIMESTAMP_TIMER_SET command.

   \param pDevCtx This parameter is a pointer to the device context
   which contains all information related to this special instance of the device.
   \param pPar Pointer to an \ref IFX_FLOW_TIMESTAMP_Timer_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
ethsw_ret_t IFX_FLOW_TimestampTimerSet(void *pDevCtx, IFX_FLOW_TIMESTAMP_Timer_t *pPar);
/*@}*/ /* FLOW_LL_OAM */
#endif /* _LTQ_ETHSW_FLOW_LL_H_ */
