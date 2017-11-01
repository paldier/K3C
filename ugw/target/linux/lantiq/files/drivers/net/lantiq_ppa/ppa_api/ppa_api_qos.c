/******************************************************************************
**
** FILE NAME    : ppa_api_qos.c
** PROJECT      : UEIP
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 11 DEC 2009
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack QOS API Implementation
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author            $Comment
** 11 DEC 2009  Shao Guohua        Initiate Version
*******************************************************************************/


//#include <linux/autoconf.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif


//#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/version.h>
//#include <linux/types.h>
//#include <linux/init.h>
//#include <linux/slab.h>
//#if defined(CONFIG_LTQ_PPA_API_PROC)
//#include <linux/proc_fs.h>
//#endif
//#include <linux/netdevice.h>
//#include <linux/in.h>
//#include <net/sock.h>
//#include <net/ip_vs.h>
//#include <asm/time.h>

/*
 *  PPA Specific Head File
 */
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_qos.h"
#include "ppe_drv_wrapper.h"
#include "ppa_api_session.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_mib.h"

#ifdef CONFIG_LTQ_ETHSW_API
#include <xway/switch-api/lantiq_gsw_api.h>
#include <xway/switch-api/lantiq_gsw_flow.h>
#endif

#define MAX_QOS_Q_CAPS 6 
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG

int g_eth_class_prio_map[2][16] = {
					{0,1,2,3,4,5,6,7,7,7,7,7,7,7,7,7},
					{0,1,2,3,4,5,6,7,7,7,7,7,7,7,7,7}
				  };
typedef struct ppa_qos_wlan_wmm_callback_t{
       	int32_t port_id;/* PortId corresponding to Wlan interface*/ 
        struct net_device *netDev; /* Member NetDevice */
        PPA_QOS_CLASS2PRIO_CB callbackfun; /* class2prio map callback function*/
        struct list_head list;  /*   */
} PPA_QOS_WLAN_WMM_CB_t;

LIST_HEAD(ppa_qos_wlan_wmm_list_g);
#if 0
static int32_t set_def_class2prio_map(uint8_t *class2prio)
{
	uint8_t c2p[16];
	int32_t i;

	for(i=0;i<MAX_TC_NUM;i++)
	{
		c2p[i] = i;
		c2p[15-i] = MAX_TC_NUM;
	}
	class2prio = &c2p[0];
	return PPA_SUCCESS;
}
#endif
static int32_t ppa_set_wlan_wmm_prio(PPA_IFNAME *ifname,int32_t port_id,int8_t caller_flag)
{
    	struct netif_info *ifinfo;
        struct list_head *now_head = NULL;
	uint8_t *class2prio;
	uint8_t c2p[16],cl2p[16] = {0};
	int32_t i;
	int8_t port=-1;
        PPA_QOS_WLAN_WMM_CB_t *ppa_qos_wlan_wmm_cb = NULL;

    	ppa_memset(&c2p[0],0,16);
	switch (caller_flag)
	{
		case 1:
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d caller_case is %d!!!\n", __FILE__,__FUNCTION__,__LINE__,caller_flag);
			if(!strcmp(ifname,"wlan0"))
			port=0;
			else if(!strcmp(ifname,"wlan1"))
			port=1;
			for(i=0;i<MAX_TC_NUM;i++)
			{
			//	c2p[i] = i;
				c2p[i] = g_eth_class_prio_map[port][i];
				c2p[15-i] = g_eth_class_prio_map[port][15-i];
			//	c2p[15-i] = MAX_TC_NUM - 1;
			}
			class2prio = &c2p[0];
		        list_for_each(now_head,&ppa_qos_wlan_wmm_list_g) 
			{
                		ppa_qos_wlan_wmm_cb = list_entry(now_head, PPA_QOS_WLAN_WMM_CB_t,list);
                		if((ppa_qos_wlan_wmm_cb->netDev->name != NULL) && (ifname != NULL)){
                        		if (!strcmp (ifname, ppa_qos_wlan_wmm_cb->netDev->name))
                        		{
                                		if(ppa_qos_wlan_wmm_cb->callbackfun != NULL)
                                		{
                                        		ppa_qos_wlan_wmm_cb->callbackfun(ppa_qos_wlan_wmm_cb->port_id,ppa_qos_wlan_wmm_cb->netDev,class2prio);
                                		}
                        		}
                		} 
        		}
			break;
		case 2:
		case 3:
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d caller_case is %d!!!\n", __FILE__,__FUNCTION__,__LINE__,caller_flag);
    			if ( ppa_netif_lookup(ifname, &ifinfo) != PPA_SUCCESS )
			{
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d netif_lookup failed!!!\n", __FILE__,__FUNCTION__,__LINE__);
        			return PPA_FAILURE;
			}
	
			if(!(ifinfo->flags & NETIF_DIRECTCONNECT))
			{
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d not a fastpath wave interface!!!\n", __FILE__,__FUNCTION__,__LINE__);
				return PPA_FAILURE;
			}

			if(ppa_qos_create_c2p_map_for_wmm(ifname,cl2p) == PPA_ENOTAVAIL)
			{
				ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d Setting default Map!!!\n", __FILE__,__FUNCTION__,__LINE__);
				if(!strcmp(ifname,"wlan0"))
				port=0;
				else if(!strcmp(ifname,"wlan1"))
				port=1;
				for(i=0;i<MAX_TC_NUM;i++)
				{
				//	cl2p[i] = i;
					cl2p[i] = g_eth_class_prio_map[port][i];
					cl2p[15-i] = g_eth_class_prio_map[port][15-i];
				//	cl2p[15-i] = MAX_TC_NUM - 1;
				}
			}
			class2prio = cl2p;
        		list_for_each(now_head,&ppa_qos_wlan_wmm_list_g) 
			{
                		ppa_qos_wlan_wmm_cb = list_entry(now_head, PPA_QOS_WLAN_WMM_CB_t,list);
                		if((ppa_qos_wlan_wmm_cb->netDev->name != NULL) && (ifinfo->netif->name != NULL)){
                        		if (!strcmp (ifinfo->netif->name, ppa_qos_wlan_wmm_cb->netDev->name))
                        		{
                                		if(ppa_qos_wlan_wmm_cb->callbackfun != NULL)
                                		{
							ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d Calling wmm callback fn!!!\n", __FILE__,__FUNCTION__,__LINE__);
                                        		ppa_qos_wlan_wmm_cb->callbackfun(ppa_qos_wlan_wmm_cb->port_id,ppa_qos_wlan_wmm_cb->netDev,class2prio);
                                		}
                        		}
                		}	 
        		}
			break;
		default:
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Invalid Wmm caller case \n");
			break;
	}

	return PPA_SUCCESS;
}

#endif // End of WMM_QOS_CONFIG

#define PPA_MAX_Q 40
#define PPA_MAX_Q_LEN 100
#define PPA_Q_CAP 1

int8_t LAN_PORT[] = {2,3,4,5,6};
int8_t LAN_PORT_Q[][4] = { {8,9,10,11}, {12,13,14,15}, {16,17,18,19},{20,21,22,23},{24,25,26,27} };
int32_t WT_Q[] = {20480,20480,65535,65535};
int32_t SCH_Q[] = {1,1,0,0};
#define MAX_NUMBER_OF_LAN_PORTS 4

#ifdef CONFIG_LTQ_PPA_QOS

extern int32_t qosal_add_qos_queue(PPA_CMD_QOS_QUEUE_INFO *, QOS_QUEUE_LIST_ITEM **);
extern int32_t qosal_modify_qos_queue(PPA_CMD_QOS_QUEUE_INFO *, QOS_QUEUE_LIST_ITEM **);
extern int32_t qosal_delete_qos_queue(PPA_CMD_QOS_QUEUE_INFO *, QOS_QUEUE_LIST_ITEM *);
extern int32_t qosal_set_qos_rate(PPA_CMD_RATE_INFO *, QOS_QUEUE_LIST_ITEM *,QOS_SHAPER_LIST_ITEM *);
extern int32_t qosal_add_shaper(PPA_CMD_RATE_INFO *, QOS_SHAPER_LIST_ITEM **);
extern int32_t qosal_eng_init_cfg();
extern int32_t qosal_eng_uninit_cfg();
//extern int32_t qosal_add_wmm_qos_queue(PPA_CMD_QOS_QUEUE_INFO *, QOS_QUEUE_LIST_ITEM **);
//extern int32_t qosal_delete_wmm_qos_queue(PPA_CMD_QOS_QUEUE_INFO *, QOS_QUEUE_LIST_ITEM *);
extern void ppa_qos_queue_lock_list(void);
extern int32_t ppa_qos_queue_lookup(int32_t qnum, PPA_IFNAME ifname[16],QOS_QUEUE_LIST_ITEM **pp_info);
extern void ppa_qos_queue_unlock_list(void);

