/*******************************************************************************
**
** FILE NAME    : ppa_api_tools.c
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 18 March 2010
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack Tools API Implementation
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 18 March 2010  Shao Guohua        Initiate Version
*******************************************************************************/

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



//#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/version.h>
//#include <linux/types.h>
//#include <linux/init.h>
//#include <linux/slab.h>
//#include <linux/netdevice.h>
//#include <linux/in.h>
//#include <net/sock.h>
//#include <net/ip_vs.h>
//#include <asm/time.h>

/*
 *  PPA Specific Head File
 */
#include "ppa_ss.h"
#include <net/ppa_api_common.h>
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_misc.h"
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#include <net/ppa_hook.h>
#include "ppa_api_core.h"

#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)
#include "ppa_api_pwm.h"
#include "ppa_api_pwm_logic.h"
#endif

#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_hal_wrapper.h"
#include <net/ppa_ppe_hal.h>
#include "../platform/ppa_datapath.h"
#include "ppa_api_mib.h"

#define PPA_WRPFN_DEFINITION
#include "ppa_api_tools.h"


/*
 * Definition
*/
#define QOS_FUNC_EXPORT_SUPPORT 0
#define MFE_FUNC_EXPORT_SUPPORT 0


/* PPA init flag */
static uint32_t g_init = 0;
PPA_HOOK_INFO g_expfn_table[PPA_HOOK_FN_MAX];

/* PPA init flag get/set functions */
void ppa_set_init_status(PPA_INIT_STATUS_t state)
{
    g_init = state;
}

uint32_t ppa_is_init(void)
{
    return g_init;
}

#ifdef CONFIG_LTQ_PPA_API_DIRECTPATH
/*special return value, cannot use macro template*/
PPA_NETIF* ppa_get_netif_for_ppa_ifid_rpfn(uint32_t if_id)
{
    PPA_NETIF *ret = NULL;
    PPA_NETIF* (*pfn)(uint32_t) = NULL;

    ppa_rcu_read_lock(); 
    if( ppa_is_init() &&
        g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].used_flag && 
        g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].hook_flag ){           
             if( in_irq() )  {  
                critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].hookname, preempt_count() );  
             } 
            else if( irqs_disabled() ) {  
                critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].hookname, preempt_count() );  
            }
            else {
                pfn = (PPA_NETIF* (*)(uint32_t))g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].hook_addr;
                if(pfn){                            
                    ret = pfn(if_id);               
                }
            }
    }
    ppa_rcu_read_unlock();

    return ret;
}
EXPORT_SYMBOL(ppa_get_netif_for_ppa_ifid_rpfn);

int32_t ppa_directpath_send_rpfn(uint32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags)
{          
    int32_t (*pfn)(uint32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags) = NULL;
    uint8_t  drop_flag = 0;
    int32_t ret = PPA_EINVAL;

    ppa_rcu_read_lock(); 
    if( ppa_is_init() &&
        g_expfn_table[PPA_DIRECTPATH_SEND_FN].used_flag && 
        g_expfn_table[PPA_DIRECTPATH_SEND_FN].hook_flag ){          
            if( in_irq() )  {  
                critial_err( "\n----Warning: %s is not allowed in_irq context with preempt_count=%x  !!!!!\n", g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].hookname, preempt_count() );  
                drop_flag = 1; 
             } 
            else if( irqs_disabled() ) {  
                critial_err( "\n----Warning: %s is not allowed irqs_disabled mode with preempt_count=%x  !!!!!\n", g_expfn_table[PPA_GET_NETIF_FOR_PPA_IFID_FN].hookname, preempt_count() );  
                drop_flag = 1; 
            }
            else {
                pfn =  (int32_t (*)(uint32_t rx_if_id, PPA_BUF *skb, int32_t len, uint32_t flags))g_expfn_table[PPA_DIRECTPATH_SEND_FN].hook_addr;
                if(pfn){                            
                    ret = pfn(rx_if_id, skb,len,flags);
                } 
                else drop_flag = 1;
            }			
    }  
    else drop_flag = 1;   
    ppa_rcu_read_unlock();

    if( drop_flag )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_directpath_send_rpfn drop packet\n");       
        if( skb) 
            PPA_SKB_FREE(skb);
        return PPA_EINVAL;
    }
    return ret;
}
EXPORT_SYMBOL(ppa_directpath_send_rpfn);
#endif

