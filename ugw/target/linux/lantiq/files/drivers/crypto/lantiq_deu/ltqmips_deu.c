/******************************************************************************
**
** FILE NAME	: ltqmips_deu.c
** PROJECT		: LTQ UEIP
** MODULES		: DEU Module for Danube
**
** DATE			: September 8, 2009
** AUTHOR		: Mohammad Firdaus
** DESCRIPTION	: Data Encryption Unit Driver
** COPYRIGHT	:		Copyright (c) 2009
**						Infineon Technologies AG
**						Am Campeon 1-12, 85579 Neubiberg, Germany
**
**	  This program is free software; you can redistribute it and/or modify
**	  it under the terms of the GNU General Public License as published by
**	  the Free Software Foundation; either version 2 of the License, or
**	  (at your option) any later version.
**
** HISTORY
** $Date		$Author				$Comment
** 08,Sept 2009 Mohammad Firdaus	Initial UEIP release
*******************************************************************************/

/*!
  \defgroup LTQ_DEU LTQ_DEU_DRIVERS
  \ingroup API
  \brief ltq deu driver module
*/

/*!
  \file ltqmips_deu.c
  \ingroup LTQ_DEU
  \brief main deu driver file
*/

/*!
 \defgroup LTQ_DEU_FUNCTIONS LTQ_DEU_FUNCTIONS
 \ingroup LTQ_DEU
 \brief LTQ DEU functions
*/

/* Project header */
#include <asm/byteorder.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/crypto.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>		/* Stuff about file systems that we need */
#include <linux/cpufreq.h>
#include <cpufreq/ltq_cpufreq.h>
#include <cpufreq/ltq_cpufreq_pmcu_compatible.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <irq.h>
#include <lantiq_dma.h>
#include <lantiq_soc.h>
#include "ltqmips_deu.h"
#include "ltqmips_deu_dma.h"

#define LTQ_DEU_DRV_VERSION			"2.0.0"
#define MAX_NAME_MASK 40

struct ltq_deu_config algo_config;
static int init_dma;
static u32 deu_power_flag;
spinlock_t pwr_lock;
spinlock_t global_deu_lock;
void aes_chip_init(void);
void des_chip_init(void);

u8 *g_dma_page_ptr;
u8 *g_dma_block;
u8 *g_dma_block2;
deu_drv_priv_t deu_dma_priv;
_ifx_deu_device ifx_deu[1];

extern struct deu_proc ltq_deu_algo[];
struct proc_dir_entry *g_deu_proc_dir;

void __iomem *ltq_deu_membase;

#if defined(CONFIG_CRYPTO_LTQ_ASYNC_MODE)
int ltq_des_init(void) { return 0; };
int ltq_aes_init(void) { return 0; };
void ltq_aes_fini(void) {};
void ltq_des_fini(void) {};
#else
int ltq_async_des_init(void) { return 0; };
int ltq_async_aes_init(void) { return 0; };
void ltq_async_aes_fini(void) {};
void ltq_async_des_fini(void) {};
#endif /* CONFIG_CRYPTO_LTQ_ASYNC_MODE */


#ifdef CONFIG_CPU_FREQ
void ltq_deu_pmcu_init(void);
void power_on_deu_module(char *algo_name);
void power_off_deu_module(char *algo_name);
void powerup_deu(int crypto);
void powerdown_deu(int crypto);

/* Linux CPUFREQ support end */
/* CoC flag, 1: CoC is turned on. 0: CoC is turned off. */
static int deu_power_status;

/* Linux CPUFREQ support start */
static IFX_PMCU_RETURN_t ifx_deu_stateGet(IFX_PMCU_STATE_t *pmcuModState);
static IFX_PMCU_RETURN_t ifx_deu_postChange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState);
static IFX_PMCU_RETURN_t ifx_deu_preChange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState);
static IFX_PMCU_RETURN_t ifx_deu_stateChange(IFX_PMCU_STATE_t newState);
static IFX_PMCU_RETURN_t ifx_deu_pwrFeatureSwitch(IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna);

extern struct list_head ltq_cpufreq_head_mod_list_g;

struct ltq_cpufreq_module_info ifx_deu_cpufreq_mod_g = {
	.name				= "DEU clock gating support",
	.pmcuModule			= IFX_PMCU_MODULE_DEU,
	.pmcuModuleNr			= 0,
	.powerFeatureStat		= IFX_PMCU_PWR_STATE_ON,
	.ltq_cpufreq_state_get		= ifx_deu_stateGet,
	.ltq_cpufreq_pwr_feature_switch	= ifx_deu_pwrFeatureSwitch,
};

