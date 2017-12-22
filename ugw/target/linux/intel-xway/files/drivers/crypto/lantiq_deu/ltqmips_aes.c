/***********************************************************************
**
** FILE NAME		: ltqmips_aes.c
** PROJECT			: LTQ UEIP
** MODULES			: DEU Module
**
** DATE			:	September 8, 2009
** AUTHOR		:	Mohammad Firdaus
** DESCRIPTION	:	Data Encryption Unit Driver for AES Algorithm
** COPYRIGHT	:	Copyright (c) 2009
**					Infineon Technologies AG
**					Am Campeon 1-12, 85579 Neubiberg, Germany
**
**		This program is free software; you can
**		redistribute it and/or modify
**		it under the terms of the GNU
**		General Public License as published by
**		the Free Software Foundation; either
**		version 2 of the License, or
**		(at your option) any later version.
**
** HISTORY
** $Date				$Author					$Comment
** 08,Sept 2009		Mohammad Firdaus		Initial UEIP release
************************************************************************/
/*!
 \defgroup LTQ_DEU LTQ_DEU_DRIVERS
 \ingroup API
 \brief ltq DEU driver module
*/

/*!
	\file ltqmips_aes.c
	\ingroup LTQ_DEU
	\brief AES Encryption Driver main file
*/

/*!
 \defgroup LTQ_AES_FUNCTIONS LTQ_AES_FUNCTIONS
 \ingroup LTQ_DEU
 \brief LTQ AES driver Functions
*/


/* Project Header Files */
#if defined(CONFIG_MODVERSIONS)
#define MODVERSIONS
#include <linux/modeversions>
#endif

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <asm/byteorder.h>
#include <crypto/algapi.h>
#include <lantiq_soc.h>
#include <irq.h>
#include <lantiq_dma.h>

#include "ltqmips_deu.h"
#include "ltqmips_deu_dma.h"

/* Definition of constants */
#define AES_MIN_KEY_SIZE		16
#define AES_MAX_KEY_SIZE		32
#define AES_BLOCK_SIZE			16
#define CTR_RFC3686_NONCE_SIZE	4
#define CTR_RFC3686_IV_SIZE		8
#define CTR_RFC3686_MAX_KEY_SIZE	(AES_MAX_KEY_SIZE + \
									CTR_RFC3686_NONCE_SIZE)

/* Function decleration */
u32 endian_swap(u32 input);

#define MAX_AES_ALGO			5
#define AES_STATUS				0
#define AES_ECB_STATUS			1
#define AES_CBC_STATUS			2
#define AES_CTR_STATUS			3
#define AES_RFC3686_STATUS		4

static int algo_status[MAX_AES_ALGO];

struct aes_ctx {
		int key_length;
		u32 buf[AES_MAX_KEY_SIZE];
		u8 nonce[CTR_RFC3686_NONCE_SIZE];
};

//extern _ifx_deu_device ifx_deu[1];
extern struct ltq_deu_config algo_config;
static spinlock_t power_lock;

static void hexdump(unsigned char *buf, unsigned int len)
{
		print_hex_dump(KERN_INFO, "", DUMP_PREFIX_OFFSET,
					16, 1, buf, len, false);
}

void set_aes_algo_status(unsigned int aes_algo, int cmd)
{
		unsigned long flag;

		if (aes_algo >= MAX_AES_ALGO) {
				pr_err("algo choice error!!\n");
				return;
		}

		spin_lock_irqsave(&power_lock, flag);
		algo_status[aes_algo] = cmd;
		spin_unlock_irqrestore(&power_lock, flag);
}

int read_aes_algo_status(void)
{
		int i;
		unsigned long flag;

		spin_lock_irqsave(&power_lock, flag);
		for (i = 0; i <= MAX_AES_ALGO; i++) {
				if (algo_status[i] != CRYPTO_IDLE) {
						spin_unlock_irqrestore(&power_lock, flag);
						return CRYPTO_STARTED;
				}
		}
		spin_unlock_irqrestore(&power_lock, flag);
		return CRYPTO_IDLE;
}

/*! \fn void aes_chip_init (void)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief initialize AES hardware
*/

void aes_chip_init(void)
{
		volatile struct aes_t *aes = (struct aes_t *) LTQ_AES_CON;

		if (!algo_config.dma) {
				aes->controlr.SM = 1;
				aes->controlr.NDC = 0;
				asm("sync");
				aes->controlr.ENDI = 1;
				asm("sync");
				aes->controlr.ARS = 0;
		} else {
				aes->controlr.SM = 1;
				aes->controlr.ARS = 0;
				asm("sync");
				aes->controlr.NDC = 1;
				asm("sync");
				aes->controlr.ENDI = 0;
		}
}

