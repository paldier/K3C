/*******************************************************************************
**
** FILE NAME    : ppa_api_tmpl_buf.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : April 2015
** AUTHOR       : Lantiq
** DESCRIPTION  : The session template buffer construction routines.
** COPYRIGHT    :              Copyright (c) 2013
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** April 2015   Syed              Initiate Version
*******************************************************************************/
#define CONFIG_LTQ_PPA_API_SW_FASTPATH 1
/*
 *  Common Head File
 */
#include "ppa_api_common.h"
#include "swa_stack_al.h"
#include "swa_ipproto.h"
#include "../platform/xrx500/ltq_mpe_api.h"

#define UDP_HDR_LEN        8
#define IPV4_HDR_LEN      20  //assuming no option fields are present
#define IPV6_HDR_LEN      40
#define ETH_HLEN          14  /* Total octets in header.   */
//#define VLAN_HLEN    4

#define L2TP_HLEN         38 /* IPv4=20 + UDP=8 + L2TP_HDR=8 + PPP_HDR=4 */
#define L2TP_HDR_LEN      6

#define PROTO_FAMILY_IP   2
#define PROTO_FAMILY_IPV6 10

#define PPP_LEN           4
#define PPPOE_LEN         6
#define PPPOE_HLEN        8

#define IPSEC_HLEN	  20

#define PPPOE_IPV4_TAG    0x0021
#define PPPOE_IPV6_TAG    0x0057
//#define DEBUG_TMPL_BUFFER 1


#undef NIPQUAD
/*! \def NIPQUAD
    \brief Macro that specifies NIPQUAD definition for printing IPV4 address
 */
#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]


#ifdef DEBUG_TMPL_BUFFER
/*
* \brief  dump data
* \param[in] len length of the data buffer
* \param[in] pData Pointer to data to dump
*
* \return void No Value
*/
#include <linux/module.h>
#include <linux/kernel.h>

#define dbg_print printk
static inline void dump_data(uint32_t len, char *pData)
{
  uint32_t bytes = 1;

  while( len ) {
    
    dbg_print("%2.2X%c",(uint8_t)(*pData), (bytes%16)?' ':'\n' ) ;
    --len;
    bytes++;
    pData++;
  }
  dbg_print("\n");
}


static void print_action_info(struct session_action* s_act, uint8_t ipV4)
{
  dbg_print("Template buffer: Session Action\n");
  dbg_print("Protocol              = %d\n",s_act->protocol);
  dbg_print("dst_pmac_port_num     = %d\n",s_act->dst_pmac_port_num);
  dbg_print("dst_pmac_port_list[0] = %d\n",s_act->dst_pmac_port_list[0]);
  
  dbg_print("routing_flag          = %d\n",s_act->routing_flag);
  
  dbg_print("new_src_ip_en         = %d\n",s_act->new_src_ip_en);
  if(ipV4)
    dbg_print("new_src_ip            = %pI4\n",&s_act->new_src_ip.ip4.ip);
  else
    dbg_print("new_src_ip            = %pI6\n",&s_act->new_src_ip.ip6.ip);
  
  dbg_print("new_dst_ip_en         = %d\n",s_act->new_dst_ip_en);
  if(ipV4)
    dbg_print("new_dst_ip            = %pI4\n",&s_act->new_dst_ip.ip4.ip);
  else
    dbg_print("new_dst_ip            = %pI6\n",&s_act->new_dst_ip.ip6.ip);
  
  dbg_print("pppoe_offset_en       = %d\n",s_act->pppoe_offset_en);
  dbg_print("pppoe_offset          = %d\n",s_act->pppoe_offset);
  
  dbg_print("tunnel_ip_offset_en   = %d\n",s_act->tunnel_ip_offset_en);
  dbg_print("tunnel_ip_offset      = %d\n",s_act->tunnel_ip_offset);

  dbg_print("tunnel_udp_offset_en  = %d\n",s_act->tunnel_udp_offset_en);
  dbg_print("tunnel_udp_offset     = %d\n",s_act->tunnel_udp_offset);

  dbg_print("in_eth_iphdr_offset_en= %d\n",s_act->in_eth_iphdr_offset_en);
  dbg_print("in_eth_iphdr_offset   = %d\n",s_act->in_eth_iphdr_offset);
 
  dbg_print("tunnel_type           = %d\n",s_act->tunnel_type);
  dbg_print("tunnel_rm_en          = %d\n",s_act->tunnel_rm_en);
  dbg_print("tunnel id          = %d\n",s_act->tunnel_id);
 
  dbg_print("pkt_len_delta         = %d\n",s_act->pkt_len_delta);
  dbg_print("templ_len             = %d\n",s_act->templ_len);

  dump_data(s_act->templ_len,s_act->templ_buf);
}
#else

