#ifndef __PPA_API_MFE_H__20100311_1703
#define __PPA_API_MFE_H__20100311_1703
/*******************************************************************************
**
** FILE NAME    : ppa_api_mfe.h
** PROJECT      : PPA
** MODULES      : PPA API ( Multiple Field Based Classification and VLAN Assignment  APIs)
**
** DATE         : 11 March 2010
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack Multiple Field Based Classification and VLAN Assignment 
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

/*! \file ppa_api_mfe.h
    \brief This file contains es.
           provide PPA power management API.
*/

/** \addtogroup  PPA_API_MULTIPLE_FIELD_EDITING */
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
extern int32_t ppa_multifield_control(uint8_t enable, uint32_t flag) ;
extern int32_t ppa_get_multifield_status(uint8_t *enable, uint32_t flag);
extern int32_t ppa_get_multifield_max_flow(uint32_t flag);
extern int32_t ppa_add_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, int32_t *index, uint32_t flag);   //hook
extern int32_t ppa_del_multifield_flow( PPA_MULTIFIELD_FLOW_INFO *p_multifield_info, uint32_t flag );   //hook
extern int32_t ppa_quick_del_multifield_flow( int32_t index, uint32_t flag) ;  //hook
extern int32_t ppa_ioctl_enable_multifield(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_multifield_status(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_multifield_max_entry(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_add_multifield_flow(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_get_multifield_flow(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_del_multifield_flow(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
extern int32_t ppa_ioctl_del_multifield_flow_via_index(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
 /* @} */


#endif  // __PPA_API_MFE_H__20100311_1703
