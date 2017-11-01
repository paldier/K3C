#ifndef __DLRX_DL_WLAN_API_H__
#define __DLRX_DL_WLAN_API_H__
#include <net/ppa_ppe_hal.h>
/*********************************************
 *	Macro definition
 *********************************************/
#define PPA_F_REGISTER		1
#define PPA_F_DEREGISTER	2

/*********************************************
 *	Structure Definition
 *********************************************/
/*
	This is the enumeration for PN check type.
*/
typedef enum {
	/*! No Packet Number Check for this Security Type of VAP */
	PPA_WLAN_NO_PN_CHECK = 0,
	/*! 48-bit Packet Number Check for this Security Type of VAP */
	PPA_WLAN_48_BIT_PN_CHECK = 1,
	/*! 128-bit Even Packet Number Check for WAPI Security Type (TBC) */
	PPA_WLAN_128_BIT_EVEN_PN_CHECK = 2,
	/*! 128-bit Odd Packet Number Check for WAPI Security Type (TBC) */
	PPA_WLAN_128_BIT_ODD_PN_CHECK = 3
} PPA_WLAN_PN_CHECK_Type_t;

/*!
\brief This is the enumeration for Peer configuration
 with Security Type. Used by  \ref ppa_dl_qca_set_peer_cfg.
*/
typedef enum {
	PPA_WLAN_ADD_PEER_ID = 1,
	PPA_WLAN_REMOVE_PEER = 2,
	PPA_WLAN_REMOVE_PEER_ID = 3,
	PPA_WLAN_SET_PN_CHECK = 4,
	PPA_WLAN_SET_PN_CHECK_WITH_RXPN = 5
} PPA_QCA_PEER_ADD_REMOVE_FLAG_Type_t;
/*
* This is the enumeration for Special Received MPDU Status.
* Used by  \ref PPA_QCA_DL_RX_SPL_PKT_FN.
*/
typedef enum {
	/* Inspect Flag Set Rx Packet */
	PPA_WLAN_INSPECT_TYPE = 0x1,
	/* Invalid Peer Packet */
	PPA_WLAN_INV_PEER_TYPE = 0x5,
	/* MIC Error Packet */
	PPA_WLAN_MIC_ERROR_TYPE = 0x9,
} PPA_WLAN_SPL_PKT_Type_t;

/*
* This is the data structure for Message Statistics Counters.
* Used by PPA_DL_WLAN_STATS_t.
*/
typedef struct {
	uint32_t ce4_cpu_msgs;
	uint32_t ce5_cpu_msgs;
	uint32_t rx_ind_msgs;
	uint32_t rx_flush_msgs;
	uint32_t tx_comp_msgs;
	uint32_t rx_ind_wl_msgs;
	uint32_t rx_flush_wl_msgs;
	uint32_t rx_frag_msgs;
} PPA_DL_WLAN_MSG_STATS_t;

/*
* This is the data structure for Receive MPDU & MSDU
* related Statistics Counters - Successful and Error. Used by
* PPA_DL_WLAN_STATS_t.
*/
typedef struct {
	uint32_t rx_mpdu_ok;
	uint32_t rx_msdu_ok;
	uint32_t rx_mpdu_err2;
	uint32_t rx_msdu_err2;
	uint32_t rx_mpdu_err3;
	uint32_t rx_msdu_err3;
	uint32_t rx_mpdu_err4;
	uint32_t rx_msdu_err4;
	uint32_t rx_mpdu_err5;
	uint32_t rx_msdu_err5;
	uint32_t rx_mpdu_err6;
	uint32_t rx_msdu_err6;
	uint32_t rx_mpdu_err7;
	uint32_t rx_msdu_err7;
	uint32_t rx_mpdu_err8;
	uint32_t rx_msdu_err8;
	uint32_t rx_mpdu_err9;
	uint32_t rx_msdu_err9;
	uint32_t rx_mpdu_errA;
	uint32_t rx_msdu_errA;
} PPA_DL_WLAN_RX_MPDU_MSDU_STATS_t;

