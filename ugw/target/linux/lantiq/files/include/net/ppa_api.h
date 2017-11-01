#ifndef __PPA_API_H__20081031_1913__
#define __PPA_API_H__20081031_1913__


/*******************************************************************************
**
** FILE NAME    : ppa_api.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 31 OCT 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Header File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                	$Comment
** 31 OCT 2008  Xu Liang               	Initiate Version
** 10 DEC 2012  Manamohan Shetty   	Added the support for RTP,MIB mode and CAPWAP 
** 08 APR 2014  Kamal Eradath		GRX500 Adaptations
*******************************************************************************/
/*! \file ppa_api.h
    \brief This file contains es.
           provide PPA API.
*/

/** \defgroup PPA_API PPA Kernel Hook and Userspace Function API
    \brief PPA is a loadable network module. Hence, it exports its API though function pointer hooks. Callers need to check that hooks are non-NULL before invoking them. The hooks are initialized when the PPA is initialized. Certain API which are control / configuration related are also exposed to user space applications through the ioctl API. The PPA Kernel and Userspace API are discussed in the following sections:
*/
/* @{*/
/** \defgroup PPA_IOCTL PPA Userspace API
    \brief  The subset of PPA API which is exposed to userspace for control and configuration of the PPA is invoked through 
            an ioctls()/system call interface as described in this section. 
            The API is defined in the following two source files:
            - ppa_api.h: Header file for PPA API
            - ppa_api.c: C Implementation file for PPA API
*/

/** \defgroup PPA_HOOK_API PPA Hook API
    \brief  PPA is a loadable network module. Hence, it exports its API though function pointer hooks.  Callers need to check that hooks are non-NULL before invoking them. The hooks are initialized  when the PPA is initialized. 
            - ppa_hook.h: Header file for PPA API
            - ppa_hook.c: C Implementation file for PPA API
*/

/** \defgroup PPA_PWM_API PPA Power Management API
    \brief  PPA Power Management  API provide PPA Power Management and IOCTL API
            The API is defined in the following two source files
            - ppa_api_pwm.h: Header file for PPA API
            - ppa_api_pwm.c: C Implementation file for PPA Power management API
            - ppa_api_pwm_logic.c: C impelementation file for Powr management Logic and interface with PPE driver
*/
 /** \defgroup PPA_API_DIRECTPATH PPA Direct Path API
    \brief  This section describes the PPA DirectPath API that allows a driver on a CPU to bypass the protocol stack and send and receive packets directly from the PPA acceleration function. For a 2-CPU system, this API is used to communicate with devices whose drivers are running on the 2nd CPU (or Core 1) - usually Core 1 is not running any protocol stack, and all protocol stack intelligence is on Core 0. This API is not yet implemented for PPE D4 or A4 firmware. It is provided as advance information on the DirectPath interfaces.The PPA DirectPath aims to accelerate packet processing by reducing CPU load  when the protocol stack processes the packet. It allows a CPU-bound driver to directly talk to the PPA and to the PPE engine bypassing the stack path and providing a short-cut. 
            - ppa_api_directpath.h: Header file for PPA API 
            - ppa_api_directpath.c: C Implementation file for PPA API
*/

/** \defgroup PPA_ADAPTATION_LAYER PPA Stack Adaptation API
    \brief  PPA module aims for OS and Protocol stack independence, and the 
            core PPA logic does not access any OS or Protocol stack implementation
            specific structures directly. The PPA Protocol Stack Adaptation layer 
            provides API that allows for straight-forward and structured OS / protocol
            stack porting of the PPA just by porting the Adaptation Layer (AL) API.
            The AL API is defined in the following two source files
            - ppa_stack_al.h: Header file for AL layer
            - ppa_stack_al.c: C Implementation file for AL API
*/
/* @}*/



#include <net/ppa_api_common.h>
#include <net/ppa_stack_al.h>
#ifdef __KERNEL__
  #include <net/ppa_api_directpath.h>
#endif

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#ifdef __KERNEL__
#include <xway/switch-api/gsw_types.h>
#include <xway/switch-api/lantiq_gsw_flow.h>
#else
#include <net/gsw_types.h>
#include <net/lantiq_gsw_flow.h>
#endif
#endif


/*
 * ####################################
 *              Definition
 * ####################################
 */
/*!
    \brief PPA_MAX_IFS_NUM
*/
//TBD: Kamal need to find a way to differentiate between legacy platforms and xrx500 platform 
#ifndef CONFIG_LTQ_PPA_GRX500
#define PPA_MAX_IFS_NUM                         10  /*!< Maximum interface number supported */
#else
#define PPA_MAX_IFS_NUM                         16 /*!< Maximum interface number supported */
#define MAX_SUBIF				16 /*!< Maximum sub interface supported */
#endif


/*!
    \brief PPA_MAX_MC_IFS_NUM
*/
//TBD: Kamal need to find a way to differentiate between legacy platforms and xrx500 platform 
#ifndef CONFIG_LTQ_PPA_GRX500
#define PPA_MAX_MC_IFS_NUM                      8   /*!< Maximum number of Multicast supporting interfaces */
#else
#define PPA_MAX_MC_IFS_NUM                      16   /*!< Maximum number of Multicast supporting interfaces */
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
/*!
    \brief PPA_MAX_CAPWAP_IFS_NUM
*/
#define PPA_MAX_CAPWAP_IFS_NUM                  8   /*!< Maximum number of CAPWAP supporting interfaces */

#define MAX_CAPWAP_ENTRIES                      5 
#define DEFAULT_TOS                             0
#define DEFAULT_TTL                             1
#define DEFAULT_MAX_FRG_SIZE                    1518

#define PPA_CAPWAP_NOT_ADDED                -1
#define PPA_CAPWAP_EXISTS                   0 
#define PPA_CAPWAP_ADDED                    1 
#endif

/*!
    \brief PPA_MAX_VLAN_FILTER
*/
#define PPA_MAX_VLAN_FILTER                     32  /*!< Maximum number of VLAN fitler */ 


/*!
    \brief PPA_IOC_MAGIC
*/ 
#define PPA_IOC_MAGIC                           ((uint32_t)'p') /*!< Magic number to differentiate PPA ioctl commands */

/*!
    \brief PPA_SUCCESS
*/
#define PPA_SUCCESS                             0   /*!< Operation was successful. */

/*!
    \brief PPA_FAILURE
*/
#define PPA_FAILURE                             -1  /*!< Operation failed */

/*!
    \brief PPA_EPERM
*/
#define PPA_EPERM                               -2  /*!<   not permitted */

/*!
    \brief PPA_EIO
*/
#define PPA_EIO                                 -5  /*!<   I/O/Hardware/Firmware error */ 

/*!
    \brief PPA_EAGAIN
*/
#define PPA_EAGAIN                              -11 /*!<   try again later */

/*!
    \brief PPA_ENOMEM
*/
#define PPA_ENOMEM                              -12 /*!<   out of memory */

/*!
    \brief PPA_EACCESS
*/
#define PPA_EACCESS                             PPA_EPERM 

/*!
    \brief PPA_EFAULT
*/
#define PPA_EFAULT                              -14 /*!<   bad address */

/*!
    \brief PPA_EBUSY
*/
#define PPA_EBUSY                               -16 /*!<   busy */

/*!
    \brief PPA_EINVAL
*/
#define PPA_EINVAL                              -22 /*!<   invalid argument */

/*!
    \brief PPA_ENOTAVAIL
*/
#define PPA_ENOTAVAIL                           -97

/*!
    \brief PPA_ENOTPOSSIBLE
*/
#define PPA_ENOTPOSSIBLE                        -98

/*!
    \brief PPA_ENOTIMPL
*/
#define PPA_ENOTIMPL                            -99 /*!<   not implemented  */

/*!
    \brief PPA_INDEX_OVERFLOW
*/
#define PPA_INDEX_OVERFLOW                      -100 


/*!
    \brief PPA_ENABLED
*/
#define PPA_ENABLED                             1   /*!< Status enabled / Device was enabled  */ 

/*!
    \brief PPA_DISABLED
*/
#define PPA_DISABLED                            0   /*!< Status disabled / Device was disabled. */



/*!
    \brief PPA_MAX_PORT_NUM
*/
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
#define PPA_MAX_PORT_NUM                            32   /*!< Maximum PPE FW Port number supported in PPA layer. */
#else
#define PPA_MAX_PORT_NUM                            16   /*!< Maximum PPE FW Port number supported in PPA layer. */
#endif 

/*!
    \brief PPA_MAX_QOS_QUEUE_NUM
*/
#if defined(CONFIG_PPA_PUMA7) && CONFIG_PPA_PUMA7
#define PPA_MAX_QOS_QUEUE_NUM                       16   /*!< Maximum PPE FW QoS Queue number supported in PPA layer for one port. */
#else
#define PPA_MAX_QOS_QUEUE_NUM                       16   /*!< Maximum PPE FW QoS Queue number supported in PPA layer for one port. */
#endif

/*
 *  flags
 */
 
/*!
    \brief PPA_F_BEFORE_NAT_TRANSFORM
    \note PPA Routing Session add hook is called before NAT transform has taken place. \n
    In Linux OS, this NSFORM corresponds to the netfilter PREROUTING hook 
*/   
#define PPA_F_BEFORE_NAT_TRANSFORM              0x00000001 

/*!
    \brief PPA_F_ACCEL_MODE
    \note notify PPA to enable or disable acceleration for one routing session. It is only for Hook/ioctl, not for PPE FW usage
*/   
#define PPA_F_ACCEL_MODE              0x00000002

/*!
    \brief PPA_F_SESSION_ORG_DIR 
    \note Packet in original direction of session i.e. the direction in which the session was established  
*/
#define PPA_F_SESSION_ORG_DIR                   0x00000010

/*!
    \brief PPA_F_SESSION_REPLY_DIR
    \note Packet in reply direction of session i.e. opposite to the direction in which session was initiated.
*/
#define PPA_F_SESSION_REPLY_DIR                 0x00000020

/*!
    \brief PPA_F_SESSION_BIDIRECTIONAL
    \note For PPA Session add, add a bidirectional session, else unidirection session is assumed.
*/
#define PPA_F_SESSION_BIDIRECTIONAL             (PPA_F_SESSION_ORG_DIR | PPA_F_SESSION_REPLY_DIR)

/*!
    \brief PPA_F_BRIDGED_SESSION
    \note Denotes that the PPA session is bridged 
*/
#define PPA_F_BRIDGED_SESSION                   0x00000100

/*!
    \brief PPA_F_SESSION_LOCAL_IN
    \note Denotes that the PPA session is local in 
*/
#define PPA_F_SESSION_LOCAL_IN                  0x00000200

/*!
    \brief PPA_F_SESSION_LOCAL_OUT
    \note Denotes that the PPA session is local out 
*/
#define PPA_F_SESSION_LOCAL_OUT                 0x00000400

/*!
    \brief PPA_F_SESSION_NEW_DSCP
    \note Denotes that the PPA session has DSCP remarking enabled
*/
#define PPA_F_SESSION_NEW_DSCP                  0x00001000

/*!
    \brief PPA_F_SESSION_VLAN
    \note Denotes that the PPA session has VLAN tagging enabled.
*/
#define PPA_F_SESSION_VLAN                      0x00002000

/*!
    \brief PPA_F_MTU
    \note Denotes that the PPA session has a MTU limit specified
*/
#define PPA_F_MTU                               0x00004000

/*!
    \brief PPA_F_SESSION_OUT_VLAN
    \note Denotes that the PPA session has Outer VLAN tagging enable 
*/
#define PPA_F_SESSION_OUT_VLAN                  0x00008000

/*!
    \brief PPA_F_BRIDGE_LOCAL
    \note  Denotes that the PPA bridge session is for a flow terminated at the CPE (i.e. not bridged out). Such an entry will not be accelerated
*/
#define PPA_F_BRIDGE_LOCAL                      0x00010000

/*!
    \brief PPA_F_PPPOE
    \note  Denotes that the PPA session has PPPOE header
*/
#define PPA_F_PPPOE                             0x00020000


/*!
    \brief PPA_F_LAN_IF
    \note Indicates that the interface is a LAN interface
*/
#define PPA_F_LAN_IF                            0x01000000

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
/*!
    \brief PPA_F_LAN_IPSEC
    \note Indicates that the interface is a LAN interface
*/
#define PPA_F_LAN_IPSEC                         0x08000000

#endif
/*!
    \brief PPA_F_STATIC_ENTRY
    \note Indicates that it is a static entry
*/
#define PPA_F_STATIC_ENTRY                      0x20000000

/*!
    \brief PPA_F_DROP_PACKET
    \note Denotes that the PPA session has a drop action specified. In other words, this acts as a fast path \n
          packet filter drop action
*/
#define PPA_F_DROP_PACKET                       0x40000000

/*!
    \brief PPA_F_BRIDGE_ACCEL_MODE
    \note Flag denoting that the PPA should accelerate bridging sessions. Reserved currently
*/
#define PPA_F_BRIDGE_ACCEL_MODE                 0x80000000

/*
 *  interface flags
 */

/*!
    \brief PPA_SESSION_NOT_ADDED
*/
#define PPA_SESSION_NOT_ADDED               -1 /*!< PPA Session Add failed. This can happen either because the Session is not yet ready for addition or \n
                                                       that PPA cannot accelerate the session because the packet is looped back */

/*!
    \brief PPA_SESSION_NOT_DELETED
*/
#define PPA_SESSION_NOT_DELETED               -1 /*!< PPA Session Add failed. This can happen either because the Session is not yet ready for addition or \n
                                                       that PPA cannot accelerate the session because the packet is looped back */

/*!
    \brief PPA_SESSION_ADDED
*/
#define PPA_SESSION_ADDED                   0  /*!< Indicates PPA was able to successfully add the session */

/*!
    \brief PPA_SESSION_DELETED
*/
#define PPA_SESSION_DELETED                   0  /*!< Indicates PPA was able to successfully add the session */



/*!
    \brief PPA_SESSION_EXISTS
*/
#define PPA_SESSION_EXISTS                  1  /*!< Indicates PPA already has the session added. This is also a success indication  */

/*!
    \brief PPA_MC_SESSION_VIOLATION
*/
#define PPA_MC_SESSION_VIOLATION            -2 /*!< PPA MC Session lookup failed. This can happen when you search IGMP V2 entry but hit one IGMPv3 entry with same \n
                                                            Multicast Group IP or vise versa*/
/*!
    \brief PPA_SESSION_FILTED
*/
#define PPA_SESSION_FILTED                 -1  /*!< Indicates session is  not qualified to be accelerated yet */

/*!
    \brief PPA_SESSION_NOT_FILTED
*/

#define PPA_SESSION_NOT_FILTED              0  /*!< Indicates session can be further processed */

/*
 *  ppa_inactivity_status return value
 */
/*!
    \brief PPA_HIT
    \note PPA Session is active i.e. was hit with packets within the configured inactivity time inter
*/
 #define PPA_HIT                             0
 
/*!
    \brief PPA_TIMEOUT
    \note PPA Session is inactive and hence has timed out
*/
#define PPA_TIMEOUT                         1

/*!
    \brief PPA_F_VLAN_FILTER_IFNAME
*/
#define PPA_F_VLAN_FILTER_IFNAME                0 /*!< Port based VLAN */

/*!
    \brief PPA_F_VLAN_FILTER_IP_SRC
*/
#define PPA_F_VLAN_FILTER_IP_SRC                1 /*!< SRC IP based VLAN */

/*!
    \brief PPA_F_VLAN_FILTER_ETH_PROTO
*/
#define PPA_F_VLAN_FILTER_ETH_PROTO             2 /*!< Ethernet Type based VLAN */

/*!
    \brief PPA_F_VLAN_FILTER_VLAN_TAG
*/
#define PPA_F_VLAN_FILTER_VLAN_TAG              3 /*!< Vlan tag based VLAN */

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
/*!
    \brief PPA_F_NETIF_PORT_MIB
*/
#define PPA_F_NETIF_PORT_MIB                    0x00000001 /*!< PPE Port MIB stats */

/*!
    \brief PPA_F_NETIF_HW_ACCEL
*/
#define PPA_F_NETIF_HW_ACCEL                    0x00000002 /*!< HW accelerated session stats */

/*!
    \brief PPA_F_NETIF_SW_ACCEL
*/
#define PPA_F_NETIF_SW_ACCEL                    0x00000004 /*!< SW accelerated session stats */
#endif

/*!
    \brief PPA_INVALID_QID
*/
#define PPA_INVALID_QID                         0xFFFF  /*!< Invalid VLAN ID. Note, it is used only in IOCTL */

/*!   
    \brief PPA_VLAN_TAG_MASK 
*/
#define PPA_VLAN_TAG_MASK                 0xFFFF1FFF  /*!< VLAN MASK to remove VLAN priority*/

/*!
    \brief MAX_HOOK_NAME_LEN
*/
#define MAX_HOOK_NAME_LEN                         71  /*!< maximum hook name length */


/*!
    \brief PPA_PORT_MODE_ETH
*/
#define PPA_PORT_MODE_ETH        1  /*!< Ethernet Port */
/*!
    \brief PPA_PORT_MODE_DSL
*/
#define PPA_PORT_MODE_DSL        2  /*!< DSL Port */

/*!
    \brief PPA_PORT_MODE_EXT
*/
#define PPA_PORT_MODE_EXT        3  /*!< Extension Port, like USB/WLAN */

/*!
    \brief PPA_PORT_MODE_CPU
*/
#define PPA_PORT_MODE_CPU        4  /*!< CPU */

/*!
  \brief MAX_Q_NAME_LEN */
#define MAX_Q_NAME_LEN 128
/*!
  \brief MAX_TC_NUM */
#define MAX_TC_NUM 8

/*!
  \brief MAX_PRIO_NUM */
#define MAX_PRIO_NUM 8

/*!
  \brief PPA_QOS_OP_F_ADD */
#define PPA_QOS_OP_F_ADD 			0x00000001

/*!
  \brief PPA_QOS_OP_F_ADD */
#define PPA_QOS_OP_F_MODIFY 			0x00000002

/*!
  \brief PPA_QOS_OP_F_DELETE */
#define PPA_QOS_OP_F_DELETE 			0x00000004

/*!
  \brief PPA_QOS_OP_F_INGRESS */
#define PPA_QOS_OP_F_INGRESS 			0x00000080

/*!
  \brief PPA_QOS_Q_F_INGRESS */
#define PPA_QOS_Q_F_INGRESS 			0x00000001

/*!
  \brief PPA_QOS_Q_F_DEFAULT */
#define PPA_QOS_Q_F_DEFAULT 			0x00000020

/*!
  \brief PPA_QOS_Q_F_WLANDP */
#define PPA_QOS_Q_F_WLANDP 			0x00000040

/*!
  \brief PPA_QOS_Q_F_INGGRP1 */
#define PPA_QOS_Q_F_INGGRP1 			0x00000100

/*!
  \brief PPA_QOS_Q_F_INGGRP2 */
#define PPA_QOS_Q_F_INGGRP2 			0x00000200

/*!
  \brief PPA_QOS_Q_F_INGGRP3 */
#define PPA_QOS_Q_F_INGGRP3 			0x00000400

/*!
  \brief PPA_QOS_Q_F_INGGRP4 */
#define PPA_QOS_Q_F_INGGRP4 			0x00000800

/*!
  \brief PPA_QOS_Q_F_INGGRP5 */
#define PPA_QOS_Q_F_INGGRP5 			0x00001000

/*!
  \brief PPA_QOS_Q_F_INGGRP6 */
#define PPA_QOS_Q_F_INGGRP6 			0x00002000


#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
#define WMM_QOS_DEV_F_REG 			0x00000001

#define WMM_QOS_DEV_F_DREG 			0x00000002
#endif/* WMM_QOS_CONFIG */

#ifdef NO_DOXY

#define SESSION_LIST_HASH_SHIFT                 8
#define SESSION_LIST_HASH_BIT_LENGTH            9
#define SESSION_LIST_HASH_MASK                  ((1 << SESSION_LIST_HASH_BIT_LENGTH) - 1)
#define SESSION_LIST_HASH_TABLE_SIZE            (1 << (SESSION_LIST_HASH_BIT_LENGTH + 1))
#define SESSION_LIST_HASH_VALUE(x, is_reply)    (((((uint32_t)(x) >> SESSION_LIST_HASH_SHIFT) & SESSION_LIST_HASH_MASK) << 1) | ((is_reply) ? 1 : 0))

#define SESSION_LIST_MC_HASH_SHIFT              0
#define SESSION_LIST_MC_HASH_BIT_LENGTH         6
#define SESSION_LIST_MC_HASH_MASK               ((1 << SESSION_LIST_MC_HASH_BIT_LENGTH) - 1)
#define SESSION_LIST_MC_HASH_TABLE_SIZE         (1 << SESSION_LIST_MC_HASH_BIT_LENGTH)
#define SESSION_LIST_MC_HASH_VALUE(x)           (((uint32_t)(x) >> SESSION_LIST_MC_HASH_SHIFT) & SESSION_LIST_MC_HASH_MASK)

#define BRIDGING_SESSION_LIST_HASH_BIT_LENGTH   8
#define BRIDGING_SESSION_LIST_HASH_MASK         ((1 << BRIDGING_SESSION_LIST_HASH_BIT_LENGTH) - 1)
#define BRIDGING_SESSION_LIST_HASH_TABLE_SIZE   (1 << BRIDGING_SESSION_LIST_HASH_BIT_LENGTH)
#define PPA_BRIDGING_SESSION_LIST_HASH_VALUE(x)     ( ( ((uint32_t)((uint8_t *)(x))[4] << 8) | ((uint8_t *)(x))[5] ) & BRIDGING_SESSION_LIST_HASH_MASK )


#define VLAN_ID_SPLIT(full_id, pri, cfi, vid)   pri=( (full_id) >> 13 ) & 7; cfi=( (full_id) >>12) & 1; vid= (full_id) & 0xFFF
#define VLAN_ID_CONBINE(full_id, pri, cfi, vid)   full_id =( ( (uint16_t)(pri) & 7) << 13 )  | ( ( (uint16_t)( cfi) & 1) << 12  )  | ((uint16_t) (vid) & 0xFFF ) 

#define PPA_JIFFY_MILLSEC(x, hz) (x * 1000 /(hz))

#define WRAPROUND_32BITS  ((uint64_t)0xFFFFFFFF)
#define WRAPROUND_64BITS  ((uint64_t)0xFFFFFFFFFFFFFFFF)
#if !(defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500)
#define WRAPROUND_SESSION_MIB  ((uint64_t)0x1FFFFFE0) /*note, 0xFFFFFF * 0x20. In PPE FW, 1 means 32 bytes, ie 0x20 this value will be different with GRX500 platform */
#else
#define WRAPROUND_SESSION_MIB WRAPROUND_32BITS
#endif

/*
 *  internal flag
 */

#define SESSION_INTERNAL_FLAG_BASE              0
#define SESSION_IS_REPLY                        0x00000001
//#define SESSION_BRIDGING_VCI_CHECK              0x00000002 //Note, it is not used now. It can be re-used for other purpose
#define SESSION_SWAP                            0x00000002
#define SESSION_IS_TCP                          0x00000004
#define SESSION_BYTE_STAMPING                   0x00000008 //for PPA session management purpose
#define SESSION_ADDED_IN_HW                     0x00000010
#define SESSION_NON_ACCE_MASK   ~SESSION_ADDED_IN_HW        //for ioctl only
#define SESSION_NOT_ACCEL_FOR_MGM               0x00000020  //Only for sesson management purpose to disable acceleration 
#define SESSION_CAN_NOT_ACCEL                   SESSION_NOT_ACCEL_FOR_MGM  //since Session Management is using this flag, we have to reserve it 

#define SESSION_STATIC                          0x00000040
#define SESSION_DROP                            0x00000080
#define SESSION_VALID_NAT_IP                    0x00000100
#define SESSION_VALID_NAT_PORT                  0x00000200
#define SESSION_VALID_NAT_SNAT                  0x00000400  //  src IP is replaced, otherwise dest IP is replaced
#define SESSION_NOT_ACCELABLE                   0x00000800   //Session cannot be accelerated for PPE FW Limitations like dont support hairpin NAT or ATM mode does not support two VLAN since PVC will use another VLAN
#define SESSION_VALID_VLAN_INS                  0x00001000
#define SESSION_VALID_VLAN_RM                   0x00002000
#define SESSION_VALID_OUT_VLAN_INS              0x00004000
#define SESSION_VALID_OUT_VLAN_RM               0x00008000
#define SESSION_VALID_PPPOE                     0x00010000
#define SESSION_VALID_NEW_SRC_MAC               0x00020000
#define SESSION_VALID_SRC_MAC                   SESSION_VALID_NEW_SRC_MAC
#define SESSION_VALID_MTU                       0x00040000
#define SESSION_VALID_NEW_DSCP                  0x00080000
#define SESSION_VALID_DSLWAN_QID                0x00100000
#define SESSION_TX_ITF_IPOA                     0x00200000
#define SESSION_TX_ITF_PPPOA                    0x00400000
#define SESSION_TX_ITF_IPOA_PPPOA_MASK          (SESSION_TX_ITF_IPOA | SESSION_TX_ITF_PPPOA)
#define SESSION_NOT_VALID_PHY_PORT		0x00800000
#if defined(L2TP_CONFIG) && L2TP_CONFIG
#define SESSION_VALID_PPPOL2TP                  0x01000000 // SESSION_SRC_MAC_DROP_EN is also same bitmap but will not be used for a routing session
#endif
#define SESSION_SRC_MAC_DROP_EN                 0x01000000
#define SESSION_TUNNEL_6RD                      0x02000000
#define SESSION_TUNNEL_DSLITE                   0x04000000
#define SESSION_TUNNEL_CAPWAP                   0x08000000
#define SESSION_LAN_ENTRY                       0x10000000
#define SESSION_WAN_ENTRY                       0x20000000
#define SESSION_IS_IPV6                         0x40000000
#define SESSION_ADDED_IN_SW                     0x80000000



/*Note, if a session cannot get hash index, it maybe ip output hook not work, or bridge mac address failed and so on*/
#define SESSION_FLAG2_HASH_INDEX_DONE          0x00000001
#define SESSION_FLAG2_ADD_HW_FAIL              0x00000002   //PPE hash full in this hash index, or IPV6 table full ,.
#define SESSION_FLAG2_HW_LOCK_FAIL             0x00000004		

#define SESSION_FLAG2_CPU_BOUND		       0x00000008   //session is locally terminating			

#define SESSION_FLAG2_BRIDGED_SESSION          0x00000010 
#define SESSION_FLAG2_GRE                      0x00000020

#define SESSION_FLAG2_LRO                      0x00400000
#define SESSION_FLAG2_VALID_IPSEC_OUTBOUND     0x00000040
#define SESSION_FLAG2_VALID_IPSEC_INBOUND      0x00000080 
#define SESSION_FLAG2_VALID_IPSEC_OUTBOUND_SA  0x00000100
#define SESSION_FLAG2_VALID_IPSEC_OUTBOUND_LAN 0x00000200
/* Other flags */
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
#define FLG_PPA_PROCESSED 			0x100	// this used to mark ecah packets which are processed by ppa datapath driver
#endif

#define SESSION_FLAG_TC_REMARK		       0x40000000 //Flag to sepcify  bit 30 in extmark which specifies packet classified by iptables when set to 1
#define SESSION_FLAG_DSCP_REMARK	       0x00000020 //Flag to enable DSCP remark in Stack when packet is not classified using PAE Flow Rule

#define SESSION_FLAG2_UPDATE_INFO_PROCESSED		0x10000000 //Flag to specify ppa_update_session_info is complete

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR)
#define  MAX_DATA_FLOW_ENGINES 3 // will be changed to runtime value
#else
#define  MAX_DATA_FLOW_ENGINES 2 // will be changed to runtime value
#endif
#define  MAX_SESSION_PRIORITY 3
#define  MAX_SESSION_TYPE 2
#endif
#ifdef __KERNEL__
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
typedef int (*PPA_QOS_CLASS2PRIO_CB)(int32_t , PPA_NETIF *, uint8_t *);
#endif
#endif

typedef enum {
  PPA_VARIABLE_EARY_DROP_FLAG=0,  
  PPA_VARIABLE_PPA_DIRECTPATH_IMQ_FLAG,
  PPA_LAN_SEPARATE_FLAG,
  PPA_WAN_SEPARATE_FLAG,

  PPA_VARIABLE_MAX,
} PPA_VARIABLE_ID;

struct variable_info
{
    char *name;
    uint32_t id;
    int32_t min;
    int32_t max;
};


#if defined(CONFIG_LTQ_PPA_MPE_IP97)



typedef enum {
    OUTBOUND=0,
    INBOUND 
} sa_direction;

typedef struct {
 int tunnel_id;
 uint32_t tx_pkt_count;
 uint32_t tx_byte_count;
 uint32_t rx_pkt_count;
 uint32_t rx_byte_count;

}IPSEC_TUNNEL_MIB_INFO;

#endif

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR)

typedef enum {
  PPE_HAL=0,
  PAE_HAL,
  MPE_HAL,
  TMU_HAL,
  LRO_HAL,
  DSL_HAL,
  SWAC_HAL,
  PUMA_HAL,
  MAX_HAL
} PPA_HAL_ID;

