/*******************************************************************************
**
** FILE NAME    : ppa_api_pp_core.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 
** AUTHOR       : Punith Kumar
** DESCRIPTION  : PPA acceleration handling for PUMA7/GRX750
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 29 Set 2016  Punith Kumar           Initiate Version
*******************************************************************************/


/*******************
 **** INCLUDES ****
 *******************/

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

/*
 *  PPA Specific Head File
 */
#include <linux/swap.h>
#include <linux/if_vlan.h>

#include <net/ppa_api.h>
#if defined(CONFIG_LTQ_DATAPATH) && CONFIG_LTQ_DATAPATH
#include <net/datapath_api.h>
#endif
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#include "ppa_api_qos.h"
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#include "ppa_api_session_limit.h"
#endif
#include "ppa_api_mib.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_hal_selector.h"
#include "ppa_api_tools.h"
#include "ppa_stack_al.h"
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif
#include "ppa_api_sess_helper.h"

#ifdef CONFIG_ARM_AVALANCHE_SOC
#include <asm-arm/arch-avalanche/generic/pal_cppi41.h>
#include <asm-arm/arch-avalanche/generic/pal.h>
#include <asm-arm/arch-avalanche/puma7/puma7_pp.h>
#include <asm-arm/arch-avalanche/generic/pp_qos_p7.h>
#include <mach/puma.h>
#include <mach/hardware.h>
#include "linux/cat_l2switch_netdev.h"
#else
#include <linux/avalanche/puma7/puma7.h>
#include <linux/avalanche/generic/pal_cppi41.h>
#include <linux/avalanche/generic/pal.h>
#include <linux/avalanche/generic/pp_qos_p7.h>
#include <linux/avalanche/puma7/puma7_pp.h>
#include <linux/avalanche/puma7/puma7_defs.h>
#include <linux/avalanche/generic/avalanche_pp_api.h>
#endif

#define ENABLE_LAN_PORT_SEPARATION

/*****************************
 **** EXTERN FUNCTIONS ****
 *****************************/


/************************************
 **** FUNCTIONS DECLARATIONS ***
 ************************************/

int32_t ppa_pp_ingress_handler(PPA_BUF *ppa_buf, struct session_list_item *p_item);
int32_t ppa_pp_session_create(PPA_BUF *ppa_buf, struct session_list_item *p_item);
int32_t	ppa_setup_ingress_property(AVALANCHE_PP_INGRESS_SESSION_PROPERTY_t *ingress_property, PPA_NETIF *netif, struct session_list_item *p_item);
int32_t	ppa_setup_egress_property(AVALANCHE_PP_EGRESS_SESSION_PROPERTY_t *egress_property, PPA_NETIF *netif, struct session_list_item *p_item);

AVALANCHE_PP_RET_e (*ppa_avalanche_pp_session_create)(AVALANCHE_PP_SESSION_INFO_t *ptr_session, void *pkt_ptr ) = NULL;
AVALANCHE_PP_RET_e  (*ppa_avalanche_pp_vpid_get_info)( Uint8 vpid_handle, AVALANCHE_PP_VPID_INFO_t **ptr_vpid) = NULL;
PP_QOS_MGR_RET_e (*ppa_pp_qos_is_vpid_registered)(Uint8 vpid_id, Bool *isRegistered) = NULL;
Bool (*ppa_avalanche_pp_state_is_active)( void ) = NULL;
AVALANCHE_PP_RET_e (*ppa_avalanche_pp_pid_get_info)(Uint8 pid_handle, AVALANCHE_PP_PID_t ** ptr_pid) = NULL;

void ppa_dump_iface_info(PPA_BUF *ppa_buf, struct session_list_item *p_item) 
{
	printk(KERN_INFO "## [%s] p_item %p\n", __FUNCTION__, p_item); 
	printk(KERN_INFO "\n[p_item->rx_if: %s]\n", ppa_get_netif_name(p_item->rx_if));
	printk(KERN_INFO "\n[p_item->tx_if: %s]\n", ppa_get_netif_name(p_item->tx_if));
	printk(KERN_INFO "\n[ppa_buf->dev: %s]\n", ppa_get_netif_name(ppa_buf->dev));
	if(p_item->br_tx_if)
		printk(KERN_INFO "\n[p_item->br_tx_if: %s]\n", ppa_get_netif_name(p_item->br_tx_if));
	if(p_item->br_rx_if)
		printk(KERN_INFO "\n[p_item->br_rx_if: %s]\n", ppa_get_netif_name(p_item->br_rx_if));

	return;
}

