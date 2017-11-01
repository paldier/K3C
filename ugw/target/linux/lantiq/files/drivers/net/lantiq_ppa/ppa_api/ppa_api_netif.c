/*******************************************************************************
**
** FILE NAME    : ppa_api_netif.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Network Interface Functions
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 03 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/



/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
//#include <linux/autoconf.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

/*
 *  PPA Specific Head File
 */
#include "ppa_api.h"
#if defined(CONFIG_LTQ_DATAPATH) && CONFIG_LTQ_DATAPATH
#include <net/datapath_api.h>
#endif
#include <net/ppa_ppe_hal.h>
#include "ppa_api_netif.h"
#include "ppa_api_misc.h"
#include "ppe_drv_wrapper.h"
#include "ppa_api_session.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"

#include "ppa_api_tools.h"

#ifdef CONFIG_LTQ_ETHSW_API
#include <xway/switch-api/lantiq_gsw_api.h>

int gsw_api_kioctl(GSW_API_HANDLE handle, u32 command, u32 arg);
#endif



/*
 * ####################################
 *              Definition
 * ####################################
 */



/*
 * ####################################
 *              Data Type
 * ####################################
 */



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  implemented in PPA PPE Low Level Driver (Data Path)
 */
//extern int get_dslwan_qid_with_vcc(PPA_VCC *vcc);



/*
 * ####################################
 *           Global Variable
 * ####################################
 */

uint32_t g_phys_port_cpu = ~0;
uint32_t g_phys_port_atm_wan = ~0;
uint32_t g_phys_port_atm_wan_vlan = 0;
static struct phys_port_info *g_phys_port_info = NULL;
static PPA_LOCK g_phys_port_lock;

static struct netif_info *g_netif = NULL;
static PPA_LOCK g_netif_lock;
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
static uint16_t intf_bitmap = 0;
#endif

/*
 * ####################################
 *           Extern Variable
 * ####################################
 */

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
extern int32_t ppa_get_fid(PPA_IFNAME *ifname, uint16_t *fid);
#endif


/*
 * ####################################
 *            Local Function
 * ####################################
 */

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
static int32_t ppa_check_if_netif_directpath(PPA_NETIF *netif, uint16_t flag) 
{
    int8_t ret=PPA_FAILURE;
    int8_t found=0;
    struct ppe_directpath_data *info;
    uint32_t pos;
    
    if ((ret = ppa_directpath_data_start_iteration(&pos, &info)) == PPA_SUCCESS )
    {
        do
        {
            //printk("<%s> info->netif=<%s> netif <%s>\n",__FUNCTION__,info->netif->name,netif->name);
            if ( (info->flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) && info->netif == netif ) {
                printk("<%s> match Entry Found !!!!!!\n",__FUNCTION__);
                found =1;
                ret=PPA_SUCCESS;
                break;
            }
        } while ( ppa_directpath_data_iterate_next(&pos, &info) == PPA_SUCCESS );
    }

    ppa_directpath_data_stop_iteration();
    if(found == 1)
            ret = PPA_SUCCESS;
    else
            ret = PPA_FAILURE;

    return ret;
}

#endif
/*
 *  Physical Network Interface Operation Function
 */
static INLINE struct phys_port_info *ppa_phys_port_alloc_item(void)
{
    struct phys_port_info *obj;

    obj = (struct phys_port_info *)ppa_malloc(sizeof(*obj));
    if ( obj )
        ppa_memset(obj, 0, sizeof(*obj));
    return obj;
}

static INLINE void ppa_phys_port_free_item(struct phys_port_info *obj)
{
    ppa_free(obj);
}

static void ppa_phys_port_free_list(void)
{
    struct phys_port_info *obj;

    ppa_lock_get(&g_phys_port_lock);
    while ( g_phys_port_info )
    {
        obj = g_phys_port_info;
        g_phys_port_info = g_phys_port_info->next;

        ppa_phys_port_free_item(obj);
    }
    g_phys_port_cpu = ~0;
    g_phys_port_atm_wan = ~0;
    ppa_lock_release(&g_phys_port_lock);
}

static int32_t ppa_phys_port_lookup(PPA_IFNAME ifname[PPA_IF_NAME_SIZE], struct phys_port_info **pp_info)
{
    int32_t ret = PPA_ENOTAVAIL;
    struct phys_port_info *obj;

    ppa_lock_get(&g_phys_port_lock);
    for ( obj = g_phys_port_info; obj; obj = obj->next ) {
        if ( strcmp(obj->ifname, ifname) == 0 )
        {
            ret = PPA_SUCCESS;
            if ( pp_info )
                *pp_info = obj;
            break;
        }
    }
    ppa_lock_release(&g_phys_port_lock);

    return ret;
}

/*
 *  Network Interface Operation Functions
 */

static INLINE struct netif_info * ppa_netif_alloc_item(void)    //  alloc_netif_info
{
    struct netif_info *obj;

    obj = (struct netif_info *)ppa_malloc(sizeof(*obj));
    if ( obj )
    {
        ppa_memset(obj, 0, sizeof(*obj));
        obj->mac_entry = ~0;
        ppa_atomic_set(&obj->count, 1);
    }
    return obj;
}

static INLINE void ppa_netif_free_item(struct netif_info *obj)  //  free_netif_info
{
    if ( ppa_atomic_dec(&obj->count) == 0 )
    {
        PPE_ROUTE_MAC_INFO mac_info = {0};
        mac_info.mac_ix = obj->mac_entry;
        ppa_drv_del_mac_entry( &mac_info, 0);

        //resotre old wanitf if necessary
        if( obj->f_wanitf.flag_already_wanitf )
        {
            PPE_WANITF_CFG wanitf_cfg={0};
            
            wanitf_cfg.lan_flag = obj->f_wanitf.old_lan_flag?1:0;
            wanitf_cfg.physical_port = obj->phys_port;
            ppa_set_wan_itf( &wanitf_cfg, 0);
        }
        
        ppa_free(obj);
    }
}

void ppa_netif_lock_list(void)    //  lock_netif_info_list
{
    ppa_lock_get(&g_netif_lock);
}

void ppa_netif_unlock_list(void)  //  unlock_netif_info_list
{
    ppa_lock_release(&g_netif_lock);
}

static INLINE void __ppa_netif_add_item(struct netif_info *obj)   //  add_netif_info
{
    ppa_atomic_inc(&obj->count);
    ppa_netif_lock_list();
    obj->next = g_netif;
    g_netif = obj;
    ppa_netif_unlock_list();
}

static INLINE void ppa_netif_remove_item(PPA_IFNAME ifname[PPA_IF_NAME_SIZE], struct netif_info **pp_info)  //  remove_netif_info
{
    struct netif_info *p_prev, *p_cur;

    if ( pp_info )
        *pp_info = NULL;
    p_prev = NULL;
    ppa_netif_lock_list();
    for ( p_cur = g_netif; p_cur; p_prev = p_cur, p_cur = p_cur->next )
        if ( strcmp(p_cur->name, ifname) == 0 )
        {
            if ( !p_prev )
                g_netif = p_cur->next;
            else
                p_prev->next = p_cur->next;
            if ( pp_info )
                *pp_info = p_cur;
            else
                ppa_netif_free_item(p_cur);
            break;
        }
    ppa_netif_unlock_list();
}

static void ppa_netif_free_list(void)    //  free_netif_info_list
{
    struct netif_info *obj;

    ppa_netif_lock_list();
    while ( g_netif )
    {
        obj = g_netif;
        g_netif = g_netif->next;

        ppa_netif_free_item(obj);
        obj = NULL;
    }
    ppa_netif_unlock_list();
}



/*
 * ####################################
 *           Global Function
 * ####################################
 */

