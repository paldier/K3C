#ifndef __PPA_STACK_AL_H__20081103_1153__
#define __PPA_STACK_AL_H__20081103_1153__



/******************************************************************************
**
** FILE NAME    : ppa_stack_al.h
** PROJECT      : PPA
** MODULES      : PPA Protocol Stack Adaption Layer (Linux)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Adaption Layer (Linux) Header File
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

/*! \file ppa_stack_al.h
    \brief This file contains es.
                provide linux os depenent api for PPA to use
*/


#include <net/ppa_api_common.h>

#ifdef __KERNEL__
  #include <linux/version.h>
  #include <linux/if_arp.h>
  #include <linux/if_pppox.h>
  #include <linux/list.h>
 #if defined(CONFIG_NF_CONNTRACK_SUPPORT) || defined(CONFIG_NF_CONNTRACK)
  #include <net/netfilter/nf_conntrack.h>    /* protocol independent conntrack */
 #else
  #include <linux/netfilter_ipv4/ip_conntrack.h>
 #endif
#ifdef CONFIG_LTQ_HANDLE_CONNTRACK_SESSIONS
#include <net/netfilter/nf_conntrack_session_limit.h>
#endif

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
#include <net/xfrm.h>
#endif

#endif  //end of __KERNEL__

#if defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE) || defined(CONFIG_LTQ_CPU_FREQ)
  //PMCU specific Head File
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
  #include <asm/ifx/types.h>
  #include <asm/ifx/pmcu.h>
#else
  //#include <types.h>
  #include <cpufreq/ltq_cpufreq.h>
//  #include <asm/mach-ltqcpe/ifx_pmcu.h>
#endif

#endif  //end of CONFIG_LTQ_PMCU

#if defined(CONFIG_LTQ_DATAPATH) && CONFIG_LTQ_DATAPATH
#include <net/datapath_api.h>
#endif

#ifdef NO_DOXY
#if defined(__KERNEL__)
typedef struct port_cell_info port_cell_info;
typedef int (*ltq_mei_atm_showtime_check_t)(const unsigned char, int *, struct port_cell_info *, void **); 
typedef int (*ltq_mei_atm_showtime_enter_t)(const unsigned char,struct port_cell_info *, void *); 
typedef int (*ltq_mei_atm_showtime_exit_t)(const unsigned char);
#endif
 
typedef enum { 
	LTQ_MEI_UNKNOWN = 0, 
	/** To register function for getting showtime status. */
	LTQ_MEI_SHOWTIME_CHECK = 1, 
	/** To register function for showtime entry signalling. */
	LTQ_MEI_SHOWTIME_ENTER = 2, 
	/** To register function for showtime exit signalling. */
	LTQ_MEI_SHOWTIME_EXIT = 3,
	/** To register function for TC-Layer selection.
	    For DSL/PP switchover concept (also refer to DSLCPE_SW-858) */ 
	LTQ_MEI_TC_REQUEST = 4,
	/** To register function for performing reset of TC-Layer.
	    (also refer to PPA_SYS-353) */ 
	LTQ_MEI_TC_RESET = 5
} e_ltq_mei_cb_type; 

/**
	This definition is used by the MEI Driver within TC request trigger to
	inform the PPA about the TC-Layer that has been negotiated during handshake
	and therefore needs to be loaded/configured on the PPA side. */
typedef enum { 
	/** Request to disable the TC */
	MEI_TC_REQUEST_OFF = 0,
	/** Request to load/configure the PTM (EFM) TC */
	MEI_TC_REQUEST_PTM = 1,
	/** Request to load/configure the ATM TC */
	MEI_TC_REQUEST_ATM = 2,
	/** Delimiter only! */
	MEI_TC_REQUEST_LAST = 3
} mei_tc_request_type; 

/**
	This bit-field definition is used by the MEI Driver to trigger reset
	handling(s) within TC-Layer of the PPA. */
typedef enum { 
	/** This value is defined for initialization purpose only.
		 At least one of the defined bits of the bitmask shall be set. */
	MEI_TC_RESET_CLEAN = 0x00000000,
	/** Request to perform a reset of the TC codeword buffer */
	MEI_TC_RESET_CW_BUFFER = 0x00000001
} mei_tc_reset_type;

struct ltq_mei_atm_showtime_info {
	void *check_ptr;
	void *enter_ptr;
	void *exit_ptr;
    void *req_tc_ptr;
    void *tc_reset_ptr;
};

/** Function pointer for new DSL/PP switchover concept(also refer to DSLCPE_SW-858)
   \param line            Line number
   \param tc_reqest_type  TC-Layer to be used/configured
   \param is_bonding      Defines whether bonding is used (>=1) or not (0)
	\return                0 if successful
 */
typedef int (*mei_tc_request_t)(
	const unsigned char line,
	mei_tc_request_type tc_reqest_type,
	int is_bonding);

/** Function pointer for triggering a reset of the TC (also refer to PPA_SYS-353)
   \param line            Line number 
	\param tc_reset_type   TC Reset type to be performed 
	\return                0 if successful */ 
typedef int (*mei_tc_reset_t)(
	const unsigned char,
	mei_tc_reset_type tc_reset_type);
 
#endif
/*
 * ####################################
 *              Definition
 * ####################################
 */

/*! \def PPA_ETH_ALEN
    \brief Macro that specifies the maximum length of an Ethernet MAC address.
 */
#define PPA_ETH_ALEN                            ETH_ALEN

/*! \def PPA_ETH_HLEN
    \brief Macro that specifies the maximum length of an Ethernet MAC header.
 */
#define PPA_ETH_HLEN                            ETH_HLEN

/*! \def PPA_ETH_CRCLEN
    \brief Macro that specifies the maximum length of an Ethernet CRC.
 */
#define PPA_ETH_CRCLEN                          4

/*! \def PPA_IF_NAME_SIZE
    \brief Macro that specifies the maximum size of one interface name
 */
#define PPA_IF_NAME_SIZE                        IFNAMSIZ

/*! \def PPA_IF_SUB_NAME_MAX_NUM
        \brief Macro that specifies the maximum size of one interface name
     */
#define PPA_IF_SUB_NAME_MAX_NUM                5


/*! \def PPA_IPPROTO_TCP
    \brief Macro that specifies TCP flag
 */
#define PPA_IPPROTO_TCP                         6

/*! \def PPA_IPPROTO_UDP
    \brief Macro that specifies UDP flag
 */
#define PPA_IPPROTO_UDP                         17

/*! \def PPA_USER
    \brief Macro that specifies the flag for the buffer type from User Space via ioctl
 */
#define  PPA_USER                                       __user

/*
 *  definition for application layer
 */
#ifndef __KERNEL__
/*! \def ETH_ALEN
    \brief Macro that specifies the maximum length of an Ethernet MAC address.
 */
  #define ETH_ALEN                              6

/*! \def IFNAMSIZ
    \brief Macro that specifies the maximum size of an interface NAME
 */
  #define IFNAMSIZ                              16
#endif

#undef NIPQUAD
/*! \def NIPQUAD
    \brief Macro that specifies NIPQUAD definition for printing IPV4 address
 */
#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

#undef NIPQUAD_FMT
/*! \def NIPQUAD_FMT
    \brief Macro that specifies NIPQUAD_FMT format definition for printing IPV4 address
 */
#define NIPQUAD_FMT "%u.%u.%u.%u"


#undef NIP6
/*! \def NIP6
    \brief Macro that specifies NIP6 definition for printing IPV6 address
 */
#define NIP6(addr) \
         ntohs(((unsigned short *)addr)[0]), \
         ntohs(((unsigned short *)addr)[1]), \
         ntohs(((unsigned short *)addr)[2]), \
         ntohs(((unsigned short *)addr)[3]), \
         ntohs(((unsigned short *)addr)[4]), \
         ntohs(((unsigned short *)addr)[5]), \
         ntohs(((unsigned short *)addr)[6]), \
         ntohs(((unsigned short *)addr)[7])

#undef NIP6_FMT
/*! \def NIP6_FMT
    \brief Macro that specifies NIP6_FMT format definition for printing IPV6 address
 */
#define NIP6_FMT "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x"


/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *  data type for application layer
 */
#ifndef __KERNEL__
#ifndef BUILD_FROM_PPA_ADAPT
/*!
    \brief This is the unsigned char 32-bit data type.
*/
  typedef unsigned long         uint32_t;

/*!
    \brief This is the unsigned char 16-bit data type.
*/
  typedef unsigned short        uint16_t;

/*!
    \brief This is the unsigned char 8-bit data type.
*/
  typedef unsigned char         uint8_t;

/*!
    \brief This is the unsigned char 64-bit data type.
*/
  typedef unsigned long long        uint64_t;

#endif
#endif
/*
 *  data type for API
 */
/*!
    \brief  Pointer to interface name
*/
typedef char                    PPA_IFNAME;
/*!
    \brief  This is the data structure holding the IP address. It helps to provide future compatibility for IPv6 support.
              Currently it only supports IPv4.

*/
typedef uint32_t                IPADDR;

/*!
    \brief Union of PPA network address
 */
  typedef union {
	uint32_t ip;        /*!< the storage buffer for ipv4 */
#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
	uint32_t ip6[4];    /*!< the storage buffer for ipv6 */
#endif
  }PPA_IPADDR;

#ifdef __KERNEL__


#if defined(CONFIG_LTQ_PPA_MPE_IP97)

typedef enum {
    SESSION_IPV4=2,
    SESSION_IPV6=10,
} session_type;


/*!
    \brief  xfrm state structure.
*/
  typedef struct xfrm_state PPA_XFRM_STATE;

/*!
    \brief  session information from xfrm state structure.
*/

session_type ppa_is_ipv4_ipv6(PPA_XFRM_STATE *x);

/*!
    \brief  session information from xfrm state structure.
*/

bool ppa_ipsec_addr_equal(xfrm_address_t *a, xfrm_address_t *b, sa_family_t family);

#endif

/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
  typedef struct sk_buff        PPA_BUF;

/*!
    \brief  Stateful Packet inspection / connection tracking session data structure.
            A packet is classified to such a session by SPI/NAT infrastructure.
            In Linux, this is defined to the Linux ip_conntrack/nf_conntrack structure.
*/
 #if defined(CONFIG_NF_CONNTRACK_SUPPORT) || defined(CONFIG_NF_CONNTRACK)
  typedef struct nf_conn        PPA_SESSION;
 #else
  typedef struct ip_conntrack   PPA_SESSION;
 #endif
/*!
      \brief This holds information of a session - 5 tuples.
*/
typedef struct nf_conntrack_tuple   PPA_TUPLE;

/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
 typedef struct udphdr		PPA_UDPHDR;

/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
 typedef struct tcphdr		PPA_TCPHDR;

/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
 typedef struct iphdr		PPA_IPHDR;
 
/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
 typedef struct ipv6hdr		PPA_IPV6HDR;

/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
 typedef struct pppoe_hdr	PPA_PPPOEHDR;

/*!
    \brief  Packet buffer structure. For Linux OS, this is the sk_buff structure.
*/
 typedef struct in6_addr	PPA_IN6ADDR;

/*!
    \brief Macro that specifies PPA network interface data structure
 */
  typedef struct net_device     PPA_NETIF;