#if 0
void pp_dump_skb(uint32_t len, void* skb1)
{
	int i;
	struct sk_buff *skb = (struct sk_buff *) skb1;
	unsigned char *data = skb->data;

	printk("skb->len =%d skb->head = %x, skb->data = %x, skb->l2hdr = %x, skb->l3hdr = %x, skb->l4hdr= %x\n", 
			skb->len, skb->head, skb->data, skb_mac_header(skb), skb_network_header(skb), skb_transport_header(skb));

	for(i=0;i<len && i < skb->len;i++) {
		printk("%2.2X%c",data[i], ((i+1)%16)?' ':'\n' ) ;
	} 
	printk("\n");
}
#endif

int32_t	ppa_update_ingress_l4_lookup(__Avalanche_PP_LUTs_Data_t  *lookup, PPA_NETIF *netif, struct session_list_item *p_item)
{
	if ( p_item->flags & SESSION_IS_TCP) {
		lookup->L4_Protocol = PP_LOOKUP_FIELD_L4_PROTOCOL_TCP;
	} else {
		lookup->L4_Protocol = PP_LOOKUP_FIELD_L4_PROTOCOL_UDP;
	}

	lookup->L4_DST_PORT = p_item->dst_port;
	lookup->L4_SRC_PORT = p_item->src_port;
	return 0;
}

int32_t	ppa_update_ingress_l3_lookup(__Avalanche_PP_LUTs_Data_t  *lookup, PPA_NETIF *netif, struct session_list_item *p_item)
{
	if ( p_item->flags & SESSION_IS_IPV6 )
		lookup->L3_type = PP_LOOKUP_FIELD_L3_TYPE_IPv6;
	else 
		lookup->L3_type = PP_LOOKUP_FIELD_L3_TYPE_IPv4;

	lookup->ToS = p_item->ip_tos;

	lookup->DST_IP.v4 = p_item->dst_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
	lookup->DST_IP.v6[1] = p_item->dst_ip.ip6[1]; 
	lookup->DST_IP.v6[2] = p_item->dst_ip.ip6[2];
	lookup->DST_IP.v6[3] = p_item->dst_ip.ip6[3];
#endif

	lookup->SRC_IP.v4 = p_item->src_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
	lookup->SRC_IP.v6[1] = p_item->src_ip.ip6[1];
	lookup->SRC_IP.v6[2] = p_item->src_ip.ip6[2];
	lookup->SRC_IP.v6[3] = p_item->src_ip.ip6[3];
#endif
	return 0;
}

#define ETH_P_IPV6      0x86DD
#define ETH_P_IP        0x0800
#define ETH_P_PPP_SES   0x8864
#define ETH_P_8021Q     0x8100
#define IPV4_HDR_LEN      20  //assuming no option fields are present
#define IPV6_HDR_LEN      40

