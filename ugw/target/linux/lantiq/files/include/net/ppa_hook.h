#ifndef __PPA_HOOK_H__20081103_1736__
#define __PPA_HOOK_H__20081103_1736__



/******************************************************************************
**
** FILE NAME    : ppa_hook.h
** PROJECT      : PPA
** MODULES      : PPA Protocol Stack Hooks
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook Pointers Header File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 03 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/
/*! \file ppa_hook.h
    \brief This file contains all exported HOOK API to linux Kernel and user space via ioctl API.
*/



#include <net/ppa_api_common.h>

/** \addtogroup  PPA_HOOK_API */
/*@{*/

/*
 * ####################################
 *             Declaration
 * ####################################
 */

#ifdef __KERNEL__


/*! 
    \brief This command Initialize the PPA module and the embedded acceleration functions.            
    \param[in] info  Pointer to the initialization info structure passed to the API.
    \param[in] flag Reserved currently.
    \return The return value can be any one of the following:  \n
            - PPA_SUCCESS \n
            - PPA_FAILURE
    \note p_info can be passed NULL to the init function wherein default handling will be applied by the PPA. This includes the following behavior. \n 
          - Identifying one LAN interface (eth0) and one WAN interface (eth1) \n 
          - Disabling acceleration in both LAN to WAN and WAN to LAN directions \n 
          - IP header verification enabled \n 
          - TCP/UDP header verification enabled \n 
          - Drop on no hit disabled - pass packets on matching any accelerated session entry to Control CPU \n 
          - Drop Multicast packet on no hit disable \n 
          Please refer to the PPA_INIT_INFO data structure and its members for all the information that is passed to the PPA module on initialization. 
          Specifically, the LAN and WAN interface list can be populated with all possible interfaces of either type expected in the system even if these interfaces don't exist at the time of initialization.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_init_fn)(PPA_INIT_INFO *info, uint32_t flag);
#else
extern int32_t ppa_hook_init_fn(PPA_INIT_INFO *info, uint32_t flag);
#endif
/*! 
    \brief This command un-Initialize the PPA module
    \return void
*/
#ifdef NO_DOXY
extern void (*ppa_hook_exit_fn)(void);
#else
extern void ppa_hook_exit_fn(void);
#endif

