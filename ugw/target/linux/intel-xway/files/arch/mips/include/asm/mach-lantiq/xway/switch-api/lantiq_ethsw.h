/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _LANTIQ_ETHSW_H_
#define _LANTIQ_ETHSW_H_

/* =================================== */
/* Global typedef forward declarations */
/* =================================== */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* IOCTL MAGIC */
#define IFX_ETHSW_MAGIC ('E')

/* Group definitions for Doxygen */
/** \defgroup ETHSW_IOCTL Ethernet Switch Application Interface
	This chapter describes the entire interface for accessing and
	configuring the services of the Ethernet switch module.
	Switching is done based on the physical and virtual ports. */
/*@{*/

/** \defgroup ETHSW_IOCTL_BRIDGE Ethernet Bridging Functions
	Ethernet bridging (or switching) is the basic task of the device. It
	provides individual configurations per port and standard global
	switch features.
*/
/** \defgroup ETHSW_IOCTL_VLAN VLAN Functions
	This chapter describes VLAN bridging functionality.
	This includes support for Customer VLAN Tags (CTAG VLAN)
	and also Service VLAN Tags(STAG VLAN/SVLAN).
*/
/** \defgroup ETHSW_IOCTL_MULTICAST Multicast Functions
	IGMP/MLD snooping configuration and support for IGMPv1,
	IGMPv2, IGMPv3,	MLDv1, and MLDv2.
*/
/** \defgroup ETHSW_IOCTL_OAM Operation, Administration, and Management Functions
	This chapter summarizes the functions that are provided to monitor the
		data traffic passing through the device.
*/
/** \defgroup ETHSW_IOCTL_QOS Quality of Service Functions
	Switch and port configuration for Quality of Service (QoS).
*/

/*@}*/

/* -------------------------------------------------------------------------- */
/*                 Structure and Enumeration Type Defintions                  */
/* -------------------------------------------------------------------------- */

/** \addtogroup ETHSW_IOCTL_BRIDGE */
/*@{*/

/** MAC Address Field Size.
    Number of bytes used to store MAC address information. */
#define IFX_MAC_ADDRESS_LENGTH 6
/** This is the unsigned 32-bit datatype. */
typedef unsigned int    u32;
/** This is the unsigned 8-bit datatype. */
typedef unsigned char   u8;
/** This is the unsigned 16-bit datatype. */
typedef unsigned short  u16;
/** This is the unsigned 16-bit datatype. */
/* typedef unsigned short  IFX_uint16_t; */
/** A type for handling boolean issues. */

/** This type has two states, success and error. */
typedef enum {
	/** Operation failed. */
	IFX_ERROR		= (-1),
	/** Operation succeeded. */
	IFX_SUCCESS	= 0
} IFX_return_t;
typedef enum {
	LTQ_DISABLE		= 0,
	LTQ_ENABLE		= 1
} ltq_enDis_t;

typedef enum {
	LTQ_FALSE		= 0,
	LTQ_TRUE		= 1
} ltq_bool_t;

typedef enum {
	LTQ_ERROR		= (-1),
	LTQ_SUCCESS	= 0
}	ethsw_ret_t;

/** MAC Table Entry to be read.
		Used by \ref IFX_ETHSW_MAC_TABLE_ENTRY_READ. */
typedef struct {
	/** Restart the get operation from the beginning of the table. Otherwise
		return the next table entry (next to the entry that was returned
		during the previous get operation). This boolean parameter is set by the
		calling application. */
	ltq_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
		table. This boolean parameter is set by the switch API
		when the Switch API is called after the last valid one was returned already. */
	ltq_bool_t	bLast;
	/** Get the MAC table entry belonging to the given Filtering Identifier (FID)
		(not supported by all switches). */
	u32	nFId;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u32	nPortId;
	/** Aging Time, given in multiples of 1 second in a range from 1 s to 1,000,000 s.
		The value read back in a GET command might differ slightly from the value
		given in the SET command due to limited hardware timing resolution.
		Filled out by the switch API implementation. */
	int	nAgeTimer;
	/** STAG VLAN Id. Only applicable in case SVLAN support is
		enabled on the device. */
	u16	nSVLAN_Id;
	/** Static Entry (value will be aged out after 'nAgeTimer' if the entry
		is not set to static). */
	ltq_bool_t	bStaticEntry;
	/** MAC Address. Filled out by the switch API implementation. */
	u8	nMAC[IFX_MAC_ADDRESS_LENGTH];
} IFX_ETHSW_MAC_tableRead_t;

/** Search for a MAC address entry in the address table.
	Used by \ref IFX_ETHSW_MAC_TABLE_ENTRY_QUERY. */
typedef struct {
	/** MAC Address. This parameter needs to be provided for the search operation.
		This is an input parameter. */
	u8	nMAC[IFX_MAC_ADDRESS_LENGTH];
	/** Get the MAC table entry belonging to the given Filtering Identifier (FID)
		(not supported by all switches). This is an input parameter. */
	u32	nFId;
	/** MAC Address Found. Switch API sets this boolean variable in case
		the requested MAC address 'nMAC' is found inside the address table,
		otherwise it is set to FALSE. This is an output parameter. */
	ltq_bool_t	bFound;
	/** Ethernet Port number (zero-based counting). The valid range
	is hardware dependent.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u32	nPortId;
	/** Aging Time, given in multiples of 1 second in a range from 1 s to 1,000,000 s.
		The value read back in a GET command might differ slightly from the value
		given in the SET command due to limited hardware timing resolution.
		Filled out by the switch API implementation.
		This is an output parameter. */
   int	nAgeTimer;
	/** STAG VLAN Id. Only applicable in case SVLAN support is
		enabled on the device. */
	u16	nSVLAN_Id;
	/** Static Entry (value will be aged out after 'nAgeTimer' if the entry
		is not set to static). This is an output parameter. */
	ltq_bool_t	bStaticEntry;
} IFX_ETHSW_MAC_tableQuery_t;

/** MAC Table Entry to be added.
		Used by \ref IFX_ETHSW_MAC_TABLE_ENTRY_ADD. */
typedef struct {
	/** Filtering Identifier (FID) (not supported by all switches) */
	u32	nFId;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
		dependent. An error code is delivered if the selected port is not
		available.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u32	nPortId;
	/** Aging Time, given in multiples of 1 second in a range
		from 1 s to 1,000,000 s.
	The configured value might be rounded that it fits to the given hardware platform. */
	int	nAgeTimer;
	/** STAG VLAN Id. Only applicable in case SVLAN support is enabled on the device. */
	u16	nSVLAN_Id;
	/** Static Entry (value will be aged out if the entry is not set to static). The
		switch API implementation uses the maximum age timer in case the entry
		is not static. */
	ltq_bool_t	bStaticEntry;
	/** Egress queue traffic class.
	The queue index starts counting from zero.   */
	u8	nTrafficClass;
   /** MAC Address to add to the table. */
	u8	nMAC[IFX_MAC_ADDRESS_LENGTH];
} IFX_ETHSW_MAC_tableAdd_t;

/** MAC Table Entry to be removed.
    Used by \ref IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE. */
typedef struct {
	/** Filtering Identifier (FID) (not supported by all switches) */
	u32	nFId;
   /** MAC Address to be removed from the table. */
	u8	nMAC[IFX_MAC_ADDRESS_LENGTH];
} IFX_ETHSW_MAC_tableRemove_t;

/** Packet forwarding.
	Used by \ref IFX_ETHSW_STP_BPDU_Rule_t and \ref IFX_ETHSW_multicastSnoopCfg_t
	and \ref IFX_ETHSW_8021X_EAPOL_Rule_t. */
typedef enum {
	/** Default; portmap is determined by the forwarding classification. */
	IFX_ETHSW_PORT_FORWARD_DEFAULT	= 0,
   /** Discard; discard packets. */
	IFX_ETHSW_PORT_FORWARD_DISCARD	= 1,
	/** Forward to the CPU port. This requires that the CPU port is previously
		set by calling \ref IFX_ETHSW_CPU_PORT_CFG_SET. */
	IFX_ETHSW_PORT_FORWARD_CPU	= 2,
	/** Forward to a port, selected by the parameter 'nForwardPortId'.
		Please note that this feature is not supported by all
		hardware platforms. */
	IFX_ETHSW_PORT_FORWARD_PORT	= 3
} IFX_ETHSW_portForward_t;

/** Spanning Tree Protocol port states.
    Used by \ref IFX_ETHSW_STP_portCfg_t. */
typedef enum {
	/** Forwarding state. The port is allowed to transmit and receive
		all packets. Address Learning is allowed. */
	IFX_ETHSW_STP_PORT_STATE_FORWARD	= 0,
	/** Disabled/Discarding state. The port entity will not transmit
		and receive any packets. Learning is disabled in this state. */
	IFX_ETHSW_STP_PORT_STATE_DISABLE	= 1,
	/** Learning state. The port entity will only transmit and receive
		Spanning Tree Protocol packets (BPDU). All other packets are discarded.
	MAC table address learning is enabled for all good frames. */
	IFX_ETHSW_STP_PORT_STATE_LEARNING	= 2,
	/** Blocking/Listening. Only the Spanning Tree Protocol packets will
		be received and transmitted. All other packets are discarded by
		the port entity. MAC table address learning is disabled in this
	state. */
	IFX_ETHSW_STP_PORT_STATE_BLOCKING	= 3
} IFX_ETHSW_STP_PortState_t;

/** Configures the Spanning Tree Protocol state of an Ethernet port.
	Used by \ref IFX_ETHSW_STP_PORT_CFG_SET
	and \ref IFX_ETHSW_STP_PORT_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
		dependent. An error code is delivered if the selected port is not
		available. */
	u8		nPortId;
	/** Filtering Identifier (FID) (not supported by all switches).
		The FID allows to keep multiple STP states per physical Ethernet port.
		Multiple CTAG VLAN groups could be a assigned to one FID and therefore
		share the same STP port state. Switch API ignores the FID value
		in case the switch device does not support it or switch CTAG VLAN
		awareness is disabled. */
	u32	nFId;
	/** Spanning Tree Protocol state of the port. */
	IFX_ETHSW_STP_PortState_t	ePortState;
} IFX_ETHSW_STP_portCfg_t;

/** Spanning tree packet detection and forwarding.
	Used by \ref IFX_ETHSW_STP_BPDU_RULE_SET
	and \ref IFX_ETHSW_STP_BPDU_RULE_GET. */
typedef struct {
	/** Filter spanning tree packets and forward them, discard them or
		disable the filter. */
	IFX_ETHSW_portForward_t	eForwardPort;
	/** Target port for forwarded packets; only used if selected by
		'eForwardPort'. Forwarding is done
		if 'eForwardPort = IFX_ETHSW_PORT_FORWARD_PORT'. */
	u8		nForwardPortId;
} IFX_ETHSW_STP_BPDU_Rule_t;

/** Describes the 802.1x port state.
    Used by \ref IFX_ETHSW_8021X_portCfg_t. */
typedef enum {
	/** Receive and transmit direction are authorized. The port is allowed to
		transmit and receive all packets and the address learning process is
		also allowed. */
	IFX_ETHSW_8021X_PORT_STATE_AUTHORIZED	= 0,
	/** Receive and transmit direction are unauthorized. All the packets
		except EAPOL are not allowed to transmit and receive. The address learning
		process is disabled. */
	IFX_ETHSW_8021X_PORT_STATE_UNAUTHORIZED	= 1,
	/** Receive direction is authorized, transmit direction is unauthorized.
		The port is allowed to receive all packets. Packet transmission to this
		port is not allowed. The address learning process is also allowed. */
	IFX_ETHSW_8021X_PORT_STATE_RX_AUTHORIZED	= 2,
	/** Transmit direction is authorized, receive direction is unauthorized.
		The port is allowed to transmit all packets. Packet reception on this
		port is not allowed. The address learning process is disabled. */
	IFX_ETHSW_8021X_PORT_STATE_TX_AUTHORIZED	= 3
} IFX_ETHSW_8021X_portState_t;

/** EAPOL frames filtering rule parameter.
		Used by \ref IFX_ETHSW_8021X_EAPOL_RULE_GET
		and \ref IFX_ETHSW_8021X_EAPOL_RULE_SET. */
typedef struct {
	/** Filter authentication packets and forward them, discard them or
	disable the filter. */
	IFX_ETHSW_portForward_t	eForwardPort;
	/** Target port for forwarded packets, only used if selected by
	'eForwardPort'. Forwarding is done
	if 'eForwardPort = IFX_ETHSW_PORT_FORWARD_PORT'. */
	u8		nForwardPortId;
} IFX_ETHSW_8021X_EAPOL_Rule_t;

/** 802.1x port authentication status.
	Used by \ref IFX_ETHSW_8021X_PORT_CFG_GET
	and \ref IFX_ETHSW_8021X_PORT_CFG_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u32	nPortId;
   /** 802.1x state of the port. */
   IFX_ETHSW_8021X_portState_t	eState;
} IFX_ETHSW_8021X_portCfg_t;

/** Global Ethernet trunking configuration.
	Used by \ref IFX_ETHSW_TRUNKING_CFG_GET
	and \ref IFX_ETHSW_TRUNKING_CFG_SET. */
typedef struct {
	/** IP source address is used by the
		hash algorithm to calculate the egress trunking port index. */
	ltq_bool_t		bIP_Src;
	/** IP destination address is used by the
		hash algorithm to calculate the egress trunking port index. */
	ltq_bool_t		bIP_Dst;
	/** MAC source address is used by the
		hash algorithm to calculate the egress trunking port index. */
	ltq_bool_t		bMAC_Src;
	/** MAC destination address is used by the
		hash algorithm to calculate the egress trunking port index. */
	ltq_bool_t		bMAC_Dst;
} IFX_ETHSW_trunkingCfg_t;

/** Ethernet port trunking configuration.
	Used by \ref IFX_ETHSW_TRUNKING_PORT_CFG_GET
	and \ref IFX_ETHSW_TRUNKING_PORT_CFG_SET. */
typedef struct {
	/** Ports are aggregated.
	Enabling means that the 'nPortId' and
	the 'nAggrPortId' ports form an aggregated link. */
	ltq_bool_t	bAggregateEnable;
	/** Ethernet Port number (zero-based counting).
	The valid range is hardware dependent.
	An error code is delivered if the selected port is not
	available. */
	u32		nPortId;
	/** Second Aggregated Ethernet Port number (zero-based counting).
	The valid range is hardware dependent.
	An error code is delivered if the selected port is not
	available. */
	u32		nAggrPortId;
} IFX_ETHSW_trunkingPortCfg_t;

/*@}*/ /* ETHSW_IOCTL_BRIDGE */

/** \addtogroup ETHSW_IOCTL_VLAN */
/*@{*/

/** VLAN port configuration for ingress packet filtering. Tagged packet and
	untagged packet can be configured to be accepted or dropped (filtered out).
	Used by \ref IFX_ETHSW_VLAN_portCfg_t. */
typedef enum {
	/** Admit all. Tagged and untagged packets are allowed. */
	IFX_ETHSW_VLAN_ADMIT_ALL		= 0,
	/** Untagged packets only (not supported yet). Tagged packets are dropped. */
	IFX_ETHSW_VLAN_ADMIT_UNTAGGED	= 1,
	/** Tagged packets only. Untagged packets are dropped. */
	IFX_ETHSW_VLAN_ADMIT_TAGGED	= 2
} IFX_ETHSW_VLAN_Admit_t;

/** Add a CTAG VLAN ID group to the CTAG VLAN hardware table of the switch.
	Used by \ref IFX_ETHSW_VLAN_ID_CREATE. */
typedef struct {
	/** CTAG VLAN ID. The valid range is from 0 to 4095.
	An error code is delivered in case of range mismatch. */
	u16		nVId;
	/** Filtering Identifier (FID) (not supported by all switches). */
	u32		nFId;
} IFX_ETHSW_VLAN_IdCreate_t;

/** Read out the CTAG VLAN ID to FID assignment. The user provides the CTAG VLAN ID
	parameter and the switch APi returns the FID parameter.
	Used by \ref IFX_ETHSW_VLAN_ID_GET. */
typedef struct {
	/** CTAG VLAN ID. The valid range is from 0 to 4095.
		An error code is delivered in case of range mismatch. */
	u16		nVId;
	/** Filtering Identifier (FID) (not supported by all switches). */
	u32		nFId;
} IFX_ETHSW_VLAN_IdGet_t;

/** Default VLAN membership portmap and egress tagmap for unconfigured VLAN groups.
	Every bit in the portmap variables represents one port (port 0 = LSB bit).
	Used by \ref IFX_ETHSW_VLAN_MEMBER_INIT. */
typedef struct {
	/** Portmap field of the uninitialized VLAN groups. */
	u32	nPortMemberMap;
	/** Egress tagmap field of the uninitialized VLAN groups. */
	u32	nEgressTagMap;
} IFX_ETHSW_VLAN_memberInit_t;

/** Remove a CTAG VLAN ID from the switch CTAG VLAN table.
	Used by \ref IFX_ETHSW_VLAN_ID_DELETE. */
typedef struct {
	/** CTAG VLAN ID. The valid range is from 0 to 4095.
	An error code is delivered in case of range mismatch. */
	u16	nVId;
} IFX_ETHSW_VLAN_IdDelete_t;

/** Adds a CTAG VLAN to a port and set its egress filter information.
	Used by \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD. */
typedef struct {
	/** CTAG VLAN ID. The valid range is from 0 to 4095.
		An error code is delivered in case of range mismatch. */
	u16		nVId;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u8		nPortId;
	/** Tag Member Egress. Enable egress tag-based support.
		If enabled, all port egress traffic
		from this CTAG VLAN group carries a CTAG VLAN group tag. */
	ltq_bool_t	bVLAN_TagEgress;
} IFX_ETHSW_VLAN_portMemberAdd_t;

/** Remove the CTAG VLAN configuration from an Ethernet port.
	Used by \ref IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE. */
typedef struct {
	/** CTAG VLAN ID. The valid range is from 0 to 4095.
		An error code is delivered in case of range mismatch.
		If the selected VLAN ID is not found in the vLAN table,
		an error code is delivered. */
	u16	nVId;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
		dependent. An error code is delivered if the selected port is not
		available.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		 marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u8	nPortId;
} IFX_ETHSW_VLAN_portMemberRemove_t;

/** Read the CTAG VLAN port membership table.
	Used by \ref IFX_ETHSW_VLAN_PORT_MEMBER_READ. */
