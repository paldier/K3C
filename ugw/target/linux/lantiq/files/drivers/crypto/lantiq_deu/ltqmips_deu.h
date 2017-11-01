/******************************************************************************
**
** FILE NAME	: ltqmips_deu.h
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
  \ingroup	API
  \brief ltq deu driver module
*/

/*!
  \file ltqmips_deu.h
  \brief main deu driver header file
*/

/*!
  \defgroup LTQ_DEU_DEFINITIONS LTQ_DEU_DEFINITIONS
  \ingroup	LTQ_DEU
  \brief ltq deu definitions
*/


#ifndef LTQMIPS_DEU_H
#define LTQMIPS_DEU_H

#include <crypto/algapi.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <lantiq.h>

#include "ltqmips_deu_dma.h"

#define IFXDEU_ALIGNMENT			16
#define IFXDEU_CRA_PRIORITY			300
#define IFXDEU_COMPOSITE_PRIORITY	400
#define IFX_PMU_ENABLE				0x1
#define IFX_PMU_DISABLE				0
#define CRYPTO_DIR_ENCRYPT			0x1
#define CRYPTO_DIR_DECRYPT			0
#define AES_IDLE					0
#define AES_BUSY					0x1
#define AES_STARTED					0x2
#define AES_COMPLETED				0x3
#define DES_IDLE					0
#define DES_BUSY					0x1
#define DES_STARTED					0x2
#define DES_COMPLETED				0x3
#define AES_INIT					0
#define DES_INIT					0x1
#define SHA1_INIT					0x2
#define MD5_INIT					0x3
#define SHA1_HMAC_INIT				0x4
#define MD5_HMAC_INIT				0x5
#define ARC4_INIT					0x6
#define PROCESS_SCATTER				0x1
#define PROCESS_NEW_PACKET			0x2
#define AES_ALGO					0x1
#define DES_ALGO					0x0

#define CRYPTO_STARTED				0x20
#define CRYPTO_IDLE					0x21

/* DEU Register definition */
#define LTQ_DEU_CLC			0x0
#define LTQ_DEU_ID			0x08
#define LTQ_DEU_DES_CON		0x10
#define LTQ_DEU_DES_IHR		0x14
#define LTQ_DEU_DES_ILR		0x18
#define LTQ_DEU_DES_K1HR	0x1c

#define LTQ_DEU_AES_CON		0x50
#define LTQ_DEU_HASH_CON	0xb0
#define LTQ_DEU_ARC4_CON	0x100
#define LTQ_DEU_DMA_CON		0xec

#define DEU_BASE_ADDR		0xBE103100
#define LTQ_DES_CON			(volatile u32*)(DEU_BASE_ADDR + LTQ_DEU_DES_CON)
#define LTQ_AES_CON			(volatile u32*)(DEU_BASE_ADDR + LTQ_DEU_AES_CON)
#define LTQ_ARC4_CON		(volatile u32*)(DEU_BASE_ADDR + LTQ_DEU_ARC4_CON)
#define LTQ_HASH_CON		(volatile u32*)(DEU_BASE_ADDR + LTQ_DEU_HASH_CON)
#define LTQ_DMA_CON			(volatile u32*)(DEU_BASE_ADDR + LTQ_DEU_DMA_CON)

//extern _ifx_deu_device ifx_deu[1];
#define DEU_DWORD_REORDERING(ptr, buffer, in_out, bytes)	  memory_alignment(ptr, buffer, in_out, bytes)

extern __iomem void *ltq_deu_membase;
#define ltq_deu_w32(x, y)	ltq_w32(x, ltq_deu_membase + (y))
#define ltq_deu_r32(x)		ltq_r32(ltq_deu_membase + (x))

static inline void ltq_stop_deu_power(void)
{
	ltq_deu_w32(LTQ_DEU_CLC, (ltq_deu_r32(LTQ_DEU_CLC) | 0x3f));
}

int __init ltq_arc4_init(void);
int __init ltq_sha1_init(void);
int __init ltq_md5_init(void);
int __init ltq_sha1_hmac_init(void);
int __init ltq_md5_hmac_init(void);

