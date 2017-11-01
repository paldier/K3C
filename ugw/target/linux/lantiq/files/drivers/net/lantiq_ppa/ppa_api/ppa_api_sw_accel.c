/*******************************************************************************
**
** FILE NAME    : ppa_api_sw_accel.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 12 Sep 2013
** AUTHOR       : Lantiq
** DESCRIPTION  : Function to offload CPU and increase Performance
**        once PPE sessions are exhausted.
** COPYRIGHT    :              Copyright (c) 2013
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 12 Sep 2013  Kamal Eradath          Initiate Version
** 14 Nov 2013  Kamal Eradath          Ported to kernel 3.10
*******************************************************************************/

/*
 *  Common Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/ip.h>
#include <linux/swap.h>
#include <linux/ipv6.h>
#include <linux/if_vlan.h>
#include <net/ip.h>
#include <net/route.h>
#include <net/protocol.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/ipv6.h>
#include <net/ip6_tunnel.h>
#include <net/ip_tunnels.h>
#include <net/xfrm.h>

/*
 *  PPA Specific Head File
 */
#include "ppa_ss.h"
#include <net/ppa_api_common.h>
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#include "ppa_api_session_limit.h"
#endif
#include <net/ppa_hook.h>
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_hal_selector.h"
#include "ppa_api_core.h"
#include "ppa_api_tools.h"
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_COC_SUPPORT)
#include "ppa_api_cpu_freq.h"
#endif
#else
#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#endif
#endif

volatile unsigned char g_sw_fastpath_enabled=0;

#if !defined(CONFIG_LTQ_PPA_GRX500) || (CONFIG_LTQ_PPA_GRX500 == 0)
#define FLG_HDR_OFFSET    64
#endif
#define IPV4_HDR_LEN      20  //assuming no option fields are present
#define IPV6_HDR_LEN      40
#define ETH_HLEN          14  /* Total octets in header.   */

#define PROTO_FAMILY_IP   2
#define PROTO_FAMILY_IPV6 10

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

#define PPPOE_HLEN  8
#define PPPOE_IPV4_TAG  0x0021
#define PPPOE_IPV6_TAG  0x0057
#define VLAN_HLEN			4

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#define PARSER_OFFSET_NUM            40

#define PARSER_PPPOE_OFFSET_IDX      14
#define PARSER_IPV4_OUTER_OFFSET_IDX 15
#define PARSER_IPV6_OUTER_OFFSET_IDX 16
#define PARSER_IPV4_INNER_OFFSET_IDX 17
#define PARSER_IPV6_INNER_OFFSET_IDX 18
#endif

// kamal this definition need to be put in a common header file//
// this is the structure of flag header filled by switch and ppe //
struct flag_header {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  //  0 - 39h
  unsigned char   offset[PARSER_OFFSET_NUM];

  //  40 - 43h
  unsigned int    res1; // bit 32-63 : Reserved for future use

  //  43 - 47h
  unsigned int    is_lro_excep        :1; // bit 31
  unsigned int    is_l2tp_data        :1; // bit 30
  unsigned int    is_ip2_udp          :1; // bit 29
  unsigned int    is_inner_ipv6_ext   :1; // bit 28 : FLAG_2IPv6EXT
  unsigned int    is_eapol            :1; // bit 27 : FLAG_EAPOL
  unsigned int    is_ip_frag          :1; // bit 26 : FLAG_IPFRAG
  unsigned int    is_tcp_ack          :1; // bit 25 : FLAG_TCPACK
  unsigned int    is_outer_ipv6_ext   :1; // bit 24 : FLAG_1IPv6EXT
  unsigned int    is_ipv4_option      :1; // bit 23 : FLAG_IPv4OPT
  unsigned int    is_igmp             :1; // bit 22 : FLAG_IGMP
  unsigned int    is_udp              :1; // bit 21 : FLAG_UDP
  unsigned int    is_tcp              :1; // bit 20 : FLAG_TCP
  unsigned int    is_rt_excep         :1; // bit 19 : FLAG_ROUTEXP
  unsigned int    is_inner_ipv6       :1; // bit 18 : FLAG_2IPv6
  unsigned int    is_inner_ipv4       :1; // bit 17 : FLAG_2IPv4
  unsigned int    is_outer_ipv6       :1; // bit 16 : FLAG_1IPv6
  unsigned int    is_outer_ipv4       :1; // bit 15 : FLAG_1IPv4
  unsigned int    is_pppoes           :1; // bit 14 : FLAG_PPPoE
  unsigned int    is_snap_encap       :1; // bit 13 : FLAG_SNAP
  unsigned int    is_vlan             :4; // bit 9-12 : FLAG_1TAG0, FLAG_1TAG1, FLAG_1TAG2, FLAG_1TAG3
  unsigned int    is_spec_tag         :1; // bit 8 : FLAG_ITAG
  unsigned int    res2                :2; // bit 6-7 : Reserved for future use
  unsigned int    is_gre_key          :1; // bit 5
  unsigned int    is_len_encap        :1; // bit 4
  unsigned int    is_gre              :1; // bit 3
  unsigned int    is_capwap           :1; // bit 2
  unsigned int    is_parser_err       :1; // bit 1
  unsigned int    is_wol              :1; // bit 0
#else
  //  0 - 3h
  unsigned int    ipv4_rout_vld       :1;
  unsigned int    ipv4_mc_vld         :1;
  unsigned int    proc_type           :1; // 0: routing, 1: bridging
  unsigned int    res1                :1;
  unsigned int    tcpudp_err          :1; //  reserved in A4
  unsigned int    tcpudp_chk          :1; //  reserved in A4
  unsigned int    is_udp              :1;
  unsigned int    is_tcp              :1;
  unsigned int    res2                :1;
  unsigned int    ip_inner_offset     :7; //offset from the start of the Ethernet frame to the IP field(if there's more than one IP/IPv6 header, it's inner one)
  unsigned int    is_pppoes           :1; //  2h
  unsigned int    is_inner_ipv6       :1;
  unsigned int    is_inner_ipv4       :1;
  unsigned int    is_vlan             :2; //  0: nil, 1: single tag, 2: double tag, 3: reserved
  unsigned int    rout_index          :11;

  //  4 - 7h
  unsigned int    dest_list           :8;
  unsigned int    src_itf             :3; //  7h
  unsigned int    tcp_rstfin          :1; //  7h
  unsigned int    qid                 :4; //  for fast path, indicate destination priority queue, for CPU path, QID determined by Switch
  unsigned int    temp_dest_list      :8; //  only for firmware use
  unsigned int    src_dir             :1; //  0: LAN, 1: WAN
  unsigned int    acc_done            :1;
  unsigned int    res3                :2;
  unsigned int    is_outer_ipv6       :1; //if normal ipv6 packet, only is_inner_ipv6 is set
  unsigned int    is_outer_ipv4       :1;
  unsigned int    is_tunnel           :2; //0-1 reserved, 2: 6RD, 3: Ds-lite

  // 8 - 11h
  unsigned int    sppid               :3; //switch port id
  unsigned int    pkt_len             :13;//packet length
  unsigned int    pl_byteoff          :8; //bytes between flag header and fram payload
  unsigned int    mpoa_type           :2;
  unsigned int    ip_outer_offset     :6; //offset from the start of the Ethernet frame to the IP field

  // 12 - 15h
  unsigned int    tc                  :4; //switch traffic class
  unsigned int    res4                :28;
#endif
};

#define IsSoftwareAccelerated(flags)  ( (flags) & SESSION_ADDED_IN_SW)
#define IsPppoeSession(flags)         ( (flags) & SESSION_VALID_PPPOE )
#define IsLanSession(flags)           ( (flags) & SESSION_LAN_ENTRY )
#define IsValidVlanIns(flags)         ( (flags) & SESSION_VALID_VLAN_INS )
#define IsValidOutVlanIns(flags)      ( (flags) & SESSION_VALID_OUT_VLAN_INS)
#define IsIpv6Session(flags)          ( (flags) & SESSION_IS_IPV6)
#define IsTunneledSession(flags)      ( (flags) & (SESSION_TUNNEL_DSLITE | SESSION_TUNNEL_6RD))
#define IsDsliteSession(flags)        ( (flags) & SESSION_TUNNEL_DSLITE )
#define Is6rdSession(flags)           ( (flags) &  SESSION_TUNNEL_6RD)
#define IsL2TPSession(flags)          ( (flags) & SESSION_VALID_PPPOL2TP)
#define IsValidNatIP(flags)           ( (flags) & SESSION_VALID_NAT_IP)