typedef struct {
	/** Restart the get operation from the start of the table. Otherwise
		return the next table entry (next to the entry that was returned
		during the previous get operation). This parameter is always reset
		during the read operation. This boolean parameter is set by the
		calling application. */
	ltq_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
		table. This boolean parameter is set by the switch API
		when the Switch API is called after the last valid one was returned already. */
	ltq_bool_t	bLast;
	/** CTAG VLAN ID. The valid range is from 0 to 4095.
		An error code is delivered in case of range mismatch. */
	u16	nVId;
	/** Ethernet Port number (zero-based counting). Every bit represents
		an Ethernet port.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u32	nPortId;
	/** Enable egress tag-Portmap. Every bit represents an Ethernet port.
	This field is used as portmap field, and the MSB bit is
	statically always set. LSB represents Port 0 with
	incrementing counting.
	The (MSB - 1) bit represent the last port.
	All port egress traffic from this CTAG VLAN group carries a
	tag, in case the port bit is set.

	\remarks
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u32	nTagId;
} IFX_ETHSW_VLAN_portMemberRead_t;

/** Port configuration for VLAN member violation.
	Used by \ref IFX_ETHSW_VLAN_portCfg_t. */
typedef enum {
	/** No VLAN member violation. Ingress and egress packets violating the
	membership pass and are not filtered out. */
	IFX_ETHSW_VLAN_MEMBER_VIOLATION_NO	= 0,
	/** VLAN member violation for ingress packets. Ingress packets violating
	the membership are filtered out. Egress packets violating the
	membership are not filtered out. */
	IFX_ETHSW_VLAN_MEMBER_VIOLATION_INGRESS	= 1,
	/** VLAN member violation for egress packets. Egress packets violating
	the membership are filtered out. Ingress packets violating the
	membership are not filtered out.*/
	IFX_ETHSW_VLAN_MEMBER_VIOLATION_EGRESS	= 2,
	/** VLAN member violation for ingress and egress packets.
	Ingress and egress packets violating the membership are filtered out. */
	IFX_ETHSW_VLAN_MEMBER_VIOLATION_BOTH	= 3
} IFX_ETHSW_VLAN_MemberViolation_t;

/** CTAG VLAN Port Configuration.
	Used by \ref IFX_ETHSW_VLAN_PORT_CFG_GET
	and \ref IFX_ETHSW_VLAN_PORT_CFG_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Port CTAG VLAN ID (PVID). The software shall ensure that the used VID has
	been configured in advance on the hardware by
	using \ref IFX_ETHSW_VLAN_ID_CREATE. */
	u16	nPortVId;
	/** Drop ingress CTAG VLAN-tagged packets if the VLAN ID
	is not listed in the active VLAN set. If disabled, all incoming
	VLAN-tagged packets are forwarded using the FID tag members and
	the port members of the PVID.
	This parameter is only supported for devices which do not
	support 4k VLAN table entries (64 entries instead). */
	ltq_bool_t	bVLAN_UnknownDrop;
	/** Reassign all ingress CTAG VLAN tagged packets to the port-based
	VLAN ID (PVID). */
	ltq_bool_t	bVLAN_ReAssign;
	/** VLAN ingress and egress membership violation mode. Allows admittance of
	VLAN-tagged packets where the port is not a member of the VLAN ID
	carried in the received and sent packet. */
	IFX_ETHSW_VLAN_MemberViolation_t	eVLAN_MemberViolation;
	/** Ingress VLAN-tagged or untagged packet filter configuration. */
	IFX_ETHSW_VLAN_Admit_t	eAdmitMode;
	/** Transparent CTAG VLAN Mode (TVM). All packets are handled as untagged
	packets. Any existing tag is ignored and treated as packet payload. */
	ltq_bool_t	bTVM;
} IFX_ETHSW_VLAN_portCfg_t;

/** This CTAG VLAN configuration supports replacing of the VID of received packets
	with the PVID of the receiving port.
	Used by \ref IFX_ETHSW_VLAN_RESERVED_ADD
	and \ref IFX_ETHSW_VLAN_RESERVED_REMOVE. */
typedef struct {
	/** VID of the received packet to be replaced by the PVID.
	The valid range is from 0 to 4095.
	An error code is delivered in case of range mismatch. */
	u16	nVId;
} IFX_ETHSW_VLAN_reserved_t;

/** STAG VLAN global configuration.
	Used by \ref IFX_ETHSW_SVLAN_CFG_GET
	and \ref IFX_ETHSW_SVLAN_CFG_SET. */
typedef struct {
	/** Protocl EtherType Field. This 16-bit of the STAG VLAN (default=0x88A8). */
	u16	nEthertype;
} IFX_ETHSW_SVLAN_cfg_t;

/** STAG VLAN Port Configuration.
	Used by \ref IFX_ETHSW_SVLAN_PORT_CFG_GET
	 and \ref IFX_ETHSW_SVLAN_PORT_CFG_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Port based STAG VLAN Support. All STAG VLAN protocol parsing and
	configuration features are only applied on this port in case the
	STAG VLAN port support is enabled. */
	ltq_bool_t	bSVLAN_TagSupport;
	/** Port Egress MAC based STAG VLAN. All egress packets contain a
	STAG VLAN ID that is based on the VLAN ID which is retrieved
	from the MAC bridging table. This MAC bridging table SVLAN ID
	can be learned from the snooped traffic or statically added. */
	ltq_bool_t	bSVLAN_MACbasedTag;
	/** Port STAG VLAN ID (PVID) */
	u16	nPortVId;
	/** Reassign all ingress STAG VLAN tagged packets to the port-based
	STAG VLAN ID (PVID). */
	ltq_bool_t	bVLAN_ReAssign;
	/** VLAN ingress and egress membership violation mode. Allows admittance of
	STAG VLAN-tagged packets where the port is not a member of the STAG VLAN ID
	carried in the received and sent packet. */
	IFX_ETHSW_VLAN_MemberViolation_t	eVLAN_MemberViolation;
	/** Ingress STAG VLAN-tagged or untagged packet filter configuration. */
	IFX_ETHSW_VLAN_Admit_t	eAdmitMode;
} IFX_ETHSW_SVLAN_portCfg_t;

/*@}*/ /* ETHSW_IOCTL_VLAN */

/** \addtogroup ETHSW_IOCTL_QOS */
/*@{*/

/** DSCP mapping table.
	Used by \ref IFX_ETHSW_QOS_DSCP_CLASS_SET
	and \ref IFX_ETHSW_QOS_DSCP_CLASS_GET. */
typedef struct {
	/** Traffic class associated with a particular DSCP value.
	DSCP is the index to an array of resulting traffic class values.
	The index starts counting from zero. */
	u8	nTrafficClass[64];
} IFX_ETHSW_QoS_DSCP_ClassCfg_t;

/** Traffic class associated with a particular 802.1P (PCP) priority mapping value.
	This table is global for the entire switch device. Priority map entry structure.
	Used by \ref IFX_ETHSW_QOS_PCP_CLASS_SET
	and \ref IFX_ETHSW_QOS_PCP_CLASS_GET. */
typedef struct {
	/** Configures the PCP to traffic class mapping.
	The queue index starts counting from zero. */
	u8	nTrafficClass[8];
} IFX_ETHSW_QoS_PCP_ClassCfg_t;

/** Ingress DSCP remarking attribute. This attribute defines on the
	ingress port packets how these will be remarked on the egress port.
	A packet is only remarked in case its ingress and its egress port
	have remarking enabled.
	Used by \ref IFX_ETHSW_QoS_portRemarkingCfg_t. */
typedef enum {
	/** No DSCP Remarking. No remarking is done on the egress port. */
	IFX_ETHSW_DSCP_REMARK_DISABLE	= 0,
	/** TC DSCP 6-Bit Remarking. The complete DSCP remarking is done based
	on the traffic class. The traffic class to DSCP value mapping is
	given in a device global table. */
	IFX_ETHSW_DSCP_REMARK_TC6	= 1,
	/** TC DSCP 3-Bit Remarking. The upper 3-Bits of the DSCP field are
	remarked based on the traffic class. The traffic class to DSCP value
	mapping is given in a device global table. */
	IFX_ETHSW_DSCP_REMARK_TC3	= 2,
	/** Drop Precedence Remarking. The Drop Precedence is remarked on the
	egress side. */
	IFX_ETHSW_DSCP_REMARK_DP3	= 3,
	/** TC Drop Precedence Remarking. The Drop Precedence is remarked on the
	egress side and the upper 3-Bits of the DSCP field are
	remarked based on the traffic class. The traffic class to DSCP value
	mapping is given in a device global table. */
	IFX_ETHSW_DSCP_REMARK_DP3_TC3	= 4
} IFX_ETHSW_Qos_ingressRemarking_t;

/** Port Remarking Configuration. Ingress and Egress remarking options for
	dedicated packet fields DSCP, CTAG VLAN PCP, STAG VLAN PCP
	and STAG VLAN DEI.
	Remarking is done either on the used traffic class or the
	drop precedence.
	Packet field specific remarking only applies on a packet if
	enabled on ingress and egress port.
	Used by \ref IFX_ETHSW_QOS_PORT_REMARKING_CFG_SET
	and \ref IFX_ETHSW_QOS_PORT_REMARKING_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Ingress DSCP Remarking. Specifies on ingress side how a packet should
	be remarked. This DSCP remarking only works in case remarking is
	enabled on the egress port.
	This configuration requires that remarking is also enabled on the
	egress port. DSCP remarking enable on either ingress or egress port
	side does not perform any remark operation. */
	IFX_ETHSW_Qos_ingressRemarking_t	eDSCP_IngressRemarkingEnable;
	/** Egress DSCP Remarking. Applies remarking on egress packets in a
	fashion as specified on the ingress port. This ingress port remarking
	is configured by the parameter 'eDSCP_IngressRemarking'.
	This configuration requires that remarking is also enabled on the
	ingress port. DSCP remarking enable on either ingress or egress port
	side does not perform any remark operation. */
	ltq_bool_t	bDSCP_EgressRemarkingEnable;
	/** Ingress PCP Remarking. Applies remarking to all port ingress packets.
	This configuration requires that remarking is also enabled on the
	egress port. PCP remarking enable on either ingress or egress port
	side does not perform any remark operation. */
	ltq_bool_t	bPCP_IngressRemarkingEnable;
	/** Egress PCP Remarking. Applies remarking for all port egress packets.
	This configuration requires that remarking is also enabled on the
	ingress port. PCP remarking enable on either ingress or egress port
	side does not perform any remark operation. */
	ltq_bool_t	bPCP_EgressRemarkingEnable;
	/** Ingress STAG VLAN PCP Remarking */
	ltq_bool_t	bSTAG_PCP_IngressRemarkingEnable;
	/** Ingress STAG VLAN DEI Remarking */
	ltq_bool_t	bSTAG_DEI_IngressRemarkingEnable;
	/** Egress STAG VLAN PCP & DEI Remarking */
	ltq_bool_t	bSTAG_PCP_DEI_EgressRemarkingEnable;
} IFX_ETHSW_QoS_portRemarkingCfg_t;

/** Traffic class to DSCP mapping table.
	Used by \ref IFX_ETHSW_QOS_CLASS_DSCP_SET
	and \ref IFX_ETHSW_QOS_CLASS_DSCP_GET. */
typedef struct {
	/** DSCP value (6-bit) associated with a particular Traffic class.
	Traffic class is the index to an array of resulting DSCP values.
	The index starts counting from zero. */
	u8	nDSCP[16];
} IFX_ETHSW_QoS_ClassDSCP_Cfg_t;

/** Traffic class associated with a particular 802.1P (PCP) priority mapping value.
	This table is global for the entire switch device. Priority map entry structure.
	Used by \ref IFX_ETHSW_QOS_CLASS_PCP_SET
	and \ref IFX_ETHSW_QOS_CLASS_PCP_GET. */
typedef struct {
	/** Configures the traffic class to PCP (3-bit) mapping.
	The queue index starts counting from zero. */
	u8	nPCP[16];
} IFX_ETHSW_QoS_ClassPCP_Cfg_t;

/** DSCP Drop Precedence to color code assignment.
	Used by \ref IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t. */
typedef enum {
	/** Critical Packet. Metering never changes the drop precedence of these packets. */
	IFX_ETHSW_DROP_PRECEDENCE_CRITICAL	 = 0,
	/** Green Drop Precedence Packet. Packet is marked with a 'low' drop precedence. */
	IFX_ETHSW_DROP_PRECEDENCE_GREEN	= 1,
	/** Yellow Drop Precedence Packet. Packet is marked with a 'middle' drop precedence. */
	IFX_ETHSW_DROP_PRECEDENCE_YELLOW	= 2,
	/** Red Drop Precedence Packet. Packet is marked with a 'high' drop precedence. */
	IFX_ETHSW_DROP_PRECEDENCE_RED	= 3
} IFX_ETHSW_QoS_DropPrecedence_t;

/** DSCP to Drop Precedence assignment table configuration.
	Used by \ref IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_SET
	and \ref IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_GET. */
typedef struct {
	/** DSCP to drop precedence assignment. Every array entry represents the
	drop precedence for one of the 64 existing DSCP values.
	DSCP is the index to an array of resulting drop precedence values.
	The index starts counting from zero. */
	IFX_ETHSW_QoS_DropPrecedence_t	nDSCP_DropPrecedence[64];
} IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t;

/** Selection of the traffic class field.
	Used by \ref IFX_ETHSW_QoS_portCfg_t.
	The port default traffic class is assigned in case non of the
	configured protocol code points given by the packet. */
typedef enum {
	/** No traffic class assignment based on DSCP or PCP */
	IFX_ETHSW_QOS_CLASS_SELECT_NO	= 0,
	/** Traffic class assignment based on DSCP. PCP information is ignored.
	The Port Class is used in case DSCP is not available in the packet. */
	IFX_ETHSW_QOS_CLASS_SELECT_DSCP	 = 1,
	/** Traffic class assignment based on PCP. DSCP information is ignored.
	The Port Class is used in case PCP is not available in the packet. */
	IFX_ETHSW_QOS_CLASS_SELECT_PCP	= 2,
	/** Traffic class assignment based on DSCP. Make the assignment based on
	PCP in case the DSCP information is not available in the packet header.
	The Port Class is used in case both are not available in the packet. */
	IFX_ETHSW_QOS_CLASS_SELECT_DSCP_PCP	= 3,
	/** CTAG VLAN PCP, IP DSCP. Traffic class assignment based
	on CTAG VLAN PCP, alternative use DSCP based assignment. */
	IFX_ETHSW_QOS_CLASS_SELECT_PCP_DSCP	= 4,
	/** STAG VLAN PCP. Traffic class assignment based
	on STAG VLAN PCP. */
	IFX_ETHSW_QOS_CLASS_SELECT_SPCP	= 5,
	/** STAG VLAN PCP, IP DSCP. Traffic class assignment based
	on STAG VLAN PCP, alternative use DSCP based assignment. */
	IFX_ETHSW_QOS_CLASS_SELECT_SPCP_DSCP         = 6,
	/** IP DSCP, STAG VLAN PCP. Traffic class assignment based
	on DSCP, alternative use STAG VLAN PCP based assignment. */
	IFX_ETHSW_QOS_CLASS_SELECT_DSCP_SPCP         = 7,
	/** STAG VLAN PCP, CTAG VLAN PCP. Traffic class assignment based
	on STAG VLAN PCP, alternative use CTAG VLAN PCP based assignment. */
	IFX_ETHSW_QOS_CLASS_SELECT_SPCP_PCP	= 8,
	/** STAG VLAN PCP, CTAG VLAN PCP, IP DSCP. Traffic class assignment
	based on STAG VLAN PCP, alternative use CTAG VLAN PCP based
	assignment, alternative use DSCP based assignment. */
	IFX_ETHSW_QOS_CLASS_SELECT_SPCP_PCP_DSCP     = 9,
	/** IP DSCP, STAG VLAN PCP, CTAG VLAN PCP. Traffic class assignment
	based on DSCP, alternative use STAG VLAN PCP based
	assignment, alternative use CTAG VLAN PCP based assignment. */
	IFX_ETHSW_QOS_CLASS_SELECT_DSCP_SPCP_PCP     = 10
} IFX_ETHSW_QoS_ClassSelect_t;

/** Describes which priority information of ingress packets is used
	(taken into account) to identify the packet priority and the related egress
	priority queue. For DSCP, the priority to queue assignment is done
	using \ref IFX_ETHSW_QOS_DSCP_CLASS_SET. For VLAN, the priority to queue
	assignment is done using \ref IFX_ETHSW_QOS_PCP_CLASS_SET.
	Used by \ref IFX_ETHSW_QOS_PORT_CFG_SET
	and \ref IFX_ETHSW_QOS_PORT_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Select the packet header field on which to base the traffic class assignment. */
	IFX_ETHSW_QoS_ClassSelect_t	eClassMode;
	/** Default port priority in case no other priority
	(such as VLAN-based PCP or IP-based DSCP) is used. */
	u8	nTrafficClass;
} IFX_ETHSW_QoS_portCfg_t;

/** Configures a rate shaper instance with the rate and the burst size.
	Used by \ref IFX_ETHSW_QOS_SHAPER_CFG_SET
	and \ref IFX_ETHSW_QOS_SHAPER_CFG_GET. */
typedef struct {
	/** Rate shaper index (zero-based counting). */
	u32	nRateShaperId;
	/** Enable/Disable the rate shaper. */
	ltq_bool_t	bEnable;
	/** 802.1Qav credit based shaper mode. This specific shaper
	algorithm mode is used by the audio/video bridging (AVB)
	network (according to 802.1Qav). By default, an token
	based shaper algorithm is used. */
	ltq_bool_t	bAVB;
	/** Committed Burst Size (CBS [bytes]) */
	u32	nCbs;
	/** Rate [kbit/s] */
	u32	nRate;
} IFX_ETHSW_QoS_ShaperCfg_t;

/** Assign one rate shaper instance to a QoS queue.
	Used by \ref IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN
	and \ref IFX_ETHSW_QOS_SHAPER_QUEUE_DEASSIGN. */
typedef struct {
	/** Rate shaper index (zero-based counting). */
	u8	nRateShaperId;
	/** QoS queue index (zero-based counting). */
	u8	nQueueId;
} IFX_ETHSW_QoS_ShaperQueue_t;

/** Retrieve if a rate shaper instance is assigned to a QoS egress queue.
	Used by \ref IFX_ETHSW_QOS_SHAPER_QUEUE_GET. */
typedef struct {
	/** QoS queue index (zero-based counting).
	This parameter is the input parameter for the GET function. */
	u8	nQueueId;
	/** Rate shaper instance assigned.
	If LTQ_TRUE, a rate shaper instance is assigned to the queue.
	Otherwise no shaper instance is assigned. */
	ltq_bool_t	bAssigned;
	/** Rate shaper index (zero-based counting). Only a valid
	instance is returned in case 'bAssigned == LTQ_TRUE'. */
	u8	nRateShaperId;
} IFX_ETHSW_QoS_ShaperQueueGet_t;

/** Drop Probability Profile. Defines the drop probability profile.
	Used by \ref IFX_ETHSW_QoS_WRED_Cfg_t. */
typedef enum {
	/** Pmin = 25%, Pmax = 75% (default) */
	IFX_ETHSW_QOS_WRED_PROFILE_P0	= 0,
	/** Pmin = 25%, Pmax = 50% */
	IFX_ETHSW_QOS_WRED_PROFILE_P1	= 1,
	/** Pmin = 50%, Pmax = 50% */
	IFX_ETHSW_QOS_WRED_PROFILE_P2	= 2,
	/** Pmin = 50%, Pmax = 75% */
	IFX_ETHSW_QOS_WRED_PROFILE_P3	= 3
} IFX_ETHSW_QoS_WRED_Profile_t;

/** Configures the global probability profile of the device.
	The min. and max. values are given in number of packet
	buffer segments. The size of a segment can be
	retrieved using \ref IFX_ETHSW_CAP_GET.
	Used by \ref IFX_ETHSW_QOS_WRED_CFG_SET
	and \ref IFX_ETHSW_QOS_WRED_CFG_GET. */
typedef struct {
	/** Drop Probability Profile. */
	IFX_ETHSW_QoS_WRED_Profile_t	eProfile;
	/** WRED Red Threshold Min [number of segments]. */
	u32	nRed_Min;
	/** WRED Red Threshold Max [number of segments]. */
	u32	nRed_Max;
	/** WRED Yellow Threshold Min [number of segments]. */
	u32	nYellow_Min;
	/** WRED Yellow Threshold Max [number of segments]. */
	u32	nYellow_Max;
	/** WRED Green Threshold Min [number of segments]. */
	u32	nGreen_Min;
	/** WRED Green Threshold Max [number of segments]. */
	u32	nGreen_Max;
} IFX_ETHSW_QoS_WRED_Cfg_t;

/** Configures the WRED threshold parameter.
	The min. and max. values are given in number of packet
	buffer segments. The size of a segment can be
	retrieved using \ref IFX_ETHSW_CAP_GET.
	Used by \ref IFX_ETHSW_QOS_WRED_QUEUE_CFG_SET
	and \ref IFX_ETHSW_QOS_WRED_QUEUE_CFG_GET. */
