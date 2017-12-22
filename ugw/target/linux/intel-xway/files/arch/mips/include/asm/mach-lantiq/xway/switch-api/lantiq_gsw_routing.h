/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************
   \file ltq_gsw_routing.h
   \remarks Implement GSWIP-3.0 Routing APIs and Structure Definition.
 *****************************************************************************/

#ifndef _LANTIQ_GSW_ROUTE_H_
#define _LANTIQ_GSW_ROUTE_H_

//#include "lantiq_types.h"
//#include "lantiq_gsw.h"
#include "gsw_types.h"
/* Routing IOCTL MAGIC Number */
#define GSW_ROUTE_MAGIC ('R')

/** \defgroup PAE_API GSWIP-Routing/PAE specific APIs
    \brief  GSWIP-Routing (PAE) APIs definition applicable to GSWIP-3.0 only. 
*/

#if 0
/** \defgroup GSWIP_ROUTE_API GSWIP-R Kernel APIs
    \brief  GSWIP-Routing (PAE) Kernel API definition. 
*/
#endif

/** \addtogroup  PAE_API */
/*@{*/

/** \brief Unused flag status conveyed through value of Zero */
#define GSW_ROUTE_F_NO_STATUS  0
/** \brief Flag to indicate existent Entry got swapped to allow High-Priority entry creation */
#define GSW_ROUTE_F_SWAP_OUT  1

/** \brief Error Code indicating no availability in MTU Table */
#define GSW_ROUTE_ERROR_MTU_FULL  -20
/** \brief Error Code indicating no availability in PPPoE Table */
#define GSW_ROUTE_ERROR_PPPOE_FULL  -21
/** \brief Error Code indicating no availability in RTP Assignment Table */
#define GSW_ROUTE_ERROR_RTP_FULL  -22
/** \brief Error Code indicating no availability in IP Address Table */
#define GSW_ROUTE_ERROR_IP_FULL  -23
/** \brief Error Code indicating no availability in MAC Address Table */
#define GSW_ROUTE_ERROR_MAC_FULL  -24
/** \brief Error Code indicating no availability in Routing Session Table */
#define GSW_ROUTE_ERROR_RT_SESS_FULL  -25
/** \brief Error Code indicating no availability in Collision List */
#define GSW_ROUTE_ERROR_RT_COLL_FULL  -26
#define GSW_ROUTE_F_SWAP_OUT_ERR	-27
/*!
    \brief This is the data structure for PAE 6rd tunnel interface - Outer IPv4.
*/
typedef struct {
	GSW_IP_t nSrcIP4Addr;   /*!< 6rd tunnel Source IPv4 address */
	GSW_IP_t nDstIP4Addr;   /*!< 6rd tunnel Dest IPv4 address */ 
} GSW_ROUTE_Tunnel_6rd;

/*!
    \brief This is the data structure for PAE DSLite tunnel interface - Outer IPv6.
*/
typedef struct {
 GSW_IP_t nSrcIP6Addr;   /*!< DS-Lite tunnel Source IPv6 address */
 GSW_IP_t nDstIP6Addr;   /*!< DS-Lite tunnel Dest IPv6 address */ 
} GSW_ROUTE_Tunnel_DSLite;

/*!
    \brief This is the data structure for PAE Tunnel type selector. Used by  \ref GSW_ROUTE_Session_action_t.
*/
typedef enum 
{
  GSW_ROUTE_TUNL_NULL = 0, /*!< Session routing tunnel type is No Tunnel action */
  GSW_ROUTE_TUNL_6RD = 1, /*!< Session routing tunnel type is 6rd */
  GSW_ROUTE_TUNL_DSLITE = 2, /*!< Session routing tunnel type is DSlite */
  GSW_ROUTE_TUNL_L2TP = 3, /*!< Session routing tunnel type is L2TP */
  GSW_ROUTE_TUNL_IPSEC = 4, /*!< Session routing tunnel type is IPsec */
  
} GSW_ROUTE_Session_Tunmode_t; 