int32_t ppa_phys_port_add(PPA_IFNAME *ifname, uint32_t port)
{
    struct phys_port_info *obj;
    uint32_t mode = 0;
    uint32_t type = 0;
    uint32_t vlan = 0;
    PPE_IFINFO if_info;
    uint32_t irq_flag = 0;

    if ( !ifname )
        return PPA_EINVAL;

    ppa_memset( &if_info, 0, sizeof(if_info) );
    if_info.port = port;
    ppa_drv_get_phys_port_info( &if_info, 0);
    if ( (if_info.if_flags & PPA_PHYS_PORT_FLAGS_VALID) )
    {
        switch ( if_info.if_flags & PPA_PHYS_PORT_FLAGS_MODE_MASK )
        {
        case PPA_PHYS_PORT_FLAGS_MODE_LAN: mode = 1; break;
        case PPA_PHYS_PORT_FLAGS_MODE_WAN: mode = 2; break;
        case PPA_PHYS_PORT_FLAGS_MODE_MIX: mode = 3;
        }
        switch ( if_info.if_flags & PPA_PHYS_PORT_FLAGS_TYPE_MASK )
        {
        case PPA_PHYS_PORT_FLAGS_TYPE_CPU: type = 0; break;
        case PPA_PHYS_PORT_FLAGS_TYPE_ATM: type = 1; break;
        case PPA_PHYS_PORT_FLAGS_TYPE_ETH: type = 2; break;
        case PPA_PHYS_PORT_FLAGS_TYPE_EXT: type = 3;
        }
        vlan = (if_info.if_flags & PPA_PHYS_PORT_FLAGS_OUTER_VLAN) ? 2 : 1;
    }
    /*trick here with ppa_lock_get2 since it will be called in irqs_disabled mode by directpath wlan 
    registering */
    irq_flag=ppa_lock_get2(&g_phys_port_lock);
    for ( obj = g_phys_port_info; obj; obj = obj->next )
        if ( obj->port == port )
		{
			strcpy(obj->ifname, ifname);
			obj->mode   = mode;
			obj->type   = type;
			obj->vlan   = vlan;
			break;
		}
    ppa_lock_release2(&g_phys_port_lock, irq_flag);

    if ( !obj )
    {
        obj = ppa_phys_port_alloc_item();
        if ( !obj )
            return PPA_ENOMEM;
        strcpy(obj->ifname, ifname);
        obj->mode   = mode;
        obj->type   = type;
        obj->port   = port;
        obj->vlan   = vlan;
        irq_flag=ppa_lock_get2(&g_phys_port_lock);
        obj->next = g_phys_port_info;
        g_phys_port_info = obj;
        ppa_lock_release2(&g_phys_port_lock, irq_flag);
    }

    return PPA_SUCCESS;
}

void ppa_phys_port_remove(uint32_t port)
{
    struct phys_port_info *p_prev, *p_cur;
    uint32_t irq_flags = 0;

    p_prev = NULL;
  /*trick here with ppa_lock_get2 since it will be called in irqs_disabled mode by directpath wlan 
    registering */
    irq_flags=ppa_lock_get2(&g_phys_port_lock);
    for ( p_cur = g_phys_port_info; p_cur; p_prev = p_cur, p_cur = p_cur->next )
        if ( p_cur->port == port )
        {
            if ( !p_prev )
                g_phys_port_info = p_cur->next;
            else
                p_prev->next = p_cur->next;
            ppa_lock_release2(&g_phys_port_lock, irq_flags);
            ppa_phys_port_free_item(p_cur);
            return;
        }
    ppa_lock_release2(&g_phys_port_lock, irq_flags);
}

int32_t ppa_phys_port_get_first_eth_lan_port(uint32_t *ifid, PPA_IFNAME **ifname)
{
    int32_t ret;
    struct phys_port_info *obj;

    if ( !ifid || !ifname )
        return PPA_EINVAL;

    ret = PPA_ENOTAVAIL;

    ppa_lock_get(&g_phys_port_lock);
    for ( obj = g_phys_port_info; obj; obj = obj->next )
        if ( obj->mode == 1 && obj->type == 2 ) //  LAN side ETH interface
        {
            *ifid = obj->port;
            *ifname = &(obj->ifname[0]);
            ret = PPA_SUCCESS;
            break;
        }
    ppa_lock_release(&g_phys_port_lock);

    return ret;
}

int32_t ppa_phys_port_start_iteration(uint32_t *ppos, struct phys_port_info **ifinfo)
{
    uint32_t l;
    struct phys_port_info *p;

    ppa_lock_get(&g_phys_port_lock);

    for ( p = g_phys_port_info, l = *ppos; p && l; p = p->next, l-- );

    if ( l == 0 && p )
    {
        ++*ppos;
        *ifinfo = p;
        return PPA_SUCCESS;
    }
    else
    {
        *ifinfo = NULL;
        return PPA_FAILURE;
    }
}

int32_t ppa_phys_port_iterate_next(uint32_t *ppos, struct phys_port_info **ifinfo)
{
    if ( *ifinfo )
    {
        ++*ppos;
        *ifinfo = (*ifinfo)->next;
        return *ifinfo ? PPA_SUCCESS : PPA_FAILURE;
    }
    else
        return PPA_FAILURE;
}

void ppa_phys_port_stop_iteration(void)
{
    ppa_lock_release(&g_phys_port_lock);
}

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
static inline void ppa_add_subif(struct netif_info* pNetIf, const char* ifname)
{
  if( pNetIf->sub_if_index < PPA_IF_SUB_NAME_MAX_NUM) {
    ppa_strncpy(pNetIf->sub_if_name[pNetIf->sub_if_index], 
        ifname, PPA_IF_NAME_SIZE);
    pNetIf->sub_if_index++;
  }
}
#else
#define ppa_add_subif(pNetIf, ifname)
#endif

#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
static
int32_t ppa_set_intfid(struct netif_info *pNetIf, const char* ifname)
{
	uint8_t tmp_bitmap = 0,v = 0;
	int32_t  ret = PPA_SUCCESS;
	struct netif_info *get_ifinfo;
	tmp_bitmap = intf_bitmap;
	pNetIf->flowId = 0;
	pNetIf->tc = 0;

#if 0
            netif_1 = ppa_get_netif(ifname);
            if ( ppa_check_is_ppp_netif(netif) )
            {
                if ( ppa_check_is_pppoe_netif(netif) && ppa_pppoe_get_physical_if(netif, NULL, underlying_ifname) == PPA_SUCCESS )
                {
                    netif_2 = ppa_get_netif(underlying_ifname);
                }
	    }
#endif
	
        if ( ppa_netif_lookup(ifname, &get_ifinfo) != PPA_SUCCESS )
	{
		for(v=0;v<MAX_TC_NUM;v++)
		{
		    if(!(tmp_bitmap >> v)& 0x1)
		    {
			pNetIf->flowId_en = 1;
		        pNetIf->flowId = v;
			intf_bitmap =(tmp_bitmap | (1 << v));
			break;
		    }
		}
		if((pNetIf->flowId > 3) && (pNetIf->flowId < MAX_TC_NUM))
		{
		    pNetIf->tc = (pNetIf->flowId & 0xFC);
//		    pNetIf->flowId = (pNetIf->flowId & 0x3);
		}
	}
	/* Done with setting flowid & TC */
	return ret;
	
}

