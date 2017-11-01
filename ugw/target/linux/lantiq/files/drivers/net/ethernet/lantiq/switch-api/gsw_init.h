/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _ETHSW_INIT_H_
#define _ETHSW_INIT_H_
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/interrupt.h>
/*#include <asm/delay.h> */
#include <linux/delay.h>
#include <linux/slab.h>

#define GSWIP_GET_BITS(x, msb, lsb)	\
		(((x) & ((1 << ((msb) + 1)) - 1)) >> (lsb))
#define GSWIP_SET_BITS(x, msb, lsb, value)	\
	(((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1)))	\
	| (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))

#define LTQ_GSW_DEV_MAX 2
#define GSW_API_MODULE_NAME "GSW SWITCH API"
#define GSW_API_DRV_VERSION "3.0.2"
#define MICRO_CODE_VERSION "212"

#ifndef CONFIG_X86_INTEL_CE2700
#include <xway/switch-api/lantiq_gsw_api.h>
#else
#include <net/switch-api/lantiq_gsw_api.h>
#endif /* CONFIG_X86_INTEL_CE2700 */
/*#include <xway/switch-api/lantiq_gsw_routing.h>*/
/*#include <xway/switch-api/gsw_types.h>*/
#include "gsw_ioctl_wrapper.h"
#include "gsw_flow_core.h"
#include "gsw_ll_func.h"
#include "gsw_reg.h"
#include "gsw_reg_top.h"
#include "gsw30_reg_top.h"
#include "gsw_pae.h"

typedef struct {
	void *ecdev;
	gsw_devtype_t sdev;
	void __iomem *gsw_base_addr;
} ethsw_core_init_t;

void gsw_r32(void *cdev, short offset, short shift, short size, u32 *value);
void gsw_w32(void *cdev, short offset, short shift, short size, u32 value);
void *ethsw_api_core_init(ethsw_core_init_t *pinit);
void gsw_corecleanup(void);
int	gsw_pmicro_code_init(void *cdev);
#endif    /* _ETHSW_INIT_H_ */
