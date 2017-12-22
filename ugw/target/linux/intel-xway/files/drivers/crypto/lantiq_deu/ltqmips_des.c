/******************************************************************************
**
** FILE NAME	: ltqmips_des.c
** PROJECT		: LTQ UEIP
** MODULES		: DEU Module
**
** DATE			: September 8, 2009
** AUTHOR		: Mohammad Firdaus
** DESCRIPTION	: Data Encryption Unit Driver for DES Algorithm
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
** 08 Sept 2009 Mohammad Firdaus	Initial UEIP release
*******************************************************************************/

/*!
  \defgroup LTQ_DEU LTQ_DEU_DRIVERS
  \ingroup API
  \brief ltq deu driver
*/

/*!
  \file		ltqmips_des.c
  \ingroup	LTQ_DEU
  \brief	DES encryption DEU driver file
*/

/*!
  \defgroup LTQ_DES_FUNCTIONS LTQ_DES_FUNCTIONS
  \ingroup LTQ_DEU
  \brief LTQ DES Encryption functions
*/

/* Project Header Files */
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <asm/byteorder.h>
#include <crypto/algapi.h>
#include <irq.h>
#include <lantiq_dma.h>
#include <lantiq_soc.h>

#include "ltqmips_deu_dma.h"
#include "ltqmips_deu.h"

/* DMA specific header and variables */
//extern _ifx_deu_device ifx_deu[1];

/* Preprocessor declerations */
#define DES_KEY_SIZE			8
#define DES_EXPKEY_WORDS		32
#define DES_BLOCK_SIZE			8
#define DES3_EDE_KEY_SIZE		(3 * DES_KEY_SIZE)
#define DES3_EDE_EXPKEY_WORDS	(3 * DES_EXPKEY_WORDS)
#define DES3_EDE_BLOCK_SIZE		DES_BLOCK_SIZE

#define MAX_DES_ALGO		5
#define DES_STATUS			0
#define DES_ECB_STATUS		1
#define DES_CBC_STATUS		2
#define DES3_ECB_STATUS		3
#define DES3_CBC_STATUS		4
#define DES3_CBC_ECB_STATUS 5

extern struct ltq_deu_config algo_config;
static int algo_status[MAX_DES_ALGO];
static spinlock_t power_lock;

/* Function Declaration to prevent warning messages */
u32 endian_swap(u32 input);

struct des_ctx {
		int controlr_M;
		int key_length;
		u8 iv[DES_BLOCK_SIZE];
		u32 expkey[DES3_EDE_EXPKEY_WORDS];
};

void set_des_algo_status(unsigned int des_algo, int cmd)
{
	unsigned long flag;

	if (des_algo >= MAX_DES_ALGO) {
		pr_err("algo choice error!!\n");
		return;
	}
	spin_lock_irqsave(&power_lock, flag);
	algo_status[des_algo] = cmd;
	spin_unlock_irqrestore(&power_lock, flag);
}

int read_des_algo_status(void)
{
	int i;
	unsigned long flag;

	spin_lock_irqsave(&power_lock, flag);
	for (i = 0; i <= MAX_DES_ALGO; i++) {
		if (algo_status[i] != CRYPTO_IDLE) {
			spin_unlock_irqrestore(&power_lock, flag);
			return CRYPTO_STARTED;
		 }
	}
	spin_unlock_irqrestore(&power_lock, flag);
	return CRYPTO_IDLE;
}

/*! \fn void des_chip_init (void)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief initialize DES hardware
*/

void des_chip_init (void)
{
	volatile struct des_t *des = (struct des_t *) LTQ_DES_CON;

	if (!algo_config.dma) {
		des->controlr.SM = 1;
		des->controlr.NDC = 1;
		asm("sync");
		des->controlr.ENDI = 1;
		asm("sync");
		des->controlr.ARS = 0;
	} else {
		des->controlr.SM = 1;
		des->controlr.ARS = 0;
		asm("sync");
		des->controlr.NDC = 1;
		asm("sync");
		des->controlr.ENDI = 0;
	}
}

