/******************************************************************************
**
** FILE NAME	: ltqmips_async_des.c
** PROJECT		: LTQ UEIP
** MODULES		: DEU Module
**
** DATE			: October 11, 2010
** AUTHOR		: Mohammad Firdaus
** DESCRIPTION	: Data Encryption Unit Driver for DES Algorithm
** COPYRIGHT	:		Copyright (c) 2010
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
** 11, Oct 2010 Mohammad Firdaus	Kernel Port incl. Async. Ablkcipher mode
** 21,March 2011 Mohammad Firdaus	Changes for Kernel 2.6.32 and IPSec integration
*******************************************************************************/
/*!
 \defgroup LTQ_DEU LTQ_DEU_DRIVERS
 \ingroup API
 \brief ltq DEU driver module
*/

/*!
  \file ltqmips_async_des.c
  \ingroup LTQ_DEU
  \brief DES Encryption Driver main file
*/

/*!
 \defgroup LTQ_DES_FUNCTIONS LTQ_DES_FUNCTIONS
 \ingroup LTQ_DEU
 \brief LTQ DES driver Functions
*/

#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/crypto.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <asm/kmap_types.h>
#include <crypto/ctr.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>
#include <irq.h>
#include <lantiq_dma.h>
#include <lantiq_soc.h>

#include "ltqmips_deu.h"
#include "ltqmips_deu_dma.h"

#define DES_KEY_SIZE			8
#define DES_EXPKEY_WORDS		32
#define DES_BLOCK_SIZE			8
#define DES3_EDE_KEY_SIZE		(3 * DES_KEY_SIZE)
#define DES3_EDE_EXPKEY_WORDS	(3 * DES_EXPKEY_WORDS)
#define DES3_EDE_BLOCK_SIZE		DES_BLOCK_SIZE

/* Function Declaration to prevent warning messages */
u32 endian_swap(u32 input);
static int algo_status;
des_priv_t *des_queue;
extern deu_drv_priv_t deu_dma_priv;
//extern _ifx_deu_device ifx_deu[1];
static spinlock_t power_lock;

struct des_ctx {
	int controlr_M;
	int key_length;
	u8 iv[DES_BLOCK_SIZE];
	u32 expkey[DES3_EDE_EXPKEY_WORDS];
};

struct des_container {
	u8 *iv;
	u8 *dst_buf;
	u8 *src_buf;
	int mode;
	int encdec;
	int complete;
	int flag;

	u32 bytes_processed;
	u32 nbytes;

	struct ablkcipher_request arequest;
};

void set_des_algo_status(unsigned int des_algo, int cmd)
{
	unsigned long flag;
	spin_lock_irqsave(&power_lock, flag);
	algo_status = cmd;
	spin_unlock_irqrestore(&power_lock, flag);
}

int read_des_algo_status(void)
{
	int status;
	unsigned long flag;

	spin_lock_irqsave(&power_lock, flag);
	status = algo_status;
	spin_unlock_irqrestore(&power_lock, flag);

	return status;
}

/*! \fn int lq_des_setkey(struct crypto_ablkcipher *tfm, const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief sets DES key
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length
*/
static int lq_des_setkey(struct crypto_ablkcipher *tfm, const u8 *key,
						 unsigned int keylen)
{
	struct des_ctx *dctx = crypto_ablkcipher_ctx(tfm);

	/* printk("setkey in %s\n", __FILE__); */
	dctx->controlr_M = 0;
	dctx->key_length = keylen;
	memcpy ((u8 *) (dctx->expkey), key, keylen);

	return 0;
}

/*! \fn int lq_des3_ede_setkey(struct crypto_ablkcipher *tfm, const u8 *key, unsigned int keylen)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief sets DES key
 *	\param tfm linux crypto algo transform
 *	\param key input key
 *	\param keylen key length
*/

static int lq_des3_ede_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
					  unsigned int keylen)
{
	struct des_ctx *dctx = crypto_ablkcipher_ctx(tfm);

	/* printk("setkey in %s\n", __FILE__); */
	dctx->controlr_M = keylen/8 + 1;
	dctx->key_length = keylen;
	memcpy((u8 *) (dctx->expkey), in_key, keylen);

	return 0;
}

/*! \fn void des_chip_init (void)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief initialize DES hardware
*/