typedef int32_t (*PPA_QCA_DL_RX_MSG_FN)(
	void *reg_handle,
	uint32_t msg_type,
	uint32_t msg_len,
	uint32_t *msg,
	uint32_t flags
	);
typedef int32_t (*PPA_QCA_DL_RX_SPL_PKT_FN)(
	void *reg_handle,
	uint32_t pkt_status,
	uint32_t pkt_len,
	struct sk_buff *pkt_skb,
	uint32_t *msg,
	uint32_t flags
	);
typedef int32_t (*PPA_QCA_DL_VAP_STATS_GET_FN)(
	void *reg_handle,
	uint16_t vap_id,
	PPA_WLAN_VAP_Stats_t *vap_stats,
	uint32_t flags
	);
#ifdef CONFIG_SOC_GRX500
typedef int32_t (*PPA_QCA_DL_RX_MARK_PEER_ACTIVE)(
	void *reg_handle,
	uint32_t peer_id
	);
#endif /* CONFIG_SOC_GRX500 */

/*
 * This is the data structure for QCA specific
 * PPA Direct Link Callbacks registration,
 * which provides the necessary callback to PPA DL Driver.
 * Used by \ref ppa_dl_qca_register.
 */
typedef struct {
	/* Pointer to QCA Driver Rx Msg function callback. */
	PPA_QCA_DL_RX_MSG_FN		 rx_msg_fn;
	/* Pointer to QCA Driver Rx Special Packets function callback */
	PPA_QCA_DL_RX_SPL_PKT_FN	 rx_splpkt_fn;
	/* Pointer to QCA Driver Statistics Query fucniton Callback */
	PPA_QCA_DL_VAP_STATS_GET_FN  vap_stats_fn;
#ifdef CONFIG_SOC_GRX500	
	/* Pointer to QCA Driver supports bandwidth steering */
	PPA_QCA_DL_RX_MARK_PEER_ACTIVE peer_act_fn;
#endif
} PPA_QCA_DL_RX_CB;



extern void ppa_dl_qca_register(
	void *dl_rx_handle,
	PPA_QCA_DL_RX_CB *dl_qca_rxcb,
	uint32_t flags
	);
extern void ppa_dl_qca_t2h_ring_init(
	uint32_t *t2h_ring_sz,
	uint32_t *dst_ring_base,
	uint32_t pcie_baddr,
	uint32_t flags
	);
extern void ppa_dl_qca_t2h_pktbuf_pool_manage(
	uint32_t *alloc_idx_ptr,
	uint32_t *t2h_rxpb_ring_sz,
	uint32_t *rxpb_ring_base,
	uint32_t flags
	);

extern int32_t ppa_hook_dl_qca_rx_offload(
	uint32_t flags
	);

/* This new function added to get the next rxpb pointer */
extern int32_t ppa_dl_qca_get_rx_net_buf(
	struct sk_buff **rx_skb,
	uint32_t flags
	);

extern void ppa_directlink_manage(
	char *name,
	uint32_t flags
	);

extern int32_t ppa_dl_qca_set_peer_cfg(
	uint32_t *dlrx_peer_reg_handle,
	uint16_t peer_id,
	uint16_t vap_id,
	PPA_WLAN_PN_CHECK_Type_t pn_chk_type,
	uint32_t *rxpn,
	uint32_t flags
#ifdef CONFIG_SOC_GRX500
	, uint8_t *mac_addr
#endif
	);
extern int32_t ppa_dl_qca_set_seq_mask(
	uint32_t *dlrx_peer_reg_handle,
	uint32_t ex_tid,
	uint32_t seq_mask,
	uint32_t flags
	);

/* API to clear vap's mib */
extern int ppa_dl_qca_clear_stats(
	uint32_t vapId,
	uint32_t flags
	);

#ifdef CONFIG_SOC_GRX500
extern void ppa_dl_qca_ipi_interrupt(void);
#endif /* CONFIG_SOC_GRX500 */

#endif
