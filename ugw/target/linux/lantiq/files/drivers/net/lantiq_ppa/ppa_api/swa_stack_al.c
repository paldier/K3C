/******************************************************************************
**
** FILE NAME    : swa_stack_al.c
** PROJECT      : PPA
** MODULES      : Software Acceleration Stack Adaption Layer (Linux)
**
** DATE         : 13 MAR 2014
** AUTHOR       : Lantiq Deutschland GmbH
** DESCRIPTION  : Software Acceleration Stack Adaption Layer (Linux)
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
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

#include <linux/kernel.h>
#include <linux/module.h>

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
#include <net/ppa_api.h>
#include <net/ppa_stack_al.h>
#include "ppa_api_session.h"
#include "ppa_api_netif.h"
#include "ppa_api_hal_selector.h"

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
extern int ppa_sw_litepath_local_deliver(struct sk_buff *skb);
#endif

void swa_dump_skb(uint32_t len, void* skb1)
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
EXPORT_SYMBOL(swa_dump_skb);

void *swa_malloc(uint32_t size)
{
    gfp_t flags;

    if ( in_atomic() || in_interrupt() )
        flags = GFP_ATOMIC;
    else
        flags = GFP_KERNEL;
    return kmalloc(size, flags);
}
EXPORT_SYMBOL(swa_malloc);

void *swa_dma(uint32_t size)
{
    gfp_t flags;

    flags = GFP_DMA;
    return kmalloc(size, flags);
}
EXPORT_SYMBOL(swa_dma);

int32_t swa_free(void *buf)
{
    kfree(buf);
    return PPA_SUCCESS;
}
EXPORT_SYMBOL(swa_free);

void swa_memset(void *dst, uint32_t pad, uint32_t n)
{
    memset(dst, pad, n);
}
EXPORT_SYMBOL(swa_memset);

int swa_skb_mark(void *skb)
{
	return ((struct sk_buff *)skb)->mark;
}
EXPORT_SYMBOL(swa_skb_mark);

#ifdef CONFIG_NETWORK_EXTMARK
int  swa_skb_get_extmark(void *skb)
{
        //printk("\nskb mark =0x%x\n", ((struct sk_buff *)skb)->mark);
        //printk("\nskb ext mark =0x%x\n", ((struct sk_buff *)skb)->extmark);
        return ((struct sk_buff *)skb)->extmark;
}
EXPORT_SYMBOL(swa_skb_get_extmark);
#endif

void  swa_debug(void *str,int val)
{
	printk("\n%s = 0x%x\n",(char*)str,val);
}
EXPORT_SYMBOL(swa_debug);

void swa_skb_update_mark(void *skb1,int flg)
{
	struct sk_buff *skb;
	skb = (struct sk_buff *)skb1;
	skb->mark &= ~flg;
}
EXPORT_SYMBOL(swa_skb_update_mark);

#ifdef CONFIG_NETWORK_EXTMARK
void swa_skb_set_extmark_prio(void *skb1,int mark,int extmark,int priority)
{
	struct sk_buff *skb;
	skb = (struct sk_buff *)skb1;
        //printk("\ndo_sw_acceleration mark =0x%x\n", mark);
        //printk("\ndo_sw_acceleration extmark =0x%x\n", extmark);
	//skb->mark = (mark & (~FLG_PPA_PROCESSED));
	skb->mark |= mark;
	skb->extmark = extmark;
	skb->priority = priority;
        //printk("\nAfter do_sw_acceleration mark =0x%x\n", skb->mark);
        //printk("\nAfter do_sw_acceleration extmark =0x%x\n", skb->extmark);
}
EXPORT_SYMBOL(swa_skb_set_extmark_prio);
#endif

#define VLAN_HLEN			4

void swa_skb_update_vlanprio(void *skb1,int vlan_tag)
{
	struct vlan_hdr *vhdr;
	struct sk_buff *skb;
	skb = (struct sk_buff *)skb1;
	
	vhdr = (struct vlan_hdr *) skb_push(skb, VLAN_HLEN);
	vhdr->h_vlan_TCI = htons((u16)vlan_tag);
	skb_pull(skb, VLAN_HLEN);
}

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
unsigned char *swa_skb_data_begin(void *skb)
{
	struct dma_rx_desc_2 *desc_2 = (struct dma_rx_desc_2 *)&((struct sk_buff *)skb)->DW2;
	//struct dma_rx_desc_3 *desc_3 = (struct dma_rx_desc_3 *)&((struct sk_buff *)skb)->DW3;

	return (unsigned char *)(desc_2->field.data_ptr + 2 /* desc_3->field.byte_offset */);
}
EXPORT_SYMBOL(swa_skb_data_begin);
#endif