#if defined(CONFIG_LTQ_DATAPATH) && CONFIG_LTQ_DATAPATH
/*!
    \brief Macro that specifies PPA sub-interface data structure
 */
  typedef struct dp_subif       PPA_DP_SUBIF;
#endif

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
/*!
    \brief Macro that specifies PPA time spec data structure
 */
  typedef struct timespec	PPA_TIMESPEC;

#endif

 /*!
    \brief Macro that specifies PPA network interface status structure
 */
  typedef struct net_device_stats     PPA_NETIF_STATS;


/*!
    \brief This is the data structure for the PPA ATM VC structure. In Linux, this is defined to the Linux atm_vcc structure
 */
  typedef struct atm_vcc        PPA_VCC;


/*!
    \brief PPA synchroniztion primitive for exclusion and/or synchroniztion, especially for PPE share buffer access
*/
  typedef spinlock_t            PPE_LOCK;

/*!
    \brief PPA datastructure for task structure; In Linux this is defined as Linux task structure
*/
  typedef struct task_struct	PPA_TASK;

#define PPA_TASK_RUNNING TASK_RUNNING
#define PPA_TASK_INTERRUPTIBLE TASK_INTERRUPTIBLE

/*!
    \brief  PPA synchronization primitive for exclusion and/or synchronization
*/
  typedef struct ppa_lock{
      PPE_LOCK        lock;  /*!< PPA  lock */
      unsigned long     flags; /*!< flag */
      uint32_t          cnt;   /*!< lock counter */
  }PPA_LOCK;


/*!
    \brief  PPA memory pool cache for efficient allocation of PPA data structures. Can be mapped to
              suitable OS allocation logic
*/
  typedef struct kmem_cache     PPA_MEM_CACHE;

/*!
    \brief  PPA Timer data structure. Should allow one shot timers to be configured with a passed
              timer callback function
*/
  typedef struct timer_list     PPA_TIMER;

/*!
    \brief  PPA atomic timer structure. In linux, it is atomic_t structure.
*/
  typedef atomic_t              PPA_ATOMIC;

/*!
    \brief PPA hash list head structure
*/
  typedef struct hlist_head     PPA_HLIST_HEAD;

/*!
    \brief PPA hash list node structure
*/
  typedef struct hlist_node     PPA_HLIST_NODE;

 /*!
    \brief  PPA SIZE_T. For Linux OS,  the size_t is unsigned int.
*/
  typedef size_t   PPA_SIZE_T;
/*!
    \brief  PPA FILE PPA_FILE_OPERATIONS. For Linux OS,  it is file_operations
*/
  typedef struct file_operations   PPA_FILE_OPERATIONS;

/*!
    \brief  PPA sync. For Linux OS,  it is __sync
    \note, someties GCC will wrongly optimize the code, so __sync is used to avoid it.  \n
              otherwise, just define PPA_SYNC to do { } while(0)
*/
#ifdef CONFIG_X86
  #define __sync()     do { } while(0)
#endif
  #define  PPA_SYNC __sync

#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
/*!
    \brief  PPA POWER MANAGEMENT RETURN VALUE.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
  typedef IFX_PMCU_STATE_t PPA_PWM_STATE_t;
#else
  typedef enum ltq_cpufreq_state PPA_PWM_STATE_t;
#endif

/*!
    \brief  PPA POWER MANAGEMENT RETURN VALUE.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
  typedef IFX_PMCU_RETURN_t   PPA_PWM_RETURN_t;
#else
//  typedef IFX_PMCU_RETURN_t   PPA_PWM_RETURN_t;
#endif

/*!
    \brief  PPA POWER MANAGEMENT MODUE STATE VALUE.
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
  typedef IFX_PMCU_MODULE_STATE_t   PPA_PWM_MODULE_STATE_t;
#else
  typedef struct ltq_cpufreq_module_state   PPA_PWM_MODULE_STATE_t;
#endif
#endif //end of CONFIG_LTQ_PMCU

/*
 *  definition
 */
/*!
    \brief  PPA session list traversal 
*/
/*                                                                                                     (tpos, pos, head, member)   */ 
#define ppa_hlist_for_each_entry        hlist_for_each_entry       

/*!
    \brief  PPA session list safe traversal
*/
/*                                                                                                               (tpos, pos, n, head, member) */
#define ppa_hlist_for_each_entry_safe   hlist_for_each_entry_safe  

/*!
    \brief  PPA session list traversal
*/

/*                                                                                                               (ptr, type, member) */
#define ppa_hlist_entry                 hlist_entry 

/*!
    \brief  PPA session list delete
*/
/*                                                                                                                (struct hlist_node *n) */
#define ppa_hlist_del                   hlist_del_init   

/*!
    \brief  PPA session list empty
*/
/*                                                                                                                (struct hlist_node *n) */
#define ppa_hlist_empty                   hlist_empty  

/*!
    \brief  PPA session list empty
*/
/*                                                                                                                (struct hlist_node *n) */
#define ppa_hlist_add_before              hlist_add_before  

/*!
    \brief  PPA session list empty
*/
/*                                                                                                                (struct hlist_node *n) */
#define ppa_hlist_add_after                   hlist_add_after  






/*!
    \brief  PPA session list op: get list header
*/

/*                                                                                             (ptr)*/
#define PPA_INIT_HLIST_HEAD             INIT_HLIST_HEAD  

/*!
    \brief  PPA session list op: get one session list 
*/
/*                                                                                              (struct hlist_node *h) */
#define PPA_INIT_HLIST_NODE             INIT_HLIST_NODE

/*!
    \brief  PPA session list op: add new list to the list header
*/
/*                                                                                              (struct hlist_node *n, struct hlist_head *h) */
#define ppa_hlist_add_head              hlist_add_head 

/*!
    \brief  PPA session list traversal
*/
/*                                                                                              (pos, head) */
#define ppa_hlist_for_each              hlist_for_each  

/* hlist rcu operations */
#define ppa_hlist_add_head_rcu        hlist_add_head_rcu
#define ppa_hlist_del_rcu             hlist_del_rcu
#define ppa_hlist_for_each_rcu       __hlist_for_each_rcu
#define ppa_hlist_for_each_entry_rcu  hlist_for_each_entry_rcu
typedef struct rcu_head   PPA_RCU_HEAD; 
#define ppa_call_rcu      call_rcu

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG

/*!
    \brief PPA list head structure
*/
  typedef struct list_head     PPA_LIST_HEAD;

/*!
    \brief PPA hash list node structure
*/
  typedef struct list_head     PPA_LIST_NODE;

  
/*!
    \brief  PPA  list traversal 
*/
/*                                                                                                     (tpos, pos, head, member)   */ 
#define ppa_list_for_each_entry        list_for_each_entry       

/*!
    \brief  PPA list safe traversal
*/
/*                                                                                                               (tpos, pos, n, head, member) */
#define ppa_list_for_each_entry_safe   list_for_each_entry_safe  

/*!
    \brief  PPA list safe traversal in reverse
*/
/*                                                                                                               (tpos, pos, head, member) */
#define ppa_list_for_each_entry_safe_reverse list_for_each_entry_safe_reverse 

/*!
    \brief  PPA list traversal
*/

/*                                                                                                               (ptr, type, member) */
#define ppa_list_entry                 list_entry 

/*!
    \brief  PPA session list delete
*/
/*                                                                                                                (struct hlist_node *n) */
#define ppa_list_del                   list_del   

/*!
    \brief  PPA list op: get list header
*/

/*                                                                                             (ptr)*/
#define PPA_INIT_LIST_HEAD             LIST_HEAD  

#define PPA_LIST_HEAD_INIT			   INIT_LIST_HEAD

/*!
    \brief  PPA session list op: add new list to the list header
*/
#define ppa_list_add_head              list_add 
#define ppa_list_add                   list_add 

/*!
    \brief  PPA list traversal
*/
#define ppa_list_for_each              list_for_each  

/*!
    \brief  PPA list delete 
*/
#define ppa_list_delete                list_del  

/*!
    \brief  PPA list iterate backwards over list with safe 
*/
#define ppa_list_for_each_entry_safe_reverse list_for_each_entry_safe_reverse

#endif

/*!
    \brief  synchronize_rcu
*/
#define ppa_synchronize_rcu             synchronize_rcu

/*!
    \brief  spin_lock_irqsave
*/
#define ppa_spin_lock_irqsave           spin_lock_irqsave

/*!
    \brief  spin_unlock_irqrestore
*/
#define ppa_spin_unlock_irqstore        spin_unlock_irqrestore

/*!
    \brief  spin_lock_bh
*/
#define ppa_spin_lock_bh                spin_lock_bh

/*!
    \brief  spin_unlock_bh
*/
#define ppa_spin_unlock_bh              spin_unlock_bh

/*!
    \brief  spin_lock_irq
*/
#define ppa_spin_lock_irq               spin_lock_irq


/*!
    \brief  spin_unlock_irq
*/
#define ppa_spin_unlock_irq             spin_unlock_irq


/*!
    \brief  rcu_read_lock
*/
#define ppa_rcu_read_lock               rcu_read_lock


/*!
    \brief  rcu_read_unlock
*/
#define ppa_rcu_read_unlock             rcu_read_unlock

/*!
    \brief  skb_headroom
*/
#define ppa_skb_headroom 	skb_headroom
/*!
    \brief  skb_realloc_headroom
*/
#define ppa_skb_realloc_headroom 	skb_realloc_headroom
/*!
    \brief  nf_conntrack_put
*/
#define ppa_nf_conntrack_put 		nf_conntrack_put
/*!
    \brief  skb_set_owner_w
*/
#define ppa_skb_set_owner_w 	skb_set_owner_w
/*!
    \brief  skb_push
*/
#define ppa_skb_push		skb_push
/*!
    \brief  skb_pull
*/
#define ppa_skb_pull 		skb_pull
/*!
    \brief  skb_set_mac_header
*/
#define ppa_skb_set_mac_header		skb_set_mac_header
/*!
    \brief  skb_set_mac_header
*/
#define ppa_skb_reset_mac_header	skb_reset_mac_header
/*!
    \brief  skb_set_network_header
*/
#define ppa_skb_set_network_header 	skb_set_network_header
/*!
    \brief  skb_set_transport_header
*/
#define ppa_skb_set_transport_header	skb_set_transport_header
/*!
    \brief  skb_network_header
*/
#define ppa_skb_network_header 		skb_network_header
/*!
    \brief  skb_transport_header
*/
#define ppa_skb_transport_header	skb_transport_header
/*!
    \brief  ip_fast_csum
*/
#define ppa_ip_fast_csum 		ip_fast_csum
/*!
    \brief  inet_proto_csum_replace4
*/
#define ppa_inet_proto_csum_replace4(ph,skb,x,y) 	inet_proto_csum_replace4(&ph->check,skb,x,y,1)
/*!
    \brief  inet_proto_csum_replace2
*/
#define ppa_inet_proto_csum_replace2(ph,skb,x,y) 	inet_proto_csum_replace2(&ph->check,skb,x,y,0)
/*!
    \brief  dev_queue_xmit
*/
#define ppa_dev_queue_xmit 	dev_queue_xmit
/*!
    \brief  skb->mark
*/
#define ppa_skb_mark(skb)	skb->mark
/*!
    \brief  skb->head
*/
#define ppa_skb_head(skb) 	skb->head
/*!
    \brief  skb->data
*/
#define ppa_skb_data(skb)	skb->data
/*!
    \brief  skb->dev
*/
#define ppa_skb_dev(skb)	skb->dev
/*!
    \brief  skb->len
*/
#define ppa_skb_len(skb)	skb->len
/*!
    \brief  skb->sk
*/
#define ppa_skb_sk(skb)		skb->sk
/*!
    \brief  to dereference any member of any structure
*/
#define ppa_get_member(x,y)	x->y



