/******************************************************************************
**
** FILE NAME	: ltqmips_sha1.c
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
  \file	ltqmips_sha1.c
  \ingroup LTQ_DEU
  \brief SHA1 encryption deu driver file
*/

/*!
  \defgroup LTQ_SHA1_FUNCTIONS LTQ_SHA1_FUNCTIONS
  \ingroup LTQ_DEU
  \brief ltq deu sha1 functions
*/


/* Project header */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <crypto/sha.h>
#include <crypto/internal/hash.h>
#include <linux/types.h>
#include <asm/scatterlist.h>
#include <asm/byteorder.h>
#include <lantiq_soc.h>
#include <irq.h>

#include "ltqmips_deu.h"

#define SHA1_DIGEST_SIZE		20
#define SHA1_HMAC_BLOCK_SIZE	64
#define MAX_SHA1_ALGO 			1
#define SHA1_STATUS   			0

static int sha1_algo_status;
extern struct ltq_deu_config algo_config;
/*
 * \brief SHA1 private structure
*/
struct sha1_ctx {
	int started;
	u64 count;
	u32 hash[5];
	u32 state[5];
	u8 buffer[64];
};

void chip_version(void);
void powerup_deu(int crypto);
void powerdown_deu(int crypto);

void set_sha1_algo_status(unsigned int sha1_algo, int cmd)
{
	sha1_algo_status = cmd;
}

int read_sha1_algo_status(void)
{
	int status;

	status = sha1_algo_status;
	return status;
}

/*! \fn static void sha1_transform (u32 *state, const u32 *in)
 *	\ingroup LTQ_SHA1_FUNCTIONS
 *	\brief main interface to sha1 hardware
 *	\param state current state
 *	\param in 64-byte block of input
*/
static void sha1_transform(struct sha1_ctx *sctx, u32 *state, const u32 *in)
{
	int i = 0;
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;

	set_sha1_algo_status(SHA1_STATUS, CRYPTO_STARTED);
	CRTCL_SECT_START;
	powerup_deu(SHA1_INIT);

	/* For context switching purposes, the previous hash output
	 * is loaded back into the output register
	*/
	if (sctx->started) {
		hashs->D1R = *((u32 *) sctx->hash + 0);
		hashs->D2R = *((u32 *) sctx->hash + 1);
		hashs->D3R = *((u32 *) sctx->hash + 2);
		hashs->D4R = *((u32 *) sctx->hash + 3);
		hashs->D5R = *((u32 *) sctx->hash + 4);
	}

	for (i = 0; i < 16; i++) {
		hashs->MR = in[i];
	};

	while (hashs->controlr.BSY) {
	}

	/* For context switching purposes, the output is saved into a
	 * context struct which can be used later on
	*/
	*((u32 *) sctx->hash + 0) = hashs->D1R;
	*((u32 *) sctx->hash + 1) = hashs->D2R;
	*((u32 *) sctx->hash + 2) = hashs->D3R;
	*((u32 *) sctx->hash + 3) = hashs->D4R;
	*((u32 *) sctx->hash + 4) = hashs->D5R;

	sctx->started = 1;

	powerdown_deu(SHA1_INIT);
	CRTCL_SECT_END;
	set_sha1_algo_status(SHA1_STATUS, CRYPTO_IDLE);
}

/*! \fn static void sha1_init(struct crypto_tfm *tfm)
 *	\ingroup LTQ_SHA1_FUNCTIONS
 *	\brief initialize sha1 hardware
 *	\param tfm linux crypto algo transform
*/
static int sha1_init(struct shash_desc *desc)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);
	unsigned long flag;

	CRTCL_SECT_START;
	SHA_HASH_INIT;
	CRTCL_SECT_END;

	sctx->started = 0;
	sctx->count = 0;
	return 0;
}

/*! \fn static void sha1_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *	\ingroup LTQ_SHA1_FUNCTIONS
 *	\brief on-the-fly sha1 computation
 *	\param tfm linux crypto algo transform
 *	\param data input data
 *	\param len size of input data
*/
static int sha1_update(struct shash_desc *desc, const u8 *data,
			unsigned int len)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);
	unsigned int i, j;

	j = (sctx->count >> 3) & 0x3f;
	sctx->count += len << 3;

	if ((j + len) > 63) {
		memcpy(&sctx->buffer[j], data, (i = 64 - j));
		sha1_transform(sctx, sctx->state, (const u32 *)sctx->buffer);
		for (; i + 63 < len; i += 64) {
			sha1_transform(sctx, sctx->state, (const u32 *)&data[i]);
		}
		j = 0;
	} else
		i = 0;

	memcpy(&sctx->buffer[j], &data[i], len - i);
	return 0;
}