/*
  * No lock, so should be called only on module initialization
*/
void ppa_reg_export_fn(PPA_EXPORT_FN_NO fn_no, uint32_t in_fn_addr, uint8_t *name,uint32_t *out_fn_addr, uint32_t mid_fn_addr )
{
    if(fn_no >= PPA_HOOK_FN_MAX){
        err("Error: function reg no is bigger than max Number !!!\n");
        return;
    }

    if( g_expfn_table[fn_no].used_flag ){
        err("Warning: function has been registered, NO: %d, address: 0x%x\n", 
            fn_no, g_expfn_table[fn_no].hook_addr);
        return;
    }

    ppa_rcu_read_lock(); 
    
    g_expfn_table[fn_no].hook_addr = in_fn_addr;
    ppa_strncpy(g_expfn_table[fn_no].hookname,name,sizeof(g_expfn_table[fn_no].hookname));
    g_expfn_table[fn_no].hook_flag =1;
    g_expfn_table[fn_no].used_flag =1;    
    
    *out_fn_addr = mid_fn_addr;

    ppa_rcu_read_unlock(); 
}

int32_t ppa_unreg_export_fn(PPA_EXPORT_FN_NO fn_no, uint32_t *out_fn_addr)
{
    if(fn_no >= PPA_HOOK_FN_MAX){
        err("Error: function reg no is bigger than max Number !!!\n");
        return PPA_FAILURE;
    }

    if( g_expfn_table[fn_no].used_flag ) {
      
      ppa_rcu_read_lock(); 
      
      g_expfn_table[fn_no].hook_flag = 0;
      g_expfn_table[fn_no].used_flag = 0;    
      g_expfn_table[fn_no].hook_addr = NULL;
      *out_fn_addr = NULL;
      
      ppa_rcu_read_unlock(); 
    } 
    return PPA_SUCCESS;
}


/*
  * must be called at the module unload
*/
void ppa_export_fn_manager_exit(void)
{
    int i;
     
    for(i=0; i<PPA_HOOK_FN_MAX; i++) {
        g_expfn_table[i].used_flag = 0;
        g_expfn_table[i].hook_flag = 0;
        /*don't set out_fn_addr to NULL in case netfilter get NULL hook during setting to NULL */
        g_expfn_table[i].hook_addr = 0;
    }
}

int32_t get_ppa_hook_list_count(void)
{
    if( !ppa_is_init() ) return 0;
    return PPA_HOOK_FN_MAX;
}

static 
int32_t ppa_enable_hook( int8_t *name, uint32_t enable, uint32_t flag)
{
    int i;
    int32_t res = PPA_FAILURE;

    for(i=0; i<PPA_HOOK_FN_MAX; i++)
    {
        if( !g_expfn_table[i].used_flag ) continue;
        if( strcmp( g_expfn_table[i].hookname, name ) == 0 )
        {
            if( enable )
            {
                g_expfn_table[i].hook_flag = 1;
            }
            else
            {
                g_expfn_table[i].hook_flag = 0;
            }

            res = PPA_SUCCESS;
            break;
        }        
    }
  
    return res;
}


int32_t ppa_ioctl_get_hook_list(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    uint32_t i=0;
    int32_t res=PPA_FAILURE;

    if( !ppa_is_init() ) goto GET_HOOK_EXIT;
    if ( ppa_copy_from_user( &cmd_info->hook_list_info, (void *)arg, sizeof(cmd_info->hook_list_info)) != 0 )
        goto GET_HOOK_EXIT;

 
    if( !cmd_info->hook_list_info.hook_count  ) 
    {
        res = PPA_SUCCESS;
        goto GET_HOOK_EXIT;
    }

    if( cmd_info->hook_list_info.hook_count > PPA_HOOK_FN_MAX ) 
        cmd_info->hook_list_info.hook_count = PPA_HOOK_FN_MAX;
   
    for(i=0; i<cmd_info->hook_list_info.hook_count; i++)
    {        
        if ( ppa_copy_to_user( (void *)&(((PPA_CMD_HOOK_LIST_INFO*)arg)->list[i]), &g_expfn_table[i], sizeof(PPA_HOOK_INFO)) != 0 )
        {
            err("ppa_copy_to_user fail\n");
            goto GET_HOOK_EXIT;
        }          
    }
    res = PPA_SUCCESS;

GET_HOOK_EXIT:
    cmd_info->hook_list_info.hook_count = i;    
    return res;
}