/*! \fn static int des_setkey(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief sets DES key
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length
*/
static int des_setkey(struct crypto_tfm *tfm, const u8 *key,
					  unsigned int keylen)
{
		struct des_ctx *dctx = crypto_tfm_ctx(tfm);

		pr_debug("setkey in %s\n", __FILE__);

		dctx->controlr_M = 0;
		dctx->key_length = keylen;

		memcpy ((u8 *) (dctx->expkey), key, keylen);

		return 0;
}

/*! \fn void ltq_deu_des_core(void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, u32 nbytes, int encdec, int mode)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief main interface to DES hardware
 *	\param ctx_arg crypto algo context
 *	\param out_arg output bytestream
 *	\param in_arg input bytestream
 *	\param iv_arg initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param mode operation mode such as ebc, cbc
*/

static void ltq_deu_des_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
			 u8 *iv_arg, u32 nbytes, int encdec, int mode)
{
	volatile struct des_t *des = (struct des_t *) LTQ_DES_CON;
	struct des_ctx *dctx = ctx_arg;
	u32 *key = dctx->expkey;
	u32 timeout = 0;
	int i = 0;
	int nblocks = 0;
	volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON;
	struct dma_device_info *dma_device = ifx_deu[0].dma_device;
	int wlen = 0;
	u32 *out_dma = NULL;

	des->controlr.M = dctx->controlr_M;
	if (dctx->controlr_M == 0) {
		des->K1HR = DEU_ENDIAN_SWAP(*((u32 *) key + 0));
		des->K1LR = DEU_ENDIAN_SWAP(*((u32 *) key + 1));

	} else {
		/* Hardware Section */
		switch (dctx->key_length) {
		case 24:
			des->K3HR = DEU_ENDIAN_SWAP(*((u32 *) key + 4));
			des->K3LR = DEU_ENDIAN_SWAP(*((u32 *) key + 5));
			/* no break; */
		case 16:
			des->K2HR = DEU_ENDIAN_SWAP(*((u32 *) key + 2));
			des->K2LR = DEU_ENDIAN_SWAP(*((u32 *) key + 3));
			/* no break; */
		case 8:
			des->K1HR = DEU_ENDIAN_SWAP(*((u32 *) key + 0));
			des->K1LR = DEU_ENDIAN_SWAP(*((u32 *) key + 1));
			break;

		default:
			pr_err("Key length not supported!\n");
			return;
		}
	}

	des->controlr.E_D = !encdec;
	/* 0 ECB 1 CBC 2 OFB 3 CFB 4 CTR */
	des->controlr.O = mode;

	if (mode > 0) {
			des->IVHR = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
			des->IVLR = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
	};

	if (!algo_config.dma) {
		nblocks = nbytes / 4;
		
		for (i = 0; i < nblocks; i += 2) {
			/* wait for busy bit to clear */
			/*--- Workaround ----------------------------------------------------
			do a dummy read to the busy flag because it is not raised early
			enough in CFB/OFB 3DES modes */
#ifdef CRYPTO_DEBUG
			pr_debbug("ihr: %x\n", (*((u32 *) in_arg + i)));
			pr_debug("ilr: %x\n", (*((u32 *) in_arg + 1 + i)));
#endif
			des->IHR = (*((u32 *) in_arg + i));
			des->ILR = (*((u32 *) in_arg + 1 + i));

			while (des->controlr.BUS) {
				timeout++;
				if (timeout >= 333000) {
					pr_err("timeout waiting for des busy bit toggle\n");
					break;
				}
			}

			*((u32 *) out_arg + 0 + i) = des->OHR;
			*((u32 *) out_arg + 1 + i) = des->OLR;
		}
	} else { /* dma mode */

		dma->controlr.ALGO = 0;
		des->controlr.DAU = 0;
		dma->controlr.BS = 0;
		dma->controlr.EN = 1;

		while (des->controlr.BUS)
			;

		wlen = dma_device_write (dma_device, (u8 *) in_arg, nbytes, NULL);
		if (wlen != nbytes) {
			dma->controlr.EN = 0;
			pr_err("[%s %s %d]: dma_device_write fail!\n", __FILE__, __func__, __LINE__);
			return;
		}

		WAIT_DES_DMA_READY();

		/* polling DMA rx channel */
		while ((dma_device_read (dma_device, (u8 **) &out_dma, NULL)) == 0) {
			timeout++;

			if (timeout >= 333000) {
				dma->controlr.EN = 0;
				pr_err("[%s %s %d]: timeout!!\n", __FILE__, __func__, __LINE__);
				return;
			}
		}

		WAIT_DES_DMA_READY();
		memcpy(out_arg, out_dma, nbytes);
	}

	if (mode > 0) {
		*(u32 *) iv_arg = DEU_ENDIAN_SWAP(des->IVHR);
		*((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(des->IVLR);
	};

}

/*! \fn void ltq_deu_des(void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg, u32 nbytes, int encdec, int mode)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief main interface to DES hardware
 *	\param ctx_arg crypto algo context
 *	\param out_arg output bytestream
 *	\param in_arg input bytestream
 *	\param iv_arg initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param mode operation mode such as ebc, cbc
*/

static void ltq_deu_des (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
			 u8 *iv_arg, u32 nbytes, int encdec, int mode)
{
	u32 remain = nbytes;
	u32 inc;

	des_chip_init();
	if (!algo_config.dma) {
		ltq_deu_des_core(ctx_arg, out_arg, in_arg,
						iv_arg, nbytes, encdec, mode);
		return;
	}

	while (remain > 0) {
		if (remain >= DEU_MAX_PACKET_SIZE)
			inc = DEU_MAX_PACKET_SIZE;
		else
			inc = remain;

		remain -= inc;
		ltq_deu_des_core(ctx_arg, out_arg,
						in_arg, iv_arg, inc, encdec, mode);

		out_arg += inc;
		in_arg += inc;
   }
}

/*! \fn  void ltq_deu_des_ecb (void *ctx, uint8_t *dst,
 *			 const uint8_t *src, uint8_t *iv,
 *			 size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief sets DES hardware to ECB mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/

static void ltq_deu_des_ecb (void *ctx, uint8_t *dst,
			const uint8_t *src, uint8_t *iv, size_t nbytes,
			int encdec, int inplace)
{
	 ltq_deu_des (ctx, dst, src, NULL, nbytes, encdec, 0);
}

/*! \fn  void ltq_deu_des_cbc (void *ctx, uint8_t *dst, const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief sets DES hardware to CBC mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/
static void ltq_deu_des_cbc (void *ctx, uint8_t *dst,
			const uint8_t *src, uint8_t *iv, size_t nbytes,
			int encdec, int inplace)
{
	 ltq_deu_des (ctx, dst, src, iv, nbytes, encdec, 1);
}

/*! \fn void des_encrypt (struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief encrypt DES_BLOCK_SIZE of data
 *	\param tfm linux crypto algo transform
 *	\param out output bytestream
 *	\param in input bytestream
*/
static void des_encrypt(struct crypto_tfm *tfm,
				uint8_t *out, const uint8_t *in)
{
	struct des_ctx *ctx = crypto_tfm_ctx(tfm);
	unsigned long flag;

	set_des_algo_status(DES_STATUS, CRYPTO_STARTED);
	CRTCL_SECT_START;
	powerup_deu(DES_INIT);
	ltq_deu_des (ctx, out, in, NULL, DES_BLOCK_SIZE,
					CRYPTO_DIR_ENCRYPT, 0);
	powerdown_deu(DES_INIT);
	CRTCL_SECT_END;
	set_des_algo_status(DES_STATUS, CRYPTO_IDLE);

}

/*! \fn void des_decrypt (struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief encrypt DES_BLOCK_SIZE of data
 *	\param tfm linux crypto algo transform
 *	\param out output bytestream
 *	\param in input bytestream
*/
static void des_decrypt (struct crypto_tfm *tfm,
				uint8_t *out, const uint8_t *in)
{
	struct des_ctx *ctx = crypto_tfm_ctx(tfm);
	unsigned long flag;

	set_des_algo_status(DES_STATUS, CRYPTO_STARTED);
	CRTCL_SECT_START;
	powerup_deu(DES_INIT);
	ltq_deu_des (ctx, out, in, NULL, DES_BLOCK_SIZE,
					CRYPTO_DIR_DECRYPT, 0);
	powerdown_deu(DES_INIT);
	CRTCL_SECT_END;
	set_des_algo_status(DES_STATUS, CRYPTO_IDLE);
}

/*
 *	 \brief RFC2451:
 *
 *	 For DES-EDE3, there is no known need to reject weak or
 *	 complementation keys.	Any weakness is obviated by the use of
 *	 multiple keys.
 *
 *	 However, if the first two or last two independent 64-bit keys are
 *	 equal (k1 == k2 or k2 == k3), then the DES3 operation is simply the
 *	 same as DES.  Implementers MUST reject keys that exhibit this
 *	 property.
 *
 */

/*! \fn int des3_ede_setkey(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief sets 3DES key
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length
*/
static int des3_ede_setkey(struct crypto_tfm *tfm,
					const u8 *key,
					unsigned int keylen)
{
		struct des_ctx *dctx = crypto_tfm_ctx(tfm);

		pr_debug("setkey in %s\n", __FILE__);
		dctx->controlr_M = keylen / 8 + 1;
		dctx->key_length = keylen;
		memcpy ((u8 *) (dctx->expkey), key, keylen);
		return 0;
}

/*
 * \brief DES function mappings
*/
struct crypto_alg ltqdeu_des_alg = {
		.cra_name				=		"des",
		.cra_driver_name		=		"ltqdeu-des",
		.cra_flags				=		CRYPTO_ALG_TYPE_CIPHER,
		.cra_blocksize			=		DES_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct des_ctx),
		.cra_module				=		THIS_MODULE,
		.cra_alignmask			=		3,
		.cra_priority			=		300,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_des_alg.cra_list),
		.cra_u					=		{
				.cipher = {
					.cia_min_keysize		=		DES_KEY_SIZE,
					.cia_max_keysize		=		DES_KEY_SIZE,
					.cia_setkey				=		des_setkey,
					.cia_encrypt			=		des_encrypt,
		.			cia_decrypt			   =	   des_decrypt }
				}
};

