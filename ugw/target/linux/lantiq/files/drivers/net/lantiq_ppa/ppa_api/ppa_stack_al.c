/******************************************************************************
**
** FILE NAME    : ppa_stack_al.c
** PROJECT      : PPA
** MODULES      : PPA Protocol Stack Adaption Layer (Linux)
**
** DATE         : 4 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Adaption Layer (Linux)
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



/*
 * ####################################
 *              Version No.
 * ####################################
 */
#define VER_FAMILY      0x60        //  bit 0: res
                                    //      1: Danube
                                    //      2: Twinpass
                                    //      3: Amazon-SE
                                    //      4: res
                                    //      5: AR9
                                    //      6: GR9
#define VER_DRTYPE      0x10        //  bit 0: Normal Data Path driver
                                    //      1: Indirect-Fast Path driver
                                    //      2: HAL driver
                                    //      3: Hook driver
                                    //      4: Stack/System Adaption Layer driver
                                    //      5: PPA API driver
#define VER_INTERFACE   0x00        //  bit 0: MII 0
                                    //      1: MII 1
                                    //      2: ATM WAN
                                    //      3: PTM WAN
#define VER_ACCMODE     0x00        //  bit 0: Routing
                                    //      1: Bridging
#define VER_MAJOR       0
#define VER_MID         0
#define VER_MINOR       3

/*#define _PPA_KMALLOC_METHOD*/


/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
//#include <linux/autoconf.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/atmdev.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_arp.h>
#include <linux/kthread.h>
#include <net/sock.h>
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
#include <linux/kallsyms.h>
/*Note, don't call any other PPA functions/varaible outside of stack adaption layer, like g_ppa_dbg_enable. The reason is the ppa
   stack layer is the first module loaded into kernel. All other PPA modules depend on it. */

/*
 *  Chip Specific Head File
 */
#include "ppa_stack_al.h"
#include "ppa_api.h"
#include "ppa_api_misc.h"
#include "ppa_stack_tnl_al.h"

#if defined(CONFIG_NF_CONNTRACK_SUPPORT) || defined(CONFIG_NF_CONNTRACK)
  #define ppa_conntrack_get             nf_ct_get
#else
  #define ppa_conntrack_get             ip_conntrack_get
#endif


/*
 * ####################################
 *              Definition
 * ####################################
 */

#if defined(DISABLE_INLINE) && DISABLE_INLINE
  #define INLINE
#else
  #define INLINE                                inline
#endif


#define ENABLE_MY_MEMCPY                        0

#define MULTICAST_IP_START                      0xE0000000  /*224.0.0.0 */
#define MULTICAST_IP_END                        0xEFFFFFFF  /*239.255.255.255*/


#if LINUX_VERSION_CODE <  KERNEL_VERSION(2,6,36)
#define VLAN_DEV_INFO vlan_dev_info
#else
#define VLAN_DEV_INFO vlan_dev_priv
#endif
/* MAC_FMT starts to defined in 2.6.24*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"


#define MAC_ARG(x) ((u8 *)(x))[0], ((u8 *)(x))[1], ((u8 *)(x))[2], \
                    ((u8 *)(x))[3], ((u8 *)(x))[4], ((u8 *)(x))[5]

/*
 * ####################################
 *              Data Type
 * ####################################
 */



/*
 * ####################################
 *             Declaration
 * ####################################
 */

extern uint32_t __div64_32(uint64_t *n, uint32_t base);
uint32_t g_ppa_dbg_enable = DBG_ENABLE_MASK_ERR;
uint32_t max_print_num=~0;


/*
 * ####################################
 *           Global Variable
 * ####################################
 */

static DEFINE_SPINLOCK(g_local_irq_save_flag_lock);



/*
 * ####################################
 *           Global Variable
 * ####################################
 */



/*
 * ####################################
 *           Extern Variable
 * ####################################
 */



/*
 * ####################################
 *            Local Function
 * ####################################
 */
static INLINE struct dst_entry *ppa_dst(const PPA_BUF *buf);
static INLINE struct net_device *ppa_dev_get_by_name(const PPA_IFNAME *ifname);
static INLINE int ppa_dev_is_br(PPA_NETIF *netif);
static uint8_t *ppa_get_transport_header(const PPA_BUF *ppa_buf);

/*
 * ####################################
 *           Global Function
 * ####################################
 */

void ppa_get_stack_al_id(uint32_t *p_family,
                         uint32_t *p_type,
                         uint32_t *p_if,
                         uint32_t *p_mode,
                         uint32_t *p_major,
                         uint32_t *p_mid,
                         uint32_t *p_minor)
{
    if ( p_family )
        *p_family = VER_FAMILY;

    if ( p_type )
        *p_type = VER_DRTYPE;

    if ( p_if )
        *p_if = VER_INTERFACE;

    if ( p_mode )
        *p_mode = VER_ACCMODE;

    if ( p_major )
        *p_major = VER_MAJOR;

    if ( p_mid )
        *p_mid = VER_MID;

    if ( p_minor )
        *p_minor = VER_MINOR;
}

PPA_SESSION *ppa_get_session(PPA_BUF *ppa_buf)
{
    enum ip_conntrack_info ctinfo;

    return ppa_conntrack_get(ppa_buf, &ctinfo);
}

/*
    port functions from 2.6.32, make it easier to port to 2.6.32
    skb coresponding functions supported by linux kernel from version 2.6.22
    ipv4_is_multicast supported by linux kernel from version 2.6.25
*/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
uint8_t *skb_network_header(const PPA_BUF *ppa_buf)
{
  return ppa_buf->nh.raw;
}

uint8_t *skb_mac_header(const PPA_BUF *ppa_buf)
{
  return ppa_buf->mac.raw;
}

struct ipv6hdr *ipv6_hdr(const PPA_BUF *ppa_buf)
{
  return (struct ipv6hdr *)skb_network_header(ppa_buf);
}

struct iphdr *ip_hdr(const PPA_BUF *ppa_buf)
{
  return (struct iphdr *)skb_network_header(ppa_buf);
}

uint32_t ipv4_is_multicast(uint32_t addr)
{
  return (addr & htonl(0xf0000000)) == htonl(0xe0000000);
}

#endif

static uint8_t *ppa_get_transport_header(const PPA_BUF *ppa_buf)
{

    struct iphdr *iph = NULL;
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){//ipv6 frame with extention header is already filtered
        return (skb_network_header(ppa_buf) + sizeof(struct ipv6hdr));
    }
#endif

    iph = ip_hdr(ppa_buf);
    return (skb_network_header(ppa_buf) + iph->ihl*4);
}


/* Support IPV6 fuctions */
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))

uint8_t ppa_get_ipv6_l4_proto(PPA_BUF *ppa_buf)
{
    uint8_t nexthdr;
    struct ipv6hdr *ip6hdr;
    //int offset;

    ip6hdr = ipv6_hdr(ppa_buf);
    nexthdr = ip6hdr->nexthdr;

/* only check the next header for normal IPv6 packet for PPE firmware doesn't support multiple IPv6 header
    if (ipv6_ext_hdr(nexthdr)) {
        // now skip over extension headers 
        offset = ipv6_skip_exthdr(ppa_buf, sizeof(struct ipv6hdr), &nexthdr);
        if (offset < 0)
            return 0;
    }
*/

    return nexthdr;
}

uint8_t ppa_get_ipv6_tos(PPA_BUF *ppa_buf)
{     
    return (( ntohl(*(uint32_t *)ipv6_hdr(ppa_buf)) >> 20 ) & 0xFF);
    
}

PPA_IPADDR ppa_get_ipv6_saddr(PPA_BUF *ppa_buf)
{
    struct ipv6hdr *ip6hdr = ipv6_hdr(ppa_buf);
    return *(PPA_IPADDR *) &ip6hdr->saddr;
}

PPA_IPADDR ppa_get_ipv6_daddr(PPA_BUF *ppa_buf)
{
    struct ipv6hdr *ip6hdr = ipv6_hdr(ppa_buf);
    return *(PPA_IPADDR *) &ip6hdr->daddr;
}

int32_t ppa_is_ipv6_multicast(PPA_BUF *ppa_buf)
{
    struct ipv6hdr *ip6hdr = ipv6_hdr(ppa_buf);
    return ipv6_addr_is_multicast(&ip6hdr->daddr);
}

uint32_t ppa_is_ipv6_fragment(PPA_BUF *ppa_buf)
{
    uint32_t nhoff, nexthdr;

    nhoff = IP6CB(ppa_buf)->nhoff;
    nexthdr = skb_network_header(ppa_buf)[nhoff];

    return (nexthdr == IPPROTO_FRAGMENT);
}


#endif /* Support IPV6 fuctions */

/*
we need this function to test if it's ipv6 pacekt or not
even if we don't support ipv6 acceleration
*/
uint8_t ppa_is_pkt_ipv6(const PPA_BUF *ppa_buf)
{
    struct ipv6hdr *hdr = ipv6_hdr(ppa_buf);
    return (hdr->version == 6);
}


/*functions special for ipv4 */
uint8_t ppa_get_ip_l4_proto(PPA_BUF *ppa_buf)
{
    struct iphdr *hdr = ip_hdr(ppa_buf);
    return hdr->protocol;
}

uint8_t ppa_get_ip_tos(PPA_BUF *ppa_buf)
{
    struct iphdr *hdr = ip_hdr(ppa_buf);
    return hdr->tos;
}

PPA_IPADDR ppa_get_ip_saddr(PPA_BUF *ppa_buf)
{
    struct iphdr *hdr = ip_hdr(ppa_buf);
    return *(PPA_IPADDR *) &hdr->saddr;
}

PPA_IPADDR ppa_get_ip_daddr(PPA_BUF *ppa_buf)
{
    struct iphdr *hdr = ip_hdr(ppa_buf);
    return *(PPA_IPADDR *) &hdr->daddr;
}

int32_t ppa_is_ip_multicast(PPA_BUF *ppa_buf)
{
    struct iphdr *hdr = ip_hdr(ppa_buf);
    return ipv4_is_multicast(hdr->daddr);
}

uint32_t ppa_is_ip_fragment(PPA_BUF *ppa_buf)
{
    struct iphdr *hdr = ip_hdr(ppa_buf);
    return (hdr->frag_off & htons(IP_MF|IP_OFFSET)) == 0 ? 0 : 1;
}
/*=====function above are special for ipv4 =====*/



uint8_t ppa_get_pkt_ip_proto(PPA_BUF *ppa_buf)
{

#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return ppa_get_ipv6_l4_proto(ppa_buf);
    }