/*! \fn int aes_set_key (struct crypto_tfm *tfm, const uint8_t *in_key,
 *						unsigned int key_len)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets the AES keys
 *	\param tfm linux crypto algo transform
 *	\param in_key input key
 *	\param key_len key lengths of 16, 24 and 32 bytes supported
 *	\return -EINVAL - bad key length, 0 - SUCCESS
*/
int aes_set_key(struct crypto_tfm *tfm, const u8 *in_key, unsigned int key_len)
{
		struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
		unsigned long *flags = (unsigned long *) &tfm->crt_flags;

		if (key_len != 16 && key_len != 24 && key_len != 32) {
				*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
				return -EINVAL;
		}

		ctx->key_length = key_len;
		pr_debug("ctx @%p, key_len %d, ctx->key_length %d\n",
						ctx, key_len, ctx->key_length);
		memcpy((u8 *) (ctx->buf), in_key, key_len);

		return 0;
}

/*! \fn void ltq_deu_aes_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
 *							u8 *iv_arg, size_t nbytes, int encdec, int mode)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief main interface to AES hardware
 *	\param ctx_arg crypto algo context
 *	\param out_arg output bytestream
 *	\param in_arg input bytestream
 *	\param iv_arg initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param mode operation mode such as ebc, cbc, ctr
 *
*/
void ltq_deu_aes_core (void *ctx_arg, u8 *out_arg,
											const u8 *in_arg,
											u8 *iv_arg, size_t nbytes,
											int encdec, int mode)

{
		volatile struct aes_t *aes = (volatile struct aes_t *) LTQ_AES_CON;
		struct aes_ctx *ctx = (struct aes_ctx *)ctx_arg;
		u32 *in_key = ctx->buf;
		int key_len = ctx->key_length;
		u32 timeout = 0;
		int i = 0;
		int byte_cnt = nbytes;
		volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON;
		struct dma_device_info *dma_device = ifx_deu[0].dma_device;
		int wlen = 0;
		int bsize = nbytes;
		u32 *out_dma = NULL;

		/* 128, 192 or 256 bit key length */
		aes->controlr.K = key_len / 8 - 2;
		if (key_len == 128 / 8) {
				aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
				aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
				aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
				aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
		}	else if (key_len == 192 / 8) {
				aes->K5R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
				aes->K4R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
				aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
				aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
				aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 4));
				aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 5));
		}	else if (key_len == 256 / 8) {
				aes->K7R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
				aes->K6R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
				aes->K5R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
				aes->K4R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
				aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 4));
				aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 5));
				aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 6));
				aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 7));
		}	else {
				pr_err("[%s %s %d]: Invalid key_len : %d\n",
						__FILE__, __func__, __LINE__, key_len);
				hexdump((u8 *) in_key, key_len);
				return;
		}

		/* let HW pre-process DEcryption key in any case (even if
			 ENcryption is used). Key Valid (KV) bit is then only
			 checked in decryption routine! */
		aes->controlr.PNK = 1;

		while (aes->controlr.BUS)
			;

		if (algo_config.dma) {
				AES_DMA_MISC_CONFIG();
		}

		aes->controlr.E_D = !encdec;
		aes->controlr.O = mode; /* 0 ECB 1 CBC 2 OFB 3 CFB 4 CTR */

		/* default; only for CFB and OFB modes;
		* change only for customer-specific apps
		* aes->controlr.F = 128;
		*/

		if (mode > 0) {
				aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
				aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
				aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
				aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));
		};

		if (!algo_config.dma) {
				/* FPI mode */
				i = 0;
				while (byte_cnt >= 16) {
						aes->ID3R = (*((u32 *) in_arg + (i * 4) + 0));
						aes->ID2R = (*((u32 *) in_arg + (i * 4) + 1));
						aes->ID1R = (*((u32 *) in_arg + (i * 4) + 2));
						/* start crypto */
						aes->ID0R = (*((u32 *) in_arg + (i * 4) + 3));

						while (aes->controlr.BUS)
							;

						*((volatile u32 *) out_arg + (i * 4) + 0) = aes->OD3R;
						*((volatile u32 *) out_arg + (i * 4) + 1) = aes->OD2R;
						*((volatile u32 *) out_arg + (i * 4) + 2) = aes->OD1R;
						*((volatile u32 *) out_arg + (i * 4) + 3) = aes->OD0R;

						i++;
						byte_cnt -= 16;
				}

				/* To handle all non-aligned bytes (not aligned to 16B size */
				if (byte_cnt) {
						aes->ID3R = (*((u32 *) in_arg + (i * 4) + 0));
						aes->ID2R = (*((u32 *) in_arg + (i * 4) + 1));
						aes->ID1R = (*((u32 *) in_arg + (i * 4) + 2));
						/* start crypto */
						aes->ID0R = (*((u32 *) in_arg + (i * 4) + 3));

						while (aes->controlr.BUS)
							;

						*((volatile u32 *) out_arg + (i * 4) + 0) = aes->OD3R;
						*((volatile u32 *) out_arg + (i * 4) + 1) = aes->OD2R;
						*((volatile u32 *) out_arg + (i * 4) + 2) = aes->OD1R;
						*((volatile u32 *) out_arg + (i * 4) + 3) = aes->OD0R;

						/* to ensure that the extended pages are clean */
						memset (out_arg + (i * 16) + (nbytes % AES_BLOCK_SIZE),
								0,
								(AES_BLOCK_SIZE - (nbytes % AES_BLOCK_SIZE)));

				}
		} else {
			/* to handle non aligned pages,
			* especially for rfc3686 algo in IPSEC
			*/
			if (bsize % AES_BLOCK_SIZE)
				nbytes = (bsize - (bsize % AES_BLOCK_SIZE) +
							AES_BLOCK_SIZE);

			dma->controlr.ALGO = 1; /* AES */
			dma->controlr.BS = 0;
			dma->controlr.EN = 1;
			aes->controlr.DAU = 0;

			while (aes->controlr.BUS)
				;

			wlen = dma_device_write (dma_device, (u8 *)in_arg, nbytes, NULL);
			if (wlen != nbytes) {
				dma->controlr.EN = 0;
				pr_err("[%s %s %d]: dma_device_write fail!\n",
						__FILE__, __func__, __LINE__);
				return;
			}

			WAIT_AES_DMA_READY();

			/* polling DMA rx channel */
			while ((dma_device_read (dma_device,
					(u8 **) &out_dma, NULL)) == 0) {
				timeout++;

				if (timeout >= 333000) {
					dma->controlr.EN = 0;
					pr_err("[%s %s %d]: timeout!!\n",
						__FILE__, __func__, __LINE__);
					return;
				}
			}
			 /* WAIT_AES_DMA_READY(); */
			if (bsize % AES_BLOCK_SIZE)
				nbytes = nbytes - AES_BLOCK_SIZE + (bsize % AES_BLOCK_SIZE);

			memcpy(out_arg, out_dma, nbytes);
		}

		if (mode > 0) {
				*((u32 *) iv_arg) = aes->IV3R;
				*((u32 *) iv_arg + 1) = aes->IV2R;
				*((u32 *) iv_arg + 2) = aes->IV1R;
				*((u32 *) iv_arg + 3) = aes->IV0R;
				*((u32 *) iv_arg) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg));
				*((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
				*((u32 *) iv_arg + 2) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
				*((u32 *) iv_arg + 3) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));
		}
}