static int ifx_deu_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data);
	static struct notifier_block ifx_deu_cpufreq_notifier_block = {
	.notifier_call	= ifx_deu_cpufreq_notifier
};

/* keep track of frequency transitions */
static int
ifx_deu_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	IFX_PMCU_STATE_t new_State, old_State;
	IFX_PMCU_RETURN_t ret;

	new_State = ltq_cpufreq_get_ps_from_khz(freq->new);
	if (new_State == IFX_PMCU_STATE_INVALID) {
		return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_DEU<<4);
	}
	old_State = ltq_cpufreq_get_ps_from_khz(freq->old);
	if (old_State == IFX_PMCU_STATE_INVALID) {
		return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_DEU<<4);
	}
	if (val == CPUFREQ_PRECHANGE) {
		ret = ifx_deu_preChange(IFX_PMCU_MODULE_DEU, new_State, old_State);
		if (ret == IFX_PMCU_RETURN_DENIED) {
			return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_DEU<<4);
		}
		ret = ifx_deu_stateChange(new_State);
		if (ret == IFX_PMCU_RETURN_DENIED) {
			return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_DEU<<4);
		}
	} else if (val == CPUFREQ_POSTCHANGE) {
		ret = ifx_deu_postChange(IFX_PMCU_MODULE_DEU, new_State, old_State);
		if (ret == IFX_PMCU_RETURN_DENIED) {
			return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_DEU<<4);
		}
	} else {
		return NOTIFY_OK | (IFX_PMCU_MODULE_DEU<<4);
	}
	return NOTIFY_OK | (IFX_PMCU_MODULE_DEU<<4);
}
#endif /* CONFIG_CPU_FREQ */

#if 0
/* fix me proc filesystem */
/*! \fn static ifx_deu_write_proc(struct file *file, const char *buf,
 *								  unsigned long count, void *data)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief proc interface write function
 *	\return buffer length
*/

static int ifx_deu_write_proc(struct file *file, const char *buf,
										 unsigned long count, void *data)
{
	int i;
	int add;
	int pos = 0;
	int done = 0;
	const char *x;
	char *mask_name;
	char substring[MAX_NAME_MASK];

	while (!done && (pos < count)) {
		done = 1;
	while ((pos < count) && isspace(buf[pos]))
		pos++;

	switch (buf[pos]) {
	case '+':
	case '-':
	case '=':
		add = buf[pos];
		pos++;
		break;

	default:
		add = ' ';
		break;
	}

	mask_name = NULL;

	for (x = buf + pos, i = 0;
		(*x == '_' || (*x >= 'a' && *x <= 'z') || (*x >= '0' && *x <= '9')) &&
		 i < MAX_NAME_MASK; x++, i++, pos++)
		substring[i] = *x;

		substring[i] = '\0';
		printk("substring: %s\n", substring);

		for (i = 0; i < MAX_DEU_ALGO; i++) {
			if (strcmp(substring, ltq_deu_algo[i].name) == 0) {
				done = 0;
				mask_name = ltq_deu_algo[i].name;
				printk("mask name: %s\n", mask_name);
				break;
			}
		}

		if (mask_name != NULL) {
			switch (add) {
			case '+':
				if (ltq_deu_algo[i].deu_status == 0) {
					ltq_deu_algo[i].deu_status = 1;
					ltq_deu_algo[i].toggle_algo(0);
				}
				break;
			case '-':
				if (ltq_deu_algo[i].deu_status == 1) {
					ltq_deu_algo[i].deu_status = 0;
					ltq_deu_algo[i].toggle_algo(1);
				}
				break;
			default:
				break;
			}
		}
	}

	return count;

}

/*! \fn static	int ifx_deu_read_proc(char *page,char **start,
 *						   off_t offset, int count, int *eof, void *data)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief proc interface read function
 *	\return buffer length
*/

