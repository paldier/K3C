#include <linux/module.h>    
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>     
#include <linux/inet.h>

#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include <xway/switch-api/lantiq_gsw_api.h>
#include <xway/switch-api/lantiq_gsw_flow.h>

#ifndef MAX_PCE_ENTRIES
#define MAX_PCE_ENTRIES 512
#endif

#ifndef MAX_ROUTING_ENTRIES
#define MAX_ROUTING_ENTRIES 4096
#endif

extern int8_t *ppa_get_pkt_mac_string(uint8_t *mac, int8_t *strbuf);

extern uint32_t ppa_drv_generic_hal_register(uint32_t hal_id, ppa_generic_hook_t generic_hook);
extern uint32_t ppa_drv_register_cap(PPA_API_CAPS cap, uint8_t wt, PPA_HAL_ID hal_id);
extern uint32_t ppa_drv_deregister_cap(PPA_API_CAPS cap, PPA_HAL_ID hal_id);

extern ppa_tunnel_entry	    	*g_tunnel_table[MAX_TUNNEL_ENTRIES];

extern int32_t add_routing_entry(PPE_ROUTING_INFO *route);
extern int32_t add_vlan_entry(PPE_OUT_VLAN_INFO *vlan_entry);
extern int32_t add_wan_mc_entry(PPE_MC_INFO *mc_route);
extern int32_t add_bridging_entry(uint32_t port,
                           uint8_t  mac[PPA_ETH_ALEN],
                           uint32_t fid,
                           int32_t age_timer,
                           uint16_t static_entry,
                           uint16_t sub_ifid);
extern int32_t add_tunnel_entry(uint32_t tunnel_type, uint32_t tunnel_idx);

extern int32_t pae_hal_add_class_rule(PPA_CLASS_RULE*);
extern int32_t pae_hal_del_class_rule(PPA_CLASS_RULE*);
extern int32_t pae_hal_get_class_rule(PPA_CLASS_RULE*);
extern int32_t pae_hal_mod_class_rule(PPA_CLASS_RULE*);

static GSW_API_HANDLE gswr;

int add_pae_mcentry(uint8_t *srcMAC, uint8_t *dstMAC, uint8_t f_ipv6, IP_ADDR *dst_IP, IP_ADDR *src_IP, 
		    uint32_t portmap, uint16_t groupid, uint16_t subifid, uint8_t numvap)
{
    PPE_MC_INFO route;

    ppa_memset( &route, 0, sizeof(route));

    route.route_type = PPA_ROUTE_TYPE_IPV4; 
    route.f_ipv6 = f_ipv6;
   
    if(!f_ipv6) { 
	route.src_ip_compare = src_IP->ip; 
	route.dest_ip_compare = dst_IP->ip;
    } else {
	memcpy(route.src_ipv6_info.ip.ip6,src_IP->ip6, sizeof(IP_ADDR));
	memcpy(route.dst_ipv6_info.ip.ip6,dst_IP->ip6, sizeof(IP_ADDR));
    }
 
    route.dest_list = portmap;
    route.subif_id = subifid;
    route.group_id = groupid;
    route.num_vap =  numvap;  
   
    memcpy(route.src_mac,srcMAC,sizeof(srcMAC));
    
    if(add_wan_mc_entry(&route) != PPA_SUCCESS) {
	printk(KERN_INFO "add_wan_mc_entry failed!!");
	return PPA_FAILURE;	
    } else {	 
	printk(KERN_INFO "add_wan_mc_entry Success sip=%u dip=%u\n", src_IP->ip, dst_IP->ip);                                  
    }
    return route.p_entry;
}
  