/*!
 *	\fn int ctr_rfc3686_aes_set_key (struct crypto_tfm *tfm,
 *							const uint8_t *in_key, unsigned int key_len)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets RFC3686 key
 *	\param tfm linux crypto algo transform
 *	\param in_key input key
 *	\param key_len key lengths of 20, 28 and 36 bytes supported; last 4 bytes is
 *			nounce
 *	\return 0 - SUCCESS
 *	-EINVAL - bad key length
*/
int ctr_rfc3686_aes_set_key (struct crypto_tfm *tfm,
			const uint8_t *in_key, unsigned int key_len)
{
		struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
		unsigned long *flags = (unsigned long *)&tfm->crt_flags;

		memcpy(ctx->nonce, in_key + (key_len - CTR_RFC3686_NONCE_SIZE),
					 CTR_RFC3686_NONCE_SIZE);

		/* Remove 4 bytes of nonce */
		key_len -= CTR_RFC3686_NONCE_SIZE;

		pr_debug("ctr_rfc3686_aes_set_key in %s, keylen: %d\n",
						__FILE__, key_len);

		if ((key_len != 16) && (key_len != 24) &&
				(key_len != 32)) {
				*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
				return -EINVAL;
		}

		ctx->key_length = key_len;
		memcpy ((u8 *) (ctx->buf), in_key, key_len);

		return 0;
}

/*! \fn void ltq_deu_aes(void *ctx_arg, u8 *out_arg,
 *				const u8 *in_arg, u8 *iv_arg, u32 nbytes, int encdec, int mode)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief main interface with deu hardware
 *	\param ctx_arg crypto algo context
 *	\param out_arg output bytestream
 *	\param in_arg input bytestream
 *	\param iv_arg initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param mode operation mode such as ebc, cbc, ctr
*/

