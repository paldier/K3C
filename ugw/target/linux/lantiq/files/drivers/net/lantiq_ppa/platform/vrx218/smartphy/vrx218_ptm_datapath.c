#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <net/xfrm.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pcie.h>
#include <asm/ifx/ifx_led.h>
#ifdef CONFIG_LTQ_ETH_FRAMEWORK
  #include <asm/ifx/ifx_eth_framework.h>
#endif
#else
//#include <ltq_regs.h>
//#include <common_routines.h>
//#include <ltq_pmu.h>
//#include <asm/mach-ltqcpe/ltq_pcie.h>
//#include <asm/mach-ltqcpe/ltq_ledc.h>
#include <lantiq_dma.h>
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>
#include <lantiq_pcie.h>

#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
  #include <lantiq_eth_framework.h>
#endif
#endif



#include <net/ppa_api.h>
#include <net/ppa_stack_al.h>
#include "../../ppa_datapath.h"

#include "vrx218_common.h"
#include "vrx218_ptm_common.h"
#include "vrx218_e1_addr_def.h"



#define ETH_WATCHDOG_TIMEOUT                    (10 * HZ)

//#define DMA_PACKET_SIZE                         1600    //  ((1518 + 8 <2 VLAN> + 62 <PPE FW> + 8 <SW Header>) + 31) & ~31
#define DMA_ALIGNMENT                           32

#define ETH_MIN_TX_PACKET_LENGTH                ETH_ZLEN

//Maximum 64 Descriptors
#define CPU_TO_WAN_TX_DESC_BASE                 ((volatile struct tx_descriptor *)KSEG1ADDR(SOC_US_CPUPATH_DES_BASE))
#define CPU_TO_WAN_TX_DESC_NUM                  SOC_US_CPUPATH_DES_NUM

#define PTM_PORT                                7

//  MIB counter
#define RECEIVE_NON_IDLE_CELL_CNT(i,base)       SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x34A0 + (i)), base)
#define RECEIVE_IDLE_CELL_CNT(i,base)           SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x34A2 + (i)), base)
#define TRANSMIT_CELL_CNT(i,base)               SOC_ACCESS_VRX218_ADDR(SB_BUFFER(0x34A4 + (i)), base)

/*
 *  Mailbox IGU0 Registers
 */
// VRX218 US Master IGU0 Interrupt
#define MBOX_IGU0_ISRS                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0200), g_vrx218_dev_us.phy_membase)
#define MBOX_IGU0_ISRC                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0201), g_vrx218_dev_us.phy_membase)
#define MBOX_IGU0_ISR                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0202), g_vrx218_dev_us.phy_membase)
#define MBOX_IGU0_IER                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0203), g_vrx218_dev_us.phy_membase)

// VRX218 DS Master IGU0 Interrupt
#define MBOX_IGU0_DSM_ISRS                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0200), g_vrx218_dev_ds.phy_membase)
#define MBOX_IGU0_DSM_ISRC                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0201), g_vrx218_dev_ds.phy_membase)
#define MBOX_IGU0_DSM_ISR                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0202), g_vrx218_dev_ds.phy_membase)
#define MBOX_IGU0_DSM_IER                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0203), g_vrx218_dev_ds.phy_membase)


/*
 *  Mailbox IGU1 Registers
 */
// VRX218 US Master IGU1 Interrupt
#define MBOX_IGU1_ISRS                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0204), g_vrx218_dev_us.phy_membase)
#define MBOX_IGU1_ISRC                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0205), g_vrx218_dev_us.phy_membase)
#define MBOX_IGU1_ISR                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0206), g_vrx218_dev_us.phy_membase)
#define MBOX_IGU1_IER                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0207), g_vrx218_dev_us.phy_membase)

// VRX218 DS Master IGU1 Interrupt
#define MBOX_IGU1_DSM_ISRS                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0204), g_vrx218_dev_ds.phy_membase)
#define MBOX_IGU1_DSM_ISRC                      SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0205), g_vrx218_dev_ds.phy_membase)
#define MBOX_IGU1_DSM_ISR                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0206), g_vrx218_dev_ds.phy_membase)
#define MBOX_IGU1_DSM_IER                       SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0207), g_vrx218_dev_ds.phy_membase)

/*
 *  IMCU Registers
 */
#define IMCU_IMER_BASE_ADDR                     0x1E000044
#define IMCU_USM_IMER                           SOC_ACCESS_VRX218_ADDR(IMCU_IMER_BASE_ADDR, g_vrx218_dev_us.phy_membase)
#define IMCU_DSM_IMER                           SOC_ACCESS_VRX218_ADDR(IMCU_IMER_BASE_ADDR, g_vrx218_dev_ds.phy_membase)

/*
 *  Internal Structure of Devices (ETH/ATM)
 */
struct eth_priv_data {
    struct  net_device_stats        stats;

    unsigned int                    rx_packets;
    unsigned int                    rx_bytes;
    unsigned int                    rx_dropped;
    unsigned int                    tx_packets;
    unsigned int                    tx_bytes;
    unsigned int                    tx_errors;
    unsigned int                    tx_dropped;

    unsigned int                    dev_id;
};