typedef struct {
	/** QoS queue index (zero-based counting). */
	u32	nQueueId;
	/** WRED Red Threshold Min [number of segments]. */
	u32	nRed_Min;
	/** WRED Red Threshold Max [number of segments]. */
	u32	nRed_Max;
	/** WRED Yellow Threshold Min [number of segments]. */
	u32	nYellow_Min;
	/** WRED Yellow Threshold Max [number of segments]. */
	u32	nYellow_Max;
	/** WRED Green Threshold Min [number of segments]. */
	u32	nGreen_Min;
	/** WRED Green Threshold Max [number of segments]. */
	u32	nGreen_Max;
} IFX_ETHSW_QoS_WRED_QueueCfg_t;

/** Configures the WRED threshold parameter per port.
	The configured thresholds apply to fill level sum
	of all egress queues which are assigned to the egress port.
	The min. and max. values are given in number of packet
	buffer segments. The size of a segment can be
	retrieved using \ref IFX_ETHSW_CAP_GET.
	Used by \ref IFX_ETHSW_QOS_WRED_PORT_CFG_SET
	and \ref IFX_ETHSW_QOS_WRED_PORT_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting).
	The valid range is hardware dependent. */
	u32	nPortId;
	/** WRED Red Threshold Min [number of segments]. */
	u32	nRed_Min;
	/** WRED Red Threshold Max [number of segments]. */
	u32	nRed_Max;
	/** WRED Yellow Threshold Min [number of segments]. */
	u32	nYellow_Min;
	/** WRED Yellow Threshold Max [number of segments]. */
	u32	nYellow_Max;
	/** WRED Green Threshold Min [number of segments]. */
	u32	nGreen_Min;
	/** WRED Green Threshold Max [number of segments]. */
	u32	nGreen_Max;
} IFX_ETHSW_QoS_WRED_PortCfg_t;

/** Configures the global buffer flow control threshold for
	conforming and non-conforming packets.
	The min. and max. values are given in number of packet
	buffer segments. The size of a segment can be
	retrieved using \ref IFX_ETHSW_CAP_GET.
	Used by \ref IFX_ETHSW_QOS_FLOWCTRL_CFG_SET
	and \ref IFX_ETHSW_QOS_FLOWCTRL_CFG_GET. */
typedef struct {
	/** Global Buffer Non Conforming Flow Control Threshold Minimum [number of segments]. */
	u32	nFlowCtrlNonConform_Min;
	/** Global Buffer Non Conforming Flow Control Threshold Maximum [number of segments]. */
	u32	nFlowCtrlNonConform_Max;
	/** Global Buffer Conforming Flow Control Threshold Minimum [number of segments]. */
	u32	nFlowCtrlConform_Min;
	/** Global Buffer Conforming Flow Control Threshold Maximum [number of segments]. */
	u32	nFlowCtrlConform_Max;
} IFX_ETHSW_QoS_FlowCtrlCfg_t;

/** Configures the ingress port flow control threshold for
	used packet segments.
	The min. and max. values are given in number of packet
	buffer segments. The size of a segment can be
	retrieved using \ref IFX_ETHSW_CAP_GET.
	Used by \ref IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET
	and \ref IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting).
	The valid range is hardware dependent. */
	u32	nPortId;
	/** Ingress Port occupied Buffer Flow Control Threshold Minimum [number of segments]. */
	u32	nFlowCtrl_Min;
	/** Ingress Port occupied Buffer Flow Control Threshold Maximum [number of segments]. */
	u32	nFlowCtrl_Max;
} IFX_ETHSW_QoS_FlowCtrlPortCfg_t;

/** Configures the parameters of a rate meter instance.
	Used by \ref IFX_ETHSW_QOS_METER_CFG_SET
	and \ref IFX_ETHSW_QOS_METER_CFG_GET. */
typedef struct {
	/** Enable/Disable the meter shaper. */
	ltq_bool_t	bEnable;
	/** Meter index (zero-based counting). */
	u32	nMeterId;
	/** Committed Burst Size (CBS [Bytes]). */
	u32	nCbs;
	/** Excess Burst Size (EBS [Bytes]). */
	u32	nEbs;
	/** Rate [kbit/s] */
	u32	nRate;
} IFX_ETHSW_QoS_meterCfg_t;

/** Specifies the direction for ingress and egress.
	Used by \ref IFX_ETHSW_QoS_meterPort_t
	and \ref IFX_ETHSW_QoS_meterPortGet_t. */
typedef enum {
	/** No direction. */
	IFX_ETHSW_DIRECTION_NONE	= 0,
	/** Ingress direction. */
	IFX_ETHSW_DIRECTION_INGRESS	= 1,
	/** Egress direction. */
	IFX_ETHSW_DIRECTION_EGRESS	= 2,
	/** Ingress and egress direction. */
	IFX_ETHSW_DIRECTION_BOTH	= 3
} IFX_ETHSW_direction_t;

/** Assign a rate meter instance to an ingress and/or egress port.
	Used by \ref IFX_ETHSW_QOS_METER_PORT_ASSIGN
	and \ref IFX_ETHSW_QOS_METER_PORT_DEASSIGN. */
typedef struct {
	/** Meter index (zero-based counting). */
	u32	nMeterId;
	/** Port assignment. Could be either ingress, egress or both. Setting it to
	'IFX_ETHSW_DIRECTION_NONE' would remove the queue for any port
	assignment. */
	IFX_ETHSW_direction_t	 eDir;
	/** Ingress Port Id. */
	u32	nPortIngressId;
	/** Egress Port Id. */
	u32	nPortEgressId;
} IFX_ETHSW_QoS_meterPort_t;

/** Reads out all meter instance to port assignments.
	Used by \ref IFX_ETHSW_QOS_METER_PORT_GET. */
typedef struct {
	/** Restart the get operation from the start of the table. Otherwise
		return the next table entry (next to the entry that was returned
		during the previous get operation). This boolean parameter is set by the
		calling application. */
	ltq_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
	table. This boolean parameter is set by the switch API
	when the Switch API is called after the last valid one was returned already. */
	ltq_bool_t	bLast;
	/** Port assignment. Could be either ingress, egress or both. Setting it to
	'IFX_ETHSW_DIRECTION_NONE' would remove the queue for any port
	assignment. */
	IFX_ETHSW_direction_t	eDir;
	/** Meter index (zero-based counting). */
	u8	nMeterId;
	/** Ingress Port Id. */
	u8	nPortIngressId;
	/** Egress Port Id. */
	u8	nPortEgressId;
} IFX_ETHSW_QoS_meterPortGet_t;

/** Assigns one meter instances for storm control.
	Used by \ref IFX_ETHSW_QOS_STORM_CFG_SET and \ref IFX_ETHSW_QOS_STORM_CFG_GET. */
typedef struct {
	/** Meter index 0 (zero-based counting). */
	int	nMeterId;
	/** Meter instances used for broadcast traffic. */
	ltq_bool_t	bBroadcast;
	/** Meter instances used for multicast traffic. */
	ltq_bool_t	bMulticast;
	/** Meter instances used for unknown unicast traffic. */
	ltq_bool_t	bUnknownUnicast;
} IFX_ETHSW_QoS_stormCfg_t;

/** Select the type of the egress queue scheduler.
	Used by \ref IFX_ETHSW_QoS_schedulerCfg_t. */
typedef enum {
	/** Strict Priority. */
	IFX_ETHSW_QOS_SCHEDULER_STRICT	= 0,
	/** Weighted Fair Queuing. */
	IFX_ETHSW_QOS_SCHEDULER_WFQ	= 1
} IFX_ETHSW_QoS_Scheduler_t;

/** Configures the egress queues attached to a single port, and that
	are scheduled to transmit the queued Ethernet packets.
	Used by \ref IFX_ETHSW_QOS_SCHEDULER_CFG_SET
	and \ref IFX_ETHSW_QOS_SCHEDULER_CFG_GET. */
typedef struct {
	/** QoS queue index (zero-based counting). */
	u8	nQueueId;
	/** Scheduler Type (Strict Priority/Weighted Fair Queuing). */
	IFX_ETHSW_QoS_Scheduler_t	eType;
	/** Weight in Token. Parameter used for WFQ configuration.
	Sets the weight in token in relation to all remaining
	queues on this egress port having WFQ configuration.
	This parameter is only used
	when 'eType=IFX_ETHSW_QOS_SCHEDULER_WFQ'. */
	u32	nWeight;
} IFX_ETHSW_QoS_schedulerCfg_t;

/** Sets the Queue ID for one traffic class of one port.
	Used by \ref IFX_ETHSW_QOS_QUEUE_PORT_SET
	and \ref IFX_ETHSW_QOS_QUEUE_PORT_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available.
	This is an input parameter for \ref IFX_ETHSW_QOS_QUEUE_PORT_GET. */
	u8	nPortId;
	/** Traffic Class index (zero-based counting).
	This is an input parameter for \ref IFX_ETHSW_QOS_QUEUE_PORT_GET. */
	u8	nTrafficClassId;
	/** QoS queue index (zero-based counting).
	This is an output parameter for \ref IFX_ETHSW_QOS_QUEUE_PORT_GET. */
	u8	nQueueId;
} IFX_ETHSW_QoS_queuePort_t;

/** Reserved egress queue buffer segments.
	Used by \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET
	and \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET. */
typedef struct {
	/** QoS queue index (zero-based counting).
	This is an input parameter for \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET. */
	u8	nQueueId;
	/** Reserved Buffer Segment Threshold [number of segments].
	This is an output parameter for \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET. */
	u32	nBufferReserved;
} IFX_ETHSW_QoS_QueueBufferReserveCfg_t;

/** Traffic class associated with a particular 802.1P (PCP) priority mapping value.
	One table is given per egress port. Priority map entry structure.
	The lowest 3 LSB bits (0 ... 2) of 'nPCP_DEI' describe the PCP field.
	Bit 3 describes the 'DEI' field.
	Used by \ref IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_SET
	and \ref IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Configures the traffic class to PCP (3-bit) mapping.
	The queue index starts counting from zero. */

	/** Configures the traffic class to CPCP (3-bit) mapping.
	The queue index starts counting from zero. */
	u8	nCPCP[16];

	/** Configures the traffic class to SPCP (3-bit) mapping.
	The queue index starts counting from zero. */
	u8	nSPCP[16];

	/** DSCP value (6-bit) associated with a particular Traffic class.
	Traffic class is the index to an array of resulting DSCP values.
	The index starts counting from zero. */
	u8	nDSCP[16];

} IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t;

/** Traffic class associated with a particular STAG VLAN 802.1P (PCP)
	priority and Drop Eligiable Indicator (DEI) mapping value.
	This table is global for the entire switch device. Priority map entry structure.
	The table index value is calculated by 'index=PCP + 8*DEI'
	Used by \ref IFX_ETHSW_QOS_SVLAN_PCP_CLASS_SET
	and \ref IFX_ETHSW_QOS_SVLAN_PCP_CLASS_GET. */
typedef struct {
	/** Configures the PCP and DEI to traffic class mapping.
	The queue index starts counting from zero. */
	u8	nTrafficClass[16];
	/**   Configures the PCP traffic color*/
	u8	nTrafficColor[16];
	/** PCP Remark disable control */
	u8	nPCP_Remark_Enable[16];
	/** DEI Remark disable control */
	u8	nDEI_Remark_Enable[16];

} IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t;

/*@}*/ /* ETHSW_IOCTL_QOS */

/** \addtogroup ETHSW_IOCTL_MULTICAST */
/*@{*/

/** Define setting the priority queue to an undefined value.
    This disables the priority feature. */
#define IFX_ETHSW_TRAFFIC_CLASS_DISABLE 0xFF

/** Configure the IGMP snooping mode.
    Used by \ref IFX_ETHSW_multicastSnoopCfg_t. */
typedef enum {
	/** IGMP management packet snooping and multicast level 3 table learning
	is disabled. */
	IFX_ETHSW_MULTICAST_SNOOP_MODE_DISABLED		= 0,
	/** IGMP management packet snooping is enabled and used for the hardware
	auto-learning to fill the multicast level 3 table. */
	IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING	= 1,
	/** IGMP management packet snooping is enabled and forwarded to the
	configured port. No autolearning of the multicast level 3 table. This
	table has to be maintained by the management software. */
	IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD		= 2
} IFX_ETHSW_multicastSnoopMode_t;

/** Configure the IGMP report suppression mode.
	Used by \ref IFX_ETHSW_multicastSnoopCfg_t. */
typedef enum {
	/** Report Suppression and Join Aggregation. */
	IFX_ETHSW_MULTICAST_REPORT_JOIN	= 0,
	/** Report Suppression. No Join Aggregation. */
	IFX_ETHSW_MULTICAST_REPORT	= 1,
	/** Transparent Mode. No Report Suppression and no Join Aggregation. */
	IFX_ETHSW_MULTICAST_TRANSPARENT	= 2
} IFX_ETHSW_multicastReportSuppression_t;

/** Configure the switch multicast configuration.
	Used by \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_SET
	and \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_GET. */
typedef struct {
	/** Enables and configures the IGMP/MLD snooping feature.
	Select autolearning or management packet forwarding mode.
	Packet forwarding is done to the port selected in 'eForwardPort'. */
	IFX_ETHSW_multicastSnoopMode_t	eIGMP_Mode;
	/** IGMPv3 hardware support.
	When enabled the IGMP table includes both the group table and
	the source list table. Otherwise the table only includes the
	group table. This feature is needed when supporting IGMPv3 and
	MLDv2 protocols. */
	ltq_bool_t	bIGMPv3;
	/** Enables snooped IGMP control packets treated as cross-CTAG VLAN packets. This
	parameter is used for hardware auto-learning and snooping packets
	forwarded to a dedicated port. This dedicated port can be selected
	over 'eForwardPort'. */
	ltq_bool_t	bCrossVLAN;
	/** Forward snooped packet, only used if forwarded mode
	is selected
	by 'eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_SNOOPFORWARD'. */
	IFX_ETHSW_portForward_t	eForwardPort;
	/** Target port for forwarded packets, only used if selected by
	'eForwardPort'. Forwarding is done
	if 'eForwardPort = IFX_ETHSW_PORT_FORWARD_PORT'. */
	u8	nForwardPortId;
	/** Snooping control class of service.
	Snooping control packet can be forwarded to the 'nForwardPortId' when
	selected in 'eIGMP_Mode'. The class of service of this port can be
	selected for the snooped control packets, starting from zero.
	The maximum possible service class depends
	on the hardware platform used. The value
	IFX_ETHSW_TRAFFIC_CLASS_DISABLE disables overwriting the given
	class assignment. */
	u8	nClassOfService;
	/** Robustness variable.
	Used when the hardware-based IGMP/MLD snooping function is enabled. This
	robust variable is used in case IGMP hardware learning is
	enabled ('eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING').
	Supported range: 1 ... 3 */
	u8	nRobust;
	/** Query interval.
	Used to define the query interval in units of 100 ms when the
	hardware-based IGMP/MLD snooping function is enabled.
	The automatically learned router port will be aged out if no IGMP/MLD
	query frame is received from the router port
	for (nQueryInterval * nRobust) seconds.
	The supported range is from 100 ms to 25.5 s, with a default value
	of 10 s. This query interval is used in case IGMP hardware learning is
	enabled ('eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING'). */
	u8	nQueryInterval;
	/** IGMP/MLD report suppression and Join Aggregation control.
	Whenever the report message is already sent out for the same multicast
	group, the successive report message within the
	query-max-responsetime with the same group ID will be filtered
	by the switch. This is called report suppression.
	Whenever the join message is already sent out for the same multicast
	group, the successive join message with the same group ID will be filtered.
	This is called join aggregation. This suppression control is used in
	case IGMP hardware learning is
	enable ('eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING'). */
	IFX_ETHSW_multicastReportSuppression_t       eSuppressionAggregation;
	/** Hardware IGMP snooping fast leave option.
	Allows the hardware to automatically clear the membership
	when receiving the IGMP leave packet. This
	fast leave option is used in case IGMP hardware learning is
	enabled ('eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING').
	Note: The fast-leave option shall only be enabled where only
	one host is connected to each interface.
	If fast-leave is enabled where more than one host is connected
	to an interface, some hosts might be dropped inadvertently.
	Fast-leave processing is supported only with IGMP version 2 hosts. */
	ltq_bool_t	bFastLeave;
	/** Hardware router port auto-learning. Allows for the
	ports on which a router is located to be learned automatically.
	This router port learning option is
	used in case IGMP hardware learning is
	enabled ('eIGMP_Mode = IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING'). */
	ltq_bool_t	bLearningRouter;
	/** Discard Unknown IP Multicast Packets.
	Multicast packets are defined as unknown in case the group address
	cannot be found in the switch multicast group table. The table group
	entries could be either automatically learned or they are statically
	added. This Boolean parameter defines if such unknown multicast
	packet are forwarded to the multicast forwarding
	portmap (command \ref IFX_ETHSW_PORT_CFG_SET,
	parameter 'bMulticastUnknownDrop') or if they are dropped instead.

	- LTQ_TRUE: Drop unknown multicast packets.
	- LTQ_FALSE: Forward unknown multicast packets for the
	multicast forwarding portmap.
	*/
	ltq_bool_t	bMulticastUnknownDrop;
} IFX_ETHSW_multicastSnoopCfg_t;

/** Add an Ethernet port as router port to the switch hardware multicast table.
	Used by \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD
	and \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u8	nPortId;
} IFX_ETHSW_multicastRouter_t;

/** Check if a port has been selected as a router port.
	Used by \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_READ. */
typedef struct {
	/** Restart the get operation from the start of the table. Otherwise
	return the next table entry (next to the entry that was returned
	during the previous get operation). This parameter is always reset
	during the read operation. This boolean parameter is set by the
	calling application. */
	ltq_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
	table. This boolean parameter is set by the switch API
	when the Switch API is called after the last valid one was returned already. */
	ltq_bool_t	bLast;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
} IFX_ETHSW_multicastRouterRead_t;

/** This is a union to describe the IPv4 and IPv6 parameter.
	Used by \ref IFX_ETHSW_multicastTable_t
	and \ref IFX_ETHSW_multicastTableRead_t. */
typedef union {
	/** Describe the IPv4 address.
	Only used if the IPv4 address should be read or configured.
	Cannot be used together with the IPv6 address fields. */
	u32	nIPv4;
	/** Describe the IPv6 address.
	Only used if the IPv6 address should be read or configured.
	Cannot be used together with the IPv4 address fields. */
	u16	nIPv6[8];
} IFX_ETHSW_IP_t;

/** Selection to use IPv4 or IPv6.
	Used by \ref IFX_ETHSW_multicastTable_t
	and \ref IFX_ETHSW_multicastTableRead_t. */
typedef enum {
	/** IPv4 */
	IFX_ETHSW_IP_SELECT_IPV4	= 0,
	/** IPv6 */
	IFX_ETHSW_IP_SELECT_IPV6	= 1
} IFX_ETHSW_IP_Select_t;

/** Defines the multicast group member mode.
	Used by \ref IFX_ETHSW_multicastTable_t
	and \ref IFX_ETHSW_multicastTableRead_t. */
typedef enum {
	/** Include source IP address membership mode.
	Only supported for IGMPv3. */
	IFX_ETHSW_IGMP_MEMBER_INCLUDE	= 0,
	/** Exclude source IP address membership mode.
	Only supported for IGMPv3. */
	IFX_ETHSW_IGMP_MEMBER_EXCLUDE	= 1,
	/** Group source IP address is 'don't care'. This means all source IP
	addresses (*) are included for the multicast group membership.
	This is the default mode for IGMPv1 and IGMPv2. */
	IFX_ETHSW_IGMP_MEMBER_DONT_CARE	= 2
} IFX_ETHSW_IGMP_MemberMode_t;