void ltq_deu_aes(void *ctx_arg, u8 *out_arg, const u8 *in_arg,
				u8 *iv_arg, u32 nbytes, int encdec, int mode)
{
		u32 remain = nbytes;
		u32 inc;

		aes_chip_init();

		if (!algo_config.dma) {
			ltq_deu_aes_core(ctx_arg, out_arg, in_arg, iv_arg,
								nbytes, encdec, mode);
			return;
		}

		while (remain > 0) {
			if (remain >= DEU_MAX_PACKET_SIZE)
				inc = DEU_MAX_PACKET_SIZE;
			else
				inc = remain;

			remain -= inc;

			ltq_deu_aes_core(ctx_arg, out_arg, in_arg,
							iv_arg, inc, encdec, mode);

			out_arg += inc;
			in_arg += inc;
		}

}

/*! \fn void ltq_deu_aes_ecb (void *ctx, uint8_t *dst,
 *		const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets AES hardware to ECB mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/
void ltq_deu_aes_ecb(void *ctx, uint8_t *dst, const uint8_t *src,
				uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
		ltq_deu_aes(ctx, dst, src, NULL, nbytes, encdec, 0);
}

/*! \fn void ltq_deu_aes_cbc (void *ctx, uint8_t *dst,
 *		const uint8_t *src, uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets AES hardware to CBC mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/
void ltq_deu_aes_cbc(void *ctx, uint8_t *dst, const uint8_t *src,
				uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
		ltq_deu_aes(ctx, dst, src, iv, nbytes, encdec, 1);
}

/*! \fn void ltq_deu_aes_ofb (void *ctx, uint8_t *dst, const uint8_t *src,
 *			uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets AES hardware to OFB mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/
void ltq_deu_aes_ofb(void *ctx, uint8_t *dst, const uint8_t *src,
				uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
		ltq_deu_aes(ctx, dst, src, iv, nbytes, encdec, 2);
}

/*! \fn void ltq_deu_aes_cfb (void *ctx, uint8_t *dst, const uint8_t *src,
 *			uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets AES hardware to CFB mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/
void ltq_deu_aes_cfb(void *ctx, uint8_t *dst, const uint8_t *src,
				uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
		ltq_deu_aes(ctx, dst, src, iv, nbytes, encdec, 3);
}

/*! \fn void ltq_deu_aes_ctr (void *ctx, uint8_t *dst, const uint8_t *src,
 *			uint8_t *iv, size_t nbytes, int encdec, int inplace)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief sets AES hardware to CTR mode
 *	\param ctx crypto algo context
 *	\param dst output bytestream
 *	\param src input bytestream
 *	\param iv initialization vector
 *	\param nbytes length of bytestream
 *	\param encdec 1 for encrypt; 0 for decrypt
 *	\param inplace not used
*/
void ltq_deu_aes_ctr(void *ctx, uint8_t *dst, const uint8_t *src,
				uint8_t *iv, size_t nbytes, int encdec, int inplace)
{
		ltq_deu_aes(ctx, dst, src, iv, nbytes, encdec, 4);
}