/*! \fn static void sha1_final(struct crypto_tfm *tfm, u8 *out)
 *	\ingroup LTQ_SHA1_FUNCTIONS
 *	\brief compute final sha1 value
 *	\param tfm linux crypto algo transform
 *	\param out final md5 output value
*/
static int sha1_final(struct shash_desc *desc, u8 *out)
{
	struct sha1_ctx *sctx = shash_desc_ctx(desc);
	u32 index, padlen;
	u64 t;
	u8 bits[8] = { 0, };
	static const u8 padding[64] = { 0x80, };
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;

	t = sctx->count;
	bits[7] = 0xff & t;
	t >>= 8;
	bits[6] = 0xff & t;
	t >>= 8;
	bits[5] = 0xff & t;
	t >>= 8;
	bits[4] = 0xff & t;
	t >>= 8;
	bits[3] = 0xff & t;
	t >>= 8;
	bits[2] = 0xff & t;
	t >>= 8;
	bits[1] = 0xff & t;
	t >>= 8;
	bits[0] = 0xff & t;

	/* Pad out to 56 mod 64 */
	index = (sctx->count >> 3) & 0x3f;
	padlen = (index < 56) ? (56 - index) : ((64 + 56) - index);
	sha1_update (desc, padding, padlen);

	/* Append length */
	sha1_update(desc, bits, sizeof bits);

	CRTCL_SECT_START;

	*((u32 *) out + 0) = hashs->D1R;
	*((u32 *) out + 1) = hashs->D2R;
	*((u32 *) out + 2) = hashs->D3R;
	*((u32 *) out + 3) = hashs->D4R;
	*((u32 *) out + 4) = hashs->D5R;

	CRTCL_SECT_END;

	/* Wipe Context */
	memset (sctx, 0, sizeof *sctx);

	return 0;
}

/*
 * \brief SHA1 function mappings
*/
static struct shash_alg ltqdeu_sha1_alg = {
	.digestsize		=		SHA1_DIGEST_SIZE,
	.init			=		sha1_init,
	.update			=		sha1_update,
	.final			=		sha1_final,
	.descsize		=		sizeof(struct sha1_ctx),
	.statesize		=		sizeof(struct sha1_state),
	.base			=		{
		.cra_name			=		"sha1",
		.cra_driver_name	=		"ltqdeu-sha1",
		.cra_flags			=		CRYPTO_ALG_TYPE_DIGEST,
		.cra_blocksize		=		SHA1_HMAC_BLOCK_SIZE,
		.cra_module			=		THIS_MODULE,
	}
};

void ltq_sha1_toggle_algo(int mode)
{
	int ret = 0;
	if (mode) {
		crypto_unregister_shash(&ltqdeu_sha1_alg);
		ltqdeu_sha1_alg.base.cra_flags = CRYPTO_ALG_TYPE_DIGEST;
	} else {
		ret = crypto_register_shash(&ltqdeu_sha1_alg);
		if (ret < 0) {
			pr_err("Error registering for sha1 hash alg\n");
			crypto_unregister_shash(&ltqdeu_sha1_alg);
		}
	}

}

/*! \fn int __init ltq_sha1_init (void)
 *	\ingroup LTQ_SHA1_FUNCTIONS
 *	\brief initialize sha1 driver
*/
int __init ltq_sha1_init(void)
{
	int ret = -ENOSYS;

	ret = crypto_register_shash(&ltqdeu_sha1_alg);
	if (ret)
		goto sha1_err;

	set_sha1_algo_status(SHA1_STATUS, CRYPTO_IDLE);

	pr_notice("LTQ DEU SHA1 initialized\n");
	return ret;

sha1_err:
	pr_err("LTQ DEU SHA1 initialization failed!\n");
	return ret;
}

/*! \fn void __exit ltqdeu_fini_sha1 (void)
 *	\ingroup LTQ_SHA1_FUNCTIONS
 *	\brief unregister sha1 driver
*/
void __exit ltq_sha1_fini(void)
{
	crypto_unregister_shash(&ltqdeu_sha1_alg);
}


