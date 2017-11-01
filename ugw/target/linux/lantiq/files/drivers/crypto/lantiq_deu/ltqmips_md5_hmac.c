/******************************************************************************
**
** FILE NAME	: ltqmips_md5_hmac.c
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
** 21,March 2011 Mohammad Firdaus	Changes for Kernel 2.6.32 and IPSec integration
*******************************************************************************/
/*!
  \defgroup LTQ_DEU LTQ_DEU_DRIVERS
  \ingroup API
  \brief  ltq deu driver module
*/

/*!
  \file	ltqmips_md5_hmac.c
  \ingroup LTQ_DEU
  \brief MD5-HMAC encryption deu driver file
*/

/*!
 \defgroup LTQ_MD5_HMAC_FUNCTIONS LTQ_MD5_HMAC_FUNCTIONS
 \ingroup LTQ_DEU
 \brief ltq md5-hmac driver functions
*/

/* Project Header files */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/types.h>
#include <crypto/internal/hash.h>
#include <asm/byteorder.h>
#include <lantiq_soc.h>
#include <irq.h>
#include "ltqmips_deu.h"

#define MD5_DIGEST_SIZE			16
#define MD5_HMAC_BLOCK_SIZE 	64
#define MD5_BLOCK_WORDS			16
#define MD5_HASH_WORDS			4
#define MD5_HMAC_DBN_TEMP_SIZE	1024
#define MAX_MD5_HMAC_ALGO 		1
#define MD5_HMAC_STATUS   		0

static int algo_status[MAX_MD5_HMAC_ALGO];

#define MAX_HASH_KEYLEN 64

struct md5_hmac_ctx {
	u8 key[MAX_HASH_KEYLEN];
	u32 hash[MD5_HASH_WORDS];
	u32 block[MD5_BLOCK_WORDS];
	u64 byte_count;
	u32 dbn;
	u32 temp[MD5_HMAC_DBN_TEMP_SIZE];
	unsigned int keylen;
};

void chip_version(void);
void powerup_deu(int crypto);
void powerdown_deu(int crypto);

static spinlock_t power_lock;

void set_md5_hmac_algo_status(unsigned int md5_hmac_algo, int cmd)
{
	unsigned long flag;

	if (md5_hmac_algo >= MAX_MD5_HMAC_ALGO) {
		pr_err("algo choice error!!\n");
		return;
	}

	spin_lock_irqsave(&power_lock, flag);
	algo_status[md5_hmac_algo] = cmd;
	spin_unlock_irqrestore(&power_lock, flag);
}

int read_md5_hmac_algo_status(void)
{
	int i;
	unsigned long flag;

	spin_lock_irqsave(&power_lock, flag);

	for (i = 0; i <= MAX_MD5_HMAC_ALGO; i++) {
		if (algo_status[i] != CRYPTO_IDLE) {
			spin_unlock_irqrestore(&power_lock, flag);
			return CRYPTO_STARTED;
		 }
	}
	spin_unlock_irqrestore(&power_lock, flag);
	return CRYPTO_IDLE;
}