/*! 
    \brief This API exposes the PPA Enable / Disable API to userspace.         
    \param[in] lan_rx_ppa_enable Enable / Disable accelerated LAN to WAN path through PPA. \n
               Valid values are:\n
                PPA_ENABLED \n
                PPA_DISABLED \n
    \param[in] wan_rx_ppa_enable Enable / Disable accelerated WAN to LAN path through PPA. \n
               Valid values are:\n
             -   PPA_ENABLED  \n
             -   PPA_DISABLED \n
    \param[in]  flag Reserved currently.

    \return The return value can be any one of the following:  \n
            - PPA_SUCCESS \n
            - PPA_FAILURE
    \note   The LAN to WAN and WAN to LAN separate acceleration disable feature is a function of the acceleration module in the system. In some cases, it maybe only possible to complete disable / enable acceleration.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_enable_fn)(uint32_t lan_rx_ppa_enable, uint32_t wan_rx_ppa_enable, uint32_t flag);
#else
extern int32_t ppa_hook_enable_fn(uint32_t lan_rx_ppa_enable, uint32_t wan_rx_ppa_enable, uint32_t flag);
#endif
/*! 
    \brief This API returns the current Enable / Disable status of the PPA to the caller.           
    \param[out] lan_rx_ppa_enable Enable / Disable accelerated LAN to WAN path through PPA. \n
               Valid values are
                PPA_ENABLED \n
                PPA_DISABLED \n
    \param[out] wan_rx_ppa_enable Enable / Disable accelerated WAN to LAN path through PPA. \n
               Valid values are
              -  PPA_ENABLED  \n
              -  PPA_DISABLED \n
    \param[in] flag Reserved currently.

    \return The return value can be any one of the following:  \n
            - PPA_SUCCESS \n
            - PPA_FAILURE
    \note   This parameter is not available on older PPA version.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_status_fn)(uint32_t *lan_rx_ppa_enable, uint32_t *wan_rx_ppa_enable, uint32_t flag);
#else
extern int32_t ppa_hook_get_status_fn(uint32_t *lan_rx_ppa_enable, uint32_t *wan_rx_ppa_enable, uint32_t flag);
#endif

/*! 
    \brief Add a PPA routing session entry
    \param[in] skb Pointer to the packet buffer for which PPA session addition is to be done.                           
    \param[in] p_session Points to the connection tracking session to which this packet belongs. It maybe passed as NULL in which case PPA will try to determine it using the PPA stack adaptation layer.
    \param[in] flags  Flags as valid for the PPA session Valid \n
                       values are one or more of: \n 
                        - PPA_F_SESSION_BIDIRECTIONAL \n
                        - PPA_F_BRIDGED_SESSION \n
                        - PPA_F_STATIC_ENTRY \n
                        - PPA_F_DROP_PACKET \n
                        - PPA_F_SESSION_ORG_DIR \n
                        - PPA_F_SESSION_REPLY_DIR \n
                        - PPA_F_BEFORE_NAT_TRANSFORM \n
    \return The return value can be any one of the following:  \n
             - PPA_SESSION_NOT_ADDED \n
             - PPA_SESSION_ADDED \n
             - PPA_SESSION_EXISTS \n

    \note   Linux 2.4 Hook-up recommendation \n
             Must be hooked into the stack before the PREROUTING and after the POSTROUTING hooks In ip_conntrack_in() function in file ip_conntrack_core.c, with the flag PPA_F_BEFORE_NAT_TRANSFORM specified. \n
          1) In ip_finish_output2(), the hook should be invoked after NAT transforms are applied and at the very beginning of function call. \n 
           Linux 2.6 Hook-up recommendation \n
            Must be in netfilter IPV4 or IPV6 hook PREROUTRING and POSTROUTING hooks. In nf_conntrack_in for function which is common for both IPV4 and V6 connection tracking, PPA_F_BEFORE_NAT_TRANSFORM. In ip_finish_output2(), the hook should be invoked after NAT transforms are applied at netfilter POSTROUTING hooks and at the very beginning of function call.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_session_add_fn)(PPA_BUF *skb, PPA_SESSION *p_session, uint32_t flags);
#else
extern int32_t ppa_hook_session_add_fn(PPA_BUF *skb, PPA_SESSION *p_session, uint32_t flags);
#endif

extern int32_t (*ppa_hook_session_bradd_fn)(PPA_BUF *skb, PPA_SESSION *p_session, uint32_t flags);

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
#ifdef NO_DOXY
extern int32_t (*ppa_hook_session_ipsec_add_fn)(PPA_XFRM_STATE *, sa_direction );
extern int32_t (*ppa_hook_session_ipsec_del_fn)(PPA_XFRM_STATE *);
#else
extern int32_t ppa_hook_session_ipsec_add_fn(PPA_XFRM_STATE *, sa_direction );
extern int32_t ppa_hook_session_ipsec_del_fn(PPA_XFRM_STATE *);
#endif
#endif


/*! 
    \brief Del a PPA routing session entry
    \param[in] p_session Points to the connection tracking session to which this packet belongs. It maybe passed as NULL in which case PPA will try to determine it using the PPA stack adaptation layer.    
    \param[in] flags Reserved currently.
    \return The return value can be any one of the following:  \n
            - PPA_SUCCESS \n
            - PPA_FAILURE
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_session_del_fn)(PPA_SESSION *p_session, uint32_t flags);
#else
extern int32_t ppa_hook_session_del_fn(PPA_SESSION *p_session, uint32_t flags);
#endif

/*!
    \brief It is used to get session priority  of a ppa session 
    \param[in] p_session Points to the connection tracking session to which this packet belongs. It maybe passed as NULL in which case PPA will try to determine it using the PPA stack adaptation layer.    
    \param[in] flags Reserved currently.
    \return The return value is PPA session priority 
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_session_prio_fn)(PPA_SESSION *p_session, uint32_t flags);
#else
extern int32_t ppa_hook_session_prio_fn(PPA_SESSION *p_session, uint32_t flags);
#endif

/*! 
    \brief Modify an existing PPA session to allow handling for additional features like VLAN, DSCP and others.
    \param[in] p_session   PPA session id 
    \param[in] p_extra Extra parameters of PPA session
    \param[in] flags  Flags passed to modify session API to indicate which processing parameters are specified. Valid values are one or more of:
              - PPA_F_SESSION_NEW_DSCP \n
              - PPA_F_SESSION_VLAN \n
              - PPA_F_MTU \n

    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE \n
    \note Linux 2.4 Hook-up recommendation \n
           - Called in the stack on a per session basis to set additional handling like VLAN, DSCP and MTU parameters. \n 
           - One place to hook the call is after the function ppa_hook_session_add_fn() returns PPA_SESSION_ADDED. \n 
                    - Needs to evaluate the settings for VLAN, DSCP, MTU etc which are required to be used for this session.  These steps need to be written for the stack and supported functionality. \n 
           - Another place for hookup is in the Configuration routines of the individual features \n
                    - For example, for a VLAN configuration, go through the list of conntrack sessions, and call ppa_hook_session_modify_fn() for each session that is impacted by the change in configuration. \n
                    - If this step is not supported, then only sessions created after the configuration change will be affected.  
   \note Linux 2.6 Hook-up recommendation \n
           Same as 2.4.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_session_modify_fn)(PPA_SESSION *p_session, PPA_SESSION_EXTRA *p_extra, uint32_t flags);
#else
extern int32_t ppa_hook_session_modify_fn(PPA_SESSION *p_session, PPA_SESSION_EXTRA *p_extra, uint32_t flags);
#endif

/*! 
    \brief Returns all the configured unicast PPA sessions
    \param[out]  pp_sessions   Allocates and returns a pointer to an array  of session data structures for all unicast  sessions in PPA. \n Caller needs to free the  memory after use.
    \param[out] p_extra Allocates and returns a pointer to an array of session attributes structures for all unicast sessions in PPA.\n  Caller needs to free the memory after use.
    \param[out] p_num_entries   Returns the number of session entries  filled in the pp_sessions and pp_extra  arrays.
    \param[in] flags Reserved. 
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE 
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_session_get_fn)(PPA_SESSION ***pp_sessions, PPA_SESSION_EXTRA **p_extra, int32_t *p_num_entries, uint32_t flags);
#else
extern int32_t ppa_hook_session_get_fn(PPA_SESSION ***pp_sessions, PPA_SESSION_EXTRA **p_extra, int32_t *p_num_entries, uint32_t flags);
#endif

/*! 
    \brief Add, Modify and Delete PPA multicast group information like membership of interfaces to a multicast group address.
    \param[in] ppa_mc_entry Pointer to multicast group entry.                                                           I
    \param[in] flags Flags for the multicast group update operation. Valid values are:\n
               PPA_F_DROP_PACKET: Drop packets with this multicast group address as destination.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE \n

    \note add or delete one or more interface(s) to a multicast group entry, the function ppa_hook_mc_group_update_fn() needs to be called, the PPA_MC_GROUP structure modified appropriately and passed to this function. \n
            -> Linux 2.4 Hook-up recommendation \n
            The function needs to be hooked up in the Linux kernel functions ipmr_mfc_add() and ipmr_mfc_del() in the IP stack multicast routing path. \n
             For bridging path, hookup is required in the bridge forwarding path if IGMP snooping is implemented. It is also possible to make configuration entries from management interface path.               
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_mc_group_update_fn)(PPA_MC_GROUP *ppa_mc_entry, uint32_t flags);
#else
extern int32_t ppa_hook_mc_group_update_fn(PPA_MC_GROUP *ppa_mc_entry, uint32_t flags);
#endif

/*! 
    \brief This function gets the multicast group entry for the specified multicast group address.
    \param[in]  ip_mc_group IP multicast group address for which Multicast group information has to be returned.
    \param[in]  src_ip source IP address 
    \param[out] ppa_mc_entry  Pointer to multicast group entry. Valid memory space to be passed by caller.
    \param[in] flags Reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_mc_group_get_fn)(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, PPA_MC_GROUP *ppa_mc_entry, uint32_t flags);
#else
extern int32_t ppa_hook_mc_group_get_fn(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, PPA_MC_GROUP *ppa_mc_entry, uint32_t flags);
#endif

/*! 
    \brief This function modifies an existing multicast group entry for additional functionality like VLAN support
    \param[in] ip_mc_group    Pointer to IP multicast group address for which additional functionality needs to be configured.
    \param[in] src_ip              Source IP address 
    \param[in] ppa_mc_entry Pointer to PPA MC group entry.
    \param[in] p_extra Pointer to additional MC entry functional configuration. Currently, only VLAN configuration is supported
    \param[in] flags Reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS if entry exists for multicast group address \n
               - PPA_FAILURE otherwise \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_mc_entry_modify_fn)(IP_ADDR_C ip_mc_group,IP_ADDR_C src_ip, PPA_MC_GROUP *ppa_mc_entry, PPA_SESSION_EXTRA *p_extra, uint32_t flags);
#else
extern int32_t ppa_hook_mc_entry_modify_fn(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, PPA_MC_GROUP *ppa_mc_entry, PPA_SESSION_EXTRA *p_extra, uint32_t flags);
#endif

/*! 
    \brief This function returns an existing multicast group entry with its additional functionality configuration like VLAN support.
    \param[in] ip_mc_group    Pointer to IP multicast group address for which additional functionality needs to be configured.
    \param[in] src_ip              Source IP address 
    \param[in] p_extra Pointer to additional MC entry functional configuration. Currently, only VLAN configuration is supported
    \param[in] flags Reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS if entry exists for multicast group address \n
               - PPA_FAILURE otherwise \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_mc_entry_get_fn)(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip, PPA_SESSION_EXTRA *p_extra, uint32_t flags);
#else
extern int32_t ppa_hook_mc_entry_get_fn(IP_ADDR_C ip_mc_group, IP_ADDR_C src_ip,PPA_SESSION_EXTRA *p_extra, uint32_t flags);
#endif

/*! 
    \brief This function is  used to get a multicast source interface
    \param[in] buf A skb buffer
    \param[in] netif The interface pointer which received the ip packet. If NULL, it can be got from PPA_BUF *.
       
    \return The return value can be any one of the following:  \n
               - PPA_SESSION_ADDED if  if entry exists for multicast group address \n
               - PPA_SESSION_EXISTS if already added
               - PPA_SESSION_NOT_ADDED otherwise \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_multicast_pkt_srcif_add_fn)(PPA_BUF *buf, PPA_NETIF *netif);
#else
extern int32_t ppa_hook_multicast_pkt_srcif_add_fn(PPA_BUF *buf, PPA_NETIF *netif);
#endif

/*! 
    \brief Checks if the "accelerated" PPA session should be timed out due to inactivity.
    \param[in]  p_session Pointer to PPA unicast or multicast session.
    \return The return value can be any one of the following:  \n
              - PPA_TIMEOUT if the PPA session inactivity timer has expired \n
              - PPA_HIT if the PPA session has been active
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_inactivity_status_fn)(PPA_U_SESSION *p_session);
#else
extern int32_t ppa_hook_inactivity_status_fn(PPA_U_SESSION *p_session);
#endif

/*! 
    \brief Update the session inactivity timeout for a PPA session as per the session inactivity configuration in the protocol stack.
    \param[in] p_session Pointer to PPA unicast or multicast session.
    \param[in] timeout  Timeout value for session inactivity in  seconds.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_inactivity_fn)(PPA_U_SESSION *p_session, int32_t timeout);
#else
extern int32_t ppa_hook_set_inactivity_fn(PPA_U_SESSION *p_session, int32_t timeout);
#endif

/*! 
    \brief Add or update a MAC entry and its source ethernet port information in the PPA bridge table.
    \param[in]  mac_addr Pointer to MAC address to add to PPA bridge table.
    \param[in]  brif pointer to PPA net interface sttructure of the bridge.
    \param[in]  netif   Pointer to PPA net interface which is the source of the MAC address.
    \param[in]  flags Valid values are: 
               - PPA_F_BRIDGE_LOCAL - traffic is destined for local termination.
               - PPA_F_STATIC_ENTRY - static MAC address entry in the PPA bridge table. It will not be aged out.
               - PPA_F_DROP_PACKET - firewall action. Always drop packets with this MAC destination.   
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE \n
    \note Static MAC entry updates and MAC address drop filters can be configured from userspace.  For dynamic entries, the function must be hooked from bridging code where new entries are inserted into bridge mac table (or forwarding database, fdb).
             Linux 2.4 Hook-up recommendation \n
             Hook in kernel function br_fdb_insert() in net/bridge/br_fdb.c.  For Linux bridging code, the netif is given by fdb->dst->dev field where fdb points to a MAC entry.               
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_bridge_entry_add_fn)(uint8_t *mac_addr, PPA_NETIF *brif, PPA_NETIF *netif, uint32_t flags);
#else
extern int32_t ppa_hook_bridge_entry_add_fn(uint8_t *mac_addr, PPA_NETIF *brif, PPA_NETIF *netif, uint32_t flags);
#endif

/*! 
    \brief Delete a MAC entry from PPA Bridge table since the MAC entry is aged out or administratively triggered.
    \param[in]  mac_addr Pointer to MAC address to delete from PPA bridge table.
    \param[in]  brif pointer to PPA net interface sttructure of the bridge.
    \param[in]  flags Reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE 
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_bridge_entry_delete_fn)(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t flags);
#else
extern int32_t ppa_hook_bridge_entry_delete_fn(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t flags);
#endif

/*! 
    \brief Get latest packet arriving time for the specified MAC address entry. This is used for aging out decisions for the MAC entry.
    \param[in]  mac_addr Pointer to MAC address whose entry hit time is being queried
    \param[in]  brif pointer to PPA net interface sttructure of the bridge.
    \param[out] p_hit_time Provides the latest packet arriving time in seconds from system bootup.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS \n
               - PPA_FAILURE 
    \note Linux 2.4 Hook-up recommendation \n
            This API can be hooked in function br_fdb_cleanup(). In Linux, there is a kernel thread (br_fdb_cleanup) polling each entry in the MAC table and removes entries without traffic for a long time.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_bridge_entry_hit_time_fn)(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t *p_hit_time);
#else
extern int32_t ppa_hook_bridge_entry_hit_time_fn(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t *p_hit_time);
#endif

/*! 
    \brief Check if a PPA Bridge entry should be aged out due to inactivity as per the aging time configured by function \ref ppa_hook_set_bridge_entry_timeout_fn.
    \param[in]  mac_addr Pointer to MAC address whose entry hit time is being queried 
    \param[in]  brif pointer to PPA net interface sttructure of the bridge.
    \return The return value can be any one of the following:  \n
               - PPA_TIMEOUT if entry should be aged out \n
               - PPA_HIT if entry should not be aged out 
    \note Linux 2.4 Hook-up recommendation \n
               This API can be hooked in function has_expired() in br_fdb.c. \n
               Note that the function pair of ppa_hook_bridge_entry_inactivity_status_fn  and ppa_hook_set_bridge_entry_timeout_fn is an alternate aging mechanism to the use of function ppa_hook_bridge_entry_hit_time_fn.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_bridge_entry_inactivity_status_fn)(uint8_t *mac_addr, PPA_NETIF *brif);
#else
extern int32_t ppa_hook_bridge_entry_inactivity_status_fn(uint8_t *mac_addr, PPA_NETIF *brif);
#endif

/*! 
    \brief Set the PPA bridge entry inactivity timeout in seconds.
    \param[in]  mac_addr Pointer to MAC address whose entry hit time is being set 
    \param[in]  brif pointer to PPA net interface sttructure of the bridge.
    \param[in] timeout Timeout in seconds for inactivity timeout.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
    \note This function should be called immediately after ppa_hook_bridge_entry_add_fn() and whenever timeout needs to be changed by bridge stack. \n
              Linux 2.4 Hook-up recommendation \n For Linux, the timeout is equal to the bridge aging time and can be called from the function br_fdb_insert().
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_bridge_entry_timeout_fn)(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t timeout);
#else
extern int32_t ppa_hook_set_bridge_entry_timeout_fn(uint8_t *mac_addr, PPA_NETIF *brif, uint32_t timeout);
#endif

/*! \brief This fucntion will enable bridging hook ?? 
  \param[in] f_enable 0--disable, 1--enable
  \param[in] flags for future usage.
  \return The following value: \n
  - PPA_SUCCESS if sucessfully \n
  - PPA_FAILURE  otherwise. 
  \note It it used to enable/disable PPA bridging mac address learning. Normally it is called by network stack which want to enable/disable mac address learning.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_bridge_enable_fn)(uint32_t f_enable,  uint32_t flags);
#else
extern int32_t ppa_hook_bridge_enable_fn(uint32_t f_enable,  uint32_t flags);
#endif

/*! 
    \brief This function configures PPA Bridge Interface VLAN configuration behaviour. This includes functionality like whether the bridge is VLAN aware.
    \param[in] netif Pointer to network interface structure.
    \param[in] vlan_tag_control Pointer to VLAN Tagging control structure. This specifies whether VLAN tag, untag, replace should be carried out for the interface.
    \param[in] vlan_cfg  Pointer to network interface structure. 
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_bridge_if_vlan_config_fn)(PPA_NETIF *netif, PPA_VLAN_TAG_CTRL *vlan_tag_control, PPA_VLAN_CFG *vlan_cfg, uint32_t flags);
#else
extern int32_t ppa_hook_set_bridge_if_vlan_config_fn(PPA_NETIF *netif, PPA_VLAN_TAG_CTRL *vlan_tag_control, PPA_VLAN_CFG *vlan_cfg, uint32_t flags);
#endif

/*! 
    \brief This function gets the PPA Bridge Interface VLAN configuration. This includes functionality like whether the bridge is VLAN aware.
    \param[in] netif Pointer to network interface structure
    \param[out] vlan_tag_control Pointer to VLAN Tagging control structure. This specifies whether VLAN tag, untag, replace should be carried out for the interface.
    \param[out] vlan_cfg  Pointer to network interface structure. 
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_bridge_if_vlan_config_fn)(PPA_NETIF *netif, PPA_VLAN_TAG_CTRL *vlan_tag_control, PPA_VLAN_CFG *vlan_cfg, uint32_t flags);
#else
extern int32_t ppa_hook_get_bridge_if_vlan_config_fn(PPA_NETIF *netif, PPA_VLAN_TAG_CTRL *vlan_tag_control, PPA_VLAN_CFG *vlan_cfg, uint32_t flags);
#endif

/*! 
    \brief This function configures filters for VLAN tag/untag/retag actions in the PPA Bridge functionality.
    \param[in] vlan_match_field  Pointer to VLAN match filter that specifies the match criteria.
    \param[in] vlan_info Pointer to VLAN Info structure that specifies what VLAN tag action needs to be taken for traffic matching the filter
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_vlan_filter_add_fn)(PPA_VLAN_MATCH_FIELD *vlan_match_field, PPA_VLAN_INFO *vlan_info, uint32_t flags);
#else
extern int32_t ppa_hook_vlan_filter_add_fn(PPA_VLAN_MATCH_FIELD *vlan_match_field, PPA_VLAN_INFO *vlan_info, uint32_t flags);
#endif

/*! 
    \brief This function removes filters for VLAN tag/untag/retag actions in the PPA Bridge functionality.
    \param[in] vlan_match_field  Pointer to VLAN match filter that specifies the match criteria.
    \param[in] vlan_info Pointer to VLAN Info structure that specifies what VLAN tag action needs to be taken for traffic matching the filter
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_vlan_filter_del_fn)(PPA_VLAN_MATCH_FIELD *vlan_match_field, PPA_VLAN_INFO *vlan_info, uint32_t flags);
#else
extern int32_t ppa_hook_vlan_filter_del_fn(PPA_VLAN_MATCH_FIELD *vlan_match_field, PPA_VLAN_INFO *vlan_info, uint32_t flags);
#endif

/*! 
    \brief This function returns all configured filters for VLAN tag/untag/retag actions in the PPA Bridge functionality.
    \param[out] num_filters  Pointer to number of VLAN filters returned by the PPA.
    \param[out] vlan_filters Pointer to allocated array of VLAN filters.Caller needs to free the allocated memory
    \param[in]  flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
    \note This function is not implemented currently.               
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_vlan_filter_get_all_fn)(int32_t *num_filters, PPA_VLAN_FILTER_CONFIG *vlan_filters, uint32_t flags);
#else
extern int32_t ppa_hook_vlan_filter_get_all_fn(int32_t *num_filters, PPA_VLAN_FILTER_CONFIG *vlan_filters, uint32_t flags);
#endif

/*! 
    \brief This function removes all filters for VLAN tag/untag/retag actions in the PPA Bridge functionality.
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_vlan_filter_del_all_fn)(uint32_t flags);
#else
extern int32_t ppa_hook_vlan_filter_del_all_fn(uint32_t flags);
#endif

/*! 
    \brief Returns interface statistics from the PPA which is a subset of IfTable in SNMP MIB-II standard.
    \param[in] ifname Pointer to the interface name
    \param[out] p_stats  Pointer to the Statistics data structure of the interface.
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
    \note  This function is provided to allow the fastpath packet and byte counters to be accounted in stack interface statistics. This function is only implemented for D4 firmware.
           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_if_stats_fn)(PPA_IFNAME *ifname, PPA_IF_STATS *p_stats, uint32_t flags);
#else
extern int32_t ppa_hook_get_if_stats_fn(PPA_IFNAME *ifname, PPA_IF_STATS *p_stats, uint32_t flags);
#endif


/*! 
    \brief Returns per interface statistics kept by the PPA which is a superset of the per Interface statistics above. This provides, for example, fastpath routed and bridged statistics.
    \param[in] ifname Pointer to the interface name
    \param[out] p_stats  Pointer to the Statistics data structure of the interface.
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
    \note  This function is only implemented for D4 firmware.
           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_accel_stats_fn)(PPA_IFNAME *ifname, PPA_ACCEL_STATS *p_stats, uint32_t flags);
#else
extern int32_t ppa_hook_get_accel_stats_fn(PPA_IFNAME *ifname, PPA_ACCEL_STATS *p_stats, uint32_t flags);
#endif

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
/*! 
    \brief Returns per interface statistics kept by the PPA which is a superset of the per Interface statistics above. This provides, for example, fastpath routed and bridged statistics.
    \param[in] ifname Pointer to the interface name
    \param[out] p_stats  Pointer to the Statistics data structure of the interface.
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error
    \note  This function is only implemented for D4 firmware.
           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_netif_accel_stats_fn)(PPA_IFNAME *ifname, PPA_NETIF_ACCEL_STATS *p_stats, uint32_t flags);
#else
extern int32_t ppa_hook_get_netif_accel_stats_fn(PPA_IFNAME *ifname, PPA_NETIF_ACCEL_STATS *p_stats, uint32_t flags);
#endif
#endif

/*! 
    \brief Configures MAC address of the Interface (if it is an Ethernet-like interface).
    \param[in] ifname Pointer to the interface name
    \param[in] mac_addr  Pointer to the MAC address of the interface which is to be set.
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_if_mac_address_fn)(PPA_IFNAME *ifname, uint8_t *mac_addr, uint32_t flags);
#else
extern int32_t ppa_hook_set_if_mac_address_fn(PPA_IFNAME *ifname, uint8_t *mac_addr, uint32_t flags);
#endif

/*! 
    \brief Returns MAC address of the Interface (if it is an Ethernet-like interface).
    \param[in] ifname Pointer to the interface name
    \param[out] mac_addr  Pointer to the MAC address of the interface.
    \param[in] flags Reserved.
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_if_mac_address_fn)(PPA_IFNAME *ifname, uint8_t *mac_addr, uint32_t flags);
#else
extern int32_t ppa_hook_get_if_mac_address_fn(PPA_IFNAME *ifname, uint8_t *mac_addr, uint32_t flags);
#endif

/*! 
    \brief Adds a new interface to the PPA interface list. The new interface maybe a LAN interface or a WAN interface
    \param[in] ifinfo Pointer to the interface info structure
    \param[in] flags  Flag indicating whether interface is LAN flags or WAN. Valid value is: \n
                      PPA_F_LAN_IF if LAN interface

    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_add_if_fn)(PPA_IFINFO *ifinfo, uint32_t flags);
#else
extern int32_t ppa_hook_add_if_fn(PPA_IFINFO *ifinfo, uint32_t flags);
#endif

/*! 
    \brief Deletes an interface from the PPA interface list. The new interface maybe a LAN interface or a WAN interface
    \param[in] ifinfo Pointer to the interface info structure
    \param[in] flags  Flag indicating whether interface is LAN flags or WAN. Valid value is: \n
                      PPA_F_LAN_IF if LAN interface

    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_del_if_fn)(PPA_IFINFO *ifinfo, uint32_t flags);
#else
extern int32_t ppa_hook_del_if_fn(PPA_IFINFO *ifinfo, uint32_t flags);
#endif

/*! 
    \brief Gets the list of LAN or WAN interfaces from the PPA interface list
    \param[in] num_ifs Pointer to number of interface elements returned by the API
    \param[out] ifinfo   ointer to the allocated array of interface info structures. Caller has to free the pointer returned
    \param[in] flags  Flag indicating whether interface is LAN flags or WAN. Valid value is: \n
                      PPA_F_LAN_IF if LAN interface
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_if_fn)(int32_t *num_ifs, PPA_IFINFO **ifinfo, uint32_t flags);
#else
extern int32_t ppa_hook_get_if_fn(int32_t *num_ifs, PPA_IFINFO **ifinfo, uint32_t flags);
#endif

#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT

/*! 
    \brief Check if network interafce like ACA is a fastpath interface
    \param[in] netif Pointer to the network interface structure in the protocol stack. For eg. pointer to a struct net_device
    \param[in] ifname Interface name
    \param[in] flags  Reserved for future use

    \return The return value can be any one of the following:  \n
               - 1 if ACA or WLAN fastpath interface \n
               - 0 otherwise
*/
#ifdef NO_DOXY
extern int32_t (*ppa_check_if_netif_fastpath_fn)(PPA_NETIF *netif, char *ifname, uint32_t flags);
#else
extern int32_t ppa_check_if_netif_fastpath_fn(PPA_NETIF *netif, char *ifname, uint32_t flags);
#endif

