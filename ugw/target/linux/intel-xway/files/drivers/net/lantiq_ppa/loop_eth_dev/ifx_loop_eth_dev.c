/******************************************************************************
**
** FILE NAME    : ifx_loop_eth_dev.c
** PROJECT      : UEIP
** MODULES      : ETH Loopback Device
**
** DATE         : 16 Oct 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : ETH Loopback Device source file
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 16 Oct 2009  Xu Liang        Init Version
*******************************************************************************/



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/atmdev.h>
#include <linux/init.h>
#include <linux/etherdevice.h>  /*  eth_type_trans  */
#include <linux/ethtool.h>      /*  ethtool_cmd     */
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/icmp.h>
#include <net/tcp.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <asm/checksum.h>
#include <linux/errno.h>
#ifdef CONFIG_XFRM
  #include <net/xfrm.h>
#endif

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
  #include <net/ifx_ppa_api.h>
  #include <net/ifx_ppa_api_directpath.h>
#endif



/*
 * ####################################
 *              Definition
 * ####################################
 */

#define ENABLE_DEBUG                            1

#define ENABLE_ASSERT                           1

#define DEBUG_DUMP_SKB                          1

#if defined(ENABLE_DEBUG) && ENABLE_DEBUG
  #define ENABLE_DEBUG_PRINT                    1
  #define DISABLE_INLINE                        1
#else
  #define ENABLE_DEBUG_PRINT                    0
  #define DISABLE_INLINE                        0
#endif

#if !defined(DISABLE_INLINE) || !DISABLE_INLINE
  #define INLINE                                inline
#else
  #define INLINE
#endif

#define err(format, arg...)                     do { printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )

#if defined(ENABLE_DEBUG_PRINT) && ENABLE_DEBUG_PRINT
  #undef  dbg
  #define dbg(format, arg...)                   do { if ( (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT) ) printk(KERN_WARNING __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #if !defined(dbg)
    #define dbg(format, arg...)
  #endif
#endif

#if defined(ENABLE_ASSERT) && ENABLE_ASSERT
  #define ASSERT(cond, format, arg...)      do { if ( (g_dbg_enable & DBG_ENABLE_MASK_ASSERT) && !(cond) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define ASSERT(cond, format, arg...)
#endif

#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  #define DUMP_SKB_LEN                          ~0
#endif

#if (defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB)                     \
    || (defined(ENABLE_DEBUG_PRINT) && ENABLE_DEBUG_PRINT)          \
    || (defined(ENABLE_ASSERT) && ENABLE_ASSERT)
  #define ENABLE_DBG_PROC                       1
#else
  #define ENABLE_DBG_PROC                       0
#endif

#define ENABLE_TXDBG							1

/*
 *  Debug Print Mask
 */
#define DBG_ENABLE_MASK_ERR                     (1 << 0)
#define DBG_ENABLE_MASK_DEBUG_PRINT             (1 << 1)
#define DBG_ENABLE_MASK_ASSERT                  (1 << 2)
#define DBG_ENABLE_MASK_DUMP_SKB_RX             (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX             (1 << 9)
#define DBG_ENABLE_MASK_ALL                     (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT \
                                                | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX)

/*
 *  Constant Definition
 */
#define ETH_WATCHDOG_TIMEOUT                    (10 * HZ)
#define MAX_RX_QUEUE_LENGTH                     100
#define TASKLET_HANDLE_BUDGET                   25

/*
 *  Ethernet Frame Definitions
 */
#define ETH_CRC_LENGTH                          4
#define ETH_MAX_DATA_LENGTH                     ETH_DATA_LEN
#define ETH_MIN_TX_PACKET_LENGTH                ETH_ZLEN

/*
 *  helper macro
 */
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))

/*
 *  printk color
 */
#define BLINK   "\033[31;1m"
#define RED     "\033[31;1m"
#define YELLOW  "\033[33;1m"
#define GREEN   "\033[32;2m"
#define BLUE    "\033[34;1m"
#define CYAN    "\033[36;2m"
#define DIM     "\033[37;1m"
#define NORMAL  "\033[0m"



/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *  Internal Structure of Devices (ETH/ATM)
 */
struct loop_eth_priv_data {
    int                             id;
    struct  net_device_stats        stats;
    unsigned int                    rx_preprocess_drop;
    unsigned char                   ip_templ[4];
    struct sk_buff_head             rx_queue;
    struct tasklet_struct           rx_tasklet;
    int                             f_tx_queue_stopped;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    unsigned char		            dev_addr[MAX_ADDR_LEN];
#endif

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
    int                             dp_if_id;
    unsigned int                    dp_pkts_to_ppe;
    unsigned int                    dp_pkts_to_ppe_fail;
    unsigned int                    dp_pkts_from_ppe;
    unsigned int                    dp_pkts_tx;
#endif
};



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  Wrapper for Different Kernel Version
 */
static inline struct net_device *ifx_dev_get_by_name(const char *name)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    return dev_get_by_name(&init_net, name);
#else
    return dev_get_by_name(name);
#endif
}

static inline unsigned long ifx_get_xmit_fn(struct net_device *dev)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    return (unsigned long)dev->netdev_ops->ndo_start_xmit;
#else
    return (unsigned long)dev->hard_start_xmit;
