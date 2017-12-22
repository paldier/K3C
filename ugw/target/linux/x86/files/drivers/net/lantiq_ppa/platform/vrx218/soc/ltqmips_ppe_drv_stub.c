#include <linux/kernel.h>
#include <linux/module.h>

#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>


/*
 * ####################################
 *             Declaration
 * ####################################
 */
/*
 *  External Functions
 */
extern int __init ppe_hal_init(void);
extern void __exit ppe_hal_exit(void);
extern int __init ppe_datapath_init(void);
extern void __exit ppe_datapath_exit(void);


/*
 * ####################################
 *           Global Variable
 * ####################################
 */
ppe_generic_hook_t ifx_ppe_ext_datapath_generic_hook = NULL;
ppe_generic_hook_t ifx_ppe_ext_hal_generic_hook = NULL;


/*
 * ####################################
 *           Global Function
 * ####################################
 */
int32_t register_ppe_ext_generic_hook(ppe_generic_hook_t datapath_hook, ppe_generic_hook_t hal_hook)
{
    ifx_ppe_ext_datapath_generic_hook = datapath_hook;
    ifx_ppe_ext_hal_generic_hook = hal_hook;
    return PPA_SUCCESS;
}
EXPORT_SYMBOL(register_ppe_ext_generic_hook);


/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */
static int __init ppe_drv_init(void)
{
    int ret;

    ret = ppe_datapath_init();
    if ( ret != 0 )
        return ret;

    ret = ppe_hal_init();
    if ( ret != 0 )
        ppe_datapath_exit();

    return ret;
}

static void __exit ppe_drv_exit(void)
{
    ppe_hal_exit();

    ppe_datapath_exit();
}

module_init(ppe_drv_init);
module_exit(ppe_drv_exit);

MODULE_LICENSE("GPL");

