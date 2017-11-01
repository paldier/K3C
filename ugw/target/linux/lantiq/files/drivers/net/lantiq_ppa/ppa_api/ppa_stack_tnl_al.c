
/*******************************************************************************
 **
 ** FILE NAME    : ppa_stack_tnl_al.c
 ** PROJECT      : PPA 
 ** MODULES      : PPA Stack Adaptation Layer
 **
 ** DATE         : 12 May 2014
 ** AUTHOR       : Mahipati Deshpande
 ** DESCRIPTION  : Stack Adaptation for tunneled interfaces and sessions.
 ** COPYRIGHT    :              Copyright (c) 2009
 **                          Lantiq Deutschland GmbH
 **                   Am Campeon 3; 85579 Neubiberg, Germany
 **
 **   For licensing information, see the file 'LICENSE' in the root folder of
 **   this software module.
 **
 ** HISTORY
 ** $Date        $Author                $Comment
 ** 12 May 2014   Mahipati Deshpande    Moved tunneled routines to this file
 **                                     Added support for GRE
 *******************************************************************************/
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_arp.h>
#include <net/ip_vs.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <net/addrconf.h>
#include <asm/time.h>
#include <../net/bridge/br_private.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#include <../net/8021q/vlan.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack.h>
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,13)
#include <net/ipip.h>
#else
#include <net/ip_tunnels.h>
#endif
#include <linux/if_tunnel.h>
#include <net/ip6_tunnel.h>
#include <net/gre.h>

#include "ppa_stack_al.h"
#include "ppa_stack_tnl_al.h"
#include "ppa_api.h"
#include "ppa_api_misc.h"

static int is_zero_mac(uint8_t *mac)
{
	return !(mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]);
}

static inline
int32_t ppa_get_mac_from_neigh(uint8_t* mac,struct neighbour *neigh,struct dst_entry *dst)
{
  int32_t ret=PPA_ENOTAVAIL;
  struct hh_cache *hh;

  ppa_neigh_hh_init(neigh,dst);   
  ppa_neigh_update_hhs(neigh);
  hh = &neigh->hh;
  
  if( !hh ) {

    if ( neigh && !is_zero_mac(neigh->ha)) {
      memcpy(mac, (uint8_t *)neigh->ha, ETH_ALEN);
      ret = PPA_SUCCESS;
    }
  } else {
  
    unsigned int seq;
    do {
    
      seq = read_seqbegin(&hh->hh_lock);
      if ( hh->hh_len == ETH_HLEN ) {
        memcpy(mac, 
            (uint8_t *)hh->hh_data + HH_DATA_ALIGN(hh->hh_len) - hh->hh_len, 
            ETH_ALEN);
        ret = PPA_SUCCESS;
      } else
        ret = PPA_ENOTAVAIL;
    } while ( read_seqretry(&hh->hh_lock, seq) );
  }
  
  return ret;
}

/*
 * Note: Caller must make sure dev is IPv4 tunnel 
 */
static
struct net_device* ppa_ipv4_tnl_phy(struct net_device *dev)
{
  struct iphdr *iph;
  struct rtable *rt = NULL;
  struct net_device *phydev = NULL;
  struct ip_tunnel *t;

  t =  netdev_priv(dev);

  iph = &t->parms.iph;

  if (iph->daddr) {
    struct flowi4 fl; 
    memset(&fl,0,sizeof(fl));
    fl.flowi4_tos = RT_TOS(iph->tos);
    fl.flowi4_oif = t->parms.link;
    fl.flowi4_proto = t->parms.iph.protocol; 
    fl.daddr = iph->daddr;
    fl.saddr = iph->saddr; 
    rt=ip_route_output_key(dev_net(dev),&fl);
    if(IS_ERR(rt)) {
      return NULL;
    }
    phydev = rt->dst.dev;
    ip_rt_put(rt);
  } else {
    phydev = __ip_dev_find(&init_net, iph->saddr,false);
  }

  return phydev;   
}

#include <net/arp.h>
/*
 * Returns PPA_SUCCESS if valid destination MAC is available
 * Note: Caller must make sure dev is IPv4 tunnel 
 */
static
int ppa_get_ipv4_tnl_dmac( uint8_t *mac,
                              struct net_device *dev,
                              struct sk_buff *skb)
{
  struct ip_tunnel *t;
  struct iphdr *iph;
  struct rtable *rt = NULL;
  int ret = PPA_ENOTAVAIL;
  struct neighbour* neigh;
  //u32 nexthop;