/** Add a host as a member to a multicast group.
	Used by \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD and
	\ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Select the IP version of the 'uIP_Gda' and 'uIP_Gsa' fields.
	 Both fields support either IPv4 or IPv6. */
	IFX_ETHSW_IP_Select_t	eIPVersion;
	/** Group Destination IP address (GDA). */
	IFX_ETHSW_IP_t	uIP_Gda;
	/** Group Source IP address. Only used in case IGMPv3 support is enabled
	and 'eModeMember != IFX_ETHSW_IGMP_MEMBER_DONT_CARE'. */
	IFX_ETHSW_IP_t	uIP_Gsa;
	/** Group member filter mode.
	This parameter is ignored when deleting a multicast membership table entry.
	The configurations 'IFX_ETHSW_IGMP_MEMBER_EXCLUDE'
	and 'IFX_ETHSW_IGMP_MEMBER_INCLUDE' are only supported
	if IGMPv3 is used. */
	IFX_ETHSW_IGMP_MemberMode_t	eModeMember;
} IFX_ETHSW_multicastTable_t;

/** Read out the multicast membership table.
	Used by \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ. */
typedef struct {
	/** Restart the get operation from the beginning of the table. Otherwise
	return the next table entry (next to the entry that was returned
	during the previous get operation). This parameter is always reset
	during the read operation. This boolean parameter is set by the
	calling application. */
	ltq_bool_t	bInitial;
	/** Indicates that the read operation got all last valid entries of the
	table. This boolean parameter is set by the switch API
	when the Switch API is called after the last valid one was returned already. */
	ltq_bool_t	bLast;
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available.

	\remarks
		This field is used as portmap field, when the MSB bit is set.
		In portmap mode, every value bit represents an Ethernet port.
		LSB represents Port 0 with incrementing counting.
		The (MSB - 1) bit represent the last port.
		The macro \ref IFX_ETHSW_PORTMAP_FLAG_SET allows to set the MSB bit,
		marking it as portmap variable.
		Checking the portmap flag can be done by
		using the \ref IFX_ETHSW_PORTMAP_FLAG_GET macro. */
	u8	nPortId;
	/** Select the IP version of the 'uIP_Gda' and 'uIP_Gsa' fields.
	Both fields support either IPv4 or IPv6. */
	IFX_ETHSW_IP_Select_t	eIPVersion;
	/** Group Destination IP address (GDA). */
	IFX_ETHSW_IP_t	uIP_Gda;
	/** Group Source IP address. Only used in case IGMPv3 support is enabled. */
	IFX_ETHSW_IP_t	uIP_Gsa;
	/** Group member filter mode.
	This parameter is ignored when deleting a multicast membership table entry.
	The configurations 'IFX_ETHSW_IGMP_MEMBER_EXCLUDE'
	and 'IFX_ETHSW_IGMP_MEMBER_INCLUDE' are only supported
	if IGMPv3 is used. */
	IFX_ETHSW_IGMP_MemberMode_t	eModeMember;
} IFX_ETHSW_multicastTableRead_t;

/*@}*/ /* ETHSW_IOCTL_MULTICAST */

/** \addtogroup ETHSW_IOCTL_OAM */
/*@{*/

/** Maximum version information string length. */
#define IFX_ETHSW_VERSION_LEN 64

/** Maximum String Length for the Capability String. */
#define IFX_ETHSW_CAP_STRING_LEN 128

/** Sets the portmap flag of a PortID variable.
	Some Switch API commands allow to use a port index as portmap variable.
	This requires that the MSB bit is set to indicate that this variable
	contains a portmap, instead of a port index.
	In portmap mode, every value bit represents an Ethernet port.
	LSB represents Port 0 with incrementing counting.
	The (MSB - 1) bit represent the last port. */
#define IFX_ETHSW_PORTMAP_FLAG_SET(varType) (1 << (sizeof(((varType *)0)->nPortId) * 8 - 1))

/** Checks the portmap flag of a PortID variable.
	Some Switch API commands allow to use a port index as portmap variable.
	This requires that the MSB bit is set to indicate that this variable
	contains a portmap, instead of a port index.
	In portmap mode, every value bit represents an Ethernet port.
	LSB represents Port 0 with incrementing counting.
	The (MSB - 1) bit represent the last port. */
#define IFX_ETHSW_PORTMAP_FLAG_GET(varType) (1 << (sizeof(((varType *)0)->nPortId) * 8 - 1))

/** Data structure used to request the Switch API and device hardware
	version information. A zero-based index is provided to the Switch API that
	describes the request version information.
	Used by \ref IFX_ETHSW_VERSION_GET. */
typedef struct {
   /** Version ID starting with 0. */
	u16	nId;
	/** Name or ID of the version information. */
	char	 cName[IFX_ETHSW_VERSION_LEN];
	/** Version string information. */
	char	cVersion[IFX_ETHSW_VERSION_LEN];
} IFX_ETHSW_version_t;

/** Switch API hardware initialization mode.
	Used by \ref IFX_ETHSW_HW_Init_t. */
typedef enum {
	/** Access the switch hardware to read out status and capability
	information. Then define the basic hardware configuration to bring
	the hardware into a pre-defined state. */
	IFX_ETHSW_HW_INIT_WR	= 0,
	/** Access the switch hardware to read out status and capability
	information. Do not write any hardware configuration to the device.
	This means that the current existing hardware configuration remains
	unchanged. */
	IFX_ETHSW_HW_INIT_RO	= 1,
	/** Initialize the switch software module but do not touch the switch
	hardware. This means that no read or write operations are done on
	the switch hardware. Status and capability information cannot be
	retrieved from the hardware. */
	IFX_ETHSW_HW_INIT_NO	= 2
} IFX_ETHSW_HW_InitMode_t;

/** Switch hardware platform initialization structure.
	Used by \ref IFX_ETHSW_HW_INIT. */
typedef struct {
	/** Select the type of Switch API and hardware initialization. */
	IFX_ETHSW_HW_InitMode_t	eInitMode;
} IFX_ETHSW_HW_Init_t;

/** Aging Timer Value.
	Used by \ref IFX_ETHSW_cfg_t. */
typedef enum {
	/** 1 second */
	IFX_ETHSW_AGETIMER_1_SEC	= 1,
	/** 10 seconds */
	IFX_ETHSW_AGETIMER_10_SEC	= 2,
	/** 300 seconds */
	IFX_ETHSW_AGETIMER_300_SEC	= 3,
	/** 1 hour */
	IFX_ETHSW_AGETIMER_1_HOUR	= 4,
	/** 24 hours */
	IFX_ETHSW_AGETIMER_1_DAY	= 5
} IFX_ETHSW_ageTimer_t;

/** Ethernet port speed mode.
	A port might support only a subset of the possible settings.
	Used by \ref IFX_ETHSW_portLinkCfg_t. */
typedef enum {
	/** 10 Mbit/s */
	IFX_ETHSW_PORT_SPEED_10	= 10,
	/** 100 Mbit/s */
	IFX_ETHSW_PORT_SPEED_100	= 100,
	/** 200 Mbit/s */
	IFX_ETHSW_PORT_SPEED_200	= 200,
	/** 1000 Mbit/s */
	IFX_ETHSW_PORT_SPEED_1000	= 1000
} IFX_ETHSW_portSpeed_t;

/** Ethernet port duplex status.
	Used by \ref IFX_ETHSW_portLinkCfg_t. */
typedef enum {
	/** Port operates in full-duplex mode */
	IFX_ETHSW_DUPLEX_FULL	= 0,
	/** Port operates in half-duplex mode */
	IFX_ETHSW_DUPLEX_HALF	= 1
} IFX_ETHSW_portDuplex_t;

/** Force the MAC and PHY link modus.
	Used by \ref IFX_ETHSW_portLinkCfg_t. */
typedef enum {
	/** Link up. Any connected LED
	still behaves based on the real PHY status. */
	IFX_ETHSW_PORT_LINK_UP	= 0,
	/** Link down. */
	IFX_ETHSW_PORT_LINK_DOWN	= 1
} IFX_ETHSW_portLink_t;

/** Enumeration used for phone capability types.
	Used by \ref IFX_ETHSW_cap_t. */
typedef enum {
	/** Number of physical Ethernet ports. */
	IFX_ETHSW_CAP_TYPE_PORT									= 0,
	/** Number of virtual Ethernet ports. */
	IFX_ETHSW_CAP_TYPE_VIRTUAL_PORT	= 1,
	/** Size of internal packet memory [in Bytes]. */
	IFX_ETHSW_CAP_TYPE_BUFFER_SIZE	= 2,
	/** Buffer segment size.
	Byte size of a segment, used to store received packet data. */
	IFX_ETHSW_CAP_TYPE_SEGMENT_SIZE	= 3,
	/** Number of priority queues per device. */
	IFX_ETHSW_CAP_TYPE_PRIORITY_QUEUE	= 4,
	/** Number of meter instances. */
	IFX_ETHSW_CAP_TYPE_METER	= 5,
	/** Number of rate shaper instances. */
	IFX_ETHSW_CAP_TYPE_RATE_SHAPER	= 6,
	/** Number of CTAG VLAN groups that can be configured on the switch hardware. */
	IFX_ETHSW_CAP_TYPE_VLAN_GROUP	= 7,
	/** Number of Filtering Identifiers (FIDs) */
	IFX_ETHSW_CAP_TYPE_FID	= 8,
	/** Number of MAC table entries */
	IFX_ETHSW_CAP_TYPE_MAC_TABLE_SIZE	= 9,
	/** Number of multicast level 3 hardware table entries */
	IFX_ETHSW_CAP_TYPE_MULTICAST_TABLE_SIZE      = 10,
	/** Number of supported PPPoE sessions. */
	IFX_ETHSW_CAP_TYPE_PPPOE_SESSION	= 11,
	/** Number of STAG VLAN groups that can be configured on the switch hardware. */
	IFX_ETHSW_CAP_TYPE_SVLAN_GROUP	= 12,
	/** Last Capability Index */
	IFX_ETHSW_CAP_TYPE_LAST	= 13
} IFX_ETHSW_capType_t;

/** Capability structure.
	Used by \ref IFX_ETHSW_CAP_GET. */
typedef struct {
	/** Defines the capability type, see \ref IFX_ETHSW_capType_t.*/
	IFX_ETHSW_capType_t	nCapType;
	/** Description of the capability. */
	char									cDesc[IFX_ETHSW_CAP_STRING_LEN];
	/** Defines if, what or how many are available. The definition of cap
	depends on the type, see captype. */
	u32									nCap;
} IFX_ETHSW_cap_t;

/** Global switch configuration.
	Used by \ref IFX_ETHSW_CFG_SET and \ref IFX_ETHSW_CFG_GET. */
typedef struct {
	/** MAC table aging timer. After this timer expires the MAC table
	entry is aged out. */
	IFX_ETHSW_ageTimer_t	eMAC_TableAgeTimer;
	/** VLAN Awareness. The switch is VLAN unaware if this variable is disabled.
	In this mode, no VLAN-related APIs are supported and return with an error.
	The existing VLAN configuration is discarded when VLAN is disabled again. */
	ltq_bool_t	bVLAN_Aware;
	/** Maximum Ethernet packet length. */
	u16	nMaxPacketLen;
	/** Automatic MAC address table learning limitation consecutive action.
	These frame addresses are not learned, but there exists control as to whether
	the frame is still forwarded or dropped.
		- false: Drop
		- LTQ_TRUE: Forward
	*/
	ltq_bool_t	bLearningLimitAction;
	/** Accept or discard MAC spoofing and port MAC locking violation packets.
	MAC spoofing detection features identifies ingress packets that carry
	 a MAC source address which was previously learned on a different
	ingress port (learned by MAC bridging table). This also applies to
	static added entries. MAC spoofing detection is enabled on port
	level by 'bMAC_SpoofingDetection'.
	MAC address port locking is configured on port level
	by 'bLearningMAC_PortLock'.
	- LTQ_FALSE: Drop
	- LTQ_TRUE: Forward
	*/
	ltq_bool_t	bMAC_SpoofingAction;
	/** Pause frame MAC source address mode. If enabled, use the alternative
	address specified with 'nMAC'. */
	ltq_bool_t	bPauseMAC_ModeSrc;
	/** Pause frame MAC source address. */
	u8	nPauseMAC_Src[IFX_MAC_ADDRESS_LENGTH];
} IFX_ETHSW_cfg_t;

/** Port Enable Options.
    Used by \ref IFX_ETHSW_portCfg_t. */
typedef enum {
	/** The port is disabled in both directions. */
	IFX_ETHSW_PORT_DISABLE	= 0,
	/** The port is enabled in both directions (ingress and egress). */
	IFX_ETHSW_PORT_ENABLE_RXTX	= 1,
	/** The port is enabled in the receive (ingress) direction only. */
	IFX_ETHSW_PORT_ENABLE_RX	= 2,
	/** The port is enabled in the transmit (egress) direction only. */
	IFX_ETHSW_PORT_ENABLE_TX	= 3
} IFX_ETHSW_portEnable_t;

/** Port Mirror Options.
	Used by \ref IFX_ETHSW_portCfg_t. */
typedef enum {
	/** Mirror Feature is disabled. Normal port usage. */
	IFX_ETHSW_PORT_MONITOR_NONE	= 0,
	/** Port Ingress packets are mirrored to the monitor port. */
	IFX_ETHSW_PORT_MONITOR_RX	= 1,
	/** Port Egress packets are mirrored to the monitor port. */
	IFX_ETHSW_PORT_MONITOR_TX	= 2,
	/** Port Ingress and Egress packets are mirrored to the monitor port. */
	IFX_ETHSW_PORT_MONITOR_RXTX	= 3,
	/** Packet mirroring of 'unknown VLAN violation' frames. */
	IFX_ETHSW_PORT_MONITOR_VLAN_UNKNOWN	= 4,
	/** Packet mirroring of 'VLAN ingress or egress membership violation' frames. */
	IFX_ETHSW_PORT_MONITOR_VLAN_MEMBERSHIP       = 16,
	/** Packet mirroring of 'port state violation' frames. */
	IFX_ETHSW_PORT_MONITOR_PORT_STATE	= 32,
	/** Packet mirroring of 'MAC learning limit violation' frames. */
	IFX_ETHSW_PORT_MONITOR_LEARNING_LIMIT        = 64,
	/** Packet mirroring of 'port lock violation' frames. */
	IFX_ETHSW_PORT_MONITOR_PORT_LOCK	= 128
} IFX_ETHSW_portMonitor_t;

/** Ethernet flow control status.
	Used by \ref IFX_ETHSW_portCfg_t. */
typedef enum {
	/** Automatic flow control mode selection through auto-negotiation. */
	IFX_ETHSW_FLOW_AUTO	= 0,
	/** Receive flow control only */
	IFX_ETHSW_FLOW_RX	= 1,
	/** Transmit flow control only */
	IFX_ETHSW_FLOW_TX	= 2,
	/** Receive and Transmit flow control */
	IFX_ETHSW_FLOW_RXTX	= 3,
	/** No flow control */
	IFX_ETHSW_FLOW_OFF	= 4
} IFX_ETHSW_portFlow_t;

/** Port Configuration.
	Used by \ref IFX_ETHSW_PORT_CFG_GET and \ref IFX_ETHSW_PORT_CFG_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Enable Port (ingress only, egress only, both directions, or disabled).
	This parameter is used for Spanning Tree Protocol and 802.1X applications. */
   IFX_ETHSW_portEnable_t	eEnable;
	/** Drop unknown unicast packets.
	Do not send out unknown unicast packets on this port,
	if the boolean parameter is enabled. By default packets of this type
	are forwarded to this port. */
	ltq_bool_t	bUnicastUnknownDrop;
	/** Drop unknown multicast packets.
	Do not send out unknown multicast packets on this port,
	if boolean parameter is enabled. By default packets of this type
	are forwarded to this port.
	Some platforms also drop broadcast packets. */
	ltq_bool_t	bMulticastUnknownDrop;
	/** Drop reserved packet types
	(destination address from '01 80 C2 00 00 00' to
	'01 80 C2 00 00 2F') received on this port. */
	ltq_bool_t	bReservedPacketDrop;
	/** Drop Broadcast packets received on this port. By default packets of this
	type are forwarded to this port. */
	ltq_bool_t	bBroadcastDrop;
	/** Enables MAC address table aging.
	The MAC table entries learned on this port are removed after the
	aging time has expired.
	The aging time is a global parameter, common to all ports. */
	ltq_bool_t	bAging;
	/** MAC address table learning on the port specified by 'nPortId'.
	By default this parameter is always enabled. */
	ltq_bool_t	bLearning;
	/** Automatic MAC address table learning locking on the port specified
	by 'nPortId'.
	This parameter is only taken into account when 'bLearning' is enabled. */
	ltq_bool_t	bLearningMAC_PortLock;
	/** Automatic MAC address table learning limitation on this port.
	The learning functionality is disabled when the limit value is zero.
	The value 0xFFFF to allow unlimited learned address.
	This parameter is only taken into account when 'bLearning' is enabled. */
	u16	nLearningLimit;
	/** MAC spoofing detection. Identifies ingress packets that carry
	a MAC source address which was previously learned on a different ingress
	port (learned by MAC bridging table). This also applies to static added
	entries. Those violated packets could be accepted or discarded,
	depending on the global switch configuration 'bMAC_SpoofingAction'.
	This parameter is only taken into account when 'bLearning' is enabled. */
	ltq_bool_t	bMAC_SpoofingDetection;
	/** Port Flow Control Status. Enables the flow control function. */
	IFX_ETHSW_portFlow_t	eFlowCtrl;
	/** Port monitor feature. Allows forwarding of egress and/or ingress
	packets to the monitor port. If enabled, the monitor port gets
	a copy of the selected packet type. */
	IFX_ETHSW_portMonitor_t	ePortMonitor;
} IFX_ETHSW_portCfg_t;

/** Special tag Ethertype mode */
typedef enum {
	/** The EtherType field of the Special Tag of egress packets is always set
	to a prefined value. This same defined value applies for all
	switch ports. */
	IFX_ETHSW_CPU_ETHTYPE_PREDEFINED	= 0,
	/** The Ethertype field of the Special Tag of egress packets is set to
	the FlowID parameter, which is a results of the switch flow
	classification result. The switch flow table rule provides this
	FlowID as action parameter. */
	IFX_ETHSW_CPU_ETHTYPE_FLOWID	= 1
} IFX_ETHSW_CPU_SpecialTagEthType_t;

/** Defines one port that is directly connected to the software running on a CPU.
	Used by \ref IFX_ETHSW_CPU_PORT_CFG_SET and \ref IFX_ETHSW_CPU_PORT_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** CPU port validity.
	Set command: set true to define a CPU port, set false to undo the setting.
	Get command: true if defined as CPU, false if not defined as CPU port. */
	ltq_bool_t	bCPU_PortValid;
	/** Special tag enable in ingress direction. */
	ltq_bool_t	bSpecialTagIngress;
	/** Special tag enable in egress direction. */
	ltq_bool_t	bSpecialTagEgress;
	/** Enable FCS check
		- false: No check, forward all frames
		- LTQ_TRUE: Check FCS, drop frames with errors
	*/
	ltq_bool_t	bFcsCheck;
	/** Enable FCS generation
		- false: Forward packets without FCS
		- LTQ_TRUE: Generate FCS for all frames
	*/
	ltq_bool_t	bFcsGenerate;
	/** Special tag Ethertype mode. */
	IFX_ETHSW_CPU_SpecialTagEthType_t	bSpecialTagEthType;
} IFX_ETHSW_CPU_PortCfg_t;

/** Ethernet layer-2 header selector, when adding or removing on
	transmitted packets.
	Used by \ref IFX_ETHSW_CPU_PortExtendCfg_t. */
typedef enum {
	/** No additional Ethernet header. */
	IFX_ETHSW_CPU_HEADER_NO	= 0,
	/** Additional Ethernet header. */
	IFX_ETHSW_CPU_HEADER_MAC	= 1,
	/** Additional Ethernet- and CTAG VLAN- header. */
	IFX_ETHSW_CPU_HEADER_VLAN	= 2
} IFX_ETHSW_CPU_HeaderMode_t;

