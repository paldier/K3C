/******************************************************************************
**
** FILE NAME	: ltqmips_async_aes.c
** PROJECT		: LTQ UEIP
** MODULES		: DEU Module
**
** DATE			: October 11, 2010
** AUTHOR		: Mohammad Firdaus
** DESCRIPTION	: Data Encryption Unit Driver for AES Algorithm
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
  \file ltqmips_async_aes.c
  \ingroup LTQ_DEU
  \brief AES Encryption Driver main file
*/

/*!
 \defgroup LTQ_AES_FUNCTIONS LTQ_AES_FUNCTIONS
 \ingroup LTQ_DEU
 \brief LTQ AES driver Functions
*/

#include <linux/wait.h>
#include <linux/crypto.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <crypto/ctr.h>
#include <crypto/aes.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>
#include <asm/kmap_types.h>
#include <irq.h>
#include <lantiq_soc.h>
#include <lantiq_dma.h>

#include "ltqmips_deu.h"

/* DMA related header and variables */
#include "ltqmips_deu_dma.h"
//extern _ifx_deu_device ifx_deu[1];
extern u32 *aes_buff_in;
extern u32 *aes_buff_out;

/* Definition of constants */
#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32
#define AES_BLOCK_SIZE		16
#define CTR_RFC3686_NONCE_SIZE	  4
#define CTR_RFC3686_IV_SIZE		 8
#define CTR_RFC3686_MAX_KEY_SIZE  (AES_MAX_KEY_SIZE + CTR_RFC3686_NONCE_SIZE)

/* Function decleration */
u32 endian_swap(u32 input);
aes_priv_t *aes_queue;
extern deu_drv_priv_t deu_dma_priv;
static int algo_status;
static spinlock_t power_lock;

struct aes_ctx {
	int key_length;
	u32 buf[AES_MAX_KEY_SIZE];
	u8 nonce[CTR_RFC3686_NONCE_SIZE];
};

struct aes_container {
	u8 iv[AES_BLOCK_SIZE];
	u8 *src_buf;
	u8 *dst_buf;

	int mode;
	int encdec;
	int complete;
	int flag;

	u32 req_bytes;
	u32 bytes_processed;
	u32 nbytes;
	u32 scatterbytes; /* scatterlist bytes */

	struct ablkcipher_request arequest;

};

void set_aes_algo_status(unsigned int aes_algo, int cmd)
{
	unsigned long flag;

	spin_lock_irqsave(&power_lock, flag);
	algo_status = cmd;
	spin_unlock_irqrestore(&power_lock, flag);
}

int read_aes_algo_status(void)
{
	unsigned long flag;
	int status;

	spin_lock_irqsave(&power_lock, flag);
	status = algo_status;
	spin_unlock_irqrestore(&power_lock, flag);

	return status;
}


static void hexdump(unsigned char *buf, unsigned int len)
{
		print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
						16, 1,
						buf, len, false);
}

/*! \fn void aes_chip_init (void)
 *	\ingroup BOARD_SPECIFIC_FUNCTIONS
 *	\brief initialize AES hardware
*/

void aes_chip_init (void)
{
	volatile struct aes_t *aes = (struct aes_t *) LTQ_AES_CON;

	aes->controlr.SM = 1;
	aes->controlr.ARS = 0;
	asm("sync");
	aes->controlr.NDC = 1;
	asm("sync");
	aes->controlr.ENDI = 0;
}

