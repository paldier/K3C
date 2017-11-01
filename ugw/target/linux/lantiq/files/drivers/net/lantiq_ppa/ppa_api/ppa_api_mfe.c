/******************************************************************************
**
** FILE NAME    : ppa_api_mfe.c
** PROJECT      : UEIP
** MODULES      : PPA API ( Multiple Field Based Classification and VLAN Assignment )
**
** DATE         : 11 March 2009
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack MMultiple Field Based Classification and VLAN Assignment API Implementation
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author              $Comment
** 16 DEC 2009  Shao Guohua          Initiate Version
*******************************************************************************/
/*!
  \defgroup IFX_PPA_API_MFE PPA API Multiple Field Editing functions
  \ingroup IFX_PPA_API
  \brief IFX PPA API Multiple Field Editing functions
*/

/*!
 \file ppa_api_mfe.c
 \ingroup IFX_PPA_API
 \brief source file for PPA API Multiple Field Based Classification and VLAN Assignment  functions
*/
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
//#include <asm/time.h>

//PPA Specific Head File
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include <net/ppa_stack_al.h>
#include "ppa_api_mfe.h"
#include "ppa_api_session.h"
#include "ppe_drv_wrapper.h"
#include "ppa_hal_wrapper.h"

#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
int32_t ppa_get_multifield_flow( int32_t index, PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag );   //hook

int32_t ppa_multifield_control(uint8_t enable, uint32_t flag)  //hook
{
    PPE_ENABLE_CFG mfe_cfg={0};
    mfe_cfg.f_enable = enable;
    mfe_cfg.flags = flag;
    
    return ppa_drv_multifield_control( &mfe_cfg, 0);
}

int32_t ppa_get_multifield_status(uint8_t *enable, uint32_t flag)  //hook
{
    int32_t res;

    PPE_ENABLE_CFG mfe_cfg={0};

    res = ppa_drv_get_multifield_status( &mfe_cfg, 0);
    if( enable)
    {    
        if( res == PPA_SUCCESS )
            *enable = mfe_cfg.f_enable;
        else *enable = 0;
    }

    return res;
}

int32_t ppa_get_multifield_max_flow(uint32_t flag)   //hook
{
    int32_t res;
    PPE_COUNT_CFG count={0};

    count.flags = flag;
    
    res = ppa_drv_get_multifield_max_entry( &count, count.flags);

    if( res == PPA_SUCCESS )
        return count.num;
    else return 0;
}