static inline void ppe_lock_init(PPE_LOCK *p_lock)
{
    spin_lock_init(p_lock);
}

static inline void ppe_lock_get(PPE_LOCK *p_lock)
{
    spin_lock_bh(p_lock);
}

static inline void ppe_lock_release(PPE_LOCK *p_lock)
{
    spin_unlock_bh(p_lock);
}


#endif //#ifdef __KERNEL__



/*
 * ####################################
 *           Inline Functions
 * ####################################
 */



/*
 * ####################################
 *             Declaration
 * ####################################
 */
/** \addtogroup  PPA_ADAPTATION_LAYER */
/*@{*/

#ifdef __KERNEL__
/*! \brief   Get the ppa adaption layer version
   \param[in] p_family Pointer to the hardware family
   \param[in] p_type Pointer to hardware type
   \param[in] p_if Pointer interface
   \param[in] p_mode Pointer mode
   \param[in] p_major Pointer major version number
   \param[in] p_mid   Pointer to min version number
   \param[in] p_minor Pointer to minor version number
   \note Provide anything required to put in remark section.
*/
  void ppa_get_stack_al_id(uint32_t *p_family,
                           uint32_t *p_type,
                           uint32_t *p_if,
                           uint32_t *p_mode,
                           uint32_t *p_major,
                           uint32_t *p_mid,
                           uint32_t *p_minor);

/*! \brief   Get the PPA session according to PPA_BUF
   \param[in] ppa_buf Pointer to the packet buffer.
   \return returns the PPA session pointer if found, otherwise return NULL
*/
  PPA_SESSION *ppa_get_session(PPA_BUF *ppa_buf);

/*! \brief   The neighbour cache initialization 
   \param[in] n Pointer to the neighbour.
   \param[in] dst Pointer to the destination entry.
*/
void ppa_neigh_hh_init(struct neighbour *n, struct dst_entry *dst);

/*! \brief   The neighbour cache update 
   \param[in] neigh Pointer to the neighbour.
*/
void ppa_neigh_update_hhs(struct neighbour *neigh);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)