static
int32_t ppa_reset_intfid(struct netif_info *pNetIf, const char* ifname)
{
    	uint16_t tmp_bitmap = intf_bitmap;
	int32_t  ret = PPA_SUCCESS;
	if(pNetIf->flowId_en == 1)
	{
		tmp_bitmap &= (~(1 << pNetIf->flowId) & 0xFF);
		intf_bitmap = tmp_bitmap;
	}
	return ret;
}
#endif
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static 
int32_t ppa_netif_config_pmac(uint8_t uiFlowId, uint8_t uiRemBytes)
{
  int32_t i,j;
  GSW_PMAC_Eg_Cfg_t egCfg;
  GSW_PMAC_Ig_Cfg_t igCfg;
  GSW_API_HANDLE gswr;

  /* Do the GSWIP-R configuration */
  gswr = gsw_api_kopen("/dev/switch_api/1");
  if (gswr == 0) {
    ppa_debug(DBG_ENABLE_MASK_ERR,
        "%s: Open SWAPI device FAILED !!\n", __FUNCTION__ );
    return PPA_FAILURE;
  }

  memset((void *)&egCfg, 0x00, sizeof(egCfg));
  pr_info ("PMAC_EG_CFG_SET for GSW-R\n");
  for (i = 0; i <= 15; i++) {
    for (j = 0; j <= 3; j++) {
      egCfg.nRxDmaChanId  = 0;
      egCfg.bPmacEna      = 0; //no PMAC should go to PAE again  
      egCfg.bFcsEna       = 0;
      egCfg.bRemL2Hdr     = 1;
      egCfg.numBytesRem   = 20; //uiRemBytes;
      egCfg.nResDW1       = 0;
      egCfg.nRes1DW0      = 0;
      egCfg.nRes2DW0      = 0;
      egCfg.nDestPortId   = 11;
      egCfg.nTrafficClass = i;
      egCfg.bMpe1Flag     = 0;
      egCfg.bMpe2Flag     = 0;
      egCfg.bEncFlag      = 0;
      egCfg.bDecFlag      = 0;
      egCfg.nFlowIDMsb    = j; //uiFlowId;
      egCfg.bTCEnable     = 1;

      gsw_api_kioctl(gswr, GSW_PMAC_EG_CFG_SET, (unsigned int)&egCfg);
    }
  }

#if 1
  memset((void *)&igCfg, 0x00, sizeof(igCfg));
  {
   
    igCfg.nTxDmaChanId  = 11; 
    igCfg.bPmacPresent  = 0; /*((i > 5) && (i < 15)) ? 0 : 1; */
    /*igCfg.bSpIdDefault  = 0; */
    igCfg.bSpIdDefault  = 1; // For channel 15, use source port ID from default PMAC header
    igCfg.bSubIdDefault = 0; //((i == 6) || (i == 13)) ? 0 : 1;
    igCfg.bClassDefault = 0; 
    igCfg.bClassEna     = 0; 
    igCfg.bErrPktsDisc  = 0; 
    
    igCfg.bPmapDefault  = 1; // take PMAC from default
    igCfg.bPmapEna      = 0; 

    igCfg.defPmacHdr[0] = 0;
    igCfg.defPmacHdr[1] = 0;
    /*igCfg.defPmacHdr[2] = (i << 4); // Byte 2 (Bits 7:4)*/
    igCfg.defPmacHdr[2] =  0xB0; // For channel 15, source port is 0.
    igCfg.defPmacHdr[3] = 0x01; // Byte 3 (Bit 7)
    igCfg.defPmacHdr[4] = 0;
    igCfg.defPmacHdr[5] = 0;
    igCfg.defPmacHdr[6] = 0xFF;
    igCfg.defPmacHdr[7] = 0xFF;

    gsw_api_kioctl(gswr, GSW_PMAC_IG_CFG_SET, (unsigned int)&igCfg);
  }
#endif

  return PPA_SUCCESS;
}
#endif

int32_t ppa_netif_add(PPA_IFNAME *ifname, int f_is_lan, struct netif_info **pp_ifinfo, PPA_IFNAME *ifname_lower, int force_wanitf)
{
    struct netif_info *p_ifinfo;
    PPA_NETIF *netif, *netif_tmp=NULL;
    PPA_VCC *vcc;
    int32_t dslwan_qid;
    PPA_IFNAME underlying_ifname[PPA_IF_NAME_SIZE];
    uint32_t vid[2]={0};
    struct phys_port_info *p_phys_port = NULL;
    #define DEFAULT_OUTER_VLAN_ID 0x81000000
    PPA_NETIF *tmp_vlan_netif[2]={NULL};
    uint8_t add_flag_fail = 0;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    dp_subif_t dp_subif={0};
    PPE_OUT_VLAN_INFO vlan_info = {0};
    PPA_NETIF *brif=NULL;
    struct module *oner = NULL;
    uint32_t brip=0;
    PPA_CLASS_RULE class_rule;
#endif

//printk(KERN_INFO "%s %s %d ifname=%s ifname_lower=%s\n", __FILE__,__FUNCTION__,__LINE__,ifname,ifname_lower);
    //ppa_netif_lock_list();
    if ( ppa_netif_lookup(ifname, &p_ifinfo) != PPA_SUCCESS )
    {

        p_ifinfo = ppa_netif_alloc_item();
        if ( !p_ifinfo ){
            //ppa_netif_unlock_list();
            return PPA_ENOMEM;
        }

        ppa_strncpy(p_ifinfo->name, ifname, sizeof(p_ifinfo->name));
        if( ifname_lower && ppa_strlen(ifname_lower) )
        {
            ppa_strncpy(p_ifinfo->manual_lower_ifname, ifname_lower, sizeof(p_ifinfo->manual_lower_ifname)-1);
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Set manual_lower_ifname to %s\n", p_ifinfo->manual_lower_ifname);
        }
        else 
            p_ifinfo->manual_lower_ifname[0] = 0;

        netif = ppa_get_netif(ifname);
        if ( netif )
        {
            p_ifinfo->netif = netif;
LOOP_CHECK:
            if(netif->type == ARPHRD_SIT) {
                p_ifinfo->flags |= NETIF_PHY_TUNNEL;
                if(ppa_get_6rd_phyif_fn == NULL || (netif_tmp = ppa_get_6rd_phyif_fn(netif)) == NULL){
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Add sit device: %s, but cannot find the physical device\n", netif->name);
                }else{
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"sit device:%s, physical device:%s\n", netif->name, netif_tmp->name);
                    netif = netif_tmp;
                    ppa_add_subif(p_ifinfo,netif->name);
                }
            } else if(netif->type == ARPHRD_TUNNEL6) {
#if defined(CONFIG_LTQ_PPA_DSLITE) && CONFIG_LTQ_PPA_DSLITE
                p_ifinfo->flags |= NETIF_PHY_TUNNEL;
                if(ppa_get_ip4ip6_phyif_fn == NULL || (netif_tmp = ppa_get_ip4ip6_phyif_fn(netif)) == NULL){
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Add ip4ip6 device: %s, but cannot find the physical device\n", netif->name);
                }else{
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ip4ip6 device: %s, physical device: %s\n", netif->name, netif_tmp->name);
                    netif = netif_tmp;
                    ppa_add_subif(p_ifinfo,netif->name);
                }
#endif          
            } else {
              
              uint8_t isIPv4Gre = 0;
              uint8_t isGreTap = 0;
              
              if( ppa_is_gre_netif_type(netif, &isIPv4Gre, &isGreTap) ) {
                
                p_ifinfo->flags |= NETIF_PHY_TUNNEL | NETIF_GRE_TUNNEL;
                if(NULL == (netif_tmp = ppa_get_gre_phyif(netif))) {
                   ppa_debug(DBG_ENABLE_MASK_ERR,
                       "GRE device: %s, Could not find phyical device\n", 
                       netif->name);
                } else {
                  netif = netif_tmp;
                  ppa_add_subif(p_ifinfo,netif->name);
                  if( isIPv4Gre ) {
                    p_ifinfo->greInfo.flowId = (isGreTap)?FLOWID_IPV4_EoGRE:FLOWID_IPV4GRE;
                  } else {
                    p_ifinfo->greInfo.flowId = (isGreTap)?FLOWID_IPV6_EoGRE:FLOWID_IPV6GRE;
                  }
                }
              }
            }

            if ( ppa_strlen(p_ifinfo->manual_lower_ifname))
            {                    
                netif_tmp = ppa_get_netif(p_ifinfo->manual_lower_ifname);
                if( netif_tmp ) 
                {
                    netif = netif_tmp;
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"set netif to its manual_lower_ifname %s\n", p_ifinfo->manual_lower_ifname);
                    ppa_add_subif(p_ifinfo,netif->name); //BUG: Possible bug, might have already added
                }
                else
                {
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ifname %s not exist ??\n", p_ifinfo->manual_lower_ifname);
                }
            }

            if ( ppa_get_physical_if(netif, NULL, p_ifinfo->phys_netif_name) == PPA_SUCCESS )
            {
                p_ifinfo->flags |= NETIF_PHY_IF_GOT;
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"phys_netif_name=%s with ifname=%s\n", p_ifinfo->phys_netif_name, ifname );
                if( ppa_memcmp(p_ifinfo->phys_netif_name, ifname, ppa_strlen(ifname) ) == 0 )
                {
                    //if ( p_phys_port->type == 3 ) //only allow directpath interface to change its wanitf
                    p_ifinfo->f_wanitf.flag_root_itf = 1;
                }
            }
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
		/* Set FlowId & TC */