static int ifx_deu_read_proc(char *page,
						   char **start,
						   off_t offset, int count, int *eof, void *data)
{
	int i, ttl_len = 0, len = 0;
	char str[1024];
	char *pstr;

	pstr = *start = page;

	for (i = 0; i < MAX_DEU_ALGO; i++) {
		len = sprintf(str, "DEU algo name: %s, registered: %s\n", ltq_deu_algo[i].name,
			   (ltq_deu_algo[i].deu_status ? "YES" : "NO"));

		if (ttl_len <= offset && len + ttl_len > offset) {
			memcpy(pstr, str + offset - ttl_len, len + ttl_len - offset);
			pstr += ttl_len + len - offset;
		} else if (ttl_len > offset) {
			memcpy(pstr, str, len);
			pstr += len;
		}

		ttl_len += len;

		if (ttl_len >= (offset + count))
			goto PROC_READ_OVERRUN;
	}

	*eof = 1;

	 return ttl_len - offset;

PROC_READ_OVERRUN:
	 return ttl_len - len - offset;

}

/*! \fn static	int ifx_deu_proc_create(void)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief proc interface initialization
 *	\return 0 if success, -ENOMEM if fails
*/

static int ifx_deu_proc_create(void)
{
	g_deu_proc_dir = create_proc_entry("driver/ifx_deu", 0775,	NULL);
	if (g_deu_proc_dir == NULL)
		return -ENOMEM;

	g_deu_proc_dir->write_proc = ifx_deu_write_proc;
	g_deu_proc_dir->read_proc = ifx_deu_read_proc;
	g_deu_proc_dir->data = NULL;

	return 0;
}

#endif /* fixme */

#if defined (CONFIG_CPU_FREQ) || defined (CONFIG_LTQ_CPU_FREQ)
static IFX_PMCU_RETURN_t ifx_deu_preChange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
{
	return IFX_PMCU_RETURN_SUCCESS;
}

static IFX_PMCU_RETURN_t ifx_deu_postChange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
{
	return IFX_PMCU_RETURN_SUCCESS;
}

static IFX_PMCU_RETURN_t ifx_deu_stateChange(IFX_PMCU_STATE_t newState)
{
	return IFX_PMCU_RETURN_SUCCESS;
}

/*! \fn static IFX_PMCU_RETURN_t ifx_deu_stateGet(IFX_PMCU_STATE_t *pmcuModState)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief provide the current state of the DEU hardware
 *	\return IFX_PMCU_RETURN_SUCCESS
*/

static IFX_PMCU_RETURN_t ifx_deu_stateGet(IFX_PMCU_STATE_t *pmcuModState)
{

	if (deu_power_status == 0)
		*pmcuModState = IFX_PMCU_STATE_INVALID;

	if (deu_power_status == 1) {
		if (deu_power_flag == 0)
			*pmcuModState = IFX_PMCU_STATE_D3;
		else
			*pmcuModState = IFX_PMCU_STATE_D0;
	}

	return IFX_PMCU_RETURN_SUCCESS;
}

/*! \fn static IFX_PMCU_RETURN_t ifx_deu_pwrFeatureSwitch(IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief switches the PMCU state to ON or OFF state
 *	\return IFX_PMCU_RETURN_SUCCESS
*/

static IFX_PMCU_RETURN_t ifx_deu_pwrFeatureSwitch(IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna)
{
	int i;

	/* 1. When there is a PMCU request to switch on, we need to power off the algos which are idling.
	 *	  This is done by toggling the deu_power_flag.
	 * 2. When we are toggling to switch on/off, we must make sure that no ALGO is currenly running.
	 *	  Due to the H/W design of DEU, we can only switch the whole DEU module on/off. Hence, we must
	 *	  make sure that all DEU algo are in idle state before switching it off.
	 * 3. When a PMCU request to switch PMCU off, we just ignore all the flags and switch on all the
	 *	  DEU algo flags.
	 */

	if (pmcuPwrStateEna == IFX_PMCU_PWR_STATE_ON) {
		if (deu_power_status == 1)
			return IFX_PMCU_RETURN_SUCCESS;

		deu_power_status = 1;

		for (i = 0; i < MAX_DEU_ALGO; i++) {
			if (ltq_deu_algo[i].get_deu_algo_state() == CRYPTO_IDLE) {
				/* check power flag, if ON, switch off */
				power_off_deu_module(ltq_deu_algo[i].name);
			}
		}
	} else if (pmcuPwrStateEna == IFX_PMCU_PWR_STATE_OFF) {
		if (deu_power_status == 0)
			return IFX_PMCU_RETURN_SUCCESS;

		for (i = 0; i < MAX_DEU_ALGO; i++) {
			if (ltq_deu_algo[i].get_deu_algo_state() == CRYPTO_IDLE) {
				/* if idle, switch on the algo */
			   power_on_deu_module(ltq_deu_algo[i].name);
			}
		}

		deu_power_status = 0;
	} else
		pr_err("Unknown power state feature command!\n");

	return IFX_PMCU_RETURN_SUCCESS;
}