unsigned char *swa_skb_head(void *skb)
{
	return ((struct sk_buff *)skb)->head;
}
EXPORT_SYMBOL(swa_skb_head);

unsigned char *swa_skb_pull(void *skb,unsigned int len)
{
	return skb_pull((struct sk_buff *)skb,len);
}
EXPORT_SYMBOL(swa_skb_pull);

unsigned char *swa_skb_data(void *skb)
{
	return ((struct sk_buff *)skb)->data;
}
EXPORT_SYMBOL(swa_skb_data);

unsigned char *swa_skb_push(void *skb,unsigned int len)
{
	return skb_push((struct sk_buff *)skb,len);
}
EXPORT_SYMBOL(swa_skb_push);

unsigned int swa_skb_headroom(void *skb)
{
	return skb_headroom((struct sk_buff *)skb);
}
EXPORT_SYMBOL(swa_skb_headroom);

struct sk_buff *swa_skb_realloc_headroom(void *skb, unsigned int len)
{
	return skb_realloc_headroom(((struct sk_buff *)skb),len);
}
EXPORT_SYMBOL(swa_skb_realloc_headroom);

void swa_nf_conntrack_put(void *sess)
{
	struct nf_conn *ip_sess;
	ip_sess = (struct nf_conn *)sess;
	nf_conntrack_put(&ip_sess->ct_general);
}
EXPORT_SYMBOL(swa_nf_conntrack_put);

void SWA_SKB_FREE(void *skb)
{
	PPA_SKB_FREE((struct sk_buff *)skb);
}
EXPORT_SYMBOL(SWA_SKB_FREE);

struct sock *swa_skb_sk(void *skb)
{
	return (((struct sk_buff *)skb)->sk);
}
EXPORT_SYMBOL(swa_skb_sk);

void swa_skb_set_owner_w(void *skb,void *sk)
{
	skb_set_owner_w((struct sk_buff *)skb,(struct sock *)sk);
}
EXPORT_SYMBOL(swa_skb_set_owner_w);

void swa_skb_set_mac_header(void *skb, int offset)
{
	skb_set_mac_header((struct sk_buff *)skb,offset);
}
EXPORT_SYMBOL(swa_skb_set_mac_header);

void swa_skb_set_network_header(void *skb, int offset)
{
	skb_set_network_header((struct sk_buff *)skb,offset);
}
EXPORT_SYMBOL(swa_skb_set_network_header);

unsigned int swa_skb_network_header_len(void *skb)
{ 
        return skb_network_header_len((struct sk_buff *)skb);
} 
EXPORT_SYMBOL(swa_skb_network_header_len);

void swa_skb_set_transport_header(void *skb, int offset)
{
	skb_set_transport_header((struct sk_buff *)skb,offset);
}
EXPORT_SYMBOL(swa_skb_set_transport_header);

unsigned char *swa_skb_network_header(void *skb)
{
	return skb_network_header((struct sk_buff *)skb);
}
EXPORT_SYMBOL(swa_skb_network_header);

__sum16 swa_ip_fast_csum(void *iph, unsigned int len)
{
	return ip_fast_csum((unsigned char *)iph,len);
}
EXPORT_SYMBOL(swa_ip_fast_csum);

unsigned char *swa_skb_transport_header(void *skb)
{
	return skb_transport_header((struct sk_buff *)skb);
}
EXPORT_SYMBOL(swa_skb_transport_header);

struct net_device *swa_get_netif(void *ifinfo)
{
	struct netif_info *tx_ifinfo;
	if(ifinfo) {
	    tx_ifinfo = (struct netif_info *)ifinfo;
	    return ppa_get_netif(tx_ifinfo->phys_netif_name);
	} else { 
	    return NULL;
	}
}
EXPORT_SYMBOL(swa_get_netif);

unsigned int swa_get_flags(void *ifinfo)
{	
	struct netif_info *tx_ifinfo;
	if(ifinfo) {
	    tx_ifinfo = (struct netif_info *)ifinfo;
	    return tx_ifinfo->flags;
	} else {
	    return 0;
	} 
}
EXPORT_SYMBOL(swa_get_flags);

unsigned int swa_get_phy_port(void *ifinfo)
{	
	struct netif_info *tx_ifinfo;
	if(ifinfo) {
	    tx_ifinfo = (struct netif_info *)ifinfo;
	    return tx_ifinfo->phys_port;
	} else {
	    return 0;   //if txifinfo = NULL then the packet is forwarded to cpu.
	}
}
EXPORT_SYMBOL(swa_get_phy_port);