#endif
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
int strncasecmp(const char *s1, const char *s2, size_t n)
{
	int c1, c2;

	do {
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while ((--n > 0) && c1 == c2 && c1 != 0);
	return c1 - c2;
}
#endif

/*
 *  Network Operations
 */
static void eth_setup(struct net_device *);
static struct net_device_stats *eth_get_stats(struct net_device *);
static int eth_open(struct net_device *);
static int eth_stop(struct net_device *);
static int eth_hard_start_xmit(struct sk_buff *, struct net_device *);
static int eth_ioctl(struct net_device *, struct ifreq *, int);
static void eth_tx_timeout(struct net_device *);

/*
 *  skb management functions
 */
static struct sk_buff* skb_break_away_from_protocol(struct sk_buff *);

/*
 *  RX path functions
 */
static INLINE int eth_rx_preprocess(struct sk_buff *, int);
static INLINE void eth_rx_handler(struct sk_buff *, int);
static void do_loop_eth_rx_tasklet(unsigned long);

/*
 *  PPA directpath functions
 */
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
  static int32_t ppa_fp_stop_tx(struct net_device *);
  static int32_t ppa_fp_restart_tx(struct net_device *);
  static int32_t ppa_fp_rx(struct net_device *, struct net_device *, struct sk_buff *, int32_t);
#endif

/*
 *  Proc File
 */
static INLINE void proc_file_create(void);
static INLINE void proc_file_delete(void);
#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static int proc_read_dbg(char *, char **, off_t, int, int *, void *);
  static int proc_write_dbg(struct file *, const char *, unsigned long, void *);
#endif
static int proc_read_dev(char *, char **, off_t, int, int *, void *);
static int proc_write_dev(struct file *, const char *, unsigned long, void *);
static int proc_read_ip(char *, char **, off_t, int, int *, void *);
static int proc_write_ip(struct file *, const char *, unsigned long, void *);
static int proc_read_mib(char *, char **, off_t, int, int *, void *);
static int proc_write_mib(struct file *, const char *, unsigned long, void *);
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
  static int proc_read_directpath(char *, char **, off_t, int, int *, void *);
  static int proc_write_directpath(struct file *, const char *, unsigned long, void *);
#endif

#if defined(ENABLE_TXDBG) && ENABLE_TXDBG
static int proc_read_cputx(char *page, char **start, off_t off, int count, int *eof, void *data);
static int proc_write_cputx(struct file *file, const char *buf, unsigned long count, void *data);

#endif
/*
 *  Proc File help functions
 */
static INLINE int stricmp(const char *, const char *);
static INLINE int strincmp(const char *, const char *, int);

/*
 *  Debug functions
 */
#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  static INLINE void dump_skb(struct sk_buff *, u32, char *, int, int);
#else
  static INLINE void dump_skb(struct sk_buff *skb, u32 len, char *title, int ch, int is_tx) {}
#endif



/*
 * ####################################
 *            Local Variable
 * ####################################
 */

static struct net_device *g_loop_eth_dev[8] = {0};

static struct proc_dir_entry *g_loop_eth_dev_proc_dir = NULL;

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
  static int g_dbg_enable = DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_ASSERT;
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static const struct net_device_ops loop_netdev_ops = {
	.ndo_open		 = eth_open,
	.ndo_stop		 = eth_stop,
	.ndo_start_xmit	 = eth_hard_start_xmit,
	.ndo_do_ioctl    = eth_ioctl,
	.ndo_tx_timeout  = eth_tx_timeout,
    .ndo_get_stats   = eth_get_stats,
    .ndo_set_mac_address    = eth_mac_addr,
    .ndo_change_mtu         = eth_change_mtu,
};

#endif


#if defined(ENABLE_TXDBG) && ENABLE_TXDBG
//typedef struct mutex          PPA_LOCK;

#define MAX_SKB_SIZE 		1600
#define MAX_PKT_SIZE 		1500
#define MIN_PKT_SIZE		60
#define DEFAULT_PKT_SIZE	128
#define MAX_PAUSE_CNT		50
#define DEFAULT_PAUSE_CNT   20

enum{
	TX_STOP = 0,
	TX_START = 1
};

static uint32_t g_tx_count = 1;
static uint32_t g_tx_start = TX_STOP;
static PPA_LOCK g_tx_start_lock;
static uint32_t g_tx_mib = 0;
static struct sk_buff *g_tx_skb = NULL;
static uint32_t g_pkt_size = DEFAULT_PKT_SIZE;
static char g_tx_dev[IFNAMSIZ]={0};
static struct task_struct *g_tx_ts = NULL;
static uint32_t g_tx_pause_cnt = 20;

#endif


/*
 * ####################################
 *           Global Variable
 * ####################################
 */



/*
 * ####################################
 *            Local Function
 * ####################################
 */

static void eth_setup(struct net_device *dev)
{
    int id = -1;
    struct loop_eth_priv_data *priv = netdev_priv(dev);
    int i;

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( !g_loop_eth_dev[i] )
        {
            id = i;
            break;
        }
    if ( id < 0 )
        return;

    ether_setup(dev);   /*  assign some members */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    /*  hook network operations */
    dev->get_stats       = eth_get_stats;
    dev->open            = eth_open;
    dev->stop            = eth_stop;
    dev->hard_start_xmit = eth_hard_start_xmit;
    dev->do_ioctl        = eth_ioctl;
    dev->tx_timeout      = eth_tx_timeout;
#else
    dev->netdev_ops = &loop_netdev_ops;
#endif
    dev->watchdog_timeo  = ETH_WATCHDOG_TIMEOUT;
    dev->dev_addr[0] = 0x00;
    dev->dev_addr[1] = 0x20;
    dev->dev_addr[2] = 0xda;
    dev->dev_addr[3] = 0x86;
    dev->dev_addr[4] = 0x23;
    dev->dev_addr[5] = 0x70 + id;

    priv->id = id;
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
    priv->dp_if_id = -1;
#endif
    skb_queue_head_init(&priv->rx_queue);
    tasklet_init(&priv->rx_tasklet, do_loop_eth_rx_tasklet, id);

    return;
}

static struct net_device_stats *eth_get_stats(struct net_device *dev)
{
    struct loop_eth_priv_data *priv = netdev_priv(dev);

    return &priv->stats;
}

static int eth_open(struct net_device *dev)
{
    dbg("open %s", dev->name);

    netif_start_queue(dev);

    return 0;
}

static int eth_stop(struct net_device *dev)
{
    netif_stop_queue(dev);

    return 0;
}

static int eth_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    unsigned long sysflag;
    struct loop_eth_priv_data *priv = netdev_priv(dev);
    int rx_queue_len;
    struct sk_buff *old_skb = skb;
    
    skb = skb_break_away_from_protocol(skb);
    dev_kfree_skb_any(old_skb);

    dump_skb(skb, DUMP_SKB_LEN, "eth_hard_start_xmit", 0, 1);

    ASSERT(skb->prev == NULL && skb->next == NULL, "skb on list: prev = 0x%08x, next = 0x%08x", (unsigned int)skb->prev, (unsigned int)skb->next);

    skb->dev = g_loop_eth_dev[priv->id];

    spin_lock_irqsave(&priv->rx_queue.lock, sysflag);
    if ( (rx_queue_len = skb_queue_len(&priv->rx_queue)) < MAX_RX_QUEUE_LENGTH )
    {
        __skb_queue_tail(&priv->rx_queue, skb);
        if ( rx_queue_len == 0 )
            tasklet_schedule(&priv->rx_tasklet);
        if ( skb_queue_len(&priv->rx_queue) >= MAX_RX_QUEUE_LENGTH )
        {
            netif_stop_queue(g_loop_eth_dev[priv->id]);
        }

        priv->stats.tx_packets++;
        priv->stats.tx_bytes += skb->len;
    }
    else
    {
        dev_kfree_skb_any(skb);
        priv->stats.tx_dropped++;
    }
    spin_unlock_irqrestore(&priv->rx_queue.lock, sysflag);

    return 0;
}