/*! \fn void ltq_deu_pmcu_init (void)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief	PMCU initialization function
 *	\return void
*/

void ltq_deu_pmcu_init(void)
{
	{
	struct ltq_cpufreq* lqdeu_cpufreq_p;
	cpufreq_register_notifier(&ifx_deu_cpufreq_notifier_block, 
                                  CPUFREQ_TRANSITION_NOTIFIER);
    	lqdeu_cpufreq_p = ltq_cpufreq_get();
	list_add_tail(&ifx_deu_cpufreq_mod_g.list, &lqdeu_cpufreq_p->list_head_module);
	}
		
	deu_power_status = 1;
}

void power_on_deu_module(char *algo_name)
{
	unsigned long flag;

	CRTCL_SECT_START;

	if (strcmp(algo_name, "aes") == 0) {
		powerup_deu(AES_INIT);
	} else if (strcmp(algo_name, "des") == 0) {
		powerup_deu(DES_INIT);
	} else if (strcmp(algo_name, "md5") == 0) {
		powerup_deu(MD5_INIT);
	} else if (strcmp(algo_name, "sha1") == 0) {
		powerup_deu(SHA1_INIT);
	} else if (strcmp(algo_name, "arc4") == 0) {
		powerup_deu(ARC4_INIT);
	} else if (strcmp(algo_name, "md5_hmac") == 0) {
		powerup_deu(MD5_HMAC_INIT);
	} else if (strcmp(algo_name, "sha1_hmac") == 0) {
		powerup_deu(SHA1_HMAC_INIT);
	}

	CRTCL_SECT_END;

	return;
}

void power_off_deu_module(char *algo_name)
{
	unsigned long flag;

	CRTCL_SECT_START;

	if (strcmp(algo_name, "aes") == 0) {
		powerdown_deu(AES_INIT);
	} else if (strcmp(algo_name, "des") == 0) {
		powerdown_deu(DES_INIT);
	} else if (strcmp(algo_name, "md5") == 0) {
		powerdown_deu(MD5_INIT);
	} else if (strcmp(algo_name, "sha1") == 0) {
		powerdown_deu(SHA1_INIT);
	} else if (strcmp(algo_name, "arc4") == 0) {
		powerdown_deu(ARC4_INIT);
	} else if (strcmp(algo_name, "md5_hmac") == 0) {
		powerdown_deu(MD5_HMAC_INIT);
	} else if (strcmp(algo_name, "sha1_hmac") == 0) {
		powerdown_deu(SHA1_HMAC_INIT);
	}
	CRTCL_SECT_END;

	return;
}

/*! \fn void powerup_deu(int crypto)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief to power up the chosen cryptography module
 *	\param crypto AES_INIT, DES_INIT, etc. are the cryptographic algorithms chosen
 *	\return void
*/

void powerup_deu(int crypto)
{

#ifndef CONFIG_CRYPTO_DEU_PERFORMANCE
	u32 temp;
	temp = 0;

	if ((!init_dma) && (algo_config.dma)) {
		init_dma = 1;
		START_DEU_POWER;
	}

	temp = 1 << crypto;

	switch (crypto) {
	case AES_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		aes_chip_init();
		break;

	case DES_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		des_chip_init();
		break;

	case SHA1_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		SHA_HASH_INIT;
		break;

	case MD5_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		md5_init_registers();
		break;

	case ARC4_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		break;

	case MD5_HMAC_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		break;

	case SHA1_HMAC_INIT:
		if (!(deu_power_flag & temp)) {
			START_DEU_POWER;
			deu_power_flag |= temp;
		}
		break;
	default:
		pr_err("Error finding initialization crypto\n");
		break;
	}
#endif /* CONFIG_CRYPTO_DEU_PERFORMANCE */
	
	return;

}

/*! \fn void powerdown_deu(int crypto)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief to power down the deu module totally if there are no other DEU encrypt/decrypt/hash
		   processes still in progress. If there is, do not power down the module.
 *	\param crypto defines the cryptographic algo to module to be switched off
 *	\return void
*/

