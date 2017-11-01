
/******************************************************************************
**
** FILE NAME    : swa_stack_al.h
** PROJECT      : PPA
** MODULES      : Software Acceleration Stack Adaption Layer (Linux)
**
** DATE         : 13 MAR 2014
** AUTHOR       : Lantiq
** DESCRIPTION  : PPA Protocol Stack Adaption Layer (Linux) Header File
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

/*! \file ppa_stack_al.h
    \brief This file contains es.
                provide linux os depenent api for PPA to use
*/

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
typedef unsigned long long uint64_t;

typedef short  int16_t;
typedef int    int32_t;
typedef long long int64_t;

#define ENABLE_SESSION_DEBUG_FLAGS                  1
#define SKB_PRIORITY_DEBUG                          1
#define SESSION_STATISTIC_DEBUG                     1
#define CONFIG_LTQ_PPA_IF_MIB                       1

#define NULL ((void *)0)

#define SESSION_IS_TCP			0x00000004
#define SESSION_IS_IPV6			0x40000000
#define SESSION_VALID_PPPOE		0x00010000
#define SESSION_LAN_ENTRY		0x10000000
#define SESSION_VALID_VLAN_INS		0x00001000
#define SESSION_VALID_OUT_VLAN_INS	0x00004000
#define SESSION_VALID_VLAN_RM           0x00002000
#define SESSION_TX_ITF_IPOA             0x00200000
#define SESSION_TX_ITF_PPPOA            0x00400000
#define SESSION_TX_ITF_IPOA_PPPOA_MASK  (SESSION_TX_ITF_IPOA | SESSION_TX_ITF_PPPOA)
#define SESSION_CAN_NOT_ACCEL		0x00000020
#define SESSION_ADDED_IN_SW		0x80000000
#define SESSION_VALID_PPPOL2TP          0x01000000
#define SESSION_VALID_NAT_PORT		0x00000200
#define SESSION_VALID_NAT_IP            0x00000100
#define SESSION_VALID_NEW_DSCP                  0x00080000
#define SESSION_TUNNEL_6RD              0x02000000
#define SESSION_TUNNEL_DSLITE           0x04000000

/*Note, if a session cannot get hash index, it maybe ip output hook not work, or bridge mac address failed and so on*/
#define SESSION_FLAG2_HASH_INDEX_DONE          0x00000001
#define SESSION_FLAG2_ADD_HW_FAIL              0x00000002   //PPE hash full in this hash index, or IPV6 table full ,.
#define SESSION_FLAG2_HW_LOCK_FAIL             0x00000004               
#define SESSION_FLAG2_CPU_BOUND                0x00000008   //session is locally terminating                    
#define SESSION_FLAG2_BRIDGED_SESSION          0x00000010 
#define SESSION_FLAG2_GRE                      0x00000020
#define SESSION_FLAG2_VALID_IPSEC_OUTBOUND     0x00000040
#define SESSION_FLAG2_VALID_IPSEC_INBOUND      0x00000080 
#define SESSION_FLAG2_VALID_IPSEC_OUTBOUND_SA  0x00000100

#define SESSION_FLAG2_LRO                      0x00400000

#define SWA_SUCCESS 	0
#define SWA_FAILURE     (-1)
#define SWA_ENOMEM      (-12)

#define DBG_ENABLE_MASK_DEBUG_PRINT   (1 << 1)
#define FLG_PPA_PROCESSED             0x100
#define PPA_SESSION_EXISTS            1
#define NETIF_BRIDGE                  0x00000002

#define ENOMEM          12
#define VLAN_HLEN       4

/* Ethernet types */
#define ETH_P_IPV6      0x86DD
#define ETH_P_IP        0x0800
#define ETH_P_PPP_SES   0x8864
#define ETH_P_8021Q     0x8100

#define ETH_ALEN 6

/*
 *  ppa acceleration type 
 */
enum ppa_accl_type { SOFT_ACCL, HARD_ACCL};

/*
 *  ppa operation type indicator
 */
enum ppa_oper_type { ADD, DELETE};
typedef struct swa_timespec {
        long		tv_sec;                 /* seconds */
        long            tv_nsec;                /* nanoseconds */
} SWA_TIMESPEC;

typedef union {
  uint32_t ip;        /*!< the storage buffer for ipv4 */
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
  uint32_t ip6[4];    /*!< the storage buffer for ipv6 */
#endif
}SWA_IPADDR;