typedef enum {
  SESS_BRIDG=1,
  SESS_IPV4,
  SESS_IPV6,
  SESS_MC_DS,
  SESS_MC_DS_VAP,
  TUNNEL_6RD,
  TUNNEL_DSLITE,
  TUNNEL_L2TP_US,
  TUNNEL_L2TP_DS,
  TUNNEL_CAPWAP_US,
  TUNNEL_CAPWAP_DS,
  TUNNEL_ENCRYPT,
  TUNNEL_DECRYPT,
  TUNNEL_GRE_US,
  TUNNEL_GRE_DS,
  TUNNEL_IPSEC_US,
  TUNNEL_IPSEC_DS,
  TUNNEL_IPSEC_MIB,
  QOS_INIT,
  QOS_UNINIT,
  QOS_CLASSIFY,
  QOS_QUEUE,
  Q_SCH_WFQ,
  Q_SCH_SP,
  Q_DROP_DT,
  Q_DROP_RED,
  Q_DROP_WRED,
  Q_SHAPER,
  PORT_SHAPER,
  QOS_LAN_CLASSIFY,
  QOS_LAN_QUEUE,
  QOS_WMM_INIT,
  QOS_WMM_UNINIT,
  XDSL_PHY,
  MAX_CAPS
} PPA_API_CAPS;

#define PPA_VERSION_LEN 	64
#define MAX_TUNNEL_ENTRIES      16

#define FLAG_SESSION_HI_PRIO    0x0001
#define FLAG_SESSION_SWAPPED    0x0002
#define FLAG_SESSION_LOCK_FAIL  0x0004

// tunnel table datastructures
#endif //defined(CONFIG_LTQ_PPA_HAL_SELECTOR)


/* PPA default values */
#define PPA_IPV4_HDR_LEN    20
#define PPA_IPV6_HDR_LEN    40
#endif //NO_DOXY

/*
 * ####################################
 *              Data Type
 * ####################################
 */

/* -------------------------------------------------------------------------- */
/*                 Structure and Enumeration Type Defintions                  */
/* -------------------------------------------------------------------------- */

/** \addtogroup  PPA_HOOK_API */
/*@{*/

/*!
    \brief This is the data structure for PPA Interface Info specification.
*/
typedef struct {
    PPA_IFNAME *ifname;     /*!< Name of the stack interface */
    uint32_t    if_flags;   /*!< Flags for Interface. Valid values are below: PPA_F_LAN_IF and PPA_F_WAN_IF */
    uint32_t    port;       /*!< physical port id  for this Interface. Valid values are below: 0 ~  */
    uint32_t    force_wanitf_flag;   /*!< force_wanitf_flag is used for force change PPE FW's LAN/WAN interface */
    PPA_IFNAME  *ifname_lower;     /*!< Name of the manually configured its lower interface */
	uint8_t    hw_disable;  /*!< If this flag is set then only HW acceleration would be disabled for ifname (SW acceleration would still work for ifname) */
} PPA_IFINFO;

/*!
    \brief This is the data structure for PPA Packet header verification checks.
*/
typedef struct ppa_verify_checks {
    uint32_t    f_ip_verify             :1; /*!< Enable/Disable IP verification checks.  Valid values are PPA_ENABLED or PPA_DISABLED */
    uint32_t    f_tcp_udp_verify        :1; /*!< Enable/Disable TCP/UDP verification checks. Valid values are PPA_ENABLED or PPA_DISABLED */
    uint32_t    f_tcp_udp_err_drop      :1; /*!< Enable/Disable drop packet if TCP/UDP checksum is wrong. \n
                                                If packet is not dropped, then it is forwarded to the control CPU. \n
                                                Valid values are PPA_ENABLED or PPA_DISABLED */
    uint32_t    f_drop_on_no_hit        :1; /*!< Drop unicast packets on no hit, forward to MIPS/Control CPU otherwise (default). Valid values are PPA_ENABLED or PPA_DISABLED */
    uint32_t    f_mc_drop_on_no_hit     :1; /*!< Drop multicast on no hit, forward to MIPS/Control CPU otherwise (default). Valid values are PPA_ENABLED or PPA_DISABLED */
} PPA_VERIFY_CHECKS;

/*!
    \brief This is the data structure for PPA Initialization kernel hook function
*/
typedef struct {
    PPA_VERIFY_CHECKS   lan_rx_checks;      /*!<   LAN Ingress packet checks */
    PPA_VERIFY_CHECKS   wan_rx_checks;      /*!<   WAN Ingress packet checks */
    uint32_t    num_lanifs;                 /*!<   Number of LAN side interfaces */
    PPA_IFINFO *p_lanifs;                   /*!<   Pointer to array of LAN Interfaces. */
    uint32_t    num_wanifs;                 /*!<   Number of WAN side interfaces */
    PPA_IFINFO *p_wanifs;                   /*!<   Pointer to array of WAN Interfaces. */
    uint32_t    max_lan_source_entries;     /*!<   Maximum Number of session entries with LAN source */
    uint32_t    max_wan_source_entries;     /*!<   Maximum Number of session entries with WAN source */
    uint32_t    max_mc_entries;             /*!<   Maximum Number of multicast sessions */
    uint32_t    max_bridging_entries;       /*!<   Maximum Number of bridging entries */
    uint32_t    add_requires_min_hits;      /*!<   Minimum number of calls to ppa_add_session() before session would be added in h/w - calls from the same hook position in stack. Currently, set to 1 */
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    uint32_t	add_requires_lan_collisions;/*!<   Minimum number of LAN collision entries to be reserved in h/w*/
    uint32_t	add_requires_wan_collisions;/*!<   Minimum number of WAN collision entries to be reserved in h/w*/
#endif
} PPA_INIT_INFO;

/*!
    \brief This is the data structure for additional session related information for the PPA. It specifies on a per session basis
            attributes like VLAN tagging, DSCP remarking etc. This structure depends on the PPE acceleration firmware
            capabilities. New versions of PPA will only support the capabilities as in PPE A4/D4 firmware,
            The current PPA driver (for PPE A4/D4 firmware) supports 2-level of VLANs (or stacked VLANs). The outer VLAN is
            the one used for separating LAN and WAN traffic on a switch (for Ethernet WAN). Inner VLAN tag is application
            specific VLAN. In case, there is no outer VLAN tag required (for LAN/WAN separation on the switch), then this
            field is not specified.
*/
typedef struct {
    uint32_t    new_dscp            :6; /*!<   New DSCP code point value for the session.Valid values are 0-63. */
    uint32_t    dscp_remark         :1; /*!<   DSCP remarking needs to be carried out for the session.Valid values are:PPA_ENABLED and PPA_DISABLED */
    uint32_t    vlan_insert         :1; /*!<   If inner VLAN tag should be inserted into the frame at egress. Valid values are: PPA_ENABLED and PPA_DISABLED */
    uint32_t    vlan_remove         :1; /*!<  If inner VLAN untagging should be performed on the received frame. Untagging, if enabled, is \n
                                            carried out before any VLAN tag insert. Valid values are:PPA_ENABLED and PPA_DISABLED */
    uint32_t    out_vlan_insert     :1; /*!<   If outer VLAN tag should be inserted into the frame at egress. Valid values are: PPA_ENABLED and PPA_DISABLED */
    uint32_t    out_vlan_remove     :1; /*!<  If outer VLAN untagging should be performed on the received frame. Untagging, if enabled, is \n
                                            carried out before any VLAN tag insert. Valid values are:PPA_ENABLED and PPA_DISABLED */
    uint16_t    dslwan_qid_remark   :1; /*!<   if dslwan qid should be set. Valid values are: PPA_ENABLED and PPA_DISABLED */
    uint32_t    reserved1           :4; /*!<   reserved */
    uint32_t    vlan_prio           :3; /*!<   802.1p VLAN priority configuration. Valid values are 0-7. */
    uint32_t    vlan_cfi            :1; /*!<   lways set to 1 for Ethernet frames */
    uint32_t    vlan_id             :12;/*!<   VLAN Id to be used to tag the frame. Valid values are 0-4095. */
    uint16_t    mtu;                    /*!<   MTU of frames classified to this session */
    uint16_t    dslwan_qid;             /*!<   dslwan_qid. Valid values are 0 ~ 16 */
    uint16_t    pppoe_id;               /*!<   pppoe session id.  */
    uint32_t    session_flags;          /*!<  Session flags used to identify which fields in the PPA_SESSION_EXTRA structure are valid in \n
                                            a call to the PPA Session Modify API. \n
                                            Valid values are one or more of: \n
                                                PPA_F_SESSION_NEW_DSCP \n
                                                PPA_F_SESSION_VLAN \n
                                                PPA_F_SESSION_OUT_VLAN \n
                                                PPA_F_MTU \n
                                                PPA_F_PPPOE\n
                                         */
    uint32_t    out_vlan_tag;           /*!<   VLAN tag value including VLAN Id */


                                            
    uint16_t    accel_enable:1;        /*!<   to enable/disable acceleartion for one specified routing session. It will be used only in PPA API level, not HAL and PPE FW level */                                 
} PPA_SESSION_EXTRA;

/*!
    \brief This is the data structure which specifies an interface and its TTL value as applicable for multicast routing.
*/
typedef struct {
    PPA_IFNAME *ifname; /*!<   Pointer to interface name.  */
    uint8_t     ttl;    /*!<  Time to Live (TTL) value of interface which is used for multicast routing to decide if a packet can be routed onto that interface
                            Note, it is not used at present.
                         */
} IF_TTL_ENTRY;

/*!
    \brief This is the data structure for basic IPV4/IPV6 address
*/
typedef union {
        uint32_t ip;     /*!< ipv4 address */
        uint32_t ip6[4];   /*!< ipv6 address */
}IP_ADDR;

/*!
    \brief This is the data structure for complex IPV4/IPV6 address
*/
typedef struct {
    uint32_t f_ipv6; /*!< flag to specify the ipv4 version: 0---IPV4, 1 -- IPV6 */
    IP_ADDR ip;  /*!< multiple ip address format support */
} IP_ADDR_C;


/*!
    \brief This is the data structure for PPA Multicast Group membership. It specifies the interfaces which are members of
            the specified IP Multicast Group address. Please see the discussion on outer and inner VLAN tags in the
            section on PPA_SESSION_EXTRA data structure.
*/
typedef struct {
    IP_ADDR_C       ip_mc_group;    /*!<   Multicast IP address group */
    int8_t          num_ifs;        /*!<   Number of Interfaces which are member of this Multicast IP group address */
    IF_TTL_ENTRY    array_mem_ifs[PPA_MAX_MC_IFS_NUM];  /*!< Array of interface elements of maximum PPA_MAX_MC_IFS_NUM elements.
                                                         Actual number of entries is specified by num_ifs */
    uint8_t         if_mask;        /*!<   Mask of Interfaces corresponding to num_ifs interfaces specified in array_mem_ifs. For internaly use only. */
    PPA_IFNAME     *src_ifname;     /*!<   the source interface of specified multicast IP address group */
    uint32_t        vlan_insert     :1;     /*!<   If inner VLAN tag should be inserted into the frame at egress. Valid values are: PPA_ENABLED and PPA_DISABLED */
    uint32_t        vlan_remove     :1;     /*!<  If inner VLAN untagging should be performed on the received frame. Untagging, if enabled, is
                                                carried out before any VLAN tag insert. Valid values are:PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_vlan_insert :1;     /*!<   If outer VLAN tag should be inserted into the frame at egress. Valid values are: PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_vlan_remove :1;     /*!<  If outer VLAN untagging should be performed on the received frame. Untagging, if enabled, is
                                                carried out before any VLAN tag insert. Valid values are:PPA_ENABLED and PPA_DISABLED */
    uint32_t        dslwan_qid_remark:1;    /*!<   not use at present */
    uint32_t        reserved1       :3;     /*!<   valid in A4/A5 */
    uint32_t        vlan_prio       :3;     /*!<   802.1p VLAN priority configuration. Valid values are 0-7. */
    uint32_t        vlan_cfi        :1;     /*!<   Always set to 1 for Ethernet frames */
    uint32_t        vlan_id         :12;    /*!<   VLAN Id to be used to tag the frame. Valid values are 0-4095 */
    uint32_t        out_vlan_tag;           /*!<   Outer VLAN tag value including VLAN Id. */
    uint32_t        new_dscp_en     :1;     /*!<   If new dscp value should be set. Valid values are:PPA_ENABLED and PPA_DISABLED */
    uint32_t        res             :15;    /*!<   reserved */  
    uint32_t        new_dscp        :16;    /*!<   New DSCP code point value for the session.Valid values are 0-63. */
    uint16_t        dslwan_qid;             /*!<   not use at present */

    uint32_t        bridging_flag;          /*!<   0 - routing mode/igmp proxy, 1 - bridge mode/igmp snooping. */
    uint8_t         mac[PPA_ETH_ALEN];      /*!<  reserved for future */
    uint8_t         SSM_flag;     /*!< Set the flag if source specific forwarding is required default 0*/ 
    IP_ADDR_C       source_ip;    /*!<  source ip address */
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    uint8_t         RTP_flag;   /*!< rtp flag */
#endif
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint16_t	    group_id;		    /*!<   Multicast group identifier allocated by the multicast daemon */
    uint8_t         src_mac[PPA_ETH_ALEN];      /*!< source mac address for grx5xx. */  	
#endif
} PPA_MC_GROUP;

/*!
    \brief This data structure is an abstraction for unicast and multicast routing sessions.
             Pointer to any kind of PPA session
*/
typedef void PPA_U_SESSION;

/*!
    \brief This is the data structure for standard packet and byte statistics for an interface.
*/
typedef struct {
    uint32_t    tx_pkts;            /*!<   Number of transmitted packets through the interface */
    uint32_t    rx_pkts;            /*!<   Number of received packets through the interface */
    uint32_t    tx_discard_pkts;    /*!<   Number of packets discarded while transmitting through the interface. */
    uint32_t    tx_error_pkts;      /*!<   Number of transmit errors through the interface. */
    uint32_t    rx_discard_pkts;    /*!<   Number of received packets through the interface that were discarded */
    uint32_t    rx_error_pkts;      /*!<   Number of received errors through the interface. */
    uint32_t    tx_bytes;           /*!<   Number of transmit bytes through the interface */
    uint32_t    rx_bytes;           /*!<   Number of received bytes through the interface */
} PPA_IF_STATS;

/*!
    \brief This is the data structure for PPA accelerated statistics for an interface. Depending on the platform and
             acceleration capabilities, some of the statistics may not be available.
*/
typedef struct {
    uint32_t    fast_routed_tcp_pkts;       /*!< Fastpath routed TCP unicast packets Tx */
    uint32_t    fast_routed_udp_pkts;       /*!< Fastpath routed UDP unicast packets Tx */
    uint32_t    fast_routed_udp_mcast_pkts; /*!< Fastpath routed UDP multicast packets Tx */
    uint32_t    fast_drop_pkts;             /*!< Fastpath ingress pkts dropped */
    uint32_t    fast_drop_bytes;            /*!< Fastpath ingress bytes dropped */
    uint32_t    fast_ingress_cpu_pkts;      /*!< Fastpath ingress CPU pkts */
    uint32_t    fast_ingress_cpu_bytes;     /*!< Fastpath ingress CPU bytes */
    uint32_t    rx_pkt_errors;              /*!< Fastpath packet error */
    uint32_t    fast_bridged_ucast_pkts;    /*!< Fastpath bridged unicast pkts */
    uint32_t    fast_bridged_mcast_pkts;    /*!< Fastpath bridged multicast pkts */
    uint32_t    fast_bridged_bcast_pkts;    /*!< Fastpath bridged broadcast pkts */
    uint32_t    fast_bridged_bytes;         /*!< Fastpath bridged bytes */
} PPA_ACCEL_STATS;

/*!
    \brief This is the data structure for VLAN tag control on a per interface basis. It is currently supported only for bridging
    paths. For PPE A4 firmware, 2 levels of VLAN is configurable, while for older PPE D4 firmware, only inner VLAN
    tag is configurable. Please see discussion in section PPA_SESSION_EXTRA. Briefly, couter VLAN tag
    configuration is used for LAN and WAN isolation on the same external switch, while the other set of VLAN tag
    configuration is driven from application needs (i.e. not stripped off when the packet hits the wire).
*/
typedef struct {
    uint32_t        unmodified      :1; /*!< Indicates if there is no VLAN tag modification. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        insertion       :1; /*!< Indicates if there is a VLAN tag inserted. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        remove          :1; /*!< Indicates if there is a VLAN tag removed. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        replace         :1; /*!< Indicates if there is a VLAN tag replaced. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_unmodified  :1; /*!< Indicates if there is no outer VLAN tag modification. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_insertion   :1; /*!< Indicates if there is a outer VLAN tag inserted. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_remove      :1; /*!< Indicates if there is a outer VLAN tag removed. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_replace     :1; /*!< Indicates if there is  a outerVLAN tag replaced. Valid values are PPA_ENABLED and PPA_DISABLED */
} PPA_VLAN_TAG_CTRL;

/*!
    \brief This is the data structure for VLAN configuration control on a per interface basis. It is currently supported only for
    bridging paths.
*/
typedef struct {
    uint32_t        src_ip_based_vlan   :1; /*!< Indicates if Source IP address filter based VLAN tagging is enabled for this interface. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        eth_type_based_vlan :1; /*!< Indicates if Ethernet header type based VLAN tagging is enabled for this interface. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        vlanid_based_vlan   :1; /*!< Indicates if VLAN re-tagging is enabled based on existing VLAN Id of received frame. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        port_based_vlan     :1; /*!< Indicates if port based VLAN tagging is enabled for this interface. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        vlan_aware          :1; /*!< Indicates if bridge is VLAN aware and enforces VLAN based forwarding for this interface. Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        out_vlan_aware      :1; /*!< Indicates if bridge is outer VLAN aware and enforces VLAN based forwarding for this
                                              interface. If this field is not enabled, then outer VLAN processing is don't care.
                                              interface. If this field is not enabled, then outer VLAN processing is don't care.
                                              Valid values are PPA_ENABLED and PPA_DISABLED */
} PPA_VLAN_CFG;

/*!
    \brief Union of PPA VLAN filter criteria.
*/
typedef union 
{
        PPA_IFNAME     *ifname; /*!< Pointer to interface name on which VLAN filter match is to be performed. */
        IPADDR          ip_src; /*!< IP source address of ingress frame for VLAN filter matching. */
        uint32_t        eth_protocol;       /*!< Ethernet protocol as a match filter for VLAN filter matching */
        uint32_t        ingress_vlan_tag;   /*!< Ingress frame VLAN tag as match criteria for VLAN filter matching */
} match_criteria_vlan;

/*!
    \brief This data structure specifies the filter or match criteria for applying VLAN transforms based on rules. It is currently supported only for bridging paths.
*/
typedef struct {
    match_criteria_vlan    match_field;            /*!< Union of VLAN filter criteria */
    uint32_t        match_flags;    /*!< Indicates which VLAN filter criteria is specified in this VLAN match entry.
                                      Valid values are one of the following: \n
                                      PPA_F_VLAN_FILTER_IFNAME \n
                                      PPA_F_VLAN_FILTER_IP_SRC \n
                                      PPA_F_VLAN_FILTER_ETH_PROTO \n
                                      PPA_F_VLAN_FILTER_VLAN_TAG \n
                                     */
} PPA_VLAN_MATCH_FIELD;

/*!
    \brief This is the data structure for PPA VLAN configuration ioctl() on a per interface basis from userspace. It is currently
supported only for bridging paths.
*/
typedef struct {
    uint16_t        vlan_vci;   /*!< VLAN Information including VLAN Id, 802.1p and CFI bits. */
    uint16_t        qid;        /*!< queue index */
    uint32_t        out_vlan_id; /*!< out vlan id */
    uint32_t        inner_vlan_tag_ctrl;/*!< none(0)/remove(1)/insert(2)/replac(3), for vlan tag based only. */
    uint32_t        out_vlan_tag_ctrl;  /*!< none(0)/remove(1)/insert(2)/replac(3), for vlan tag based only. */
    uint16_t        num_ifs;    /*!< Number of interfaces in the array of PPA_IFINFO structures. */
    PPA_IFINFO     *vlan_if_membership; /*!< Pointer to array of interface info structures for each interface which is a member of this VLAN group. The number of entries is given by num_ifs. */
} PPA_VLAN_INFO;

/*!
    \brief This is the data structure for PPA VLAN filter configuration. It is currently supported only for bridging paths
*/
typedef struct {
    PPA_VLAN_MATCH_FIELD    match_field;    /*!< VLAN Match field information */
    PPA_VLAN_INFO           vlan_info;      /*!< VLAN Group and Membership Info */
} PPA_VLAN_FILTER_CONFIG;

/*!
    \brief This is the data structure for cout information, like lan interface count, LAN acceleration count and so on  
*/
typedef struct {
    uint32_t    count;  /*!< the number */
    uint32_t    flag;   /*!< the flag */
    uint32_t    stamp_flag;  /*!< the stamping flag */
    uint32_t    hash_index; /*!< to specify hash index */
} PPA_CMD_COUNT_INFO;

/*!
    \brief This is the data structure for get some structure size 
*/
typedef struct {
    uint32_t    rout_session_size;  /*!< the structure size of one routing session */
    uint32_t    mc_session_size;   /*!< the structure size of one multicast session */
    uint32_t   br_session_size;    /*!< the structure size of one bridge session */
    uint32_t   netif_size;         /*!< the structure size of one network interface information*/
} PPA_CMD_SIZE_INFO;
/*@}*/ /* PPA_HOOK_API */

/*
 *  ioctl command structures
 */

/** \addtogroup  PPA_IOCTL */
/*@{*/

/*!
    \brief This is the data structure for PPA Interface information used from the userspacef
*/
typedef struct {
    PPA_IFNAME  ifname[PPA_IF_NAME_SIZE];   /*!<  Name of the stack interface ( provide storage buffer )  */
    uint32_t    force_wanitf_flag;   /*!< force_wanitf_flag is used for force change PPE FW's LAN/WAN interface */
    uint64_t    acc_tx; /*!< tx tx mib ( bytes counter/packet counter) */
    uint64_t    acc_rx; /*!< rx rx mib ( bytes counter/packet counter) */
    uint32_t    if_flags;   /*!< Flags for Interface. Valid values are below: PPA_F_LAN_IF and PPA_F_WAN_IF */
    PPA_IFNAME  ifname_lower[PPA_IF_NAME_SIZE];   /*!<  Name of the manually configured its lower stack interface ( provide storage buffer )  */
	uint8_t    hw_disable;  /*!< If this flag is set then only HW acceleration would be disabled for ifname (SW acceleration would still work for ifname) */
} PPA_CMD_IFINFO;

/*!
    \brief This is the data structure for PPA Init used from the userspace
*/
typedef struct {
    PPA_VERIFY_CHECKS   lan_rx_checks;          /*!<  LAN Ingress checks */
    PPA_VERIFY_CHECKS   wan_rx_checks;          /*!<  WAN Ingress checks */
    uint32_t        num_lanifs;                 /*!<   Number of LAN side interfaces */
    PPA_CMD_IFINFO  p_lanifs[PPA_MAX_IFS_NUM];  /*!<  Array of LAN Interface Info structures (provides storage buffer). */
    uint32_t        num_wanifs;                 /*!<   Number of WAN side interfaces */
    PPA_CMD_IFINFO  p_wanifs[PPA_MAX_IFS_NUM];  /*!<  Array of WAN Interface Info structures (provides storage buffer). */
    uint32_t        max_lan_source_entries;     /*!<   Number of session entries with LAN source */
    uint32_t        max_wan_source_entries;     /*!<   Number of session entries with WAN source */
    uint32_t        max_mc_entries;             /*!<   Number of multicast sessions */
    uint32_t        max_bridging_entries;       /*!<   Number of bridging entries */
    uint32_t        add_requires_min_hits;      /*!<   Minimum number of calls to ppa_add before session would be added in h/w */
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    uint32_t        add_requires_lan_collisions;/*!<   Minimum number of LAN collision entries to be reserved in h/w*/
    uint32_t        add_requires_wan_collisions;/*!<   Minimum number of WAN collision entries to be reserved in h/w*/
#endif
    uint32_t        mtu; /*!<   specify PPA network internface's MTU size, default is 1500 */
    uint32_t        flags;  /*!< Flags for PPA Initialization. Currently this field is reserved. */
    
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    uint8_t             mib_mode;   /*!< mib mode: 0-Byte mode,1-Packet mode */
#endif

} PPA_CMD_INIT_INFO;

/*!
    \brief This is the data structure for PPA Acceleration Enable / Disable configuration
*/
typedef struct {
    uint32_t        lan_rx_ppa_enable;  /*!<  lan Interface specific flags. Current Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        wan_rx_ppa_enable;  /*!<  wan Interface specific flags. Current Valid values are PPA_ENABLED and PPA_DISABLED */
    uint32_t        flags;              /*!< Reserved currently */
} PPA_CMD_ENABLE_INFO;

/*!
    \brief This is the data structure for MAC table entry used in PPA ioctl interface
*/
typedef struct {
    IP_ADDR_C           mcast_addr; /*!< MC  address of the entry */
    IP_ADDR_C           source_ip;  /*!< source ip */
    uint8_t             SSM_flag;   /*!< ssm flag */
    PPA_SESSION_EXTRA   mc_extra;   /*!< Pointer to PPA Multicast session parameters like VLAN configuration, DSCP remarking */
    uint32_t            flags;      /*!< Flags for the PPA Multicast entry info structure. Reserved currently. */
    uint64_t            mips_bytes; /*!< bytes processed by the mips */
    uint64_t            hw_bytes;   /*!< bytes proccesed by hareware acceleration unit*/
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    uint8_t             RTP_flag;   /*!< rtp flag */
    uint32_t            rtp_pkt_cnt;  /*!< RTP packet mib */
    uint32_t            rtp_seq_num;  /*!< RTP sequence number */
#endif

} PPA_CMD_MC_ENTRY;


#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
/*!
    \brief This is the data structure for MIB mode configuration 
*/
typedef struct {
    uint8_t             mib_mode;   /*!< mib mode: 0-Byte mode,1-Packet mode */

} PPA_CMD_MIB_MODE_INFO;

#endif

/*!
    \brief This is the data structure for learned MAC address used in PPA ioctl interface
*/
typedef struct {
    uint8_t         mac_addr[PPA_ETH_ALEN];   /*!< MAC address learned */
    PPA_IFNAME      ifname[PPA_IF_NAME_SIZE]; /*!< The interface which learned the MAC address */
    PPA_IFNAME      brname[PPA_IF_NAME_SIZE]; /*!< The bridge which learned the MAC address */
    uint32_t        flags;    /*!< for future */
} PPA_CMD_MAC_ENTRY;

/*!
    \brief This is the data structure for PPA VLAN configuration ioctl() on a per interface basis from userspace. It is currently
    supported only for bridging paths.
*/
typedef struct
{
    PPA_IFNAME          if_name[PPA_IF_NAME_SIZE];  /*!< Pointer to interface name for which VLAN related configuration is specified. */
    PPA_VLAN_TAG_CTRL   vlan_tag_ctrl;  /*!< VLAN Tag Control structure for the interface */
    PPA_VLAN_CFG        vlan_cfg;       /*!< VLAN Configuration control structure for the interface */
    uint32_t            flags;          /*!< Flags field. Reserved currently and omitted in implementation. */
} PPA_CMD_BR_IF_VLAN_CONFIG;


/*!
  \brief Union for VLAN filter matching criteria. 
*/       
typedef union {
        PPA_IFNAME      ifname[PPA_IF_NAME_SIZE];  /*!<  Pointer to interface name on which VLAN filter match is to be performed. */
        IPADDR          ip_src;       /*!< IP source address of ingress frame for VLAN filter matching. */               
        uint32_t        eth_protocol; /*!< Ethernet protocol as a match filter for VLAN filter matching. */  
        uint32_t        ingress_vlan_tag; /*!< Ingress frame VLAN tag as match criteria for VLAN filter matching. */
} filter_criteria;          


/*!
  \brief This data structure specifies the filter or match criteria for applying VLAN transforms based on rules. It is currently supported only for bridging paths.
*/
typedef struct {
    filter_criteria     match_field;          /*!< Union for VLAN filter criteria. */       
    uint32_t        match_flags;      /*!< Indicates which VLAN filter criteria is specified in this VLAN match entry. \n
                                            Valid values are one of the following: \n
                                            - PPA_F_VLAN_FILTER_IFNAME \n
                                            - PPA_F_VLAN_FILTER_IP_SRC \n
                                            - PPA_F_VLAN_FILTER_ETH_PROTO \n
                                            - PPA_F_VLAN_FILTER_VLAN_TAG
                                        */
} PPA_CMD_VLAN_MATCH_FIELD;