#define IsBridgedSession(p_item)       ( ((p_item)->flag2) & SESSION_FLAG2_BRIDGED_SESSION) 

static int flag_header_ipv6( struct flag_header *pFlagHdr, 
                              const unsigned char* data,
                              unsigned char data_offset );
static int flag_header_ipv4( struct flag_header *pFlagHdr, 
                              const unsigned char* data,
                              unsigned char data_offset);

typedef unsigned int  uint32_t;
typedef unsigned short  uint16_t;

#ifdef CONFIG_NET_CLS_ACT
extern struct sk_buff *handle_ing(struct sk_buff *skb, struct packet_type **pt_prev,
                                         int *ret, struct net_device *orig_dev);
extern int check_ingress(struct sk_buff *skb);
#endif

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
extern int ppa_sw_litepath_local_deliver(struct sk_buff *skb);
#endif

unsigned short swa_sw_out_header_len( struct session_list_item *p_item,  
                                      /* ETH type of outgoing packet */
                                      unsigned short *ethtype 
                                    ) 
{

  uint16_t headerlength=0;
       
   *ethtype = ETH_P_IP;
  if( ! IsBridgedSession(p_item) ) {
   
   if( IsDsliteSession(p_item->flags) ) {
     /* Handle DS-Lite Tunneled sessions */ 
     if( IsLanSession( p_item->flags ) ) {
       headerlength += IPV6_HDR_LEN;
       *ethtype = ETH_P_IPV6; 
     } 
   } else if( Is6rdSession(p_item->flags) ) {
     /* Handle DS-Lite Tunneled sessions */ 
     if(  IsLanSession( p_item->flags) ) {
       headerlength += IPV4_HDR_LEN;
     } else {
       *ethtype = ETH_P_IPV6; 
     }
   }
    
   if( IsLanSession(p_item->flags) && IsPppoeSession(p_item->flags) ) {
      headerlength += PPPOE_HLEN;
      *ethtype = ETH_P_PPP_SES; 
   } 
  }
 
   if( IsValidVlanIns(p_item->flags) ) {
      headerlength += VLAN_HLEN;
      *ethtype = ETH_P_8021Q; 
   }

   if( IsLanSession(p_item->flags) && IsValidOutVlanIns(p_item->flags) ) {
      headerlength += VLAN_HLEN;
      *ethtype = ETH_P_8021Q; 
   }

   if ( !(p_item->flags & SESSION_TX_ITF_IPOA_PPPOA_MASK) ) {
      headerlength += ETH_HLEN;  /* mac header offset */
   } else {
      *ethtype = 0;
   }
  
   return headerlength;   
}

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
static inline struct dst_entry * get_pkt_dst(PPA_BUF *skb, PPA_NETIF* netif)
{
    struct dst_entry *dst=NULL;
	
    if(((dst=skb_dst(skb))!=NULL) && (dst->obsolete != DST_OBSOLETE_NONE)) {
	return dst;
    } else {
	int err = ip_route_input_noref(skb, ip_hdr(skb)->daddr, ip_hdr(skb)->saddr,
                                               ip_hdr(skb)->tos, netif);
        if (unlikely(err)) {
	    return NULL;
	}
		
	if(((dst=skb_dst(skb))!=NULL) && (dst->obsolete != DST_OBSOLETE_NONE)) {
            return (void*) dst;
	} else {
	    return NULL;
	}
    }	
}
#endif

static inline int get_dslite_tunnel_header(PPA_NETIF *dev, struct ipv6hdr *ip6hdr)
{
    struct ip6_tnl *t;

    if(dev->type != ARPHRD_TUNNEL6 ){
        return -1;
    }
    t = (struct ip6_tnl *)netdev_priv(dev);

    ppa_memset(ip6hdr, 0, sizeof(*ip6hdr));
    ip6hdr->version = 6;
    ip6hdr->hop_limit = t->parms.hop_limit;
    ip6hdr->nexthdr = IPPROTO_IPIP;
    ipv6_addr_copy(&ip6hdr->saddr, &t->parms.laddr);
    ipv6_addr_copy(&ip6hdr->daddr, &t->parms.raddr);

    return 0;
}

static inline int get_6rd_tunnel_header(PPA_NETIF *dev, struct iphdr* iph)
{
    
  struct ip_tunnel *t;

  if(dev->type != ARPHRD_SIT ){
      return -1;
  } 
  
  t = (struct ip_tunnel *)netdev_priv(dev);

  ppa_memset(iph, 0, sizeof(struct iphdr));
  iph->version        = 4;
  iph->protocol       = IPPROTO_IPV6;
  iph->ihl        = 5;
  iph->ttl        = 64;
  iph->saddr      = t->parms.iph.saddr;
  iph->daddr      = 0; /* Don't use tunnel destination address; Later, it is selected based on IPv6 dst address. */

  return 0;
}