/*!
    \brief This is the data structure for GSWIP Tunnel entry configuration Interface.
*/
typedef struct 
{
   /*@{*/
  GSW_ROUTE_Session_Tunmode_t eTunnelType; /*!< Tunnel type enum for DSLite, 6RD, IPSec, L2TP */	
   /*@}*/
   /**
    * @name Union tunnel
    */
 
   /*@{*/
  union {
  	GSW_ROUTE_Tunnel_6rd    tun6RD; /*!< 6RD tunnel configuration */
  	u32                     nTunL2TP; /*!< L2TP tunnel configuration. Just a placeholder in union, since PAE only supports L2TP detunneling only */
  	u32                     nTunIPsec; /*!< IPsec crypto context configuration. GSWIP action not defined */
  	GSW_ROUTE_Tunnel_DSLite tunDSlite; /*!< DSlite tunnel configuration */
  } t; 
   /*@}*/
} GSW_ROUTE_Tunnel_t;



/*!
    \brief This is the data structure for GSWIP Routing Session Direction. Used by  \ref GSW_ROUTE_Session_action_t.
*/
typedef enum 
{
  GSW_ROUTE_DIRECTION_DNSTREAM = 0, /*!< Session is LAN egress i.e WAN Downstream */
  GSW_ROUTE_DIRECTION_UPSTREAM = 1, /*!< Session is LAN ingress i.e. WAN Upstream session */
} GSW_ROUTE_Session_Direction_t; 


/*!
    \brief This is the data structure for GSWIP Routing mode. Used by  \ref GSW_ROUTE_Session_action_t.
*/
typedef enum 
{
  GSW_ROUTE_MODE_NULL = 0, /*!< Session routing type NULL. Can be used for Bridge sessions in RT table. */
  GSW_ROUTE_MODE_ROUTING = 1, /*!< Session routing type plain routing without NAT/NAPT*/
  GSW_ROUTE_MODE_NAT = 2, /*!< Session routing type is Src IP address NAT*/
  GSW_ROUTE_MODE_NAPT = 3, /*!< Session routing type is Src IP/Port NAT */

} GSW_ROUTE_Session_Routmode_t; 

/*!
    \brief This is the data structure for GSWIP Routing Outer DSCP remarking action. Used by \ref GSW_ROUTE_Session_action_t.
*/
typedef enum 
{
  GSW_ROUTE_OUT_DSCP_NULL = 0, /*!< Session routing no outer DSCP remarking action */
  GSW_ROUTE_OUT_DSCP_INNER = 1, /*!< Session routing outer DSCP from inner IP header */
  GSW_ROUTE_OUT_DSCP_SESSION = 2, /*!< Session routing outer DSCP from session action table */
  GSW_ROUTE_OUT_DSCP_RES = 3, /*!< Session routing outer DSCP action reserved */
  
} GSW_ROUTE_Session_Outer_DSCP_Remarking_t;

/** \brief Routing Session selection for IP Address - IPv4/IPv6/Unused.
    Used by \ref GSW_PCE_pattern_t. */
typedef enum
{
   GSW_RT_IP_DISABLED	= 0, /*!< Routing IP selection disabled. */
   GSW_RT_IP_V4	= 1, /*!< Routing IP selection type is IPv4. */
   GSW_RT_IP_V6	= 2 /*!< Routing IP selection type is IPv6. */
} GSW_RT_IP_t;

/*!
    \brief This is the data structure for GSWIP Routing Session Pattern Interface. SrcIP, DstIP, SrcPort, DstPort & RoutingExtensionId together is used as input keys for lookup of Routing sessions. Except RoutingExtensionId other fields in pattern are optional and decided through PCE rule - IP and Port Compare Selector.
*/
typedef struct 
{
    GSW_RT_IP_t eIpType; /*!< IP Address Type - IPv4 or IPv6 or Unused */
    GSW_IP_t nSrcIP;  /*!< The session source IPv4/v6 address used for hash computation. Internally this gets mapped to Routing IP Addr table. */
    GSW_IP_t nDstIP; /*!< The session destination IPv4/v6 address for hash computation. Internally this gets mapped to Routing IP Addr table.  */ 	    
    u16 nSrcPort;      /*!< TCP/UDP source port information - used in hash computation*/
    u16 nDstPort;      /*!< TCP/UDP destination port information - used in hash computation*/
    u8  nRoutExtId; /*!< Routing extension Id from Flow Table action. Valid value any 8-bit integer also matching to output of Flow rule. Used in hash index computation */
    ltq_bool_t bValid;       /*!< Indicate, if a particular routing entry is valid or not */
} GSW_ROUTE_Session_pattern_t;