  t = netdev_priv(dev);
  iph = &t->parms.iph;

  if (iph->daddr) {

    struct flowi4 fl; 
    memset(&fl,0,sizeof(fl));
    fl.flowi4_tos = RT_TOS(iph->tos);
    fl.flowi4_oif = t->parms.link;
    fl.flowi4_proto = t->parms.iph.protocol; 
    fl.daddr = iph->daddr;
    fl.saddr = iph->saddr; 

    rt=ip_route_output_key(dev_net(dev), &fl);
    if(IS_ERR(rt))
      return ret;
  } else {
    return ret; //No daddr so cannot get dmac
  }

  /* 
   * Note: Since sbk's outer IP is not yer formed when packet reached here. 
   * 
   */
  //nexthop = (__force u32) rt_nexthop(rt, iph->daddr);
   __be32 pkey;
  /**To calculate MAC addr of nxt hop based on it is dst addr / gateway addr */
   if (rt->rt_gateway)
	pkey = (__be32 ) rt->rt_gateway;
   else if (skb)
	pkey = (__be32)iph->daddr; 
  neigh = __ipv4_neigh_lookup(rt->dst.dev, (__force u32)(pkey));
//  neigh = __ipv4_neigh_lookup(rt->dst.dev, (__force u32)(iph->daddr));
  if(neigh) {
    ret = ppa_get_mac_from_neigh(mac,neigh,&rt->dst);
    neigh_release(neigh);
  }
  
  ip_rt_put(rt);
  return ret;
}

/*
 * Returns base dev of the given IPv6 tunneled devie
 * Note: Caller must make sure dev is IPv6 tunnel 
 */
static
struct net_device* ppa_ipv6_tnl_phy(struct net_device *dev)
{
  struct dst_entry *dst;
  struct net *net = dev_net(dev);
  struct net_device* phydev = NULL;
  struct ip6_tnl *t;
  struct flowi6 fl6;

  t = netdev_priv(dev);

  memcpy(&fl6, &t->fl.u.ip6, sizeof (fl6));
  fl6.flowi6_proto = t->parms.proto;
  dst = ip6_route_output(net, NULL, &fl6);

  if(!(dst->error)) {
    dst = xfrm_lookup(net, dst, flowi6_to_flowi(&fl6), NULL, 0);
    if (IS_ERR(dst)) 
      return NULL;
  } 
    
  if(dst && dst->dev != dev) {
    phydev=dst->dev;
  }
    
  dst_release(dst);

  return phydev;
}

/*
 * Returns PPA_SUCCESS if destination MAC is copied.
 * Note: Caller should make sure dev is IPv6 tunnel. 
 */
static
int ppa_get_ipv6_tnl_dmac(uint8_t *mac,
                             struct net_device *dev,
                             struct sk_buff *skb)
{
  struct ip6_tnl *t = netdev_priv(dev);
  struct dst_entry *dst = NULL;
	struct flowi6 fl6;
	struct net *net = dev_net(dev);
	int ret = PPA_ENOTAVAIL;

  memcpy(&fl6, &t->fl.u.ip6, sizeof (fl6));
  fl6.flowi6_proto = t->parms.proto;

  dst = ip6_route_output(net, NULL, &fl6);

  if (dst->error) {
    goto no_dst;
  }
  dst = xfrm_lookup(net, dst, flowi6_to_flowi(&fl6), NULL, 0);
  if(IS_ERR(dst)) {
    return ret;
  }

  if(dst->dev == dev) {
    goto no_dst;
  }
    
  ret = ppa_get_dmac_from_dst_entry(mac,skb,dst);
no_dst:
  dst_release(dst);
  return ret;
}

static
int ppa_get_ipv4_tnl_iph(struct iphdr* iph,
                         struct net_device *dev,
                         uint16_t dataLen)
{
  struct ip_tunnel *t;

  t = netdev_priv(dev);

  ppa_memset(iph,0,sizeof(*iph));
  ppa_memcpy(iph, &t->parms.iph,sizeof(*iph));
  iph->tot_len = sizeof(*iph) + dataLen;
  iph->check = ip_fast_csum(iph, iph->ihl);

  return PPA_SUCCESS;
}