/*! \brief   Get the PPA buffer network protocol header.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return point to the network header.
   \note Provide anything required to put in remark section.
*/
  uint8_t *skb_network_header(const PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer transport protocol header.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return point to the transport protocol header.
   \note Provide anything required to put in remark section.
*/
//  uint8_t *skb_transport_header(const PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer MAC header.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return point to the MAC header.
   \note Provide anything required to put in remark section.
*/
  uint8_t *skb_mac_header(const PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv6 packet's header.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return point to the IPv6 header structure.
   \note Provide anything required to put in remark section.
*/
  struct ipv6hdr *ipv6_hdr(const PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv4 packet's header.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return point to the IPv4 header structure.
   \note Provide anything required to put in remark section.
*/
  struct iphdr *ip_hdr(const PPA_BUF *ppa_buf);

/*! \brief   Returns if the IPv4 address is an IPv4 multicast packet.
   \param[in]  addr IPv4 address value.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the packet is a IPv4 multicast packet. \n
                           - IFX_FALSE otherwise. \n
   \note Provide anything required to put in remark section.
*/
  uint32_t ipv4_is_multicast(uint32_t addr);


#endif /*LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)*/


#if defined(CONFIG_LTQ_PPA_IPv6_ENABLE) && (defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE))


/*! \brief   Get the PPA buffer IPv6 transport protocol.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return transport protocol value.
   \note Provide anything required to put in remark section.
*/
  uint8_t ppa_get_ipv6_l4_proto(PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv6 packet's Type of Service value.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return Type of Service value.
   \note Provide anything required to put in remark section.
*/
  uint8_t ppa_get_ipv6_tos(PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv6 packet's source IP address.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return source IP address value.
   \note Provide anything required to put in remark section.
*/
  PPA_IPADDR ppa_get_ipv6_saddr(PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv6 packet's destination IP address.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return destination IP address value.
   \note Provide anything required to put in remark section.
*/
  PPA_IPADDR ppa_get_ipv6_daddr(PPA_BUF *ppa_buf);


/*! \brief   Returns if the packet pointed to by ppa_buf is an IPv6 multicast packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the packet is an IPv6 multicast packet. \n
                           - IFX_FALSE otherwise. \n
   \note Provide anything required to put in remark section.
*/
  int32_t ppa_is_ipv6_multicast(PPA_BUF *ppa_buf);


/*! \brief   Returns if the packet pointed to by ppa_buf is an IPv6 fragment packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the packet is an IPv6 fragment packet. \n
                           - IFX_FALSE otherwise. \n
   \note Provide anything required to put in remark section.
*/
  uint32_t ppa_is_ipv6_fragment(PPA_BUF *ppa_buf);

#endif /*CONFIG_LTQ_PPA_IPv6_ENABLE*/


/*! \brief   return the judgement of IPv6 packet type check.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   The function returns one of the following. \n
                     - IFX_TRUE, if the packet is an IPv6 packet. \n
                     - IFX_FALSE, if the packet is not an IPv6 packet.
   \note Provide anything required to put in remark section.
*/
  uint8_t ppa_is_pkt_ipv6(const PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv4 packet's transport protocol value.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return transport protocol value.
   \note Provide anything required to put in remark section.
*/
  uint8_t ppa_get_ip_l4_proto(PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv4 packet's Type of Service value.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return Type of Service value.
   \note Provide anything required to put in remark section.
*/
  uint8_t ppa_get_ip_tos(PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv4 packet's source IP address.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return source IP address value.
   \note Provide anything required to put in remark section.
*/
  PPA_IPADDR ppa_get_ip_saddr(PPA_BUF *ppa_buf);


/*! \brief   Get the PPA buffer IPv4 packet's destination IP address.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return destination IP address value.
   \note Provide anything required to put in remark section.
*/
  PPA_IPADDR ppa_get_ip_daddr(PPA_BUF *ppa_buf);


/*! \brief   Returns if the packet pointed to by ppa_buf is an IPv4 multicast packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the packet is an IPv4 multicast packet. \n
                           - IFX_FALSE otherwise. \n
   \note Provide anything required to put in remark section.
*/
  int32_t ppa_is_ip_multicast(PPA_BUF *ppa_buf);


/*! \brief   Returns if the packet pointed to by ppa_buf is an IPv4 fragment packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the packet is an IPv4 fragment packet. \n
                           - IFX_FALSE otherwise. \n
   \note Provide anything required to put in remark section.
*/
  uint32_t ppa_is_ip_fragment(PPA_BUF *ppa_buf);


/*! \brief   Turn the given IP to string and put it to the given buffer.
   \param[in]  ppa_ip the source ip address
   \param[in]  flag the flag of ipv6 or ipv4: 1--ipv6, 0-ipv4
   \param[out] strbuf contains the string format of IP ( the storage buffer should be allocated before calling the api).
   \return   return the point to the given buffer.
   \note Provide anything required to put in remark section.
*/
  int8_t *ppa_get_pkt_ip_string(PPA_IPADDR ppa_ip, uint32_t flag, int8_t *strbuf);


/*! \brief   Turn the given MAC address to string and put it to the given buffer.
   \param[in]  *mac point to MAC address buffer.
   \param[out] *strbuf contains the string format of MAC.
   \return   return the point to the given buffer.
   \note Provide anything required to put in remark section.
*/
  int8_t *ppa_get_pkt_mac_string(uint8_t *mac, int8_t *strbuf);


/*! \brief   return the length of IP address.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   return the number of bytes of the length of IP address.
   \note Provide anything required to put in remark section.
*/
  uint32_t ppa_get_pkt_ip_len(PPA_BUF *ppa_buf);



/*! \brief   Get the PPA buffer IP protocol
   \param[in]  buf Pointer to the packet buffer.
   \return The return value can be IP protocol value between 0-255. A value of 0
   \note Provide anything required to put in remark section.
*/
  uint8_t ppa_get_pkt_ip_proto(PPA_BUF *buf);



/*! \brief   Get the PPA buffer IP Type of Service field.
   \param[in]  buf Pointer to the packet buffer.
   \return  The return value is IP header ToS value.
   \note
*/
  uint8_t ppa_get_pkt_ip_tos(PPA_BUF *buf);



/*! \brief   Returns source IP address of the packet.
   \param[in] buf Pointer to the Packet buffer.
   \return  The Source IP address of the packet.
   \note
*/
  PPA_IPADDR ppa_get_pkt_src_ip(PPA_BUF *buf);



/*! \brief   Get multicast packet's dest & src  IP address 
   \param[in]  ppa_buf Pointer to the packet buffer.
   \param[out]  dst_ip Pointer to the dst ip buffer.
   \param[out]  src_ip Pointer to the src ip buffer.
   \return  Success if the packet is a mulitcast packet 
   \note
*/
 int ppa_get_multicast_pkt_ip(PPA_BUF *ppa_buf, void *dst_ip, void *src_ip);



/*! \brief   Returns destination IP address of the packet.
   \param[in]  buf Pointer to the packet buffer.
   \return  The Destination IP address of the packet..
   \note
*/
  PPA_IPADDR ppa_get_pkt_dst_ip(PPA_BUF *buf);


/*! \brief   Returns source TCP/UDP port of the IP packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return TCP/UDP Source Port of the packet.
   \note
*/
  uint16_t ppa_get_pkt_src_port(PPA_BUF *ppa_buf);


/*! \brief   Returns destination TCP/UDP port of the packet.
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \return TCP/UDP Destination Port of the packet.
   \note
*/
  uint16_t ppa_get_pkt_dst_port(PPA_BUF *ppa_buf);


/*! \brief   Get the Source MAC address of the packet as received by the router.
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \param[out] mac  MAC address buffer in which the source MAC address is copied by the function.
   \return  This function does not return anything.
   \note   This API may not implemented on older PPA version.
*/
  void ppa_get_pkt_rx_src_mac_addr(PPA_BUF *ppa_buf, uint8_t mac[PPA_ETH_ALEN]);


/*! \brief   Get the Destination MAC address of the packet as received by the router.
   \param[in]  ppa_buf  Pointer to the PPA packet buffer.
   \param[out] mac  MAC address buffer in which the Destination MAC address is copied by the function.
   \return  This function does not return anything.
   \note   This API may not implemented on older PPA version.
*/
  void ppa_get_pkt_rx_dst_mac_addr(PPA_BUF *ppa_buf, uint8_t mac[PPA_ETH_ALEN]);


/*! \brief   Returns source (i.e. Received) interface of the packet at the router.
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \return Pointer to the Source /Rx Interface of the packet. The following values can be returned.\n
                     - NULL on error
                     - Pointer to Rx interface of the packet
   \note   This API may not implemented on older PPA version.
*/
  PPA_NETIF *ppa_get_pkt_src_if(PPA_BUF *ppa_buf);


/*! \brief   Returns Destination (i.e. Tx) interface of the packet at the router (for packets forwarded at IP or bridge level).
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \return Pointer to the Destination /Tx Interface of the packet. The following values can be returned.\n
                     - NULL on error
                     - Pointer to Rx interface of the packet
   \note   This API may not implemented on older PPA version.
*/
  PPA_NETIF *ppa_get_pkt_dst_if(PPA_BUF *ppa_buf);

/*! \brief   Returns skb priority of the packet at the router (for packets forwarded at IP or bridge level).
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \return Pointer to the Destination /Tx Interface of the packet. The following values can be returned.\n
                     - NULL on error
                     - Pointer to Rx interface of the packet
*/
uint32_t ppa_get_pkt_priority(PPA_BUF *ppa_buf);

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
/*! \brief   Returns session priority based on skb
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \return mark  if sucessful. otherwise return -1;
*/
uint32_t ppa_get_session_priority(PPA_BUF *ppa_buf);

/*! \brief   Returns low priority session threshold value
   \param[in]  flags ( for future use )
   \return nf_conntrack_low_prio_thresh;
*/
uint32_t ppa_get_low_prio_thresh(uint32_t flags);

/*! \brief   Returns default priority session threshold value
   \param[in]  flags ( for future use )
   \return nf_conntrack_def_prio_thresh;
*/
uint32_t ppa_get_def_prio_thresh(uint32_t flags);

/*! \brief   Returns low priority session data rate
   \param[in]  flags ( for future use )
   \return nf_conntrack_low_prio_data_rate;
*/
uint32_t ppa_get_low_prio_data_rate(uint32_t flags);

/*! \brief   Returns tcp initial offset
   \param[in]  flags ( for future use )
   \return nf_conntrack_tcp_initial_offset;
*/
uint32_t ppa_get_tcp_initial_offset(uint32_t flags);

/*! \brief   Returns tcp steady offset
   \param[in]  flags ( for future use )
   \return nf_conntrack_tcp_steady_offset;
*/
uint32_t ppa_get_tcp_steady_offset(uint32_t flags);

/*! \brief   Returns default priority session data rate
   \param[in]  flags ( for future use )
   \return nf_conntrack_def_prio_data_rate;
*/
uint32_t ppa_get_def_prio_data_rate(uint32_t flags);

/*! \brief   Returns nf_conntrack_session_limit_enable value
   \param[in]  flags ( for future use)
   \return nf_conntrack_session_limit_enable;
*/
uint32_t ppa_get_session_limit_enable(uint32_t flags);

/*! \brief   Returns delta timespec structure between lhs and rhs
   \param[in]  lhs timespec structure, rhs timespec structure
   \return delta timespec structure;
*/
struct timespec ppa_timespec_sub(struct timespec lhs,struct timespec rhs);

/*! \brief   Returns time in nano seconds for a timespec structure
   \param[in]  Pointer to timespec structure
   \return time in nanoseconds;
*/
s64 ppa_timespec_to_ns(struct timespec *lhs);

/*! \brief   Returns time in nano seconds for a timespec structure
   \param[in]  Pointer to timespec structure
   \return time in nanoseconds;
*/
void ppa_get_monotonic(struct timespec *lhs);
#endif

/*! \brief   Returns skb mark of the packet: for test purpose only
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
   \return mark  if sucessful. otherwise return -1;
*/

uint32_t ppa_get_skb_mark(PPA_BUF *ppa_buf);
uint32_t ppa_get_skb_extmark(PPA_BUF *ppa_buf);

/*! \brief   set new skb priority of the packet: for test purpose only
   \param[in]  ppa_buf Pointer to the PPA packet buffer.
    \param[in]  new_pri new skb priority value
   \return new priority if sucessful. otherwise return -1;
*/


uint32_t ppa_set_pkt_priority(PPA_BUF *ppa_buf, uint32_t new_pri);


/*!
   \brief   get ppoe  mac address and session id
   \param[in]  netif Pointer to pppoe network interface.
   \param[out] *pa Pointer to pppoe addres.
   \return Pointer to the Destination /Tx Interface of the packet. The following values can be returned.\n
                     - NULL on error
                     - Pointer to Rx interface of the packet
   \note   This API may not implemented on older PPA version.
*/
  int32_t ppa_pppoe_get_pppoe_addr(PPA_NETIF *netif, struct pppoe_addr *pa);



/*! \brief   Returns the PPPoE session Id  of the net interface structure.
   \param[in]  netif Pointer network interface structure.
   \return  The return value can be any one of the following:\n
                     - Non-zero value is PPPoE Session Id
                     - Zero indicates no valid PPPoE session.
   \note
*/
__u16 ppa_pppoe_get_pppoe_session_id(PPA_NETIF *netif);


/*! \brief   Returns the PPPoE session Id of the packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return  The return value can be any one of the following:\n
                     - Non-zero value is PPPoE Session Id. \n
                     - Zero indicates no valid PPPoE session i.e. not a PPPoE session packet.
   \note
*/
  __u16 ppa_get_pkt_pppoe_session_id(PPA_BUF *ppa_buf);


/*! \brief   get the pppoe's sub ethernet interface name
   \param[in]  netif Pointer to ppp network interface
   \param[out] pppoe_eth_ifname Provide buffer to store its sub ethernet interface name
   \return  The return value can be any one of the following:\n
                     - Non-zero fail to get its sub ethernet interface name \n
                     - Zero indicates succeed
   \note
*/
  int32_t ppa_pppoe_get_eth_netif(PPA_NETIF *netif, PPA_IFNAME pppoe_eth_ifname[PPA_IF_NAME_SIZE]);



/*! \brief   This function returns the physical or underlying interface (Ethernet-like) for a PPPoE interface specified by netif..
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \param[in]  ifname   Pointer to the network interface name.
   \param[out] phy_ifname   Interface name buffer in which the Physical interface name is copied by the function.
   \return  The return value can be any one of the following:\n
                   - PPA_SUCCESS, if PPPoE physical address retrieved ok
                   - PPA_FAILURE, on error
   \note
*/
  int32_t ppa_pppoe_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE]);


/*! \brief   check whether it is a ppp interface
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \return  The return value can be any one of the following:\n
                   - PPA_SUCCESS, on success. \n
                   - PPA_FAILURE, on error.
   \note   This API may not implemented on older PPA version.
*/
  uint32_t ppa_check_is_ppp_netif(PPA_NETIF *netif);

/*! \brief   check whether it is a pppoe interface
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \return  The return value can be any one of the following:\n
                   - PPA_SUCCESS, on success. \n
                   - PPA_FAILURE, on error.
   \note   This API may not implemented on older PPA version.
*/
  uint32_t ppa_check_is_pppoe_netif(PPA_NETIF *netif);

/*! \brief   get pppoe's destination mac address, ie, remote peer's mac address
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \param[out] mac   provide buffer to store desnation mac address
   \return  The return value can be any one of the following:\n
                   - PPA_SUCCESS, on success. \n
                   - PPA_FAILURE, on error.
   \note   This API may not implemented on older PPA version.
*/
  int32_t ppa_pppoe_get_dst_mac(PPA_NETIF *netif , uint8_t mac[PPA_ETH_ALEN]);


  int32_t ppa_pppol2tp_get_base_netif(PPA_NETIF *netif, PPA_IFNAME pppol2tp_eth_ifname[PPA_IF_NAME_SIZE]);
  int32_t ppa_pppol2tp_get_src_addr(PPA_NETIF *netif, uint32_t *outer_srcip);
  int32_t ppa_pppol2tp_get_dst_addr(PPA_NETIF *netif, uint32_t *outer_dstip);
  int32_t ppa_pppol2tp_get_l2tp_addr(PPA_NETIF *netif, struct pppol2tp_addr *pa);
  int32_t ppa_pppol2tp_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE]);


/*! \brief   get pppol2tp's destination mac address, ie, remote peer's mac address
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \param[out] mac   provide buffer to store desnation mac address
   \return  The return value can be any one of the following:\n
                   - IFX_SUCCESS, on success. \n
                   - IFX_FAILURE, on error.
   \note   This API may not implemented on older PPA version.
*/
  int32_t ppa_pppol2tp_get_l2tp_dmac(PPA_NETIF *netif, uint8_t *mac);


/*! \brief   Returns the PPPoL2TP Session Id of the packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return  The return value can be any one of the following:\n
                     - Non-zero value is PPPoL2TP Session Id. \n
                     - Zero indicates no valid PPPoL2TP Session i.e. not a PPPoL2TP Session packet.
   \note
*/
__u16 ppa_pppol2tp_get_l2tp_session_id(PPA_NETIF *netif);

/*! \brief   Returns the PPPoL2TP tunnel Id of the packet.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return  The return value can be any one of the following:\n
                     - Non-zero value is PPPoL2TP tunnel Id. \n
                     - Zero indicates no valid PPPoL2TP tunnel i.e. not a PPPoL2TP tunnel packet.
   \note
*/
__u16 ppa_pppol2tp_get_l2tp_tunnel_id(PPA_NETIF *netif);



/*! \brief   check whether it is a pppol2tp interface
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \return  The return value can be any one of the following:\n
                   - IFX_SUCCESS, on success. \n
                   - IFX_FAILURE, on error.
   \note   This API may not implemented on older PPA version.
*/
  uint32_t ppa_check_is_pppol2tp_netif(PPA_NETIF *netif);


/*! \brief   check whether lro is enabled on interface
   \param[in]  ppa_buf Pointer to the packet buffer.
   \param[in]  netif    Pointer to the network interface structure in the stack.
   \return  The return value can be any one of the following:\n
                   - IFX_SUCCESS, on success. \n
                   - IFX_FAILURE, on error.
   \note   This API may not implemented on older PPA version.
*/
uint32_t ppa_check_is_lro_enabled_netif(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN]);


/*! \brief   This function returns the destination MAC address to be used in the Ethernet frame when transmitted out of the router.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \param[in]  p_session Pointer to the NAT connection tracking session to which the packet belongs. This parameter may not be required in the function implementation, for eg. on Linux 2.4 adaptation.
   \param[out] mac The destination MAC address for the specific packet is copied into this field.
   \return  The return value can be any one of the following: \n
                   - PPA_SUCCESS, if destination MAC address is retrieved ok \n
                   - PPA_FAILURE, on error
   \note
*/
  int32_t ppa_get_dst_mac(PPA_BUF *ppa_buf, PPA_SESSION *p_session, uint8_t mac[PPA_ETH_ALEN],uint32_t daddr);



/*! \brief   Returns the pointer to network interface data structure in the stack for the specified interface name. For Linux, this is the netdevice structure pointer.
   \param[in]  ifname Pointer to the interface name.
   \return  The return value can be any one of the following: \n
                   - Pointer to network interface structure, on success. \n
                   - NULL on error.
   \note   This function needs to ensure that it has a handle / reference count to the network interface structure, so that the interface structure  cannot be deleted while the PPA has a reference to it. Please see the section Release Reference to network interface structure for the function to release the PPA's reference to the PPA_NETIF structure when done.
*/
  PPA_NETIF *ppa_get_netif(PPA_IFNAME *ifname);



#ifdef CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT
/*! \brief   This function returns the MTU of the interface.
   \param[in]  netif Pointer to the netif structure.
   \return   MTU size.
   \note
*/

int  ppa_get_mtu(PPA_NETIF *netif);
#endif

/*! \brief   This function releases the reference to a PPA_NETIF structure obtained through the function ppa_get_netif.
   \param[in]  netif Pointer to the netif structure.
   \return   No return value.
   \note
*/
  void ppa_put_netif(PPA_NETIF *netif);



/*! \brief   Get the MAC address of the specified interface of the router. It is valid for an Ethernet-like interface or a PPPoE interface bound to the former.
   \param[in]  netif  Pointer to the network interface structure.
   \param[out] mac  MAC address buffer in which the MAC address of the interface is copied by the function if its an Ethernet like interface.
   \param[in]  flag_down_only   down search only or up stream search also
   \return   No return value.
   \note  This API may not implemented on older PPA version.
*/
  int32_t ppa_get_netif_hwaddr(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN], uint32_t flag_down_only);


 /*! \brief   Get the bridge device from given device
    \param[in]  netif  Pointer to the network interface structure.
    \return   return bridge netdevice pointer or NULL
    \note  This API may not implemented on older PPA version.
 */

 PPA_NETIF *ppa_get_br_dev(PPA_NETIF *netif);


 /*! \brief   Returns the pointer to the interface name for the specified netif structure.
   \param[in]  netif  Pointer to the network interface structure.
   \return The return value can be any one of the following: \n
                         - Pointer to interface name, on success. \n
                         - NULL on error.
   \note
*/
 PPA_IFNAME *ppa_get_netif_name(PPA_NETIF *netif);




 /*! \brief   Returns true if both the netif structure points to same physical interface.
   \param[in]  netif1  Pointer to the first network interface structure.
   \param[in]  netif2  Pointer to the second network interface structure.
   \return   Valid values are below. \n
                       - IFX_TRUE, if netif1 is same as netif2 interface \n
                       - IFX_FALSE, if interface are not equal \n
   \note
*/
uint32_t ppa_is_netif_equal(PPA_NETIF *netif1, PPA_NETIF *netif2);




 /*! \brief   This function returns if the Network interface structure pointer corresponds to the interface name specified.
   \param[in]  netif  Pointer to the network interface structure.
   \param[in]  ifname  Pointer to the network interface name.
   \return   The function returns one of the following. \n
                     - IFX_TRUE, if the netif corresponds to the ifname. \n
                     - IFX_FALSE, if the netif is not for the ifname.
   \note This API may not implemented on older PPA version.
*/
 uint32_t ppa_is_netif_name(PPA_NETIF *netif, PPA_IFNAME *ifname);


/*! \brief   This function checks if the interface name prefix specified applies for the interface name of the specified PPA_NETIF structure. For eg., eth0 and eth1 both have network prefix of eth (n=3).
   \param[in] netif  Pointer to the network interface structure.
   \param[in] ifname_prefix  Pointer to the network interface name prefix.
   \param[in] n  Number of bytes of the prefix to compare with the interface name of the netif.
   \return   The function returns one of the following. \n
                     - IFX_TRUE, if the netif corresponds to the ifname prefix. \n
                     - IFX_FALSE, if the netif is not matching the ifname prefix.
   \note This API may not implemented on older PPA version.
*/

  uint32_t ppa_is_netif_name_prefix(PPA_NETIF *netif, PPA_IFNAME *ifname_prefix, int32_t n);



/*! \brief   Get the Physical or underlying Interface for the interface specified by netif or ifname pointers. If netif is specified, it is used for the lookup, else ifname is used.
   \param[in]  netif  Pointer to the network interface structure for which physical interface needs to be determined.
   \param[in]  ifname  Pointer to the network interface name for which physical interface needs to be determined.
   \param[in] phy_ifname  Interface name buffer in which the Physical interface name is copied by the function.
   \return   This function returns the following values. \n
                        - PPA_SUCCESS, on success. \n
                        - PPA_FAILURE, on error. \n
   \note This API may not implemented on older PPA version.
*/
  int32_t ppa_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE]);



/*! \brief   This function gives the vlan interface name specified by netif strucutre or ifname pointers. One of the two arguments needs to be specified in the function.
   \param[in]  netif  Pointer to the network interface structure for VLAN interface check is to be done.
   \param[in]  ifname  Pointer to the network interface name for which VLAN check is to be done.
   \param[in] vlan_ifname  Buffer where the vlan interface name is copied by the function.
   \return   The function returns one of the following. \n
                     - IFX_TRUE, if the interface exist. \n
                     - IFX_FALSE, if the interface doesn't exist.
   \note
*/
  int32_t ppa_get_underlying_vlan_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME vlan_ifname[PPA_IF_NAME_SIZE]);