/*! \fn void lq_deu_aes_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg, u8 *iv_arg,
							 size_t nbytes, int encdec, int mode)
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

static int lq_deu_aes_core (void *ctx_arg, u8 *out_arg, const u8 *in_arg,
							u8 *iv_arg, size_t nbytes, int encdec, int mode)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	volatile struct aes_t *aes = (volatile struct aes_t *) LTQ_AES_CON;
	struct aes_ctx *ctx = (struct aes_ctx *)ctx_arg;
	u32 *in_key = ctx->buf;
	unsigned long flag;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
	int key_len = ctx->key_length;

	volatile struct deu_dma_t *dma = (struct deu_dma_t *) LTQ_DMA_CON;
	struct dma_device_info *dma_device = ifx_deu[0].dma_device;
	deu_drv_priv_t *deu_priv = (deu_drv_priv_t *)dma_device->priv;
	int wlen = 0;
	unsigned int bsize = nbytes;

	set_aes_algo_status(AES_INIT, CRYPTO_STARTED);
	powerup_deu(AES_INIT);
	aes_chip_init();

	CRTCL_SECT_START;

	/* 128, 192 or 256 bit key length */
	aes->controlr.K = key_len / 8 - 2;
		if (key_len == 128 / 8) {
		aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
		aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
		aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
		aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
	} else if (key_len == 192 / 8) {
		aes->K5R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
		aes->K4R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
		aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
		aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
		aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 4));
		aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 5));
	} else if (key_len == 256 / 8) {
		aes->K7R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 0));
		aes->K6R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 1));
		aes->K5R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 2));
		aes->K4R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 3));
		aes->K3R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 4));
		aes->K2R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 5));
		aes->K1R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 6));
		aes->K0R = DEU_ENDIAN_SWAP(*((u32 *) in_key + 7));
	} else {
		pr_err("[%s %s %d]: Invalid key_len : %d\n",
				__FILE__, __func__, __LINE__, key_len);
		hexdump ((u8 *)in_key, key_len);
		CRTCL_SECT_END;
		return -EINVAL;
	}

	/* let HW pre-process DEcryption key in any case (even if
	   ENcryption is used). Key Valid (KV) bit is then only
	   checked in decryption routine! */
	aes->controlr.PNK = 1;

	while (aes->controlr.BUS)
		;

	AES_DMA_MISC_CONFIG();

	aes->controlr.E_D = !encdec;
	aes->controlr.O = mode; /* 0 ECB 1 CBC 2 OFB 3 CFB 4 CTR  */

	/* default; only for CFB and OFB modes; change only for customer-specific apps */
	/* aes->controlr.F = 128; */

	if (mode > 0) {
		aes->IV3R = DEU_ENDIAN_SWAP(*(u32 *) iv_arg);
		aes->IV2R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
		aes->IV1R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
		aes->IV0R = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));
	};

	if (bsize % AES_BLOCK_SIZE)
		nbytes = bsize - (bsize % AES_BLOCK_SIZE) + AES_BLOCK_SIZE;

	/* printk(" mode: %d, bytes proc >> %d, enc/dec: %d\n", mode, nbytes, encdec); */

	/* Prepare Rx buf length used in dma psuedo interrupt */
	deu_priv->deu_rx_buf = (u32 *)out_arg;
	deu_priv->deu_rx_len = bsize;
	deu_priv->rx_aligned_len = nbytes;
	deu_priv->iv_arg = iv_arg;
	deu_priv->mode = mode;

	dma->controlr.ALGO = 1;
	dma->controlr.BS = 0;
	aes->controlr.DAU = 0;
	dma->controlr.EN = 1;

	deu_priv->outcopy = (u32 *) out_arg;
	deu_priv->event_src = AES_ASYNC_EVENT;

	while (aes->controlr.BUS)
		;

	wlen = dma_device_write (dma_device, (u8 *)in_arg, nbytes, NULL);
	if (wlen != nbytes) {
		dma->controlr.EN = 0;
		CRTCL_SECT_END;
		pr_err("[%s %s %d]: dma_device_write fail!\n",
				__FILE__, __func__, __LINE__);
		return -EINVAL;
	}

	CRTCL_SECT_END;
	return -EINPROGRESS;
}

/* \fn static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Counts and return the number of scatterlists
 * \param *sl Function pointer to the scatterlist
 * \param total_bytes The total number of bytes that needs to be encrypted/decrypted
 * \return The number of scatterlists
*/

static int count_sgs(struct scatterlist *sl, unsigned int total_bytes)
{
	int i = 0;

	do {
		total_bytes -= sl[i].length;
		i++;

	} while (total_bytes > 0);

	return i;
}

/* \fn void lq_sg_init(struct scatterlist *src,
 *					   struct scatterlist *dst)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Maps the scatterlists into a source/destination page.
 * \param *src Pointer to the source scatterlist
 * \param *dst Pointer to the destination scatterlist
*/

static void lq_sg_init(struct aes_container *aes_con, struct scatterlist *src,
					   struct scatterlist *dst)
{
	void *dst_page, *src_page;
	unsigned int src_offset, dst_offset;