typedef struct swa_hlist_node {
        struct swa_hlist_node *next, **pprev;
}SWA_HLIST_NODE;

typedef struct swa_list_node {
	struct swa_list_node *next, *prev;
}SWA_LIST_NODE;

typedef void SWA_SESSION ;
typedef void SWA_NETIF ;

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR

#define MAX_RT_SESS_CAPS  3
#define MAX_MC_SESS_CAPS 2

typedef enum {
  SESS_BRIDG=1,
  SESS_IPV4,
  SESS_IPV6,
  SESS_MC_DS,
  SESS_MC_DS_VAP,
  TUNNEL_6RD,
  TUNNEL_DSLITE,
  TUNNEL_L2TP_US,
  TUNNEL_L2TP_DS,
  TUNNEL_CAPWAP_US,
  TUNNEL_CAPWAP_DS,
  TUNNEL_ENCRYPT,
  TUNNEL_DECRYPT,
  QOS_CLASSIFY,
  QOS_QUEUE,
//#if defined (QOS_AL_CONFIG) && QOS_AL_CONFIG
  Q_SCH_WFQ,
  Q_SCH_SP,
  Q_DROP_DT,
  Q_DROP_RED,
  Q_DROP_WRED,
  Q_SHAPER,
//#endif
  QOS_LAN_CLASSIFY,
  QOS_LAN_QUEUE,
  XDSL_PHY,
  MAX_CAPS
} SWA_API_CAPS;

typedef enum {
PPE_HAL=0,
PAE_HAL,
MPE_HAL,
TMU_HAL,
LRO_HAL,
DSL_HAL,
SWAC_HAL,
MAX_HAL
} SWA_HAL_ID;

typedef struct swa_hsel_cnode{
    SWA_HLIST_NODE  cap_list;
    unsigned short wt;
    SWA_HAL_ID hal_id;
    SWA_API_CAPS cap;
} SWA_HSEL_CAP_NODE;

#endif

#ifndef L2TP_CONFIG     //if not defined in kernel's .configure file, then use local's definition
#define L2TP_CONFIG               1
#endif

typedef struct {
    int counter;
} swa_automic_t;

struct swa_session_list_item {

    SWA_HLIST_NODE              hlist;
    SWA_SESSION                 *session;
    swa_automic_t               used; 
    uint32_t                    hash;
    uint16_t                    ip_proto;
    uint16_t                    ip_tos;
    SWA_IPADDR                  src_ip;
    uint16_t                    src_port;
    uint8_t                     src_mac[ETH_ALEN];
    SWA_IPADDR                  dst_ip;
    uint16_t                    dst_port;
    uint8_t                     dst_mac[ETH_ALEN];
    SWA_IPADDR                  nat_ip;         //  IP address to be replaced by NAT if NAT applies
    uint16_t                    nat_port;       //  Port to be replaced by NAT if NAT applies
    uint16_t                    num_adds;       //  Number of attempts to add session
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS) && CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS
    SWA_LIST_NODE               priority_list;
    uint16_t                    ewma_num_adds;  //  Number of attempts over which session criteria is decided
    uint64_t                    ewma_bytes;     // EWMA in bytes 
    uint64_t                    ewma_time;      // EWMA in time 
    SWA_TIMESPEC                timespent;// Time spent in EWMA window
    uint64_t                    ewma_session_bytes; // bytes accmulated by mips over a EWMA window
    uint32_t                    session_priority;   // priority of session
    uint32_t                    session_class;
    uint32_t                    mult_factor;        //introduce to calculate average data rate of session over multiple min time intervals session tried to add.
#endif
    SWA_NETIF                  *rx_if;
    SWA_NETIF                  *tx_if;
    uint32_t                    timeout;
    uint32_t                    last_hit_time;  //  Updated by bookkeeping thread
    uint32_t                    sixrd_daddr;
    uint32_t                    new_dscp;
    uint16_t                    pppoe_session_id;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    uint16_t                    pppol2tp_session_id;
    uint16_t                    pppol2tp_tunnel_id;