/*! \brief   This function checks whether the interface specified by netif or ifname pointers is a VLAN interface. One of the two arguments needs to be specified in the function.
   \param[in]  netif  Pointer to the network interface structure for VLAN interface check is to be done.
   \param[in]  ifname  Pointer to the network interface name for which VLAN check is to be done.
   \return   This function returns the following values. \n
                      - PPA_SUCCESS, if the VLAN interface exist. \n
                      - IFX_FALSE, if the interface is does not exist.
   \note
*/
  int32_t ppa_if_is_vlan_if(PPA_NETIF *netif, PPA_IFNAME *ifname);

  int32_t ppa_is_macvlan_if(PPA_NETIF *netif, PPA_IFNAME *ifname);


/*! \brief   This function returns the physical or underlying interface (Ethernet-like) for a pseudo VLAN interface specified by netif structure or interface name.
   \param[in]  netif  Pointer to the VLAN net interface structure.
   \param[in]  ifname Pointer to the VLAN interface name for which underlying interface is to be determined
   \param[out] phy_ifname  Buffer where the physical/underlying interface is copied by the function for the VLAN interface ifname.
   \return   This function returns the following values. \n
                     - IFX_TRUE, if the interface is a VLAN interface \n
                     - PPA_FAILURE, if the interface is not a VLAN interface
   \note
*/
  int32_t ppa_vlan_get_underlying_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE]);




/*! \brief   This function returns the physical or underlying interface (Ethernet-like) for a pseudo VLAN interface specified by netif structure or interface name.
   \param[in]  netif  Pointer to the VLAN network interface structure.
   \param[in]  ifname  Pointer to the VLAN interface name for which underlying interface is to be determined.
   \param[out] phy_ifname  Buffer where the physical/underlying interface is copied by the function for the VLAN interface.
   \return   This function returns the following values. \n
                       - PPA_SUCCESS, if the VLAN interface exist \n
                       - PPA_FAILURE, if the interface is does not exist\n
   \note
*/
  int32_t ppa_vlan_get_physical_if(PPA_NETIF *netif, PPA_IFNAME *ifname, PPA_IFNAME phy_ifname[PPA_IF_NAME_SIZE]);



/*! \brief   This function returns the VLAN Id and tag info for a VLAN interface specified by netif. This includes the VLAN tag, 802.1P bits and the CFI bit. The caller will first determine if the network interface is a VLAN  interface before invoking this function.
   \param[in]  netif  Pointer to the network interface structure for which VLANId is to be returned.
   \return   This function returns the VLAN TCI (Tag control information).
   \note
*/
  uint32_t ppa_get_vlan_id(PPA_NETIF *netif);




/*! \brief   This function returns the TCI including priority and VLAN Id for a PPA buffer pointer by buf.
   \param[in]  buf  Pointer to PPA buffer.
   \return   This function returns the VLAN TCI (Tag control information).
   \note
*/
 uint32_t ppa_get_vlan_tag(PPA_BUF *buf);



/*! \brief   This function returns whether the interface specified by ifname or netif pointer is enslaved to a bridge, i.e. member of a bridge.
   \param[in]  ifname  Pointer to the network interface name for which bridge membership has to be determined.
   \param[in]  netif  Pointer to the network interface structure for which bridge membership is to be determined.
   \return   This function returns the one of the following values: \n
                    - IFX_TRUE, if the network interface is enslaved to a bridge. \n
                    - IFX_FALSE, if the network interface is not enslaved to a bridge.
   \note
*/
  int32_t ppa_is_netif_bridged(PPA_IFNAME *ifname, PPA_NETIF *netif);


  #ifdef NO_DOXY
  int32_t ppa_get_bridge_member_ifs(PPA_IFNAME *ifname, int *, PPA_IFNAME **);
  uint32_t cal_64_div(uint64_t t1, uint64_t t2);
  //void dev_kfree_skb_any(struct sk_buff *skb)
  #define PPA_SKB_FREE dev_kfree_skb_any
  #endif

/*! \brief   This function returns whether the interface specified by ifname or netif pointer is a bridge interface, i.e. other interfaces are enslaved to this bridge interface. For eg., br0 is a bridge interface in Linux which may have bridge members like eth0, nas0 etc.
   \param[in]  netif   Pointer to the network interface structure for which bridge interface check is to be done.
   \param[in]  ifname  Pointer to the network interface name for which bridge interface check is to be done.
   \return   This function returns the one of the following values: \n
                    - IFX_TRUE, if the network interface is a bridge interface/port. \n
                    - IFX_FALSE, if the network interface is not a bridge interface/port.
   \note
*/
  int32_t ppa_if_is_br_if(PPA_NETIF *netif, PPA_IFNAME *ifname);


/*! \brief   This function performs a bridge forwarding database lookup for the bridge specified by netif and returns the member interface on which the packet needs to be forwarded.
   \param[in]  netif  Pointer to the network interface structure for the bridge interface where destination lookup is to be performed.
   \param[in]  buf  Pointer to the packet buffer for the frame which has to be bridged (forwarded at Layer-2).
   \param[out] p_netif  Pointer to the bridge member network interface structure to which the packet needs to be forwarded.
   \return   This function returns the one of the following values: \n
                    - PPA_SUCCESS, if the lookup is successful in the bridge forwarding database. \n
                    - PPA_FAILURE, if the lookup is not successful.\n
   \note
*/
  int32_t ppa_get_br_dst_port(PPA_NETIF *netif, PPA_BUF *buf, PPA_NETIF **p_netif);

/*! \brief   This function returns the ip address of the network interface.
   \param[in]  ip  Pointer to the buffer which holds the ip address.
   \param[in]  netif   Pointer to the network interface structure for which bridge interface check is to be done.
   \return   This function returns the one of the following values: \n
                    - PPA_SUCCESS, if the network interface ip address is found. \n
                    - PPA_FAILURE, if the network interface is not having any ip address assigned.
   \note
*/
 
  int32_t ppa_get_netif_ip(uint32_t *ip, PPA_NETIF *netif);


/*! \brief   This function computes 6rd destination address based on the ipv6 prefix match if not then BR address as the destination address.
   \param[in]  dev  Pointer to the network interface structure.
   \param[in]  ppa_buf Pointer to the packet buffer.
   \return   This function returns the tunnel destination address 
\note
*/
 uint32_t ppa_get_6rdtunel_dst_ip(PPA_BUF *skb, PPA_NETIF *dev);


/*! \brief  This function check whether given interface is GRE. 
   \param[in]  dev  Pointer to the network interface structure.
   \return    Returns non-zero value if given interface GRE 
   \note
  */
 uint32_t ppa_is_gre_netif(PPA_NETIF* dev);

