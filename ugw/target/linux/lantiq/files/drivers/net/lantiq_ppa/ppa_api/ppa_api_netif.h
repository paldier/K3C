#ifndef __PPA_API_NETIF_H__20081104_1138__
#define __PPA_API_NETIF_H__20081104_1138__



/*******************************************************************************
**
** FILE NAME    : ppa_api_netif.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 4 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Network Interface Functions Header
**                File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 04 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/
/*! \file ppa_api_netif.h
    \brief This file contains network related api
*/

/** \defgroup PPA_NETIF_API PPA Network Interface API
    \brief  provide network interface related api to get/add/delete/update information
            - ppa_api_netif.h: Header file for PPA API
            - ppa_api_netif.c: C Implementation file for PPA API
*/
/* @{ */ 

/*
 * ####################################
 *              Definition
 * ####################################
 */

#define PPA_IS_PORT_CPU0_AVAILABLE()        (g_phys_port_cpu == ~0 ? 0 : 1)
#define PPA_IS_PORT_ATM_AVAILABLE()         (g_phys_port_atm_wan == ~0 ? 0 : 1)

#define PPA_PORT_CPU0                       g_phys_port_cpu
#define PPA_PORT_ATM                        g_phys_port_atm_wan

#define PPA_DEST_LIST_CPU0                  (1 << g_phys_port_cpu)
#define PPA_DEST_LIST_ATM                   (1 << g_phys_port_atm_wan)

#define PPA_PORT_ATM_VLAN_FLAGS             g_phys_port_atm_wan_vlan

/*
 *  net interface type
 */
#define NETIF_VLAN                              0x00000001
#define NETIF_BRIDGE                            0x00000002
#define NETIF_PHY_ETH                           0x00000010
#define NETIF_PHY_ATM                           0x00000020
#define NETIF_PHY_TUNNEL                        0x00000040
#define NETIF_BR2684                            0x00000100
#define NETIF_EOA                               0x00000200
#define NETIF_IPOA                              0x00000400
#define NETIF_PPPOATM                           0x00000800
#define NETIF_PPPOE                             0x00001000
#define NETIF_VLAN_INNER                        0x00002000
#define NETIF_VLAN_OUTER                        0x00004000
#define NETIF_VLAN_CANT_SUPPORT                 0x00008000
#define NETIF_LAN_IF                            0x00010000
#define NETIF_WAN_IF                            0x00020000
#define NETIF_PHY_IF_GOT                        0x00040000
#define NETIF_PHYS_PORT_GOT                     0x00080000
#define NETIF_MAC_AVAILABLE                     0x00100000
#define NETIF_MAC_ENTRY_CREATED                 0x00200000
#define NETIF_PPPOL2TP                          0x00400000
#define NETIF_DIRECTPATH			0x00800000
#define NETIF_GRE_TUNNEL                        0x01000000
#define NETIF_DIRECTCONNECT                     0x02000000

/*
 * ####################################
 *              Data Type
 * ####################################
 */

struct phys_port_info {
    struct phys_port_info      *next;
    unsigned int                mode    :2; //  0: CPU, 1: LAN, 2: WAN, 3: MIX (LAN/WAN)
    unsigned int                type    :2; //  0: CPU, 1: ATM, 2: ETH, 3: EXT
    unsigned int                vlan    :2; //  0: no VLAN, 1: inner VLAN, 2: outer VLAN
    unsigned int                port    :26;
    PPA_IFNAME                  ifname[PPA_IF_NAME_SIZE];
};

struct flag_wanitf{
    uint32_t                     flag_root_itf:1; //flag whether it is a root interface, 1 means it is a physical root interface
    uint32_t                     flag_force_wanitf:1; //flag: whether force wanitf set or not. 1 means the force flag is set by ppacmd/hook. 
    uint32_t                     flag_already_wanitf:1; //flag whether wanitf value is already set or not.
    uint32_t                     old_lan_flag:1;  // old lan flag value: 1 means it is LAN port, 0 means WAN port
} ;

typedef enum {
  FLOWID_IPV4_EoGRE = 0,
  FLOWID_IPV6_EoGRE,
  FLOWID_IPV4GRE,
  FLOWID_IPV6GRE,
  FLOWID_CAPWAP
}e_flowId;

typedef struct {
  uint32_t        pceRuleIndex;
  e_flowId        flowId;   /* Tunnel flow id - for egress PMAC handling  */ 
  uint32_t        extId;  /* Extension id - defined in PCE rule */
  uint16_t        tnl_hdrlen;
} gre_tunnel_info;
/*
typedef enum {
  FLOWID_LOGINTF_1 = 0,
  FLOWID_LOGINTF_2,
  FLOWID_LOGINTF_3,
  FLOWID_LOGINTF_4
}qos_flowId;
*/
struct netif_info {
    struct netif_info          *next;
    PPA_ATOMIC                  count;