//		ppa_set_intfid(p_ifinfo,netif->name);
#endif
#if defined(L2TP_CONFIG) && L2TP_CONFIG
back_to_ppp: 
#endif
            if ( ppa_if_is_ipoa(netif, NULL) && ppa_br2684_get_vcc(netif, &vcc) == PPA_SUCCESS && (dslwan_qid = ppa_drv_get_dslwan_qid_with_vcc(vcc)) >= 0 )
            {
                p_ifinfo->flags |= NETIF_PHY_ATM | NETIF_BR2684 | NETIF_IPOA;
                p_ifinfo->vcc = vcc;
                p_ifinfo->dslwan_qid = dslwan_qid;
                goto PPA_NETIF_ADD_ATM_BASED_NETIF_DONE;
            }
            else if ( ppa_check_is_ppp_netif(netif) )
            {
                if ( ppa_check_is_pppoe_netif(netif) && ppa_pppoe_get_physical_if(netif, NULL, underlying_ifname) == PPA_SUCCESS )
                {
                    p_ifinfo->pppoe_session_id = ppa_pppoe_get_pppoe_session_id(netif);
                    if ( ppa_get_netif_hwaddr(netif, p_ifinfo->mac, 1) == PPA_SUCCESS )
                        p_ifinfo->flags |= NETIF_PPPOE | NETIF_MAC_AVAILABLE;
                    else
                        p_ifinfo->flags |= NETIF_PPPOE;
                    netif = ppa_get_netif(underlying_ifname);
                    if( netif )
                      ppa_add_subif(p_ifinfo,netif->name);
                }
                else if ( ppa_if_is_pppoa(netif, NULL) && ppa_pppoa_get_vcc(netif, &vcc) == PPA_SUCCESS && (dslwan_qid = ppa_drv_get_dslwan_qid_with_vcc(vcc)) >= 0 )
                {
                    p_ifinfo->flags |= NETIF_PHY_ATM | NETIF_PPPOATM;
                    p_ifinfo->vcc = vcc;
                    p_ifinfo->dslwan_qid = dslwan_qid;
                    goto PPA_NETIF_ADD_ATM_BASED_NETIF_DONE;
                }
#if defined(L2TP_CONFIG) && L2TP_CONFIG
                else if ( ppa_check_is_pppol2tp_netif(netif))
                {
                        if(ppa_pppol2tp_get_physical_if(netif, NULL, underlying_ifname))
                        {
                                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_pppol2tp_get_physical_if failed \n");
                        }
                        p_ifinfo->pppol2tp_session_id = ppa_pppol2tp_get_l2tp_session_id(netif);
                        p_ifinfo->pppol2tp_tunnel_id = ppa_pppol2tp_get_l2tp_tunnel_id(netif);
                        if ( ppa_get_netif_hwaddr(netif, p_ifinfo->mac, 1) == PPA_SUCCESS )
                        {
                                p_ifinfo->flags |= NETIF_PPPOL2TP | NETIF_MAC_AVAILABLE;
                        }
                        else
                        {
                                p_ifinfo->flags |= NETIF_PPPOL2TP;
                        }
                        netif = ppa_get_netif(underlying_ifname);
                        if ( ppa_check_is_ppp_netif(netif) )
                        {
                                goto back_to_ppp;
                        }
                }
#endif
            }
            else if ( ppa_get_netif_hwaddr(netif, p_ifinfo->mac, 1) == PPA_SUCCESS
                && (p_ifinfo->mac[0] | p_ifinfo->mac[1] | p_ifinfo->mac[2] | p_ifinfo->mac[3] | p_ifinfo->mac[4] | p_ifinfo->mac[5]) != 0 )
                p_ifinfo->flags |= NETIF_MAC_AVAILABLE;

            while ( netif && ppa_if_is_vlan_if(netif, NULL) && 
                ppa_vlan_get_underlying_if(netif, NULL, underlying_ifname) == PPA_SUCCESS )
            {
                PPA_NETIF *new_netif;
                p_ifinfo->flags |= NETIF_VLAN;

#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
		/* Set flowid & TC here*/
		ppa_set_intfid(p_ifinfo,netif->name);
#endif
		/* Done with setting flowid & TC */
                if ( p_ifinfo->vlan_layer < NUM_ENTITY(vid) )
                {
                    vid[p_ifinfo->vlan_layer] = ppa_get_vlan_id(netif);
                    tmp_vlan_netif[p_ifinfo->vlan_layer]=netif;
                }
                      
                p_ifinfo->vlan_layer++;
                new_netif = ppa_get_netif(underlying_ifname);
                if (new_netif == netif) {
                  /* VLAN interface and underlying interface share the same name! Break the loop */
                  break;
                }
                netif=new_netif;
                ppa_add_subif(p_ifinfo,netif->name);
            }

            /* Special handling for those interface which added with manually 
               specified lower interface with 
               ppacmd addwan/addlan -l lower_interface 
             */
            if( netif)
            {                
                struct netif_info *tmp_ifinfo;
                if ( __ppa_netif_lookup(netif->name, &tmp_ifinfo) == PPA_SUCCESS )
                {
                    if( tmp_ifinfo->manual_lower_ifname[0] )
                    {
                        PPA_NETIF *tmp_netif = ppa_get_netif(tmp_ifinfo->manual_lower_ifname);
                        if( tmp_netif )
                        {
                            netif = tmp_netif;
                            ppa_add_subif(p_ifinfo,netif->name); //Possible bug, might have already added
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
			    /* Set flowid & TC here*/
//			    ppa_set_intfid(p_ifinfo,netif->name);
#endif
                            ppa_atomic_dec(&tmp_ifinfo->count);
                            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,
                                "Need continue check for %s is manually added interface", 
                                tmp_ifinfo->manual_lower_ifname);
                            goto LOOP_CHECK;
                         }
                    }
                    ppa_atomic_dec(&tmp_ifinfo->count);
                }
            }

            if ( netif )
            {
                if ( ppa_if_is_br_if(netif, NULL) ) {
                    p_ifinfo->flags |= NETIF_BRIDGE;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
		    // Add pce rule to avoid local traffic from hitting the acceleration lookup
		    if(ppa_get_netif_ip(&brip, netif) == PPA_SUCCESS) {

			ppa_memset(&class_rule,0,sizeof(PPA_CLASS_RULE));

			class_rule.in_dev = GSWR_INGRESS;
			class_rule.category = CAT_MGMT;

			class_rule.pattern.bEnable=1;
			class_rule.pattern.eDstIP_Select = 1;
			class_rule.pattern.nDstIP.nIPv4 = brip;
			class_rule.pattern.nDstIP_Mask = 0xFF00;

			class_rule.action.fwd_action.rtaccelenable = 0;
			class_rule.action.fwd_action.rtctrlenable = 1;

			class_rule.action.fwd_action.processpath = 4; // CPU
			
			class_rule.action.rmon_action = 1;
    			class_rule.action.rmon_id = 23; //RMON_CPU_EG_CNTR;


			if(ppa_add_class_rule(&class_rule) == PPA_SUCCESS) { 
			
			    p_ifinfo->fid_index = class_rule.uidx;
			}
		    } 
#endif
  		}
                else if ( ppa_if_is_br2684(netif, NULL) && ppa_br2684_get_vcc(netif, &vcc) == PPA_SUCCESS && (dslwan_qid = ppa_drv_get_dslwan_qid_with_vcc(vcc)) >= 0 )
                {
                    p_ifinfo->flags |= NETIF_PHY_ATM | NETIF_BR2684 | NETIF_EOA;
                    p_ifinfo->vcc = vcc;
                    p_ifinfo->dslwan_qid = dslwan_qid;
                }
                else
                    p_ifinfo->flags |= NETIF_PHY_ETH;
            }

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
            if(ppa_check_if_netif_directpath(netif, 0) == PPA_SUCCESS) {
              p_ifinfo->flags |= NETIF_DIRECTPATH;  
              ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"<%s> Interface %s added to the list\n",__FUNCTION__,netif->name);
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
	      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"<%s> Calling ppa_set_intfid for interface %s \n",__FUNCTION__,netif->name);
			    ppa_set_intfid(p_ifinfo,netif->name);