/*
 * \brief DES function mappings
*/
struct crypto_alg ltqdeu_des3_ede_alg = {
		.cra_name				=		"des3_ede",
		.cra_driver_name		=		"ltqdeu-des3_ede",
		.cra_flags				=		CRYPTO_ALG_TYPE_CIPHER,
		.cra_blocksize			=		DES_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct des_ctx),
		.cra_module				=		THIS_MODULE,
		.cra_alignmask			=		3,
		.cra_priority			=		300,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_des3_ede_alg.cra_list),
		.cra_u					=		{ .cipher = {
		.cia_min_keysize		=		DES_KEY_SIZE,
		.cia_max_keysize		=		DES_KEY_SIZE,
		.cia_setkey				=		des3_ede_setkey,
		.cia_encrypt			=		des_encrypt,
		.cia_decrypt			=		des_decrypt } }
};

/*! \fn int ecb_des_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief ECB DES encrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
*/
static int ecb_des_encrypt(struct blkcipher_desc *desc,
					struct scatterlist *dst, struct scatterlist *src,
					unsigned int nbytes)
{
		struct des_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int enc_bytes;
		unsigned long flag;

		set_des_algo_status(DES_ECB_STATUS, CRYPTO_STARTED);
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = enc_bytes = walk.nbytes)) {
				enc_bytes -= (nbytes % DES_BLOCK_SIZE);
				CRTCL_SECT_START;
				powerup_deu(DES_INIT);
				ltq_deu_des_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
							   NULL, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
				nbytes &= DES_BLOCK_SIZE - 1;
				powerdown_deu(DES_INIT);
				CRTCL_SECT_END;
				err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_des_algo_status(DES_ECB_STATUS, CRYPTO_IDLE);
		return err;
}

