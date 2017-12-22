/*******************************************************************************
**
** FILE NAME    : ppa_api_tmplbuf_module.c
** PROJECT      : PPA - Tempalte buffer 
** MODULES      : PPA - Tempalte buffer module
**
** DATE         : 25 June 2015
** AUTHOR       : Lantiq
** DESCRIPTION  : WRAPPER FUNCTIONS TO MAKE SOFTWARE ACCELERATION A LODABLE 
**   		  KERNEL MODULE. ALL CODE AVAILABLE IN SOFTWARE ACCELERATION 
**		  MODULE IS LANTIQ PROPRIETARY AND NON GPL.
**
** COPYRIGHT    :              Copyright (c) 2013
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author                $Comment
*******************************************************************************/

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
#include <linux/netdevice.h>
#include <linux/atmdev.h>
#include <net/sock.h>

int32_t ppa_tmplbuf_register_hooks(void);
void ppa_tmplbuf_unregister_hooks(void);

static int __init ppa_tmplbuf_init(void)
{
	return ppa_tmplbuf_register_hooks();
}

static void __exit ppa_tmplbuf_exit(void)
{
  ppa_tmplbuf_unregister_hooks();
}

module_init(ppa_tmplbuf_init);
module_exit(ppa_tmplbuf_exit);

MODULE_LICENSE("GPL");
