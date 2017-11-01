/*******************************************************************************
**
** FILE NAME    : ppa_api_misc.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Miscellaneous Functions
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
** 03 NOV 2008  Xu Liang               Initiate Version
** 10 DEC 2012  Manamohan Shetty       Added the support for RTP,MIB mode and CAPWAP 
**                                     Features 
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#if defined(CONFIG_LTQ_PPA_API_PROC)
#include <linux/proc_fs.h>
#endif
#include <linux/netdevice.h>
#include <linux/in.h>
#include <net/sock.h>
#include <net/ip_vs.h>
#include <asm/time.h>

/*
 *  PPA Specific Head File
 */
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include <net/ppa_hook.h>
#include "ppa_api_misc.h"
#include "ppa_api_session.h"
#include "ppa_api_netif.h"
#if defined(CONFIG_LTQ_PPA_API_PROC)
#include "ppa_api_proc.h"
#endif
#include "ppa_api_tools.h"
#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
#include "ppa_api_mfe.h"
#endif
#ifdef CONFIG_LTQ_PPA_QOS
#include "ppa_api_qos.h"
#endif
#include "ppa_api_mib.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include "ppa_api_hal_selector.h"
#include "ppa_api_tools.h"

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
#include "ppa_api_sw_accel.h"
#include "ppa_sae_hal.h"
#endif

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
#include "ppa_api_session_limit.h"
#endif

/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  device constant
 */
#define PPA_CHR_DEV_MAJOR                       181
#define PPA_DEV_NAME                            "ppa"



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


static int ppa_dev_open(struct inode *, struct file *);
static int ppa_dev_release(struct inode *, struct file *);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int ppa_dev_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
#else
static long ppa_dev_ioctl(struct file *, unsigned int, unsigned long);
#endif
static int32_t ppa_get_all_vlan_filter_count(uint32_t);