#endif
              if ((dp_get_netif_subifid(netif, NULL, NULL, NULL, &dp_subif, 0))==PPA_SUCCESS) {
                              p_ifinfo->phys_port = dp_subif.port_id;
                              p_ifinfo->flags |= NETIF_PHYS_PORT_GOT;
                              p_ifinfo->subif_id = dp_subif.subif;
                          } else {
                            //ppa_netif_unlock_list();
                  return PPA_ENOTPOSSIBLE;
              }
            }
#endif
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
            /* Check whether the netif is directconnect fastpath interface or not */
            if (ppa_check_if_netif_fastpath_fn && ppa_check_if_netif_fastpath_fn(netif, NULL, 0))
            {
                p_ifinfo->flags |= NETIF_DIRECTCONNECT;

#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
	      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"<%s> Calling ppa_set_intfid for interface %s \n",__FUNCTION__,netif->name);
			    ppa_set_intfid(p_ifinfo,netif->name);
#endif
                /* Get PortId and SubifId={VapId} corresponding to the directconnect netif */
                if ((dp_get_netif_subifid(netif, NULL, NULL, NULL, &dp_subif, 0))==PPA_SUCCESS)
                {
                    p_ifinfo->phys_port = dp_subif.port_id;
                    p_ifinfo->flags |= NETIF_PHYS_PORT_GOT;
                    p_ifinfo->subif_id = dp_subif.subif;
                }
                else
                {
                    //ppa_netif_unlock_list();
                    return PPA_ENOTPOSSIBLE;
                }
            }
#endif
#endif
        }

PPA_NETIF_ADD_ATM_BASED_NETIF_DONE:
        if ( (p_ifinfo->flags & NETIF_PHY_ATM) )
        {
            //  ATM port is WAN port only
            if ( !f_is_lan )
            {
                if ( p_ifinfo->vlan_layer > 0 )
                {
                    if ( p_ifinfo->vlan_layer <= g_phys_port_atm_wan_vlan )
                    {
                        if ( p_ifinfo->vlan_layer == 2 )
                        {
                            p_ifinfo->inner_vid = vid[0];
                            p_ifinfo->outer_vid = DEFAULT_OUTER_VLAN_ID | vid[1];
                            p_ifinfo->flags |= NETIF_VLAN_INNER | NETIF_VLAN_OUTER;
                            p_ifinfo->in_vlan_netif = tmp_vlan_netif[0];
                            p_ifinfo->out_vlan_netif = tmp_vlan_netif[1];
                        }
                        else if ( p_ifinfo->vlan_layer == 1 )
                        {
                            if ( g_phys_port_atm_wan_vlan == 2 )
                            {
                                p_ifinfo->outer_vid = DEFAULT_OUTER_VLAN_ID | vid[0];
                                p_ifinfo->flags |= NETIF_VLAN_OUTER;
                                p_ifinfo->out_vlan_netif = tmp_vlan_netif[0];
                            }
                            else if ( g_phys_port_atm_wan_vlan == 1 )
                            {
                                p_ifinfo->inner_vid = vid[0];
                                p_ifinfo->flags |= NETIF_VLAN_INNER;
                                p_ifinfo->in_vlan_netif = tmp_vlan_netif[0];
                            }
                        }
                    }
                    else
                        p_ifinfo->flags |= NETIF_VLAN_CANT_SUPPORT;
                }
                p_ifinfo->phys_port = g_phys_port_atm_wan;
                p_ifinfo->flags |= NETIF_PHYS_PORT_GOT;
    
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
                if ((dp_get_netif_subifid(netif, NULL, p_ifinfo->vcc, NULL, &dp_subif, 0))==PPA_SUCCESS) {
                  p_ifinfo->subif_id = dp_subif.subif;
                  p_ifinfo->phys_port = dp_subif.port_id;
                }
#endif
            }
        }
        else if ( (p_ifinfo->flags & NETIF_PHY_IF_GOT) && !(p_ifinfo->flags & NETIF_BRIDGE) )
        {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
PPA_NETIF_ADD_PHYS_IF_CONTINUE:
#endif
            if ( ppa_phys_port_lookup(p_ifinfo->phys_netif_name, &p_phys_port) == PPA_SUCCESS )
            {  
                if ( p_ifinfo->vlan_layer > 0 )
                {
                    if ( p_ifinfo->vlan_layer <= p_phys_port->vlan )
                    {
                        if ( p_ifinfo->vlan_layer == 2 )
                        {
                            p_ifinfo->inner_vid = vid[0];
                            p_ifinfo->outer_vid = DEFAULT_OUTER_VLAN_ID | vid[1];
                            p_ifinfo->flags |= NETIF_VLAN_INNER | NETIF_VLAN_OUTER;
                            p_ifinfo->in_vlan_netif = tmp_vlan_netif[0];
                            p_ifinfo->out_vlan_netif = tmp_vlan_netif[1];
                        }
                        else if ( p_ifinfo->vlan_layer == 1 )
                        {
                            if ( p_phys_port->vlan == 2 )
                            {
                                p_ifinfo->outer_vid = DEFAULT_OUTER_VLAN_ID | vid[0];
                                p_ifinfo->flags |= NETIF_VLAN_OUTER;
                                p_ifinfo->out_vlan_netif = tmp_vlan_netif[0];
                            }
                            else if ( p_phys_port->vlan == 1 )
                            {
                                p_ifinfo->inner_vid = vid[0];
                                p_ifinfo->flags |= NETIF_VLAN_INNER;
                                p_ifinfo->in_vlan_netif = tmp_vlan_netif[0];                                
                            }
                        }
                    }
                    else
                        p_ifinfo->flags |= NETIF_VLAN_CANT_SUPPORT;
                }
       
	        p_ifinfo->phys_port = p_phys_port->port;
                p_ifinfo->flags |= NETIF_PHYS_PORT_GOT;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
		ppa_memset(&dp_subif,0,sizeof(dp_subif));
		//dp_subif.port_id = p_ifinfo->phys_port;
		//if Interface is VDSL so we need to get the Qid[6:3] from the base interface
		if ((dp_get_netif_subifid(netif, NULL, NULL, NULL, &dp_subif, 0))==PPA_SUCCESS) {
		    p_ifinfo->phys_port = dp_subif.port_id;
		    p_ifinfo->subif_id = dp_subif.subif;
		}

		if((p_ifinfo->flags & NETIF_VLAN_INNER) || (p_ifinfo->flags & NETIF_VLAN_OUTER)) {
		    
		    netif = (p_ifinfo->out_vlan_netif) ? p_ifinfo->out_vlan_netif : ((p_ifinfo->in_vlan_netif) ? p_ifinfo->in_vlan_netif : NULL);
    
		    if(netif) {
			ppa_memset(&dp_subif,0,sizeof(dp_subif));
			dp_subif.port_id = p_ifinfo->phys_port;
			//get the subifid field [11:8] from dp library
			if ((dp_get_netif_subifid(netif, NULL, NULL, NULL, &dp_subif, DP_F_SUBIF_LOGICAL))==PPA_SUCCESS) {
			    p_ifinfo->subif_id |= (dp_subif.subif) & 0xF00;
			} else {
			   if(strlen(p_ifinfo->manual_lower_ifname)==0) { // not the mac vlan interfaces
			   	dp_subif.subif = -1; 
	  
			    	oner = dp_get_module_owner(p_ifinfo->phys_port);	

			    	if(dp_register_subif(oner, p_ifinfo->netif, p_ifinfo->name, &dp_subif, DP_F_SUBIF_LOGICAL) != PPA_SUCCESS) {
				    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"dp_register_subif fail\n");
				    add_flag_fail = 1;
				} else {
				    p_ifinfo->subif_id |= (dp_subif.subif) & 0xF00;

				    vlan_info.port_id = p_ifinfo->phys_port;
				    vlan_info.subif_id = ((p_ifinfo->subif_id & 0xF00) >> 8);
				    //UGW stack currently does not support VLAN from the LAN side
				    //all incoming VLANs are removed. 
				    //Revisit when Q-in-Q support is added to UGW
				    if(p_ifinfo->flags & NETIF_VLAN_OUTER) {
					vlan_info.stag_vlan_id = p_ifinfo->outer_vid;
					vlan_info.stag_ins = 1;
				    //} else {
				    	vlan_info.stag_rem = 1;
				    }
			    
				    if(p_ifinfo->flags & NETIF_VLAN_INNER) {
					vlan_info.vlan_id = p_ifinfo->inner_vid;
					vlan_info.ctag_ins = 1;
				    //} else {
					vlan_info.ctag_rem = 1;
				    }
    
				    if ( ppa_hsel_add_outer_vlan_entry( &vlan_info, 0, PAE_HAL) != PPA_SUCCESS ){
					ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_hsel_add_outer_vlan_entry fail\n");
				    }            
				}
			    }
			}
		    }
		}
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"netif=%s portid=%d subifid=%d\n", netif->name, dp_subif.port_id, dp_subif.subif );
	    } else {
		//If the interface is not a part of physical interface list
		//Check if it is a dynamic interface
		//printk(KERN_INFO"%s %s %d netif=%s \n", __FILE__, __FUNCTION__, __LINE__, netif->name );
		ppa_memset(&dp_subif,0,sizeof(dp_subif));
		if ((dp_get_netif_subifid(netif, NULL, NULL, NULL, &dp_subif, 0))==PPA_SUCCESS) {
		    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"netif=%s portid=%d subifid=%d\n", netif->name, dp_subif.port_id, dp_subif.subif );
		    if(ppa_phys_port_add(netif->name,dp_subif.port_id) == PPA_SUCCESS) {
			p_ifinfo->subif_id = dp_subif.subif;
			goto PPA_NETIF_ADD_PHYS_IF_CONTINUE;
		    }
		}