void __exit ltq_arc4_fini(void);
void __exit ltq_sha1_fini(void);
void __exit ltq_md5_fini(void);
void __exit ltq_sha1_hmac_fini(void);
void __exit ltq_md5_hmac_fini(void);
void __exit ltq_deu_dma_fini(void);

int ltq_async_des_init(void);
int ltq_async_aes_init(void);
void ltq_async_aes_fini(void);
void ltq_async_des_fini(void);
int ltq_des_init(void);
int ltq_aes_init(void);
void ltq_des_fini(void);
void ltq_aes_fini(void);
void md5_init_registers(void);

extern spinlock_t global_deu_lock;
extern spinlock_t pwr_lock;

#define CRTCL_SECT_INIT		   spin_lock_init(&global_deu_lock)
#define CRTCL_SECT_START	   spin_lock_irqsave(&global_deu_lock, flag)
#define CRTCL_SECT_END		   spin_unlock_irqrestore(&global_deu_lock, flag)

#define DEU_WAKELIST_INIT(queue) \
	init_waitqueue_head(&queue)

#define DEU_WAIT_EVENT_TIMEOUT(queue, event, flags, timeout)	 \
	do {														 \
		wait_event_interruptible_timeout((queue),				 \
			test_bit((event), &(flags)), (timeout));			\
		clear_bit((event), &(flags));							 \
	} while (0)


#define DEU_WAKEUP_EVENT(queue, event, flags)		  \
	do {											  \
		set_bit((event), &(flags));					  \
		wake_up_interruptible(&(queue));			  \
	} while (0)

#define DEU_WAIT_EVENT(queue, event, flags)			  \
	do {											  \
		wait_event_interruptible(queue,				  \
			test_bit((event), &(flags)));			  \
		clear_bit((event), &(flags));				  \
	} while (0)


typedef struct deu_drv_priv {
	wait_queue_head_t  deu_thread_wait;
#define DEU_EVENT		1
#define DES_ASYNC_EVENT 2
#define AES_ASYNC_EVENT 3
	volatile long	   des_event_flags;
	volatile long	   aes_event_flags;
	volatile long	   deu_event_flags;
	int				   event_src;
	int				   mode;
	u8				   *iv_arg;
	u32				   *deu_rx_buf;
	u32				   *outcopy;
	u32				   deu_rx_len;
	u32				   rx_aligned_len;

	struct aes_priv    *aes_dataptr;
	struct des_priv    *des_dataptr;
} deu_drv_priv_t;


/**
 *	struct aes_priv_t - ASYNC AES
 *	@lock: spinlock lock
 *	@list: crypto queue API list
 *	@hw_status: DEU hw status flag
 *	@aes_wait_flag: flag for sleep queue
 *	@aes_wait_queue: queue attributes for aes
 *	@aes_pid: pid number for AES thread
 *	@aes_sync: atomic wait sync for AES
 *
*/

typedef struct {
	spinlock_t lock;
	struct crypto_queue list;
	unsigned int hw_status;
	volatile long aes_wait_flag;
	wait_queue_head_t aes_wait_queue;

	pid_t aes_pid;

	struct tasklet_struct aes_task;

} aes_priv_t;

/**
 *	struct des_priv_t - ASYNC DES
 *	@lock: spinlock lock
 *	@list: crypto queue API list
 *	@hw_status: DEU hw status flag
 *	@des_wait_flag: flag for sleep queue
 *	@des_wait_queue: queue attributes for des
 *	@des_pid: pid number for DES thread
 *	@des_sync: atomic wait sync for DES
 *
*/

typedef struct {
	spinlock_t lock;
	struct crypto_queue list;
	unsigned int hw_status;
	volatile long des_wait_flag;
	wait_queue_head_t des_wait_queue;

	pid_t des_pid;

	struct tasklet_struct des_task;

} des_priv_t;

struct ltq_deu_config {
	bool aes;
	bool des;
	bool arc4;
	bool md5;
	bool sha1;
	bool md5_hmac;
	bool sha1_hmac;
	bool dma;
	bool async_mode;
};

