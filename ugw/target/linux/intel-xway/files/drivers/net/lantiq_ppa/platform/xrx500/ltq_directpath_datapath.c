/******************************************************************************
**
** FILE NAME    : lantiq_directpath_drv.c
** PROJECT      : Lantiq UEIP
** MODULES      : Lantiq CPE directpath driver
** DATE         : 3 Dec  2014
** AUTHOR       : Purnendu Ghosh
** DESCRIPTION  : Lantiq directpath driver for XRX500 series
** COPYRIGHT    :       Copyright (c) 2014
**                      Lantiq Deutschland
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date                $Author                 $Comment
*******************************************************************************/
#include <linux/version.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/kernel.h> /* printk() */
#include <linux/types.h>  /* size_t */
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/proc_fs.h>
#include <linux/etherdevice.h> /* eth_type_trans */
#include <asm/delay.h>
#include <linux/init.h>
#include <linux/clk.h>

#include <linux/of_net.h>
#include <linux/of_gpio.h>

#include <net/ppa_api.h>
#include "../ppa_datapath.h"
#include <net/ppa_stack_al.h>
#include <net/ppa_api_directpath.h>
//#include "ltq_tmu_hal_dp_connectivity.h"
#include <net/datapath_api.h>
#include <net/lantiq_cbm.h>
#include <xway/switch-api/lantiq_gsw_api.h>

GSW_API_HANDLE ltq_ethsw_api_kopen(char *name);
int ltq_ethsw_api_kioctl(GSW_API_HANDLE handle, u32 command, u32 arg);
GSW_API_HANDLE ltq_ethsw_api_kclose(char *name);

/*
 *  variable for directpath
 */
#define     MAX_DIRECTPATH_NUM      16
#define     MAX_SUBIF_NUM      17
struct ppe_directpath_data g_ppe_directpath_data[MAX_DIRECTPATH_NUM * MAX_SUBIF_NUM];
struct module g_ltq_dp_module[MAX_DIRECTPATH_NUM];
int32_t g_dp_index = 0;

struct q_connect {
	int32_t egress_q;
	int32_t ingress_q;
	int32_t no_of_subif;
};

struct q_connect g_q_conn_status[MAX_DIRECTPATH_NUM];

static int32_t dp_cb_rx (struct net_device *, struct net_device *, struct sk_buff *, int32_t);
#define dp_err printk
#define dbg_info //printk

#define DRV_MODULE_NAME             "lantiq_directpath_drv_xrx500"
#define DRV_MODULE_VERSION          "1.1.1"
static char version[] =
        DRV_MODULE_NAME ".c:v" DRV_MODULE_VERSION " \n";

//#define DUMP_PACKET
//#define TX_DUMP_PACKET

#define NEED_PMAC_HEADER_REMOVEL

#ifdef NEED_PMAC_HEADER_REMOVEL
/*
    This structure is used internal purpose
*/
typedef struct
{
    u32 tcp_checksum        :1;  /*!< Reserved bits*/
    u32 ver_done            :1;  /*!< IP Offset */
    u32 ip_offset           :6; /*!< Destination Group and Port Map */
    u32 tcp_h_offset        :5; /*!< Source Logical Port ID */
    u32 tcp_type            :3; /*!< Reserved bits*/
    u32 sppid               :4; /*!< Is Tagged */
    u32 class_id            :4; /*!< Reserved bits*/
    u32 port_map_en         :1; /*!< Reserved bits*/
    u32 port_map_sel        :1; /*!< PPPoE Session Packet */
    u32 lrn_dis             :1; /*!< IPv6 Packet */
    u32 class_en            :1; /*!< IPv4 Packet */
    u32 reserved            :2; /*!< Mirrored */
    u32 pkt_type            :2; /*!< Reserved bits*/
    u32 crcgen_dis          :1; /* Packet Length High Bits */
    u32 redirect            :1; /* Packet Length Low Bits */
    u32 timestamp           :1; /*!< Reserved bits*/
    u32 sub_if_sc_hi        :5; /*!< Reserved bits*/
    u32 sub_if_sc_lo        :8; /*!< Source Physical Port ID */
    u32 port_map_hi         :8; /*!< Traffic Class */
    u32 port_map_lo         :8; /*!< Traffic Class */
} pmac_header_t;