#endif

    return ppa_get_ip_l4_proto(ppa_buf);
}

uint8_t ppa_get_pkt_ip_tos(PPA_BUF *ppa_buf)
{

#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return ppa_get_ipv6_tos(ppa_buf);
    }
#endif

    return ppa_get_ip_tos(ppa_buf);
}

PPA_IPADDR ppa_get_pkt_src_ip(PPA_BUF *ppa_buf)
{

#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return ppa_get_ipv6_saddr(ppa_buf);
    }
#endif

    return ppa_get_ip_saddr(ppa_buf);
}

uint32_t ppa_get_pkt_ip_len(PPA_BUF *ppa_buf)
{

#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return sizeof(struct in6_addr);
    }
#endif

    return sizeof(uint32_t);
}

PPA_IPADDR ppa_get_pkt_dst_ip(PPA_BUF *ppa_buf)
{
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return ppa_get_ipv6_daddr(ppa_buf);
    }
#endif

    return ppa_get_ip_daddr(ppa_buf);
}

int8_t *ppa_get_pkt_ip_string(PPA_IPADDR ppa_ip, uint32_t flag, int8_t *strbuf)
{
    if(!strbuf)
        return strbuf;

    strbuf[0] = 0;
    if(flag){
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE)
        ppa_sprintf(strbuf, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", NIP6(ppa_ip.ip6));
#endif

    }else{
        ppa_sprintf(strbuf, NIPQUAD_FMT, NIPQUAD(ppa_ip.ip));
    }

    return strbuf;
}

int8_t *ppa_get_pkt_mac_string(uint8_t *mac, int8_t *strbuf)
{
    if(!strbuf)
        return strbuf;

    ppa_sprintf(strbuf, MAC_FMT, MAC_ARG(mac));

    return strbuf;
}

int32_t ppa_is_pkt_multicast(PPA_BUF *ppa_buf)
{
    if ( ppa_buf->pkt_type == PACKET_MULTICAST )
        return 1;

#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return ppa_is_ipv6_multicast(ppa_buf);
    }
#endif

    return ppa_is_ip_multicast(ppa_buf);
}

uint32_t ppa_is_pkt_fragment(PPA_BUF *ppa_buf)
{
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return ppa_is_ipv6_fragment(ppa_buf);
    }
#endif

    return ppa_is_ip_fragment(ppa_buf);

}

int32_t ppa_is_pkt_routing(PPA_BUF *ppa_buf)
{
    struct dst_entry *dst = ppa_dst(ppa_buf);
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return dst != NULL && (uint32_t)(dst->input) == (uint32_t)ip6_forward ? 1: 0;
    }
#endif

    return dst != NULL && (uint32_t)(dst->input) == (uint32_t)ip_forward ? 1 : 0;
}

int32_t ppa_is_pkt_mc_routing(PPA_BUF *ppa_buf)
{
    struct dst_entry *dst = ppa_dst(ppa_buf);
#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))
    if(ppa_is_pkt_ipv6(ppa_buf)){
        return 0;//dst != NULL && (uint32_t)(dst->input) == (uint32_t)ip6_mc_input ? 1 : 0;
    }
#endif

    return dst != NULL && (uint32_t)(dst->output) == (uint32_t)ip_mc_output ? 1 : 0;
}

uint16_t ppa_get_pkt_src_port(PPA_BUF *ppa_buf)
{
    return (uint16_t)(((struct udphdr *)ppa_get_transport_header(ppa_buf))->source);
}

uint16_t ppa_get_pkt_dst_port(PPA_BUF *ppa_buf)
{
    return (uint16_t)(((struct udphdr *)ppa_get_transport_header(ppa_buf))->dest);
}

void ppa_get_pkt_rx_src_mac_addr(PPA_BUF *ppa_buf, uint8_t mac[PPA_ETH_ALEN])
{
#ifdef CONFIG_MIPS
    if ( (uint32_t)skb_mac_header(ppa_buf) >= KSEG0 )
#endif
        ppa_memcpy(mac, skb_mac_header(ppa_buf) + PPA_ETH_ALEN, PPA_ETH_ALEN);
}

void ppa_get_pkt_rx_dst_mac_addr(PPA_BUF *ppa_buf, uint8_t mac[PPA_ETH_ALEN])
{
#ifdef CONFIG_MIPS
    if ( (uint32_t)skb_mac_header(ppa_buf) >= KSEG0 )
#endif
        ppa_memcpy(mac, skb_mac_header(ppa_buf), PPA_ETH_ALEN);
}

/*
 *  If it is multicast packet, then return multicast dst & src ip address and success, otherwise return failure(-1).
 *  Note, this function will be called at bridge level and ip stack level.
 *  i.e. skb->data point to mac header or ip header
 */
int ppa_get_multicast_pkt_ip(PPA_BUF *ppa_buf, void *dst_ip, void *src_ip)
{
    /*  note, here ppa_buf may be L2 level, or L3 level. So we have to move pointer to get its real ip  */
    uint16_t protocol = ppa_buf->protocol;
    uint8_t *p;
    IP_ADDR_C *dstip,*srcip;

    p = (uint8_t *) ppa_buf->data;

    if(!dst_ip || !src_ip){
        return PPA_FAILURE;
    }

    dstip = (IP_ADDR_C *)dst_ip;
    srcip = (IP_ADDR_C *)src_ip;
    
    if( p[0] == 0x01 ) // we regard it a l2 level packet if first byte is 0x01 ( multicast mac address, otherwise we think it is a ip packet.
    {
#if defined(PPA_STACK_ENABLE_DEBUG_PRINT) && PPA_STACK_ENABLE_DEBUG_PRINT
        {
            int i;
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_get_pkt_src_ip2(protocol=%x)-1:", ppa_buf->protocol);
            for ( i = 0; i < 30; i++ )
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%02x ", ppa_buf->data[i]);
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n");
         }
#endif

        p += ETH_HLEN -2;  // p point to protocol
        protocol = ( p[0] << 8 ) + p[1];
#if defined(PPA_STACK_ENABLE_DEBUG_PRINT) && PPA_STACK_ENABLE_DEBUG_PRINT
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"protocol=%04x\n", protocol );
#endif

        while ( protocol == ETH_P_8021Q )
        {
            /*  move p to next protocol */
            p += 4 ;  //8021q have

            protocol = ( p[0] << 8 ) + p[1];
#if defined(PPA_STACK_ENABLE_DEBUG_PRINT) && PPA_STACK_ENABLE_DEBUG_PRINT
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"protocol=%04x\n", protocol);
#endif
        }

#if defined(PPA_STACK_ENABLE_DEBUG_PRINT) && PPA_STACK_ENABLE_DEBUG_PRINT
        {
            int i;

            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"p index=%d\n", (char *)p - (char *)ppa_buf->data);

            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_get_pkt_src_ip2(protocol=%x )-2:", ppa_buf->protocol);
            for ( i = 0; i < 30; i++ )
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%02x ", ppa_buf->data[i]);
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"\n");
        }
#endif
    }

    if( protocol == ETH_P_IP )
    {
        struct iphdr *iph = (struct iphdr *)p;

        if ( iph->daddr >= MULTICAST_IP_START && iph->daddr<= MULTICAST_IP_END ){
            
            dstip->f_ipv6 = srcip->f_ipv6 = 0;
            dstip->ip.ip = iph->daddr;
            srcip->ip.ip = iph->saddr;
            
            return PPA_SUCCESS;
        }
        return PPA_FAILURE;
    }
    else if( protocol == ETH_P_IPV6 )
    {
        struct ipv6hdr *ip6hdr = (struct ipv6hdr *)p;

        if(ipv6_addr_is_multicast(&ip6hdr->daddr)){
            
            dstip->f_ipv6 = srcip->f_ipv6 = 1;
            ppa_memcpy(dstip->ip.ip6, &ip6hdr->daddr, sizeof(dstip->ip.ip6));
            ppa_memcpy(srcip->ip.ip6, &ip6hdr->saddr, sizeof(srcip->ip.ip6));
            
            return PPA_SUCCESS;
        }
        return PPA_FAILURE;
    }

    return PPA_FAILURE;
}

PPA_NETIF *ppa_get_pkt_src_if(PPA_BUF *ppa_buf)
{
    return ppa_buf->dev;
}

PPA_NETIF *ppa_get_pkt_dst_if(PPA_BUF *ppa_buf)
{
    struct dst_entry *dst = ppa_dst(ppa_buf);
    if(dst != NULL)
        return dst->dev;
    else {
        //return NULL;
      return ppa_buf->dev; //For bridged session
    }
}

uint32_t ppa_get_pkt_priority(PPA_BUF *ppa_buf)
{
  if( ppa_buf ) return ppa_buf->priority;
    
  return 0;
}

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
uint32_t ppa_get_session_priority(PPA_BUF *ppa_buf)
{
  if( ppa_buf ) return skb_get_session_priority(ppa_buf);
    
  return 0;
}

uint32_t ppa_get_low_prio_thresh(uint32_t flags)
{
	return nf_conntrack_low_prio_thresh;
}

struct timespec ppa_timespec_sub(struct timespec lhs, struct timespec rhs)
{
	return timespec_sub(lhs,rhs);
}

s64 ppa_timespec_to_ns(struct timespec *lhs)
{
	return timespec_to_ns(lhs);
}
void ppa_get_monotonic( struct timespec *lhs)
{
	getrawmonotonic(lhs);
}

uint32_t ppa_get_def_prio_thresh(uint32_t flags)
{
	return nf_conntrack_default_prio_thresh;
}

uint32_t ppa_get_low_prio_data_rate(uint32_t flags)
{
	return nf_conntrack_low_prio_data_rate;
}

uint32_t ppa_get_def_prio_data_rate(uint32_t flags)
{
	return nf_conntrack_default_prio_data_rate;
}

uint32_t ppa_get_session_limit_enable(uint32_t flags)
{
	return nf_conntrack_session_limit_enable;
}
uint32_t ppa_get_tcp_initial_offset(uint32_t flags)
{
	return nf_conntrack_tcp_initial_offset;
}
uint32_t ppa_get_tcp_steady_offset(uint32_t flags)
{
	return nf_conntrack_tcp_steady_offset;
}

#endif

uint32_t ppa_get_skb_mark(PPA_BUF *ppa_buf)
{
    if( ppa_buf ) return ppa_buf->mark;
    else return 0;
}