/*!
  \brief This is the data structure for PPA VLAN configuration ioctl() on a per interface basis from userspace. It is currently supported only for bridging paths.
*/  
typedef struct {
    uint16_t        vlan_vci;       /*!< VLAN Information including VLAN Id, 802.1p and CFI bits */
    uint16_t        qid;            /*!< dest_qos */
    uint32_t        out_vlan_id;    /*!< new out vlan id */
    uint32_t        out_vlan_tag_ctrl;  /*!< unmodify(0)/remove(1)/insert(2)/replac(3), for vlan tag based only. */
    uint32_t        inner_vlan_tag_ctrl;/*!< unmodify(0)/remove(1)/insert(2)/replac(3), for vlan tag based only. */
    uint16_t        num_ifs;         /*!< Number of interfaces in the array of PPA_IFINFO structures.  */
    PPA_CMD_IFINFO  vlan_if_membership[PPA_MAX_IFS_NUM]; /*!< Pointer to array of interface info structures for each interface which is a member of this VLAN group. The number of entries is given by num_ifs. */
} PPA_CMD_VLAN_INFO;

/*!
    \brief This is the data structure for basic VLAN filter setting in PPA ioctl interface
*/
typedef struct {
    PPA_CMD_VLAN_MATCH_FIELD    match_field;  /*!< vlan filter match field */
    PPA_CMD_VLAN_INFO           vlan_info;    /*!< vlan information */
} _PPA_CMD_VLAN_FILTER_CONFIG;

/*!
    \brief This is the data structure for VLAN filter configure in PPA ioctl interface
*/
typedef struct {
    _PPA_CMD_VLAN_FILTER_CONFIG vlan_filter_cfg;  /*!< vlan filter basc information */
    uint32_t                    flags;            /*!< flag */
} PPA_CMD_VLAN_FILTER_CONFIG;

/*!
    \brief This is the data structure for PPA VLAN configuration as passed to the PPA ioctl() API from userspace. It is
currently supported only for bridging paths.
*/
typedef struct {
    PPA_CMD_COUNT_INFO          count_info; /*!< Number of filters returned in pointer to array of filters. */
    PPA_CMD_VLAN_FILTER_CONFIG  filters[1]; /*!< it is a dummy array. Userspace should apply storage buffer for it */
} PPA_CMD_VLAN_ALL_FILTER_CONFIG;
/*!
    \brief This is the data structure for PPA accelerated statistics for an interface. Depending on the platform and
             acceleration capabilities, some of the statistics may not be available.
*/
typedef struct {
    PPA_IFNAME      ifname[PPA_IF_NAME_SIZE];   /*!< interface name ( provides storage buffer) */
    uint8_t         mac[PPA_ETH_ALEN];  /*!< MAC address of the Ethernet Interface ( provides storage buffer) */
    uint32_t        flags;              /*!< reserved for future */
} PPA_CMD_IF_MAC_INFO;

/*!
    \brief This is the data structure for LAN/WAN interface setting
*/
typedef struct {
    uint32_t        num_ifinfos;             /*!< number of interface in the list */
    PPA_CMD_IFINFO  ifinfo[PPA_MAX_IFS_NUM]; /*!< buffer for storing network interface list */
} PPA_CMD_IFINFOS;

/*!
    \brief This is the data structure for Multicast group related ioctl
*/
typedef struct {
    uint8_t             mac[PPA_ETH_ALEN];                                /*!< mac address of the multicast group, reserved for future */
    PPA_IFNAME          lan_ifname[PPA_MAX_MC_IFS_NUM][PPA_IF_NAME_SIZE]; /*!< downstream interface list buffer */
    PPA_IFNAME          src_ifname[PPA_IF_NAME_SIZE];                     /*!< source interface which receive multicast streaming packet */
    uint32_t            num_ifs;                                          /*!< downstream interface number */
    uint32_t            bridging_flag;  /*!< IGMP Proxy/snooping flag:  0 - routing mode/igmp proxy, 1 - bring mode/igmp snooping. */ 

    uint32_t            new_dscp_en;    /*!< dscp editing flag: 1 -- need to edit, 0 --unmodify */
    PPA_CMD_MC_ENTRY    mc;             /*!< multicast group information */ 
} PPA_CMD_MC_GROUP_INFO;


#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
/*!
    \brief This is the data structure for CAPWAP related ioctl
*/
typedef struct {
   
   PPA_IFNAME          lan_ifname[PPA_MAX_CAPWAP_IFS_NUM][PPA_IF_NAME_SIZE]; /*!< destination list for upstream */
   
   uint32_t            num_ifs;                                          /*!< upstream destination interface number*/

   uint8_t             phy_port_id[PPA_MAX_CAPWAP_IFS_NUM]; /*!< physical port Id for upstream */
   uint8_t             directpath_portid;  /*!< Directpath port Id */
   
   uint16_t            dest_ifid; /*!< destination list */
   
   uint8_t             qid; /*!< PPE FW QoS Queue Id */

   uint8_t             dst_mac[PPA_ETH_ALEN]; /*!< destination mac address */
   uint8_t             src_mac[PPA_ETH_ALEN]; /*!< source mac address */
   
   uint8_t             tos; /*IPV4 header TOS */
   uint8_t             ttl; /*IPV4 header TTL */
   IP_ADDR_C           source_ip;  /*!< source ip */
   IP_ADDR_C           dest_ip;    /*!< destination ip */

   uint16_t            ip_chksum;   /*!< ip checksum */
   uint16_t            source_port; /* UDP source port */
   uint16_t            dest_port;   /* UDP destination port */
   uint16_t            udp_chksum;  /*!< UDP checksum */

   uint8_t             rid; /* CAPWAP RID */
   uint8_t             wbid; /* CAPWAP WBID */
   uint8_t             t_flag; /* CAPWAP T */
   uint32_t            max_frg_size; /*!< Maximum Fragment size */
   uint32_t            p_entry;
   uint32_t            ds_mib; /*!< DS CAPWAP MIB */
   uint32_t            us_mib; /*!< US CAPWAP MIB */
   uint32_t	       tunnel_idx; /*!< Tunnel Index */

} PPA_CMD_CAPWAP_INFO;

/*!
    \brief This is the data structure for get all CAPWAP group via ioctl
*/
typedef struct {
    PPA_CMD_COUNT_INFO      count_info;       /*!< the capwap counter */
    PPA_CMD_CAPWAP_INFO     capwap_list[1]; /*!< Note, here is a dummy array, user need to malloc memory accordingly to the counter  */
} PPA_CMD_CAPWAP_GROUPS_INFO;



#endif

/*!
    \brief This is the data structure for get all Multicast group via ioctl
*/
typedef struct {
    PPA_CMD_COUNT_INFO      count_info;       /*!< the multicast counter */
    PPA_CMD_MC_GROUP_INFO   mc_group_list[1]; /*!< Note, here is a dummy array, user need to malloc memory accordingly to the session number */
} PPA_CMD_MC_GROUPS_INFO;

/*!
    \brief This is the data structure contains PPA session information.
*/
typedef struct {
    uint32_t                    session;                     /*!< PPA SESSION pointer. Note, here we just use its address to delete a session for ioctl*/
    uint32_t                    hash;
    uint16_t                    ip_proto;      /*!< IP portocol TCP,UDP.  */
    uint16_t                    ip_tos;        /*!< IP ToS value  */   
    PPA_IPADDR                  src_ip;        /*!< source IP address  */
    uint16_t                    src_port;      /*!< source port  */  
    PPA_IPADDR                  dst_ip;        /*!< destination IP address  */ 
    uint16_t                    dst_port;      /*!< destination port  */ 
    PPA_IPADDR                  nat_ip;         /*!< IP address to be replaced by NAT if NAT applies */
    uint16_t                    nat_port;       /*!< Port to be replaced by NAT if NAT applies */
    uint32_t                    new_dscp;       /*!< If DSCP remarking required  */
    uint16_t                    new_vci;        /*!<  new vci ( in fact, it is new inner vlan id )*/
    uint32_t                    out_vlan_tag;   /*!< Out VLAN tag  */
    uint16_t                    dslwan_qid;     /*!< WAN qid  */
    uint16_t                    dest_ifid;      /*!< Destination interface  */

    uint32_t                    flags;          /*!<   Internal flag : SESSION_IS_REPLY, SESSION_IS_TCP, \n
                                                                       SESSION_ADDED_IN_HW, SESSION_NOT_ACCEL_FOR_MGM \n
                                                                       SESSION_VALID_NAT_IP, SESSION_VALID_NAT_PORT, \n
                                                                       SESSION_VALID_VLAN_INS, SESSION_VALID_VLAN_RM, \n
                                                                       SESSION_VALID_OUT_VLAN_INS, SESSION_VALID_OUT_VLAN_RM, \n
                                                                       SESSION_VALID_PPPOE, SESSION_VALID_NEW_SRC_MAC, \n
                                                                       SESSION_VALID_MTU, SESSION_VALID_NEW_DSCP, \n
                                                                       SESSION_VALID_DSLWAN_QID, \n
                                                                       SESSION_TX_ITF_IPOA, SESSION_TX_ITF_PPPOA \n
                                                                       SESSION_LAN_ENTRY, SESSION_WAN_ENTRY,    */ 
    uint32_t                     flag2; /*!<SESSION_FLAG2_HASH_INDEX_DONE/SESSION_FLAG2_ADD_HW_FAIL  */                                                                       
    PPA_IFNAME                rx_if_name[PPA_IF_NAME_SIZE]; /*!< receive interface name. Note, in struct session_list_item, rx_if and tx_if is a pointer, so here we have to make a workaround for it. */
    PPA_IFNAME                tx_if_name[PPA_IF_NAME_SIZE]; /*!< txansmit interface name. */
    uint64_t                  mips_bytes;                   /*!< bytes processed by the mips */
    uint64_t                  hw_bytes;                     /*!< bytes proccesed by hareware acceleration unit*/
    uint64_t                  prev_sess_bytes;             /*!< last bytes proccesed by hareware acceleration unit or */
    uint8_t                   collision_flag;            /*!< 1 mean the entry is in collsion table or none-hashed, like ASE/Danubel*/
    uint32_t                  priority;                     /*!< skb->priority*/
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    uint32_t                  session_priority;		/*!< session priority */
#endif

#if defined(SESSION_STATISTIC_DEBUG) && SESSION_STATISTIC_DEBUG 
    /*below variable is used for session management debugging purpose */   
    uint16_t                     hash_index;  /*!< hash_index according to PPE FW hash algo */
    uint16_t                     hash_table_id; /*!< 0-first hash table, 1 WAN */
    uint16_t                     src_ip6_index; /*!< Note, 0 means not valid data. so for its correct value, it should be "real index + 1 "  */
    uint16_t                     src_ip6_psuedo_ip; 
    uint16_t                     dst_ip6_index; /*!< Note, 0 means not valid data. so for its correct value, it should be "real index + 1 " */
    uint16_t                     dst_ip6_psuedo_ip;    /*!<  psuedo_ip  */
    uint16_t                     reserved;   /*!< reserved flag */
#endif   
} PPA_CMD_SESSION_ENTRY;

/*!
    \brief This is the data structure contains PPA session extra information.
*/
typedef struct {    
    uint32_t          session;                     /*!< PPA SESSION pointer. Note, here we just use its address to modify a session for ioctl*/
    uint32_t          hash; /* Hash value of the session */
    PPA_SESSION_EXTRA  session_extra; /*!< PPA SESSION extra pointer. */ 
    uint32_t                    flags;   /*!<   Internal flag : PPA_F_SESSION_NEW_DSCP \n
                                                            PPA_F_MTU, PPA_F_SESSION_OUT_VLAN, PPA_F_ACCEL_MODE ....\n                                                            
                                                */
    uint32_t     lan_wan_flags; /*!<   Internal flag : the flag to matcn LAN only, or WAN only or both \n
                                                                              The possible value is SESSION_WAN_ENTRY, SESSION_LAN_ENTRY
                                                */
}PPA_CMD_SESSION_EXTRA_ENTRY;


/*!
    \brief This is the data structure for routing session Timer Info
*/
typedef struct {    
    uint32_t          session;                     /*!< PPA SESSION pointer. Note, here we just use its address to modify a session for ioctl*/
    int32_t          timer_in_sec; /*!< PPA SESSION polling timer in seconds. */ 
    uint32_t                    flags;   /*!<   Reserved for future */
}PPA_CMD_SESSION_TIMER;


/*!
    \brief This is the data structure for routing session information
*/
typedef struct {
    PPA_CMD_COUNT_INFO      count_info;         /*!< session count */
    PPA_CMD_SESSION_ENTRY   session_list[1];    /*!< Note, here is a dummy array, user need to malloc memory accordingly to the session number */
} PPA_CMD_SESSIONS_INFO;

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
/*!
    \brief This is the data structure for session criteria information
*/
typedef struct {
	uint32_t ppa_low_prio_data_rate;	/*!< low priority session data rate */
	uint32_t ppa_def_prio_data_rate;	/*!< default priority session data rate */
	uint32_t ppa_low_prio_thresh;		/*!< low priority session threshold */
	uint32_t ppa_def_prio_thresh;		/*!< default priority session threshold */
} PPA_CMD_SESSIONS_CRITERIA_INFO;

/*!
    \brief This is the data structure for session criteria information
*/
typedef struct {
    uint32_t		sessions_low_prio_data_rate;	/*!< low priority session data rate */
    uint32_t		sessions_def_prio_data_rate;    /*!< default priority session data rate */
    uint32_t		sessions_low_prio_thresh;    /*!< low priority session threshold */
    uint32_t		sessions_def_prio_thresh;    /*!< default priority session threshold */
} PPA_CMD_SESSIONS_CRITERIA;

/*!
    \brief This is the data structure for session criteria information
*/
typedef struct {
    uint32_t		flags;	
} PPA_CMD_SWAP_SESSIONS;

typedef struct {
	PPA_CMD_COUNT_INFO count_info[MAX_SESSION_TYPE][MAX_SESSION_PRIORITY][MAX_DATA_FLOW_ENGINES];
} PPA_CMD_SESSION_COUNT_INFO;

typedef struct {
	PPA_CMD_COUNT_INFO 				count_info;
	PPA_CMD_SESSION_EXTRA_ENTRY 	session_extra_info[1];
}PPA_CMD_MANAGE_SESSION;

typedef struct {
	PPA_CMD_SESSION_COUNT_INFO session_count;
    PPA_CMD_SESSION_ENTRY   session_list[1];    /*!< Note, here is a dummy array, user need to malloc memory accordingly to the session number */
}PPA_CMD_GET_SESSIONS_INFO;

#endif

/*!
    \brief This is the data structure for routing detail session information
*/
typedef struct { 
    uint16_t                    ip_proto;      /*!< IP portocol TCP,UDP.  */
    PPA_IPADDR             src_ip;        /*!< source IP address  */
    uint16_t                    src_port;      /*!< source port  */  
    PPA_IPADDR             dst_ip;        /*!< destination IP address  */ 
    uint16_t                    dst_port;      /*!< destination port  */ 
    PPA_IPADDR             nat_ip;         /*!< IP address to be replaced by NAT if NAT applies */
    uint16_t                    nat_port;       /*!< Port to be replaced by NAT if NAT applies */
    uint32_t                    new_dscp;       /*!< If DSCP remarking required  */
    uint16_t                    in_vci_vlanid;        /*!<  new vci ( in fact, it is new inner vlan id )*/
    uint32_t                    out_vlan_tag;   /*!< Out VLAN tag  */
    uint16_t                    qid;     /*!< WAN qid  */ 
    uint32_t                    flags;          /*!<   Internal flag : SESSION_IS_REPLY, SESSION_IS_TCP, \n
                                                                       SESSION_ADDED_IN_HW, SESSION_NOT_ACCEL_FOR_MGM \n
                                                                       SESSION_VALID_NAT_IP, SESSION_VALID_NAT_PORT, \n
                                                                       SESSION_VALID_VLAN_INS, SESSION_VALID_VLAN_RM, \n
                                                                       SESSION_VALID_OUT_VLAN_INS, SESSION_VALID_OUT_VLAN_RM, \n
                                                                       SESSION_VALID_PPPOE, SESSION_VALID_NEW_SRC_MAC, \n
                                                                       SESSION_VALID_MTU, SESSION_VALID_NEW_DSCP, \n
                                                                       SESSION_VALID_DSLWAN_QID, \n
                                                                       SESSION_TX_ITF_IPOA, SESSION_TX_ITF_PPPOA \n
                                                                       SESSION_LAN_ENTRY, SESSION_WAN_ENTRY,    */                                                                      
    uint32_t                dest_ifid; /*!< txansmit interface name. */
    uint8_t                  src_mac[PPA_ETH_ALEN];    /*!< src mac addres */
    uint8_t                  dst_mac[PPA_ETH_ALEN];    /*!< dst mac address */
    uint16_t                 pppoe_session_id;   /*!< pppoe session id */
    
    uint32_t                 mtu; /*!<  mtu */
}PPA_CMD_SESSIONS_DETAIL_INFO; 
    
/*!
    \brief This is the data structure for basic ppa Versions
*/
typedef struct {
     uint32_t index; /*!< index for PP32 */
     uint32_t family; /*!< ppa version hardware family */
     uint32_t type; /*!< ppa version hardware type */
     uint32_t itf;/*!< ppa version itf */
     uint32_t mode; /*!< ppa version mode */
     uint32_t major; /*!< ppa version major version number */
     uint32_t mid; /*!< ppa version mid version number */
     uint32_t minor;  /*!< ppa version minor version number */
     uint32_t tag;  /*!< ppa version tag number. Normally for internal usage */
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) 
     uint32_t 	id;
     char 	name[PPA_VERSION_LEN];
     char 	version[PPA_VERSION_LEN];
#endif
} PPA_VERSION;

/*!
    \brief This is the data structure for ppa wan mode information
*/
typedef struct{
    uint32_t   wan_port_map;   /*!< wan port map information*/   
    uint32_t   mixed;  /*!< mixed flag */       
} PPA_WAN_INFO;

/*!
    \brief This is the data structure for ppa supported feature list information
*/
typedef struct{
    uint8_t   ipv6_en;   /*!< ipv6 enable/disable status */   
    uint8_t   qos_en;   /*!< qos enable/disable status  */       
} PPA_FEATURE_INFO;

/*!
    \brief This is the data structure for PPA subsystem Versions, like ppa subsystem, ppe fw, ppe driver and so on
*/
typedef struct {
    PPA_VERSION ppa_api_ver;               /*!< PPA API verion */
    PPA_VERSION ppa_stack_al_ver;          /*!< PPA stack verion */  
    PPA_VERSION ppe_hal_ver;               /*!< PPA HAL verion */ 
    PPA_VERSION ppe_fw_ver[2];                /*!< PPA FW verion */
    PPA_VERSION ppa_subsys_ver;            /*!< PPA Subsystem verion */
    PPA_WAN_INFO ppa_wan_info;            /*!< PPA WAN INFO */
    PPA_FEATURE_INFO  ppe_fw_feature;  /*!< PPE FW feature lists */
    PPA_FEATURE_INFO  ppa_feature;  /*!< PPA Level feature lists */
    
} PPA_CMD_VERSION_INFO;

/*!
    \brief This is the data structure for basic vlan range
*/
typedef struct  {
    uint32_t start_vlan_range;  /*!< WAN interface start vlan id */
    uint32_t end_vlan_range;    /*!< WAN interface end vlan id */
}PPA_VLAN_RANGE;

/*!
    \brief This is the data structure VLAN range in mixed mode
*/
typedef struct {
    PPA_CMD_COUNT_INFO  count_info;  /*!< PPA Count info */
    PPA_VLAN_RANGE      ranges[1];  /*!< it is dummy array, need to malloc in userspace */
} PPA_CMD_VLAN_RANGES;

/*!
    \brief This is the data structure for MAC INFO 
*/
typedef struct {
    PPA_CMD_COUNT_INFO  count_info;          /*!< PPA Count info */
    PPA_CMD_MAC_ENTRY   session_list[1];    /*!< it is a dummy array, need to malloc bedore use it in userspace */
} PPA_CMD_ALL_MAC_INFO;

/*!
    \brief This is the data structure for BRIGE MAC LEARNING ENABLE/DISABLE INFO 
*/
typedef struct {
    uint32_t        bridge_enable;  /*!< enable/disable bridging mac address learning flag */
    uint32_t        flags;   /*!< reserved for future */
} PPA_CMD_BRIDGE_ENABLE_INFO;

/*!
    \brief QoS Shaper Mode Configuration
*/
typedef enum {
        PPA_QOS_SHAPER_NONE=0,
        PPA_QOS_SHAPER_COLOR_BLIND=1,
        PPA_QOS_SHAPER_TR_TCM=2,
        PPA_QOS_SHAPER_TR_TCM_RFC4115=3,
        PPA_QOS_SHAPER_LOOSE_COLOR_BLIND=4
}PPA_QOS_SHAPER_MODE;

/*!
    \brief QoS Shaper configuration structure
*/
typedef struct {
        PPA_QOS_SHAPER_MODE       mode; /*!< Mode of Token Bucket shaper */
	uint32_t		enable; /*!< Enable for Shaper */
        uint32_t                cir; /*!< Committed Information Rate in bytes/s */
        uint32_t                cbs; /*!< Committed Burst Size in bytes */
        uint32_t                pir; /*!< Peak Information Rate in bytes/s */
        uint32_t                pbs; /*!< Peak Burst Size */
        uint32_t                flags; /*!< Flags define which shapers are enabled
                                                - QOS_SHAPER_F_PIR
                                                - QOS_SHAPER_F_CIR */
	int32_t			phys_shaperid;
}PPA_QOS_SHAPER_CFG;

/*!
    \brief This is the data structure for PPA QOS Internal INFO
*/
typedef struct { 
    uint32_t   t; /*!<  Time Tick */
    uint32_t   w; /*!<  weight */
    uint32_t   s; /*!<  burst */
    uint32_t   r; /*!<  Replenish */
    uint32_t   d; /*!<  ppe internal variable */
    uint32_t   tick_cnt; /*!<  ppe internal variable */
    uint32_t   b; /*!<  ppe internal variable */

    /*For PPA Level only */
    uint32_t   reg_addr;  /*!<  register address */
    uint32_t bit_rate_kbps;  /*!<  rate shaping in kbps */  
    uint32_t weight_level;   /*!<  internal wfq weight */
    
}PPA_QOS_INTERVAL;

/*!
    \brief This is the data structure for PPA QOS Internal Debug INFO
*/
typedef struct  {
    //struct wtx_qos_q_desc_cfg
    uint32_t    threshold; /*!<  qos wtx threshold */
    uint32_t    length;  /*!<  qos wtx length  */
    uint32_t    addr; /*!<  qos wtx address */
    uint32_t    rd_ptr; /*!<  qos wtx read pointer  */
    uint32_t    wr_ptr; /*!<  qos wtx write pointer */

        /*For PPA Level only */
   uint32_t   reg_addr;  /*!<  register address */     
}PPA_QOS_DESC_CFG_INTERNAL;


/*!
    \brief This is the data structure for PPA QOS to get the maximum queue number supported for one physical port
*/
typedef struct {
    uint32_t        portid;   /*!<  the phisical port id which support qos queue */    
    uint32_t        queue_num;  /*!<  the maximum queue number is supported */
    uint32_t        flags;    /*!<  Reserved currently */
} PPA_CMD_QUEUE_NUM_INFO;

/*!
    \brief This is the data structure for PPA QOS MIB Counter
*/
typedef struct {
    uint64_t        total_rx_pkt;   /*!<  all packets received by this queue */
    uint64_t        total_rx_bytes; /*!<  all bytes received by thi queue */
    uint64_t        total_tx_pkt;   /*!<  all packets trasmitted by this queue */
    uint64_t        total_tx_bytes; /*!<  all bytes trasmitted by thi queue */
    
    uint64_t        cpu_path_small_pkt_drop_cnt;  /*!< all small packets dropped in CPU path for lack of TX DMA descriptor in the queue*/
    uint64_t        cpu_path_total_pkt_drop_cnt;  /*!< all packets dropped in CPU path for lack of TX DMA descriptor in the queue*/
    uint64_t        fast_path_small_pkt_drop_cnt; /*!< all small packets dropped in fast path for lack of TX DMA descriptor */
    uint64_t        fast_path_total_pkt_drop_cnt; /*!< all packets dropped in fast path for lack of TX DMA descriptor */

    uint64_t        tx_diff;  /*!< tx bytes since last read */
    uint64_t        tx_diff_L1;  /*!< tx bytes plus L1 overheader since last read */
    uint64_t        tx_diff_jiffy;  /*!< jiffy diff since last read */
    uint32_t        sys_hz;  /*!< system HZ value. For debugging purpose */
} PPA_QOS_MIB;

/*!
    \brief This is the data structure for PPA QOS to get the maximum queue number supported for one physical port
*/
typedef struct {
		PPA_IFNAME			ifname[PPA_IF_NAME_SIZE];/*!< Interface name on which the Queue is modified*/
    uint32_t        portid; /*!<  the phisical port id which support qos queue */
		int32_t         queue_num; /*!< Logical queue number added will be filled up in this field */
    PPA_QOS_MIB     mib;    /*!<  the mib information for the current specified queue */
    uint32_t        flags;  /*!<  Reserved currently */
} PPA_CMD_QOS_MIB_INFO;



/*!
    \brief This is the data structure for PPA QOS to be enabled/disabled
*/
typedef struct {
    uint32_t        portid;  /*!<  which support qos queue. */
    uint32_t        enable;  /*!<  enable/disable  flag */
    uint32_t        flags;   /*!<  Reserved currently */
} PPA_CMD_QOS_CTRL_INFO;

/*!
    \brief This is the data structure for PPA Rate Shapping Set/Get/Reset one queue's rate limit
*/
typedef struct {
    PPA_IFNAME 		ifname[PPA_IF_NAME_SIZE];
    uint32_t        	portid;   /*!<  the phisical port id which support qos queue */
    int32_t        	queueid;  /*!<  the queu id. Now it only support 0 ~ 7 */
#if defined(MBR_CONFIG) && MBR_CONFIG
    int32_t 		shaperid;  /*!<  the shaper id. Now it only support 0 ~ 7 */
#endif
    PPA_QOS_SHAPER_CFG 	shaper;
    uint32_t        	rate;     /*!<  rate limit in kbps  */
    uint32_t        	burst;    /*!<  rate limit in bytes. Note: it is PPE FW QOS internal value. Normally there is no need to set this value or just set to default value zero */
    uint32_t        	flags;    /*!<  Reserved currently */
} PPA_CMD_RATE_INFO;



/*!
    \brief This is the data structure for PPA WFQ Set/Get/Reset one queue's weight
*/
typedef struct {
    uint32_t        portid;   /*!<  the phisical port id which support qos queue */
    uint32_t        queueid;  /*!<  the queu id. Now it only support 0 ~ 7 */    
    uint32_t        weight;   /*!<  WFQ weight. The value is from 0 ~ 100 */
    uint32_t        flags;    /*!<  Reserved currently */
} PPA_CMD_WFQ_INFO;

#if defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
/*!
    \brief Union of ppa power transitin watermark.
*/
union watermark {
        uint32_t ppa_pwm_wm1;  /*!< Watermark value for PPE transition between D0 and D1 */
        uint32_t ppa_pwm_wm2;  /*!< Watermark value for PPE transition between D1 and D2 */
        uint32_t ppa_pwm_wm3;  /*!< Watermark value for PPE transition between D2 and D3 */
};

/*!
    \brief This is the data structure definition for PPA PWM states water mark
*/
typedef struct {
    int16_t flag;   /*!< flag indicating if watermark type. flag=1: watermark is packet count; flag=2: watermark is byte count  */
    int32_t time_interval; /*!< time interval of watermarks in milliseconds. */
    union watermark WM;  /*!< Watermark value for PPE transition */
}WM_t;

/*!
    \brief This is the data structure for PPA Power management basic watermark configuration 
*/
typedef struct {    
    WM_t ppa_pwm_wm1;  /*!< Watermark value for PPE transition between D0 and D1 */
    WM_t ppa_pwm_wm2;  /*!< Watermark value for PPE transition between D1 and D2*/
    WM_t ppa_pwm_wm3;  /*!< Watermark value for PPE transition between D2 and D3*/
}PPA_PWM_WM_t;

/*!
    \brief This is the data structure for PPA Power management configuration
*/
typedef struct {
    uint8_t ppa_pwm;  /*!< PPA power management mode: 0/1-OFF/ON */
    PPA_PWM_WM_t ppa_pwm_wm_up;  /*!< Watermark value for PPE transition for various states. */
    PPA_PWM_WM_t ppa_pwm_wm_down;  /*!< Watermark value for PPE transition for various states. */
    PPA_PWM_STATE_t e_ppa_pwm_init_state;  /*!< Initial power/performance state for PPE */
    uint32_t flag;  /*!< reserved.*/
}PPA_PWM_CONFIG_t;
#endif //end of CONFIG_LTQ_PMCU

/*!
    \brief This is the data structure for Mutiple Field Based Classification And VLAN Assigment feature's basic auto-learning VLAN related information.
*/
typedef struct
{
    PPA_IFNAME tx_ifname[PPA_IF_NAME_SIZE]; /*!<  destination interface name, like eth0.3. If blank, then match all interface */
    PPA_IFNAME rx_ifname[PPA_IF_NAME_SIZE]; /*!<  receiving interface name, like eth1.2, If blank, then match all interface */
}PPA_MULTIFIELD_VLAN_INFO_AUTO;