void des_chip_init(void)
{
	volatile struct des_t *des = (struct des_t *) LTQ_DES_CON;

	des->controlr.SM = 1;
	des->controlr.ARS = 0;
	asm("sync");
	des->controlr.NDC = 1;
	asm("sync");
	des->controlr.ENDI = 0;
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

static int lq_deu_des_core(void *ctx_arg, u8 *out_arg, const u8 *in_arg,
			 u8 *iv_arg, u32 nbytes, int encdec, int mode)
{
	volatile struct des_t *des = (struct des_t *) LTQ_DES_CON;
	struct des_ctx *dctx = ctx_arg;
	u32 *key = dctx->expkey;
	unsigned long flag;
	volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON;
	struct dma_device_info *dma_device = ifx_deu[0].dma_device;
	deu_drv_priv_t *deu_priv = (deu_drv_priv_t *)dma_device->priv;
	int wlen = 0;

	set_des_algo_status(DES_INIT, CRYPTO_STARTED);

	CRTCL_SECT_START;
	powerup_deu(DES_INIT);
	des_chip_init();

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
			CRTCL_SECT_END;
			return -EINVAL;
		}
	}

	des->controlr.E_D = !encdec;
	/* 0 ECB 1 CBC 2 OFB 3 CFB 4 CTR */
	des->controlr.O = mode;

	if (mode > 0) {
		des->IVHR = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
		des->IVLR = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
	};

	deu_priv->deu_rx_buf = (u32 *) out_arg;
	deu_priv->deu_rx_len = nbytes;
	deu_priv->rx_aligned_len = nbytes;

	dma->controlr.ALGO = 0;
	des->controlr.DAU = 0;
	dma->controlr.BS = 0;
	dma->controlr.EN = 1;

	while (des->controlr.BUS)
		;

	wlen = dma_device_write (dma_device, (u8 *) in_arg, nbytes, NULL);
	if (wlen != nbytes) {
		dma->controlr.EN = 0;
		CRTCL_SECT_END;
		pr_err("[%s %s %d]: dma_device_write fail!\n",
				__FILE__, __func__, __LINE__);
		return -EINVAL;
	}

	/* Prepare Rx buf length used in dma psuedo interrupt */
	deu_priv->outcopy = (u32 *) out_arg;
	deu_priv->event_src = DES_ASYNC_EVENT;

	if (mode > 0) {
		*(u32 *) iv_arg = DEU_ENDIAN_SWAP(des->IVHR);
		*((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(des->IVLR);
	};

	CRTCL_SECT_END;
	return -EINPROGRESS;
}

static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
{
	int i = 0;

	do {
			total_bytes -= sl[i].length;
			i++;

	} while (total_bytes > 0);

	return i;
}

/* \fn static inline struct des_container *des_container_cast (
 *					   struct scatterlist *dst)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Locate the structure des_container in memory.
 * \param *areq Pointer to memory location where ablkcipher_request is located
 * \return *des_cointainer The function pointer to des_container
*/

static inline struct des_container *des_container_cast(
						struct ablkcipher_request *areq)
{
	return container_of(areq, struct des_container, arequest);
}

/* \fn static void lq_sg_complete(struct des_container *des_con)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Free the used up memory after encryt/decrypt.
*/

static void lq_sg_complete(struct des_container *des_con)
{
	kfree(des_con);
}

/* \fn void lq_sg_init(struct scatterlist *src,
 *					   struct scatterlist *dst)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Maps the scatterlists into a source/destination page.
 * \param *src Pointer to the source scatterlist
 * \param *dst Pointer to the destination scatterlist
*/

static void lq_sg_init(struct des_container *des_con, struct scatterlist *src,
					   struct scatterlist *dst)
{
	struct page *dst_page, *src_page;

	src_page = sg_virt(src);
	des_con->src_buf = (char *) src_page;

	dst_page = sg_virt(dst);
	des_con->dst_buf = (char *) dst_page;
}

/* \fn static int process_next_packet(struct des_container *des_con,  struct ablkcipher_request *areq,
 *									   int state)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Process the next packet after dequeuing the packet from crypto queue
 * \param *des_con	Pointer to DES container structure
 * \param *areq		Pointer to ablkcipher_request container
 * \param state		State of the packet (scattered packet or new packet to be processed)
 * \return -EINVAL: DEU failure, -EINPROGRESS: DEU encrypt/decrypt in progress, 1: no scatterlist left
*/

static int process_next_packet(struct des_container *des_con,  struct ablkcipher_request *areq,
							   int state)
{
	u8 *iv;
	int mode, encdec, err = -EINVAL;
	u32 remain, inc, chunk_size, nbytes;
	struct scatterlist *src = NULL;
	struct scatterlist *dst = NULL;
	struct crypto_ablkcipher *cipher;
	struct des_ctx *ctx;
	unsigned long queue_flag;

	spin_lock_irqsave(&des_queue->lock, queue_flag);

	mode = des_con->mode;
	encdec = des_con->encdec;
	iv = des_con->iv;

	if (state & PROCESS_SCATTER) {
		src = scatterwalk_sg_next(areq->src);
		dst = scatterwalk_sg_next(areq->dst);

		if (!src || !dst) {
			spin_unlock_irqrestore(&des_queue->lock, queue_flag);
			return 1;
		}
	} else if (state & PROCESS_NEW_PACKET) {
		src = areq->src;
		dst = areq->dst;
	}

	remain = des_con->bytes_processed;
	chunk_size = src->length;

	/* printk("debug ln: %d, func: %s, reqsize: %d, scattersize: %d\n",
	*	__LINE__, __func__, areq->nbytes, chunk_size);
	*/

	if (remain > DEU_MAX_PACKET_SIZE)
		inc = DEU_MAX_PACKET_SIZE;
	else if (remain > chunk_size)
		inc = chunk_size;
	else
		inc = remain;

	remain -= inc;
	des_con->nbytes = inc;

	if (state & PROCESS_SCATTER) {
		des_con->src_buf += des_con->nbytes;
		des_con->dst_buf += des_con->nbytes;
	}

	lq_sg_init(des_con, src, dst);

	nbytes = des_con->nbytes;

	cipher = crypto_ablkcipher_reqtfm(areq);
	ctx = crypto_ablkcipher_ctx(cipher);

	if (des_queue->hw_status == DES_IDLE)
		des_queue->hw_status = DES_STARTED;

	des_con->bytes_processed -= des_con->nbytes;

	err = ablkcipher_enqueue_request(&des_queue->list, &des_con->arequest);
	if (err == -EBUSY) {
		pr_err("Failed to enqueue request, ln: %d, err: %d\n",
			   __LINE__, err);
		spin_unlock_irqrestore(&des_queue->lock, queue_flag);
		return -EINVAL;
	}

	spin_unlock_irqrestore(&des_queue->lock, queue_flag);

	err = lq_deu_des_core(ctx, des_con->dst_buf, des_con->src_buf, iv, nbytes, encdec, mode);

	return err;
}

/* \fn static void process_queue(unsigned long data)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief DES thread that handles crypto requests from upper layer & DMA
 * \param *data Not used
 * \return
*/

static void process_queue(unsigned long data)
{
	struct des_container *des_con = NULL;
	struct ablkcipher_request *areq = NULL;
	int err;
	unsigned long flag, queue_flag;

	CRTCL_SECT_START;
	powerdown_deu(DES_INIT);
	CRTCL_SECT_END;
	set_des_algo_status(DES_INIT, CRYPTO_IDLE);

proc_packet:
	spin_lock_irqsave(&des_queue->lock, queue_flag);

	if (des_queue->hw_status == DES_COMPLETED) {
		areq->base.complete(&areq->base, 0);
		lq_sg_complete(des_con);
		des_queue->hw_status = DES_IDLE;
		spin_unlock_irqrestore(&des_queue->lock, queue_flag);
		return;
	} else if (des_queue->hw_status == DES_STARTED) {
		areq = ablkcipher_dequeue_request(&des_queue->list);
		des_con = des_container_cast(areq);
		des_queue->hw_status = DES_BUSY;
	} else if (des_queue->hw_status == DES_IDLE) {
		areq = ablkcipher_dequeue_request(&des_queue->list);
		des_con = des_container_cast(areq);
		des_queue->hw_status = DES_STARTED;
	} else {
		areq = ablkcipher_dequeue_request(&des_queue->list);
		des_con = des_container_cast(areq);
	}

	spin_unlock_irqrestore(&des_queue->lock, queue_flag);

	/* printk("debug line - %d, func: %s, qlen: %d, bytes_proc: %d\n",
	 *		 __LINE__, __func__, des_queue->list.qlen, des_con->bytes_processed);
	*/

	if ((des_con->bytes_processed == 0))
		goto des_done;

	if ((!des_con))
		goto des_done;

	if (des_con->flag & PROCESS_NEW_PACKET) {
		des_con->flag = PROCESS_SCATTER;
		err = process_next_packet(des_con, areq, PROCESS_NEW_PACKET);
	} else
		err = process_next_packet(des_con, areq, PROCESS_SCATTER);

	if (err == -EINVAL) {
		areq->base.complete(&areq->base, err);
		lq_sg_complete(des_con);
		pr_notice("src/dst returned -EINVAL in func: %s\n", __func__);
	} else if (err > 0) {
		pr_notice("src/dst returned zero in func: %s\n", __func__);
		goto des_done;
	}

	return;

des_done:
	areq->base.complete(&areq->base, 0);
	lq_sg_complete(des_con);

	spin_lock_irqsave(&des_queue->lock, queue_flag);
	if (des_queue->list.qlen > 0) {
		spin_unlock_irqrestore(&des_queue->lock, queue_flag);
		goto proc_packet;
	} else
		des_queue->hw_status = DES_IDLE;

	spin_unlock_irqrestore(&des_queue->lock, queue_flag);

	return;
}

/* \fn static int lq_des_queue_mgr(struct des_ctx *ctx, struct ablkcipher_request *areq,
							u8 *iv, int encdec, int mode)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief starts the process of queuing DEU requests
 * \param *ctx crypto algo contax
 * \param *areq Pointer to the balkcipher requests
 * \param *iv Pointer to intput vector location
 * \param dir Encrypt/Decrypt
 * \mode The mode DES algo is running
 * \return 0 if success
*/

static int lq_queue_mgr(struct des_ctx *ctx, struct ablkcipher_request *areq,
						u8 *iv, int encdec, int mode)
{
	int err = -EINVAL;
	struct scatterlist *src = areq->src;
	struct scatterlist *dst = areq->dst;
	struct des_container *des_con = NULL;
	unsigned long queue_flag;
	u32 remain, inc, nbytes = areq->nbytes;
	u32 chunk_bytes = src->length;

	des_con = (struct des_container *)kmalloc(sizeof(struct des_container),
									   GFP_KERNEL);

	if (!(des_con)) {
		pr_err("Cannot allocate memory for AES container, fn %s, ln %d\n",
				__func__, __LINE__);
		return -ENOMEM;
	}

	/* DES encrypt/decrypt mode  */
	if (mode == 5) {
		nbytes = DES_BLOCK_SIZE;
		chunk_bytes = DES_BLOCK_SIZE;
		mode = 0;
	}

	des_con->bytes_processed = nbytes;
	des_con->arequest = (*areq);
	remain = nbytes;

	/* printk("debug - Line: %d, func: %s, reqsize: %d, scattersize: %d\n",
	*		__LINE__, __func__, nbytes, chunk_bytes);
	*/

	if (remain > DEU_MAX_PACKET_SIZE)
		inc = DEU_MAX_PACKET_SIZE;
	else if (remain > chunk_bytes)
		inc = chunk_bytes;
	else
		inc = remain;

	remain -= inc;
	lq_sg_init(des_con, src, dst);

	if (remain <= 0)
		des_con->complete = 1;
	else
		des_con->complete = 0;

	des_con->nbytes = inc;
	des_con->iv = iv;
	des_con->mode = mode;
	des_con->encdec = encdec;

	spin_lock_irqsave(&des_queue->lock, queue_flag);

	if (des_queue->hw_status == DES_STARTED || des_queue->hw_status == DES_BUSY ||
		des_queue->list.qlen > 0) {

		des_con->flag = PROCESS_NEW_PACKET;

		err = ablkcipher_enqueue_request(&des_queue->list, &des_con->arequest);
		if (err == -EBUSY) {
			spin_unlock_irqrestore(&des_queue->lock, queue_flag);
			pr_err("Fail to enqueue ablkcipher request ln: %d, err: %d\n",
				   __LINE__, err);
			return err;
		}

		spin_unlock_irqrestore(&des_queue->lock, queue_flag);
		return -EINPROGRESS;

	} else
		des_queue->hw_status = DES_STARTED;

	des_con->flag = PROCESS_SCATTER;
	des_con->bytes_processed -= des_con->nbytes;

	err = ablkcipher_enqueue_request(&des_queue->list, &des_con->arequest);
	if (err == -EBUSY) {
		pr_err("Fail to enqueue ablkcipher request ln: %d, err: %d\n",
		   __LINE__, err);

		spin_unlock_irqrestore(&des_queue->lock, queue_flag);
		return err;
	 }

	 spin_unlock_irqrestore(&des_queue->lock, queue_flag);

	 return lq_deu_des_core(ctx, des_con->dst_buf, des_con->src_buf, iv, inc, encdec, mode);
}

/* \fn static int lq_des_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_des_encrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_ENCRYPT, 5);

}

/* \fn static int lq_des_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_des_decrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_DECRYPT, 5);
}

/* \fn static int lq_ecb_des_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_ecb_des_encrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 0);
}

/* \fn static int lq_ecb_des_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/
static int lq_ecb_des_decrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 0);

}

/* \fn static int lq_cbc_ecb_des_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_cbc_des_encrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 1);
}
/* \fn static int lq_cbc_des_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_DES_FUNCTIONS
 * \brief Decrypt function for DES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int lq_cbc_des_decrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct des_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 1);
}

struct lq_des_alg {
	struct crypto_alg alg;
};

/* DES Supported algo array */
static struct lq_des_alg des_drivers_alg[] = {
	{
		.alg = {
			.cra_name		 = "des",
			.cra_driver_name = "ltqdeu-des",
			.cra_flags		 = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize	 = DES_BLOCK_SIZE,
			.cra_ctxsize	 = sizeof(struct des_ctx),
			.cra_type		 = &crypto_ablkcipher_type,
			.cra_priority	 = 300,
			.cra_module		 = THIS_MODULE,
			.cra_ablkcipher  = {
				.setkey = lq_des_setkey,
				.encrypt = lq_des_encrypt,
				.decrypt = lq_des_decrypt,
				.geniv = "eseqiv",
				.min_keysize = DES_KEY_SIZE,
				.max_keysize = DES_KEY_SIZE,
				.ivsize = DES_BLOCK_SIZE,
			}
		}

	}, {
		.alg = {
			.cra_name		 = "ecb(des)",
			.cra_driver_name = "ltqdeu-ecb(des)",
			.cra_flags		 = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize	 = DES_BLOCK_SIZE,
			.cra_ctxsize	 = sizeof(struct des_ctx),
			.cra_type		 = &crypto_ablkcipher_type,
			.cra_priority	 = 300,
			.cra_module		 = THIS_MODULE,
			.cra_ablkcipher  = {
				.setkey = lq_des_setkey,
				.encrypt = lq_ecb_des_encrypt,
				.decrypt = lq_ecb_des_decrypt,
				.geniv = "eseqiv",
				.min_keysize = DES_KEY_SIZE,
				.max_keysize = DES_KEY_SIZE,
				.ivsize = DES_BLOCK_SIZE,
			}
		 }
	}, {
		.alg = {
			.cra_name		 = "cbc(des)",
			.cra_driver_name = "ltqdeu-cbc(des)",
			.cra_flags		 = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize	 = DES_BLOCK_SIZE,
			.cra_ctxsize	 = sizeof(struct des_ctx),
			.cra_type		 = &crypto_ablkcipher_type,
			.cra_priority	 = 300,
			.cra_module		 = THIS_MODULE,
			.cra_ablkcipher  = {
				.setkey = lq_des_setkey,
				.encrypt = lq_cbc_des_encrypt,
				.decrypt = lq_cbc_des_decrypt,
				.geniv = "eseqiv",
				.min_keysize = DES3_EDE_KEY_SIZE,
				.max_keysize = DES3_EDE_KEY_SIZE,
				.ivsize = DES3_EDE_BLOCK_SIZE,
			}
		 }
	}, {
		.alg = {
			.cra_name		 = "des3_ede",
			.cra_driver_name = "ltqdeu-des3_ede",
			.cra_flags		 = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize	 = DES_BLOCK_SIZE,
			.cra_ctxsize	 = sizeof(struct des_ctx),
			.cra_type		 = &crypto_ablkcipher_type,
			.cra_priority	 = 300,
			.cra_module		 = THIS_MODULE,
			.cra_ablkcipher  = {
				.setkey = lq_des3_ede_setkey,
				.encrypt = lq_des_encrypt,
				.decrypt = lq_des_decrypt,
				.geniv = "eseqiv",
				.min_keysize = DES_KEY_SIZE,
				.max_keysize = DES_KEY_SIZE,
				.ivsize = DES_BLOCK_SIZE,
			}
		 }
	}, {
		.alg = {
			.cra_name		 = "ecb(des3_ede)",
			.cra_driver_name = "ltqdeu-ecb(des3_ede)",
			.cra_flags		 = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize	 = DES_BLOCK_SIZE,
			.cra_ctxsize	 = sizeof(struct des_ctx),
			.cra_type		 = &crypto_ablkcipher_type,
			.cra_priority	 = 300,
			.cra_module		 = THIS_MODULE,
			.cra_ablkcipher  = {
				.setkey = lq_des3_ede_setkey,
				.encrypt = lq_ecb_des_encrypt,
				.decrypt = lq_ecb_des_decrypt,
				.geniv = "eseqiv",
				.min_keysize = DES3_EDE_KEY_SIZE,
				.max_keysize = DES3_EDE_KEY_SIZE,
				.ivsize = DES3_EDE_BLOCK_SIZE,
			}
		 }
	}, {
		.alg = {
			.cra_name		 = "cbc(des3_ede)",
			.cra_driver_name = "ltqdeu-cbc(des3_ede)",
			.cra_flags		 = CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
			.cra_blocksize	 = DES_BLOCK_SIZE,
			.cra_ctxsize	 = sizeof(struct des_ctx),
			.cra_type		 = &crypto_ablkcipher_type,
			.cra_priority	 = 300,
			.cra_module		 = THIS_MODULE,
			.cra_ablkcipher  = {
				.setkey = lq_des3_ede_setkey,
				.encrypt = lq_cbc_des_encrypt,
				.decrypt = lq_cbc_des_decrypt,
				.geniv = "eseqiv",
				.min_keysize = DES3_EDE_KEY_SIZE,
				.max_keysize = DES3_EDE_KEY_SIZE,
				.ivsize = DES3_EDE_BLOCK_SIZE,
		   }
		 }
	}
};

void ltq_des_toggle_algo(int mode)
{
	int i, j, ret = 0;
	pr_info("%s des algo\n", mode ? "Unregister" : "Register");

	if (mode) {
		for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++) {
			crypto_unregister_alg(&des_drivers_alg[i].alg);
			des_drivers_alg[i].alg.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
												CRYPTO_ALG_ASYNC;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++) {
			ret = crypto_register_alg(&des_drivers_alg[i].alg);
			if (ret)
				goto des_err;
		}
	}

	return;

des_err:
	for (j = 0; j < i; j++)
	   crypto_unregister_alg(&des_drivers_alg[i].alg);

	pr_err("Lantiq %s driver initialization failed!\n",
			(char *)&des_drivers_alg[i].alg.cra_driver_name);
	return;
}