int32_t ppa_ioctl_set_hook(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */

    if ( ppa_copy_from_user( &cmd_info->hook_control_info, (void *)arg, sizeof(cmd_info->hook_control_info)) != 0 )
        return PPA_FAILURE;

    return ppa_enable_hook( cmd_info->hook_control_info.hookname, cmd_info->hook_control_info.enable, cmd_info->hook_control_info.flag );
}

/*** HOOK LIST MAPPING ------------------------------------end */

/*** Memory read/write ------------------------------------begin */
/**
* \brief directly read memory address with 4 bytes alignment.
* \param  reg_addr memory address ( it must be 4 bytes alignment )
* \param  shift to the expected bits ( its value is from 0 ~ 31)
* \param  size the bits number ( its value is from 1 ~ 32 ). Note: shift + size <= 32
* \param  repeat repeat time to write the memory.
* \param  new value
* \return on Success return PPA_SUCCESS
*/
static
int32_t ppa_api_mem_read(uint32_t reg_addr, uint32_t shift, uint32_t size, uint32_t *buffer)
{
    volatile uint32_t value;
    uint32_t mask=0;

   //read data from specified address
    value = *(uint32_t *)reg_addr;

    //prepare the mask
    if( size > 32 )
        return PPA_FAILURE;
    else if( size == 32 )
        mask=-1;
    else
        mask = ( 1 << size ) - 1 ;

    /* Bit shifting to the exract bit field */
    value = (value >> shift);

    *buffer = (value & mask);
    return PPA_SUCCESS;
}

/**
* \brief directly write memory address with
* \param  reg_addr memory address ( it must be 4 bytes alignment )
* \param  shift to the expected bits ( its value is from 0 ~ 31)
* \param  size the bits number ( its value is from 1 ~ 32 )
* \param  repeat repeat time to write the memory.
* \param  new value
* \return on Success return PPA_SUCCESS
*/
static
int32_t ppa_api_mem_write(uint32_t reg_addr, uint32_t shift, uint32_t size, uint32_t value)
{
    volatile uint32_t orgi_value;
    uint32_t mask=0;

    /* Read the Whole 32bit register */
    orgi_value = *(uint32_t *)reg_addr;

    /* Prepare the mask	*/
     if( size > 32 )
        return PPA_FAILURE;
    else if( size == 32 )
        mask=-1;
    else
        mask = ( 1 << size ) - 1 ;
    mask = (mask << shift);

    /* shift the value to the right place and mask the rest of the bit*/
    value = ( value << shift ) & mask;

    /*  Mask out the bit field from the read register and place in the new value */
    value = ( orgi_value & ~mask ) | value ;

    /* Write into register */
    *(uint32_t *)reg_addr = value ;

    return PPA_SUCCESS;
}

int32_t ppa_ioctl_read_mem(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    uint32_t value, i;
    PPA_FPI_ADDR fpi_addr={0};
    
    if ( ppa_copy_from_user( &cmd_info->read_mem_info, (void *)arg, sizeof(cmd_info->read_mem_info)) != 0 )
        return PPA_FAILURE;

    if( cmd_info->read_mem_info.repeat == 0 ) return PPA_FAILURE;

    fpi_addr.addr_orig = cmd_info->read_mem_info.addr;
    if( ppa_drv_dp_sb_addr_to_fpi_addr_convert(&fpi_addr, 0) != 0 ) 
    { 
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_drv_dp_sb_addr_to_fpi_addr_convert failure\n");        
        return PPA_FAILURE;
    }
    cmd_info->read_mem_info.addr_mapped= fpi_addr.addr_fpi;
    if( cmd_info->read_mem_info.addr_mapped % 4 != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "0x%x_%d not aligned to 4 bytes\n", cmd_info->read_mem_info.addr, cmd_info->read_mem_info.addr_mapped);
        return PPA_FAILURE;
    }
    if ( ppa_copy_to_user( (void *)&(((PPA_CMD_READ_MEM_INFO*)arg)->addr_mapped), &cmd_info->read_mem_info.addr_mapped, sizeof(cmd_info->read_mem_info.addr_mapped)) != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_copy_to_user failed 1 \n");
        return PPA_FAILURE;
    }

    for(i=0; i<cmd_info->read_mem_info.repeat;  i++ )
    {
        value = 0;
        if ( ppa_api_mem_read( cmd_info->read_mem_info.addr_mapped + i * 4, cmd_info->read_mem_info.shift, cmd_info->read_mem_info.size, &value ) == PPA_FAILURE )
        {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_api_mem_read failed\n");
            return PPA_FAILURE;
        }
        if ( ppa_copy_to_user( (void *)&(((PPA_CMD_READ_MEM_INFO*)arg)->buffer[i]), &value, sizeof(value)) != 0 )
        {
            ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_copy_to_user failed 2\n");
            return PPA_FAILURE;
        }
    }

    return PPA_SUCCESS;
}

