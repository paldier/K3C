#ifndef __PPA_P7_LPDP_API_H__
#define __PPA_P7_LPDP_API_H__

#include <linux/skbuff.h>
#include <linux/netdevice.h>



struct netops_cb {
	int (*netops_rx)(struct sk_buff *skb);
	int (*netops_rx_ni)(struct sk_buff *skb);
	int (*netops_dev_register)(struct net_device *dev);
	void (*netops_dev_unregister)(struct net_device *dev);
	void (*netops_start_queue)(struct net_device *dev);
	void (*netops_stop_queue)(struct net_device *dev);
};

#define LPDP_SKB_CB(skb) ((struct lpdp_skb_cb *)((skb)->cb))

struct lpdp_skb_cb {
	uint8_t vpid;
};

static uint8_t inline
ppa_lpdp_skb_get_vpid(struct sk_buff *skb)
{
	return LPDP_SKB_CB(skb)->vpid;
}

static void inline
ppa_lpdp_skb_set_vpid(struct sk_buff *skb, uint8_t vpid)
{
	LPDP_SKB_CB(skb)->vpid = vpid;
}

#endif