/*! \fn void aes_encrypt (struct crypto_tfm *tfm, uint8_t *out,
 *			const uint8_t *in)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief encrypt AES_BLOCK_SIZE of data
 *	\param tfm linux crypto algo transform
 *	\param out output bytestream
 *	\param in input bytestream
*/
void aes_encrypt(struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
{
		struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
		unsigned long flag;

		set_aes_algo_status(AES_STATUS, CRYPTO_STARTED);
		CRTCL_SECT_START;
		powerup_deu(AES_INIT);
		ltq_deu_aes(ctx, out, in, NULL, AES_BLOCK_SIZE,
						CRYPTO_DIR_ENCRYPT, 0);
		powerdown_deu(AES_INIT);
		CRTCL_SECT_END;
		set_aes_algo_status(AES_STATUS, CRYPTO_IDLE);
}

/*! \fn void aes_decrypt (struct crypto_tfm *tfm, uint8_t *out,
 *			const uint8_t *in)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief decrypt AES_BLOCK_SIZE of data
 *	\param tfm linux crypto algo transform
 *	\param out output bytestream
 *	\param in input bytestream
*/
void aes_decrypt(struct crypto_tfm *tfm, uint8_t *out, const uint8_t *in)
{
		struct aes_ctx *ctx = crypto_tfm_ctx(tfm);
		unsigned long flag;

		set_aes_algo_status(AES_STATUS, CRYPTO_STARTED);
		CRTCL_SECT_START;
		powerup_deu(AES_INIT);
		ltq_deu_aes (ctx, out, in, NULL, AES_BLOCK_SIZE,
						CRYPTO_DIR_DECRYPT, 0);
		powerdown_deu(AES_INIT);
		CRTCL_SECT_END;
		set_aes_algo_status(AES_STATUS, CRYPTO_IDLE);
}

/*
 * \brief AES function mappings
*/

struct crypto_alg ltqdeu_aes_alg = {
		.cra_name				=		"aes",
		.cra_driver_name		=		"ltqdeu-aes",
		.cra_flags				=		CRYPTO_ALG_TYPE_CIPHER,
		.cra_blocksize			=		AES_BLOCK_SIZE,
		.cra_ctxsize			=		sizeof(struct aes_ctx),
		.cra_module				=		THIS_MODULE,
		.cra_list				=		LIST_HEAD_INIT(ltqdeu_aes_alg.cra_list),
		.cra_u					=		{
			.cipher = {
				.cia_min_keysize	=		AES_MIN_KEY_SIZE,
				.cia_max_keysize	=		AES_MAX_KEY_SIZE,
				.cia_setkey			=		aes_set_key,
				.cia_encrypt		=		aes_encrypt,
				.cia_decrypt		=		aes_decrypt,
			}
		}
};


/*! \fn int ecb_aes_encrypt(struct blkcipher_desc *desc,
 *			struct scatterlist *dst, struct scatterlist *src,
 *			unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief ECB AES encrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int ecb_aes_encrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int enc_bytes;
		unsigned long flag;

		set_aes_algo_status(AES_ECB_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = enc_bytes = walk.nbytes)) {
				enc_bytes -= (nbytes % AES_BLOCK_SIZE);
				CRTCL_SECT_START;
				powerup_deu(AES_INIT);
				ltq_deu_aes_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
									 NULL, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
				powerdown_deu(AES_INIT);
				CRTCL_SECT_END;
				nbytes &= AES_BLOCK_SIZE - 1;
				err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_aes_algo_status(AES_ECB_STATUS, CRYPTO_IDLE);
		return err;
}

/*! \fn int ecb_aes_decrypt(struct blkcipher_desc *desc,
 *				struct scatterlist *dst, struct scatterlist *src,
 *				unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief ECB AES decrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int ecb_aes_decrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int dec_bytes;
		unsigned long flag;

		set_aes_algo_status(AES_ECB_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = dec_bytes = walk.nbytes)) {
				dec_bytes -= (nbytes % AES_BLOCK_SIZE);
				CRTCL_SECT_START;
				powerup_deu(AES_INIT);
				ltq_deu_aes_ecb(ctx, walk.dst.virt.addr, walk.src.virt.addr,
									 NULL, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
				powerdown_deu(AES_INIT);
				CRTCL_SECT_END;
				nbytes &= AES_BLOCK_SIZE - 1;
				err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_aes_algo_status(AES_ECB_STATUS, CRYPTO_IDLE);
		return err;
}

/*
 * \brief AES function mappings
*/
struct crypto_alg ltqdeu_ecb_aes_alg = {
		.cra_name			=		"ecb(aes)",
		.cra_driver_name	=		"ltqdeu-ecb(aes)",
		.cra_flags			=		CRYPTO_ALG_TYPE_BLKCIPHER,
		.cra_blocksize		=		AES_BLOCK_SIZE,
		.cra_ctxsize		=		sizeof(struct aes_ctx),
		.cra_type			=		&crypto_blkcipher_type,
		.cra_module			=		THIS_MODULE,
		.cra_list			=		LIST_HEAD_INIT(ltqdeu_ecb_aes_alg.cra_list),
		.cra_u				=		{
				.blkcipher = {
						.min_keysize	=		AES_MIN_KEY_SIZE,
						.max_keysize	=		AES_MAX_KEY_SIZE,
						.setkey			=		aes_set_key,
						.encrypt		=		ecb_aes_encrypt,
						.decrypt		=		ecb_aes_decrypt,
				}
		}
};

/*! \fn int cbc_aes_encrypt(struct blkcipher_desc *desc,
 *				struct scatterlist *dst, struct scatterlist *src,
 *				unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief CBC AES encrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int cbc_aes_encrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int enc_bytes;
		unsigned long flag;

		set_aes_algo_status(AES_CBC_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = enc_bytes = walk.nbytes)) {
			u8 *iv = walk.iv;
			enc_bytes -= (nbytes % AES_BLOCK_SIZE);
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);
			ltq_deu_aes_cbc(ctx, walk.dst.virt.addr,
							walk.src.virt.addr,
							iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_aes_algo_status(AES_CBC_STATUS, CRYPTO_IDLE);
		return err;
}

/*! \fn int cbc_aes_decrypt(struct blkcipher_desc *desc,
 *				struct scatterlist *dst, struct scatterlist *src,
 *				unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief CBC AES decrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int cbc_aes_decrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int dec_bytes;
		unsigned long flag;

		set_aes_algo_status(AES_CBC_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = dec_bytes = walk.nbytes)) {
			u8 *iv = walk.iv;
			dec_bytes -= (nbytes % AES_BLOCK_SIZE);
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);
			ltq_deu_aes_cbc(ctx, walk.dst.virt.addr,
							walk.src.virt.addr,
							iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}


		set_aes_algo_status(AES_CBC_STATUS, CRYPTO_IDLE);
		return err;
}

/*
 * \brief AES function mappings
*/
struct crypto_alg ltqdeu_cbc_aes_alg = {
		.cra_name			=		"cbc(aes)",
		.cra_driver_name	=		"ltqdeu-cbc(aes)",
		.cra_flags			=		CRYPTO_ALG_TYPE_BLKCIPHER,
		.cra_blocksize		=		AES_BLOCK_SIZE,
		.cra_ctxsize		=		sizeof(struct aes_ctx),
		.cra_type			=		&crypto_blkcipher_type,
		.cra_module			=		THIS_MODULE,
		.cra_list			=		LIST_HEAD_INIT(ltqdeu_cbc_aes_alg.cra_list),
		.cra_u				=		{
				.blkcipher	= {
					.min_keysize	=		AES_MIN_KEY_SIZE,
					.max_keysize	=		AES_MAX_KEY_SIZE,
					.ivsize			=		AES_BLOCK_SIZE,
					.setkey			=		aes_set_key,
					.encrypt		=		cbc_aes_encrypt,
					.decrypt		=		cbc_aes_decrypt,
				}
		}
};

/*! \fn int ctr_basic_aes_encrypt(struct blkcipher_desc *desc,
 *				struct scatterlist *dst, struct scatterlist *src,
 *				unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief Counter mode AES encrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int ctr_basic_aes_encrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int enc_bytes;
		unsigned long flag;

		set_aes_algo_status(AES_CTR_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = enc_bytes = walk.nbytes)) {
			u8 *iv = walk.iv;
			enc_bytes -= (nbytes % AES_BLOCK_SIZE);
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);
			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr,
							walk.src.virt.addr,
							iv, enc_bytes, CRYPTO_DIR_ENCRYPT, 0);
			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_aes_algo_status(AES_CTR_STATUS, CRYPTO_IDLE);
		return err;
}

/*! \fn  int ctr_basic_aes_decrypt(struct blkcipher_desc *desc,
 *				struct scatterlist *dst, struct scatterlist *src,
 *				unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief Counter mode AES decrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int ctr_basic_aes_decrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err;
		unsigned int dec_bytes;
		unsigned long flag;

		set_aes_algo_status(AES_CTR_STATUS, CRYPTO_STARTED);
		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		while ((nbytes = dec_bytes = walk.nbytes)) {
			u8 *iv = walk.iv;
			dec_bytes -= (nbytes % AES_BLOCK_SIZE);
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);
			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr,
							walk.src.virt.addr,
							iv, dec_bytes, CRYPTO_DIR_DECRYPT, 0);
			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes &= AES_BLOCK_SIZE - 1;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		set_aes_algo_status(AES_CTR_STATUS, CRYPTO_IDLE);
		return err;
}

/*
 * \brief AES function mappings
*/
struct crypto_alg ltqdeu_ctr_basic_aes_alg = {
	.cra_name			=	"ctr(aes)",
	.cra_driver_name	=	"ltqdeu-ctr(aes)",
	.cra_flags			=	CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_blocksize		=	AES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct aes_ctx),
	.cra_type			=	&crypto_blkcipher_type,
	.cra_module			=	THIS_MODULE,
	.cra_list			=	LIST_HEAD_INIT(ltqdeu_ctr_basic_aes_alg.cra_list),
	.cra_u				=	{
		.blkcipher		= {
			.min_keysize	=		AES_MIN_KEY_SIZE,
			.max_keysize	=		AES_MAX_KEY_SIZE,
			.ivsize			=		AES_BLOCK_SIZE,
			.setkey			=		aes_set_key,
			.encrypt		=		ctr_basic_aes_encrypt,
			.decrypt		=		ctr_basic_aes_decrypt,
		}
	}
};