/* This function reads the necessary information for software acceleation from  skb and updates the p_item 
*
*/
int32_t sw_update_session(PPA_BUF *skb, struct session_list_item *p_item, struct netif_info *tx_ifinfo)
{
  int ret = PPA_SUCCESS;
  unsigned short tlen=0;
  unsigned short proto_type;
  unsigned isIPv6 = 0;
  unsigned char* hdr;

  t_sw_hdr  swaHdr;
  t_sw_hdr  *p_swaHdr;

  // if the header is already allocated return
  if(p_item->sah == NULL) {

    //allocate memory for thesw_acc_hdr datastructure
    ppa_memset(&swaHdr,0,sizeof(t_sw_hdr)); 
   
    //default tx handler 
    swaHdr.tx_handler =  &dev_queue_xmit;

    if(tx_ifinfo) 
    {
	//get the actual txif
	swaHdr.tx_if = ppa_get_netif(tx_ifinfo->phys_netif_name);
	if( tx_ifinfo->flags & NETIF_BRIDGE) {
	    if ( (ret = ppa_get_br_dst_port(swaHdr.tx_if, skb, &swaHdr.tx_if)) != PPA_SUCCESS ){
	    // ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Failed to get the device under bridge!!\n");
		return PPA_FAILURE;
	    }   
	}
    }
      
    if( IsIpv6Session(p_item->flags) )
      isIPv6 = 1;
  
    /* 
     * Find the length of the header to be uppended 
     */
    tlen =  swa_sw_out_header_len(p_item, &proto_type);
    
    if( IsBridgedSession(p_item) ) {
      swaHdr.network_offset = tlen;
      swaHdr.transport_offset = tlen + ((isIPv6)?IPV6_HDR_LEN:IPV4_HDR_LEN);
      swaHdr.tot_hdr_len =  tlen; 
      swaHdr.type = SW_ACC_TYPE_BRIDGED;
    } else if( IsTunneledSession(p_item->flags) ) {
      
      swaHdr.tot_hdr_len = tlen;

      if( IsDsliteSession(p_item->flags) ) {
        swaHdr.type = SW_ACC_TYPE_DSLITE;
        if( IsLanSession(p_item->flags) ) {
          swaHdr.network_offset = tlen - IPV6_HDR_LEN;
          swaHdr.transport_offset = tlen ; /* transport header is poingting to inner IPv4 */
          isIPv6 = 1;
        } else {
          swaHdr.network_offset = tlen;
          swaHdr.transport_offset = tlen + IPV4_HDR_LEN;
        }
      } else {
        /* 6rd tunnel */
        swaHdr.type = SW_ACC_TYPE_6RD;
        if( IsLanSession(p_item->flags) ) {
          swaHdr.network_offset = tlen - IPV4_HDR_LEN;
          swaHdr.transport_offset = tlen ; /* transport header is poingting to inner IPv4 */
          isIPv6 = 0;
        } else {
          swaHdr.network_offset = tlen;
          swaHdr.transport_offset = tlen + IPV6_HDR_LEN;
        }

      }
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
    // local session = CPU_BOUND 
    } else if((p_item->flag2 & SESSION_FLAG2_CPU_BOUND) && !tx_ifinfo) {
	swaHdr.network_offset = tlen;
	swaHdr.transport_offset = tlen + ((isIPv6)?IPV6_HDR_LEN:IPV4_HDR_LEN);
	swaHdr.tot_hdr_len =  swaHdr.transport_offset;
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
	if(p_item->flag2 & SESSION_FLAG2_LRO) {
	    swaHdr.type = SW_ACC_TYPE_LTCP;
	} else {
	    swaHdr.type = SW_ACC_TYPE_LTCP_LRO;
	}
#else
	swaHdr.type = SW_ACC_TYPE_LTCP;
#endif 
	swaHdr.tx_if = ppa_get_pkt_src_if(skb); // rx_if will be bridge interface; we save it for setting skb->dev in the accelerated path
	swaHdr.dst = get_pkt_dst(skb, swaHdr.tx_if); // skb->dst to be stored here for forwarding.		
    	//default special tx handler for tcp local in traffic
    	swaHdr.tx_handler = &ppa_sw_litepath_local_deliver;
//printk("in %s %d network_offset=%d transport_offset=%d swaHdr.tot_hdr_len=%d\n",__FUNCTION__, __LINE__, swaHdr.network_offset, swaHdr.transport_offset, swaHdr.tot_hdr_len);
//swa_dump_skb(128,skb);
#endif
    } else {
      /* IPV4/IPV6 session */
      swaHdr.network_offset = swaHdr.transport_offset = tlen;
      if( isIPv6 ) {
        swaHdr.type = SW_ACC_TYPE_IPV6;
        swaHdr.transport_offset += IPV6_HDR_LEN;
      } else {
        swaHdr.type = SW_ACC_TYPE_IPV4;
        swaHdr.transport_offset += IPV4_HDR_LEN;
      }
    
      /* 
       * Since copying original IPV4/IPV6 from skb, so need to allocate memory 
       * for network header. While accelerating, the network header is also 
       * copied to skb. Copying network header into skb can be avoided if 
       * NATing is done during acceleration 
       */
      swaHdr.tot_hdr_len =  swaHdr.transport_offset; 
    }

    /* 
     * Allocate memory
     * Now software header + header to be copied is allocated in contineous
     * memory
     */
    p_swaHdr = (t_sw_hdr*)ppa_malloc(sizeof(t_sw_hdr)+swaHdr.tot_hdr_len);
    if(p_swaHdr == NULL) {
      return PPA_ENOMEM;
    }
    ppa_memcpy(p_swaHdr, &swaHdr,sizeof(t_sw_hdr));

    hdr = p_swaHdr->hdr;

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
    if  ( swaHdr.type != SW_ACC_TYPE_LTCP 
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
	&& swaHdr.type != SW_ACC_TYPE_LTCP_LRO 
#endif
	)  /* no template buffer needed for local traffic */
#endif
    {
	//construct the datalink header
	if( !(p_item->flags & SESSION_TX_ITF_IPOA_PPPOA_MASK) )  /* put ethernet header */
	{
	    if( IsBridgedSession(p_item) ) {
		ppa_memcpy(hdr,skb->data, ETH_ALEN*2);  
	    }else {
	    //get the MAC address of txif
		ppa_get_netif_hwaddr(p_item->tx_if, hdr + ETH_ALEN, 1);
		ppa_memcpy(hdr, p_item->dst_mac, ETH_ALEN);
	    }
	    hdr += ETH_ALEN*2;
    
	    /* If the destination interface is a LAN VLAN interface under bridge the 
	    below steps header is not needed */
	    if( IsValidOutVlanIns(p_item->flags) && IsLanSession(p_item->flags) ) { 
      
		*((uint32_t*)(hdr)) = htonl(p_item->out_vlan_tag); 
		hdr +=VLAN_HLEN;
	    }
      
	    if( IsValidVlanIns(p_item->flags) ) {
	
	        *((uint16_t*)(hdr)) = htons(ETH_P_8021Q); 
		*((uint16_t*)(hdr+2)) = htons(p_item->new_vci); 
		hdr +=VLAN_HLEN;
	    }

	    if( IsLanSession(p_item->flags) && IsPppoeSession(p_item->flags) ) {
		proto_type=ETH_P_PPP_SES;
	    } else if( isIPv6 ) {
		proto_type=ETH_P_IPV6;
	    } else {
		proto_type=ETH_P_IP;
	    }
      
	    *((uint16_t*)(hdr)) = htons(proto_type);
	    hdr += 2; /* Two bytes for ETH protocol field */
      
	    // construct pppoe header
	    if( IsLanSession(p_item->flags) && IsPppoeSession(p_item->flags) ) { 
		//struct swa_pppoe_hdr *ppphdr; //Make use of this struct
		*((uint16_t*)(hdr)) = htons(0x1100);
		*((uint16_t*)(hdr+2)) = p_item->pppoe_session_id; //sid
		/* payload length: Actual payload length will be updated in data path */
		*((uint16_t*)(hdr+4)) = 0x0000;
		if(isIPv6) { // ppp type ipv6
		    *((uint16_t*)(hdr+6)) = htons(PPPOE_IPV6_TAG);
		} else {
		    *((uint16_t*)(hdr+6)) = htons(PPPOE_IPV4_TAG);
		}
		hdr += PPPOE_HLEN;
	    }
	}

	if(IsBridgedSession(p_item) ) {
	    goto hdr_done;
	}

	//Now 'hdr' should point to network header
	if( ! IsTunneledSession( p_item->flags) ) {
	    //copy the network header to the buffer
	    ppa_memcpy(p_swaHdr->hdr + p_swaHdr->network_offset, 
                      skb->data, (p_swaHdr->transport_offset)-(p_swaHdr->network_offset));  

	    if( p_item->nat_ip.ip && IsValidNatIP(p_item->flags) && (isIPv6 = 0) ) {
		if( IsLanSession(p_item->flags) ) {  
		    //replace source ip
		    ppa_memcpy(p_swaHdr->hdr + p_swaHdr->network_offset + 12, &p_item->nat_ip.ip, 4);  
		} else {
		    //replace destination ip
		    ppa_memcpy(p_swaHdr->hdr + p_swaHdr->network_offset + 16, &p_item->nat_ip.ip, 4);  
		}
	    }
	} else if( IsLanSession(p_item->flags) ) {
	    /* Add Tunnel header here */
	    if( IsDsliteSession(p_item->flags) ) {
      
		struct ipv6hdr ip_6hdr;
		get_dslite_tunnel_header(p_item->tx_if,&ip_6hdr);
		ppa_memcpy(p_swaHdr->hdr+p_swaHdr->network_offset, &ip_6hdr, sizeof(ip_6hdr));
	    } else if( Is6rdSession(p_item->flags) ) {
      
		struct iphdr iph;
		get_6rd_tunnel_header(p_item->tx_if, &iph);
		iph.daddr = p_item->sixrd_daddr;
		ppa_memcpy(p_swaHdr->hdr+p_swaHdr->network_offset, &iph, sizeof(iph));
	    }
	}
    }

hdr_done:    
    /* Software Qeueing */
#ifdef CONFIG_NETWORK_EXTMARK
    p_swaHdr->extmark = skb->extmark;
#endif
    p_item->sah=p_swaHdr;
  }
  
  return ret;
}

int32_t sw_add_session(PPA_BUF *skb, struct session_list_item *p_item) {

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
  // local session = CPU_BOUND 
  if( (p_item->flag2 & SESSION_FLAG2_CPU_BOUND) ) {
	// in case of local in traffic
	// session can be sw accelerated IFF  1: session is added to LRO 2: session cannot be added to lro 
	// in case of local out traffic SESSION_FLAG2_ADD_HW_FAIL will always be set
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO
	if(!(p_item->flag2 & SESSION_FLAG2_LRO) && !(p_item->flag2 & SESSION_FLAG2_ADD_HW_FAIL)) {
//	    return PPA_FAILURE;
  	}
#endif
  }
#endif 

  if(p_item->flags & SESSION_ADDED_IN_SW)
    return PPA_SUCCESS;

  if( !(p_item->flags & SESSION_CAN_NOT_ACCEL) && g_sw_fastpath_enabled) { 
    p_item->flags |= SESSION_ADDED_IN_SW;
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    update_session_mgmt_stats(p_item, ADD);
#endif 
  }
  
  return PPA_SUCCESS;
}