#endif

#ifdef TX_DUMP_PACKET
/*
* \brief	dump skb data
* \param[in] len length of the data buffer
* \param[in] pData Pointer to data to dump
*
* \return void No Value
*/
static inline void dump_skb_tx(u32 len, char *pData){
	int i;
	for(i=0;i<len;i++){
		printk("%2.2x ",(u8)(pData[i]));
		if (i % 16 == 15)
			printk("\n");
	}
	printk("\n");
}
#endif


#ifdef DUMP_PACKET
/*
* \brief	dump skb data
* \param[in] len length of the data buffer
* \param[in] pData Pointer to data to dump
*
* \return void No Value
*/
static inline void dump_skb(u32 len, char *pData){
	int i;
	for(i=0;i<len;i++){
		printk("%2.2x ",(u8)(pData[i]));
		if (i % 16 == 15)
			printk("\n");
	}
	printk("\n");
}
#endif



/* Driver version info */
static inline int directpath_drv_ver(char *buf)
{
    return sprintf(buf, "Lantiq directpath datapath driver for XRX500, version %s,(c)2009 Lantiq AG\n", version);
}

/**   ======================================================== */

/** These API's are used to set the connectivity of the directpath */
/** TMU HAL needs to expose API*/
int32_t ltq_directpath_egress_connect(struct net_device *netdev,
                        uint32_t pmac_port)
{
	dbg_info (" Setup Egress Connectivity for Directpath port!!!\n");
	tmu_hal_setup_dp_egress_connectivity(netdev, pmac_port);
	dbg_info (" Updating egress connection status for pmac port %d!!!\n",pmac_port);
	g_q_conn_status[pmac_port].egress_q =1;
	return 0;
}

int32_t ltq_directpath_ingress_connect(struct net_device *netdev,
                        uint32_t pmac_port)
{
	dbg_info (" Setup Ingress Connectivity for Directpath port!!!\n");
	tmu_hal_setup_dp_ingress_connectivity(netdev, pmac_port);
	dbg_info (" Updating ingress connection status for pmac port %d!!!\n",pmac_port);
	g_q_conn_status[pmac_port].ingress_q =1;
	return 0;

}

int32_t ltq_directpath_egress_disconnect(struct net_device *netdev,
                        uint32_t pmac_port)
{
	dbg_info (" Remove Egress Connectivity for Directpath port!!!\n");
	tmu_hal_remove_dp_egress_connectivity(netdev, pmac_port);
	g_q_conn_status[pmac_port].egress_q =0;
	return 0;
}

int32_t ltq_directpath_ingress_disconnect(struct net_device *netdev,
                        uint32_t pmac_port)
{
	dbg_info (" Remove Ingress Connectivity for Directpath port!!!\n");
	tmu_hal_remove_dp_ingress_connectivity(netdev, pmac_port);
	g_q_conn_status[pmac_port].ingress_q =0;
	return 0;

}

/**  ======================================================  */

int32_t ltq_dump_conn_status(void)
{
	int32_t i = 0;
	for(i=0; i < MAX_DIRECTPATH_NUM; i++)
	{
		printk("i=%d Egress %d Ingress %d \n",i,g_q_conn_status[i].egress_q,g_q_conn_status[i].ingress_q);
	}
	return 0;

}

int32_t ltq_directpath_get_free_module_index(void)
{
	int32_t i = 0;
	for(i=0; i < MAX_DIRECTPATH_NUM; i++)
	{
		if(g_ltq_dp_module[i].name[0] != 0)
			printk("g_ltq_dp_module[%d].name %s\n",i,g_ltq_dp_module[i].name);
		else
			break;
	}
	//dbg_info (" free module index %d\n",i);
	return i;

}