/*! \fn  int ctr_rfc3686_aes_encrypt(struct blkcipher_desc *desc,
 *				struct scatterlist *dst, struct scatterlist *src,
 *				unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief Counter mode AES (rfc3686) encrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int ctr_rfc3686_aes_encrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err, bsize = nbytes;
		u8 rfc3686_iv[16];
		unsigned long flag;

		set_aes_algo_status(AES_RFC3686_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE);
		memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, walk.iv,
						CTR_RFC3686_IV_SIZE);

		/* initialize counter portion of counter block */
		*(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE +
								CTR_RFC3686_IV_SIZE) =
				cpu_to_be32(1);

		/* scatterlist source is the same size as
		* request size, just process once
		*/
		if (nbytes == walk.nbytes) {
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);

			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
							 rfc3686_iv, nbytes, CRYPTO_DIR_ENCRYPT, 0);

			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes -= walk.nbytes;
			err = blkcipher_walk_done(desc, &walk, nbytes);
			goto rfc3686_enc_done;
		}

		while ((nbytes = walk.nbytes) &&
				(walk.nbytes >= AES_BLOCK_SIZE)) {

			CRTCL_SECT_START;
			powerup_deu(AES_INIT);

			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
							 rfc3686_iv, nbytes, CRYPTO_DIR_ENCRYPT, 0);

			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes -= walk.nbytes;
			bsize -= walk.nbytes;
			err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		/* to handle remaining bytes < AES_BLOCK_SIZE */
		if (walk.nbytes) {
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);

			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
							 rfc3686_iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);

			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;

			err = blkcipher_walk_done(desc, &walk, 0);
		}

