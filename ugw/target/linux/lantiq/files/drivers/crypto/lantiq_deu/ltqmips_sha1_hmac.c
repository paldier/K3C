/******************************************************************************
**
** FILE NAME	: ltqmips_sha1_hmac.c
** PROJECT		: LTQ UEIP
** MODULES		: DEU Module for UEIP
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
** 21,March 2011 Mohammad Firdaus	Changes for Kernel 2.6.32 and IPSec integration
*******************************************************************************/
/*!
  \defgroup LTQ_DEU LTQ_DEU_DRIVERS
  \ingroup API
  \brief ltq deu driver module
*/

/*!
  \file	ltqmips_sha1_hmac.c
  \ingroup LTQ_DEU
  \brief SHA1-HMAC deu driver file
*/

/*!
  \defgroup LTQ_SHA1_HMAC_FUNCTIONS LTQ_SHA1_HMAC_FUNCTIONS
  \ingroup LTQ_DEU
  \brief ltq sha1 hmac functions
*/


/* Project header */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/crypto.h>
#include <linux/cryptohash.h>
#include <crypto/internal/hash.h>
#include <linux/types.h>
#include <asm/scatterlist.h>
#include <asm/byteorder.h>
#include <linux/delay.h>
#include <lantiq_soc.h>
#include <irq.h>
#include "ltqmips_deu.h"

#define SHA1_DIGEST_SIZE			20
#define SHA1_HMAC_BLOCK_SIZE		64
#define SHA1_HMAC_DBN_TEMP_SIZE		1024
#define SHA1_HMAC_MAX_KEYLEN		64
#define MAX_SHA1_HMAC_ALGO			1
#define SHA1_HMAC_STATUS			0

static int algo_status[MAX_SHA1_HMAC_ALGO];

struct sha1_hmac_ctx {
	int keylen;
	u8 buffer[SHA1_HMAC_BLOCK_SIZE];
	u8 key[SHA1_HMAC_MAX_KEYLEN];
	u32 state[5];
	u32 dbn;
	u64 count;
	u32 temp[SHA1_HMAC_DBN_TEMP_SIZE];

};

void chip_version(void);
void powerup_deu(int crypto);
void powerdown_deu(int crypto);
static spinlock_t power_lock;

void set_sha1_hmac_algo_status(unsigned int sha1_hmac_algo, int cmd)
{
	unsigned long flag;

	if (sha1_hmac_algo >= MAX_SHA1_HMAC_ALGO) {
		pr_err("algo choice error!!\n");
		return;
	}

	spin_lock_irqsave(&power_lock, flag);
	algo_status[sha1_hmac_algo] = cmd;
	spin_unlock_irqrestore(&power_lock, flag);
}

int read_sha1_hmac_algo_status(void)
{
	int i;
	unsigned long flag;

	spin_lock_irqsave(&power_lock, flag);
	for (i = 0; i <= MAX_SHA1_HMAC_ALGO; i++) {
		if (algo_status[i] != CRYPTO_IDLE) {
			spin_unlock_irqrestore(&power_lock, flag);
			return CRYPTO_STARTED;
		 }
	}
	spin_unlock_irqrestore(&power_lock, flag);
	return CRYPTO_IDLE;
}


/*! \fn static int sha1_hmac_transform(struct crypto_tfm *tfm,
 *							u32 const *in)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief save input block to context
 *	\param tfm linux crypto algo transform
 *	\param in 64-byte block of input
*/
static int sha1_hmac_transform(struct shash_desc *desc,
							u32 const *in)
{
	struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);
	/* dbn workaround */
	memcpy(&sctx->temp[sctx->dbn << 4], in, 64);
	sctx->dbn += 1;

	if ((sctx->dbn<<4) > SHA1_HMAC_DBN_TEMP_SIZE)
		pr_err("SHA1_HMAC_DBN_TEMP_SIZE exceeded\n");

	return 0;
}