int32_t	ppa_setup_ingress_property(AVALANCHE_PP_INGRESS_SESSION_PROPERTY_t *ingress_property, PPA_NETIF *netif, struct session_list_item *p_item)
{
	__Avalanche_PP_LUTs_Data_t  *lookup;
	AVALANCHE_PP_PID_t  *pid;
	Uint8 *vpid_handle;
	uint8_t netif_mac[PPA_ETH_ALEN];
	AVALANCHE_PP_VPID_INFO_t *ptr_vpid;


	if (ingress_property == NULL || netif == NULL || p_item == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " ingress fail. Abort \n");
		return 0;
	}

	if (ppa_avalanche_pp_pid_get_info(netif->pid_handle, &pid) != PP_RC_SUCCESS) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " failed in ppa_avalanche_pp_pid_get_info(). Abort \n");
		return 0;
	}

	lookup = &ingress_property->lookup;
	vpid_handle = &ingress_property->vpid_handle;

	lookup->PID = pid->pid_handle;

	if (netif->vpid_handle != -1 && vpid_handle != NULL)
	{
		*vpid_handle = netif->vpid_handle;
	}

	ppa_get_netif_hwaddr(p_item->rx_if, netif_mac, 1);
	lookup->L2_type = PP_LOOKUP_FIELD_L2_TYPE_ETHERNET;
	memcpy(lookup->srcmac, p_item->src_mac, PPA_ETH_ALEN);
	memcpy(lookup->dstmac, netif_mac, PPA_ETH_ALEN);

	ppa_avalanche_pp_vpid_get_info(ingress_property->vpid_handle, &ptr_vpid);

	if (ptr_vpid->type == AVALANCHE_PP_VPID_VLAN) {
#ifdef ENABLE_LAN_PORT_SEPARATION
		if (memcmp(ptr_vpid->devName, "eth0", 4) == 0) {
			lookup->Vlan1 = cpu_to_be16((ptr_vpid->vlan_identifier & VLAN_VID_MASK) | 0x0800);
			lookup->Flags |= PP_LOOKUP_FIELD_FLAGS_VLAN1;
		} else
#endif
		{
			lookup->Vlan1 = cpu_to_be16(ptr_vpid->vlan_identifier);
			lookup->Flags |= PP_LOOKUP_FIELD_FLAGS_VLAN1;
		}
	}

	if (p_item->protocol == ETH_P_8021Q) {
		if (lookup->Flags & PP_LOOKUP_FIELD_FLAGS_VLAN1) {
			lookup->Vlan2 = cpu_to_be16(p_item->new_vci);
			lookup->Flags |= PP_LOOKUP_FIELD_FLAGS_VLAN2;
		} else {
			lookup->Vlan1 = cpu_to_be16(p_item->new_vci);
			lookup->Flags |= PP_LOOKUP_FIELD_FLAGS_VLAN1;
		}
	}


	lookup->PPPoE_session_id = cpu_to_be16(PP_LOOKUP_FIELD_PPPOE_SESSION_INVALID);

	if( (p_item->flags & SESSION_WAN_ENTRY) && (p_item->flags & SESSION_VALID_PPPOE) ) {
		lookup->Tunnel_type = PP_LOOKUP_FIELD_TUNNEL_TYPE_PPPoE;
		lookup->PPPoE_session_id = p_item->pppoe_session_id;
	}
	if( (p_item->flags & SESSION_WAN_ENTRY) && (p_item->flags & SESSION_TUNNEL_6RD) ) {
		lookup->Tunnel_type = PP_LOOKUP_FIELD_TUNNEL_TYPE_6rd;
	} else if( (p_item->flags & SESSION_WAN_ENTRY) && (p_item->flags & SESSION_TUNNEL_DSLITE) ) {
		lookup->Tunnel_type = PP_LOOKUP_FIELD_TUNNEL_TYPE_DsLITE;
	}

	ppa_update_ingress_l3_lookup(lookup, netif, p_item);
	ppa_update_ingress_l4_lookup(lookup, netif, p_item);

	return 0;
}

int32_t	ppa_update_egress_l4_lookup(__Avalanche_PP_LUTs_Data_t  *lookup, PPA_NETIF *netif, struct session_list_item *p_item)
{
	if ( p_item->flags & SESSION_IS_TCP) {
		lookup->L4_Protocol = PP_LOOKUP_FIELD_L4_PROTOCOL_TCP;
	} else {
		lookup->L4_Protocol = PP_LOOKUP_FIELD_L4_PROTOCOL_UDP;
	}

	if ( (p_item->flags & SESSION_VALID_NAT_PORT) ) {
		if ( (p_item->flags & SESSION_LAN_ENTRY) ) {
			lookup->L4_DST_PORT = p_item->dst_port;
			lookup->L4_SRC_PORT = p_item->nat_port;
		} else {
			lookup->L4_DST_PORT = p_item->nat_port;
			lookup->L4_SRC_PORT = p_item->src_port;
		}
	} else {
		lookup->L4_DST_PORT = p_item->dst_port;
		lookup->L4_SRC_PORT = p_item->src_port;
	}
	return 0;
}

