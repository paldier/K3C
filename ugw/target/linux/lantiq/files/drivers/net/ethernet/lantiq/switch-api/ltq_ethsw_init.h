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
/*#include <asm/delay.h> */
#include <linux/delay.h>
#include <linux/slab.h>

#define GSWIP_GET_BITS(x, msb, lsb)	\
		(((x) & ((1 << ((msb) + 1)) - 1)) >> (lsb))
#define GSWIP_SET_BITS(x, msb, lsb, value)	\
	(((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1)))	\
	| (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))
	
#define LTQ_GSW_DEV_MAX			2
#define GSW_API_MODULE_NAME		"LTQ ETH SWITCH API"
#define GSW_API_DRV_VERSION		"2.0.1"
#define MICRO_CODE_VERSION		"212"
#define GSWITCH_R32_ACCESS(addr)	(*((volatile int *)(addr)))

typedef struct {
	u8 minorNum;
	void *pCoreDev;
} IFX_ETHSW_coreHandle_t;

#include <xway/switch-api/lantiq_ethsw_api.h>
#include "ltq_ethsw_ioctl_wrapper.h"
#include "ltq_flow_core.h"
#include "ltq_ethsw_flow_ll.h"
#include "ltq_flow_reg_switch.h"
#include "ltq_flow_reg_top.h"

typedef struct {
	void *pDev;
	gsw_devType_t	eDev;
} ethsw_core_init_t;

void gsw_r32(short Offset, short Shift, short Size, u32 *value);
//void gsw_r32(void *pDevCtx, short Offset, short Shift, short Size, u32 *value);
void gsw_w32(short Offset, short Shift, short Size, u32 value);
//void gsw_w32(void *pDevCtx, short Offset, short Shift, short Size, u32 value);
void *ethsw_api_core_init(ethsw_core_init_t *pInit);
void gsw_CoreCleanUP(void);
ltq_bool_t	PHY_mediumDetectStatusGet(void *pDevCtx,	\
	unsigned char nPortID);
ethsw_ret_t	PHY_PDN_Set(void *pDevCtx, unsigned char PHYAD);
ethsw_ret_t	PHY_PDN_Clear(void *pDevCtx, unsigned char nPHYAD);
ltq_bool_t	PHY_Link_Status_Get(void *pDevCtx,	\
	unsigned char nPortID);
ethsw_ret_t	ethsw_pce_micro_code_init(void *pDevCtx);
#endif    /* _ETHSW_INIT_H_ */