/*!
    \brief This is the data structure for GSWIP Routing Session Action Interface.
*/
typedef struct 
{

	u32 nDstPortMap;     /*!< Session destination port map specified in form of bitmask. E.g. Starting with least significant -  0th Bit position denotes port no. 0, 1st bit position denotes port no. 1 and so on. Bit vale of 1 indicates port is specified and 0 indicates port is not specified*/  
	u16 nDstSubIfId;     /*!< Session destination sub-interace Id. For multicast stream this field's GroupIndex part may carry the corresponding Group Id value.  */
	GSW_RT_IP_t eIpType; /*!< IP Address Type - IPv4 or IPv6 or Unused */
	GSW_IP_t nNATIPaddr;  /*!< Session new IP address after NAT for eIPType. */
	u16  nTcpUdpPort;     /*!< Session new TCP/UDP port number. Used if eSessionRoutingMode is NAPT */ 
	u16  nMTUvalue;       /*!< Session MTU value.*/
	ltq_bool_t	bMAC_SrcEnable; /*!< Source MAC address used */
	u8   nSrcMAC[6];      /*!< New source MAC address for the session. */
	ltq_bool_t	bMAC_DstEnable; /*!< Destination MAC address used */
	u8   nDstMAC[6];      /*!< New destination MAC address for the session */  
	ltq_bool_t bPPPoEmode;       /*!< Session PPPoE mode. Value 
                                     0:PPPoE Mode transparent.
                                     1:PPPoE Mode Termination*/
	u16 nPPPoESessId;      /*!< Session PPPoE Session Identifier used */
	ltq_bool_t	bTunnel_Enable; /*!< Tunnel used selector */
	GSW_ROUTE_Session_Tunmode_t eTunType; /*!< Session Tunnel type used*/
	u8  nTunnelIndex;        /*!< Preconfigured tunnel Index. The tunnel Index maps to Tunnel table. First configure Tunnel entry and then set Index here (0..15) */
	ltq_bool_t bMeterAssign;    /*!< MeterId assignment action. Value 
				     0:Assignment disabled.
				     1:Assignment Enabled */
	u8  nMeterId;         /*!< Meter Id used for the session. The metering configuration can be done using differnt switch api function */
	ltq_bool_t bRTPMeasEna;     /*!< RTP Multicast session's sequence number counter Action. Value 
				     0:Counting Disabled .
				     1:Counting Enabled */
	u16 nRTPSeqNumber;      /*!< 16 bit RTP sequence number for which the multicast packet will be counted */
	u16 nRTPSessionPktCnt;  /*!< 16 bit RTP packet Rolling Counter. R-O */
	u8  nFID;  /*!< Session FID. Value 0-63, Session FID is used for Egress VLAN action */
	u8  nFlowId; /*!< Flow Id value. Value 0-255. Default value is 0. */
	GSW_ROUTE_Session_Outer_DSCP_Remarking_t eOutDSCPAction; /*!< Outer DSCP remarking action - Valid for Tunnel associated entries */
	ltq_bool_t bInnerDSCPRemark; /*!< Session routing inner DSCP remarking action. Value 
				      0: No remarking.
				      1: remarking based on session */ 	    
	u8  nDSCP;  /*!< DSCP remarking value for the session */
	ltq_bool_t bTCremarking; /*!< Routing session traffic class remarking action. Value 
				  0: No remarking. 
				  1: TC remarking enabled */
	u8  nTrafficClass; /*!< Traffic class remarking value for the session */
	u32 nSessionCtrs;   /*!< Session MIB Counters. - R-O */ 
	GSW_ROUTE_Session_Direction_t eSessDirection; /*!<Session direction WAN-Downnstream or WAN-Upstream */
	GSW_ROUTE_Session_Routmode_t eSessRoutingMode; /*!< Session routing action mode */	    
	ltq_bool_t bTTLDecrement; /*!< Enable TTL decrement action for the session. Value 
				  0: TTL decrement disabled.
				  1: TTL decrement enabled.*/
	ltq_bool_t  bHitStatus;  /*!< Session hit Status - RW.*/

} GSW_ROUTE_Session_action_t;