int32_t	ppa_update_egress_l3_lookup(__Avalanche_PP_LUTs_Data_t  *lookup, PPA_NETIF *netif, struct session_list_item *p_item)
{
	if ( p_item->flags & SESSION_IS_IPV6 )
		lookup->L3_type = PP_LOOKUP_FIELD_L3_TYPE_IPv6;
	else 
		lookup->L3_type = PP_LOOKUP_FIELD_L3_TYPE_IPv4;

	lookup->ToS = p_item->ip_tos;

	if ( (p_item->flags & SESSION_VALID_NAT_IP) ) {
		if ( (p_item->flags & SESSION_LAN_ENTRY) ) {
			lookup->DST_IP.v4 = p_item->dst_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
			lookup->DST_IP.v6[1] = p_item->dst_ip.ip6[1];
			lookup->DST_IP.v6[2] = p_item->dst_ip.ip6[2];
			lookup->DST_IP.v6[3] = p_item->dst_ip.ip6[3];
#endif
			lookup->SRC_IP.v4 = p_item->nat_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
			lookup->SRC_IP.v6[1] = p_item->nat_ip.ip6[1];
			lookup->SRC_IP.v6[2] = p_item->nat_ip.ip6[2];
			lookup->SRC_IP.v6[3] = p_item->nat_ip.ip6[3];
#endif
		} else {
			lookup->DST_IP.v4 = p_item->nat_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
			lookup->DST_IP.v6[1] = p_item->nat_ip.ip6[1];
			lookup->DST_IP.v6[2] = p_item->nat_ip.ip6[2];
			lookup->DST_IP.v6[3] = p_item->nat_ip.ip6[3];
#endif

			lookup->SRC_IP.v4 = p_item->src_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
			lookup->SRC_IP.v6[1] = p_item->src_ip.ip6[1];
			lookup->SRC_IP.v6[2] = p_item->src_ip.ip6[2];
			lookup->SRC_IP.v6[3] = p_item->src_ip.ip6[3];
#endif
		}
	} else {
		lookup->DST_IP.v4 = p_item->dst_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
		lookup->DST_IP.v6[1] = p_item->dst_ip.ip6[1]; 
		lookup->DST_IP.v6[2] = p_item->dst_ip.ip6[2];
		lookup->DST_IP.v6[3] = p_item->dst_ip.ip6[3];
#endif

		lookup->SRC_IP.v4 = p_item->src_ip.ip;
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
		lookup->SRC_IP.v6[1] = p_item->src_ip.ip6[1];
		lookup->SRC_IP.v6[2] = p_item->src_ip.ip6[2];
		lookup->SRC_IP.v6[3] = p_item->src_ip.ip6[3];
#endif
	}
	return 0;
}

#define PPPOE_IPV4_TAG  0x0021
#define PPPOE_IPV6_TAG  0x0057

typedef enum {
	SW_ACC_TYPE_IPV4,
	SW_ACC_TYPE_IPV6,
	SW_ACC_TYPE_6RD,
	SW_ACC_TYPE_DSLITE,
	SW_ACC_TYPE_BRIDGED,
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
	SW_ACC_TYPE_LTCP,
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
	SW_ACC_TYPE_LTCP_LRO,
#endif
#endif
	SW_ACC_TYPE_MAX
}sw_acc_type;

typedef struct sw_header {

	uint16_t tot_hdr_len;		/* Total length of outgoing header (for future use) */
	uint16_t transport_offset;	/* Transport header offset - from beginning of MAC header  */ 
	uint16_t network_offset; 	/* Network header offset - from beginning of MAC header  */ 
	uint32_t extmark; 		/* marking */
	sw_acc_type type;  		/* Software acceleration type */
	PPA_NETIF *tx_if;   		/* Out interface */
	int (*tx_handler)(PPA_BUF *skb); /* tx handler function */
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
	struct dst_entry *dst; 	/* route entry */
#endif
	uint8_t hdr[0];		/* Header to be copied */
} t_sw_hdr;