static int eth_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    switch ( cmd )
    {
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static void eth_tx_timeout(struct net_device *dev)
{
    //TODO:must restart the TX channels
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
//    int sysflag;
#endif

    struct loop_eth_priv_data *priv = netdev_priv(dev);

    priv->stats.tx_errors++;

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
//    spin_lock_irqsave(&priv->rx_queue.lock, sysflag);
//    if ( priv->f_tx_queue_stopped && ppe_app_queue_on(3, 1) == 0 )
//        priv->f_tx_queue_stopped = 0;
//    spin_unlock_irqrestore(&priv->rx_queue.lock, sysflag);
#endif
    netif_wake_queue(dev);

    return;
}

static struct sk_buff* skb_break_away_from_protocol(struct sk_buff *skb)
{
    struct sk_buff *new_skb;

    if ( skb_shared(skb) ) {
        new_skb = skb_clone(skb, GFP_ATOMIC);
        if ( new_skb == NULL )
            return NULL;
    }
    else
        new_skb = skb_get(skb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dst_release(new_skb->dst);
    new_skb->dst = NULL;
#else
    skb_dst_drop(new_skb);
#endif
#ifdef CONFIG_XFRM
	secpath_put(new_skb->sp);
	new_skb->sp = NULL;
#endif
#if defined(CONFIG_NETFILTER) || LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	nf_conntrack_put(new_skb->nfct);
	new_skb->nfct = NULL;
  #if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	nf_conntrack_put_reasm(new_skb->nfct_reasm);
	new_skb->nfct_reasm = NULL;
  #endif
  #ifdef CONFIG_BRIDGE_NETFILTER
	nf_bridge_put(new_skb->nf_bridge);
	new_skb->nf_bridge = NULL;
  #endif
#endif

    return new_skb;
}

/* Ethernet frame types according to RFC 2516 */
#define ETH_PPPOE_DISCOVERY 0x8863
#define ETH_PPPOE_SESSION   0x8864

/* A PPPoE Packet, including Ethernet headers */
typedef struct PPPoEPacketStruct {
#ifdef PACK_BITFIELDS_REVERSED
    unsigned int type:4;	/* PPPoE Type (must be 1) */
    unsigned int ver:4;/* PPPoE Version (must be 1) */
#else
    unsigned int ver:4;/* PPPoE Version (must be 1) */
    unsigned int type:4;/* PPPoE Type (must be 1) */
#endif
    unsigned int code:8;/* PPPoE code */
    unsigned int session:16;/* PPPoE session */
    unsigned int length:16;/* Payload length */
    unsigned char payload[ETH_DATA_LEN]; /* A bit of room to spare, here just for space holder only */
} PPPoEPacket;
/* PPPoE Tag */

unsigned char ppp_ipv4_proto[2]={0x00, 0x21};
unsigned char ppp_ipv6_proto[2]={0x00, 0x57};
#define VLAN_HEAD_SIZE  4
#define PPPOE_HEAD_SIZE  8


static INLINE int eth_rx_preprocess(struct sk_buff *skb, int id)
{
    struct loop_eth_priv_data *priv = netdev_priv(g_loop_eth_dev[id]);
    unsigned char *p = skb->data;
    unsigned char mac[6];
    unsigned char ip[4];
    unsigned char port[2];
    struct iphdr *iph;
    struct icmphdr *icmph;
	struct tcphdr *tcph;
	uint32_t t, off_t, *opt;
    int csum;
    PPPoEPacket *pppoe;
    int offset=0;
    int vlan_num=0;
    unsigned char *p_new_src_mac;
    static unsigned char zero_mac[4]={0};

    //skb->dst = NULL;
    dump_skb(skb, DUMP_SKB_LEN, "eth_rx_preprocess", id, 0);
    if ( p[offset+12] == 0x81 && p[offset+13] == 0x00 ) //VLAN header
    {
          offset+= VLAN_HEAD_SIZE;
          vlan_num++;
          dbg("Found VLAN%d\n", vlan_num );
    }
    
    if ( p[offset+12] == 0x88 && p[offset+13] == 0x63 ) //pppoe Discover(0x9)/Offer(0x7)/request(0x19)/Confirm(0x65)
    {       
        return 0;
    }
	
    if ( p[offset+12] == 0x88 && p[offset+13] == 0x64 ) //ppp
    {        
        pppoe =(PPPoEPacket *) (p+offset+14);
		if ( ( pppoe->payload[0 ] == ppp_ipv4_proto[0]  && pppoe->payload[1] == ppp_ipv4_proto[1] ) /*  PPP IPv4*/ ||
             ( pppoe->payload[0 ] == ppp_ipv6_proto[0]  && pppoe->payload[1] == ppp_ipv6_proto[1] ) /*  PPP IPv6*/ )
        {      
            offset += PPPOE_HEAD_SIZE; // skip 8 bytes ppp header
            dbg("Found PPP IP packet\n"); 
        }
        else
        {
             return 0;
        }
    }
	
	//swap dst/src mac address
    memcpy(mac, p, 6);
    memcpy(p, p+6, 6);
    memcpy(p+6, mac,6);

    p_new_src_mac = p + 6;
    p += offset;  //Note, now p[12~13] points to protocol
       
    if ( p[12] == 0x08 && p[13] == 0x06 )
    {
        //  arp
        if ( p[14] == 0x00 && p[15] == 0x01 && p[16] == 0x08 && p[17] == 0x00 && p[20] == 0x00 && p[21] == 0x01 )
        {
            dbg("arp request:%d.%d.%d.%d\n",  p[38],  p[39],  p[40], p[41]);
            //  Ethernet IP - 10.10.xx.xx
            if ( (p[38] == priv->ip_templ[0] && p[39] == priv->ip_templ[1]) || memcmp(priv->ip_templ, zero_mac, sizeof(priv->ip_templ))== 0  )
            {
                 //fill in spoof mac address
                memcpy(p_new_src_mac, g_loop_eth_dev[id]->dev_addr, 4);
                p[8]  = p[38];
                p[9]  = p[39];
                p[10] = p[40];
                p[11] = p[41];
                //  arp reply
                p[21] = 0x02;
                //  sender mac

                memcpy( mac, p+22, 6); //save orignal sender mac
                memcpy(p + 22, p_new_src_mac, 6); //set new sender mac
                //  sender IP
                memcpy(ip, p + 28, 4);  //save original sender ip address
                memcpy(p + 28, p + 38, 4); //set new sender ip address
                //  target mac
                memcpy(p + 32, mac, 6);
                //  target IP
                memcpy(p + 38, ip, 4);
                return 1;
            }
        }
    }
    else if ( (p[12] == 0x08 && p[13] == 0x00 )/*Normal IPV4*/ || 
              (p[12] == ppp_ipv4_proto[0] && p[13] == ppp_ipv4_proto[1] )/*PPP IPV4*/||
              (p[12] == ppp_ipv6_proto[0] && p[13] == ppp_ipv6_proto[1] ) /*PPP IPV6*/ )
    {
        //  IP
        switch ( (int)p[23] )
        {
        case 0x01:
            //  ICMP - request
            if ( p[34] == 0x08 )
            {               
                //  src IP
                memcpy(ip, p + 26, 4);
                memcpy(p + 26, p + 30, 4);
                //  dest IP
                memcpy(p + 30, ip, 4);
                //  ICMP reply
                p[34] = 0x00;
                //  IP checksum
                iph = (struct iphdr *)(p + 14);
                iph->check = 0;
                iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
                //  ICMP checksum
                icmph = (struct icmphdr *)(p + 34);
                icmph->checksum = 0;
                csum = csum_partial((unsigned char *)icmph, skb->len - 34, 0);
                icmph->checksum = csum_fold(csum);
#if 0
                atomic_inc(&skb->users);
                skb->dev = ifx_dev_get_by_name("eth1");
                dev_put(skb->dev);
                dev_queue_xmit(skb);
                return 0;
#endif
                return 1;
            }
            break;
        case 0x11:
            //  UDP
        case 0x06:
            //  TCP            
            /*swap src/dst ip */
            //  src IP
            memcpy(ip, p + 26, 4);
            memcpy(p + 26, p + 30, 4);
            //  dest IP
            memcpy(p + 30, ip, 4);
            /*shaoguoh remove below checksum item since we just swap ip and port only */
#if 0
            //  IP checksum
            iph = (struct iphdr *)(p + 14);
            iph->check = 0;
            iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);
            //  no UDP checksum
            p[40] = p[41] = 0x00;
#endif
            //shaoguoh add below to swap src/dst port 34~35 36~37
            //save src port to port array and copy original dest port to new src port
            memcpy(port, p + 34, 2);
            memcpy(p + 34, p + 36, 2);
            //  copy original src port to dest port
            memcpy(p + 36, port, 2);

			//return if UDP
			if((int)p[23] == 0x11)
				return 1;

			iph = (struct iphdr *)(p + 14);
			tcph = (struct tcphdr *)(p + 34);

			if(tcph->syn == 1){//set syn & ack, set seq NO same as the incoming syn TCP packet, set ack seq NO as seq NO + 1
				tcph->ack = 1;
				tcph->ack_seq = tcph->seq + 1;
				
		    }else if(tcph->fin == 1){//set fin & ack
		        tcph->ack = 1;
				t = tcph->ack_seq;
				tcph->ack_seq = tcph->seq + 1;
				tcph->seq = t;
		    }else if(tcph->rst == 1 || (tcph->psh == 0 && tcph->ack == 1)){//rest or only ack, we ignore it.
		        return 0;
		    }else if(tcph->psh == 1){
		    	t = tcph->ack_seq;
				if(iph->tot_len < 40){//corrupt packet, ignore it.
				    return -1;
				}
				tcph->ack_seq = tcph->seq + iph->tot_len - (iph->ihl * 4) - (tcph->doff * 4);
				tcph->seq = t;
		    }
			
			//check timestamp
			off_t = 14 + 20 + 20; //mac + ip + tcp
			//printk("tcp head len: %d\n", tcph->doff);
			while((tcph->doff << 2) > (off_t - 34)){//tcp option compare tcp header length
			    //printk("tcp option: %d\n", p[off_t]);
			    switch(p[off_t]){
					case 0x0: //Option End
						break;
					case 0x1: // NO Operation
						off_t += 1;
						continue;
					case 0x2: //Max Segment Size
						off_t += 4;
						continue;
					case 0x3: // Window Scale
						off_t += 3;
						continue;
					case 0x4: //TCP Sack permitted
						off_t += 2;
						continue;
					case 0x8: //TCP timestamp
		      #if 1
					    opt = (uint32_t *)(p + off_t + 2);						
						*(opt + 1) = htons(tcp_time_stamp);
						t = *opt;
						*opt = *(opt + 1);
						*(opt + 1) = t;
						//memcpy(ip,p+off_t+2, 4);
						//memcpy(p+off_t+2, p+off_t+6,4);
						//memcpy(p+off_t+6, ip, 4);
						//printk("swap tcp time stamp\n");
			  #else
						for(t = 0; t < 10; t ++)
			  				*(p + off_t + t) = 1;
			  #endif
						off_t += 10; //option max is 64-20
						continue;
					default:
						off_t += 64;
						break;
			    }
			    
			}
			
			
			//  IP checksum
            iph = (struct iphdr *)(p + 14);
            iph->check = 0;
            iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

			// TCP checksum
			tcph->check = 0;
			t = iph->tot_len - (iph->ihl * 4);
			//tcph->check = csum_partial((unsigned char *)tcph, iph->tot_len - 20, 0);
			tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, t, IPPROTO_TCP, 
			              csum_partial(tcph,t, 0));
#if 0
			printk("src ip: %d.%d.%d.%d, dst ip: %d.%d.%d.%d, tcp_len: %d, checksum:0x%x\n",
				(iph->saddr >> 24) & 0xFF, (iph->saddr & 0xFF0000) >> 16, (iph->saddr & 0xFF00)>>8, iph->saddr & 0xFF,
				(iph->daddr >> 24) & 0xFF, (iph->daddr & 0xFF0000) >> 16, (iph->daddr & 0xFF00)>>8, iph->daddr & 0xFF,
				t,tcph->check);

			printk("tcp data: 0x%x, 0x%x,0x%x, 0x%x\n", 
				*((char*)tcph),*((char*)tcph + 1),*((char*)tcph + 2),*((char*)tcph + 3));
#endif
            return 1;
        }
    }

    return 0;
}