/*!
    \brief This is the data structure for Mutiple Field Based Classification And VLAN Assigment's manual-learning VLAN information based on default key selection.
*/
typedef struct
{
    uint8_t tx_if_id; /*!<  physical destination interface id match, like 0 for eth0, 1 for eth1. It is part of key 14 */
    uint8_t rx_if_id; /*!<  physical destination interface id match, like 0 for eth0, 1 for eth1. It is part of key 15 */
    uint8_t is_vlan; /*!< VLAN Flag match. 1: only match single vlan, 2: match double vlan, 0: match no vlan packet. It is part of key14 */
    uint8_t is_vlan_mask; /*!< VLAN Flag mask*/

    uint8_t out_vlan_pri;  /*!< outer vlan priority ( 3 bits only) match. Based on PPA default Key Selection, it is part of key 8/l2_off14 */
    uint8_t out_vlan_pri_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/
    uint8_t out_vlan_cfi;  /*!< outer vlan cfi ( 1 bits only) match. Based on PPA default Key Selection, it is part of key 8/l2_off14 */
    uint8_t out_vlan_cfi_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/

    uint16_t out_vlan_vid;  /*!< outer vlan id ( 1 bits only) match. Based on PPA default Key Selection, it is part of key 8 and 9/l2_off14-15 */
    uint16_t out_vlan_vid_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/

    uint8_t in_vlan_pri;  /*!< inner vlan priority ( 3 bits only) match. Based on PPA default Key Selection, it is part of key 12/l2_off18 */
    uint8_t in_vlan_pri_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/
    uint8_t in_vlan_cfi;  /*!< inner vlan cfi ( 1 bits only) match. Based on PPA default Key Selection, it is part of key 12/l2_off18 */
    uint8_t in_vlan_cfi_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/

    uint16_t in_vlan_vid;  /*!< inner vlan id ( 1 bits only) match. Based on PPA default Key Selection, it is part of key 12 and 13/l2_off18-19 */
    uint16_t in_vlan_vid_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/

    uint8_t  action_out_vlan_insert;  /*!< out vlan insert action */
    uint8_t  action_in_vlan_insert;    /*!< inner vlan insert action */
    uint8_t  action_out_vlan_remove;   /*!< out vlan remove action */
    uint8_t  action_in_vlan_remove;    /*!< inner vlan remove action */

    uint8_t  new_out_vlan_pri;  /*!< action: new out vlan priority  */
    uint8_t  new_out_vlan_cfi;  /*!< action:new out vlan cfi */
    uint8_t  new_in_vlan_pri;   /*!< action:new inner vlan priority */
    uint8_t  new_in_vlan_cfi;   /*!< action:new inner vlan cfi  */

    uint16_t new_out_vlan_vid;   /*!< action:new out vlan id */
    uint16_t new_in_vlan_vid;      /*!< action:new inner vlan id */ 

    uint16_t new_out_vlan_tpid;   /*!< action:new out vlan tpid */ 
}PPA_MULTIFIELD_VLAN_INFO_MANUAL;

/*!
    \brief This is the data structure for Mutiple Field Based Classification And VLAN Assigment's VLAN KEY/MASK/ACTION based on default key selection.
*/
typedef struct  
{
    uint8_t bfauto;  /*!< flag to use simple autoway to add a multiple field editing flow. It is used by PPA API level and hook/ppacmd only*/
    PPA_MULTIFIELD_VLAN_INFO_AUTO vlan_info_auto;  /*!< auto-learn vlan key/mask/action. Note, it is only for add/delete a rule, not for get commands */
    PPA_MULTIFIELD_VLAN_INFO_MANUAL vlan_info_manual; /*!< munually provide vlan key/mask/action. Even vlan_info_auto is used, ppa will set vlan_info_manual structure for PPE driver */
       
}PPA_MULTIFIELD_VLAN_INFO;

/*!
    \brief This is the data structure for Mutiple Field Based Classification And VLAN Assigment's configuration based on default key selection.
*/
typedef struct  
{                                                  
    uint16_t ether_type;  /*!< ethernet type match, like 0x0800. Based on PPA default Key Selection, it is key0_1*/
    uint16_t ether_type_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/

    uint8_t dscp;  /*!< dscp(tos) match. 1: key of  dscp in ip header, like 0x08. Based on PPA default Key Selection, it is key2*/
    uint8_t dscp_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/
    uint8_t pkt_length;  /*!< packet length ( less than) match. Its value is got from ethernet packet length /64. Based on PPA default Key Selection, it is key3*/
    uint8_t pkt_length_mask;  /*!< packet length mask.*/

    uint32_t s_ip;  /*!< source ip match, like 0x0a000009 ( 10.0.0.9) . Based on PPA default Key Selection, it is key4 ~ Key7*/
    uint32_t s_ip_mask;  /*!< match mask to specify the bits to match. Note, 0 means need to match and 1 means not to match*/

    uint8_t l3_off0;  /*!< L3 Offset 0 match. Baed on PPA default key selection, it is key10 */
    uint8_t l3_off0_mask; /*!< L3 Offset 0 mask */
    uint8_t l3_off1; /*!< L3 Offset 1 match. Baed on PPA default key selection, it is key11 */
    uint8_t l3_off1_mask; /*!< L3 Offset 1 mask */
    
    uint8_t ipv4;  /*!< ipv4 match ( 1 bit). 1: match only ipv4.  0: match none ipv4 packet. It is part of key15. */
    uint8_t ipv4_mask;  /*!< ipv4 mask. 0 -need to match, 1-no need to match */
    uint8_t ipv6;  /*!< < ipv6 match ( 1 bit). 1: match only ipv6.  0: match none ipv6 packet. 1: match only ipv6, 0: match none ipv6 packet. It is part of key15 */
    uint8_t ipv6_mask;  /*!< ipv6 mask. 0 -need to match, 1-no need to match */
    
    uint8_t pppoe_session;  /*!< pppoe session flag match: 1: match pppoe session only. 0-- match none pppoe session packet. It is part of key14 */
    uint8_t pppoe_session_mask;  /*!< pppoe session flag mask. 0 -need to match, 1-no need to match */
    uint8_t fwd_cpu; /*!< action: forward packet to CPU or not. \n
                                       1: forward to CPU.
                                       0: forward to its original destination port\n */
                            
    uint8_t queue_id; /*!< action: which queue assign for the current flow. */

    PPA_MULTIFIELD_VLAN_INFO vlan_info; /*!< specify vlan key/mask/action. Based on PPA default Key Selection. It relates to key8/key9, key 12/key13 and key14/key15 and part of vlan action*/
} PPA_MULTIFIELD_DEFAULT_INFO;

/*!
    \brief This is the data structure for Mutiple Field Based Classification And VLAN Assigment's configuration based on second default key selection.
    \note, for future only now
*/

typedef struct  
{
 
}PPA_MULTIFIELD_DEFAULT2_INFO;

/*!
    \brief This is the data structure for Mutiple Field Based Classification And VLAN Assigment's configuration based on different key selection.
    \note More key selection based configuration will be implemented. Note, different key selectoin may have different configuration.  \n
    PPA should parse the cfg according to current  key selection mode
    
*/
typedef union  
{
    PPA_MULTIFIELD_DEFAULT_INFO cfg0; /*!< multiple field configuration based on default key selection. */
    PPA_MULTIFIELD_DEFAULT2_INFO cfg2; /*!< multiple field configuration based on second default key selection. */
}PPA_MULTIFIELD_FLOW_INFO;

/*!
    \brief This is the data structure for IOCTL of Mutiple Field Based Classification And VLAN Assigment's configuration.
    
*/
typedef struct 
{    
    int32_t index; /*!< for get command, it is input, for add command, it is input. for del, it is input ( index must be valid in this case, -1 means delete all flow ) */
    int32_t last_index; /*!< for get command. It will be used in ppacmd.c only */
    uint32_t flag; /*!< Most time, it is input only. But for PPA_CMD_GET_MULTIFIELD_STATUS, it is input/ouput */
    PPA_MULTIFIELD_FLOW_INFO flow;     /*!< the Mutiple Field Based Classification And VLAN Assigment configuration/information */
} PPA_CMD_MULTIFIELD_FLOW_INFO ; 

/*!
    \brief This is the data structure for IOCTL to enable/disable Mutiple Field Based Classification And VLAN Assigment.
    
*/
typedef struct PPA_CMD_ENABLE_MULTIFIELD_INFO
{
    uint32_t enable_multifield;  /*!< flag of enable/disable the Mutiple Field Based Classification And VLAN Assigment feature  */
    uint32_t flag;  /*!< reserved for future */
} PPA_CMD_ENABLE_MULTIFIELD_INFO;

/*!
    \brief This is the data structure for getting all exported PPA hooks.
*/
typedef struct EXP_FN_INFO
{
    uint8_t hookname[MAX_HOOK_NAME_LEN]; /*!< hook name */
    uint32_t hook_addr;   /*!< mid hook address */
    uint8_t hook_flag:1;   /*!< hook_flag: 0-disabled, otherwise -enabled */
    uint8_t used_flag:1;   /*!< used_flag: 0-not used, otherwise -used */
    uint8_t reserved:6;   /*!< reserved for future */
}PPA_HOOK_INFO;

/*!
    \brief This is the data structure for  PPA hooks list
*/
typedef struct PPA_HOOK_INFO_LIST {
    PPA_HOOK_INFO info;  /*!< ppa hook info */

    struct PPA_HOOK_INFO_LIST *next; /*!< point to next ppa hook info */
} PPA_HOOK_INFO_LIST;

/*!
    \brief This is the data structure for getting all exported PPA hooks.
*/
typedef struct 
{
    uint32_t hook_count; /*!< hook counter */ 
    uint32_t flag; /*!< reserved for future */ 
    PPA_HOOK_INFO list[1];   /*!< it is a dummy array. Userspace should apply storage buffer for it.  */
}PPA_CMD_HOOK_LIST_INFO;

/*!
    \brief This is the data structure for enable/disable ppa hook
*/
typedef struct
{
    uint8_t hookname[MAX_HOOK_NAME_LEN]; /*!< hook name */
    uint32_t enable; /*!< enable/disable ppa hook  */ 
    uint32_t flag; /*!< reserved for future */ 
}PPA_HOOK_ENABLE_INFO;

/*!
    \brief This is the data structure for IOCTL to enable/disable ppa hook
*/
typedef PPA_HOOK_ENABLE_INFO PPA_CMD_HOOK_ENABLE_INFO;

/*!
    \brief This is the data structure to get the memory value.
*/
typedef struct 
{
    uint32_t  addr;  /*!< The memory adddress to read */
    uint32_t  addr_mapped;  /*!< The mapped memory adddress to read */
    uint32_t shift;  /*!< the bits to shitf */
    uint32_t size; /*!< size of bits to read*/
    uint32_t repeat; /*!< read repeat times  */
    uint32_t flag; /*!< reserved for future */ 
    uint32_t buffer[1]; /*!< the buffer to store the value.  it is a dummy array. Userspace should apply storage buffer for it. Its size should be at least size * sizeof (uint32_t) */
}PPA_READ_MEM_INFO;

#ifdef NO_DOXY
typedef PPA_READ_MEM_INFO PPA_CMD_READ_MEM_INFO;

/*!
    \brief This is the data structure to set the memory value.
*/
typedef struct 
{
    uint32_t addr;  /*!< The memory adddress to set */
    uint32_t  addr_mapped;  /*!< The mapped memory adddress to read */
    uint32_t shift;  /*!< the bits to shitf */
    uint32_t size; /*!< size of bits */
    uint32_t value;  /*!< value of the data*/
    uint32_t repeat; /*!< set repeat times*/
    uint32_t flag; /*!< reserved for future */ 
}PPA_SET_MEM_INFO;

/*!
    \brief This is the data structure to set the memory value for IOCTL.
*/
typedef PPA_SET_MEM_INFO PPA_CMD_SET_MEM_INFO;
#endif

/*!
    \brief This is the data structure to get the maximum entries, like lan/wan/mc/bridging
*/
typedef struct 
{
    uint32_t        max_lan_entries;      /*!< Maximum LAN session entries,  including collision */
    uint32_t        max_wan_entries;      /*!< Maximum WAN session entries,  including collision */
    uint32_t        max_lan_collision_entries;      /*!< Maximum LAN session entries */
    uint32_t        max_wan_collision_entries;      /*!< Maximum WAN session entries */
    uint32_t        max_mc_entries;       /*!< Maximum Multicast session entries */
    uint32_t        max_bridging_entries;/*!< Maximum Bridge session entries */    
    uint32_t        max_ipv6_addr_entries;      /*!< Maximum IPV6 address entries */
    uint32_t        max_fw_queue;                       /*!< Maximum PPE FW queue number */
    uint32_t        max_6rd_entries;       /*!<Maximum 6rd tunnel entries */
    uint32_t        max_dslite_entries;    /*!<Maximum dslite tunnel entries */
    uint32_t        max_mf_flow;       /*!<Maximum multiple filed flow */
    uint32_t        session_hash_table_num; /*!<Session Hash Table number, currently AR9/VR9/AR10 have 2 hash table for LAN/WAN respectively */
    uint32_t        max_lan_hash_index_num;      /*!< Maximum LAN hash number */
    uint32_t        max_wan_hash_index_num;      /*!< Maximum WAN hash number */
    uint32_t        max_lan_hash_bucket_num;      /*!< Maximum LAN bucket number per hash idnex */
    uint32_t        max_wan_hash_bucket_num;      /*!< Maximum WAN bucket number per hash idnex  */
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint32_t	    ppe_hal_disabled;
    uint32_t	    max_pae_routing_entries;
    uint32_t 	    max_mpe_routing_entries;
    uint32_t 	    max_tunnel_entries;
#endif 
} PPA_MAX_ENTRY_INFO;

/*!
    \brief This is the data structure to get the maximum entries for IOCTL.
*/
typedef struct{
    PPA_MAX_ENTRY_INFO entries; /*!< max entry info*/
    uint32_t        flags;    /*!< reserved for future */
} PPA_CMD_MAX_ENTRY_INFO;

/*!
    \brief This is the data structure to get the physical port id For IOCTL.
*/
typedef struct{
    PPA_IFNAME ifname[PPA_IF_NAME_SIZE]; /*!< the interface name */
    uint32_t   portid;   /*!< the port id */   
    uint32_t   flags;    /*!< reserved for future */
} PPA_CMD_PORTID_INFO;


/*!
    \brief This is the data structure for DSL queue mib 
*/
typedef struct{
    uint32_t   queue_num;   /*!< the queue number filled in the drop mib*/   
    uint32_t   drop_mib[16]; /*!< the drop mib counter */

    //later other DSL mib counter will be added here
    uint32_t   flag; /*!< reserved for futurer */    
} PPA_DSL_QUEUE_MIB;

/*!
    \brief This is the data structure for general  ENABLE/DISABLE INFO 
*/
typedef struct {
    uint32_t        enable;  /*!< enable/disable flag */
    uint32_t        flags;   /*!< reserved for future */
} PPA_CMD_GENERAL_ENABLE_INFO;

/*!
    \brief This is the data structure to get the DSL queue drop mib for IOCTL.
*/
typedef struct{
    PPA_DSL_QUEUE_MIB mib;  /*!< dsl queue mib coutner. */
    uint32_t   flags;    /*!< reserved for future */
} PPA_CMD_DSL_MIB_INFO;

/*!
    \brief This is the data structure for PORT mib 
*/
typedef struct{  
    uint64_t     ig_fast_brg_pkts;           /*!<  fast bridge receving packets */
    uint64_t     ig_fast_brg_bytes;          /*!< the fast bridge receving bytes */

    uint64_t     ig_fast_rt_ipv4_udp_pkts;   /*!<  the fast ipv4 routing udp receving packets */
    uint64_t     ig_fast_rt_ipv4_tcp_pkts;   /*!<  the fast ipv4 routing tcp receving packets */
    uint64_t     ig_fast_rt_ipv4_mc_pkts;    /*!<  the fast ipv4 routing multicast receving packets */
    uint64_t     ig_fast_rt_ipv4_bytes;      /*!<  the fast ipv4 receving bytes */

    uint64_t     ig_fast_rt_ipv6_udp_pkts;   /*!<  the fast ipv6 routing udp receving packets*/
    uint64_t     ig_fast_rt_ipv6_tcp_pkts;   /*!<  the fast ipv6 routing tcp receving packets */
    uint64_t     ig_fast_rt_ipv6_mc_pkts;    /*!<  the fast ipv6 routing multicast receving packets */    
    uint64_t     ig_fast_rt_ipv6_bytes;      /*!<  the fast ipv6 routing receving bytes */

    uint64_t     ig_cpu_pkts;                /*!<  cpu packets */
    uint64_t     ig_cpu_bytes;               /*!<  cpu bytes */

    uint64_t     ig_drop_pkts;               /*!<  drop packets */
    uint64_t     ig_drop_bytes;              /*!<  drop bytes */

    uint64_t     eg_fast_pkts;               /*!<  the fast transmiting packets  */

    uint32_t     port_flag;                  /*!<  port flag: PORT_MODE_ETH, PORT_MODE_DSL, PORT_MODE_EXT  */
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint64_t     eg_fast_bytes;              /*!<  the fast transmiting bytes */
#endif    
} PPA_PORT_MIB_INFO;

/*!
    \brief This is the data structure for PORT mib 
*/
typedef struct{  
    PPA_PORT_MIB_INFO mib_info[PPA_MAX_PORT_NUM];     /*!<  mib array: eth0, eth1, DSL, DSL(IPOA/PPPOA), ext1, ext2, ext3, ext4 */
    uint32_t                    port_num;   /*!< maximum port number supported */
    uint32_t                    flags;   /*!< reserved for future */
}PPA_PORT_MIB;


/*!
    \brief This is the data structure for changing to FPI address
*/
typedef struct{  
    uint32_t                    addr_orig;   /*!< original address */
    uint32_t                    addr_fpi;   /*!< converted FPI address */
    uint32_t                    flags;   /*!< reserved for future */
}PPA_FPI_ADDR;

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
/*!
    \brief This is the data structure for PPA HW accelerated statistics for an interface. Depending on the platform and
             acceleration capabilities, some of the statistics may not be available.
*/
typedef struct {
    uint64_t    rx_bytes;       /*!< HW accelerated Rx bytes */
    uint64_t    tx_bytes;       /*!< HW accelerated Tx bytes */
} PPA_HW_ACCEL_STATS;

/*!
    \brief This is the data structure for PPA SW accelerated statistics for an interface.
*/
typedef struct {
    uint64_t    rx_bytes;       /*!< SW accelerated Rx bytes */
    uint64_t    tx_bytes;       /*!< SW accelerated Tx bytes */
} PPA_SW_ACCEL_STATS;

/*!
    \brief This is the data structure for PPA accelerated statistics for an interface. Depending on the platform and
             acceleration capabilities, some of the statistics may not be available.
*/
typedef struct {
    PPA_PORT_MIB_INFO  port_mib_stats;
    PPA_HW_ACCEL_STATS hw_accel_stats;
    PPA_SW_ACCEL_STATS sw_accel_stats;
} PPA_NETIF_ACCEL_STATS;
#endif

#if defined(L2TP_CONFIG) && L2TP_CONFIG
/*!
    \brief This is the data structure for L2TP INFO
*/
typedef struct {
    uint32_t source_ip;
    uint32_t dest_ip;
    uint16_t tunnel_id;
    uint16_t session_id;
    uint32_t tunnel_idx;
    uint32_t tunnel_type;
    uint16_t ip_chksum;
    uint16_t udp_chksum;
} PPA_L2TP_INFO;

#endif

#ifdef NO_DOXY
typedef struct {
    uint32_t mtu_ix;
    uint32_t mtu;  // for add/del/get mtu entry only 
} PPE_MTU_INFO;

typedef struct {
    uint32_t pppoe_ix;
    uint32_t pppoe_session_id;  // for add/del/get a pppoe entry only 
} PPE_PPPOE_INFO;

#if defined(L2TP_CONFIG) && L2TP_CONFIG
typedef struct {
    uint32_t pppol2tp_ix;
    uint32_t pppol2tp_session_id;  // for add/del/get a pppoe entry only
    uint32_t pppol2tp_tunnel_id;
} PPE_PPPOL2TP_INFO;
#endif

typedef struct {
    uint32_t mac_ix;
    uint8_t mac[PPA_ETH_ALEN]; // for add/del/get a MAC entry only 
} PPE_ROUTE_MAC_INFO;

typedef struct {
    uint32_t vlan_id; //out vlanid or ctag_vlan_id
    uint32_t vlan_entry; 
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint16_t port_id;
    uint16_t subif_id;
    uint8_t  ctag_rem;
    uint8_t  ctag_ins;
    uint8_t  stag_rem;
    uint8_t  stag_ins;
    uint32_t stag_vlan_id;
#endif
} PPE_OUT_VLAN_INFO;

typedef struct {
	IP_ADDR ip;
	uint32_t ipv6_entry;
	uint32_t psuedo_ip;
} PPE_IPV6_INFO;

typedef enum {
    TUNNEL_TYPE_NULL = 0, /*!< Not Tunnel  */
    TUNNEL_TYPE_6RD,      /*!< 6rd Tunnel  */
    TUNNEL_TYPE_DSLITE,   /*!< DSLITE Tunnel  */
    TUNNEL_TYPE_CAPWAP,   /*!< CAPWAP Tunnel  */
    TUNNEL_TYPE_L2TP,     /*!< L2TP Tunnel  */
    TUNNEL_TYPE_EOGRE,    /*!< IPv4 Ethernet Over GRE Tunnel  */
    TUNNEL_TYPE_6EOGRE,    /*!< IPv6 Ethernet Over GRE Tunnel  */
    TUNNEL_TYPE_IPOGRE,   /*!< IPv4 IP Over GRE Tunnel  */
    TUNNEL_TYPE_IP6OGRE,   /*!< IPv6 IP Over GRE Tunnel  */
    TUNNEL_TYPE_IPSEC,    /*!< IPSEC Tunnel  */
    TUNNEL_TYPE_MAX       /*!< Not Valid Tunnel type  */
} e_Tunnel_Types;

typedef struct {
    uint32_t tunnel_idx;
    uint32_t tunnel_type;
    struct net_device *tx_dev;
} PPE_TUNNEL_INFO;

typedef struct {
    uint8_t f_ipv6;
    IP_ADDR src_ip;
    IP_ADDR dst_ip;
    uint32_t src_port;
    uint32_t dst_port;
    uint8_t session_id;
} PPA_LRO_INFO;

typedef struct {
    uint32_t f_is_lan;
    IP_ADDR_C src_ip;
    uint32_t src_port;
    IP_ADDR_C dst_ip;
    uint32_t dst_port;
    uint32_t f_is_tcp;   //  1: TCP, 0: UDP
    uint32_t route_type;
    IP_ADDR_C new_ip;   //NAT IP
    uint16_t new_port; //NAT UDP/TCP Port
    uint8_t  new_dst_mac[PPA_ETH_ALEN];
    PPE_ROUTE_MAC_INFO src_mac;
    PPE_MTU_INFO mtu_info;
    uint32_t pppoe_mode;
    PPE_PPPOE_INFO pppoe_info;
    PPE_TUNNEL_INFO tnnl_info;
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    PPE_PPPOL2TP_INFO pppol2tp_info;
    PPE_TUNNEL_INFO l2tptnnl_info;
#endif

    uint32_t f_new_dscp_enable;
    uint32_t new_dscp;
    uint32_t f_vlan_ins_enable;
    uint32_t new_vci;
    uint32_t f_vlan_rm_enable;

    uint32_t f_out_vlan_ins_enable;
    PPE_OUT_VLAN_INFO out_vlan_info;
    uint32_t f_out_vlan_rm_enable;
    uint32_t dslwan_qid;
    uint32_t dest_list;
    uint32_t entry;
    uint32_t update_flags;  //for update_routing_entry only
    uint64_t bytes;  //for MIB
    uint32_t f_hit;  //only for test_and_clear_hit_stat
    uint8_t  collision_flag; // 1 mean the entry is in collsion table or no hashed table, like ASE/Danube
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    int32_t hash_val;
    uint32_t f_tc_remark_enable;
    uint32_t tc_remark;
    uint16_t dest_subif_id;
    uint16_t flags;
    uint8_t hi_prio;
    uint32_t   sessionAction; /* Pointer to session action */
    uint8_t     nFlowId; /* FlowId */
#endif
} PPE_ROUTING_INFO;


/*!
    \brief This is the data structure to get the PORT queue mib for IOCTL.
*/
typedef struct {
    PPA_PORT_MIB mib;  /*!< port mib counter. */
    uint32_t   flags;    /*!< reserved for future */
} PPA_CMD_PORT_MIB_INFO;

typedef struct{
    uint32_t f_is_lan;
    uint32_t entry_num;
    uint32_t mc_entry_num;
    uint32_t f_ip_verify;
    uint32_t f_tcpudp_verify;
    uint32_t f_tcpudp_err_drop;
    uint32_t f_drop_on_no_hit;
    uint32_t f_mc_drop_on_no_hit;
    uint32_t flags; 
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint8_t f_mpe_route;
    uint8_t f_l2tp_ds;
    uint8_t f_capwap_ds;
    uint8_t f_mc_vaps;
#endif
} PPE_ROUTING_CFG;

typedef struct{
    uint32_t entry_num;
    uint32_t br_to_src_port_mask; 
    uint32_t br_to_src_port_en;
    uint32_t f_dest_vlan_en;
    uint32_t f_src_vlan_en;
    uint32_t f_mac_change_drop;
    uint32_t flags;
} PPE_BRDG_CFG;

typedef struct{
    uint32_t mode;
    uint32_t flags;
} PPE_FAST_MODE_CFG;

typedef struct {
    uint32_t        f_is_lan;  
    uint32_t        f_enable; 
    uint32_t        flags;
} PPE_ACC_ENABLE;

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE

typedef struct {
    uint8_t        session_mib_unit;  
} PPE_MIB_MODE_ENABLE;

#endif

typedef struct {
    uint32_t uc_dest_list;
    uint32_t mc_dest_list;
    uint32_t if_no;
} PPE_DEST_LIST;

typedef struct {
    uint32_t if_no;
    uint32_t f_eg_vlan_insert;
    uint32_t f_eg_vlan_remove;
    uint32_t f_ig_vlan_aware;
    uint32_t f_ig_src_ip_based;
    uint32_t f_ig_eth_type_based;
    uint32_t f_ig_vlanid_based;
    uint32_t f_ig_port_based;
    uint32_t f_eg_out_vlan_insert;
    uint32_t f_eg_out_vlan_remove;
    uint32_t f_ig_out_vlan_aware;
} PPE_BRDG_VLAN_CFG;


typedef struct {
    uint32_t entry;  /*so far it is only for get command*/
    uint32_t ig_criteria_type;
    uint32_t ig_criteria;;
    uint32_t new_vci;
    uint32_t dest_qos;
    PPE_OUT_VLAN_INFO out_vlan_info;
    uint32_t in_out_etag_ctrl;
    uint32_t vlan_port_map;
} PPE_BRDG_VLAN_FILTER_MAP;

typedef struct {
    uint32_t dest_ip_compare;
    uint32_t src_ip_compare;
    uint32_t f_vlan_ins_enable;
    uint32_t new_vci;
    uint32_t f_vlan_rm_enable;
    uint32_t f_src_mac_enable;
    uint32_t src_mac_ix;
    uint32_t pppoe_mode;
    uint32_t f_out_vlan_ins_enable;
    uint32_t f_tunnel_rm_enable;
    PPE_OUT_VLAN_INFO out_vlan_info;
    PPE_IPV6_INFO dst_ipv6_info;
    PPE_IPV6_INFO src_ipv6_info;
    uint32_t f_out_vlan_rm_enable;
    uint32_t f_new_dscp_enable;
    uint32_t new_dscp;
    uint32_t dest_qid;
    uint32_t dest_list;
    uint32_t route_type;
    uint32_t p_entry;
    uint64_t bytes;
    uint32_t f_hit;         //only for test_and_clear_hit_stat
    uint32_t update_flags;  //only for update only,not for new added one
#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    uint8_t  sample_en;   /*!< rtp flag */
    uint32_t rtp_pkt_cnt;  /*!< RTP packet mib */
    uint32_t rtp_seq_num;  /*!< RTP sequence number */
#endif
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    int32_t	hash_val;
    uint32_t	f_ipv6;
    uint16_t	src_port;
    uint16_t	dst_port;
    uint16_t	group_id; //multicast grout id passed from mcastd
    uint16_t	subif_id;
    uint8_t	num_vap;
    uint8_t 	src_mac[PPA_ETH_ALEN];  
    PPE_TUNNEL_INFO tnnl_info;
    uint32_t	mtu;
    uint16_t	flags; 	//input FLAG_SESSION_HI_PRIO
			//output FLAG_SESSION_SWAPPED
    uint16_t dest_subif[16];
    uint32_t sessionAction;
#endif	
} PPE_MC_INFO;

typedef struct {
    uint32_t port;
    uint8_t  mac[PPA_ETH_ALEN];
    uint32_t f_src_mac_drop;
    uint32_t dslwan_qid;
    uint32_t dest_list;
    uint32_t p_entry;
    uint32_t f_hit; //only for test_and_clear_bridging_hit_stat
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint32_t fid;
    int32_t  age_timer;
    uint16_t static_entry;
    uint16_t sub_ifid;
#endif 
} PPE_BR_MAC_INFO;