#define dbg_print(fmt , ...)
#define print_action_info(s_act, ipV4)

#endif

#define IsSoftwareAccelerated(flags)  ( (flags) & SESSION_ADDED_IN_SW)
#define IsPppoeSession(flags)         ( (flags) & SESSION_VALID_PPPOE )
#define IsLanSession(flags)           ( (flags) & SESSION_LAN_ENTRY )
#define IsValidVlanIns(flags)         ( (flags) & SESSION_VALID_VLAN_INS )
#define IsValidVlanRm(flags)          ( (flags) & SESSION_VALID_VLAN_RM )
#define IsValidOutVlanIns(flags)      ( (flags) & SESSION_VALID_OUT_VLAN_INS)
#define IsIpv6Session(flags)          ( (flags) & SESSION_IS_IPV6)
#define IsTunneledSession(flags)      ( (flags) & (SESSION_TUNNEL_DSLITE | \
                                                   SESSION_TUNNEL_6RD | \
                                                   SESSION_VALID_PPPOL2TP))
#define IsDsliteSession(flags)        ( (flags) & SESSION_TUNNEL_DSLITE )
#define Is6rdSession(flags)           ( (flags) &  SESSION_TUNNEL_6RD)
#define IsL2TPSession(flags)          ( (flags) & SESSION_VALID_PPPOL2TP)
#define IsValidNatIP(flags)           ( (flags) & SESSION_VALID_NAT_IP)

#define IsBridgedSession(p_item)      ( ((p_item)->flag2) & SESSION_FLAG2_BRIDGED_SESSION) 
#define IsIpsecSession(p_item)      ( ((p_item)->flag2) & SESSION_FLAG2_VALID_IPSEC_OUTBOUND) 

static inline void ppa_htons(uint16_t *ptr, uint16_t val )
{
  *ptr = val;
}

static inline void ppa_htonl(uint32_t *ptr, uint32_t val )
{
  *ptr = val;
}

static
void form_IPv4_header(struct iphdr *iph, 
                     uint32_t src_ip, 
                     uint32_t dst_ip, 
                     uint32_t protocol_type,
                     uint16_t dataLen )
{

  swa_memset((void *)iph, 0, sizeof(struct iphdr));

  iph->version  = 4;
  iph->protocol  = protocol_type ;
  iph->ihl    = 5;
  iph->ttl    = 64 ;
  iph->saddr    = src_ip;
  iph->daddr    = dst_ip;

  iph->tot_len  = dataLen;
  
  iph->check = swa_ip_fast_csum((unsigned char *)iph, iph->ihl);
}

static
void form_UDP_header(struct udphdr *udph, 
                    uint16_t sport, 
                    uint16_t dport,
                    uint16_t len )
{
  swa_memset((void *)udph, 0, sizeof(struct udphdr)); 
  udph->source   = sport ;
  udph->dest     = dport ;
  udph->len      = len;

  udph->check = swa_ip_fast_csum((unsigned char *)udph, udph->len);
}

  