int32_t ltq_directpath_match_module_index(char * module_name)
{
	int32_t i = 0;
	for(i=0; i < MAX_DIRECTPATH_NUM; i++)
	{
		printk("g_ltq_dp_module[%d].name %s module name %s\n",i,g_ltq_dp_module[i].name,module_name);
		if(g_ltq_dp_module[i].name[0] != 0) {
			if(!strcmp(&g_ltq_dp_module[i].name[0], module_name)) {
				printk("g_ltq_dp_module[%d].name %s matched with module name %s\n",i,g_ltq_dp_module[i].name,module_name);
				break;
			}
		}
	}
	dbg_info (" matched module index %d\n",i);
	return i;

}

int32_t ltq_directpath_get_free_data_index(void)
{
	int32_t i = 0, j=0;
	for(i=1; i < MAX_DIRECTPATH_NUM ; i++)
	{
		if(g_ppe_directpath_data[j].dp_subif.port_id == 0)
			break;
		j = i*MAX_SUBIF_NUM;
	}
	//dbg_info (" free data index %d\n",j);
	return j;

}

int32_t ltq_directpath_get_data_index_for_port(int32_t portid)
{
	int32_t i = 0, j=0;
	for(i=1; i < MAX_DIRECTPATH_NUM ; i++)
	{
		if(g_ppe_directpath_data[j].dp_subif.port_id == portid)
			break;
		j = i*MAX_SUBIF_NUM;
	}
	//dbg_info ("data index for port %d is %d\n",portid, j);
	return j;

}

int32_t ltq_directpath_get_data_index_from_netdev(struct net_device *netdev)
{
	int32_t i = 0;
	for(i=0; i < MAX_DIRECTPATH_NUM * MAX_SUBIF_NUM ; i++)
	{
		if(g_ppe_directpath_data[i].netif == netdev)
			break;
	}
	//dbg_info ("data index for net device is %d\n", i);
	return i;

}

#ifdef NEED_PMAC_HEADER_REMOVEL
static int32_t dp_cb_rx (struct net_device *rxif, struct net_device *txif, struct sk_buff *skb, int32_t len)
{
	int32_t if_id;
#ifdef DUMP_PACKET
	if (rxif)
		printk("%s: <rxif> rxed a packet from DP lib on interface %s ..\n", __func__, rxif->name);
	else
		printk("%s: <txif> rxed a packet from DP lib on interface %x ..\n", __func__, (unsigned int)rxif);
#endif
	/** Maximum length is changed from 0x600 to 0x10000 for LRO packet support */
	if ((len >= 0x10000) || (len < 60) ) {
		printk("%s[%d]: Packet is too large/small (%d)!!!\n", __func__,__LINE__,len);
		goto rx_err_exit;
	}

#ifdef DUMP_PACKET
	/*if (skb->data) {
		printk("Recvd data len:%d\n",len);
		dump_skb(len, (char *)skb->data);
	}*/
#endif

	len -= (sizeof(pmac_header_t));  /*Remove PMAC to DMA header */
	skb_pull(skb,(sizeof(pmac_header_t)));

	/* Pass it to the stack */
#ifdef DUMP_PACKET
	if (skb->data) {
		printk(" ================ \n");
		printk("Data sent up to the Driver \n");
		dump_skb(len, (char *)skb->data);
		printk(" ================ \n");
	}
#endif
   	if (rxif) {
		if_id = ltq_directpath_get_data_index_from_netdev(rxif);
		if(if_id >= MAX_DIRECTPATH_NUM * MAX_SUBIF_NUM)
			return PPA_FAILURE;
		skb->dev = rxif;
		//skb->protocol = eth_type_trans(skb, rxif);
        	//printk("Driver Callback is called for rxif<ifid: %d> of skb length %d port id %d\n",if_id,len, g_ppe_directpath_data[if_id].dp_subif.port_id);
		g_ppe_directpath_data[if_id].rx_fn_rxif_pkt++;
		g_ppe_directpath_data[if_id].callback.rx_fn(rxif, NULL, skb, skb->len);
		//g_ppe_directpath_data[g_ppe_directpath_data[if_id].dp_subif.port_id * 16].callback.rx_fn(rxif, NULL, skb, skb->len);
	}
	if (txif) {
		if_id = ltq_directpath_get_data_index_from_netdev(txif);
		if(if_id >= MAX_DIRECTPATH_NUM * MAX_SUBIF_NUM)
			return PPA_FAILURE;
		skb->dev = txif;
		//skb->protocol = eth_type_trans(skb, txif);
        	//printk("Driver Callback is called for txif<ifid: %d> of skb length %d port id %d\n",if_id,len, g_ppe_directpath_data[if_id].dp_subif.port_id);
		g_ppe_directpath_data[if_id].rx_fn_txif_pkt++;
		g_ppe_directpath_data[if_id].callback.rx_fn(NULL, txif, skb, skb->len);
		//g_ppe_directpath_data[g_ppe_directpath_data[if_id].dp_subif.port_id * 16].callback.rx_fn(NULL, txif, skb, skb->len);
	}

	return 0;
rx_err_exit:
	if (skb)
		dev_kfree_skb_any(skb);
	return -1;
}
#endif