void sw_del_session(struct session_list_item *p_item)
{
  
  if(p_item->sah) {
    ppa_free(p_item->sah);
    p_item->sah = NULL;   
  }
  if( (p_item->flags & SESSION_ADDED_IN_SW) ) {
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
	update_session_mgmt_stats(p_item, DELETE); 
#endif
    p_item->flags &=  ~SESSION_ADDED_IN_SW;
  }
}

static int flag_header_ipv4( struct flag_header *pFlagHdr, 
                              const unsigned char* data,
                              unsigned char data_offset)
{
  int isValid=1;
  struct iphdr *iph = (struct iphdr*)(data);
  struct tcphdr *tcph;

#if !defined(CONFIG_LTQ_PPA_GRX500) || (CONFIG_LTQ_PPA_GRX500 == 0)
  pFlagHdr->is_inner_ipv4 = 1;
  pFlagHdr->ip_inner_offset = data_offset;
#endif

  switch(iph->protocol)
  {
    case IPPROTO_UDP:
      {
        pFlagHdr->is_udp = 1;
        break;
      }
    case IPPROTO_TCP:
      {
        pFlagHdr->is_tcp = 1;
        tcph = (struct tcphdr *)(data + IPV4_HDR_LEN);  
        if(tcph->rst||tcph->fin) {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
          pFlagHdr->is_rt_excep = 1;
#else
          pFlagHdr->tcp_rstfin = 1;
#endif
        }

        break;
      }
    case IPPROTO_IPV6:
      {
        data_offset += sizeof(*iph);
        pFlagHdr->is_inner_ipv4 = 0;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        pFlagHdr->offset[PARSER_IPV4_INNER_OFFSET_IDX] = 0;
        pFlagHdr->is_inner_ipv6 = 1;
        pFlagHdr->offset[PARSER_IPV6_INNER_OFFSET_IDX] = data_offset;
#endif
        isValid=flag_header_ipv6(pFlagHdr,data+sizeof(*iph), data_offset);
        break;
      }
    default:
      isValid = 0;
      break;
  }

  return isValid;
}

static int flag_header_ipv6( struct flag_header *pFlagHdr, 
                              const unsigned char* data,
                              unsigned char data_offset )
{
  int isValid=1;
  struct ipv6hdr *ip6h = (struct ipv6hdr*)(data);

#if !defined(CONFIG_LTQ_PPA_GRX500) || (CONFIG_LTQ_PPA_GRX500 == 0)
  pFlagHdr->is_inner_ipv6 = 1;
  pFlagHdr->ip_inner_offset = data_offset;
#endif
  switch(ip6h->nexthdr)
  {
    case IPPROTO_UDP:
      pFlagHdr->is_udp = 1;
      break;

    case IPPROTO_TCP:
      {
        struct tcphdr *tcph;

        pFlagHdr->is_tcp = 1;
        tcph = (struct tcphdr *)(data + IPV6_HDR_LEN);
        if(tcph->rst||tcph->fin) 
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
          pFlagHdr->is_rt_excep = 1;
#else
          pFlagHdr->tcp_rstfin = 1;
#endif
        break;
      }
    case IPPROTO_IPIP:
      {
        data_offset += sizeof(*ip6h);
        pFlagHdr->is_inner_ipv6 = 0;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        pFlagHdr->offset[PARSER_IPV6_INNER_OFFSET_IDX] = 0;
        pFlagHdr->is_inner_ipv4 = 1;
        pFlagHdr->offset[PARSER_IPV4_INNER_OFFSET_IDX] = data_offset;
#endif
        isValid=flag_header_ipv4(pFlagHdr,data+sizeof(*ip6h), data_offset);
        break;
      }
    default:
      isValid = 0;
      break;
  }
  return isValid;
}

static int set_flag_header( struct flag_header *pFlagHdr, 
                         unsigned short ethType,
                         const unsigned char* data,
                         unsigned char data_offset)
{
  int isValid=1;  
  switch(ntohs(ethType)) 
  {
  case ETH_P_IP:
    {
      pFlagHdr->is_outer_ipv4 = 1;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      pFlagHdr->offset[PARSER_IPV4_OUTER_OFFSET_IDX] = data_offset;
#endif
      isValid=flag_header_ipv4(pFlagHdr, data, data_offset);
      break;
    }
  case ETH_P_IPV6:
    {
      pFlagHdr->is_outer_ipv6 = 1;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      pFlagHdr->offset[PARSER_IPV6_OUTER_OFFSET_IDX] = data_offset;
#endif
      isValid=flag_header_ipv6(pFlagHdr, data, data_offset);
      break;
    }
  case ETH_P_PPP_SES:
    {
      pFlagHdr->is_pppoes = 1;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      pFlagHdr->offset[PARSER_PPPOE_OFFSET_IDX] = data_offset;
#endif
      data_offset += 8;
      if((*(unsigned short*)(data+6)) == htons(PPPOE_IPV4_TAG))
      {
        pFlagHdr->is_outer_ipv4 = 1;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        pFlagHdr->offset[PARSER_IPV4_OUTER_OFFSET_IDX] = data_offset;
#endif
        isValid=flag_header_ipv4(pFlagHdr, data + 8, data_offset);
      }
      else if((*(unsigned short*)(data + 6)) == htons(PPPOE_IPV6_TAG))
      {
        pFlagHdr->is_outer_ipv6 = 1;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
        pFlagHdr->offset[PARSER_IPV6_OUTER_OFFSET_IDX] = data_offset;
#endif
        isValid=flag_header_ipv6(pFlagHdr, data + 8, data_offset);
      } else {
    	isValid=0;
      }
      break;
    }
#ifdef CONFIG_PPA_PUMA7
  case ETH_P_8021Q:
    {
      pFlagHdr->ip_inner_offset += 4;
      pFlagHdr->is_vlan = 1;
	  isValid = set_flag_header(pFlagHdr,*(unsigned short*)(data+2),data+4, data_offset+4);

      break;
    }
#endif
  default:
    isValid=0;
    break;
  }

  return isValid;
}

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static inline unsigned char *skb_data_begin(PPA_BUF *skb)
{
	struct dma_rx_desc_2 *desc_2 = (struct dma_rx_desc_2 *)&((struct sk_buff *)skb)->DW2;

	return (unsigned char *)(desc_2->field.data_ptr + 2 /* desc_3->field.byte_offset */);
}
#endif


static inline unsigned char *get_skb_flag_header(PPA_BUF *skb)
{
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  return (skb_data_begin(skb));
#else
  return (skb->head + FLG_HDR_OFFSET);
#endif
}

static inline unsigned int IsSoftwareAccelerable(struct flag_header *flg_hdr)
{
  //if the packet is UDP or is TCP and not RST or FIN
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  return ( ( flg_hdr->is_udp || flg_hdr->is_tcp ) && !flg_hdr->is_rt_excep );
#else
  return ( flg_hdr->is_udp || ( flg_hdr->is_tcp && !flg_hdr->tcp_rstfin ) );
#endif
}

static inline unsigned int get_ip_inner_offset(struct flag_header *flg_hdr)
{
  unsigned int ip_inner_offset;

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  if (flg_hdr->is_inner_ipv4) {
    ip_inner_offset = flg_hdr->offset[PARSER_IPV4_INNER_OFFSET_IDX]; 
  } else if (flg_hdr->is_inner_ipv6) {
    ip_inner_offset = flg_hdr->offset[PARSER_IPV6_INNER_OFFSET_IDX]; 
  } else if (flg_hdr->is_outer_ipv4) {
    ip_inner_offset = flg_hdr->offset[PARSER_IPV4_OUTER_OFFSET_IDX]; 
  } else {
    ip_inner_offset = flg_hdr->offset[PARSER_IPV6_OUTER_OFFSET_IDX]; 
  }
#else
  ip_inner_offset = flg_hdr->ip_inner_offset;
#endif

  return ip_inner_offset;
}

static inline unsigned char get_pf(struct flag_header *flg_hdr)
{
  unsigned char pf;

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  if (flg_hdr->is_inner_ipv4) {
    pf = PROTO_FAMILY_IP;
  } else if (flg_hdr->is_inner_ipv6) {
    pf = PROTO_FAMILY_IPV6;
  } else if (flg_hdr->is_outer_ipv4) {
    pf = PROTO_FAMILY_IP;
  } else {
    pf = PROTO_FAMILY_IPV6;
  }
#else
  pf = flg_hdr->is_inner_ipv4?PROTO_FAMILY_IP:PROTO_FAMILY_IPV6;
#endif

  return pf;
}

