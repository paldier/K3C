/******************************************************************************
**
** FILE NAME	: ltqmips_md5.c
** PROJECT		: LTQ UEIP
** MODULES		: DEU Module for UEIP
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
  \defgroup    LTQ_DEU LTQ_DEU_DRIVERS
  \ingroup API
  \brief ltq deu driver module
*/

/*!
  \file		ltqmips_md5.c
  \ingroup	LTQ_DEU
  \brief	MD5 encryption deu driver file
*/

/*!
  \defgroup LTQ_MD5_FUNCTIONS LTQ_MD5_FUNCTIONS
  \ingroup LTQ_DEU
  \brief ltq deu MD5 functions
*/

/*Project header files */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/types.h>
#include <crypto/internal/hash.h>
#include <asm/byteorder.h>

/* Project header */
#include <lantiq_soc.h>
#include <irq.h>
#include "ltqmips_deu.h"

#define MD5_DIGEST_SIZE		16
#define MD5_HMAC_BLOCK_SIZE 64
#define MD5_BLOCK_WORDS		16
#define MD5_HASH_WORDS		4
#define MAX_MD5_ALGO 1
#define MD5_STATUS	 0

static int md5_algo_status;

struct md5_ctx {
	int started;
	u32 hash[MD5_HASH_WORDS];
	u32 block[MD5_BLOCK_WORDS];
	u64 byte_count;
};

void chip_version(void);
void powerup_deu(int crypto);
void powerdown_deu(int crypto);

void set_md5_algo_status(unsigned int md5_algo, int cmd)
{
	md5_algo_status = cmd;
}

int read_md5_algo_status(void)
{
	int status;

	status = md5_algo_status;

	return status;
}

void md5_init_registers(void)
{
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;

	hashs->controlr.ENDI = 0;
	hashs->controlr.SM = 1;
	/* 1 = md5	0 = sha1 */
	hashs->controlr.ALGO = 1;
	/*	Initialize the hash operation by writing a '1' to the INIT bit. */
	hashs->controlr.INIT = 1;
}

/*! \fn static u32 endian_swap(u32 input)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief perform dword level endian swap
 *	\param input value of dword that requires to be swapped
*/
static u32 endian_swap(u32 input)
{
	u8 *ptr = (u8 *)&input;

	return (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
}

/*! \fn static void md5_transform(u32 *hash, u32 const *in)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief main interface to md5 hardware
 *	\param hash current hash value
 *	\param in 64-byte block of input
*/
static void md5_transform(struct md5_ctx *mctx, u32 *hash, u32 const *in)
{
	int i;
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;

	set_md5_algo_status(MD5_STATUS, CRYPTO_STARTED);

	CRTCL_SECT_START;
	powerup_deu(MD5_INIT);

	if (mctx->started) {
		hashs->D1R = endian_swap(*((u32 *) hash + 0));
		hashs->D2R = endian_swap(*((u32 *) hash + 1));
		hashs->D3R = endian_swap(*((u32 *) hash + 2));
		hashs->D4R = endian_swap(*((u32 *) hash + 3));
	}

	for (i = 0; i < 16; i++) {
		hashs->MR = endian_swap(in[i]);
	/*	printk("in[%d]: %08x\n", i, endian_swap(in[i])); */
	};

	while (hashs->controlr.BSY)
		;

	*((u32 *) hash + 0) = endian_swap(hashs->D1R);
	*((u32 *) hash + 1) = endian_swap(hashs->D2R);
	*((u32 *) hash + 2) = endian_swap(hashs->D3R);
	*((u32 *) hash + 3) = endian_swap(hashs->D4R);

	mctx->started = 1;

	powerdown_deu(MD5_INIT);
	CRTCL_SECT_END;

	set_md5_algo_status(MD5_STATUS, CRYPTO_IDLE);
}

/*! \fn static inline void md5_transform_helper(struct md5_ctx *ctx)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief interfacing function for md5_transform()
 *	\param ctx crypto context
*/
static inline void md5_transform_helper(struct md5_ctx *ctx)
{
	md5_transform(ctx, ctx->hash, ctx->block);
}

/*! \fn static int md5_init(struct crypto_tfm *tfm)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief initialize md5 hardware
 *	\param tfm linux crypto algo transform
*/
static int md5_init(struct shash_desc *desc)
{
	struct md5_ctx *mctx = shash_desc_ctx(desc);
	unsigned long flag;

	CRTCL_SECT_START;

	md5_init_registers();
	mctx->byte_count = 0;
	mctx->started = 0;

	CRTCL_SECT_END;
	return 0;
}

/*! \fn static void md5_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief on-the-fly md5 computation
 *	\param tfm linux crypto algo transform
 *	\param data input data
 *	\param len size of input data
*/
static int md5_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
	struct md5_ctx *mctx = shash_desc_ctx(desc);
	const u32 avail = sizeof(mctx->block) - (mctx->byte_count & 0x3f);

	mctx->byte_count += len;

	if (avail > len) {
		memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
			   data, len);
		return 0;
	}

	memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
		   data, avail);

	md5_transform_helper(mctx);
	data += avail;
	len -= avail;

	while (len >= sizeof(mctx->block)) {
		memcpy(mctx->block, data, sizeof(mctx->block));
		md5_transform_helper(mctx);
		data += sizeof(mctx->block);
		len -= sizeof(mctx->block);
	}

	memcpy(mctx->block, data, len);
	return 0;
}