int32_t ltq_directpath_register(PPA_SUBIF *p_subif, struct net_device *netif, PPA_DIRECTPATH_CB *pDirectpathCb, int32_t *index, uint32_t flags)
{
	dp_cb_t cb={0};
	struct ppe_directpath_data *priv;

	dbg_info ("<%s> Entry\n",__FUNCTION__);
	if ( (flags & PPA_F_DIRECTPATH_REGISTER) )
	{
		uint32_t dp_port_id;
		int32_t data_index=0;

		dbg_info ("<%s> Register \n",__FUNCTION__);
		if(p_subif->port_id == -1){ //register port
			g_dp_index = ltq_directpath_get_free_module_index();
			if(g_dp_index >= MAX_DIRECTPATH_NUM)
				return PPA_FAILURE;
			dbg_info ("<%s> Free Module Index %d \n",__FUNCTION__,g_dp_index);
			sprintf(&g_ltq_dp_module[g_dp_index].name, "dp_module%02d", g_dp_index);

			dbg_info ("<%s> Allocate port for net device %s  \n",__FUNCTION__,netif->name);
			if ((flags & DP_F_DIRECTLINK)) {
				dbg_info("%s: DIRECTLINK\n", __func__);
				dp_port_id  = dp_alloc_port(&g_ltq_dp_module[g_dp_index], netif, 0, 0, NULL, DP_F_DIRECTLINK);
			} else {
				dbg_info("%s: NO DIRECTLINK\n", __func__);
				dp_port_id  = dp_alloc_port(&g_ltq_dp_module[g_dp_index], netif, 0, 0, NULL, DP_F_DIRECT);
			}
			if(dp_port_id == DP_FAILURE) {
				dp_err("%s: dp_dealloc_port failed for %s\n", __func__, netif->name);
				memset(&g_ltq_dp_module[g_dp_index], 0 , sizeof(struct module));
				return -ENODEV;
			}
			data_index = dp_port_id * MAX_SUBIF_NUM;
			memset(&g_ppe_directpath_data[data_index], 0 , sizeof(struct ppe_directpath_data));
			priv = &g_ppe_directpath_data[data_index];
			priv->owner = &g_ltq_dp_module[g_dp_index];
			priv->dp_port_id = dp_port_id;

			cb.stop_fn   =(dp_stop_tx_fn_t ) pDirectpathCb->stop_tx_fn;
			cb.restart_fn  = (dp_restart_tx_fn_t )pDirectpathCb->start_tx_fn;
#ifdef NEED_PMAC_HEADER_REMOVEL
			cb.rx_fn        = (dp_rx_fn_t )dp_cb_rx;
#else
			cb.rx_fn        = (dp_rx_fn_t )pDirectpathCb->rx_fn;
#endif
			if (dp_register_dev (priv->owner,  dp_port_id, &cb, 0) != DP_SUCCESS) {
				dp_err("dp_register_dev failed for %s\n and port_id %d", netif->name, dp_port_id);
				dp_alloc_port (priv->owner, netif, 0, dp_port_id, NULL, DP_F_DEREGISTER);
				memset(&g_ppe_directpath_data[data_index], 0 , sizeof(struct ppe_directpath_data));
				memset(&g_ltq_dp_module[g_dp_index], 0 , sizeof(struct module));
				return -ENODEV;
			}

			p_subif->port_id = dp_port_id;
			priv->dp_subif.port_id = dp_port_id; 
			priv->dp_subif.subif = -1; 
			priv->ifid = dp_port_id; 
			priv->callback = *pDirectpathCb;	
			priv->netif = netif;
			if((flags & PPE_DIRECTPATH_LEGACY) == PPE_DIRECTPATH_LEGACY) {
				dbg_info ("<%s> For Legacy --> register subif\n",__FUNCTION__);
				if (dp_register_subif (priv->owner, netif, netif->name, &priv->dp_subif, 0) != DP_SUCCESS) {
					dp_err("%s: failed to register subif for device: %s \n", __FUNCTION__, netif->name);
					return PPA_FAILURE;
				}
				dbg_info ("<%s> g_q_conn_status[dp_port_id].egress_q=%d g_q_conn_status[dp_port_id].ingress_q=%d\n",
						__FUNCTION__, g_q_conn_status[dp_port_id].egress_q,g_q_conn_status[dp_port_id].ingress_q);
				if((g_q_conn_status[dp_port_id].egress_q != 1) &&
			   		(g_q_conn_status[dp_port_id].ingress_q != 1)) {
					ltq_directpath_egress_connect(netif, dp_port_id);
					ltq_directpath_ingress_connect(netif, dp_port_id);
				}
			}
			*index = data_index;
			dbg_info ("<%s> Owner name %s \n",__FUNCTION__, priv->owner->name);
			dbg_info ("<%s> Port Id %d subif %d \n",__FUNCTION__, priv->dp_subif.port_id, priv->dp_subif.subif);
			dbg_info ("<%s> \n\t g_dp_index: %d\n",__FUNCTION__, g_dp_index);
		
		} else if(p_subif->port_id >= 0 && p_subif->subif == -1) { // register subif
			int32_t port_index=0, subif_index;
			struct ppe_directpath_data *priv_subif = NULL;
			dp_subif_t subif_id = {-1};	
			dbg_info (" Register subif for Port id: %d\n",p_subif->port_id);
			//dump_directpath_data();
			port_index = ltq_directpath_get_data_index_for_port(p_subif->port_id);
			dbg_info (" Port index: %d\n",port_index);
			if(port_index == -1)
				return PPA_FAILURE;

			priv = &g_ppe_directpath_data[port_index];
			dbg_info ("<%s> Owner name: %s\n",__FUNCTION__,priv->owner->name);
			subif_id.port_id = p_subif->port_id;
			subif_id.subif = -1;
			if (dp_register_subif (priv->owner, netif, netif->name, &subif_id, 0) != DP_SUCCESS) {
				dp_err("%s: failed to open for device: %s \n", __FUNCTION__, netif->name);
				return PPA_FAILURE;
			}
			p_subif->subif = subif_id.subif >> 8;
			dbg_info ("<%s>Returned Sub interface: %d\n",__FUNCTION__,subif_id.subif);
			dbg_info ("<%s>Sub interface: %d\n",__FUNCTION__,p_subif->subif);

			subif_index = port_index + 1 + p_subif->subif;
			dbg_info ("<%s> Sub interface Index is: %d\n",__FUNCTION__,subif_index);
			memset(&g_ppe_directpath_data[subif_index], 0 , sizeof(struct ppe_directpath_data));
			priv_subif = &g_ppe_directpath_data[subif_index];
			priv_subif->dp_subif.port_id = p_subif->port_id;
			//priv_subif->dp_subif.subif = p_subif->subif;
			priv_subif->dp_subif.subif = subif_id.subif;
			priv_subif->netif = netif;
			priv_subif->callback = priv->callback;
			*index = subif_index;
			dbg_info ("<%s> Returned Sub interface Index is: %d\n",__FUNCTION__,subif_index);
			g_q_conn_status[p_subif->port_id].no_of_subif ++;

			dbg_info ("<%s> Status: egress_q = %d ingress_q = %d\n",
					__FUNCTION__, g_q_conn_status[p_subif->port_id].egress_q, g_q_conn_status[p_subif->port_id].ingress_q);
			//ltq_dump_conn_status();

			/** For the first subif registered setup the 
			    egress and ingress connectivity */
			if((g_q_conn_status[p_subif->port_id].egress_q != 1) &&
			   (g_q_conn_status[p_subif->port_id].ingress_q != 1)) {

				dbg_info ("<%s> Setup the Egress/Ingress connectivity for port p_subif->port_id %d:\n",__FUNCTION__,p_subif->port_id);
				if (!(flags & DP_F_DIRECTLINK)) {
					ltq_directpath_egress_connect(netif, p_subif->port_id);
				}
				ltq_directpath_ingress_connect(netif, p_subif->port_id);
			}
			//ltq_dump_conn_status();

		}
	}
	else // for unregister
	{
		int32_t port_index, subif_index;
		int32_t module_index;
	
		dbg_info ("<%s>Un Register Port ID %d\n",__FUNCTION__,p_subif->port_id);
		port_index = ltq_directpath_get_data_index_for_port(p_subif->port_id);
		//dbg_info ("<%s> Data index %d\n",__FUNCTION__,port_index);
		priv = &g_ppe_directpath_data[port_index];
		dbg_info ("<%s> Owner name %s \n",__FUNCTION__, priv->owner->name);


		//dbg_info ("<%s>Private Port ID %d and subif %s\n",__FUNCTION__,priv->dp_subif.port_id,priv->dp_subif.subif);
		
		if(p_subif->subif == -1){ //Only required when physical port is removed
			dbg_info ("<%s>Un Register and De allocate for Port ID %d and device %s\n",__FUNCTION__,p_subif->port_id,netif->name);
			if((flags & PPE_DIRECTPATH_LEGACY) == PPE_DIRECTPATH_LEGACY) {
				dbg_info ("<%s> For Legacy --> unregister subif\n",__FUNCTION__);
				dbg_info ("<%s> netif name %s\n",__FUNCTION__,priv->netif->name);
				if (dp_register_subif (priv->owner, priv->netif, priv->netif->name, &(priv->dp_subif), DP_F_DEREGISTER) != DP_SUCCESS) {
					pr_err("%s: failed to close for device: %s \n", __FUNCTION__, netif->name);
					return -1;
				}
				ltq_directpath_egress_disconnect(netif, priv->dp_port_id);
				ltq_directpath_ingress_disconnect(netif, priv->dp_port_id);
			}
			if (dp_register_dev (priv->owner,  priv->dp_port_id, &cb, DP_F_DEREGISTER) != DP_SUCCESS) {
				dp_err("dp_register_dev failed for %s\n and port_id %d", netif->name, priv->dp_port_id);
				dp_alloc_port (priv->owner, netif, 0, priv->dp_port_id, NULL, DP_F_DEREGISTER);
				return -ENODEV;
			}
			if(dp_alloc_port (priv->owner, netif, 0, priv->dp_port_id, NULL, DP_F_DEREGISTER) != DP_SUCCESS) {
				dp_err("dp_unregister_dev failed for %s\n and port_id %d", netif->name, priv->dp_port_id);
				return -ENODEV;
			}

			//ltq_directpath_egress_disconnect(netif, priv->dp_port_id);
			//ltq_directpath_ingress_disconnect(netif, priv->dp_port_id);
			//ltq_dump_conn_status();
			priv->dp_subif.subif = 0;
			priv->dp_subif.port_id = priv->dp_port_id; 
			*index = port_index;
			//printk("length of string %d\n",strlen(priv->owner->name));
			module_index = ltq_directpath_match_module_index(priv->owner->name);
			printk("<%s> Module index %d\n",__FUNCTION__, module_index);
			memset(&g_ppe_directpath_data[port_index], 0 , sizeof(struct ppe_directpath_data));
			memset(&g_ltq_dp_module[module_index], 0 , sizeof(struct module));
		} else {
			struct ppe_directpath_data *priv_subif = NULL;
			subif_index = port_index + 1 + p_subif->subif;
			//dbg_info ("<%s> Port Index %d subif_index %d\n",__FUNCTION__,port_index,subif_index);
			priv_subif = &g_ppe_directpath_data[subif_index];
			dbg_info ("<%s> Interface %s Port id %d subif %d\n",
				__FUNCTION__,priv_subif->netif->name,
				priv_subif->dp_subif.port_id,priv_subif->dp_subif.subif);
			//if (dp_register_subif (priv->owner, netif, netif->name, p_subif, DP_F_DEREGISTER) != DP_SUCCESS) {
			if (dp_register_subif (priv->owner, priv_subif->netif, priv_subif->netif->name, &priv_subif->dp_subif, DP_F_DEREGISTER) != DP_SUCCESS) {
				pr_err("%s: failed to close for device: %s \n", __FUNCTION__, netif->name);
				return -1;
			}
			//priv_subif->dp_subif.subif = ;
			//priv_subif->dp_subif.port_id = priv->dp_port_id; 
			*index = subif_index;
			g_q_conn_status[p_subif->port_id].no_of_subif --;
			/** Last subif has unregistered so remove the 
			    egress and ingress connectivity */
			if(g_q_conn_status[p_subif->port_id].no_of_subif == 0)
			{
				if (!(flags & DP_F_DIRECTLINK)) {
					ltq_directpath_egress_disconnect(netif, priv->dp_port_id);
				}
				//ltq_directpath_egress_disconnect(netif, priv->dp_port_id);
				ltq_directpath_ingress_disconnect(netif, priv->dp_port_id);
			}
			memset(&g_ppe_directpath_data[subif_index], 0 , sizeof(struct ppe_directpath_data));
		}

	}
	return PPA_SUCCESS;

}