typedef struct {
    uint16_t brid;
    uint32_t port;
    uint16_t vid;
    uint16_t index;
} PPA_BR_PORT_INFO;

struct ppe_itf_mib {
    uint32_t             ig_fast_brg_pkts;           // 0 bridge ?
    uint32_t             ig_fast_brg_bytes;          // 1 ?

    uint32_t             ig_fast_rt_ipv4_udp_pkts;   // 2 IPV4 routing
    uint32_t             ig_fast_rt_ipv4_tcp_pkts;   // 3
    uint32_t             ig_fast_rt_ipv4_mc_pkts;    // 4
    uint32_t             ig_fast_rt_ipv4_bytes;      // 5

    uint32_t             ig_fast_rt_ipv6_udp_pkts;   // 6 IPV6 routing
    uint32_t             ig_fast_rt_ipv6_tcp_pkts;   // 7
    uint32_t             res0;                       // 8
    uint32_t             ig_fast_rt_ipv6_bytes;      // 9

    uint32_t             res1;                       // A
    uint32_t             ig_cpu_pkts;
    uint32_t             ig_cpu_bytes;

    uint32_t             ig_drop_pkts;
    uint32_t             ig_drop_bytes;

    uint32_t             eg_fast_pkts;
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    uint32_t		 ig_fast_rt_ipv6_mc_pkts;
    uint32_t		 eg_fast_bytes;
#endif
};


typedef struct {
    uint32_t itf ; //port
    struct ppe_itf_mib mib;
    uint32_t flag;
} PPE_ITF_MIB_INFO;

typedef struct {
    uint32_t f_enable ; 
    uint32_t flags ; 
} PPE_ENABLE_CFG;

typedef struct {
    int32_t num ; 
    uint32_t flags ; 
} PPE_COUNT_CFG;

typedef struct {
    uint32_t vfitler_type;
    int32_t num ; 
    uint32_t flags ; 
} PPE_VFILTER_COUNT_CFG;

typedef struct {    
    PPA_MULTIFIELD_FLOW_INFO multifield_info;
    uint32_t entry;
    uint32_t flag;
} PPE_MULTIFILED_FLOW;

typedef struct {
    uint32_t portid;
    uint32_t num ; 
    uint32_t flags ; 
} PPE_QOS_COUNT_CFG;

typedef struct {
    char                            *ifname;/*!< Interface name on which Queue is to be added*/
    uint32_t portid;
    uint32_t queueid;
    PPA_QOS_MIB mib;
    uint32_t reg_addr; 
    uint32_t flag;
} PPE_QOS_MIB_INFO;

typedef struct {
    uint32_t portid;
    uint32_t queueid;
    uint32_t weight_level;
    uint32_t flag;
} PPE_QOS_WFQ_CFG;

typedef struct {
    uint32_t portid;
    uint32_t f_enable;
    uint32_t flag;
} PPE_QOS_ENABLE_CFG;

typedef struct {
    uint32_t portid;
    uint32_t queueid;
#if defined(MBR_CONFIG) && MBR_CONFIG
    int32_t  shaperid;
#endif
    uint32_t rate_in_kbps;
    uint32_t burst;
    uint32_t flag;
} PPE_QOS_RATE_SHAPING_CFG;


typedef struct {
    uint32_t vid;
} PPE_WAN_VID_RANGE;

typedef struct {
    PPA_IFNAME ifname[PPA_IF_NAME_SIZE];     /*!< Name of the stack interface */
    uint32_t    if_flags;   /*!< Flags for Interface. Valid values are below: PPA_F_LAN_IF and PPA_F_WAN_IF */
    uint32_t    port;   /*!< physical port id  for this Interface. Valid values are below: 0 ~  */
} PPE_IFINFO;

typedef struct {
    uint8_t lan_flag; // 1 means lan port, 0 means wan ports
    uint32_t physical_port;
    uint32_t old_lan_flag;  // 1 means lan port, 0 means wan ports
} PPE_WANITF_CFG;


typedef struct {
    uint8_t f_is_lan; // input: 1 means lan port, 0 means wan ports
    uint32_t src_ip;  // input:
    uint32_t src_port; //input:
    uint32_t dst_ip;   // input:
    uint32_t dst_port; // input:
    uint32_t hash_index;  //output
    uint32_t hash_table_id; //output mainly reserved for future GRX500 since LAN/WAN will share same hash table
    uint32_t flag; //reserved
} PPE_SESSION_HASH;


#define PPA_CMD_DBG_TOOL_LOW_MEM_TEST 1   /*!< LOW Memory Test  */

typedef union 
{        
    uint32_t size;
    uint32_t reserved;
} DBG_TOOL_VALUE;

typedef struct PPA_CMD_DBG_TOOL_INFO
{
     uint32_t flag; /*reserved */
     uint32_t mode;
     DBG_TOOL_VALUE value; 
     
} PPA_CMD_DBG_TOOL_INFO;

/*!
    \brief This is the data structure for PPA IOCTL set/get variable value
*/
typedef struct PPA_CMD_VARIABLE_VALUE_INFO
{
     uint32_t flag; /*!< reverved for future */
     uint32_t id;    /*!< variable id */
     int32_t value; /*!< variable value */
     
} PPA_CMD_VARIABLE_VALUE_INFO;

#endif  //end of NO_DOXY

/*!
    \brief This is the data structure for PPA Session Summary
*/

typedef struct PPA_SESSION_SUMMARY
{
    uint32_t count[1]; /*!< just for memory holder, it needs to allocate memory by application itself */
} PPA_SESSION_SUMMARY;


/*!
    \brief This is the data structure for PPA IOCTL Session Summary
*/
typedef struct PPA_CMD_SESSION_SUMMARY_INFO
{
     uint32_t flag; /*!< reverved for future */
     uint32_t hash_max_num;  /*!< maximum session number which memory has beeen allocated in below hash_info buffer */
     PPA_SESSION_SUMMARY hash_info; /*!< detail hash usage information */
} PPA_CMD_SESSION_SUMMARY_INFO;



/*!
    \brief This is the data structure for PPA QOS status
*/
typedef struct
{
    uint32_t qos_queue_portid; /*!<  the port id which support qos. at present, only one port can support QOS at run time */

    //port's qos status
    uint32_t time_tick     ; /*!<    number of PP32 cycles per basic time tick */
    uint32_t overhd_bytes  ; /*!<    number of overhead bytes per packet in rate shaping */
    uint32_t eth1_eg_qnum  ; /*!<    maximum number of egress QoS queues; */        
    uint32_t eth1_burst_chk; /*!<    always 1, more accurate WFQ */    
    uint32_t eth1_qss      ; /*!<    1-FW QoS, 0-HW QoS */
    uint32_t shape_en      ; /*!<    1-enable rate shaping, 0-disable */
    uint32_t wfq_en        ; /*!<    1-WFQ enabled, 0-strict priority enabled */
	
    uint32_t tx_qos_cfg_addr; /*!<  qos cfg address */
    uint32_t pp32_clk;   /*!<  pp32 clock  */
    uint32_t basic_time_tick; /*!<  pp32 qos time tick  */
	
    uint32_t wfq_multiple; /*!<  qos wfq multipler  */
    uint32_t wfq_multiple_addr; /*!<  qos wfq multipler address  */
	
    uint32_t wfq_strict_pri_weight; /*!<  strict priority's weight value */
    uint32_t wfq_strict_pri_weight_addr; /*!<  strict priority's weight address  */	

    uint32_t    wan_port_map;  /*!<  wan port interface register value  */
    uint32_t    wan_mix_map;   /*!<  mixed register value  */


    PPA_QOS_INTERVAL qos_port_rate_internal;   /*!<  internal qos port parameters  */
    PPA_QOS_INTERVAL ptm_qos_port_rate_shaping[4]; /*!< internal ptm qos port rate shaping parameters */

    uint32_t max_buffer_size;  /*!<  must match with below arrays, ie, here set to 8 */
    PPE_QOS_MIB_INFO  mib[PPA_MAX_QOS_QUEUE_NUM];   /*!<  Qos mib buffer */
    PPA_QOS_INTERVAL queue_internal[PPA_MAX_QOS_QUEUE_NUM];   /*!<  internal qos queue parameters */
    PPA_QOS_DESC_CFG_INTERNAL desc_cfg_interanl[PPA_MAX_QOS_QUEUE_NUM];/*!<  internal desc cfg parameters */
    uint32_t res;   /*!<  res flag for qos status succeed or not: possible value is PPA_SUCCESS OR PPA_FAILURE  */
  
} PPA_QOS_STATUS;


/*!
    \brief This is the data structure for PPA QOS to get the status
*/
typedef struct {
    PPA_QOS_STATUS qstat; /*!< qos status buffer */
    uint32_t        flags;    /*!<  Reserved currently */
} PPA_CMD_QOS_STATUS_INFO;

/*!
    \brief This is the data structure for PPE fastpath enabled info 
*/
typedef struct {
    uint32_t        ppe_fastpath_enable;  /*!< enable/disable ppe fastpath enable flag */
    uint32_t        flags;   /*!< reserved for future */
} PPA_CMD_PPE_FASTPATH_ENABLE_INFO;

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
/*!
    \brief This is the data structure for SW fastpath enabled info 
*/
typedef struct {
    uint32_t        sw_fastpath_enable;  /*!< enable/disable ppe fastpath enable flag */
    uint32_t        flags;   /*!< reserved for future */
} PPA_CMD_SW_FASTPATH_ENABLE_INFO;

/*!
    \brief This is the data structure for SW session enabled info 
*/
typedef struct {
    uint32_t        session; /*!< PPA SESSION pointer. Note, here we just use its address to enable a sw session for ioctl*/
//    uint32_t        hash; /* Hash value of the session */
    uint32_t        sw_session_enable;  /*!< enable/disable sw session enable flag */
    uint32_t        flags;   /*!< reserved for future */
} PPA_CMD_SW_SESSION_ENABLE_INFO;
#endif

/*!
    \brief Enum for List of Engines that can perform QoS
*/
typedef enum {
        PPA_QOS_RESOURCE_CPU,
        PPA_QOS_RESOURCE_FW,
        PPA_QOS_RESOURCE_HW,
        PPA_QOS_RESOURCE_PPA
} PPA_QOS_RESOURCE;

/*!
    \brief Enum for Module type to be configured in PPA Engine
*/
typedef enum ppa_qos_module_type {
        PPA_QOS_MODULE_CLASSIFIER,
        PPA_QOS_MODULE_QUEUE,
        PPA_QOS_MODULE_BOTH
} PPA_QOS_MODULE_TYPE;

/*!
    \brief Engine Selector Capabilities Structure
*/
typedef struct {
        uint32_t q_max; /* Max number of queues supported by the engine */
        uint32_t q_cap; /* Bitmap to indicate various queueing capabilities supported by the engine */
        uint32_t q_len_max; /* Max queue length in Bytes supported by the engine */
        uint32_t cl_max; /* Max number of classifiers supported by the engine */
        uint32_t cl_cap; /* Bitmap to indicate various classifying capabilities supported by the engine */
} PPA_QOS_ENG_CAP;

/*!
    \brief QoS Queue drop Mode Configuration
*/
typedef enum {
        PPA_QOS_DROP_TAIL=0,
        PPA_QOS_DROP_RED=1,
        PPA_QOS_DROP_WRED=2,
        PPA_QOS_DROP_CODEL=3
}PPA_QOS_DROP_MODE;

/*!
    \brief QoS WRED Drop configuration structure
    \remark If RED, then this is same as WRED Curve 0 only configuration
*/
typedef struct {
        uint32_t        weight; /*!< Weighting factor for QAVG calculation */
        uint32_t        min_th0; /*!< Min Threshold for WRED Curve 0 in % of qlen */
        uint32_t        max_th0; /*!< Max Threshold for WRED Curve 0 in % of qlen */
        uint32_t        max_p0; /*!< Max Drop Probability % at max_th0 for WRED Curve 0 */
        uint32_t        min_th1; /*!< Min Threshold for WRED Curve 1 in % of
                                  qlen*/
        uint32_t        max_th1; /*!< Max Threshold for WRED Curve 1 in % of
                                  qlen*/
        uint32_t        max_p1; /*!< Max Drop Probability % at max_th1 for WRED Curve 1 */
}PPA_QOS_WRED;

/*!
    \brief QoS Queue Drop configuration structure
*/
typedef struct {
        int32_t                 enable; /*!< Whether shaper is enabled */
        PPA_QOS_DROP_MODE         mode; /*!< Mode of Queue Drop - Taildrop, WRED
                                       */
        PPA_QOS_WRED              wred; /*!< WRED configuration of the queue */


}PPA_QOS_DROP_CFG;

/*!
    \brief Enum for QoS Shaper Mode Configuration
*/
typedef enum {
        PPA_QOS_SCHED_SP=0,
        PPA_QOS_SCHED_WFQ=1,
}PPA_QOS_QSCHED_MODE;

/*!
    \brief QoS Modify Queue Configuration structure
*/
typedef struct{
	uint32_t	enable; /*!< Whether Queue is enabled */
	int32_t		weight; /*!< WFQ Weight */
	int32_t		priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t		qlen; /*!< Length of Queue in bytes */
	PPA_QOS_DROP_CFG	drop; /*!< Queue Drop Properties */
	PPA_QOS_SHAPER_CFG				shaper;/*!< Queue Shaper Properties*/
} PPA_CMD_QOS_MOD_QUEUE_CFG;


/*!
    \brief QoS Queue Add configuration info structure
*/
typedef struct{
	char				*ifname;/*!< Interface name on which Queue is to be added*/
	int32_t				queue_num; /*!< Logical queue number added will be filled up in this field */
	int32_t				weight; /* Weight of the Queue being added */ //Added: venu
	int32_t				priority; /* Priority of the Queue being added */ //Added: venu
      uint32_t 					portid;/*!< Portid*/
	PPA_QOS_SHAPER_CFG				shaper;/*!< Queue Shaper Properties*/
	uint32_t 			flags;/*!< Flags for future use*/

}PPA_CMD_QOS_ADD_QUEUE_INFO;

/*!
    \brief QoS Queue Modify configuration info structure
*/
typedef struct{
	char						*ifname;/*!< Interface name on which the Queue is modified*/
	PPA_CMD_QOS_MOD_QUEUE_CFG		*q;/*!< Modify Queue config*/	
	uint32_t 					flags;/*!< Flags for future use*/

}PPA_CMD_QOS_MOD_QUEUE_INFO;

/*!
    \brief QoS Queue Delete configuration info structure
*/
typedef struct{
	char						*ifname;/*!< Interface name on which Queue is being deleted*/
	uint32_t					queueid;/*!< Queue id*/
	uint32_t 					portid;/*!< Portid*/

}PPA_CMD_QOS_DEL_QUEUE_INFO;

/*!
    \brief QoS Queue common configuration info structure for ADD/MODIFY/DEL operation
*/
typedef struct{
	PPA_IFNAME					ifname[PPA_IF_NAME_SIZE];/*!< Interface name on which the Queue is modified*/
	char						tc_map[MAX_TC_NUM]; /*!< Which all Traffic Class(es) map to this Queue */
	uint8_t						tc_no; /*!< Number of Traffic Class(es) map to this Queue */
	uint8_t						flowId_en; /*!<Enable/Disable for flow Id + tc bits used for VAP's & Vlan interfaces*/
	uint16_t					flowId; /*!<flow Id + tc bits used for VAP's & Vlan interfaces*/
	uint32_t					enable; /*!< Whether Queue is enabled */
	int32_t						weight; /*!< WFQ Weight */
	int32_t						priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t						qlen; /*!< Length of Queue in bytes */
	int32_t						queue_num; /*!< Logical queue number added will be filled up in this field */
	int32_t						shaper_num; /*!< Logical queue number added will be filled up in this field */
      	uint32_t 					portid;/*!< Portid*/
	PPA_QOS_DROP_CFG				drop; /*!< Queue Drop Properties */
	PPA_QOS_SHAPER_CFG				shaper;/*!< Queue Shaper Properties*/
	PPA_QOS_QSCHED_MODE				sched;/*!< Queue Scheduler Properties*/
	uint32_t 					flags;/*!< Flags for future use*/

}PPA_CMD_QOS_QUEUE_INFO;


/*!
    \brief QoS subif port info structure 
*/
typedef struct{
        PPA_IFNAME                                      ifname[PPA_IF_NAME_SIZE];/*!< Interface name on which the Queue is modified*/
	int32_t 					port_id; /*!< Portid*/
	uint32_t        				priority_level; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	uint32_t 					weight; /*!< WFQ Weight */
	uint32_t					flags; /*!< Flags for future use*/

}PPA_CMD_SUBIF_PORT_INFO; 

/*!
    \brief QoS HAL Rate Shaping configuration structure
*/
typedef struct {
    PPA_IFNAME ifname[PPA_IF_NAME_SIZE];/*!< ifname of the interface on which the Queues & its shapers exist */
    uint32_t portid;/*!< Port Id corresponding the interface/netif on which Queues/shapers exist */
    uint32_t queueid;/*!< Logical queue id while creating queue/physical queue id passed to TMU while shaper->queue assign */
#if defined(MBR_CONFIG) && MBR_CONFIG
    int32_t  shaperid;/*!< Logical shaper id while creating shaper/physical shaper id passed to TMU while shaper->queue assign */
#endif
    int32_t phys_shaperid;/*!< Physical shaper ID returned by TMU when a shaper is created*/
    PPA_QOS_SHAPER_CFG shaper;/*!< Shaper Info*/
    uint32_t rate_in_kbps;/*!< Peak rate in kbps*/
    uint32_t burst;/*!< Peak burst in kbps*/
    uint32_t flag;/*!< Flags for further use */
} QOS_RATE_SHAPING_CFG;

/*!
    \brief QoS Queue HAL DEL configuration structure
*/
typedef struct qos_q_del_cfg{
	char						*ifname;/*!< Interface name on which the Queue is modified*/
	int32_t						priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t						q_id; /*!< Original Queue id of the queue to be deleted */
      	uint32_t 					portid;/*!< Portid*/
	uint32_t 					flags;/*!< Flags for future use*/

}QOS_Q_DEL_CFG;

/*!
    \brief QoS Queue HAL ADD configuration structure
*/
typedef struct qos_q_add_cfg{
	char						*ifname;/*!< Interface name on which the Queue is modified*/
      	char	 					tc_map[MAX_TC_NUM];/*!< Which all Traffic Class(es) map to this Queue */
	uint8_t						tc_no; /*!< Number of Traffic Class(es) map to this Queue */
      	uint8_t 					intfId_en;/*!<Enable/Disable for flow Id + tc bits used for VAP's & Vlan interfaces*/
	int32_t						weight; /*!< WFQ Weight */
	int32_t						priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t						qlen; /*!< Length of Queue in bytes */
	int32_t						q_id; /*!< Original Queue id of the Queue added */
      	uint32_t 					portid;/*!< Portid*/
      	uint16_t 					intfId;/*!< flow Id + tc bits used for VAP's & Vlan interfaces*/
	/* PPA_QOS_QSCHED_MODE may not be required as tmu hal can internaly find this out : Need to remove later*/
	PPA_QOS_QSCHED_MODE				q_type;/*!< Scheduler type */
	PPA_QOS_DROP_CFG				drop; /*!< Queue Drop Properties */
	uint32_t 					flags;/*!< Flags for future use*/

} QOS_Q_ADD_CFG;

/*!
    \brief QoS Queue HAL MOD configuration structure
*/
typedef struct qos_q_mod_cfg{
	char						*ifname;/*!< Interface name on which the Queue is modified*/
	uint32_t					enable; /*!< Whether Queue is enabled */
	int32_t						weight; /*!< WFQ Weight */
	int32_t						priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t						qlen; /*!< Length of Queue in bytes */
	int32_t						q_id; /*!< Original Queue id of the Queue modified */
      	uint32_t 					portid;/*!< Portid*/
	PPA_QOS_QSCHED_MODE				q_type;/*!< Scheduler type */
	PPA_QOS_DROP_CFG				drop; /*!< Queue Drop Properties */
	uint32_t 					flags;/*!< Flags for future use*/

}QOS_Q_MOD_CFG;

/*!
    \brief QoS Modify QOS Sub interface to Port configuration structure
*/
typedef struct qos_mod_subif_port_cfg{
	char						ifname[PPA_IF_NAME_SIZE];/*!< Interface name on which the Queue is modified*/
      	uint32_t 					port_id;/*!< Portid*/
	int32_t						priority_level; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t						weight; /*!< WFQ Weight */
	uint32_t 					flags;/*!< Flags for future use*/

} QOS_MOD_SUBIF_PORT_CFG;


/*!
    \brief PPA API QoS Add Shaper Configuration structure
*/
typedef struct ppa_qos_add_shaper_cfg {
        PPA_QOS_SHAPER_MODE       mode; /*!< Mode of Token Bucket shaper */
	uint32_t		enable; /*!< Enable for Shaper */
        uint32_t                cir; /*!< Committed Information Rate in bytes/s */
        uint32_t                cbs; /*!< Committed Burst Size in bytes */
        uint32_t                pir; /*!< Peak Information Rate in bytes/s */
        uint32_t                pbs; /*!< Peak Burst Size */
        uint32_t                flags; /*!< Flags define which shapers are enabled
                                                - QOS_SHAPER_F_PIR
                                                - QOS_SHAPER_F_CIR */
	uint32_t 	phys_shaperid;/*!< Physical Queue id of Queue that is added*/
	#ifdef CONFIG_PPA_PUMA7
        char                   ifname[PPA_IF_NAME_SIZE];/*!< Interface name on which the shaper is created*/
	#endif
} PPA_QOS_ADD_SHAPER_CFG;

/*!
    \brief PPA API QoS Add Queue Configuration structure
*/
typedef struct ppa_qos_add_queue_cfg {
	char		tc_map[MAX_TC_NUM];/*!< Which all Traffic Class(es) map to this Queue  */
	uint8_t		tc_no; /*!< Number of Traffic Class(es) map to this Queue */
      	uint8_t 	intfId_en;/*!<Enable/Disable for flow Id + tc bits used for VAP's & Vlan interfaces*/
	uint16_t	intfId;/*!< flow Id + tc bits used for VAP's & Vlan interfaces */
	uint32_t	portid;/*!< PORT ID */
	int32_t		weight; /*!< WFQ Weight */
	int32_t		priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	int32_t		qlen; /*!< Length of Queue in bytes */
        PPA_QOS_QSCHED_MODE        q_type; /*!< QoS scheduler mode - Priority, WFQ */
	uint32_t 	queue_id;/*!< Physical Queue id of Queue that is added*/
	PPA_QOS_DROP_CFG	drop; /*!< Queue Drop Properties */
} PPA_QOS_ADD_QUEUE_CFG;

/*!
    \brief PPA API QoS Modify Queue Configuration structure
*/
typedef struct ppa_qos_mod_queue_cfg {
	uint32_t		enable; /*!< Whether Queue is enabled */
	int32_t			weight; /*!< WFQ Weight */
	int32_t			priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
	uint32_t	portid;/*!< PORT ID */
        PPA_QOS_QSCHED_MODE        q_type; /*!< QoS scheduler mode - Priority, WFQ */
	int32_t			qlen; /*!< Length of Queue in bytes */
	uint32_t		queue_id; /*!< Physical Queue id of Queue being modified*/
	uint32_t		flags; /*!< Flags for future use*/
	PPA_QOS_DROP_CFG	drop; /*!< Queue Drop Properties */
} PPA_QOS_MOD_QUEUE_CFG;

/*!
    \brief PPA API Common QoS Queue Configuration structure for ADD/MODIFY/DEL operations
*/
typedef struct {
        uint32_t        enable; /*!< Whether Queue is enabled */
        uint32_t        status; /*!< Operational status of the queue. Only
                                  valid on GET */
        char            name[MAX_Q_NAME_LEN]; /*!< Name or Alias of the
                                                  queue, if any */
        uint32_t        owner; /*<!  Enumerated value of who is doing the operation.
                                        Possible owners can be: TR-069, Web GUI, CLI, System, BaseRules */
        uint32_t        flags; /*<! Following flags can be kept per
                                 filter structure -
                - QOS_Q_F_MODIFIED : Modified after being originally added
                - QOS_Q_F_MODIFIED_OTHER : Modified by an entity other than the owner
                - QOS_Q_F_HW     : Set if this Q is in HW
                - QOS_Q_F_INGRESS  : This queue is applied to ingress from the interface */
        uint32_t        queue_id; /*!< Queue Id for other operations */
        char	        tc_map[MAX_TC_NUM]; /*!< Which all Traffic Class(es) map to this Queue */
	uint8_t		tc_no; /*!< Number of Traffic Class(es) map to this Queue */
        /* ?? What about additional qselectors possible in GRX350 */
        int32_t         qlen; /*!< Length of Queue in bytes */
        int32_t         weight; /*!< WFQ Weight */
        int32_t         priority; /*!< Queue Priority or Precedence. Start from 1-16, with 1 as highest priority */
        PPA_QOS_DROP_CFG  drop; /*!< Queue Drop Properties */
        PPA_QOS_QSCHED_MODE        sched; /*!< QoS scheduler mode - Priority, WFQ */
        PPA_QOS_SHAPER_CFG    shaper; /*!< Shaper Configuration */
} PPA_QOS_QUEUE_CFG;


/*!
    \brief PPA API Engine Info Configuration structure
*/
typedef struct {
	uint32_t engine_id; /* to be alloted by engine selector, es can create and pass this id to engine on first invoke, Id can be allotted from enum engines_t */
	int32_t (*init) (PPA_QOS_MODULE_TYPE); /* Callback function to initialize the engine*/
	int32_t (*fini) (PPA_QOS_MODULE_TYPE); /* Callback function to un-initialize the engine*/
	int32_t (*queue_add) (char *, PPA_QOS_QUEUE_CFG *); /* Callback function to add a queue for the given interface in the engine*/
	int32_t (*queue_modify) (char *, PPA_QOS_QUEUE_CFG *); /* Callback function to modify a queue for the given interface in the engine*/
	int32_t (*queue_delete) (char *, PPA_QOS_QUEUE_CFG *); /* Callback function to delete a queue for the given interface in the engine*/
	int32_t (*class_add) (PPA_QOS_QUEUE_CFG *); /* Callback function to add a classifier for the given interface in the engine*/
	int32_t (*class_modify) (PPA_QOS_QUEUE_CFG *); /* Callback function to modify a classifier for the given interface in the engine*/
	int32_t (*class_delete) (PPA_QOS_QUEUE_CFG *); /* Callback function to delete a classifier for the given interface in the engine*/
	//Callback for port shape
	PPA_QOS_ENG_CAP engine_capab; /* engine's capabilities */
	PPA_QOS_RESOURCE used_resource; /* Used resource to achieve the functionality. //each engine shall provide its capabilities and resources needed for that, so that engine selector can use this as well as some other info like number of sessions handeled by a particular resource at present. eg switch can do queueing in hardware and sw engine can do the same using CPU. */
}PPA_CMD_QOS_ENGINE_INFO;

typedef PPA_QOS_SHAPER_CFG PPA_CMD_SHAPER_INFO;

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
// classification api datastructures
typedef enum {
	GSWR_INGRESS=0, 	/*!< GSWIP-R ingress traffic*/
	GSWL_INGRESS=1		/*!< GSWIP-L ingress traffic*/
} ppa_class_devingress_t;

typedef enum {
	CAT_NONE=0,		/*!< PPA category none */
	CAT_FILTER,		/*!< PPA category filter */
	CAT_VLAN,		/*!< PPA category VLAN */
	CAT_FWD,		/*!< PPA category forward */
	CAT_USQOS,		/*!< PPA category US QOS */
	CAT_DSQOS,		/*!< PPA category DS QOS */
	CAT_MGMT,		/*!< PPA category Management */
	CAT_LRO,		/*!< PPA category LRO */
	CAT_TUN,		/*!< PPA category TUNNEL */
	CAT_MAX			/*!< PPA category MAX*/
} ppa_class_category_t;

typedef enum {
	SUBCAT_NONE=0,		/*!< PPA sub category none */
	SUBCAT_WLAN_FILTER,	/*!< PPA category wlan filter */
	SUBCAT_MAX		/*!< PPA category max */
} ppa_class_sub_category_t;
	
typedef enum {
	LEARNING_DISABLE=0,		/*!< PPA class action learning disable */
	LEARNING_REGULAR=1,		/*!< PPA class action learning regular */
	LEARNING_FORCE_NOT = 2,		/*!< PPA class action learning force not*/
	LEARNING_FORCE = 3		/*!< PPA class action learning force */
} ppa_class_action_learning_t;		
	
typedef enum {
	METER_DISABLE = 0,		/*!< PPA class action meter disable */
	METER_REGULAR = 1,		/*!< PPA class action meter regular */
	METER_1 = 2,			/*!< PPA class action meter 1*/
	METER_1_2 = 3			/*!< PPA class action meter 2*/
} ppa_class_action_meter_t;