int add_pae_entry(uint8_t *wanSrcMAC, uint8_t *wanDstMAC, uint8_t f_ipv6,
		  IP_ADDR *Src_IP, IP_ADDR *Dst_IP, uint8_t f_tcp, uint32_t SrcPort, uint32_t DstPort,
		  uint8_t rt_type, IP_ADDR *NATIPaddr, uint32_t TcpUdpPort,
		  uint8_t f_islan, uint32_t mtu, uint32_t portmap, uint16_t subifid,
		  uint8_t pppoe_mode, uint32_t pppoe_sessionid)
{
    int ret = PPA_SUCCESS;
    PPE_ROUTING_INFO route;

    ppa_memset( &route, 0, sizeof(route));
   
    route.route_type = rt_type;

    route.new_ip.f_ipv6 = f_ipv6; 
    memcpy(&route.new_ip.ip,NATIPaddr,sizeof(IP_ADDR)); 
    route.new_port = TcpUdpPort; 
    
    route.pppoe_mode = pppoe_mode;
    route.pppoe_info.pppoe_session_id = pppoe_sessionid;  
 
    route.mtu_info.mtu = mtu;
    
    route.dest_list = portmap; 
    route.dest_subif_id = subifid;
   
    route.f_is_lan = f_islan; 
 
    memcpy(route.src_mac.mac,wanSrcMAC,sizeof(uint8_t)*PPA_ETH_ALEN);
    memcpy(route.new_dst_mac,wanDstMAC,sizeof(uint8_t)*PPA_ETH_ALEN); 

    route.dst_port = DstPort;
    route.src_port = SrcPort;
    route.src_ip.f_ipv6 = f_ipv6;
    route.dst_ip.f_ipv6 = f_ipv6;
    memcpy(&route.src_ip.ip,Src_IP, sizeof(IP_ADDR));
    memcpy(&route.dst_ip.ip,Dst_IP, sizeof(IP_ADDR));
    route.f_is_tcp = f_tcp; 

    if((ret=add_routing_entry(&route)) < PPA_SUCCESS) {
 	printk(KERN_INFO "add_routing_entry returned Failure %d\n", ret);                                  
    }
   
    printk(KERN_INFO "add_routing_entry Success sip=%u dip=%u sp=%u dp=%u\n", Src_IP->ip, Dst_IP->ip, SrcPort, DstPort);                                  
    return ret;
}

int add_pae_tnnlentry(uint8_t *wanSrcMAC, uint8_t *wanDstMAC, uint8_t f_ipv6,
		  IP_ADDR *Src_IP, IP_ADDR *Dst_IP, uint8_t f_tcp, uint32_t SrcPort, uint32_t DstPort,
		  uint8_t rt_type, IP_ADDR *NATIPaddr, uint32_t TcpUdpPort,
		  uint8_t f_islan, uint32_t portmap, uint16_t subifid, 
		  uint16_t tunnel_index, uint32_t tunnel_type,IP_ADDR *t_srcip, IP_ADDR *t_dstip)
{
    int ret = PPA_SUCCESS;
    PPE_ROUTING_INFO route;
    ppa_tunnel_entry *t_entry=NULL;

    ppa_memset( &route, 0, sizeof(route));
   
    route.route_type = rt_type;

    route.new_ip.f_ipv6 = f_ipv6; 
    memcpy(&route.new_ip.ip,NATIPaddr,sizeof(IP_ADDR)); 
    route.new_port = TcpUdpPort; 
   
    route.dest_list = portmap; 
    route.dest_subif_id = subifid;
    route.tnnl_info.tunnel_idx = (tunnel_index > 0 ? tunnel_index - 1 : 0);
    route.tnnl_info.tunnel_type = tunnel_type;
   
    route.f_is_lan = f_islan; 
 
    memcpy(route.src_mac.mac,wanSrcMAC,sizeof(uint8_t)*PPA_ETH_ALEN);
    memcpy(route.new_dst_mac,wanDstMAC,sizeof(uint8_t)*PPA_ETH_ALEN); 

    route.dst_port = DstPort;
    route.src_port = SrcPort;
    route.src_ip.f_ipv6 = f_ipv6;
    route.dst_ip.f_ipv6 = f_ipv6;
    memcpy(&route.src_ip.ip,Src_IP, sizeof(IP_ADDR));
    memcpy(&route.dst_ip.ip,Dst_IP, sizeof(IP_ADDR));
    route.f_is_tcp = f_tcp; 

    if(tunnel_index) {
	t_entry = (ppa_tunnel_entry*) ppa_malloc (sizeof(ppa_tunnel_entry));
		
	if(tunnel_type == TUNNEL_TYPE_6RD) {
	    t_entry->tunnel_type = TUNNEL_TYPE_6RD;
		
	    t_entry->tunnel_info.ip4_hdr.saddr = t_srcip->ip;
	    t_entry->tunnel_info.ip4_hdr.daddr = t_dstip->ip;
	} else if (tunnel_type == TUNNEL_TYPE_DSLITE) {
	    t_entry->tunnel_type = TUNNEL_TYPE_DSLITE;
	    memcpy(&t_entry->tunnel_info.ip6_hdr.saddr, &t_srcip->ip6,sizeof(uint32_t)*4);
	    memcpy(&t_entry->tunnel_info.ip6_hdr.daddr, &t_dstip->ip6,sizeof(uint32_t)*4);
	}
	t_entry->hal_buffer = NULL;
	--tunnel_index;
	g_tunnel_table[tunnel_index] = t_entry;
    
	if((ret=add_tunnel_entry(tunnel_type, tunnel_index)) < PPA_SUCCESS) {
 		printk(KERN_INFO "add_tunnel_entry returned Failure %d\n", ret);                                  
    	}
    }  
 
    if((ret=add_routing_entry(&route)) < PPA_SUCCESS) {
 	printk(KERN_INFO "add_routing_entry returned Failure %d\n", ret);                                  
    }

    printk(KERN_INFO "add_routing_entry Success sip=%pI6 dip=%pI6 sp=%u dp=%u tun-src=%pI4 tun-dst=%pI4 index=%d\n", &(Src_IP->ip6), &(Dst_IP->ip6), SrcPort, DstPort, &(g_tunnel_table[tunnel_index]->tunnel_info.ip4_hdr.saddr), &(g_tunnel_table[tunnel_index]->tunnel_info.ip4_hdr.daddr), tunnel_index);

    return route.entry;
}