int swa_get_br_dst_port(void *netif, void *skb, void **p_netif)
{
	return ppa_get_br_dst_port((PPA_NETIF *)netif,(struct sk_buff *)skb,(PPA_NETIF **)p_netif);
}
EXPORT_SYMBOL(swa_get_br_dst_port);

void swa_memcpy_data(void *dst, void *src, uint32_t offset)
{
	struct sk_buff *skb;
	skb = (struct sk_buff *)src;
	ppa_memcpy(dst,skb->data,offset);
}
EXPORT_SYMBOL(swa_memcpy_data);

void swa_memcpy(void *dst, void *src, uint32_t offset)
{
	ppa_memcpy(dst,src,offset);
}
EXPORT_SYMBOL(swa_memcpy);

int swa_get_netif_hwaddr(void *tx_if, unsigned char *hdr, uint32_t i)
{
	return ppa_get_netif_hwaddr((struct net_device *)tx_if,hdr,i);
}
EXPORT_SYMBOL(swa_get_netif_hwaddr);

int swa_get_session_from_skb(void *skb, unsigned char pf, void **pp_item)
{
//  ((struct sk_buff *)skb)->network_header = ((struct sk_buff *)skb)->data;
	skb_reset_network_header((struct sk_buff *)skb);
  return ppa_find_session_from_skb((struct sk_buff *)skb,
                                    pf,(struct session_list_item**)pp_item);
}
EXPORT_SYMBOL(swa_get_session_from_skb);

void swa_put_session(void* p_item)
{
  return ppa_session_put((struct session_list_item*)p_item);
}
EXPORT_SYMBOL(swa_put_session);

void swa_skb_set_len(void *skb, unsigned short len)
{
  ((struct sk_buff *)skb)->len = len;
  ((struct sk_buff *)skb)->pkt_type = PACKET_HOST;
}
EXPORT_SYMBOL(swa_skb_set_len);

unsigned int swa_skb_len(void *skb)
{
	struct sk_buff *skb1;
	skb1 = (struct sk_buff *)skb;
	return (skb1->len);
}
EXPORT_SYMBOL(swa_skb_len);

void swa_skb_set_iif(void *skb, void *netif)
{
        struct sk_buff *skb1;
	PPA_NETIF *rx_netif;
        skb1 = (struct sk_buff *)skb;
	rx_netif = (PPA_NETIF *) netif;
	
	skb1->dev = rx_netif;	
	skb1->skb_iif = rx_netif->ifindex;		
}
EXPORT_SYMBOL(swa_skb_set_iif);

void* swa_get_pkt_dev(void *skb)
{
	PPA_BUF *skb1;
        PPA_NETIF *netif;
        skb1 = (PPA_BUF *) skb;
	netif = ppa_get_pkt_src_if(skb1);
	//printk("swa_get_pkt_dev = %s\n", netif->name);
        return (void*) netif;   
}
EXPORT_SYMBOL(swa_get_pkt_dev);

void* swa_get_pkt_dst_if(void *skb)
{
	PPA_BUF *skb1;
	PPA_NETIF *netif;
        skb1 = (PPA_BUF *) skb;
	netif = ppa_get_pkt_dst_if(skb1);
	//printk("swa_get_pkt_dst_if = %s\n", netif->name);
	return (void*) netif;	
}
EXPORT_SYMBOL(swa_get_pkt_dst_if);

void* swa_get_pkt_dst(void *skb, void* dstif)
{
	PPA_BUF *skb1=NULL;
	struct dst_entry *dst1=NULL;
	PPA_NETIF *netif;
	
	skb1 = (PPA_BUF *) skb;
	netif = (PPA_NETIF *) dstif;
	
	if((dst1=skb_dst(skb))!=NULL) {
//printk("skb already have a valid dst\n");
		return (void*) dst1;
	} else {
		int err = ip_route_input_noref(skb1, ip_hdr(skb1)->daddr, ip_hdr(skb1)->saddr,
                                               ip_hdr(skb1)->tos, netif);
                if (unlikely(err)) {
//      			printk("Error: ip_route_input_noref\n");
			return NULL;
		}
		
		if((dst1=skb_dst(skb))!=NULL) {
//printk("skb dst found\n");
                	return (void*) dst1;
		} else {
//printk("skb dst not found\n");
			return NULL;
		}
	}	
}
EXPORT_SYMBOL(swa_get_pkt_dst);

void swa_dst_hold(void *dst)
{
	struct dst_entry *dst1 = (struct dst_entry *)dst;
	dst_hold(dst1);
}
EXPORT_SYMBOL(swa_dst_hold);