void powerdown_deu(int crypto)
{
#ifndef CONFIG_CRYPTO_DEU_PERFORMANCE
	u32 temp;

	if (deu_power_status == 0) {
	   return;
	}

	temp = 0;

	temp = 1 << crypto;

	if (!(temp & deu_power_flag)) {
		return;
	}

	deu_power_flag &= ~temp;
	if (deu_power_flag) {
		pr_debug("DEU power down not permitted. DEU modules still in use! \n");
		return;
	}

	STOP_DEU_POWER;

#endif /* CONFIG_CRYPTO_DEU_PERFORMANCE */
	return;
}

struct deu_proc ltq_deu_algo[] = {
	{
		.name = "aes",
		.deu_status = 1,
		.toggle_algo = ltq_aes_toggle_algo,
		.get_deu_algo_state = read_aes_algo_status
	}, {
		.name = "des",
		.deu_status = 1,
		.toggle_algo = ltq_des_toggle_algo,
		.get_deu_algo_state = read_des_algo_status
	}, {
		.name = "md5",
		.deu_status = 1,
		.toggle_algo = ltq_md5_toggle_algo,
		.get_deu_algo_state = read_md5_algo_status
	}, {
		.name = "sha1",
		.deu_status = 1,
		.toggle_algo = ltq_sha1_toggle_algo,
		.get_deu_algo_state = read_sha1_algo_status
	}, {
		.name = "md5_hmac",
		.deu_status = 1,
		.toggle_algo = ltq_md5_hmac_toggle_algo,
		.get_deu_algo_state = read_md5_hmac_algo_status
	}, {
		.name = "sha1_hmac",
		.deu_status = 1,
		.toggle_algo = ltq_sha1_hmac_toggle_algo,
		.get_deu_algo_state = read_sha1_hmac_algo_status
	}, {
		.name = "arc4",
		.deu_status = 1,
		.toggle_algo = ltq_arc4_toggle_algo,
		.get_deu_algo_state = read_arc4_algo_status
	}
};

#endif /* CONFIG_CPU_FREQ || CONFIG_LTQ_CPU_FREQ */

/*! \fn u32 endian_swap(u32 input)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief Swap data given to the function
 *	\param input Data input to be swapped
 *	\return either the swapped data or the input data depending on whether it is in DMA mode or FPI mode
*/