#ifdef CONFIG_NETWORK_EXTMARK
uint32_t ppa_get_skb_extmark(PPA_BUF *ppa_buf)
{
    if( ppa_buf ) return ppa_buf->extmark;
    else return 0;
}
#endif

uint32_t ppa_set_pkt_priority(PPA_BUF *ppa_buf, uint32_t new_pri)
{
    if( ppa_buf )
    {
        ppa_buf->priority = new_pri;
        return ppa_buf->priority;
    }
    else
        return 0;
}

#if (!defined(CONFIG_PPPOE) && !defined(CONFIG_PPPOE_MODULE)) || (!defined(CONFIG_LTQ_PPA_API) && !defined(CONFIG_LTQ_PPA_API_MODULE))
int32_t ppa_pppoe_get_pppoe_addr(PPA_NETIF *netif, struct pppoe_addr *pa)
{
    return PPA_EPERM;
}

__u16 ppa_pppoe_get_pppoe_session_id(PPA_NETIF *netif)
{
    return 0;
}

__u16 ppa_get_pkt_pppoe_session_id(PPA_BUF *ppa_buf)
{
    return 0;
}

int32_t ppa_pppoe_get_eth_netif(PPA_NETIF *netif, PPA_IFNAME pppoe_eth_ifname[PPA_IF_NAME_SIZE])
{
    return PPA_EPERM;
}
#endif

#if (!defined(CONFIG_PPPOL2TP) && !defined(CONFIG_PPPOL2TP_MODULE)) || (!defined(CONFIG_LTQ_PPA_API) && !defined(CONFIG_LTQ_PPA_API_MODULE))
__u16 ppa_pppol2tp_get_l2tp_session_id(struct net_device *dev)
{
    return 0;
}
__u16 ppa_pppol2tp_get_l2tp_tunnel_id(struct net_device *dev)
{
    return 0;
}
int32_t ppa_pppol2tp_get_base_netif(PPA_NETIF *netif, PPA_IFNAME pppol2tp_eth_ifname[PPA_IF_NAME_SIZE])
{
    return PPA_EPERM;
}
int32_t ppa_pppol2tp_get_l2tp_addr(struct net_device *dev, struct pppol2tp_addr *pa)
{
    return PPA_EPERM;
}
int32_t ppa_pppol2tp_get_l2tp_dmac(struct net_device *dev, uint8_t *mac)
{
    return PPA_EPERM;
}
int32_t ppa_pppol2tp_get_src_addr(struct net_device *dev, uint32_t *outer_srcip)
{
    return PPA_EPERM;
}
int32_t ppa_pppol2tp_get_dst_addr(struct net_device *dev, uint32_t *outer_dstip)
{
    return PPA_EPERM;
}
#endif



#if (defined(CONFIG_PPPOE) || defined(CONFIG_PPPOE_MODULE))

uint32_t ppa_check_is_ppp_netif(PPA_NETIF *netif)
{
  return (netif->type == ARPHRD_PPP && (netif->flags & IFF_POINTOPOINT) );
}

uint32_t ppa_check_is_pppoe_netif(PPA_NETIF *netif)
{
  struct pppoe_addr pa;

  if(!(netif->type == ARPHRD_PPP && (netif->flags & IFF_POINTOPOINT) ))
    return 0;
  return ppa_pppoe_get_pppoe_addr(netif, &pa) == PPA_SUCCESS ? 1 : 0;
}

int32_t ppa_pppoe_get_dst_mac(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN])
{
    int32_t ret;
    struct pppoe_addr pa;

    if ( (ret = ppa_pppoe_get_pppoe_addr(netif, &pa)) != PPA_SUCCESS )
    {
        printk("fail in getting pppoe addr\n");
        return ret;
    }

    ppa_memcpy(mac, pa.remote, sizeof(pa.remote));
    return PPA_SUCCESS;
}

int32_t ppa_pppoe_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE])
{
  if ( !netif )
      netif = ppa_get_netif(ifname);

  if ( !netif )
      return PPA_EINVAL;

  if ( !ppa_check_is_pppoe_netif(netif) )
      return PPA_EINVAL;

  return ppa_pppoe_get_eth_netif(netif, phy_ifname);
}

#else

uint32_t ppa_check_is_pppoe_netif(PPA_NETIF *netif)
{
  return 0;
}
int32_t ppa_pppoe_get_dst_mac(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN])
{
  return PPA_EPERM;
}
int32_t ppa_pppoe_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE])
{
    return PPA_EPERM;
}

uint32_t ppa_check_is_ppp_netif(PPA_NETIF *netif)
{
  return 0;
}

#endif


#if defined(CONFIG_PPPOL2TP) || defined(CONFIG_PPPOL2TP_MODULE)

int32_t ppa_pppol2tp_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE])
{
  return ppa_pppol2tp_get_base_netif(netif, phy_ifname) == PPA_SUCCESS ? 0 : 1;
}

uint32_t ppa_check_is_pppol2tp_netif(PPA_NETIF *netif)
{
  struct pppol2tp_addr pa;

#if 1
  if(netif->type == ARPHRD_PPP && (netif->flags & IFF_POINTOPOINT) ) {

    return ppa_pppol2tp_get_l2tp_addr(netif, &pa) == PPA_SUCCESS ? 1 : 0;
  }
  return 0;
#else
  if ( !(netif->flags & IFF_POINTOPOINT) ) {
    return 0;
  }
  
  if(netif->type == ARPHRD_SIT || 
      netif->type == ARPHRD_TUNNEL6 ||
      ppa_is_gre_netif(netif)) {
    //6rd device or ip6 tunnel
    return 0;
  }
  return ppa_pppol2tp_get_l2tp_addr(netif, &pa) == PPA_SUCCESS ? 1 : 0;
#endif
}

uint32_t ppa_check_is_lro_enabled_netif(PPA_NETIF *netif, uint8_t src_mac[PPA_ETH_ALEN])
{
    PPA_NETIF *p_netif = netif;

    if(!netif)
	return PPA_FAILURE;

    if(ppa_dev_is_br(netif)) {
    	if ( (ppa_get_br_dst_port_with_mac(netif, src_mac, &p_netif)) != PPA_SUCCESS )
	    return PPA_FAILURE;	
    }
     
    if(p_netif->features & NETIF_F_LRO) {
	return PPA_SUCCESS;
    }

// printk(KERN_INFO"netif = %s is not lro enabled\n", p_netif->name);

    return PPA_FAILURE;
}

int32_t ppa_pppol2tp_get_dst_mac(PPA_NETIF *netif, uint8_t *mac)
{
    int32_t ret;
    struct pppol2tp_addr pa;

    if ( (ret = ppa_pppol2tp_get_l2tp_addr(netif, &pa)) != PPA_SUCCESS )
    {
        printk("fail in getting pppol2tp addr\n");
        return ret;
    }
    if( (ret = ppa_pppol2tp_get_l2tp_dmac(netif,mac)) != PPA_SUCCESS )
    {
        printk("fail in getting pppol2tp dmac\n");
        return ret;
    }
    return PPA_SUCCESS;
}

#else

int32_t ppa_pppol2tp_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE])
{
    return PPA_EPERM;
}

uint32_t ppa_check_is_pppol2tp_netif(PPA_NETIF *netif)
{
  return 0;
}

int32_t ppa_pppol2tp_get_dst_mac(PPA_NETIF *netif, uint8_t *mac)
{
  return PPA_EPERM;
}

#endif

int32_t ppa_get_dst_mac(PPA_BUF *ppa_buf, 
                        PPA_SESSION *p_session, 
                        uint8_t mac[PPA_ETH_ALEN],
                        uint32_t daddr)
{
  int32_t ret = PPA_ENOTAVAIL;
  struct dst_entry *dst;
  struct net_device *netif;

  /*
   *  Assumption, function only gets called from POSTROUTING so skb->dev = o/p i/f
   */
  netif = ppa_buf->dev;

  if(netif->type == ARPHRD_SIT){
    return ppa_get_6rd_dst_mac(netif,ppa_buf, mac,daddr);
  }

  if(ppa_buf->dev->type == ARPHRD_TUNNEL6) {
    return ppa_get_dslite_dst_mac(netif,ppa_buf, mac);
  }

  if(ppa_is_gre_netif(netif)) {
    return ppa_get_gre_dmac(mac,netif,ppa_buf); 
  }

  /* First need to check if PPP output interface */
  if ( ppa_check_is_ppp_netif(netif) ) {

    /*
     * If interface is neither PPPoE or L2TP, return failure(not possible).
     */
back_to_pppoe:
    /* Check if PPPoE interface */
    if ( ppa_check_is_pppoe_netif(netif) )
    {
      /* Determine PPPoE MAC address */
      return ppa_pppoe_get_dst_mac(netif, mac);
    }

    /* Check if PPPoL2TP interface */
    if ( ppa_check_is_pppol2tp_netif(netif) )
    {
      char name[PPA_IF_NAME_SIZE];
      if ( ppa_pppol2tp_get_base_netif(netif, name) || !(netif = ppa_get_netif(name)) )
      {
          ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s: Cannot get PPPOL2TP ppa netif %s address!",
                        __FUNCTION__,
                        ((netif) ? netif->name : "NULL"));
          return PPA_ENOTAVAIL;
      }
        
      if(ppa_check_is_ppp_netif(netif))
        goto back_to_pppoe;
        
      /* Determine PPPoE MAC address */
      return ppa_pppol2tp_get_dst_mac(ppa_buf->dev, mac);
    } 
    return PPA_ENOTPOSSIBLE;
  }

  dst = ppa_dst(ppa_buf);
  if ( !dst )
  {
    //printk("dest entry is not ready: ppa_buf->dst == NULL\n");
    return PPA_ENOTAVAIL; //  Dst MAC address not known
  }
 
  ret = ppa_get_dmac_from_dst_entry(mac,ppa_buf,dst);
  return ret;
}

PPA_NETIF *ppa_get_netif(PPA_IFNAME *ifname)
{
    //  This implementation has risk that the device is destroyed after we get and free it

    PPA_NETIF *netif;

    if ( ifname && (netif = ppa_dev_get_by_name(ifname)) )
    {
        dev_put(netif);
        return netif;
    }
    else
        return NULL;
}

void ppa_put_netif(PPA_NETIF *netif)
{
    dev_put(netif);
}

PPA_NETIF *ppa_get_br_dev(PPA_NETIF *netif)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    if(netif && netif->br_port){
        return netif->br_port->br->dev;
    }
#else
    if(netif && br_port_exists(netif))
            return(br_port_get_rcu(netif)->br->dev);

#endif

    return NULL;
}