    PPA_IFNAME                  name[PPA_IF_NAME_SIZE];
    PPA_IFNAME                  manual_lower_ifname[PPA_IF_NAME_SIZE]; /*only for simple network interface, like macvlan, wlan multiple SSIS, not for tunnel/pppoe/... */
    PPA_IFNAME                  phys_netif_name[PPA_IF_NAME_SIZE];
    PPA_NETIF                  *netif;
    PPA_VCC                    *vcc;
    uint8_t                     mac[PPA_ETH_ALEN];
    uint32_t                    flags;  //  NETIF_VLAN, NETIF_BRIDGE, NETIF_PHY_ETH, NETIF_PHY_ATM,
                                        //  NETIF_BR2684, NETIF_EOA, NETIF_IPOA, NETIF_PPPOATM,
                                        //  NETIF_PPPOE, NETIF_PHY_TUNNEL
                                        //  NETIF_VLAN_INNER, NETIF_VLAN_OUTER, NETIF_VLAN_CANT_SUPPORT,
                                        //  NETIF_LAN_IF, NETIF_WAN_IF,
                                        //  NETIF_PHY_IF_GOT, NETIF_PHYS_PORT_GOT,
                                        //  NETIF_MAC_AVAILABLE, NETIF_MAC_ENTRY_CREATED
    uint32_t                    vlan_layer;
    uint32_t                    inner_vid;
    uint32_t                    outer_vid;
    uint32_t                    pppoe_session_id;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    uint32_t                    pppol2tp_session_id;
    uint32_t                    pppol2tp_tunnel_id;
#endif
    gre_tunnel_info             greInfo;
    uint32_t                    dslwan_qid;
    uint32_t                    phys_port;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint16_t 			fid;
    uint16_t			fid_index;
    uint16_t			subif_id;
    uint8_t			tc;
    uint8_t			flowId;
    uint8_t			flowId_en;
#endif
    uint32_t                    mac_entry;
    PPA_NETIF                   *out_vlan_netif;
    PPA_NETIF                   *in_vlan_netif;
    struct flag_wanitf        f_wanitf;
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB	
    uint8_t                     sub_if_index;
    PPA_IFNAME                  sub_if_name[PPA_IF_SUB_NAME_MAX_NUM][PPA_IF_NAME_SIZE];
#if 0
    uint64_t                    acc_tx;
    uint64_t                    acc_rx;    
#else /* #if 0 */
    PPA_HW_ACCEL_STATS          hw_accel_stats;
    PPA_SW_ACCEL_STATS          sw_accel_stats;
#endif /* #else */
    uint64_t                    prev_clear_acc_tx;
    uint64_t                    prev_clear_acc_rx; 
#endif	
};



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  variable
 */
extern uint32_t g_phys_port_cpu;
extern uint32_t g_phys_port_atm_wan;
extern uint32_t g_phys_port_atm_wan_vlan;

/*
 *  physical network interface
 */
int32_t ppa_phys_port_add(PPA_IFNAME *, uint32_t);
void ppa_phys_port_remove(uint32_t);
int32_t ppa_phys_port_get_first_eth_lan_port(uint32_t *, PPA_IFNAME **);

int32_t ppa_phys_port_start_iteration(uint32_t *, struct phys_port_info **);
int32_t ppa_phys_port_iterate_next(uint32_t *, struct phys_port_info **);
void ppa_phys_port_stop_iteration(void);

/*
 *  network interface
 */
int32_t ppa_netif_add(PPA_IFNAME *, int, struct netif_info **, PPA_IFNAME *, int );
void ppa_netif_remove(PPA_IFNAME *, int);
int32_t ppa_netif_lookup(PPA_IFNAME *, struct netif_info **);
int32_t __ppa_netif_lookup(PPA_IFNAME *, struct netif_info **);
void ppa_netif_put(struct netif_info *);
int32_t ppa_netif_update(PPA_NETIF *, PPA_IFNAME *);

int32_t ppa_netif_start_iteration(uint32_t *, struct netif_info **);
int32_t ppa_netif_iterate_next(uint32_t *, struct netif_info **);
void ppa_netif_stop_iteration(void);
void ppa_netif_lock_list(void);
void ppa_netif_unlock_list(void);



/*
 *  inline functions
 */

/*
 *  Init/Uninit Functions
 */
int32_t ppa_api_netif_manager_init(void);
void ppa_api_netif_manager_exit(void);
int32_t ppa_api_netif_manager_create(void);
void ppa_api_netif_manager_destroy(void);



/* @} */
#endif  //  __PPA_API_NETIF_H__20081104_1138__