u32 endian_swap(u32 input)
{
	if (algo_config.dma) {
		u8 *ptr = (u8 *)&input;
		return (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
	} else
		return input;
}

static void deu_dma_priv_init(void)
{
	DEU_WAKELIST_INIT(deu_dma_priv.deu_thread_wait);
	deu_dma_priv.deu_event_flags = 0;
	deu_dma_priv.des_event_flags = 0;
	deu_dma_priv.aes_event_flags = 0;
	deu_dma_priv.event_src = 0;
	deu_dma_priv.deu_rx_buf = NULL;
	deu_dma_priv.deu_rx_len = 0;
	deu_dma_priv.iv_arg = NULL;
	deu_dma_priv.mode = 0;
}

/*! \fn int deu_dma_init (void)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief Initialize DMA for DEU usage. DMA specific registers are
 *		   intialized here, including a pointer to the device, memory
 *		   space for the device and DEU-DMA descriptors
 *	\return -1 : fail to init, 0 : success
*/

static int deu_dma_init (struct platform_device *pdev)
{
	volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON;
	struct dma_device_info *dma_device = NULL;
	struct dma_device_info *deu_dma_device_ptr;
	const __be32 *sync_mode = of_get_property(pdev->dev.of_node,
								 "lantiq,sync-mode", NULL);
	int i = 0;

	/* get one free page and share between g_dma_block and g_dma_block2 */
	pr_info("DMA Referenced PAGE_SIZE = %ld\n", PAGE_SIZE);
	g_dma_page_ptr = (u8 *)__get_free_page(GFP_KERNEL); /* need 16-byte alignment memory block */
	g_dma_block = g_dma_page_ptr;
	g_dma_block2 = (u8 *)(g_dma_page_ptr + (PAGE_SIZE >> 1));

	deu_dma_device_ptr = dma_device_reserve ("DEU");
	if (!deu_dma_device_ptr) {
		pr_err("DEU: reserve DMA fail!\n");
		return -1;
	}
	ifx_deu[0].dma_device = deu_dma_device_ptr;

	dma_device = deu_dma_device_ptr;
	dma_device->priv = &deu_dma_priv;
	dma_device->buffer_alloc = &deu_dma_buffer_alloc;
	dma_device->buffer_free = &deu_dma_buffer_free;
	dma_device->intr_handler = &deu_dma_intr_handler;

	dma_device->tx_endianness_mode = IFX_DMA_ENDIAN_TYPE3;
	dma_device->rx_endianness_mode = IFX_DMA_ENDIAN_TYPE3;
	dma_device->port_num = 1;
	dma_device->tx_burst_len = 2;
	dma_device->rx_burst_len = 2;
	dma_device->max_rx_chan_num = 1;
	dma_device->max_tx_chan_num = 1;
	dma_device->port_packet_drop_enable = 0;

	for (i = 0; i < dma_device->max_rx_chan_num; i++) {
		dma_device->rx_chan[i]->packet_size = DEU_MAX_PACKET_SIZE;
		dma_device->rx_chan[i]->desc_len = 1;
		dma_device->rx_chan[i]->control = IFX_DMA_CH_ON;
		dma_device->rx_chan[i]->byte_offset = 0;
	}

	for (i = 0; i < dma_device->max_tx_chan_num; i++) {
		dma_device->tx_chan[i]->control = IFX_DMA_CH_ON;
		dma_device->tx_chan[i]->desc_len = 1;
	}

	dma_device->current_tx_chan = 0;
	dma_device->current_rx_chan = 0;

	i = dma_device_register (dma_device);

	for (i = 0; i < dma_device->max_rx_chan_num; i++) {
		(dma_device->rx_chan[i])->open (dma_device->rx_chan[i]);
		if (sync_mode && (*sync_mode == 1)) {
			(dma_device->rx_chan[i])->disable_irq(dma_device->rx_chan[i]);
		}
	}

	dma->controlr.BS = 0;
	dma->controlr.RXCLS = 0;
	dma->controlr.EN = 1;

	return 0;
}

/*! \fn __exit ltq_deu_dma_fini(void)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief unregister dma devices after exit
*/

void __exit ltq_deu_dma_fini(void)
{
	if (g_dma_page_ptr)
		free_page((u32) g_dma_page_ptr);
	dma_device_release(ifx_deu[0].dma_device);
	dma_device_unregister(ifx_deu[0].dma_device);
}

static int ltq_start_deu_power(struct platform_device *pdev)
{
	struct clk *clk;

	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to get clk\n");
		return PTR_ERR(clk);
	}

	clk_enable(clk);

	ltq_deu_w32(LTQ_DEU_CLC, 0x0);
	return 0;
}

/*! \fn static int __init ltq_deu_init (void)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief link all modules that have been selected in kernel config for ltq hw crypto support
 *	\return ret
*/