int32_t	ppa_setup_egress_property(AVALANCHE_PP_EGRESS_SESSION_PROPERTY_t *egress_property, PPA_NETIF *netif, struct session_list_item *p_item)
{
	__Avalanche_PP_LUTs_Data_t  *lookup;
	AVALANCHE_PP_PID_t  *pid;
	AVALANCHE_PP_VPID_INFO_t *ptr_vpid = NULL;

	char hdr[PPPOE_SES_HLEN+1];
	t_sw_hdr  *swa;
	int i;

	Uint8 *vpid_handle;
	uint8_t netif_mac[PPA_ETH_ALEN];
	Uint32 vlanTag = 0;
	Bool vpid_vlan_present = False;

	if (egress_property == NULL || netif == NULL || p_item == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " egress fail. Abort \n");
		return 0;
	}

	if (ppa_avalanche_pp_pid_get_info(netif->pid_handle, &pid) != PP_RC_SUCCESS) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " failed in ppa_avalanche_pp_pid_get_info(). Abort \n");
		return 0;
	}

	lookup = &egress_property->lookup;
	vpid_handle = &egress_property->vpid_handle;

	lookup->PID = pid->pid_handle;

	if (netif->vpid_handle != -1 && vpid_handle != NULL)
	{
		*vpid_handle = netif->vpid_handle;
	}

	ppa_get_netif_hwaddr(netif, netif_mac, 1);

	lookup->L2_type = PP_LOOKUP_FIELD_L2_TYPE_ETHERNET;
	memcpy(lookup->srcmac, netif_mac, PPA_ETH_ALEN);
	memcpy(lookup->dstmac, p_item->dst_mac, PPA_ETH_ALEN);

	memcpy(egress_property->wrapHeader, p_item->dst_mac, ETH_ALEN);
	memcpy(egress_property->wrapHeader + ETH_ALEN, netif_mac, ETH_ALEN);
	egress_property->wrapHeaderLen = ETH_ALEN * 2;

	lookup->Vlan1 = cpu_to_be16(0);
	lookup->Vlan2 = cpu_to_be16(0);

	ppa_avalanche_pp_vpid_get_info(egress_property->vpid_handle, &ptr_vpid);

	if (ptr_vpid != NULL && ptr_vpid->type == AVALANCHE_PP_VPID_VLAN) {
		lookup->Vlan1 = cpu_to_be16(ptr_vpid->vlan_identifier & VLAN_VID_MASK);
		lookup->Vlan1 |= cpu_to_be16(VLAN_PRIO_MASK & (((Uint16)(0x00)) << VLAN_PRIO_SHIFT));
		vpid_vlan_present = True;

		vlanTag = cpu_to_be32((ETH_P_8021Q << 16) | be16_to_cpu(lookup->Vlan1));
		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)&vlanTag, VLAN_HLEN);
		egress_property->wrapHeaderLen += VLAN_HLEN;
	}

	if ( p_item->protocol == ETH_P_8021Q) {
		if (vpid_vlan_present) {
			lookup->Vlan2 = cpu_to_be16(p_item->new_vci);
			lookup->Vlan2 |= cpu_to_be16(VLAN_PRIO_MASK & (((Uint16)(0x00)) << VLAN_PRIO_SHIFT));
		} else {
			lookup->Vlan1 = cpu_to_be16(p_item->new_vci); 
			lookup->Vlan1 |= cpu_to_be16(VLAN_PRIO_MASK & (((Uint16)(0x00)) << VLAN_PRIO_SHIFT));
		}
		egress_property->enable |= AVALANCHE_PP_EGRESS_FIELD_ENABLE_VLAN;
	}

	if (egress_property->enable & AVALANCHE_PP_EGRESS_FIELD_ENABLE_VLAN) {
		if (vpid_vlan_present) {
			vlanTag = cpu_to_be32((ETH_P_8021Q << 16) | be16_to_cpu(lookup->Vlan2));
		} else {
			vlanTag = cpu_to_be32((ETH_P_8021Q << 16) | be16_to_cpu(lookup->Vlan2));
		}
		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)&vlanTag, VLAN_HLEN);
		egress_property->wrapHeaderLen += VLAN_HLEN;
	} 

	egress_property->eth_type = htons(p_item->protocol);
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "egress_property->eth_type = [%x]\n", egress_property->eth_type);

	lookup->PPPoE_session_id = cpu_to_be16(PP_LOOKUP_FIELD_PPPOE_SESSION_INVALID);


	if( (p_item->flags & SESSION_LAN_ENTRY) && (p_item->flags & SESSION_VALID_PPPOE) ) {
		lookup->Tunnel_type = PP_LOOKUP_FIELD_TUNNEL_TYPE_PPPoE;
		lookup->PPPoE_session_id = p_item->pppoe_session_id;
		p_item->protocol = htons(ETH_P_PPP_SES);
		egress_property->eth_type = ETH_P_PPP_SES;

		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)&p_item->protocol, sizeof(egress_property->eth_type));
		egress_property->wrapHeaderLen += sizeof(egress_property->eth_type);

		hdr[0]='\0';

		//88641100.00040596.0021
		//88640011.00E45B47.0050
		*((uint16_t*)(hdr)) = htons(0x1100);
		*((uint16_t*)(hdr+2)) = lookup->PPPoE_session_id;
		*((uint16_t*)(hdr+4)) = 0x0000;

		if( p_item->flags & SESSION_IS_IPV6)
			*((uint16_t*)(hdr+6)) = htons(PPPOE_IPV6_TAG);
		else
			*((uint16_t*)(hdr+6)) = htons(PPPOE_IPV4_TAG);

		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)hdr, PPPOE_SES_HLEN);
		egress_property->wrapHeaderLen += PPPOE_SES_HLEN;
	} else {
		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)&p_item->protocol, sizeof(egress_property->eth_type));
		egress_property->wrapHeaderLen += sizeof(egress_property->eth_type);
	}

	if ( p_item->sah != NULL ) {
		swa = (t_sw_hdr*)(p_item->sah);
		if ( swa != NULL) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "\nswa->hdr: ");
			for(i=0;i<swa->tot_hdr_len;i++) {
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "%2.2X%c",swa->hdr[i], ((i+1)%16)?' ':'\n' );
			}
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "swa->network_offset: %d#\n", swa->network_offset);
			memcpy(egress_property->wrapHeader, (void *)swa->hdr, (swa->network_offset));
			egress_property->wrapHeaderLen = egress_property->wrapHeaderOffLayer3 = (swa->network_offset);
		}
	}

	egress_property->wrapHeaderOffLayer3 = egress_property->wrapHeaderLen;

	if( (p_item->flags & SESSION_LAN_ENTRY) && (p_item->flags & SESSION_TUNNEL_6RD) ) {
		egress_property->eth_type = ETH_P_IP;
		lookup->Tunnel_type = PP_LOOKUP_FIELD_TUNNEL_TYPE_6rd;
		egress_property->enable |= AVALANCHE_PP_EGRESS_WH_IPv4;
		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)(swa->hdr+swa->network_offset), IPV4_HDR_LEN);
		egress_property->wrapHeaderLen += IPV4_HDR_LEN;

	} else if( (p_item->flags & SESSION_LAN_ENTRY) && (p_item->flags & SESSION_TUNNEL_DSLITE) ) {
		egress_property->eth_type = ETH_P_IPV6;
		lookup->Tunnel_type = PP_LOOKUP_FIELD_TUNNEL_TYPE_DsLITE;
		egress_property->enable |= AVALANCHE_PP_EGRESS_WH_IPv6;
		memcpy(egress_property->wrapHeader + egress_property->wrapHeaderLen, (void *)(swa->hdr+swa->network_offset), IPV6_HDR_LEN);
		egress_property->wrapHeaderLen += IPV6_HDR_LEN;

	}

	ppa_update_egress_l3_lookup(lookup, netif, p_item);
	ppa_update_egress_l4_lookup(lookup, netif, p_item);

	return 0;
}