/*! 
    \brief Deletes an WAVE500 STA session information from the PPA session list.
    \param[in] dev Pointer to the network device structure in the protocol stack. For eg. pointer to a struct netdevice
    \param[in] subif Pointer to the sub-interface structure in the datapath library.
    \param[in] mac station mac address
    \param[in] flags  Reserved for future use

    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_disconn_if_fn)(PPA_NETIF *dev, PPA_DP_SUBIF *subif, uint8_t *mac, uint32_t flags);
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
extern int32_t (*ppa_register_qos_class2prio_hook_fn)( int32_t port_id, PPA_NETIF *netif, PPA_QOS_CLASS2PRIO_CB qos_class2prio_cb, uint32_t flags);
#endif // WMM
#else
extern int32_t ppa_hook_disconn_if_fn(PPA_NETIF *dev, PPA_DP_SUBIF *subif, uint8_t *mac, uint32_t flags);
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
extern int32_t ppa_register_qos_class2prio_hook_fn( int32_t port_id, PPA_NETIF *netif, PPA_QOS_CLASS2PRIO_CB qos_class2prio_cb, uint32_t flags);
#endif // WMM
#endif
#endif

/*! 
    \brief This function allows a device driver to register or deregister a network device to the PPA
    \param[out] if_id  PPA specific Interface Identifier. It is currently a number between 0 to 7. This Id is returned by the PPA module
    \param[in] dev Pointer to the network device structure in  the protocol stack. For eg. pointer to a struct netdevice
    \param[in] pDirectpathCb   Pointer to the DirectPath callback structure which provides the driver callbacks for rx_fn, stop_tx_fn and restart_tx_fn.

    \param[in] flags Flag to indicate if device is being registered or deregisterd. Valid values are: \n
               PPA_F_DIRECTPATH_DEREGISTER, if de-registering, zero otherwise
               PPA_F_DIRECTPATH_CORE1 if the driver for the network interface is running on Core 1 (i.e. 2nd CPU)
               PPA_F_DIRECTPATH_ETH_IF if the network interface is an Ethernet-like interface
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_register_dev_fn)(uint32_t *if_id, PPA_NETIF *dev, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_register_dev_fn(uint32_t *if_id, PPA_NETIF *dev, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);
#endif

/*! 
    \brief This function allows a device driver to register or deregister a network device to the PPA
    \param[out] subif  PPA specific Interface Identifier. It is currently a number between 0 to 16. This Id is returned by the PPA module
    \param[in] dev Pointer to the network device structure in  the protocol stack. For eg. pointer to a struct netdevice
    \param[in] pDirectpathCb   Pointer to the DirectPath callback structure which provides the driver callbacks for rx_fn, stop_tx_fn and restart_tx_fn.

    \param[in] flags Flag to indicate if device is being registered or deregisterd. Valid values are: \n
               PPA_F_DIRECTPATH_DEREGISTER, if de-registering, zero otherwise
               PPA_F_DIRECTPATH_CORE1 if the driver for the network interface is running on Core 1 (i.e. 2nd CPU)
               PPA_F_DIRECTPATH_ETH_IF if the network interface is an Ethernet-like interface
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_ex_register_dev_fn)(PPA_SUBIF *subif, PPA_NETIF *dev, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_ex_register_dev_fn(PPA_SUBIF *subif, PPA_NETIF *dev, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);
#endif



/*! 
    \brief This function allows a device driver to register or deregister a network device to the PPA directlink 
    \param[out] if_id  PPA virtual Interface Identifier. It is currently a number between 0 to 15. This Id is returned by the PPA module
    \param[in] dtlk Pointer to the directlink structure. inside the structure, it has some elements:\n
                    dev: pointer to network device 
                    vap_id:  VAP index from wifi driver 
                    flags:  flag to indicate the actions  
    \param[in] pDirectpathCb   Pointer to the DirectPath callback structure which provides the driver callbacks for rx_fn, stop_tx_fn and restart_tx_fn.
    \param[in] flags Flag to indicate if device is being registered or deregisterd. Valid values are: \n
               PPA_F_DIRECTPATH_DEREGISTER, if de-registering, zero otherwise
               PPA_F_DIRECTPATH_CORE1 if the driver for the network interface is running on Core 1 (i.e. 2nd CPU)
               PPA_F_DIRECTPATH_ETH_IF if the network interface is an Ethernet-like interface
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
*/