	/* to circumvent the packets bigger than MAX DMA transfer size */
	if (aes_con->req_bytes > src->length) {
		src_offset = (src->length - aes_con->scatterbytes);
		dst_offset = (dst->length - aes_con->scatterbytes);
	} else {
		src_offset = 0;
		dst_offset = 0;
	}

	src_page = sg_virt(src);
	aes_con->src_buf = (char *) src_page + src_offset;

	dst_page = sg_virt(dst);
	aes_con->dst_buf = (char *) dst_page + dst_offset;
}


/* \fn static void lq_sg_complete(struct aes_container *aes_con)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Free the used up memory after encryt/decrypt.
*/

static void lq_sg_complete(struct aes_container *aes_con)
{
	kfree(aes_con);
}

/* \fn static inline struct aes_container *aes_container_cast (
 *					   struct scatterlist *dst)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Locate the structure aes_container in memory.
 * \param *areq Pointer to memory location where ablkcipher_request is located
 * \return *aes_cointainer The function pointer to aes_container
*/
static inline struct aes_container *aes_container_cast (
		struct ablkcipher_request *areq)
{
	return container_of(areq, struct aes_container, arequest);
}


/* \fn static int process_next_packet(struct aes_container *aes_con, struct ablkcipher_request *areq,
 * \								  int state)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Process next packet to be encrypt/decrypt
 * \param *aes_con	AES container structure
 * \param *areq Pointer to memory location where ablkcipher_request is located
 * \param state The state of the current packet (part of scatterlist or new packet)
 * \return -EINVAL: error, -EINPROGRESS: Crypto still running, 1: no more scatterlist
*/

static int process_next_packet(struct aes_container *aes_con, struct ablkcipher_request *areq,
							   int state)
{
	u8 *iv;
	int mode, dir, err = -EINVAL;
	unsigned long queue_flag;
	struct scatterlist *src = NULL;
	struct scatterlist *dst = NULL;
	struct crypto_ablkcipher *cipher;
	struct aes_ctx *ctx;
	u32 inc, nbytes, remain, chunk_size;
	u32 scatterbytes = aes_con->scatterbytes;

	spin_lock_irqsave(&aes_queue->lock, queue_flag);

	dir = aes_con->encdec;
	mode = aes_con->mode;
	iv = aes_con->iv;

	if (state & PROCESS_SCATTER) {
		if (scatterbytes == 0) {
			src = scatterwalk_sg_next(areq->src);
			dst = scatterwalk_sg_next(areq->dst);

			aes_con->scatterbytes = src->length;

			if (!src || !dst) {
				spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
				return 1;
			}
		} else {
			src = areq->src;
			dst = areq->dst;
		}
	} else if (state & PROCESS_NEW_PACKET) {
		src = areq->src;
		dst = areq->dst;
		aes_con->req_bytes = areq->nbytes;
	}

	if ((state & PROCESS_SCATTER) && (scatterbytes != 0))
		remain = scatterbytes;
	else
		remain = aes_con->bytes_processed;

	chunk_size = src->length;

	if (remain > DEU_MAX_PACKET_SIZE)
	   inc = DEU_MAX_PACKET_SIZE;
	else if (remain > chunk_size)
	   inc = chunk_size;
	else
	   inc = remain;

	remain -= inc;
	aes_con->nbytes = inc;

	if (state & PROCESS_SCATTER) {
		aes_con->src_buf += aes_con->nbytes;
		aes_con->dst_buf += aes_con->nbytes;
	}

	lq_sg_init(aes_con, src, dst);

	nbytes = aes_con->nbytes;
	aes_con->scatterbytes -= inc;

	cipher = crypto_ablkcipher_reqtfm(areq);
	ctx = crypto_ablkcipher_ctx(cipher);

	/* printk("\ndebug - Line: %d, func: %s, reqsize: %d, scattersize: %d, scatterbytes %d\n",
	*			  __LINE__, __func__, nbytes, chunk_size, aes_con->scatterbytes);
	*/

	/* For some reason, virt mem given may at times be out of bounds
	 * causing a kernel oops. Only happens in RFC3686-CTR mode and a fix is
	 * made by forcing the memory to be  accessible again in the 0x8000000 range
	 */
	if (!(((u32) aes_con->src_buf) & 0x80000000)) {
		aes_con->src_buf = (((u32)aes_con->src_buf) | 0x80000000);
		aes_con->dst_buf = (((u32)aes_con->dst_buf) | 0x80000000);
	}

	if (aes_queue->hw_status == AES_IDLE)
		aes_queue->hw_status = AES_STARTED;

	aes_con->bytes_processed -= aes_con->nbytes;

	err = ablkcipher_enqueue_request(&aes_queue->list, &aes_con->arequest);
	if (err == -EBUSY) {
		spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
		pr_err("Failed to enqueue request, ln: %d, err: %d\n",
				__LINE__, err);
		return -EINVAL;
	}

	spin_unlock_irqrestore(&aes_queue->lock, queue_flag);

	return lq_deu_aes_core(ctx, aes_con->dst_buf, aes_con->src_buf, iv, nbytes, dir, mode);

}