int32_t ppa_ioctl_add_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    QOS_QUEUE_LIST_ITEM *p_item;

    if ( copy_from_user( &cmd_info->qos_queue_info, (void *)arg, sizeof(cmd_info->qos_queue_info)) != 0 )
        return PPA_FAILURE;
    
    if( qosal_add_qos_queue(&cmd_info->qos_queue_info,&p_item)!= PPA_SUCCESS)
    {
	return PPA_FAILURE;
    }
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
    else
    ppa_set_wlan_wmm_prio(cmd_info->qos_queue_info.ifname,-1,2);
#endif
    return PPA_SUCCESS;

}

int32_t ppa_ioctl_modify_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    QOS_QUEUE_LIST_ITEM *p_item;
    int32_t ret;

    if ( copy_from_user( &cmd_info->qos_queue_info, (void *)arg, sizeof(cmd_info->qos_queue_info)) != 0 )
        return PPA_FAILURE;
	
    ppa_qos_queue_lock_list();
    ret = ppa_qos_queue_lookup(cmd_info->qos_queue_info.queue_num,cmd_info->qos_queue_info.ifname,&p_item);
    if( ret == PPA_QOS_QUEUE_NOT_FOUND )
    return PPA_FAILURE;
    else
    ret = qosal_modify_qos_queue(&cmd_info->qos_queue_info,&p_item);
    
    ppa_qos_queue_unlock_list();
    
    return ret;
}

int32_t ppa_ioctl_delete_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{

    QOS_QUEUE_LIST_ITEM *p_item;
    QOS_SHAPER_LIST_ITEM *p_s_item;
    int32_t ret=0;
    if ( copy_from_user( &cmd_info->qos_queue_info, (void *)arg, sizeof(cmd_info->qos_queue_info)) != 0 )
        return PPA_FAILURE;
    ret = ppa_qos_queue_lookup(cmd_info->qos_queue_info.queue_num,cmd_info->qos_queue_info.ifname,&p_item);
    if( ret == PPA_ENOTAVAIL )
    return PPA_FAILURE;
    else
    ret = qosal_delete_qos_queue(&cmd_info->qos_queue_info,p_item);
    /* Delete Shaper assigned to the Queue when the Queue is deleted */
#if 1
    if(ret == PPA_SUCCESS)
    {
    	if(ppa_qos_shaper_lookup(cmd_info->qos_queue_info.shaper_num,cmd_info->qos_queue_info.ifname,&p_s_item) == PPA_SUCCESS)
    	{
    		ppa_qos_shaper_remove_item(p_s_item->shaperid,p_s_item->ifname,NULL);
    		ppa_qos_shaper_free_item(p_s_item);
    	}
    }
#endif
    ppa_qos_queue_remove_item(p_item->queue_num,p_item->ifname,NULL);
    ppa_qos_queue_free_item(p_item);
    if(ret == PPA_SUCCESS)
    {
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
    	ppa_set_wlan_wmm_prio(cmd_info->qos_queue_info.ifname,-1,3);
#endif
    }

    return ret;
}

int32_t ppa_modify_qos_subif_to_port(PPA_CMD_SUBIF_PORT_INFO *subif_port_info)
{
	int32_t ret=PPA_SUCCESS;
	QOS_MOD_SUBIF_PORT_CFG SubifPort_info;
	
	memset(&SubifPort_info, 0, sizeof(QOS_MOD_SUBIF_PORT_CFG));

	strcpy(SubifPort_info.ifname, subif_port_info->ifname);
	SubifPort_info.port_id = subif_port_info->port_id;
	SubifPort_info.priority_level = subif_port_info->priority_level;
	SubifPort_info.weight = subif_port_info->weight;
	SubifPort_info.flags = subif_port_info->flags;

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	 if ( (ret = ppa_hsel_mod_subif_port_cfg( &SubifPort_info, 0, TMU_HAL) ) != PPA_SUCCESS )
        {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d hal select Mod subif to port failed!!!\n", __FILE__,__FUNCTION__,__LINE__);
        }
#endif
        return ret;
}


int32_t ppa_ioctl_mod_subif_port_config(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
	
    if ( copy_from_user( &cmd_info->subif_port_info, (void *)arg, sizeof(cmd_info->subif_port_info)) != 0 )
        return PPA_FAILURE;
   
   if( ppa_modify_qos_subif_to_port(&cmd_info->subif_port_info)!= PPA_SUCCESS)
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d Mod subif to port failed!!!\n", __FILE__,__FUNCTION__,__LINE__);
        return PPA_FAILURE;
    }
  
   return PPA_SUCCESS;
}

#if ( defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500 ) 
extern GSW_API_HANDLE gsw_api_kopen(char *name);
extern int gsw_api_kioctl(GSW_API_HANDLE handle, u32 command, u32 arg);
extern int gsw_api_kclose(GSW_API_HANDLE handle);

int32_t qosal_set_gswr(uint32_t cmd,void *cfg)
{
	GSW_API_HANDLE gswr_handle = 0;
	GSW_QoS_queuePort_t *qos_queueport_set;
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t *qos_classpcp;
	GSW_QoS_portRemarkingCfg_t *qos_remark_cfg;
	int32_t ret = 0;
	
	gswr_handle = gsw_api_kopen("/dev/switch_api/1");
	
	switch(cmd)
	{
		case GSW_QOS_QUEUE_PORT_SET:
		{	
		qos_queueport_set = (GSW_QoS_queuePort_t *)cfg;
        	ret =  gsw_api_kioctl(gswr_handle, GSW_QOS_QUEUE_PORT_SET,(u32)qos_queueport_set);
		break;
		}
		case GSW_QOS_PORT_REMARKING_CFG_SET:
		{
		qos_remark_cfg = (GSW_QoS_portRemarkingCfg_t *)cfg;
		ret = gsw_api_kioctl(gswr_handle, GSW_QOS_PORT_REMARKING_CFG_SET,(u32)qos_remark_cfg);
		break;
		}
		case GSW_QOS_SVLAN_CLASS_PCP_PORT_SET:
		{
		qos_classpcp = (GSW_QoS_SVLAN_ClassPCP_PortCfg_t *)cfg;
		ret = gsw_api_kioctl(gswr_handle, GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,(u32)qos_classpcp);
		break;
		}
		case GSW_QOS_SVLAN_CLASS_PCP_PORT_GET:
		{
		qos_classpcp = (GSW_QoS_SVLAN_ClassPCP_PortCfg_t *)cfg;
		ret = gsw_api_kioctl(gswr_handle, GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,(u32)qos_classpcp);
		break;
		}
		default:
		break;
	}
	if(ret < GSW_statusOk)
	printk(" gsw_api_kioctl returned failure \n");
	gsw_api_kclose(gswr_handle);

	return ret;
}