void pce_rule_read(int index)
{
    GSW_PCE_rule_t rule;
    int ret = 0;
    
    gswr = gsw_api_kopen("/dev/switch_api/1");
    if (gswr == 0) {
	printk(KERN_INFO "%s: Open SWAPI device FAILED !!\n", __func__ );
        return;
    }
    
    memset(&rule,0,sizeof(GSW_PCE_rule_t));
    rule.pattern.nIndex=index;

    if((ret=gsw_api_kioctl(gswr, GSW_PCE_RULE_READ, (unsigned int)&rule)) < GSW_statusOk) {
	printk(KERN_INFO "PCE rule read-%d returned failure %d\n",index,ret);
	return;	
    }
    printk(KERN_INFO "PCE rule read-%d returned success %d\n",index, ret);	

    if(rule.pattern.bEnable) {
	printk(KERN_INFO "pce rule read successfully index = %d!!",index);
    }
    
    gsw_api_kclose(gswr);
      	
}

void routing_entry_read(int index)
{
    GSW_ROUTE_Entry_t rt_entry;
    int ret = 0;
    int8_t strbuf1[24]={0},strbuf2[24]={0};

    memset(&rt_entry,0,sizeof(GSW_ROUTE_Entry_t));

    gswr = gsw_api_kopen("/dev/switch_api/1");
    if (gswr == 0) {
	printk(KERN_INFO "%s: Open SWAPI device FAILED !!\n", __func__ );
        return;
    }

    rt_entry.nRtIndex = index;
    
    if((ret=gsw_api_kioctl(gswr, GSW_ROUTE_ENTRY_READ, (unsigned int)&rt_entry)) < GSW_statusOk) {
 	printk(KERN_INFO "read_routing_eintry returned Failure %d\n", ret);                                   
	return;
    }
  
    if(rt_entry.routeEntry.pattern.bValid) 
	printk(KERN_INFO "read_routing_entry returned nRtIndex=%d\n IpType=%d\n srcip=%u\n destip=%u\n srcport=%d\n dstport=%d\n RoutExtId=%d\n \
 dest_list=%0x\n dest_subifid=%0x\n IpType=%d\n NATIPaddr=%u\n TcpUdpPort=%d\n MTU=%d\n SrcMAC=%s\n DstMAC=%s\n \
 PPPoEmode=%u\n PPPoESessionId=%u\n SessDirection=%u\n SessRoutingMode=%u\n", 
			rt_entry.nRtIndex, 
			rt_entry.routeEntry.pattern.eIpType,
			rt_entry.routeEntry.pattern.nSrcIP.nIPv4,
			rt_entry.routeEntry.pattern.nDstIP.nIPv4,
			rt_entry.routeEntry.pattern.nSrcPort,
			rt_entry.routeEntry.pattern.nDstPort,
			rt_entry.routeEntry.pattern.nRoutExtId,
			rt_entry.routeEntry.action.nDstPortMap,
			rt_entry.routeEntry.action.nDstSubIfId,
			rt_entry.routeEntry.action.eIpType,
			rt_entry.routeEntry.action.nNATIPaddr.nIPv4,
			rt_entry.routeEntry.action.nTcpUdpPort,
			rt_entry.routeEntry.action.nMTUvalue,
			ppa_get_pkt_mac_string(rt_entry.routeEntry.action.nSrcMAC,strbuf1),
			ppa_get_pkt_mac_string(rt_entry.routeEntry.action.nDstMAC,strbuf2),
			rt_entry.routeEntry.action.bPPPoEmode,
			rt_entry.routeEntry.action.nPPPoESessId,
			rt_entry.routeEntry.action.eSessDirection,
			rt_entry.routeEntry.action.eSessRoutingMode);
  
    gsw_api_kclose(gswr);
       
}