int32_t ppa_pp_ingress_handler(PPA_BUF *ppa_buf, struct session_list_item *p_item)
{

	AVALANCHE_PP_SESSION_INFO_t* session_info = NULL;
	PPA_NETIF *rx_netif;
	PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE];
	AVALANCHE_PP_VPID_INFO_t *ptr_vpid = NULL;
	AVALANCHE_PP_INGRESS_SESSION_PROPERTY_t *ingress_property;
	Uint32 epiFlags        = 0;
	Uint32 sessionHandle   = 0;
	Uint32 vlanSize        = 0;

	if(ppa_avalanche_pp_state_is_active == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " pp core apis are not refrenced here. abort\n");
		return 0;
	}

	if (!ppa_avalanche_pp_state_is_active() || ppa_buf == NULL || p_item == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " ppa_avalanche_pp_state_is_active() = [%u] . abort \n", ppa_avalanche_pp_state_is_active());
		return 0;
	}

	if (p_item->br_rx_if)
		rx_netif = p_item->br_rx_if;
	else 
		rx_netif = p_item->rx_if;

	if( ppa_check_is_pppoe_netif(rx_netif) &&
			ppa_pppoe_get_physical_if(rx_netif, NULL, underlying_ifname) == PPA_SUCCESS ) {
		if ( (rx_netif = ppa_get_netif(underlying_ifname)) == NULL ) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_pppoe_get_physical_if failed. abort \n");
			return 0;
		}
	}
	if( rx_netif->type == ARPHRD_SIT){ //6RD device
		if(ppa_get_6rd_phyif_fn == NULL || (rx_netif = ppa_get_6rd_phyif_fn(rx_netif)) == NULL){
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"6RD, cannot get physical device\n");
			return PPA_FAILURE;
		}
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"6RD physical device name: %s \n", rx_netif->name);
	} else if(rx_netif->type == ARPHRD_TUNNEL6) {
#if defined(CONFIG_LTQ_PPA_DSLITE) && CONFIG_LTQ_PPA_DSLITE
		if(ppa_get_ip4ip6_phyif_fn == NULL || (rx_netif = ppa_get_ip4ip6_phyif_fn(rx_netif)) == NULL) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Add ip4ip6 device: %s, but cannot find the physical device\n", rx_netif->name);
			return PPA_FAILURE;
		}