#endif
    uint16_t                    new_vci;
    uint32_t                    out_vlan_tag;
    uint32_t                    mtu;
    uint16_t                    dslwan_qid;
    uint16_t                    dest_ifid;

    uint32_t                    flags;          //  Internal flag : SESSION_IS_REPLY, SESSION_IS_TCP,
                                                //                  SESSION_ADDED_IN_HW, SESSION_NOT_ACCEL_FOR_MGM
                                                //                  SESSION_VALID_NAT_IP, SESSION_VALID_NAT_PORT,
                                                //                  SESSION_VALID_VLAN_INS, SESSION_VALID_VLAN_RM,
                                                //                  SESSION_VALID_OUT_VLAN_INS, SESSION_VALID_OUT_VLAN_RM,
                                                //                  SESSION_VALID_PPPOE, SESSION_VALID_NEW_SRC_MAC,
                                                //                  SESSION_VALID_MTU, SESSION_VALID_NEW_DSCP,
                                                //                  SESSION_VALID_DSLWAN_QID,
                                                //                  SESSION_TX_ITF_IPOA, SESSION_TX_ITF_PPPOA
                                                //                  SESSION_LAN_ENTRY, SESSION_WAN_ENTRY, SESSION_IS_IPV6
    uint32_t                     flag2; //SESSION_FLAG2_HASH_INDEX_DONE/SESSION_FLAG2_ADD_HW_FAIL 
#if defined(ENABLE_SESSION_DEBUG_FLAGS) && ENABLE_SESSION_DEBUG_FLAGS
    uint32_t                    debug_flags;
#endif

    uint32_t                    routing_entry;
    uint32_t                    pppoe_entry;
    uint32_t                    mtu_entry;
    uint32_t                    src_mac_entry;
    uint32_t                    out_vlan_entry;
    uint32_t                    priority;   //skb priority
    uint32_t                    mark;       //skb mark value
    uint32_t                    extmark;    //skb ext mark value. 
    uint64_t                    mips_bytes; //bytes handled by mips
    uint64_t                    acc_bytes;  //bytes handled by PPE acceleration
    uint64_t                    last_bytes; //last updated bytes handled by PPE acceleration
    uint64_t                    prev_sess_bytes; //last updated bytes. This is for PPA session management purpose
    uint64_t                    prev_clear_acc_bytes;  //last cleared bytes. We don't really clear session's acceleration mib in PPE FW, but instead, we just record the mib counter for adjustment later
    uint64_t                    prev_clear_mips_bytes;  //last cleared bytes. We don't really clear session's acceleration mib in PPE FW, but instead, we just record the mib counter for adjustment later
    uint32_t                    tunnel_idx; //tunnel idx for PPE action table
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    uint32_t                    l2tptunnel_idx; //l2tptunnel idx for PPE action table
#endif

    uint8_t                     collision_flag; // 1 mean the entry is in collsion table or no hashed table, like ASE/Danube

#if (defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB) || (defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
    SWA_NETIF                   *br_tx_if;  //record its subinterface name under bridge interface
    SWA_NETIF                   *br_rx_if;  //record its subinterface name under bridge interface
#endif

#if defined(SESSION_STATISTIC_DEBUG) && SESSION_STATISTIC_DEBUG 
    /*below variable is used for session management debugging purpose */
    uint16_t                     hash_index;
    uint16_t                     hash_table_id; /* 0-first hash table, 1 WAN */
    uint16_t                     src_ip6_index;  /* Note, 0 means not valid data. so for its correct value, it should be "real index + 1 "  */
    uint16_t                     src_ip6_psuedo_ip; 
    uint16_t                     dst_ip6_index;  /* Note, 0 means not valid data. so for its correct value, it should be "real index + 1 " */
    uint16_t                     dst_ip6_psuedo_ip;    
#endif
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    int32_t         sess_hash;
    uint16_t        dest_subifid;
    uint16_t        src_ifid;
    uint8_t         s_mac[ETH_ALEN];   /* actual source mac of packet for PAE wanport learning */
    uint32_t        cp_rt_index; /* Complimentary processing route index */
    uint32_t        sessionAction; /* Session action info */
#endif
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    uint16_t              num_caps;
    SWA_HSEL_CAP_NODE     caps_list[MAX_RT_SESS_CAPS];
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH) && CONFIG_LTQ_PPA_API_SW_FASTPATH
    void      *sah;
#endif
};

    
/*! 
    \brief SWA_MAX_MC_IFS_NUM
*/
//TBD: Kamal need to find a way to differentiate between legacy platforms and xrx500 platform 
#ifndef CONFIG_LTQ_PPA_GRX500
#define SWA_MAX_MC_IFS_NUM                      8   /*!< Maximum number of Multicast supporting interfaces */
#else
#define SWA_MAX_MC_IFS_NUM                      16   /*!< Maximum number of Multicast supporting interfaces */
#endif
typedef struct {
	uint32_t f_ipv6; /*!< flag to specify the ipv4 version: 0---IPV4, 1 -- IPV6 */
	SWA_IPADDR ip; /*!< multiple ip address format support */
}SWA_IPADDR_C;