/*!
    \brief This is the data structure for GSWIP Routing Session Destination Information.
*/
typedef struct 
{
    u16 nRtIdx;          /*!< Session Index */
    u32 nDstPortMap;     /*!< Session destination port map specified in form of bit-mask with every bit position indicating corresponding port number.*/  
    u16 nDstSubIfId;     /*!< Session destination sub-interace Id.  */
} GSW_ROUTE_Session_Dest_t;

/*!
    \brief This is the data structure for configuring Source L2NAT on egress port of PAE.
*/
typedef struct 
{
    ltq_bool_t bL2NATEna;   /*!< Enable L2NAT on this egress port of PAE*/  
    u16  nEgPortId;     /*!< Egress Port Id */  
    u8   nNatMAC[6];    /*!< New source MAC address for L2NAT on this egress port*/

} GSW_ROUTE_EgPort_L2NAT_Cfg_t;

/*!
    \brief This is the data structure for GSWIP Routing Session Entry.
*/
typedef struct
{
    GSW_ROUTE_Session_pattern_t    pattern; /*!< The structure for routing session pattern parameters */
    GSW_ROUTE_Session_action_t     action;  /*!< The structure for routing session action associated with the above pattern */
} GSW_ROUTE_session_t;

/*!
    \brief This is the data structure for GSWIP Routing Session Table Entry. 
*/
typedef struct
{
  int                  nHashVal; /*!< Routing Session Entry Hash Value - optional (valid range : 0..4095). When not supplied, carries a special value of (-1) */
  int                  nRtIndex; /*!< Routing Session Entry Index Value - returned in ADD operation. Mandatorily, to be passed for the rest of operations */
  GSW_ROUTE_session_t  routeEntry; /*!< Routing Session pattern and action values. */
  ltq_bool_t           bPrio;    /*!< Indicate it is a priority session - mandatorily used in ADD. If set then it can replace normal session if table is full*/
  u32                  nFlags;   /*!< Flags to indicate special status E.g. - Swap done (1), Free (2),  etc. */

} GSW_ROUTE_Entry_t;


/*!
    \brief This is the data structure for GSWIP Routing Tunnel Table Entry. 
*/
typedef struct
{ 
  int                 nTunIndex; /*!< Tunnel table entry index, optional in case of Add operation (-1). When (-1) is supplied the API would return assigned index */
  GSW_ROUTE_Tunnel_t  tunnelEntry; /*!< Tunnel Table Entry */

}GSW_ROUTE_Tunnel_Entry_t; 


/*!
    \brief This is the enumeration for GSWIP Routing Hit Status action. Used by  \ref GSW_ROUTE_Session_Hit_t.
*/
typedef enum 
{
  GSW_ROUTE_HIT_READ = 0, /*!< Session routing Hit Status Read Action */
  GSW_ROUTE_HIT_CLEAR = 1, /*!< Session routing Hit Status Clear Action */
  GSW_ROUTE_HIT_N_CNTR_READ = 2, /*!< Session routing Hit Status & Session Counters Read Action */
  GSW_ROUTE_HIT_N_CNTR_CLEAR = 3 /*!< Session routing Hit Status & Session Counters Clear Action */
}GSW_ROUTE_Session_HitOp_t; 

/*!
    \brief This is the data structure for handling Session Hit Status in PAE.
*/
typedef struct 
{
    GSW_ROUTE_Session_HitOp_t eHitOper; /*!< Session Hit Operation */
    int                       nRtIndex; /*!< Routing Session Index */  
    ltq_bool_t                bHitStatus; /*!< Session Hit Status*/
    u32                       nSessCntr; /*!< Session Counter - Packet or Bytes programmed using RMON API */  
}GSW_ROUTE_Session_Hit_t;

#if 0
/** \addtogroup  GSWIP_ROUTE_API */
/*@{*/