#endif
            }

        }
  
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  //Is netdev under bridge
  //Note that device must be added to bridge before getting added to PPA
  if( (p_ifinfo->netif) && p_ifinfo->netif->priv_flags & IFF_BRIDGE_PORT ) {
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"netif=%s is under bridge\n", p_ifinfo->netif->name); 
    ppa_rtnl_lock();
    if((brif=ppa_netdev_master_upper_dev_get(p_ifinfo->netif)) != NULL) {
	ppa_rtnl_unlock();  
	if(ppa_if_is_br_if(brif, NULL)) {
	    if(ppa_get_fid(ppa_get_netif_name(brif), &p_ifinfo->fid)!=PPA_SUCCESS) {
		ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in getting fid of bridge %s\n", ppa_get_netif_name(brif));
	    }

	    // if fid > 0 then bridge is not br0; then we need to add PCE rules to set alternate FID
	    if(p_ifinfo->fid) {
		PPA_BR_PORT_INFO port_info={0};
		port_info.brid = p_ifinfo->fid;
		port_info.port = p_ifinfo->phys_port;
		port_info.vid = p_ifinfo->inner_vid;
		if(ppa_drv_add_br_port(&port_info, 0)==PPA_SUCCESS) {
		    p_ifinfo->fid_index = port_info.index;
		}
	    }
	}   
    } else {
      ppa_rtnl_unlock();
    }
  }

#endif
        if ( (p_ifinfo->flags & NETIF_MAC_AVAILABLE) )
        {
            PPE_ROUTE_MAC_INFO mac_info={0};
            ppa_memcpy(mac_info.mac, p_ifinfo->mac, sizeof(mac_info.mac));
            if ( ppa_drv_add_mac_entry( &mac_info, 0) == PPA_SUCCESS )
            {
                p_ifinfo->mac_entry = mac_info.mac_ix;
                p_ifinfo->flags |= NETIF_MAC_ENTRY_CREATED;
            }
            else
                ppa_debug(DBG_ENABLE_MASK_ERR,"add_mac_entry failed\n");
        }      
        
        __ppa_netif_add_item(p_ifinfo);        
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"interface %s already exist\n", ifname);
        if( ifname_lower && ppa_strlen(ifname_lower) && ppa_strlen(p_ifinfo->manual_lower_ifname) &&
          strcmp( ifname_lower, p_ifinfo->manual_lower_ifname ) != 0 )
        { /* conflicts and it should update its physical port id and names. But temporarily we just return fail  */
            add_flag_fail = 1;
        }
    }        

    if( force_wanitf )
    {
        p_ifinfo->f_wanitf.flag_force_wanitf = force_wanitf;        
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"step 1: flag_force_wanitf=%d, flag_root_itf=%d,flag_already_wanitf=%d\n", p_ifinfo->f_wanitf.flag_force_wanitf, p_ifinfo->f_wanitf.flag_root_itf, p_ifinfo->f_wanitf.flag_already_wanitf);
        if( p_ifinfo->f_wanitf.flag_root_itf && p_ifinfo->f_wanitf.flag_force_wanitf )
        {  //since it is physical root interface and the force_wanitf flag is set, then try to set wantif value accordingly
            PPE_WANITF_CFG wanitf_cfg={0};
            
            wanitf_cfg.lan_flag = f_is_lan?1:0;
            wanitf_cfg.physical_port = p_ifinfo->phys_port;
            if( ppa_set_wan_itf( &wanitf_cfg, 0 ) == PPA_SUCCESS )
            {
                if( ! p_ifinfo->f_wanitf.flag_already_wanitf )
                {
                    p_ifinfo->f_wanitf.flag_already_wanitf= 1; //only if 
                    p_ifinfo->f_wanitf.old_lan_flag = wanitf_cfg.old_lan_flag; //only if 
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"flag_already_wanitf set to 1 with old_lan_flag=%d\n", wanitf_cfg.old_lan_flag);
                }
            }
            else ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_set_wan_itf fail\n");                
        }
    }

    p_ifinfo->flags &= ~( NETIF_LAN_IF | NETIF_WAN_IF ); //unset all LAN/WAN flag 
    p_ifinfo->flags |= f_is_lan ? NETIF_LAN_IF : NETIF_WAN_IF; //reset LAN/WAN flag
    if ( pp_ifinfo )
    {
        *pp_ifinfo = p_ifinfo;        
    }
    else
        ppa_netif_free_item(p_ifinfo);

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    if( !add_flag_fail && p_ifinfo->flags & NETIF_GRE_TUNNEL ) {
      /* Add GRE PCE rule */
      ppa_get_gre_hdrlen(p_ifinfo->netif, &p_ifinfo->greInfo.tnl_hdrlen);
#if 0 //Single pass method is used
      ppa_netif_config_pmac(p_ifinfo->greInfo.flowId,p_ifinfo->greInfo.tnl_hdrlen);
      p_ifinfo->greInfo.extId = 150;
      p_ifinfo->greInfo.pceRuleIndex = 70;
#endif
    } 
#endif
    //ppa_netif_unlock_list();

    if( add_flag_fail ) return PPA_FAILURE;
    
    return PPA_SUCCESS;
}