#ifdef NO_DOXY
extern int32_t (*ppa_hook_directlink_register_dev_fn)(int32_t *if_id, PPA_DTLK_T *dtlk, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);

#else
extern int32_t ppa_hook_directlink_register_dev_fn(int32_t *if_id, PPA_DTLK_T *dtlk, PPA_DIRECTPATH_CB *pDirectpathCb, uint32_t flags);
#endif

/*! 
    \brief This function allows the device driver to transmit a packet using the PPA DirectPath interface. The packet buffer 
              passed to the function must have its packet data pointer set to the IP header with the Ethernet
              header/Link layer header etc preceding the IP header (For eg., on Linux skb->data points to IP header, and
              doing the appropriate skb_push() will allow skb->data to move backwards and point to Ethernet header).
              In other words, PPA Directpath must be able to access the full frame even though the packet buffer points to the
              IP header as required by the Linux netif_rx() function.
    \param[in] rx_if_id  Receive interface Id in the protocol stack
    \param[in] buf     Pointer to the packet buffer structure of the stack for the packet which is to be transmitted
    \param[in] len     Size of packet in bytes
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
    \note The DirectPath Tx API can have internal shortcut path to the destination or fallback to passing the packet to the
              protocol stack. The aim is to insulate the device driver calling the API from such details. \n
              For Linux, the driver must call this function through the hook pointer where it passes packets to the network stack by calling the netif_rx() or netif_receive_skb() functions. \n
              Note : The CPU-bound device driver is strongly recommended to call this API from tasklet mode (or equivalent non-interrupt context on non-Linux OS) and not from IRQ context for better system dynamics.           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_send_fn)(uint32_t rx_if_id, PPA_BUF *buf, int32_t len, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_send_fn(uint32_t rx_if_id, PPA_BUF *buf, int32_t len, uint32_t flags);
#endif

/*! 
    \brief This function allows the device driver to transmit a packet using the PPA DirectPath interface. The packet buffer 
              passed to the function must have its packet data pointer set to the IP header with the Ethernet
              header/Link layer header etc preceding the IP header (For eg., on Linux skb->data points to IP header, and
              doing the appropriate skb_push() will allow skb->data to move backwards and point to Ethernet header).
              In other words, PPA Directpath must be able to access the full frame even though the packet buffer points to the
              IP header as required by the Linux netif_rx() function.
    \param[in] subif  Receive interface Id and sub interface id 
    \param[in] buf     Pointer to the packet buffer structure of the stack for the packet which is to be transmitted
    \param[in] len     Size of packet in bytes
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess \n
               - PPA_FAILURE on error           
    \note The DirectPath Tx API can have internal shortcut path to the destination or fallback to passing the packet to the
              protocol stack. The aim is to insulate the device driver calling the API from such details. \n
              For Linux, the driver must call this function through the hook pointer where it passes packets to the network stack by calling the netif_rx() or netif_receive_skb() functions. \n
              Note : The CPU-bound device driver is strongly recommended to call this API from tasklet mode (or equivalent non-interrupt context on non-Linux OS) and not from IRQ context for better system dynamics.           
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_ex_send_fn)(PPA_SUBIF *subif, PPA_BUF *buf, int32_t len, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_ex_send_fn(PPA_SUBIF *subif, PPA_BUF *buf, int32_t len, uint32_t flags);
#endif



/*! 
    \brief This function allows the device driver to indicate to the PPA that it cannot receive any further packets from the
               latter. The device driver can call this function for flow control.
    \param[in] if_id  interface Id for which to apply flow control
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess\n
               - PPA_FAILURE on fail\n 
               - PPA_EINVAL if the interface not exist 
    \note It is recommended for a device driver to use the PPA DirectPath flow control functions for efficient packet processing.        
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_rx_stop_fn)(uint32_t if_id, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_rx_stop_fn(uint32_t if_id, uint32_t flags);
#endif


/*! 
    \brief This function allows the device driver to indicate to the PPA that it cannot receive any further packets from the
               latter. The device driver can call this function for flow control.
    \param[in] subif  Receive interface Id and sub interface id 
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess\n
               - PPA_FAILURE on fail\n 
               - PPA_EINVAL if the interface not exist 
    \note It is recommended for a device driver to use the PPA DirectPath flow control functions for efficient packet processing.        
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_ex_rx_stop_fn)(PPA_SUBIF *subif, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_ex_rx_stop_fn(PPA_SUBIF *subif, uint32_t flags);
#endif

/*! 
    \brief This function allows the device driver to indicate to the PPA that it can again receive packets from the latter. The
              device driver can call this function for flow control after it has called the Rx Stop function to halt supply of packets
              from the PPA.
    \param[in] if_id  interface Id for which the driver requests the flow control action to restart transmission.
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess\n
               - PPA_FAILURE on fail\n 
               - PPA_EINVAL if the interface not exist 
    \note   It is recommended for a device driver to use the PPA DirectPath flow control functions for efficient packet processing. This function must be used in conjunction with the PPA_FP_STOP_TX_FN.

*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_rx_restart_fn)(uint32_t if_id, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_rx_restart_fn(uint32_t if_id, uint32_t flags);
#endif


/*! 
    \brief This function allows the device driver to indicate to the PPA that it can again receive packets from the latter. The
              device driver can call this function for flow control after it has called the Rx Stop function to halt supply of packets
              from the PPA.
    \param[in] subif  Receive interface Id and sub interface id 
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess\n
               - PPA_FAILURE on fail\n 
               - PPA_EINVAL if the interface not exist 
    \note   It is recommended for a device driver to use the PPA DirectPath flow control functions for efficient packet processing. This function must be used in conjunction with the PPA_FP_STOP_TX_FN.

*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_ex_rx_restart_fn)(PPA_SUBIF *subif, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_ex_rx_restart_fn(PPA_SUBIF *subif, uint32_t flags);
#endif

/*! 
    \brief This function allows the device driver to allocate the skb buffer.
    \param[in] subif  Receive interface Id and sub interface id 
    \param[in] len    Length of the skb buffer
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - Not-null PPA_BUF pointer on sucess\n
               - NULL or (void *)PPA_FAILURE or (void *)PPA_EINVAL on fail\n 
    \note      It is recommended for a device driver to recycle this buffer using ppa_hook_directpath_recycle_skb_fn function.

*/
#ifdef NO_DOXY
extern PPA_BUF * (*ppa_hook_directpath_alloc_skb_fn)(PPA_SUBIF* psubif, int32_t len, uint32_t flags);
#else
extern PPA_BUF * ppa_hook_directpath_alloc_skb_fn(PPA_SUBIF* psubif, int32_t len, uint32_t flags);
#endif

