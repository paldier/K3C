#include "lpdp_api.h"

static struct ppa_lpdp_cb g_lpdp_cb;

int32_t
ppa_litepath_ll_register(struct ppa_lpdp_cb* lpcb, int flags)
{
	if (!lpcb) {
		LOG_ERROR("lpcb NULL !!\n");
		return PPA_FAILURE;
	} else if (flags == PPA_LPDP_F_REGISTER) {
		memcpy(&g_lpdp_cb, lpcb, sizeof(g_lpdp_cb));
	} else if (flags == PPA_LPDP_F_DEREGISTER) {
		memset(&g_lpdp_cb, 0, sizeof(g_lpdp_cb));
	} else {
		LOG_ERROR("Invalid flags: 0x%X\n", flags);
		return PPA_FAILURE;
	}

	return PPA_SUCCESS;
}
EXPORT_SYMBOL(ppa_litepath_ll_register);

#if 0
/* Exposed API to get lpdp callback reference */
ppa_lpdp_regsiter_cb	LPDP_REGISTER_CB(void)		{ return g_lpdp_cb.cb_register; }
ppa_lpdp_send_cb		LPDP_SEND_CB(void)			{ return g_lpdp_cb.cb_send; }
ppa_lpdp_allocskb_cb	LPDP_ALLOC_SKB_CB(void)		{ return g_lpdp_cb.cb_alloc_skb; }
ppa_lpdp_recycleskb_cb	LPDP_RECYCLE_SKB_CB(void)	{ return g_lpdp_cb.cb_recycle_sbk; }
ppa_lpdp_flowctrl_cb	LPDP_FLOWCTRL_CB(void) 		{ return g_lpdp_cb.cb_flowctrl; }
ppa_lpdp_stats_cb		LPDP_STATS_CB(void) 		{ return g_lpdp_cb.cb_stats; }

EXPORT_SYMBOL(LPDP_REGISTER_CB);
EXPORT_SYMBOL(LPDP_SEND_CB);
EXPORT_SYMBOL(LPDP_ALLOC_SKB_CB);
EXPORT_SYMBOL(LPDP_RECYCLE_SKB_CB);
EXPORT_SYMBOL(LPDP_FLOWCTRL_CB);
EXPORT_SYMBOL(LPDP_STATS_CB);
#endif

int32_t
ppa_drv_lpdp_directpath_register(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, int32_t *index, uint32_t flags)
{
	if (g_lpdp_cb.cb_register == NULL) return PPA_EINVAL;
	return g_lpdp_cb.cb_register(subif, netif, pDirectpathCb, index, flags);
}
EXPORT_SYMBOL(ppa_drv_lpdp_directpath_register);

int32_t
ppa_drv_lpdp_directpath_send(PPA_SUBIF *subif, struct sk_buff *skb, int32_t len, uint32_t flags)
{
    if (g_lpdp_cb.cb_send == NULL) return PPA_EINVAL;
    return g_lpdp_cb.cb_send(subif, skb, len, flags);
}
EXPORT_SYMBOL(ppa_drv_lpdp_directpath_send);

int32_t
ppa_drv_lpdp_directpath_flowctrl(PPA_SUBIF *subif, uint32_t flags)
{
    if (g_lpdp_cb.cb_flowctrl == NULL) return PPA_EINVAL;
    return g_lpdp_cb.cb_flowctrl(subif, flags);
}
EXPORT_SYMBOL(ppa_drv_lpdp_directpath_flowctrl);

PPA_BUF *
ppa_drv_lpdp_directpath_alloc_skb(PPA_SUBIF *subif, int32_t len, uint32_t flags)
{
    if (g_lpdp_cb.cb_alloc_skb == NULL) return (PPA_BUF *)PPA_EINVAL;
    return g_lpdp_cb.cb_alloc_skb(subif, len, flags);
}
EXPORT_SYMBOL(ppa_drv_lpdp_directpath_alloc_skb);

int32_t
ppa_drv_lpdp_directpath_recycle_skb(PPA_SUBIF *subif, PPA_BUF *skb, uint32_t flags)
{
    if (g_lpdp_cb.cb_recycle_skb == NULL) return PPA_EINVAL;
    return g_lpdp_cb.cb_recycle_skb(subif, skb, flags);
}
EXPORT_SYMBOL(ppa_drv_lpdp_directpath_recycle_skb);