int32_t ppa_get_netif_hwaddr(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN], uint32_t flag_down_only)
{

  PPA_NETIF* orgif;

  orgif = netif; //Needed to print debug info
  ppa_memset(mac, 0, PPA_ETH_ALEN);

hwaddr_top: 

  if( ppa_check_is_ppp_netif(netif) ) {
  //if( ppa_check_is_pppoe_netif(netif) ) {

    char name[PPA_IF_NAME_SIZE];

    if ( ppa_check_is_pppol2tp_netif(netif) ) {

      if ( ppa_pppol2tp_get_base_netif(netif, name) || 
                                      !(netif = ppa_get_netif(name)) ) {
        goto hwaddr_error;
      }
    }
      
    if ( ppa_check_is_pppoe_netif(netif) ) {

      if ( ppa_pppoe_get_eth_netif(netif, name) || !(netif = ppa_get_netif(name)) ) {
        goto hwaddr_error;
      }
    }
  }
  else if ( netif->type == ARPHRD_SIT ) {
    if ( ppa_get_6rd_phyif_fn == NULL || 
        (netif = ppa_get_6rd_phyif_fn(netif)) == NULL )
      goto hwaddr_error;
    goto hwaddr_top; //Alternative: call ppa_get_netif_hwaddr 
  }
  else if ( netif->type == ARPHRD_TUNNEL6) {
    if ( ppa_get_ip4ip6_phyif_fn == NULL || 
        (netif = ppa_get_ip4ip6_phyif_fn(netif)) == NULL )
      goto hwaddr_error;
    
    goto hwaddr_top; //Alternative: ppa_get_netif_hwaddr
  } else if( ppa_is_gre_netif(netif) ) {
    if( (netif = ppa_get_gre_phyif(netif)) == NULL) 
      goto hwaddr_error;
    goto hwaddr_top; //Alternative: call ppa_get_netif_hwaddr
  }

  if ( netif->type == ARPHRD_ETHER || netif->type == ARPHRD_EETHER )
  {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    if ( netif->br_port == NULL || flag_down_only )
        ppa_memcpy(mac, netif->dev_addr, PPA_ETH_ALEN);
    else
        ppa_memcpy(mac, netif->br_port->br->dev->dev_addr, PPA_ETH_ALEN);
#else
    if(  !(br_port_exists(netif))  || flag_down_only )
        ppa_memcpy(mac, netif->dev_addr, PPA_ETH_ALEN);
    else
        ppa_memcpy(mac, br_port_get_rcu(netif)->br->dev->dev_addr, PPA_ETH_ALEN);
#endif
    return PPA_SUCCESS;
  }

hwaddr_error:
  ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
            "Cannot get hw-mac for %s dev!", orgif->name);
  return PPA_ENOTAVAIL;
}

PPA_IFNAME *ppa_get_netif_name(PPA_NETIF *netif)
{
    return netif ? netif->name : NULL;
}

uint32_t ppa_is_netif_equal(PPA_NETIF *netif1, PPA_NETIF *netif2)
{
    return netif1 && netif2 && netif1->ifindex == netif2->ifindex ? 1 : 0;
}

uint32_t ppa_is_netif_name(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
    return netif && ifname && strcmp(netif->name, ifname) == 0 ? 1 : 0;
}

//uint32_t ppa_is_netif_name_prefix(PPA_NETIF *netif, PPA_IFNAME *ifname_prefix, int n)
//{
//    return netif && strncmp(netif->name, ifname_prefix, n) == 0 ? 1 : 0;
//}




int32_t ppa_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE])
{
         
  uint32_t ret = 0;
  if ( !netif )
    netif = ppa_get_netif(ifname);

  if ( !netif )
    return PPA_EINVAL;

  if( ppa_check_is_ppp_netif(netif) ) {

    if(ppa_if_is_pppoa(netif, NULL)) {
  	strcpy(phy_ifname, netif->name);
	ret = PPA_SUCCESS;
	goto ppp_ret;
    }
    else if ( ppa_check_is_pppol2tp_netif(netif) ) {

      ret = ppa_pppol2tp_get_base_netif(netif, phy_ifname);
      if ( ret == PPA_SUCCESS )
        ret = ppa_get_physical_if(NULL,phy_ifname, phy_ifname);
    } else if ( ppa_check_is_pppoe_netif(netif) ) {

      ret = ppa_pppoe_get_eth_netif(netif, phy_ifname);
      if ( ret == PPA_SUCCESS )
        ret = ppa_get_physical_if(NULL, phy_ifname, phy_ifname);
    }
ppp_ret:        
    return ret;
  }

  if( ppa_is_gre_netif(netif) ) {
    if((netif=ppa_get_gre_phyif(netif)))
      return ppa_get_physical_if(netif, NULL, phy_ifname);
    return PPA_FAILURE;
  }

  if ( (netif->priv_flags & IFF_802_1Q_VLAN) ) {
    return ppa_get_physical_if(NULL, 
                               VLAN_DEV_INFO(netif)->real_dev->name, 
                               phy_ifname);
  }

#if defined(CONFIG_IPV6_SIT) || defined(CONFIG_IPV6_SIT_MODULE)
  if( netif->type == ARPHRD_SIT){ //6RD device
    if(ppa_get_6rd_phyif_fn == NULL || (netif = ppa_get_6rd_phyif_fn(netif)) == NULL){
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"6RD, cannot get physical device\n");
      return PPA_FAILURE;
    } 
    
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"6RD physical device name: %s \n", netif->name);
    return ppa_get_physical_if(netif, NULL, phy_ifname);
  }
#endif

#if defined(CONFIG_IPV6_TUNNEL) || defined(CONFIG_IPV6_TUNNEL_MODULE)
  if( netif->type == ARPHRD_TUNNEL6) { //ipv6 tunnel device
#if defined(CONFIG_LTQ_PPA_DSLITE) && CONFIG_LTQ_PPA_DSLITE
    if(ppa_get_ip4ip6_phyif_fn == NULL || (netif = ppa_get_ip4ip6_phyif_fn(netif)) == NULL){
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"dslite, cannot get physical device\n");
      return PPA_FAILURE;
    }

    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"dslite physical device name: %s \n", netif->name);
    return ppa_get_physical_if(netif, NULL, phy_ifname);
#else
    return PPA_EINVAL;
#endif
  }
#endif   
  
  strcpy(phy_ifname, netif->name);
  return PPA_SUCCESS;
}

//int32_t ppa_get_underlying_vlan_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME vlan_ifname[PPA_IF_NAME_SIZE])
//{
//    int ret = -1;
//
//    if ( !netif )
//        netif = ppa_get_netif(ifname);
//
//    if ( !netif )
//        return -1;
//
//#if CONFIG_VLAN_8021Q
//  #ifdef CONFIG_PPPOE
//    if ( ppa_check_is_pppoe_netif(netif) && ppa_pppoe_get_eth_netif(netif, vlan_ifname) == 0 )
//        netif = ppa_get_netif(vlan_ifname);
//  #endif
//    if ( (netif->priv_flags & IFF_802_1Q_VLAN) )
//    {
//        strcpy(vlan_ifname, netif->name);
//        ret = 0;
//    }
//#endif
//
//    return ret;
//}

int32_t ppa_if_is_vlan_if(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE) || defined (CONFIG_WAN_VLAN_SUPPORT)
    if ( !netif )
        netif = ppa_get_netif(ifname);

    if ( !netif )
        return 0;  //  can not get interface

    if ( (netif->priv_flags & IFF_802_1Q_VLAN) )
        return 1;

#if defined(CONFIG_WAN_VLAN_SUPPORT)
    if ( (netif->priv_flags & IFF_BR2684_VLAN) )
        return 1;
#endif

#endif

    return 0;
}

int32_t ppa_is_macvlan_if(PPA_NETIF *netif, PPA_IFNAME *ifname) 
{
    if ( !netif )
	netif = ppa_get_netif(ifname);

    if ( !netif )
        return 0;  //  can not get interface
    
    if ( (netif->priv_flags & IFF_MACVLAN_PORT) )
        return 1;

    return 0;
}

int32_t ppa_vlan_get_underlying_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE])
{
    int ret=PPA_EINVAL;
#if !defined(CONFIG_VLAN_8021Q) && !defined(CONFIG_VLAN_8021Q_MODULE) && !defined(CONFIG_WAN_VLAN_SUPPORT)
    goto lbl_ret;
#endif
    if ( !netif )
        netif = ppa_get_netif(ifname);

    if ( !netif )
        goto lbl_ret;

/*
 * This order of eval below is important, so that nas0.10 iface
 * is correctly eval as VLAN_8021Q, and then nas0 can be eval
 * as BR2684_VLAN :-)
*/
#if defined(CONFIG_VLAN_8021Q) && !defined(CONFIG_VLAN_8021Q_MODULE)
    if ( (netif->priv_flags & IFF_802_1Q_VLAN) ) {
        strcpy(underlying_ifname, VLAN_DEV_INFO(netif)->real_dev->name);
        ret=PPA_SUCCESS;
    }
#endif
#if defined(CONFIG_WAN_VLAN_SUPPORT)
    if ( (netif->priv_flags & IFF_BR2684_VLAN) ) {
        /* br2684 does not create a new netdevice, so name is same */
        strcpy(underlying_ifname, netif->name);
        ret=PPA_SUCCESS;
    }
#endif
lbl_ret:
    return ret;
}

int32_t ppa_vlan_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE])
{
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    PPA_IFNAME ifname_tmp[2][PPA_IF_NAME_SIZE] = {{0}};
    int pos = 0;

    strcpy(ifname_tmp[pos], netif ? netif->name : ifname);

    while ( ppa_vlan_get_underlying_if(NULL, ifname_tmp[pos], ifname_tmp[pos ^ 0x01]) == PPA_SUCCESS )
        pos ^= 0x01;

    strcpy(phy_ifname, ifname_tmp[pos]);

    return PPA_SUCCESS;
#else
    return PPA_EPERM;
#endif
}

#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
extern int vlan_dev_get_vid(const char *dev_name, unsigned short* result);
extern u16 vlan_dev_get_egress_qos_mask(struct net_device *dev, struct sk_buff *skb);
#endif
#ifdef CONFIG_WAN_VLAN_SUPPORT
extern int br2684_vlan_dev_get_vid(struct net_device *dev, uint16_t *vid);
#endif