int32_t qosal_set_gswl(uint32_t cmd,void *cfg)
{
	GSW_API_HANDLE gswl_handle = 0;
	GSW_QoS_schedulerCfg_t *qos_schcfg_set;
	GSW_QoS_DSCP_ClassCfg_t *qos_dscp_class_set;
	GSW_QoS_portCfg_t *qos_portcfg_set;
	GSW_portCfg_t *portcfg_set;
	GSW_QoS_queuePort_t *qos_queueport_set;
	GSW_QoS_WRED_QueueCfg_t *qos_wred_set;
	GSW_QoS_FlowCtrlPortCfg_t *qos_flowctrl;
	GSW_register_t *regCfg;
	int32_t ret = 0;

	gswl_handle = gsw_api_kopen("/dev/switch_api/0");

	switch(cmd)
	{
		case GSW_QOS_SCHEDULER_CFG_SET:
		{
		qos_schcfg_set = (GSW_QoS_schedulerCfg_t *)cfg;
        	ret =  gsw_api_kioctl(gswl_handle, GSW_QOS_SCHEDULER_CFG_SET,(u32)qos_schcfg_set);
		break;
		}
		
		case GSW_QOS_DSCP_CLASS_SET:
		{	
		qos_dscp_class_set = (GSW_QoS_DSCP_ClassCfg_t *)cfg;
        	ret =  gsw_api_kioctl(gswl_handle, GSW_QOS_DSCP_CLASS_SET,(u32)qos_dscp_class_set);
		break;
		}
		
		case GSW_QOS_PORT_CFG_SET:
		{	
		qos_portcfg_set = (GSW_QoS_portCfg_t *)cfg;
        	ret =  gsw_api_kioctl(gswl_handle, GSW_QOS_PORT_CFG_SET,(u32)qos_portcfg_set);
		break;
		}
		
		case GSW_QOS_QUEUE_PORT_SET:
		{	
		qos_queueport_set = (GSW_QoS_queuePort_t *)cfg;
        	ret =  gsw_api_kioctl(gswl_handle, GSW_QOS_QUEUE_PORT_SET,(u32)qos_queueport_set);
		break;
		}
		case GSW_PORT_CFG_SET:
		{
		portcfg_set = (GSW_portCfg_t *)cfg;
        	ret =  gsw_api_kioctl(gswl_handle, GSW_PORT_CFG_SET,(u32)portcfg_set);
		break;
		}
		case GSW_QOS_WRED_QUEUE_CFG_SET:
		{
		qos_wred_set = (GSW_QoS_WRED_QueueCfg_t *)cfg; 
        	ret =  gsw_api_kioctl(gswl_handle, GSW_QOS_WRED_QUEUE_CFG_SET,(u32)qos_wred_set);
		break;
		}
		case GSW_QOS_FLOWCTRL_PORT_CFG_SET:
		{
		qos_flowctrl = (GSW_QoS_FlowCtrlPortCfg_t *)cfg;
        	ret =  gsw_api_kioctl(gswl_handle, GSW_QOS_FLOWCTRL_PORT_CFG_SET,(u32)qos_flowctrl);
		break;
		}
		case GSW_REGISTER_SET:
		{
		regCfg = (GSW_register_t *)cfg;
		ret = gsw_api_kioctl(gswl_handle, GSW_REGISTER_SET, (u32)regCfg);
		break;
		}
		default:
		break;
	}

	if(ret < GSW_statusOk)
	printk(" gsw_api_kioctl returned failure \n");
	gsw_api_kclose(gswl_handle);
	return ret;
}
#endif/* GRX500 */
int32_t ppa_ioctl_qos_init_cfg(unsigned int cmd)
{
	int32_t ret = PPA_SUCCESS;

	switch ( cmd )
	{
		case PPA_CMD_ENG_QUEUE_INIT:
		{
			ret = qosal_eng_init_cfg();
			break;
		}
		case PPA_CMD_ENG_QUEUE_UNINIT:
		{
			ret = qosal_eng_uninit_cfg();
			break;
		}
		default:
		{
		break;
		}
	}
	
	return ret;
}

#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG

int32_t ppa_ioctl_add_wmm_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    QOS_QUEUE_LIST_ITEM *p_item;
    PPA_HSEL_CAP_NODE *caps_list=NULL;
    uint32_t num_caps = 0, i, j, ret=PPA_SUCCESS;
    uint8_t f_more_hals = 0;
    
    if ( copy_from_user( &cmd_info->qos_queue_info, (void *)arg, sizeof(cmd_info->qos_queue_info)) != 0 )
    return PPA_FAILURE;

    caps_list = (PPA_HSEL_CAP_NODE*) ppa_malloc (MAX_QOS_Q_CAPS*sizeof(PPA_HSEL_CAP_NODE));
    ppa_memset(caps_list,0,(MAX_QOS_Q_CAPS*sizeof(PPA_HSEL_CAP_NODE)));
    
    if(!caps_list)
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"alloc caps_list item failed \n" ); 
	return PPA_FAILURE;
    }
    
    caps_list[num_caps++].cap = QOS_WMM_INIT;
    if(ppa_select_hals_from_caplist(0, num_caps, caps_list) != PPA_SUCCESS) {
		ppa_free(caps_list);
		return PPA_FAILURE;
    }
    for(i = 0; i < num_caps;) 
    { 
		f_more_hals = 0;
		j = ppa_group_hals_in_capslist(i, num_caps, caps_list);
		// Based on the capability of first entry in the list we can decide the action
		switch(caps_list[i].cap) 
		{
	    		case QOS_WMM_INIT:
			{
				if( caps_list[i].hal_id == PPE_HAL )
				break;
				else
				{
    					if( qosal_add_qos_queue(&cmd_info->qos_queue_info,&p_item)!= PPA_SUCCESS)
					return PPA_FAILURE;
    					else
    					ppa_set_wlan_wmm_prio(cmd_info->qos_queue_info.ifname,-1,2);
				}
			}
			break;
			default:
			break;
		}
		i+=j;
    }
    
    ppa_free(caps_list);
#endif
    return PPA_SUCCESS;

}
int32_t ppa_ioctl_delete_wmm_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{

    int32_t ret=0;
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    QOS_QUEUE_LIST_ITEM *p_item;
    QOS_SHAPER_LIST_ITEM *p_s_item;
    if ( copy_from_user( &cmd_info->qos_queue_info, (void *)arg, sizeof(cmd_info->qos_queue_info)) != 0 )
        return PPA_FAILURE;
    ret = ppa_qos_queue_lookup(cmd_info->qos_queue_info.queue_num,cmd_info->qos_queue_info.ifname,&p_item);
    if( ret == PPA_ENOTAVAIL )
    return PPA_FAILURE;
    else
    ret = qosal_delete_qos_queue(&cmd_info->qos_queue_info,p_item);
    /* Delete Shaper assigned to the Queue when the Queue is deleted */
#if 1
    if(ret == PPA_SUCCESS)
    {
    	if(ppa_qos_shaper_lookup(cmd_info->qos_queue_info.shaper_num,cmd_info->qos_queue_info.ifname,&p_s_item) == PPA_SUCCESS)
    	{
    		ppa_qos_shaper_remove_item(p_s_item->shaperid,p_s_item->ifname,NULL);
    		ppa_qos_shaper_free_item(p_s_item);
    	}
    }
#endif
    ppa_qos_queue_remove_item(p_item->queue_num,p_item->ifname,NULL);
    ppa_qos_queue_free_item(p_item);
    if(ret == PPA_SUCCESS)
    {
    	ppa_set_wlan_wmm_prio(cmd_info->qos_queue_info.ifname,-1,3);
    }
#endif

    return ret;
}
static int32_t register_for_qos_class2prio(int32_t port_id, struct net_device *netif,PPA_QOS_CLASS2PRIO_CB cbfun, uint32_t flags)
{
	PPA_QOS_WLAN_WMM_CB_t *ppa_qos_wlan_wmm_cb = NULL;

	if(netif == NULL) // Can netif be null and port_id have a valid value ? Need to check this.
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d netif is NULL!!!\n", __FILE__,__FUNCTION__,__LINE__);
		return PPA_FAILURE;
	}

	ppa_qos_wlan_wmm_cb = (PPA_QOS_WLAN_WMM_CB_t *)ppa_malloc(sizeof(*ppa_qos_wlan_wmm_cb));
	if(ppa_qos_wlan_wmm_cb == NULL)
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d ppa_qos_wlan_wmm_cb malloc failed!!!\n", __FILE__,__FUNCTION__,__LINE__);
		return PPA_FAILURE;
	}
	
	ppa_qos_wlan_wmm_cb->netDev = netif;
	ppa_qos_wlan_wmm_cb->port_id = port_id;
	ppa_qos_wlan_wmm_cb->callbackfun = (void *)cbfun;
	
	INIT_LIST_HEAD(&ppa_qos_wlan_wmm_cb->list);
	list_add_tail(&ppa_qos_wlan_wmm_cb->list,&ppa_qos_wlan_wmm_list_g);

	ppa_set_wlan_wmm_prio(netif->name,port_id,1);
	
	return PPA_SUCCESS;

}

static int32_t deregister_for_qos_class2prio(int32_t port_id, struct net_device *netif,PPA_QOS_CLASS2PRIO_CB cbfun, uint32_t flags)
{
	struct list_head *now_head = NULL,*next_head = NULL;
	PPA_QOS_WLAN_WMM_CB_t *ppa_qos_wlan_wmm_cb = NULL;
	
	if(netif == NULL) // Can netif be null and port_id have a valid value ? Need to check this.
	{
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d netif is NULL!!!\n", __FILE__,__FUNCTION__,__LINE__);
		return PPA_FAILURE;
	}
	
	list_for_each_safe(now_head,next_head,&ppa_qos_wlan_wmm_list_g)
	{
		ppa_qos_wlan_wmm_cb = list_entry(now_head, PPA_QOS_WLAN_WMM_CB_t, list);
		if(ppa_qos_wlan_wmm_cb != NULL)
		{
			if(!strncmp(netif->name ,ppa_qos_wlan_wmm_cb->netDev->name,strlen(ppa_qos_wlan_wmm_cb->netDev->name)))
			{
				list_del(&ppa_qos_wlan_wmm_cb->list);
				kfree(ppa_qos_wlan_wmm_cb);	
				return PPA_SUCCESS;
			}
		}
	}
    	return PPA_SUCCESS;
}