static void ppa_mf_flow_display(PPA_MULTIFIELD_FLOW_INFO *flow) //internal
{
    uint32_t full_id, mask;

    if( !(g_ppa_dbg_enable & DBG_ENABLE_MASK_MFE ) ) return;

    if( flow->cfg0.vlan_info.bfauto )
    {
        ppa_debug(DBG_ENABLE_MASK_MFE,"\tAcclerate packet From %s (port %d) to %s ( port %d) via queue %d %s\n", flow->cfg0.vlan_info.vlan_info_auto.rx_ifname, flow->cfg0.vlan_info.vlan_info_manual.rx_if_id,
                        flow->cfg0.vlan_info.vlan_info_auto.tx_ifname, flow->cfg0.vlan_info.vlan_info_manual.tx_if_id, flow->cfg0.queue_id, flow->cfg0.fwd_cpu ?"fwd to CPU":"");
        ppa_debug(DBG_ENABLE_MASK_MFE,"\tIts detail flow information as below:\n");
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_MFE,"\tAcclerate packet From port id %d to %d via queue %d %s\n", flow->cfg0.vlan_info.vlan_info_manual.rx_if_id, flow->cfg0.vlan_info.vlan_info_manual.tx_if_id, flow->cfg0.queue_id, flow->cfg0.fwd_cpu ?"fwd to CPU":"as original Destination");
    }
    
    if( flow->cfg0.vlan_info.vlan_info_manual.action_out_vlan_insert )
    {
        VLAN_ID_CONBINE(full_id, flow->cfg0.vlan_info.vlan_info_manual.new_out_vlan_pri, flow->cfg0.vlan_info.vlan_info_manual.new_out_vlan_cfi, flow->cfg0.vlan_info.vlan_info_manual.new_out_vlan_vid );
        full_id |= (flow->cfg0.vlan_info.vlan_info_manual.new_out_vlan_tpid << 16 );
        ppa_debug(DBG_ENABLE_MASK_MFE,"\tInsert new out vlan id 0x%04x\n", (unsigned int)full_id );
    }
    if( flow->cfg0.vlan_info.vlan_info_manual.action_in_vlan_insert )
    {
        VLAN_ID_CONBINE(full_id, flow->cfg0.vlan_info.vlan_info_manual.new_in_vlan_pri, flow->cfg0.vlan_info.vlan_info_manual.new_in_vlan_cfi, flow->cfg0.vlan_info.vlan_info_manual.new_in_vlan_vid );
        ppa_debug(DBG_ENABLE_MASK_MFE,"\tInsert new inner vlan id 0x%04x\n", (unsigned int)full_id );
    }

    if( flow->cfg0.vlan_info.vlan_info_manual.action_out_vlan_remove ||flow->cfg0.vlan_info.vlan_info_manual.action_in_vlan_remove)
        ppa_debug(DBG_ENABLE_MASK_MFE,"\tRemove: %s %s \n", flow->cfg0.vlan_info.vlan_info_manual.action_out_vlan_remove?"outer vlan":"", flow->cfg0.vlan_info.vlan_info_manual.action_in_vlan_remove?"inner vlan":"");

    ppa_debug(DBG_ENABLE_MASK_MFE,"\tFlow classification info:\n");
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch vlan number: 0x%01x/0x%01x\n", flow->cfg0.vlan_info.vlan_info_manual.is_vlan, flow->cfg0.vlan_info.vlan_info_manual.is_vlan_mask);

    VLAN_ID_CONBINE(full_id, flow->cfg0.vlan_info.vlan_info_manual.out_vlan_pri, flow->cfg0.vlan_info.vlan_info_manual.out_vlan_cfi, flow->cfg0.vlan_info.vlan_info_manual.out_vlan_vid );
    VLAN_ID_CONBINE(mask,   flow->cfg0.vlan_info.vlan_info_manual.out_vlan_pri_mask, flow->cfg0.vlan_info.vlan_info_manual.out_vlan_cfi_mask, flow->cfg0.vlan_info.vlan_info_manual.out_vlan_vid_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch out vlan id 0x%04x/0x%04x\n", (unsigned int)full_id, (unsigned int)mask );

    VLAN_ID_CONBINE(full_id, flow->cfg0.vlan_info.vlan_info_manual.in_vlan_pri, flow->cfg0.vlan_info.vlan_info_manual.in_vlan_cfi, flow->cfg0.vlan_info.vlan_info_manual.in_vlan_vid );
    VLAN_ID_CONBINE(mask,   flow->cfg0.vlan_info.vlan_info_manual.in_vlan_pri_mask, flow->cfg0.vlan_info.vlan_info_manual.in_vlan_cfi_mask, flow->cfg0.vlan_info.vlan_info_manual.in_vlan_vid_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch inner vlan id 0x%04x/0x%04x\n", (unsigned int)full_id, (unsigned int)mask );

    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch DSCP: 0x%x/0x%x\n", flow->cfg0.dscp, flow->cfg0.dscp_mask );
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch Ether Type: 0x%x/0x%x\n", flow->cfg0.ether_type, flow->cfg0.ether_type_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch IPV4 flag: 0x%x/0x%x\n", flow->cfg0.ipv4, flow->cfg0.ipv4_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch IPV6 flag: 0x%x/0x%x\n", flow->cfg0.ipv6, flow->cfg0.ipv6_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch L3 off0: 0x%x/0x%x\n", flow->cfg0.l3_off0, flow->cfg0.l3_off0_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch L3 off1: 0x%x/0x%x\n", flow->cfg0.l3_off1, flow->cfg0.l3_off1_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch source ip: 0x%x(%u.%u.%u.%u)/0x%x\n", (unsigned int)flow->cfg0.s_ip, NIPQUAD(flow->cfg0.s_ip), (unsigned int)flow->cfg0.s_ip_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch packet length: 0x%x/0x%x\n", (unsigned int)flow->cfg0.pkt_length, (unsigned int)flow->cfg0.pkt_length_mask);
    ppa_debug(DBG_ENABLE_MASK_MFE,"\tMatch pppoe session: 0x%x/0x%x\n", (unsigned int)flow->cfg0.pppoe_session, (unsigned int)flow->cfg0.pppoe_session_mask);
}

static int32_t ppa_auto_learn_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info) //internal
{ //note, in vlan information auto-learnign, here set all vlan related mask to zero
    int32_t res = PPA_SUCCESS;

    struct netif_info *rx_ifinfo=NULL, *tx_ifinfo=NULL;

    if ( p_multifield_info->cfg0.vlan_info.bfauto )
    {
        p_multifield_info->cfg0.vlan_info.vlan_info_manual.is_vlan_mask = 0;
       
        p_multifield_info->cfg0.ether_type_mask = -1;
        p_multifield_info->cfg0.pppoe_session_mask = -1;
        p_multifield_info->cfg0.dscp_mask = -1;
        p_multifield_info->cfg0.ipv4_mask = -1;
        p_multifield_info->cfg0.ipv6_mask = -1;
        p_multifield_info->cfg0.s_ip_mask = -1;
        p_multifield_info->cfg0.l3_off0_mask = -1;
        p_multifield_info->cfg0.l3_off1_mask = -1;
        p_multifield_info->cfg0.pkt_length_mask = -1;        
    }

    if( p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname[0] != 0 )  //for auto and manul 
    {
        if( ppa_netif_update(NULL, p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname)  != PPA_SUCCESS )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: ppa_netif_update fail for rx interface %s\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname);
            return PPA_FAILURE;
        }

        if ( ppa_netif_lookup(p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname, &rx_ifinfo) != PPA_SUCCESS )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: Rx interface %s ppa_netif_lookup fail\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname);
            return PPA_FAILURE;
        }        
        if ( !(rx_ifinfo->flags & NETIF_PHYS_PORT_GOT) )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: Rx interface %s no physical port id\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname);
            res = PPA_FAILURE;
            goto EXIT;
        }
        
    }

    if( p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname[0] != 0  ) //for auto and manul 
    {
         if( ppa_netif_update(NULL, p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname)  != PPA_SUCCESS )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: ppa_netif_update fail for tx interface %s\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname);
            return PPA_FAILURE;
        }
        if ( ppa_netif_lookup(p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname, &tx_ifinfo) != PPA_SUCCESS )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: Tx interface %s ppa_netif_lookup fail\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname);
            res = PPA_FAILURE;
            goto EXIT;
        }        
        if ( !(tx_ifinfo->flags & NETIF_PHYS_PORT_GOT) )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: Tx interface %s no physical port id\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname);
            res = PPA_FAILURE;
            goto EXIT;
        }
    }

    
    if( rx_ifinfo ) p_multifield_info->cfg0.vlan_info.vlan_info_manual.rx_if_id = rx_ifinfo->phys_port;   //for auto and manul 
    if( tx_ifinfo ) p_multifield_info->cfg0.vlan_info.vlan_info_manual.tx_if_id = tx_ifinfo->phys_port;   //for auto and manul 

    if ( p_multifield_info->cfg0.vlan_info.bfauto )
    {
        if( rx_ifinfo->flags & NETIF_VLAN_CANT_SUPPORT )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: rx interface %s cannot support too many vlan tag\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.rx_ifname);
            res = PPA_FAILURE;
            goto EXIT;
        }
        if( tx_ifinfo->flags & NETIF_VLAN_CANT_SUPPORT )
        {
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: tx interface %s cannot support too many vlan tag\n", p_multifield_info->cfg0.vlan_info.vlan_info_auto.tx_ifname);
            res = PPA_FAILURE;
            goto EXIT;
        }

        if( rx_ifinfo->flags & NETIF_VLAN_OUTER )
        {
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.action_out_vlan_remove =1; //action: remove out vlan
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_pri_mask = -1;  //don't match pri/cfi at present
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_cfi_mask = -1;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_vid_mask = 0;

            //set is_vlan and vlan tag match/classification
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.is_vlan ++;
            VLAN_ID_SPLIT( rx_ifinfo->outer_vid, p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_pri, p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_cfi, p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_vid );
        }
        else
        {
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_pri = 0;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_cfi = 0;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_vid = 0;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_pri_mask = -1;  //don't match pri/cfi at present
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_cfi_mask = -1;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.out_vlan_vid_mask = -1;
        }

        if( rx_ifinfo->flags & NETIF_VLAN_INNER )
        {
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.action_in_vlan_remove =1;  //action: remove inner vlan
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_pri_mask = -1;  //don't match pri/cfi at present
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_cfi_mask = -1;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_vid_mask = 0;
            //set is_vlan and vlan tag match/classification
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.is_vlan ++;
            VLAN_ID_SPLIT( rx_ifinfo->inner_vid, p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_pri, p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_cfi, p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_vid );
        }
       else
        {
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_pri = 0;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_cfi = 0;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_vid = 0;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_pri_mask = -1;  //don't match pri/cfi at present
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_cfi_mask = -1;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.in_vlan_vid_mask = -1;
        }
       
        if( tx_ifinfo->flags & NETIF_VLAN_OUTER )
        {
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.action_out_vlan_insert =1;
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_out_vlan_tpid = ( tx_ifinfo->outer_vid >> 16 ) & 0xFFFF;
            VLAN_ID_SPLIT( tx_ifinfo->outer_vid, p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_out_vlan_pri, p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_out_vlan_cfi, p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_out_vlan_vid );
        }
        if( tx_ifinfo->flags & NETIF_VLAN_INNER )
        {
            p_multifield_info->cfg0.vlan_info.vlan_info_manual.action_in_vlan_insert = 1;
            VLAN_ID_SPLIT( tx_ifinfo->inner_vid, p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_in_vlan_pri, p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_in_vlan_cfi, p_multifield_info->cfg0.vlan_info.vlan_info_manual.new_in_vlan_vid );
        }       
    }

    //change relative queue id to real queue id in ADSL wan mode
    if ( (tx_ifinfo->flags & NETIF_PHY_ATM) )  //for auto and manual
    {
        int qid, old_qid;
        PPA_BUF fake_skb;
        
        old_qid = p_multifield_info->cfg0.queue_id;
        fake_skb.priority = p_multifield_info->cfg0.queue_id;
        qid = ppa_drv_get_netif_qid_with_pkt(&fake_skb, tx_ifinfo->vcc, 1);
        if ( qid >= 0 )
            p_multifield_info->cfg0.queue_id = qid;
        else
            p_multifield_info->cfg0.queue_id = tx_ifinfo->dslwan_qid;
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow: mapped queue id from %d to %d\n", old_qid, p_multifield_info->cfg0.queue_id);
    }