typedef enum {
	TRAFFIC_CLASS_DISABLE = 0,	/*!< PPA class action trafficclass disable */
	TRAFFIC_CLASS_REGULAR = 1,	/*!< PPA class action trafficclass regular */
	TRAFFIC_CLASS_ALTERNATIVE = 2	/*!< PPA class action trafficclassalternative */
} ppa_class_action_trafficclass_t;	

typedef enum {
	IRQ_DISABLE = 0,		/*!< PPA class action irq diasble */
	IRQ_REGULAR = 1,		/*!< PPA class action irq regulat */
	IRQ_EVENT = 2			/*!< PPA class action irq event */
} ppa_class_action_irq_t;

typedef enum {
	CROSS_STATE_DISABLE = 0,	/*!< PPA class action crossstate disable*/
	CROSS_STATE_REGULAR = 1,	/*!< PPA class action crossstate regular */
	CROSS_STATE_CROSS = 2		/*!< PPA class action crossstate cross*/
} ppa_class_action_crossstate_t;

	
typedef enum {
	FRAME_DISABLE = 0,		/*!< PPA class action criticalframe disable */
	FRAME_REGULAR = 1,		/*!< PPA class action criticalframe regular */
	FRAME_CRITICAL = 2		/*!< PPA class action criticalframe critical*/
} ppa_class_action_criticalframe_t;

typedef enum {
	TIMESTAMP_DISABLE = 0,		/*!< PPA class action timestamp disable */
	TIMESTAMP_REGULAR = 1,		/*!< PPA class action timestamp regular */
	TIMESTAMP_STORED = 2		/*!< PPA class action timestamp stored */
} ppa_class_action_timestamp_t;

typedef enum {
	PORTMAP_DISABLE = 0,			/*!< PPA class action portmap disable */
	PORTMAP_REGULAR = 1,			/*!< PPA class action portmap  regular*/
	PORTMAP_DISCARD = 2,			/*!< PPA class action portmap discard*/
	PORTMAP_CPU = 3,			/*!< PPA class action portmap CPU*/
	PORTMAP_ALTERNATIVE = 4,		/*!< PPA class action portmap alternative*/
	PORTMAP_MULTICAST_ROUTER = 5,		/*!< PPA class action portmap router*/
	PORTMAP_MULTICAST_HW_TABLE = 6,		/*!< PPA class action portmap hw table*/
	PORTMAP_ALTERNATIVE_VLAN = 7,		/*!< PPA class action portmap VLAN*/
	PORTMAP_ALTERNATIVE_STAG_VLAN = 8	/*!< PPA class action portmap STAG VLAN*/
} ppa_class_action_portmap_t;

typedef enum {
	VLAN_DISABLE = 0,		/*!< PPA class action vlan disable*/
	VLAN_REGULAR = 1,		/*!< PPA class action vlan regular*/
	VLAN_ALTERNATIVE = 2		/*!< PPA class action vlan alternative*/
} ppa_class_action_vlan_t;		

typedef enum {
	PORT_FILTER_ACTION_UNUSED = 0,	/*!< PPA class action portfilter disabled*/
	PORT_FILTER_ACTION_1 = 1,	/*!< PPA class action portfilter action 1*/
	PORT_FILTER_ACTION_2 = 2,	/*!< PPA class action portfilter action 2*/
	PORT_FILTER_ACTION_3 = 3,	/*!< PPA class action portfilter action 3*/
	PORT_FILTER_ACTION_4 = 4,	/*!< PPA class action portfilter action 4*/
	PORT_FILTER_ACTION_5 = 5,	/*!< PPA class action portfilter action 5*/
	PORT_FILTER_ACTION_6 = 6	/*!< PPA class action portfilter action 6*/
} ppa_class_action_portfilter_t;

typedef enum {
        IF_CATEGORY_ETHLAN, /*!< Ethernet LAN interface category. */
        IF_CATEGORY_ETHWAN, /*!< Ethernet WAN interface category. */
        IF_CATEGORY_PTMWAN, /*!< xDSL PTM WAN interface category. */
        IF_CATEGORY_ATMWAN, /*!< xDSL ATM WAN interface category. */
        IF_CATEGORY_LTEWAN, /*!< LTE WAN interface category. */
        IF_CATEGORY_WLANDP, /*!< WLAN in Directpath interface category. */
        IF_CATEGORY_WLANNDP, /*!< WLAN in Non-directpath interface category. */
        IF_CATEGORY_LOCAL, /*!< Local interface category. */
        IF_CATEGORY_MAX /*!< Max interface category count. */
} ppa_class_action_iftype_t;

typedef enum {
	PROCESSING_PATH_UNUSED = 0, 	/*!< PPA class action processingpath disabled*/
	PROCESSING_PATH_1 = 1,		/*!< PPA class action processingpath 1*/
	PROCESSING_PATH_2 = 2 		/*!< PPA class action processingpath 2*/
} ppa_class_action_processingpath_t;

typedef enum {
	CROSS_VLAN_DISABLE = 0,		/*!< PPA class action crossvlan disabled*/
	CROSS_VLAN_REGULAR = 1,		/*!< PPA class action crossvlan regular*/
	CROSS_VLAN_CROSS = 2		/*!< PPA class action crossvlan cross*/
} ppa_class_action_crossvlan_t;

typedef GSW_PCE_pattern_t ppa_class_pattern_t; 		/*!< PPA class pattern */

typedef struct {
	ppa_class_action_portfilter_t portfilter;	/*!< PPA class filter action portfilter*/
	ppa_class_action_crossstate_t crossstate;	/*!< PPA class filter action crossstate*/
} ppa_class_filter_action_t;

typedef struct {
	ppa_class_action_vlan_t cvlan;			/*!< PPA class vlan_action cvlan*/
	uint16_t vlan_id;				/*!< PPA class vlan_action vlanid*/
	uint8_t fid;					/*!< PPA class vlan_action fid*/
	ppa_class_action_vlan_t svlan;			/*!< PPA class vlan_action svlan*/
	uint16_t svlan_id;				/*!< PPA class vlan_action svlanid*/
	ppa_class_action_crossvlan_t cross_vlan;	/*!< PPA class vlan_action crossvlan*/
	uint8_t cvlan_ignore;				/*!< PPA class vlan_action cvlan ignore*/
} ppa_class_vlan_action_t;

typedef struct {
	ppa_class_action_learning_t learning;		/*!< PPA class action_fwd learning*/
	uint8_t port_trunk;				/*!< PPA class action_fwd port trunk*/
	ppa_class_action_portmap_t portmap;		/*!< PPA class action_fwd portmap*/
	uint32_t forward_portmap;			/*!< PPA class action_fwd forward_portmap*/
	uint16_t forward_subifid;			/*!< PPA class action_fwd forward_subifid*/
	uint8_t routextid_enable;			/*!< PPA class action_fwd routextid_enable*/
	uint8_t routextid;				/*!< PPA class action_fwd routextid*/
	uint8_t rtdestportmaskcmp;			/*!< PPA class action_fwd rt dest port compare*/
	uint8_t rtsrcportmaskcmp;			/*!< PPA class action_fwd rt src port compare*/
	uint8_t rtdstipmaskcmp;				/*!< PPA class action_fwd rt dest ip compare*/
	uint8_t rtsrcipmaskcmp;				/*!< PPA class action_fwd rt src ip compare*/
	uint8_t rtinneripaskey;				/*!< PPA class action_fwd rt inner ip compare enable*/
	uint8_t rtaccelenable;				/*!< PPA class action_fwd rt acceleration enable*/
	uint8_t rtctrlenable;				/*!< PPA class action_fwd rt ctrl enable*/
	ppa_class_action_processingpath_t processpath;  /*!< PPA class action_fwd processing path*/
} ppa_class_action_fwd_t;

typedef struct {
	ppa_class_action_trafficclass_t trafficclass;	/*!< PPA class action_qos trafficclass*/
	uint8_t alt_trafficclass;			/*!< PPA class action_qos alternate traffic class*/
	ppa_class_action_meter_t meter;			/*!< PPA class action_qos meter*/
	uint8_t meterid;				/*!< PPA class action_qos meter id*/
	ppa_class_action_criticalframe_t criticalframe;	/*!< PPA class action_qos critical frame*/
	uint8_t remark;					/*!< PPA class action_qos remark*/
	uint8_t remarkpcp;				/*!< PPA class action_qos remark pcp*/
	uint8_t new_pcp;				/*!< PPA class action_qos new pcp*/
	uint8_t remark_stagpcp;				/*!< PPA class action_qos remark stag pcp*/
	uint8_t remark_stagdei;				/*!< PPA class action_qos remark stag dei*/
	uint8_t remark_dscp;				/*!< PPA class action_qos remark dscp*/
	uint8_t remark_class;				/*!< PPA class action_qos remark class*/
	uint8_t flowid_enabled;				/*!< PPA class action_qos flowid enable*/
	uint16_t flowid; 				/*!< PPA class action_qos flowid*/
} ppa_class_action_qos_t;

typedef struct {
	ppa_class_action_irq_t irq;			/*!< PPA class action_mgmt irq action*/
	ppa_class_action_timestamp_t timestamp;		/*!< PPA class action_mgmt timestamp action*/
} ppa_class_action_mgmt_t;

typedef struct {
	ppa_class_filter_action_t filter_action;	/*!< PPA filtering actions */
	ppa_class_vlan_action_t vlan_action;		/*!< PPA VLAN actions */
	ppa_class_action_fwd_t fwd_action;		/*!< PPA forwarding actions */
	ppa_class_action_qos_t qos_action;		/*!< PPA QOS actions */
	ppa_class_action_mgmt_t mgmt_action;		/*!< PPA Management actions */
	ppa_class_action_iftype_t iftype;		/*!< PPA Interface type actions */
	uint8_t rmon_action;				/*!< PPA RMON actions */
	uint8_t rmon_id;				/*!< PPA RMON id to be used */
} ppa_class_action_t;

typedef struct ppa_class_rule_t {
	ppa_class_devingress_t in_dev; 		/*!< PPA ingress device id*/
	uint8_t order;				/*!< PPA order within the category selected */
	ppa_class_category_t category;		/*!< PPA category of the classifier rule */
	ppa_class_sub_category_t subcategory;   /*!< PPA subcategory of the classifier rule */
	char owner[32];				/*!< PPA owner of the rule to be configures */
	char rx_if[PPA_IF_NAME_SIZE];		/*!< PPA receive interface name */
	char tx_if[PPA_IF_NAME_SIZE];		/*!< PPA trasmit interface name */
	ppa_class_pattern_t pattern;		/*!< PPA pattern of the rule to be configured */
	ppa_class_action_t action;		/*!< PPA action of the rule to be confiured */
	uint16_t uidx;				/*!< PPA unique ID returned by the classification API upon successful addition of rule */
} PPA_CLASS_RULE;

typedef struct ppa_class_rule_t PPA_CMD_CLASSIFIER_INFO; /*!< PPA classification command parameter */
#endif//GRX500 MACRO
/*!
    \brief This is the data structure for IOCTL
*/
typedef union
{
    PPA_CMD_INIT_INFO               init_info;    /*!< PPA init parameter */      
    PPA_CMD_ENABLE_INFO             ena_info;     /*!< PPA enable parameter */      
    PPA_CMD_MC_ENTRY                mc_entry;     /*!< PPA multicast parameter */ 
    PPA_CMD_MAC_ENTRY               mac_entry;    /*!< PPA mac parameter */ 
    PPA_CMD_BR_IF_VLAN_CONFIG       br_vlan;      /*!< PPA bridge parameter */ 
    PPA_CMD_VLAN_FILTER_CONFIG      vlan_filter;  /*!< PPA vlan filter parameter */ 
    PPA_CMD_VLAN_ALL_FILTER_CONFIG  all_vlan_filter;/*!< PPA get all vlan filter list parameter */ 
    PPA_CMD_IF_MAC_INFO             if_mac;       /*!< PPA interface mac parameter */ 
    PPA_CMD_IFINFO                  if_info;      /*!< PPA interface parameter */ 
    PPA_CMD_IFINFOS                 all_if_info;  /*!< PPA all interface list parameter */ 
    PPA_CMD_MC_GROUP_INFO           mc_add_info;   /*!< PPA add multcast parameter */ 
    PPA_CMD_COUNT_INFO              count_info;   /*!< PPA get count parameter */ 
    PPA_CMD_SESSIONS_INFO           session_info; /*!< PPA unicast session parameter */
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    PPA_CMD_CLASSIFIER_INFO	    class_info; /*!<PPA engine info parameter */
#endif
    /* If not required ADD/MOD/DEL Queue info structures can be removed later.*/
    PPA_CMD_QOS_ADD_QUEUE_INFO	    qos_add_queue_info; /*!<PPA engine info parameter */ 
    PPA_CMD_QOS_MOD_QUEUE_INFO	    qos_mod_queue_info; /*!<PPA engine info parameter */ 
    PPA_CMD_QOS_DEL_QUEUE_INFO	    qos_del_queue_info; /*!<PPA engine info parameter */ 
    PPA_CMD_QOS_QUEUE_INFO	    qos_queue_info; /*!<PPA engine info parameter */
    PPA_CMD_SHAPER_INFO		    qos_shaper_info;/*!<PPA Qos shaper info parameter */
    PPA_CMD_SUBIF_PORT_INFO	    subif_port_info; /*!<PPA Qos subif port info parameter */
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
	PPA_CMD_MANAGE_SESSION			manage_sessions;
	PPA_CMD_SESSION_COUNT_INFO		session_count_info;
	PPA_CMD_GET_SESSIONS_INFO		sessions_info;
    PPA_CMD_SESSIONS_CRITERIA_INFO  session_criteria_info; /*!< PPA session criteria info */ 
#endif
    PPA_CMD_SESSION_EXTRA_ENTRY session_extra_info; /*!< PPA extra session parameter */ 
    PPA_CMD_SESSION_TIMER session_timer_info; /*!< PPA extra session timer parameter */ 
    PPA_CMD_SESSIONS_DETAIL_INFO   detail_session_info; /*!< PPA unicast session detail parameter for add a testing routing acceleration entry into PPE FW directly*/ 
    PPA_CMD_VERSION_INFO            ver;          /*!< PPA version parameter */ 
    PPA_CMD_MC_GROUPS_INFO          mc_groups;    /*!< PPA multicast group parameter */ 
    PPA_CMD_ALL_MAC_INFO            all_br_info;  /*!< PPA all bridge list parameter */ 
    PPA_VLAN_RANGE                  wan_vlanid_range; /*!< PPA wan vlan range parameter */ 
    PPA_CMD_VLAN_RANGES             all_wan_vlanid_range_info; /*!< PPA all wan vlan range list parameter */ 
    PPA_CMD_SIZE_INFO               size_info; /*!< PPA major structure size parameter */ 
    PPA_CMD_BRIDGE_ENABLE_INFO      br_enable_info; /*!< PPA enable/disable bridging mac learning parameter */
#ifdef CONFIG_LTQ_PPA_QOS
    PPA_CMD_QOS_STATUS_INFO        qos_status_info; /*!< PPA qos status parameter */
    PPA_CMD_QUEUE_NUM_INFO          qnum_info;  /*!< PPA qos queue parameter */
    PPA_CMD_QOS_CTRL_INFO           qos_ctrl_info;  /*!< PPA qos control parameter */
    PPA_CMD_QOS_MIB_INFO            qos_mib_info; /*!< PPA qos mib parameter */
#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
    PPA_CMD_RATE_INFO               qos_rate_info;  /*!< PPA qos rate shapping parameter */
#endif  //end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING
#ifdef CONFIG_LTQ_PPA_QOS_WFQ
    PPA_CMD_WFQ_INFO                qos_wfq_info;  /*!< PPA qos wfq parameter */
#endif  //end of CONFIG_LTQ_PPA_QOS_WFQ
#endif  //end of CONFIG_LTQ_PPA_QOS
    PPA_CMD_HOOK_LIST_INFO          hook_list_info;   /*!< PPA all registered hook list */
    PPA_CMD_HOOK_ENABLE_INFO        hook_control_info; /*!< PPA control information: enable/disable */
#ifdef NO_DOXY
    PPA_CMD_READ_MEM_INFO           read_mem_info;    /*!< PPA read memory info */
    PPA_CMD_SET_MEM_INFO            set_mem_info;  /*!< PPA write memory info */
#endif    
#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
    PPA_CMD_ENABLE_MULTIFIELD_INFO  mf_ctrl_info;        /*!< PPA multiple field edting status info */
    PPA_CMD_MULTIFIELD_FLOW_INFO    mf_flow_info;   /*!< PPA multiple field flow info */
#endif //end of CONFIG_LTQ_PPA_MFE

    PPA_CMD_MAX_ENTRY_INFO          acc_entry_info; /*!< PPA maximum entries supported */
    PPA_CMD_PORTID_INFO             portid_info;   /*!< PPA portid from interface name */
    PPA_CMD_DSL_MIB_INFO            dsl_mib_info;  /*!< PPA DSL queue mib counter. At present only drop counter is supported */  
    PPA_CMD_PORT_MIB_INFO           port_mib_info;  /*!< PPA PORT mib counter.  */  
    PPA_CMD_GENERAL_ENABLE_INFO     genernal_enable_info; /*!< General PPA enable/disable info.  */ 
    PPA_CMD_SESSION_SUMMARY_INFO    ppa_hash_info;    /*!<  PPA hash info.  */ 
    PPA_CMD_DBG_TOOL_INFO           dbg_tool_info;   /*!<  PPA debug info.  */ 
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
    PPA_CMD_MIB_MODE_INFO           mib_mode_info;   /*!< MIB mode configuration parameter */      
#endif
#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
    PPA_CMD_CAPWAP_INFO             capwap_add_info;   /*!< PPA CAPWAP parameter */ 
    PPA_CMD_CAPWAP_GROUPS_INFO      capwap_groups_info; /*!< PPA CAPWAP group */ 
#endif
    PPA_CMD_PPE_FASTPATH_ENABLE_INFO	ppe_fastpath_enable_info; /*!< PPA PPE FASTPATH enable parameter */
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
    PPA_CMD_SW_FASTPATH_ENABLE_INFO	sw_fastpath_enable_info;  /*!< PPA SW fastpath enable parameter */
    PPA_CMD_SW_SESSION_ENABLE_INFO  sw_session_enable_info;   /*!< PPA SW session enable parameter */
#endif

    PPA_CMD_VARIABLE_VALUE_INFO          var_value_info; /*!<  PPA VARABILE value.  */ 
} PPA_CMD_DATA;

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR)
typedef struct {
    uint8_t hop_limit; 		/*!< PPA ip6hdr hop limit*/
    uint32_t	saddr[4];	/*!< PPA ip6hdr src address*/
    uint32_t    daddr[4];	/*!< PPA ip6hdr dst address*/
} PPA_IP6HDR;

typedef struct {
    uint32_t saddr;		/*!< PPA ip4hdr src address*/
    uint32_t daddr;		/*!< PPA ip4hdr src address*/
} PPA_IP4HDR;


#if defined(CONFIG_LTQ_PPA_MPE_IP97) && CONFIG_LTQ_PPA_MPE_IP97 
#define PPA_IPSEC_NOT_ADDED                -1
#define PPA_IPSEC_EXISTS                   0 
#define PPA_IPSEC_ADDED                    1 

#define  IP_PROTO_ESP      50
#define	 ESP_HEADER	   73
typedef struct {
    PPA_XFRM_STATE * inbound;
    PPA_XFRM_STATE * outbound;
    sa_direction dir;
    uint32_t  routeindex;
} PPA_IPSEC_INFO;
#endif


typedef union {
    PPA_IP4HDR		ip4_hdr;	/*!< PPA ip4 hdr*/
    PPA_IP6HDR		ip6_hdr;	/*!< PPA ip6 hdr*/
#if defined(L2TP_CONFIG) && L2TP_CONFIG
    PPA_L2TP_INFO	l2tp_hdr;	/*!< PPA l2tp hdr*/
#endif
#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
    PPA_CMD_CAPWAP_INFO	capwap_hdr;	/*!< PPA capwap hdr*/
#endif

#if defined(CONFIG_LTQ_PPA_MPE_IP97) && CONFIG_LTQ_PPA_MPE_IP97 
    PPA_IPSEC_INFO	ipsec_hdr;	/*!< PPA ipsec hdr*/
#endif

} ppa_tunnel_info;

typedef struct {
    int32_t		tunnel_type; 	/*!< PPA tunnel type*/
    ppa_tunnel_info	tunnel_info;	/*!< PPA tunnel info*/ 
    void	       *hal_buffer;	/*!< PPA hal buffer*/
} ppa_tunnel_entry;

typedef struct{
    uint16_t enabled;   /*!< Entry is valid*/
    uint16_t f_ipv6; 	/*!< PPA flag ipv6*/
    IP_ADDR srcip;	/*!< PPA source ip*/ 
    IP_ADDR dstip;	/*!< PPA destination ip*/
    uint32_t srcport;	/*!< PPA source port*/ 
    uint32_t dstport;	/*!< PPA destination port*/
    uint16_t aggr_mtu;	/*!< PPA aggrate mtu*/
    uint32_t timeout;	/*!< PPA timeout*/ 
    uint16_t session_uid[2];	/*!< PPA session id*/
} ppa_lro_entry;
#endif

/*@}*/ /* PPA_IOCTL */


/* -------------------------------------------------------------------------- */
/*                        IOCTL Command Definitions                           */
/* -------------------------------------------------------------------------- */
/** \addtogroup  PPA_IOCTL */
/*@{*/