/** CPU Port Layer-2 Header extension.
	Used by \ref IFX_ETHSW_CPU_PortExtendCfg_t. */
typedef struct {
	/** Packet MAC Source Address. */
	u8	nMAC_Src[IFX_MAC_ADDRESS_LENGTH];
	/** Packet MAC Destination Address. */
	u8	nMAC_Dst[IFX_MAC_ADDRESS_LENGTH];
	/** Packet EtherType Field. */
	u16	nEthertype;
	/** CTAG VLAN Tag Priority Field.
	Only used when adding VLAN tag is
	enabled (eHeaderAdd=IFX_ETHSW_CPU_HEADER_VLAN). */
	u8	nVLAN_Prio;
	/** CTAG VLAN Tag Canonical Format Identifier.
	Only used when adding VLAN tag is
	enabled (eHeaderAdd=IFX_ETHSW_CPU_HEADER_VLAN). */
	u8	nVLAN_CFI;
	/** CTAG VLAN Tag VID.
	Only used when adding VLAN tag is
	enabled (eHeaderAdd=IFX_ETHSW_CPU_HEADER_VLAN). */
	u16	nVLAN_ID;
} IFX_ETHSW_CPU_Header_t;

/** CPU port PAUSE frame handling.
	Used by \ref IFX_ETHSW_CPU_PortExtendCfg_t. */
typedef enum {
	/** Forward all PAUSE frames coming from the switch macro towards
	the DMA channel. These frames do not influence the packet transmission. */
	IFX_ETHSW_CPU_PAUSE_FORWARD	= 0,
	/** Dispatch all PAUSE frames coming from the switch macro towards
	the DMA channel. These are filtered out and the packets transmission is
	stopped and restarted accordingly. */
	IFX_ETHSW_CPU_PAUSE_DISPATCH	= 1
} IFX_ETHSW_CPU_Pause_t;

/** Ethernet port interface mode.
	A port might support only a subset of the possible settings.
	Used by \ref IFX_ETHSW_portLinkCfg_t. */
typedef enum {
	/** Normal PHY interface (twisted pair), use the internal MII Interface. */
	IFX_ETHSW_PORT_HW_MII	= 0,
	/** Reduced MII interface in normal mode. */
	IFX_ETHSW_PORT_HW_RMII	= 1,
	/** GMII or MII, depending upon the speed. */
	IFX_ETHSW_PORT_HW_GMII	= 2,
	/** RGMII mode. */
	IFX_ETHSW_PORT_HW_RGMII	= 3
} IFX_ETHSW_MII_Mode_t;

/** Ethernt port configuration for PHY or MAC mode.
	Used by \ref IFX_ETHSW_portLinkCfg_t. */
typedef enum {
	/** MAC Mode. The Ethernet port is configured to work in MAC mode. */
	IFX_ETHSW_PORT_MAC	= 0,
	/** PHY Mode. The Ethernet port is configured to work in PHY mode. */
	IFX_ETHSW_PORT_PHY	= 1
} IFX_ETHSW_MII_Type_t;

/** Ethernet port clock source configuration.
	Used by \ref IFX_ETHSW_portLinkCfg_t. */
typedef enum {
	/** Clock Mode not applicable. */
	IFX_ETHSW_PORT_CLK_NA	= 0,
	/** Clock Master Mode. The port is configured to provide the clock as output signal. */
	IFX_ETHSW_PORT_CLK_MASTER	= 1,
	/** Clock Slave Mode. The port is configured to use the input clock signal. */
	IFX_ETHSW_PORT_CLK_SLAVE	= 2
} IFX_ETHSW_clkMode_t;

/** Additional CPU port configuration for platforms where the CPU port is
	fixed set on a dedicated port.

	Used by \ref IFX_ETHSW_CPU_PORT_EXTEND_CFG_SET
	and \ref IFX_ETHSW_CPU_PORT_EXTEND_CFG_GET. */
typedef struct {
	/** Add Ethernet layer-2 header (also CTAG VLAN) to the transmit packet.
	The corresponding header fields are set in 'sHeader'. */
	IFX_ETHSW_CPU_HeaderMode_t	eHeaderAdd;
	/** Remove Ethernet layer-2 header (also CTAG VLAN) for packets going from
	Ethernet switch to the DMA. Only the first VLAN tag found is removed
	and additional available VLAN tags remain untouched. */
	ltq_bool_t	bHeaderRemove;
	/** Ethernet layer-2 header information. Used when adding a header to the
	transmitted packet. The parameter 'eHeaderAdd' selects the mode if
	a layer-2 header should be added (including VLAN).
	This structure contains all fields of the Ethernet and VLAN header. */
	IFX_ETHSW_CPU_Header_t	sHeader;
	/** Describes how the port handles received PAUSE frames coming from the
	switch. Either forward them to DMA or stop/start transmission.
	Note that the parameter 'eFlowCtrl' of the
	command 'IFX_ETHSW_PORT_CFG_SET' determines whether the switch
	generates PAUSE frames. */
	IFX_ETHSW_CPU_Pause_t	ePauseCtrl;
	/** Remove the CRC (FCS) of all packets coming from the switch towards
	the DMA channel.
	Note that the FCS check and generation option can be configured
	using 'IFX_ETHSW_CPU_PORT_CFG_SET'. */
	ltq_bool_t	bFcsRemove;
	/** Port map of Ethernet switch ports that are assigned to the WAN side
	(dedicated for applications where ports are grouped into WAN- and
	LAN- segments). All ports that are not selected belong to the LAN segment.
	The LSB bit represents port 0, the higher bits represent the higher
	port numbers. */
	u32	nWAN_Ports;
} IFX_ETHSW_CPU_PortExtendCfg_t;

/** Ethernet port link, speed status and flow control status.
    Used by \ref IFX_ETHSW_PORT_LINK_CFG_GET
    and \ref IFX_ETHSW_PORT_LINK_CFG_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Force Port Duplex Mode.
		- LTQ_FALSE: Negotiate Duplex Mode. Auto-negotiation mode.
			Negotiated duplex mode given in 'eDuplex' during
			IFX_ETHSW_PORT_LINK_CFG_GET calls.
		- LTQ_TRUE: Force Duplex Mode. Force duplex mode in 'eDuplex'.
	*/
	ltq_bool_t	bDuplexForce;
	/** Port Duplex Status. */
	IFX_ETHSW_portDuplex_t	eDuplex;
	/** Force Link Speed.
		- LTQ_FALSE: Negotiate Link Speed. Negotiated speed given in
				'eSpeed' during IFX_ETHSW_PORT_LINK_CFG_GET calls.
		- LTQ_TRUE: Force Link Speed. Forced speed mode in 'eSpeed'.
	*/
	ltq_bool_t	bSpeedForce;
	/** Ethernet port link up/down and speed status. */
	IFX_ETHSW_portSpeed_t	eSpeed;
	/** Force Link.
	- LTQ_FALSE: Auto-negotiate Link. Current link status is given in
		'eLink' during IFX_ETHSW_PORT_LINK_CFG_GET calls.
	- LTQ_TRUE: Force Duplex Mode. Force duplex mode in 'eLink'.
	*/
	ltq_bool_t	bLinkForce;
	/** Link Status. Read out the current link status.
	Note that the link could be forced by setting 'bLinkForce'. */
	IFX_ETHSW_portLink_t	eLink;
	/** Selected interface mode (MII/RMII/RGMII/GMII). */
	IFX_ETHSW_MII_Mode_t	eMII_Mode;
	/** Select MAC or PHY mode (PHY = Reverse xMII). */
	IFX_ETHSW_MII_Type_t	eMII_Type;
	/** Interface Clock mode (used for RMII mode). */
	IFX_ETHSW_clkMode_t	eClkMode;
	/** 'Low Power Idle' Support for 'Energy Efficient Ethernet'.
	Only enable this feature in case the attached PHY also supports it. */
	ltq_bool_t	bLPI;
} IFX_ETHSW_portLinkCfg_t;

/** Ethernet Interface RGMII Clock Configuration. Only needed in case the
	interface runs in RGMII mode.
	Used by \ref IFX_ETHSW_PORT_RGMII_CLK_CFG_SET
	and \ref IFX_ETHSW_PORT_RGMII_CLK_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Clock Delay RX [multiple of 500 ps]. */
	u8	nDelayRx;
	/** Clock Delay TX [multiple of 500 ps]. */
	u8	nDelayTx;
} IFX_ETHSW_portRGMII_ClkCfg_t;

/** Query whether the Ethernet switch hardware has detected a connected
	PHY on the port.
	Used by \ref IFX_ETHSW_PORT_PHY_QUERY. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Check if the Ethernet switch hardware has detected a connected PHY
	on this port. */
	ltq_bool_t	bPHY_Present;
} IFX_ETHSW_portPHY_Query_t;

/** Ethernet PHY address definition. Defines the relationship between a
	bridge port and the MDIO address of a PHY that is attached to this port.
	Used by \ref IFX_ETHSW_PORT_PHY_ADDR_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Device address on the MDIO interface */
	u8	nAddressDev;
} IFX_ETHSW_portPHY_Addr_t;

/** Port redirection control.
	Used by \ref IFX_ETHSW_PORT_REDIRECT_GET
	and \ref IFX_ETHSW_PORT_REDIRECT_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Port Redirect Option.
	If enabled, all packets destined to 'nPortId' are redirected to the
	CPU port. The destination port map in the status header information is
	not changed so that the original destination port can be identified by
	software. */
	ltq_bool_t	bRedirectEgress;
	/** Port Ingress Direct Forwarding.
	If enabled, all packets sourced from 'nPortId' are directly forwarded to queue 0
	of the CPU port. These packets are not modified and are not affected by
	normal learning, look up, VLAN processing and queue selection. */
	ltq_bool_t	bRedirectIngress;
} IFX_ETHSW_portRedirectCfg_t;

/** Port monitor configuration.
	Used by \ref IFX_ETHSW_MONITOR_PORT_CFG_GET
	and \ref IFX_ETHSW_MONITOR_PORT_CFG_SET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** This port is used as a monitor port. To use this feature, the port
	mirror function is enabled on one or more ports. */
	ltq_bool_t	bMonitorPort;
} IFX_ETHSW_monitorPortCfg_t;

/** RMON Counters - Type 1.
	This structure contains the RMON counters of one Ethernet Switch Port.
	Used by \ref IFX_ETHSW_RMON_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. This parameter specifies for which MAC port the RMON1
	counter is read. It has to be set by the application before
	calling \ref IFX_ETHSW_RMON_GET. */
	u8	nPortId;
	/** Receive Packet Count (only packets that are accepted and not discarded). */
	u32	nRxGoodPkts;
	/** Receive Unicast Packet Count. */
	u32	nRxUnicastPkts;
	/** Receive Broadcast Packet Count. */
	u32	nRxBroadcastPkts;
	/** Receive Multicast Packet Count. */
	u32	nRxMulticastPkts;
	/** Receive FCS Error Packet Count. */
	u32	nRxFCSErrorPkts;
	/** Receive Undersize Good Packet Count. */
	u32	nRxUnderSizeGoodPkts;
	/** Receive Oversize Good Packet Count. */
	u32	nRxOversizeGoodPkts;
	/** Receive Undersize Error Packet Count. */
	u32	nRxUnderSizeErrorPkts;
	/** Receive Good Pause Packet Count. */
	u32	nRxGoodPausePkts;
	/** Receive Oversize Error Packet Count. */
	u32	nRxOversizeErrorPkts;
	/** Receive Align Error Packet Count. */
	u32	nRxAlignErrorPkts;
	/** Filtered Packet Count. */
	u32	nRxFilteredPkts;
	/** Receive Size 64 Packet Count. */
	u32	nRx64BytePkts;
	/** Receive Size 65-127 Packet Count. */
	u32	nRx127BytePkts;
	/** Receive Size 128-255 Packet Count. */
	u32	nRx255BytePkts;
	/** Receive Size 256-511 Packet Count. */
	u32	nRx511BytePkts;
	/** Receive Size 512-1023 Packet Count. */
	u32	nRx1023BytePkts;
	/** Receive Size 1024-1522 (or more, if configured) Packet Count. */
	u32	nRxMaxBytePkts;
	/** Transmit Packet Count. */
	u32	nTxGoodPkts;
	/** Transmit Unicast Packet Count. */
	u32	nTxUnicastPkts;
	/** Transmit Broadcast Packet Count. */
	u32	nTxBroadcastPkts;
	/** Transmit Multicast Packet Count. */
	u32	nTxMulticastPkts;
	/** Transmit Single Collision Count. */
	u32	nTxSingleCollCount;
	/** Transmit Multiple Collision Count. */
	u32	nTxMultCollCount;
	/** Transmit Late Collision Count. */
	u32	nTxLateCollCount;
	/** Transmit Excessive Collision Count. */
	u32	nTxExcessCollCount;
	/** Transmit Collision Count. */
	u32	nTxCollCount;
	/** Transmit Pause Packet Count. */
	u32	nTxPauseCount;
	/** Transmit Size 64 Packet Count. */
	u32	nTx64BytePkts;
	/** Transmit Size 65-127 Packet Count. */
	u32	nTx127BytePkts;
	/** Transmit Size 128-255 Packet Count. */
	u32	nTx255BytePkts;
	/** Transmit Size 256-511 Packet Count. */
	u32	nTx511BytePkts;
	/** Transmit Size 512-1023 Packet Count. */
	u32	nTx1023BytePkts;
	/** Transmit Size 1024-1522 (or more, if configured) Packet Count. */
	u32	nTxMaxBytePkts;
	/** Transmit Drop Packet Count. */
	u32	nTxDroppedPkts;
	/** Transmit Dropped Packet Count, based on Congestion Management. */
	u32	nTxAcmDroppedPkts;
	/** Receive Dropped Packet Count. */
	u32	nRxDroppedPkts;
	/** Receive Good Byte Count (64 bit). */
	unsigned long long int	nRxGoodBytes;
	/** Receive Bad Byte Count (64 bit). */
	unsigned long long int	nRxBadBytes;
	/** Transmit Good Byte Count (64 bit). */
	unsigned long long int	nTxGoodBytes;
} IFX_ETHSW_RMON_cnt_t;

/** RMON Counter Clear.
	This structure specifies on which port the RMON counter should be deleted.
	Used by \ref IFX_ETHSW_RMON_CLEAR. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
} IFX_ETHSW_RMON_clear_t;

/** MDIO Interface Configuration.
	Used by \ref IFX_ETHSW_MDIO_CFG_GET and \ref IFX_ETHSW_MDIO_CFG_SET. */
typedef struct {
	/** MDIO interface clock and data rate [in kHz]. */
	u32	nMDIO_Speed;
	/** MDIO interface enable. */
	ltq_bool_t	bMDIO_Enable;
} IFX_ETHSW_MDIO_cfg_t;

/** MDIO Register Access.
	The 'nData' value is directly written to the device register
	or read from the device.
	Some PHY device registers have standard bit definitions as stated in
	IEEE 802.
	Used by \ref IFX_ETHSW_MDIO_DATA_READ and \ref IFX_ETHSW_MDIO_DATA_WRITE. */
typedef struct {
	/** Device address on the MDIO interface */
	u8	nAddressDev;
	/** Register address inside the device. */
	u8	nAddressReg;
	/** Exchange data word with the device (read / write). */
	u16	nData;
} IFX_ETHSW_MDIO_data_t;

/** MMD Register Access. The 'nData' value is directly written
	to the device register or read from the device. Some PHY
	device registers have standard bit definitions as stated in
	IEEE 802. Used by \ref IFX_ETHSW_MMD_DATA_READ and \ref
	IFX_ETHSW_MMD_DATA_WRITE. */
typedef struct {
	/** Device address on the MDIO interface */
	u8	nAddressDev;
	/** MMD Register address/offset inside the device. */
	u32	nAddressReg;
	/** Exchange data word with the device (read / write). */
	u16	nData;
} IFX_ETHSW_MMD_data_t;

/** Enumeration for function status return. The upper four bits are reserved for
    error classification */
typedef enum {
	IFX_ETHSW_statusOk	= 0,
	/** Invalid function parameter */
	IFX_ETHSW_statusParam	= -2,
	/** No space left in VLAN table */
	IFX_ETHSW_statusVLAN_Space	= -3,
	/** Requested VLAN ID not found in table */
	IFX_ETHSW_statusVLAN_ID	= -4,
	/** Invalid ioctl */
	IFX_ETHSW_statusInvalIoctl	= -5,
	/** Operation not supported by hardware */
	IFX_ETHSW_statusNoSupport	= -6,
	/** Timeout */
	IFX_ETHSW_statusTimeout	= -7,
	/** At least one value is out of range */
	IFX_ETHSW_statusValueRange	= -8,
	/** The PortId/QueueId/etc. is not available in this hardware or the
	selected feature is not available on this port */
	IFX_ETHSW_statusPortInvalid	= -9,
	/** The interrupt is not available in this hardware */
	IFX_ETHSW_statusIRQ_Invalid	= -10,
	/** The MAC table is full, an entry could not be added */
	IFX_ETHSW_statusMAC_TableFull	= -11,
	/** Generic or unknown error occurred */
	IFX_ETHSW_statusErr	= -1
} IFX_ETHSW_status_t;

/** Configures the Wake-on-LAN function.
    Used by \ref IFX_ETHSW_WOL_CFG_SET and \ref IFX_ETHSW_WOL_CFG_GET. */
typedef struct {
	/** WoL MAC address. */
	u8	nWolMAC[IFX_MAC_ADDRESS_LENGTH];
	/** WoL password. */
	u8	nWolPassword[IFX_MAC_ADDRESS_LENGTH];
	/** WoL password enable. */
	ltq_bool_t	bWolPasswordEnable;
} IFX_ETHSW_WoL_Cfg_t;

/** Enables Wake-on-LAN functionality on the port.
	Used by \ref IFX_ETHSW_WOL_PORT_CFG_SET
	and \ref IFX_ETHSW_WOL_PORT_CFG_GET. */
typedef struct {
	/** Ethernet Port number (zero-based counting). The valid range is hardware
	dependent. An error code is delivered if the selected port is not
	available. */
	u8	nPortId;
	/** Enable Wake-on-LAN. */
	ltq_bool_t	bWakeOnLAN_Enable;
} IFX_ETHSW_WoL_PortCfg_t;

/*@}*/ /* ETHSW_IOCTL_OAM */

/* -------------------------------------------------------------------------- */
/*                        IOCTL Command Definitions                           */
/* -------------------------------------------------------------------------- */

/** \addtogroup ETHSW_IOCTL_BRIDGE */
/*@{*/