int32_t ppa_ioctl_set_mem(unsigned int cmd, unsigned long arg, PPA_CMD_DATA * cmd_info)
{ /*note, arg is a pointer from ioctl, not normally pointer  */
    uint32_t i;
    PPA_FPI_ADDR fpi_addr={0};

    if ( ppa_copy_from_user( &cmd_info->set_mem_info, (void *)arg, sizeof(cmd_info->set_mem_info)) != 0 )
        return PPA_FAILURE;

    fpi_addr.addr_orig = cmd_info->set_mem_info.addr;
    if( ppa_drv_dp_sb_addr_to_fpi_addr_convert(&fpi_addr, 0) != 0) 
    {        
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_drv_dp_sb_addr_to_fpi_addr_convert failure\n");        
        return PPA_FAILURE;
    }
    cmd_info->set_mem_info.addr_mapped= fpi_addr.addr_fpi;
    if( cmd_info->set_mem_info.addr_mapped % 4 != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "0x%x_%d not aligned to 4 bytes\n", cmd_info->set_mem_info.addr_mapped, cmd_info->set_mem_info.addr_mapped);
        return PPA_FAILURE;
    }

    if( cmd_info->set_mem_info.repeat == 0 ) return PPA_SUCCESS;
    for(i=0; i<cmd_info->set_mem_info.repeat;  i++ )
    {
        if( ppa_api_mem_write( cmd_info->set_mem_info.addr_mapped + i * 4, cmd_info->set_mem_info.shift, cmd_info->set_mem_info.size, cmd_info->set_mem_info.value ) == PPA_FAILURE )
            return PPA_FAILURE;
    }
     if ( ppa_copy_to_user( (void *)&(((PPA_CMD_SET_MEM_INFO*)arg)->addr_mapped), &cmd_info->set_mem_info.addr_mapped, sizeof(cmd_info->set_mem_info.addr_mapped)) != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_copy_to_user failed \n");
        return PPA_FAILURE;
    }


    return PPA_SUCCESS;
}