/* \fn static void process_queue(unsigned long data)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief AES thread that handles crypto requests from upper layer & DMA
 * \param *data Not used
 * \return
*/
static void process_queue(unsigned long data)
{
	struct aes_container *aes_con = NULL;
	struct ablkcipher_request *areq = NULL;
	int err;
	unsigned long queue_flag, flag;
	u8 *iv_arg;
	struct dma_device_info *dma_device = ifx_deu[0].dma_device;
	volatile struct aes_t *aes = (volatile struct aes_t *) LTQ_AES_CON;
	deu_drv_priv_t *deu_priv = (deu_drv_priv_t *)dma_device->priv;

proc_aes_pkt:
	spin_lock_irqsave(&aes_queue->lock, queue_flag);

	if (aes_queue->hw_status == AES_COMPLETED) {
		spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
		lq_sg_complete(aes_con);
		aes_queue->hw_status = AES_IDLE;
		areq->base.complete(&areq->base, 0);
		return;
	} else if (aes_queue->hw_status == AES_STARTED) {
		areq = ablkcipher_dequeue_request(&aes_queue->list);
		aes_con = aes_container_cast(areq);
		aes_queue->hw_status = AES_BUSY;
	} else if (aes_queue->hw_status == AES_IDLE) {
		areq = ablkcipher_dequeue_request(&aes_queue->list);
		aes_con = aes_container_cast(areq);
		aes_queue->hw_status = AES_STARTED;
	} else {
		areq = ablkcipher_dequeue_request(&aes_queue->list);
		aes_con = aes_container_cast(areq);
	}

	spin_unlock_irqrestore(&aes_queue->lock, queue_flag);

	/*printk("debug ln: %d, bytes proc: %d, qlen: %d, update_iv: %d\n",
	*		__LINE__, aes_con->bytes_processed, aes_queue->list.qlen, update_iv);
	*/

	if (!aes_con) {
	   goto aes_done;
	}

	/* IV have to be updated here because of async nature of the
		driver's datapath */
	if ((deu_priv->mode > 0)) {
		CRTCL_SECT_START;
		iv_arg = deu_priv->iv_arg;

		*((u32 *) iv_arg) = aes->IV3R;
		*((u32 *) iv_arg + 1) = aes->IV2R;
		*((u32 *) iv_arg + 2) = aes->IV1R;
		*((u32 *) iv_arg + 3) = aes->IV0R;
		*((u32 *) iv_arg) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg));
		*((u32 *) iv_arg + 1) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 1));
		*((u32 *) iv_arg + 2) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 2));
		*((u32 *) iv_arg + 3) = DEU_ENDIAN_SWAP(*((u32 *) iv_arg + 3));

		/* for (i = 0; i < 16; i+=4)
		*	printk("IV[%d]:%02x%02x%02x%02x ", i,
		*	iv_arg[i], iv_arg[i+1], iv_arg[i+2], iv_arg[i+3]);
		*/

		deu_priv->mode = 0;
		powerdown_deu(AES_INIT);
		CRTCL_SECT_END;
		set_aes_algo_status(AES_INIT, CRYPTO_IDLE);
	}

	if (aes_con->bytes_processed == 0) {
		goto aes_done;
	}

	/* Process new packet or the next packet in a scatterlist */
	if (aes_con->flag & PROCESS_NEW_PACKET) {
	   aes_con->flag = PROCESS_SCATTER;
	   err = process_next_packet(aes_con, areq, PROCESS_NEW_PACKET);
	   return;
	} else {
		err = process_next_packet(aes_con, areq, PROCESS_SCATTER);
		return;
	}
	if (err == -EINVAL) {
		areq->base.complete(&areq->base, err);
		lq_sg_complete(aes_con);
		pr_notice("src/dst returned -EINVAL in func: %s\n", __func__);
	} else if (err > 0) {
		pr_notice("src/dst returned zero in func: %s\n", __func__);
		goto aes_done;
	}

	return;