uint32_t ppa_get_vlan_id(PPA_NETIF *netif)
{
    unsigned short vid = ~0;

    if ( !netif )
        return ~0;
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    if ( vlan_dev_get_vid(netif->name, &vid) == 0 )
        return (uint32_t)vid;
#endif
#if defined(CONFIG_WAN_VLAN_SUPPORT)
    if (br2684_vlan_dev_get_vid(netif, &vid) == 0 )
        return (uint32_t) vid;
#endif

    return ~0;
}

uint32_t ppa_get_vlan_tag(PPA_BUF *ppa_buf)
{
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    unsigned short tag;

    if ( ppa_buf && vlan_get_tag(ppa_buf, &tag) == 0 )
        return (uint32_t)tag;
#endif

    return ~0;
}

uint32_t ppa_is_bond_slave(PPA_IFNAME *ifname, PPA_NETIF *netif)
{
    if ( !netif )
        netif = ppa_get_netif(ifname);

    if ( !netif )
        return 0;   //  can not get
    return netif_is_bond_slave(netif);
}

int32_t ppa_is_netif_bridged(PPA_IFNAME *ifname, PPA_NETIF *netif)
{
    if ( !netif )
        netif = ppa_get_netif(ifname);

    if ( !netif )
        return 0;   //  can not get

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    if ( netif->br_port != NULL )
        return 1;
#else
	if (br_port_exists(netif))
        return 1;
#endif
    else
        return 0;
}

int32_t ppa_get_bridge_member_ifs(PPA_IFNAME *ifname, int *num_ifs, PPA_IFNAME **pp_member_ifs)
{
    //  TODO: wait for implementation
    return PPA_ENOTIMPL;
}

int32_t ppa_if_is_br_if(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
    if ( !netif )
    {
        if ( !ifname || !(netif = ppa_get_netif(ifname)) )
            return 0;
    }

    if(ppa_dev_is_br(netif))
        return 1;

    return 0;
}

int32_t ppa_get_netif_ip(uint32_t *ip, PPA_NETIF *netif)
{
    struct in_ifaddr* if_info = NULL;
    struct in_device* in_dev = NULL;

    if ( !netif ) {
	return PPA_FAILURE;
    }
    
    in_dev = netif->ip_ptr;
    if ( !in_dev ) {
        return PPA_FAILURE;
    }    
       
    if_info = in_dev->ifa_list;
    for (;if_info;if_info=if_info->ifa_next)
    {
	if (!(strcmp(if_info->ifa_label,netif->name))) {
	    *ip = if_info->ifa_address;
	    return PPA_SUCCESS;
	}
    }
    return PPA_FAILURE; 
}


int32_t ppa_get_br_dst_port(PPA_NETIF *netif, PPA_BUF *ppa_buf, PPA_NETIF **p_netif)
{
    int32_t ret;
    struct net_bridge *br;
    //unsigned char *dest;
    unsigned char dest[PPA_ETH_ALEN];
    struct net_bridge_fdb_entry *dst;

    if ( !netif || !ppa_dev_is_br(netif) )
        return PPA_EINVAL;

    br = netdev_priv(netif);
    //dest = ppa_buf->data;
    if ( (ret = ppa_get_dst_mac(ppa_buf, NULL, dest,0)) != PPA_SUCCESS )
        return ret == PPA_ENOTAVAIL ? PPA_EAGAIN : PPA_FAILURE;

    if ( (dest[0] & 1) )
        return PPA_ENOTAVAIL;

    ret = PPA_FAILURE;
    ppa_rcu_read_lock();
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 13)
    if((dst = __br_fdb_get(br,dest)) != NULL)
#else    
    if((dst = __br_fdb_get(br,dest,0)) != NULL)
#endif
    {
        *p_netif = dst->dst->dev;
        ret = PPA_SUCCESS;
    }
    ppa_rcu_read_unlock();
    return ret;

#if 0
    if ( (dst = br_fdb_get(br, dest)) != NULL )
    {
        *p_netif = dst->dst->dev;
        br_fdb_put(dst);
        return PPA_SUCCESS;
    }

    return PPA_FAILURE;
#endif
}

int32_t ppa_get_br_dst_port_with_mac(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN], PPA_NETIF **p_netif)
{
    struct net_bridge *br;
    struct net_bridge_fdb_entry *dst;
    int32_t ret;

    if ( !netif || !ppa_dev_is_br(netif))
        return PPA_EINVAL;

    br = netdev_priv(netif);

    if ( mac[0] & 1 )
        return PPA_ENOTAVAIL;

    ret = PPA_FAILURE;
    ppa_rcu_read_lock();
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 8, 13)
    if((dst = __br_fdb_get(br,mac)) != NULL)
#else
    if((dst = __br_fdb_get(br,mac,0)) != NULL)
#endif
    {
        *p_netif = dst->dst->dev;
        ret = PPA_SUCCESS;
    }
    ppa_rcu_read_unlock();
    return ret;

#if 0
    if ( (dst = br_fdb_get(br, mac)) != NULL )
    {
        *p_netif = dst->dst->dev;
        br_fdb_put(dst);
        return PPA_SUCCESS;
    }

    return PPA_FAILURE;
#endif
}

#if !defined(CONFIG_ATM_BR2684) && !defined(CONFIG_ATM_BR2684_MODULE)
int32_t ppa_br2684_get_vcc(PPA_NETIF *netif, PPA_VCC **pvcc)
{
    return PPA_EPERM;
}

int32_t ppa_if_is_br2684(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
    return 0;
}

int32_t ppa_if_is_ipoa(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
    return 0;
}
#endif

#if !defined(CONFIG_PPPOATM) && !defined(CONFIG_PPPOATM_MODULE)
int32_t ppa_pppoa_get_vcc(PPA_NETIF *netif, PPA_VCC **patmvcc)
{
    return PPA_EPERM;
}

int32_t ppa_if_is_pppoa(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
    return 0;
}
#endif

uint32_t ppa_is_session_equal(PPA_SESSION *session1, PPA_SESSION *session2)
{
    return session1 == session2 ? 1 : 0;
}

uint32_t ppa_get_session_helper(PPA_SESSION *p_session)
{
#if defined (CONFIG_NF_CONNTRACK_SUPPORT) || defined (CONFIG_NF_CONNTRACK)
    struct nf_conn_help *help = nfct_help(p_session);
    return help ? (uint32_t)help->helper : 0;
#else
    return (uint32_t)(p_session->helper);
#endif
}

static inline uint32_t ppa_get_nat_helper(PPA_SESSION *p_session)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0) && defined(CONFIG_IP_NF_NAT_NEEDED)
    return p_session ? (uint32_t)(p_session->nat.info.helper) : 0;
#else
    return 0;
#endif
}

uint32_t ppa_check_is_special_session(PPA_BUF *ppa_buf, PPA_SESSION *p_session)
{
    enum ip_conntrack_info  ct_info;

    if ( !p_session )
        p_session = ppa_conntrack_get(ppa_buf, &ct_info);

    if(p_session) {
	if ( ppa_get_session_helper(p_session) )
	    return 1;

	if ( ppa_get_nat_helper(p_session) )
	    return 1;
    }
    /* XXX: Any other criteria for special sessions ? Add here */

    return 0;
}

int32_t ppa_is_pkt_host_output(PPA_BUF *ppa_buf)
{
    return ppa_buf->dev == NULL || ppa_buf->pkt_type == PACKET_OUTGOING ? 1 : 0;
}

int32_t ppa_is_pkt_host_in(PPA_BUF *ppa_buf)
{
    return ppa_buf->dev == NULL || ppa_buf->pkt_type == PACKET_HOST ? 1 : 0;
}

int32_t ppa_is_pkt_broadcast(PPA_BUF *ppa_buf)
{
    return ppa_buf->pkt_type == PACKET_BROADCAST ? 1 : 0;
}

int32_t ppa_is_pkt_loopback(PPA_BUF *ppa_buf)
{
    return ppa_buf->pkt_type == PACKET_LOOPBACK ? 1 : 0;
}

int32_t ppa_is_pkt_local(PPA_BUF *ppa_buf)
{
    /* XXX: Check this. What about ip_mc_forward */
    /* return ((uint32_t)ppa_buf->dst->input != (uint32_t)ip_forward); */
    struct dst_entry *dst = ppa_dst(ppa_buf);
    return dst != NULL && (uint32_t)(dst->input) == (uint32_t)ip_local_deliver ? 1 : 0;
}

int32_t ppa_is_tcp_established(PPA_SESSION *ppa_session)
{
    return ppa_session->proto.tcp.state == TCP_CONNTRACK_ESTABLISHED ? 1 : 0;
}

//  for backward compatible
int32_t ppa_is_tcp_open(PPA_SESSION *ppa_session)
{
    //  return nonzero if the tcp state is not TIME_WAIT or error
    return ppa_session->proto.tcp.state < TCP_CONNTRACK_TIME_WAIT ? 1 : 0;
}

int32_t ppa_lock_init(PPA_LOCK *p_lock)
{
    if ( !p_lock )
        return PPA_EINVAL;

    //mutex_init(p_lock);
    ppe_lock_init(&p_lock->lock);
    return PPA_SUCCESS;
}

void ppa_trace_lock_get(PPA_LOCK *p_lock)
{
    ASSERT(p_lock->cnt == 0,"Lock already taken!!!, lock cnt: %d\n", p_lock->cnt);
    if(p_lock->cnt != 0){
        dump_stack();
    }
    p_lock->cnt += 1;
}

void ppa_trace_lock_release(PPA_LOCK *p_lock)
{
    ASSERT(p_lock->cnt == 1, "Lock already released!!!, lock cnt: %d\n", p_lock->cnt);
    if(p_lock->cnt != 1){
        dump_stack();
    }
    p_lock->cnt -= 1;
}

void ppa_lock_get(PPA_LOCK *p_lock)
{    
    ppa_spin_lock_bh(&p_lock->lock);
    ppa_trace_lock_get(p_lock);
    
}

void ppa_lock_release(PPA_LOCK *p_lock)
{
     ppa_trace_lock_release(p_lock);
    ppa_spin_unlock_bh(&p_lock->lock);
}

uint32_t ppa_lock_get2(PPA_LOCK *p_lock)
{  
    unsigned long sys_flag = 0;

    spin_lock_irqsave(&p_lock->lock, sys_flag);
    return sys_flag;
}

void ppa_lock_release2(PPA_LOCK *p_lock, uint32_t sys_flag)
{  
    spin_unlock_irqrestore(&p_lock->lock, (unsigned long)sys_flag);
}

void ppa_lock_destroy(PPA_LOCK *p_lock)
{
}

