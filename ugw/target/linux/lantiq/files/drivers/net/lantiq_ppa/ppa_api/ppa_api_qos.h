#ifndef __PPA_API_MFE_H__20100427_1703
#define __PPA_API_MFE_H__20100427_1703
/*******************************************************************************
**
** FILE NAME    : ppa_api_qos.h
** PROJECT      : PPA
** MODULES      : PPA API ( PPA QOS  APIs)
**
** DATE         : 27 April 2010
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA QOS APIs 
**                File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date                $Author         $Comment
** 16 Dec 2009  Shao Guohua             Initiate Version
*******************************************************************************/

/*! \file ppa_api_qos.h
    \brief This file contains es.
           provide PPA power management API.
*/

/** \addtogroup  PPA_API_QOS */
/*@{*/



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
//typedef int (*PPA_QOS_CLASS2PRIO_CB)(int32_t port_id, struct net_device *netif, uint8_t *class2prio);
#ifdef CONFIG_LTQ_PPA_QOS
extern int32_t ppa_ioctl_get_qos_status(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_get_qos_qnum( uint32_t portid, uint32_t flag);
extern int32_t ppa_ioctl_get_qos_qnum(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_get_qos_mib( uint32_t portid, uint32_t queueid, PPA_QOS_MIB *mib, uint32_t flag);
extern int32_t ppa_ioctl_get_qos_mib(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);

#ifdef CONFIG_LTQ_PPA_QOS_WFQ
extern int32_t ppa_set_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t weight_level, uint32_t flag );
extern int32_t ppa_get_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t *weight_level, uint32_t flag);
extern int32_t ppa_reset_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t flag);
extern int32_t ppa_set_ctrl_qos_wfq(uint32_t portid,  uint32_t f_enable, uint32_t flag);
extern int32_t ppa_get_ctrl_qos_wfq(uint32_t portid,  uint32_t *f_enable, uint32_t flag);

extern int32_t ppa_ioctl_add_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_modify_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_delete_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_qos_init_cfg(unsigned int cmd);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
extern int32_t ppa_ioctl_add_class_rule(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_mod_class_rule(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_del_class_rule(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_class_rule(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
#endif

extern int32_t ppa_ioctl_mod_subif_port_config(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_add_wmm_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_delete_wmm_qos_queue(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_set_ctrl_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_ctrl_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_set_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_reset_qos_wfq(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info); 
#endif //end of CONFIG_LTQ_PPA_QOS_WFQ

#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
extern int32_t ppa_set_ctrl_qos_rate(uint32_t portid,  uint32_t f_enable, uint32_t flag);
extern int32_t ppa_get_ctrl_qos_rate(uint32_t portid,  uint32_t *f_enable, uint32_t flag);
#if defined(MBR_CONFIG) && MBR_CONFIG
extern int32_t ppa_set_qos_rate( char *ifname, uint32_t portid, uint32_t queueid, int32_t shaperid, uint32_t rate, uint32_t burst, uint32_t flag, int32_t hal_id );
extern int32_t ppa_get_qos_rate( uint32_t portid, uint32_t queueid, int32_t* shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag);
#else
extern int32_t ppa_set_qos_rate( char *ifname, uint32_t portid, uint32_t queueid, uint32_t rate, uint32_t burst, uint32_t flag, int32_t hal_id );
extern int32_t ppa_get_qos_rate( uint32_t portid, uint32_t queueid, uint32_t *rate, uint32_t *burst, uint32_t flag);
#endif
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
extern int32_t ppa_reset_qos_rate( char *ifname, uint32_t portid, int32_t queueid, int32_t shaperid, uint32_t flag, int32_t hal_id);
#else
extern int32_t ppa_reset_qos_rate( uint32_t portid, int32_t queueid, uint32_t flag);
#endif
#if defined(MBR_CONFIG) && MBR_CONFIG
extern int32_t ppa_set_qos_shaper( int32_t shaperid, uint32_t rate, uint32_t burst,PPA_QOS_ADD_SHAPER_CFG *, uint32_t flags, int32_t hal_id );
extern int32_t ppa_get_qos_shaper( int32_t shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag);
extern int32_t ppa_ioctl_set_qos_shaper(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_qos_shaper(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
#endif
extern int32_t ppa_ioctl_set_ctrl_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_ctrl_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_set_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_reset_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_qos_rate(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
#endif //END OF CONFIG_LTQ_PPA_QOS_RATE_SHAPING

#endif //END OF CONFIG_LTQ_PPA_QOS


#endif  //end of __PPA_API_MFE_H__20100427_1703