aes_done:
	areq->base.complete(&areq->base, 0);
	lq_sg_complete(aes_con);

	spin_lock_irqsave(&aes_queue->lock, queue_flag);

	if (aes_queue->list.qlen > 0) {
		spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
		goto proc_aes_pkt;
	} else
		aes_queue->hw_status = AES_IDLE;

	spin_unlock_irqrestore(&aes_queue->lock, queue_flag);

	return;
}

/* \fn static int lq_aes_queue_mgr(struct aes_ctx *ctx, struct ablkcipher_request *areq,
							u8 *iv, int dir, int mode)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief starts the process of queuing DEU requests
 * \param *ctx crypto algo contax
 * \param *areq Pointer to the balkcipher requests
 f \param *iv Pointer to intput vector location
 * \param dir Encrypt/Decrypt
 * \mode The mode AES algo is running
 * \return 0 if success
*/

static int lq_aes_queue_mgr(struct aes_ctx *ctx, struct ablkcipher_request *areq,
							u8 *iv, int dir, int mode)
{
	int err = -EINVAL;
	unsigned long queue_flag;
	struct scatterlist *src = areq->src;
	struct scatterlist *dst = areq->dst;
	struct aes_container *aes_con = NULL;
	u32 remain, inc, nbytes = areq->nbytes;
	u32 chunk_bytes = src->length;

	aes_con = (struct aes_container *)kmalloc(sizeof(struct aes_container),
											   GFP_KERNEL);

	if (!(aes_con)) {
	   pr_err("Cannot allocate memory for AES container, fn %s, ln %d\n",
			__func__, __LINE__);
	   return -ENOMEM;
	}

	/* AES encrypt/decrypt mode */
	if (mode == 5) {
		nbytes = AES_BLOCK_SIZE;
		chunk_bytes = AES_BLOCK_SIZE;
		mode = 0;
	}

	aes_con->scatterbytes = src->length;
	aes_con->bytes_processed = nbytes;
	aes_con->req_bytes = nbytes;
	aes_con->arequest = *(areq);

	remain = nbytes;

	if (remain > DEU_MAX_PACKET_SIZE)
	   inc = DEU_MAX_PACKET_SIZE;
	else if (remain > chunk_bytes)
	   inc = chunk_bytes;
	else
	   inc = remain;

	remain -= inc;
	if (remain <= 0)
		aes_con->complete = 1;
	else
		aes_con->complete = 0;

	aes_con->nbytes = inc;
	aes_con->mode = mode;
	aes_con->encdec = dir;

	/* IVs are AES_BLOCK_SIZE large, incl. rfc3686 [NOUNCE | IV | CTR]*/
	memcpy(aes_con->iv, iv, AES_BLOCK_SIZE);

	spin_lock_irqsave(&aes_queue->lock, queue_flag);

	if (aes_queue->hw_status == AES_STARTED || aes_queue->hw_status == AES_BUSY ||
			 aes_queue->list.qlen > 0) {

		aes_con->flag = PROCESS_NEW_PACKET;
		err = ablkcipher_enqueue_request(&aes_queue->list, &aes_con->arequest);

		 /* max queue length reached */
		if (err == -EBUSY) {
			spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
			pr_err("Unable to enqueue request ln: %d, err: %d\n", __LINE__, err);
			 return err;
		 }

		spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
		return -EINPROGRESS;
	} else if (aes_queue->hw_status == AES_IDLE)
		aes_queue->hw_status = AES_STARTED;

	lq_sg_init(aes_con, src, dst);
	aes_con->flag = PROCESS_SCATTER;
	aes_con->bytes_processed -= aes_con->nbytes;
	aes_con->scatterbytes -= aes_con->nbytes;

	/* or enqueue the whole structure so as to get back the info
	 * at the moment that it's queued. nbytes might be different */
	err = ablkcipher_enqueue_request(&aes_queue->list, &aes_con->arequest);

	if (err == -EBUSY) {
		spin_unlock_irqrestore(&aes_queue->lock, queue_flag);
		pr_err("Unable to enqueue request ln: %d, err: %d\n", __LINE__, err);
		return err;
	}

	spin_unlock_irqrestore(&aes_queue->lock, queue_flag);

	return lq_deu_aes_core(ctx, aes_con->dst_buf, aes_con->src_buf, aes_con->iv, inc, dir, mode);

}