static uint16_t ppa_out_header_len( struct swa_session_list_item *p_item,  
                                   uint32_t *delta) 
{
  uint16_t headerlength=0;

  uint8_t is_lan = IsLanSession( p_item->flags )?1:0;
       
  if( ! IsBridgedSession(p_item) ) {
   
    if( IsDsliteSession(p_item->flags) ) {
      /* Handle DS-Lite Tunneled sessions */ 
      if( is_lan ) {
         headerlength += IPV6_HDR_LEN;
      } 
    } else if( Is6rdSession(p_item->flags) ) {
     
      /* Handle 6rd Tunneled sessions */ 
      if( is_lan ) {
        headerlength += IPV4_HDR_LEN;
      }
    } else if ( IsL2TPSession(p_item->flags)) {
      /* handle for l2tp Tunneled session */
      headerlength += L2TP_HLEN ;
      *delta += L2TP_HLEN;
    } else if(IsIpsecSession(p_item)) {

      headerlength += IPSEC_HLEN ;
      *delta += IPSEC_HLEN;
    } 
  }
    
  if( swa_is_GreSession(p_item)) {
    //Both IPoGRE and EoGRE get tunnel hdr length
    if( is_lan) {
      uint16_t hdrlen=0;
      swa_get_gre_hdrlen(p_item->tx_if, &hdrlen);
      headerlength += hdrlen;  
      *delta += hdrlen;
    }
  }
     
  if( IsValidVlanIns(p_item->flags) ) {
    *delta += VLAN_HLEN;
    headerlength += VLAN_HLEN;
  }
  if( IsValidVlanRm(p_item->flags) ) {
    *delta += -VLAN_HLEN;
  }
  if( IsPppoeSession(p_item->flags) ) {
    if( is_lan ) {
      headerlength += PPPOE_HLEN;
      *delta += PPPOE_HLEN;
    } else {
      *delta += -PPPOE_HLEN;
    }
  }

  if( IsLanSession(p_item->flags) && IsValidOutVlanIns(p_item->flags) ) {
    *delta += VLAN_HLEN;
    headerlength += VLAN_HLEN;
  }


  if ( !(p_item->flags & SESSION_TX_ITF_IPOA_PPPOA_MASK) ) {
    headerlength += ETH_HLEN;  /* mac header offset */
  }
  
  return headerlength;   
}

      

static uint32_t ppa_form_ipsec_tunnel_hdr(const struct swa_session_list_item* p_item, 
                                     uint8_t* hdr, unsigned isIPv6)
{

  struct iphdr iph;
  form_IPv4_header(&iph, p_item->src_ip.ip, p_item->dst_ip.ip, 50,20); // need to check from where can i get protocol
  swa_memcpy(hdr , &iph, sizeof(struct iphdr));
  hdr += IPV4_HDR_LEN;
}

static uint32_t ppa_form_l2tp_tunnel(const struct swa_session_list_item* p_item, 
                                     uint8_t* hdr, unsigned isIPv6)
{
  struct iphdr iph;
  struct udphdr udph;
  uint32_t outer_srcip, outer_dstip;

  // adding IP header to templet buffer
  swa_pppol2tp_get_src_addr(p_item->tx_if,&outer_srcip);
  swa_pppol2tp_get_dst_addr(p_item->tx_if,&outer_dstip);
  form_IPv4_header(&iph, outer_srcip, outer_dstip, 17,38); // need to check from where can i get protocol
  swa_memcpy(hdr , &iph, sizeof(struct iphdr));
  hdr += IPV4_HDR_LEN;

  // adding UDP header to templet buffer
  form_UDP_header(&udph, 0x06a5, 0x06a5,18);
  swa_memcpy(hdr , &udph, sizeof(struct udphdr));
  hdr += UDP_HDR_LEN;

  //adding L2TP header to templet buffer
  ppa_htons((uint16_t*)hdr, 0x0002);
  ppa_htons((uint16_t*)(hdr + 2), p_item->pppol2tp_tunnel_id); //copying l2tp tunnel_id @ appropriate offset 
  ppa_htons((uint16_t*)(hdr + 4), p_item->pppol2tp_session_id) ; //copying l2tp session_id @ appropriate offset
  hdr += L2TP_HDR_LEN ;
  //adding ppp header to templet buffer
  *(hdr) = 0xff;  
  *(hdr + 1) = 0x03;
  if(isIPv6)
  	ppa_htons((uint16_t*)(hdr + 2),  0x0057);
   else
  	ppa_htons((uint16_t*)(hdr + 2),  0x0021);

  return 38;
}