int32_t ppa_register_for_qos_class2prio(int32_t port_id, struct net_device *netif,PPA_QOS_CLASS2PRIO_CB class2prio_cbfun, uint32_t flags)
{
    if(netif != NULL) // Can netif be null and port_id have a valid value? Need to check this.
    {	
    	if(flags & WMM_QOS_DEV_F_REG)
	{
	    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d in WMM_QOS_DEV_F_REG !!!\n", __FILE__,__FUNCTION__,__LINE__);
    	    register_for_qos_class2prio(port_id,netif,class2prio_cbfun,flags);
	}
    	else if((flags & WMM_QOS_DEV_F_DREG) || (class2prio_cbfun == NULL))
	{
	    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d in WMM_QOS_DEV_F_DREG !!!\n", __FILE__,__FUNCTION__,__LINE__);
	    deregister_for_qos_class2prio(port_id,netif,NULL,flags);
	}
    }
    return PPA_SUCCESS;
}

#endif // End of WMM_QOS_CONFIG

int32_t ppa_ioctl_get_qos_status(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{   // Try to return PPA_SUCCESS with ioctl, but make sure put real return value in internal variable. 
    if ( copy_from_user( &cmd_info->qos_status_info, (void *)arg, sizeof(cmd_info->qos_status_info)) != 0 )
        return PPA_FAILURE;

    if( ppa_update_qos_mib(&cmd_info->qos_status_info.qstat, 1, cmd_info->qos_status_info.flags ) != PPA_SUCCESS )
    {
        cmd_info->qos_status_info.qstat.res = PPA_FAILURE;
    }
    else 
        cmd_info->qos_status_info.qstat.res = PPA_SUCCESS;
   
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_status_info, sizeof(cmd_info->qos_status_info)) != 0 )
        return PPA_FAILURE;

   return PPA_SUCCESS;
}

int32_t ppa_get_qos_qnum( uint32_t portid, uint32_t flag)
{
    PPE_QOS_COUNT_CFG  count={0};

    count.portid = portid;
    count.flags = flag;

    ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_get_qos_qnum for portid:%d\n", count.portid );
    if( ppa_drv_get_qos_qnum( &count, 0) != PPA_SUCCESS ) 
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_drv_get_qos_qnum failed\n");
        count.num = 0;
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "qos num is :%d\n", count.num );
    }
    
    return count.num;
}

int32_t ppa_ioctl_get_qos_qnum(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    int res = PPA_FAILURE;
    PPE_QOS_COUNT_CFG count={0};
    
    ppa_memset(&cmd_info->qnum_info, 0, sizeof(cmd_info->qnum_info) );
    if ( copy_from_user( &cmd_info->qnum_info, (void *)arg, sizeof(cmd_info->qnum_info)) != 0 )
        return PPA_FAILURE;
    count.portid = cmd_info->qnum_info.portid;
    res = ppa_drv_get_qos_qnum( &count, 0);
    cmd_info->qnum_info.queue_num = count.num;
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qnum_info, sizeof(cmd_info->qnum_info)) != 0 )
        return PPA_FAILURE;

   return res;
}

int32_t ppa_get_qos_mib( uint32_t portid, uint32_t queueid, PPA_QOS_MIB *mib, uint32_t flag)
{
    uint32_t res;
    PPE_QOS_MIB_INFO mib_info={0};

    mib_info.portid = portid;
    mib_info.queueid = queueid;
    mib_info.flag = flag;
    res = ppa_drv_get_qos_mib( &mib_info, 0);

    *mib = mib_info.mib;
    
    return res;
}

int32_t ppa_ioctl_get_qos_mib(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{

    int res = PPA_FAILURE;
    PPE_QOS_MIB_INFO mib={0};
    
    ppa_memset(&cmd_info->qos_mib_info, 0, sizeof(cmd_info->qos_mib_info) );
    if ( copy_from_user( &cmd_info->qos_mib_info, (void *)arg, sizeof(cmd_info->qos_mib_info)) != 0 )
        return PPA_FAILURE;
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    QOS_QUEUE_LIST_ITEM *p_item;
    res = ppa_qos_queue_lookup(cmd_info->qos_mib_info.queue_num,cmd_info->qos_queue_info.ifname,&p_item);
    if( res == PPA_ENOTAVAIL ){
    	return PPA_FAILURE;
		}

    mib.ifname = cmd_info->qos_mib_info.ifname;
    mib.queueid = p_item->p_entry;
#endif
    mib.portid = cmd_info->qos_mib_info.portid;
    mib.flag = cmd_info->qos_mib_info.flags;
    res = ppa_drv_get_qos_mib( &mib, 0);
    
    cmd_info->qos_mib_info.mib = mib.mib;

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_mib_info, sizeof(cmd_info->qos_mib_info)) != 0 )

       return PPA_FAILURE;

   return res;
}


#ifdef CONFIG_LTQ_PPA_QOS_WFQ
int32_t ppa_set_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t weight_level, uint32_t flag )
{
    PPE_QOS_WFQ_CFG wfq={0};

    wfq.portid = portid;
    wfq.queueid = queueid;
    wfq.weight_level = weight_level;
    wfq.flag = flag;

    return ppa_drv_set_qos_wfq( &wfq, 0);    
}

int32_t ppa_get_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t *weight_level, uint32_t flag)
{
    PPE_QOS_WFQ_CFG wfq={0};
    int32_t res;

    wfq.portid = portid;
    wfq.queueid = queueid;
    wfq.flag = flag;

    res = ppa_drv_get_qos_wfq (&wfq, 0);

    if( weight_level ) *weight_level = wfq.weight_level;
    
    return res;
}

int32_t ppa_reset_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t flag)
{
    PPE_QOS_WFQ_CFG cfg={0};

    cfg.portid = portid;
    cfg.queueid = queueid;
    cfg.flag = flag;
   return ppa_drv_reset_qos_wfq(&cfg, 0);
}

int32_t ppa_set_ctrl_qos_wfq(uint32_t portid,  uint32_t f_enable, uint32_t flag)
{
    int i;
    PPE_QOS_COUNT_CFG count={0};
    PPE_QOS_ENABLE_CFG enable_cfg={0};

    count.portid = portid;
    count.flags = flag;
    ppa_drv_get_qos_qnum( &count, 0);
    
    if( count.num <= 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_set_ctrl_qos_wfq: count.num not valid (%d) to %s wfq on port %d\n", count.num, f_enable?"enable":"disable", portid );
        return PPA_FAILURE;
    }

    enable_cfg.portid = portid;
    enable_cfg.flag = flag;
    enable_cfg.f_enable = f_enable;
    ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_set_ctrl_qos_wfq to %s wfq on port %d\n", f_enable?"enable":"disable", portid );
    ppa_drv_set_ctrl_qos_wfq( &enable_cfg, 0);    

    for( i=0; i<count.num; i++ )
           ppa_reset_qos_wfq( portid, i, 0);
    return PPA_SUCCESS;
}

int32_t ppa_get_ctrl_qos_wfq(uint32_t portid,  uint32_t *f_enable, uint32_t flag)
{ 
    int32_t res = PPA_FAILURE;
    
    if( f_enable )
    {
        PPE_QOS_ENABLE_CFG enable_cfg={0};

        enable_cfg.portid = portid;
        enable_cfg.flag = flag;
            
        res = ppa_drv_get_ctrl_qos_wfq(&enable_cfg, 0);

        if( f_enable ) *f_enable = enable_cfg.f_enable;
    }
    
    return res;
}

int32_t ppa_ioctl_set_ctrl_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int32_t res;
    
    ppa_memset(&cmd_info->qos_ctrl_info, 0, sizeof(cmd_info->qos_ctrl_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_ctrl_info, (void *)arg, sizeof(cmd_info->qos_ctrl_info)) != 0 )
        return PPA_FAILURE;
  
     res = ppa_set_ctrl_qos_wfq(cmd_info->qos_ctrl_info.portid, cmd_info->qos_ctrl_info.enable, cmd_info->qos_ctrl_info.flags);
     if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_ctrl_qos_wfq fail\n");
        res = PPA_FAILURE;
    }

     return res;
}