/*! \fn int ecb_des_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief ECB DES decrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
static int ecb_des_decrypt(struct blkcipher_desc *desc,
					struct scatterlist *dst, struct scatterlist *src,
					unsigned int nbytes)
{
		struct des_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		unsigned int dec_bytes;
		int err;
		unsigned long flag;

		set_des_algo_status(DES_ECB_STATUS, CRYPTO_STARTED);
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = dec_bytes = walk.nbytes)) {
				dec_bytes -= (nbytes % DES_BLOCK_SIZE);
				CRTCL_SECT_START;
				powerup_deu(DES_INIT);
				ltq_deu_des_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
							   NULL, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
				nbytes &= DES_BLOCK_SIZE - 1;
				powerdown_deu(DES_INIT);
				CRTCL_SECT_END;
				err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_des_algo_status(DES_ECB_STATUS, CRYPTO_IDLE);
		return err;
}

/*
 * \brief DES function mappings
*/
struct crypto_alg ltqdeu_ecb_des_alg = {
		.cra_name				=		"ecb(des)",
		.cra_driver_name		=		"ltqdeu-ecb(des)",
		.cra_flags				=		CRYPTO_ALG_TYPE_BLKCIPHER,
		.cra_blocksize			=		DES_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct des_ctx),
		.cra_type				=		&crypto_blkcipher_type,
		.cra_priority			=		300,
		.cra_module				=		THIS_MODULE,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_ecb_des_alg.cra_list),
		.cra_u					=		{
				.blkcipher = {
						.min_keysize			=		DES_KEY_SIZE,
						.max_keysize			=		DES_KEY_SIZE,
						.setkey					=		des_setkey,
						.encrypt				=		ecb_des_encrypt,
						.decrypt				=		ecb_des_decrypt,
				}
		}
};