/*! 
    \brief This function allows the device driver to de-allocate the skb buffer.
    \param[in] subif  Receive interface Id and sub interface id 
    \param[in] skb    Allocated skb pointer
    \param[in] flags  reserved
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on sucess\n
               - PPA_FAILURE on fail\n 
               - PPA_EINVAL if the interface not exist 

*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_recycle_skb_fn)(PPA_SUBIF* psubif, PPA_BUF* skb, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_recycle_skb_fn(PPA_SUBIF* psubif, PPA_BUF* skb, uint32_t flags);
#endif

/*! 
    \brief This function maps the PPA Interface Id to Protocol stack interface structure.
    \param[in] if_id  PPA Interface Identifier, if_id
    \return The return value can be any one of the following: \n
                 - Pointer to the interface structure in the protocol stack \n
                 - NULL
*/
#ifdef NO_DOXY
extern PPA_NETIF *(*ppa_hook_get_netif_for_ppa_ifid_fn)(uint32_t if_id);
#else
extern PPA_NETIF *ppa_hook_get_netif_for_ppa_ifid_fn(uint32_t if_id);
#endif

/*! 
    \brief This function maps the Protocol stack interface structure to the PPA Interface Id.
    \param[in] netif Pointer to the protocol stack network interface structure for the device
    \return The return value can be any one of the following: \n
			- PPA Interface Identifier, if_id \n
			- PPA_FAILURE
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_ifid_for_netif_fn)(PPA_NETIF *netif);
#else
extern int32_t ppa_hook_get_ifid_for_netif_fn(PPA_NETIF *netif);
#endif

/*! 
    \brief This function add a vlan range for wan interface in mixed mode
    \param[in] vlan_range Pointer to vlan id range
    \param[in] flags  reserved
    \return The return value can be any one of the following: \n
               - PPA_SUCCESS on success\n
               - PPA_FAILURE on fail\n 
*/
#ifdef NO_DOXY
extern  int32_t (*ppa_hook_wan_mii0_vlan_range_add_fn)(PPA_VLAN_RANGE *vlan_range, uint32_t flags) ;
#else
extern  int32_t ppa_hook_wan_mii0_vlan_range_add_fn(PPA_VLAN_RANGE *vlan_range, uint32_t flags) ;
#endif