static INLINE void eth_rx_handler(struct sk_buff *skb, int id)
{
    struct loop_eth_priv_data *priv = netdev_priv(g_loop_eth_dev[id]);
    int pktlen;

    dump_skb(skb, DUMP_SKB_LEN, "eth_rx_handler", 0, 0);

    if ( !netif_running(g_loop_eth_dev[id]) )
    {
        dev_kfree_skb_any(skb);
        priv->stats.rx_dropped++;
        return;
    }

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
    if ( priv->dp_if_id >= 0 )
    {
        if ( ppa_hook_directpath_send_fn == NULL )
        {
            dev_kfree_skb_any(skb);
            priv->stats.rx_dropped++;
            return;
        }
        else if ( ppa_hook_directpath_send_fn(priv->dp_if_id, skb, skb->len, 0) == IFX_SUCCESS )
        {
            priv->dp_pkts_to_ppe++;
            return;
        }
        else{
            priv->dp_pkts_to_ppe_fail++;
            return;
        }
    }
#endif

    pktlen = skb->len;
    skb->dev = g_loop_eth_dev[id];
    skb->protocol = eth_type_trans(skb, g_loop_eth_dev[id]);

    if ( netif_rx(skb) == NET_RX_DROP )
        priv->stats.rx_dropped++;
    else
    {
        priv->stats.rx_packets++;
        priv->stats.rx_bytes += pktlen;
    }
}

static void do_loop_eth_rx_tasklet(unsigned long id)
{
    struct loop_eth_priv_data *priv = netdev_priv(g_loop_eth_dev[id]);
    struct sk_buff *skb;
    int i = 0;

    while ( 1 )
    {
        if ( i >= TASKLET_HANDLE_BUDGET )
        {
            tasklet_schedule(&priv->rx_tasklet);
            break;
        }

        skb = skb_dequeue(&priv->rx_queue);
        if ( !skb )
            break;

        netif_wake_queue(g_loop_eth_dev[id]);
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
//        if ( priv->f_tx_queue_stopped && ppe_app_queue_on(3, 1) == 0 )
//            priv->f_tx_queue_stopped = 0;
#endif

        if ( eth_rx_preprocess(skb, (int)id) )
            eth_rx_handler(skb, (int)id);
        else
        {
            priv->rx_preprocess_drop++;
            dev_kfree_skb_any(skb);
        }

        i++;
    }
}

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
//static int ppe_fp_xmit(struct sk_buff *skb)
//{
//    skb->dev = &g_loop_eth_dev[0];
//    return dev_queue_xmit(skb);
//}
//
//static int ppe_fp_push(struct sk_buff *skb)
//{
//    eth_rx_handler(skb);
//    return 0;
//}
//
//static int ppe_fp_device_on(int f_on)
//{
//    int ret = -EIO;
//    int sysflag;
//
//    if ( f_on )
//    {
//        spin_lock_irqsave(&g_loop_eth_rx_queue_spinlock, sysflag);
//        if ( g_loop_eth_rx_queue_stop )
//        {
//            g_loop_eth_rx_queue_stop = 0;
//            if ( g_loop_eth_rx_queue )
//                tasklet_schedule(&g_loop_eth_rx_tasklet);
//            ret = 0;
//        }
//        spin_unlock_irqrestore(&g_loop_eth_rx_queue_spinlock, sysflag);
//    }
//    else
//    {
//        spin_lock_irqsave(&g_loop_eth_rx_queue_spinlock, sysflag);
//        if ( !g_loop_eth_rx_queue_stop )
//        {
//            g_loop_eth_rx_queue_stop = 1;
//            ret = 0;
//        }
//        spin_unlock_irqrestore(&g_loop_eth_rx_queue_spinlock, sysflag);
//    }
//
//    return ret;
//}

static int32_t ppa_fp_stop_tx(struct net_device *netif)
{
    return 0;
}

static int32_t ppa_fp_restart_tx(struct net_device *netif)
{
    return 0;
}

static int32_t ppa_fp_rx(struct net_device *rxif, struct net_device *txif, struct sk_buff *skb, int32_t len)
{
    struct loop_eth_priv_data *priv;
    int pktlen;

    if ( rxif )
    {
        dump_skb(skb, DUMP_SKB_LEN, "callback: rx", 0, 0);
        if ( netif_running(rxif) )
        {
            priv = netdev_priv(rxif);

            pktlen = skb->len;
            skb->dev = rxif;
            skb->protocol = eth_type_trans(skb, rxif);

            if ( netif_rx(skb) == NET_RX_DROP )
                priv->stats.rx_dropped++;
            else
            {
                priv->stats.rx_packets++;
                priv->stats.rx_bytes += pktlen;
            }

            priv->dp_pkts_from_ppe++;

            return 0;
        }
    }
    else if ( txif )
    {
        //if ( netif_running(txif) && !netif_queue_stopped(txif) )
        //{
        //    priv = (struct loop_eth_priv_data *)txif->priv;
        //
        //    eth_hard_start_xmit(skb, txif);
        //
        //    priv->dp_pkts_tx++;
        //
        //    return 0;
        //}
        priv = netdev_priv(txif);
        skb->dev = txif;
        dev_queue_xmit(skb);

        priv->dp_pkts_tx++;

        return 0;
    }

    dev_kfree_skb_any(skb);
    return 0;
}

#endif

static INLINE void proc_file_create(void)
{
    struct proc_dir_entry *res;

    g_loop_eth_dev_proc_dir = proc_mkdir("loopeth", NULL);

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
    res = create_proc_read_entry("dbg",
                                  0,
                                  g_loop_eth_dev_proc_dir,
                                  proc_read_dbg,
                                  NULL);
    if ( res )
        res->write_proc = proc_write_dbg;
#endif

    res = create_proc_read_entry("dev",
                                  0,
                                  g_loop_eth_dev_proc_dir,
                                  proc_read_dev,
                                  NULL);
    if ( res )
        res->write_proc = proc_write_dev;

    res = create_proc_read_entry("ip",
                                  0,
                                  g_loop_eth_dev_proc_dir,
                                  proc_read_ip,
                                  NULL);
    if ( res )
        res->write_proc = proc_write_ip;

    res = create_proc_read_entry("mib",
                                  0,
                                  g_loop_eth_dev_proc_dir,
                                  proc_read_mib,
                                  NULL);
    if ( res )
        res->write_proc = proc_write_mib;

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
    res = create_proc_read_entry("directpath",
                                  0,
                                  g_loop_eth_dev_proc_dir,
                                  proc_read_directpath,
                                  NULL);
    if ( res )
        res->write_proc = proc_write_directpath;

#if defined(ENABLE_TXDBG) && ENABLE_TXDBG
	res = create_proc_read_entry("cputx",
								  0,
								  g_loop_eth_dev_proc_dir,
								  proc_read_cputx,
								  NULL);
	if(res)
		res->write_proc = proc_write_cputx;
#endif
	
#endif
}