/*
 * \brief DES function mappings
*/
struct crypto_alg ltqdeu_ecb_des3_ede_alg = {
		.cra_name				=		"ecb(des3_ede)",
		.cra_driver_name		=		"ltqdeu-ecb(des3_ede)",
		.cra_flags				=		CRYPTO_ALG_TYPE_BLKCIPHER,
		.cra_blocksize			=		DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct des_ctx),
		.cra_type				=		&crypto_blkcipher_type,
		.cra_priority			=		300,
		.cra_module				=		THIS_MODULE,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_ecb_des3_ede_alg.cra_list),
		.cra_u					=		{
				.blkcipher = {
						.min_keysize			=		DES3_EDE_KEY_SIZE,
						.max_keysize			=		DES3_EDE_KEY_SIZE,
						.setkey					=		des3_ede_setkey,
						.encrypt				=		ecb_des_encrypt,
						.decrypt				=		ecb_des_decrypt,
				}
		}
};

/*! \fn int cbc_des_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief CBC DES encrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
static int cbc_des_encrypt(struct blkcipher_desc *desc,
					struct scatterlist *dst, struct scatterlist *src,
					unsigned int nbytes)
{
	struct des_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;
	int err;
	unsigned int enc_bytes;
	unsigned long flag;

	set_des_algo_status(DES_CBC_STATUS, CRYPTO_STARTED);

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);

	while ((nbytes = enc_bytes = walk.nbytes)) {
			u8 *iv = walk.iv;
			enc_bytes -= (nbytes % DES_BLOCK_SIZE);
			CRTCL_SECT_START;
			powerup_deu(DES_INIT);
			ltq_deu_des_cbc(ctx, walk.dst.virt.addr, walk.src.virt.addr,
						   iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
			powerdown_deu(DES_INIT);
			CRTCL_SECT_END;
			nbytes &= (DES_BLOCK_SIZE - 1);
			err = blkcipher_walk_done(desc, &walk, nbytes);
	}
	set_des_algo_status(DES_CBC_STATUS, CRYPTO_IDLE);


	return err;
}

/*! \fn int cbc_des_decrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief CBC DES decrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
static int cbc_des_decrypt(struct blkcipher_desc *desc,
					struct scatterlist *dst, struct scatterlist *src,
					unsigned int nbytes)
{
	struct des_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
	struct blkcipher_walk walk;
	int err;
	unsigned int dec_bytes;
	unsigned long flag;

	set_des_algo_status(DES_CBC_STATUS, CRYPTO_STARTED);

	blkcipher_walk_init(&walk, dst, src, nbytes);
	err = blkcipher_walk_virt(desc, &walk);

	while ((nbytes = dec_bytes = walk.nbytes)) {
			u8 *iv = walk.iv;
			dec_bytes -= (nbytes % DES_BLOCK_SIZE);;
			CRTCL_SECT_START;
			powerup_deu(DES_INIT);
			ltq_deu_des_cbc(ctx, walk.dst.virt.addr, walk.src.virt.addr,
						   iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
			powerdown_deu(DES_INIT);
			CRTCL_SECT_END;
			nbytes &= (DES_BLOCK_SIZE - 1);
			err = blkcipher_walk_done(desc, &walk, nbytes);
	}

	set_des_algo_status(DES_CBC_STATUS, CRYPTO_IDLE);

	return err;
}

/*
 * \brief DES function mappings
*/
struct crypto_alg ltqdeu_cbc_des_alg = {
		.cra_name				=		"cbc(des)",
		.cra_driver_name		=		"ltqdeu-cbc(des)",
		.cra_flags				=		CRYPTO_ALG_TYPE_BLKCIPHER,
		.cra_blocksize			=		DES_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct des_ctx),
		.cra_type				=		&crypto_blkcipher_type,
		.cra_priority			=		300,
		.cra_module				=		THIS_MODULE,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_cbc_des_alg.cra_list),
		.cra_u					=		{
				.blkcipher = {
						.min_keysize			=		DES_KEY_SIZE,
						.max_keysize			=		DES_KEY_SIZE,
						.ivsize					=		DES_BLOCK_SIZE,
						.setkey					=		des_setkey,
						.encrypt				=		cbc_des_encrypt,
						.decrypt				=		cbc_des_decrypt,
				}
		}
};