/*! 
    \brief This function remove a vlan range from wan interface in mixed mode/RAN
    
    \param[in] vlan_range Pointer to vlan id range
    \param[in] flags  reserved
    \return The return value can be any one of the following: \n
               - PPA_SUCCESS on success\n
               - PPA_FAILURE on fail\n 
*/
#ifdef NO_DOXY
extern  int32_t (*ppa_hook_wan_mii0_vlan_range_del_fn)(PPA_VLAN_RANGE *vlan_range, int32_t flags);
#else
extern  int32_t ppa_hook_wan_mii0_vlan_range_del_fn(PPA_VLAN_RANGE *vlan_range, int32_t flags);
#endif

/*! 
    \brief This function get a vlan range list from wan interface in mixed mode
    \param[out] vlan The vlan id range number
    \param[out] vlan_range Pointer to vlan id range list
    \param[in]  flags  reserved
    \return The return value can be any one of the following: \n
               - PPA_SUCCESS on success\n
               - PPA_FAILURE on fail\n 
*/
#ifdef NO_DOXY
extern  int32_t (*ppa_hook_wan_mii0_vlan_ranges_get_fn)(int32_t *vlan, PPA_VLAN_RANGE *vlan_range, uint32_t flags);
#else
extern  int32_t ppa_hook_wan_mii0_vlan_ranges_get_fn(int32_t *vlan, PPA_VLAN_RANGE *vlan_range, uint32_t flags);
#endif