int ltq_directpath_send(PPA_SUBIF *p_subif, struct sk_buff *skb, int32_t len, uint32_t flags)
{
	int32_t port_index, subif_index;
	struct ppe_directpath_data *priv;
	
	struct dma_tx_desc_0 *desc_0 = (struct dma_tx_desc_0 *)&skb->DW0;
        struct dma_tx_desc_1 *desc_1 = (struct dma_tx_desc_1 *)&skb->DW1;

	if (!skb)
	{
		pr_err("%s: skb is null\n", __func__);
		return PPA_EPERM;
	}
	//pr_err("%s: p_subif[0x%x]\n", __func__, p_subif);
	if (!p_subif) {
		dp_err("invalid p_subif\n");
		return PPA_EPERM;
	}
	//pr_err("%s:p_subif->port_id[%d] p_subif->subif[%d]\n", __func__, p_subif->port_id, p_subif->subif);
	
	port_index = ltq_directpath_get_data_index_for_port(p_subif->port_id);
	if(p_subif->subif == -1)
		subif_index = port_index ;
	else
		subif_index = port_index + 1 + p_subif->subif;
	//dbg_info ("<%s> Subif Index %d\n",__FUNCTION__,subif_index);
	priv = &g_ppe_directpath_data[subif_index];
	if (!priv) {
		dp_err("invalid directpath priv\n");
		return PPA_EPERM;
	}
	if (!desc_1) {
		dp_err("invalid desc_1\n");
		return PPA_EPERM;
	}
	if (!desc_0) {
		dp_err("invalid desc_0\n");
		return PPA_EPERM;
	}
	desc_1->field.ep = priv->dp_subif.port_id;
        desc_0->field.dest_sub_if_id = priv->dp_subif.subif;	
	//dbg_info ("<%s>Port ID %d Subif %d\n",__FUNCTION__,priv->dp_subif.port_id, priv->dp_subif.subif);
#ifdef TX_DUMP_PACKET
	pr_err("%s: skb[0x%x] data[0x%x]\n", __func__, skb, skb->data);
	if (skb->data) {
		printk(" ================ \n");
		printk("Data sent to the DP LIB for dev %s\n",skb->dev->name);
		dump_skb_tx(len, (char *)skb->data);
		printk(" ================\n ");
	}
#endif

	/* Call the Datapath Library's TX function */
	if (flags & DP_TX_TO_DL_MPEFW) {
		//pr_err("%s: send DIRECTLINK [%d] [%d]\n", __func__, p_subif->port_id, p_subif->subif);
		/* SET EP */
		skb->DW1 = (skb->DW1 & (~0xF00)) |
			((p_subif->port_id & 0xF) << 8);
		/* SET SUBIFID */
		skb->DW0 = (skb->DW0 & ~0x7FFF) | (p_subif->subif << 8); //my test 20150525
		if (dp_xmit(priv->netif, &(priv->dp_subif), skb, skb->len, flags) == 0) {
			priv->tx_pkt++;
		} else {
			priv->tx_pkt_dropped++;
			dp_err("dp_xmit fails for if_id: %d\n", priv->ifid);
			//kfree(skb);
			return PPA_EPERM;
		}
	} else {
		//pr_err("%s: send NO DIRECTLINK\n", __func__);
		if (dp_xmit(priv->netif, &(priv->dp_subif), skb, skb->len, 0) == 0) {
			priv->tx_pkt++;
		} else {
			priv->tx_pkt_dropped++;
			dp_err("dp_xmit fails for if_id: %d\n", priv->ifid);
			//kfree(skb);
			return PPA_EPERM;
		}
	}
	return PPA_SUCCESS;
}