EXIT:
     if( rx_ifinfo ) ppa_netif_put(rx_ifinfo);
     if( tx_ifinfo ) ppa_netif_put(tx_ifinfo);

     return res;

}

int32_t ppa_add_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, int32_t *index, uint32_t flag)   //hook
{
    int32_t res;
    PPE_MULTIFILED_FLOW flow;

    ppa_memset( &flow, 0, sizeof(flow) );

    if( (res = ppa_auto_learn_multifield_flow(p_multifield_info ) ) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_auto_learn_multifield_flow fail\n");
        return res;
    }    

    flow.multifield_info = *p_multifield_info;
    flow.flag = flag;
    ppa_mf_flow_display(p_multifield_info);
    
    res = ppa_drv_add_multifield_entry( &flow, 0);
     if( res == PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_add_multifield_flow: suceed to add below flow to index %d:\n", flow.entry);
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_add_multifield_flow: fail to add below flow---- \n");
    }
    
    *p_multifield_info = flow.multifield_info;
    if(index ) *index = flow.entry;
    
    return res;
}

int32_t ppa_del_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag )   //hook
{
     int32_t res=PPA_FAILURE;
     PPE_MULTIFILED_FLOW flow;

    ppa_memset( &flow, 0, sizeof(flow) );
    if( (res = ppa_auto_learn_multifield_flow(p_multifield_info ) ) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_del_multifield_flow learning fail\n");
        return res;
    }  

    flow.multifield_info = *p_multifield_info;
    flow.flag = flag;
    ppa_mf_flow_display(p_multifield_info);

    res = ppa_drv_del_multifield_entry( &flow, 0);
     if( res == PPA_SUCCESS )
    {     
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_del_multifield_flow: suceed to add below flow to index %d:\n", flow.entry);
    }
    else
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_del_multifield_flow: fail to add below flow---- \n");
    }
       

    return res;
}

    PPA_MULTIFIELD_VLAN_INFO vlan_info; 