/**  PPA IOCTL NR values
*/
typedef enum {
PPA_CMD_INIT_NR=0,             /*!< NR for PPA_CMD_INI  */
PPA_CMD_EXIT_NR,               /*!< NR for PPA_CMD_EXIT  */
PPA_CMD_ENABLE_NR,             /*!< NR for PPA_CMD_ENABLE  */  
PPA_CMD_GET_STATUS_NR,         /*!< NR for PPA_CMD_GET_STATUS  */
PPA_CMD_MODIFY_MC_ENTRY_NR,    /*!< NR for PPA_CMD_MODIFY_MC_ENTRY  */
PPA_CMD_GET_MC_ENTRY_NR,       /*!< NR for  PPA_CMD_GET_MC_ENTRY */ 
PPA_CMD_ADD_MAC_ENTRY_NR,      /*!< NR for PPA_CMD_ADD_MAC_ENTRY  */
PPA_CMD_DEL_MAC_ENTRY_NR,      /*!< NR for  PPA_CMD_DEL_MAC_ENTRY */
PPA_CMD_SET_VLAN_IF_CFG_NR,    /*!< NR for PPA_CMD_SET_VLAN_IF_CFG  */
PPA_CMD_GET_VLAN_IF_CFG_NR,    /*!< NR for PPA_CMD_GET_VLAN_IF_CFG  */
PPA_CMD_ADD_VLAN_FILTER_CFG_NR,         /*!< NR for PPA_CMD_ADD_VLAN_FILTER_CFG  */
PPA_CMD_DEL_VLAN_FILTER_CFG_NR,         /*!< NR for PPA_CMD_DEL_VLAN_FILTER_CFG  */
PPA_CMD_GET_ALL_VLAN_FILTER_CFG_NR,     /*!< NR for PPA_CMD_GET_ALL_VLAN_FILTER_CFG  */
PPA_CMD_DEL_ALL_VLAN_FILTER_CFG_NR,     /*!< NR for PPA_CMD_DEL_ALL_VLAN_FILTER_CFG  */
PPA_CMD_SET_IF_MAC_NR,   /*!< NR for PPA_CMD_SET_IF_MAC  */
PPA_CMD_GET_IF_MAC_NR,   /*!< NR for PPA_CMD_GET_IF_MAC */ 
PPA_CMD_ADD_LAN_IF_NR,   /*!< NR for PPA_CMD_ADD_LAN_IF   */
PPA_CMD_ADD_WAN_IF_NR,   /*!< NR for PPA_CMD_ADD_WAN_IF   */
PPA_CMD_DEL_LAN_IF_NR,   /*!< NR for PPA_CMD_DEL_LAN_IF  */
PPA_CMD_DEL_WAN_IF_NR,   /*!< NR for PPA_CMD_DEL_WAN_IF   */
PPA_CMD_GET_LAN_IF_NR,   /*!< NR for PPA_CMD_GET_LAN_IF  */
PPA_CMD_GET_WAN_IF_NR,   /*!< NR for  PPA_CMD_GET_WAN_IF */
PPA_CMD_ADD_MC_NR,       /*!< NR for  PPA_CMD_ADD_MC */
PPA_CMD_GET_MC_GROUPS_NR,           /*!< NR for PPA_CMD_GET_MC_GROUPS  */
PPA_CMD_GET_COUNT_LAN_SESSION_NR,   /*!< NR for  PPA_CMD_GET_COUNT_LAN_SESSION */
PPA_CMD_GET_COUNT_WAN_SESSION_NR,   /*!< NR for  PPA_CMD_GET_COUNT_WAN_SESSION */
PPA_CMD_GET_COUNT_NONE_LAN_WAN_SESSION_NR, /*!< NR for  PPA_CMD_GET_COUNT_NONE_LAN_WAN_SESSION_NR */
PPA_CMD_GET_COUNT_LAN_WAN_SESSION_NR,   /*!< NR for  PPA_CMD_GET_COUNT_LAN_WAN_SESSION */
PPA_CMD_GET_COUNT_MC_GROUP_NR,      /*!< NR for PPA_CMD_GET_COUNT_MC_GROUP  */
PPA_CMD_GET_COUNT_VLAN_FILTER_NR,   /*!< NR for PPA_CMD_GET_COUNT_VLAN_FILTER  */
PPA_CMD_GET_LAN_SESSIONS_NR,        /*!< NR for PPA_CMD_GET_LAN_SESSIONS  */  
PPA_CMD_GET_WAN_SESSIONS_NR,        /*!< NR for PPA_CMD_GET_WAN_SESSIONS  */
PPA_CMD_GET_LAN_WAN_SESSIONS_NR,        /*!< NR for PPA_CMD_GET_WAN_SESSIONS  */
PPA_CMD_GET_VERSION_NR,             /*!< NR for PPA_CMD_GET_VERSION  */
PPA_CMD_GET_COUNT_MAC_NR,           /*!< NR for PPA_CMD_GET_COUNT_MAC  */ 
PPA_CMD_GET_ALL_MAC_NR,             /*!< NR for PPA_CMD_GET_ALL_MAC  */
PPA_CMD_WAN_MII0_VLAN_RANGE_ADD_NR,       /*!< NR for PPA_CMD_WAN_MII0_VLAN_RANGE_ADD  */
PPA_CMD_WAN_MII0_VLAN_RANGE_GET_NR,       /*!< NR for PPA_CMD_WAN_MII0_VLAN_RANGE_GET  */
PPA_CMD_GET_COUNT_WAN_MII0_VLAN_RANGE_NR, /*!< NR for PPA_CMD_GET_COUNT_WAN_MII0_VLAN_RANGE  */
PPA_CMD_GET_SIZE_NR,                      /*!< NR for PPA_CMD_GET_SIZE   */
PPA_CMD_BRIDGE_ENABLE_NR,                 /*!< NR for PPA_CMD_BRIDGE_ENABLE  */
PPA_CMD_GET_BRIDGE_STATUS_NR,             /*!< NR for PPA_CMD_GET_BRIDGE_STATUS  */
PPA_CMD_GET_QOS_QUEUE_MAX_NUM_NR,         /*!< NR for PPA_CMD_GET_QOS_QUEUE_MAX_NUM  */
PPA_CMD_SET_QOS_WFQ_NR,                   /*!< NR for PPA_CMD_SET_QOS_WFQ  */
PPA_CMD_GET_QOS_WFQ_NR,                   /*!< NR for PPA_CMD_SET_QOS_WFQ  */
PPA_CMD_RESET_QOS_WFQ_NR,                 /*!< NR for PPA_CMD_GET_QOS_WFQ  */
PPA_CMD_ENABLE_MULTIFIELD_NR,             /*!< NR for PPA_CMD_ENABLE_MULTIFIELD  */
PPA_CMD_GET_MULTIFIELD_STATUS_NR,         /*!< NR for PPA_CMD_GET_MULTIFIELD_STATUS  */
PPA_CMD_GET_MULTIFIELD_ENTRY_MAX_NR,      /*!< NR for PPA_CMD_GET_MULTIFIELD_ENTRY_MAX  */
PPA_CMD_GET_MULTIFIELD_KEY_NUM_NR,        /*!< NR for  PPA_CMD_GET_MULTIFIELD_KEY_NUM */
PPA_CMD_ADD_MULTIFIELD_NR,                /*!< NR for PPA_CMD_ADD_MULTIFIELD  */
PPA_CMD_GET_MULTIFIELD_NR,                /*!< NR for  PPA_CMD_GET_MULTIFIELD */
PPA_CMD_DEL_MULTIFIELD_NR,                /*!< NR for PPA_CMD_DEL_MULTIFIELD  */
PPA_CMD_DEL_MULTIFIELD_VIA_INDEX_NR,      /*!< NR for PPA_CMD_DEL_MULTIFIELD_VIA_INDEX  */ 
PPA_CMD_GET_HOOK_COUNT_NR,                /*!< NR for PPA_CMD_GET_HOOK_COUNT  */
PPA_CMD_GET_HOOK_LIST_NR,                 /*!< NR for PPA_CMD_GET_HOOK_LIST  */   
PPA_CMD_SET_HOOK_NR,                      /*!< NR for PPA_CMD_SET_HOOK   */ 
PPA_CMD_READ_MEM_NR,                      /*!< NR for  PPA_CMD_SET_MEM */
PPA_CMD_SET_MEM_NR,                       /*!< NR for PPA_CMD_SET_MEM  */
PPA_CMD_SET_CTRL_QOS_WFQ_NR,              /*!< NR for PPA_CMD_SET_CTRL_QOS_WFQ  */
PPA_CMD_GET_CTRL_QOS_WFQ_NR,              /*!< NR for  PPA_CMD_GET_CTRL_QOS_WFQ */
PPA_CMD_SET_CTRL_QOS_RATE_NR,             /*!< NR for PPA_CMD_SET_CTRL_QOS_RATE   */
PPA_CMD_GET_CTRL_QOS_RATE_NR,             /*!< NR for PPA_CMD_GET_CTRL_QOS_RATE  */
PPA_CMD_SET_QOS_RATE_NR,                  /*!< NR for PPA_CMD_SET_QOS_RATE  */ 
PPA_CMD_GET_QOS_RATE_NR,                  /*!< NR for PPA_CMD_GET_QOS_RATE  */ 
PPA_CMD_RESET_QOS_RATE_NR,                /*!< NR for PPA_CMD_RESET_QOS_RATE  */
/*PPA Classification */   
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
PPA_CMD_ADD_CLASSIFIER_NR,              /*NR for PPA_CMD_ADD_CLASSIFIER*/
PPA_CMD_DEL_CLASSIFIER_NR,              /*NR for PPA_CMD_DEL_CLASSIFIER*/
PPA_CMD_GET_CLASSIFIER_NR,              /*NR for PPA_CMD_GET_CLASSIFIER*/
PPA_CMD_MOD_CLASSIFIER_NR,              /*NR for PPA_CMD_MOD_CLASSIFIER*/ 
#endif
PPA_CMD_ADD_QOS_QUEUE_NR, 			/*! NR for PPA_CMD_ADD_QOS_QUEUE */
PPA_CMD_MOD_QOS_QUEUE_NR, 			/*! NR for PPA_CMD_MOD_QOS_QUEUE */
PPA_CMD_DEL_QOS_QUEUE_NR, 			/*! NR for PPA_CMD_DEL_QOS_QUEUE */
PPA_CMD_ENG_QUEUE_INIT_NR, 			/*! NR for PPA_CMD_ENG_QUEUE_INIT */
PPA_CMD_ENG_QUEUE_UNINIT_NR, 			/*! NR for PPA_CMD_ENG_QUEUE_UNINIT */
PPA_CMD_MOD_SUBIF_PORT_NR,			/*! NR for PPA_CMD_MOD_SUBIF_PORT */
#if defined(MBR_CONFIG) && MBR_CONFIG
PPA_CMD_SET_QOS_SHAPER_NR,					/*!< NR for PPA_CMD_SET_QOS_SHAPER	*/ 
PPA_CMD_GET_QOS_SHAPER_NR,					/*!< NR for PPA_CMD_GET_QOS_SHAPER	*/ 
#endif
PPA_CMD_ADD_WMM_QOS_QUEUE_NR, 			/*! NR for PPA_CMD_ADD_WMM_QOS_QUEUE */
PPA_CMD_DEL_WMM_QOS_QUEUE_NR, 			/*! NR for PPA_CMD_DEL_WMM_QOS_QUEUE */
PPA_CMD_GET_QOS_MIB_NR,                   /*!< NR for PPA_CMD_GET_QOS_MIB  */
PPA_CMD_GET_MAX_ENTRY_NR,                 /*!< NR for PPA_CMD_GET_MAX_ENTRY  */
PPA_CMD_GET_PORTID_NR,                         /*!< NR for PPA_GET_CMD_PORTID  */
PPA_CMD_GET_DSL_MIB_NR,                        /*!< NR for PPA_GET_DSL_MIB  */
PPA_CMD_CLEAR_DSL_MIB_NR,                      /*!< NR for PPA_CLEAR_DSL_MIB  */
PPA_CMD_DEL_SESSION_NR,                             /*!< NR for PPA_CMD_DEL_SESSION  */
PPA_CMD_ADD_SESSION_NR,                             /*!< NR for PPA_CMD_ADD_SESSION  */
PPA_CMD_MODIFY_SESSION_NR,                             /*!< NR for PPA_CMD_MODIFY_SESSION  */
PPA_CMD_SET_SESSION_TIMER_NR,                             /*!< NR for PPA_CMD_SET_SESSION_TIMER  */
PPA_CMD_GET_SESSION_TIMER_NR,                             /*!< NR for PPA_CMD_GET_SESSION_TIMER  */
PPA_CMD_GET_PORT_MIB_NR,                        /*!< NR for PPA_GET_PORT_MIB  */
PPA_CMD_CLEAR_PORT_MIB_NR,                      /*!< NR for PPA_CLEAR_PORT_MIB  */
PPA_CMD_SET_HAL_DBG_FLAG_NR,                    /*!< NR for PPA_CMD_SET_HAL_DBG_FLAG  */
PPA_CMD_GET_QOS_STATUS_NR,                       /*!< NR for PPA_CMD_GET_QOS_STATUS  */
PPA_CMD_GET_PPA_HASH_SUMMARY_NR,                 /*!< NR for PPA_CMD_GET_PPA_HASH_SUMMARY  */
PPA_CMD_GET_COUNT_PPA_HASH_NR,                  /*!< NR for PPA_CMD_GET_COUNT_PPA_HASH  */
PPA_CMD_DBG_TOOL_NR,                           /*!< NR for PPA_CMD_DBG_TOOL  */
PPA_CMD_SET_VALUE_NR,                           /*!< NR for PPA_CMD_SET_VALUE  */
PPA_CMD_GET_VALUE_NR,                           /*!< NR for PPA_CMD_GET_VALUE  */
PPA_CMD_SET_PPE_FASTPATH_ENABLE_NR,		/*!< NR for PPA_CMD_SET_PPE_FASTPATH_ENABLE */
PPA_CMD_GET_PPE_FASTPATH_ENABLE_NR,		/*!< NR for PPA_CMD_GET_PPE_FASTPATH_ENABLE */


#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
PPA_CMD_SET_MIB_MODE_NR,      /*!< NR for  PPA_CMD_SET_MIB_MODE */ 
PPA_CMD_GET_MIB_MODE_NR,      /*!< NR for  PPA_CMD_GET_MIB_MODE */ 
#endif

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
PPA_CMD_SET_RTP_NR,            /*!< NR for  PPA_CMD_SET_RTP */ 
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
PPA_CMD_ADD_CAPWAP_NR,       /*!< NR for  PPA_CMD_ADD_CAPWAP */
PPA_CMD_DEL_CAPWAP_NR,       /*!< NR for  PPA_CMD_DEL_CAPWAP */
PPA_CMD_GET_CAPWAP_GROUPS_NR,/*!< NR for  PPA_CMD_GET_CAPWAP_GROUPS */
PPA_CMD_GET_COUNT_CAPWAP_NR,/*!< NR for PPA_CMD_GET_COUNT_CAPWAP_GROUP_NR */
PPA_CMD_GET_MAXCOUNT_CAPWAP_NR,/*!< NR for PPA_CMD_GET_MAXCOUNT_CAPWAP_GROUP_NR */
#endif

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
PPA_CMD_SET_SW_FASTPATH_ENABLE_NR,              /*!< NR for PPA_CMD_SET_SW_FASTPATH_ENABLE */
PPA_CMD_GET_SW_FASTPATH_ENABLE_NR,              /*!< NR for PPA_CMD_GET_SW_FASTPATH_ENABLE */
PPA_CMD_SET_SW_SESSION_ENABLE_NR,               /*!< NR for PPA_CMD_SET_SW_SESSION_ENABLE */
PPA_CMD_GET_SW_SESSION_ENABLE_NR,               /*!< NR for PPA_CMD_GET_SW_SESSION_ENABLE */
#endif
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
PPA_CMD_GET_SESSIONS_CRITERIA_NR,        /*!< NR for PPA_CMD_GET_SESSIONS_CRITERIA  */  
PPA_CMD_SWAP_SESSIONS_NR,        /*!< NR for PPA_CMD_GET_SESSIONS_CRITERIA  */ 
PPA_CMD_MANAGE_SESSIONS_NR,
PPA_CMD_GET_SESSIONS_NR,
PPA_CMD_GET_SESSIONS_COUNT_NR,
#endif

/*  PPA_IOC_MAXNR should be the last one in the enumberation */    
PPA_IOC_MAXNR                            /*!< NR for PPA_IOC_MAXNR  */
}PPA_IOC_NR;

/**  PPA Initialization Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_INIT_INFO The parameter points to a
          \ref PPA_CMD_INIT_INFO structure   
   \return The return value can be any one of the following: \n
            - PPA_SUCCESS \n
            - PPA_FAILURE 
*/
#define PPA_CMD_INIT                            _IOW(PPA_IOC_MAGIC,  PPA_CMD_INIT_NR,  PPA_CMD_INIT_INFO)



/**  PPA Un-init or exit command. Value is manipulated by _IO() macro for final value
   \return The return value can be any one of the following: \n
            - PPA_SUCCESS \n
            - PPA_FAILURE 
*/
#define PPA_CMD_EXIT                            _IO(PPA_IOC_MAGIC,   PPA_CMD_EXIT_NR)

/**  PPA Acceleration Enable / Disable Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_ENABLE_INFO The parameter points to a
          \ref PPA_CMD_ENABLE_INFO structure  
   \return The return value can be any one of the following: \n
            - PPA_SUCCESS \n
            - PPA_FAILURE         
*/
#define PPA_CMD_ENABLE                          _IOW(PPA_IOC_MAGIC,  PPA_CMD_ENABLE_NR,  PPA_CMD_ENABLE_INFO)

/**  PPA Acceleration Get Status Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_ENABLE_INFO The parameter points to a
          \ref PPA_CMD_ENABLE_INFO structure. Enable or disable configuration status.    
*/
#define PPA_CMD_GET_STATUS                      _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_STATUS_NR,  PPA_CMD_ENABLE_INFO)

/**  PPA Modify Multicast session parameters Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MC_ENTRY The parameter points to a
          \ref PPA_CMD_MC_ENTRY structure   
*/
#define PPA_CMD_MODIFY_MC_ENTRY                 _IOW(PPA_IOC_MAGIC,  PPA_CMD_MODIFY_MC_ENTRY_NR,  PPA_CMD_MC_ENTRY)

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE

/**  PPA RTP sampling configuration Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MC_ENTRY The parameter points to a
          \ref PPA_CMD_MC_ENTRY structure   
*/
#define PPA_CMD_SET_RTP                 _IOW(PPA_IOC_MAGIC,  PPA_CMD_SET_RTP_NR,  PPA_CMD_MC_ENTRY)

#endif

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE

/**  PPA Set Unicast/multicast session mib configuration. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MIB_MODE_INFO The parameter points to a
          \ref PPA_CMD_MIB_MODE_INFO structure   
*/
#define PPA_CMD_SET_MIB_MODE                   _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_MIB_MODE_NR,  PPA_CMD_MIB_MODE_INFO)



/**  PPA Get Unicast/multicast session mib configuration. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_MIB_MODE_INFO The parameter points to a
          \ref PPA_CMD_MIB_MODE_INFO structure   
*/
#define PPA_CMD_GET_MIB_MODE                   _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_MIB_MODE_NR,  PPA_CMD_MIB_MODE_INFO)

#endif



/**  PPA Get Multicast session parameters Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_MC_ENTRY The parameter points to a
          \ref PPA_CMD_MC_ENTRY structure   
*/
#define PPA_CMD_GET_MC_ENTRY                    _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_MC_ENTRY_NR,  PPA_CMD_MC_ENTRY)

/**  PPA Add a MAC entry to the bridging MAC table Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MAC_ENTRY The parameter points to a
          \ref PPA_CMD_MAC_ENTRY structure   
*/
#define PPA_CMD_ADD_MAC_ENTRY                   _IOW(PPA_IOC_MAGIC,  PPA_CMD_ADD_MAC_ENTRY_NR,  PPA_CMD_MAC_ENTRY)

/**  PPA Delete a MAC entry to the bridging MAC table Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MAC_ENTRY The parameter points to a
          \ref PPA_CMD_MAC_ENTRY structure   
*/
#define PPA_CMD_DEL_MAC_ENTRY                   _IOW(PPA_IOC_MAGIC,  PPA_CMD_DEL_MAC_ENTRY_NR,  PPA_CMD_MAC_ENTRY)

/**  PPA Set Interface VLAN configuration Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_BR_IF_VLAN_CONFIG The parameter points to a
          \ref PPA_CMD_BR_IF_VLAN_CONFIG structure   
*/
#define PPA_CMD_SET_VLAN_IF_CFG                 _IOW(PPA_IOC_MAGIC,  PPA_CMD_SET_VLAN_IF_CFG_NR,  PPA_CMD_BR_IF_VLAN_CONFIG)

/**  PPA Get All VLAN Filter Configuration Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_BR_IF_VLAN_CONFIG The parameter points to a
          \ref PPA_CMD_BR_IF_VLAN_CONFIG structure   
*/
#define PPA_CMD_GET_VLAN_IF_CFG                 _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_VLAN_IF_CFG_NR,  PPA_CMD_BR_IF_VLAN_CONFIG)

/**  PPA Add VLAN filter configuration Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_VLAN_FILTER_CONFIG The parameter points to a
          \ref PPA_CMD_VLAN_FILTER_CONFIG structure   
*/
#define PPA_CMD_ADD_VLAN_FILTER_CFG             _IOW(PPA_IOC_MAGIC,  PPA_CMD_ADD_VLAN_FILTER_CFG_NR, PPA_CMD_VLAN_FILTER_CONFIG)

/**  PPA Delete VLAN filter configuration Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_VLAN_FILTER_CONFIG The parameter points to a
          \ref PPA_CMD_VLAN_FILTER_CONFIG structure   
*/
#define PPA_CMD_DEL_VLAN_FILTER_CFG             _IOW(PPA_IOC_MAGIC,  PPA_CMD_DEL_VLAN_FILTER_CFG_NR, PPA_CMD_VLAN_FILTER_CONFIG)

/**  PPA Delete all VLAN Filter Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_VLAN_ALL_FILTER_CONFIG The parameter points to a
          \ref PPA_CMD_VLAN_ALL_FILTER_CONFIG structure   
*/
#define PPA_CMD_GET_ALL_VLAN_FILTER_CFG         _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_ALL_VLAN_FILTER_CFG_NR, PPA_CMD_VLAN_ALL_FILTER_CONFIG)

/**  PPA Get All VLAN Filter Configuration Command. Value is manipulated by _IOR() macro for final value
*/
#define PPA_CMD_DEL_ALL_VLAN_FILTER_CFG         _IO(PPA_IOC_MAGIC,   PPA_CMD_DEL_ALL_VLAN_FILTER_CFG_NR)

/**  PA Set Interface MAC address Command. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_IF_MAC_INFO The parameter points to a
          \ref PPA_CMD_IF_MAC_INFO structure   
*/
#define PPA_CMD_SET_IF_MAC                      _IOW(PPA_IOC_MAGIC,  PPA_CMD_SET_IF_MAC_NR, PPA_CMD_IF_MAC_INFO)

/**  PPA Get Interface MAC address Command. Value is anipulated by _IOWR() macro for final value
   \param PPA_CMD_IF_MAC_INFO The parameter points to a
          \ref PPA_CMD_IF_MAC_INFO structure   
*/
#define PPA_CMD_GET_IF_MAC                      _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_IF_MAC_NR, PPA_CMD_IF_MAC_INFO)

/**  PPA Add LAN Interface Command. It is used to register a LAN network interface with the PPA. Value is anipulated by _IOW() macro for final value
   \param PPA_CMD_IFINFO The parameter points to a
          \ref PPA_CMD_IFINFO structure   
*/
#define PPA_CMD_ADD_LAN_IF                      _IOW(PPA_IOC_MAGIC,  PPA_CMD_ADD_LAN_IF_NR, PPA_CMD_IFINFO)

/**  PPA Add WAN interface Command. It is used to register a WN network interface with the PPA. Value is anipulated by _IOW() macro for final value
   \param PPA_CMD_IFINFO The parameter points to a
          \ref PPA_CMD_IFINFO structure   
*/
#define PPA_CMD_ADD_WAN_IF                      _IOW(PPA_IOC_MAGIC,  PPA_CMD_ADD_WAN_IF_NR, PPA_CMD_IFINFO)

/**  A Delete LAN Interface Command. It is used to de-register a LAN network interface. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_IFINFO The parameter points to a
          \ref PPA_CMD_IFINFO structure   
*/
#define PPA_CMD_DEL_LAN_IF                      _IOW(PPA_IOC_MAGIC,  PPA_CMD_DEL_LAN_IF_NR, PPA_CMD_IFINFO)

/**  PA Delete WAN Interface Command. It is used to de-register a WAN network interface. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_IFINFO The parameter points to a
          \ref PPA_CMD_IFINFO structure   
*/
#define PPA_CMD_DEL_WAN_IF                      _IOW(PPA_IOC_MAGIC,  PPA_CMD_DEL_WAN_IF_NR, PPA_CMD_IFINFO)

/**  PA Get all LAN Interface Information Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_IFINFOS The parameter points to a
          \ref PPA_CMD_IFINFOS structure   
*/
#define PPA_CMD_GET_LAN_IF                      _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_LAN_IF_NR, PPA_CMD_IFINFOS)

/**  PPA Get all WAN Interface Information Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_IFINFOS The parameter points to a
          \ref PPA_CMD_IFINFOS structure   
*/
#define PPA_CMD_GET_WAN_IF                      _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_WAN_IF_NR, PPA_CMD_IFINFOS)

/**  PPA Add a Multicast session Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_MC_GROUP_INFO The parameter points to a
          \ref PPA_CMD_MC_GROUP_INFO structure   
*/
#define PPA_CMD_ADD_MC                          _IOR(PPA_IOC_MAGIC,  PPA_CMD_ADD_MC_NR, PPA_CMD_MC_GROUP_INFO)


#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG

/**  PPA Add a CAPWAP Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_CAPWAP_INFO The parameter points to a
          \ref PPA_CMD_CAPWAP_INFO structure   
*/
#define PPA_CMD_ADD_CAPWAP                          _IOR(PPA_IOC_MAGIC, PPA_CMD_ADD_CAPWAP_NR, PPA_CMD_CAPWAP_INFO)


/**  PPA Delete a CAPWAP Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_CAPWAP_INFO The parameter points to a
          \ref PPA_CMD_CAPWAP_INFO structure   
*/
#define PPA_CMD_DEL_CAPWAP                          _IOR(PPA_IOC_MAGIC, PPA_CMD_DEL_CAPWAP_NR, PPA_CMD_CAPWAP_INFO)


/**  PPA Get a CAPWAP Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_CAPWAP_GROUPS_INFO The parameter points to a
          \ref PPA_CMD_CAPWAP_GROUPS_INFO structure   
*/
#define PPA_CMD_GET_CAPWAP_GROUPS                   _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_CAPWAP_GROUPS_NR, PPA_CMD_CAPWAP_GROUPS_INFO)

/**  PPA Get CAPWAP Tunnel counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_CAPWAP              _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_CAPWAP_NR, PPA_CMD_COUNT_INFO)

/**  PPA Get CAPWAP Maximum Tunnel counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_MAXCOUNT_CAPWAP              _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_MAXCOUNT_CAPWAP_NR, PPA_CMD_COUNT_INFO)

#endif



/**  PPA Get a Multicast session Command. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_MC_GROUPS_INFO The parameter points to a
          \ref PPA_CMD_MC_GROUPS_INFO structure   
*/
#define PPA_CMD_GET_MC_GROUPS                   _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_MC_GROUPS_NR, PPA_CMD_MC_GROUPS_INFO)

/**  PPA Get LAN accelerated session counter. Value is manipulated by _IOWR() macro for final value 
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_LAN_SESSION           _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_LAN_SESSION_NR, PPA_CMD_COUNT_INFO)

/**  PPA PPA Get WAN accelerated session counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_WAN_SESSION           _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_WAN_SESSION_NR, PPA_CMD_COUNT_INFO)

/**  PPA PPA Get NON LAN/WAN session counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_NONE_LAN_WAN_SESSION          _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_NONE_LAN_WAN_SESSION_NR, PPA_CMD_COUNT_INFO)

/**  PPA PPA Get LAN/WAN accelerated session counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_LAN_WAN_SESSION           _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_LAN_WAN_SESSION_NR, PPA_CMD_COUNT_INFO)

/**  PPA Get Multicast accelerated  session counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_MC_GROUP              _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_MC_GROUP_NR, PPA_CMD_COUNT_INFO)

/**  PPA Get All VLAN filter count. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_VLAN_FILTER           _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_VLAN_FILTER_NR, PPA_CMD_COUNT_INFO)

/**  PPA PPA Get LAN session information. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SESSIONS_INFO The parameter points to a
          \ref PPA_CMD_SESSIONS_INFO structure   
*/
#define PPA_CMD_GET_LAN_SESSIONS                _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_LAN_SESSIONS_NR, PPA_CMD_SESSIONS_INFO)

/**  PPA Get WAN  session information. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SESSIONS_INFO The parameter points to a
          \ref PPA_CMD_SESSIONS_INFO structure   
*/
#define PPA_CMD_GET_WAN_SESSIONS                _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_WAN_SESSIONS_NR, PPA_CMD_SESSIONS_INFO)

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
/**  PPA PPA Get sessions criteria information. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SESSIONS_CRITERIA The parameter points to a
          \ref PPA_CMD_SESSIONS_CRITERIA structure   
*/
#define PPA_CMD_GET_SESSIONS_CRITERIA                _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_SESSIONS_CRITERIA_NR, PPA_CMD_SESSIONS_CRITERIA)

#define PPA_CMD_SWAP_SESSIONS                _IOWR(PPA_IOC_MAGIC, PPA_CMD_SWAP_SESSIONS_NR, PPA_CMD_SWAP_SESSIONS)

#define	PPA_CMD_MANAGE_SESSIONS 			_IOWR(PPA_IOC_MAGIC, PPA_CMD_MANAGE_SESSIONS_NR, PPA_CMD_MANAGE_SESSION)

#define PPA_CMD_GET_SESSIONS_COUNT			_IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_SESSIONS_COUNT_NR, PPA_CMD_SESSION_COUNT_INFO)

#define PPA_CMD_GET_SESSIONS 				_IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_SESSIONS_NR, PPA_CMD_GET_SESSIONS_INFO)
#endif

/**  PPA Get LAN/WAN session information. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SESSIONS_INFO The parameter points to a
          \ref PPA_CMD_SESSIONS_INFO structure   
*/
#define PPA_CMD_GET_LAN_WAN_SESSIONS             _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_LAN_WAN_SESSIONS_NR, PPA_CMD_SESSIONS_INFO)


/**  PPA ADD routing session. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SESSIONS_DETAIL_INFO The parameter points to a
          \ref PPA_CMD_SESSIONS_DETAIL_INFO structure   
*/
#define PPA_CMD_ADD_SESSION                _IOWR(PPA_IOC_MAGIC, PPA_CMD_ADD_SESSION_NR, PPA_CMD_SESSIONS_DETAIL_INFO)

/**  PPA DEL accelerated routing session. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SESSIONS_INFO The parameter points to a
          \ref PPA_CMD_SESSIONS_INFO structure   
*/
#define PPA_CMD_DEL_SESSION                _IOWR(PPA_IOC_MAGIC, PPA_CMD_DEL_SESSION_NR, PPA_CMD_SESSIONS_INFO)


/**  PPA DEL accelerated routing session. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_MODIFY_SESSION The parameter points to a
          \ref PPA_CMD_MODIFY_SESSION structure   
*/
#define PPA_CMD_MODIFY_SESSION                _IOWR(PPA_IOC_MAGIC, PPA_CMD_MODIFY_SESSION_NR, PPA_CMD_SESSION_EXTRA_ENTRY)

/**  PPA Set accelerated routing polling timer. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_SET_SESSION_TIMER The parameter points to a
          \ref PPA_CMD_SET_SESSION_TIMER structure   
*/
#define PPA_CMD_SET_SESSION_TIMER                _IOWR(PPA_IOC_MAGIC, PPA_CMD_SET_SESSION_TIMER_NR, PPA_CMD_SESSION_TIMER)


/**  PPA Get accelerated routing polling timer. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_GET_SESSION_TIMER The parameter points to a
          \ref PPA_CMD_GET_SESSION_TIMER structure   
*/
#define PPA_CMD_GET_SESSION_TIMER                _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_SESSION_TIMER_NR, PPA_CMD_SESSION_TIMER)



/**  PPA Get PPA subsystem version. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_VERSION_INFO The parameter points to a
          \ref PPA_CMD_VERSION_INFO structure   
*/
#define PPA_CMD_GET_VERSION                     _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_VERSION_NR, PPA_CMD_VERSION_INFO)

/**  PPA Get bridge mac counter. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_MAC                   _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_MAC_NR, PPA_CMD_COUNT_INFO)

/**  PPA Get all mac address value. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_ALL_MAC_INFO The parameter points to a
          \ref PPA_CMD_ALL_MAC_INFO structure   
*/
#define PPA_CMD_GET_ALL_MAC                     _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_ALL_MAC_NR, PPA_CMD_ALL_MAC_INFO )

/**  PPA Add VLAN range for WAN interface in mixed mode. Value is manipulated by _IOR() macro for final value
   \param PPA_VLAN_RANGE The parameter points to a
          \ref PPA_VLAN_RANGE structure   
*/
#define PPA_CMD_WAN_MII0_VLAN_RANGE_ADD         _IOR(PPA_IOC_MAGIC,  PPA_CMD_WAN_MII0_VLAN_RANGE_ADD_NR, PPA_VLAN_RANGE)

/**  PPA Get VLAN range for WAN interface in mixed mode. Value is manipulated by _IOR() macro for final value 
   \param PPA_CMD_VLAN_RANGES The parameter points to a
          \ref PPA_CMD_VLAN_RANGES structure   
*/
#define PPA_CMD_WAN_MII0_VLAN_RANGE_GET         _IOR(PPA_IOC_MAGIC,  PPA_CMD_WAN_MII0_VLAN_RANGE_GET_NR, PPA_CMD_VLAN_RANGES)

/**  PPA Get VLAN range count in mixed mode. Value is manipulated by _IOWR() macro for final value 
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_COUNT_WAN_MII0_VLAN_RANGE   _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_WAN_MII0_VLAN_RANGE_NR, PPA_CMD_COUNT_INFO)

/**  PPA Get some information entry size. It is for internal usage. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_SIZE_INFO The parameter points to a
          \ref PPA_CMD_SIZE_INFO structure   
*/
#define PPA_CMD_GET_SIZE                    _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_SIZE_NR, PPA_CMD_SIZE_INFO)

/**  PPA enable/disable ppa bridge mac learning hooks. Value is manipulated by _IOWR() macro for final value
   \param PPA_CMD_BRIDGE_ENABLE_INFO The parameter points to a
          \ref PPA_CMD_BRIDGE_ENABLE_INFO structure   
*/
#define PPA_CMD_BRIDGE_ENABLE                    _IOW(PPA_IOC_MAGIC, PPA_CMD_BRIDGE_ENABLE_NR, PPA_CMD_BRIDGE_ENABLE_INFO)

/**  PPA get ppa bridge mac learning hooks enable/disabe status. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_GET_BRIDGE_STATUS The parameter points to a
          \ref PPA_CMD_GET_BRIDGE_STATUS structure   
*/
#define PPA_CMD_GET_BRIDGE_STATUS          _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_BRIDGE_STATUS_NR, PPA_CMD_BRIDGE_ENABLE_INFO)

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
// Classification start

/** PPA Add QOS Class. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_CLASSIFIER_INFO The parameter points to a
    \ref PPA_CMD_CLASSIFIER_INFO structure
*/
#define PPA_CMD_ADD_CLASSIFIER _IOWR(PPA_IOC_MAGIC, PPA_CMD_ADD_CLASSIFIER_NR, PPA_CMD_CLASSIFIER_INFO)

/** PPA Modify QOS Class. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_CLASSIFIER_INFO The parameter points to a
    \ref PPA_CMD_CLASSIFIER_INFO structure
*/
#define PPA_CMD_MOD_CLASSIFIER _IOWR(PPA_IOC_MAGIC, PPA_CMD_MOD_CLASSIFIER_NR, PPA_CMD_CLASSIFIER_INFO)

/** PPA Delete QOS Class. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_CLASSIFIER_INFO The parameter points to a
    \ref PPA_CMD_CLASSIFIER_INFO structure
*/
#define PPA_CMD_DEL_CLASSIFIER _IOWR(PPA_IOC_MAGIC, PPA_CMD_DEL_CLASSIFIER_NR, PPA_CMD_CLASSIFIER_INFO)
/** PPA Get QoS Class. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_CLASSIFIER_INFO The parameter points to a
    \ref PPA_CMD_CLASSIFIER_INFO structure
*/      
#define PPA_CMD_GET_CLASSIFIER _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_CLASSIFIER_NR, PPA_CMD_CLASSIFIER_INFO)
// Classification End
#endif