/* \fn static int aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
 *					   unsigned int keylen)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Sets AES key
 * \param *tfm Pointer to the ablkcipher transform
 * \param *in_key Pointer to input keys
 * \param key_len Length of the AES keys
 * \return 0 is success, -EINVAL if bad key length
*/

static int aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
					  unsigned int keylen)
{
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(tfm);
	unsigned long *flags = (unsigned long *) &tfm->base.crt_flags;

	if (keylen != 16 && keylen != 24 && keylen != 32) {
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	ctx->key_length = keylen;
	pr_debug("ctx @%p, keylen %d, ctx->key_length %d\n",
			ctx, keylen, ctx->key_length);
	memcpy ((u8 *) (ctx->buf), in_key, keylen);

	return 0;

}

/* \fn static int aes_generic_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
 *					   unsigned int keylen)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Sets AES key
 * \param *tfm Pointer to the ablkcipher transform
 * \param *key Pointer to input keys
 * \param keylen Length of AES keys
 * \return 0 is success, -EINVAL if bad key length
*/

static int aes_generic_setkey(struct crypto_ablkcipher *tfm, const u8 *key,
							  unsigned int keylen)
{
   return aes_setkey(tfm, key, keylen);
}

/* \fn static int rfc3686_aes_setkey(struct crypto_ablkcipher *tfm, const u8 *in_key,
 *					   unsigned int keylen)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Sets AES key
 * \param *tfm Pointer to the ablkcipher transform
 * \param *in_key Pointer to input keys
 * \param key_len Length of the AES keys
 * \return 0 is success, -EINVAL if bad key length
*/

static int rfc3686_aes_setkey(struct crypto_ablkcipher *tfm,
							 const u8 *in_key, unsigned int keylen)
{
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(tfm);
	unsigned long *flags = (unsigned long *)&tfm->base.crt_flags;

	memcpy(ctx->nonce, in_key + (keylen - CTR_RFC3686_NONCE_SIZE),
		   CTR_RFC3686_NONCE_SIZE);

	/* remove 4 bytes of nonce */
	keylen -= CTR_RFC3686_NONCE_SIZE;

	if (keylen != 16 && keylen != 24 && keylen != 32) {
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	ctx->key_length = keylen;
	memcpy ((u8 *) (ctx->buf), in_key, keylen);

	return 0;
}

/* \fn static int aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int aes_encrypt (struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_ENCRYPT, 5);

}

/* \fn static int aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/
static int aes_decrypt (struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);


	return lq_aes_queue_mgr(ctx, areq, NULL, CRYPTO_DIR_DECRYPT, 5);
}

/* \fn static int ecb_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int ecb_aes_encrypt (struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 0);

}
/* \fn static int ecb_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/
static int ecb_aes_decrypt(struct ablkcipher_request *areq)

{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 0);
}

/* \fn static int cbc_aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int cbc_aes_encrypt (struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 1);

}

/* \fn static int cbc_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int cbc_aes_decrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 1);
}

/* \fn static int ctr_aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int ctr_aes_encrypt (struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_ENCRYPT, 4);

}

/* \fn static int ctr_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int ctr_aes_decrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);

	return lq_aes_queue_mgr(ctx, areq, areq->info, CRYPTO_DIR_DECRYPT, 4);
}

/* \fn static int rfc3686_aes_encrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Encrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int rfc3686_aes_encrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);
	int ret;
	u8 *info = areq->info;
	u8 rfc3686_iv[16];

	memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE);
	memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, info, CTR_RFC3686_IV_SIZE);

	/* initialize counter portion of counter block */
	*(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
		cpu_to_be32(1);

	areq->info = rfc3686_iv;
	ret = lq_aes_queue_mgr(ctx, areq, rfc3686_iv, CRYPTO_DIR_ENCRYPT, 4);

	areq->info = info;
	return ret;
}

/* \fn static int rfc3686_aes_decrypt(struct ablkcipher_request *areq)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Decrypt function for AES algo
 * \param *areq Pointer to ablkcipher request in memory
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

static int rfc3686_aes_decrypt(struct ablkcipher_request *areq)
{
	struct crypto_ablkcipher *cipher = crypto_ablkcipher_reqtfm(areq);
	struct aes_ctx *ctx = crypto_ablkcipher_ctx(cipher);
	int ret;
	u8 *info = areq->info;
	u8 rfc3686_iv[16];

	/* set up counter block */
	memcpy(rfc3686_iv, ctx->nonce, CTR_RFC3686_NONCE_SIZE);
	memcpy(rfc3686_iv + CTR_RFC3686_NONCE_SIZE, info, CTR_RFC3686_IV_SIZE);

	/* initialize counter portion of counter block */
	*(__be32 *)(rfc3686_iv + CTR_RFC3686_NONCE_SIZE + CTR_RFC3686_IV_SIZE) =
		cpu_to_be32(1);

	areq->info = rfc3686_iv;

	ret = lq_aes_queue_mgr(ctx, areq, rfc3686_iv, CRYPTO_DIR_DECRYPT, 4);
	areq->info = info;

	return ret;
}

struct lq_aes_alg {
	struct crypto_alg alg;
};

/* AES supported algo array */
static struct lq_aes_alg aes_drivers_alg[] = {
	 {
		 .alg = {
		   .cra_name		= "aes",
		   .cra_driver_name = "ltqdeu-aes",
		   .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		   .cra_blocksize	= AES_BLOCK_SIZE,
		   .cra_ctxsize		= sizeof(struct aes_ctx),
		   .cra_type		= &crypto_ablkcipher_type,
		   .cra_priority	= 300,
		   .cra_module		= THIS_MODULE,
		   .cra_ablkcipher = {
				.setkey = aes_setkey,
				.encrypt = aes_encrypt,
				.decrypt = aes_decrypt,
				.geniv = "eseqiv",
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.ivsize = AES_BLOCK_SIZE,
			}
		}
	 }, {
		.alg = {
		   .cra_name		= "ecb(aes)",
		   .cra_driver_name = "ltqdeu-ecb(aes)",
		   .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		   .cra_blocksize	= AES_BLOCK_SIZE,
		   .cra_ctxsize		= sizeof(struct aes_ctx),
		   .cra_type		= &crypto_ablkcipher_type,
		   .cra_priority	= 300,
		   .cra_module		= THIS_MODULE,
		   .cra_ablkcipher = {
				.setkey = aes_generic_setkey,
				.encrypt = ecb_aes_encrypt,
				.decrypt = ecb_aes_decrypt,
				.geniv = "eseqiv",
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.ivsize = AES_BLOCK_SIZE,
			}
		}
	 }, {
		 .alg = {
		   .cra_name		= "cbc(aes)",
		   .cra_driver_name = "ltqdeu-cbc(aes)",
		   .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		   .cra_blocksize	= AES_BLOCK_SIZE,
		   .cra_ctxsize		= sizeof(struct aes_ctx),
		   .cra_type		= &crypto_ablkcipher_type,
		   .cra_priority	= 300,
		   .cra_module		= THIS_MODULE,
		   .cra_ablkcipher = {
				.setkey = aes_generic_setkey,
				.encrypt = cbc_aes_encrypt,
				.decrypt = cbc_aes_decrypt,
				.geniv = "eseqiv",
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.ivsize = AES_BLOCK_SIZE,
			}
		}
	 }, {
		 .alg = {
		   .cra_name		= "ctr(aes)",
		   .cra_driver_name = "ltqdeu-ctr(aes)",
		   .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		   .cra_blocksize	= AES_BLOCK_SIZE,
		   .cra_ctxsize		= sizeof(struct aes_ctx),
		   .cra_type		= &crypto_ablkcipher_type,
		   .cra_priority	= 300,
		   .cra_module		= THIS_MODULE,
		   .cra_ablkcipher = {
				.setkey = aes_generic_setkey,
				.encrypt = ctr_aes_encrypt,
				.decrypt = ctr_aes_decrypt,
				.geniv = "eseqiv",
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.ivsize = AES_BLOCK_SIZE,
			}
		}
	 }, {
		.alg = {
		   .cra_name		= "rfc3686(ctr(aes))",
		   .cra_driver_name = "ltqdeu-rfc3686(ctr(aes))",
		   .cra_flags		= CRYPTO_ALG_TYPE_ABLKCIPHER | CRYPTO_ALG_ASYNC,
		   .cra_blocksize	= AES_BLOCK_SIZE,
		   .cra_ctxsize		= sizeof(struct aes_ctx),
		   .cra_type		= &crypto_ablkcipher_type,
		   .cra_priority	= 300,
		   .cra_module		= THIS_MODULE,
		   .cra_ablkcipher = {
				.setkey = rfc3686_aes_setkey,
				.encrypt = rfc3686_aes_encrypt,
				.decrypt = rfc3686_aes_decrypt,
				.geniv = "seqiv",
				.min_keysize = AES_MIN_KEY_SIZE,
				.max_keysize = AES_MAX_KEY_SIZE,
				.ivsize = CTR_RFC3686_IV_SIZE,
			}
		 }
	  }
};

void ltq_aes_toggle_algo (int mode)
{
	int i, j, ret = 0;

	pr_info("%s aes algo\n", mode ? "Unregister" : "Register");

	if (mode) {
		for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++) {
			crypto_unregister_alg(&aes_drivers_alg[i].alg);
			aes_drivers_alg[i].alg.cra_flags = CRYPTO_ALG_TYPE_ABLKCIPHER |
											   CRYPTO_ALG_ASYNC;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++) {
			ret = crypto_register_alg(&aes_drivers_alg[i].alg);
			if (ret)
				goto aes_err;
		}
	}

	return;

aes_err:
	for (j = 0; j < i; j++)
		crypto_unregister_alg(&aes_drivers_alg[j].alg);

	pr_err("Lantiq %s driver initialization failed!\n", (char *)&aes_drivers_alg[i].alg.cra_driver_name);
	return;

}