rfc3686_enc_done:
		set_aes_algo_status(AES_RFC3686_STATUS, CRYPTO_IDLE);
		return err;
}


/*! \fn int ctr_rfc3686_aes_decrypt(struct blkcipher_desc *desc,
 *							struct scatterlist *dst, struct scatterlist *src,
 *							unsigned int nbytes)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief Counter mode AES (rfc3686) decrypt using linux crypto blkcipher
 *	\param desc blkcipher descriptor
 *	\param dst output scatterlist
 *	\param src input scatterlist
 *	\param nbytes data size in bytes
 *	\return err
*/
int ctr_rfc3686_aes_decrypt(struct blkcipher_desc *desc,
							 struct scatterlist *dst, struct scatterlist *src,
							 unsigned int nbytes)
{
		struct aes_ctx *ctx = crypto_blkcipher_ctx(desc->tfm);
		struct blkcipher_walk walk;
		int err, bsize = nbytes;
		u8 rfc3686_iv[16];
		unsigned long flag;

		set_aes_algo_status(AES_RFC3686_STATUS, CRYPTO_STARTED);

		blkcipher_walk_init(&walk, dst, src, nbytes);
		err = blkcipher_walk_virt(desc, &walk);

		memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE);
		memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, walk.iv,
							CTR_RFC3686_IV_SIZE);

		/* initialize counter portion of counter block */
		*(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE +
							CTR_RFC3686_IV_SIZE) =	cpu_to_be32(1);

		/* scatterlist source is the same size as request size,
		* just process once
		*/
		if (nbytes == walk.nbytes) {
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);
			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
							 rfc3686_iv, nbytes, CRYPTO_DIR_ENCRYPT, 0);

			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes -= walk.nbytes;
			err = blkcipher_walk_done(desc, &walk, nbytes);
			goto rfc3686_dec_done;
		}

		while ((nbytes = walk.nbytes) %
				(walk.nbytes >= AES_BLOCK_SIZE)) {
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);

			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
						 rfc3686_iv, nbytes, CRYPTO_DIR_DECRYPT, 0);

			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			nbytes -= walk.nbytes;
			bsize -= walk.nbytes;

			err = blkcipher_walk_done(desc, &walk, nbytes);
		}

		/* to handle remaining bytes < AES_BLOCK_SIZE */
		if (walk.nbytes) {
			CRTCL_SECT_START;
			powerup_deu(AES_INIT);
			ltq_deu_aes_ctr(ctx, walk.dst.virt.addr, walk.src.virt.addr,
						 rfc3686_iv, walk.nbytes, CRYPTO_DIR_ENCRYPT, 0);
			powerdown_deu(AES_INIT);
			CRTCL_SECT_END;
			err = blkcipher_walk_done(desc, &walk, 0);
		}

rfc3686_dec_done:
		set_aes_algo_status(AES_RFC3686_STATUS, CRYPTO_IDLE);
		return err;
}

/*
 * \brief AES function mappings
*/
struct crypto_alg ltqdeu_ctr_rfc3686_aes_alg = {
	.cra_name			=	"rfc3686(ctr(aes))",
	.cra_driver_name	=	"ltqdeu-ctr-rfc3686(aes)",
	.cra_flags			=	CRYPTO_ALG_TYPE_BLKCIPHER,
	.cra_blocksize		=	AES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct aes_ctx),
	.cra_type			=	&crypto_blkcipher_type,
	.cra_module			=	THIS_MODULE,
	.cra_list			=	LIST_HEAD_INIT(ltqdeu_ctr_rfc3686_aes_alg.cra_list),
	.cra_u				=	{
		.blkcipher = {
			.min_keysize		=		AES_MIN_KEY_SIZE,
			.max_keysize		=		CTR_RFC3686_MAX_KEY_SIZE,
			.ivsize				=		CTR_RFC3686_IV_SIZE,
			.setkey				=		ctr_rfc3686_aes_set_key,
			.encrypt			=		ctr_rfc3686_aes_encrypt,
			.decrypt			=		ctr_rfc3686_aes_decrypt,
		}
	}
};