static int sw_mod_ipv4_skb( PPA_BUF *skb, 
                            struct session_list_item *p_item)
{
  
  t_sw_hdr  *swa;
  struct iphdr org_iph;
  struct iphdr *iph=NULL;
  
  swa = (t_sw_hdr*)(p_item->sah);
  
  ppa_memcpy(&org_iph, skb->data,sizeof(org_iph));
  /* skb has enough headroom available */  

  /* set the skb->data to the point where we can copy the new header which includes (ETH+VLAN*+PPPoE*+IP) 
    *optional  */
  skb_push(skb, swa->tot_hdr_len - IPV4_HDR_LEN);

  // copy the header buffer to the packet
  ppa_memcpy(skb->data, swa->hdr, swa->tot_hdr_len);
  // set the skb pointers porperly
  skb_set_mac_header(skb, 0);
  skb_set_network_header(skb, swa->network_offset); 
  skb_set_transport_header(skb, swa->transport_offset);
  // point to ip header
  iph = (struct iphdr *)skb_network_header(skb);

  // decrement the original ttl update in the packet
  iph->ttl = org_iph.ttl-1; 
  // update the id with original id
  iph->id = org_iph.id; 

  /* Update the ToS for DSCP remarking */
  if ( (p_item->flags & SESSION_VALID_NEW_DSCP) )
    iph->tos |= ((p_item->new_dscp) << 2);

  iph->tot_len = org_iph.tot_len; 
  // calculate header checksum
  iph->check = 0;
  iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl); 
        
  // calculate tcp/udp checksum as the pseudo header has changed
  // we compute only partial checksum using the original value of saddr daddr and port
  switch(iph->protocol)
  {
    case IPPROTO_UDP:
      { 
        struct udphdr *udph;

        udph = (struct udphdr *)skb_transport_header(skb);
        inet_proto_csum_replace4(&udph->check, skb, org_iph.saddr, iph->saddr, 1);
        inet_proto_csum_replace4(&udph->check, skb, org_iph.daddr, iph->daddr, 1);
        if( p_item->nat_port && (p_item->flags & SESSION_VALID_NAT_PORT)) { 
          if( p_item->flags & SESSION_LAN_ENTRY ) {
            inet_proto_csum_replace2(&udph->check, skb, udph->source, p_item->nat_port, 0);
            udph->source = p_item->nat_port;
          } else {
            inet_proto_csum_replace2(&udph->check, skb, udph->dest, p_item->nat_port, 0);
            udph->dest = p_item->nat_port;
          }
        }
        break;
      }
    case IPPROTO_TCP:
      {
        struct tcphdr *tcph;

        tcph = (struct tcphdr *)skb_transport_header(skb);
        inet_proto_csum_replace4(&tcph->check, skb, org_iph.saddr, iph->saddr, 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_iph.daddr, iph->daddr, 1);
        if( p_item->nat_port && (p_item->flags & SESSION_VALID_NAT_PORT)) { 
          if( p_item->flags & SESSION_LAN_ENTRY ) {
            inet_proto_csum_replace2(&tcph->check, skb, tcph->source, p_item->nat_port, 0);
            tcph->source = p_item->nat_port;
          } else {
            inet_proto_csum_replace2(&tcph->check, skb, tcph->dest, p_item->nat_port, 0);
            tcph->dest = p_item->nat_port;
          }
        }
        break;
      }
    default:
      //BUG();
      break;
  }
  return org_iph.tot_len;
}

static int sw_mod_ipv6_skb( PPA_BUF *skb, 
                            struct session_list_item *p_item)
{
  t_sw_hdr  *swa;
  struct ipv6hdr org_ip6;
  struct ipv6hdr *ip6h;

  ppa_memcpy(&org_ip6, (struct ipv6hdr *)skb->data, sizeof(org_ip6));

  swa = (t_sw_hdr*)(p_item->sah);

  /* skb has enough headroom is available */  

  /* set the skb->data to the point where we can copy the new header 
     which includes (ETH+VLAN*+PPPoE*+IP)  */
  skb_push(skb, swa->transport_offset - IPV6_HDR_LEN);

  // copy the header buffer to the packet
  ppa_memcpy(skb->data, swa->hdr, swa->transport_offset);
  // set the skb pointers porperly
  skb_set_mac_header(skb, 0);
  skb_set_network_header(skb, swa->network_offset); 
  skb_set_transport_header(skb, swa->transport_offset);
  ip6h = (struct ipv6hdr *)skb_network_header(skb);
	
  ip6h->hop_limit = org_ip6.hop_limit-1; 
  ip6h->payload_len = org_ip6.payload_len;

#if defined IPV6_NAT

  // this is needed iff there is any ipv6 nat functionality.
  switch(ip6h->nexthdr)
  {
    case IPPROTO_UDP:
      {  
        struct udphdr *udph;

        udph = (struct udphdr *)skb_transport_header(skb);

        inet_proto_csum_replace4(&udph->check, skb, org_ip6.saddr.ip[0], ip6h->saddr.ip[0], 1);
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.saddr.ip[1], ip6h->saddr.ip[1], 1);
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.saddr.ip[2], ip6h->saddr.ip[2], 1);
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.saddr.ip[3], ip6h->saddr.ip[3], 1);
          
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.daddr.ip[0], ip6h->daddr.ip[0], 1);
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.daddr.ip[1], ip6h->daddr.ip[1], 1);
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.daddr.ip[2], ip6h->daddr.ip[2], 1);
        inet_proto_csum_replace4(&udph->check, skb, org_ip6.daddr.ip[3], ip6h->daddr.ip[3], 1);

        if( p_item->nat_port && (p_item->flags & SESSION_VALID_NAT_PORT)) { 
          if( p_item->flags & SESSION_LAN_ENTRY ) {
            inet_proto_csum_replace2(&udph->check, skb, udph->source, p_item->nat_port, 0);
            udph->source = p_item->nat_port;
          } else {
            inet_proto_csum_replace2(&udph->check, skb, udph->dest, p_item->nat_port, 0);
            udph->dest = p_item->nat_port;
          }
        }
        break;
      }
    case IPPROTO_TCP:
      {
        struct tcphdr *tcph;
        
        tcph = (struct tcphdr *)skb_transport_header(skb);

        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.saddr.ip[0], ip6h->saddr.ip[0], 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.saddr.ip[1], ip6h->saddr.ip[1], 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.saddr.ip[2], ip6h->saddr.ip[2], 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.saddr.ip[3], ip6h->saddr.ip[3], 1);
        
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.daddr.ip[0], ip6h->daddr.ip[0], 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.daddr.ip[1], ip6h->daddr.ip[1], 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.daddr.ip[2], ip6h->daddr.ip[2], 1);
        inet_proto_csum_replace4(&tcph->check, skb, org_ip6.daddr.ip[3], ip6h->daddr.ip[3], 1);

        if( p_item->nat_port && (p_item->flags & SESSION_VALID_NAT_PORT)) { 
          if( p_item->flags & SESSION_LAN_ENTRY ) {
	    inet_proto_csum_replace2(&tcph->check, skb, tcph->source, p_item->nat_port, 0);
            tcph->source = p_item->nat_port;
          } else {
            inet_proto_csum_replace2(&tcph->check, skb, tcph->dest, p_item->nat_port, 0);
                                          tcph->dest = p_item->nat_port;
          }
        }
        break;
      }
    default:
      break;
  }
#endif //IPV6_NAT
  return htons(ntohs(ip6h->payload_len) + IPV6_HDR_LEN);
}

static int sw_mod_dslite_skb( PPA_BUF *skb, 
                               struct session_list_item *p_item)
{
  t_sw_hdr  *swa;
  struct iphdr org_iph;
  struct iphdr *iph;
  int ret = 0;

  swa = (t_sw_hdr*)(p_item->sah);

  ppa_memcpy(&org_iph, skb->data, sizeof(org_iph));

  // copy the header buffer to the packet
  skb_push(skb, swa->tot_hdr_len);
  ppa_memcpy(skb->data, swa->hdr, swa->tot_hdr_len);
  skb_set_mac_header(skb, 0);
  skb_set_network_header(skb, swa->network_offset); 
  skb_set_transport_header(skb, swa->transport_offset);
	