/* \fn int ltq_async_aes_init (void)
 * \ingroup LTQ_AES_FUNCTIONS
 * \brief Initializes the Async. AES driver
 * \return 0 is success, -EINPROGRESS if encryting, EINVAL if failure
*/

int ltq_async_aes_init (void)
{
	int i, j, ret = -EINVAL;

	for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++) {
		ret = crypto_register_alg(&aes_drivers_alg[i].alg);
		pr_info("driver: %s\n", aes_drivers_alg[i].alg.cra_name);
		if (ret)
			goto aes_err;
	}

	aes_queue = kmalloc(sizeof(aes_priv_t), GFP_KERNEL);
	crypto_init_queue(&aes_queue->list, 500);

	spin_lock_init(&aes_queue->lock);

	spin_lock_init(&power_lock);
	aes_queue->hw_status = AES_IDLE;
	tasklet_init(&aes_queue->aes_task, process_queue, 0);

	set_aes_algo_status(AES_INIT, CRYPTO_IDLE);

	pr_notice("Lantiq DEU ASYNC AES initialized\n");

	return ret;

aes_err:

	for (j = 0; j < i; j++)
		crypto_unregister_alg(&aes_drivers_alg[j].alg);

	pr_err("Lantiq %s driver initialization failed!\n",
		(char *)&aes_drivers_alg[i].alg.cra_driver_name);
	return ret;
}

/*! \fn void ltq_async_aes_fini (void)
 *	\ingroup LTQ_AES_FUNCTIONS
 *	\brief unregister aes driver
*/
void ltq_async_aes_fini(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(aes_drivers_alg); i++)
		crypto_unregister_alg(&aes_drivers_alg[i].alg);

	aes_queue->hw_status = AES_COMPLETED;

	DEU_WAKEUP_EVENT(deu_dma_priv.deu_thread_wait, AES_ASYNC_EVENT,
								 deu_dma_priv.aes_event_flags);

	kfree(aes_queue);
	ltq_deu_dma_fini();
}