int32_t ppa_ioctl_dbg_tool_test(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{    
    int res = PPA_SUCCESS;
    
   if ( ppa_copy_from_user( &cmd_info->dbg_tool_info, (void *)arg, sizeof(cmd_info->dbg_tool_info)) != 0 )
        return PPA_FAILURE;

    if( cmd_info->dbg_tool_info.mode == PPA_CMD_DBG_TOOL_LOW_MEM_TEST )
    {  /*currently once the memory is allocated, there is no way to free it any more */
        uint8_t *pBuf = NULL;
        uint32_t  left_size = cmd_info->dbg_tool_info.value.size * 1000; 
        uint32_t  blocksize = left_size; 
        #define MIN_DBG_TOOL_TEST_BLK_SIZE  10000  /*bytes */
        #define MAX_DBG_TOOL_TEST_BLK_SIZE  1000000 /*bytes */

        if( blocksize > MAX_DBG_TOOL_TEST_BLK_SIZE )
            blocksize = MAX_DBG_TOOL_TEST_BLK_SIZE;
          
        while( left_size )
        {  
            pBuf = ppa_malloc(blocksize );
            if( pBuf ) 
            {
                left_size -= blocksize;
                if( blocksize > left_size )
                    blocksize = left_size; //make sure blocksize always less than left_size, then left size will not be negative since it is uint32_t
            }
            else
            { /*fail then half the block size to try */
                blocksize /= 10;
                if( blocksize <= MIN_DBG_TOOL_TEST_BLK_SIZE ) //memory block is too small, stop allocating anyone
                {
                    break;                   
                }              
            }
            
        }
        
        cmd_info->dbg_tool_info.value.size -= left_size/1000; //change to Kbytes and return the size PPA have succeed to allocate
    }

    if ( ppa_copy_to_user( (void *) arg, &cmd_info->dbg_tool_info, sizeof(cmd_info->dbg_tool_info)) != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_copy_to_user failed \n");
        return PPA_FAILURE;
    }
    
    return res;

}

int32_t ppa_ioctl_set_value(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{    
    int res = PPA_SUCCESS;
    
    if ( ppa_copy_from_user( (void *) &cmd_info->var_value_info, (void *)arg, sizeof(cmd_info->var_value_info)) != 0 )
        return PPA_FAILURE;
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
	if( cmd_info->var_value_info.id == PPA_VARIABLE_PPA_DIRECTPATH_IMQ_FLAG)
	{
		ppa_directpath_imq_en_flag = cmd_info->var_value_info.value;
		res = 0;
		goto EXIT;
	}
#endif
    if( cmd_info->var_value_info.id == PPA_LAN_SEPARATE_FLAG)
	{
		ppa_hook_set_lan_seperate_flag(cmd_info->var_value_info.value);
		res = 0;
		goto EXIT;
	}
    else if( cmd_info->var_value_info.id == PPA_WAN_SEPARATE_FLAG)
	{
		ppa_hook_set_wan_seperate_flag(cmd_info->var_value_info.value);
		res = 0;
		goto EXIT;
	}
    res = ppa_drv_set_value( &cmd_info->var_value_info, 0 );

    if ( ppa_copy_to_user( (void *) arg, &cmd_info->var_value_info, sizeof(cmd_info->var_value_info)) != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_copy_to_user failed \n");
        return PPA_FAILURE;
    }

EXIT:    
    return res;

}

int32_t ppa_ioctl_get_value(unsigned int cmd, unsigned long arg, PPA_CMD_DATA *cmd_info)
{    
    int res = PPA_SUCCESS;
    
   if ( ppa_copy_from_user( (void *) &cmd_info->var_value_info, (void *)arg, sizeof(cmd_info->var_value_info)) != 0 )
        return PPA_FAILURE;
#if defined(CONFIG_LTQ_PPA_DIRECTPATH_TX_IMQ)
	if( cmd_info->var_value_info.id == PPA_VARIABLE_PPA_DIRECTPATH_IMQ_FLAG )
    {
       cmd_info->var_value_info.value = ppa_directpath_imq_en_flag;
       res = 0;
	   goto EXIT;
    }
#endif
    if( cmd_info->var_value_info.id == PPA_LAN_SEPARATE_FLAG)
	{
		cmd_info->var_value_info.value=ppa_hook_get_lan_seperate_flag(0);
		res = 0;
		goto EXIT;
	}
    else if( cmd_info->var_value_info.id == PPA_WAN_SEPARATE_FLAG)
	{
		cmd_info->var_value_info.value=ppa_hook_get_wan_seperate_flag(0);
		res = 0;
		goto EXIT;
	}
    res = ppa_drv_get_value( &cmd_info->var_value_info, 0 );

EXIT:
    if ( ppa_copy_to_user( (void *) arg, &cmd_info->var_value_info, sizeof(cmd_info->var_value_info)) != 0 )
    {
        ppa_debug(DBG_ENABLE_MASK_DEBUG_PRINT, "ppa_copy_to_user failed \n");
        return PPA_FAILURE;
    }
    
    return res;
}

/*** Memory read/write -----------------------------------end */

EXPORT_SYMBOL(get_ppa_hook_list_count);
//EXPORT_SYMBOL(ppa_enable_hook);
EXPORT_SYMBOL(ppa_ioctl_get_hook_list);
EXPORT_SYMBOL(ppa_ioctl_set_hook);
//EXPORT_SYMBOL(ppa_api_mem_read);
//EXPORT_SYMBOL(ppa_api_mem_write);
EXPORT_SYMBOL(ppa_ioctl_read_mem);
EXPORT_SYMBOL(ppa_ioctl_set_mem);
EXPORT_SYMBOL(ppa_is_init);
EXPORT_SYMBOL(ppa_set_init_status);
EXPORT_SYMBOL(ppa_reg_export_fn);
EXPORT_SYMBOL(ppa_unreg_export_fn);
EXPORT_SYMBOL(ppa_export_fn_manager_exit);
EXPORT_SYMBOL(ppa_ioctl_dbg_tool_test);
EXPORT_SYMBOL(ppa_ioctl_set_value );
EXPORT_SYMBOL(ppa_ioctl_get_value );