void ppa_netif_remove(PPA_IFNAME *ifname, int f_is_lan)
{
    struct netif_info *p_ifinfo;
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
    uint16_t tmp_bitmap = intf_bitmap;
#endif
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    struct module *oner = NULL;
    dp_subif_t dp_subif = {0};
    PPA_CLASS_RULE class_rule;
#endif

    if ( ppa_netif_lookup(ifname, &p_ifinfo) == PPA_SUCCESS )
    {
        p_ifinfo->flags &= f_is_lan ? ~NETIF_LAN_IF : ~NETIF_WAN_IF;
        if ( !(p_ifinfo->flags & (NETIF_LAN_IF | NETIF_WAN_IF)) )
        {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
      if(p_ifinfo->fid) {
    	PPA_BR_PORT_INFO port_info={0};
    	port_info.brid = p_ifinfo->fid;
    	port_info.port = p_ifinfo->phys_port;
    	port_info.index =  p_ifinfo->fid_index;
    	ppa_drv_del_br_port(&port_info, 0);
      }

    if(p_ifinfo->flags & NETIF_BRIDGE) {

	ppa_memset(&class_rule,0,sizeof(PPA_CLASS_RULE));

	class_rule.in_dev = GSWR_INGRESS;
        class_rule.category = CAT_MGMT;	
	class_rule.uidx = p_ifinfo->fid_index;

	ppa_del_class_rule(&class_rule);
    }

    if((p_ifinfo->flags & NETIF_VLAN_INNER) || (p_ifinfo->flags & NETIF_VLAN_OUTER)) {

	if(strlen(p_ifinfo->manual_lower_ifname)==0) { // not the mac vlan interfaces

	    oner = dp_get_module_owner(p_ifinfo->phys_port); 

	    dp_subif.port_id = p_ifinfo->phys_port;
	    dp_subif.subif = p_ifinfo->subif_id;

	    if(dp_register_subif(oner, p_ifinfo->netif, p_ifinfo->name, &dp_subif, DP_F_DEREGISTER) != PPA_SUCCESS) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"dp_register_subif deregister failed\n");
	    } else { 
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s unregistered!!\n", p_ifinfo->name);  
	    }
	}
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," %s:%d : before reset : intf_bitmap = %d \n",intf_bitmap,__FUNCTION__,__LINE__);
	ppa_reset_intfid(p_ifinfo,ifname);
#if 0
	tmp_bitmap &= (~(1 << p_ifinfo->flowId) & 0xFF);
	intf_bitmap = tmp_bitmap;
#endif
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," %s:%d : after reset : intf_bitmap = %d \n",intf_bitmap,__FUNCTION__,__LINE__);
#endif   

    }
#if defined(VLAN_VAP_QOS) && VLAN_VAP_QOS
        if(ppa_check_if_netif_directpath(p_ifinfo->netif, 0) == PPA_SUCCESS) {
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," %s:%d : before reset for DP interface : intf_bitmap = %d \n",intf_bitmap,__FUNCTION__,__LINE__);
		ppa_reset_intfid(p_ifinfo,ifname);
	ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT," %s:%d : after reset for DP interface : intf_bitmap = %d \n",intf_bitmap,__FUNCTION__,__LINE__);
	}
#endif   
#endif
            ppa_netif_remove_item(ifname, NULL);
            if ( (p_ifinfo->flags & NETIF_MAC_ENTRY_CREATED) )
            {
                PPE_ROUTE_MAC_INFO mac_info={0};
                mac_info.mac_ix = p_ifinfo->mac_entry;
                ppa_drv_del_mac_entry(&mac_info, 0);
                p_ifinfo->mac_entry = ~0;
                p_ifinfo->flags &= ~NETIF_MAC_ENTRY_CREATED;
            }
        }

        ppa_netif_free_item(p_ifinfo);
    }
}

int32_t __ppa_netif_lookup(PPA_IFNAME *ifname, struct netif_info **pp_info)    //  netif_info_is_added
{
    int32_t ret = PPA_ENOTAVAIL;
    struct netif_info *p;

    //ppa_netif_lock_list();
    for ( p = g_netif; p; p = p->next )
        if ( strcmp(p->name, ifname) == 0 )
        {
            ret = PPA_SUCCESS;
            if ( pp_info )
            {
                ppa_atomic_inc(&p->count);
                *pp_info = p;
            }
            break;
        }
    //ppa_netif_unlock_list();

    return ret;
}

int32_t ppa_netif_lookup(PPA_IFNAME *ifname, struct netif_info **pp_info)    //  netif_info_is_added
{
    int32_t ret;
    ppa_netif_lock_list();
    ret = __ppa_netif_lookup(ifname, pp_info);
    ppa_netif_unlock_list();

    return ret;
}


void ppa_netif_put(struct netif_info *p_info)
{
    if(p_info)
	ppa_netif_free_item(p_info);
}

int32_t ppa_netif_update(PPA_NETIF *netif, PPA_IFNAME *ifname)
{
    struct netif_info *p_info;
    int f_need_update = 0;
    uint32_t flags;    
    PPE_ROUTE_MAC_INFO mac_info={0};
    uint32_t force_wantif;
    PPA_IFNAME manual_lower_ifname[PPA_IF_NAME_SIZE];
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE];
    PPA_NETIF *new_netif;
#endif

    if ( netif )
        ifname = ppa_get_netif_name(netif);
    else
        netif = ppa_get_netif(ifname);

    if ( !netif || !ifname ){
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"fail: cannot find device\n");
        return PPA_EINVAL;
    }

    if ( ppa_netif_lookup(ifname, &p_info) != PPA_SUCCESS ){
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"fail: device: %s not accelerated\n", ifname);
        return PPA_ENOTAVAIL;
    }

    flags = p_info->flags;

    if ( !ppa_is_netif_equal(netif, p_info->netif) )
        f_need_update = 1;
    else if ( (flags & NETIF_PHYS_PORT_GOT) == 0 && (flags & (NETIF_BRIDGE | NETIF_PHY_IF_GOT)) != (NETIF_BRIDGE | NETIF_PHY_IF_GOT) )
        f_need_update = 1;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    else if( (flags & NETIF_PPPOL2TP))
    {
        if ( !ppa_check_is_pppol2tp_netif(netif) )
        {
            f_need_update = 1;
        }
        else
        {
            p_info->pppol2tp_session_id = ppa_pppol2tp_get_l2tp_session_id(netif);
            p_info->pppol2tp_tunnel_id = ppa_pppol2tp_get_l2tp_tunnel_id(netif);

            if(ppa_pppol2tp_get_base_netif(netif, phy_ifname) == PPA_SUCCESS )
            {
                new_netif = ppa_get_netif(phy_ifname);
                if( ppa_check_is_ppp_netif(new_netif))
                {
                    if ( !ppa_check_is_pppoe_netif(new_netif) )
                    {
                        f_need_update = 1;
                    }
                    else
                    {
                        p_info->pppoe_session_id = ppa_pppoe_get_pppoe_session_id(new_netif);
                    }
                }
            }
        }
    }
#endif
    else if ( (flags & NETIF_PPPOE) )
    {
        if ( !ppa_check_is_pppoe_netif(netif) )
            f_need_update = 1;
        else
            p_info->pppoe_session_id = ppa_pppoe_get_pppoe_session_id(netif);
    }
    else if ( (flags & NETIF_PPPOATM) )
    {
        if ( !ppa_if_is_pppoa(netif, NULL) )
            f_need_update = 1;
    }

    if ( !f_need_update && (flags & (NETIF_BRIDGE | NETIF_PHY_ETH | NETIF_EOA)) != 0 )
    {
        //  update MAC address
        if ( ppa_get_netif_hwaddr(netif, mac_info.mac, 1) == PPA_SUCCESS
            && (mac_info.mac[0] | mac_info.mac[1] | mac_info.mac[2] | mac_info.mac[3] | mac_info.mac[4] | mac_info.mac[5]) != 0 )
        {
            if ( ppa_memcmp(p_info->mac, mac_info.mac, PPA_ETH_ALEN) != 0 )
            {                
                mac_info.mac_ix = p_info->mac_entry;
                ppa_drv_del_mac_entry( &mac_info, 0);
                if ( ppa_drv_add_mac_entry( &mac_info, 0) == PPA_SUCCESS )
                {
                    p_info->mac_entry = mac_info.mac_ix;
                    p_info->flags |= NETIF_MAC_ENTRY_CREATED;
                }
                else
                {
                    p_info->mac_entry = ~0;
                    p_info->flags &= ~NETIF_MAC_ENTRY_CREATED;
                }
                ppa_memcpy(p_info->mac, mac_info.mac, PPA_ETH_ALEN);
                p_info->flags |= NETIF_MAC_AVAILABLE;
            }
        }
        else
        {
            mac_info.mac_ix = p_info->mac_entry;
            ppa_drv_del_mac_entry( &mac_info, 0);
            p_info->mac_entry = ~0;
            p_info->flags &= ~(NETIF_MAC_ENTRY_CREATED | NETIF_MAC_AVAILABLE);
        }
    }

    force_wantif = p_info->f_wanitf.flag_force_wanitf; //save the force_wanitf flag
    if( ppa_strlen(p_info->manual_lower_ifname) )
        ppa_strncpy(manual_lower_ifname, p_info->manual_lower_ifname, sizeof(manual_lower_ifname));
    else
        manual_lower_ifname[0] = 0;
    ppa_netif_put(p_info);

    if ( f_need_update )
    {
        if ( (flags & NETIF_LAN_IF) )
            ppa_netif_remove(ifname, 1);
        if ( (flags & NETIF_WAN_IF) )
            ppa_netif_remove(ifname, 0);

        if ( (flags & NETIF_LAN_IF) && ppa_netif_add(ifname, 1, NULL, manual_lower_ifname, force_wantif) != PPA_SUCCESS ){
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update lan interface %s fail\n",ifname);
            return PPA_FAILURE;
        }

        if ( (flags & NETIF_WAN_IF) && ppa_netif_add(ifname, 0, NULL, manual_lower_ifname, force_wantif) != PPA_SUCCESS ){
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"update wan interface %s fail\n", ifname);
            return PPA_FAILURE;
        }        
        
    }

    return PPA_SUCCESS;
}