  if( IsLanSession(p_item->flags) ) {
    struct ipv6hdr *ip6h;

    ip6h = (struct ipv6hdr *)skb_network_header(skb);
    ip6h->payload_len = org_iph.tot_len ;
    ret = htons(ntohs(org_iph.tot_len) + IPV6_HDR_LEN);
    iph = (struct iphdr *)skb_transport_header(skb);
  } else {
    ret = org_iph.tot_len;
    iph = (struct iphdr *)skb_network_header(skb);
  }

  /* Decrment iph ttl */  
  iph->ttl--;
  iph->check = 0;
  iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl); 
  /* TODO: DSCP marking ??? */
  return ret;
}

static int sw_mod_6rd_skb( PPA_BUF *skb, 
				struct session_list_item *p_item)
{
  t_sw_hdr  *swa;
  struct ipv6hdr org_ip6h;
  struct ipv6hdr *ip6h;
  int ret = 0;

  swa = (t_sw_hdr*)(p_item->sah);

  ppa_memcpy(&org_ip6h, skb->data,sizeof(org_ip6h));
  
  // copy the header buffer to the packet
  skb_push(skb, swa->tot_hdr_len);
  ppa_memcpy(skb->data, swa->hdr, swa->tot_hdr_len);
  skb_set_mac_header(skb, 0);
  skb_set_network_header(skb, swa->network_offset); 
  skb_set_transport_header(skb, swa->transport_offset);
  
  if( IsLanSession(p_item->flags) ) {
  
    struct iphdr *iph;
    
    iph = (struct iphdr *)skb_network_header(skb);
	ret = iph->tot_len = htons(ntohs(org_ip6h.payload_len) + IPV6_HDR_LEN + IPV4_HDR_LEN);

    iph->check = 0;
    iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl); 
    ip6h = (struct ipv6hdr*)skb_transport_header(skb);
  } else {
    ret = htons(ntohs(org_ip6h.payload_len) + IPV6_HDR_LEN);
    ip6h = (struct ipv6hdr *)skb_network_header(skb);
  }

  /* Decrement hop limit */
  ip6h->hop_limit--;
  /* TODO: DSCP marking ??? */
  return ret;
}

static int sw_mod_bridged_skb( PPA_BUF *skb, 
                               struct session_list_item *p_item)
{
  int ret;
  t_sw_hdr  *swa;

  swa = (t_sw_hdr*)(p_item->sah);
  
  ret = skb->len+swa->tot_hdr_len;

  skb_push(skb, swa->tot_hdr_len);
  ppa_memcpy(skb->data, swa->hdr, swa->tot_hdr_len);

  skb_set_mac_header(skb, 0);
  skb_set_network_header(skb, swa->network_offset);
  skb_set_transport_header(skb, swa->transport_offset);
  //dump_packet("SW-out",skb->data,64);

  return ret;
}

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
struct dst_entry * swa_get_pkt_dst(PPA_BUF* skb1, PPA_NETIF* netif)
{
    struct dst_entry *dst1=NULL;
	
    if( ( (dst1=skb_dst(skb1))!=NULL) && ( dst1->obsolete != DST_OBSOLETE_NONE) ) {
	return dst1;
    } else {
	int err = ip_route_input_noref(skb1, ip_hdr(skb1)->daddr, ip_hdr(skb1)->saddr,
                                               ip_hdr(skb1)->tos, netif);
        if (unlikely(err)) {
	    return NULL;
	}
		
	if( ( (dst1=skb_dst(skb1))!=NULL ) && (dst1->obsolete != DST_OBSOLETE_NONE) ) {
            return (void*) dst1;
	} else {
	    return NULL;
	}
    }	
}

static inline int  sw_mod_ltcp(PPA_BUF* skb, t_sw_hdr  *swa, struct iphdr *iph)
{
  //set packet length and packet type = PACKET_HOST
  skb->len = iph->tot_len;
  skb->pkt_type = PACKET_HOST;

  //set the dst
  if(swa->dst->obsolete != DST_OBSOLETE_NONE) {
    dst_hold(swa->dst);
    skb_dst_set(skb,swa->dst);
  } else {
    swa->dst = swa_get_pkt_dst(skb, swa->tx_if); // skb->dst to be stored here for forwarding.		
    if(!swa->dst) return PPA_FAILURE;
    dst_hold(swa->dst);
  }

  //set transport header 
  skb_set_transport_header(skb, (swa->transport_offset - swa->network_offset));
  //set the skb->data point to the transport header
  skb_pull(skb, skb_network_header_len(skb));

  // set the iif
  skb->dev = swa->tx_if;
  skb->skb_iif = skb->dev->ifindex;  

  return PPA_SUCCESS;
//swa_dump_skb(128,skb);
//printk("in %s %d iph->len = %d\n",__FUNCTION__, __LINE__, iph->tot_len);
}

#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
static int sw_mod_ltcp_skb_lro(PPA_BUF *skb, struct session_list_item *p_item)
{
  t_sw_hdr  *swa;
  struct iphdr *iph;

  swa = (t_sw_hdr*)(p_item->sah);

  // set the skb pointers porperly
  skb_set_network_header(skb, 0); 

  iph = (struct iphdr *)skb_network_header(skb);

  //if SESSION_FLAG2_LRO is not already set try adding the session to LRO
  if( !(p_item->flag2 & (SESSION_FLAG2_LRO|SESSION_FLAG2_ADD_HW_FAIL))){
    if ( ppa_lro_entry_criteria(p_item, skb, 0) == PPA_SUCCESS) {
    	p_item->flags &= ~SESSION_ADDED_IN_SW;
	return 0; // force the packet back through stack to do lro learning
    }
  } else {
    swa->type = SW_ACC_TYPE_LTCP;
  } 

  //Do necessary packet modifications
  if(sw_mod_ltcp(skb, swa, iph) == PPA_SUCCESS) {
	return iph->tot_len;
  } else {
	return 0;
  }

}
#endif //CONFIG_LTQ_PPA_LRO

static int sw_mod_ltcp_skb( PPA_BUF *skb, struct session_list_item *p_item)
{
  t_sw_hdr  *swa;
  struct iphdr *iph;

  swa = (t_sw_hdr*)(p_item->sah);

  // set the skb pointers porperly
  skb_set_network_header(skb, 0); 

  iph = (struct iphdr *)skb_network_header(skb);

  //Do necessary packet modifications

   if(sw_mod_ltcp(skb, swa, iph) == PPA_SUCCESS) {
  	return iph->tot_len;
   }  else {
	return 0;
   }
}
#endif

typedef int (*sw_acc_type_fn)(PPA_BUF *skb, struct session_list_item *p_item);
static sw_acc_type_fn afn_SoftAcceleration[SW_ACC_TYPE_MAX] = {
  sw_mod_ipv4_skb,
  sw_mod_ipv6_skb,
  sw_mod_6rd_skb,
  sw_mod_dslite_skb,
  sw_mod_bridged_skb,
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
  sw_mod_ltcp_skb,
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
  sw_mod_ltcp_skb_lro
#endif
#endif
};

int32_t swa_check_ingress(PPA_BUF *skb)
{
#ifdef CONFIG_NET_CLS_ACT
	return check_ingress(skb);
#else
	return -1;
#endif
}
PPA_BUF* swa_handle_ing(PPA_BUF	*skb)
{
#ifdef CONFIG_NET_CLS_ACT
	struct net_device *orig_dev;
	struct packet_type *pt_prev = NULL;
	int ret = 0;
	orig_dev = skb->dev;
	if(skb->protocol != htons(ETH_P_IP)) {
		skb->protocol = htons(ETH_P_IP);
	}
	skb = handle_ing(skb, &pt_prev, &ret, orig_dev);
#endif
	return skb;
}

static inline int get_time_in_sec(void)
{       
        return (jiffies + HZ / 2) / HZ;
}

static inline int swa_get_session_from_skb(PPA_BUF *skb, unsigned char pf, struct session_list_item **pp_item)
{
  skb_reset_network_header((struct sk_buff *)skb);
  return ppa_find_session_from_skb(skb, pf, pp_item);
}