/**
   Read an entry of the MAC table.
   If the parameter 'bInitial=TRUE', the GET operation starts at the beginning
   of the table. Otherwise it continues the GET operation at the entry that
   follows the previous access.
   The function sets all fields to zero in case the end of the table is reached.
   In order to read out the complete table, this function can be called in a loop.
   The Switch API sets 'bLast=LTQ_TRUE' when the last entry is read out.
   This 'bLast' parameter could be the loop exit criteria.

   \param IFX_ETHSW_MAC_tableRead_t Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableRead_t structure that is filled out by the switch
   implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MAC_TABLE_ENTRY_READ	_IOWR(IFX_ETHSW_MAGIC, 0x01, IFX_ETHSW_MAC_tableRead_t)

/**
   Search the MAC Address table for a specific address entry.
   A MAC address is provided by the application and Switch API
   performs a search operation on the hardware table.
   Many hardware platforms provide an optimized and fast address search algorithm.

   \param IFX_ETHSW_MAC_tableQuery_t Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableQuery_t structure that is filled out by the switch
   implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MAC_TABLE_ENTRY_QUERY	_IOWR(IFX_ETHSW_MAGIC, 0x02, IFX_ETHSW_MAC_tableQuery_t)

/**
   Add a MAC table entry. If an entry already exists for the given MAC Address
   and Filtering Database (FID), this entry is overwritten. If not,
   a new entry is added.

   \param IFX_ETHSW_MAC_tableAdd_t Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableAdd_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MAC_TABLE_ENTRY_ADD	_IOW(IFX_ETHSW_MAGIC, 0x03, IFX_ETHSW_MAC_tableAdd_t)

/**
   Remove a single MAC entry from the MAC table.

   \param IFX_ETHSW_MAC_tableRemove_t Pointer to a MAC table entry
   \ref IFX_ETHSW_MAC_tableRemove_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MAC_TABLE_ENTRY_REMOVE	_IOW(IFX_ETHSW_MAGIC, 0x04, IFX_ETHSW_MAC_tableRemove_t)

/**
	Remove all MAC entries from the MAC table.

	\param void This command does not require any parameter structure

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_MAC_TABLE_CLEAR	_IO(IFX_ETHSW_MAGIC, 0x05)

/**
   Configure the Spanning Tree Protocol state of an Ethernet port.
   The switch supports four Spanning Tree Port states (Disable/Discarding,
   Blocking/Listening, Learning and Forwarding state) for every port, to enable
   the Spanning Tree Protocol function when co-operating with software on
   the CPU port.
   Identified Spanning Tree Protocol packets can be redirected to the CPU port.
   Depending on the hardware implementation, the CPU port assignement is fixed
   or can be configured using \ref IFX_ETHSW_CPU_PORT_CFG_SET.
   The current port state can be read back
   using \ref IFX_ETHSW_STP_PORT_CFG_GET.

	\param IFX_ETHSW_STP_portCfg_t Pointer to \ref IFX_ETHSW_STP_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_STP_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x06, IFX_ETHSW_STP_portCfg_t)

/**
	Read out the current Spanning Tree Protocol state of an Ethernet port.
	This configuration can be set using \ref IFX_ETHSW_STP_PORT_CFG_SET.

	\param IFX_ETHSW_STP_portCfg_t Pointer to \ref IFX_ETHSW_STP_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_STP_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x07, IFX_ETHSW_STP_portCfg_t)

/**
   Set the Spanning Tree configuration. This configuration includes the
   filtering of detected spanning tree packets. These packets could be
   redirected to one dedicated port (e.g. CPU port) or they could be discarded.
   The current configuration can be read using \ref IFX_ETHSW_STP_BPDU_RULE_GET.

	\param IFX_ETHSW_STP_BPDU_Rule_t Pointer to \ref IFX_ETHSW_STP_BPDU_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_STP_BPDU_RULE_SET	_IOW(IFX_ETHSW_MAGIC, 0x08, IFX_ETHSW_STP_BPDU_Rule_t)

/**
   Read the Spanning Tree configuration.
   The configuration can be modified using \ref IFX_ETHSW_STP_BPDU_RULE_SET.

	\param IFX_ETHSW_STP_BPDU_Rule_t Pointer to \ref IFX_ETHSW_STP_BPDU_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	 - An error code in case an error occurs
*/
#define IFX_ETHSW_STP_BPDU_RULE_GET	_IOWR(IFX_ETHSW_MAGIC, 0x09, IFX_ETHSW_STP_BPDU_Rule_t)

/**
   Read the IEEE 802.1x filter configuration.
   The parameters can be modified using \ref IFX_ETHSW_8021X_EAPOL_RULE_SET.

	\param IFX_ETHSW_8021X_EAPOL_Rule_t Pointer to \ref IFX_ETHSW_8021X_EAPOL_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_8021X_EAPOL_RULE_GET	_IOR(IFX_ETHSW_MAGIC, 0x0A, IFX_ETHSW_8021X_EAPOL_Rule_t)

/**
   Set the IEEE 802.1x filter rule for a dedicated port. Filtered packets can be
   redirected to one dedicated port (e.g. CPU port).
   The switch supports the addition of a specific packet header to the filtered packets
   that contains information like source port, priority and so on.
   The parameters can be read using \ref IFX_ETHSW_8021X_EAPOL_RULE_GET.

	\param IFX_ETHSW_8021X_EAPOL_Rule_t Pointer to \ref IFX_ETHSW_8021X_EAPOL_Rule_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_8021X_EAPOL_RULE_SET	_IOW(IFX_ETHSW_MAGIC, 0x0B, IFX_ETHSW_8021X_EAPOL_Rule_t)

/**
	Get the 802.1x port status for a switch port.
	A configuration can be set using \ref IFX_ETHSW_8021X_PORT_CFG_SET

	\param IFX_ETHSW_8021X_portCfg_t Pointer to a
		802.1x port authorized state port
		configuration \ref IFX_ETHSW_8021X_portCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_8021X_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x0C, IFX_ETHSW_8021X_portCfg_t)

/**
	Set the 802.1x port status for a switch port.
	The port configuration can be read using \ref IFX_ETHSW_8021X_PORT_CFG_GET.

	\param IFX_ETHSW_8021X_portCfg_t Pointer to a
	802.1x port authorized state port
	configuration \ref IFX_ETHSW_8021X_portCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_8021X_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x0D, IFX_ETHSW_8021X_portCfg_t)

/**
   Read out the current port trunking algorithm that is used to retrieved if
   a packet is sent on the lower or higher trunking port index number. The algorithm
   performs an hash calculation over the MAC- and IP- addresses using the
   source- and destination- fields. This command retrieve which of the
   mentioned fields is used by the hash algorithm.
   The usage of any field could be configured over
   the \ref IFX_ETHSW_TRUNKING_CFG_SET command.

	\param IFX_ETHSW_trunkingCfg_t Pointer to a
		configuration \ref IFX_ETHSW_trunkingCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_TRUNKING_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x65, IFX_ETHSW_trunkingCfg_t)

/**
	Configure the current port trunking algorithm that is used to retrieved if
	a packet is sent on the lower or higher trunking port index number. The algorithm
	performs an hash calculation over the MAC- and IP- addresses using the
	source- and destination- fields. This command retrieve which of the
	mentioned fields is used by the hash algorithm.
	The usage of any field could be configured over
	the \ref IFX_ETHSW_TRUNKING_CFG_SET command.

	\param IFX_ETHSW_trunkingCfg_t Pointer to a
		configuration \ref IFX_ETHSW_trunkingCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_TRUNKING_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x66, IFX_ETHSW_trunkingCfg_t)

/**
	Read out the port trunking state of a given physical Ethernet switch port 'nPortId'.
	Switch API sets the boolean flag 'bAggregateEnable' and the aggregated trunking
	port 'nAggrPortId' in case trunking is enabled on the port.

	Port trunking can be configures by using the command \ref IFX_ETHSW_TRUNKING_PORT_CFG_GET.

	\param IFX_ETHSW_trunkingPortCfg_t Pointer to a
		configuration \ref IFX_ETHSW_trunkingPortCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_TRUNKING_PORT_CFG_GET	_IOW(IFX_ETHSW_MAGIC, 0x67, IFX_ETHSW_trunkingPortCfg_t)

/**
	Configure the port trunking on two physical Ethernet switch ports.
	A new port trunking group of two groups could be placed or removed.
	The two port index number are given with the parameter 'nPortId' and 'nAggrPortId'.

	The current trunking port state can be read out by using the command \ref IFX_ETHSW_TRUNKING_PORT_CFG_GET.

	\param IFX_ETHSW_trunkingPortCfg_t Pointer to a
		configuration \ref IFX_ETHSW_trunkingPortCfg_t

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_TRUNKING_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x68, IFX_ETHSW_trunkingPortCfg_t)

/*@}*/ /* ETHSW_IOCTL_BRIDGE */

/** \addtogroup ETHSW_IOCTL_VLAN */
/*@{*/

/**
	Add VLAN ID to a reserved VLAN list.
	The switch supports replacing the VID of received packets with the PVID of
	the receiving port. This function adds a VID to the list of VIDs to replace.
	All switch devices support adding VID=0, VID=1 and VID=FFF to be replaced.
	Some devices also allow adding other VIDs to be replaced.
	An added VID could be removed again by
	calling \ref IFX_ETHSW_VLAN_RESERVED_REMOVE.
	This configuration applies to the whole switch device.

	\param IFX_ETHSW_VLAN_reserved_t Pointer to
		an \ref IFX_ETHSW_VLAN_reserved_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_RESERVED_ADD	_IOW(IFX_ETHSW_MAGIC, 0x0E, IFX_ETHSW_VLAN_reserved_t)

/**
	Remove VLAN ID from a reserved VLAN group list.
	This function removes a VID replacement configuration from the switch
	hardware. This replacement configuration replaces the VID of received
	packets with the PVID of the receiving port. This configuration can be
	added using \ref IFX_ETHSW_VLAN_RESERVED_ADD.
	This configuration applies to the whole switch device.

	\param IFX_ETHSW_VLAN_reserved_t Pointer to
	an \ref IFX_ETHSW_VLAN_reserved_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_RESERVED_REMOVE	_IOW(IFX_ETHSW_MAGIC, 0x0F, IFX_ETHSW_VLAN_reserved_t)

/**
	Get VLAN Port Configuration.
	This function returns the VLAN configuration of the given Port 'nPortId'.

	\param IFX_ETHSW_VLAN_portCfg_t Pointer to an
	\ref IFX_ETHSW_VLAN_portCfg_t structure element. Based on the parameter
	'nPortId', the switch API implementation fills out the remaining structure
	elements.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x10, IFX_ETHSW_VLAN_portCfg_t)

/**
	Set VLAN Port Configuration.
	This function sets the VLAN configuration of the given Port 'nPortId'.

	\param IFX_ETHSW_VLAN_portCfg_t Pointer to an \ref IFX_ETHSW_VLAN_portCfg_t
		structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x11, IFX_ETHSW_VLAN_portCfg_t)

/**
	Add a VLAN ID group to the active VLAN set of the
	Ethernet switch hardware.
	Based on this configuration, VLAN group port members can
	be added using \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD.
	The VLAN ID configuration can be removed again by
	calling \ref IFX_ETHSW_VLAN_ID_DELETE.

	\param IFX_ETHSW_VLAN_IdCreate_t Pointer to
		an \ref IFX_ETHSW_VLAN_IdCreate_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_ID_CREATE	_IOW(IFX_ETHSW_MAGIC, 0x12, IFX_ETHSW_VLAN_IdCreate_t)

/**
	Remove a VLAN ID group from the active VLAN set of the switch
	hardware. The VLAN ID group was set
	using \ref IFX_ETHSW_VLAN_ID_CREATE. A VLAN ID group can only be
	removed when no port group members are currently configured on the hardware.
	This VLAN ID group membership configuration is done
	using \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD and can be
	removed again using \ref IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE.

	\param IFX_ETHSW_VLAN_IdDelete_t Pointer to an
		\ref IFX_ETHSW_VLAN_IdDelete_t structure element.

	\remarks A VLAN ID can only be removed in case it was created by
		\ref IFX_ETHSW_VLAN_ID_CREATE and is currently not assigned
		to any Ethernet port (done using \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD).

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_ID_DELETE	_IOW(IFX_ETHSW_MAGIC, 0x13, IFX_ETHSW_VLAN_IdDelete_t)

/**
	Add Ethernet port to port members of a given CTAG VLAN group.
	The assignment can be removed using \ref IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE.

	\param IFX_ETHSW_VLAN_portMemberAdd_t Pointer to
	an \ref IFX_ETHSW_VLAN_portMemberAdd_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_PORT_MEMBER_ADD	_IOW(IFX_ETHSW_MAGIC, 0x14, IFX_ETHSW_VLAN_portMemberAdd_t)

/**
	Remove Ethernet port from port members of a given CTAG VLAN group.
	This assignment was done using \ref IFX_ETHSW_VLAN_PORT_MEMBER_ADD.

	\param IFX_ETHSW_VLAN_portMemberRemove_t Pointer to
		an \ref IFX_ETHSW_VLAN_portMemberRemove_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_PORT_MEMBER_REMOVE	_IOW(IFX_ETHSW_MAGIC, 0x15, IFX_ETHSW_VLAN_portMemberRemove_t)

/**
   Read out all given CTAG VLAN group port memberships. Every command call
   returns one VLAN and port membership pair with the corresponding
   egress traffic tag behavior. Call the command in a loop till
   Switch API sets the 'bLast' variable to read all VLAN port memberships.
   Please set the 'bInitial' parameter for the first call starting the
   read operation at the beginning of the VLAN table.

	\param IFX_ETHSW_VLAN_portMemberRead_t Pointer to
		an \ref IFX_ETHSW_VLAN_portMemberRead_t structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_PORT_MEMBER_READ	_IOR(IFX_ETHSW_MAGIC, 0x16, IFX_ETHSW_VLAN_portMemberRead_t)

/**
   Read out the FID of a given CTAG VLAN ID.
   This VLAN ID can be added using \ref IFX_ETHSW_VLAN_ID_CREATE.
   This function returns an error in case no valid configuration is
   available for the given VLAN ID.

	\param IFX_ETHSW_VLAN_IdGet_t Pointer to \ref IFX_ETHSW_VLAN_IdGet_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_ID_GET	_IOWR(IFX_ETHSW_MAGIC, 0x17, IFX_ETHSW_VLAN_IdGet_t)

/**
	This function initializes the VLAN membership and the egress tagged
	portmap of all unconfigured VLAN groups. A VLAN group is defined as
	unconfigured in case it is not configured
	through \ref IFX_ETHSW_VLAN_ID_CREATE.

	\param IFX_ETHSW_VLAN_memberInit_t Pointer to \ref IFX_ETHSW_VLAN_memberInit_t.

	\remarks
	This API is only supported in case the device support 4k VLAN entries
	(capability retrieved by \ref IFX_ETHSW_CAP_GET).
	Devices with a smaller VLAN group table do not support this API,
	because that table does not contain unconfigured entries.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_VLAN_MEMBER_INIT	_IOW(IFX_ETHSW_MAGIC, XX, IFX_ETHSW_VLAN_memberInit_t)

/**
	Get The current STAG VLAN global device configuration.
	The configuration can be modified by \ref IFX_ETHSW_SVLAN_CFG_SET.

	\param IFX_ETHSW_SVLAN_cfg_t Pointer to an \ref IFX_ETHSW_SVLAN_cfg_t
		structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_SVLAN_CFG_GET	 _IOWR(IFX_ETHSW_MAGIC, 0x71, IFX_ETHSW_SVLAN_cfg_t)

/**
	Set STAG VLAN global device configuration.
	The current configuration can be retrieved by \ref IFX_ETHSW_SVLAN_CFG_GET.

	\param IFX_ETHSW_SVLAN_cfg_t Pointer to an \ref IFX_ETHSW_SVLAN_cfg_t
	structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_SVLAN_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x72, IFX_ETHSW_SVLAN_cfg_t)

/**
	Get STAG VLAN Port Configuration.
	This function returns the STAG VLAN configuration of the given Port 'nPortId'.

	\param IFX_ETHSW_SVLAN_portCfg_t Pointer to an
	\ref IFX_ETHSW_SVLAN_portCfg_t structure element. Based on the parameter
	'nPortId', the switch API implementation fills out the remaining structure
	elements.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_SVLAN_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x73, IFX_ETHSW_SVLAN_portCfg_t)

/**
	Set STAG VLAN Port Configuration.
	This function sets the STAG VLAN configuration of the given Port 'nPortId'.

	\param IFX_ETHSW_SVLAN_portCfg_t Pointer to an \ref IFX_ETHSW_SVLAN_portCfg_t
		structure element.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_SVLAN_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x74, IFX_ETHSW_SVLAN_portCfg_t)

/*@}*/ /* ETHSW_IOCTL_VLAN */

/** \addtogroup ETHSW_IOCTL_QOS */
/*@{*/