struct swa_mc_group_list_item {
    SWA_HLIST_NODE              mc_hlist;

    SWA_IPADDR_C                ip_mc_group;
    SWA_IPADDR_C                source_ip;    /*!<  source ip address */
    uint32_t                    num_ifs;
    SWA_NETIF                  *netif[SWA_MAX_MC_IFS_NUM]; //the interface list where mc traffic will be delivered to
    uint32_t                    ttl[SWA_MAX_MC_IFS_NUM];
    uint32_t                    if_mask;
    SWA_NETIF                  *src_netif;  //the interface which received the mc streaming traffic 
    uint16_t                    new_dscp;
    uint16_t                    new_vci;
    uint32_t                    out_vlan_tag;
    uint16_t                    dslwan_qid;
    uint16_t                    dest_ifid;

    uint32_t                    flags;          //  Internal flag : SESSION_IS_REPLY,
                                                //                  SESSION_ADDED_IN_HW, SESSION_NOT_ACCEL_FOR_MGM
                                                //                  SESSION_VALID_NAT_IP, SESSION_VALID_NAT_PORT,
                                                //                  SESSION_VALID_VLAN_INS, SESSION_VALID_VLAN_RM,
                                                //                  SESSION_VALID_OUT_VLAN_INS, SESSION_VALID_OUT_VLAN_RM,
                                                //                  SESSION_VALID_PPPOE, SESSION_VALID_SRC_MAC,
                                                //                  SESSION_VALID_MTU, SESSION_VALID_NEW_DSCP,
                                                //                  SESSION_VALID_DSLWAN_QID,
                                                //                  SESSION_TX_ITF_IPOA, SESSION_TX_ITF_PPPOA
                                                //                  SESSION_LAN_ENTRY, SESSION_WAN_ENTRY
#if defined(ENABLE_SESSION_DEBUG_FLAGS) && ENABLE_SESSION_DEBUG_FLAGS
    uint32_t                    debug_flags;
#endif

    uint32_t                    mc_entry;
    uint32_t                    src_mac_entry;
    uint32_t                    out_vlan_entry;
    uint32_t            dst_ipv6_entry;
    uint32_t            src_ipv6_entry;
    uint32_t                    bridging_flag;  //  sgh add: 0 - routing mode/igmp proxy, 1 - bring mode/igmp snooping.
    uint8_t                     SSM_flag;   /*!< Set the flag if source specific forwarding is required default 0*/ 
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    uint8_t                     RTP_flag;   /*!< rtp flag */
    uint32_t                    rtp_pkt_cnt;  /*!< RTP packet mib */
    uint32_t                    rtp_seq_num;  /*!< RTP sequence number */
#endif

    uint32_t                    last_hit_time;  //  Updated by timer
    uint64_t                    acc_bytes; //bytes handled by PPE acceleration
    uint64_t                    prev_clear_acc_bytes; //
    uint32_t                    last_bytes; //last updated bytes handled by PPE acceleration

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint8_t                     num_vap;
    uint16_t                    dest_subifid;
    uint16_t            group_id;
    int32_t             sess_hash;
    uint16_t            src_port;
    uint16_t            dst_port;
    uint8_t                     s_mac[ETH_ALEN];
#endif

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    uint16_t             num_caps;
    SWA_HSEL_CAP_NODE         caps_list[MAX_MC_SESS_CAPS];
#endif
};


#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS) && CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS
extern void update_session_mgmt_stats(void *p_item, enum ppa_oper_type oper);
#endif


void swa_dump_skb(uint32_t len, void* skb1);
void *swa_malloc(uint32_t size);
void *swa_dma(uint32_t size);
signed long swa_free(void *buf);
void swa_memset(void *dst, uint32_t pad, uint32_t n);

void swa_skb_dev(void *skb, void *itf);

void swa_skb_set_extmark_prio(void *skb1,int mark, int extmark, int priority);
int  swa_skb_get_extmark(void *skb1);
void swa_skb_update_vlanprio(void *skb1,int vlan_tag);

void swa_skb_update_mark(void *skb1,int flg);

unsigned int swa_skb_len(void *skb);