int ltq_directpath_rx_stop(uint32_t if_id, uint32_t flags)
{
    return 0;
}

int ltq_directpath_rx_start(uint32_t if_id, uint32_t flags)
{
    return 0;
}


/* Initilization  Directpath module */
static int __init ltq_directpath_drv_init (void)
{
	char ver_str[128] = {0};

	/*
	*  init variable for directpath
	*/
	//printk("size of g_ppe_directpath_data %d\n", sizeof(g_ppe_directpath_data));
	//printk("size of ppe_directpath_data %d\n", sizeof(struct ppe_directpath_data));
	//printk("size of kernel module %d\n", sizeof(struct module));
	//printk("size of g_ltq_dp_module %d\n", sizeof(g_ltq_dp_module));

	memset(g_ppe_directpath_data, 0, sizeof(g_ppe_directpath_data));
	memset( g_ltq_dp_module, 0 , sizeof(g_ltq_dp_module));
	memset( g_q_conn_status, 0 , sizeof(g_q_conn_status));


	ppa_drv_g_ppe_directpath_data = g_ppe_directpath_data;
	ppa_drv_directpath_register_hook = ltq_directpath_register;
	ppa_drv_directpath_send_hook = ltq_directpath_send;
	ppa_drv_directpath_rx_start_hook = ltq_directpath_rx_start;
	ppa_drv_directpath_rx_stop_hook = ltq_directpath_rx_stop;

	/* Print the driver version info */
	directpath_drv_ver(ver_str);
	printk(KERN_INFO "%s", ver_str);
	return  0;

}

static void  __exit ltq_directpath_drv_exit (void)
{
	ppa_drv_g_ppe_directpath_data = NULL;
	ppa_drv_directpath_register_hook = NULL;
	ppa_drv_directpath_send_hook = NULL;
	ppa_drv_directpath_rx_start_hook = NULL;
	ppa_drv_directpath_rx_stop_hook = NULL;
  
}

module_init(ltq_directpath_drv_init);
module_exit(ltq_directpath_drv_exit);

MODULE_AUTHOR("Purnendu Ghosh");
MODULE_DESCRIPTION("Lantiq directpath datapath driver (Supported XRX500)");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_MODULE_VERSION);