#ifdef CONFIG_LTQ_PPA_QOS
/*! 
    \brief This is to get the maximum queue number supported on specified port
    \param[in] portid the physical port id which support qos queue
    \param[in] flag reserved for future
    \return returns the queue number supported on this port.  
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_qos_qnum)( uint32_t portid, uint32_t flag);
#else
extern int32_t ppa_hook_get_qos_qnum( uint32_t portid, uint32_t flag);
#endif

/*! 
    \brief This is to get the mib counter on specified port and queueid
    \param[in] portid the physical port id which support qos queue
    \param[in] queueid the queue id for the mib 
    \param[out] mib the buffer to store qos mib
    \param[in] flag reserved for future
    \return returns the queue number supported on this port.  
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_qos_mib)( uint32_t portid, uint32_t queueid, PPA_QOS_MIB *mib, uint32_t flag);
#else
extern int32_t ppa_hook_get_qos_mib( uint32_t portid, uint32_t queueid, PPA_QOS_MIB *mib, uint32_t flag);
#endif


#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
/*!  
    \brief This is to eanble/disable Rate Shaping feature
    \param[in] portid the phisical port id which support qos queue
    \param[in] enable 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_ctrl_qos_rate)( uint32_t portid, uint32_t enable, uint32_t flag);
#else
extern int32_t ppa_hook_set_ctrl_qos_rate( uint32_t portid, uint32_t enable, uint32_t flag);
#endif

/*!  
    \brief This is to get Rate Shaping feature status: enabled or disabled
    \param[in] portid the phisical port id which support qos queue
    \param[out] enable buffer to store status: 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_ctrl_qos_rate)( uint32_t portid, uint32_t *enable, uint32_t flag);
#else
extern int32_t ppa_hook_get_ctrl_qos_rate( uint32_t portid, uint32_t *enable, uint32_t flag);
#endif

/*!  
    \brief This is to set Rate Shaping for one specified port and queue
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set rate shaping
    \param[in] rate the maximum rate limit in kbps
    \param[in] burst the maximun burst in bytes
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_qos_rate)( uint32_t portid, uint32_t queueid, uint32_t rate, uint32_t burst, uint32_t flag);
#else
extern int32_t ppa_hook_set_qos_rate( uint32_t portid, uint32_t queueid, uint32_t rate, uint32_t burst, uint32_t flag);
#endif

/*! 
    \brief This is to get Rate Shaping settings for one specified port and queue 
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set rate shaping
    \param[out] rate the maximum rate limit in kbps
    \param[out] burst the maximun burst in bytes
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n    
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_qos_rate)( uint32_t portid, uint32_t queueid, uint32_t *rate, uint32_t *burst, uint32_t flag); 
#else
extern int32_t ppa_hook_get_qos_rate( uint32_t portid, uint32_t queueid, uint32_t *rate, uint32_t *burst, uint32_t flag);
#endif

/*! 
    \brief This is to reset Rate Shaping for one specified port and queue (
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set rate shaping
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n    
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_reset_qos_rate)( uint32_t portid, uint32_t queueid, uint32_t flag );  
#else
extern int32_t ppa_hook_reset_qos_rate( uint32_t portid, uint32_t queueid, uint32_t flag );  
#endif

#endif /*end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING*/


#ifdef CONFIG_LTQ_PPA_QOS_WFQ
/*!  
    \brief This is to eanble/disable WFQ feature
    \param[in] portid the phisical port id which support qos queue
    \param[in] enable 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_ctrl_qos_wfq)( uint32_t portid, uint32_t enable, uint32_t flag);
#else
extern int32_t ppa_hook_set_ctrl_qos_wfq( uint32_t portid, uint32_t enable, uint32_t flag);
#endif

/*!  
    \brief This is to get WFQ feature status: enabled or disabled
    \param[in] portid the phisical port id which support qos queue
    \param[out] enable 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_ctrl_qos_wfq)( uint32_t portid, uint32_t *enable, uint32_t flag);
#else
extern int32_t ppa_hook_get_ctrl_qos_wfq( uint32_t portid, uint32_t *enable, uint32_t flag);
#endif

/*!  
    \brief This is to set WFQ weight level for one specified port and queue
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set WFQ
    \param[out] weight_level the value should be 0 ~ 100. 
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_qos_wfq)( uint32_t portid, uint32_t queueid, uint32_t weight_level, uint32_t flag);  
#else
extern int32_t ppa_hook_set_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t weight_level, uint32_t flag);  
#endif

/*! 
    \brief This is to get WFQ settings for one specified port and queue ( default value should be 0xFFFF)
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set WFQ
    \param[out] weight_level the value should be 0 ~ 100. 
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n    
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_qos_wfq)( uint32_t portid, uint32_t queueid, uint32_t *weight_level, uint32_t flag);  
#else
extern int32_t ppa_hook_get_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t *weight_level, uint32_t flag);  
#endif

/*! 
    \brief This is to reset WFQ for one specified port and queue ( default value should be 0xFFFF)
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set WFQ
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n    
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_reset_qos_wfq)( uint32_t portid, uint32_t queueid, uint32_t flag );  
#else
extern int32_t ppa_hook_reset_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t flag );  
#endif

#endif /*end of CONFIG_LTQ_PPA_QOS_WFQ*/
#endif /*end of CONFIG_LTQ_PPA_QOS*/

#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
/*! 
    \brief This is to enable/disable multiple field function
    \param[in] enable,  1--enable/0--disable multiple field
    \return uint8_t, The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_multifield_control)(uint8_t enable, uint32_t flag);
#else
extern int32_t ppa_hook_multifield_control(uint8_t enable, uint32_t flag);
#endif //end of NO_DOXY

/*!
    \brief This is to get multiple field status: enable or disable
    \param[out] enable, buffer for store the multiple filed feature stauts: 1 enabled, 0 disabled.
    \param[in] flag, reserved for future
    \return uint8_t The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_multifield_status)(uint8_t *enable, uint32_t flag);
#else
extern int32_t ppa_hook_get_multifield_status(uint8_t *enable, uint32_t flag);
#endif //end of NO_DOXY

/*!
    \brief This is to get the maximum multiple field entry/flow number
    \return int32_t, return the maximum multiple field entry number. For PPA 2.3 it is 32 once multiple filed feature is enabled.
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_multifield_max_flow)(uint32_t flag);
#else
extern int32_t ppa_hook_get_multifield_max_flow(uint32_t flag);
#endif //end of NO_DOXY

/*!
    \brief This is to add one multiple field flow
    \param[in] p_multifield_info, the pointer which store the classficication set. 
    \param[out]: index return the flow index if successfully added into. \n
    \param[in] flag, reserved for future
    \return int32_t, The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error \n
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_add_multifield_flow)( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, int32_t *index, uint32_t flag);
#else
extern int32_t ppa_hook_add_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, int32_t *index, uint32_t flag);
#endif //end of NO_DOXY

/*!
    \brief This is to get one multiple field flow as specified via index
    \param[in] p_multifield_info, the pointer which stores the classficication set configuration    
    \param[in] flag, reserved for future
    \return int32_t, return the bytes in the compare table. It can be any one of the following now: \n              
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_multifield_flow)( int32_t index, PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag );
#else
extern int32_t ppa_hook_get_multifield_flow( int32_t index, PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag );
#endif
/*!
    \brief This is to delete multiple field entry if compare/mask/key completely match
    \param[in] p_multifield_info, the pointer to store the classficication set configuration    
    \param[in] flag, reserved for future
    \return int32_t, The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error, like entry full already
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_del_multifield_flow)( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag );
#else
extern int32_t ppa_hook_del_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag );
#endif //end of NO_DOXY

/*!
    \brief This is to delete multiple field entry as specified via index
    \param[in] index, the index of compare table to delete
    \return int32_t, return the bytes in the compare table. It can be any one of the following now:  \n
              \return int32_t, The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error
    \note if index is _1, it will delete all multiple field entries
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_quick_del_multifield_flow)( int32_t index, uint32_t flag);
#else
extern int32_t ppa_hook_quick_del_multifield_flow( int32_t index, uint32_t flag);
#endif //end of NO_DOXY
#endif //end of CONFIG_LTQ_PPA_API_MFE


/*!
    \brief This is to get 6rd tunnel's destination mac address
    \param[in] netif network device pointer
    \param[out] mac buffer to store ethernet mac address
    \param[in]  daddr 
    \return int32_t, The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error
*/
#ifdef NO_DOXY
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,10)
extern int32_t (*ppa_get_6rd_dmac_fn)(PPA_NETIF *netif, uint8_t *mac,uint32_t daddr);
#else
extern int32_t (*ppa_get_6rd_dmac_fn)(PPA_NETIF *netif,PPA_BUF *ppa_buf, uint8_t *mac,uint32_t daddr);
#endif
#else
extern int32_t ppa_get_6rd_dmac_fn(PPA_NETIF *netif, uint8_t *mac,uint32_t daddr);
#endif