uint32_t ppa_disable_int(void)
{
    unsigned long sys_flag = 0;

    spin_lock_irqsave(&g_local_irq_save_flag_lock, sys_flag);
    return sys_flag;
}

void ppa_enable_int(uint32_t flag)
{
    spin_unlock_irqrestore(&g_local_irq_save_flag_lock, (unsigned long)flag);
}

void *ppa_malloc(uint32_t size)
{
    gfp_t flags;

    if ( in_atomic() || in_interrupt() )
        flags = GFP_ATOMIC;
    else
        flags = GFP_KERNEL;
    return kmalloc(size, flags);
}

int32_t ppa_free(void *buf)
{
    kfree(buf);
    return PPA_SUCCESS;
}

#if defined(PPA_KMALLOC_METHOD)
#define MAX_MEM_SIZE_NUM  40
typedef struct cache_info
{
    uint32_t size;
    uint32_t count;
    //char *name;
}cache_info;
cache_info cache_size_array[MAX_MEM_SIZE_NUM];
uint32_t cache_size_num = 0;

int32_t ppa_mem_cache_create(const char *name, uint32_t size, PPA_MEM_CACHE **pp_cache)
{ //return pp_cache from 1, not from 0
    uint32_t i, index;

    if ( !pp_cache )
        return PPA_EINVAL;
 
    if( cache_size_num == 0 )
    {
        ppa_memset(cache_size_array, 0, sizeof(cache_size_array) );
    }
    else
    {
        //check whether such cache size alerady exists
        for(i=0; i<cache_size_num; i++)
        {
            if( cache_size_array[i].size == size )
            {           
                *pp_cache =(PPA_MEM_CACHE *) (i+1);
                cache_size_array[i].count ++;
                err(" *pp_cache=%u\n", (uint32_t )*pp_cache );
                return PPA_SUCCESS;
            }
        }
    }
    if( cache_size_num >= MAX_MEM_SIZE_NUM )
    {
        err("Too many cache size: %d. Need increase the cache_size_array.\n", cache_size_num );
        return PPA_EINVAL;
    }
    index = cache_size_num;
   
    cache_size_array[index].size = size;
    cache_size_array[index].count ++;
    cache_size_num++;
    *pp_cache = (PPA_MEM_CACHE *)(index + 1 );
    err(" *pp_cache=%u for size=%d\n", (uint32_t )*pp_cache,  cache_size_array[index].size);
    
    return PPA_SUCCESS;
}

int32_t ppa_mem_cache_destroy(PPA_MEM_CACHE *p_cache)
{  
      return PPA_SUCCESS;
}

void *ppa_mem_cache_alloc(PPA_MEM_CACHE *p_cache)
{
    uint32_t index = (uint32_t)p_cache -1;
    
    if( index >= cache_size_num )
    {
        err("Why p_cache index (%u) > cache_size_num(%u)\n", (unsigned int)index, (unsigned int)cache_size_num );
        return NULL;
    }
    if( cache_size_array[index].size == 0 )
    {
        err("why cache_size_array[%d] is zero\n", (uint32_t)index );
        return NULL;
    }    

    return ppa_malloc(cache_size_array[index].size);
}

void ppa_mem_cache_free(void *buf, PPA_MEM_CACHE *p_cache)
{
    ppa_free(buf);
}

int32_t ppa_kmem_cache_shrink(PPA_MEM_CACHE *cachep)
{
    return PPA_SUCCESS;
}

#else
int32_t ppa_mem_cache_create(const char *name, uint32_t size, PPA_MEM_CACHE **pp_cache)
{
    PPA_MEM_CACHE* p_cache;

    if ( !pp_cache )
        return PPA_EINVAL;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    p_cache = kmem_cache_create(name, size, 0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#else
    p_cache = kmem_cache_create(name, size, 0, SLAB_HWCACHE_ALIGN, NULL);
#endif
    if ( !p_cache )
        return PPA_ENOMEM;

    *pp_cache = p_cache;
    return PPA_SUCCESS;
}

int32_t ppa_mem_cache_destroy(PPA_MEM_CACHE *p_cache)
{
    if ( !p_cache )
        return PPA_EINVAL;

    kmem_cache_destroy(p_cache);

    return PPA_SUCCESS;
}

void *ppa_mem_cache_alloc(PPA_MEM_CACHE *p_cache)
{
    return kmem_cache_alloc(p_cache, GFP_ATOMIC);
}

void ppa_mem_cache_free(void *buf, PPA_MEM_CACHE *p_cache)
{
    kmem_cache_free(p_cache, buf);
}

int32_t ppa_kmem_cache_shrink(PPA_MEM_CACHE *cachep)
{
    return kmem_cache_shrink(cachep);
}
#endif

void ppa_memcpy(void *dst, const void *src, uint32_t count)
{
#if defined(ENABLE_MY_MEMCPY) && ENABLE_MY_MEMCPY
    char *d = (char *)dst, *s = (char *)src;

    if (count >= 32) {
        int i = 8 - (((unsigned long) d) & 0x7);

        if (i != 8)
            while (i-- && count--) {
                *d++ = *s++;
            }

        if (((((unsigned long) d) & 0x7) == 0) &&
                ((((unsigned long) s) & 0x7) == 0)) {
            while (count >= 32) {
                unsigned long long t1, t2, t3, t4;
                t1 = *(unsigned long long *) (s);
                t2 = *(unsigned long long *) (s + 8);
                t3 = *(unsigned long long *) (s + 16);
                t4 = *(unsigned long long *) (s + 24);
                *(unsigned long long *) (d) = t1;
                *(unsigned long long *) (d + 8) = t2;
                *(unsigned long long *) (d + 16) = t3;
                *(unsigned long long *) (d + 24) = t4;
                d += 32;
                s += 32;
                count -= 32;
            }
            while (count >= 8) {
                *(unsigned long long *) d =
                                            *(unsigned long long *) s;
                d += 8;
                s += 8;
                count -= 8;
            }
        }

        if (((((unsigned long) d) & 0x3) == 0) &&
                ((((unsigned long) s) & 0x3) == 0)) {
            while (count >= 4) {
                *(unsigned long *) d = *(unsigned long *) s;
                d += 4;
                s += 4;
                count -= 4;
            }
        }

        if (((((unsigned long) d) & 0x1) == 0) &&
                ((((unsigned long) s) & 0x1) == 0)) {
            while (count >= 2) {
                *(unsigned short *) d = *(unsigned short *) s;
                d += 2;
                s += 2;
                count -= 2;
            }
        }
    }

    while (count--) {
        *d++ = *s++;
    }

//    return d;
#else
    memcpy(dst, src, count);
#endif
}

void ppa_memset(void *dst, uint32_t pad, uint32_t n)
{
    memset(dst, pad, n);
}

int ppa_memcmp(const void *src, const void *dest, uint32_t count)
{
  return memcmp(src, dest, count);
}

int32_t ppa_timer_setup(PPA_TIMER *p_timer, void (*callback)(unsigned long), unsigned long data)
{
  setup_timer(p_timer,callback,data);
  return 0;
}
EXPORT_SYMBOL(ppa_timer_setup);

int32_t ppa_timer_pending(PPA_TIMER *p_timer)
{
  return timer_pending(p_timer);
}
EXPORT_SYMBOL(ppa_timer_pending);

int32_t ppa_timer_init(PPA_TIMER *p_timer, void (*callback)(unsigned long))
{
    //ppa_memset(p_timer, 0, sizeof(*p_timer));
    init_timer(p_timer);
    p_timer->function = callback;

    return 0;
}

int32_t ppa_timer_add(PPA_TIMER *p_timer, uint32_t timeout_in_sec)
{
    p_timer->expires = jiffies + timeout_in_sec * HZ - 1;
    add_timer(p_timer);

    return 0;
}

void ppa_timer_del(PPA_TIMER *p_timer)
{
    del_timer_sync(p_timer);
}

uint32_t ppa_get_time_in_10msec(void)
{
    if ( HZ == 100 )
        return jiffies;
    else if ( HZ >= 1000 )
    {
        return (jiffies + HZ / 200) / (HZ / 100);
    }
    else
    {
        uint64_t divident = (uint64_t)jiffies * 100 + HZ / 2;

        return __div64_32(&divident, HZ);
    }
}

uint32_t ppa_get_time_in_sec(void)
{
    return (jiffies + HZ / 2) / HZ;
}

PPA_TASK* ppa_kthread_create( int (*threadfn)(void *data), void *data, const char fn_name[])
{
    return kthread_create(threadfn,data,fn_name);	
}

int ppa_kthread_should_stop(void)
{
    return kthread_should_stop();
}

int ppa_kthread_stop(PPA_TASK* k)
{
    return kthread_stop(k);
}

void ppa_wake_up_process(PPA_TASK* k)
{
    wake_up_process(k);
}

void ppa_schedule(void)
{
    schedule();
}

void ppa_set_current_state(int state)
{
    __set_current_state(state);
}

PPA_NETIF* ppa_netdev_master_upper_dev_get(PPA_NETIF *netif)
{
    return netdev_master_upper_dev_get(netif);
}

void ppa_rtnl_lock(void) 
{ 
    rtnl_lock();
}

void ppa_rtnl_unlock(void)
{
    rtnl_unlock();
}

int32_t ppa_atomic_read(PPA_ATOMIC *v)
{
    return atomic_read(v);
}

void ppa_atomic_set(PPA_ATOMIC *v, int32_t i)
{
    atomic_set(v, i);
}

int32_t ppa_atomic_inc(PPA_ATOMIC *v)
{
    return atomic_inc_return(v);
}

int32_t ppa_atomic_dec(PPA_ATOMIC *v)
{
    return atomic_dec_if_positive(v);
}

int32_t ppa_atomic_inc_not_zero(PPA_ATOMIC *v)
{
    return atomic_inc_not_zero(v);
}

int32_t ppa_atomic_dec_and_test(PPA_ATOMIC *v)
{
    return atomic_dec_and_test(v);
}
EXPORT_SYMBOL(ppa_atomic_dec_and_test);

void ppa_hlist_replace(PPA_HLIST_NODE *old, PPA_HLIST_NODE *new)
{
    new->next = old->next;
    new->pprev = old->pprev;
    *(new->pprev) = new;
    if(old->next)
        new->next->pprev = &new->next;
    old->next = LIST_POISON1;
    old->pprev = LIST_POISON2;
}

PPA_BUF *ppa_buf_clone(PPA_BUF *buf, uint32_t flags)
{
    return skb_clone(buf, 0);
}

int32_t ppa_buf_cloned(PPA_BUF *buf)
{
    return skb_cloned(buf) ? 1 : 0;
}

PPA_BUF *ppa_buf_get_prev(PPA_BUF *buf)
{
    return buf != NULL ? buf->prev : NULL;
}

PPA_BUF *ppa_buf_get_next(PPA_BUF *buf)
{
    return buf != NULL ? buf->next : NULL;
}

void ppa_buf_free(PPA_BUF *buf)
{
    if ( buf != NULL )
    {
        buf->prev = buf->next = NULL;
        dev_kfree_skb_any(buf);
    }
}

uint32_t ppa_copy_from_user(void *to, const void PPA_USER  *from, uint32_t  n)
{
    return copy_from_user(to, from, n);
}
uint32_t ppa_copy_to_user(void PPA_USER *to, const void *from, uint32_t  n)
{
    return copy_to_user(to, from, n);
}
uint8_t *ppa_strcpy(uint8_t *dest, const uint8_t *src)
{
    return strcpy(dest, src);
}

uint8_t *ppa_strncpy(uint8_t *dest, const uint8_t *src, PPA_SIZE_T n)
{
    return strncpy(dest, src, n);
}

PPA_SIZE_T  ppa_strlen(const uint8_t *s)
{
    return strlen(s);
}

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
int32_t ppa_str_cmp(char *str1,char *str2)
{
    return ( (strcmp(str1,str2) == 0) ? 1 : 0);
}


session_type ppa_is_ipv4_ipv6(PPA_XFRM_STATE *x)
{       
        switch (x->props.family) {
        default:
        case AF_INET:
                return SESSION_IPV4;
        case AF_INET6:
                return SESSION_IPV6;
        }
}

bool ppa_ipsec_addr_equal(xfrm_address_t *a, xfrm_address_t *b, sa_family_t family)
{
        switch (family) {
        default:
        case AF_INET:
                return ((__force u32)a->a4 ^ (__force u32)b->a4) == 0;
        case AF_INET6:
                return xfrm6_addr_equal(a, b);
        }
}
#endif

const uint8_t *ppa_kallsyms_lookup(uint32_t addr, uint32_t *symbolsize, uint32_t *offset, uint8_t **modname, uint8_t *namebuf)
{
#if defined(CONFIG_LTQ_PPA_API)
    return kallsyms_lookup(addr, (unsigned long *)symbolsize, (unsigned long *)offset, (char **)modname, (char *)namebuf );
#else
    return NULL;
#endif
}

int32_t ppa_register_netdev(PPA_NETIF *dev)
{
    return register_netdev(dev);
}

void ppa_unregister_netdev(PPA_NETIF *dev)
{
    unregister_netdev(dev);
}

int32_t ppa_register_chrdev(int32_t  major, const uint8_t *name, PPA_FILE_OPERATIONS  *fops)
{
    return register_chrdev(major, name, fops);
}

void ppa_unregister_chrdev(int32_t  major, const uint8_t *name)
{
    unregister_chrdev(major, name);
    return;
}

int ppa_snprintf(uint8_t * buf, size_t size, const uint8_t *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i=vsnprintf(buf,size,fmt,args);
    va_end(args);
    return i;
}

int ppa_sprintf(uint8_t * buf, const uint8_t *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i=vsprintf(buf,fmt,args);
    va_end(args);
    return i;
}

/*Note, in linux, IOC is defined as below
   define _IOC(dir,type,nr,size) \
         (((dir)  << _IOC_DIRSHIFT) | \
          ((type) << _IOC_TYPESHIFT) | \
          ((nr)   << _IOC_NRSHIFT) | \
          ((size) << _IOC_SIZESHIFT))
   or other os, should refer to it to define below adaption layer accordingly
*/

uint32_t ppa_ioc_type(uint32_t nr)
{
    return  _IOC_TYPE(nr);
}

uint32_t ppa_ioc_nr(uint32_t nr)
{
    return  _IOC_NR(nr);
}

uint32_t ppa_ioc_dir(uint32_t nr)
{
    return  _IOC_DIR(nr);
}

uint32_t ppa_ioc_read(void)
{
    return  _IOC_READ;
}

uint32_t ppa_ioc_write(void)
{
    return  _IOC_WRITE;
}

uint32_t ppa_ioc_size(uint32_t nr)
{
    return  _IOC_SIZE(nr);
}

uint32_t ppa_ioc_access_ok(uint32_t type, uint32_t addr, uint32_t size)
{
    return  access_ok(type, addr, size);
}

uint32_t ppa_ioc_verify_write(void)
{
    return VERIFY_WRITE;
}

uint32_t ppa_ioc_verify_read(void)
{
    return VERIFY_READ;
}

uint16_t ppa_vlan_dev_get_egress_qos_mask(PPA_NETIF *dev, PPA_BUF *buf)
{
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
    extern uint16_t vlan_dev_get_egress_qos_mask(PPA_NETIF *dev, PPA_BUF *skb);

    if ( (dev->priv_flags & IFF_802_1Q_VLAN) )
        return vlan_dev_get_egress_qos_mask(dev, buf);
#endif
#if defined(CONFIG_WAN_VLAN_SUPPORT)
    if ( (dev->priv_flags & IFF_BR2684_VLAN) )
        return ((struct sk_buff *)buf)->priority & 0x7;
#endif
    return 0;
}

uint32_t ppa_get_hash_from_ct( const PPA_SESSION *ct, 
                               uint8_t dir, 
                               PPA_TUPLE* tuple)
{
  return get_hash_from_ct(ct,dir,tuple);
}
EXPORT_SYMBOL(ppa_get_hash_from_ct);

int ppa_get_hash_from_packet( PPA_BUF *ppa_buf, 
                           unsigned char pf, 
                           uint32_t *u32_hash, 
                           PPA_TUPLE* tuple )
{
  return get_hash_from_skb(ppa_buf,pf,u32_hash,tuple); 
}
EXPORT_SYMBOL(ppa_get_hash_from_packet);

static INLINE struct dst_entry *ppa_dst(const PPA_BUF *buf)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    return buf->dst;
#else
    return skb_dst(buf);
#endif
}