/*! \fn static u32 endian_swap(u32 input)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief perform dword level endian swap
 *	\param input value of dword that requires to be swapped
*/
static u32 endian_swap(u32 input)
{
	u8 *ptr = (u8 *)&input;
	return (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
}

/*! \fn static void md5_hmac_transform(struct crypto_tfm *tfm, u32 const *in)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief save input block to context
 *	\param tfm linux crypto algo transform
 *	\param in 64-byte block of input
*/
static void md5_hmac_transform(struct shash_desc *desc, u32 const *in)
{
	struct md5_hmac_ctx *mctx = crypto_shash_ctx(desc->tfm);

	/* dbn workaround */
	memcpy(&mctx->temp[mctx->dbn << 4], in, 64);
	mctx->dbn += 1;

	if ((mctx->dbn << 4) > MD5_HMAC_DBN_TEMP_SIZE) {
		pr_err("MD5_HMAC_DBN_TEMP_SIZE exceeded, %d\n",
			(mctx->dbn << 4));
	}

}

/*! \fn int md5_hmac_setkey(struct crypto_tfm *tfm,
 *					const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief sets md5 hmac key
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length greater than 64 bytes IS NOT SUPPORTED
*/
static int md5_hmac_setkey(struct crypto_shash *tfm,
				const u8 *key, unsigned int keylen)
{
	struct md5_hmac_ctx *mctx = crypto_shash_ctx(tfm);
	volatile struct deu_hash_t *hash = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;

	if (keylen > MAX_HASH_KEYLEN) {
		pr_err("Key length more than what DEU hash can handle\n");
		return -EINVAL;
	}

	set_md5_hmac_algo_status(MD5_HMAC_STATUS, CRYPTO_IDLE);
	powerup_deu(MD5_HMAC_INIT);

	CRTCL_SECT_START;
	/* reset all 16 words of the key to '0' */
	hash->KIDX |= 0x80000000;
	CRTCL_SECT_END;

	memcpy(&mctx->key, key, keylen);
	mctx->keylen = keylen;

	return 0;
}


/*! \fn int md5_hmac_setkey_hw(const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief sets md5 hmac key into the hardware registers
 *	\param key input key
 *	\param keylen key length greater than 64 bytes IS NOT SUPPORTED
*/

static int md5_hmac_setkey_hw(const u8 *key, unsigned int keylen)
{
	volatile struct deu_hash_t *hash = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;
	int i, j;
	u32 *in_key = (u32 *)key;

	/* printk("\nsetkey keylen: %d\n key: ", keylen); */

	CRTCL_SECT_START;
	j = 0;
	for (i = 0; i < keylen; i += 4) {
		hash->KIDX = j;
		asm("sync");
		hash->KEY = *((u32 *) in_key + j);
		asm("sync");
		j++;
	}
	CRTCL_SECT_END;

	return 0;
}

/*! \fn void md5_hmac_init(struct crypto_tfm *tfm)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief initialize md5 hmac context
 *	\param tfm linux crypto algo transform
*/
static int md5_hmac_init(struct shash_desc *desc)
{

	struct md5_hmac_ctx *mctx = crypto_shash_ctx(desc->tfm);

	set_md5_hmac_algo_status(MD5_HMAC_STATUS, CRYPTO_STARTED);
	powerup_deu(MD5_HMAC_INIT);
	
	mctx->dbn = 0;
	md5_hmac_setkey_hw(mctx->key, mctx->keylen);

	return 0;
}
EXPORT_SYMBOL(md5_hmac_init);

/*! \fn void md5_hmac_update(struct crypto_tfm *tfm,
 *					const u8 *data, unsigned int len)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief on-the-fly md5 hmac computation
 *	\param tfm linux crypto algo transform
 *	\param data input data
 *	\param len size of input data
*/
static int md5_hmac_update(struct shash_desc *desc,
					const u8 *data, unsigned int len)
{
	struct md5_hmac_ctx *mctx = crypto_shash_ctx(desc->tfm);
	const u32 avail = sizeof(mctx->block) - (mctx->byte_count & 0x3f);

	mctx->byte_count += len;

	if (avail > len) {
		memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
			   data, len);
		return 0;
	}

	memcpy((char *)mctx->block + (sizeof(mctx->block) - avail),
		   data, avail);

	md5_hmac_transform(desc, mctx->block);
	data += avail;
	len -= avail;

	while (len >= sizeof(mctx->block)) {
		memcpy(mctx->block, data, sizeof(mctx->block));
		md5_hmac_transform(desc, mctx->block);
		data += sizeof(mctx->block);
		len -= sizeof(mctx->block);
	}

	memcpy(mctx->block, data, len);
	return 0;
}
EXPORT_SYMBOL(md5_hmac_update);