/**
   \brief This function creates a Routing Session entry in the Routing-Session table.
   The pattern part describes the five tuple serving as input key on which hash computation should be done on an incoming packet to which the dedicated actions should be applied.
   A rule can be deleted using the command \ref GSW_ROUTE_SessionEntryDel or read using the command \ref GSW_ROUTE_SessionEntryRead.
   \param[in] pDevCtx device context
   \param[in, out] pRtEntry Pointer to Routing Entry structure \ref GSW_ROUTE_Entry_t. The nHashVal is optional and can be supplied (-1). The nPrio field is carrying the priority information - 1 (Priority session), 0  (Normal session). Upon return of API, nHashVal and nRtIndex carries back computed hash value and Index location. In case of swap of an existent session by a high priority new session, the existing session that got removed from acceleration is returned back in routeEntry struct member. The swap done is informed through nFlag member carrying special value(1).
   \return Return value as follows:
   - Routing session index number >=0 : if successful
   - An error code < 0 in case an error occurs. There has to be detailed error codes covering the maximum reasons : - E.g. Collision List is full, RT_Table full, PPPoE_table full, MTU table full etc.
*/
int GSW_ROUTE_SessionEntryAdd (void *pDevCtx, GSW_ROUTE_Entry_t *pRtEntry);

/**
   \brief This function deletes a Routing Session entry at specififed index in the Routing-Session table.
   A rule can be created using the command \ref GSW_ROUTE_SessionEntryAdd
   \param[in] pDevCtx device context
   \param[in] pRtEntry Routing Session Entry carrying mandatory nRtIndex value.
   \return Return value as follows:
   - GSW_SUCCESS : if successful
   - An error code < 0 in case an error occurs.
*/
int GSW_ROUTE_SessionEntryDel (void *pDevCtx, GSW_ROUTE_Entry_t *pRtEntry);

/**
   \brief This function reads a session entry in the Routing session table at an specified index. The index must be valid entry of the routing index table.
   \param[in] pDevCtx device context.
   \param[in,out] pRtEntry  Pointer to Routing Session Entry structure \ref GSW_ROUTE_Entry_t.
   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code < 0, in case an error occurs
*/
int GSW_ROUTE_SessionEntryRead(void *pDevCtx, GSW_ROUTE_Entry_t *pRtEntry);

/**
   \brief This function creates a tunnel entry in the Tunnel table. For complete configuration of tunnel, it is a multi-step config. Besides tunnel entry creation in tunnel table, it should also be programmed in RoutingSession table.
   A configured tunnel entry can be read using the command \ref GSW_ROUTE_TunnelEntryRead
   \param[in] pDevCtx device context
   \param[in] nTunIdx Tunnel Index number, where the Tunnel entry is stored in the Tunnel Table. A value of -1 is used to indicate the index is not passed by caller. 
   \param[in] pTunnel Pointer to Tunnel structure \ref GSW_ROUTE_Tunnel_t.
   \return Return value as follows:
   - Tunnel Entry index number >=0 : if successful (Number of Tunnels supported are 16 so return value is 0..15 range)
   - An error code < 0 in case an error occurs
*/
int GSW_ROUTE_TunnelEntryAdd(void *pDevCtx, int nTunIdx, GSW_ROUTE_Tunnel_t *pTunnel);

/**
   \brief This function deletes a tunnel entry in the Tunnel table. 
   \param[in] pDevCtx device context
   \param[in] nTunIdx number, where the Tunnel entry is stored in the Tunnel Table. 
   \return Return value as follows:
   - Tunnel Entry index number >=0 : if successful 
   - An error code < 0 in case an error occurs
*/
int GSW_ROUTE_TunnelEntryDel(void *pDevCtx, int nTunIdx);

/**
   \brief This function reads a tunnel entry in the Tunnel table at an specified index. The index must be valid entry of the Tunnel index table.
   \param[in] pDevCtx device context.
   \param[in] nTunIdx Index number, where the tunnel is stored.  
   \param[out] pTunnel Pointer to Tunnel structure \ref GSW_ROUTE_Tunnel_t.
   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code in case an error occurs
*/
int GSW_ROUTE_TunnelEntryRead(void *pDevCtx, int nTunIdx, GSW_ROUTE_Tunnel_t *pTunnel);

/**
   \brief This function configures a Source L2NAT on an egress port.
   A configured tunnel entry can be read using the command \ref GSW_ROUTE_TunnelEntryRead
   \param[in] pDevCtx device context
   \param[in] pL2NatCfg Pointer to Tunnel structure \ref GSW_ROUTE_EgPort_L2NAT_Cfg_t.

   \return Return value as follows:
   - Tunnel Entry index number >=0 : if successful
   - An error code < 0 in case an error occurs
*/
int GSW_ROUTE_L2NATCfgWrite(void *pDevCtx, GSW_ROUTE_EgPort_L2NAT_Cfg_t *pL2NatCfg);