/**
	Configures the Ethernet port based traffic class assignment of ingress packets.
	It is used to identify the packet priority and the related egress
	priority queue. For DSCP, the priority to queue assignment is done
	using \ref IFX_ETHSW_QOS_DSCP_CLASS_SET.
	For VLAN, the priority to queue assignment is done
	using \ref IFX_ETHSW_QOS_PCP_CLASS_SET. The current port configuration can be
	read using \ref IFX_ETHSW_QOS_PORT_CFG_GET.

	\param IFX_ETHSW_QoS_portCfg_t Pointer to a
		QOS port priority control configuration \ref IFX_ETHSW_QoS_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x18, IFX_ETHSW_QoS_portCfg_t)

/**
   Read out the current Ethernet port traffic class of ingress packets.
   It is used to identify the packet priority and the related egress
   priority queue. The port configuration can be set
   using \ref IFX_ETHSW_QOS_PORT_CFG_SET.

   \param IFX_ETHSW_QoS_portCfg_t Pointer to a
      QOS port priority control configuration \ref IFX_ETHSW_QoS_portCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x19, IFX_ETHSW_QoS_portCfg_t)

/**
   Initialize the QoS 64 DSCP mapping to the switch priority queues.
   This configuration applies for the whole switch device. The table
   configuration can be read using \ref IFX_ETHSW_QOS_DSCP_CLASS_GET.

   \param IFX_ETHSW_QoS_DSCP_ClassCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_DSCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_DSCP_CLASS_SET	_IOW(IFX_ETHSW_MAGIC, 0x1A, IFX_ETHSW_QoS_DSCP_ClassCfg_t)

/**
   Read out the QoS 64 DSCP mapping to the switch priority queues.
   The table configuration can be set using \ref IFX_ETHSW_QOS_DSCP_CLASS_SET.

   \param IFX_ETHSW_QoS_DSCP_ClassCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_DSCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_DSCP_CLASS_GET	_IOWR(IFX_ETHSW_MAGIC, 0x1B, IFX_ETHSW_QoS_DSCP_ClassCfg_t)

/**
   Configure the PCP to traffic class mapping table.
   This configuration applies to the entire switch device.
   The table configuration can be read using \ref IFX_ETHSW_QOS_PCP_CLASS_GET.

   \param IFX_ETHSW_QoS_PCP_ClassCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_PCP_CLASS_SET	_IOW(IFX_ETHSW_MAGIC, 0x1C, IFX_ETHSW_QoS_PCP_ClassCfg_t)

/**
   Read out the PCP to traffic class mapping table.
   The table configuration can be set using \ref IFX_ETHSW_QOS_PCP_CLASS_SET.

   \param IFX_ETHSW_QoS_PCP_ClassCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_PCP_CLASS_GET	_IOWR(IFX_ETHSW_MAGIC, 0x1D, IFX_ETHSW_QoS_PCP_ClassCfg_t)

/**
   Configures the DSCP to Drop Precedence assignment mapping table.
   This mapping table is used to identify the switch internally used drop
   precedence based on the DSCP value of the incoming packet.
   The current mapping table configuration can be read
   using \ref IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_GET.

   \param IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t Pointer to the QoS
   DSCP drop precedence parameters
   \ref IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_SET _IOW(IFX_ETHSW_MAGIC, 0x1E, IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t)

/**
   Read out the current DSCP to Drop Precedence assignment mapping table.
   The table can be configured
   using \ref IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_SET.

   \param IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t Pointer to the QoS
   DSCP drop precedence parameters
   \ref IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_DSCP_DROP_PRECEDENCE_CFG_GET _IOWR(IFX_ETHSW_MAGIC, 0x1F, IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t)

/**
   Port Remarking Configuration. Ingress and Egress remarking options for
   DSCP and PCP. Remarking is done either on the used traffic class or
   the drop precedence.
   The current configuration can be read
   using \ref IFX_ETHSW_QOS_PORT_REMARKING_CFG_GET.

   \param IFX_ETHSW_QoS_portRemarkingCfg_t Pointer to the remarking configuration
   \ref IFX_ETHSW_QoS_portRemarkingCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_PORT_REMARKING_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x20, IFX_ETHSW_QoS_portRemarkingCfg_t)

/**
   Read out the Port Remarking Configuration. Ingress and Egress remarking options for
   DSCP and PCP. Remarking is done either on the used traffic class or
   the drop precedence.
   The current configuration can be set
   using \ref IFX_ETHSW_QOS_PORT_REMARKING_CFG_SET.

   \param IFX_ETHSW_QoS_portRemarkingCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_portRemarkingCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_PORT_REMARKING_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x21, IFX_ETHSW_QoS_portRemarkingCfg_t)

/**
   Configure the traffic class to DSCP mapping table.
   This table is global and valid for the entire switch device.
   The table can be read using \ref IFX_ETHSW_QOS_CLASS_DSCP_GET.

   \param IFX_ETHSW_QoS_ClassDSCP_Cfg_t Pointer to the DSCP mapping parameter
   \ref IFX_ETHSW_QoS_ClassDSCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_CLASS_DSCP_SET	_IOW(IFX_ETHSW_MAGIC, 0x22, IFX_ETHSW_QoS_ClassDSCP_Cfg_t)

/**
   Read out the current traffic class to DSCP mapping table.
   The table can be written using \ref IFX_ETHSW_QOS_CLASS_DSCP_SET.

   \param IFX_ETHSW_QoS_ClassDSCP_Cfg_t Pointer to the DSCP mapping parameter
   \ref IFX_ETHSW_QoS_ClassDSCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_CLASS_DSCP_GET	_IOWR(IFX_ETHSW_MAGIC, 0x23, IFX_ETHSW_QoS_ClassDSCP_Cfg_t)

/**
   Configure the traffic class to 802.1P (PCP) priority mapping table.
   This table is global and valid for the entire switch device.
   The table can be read using \ref IFX_ETHSW_QOS_CLASS_PCP_GET.

   \param IFX_ETHSW_QoS_ClassPCP_Cfg_t Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_ClassPCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_CLASS_PCP_SET	_IOWR(IFX_ETHSW_MAGIC, 0x24, IFX_ETHSW_QoS_ClassPCP_Cfg_t)

/**
   Read out the current traffic class to 802.1P (PCP) priority mapping table.
   This table is global and valid for the entire switch device.
   The table can be written using \ref IFX_ETHSW_QOS_CLASS_PCP_SET.

   \param IFX_ETHSW_QoS_ClassPCP_Cfg_t Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_ClassPCP_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_CLASS_PCP_GET	_IOWR(IFX_ETHSW_MAGIC, 0x25, IFX_ETHSW_QoS_ClassPCP_Cfg_t)

/** This command configures a rate shaper instance with the rate and the
    burst size. This instance can be assigned to QoS queues by
    using \ref IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.

	\param IFX_ETHSW_QoS_ShaperCfg_t Pointer to the parameters
	structure \ref IFX_ETHSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SHAPER_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x26, IFX_ETHSW_QoS_ShaperCfg_t)

/** This command retrieves the rate and the burst size configuration of a
    rate shaper instance. A configuration can be modified
    using \ref IFX_ETHSW_QOS_SHAPER_CFG_SET.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_ShaperCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SHAPER_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x27, IFX_ETHSW_QoS_ShaperCfg_t)

/** Assign one rate shaper instance to a QoS queue. The function returns with an
    error in case there already are too many shaper instances assigned to a queue.
    The queue instance can be enabled and configured
    using \ref IFX_ETHSW_QOS_SHAPER_CFG_SET.
    To remove a rate shaper instance from a QoS queue,
    please use \ref IFX_ETHSW_QOS_SHAPER_QUEUE_DEASSIGN.
    The total number of available rate shaper instances can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_ShaperQueue_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN	_IOW(IFX_ETHSW_MAGIC, 0x28, IFX_ETHSW_QoS_ShaperQueue_t)

/** Deassign one rate shaper instance from a QoS queue. The function returns
    with an error in case the requested instance is not currently assigned
    to the queue.
    The queue instance can be enabled and configured by
    using \ref IFX_ETHSW_QOS_SHAPER_CFG_SET.
    To assign a rate shaper instance to a QoS queue,
    please use \ref IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_ShaperQueue_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperQueue_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SHAPER_QUEUE_DEASSIGN        _IOW(IFX_ETHSW_MAGIC, 0x29, IFX_ETHSW_QoS_ShaperQueue_t)

/** Check whether a rate shaper instance is assigned to the egress queue.
    The egress queue index is the function input parameter.
    The switch API sets the boolean parameter 'bAssigned == LTQ_TRUE' in case a
    rate shaper is assigned and then sets 'nRateShaperId' to describe the rater
    shaper instance.
    The parameter 'bAssigned == LTQ_FALSE' in case no rate shaper instance
    is currently assigned to the queue instance.
    The commands \ref IFX_ETHSW_QOS_SHAPER_QUEUE_ASSIGN allow a
    rate shaper instance to be assigned, and \ref IFX_ETHSW_QOS_SHAPER_CFG_SET allows
    for configuration of a shaper instance.
    The total number of available rate shapers can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_ShaperQueueGet_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_ShaperQueueGet_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SHAPER_QUEUE_GET	_IOWR(IFX_ETHSW_MAGIC, 0x2A, IFX_ETHSW_QoS_ShaperQueueGet_t)

/** Configures the global WRED drop probability profile and thresholds of the device.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_WRED_Cfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_WRED_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x2B, IFX_ETHSW_QoS_WRED_Cfg_t)

/** Read out the global WRED drop probability profile and thresholds of the device.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_WRED_Cfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_Cfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_WRED_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x2C, IFX_ETHSW_QoS_WRED_Cfg_t)

/** Configures the WRED drop thresholds for a dedicated egress queue.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.
    The command \ref IFX_ETHSW_QOS_WRED_QUEUE_CFG_GET retrieves the current
    configuration.

   \param IFX_ETHSW_QoS_WRED_QueueCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_WRED_QUEUE_CFG_SET	 _IOW(IFX_ETHSW_MAGIC, 0x2D, IFX_ETHSW_QoS_WRED_QueueCfg_t)

/** Read out the WRED drop thresholds for a dedicated egress queue.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.
    The configuration can be changed by
    using \ref IFX_ETHSW_QOS_WRED_QUEUE_CFG_SET.

   \param IFX_ETHSW_QoS_WRED_QueueCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_QueueCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_WRED_QUEUE_CFG_GET	 _IOWR(IFX_ETHSW_MAGIC, 0x2E, IFX_ETHSW_QoS_WRED_QueueCfg_t)

/** Configures the WRED drop thresholds for a dedicated egress port.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.
    The command \ref IFX_ETHSW_QOS_WRED_PORT_CFG_GET retrieves the current
    configuration.

   \param IFX_ETHSW_QoS_WRED_PortCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_WRED_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x69, IFX_ETHSW_QoS_WRED_PortCfg_t)

/** Read out the WRED drop thresholds for a dedicated egress port.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.
    The configuration can be changed by
    using \ref IFX_ETHSW_QOS_WRED_PORT_CFG_SET.

   \param IFX_ETHSW_QoS_WRED_PortCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_WRED_PortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_WRED_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x6a, IFX_ETHSW_QoS_WRED_PortCfg_t)

/** Configures the global flow control thresholds for conforming and non-conforming packets.
    The configured thresholds apply to the global switch segment buffer.
    The current configuration can be retrieved by \ref IFX_ETHSW_QOS_FLOWCTRL_CFG_GET.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_FlowCtrlCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_FLOWCTRL_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x6b, IFX_ETHSW_QoS_FlowCtrlCfg_t)

/** Read out the global flow control thresholds for conforming and non-conforming packets.
    The configured thresholds apply to the global switch segment buffer.
    The configuration can be changed by \ref IFX_ETHSW_QOS_FLOWCTRL_CFG_SET.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_FlowCtrlCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_FLOWCTRL_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x6c, IFX_ETHSW_QoS_FlowCtrlCfg_t)

/** Configures the ingress port flow control thresholds for occupied buffer segments.
    The current configuration can be retrieved by \ref IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_GET.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_FlowCtrlPortCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET        _IOW(IFX_ETHSW_MAGIC, 0x6d, IFX_ETHSW_QoS_FlowCtrlPortCfg_t)

/** Read out the ingress port flow control thresholds for occupied buffer segments.
    The configuration can be changed by \ref IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_SET.
    Given parameters are rounded to the segment size of the HW platform.
    The supported segment size is given by the capability list by
    using \ref IFX_ETHSW_CAP_GET.

   \param IFX_ETHSW_QoS_FlowCtrlPortCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_FlowCtrlPortCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_FLOWCTRL_PORT_CFG_GET        _IOWR(IFX_ETHSW_MAGIC, 0x6e, IFX_ETHSW_QoS_FlowCtrlPortCfg_t)

/** This command configures the parameters of a rate meter instance.
    This instance can be assigned to an ingress/egress port by
    using \ref IFX_ETHSW_QOS_METER_PORT_ASSIGN. It can also be used by the
    flow classification engine.
    The total number of available rate meters can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.
    The current configuration of a meter instance can be retrieved
    using \ref IFX_ETHSW_QOS_METER_CFG_GET.

   \param IFX_ETHSW_QoS_meterCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_METER_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x2F, IFX_ETHSW_QoS_meterCfg_t)

/** Configure the parameters of a rate meter instance.
    This instance can be assigned to an ingress/egress port
    using \ref IFX_ETHSW_QOS_METER_PORT_ASSIGN. It can also be used by the
    flow classification engine.
    The total number of available rate meters can be retrieved by the
    capability list using \ref IFX_ETHSW_CAP_GET.
    The current configuration of a meter instance can be retrieved
    using \ref IFX_ETHSW_QOS_METER_CFG_GET.

   \param IFX_ETHSW_QoS_meterCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_METER_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x30, IFX_ETHSW_QoS_meterCfg_t)

/** Assign a rate meter instance to an ingress and/or egress port.
    A maximum of two meter IDs can be assigned to one single ingress port.
    This meter instance to port assignment can be removed
    using \ref IFX_ETHSW_QOS_METER_PORT_DEASSIGN. A list of all available
    assignments can be read using \ref IFX_ETHSW_QOS_METER_PORT_GET.

   \param IFX_ETHSW_QoS_meterPort_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterPort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_METER_PORT_ASSIGN	_IOW(IFX_ETHSW_MAGIC, 0x31, IFX_ETHSW_QoS_meterPort_t)

/** Deassign a rate meter instance from an ingress and/or egress port.
    A maximum of two meter IDs can be assigned to one single ingress port.
    The meter instance is given to the command and the port configuration is
    returned. An instance to port assignment can be done
    using \ref IFX_ETHSW_QOS_METER_PORT_ASSIGN. A list of all available
    assignments can be read using \ref IFX_ETHSW_QOS_METER_PORT_GET.

   \param IFX_ETHSW_QoS_meterPort_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterPort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_METER_PORT_DEASSIGN	_IOW(IFX_ETHSW_MAGIC, 0x32, IFX_ETHSW_QoS_meterPort_t)

/** Reads out all meter instance to port assignments that are done
    using \ref IFX_ETHSW_QOS_METER_PORT_ASSIGN. All assignments are read from an
    internal table where every read call retrieves the next entry of the table.
    Setting the parameter 'bInitial' starts the read operation at the beginning
    of the table. The returned parameter 'bLast' indicates that the last
    element of the table was returned.

   \param IFX_ETHSW_QoS_meterPortGet_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_meterPortGet_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_METER_PORT_GET	_IOWR(IFX_ETHSW_MAGIC, 0x33, IFX_ETHSW_QoS_meterPortGet_t)

/** This command configures one meter instances for storm control.
    These instances can be used for ingress broadcast-, multicast- and
    unknown unicast- packets. Some platforms support addition of additional meter
    instances for this type of packet.
    Repeated calls of \ref IFX_ETHSW_QOS_STORM_CFG_SET allow addition of
    additional meter instances.
    An assignment can be retrieved using \ref IFX_ETHSW_QOS_STORM_CFG_GET.
    Setting the broadcast, multicast and unknown unicast packets boolean switch to zero
    deletes all metering instance assignments.

   \param IFX_ETHSW_QoS_stormCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_stormCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_STORM_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x34, IFX_ETHSW_QoS_stormCfg_t)

/** Reads out the current meter instance assignment for storm control. This
    configuration can be modified using \ref IFX_ETHSW_QOS_STORM_CFG_SET.

   \param IFX_ETHSW_QoS_stormCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_stormCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_STORM_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x35, IFX_ETHSW_QoS_stormCfg_t)

/** This configuration decides how the egress queues, attached to a single port,
    are scheduled to transmit the queued Ethernet packets.
    The configuration differentiates between 'Strict Priority' and
    'weighted fair queuing'. This applies when multiple egress queues are
    assigned to an Ethernet port.
    Using the WFQ feature on a port requires the configuration of weights on all
    given queues that are assigned to that port.
    Strict Priority means that no dedicated weight is configured and the
    queue can transmit following its priority status.
    The given configuration can be read out
    using \ref IFX_ETHSW_QOS_SCHEDULER_CFG_GET.

   \param IFX_ETHSW_QoS_schedulerCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SCHEDULER_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x36, IFX_ETHSW_QoS_schedulerCfg_t)

/** Read out the current scheduler configuration of a given egress port. This
    configuration can be modified
    using \ref IFX_ETHSW_QOS_SCHEDULER_CFG_SET.

   \param IFX_ETHSW_QoS_schedulerCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_schedulerCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SCHEDULER_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x37, IFX_ETHSW_QoS_schedulerCfg_t)

/** Sets the Queue ID for one traffic class of one port.
    The total amount of supported ports, queues and traffic classes can be
    retrieved from the capability list using \ref IFX_ETHSW_CAP_GET.
    Please note that the device comes along with a
    default configuration and assignment.

   \param IFX_ETHSW_QoS_queuePort_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_queuePort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_QUEUE_PORT_SET	_IOW(IFX_ETHSW_MAGIC, 0x38, IFX_ETHSW_QoS_queuePort_t)

/** Read out the traffic class and port assignment done
    using \ref IFX_ETHSW_QOS_QUEUE_PORT_SET.
    Please note that the device comes along with a
    default configuration and assignment.

   \param IFX_ETHSW_QoS_queuePort_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_queuePort_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_QUEUE_PORT_GET	_IOWR(IFX_ETHSW_MAGIC, 0x39, IFX_ETHSW_QoS_queuePort_t)
/** Configure the egress queue buffer reservation.
    WRED GREEN packets are never dropped by any WRED algorithm (queue,
    port or global buffer level) in case they are below this reservation threshold.
    The amount of reserved segments cannot be occupied by other queues of the switch.
    The egress queue related configuration can be retrieved by
    calling \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET.

    \remarks
    The command \ref IFX_ETHSW_QOS_QUEUE_PORT_SET allows to assign egress queue to ports with related traffic classes.

   \param IFX_ETHSW_QoS_QueueBufferReserveCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET _IOW(IFX_ETHSW_MAGIC, 0x6f, IFX_ETHSW_QoS_QueueBufferReserveCfg_t)

/** Read out the egress queue specific buffer reservation.
    Configuration can be read by \ref IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_SET.

   \param IFX_ETHSW_QoS_QueueBufferReserveCfg_t Pointer to the parameters
   structure \ref IFX_ETHSW_QoS_QueueBufferReserveCfg_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_QUEUE_BUFFER_RESERVE_CFG_GET _IOWR(IFX_ETHSW_MAGIC, 0x70, IFX_ETHSW_QoS_QueueBufferReserveCfg_t)
/**
   Configure the egress port related traffic class to STAG VLAN 802.1P (PCP) priority mapping table.
   One table is given for each egress port.
   The table can be read using \ref IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_GET.

   \param IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_SET	_IOWR(IFX_ETHSW_MAGIC, 0x75, IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t)

/**
   Read out the current egress port related traffic class to 802.1P (PCP) priority mapping table.
   One table is given for each egress port.
   The table can be written using \ref IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_SET.

   \param IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t Pointer to the PCP priority mapping parameter
   \ref IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SVLAN_CLASS_PCP_PORT_GET	_IOWR(IFX_ETHSW_MAGIC, 0x76, IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t)

/**
   Configure the PCP to traffic class mapping table.
   This configuration applies to the entire switch device.
   The table configuration can be read using \ref IFX_ETHSW_QOS_SVLAN_PCP_CLASS_GET.

   \param IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SVLAN_PCP_CLASS_SET	_IOW(IFX_ETHSW_MAGIC, 0x77, IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t)

/**
   Read out the PCP to traffic class mapping table.
   The table configuration can be set using \ref IFX_ETHSW_QOS_SVLAN_PCP_CLASS_SET.

   \param IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t Pointer to the QoS filter parameters
   \ref IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_QOS_SVLAN_PCP_CLASS_GET	_IOWR(IFX_ETHSW_MAGIC, 0x78, IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t)

/*@}*/ /* ETHSW_IOCTL_QOS */

/** \addtogroup ETHSW_IOCTL_MULTICAST */
/*@{*/

/**
   Configure the switch multicast configuration. The currently used
   configuration can be read using \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_GET.

   \param IFX_ETHSW_multicastSnoopCfg_t Pointer to the
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
#define IFX_ETHSW_MULTICAST_SNOOP_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x3A, IFX_ETHSW_multicastSnoopCfg_t)

/**
   Read out the current switch multicast configuration.
   The configuration can be set using \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_SET.

   \param IFX_ETHSW_multicastSnoopCfg_t Pointer to the
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
#define IFX_ETHSW_MULTICAST_SNOOP_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x3B, IFX_ETHSW_multicastSnoopCfg_t)

/**
   Add static router port to the switch hardware multicast table.
   These added router ports will not be removed by the router port learning aging process.
   The router port learning is enabled over the parameter 'bLearningRouter'
   over the \ref IFX_ETHSW_MULTICAST_SNOOP_CFG_GET command.
   Router port learning and static added entries can both be used together.
   In case of a sofware IGMP stack/daemon environemtn, the router port learning does
   not have to be configured on the switch hardware. Instead the router port
   management is handled by the IGMP stack/daemon.
   A port can be removed using \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE.

   \param IFX_ETHSW_multicastRouter_t Pointer to \ref IFX_ETHSW_multicastRouter_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD	_IOW(IFX_ETHSW_MAGIC, 0x3C, IFX_ETHSW_multicastRouter_t)

/**
   Remove an Ethernet router port from the switch hardware multicast table.
   A port can be added using \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD.

   \param IFX_ETHSW_multicastRouter_t Pointer to \ref IFX_ETHSW_multicastRouter_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs (e.g. Ethernet port parameter out of range)
*/
#define IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE	_IOW(IFX_ETHSW_MAGIC, 0x3D, IFX_ETHSW_multicastRouter_t)

/**
   Check if a port has been selected as a router port, either by automatic learning or by manual setting.
   A port can be added using \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_ADD.
   A port can be removed again using \ref IFX_ETHSW_MULTICAST_ROUTER_PORT_REMOVE.

   \param IFX_ETHSW_multicastRouterRead_t Pointer to \ref IFX_ETHSW_multicastRouterRead_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs (e.g. Ethernet port parameter out of range)
*/
#define IFX_ETHSW_MULTICAST_ROUTER_PORT_READ	_IOWR(IFX_ETHSW_MAGIC, 0x3E, IFX_ETHSW_multicastRouterRead_t)