/*
 * \brief DES function mappings
*/
struct crypto_alg ltqdeu_cbc_des3_ede_alg = {
		.cra_name				=		"cbc(des3_ede)",
		.cra_driver_name		=		"ltqdeu-cbc(des3_ede)",
		.cra_flags				=		CRYPTO_ALG_TYPE_BLKCIPHER,
		.cra_blocksize			=		DES3_EDE_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct des_ctx),
		.cra_type				=		&crypto_blkcipher_type,
		.cra_priority			=		300,
		.cra_module				=		THIS_MODULE,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_cbc_des3_ede_alg.cra_list),
		.cra_u					=		{
				.blkcipher = {
						.min_keysize			=		DES3_EDE_KEY_SIZE,
						.max_keysize			=		DES3_EDE_KEY_SIZE,
						.ivsize					=		DES_BLOCK_SIZE,
						.setkey					=		des3_ede_setkey,
						.encrypt				=		cbc_des_encrypt,
						.decrypt				=		cbc_des_decrypt,
				}
		}
};

void ltq_des_toggle_algo(int mode)
{
	int ret = 0;

	if (mode) {
		crypto_unregister_alg(&ltqdeu_des_alg);
		crypto_unregister_alg(&ltqdeu_ecb_des_alg);
		crypto_unregister_alg(&ltqdeu_cbc_des_alg);
		crypto_unregister_alg(&ltqdeu_des3_ede_alg);
		crypto_unregister_alg(&ltqdeu_ecb_des3_ede_alg);
		crypto_unregister_alg(&ltqdeu_cbc_des3_ede_alg);

		ltqdeu_des_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_ecb_des_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_cbc_des_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_des3_ede_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_ecb_des3_ede_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_cbc_des3_ede_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
	} else {
		ret = crypto_register_alg(&ltqdeu_des_alg);
		if (ret < 0)
			goto des_err;
		ret = crypto_register_alg(&ltqdeu_ecb_des_alg);
		if (ret < 0)
			goto ecb_des_err;
		ret = crypto_register_alg(&ltqdeu_cbc_des_alg);
		if (ret < 0)
			goto cbc_des_err;
		ret = crypto_register_alg(&ltqdeu_des3_ede_alg);
		if (ret < 0)
			goto des3_ede_err;
		ret = crypto_register_alg(&ltqdeu_ecb_des3_ede_alg);
		if (ret < 0)
			goto ecb_des3_ede_err;
		ret = crypto_register_alg(&ltqdeu_cbc_des3_ede_alg);
		if (ret < 0)
			goto cbc_des3_ede_err;
	}

	return;

des_err:
		crypto_unregister_alg(&ltqdeu_des_alg);
		pr_err("LTQ des initialization failed!\n");
		return;
ecb_des_err:
		crypto_unregister_alg(&ltqdeu_ecb_des_alg);
		pr_err("LTQ ecb_des initialization failed!\n");
		return;
cbc_des_err:
		crypto_unregister_alg(&ltqdeu_cbc_des_alg);
		pr_err("LTQ cbc_des initialization failed!\n");
		return;
des3_ede_err:
		crypto_unregister_alg(&ltqdeu_des3_ede_alg);
		pr_err("LTQ des3_ede initialization failed!\n");
		return;
ecb_des3_ede_err:
		crypto_unregister_alg(&ltqdeu_ecb_des3_ede_alg);
		pr_err("LTQ ecb_des3_ede initialization failed!\n");
		return;
cbc_des3_ede_err:
		crypto_unregister_alg(&ltqdeu_cbc_des3_ede_alg);
		pr_err("LTQ cbc_des3_ede initialization failed!\n");
		return;
}


