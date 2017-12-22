/******************************************************************************
**
** FILE NAME	: ltqmips_deu_dma.c
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
** 08 Sept 2009 Mohammad Firdaus	Initial UEIP release
*******************************************************************************/
/*!
  \defgroup LTQ_DEU LTQ_DEU_DRIVERS
  \ingroup	LTQ_API
  \brief ltq deu driver module
*/

/*!
  \file	ltqmips_deu_dma.c
  \ingroup LTQ_DEU
  \brief DMA deu driver file
*/

/*!
 \defgroup LTQ_DMA_FUNCTIONS LTQ_DMA_FUNCTIONS
 \ingroup LTQ_DEU
 \brief deu-dma driver functions
*/

/* Project header files */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/crypto.h>
#include <linux/spinlock.h>
#include <crypto/algapi.h>
#include <crypto/scatterwalk.h>

#include <asm/io.h>
#include <lantiq_soc.h>
#include <irq.h>
#include <lantiq_dma.h>
#include "ltqmips_deu_dma.h"
#include "ltqmips_deu.h"

extern struct ltq_deu_config algo_config;
void aes_dma_memory_copy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes);
extern deu_drv_priv_t deu_dma_priv;

#if defined (CONFIG_CRYPTO_LTQ_ASYNC_MODE)
extern aes_priv_t *aes_queue;
extern des_priv_t *des_queue;

static inline void schedule_des_task(void)
{
	tasklet_schedule(&des_queue->des_task);
}

static inline void schedule_aes_task(void)
{
	tasklet_schedule(&aes_queue->aes_task);
}
#endif /* CONFIG_CRYPTO_LTQ_ASYNC_MODE */

#ifdef CONFIG_DEBUG
static void hexdump(unsigned char *buf, unsigned int len)
{
		print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
						16, 1,
						buf, len, false);
}

#endif

/*! \fn int deu_dma_intr_handler (struct dma_device_info *dma_dev, int status)
 *	\ingroup LTQ_DMA_FUNCTIONS
 *	\brief callback function for deu dma interrupt
 *	\param dma_dev dma device
 *	\param status not used
*/
int deu_dma_intr_handler (struct dma_device_info *dma_dev, int status)
{
	deu_drv_priv_t *deu_priv = (deu_drv_priv_t *)dma_dev->priv;
	u8 *buf;
	int len = 0;
	int nbytes;
	unsigned long flag;

	/* return immediately if we are in polling mode */
	if (algo_config.async_mode == 0)
		return 0;

	switch (status) {
	case RCV_INT:
		/* unaligned crypto fix !! */
		if (deu_priv->deu_rx_len != deu_priv->rx_aligned_len)
			nbytes = deu_priv->rx_aligned_len;
		else
			nbytes = deu_priv->deu_rx_len;

		pr_debug("actual bytes xfer >> %d, memcpy bytes >> %d\n",
			nbytes, deu_priv->deu_rx_len);

		CRTCL_SECT_START;
		len = dma_device_read(dma_dev, (u8 **)&buf, NULL);
		if (len < nbytes) {
			pr_err("%s packet length %d is not equal to expect %d\n",
				__func__, len, deu_priv->deu_rx_len);
			return -EINVAL;
		}
		CRTCL_SECT_END;

#ifdef CONFIG_DEBUG
		pr_info(" **** dumping buf of addr: %08x **** \n", buf);
		hexdump((char *)buf, nbytes);
#endif

		memcpy(deu_priv->deu_rx_buf, buf, deu_priv->deu_rx_len);
		deu_priv->deu_rx_buf = NULL;
		deu_priv->deu_rx_len = 0;
		deu_priv->rx_aligned_len = 0;

		if (deu_priv->event_src == DES_ASYNC_EVENT) {
			schedule_des_task();
			return 0;
		}

		if (deu_priv->event_src == AES_ASYNC_EVENT) {
			schedule_aes_task();
			return 0;
		}

		break;
	case TX_BUF_FULL_INT:
		 break;

	case TRANSMIT_CPT_INT:
		break;

	default:
		break;
	}

	return 0;
}

extern u8 *g_dma_block;
extern u8 *g_dma_block2;

/*! \fn u8 *deu_dma_buffer_alloc (int len, int *byte_offset, void **opt)
 *	\ingroup LTQ_DMA_FUNCTIONS
 *	\brief callback function for allocating buffers for dma receive descriptors
 *	\param len not used
 *	\param byte_offset dma byte offset
 *	\param *opt not used
 *
*/
u8 *deu_dma_buffer_alloc (int len, int *byte_offset, void **opt)
{
	u8 *swap = NULL;

	/* dma-core needs at least 2 blocks of memory */
	swap = g_dma_block;
	g_dma_block = g_dma_block2;
	g_dma_block2 = swap;

	/* dma_cache_wback_inv((unsigned long) g_dma_block, (PAGE_SIZE >> 1));	 */

	*byte_offset = 0;

	return g_dma_block;
}

/*! \fn int deu_dma_buffer_free (u8 * dataptr, void *opt)
 *	\ingroup LTQ_DMA_FUNCTIONS
 *	\brief callback function for freeing dma transmit descriptors
 *	\param dataptr data pointer to be freed
 *	\param opt not used
*/
int deu_dma_buffer_free (u8 *dataptr, void *opt)
{
	return 0;
}