struct tx_descriptor {
    unsigned int own        :1;
    unsigned int c          :1;
    unsigned int sop        :1;
    unsigned int eop        :1;
    unsigned int byteoff    :5;
    unsigned int qos        :4;
    unsigned int res        :4;
    unsigned int small      :1;
    unsigned int datalen    :14;

    unsigned int dataptr    :32;
};



/*
 *  Network Operations
 */
static void ptm_setup(struct net_device *);
static struct net_device_stats *ptm_get_stats(struct net_device *);
static int ptm_open(struct net_device *);
static int ptm_stop(struct net_device *);
static int ptm_qos_hard_start_xmit(struct sk_buff *, struct net_device *);
static int ptm_qos_hard_start_xmit_b(struct sk_buff *, struct net_device *);
static int ptm_set_mac_address(struct net_device *, void *);
static int ptm_ioctl(struct net_device *, struct ifreq *, int);
static void ptm_tx_timeout(struct net_device *);
static int ptm_push(struct sk_buff *skb, struct flag_header *header, unsigned int ifid);

/* DSL Showtime Function */
static int in_showtime(void);


/*
 *  DSL Data LED
 */
#ifdef CONFIG_IFX_LED
  static void dsl_led_flash(void);
  static void dsl_led_polling(unsigned long);
#endif

/*
 *  Mailbox handler
 */
static irqreturn_t mailbox_irq_handler(int irq, void *dev_id);
static irqreturn_t mailbox_vrx218_dsm_irq_handler(int irq, void *dev_id);

/*
 *  Debug functions
 */
static void dump_skb(struct sk_buff *skb, u32 len, const char *title, int port, int ch, int is_tx);
static int swap_mac(unsigned char *data);

/*
 *  External Functions
 */

static int g_dsl_bonding = 0;

static ifx_pcie_ep_dev_t g_vrx218_dev_us, g_vrx218_dev_ds;

static int g_cpu_to_wan_tx_desc_pos = 0;
static DEFINE_SPINLOCK(g_cpu_to_wan_tx_desc_lock);

static struct net_device *g_ptm_net_dev[1] = {0};
#if !defined(CONFIG_LANTIQ_ETH_FRAMEWORK) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
  static struct net_device_ops g_ptm_netdev_ops[1] = {
    {
    .ndo_open               = ptm_open,
    .ndo_stop               = ptm_stop,
    .ndo_get_stats          = ptm_get_stats,
    .ndo_set_mac_address    = ptm_set_mac_address,
    .ndo_start_xmit         = ptm_qos_hard_start_xmit,
    .ndo_tx_timeout         = ptm_tx_timeout,
    .ndo_do_ioctl           = ptm_ioctl,
    },
  };
#endif

#define LINE_NUMBER       2
int g_showtime[LINE_NUMBER]= {0};

#ifdef CONFIG_IFX_LED
  static unsigned int g_wrx_bc_user_cw = 0;
  static unsigned int g_total_tx_cw = 0;
  static struct timer_list g_dsl_led_polling_timer;
  static void *g_data_led_trigger = NULL;
#endif

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
  #define ifx_eth_fw_alloc_netdev(size, ifname, dummy)      alloc_netdev(size, ifname, ether_setup)
  #define ifx_eth_fw_free_netdev(netdev, dummy)             free_netdev(netdev)
  #define ifx_eth_fw_register_netdev(netdev)                register_netdev(netdev)
  #define ifx_eth_fw_unregister_netdev(netdev, dummy)       unregister_netdev(netdev)
  #define ifx_eth_fw_netdev_priv(netdev)                    netdev_priv(netdev)
#endif



static uint32_t g_pcie_reset_sig = 0;

static void ptm_setup(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    int val;
    int i;

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dev->get_stats       = ptm_get_stats;
    dev->open            = ptm_open;
    dev->stop            = ptm_stop;
    dev->hard_start_xmit = g_dsl_bonding ? ptm_qos_hard_start_xmit_b : ptm_qos_hard_start_xmit;
    dev->set_mac_address = ptm_set_mac_address;
    dev->do_ioctl        = ptm_ioctl;
    dev->tx_timeout      = ptm_tx_timeout;
  #else
    if ( g_dsl_bonding )
        g_ptm_netdev_ops[0].ndo_start_xmit = ptm_qos_hard_start_xmit_b;
    dev->netdev_ops      = &g_ptm_netdev_ops[0];
  #endif
    dev->watchdog_timeo  = ETH_WATCHDOG_TIMEOUT;
#endif
    priv->dev_id = 7;   //  DSL

    for ( i = 0, val = 0; i < 6; i++ )
        val += dev->dev_addr[i];
    if ( val == 0 )
    {
        dev->dev_addr[0] = 0x00;
        dev->dev_addr[1] = 0x20;
        dev->dev_addr[2] = 0xda;
        dev->dev_addr[3] = 0x86;
        dev->dev_addr[4] = 0x23;
        dev->dev_addr[5] = 0xee;
    }
}