void ltq_aes_toggle_algo(int mode)
{
	int ret = 0;

	if (mode) {
		crypto_unregister_alg(&ltqdeu_aes_alg);
		crypto_unregister_alg(&ltqdeu_ecb_aes_alg);
		crypto_unregister_alg(&ltqdeu_cbc_aes_alg);
		crypto_unregister_alg(&ltqdeu_ctr_basic_aes_alg);
		crypto_unregister_alg(&ltqdeu_ctr_rfc3686_aes_alg);


		ltqdeu_aes_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_ecb_aes_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_cbc_aes_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_ctr_basic_aes_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
		ltqdeu_ctr_rfc3686_aes_alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
	} else {
		ret = crypto_register_alg(&ltqdeu_aes_alg);
		if (ret)
			goto aes_err;
		ret = crypto_register_alg(&ltqdeu_ecb_aes_alg);
		if (ret)
			goto ecb_aes_err;
		ret = crypto_register_alg(&ltqdeu_cbc_aes_alg);
		if (ret)
			goto cbc_aes_err;
		ret = crypto_register_alg(&ltqdeu_ctr_basic_aes_alg);
		if (ret)
			goto ctr_basic_aes_err;
		ret = crypto_register_alg(&ltqdeu_ctr_rfc3686_aes_alg);
		if (ret)
			goto ctr_rfc3686_aes_err;
	}

		return;

ctr_rfc3686_aes_err:
		crypto_unregister_alg(&ltqdeu_ctr_rfc3686_aes_alg);
		pr_err("LTQ ctr_rfc3686_aes initialization failed!\n");
		return;
ctr_basic_aes_err:
		crypto_unregister_alg(&ltqdeu_ctr_basic_aes_alg);
		pr_err("LTQ ctr_basic_aes initialization failed!\n");
		return;
cbc_aes_err:
		crypto_unregister_alg(&ltqdeu_cbc_aes_alg);
		pr_err("LTQ cbc_aes initialization failed!\n");
		return;
ecb_aes_err:
		crypto_unregister_alg(&ltqdeu_ecb_aes_alg);
		pr_err("LTQ aes initialization failed!\n");
		return;
aes_err:
		pr_err("LTQ DEU AES initialization failed!\n");
}


/*! \fn int ltq_aes_init (void)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief function to initialize AES driver
 *	\return ret
*/
int ltq_aes_init(void)
{
		int i, ret = -ENOSYS;

		ret = crypto_register_alg(&ltqdeu_aes_alg);
		if (ret)
			goto aes_err;

		ret = crypto_register_alg(&ltqdeu_ecb_aes_alg);
		if (ret)
			goto ecb_aes_err;

		ret = crypto_register_alg(&ltqdeu_cbc_aes_alg);
		if (ret)
			goto cbc_aes_err;

		ret = crypto_register_alg(&ltqdeu_ctr_basic_aes_alg);
		if (ret)
			goto ctr_basic_aes_err;

		ret = crypto_register_alg(&ltqdeu_ctr_rfc3686_aes_alg);
		if (ret)
			goto ctr_rfc3686_aes_err;

		spin_lock_init(&power_lock);

		for (i = 0; i < MAX_AES_ALGO; i++)
				set_aes_algo_status(i, CRYPTO_IDLE);

		pr_notice("LTQ DEU AES initialized%s.\n",
				!algo_config.dma ? "" : " (DMA)");

		return ret;

ctr_rfc3686_aes_err:
		crypto_unregister_alg(&ltqdeu_ctr_rfc3686_aes_alg);
		pr_err("LTQ ctr_rfc3686_aes initialization failed!\n");
		return ret;
ctr_basic_aes_err:
		crypto_unregister_alg(&ltqdeu_ctr_basic_aes_alg);
		pr_err("LTQ ctr_basic_aes initialization failed!\n");
		return ret;
cbc_aes_err:
		crypto_unregister_alg(&ltqdeu_cbc_aes_alg);
		pr_err("LTQ cbc_aes initialization failed!\n");
		return ret;
ecb_aes_err:
		crypto_unregister_alg(&ltqdeu_ecb_aes_alg);
		pr_err("LTQ aes initialization failed!\n");
		return ret;
aes_err:
		pr_err("LTQ DEU AES initialization failed!\n");

		return ret;
}

/*! \fn void ltq_aes_fini(void)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief unregister aes driver
*/
void ltq_aes_fini(void)
{
	crypto_unregister_alg (&ltqdeu_aes_alg);
	crypto_unregister_alg (&ltqdeu_ecb_aes_alg);
	crypto_unregister_alg (&ltqdeu_cbc_aes_alg);
	crypto_unregister_alg (&ltqdeu_ctr_basic_aes_alg);
	crypto_unregister_alg (&ltqdeu_ctr_rfc3686_aes_alg);

}