/*! \brief  This function check whether given interface is GRE and also returns
            type of GRE tunnel - IPv4/IPv6 & IP GRE/EoGRE
   \param[in]  dev  Pointer to the network interface structure.
   \param[out]  isIPv4Gre  Pointer to uint8_t. Set to 1 if IPv4 tunnel
   \param[out]  isGreTap   Pointer to uint8_t. Set 1 if tunnel is EoGRE
   \return    Returns non-zero value if given interface GRE 
   \note
  */
 uint32_t ppa_is_gre_netif_type(struct net_device* dev, 
                                uint8_t* isIPv4Gre,
                                uint8_t* isGreTap);

/*! \brief  This function returns the base interface of GRE device.
   \param[in]  dev  Pointer to the network interface structure.
   \return    On success returns base physical interface of GRE dev.
              On failure returns NULL
   \note
  */
 struct net_device* ppa_get_gre_phyif(PPA_NETIF* dev);

/*! \brief  This function returns required GRE header len.
   \param[in]  dev  Pointer to the network interface structure.
   \param[out] hdrlen  Pointer to the uint32_t to hold GRE header len.
   \return    On success returns PPA_SUCCESS and GRE header len is passed 
              in hdrlen
   \note
  */
 int32_t ppa_get_gre_hdrlen(PPA_NETIF* dev, uint16_t *hdrlen);

/*! \brief  This function forms GRE header for a given GRE interface. 
   \param[in]  dev  Pointer to the network interface structure.
   \param[in]  isIPv6  Set to 1 if session is IPv6
   \param[in]  Data length to be carried on GRE 
   \param[out] pHdr Pointer to buffer. The formated GRE header is returned 
                    in this buffer.
   \param[in,out]  len  Pointer to uint32_t. Caller should pass the len of 
                   the buffer. On return it contains actual GRE header len.
   \return    Returns PPA_SUCCESS value if GRE header is formed. 
   \note
  */
 int32_t ppa_form_gre_hdr(PPA_NETIF* dev, 
                          uint8_t isIPv6,
                          uint16_t dataLen,
                          uint8_t *pHdr, 
                          uint16_t* len);

#if 0
/*! \brief  This function returns the IPv4 GRE tunnel's IP header.
   \param[out] iph  Pointer to the IPv4 header.
   \param[in]  dev  Pointer to the network interface structure.
   \return    Returns PPA_SUCCESS if given interface is GRE and 
              IPv4 header is copied to iph. 
   \note
  */
 int ppa_get_gre4_tnl_iph(struct iphdr* iph,
                          PPA_NETIF *dev);

/*! \brief  This function returns the IPv6 GRE tunnel's IP header.
   \param[out] iph  Pointer to the IPv6 header.
   \param[in]  dev  Pointer to the network interface structure.
   \return    Returns PPA_SUCCESS if given interface is GRE and 
              IPv6 header is copied to iph. 
   \note
  */
 int ppa_get_gre6_tnl_iph(struct ipv6hdr* ip6h,
                          PPA_NETIF *dev);

#endif
/*! \brief   This function performs a bridge forwarding database lookup for the bridge specified by netif and returns the member interface on which the packet needs to be forwarded.
   \param[in]  netif  Pointer to the network interface structure for the bridge interface where destination lookup is to be performed.
   \param[in]  mac  Pointer to destination mac address.
   \param[out] p_netif  Pointer to the bridge member network interface structure to which the packet needs to be forwarded.
   \return   This function returns the one of the following values: \n
                       - PPA_SUCCESS, if the lookup is successful in the bridge forwarding database. \n
                       - PPA_FAILURE, if the lookup is not successful.
\note
*/
  int32_t ppa_get_br_dst_port_with_mac(PPA_NETIF *netif, uint8_t mac[PPA_ETH_ALEN], PPA_NETIF **p_netif);


/*! \brief   This function returns the PPA ATM VC structure for the EoATM (RFC 2684 Ethernet over ATM) interface specified by netif.
   \param[in] netif  Pointer to the network interface structure for the bridge interface where destination lookup is to be performed.
   \param[in] pvcc  Pointer to the pointer to PPA_VCC structure which is set to the VC associated with the EoATM interface specified by netif.
   \return   This function returns the one of the following values: \n
                         - PPA_SUCCESS, if the VCC structure is found for the EoATM interface \n
                         - PPA_FAILURE, on error
\note
*/
  int32_t ppa_br2684_get_vcc(PPA_NETIF *netif, PPA_VCC **pvcc);




 /*! \brief   This function checks if the interface specified by netif or ifname pointers is an EoATM interface as per RFC2684. The interface will be specified by passing one of netif and ifname in the call.
   \param[in] netif   Pointer to the network interface structure for the EoATM check is to be performed.
   \param[in] ifname  Pointer to the interface name for which the EoATM check is to be performed.
   \return   This function returns the one of the following values: \n
                       - IFX_TRUE, if the interface is an EoATM interface. \n
                       - IFX_FALSE, if the interface is not an EoATM interface. \n
\note
*/
int32_t ppa_if_is_br2684(PPA_NETIF *netif, PPA_IFNAME *ifname);

/*! \brief   This function checks if the interface specified by netif or ifname pointers is bridged or routed encapsulaton.
   \param[in] netif   Pointer to the network interface structure for the check is to be performed.
   \param[in] ifname  Pointer to the interface name for which the check is to be performed.
   \return   This function returns the one of the following values: \n
                       - PPA_SUCCESS,  Interface is enabled with IP encapsulation. \n
                       - PPA_FAILURE, Error.
\note
*/
int32_t ppa_if_is_ipoa(PPA_NETIF *netif, PPA_IFNAME *ifname);



/*! \brief   This function returns the PPA ATM VC structure for the PPPoA (RFC 2364 PPP over AAL5) interface specified by netif.
   \param[in] netif      Pointer to the network interface structure for the bridge interface where destination lookup is to be performed.
   \param[out] patmvcc  Pointer to the pointer to PPA_VCC structure which is set to the VC associated with the PPPoATM interface specified by netif.
   \return   This function returns the one of the following values: \n
                         - PPA_SUCCESS, if the VCC structure is found for the PPPoATM interface. \n
                         - PPA_FAILURE, on error.
\note
*/
int32_t ppa_pppoa_get_vcc(PPA_NETIF *netif, PPA_VCC **patmvcc);


/*! \brief   check whether it is a pppoa session.
   \param[in] netif   Pointer to the interface's netif
   \param[in] ifname   Pointer to interface name
   \return   This function returns the one of the following values: \n
                     - IFX_TRUE if the two session pointers are the same. \n
                     - IFX_FALSE if the two session pointers point to different sessions. \n
    \note, one of netif and ifname should be not NULL.
*/
  int32_t ppa_if_is_pppoa(PPA_NETIF *netif, PPA_IFNAME *ifname);


/*! \brief   Returns true if the two sessions are the same.
   \param[in] p_session1   Pointer to the PPA session 1.
   \param[in] p_session2   Pointer to the PPA session 2.
   \return   This function returns the one of the following values: \n
                     - IFX_TRUE if the two session pointers are the same. \n
                     - IFX_FALSE if the two session pointers point to different sessions. \n
\note
*/
  uint32_t ppa_is_session_equal(PPA_SESSION *p_session1, PPA_SESSION *p_session2);



/*! \brief   Get the Stack session Helper function for connection tracking. Such helper functions exist when a Connection tracking / SPI logic for the application protocol of that session. Examples are FTP control session, SIP signalling session etc.
   \param[in] p_session   Pointer to the PPA Session.
   \return   This function returns the one of the following values: \n
                       - Pointer to the session helper function as an uint32_t if helper exists. \n
                       - NULL otherwise. \n
   \note    The exact pointer of the session helper function is not of interest to PPA. Adaptations may just return IFX_TRUE if session has helper function, and return IFX_FALSE otherwise.
*/
  uint32_t ppa_get_session_helper(PPA_SESSION *p_session);



/*! \brief   Is the PPA session pointing to a special session which needs "slow path" handling due to protocol processing requirements of connection tracking, NAT or by any other criteria. Examples are  FTP control session, SIP signalling session etc.The API can check the session based on either a PPA buffer pointer or a PPA session pointer.
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \param[in] p_session  Pointer to the PPA Session.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the session is a special session. \n
                           - IFX_FALSE otherwise\note. \n
   \note
*/
  uint32_t ppa_check_is_special_session(PPA_BUF *ppa_buf, PPA_SESSION *p_session);



/*! \brief   Returns if the packet pointed to by ppa_buf is a fragmented IP datagram.
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                      - IFX_TRUE if packet is fragment of an IP datagram. \n
                      - IFX_FALSE if the packet is a non-fragmented IP datagram. \n
   \note
*/
  uint32_t ppa_is_pkt_fragment(PPA_BUF *ppa_buf);



/*! \brief   Returns if the packet pointed to by ppa_buf is addressed to the host (i.e. terminated inside the router).
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                      - IFX_TRUE if packet is addressed to host, i.e. for host output. \n
                      - IFX_FALSE if the packet is to be forwarded out of the router. \n
   \note
*/
  int32_t ppa_is_pkt_host_output(PPA_BUF *ppa_buf);

/*! \brief   Returns if the packet pointed to by ppa_buf is addressed to the host (i.e. terminated inside the router).
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                      - IFX_TRUE if packet is addressed to host, i.e. for host output. \n
                      - IFX_FALSE if the packet is to be forwarded out of the router. \n
   \note
*/
  int32_t ppa_is_pkt_host_in(PPA_BUF *ppa_buf);



/*! \brief   Returns if the packet pointed to by ppa_buf is a broadcast packet.
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                       - IFX_TRUE if packet is a broadcast packet. \n
                       - IFX_FALSE if the packet is not a broadcast packet. \n
  \note
*/
  int32_t ppa_is_pkt_broadcast(PPA_BUF *ppa_buf);



/*! \brief   Returns if the packet pointed to by ppa_buf is a multicast packet.
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if the packet is a multicast packet. \n
                           - IFX_FALSE otherwise. \n
  \note
*/
  int32_t ppa_is_pkt_multicast(PPA_BUF *ppa_buf);



/*! \brief   Returns if the packet pointed to by ppa_buf is a loopback packet, i.e. output to a loopback interface in the router (and not transmitted out of the router external interfaces).
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                           - IFX_TRUE if packet is a loopback packet.  \n
                           - IFX_FALSE if the packet is not a loopback packet \n
   \note
*/
  int32_t ppa_is_pkt_loopback(PPA_BUF *ppa_buf);



/*! \brief   Returns if the packet pointed to by ppa_buf is for local delivery, i.e. ingress packet delivered to Layer-4 and above)..
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                      - IFX_TRUE if packet is for local delivery to Layer-4 and above. \n
                      - IFX_FALSE if the packet is not a local delivery packet. \n
   \note
*/
  int32_t ppa_is_pkt_local(PPA_BUF *ppa_buf);



 /*! \brief   Returns if the packet pointed to by ppa_buf is routed, i.e. forwarded at IP layer.
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                         - IFX_TRUE if packet is forwarded at IP layer. \n
                         - IFX_FALSE if the packet is not forwarded at IP layer \n
   \note
*/
 int32_t ppa_is_pkt_routing(PPA_BUF *ppa_buf);