#if defined(CONFIG_LTQ_PPA_API_PROC)
  static int print_fw_ver(char *, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
  static int print_ppa_subsystem_ver(char *, int, char *, unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int , unsigned int );
  static int print_driver_ver(char *, int, char *, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);

  static INLINE void proc_file_create(void);
  static INLINE void proc_file_delete(void);
  static int proc_read_ver(char *, char **, off_t, int, int *, void *);
#endif

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
  int32_t ppa_api_directpath_create(void);
  void ppa_api_directpath_destroy(void);
#endif

int32_t ppa_ioctl_bridge_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_get_bridge_enable_status(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_get_hook_list(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_set_hook(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_get_max_entry(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_get_portid(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_del_session(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_add_session(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_modify_session(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
int32_t ppa_ioctl_get_prio_sessions(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_manage_sessions(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_get_sessions_count(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
#endif
int32_t ppa_ioctl_set_session_timer(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_get_session_timer(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info);
int32_t ppa_ioctl_set_ppe_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_get_ppe_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
int32_t ppa_ioctl_set_sw_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_get_sw_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_set_sw_session_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
int32_t ppa_ioctl_get_sw_session_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info);
#endif

/*
 * ####################################
 *           Global Variable
 * ####################################
 */

#if defined(CONFIG_LTQ_PPA_API_PROC)
static int g_ppa_proc_dir_flag = 0;
static int g_ppa_api_proc_dir_flag = 0;
static struct proc_dir_entry *g_ppa_proc_dir = NULL;
static struct proc_dir_entry *g_ppa_api_proc_dir = NULL;
#endif


static struct file_operations g_ppa_dev_ops = {
    owner:      THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
    ioctl:      ppa_dev_ioctl,
#else
    unlocked_ioctl:      ppa_dev_ioctl,
#endif
    open:       ppa_dev_open,
    release:    ppa_dev_release
};

static char ZeroMAC[]={0,0,0,0,0,0};


/*
 * ####################################
 *           Extern Variable
 * ####################################
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    extern struct proc_dir_entry proc_root;
#endif



/*
 * ####################################
 *            Local Function
 * ####################################
 */

/*
 *  file operation functions
 */

static int ppa_dev_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int ppa_dev_release(struct inode *inode, struct file *file)
{
    return 0;
}

#ifdef CONFIG_LTQ_PPA_QOS
static unsigned int non_checking_cmd_list[]={ PPA_CMD_INIT, PPA_CMD_EXIT, PPA_CMD_GET_STATUS, \
    PPA_CMD_GET_COUNT_PPA_HASH, PPA_CMD_GET_VERSION, PPA_CMD_GET_SIZE, \
    PPA_CMD_GET_QOS_STATUS,PPA_CMD_GET_QOS_QUEUE_MAX_NUM, PPA_CMD_GET_QOS_MIB, PPA_CMD_GET_CTRL_QOS_WFQ, \
    PPA_CMD_GET_QOS_WFQ, PPA_CMD_GET_CTRL_QOS_RATE, PPA_CMD_GET_QOS_RATE,\
    PPA_CMD_GET_HOOK_LIST, PPA_CMD_SET_HOOK,PPA_CMD_READ_MEM, PPA_CMD_SET_MEM,\
    PPA_CMD_GET_MAX_ENTRY,PPA_CMD_GET_PORTID, \
    PPA_CMD_GET_DSL_MIB,PPA_CMD_CLEAR_DSL_MIB,PPA_CMD_GET_PORT_MIB,PPA_CMD_CLEAR_PORT_MIB, \
    PPA_CMD_DBG_TOOL 
};
#else 
static unsigned int non_checking_cmd_list[]={ PPA_CMD_INIT, PPA_CMD_EXIT, PPA_CMD_GET_STATUS, \
    PPA_CMD_GET_COUNT_PPA_HASH, PPA_CMD_GET_VERSION, PPA_CMD_GET_SIZE, \
    PPA_CMD_GET_HOOK_LIST, PPA_CMD_SET_HOOK,PPA_CMD_READ_MEM, PPA_CMD_SET_MEM,\
    PPA_CMD_GET_MAX_ENTRY,PPA_CMD_GET_PORTID, \
    PPA_CMD_GET_DSL_MIB,PPA_CMD_CLEAR_DSL_MIB,PPA_CMD_GET_PORT_MIB,PPA_CMD_CLEAR_PORT_MIB, \
    PPA_CMD_DBG_TOOL 
};
#endif

int32_t inline need_checking_ppa_status_cmd(unsigned int cmd )
{
    int check_flag = 1;
    int i;
    
    for(i=0; i< NUM_ENTITY(non_checking_cmd_list); i++ )
    {
        if( non_checking_cmd_list[i] == cmd )
        {
            check_flag = 0;
            break;
        }
    }

    return check_flag;
}

//#define FIX_MEM_SIZE  20
#if defined(FIX_MEM_SIZE)
struct session_list_item tmp_session_buf[FIX_MEM_SIZE];

#endif

static int ppa_ioctl_get_sessions(unsigned int cmd, PPA_CMD_SESSIONS_INFO *pSessIn, unsigned long arg)
{
  struct session_list_item *p_item_one_index=NULL;
  uint32_t   session_flag;
  uint32_t pos = 0, buff_len;
  uint32_t count=0, ret=PPA_SUCCESS;
  uint32_t i, tmp_num=0;
  //void *pUserSessionList =((void*)arg) + sizeof(pSessIn->count_info);
  void *pUserSessionList =((void*)arg) + offsetof(PPA_CMD_SESSIONS_INFO,session_list);

  if ( ppa_copy_from_user(&pSessIn->count_info, (void *)arg, sizeof(pSessIn->count_info)) != 0 )
      return -EFAULT;

  if( cmd == PPA_CMD_GET_LAN_SESSIONS )
    session_flag = SESSION_LAN_ENTRY;
  else if( cmd == PPA_CMD_GET_WAN_SESSIONS )
    session_flag = SESSION_WAN_ENTRY;
  else if( cmd == PPA_CMD_GET_LAN_WAN_SESSIONS )
    session_flag = SESSION_LAN_ENTRY | SESSION_WAN_ENTRY;
  else 
    return -EIO;

  if( pSessIn->count_info.hash_index )
  {
    pos = pSessIn->count_info.hash_index -1;
  }
  if( pSessIn->count_info.count == 0 )
    return -EIO;            

  do {

    struct session_list_item *pp_item;
#if defined(FIX_MEM_SIZE)
    p_item_one_index = tmp_session_buf;  
    buff_len = NUM_ENTITY(tmp_session_buf);
#else           

    if( p_item_one_index ) {
      ppa_free(p_item_one_index);
      p_item_one_index= NULL;
    }
    buff_len = 0;
#endif      
    tmp_num = 0;

    if( (ret == ppa_session_get_items_in_hash(pos,
        &p_item_one_index,buff_len,
        &tmp_num,
        pSessIn->count_info.stamp_flag)) == PPA_INDEX_OVERFLOW ) //end of list
    {   
      ret = PPA_SUCCESS;
      break;
    }
    pos ++;

    if( ret == PPA_ENOMEM) {
      critial_err("p_item_one_index NULL for no memory for hash index %d\n", pos);
      ret = PPA_FAILURE;
      break;
    }
    
    if( ret != PPA_SUCCESS ) continue;  
    if( tmp_num == 0 || !p_item_one_index ) continue;
                                
    for(i=0; i<tmp_num; i++) {

      pp_item = &p_item_one_index[i];
      if ( pp_item->flags & session_flag ) {
        if( pSessIn->count_info.flag == 0 ||/* get all LAN or WAN sessions */
            ( (pSessIn->count_info.flag == SESSION_ADDED_IN_HW) && (pp_item->flags & SESSION_ADDED_IN_HW) )  || /*get all accelerated sessions only */
            ( (pSessIn->count_info.flag == SESSION_ADDED_IN_SW) && (pp_item->flags & SESSION_ADDED_IN_SW) ) ||
            ( (pSessIn->count_info.flag == SESSION_NON_ACCE_MASK) && 
                              !(pp_item->flags & SESSION_ADDED_IN_HW ) && !(pp_item->flags & SESSION_ADDED_IN_SW ) ) )
        {
           
          if( count >= pSessIn->count_info.count ) break;  /*buffer is full. We should stop now*/
          pSessIn->session_list[0].ip_proto = pp_item->ip_proto;
          pSessIn->session_list[0].ip_tos = pp_item->ip_tos;
          ppa_memcpy( &pSessIn->session_list[0].src_ip, &pp_item->src_ip, sizeof(pSessIn->session_list[0].src_ip));
          pSessIn->session_list[0].src_port = pp_item->src_port;
          ppa_memcpy( &pSessIn->session_list[0].dst_ip, &pp_item->dst_ip, sizeof(pSessIn->session_list[0].dst_ip));
          pSessIn->session_list[0].dst_port = pp_item->dst_port;
          ppa_memcpy( &pSessIn->session_list[0].nat_ip, &pp_item->nat_ip, sizeof(pSessIn->session_list[0].nat_ip));
          pSessIn->session_list[0].nat_port= pp_item->nat_port;
          pSessIn->session_list[0].new_dscp= pp_item->new_dscp;
          pSessIn->session_list[0].new_vci = pp_item->new_vci;
          pSessIn->session_list[0].dslwan_qid = pp_item->dslwan_qid;
          pSessIn->session_list[0].dest_ifid = pp_item->dest_ifid;
          pSessIn->session_list[0].flags= pp_item->flags;
          pSessIn->session_list[0].flag2 = pp_item->flag2;
          pSessIn->session_list[0].mips_bytes = pp_item->mips_bytes - pp_item->prev_clear_mips_bytes;
          pSessIn->session_list[0].hw_bytes = pp_item->acc_bytes - pp_item->prev_clear_acc_bytes;
          pSessIn->session_list[0].session = (uint32_t ) pp_item->session;
          pSessIn->session_list[0].hash = (uint32_t ) pp_item->hash;
          pSessIn->session_list[0].collision_flag = (uint32_t ) pp_item->collision_flag;
#if defined(SKB_PRIORITY_DEBUG) && SKB_PRIORITY_DEBUG
          pSessIn->session_list[0].priority = (uint32_t ) pp_item->priority;
#endif
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
			    if( ppa_get_session_limit_enable(0) )
	          pSessIn->session_list[0].session_priority = (uint32_t ) pp_item->session_priority;
			    else
	          pSessIn->session_list[0].session_priority = 0;
				
#endif
            
#if defined(SESSION_STATISTIC_DEBUG) && SESSION_STATISTIC_DEBUG 

          /*below variable is used for session management debugging purpose */   
          pSessIn->session_list[0].hash_index = (uint32_t ) pp_item->hash_index;
          pSessIn->session_list[0].hash_table_id = (uint32_t ) pp_item->hash_table_id;                            
#endif     
          pSessIn->session_list[0].prev_sess_bytes = pp_item->prev_sess_bytes;
         
	  if(pp_item->rx_if) 
          ppa_strncpy( (void *)pSessIn->session_list[0].rx_if_name, (void *)ppa_get_netif_name(pp_item->rx_if), sizeof(pSessIn->session_list[0].rx_if_name) );
	  if(pp_item->tx_if)
          ppa_strncpy( (void *)pSessIn->session_list[0].tx_if_name, (void *)ppa_get_netif_name(pp_item->tx_if), sizeof(pSessIn->session_list[0].tx_if_name) );


          if ( ppa_copy_to_user( pUserSessionList + count*sizeof(pSessIn->session_list[0]) , 
                                 pSessIn->session_list, 
                                 sizeof(pSessIn->session_list[0] )) != 0 )
          {
            if(p_item_one_index){
              
              ppa_free(p_item_one_index);
              p_item_one_index= NULL;
            }
            return -EFAULT;
          }
                            
          count ++;                         
        }
      }              
    }

    if( pSessIn->count_info.hash_index ) 
      break; /* retrive only from ONE hash(bucket) */
           
  } while(count < pSessIn->count_info.count );

#if defined(FIX_MEM_SIZE)           
#else

  if( p_item_one_index ) {
      
    ppa_free(p_item_one_index);
    p_item_one_index= NULL;
  }
#endif
  pSessIn->count_info.count = count;  //update the real session number to user space
  if ( ppa_copy_to_user(  (void *)arg, &pSessIn->count_info, sizeof(pSessIn->count_info)) != 0 )
  {
    return -EFAULT;
  }
  if( ret == PPA_SUCCESS ) 
    return 0;
  
  return -EINVAL;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static int ppa_dev_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#else
static long ppa_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
    int res = 0;
    PPA_CMD_DATA *cmd_info = (PPA_CMD_DATA *)ppa_malloc(sizeof(PPA_CMD_DATA));

    if ( cmd_info == NULL )
        return -EFAULT;

    if ( ppa_ioc_type(cmd) != PPA_IOC_MAGIC )
    {
        printk("ppa_ioc_type(%08X - %d) != PPA_IOC_MAGIC(%d)\n", cmd, _IOC_TYPE(cmd), PPA_IOC_MAGIC);
        goto EXIT_EIO;
    }
    else if( ppa_ioc_nr(cmd) >= PPA_IOC_MAXNR )
    {
        printk("Current cmd is %02x wrong, it should less than %02x\n", _IOC_NR(cmd), PPA_IOC_MAXNR );
        goto EXIT_EIO;
    }

    if ( ((ppa_ioc_dir(cmd) & ppa_ioc_read()) && !access_ok(VERIFY_WRITE, arg, ppa_ioc_size(cmd)))
        || ((ppa_ioc_dir(cmd) & ppa_ioc_write()) && !access_ok(VERIFY_READ, arg, ppa_ioc_size(cmd))) )
    {
        printk("access check: (%08X && %d) || (%08X && %d)\n", 
            (ppa_ioc_dir(cmd) & ppa_ioc_read()), 
            (int)!ppa_ioc_access_ok(ppa_ioc_verify_write(), arg, ppa_ioc_size(cmd)),
            (ppa_ioc_dir(cmd) & ppa_ioc_write()), 
            (int)!ppa_ioc_access_ok(ppa_ioc_verify_read(), arg, ppa_ioc_size(cmd)));
        goto EXIT_EFAULT;
    }

    if( need_checking_ppa_status_cmd(cmd))
    {
        if( !ppa_is_init())
        {
            goto EXIT_EFAULT;
        }
    }

    switch ( cmd )
    {
    case PPA_CMD_INIT:
        {
            PPA_INIT_INFO info;
            PPA_IFINFO ifinfo[ 2 * PPA_MAX_IFS_NUM ];
            int i;

            if ( ppa_copy_from_user(&cmd_info->init_info, (void *)arg, sizeof(cmd_info->init_info)) != 0 )
                goto EXIT_EFAULT;

            ppa_memset(&info, 0, sizeof(info));
            ppa_memset(ifinfo, 0, sizeof(ifinfo));

            info.lan_rx_checks          = cmd_info->init_info.lan_rx_checks;
            info.wan_rx_checks          = cmd_info->init_info.wan_rx_checks;
            info.num_lanifs             = cmd_info->init_info.num_lanifs;
            info.p_lanifs               = ifinfo;
            info.num_wanifs             = cmd_info->init_info.num_wanifs;
            info.p_wanifs               = ifinfo + NUM_ENTITY(cmd_info->init_info.p_lanifs);
            info.max_lan_source_entries = cmd_info->init_info.max_lan_source_entries;
            info.max_wan_source_entries = cmd_info->init_info.max_wan_source_entries;
            info.max_mc_entries         = cmd_info->init_info.max_mc_entries;
            info.max_bridging_entries   = cmd_info->init_info.max_bridging_entries;
            if( cmd_info->init_info.add_requires_min_hits )
            {
                info.add_requires_min_hits  = cmd_info->init_info.add_requires_min_hits;
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"add_requires_min_hits is set to %d\n", info.add_requires_min_hits);
            }
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
            if( cmd_info->init_info.add_requires_lan_collisions )
            {
                info.add_requires_lan_collisions  = cmd_info->init_info.add_requires_lan_collisions;
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"add_requires_lan_collisions is set to %d\n", info.add_requires_lan_collisions);
            }
            if( cmd_info->init_info.add_requires_wan_collisions )
            {
                info.add_requires_wan_collisions  = cmd_info->init_info.add_requires_wan_collisions;
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"add_requires_wan_collisions is set to %d\n", info.add_requires_wan_collisions);
            }
#endif
            if( cmd_info->init_info.mtu )
            {
                g_ppa_ppa_mtu = cmd_info->init_info.mtu;
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"g_ppa_ppa_mtu is set to %d at %p\n", g_ppa_ppa_mtu, &g_ppa_ppa_mtu);
            }
            for ( i = 0; i < info.num_lanifs; i++ )
            {
                info.p_lanifs[i].ifname   = cmd_info->init_info.p_lanifs[i].ifname;
                info.p_lanifs[i].if_flags = cmd_info->init_info.p_lanifs[i].if_flags;
            }

            for ( i = 0; i < info.num_wanifs; i++ )
            {
                info.p_wanifs[i].ifname   = cmd_info->init_info.p_wanifs[i].ifname;
                info.p_wanifs[i].if_flags = cmd_info->init_info.p_wanifs[i].if_flags;
            }

            if ( ppa_init(&info, cmd_info->init_info.flags) != PPA_SUCCESS )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_init fail");
                goto EXIT_EIO;
            }
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE
            if ( ppa_set_mib_mode(cmd_info->init_info.mib_mode) != PPA_SUCCESS )
                goto EXIT_EIO;
#endif
            else
                goto EXIT_ZERO;

        }

    case PPA_CMD_EXIT:
        {
            ppa_enable(0, 0, 0);
            ppa_exit();
            goto EXIT_ZERO;
        }

    case PPA_CMD_ENABLE:
        {
            if ( ppa_copy_from_user(&cmd_info->ena_info, (void *)arg, sizeof(cmd_info->ena_info)) != 0 )
                goto EXIT_EFAULT;

            if ( ppa_enable(cmd_info->ena_info.lan_rx_ppa_enable, cmd_info->ena_info.wan_rx_ppa_enable, cmd_info->ena_info.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }
#if defined(MIB_MODE_ENABLE) && MIB_MODE_ENABLE

    case PPA_CMD_SET_MIB_MODE:
        {
            if ( ppa_copy_from_user(&cmd_info->mib_mode_info, (void *)arg, sizeof(cmd_info->mib_mode_info)) != 0 )
                goto EXIT_EFAULT;

            ppa_ioctl_clear_ports_mib(0, 0, NULL);

            if ( ppa_set_mib_mode(cmd_info->mib_mode_info.mib_mode) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;

            ppa_ioctl_clear_ports_mib(0, 0, NULL);
        }

    case PPA_CMD_GET_MIB_MODE:
        {
            if ( ppa_copy_from_user(&cmd_info->mib_mode_info, (void *)arg, sizeof(cmd_info->mib_mode_info)) != 0 )
                goto EXIT_EFAULT;

            if ( ppa_get_mib_mode(&cmd_info->mib_mode_info.mib_mode) != PPA_SUCCESS )
                goto EXIT_EIO;
        
            if ( ppa_copy_to_user((void *)arg, &cmd_info->mib_mode_info, sizeof(cmd_info->mib_mode_info)) != 0 )
                goto EXIT_EFAULT;
            else
                goto EXIT_ZERO;
        }

#endif
    case PPA_CMD_GET_STATUS:
        {
            cmd_info->ena_info.lan_rx_ppa_enable = 0;
            cmd_info->ena_info.wan_rx_ppa_enable = 0;

            if ( ppa_copy_from_user(&cmd_info->ena_info, (void *)arg, sizeof(cmd_info->ena_info)) != 0 )
                goto EXIT_EFAULT;

            if ( ppa_get_status(&cmd_info->ena_info.lan_rx_ppa_enable, &cmd_info->ena_info.wan_rx_ppa_enable, cmd_info->ena_info.flags) != PPA_SUCCESS )
                  cmd_info->ena_info.flags = 0;
            else
                 cmd_info->ena_info.flags = 1;
            if ( ppa_copy_to_user((void *)arg, &cmd_info->ena_info, sizeof(cmd_info->ena_info)) != 0 )
                goto EXIT_EFAULT;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_MODIFY_MC_ENTRY:
        {
            if ( ppa_copy_from_user(&cmd_info->mc_entry, (void *)arg, sizeof(cmd_info->mc_entry)) != 0 )
                goto EXIT_EFAULT;

            if ( ppa_mc_entry_modify(cmd_info->mc_entry.mcast_addr,cmd_info->mc_entry.source_ip, NULL, &cmd_info->mc_entry.mc_extra, cmd_info->mc_entry.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_GET_MC_ENTRY:
        {
            if ( ppa_copy_from_user(&cmd_info->mc_entry, (void *)arg, sizeof(cmd_info->mc_entry)) != 0 )
                goto EXIT_EFAULT;

            ppa_memset(&cmd_info->mc_entry.mc_extra, 0, sizeof(cmd_info->mc_entry.mc_extra));

            if ( ppa_mc_entry_get(cmd_info->mc_entry.mcast_addr,cmd_info->mc_entry.source_ip, &cmd_info->mc_entry.mc_extra, -1) != PPA_SUCCESS ) //-1: to get all flag related parameters
                goto EXIT_EIO;
            else if ( ppa_copy_to_user((void *)arg, &cmd_info->mc_entry, sizeof(cmd_info->mc_entry)) != 0 )
                goto EXIT_EFAULT;
            else
                goto EXIT_ZERO;
        }

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
    case PPA_CMD_SET_RTP:
        {

            PPA_MC_GROUP mc_group;
            int32_t res;

            ppa_memset( &mc_group, 0, sizeof(mc_group));
            if ( ppa_copy_from_user(&cmd_info->mc_entry, (void *)arg, sizeof(cmd_info->mc_entry)) != 0 )
                goto EXIT_EFAULT;

            ppa_memcpy( &mc_group.ip_mc_group, &cmd_info->mc_entry.mcast_addr, sizeof(mc_group.ip_mc_group) ) ;            //  multicast address: 3
            ppa_memcpy( &mc_group.source_ip, &cmd_info->mc_entry.source_ip, sizeof(mc_group.source_ip));

            mc_group.RTP_flag = cmd_info->mc_entry.RTP_flag;
            res = ppa_mc_entry_rtp_set(&mc_group);
            if ( res != PPA_SUCCESS )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_mc_entry_rtp_set fail\n");
                goto EXIT_EIO;
            }
            else
                goto EXIT_ZERO;
        }

#endif

    case PPA_CMD_ADD_MAC_ENTRY:
        {
            PPA_NETIF *netif, *brif;

            if ( ppa_copy_from_user(&cmd_info->mac_entry, (void *)arg, sizeof(cmd_info->mac_entry)) != 0 )
                goto EXIT_EFAULT;

            netif = ppa_get_netif(cmd_info->mac_entry.ifname);
            if ( !netif )
                goto EXIT_EIO;
		
	    brif = ppa_get_netif(cmd_info->mac_entry.brname);
	    if( !brif) 
		goto EXIT_EIO;

            if ( ppa_bridge_entry_add(cmd_info->mac_entry.mac_addr, brif, netif, cmd_info->mac_entry.flags) != PPA_SESSION_ADDED )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_DEL_MAC_ENTRY:
        {
	    PPA_NETIF *brif;

            if ( ppa_copy_from_user(&cmd_info->mac_entry, (void *)arg, sizeof(cmd_info->mac_entry)) != 0 )
                goto EXIT_EFAULT;

	    brif = ppa_get_netif(cmd_info->mac_entry.brname);
            if( !brif)
                goto EXIT_EIO;

            if ( ppa_bridge_entry_delete(cmd_info->mac_entry.mac_addr, brif,cmd_info->mac_entry.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_SET_VLAN_IF_CFG:
        {
            PPA_NETIF *netif;

            if ( ppa_copy_from_user(&cmd_info->br_vlan, (void *)arg, sizeof(cmd_info->br_vlan)) != 0 )
                goto EXIT_EFAULT;

            netif = ppa_get_netif(cmd_info->br_vlan.if_name);
            if ( !netif )
                goto EXIT_EIO;

            if ( ppa_set_bridge_if_vlan_config(netif, &cmd_info->br_vlan.vlan_tag_ctrl, &cmd_info->br_vlan.vlan_cfg, cmd_info->br_vlan.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_GET_VLAN_IF_CFG:
        {
            PPA_NETIF *netif;

            if ( ppa_copy_from_user(&cmd_info->br_vlan, (void *)arg, sizeof(cmd_info->br_vlan)) != 0 )
                goto EXIT_EFAULT;

            netif = ppa_get_netif(cmd_info->br_vlan.if_name);
            if ( !netif )
                goto EXIT_EIO;

            ppa_memset(&cmd_info->br_vlan.vlan_tag_ctrl, 0, sizeof(cmd_info->br_vlan.vlan_tag_ctrl));
            ppa_memset(&cmd_info->br_vlan.vlan_cfg, 0, sizeof(cmd_info->br_vlan.vlan_cfg));

            if ( ppa_get_bridge_if_vlan_config(netif, &cmd_info->br_vlan.vlan_tag_ctrl, &cmd_info->br_vlan.vlan_cfg, cmd_info->br_vlan.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else if ( ppa_copy_to_user((void *)arg, &cmd_info->br_vlan, sizeof(cmd_info->br_vlan)) != 0 )
               goto EXIT_EFAULT;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_ADD_VLAN_FILTER_CFG:
        {
            PPA_VLAN_MATCH_FIELD field;
            PPA_VLAN_INFO info;
            PPA_IFINFO ifinfo[PPA_MAX_IFS_NUM];
            struct netif_info *netifinfo;
            int i;

            if ( ppa_copy_from_user(&cmd_info->vlan_filter, (void *)arg, sizeof(cmd_info->vlan_filter)) != 0 )
                goto EXIT_EFAULT;

            field.match_flags = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_flags;
            switch ( field.match_flags )
            {
            case PPA_F_VLAN_FILTER_IFNAME:      field.match_field.ifname = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.ifname; break;
            case PPA_F_VLAN_FILTER_IP_SRC:      field.match_field.ip_src = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.ip_src; break;
            case PPA_F_VLAN_FILTER_ETH_PROTO:   field.match_field.eth_protocol = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.eth_protocol; break;
            case PPA_F_VLAN_FILTER_VLAN_TAG:    field.match_field.ingress_vlan_tag = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.ingress_vlan_tag; break;
            default: goto EXIT_EINVAL;
            }

            info.vlan_vci = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.vlan_vci;
            info.out_vlan_id = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.out_vlan_id;
            info.qid = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.qid;
            info.inner_vlan_tag_ctrl = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.inner_vlan_tag_ctrl;
            info.out_vlan_tag_ctrl = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.out_vlan_tag_ctrl;
            info.num_ifs = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.num_ifs;
            info.vlan_if_membership = ifinfo;

            for ( i = 0; i < info.num_ifs; i++ )
            {
                if ( ppa_netif_lookup(cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.vlan_if_membership[i].ifname, &netifinfo) != PPA_SUCCESS )
                {
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"%s is not a valid interfce\n", cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.vlan_if_membership[i].ifname);
                    goto EXIT_EIO;
                }
                if( info.qid == PPA_INVALID_QID )
                {
                    info.qid = netifinfo->dslwan_qid;
                }
                ppa_netif_put(netifinfo);

                ifinfo[i].ifname = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.vlan_if_membership[i].ifname;
                ifinfo[i].if_flags = cmd_info->vlan_filter.vlan_filter_cfg.vlan_info.vlan_if_membership[i].if_flags;

            }

            if ( ppa_vlan_filter_add(&field, &info, cmd_info->vlan_filter.flags) != PPA_SUCCESS )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_vlan_filter_add fail\n");
                goto EXIT_EIO;
            }
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_DEL_VLAN_FILTER_CFG:
        {
            PPA_VLAN_MATCH_FIELD field;

            if ( ppa_copy_from_user(&cmd_info->vlan_filter, (void *)arg, sizeof(cmd_info->vlan_filter)) != 0 )
                goto EXIT_EFAULT;

            field.match_flags = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_flags;
            switch ( field.match_flags )
            {
            case PPA_F_VLAN_FILTER_IFNAME:      field.match_field.ifname = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.ifname; break;
            case PPA_F_VLAN_FILTER_IP_SRC:      field.match_field.ip_src = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.ip_src; break;
            case PPA_F_VLAN_FILTER_ETH_PROTO:   field.match_field.eth_protocol = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.eth_protocol; break;
            case PPA_F_VLAN_FILTER_VLAN_TAG:    field.match_field.ingress_vlan_tag = cmd_info->vlan_filter.vlan_filter_cfg.match_field.match_field.ingress_vlan_tag; break;
            default: goto EXIT_EINVAL;
            }

            if ( ppa_vlan_filter_del(&field, NULL, cmd_info->vlan_filter.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_GET_ALL_VLAN_FILTER_CFG:
        {
            uint32_t if_num, i, j, k, count;
            uint32_t vlan_filter_type[]={PPA_F_VLAN_FILTER_IFNAME, PPA_F_VLAN_FILTER_IP_SRC, PPA_F_VLAN_FILTER_ETH_PROTO, PPA_F_VLAN_FILTER_VLAN_TAG};
            PPA_IFNAME *PhysicalPortName[PPA_MAX_IFS_NUM]={"eth0", "wan","CPU0", "EXT0", "EXT1", "EXT2", "EXT3", "EXT4"};
            PPE_BRDG_VLAN_FILTER_MAP filter_map={0};

            ppa_memset( &cmd_info->all_vlan_filter, 0, sizeof(cmd_info->all_vlan_filter) );
             if ( ppa_copy_from_user(&cmd_info->all_vlan_filter.count_info, (void *)arg, sizeof(cmd_info->all_vlan_filter.count_info)) != 0 )
                goto EXIT_EFAULT;

            if( cmd_info->all_vlan_filter.count_info.count == 0 )
                goto EXIT_ZERO;

            count = 0;
            for(i=0; i<sizeof(vlan_filter_type)/sizeof(vlan_filter_type[0]); i++ )
            {
                PPE_VFILTER_COUNT_CFG vfilter_count={0};

                vfilter_count.vfitler_type = vlan_filter_type[i];
                ppa_drv_get_max_vfilter_entries( &vfilter_count, 0);

                for(j=0; j<vfilter_count.num; j++ )
                {
                    ppa_memset( &filter_map, 0, sizeof(filter_map));
                    ppa_memset( cmd_info->all_vlan_filter.filters, 0, sizeof(cmd_info->all_vlan_filter.filters) );

                     filter_map.ig_criteria_type = vlan_filter_type[i];
                     filter_map.entry = j;

                     if( ppa_drv_get_vlan_map(&filter_map, 0 ) == 1 )
                    {
                        if( ppa_drv_get_outer_vlan_entry(&filter_map.out_vlan_info, 0) == PPA_SUCCESS )
                        {
                            cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.out_vlan_id = filter_map.out_vlan_info.vlan_id;
                        }

                        cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.match_field.match_flags = vlan_filter_type[i];
                        if( vlan_filter_type[i] == PPA_F_VLAN_FILTER_IFNAME )
                        {
                            if( filter_map.ig_criteria >= PPA_MAX_IFS_NUM )
                            {
                                printk("Why ig_criteria (%d) big than %d for PPA_CMD_GET_ALL_VLAN_FILTER_CFG\n", filter_map.ig_criteria, PPA_MAX_IFS_NUM );
                                continue;
                            }
                            if( filter_map.ig_criteria < PPA_MAX_IFS_NUM )
                                ppa_strncpy( cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.match_field.match_field.ifname,
                                                        PhysicalPortName[ filter_map.ig_criteria],
                                                        sizeof(cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.match_field.match_field.ifname) );
                            else
                                filter_map.ig_criteria = 0;
                        }
                        else if( vlan_filter_type[i] == PPA_F_VLAN_FILTER_IP_SRC )
                            cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.match_field.match_field.ip_src = filter_map.ig_criteria;
                        else if( vlan_filter_type[i] == PPA_F_VLAN_FILTER_ETH_PROTO)
                            cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.match_field.match_field.eth_protocol = filter_map.ig_criteria;
                        else if( vlan_filter_type[i] == PPA_F_VLAN_FILTER_VLAN_TAG)
                            cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.match_field.match_field.ingress_vlan_tag = filter_map.ig_criteria;
                        else
                        {
                            printk("unknown type: %x\n", vlan_filter_type[i]  );
                            break;
                        }

                         //Since PPA dont' save vlan filter's original network interface name, so here we have to use faked one
                        if_num = 0;
                        for(k=0; k<PPA_MAX_IFS_NUM; k++ )
                        {
                            if( filter_map.vlan_port_map & (1 << k) )
                            {
                                ppa_strncpy( cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.vlan_if_membership[if_num].ifname, PhysicalPortName[k], sizeof(cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.vlan_if_membership[if_num].ifname) );
                                if_num ++;
                            }
                        }

                        cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.vlan_vci = filter_map.new_vci;

                        cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.out_vlan_tag_ctrl = filter_map.in_out_etag_ctrl;
                        cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.inner_vlan_tag_ctrl = filter_map.in_out_etag_ctrl;
                        cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.qid = filter_map.dest_qos;
                        cmd_info->all_vlan_filter.filters[0].vlan_filter_cfg.vlan_info.num_ifs = if_num;

                        if ( ppa_copy_to_user(  (void *)(arg + (void *)cmd_info->all_vlan_filter.filters - (void *)&cmd_info->all_vlan_filter.count_info + count * sizeof(cmd_info->all_vlan_filter.filters[0])) , cmd_info->all_vlan_filter.filters,  sizeof(cmd_info->all_vlan_filter.filters[0])) != 0 )
                            goto EXIT_EFAULT;

                        count ++;

                        if( count == cmd_info->all_vlan_filter.count_info.count  )
                            break;
                    }
                }
            }

            cmd_info->all_vlan_filter.count_info.count = count;  //update the real session number to user space

            if ( ppa_copy_to_user(  (void *)arg, &cmd_info->all_vlan_filter.count_info, sizeof(cmd_info->all_vlan_filter.count_info)) != 0 )
                 goto EXIT_EFAULT;

            goto EXIT_ZERO;
        }

    case PPA_CMD_DEL_ALL_VLAN_FILTER_CFG:
        {
            ppa_vlan_filter_del_all(0);
            goto EXIT_ZERO;
        }

    case PPA_CMD_SET_IF_MAC:
        {
            if ( ppa_copy_from_user(&cmd_info->if_mac, (void *)arg, sizeof(cmd_info->if_mac)) != 0 )
                goto EXIT_EFAULT;

            if ( ppa_set_if_mac_address(cmd_info->if_mac.ifname, cmd_info->if_mac.mac, cmd_info->if_mac.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_GET_IF_MAC:
        {
            if ( ppa_copy_from_user(&cmd_info->if_mac, (void *)arg, sizeof(cmd_info->if_mac)) != 0 )
                goto EXIT_EFAULT;

            ppa_memset(cmd_info->if_mac.mac, 0, sizeof(cmd_info->if_mac.mac));

            if ( ppa_get_if_mac_address(cmd_info->if_mac.ifname, cmd_info->if_mac.mac, cmd_info->if_mac.flags) != PPA_SUCCESS )
                goto EXIT_EIO;
            else if ( ppa_copy_to_user((void *)arg, &cmd_info->if_mac, sizeof(cmd_info->if_mac)) != 0 )
                goto EXIT_EFAULT;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_ADD_LAN_IF:
    case PPA_CMD_ADD_WAN_IF:
        {
            PPA_IFINFO ifinfo = {0};

            if ( ppa_copy_from_user(&cmd_info->if_info, (void *)arg, sizeof(cmd_info->if_info)) != 0 )
                goto EXIT_EFAULT;

            ifinfo.ifname = cmd_info->if_info.ifname;
            ifinfo.ifname_lower = cmd_info->if_info.ifname_lower;
            ifinfo.force_wanitf_flag = cmd_info->if_info.force_wanitf_flag;
            if ( cmd == PPA_CMD_ADD_LAN_IF )
                ifinfo.if_flags = PPA_F_LAN_IF;
            if ( ppa_add_if(&ifinfo, 0) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_DEL_LAN_IF:
    case PPA_CMD_DEL_WAN_IF:
        {
            PPA_IFINFO ifinfo = {0};

            if ( ppa_copy_from_user(&cmd_info->if_info, (void *)arg, sizeof(cmd_info->if_info)) != 0 )
                goto EXIT_EFAULT;

            ifinfo.ifname = cmd_info->if_info.ifname;
            ifinfo.hw_disable = cmd_info->if_info.hw_disable;
            if ( cmd == PPA_CMD_DEL_LAN_IF )
                ifinfo.if_flags = PPA_F_LAN_IF;
            if ( ppa_del_if(&ifinfo, 0) != PPA_SUCCESS )
                goto EXIT_EIO;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_GET_LAN_IF:
    case PPA_CMD_GET_WAN_IF:
        {
            struct netif_info *ifinfo;
            uint32_t pos = 0;
            int i = 0;
            uint32_t flag_mask = ( cmd == PPA_CMD_GET_LAN_IF)  ? NETIF_LAN_IF : NETIF_WAN_IF;
            uint32_t if_flags = (cmd == PPA_CMD_GET_LAN_IF) ? PPA_F_LAN_IF : 0;

            ppa_memset(&cmd_info->all_if_info, 0, sizeof(cmd_info->all_if_info));

            if ( ppa_netif_start_iteration(&pos, &ifinfo) == PPA_SUCCESS )
            {
                do
                {
                    if ( (ifinfo->flags & flag_mask) )
                    {
                        strcpy(cmd_info->all_if_info.ifinfo[i].ifname, ifinfo->name);
                        cmd_info->all_if_info.ifinfo[i].if_flags = if_flags;

#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB                        
                        cmd_info->all_if_info.ifinfo[i].acc_rx = ifinfo->hw_accel_stats.rx_bytes - ifinfo->prev_clear_acc_rx;
                        cmd_info->all_if_info.ifinfo[i].acc_tx = ifinfo->hw_accel_stats.tx_bytes - ifinfo->prev_clear_acc_tx;
#endif
                        if ( ++i >= NUM_ENTITY(cmd_info->all_if_info.ifinfo) )
                            break;
                    }
                } while ( ppa_netif_iterate_next(&pos, &ifinfo) == PPA_SUCCESS );
            }

            ppa_netif_stop_iteration();

            cmd_info->all_if_info.num_ifinfos = i;

            if ( ppa_copy_to_user((void *)arg, &cmd_info->all_if_info, sizeof(cmd_info->all_if_info)) != 0 )
                goto EXIT_EFAULT;
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_ADD_MC:
        {
            PPA_MC_GROUP mc_group;
            int32_t res;
            int i, idx = 0;

            ppa_memset( &mc_group, 0, sizeof(mc_group));
            if ( ppa_copy_from_user(&cmd_info->mc_add_info, (void *)arg, sizeof(cmd_info->mc_add_info)) != 0 )
                goto EXIT_EFAULT;

            if ( cmd_info->mc_add_info.num_ifs > PPA_MAX_MC_IFS_NUM )
            {
                /*  if num_ifs is zero, it means to remove the multicast session from PPE HW    */
                printk("Error cmd_info->mc_add_info.num_ifs=%d is over max limit %d", cmd_info->mc_add_info.num_ifs, PPA_MAX_MC_IFS_NUM);
                goto EXIT_EIO;
            }

            for ( i = 0, idx = 0; i < cmd_info->mc_add_info.num_ifs; i++ )
            {
                mc_group.array_mem_ifs[i].ifname = cmd_info->mc_add_info.lan_ifname[i];  //  downstream interfce: 1

                mc_group.if_mask |= 1 << idx;
                idx++;
            }
            mc_group.num_ifs = idx; //  downstream interfce number

            if ( ppa_strlen(cmd_info->mc_add_info.src_ifname) )                              //  upstream interfce: 2
            {
                mc_group.src_ifname = cmd_info->mc_add_info.src_ifname;
            }

            ppa_memcpy( &mc_group.ip_mc_group, &cmd_info->mc_add_info.mc.mcast_addr, sizeof(mc_group.ip_mc_group) ) ;            //  multicast address: 3
            ppa_memcpy( &mc_group.source_ip, &cmd_info->mc_add_info.mc.source_ip, sizeof(mc_group.source_ip));
            mc_group.SSM_flag = cmd_info->mc_add_info.mc.SSM_flag ;
            mc_group.bridging_flag = cmd_info->mc_add_info.bridging_flag;                //  multicast mode: 4
            if( ppa_memcmp( cmd_info->mc_add_info.mac, ZeroMAC, PPA_ETH_ALEN ) != 0 )
                ppa_memcpy(mc_group.mac, cmd_info->mc_add_info.mac, PPA_ETH_ALEN);               //  multicast new mac address: 5. Note, it is not used at present. only for future

            if ( cmd_info->mc_add_info.mc.flags & PPA_F_SESSION_VLAN )                                  //  flag for modifying inner vlan
            {
                mc_group.vlan_insert        = cmd_info->mc_add_info.mc.mc_extra.vlan_insert;
                mc_group.vlan_remove        = cmd_info->mc_add_info.mc.mc_extra.vlan_remove;
                mc_group.vlan_prio          = cmd_info->mc_add_info.mc.mc_extra.vlan_prio;
                mc_group.vlan_cfi           = cmd_info->mc_add_info.mc.mc_extra.vlan_cfi;
                mc_group.vlan_id            = cmd_info->mc_add_info.mc.mc_extra.vlan_id;
            }

            if ( cmd_info->mc_add_info.mc.flags & PPA_F_SESSION_OUT_VLAN )                                  //  flag for modifying outer vlan
            {
                mc_group.out_vlan_insert    = cmd_info->mc_add_info.mc.mc_extra.out_vlan_insert;
                mc_group.out_vlan_remove    = cmd_info->mc_add_info.mc.mc_extra.out_vlan_remove;
                mc_group.out_vlan_tag       = cmd_info->mc_add_info.mc.mc_extra.out_vlan_tag;
            }

            if ( cmd_info->mc_add_info.mc.flags & PPA_F_SESSION_NEW_DSCP )                                  //  flag for modifying dscp
            {
                mc_group.new_dscp_en        = cmd_info->mc_add_info.new_dscp_en;
                mc_group.new_dscp           = cmd_info->mc_add_info.mc.mc_extra.new_dscp;
            }

            mc_group.dslwan_qid_remark = cmd_info->mc_add_info.mc.mc_extra.dslwan_qid_remark;
            mc_group.dslwan_qid = cmd_info->mc_add_info.mc.mc_extra.dslwan_qid;

            res = ppa_mc_group_update( &mc_group, cmd_info->mc_add_info.mc.flags);

            if ( res != PPA_SUCCESS )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_mc_group_update fail\n");
                goto EXIT_EIO;
            }
            else
                goto EXIT_ZERO;
        }
#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG

    case PPA_CMD_ADD_CAPWAP:
        {
            int32_t res;

            if ( ppa_copy_from_user(&cmd_info->capwap_add_info, (void *)arg, sizeof(cmd_info->capwap_add_info)) != 0 )
                goto EXIT_EFAULT;

            if(ppa_get_capwap_count() > MAX_CAPWAP_ENTRIES)
                 goto EXIT_EFAULT;

            if ( cmd_info->capwap_add_info.num_ifs >= PPA_MAX_CAPWAP_IFS_NUM )
            {
                printk("Error cmd_info->capwap_add_info.num_ifs=%d is over max limit %d", cmd_info->capwap_add_info.num_ifs, PPA_MAX_CAPWAP_IFS_NUM);
                goto EXIT_EIO;
            }

            if ( (cmd_info->capwap_add_info.directpath_portid < 3) || (cmd_info->capwap_add_info.directpath_portid > 7) )
            {
                printk("\nError Invalid portid\n");
                goto EXIT_EIO;
            }

            res = ppa_capwap_update(&cmd_info->capwap_add_info);

            if ( res != PPA_SUCCESS )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_capwap_group_update fail\n");
                goto EXIT_EIO;
            }
            else
                goto EXIT_ZERO;
        }


   case PPA_CMD_DEL_CAPWAP:
        {
            int32_t res;

            if ( ppa_copy_from_user(&cmd_info->capwap_add_info, (void *)arg, sizeof(cmd_info->capwap_add_info)) != 0 )
                goto EXIT_EFAULT;

            res = ppa_capwap_delete(&cmd_info->capwap_add_info);

            if ( res != PPA_SUCCESS )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_capwap_delete fail\n");
                goto EXIT_EIO;
            }
            else
                goto EXIT_ZERO;
        }

    case PPA_CMD_GET_CAPWAP_GROUPS:
        {
            PPA_CMD_CAPWAP_INFO ppa_capwap_entry;
            struct capwap_group_list_item *pp_item;

            uint32_t j = 0;
            int32_t  i=0, index=0;

            if ( ppa_copy_from_user(&cmd_info->capwap_groups_info.count_info, (void *)arg, sizeof(cmd_info->capwap_groups_info.count_info)) != 0 )
                goto EXIT_EFAULT;

            if( cmd_info->capwap_groups_info.count_info.count == 0 )
                goto EXIT_ZERO;

            for(i=0; i<cmd_info->capwap_groups_info.count_info.count; i++)
            {
               if( ppa_capwap_start_iteration(i, &pp_item) == PPA_SUCCESS )
               {
                  ppa_memset(&cmd_info->capwap_groups_info.capwap_list, 0, sizeof(cmd_info->capwap_groups_info.capwap_list) );


                  cmd_info->capwap_groups_info.capwap_list[0].directpath_portid = pp_item->directpath_portid;
                  cmd_info->capwap_groups_info.capwap_list[0].qid = pp_item->qid;
                  ppa_memcpy(cmd_info->capwap_groups_info.capwap_list[0].dst_mac,pp_item->dst_mac,PPA_ETH_ALEN);
                  ppa_memcpy(cmd_info->capwap_groups_info.capwap_list[0].src_mac,pp_item->src_mac,PPA_ETH_ALEN);
                 
                  cmd_info->capwap_groups_info.capwap_list[0].tos = pp_item->tos;
                  cmd_info->capwap_groups_info.capwap_list[0].ttl = pp_item->ttl;

                  ppa_memcpy( &cmd_info->capwap_groups_info.capwap_list[0].source_ip, &pp_item->source_ip, sizeof(cmd_info->capwap_groups_info.capwap_list[0].source_ip));
                  ppa_memcpy( &cmd_info->capwap_groups_info.capwap_list[0].dest_ip, &pp_item->dest_ip, sizeof(cmd_info->capwap_groups_info.capwap_list[0].dest_ip));


                  cmd_info->capwap_groups_info.capwap_list[0].source_port = pp_item->source_port;
                  cmd_info->capwap_groups_info.capwap_list[0].dest_port = pp_item->dest_port;
                  cmd_info->capwap_groups_info.capwap_list[0].rid = pp_item->rid;
                  cmd_info->capwap_groups_info.capwap_list[0].wbid = pp_item->wbid;
                  cmd_info->capwap_groups_info.capwap_list[0].t_flag = pp_item->t_flag;
                  cmd_info->capwap_groups_info.capwap_list[0].max_frg_size = pp_item->max_frg_size;

                  cmd_info->capwap_groups_info.capwap_list[0].num_ifs = pp_item->num_ifs;

                  for(j=0; j<pp_item->num_ifs; j++ )
                  {
                     if( pp_item->netif[j] )
                        //ppa_strncpy( cmd_info->mc_groups.mc_group_list[0].lan_ifname[i], (void *)ppa_get_netif_name(pp_item->netif[i] ), sizeof(cmd_info->mc_groups.mc_group_list[0].lan_ifname[i]) -1);
                        ppa_strncpy( cmd_info->capwap_groups_info.capwap_list[0].lan_ifname[j], (void *)ppa_get_netif_name(pp_item->netif[j]), sizeof(cmd_info->capwap_groups_info.capwap_list[0].lan_ifname[j]) -1);
                  
                        cmd_info->capwap_groups_info.capwap_list[0].phy_port_id[j]
                                = pp_item->phy_port_id[j];
                  }

                  index = (void *)cmd_info->capwap_groups_info.capwap_list - (void *)&cmd_info->capwap_groups_info.count_info + i * sizeof(cmd_info->capwap_groups_info.capwap_list[0]);

                  //Get MIB info per CAPWAP tunnel
                  ppa_capwap_entry.p_entry = pp_item->p_entry; 
                  if ( ppa_drv_get_capwap_mib(&ppa_capwap_entry,0 ) == PPA_SUCCESS )
                  {
                     cmd_info->capwap_groups_info.capwap_list[0].ds_mib =ppa_capwap_entry.ds_mib;
                     cmd_info->capwap_groups_info.capwap_list[0].us_mib =ppa_capwap_entry.us_mib;
                  }
                  else
                     goto EXIT_EFAULT;

                  if ( ppa_copy_to_user( (void *)(arg + index),cmd_info->capwap_groups_info.capwap_list, sizeof(cmd_info->capwap_groups_info.capwap_list[0])) != 0 )
                  {
                     goto EXIT_EFAULT;
                  }

               }
            } //End of for loop for info count

            goto EXIT_ZERO;
        }

#endif
    case PPA_CMD_GET_COUNT_PPA_HASH:
        {
            if ( ppa_copy_from_user(&cmd_info->count_info, (void *)arg, sizeof(cmd_info->count_info)) != 0 )
                goto EXIT_EFAULT;
            
            get_ppa_hash_count(&cmd_info->count_info) ;          
            if ( ppa_copy_to_user((void *)arg, &cmd_info->count_info, sizeof(cmd_info->count_info)) != 0 )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail\n");
                goto EXIT_EFAULT;
            }
            goto EXIT_ZERO;
        }   
    case PPA_CMD_GET_PPA_HASH_SUMMARY:
        {
            int i;
            uint32_t offset =(uint32_t)(&cmd_info->ppa_hash_info.hash_info) - (uint32_t)( &cmd_info->ppa_hash_info );
      
            if ( ppa_copy_from_user(&cmd_info->ppa_hash_info, (void *)arg, sizeof(cmd_info->ppa_hash_info)) != 0 )
                goto EXIT_EFAULT;

            for(i=0; i<cmd_info->ppa_hash_info.hash_max_num; i++)
            {
                cmd_info->ppa_hash_info.hash_info.count[0] = ppa_sesssion_get_count_in_hash(i);
                if ( ppa_copy_to_user(  (void *)(arg + offset + i * sizeof(cmd_info->ppa_hash_info.hash_info.count[0])), 
                                        cmd_info->ppa_hash_info.hash_info.count, 
                                        sizeof(cmd_info->ppa_hash_info.hash_info.count[0])) != 0 )
                {       
                    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail for hash index %d\n", i);
                    goto EXIT_EFAULT;
                }            
            }            
            
            goto EXIT_ZERO;
        } 
    case PPA_CMD_GET_COUNT_LAN_SESSION:
    case PPA_CMD_GET_COUNT_WAN_SESSION:
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
    case PPA_CMD_GET_SESSIONS_CRITERIA:
    case PPA_CMD_SWAP_SESSIONS:
#endif
    case PPA_CMD_GET_COUNT_NONE_LAN_WAN_SESSION:
    case PPA_CMD_GET_COUNT_LAN_WAN_SESSION:    
    case PPA_CMD_GET_COUNT_MC_GROUP:
    case PPA_CMD_GET_COUNT_VLAN_FILTER:
    case PPA_CMD_GET_COUNT_MAC:
    case PPA_CMD_GET_COUNT_WAN_MII0_VLAN_RANGE:
    case PPA_CMD_GET_HOOK_COUNT:
#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
    case PPA_CMD_GET_COUNT_CAPWAP:
    case PPA_CMD_GET_MAXCOUNT_CAPWAP:
#endif
        {
            if ( ppa_copy_from_user(&cmd_info->count_info, (void *)arg, sizeof(cmd_info->count_info)) != 0 )
                goto EXIT_EFAULT;

            if( cmd == PPA_CMD_GET_COUNT_LAN_SESSION )
                cmd_info->count_info.count = ppa_session_get_routing_count( 1,  cmd_info->count_info.flag, cmd_info->count_info.hash_index) ;
            else if( cmd == PPA_CMD_GET_COUNT_WAN_SESSION )
                cmd_info->count_info.count = ppa_session_get_routing_count(0,  cmd_info->count_info.flag, cmd_info->count_info.hash_index ) ;
            else if( cmd == PPA_CMD_GET_COUNT_NONE_LAN_WAN_SESSION )
                cmd_info->count_info.count = ppa_session_get_routing_count(2,  cmd_info->count_info.flag, cmd_info->count_info.hash_index ) ;                
             else if( cmd == PPA_CMD_GET_COUNT_LAN_WAN_SESSION )
                cmd_info->count_info.count = ppa_session_get_routing_count( 1,  cmd_info->count_info.flag, cmd_info->count_info.hash_index) + ppa_session_get_routing_count(0,  cmd_info->count_info.flag, cmd_info->count_info.hash_index );
            else if( cmd == PPA_CMD_GET_COUNT_MC_GROUP )
                cmd_info->count_info.count = ppa_get_mc_group_count(  cmd_info->count_info.flag ) ;

#if defined(CAP_WAP_CONFIG) && CAP_WAP_CONFIG
            else if( cmd == PPA_CMD_GET_COUNT_CAPWAP )
                cmd_info->count_info.count = ppa_get_capwap_count();
            else if( cmd == PPA_CMD_GET_MAXCOUNT_CAPWAP )
                cmd_info->count_info.count = MAX_CAPWAP_ENTRIES;
#endif
            else if( cmd == PPA_CMD_GET_COUNT_VLAN_FILTER)
                cmd_info->count_info.count = ppa_get_all_vlan_filter_count(  cmd_info->count_info.flag ) ;
            else if( cmd == PPA_CMD_GET_COUNT_MAC)
                cmd_info->count_info.count = ppa_get_br_count( cmd_info->count_info.flag ) ;
            else if( cmd == PPA_CMD_GET_COUNT_WAN_MII0_VLAN_RANGE )
                   cmd_info->count_info.count = 1;
            else if( cmd == PPA_CMD_GET_HOOK_COUNT )
                    cmd_info->count_info.count = get_ppa_hook_list_count() ;          
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
	    else if ( cmd == PPA_CMD_GET_SESSIONS_CRITERIA ){
        
        if ( ppa_get_session_limit_enable(0) ) {
          cmd_info->session_criteria_info.ppa_low_prio_data_rate = ppa_get_low_prio_data_rate(0);
          cmd_info->session_criteria_info.ppa_def_prio_data_rate = ppa_get_def_prio_data_rate(0);
          cmd_info->session_criteria_info.ppa_low_prio_thresh = ppa_get_low_prio_thresh(0);
          cmd_info->session_criteria_info.ppa_def_prio_thresh = ppa_get_def_prio_thresh(0);
        } else {
          cmd_info->session_criteria_info.ppa_low_prio_data_rate = 0;
          cmd_info->session_criteria_info.ppa_def_prio_data_rate = 0;
          cmd_info->session_criteria_info.ppa_low_prio_thresh = 0;
          cmd_info->session_criteria_info.ppa_def_prio_thresh = 0;
        }
	   }
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
	    else if ( cmd == PPA_CMD_SWAP_SESSIONS ) {
			ppa_swap_sessions(0);
		}
#endif
#endif
            if ( ppa_copy_to_user((void *)arg, &cmd_info->count_info, sizeof(cmd_info->count_info)) != 0 )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail\n");
                goto EXIT_EFAULT;
            }
            goto EXIT_ZERO;
        }
    case PPA_CMD_GET_MC_GROUPS:
        {
            struct mc_group_list_item *pp_item;
            uint32_t pos = 0;
            int32_t count=0, i, index;

            if ( ppa_copy_from_user(&cmd_info->mc_groups.count_info, (void *)arg, sizeof(cmd_info->mc_groups.count_info)) != 0 )
                goto EXIT_EFAULT;

            if( cmd_info->mc_groups.count_info.count == 0 )
                goto EXIT_ZERO;

            if ( ppa_mc_group_start_iteration(&pos, &pp_item) == PPA_SUCCESS )
            {
                count = 0;
                do
                {
                    /*copy session information one by one to user space */
                    if ( 1 /*pp_item->flags & SESSION_ADDED_IN_HW */ )
                    {
                        ppa_memset(&cmd_info->mc_groups.mc_group_list, 0, sizeof(cmd_info->mc_groups.mc_group_list) );

#if defined(RTP_SAMPLING_ENABLE) && RTP_SAMPLING_ENABLE
                        if(pp_item->RTP_flag == 1)
                        {
                           if ( ppa_hw_get_mc_rtp_sampling_cnt( pp_item ) != PPA_SUCCESS )
                           { 
                              ppa_mc_group_stop_iteration();
                              goto EXIT_EIO;
                           }
                        }
                        else
                        {
                           cmd_info->mc_groups.mc_group_list[0].mc.rtp_pkt_cnt=0;
                           cmd_info->mc_groups.mc_group_list[0].mc.rtp_seq_num=0;
                        }
                        ppa_memcpy(&cmd_info->mc_groups.mc_group_list[0].mc.RTP_flag , &pp_item->RTP_flag, sizeof(cmd_info->mc_groups.mc_group_list[0].mc.RTP_flag));
                        ppa_memcpy(&cmd_info->mc_groups.mc_group_list[0].mc.rtp_pkt_cnt , &pp_item->rtp_pkt_cnt, sizeof(cmd_info->mc_groups.mc_group_list[0].mc.rtp_pkt_cnt));
                        ppa_memcpy(&cmd_info->mc_groups.mc_group_list[0].mc.rtp_seq_num , &pp_item->rtp_seq_num, sizeof(cmd_info->mc_groups.mc_group_list[0].mc.rtp_seq_num));

#endif

                        ppa_memcpy( &cmd_info->mc_groups.mc_group_list[0].mc.mcast_addr, &pp_item->ip_mc_group, sizeof(cmd_info->mc_groups.mc_group_list[0].mc.mcast_addr));
                        ppa_memcpy( &cmd_info->mc_groups.mc_group_list[0].mc.source_ip, &pp_item->source_ip, sizeof(cmd_info->mc_groups.mc_group_list[0].mc.source_ip));
                        cmd_info->mc_groups.mc_group_list[0].mc.SSM_flag = pp_item->SSM_flag;
                        cmd_info->mc_groups.mc_group_list[0].num_ifs = pp_item->num_ifs;


                        for(i=0; i<pp_item->num_ifs; i++ )
                        {
                            if( pp_item->netif[i] )
                                ppa_strncpy( cmd_info->mc_groups.mc_group_list[0].lan_ifname[i], (void *)ppa_get_netif_name(pp_item->netif[i] ), sizeof(cmd_info->mc_groups.mc_group_list[0].lan_ifname[i]) -1);
                        }

                        if( pp_item->src_netif )
                        {
                            ppa_strncpy( cmd_info->mc_groups.mc_group_list[0].src_ifname, (void *)ppa_get_netif_name(pp_item->src_netif), sizeof(cmd_info->mc_groups.mc_group_list[0].src_ifname) -1 );
                        }

                        cmd_info->mc_groups.mc_group_list[0].mc.mc_extra.new_dscp = pp_item->new_dscp;

                        cmd_info->mc_groups.mc_group_list[0].mc.mc_extra.vlan_id = pp_item->new_vci &0xFFF;
                        cmd_info->mc_groups.mc_group_list[0].mc.mc_extra.vlan_prio = ( pp_item->new_vci >>13 )& 0x7;
                        cmd_info->mc_groups.mc_group_list[0].mc.mc_extra.vlan_cfi =( pp_item->new_vci >>12 )& 0x1;

                        cmd_info->mc_groups.mc_group_list[0].mc.mc_extra.out_vlan_tag = pp_item->out_vlan_tag;

                        cmd_info->mc_groups.mc_group_list[0].mc.mc_extra.dslwan_qid= pp_item->dslwan_qid;

                        cmd_info->mc_groups.mc_group_list[0].mc.flags= pp_item->flags;
                        cmd_info->mc_groups.mc_group_list[0].mc.hw_bytes = pp_item->acc_bytes - pp_item->prev_clear_acc_bytes;
                        cmd_info->mc_groups.mc_group_list[0].bridging_flag= pp_item->bridging_flag;

                        index = (void *)cmd_info->mc_groups.mc_group_list - (void *)&cmd_info->mc_groups.count_info + count * sizeof(cmd_info->mc_groups.mc_group_list[0]);
                        
                        if ( ppa_copy_to_user(  (void *)(arg + index) , cmd_info->mc_groups.mc_group_list,  sizeof(cmd_info->mc_groups.mc_group_list[0])) != 0 )
                        {
                            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail during copy_to_user for mc: count=%d index=%d\n", count, index);
                            ppa_mc_group_stop_iteration();
                            goto EXIT_EFAULT;
                        }

                        count ++;
                    }
                } while ( (ppa_mc_group_iterate_next(&pos, &pp_item) == PPA_SUCCESS ) && (count < cmd_info->mc_groups.count_info.count) );
            }

            cmd_info->mc_groups.count_info.count = count;  //update the real session number to user space
            ppa_mc_group_stop_iteration();

            if ( ppa_copy_to_user(  (void *)arg, &cmd_info->mc_groups.count_info, sizeof(cmd_info->mc_groups.count_info)) != 0 )
            {
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail during copy_to_user for mc counter_info\n");
                goto EXIT_EFAULT;
            }

            goto EXIT_ZERO;
        }

    case PPA_CMD_GET_LAN_SESSIONS:
    case PPA_CMD_GET_WAN_SESSIONS:
    case PPA_CMD_GET_LAN_WAN_SESSIONS:
        {
          res = ppa_ioctl_get_sessions(cmd, &cmd_info->session_info, arg);

          goto EXIT_LAST;
        }
        
    case PPA_CMD_GET_ALL_MAC:
        {
    
            struct bridging_session_list_item *pp_item;
            int32_t count = 0;
            uint32_t pos = 0;

            if ( ppa_copy_from_user(&cmd_info->all_br_info.count_info, (void *)arg, sizeof(cmd_info->all_br_info.count_info)) != 0 )
                goto EXIT_EFAULT;


            if ( ppa_bridging_session_start_iteration(&pos, &pp_item) == PPA_SUCCESS && cmd_info->all_br_info.count_info.count )
            {
                do
                {
                    ppa_memcpy( cmd_info->all_br_info.session_list[0].mac_addr, pp_item->mac, PPA_ETH_ALEN);
                    ppa_strncpy( cmd_info->all_br_info.session_list[0].ifname, ppa_get_netif_name(pp_item->netif), sizeof(cmd_info->all_br_info.session_list[0].ifname) );
                    cmd_info->all_br_info.session_list[0].flags = pp_item->flags;

                    if ( ppa_copy_to_user( ( (void *)arg) + ( (void *)cmd_info->all_br_info.session_list  - (void *)&cmd_info->all_br_info.count_info) + count * sizeof(cmd_info->all_br_info.session_list[0]) , cmd_info->all_br_info.session_list, sizeof(cmd_info->all_br_info.session_list[0])) != 0 )
                    {
                        ppa_bridging_session_stop_iteration();
                        goto EXIT_EFAULT;
                    }
                    count ++;

                    if( count == cmd_info->all_br_info.count_info.count ) break;  //buffer is full. We should stop now
                } while ( ppa_bridging_session_iterate_next(&pos, &pp_item) == PPA_SUCCESS );
            }

            ppa_bridging_session_stop_iteration();
            cmd_info->all_br_info.count_info.count = count;  //update the real session number to user space
            if ( ppa_copy_to_user(  (void *)arg, &cmd_info->all_br_info.count_info, sizeof(cmd_info->all_br_info.count_info)) != 0 )
            {
                goto EXIT_EFAULT;
            }

            goto EXIT_ZERO;
        }
    case PPA_CMD_GET_VERSION:
        {
            ppa_memset(&cmd_info->ver, 0, sizeof(cmd_info->ver) );
            ppa_get_api_id(&cmd_info->ver.ppa_api_ver.family, &cmd_info->ver.ppa_api_ver.type, &cmd_info->ver.ppa_api_ver.itf, &cmd_info->ver.ppa_api_ver.mode, &cmd_info->ver.ppa_api_ver.major, &cmd_info->ver.ppa_api_ver.mid, &cmd_info->ver.ppa_api_ver.minor);
            ppa_get_stack_al_id(&cmd_info->ver.ppa_stack_al_ver.family, &cmd_info->ver.ppa_stack_al_ver.type, &cmd_info->ver.ppa_stack_al_ver.itf, &cmd_info->ver.ppa_stack_al_ver.mode, &cmd_info->ver.ppa_stack_al_ver.major, &cmd_info->ver.ppa_stack_al_ver.mid, &cmd_info->ver.ppa_stack_al_ver.minor);

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	    if( ppa_drv_get_hal_id(&cmd_info->ver.ppe_hal_ver,0) == PPA_SUCCESS) {
            	cmd_info->ver.ppe_fw_ver[0].index = 0;
            	cmd_info->ver.ppe_fw_ver[1].index = 1;
            	ppa_drv_get_firmware_id(&cmd_info->ver.ppe_fw_ver[0], 0);
            	ppa_drv_get_firmware_id(&cmd_info->ver.ppe_fw_ver[1], 0);
	    }
#else
            ppa_drv_get_ppe_hal_id(&cmd_info->ver.ppe_hal_ver, 0);
            cmd_info->ver.ppe_fw_ver[0].index = 0;
            cmd_info->ver.ppe_fw_ver[1].index = 1;
            ppa_drv_get_firmware_id(&cmd_info->ver.ppe_fw_ver[0], 0);
            ppa_drv_get_firmware_id(&cmd_info->ver.ppe_fw_ver[1], 0);
#endif
            ppa_subsystem_id(&cmd_info->ver.ppa_subsys_ver.family, &cmd_info->ver.ppa_subsys_ver.type, &cmd_info->ver.ppa_subsys_ver.itf, &cmd_info->ver.ppa_subsys_ver.mode, &cmd_info->ver.ppa_subsys_ver.major, &cmd_info->ver.ppa_subsys_ver.mid, &cmd_info->ver.ppa_subsys_ver.minor, &cmd_info->ver.ppa_subsys_ver.tag);
             if( ppa_drv_hal_generic_hook )
            {
                ppa_drv_hal_generic_hook(PPA_GENERIC_WAN_INFO,(void *)&cmd_info->ver.ppa_wan_info, 0 );
                ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_GET_FEATURE_LIST,(void *)&cmd_info->ver.ppe_fw_feature, 0 );
            }
             else
                ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Why ppa_drv_hal_generic_hook is NULL\n");

#ifdef CONFIG_LTQ_PPA_IPv6_ENABLE
            cmd_info->ver.ppa_feature.ipv6_en = 1;
#endif
#ifdef CONFIG_LTQ_PPA_QOS
            cmd_info->ver.ppa_feature.qos_en = 1;
#endif

            if ( ppa_copy_to_user(  (void *)arg, &cmd_info->ver, sizeof(cmd_info->ver)) != 0 )
                goto EXIT_EFAULT;
            goto EXIT_ZERO;
        }

    case PPA_CMD_WAN_MII0_VLAN_RANGE_ADD:
        {
            if ( ppa_copy_from_user(&cmd_info->wan_vlanid_range, (void *)arg, sizeof(cmd_info->wan_vlanid_range)) != 0 )
                goto EXIT_EFAULT;

            ppa_hook_wan_mii0_vlan_range_add(&cmd_info->wan_vlanid_range, 0);

            goto EXIT_ZERO;
        }
    case PPA_CMD_WAN_MII0_VLAN_RANGE_GET:
        {
            PPE_WAN_VID_RANGE vlanid;
            if ( ppa_copy_from_user(&cmd_info->all_wan_vlanid_range_info.count_info, (void *)arg, sizeof(cmd_info->all_wan_vlanid_range_info.count_info)) != 0 )
                goto EXIT_EFAULT;

            if( cmd_info->all_wan_vlanid_range_info.count_info.count == 0 )
                goto EXIT_ZERO;

            if( cmd_info->all_wan_vlanid_range_info.count_info.count > 1 )
                cmd_info->all_wan_vlanid_range_info.count_info.count = 1; //at present only support one wan vlan range

            ppa_drv_get_mixed_wan_vlan_id(&vlanid, 0);
            cmd_info->all_wan_vlanid_range_info.ranges[0].start_vlan_range = vlanid.vid& 0xFFF;
            cmd_info->all_wan_vlanid_range_info.ranges[0].end_vlan_range = ( vlanid.vid>> 16 ) & 0xFFF;
            if ( ppa_copy_to_user(  (void *)arg, &cmd_info->all_wan_vlanid_range_info, sizeof(cmd_info->all_wan_vlanid_range_info)) != 0 )
                goto EXIT_EFAULT;

            goto EXIT_ZERO;
        }
    case PPA_CMD_GET_SIZE:
        {
            ppa_memset(&cmd_info->size_info, 0, sizeof(cmd_info->size_info) );
            cmd_info->size_info.rout_session_size = sizeof(struct session_list_item);
            cmd_info->size_info.mc_session_size= sizeof(struct mc_group_list_item);
            cmd_info->size_info.br_session_size= sizeof(struct bridging_session_list_item);
            cmd_info->size_info.netif_size= sizeof(struct netif_info);

            if ( ppa_copy_to_user(  (void *)arg, &cmd_info->size_info, sizeof(cmd_info->size_info)) != 0 )
                goto EXIT_EFAULT;
            goto EXIT_ZERO;
        }
    case PPA_CMD_BRIDGE_ENABLE:
    {
        res = ppa_ioctl_bridge_enable(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_BRIDGE_STATUS:
    {
        res = ppa_ioctl_get_bridge_enable_status(cmd, arg, cmd_info );
        break;
    }

#ifdef CONFIG_LTQ_PPA_QOS
    case PPA_CMD_GET_QOS_STATUS:
    {
        res = ppa_ioctl_get_qos_status(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_QOS_QUEUE_MAX_NUM:
    {
        res = ppa_ioctl_get_qos_qnum(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_QOS_MIB:
    {
        res = ppa_ioctl_get_qos_mib(cmd, arg, cmd_info );
        break;
    }
#if !defined(CONFIG_PPA_PUMA7) 
#ifdef CONFIG_LTQ_PPA_QOS_WFQ
    case PPA_CMD_SET_CTRL_QOS_WFQ:
    {
        res = ppa_ioctl_set_ctrl_qos_wfq(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_CTRL_QOS_WFQ:
    {
        res = ppa_ioctl_get_ctrl_qos_wfq(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_SET_QOS_WFQ:
    {
        res = ppa_ioctl_set_qos_wfq(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_RESET_QOS_WFQ:
    {
        res = ppa_ioctl_reset_qos_wfq(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_QOS_WFQ:
    {
        res = ppa_ioctl_get_qos_wfq(cmd, arg, cmd_info );
        break;
    }
#endif //end of CONFIG_LTQ_PPA_QOS_WFQ
#endif //end of !PUMA7 
#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
    case PPA_CMD_SET_CTRL_QOS_RATE:
    {
        res = ppa_ioctl_set_ctrl_qos_rate(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_CTRL_QOS_RATE:
    {
        res = ppa_ioctl_get_ctrl_qos_rate(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_SET_QOS_RATE:
    {
        res = ppa_ioctl_set_qos_rate(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_RESET_QOS_RATE:
    {
        res = ppa_ioctl_reset_qos_rate(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_QOS_RATE:
    {
        res = ppa_ioctl_get_qos_rate(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_ADD_QOS_QUEUE:
    {
        res = ppa_ioctl_add_qos_queue(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_MOD_QOS_QUEUE:
    {
        res = ppa_ioctl_modify_qos_queue(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_DEL_QOS_QUEUE:
    {
        res = ppa_ioctl_delete_qos_queue(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_ENG_QUEUE_INIT:
    {
	res = ppa_ioctl_qos_init_cfg(cmd);
	break;
    }
    case PPA_CMD_ENG_QUEUE_UNINIT:
    {
	res = ppa_ioctl_qos_init_cfg(cmd);
	break;
    }
    case PPA_CMD_MOD_SUBIF_PORT:
    {
	res = ppa_ioctl_mod_subif_port_config(cmd, arg, cmd_info );
	break;
    }
    case PPA_CMD_ADD_WMM_QOS_QUEUE:
    {
        res = ppa_ioctl_add_wmm_qos_queue(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_DEL_WMM_QOS_QUEUE:
    {
        res = ppa_ioctl_delete_wmm_qos_queue(cmd, arg, cmd_info );
        break;
    }

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
    case PPA_CMD_ADD_CLASSIFIER:
    {
	res = ppa_ioctl_add_class_rule(cmd , arg, cmd_info);
	break;
    }
    case PPA_CMD_MOD_CLASSIFIER:
    {
        res = ppa_ioctl_mod_class_rule(cmd , arg, cmd_info);
        break;
    }
    case PPA_CMD_GET_CLASSIFIER:
    {
        res = ppa_ioctl_get_class_rule(cmd , arg, cmd_info);
        break;
    }
    case PPA_CMD_DEL_CLASSIFIER:
    {
        res = ppa_ioctl_del_class_rule(cmd , arg, cmd_info);
        break;
    }
#endif

#if defined(MBR_CONFIG) && MBR_CONFIG
    case PPA_CMD_SET_QOS_SHAPER:
    {
        res = ppa_ioctl_set_qos_shaper(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_QOS_SHAPER:
    {
        res = ppa_ioctl_get_qos_shaper(cmd, arg, cmd_info );
        break;
    }
    
    
	
#endif //end of MBR_CONFIG

#endif //end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING

#endif //end of CONFIG_LTQ_PPA_QOS
    case  PPA_CMD_GET_HOOK_LIST:
    {
        res = ppa_ioctl_get_hook_list(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_SET_HOOK:
    {
        res = ppa_ioctl_set_hook(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_READ_MEM:
    {
        res = ppa_ioctl_read_mem(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_SET_MEM:
    {
        res = ppa_ioctl_set_mem(cmd, arg, cmd_info );
        break;
    }
#if defined(CONFIG_LTQ_PPA_MFE) && CONFIG_LTQ_PPA_MFE
    case  PPA_CMD_ENABLE_MULTIFIELD:
    {
        res = ppa_ioctl_enable_multifield(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_GET_MULTIFIELD_STATUS:
    {
        res = ppa_ioctl_get_multifield_status(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_GET_MULTIFIELD_ENTRY_MAX:
    {
        res = ppa_ioctl_get_multifield_max_entry(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_ADD_MULTIFIELD:
    {
        res = ppa_ioctl_add_multifield_flow(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_GET_MULTIFIELD:
    {
        res = ppa_ioctl_get_multifield_flow(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_DEL_MULTIFIELD:
    {
        res = ppa_ioctl_del_multifield_flow(cmd, arg, cmd_info );
        break;
    }
    case  PPA_CMD_DEL_MULTIFIELD_VIA_INDEX:
    {
        res = ppa_ioctl_del_multifield_flow_via_index(cmd, arg, cmd_info );
        break;
    }

#endif //CONFIG_LTQ_PPA_MFE

    case  PPA_CMD_GET_MAX_ENTRY:
    {
        res = ppa_ioctl_get_max_entry(cmd, arg, cmd_info );
        break;
    }

    case PPA_CMD_GET_PORTID:
    {
        res = ppa_ioctl_get_portid(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_DSL_MIB:
    {
        res = ppa_ioctl_get_dsl_mib(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_CLEAR_DSL_MIB:
    {
        res = ppa_ioctl_clear_dsl_mib(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_DEL_SESSION:
    {
        res = ppa_ioctl_del_session(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_ADD_SESSION:
    {
        res = ppa_ioctl_add_session(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_MODIFY_SESSION:
    {
        res = ppa_ioctl_modify_session(cmd, arg, cmd_info );
        break;
    }
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
	case PPA_CMD_MANAGE_SESSIONS:
	{
		res = ppa_ioctl_manage_sessions(cmd, arg, cmd_info );
		break;
	}
	case PPA_CMD_GET_SESSIONS:
	{
		res = ppa_ioctl_get_prio_sessions(cmd, arg, cmd_info );
		break;
	}
	case PPA_CMD_GET_SESSIONS_COUNT:
	{
		res = ppa_ioctl_get_sessions_count(cmd, arg, cmd_info );
		break;
	}
#endif
    case PPA_CMD_SET_SESSION_TIMER:
    {
        res = ppa_ioctl_set_session_timer(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_SESSION_TIMER:
    {
        res = ppa_ioctl_get_session_timer(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_GET_PORT_MIB:
    {
        res = ppa_ioctl_get_ports_mib(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_CLEAR_PORT_MIB:
    {
        if ( ppa_copy_from_user(&cmd_info->port_mib_info, (void *)arg, sizeof(cmd_info->port_mib_info)) != 0 )
                goto EXIT_EFAULT;
        res = ppa_ioctl_clear_ports_mib(cmd, arg, cmd_info );
        break;
    }
    case PPA_CMD_SET_HAL_DBG_FLAG:
    {
        if ( ppa_copy_from_user(&cmd_info->genernal_enable_info, (void *)arg, sizeof(cmd_info->genernal_enable_info)) != 0 )
                goto EXIT_EFAULT;
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
	ppa_drv_set_hal_dbg(&cmd_info->genernal_enable_info,0);	
#else
        if( ppa_drv_hal_generic_hook )
        {
            ppa_drv_hal_generic_hook(PPA_GENERIC_HAL_SET_DEBUG,(void *)&cmd_info->genernal_enable_info.enable, 0 );
        }
         else
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Why ppa_drv_hal_generic_hook is NULL\n");
#endif
         break;
    }
    case PPA_CMD_DBG_TOOL:
    {         
        res = ppa_ioctl_dbg_tool_test(cmd, arg, cmd_info );

        break;
    }
    case PPA_CMD_SET_VALUE:
    {         
        res = ppa_ioctl_set_value(cmd, arg, cmd_info );

        break;
    }
    case PPA_CMD_GET_VALUE:
    {         
        res = ppa_ioctl_get_value(cmd, arg, cmd_info );

        break;
    }
    case PPA_CMD_SET_PPE_FASTPATH_ENABLE:
    {
	res = ppa_ioctl_set_ppe_fastpath_enable(cmd, arg, cmd_info );
	
	break;
    }
    case PPA_CMD_GET_PPE_FASTPATH_ENABLE:
    {
	res = ppa_ioctl_get_ppe_fastpath_enable(cmd, arg, cmd_info );

        break;
    }
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
    case PPA_CMD_SET_SW_FASTPATH_ENABLE:
    {
        res = ppa_ioctl_set_sw_fastpath_enable(cmd, arg, cmd_info );
     
        break;
    }
    case PPA_CMD_GET_SW_FASTPATH_ENABLE:
    {
        res = ppa_ioctl_get_sw_fastpath_enable(cmd, arg, cmd_info );

        break;
    }
    case PPA_CMD_SET_SW_SESSION_ENABLE:
    {
        res = ppa_ioctl_set_sw_session_enable(cmd, arg, cmd_info );
     
        break;
    }
    case PPA_CMD_GET_SW_SESSION_ENABLE:
    {
        res = ppa_ioctl_get_sw_session_enable(cmd, arg, cmd_info );

        break;
    }
#endif
    default:
        printk("wrong cmd:0x%0x(its nr value=0x%0x)\n", cmd, ppa_ioc_nr(cmd) );
        goto EXIT_ENOIOCTLCMD;
    }
    if ( res != PPA_SUCCESS )
    {
        goto EXIT_EIO;
    }
    else
        goto EXIT_ZERO;

EXIT_EIO:
    res = -EIO;
    goto EXIT_LAST;
EXIT_EFAULT:
    res = -EFAULT;
    goto EXIT_LAST;
EXIT_ZERO:
    res = 0;
    goto EXIT_LAST;
EXIT_EINVAL:
    res = -EINVAL;
    goto EXIT_LAST;
EXIT_ENOIOCTLCMD:
    res = ENOIOCTLCMD;
    goto EXIT_LAST;
EXIT_LAST:
    if( cmd_info )
        ppa_free(cmd_info);
    return res;
}

static int32_t ppa_get_all_vlan_filter_count(uint32_t count_flag)
{
    uint32_t  i, j;
    int32_t count = 0;
    uint32_t vlan_filter_type[] = {PPA_F_VLAN_FILTER_IFNAME, PPA_F_VLAN_FILTER_IP_SRC, PPA_F_VLAN_FILTER_ETH_PROTO, PPA_F_VLAN_FILTER_VLAN_TAG};
    PPE_VFILTER_COUNT_CFG vfilter_count={0};
    PPE_BRDG_VLAN_FILTER_MAP vlan_map={0};

    count = 0;
    for ( i = 0; i < NUM_ENTITY(vlan_filter_type); i++ )
    {
        vfilter_count.vfitler_type = vlan_filter_type[i];
        ppa_drv_get_max_vfilter_entries(&vfilter_count, 0);

        for ( j = 0; j < vfilter_count.num; j++ )
        {
            vlan_map.ig_criteria_type = vlan_filter_type[i];
            vlan_map.entry = j;

            if ( ppa_drv_get_vlan_map( &vlan_map , 0)  == PPA_SUCCESS )
            {
                count++;
            }
        }
    }

    return  count;
}


#if defined(CONFIG_LTQ_PPA_API_PROC)
static int print_fw_ver(char *buf, int buf_len, unsigned int family, unsigned int type, unsigned int itf, unsigned int mode, unsigned int major, unsigned int minor)
{
    static char * fw_ver_family_str[] = {
        "reserved - 0",
        "Danube",
        "Twinpass",
        "Amazon-SE",
        "reserved - 4",
        "AR9",
        "GR9",
        "VR9"
    };
    static char * fw_ver_type_str[] = {
        "reserved - 0",
        "Standard",
        "Acceleration",
        "VDSL2 Bonding"
    };
    static char * fw_ver_interface_str[] = {
        "MII0",
        "MII0 + MII1",
        "MII0 + ATM",
        "MII0 + PTM",
        "MII0/1 + ATM",
        "MII0/1 + PTM"
    };
    static char * fw_ver_mode_str[] = {
        "reserved - 0",
        "reserved - 1",
        "reserved - 2",
        "Routing",
        "reserved - 4",
        "Bridging",
        "Bridging + IPv4 Routing",
        "Bridging + IPv4/6 Routing"
    };

    int len = 0;

    len += ppa_snprintf(buf + len, buf_len - len, "PPE firmware info:\n");
    len += ppa_snprintf(buf + len, buf_len - len, "  Version ID: %d.%d.%d.%d.%d.%d\n", family, type, itf, mode, major, minor);
    if ( family > NUM_ENTITY(fw_ver_family_str) -1 )
        len += ppa_snprintf(buf + len, buf_len - len, "  Family    : reserved - %d\n", family);
    else
        len += ppa_snprintf(buf + len, buf_len - len, "  Family    : %s\n", fw_ver_family_str[family]);
    if ( type > NUM_ENTITY(fw_ver_type_str) -1 )
        len += ppa_snprintf(buf + len, buf_len - len, "  FW Type   : reserved - %d\n", type);
    else
        len += ppa_snprintf(buf + len, buf_len - len, "  FW Type   : %s\n", fw_ver_type_str[type]);
    if ( itf > NUM_ENTITY(fw_ver_interface_str) -1 )
        len += ppa_snprintf(buf + len, buf_len - len, "  Interface : reserved - %d\n", itf);
    else
        len += ppa_snprintf(buf + len, buf_len - len, "  Interface : %s\n", fw_ver_interface_str[itf]);
    if ( mode > NUM_ENTITY(fw_ver_mode_str) -1 )
        len += ppa_snprintf(buf + len, buf_len - len, "  Mode      : reserved - %d\n", mode);
    else
        len += ppa_snprintf(buf + len, buf_len - len, "  Mode      : %s\n", fw_ver_mode_str[mode]);
    len += ppa_snprintf(buf + len, buf_len - len, "  Release   : %d.%d\n", major, minor);

    return len;
}

static int print_ppa_subsystem_ver(char *buf, int buf_len, char *title, unsigned int family, unsigned int type, unsigned int itf, unsigned int mode, unsigned int major, unsigned int mid, unsigned int minor, unsigned int tag)
{
    int len = 0;
    len += ppa_snprintf(buf + len, buf_len - len, "%s:\n", title);
    len += ppa_snprintf(buf + len, buf_len - len, "  Version ID: %d.%d.%d-%d\n", major, mid, minor, tag);
    return len;
}

static int print_driver_ver(char *buf, int buf_len, char *title, unsigned int family, unsigned int type, unsigned int itf, unsigned int mode, unsigned int major, unsigned int mid, unsigned int minor)
{
    static char * dr_ver_family_str[] = {
        NULL,
        "Danube",
        "Twinpass",
        "Amazon-SE",
        NULL,
        "AR9",
        "GR9",
        "VR9"
	"GRX500"
    };
    static char * dr_ver_type_str[] = {
        "Normal Data Path",
        "Indirect-Fast Path",
        "HAL",
        "Hook",
        "OS Adatpion Layer",
        "PPA API"
    };
    static char * dr_ver_interface_str[] = {
        "MII0",
        "MII1",
        "ATM",
        "PTM"
    };
    static char * dr_ver_accmode_str[] = {
        "Routing",
        "Bridging",
    };

    int len = 0;
    unsigned int bit;
    int i, j;

    len += ppa_snprintf(buf + len, buf_len - len, "%s:\n", title);
    len += ppa_snprintf(buf + len, buf_len - len, "  Version ID: %d.%d.%d.%d.%d.%d.%d\n", family, type, itf, mode, major, mid, minor);
    len += ppa_snprintf(buf + len, buf_len - len, "  Family    : ");
    for ( bit = family, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_family_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_family_str[i] != NULL )
        {
            if ( j )
                len += ppa_snprintf(buf + len, buf_len - len, " | %s", dr_ver_family_str[i]);
            else
                len += ppa_snprintf(buf + len, buf_len - len, dr_ver_family_str[i]);
            j++;
        }
    if ( j )
        len += ppa_snprintf(buf + len, buf_len - len, "\n");
    else
        len += ppa_snprintf(buf + len, buf_len - len, "N/A\n");
    len += ppa_snprintf(buf + len, buf_len - len, "  DR Type   : ");
    for ( bit = type, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_type_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_type_str[i] != NULL )
        {
            if ( j )
                len += ppa_snprintf(buf + len, buf_len - len, " | %s", dr_ver_type_str[i]);
            else
                len += ppa_snprintf(buf + len, buf_len - len, dr_ver_type_str[i]);
            j++;
        }
    if ( j )
        len += ppa_snprintf(buf + len, buf_len - len, "\n");
    else
        len += ppa_snprintf(buf + len, buf_len - len, "N/A\n");
    len += ppa_snprintf(buf + len, buf_len - len, "  Interface : ");
    for ( bit = itf, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_interface_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_interface_str[i] != NULL )
        {
            if ( j )
                len += ppa_snprintf(buf + len, buf_len - len, " | %s", dr_ver_interface_str[i]);
            else
                len += ppa_snprintf(buf + len, buf_len - len, dr_ver_interface_str[i]);
            j++;
        }
    if ( j )
        len += ppa_snprintf(buf + len, buf_len - len, "\n");
    else
        len += ppa_snprintf(buf + len, buf_len - len, "N/A\n");
    len += ppa_snprintf(buf + len, buf_len - len, "  Mode      : ");
    for ( bit = mode, i = j = 0; bit != 0 && i < NUM_ENTITY(dr_ver_accmode_str); bit >>= 1, i++ )
        if ( (bit & 0x01) && dr_ver_accmode_str[i] != NULL )
        {
            if ( j )
                len += ppa_snprintf(buf + len, buf_len - len, " | %s", dr_ver_accmode_str[i]);
            else
                len += ppa_snprintf(buf + len, buf_len - len, dr_ver_accmode_str[i]);
            j++;
        }
    if ( j )
        len += ppa_snprintf(buf + len, buf_len - len, "\n");
    else
        len += ppa_snprintf(buf + len, buf_len - len, "N/A\n");
    len += ppa_snprintf(buf + len, buf_len - len, "  Release   : %d.%d.%d\n", major, mid, minor);

    return len;
}

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static int print_switch_ver(char *buf, int buf_len, char *title, unsigned int id, char* name, char* version)
{
    int len = 0;
    len += ppa_snprintf(buf + len, buf_len - len, "%s:\n", title);
    len += ppa_snprintf(buf + len, buf_len - len, "  ID: %d\n", id);
    len += ppa_snprintf(buf + len, buf_len - len, "  Name: %s:\n", name);
    len += ppa_snprintf(buf + len, buf_len - len, "  Version%s:\n", version);
    return len;
}
#endif

static INLINE void proc_file_create(void)
{
    struct proc_dir_entry *res;

    for ( res = proc_root.subdir; res; res = res->next )
        if ( res->namelen == 3
            && res->name[0] == 'p'
            && res->name[1] == 'p'
            && res->name[2] == 'a' ) //  "ppa"
        {
            g_ppa_proc_dir = res;
            break;
        }
    if ( !res )
    {
        g_ppa_proc_dir = proc_mkdir("ppa", NULL);
        g_ppa_proc_dir_flag = 1;
    }

    for ( res = g_ppa_proc_dir->subdir; res; res = res->next )
        if ( res->namelen == 3
            && res->name[0] == 'a'
            && res->name[1] == 'p'
            && res->name[2] == 'i' )    //  "api"
        {
            g_ppa_api_proc_dir = res;
            break;
        }
    if ( !res )
    {
        g_ppa_api_proc_dir = proc_mkdir("api", g_ppa_proc_dir);
        g_ppa_api_proc_dir_flag = 1;
    }

    res = create_proc_read_entry("ver",
                                  0,
                                  g_ppa_api_proc_dir,
                                  proc_read_ver,
                                  NULL);
}

static INLINE void proc_file_delete(void)
{
    remove_proc_entry("ver",
                      g_ppa_api_proc_dir);

    if ( g_ppa_api_proc_dir_flag )
    {
        remove_proc_entry("api",
                          g_ppa_proc_dir);
        g_ppa_api_proc_dir = NULL;
        g_ppa_api_proc_dir_flag = 0;
    }

    if ( g_ppa_proc_dir_flag )
    {
        remove_proc_entry("ppa", NULL);
        g_ppa_proc_dir = NULL;
        g_ppa_proc_dir_flag = 0;
    }
}

static int proc_read_ver(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    PPA_VERSION ver={0};
    int len = 0;

    ppa_subsystem_id(&ver.family, &ver.type, &ver.itf, &ver.mode, &ver.major, &ver.mid, &ver.minor, &ver.tag);
    len += print_ppa_subsystem_ver(page + off + len, count - len, "PPA Sub System info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor, ver.tag);
    ppa_get_api_id(&ver.family, &ver.type, &ver.itf, &ver.mode, &ver.major, &ver.mid, &ver.minor);
    len += print_driver_ver(page + off + len, count - len, "PPA API driver info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor);
    ppa_get_stack_al_id(&ver.family, &ver.type, &ver.itf, &ver.mode, &ver.major, &ver.mid, &ver.minor);
    len += print_driver_ver(page + off + len, count - len, "PPA Stack Adaption Layer driver info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor);

#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
    if( ppa_drv_get_hal_id(&ver,0) == PPA_SUCCESS) {
	len += print_driver_ver(page + off + len, count - len, "PPE HAL driver info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor);
	ppa_memset(&ver,0,sizeof(ver));
	ver.index = 0;
	ppa_drv_get_firmware_id(&ver, 0);
	len += print_driver_ver(page + off + len, count - len, "PPE HAL driver info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor);
	ppa_memset(&ver,0,sizeof(ver));
	ver.index = 1;
	ppa_drv_get_firmware_id(&ver, 0);
	len += print_driver_ver(page + off + len, count - len, "PPE HAL driver info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor);
    }
#else
    ppa_drv_get_ppe_hal_id(&ver, 0);
    len += print_driver_ver(page + off + len, count - len, "PPE HAL driver info", ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.mid, ver.minor);
    ver.index = 0;
    if( ppa_drv_get_firmware_id(&ver, 0) == PPA_SUCCESS )
        len += print_fw_ver(page + off + len, count - len, ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.minor);
    ver.index =1;
    if( ppa_drv_get_firmware_id(&ver, 0) == PPA_SUCCESS )
        len += print_fw_ver(page + off + len, count - len, ver.family, ver.type, ver.itf, ver.mode, ver.major, ver.minor);
#endif
    *eof = 1;

    return len;
}

#endif

int32_t ppa_ioctl_bridge_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;

    if ( copy_from_user( &cmd_info->br_enable_info, (void *)arg, sizeof(cmd_info->br_enable_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_hook_bridge_enable( cmd_info->br_enable_info.bridge_enable, cmd_info->br_enable_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_bridge_enable fail\n");
        res = PPA_FAILURE;
    }

   return res;
}

int32_t ppa_ioctl_get_bridge_enable_status(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;

    ppa_memset(&cmd_info->br_enable_info, 0, sizeof(cmd_info->br_enable_info) );

    if ( copy_from_user( &cmd_info->br_enable_info, (void *)arg, sizeof(cmd_info->br_enable_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_hook_get_bridge_status( &cmd_info->br_enable_info.bridge_enable, cmd_info->br_enable_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_get_bridge_enable_status fail\n");
        res = PPA_FAILURE;
    }
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->br_enable_info, sizeof(cmd_info->br_enable_info)) != 0 )
        return PPA_FAILURE;

   return res;
}


int32_t ppa_ioctl_get_max_entry(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;
    
    ppa_memset(&cmd_info->acc_entry_info, 0, sizeof(cmd_info->acc_entry_info) );

    if ( copy_from_user( &cmd_info->acc_entry_info, (void *)arg, sizeof(cmd_info->acc_entry_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_get_max_entries( &cmd_info->acc_entry_info.entries, cmd_info->acc_entry_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_get_max_entry fail\n");
        res = PPA_FAILURE;
    }
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->acc_entry_info, sizeof(cmd_info->acc_entry_info)) != 0 )
        return PPA_FAILURE;

   return res;
}

int32_t ppa_ioctl_get_portid(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;
    struct netif_info *rx_ifinfo=NULL;
    
    ppa_memset(&cmd_info->portid_info, 0, sizeof(cmd_info->portid_info) );

    if ( copy_from_user( &cmd_info->portid_info, (void *)arg, sizeof(cmd_info->portid_info)) != 0 )
        return PPA_FAILURE;

    if ( ppa_netif_lookup(cmd_info->portid_info.ifname, &rx_ifinfo) != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"failed in getting info structure of ifname (%s)\n", cmd_info->portid_info.ifname );

        return PPA_FAILURE;
    }
    cmd_info->portid_info.portid = rx_ifinfo->phys_port;

    ppa_netif_put(rx_ifinfo);

    res = PPA_SUCCESS;

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->portid_info, sizeof(cmd_info->portid_info)) != 0 )
        return PPA_FAILURE;

   return res;
}

int32_t ppa_ioctl_del_session(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
  printk("This ioctl is no more supported. No Session will be added/deleted from user space\n");
  return PPA_FAILURE;
}


int32_t ppa_ioctl_add_session(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
  printk("This ioctl is no more supported. No Session will be added/deleted from user space\n");
  return PPA_FAILURE;
}

static inline 
void __ppa_ioctl_modify_session(PPA_CMD_SESSION_EXTRA_ENTRY *pSessExtraInfo,
                                struct session_list_item *p_item)
{
  /*below code copy from ppa_session_modify ---start*/
  ppa_update_session_extra(&pSessExtraInfo->session_extra, p_item, 
                           pSessExtraInfo->flags);

  if( pSessExtraInfo->flags & PPA_F_ACCEL_MODE) {  
    /* Only need to remove or add to accleration path and no other 
       information need to update */
			
    if (p_item->flags & SESSION_ADDED_IN_HW && pSessExtraInfo->session_extra.accel_enable == 0) {  
      //just remove the accelerated session from PPE FW.
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
		ppa_hsel_del_routing_session(p_item);
#else
		ppa_hw_del_session(p_item);
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH) && defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
		update_session_mgmt_stats(p_item, ADD); 
#endif
    }
  } else if ( pSessExtraInfo->flags != 0) {   
    //update parameters to PPE FW SESSION TABLE
    if( ppa_hw_update_session_extra(p_item, pSessExtraInfo->flags) != PPA_SUCCESS ) {

      //  update failed
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
		ppa_hsel_del_routing_session(p_item);
#else
     	ppa_hw_del_session(p_item);
#endif
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH) && defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
	  	update_session_mgmt_stats(p_item, ADD); 
#endif
    }                           
  }
  /*below code copy from ppa_session_modify ---end*/
}

int32_t ppa_ioctl_modify_session(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
  struct session_list_item *p_item=NULL;
  struct session_list_item *p_r_item=NULL;
  PPA_CMD_SESSION_EXTRA_ENTRY *pSessExtraInfo;

  pSessExtraInfo = &cmd_info->session_extra_info;

  if ( ppa_copy_from_user(pSessExtraInfo, (void *)arg, sizeof(cmd_info->session_extra_info)) != 0 )
  {
        return PPA_FAILURE;
  }

  if( pSessExtraInfo->flags == 0 || pSessExtraInfo->session == 0) 
    return PPA_SUCCESS; //no information need to update

  ppa_session_list_lock();
  if( PPA_SESSION_EXISTS ==
         __ppa_session_find_ct_hash((PPA_SESSION*)(pSessExtraInfo->session), 
                                  pSessExtraInfo->hash, &p_item) ) {
    int8_t r_direction = -1;

    ppa_debug( DBG_ENABLE_MASK_DEBUG_PRINT,
               "ConnTrack matched %x match with %x\n", 
               pSessExtraInfo->session, (uint32_t)p_item->session );

    /* Check if other direction need to be modified */
    if( p_item->flags & SESSION_WAN_ENTRY && 
         (pSessExtraInfo->lan_wan_flags & SESSION_LAN_ENTRY )) {
      r_direction = 1;
    } else if( p_item->flags & SESSION_LAN_ENTRY &&
         pSessExtraInfo->lan_wan_flags & SESSION_WAN_ENTRY ) {
      r_direction = 1;
                }

    if( r_direction != -1 ) {
      __ppa_session_find_by_ct(p_item->session,
           (p_item->flags & SESSION_IS_REPLY)?0:1, 
           &p_r_item);
	    }

    if( p_item ) {
       /*  Modify and put session */
       __ppa_ioctl_modify_session(pSessExtraInfo,p_item);
       __ppa_session_put(p_item);
                }                           

    if( p_r_item ) {
       /*  Modify and put session */
       __ppa_ioctl_modify_session(pSessExtraInfo,p_r_item);
       __ppa_session_put(p_r_item);
    }
  } 
  ppa_session_list_unlock();

  return PPA_SUCCESS;
}

#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
int32_t ppa_ioctl_get_sessions_count(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
	uint16_t session_type,session_prio,engine;

	if ( ! ppa_get_session_limit_enable(0))
		return PPA_SUCCESS;

	for(session_type=0; session_type<MAX_SESSION_TYPE; session_type++)
		for(session_prio=0; session_prio<MAX_SESSION_PRIORITY; session_prio++)
			for(engine=0; engine<MAX_DATA_FLOW_ENGINES; engine++)
				cmd_info->session_count_info.count_info[session_type][session_prio][engine].count = \
				session_count[session_type][session_prio][engine];

	if ( ppa_copy_to_user((void *)arg, &cmd_info->session_count_info,
				sizeof(PPA_CMD_SESSION_COUNT_INFO)) != 0 )
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail\n");
	return PPA_SUCCESS;
}

int32_t ppa_ioctl_get_prio_sessions(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
	PPA_CMD_GET_SESSIONS_INFO *pSessInfo;
	PPA_CMD_SESSION_COUNT_INFO *session_count;

	uint16_t session_type,session_prio,engine;
	uint32_t size=0, count=0;

	struct session_list_item *p_item, *start;
	void *pUserSessionList =((void*)arg) + 
			offsetof(PPA_CMD_GET_SESSIONS_INFO, session_list);

	extern PPA_LIST_HEAD session_list_head[MAX_SESSION_TYPE][MAX_SESSION_PRIORITY][MAX_DATA_FLOW_ENGINES];

	if ( ! ppa_get_session_limit_enable(0))
		return PPA_SUCCESS;

	session_count = &cmd_info->sessions_info.session_count;
	if ( ppa_copy_from_user(session_count, 
			(void *)arg, sizeof(PPA_CMD_SESSION_COUNT_INFO)) != 0 )
		return PPA_FAILURE;

	pSessInfo = &cmd_info->sessions_info;

	for(session_type=0; session_type<MAX_SESSION_TYPE; session_type++)
		for(session_prio=0; session_prio<MAX_SESSION_PRIORITY; session_prio++)
			for(engine=0; engine<MAX_DATA_FLOW_ENGINES; engine++) {
				count = 0;
				ppa_session_list_lock();
				ppa_list_for_each_entry_safe_reverse(p_item, start, 
					&session_list_head[session_type][session_prio][engine], priority_list) {

					if( count++ >= session_count->count_info[session_type][session_prio][engine].count)
						break;

					if(p_item->prev_sess_bytes) { 
						pSessInfo->session_list[0].flags = p_item->flags;
						pSessInfo->session_list[0].mips_bytes = p_item->mips_bytes - p_item->prev_clear_mips_bytes;
						pSessInfo->session_list[0].hw_bytes = p_item->acc_bytes - p_item->prev_clear_acc_bytes;
						pSessInfo->session_list[0].prev_sess_bytes = p_item->prev_sess_bytes;
						pSessInfo->session_list[0].session = (uint32_t ) p_item->session;
						pSessInfo->session_list[0].hash = (uint32_t ) p_item->hash;
						pSessInfo->session_list[0].collision_flag = (uint32_t ) p_item->collision_flag;	
		
						ppa_copy_to_user( pUserSessionList + size*sizeof(pSessInfo->session_list[0]),
							 pSessInfo->session_list,sizeof(pSessInfo->session_list[0]));
						size++;
					}
					
			if(session_count->count_info[session_type][session_prio][engine].stamp_flag & SESSION_BYTE_STAMPING) {
				if(p_item->flags & SESSION_ADDED_IN_HW) 
					p_item->prev_sess_bytes = p_item->acc_bytes - p_item->prev_clear_acc_bytes;
				else
					p_item->prev_sess_bytes = p_item->mips_bytes;
			}
			}
			ppa_session_list_unlock();
			session_count->count_info[session_type][session_prio][engine].count = count;
		}

		if( ppa_copy_to_user((void *)arg, &pSessInfo->session_count, 
					sizeof(PPA_CMD_SESSION_COUNT_INFO)) != 0 ) {
			ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_to_user fail\n");
			return -EFAULT;
		}
	return PPA_SUCCESS;
}

int32_t ppa_ioctl_manage_sessions(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
	struct session_list_item *p_item=NULL;
	PPA_CMD_MANAGE_SESSION *psess_buffer;
	PPA_CMD_COUNT_INFO     *count_info;
	int size = 0, index = 0;

	if ( ! ppa_get_session_limit_enable(0))
		return PPA_SUCCESS;

	count_info = &cmd_info->manage_sessions.count_info;
	if ( ppa_copy_from_user(count_info, (void *)arg, 
						sizeof(PPA_CMD_COUNT_INFO)) != 0 ) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_from_user fail\n");
	        return PPA_FAILURE;
	}

	size = sizeof(PPA_CMD_MANAGE_SESSION) + 
			(sizeof(PPA_CMD_SESSION_EXTRA_ENTRY) * ( count_info->count));

    psess_buffer = (PPA_CMD_MANAGE_SESSION *)ppa_malloc(size);
    if ( psess_buffer == NULL )
        return -EFAULT;

	if ( ppa_copy_from_user(psess_buffer, (void *)arg, size) != 0 ) {
		ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_copy_from_user fail\n");
    	ppa_free(psess_buffer);
		return PPA_FAILURE;
	}

	if ( psess_buffer->count_info.count > 0 ) {
		ppa_session_list_lock();
		for ( index = 0; index < psess_buffer->count_info.count; index++ ) {
			if( PPA_SESSION_EXISTS ==
				__ppa_session_find_ct_hash((PPA_SESSION*)psess_buffer->session_extra_info[index].session,
						psess_buffer->session_extra_info[index].hash, &p_item) ) {
				if(p_item->flags & SESSION_ADDED_IN_HW) {
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
					ppa_hsel_del_routing_session(p_item);
#else
					ppa_hw_del_session(p_item);
#endif
					p_item->flags |= SESSION_ADDED_IN_SW;
					update_session_mgmt_stats(p_item, ADD);
					p_item->prev_sess_bytes = 0x00;

				} else if(p_item->flags & SESSION_ADDED_IN_SW) {
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
     				update_session_mgmt_stats(p_item, DELETE);
					p_item->flags &= ~SESSION_ADDED_IN_SW;
#if defined(CONFIG_LTQ_PPA_HAL_SELECTOR) && CONFIG_LTQ_PPA_HAL_SELECTOR
					if(ppa_hsel_add_routing_session(p_item, 0) != PPA_SUCCESS )
#else
					if(ppa_hw_add_session(p_item, 0) != PPA_SUCCESS )
#endif
    				{
						p_item->flag2 |= SESSION_FLAG2_ADD_HW_FAIL;
					} else {
						p_item->prev_sess_bytes = 0x00;
					}
#endif
				}
       			__ppa_session_put(p_item);
			}
		}
		ppa_session_list_unlock();
	}
    ppa_free(psess_buffer);
  	return PPA_SUCCESS;
}
#endif

int32_t ppa_ioctl_set_session_timer(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    if ( ppa_copy_from_user(&cmd_info->session_timer_info, (void *)arg, sizeof(cmd_info->session_timer_info)) != 0 ) {
        return PPA_FAILURE;
    }

    if( ppa_hook_set_inactivity_fn )
        return ppa_hook_set_inactivity_fn( (PPA_U_SESSION *)cmd_info->session_timer_info.session, cmd_info->session_timer_info.timer_in_sec);
    else return PPA_FAILURE;
}

int32_t ppa_ioctl_get_session_timer(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    ppa_memset(&cmd_info->session_timer_info, 0, sizeof(cmd_info->session_timer_info) );
    cmd_info->session_timer_info.timer_in_sec = ppa_api_get_session_poll_timer();

    if ( ppa_copy_to_user( (void *)arg, &cmd_info->session_timer_info, sizeof(cmd_info->session_timer_info)) != 0 )
        return PPA_FAILURE;
    else
        return PPA_SUCCESS;
}

int32_t ppa_ioctl_set_ppe_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;

    if ( copy_from_user( &cmd_info->ppe_fastpath_enable_info, (void *)arg, sizeof(cmd_info->ppe_fastpath_enable_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_hook_set_ppe_fastpath_enable( cmd_info->ppe_fastpath_enable_info.ppe_fastpath_enable, cmd_info->ppe_fastpath_enable_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_get_ppe_fastpath_enable fail\n");
        res = PPA_FAILURE;
    }

   return res;
}

int32_t ppa_ioctl_get_ppe_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;

    ppa_memset(&cmd_info->ppe_fastpath_enable_info, 0, sizeof(cmd_info->ppe_fastpath_enable_info) );

    if ( copy_from_user( &cmd_info->ppe_fastpath_enable_info, (void *)arg, sizeof(cmd_info->ppe_fastpath_enable_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_hook_get_ppe_fastpath_enable( &cmd_info->ppe_fastpath_enable_info.ppe_fastpath_enable, cmd_info->ppe_fastpath_enable_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_get_ppe_fastpath_enable fail\n");
        res = PPA_FAILURE;
    }
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->ppe_fastpath_enable_info, sizeof(cmd_info->ppe_fastpath_enable_info)) != 0 )
        return PPA_FAILURE;

   return res;
}

#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
int32_t ppa_ioctl_set_sw_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;

    if ( copy_from_user( &cmd_info->sw_fastpath_enable_info, (void *)arg, sizeof(cmd_info->sw_fastpath_enable_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_sw_fastpath_enable( cmd_info->sw_fastpath_enable_info.sw_fastpath_enable, cmd_info->sw_fastpath_enable_info.flags);

    if ( res == PPA_SUCCESS ) {
      if( cmd_info->sw_fastpath_enable_info.sw_fastpath_enable ) {
        /* Re-register the hooks */
	      ppa_reg_export_fn(PPA_SW_FASTPATH_SEND_FN, 
                          (uint32_t) ppa_sw_fastpath_send, 
                          "ppa_sw_fastpath_send", 
                          (uint32_t *)&ppa_hook_sw_fastpath_send_fn,
                          (uint32_t)ppa_hook_sw_fastpath_send_rpfn);
      } else {
        /* Un-register the hooks */
        ppa_unreg_export_fn(PPA_SW_FASTPATH_SEND_FN, 
                            (uint32_t *)&ppa_hook_sw_fastpath_send_fn);
      }

    } else {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_set_sw_fastpath_enable fail\n");
    }

   return res;
}

int32_t ppa_ioctl_get_sw_fastpath_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
    int res=PPA_FAILURE;

    ppa_memset(&cmd_info->sw_fastpath_enable_info, 0, sizeof(cmd_info->sw_fastpath_enable_info) );

    if ( copy_from_user( &cmd_info->sw_fastpath_enable_info, (void *)arg, sizeof(cmd_info->sw_fastpath_enable_info)) != 0 )
        return PPA_FAILURE;

    res = ppa_get_sw_fastpath_status( &cmd_info->sw_fastpath_enable_info.sw_fastpath_enable, cmd_info->sw_fastpath_enable_info.flags);

    if ( res != PPA_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_get_sw_fastpath_enable fail\n");
        res = PPA_FAILURE;
    }
    if ( ppa_copy_to_user( (void *)arg, &cmd_info->sw_fastpath_enable_info, sizeof(cmd_info->sw_fastpath_enable_info)) != 0 )
        return PPA_FAILURE;

   return res;
}

int32_t ppa_ioctl_set_sw_session_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
  int res=PPA_FAILURE;
  PPA_CMD_SW_SESSION_ENABLE_INFO *pSwSessEnableInfo;
  struct session_list_item *p_item=NULL;

  pSwSessEnableInfo = &cmd_info->sw_session_enable_info;

  if ( ppa_copy_from_user(pSwSessEnableInfo, (void *)arg, sizeof(cmd_info->sw_session_enable_info)) != 0 )
  {
    return PPA_FAILURE;
  }

  if( pSwSessEnableInfo->session == 0 ) {
    ppa_debug( DBG_ENABLE_MASK_DEBUG_PRINT,
               "ppa_ioctl_set_sw_session_enable failed as session 0x%x\n", 
               pSwSessEnableInfo->session );
    return PPA_FAILURE; //sw session can not be enabled
  }

  ppa_session_list_lock();
  if( PPA_SESSION_EXISTS ==
         __ppa_session_find_by_ct((PPA_SESSION*)(pSwSessEnableInfo->session), 
                                  0, &p_item) ) {

    ppa_debug( DBG_ENABLE_MASK_DEBUG_PRINT,
               "ConnTrack matched 0x%x match with 0x%x\n", 
               pSwSessEnableInfo->session, (uint32_t)p_item->session );

    /* Check if other direction need to be modified */
    if ( (p_item->flags & SESSION_ADDED_IN_SW) ) {
      res = ppa_sw_session_enable( p_item, pSwSessEnableInfo->sw_session_enable, pSwSessEnableInfo->flags );

      if ( res != PPA_SUCCESS )
      {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_set_sw_session_enable fail\n");
        res = PPA_FAILURE;
      }
    } else {
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"No point to enable or disable session in sw accel.\n");
      res = PPA_SUCCESS; // No point to enable/disable
    }
  } else {
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Session 0x%x does not exist!!!\n", pSwSessEnableInfo->session);
  }

  ppa_session_list_unlock();

  return res;
}

int32_t ppa_ioctl_get_sw_session_enable(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{
  int res=PPA_FAILURE;
  PPA_CMD_SW_SESSION_ENABLE_INFO *pSwSessEnableInfo;
  struct session_list_item *p_item=NULL;

  pSwSessEnableInfo = &cmd_info->sw_session_enable_info;
  pSwSessEnableInfo->sw_session_enable = 0;

  if ( ppa_copy_from_user(pSwSessEnableInfo, (void *)arg, sizeof(cmd_info->sw_session_enable_info)) != 0 )
  {
    return PPA_FAILURE;
  }

  if( pSwSessEnableInfo->session == 0 ) {
    ppa_debug( DBG_ENABLE_MASK_DEBUG_PRINT,
               "ppa_ioctl_get_sw_session_enable failed as session=0x%x\n", 
               pSwSessEnableInfo->session );
    return PPA_FAILURE; //sw session can not be enabled
  }

  ppa_session_list_lock();
  if( PPA_SESSION_EXISTS ==
         __ppa_session_find_by_ct((PPA_SESSION*)(pSwSessEnableInfo->session), 
                                  0, &p_item) ) {

    ppa_debug( DBG_ENABLE_MASK_DEBUG_PRINT,
               "ConnTrack matched 0x%x match with 0x%x\n", 
               pSwSessEnableInfo->session, (uint32_t)p_item->session );

    /* Check if other direction need to be modified */
    if ( (p_item->flags & SESSION_ADDED_IN_SW) ) {
      res = ppa_get_sw_session_status( p_item, &pSwSessEnableInfo->sw_session_enable, pSwSessEnableInfo->flags );

      if ( res != PPA_SUCCESS )
      {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"ppa_ioctl_get_sw_session_enable fail\n");
        res = PPA_FAILURE;
      }
    } else {
      ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"No point to enable or disable session in sw accel.\n");
      res = PPA_SUCCESS; // No point to enable/disable
    }
  } else {
    ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT,"Session 0x%x does not exist!!!\n", pSwSessEnableInfo->session);
  }

  ppa_session_list_unlock();

  if ( ppa_copy_to_user( (void *)arg, pSwSessEnableInfo, sizeof(cmd_info->sw_session_enable_info)) != 0 )
    return PPA_FAILURE;

  return res;
}
#endif


/*
 * ####################################
 *           Global Function
 * ####################################
 */



/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

void ppa_reg_expfn(void)
{
  //extern uint32_t ppa_enable_rpfn, ppa_get_status_rpfn;
  ppa_reg_export_fn(PPA_ENABLE_FN,        (uint32_t)ppa_enable, "ppa_enable", (uint32_t *)&ppa_hook_enable_fn,(uint32_t)ppa_enable_rpfn );
  ppa_reg_export_fn(PPA_GET_STATUS_FN,    (uint32_t)ppa_get_status, "ppa_get_status", (uint32_t *)&ppa_hook_get_status_fn, (uint32_t)ppa_get_status_rpfn);

  //extern uint32_t ppa_session_add_rpfn, ppa_session_del_rpfn,ppa_session_modify_rpfn, ppa_session_get_rpfn;
  ppa_reg_export_fn(PPA_SESSION_ADD_FN,   (uint32_t)ppa_session_add, "ppa_session_add",(uint32_t *)&ppa_hook_session_add_fn, (uint32_t)ppa_session_add_rpfn );
  ppa_reg_export_fn(PPA_SESSION_DEL_FN,   (uint32_t)ppa_session_delete, "ppa_session_delete",(uint32_t *)&ppa_hook_session_del_fn, (uint32_t)ppa_session_del_rpfn);
  ppa_reg_export_fn(PPA_SESSION_MODIFY_FN,(uint32_t)ppa_session_modify, "ppa_session_modify", (uint32_t *)&ppa_hook_session_modify_fn,(uint32_t)ppa_session_modify_rpfn );
  ppa_reg_export_fn(PPA_SESSION_GET_FN,   (uint32_t)ppa_session_get, "ppa_session_get", (uint32_t *)&ppa_hook_session_get_fn, (uint32_t)ppa_session_get_rpfn);
  ppa_reg_export_fn(PPA_SESSION_BRADD_FN, (uint32_t)ppa_session_add, "ppa_session_bradd",(uint32_t *)&ppa_hook_session_bradd_fn, (uint32_t)ppa_session_bradd_rpfn );
#if defined(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS)
  ppa_reg_export_fn(PPA_SESSION_PRIO_FN,   (uint32_t)ppa_session_prio, "ppa_session_prio", (uint32_t *)&ppa_hook_session_prio_fn, (uint32_t)ppa_session_prio_rpfn);
#endif
#if defined(CONFIG_LTQ_PPA_MPE_IP97)
  ppa_reg_export_fn(PPA_SESSION_IPSEC_ADD_FN,   (uint32_t)ppa_session_ipsec_add, "ppa_session_ipsec_add", (uint32_t *)&ppa_hook_session_ipsec_add_fn, (uint32_t)ppa_session_ipsec_add_rpfn);
  ppa_reg_export_fn(PPA_SESSION_IPSEC_DEL_FN,   (uint32_t)ppa_session_ipsec_delete, "ppa_session_ipsec_delete", (uint32_t *)&ppa_hook_session_ipsec_del_fn, (uint32_t)ppa_session_ipsec_del_rpfn);
#endif

  //extern uint32_t ppa_mc_group_update_rpfn,ppa_mc_group_get_rpfn,ppa_mc_entry_get_rpfn,ppa_mc_entry_modify_rpfn,ppa_mc_pkt_srcif_add_rpfn;   
  ppa_reg_export_fn(PPA_MC_GROUP_UPDATE_FN,   (uint32_t)ppa_mc_group_update, "ppa_mc_group_update", (uint32_t *)&ppa_hook_mc_group_update_fn,(uint32_t)ppa_mc_group_update_rpfn );
  ppa_reg_export_fn(PPA_MC_GROUP_GET_FN,      (uint32_t)ppa_mc_group_get, "ppa_mc_group_get",(uint32_t *)&ppa_hook_mc_group_get_fn,(uint32_t)ppa_mc_group_get_rpfn  );
  ppa_reg_export_fn(PPA_MC_ENTRY_MODIFY_FN,   (uint32_t)ppa_mc_entry_modify, "ppa_mc_entry_modify", (uint32_t *)&ppa_hook_mc_entry_modify_fn,(uint32_t)ppa_mc_entry_modify_rpfn );
  ppa_reg_export_fn(PPA_MC_ENTRY_GET_FN,      (uint32_t)ppa_mc_entry_get, "ppa_mc_entry_get", (uint32_t *)&ppa_hook_mc_entry_get_fn,(uint32_t)ppa_mc_entry_get_rpfn );
  ppa_reg_export_fn(PPA_MC_PKT_SRCIF_ADD_FN,  (uint32_t)ppa_multicast_pkt_srcif_add, "ppa_multicast_pkt_srcif_add",(uint32_t *)&ppa_hook_multicast_pkt_srcif_add_fn,(uint32_t)ppa_mc_pkt_srcif_add_rpfn);
        
  //extern uint32_t ppa_inactivity_status_rpfn, ppa_set_inactivity_rpfn;    
  ppa_reg_export_fn(PPA_INACTIVITY_STATUS_FN, (uint32_t)ppa_inactivity_status, "ppa_inactivity_status",(uint32_t *)&ppa_hook_inactivity_status_fn, (uint32_t)ppa_inactivity_status_rpfn );
  ppa_reg_export_fn(PPA_SET_INACTIVITY_FN,    (uint32_t)ppa_set_session_inactivity, "ppa_set_session_inactivity",(uint32_t *)&ppa_hook_set_inactivity_fn, (uint32_t)ppa_set_inactivity_rpfn );
        
  //extern uint32_t ppa_bridge_entry_add_rpfn, ppa_bridge_entry_delete_rpfn,ppa_bridge_entry_hit_time_rpfn,ppa_bridge_entry_inactivity_status_rpfn,ppa_set_bridge_entry_timeout_rpfn, ppa_bridge_enable_rpfn;
  ppa_reg_export_fn(PPA_BRIDGE_ENTRY_ADD_FN,              (uint32_t)ppa_bridge_entry_add, "ppa_bridge_entry_add", (uint32_t *)&ppa_hook_bridge_entry_add_fn,(uint32_t)ppa_bridge_entry_add_rpfn );
  ppa_reg_export_fn(PPA_BRIDGE_ENTRY_DELETE_FN,           (uint32_t)ppa_bridge_entry_delete, "ppa_bridge_entry_delete",(uint32_t *)&ppa_hook_bridge_entry_delete_fn, (uint32_t)ppa_bridge_entry_delete_rpfn );
  ppa_reg_export_fn(PPA_BRIDGE_ENTRY_HIT_TIME_FN,         (uint32_t)ppa_bridge_entry_hit_time, "ppa_bridge_entry_hit_time",(uint32_t *)&ppa_hook_bridge_entry_hit_time_fn,(uint32_t)ppa_bridge_entry_hit_time_rpfn  );
  ppa_reg_export_fn(PPA_BRIDGE_ENTRY_INACTIVITY_STATUS_FN,(uint32_t)ppa_bridge_entry_inactivity_status, "ppa_bridge_entry_inactivity_status", (uint32_t *)&ppa_hook_bridge_entry_inactivity_status_fn,(uint32_t)ppa_bridge_entry_inactivity_status_rpfn );
  ppa_reg_export_fn(PPA_SET_BRIDGE_ENTRY_TIMEOUT_FN,      (uint32_t)ppa_set_bridge_entry_timeout, "ppa_set_bridge_entry_timeout",(uint32_t *)&ppa_hook_set_bridge_entry_timeout_fn, (uint32_t)ppa_set_bridge_entry_timeout_rpfn );
  ppa_reg_export_fn(PPA_BRIDGE_ENABLE_FN,                 (uint32_t)ppa_hook_bridge_enable, "ppa_hook_bridge_enable",(uint32_t *)&ppa_hook_bridge_enable_fn,(uint32_t)ppa_bridge_enable_rpfn  );
        
  //extern uint32_t ppa_set_bridge_if_vlan_config_rpfn,ppa_get_bridge_if_vlan_config_rpfn,ppa_vlan_filter_add_rpfn,ppa_vlan_filter_del_rpfn,ppa_vlan_filter_get_all_rpfn,ppa_vlan_filter_del_all_rpfn;
  ppa_reg_export_fn(PPA_SET_BRIDGE_IF_VLAN_CONFIG_FN, (uint32_t)ppa_set_bridge_if_vlan_config, "ppa_set_bridge_if_vlan_config",(uint32_t *)&ppa_hook_set_bridge_if_vlan_config_fn, (uint32_t)ppa_set_bridge_if_vlan_config_rpfn );
  ppa_reg_export_fn(PPA_GET_BRIDGE_IF_VLAN_CONFIG_FN, (uint32_t)ppa_get_bridge_if_vlan_config, "ppa_get_bridge_if_vlan_config",(uint32_t *)&ppa_hook_get_bridge_if_vlan_config_fn, (uint32_t)ppa_get_bridge_if_vlan_config_rpfn );
  ppa_reg_export_fn(PPA_VLAN_FILTER_ADD_FN,           (uint32_t)ppa_vlan_filter_add,"ppa_vlan_filter_add", (uint32_t *)&ppa_hook_vlan_filter_add_fn, (uint32_t)ppa_vlan_filter_add_rpfn );
  ppa_reg_export_fn(PPA_VLAN_FILTER_DEL_FN,           (uint32_t)ppa_vlan_filter_del, "ppa_vlan_filter_del", (uint32_t *)&ppa_hook_vlan_filter_del_fn, (uint32_t)ppa_vlan_filter_del_rpfn);
  ppa_reg_export_fn(PPA_VLAN_FILTER_GET_ALL_FN,       (uint32_t)ppa_vlan_filter_get_all, "ppa_vlan_filter_get_all", (uint32_t *)&ppa_hook_vlan_filter_get_all_fn,(uint32_t)ppa_vlan_filter_get_all_rpfn );
  ppa_reg_export_fn(PPA_VLAN_FILTER_DEL_ALL_FN,       (uint32_t)ppa_vlan_filter_del_all, "ppa_vlan_filter_del_all", (uint32_t *)&ppa_hook_vlan_filter_del_all_fn,(uint32_t)ppa_vlan_filter_del_all_rpfn );
        
  //extern uint32_t ppa_get_if_stats_rpfn,ppa_get_accel_stats_rpfn;
  ppa_reg_export_fn(PPA_GET_IF_STATS_FN,      (uint32_t)ppa_get_if_stats, "ppa_get_if_stats",(uint32_t *)&ppa_hook_get_if_stats_fn,(uint32_t)ppa_get_if_stats_rpfn);
  ppa_reg_export_fn(PPA_GET_ACCEL_STATS_FN,   (uint32_t)ppa_get_accel_stats, "ppa_get_accel_stats", (uint32_t *)&ppa_hook_get_accel_stats_fn, (uint32_t)ppa_get_accel_stats_rpfn);
#if defined(CONFIG_LTQ_PPA_IF_MIB) && CONFIG_LTQ_PPA_IF_MIB
  ppa_reg_export_fn(PPA_GET_NETIF_ACCEL_STATS_FN,   (uint32_t)ppa_get_netif_accel_stats, "ppa_get_netif_accel_stats", (uint32_t *)&ppa_hook_get_netif_accel_stats_fn, (uint32_t)ppa_get_netif_accel_stats_rpfn);
#endif
        
  //extern uint32_t ppa_set_if_mac_address_rpfn, ppa_get_if_mac_address_rpfn;
  ppa_reg_export_fn(PPA_SET_IF_MAC_ADDRESS_FN, (uint32_t)ppa_set_if_mac_address, "ppa_set_if_mac_address",(uint32_t *)&ppa_hook_set_if_mac_address_fn,(uint32_t)ppa_set_if_mac_address_rpfn  );
  ppa_reg_export_fn(PPA_GET_IF_MAC_ADDRESS_FN, (uint32_t)ppa_get_if_mac_address, "ppa_get_if_mac_address",(uint32_t *)&ppa_hook_get_if_mac_address_fn, (uint32_t)ppa_get_if_mac_address_rpfn );
        
  //extern uint32_t ppa_add_if_rpfn, ppa_del_if_rpfn, ppa_get_if_rpfn;
  ppa_reg_export_fn(PPA_ADD_IF_FN, (uint32_t)ppa_add_if, "ppa_add_if",(uint32_t *)&ppa_hook_add_if_fn,(uint32_t)ppa_add_if_rpfn) ;
  ppa_reg_export_fn(PPA_DEL_IF_FN, (uint32_t)ppa_del_if, "ppa_del_if", (uint32_t *)&ppa_hook_del_if_fn,(uint32_t)ppa_del_if_rpfn );
  ppa_reg_export_fn(PPA_GET_IF_FN, (uint32_t)ppa_get_if, "ppa_get_if", (uint32_t *)&ppa_hook_get_if_fn,(uint32_t)ppa_get_if_rpfn );
#if defined(CONFIG_PPA_API_DIRECTCONNECT) && CONFIG_PPA_API_DIRECTCONNECT
  ppa_reg_export_fn(PPA_DISCONN_IF_FN, (uint32_t)ppa_disconn_if, "ppa_disconn_if", (uint32_t *)&ppa_hook_disconn_if_fn,(uint32_t)ppa_disconn_if_rpfn );
#if defined(WMM_QOS_CONFIG) && WMM_QOS_CONFIG
  ppa_reg_export_fn(PPA_REG_CLASS2PRIO_FN, (uint32_t)ppa_register_for_qos_class2prio, "ppa_register_for_qos_class2prio", (uint32_t *)&ppa_register_qos_class2prio_hook_fn,(uint32_t)ppa_register_for_qos_class2prio_rpfn );
#endif
#endif

#ifdef CONFIG_LTQ_PPA_API_DIRECTPATH
  //extern uint32_t ppa_directpath_register_dev_rpfn, ppa_directpath_send_rpfn, ppa_directpath_rx_stop_rpfn,ppa_directpath_rx_restart_rpfn,ppa_get_netif_for_ppa_ifid_rpfn,ppa_get_ifid_for_netif_rpfn;
  ppa_reg_export_fn(PPA_DIRECTPATH_EX_REGISTER_DEV_FN,(uint32_t)ppa_directpath_ex_register_dev, "ppa_directpath_ex_register_dev", (uint32_t *)&ppa_hook_directpath_ex_register_dev_fn,(uint32_t)ppa_directpath_ex_register_dev_rpfn );
  ppa_reg_export_fn(PPA_DIRECTPATH_REGISTER_DEV_FN,   (uint32_t)ppa_directpath_register_dev, "ppa_directpath_register_dev", (uint32_t *)&ppa_hook_directpath_register_dev_fn,(uint32_t)ppa_directpath_register_dev_rpfn );
  ppa_reg_export_fn(PPA_DIRECTPATH_SEND_FN,           (uint32_t)ppa_directpath_send, "ppa_directpath_send", (uint32_t *)&ppa_hook_directpath_send_fn,(uint32_t)ppa_directpath_send_rpfn );
  ppa_reg_export_fn(PPA_DIRECTPATH_EX_SEND_FN,        (uint32_t)ppa_directpath_ex_send, "ppa_directpath_ex_send", (uint32_t *)&ppa_hook_directpath_ex_send_fn,(uint32_t)ppa_directpath_ex_send_rpfn );
  ppa_reg_export_fn(PPA_DIRECTPATH_RX_STOP_FN,        (uint32_t)ppa_directpath_rx_stop, "ppa_directpath_rx_stop", (uint32_t *)&ppa_hook_directpath_rx_stop_fn, (uint32_t)ppa_directpath_rx_stop_rpfn);
  ppa_reg_export_fn(PPA_DIRECTPATH_EX_RX_STOP_FN,        (uint32_t)ppa_directpath_ex_rx_stop, "ppa_directpath_ex_rx_stop", (uint32_t *)&ppa_hook_directpath_ex_rx_stop_fn, (uint32_t)ppa_directpath_ex_rx_stop_rpfn);
  ppa_reg_export_fn(PPA_DIRECTPATH_RX_RESTART_FN,     (uint32_t)ppa_directpath_rx_restart, "ppa_directpath_rx_restart",(uint32_t *)&ppa_hook_directpath_rx_restart_fn, (uint32_t)ppa_directpath_rx_restart_rpfn);
  ppa_reg_export_fn(PPA_DIRECTPATH_EX_RX_RESTART_FN,     (uint32_t)ppa_directpath_ex_rx_restart, "ppa_directpath_ex_rx_restart",(uint32_t *)&ppa_hook_directpath_ex_rx_restart_fn, (uint32_t)ppa_directpath_ex_rx_restart_rpfn);
  ppa_reg_export_fn(PPA_DIRECTPATH_ALLOC_SKB_FN,     (uint32_t)ppa_directpath_alloc_skb, "ppa_directpath_alloc_skb",(uint32_t *)&ppa_hook_directpath_alloc_skb_fn, (uint32_t)ppa_directpath_alloc_skb_rpfn);
  ppa_reg_export_fn(PPA_DIRECTPATH_RECYCLE_SKB_FN,     (uint32_t)ppa_directpath_recycle_skb, "ppa_directpath_recycle_skb",(uint32_t *)&ppa_hook_directpath_recycle_skb_fn, (uint32_t)ppa_directpath_recycle_skb_rpfn);
  ppa_reg_export_fn(PPA_GET_NETIF_FOR_PPA_IFID_FN,    (uint32_t)ppa_get_netif_for_ppa_ifid, "ppa_get_netif_for_ppa_ifid", (uint32_t *)&ppa_hook_get_netif_for_ppa_ifid_fn,(uint32_t)ppa_get_netif_for_ppa_ifid_rpfn );
  ppa_reg_export_fn(PPA_GET_IFID_FOR_NETIF_FN,        (uint32_t)ppa_get_ifid_for_netif, "ppa_get_ifid_for_netif",(uint32_t *)&ppa_hook_get_ifid_for_netif_fn,(uint32_t)ppa_get_ifid_for_netif_rpfn  );
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
  ppa_reg_export_fn(PPA_DTLK_REGISTER_DEV_FN, (uint32_t)ppa_directlink_register_dev, "ppa_directlink_register_dev", (uint32_t *)&ppa_hook_directlink_register_dev_fn,(uint32_t)ppa_directlink_register_dev_rpfn );
#endif /*CONFIG_ACCL_11AC*/
#endif /* CONFIG_LTQ_PPA_API_DIRECTPATH */
        
  //extern uint32_t ppa_wan_mii0_vlan_range_add_rpfn,ppa_wan_mii0_vlan_range_del_rpfn,ppa_wan_mii0_vlan_ranges_get_rpfn;
  ppa_reg_export_fn(PPA_WAN_MII0_VLAN_RANGE_ADD_FN,   (uint32_t)ppa_hook_wan_mii0_vlan_range_add,"ppa_hook_wan_mii0_vlan_range_add", (uint32_t *)&ppa_hook_wan_mii0_vlan_range_add_fn,(uint32_t)ppa_wan_mii0_vlan_range_add_rpfn );
  ppa_reg_export_fn(PPA_WAN_MII0_VLAN_RANGE_DEL_FN,   (uint32_t)ppa_hook_wan_mii0_vlan_range_del, "ppa_hook_wan_mii0_vlan_range_del",(uint32_t *)&ppa_hook_wan_mii0_vlan_range_del_fn,(uint32_t)ppa_wan_mii0_vlan_range_del_rpfn  );
  ppa_reg_export_fn(PPA_WAN_MII0_VLAN_RANGES_GET_FN,  (uint32_t)ppa_hook_wan_mii0_vlan_ranges_get, "ppa_hook_wan_mii0_vlan_ranges_get",(uint32_t *)&ppa_hook_wan_mii0_vlan_ranges_get_fn, (uint32_t)ppa_wan_mii0_vlan_ranges_get_rpfn);

#if defined(QOS_HOOK_EXPORT_SUPPORT) /* qos functions not used by kernel by now. So not export yet*/
#ifdef CONFIG_LTQ_PPA_QOS   
  //extern uint32_t ppa_get_qos_qnum_rpfn, ppa_get_qos_mib_rpfn;
  ppa_reg_export_fn(PPA_GET_DSL_MIB_FN,   (uint32_t)ppa_get_dsl_mib, "ppa_get_dsl_mib", (uint32_t *)&ppa_hook_get_qos_qnum,(uint32_t)ppa_get_qos_qnum_rpfn );
  ppa_reg_export_fn(PPA_GET_PORT_MIB_FN,  (uint32_t)ppa_get_ports_mib, "ppa_get_ports_mib",(uint32_t *)&ppa_hook_get_qos_mib,(uint32_t)ppa_get_qos_mib_rpfn  );
#endif /* CONFIG_LTQ_PPA_QOS_RATE_SHAPING */
#endif /* CONFIG_LTQ_PPA_QOS */
        
  ppa_reg_export_fn(PPA_SET_LAN_SEPARATE_FLAG_FN,   (uint32_t)ppa_hook_set_lan_seperate_flag, "ppa_hook_set_lan_seperate_flag",(uint32_t *)&ppa_hook_set_lan_seperate_flag_fn,(uint32_t)ppa_hook_set_lan_seperate_flag_rpfn );
  ppa_reg_export_fn(PPA_GET_LAN_SEPARATE_FLAG_FN,   (uint32_t)ppa_hook_get_lan_seperate_flag, "ppa_hook_get_lan_seperate_flag",(uint32_t *)&ppa_hook_get_lan_seperate_flag_fn,(uint32_t)ppa_hook_get_lan_seperate_flag_rpfn  );
  ppa_reg_export_fn(PPA_SET_WAN_SEPARATE_FLAG_FN,   (uint32_t)ppa_hook_set_wan_seperate_flag, "ppa_hook_set_wan_seperate_flag",(uint32_t *)&ppa_hook_set_wan_seperate_flag_fn,(uint32_t)ppa_hook_set_wan_seperate_flag_rpfn );
  ppa_reg_export_fn(PPA_GET_WAN_SEPARATE_FLAG_FN,   (uint32_t)ppa_hook_get_wan_seperate_flag, "ppa_hook_get_wan_seperate_flag",(uint32_t *)&ppa_hook_get_wan_seperate_flag_fn,(uint32_t)ppa_hook_get_wan_seperate_flag_rpfn  );
#if defined(CONFIG_LTQ_PPA_API_SW_FASTPATH)
	ppa_reg_export_fn(PPA_ENABLE_SW_FASTPATH_FN, (uint32_t) ppa_sw_fastpath_enable, "ppa_sw_fastpath_enable", (uint32_t *)&ppa_hook_set_sw_fastpath_enable_fn, (uint32_t)ppa_hook_set_sw_fastpath_enable_rpfn );
	ppa_reg_export_fn(PPA_GET_SW_FASTPATH_STATUS_FN, (uint32_t) ppa_get_sw_fastpath_status, "ppa_get_sw_fastpath_status", (uint32_t *)&ppa_hook_get_sw_fastpath_status_fn,(uint32_t)ppa_hook_get_sw_fastpath_status_rpfn );
	ppa_reg_export_fn(PPA_SW_FASTPATH_SEND_FN, (uint32_t) ppa_sw_fastpath_send, "ppa_sw_fastpath_send", (uint32_t *)&ppa_hook_sw_fastpath_send_fn,(uint32_t)ppa_hook_sw_fastpath_send_rpfn);
#endif
}

int __init ppa_driver_init(void)
{
    int ret;

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
    if ( ppa_api_directpath_create() != PPA_SUCCESS )
        goto PPA_API_DIRECTPATH_CREATE_FAIL;
#endif

    if ( ppa_api_netif_manager_create() != PPA_SUCCESS )
        goto PPA_API_NETIF_MANAGER_CREATE_FAIL;

    if ( ppa_api_session_manager_create() != PPA_SUCCESS )
        goto PPA_API_SESSION_MANAGER_CREATE_FAIL;

#if defined(CONFIG_LTQ_PPA_API_PROC)
    ppa_api_proc_file_create();
#endif

    ppa_hook_init_fn = ppa_init;
    ppa_hook_exit_fn = ppa_exit;
    ppa_reg_expfn();
    ret = ppa_register_chrdev(PPA_CHR_DEV_MAJOR, PPA_DEV_NAME, &g_ppa_dev_ops);
    if ( ret != 0 )
        printk("PPA API --- failed in register_chrdev(%d, %s, g_ppa_dev_ops), errno = %d\n", PPA_CHR_DEV_MAJOR, PPA_DEV_NAME, ret);

#if defined(CONFIG_LTQ_PPA_API_PROC)
    proc_file_create();
#endif

    printk("PPA API --- init successfully\n");
    return 0;

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
PPA_API_DIRECTPATH_CREATE_FAIL:
    ppa_api_directpath_destroy();
#endif
PPA_API_NETIF_MANAGER_CREATE_FAIL:
    ppa_api_session_manager_destroy();
PPA_API_SESSION_MANAGER_CREATE_FAIL:
    ppa_drv_hal_exit(0);
    return -1;
}

void __exit ppa_driver_exit(void)
{
#if defined(CONFIG_LTQ_PPA_API_PROC)
    proc_file_delete();
#endif

    ppa_unregister_chrdev(PPA_CHR_DEV_MAJOR, PPA_DEV_NAME);

    ppa_hook_init_fn = NULL;
    ppa_hook_exit_fn = NULL;
    ppa_exit();
    ppa_export_fn_manager_exit();

#if defined(CONFIG_LTQ_PPA_API_PROC)
    ppa_api_proc_file_destroy();
#endif

    ppa_api_session_manager_destroy();

    ppa_api_netif_manager_destroy();

#if defined(CONFIG_LTQ_PPA_API_DIRECTPATH)
    ppa_api_directpath_destroy();
#endif
}

module_init(ppa_driver_init);
module_exit(ppa_driver_exit);

MODULE_LICENSE("GPL");