void swa_skb_dst_set(void* skb, void* *dst)
{
	struct sk_buff * skb1= (struct sk_buff *)skb;
	struct dst_entry *dst1 = (struct dst_entry *)dst;
	skb_dst_set(skb1,dst1);
}
EXPORT_SYMBOL(swa_skb_dst_set);

int swa_isvalid_dst(void *dst1)
{
	if(dst1) {
		struct dst_entry *dst = (struct dst_entry *)dst1;
		if(dst->obsolete == DST_OBSOLETE_NONE) 
			return 0;
	}
	return 1;
}
EXPORT_SYMBOL(swa_isvalid_dst);

#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
int32_t swa_lro_entry_criteria(void* p_item1, void *ppa_buf)
{	
	struct session_list_item * p_item = (struct session_list_item *)p_item1;
	PPA_BUF *skb = (PPA_BUF *)ppa_buf;
	return ppa_lro_entry_criteria(p_item, skb, 0);
}
EXPORT_SYMBOL(swa_lro_entry_criteria);

int32_t swa_add_lro_entry(void* p_item1)
{
	struct session_list_item * p_item = (struct session_list_item *)p_item1;	
	return ppa_add_lro_entry(p_item, 0);
}
EXPORT_SYMBOL(swa_add_lro_entry);
#endif

void swa_inet_proto_csum_replace4_u(void *hdr, void *skb, __be32 from, __be32 to)
{
	struct udphdr *uhdr;
	uhdr = (struct udphdr *)hdr;
	inet_proto_csum_replace4(&uhdr->check,(struct sk_buff *)skb,from,to,1);
}
EXPORT_SYMBOL(swa_inet_proto_csum_replace4_u);

void swa_skb_dev(void *skb, void *itf)
{
	struct sk_buff *skb1;
	struct net_device *dev;
	skb1 = (struct sk_buff *)skb;
	dev = (struct net_device *)itf;
	skb1->dev = dev;
}
EXPORT_SYMBOL(swa_skb_dev);

void swa_reset_vlantci(void *skb)
{
	struct sk_buff *skb1;
	skb1 = (struct sk_buff *)skb;
	skb1->vlan_tci &= 0x0111;
}
EXPORT_SYMBOL(swa_reset_vlantci);

void swa_inet_proto_csum_replace4_t(void *hdr, void *skb, __be32 from, __be32 to)
{
	struct tcphdr *thdr;
	thdr = (struct tcphdr *)hdr;
	inet_proto_csum_replace4(&thdr->check,(struct sk_buff *)skb,from,to,1);
}
EXPORT_SYMBOL(swa_inet_proto_csum_replace4_t);

void swa_inet_proto_csum_replace2_u(void *hdr, void *skb, __be16 from, __be16 to)
{
	struct udphdr *uhdr;
	uhdr = (struct udphdr *)hdr;
	inet_proto_csum_replace2(&uhdr->check,(struct sk_buff *)skb,from,to,0);
}
EXPORT_SYMBOL(swa_inet_proto_csum_replace2_u);

void swa_inet_proto_csum_replace2_t(void *hdr, void *skb, __be16 from, __be16 to)
{
	struct tcphdr *thdr;
	thdr = (struct tcphdr *)hdr;
	inet_proto_csum_replace2(&thdr->check,(struct sk_buff *)skb,from,to,0);
}
EXPORT_SYMBOL(swa_inet_proto_csum_replace2_t);

int swa_get_time_in_sec(void)
{
	return (jiffies + HZ / 2) / HZ;
}
EXPORT_SYMBOL(swa_get_time_in_sec);

int swa_dev_queue_xmit(void *skb)
{
	return dev_queue_xmit((struct sk_buff *)skb);
}
EXPORT_SYMBOL(swa_dev_queue_xmit);

#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
int sw_litepath_tcp_recv_skb(void *skb)
{
    struct sk_buff *skb1 = (struct sk_buff *)skb;
//swa_dump_skb(128,skb);
//printk("in %s %d\n",__FUNCTION__, __LINE__);
    return ppa_sw_litepath_local_deliver(skb1);

}
EXPORT_SYMBOL(sw_litepath_tcp_recv_skb);

