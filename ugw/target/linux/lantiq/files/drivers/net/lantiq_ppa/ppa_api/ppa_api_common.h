#ifndef __PPA_API_COMMON_H__20100203__1740__
#define __PPA_API_COMMON_H__20100203__1740__

/*******************************************************************************
**
** FILE NAME    : ppa_api_common.h
** PROJECT      : PPA
** MODULES      : PPA Common header file
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Common Header File
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

#define NO_DOXY                 1

#ifndef CONFIG_LTQ_PPA_DSLITE   //if not defined in kernel's .configure file, then use local's definition
#define CONFIG_LTQ_PPA_DSLITE            1
#endif

#ifndef RTP_SAMPLING_ENABLE     //if not defined in kernel's .configure file, then use local's definition
#define RTP_SAMPLING_ENABLE               1
#endif

#ifndef MIB_MODE_ENABLE     //if not defined in kernel's .configure file, then use local's definition
#define MIB_MODE_ENABLE               1
#endif

#ifndef CAP_WAP_CONFIG     //if not defined in kernel's .configure file, then use local's definition
#define CAP_WAP_CONFIG               1
#endif

#ifndef L2TP_CONFIG     //if not defined in kernel's .configure file, then use local's definition
#define L2TP_CONFIG               1
#endif

#ifndef MBR_CONFIG     //if not defined in kernel's .configure file, then use local's definition
#define MBR_CONFIG               1
#endif

#ifndef QOS_AL_CONFIG     //if not defined in kernel's .configure file, then use local's definition
#define QOS_AL_CONFIG               1
#endif

#ifndef CONFIG_LTQ_PPA_MFE      //if not defined in kernel's .configure file, then use local's definition
#define CONFIG_LTQ_PPA_MFE               0
#endif

#ifndef VLAN_VAP_QOS     //if not defined in kernel's .configure file, then use local's definition
#define VLAN_VAP_QOS               0
#endif

#ifndef WMM_QOS_CONFIG     //if not defined in kernel's .configure file, then use local's definition
#define WMM_QOS_CONFIG               1
#endif
 /*force dynamic ppe driver's module parameter */
#define PPA_DP_DBG_PARAM_ENABLE  1   //for PPA automation purpose. for non-linux os porting, just disable it

#define CONFIG_LTQ_PPA_IF_MIB 1   //Flag to enable/disable PPA software interface based mib counter
#define SESSION_STATISTIC_DEBUG 1 //flag to enable session management statistics support


#if PPA_DP_DBG_PARAM_ENABLE
    extern int ppa_drv_dp_dbg_param_enable;
    extern int  ppa_drv_dp_dbg_param_ethwan;
    extern int ppa_drv_dp_dbg_param_wanitf;
    extern int ppa_drv_dp_dbg_param_ipv6_acc_en;
    extern int ppa_drv_dp_dbg_param_wanqos_en;
#endif // end of PPA_DP_DBG_PARAM_ENABLE

#endif