#endif          
	}



	if (rx_netif == NULL || rx_netif->pid_handle == -1) {
		//		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "rx_netif->pid_handle is -1 for [%s] in Ingress hook. abort \n", 
		//				ppa_get_netif_name(rx_netif));
		return 0;
	}

	if (rx_netif->packet_handler == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " rx_netif->packet_handler is NULL. abort\n" );
		return 0;
	}

	session_info = &ppa_buf->pp_packet_info->pp_session;

	if (session_info == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " session_info inside skb is NULL. abort\n" );
		return 0;
	}

	ppa_setup_ingress_property(&session_info->ingress, rx_netif, p_item);
	ppa_buf->pp_packet_info->flags |= TI_HIL_PACKET_FLAG_PP_SESSION_INGRESS_RECORDED;

	ppa_buf->pp_packet_info->pp_session.phy_cluster_id = PP_QOS_INVALID_CLUSTER;
	ppa_buf->pp_packet_info->input_device_index = rx_netif->ifindex;

	/* Get flags from EPI1 */
	// If session is valid, it means it was not accelerated
	// We need to exclude this packet from being counted as 'Forwarded Packet'
	if (PAL_CPPI4_HOSTDESC_NETINFW0_SESSION_VALID & epiFlags)
	{    
		sessionHandle = SKB_GET_PP_INFO_P(ppa_buf)->pp_session.session_handle;
		ingress_property = &(SKB_GET_PP_INFO_P(ppa_buf)->pp_session.ingress);
		ppa_avalanche_pp_vpid_get_info(ingress_property->vpid_handle, &ptr_vpid);

		if (ptr_vpid->type == AVALANCHE_PP_VPID_VLAN)
		{
			vlanSize = VLAN_HLEN;
		}

		//	ppa_avalanche_pp_modify_stats_counters (sessionHandle, ppa_buf->len + ppa_buf->mac_len + vlanSize);
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "Added a non accelerated packet. session=%d. len=%d, dataLen=%d, macLen=%d, hdrLen=%d, vlanSize=%d\n", sessionHandle, ppa_buf->len, ppa_buf->data_len, ppa_buf->mac_len, ppa_buf->hdr_len, vlanSize);
	}
	return 0;
}