extern struct ltq_deu_config algo_config;

#if defined (CONFIG_CPU_FREQ) || defined (CONFIG_LTQ_CPU_FREQ)
void ltq_deu_pmcu_init(void);
void power_on_deu_module(char *algo_name);
void power_off_deu_module(char *algo_name);
void powerup_deu(int crypto);
void powerdown_deu(int crypto);
#else
static inline void ltq_deu_pmcu_init(void){};
static inline void power_on_deu_module(char *algo_name) {};
static inline void power_off_deu_module(char *algo_name) {};
static inline void powerup_deu(int crypto) {};
static inline void powerdown_deu(int crypto) {};
#endif /* CONFIG_CPU_FREQ || CONFIG_LTQ_CPU_FREQ */

/* proc stuffs */
#define MAX_DEU_ALGO 7
struct deu_proc {
	char name[10];
	int deu_status;
	void (*toggle_algo)(int mode);
	int (*get_deu_algo_state)(void);
};

void ltq_aes_toggle_algo (int mode);
void ltq_des_toggle_algo (int mode);
void ltq_md5_toggle_algo (int mode);
void ltq_sha1_toggle_algo (int mode);
void ltq_md5_hmac_toggle_algo (int mode);
void ltq_sha1_hmac_toggle_algo (int mode);
void ltq_arc4_toggle_algo (int mode);

int read_aes_algo_status(void);
int read_des_algo_status(void);
int read_arc4_algo_status(void);
int read_md5_algo_status(void);
int read_sha1_algo_status(void);
int read_md5_hmac_algo_status(void);
int read_sha1_hmac_algo_status(void);

/* SHA1 CONSTANT */
#define HASH_CON_VALUE	  0x0701002C
#define INPUT_ENDIAN_SWAP(input)	input_swap(input)
#define DEU_ENDIAN_SWAP(input)	  endian_swap(input)
#define FIND_DEU_CHIP_VERSION	 chip_version()

#define DELAY_PERIOD	10

#define WAIT_AES_DMA_READY()		  \
	do {				  \
		int i;				  \
		volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON; \
		volatile struct aes_t *aes = (volatile struct aes_t *) LTQ_AES_CON; \
		for (i = 0; i < 10; i++)	  \
			udelay(DELAY_PERIOD);	  \
		while (dma->controlr.BSY)	  \
			; 						  \
		while (aes->controlr.BUS) 	  \
			; 						  \
	} while (0)

#define WAIT_DES_DMA_READY()		  \
	do {				  \
		int i;				  \
		volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON; \
		volatile struct des_t *des = (struct des_t *) LTQ_DES_CON; \
		for (i = 0; i < 10; i++)	 \
			udelay(DELAY_PERIOD);	 \
		while (dma->controlr.BSY)	 \
			; 						 \
		while (des->controlr.BUS)	 \
			; 						 \
	} while (0)

#define AES_DMA_MISC_CONFIG()		 \
	do { \
		volatile struct aes_t *aes = (volatile struct aes_t *) LTQ_AES_CON; \
		aes->controlr.KRE = 1;		  \
		aes->controlr.GO = 1;		  \
	} while (0)

#define SHA_HASH_INIT				 \
	do {							   \
		volatile struct deu_hash_t *hash = (struct deu_hash_t *) LTQ_HASH_CON; \
		hash->controlr.ENDI = 1;  \
		hash->controlr.SM = 1;	  \
		hash->controlr.ALGO = 0;  \
		hash->controlr.INIT = 1;  \
	} while (0)

#define MD5_HASH_INIT				 \
	do {							   \
		volatile struct deu_hash_t *hash = (struct deu_hash_t *) LTQ_HASH_CON; \
		hash->controlr.ENDI = 1;  \
		hash->controlr.SM = 1;	  \
		hash->controlr.ALGO = 1;  \
		hash->controlr.INIT = 1;  \
	} while (0)

#define START_DEU_POWER        \
    do { 						\
		ltq_deu_w32(LTQ_DEU_CLC, (ltq_deu_r32(LTQ_DEU_CLC) & ~0x3f)); \
    } while(0)