/*! \fn static void md5_final(struct crypto_tfm *tfm, u8 *out)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief compute final md5 value
 *	\param tfm linux crypto algo transform
 *	\param out final md5 output value
*/
static int md5_final(struct shash_desc *desc, u8 *out)
{
	struct md5_ctx *mctx = shash_desc_ctx(desc);
	const unsigned int offset = mctx->byte_count & 0x3f;
	char *p = (char *)mctx->block + offset;
	int padding = 56 - (offset + 1);
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;

	*p++ = 0x80;
	if (padding < 0) {
		memset(p, 0x00, padding + sizeof (u64));
		md5_transform_helper(mctx);
		p = (char *)mctx->block;
		padding = 56;
	}

	memset(p, 0, padding);
	mctx->block[14] = endian_swap(mctx->byte_count << 3);
	mctx->block[15] = endian_swap(mctx->byte_count >> 29);

	md5_transform(mctx, mctx->hash, mctx->block);

	CRTCL_SECT_START;

	*((u32 *) out + 0) = endian_swap(hashs->D1R);
	*((u32 *) out + 1) = endian_swap(hashs->D2R);
	*((u32 *) out + 2) = endian_swap(hashs->D3R);
	*((u32 *) out + 3) = endian_swap(hashs->D4R);

	CRTCL_SECT_END;

	/* Wipe context */
	memset(mctx, 0, sizeof(*mctx));

	return 0;
}

/*
 * \brief MD5 function mappings
*/
static struct shash_alg ltqdeu_md5_alg = {
	.digestsize			=		MD5_DIGEST_SIZE,
	.init				=		md5_init,
	.update				=		md5_update,
	.final				=		md5_final,
	.descsize			=		sizeof(struct md5_ctx),
	.base				=		{
		.cra_name			=		"md5",
		.cra_driver_name	=		"ltqdeu-md5",
		.cra_flags			=		CRYPTO_ALG_TYPE_DIGEST,
		.cra_blocksize		=		MD5_HMAC_BLOCK_SIZE,
		.cra_module			=		THIS_MODULE,
	}
};

void ltq_md5_toggle_algo(int mode)
{
	if (mode) {
		crypto_unregister_shash(&ltqdeu_md5_alg);
		ltqdeu_md5_alg.base.cra_flags = CRYPTO_ALG_TYPE_DIGEST;
	} else
		crypto_register_shash(&ltqdeu_md5_alg);
}


/*! \fn int __init ltq_md5_init(void)
 *	\ingroup LTQ_MD5_FUNCTIONS
 *	\brief initialize md5 driver
*/
int __init ltq_md5_init(void)
{
	int ret = -ENOSYS;

	ret = crypto_register_shash(&ltqdeu_md5_alg);
	if (ret)
		goto md5_err;

	set_md5_algo_status(MD5_STATUS, CRYPTO_IDLE);

	pr_notice("LTQ DEU MD5 initialized \n");
	return ret;

md5_err:
	pr_err("LTQ DEU MD5 initialization failed!\n");
	return ret;
}

/*! \fn void __exit ltqdeu_fini_md5 (void)
  * \ingroup LTQ_MD5_FUNCTIONS
  * \brief unregister md5 driver
*/

void __exit ltq_md5_fini(void)
{
	crypto_unregister_shash(&ltqdeu_md5_alg);
}