static struct net_device_stats *ptm_get_stats(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

#if 0
    int port = 7;

    priv->stats.rx_packets  = priv->rx_packets
                            + ITF_MIB_TBL(port)->ig_fast_brg_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_udp_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_tcp_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_mc_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv6_udp_pkts
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv6_tcp_pkts;
    priv->stats.rx_bytes    = priv->rx_bytes
                            + ITF_MIB_TBL(port)->ig_fast_brg_bytes
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv4_bytes
                            + ITF_MIB_TBL(port)->ig_fast_rt_ipv6_bytes;
    priv->stats.rx_errors   = 0;
    priv->stats.rx_dropped  = priv->rx_dropped
                            + ITF_MIB_TBL(port)->ig_drop_pkts;

    priv->stats.tx_packets  = priv->tx_packets + ITF_MIB_TBL(port)->eg_fast_pkts;
    priv->stats.tx_bytes    = priv->tx_bytes;
    priv->stats.tx_errors   = priv->tx_errors;
    priv->stats.tx_dropped  = priv->tx_dropped;
#else
    priv->stats.rx_packets  = priv->rx_packets;
    priv->stats.rx_bytes    = priv->rx_bytes;
    priv->stats.rx_errors   = 0;
    priv->stats.rx_dropped  = priv->rx_dropped;

    priv->stats.tx_packets  = priv->tx_packets;
    priv->stats.tx_bytes    = priv->tx_bytes;
    priv->stats.tx_errors   = priv->tx_errors;
    priv->stats.tx_dropped  = priv->tx_dropped;
#endif

    return &priv->stats;
}

static int ptm_open(struct net_device *dev)
{
    int port = 7;

    if(ppa_drv_datapath_mac_entry_setting){
        ppa_drv_datapath_mac_entry_setting(dev->dev_addr, 0, 6, 10, 1, 1);
    }

    turn_on_dma_rx(port);

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
    netif_start_queue(dev);
#endif

#ifdef CONFIG_IFX_LED
    g_wrx_bc_user_cw = *RECEIVE_NON_IDLE_CELL_CNT(0, g_vrx218_dev_us.phy_membase) + *(1, g_vrx218_dev_us.phy_membase);
    g_total_tx_cw = *TRANSMIT_CELL_CNT(0, g_vrx218_dev_us.phy_membase) + *TRANSMIT_CELL_CNT(1RECEIVE_NON_IDLE_CELL_CNT, g_vrx218_dev_us.phy_membase);
    if ( g_dsl_bonding ) {
        g_wrx_bc_user_cw += *RECEIVE_NON_IDLE_CELL_CNT(0, g_vrx218_dev_ds.phy_membase) + *RECEIVE_NON_IDLE_CELL_CNT(1, g_vrx218_dev_ds.phy_membase);
        g_total_tx_cw += *TRANSMIT_CELL_CNT(0, g_vrx218_dev_ds.phy_membase) + *TRANSMIT_CELL_CNT(1, g_vrx218_dev_ds.phy_membase);
    }
    g_dsl_led_polling_timer.expires = jiffies + HZ / 10;    //  100ms
    add_timer(&g_dsl_led_polling_timer);
#endif

    return 0;
}