/* 
 * This function reads the necessary information for software acceleation from  packet and updates the p_item 
 */
int32_t ppa_form_session_tmpl(void *s_pkt, void *pitem, void *tx_ifinfo)
{
  int ret = SWA_SUCCESS;
  uint32_t delta = 0;
  uint16_t tlen = 0;
  uint16_t proto_type = ETH_P_IP;
  unsigned isIPv6 = 0;
  uint8_t* hdr;

  struct swa_session_list_item *p_item;
  struct session_action  swaHdr;
  struct session_action  *p_swaHdr;
        
  uint8_t  isIPv4Gre = 1; //Valid if session is GRE
  uint8_t  isGreTap = 0;  //Valid if sessions is GRE

  uint32_t mtu = 0;
  p_item = (struct swa_session_list_item *)pitem;

  /** comment to support full processing */
#if 1
  if( !IsLanSession(p_item->flags)) {
    return ret;
  }
  if (!(IsL2TPSession(p_item->flags) || swa_is_GreSession(p_item))) {
    return ret;
  }
#endif
  // if the header is already allocated return
  if(p_item->sessionAction) {
    return ret;
  }
  mtu = p_item->mtu;

  swa_memset(&swaHdr,0,sizeof(struct session_action));  
  
  
  if( IsIpv6Session(p_item->flags) ) {
    isIPv6 = 1;
    proto_type = ETH_P_IPV6;
  } 
  
  //get the actual txif
  swaHdr.dst_pmac_port_num=1;
  swaHdr.dst_pmac_port_list[0] = swa_get_phy_port(tx_ifinfo); // dp_subif.port_id;

  /* 
   * Find the length of template buffer 
   */
  tlen =  ppa_out_header_len(p_item, &delta);
        
  if( swa_is_GreSession(p_item) ) { //EoGRE
    swa_is_gre_netif_type(p_item->tx_if, &isIPv4Gre, &isGreTap);
  }
  
  if( IsBridgedSession(p_item) || isGreTap) {
    if( IsLanSession(p_item->flags) ) {
      /* UP stream handling */
      if( swa_is_GreSession(p_item) ) { //EoGRE
        uint16_t hdrlen;

        swaHdr.tunnel_type = TUNL_EOGRE;
       
        swa_get_gre_hdrlen(p_item->tx_if, &hdrlen);
        swaHdr.tunnel_ip_offset_en = 1;
        swaHdr.tunnel_ip_offset = tlen - hdrlen;
      
        delta = tlen;  /* Needs ETH header for EoGRE */
        tlen += ETH_HLEN; 
        swaHdr.in_eth_iphdr_offset_en = 1;  
        swaHdr.in_eth_iphdr_offset = tlen;  
        
        if( isIPv4Gre )
          proto_type = ETH_P_IP;
        else
          proto_type = ETH_P_IPV6;
        
        if( IsPppoeSession(p_item->flags) ) {
          swaHdr.pppoe_offset_en = 1;
          swaHdr.pppoe_offset =  swaHdr.tunnel_ip_offset - PPPOE_HLEN;
        }
      } 
    } // else  Down stream
  } else if( IsTunneledSession(p_item->flags) | swa_is_GreSession(p_item) | IsIpsecSession(p_item)) {

    if( IsLanSession(p_item->flags) ) {
        
      if( IsL2TPSession(p_item->flags)) {
        swaHdr.tunnel_type      = TUNL_L2TP;
        swaHdr.tunnel_ip_offset_en = 1;       
        swaHdr.tunnel_ip_offset     = tlen - L2TP_HLEN ;
        swaHdr.in_eth_iphdr_offset_en = 1;  
        swaHdr.in_eth_iphdr_offset = tlen;  

        
        swaHdr.tunnel_udp_offset_en = 1; 
        swaHdr.tunnel_udp_offset   = ((swaHdr.tunnel_ip_offset + IPV4_HDR_LEN) ) ;
        proto_type = ETH_P_IP;

        if( IsPppoeSession( p_item->flags))
             mtu += 48;
        else		
             mtu += 40;


      } else if (swa_is_GreSession(p_item)) {

        /*
         * Required fields for gre tunnel -
         * tunnel_type, in_eth_iphdr_offset,tunnel_ip_offset_en & corresponding 
         * flags
         */
        uint16_t hdrlen;

        swaHdr.tunnel_type = TUNL_IPOGRE ;
       
        swaHdr.in_eth_iphdr_offset_en = 1;  
        swaHdr.in_eth_iphdr_offset = tlen;  
        
        swa_get_gre_hdrlen(p_item->tx_if, &hdrlen);
        swaHdr.tunnel_ip_offset_en = 1;
        swaHdr.tunnel_ip_offset = tlen - hdrlen;

        if( isIPv4Gre )
          proto_type = ETH_P_IP;
        else
          proto_type = ETH_P_IPV6;

      } else if( IsIpsecSession(p_item)) {

        swaHdr.tunnel_type = TUNL_ESP ;
        swaHdr.tunnel_id = p_item->tunnel_idx ;
        swaHdr.tunnel_ip_offset_en = 1;
        swaHdr.tunnel_ip_offset = tlen - IPSEC_HLEN;
      }
      
      if( IsPppoeSession(p_item->flags) ) {
        swaHdr.pppoe_offset_en = 1;
        swaHdr.pppoe_offset =  swaHdr.tunnel_ip_offset - PPPOE_HLEN;
      }
    } else {
      //TODO: UP stream  - Full Processing
      swaHdr.tunnel_rm_en = 1;
      if( IsL2TPSession(p_item->flags)) {
        swaHdr.tunnel_type = TUNL_L2TP;
      } else if (swa_is_GreSession(p_item)) {
        swaHdr.tunnel_type = TUNL_IPOGRE ;
      }
    }
  } else {

    /* IPV4/IPV6 session */
    swaHdr.new_dst_ip_en = 1;
    swaHdr.new_src_ip_en = 1;

    if( IsLanSession(p_item->flags) ) {
      if( IsPppoeSession(p_item->flags) ) {
        swaHdr.pppoe_offset_en = 1;
        swaHdr.pppoe_offset =  tlen - PPPOE_HLEN;
      }
      if( isIPv6 ) {
	/** Right now MPE requires the DW in this format */
	swaHdr.new_dst_ip.ip6.ip[3] = p_item->dst_ip.ip6[0];
	swaHdr.new_dst_ip.ip6.ip[2] = p_item->dst_ip.ip6[1];
	swaHdr.new_dst_ip.ip6.ip[1] = p_item->dst_ip.ip6[2];
	swaHdr.new_dst_ip.ip6.ip[0] = p_item->dst_ip.ip6[3];

	swaHdr.new_src_ip.ip6.ip[3] = p_item->src_ip.ip6[0];
	swaHdr.new_src_ip.ip6.ip[2] = p_item->src_ip.ip6[1];
	swaHdr.new_src_ip.ip6.ip[1] = p_item->src_ip.ip6[2];
	swaHdr.new_src_ip.ip6.ip[0] = p_item->src_ip.ip6[3];

#if 0
        swa_ipv6_addr_copy( (struct ip6_addr *)swaHdr.new_src_ip.ip6.ip, 
                        (struct ip6_addr *)p_item->nat_ip.ip6);
        //swa_ipv6_addr_copy( (struct ip6_addr *)swaHdr.new_dst_ip.ip6.ip, 
          //              (struct ip6_addr *)p_item->dst_ip.ip6);
        swa_ipv6_addr_copy( (struct ip6_addr *)swaHdr.new_dst_ip.ip6.ip, 
                        (struct ip6_addr *)l_ip6_dst.ip);
#endif
      } else {
        swaHdr.new_src_ip.ip4.ip = p_item->nat_ip.ip;
        swaHdr.new_dst_ip.ip4.ip = p_item->dst_ip.ip;
      }
    } else {

      if( isIPv6 ) {
#if 1
	/** Right now MPE requires the DW in this format */
	swaHdr.new_dst_ip.ip6.ip[3] = p_item->dst_ip.ip6[0];
	swaHdr.new_dst_ip.ip6.ip[2] = p_item->dst_ip.ip6[1];
	swaHdr.new_dst_ip.ip6.ip[1] = p_item->dst_ip.ip6[2];
	swaHdr.new_dst_ip.ip6.ip[0] = p_item->dst_ip.ip6[3];

	swaHdr.new_src_ip.ip6.ip[3] = p_item->src_ip.ip6[0];
	swaHdr.new_src_ip.ip6.ip[2] = p_item->src_ip.ip6[1];
	swaHdr.new_src_ip.ip6.ip[1] = p_item->src_ip.ip6[2];
	swaHdr.new_src_ip.ip6.ip[0] = p_item->src_ip.ip6[3];
#else

        swa_ipv6_addr_copy( (struct ip6_addr *)swaHdr.new_src_ip.ip6.ip, 
                        (struct ip6_addr *)p_item->src_ip.ip6);
        swa_ipv6_addr_copy( (struct ip6_addr *)swaHdr.new_dst_ip.ip6.ip, 
                        (struct ip6_addr *)p_item->nat_ip.ip6);
#endif
      } else {
        swaHdr.new_src_ip.ip4.ip = p_item->src_ip.ip;
        swaHdr.new_dst_ip.ip4.ip = p_item->nat_ip.ip;
      }
    }
  }
  
  swaHdr.entry_vld = 1;
  swaHdr.routing_flag = 1;
  swaHdr.templ_len = tlen ;
  //swaHdr.mtu = p_item->mtu;
  swaHdr.mtu = mtu;
  swaHdr.pkt_len_delta = delta;

  //p_swaHdr = ( struct session_action *)swa_malloc(sizeof(struct session_action));
  p_swaHdr = ( struct session_action *)swa_dma(sizeof(struct session_action));
  if(p_swaHdr == NULL) {
    return SWA_ENOMEM; 
  }
  swa_memset(p_swaHdr, 0, sizeof(struct session_action));
  swa_memcpy(p_swaHdr, &swaHdr,sizeof(struct session_action));

  if( swaHdr.templ_len ) {
    //p_swaHdr->templ_buf = swa_malloc(swaHdr.templ_len);
    p_swaHdr->templ_buf = swa_dma(swaHdr.templ_len);
    swa_memset(p_swaHdr->templ_buf, 0, swaHdr.templ_len);
  } 
  /* else  TODO: Handle when no buffer need to be inserted. 
             Required mainly for downstream */
  hdr = p_swaHdr->templ_buf;

  //construct the datalink header
  if(!(p_item->flags & SESSION_TX_ITF_IPOA_PPPOA_MASK)) /* put ethernet header */
  {
    if( IsBridgedSession(p_item) || isGreTap ) {
      if( TUNL_EOGRE == swaHdr.tunnel_type ) {
        swa_memcpy(hdr, p_item->dst_mac, ETH_ALEN);
        swa_get_netif_hwaddr(p_item->tx_if, hdr + ETH_ALEN, 1);
      } else
        swa_memcpy_data(hdr, s_pkt, ETH_ALEN*2);
    } else {
      //get the MAC address of txif
      ret =  swa_get_netif_hwaddr(p_item->tx_if, hdr + ETH_ALEN, 1);
      if (ret == SWA_FAILURE){
        return ret;
      }
      swa_memcpy(hdr, p_item->dst_mac, ETH_ALEN);
    }
    hdr += ETH_ALEN*2;
    /* If the destination interface is a LAN VLAN interface under bridge the 
       below steps header is not needed */
    if( IsValidOutVlanIns(p_item->flags) && IsLanSession(p_item->flags) ) {

      ppa_htonl((uint32_t*)hdr, p_item->out_vlan_tag);
      hdr +=VLAN_HLEN;
    }

    if( IsValidVlanIns(p_item->flags) ) {

      ppa_htons((uint16_t*)hdr, ETH_P_8021Q);
      ppa_htons((uint16_t*)(hdr+2), p_item->new_vci);
      hdr +=VLAN_HLEN;
    }

    // construct pppoe header
    if( IsLanSession(p_item->flags) && IsPppoeSession(p_item->flags) ) {
      
      ppa_htons((uint16_t*)hdr, ETH_P_PPP_SES);
      hdr += 2; /* Two bytes for ETH protocol field */

      ppa_htons((uint16_t*)hdr, 0x1100);
      ppa_htons((uint16_t*)(hdr+2), p_item->pppoe_session_id); //sid
      
      /* payload length: Length of PPPoE payload in template buffer */
      ppa_htons((uint16_t*)(hdr+4), 
          (swaHdr.templ_len - (swaHdr.pppoe_offset + PPPOE_LEN)));
      
      if( ETH_P_IPV6 == proto_type ) { // ppp type ipv6
        ppa_htons((uint16_t*)(hdr+6), PPPOE_IPV6_TAG);
      } else {
        ppa_htons((uint16_t*)(hdr+6), PPPOE_IPV4_TAG);
      }
      hdr += PPPOE_HLEN;
    } else {
      ppa_htons((uint16_t*)hdr, proto_type);
      hdr += 2; /* Two bytes for ETH protocol field */
    }
  }
  
  if( IsBridgedSession(p_item) || isGreTap) {
     if(swa_is_GreSession(p_item)) {
       // Add GRE Header to buffer for EoGre
       tlen=100;
       swa_form_gre_hdr(p_item->tx_if, isIPv6, ETH_HLEN,hdr,&tlen);
       hdr += tlen;
       /* Inner MAC hdr. In future CVLAN handle CVLANs here */
       swa_memcpy_data(hdr, s_pkt, ETH_HLEN); 
     }
    goto hdr_done;
  }

  if( IsLanSession(p_item->flags) ) {
    /* Add Tunnel header here */
    if ( IsL2TPSession(p_item->flags)) {
      ppa_form_l2tp_tunnel(p_item, hdr,isIPv6);
    } else if(swa_is_GreSession(p_item)) {
      //Add GRE Header to templet buffer
      tlen=100;
      swa_form_gre_hdr(p_item->tx_if, isIPv6,0,hdr,&tlen);
    } else if(IsIpsecSession(p_item)) {
	ppa_form_ipsec_tunnel_hdr(p_item, hdr,isIPv6);
    }
  }

hdr_done:
  p_item->sessionAction = (uint32_t)p_swaHdr;
  print_action_info(p_swaHdr,isIPv6==0);
  return ret;
}