int32_t ppa_ioctl_get_ctrl_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
        
    ppa_memset(&cmd_info->qos_ctrl_info, 0, sizeof(cmd_info->qos_ctrl_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_ctrl_info, (void *)arg, sizeof(cmd_info->qos_ctrl_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_get_ctrl_qos_wfq(cmd_info->qos_ctrl_info.portid, &cmd_info->qos_ctrl_info.enable, cmd_info->qos_ctrl_info.flags);
    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_get_ctrl_qos_wfq fail\n");
        res = PPA_FAILURE;
    }

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_ctrl_info, sizeof(cmd_info->qos_ctrl_info)) != 0 )
        return PPA_FAILURE;

    return res;
}

int32_t ppa_ioctl_set_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int32_t res;
    
    ppa_memset(&cmd_info->qos_wfq_info, 0, sizeof(cmd_info->qos_wfq_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_wfq_info, (void *)arg, sizeof(cmd_info->qos_wfq_info)) != 0 )
        return PPA_FAILURE;
  
     res = ppa_set_qos_wfq(cmd_info->qos_wfq_info.portid, cmd_info->qos_wfq_info.queueid, cmd_info->qos_wfq_info.weight, cmd_info->qos_wfq_info.flags);
     if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_qos_wfq fail\n");
        res = PPA_FAILURE;
    }

     return res;
}

int32_t ppa_ioctl_reset_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
    
    ppa_memset(&cmd_info->qos_wfq_info, 0, sizeof(cmd_info->qos_wfq_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_wfq_info, (void *)arg, sizeof(cmd_info->qos_wfq_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_reset_qos_wfq(cmd_info->qos_wfq_info.portid, cmd_info->qos_wfq_info.queueid, cmd_info->qos_wfq_info.flags);
    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_reset_qos_wfq fail\n");
        res = PPA_FAILURE;
    }

    return res;   
}

int32_t ppa_ioctl_get_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
        
    ppa_memset(&cmd_info->qos_wfq_info, 0, sizeof(cmd_info->qos_wfq_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_wfq_info, (void *)arg, sizeof(cmd_info->qos_wfq_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_get_qos_wfq(cmd_info->qos_wfq_info.portid, cmd_info->qos_wfq_info.queueid, &cmd_info->qos_wfq_info.weight, cmd_info->qos_wfq_info.flags);
    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_get_qos_wfq fail\n");
        res = PPA_FAILURE;
    }

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_wfq_info, sizeof(cmd_info->qos_wfq_info)) != 0 )
        return PPA_FAILURE;

    return res;
}

EXPORT_SYMBOL(ppa_set_ctrl_qos_wfq);
EXPORT_SYMBOL(ppa_get_ctrl_qos_wfq);
EXPORT_SYMBOL(ppa_set_qos_wfq);
EXPORT_SYMBOL(ppa_get_qos_wfq);
EXPORT_SYMBOL(ppa_reset_qos_wfq);
EXPORT_SYMBOL(ppa_ioctl_set_ctrl_qos_wfq);
EXPORT_SYMBOL(ppa_ioctl_get_ctrl_qos_wfq);
EXPORT_SYMBOL(ppa_ioctl_set_qos_wfq);
EXPORT_SYMBOL(ppa_ioctl_reset_qos_wfq);
EXPORT_SYMBOL(ppa_ioctl_get_qos_wfq);
#endif  //end of CONFIG_LTQ_PPA_QOS_WFQ

#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
int32_t ppa_set_ctrl_qos_rate(uint32_t portid,  uint32_t f_enable, uint32_t flag)
{
    uint32_t i;
    PPE_QOS_COUNT_CFG count={0};
    PPE_QOS_ENABLE_CFG enable_cfg={0};
    PPE_QOS_RATE_SHAPING_CFG rate={0};

    count.portid = portid;
    count.flags = flag;
    ppa_drv_get_qos_qnum( &count, 0);

    if( count.num <= 0 )
        return PPA_FAILURE;

    enable_cfg.portid = portid;
    enable_cfg.flag = flag;
    enable_cfg.f_enable = f_enable;
    ppa_drv_set_ctrl_qos_rate( &enable_cfg, 0);    

    for( i=0; i<count.num; i++ )
    {
        rate.flag = 0;
        rate.portid = portid;
        rate.queueid    = i;
        ppa_drv_reset_qos_rate( &rate, 0);
    }

    return PPA_SUCCESS;
}

int32_t ppa_get_ctrl_qos_rate(uint32_t portid,  uint32_t *f_enable, uint32_t flag)
{ 
    PPE_QOS_ENABLE_CFG enable_cfg={0};
    int32_t res;

    enable_cfg.portid = portid;
    enable_cfg.flag = flag;

    res= ppa_drv_get_ctrl_qos_rate( &enable_cfg, 0);

    if( *f_enable ) *f_enable = enable_cfg.f_enable;
    return res;
}
#if defined(MBR_CONFIG) && MBR_CONFIG
int32_t ppa_set_qos_rate( char *ifname, uint32_t portid, uint32_t queueid, int32_t shaperid, uint32_t rate, uint32_t burst, uint32_t flag, int32_t hal_id )
#else
int32_t ppa_set_qos_rate( char *ifname, uint32_t portid, uint32_t queueid, uint32_t rate, uint32_t burst, uint32_t flag , int32_t hal_id)
#endif
{

    int32_t ret=PPA_SUCCESS;

#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 

    QOS_RATE_SHAPING_CFG tmu_rate_cfg;
    memset(&tmu_rate_cfg,0x00,sizeof(QOS_RATE_SHAPING_CFG));
    strcpy(tmu_rate_cfg.ifname,ifname);
    tmu_rate_cfg.portid = portid;
    tmu_rate_cfg.queueid = queueid;
#if defined(MBR_CONFIG) && MBR_CONFIG
    tmu_rate_cfg.shaperid = shaperid;
#endif
    tmu_rate_cfg.rate_in_kbps = rate;
    tmu_rate_cfg.burst = burst;
    tmu_rate_cfg.flag = flag;
    if ( (ret = ppa_hsel_set_qos_rate_entry( &tmu_rate_cfg, 0, hal_id) ) == PPA_SUCCESS )
    {
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n", __FILE__,__FUNCTION__,__LINE__);
    }
#else /* !CONFIG_LTQ_PPA_GRX500 */
    PPE_QOS_RATE_SHAPING_CFG rate_cfg={0};

    rate_cfg.portid = portid;
    rate_cfg.queueid = queueid;
#if defined(MBR_CONFIG) && MBR_CONFIG
    rate_cfg.shaperid = shaperid;
#endif
    rate_cfg.rate_in_kbps = rate;
    rate_cfg.burst = burst;
    rate_cfg.flag = flag;
    
    ret=ppa_drv_set_qos_rate( &rate_cfg, 0);
#endif /* CONFIG_LTQ_PPA_GRX500 */
    return ret;
}

#if defined(MBR_CONFIG) && MBR_CONFIG
int32_t ppa_get_qos_rate( uint32_t portid, uint32_t queueid, int32_t *shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag)
#else
int32_t ppa_get_qos_rate( uint32_t portid, uint32_t queueid, uint32_t *rate, uint32_t *burst, uint32_t flag)
#endif
{
    PPE_QOS_RATE_SHAPING_CFG rate_cfg={0};
    int32_t res = PPA_FAILURE;

    rate_cfg.portid = portid;
    rate_cfg.flag = flag;
    rate_cfg.queueid = queueid;
#if defined(MBR_CONFIG) && MBR_CONFIG
    rate_cfg.shaperid = -1;
#endif    
    res = ppa_drv_get_qos_rate( &rate_cfg, 0);

    if( rate ) *rate = rate_cfg.rate_in_kbps;
    if( burst )  *burst = rate_cfg.burst;
#if defined(MBR_CONFIG) && MBR_CONFIG
    if( shaperid ) *shaperid = rate_cfg.shaperid;
#endif
    return res;

}