static int ptm_stop(struct net_device *dev)
{
    int port = 7;

#ifdef CONFIG_IFX_LED
    del_timer(&g_dsl_led_polling_timer);
#endif

    turn_off_dma_rx(port);

#ifndef CONFIG_LANTIQ_ETH_FRAMEWORK
    netif_stop_queue(dev);
#endif

    if(ppa_drv_datapath_mac_entry_setting){
        ppa_drv_datapath_mac_entry_setting(dev->dev_addr, 0, 6, 10, 1, 2);
    }

    return 0;
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
    if ( new_skb->destructor )
    {
        WARN_ON(in_irq());
        new_skb->destructor(new_skb);
        new_skb->destructor = NULL;
    }
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

static struct sk_buff *alloc_skb_tx(unsigned int len)
{
    struct sk_buff *skb;

    len = (len + DMA_ALIGNMENT - 1) & ~(DMA_ALIGNMENT - 1);

    skb = dev_alloc_skb(len);
    if ( skb )
    {
        if ( ((u32)skb->data & (DMA_ALIGNMENT - 1)) != 0 ) {
            panic("%s:%d: unaligned address - skb->data = 0x%08x!", __FUNCTION__, __LINE__, (u32)skb->data);
        }
        *((u32 *)skb->data - 1) = (u32)skb;
#ifndef CONFIG_MIPS_UNCACHED
        dma_cache_wback((u32)skb->data - sizeof(u32), sizeof(u32));
#endif
    }

    return skb;
}

static int ptm_qos_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
    int qid;
    unsigned long sys_flag;
    volatile struct tx_descriptor *desc;
    struct tx_descriptor reg_desc;
    struct sk_buff *skb_to_free;
    int byteoff;
    int len;
    int snd_len;
    int padding = 0;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    if( skb->len < ETH_MIN_TX_PACKET_LENGTH )
    {
        len = ETH_MIN_TX_PACKET_LENGTH;
        padding = 1;
    }else
    {
        len =  skb->len;
    }

    if ( !in_showtime() )
        goto ETH_XMIT_DROP;

    dump_skb(skb, skb->len, "ptm_qos_hard_start_xmit - raw", 0, 0, 1);
    swap_mac(skb->data);
    dump_skb(skb, skb->len, "ptm_qos_hard_start_xmit - swap", 0, 0, 1);

#if 0
    if ( skb->cb[13] == 0x5A )  //  magic number indicating forcing QId (e.g. directpath)
        qid = skb->cb[15];
    else
#endif
    qid = get_qid_by_priority(PTM_PORT , skb->priority);

    snd_len = len;

    byteoff = (unsigned int)skb->data & (DMA_ALIGNMENT - 1);
    if ( byteoff || skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff || skb_shared(skb) || skb_cloned(skb) ||
          (unsigned int)skb->end - (unsigned int)skb->data < DMA_PACKET_SIZE  )
    {
        struct sk_buff *new_skb;

        new_skb = alloc_skb_tx(skb->len < DMA_PACKET_SIZE ? DMA_PACKET_SIZE : skb->len);    //  may be used by RX fastpath later
        if ( !new_skb )
        {
            goto ALLOC_SKB_TX_FAIL;
        }
        skb_put(new_skb, skb->len);
        memcpy(new_skb->data, skb->data, skb->len);
        dev_kfree_skb_any(skb);
        skb = new_skb;
        byteoff = (u32)skb->data & (DMA_ALIGNMENT - 1);
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((u32)skb->data, skb->len);
#endif
    }
    else
    {
        *(struct sk_buff **)((u32)skb->data - byteoff - sizeof(struct sk_buff *)) = skb;
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((u32)skb->data - byteoff - sizeof(struct sk_buff *), skb->len + byteoff + sizeof(struct sk_buff *));
#endif
    }
    //fix padding problem
    if( padding)
    {
        if( skb_padto(skb,snd_len) == 0 )
        {
            dma_cache_wback((u32)skb->data, snd_len);
        }
    }

    /*  allocate descriptor */
    spin_lock_irqsave(&g_cpu_to_wan_tx_desc_lock, sys_flag);
    desc = (volatile struct tx_descriptor *)(CPU_TO_WAN_TX_DESC_BASE + g_cpu_to_wan_tx_desc_pos);
    if ( desc->own )    //  PPE hold
    {
        spin_unlock_irqrestore(&g_cpu_to_wan_tx_desc_lock, sys_flag);
        goto NO_FREE_DESC;
    }
    if ( ++g_cpu_to_wan_tx_desc_pos == CPU_TO_WAN_TX_DESC_NUM )
        g_cpu_to_wan_tx_desc_pos = 0;
    spin_unlock_irqrestore(&g_cpu_to_wan_tx_desc_lock, sys_flag);

    /*  load descriptor from memory */
    reg_desc = *desc;

    /*  free previous skb   */
    skb_to_free = get_skb_pointer(reg_desc.dataptr);
//    printk("skb_to_free = %08x\n", (unsigned int)skb_to_free);
    dev_kfree_skb_any(skb_to_free);

    /*  detach from protocol    */
    skb_to_free = skb;
    skb = skb_break_away_from_protocol(skb);
    dev_kfree_skb_any(skb_to_free);

    put_skb_to_dbg_pool(skb);

    /*  update descriptor   */
    reg_desc.small      = (unsigned int)skb->end - (unsigned int)skb->data < DMA_PACKET_SIZE ? 1 : 0;
    reg_desc.dataptr    = (u32)skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1));  //  byte address
    reg_desc.byteoff    = byteoff;
    reg_desc.datalen    = snd_len;
    reg_desc.qos        = qid;
    reg_desc.own        = 1;
    reg_desc.c          = 0;
    reg_desc.sop = reg_desc.eop = 1;

    /*  update MIB  */

    dev->trans_start = jiffies;
    priv->tx_packets++;
    priv->tx_bytes += len;

    /*  write discriptor to memory and write back cache */
    *((volatile u32 *)desc + 1) = *((u32 *)&reg_desc + 1);
    *(volatile u32 *)desc = *(u32 *)&reg_desc;

    return 0;

NO_FREE_DESC:
ALLOC_SKB_TX_FAIL:
ETH_XMIT_DROP:
    dev_kfree_skb_any(skb);
    priv->tx_dropped++;
    return 0;
}

static int ptm_qos_hard_start_xmit_b(struct sk_buff *skb, struct net_device *dev)
{
    int qid;

    if(!in_showtime()){
	dev_kfree_skb_any(skb);
	return 0;	
    }
    skb->dev = dev;
	#if 0
    if ( skb->cb[13] == 0x5A )  //  magic number indicating forcing QId (e.g. directpath)
        qid = skb->cb[15];
    else
		#endif
       qid = get_qid_by_priority(PTM_PORT, skb->priority);
    dump_skb(skb, skb->len, __FUNCTION__, 0, 0, 1);
    swap_mac(skb->data);
    eth_xmit(skb, PTM_PORT /* DSL */, 2, 2, qid ,0);/* Changed tc to qid to fix Qos in Sw fast path*/
    return 0;
}