int sw_update_iph(void *skb1, int* offset, unsigned char *pf)
{ 
    struct sk_buff *skb = (struct sk_buff *)skb1;
    struct rtable *rt = NULL;
    struct iphdr *iph = NULL;
//    struct ipv6hdr *ip6h = NULL;
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
EXPORT_SYMBOL(sw_update_iph);

#endif

#ifdef CONFIG_NET_CLS_ACT
extern struct sk_buff *handle_ing(struct sk_buff *skb, struct packet_type **pt_prev,
					 int *ret, struct net_device *orig_dev);
extern int check_ingress(struct sk_buff *skb);
#endif

int32_t swa_check_ingress(void *vskb)
{
#ifdef CONFIG_NET_CLS_ACT
	return check_ingress((struct sk_buff *)vskb);
#else
	return -1;
#endif
}
EXPORT_SYMBOL(swa_check_ingress);

void *swa_handle_ing(void *vskb)
{
	struct sk_buff *skb = (struct sk_buff *)vskb;
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
	return (void *)skb;
}
EXPORT_SYMBOL(swa_handle_ing);

int sw_get_dslite_tunnel_header(void *net_dev, void *p_ipv6hdr)
{
    struct ip6_tnl *t;
    struct net_device *dev =(struct net_device *) net_dev;
    struct ipv6hdr *ip6hdr = (struct ipv6hdr*)p_ipv6hdr;

    if(dev->type != ARPHRD_TUNNEL6 ){
        return -1;
    }
    t = (struct ip6_tnl *)netdev_priv(dev);

    ppa_memset(p_ipv6hdr, 0, sizeof(*ip6hdr));
    ip6hdr->version = 6;
    ip6hdr->hop_limit = t->parms.hop_limit;
    ip6hdr->nexthdr = IPPROTO_IPIP;
    ipv6_addr_copy(&ip6hdr->saddr, &t->parms.laddr);
    ipv6_addr_copy(&ip6hdr->daddr, &t->parms.raddr);

    return 0;
}
EXPORT_SYMBOL(sw_get_dslite_tunnel_header);

int sw_get_6rd_tunnel_header(void *net_dev, void *p_iph)
{
    
  struct ip_tunnel *t;
  struct net_device *dev =(struct net_device *) net_dev;
  struct iphdr *iph = (struct iphdr*)p_iph;

  if(dev->type != ARPHRD_SIT ){
      return -1;
  } 
  
  t = (struct ip_tunnel *)netdev_priv(dev);

  ppa_memset((void *)iph, 0, sizeof(struct iphdr));
  iph->version        = 4;
  iph->protocol       = IPPROTO_IPV6;
  iph->ihl        = 5;
  iph->ttl        = 64;
  iph->saddr      = t->parms.iph.saddr;
  iph->daddr      = 0; /* Don't use tunnel destination address; Later, it is selected based on IPv6 dst address. */

  //iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
  return 0;
}
EXPORT_SYMBOL(sw_get_6rd_tunnel_header);

#ifdef CONFIG_NETWORK_EXTMARK
void swa_set_extmark(void *skb, int extmark) 
{
	((struct sk_buff *)skb)->extmark = extmark;
}
EXPORT_SYMBOL(swa_set_extmark);
#endif

uint32_t swa_is_GreSession(void* p_item)
{
  return ppa_is_GreSession((struct session_list_item*) p_item );
}
EXPORT_SYMBOL(swa_is_GreSession);

int32_t swa_get_gre_hdrlen(struct net_device* dev, uint16_t *hdrlen)
{
	return ppa_get_gre_hdrlen (dev, hdrlen);
}
EXPORT_SYMBOL(swa_get_gre_hdrlen);

int32_t swa_form_gre_hdr(void* dev, uint8_t isIPv6, uint16_t dataLen, uint8_t *pHdr, uint16_t* len)
{
  return ppa_form_gre_hdr( (struct net_device *)dev, isIPv6, dataLen, pHdr, len);
}
EXPORT_SYMBOL(swa_form_gre_hdr);


int32_t swa_pppol2tp_get_src_addr(void* dev, uint32_t *outer_srcip)
{
	return ppa_pppol2tp_get_src_addr((struct net_device *)dev, outer_srcip);
}
EXPORT_SYMBOL(swa_pppol2tp_get_src_addr);

int32_t swa_pppol2tp_get_dst_addr(void  *dev, uint32_t *outer_srcip)
{
	return ppa_pppol2tp_get_dst_addr((struct net_device *)dev, outer_srcip);
}
EXPORT_SYMBOL(swa_pppol2tp_get_dst_addr);


uint32_t swa_is_gre_netif_type(void* dev, uint8_t* isIPv4Gre, uint8_t* isGreTap)
{
	return ppa_is_gre_netif_type((struct net_device *)dev, isIPv4Gre, isGreTap);
}
EXPORT_SYMBOL(swa_is_gre_netif_type);


MODULE_LICENSE("GPL");