static INLINE struct net_device *ppa_dev_get_by_name(const PPA_IFNAME *ifname)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    return dev_get_by_name(ifname);
#else
    return dev_get_by_name(&init_net, ifname);
#endif

}

#ifdef CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT
int  ppa_get_mtu(PPA_NETIF *netif)
{
  PPA_NETIF *netif_tmp=NULL;
  PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE]={0};
#if 0 
	if(netif->type == ARPHRD_SIT){
  
    if(ppa_get_6rd_phyif_fn == NULL || (netif_tmp = ppa_get_6rd_phyif_fn(netif)) == NULL){
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
          "Add sit device: %s, but cannot find the physical device\n", 
          netif->name);
    }else{
      return(netif_tmp->mtu);
    }
  }

#if defined(CONFIG_LTQ_PPA_DSLITE) && CONFIG_LTQ_PPA_DSLITE
	if(netif->type == ARPHRD_TUNNEL6){

    if(ppa_get_ip4ip6_phyif_fn == NULL || (netif_tmp = ppa_get_ip4ip6_phyif_fn(netif)) == NULL){
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Add ip4ip6 device: %s, but cannot find the physical device\n", netif->name);
    }else{
      return(netif_tmp->mtu);
    }
  }
#endif //CONFIG_LTQ_PPA_DSLITE

	if ( ppa_check_is_ppp_netif(netif) ) {

    if( ppa_check_is_pppoe_netif(netif) && 
        ppa_pppoe_get_physical_if(netif, NULL, underlying_ifname) == PPA_SUCCESS ) {
      netif_tmp = ppa_get_netif(underlying_ifname);
      return(netif_tmp->mtu);
    }
  }
#else 
  if( ppa_get_physical_if( netif, NULL, underlying_ifname) == PPA_SUCCESS)  {
    netif_tmp = ppa_get_netif(underlying_ifname);
    if(netif_tmp)
	return (netif_tmp->mtu);
  } 
  
#endif
  return netif->mtu;
}
#endif

uint32_t cal_64_div(uint64_t t1, uint64_t t2)
{ /* cal the value of t1 divided by t2 */
    if( t1 == 0 ) return 0;
    if( t2 == 0 ) return (uint32_t)-1;

    while( (t1 > WRAPROUND_32BITS) || (t2 > WRAPROUND_32BITS) )
    {
        t2 = t2 >> 1;
        t1 = t1 >> 1;
    }

    if( t1 == 0 ) return 0;
    if( t2 == 0 ) return (uint32_t)-1;
    
    return (uint32_t)t1/(uint32_t)t2;
}


static INLINE int ppa_dev_is_br(PPA_NETIF *netif)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    return (netif->hard_start_xmit == br_dev_xmit);
#else
    return (netif->netdev_ops->ndo_start_xmit == br_dev_xmit);
#endif
}

#ifdef CONFIG_LTQ_PPA_STANDALONE_AL
int __init ppa_api_stack_init(void)
{
    return 0;
}

void __exit ppa_api_stack_exit(void)
{
}

module_init(ppa_api_stack_init);
module_exit(ppa_api_stack_exit);
#endif


EXPORT_SYMBOL(ppa_get_stack_al_id);
EXPORT_SYMBOL(ppa_get_session);
EXPORT_SYMBOL(ppa_is_pkt_ipv6);
EXPORT_SYMBOL(ppa_get_pkt_ip_proto);
EXPORT_SYMBOL(ppa_get_pkt_ip_tos);
EXPORT_SYMBOL(ppa_get_pkt_src_ip);
EXPORT_SYMBOL(ppa_get_multicast_pkt_ip);
EXPORT_SYMBOL(ppa_get_pkt_ip_len);
EXPORT_SYMBOL(ppa_get_pkt_dst_ip);
EXPORT_SYMBOL(ppa_get_pkt_ip_string);
EXPORT_SYMBOL(ppa_get_pkt_src_port);
EXPORT_SYMBOL(ppa_get_pkt_dst_port);
EXPORT_SYMBOL(ppa_get_pkt_rx_src_mac_addr);
EXPORT_SYMBOL(ppa_get_pkt_rx_dst_mac_addr);
EXPORT_SYMBOL(ppa_get_pkt_src_if);
EXPORT_SYMBOL(ppa_get_pkt_dst_if);
EXPORT_SYMBOL(ppa_get_pkt_priority);
EXPORT_SYMBOL(ppa_get_skb_mark);
#ifdef CONFIG_NETWORK_EXTMARK
EXPORT_SYMBOL(ppa_get_skb_extmark);
#endif
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
EXPORT_SYMBOL(ppa_get_low_prio_thresh);
EXPORT_SYMBOL(ppa_get_def_prio_thresh);
EXPORT_SYMBOL(ppa_get_low_prio_data_rate);
EXPORT_SYMBOL(ppa_get_def_prio_data_rate);
EXPORT_SYMBOL(ppa_timespec_to_ns);
EXPORT_SYMBOL(ppa_get_monotonic);
EXPORT_SYMBOL(ppa_timespec_sub);
EXPORT_SYMBOL(ppa_get_session_priority);
EXPORT_SYMBOL(ppa_get_session_limit_enable);
EXPORT_SYMBOL(ppa_get_tcp_steady_offset);
EXPORT_SYMBOL(ppa_get_tcp_initial_offset);
#endif