/*! \fn int ltq_des_init (void)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief initialize des driver
*/
int ltq_des_init (void)
{
	int i, ret = -ENOSYS;

	ret = crypto_register_alg(&ltqdeu_des_alg);
	if (ret < 0)
		goto des_err;

	ret = crypto_register_alg(&ltqdeu_ecb_des_alg);
	if (ret < 0)
		goto ecb_des_err;

	ret = crypto_register_alg(&ltqdeu_cbc_des_alg);
	if (ret < 0)
		goto cbc_des_err;


	ret = crypto_register_alg(&ltqdeu_des3_ede_alg);
	if (ret < 0)
		goto des3_ede_err;

	ret = crypto_register_alg(&ltqdeu_ecb_des3_ede_alg);
	if (ret < 0)
		goto ecb_des3_ede_err;

	ret = crypto_register_alg(&ltqdeu_cbc_des3_ede_alg);
	if (ret < 0)
		goto cbc_des3_ede_err;

	spin_lock_init(&power_lock);

	for (i = 0; i < MAX_DES_ALGO; i++)
		set_des_algo_status(i, CRYPTO_IDLE);


	pr_notice("LTQ DEU DES initialized%s.\n",
		!algo_config.dma ? "" : " (DMA)");
	return ret;

des_err:
	crypto_unregister_alg(&ltqdeu_des_alg);
	pr_err("LTQ des initialization failed!\n");
	return ret;
ecb_des_err:
	crypto_unregister_alg(&ltqdeu_ecb_des_alg);
	pr_err("LTQ ecb_des initialization failed!\n");
	return ret;
cbc_des_err:
	crypto_unregister_alg(&ltqdeu_cbc_des_alg);
	pr_err("LTQ cbc_des initialization failed!\n");
	return ret;
des3_ede_err:
	crypto_unregister_alg(&ltqdeu_des3_ede_alg);
	pr_err("LTQ des3_ede initialization failed!\n");
	return ret;
ecb_des3_ede_err:
	crypto_unregister_alg(&ltqdeu_ecb_des3_ede_alg);
	pr_err("LTQ ecb_des3_ede initialization failed!\n");
	return ret;
cbc_des3_ede_err:
	crypto_unregister_alg(&ltqdeu_cbc_des3_ede_alg);
	pr_err("LTQ cbc_des3_ede initialization failed!\n");
	return ret;
}

/*! \fn void ltq_des_fini (void)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief unregister des driver
*/
void ltq_des_fini (void)
{
	crypto_unregister_alg (&ltqdeu_des_alg);
	crypto_unregister_alg (&ltqdeu_ecb_des_alg);
	crypto_unregister_alg (&ltqdeu_cbc_des_alg);
	crypto_unregister_alg (&ltqdeu_des3_ede_alg);
	crypto_unregister_alg (&ltqdeu_ecb_des3_ede_alg);
	crypto_unregister_alg (&ltqdeu_cbc_des3_ede_alg);
}