/*! \brief   Returns if the packet pointed to by ppa_buf is multicast routed.
   \param[in] ppa_buf    Pointer to the PPA Buffer.
   \return   This function returns the one of the following values: \n
                        - IFX_TRUE if packet is multicast forwarded at IP layer. \n
                        - IFX_FALSE if the packet is not multicast forwarded at IP layer. \n
   \note
*/
  int32_t ppa_is_pkt_mc_routing(PPA_BUF *ppa_buf);



 /*! \brief   Returns true if tcp connection state is established for a PPA session.
   \param[in] p_session  Pointer to ppa connection tracking session data structure.
   \return   This function returns the one of the following values: \n
                      - IFX_TRUE if TCP connection state is established after SYN from server. \n
                      - IFX_FALSE if TCP connection is not established completely. \n
   \note
*/
 int32_t ppa_is_tcp_established(PPA_SESSION *p_session);


 /*! \brief   check whether the TCP session is open or not.
   \param[in] p_session  Pointer to ppa connection tracking session data structure.
   \return   This function returns the one of the following values: \n
                      - 1 if the tcp state is not TIME_WAIT or error
                      - otherwise, return 0
   \note
*/
 int32_t ppa_is_tcp_open(PPA_SESSION *p_session);



/*! \brief   Initialize a lock for synchronization.
    \param[in] p_lock  Pointer to the PPA lock variable which is allocated by the caller.
    \return   This function returns the one of the following values: \n
                      - PPA_SUCCESS, if PPA Lock initialization is success. \n
                      - PPA_FAILURE, if the PPA Lock initialization fails.  \n
    \note
*/
  int32_t ppa_lock_init(PPA_LOCK *p_lock);

/*! \brief   Get or Acquire a PPA lock for synchronization.
    \param[in] p_lock  Pointer to the PPA lock variable which has been already initialized by the caller.
    \return   No value returned.
    \note
*/
  void ppa_lock_get(PPA_LOCK *p_lock);


/*! \brief   Release a PPA Lock acquired for synchronization.
    \param[in] p_lock  Pointer to the PPA lock variable which is to be released by the caller..
    \return    No valure returned.
    \note
*/
  void ppa_lock_release(PPA_LOCK *p_lock);

/*! \brief   Get or Acquire a PPA lock for synchronization.
    \param[in] p_lock  Pointer to the PPA lock variable which has been already initialized by the caller.
    \return   current flag.
    \note
*/
  uint32_t ppa_lock_get2(PPA_LOCK *p_lock);


/*! \brief   Release a PPA Lock acquired for synchronization.
    \param[in] p_lock  Pointer to the PPA lock variable which is to be released by the caller..
    \param[in] flag  system flag
    \return    No valure returned.
    \note
*/
  void ppa_lock_release2(PPA_LOCK *p_lock, uint32_t flag);


/*! \brief   Destroy a PPA lock created with the ppa_lock_init API
    \param[in] p_lock  Pointer to the PPA lock variable which is allocated by the caller.
    \return   No valure returned.
    \note
*/
  void ppa_lock_destroy(PPA_LOCK *p_lock);


/*! \brief   Disable interrupt processing to protect certain PPA critical regions and save current interrupt state to a global variable in the AL.
    \return   No valure returned.
    \note
*/
uint32_t ppa_disable_int(void);


/*! \brief   Enable interrupt processing to protect certain PPA critical regions. This must actually restore interrupt status from the last ppa_disable_int call.
    \param[in]  flag   Interrupt status flag.
    \return   No valure returned.
    \note
*/
void ppa_enable_int(uint32_t flag);


/*! \brief   This function dynamically allocates memory for PPA use.
    \param[in]  size   Specifies the number of bytes to be allocated.
    \return  The return value is one of the following: \n
                    - Non-NULL value, if memory allocation is successful. \n
                    - NULL, if the PPA Lock initialization fails.  \n
    \note
*/
  void *ppa_malloc(uint32_t size);

/*! \brief   This function frees dynamically allocated memory.
    \param[in] buff Pointer to buffer allocated by ppa_malloc routine, which needs to be freed.
    \return   The return value is one of the following: \n
                           - PPA_SUCCESS, if memory free is successful. \n
                           - PPA_FAILURE, if the memory free operation fails. \n
    \note
*/
  int32_t ppa_free(void *buff);



  /*! \brief   This function dynamically allocates memory for a cache of objects of a fixed size for PPA use.
    \param[in] name   Specifies the name of the memory cache as a string.
    \param[in]  size    Specifies the object size in bytes for the memory cache to be created.
    \param[out] pp_cache  Pointer to pointer to the memory cache to be created. *pp_cache is set by the function.
    \return   The return value is one of the following: \n
                           - PPA_SUCCESS value, if memory cache creation is successful. \n
                           - PPA_FAILURE, if the memory cache creation fails. \n
    \note
*/int32_t ppa_mem_cache_create(const char *name, uint32_t size, PPA_MEM_CACHE **pp_cache);


  /*! \brief   This function frees (or destroys) dynamically created memory cache using ppa_mem_cache_create API.
    \param[in] p_cache  Pointer to memory cache created by ppa_mem_cache_create routine, which needs to be destroyed.
    \return   The return value is one of the following: \n
                           - PPA_SUCCESS, if memory cache is destroyed. \n
                           - PPA_FAILURE, if the memory cache free operation fails \n
    \note
*/
int32_t ppa_mem_cache_destroy(PPA_MEM_CACHE *p_cache);


/*! \brief   This function allocates a memory cache object from the specified memory cache created using ppa_mem_cache_create API.
    \param[in] p_cache Pointer to memory cache created by ppa_mem_cache_create routine, to which an object needs to be freed.
    \return   No return value.
    \note
*/
  void *ppa_mem_cache_alloc(PPA_MEM_CACHE *p_cache);


/*! \brief   This function frees (or returns) allocated memory cache object using ppa_mem_cache_alloc API back to the memory cache pool.
    \param[in] buf  Pointer to memory cache object allocated from memory cache pointed to by p_cache pointer.
    \param[in] p_cache  Pointer to memory cache created by ppa_mem_cache_create routine, which needs to be destroyed.
    \return   The return value is one of the following:  \n
                         - PPA_SUCCESS, if memory cache is destroyed. \n
                         - PPA_FAILURE, if the memory cache free operation fails. \n
    \note
*/
void ppa_mem_cache_free(void *buf, PPA_MEM_CACHE *p_cache);


/*! \brief   This function does a byte copy from source buffer to destination buffer for the specified number of bytes.
    \param[in] dst  Pointer to destination buffer to copy to.
    \param[in] src  Pointer to source buffer to copy from.
	\param[in]  count Specifies the number of bytes to copy.
    \return   No return value.
    \note
*/
  void ppa_memcpy(void *dst, const void *src, uint32_t count);


  /*! \brief   This function does a byte set to destination buffer with the specified fill byte for the specified number of bytes..
    \param[in] dst      Pointer to destination buffer to set bytes.
    \param[in]  fillbyte  Byte value to fill in the destination buffer.
	\param[in]  count Specifies the number of bytes to set to fillbyte.
    \return   No return value.
    \note
*/
  void ppa_memset(void *dst, uint32_t fillbyte, uint32_t count);

  /*! \brief   This function compares  the memory areas buff1 and buff2 for specified number of bytes.
    \param[in] buff1  Pointer to destination first buffer.
    \param[in] buff2  Pointer to source second buffer.
	\param[in]   count Specifies the number of bytes to compare.
    \return  Returns an integer less than, equal to, or greater than zero if the first n bytes of buff1 is found, respectively, to be less than, to match, or be greater than the first n bytes of buff2.
    \note
*/
  int ppa_memcmp(const void *buff1, const void *buff2, size_t count);

/*! \brief   This function initializes the PPA_TIMER structure and fills in the callback function which is to be invoked by the timer facility when the timer expires. The PPA timer facility is a "one-shot" timer and not a periodic one.
    \param[in] p_timer  Pointer to the PPA_TIMER structure allocated by caller.
    \param[in]  callback  Timer callback function that is invoked when the timer expires.
	\return  The function returns one of the following values: \n
                        - PPA_SUCCESS, on success. \n
                        - PPA_FAILURE, on error. \n
    \note
*/
int32_t ppa_timer_init(PPA_TIMER *p_timer, void (*callback)(unsigned long));

/*! \brief   This function adds or installs a timer with the specified timer interval.
    \param[in] p_timer  Pointer to the initialized PPA_TIMER structure to be installed.
    \param[in]  timeout_in_sec   Timer expiry interval in seconds after which the one-shot timer will fire.
	\return  The function returns one of the following values: \n
                        - PPA_SUCCESS, on success. \n
                        - PPA_FAILURE, on error. \n
    \note
*/
int32_t ppa_timer_add(PPA_TIMER *p_timer, uint32_t timeout_in_sec);


/*! \brief   This function deletes an install timer which has not yet expired.
    \param[in] p_timer  Pointer to the installed PPA_TIMER structure to be deleted.
	\return  The function returns one of the following values: \n
                        - PPA_SUCCESS, on successful deletion of the timer. \n
                        - PPA_FAILURE, on error (for eg., timer already expired, or invalid timer pointer). \n
    \note
*/
void ppa_timer_del(PPA_TIMER *p_timer);


/*! \brief   This function adds or installs a timer with the specified timer interval.
   	\return  The function returns the following value: \n
                           - Current time in 10 milliseconds resolution.
    \note
*/
  uint32_t ppa_get_time_in_10msec(void);

/*! \brief   This function returns the current time of the system in seconds. It can be the time since reboot of the system, or an absolute time wrt NTP synced world time. PPA uses this function for timing intervals or periods.
	\return  The function returns the following values: \n
                           - Current time in seconds \n
    \note
*/
  uint32_t ppa_get_time_in_sec(void);

/*! \brief This function creates a kernel thread; it executes the function in a seperate kernel thread and returns the task structure of the kernel thread.
    \param[in] threadfn Pointer to the thread function.
    \param[in] data  Pointer to data for the threadfn
    \param[in] fn_name Name of the thread
    \return Pointer to the PPA_TASK structure 
*/

  
  PPA_TASK* ppa_kthread_create( int (*threadfn)(void *data), void *data, const char fn_name[]);

/*! \brief   This function initializes the PPA_TIMER structure and fills in the 
             callback function which is to be invoked by the timer facility 
             when the timer expires. On expiry the 'data' passed to this function
             is passed to callback function.
    \param[in] p_timer  Pointer to the PPA_TIMER structure allocated by caller.
    \param[in]  callback  Timer callback function that is invoked when the timer expires.
    \param[in]  data    The value to be passed to callback function.
	  \return  The function returns one of the following values: \n
                        - PPA_SUCCESS, on success. \n
                        - PPA_FAILURE, on error. \n
*/
  int ppa_timer_setup(PPA_TIMER *p_timer, 
                      void (*callback)(unsigned long), 
                      unsigned long data);

/*! \brief   This function checks whether time is pending. 
    \param[in] p_timer  Pointer to the PPA_TIMER.
    \return  The function returns succes if timer is pending \n
                        - PPA_SUCCESS, on success. \n
                        - PPA_FAILURE, on error. \n
*/
  int ppa_timer_pending(PPA_TIMER *p_timer);

