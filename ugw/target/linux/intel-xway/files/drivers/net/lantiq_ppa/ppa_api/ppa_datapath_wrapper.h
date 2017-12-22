/******************************************************************************
**
** FILE NAME    : ppe_datapath_wrapper.h
** PROJECT      : PPA
** MODULES      : PPA Wrapper for Datapath Driver API
**
** DATE         : 27 Feb 2014
** AUTHOR       : Kamal Erdath
** DESCRIPTION  : PPA Wrapper for PPE Driver API
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 27 FEB 2014  Kamal Eradath     Initiate Version
*******************************************************************************/
#ifndef PPA_DATAPATH_WRAPPER_2014_02_27
#define PPA_DATAPATH_WRAPPER_2014_02_27


#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
int ppa_drv_directpath_register(PPA_SUBIF *subif, PPA_NETIF *netif, PPA_DIRECTPATH_CB *pDirectpathCb, int32_t *index,  uint32_t flags);
#endif
extern struct ppe_directpath_data *ppa_drv_g_ppe_directpath_data;
int ppa_drv_directpath_send(uint32_t if_id, struct sk_buff *skb, int32_t len, uint32_t flags);
int ppa_drv_directpath_rx_stop(uint32_t if_id, uint32_t flags);
int ppa_drv_directpath_rx_start(uint32_t if_id, uint32_t flags);
int ppa_drv_get_dslwan_qid_with_vcc(struct atm_vcc *vcc);
int ppa_drv_get_netif_qid_with_pkt(struct sk_buff *skb, void *arg, int is_atm_vcc);
int ppa_drv_ppe_clk_change(unsigned int arg, unsigned int flags);
int ppa_drv_ppe_pwm_change(unsigned int arg, unsigned int flags);   //  arg - parameter, flags - 1: clock gating on/off, 2: power gating on/off
extern uint32_t ppa_drv_dp_sb_addr_to_fpi_addr_convert(PPA_FPI_ADDR*a, uint32_t flag);

extern int32_t ppa_hook_set_lan_seperate_flag( uint32_t flag);
extern int32_t ppa_hook_get_lan_seperate_flag( uint32_t flag);
extern uint32_t ppa_hook_set_wan_seperate_flag( uint32_t flag);
extern uint32_t ppa_hook_get_wan_seperate_flag( uint32_t flag);

#endif //end of PPA_DATAPATH_WRAPPER_2014_02_27