#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 
int32_t ppa_reset_qos_rate( char *ifname, uint32_t portid, int32_t queueid, int32_t shaperid, uint32_t flag, int32_t hal_id)
#else
int32_t ppa_reset_qos_rate( uint32_t portid, int32_t queueid, uint32_t flag)
#endif
{
#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 
    int32_t ret = PPA_FAILURE;
    QOS_RATE_SHAPING_CFG tmu_rate_cfg;
    memset(&tmu_rate_cfg,0x00,sizeof(QOS_RATE_SHAPING_CFG));
    strcpy(tmu_rate_cfg.ifname,ifname);
    tmu_rate_cfg.portid = portid;
    tmu_rate_cfg.queueid = queueid;
    tmu_rate_cfg.shaperid = shaperid;
    tmu_rate_cfg.flag = flag;
    
    if ( (ret = ppa_hsel_reset_qos_rate_entry( &tmu_rate_cfg, 0, hal_id) ) == PPA_SUCCESS )
    {
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n", __FILE__,__FUNCTION__,__LINE__);
    }
    return ret;
#else
    PPE_QOS_RATE_SHAPING_CFG rate_cfg={0};

    rate_cfg.portid = portid;
    rate_cfg.queueid = queueid;
    rate_cfg.flag = flag;
    
    return ppa_drv_reset_qos_rate( &rate_cfg, 0);
#endif
}


int32_t ppa_ioctl_set_ctrl_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int32_t res;
    
    ppa_memset(&cmd_info->qos_ctrl_info, 0, sizeof(cmd_info->qos_ctrl_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_ctrl_info, (void *)arg, sizeof(cmd_info->qos_ctrl_info)) != 0 )
        return PPA_FAILURE;
  
     res = ppa_set_ctrl_qos_rate(cmd_info->qos_ctrl_info.portid, cmd_info->qos_ctrl_info.enable, cmd_info->qos_ctrl_info.flags);
     if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_ctrl_qos_rate fail\n");
        res = PPA_FAILURE;
    }

     return res;
}

int32_t ppa_ioctl_get_ctrl_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
        
    ppa_memset(&cmd_info->qos_ctrl_info, 0, sizeof(cmd_info->qos_ctrl_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_ctrl_info, (void *)arg, sizeof(cmd_info->qos_ctrl_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_get_ctrl_qos_rate(cmd_info->qos_ctrl_info.portid, &cmd_info->qos_ctrl_info.enable, cmd_info->qos_ctrl_info.flags);
    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_get_ctrl_qos_rate fail\n");
        res = PPA_FAILURE;
    }

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_ctrl_info, sizeof(cmd_info->qos_ctrl_info)) != 0 )
        return PPA_FAILURE;

    return res;
}

#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 

int32_t qosal_reset_qos_rate(PPA_CMD_RATE_INFO *qos_rate_info, QOS_QUEUE_LIST_ITEM *p_item, QOS_SHAPER_LIST_ITEM *p_s_item)
{
        PPA_HSEL_CAP_NODE *caps_list = NULL;
        uint32_t i = 0,ret=PPA_SUCCESS,numcap;
     
	if(!p_s_item->caps_list) {
        return PPA_FAILURE;
        }
        caps_list = (PPA_HSEL_CAP_NODE *)p_s_item->caps_list;
        numcap = p_s_item->num_caps;

        	for( i = 0; i < numcap; ) {
                	switch(caps_list[i].cap)
                	{
                	        case Q_SHAPER:
                        	{
					if( caps_list[i].hal_id == PPE_HAL)
					break;
                                	else {
                                		ret = ppa_reset_qos_rate(qos_rate_info->ifname,qos_rate_info->portid, p_item->p_entry, p_s_item->p_entry, qos_rate_info->flags,caps_list[i].hal_id);
                                		if(ret != PPA_SUCCESS)
                                		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_set_qos_rate failed \n" );
                                	}
                        	break;
                        	}
				case PORT_SHAPER:
				{
					if( caps_list[i].hal_id == PPE_HAL)
					break;
                 	               	else {
						if((qos_rate_info->shaperid == -1) && (qos_rate_info->queueid == -1))
						{
							ret = ppa_reset_qos_rate(qos_rate_info->ifname,qos_rate_info->portid, -1 , p_s_item->p_entry, qos_rate_info->flags,caps_list[i].hal_id);
	        					if(ret != PPA_SUCCESS)
        						ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_set_qos_rate failed \n" );
						}
                        	        }
				break;
				}
                        	default:
                        	break;
                	}
	      	i++;
		}
	return ret;
}

int32_t qosal_set_qos_rate(PPA_CMD_RATE_INFO *qos_rate_info, QOS_QUEUE_LIST_ITEM *p_item, QOS_SHAPER_LIST_ITEM *p_s_item)
{
        PPA_HSEL_CAP_NODE *caps_list = NULL;
        uint32_t i = 0,ret=PPA_SUCCESS,numcap;

        if(!p_s_item->caps_list) {
        return PPA_FAILURE;
        }
        caps_list = (PPA_HSEL_CAP_NODE *)p_s_item->caps_list;
        numcap = p_s_item->num_caps;

        //  when init, these entry values are ~0, the max the number which can be detected by these functions

        for( i = 0; i < numcap; ) {
                switch(caps_list[i].cap)
                {
                        case Q_SHAPER:
                        {
				if( caps_list[i].hal_id == PPE_HAL)
				break;
                                else {
#if defined(MBR_CONFIG) && MBR_CONFIG
                                ret = ppa_set_qos_rate(qos_rate_info->ifname,qos_rate_info->portid, p_item->p_entry, p_s_item->p_entry, qos_rate_info->rate,qos_rate_info->burst, qos_rate_info->flags,caps_list[i].hal_id);
#else
                                ret = ppa_set_qos_rate(qos_rate_info->ifname,qos_rate_info->portid, p_item->p_entry, qos_rate_info->rate,qos_rate_info->burst, qos_rate_info->flags,caps_list[i].hal_id);
#endif
                                if(ret != PPA_SUCCESS)
                                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_set_qos_rate failed \n" );
                                }
                        break;
                        }
			case PORT_SHAPER:
			{
				if( caps_list[i].hal_id == PPE_HAL)
				break;
                                else {
					if((qos_rate_info->shaperid == -1) && (qos_rate_info->queueid == -1))
					{
						ret = ppa_set_qos_rate(qos_rate_info->ifname,qos_rate_info->portid, -1 , p_s_item->p_entry, qos_rate_info->rate,qos_rate_info->burst, qos_rate_info->flags,caps_list[i].hal_id);
	        				if(ret != PPA_SUCCESS)
        					ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_set_qos_rate failed \n" );
					}
                                }
			break;
			}
                        default:
                        break;
                }
	      i++;
        }
    return ret;

}

#endif

int32_t ppa_ioctl_set_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int32_t res;
    
#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 

    uint32_t phy_qid,phy_shaperid;
    QOS_QUEUE_LIST_ITEM *p_item;
    QOS_SHAPER_LIST_ITEM *p_s_item;
    
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    phy_qid = cmd_info->qos_rate_info.queueid;
    phy_shaperid = cmd_info->qos_rate_info.shaperid;

 
    	res = ppa_qos_shaper_lookup(cmd_info->qos_rate_info.shaperid,cmd_info->qos_rate_info.ifname,&p_s_item);
    	if( res == PPA_ENOTAVAIL )
    	{
            ppa_debug(DBG_ENABLE_MASK_QOS, " ppa_ioctl_set_qos_rate: PPA_QOS_SHAPER_NOT_FOUND  \n");
    	    return PPA_FAILURE;
    	}
    	else
    	phy_shaperid = p_s_item->p_entry;

    if(phy_qid != -1)
    {
    	res = ppa_qos_queue_lookup(cmd_info->qos_rate_info.queueid,cmd_info->qos_queue_info.ifname,&p_item);
    	if( res == PPA_ENOTAVAIL )
    	{
            ppa_debug(DBG_ENABLE_MASK_QOS, " ppa_ioctl_set_qos_rate: PPA_QOS_QUEUE_NOT_FOUND  \n");
    	    return PPA_FAILURE;
    	}
    	else
    	phy_qid = p_item->p_entry;
    }
    res = qosal_set_qos_rate(&cmd_info->qos_rate_info,p_item,p_s_item); 

    if ( res != PPA_SUCCESS )
    {  
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_qos_rate fail\n");
        res = PPA_FAILURE;
    }
    
#else /* !CONFIG_LTQ_PPA_GRX500 */
    int32_t hal_id = -1;
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

#if defined(MBR_CONFIG) && MBR_CONFIG
    res = ppa_set_qos_rate(cmd_info->qos_rate_info.ifname,cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, cmd_info->qos_rate_info.shaperid, cmd_info->qos_rate_info.rate, cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags,hal_id);