static int ptm_set_mac_address(struct net_device *dev, void *p)
{
    struct sockaddr *addr = (struct sockaddr *)p;
#ifdef ROUT_MAC_CFG_TBL
    u32 addr1, addr2;
    int i;
#endif

#ifdef ROUT_MAC_CFG_TBL
    addr1 = (((u32)dev->dev_addr[0] & 0xFF) << 24) | (((u32)dev->dev_addr[1] & 0xFF) << 16) | (((u32)dev->dev_addr[2] & 0xFF) << 8) | ((u32)dev->dev_addr[3] & 0xFF);
    addr2 = (((u32)dev->dev_addr[4] & 0xFF) << 24) | (((u32)dev->dev_addr[5] & 0xFF) << 16);
    for ( i = 0; i < 16; i++ )
        if ( ROUT_MAC_CFG_TBL(i)[0] == addr1 && ROUT_MAC_CFG_TBL(i)[1] == addr2 )
        {
            ROUT_MAC_CFG_TBL(i)[0] = (((u32)addr->sa_data[0] & 0xFF) << 24) | (((u32)addr->sa_data[1] & 0xFF) << 16) | (((u32)addr->sa_data[2] & 0xFF) << 8) | ((u32)addr->sa_data[3] & 0xFF);
            ROUT_MAC_CFG_TBL(i)[1] = (((u32)addr->sa_data[4] & 0xFF) << 24) | (((u32)addr->sa_data[5] & 0xFF) << 16);
            break;
        }
#endif

    memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    return 0;
}