int32_t ppa_do_sw_acceleration(PPA_BUF *skb)
{
  struct flag_header *flg_hdr=NULL,flghdr;
  unsigned int ppa_processed=0;           
  long int ret=PPA_FAILURE;
  unsigned int data_offset;

#ifndef CONFIG_LTQ_PPA_GRX500
  struct iphdr *iph;
#endif
          
   /* datapath driver marks the packet coming from PPE with this flag in skb->mark */    
  if( skb->mark & FLG_PPA_PROCESSED) { //packet coming from ppa datapath driver 
    // kamal: 16 is the length of flag header; flag header found at an offset 64;
    flg_hdr = (struct flag_header *) get_skb_flag_header(skb);
    ppa_processed = 1;
    skb->mark &= ~FLG_PPA_PROCESSED;
  } else {
    /*
     * By default we support only packets from interfaces registred with PPA 
     * which has flag header.
     * The below code is to handle the packets from directpath interfaces which
     * are directly passed to software acceleration. 
     * Limitation: Packets with VLANs are not handled
     */
    unsigned char *data;
        
    ppa_memset(&flghdr,0,sizeof(struct flag_header));
    data_offset = ETH_HLEN;
         
    data = skb->data;    
    if( !set_flag_header(&flghdr,*(unsigned short *)(data - 2), data, data_offset) ) {
      goto normal_path;
    }
    flg_hdr = &flghdr;
  }

  if (IsSoftwareAccelerable(flg_hdr)) {
    struct session_list_item *p_item;
    t_sw_hdr  *swa;
    unsigned int orighdrlen, totlen=0;
             
    /* 
     * skb->data curently pointing to end of mac header
     * if there is vlan header or pppoe header we need to skip them and 
     * point to the begining of network header  
     */
    data_offset = get_ip_inner_offset(flg_hdr) - ETH_HLEN; 

    /*
     * If IPv4 packet is a fragmented, let the stack process it
     */
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    /* NOTE : It may be handled in is_rt_excep flag */
    if( flg_hdr->is_inner_ipv4 && flg_hdr->is_ip_frag ) {
        goto normal_path;
    }
#else
    if( flg_hdr->is_inner_ipv4 ) {
      
      iph = (struct iphdr *) (skb->data+data_offset);
	  if( (!(iph->frag_off & htons(IP_DF))) && ((iph->frag_off) & 0x3FFF)) 
	  { 
        goto normal_path;
      }
    }
#endif
               
    if(data_offset) 
      skb_pull(skb, data_offset);
    p_item = NULL;
    /* If sessions exist try to accelerate the packet */
    if( PPA_SESSION_EXISTS == 
	swa_get_session_from_skb(skb, get_pf(flg_hdr), &p_item)) {
       
      /* 
       * Can the session be accelaratable ? 
       *  - Session must be added into sotware path 
       *  - Not L2TP
       */ 
	if( IsL2TPSession(p_item->flags) || 
        	  ! (IsSoftwareAccelerated(p_item->flags)) ||
          	((skb->len > p_item->mtu) 
#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO
		&& !(p_item->flag2 & SESSION_FLAG2_LRO) 
#endif
          	)) {

	    goto skip_accel;
	}

	swa = (t_sw_hdr*)(p_item->sah);
       
	if(!(p_item->flag2 & SESSION_FLAG2_CPU_BOUND)) { 

#ifdef CONFIG_NETWORK_EXTMARK
	  skb->extmark= p_item->extmark; 
#endif
	  if (!swa_check_ingress(skb)) {
	    skb_set_network_header(skb, swa->network_offset); 
	    skb_set_transport_header(skb, swa->transport_offset);
	    skb = swa_handle_ing(skb);
	    if (!skb) {
		ret = PPA_SUCCESS; /* return NET_RX_DROP; */
		goto skip_accel;
	    }
	  }
	  /* If headroom is not enough, increase the headroom */
	  orighdrlen = get_ip_inner_offset(flg_hdr);
	  orighdrlen += (IPPROTO_IP == get_pf(flg_hdr))?IPV4_HDR_LEN:IPV6_HDR_LEN; 
 	  if(orighdrlen < swa->tot_hdr_len) { 
	    unsigned int reqHeadRoom;
          
	    reqHeadRoom = swa->tot_hdr_len - orighdrlen;
	    /*
	    * Mahipati:: Note
	    * This is not a good idea to re-allocate the skb, even when headroom is
	    * available. This happens only when packet is received from directpath 
	    * interfaces
	    */
	    if ( (!ppa_processed && reqHeadRoom ) || 
                              (skb_headroom(skb) < reqHeadRoom) ) {
            
		PPA_BUF* skb2;
		skb2 = skb_realloc_headroom(skb, reqHeadRoom);
		if (skb2 == NULL) {
		/* Drop the packet */
		    PPA_SKB_FREE(skb);
		    skb = NULL;
		    ret = PPA_SUCCESS; //Must return success
		    goto skip_accel;
		}
		if (skb->sk)
		    skb_set_owner_w(skb2, skb->sk);
		PPA_SKB_FREE(skb);
		skb = skb2;
	    }
	  }
	}
	if(swa->type < SW_ACC_TYPE_MAX && !(totlen = afn_SoftAcceleration[swa->type](skb,p_item))) { 
	    goto skip_accel;
	} 

	if( !IsBridgedSession(p_item) &&  
          IsLanSession(p_item->flags) && IsPppoeSession(p_item->flags) ) {
          
	    //update ppp header with length of the ppp payload
	    struct pppoe_hdr *ppphdr;
	    //ppphdr = (struct pppoe_hdr *)(swa_skb_network_header(skb) - 8);
	    ppphdr = (struct pppoe_hdr *)(skb->data + (swa->network_offset - PPPOE_HLEN));
	    ppphdr->length = htons(ntohs(totlen) + 2); // ip payload length + ppp header length
	}
        
	// set the destination dev
	skb->dev=swa->tx_if;
	skb->vlan_tci &= 0x0111; //reset vlan tci
	/* Mark update for Software queuing */
	skb->mark |= p_item->mark;
#ifdef CONFIG_NETWORK_EXTMARK
        skb->extmark = swa->extmark;
#endif
        skb->priority = p_item->priority;
	/* update the packet counter */
	totlen = skb->len;
	p_item->mips_bytes += totlen;
	p_item->acc_bytes += totlen; 
	/* update last hit time pf the session */
	p_item->last_hit_time = get_time_in_sec();  

	// queue the packet for transmit

	//swa_dump_skb(128,skb);
      	//session tx handler 
	swa->tx_handler(skb);
	ppa_session_put(p_item);
	return PPA_SUCCESS;
    }
 
skip_accel:
    if(p_item)
	ppa_session_put(p_item);

    if( skb && data_offset) 
      skb_push(skb,data_offset);

    return ret;
  }
 
normal_path:
  return PPA_FAILURE;
}

int32_t sw_fastpath_enable(uint32_t f_enable, uint32_t flags)
{
  g_sw_fastpath_enabled=f_enable;
  return PPA_SUCCESS;
}
        
int32_t get_sw_fastpath_status(uint32_t *f_enable, uint32_t flags)
{
  if( f_enable )
      *f_enable = g_sw_fastpath_enabled;
  return PPA_SUCCESS;
}

int32_t sw_session_enable(struct session_list_item *p_item, uint32_t f_enable, uint32_t flags)
{
  if ( f_enable )
    p_item->flags |= SESSION_ADDED_IN_SW;
  else
    p_item->flags &= ~SESSION_ADDED_IN_SW;

  return PPA_SUCCESS;
}
        
int32_t get_sw_session_status(struct session_list_item *p_item, uint32_t *f_enable, uint32_t flags)
{
  if ( f_enable ) {
    if ( (p_item->flags & SESSION_ADDED_IN_SW) )
      *f_enable = 1;
    else
      *f_enable = 0;
  }

  return PPA_SUCCESS;
}