static int __init ltq_deu_init (struct platform_device *pdev)
{
	int ret = -ENOSYS;
	struct device_node *np = pdev->dev.of_node;
	struct resource *res;
	const __be32 *dma_mode = of_get_property(pdev->dev.of_node,
								 "lantiq,dma-mode", NULL);
	const __be32 *sync_mode = of_get_property(pdev->dev.of_node,
								 "lantiq,sync-mode", NULL);

	init_dma = 0;
	deu_power_flag = 0;

	pr_info("Lantiq DEU driver version %s \n", LTQ_DEU_DRV_VERSION);

	if ((sync_mode && (*sync_mode == 0)) &&
		(dma_mode && (*dma_mode == 0))) {
		pr_err("Device tree settings are not compatible\n");
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("Cannot get DEU resource\n");
		return -ENOMEM;
	}

	res = devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
			pdev->name);
	if (!res) {
		pr_err("failed to get memory region\n");
		return -ENXIO;
	}

	ltq_deu_membase = devm_ioremap_nocache(&pdev->dev, res->start,
						resource_size(res));
	if (!ltq_deu_membase) {
		pr_err("failed to remap memory\n");
		return -ENXIO;
	}

	memset(&algo_config, 0, sizeof(algo_config));
	ltq_deu_pmcu_init();

	/* if PMCU/CPU CLK gating is not enabled, we should enable the DEU
	 * drivers during init
	*/
	deu_power_flag = 0x0F;
	ret = ltq_start_deu_power(pdev);
	if (ret)
		return ret;

	CRTCL_SECT_INIT;

	/* Initialize the dma priv members
	 * have to do here otherwise the tests will fail
	 * caused by exceptions. Only needed in DMA mode.
	*/
	if (dma_mode && (*dma_mode == 1)) {
		deu_dma_priv_init();
		deu_dma_init(pdev);
		init_dma = 0;
		algo_config.dma = 1;
	}

	/* FIND_DEU_CHIP_VERSION; // need for danube --fixme */

	if (of_property_match_string(np, "lantiq,algo", "des") >= 0) {
		if (sync_mode && (*sync_mode == 0)) {
			ret = ltq_async_des_init();
			algo_config.des = 1;
			algo_config.async_mode = 1;
			if (ret)
				pr_err("Lantiq ASYNC-DES Initialization failed\n");
		} else {
			ret = ltq_des_init();
			algo_config.des = 1;
			algo_config.async_mode = 0;
			if (ret)
				pr_err("Lantiq DES Initialization failed\n");
		}
	}

	if (of_property_match_string(np, "lantiq,algo", "aes") >= 0) {
		if (sync_mode && (*sync_mode == 0)) {
			ret = ltq_async_aes_init();
			algo_config.aes = 1;
			algo_config.async_mode = 1;
			if (ret)
				pr_err("Lantiq ASYNC-AES Initialization failed\n");
		} else {
			ret = ltq_aes_init();
			algo_config.aes = 1;
			algo_config.async_mode = 0;
			if (ret)
				pr_err("Lantiq AES Initialization failed\n");
		}
	}

	if (of_property_match_string(np, "lantiq,algo", "arc4") >= 0) {
		algo_config.arc4 = 1;
		ret = ltq_arc4_init();
		if (ret) {
			pr_err("Lantiq ARC4 Initialization failed\n");
		}
	}

	if (of_property_match_string(np, "lantiq,algo", "sha1") >= 0) {
		algo_config.sha1 = 1;
		ret = ltq_sha1_init();
		if (ret) {
			pr_err("Lantiq SHA1 Initialization failed\n");
		}
	}

	if (of_property_match_string(np, "lantiq,algo", "md5") >= 0) {
		algo_config.md5 = 1;
		ret = ltq_md5_init();
		if (ret) {
			pr_err("Lantiq MD5 Initialization failed\n");
		}
	}

	if (of_property_match_string(np, "lantiq,algo", "sha1-hmac") >= 0) {
		algo_config.sha1_hmac = 1;
		ret = ltq_sha1_hmac_init();
		if (ret) {
			pr_err("Lantiq SHA1-HMAC Initialization failed\n");
		}
	}

	if (of_property_match_string(np, "lantiq,algo", "md5-hmac") >= 0) {
		algo_config.md5_hmac = 1;
		ret = ltq_md5_hmac_init();
		if (ret) {
			pr_err("Lantiq MD5-HMAC Initialization failed\n");
		}
	}

	/* ifx_deu_proc_create(); */

	pr_info("DEU driver initialization complete!\n");

	return ret;

}

/*! \fn static static int ltq_deu_fini (void)
 *	\ingroup LTQ_DEU_FUNCTIONS
 *	\brief remove the loaded crypto algorithms
*/
static int ltq_deu_fini (struct platform_device *pdev)
{
	if ((algo_config.des) && !(algo_config.async_mode))
		ltq_des_fini();
	if ((algo_config.aes) && !(algo_config.async_mode))
		ltq_aes_fini();
	if (algo_config.arc4)
		ltq_arc4_fini();
	if (algo_config.sha1)
		ltq_sha1_fini();
	if (algo_config.md5)
		ltq_md5_fini();
	if (algo_config.sha1_hmac)
		ltq_sha1_hmac_fini();
	if (algo_config.md5_hmac)
		ltq_md5_hmac_fini();

	if (algo_config.dma)
		ltq_deu_dma_fini();

	if (algo_config.async_mode) {
		ltq_async_aes_fini();
		ltq_async_des_fini();
	}

	pr_info("DEU has exited successfully\n");
	return 0;
}

static struct of_device_id ltq_deu_of_match_table[] = {
	{ .compatible = "lantiq,deu-lantiq", },
	{},
};

static struct platform_driver ltq_deu_driver = {
	.remove = ltq_deu_fini,
	.driver = {
		.owner = THIS_MODULE,
		.name = "ltq-deu-driver",
		.of_match_table = of_match_ptr(ltq_deu_of_match_table),
	},
};

module_platform_driver_probe(ltq_deu_driver, ltq_deu_init);
MODULE_DESCRIPTION ("Lantiq DEU crypto engine support.");
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Mohammad Firdaus");