static int ptm_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    switch ( cmd )
    {
#if 0
    case IFX_PTM_MIB_CW_GET:
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxNoIdleCodewords   = PTM_MIB_TABLE(0)->wrx_nonidle_cw + PTM_MIB_TABLE(1)->wrx_nonidle_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxIdleCodewords     = PTM_MIB_TABLE(0)->wrx_idle_cw + PTM_MIB_TABLE(1)->wrx_idle_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifRxCodingViolation   = PTM_MIB_TABLE(0)->wrx_err_cw + PTM_MIB_TABLE(1)->wrx_err_cw;
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxNoIdleCodewords   = 0;    //  not available
        ((PTM_CW_IF_ENTRY_T *)ifr->ifr_data)->ifTxIdleCodewords     = 0;    //  not available
        break;
    case IFX_PTM_MIB_FRAME_GET:
        {
            PTM_FRAME_MIB_T data = {0};

            data.RxCorrect   = PTM_MIB_TABLE(0)->wrx_correct_pdu + PTM_MIB_TABLE(1)->wrx_correct_pdu;
            data.RxDropped   = PTM_MIB_TABLE(0)->wrx_nodesc_drop_pdu + PTM_MIB_TABLE(0)->wrx_len_violation_drop_pdu + PTM_MIB_TABLE(1)->wrx_nodesc_drop_pdu + PTM_MIB_TABLE(1)->wrx_len_violation_drop_pdu;
            data.TxSend      = PTM_MIB_TABLE(0)->wtx_total_pdu + PTM_MIB_TABLE(1)->wtx_total_pdu;
            data.TC_CrcError = 0;   //  not available

            *((PTM_FRAME_MIB_T *)ifr->ifr_data) = data;
        }
        break;
    case IFX_PTM_CFG_GET:
        //  use bear channel 0 preemption gamma interface settings
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent = PTM_CRC_CFG->wrx_fcs_crc_vld;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck   = PTM_CRC_CFG->wrx_fcs_crc_chk;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck    = PTM_CRC_CFG->wrx_tc_crc_chk;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen      = PTM_CRC_CFG->wrx_tc_crc_len;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen     = PTM_CRC_CFG->wtx_fcs_crc_gen;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen      = PTM_CRC_CFG->wtx_tc_crc_gen;
        ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen      = PTM_CRC_CFG->wtx_tc_crc_len;
        break;
    case IFX_PTM_CFG_SET:
        PTM_CRC_CFG->wrx_fcs_crc_vld = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcPresent;
        PTM_CRC_CFG->wrx_fcs_crc_chk = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxEthCrcCheck;
        PTM_CRC_CFG->wrx_tc_crc_chk  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcCheck;
        PTM_CRC_CFG->wrx_tc_crc_len  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->RxTcCrcLen;
        PTM_CRC_CFG->wtx_fcs_crc_gen = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxEthCrcGen;
        PTM_CRC_CFG->wtx_tc_crc_gen  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcGen;
        PTM_CRC_CFG->wtx_tc_crc_len  = ((IFX_PTM_CFG_T *)ifr->ifr_data)->TxTcCrcLen;
        break;
    case IFX_PTM_MAP_PKT_PRIO_TO_Q:
        break;
#endif
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

static void ptm_tx_timeout(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    priv->tx_errors++;

    netif_wake_queue(dev);

    return;
}

static int ptm_push(struct sk_buff *skb, struct flag_header *header, unsigned int ifid)
{
    struct eth_priv_data *priv;

    if ( ifid != 7 )
        return -EINVAL;

    if ( !g_ptm_net_dev[0] )    //  Ethernet WAN mode, PTM device is not initialized, but it might get ingress packet, just drop the packet.
        return -EIO;

    dump_skb(skb, skb->len, __FUNCTION__, 0, 0, 0);

    priv = ifx_eth_fw_netdev_priv(g_ptm_net_dev[0]);
    if ( netif_running(g_ptm_net_dev[0]) )
    {
        skb->dev = g_ptm_net_dev[0];
        skb->protocol = eth_type_trans(skb, g_ptm_net_dev[0]);

        if ( netif_rx(skb) == NET_RX_DROP )
        {
            priv->rx_dropped++;
        }
        else
        {
            priv->rx_packets++;
            priv->rx_bytes += skb->len;
        }
    }
    else
    {
        dev_kfree_skb_any(skb);
        priv->rx_dropped++;
    }

    return 0;
}

#ifdef CONFIG_IFX_LED
static void dsl_led_flash(void)
{
    if ( g_data_led_trigger != NULL )
        ifx_led_trigger_activate(g_data_led_trigger);
}

static void dsl_led_polling(unsigned long arg)
{
    unsigned int wrx_bc_user_cw, total_tx_cw;

    wrx_bc_user_cw = *RECEIVE_NON_IDLE_CELL_CNT(0, g_vrx218_dev_us.phy_membase) + *RECEIVE_NON_IDLE_CELL_CNT(1, g_vrx218_dev_us.phy_membase);
    total_tx_cw = *TRANSMIT_CELL_CNT(0, g_vrx218_dev_us.phy_membase) + *TRANSMIT_CELL_CNT(1, g_vrx218_dev_us.phy_membase);
    if ( g_dsl_bonding ) {
        wrx_bc_user_cw += *RECEIVE_NON_IDLE_CELL_CNT(0, g_vrx218_dev_ds.phy_membase) + *RECEIVE_NON_IDLE_CELL_CNT(1, g_vrx218_dev_ds.phy_membase);
        total_tx_cw += *TRANSMIT_CELL_CNT(0, g_vrx218_dev_ds.phy_membase) + *TRANSMIT_CELL_CNT(1, g_vrx218_dev_ds.phy_membase);
    }

    if ( wrx_bc_user_cw != g_wrx_bc_user_cw || total_tx_cw != g_total_tx_cw ) {
        g_wrx_bc_user_cw = wrx_bc_user_cw;
        g_total_tx_cw = total_tx_cw;

        dsl_led_flash();
    }

    if ( g_ptm_net_dev[0] != NULL && netif_running(g_ptm_net_dev[0]) )
    {
        g_dsl_led_polling_timer.expires = jiffies + HZ / 10;    //  100ms
        add_timer(&g_dsl_led_polling_timer);
    }
}
#endif

static irqreturn_t mailbox_irq_handler(int irq, void *dev_id)
{
    u32 mailbox0_signal = 0;
    u32 mailbox1_signal = 0;
    volatile uint32_t peer_state;

    //Disable IMCU for mailbox 0 and mailbox1
    *IMCU_USM_IMER &= ~0x03;

    while((mailbox1_signal = *MBOX_IGU1_ISR & *MBOX_IGU1_IER) != 0){    
        
        *MBOX_IGU1_ISRC = mailbox1_signal;

        if ( (mailbox1_signal & 0x01) ) {//Bit 0
            *MBOX_IGU1_IER &= ~0x1;
            enable_vrx218_dma_tx(1);
        }

        if ( (mailbox1_signal & 0x04) ) {//Bit 2
            /* TODO: swap queue */
        }

        if ( (mailbox1_signal & 0x08 ) ) {//Bit 3 EDMA Hang
            g_pcie_reset_sig = 1;
        }

        if ( (mailbox1_signal & 0x10 ) ) {//Bit 4  Peer to Peer link state update
            peer_state = *SOC_ACCESS_VRX218_SB(__PEER_GIF_LINK_STATE_TMP,g_vrx218_dev_us.phy_membase);
            *SOC_ACCESS_VRX218_SB(__PEER_GIF_LINK_STATE,g_vrx218_dev_ds.phy_membase) = peer_state;
        }
    }

    /* Clear TX interrupt at this moment.
     * Implement flow control mechansim if there is specific requirement.
     */
    mailbox0_signal = *MBOX_IGU0_ISR & *MBOX_IGU0_IER;
    *MBOX_IGU0_ISRC = mailbox0_signal;

   *IMCU_USM_IMER |= 0x03;
    
    return IRQ_HANDLED;
}

static irqreturn_t mailbox_vrx218_dsm_irq_handler(int irq, void *dev_id)
{
    u32 mailbox0_signal = 0;
    u32 mailbox1_signal = 0;
    volatile uint32_t peer_state;

    *IMCU_DSM_IMER &= ~0x03;
   

    while((mailbox1_signal = *MBOX_IGU1_DSM_ISR & *MBOX_IGU1_DSM_IER) != 0){
        *MBOX_IGU1_DSM_ISRC = mailbox1_signal;

        if ( (mailbox1_signal & 0x01) ) {//Bit 0
            *MBOX_IGU1_DSM_IER &= ~0x01;
            enable_vrx218_dma_tx(1);
        }

        if ( (mailbox1_signal & 0x04) ) {//Bit 2
            /* TODO: swap queue */
        }

        if ( (mailbox1_signal & 0x08 ) ) {//Bit 3  EDMA HANG
            g_pcie_reset_sig = 1;
        }

        if ( (mailbox1_signal & 0x10 ) ) {//Bit 4  Peer to Peer link state update
            peer_state = *SOC_ACCESS_VRX218_SB(__PEER_GIF_LINK_STATE_TMP,g_vrx218_dev_ds.phy_membase);
            *SOC_ACCESS_VRX218_SB(__PEER_GIF_LINK_STATE,g_vrx218_dev_us.phy_membase) = peer_state;
        }
    }

    /* Clear TX interrupt at this moment.
     * Implement flow control mechansim if there is specific requirement.
     */
    mailbox0_signal = *MBOX_IGU0_DSM_ISR & *MBOX_IGU0_DSM_IER;
    *MBOX_IGU0_DSM_ISRC = mailbox0_signal;
    
    *IMCU_DSM_IMER |= 0x03;

    return IRQ_HANDLED;
}

int proc_read_pcie_rst(struct seq_file *seq, void *v)
{
    int len = 0;

    seq_printf(seq, "%d\n", g_pcie_reset_sig);


    return len;
}


static void dump_skb(struct sk_buff *skb, u32 len, const char *title, int port, int ch, int is_tx)
{
#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
    int i;

    if (!g_dump_cnt || !(g_dbg_enable & (is_tx ? DBG_ENABLE_MASK_DUMP_SKB_TX : DBG_ENABLE_MASK_DUMP_SKB_RX)) )
        return;

    if ( g_dump_cnt > 0 )
        g_dump_cnt--;

    if ( skb->len < len )
        len = skb->len;

    if ( ch >= 0 )
        printk("%s (port %d, ch %d)\n", title, port, ch);
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
#endif
}

static int swap_mac(unsigned char *data)
{
#if defined(DEBUG_SWAP_MAC) && DEBUG_SWAP_MAC
    int ret = 0;

    if ( (g_dbg_enable & DBG_ENABLE_MASK_MAC_SWAP) )
    {
        unsigned char tmp[8];
        unsigned char *p = data;

        if ( p[12] == 0x08 && p[13] == 0x06 ) { //  arp
            if ( p[14] == 0x00 && p[15] == 0x01 && p[16] == 0x08 && p[17] == 0x00 && p[20] == 0x00 && p[21] == 0x01 ) {
                //  dest mac
                memcpy(p, p + 6, 6);
                //  src mac
                p[6] = p[7] = 0;
                memcpy(p + 8, p + 38, 4);
                //  arp reply
                p[21] = 0x02;
                //  sender mac
                memcpy(p + 22, p + 6, 6);
                //  sender IP
                memcpy(tmp, p + 28, 4);
                memcpy(p + 28, p + 38, 4);
                //  target mac
                memcpy(p + 32, p, 6);
                //  target IP
                memcpy(p + 38, tmp, 4);

                ret = 42;
            }
        }
        else if ( !(p[0] & 0x01) ) {    //  bypass broadcast/multicast
            //  swap MAC
            memcpy(tmp, p, 6);
            memcpy(p, p + 6, 6);
            memcpy(p + 6, tmp, 6);
            p += 12;

            //  bypass VLAN
            while ( p[0] == 0x81 && p[1] == 0x00 )
                p += 4;

            //  IP
            if ( p[0] == 0x08 && p[1] == 0x00 ) {
                p += 14;
                memcpy(tmp, p, 4);
                memcpy(p, p + 4, 4);
                memcpy(p + 4, tmp, 4);
                p += 8;
            }

            ret = (int)((unsigned long)p - (unsigned long)data);
        }
    }

    return ret;
#else
    return 0;
#endif
}


static int in_showtime(void)
{
    int i;

    for(i = 0; i < LINE_NUMBER; i ++){
        if(g_showtime[i]){
            return 1;
        }
    }

    return 0;
}

static int dsl_showtime_enter(const unsigned char line_idx,struct port_cell_info *port_cell, void *xdata_addr)
{
    ASSERT(line_idx < LINE_NUMBER, "line_idx: %d large than max line num: %d", line_idx, LINE_NUMBER);
    
    g_showtime[line_idx] = 1;

    dbg("line[%d]:enter showtime", line_idx);

    return PPA_SUCCESS;
}

static int dsl_showtime_exit(const unsigned char line_idx)
{
    ASSERT(line_idx < LINE_NUMBER, "line_idx: %d large than max line num: %d", line_idx, LINE_NUMBER);
    if ( !g_showtime[line_idx] )
        return PPA_FAILURE;

    g_showtime[line_idx] = 0;

    dbg("line[%d]:leave showtime", line_idx);

    return PPA_SUCCESS;
}

static void check_showtime(void)
{
    int i;
    struct port_cell_info port_cell = {0};
    int showtime = 0;
    void *xdata_addr = NULL;
    int ret = -1;
    ltq_mei_atm_showtime_check_t ltq_mei_atm_showtime_check = NULL;

    ltq_mei_atm_showtime_check = (ltq_mei_atm_showtime_check_t)ppa_callback_get(LTQ_MEI_SHOWTIME_CHECK);
    if (!ltq_mei_atm_showtime_check) {
        return;
    }

    for(i = 0; i < LINE_NUMBER; i++){
        ret = ltq_mei_atm_showtime_check(i, &showtime, &port_cell, &xdata_addr);
        if(!ret && showtime){
            dsl_showtime_enter(i,&port_cell, &xdata_addr);
        }
    }

    return;
}


int __init vrx218_ptm_datapath_init(const ifx_pcie_ep_dev_t *p_vrx218_dev_us, const ifx_pcie_ep_dev_t *p_vrx218_dev_ds)
{
    int ret;
    
#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
    struct ifx_eth_fw_netdev_ops ptm_ops = {
        .get_stats      = ptm_get_stats,
        .open           = ptm_open,
        .stop           = ptm_stop,
        .start_xmit     = ptm_qos_hard_start_xmit,
        .set_mac_address= ptm_set_mac_address,
        .do_ioctl       = ptm_ioctl,
        .tx_timeout     = ptm_tx_timeout,
        .watchdog_timeo = ETH_WATCHDOG_TIMEOUT,
    };
#endif
    ltq_mei_atm_showtime_check_t ltq_mei_atm_showtime_check = NULL;

    g_vrx218_dev_us = *p_vrx218_dev_us;
    g_vrx218_dev_ds = *p_vrx218_dev_ds;

    if ( g_vrx218_dev_us.phy_membase != g_vrx218_dev_ds.phy_membase ) {
        g_dsl_bonding = 2;
#ifdef CONFIG_LANTIQ_ETH_FRAMEWORK
        ptm_ops.start_xmit = ptm_qos_hard_start_xmit_b;
#endif
    }

#ifdef CONFIG_IFX_LED
    setup_timer(&g_dsl_led_polling_timer, dsl_led_polling, 0);
#endif

    g_ptm_net_dev[0] = ifx_eth_fw_alloc_netdev(sizeof(struct eth_priv_data), "ptm0", &ptm_ops);
    if ( g_ptm_net_dev[0] == NULL ) {
        ret = -ENOMEM;
        goto ALLOC_NETDEV_PTM_FAIL;
    }
    ptm_setup(g_ptm_net_dev[0]);
    ret = ifx_eth_fw_register_netdev(g_ptm_net_dev[0]);
    if ( ret != 0 )
        goto RETISTER_NETDEV_PTM_FAIL;

    /* request irq (enable by default) */
    if(g_dsl_bonding){
        ret = request_irq(g_vrx218_dev_ds.irq, mailbox_vrx218_dsm_irq_handler, IRQF_DISABLED, "vrx318_ppe_isr", NULL);
        if( ret != 0 ){
            printk(KERN_ERR "Failed to request PCIe MSI irq %s:%u\n", "vrx318 DS ",g_vrx218_dev_ds.irq);
            goto REQUEST_IRQ_FAIL;
        }
    }
    ret = request_irq(g_vrx218_dev_us.irq, mailbox_irq_handler, IRQF_DISABLED, "vrx318_ppe_isr", NULL);
    if ( ret != 0 ) {
        printk(KERN_ERR "Failed to request PCIe MSI irq %s:%u\n", "vrx318 US ",g_vrx218_dev_us.irq);
        goto REQUEST_IRQ_FAIL;
    }

    enable_vrx218_swap(1, 0, g_dsl_bonding);
    enable_vrx218_dma_rx(1);
    /* call enable_vrx218_dma_tx(1); in mailbox_irq_handler in vrx218_ptm_main.c */

    g_smartphy_push_fn = ptm_push;
    g_smartphy_port_num = 1;

    check_showtime();
    ppa_callback_set(LTQ_MEI_SHOWTIME_ENTER, dsl_showtime_enter);
    ppa_callback_set(LTQ_MEI_SHOWTIME_EXIT, dsl_showtime_exit);

#ifdef CONFIG_IFX_LED
    ifx_led_trigger_register("dsl_data", &g_data_led_trigger);
#endif

    return 0;

REQUEST_IRQ_FAIL:
    printk("PTM datapath initialization fail!!!\n");
    ifx_eth_fw_unregister_netdev(g_ptm_net_dev[0], 0);
RETISTER_NETDEV_PTM_FAIL:
    ifx_eth_fw_free_netdev(g_ptm_net_dev[0], 0);
    g_ptm_net_dev[0] = NULL;
ALLOC_NETDEV_PTM_FAIL:
    return ret;
}

void __exit vrx218_ptm_datapath_exit(void)
{
#ifdef CONFIG_IFX_LED
    del_timer(&g_dsl_led_polling_timer);
    ifx_led_trigger_deregister(g_data_led_trigger);
    g_data_led_trigger = NULL;
#endif

ppa_callback_set(LTQ_MEI_SHOWTIME_ENTER, (void *)NULL);
ppa_callback_set(LTQ_MEI_SHOWTIME_EXIT, (void *)NULL);

    g_smartphy_port_num = 0;
    g_smartphy_push_fn = NULL;

    enable_vrx218_dma_tx(0);
    enable_vrx218_dma_rx(0);
    enable_vrx218_swap(0, 0, 0);

    free_irq(g_vrx218_dev_us.irq, NULL);
    if(g_dsl_bonding){
        free_irq(g_vrx218_dev_ds.irq, NULL);
    }

    ifx_eth_fw_unregister_netdev(g_ptm_net_dev[0], 0);
    ifx_eth_fw_free_netdev(g_ptm_net_dev[0], 0);
    g_ptm_net_dev[0] = NULL;
}