#define STOP_DEU_POWER      \
    do {            \
		ltq_deu_w32(LTQ_DEU_CLC, (ltq_deu_r32(LTQ_DEU_CLC) | 0x3f)); \
    } while (0)

/* DEU Common Structures */

struct clc_controlr_t {
	u32 Res:26;
	u32 FSOE:1;
	u32 SBWE:1;
	u32 EDIS:1;
	u32 SPEN:1;
	u32 DISS:1;
	u32 DISR:1;

};

struct des_t {
	struct des_controlr {
		u32 KRE:1;
		u32 reserved1:5;
		u32 GO:1;
		u32 STP:1;
		u32 Res2:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 Res3:2;
		u32 F:3;
		u32 O:3;
		u32 BUS:1;
		u32 DAU:1;
		u32 ARS:1;
		u32 SM:1;
		u32 E_D:1;
		u32 M:3;

	} controlr;
	u32 IHR;
	u32 ILR;
	u32 K1HR;
	u32 K1LR;
	u32 K2HR;
	u32 K2LR;
	u32 K3HR;
	u32 K3LR;
	u32 IVHR;
	u32 IVLR;
	u32 OHR;
	u32 OLR;
};

struct aes_t {
	struct aes_controlr {

		u32 KRE:1;
		u32 reserved1:4;
		u32 PNK:1;
		u32 GO:1;
		u32 STP:1;
		u32 reserved2:6;
		u32 NDC:1;
		u32 ENDI:1;
				u32 reserved3:2;
		u32 F:3;
		u32 O:3;
		u32 BUS:1;
		u32 DAU:1;
		u32 ARS:1;
		u32 SM:1;
		u32 E_D:1;
		u32 KV:1;
		u32 K:2;

	} controlr;
	u32 ID3R;
	u32 ID2R;
	u32 ID1R;
	u32 ID0R;
	u32 K7R;
	u32 K6R;
	u32 K5R;
	u32 K4R;
	u32 K3R;
	u32 K2R;
	u32 K1R;
	u32 K0R;
	u32 IV3R;
	u32 IV2R;
	u32 IV1R;
	u32 IV0R;
	u32 OD3R;
	u32 OD2R;
	u32 OD1R;
	u32 OD0R;
};

struct arc4_t {
	struct arc4_controlr {

		u32 KRE:1;
		u32 KLEN:4;
		u32 KSAE:1;
		u32 GO:1;
		u32 STP:1;
		u32 reserved1:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved2:8;
		u32 BUS:1;
		u32 reserved3:1;
		u32 ARS:1;
		u32 SM:1;
		u32 reserved4:4;

	} controlr;
	u32 K3R;
	u32 K2R;
	u32 K1R;
	u32 K0R;

	u32 IDLEN;

	u32 ID3R;
	u32 ID2R;
	u32 ID1R;
	u32 ID0R;

	u32 OD3R;
	u32 OD2R;
	u32 OD1R;
	u32 OD0R;
};

struct deu_hash_t {
	struct hash_controlr {
		u32 reserved1:5;
		u32 KHS:1;
		u32 GO:1;
		u32 INIT:1;
		u32 reserved2:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved3:7;
		u32 DGRY:1;
		u32 BSY:1;
		u32 reserved4:1;
		u32 IRCL:1;
		u32 SM:1;
		u32 KYUE:1;
		u32 HMEN:1;
		u32 SSEN:1;
		u32 ALGO:1;

	} controlr;
	u32 MR;
	u32 D1R;
	u32 D2R;
	u32 D3R;
	u32 D4R;
	u32 D5R;

	u32 dummy;

	u32 KIDX;
	u32 KEY;
	u32 DBN;
};


struct deu_dma_t {
	struct dma_controlr {
		u32 reserved1:22;
		u32 BS:2;
		u32 BSY:1;
		u32 reserved2:1;
		u32 ALGO:2;
		u32 RXCLS:2;
		u32 reserved3:1;
		u32 EN:1;

	} controlr;
};

#endif	/* LTQMIPS_DEU_H */