/**
   Adds a multicast group configuration to the multicast table.
   No new entry is added in case this multicast group already
   exists in the table. This commands adds a host member to
   the multicast group.
   A member can be removed again using \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE.

   \param IFX_ETHSW_multicastTable_t Pointer
      to \ref IFX_ETHSW_multicastTable_t.

   \remarks The Source IP parameter is ignored in case IGMPv3 support is
      not enabled in the hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD	_IOW(IFX_ETHSW_MAGIC, 0x3F, IFX_ETHSW_multicastTable_t)

/**
   Remove an host member from a multicast group. The multicast group entry
   is completely removed from the multicast table in case it has no
   host member port left.
   Group members can be added using \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_ADD.

   \param IFX_ETHSW_multicastTable_t Pointer
      to \ref IFX_ETHSW_multicastTable_t.

   \remarks The Source IP parameter is ignored in case IGMPv3 support is
      not enabled in the hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MULTICAST_TABLE_ENTRY_REMOVE	_IOWR(IFX_ETHSW_MAGIC, 0x40, IFX_ETHSW_multicastTable_t)

/**
   Read out the multicast membership table that is located inside the switch
   hardware. The 'bInitial' parameter restarts the read operation at the beginning of
   the table. Every following \ref IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ call reads out
   the next found entry. The 'bLast' parameter is set by the switch API in case
   the last entry of the table is reached.

   \param IFX_ETHSW_multicastTableRead_t Pointer
      to \ref IFX_ETHSW_multicastTableRead_t.

   \remarks The 'bInitial' parameter is reset during the read operation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ	_IOWR(IFX_ETHSW_MAGIC, 0x41, IFX_ETHSW_multicastTableRead_t)

/*@}*/ /* ETHSW_IOCTL_MULTICAST */

/** \addtogroup ETHSW_IOCTL_OAM */
/*@{*/

/** Hardware Initialization. This command should be called right after the
    Switch API software module is initialized and loaded.
    It accesses the hardware platform, retrieving platform capabilities and
    performing the first basic configuration.

   \param IFX_ETHSW_HW_Init_t Pointer to pre-allocated initialization structure
   \ref IFX_ETHSW_HW_Init_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_HW_INIT	_IOW(IFX_ETHSW_MAGIC, 0x42, IFX_ETHSW_HW_Init_t)

/**
   Retrieve the version string of the currently version index. The returned
   string format might vary between the device platforms used. This
   means that the version information cannot be compared between different
   device platforms.
   All returned version information is in the form of zero-terminated character strings.
   The returned strings are empty ('') in case the given version
   index is out of range.

   \param IFX_ETHSW_version_t* The parameter points to a
   \ref IFX_ETHSW_version_t structure.

   \return Returns value as follows:
   - LTQ_SUCCESS: if successful
   - LTQ_ERROR: in case of an error

   \code
   IFX_ETHSW_version_t param;
   int fd;

   memset (&param, 0, sizeof(IFX_ETHSW_version_t));

   while (1) {
		if (ioctl(fd, IFX_ETHSW_VERSION_GET, (int) &param) != LTQ_SUCCESS) {
			printf("ERROR: TAPI version request failed!\n);
			return LTQ_ERROR;
		}
		if ((strlen(param.cName) == 0) || (strlen(param.cVersion) == 0))
			// No more version entries found
			break;
		printf("%s version: %s", param.cName, param.cVersion);
		param.nId++;
	}
	return LTQ_SUCCESS;
	\endcode
*/
#define IFX_ETHSW_VERSION_GET	_IOWR(IFX_ETHSW_MAGIC, 0x43, IFX_ETHSW_version_t)

/** This service returns the capability referenced by the provided index
    (zero-based counting index value). The Switch API uses the index to return
    the capability parameter from an internal list. For instance,
    the capability list contains information about the amount of supported
    features like number of supported VLAN groups or MAC table entries.
    The command returns zero-length strings ('') in case the
    requested index number is out of range.

	\param IFX_ETHSW_cap_t Pointer to pre-allocated capability
		list structure \ref IFX_ETHSW_cap_t.
		The switch API implementation fills out the structure with the supported
		features, based on the provided 'nCapType' parameter.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs

	\code
	IFX_ETHSW_cap_t param;
	int fd;
	int i;

   // Open tapi file descriptor *
	fd = open("/dev/switchapi/1", O_RDWR, 0x644);

	for (i = 0; i < IFX_ETHSW_CAP_TYPE_LAST, i++) {
		memset(&param, 0, sizeof(param));
		param.nCapType = i;
		//Get the cap list
		if (ioctl(fd, IFX_ETHSW_CAP_GET, (int) &param) == LTQ_ERROR)
			return LTQ_ERROR;
		printf("%s: %d\n", param.cDesc, param.nCap);
	}
	// Close open fd
	close(fd);
	return LTQ_SUCCESS;
	\endcode
*/
#define IFX_ETHSW_CAP_GET	_IOWR(IFX_ETHSW_MAGIC, 0x44, IFX_ETHSW_cap_t)

/**
   Modify the switch configuration.
   The configuration can be read using \ref IFX_ETHSW_CFG_GET.
   The switch can be enabled using \ref IFX_ETHSW_ENABLE.

   \param IFX_ETHSW_cfg_t Pointer to an \ref IFX_ETHSW_cfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x45, IFX_ETHSW_cfg_t)

/**
   Read the global switch configuration.
   This configuration can be set using \ref IFX_ETHSW_CFG_SET.

   \param IFX_ETHSW_cfg_t Pointer to an \ref IFX_ETHSW_cfg_t structure.
      The structure is filled out by the switch implementation.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x46, IFX_ETHSW_cfg_t)

/**
   Enables the whole switch. The switch device is enabled with the default
   configuration in case no other configuration is applied.
   The switch can be disabled using the \ref IFX_ETHSW_DISABLE command

   \param void This command does not require any parameter structure

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_ENABLE	_IO(IFX_ETHSW_MAGIC, 0x47)

/**
   Disables the whole switch.
   The switch can be enabled using the \ref IFX_ETHSW_ENABLE command

   \param void This command does not require any parameter structure

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_DISABLE	_IO(IFX_ETHSW_MAGIC, 0x48)

/**
   Read out the current Ethernet port configuration.

   \param IFX_ETHSW_portCfg_t Pointer to a port configuration
   \ref IFX_ETHSW_portCfg_t structure to fill out by the driver.
   The parameter 'nPortId' tells the driver which port parameter is requested.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x49, IFX_ETHSW_portCfg_t)

/**
   Set the Ethernet port configuration.

   \param IFX_ETHSW_portCfg_t Pointer to an \ref IFX_ETHSW_portCfg_t structure
   to configure the switch port hardware.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x4A, IFX_ETHSW_portCfg_t)

/**
   Defines one port that is directly connected to the software running on a CPU.
   This allows for the redirecting of protocol-specific packets to the CPU port and
   special packet treatment when sent by the CPU.
   If the CPU port cannot be set, the function returns an error code.

   \param IFX_ETHSW_CPU_PortCfg_t Pointer to
      an \ref IFX_ETHSW_CPU_PortCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_CPU_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x4B, IFX_ETHSW_CPU_PortCfg_t)

/**
   Get the port that is directly connected to the software running on a CPU and defined as
   CPU port. This port assignment can be set using \ref IFX_ETHSW_CPU_PORT_CFG_SET
   if it is not fixed and defined by the switch device architecture.

   \param IFX_ETHSW_CPU_PortCfg_t Pointer to
      an \ref IFX_ETHSW_CPU_PortCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_CPU_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x4C, IFX_ETHSW_CPU_PortCfg_t)

/**
   Configure an additional CPU port configuration. This configuration applies to
   devices where the CPU port is fixed to one dedicated port.

   \param IFX_ETHSW_CPU_PortExtendCfg_t Pointer to
      an \ref IFX_ETHSW_CPU_PortExtendCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_CPU_PORT_EXTEND_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x4D, IFX_ETHSW_CPU_PortExtendCfg_t)

/**
   Reads out additional CPU port configuration. This configuration applies to
   devices where the CPU port is fixed to one dedicated port.

   \param IFX_ETHSW_CPU_PortExtendCfg_t Pointer to
      an \ref IFX_ETHSW_CPU_PortExtendCfg_t structure.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

*/
#define IFX_ETHSW_CPU_PORT_EXTEND_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x4E, IFX_ETHSW_CPU_PortExtendCfg_t)

/**
   Read out the Ethernet port's speed, link status, and flow control status.
   The information for one single port 'nPortId' is returned.
   An error code is returned if the selected port does not exist.

   \param IFX_ETHSW_portLinkCfg_t Pointer to
      an \ref IFX_ETHSW_portLinkCfg_t structure to read out the port status.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_LINK_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x4F, IFX_ETHSW_portLinkCfg_t)

/**
   Set the Ethernet port link, speed status and flow control status.
   The configuration applies to a single port 'nPortId'.

   \param IFX_ETHSW_portLinkCfg_t Pointer to
      an \ref IFX_ETHSW_portLinkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_LINK_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x50, IFX_ETHSW_portLinkCfg_t)

/**
   Configure the RGMII clocking parameter in case the Ethernet port is configured in RGMII mode.
   The configuration can be read by calling \ref IFX_ETHSW_PORT_RGMII_CLK_CFG_GET.
   It applies to a single port 'nPortId'.

   \param IFX_ETHSW_portRGMII_ClkCfg_t Pointer to
      an \ref IFX_ETHSW_portRGMII_ClkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_RGMII_CLK_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x51, IFX_ETHSW_portRGMII_ClkCfg_t)

/**
   Read the RGMII clocking parameter in case the Ethernet port is configured in RGMII mode.
   The configuration can be set by calling \ref IFX_ETHSW_PORT_RGMII_CLK_CFG_SET.
   It applies to a single port 'nPortId'.

   \param IFX_ETHSW_portRGMII_ClkCfg_t Pointer to
      an \ref IFX_ETHSW_portRGMII_ClkCfg_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_RGMII_CLK_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x52, IFX_ETHSW_portRGMII_ClkCfg_t)

/**
   Check whether the Ethernet switch hardware has detected an Ethernet PHY connected
   to the given Ethernet port 'nPortId'.

   \param IFX_ETHSW_portPHY_Query_t Pointer to
      an \ref IFX_ETHSW_portPHY_Query_t structure to set the port configuration.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_PHY_QUERY	_IOWR(IFX_ETHSW_MAGIC, 0x53, IFX_ETHSW_portPHY_Query_t)

/**
	Read out the MDIO device address of an Ethernet PHY that is connected to
	an Ethernet port. This device address is useful when accessing PHY
	registers using the commands \ref IFX_ETHSW_MDIO_DATA_WRITE,
	\ref IFX_ETHSW_MDIO_DATA_READ, \ref IFX_ETHSW_MMD_DATA_WRITE
		and \ref IFX_ETHSW_MMD_DATA_READ.

	\param IFX_ETHSW_portPHY_Addr_t Pointer to \ref IFX_ETHSW_portPHY_Addr_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_PHY_ADDR_GET	_IOWR(IFX_ETHSW_MAGIC, 0x54, IFX_ETHSW_portPHY_Addr_t)

/**
	Ingress and egress packets of one specific Ethernet port can be redirected to
	the CPU port. The ingress and egress packet redirection can be configured
	individually. This command reads out the current configuration of a
	dedicated port. A new configuration can be applied
	by calling \ref IFX_ETHSW_PORT_REDIRECT_SET.

	\param IFX_ETHSW_portRedirectCfg_t Pointer
		to \ref IFX_ETHSW_portRedirectCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.
	\remarks Not all hardware platforms support this feature. The function
		returns an error if this feature is not supported.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_REDIRECT_GET	_IOWR(IFX_ETHSW_MAGIC, 0x55, IFX_ETHSW_portRedirectCfg_t)

/**
   Select ingress and egress packets of one specific Ethernet port that can be
   redirected to a port that is configured as the 'CPU port'. The ingress and
   egress packet direction can be configured individually.
   The packet filter of the original port still
   applies to the packet (for example, MAC address learning is done for the
   selected port and not for the CPU port).
   On CPU port side, no additional learning, forwarding look up,
   VLAN processing and queue selection is performed for redirected packets.
   Depending on the hardware platform used, the CPU port has to be set in
   advance using \ref IFX_ETHSW_CPU_PORT_CFG_SET.
   The currently used configuration can be read
   using \ref IFX_ETHSW_PORT_REDIRECT_GET.

	\param IFX_ETHSW_portRedirectCfg_t Pointer
		to \ref IFX_ETHSW_portRedirectCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.
	\remarks Not all hardware platforms support this feature. The function
		returns an error if this feature is not supported.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_PORT_REDIRECT_SET	_IOW(IFX_ETHSW_MAGIC, 0x56, IFX_ETHSW_portRedirectCfg_t)

/**
   Reads out the current monitor options for a
   dedicated Ethernet port. This configuration can be set
   using \ref IFX_ETHSW_MONITOR_PORT_CFG_SET.

	\param IFX_ETHSW_monitorPortCfg_t Pointer
		to \ref IFX_ETHSW_monitorPortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_MONITOR_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x57, IFX_ETHSW_monitorPortCfg_t)

/**
   Configures the monitor options for a
   dedicated Ethernet port. This current configuration can be read back
   using \ref IFX_ETHSW_MONITOR_PORT_CFG_GET.

	\param IFX_ETHSW_monitorPortCfg_t Pointer
		to \ref IFX_ETHSW_monitorPortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_MONITOR_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x58, IFX_ETHSW_monitorPortCfg_t)

/**
   Read out the Ethernet port statistic counter (RMON counter).
   The zero-based 'nPortId' structure element describes the physical switch
   port for the requested statistic information.

   \param IFX_ETHSW_RMON_cnt_t  Pointer to pre-allocated
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
#define IFX_ETHSW_RMON_GET	_IOWR(IFX_ETHSW_MAGIC, 0x59, IFX_ETHSW_RMON_cnt_t)

/**
   Clears an Ethernet port traffic statistic counter (RMON counter).

   \param IFX_ETHSW_RMON_clear_t  Pointer to a pre-allocated
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
#define IFX_ETHSW_RMON_CLEAR	_IOW(IFX_ETHSW_MAGIC, 0x5A, IFX_ETHSW_RMON_clear_t)

/**
   Read the MDIO interface configuration.
   The parameters can be modified using \ref IFX_ETHSW_MDIO_CFG_SET.

   \param IFX_ETHSW_MDIO_cfg_t Pointer to \ref IFX_ETHSW_MDIO_cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_MDIO_CFG_GET	_IOR(IFX_ETHSW_MAGIC, 0x5B, IFX_ETHSW_MDIO_cfg_t)

/**
   Set the MDIO interface configuration.
   The parameters can be read using \ref IFX_ETHSW_MDIO_CFG_GET.
   The given frequency is rounded off to fitting to the hardware support.
   \ref IFX_ETHSW_MDIO_CFG_GET will return the exact programmed (rounded) frequency value.

   \param IFX_ETHSW_MDIO_cfg_t Pointer to \ref IFX_ETHSW_MDIO_cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_MDIO_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x5C, IFX_ETHSW_MDIO_cfg_t)

/**
	Read data from the MDIO Interface of the switch device. This function allows
	various kinds of information to be read out for any attached device by register and
	device addressing.
	The 'nData' value (\ref IFX_ETHSW_MDIO_data_t) contains the read
	device register.
	A write operation can be done using \ref IFX_ETHSW_MDIO_DATA_WRITE.

	\param IFX_ETHSW_MDIO_data_t Pointer to \ref IFX_ETHSW_MDIO_data_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

	\return Return value as follows:
	- LTQ_SUCCESS: if successful
	- An error code in case an error occurs
*/
#define IFX_ETHSW_MDIO_DATA_READ	_IOWR(IFX_ETHSW_MAGIC, 0x5D, IFX_ETHSW_MDIO_data_t)

/**
   Write data to the MDIO Interface of the switch device. This function allows
   for configuration of any attached device by register and device addressing.
   This applies to external and internal Ethernet PHYs as well.
   The 'nData' value (\ref IFX_ETHSW_MDIO_data_t) is directly written to the
   device register.
   A read operation can be performed using \ref IFX_ETHSW_MDIO_DATA_READ.

   \param IFX_ETHSW_MDIO_data_t Pointer to \ref IFX_ETHSW_MDIO_data_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs

   \code
   IFX_ETHSW_MDIO_data_t         phy;

   // access the device 2
   phy.nAddressDev = 2;
   // PHY register 0
   phy.nAddressReg = 0;
   // copy the helper PHY register union to the data field to configure
   phy.nData = 0x1235;

   if (ioctl(fd, IFX_ETHSW_MDIO_DATA_WRITE, (int)&phy))
      return LTQ_ERROR;

   // access the device 5
   phy.nAddressDev = 5;
   // Device specific register 20
   phy.nAddressReg = 20;
   // set the data field to configure
   phy.nData = 0x1234;

   if (ioctl(fd, IFX_ETHSW_MDIO_DATA_WRITE, (int)&phy))
      return LTQ_ERROR;

   return LTQ_SUCCESS;
   \endcode
*/
#define IFX_ETHSW_MDIO_DATA_WRITE	_IOW(IFX_ETHSW_MAGIC, 0x5E, IFX_ETHSW_MDIO_data_t)

/**
   Read MMD Ethernet PHY register over the MDIO Interface
   attached to the switch device. This function allows various
   kinds of information to be read out for any attached device
   by register and device addressing. The 'nData' value (\ref
   IFX_ETHSW_MMD_data_t) contains the read MMD device register.
   A write operation can be done using \ref
   IFX_ETHSW_MMD_DATA_WRITE.

	\param IFX_ETHSW_MMD_data_t Pointer to
		\ref IFX_ETHSW_MMD_data_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MMD_DATA_READ	_IOWR(IFX_ETHSW_MAGIC, 0x5F, IFX_ETHSW_MMD_data_t)

/**
   Write MMD Ethernet PHY register over the MDIO Interface
   attached to the switch device. This function allows
   configuration of any attached device by MMD register and
   device addressing. This applies to external and internal
   Ethernet PHYs as well. The 'nData' value (\ref
   IFX_ETHSW_MMD_data_t) is directly written to the device
   register. A read operation can be performed using \ref
   IFX_ETHSW_MMD_DATA_READ.

	\param IFX_ETHSW_MMD_data_t Pointer to \ref
		IFX_ETHSW_MMD_data_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_MMD_DATA_WRITE	_IOW(IFX_ETHSW_MAGIC, 0x60, IFX_ETHSW_MMD_data_t)

/**
   Set the Wake-on-LAN configuration.
   The parameters can be read using \ref IFX_ETHSW_WOL_CFG_GET.

   \param IFX_ETHSW_WoL_Cfg_t Pointer to \ref IFX_ETHSW_WoL_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_WOL_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x61, IFX_ETHSW_WoL_Cfg_t)

/**
   Read the Wake-on-LAN configuration.
   The parameters can be modified using \ref IFX_ETHSW_WOL_CFG_SET.

   \param IFX_ETHSW_WoL_Cfg_t Pointer to \ref IFX_ETHSW_WoL_Cfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_WOL_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x62, IFX_ETHSW_WoL_Cfg_t)

/**
   Set the current Wake-On-LAN status for a dedicated port. The
   Wake-On-LAN specific parameter can be configured
   using \ref IFX_ETHSW_WOL_CFG_SET.

   \param IFX_ETHSW_WoL_PortCfg_t Pointer to \ref IFX_ETHSW_WoL_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_WOL_PORT_CFG_SET	_IOW(IFX_ETHSW_MAGIC, 0x63, IFX_ETHSW_WoL_PortCfg_t)

/**
   Read out the current status of the Wake-On-LAN feature
   on a dedicated port. This status can be changed
   using \ref IFX_ETHSW_WOL_PORT_CFG_SET.
   The Wake-On-LAN specific parameter can be configured
   using \ref IFX_ETHSW_WOL_CFG_SET.

   \param IFX_ETHSW_WoL_PortCfg_t Pointer to \ref IFX_ETHSW_WoL_PortCfg_t.

	\remarks The function returns an error code in case an error occurs.
		The error code is described in \ref IFX_ETHSW_status_t.

   \return Return value as follows:
   - LTQ_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define IFX_ETHSW_WOL_PORT_CFG_GET	_IOWR(IFX_ETHSW_MAGIC, 0x64, IFX_ETHSW_WoL_PortCfg_t)

/*@}*/ /* ETHSW_IOCTL_OAM */

#endif    /* _LANTIQ_ETHSW_H_ */