int32_t ppa_netif_start_iteration(uint32_t *ppos, struct netif_info **ifinfo)
{
    uint32_t l;
    struct netif_info *p;

    ppa_netif_lock_list();

    if( !ppa_is_init() )
    {
      *ifinfo = NULL;
       return PPA_FAILURE; 
    }

    for ( p = g_netif, l = *ppos; p && l; p = p->next, l-- );

    if ( l == 0 && p )
    {
        ++*ppos;
        *ifinfo = p;
        return PPA_SUCCESS;
    }
    else
    {
        *ifinfo = NULL;
        return PPA_FAILURE;
    }
}

int32_t ppa_netif_iterate_next(uint32_t *ppos, struct netif_info **ifinfo)
{
    if ( *ifinfo )
    {
        ++*ppos;
        *ifinfo = (*ifinfo)->next;
        return *ifinfo ? PPA_SUCCESS : PPA_FAILURE;
    }
    else
        return PPA_FAILURE;
}

void ppa_netif_stop_iteration(void)
{
    ppa_netif_unlock_list();
}

/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

int32_t ppa_api_netif_manager_init(void)
{
    struct phys_port_info *p_phys_port_info;
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
    struct ppe_directpath_data *info;
#endif
    uint32_t pos;
    int i,ret=PPA_SUCCESS;
    PPE_IFINFO if_info;
    PPE_COUNT_CFG count={0};

    ppa_netif_free_list();
    ppa_phys_port_free_list();

    ppa_drv_get_number_of_phys_port( &count, 0);

    
    for ( i = 0; i <= count.num; i++ )
    {
        ppa_memset(&if_info,0,sizeof(PPE_IFINFO)); 
        if_info.port = i;
        ppa_drv_get_phys_port_info( &if_info, 0);
        switch ( (if_info.if_flags & (PPA_PHYS_PORT_FLAGS_TYPE_MASK | PPA_PHYS_PORT_FLAGS_MODE_MASK | PPA_PHYS_PORT_FLAGS_VALID)) )
        {
        case PPA_PHYS_PORT_FLAGS_MODE_CPU_VALID:
            if ( g_phys_port_cpu == ~0 )
                g_phys_port_cpu = i;
            break;
        case PPA_PHYS_PORT_FLAGS_MODE_ATM_WAN_VALID:
            if ( g_phys_port_atm_wan == ~0 )
                g_phys_port_atm_wan = i;
            g_phys_port_atm_wan_vlan = (if_info.if_flags & PPA_PHYS_PORT_FLAGS_OUTER_VLAN) ? 2 : 1;
            break;
        case PPA_PHYS_PORT_FLAGS_MODE_ETH_LAN_VALID:
        case PPA_PHYS_PORT_FLAGS_MODE_ETH_WAN_VALID:
        case PPA_PHYS_PORT_FLAGS_MODE_ETH_MIX_VALID:
        case PPA_PHYS_PORT_FLAGS_MODE_EXT_LAN_VALID:
            if ( if_info.ifname[0] )
            {
                p_phys_port_info = ppa_phys_port_alloc_item();
                if ( !p_phys_port_info )
                    goto PPA_API_NETIF_MANAGER_INIT_FAIL;
                strcpy(p_phys_port_info->ifname, if_info.ifname);
                switch ( (if_info.if_flags & PPA_PHYS_PORT_FLAGS_MODE_MASK) )
                {
                case PPA_PHYS_PORT_FLAGS_MODE_LAN: p_phys_port_info->mode = 1; break;
                case PPA_PHYS_PORT_FLAGS_MODE_WAN: p_phys_port_info->mode = 2; break;
                case PPA_PHYS_PORT_FLAGS_MODE_MIX: p_phys_port_info->mode = 3;
                }
                p_phys_port_info->type = (if_info.if_flags & PPA_PHYS_PORT_FLAGS_TYPE_MASK) == PPA_PHYS_PORT_FLAGS_TYPE_ETH ? 2 : 3;
                p_phys_port_info->vlan = (if_info.if_flags & PPA_PHYS_PORT_FLAGS_OUTER_VLAN) ? 2 : 1;
                p_phys_port_info->port = i;
                ppa_lock_get(&g_phys_port_lock);
                p_phys_port_info->next = g_phys_port_info;
                g_phys_port_info = p_phys_port_info;
                ppa_lock_release(&g_phys_port_lock);
            }
        }
    }

    pos = 0;
#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
    if (ppa_is_init()) {
  if ((ret = ppa_directpath_data_start_iteration(&pos, &info)) == PPA_SUCCESS )
  {
      do
      {
    if ( (info->flags & PPE_DIRECTPATH_DATA_ENTRY_VALID) && info->netif )
        ppa_phys_port_add(info->netif->name, info->ifid);
      } while ( ppa_directpath_data_iterate_next(&pos, &info) == PPA_SUCCESS );
  }
  
  ppa_directpath_data_stop_iteration();
    }
#endif  

    return ret;

PPA_API_NETIF_MANAGER_INIT_FAIL:
    ppa_phys_port_free_list();
    return PPA_ENOMEM;
}

void ppa_api_netif_manager_exit(void)
{
    ppa_netif_free_list();
    ppa_phys_port_free_list();
}

int32_t ppa_api_netif_manager_create(void)
{
    if ( ppa_lock_init(&g_phys_port_lock) )
    {
        ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in creating lock for physical network interface list.\n");
        return PPA_EIO;
    }

    if ( ppa_lock_init(&g_netif_lock) )
    {
        ppa_lock_destroy(&g_phys_port_lock);
        ppa_debug(DBG_ENABLE_MASK_ERR,"Failed in creating lock for network interface list.\n");
        return PPA_EIO;
    }

    return PPA_SUCCESS;
}

void ppa_api_netif_manager_destroy(void)
{
    ppa_lock_destroy(&g_netif_lock);
    ppa_lock_destroy(&g_phys_port_lock);
}



EXPORT_SYMBOL(g_phys_port_cpu);
EXPORT_SYMBOL(g_phys_port_atm_wan);
EXPORT_SYMBOL(g_phys_port_atm_wan_vlan);
EXPORT_SYMBOL(ppa_phys_port_start_iteration);
EXPORT_SYMBOL(ppa_phys_port_iterate_next);
EXPORT_SYMBOL(ppa_phys_port_stop_iteration);
EXPORT_SYMBOL(ppa_netif_update);
EXPORT_SYMBOL(ppa_netif_start_iteration);
EXPORT_SYMBOL(ppa_netif_iterate_next);
EXPORT_SYMBOL(ppa_netif_stop_iteration);
EXPORT_SYMBOL(ppa_netif_lock_list);
EXPORT_SYMBOL(ppa_netif_unlock_list);