int ipv4_routing_vlan(void)
{
    int session_index = -1;

    PPE_OUT_VLAN_INFO vid;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *lanip = "192.168.1.2";
    char *brip = "192.168.1.1";
    char *wanip = "100.100.100.2";
    char *gwip = "100.100.100.1";

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, br0_IP, eth1_IP, gw_IP;

    uint32_t lanPort =5000;
    uint32_t wanPort = 80;
    uint32_t TcpUdpPort = 4000;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_NAPT; 
    uint32_t mtu = 1500;

    lanpc_IP.ip = in_aton(lanip);
    br0_IP.ip	= in_aton(brip);
    eth1_IP.ip	= in_aton(wanip);
    gw_IP.ip	= in_aton(gwip);

    pce_rule_read(16);
    
    memset(&vid, 0, sizeof(PPE_OUT_VLAN_INFO));
    vid.vlan_id = 10;
    vid.port_id = 15;
    vid.subif_id = 1;
    vid.ctag_ins = 1; 
    add_vlan_entry(&vid);

    // LAN to WAN session
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 1;
    portmap = 1 << 15;
    session_index = add_pae_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &eth1_IP, TcpUdpPort, f_islan, mtu, portmap, 1, 0, 0); 

    routing_entry_read(session_index);
    
    // WAN to LAN session
    f_islan = 0;
    portmap = 1 << 2;
    session_index = add_pae_entry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &eth1_IP, f_tcp, wanPort, TcpUdpPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, mtu, portmap, 0, 0, 0);

    routing_entry_read(session_index);
    
	
    printk(KERN_INFO "ipv4 route added.. !\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

int ipv6_routing(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *v6lan = "3ffe:507:0:1:200:86ff:fe05:80da";
    char *v6wan = "3ffe:501:410:0:2c0:dfff:fe47:33e";  

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, gw_IP;

    uint32_t lanPort =1022;
    uint32_t wanPort = 22;
    uint32_t TcpUdpPort = 0;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_IPV4; 
    uint32_t mtu = 1500;
    const char *end;

    in6_pton(v6lan,-1,(void*)&lanpc_IP.ip6,-1,&end);
    in6_pton(v6wan,-1,(void*)&gw_IP.ip6,-1,&end);
    
    // LAN to WAN session
    f_ipv6 = 1;
    f_tcp = 1;
    f_islan = 1;
    portmap = 1 << 15;
    session_index = add_pae_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &lanpc_IP, TcpUdpPort, f_islan, mtu, portmap, 0, 0, 0); 

    routing_entry_read(session_index);
    
    // WAN to LAN session
    f_islan = 0;
    portmap = 1 << 2;

    session_index = add_pae_entry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &lanpc_IP, f_tcp, wanPort, lanPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, mtu, portmap, 0, 0, 0);
    routing_entry_read(session_index);
    printk(KERN_INFO "ipv6 route added.. !\n");
	
    return 0;    // Non-zero return means that the module couldn't be loaded.
}
int pppoe_routing(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *lanip = "192.168.1.2";
    char *brip = "192.168.1.1";
    char *wanip = "100.100.100.2";
    char *gwip = "100.100.100.1";

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, br0_IP, eth1_IP, gw_IP;

    uint32_t lanPort =5000;
    uint32_t wanPort = 80;
    uint32_t TcpUdpPort = 4000;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_NAPT; 
    uint32_t mtu = 1500;

    lanpc_IP.ip = in_aton(lanip);
    br0_IP.ip	= in_aton(brip);
    eth1_IP.ip	= in_aton(wanip);
    gw_IP.ip	= in_aton(gwip);

    // LAN to WAN session
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 1;
    portmap = 0x8000;
    session_index = add_pae_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &eth1_IP, TcpUdpPort, f_islan, mtu, portmap, 0, 1, 100); 

    routing_entry_read(session_index);
    
    // WAN to LAN session
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 0;
    portmap = 0x04; // lan port 2
    session_index = add_pae_entry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &eth1_IP, f_tcp, wanPort, TcpUdpPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, mtu, portmap, 0, 1, 100);

    routing_entry_read(session_index);
	
    printk(KERN_INFO "ipv4 route added.. !\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

int ipv4_routing(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *lanip = "192.168.1.2";
    char *brip = "192.168.1.1";
    char *wanip = "100.100.100.2";
    char *gwip = "100.100.100.1";

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, br0_IP, eth1_IP, gw_IP;

    uint32_t lanPort =5000;
    uint32_t wanPort = 80;
    uint32_t TcpUdpPort = 4000;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_NAPT; 
    uint32_t mtu = 1500;

    lanpc_IP.ip = in_aton(lanip);
    br0_IP.ip	= in_aton(brip);
    eth1_IP.ip	= in_aton(wanip);
    gw_IP.ip	= in_aton(gwip);

    // LAN to WAN session
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 1;
    portmap = 0x8000;
    session_index = add_pae_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &eth1_IP, TcpUdpPort, f_islan, mtu, portmap, 0, 0, 0); 

    routing_entry_read(session_index);
    
    // WAN to LAN session
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 0;
    portmap = 0x04; // lan port 2
    session_index = add_pae_entry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &eth1_IP, f_tcp, wanPort, TcpUdpPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, mtu, portmap, 0, 0, 0);

    routing_entry_read(session_index);
	
    printk(KERN_INFO "ipv4 route added.. !\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

int multicast_routing(void)
{
    int session_index = -1;

    u8 mcMAC[6]	= {0x01, 0x00, 0x5e, 0x00, 0x00, 0x10}; // multicast mac
    u8 br0MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac

    char *mcastip = "235.0.10.10";
    char *gwip = "100.100.100.1";

    uint8_t f_ipv6;
    uint32_t portmap=0, groupid=1, numvap=0, subifid=0;
    
    IP_ADDR gw_IP, mc_IP;
    
    mc_IP.ip	= in_aton(mcastip);
    gw_IP.ip	= in_aton(gwip);

    // LAN to WAN session
    f_ipv6 = 0;
    portmap = 1;
    portmap |= (1 << 2);
    subifid = 0;
    numvap = 0;
    session_index = add_pae_mcentry(br0MAC, mcMAC, f_ipv6, &mc_IP, &gw_IP, portmap, groupid, subifid, numvap); 

    routing_entry_read(session_index);
	
    printk(KERN_INFO "multicast route added.. !\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

int unicast_bridging(void)
{
    u8 lanpcMAC1[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc1 interface mac
    u8 lanpcMAC2[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x10}; // lan pc1 interface mac
    u8 gwMAC1[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port1 mac
    u8 gwMAC2[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x15}; // smb wan port2 mac

    PPE_OUT_VLAN_INFO vid; 

    //lan PC1 MAC to be learned on port 2
    if(add_bridging_entry(2, lanpcMAC1, 0, 300, 1, 0) != PPA_SUCCESS){
	printk(KERN_INFO "Add bridging entry1 failed!\n");
    }

    //wan pc1 ip to be learned on port 15
    if(add_bridging_entry(15, gwMAC1, 0, 300, 1, 0) != PPA_SUCCESS) {
	printk(KERN_INFO "Add bridging entry2 failed!\n");
    }

    //lan pc2 MAC to be learned on port 2 fid 1
    if(add_bridging_entry(2, lanpcMAC2, 1, 300, 0, 0) != PPA_SUCCESS) {
	printk(KERN_INFO "Add bridging entry3 failed!\n");
    }

    //wanpc2 MAC to be learned on port 15 sub ifid 1
    if(add_bridging_entry(15, gwMAC2, 1, 300, 0, 1) != PPA_SUCCESS) {
	printk(KERN_INFO "Add bridging entry4 failed!\n");
    }

    memset(&vid, 0, sizeof(PPE_OUT_VLAN_INFO));
    vid.vlan_id = 20;
    vid.port_id = 15;
    vid.subif_id = 1;
    vid.ctag_ins = 1; 
    add_vlan_entry(&vid);

    return 0;
}

int tunnel_6rd(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *v6lan = "2001:67c:2158:a019::ace";
    char *v6wan = "2001:0:5ef5:79fd:380c:1d57:a601:24fa";  
    char *wanip = "100.100.100.2";
    char *gwip = "100.100.100.1";

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, gw_IP, eth1_IP, wan_IP;

    uint32_t lanPort = 53104;
    uint32_t wanPort = 13788;
    uint32_t TcpUdpPort = 0;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_IPV4; 
    const char *end;

    in6_pton(v6lan,-1,(void*)&lanpc_IP.ip6,-1,&end);
    in6_pton(v6wan,-1,(void*)&gw_IP.ip6,-1,&end);
    
    eth1_IP.ip	= in_aton(wanip);
    wan_IP.ip	= in_aton(gwip);
    
	// LAN to WAN session
    f_ipv6 = 1;
    f_tcp = 0;
    f_islan = 1;
    portmap = 1 << 15;
    session_index = add_pae_tnnlentry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &lanpc_IP, TcpUdpPort, f_islan, portmap, 0, 1, TUNNEL_TYPE_6RD, &eth1_IP, &wan_IP); 

    routing_entry_read(session_index);
/*    
    // WAN to LAN session
    f_islan = 0;
    portmap = 1 << 2;

    session_index = add_pae_tnnlentry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &lanpc_IP, f_tcp, wanPort, lanPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, portmap, 0, 0, TUNNEL_TYPE_6RD, &eth1_IP, &wan_IP);
    routing_entry_read(session_index);
*/
    printk(KERN_INFO "6RD route added.. !\n");
    printk(KERN_INFO "add_routing_entry Success sip=%pI6 dip=%pI6 sp=%u dp=%u tun-src=%pI4 tun-dst=%pI4\n", (void*)&lanpc_IP.ip6, (void*)&gw_IP.ip6, lanPort, wanPort, (void*)&eth1_IP.ip,(void*)&wan_IP.ip);                                  

    return 0;	
}

int tunnel_dslite(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *v6lan = "3100::6000";
    char *v6wan = "3100::5000";  
    char *wanip = "192.168.1.10";
    char *gwip = "192.168.1.20";

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, gw_IP, eth1_IP, wan_IP;

    uint32_t lanPort = 1000;
    uint32_t wanPort = 2000;
    uint32_t TcpUdpPort = 0;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_IPV4; 
    const char *end;

    in6_pton(v6lan,-1,(void*)&eth1_IP.ip6,-1,&end);
    in6_pton(v6wan,-1,(void*)&wan_IP.ip6,-1,&end);
    
    lanpc_IP.ip	= in_aton(wanip);
    gw_IP.ip	= in_aton(gwip);
    
    // LAN to WAN session
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 1;
    portmap = 1 << 15;
    session_index = add_pae_tnnlentry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &lanpc_IP, TcpUdpPort, f_islan, portmap, 0, 3, SESSION_TUNNEL_DSLITE, &eth1_IP, &wan_IP); 

    routing_entry_read(session_index);
    
    // WAN to LAN session
    f_islan = 0;
    portmap = 1 << 2;

    session_index = add_pae_tnnlentry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &lanpc_IP, f_tcp, wanPort, lanPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, portmap, 0, 0, SESSION_TUNNEL_DSLITE, &eth1_IP, &wan_IP);
    routing_entry_read(session_index);
    printk(KERN_INFO "DSLITE route added.. !\n");

    return 0;	
}

void read_all_entries(void)
{
    int i=0;
    
    for(i=0; i< 4095; i++)
	routing_entry_read(i);
}

void test_class_api(void)
{
    PPA_CLASS_RULE rule1, rule2;   
    ppa_memset(&rule1,0,sizeof(PPA_CLASS_RULE));
    ppa_memset(&rule2,0,sizeof(PPA_CLASS_RULE));

    rule1.in_dev = GSWR_INGRESS;
    rule1.category = CAT_TUN; 

//delete rule at index 13 "l2tp ds udp inner ipv6"
    rule1.uidx=13;

printk(KERN_INFO"read rule uid 13\n");
    if(pae_hal_get_class_rule(&rule1)!=PPA_SUCCESS) {
	printk(KERN_INFO"read rule failed\n");
    }
    
    if(pae_hal_del_class_rule(&rule1)!=PPA_SUCCESS) {
printk(KERN_INFO"del rule failed\n");
	return;
    }
printk(KERN_INFO"del rule succeeded\n\n");

    rule2.in_dev = GSWR_INGRESS;
    rule2.category = CAT_TUN; 
    rule2.uidx=13;
printk(KERN_INFO"read rule uid 13\n");
    if(pae_hal_get_class_rule(&rule2)!=PPA_SUCCESS) {
	printk(KERN_INFO"read rule failed uid 13\n");
    }

    rule2.uidx=0;
    rule2.order=10;

printk(KERN_INFO"read rule order=10\n");
    if(pae_hal_get_class_rule(&rule2)!=PPA_SUCCESS) {
	printk(KERN_INFO"read rule failed order 13\n");
    }

printk(KERN_INFO"adding the deleted rule back at order \n");
// L2TP downstream UDP inner ipv6
    
    rule1.uidx=0;
    rule1.order=7;

    if(pae_hal_add_class_rule(&rule1)!=PPA_SUCCESS) {
printk(KERN_INFO"adding the rule back at order 7\n");
	return ;
    }
printk(KERN_INFO"add rule succeeded\n\n");

    rule2.uidx=12;
    rule2.order=0;

printk(KERN_INFO"read rule uid=12\n");
    if(pae_hal_get_class_rule(&rule2)!=PPA_SUCCESS) {
	printk(KERN_INFO"read rule failed order 13\n");
    }

printk(KERN_INFO"del rule uid=12\n");
    if(pae_hal_del_class_rule(&rule2)!=PPA_SUCCESS) {
printk(KERN_INFO"del rule failed\n");
	return;
    }
printk(KERN_INFO"del rule succeeded\n\n");

printk(KERN_INFO"adding the deleted rule back at order 16\n");
// L2TP downstream UDP inner ipv6
    
    rule2.uidx=0;
    rule2.order=10;
    
    if(pae_hal_add_class_rule(&rule2)!=PPA_SUCCESS) {
printk(KERN_INFO"adding the rule back at order 10\n");
        return ;
    }
printk(KERN_INFO"add rule succeeded\n\n");
    
}

static int32_t pae_hal_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    switch (cmd)  {

    case PPA_GENERIC_HAL_INIT: 
        {
	   ppa_drv_register_cap(SESS_IPV4, 2, TMU_HAL);
	   ppa_drv_register_cap(SESS_IPV4, 4, MPE_HAL);
	   ppa_drv_register_cap(SESS_IPV4, 3, LRO_HAL);
	   return PPA_SUCCESS;
        }

    case PPA_GENERIC_HAL_EXIT: 
        {
	   ppa_drv_deregister_cap(SESS_IPV4,TMU_HAL);
	   ppa_drv_deregister_cap(SESS_IPV4,MPE_HAL);
	   ppa_drv_deregister_cap(SESS_IPV4,LRO_HAL);
	   return PPA_SUCCESS;
        }

    default:
        return PPA_FAILURE;
    }
}

static int __init dp_test_init(void)
{
    int ret=0;
    ret=ipv4_routing();

//    ret=ipv6_routing();    
//    ret=ipv4_routing_vlan();
//    ret=pppoe_routing();
//    multicast_routing();
//    unicast_bridging();
//    tunnel_6rd();	
//    tunnel_dslite();

//      read_all_entries();
//    ppa_drv_generic_hal_register(TMU_HAL, pae_hal_generic_hook);
	
//    test_class_api();

    printk(KERN_INFO "pae hal test loaded.. !\n");
    return ret;
}

static void __exit dp_test_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module.\n");
}
            
module_init(dp_test_init);
module_exit(dp_test_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lantiq co. ltd");
MODULE_DESCRIPTION("Datapath Test Kernel Module");