#else    
    res = ppa_set_qos_rate(cmd_info->qos_rate_info.ifname,cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, cmd_info->qos_rate_info.rate, cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags,hal_id);
#endif

    if ( res != PPA_SUCCESS )
    {  
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_qos_rate fail\n");
        res = PPA_FAILURE;
    }
#endif /* CONFIG_LTQ_PPA_GRX500 */
    return res;
}

int32_t ppa_ioctl_reset_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
    
#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 

    uint32_t phy_qid,phy_shaperid;
    QOS_QUEUE_LIST_ITEM *p_item;
    QOS_SHAPER_LIST_ITEM *p_s_item;

    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;
    
    phy_qid = cmd_info->qos_rate_info.queueid;
    phy_shaperid = cmd_info->qos_rate_info.shaperid;
    	
    res = ppa_qos_shaper_lookup(cmd_info->qos_rate_info.shaperid,cmd_info->qos_rate_info.ifname,&p_s_item);
    if( res == PPA_ENOTAVAIL )
    {
    	ppa_debug(DBG_ENABLE_MASK_QOS, " ppa_ioctl_reset_qos_rate: PPA_QOS_SHAPER_NOT_FOUND  \n");
    	return PPA_FAILURE;
    }
    else
    phy_shaperid = p_s_item->p_entry;

    if(phy_qid != -1)
    {
    	res = ppa_qos_queue_lookup(cmd_info->qos_rate_info.queueid,cmd_info->qos_queue_info.ifname,&p_item);
    	if( res == PPA_ENOTAVAIL )
    	{
            ppa_debug(DBG_ENABLE_MASK_QOS, " ppa_ioctl_set_qos_rate: PPA_QOS_QUEUE_NOT_FOUND  \n");
    	    return PPA_FAILURE;
    	}
    	else
    	phy_qid = p_item->p_entry;
    }

    res = qosal_reset_qos_rate(&cmd_info->qos_rate_info,p_item,p_s_item); 

    if ( res != PPA_SUCCESS )
    {  
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_qos_rate fail\n");
        res = PPA_FAILURE;
    }

#else
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_reset_qos_rate(cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, cmd_info->qos_rate_info.flags);
    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_reset_rate fail\n");
        res = PPA_FAILURE;
    }
#endif
    return res;   
}

int32_t ppa_ioctl_get_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
        
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;
#if defined(MBR_CONFIG) && MBR_CONFIG
    res = ppa_get_qos_rate(cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, &cmd_info->qos_rate_info.shaperid, &cmd_info->qos_rate_info.rate, &cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags);
#else
    res = ppa_get_qos_rate(cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, &cmd_info->qos_rate_info.rate, &cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags);
#endif
    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_get_qos_rate fail\n");
        res = PPA_FAILURE;
    }

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_rate_info, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    return res;
}

#if defined(MBR_CONFIG) && MBR_CONFIG

int32_t ppa_set_qos_shaper( int32_t shaperid, uint32_t rate, uint32_t burst, PPA_QOS_ADD_SHAPER_CFG *s, uint32_t flags, int32_t hal_id )
{


#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 
    uint32_t ret=PPA_SUCCESS;
    QOS_RATE_SHAPING_CFG tmu_shape_cfg;

    memset(&tmu_shape_cfg,0x00,sizeof(QOS_RATE_SHAPING_CFG));
    tmu_shape_cfg.shaper.mode = s->mode;
    tmu_shape_cfg.shaper.enable = s->enable;
    tmu_shape_cfg.shaper.pir = s->pir;
    tmu_shape_cfg.shaper.pbs = s->pbs;
    tmu_shape_cfg.shaper.cir = s->cir;
    tmu_shape_cfg.shaper.cbs = s->cbs;
    tmu_shape_cfg.shaper.flags = s->flags;
	#ifdef CONFIG_PPA_PUMA7
   	strcpy(tmu_shape_cfg.ifname,s->ifname);
	#endif
    if ( (ret = ppa_hsel_set_qos_shaper_entry( &tmu_shape_cfg, 0, hal_id) ) == PPA_SUCCESS )
    {
	s->phys_shaperid = tmu_shape_cfg.phys_shaperid;
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add success!!!\n", __FILE__,__FUNCTION__,__LINE__);
    }
    else
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s %s %d add failed!!!\n", __FILE__,__FUNCTION__,__LINE__);

    return ret;
#else

    PPE_QOS_RATE_SHAPING_CFG rate_cfg={0};

    rate_cfg.shaperid = shaperid;
    rate_cfg.rate_in_kbps = rate;
    rate_cfg.burst = burst;
    return ppa_drv_set_qos_shaper( &rate_cfg, 0);
#endif
}

int32_t ppa_get_qos_shaper( int32_t shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag)
{
    PPE_QOS_RATE_SHAPING_CFG rate_cfg={0};
    int32_t res = PPA_FAILURE;

    rate_cfg.flag = flag;
    rate_cfg.shaperid = shaperid;
    
    //res = ifx_ppa_drv_get_qos_rate( &rate_cfg, 0);
    res = ppa_drv_get_qos_shaper( &rate_cfg, 0);
	

    if( rate ) *rate = rate_cfg.rate_in_kbps;
    if( burst )  *burst = rate_cfg.burst;

    return res;

}

int32_t ppa_ioctl_set_qos_shaper(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int32_t res;

#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 
    QOS_SHAPER_LIST_ITEM *p_item;
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    if( qosal_add_shaper(&cmd_info->qos_rate_info,&p_item)!= PPA_SUCCESS)
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "qosal_add_shaper returned failure\n");
	return PPA_FAILURE;
    }
    return PPA_SUCCESS;
#else
    int32_t hal_id=-1;
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    //res = ifx_ppa_set_qos_rate(cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, cmd_info->qos_rate_info.shaperid, cmd_info->qos_rate_info.rate, cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags);
    res = ppa_set_qos_shaper(cmd_info->qos_rate_info.shaperid, cmd_info->qos_rate_info.rate, cmd_info->qos_rate_info.burst,&cmd_info->qos_rate_info.shaper,cmd_info->qos_rate_info.flags,hal_id);

    if ( res != PPA_SUCCESS )
    {  
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_set_qos_shaper fail\n");
        res = PPA_FAILURE;
    }

#endif
    return res;
}

int32_t ppa_ioctl_get_qos_shaper(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res = PPA_FAILURE;
   
    ppa_memset(&cmd_info->qos_rate_info, 0, sizeof(cmd_info->qos_rate_info) );

    if ( ppa_copy_from_user( &cmd_info->qos_rate_info, (void *)arg, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    //res = ifx_ppa_get_qos_rate(cmd_info->qos_rate_info.portid, cmd_info->qos_rate_info.queueid, &cmd_info->qos_rate_info.shaperid, &cmd_info->qos_rate_info.rate, &cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags);
    res = ppa_get_qos_shaper(cmd_info->qos_rate_info.shaperid, &cmd_info->qos_rate_info.rate, &cmd_info->qos_rate_info.burst, cmd_info->qos_rate_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_QOS, "ppa_ioctl_get_qos_shaper fail\n");
        res = PPA_FAILURE;
    }

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->qos_rate_info, sizeof(cmd_info->qos_rate_info)) != 0 )
        return PPA_FAILURE;

    return res;
}



#endif //end of MBR_CONFIG



EXPORT_SYMBOL(ppa_set_ctrl_qos_rate);
EXPORT_SYMBOL(ppa_get_ctrl_qos_rate);
EXPORT_SYMBOL(ppa_set_qos_rate);
#if ( defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR ) 
EXPORT_SYMBOL(qosal_set_qos_rate);
#endif
EXPORT_SYMBOL(ppa_get_qos_rate);
EXPORT_SYMBOL(ppa_reset_qos_rate);
EXPORT_SYMBOL(ppa_ioctl_set_ctrl_qos_rate);
EXPORT_SYMBOL(ppa_ioctl_get_ctrl_qos_rate);
EXPORT_SYMBOL(ppa_ioctl_set_qos_rate);
EXPORT_SYMBOL(ppa_ioctl_reset_qos_rate);
EXPORT_SYMBOL(ppa_ioctl_get_qos_rate);
#if defined(MBR_CONFIG) && MBR_CONFIG
EXPORT_SYMBOL(ppa_ioctl_set_qos_shaper);
EXPORT_SYMBOL(ppa_ioctl_get_qos_shaper);
#endif
#endif  //end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING


#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500

int32_t reset_pcp_remark(PPA_CLASS_RULE *rule)
{
	int32_t ret = PPA_SUCCESS,i = 0;
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t qos_classpcp;

	memset(&qos_classpcp, 0, sizeof(GSW_QoS_SVLAN_ClassPCP_PortCfg_t));

	if((rule->action.iftype == IF_CATEGORY_ETHWAN) || (rule->action.iftype == IF_CATEGORY_PTMWAN) || (rule->action.iftype == IF_CATEGORY_ATMWAN))
	{
		for(i=0; i<sizeof(LAN_PORT)/sizeof(int8_t); i++)
        	{
			qos_classpcp.nPortId = LAN_PORT[i];
        		ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,&qos_classpcp);
			qos_classpcp.nCPCP[rule->action.qos_action.alt_trafficclass] = 0;
        		ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,&qos_classpcp);
        	}
	} 
	else if((rule->action.iftype == IF_CATEGORY_ETHLAN))
	{
		qos_classpcp.nPortId = 15;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,&qos_classpcp);
		qos_classpcp.nCPCP[rule->action.qos_action.alt_trafficclass] = 0;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,&qos_classpcp);
		qos_classpcp.nPortId = 7;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,&qos_classpcp);
		qos_classpcp.nCPCP[rule->action.qos_action.alt_trafficclass] = 0;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,&qos_classpcp);
	}
	
	return ret;	
}

int32_t set_pcp_remark(PPA_CLASS_RULE *rule)
{
	int32_t ret = PPA_SUCCESS,i = 0;
	GSW_QoS_portRemarkingCfg_t qos_remark_cfg;
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t qos_classpcp;

	memset(&qos_remark_cfg, 0, sizeof(GSW_QoS_portRemarkingCfg_t));
	memset(&qos_classpcp, 0, sizeof(GSW_QoS_SVLAN_ClassPCP_PortCfg_t));


	for(i=0; i<sizeof(LAN_PORT)/sizeof(int8_t); i++)
        {
		qos_remark_cfg.nPortId = LAN_PORT[i];
		qos_remark_cfg.bPCP_IngressRemarkingEnable = 1;
		qos_remark_cfg.bPCP_EgressRemarkingEnable = 1;
                ret = qosal_set_gswr(GSW_QOS_PORT_REMARKING_CFG_SET,&qos_remark_cfg);
        }
	qos_remark_cfg.nPortId = 15;
	qos_remark_cfg.bPCP_IngressRemarkingEnable = 1;
	qos_remark_cfg.bPCP_EgressRemarkingEnable = 1;
        ret = qosal_set_gswr(GSW_QOS_PORT_REMARKING_CFG_SET,&qos_remark_cfg);
	
	if((rule->action.iftype == IF_CATEGORY_ETHWAN) || (rule->action.iftype == IF_CATEGORY_PTMWAN) || (rule->action.iftype == IF_CATEGORY_ATMWAN))
	{
		for(i=0; i<sizeof(LAN_PORT)/sizeof(int8_t); i++)
        	{
			qos_classpcp.nPortId = LAN_PORT[i];
        		ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,&qos_classpcp);
			qos_classpcp.nCPCP[rule->action.qos_action.alt_trafficclass] = rule->action.qos_action.new_pcp;
        		ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,&qos_classpcp);
        	}
	} 
	else if((rule->action.iftype == IF_CATEGORY_ETHLAN))
	{
		qos_classpcp.nPortId = 15;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,&qos_classpcp);
		qos_classpcp.nCPCP[rule->action.qos_action.alt_trafficclass] = rule->action.qos_action.new_pcp;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,&qos_classpcp);
		qos_classpcp.nPortId = 7;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_GET,&qos_classpcp);
		qos_classpcp.nCPCP[rule->action.qos_action.alt_trafficclass] = rule->action.qos_action.new_pcp;
        	ret = qosal_set_gswr(GSW_QOS_SVLAN_CLASS_PCP_PORT_SET,&qos_classpcp);
	}
	
	return ret;	
}

int32_t set_port_subifid(PPA_CLASS_RULE *rule)
{
	int32_t ret = PPA_SUCCESS,i = 0;
    	dp_subif_t dp_subif={0};
        
	if ((dp_get_netif_subifid(ppa_get_netif(rule->rx_if), NULL, NULL, NULL, &dp_subif, 0))==PPA_SUCCESS) {
		if(dp_subif.subif == 0)
		{
			ppa_debug(DBG_ENABLE_MASK_QOS," In set_port_subifid : Sub interface Id is 0 \n");
			rule->pattern.bPortIdEnable = 1;
        		rule->pattern.nPortId = dp_subif.port_id;
		}
		else
		{
			ppa_debug(DBG_ENABLE_MASK_QOS," In set_port_subifid : Sub interface Id is %d \n",rule->pattern.nSubIfId);
			rule->pattern.bSubIfIdEnable = 1;
                	rule->pattern.nSubIfId = dp_subif.subif;
		}
		ppa_debug(DBG_ENABLE_MASK_QOS," In set_port_subifid: dp_get_netif_subifid: portid enable = %d:portid = %d:subif enable= %d:subif = %d \n",rule->pattern.bPortIdEnable,rule->pattern.nPortId,rule->pattern.bSubIfIdEnable,rule->pattern.nSubIfId);
	}
	else
	ppa_debug(DBG_ENABLE_MASK_QOS," In set_port_subifid:: dp_get_netif_subifid failed for the interface %s \n",rule->rx_if);

	return ret;
}

int32_t ppa_ioctl_add_class_rule (unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    int32_t ret=PPA_SUCCESS;
    if(copy_from_user(&cmd_info->class_info,(void *)arg, sizeof(cmd_info->class_info))!=0) {
	return PPA_FAILURE;
    }

    if( cmd_info->class_info.action.qos_action.remarkpcp == 1)
    set_pcp_remark(&cmd_info->class_info);
   
    set_port_subifid(&cmd_info->class_info); 

    ret=ppa_add_class_rule(&cmd_info->class_info);
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->class_info, sizeof(cmd_info->class_info)) != 0 )
        return PPA_FAILURE;

    return ret;
	
}

int32_t ppa_ioctl_mod_class_rule (unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    int32_t ret=PPA_SUCCESS;
    if(copy_from_user(&cmd_info->class_info,(void *)arg, sizeof(cmd_info->class_info))!=0) {
        return PPA_FAILURE;
    }
    ret=ppa_mod_class_rule(&cmd_info->class_info);
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->class_info, sizeof(cmd_info->class_info)) != 0 )
        return PPA_FAILURE;
    
    return ret;

}

int32_t ppa_ioctl_get_class_rule (unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    int32_t ret=PPA_SUCCESS;
    if(copy_from_user(&cmd_info->class_info,(void *)arg, sizeof(cmd_info->class_info))!=0) {
        return PPA_FAILURE;
    }
    ret=ppa_get_class_rule(&cmd_info->class_info);
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->class_info, sizeof(cmd_info->class_info)) != 0 )
        return PPA_FAILURE;

    return ret;

}

int32_t ppa_ioctl_del_class_rule (unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{
    int32_t ret=PPA_SUCCESS;
    if(copy_from_user(&cmd_info->class_info,(void *)arg, sizeof(cmd_info->class_info))!=0) {
        return PPA_FAILURE;
    }
    if( cmd_info->class_info.action.qos_action.remarkpcp == 1)
    reset_pcp_remark(&cmd_info->class_info);
    ret=ppa_del_class_rule(&cmd_info->class_info);
    return ret;

}
#endif

EXPORT_SYMBOL(ppa_ioctl_add_qos_queue);
EXPORT_SYMBOL(ppa_ioctl_modify_qos_queue);
EXPORT_SYMBOL(ppa_ioctl_delete_qos_queue);
EXPORT_SYMBOL(ppa_ioctl_qos_init_cfg);
EXPORT_SYMBOL(ppa_ioctl_add_wmm_qos_queue);
EXPORT_SYMBOL(ppa_ioctl_delete_wmm_qos_queue);
EXPORT_SYMBOL(g_eth_class_prio_map);
EXPORT_SYMBOL(ppa_get_qos_qnum);
EXPORT_SYMBOL(ppa_ioctl_get_qos_qnum);
EXPORT_SYMBOL(ppa_get_qos_mib);
EXPORT_SYMBOL(ppa_ioctl_get_qos_mib);
#endif  //end of CONFIG_LTQ_PPA_QOS