static INLINE void proc_file_delete(void)
{
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
    remove_proc_entry("directpath",
                      g_loop_eth_dev_proc_dir);
#endif

    remove_proc_entry("mib",
                      g_loop_eth_dev_proc_dir);

    remove_proc_entry("ip",
                      g_loop_eth_dev_proc_dir);

    remove_proc_entry("dev",
                      g_loop_eth_dev_proc_dir);

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
    remove_proc_entry("dbg",
                      g_loop_eth_dev_proc_dir);
#endif

#if defined(ENABLE_TXDBG) && ENABLE_TXDBG
	remove_proc_entry("cputx",
					  g_loop_eth_dev_proc_dir);
#endif

    remove_proc_entry("loopeth", NULL);
}

#if defined(ENABLE_DBG_PROC) && ENABLE_DBG_PROC
static int proc_read_dbg(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;

    len += sprintf(page + off + len, "error print      - enabled\n");
    len += sprintf(page + off + len, "debug print      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT)      ? "enabled" : "disabled");
    len += sprintf(page + off + len, "assert           - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_ASSERT)           ? "enabled" : "disabled");
    len += sprintf(page + off + len, "dump rx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_RX)      ? "enabled" : "disabled");
    len += sprintf(page + off + len, "dump tx skb      - %s\n", (g_dbg_enable & DBG_ENABLE_MASK_DUMP_SKB_TX)      ? "enabled" : "disabled");

    *eof = 1;

    return len;
}

static int proc_write_dbg(struct file *file, const char *buf, unsigned long count, void *data)
{
    static const char *dbg_enable_mask_str[] = {
        " error print",
        " err",
        " debug print",
        " dbg",
        " assert",
        " assert",
        " dump rx skb",
        " rx",
        " dump tx skb",
        " tx",
        " dump flag header",
        " header",
        " dump init",
        " init",
        " all"
    };
    static const int dbg_enable_mask_str_len[] = {
        12, 4,
        12, 4,
        7,  7,
        12, 3,
        12, 3,
        17, 7,
        10, 5,
        4
    };
    u32 dbg_enable_mask[] = {
        DBG_ENABLE_MASK_ERR,
        DBG_ENABLE_MASK_DEBUG_PRINT,
        DBG_ENABLE_MASK_ASSERT,
        DBG_ENABLE_MASK_DUMP_SKB_RX,
        DBG_ENABLE_MASK_DUMP_SKB_TX,
        DBG_ENABLE_MASK_ALL
    };

    char str[2048];
    char *p;

    int len, rlen;

    int f_enable = 0;
    int i;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    if ( strincmp(p, "enable", 6) == 0 )
    {
        p += 6;
        f_enable = 1;
    }
    else if ( strincmp(p, "disable", 7) == 0 )
    {
        p += 7;
        f_enable = -1;
    }
    else if ( strincmp(p, "help", 4) == 0 || *p == '?' )
    {
        printk("echo <enable/disable> [err/dbg/assert/rx/tx/all] > /proc/loopeth/dbg\n");
    }

    if ( f_enable )
    {
        if ( *p == 0 )
        {
            if ( f_enable > 0 )
                g_dbg_enable |= DBG_ENABLE_MASK_ALL;
            else
                g_dbg_enable &= ~DBG_ENABLE_MASK_ALL;
        }
        else
        {
            do
            {
                for ( i = 0; i < NUM_ENTITY(dbg_enable_mask_str); i++ )
                    if ( strincmp(p, dbg_enable_mask_str[i], dbg_enable_mask_str_len[i]) == 0 )
                    {
                        if ( f_enable > 0 )
                            g_dbg_enable |= dbg_enable_mask[i >> 1];
                        else
                            g_dbg_enable &= ~dbg_enable_mask[i >> 1];
                        p += dbg_enable_mask_str_len[i];
                        break;
                    }
            } while ( i < NUM_ENTITY(dbg_enable_mask_str) );
        }
    }

    return count;
}
#endif

static int proc_read_dev(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    int i;

    len += sprintf(page + off + len, "Devices:\n");

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( g_loop_eth_dev[i] )
            len += sprintf(page + off + len, "  %s\n", g_loop_eth_dev[i]->name);

    *eof = 1;

    return len;
}

static int proc_write_dev(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;

    int len, rlen;

    struct loop_eth_priv_data *priv;
    char ifname[32];
    int i;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    if ( strincmp(p, "add", 3) == 0 )
    {        
        if( strlen(p) >4 )
        {              
            int overhead=4; //add + one space
            p = p + overhead;            
            i=simple_strtol(p, NULL, 10);
            dbg("i=%d\n", i);
            if( i>=0 && i<NUM_ENTITY(g_loop_eth_dev) )
            {
                sprintf(ifname, "loopeth%d", i);               
                if ( !g_loop_eth_dev[i] )
                {
                    g_loop_eth_dev[i] = alloc_netdev(sizeof(struct loop_eth_priv_data), ifname, eth_setup);
                    if( g_loop_eth_dev[i] == NULL )
                    {
                        printk("alloc_netdev fail\n");
                        return count;
                    }
                    if ( register_netdev(g_loop_eth_dev[i]) )
                    {
                        free_netdev(g_loop_eth_dev[i]);
                        g_loop_eth_dev[i] = NULL;
                        printk("register device \"%s\" fail\n", ifname);
                    }
                    else
                    {
                        printk("add \"%s\" successfully\n", ifname);
                        priv = netdev_priv(g_loop_eth_dev[i]);
                        priv->ip_templ[0] = priv->ip_templ[1] = 10;
                    }
                }
                else printk("interface %s already exist\n", ifname);
            }
            else printk("index too big to be supported: %d\n", i);
        }
        else for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        {
            if ( !g_loop_eth_dev[i] )
            {
                sprintf(ifname, "loopeth%d", i);
                g_loop_eth_dev[i] = alloc_netdev(sizeof(struct loop_eth_priv_data), ifname, eth_setup);
                if ( g_loop_eth_dev[i] )
                {
                    if ( register_netdev(g_loop_eth_dev[i]) )
                    {
                        free_netdev(g_loop_eth_dev[i]);
                        g_loop_eth_dev[i] = NULL;
                        printk("register device \"%s\" fail\n", ifname);
                    }
                    else
                    {
                        printk("add \"%s\" successfully\n", ifname);
                        priv = netdev_priv(g_loop_eth_dev[i]);
                        priv->ip_templ[0] = priv->ip_templ[1] = 10;
                    }
                }
                else
                    printk("allocate device \"%s\" fail\n", ifname);
                break;
            }
        }
    }
    else if ( strincmp(p, "del ", 4) == 0 || strincmp(p, "rem ", 4) == 0 )
    {
        p += 4;
        while ( *p && *p <= ' ' )
            p++;
        //printk("p = %s\n", p);
        for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        {
            if ( g_loop_eth_dev[i] && stricmp(g_loop_eth_dev[i]->name, p) == 0 )
            {
                //first unregister from directpath first
                priv = netdev_priv(g_loop_eth_dev[i]);
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)                
                if( priv->dp_if_id > 0 )
                {                    
                    ppa_hook_directpath_register_dev_fn(&priv->dp_if_id, g_loop_eth_dev[i], NULL, 0);
                    priv->dp_if_id = -1;
                }
#endif          
                //unregister loopeth dev itself
                unregister_netdev(g_loop_eth_dev[i]);
                free_netdev(g_loop_eth_dev[i]);
                g_loop_eth_dev[i] = NULL;
                break;
            }
            else 
            {
                 //printk("fail to unregister %s\n", p);
            }
        }
    }
    else
    {
        printk("echo add <index>> /proc/loopeth/dev\n");
        printk("   example: echo add 1 > /proc/loopeth/dev\n");
        printk("        Note, the maximum index is %d\n", NUM_ENTITY(g_loop_eth_dev) -1);
        printk("echo <del/rem> <index><device name> > /proc/loopeth/dev\n");
        printk("   example: echo del loopeth1 > /proc/loopeth/dev\n");
    }

    return count;
}

static int proc_read_ip(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    struct loop_eth_priv_data *priv;
    int i;

    len += sprintf(page + off + len, "IP template:\n");

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( g_loop_eth_dev[i] )
        {
            priv = netdev_priv(g_loop_eth_dev[i]);
            len += sprintf(page + off + len, "  %s\t- %d.%d.xx.xx\n", g_loop_eth_dev[i]->name, (int)priv->ip_templ[0], (int)priv->ip_templ[1]);
        }

    *eof = 1;

    return len;
}

static int proc_write_ip(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;

    int len, rlen;

    struct loop_eth_priv_data *priv;
    int i, j;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( g_loop_eth_dev[i] && strincmp(g_loop_eth_dev[i]->name, p, strlen(g_loop_eth_dev[i]->name)) == 0 )
        {
            p += strlen(g_loop_eth_dev[i]->name);
            if ( !*p || *p > ' ' )
                continue;
            while ( *p && *p <= ' ' )
                p++;
            break;
        }

    if ( i < NUM_ENTITY(g_loop_eth_dev) )
    {
        priv = netdev_priv(g_loop_eth_dev[i]);
        for ( i = 0; i < 4; i++ )
        {
            j = 0;
            while ( *p >= '0' && *p <= '9' )
            {
                j = j * 10 + *p - '0';
                p++;
            }
            priv->ip_templ[i] = j;
            if ( *p++ != '.' )
                break;
        }
    }
    else
    {        
        printk("echo <device name> <IP address> > /proc/loopeth/ip\n");
        printk("Note: \n");
        printk("    a) This IP provides a netmask for loopeth to spoof arp request.\n");
        printk("       By default, it only check first two bytes, for example, 10.0.x.x\n");
        printk("    b) If IP address is 0.0.0.0, it means loopeth will spoof all IP address and all netmask\n");
    }

    return count;
}

static int proc_read_mib(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    struct loop_eth_priv_data *priv;
    int i;

    len += sprintf(page + off + len,         "MIB:\n");
    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( g_loop_eth_dev[i] )
        {
            priv = netdev_priv(g_loop_eth_dev[i]);

            len += sprintf(page + off + len,         "  %s:\n", g_loop_eth_dev[i]->name);
            len += sprintf(page + off + len,         "    rx_packets: %lu\n", priv->stats.rx_packets);
            len += sprintf(page + off + len,         "    rx_bytes:   %lu\n", priv->stats.rx_bytes);
            len += sprintf(page + off + len,         "    rx_errors:  %lu\n", priv->stats.rx_errors);
            len += sprintf(page + off + len,         "    rx_dropped: %lu\n", priv->stats.rx_dropped);
            len += sprintf(page + off + len,         "    tx_packets: %lu\n", priv->stats.tx_packets);
            len += sprintf(page + off + len,         "    tx_bytes:   %lu\n", priv->stats.tx_bytes);
            len += sprintf(page + off + len,         "    tx_errors:  %lu\n", priv->stats.tx_errors);
            len += sprintf(page + off + len,         "    tx_dropped: %lu\n", priv->stats.tx_dropped);
            len += sprintf(page + off + len,         "    rx_preprocess_drop:  %u\n", priv->rx_preprocess_drop);
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
            len += sprintf(page + off + len,         "    dp_pkts_to_ppe:      %u\n", priv->dp_pkts_to_ppe);
            len += sprintf(page + off + len,         "    dp_pkts_to_ppe_fail: %u\n", priv->dp_pkts_to_ppe_fail);
            len += sprintf(page + off + len,         "    dp_pkts_from_ppe:    %u\n", priv->dp_pkts_from_ppe);
            len += sprintf(page + off + len,         "    dp_pkts_tx:          %u\n", priv->dp_pkts_tx);
#endif
        }

    *eof = 1;

    return len;
}

static int proc_write_mib(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;
    u32 eth_clear;
    struct loop_eth_priv_data *priv;
    int i;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    eth_clear = 0;
    if ( stricmp(p, "clear") == 0 || stricmp(p, "clear all") == 0
        || stricmp(p, "clean") == 0 || stricmp(p, "clean all") == 0 )
        eth_clear = (1 << NUM_ENTITY(g_loop_eth_dev)) - 1;
    else if ( strincmp(p, "clear ", 6) == 0 || strincmp(p, "clean ", 6) == 0 )
    {
        p += 6;
        for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
            if ( g_loop_eth_dev[i] && stricmp(g_loop_eth_dev[i]->name, p) == 0 )
            {
                eth_clear |= 1 << i;
                break;
            }
    }
    else
        printk("echo <clear/clean> [all/device name] > /proc/loopeth/mib\n");

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( (eth_clear & (1 << i)) && g_loop_eth_dev[i] )
        {
            priv = netdev_priv(g_loop_eth_dev[i]);
            memset(&priv->stats, 0, sizeof(priv->stats));
            priv->rx_preprocess_drop    = 0;
#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
            priv->dp_pkts_to_ppe        = 0;
            priv->dp_pkts_to_ppe_fail   = 0;
            priv->dp_pkts_from_ppe      = 0;
            priv->dp_pkts_tx            = 0;
#endif
        }

    return count;
}

#if defined(CONFIG_LTQ_PPA_API) || defined(CONFIG_LTQ_PPA_API_MODULE)
static int proc_read_directpath(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len = 0;
    struct loop_eth_priv_data *priv;
    int i;

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( g_loop_eth_dev[i] )
        {
            priv = netdev_priv(g_loop_eth_dev[i]);
            if ( priv->dp_if_id >= 0 )
                len += sprintf(page + off + len, "%s - directpath on (ifid %d)\n", g_loop_eth_dev[i]->name, priv->dp_if_id);
            else
                len += sprintf(page + off + len, "%s - directpath off\n", g_loop_eth_dev[i]->name);
        }

    *eof = 1;

    return len;
}

static int proc_write_directpath(struct file *file, const char *buf, unsigned long count, void *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    int f_reg = 0;
    char ifname[32];
    PPA_DIRECTPATH_CB cb;
    uint32_t ifid;
    struct loop_eth_priv_data *priv;
    int i;

    if ( ppa_hook_directpath_register_dev_fn == NULL )
    {
        printk(RED "error" NORMAL ": PPA is not initialized!\n");
        return count;
    }

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

    if ( strincmp(p, "enable ", 7) == 0 )
    {
        p += 7;
        f_reg = 1;
    }
    else if ( strincmp(p, "on ", 3) == 0 )
    {
        p += 3;
        f_reg = 1;
    }
    else if ( strincmp(p, "register ", 9) == 0 )
    {
        p += 9;
        f_reg = 1;
    }
    else if ( strincmp(p, "reg ", 4) == 0 )
    {
        p += 4;
        f_reg = 1;
    }
    else if ( strincmp(p, "disable ", 8) == 0 )
    {
        p += 8;
        f_reg = -1;
    }
    else if ( strincmp(p, "off ", 4) == 0 )
    {
        p += 4;
        f_reg = -1;
    }
    else if ( strincmp(p, "unregister ", 11) == 0 )
    {
        p += 11;
        f_reg = -1;
    }
    else if ( strincmp(p, "unreg ", 6) == 0 )
    {
        p += 6;
        f_reg = -1;
    }

    if ( f_reg )
    {
        while ( 1 )
        {
            for ( ; *p && *p <= ' '; p++ );
            for ( i = 0; *p > ' ' && i < NUM_ENTITY(ifname) - 1; ifname[i++] = *p++ );
            ifname[i] = 0;
            if ( !ifname[0] )
                break;
            for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
                if ( g_loop_eth_dev[i] && stricmp(ifname, g_loop_eth_dev[i]->name) == 0 )
                {
                    if ( f_reg > 0 )
                    {
                        cb.stop_tx_fn   = ppa_fp_stop_tx;
                        cb.start_tx_fn  = ppa_fp_restart_tx;
                        cb.rx_fn        = ppa_fp_rx;
                        if ( ppa_hook_directpath_register_dev_fn(&ifid, g_loop_eth_dev[i], &cb, PPA_F_DIRECTPATH_REGISTER | PPA_F_DIRECTPATH_ETH_IF) == IFX_SUCCESS )
                        {
                            priv = netdev_priv(g_loop_eth_dev[i]);
                            priv->dp_if_id = ifid;
                            printk("manage to register directpath for %s (ifid %d)\n", ifname, priv->dp_if_id);
                        }
                        else
                            printk("failed in register directpath for %s\n", ifname);
                    }
                    else
                    {
                        priv = netdev_priv(g_loop_eth_dev[i]);
                        ppa_hook_directpath_register_dev_fn(&priv->dp_if_id, g_loop_eth_dev[i], NULL, 0);
                        priv->dp_if_id = -1;
                    }
                    break;
                }
        }
    }
    else
        printk("echo <register/unregister> <device name> > /proc/loopeth/directpath\n");

    return count;
}
#endif

static INLINE int stricmp(const char *p1, const char *p2)
{
    int c1, c2;

    while ( *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

static INLINE int strincmp(const char *p1, const char *p2, int n)
{
    int c1 = 0, c2;

    while ( n && *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
        n--;
    }

    return n ? *p1 - *p2 : c1;
}

#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
static INLINE void dump_skb(struct sk_buff *skb, u32 len, char *title, int ch, int is_tx)
{
    int i;

    if ( !(g_dbg_enable & (is_tx ? DBG_ENABLE_MASK_DUMP_SKB_TX : DBG_ENABLE_MASK_DUMP_SKB_RX)) )
        return;

    if ( skb->len < len )
        len = skb->len;

    if ( ch >= 0 )
        printk("%s (ch %d)\n", title, ch);
    else
        printk("%s\n", title);
    printk("  skb->data = %08X, skb->tail = %08X, skb->len = %d\n", (u32)skb->data, (u32)skb->tail, (int)skb->len);
    for ( i = 1; i <= len; i++ )
    {
        if ( i % 16 == 1 )
            printk("  %4d:", i - 1);
        printk(" %02X", (int)(*((char*)skb->data + i - 1) & 0xFF));
        if ( i % 16 == 0 )
            printk("\n");
    }
    if ( (i - 1) % 16 != 0 )
        printk("\n");
}
#endif

#if defined(ENABLE_TXDBG) && ENABLE_TXDBG

static struct sk_buff* alloc_cputx_skb(int skb_len)
{
	struct sk_buff *new_skb = dev_alloc_skb(skb_len);
	if(!new_skb){
		return NULL;
	}

	return new_skb;	
}

static int init_cputx_skb(void)
{
	struct ethhdr *eth_h;
	struct iphdr *iph;
	struct udphdr *udph;
	uint32_t srcip, dstip, i;

	char *dst_mac[]={"00","01","02","03","04","05"};
	char *src_mac[]={"00","10","E0","00","00","01"};
	char *src_ip[] ={"192","168","1","100"};
	char *dst_ip[] ={"192","168","1","200"};
	
	g_tx_skb = alloc_cputx_skb(MAX_SKB_SIZE);
	if(!g_tx_skb){
		printk("failed to alloc skb. initialization failed\n");
		return -1;
	}

	if(g_pkt_size > MAX_PKT_SIZE){
		g_pkt_size = MAX_PKT_SIZE;
	}

	skb_reserve(g_tx_skb, 8); //reserve for flag header
	skb_put(g_tx_skb, g_pkt_size);
	eth_h = (struct ethhdr *)g_tx_skb->data;

	for(i = 0; i < 6; i ++){
		eth_h->h_dest[i] = simple_strtoul(dst_mac[i], NULL, 10);
		eth_h->h_source[i] = simple_strtoul(src_mac[i], NULL, 10);
	}
	eth_h->h_proto = htons(ETH_P_IP);

	iph = (struct iphdr *)(g_tx_skb->data + sizeof(struct ethhdr));
	memset(iph, 0, sizeof(struct iphdr));
	iph->version = 4;
	iph->ihl = 5;
	iph->tot_len = g_pkt_size - 14;//totoal pkt size  - ethernet hdr
	iph->ttl = 64;
	iph->protocol = IPPROTO_UDP;
	iph->check = 0;
	srcip = (simple_strtoul(src_ip[0], NULL, 10)<<24) | 
		    (simple_strtoul(src_ip[1], NULL, 10)<<16) | 
		    (simple_strtoul(src_ip[2], NULL, 10)<<8)  | 
		    (simple_strtoul(src_ip[3], NULL, 10));
	
	dstip = (simple_strtoul(dst_ip[0], NULL, 10)<<24) | 
		    (simple_strtoul(dst_ip[1], NULL, 10)<<16) | 
		    (simple_strtoul(dst_ip[2], NULL, 10)<<8)  | 
		    (simple_strtoul(dst_ip[3], NULL, 10));
	
	iph->saddr = srcip;
	iph->daddr = dstip;
	iph->check = ip_fast_csum(iph, iph->ihl);

	udph = (struct udphdr *)(g_tx_skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr));
	udph->source = htons(1024);
	udph->dest = htons(1024);
	udph->check = 0;
	udph->len = g_pkt_size - 14 - 20; //totol pkt size -  CRC - ethernet hdr - ip hdr

	return 0;
	
}

static void reset_pkt_size(struct sk_buff *skb, uint32_t new_size)
{
	struct iphdr *iph;
	struct udphdr *udph;
	
	int len = new_size - skb->len;
	skb_put(skb, len);

	iph = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
	iph->tot_len = iph->tot_len + len;
	iph->check = 0;
	iph->check = ip_fast_csum(iph, iph->ihl);
	udph = (struct udphdr *)(skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr));
	udph->len += len;

	return;
}

static void reset_tx_dev(char *dev_name, struct sk_buff *pskb)
{
	struct net_device * pdev;

	pdev = ifx_dev_get_by_name(dev_name);
	if(pdev){
		dev_put(pdev);
	}

	pskb->dev = pdev;

	return;
}

static int send_out_pkt(void *arg)
{
	struct sk_buff *skb_tx;
	uint32_t tx_cnt= 1;
	
	g_tx_mib = 0;
	g_tx_start = TX_START;

	if(!g_tx_skb->dev){
		goto __THREAD_STOP;
	}
	
	while(!kthread_should_stop() && tx_cnt <= g_tx_count){
			if(tx_cnt % g_tx_pause_cnt == 0)
				schedule_timeout_uninterruptible(1);
			skb_tx = skb_clone(g_tx_skb, GFP_ATOMIC);
			dev_queue_xmit(skb_tx);
			g_tx_mib ++;
			tx_cnt ++;
	}

__THREAD_STOP:
	g_tx_start = TX_STOP;
	
	return 0;
}

static void send_out_manager(char *dev_name)
{
	struct net_device *dev;
	
	if(!g_tx_count){
		printk("Warning: tx count is zero, stop sending\n");
		return;
	}

	if(!g_tx_skb){
		printk("Error: tx skb is not init\n");
		return;
	}
	
	dev = ifx_dev_get_by_name(dev_name);
	if(dev){
		dev_put(dev);
	}else{
		printk("Cannot find the dev by name: %s\n", dev_name);
		return;
	}

	if(!ifx_get_xmit_fn(dev)){
		printk("The dev %s don't support xmit function", dev_name);
		return;
	}

	if(g_tx_ts && g_tx_start == TX_START){
		//printk("Warning: last thread is still running\n");
		kthread_stop(g_tx_ts);
	}

	g_tx_ts = kthread_run(send_out_pkt, NULL, "sending_out");
	if(IS_ERR(g_tx_ts)){
		printk("Cannot alloc a new thread to send the pkt\n");
		g_tx_ts = NULL;
		g_tx_start = TX_STOP;
	}

	return;
}

static int print_skb(char *buf, struct sk_buff *skb)
{
	int len = 0;
	struct ethhdr *eth_h = (struct ethhdr *)skb->data;
	struct iphdr *iph = (struct iphdr *)(skb->data + sizeof(struct ethhdr));

	if(!skb)
		return len;

	
	len += sprintf(buf + len, "dst mac: %x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",
		eth_h->h_dest[0]>>4, eth_h->h_dest[0], 
		eth_h->h_dest[1]>>4, eth_h->h_dest[1],
		eth_h->h_dest[2]>>4, eth_h->h_dest[2],
		eth_h->h_dest[3]>>4, eth_h->h_dest[3],
		eth_h->h_dest[4]>>4, eth_h->h_dest[4],
		eth_h->h_dest[5]>>4, eth_h->h_dest[5]);

	len += sprintf(buf + len, "src mac: %x%x:%x%x:%x%x:%x%x:%x%x:%x%x\n",
		eth_h->h_source[0]>>4, eth_h->h_source[0],
		eth_h->h_source[1]>>4, eth_h->h_source[1],
		eth_h->h_source[2]>>4, eth_h->h_source[2],
		eth_h->h_source[3]>>4, eth_h->h_source[3],
		eth_h->h_source[4]>>4, eth_h->h_source[4],
		eth_h->h_source[5]>>4, eth_h->h_source[5]);

	len += sprintf(buf + len, "dst ip: %d.%d.%d.%d, src ip: %d.%d.%d.%d\n",
		(iph->daddr >> 24) & 0xFF, (iph->daddr >> 16) & 0xFF,
		(iph->daddr >> 8)  & 0xFF, iph->daddr & 0xFF,
		(iph->saddr >> 24) & 0xFF, (iph->saddr >> 16) & 0xFF,
		(iph->saddr >> 8)  & 0xFF, iph->saddr & 0xFF);
		

	return len;
}

static int proc_read_cputx(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

    len += sprintf(page + off + len, "frame count: 0x%x\n", g_tx_count);
	len += sprintf(page + off + len, "frame mib count: 0x%x\n", g_tx_mib);
	len += sprintf(page + off + len, "TX dev name: %s\n", g_tx_dev);
	len += sprintf(page + off + len, "Tx pause num: %d\n", g_tx_pause_cnt);
	len += sprintf(page + off + len, "frame size: %d(Not including CRC)\n", g_pkt_size);

	len += print_skb(page + off + len, g_tx_skb);
	
	*eof = 1;
	return len;
}


static int proc_write_cputx(struct file *file, const char *buf, unsigned long count, void *data)
{
	char str[1024];
    char *p, *p1;

    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return 0;

	if(strncasecmp(p, "help", 4) == 0){
		printk("echo count <num> > /proc/loopeth/cputx (0xFFFF would be continues sending)\n");
		printk("echo start/stop > /proc/loopeth/cputx\n");
		printk("echo dev <name> > /proc/loopeth/cputx\n");
		printk("echo intcnt <num> /proc/loopeth/cputx  (min: 1, max: %d)\n", MAX_PAUSE_CNT);
		printk("echo pktsize <num> /proc/loopeth/cputx (min: %d, max:%d)\n", MIN_PKT_SIZE, MAX_PKT_SIZE);
	}else if(strncasecmp(p, "count ", 6) == 0){
		p = p + 5;
		while ( *p && *p <= ' ' )
           p++;
		g_tx_count = simple_strtoul(p, NULL, 10);
		
	}else if(strncasecmp(p,"start", 5) == 0){
		ppa_lock_get(&g_tx_start_lock);
		if(g_tx_start == TX_START){
			printk("still running, please stop first\n");
			goto __EXIT_TX_LOCK;
		}
		send_out_manager(g_tx_dev);
		ppa_lock_release(&g_tx_start_lock);
			
	}else if(strncasecmp(p,"stop", 4) == 0){
		ppa_lock_get(&g_tx_start_lock);
		if(g_tx_start == TX_STOP){
			goto __EXIT_TX_LOCK;
		}
		kthread_stop(g_tx_ts);
		g_tx_ts = NULL;
		ppa_lock_release(&g_tx_start_lock);
	}else if(strncasecmp(p,"dev ", 4) == 0){
		p = p + 4;
		while ( *p && *p == ' ' )
           p++;
		for(p1 = p; *p1 && *p1 != ' '; p1 ++);
		*p1 = 0;
		strncpy(g_tx_dev, p, strlen(p)+1);
		reset_tx_dev(g_tx_dev, g_tx_skb);
	}else if(strncasecmp(p, "dumpskb", 7) == 0){
		if(g_tx_skb){
			dump_skb(g_tx_skb,g_tx_skb->len, "dump tx skb", -1, 0);
		}
	}else if(strncasecmp(p, "intcnt ", 7) == 0){
		p = p + 7;
		while ( *p && *p <= ' ' )
           p++;
		g_tx_pause_cnt = simple_strtoul(p, NULL, 10);
		if(g_tx_pause_cnt == 0){
			printk("Cannot set intermittent number to zero!!!\n");
			g_tx_pause_cnt = DEFAULT_PAUSE_CNT;
		}else if(g_tx_pause_cnt > MAX_PAUSE_CNT){
			printk("Set to MAX PAUSE COUNT: %d\n", MAX_PAUSE_CNT);
			g_tx_pause_cnt = MAX_PAUSE_CNT;
		}
	}else if(strncasecmp(p, "pktsize ", 8) == 0){
		ppa_lock_get(&g_tx_start_lock);
		if(g_tx_start == TX_START){
			printk("Cannot change the packet size when sending traffic!\n");
			goto __EXIT_TX_LOCK;
		}
		p = p + 8;
		while ( *p && *p <= ' ' )
           p++;
		g_pkt_size = simple_strtoul(p, NULL, 10);
		if(g_pkt_size < MIN_PKT_SIZE || g_pkt_size > MAX_PKT_SIZE){
			printk("pkt size cannot be less than %d, or larger than %d\n",MIN_PKT_SIZE, MAX_PKT_SIZE);
			g_pkt_size = DEFAULT_PKT_SIZE;
		}
		reset_pkt_size(g_tx_skb, g_pkt_size);
		ppa_lock_release(&g_tx_start_lock);
	}
	return count;

__EXIT_TX_LOCK:
	ppa_lock_release(&g_tx_start_lock);
	return count;
	
}


#endif


/*
 * ####################################
 *           Global Function
 * ####################################
 */



/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

static int __init loop_eth_dev_init(void)
{
    printk("Loading loop_net_dev driver ...... ");

    proc_file_create();

#if defined(ENABLE_TXDBG) && ENABLE_TXDBG
	ppa_lock_init(&g_tx_start_lock);
	init_cputx_skb();
#endif

    printk("Succeeded!\n");
    return 0;
}

static void __exit loop_eth_dev_exit(void)
{
    int i;

    proc_file_delete();

    for ( i = 0; i < NUM_ENTITY(g_loop_eth_dev); i++ )
        if ( g_loop_eth_dev[i] )
        {
            unregister_netdev(g_loop_eth_dev[i]);
            free_netdev(g_loop_eth_dev[i]);
        }

#if defined(ENABLE_TXDBG) && ENABLE_TXDBG
	if(g_tx_ts && g_tx_start == TX_START){
		kthread_stop(g_tx_ts);
	}
	if(g_tx_skb != NULL){
		dev_kfree_skb_any(g_tx_skb);
	}
	ppa_lock_destroy(&g_tx_start_lock);
	
#endif
}

module_init(loop_eth_dev_init);
module_exit(loop_eth_dev_exit);