/**
   \brief This function reads currently configured L2NAT entry in the Tunnel table for the specified port. The port number must be a valid number.
   \param[in] pDevCtx device context.
   \param[in,out] pL2NatCfg Pointer to L2NAT Config structure \ref GSW_ROUTE_EgPort_L2NAT_Cfg_t. The port number must be filled in this structure.
   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code in case an error occurs
*/
int GSW_ROUTE_L2NATCfgRead(void *pDevCtx, GSW_ROUTE_EgPort_L2NAT_Cfg_t *pL2NatCfg);

/**
   \brief This function reads or reads-n-clears Session Hit Sttaus for the specified index.
   \param[in] pDevCtx device context.
   \param[in,out] pHitOp Pointer to Session-Hit structure \ref GSW_ROUTE_Session_Hit_t. The index number must be filled in this structure.
   \return Return value as follows:
   - 0: Showing Session is not Hit. 
   - 1: Showing Session is Hit. 
   - (-1): In case of any error in reading or writing Session Hit Status. 
   - An error code in case an error occurs
*/
int GSW_ROUTE_SessHitOp(void *pDevCtx, GSW_ROUTE_Session_Hit_t *pHitOp);

/**
   \brief This function modifies the destination ports of Routing Session.
   \param[in] pDevCtx device context.
   \param[in,out] pDestCfg Pointer to destination structure \ref GSW_ROUTE_Session_Dest_t. 
   \return Return value as follows:
   - GSW_SUCCESS : if successful.
   - An error code in case an error occurs
*/
int GSW_ROUTE_SessDestModify(void *pDevCtx, GSW_ROUTE_Session_Dest_t *pDestCfg);
/*@}*/ /* GSWIP_ROUTE_API */
#endif /* #if 0 */ 

/**
   \brief This command adds a routing session of specified pattern and action. The pattern part describes the parameters to identify an incoming packet session to which the dedicated actions should be applied. Packets having the same pattern field belongs to same session and applied to same action.  A routing rule and action can be read using the command \ref GSW_ROUTE_ENTRY_READ.

   \remarks If a priority session addition is attempted and there is no space for new session then one of the existing normal session would be replaced by this new priority session. The replaced entry is returned in this case through GSW_Route_Entry_t pointer. 
   \param GSW_ROUTE_Entry_t Pointer to \ref GSW_ROUTE_Entry_t containing the session information filled by ioctl caller. This structure carries the config of replaced entry when swapped by high priority entry. 

   \remarks If any error happens during creation of session or configuring the individual unit tables, this error code of negative value is conveyed back to caller. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code <0, in case an error occurs
*/
#define GSW_ROUTE_ENTRY_ADD _IOWR(GSW_ROUTE_MAGIC, 0x01, GSW_ROUTE_Entry_t)

/**
   \brief This command deletes an earlier added routing session of specified index and pattern. It is must specify the index returned during creation. The pattern part is only used for comparing with the pattern stored in index. A routing rule and action of specified index can be read using the command \ref GSW_ROUTE_ENTRY_READ.

   \param GSW_ROUTE_Entry_t Pointer to \ref GSW_ROUTE_Entry_t containing the session information filled by ioctl caller. This structure carries the config of replaced entry when swapped by high prirority entry. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code <0, in case an error occurs E.g. Invalid index, Non-matching pattern at specified index.
*/
#define GSW_ROUTE_ENTRY_DELETE _IOW(GSW_ROUTE_MAGIC, 0x02, GSW_ROUTE_Entry_t)

/**
   \brief This command reads a Routing session config (pattern and action info) for the given index.
   A routing session (pattern and action) can be added using the command \ref GSW_ROUTE_ENTRY_ADD.

   \param GSW_ROUTE_Entry_t Pointer to \ref GSW_ROUTE_Entry_t containing the index. The stored session configuration is returned to caller.


   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code <0, in case an error occurs
*/
#define GSW_ROUTE_ENTRY_READ _IOWR(GSW_ROUTE_MAGIC, 0x03, GSW_ROUTE_Entry_t)