int32_t sw_fastpath_send(PPA_BUF *skb) 
{
    
  if( skb == NULL )
  {
    return PPA_FAILURE;
  }
  
  if(g_sw_fastpath_enabled) {
    return ppa_do_sw_acceleration(skb);
  } 

  return PPA_FAILURE;
}

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
static inline int sw_update_iph(PPA_BUF *skb, int* offset, unsigned char *pf)
{ 
    struct rtable *rt = NULL;
    struct iphdr *iph = NULL;
    struct inet_sock *inet = NULL;
    struct sock *sk = NULL;
    
    if(skb==NULL || skb->sk==NULL) {	
	return PPA_FAILURE;
    }

    rt = skb_rtable(skb);
    sk = skb->sk;
    inet = inet_sk(sk);
#if 0
    if (sk->sk_family==PF_INET6) //ipv6
    {
	 
printk("in %s %d ipv6\n",__FUNCTION__, __LINE__);
	*offset = sizeof(struct ipv6hdr); 
	skb_push(skb, sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);	
	    
	*pf = PF_INET6; 
	ip6h = ipv6_hdr(skb);
	memcpy(&(ip6h->saddr),&(inet->pinet6->saddr),sizeof(struct in6_addr));
	memcpy(&(ip6h->daddr),&(inet->pinet6->daddr),sizeof(struct in6_addr));
	ip6h->nexthdr = sk->sk_protocol;
	ip6h->payload_len = skb->len;
	skb->protocol = ETH_P_IPV6;

    } else { //assume ipv4   
#endif 
	*offset = sizeof(struct iphdr); 
	skb_push(skb, sizeof(struct iphdr));
	skb_reset_network_header(skb);	

	*pf = PF_INET; 
	iph = ip_hdr(skb);
	iph->saddr = inet->inet_saddr;
	iph->daddr = inet->inet_daddr;
	iph->protocol = sk->sk_protocol;
	iph->tot_len = skb->len;
#if ((LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0)) && (LINUX_VERSION_CODE != KERNEL_VERSION(3, 10, 102)))
	ip_select_ident_more(iph, &rt->dst, sk, (skb_shinfo(skb)->gso_segs ?: 1) - 1);
#else
	ip_select_ident_segs(skb, sk, (skb_shinfo(skb)->gso_segs ?: 1));	
#endif
	skb->protocol = ETH_P_IP;
//    }
 
    skb->priority = sk->sk_priority;
    skb->mark = sk->sk_mark;
    return PPA_SUCCESS;
}


int32_t sw_litepath_tcp_send_skb(PPA_BUF *skb)
{
    int32_t offset = 0;
    unsigned char pf=0;
    struct session_list_item *p_item = NULL; 
    t_sw_hdr  *swa=NULL;
    unsigned int totlen=0;

    // put the dummy ip header		
    if(sw_update_iph(skb, &offset, &pf)==PPA_SUCCESS ) { 
	//do lookup
	if( PPA_SESSION_EXISTS == swa_get_session_from_skb(skb, pf, &p_item)) {	
	    //printk("in %s %d session exists\n",__FUNCTION__, __LINE__, offset);
	    //skip non acceleratable sessions    
	    // session is litepath 
	    if( IsSoftwareAccelerated(p_item->flags) 
		&& (p_item->flag2 & SESSION_FLAG2_CPU_BOUND)) { 
		
		swa = (t_sw_hdr*)(p_item->sah);
#if 1
		if(skb_headroom(skb) < swa->tot_hdr_len) {
		    // realloc headroom
		    PPA_BUF *skb2;
		    skb2 = skb_realloc_headroom(skb, swa->tot_hdr_len);
		    if (skb2 == NULL) {
			/* Drop the packet */
			PPA_SKB_FREE(skb);
			skb = NULL;
			goto skip_accel;
		    }
		    
		    if (skb->sk)
			skb_set_owner_w(skb2, skb->sk);
		    PPA_SKB_FREE(skb);
		    skb = skb2;
		}

		// copy priority learnt from session entry.. for scheduling use
		skb->priority = p_item->priority;
		// packet modification
		if((swa->type < SW_ACC_TYPE_MAX) && !(totlen = afn_SoftAcceleration[swa->type](skb,p_item))) {
		    goto skip_accel;
		}
#else
  		swa_skb_push(skb, swa->tot_hdr_len - IPV4_HDR_LEN);

  		// set the skb pointers porperly
  		swa_skb_set_mac_header(skb, 0);
  		swa_skb_set_network_header(skb, swa->network_offset); 
  		swa_skb_set_transport_header(skb, swa->transport_offset);
  		
		struct iphdr *iph = (struct iphdr *)swa_skb_network_header(skb);
		totlen = iph->tot_len;
 
#endif
		
		if( IsPppoeSession(p_item->flags) ) {
		    //update ppp header with length of the ppp payload
		    struct pppoe_hdr *ppphdr;
		    ppphdr = (struct pppoe_hdr *)(skb->data + (swa->network_offset - PPPOE_HLEN));
			ppphdr->length = htons(ntohs(totlen) + 2); // ip payload length + ppp header length
		}
		
		// set the destination dev
		skb->dev = swa->tx_if;
		
		/* update the packet counter */
		totlen = skb->len;
		p_item->mips_bytes += totlen;
		p_item->acc_bytes += totlen;
		/* update last hit time pf the session */
		p_item->last_hit_time = get_time_in_sec();
		
		//swa_dump_skb(128,skb);
		// session tx handler 
		swa->tx_handler(skb);
		ppa_session_put(p_item);
		return PPA_SUCCESS;
	    }	
	} else { 
//printk("in %s %d session not found offset=%d\n",__FUNCTION__, __LINE__, offset);
	}
    }	

skip_accel:
    if(p_item)
	ppa_session_put(p_item);

    if(offset)
	skb_pull(skb,offset);
//printk("in %s %d return failue\n",__FUNCTION__, __LINE__);
    return PPA_FAILURE;
}
               
int32_t sw_litepath_tcp_send(PPA_BUF *skb) 
{
    
  if( skb == NULL )
  {
    return PPA_FAILURE;
  }
  
  if(g_sw_fastpath_enabled) {
    return sw_litepath_tcp_send_skb(skb);
  } 

  return PPA_FAILURE;
}
extern int32_t (*ppa_sw_litepath_tcp_send_hook)(PPA_BUF *);
#endif

extern int32_t (*ppa_sw_add_session_hook)(PPA_BUF* skb, struct session_list_item *p_item);
extern int32_t (*ppa_sw_update_session_hook)(PPA_BUF *skb, struct session_list_item *p_item, struct netif_info *tx_ifinfo);
extern void (*ppa_sw_del_session_hook)(struct session_list_item *p_item);

extern int32_t (*ppa_sw_fastpath_enable_hook)(uint32_t, uint32_t);
extern int32_t (*ppa_get_sw_fastpath_status_hook)(uint32_t *, uint32_t);
extern int32_t (*ppa_sw_session_enable_hook)(struct session_list_item *p_item, uint32_t, uint32_t);
extern int32_t (*ppa_get_sw_session_status_hook)(struct session_list_item *p_item, uint32_t *, uint32_t);
extern int32_t (*ppa_sw_fastpath_send_hook)(PPA_BUF *);

extern int32_t sw_fastpath_send(PPA_BUF *skb);
extern int32_t get_sw_fastpath_status(uint32_t *f_enable, uint32_t flags);
extern int32_t sw_fastpath_enable(uint32_t f_enable, uint32_t flags);
extern int32_t get_sw_session_status(struct session_list_item *p_item, uint32_t *f_enable, uint32_t flags);
extern int32_t sw_session_enable(struct session_list_item *p_item, uint32_t f_enable, uint32_t flags);

int  ppa_sw_init(void)
{
  ppa_sw_update_session_hook = sw_update_session;
  ppa_sw_add_session_hook = sw_add_session;
  ppa_sw_del_session_hook = sw_del_session;
  ppa_sw_fastpath_enable_hook = sw_fastpath_enable;
  ppa_get_sw_fastpath_status_hook = get_sw_fastpath_status;
  ppa_sw_session_enable_hook = sw_session_enable;
  ppa_get_sw_session_status_hook = get_sw_session_status;
  ppa_sw_fastpath_send_hook = sw_fastpath_send;
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
  ppa_sw_litepath_tcp_send_hook = sw_litepath_tcp_send; 
#endif
  g_sw_fastpath_enabled = 1;
  return 0;
}

void  ppa_sw_exit(void)
{
  ppa_sw_update_session_hook = NULL;
  ppa_sw_add_session_hook = NULL;
  ppa_sw_del_session_hook = NULL;
  ppa_sw_fastpath_enable_hook = NULL;
  ppa_get_sw_fastpath_status_hook = NULL;
  ppa_sw_session_enable_hook = NULL;
  ppa_get_sw_session_status_hook = NULL;
  ppa_sw_fastpath_send_hook = NULL;
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
  ppa_sw_litepath_tcp_send_hook = NULL; 
#endif
  g_sw_fastpath_enabled = 0;
}