int32_t ppa_quick_del_multifield_flow( int32_t index, uint32_t flag)   //hook
{
    int32_t res=PPA_SUCCESS;
    PPE_MULTIFILED_FLOW flow;

    ppa_memset( &flow, 0, sizeof(flow) );
    
    if( index == - 1)  // delete all mfe flow
    {
        int32_t num = ppa_get_multifield_max_flow(0);
        int32_t i;

        for( i = 0; i < num; i++ )
        {
            flow.entry = i;
            flow.flag = flag;
            res = ppa_drv_del_multifield_entry_via_index(&flow, flag);
        }
    }
    else 
    {        
        flow.entry = index;
        flow.flag = flag;
        res = ppa_drv_del_multifield_entry_via_index(&flow, flag);
    }
    
    if( res == PPA_SUCCESS )
    {   
        if( index == -1 )
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_quick_del_multifield_flow: succeed to delete all flows\n");
        else
            ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_quick_del_multifield_flow: succeed to delete flow %d\n", index);
    }
    else
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_quick_del_multifield_flow: fail to delete flow %d\n", index);

    return res;
}

int32_t ppa_get_multifield_flow( int32_t index, PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag )   //hook
{
    int32_t res;
    PPE_MULTIFILED_FLOW flow;
    
    ppa_memset( &flow, 0, sizeof(flow) );
    flow.entry = index;
    flow.flag = flag;
    
    res = ppa_drv_get_multifield_entry( &flow, 0);

    if( p_multifield_info ) *p_multifield_info = flow.multifield_info;

    if( res == PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_get_multifield_flow: suceed to get flow information %d\n", index);
        ppa_mf_flow_display(p_multifield_info);
    }
    else
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_get_multifield_flow: fail to get flow information %d\n", index);

    return res;
}