/*!
    \brief This is to get ipv6 tunnel's destination mac address
    \param[in] netif network device pointer
    \param[out] mac buffer to store ethernet mac address
    \return int32_t, The return value can be any one of the following:  \n
               PPA_SUCCESS on sucess \n
               PPA_FAILURE on error
*/
#ifdef NO_DOXY
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,10)
extern int32_t (*ppa_get_ip4ip6_dmac_fn)(PPA_NETIF *netif, uint8_t *mac);
#else
extern int32_t (*ppa_get_ip4ip6_dmac_fn)(PPA_NETIF *netif,PPA_BUF* buf, uint8_t *mac);
#endif
#else
extern int32_t ppa_get_ip4ip6_dmac_fn(PPA_NETIF *netif, uint8_t *mac);
#endif


/*!
    \brief This is to get 6rd tunnel's underlayer device
    \param[in] netif network device pointer
    \return int32_t, The return value can be any one of the following:  \n
               pointer to the 6rd's underlayer device \n
               NULL on error
*/
#ifdef NO_DOXY
extern PPA_NETIF* (*ppa_get_6rd_phyif_fn)(PPA_NETIF *netif);
#else
extern PPA_NETIF* ppa_get_6rd_phyif_fn(PPA_NETIF *netif);

#endif

/*!
    \brief This is to get ipv6  tunnel's underlayer device
    \param[in] netif network device pointer
    \return int32_t, The return value can be any one of the following:  \n
               pointer to the ipv6 tunnel's underlayer device \n
               NULL on error
*/
#ifdef NO_DOXY
extern PPA_NETIF* (*ppa_get_ip4ip6_phyif_fn)(PPA_NETIF *netif);
#else
extern PPA_NETIF* ppa_get_ip4ip6_phyif_fn(PPA_NETIF *netif);
#endif

//#ifdef CONFIG_PPA_GRE
extern uint32_t (*ppa_is_ipv4_gretap_fn)(struct net_device *dev);
extern PPA_NETIF* (*ppa_ipv4_gre_phy_fn)(struct net_device *dev);
extern uint32_t (*ppa_is_ipv6_gretap_fn)(struct net_device *dev);
extern PPA_NETIF* (*ppa_ipv6_gre_phy_fn)(struct net_device *dev);
//#endif

/*!
    \brief This is to enqueue skb to linux imq for rateshaping
    \param[in] skb skb buffer
    \param[in] portID directpath port id
    \return int32_t, The return value can be 0 or 1
*/               
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_enqueue_to_imq_fn)(PPA_BUF *skb, uint32_t portID);
#else
extern int32_t ppa_hook_directpath_enqueue_to_imq_fn(PPA_BUF *skb, uint32_t portID);
#endif

/*!
    \brief This is to reinject skb from linux imq for rateshaping
    \param[in] rx_if_id Recive interface id  
    \param[in] buf skb buffer
    \param[in] len 
    \param[in] flags 
    \return int32_t, The return value can be 0 or 1
*/  
#ifdef NO_DOXY
extern int32_t (*ppa_hook_directpath_reinject_from_imq_fn)(int32_t rx_if_id, PPA_BUF *buf, int32_t len, uint32_t flags);
#else
extern int32_t ppa_hook_directpath_reinject_from_imq_fn(int32_t rx_if_id, PPA_BUF *buf, int32_t len, uint32_t flags);
#endif


/*!
    \brief This is to set the lan separation flag 
    \param[in] flag Lan separation flag 
    \return int32_t, The return value can be 0 or 1
*/  
#ifdef NO_DOXY
extern int32_t(*ppa_hook_set_lan_seperate_flag_fn)( uint32_t flag );
#else
extern int32_t ppa_hook_set_lan_seperate_flag_fn( uint32_t flag);
#endif

 /*!
    \brief This is to set the wan separation flag 
    \param[in] flag Wan separation flag 
    \return int32_t, The return value can be 0 or 1
*/  
#ifdef NO_DOXY
extern int32_t(*ppa_hook_set_wan_seperate_flag_fn)( uint32_t flag );
#else
extern int32_t ppa_hook_set_wan_seperate_flag_fn ( uint32_t flag);
#endif

 /*!
    \brief This is to get the lan separation flag 
    \param[in] flag Lan separation flag 
    \return int32_t, The return value can be 0 or 1
*/  
#ifdef NO_DOXY
extern uint32_t (*ppa_hook_get_lan_seperate_flag_fn)(uint32_t flag);
#else
extern uint32_t ppa_hook_get_lan_seperate_flag_fn(uint32_t flag);
#endif

 /*!
    \brief This is to get the wan separation flag 
    \param[in] flag Wan separation flag 
    \return int32_t, The return value can be 0 or 1
*/  
#ifdef NO_DOXY
extern uint32_t (*ppa_hook_get_wan_seperate_flag_fn)(uint32_t flag);
#else
extern uint32_t ppa_hook_get_wan_seperate_flag_fn(uint32_t flag);
#endif

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
/*!
    \brief This is to send skb through software fastpath
    \param[in] skb skb buffer
    \return int32_t, The return value can be 0 or 1
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_sw_fastpath_send_fn)( struct sk_buff *skb );

#else
extern int32_t ppa_hook_sw_fastpath_send_fn(struct sk_buff *skb);
#endif

/*!
    \brief This is to enable software fastpath
    \param[in] flags for software fp enable disable
    \return int32_t, The return value can be 0 or 1
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_set_sw_fastpath_enable_fn)( uint32_t flags );
#else
extern int32_t ppa_hook_set_sw_fastpath_enable_fn(uint32_t* flags);
#endif

/*!
    \brief This is to get software fastpath status
    \param[in] flags for software fp enable disable
    \return int32_t, The return value can be 0 or 1
*/
#ifdef NO_DOXY
extern int32_t (*ppa_hook_get_sw_fastpath_status_fn)( uint32_t flags );
#else
extern int32_t ppa_hook_get_sw_fastpath_status_fn(uint32_t* flags);
#endif

#endif

#endif //end of __KERNEL__


/* @} */
#endif  //  __PPA_HOOK_H__20081103_1736__
