#ifndef __PPA_LPDP_API_H__
#define __PPA_LPDP_API_H__

#include <net/ppa_api.h>
#include <net/ppa_stack_al.h>
#include <net/ppa_api_directpath.h>
#include <net/ppa_api_misc.h>
#include <linux/skbuff.h>


#define LOG_PRINT(msg, arg...) printk(KERN_CRIT "[%s:%d] "msg, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERROR                err
#define LOG_INFO(msg, arg...)    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, msg, ##arg)
#define LOG_DEBUG(msg, arg...)   ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, msg, ##arg)
#define LOG_TRACE(msg, arg...)   ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, msg, ##arg)

enum {
	PPA_LPDP_F_REGISTER = 0x1,
	PPA_LPDP_F_DEREGISTER = 0x2,
	/* = 0x4 */
};

#define MAC_FMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MAC_ADDR(m) \
	((uint8_t *)(m))[0], \
	((uint8_t *)(m))[1], \
	((uint8_t *)(m))[2], \
	((uint8_t *)(m))[3], \
	((uint8_t *)(m))[4], \
	((uint8_t *)(m))[5]

#define	MAX_PID		32
#define	MAX_VPID	51

#define DP_DATA_INDEX(__p)	((__p)->port_id * (MAX_VPID+1) + ((__p)->subif > 0 ? (__p)->subif : 0))
#define DP_PORT_ID(_idx)	((_idx) ? ((_idx) / (MAX_VPID+1)) : 0)
#define DP_SUBIF(_idx)		((_idx) ? ((_idx) % (MAX_VPID+1)) : 0)

struct ppa_lpdp_ifstats {
	uint64_t rx_pkts;
	uint64_t tx_pkts;
	uint64_t rx_bytes;
	uint64_t tx_bytes;
	uint64_t droped_pkts;
	/* ... */
};

typedef int32_t (*ppa_lpdp_regsiter_cb)(PPA_SUBIF *psubif, PPA_NETIF * netif, PPA_DIRECTPATH_CB *cb, uint32_t *index, uint32_t flags);

typedef int32_t (*ppa_lpdp_send_cb)(PPA_SUBIF *p_subif, struct sk_buff *skb, int32_t len, uint32_t flags);

typedef struct sk_buff* (*ppa_lpdp_allocskb_cb)(PPA_SUBIF *p_subif, int32_t len, uint32_t flags);

typedef int32_t (*ppa_lpdp_recycleskb_cb)(PPA_SUBIF *p_subif, struct sk_buff *skb, uint32_t flags);

typedef int32_t (*ppa_lpdp_flowctrl_cb)(PPA_SUBIF *p_subif, uint32_t flags);

typedef int32_t (*ppa_lpdp_stats_cb)(PPA_SUBIF *p_subi, struct ppa_lpdp_ifstats* stats);

struct ppa_lpdp_cb {
	ppa_lpdp_regsiter_cb cb_register;
	ppa_lpdp_send_cb cb_send;
	ppa_lpdp_allocskb_cb cb_alloc_skb;
	ppa_lpdp_recycleskb_cb cb_recycle_skb;
	ppa_lpdp_flowctrl_cb cb_flowctrl;
	ppa_lpdp_stats_cb	cb_stats;
};

extern struct ppe_directpath_data *ppa_drv_g_ppe_directpath_data;
extern int32_t ppa_litepath_ll_register(struct ppa_lpdp_cb* lpcb, int flags);
extern int32_t ppa_drv_lpdp_directpath_register(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, int32_t *index, uint32_t flags);
extern int32_t ppa_drv_lpdp_directpath_send(PPA_SUBIF *p_subif, struct sk_buff *skb, int32_t len, uint32_t flags);
extern int32_t ppa_drv_lpdp_directpath_flowctrl(PPA_SUBIF *psubif, uint32_t flags);
extern PPA_BUF *ppa_drv_lpdp_directpath_alloc_skb(PPA_SUBIF *subif, int32_t len, uint32_t flags);
extern int32_t ppa_drv_lpdp_directpath_recycle_skb(PPA_SUBIF *subif, PPA_BUF *skb, uint32_t flags);

#if 0
/* Exposed API to get lpdp callback reference */
extern ppa_lpdp_regsiter_cb		LPDP_REGISTER_CB(void);
extern ppa_lpdp_send_cb			LPDP_SEND_CB(void);
extern ppa_lpdp_allocskb_cb		LPDP_ALLOC_SKB_CB(void);
extern ppa_lpdp_recycleskb_cb	LPDP_RECYCLE_SKB_CB(void);
extern ppa_lpdp_flowctrl_cb		LPDP_FLOWCTRL_CB(void);
extern ppa_lpdp_stats_cb		LPDP_STATS_CB(void);
#endif

#endif