/*! \fn void md5_hmac_final(struct crypto_tfm *tfm, u8 *out)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief compute final md5 hmac value
 *	\param tfm linux crypto algo transform
 *	\param out final md5 hmac output value
*/
static int md5_hmac_final(struct shash_desc *desc, u8 *out)
{
	struct md5_hmac_ctx *mctx = crypto_shash_ctx(desc->tfm);
	const unsigned int offset = mctx->byte_count & 0x3f;
	char *p = (char *)mctx->block + offset;
	int padding = 56 - (offset + 1);
	volatile struct deu_hash_t *hashs = (struct deu_hash_t *) LTQ_HASH_CON;
	unsigned long flag;
	int i = 0;
	int dbn;
	u32 *in = &mctx->temp[0];

	*p++ = 0x80;
	if (padding < 0) {
		memset(p, 0x00, padding + sizeof(u64));
		md5_hmac_transform(desc, mctx->block);
		p = (char *)mctx->block;
		padding = 56;
	}

	memset(p, 0, padding);
	/* need to add 512 bit of the IPAD operation */
	mctx->block[14] = endian_swap((mctx->byte_count + 64) << 3);
	mctx->block[15] = 0x00000000;

	md5_hmac_transform(desc, mctx->block);

	CRTCL_SECT_START;
	powerup_deu(MD5_HMAC_INIT);

	/* printk("\ndbn = %d\n", mctx->dbn); */
	hashs->DBN = mctx->dbn;
	asm("sync");

	/* khs, go, init, ndc, endi, kyue, hmen, md5 */
	/* IFX_HASH_CON = 0x0703002D; */
	ltq_deu_w32(0x0703002D, LTQ_DEU_HASH_CON);

	while (hashs->controlr.BSY)
		;

	for (dbn = 0; dbn < mctx->dbn; dbn++) {
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

	/* reset the context after we finish with the hash */
	mctx->byte_count = 0;
	memset(&mctx->hash[0], 0, sizeof(MD5_HASH_WORDS));
	memset(&mctx->block[0], 0, sizeof(MD5_BLOCK_WORDS));
	memset(&mctx->temp[0], 0, MD5_HMAC_DBN_TEMP_SIZE);

	powerdown_deu(MD5_HMAC_INIT);
	CRTCL_SECT_END;
	set_md5_hmac_algo_status(MD5_HMAC_STATUS, CRYPTO_IDLE);

	return 0;
}

EXPORT_SYMBOL(md5_hmac_final);

/*
 * \brief MD5_HMAC function mappings
*/

static struct shash_alg ltqdeu_md5_hmac_alg = {
	.digestsize			=		MD5_DIGEST_SIZE,
	.init				=		md5_hmac_init,
	.update				=		md5_hmac_update,
	.final				=		md5_hmac_final,
	.setkey				=		md5_hmac_setkey,
	.base				=		{
		.cra_name			=		"hmac(md5)",
		.cra_driver_name	=		"ltqdeu-md5_hmac",
		.cra_ctxsize		=		sizeof(struct md5_hmac_ctx),
		.cra_flags			=		CRYPTO_ALG_TYPE_DIGEST,
		.cra_blocksize		=		MD5_HMAC_BLOCK_SIZE,
		.cra_module			=		THIS_MODULE,
		}
};

void ltq_md5_hmac_toggle_algo(int mode)
{
	if (mode) {
		crypto_unregister_shash(&ltqdeu_md5_hmac_alg);
		ltqdeu_md5_hmac_alg.base.cra_flags = CRYPTO_ALG_TYPE_DIGEST;
	} else
		crypto_register_shash(&ltqdeu_md5_hmac_alg);
}


/*! \fn int __init ltq_md5_hmac_init(void)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief initialize md5 hmac driver
*/
int __init ltq_md5_hmac_init(void)
{

	int i, ret = -ENOSYS;

	ret = crypto_register_shash(&ltqdeu_md5_hmac_alg);
	if (ret)
		goto md5_hmac_err;

	spin_lock_init(&power_lock);
	for (i = 0; i < MAX_MD5_HMAC_ALGO; i++)
		set_md5_hmac_algo_status(i, CRYPTO_IDLE);

	pr_notice("LTQ DEU MD5_HMAC initialized\n");
	return ret;

md5_hmac_err:
	pr_err("LTQ DEU MD5_HMAC initialization failed!\n");
	return ret;
}

/** \fn void __exit ltqdeu_fini_md5_hmac (void)
 *	\ingroup LTQ_MD5_HMAC_FUNCTIONS
 *	\brief unregister md5 hmac driver
*/
void __exit ltq_md5_hmac_fini(void)
{
	crypto_unregister_shash(&ltqdeu_md5_hmac_alg);
}

