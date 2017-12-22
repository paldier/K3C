#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

#include "ppa_xrx200_e5_common.h"


#if !(defined(CONFIG_IFXMIPS_DSL_CPE_MEI) || defined(CONFIG_IFXMIPS_DSL_CPE_MEI_MODULE))
inline int ltq_mei_atm_showtime_check_local(int *is_showtime, struct port_cell_info *port_cell, void **xdata_addr) 
{ 
    if ( is_showtime != NULL ) 
        *is_showtime = 1; 

    return 0; 
} 
#endif 

struct ltq_mei_atm_showtime_info g_ltq_mei_atm_showtime = 

#if defined(CONFIG_IFXMIPS_DSL_CPE_MEI) || defined(CONFIG_IFXMIPS_DSL_CPE_MEI_MODULE) 
{NULL, NULL, NULL}; 
#else 
{NULL, NULL, (void *)ltq_mei_atm_showtime_check_local}; 
#endif 

int ppa_callback_set(e_ltq_mei_cb_type type, void *func) 
{ 
    int ret = 0; 
    if (func) 
    { 
        switch (type) 
        { 

            /* save func address within global struct */ 
            case LTQ_MEI_SHOWTIME_CHECK: 
                g_ltq_mei_atm_showtime.check_ptr = func; 
                break; 
            case LTQ_MEI_SHOWTIME_ENTER: 
                g_ltq_mei_atm_showtime.enter_ptr = func; 
                break; 
            case LTQ_MEI_SHOWTIME_EXIT: 
                g_ltq_mei_atm_showtime.exit_ptr = func; 
                break; 
            default: 
                //err("mei unknown function type"); 
                ret = -1; 
                break; 
        } 
    } 
    else 
    { 
        //err("could not register NULL pointer"); 
        ret = -1; 
    } 
    return ret; 
} 

void *ppa_callback_get(e_ltq_mei_cb_type type) 
{ 
    switch (type) 
    { 
        case LTQ_MEI_SHOWTIME_CHECK: 
            return g_ltq_mei_atm_showtime.check_ptr; 
        case LTQ_MEI_SHOWTIME_ENTER: 
            return g_ltq_mei_atm_showtime.enter_ptr; 
        case LTQ_MEI_SHOWTIME_EXIT: 
            return g_ltq_mei_atm_showtime.exit_ptr; 
        default: 
            //err("mei unknown function type"); 
            return NULL; 
    } 
} 

EXPORT_SYMBOL(ppa_callback_get); 
EXPORT_SYMBOL(ppa_callback_set); 