EXPORT_SYMBOL(ppa_set_pkt_priority);
#if !defined(CONFIG_PPPOE) && !defined(CONFIG_PPPOE_MODULE)
  EXPORT_SYMBOL(ppa_pppoe_get_pppoe_addr);
  EXPORT_SYMBOL(ppa_pppoe_get_pppoe_session_id);
  EXPORT_SYMBOL(ppa_get_pkt_pppoe_session_id);
  EXPORT_SYMBOL(ppa_pppoe_get_eth_netif);
#endif
EXPORT_SYMBOL(ppa_pppoe_get_physical_if);
EXPORT_SYMBOL(ppa_pppol2tp_get_physical_if);
EXPORT_SYMBOL(ppa_check_is_ppp_netif);
EXPORT_SYMBOL(ppa_check_is_pppoe_netif);
EXPORT_SYMBOL(ppa_pppoe_get_dst_mac);
EXPORT_SYMBOL(ppa_check_is_pppol2tp_netif);
EXPORT_SYMBOL(ppa_check_is_lro_enabled_netif);
EXPORT_SYMBOL(ppa_pppol2tp_get_dst_mac);
#if (!defined(CONFIG_PPPOL2TP) && !defined(CONFIG_PPPOL2TP_MODULE)) || (!defined(CONFIG_LTQ_PPA_API) && !defined(CONFIG_LTQ_PPA_API_MODULE))
EXPORT_SYMBOL(ppa_pppol2tp_get_l2tp_session_id);
EXPORT_SYMBOL(ppa_pppol2tp_get_l2tp_tunnel_id);
EXPORT_SYMBOL(ppa_pppol2tp_get_base_netif);
EXPORT_SYMBOL(ppa_pppol2tp_get_l2tp_addr);
EXPORT_SYMBOL(ppa_pppol2tp_get_l2tp_dmac);
EXPORT_SYMBOL(ppa_pppol2tp_get_src_addr);
EXPORT_SYMBOL(ppa_pppol2tp_get_dst_addr);
#endif
EXPORT_SYMBOL(ppa_get_dst_mac);
EXPORT_SYMBOL(ppa_get_netif);
#ifdef CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT
EXPORT_SYMBOL(ppa_get_mtu);
#endif
EXPORT_SYMBOL(ppa_put_netif);
EXPORT_SYMBOL(ppa_get_netif_hwaddr);
EXPORT_SYMBOL(ppa_get_br_dev);
EXPORT_SYMBOL(ppa_get_netif_name);
EXPORT_SYMBOL(ppa_is_netif_equal);
EXPORT_SYMBOL(ppa_is_netif_name);
//EXPORT_SYMBOL(ppa_is_netif_name_prefix);
EXPORT_SYMBOL(ppa_get_physical_if);
//EXPORT_SYMBOL(ppa_get_underlying_vlan_if);
EXPORT_SYMBOL(ppa_if_is_vlan_if);
EXPORT_SYMBOL(ppa_is_macvlan_if);
EXPORT_SYMBOL(ppa_vlan_get_underlying_if);
EXPORT_SYMBOL(ppa_vlan_get_physical_if);
EXPORT_SYMBOL(ppa_get_vlan_id);
EXPORT_SYMBOL(ppa_get_vlan_tag);
EXPORT_SYMBOL(ppa_is_bond_slave);
EXPORT_SYMBOL(ppa_is_netif_bridged);
EXPORT_SYMBOL(ppa_get_bridge_member_ifs);
EXPORT_SYMBOL(ppa_if_is_br_if);
EXPORT_SYMBOL(ppa_get_netif_ip);
EXPORT_SYMBOL(ppa_get_br_dst_port);
EXPORT_SYMBOL(ppa_get_br_dst_port_with_mac);
#if !defined(CONFIG_ATM_BR2684) && !defined(CONFIG_ATM_BR2684_MODULE)
  EXPORT_SYMBOL(ppa_br2684_get_vcc);
  EXPORT_SYMBOL(ppa_if_is_br2684);
  EXPORT_SYMBOL(ppa_if_is_ipoa);
#endif
#if !defined(CONFIG_PPPOATM) && !defined(CONFIG_PPPOATM_MODULE)
  EXPORT_SYMBOL(ppa_pppoa_get_vcc);
  EXPORT_SYMBOL(ppa_if_is_pppoa);
#endif
EXPORT_SYMBOL(ppa_is_session_equal);
EXPORT_SYMBOL(ppa_get_session_helper);
EXPORT_SYMBOL(ppa_check_is_special_session);
EXPORT_SYMBOL(ppa_is_pkt_fragment);
EXPORT_SYMBOL(ppa_is_pkt_host_output);
EXPORT_SYMBOL(ppa_is_pkt_host_in);
EXPORT_SYMBOL(ppa_is_pkt_broadcast);
EXPORT_SYMBOL(ppa_get_pkt_mac_string);
EXPORT_SYMBOL(ppa_is_pkt_multicast);
EXPORT_SYMBOL(ppa_is_pkt_loopback);
EXPORT_SYMBOL(ppa_is_pkt_local);
EXPORT_SYMBOL(ppa_is_pkt_routing);
EXPORT_SYMBOL(ppa_is_pkt_mc_routing);
EXPORT_SYMBOL(ppa_is_tcp_established);
EXPORT_SYMBOL(ppa_is_tcp_open);
EXPORT_SYMBOL(ppa_lock_init);
EXPORT_SYMBOL(ppa_lock_get);
EXPORT_SYMBOL(ppa_lock_release);
EXPORT_SYMBOL(ppa_lock_get2);
EXPORT_SYMBOL(ppa_lock_release2);
EXPORT_SYMBOL(ppa_lock_destroy);
EXPORT_SYMBOL(ppa_disable_int);
EXPORT_SYMBOL(ppa_enable_int);
EXPORT_SYMBOL(ppa_malloc);
EXPORT_SYMBOL(ppa_free);
EXPORT_SYMBOL(ppa_mem_cache_create);
EXPORT_SYMBOL(ppa_mem_cache_destroy);
EXPORT_SYMBOL(ppa_mem_cache_alloc);
EXPORT_SYMBOL(ppa_mem_cache_free);
EXPORT_SYMBOL(ppa_kmem_cache_shrink);
EXPORT_SYMBOL(ppa_memcpy);
EXPORT_SYMBOL(ppa_memset);
EXPORT_SYMBOL(ppa_memcmp);
EXPORT_SYMBOL(ppa_timer_init);
EXPORT_SYMBOL(ppa_timer_add);
EXPORT_SYMBOL(ppa_timer_del);
EXPORT_SYMBOL(ppa_get_time_in_10msec);
EXPORT_SYMBOL(ppa_get_time_in_sec);
EXPORT_SYMBOL(ppa_kthread_create);
EXPORT_SYMBOL(ppa_kthread_should_stop);
EXPORT_SYMBOL(ppa_kthread_stop);
EXPORT_SYMBOL(ppa_wake_up_process);
EXPORT_SYMBOL(ppa_schedule);
EXPORT_SYMBOL(ppa_set_current_state);
EXPORT_SYMBOL(ppa_netdev_master_upper_dev_get);
EXPORT_SYMBOL(ppa_rtnl_lock);
EXPORT_SYMBOL(ppa_rtnl_unlock);
EXPORT_SYMBOL(ppa_atomic_read);
EXPORT_SYMBOL(ppa_atomic_set);
EXPORT_SYMBOL(ppa_atomic_inc);
EXPORT_SYMBOL(ppa_atomic_dec);
EXPORT_SYMBOL(ppa_atomic_inc_not_zero);
EXPORT_SYMBOL(ppa_hlist_replace);
EXPORT_SYMBOL(ppa_buf_clone);
EXPORT_SYMBOL(ppa_buf_cloned);
EXPORT_SYMBOL(ppa_buf_get_prev);
EXPORT_SYMBOL(ppa_buf_get_next);
EXPORT_SYMBOL(ppa_buf_free);
EXPORT_SYMBOL(ppa_copy_from_user);
EXPORT_SYMBOL(ppa_copy_to_user);
EXPORT_SYMBOL(ppa_strcpy);
EXPORT_SYMBOL(ppa_strncpy);
EXPORT_SYMBOL(ppa_strlen);
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
EXPORT_SYMBOL(ppa_str_cmp);
EXPORT_SYMBOL(ppa_is_ipv4_ipv6);
EXPORT_SYMBOL(ppa_ipsec_addr_equal);
#endif
EXPORT_SYMBOL(ppa_kallsyms_lookup);
EXPORT_SYMBOL(ppa_register_netdev);
EXPORT_SYMBOL(ppa_unregister_netdev);
EXPORT_SYMBOL(ppa_register_chrdev);
EXPORT_SYMBOL(ppa_unregister_chrdev);
EXPORT_SYMBOL(ppa_sprintf);
EXPORT_SYMBOL(ppa_snprintf);
EXPORT_SYMBOL(ppa_ioc_type);
EXPORT_SYMBOL(ppa_ioc_nr);
EXPORT_SYMBOL(ppa_ioc_read);
EXPORT_SYMBOL(ppa_ioc_write);
EXPORT_SYMBOL(ppa_ioc_size);
EXPORT_SYMBOL(ppa_ioc_access_ok);
EXPORT_SYMBOL(ppa_ioc_dir);
EXPORT_SYMBOL(ppa_ioc_verify_read);
EXPORT_SYMBOL(ppa_ioc_verify_write);
EXPORT_SYMBOL(ppa_vlan_dev_get_egress_qos_mask);
EXPORT_SYMBOL(g_ppa_dbg_enable);
EXPORT_SYMBOL(max_print_num);
EXPORT_SYMBOL(cal_64_div);