int32_t ppa_ioctl_enable_multifield(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)   //hook
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int32_t res;

    if ( ppa_copy_from_user( &cmd_info->mf_ctrl_info, (void *)arg, sizeof(cmd_info->mf_ctrl_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_multifield_control( cmd_info->mf_ctrl_info.enable_multifield, cmd_info->mf_ctrl_info.flag);

    if( res == PPA_SUCCESS )
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_ioctl_enable_multifield: suceed to enable multiple field feature\n");
    else
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_ioctl_enable_multifield: fail to enable multiple field feature\n");

    return res;
}

int32_t ppa_ioctl_get_multifield_status(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    uint8_t  enable=0;

    if( ppa_get_multifield_status( &enable, 0 )  != PPA_SUCCESS)
        return PPA_FAILURE;

    if ( ppa_copy_to_user( (void *)&(((PPA_CMD_ENABLE_MULTIFIELD_INFO*)arg)->enable_multifield), &enable, sizeof(enable)) != 0 )
    {
        return PPA_FAILURE;
    }

    return PPA_SUCCESS;
}

/***********************Below is ioctl exported API ************************************/
int32_t ppa_ioctl_add_multifield_flow(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    if ( ppa_copy_from_user( &cmd_info->mf_flow_info, (void *)arg, sizeof(cmd_info->mf_flow_info)) != 0 )
        return PPA_FAILURE;

    ppa_add_multifield_flow( &cmd_info->mf_flow_info.flow, &cmd_info->mf_flow_info.index, cmd_info->mf_flow_info.flag );

    //return the index value back to user space
    if ( ppa_copy_to_user( (void *)arg, (void *)&cmd_info->mf_flow_info, sizeof(cmd_info->mf_flow_info)) != 0 )
    {
        return PPA_FAILURE;
    }


    return PPA_SUCCESS;
}

int32_t ppa_ioctl_get_multifield_max_entry(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    if ( ppa_copy_from_user( &cmd_info->count_info, (void *)arg, sizeof(cmd_info->count_info)) != 0 )
        return PPA_FAILURE;
    
    cmd_info->count_info.count = ppa_get_multifield_max_flow( cmd_info->count_info.flag );

    if ( ppa_copy_to_user( (void *)arg, (void *)&cmd_info->count_info, sizeof(cmd_info->count_info)) != 0 )
    {
        return PPA_FAILURE;
    }

    return PPA_SUCCESS;
}

int32_t ppa_ioctl_get_multifield_flow(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    int res;
    if ( ppa_copy_from_user( &cmd_info->mf_flow_info, (void *)arg, sizeof(cmd_info->mf_flow_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_get_multifield_flow( cmd_info->mf_flow_info.index, &cmd_info->mf_flow_info.flow, cmd_info->mf_flow_info.flag );
    ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_ioctl_get_multifield_flow with res: %x\n", res);
    
    if(  res == PPA_FAILURE )
    {
        cmd_info->mf_flow_info.flag = PPA_FAILURE;
        res = PPA_SUCCESS; //note: here return PPA_SUCESS with cmd_info->mf_flow_info.flag setting to PPA_FAILURE in order to avoid ioctl failure for not used mf flow 
    }
    else if( res == PPA_SUCCESS )
    {
        cmd_info->mf_flow_info.flag = PPA_SUCCESS;
        res = PPA_SUCCESS;
    }
    else
    {
        cmd_info->mf_flow_info.flag = PPA_FAILURE;        
        res = PPA_FAILURE;
    }
    
    if ( ppa_copy_to_user( (void *)arg, (void *)&cmd_info->mf_flow_info, sizeof(cmd_info->mf_flow_info)) != 0 )
    {
        return PPA_FAILURE;
    }

    return res;
}
int32_t ppa_ioctl_del_multifield_flow(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    if ( ppa_copy_from_user( &cmd_info->mf_flow_info, (void *)arg, sizeof(cmd_info->mf_flow_info)) != 0 )
        return PPA_FAILURE;

    if( ppa_del_multifield_flow( &cmd_info->mf_flow_info.flow, cmd_info->mf_flow_info.flag ) != PPA_SUCCESS )
   {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_del_multifield_flow\n");
        return PPA_FAILURE;
    }

    return PPA_SUCCESS;
}

int32_t ppa_ioctl_del_multifield_flow_via_index(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    if ( ppa_copy_from_user( &cmd_info->mf_flow_info, (void *)arg, sizeof(cmd_info->mf_flow_info)) != 0 )
        return PPA_FAILURE;

    if( ppa_quick_del_multifield_flow( cmd_info->mf_flow_info.index, cmd_info->mf_flow_info.flag ) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_MFE, "ppa_quick_del_multifield_flow fail\n");
        return PPA_FAILURE;
    }

    return PPA_SUCCESS;
}

EXPORT_SYMBOL(ppa_multifield_control);
EXPORT_SYMBOL(ppa_get_multifield_status);
EXPORT_SYMBOL(ppa_get_multifield_max_flow);
EXPORT_SYMBOL(ppa_add_multifield_flow);
EXPORT_SYMBOL(ppa_del_multifield_flow);
EXPORT_SYMBOL(ppa_quick_del_multifield_flow);
EXPORT_SYMBOL(ppa_ioctl_enable_multifield);
EXPORT_SYMBOL(ppa_ioctl_get_multifield_status);
EXPORT_SYMBOL(ppa_ioctl_get_multifield_max_entry);
EXPORT_SYMBOL(ppa_ioctl_get_multifield_flow);

EXPORT_SYMBOL(ppa_ioctl_del_multifield_flow);
EXPORT_SYMBOL(ppa_ioctl_del_multifield_flow_via_index);

#endif //end of CONFIG_LTQ_PPA_MFE