/*! \fn int sha1_hmac_setkey(struct crypto_tfm *tfm,
 *					const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief sets sha1 hmac key
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length greater than 64 bytes IS NOT SUPPORTED
*/
static int sha1_hmac_setkey(struct crypto_shash *tfm,
					const u8 *key, unsigned int keylen)
{
	unsigned long flag;
	struct sha1_hmac_ctx *sctx = crypto_shash_ctx(tfm);
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;

	if (keylen > SHA1_HMAC_MAX_KEYLEN) {
		pr_err("Key length exceeds maximum key length\n");
		return -EINVAL;
	}

	set_sha1_hmac_algo_status(SHA1_HMAC_STATUS, CRYPTO_STARTED);
	CRTCL_SECT_START;
	powerup_deu(SHA1_HMAC_INIT);
	/* reset keys back to 0 */
	hashs->KIDX |= 0x80000000;
	CRTCL_SECT_END;

	memcpy(&sctx->key, key, keylen);
	sctx->keylen = keylen;

	return 0;
}


/*! \fn int sha1_hmac_setkey_hw(struct crypto_tfm *tfm, const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief sets sha1 hmac key  into hw registers
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length greater than 64 bytes IS NOT SUPPORTED
*/
static int sha1_hmac_setkey_hw(const u8 *key, unsigned int keylen)
{
	volatile struct deu_hash_t *hash = (struct deu_hash_t *) LTQ_HASH_CON;
	int i, j;
	unsigned long flag;
	u32 *in_key = (u32 *)key;

	j = 0;

	set_sha1_hmac_algo_status(SHA1_HMAC_STATUS, CRYPTO_STARTED);
	CRTCL_SECT_START;
	powerup_deu(SHA1_HMAC_INIT);

	for (i = 0; i < keylen; i += 4) {
		hash->KIDX = j;
		asm("sync");
		hash->KEY = *((u32 *) in_key + j);
		j++;
	}

	CRTCL_SECT_END;
	return 0;
}

/*! \fn void sha1_hmac_init(struct crypto_tfm *tfm)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief initialize sha1 hmac context
 *	\param tfm linux crypto algo transform
*/
static int sha1_hmac_init(struct shash_desc *desc)
{
	struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);

	sctx->dbn = 0;
	sha1_hmac_setkey_hw(sctx->key, sctx->keylen);

	return 0;
}

/*! \fn static void sha1_hmac_update(struct crypto_tfm *tfm, const u8 *data, unsigned int len)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief on-the-fly sha1 hmac computation
 *	\param tfm linux crypto algo transform
 *	\param data input data
 *	\param len size of input data
*/
static int sha1_hmac_update(struct shash_desc *desc, const u8 *data,
			unsigned int len)
{
	struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);
	unsigned int i, j;

	j = (sctx->count >> 3) & 0x3f;
	sctx->count += len << 3;

   /* printk("sctx->count = %d\n", sctx->count); */

	if ((j + len) > 63) {
		memcpy(&sctx->buffer[j], data, (i = 64 - j));
		sha1_hmac_transform(desc, (const u32 *)sctx->buffer);

		for (; i + 63 < len; i += 64)
			sha1_hmac_transform(desc, (const u32 *)&data[i]);

		j = 0;
	} else
		i = 0;

	memcpy(&sctx->buffer[j], &data[i], len - i);
	return 0;
}