void ppa_remove_session_tmpl(void *p_item)
{
  struct session_action *s_act = 
    (struct session_action*)((struct swa_session_list_item*)p_item)->sessionAction;
  
  if(s_act) {
    swa_free(s_act->templ_buf); // free templ_buf
    swa_free(s_act); // free complete session_action
  }
}

void ppa_remove_session_mc_tmplbuf(void *sessionAction)
{
  struct session_action *s_act = 
    (struct session_action*)sessionAction;
  
  if(s_act) {
    //swa_free(s_act->templ_buf); // free templ_buf
    swa_free(s_act); // free complete session_action
  }
}


void * ppa_form_mc_tmplbuf( void *pitem, uint32_t dest_list)
{
	struct swa_mc_group_list_item *p_item;
	struct session_action *sessionAction;
	uint32_t i, k=0;
	uint8_t* Hdr;
        uint8_t mc_dst_mac[ETH_ALEN] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0x01 }; 

	p_item = (struct swa_mc_group_list_item *)pitem; 
	if (!p_item){
		dbg_print("pitem = NULL\n");
		return -1;
	}
	// filling of session Action
	sessionAction = (struct session_action *) swa_malloc(sizeof(struct session_action)); 
	swa_memset(sessionAction, 0, sizeof(struct session_action));
	for (i = 0; i < MAX_PMAC_PORT; i++)
	{
		if (dest_list & (1 << i)) {
			sessionAction->dst_pmac_port_num++;
			sessionAction->dst_pmac_port_list[k++] = i;
		}
		
	}
	sessionAction->templ_len =  ETH_HLEN;
	sessionAction->pkt_len_delta = 0 ;
	sessionAction->mtu = 1500;
	sessionAction->entry_vld = 1;
	sessionAction->mc_index = 1;
	sessionAction->mc_way = 1; //Need to enable with new firmware for workaround for VAP level multicast.

	sessionAction->templ_buf = swa_dma(sessionAction->templ_len);
    	swa_memset(sessionAction->templ_buf, 0, sessionAction->templ_len);
	// filling of templet buffer
	Hdr = sessionAction->templ_buf;
	swa_memcpy(Hdr, mc_dst_mac, ETH_ALEN);
	swa_memcpy(Hdr + ETH_ALEN, p_item->s_mac, ETH_ALEN);
	 Hdr += ETH_ALEN*2;
	ppa_htons((uint16_t*)Hdr, ETH_P_IP);

	dbg_print("Ifid=%d group_id=%d dest_subifid=%d src_port=%d dst_port=%d  dst_ip=%d.%d.%d.%d src_ip=%d.%d.%d.%d\n", p_item->dest_ifid,
	 p_item->group_id, p_item->dest_subifid, p_item->src_port, p_item->dst_port, NIPQUAD(p_item->ip_mc_group.ip), NIPQUAD(p_item->source_ip.ip));
	return sessionAction;
}
//EXPORT_SYMBOL(ppa_session_construct_mc_tmplbuf);
extern struct session_action * (*ppa_construct_mc_template_buf_hook)( void *pitem, uint32_t dest_list);
extern int32_t (*ppa_construct_template_buf_hook)(void *skb, 
                                                  void *p_item,
                                                  void *tx_ifinfo);
extern void (*ppa_destroy_template_buf_hook)(void* tmpl_buf);
extern void (*ppa_session_mc_destroy_tmplbuf_hook)(void* sessionAction);

int32_t ppa_tmplbuf_register_hooks(void)
{
  ppa_construct_template_buf_hook = ppa_form_session_tmpl;
  ppa_construct_mc_template_buf_hook = ppa_form_mc_tmplbuf;
  ppa_destroy_template_buf_hook = ppa_remove_session_tmpl;
  ppa_session_mc_destroy_tmplbuf_hook = ppa_remove_session_mc_tmplbuf;
  return SWA_SUCCESS;
}

void ppa_tmplbuf_unregister_hooks(void)
{
  ppa_construct_template_buf_hook = NULL;
  ppa_construct_mc_template_buf_hook = NULL;
  ppa_destroy_template_buf_hook = NULL;
  ppa_session_mc_destroy_tmplbuf_hook = NULL;
}