/*! \brief This function checks whether someone has called kthread_stop function on this kthread
    \param[in] void
    \return true/false 	
*/

    int ppa_kthread_should_stop (void);

/*! \brief This function Sets kthread_should_stop for k to return true, wakes it, and waits for it to exit.
    \param[in] Pointer to PPA_TASK pointer created by calling ppa_kthread_start
    \return the result of threadfn, or -EINTR if ppa_wake_up_process was never called 
*/

    int ppa_kthread_stop(PPA_TASK* k);

/*! \brief This function wakes the process k from sleep, wakes it and put in run queue.
    \param[in] Pointer to PPA_TASK pointer created by calling ppa_kthread_start
    \return the result of threadfn, or -EINTR if ppa_wake_up_process was never called 
*/
void ppa_wake_up_process(PPA_TASK* k);


/*! \brief function to call the scheduler to relinquish the CPU. This will put the calling process in sleep
    \param[in] none
    \return none
*/

    void ppa_schedule(void);

/*! \brief function set the state of current process. 
    \param[in] process state PPA_TASK_RUNNING or PPA_TASK_INTERRUPTIBLE
    \return none
*/

    void ppa_set_current_state(int state);

/*! \brief function to get the master netdevice of an interface if any 
    \param[in] PPA_NETIF* netif to be checked
    \return PPA_NETIF* bridge interface if any else NULL
*/

PPA_NETIF* ppa_netdev_master_upper_dev_get(PPA_NETIF *netif);

/*! \brief function to take the rtnl_lock of kernel
*/
void ppa_rtnl_lock(void);

/*! \brief function to release the rtnl_lock of kernel
*/
void ppa_rtnl_unlock(void);


/*! \brief   Read atomic variable.
    \param[in] v  Pointer to the PPA atomic variable which is to be read.
	\return  No return value.
    \note
*/
  int32_t ppa_atomic_read(PPA_ATOMIC *v);

/*! \brief   Initialize the PPA atomic variable to specified value.
    \param[in]  v  Pointer to the PPA atomic variable which is to be initalized.
    \param[in]  i  Intended value to be set for atomic variable p_atomic.
    \return  No return value.
    \note
*/
  void ppa_atomic_set(PPA_ATOMIC *v, int32_t i);

/*! \brief   Atomic Increment of variable.
    \param[in] v  Pointer to the PPA atomic variable which is to be incremented.
    \return  No return value.
    \note
*/
  int32_t ppa_atomic_inc(PPA_ATOMIC *v);

/*! \brief   Atomic decrement of variable.
    \param[in] v  Pointer to the PPA atomic variable which is to be decremented.
    \return  No return value.
    \note
*/
int32_t ppa_atomic_dec(PPA_ATOMIC *v);

/*! \brief   Atomic Increment of variable if not zero.
    \param[in] v  Pointer to the PPA atomic variable which is to be incremented.
    \return   return value depends on low level API
    \note
*/
int32_t ppa_atomic_inc_not_zero(PPA_ATOMIC *v);

/*! \brief   Atomic decrement of variable and test
    \param[in] v  Pointer to the PPA atomic variable which is to be incremented.
    \return   return value depends on low level API
    \note
*/
int32_t ppa_atomic_dec_and_test(PPA_ATOMIC *v);

/*! \brief replace the old hash item with new one
    \param[in] old  Pointer to the hash item to be replaced
    \param[in] new  Pointer to the hash item to replace
    \return  No return value
    \note
*/
void ppa_hlist_replace(PPA_HLIST_NODE *old, PPA_HLIST_NODE *new);


/*! \brief   Used to perform buffer cloning.
    \param[in] ppa_buf  Pointer to ppa buffer.
	\param[in] flags   Reserved for future use.
	\return The return value is the pointer to cloned PPA buffer structure.
    \note
*/
  PPA_BUF *ppa_buf_clone(PPA_BUF *ppa_buf, uint32_t flags);



/*! \brief   Used to check if the buffer is cloned.
    \param[in] ppa_buf  Pointer to ppa buffer.
	\return  The return value is IFX_TRUE if the buffer is cloned and IFX_FLASE otherwise.
    \note
*/
int32_t ppa_buf_cloned(PPA_BUF *ppa_buf);


/*! \brief   get ppa prevous buffer
    \param[in] ppa_buf  Pointer to ppa buffer.
    \return return the prevois buffer
    \note
*/
  PPA_BUF *ppa_buf_get_prev(PPA_BUF *ppa_buf);

/*! \brief   get ppa next buffer
    \param[in] ppa_buf  Pointer to ppa buffer.
    \return return the next buffer
    \note
*/
  PPA_BUF *ppa_buf_get_next(PPA_BUF *ppa_buf);

/*! \brief   free ppa buffer
    \param[in] ppa_buf  Pointer to ppa buffer .
    \note
*/
  void ppa_buf_free(PPA_BUF *ppa_buf);

/*! \brief   copy data from username to kernel
    \param[out] to  destination buffer
    \param[in] from  source buffer
    \param[in] n  bytes to copy
    \note
*/
  uint32_t ppa_copy_from_user(void *to, const void PPA_USER  *from, uint32_t  n);

/*! \brief   copy data from kernel to username
    \param[out] to  destination buffer
    \param[in] from  source buffer
    \param[in] n  bytes to copy
    \note
*/
  uint32_t ppa_copy_to_user(void PPA_USER *to, const void *from, uint32_t  n);

/*! \brief   copy string, like strcpy
    \param[out] dest  destination buffer
    \param[in] src  source buffer
    \note
*/
  uint8_t *ppa_strcpy(uint8_t *dest, const uint8_t *src);

/*! \brief   copy string, like strncpy
    \param[out] dest  destination buffer
    \param[in] src  source buffer
    \param[in] n maximum bytes to copy
    \note
*/
  uint8_t *ppa_strncpy(uint8_t *dest, const uint8_t *src, PPA_SIZE_T n);

/*! \brief   get string length, like strlen
    \param[in] s string buffer
    \return return the string length
    \note
*/
  PPA_SIZE_T  ppa_strlen(const uint8_t *s);

#if defined(CONFIG_LTQ_PPA_MPE_IP97)
/*! \brief   copy string, like strncpy
    \param[out] dest  destination buffer
    \param[in] src  source buffer
    \param[in] n maximum bytes to copy
    \note
*/
  int32_t ppa_str_cmp(char *str1,char *str2);

#endif
/*! \brief   shrink cache buffer. in linux, it is kmem_cache_shrink
    \param[in] p_cache Pointer to cache buffer
    \return return the string length
    \note
*/
  int32_t ppa_kmem_cache_shrink(PPA_MEM_CACHE *p_cache);

/*! \brief   lookup symble. In linux, it is kallsyms_lookup
    \note
*/
  const uint8_t *ppa_kallsyms_lookup(uint32_t addr, uint32_t *symbolsize, uint32_t *offset, uint8_t **modname, uint8_t *namebuf);

/*! \brief   register network device, in linux, it is register_netdev
    \param[in] dev pointer to network device
    \return
    \note
*/
  int32_t ppa_register_netdev(PPA_NETIF *dev);

/*! \brief   unregister network device, in linux, it is unregister_netdev
    \param[in] dev pointer to network device
    \return
    \note
*/
  void ppa_unregister_netdev(PPA_NETIF *dev);

/*! \brief   register char devide, in linux, it is register_chrdev
    \param[in]  major Character device major version
    \param[in]  name  Character device name
    \param[in]  fops  Character device operation pointer
    \return
    \note
*/
  int32_t ppa_register_chrdev(int32_t  major, const uint8_t *name, PPA_FILE_OPERATIONS  *fops);

/*! \brief   unregister char devide, in linux, it is unregister_chrdev
    \param[in]  major char device major version
    \param[in]  name char device name
    \return
    \note
*/
  void ppa_unregister_chrdev(int32_t  major, const uint8_t *name);

/*! \brief  format a string to buffer, in linux, it is snprintf
    \return
    \note
*/
  int ppa_snprintf(uint8_t* buf, size_t size, const uint8_t *fmt, ...);

/*! \brief  format a string to buffer, in linux, it is sprintf
    \return
    \note
*/
  int ppa_sprintf(uint8_t * buf, const uint8_t *fmt, ...);

/*! \brief  get ioctl type, in linux, it is _IOC_TYPE
    \return
    \note
*/
  uint32_t ppa_ioc_type(uint32_t);

/*! \brief  get ioctl nr, in linux, it is _IOC_NR
    \return
    \note
*/
  uint32_t ppa_ioc_nr(uint32_t);

/*! \brief  get ioctl dir, in linux, it is _IOC_DIR
    \return
    \note
*/
  uint32_t ppa_ioc_dir(uint32_t);

/*! \brief  get ioctl read flag, in linux, it is _IOC_READ
    \return
    \note
*/
  uint32_t ppa_ioc_read(void);

/*! \brief  get ioctl write flag, in linux, it is _IOC_WRITE
    \return
    \note
*/
  uint32_t ppa_ioc_write(void);

/*! \brief  get ioctl size, in linux, it is _IOC_SIZE
    \return
    \note
*/
  uint32_t ppa_ioc_size(uint32_t);

/*! \brief  check ioctl access right, in linux, it is access_ok
    \return
    \note
*/
  uint32_t ppa_ioc_access_ok(uint32_t type, uint32_t addr, uint32_t size);

/*! \brief  get ioctl verify write flag, in linux it is VERIFY_WRITE
    \return
    \note
*/
  uint32_t ppa_ioc_verify_write(void);

/*! \brief  get ioctl verify read flag, in linux it is VERIFY_READ
    \return
    \note
*/
  uint32_t ppa_ioc_verify_read(void);


/*! \brief  get egress qos mask
    \param dev pointer to net device structure.
	\param buf pointer to buffer.
    \return
    \note
*/
  uint16_t ppa_vlan_dev_get_egress_qos_mask(PPA_NETIF *dev, PPA_BUF *buf);

/*! \brief  get hash value computed using connection track.
    \param ct Pointer to PPA_SESSION
    \param dir Direction of the session.
    \param tuple Pointer to hold the connection tuple.
    \return On success returns calculated hash value, else returns zero
    \note
*/
  uint32_t ppa_get_hash_from_ct( const PPA_SESSION *ct, 
                                 uint8_t dir, 
                                 PPA_TUPLE* tuple);
/*! \brief  get hash value computed from given packet
    \param ppa_buf pointer to the packet buffer
    \param pf Protocol family of the packet.
    \param u32_hash pointer to unsigned integer. On success, this contains
          valid hash value computed using 5 tuples of the packet.
    \return On success returns zero
    \note
*/
  int ppa_get_hash_from_packet( PPA_BUF *ppa_buf, 
                             unsigned char pf, 
                             uint32_t *u32_hash, 
                             PPA_TUPLE* tuple );


/*! \brief  check if ifname is a slave of bond interface 
    \param ifname 
    \param netif 
    \return On success returns one
    \note
*/
  uint32_t ppa_is_bond_slave(PPA_IFNAME *ifname, PPA_NETIF *netif);
#endif  //  __KERNEL__
/* @} */


#endif  //  __IFX_PPA_STACK_AL_H__20081103_1153__