/*! \fn static void sha1_hmac_final(struct crypto_tfm *tfm, u8 *out)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief ompute final sha1 hmac value
 *	\param tfm linux crypto algo transform
 *	\param out final sha1 hmac output value
*/
static int sha1_hmac_final(struct shash_desc *desc, u8 *out)
{
	struct sha1_hmac_ctx *sctx =  crypto_shash_ctx(desc->tfm);
	u32 index, padlen;
	u64 t;
	u8 bits[8] = { 0, };
	static const u8 padding[64] = { 0x80, };
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;
	int i = 0;
	int dbn;
	u32 *in = &sctx->temp[0];

	/* need to add 512 bit of the IPAD operation */
	t = sctx->count + 512;
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
	sha1_hmac_update(desc, padding, padlen);

	/* Append length */
	sha1_hmac_update(desc, bits, sizeof(bits));

	set_sha1_hmac_algo_status(SHA1_HMAC_STATUS, CRYPTO_STARTED);

	CRTCL_SECT_START;
	powerup_deu(SHA1_HMAC_INIT);

	hashs->DBN = sctx->dbn;

	ltq_deu_w32(HASH_CON_VALUE, LTQ_DEU_HASH_CON);

	while (hashs->controlr.BSY)
		;

	for (dbn = 0; dbn < sctx->dbn; dbn++) {
		for (i = 0; i < 16; i++)
			hashs->MR = in[i];

		hashs->controlr.GO = 1;
		asm("sync");

		while (hashs->controlr.BSY)
			;

		in += 16;
	}

	while (!hashs->controlr.DGRY)
		;

	*((u32 *) out + 0) = hashs->D1R;
	*((u32 *) out + 1) = hashs->D2R;
	*((u32 *) out + 2) = hashs->D3R;
	*((u32 *) out + 3) = hashs->D4R;
	*((u32 *) out + 4) = hashs->D5R;

	powerdown_deu(SHA1_HMAC_INIT);
	CRTCL_SECT_END;

	memset(&sctx->buffer[0], 0, SHA1_HMAC_BLOCK_SIZE);
	sctx->count = 0;

	set_sha1_hmac_algo_status(SHA1_HMAC_STATUS, CRYPTO_IDLE);

	return 0;

}

/*
 * \brief SHA1-HMAC function mappings
*/
static struct shash_alg ltqdeu_sha1_hmac_alg = {
	.digestsize		=		SHA1_DIGEST_SIZE,
	.init			=		sha1_hmac_init,
	.update			=		sha1_hmac_update,
	.final			=		sha1_hmac_final,
	.setkey			=		sha1_hmac_setkey,
	.base			=		{
		.cra_name			=		"hmac(sha1)",
		.cra_driver_name	=		"ltqdeu-sha1_hmac",
		.cra_ctxsize		=	sizeof(struct sha1_hmac_ctx),
		.cra_flags			=		CRYPTO_ALG_TYPE_DIGEST,
		.cra_blocksize		=		SHA1_HMAC_BLOCK_SIZE,
		.cra_module			=		THIS_MODULE,
	}
};

void ltq_sha1_hmac_toggle_algo(int mode)
{
	if (mode) {
		crypto_unregister_shash(&ltqdeu_sha1_hmac_alg);
		ltqdeu_sha1_hmac_alg.base.cra_flags = CRYPTO_ALG_TYPE_DIGEST;
	} else
		crypto_register_shash(&ltqdeu_sha1_hmac_alg);
}

/*! \fn int __init ltq_sha1_hmac_init(void)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief initialize sha1 hmac driver
*/
int __init ltq_sha1_hmac_init(void)
{
	int i, ret = -ENOSYS;

	ret = crypto_register_shash(&ltqdeu_sha1_hmac_alg);
	if (ret)
		goto sha1_err;

	spin_lock_init(&power_lock);

	for (i = 0; i < MAX_SHA1_HMAC_ALGO; i++)
		set_sha1_hmac_algo_status(i, CRYPTO_IDLE);

	pr_notice("LTQ DEU SHA1_HMAC initialized\n");
	return ret;

sha1_err:
	pr_err("LTQ DEU SHA1_HMAC initialization failed!\n");
	return ret;
}

/*! \fn void __exit ltqdeu_fini_sha1_hmac (void)
 *	\ingroup LTQ_SHA1_HMAC_FUNCTIONS
 *	\brief unregister sha1 hmac driver
*/
void __exit ltq_sha1_hmac_fini(void)
{
	crypto_unregister_shash(&ltqdeu_sha1_hmac_alg);
}