/**
   \brief This command adds a new Tunnel entry in the Routing Tunnel table. The Tunnel entry can be read using the command \ref GSW_ROUTE_TUNNEL_ENTRY_READ.

   \param GSW_ROUTE_Tunnel_Entry_t Pointer to \ref GSW_ROUTE_Tunnel_Entry_t containing the values of a tunnel type and associated attributes. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code <0, in case any error occurs (e.g. Tunnel table is full error)
*/
#define GSW_ROUTE_TUNNEL_ENTRY_ADD _IOW(GSW_ROUTE_MAGIC, 0x04, GSW_ROUTE_Tunnel_Entry_t)

/**
   \brief This command deletes  a specified Tunnel entry in the Routing Tunnel table. The Tunnel entry can be read using the command \ref GSW_ROUTE_TUNNEL_ENTRY_READ.

   \param GSW_ROUTE_Tunnel_Entry_t Pointer to \ref GSW_ROUTE_Tunnel_Entry_t containing the Tunnel-Index and tunnel config values. The values are used to match and perform verification with stored values in Tunnel table at specified index. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code <0, in case any error occurs (e.g. Invalid Tunnel Index, Tunnel value is not matching with what is stored in Tunnel table etc.)
*/
#define GSW_ROUTE_TUNNEL_ENTRY_DELETE _IOW(GSW_ROUTE_MAGIC, 0x05, GSW_ROUTE_Tunnel_Entry_t)

/**
   \brief This command reads the Tunnel values from the routing Tunnel table at a given specified Tunnel Index.
   A Tunnel entry can be written using the command \ref GSW_ROUTE_TUNNEL_ENTRY_ADD.

   \param GSW_ROUTE_Tunnel_Entry_t Pointer to \ref GSW_ROUTE_Tunnel_Entry_t containing the Tunnel-Index as input. The tunnel config values at specified index is returned back to caller. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code <0, in case an error occurs
*/
#define GSW_ROUTE_TUNNEL_ENTRY_READ _IOWR(GSW_ROUTE_MAGIC, 0x06, GSW_ROUTE_Tunnel_Entry_t)

/**
   \brief This command configures L2NAT on egress port of PAE. When enabled the Source MAC Address of 
          traffic leaving specified egress port would be NAT-ed with configured MAC address.

   \param  GSW_ROUTE_EgPort_L2NAT_Cfg_t  L2NAT Attributes as defined in \ref GSW_ROUTE_EgPort_L2NAT_Cfg_t. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code in case any error occurs
*/
#define GSW_ROUTE_L2NAT_CFG_WRITE _IOW(GSW_ROUTE_MAGIC, 0x07, GSW_ROUTE_EgPort_L2NAT_Cfg_t)

/**
   \brief This command reads L2NAT configurations on specified egress port of PAE.

   \param  GSW_ROUTE_EgPort_L2NAT_Cfg_t  L2NAT Attributes as defined in \ref GSW_ROUTE_EgPort_L2NAT_Cfg_t. 

   \return Return value as follows:
   - GSW_SUCCESS: if successful
   - An error code in case an error occurs
*/
#define GSW_ROUTE_L2NAT_CFG_READ _IOWR(GSW_ROUTE_MAGIC, 0x08, GSW_ROUTE_EgPort_L2NAT_Cfg_t)

/**
   \brief This command reads or reads-n-clears Hit-Status for high priority sessions.

   \param  GSW_ROUTE_Session_Hit_t  Hit Status Data struct carrying operation. 

   \return Return value is Session Hit Status:
   - 1 : if session status is Hit (before cleared).
   - 0 : if session status is Not-Hit (before cleared).
   - (-1) : if any error is encountered.
*/

#define GSW_ROUTE_SESSION_HIT_OP _IOWR(GSW_ROUTE_MAGIC, 0x09, GSW_ROUTE_Session_Hit_t) 

/**
   \brief This command modifies the destination of an existing Routing session.

   \param  GSW_ROUTE_Session_Dest_t  Data struct carrying Routing session index and destination portmap & sub-interface configuration. 

   \return Return value is 
   - GSW_SUCCESS : if successful.
   - An error code in case an error occurs
*/

#define GSW_ROUTE_SESSION_DEST_MOD _IOWR(GSW_ROUTE_MAGIC, 0x0A, GSW_ROUTE_Session_Dest_t) 

/*@}*/ /* PAE_API */

#endif /* _LANTIQ_GSW_ROUTE_H_ */