int32_t ppa_pp_session_create(PPA_BUF *ppa_buf, struct session_list_item *p_item)
{
	AVALANCHE_PP_SESSION_INFO_t* session_info = NULL;

	PPA_NETIF *tx_netif;
	PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE];
	Bool isVpidRegistered = False;
	AVALANCHE_PP_RET_e rc;

	if(ppa_avalanche_pp_state_is_active == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " pp core apis are not refrenced here. abort\n");
		return 0;
	}

	if (!ppa_avalanche_pp_state_is_active() || ppa_buf == NULL || p_item == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " ppa_avalanche_pp_state_is_active() = [%u] . abort \n", ppa_avalanche_pp_state_is_active());
		return 0;
	}

	if (p_item->br_tx_if)
		tx_netif = p_item->br_tx_if;
	else 
		tx_netif = p_item->tx_if;

	if( ppa_check_is_pppoe_netif(tx_netif) &&
			ppa_pppoe_get_physical_if(tx_netif, NULL, underlying_ifname) == PPA_SUCCESS ) {
		if ( (tx_netif = ppa_get_netif(underlying_ifname)) == NULL ) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_pppoe_get_physical_if failed. abort \n");
			return 0;
		}
	}

	if( tx_netif->type == ARPHRD_SIT){ //6RD device
		if(ppa_get_6rd_phyif_fn == NULL || (tx_netif = ppa_get_6rd_phyif_fn(tx_netif)) == NULL){
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"6RD, cannot get physical device\n");
			return PPA_FAILURE;
		}
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"6RD physical device name: %s \n", tx_netif->name);
	} else if(tx_netif->type == ARPHRD_TUNNEL6) {
#if defined(CONFIG_LTQ_PPA_DSLITE) && CONFIG_LTQ_PPA_DSLITE
		if(ppa_get_ip4ip6_phyif_fn == NULL || (tx_netif = ppa_get_ip4ip6_phyif_fn(tx_netif)) == NULL) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Add ip4ip6 device: %s, but cannot find the physical device\n", tx_netif->name);
			return PPA_FAILURE;
		}
#endif          
	}

	session_info = &ppa_buf->pp_packet_info->pp_session;

	if (session_info == NULL) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " session_info inside skb is NULL. abort\n" );
		return 0;
	}

	session_info->session_handle = AVALANCHE_PP_MAX_ACCELERATED_SESSIONS;
	SKB_GET_PP_INFO_P(ppa_buf)->egress_queue = TI_PPM_EGRESS_QUEUE_INVALID;

	ppa_pp_qos_is_vpid_registered(tx_netif->vpid_handle, &isVpidRegistered);
	if ((isVpidRegistered)) {
		if (NULL != tx_netif->qos_select_hook) {
			SKB_GET_PP_INFO_P(ppa_buf)->egress_queue = tx_netif->qos_select_hook(ppa_buf);
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " egress_queue[0x%x]\n", SKB_GET_PP_INFO_P(ppa_buf)->egress_queue);
		}
	} else {
		session_info->phy_cluster_id  = PP_QOS_INVALID_CLUSTER;
		session_info->priority = PP_QOS_DEFAULT_PRIORITY;
	}

	if ((ppa_buf->pp_packet_info->flags & TI_HIL_PACKET_FLAG_PP_SESSION_INGRESS_RECORDED) == 0) {   
		/* If the Packet has not HIT the ingress hook there is no point in creating the session since the packet is locally generated */
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " num_other_pkts [excedes]. Abort \n");
		return 0;
	}   

	if (ppa_buf->pp_packet_info->flags & TI_HIL_PACKET_FLAG_PP_SESSION_BYPASS) {
		/* The Host Intelligence layers have decided not to "acclerate" this session */
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " num_bypassed_pkts [nothing]. Abort \n");
		return 0;
	}

	if (tx_netif == NULL || tx_netif->pid_handle == -1) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " pid_handle == -1. Unable to proceed. Abort\n");
		return 0;
	}

	ppa_setup_egress_property(&session_info->egress, tx_netif, p_item);

	if ( p_item->flag2 & SESSION_FLAG2_BRIDGED_SESSION)
		session_info->is_routable_session = 0;
	else
		session_info->is_routable_session = 1;

	session_info->session_type = AVALANCHE_PP_SESSIONS_TYPE_DATA;

	rc = ppa_avalanche_pp_session_create(session_info, (void*)ppa_buf);

	if ((rc != PP_RC_SUCCESS) && (rc != PP_RC_OBJECT_EXIST)) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " error in ppa_avalanche_pp_session_create(). num_sessions_create_error++ [nothing]. Abort \n");
		return 0;
	} else {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, " session [%u] was created successfully \n", session_info->session_handle);
	}
	return 0;
}

EXPORT_SYMBOL(ppa_avalanche_pp_vpid_get_info);
EXPORT_SYMBOL(ppa_pp_qos_is_vpid_registered);
EXPORT_SYMBOL(ppa_avalanche_pp_session_create);
EXPORT_SYMBOL(ppa_avalanche_pp_state_is_active);
EXPORT_SYMBOL(ppa_avalanche_pp_pid_get_info);