/*! \fn int ltq_async_des_init (void)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief initialize des driver
*/
int ltq_async_des_init (void)
{
	int i, j, ret = -EINVAL;

	for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++) {
		ret = crypto_register_alg(&des_drivers_alg[i].alg);
		pr_debug("driver: %s\n", des_drivers_alg[i].alg.cra_name);

		if (ret)
			goto des_err;
	}

	spin_lock_init(&power_lock);
	des_queue = kmalloc(sizeof(des_priv_t), GFP_KERNEL);
	crypto_init_queue(&des_queue->list, 500);
	spin_lock_init(&des_queue->lock);

	tasklet_init(&des_queue->des_task, process_queue, 0);

	des_queue->hw_status = DES_IDLE;

	set_des_algo_status(DES_INIT, CRYPTO_IDLE);

	pr_info("LTQ DEU ASYNC DES initialized.\n");
	return ret;

des_err:
	for (j = 0; j < i; j++)
		crypto_unregister_alg(&des_drivers_alg[i].alg);

	pr_err("Lantiq %s driver initialization failed!\n",
			(char *)&des_drivers_alg[i].alg.cra_driver_name);
	return ret;
}

/*! \fn void lqdeu_fini_async_des (void)
 *	\ingroup LTQ_DES_FUNCTIONS
 *	\brief unregister des driver
*/
void ltq_async_des_fini (void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(des_drivers_alg); i++)
		crypto_unregister_alg(&des_drivers_alg[i].alg);

	des_queue->hw_status = DES_COMPLETED;
	DEU_WAKEUP_EVENT(deu_dma_priv.deu_thread_wait,
					DES_ASYNC_EVENT,
					deu_dma_priv.des_event_flags);

	kfree(des_queue);
	ltq_deu_dma_fini();
}