static
int ppa_get_ipv6_tnl_iph(struct ipv6hdr* ip6h,
                         struct net_device *dev,
                         uint16_t dataLen)
{
  struct ip6_tnl *t;

  t = netdev_priv(dev);

  ppa_memset(ip6h, 0, sizeof(*ip6h));
  ip6h->version = 6;
  ip6h->hop_limit = t->parms.hop_limit;
  ip6h->nexthdr = t->parms.proto;
  ipv6_addr_copy(&ip6h->saddr, &t->parms.laddr);
  ipv6_addr_copy(&ip6h->daddr, &t->parms.raddr);
  ip6h->payload_len = dataLen;

  return PPA_SUCCESS;
}

/* 
 * Returns 1 if given netif is gre tunnel, else returns zero
 */
uint32_t ppa_is_gre_netif(struct net_device* dev)
{
  uint32_t t_flags = TUNNEL_SEQ;
  
  /* IPv4 GRE/ GRE tap */
  if( dev->type == ARPHRD_IPGRE ||
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {

    t_flags = ((struct ip_tunnel*)netdev_priv(dev))->parms.o_flags;
  } else if( dev->type == ARPHRD_IP6GRE || 
      (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {

    /* IPv6 GRE/ GRE tap */
    t_flags = ((struct ip_tunnel*)netdev_priv(dev))->parms.o_flags;
  }

  /* GRE with sequence number is not supported */
	return (t_flags & TUNNEL_SEQ)?0:1;
}
EXPORT_SYMBOL(ppa_is_gre_netif);

uint32_t ppa_is_gre_netif_type(struct net_device* dev, 
                     uint8_t* isIPv4Gre,
                     uint8_t* isGreTap)
{
  uint32_t t_flags = TUNNEL_SEQ;


  /* IPv4 GRE/ GRE tap */
  if( dev->type == ARPHRD_IPGRE ||
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {
    
    *isIPv4Gre = 1;
    *isGreTap = (dev->type == ARPHRD_IPGRE)?0:1;
    t_flags = ((struct ip_tunnel*)netdev_priv(dev))->parms.o_flags;
  } else if( dev->type == ARPHRD_IP6GRE || 
      (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {

    /* IPv6 GRE/ GRE tap */
    *isIPv4Gre = 0;
    *isGreTap = (dev->type == ARPHRD_IP6GRE)?0:1;
    t_flags = ((struct ip_tunnel*)netdev_priv(dev))->parms.o_flags;
  }  

	return (t_flags & TUNNEL_SEQ)?0:1;
}
EXPORT_SYMBOL(ppa_is_gre_netif_type);

/* 
 * Returns physical interface of the gre tunnel.
 * On failure returns NULL.
 */
struct net_device* ppa_get_gre_phyif(struct net_device* dev)
{
  if( dev->type == ARPHRD_IPGRE || 
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {

    return ppa_ipv4_tnl_phy(dev);
  } else if(dev->type == ARPHRD_IP6GRE ||
           (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {
    return ppa_ipv6_tnl_phy(dev);
  }
  return NULL;
}
EXPORT_SYMBOL(ppa_get_gre_phyif);

/*
 * On success returns destination MAC address.
 */
int32_t ppa_get_gre_dmac(uint8_t *mac,
                     struct net_device* dev,
                     struct sk_buff *skb)
{
  int ret = PPA_FAILURE; 
  struct net_device* phydev;
 
  phydev = ppa_get_gre_phyif(dev); 
  if(!phydev)
    goto mac_err;

  if( ppa_check_is_ppp_netif(phydev)) {
    /* Check if PPPoE interface */
    if ( ppa_check_is_pppoe_netif(phydev) )
      ret= ppa_pppoe_get_dst_mac(phydev, mac);
  } else {

  if( dev->type == ARPHRD_IPGRE || 
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {

    ret = ppa_get_ipv4_tnl_dmac(mac, dev, skb);
  } else if(dev->type == ARPHRD_IP6GRE ||
           (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {

    ret = ppa_get_ipv6_tnl_dmac(mac, dev, skb);
  }
  }

mac_err:
  return ret;
}

int32_t ppa_get_gre_hdrlen(PPA_NETIF* dev, uint16_t *hdrlen)
{
  uint32_t t_flags = 0;
  uint32_t hlen = 4;
  *hdrlen = 0;
  if( dev->type == ARPHRD_IPGRE ||
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {
    struct ip_tunnel *t =  netdev_priv(dev);
    *hdrlen = t->hlen + sizeof(struct iphdr);
    t_flags = t->parms.o_flags;
  } else if( dev->type == ARPHRD_IP6GRE ||
      (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {

    struct ip6_tnl *t =  netdev_priv(dev);
    *hdrlen = t->hlen;// + sizeof(struct ipv6hdr);
    t_flags = t->parms.o_flags;
  } else {
    return PPA_FAILURE;
  }
	
  if(t_flags & TUNNEL_CSUM)
		hlen += 4;
	if(t_flags & TUNNEL_KEY)
		hlen += 4;
#if 0
	if(t_flags & TUNNEL_SEQ)
		hlen += 4; //Not handled...should be an error
#endif

  return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_get_gre_hdrlen);


int32_t ppa_form_gre_hdr(PPA_NETIF* dev, 
                         uint8_t isIPv6, 
                         uint16_t dataLen,
                         uint8_t *pHdr, 
                         uint16_t* len)
{
  int hlen=4;
  uint32_t t_flags;
  uint32_t t_key;
  uint16_t proto;
  uint8_t ipv4 = 0;
  uint16_t ip_hlen;
  struct gre_base_hdr* grehdr; 

  if( dev->type == ARPHRD_IPGRE ||
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {
    
    struct ip_tunnel *t =  netdev_priv(dev);
    t_flags = t->parms.o_flags;
    t_key = t->parms.o_key;
    proto = (dev->type == ARPHRD_IPGRE)?ETH_P_IP:ETH_P_TEB;
    ipv4 = 1;
    ip_hlen = sizeof(struct iphdr);
  } else if( dev->type == ARPHRD_IP6GRE ||
      (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {
    
    struct ip6_tnl *t =  netdev_priv(dev);
    t_flags = t->parms.o_flags;
    t_key = t->parms.o_key;
    proto = (dev->type == ARPHRD_IP6GRE)?ETH_P_IPV6:ETH_P_TEB;
    ip_hlen = sizeof(struct ipv6hdr);
  } else {
    return PPA_FAILURE;
  }

  if(t_flags & TUNNEL_CSUM)
		hlen += 4;
	if(t_flags & TUNNEL_KEY)
		hlen += 4;
 
  if( *len < (hlen + ip_hlen) )
   return PPA_FAILURE;
  
  if( ipv4 ) {
    ppa_get_ipv4_tnl_iph((struct iphdr*)pHdr,dev,hlen+dataLen);
  } else {
    ppa_get_ipv6_tnl_iph((struct ipv6hdr*)pHdr,dev,hlen+dataLen);
    ((struct ipv6hdr*)pHdr)->nexthdr = IPPROTO_GRE;
  }
  pHdr += ip_hlen;
  hlen += ip_hlen;
  
  grehdr =  (struct gre_base_hdr*)pHdr;
  grehdr->flags = 0x00;

  if(proto != ETH_P_TEB) {
    if(isIPv6)
      proto = ETH_P_IPV6;
    else
      proto = ETH_P_IP;
  }

  grehdr->protocol = htons(proto);
  pHdr += sizeof(*grehdr);

  //Note: Sequence numbering is not supported.
  if(t_flags & TUNNEL_CSUM) {
		pHdr += 4;
    *((uint32_t*)(pHdr)) = 0;
    grehdr->flags |= GRE_CSUM;
  }
  if(t_flags & TUNNEL_KEY) {
    *((uint32_t*)(pHdr)) = t_key;
    grehdr->flags |= GRE_KEY;
  }

  *len = hlen;
  return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_form_gre_hdr);

#if 0 //Not used
int ppa_get_gre4_tnl_iph(struct iphdr* iph,
                         struct net_device *dev)
{
  if( dev->type == ARPHRD_IPGRE ||
      (ppa_is_ipv4_gretap_fn && ppa_is_ipv4_gretap_fn(dev)) ) {
    uint16_t hdrlen;
    ppa_get_gre_hdrlen(dev,&hdrlen);
    return ppa_get_ipv4_tnl_iph(iph, dev,hdrlen);
  }

  return PPA_FAILURE;
}
EXPORT_SYMBOL(ppa_get_gre4_tnl_iph);

int ppa_get_gre6_tnl_iph(struct ipv6hdr* iph,
                         struct net_device *dev)
{
  if( dev->type == ARPHRD_IP6GRE ||
      (ppa_is_ipv6_gretap_fn && ppa_is_ipv6_gretap_fn(dev)) ) {
   
    uint16_t hdrlen;
    ppa_get_gre_hdrlen(dev,&hdrlen);
    return ppa_get_ipv6_tnl_iph(iph, dev,hdrlen);
  }

  return PPA_FAILURE;
}
EXPORT_SYMBOL(ppa_get_gre6_tnl_iph);
#endif

/*
 * Returns PPA_SUCCESS if valid destination is available.
 *
 */
int32_t ppa_get_dmac_from_dst_entry( uint8_t* mac, 
                                        PPA_BUF* skb, 
                                        struct dst_entry *dst)
{
  struct neighbour *neigh;
  int ret = PPA_ENOTAVAIL;

  neigh = dst_neigh_lookup_skb(dst,skb);

  if(neigh) {
    ret = ppa_get_mac_from_neigh(mac,neigh,dst);
    neigh_release(neigh);
  }
  return ret;
}

/* 6RD tunnel routines */
#if defined(CONFIG_IPV6_SIT) || defined(CONFIG_IPV6_SIT_MODULE)

int32_t ppa_get_6rd_dst_mac(struct net_device *dev, PPA_BUF *ppa_buf,uint8_t *mac,uint32_t daddr)
{
  struct net_device *phy_dev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,10)
  buf = buf;
#endif

  if(!ppa_get_6rd_phyif_fn)
    return PPA_ENOTAVAIL;

  phy_dev = ppa_get_6rd_phyif_fn(dev);
  if(!phy_dev){
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"cannot get infrastructural device\n");
    return PPA_ENOTAVAIL;
  }
      /* First need to check if PPP output interface */
  if( ppa_check_is_ppp_netif(phy_dev) )
  {
    /* Check if PPPoE interface */
    if ( ppa_check_is_pppoe_netif(phy_dev) )
    {
      return ppa_pppoe_get_dst_mac(phy_dev, mac);
    }
    return PPA_ENOTPOSSIBLE;
  } else if(ppa_get_6rd_dmac_fn != NULL) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,10)
    return ppa_get_6rd_dmac_fn(dev, mac,daddr);
#else
    return ppa_get_6rd_dmac_fn(dev, ppa_buf,mac,daddr);
#endif
  }

  return PPA_ENOTAVAIL;
}

static inline
uint32_t try_6rd_ppa(struct in6_addr *v6dst, struct ip_tunnel *tunnel)
{
  uint32_t dst = 0;

#ifdef CONFIG_IPV6_SIT_6RD
  if(ipv6_prefix_equal(v6dst, 
        &tunnel->ip6rd.prefix, tunnel->ip6rd.prefixlen)) {
  
    unsigned pbw0, pbi0;
    int pbi1;
    u32 d;

    pbw0 = tunnel->ip6rd.prefixlen >> 5;
    pbi0 = tunnel->ip6rd.prefixlen & 0x1f;

    d = (ntohl(v6dst->s6_addr32[pbw0]) << pbi0) >>
        tunnel->ip6rd.relay_prefixlen;

    pbi1 = pbi0 - tunnel->ip6rd.relay_prefixlen;
    if (pbi1 > 0)
            d |= ntohl(v6dst->s6_addr32[pbw0 + 1]) >>
                 (32 - pbi1);

    dst = tunnel->ip6rd.relay_prefix | htonl(d);
  }
#else
  if (v6dst->s6_addr16[0] == htons(0x2002)) {
          /* 6to4 v6 addr has 16 bits prefix, 32 v4addr, 16 SLA, ... */
          memcpy(&dst, &v6dst->s6_addr16[1], 4);
  }
#endif
  return dst;
} 

uint32_t ppa_get_6rdtunel_dst_ip(PPA_BUF *skb, PPA_NETIF *dev)
{
	struct ip_tunnel *tunnel = netdev_priv(dev);
	struct iphdr  *tiph = &tunnel->parms.iph;
	struct ipv6hdr *iph6 = ipv6_hdr(skb);
	__be32 dst = tiph->daddr;
	struct in6_addr *addr6;
	int addr_type;
	
   if (skb->protocol != htons(ETH_P_IPV6))
		goto tx_error;

	if (dev->priv_flags & IFF_ISATAP) {
		struct neighbour *neigh = NULL;
		bool do_tx_error = false;

		if (skb_dst(skb))
			//neigh = skb_dst(skb)->neighbour;
			neigh = dst_neigh_lookup(skb_dst(skb), &iph6->daddr);

		if (neigh == NULL) {
			net_dbg_ratelimited("sit: nexthop == NULL\n");
			goto tx_error;
		}

		addr6 = (struct in6_addr*)&neigh->primary_key;
		addr_type = ipv6_addr_type(addr6);

		if ((addr_type & IPV6_ADDR_UNICAST) &&
		     ipv6_addr_is_isatap(addr6))
			dst = addr6->s6_addr32[3];
		else
			do_tx_error = true;

		neigh_release(neigh);
		if (do_tx_error)
			goto tx_error;
	}

	if (!dst)
		dst = try_6rd_ppa(&iph6->daddr, tunnel);

	if (!dst) {
		struct neighbour *neigh = NULL;

		if (skb_dst(skb))
			//neigh = skb_dst(skb)->neighbour;
			neigh = dst_neigh_lookup(skb_dst(skb), &iph6->daddr);

		if (neigh == NULL) {
			net_dbg_ratelimited("sit: nexthop == NULL\n");
			goto tx_error;
		}

		addr6 = (struct in6_addr*)&neigh->primary_key;
		addr_type = ipv6_addr_type(addr6);

		if (addr_type == IPV6_ADDR_ANY) {
			addr6 = &ipv6_hdr(skb)->daddr;
			addr_type = ipv6_addr_type(addr6);
		}


		if ((addr_type & IPV6_ADDR_COMPATv4) == 0)
			goto tx_error;

		dst = addr6->s6_addr32[3];
	}

tx_error:
	return dst;
}

#else

int32_t ppa_get_6rd_dst_mac(struct net_device *dev, PPA_BUF *ppa_buf,uint8_t *mac,uint32_t daddr)
{
    return PPA_EPERM;
}

uint32_t ppa_get_6rdtunel_dst_ip(PPA_BUF *skb, PPA_NETIF *dev)
{
  return 0;
}
#endif
EXPORT_SYMBOL(ppa_get_6rdtunel_dst_ip);

/* DS-Lite tunnel routines  */
#if defined(CONFIG_IPV6_TUNNEL) || defined(CONFIG_IPV6_TUNNEL_MODULE)

int32_t ppa_get_dslite_dst_mac(struct net_device *dev,PPA_BUF* buf, uint8_t *mac)
{
  struct net_device *phy_dev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,10)
buf = buf; 
#endif
  if(!ppa_get_ip4ip6_phyif_fn)
    return PPA_ENOTAVAIL;

  phy_dev = ppa_get_ip4ip6_phyif_fn(dev);
  if(!phy_dev){
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"cannot get infrastructural device\n");
    return PPA_ENOTAVAIL;
  }
      
  /* First need to check if PPP output interface */
  if ( ppa_check_is_ppp_netif(phy_dev) ) {
    /* Check if PPPoE interface */
    if ( ppa_check_is_pppoe_netif(phy_dev) )
    {
      /* Determine PPPoE MAC address */
      return ppa_pppoe_get_dst_mac(phy_dev, mac);
    }
    return PPA_ENOTPOSSIBLE;
  } else if( ppa_get_ip4ip6_dmac_fn != NULL ) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,10)
    return ppa_get_ip4ip6_dmac_fn(dev, mac);
#else
    return  ppa_get_ip4ip6_dmac_fn(dev,buf, mac);
#endif
   }

   return PPA_ENOTAVAIL;
}

#else

int32_t ppa_get_dslite_dst_mac(struct net_device *dev,PPA_BUF* buf, uint8_t *mac)
{
  return PPA_EPERM;
}

#endif

void ppa_neigh_update_hhs(struct neighbour *neigh)
{
	struct hh_cache *hh;
	void (*update)(struct hh_cache*, const struct net_device*, const unsigned char *)
		= NULL;

	//if (neigh->dev->header_ops && neigh->dev->header_ops->cache_update == NULL)
	update = eth_header_cache_update;

	if (update) {
		hh = &neigh->hh;
     if(hh == NULL)
       return;

		if (hh->hh_len) {
			write_seqlock_bh(&hh->hh_lock);
			update(hh, neigh->dev, neigh->ha);
			write_sequnlock_bh(&hh->hh_lock);
		}
	}
}
EXPORT_SYMBOL(ppa_neigh_update_hhs);

void ppa_neigh_hh_init(struct neighbour *n, struct dst_entry *dst)
{
	//struct net_device *dev = dst->dev;
	__be16 prot = dst->ops->protocol;
	struct hh_cache	*hh = &n->hh;

	write_lock_bh(&n->lock);
  if(hh == NULL)
  {
    write_unlock_bh(&n->lock);
    return;
  }
	/* Only one thread can come in here and initialize the
	 * hh_cache entry.
	 */
	//if (!hh->hh_len && dev->header_ops->cache == NULL) 
	if (!hh->hh_len) 
		eth_header_cache(n, hh, prot);

	write_unlock_bh(&n->lock);
}
EXPORT_SYMBOL(ppa_neigh_hh_init);