#ifdef CONFIG_LTQ_PPA_QOS
/** PPA GET QOS status. Value is manipulated by _IOR() macro for final value
    \param[out] PPA_CMD_QOS_STATUS_INFO The parameter points to a
                \ref PPA_CMD_QOS_STATUS_INFO structure 
*/
#define PPA_CMD_GET_QOS_STATUS  _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_QOS_STATUS_NR, PPA_CMD_QOS_STATUS_INFO) 


/** PPA GET the maximum queue supported for WFQ/RateShapping. Value is manipulated by _IOR() macro for final value
    \param[out] PPA_CMD_QUEUE_NUM_INFO The parameter points to a
                \ref PPA_CMD_QUEUE_NUM_INFO structure 
    \note: portid is input parameter, and queue_num  is output value . 
*/
#define PPA_CMD_GET_QOS_QUEUE_MAX_NUM  _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_QOS_QUEUE_MAX_NUM_NR, PPA_CMD_QUEUE_NUM_INFO) 

/** PPA GET the QOS mib counter. Value is manipulated by _IOR() macro for final value
    \param[out] PPA_CMD_QUEUE_NUM_INFO The parameter points to a
                \ref PPA_CMD_QUEUE_NUM_INFO structure 
    \note: portid is input parameter, and queue_num  is output value . 
*/
#define PPA_CMD_GET_QOS_MIB  _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_QOS_MIB_NR, PPA_CMD_QOS_MIB_INFO) 



#ifdef CONFIG_LTQ_PPA_QOS_WFQ
/** PPA Enable/Disable QOS WFQ feature. Value is manipulated by _IOW() macro for final value
    \param[in] PPA_CMD_QOS_CTRL_INFO The parameter points to a
                            \ref PPA_CMD_QOS_CTRL_INFO structure 
*/
#define PPA_CMD_SET_CTRL_QOS_WFQ _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_CTRL_QOS_WFQ_NR, PPA_CMD_QOS_CTRL_INFO) 


/** PPA get QOS WFQ feature status: enabled or disabled. Value is manipulated by _IOR() macro for final value
    \param[in] PPA_CMD_QOS_CTRL_INFO The parameter points to a
                            \ref PPA_CMD_QOS_CTRL_INFO structure 
*/
#define PPA_CMD_GET_CTRL_QOS_WFQ _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_CTRL_QOS_WFQ_NR, PPA_CMD_QOS_CTRL_INFO) 

/** PPA Set WFQ weight. Value is manipulated by _IOW() macro for final value
    \param PPA_CMD_WFQ_INFO The parameter points to a
    \ref PPA_CMD_WFQ_INFO structure 
*/
#define PPA_CMD_SET_QOS_WFQ _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_QOS_WFQ_NR, PPA_CMD_WFQ_INFO) 

/** PPA Get WFQ weight. Value is manipulated by _IOR() macro for final value
    \param[out] PPA_CMD_WFQ_INFO The parameter points to a
                     \ref PPA_CMD_WFQ_INFO structure 
    \note portid, queueid and weight should be set accordingly. 
*/
#define PPA_CMD_GET_QOS_WFQ _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_QOS_WFQ_NR, PPA_CMD_WFQ_INFO) 

/** PPA Reset WFQ weight. Value is manipulated by _IOW() macro for final value
    \param[out] PPA_CMD_WFQ_INFO The parameter points to a
                            \ref PPA_CMD_WFQ_INFO structure 
    \note: portid/queueid is input parameter, and weight is output value. 
*/
#define PPA_CMD_RESET_QOS_WFQ _IOW(PPA_IOC_MAGIC, PPA_CMD_RESET_QOS_WFQ_NR, PPA_CMD_WFQ_INFO) 

#endif  //end of CONFIG_LTQ_PPA_QOS_WFQ

#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
/** PPA Enable/Disable QOS Rate Shaping feature. Value is manipulated by _IOW() macro for final value
    \param[in] PPA_CMD_QOS_CTRL_INFO The parameter points to a
                            \ref PPA_CMD_QOS_CTRL_INFO structure 
*/
#define PPA_CMD_SET_CTRL_QOS_RATE _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_CTRL_QOS_RATE_NR, PPA_CMD_QOS_CTRL_INFO) 


/** PPA get QOS Rate Shaping feature status: enabled or disabled. Value is manipulated by _IOR() macro for final value
    \param[in] PPA_CMD_QOS_CTRL_INFO The parameter points to a
                            \ref PPA_CMD_QOS_CTRL_INFO structure 
*/
#define PPA_CMD_GET_CTRL_QOS_RATE _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_CTRL_QOS_RATE_NR, PPA_CMD_QOS_CTRL_INFO) 

/** PPA Set QOS rate shaping. Value is manipulated by _IOW() macro for final value
    \param[in] PPA_CMD_RATE_INFO The parameter points to a
    \ref PPA_CMD_RATE_INFO structure 
*/
#define PPA_CMD_SET_QOS_RATE _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_QOS_RATE_NR, PPA_CMD_RATE_INFO) 

/** PPA Get QOS Rate shaping configuration. Value is manipulated by _IOR() macro for final value
    \param[out] PPA_CMD_RATE_INFO The parameter points to a
                     \ref PPA_CMD_RATE_INFO structure 
    \note portid, queueid and weight should be set accordingly. 
*/
#define PPA_CMD_GET_QOS_RATE _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_QOS_RATE_NR, PPA_CMD_RATE_INFO) 

/** PPA Reset QOS Rate shaping. Value is manipulated by _IOW() macro for final value
    \param[in] PPA_CMD_RATE_INFO The parameter points to a
                            \ref PPA_CMD_RATE_INFO structure 
    \note: portid/queueid is input parameter, and weight is output value. 
*/
#define PPA_CMD_RESET_QOS_RATE _IOW(PPA_IOC_MAGIC, PPA_CMD_RESET_QOS_RATE_NR, PPA_CMD_RATE_INFO) 

#endif  //end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING
#endif //end of CONFIG_LTQ_PPA_QOS

/** PPA Add QOS Queue. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_QOS_ADD_QUEUE_INFO The parameter points to a
    \ref PPA_CMD_QOS_ADD_QUEUE_INFO structure
*/
#define PPA_CMD_ADD_QOS_QUEUE _IOWR(PPA_IOC_MAGIC, PPA_CMD_ADD_QOS_QUEUE_NR, PPA_CMD_QOS_QUEUE_INFO)

/** PPA Modify QOS Queue. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_QOS_MOD_QUEUE_INFO The parameter points to a
    \ref PPA_CMD_QOS_MOD_QUEUE_INFO structure
*/
#define PPA_CMD_MOD_QOS_QUEUE _IOWR(PPA_IOC_MAGIC, PPA_CMD_MOD_QOS_QUEUE_NR, PPA_CMD_QOS_QUEUE_INFO)

/** PPA Delete QOS Queue. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_QOS_DEL_QUEUE_INFO The parameter points to a
    \ref PPA_CMD_QOS_DEL_QUEUE_INFO structure
*/
#define PPA_CMD_DEL_QOS_QUEUE _IOWR(PPA_IOC_MAGIC, PPA_CMD_DEL_QOS_QUEUE_NR, PPA_CMD_QOS_QUEUE_INFO)

/** PPA Add WMM QOS Queue. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_QOS_ADD_QUEUE_INFO The parameter points to a
    \ref PPA_CMD_QOS_ADD_QUEUE_INFO structure
*/
#define PPA_CMD_ADD_WMM_QOS_QUEUE _IOWR(PPA_IOC_MAGIC, PPA_CMD_ADD_WMM_QOS_QUEUE_NR, PPA_CMD_QOS_QUEUE_INFO)

/** PPA Delete WMM QOS Queue. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_QOS_DEL_QUEUE_INFO The parameter points to a
    \ref PPA_CMD_QOS_DEL_QUEUE_INFO structure
*/
#define PPA_CMD_DEL_WMM_QOS_QUEUE _IOWR(PPA_IOC_MAGIC, PPA_CMD_DEL_WMM_QOS_QUEUE_NR, PPA_CMD_QOS_QUEUE_INFO)

#define PPA_CMD_ENG_QUEUE_INIT _IO(PPA_IOC_MAGIC, PPA_CMD_ENG_QUEUE_INIT_NR)
        
/** PPA UnInit GSWIP-L Queues. Value is manipulated by _IOW() macro for final value
    \param[in,out] PPA_CMD_QOS_DEL_QUEUE_INFO The parameter points to a
    \ref PPA_CMD_QOS_DEL_QUEUE_INFO structure
*/      
#define PPA_CMD_ENG_QUEUE_UNINIT _IO(PPA_IOC_MAGIC, PPA_CMD_ENG_QUEUE_UNINIT_NR)

/** PPA Modify QOS Sub interface to Port . Value is manipulated by _IOW() macro for final value
    \param[in] PPA_CMD_SUBIF_PORT_INFO The parameter points to a
    \ref PPA_CMD_SUBIF_PORT_INFO structure
*/
#define PPA_CMD_MOD_SUBIF_PORT _IOW(PPA_IOC_MAGIC, PPA_CMD_MOD_SUBIF_PORT_NR, PPA_CMD_SUBIF_PORT_INFO )

#ifdef CONFIG_LTQ_PPA_QOS
#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
#if defined(MBR_CONFIG) && MBR_CONFIG

/** PPA Set QOS rate shaper. Value is manipulated by _IOW() macro for final value
    \param[in] PPA_CMD_RATE_INFO The parameter points to a
    \ref PPA_CMD_RATE_INFO structure 
*/
#define PPA_CMD_SET_QOS_SHAPER _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_QOS_SHAPER_NR, PPA_CMD_RATE_INFO) 

/** PPA Get QOS Rate shaper configuration. Value is manipulated by _IOR() macro for final value
    \param[out] PPA_CMD_RATE_INFO The parameter points to a
                     \ref PPA_CMD_RATE_INFO structure 
    \note portid, queueid and weight should be set accordingly. 
*/
#define PPA_CMD_GET_QOS_SHAPER _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_QOS_SHAPER_NR, PPA_CMD_RATE_INFO) 

#endif //end of MBR_CONFIG
#endif  //end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING



#endif //end of CONFIG_LTQ_PPA_QOS



/**  PPA enable multiple field feature. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_ENABLE_MULTIFIELD_INFO The parameter points to a
          \ref PPA_CMD_ENABLE_MULTIFIELD_INFO structure   
*/
#define PPA_CMD_ENABLE_MULTIFIELD     _IOW(PPA_IOC_MAGIC,  PPA_CMD_ENABLE_MULTIFIELD_NR,  PPA_CMD_ENABLE_MULTIFIELD_INFO)

/**  PPA enable multiple field feature. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_ENABLE_MULTIFIELD_INFO The parameter points to a
          \ref PPA_CMD_ENABLE_MULTIFIELD_INFO structure   
*/
#define PPA_CMD_GET_MULTIFIELD_STATUS     _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_MULTIFIELD_STATUS_NR,  PPA_CMD_ENABLE_MULTIFIELD_INFO)

/**  PPA get multiple field max entry number supported. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_MULTIFIELD_ENTRY_MAX     _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_MULTIFIELD_ENTRY_MAX_NR,  PPA_CMD_COUNT_INFO)

/**  PPA get multiple field key number suported per compare set. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_MULTIFIELD_KEY_NUM     _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_MULTIFIELD_KEY_NUM_NR,  PPA_CMD_COUNT_INFO)   #not use at present

/**  PPA add a  multiple field compare set. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MULTIFIELD_FLOW_INFO The parameter points to a
          \ref PPA_CMD_MULTIFIELD_FLOW_INFO structure   
*/
#define PPA_CMD_ADD_MULTIFIELD     _IOW(PPA_IOC_MAGIC,  PPA_CMD_ADD_MULTIFIELD_NR,  PPA_CMD_MULTIFIELD_FLOW_INFO)

/**  PPA get a multiple field flow information. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MULTIFIELD_FLOW_INFO The parameter points to a
          \ref PPA_CMD_MULTIFIELD_FLOW_INFO structure   
*/
#define PPA_CMD_GET_MULTIFIELD     _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_MULTIFIELD_NR,  PPA_CMD_MULTIFIELD_FLOW_INFO)

/**  PPA delete a multiple field compare set according to specified compare key/mask/key_sel information. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MULTIFIELD_FLOW_INFO The parameter points to a
          \ref PPA_CMD_MULTIFIELD_FLOW_INFO structure   
*/
#define PPA_CMD_DEL_MULTIFIELD     _IOW(PPA_IOC_MAGIC,  PPA_CMD_DEL_MULTIFIELD_NR,  PPA_CMD_MULTIFIELD_FLOW_INFO)

/**  PPA delete a multiple field compare set according to specified index. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_MULTIFIELD_FLOW_INFO The parameter points to a
          \ref PPA_CMD_MULTIFIELD_FLOW_INFO structure   
*/
#define PPA_CMD_DEL_MULTIFIELD_VIA_INDEX  _IOW(PPA_IOC_MAGIC,  PPA_CMD_DEL_MULTIFIELD_VIA_INDEX_NR,  PPA_CMD_MULTIFIELD_FLOW_INFO)


/**  PPA get all exported hook count. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_GET_HOOK_COUNT       _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_HOOK_COUNT_NR,  PPA_CMD_COUNT_INFO)

/**  PPA get the exported hook list. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_HOOK_LIST_INFO The parameter points to a
          \ref PPA_CMD_HOOK_LIST_INFO structure   
*/
#define PPA_CMD_GET_HOOK_LIST           _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_HOOK_LIST_NR,  PPA_CMD_HOOK_LIST_INFO)

/**  PPA to enable/disable the exported hook. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_HOOK_ENABLE_INFO The parameter points to a
          \ref PPA_CMD_HOOK_ENABLE_INFO structure   
*/
#define PPA_CMD_SET_HOOK     _IOW(PPA_IOC_MAGIC,  PPA_CMD_SET_HOOK_NR,  PPA_CMD_HOOK_ENABLE_INFO)

#ifdef NO_DOXY
/**  PPA get the memory value. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_MEM_INFO The parameter points to a
          \ref PPA_CMD_MEM_INFO structure   
*/
#define PPA_CMD_READ_MEM         _IOR(PPA_IOC_MAGIC,  PPA_CMD_READ_MEM_NR,  PPA_CMD_READ_MEM_INFO)

/**  PPA to set the memory value. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/
#define PPA_CMD_SET_MEM     _IOW(PPA_IOC_MAGIC,  PPA_CMD_SET_MEM_NR,  PPA_CMD_SET_MEM_INFO)

#define PPA_CMD_GET_PPA_HASH_SUMMARY   _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_PPA_HASH_SUMMARY_NR, PPA_CMD_SESSION_SUMMARY_INFO)
//get PPA layer max hash index 
#define PPA_CMD_GET_COUNT_PPA_HASH     _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_COUNT_PPA_HASH_NR, PPA_CMD_COUNT_INFO)

#endif

/**  PPA to get the maximum accleration entry number. Value is manipulated by _IOR() macro for final value
    \param PPA_CMD_MAX_ENTRY_INFO The parameter points to a
          \ref PPA_CMD_MAX_ENTRY_INFO structure   
*/
#define PPA_CMD_GET_MAX_ENTRY   _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_MAX_ENTRY_NR,  PPA_CMD_MAX_ENTRY_INFO)

/**  PPA to get the port id of one specified interface name. Value is manipulated by _IOR() macro for final value
    \param PPA_CMD_MAX_ENTRY_INFO The parameter points to a
          \ref PPA_CMD_MAX_ENTRY_INFO structure   
*/
#define PPA_CMD_GET_PORTID   _IOR(PPA_IOC_MAGIC,  PPA_CMD_GET_PORTID_NR,  PPA_CMD_PORTID_INFO)

/**  PPA Get DSL MIB info. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_GET_DSL_MIB The parameter points to a
          \ref PPA_CMD_GET_DSL_MIB structure   
*/
#define PPA_CMD_GET_DSL_MIB                    _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_DSL_MIB_NR, PPA_CMD_DSL_MIB_INFO)

/**  PPA Clear DSL MIB info. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_CLEAR_DSL_MIB The parameter points to a
          \ref PPA_CMD_CLEAR_DSL_MIB structure   
*/
#define PPA_CMD_CLEAR_DSL_MIB                    _IOR(PPA_IOC_MAGIC, PPA_CMD_CLEAR_DSL_MIB_NR, PPA_CMD_DSL_MIB_INFO)

/**  PPA Get PPE port MIB info. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_GET_PORT_MIB The parameter points to a
          \ref PPA_CMD_GET_PORT_MIB structure   
*/
#define PPA_CMD_GET_PORT_MIB                    _IOR(PPA_IOC_MAGIC, PPA_CMD_GET_PORT_MIB_NR, PPA_CMD_PORT_MIB_INFO)

/**  PPA Clear PORT MIB info. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_CLEAR_PORT_MIB The parameter points to a
          \ref PPA_CMD_CLEAR_PORT_MIB structure   
*/
#define PPA_CMD_CLEAR_PORT_MIB                    _IOW(PPA_IOC_MAGIC, PPA_CMD_CLEAR_PORT_MIB_NR, PPA_CMD_PORT_MIB_INFO)

/**  PPA Enable/disable HAL debug flag. Value is manipulated by _IOR() macro for final value
   \param PPA_CMD_SET_HAL_DBG_FLAG The parameter points to a
          \ref PPA_CMD_SET_HAL_DBG_FLAG structure   
*/
#define PPA_CMD_SET_HAL_DBG_FLAG               _IOW(PPA_IOC_MAGIC, PPA_CMD_SET_HAL_DBG_FLAG_NR, PPA_CMD_ENABLE_INFO)

/**  PPA Debug Tools. Value is manipulated by _IOW() macro for final value
   \param PPA_CMD_SET_HAL_DBG_FLAG The parameter points to a
          \ref PPA_CMD_SET_HAL_DBG_FLAG structure   
*/
#define PPA_CMD_DBG_TOOL               _IOW(PPA_IOC_MAGIC, PPA_CMD_DBG_TOOL_NR, PPA_CMD_ENABLE_INFO)

/**  PPA set variable values
   \param PPA_CMD_SET_VALUE The parameter points to a
          \ref PPA_CMD_VARIABLE_VALUE_INFO structure   
*/
#define PPA_CMD_SET_VALUE               _IOWR(PPA_IOC_MAGIC, PPA_CMD_SET_VALUE_NR, PPA_CMD_VARIABLE_VALUE_INFO)

/**  PPA set variable values
   \param PPA_CMD_GET_VALUE The parameter points to a
          \ref PPA_CMD_VARIABLE_VALUE_INFO structure   
*/
#define PPA_CMD_GET_VALUE               _IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_VALUE_NR, PPA_CMD_VARIABLE_VALUE_INFO)

/**  PPA get variable values
   \param PPA_CMD_GET_PPE_FASTPATH_ENABLE The parameter points to a
          \ref PPA_CMD_PPE_FASTPATH_ENABLE_INFO structure   
*/
#define PPA_CMD_GET_PPE_FASTPATH_ENABLE		_IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_PPE_FASTPATH_ENABLE_NR, PPA_CMD_PPE_FASTPATH_ENABLE_INFO)

/**  PPA set variable values
   \param PPA_CMD_SET_PPE_FASTPATH_ENABLE The parameter points to a
          \ref PPA_CMD_PPE_FASTPATH_ENABLE_INFO structure   
*/
#define PPA_CMD_SET_PPE_FASTPATH_ENABLE		_IOWR(PPA_IOC_MAGIC, PPA_CMD_SET_PPE_FASTPATH_ENABLE_NR, PPA_CMD_PPE_FASTPATH_ENABLE_INFO)


#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
/**  PPA get variable values
   \param PPA_CMD_GET_SW_FASTPATH_ENABLE The parameter points to a
          \ref PPA_CMD_SW_FASTPATH_ENABLE_INFO structure   
*/
#define PPA_CMD_GET_SW_FASTPATH_ENABLE		_IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_SW_FASTPATH_ENABLE_NR, PPA_CMD_SW_FASTPATH_ENABLE_INFO)

/**  PPA set variable values
   \param PPA_CMD_SET_PPE_FASTPATH_ENABLE The parameter points to a
          \ref PPA_CMD_PPE_FASTPATH_ENABLE_INFO structure   
*/
#define PPA_CMD_SET_SW_FASTPATH_ENABLE		_IOWR(PPA_IOC_MAGIC, PPA_CMD_SET_SW_FASTPATH_ENABLE_NR, PPA_CMD_SW_FASTPATH_ENABLE_INFO)

/**  PPA get variable values
   \param PPA_CMD_GET_SW_SESSION_ENABLE The parameter points to a
          \ref PPA_CMD_SW_SESSION_ENABLE_INFO structure   
*/
#define PPA_CMD_GET_SW_SESSION_ENABLE		_IOWR(PPA_IOC_MAGIC, PPA_CMD_GET_SW_SESSION_ENABLE_NR, PPA_CMD_SW_SESSION_ENABLE_INFO)

/**  PPA set variable values
   \param PPA_CMD_SET_SW_SESSION_ENABLE The parameter points to a
          \ref PPA_CMD_SW_SESSION_ENABLE_INFO structure   
*/
#define PPA_CMD_SET_SW_SESSION_ENABLE		_IOWR(PPA_IOC_MAGIC, PPA_CMD_SET_SW_SESSION_ENABLE_NR, PPA_CMD_SW_SESSION_ENABLE_INFO)
#endif /*CONFIG_LTQ_PPA_API_SW_FASTPATH*/


/**  PPA Get PPA Hash Summary. Value is manipulated by _IOWR() macro for final value 
   \param PPA_CMD_COUNT_INFO The parameter points to a
          \ref PPA_CMD_COUNT_INFO structure   
*/


/*@}*/ /* PPA_IOCTL */


/*
 * ####################################
 *             Declaration
 * ####################################
 */

#ifdef __KERNEL__
#ifdef NO_DOXY

#ifdef CONFIG_LTQ_PORT_MIRROR
  extern struct net_device * (*get_mirror_netdev)(void);
  extern uint32_t (*is_device_type_wireless)(void);
   #define SKB_MIRROR_FLAG  0x2000
#endif

#define ENUM_STRING(x)   #x
  void ppa_subsystem_id(uint32_t *,
                            uint32_t *,
                            uint32_t *,
                            uint32_t *,
                            uint32_t *,
                            uint32_t *,
                            uint32_t *,
                            uint32_t *);

  void ppa_get_api_id(uint32_t *,
                          uint32_t *,
                          uint32_t *,
                          uint32_t *,
                          uint32_t *,
                          uint32_t *,
                          uint32_t *);


  int32_t ppa_init(PPA_INIT_INFO *, uint32_t);
  void ppa_exit(void);

  int32_t ppa_enable(uint32_t, uint32_t, uint32_t);
  int32_t ppa_get_status(uint32_t *, uint32_t *, uint32_t);

#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
  int32_t ppa_set_mib_mode(uint8_t );
  int32_t ppa_get_mib_mode(uint8_t* );
#endif
  int32_t ppa_session_add(PPA_BUF *, PPA_SESSION *, uint32_t);
  int32_t ppa_session_modify(PPA_SESSION *, PPA_SESSION_EXTRA *, uint32_t);
  int32_t ppa_session_get(PPA_SESSION ***, PPA_SESSION_EXTRA **, int32_t *, uint32_t);

  int32_t ppa_mc_group_update(PPA_MC_GROUP *, uint32_t);
  int32_t ppa_mc_group_get(IP_ADDR_C, IP_ADDR_C, PPA_MC_GROUP *, uint32_t);
  int32_t ppa_mc_entry_modify(IP_ADDR_C,IP_ADDR_C, PPA_MC_GROUP *, PPA_SESSION_EXTRA *, uint32_t);
  int32_t ppa_mc_entry_get(IP_ADDR_C, IP_ADDR_C, PPA_SESSION_EXTRA *, uint32_t);

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
int32_t ppa_session_ipsec_add(PPA_XFRM_STATE *ppa_x, sa_direction dir);
int32_t ppa_session_ipsec_delete(PPA_XFRM_STATE *ppa_x);
#endif

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
  int32_t ppa_mc_entry_rtp_get(IP_ADDR_C, IP_ADDR_C, uint8_t*);
  int32_t ppa_mc_entry_rtp_set(PPA_MC_GROUP*);
#endif

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
   int32_t ppa_capwap_update(PPA_CMD_CAPWAP_INFO *);
   int32_t ppa_capwap_delete(PPA_CMD_CAPWAP_INFO *);
#endif

  int32_t ppa_multicast_pkt_srcif_add(PPA_BUF *, PPA_NETIF *);

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
  int32_t ppa_add_class_rule(PPA_CLASS_RULE *rule);
  int32_t ppa_mod_class_rule(PPA_CLASS_RULE *rule);
  int32_t ppa_del_class_rule(PPA_CLASS_RULE *rule);
  int32_t ppa_get_class_rule(PPA_CLASS_RULE *rule);
#endif

  int32_t ppa_inactivity_status(PPA_U_SESSION *);
  int32_t ppa_set_session_inactivity(PPA_U_SESSION *, int32_t);

  int32_t ppa_bridge_entry_add(uint8_t *, PPA_NETIF *, PPA_NETIF *, uint32_t);
  int32_t ppa_bridge_entry_delete(uint8_t *, PPA_NETIF *, uint32_t);
  int32_t ppa_bridge_entry_hit_time(uint8_t *, PPA_NETIF *, uint32_t *);
  int32_t ppa_bridge_entry_inactivity_status(uint8_t *, PPA_NETIF *);
  int32_t ppa_set_bridge_entry_timeout(uint8_t *, PPA_NETIF *, uint32_t);
  int32_t ppa_hook_bridge_enable(uint32_t f_enable, uint32_t flags);
  int32_t ppa_hook_get_bridge_status(uint32_t *f_enable, uint32_t flags);

  int32_t ppa_set_bridge_if_vlan_config(PPA_NETIF *, PPA_VLAN_TAG_CTRL *, PPA_VLAN_CFG *, uint32_t);
  int32_t ppa_get_bridge_if_vlan_config(PPA_NETIF *, PPA_VLAN_TAG_CTRL *, PPA_VLAN_CFG *, uint32_t);
  int32_t ppa_vlan_filter_add(PPA_VLAN_MATCH_FIELD *, PPA_VLAN_INFO *, uint32_t);
  int32_t ppa_vlan_filter_del(PPA_VLAN_MATCH_FIELD *, PPA_VLAN_INFO *, uint32_t);
  int32_t ppa_vlan_filter_get_all(int32_t *, PPA_VLAN_FILTER_CONFIG *, uint32_t);
  int32_t ppa_vlan_filter_del_all(uint32_t);

  int32_t ppa_get_if_stats(PPA_IFNAME *, PPA_IF_STATS *, uint32_t);
  int32_t ppa_get_accel_stats(PPA_IFNAME *, PPA_ACCEL_STATS *, uint32_t);
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
  int32_t ppa_get_netif_accel_stats(PPA_IFNAME *, PPA_NETIF_ACCEL_STATS *, uint32_t);
#endif

  int32_t ppa_hook_set_ppe_fastpath_enable(uint32_t f_enable, uint32_t flags);
  int32_t ppa_hook_get_ppe_fastpath_enable(uint32_t *f_enable, uint32_t flags);
  
  int32_t ppa_set_if_mac_address(PPA_IFNAME *, uint8_t *, uint32_t);
  int32_t ppa_get_if_mac_address(PPA_IFNAME *, uint8_t *, uint32_t);

  int32_t ppa_add_if(PPA_IFINFO *, uint32_t);
  int32_t ppa_del_if(PPA_IFINFO *, uint32_t);
  int32_t ppa_get_if(int32_t *, PPA_IFINFO **, uint32_t);
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
  int32_t ppa_disconn_if(PPA_NETIF *, PPA_DP_SUBIF *, uint8_t *, uint32_t);
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
  int32_t ppa_register_for_qos_class2prio(int32_t , struct net_device *,PPA_QOS_CLASS2PRIO_CB  , uint32_t );
#endif
#endif

  int32_t ppa_hook_wan_mii0_vlan_range_add(PPA_VLAN_RANGE *, uint32_t);
  int32_t ppa_hook_wan_mii0_vlan_range_del(PPA_VLAN_RANGE *, int32_t);
  int32_t ppa_hook_wan_mii0_vlan_ranges_get(int32_t *, PPA_VLAN_RANGE *, uint32_t);

  int32_t ppa_get_max_entries(PPA_MAX_ENTRY_INFO *max_entry, uint32_t flags);
  int32_t ppa_ip_comare( PPA_IPADDR ip1, PPA_IPADDR ip2, uint32_t flag );
  int32_t ppa_zero_ip( PPA_IPADDR ip);
  int32_t ppa_ip_sprintf( char *buf, PPA_IPADDR ip, uint32_t flag);
  extern uint32_t g_ppa_ppa_mtu;
  extern uint32_t g_ppa_min_hits;
  extern volatile u_int8_t g_ppe_fastpath_enabled;
#endif //NO_DOXY

#endif //end of  __KERNEL__

#ifdef __KERNEL__
  #include <net/ppa_hook.h>
#endif


#endif  //   __PPA_API_H__20081031_1913__