void swa_skb_set_iif(void *skb, void* dstif);
void* swa_get_pkt_dev(void *skb);
void* swa_get_pkt_dst_if(void *skb);
void* swa_get_pkt_dst(void *skb, void* dstif);
void swa_dst_hold(void *dst);
void swa_skb_dst_set(void* skb, void* *dst);
int swa_isvalid_dst(void *dst1);

int swa_skb_mark(void *skb);
void swa_debug(void *str,int val);

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
unsigned char *swa_skb_data_begin(void *skb);
#endif

unsigned char *swa_skb_head(void *skb);

unsigned char *swa_skb_pull(void *skb,unsigned int len);

unsigned char *swa_skb_data(void *skb);

unsigned char *swa_skb_push(void *skb,unsigned int len);

unsigned int swa_skb_headroom(void *skb);

struct sk_buff *swa_skb_realloc_headroom(void *skb, unsigned int len);

void swa_nf_conntrack_put(void *sess);

void SWA_SKB_FREE(void *skb);

struct sock *swa_skb_sk(void *skb);

void swa_skb_set_owner_w(void *skb,void *sk);

void swa_skb_set_mac_header(void *skb, int offset);

void swa_skb_set_network_header(void *skb, int offset);

unsigned int swa_skb_network_header_len(void *skb);

void swa_skb_set_transport_header(void *skb, int offset);

unsigned char *swa_skb_network_header(void *skb);

unsigned short swa_ip_fast_csum(void *iph, unsigned int len);

unsigned char *swa_skb_transport_header(void *skb);

struct net_device *swa_get_netif(void *ifinfo);

unsigned int swa_get_flags(void *ifinfo);

int swa_get_br_dst_port(void *netif, void *skb, void **p_netif);

void swa_memcpy_data(void *dst, void *src, unsigned long offset);

void swa_memcpy(void *dst, void *src, unsigned long offset);

int swa_get_netif_hwaddr(void *tx_if, unsigned char *hdr, unsigned long i);

void swa_inet_proto_csum_replace4_u(void *hdr, void *skb, unsigned int from, unsigned int to);

void swa_inet_proto_csum_replace4_t(void *hdr, void *skb, unsigned int from, unsigned int to);

void swa_inet_proto_csum_replace2_u(void *hdr, void *skb, unsigned short from, unsigned short to);

void swa_inet_proto_csum_replace2_t(void *hdr, void *skb, unsigned short from, unsigned short to);

int swa_get_time_in_sec(void);

int swa_dev_queue_xmit(void *skb);
#if defined(CONFIG_LTQ_PPA_TCP_LITEPATH) && CONFIG_LTQ_PPA_TCP_LITEPATH
int sw_litepath_tcp_recv_skb(void *skb);
int sw_update_iph(void *skb1, int* offset, unsigned char* pf);
#endif

int sw_get_dslite_tunnel_header(void *net_dev, void *p_ipv6hdr);
int sw_get_6rd_tunnel_header(void *net_dev, void *p_iph);

int swa_get_session_from_skb(void *skb, unsigned char pf, struct swa_session_list_item **pp_item);
void swa_put_session(void* p_item);
void swa_skb_set_len(void *skb, unsigned short len);

void swa_reset_vlantci(void *skb);

#ifdef CONFIG_NET_CLS_ACT
void *swa_handle_ing(void *skb);
int swa_check_ingress(void *vskb);
#endif

#if defined(SKB_PRIORITY_DEBUG) && SKB_PRIORITY_DEBUG
void swa_set_extmark(void *skb, int extmark);
#endif

uint32_t swa_get_phy_port(void *ifinfo);
uint32_t swa_is_GreSession(void* p_item);
int32_t  swa_get_gre_hdrlen(struct net_device* dev, uint16_t *hdrlen);
int32_t  swa_form_gre_hdr(void* dev, uint8_t isIPv6, uint16_t dataLen, uint8_t *pHdr, uint16_t* len);
int32_t  swa_pppol2tp_get_src_addr(void* dev, uint32_t *outer_srcip);
int32_t  swa_pppol2tp_get_dst_addr(void  *dev, uint32_t *outer_srcip);
uint32_t swa_is_gre_netif_type(void* dev, uint8_t* isIPv4Gre, uint8_t* isGreTap);

#if defined(CONFIG_LTQ_PPA_LRO) && CONFIG_LTQ_PPA_LRO 
int32_t swa_lro_entry_criteria(void* p_item1, void *ppa_buf);
int32_t swa_add_lro_entry(void* p_item1);
#endif
