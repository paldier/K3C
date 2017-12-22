/* $Id: he.c,v 1.1.2.1 2001/10/15 23:19:43 paulsch Exp $ */

/*

  he.c

  ForeRunnerHE ATM Adapter driver for ATM on Linux
  Copyright (C) 1999-2001  Naval Research Laboratory

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

/*

  he.c

  ForeRunnerHE ATM Adapter driver for ATM on Linux
  Copyright (C) 1999-2001  Naval Research Laboratory

  Permission to use, copy, modify and distribute this software and its
  documentation is hereby granted, provided that both the copyright
  notice and this permission notice appear in all copies of the software,
  derivative works or modified versions, and any portions thereof, and
  that both notices appear in supporting documentation.

  NRL ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
  DISCLAIMS ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER
  RESULTING FROM THE USE OF THIS SOFTWARE.

  This driver was written using the "Programmer's Reference Manual for
  ForeRunnerHE(tm)", MANU0361-01 - Rev. A, 08/21/98.

  AUTHORS:
	chas williams <chas@cmf.nrl.navy.mil>
	eric kinzie <ekinzie@cmf.nrl.navy.mil>

  NOTES:
	4096 supported 'connections'
	group 0 is used for all traffic
	interrupt queue 0 is used for all interrupts
	aal0 support for receive only

 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <asm/uaccess.h>

#include <linux/atmdev.h>
#include <linux/atm.h>
#include <linux/sonet.h>
#ifndef ATM_OC12_PCR
#define ATM_OC12_PCR (622080000/1080*1040/8/53)
#endif

#define USE_TASKLET

/* 2.2 kernel support */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,43)
#define dev_kfree_skb_irq(skb)		dev_kfree_skb(skb)
#define dev_kfree_skb_any(skb)		dev_kfree_skb(skb)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,18)
#define set_current_state(x)		current->state = (x);
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,43)
#undef USE_TASKLET
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,43)
#define pci_map_single(dev, addr, size, dir)	virt_to_bus(addr)
#define pci_unmap_single(dev, addr, size, dir)
#define pci_dma_sync_single(dev, addr, size, dir)
#define pci_set_dma_mask(dev, mask)			0
#endif


#include "he.h"

#ifdef USE_SUNI
#include "suni.h"
#endif /* USE_SUNI */

#include <linux/atm_he.h>

#define hprintk(fmt,args...)	printk(DEV_LABEL "%d: " fmt, he_dev->number, args)
#define hprintk1(fmt)		printk(DEV_LABEL "%d: " fmt, he_dev->number)

#undef DEBUG
#ifdef DEBUG
#define HPRINTK(fmt,args...)	hprintk(fmt,args)
#define HPRINTK1(fmt)		hprintk1(fmt)
#else
#define HPRINTK(fmt,args...)
#define HPRINTK1(fmt,args...)
#endif /* DEBUG */


/* version definition */

char kernel_version[] = UTS_RELEASE;

/* defines */
#define ALIGN_ADDRESS(addr, alignment) \
	((((unsigned long) (addr)) + (((unsigned long) (alignment)) - 1)) & ~(((unsigned long) (alignment)) - 1))

/* declarations */

static int he_open(struct atm_vcc *vcc, short vpi, int vci);
static void he_close(struct atm_vcc *vcc);
static int he_send(struct atm_vcc *vcc, struct sk_buff *skb);
static int he_sg_send(struct atm_vcc *vcc, unsigned long start, unsigned long size);
static int he_ioctl(struct atm_dev *dev, unsigned int cmd, void *arg);
static void he_irq_handler(int irq, void *dev_id, struct pt_regs *regs);
static void he_tasklet(unsigned long data);
static int he_proc_read(struct atm_dev *dev,loff_t *pos,char *page);
static int he_start(struct atm_dev *dev);
static void he_stop(struct he_dev *dev);
static void he_phy_put(struct atm_dev *, unsigned char, unsigned long);
static unsigned char he_phy_get(struct atm_dev *, unsigned long);

static u8 read_prom_byte(struct he_dev *he_dev, int addr);

/* globals */

struct he_dev *he_devs = NULL;
static short disable64 = -1;

static struct atmdev_ops he_ops =
{
   open:	he_open,
   close:	he_close,	
   ioctl:	he_ioctl,	
   send:	he_send,
   sg_send:	he_sg_send,	
   phy_put:	he_phy_put,
   phy_get:	he_phy_get,
   proc_read:	he_proc_read,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,1)
   owner:	THIS_MODULE
#endif
};

/* see the comments in he.h about global_lock */

#ifdef CONFIG_SMP
#define he_spin_lock(lock, flags)	spin_lock_irqsave(lock, flags)
#define he_spin_unlock(lock, flags)	spin_unlock_irqrestore(lock, flags)
#else
#define he_spin_lock(lock, flags)	flags = 0
#define he_spin_unlock(lock, flags)	flags = 0
#endif


#define he_writel(dev, val, reg)	writel(val, (dev)->membase + (reg))
#define he_readl(dev, reg)		readl((dev)->membase + (reg))

/* section 2.12 connection memory access */

static __inline__ void
he_writel_internal(struct he_dev *he_dev, unsigned val, unsigned addr,
								unsigned flags)
{
	he_writel(he_dev, val, CON_DAT);
	he_writel(he_dev, flags | CON_CTL_WRITE | CON_CTL_ADDR(addr) | CON_CTL_BUSY, CON_CTL);
	while(he_readl(he_dev, CON_CTL) & CON_CTL_BUSY);
}

#define he_writel_rcm(dev, val, reg) 				\
			he_writel_internal(dev, val, reg, CON_CTL_RCM)

#define he_writel_tcm(dev, val, reg) 				\
			he_writel_internal(dev, val, reg, CON_CTL_TCM)

#define he_writel_mbox(dev, val, reg) 				\
			he_writel_internal(dev, val, reg, CON_CTL_MBOX)

static __inline__ unsigned
he_readl_internal(struct he_dev *he_dev, unsigned addr, unsigned flags)
{
	he_writel(he_dev, flags | CON_CTL_READ | CON_CTL_ADDR(addr) | CON_CTL_BUSY, CON_CTL);
	while(he_readl(he_dev, CON_CTL) & CON_CTL_BUSY);
	return he_readl(he_dev, CON_DAT);
}

#define he_readl_rcm(dev, reg) \
			he_readl_internal(dev, reg, CON_CTL_RCM)

#define he_readl_tcm(dev, reg) \
			he_readl_internal(dev, reg, CON_CTL_TCM)

#define he_readl_mbox(dev, reg) \
			he_readl_internal(dev, reg, CON_CTL_MBOX)


/* figure 2.2 connection id */

#define he_mkcid(dev, vpi, vci)		(((vpi<<(dev)->vcibits) | vci) & 0x1fff)

/* 2.5.1 per connection transmit state registers */

#define he_writel_tsr0(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 0)
#define he_readl_tsr0(dev, cid) \
		he_readl_tcm(dev, CONFIG_TSRA | (cid<<3) | 0)

#define he_writel_tsr1(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 1)

#define he_writel_tsr2(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 2)

#define he_writel_tsr3(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 3)

#define he_writel_tsr4(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 4)

	/* from page 2-20
	 *
	 * NOTE While the transmit connection is active, bits 23 through 0
	 *      of this register must not be written by the host.  Byte
	 *      enables should be used during normal operation when writing
	 *      the most significant byte.
	 */

#define he_writel_tsr4_upper(dev, val, cid) \
		he_writel_internal(dev, val, CONFIG_TSRA | (cid<<3) | 4, \
							CON_CTL_TCM \
							| CON_BYTE_DISABLE_2 \
							| CON_BYTE_DISABLE_1 \
							| CON_BYTE_DISABLE_0)

#define he_readl_tsr4(dev, cid) \
		he_readl_tcm(dev, CONFIG_TSRA | (cid<<3) | 4)

#define he_writel_tsr5(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 5)

#define he_writel_tsr6(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 6)

#define he_writel_tsr7(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRA | (cid<<3) | 7)


#define he_writel_tsr8(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRB | (cid<<1) | 0)

#define he_writel_tsr9(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRB | (cid<<1) | 1)


#define he_writel_tsr10(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRC | (cid<<2) | 0)

#define he_writel_tsr11(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRC | (cid<<2) | 1)

#define he_writel_tsr12(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRC | (cid<<2) | 2)

#define he_writel_tsr13(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRC | (cid<<2) | 3)


#define he_writel_tsr14(dev, val, cid) \
		he_writel_tcm(dev, val, CONFIG_TSRD | cid)

#define he_writel_tsr14_upper(dev, val, cid) \
		he_writel_internal(dev, val, CONFIG_TSRD | cid, \
							CON_CTL_TCM \
							| CON_BYTE_DISABLE_2 \
							| CON_BYTE_DISABLE_1 \
							| CON_BYTE_DISABLE_0)

/* 2.7.1 per connection receive state registers */

#define he_writel_rsr0(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 0)
#define he_readl_rsr0(dev, cid) \
		he_readl_rcm(dev, 0x00000 | (cid<<3) | 0)

#define he_writel_rsr1(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 1)

#define he_writel_rsr2(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 2)

#define he_writel_rsr3(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 3)

#define he_writel_rsr4(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 4)

#define he_writel_rsr5(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 5)

#define he_writel_rsr6(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 6)

#define he_writel_rsr7(dev, val, cid) \
		he_writel_rcm(dev, val, 0x00000 | (cid<<3) | 7)

static int __devinit
he_init_one(struct pci_dev *pci_dev, const struct pci_device_id *pci_ent)
{
	struct atm_dev *atm_dev;
	struct he_dev *he_dev;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,43)
	if (pci_enable_device(pci_dev)) return -EIO;
#endif
	if (pci_set_dma_mask(pci_dev, HE_DMA_MASK) != 0) return -EIO;

	atm_dev = atm_dev_register(DEV_LABEL, &he_ops, -1, 0);
	if (!atm_dev) return -ENODEV;
	pci_dev->driver_data = atm_dev;

	he_dev = (struct he_dev *) kmalloc(sizeof(struct he_dev),
							GFP_KERNEL);
	if (!he_dev) return -ENOMEM;
	memset(he_dev, 0, sizeof(struct he_dev));

	he_dev->pci_dev = pci_dev;
	he_dev->atm_dev = atm_dev;
	he_dev->atm_dev->dev_data = he_dev;
	HE_DEV(atm_dev) = he_dev;
	he_dev->number = atm_dev->number;	/* was devs */
	if (he_start(atm_dev)) {
		atm_dev_deregister(atm_dev);
		he_stop(he_dev);
		kfree(he_dev);
		return -ENODEV;
	}
	he_dev->next = NULL;
	if (he_devs) he_dev->next = he_devs;
	he_devs = he_dev;

	return 0;
}

static void __devexit
he_remove_one (struct pci_dev *pci_dev)
{
	struct atm_dev *atm_dev;
	struct he_dev *he_dev;

	atm_dev = pci_dev->driver_data;
	he_dev = HE_DEV(atm_dev);

	/* need to remove from he_devs */

	atm_dev_deregister(atm_dev);
	he_stop(he_dev);
	kfree(he_dev);

	pci_dev->driver_data = NULL;
}


static unsigned
rate_to_atmf(unsigned rate)		/* cps to atm forum format */
{
#define NONZERO (1<<14)

        unsigned exp = 0;

        if (rate == 0) return(0);

        rate <<= 9;
        while (rate > 0x3ff)
        {
                ++exp;
                rate >>= 1;
        }

        return (NONZERO | (exp << 9) | (rate & 0x1ff));
}

static void __init
he_init_rx_lbfp0(struct he_dev *he_dev)
{
        unsigned i, lbm_offset, lbufd_index, lbuf_addr, lbuf_count;
        unsigned lbufs_per_row = he_dev->cells_per_row / he_dev->cells_per_lbuf;
        unsigned lbuf_bufsize = he_dev->cells_per_lbuf * ATM_CELL_PAYLOAD;
        unsigned row_offset = he_dev->r0_startrow * he_dev->bytes_per_row;
        
	lbufd_index = 0;
        lbm_offset = he_readl(he_dev, RCMLBM_BA);

	he_writel(he_dev, lbufd_index, RLBF0_H);

        for (i = 0, lbuf_count = 0; i < he_dev->r0_numbuffs; ++i)
        {
		lbufd_index += 2;
                lbuf_addr = (row_offset + (lbuf_count * lbuf_bufsize)) / 32;

		he_writel_rcm(he_dev, lbuf_addr, lbm_offset);
		he_writel_rcm(he_dev, lbufd_index, lbm_offset + 1);

                if (++lbuf_count == lbufs_per_row)
                {
                        lbuf_count = 0;
                        row_offset += he_dev->bytes_per_row;
                }
		lbm_offset += 4;
        }
                
        he_writel(he_dev, lbufd_index - 2, RLBF0_T);
	he_writel(he_dev, he_dev->r0_numbuffs, RLBF0_C);
}

static void __init
he_init_rx_lbfp1(struct he_dev *he_dev)
{
        unsigned i, lbm_offset, lbufd_index, lbuf_addr, lbuf_count;
        unsigned lbufs_per_row = he_dev->cells_per_row / he_dev->cells_per_lbuf;
        unsigned lbuf_bufsize = he_dev->cells_per_lbuf * ATM_CELL_PAYLOAD;
        unsigned row_offset = he_dev->r1_startrow * he_dev->bytes_per_row;
        
	lbufd_index = 1;
        lbm_offset = he_readl(he_dev, RCMLBM_BA) + (2 * lbufd_index);

	he_writel(he_dev, lbufd_index, RLBF1_H);

        for (i = 0, lbuf_count = 0; i < he_dev->r1_numbuffs; ++i)
        {
		lbufd_index += 2;
                lbuf_addr = (row_offset + (lbuf_count * lbuf_bufsize)) / 32;

		he_writel_rcm(he_dev, lbuf_addr, lbm_offset);
		he_writel_rcm(he_dev, lbufd_index, lbm_offset + 1);

                if (++lbuf_count == lbufs_per_row)
                {
                        lbuf_count = 0;
                        row_offset += he_dev->bytes_per_row;
                }
		lbm_offset += 4;
        }
                
        he_writel(he_dev, lbufd_index - 2, RLBF1_T);
	he_writel(he_dev, he_dev->r1_numbuffs, RLBF1_C);
}

static void __init
he_init_tx_lbfp(struct he_dev *he_dev)
{
        unsigned i, lbm_offset, lbufd_index, lbuf_addr, lbuf_count;
        unsigned lbufs_per_row = he_dev->cells_per_row / he_dev->cells_per_lbuf;
        unsigned lbuf_bufsize = he_dev->cells_per_lbuf * ATM_CELL_PAYLOAD;
        unsigned row_offset = he_dev->tx_startrow * he_dev->bytes_per_row;
        
	lbufd_index = he_dev->r0_numbuffs + he_dev->r1_numbuffs;
        lbm_offset = he_readl(he_dev, RCMLBM_BA) + (2 * lbufd_index);

	he_writel(he_dev, lbufd_index, TLBF_H);

        for (i = 0, lbuf_count = 0; i < he_dev->tx_numbuffs; ++i)
        {
		lbufd_index += 1;
                lbuf_addr = (row_offset + (lbuf_count * lbuf_bufsize)) / 32;

		he_writel_rcm(he_dev, lbuf_addr, lbm_offset);
		he_writel_rcm(he_dev, lbufd_index, lbm_offset + 1);

                if (++lbuf_count == lbufs_per_row)
                {
                        lbuf_count = 0;
                        row_offset += he_dev->bytes_per_row;
                }
		lbm_offset += 2;
        }
                
        he_writel(he_dev, lbufd_index - 1, TLBF_T);
}

static int __init
he_init_tpdrq(struct he_dev *he_dev)
{
	he_dev->tpdrq_p = kmalloc((CONFIG_TPDRQ_SIZE * sizeof(struct he_tpdrq))
				+ TPDRQ_ALIGNMENT, GFP_DMA);
	if (he_dev->tpdrq_p == NULL) 
	{
		hprintk1("failed to alloc tpdrq\n");
		return -ENOMEM;
	}
	he_dev->tpdrq_base = (struct he_tpdrq *)
			ALIGN_ADDRESS(he_dev->tpdrq_p, TPDRQ_ALIGNMENT);
	memset(he_dev->tpdrq_base, 0,
				CONFIG_TPDRQ_SIZE * sizeof(struct he_tpdrq));

	he_dev->tpdrq_tail = he_dev->tpdrq_base;
	he_dev->tpdrq_head = he_dev->tpdrq_base;

	he_writel(he_dev, virt_to_bus(he_dev->tpdrq_base), TPDRQ_B_H);
	he_writel(he_dev, 0, TPDRQ_T);	
	he_writel(he_dev, CONFIG_TPDRQ_SIZE - 1, TPDRQ_S);

	return 0;
}

static void __init
he_init_cs_block(struct he_dev *he_dev)
{
	unsigned clock, rate, delta;
	int reg;

	/* 5.1.7 cs block initialization */

	for(reg = 0; reg < 0x20; ++reg)
		he_writel_mbox(he_dev, 0x0, CS_STTIM0 + reg);

	/* rate grid timer reload values */

	clock = he_is622(he_dev) ? 66667000 : 50000000;
	rate = he_dev->atm_dev->link_rate;
	delta = rate / 16 / 2;

	for(reg = 0; reg < 0x10; ++reg)
	{
		/* 2.4 internal transmit function
		 *
	 	 * we initialize the first row in the rate grid.
		 * values are period (in clock cycles) of timer
		 */
		unsigned period = clock / rate;

		he_writel_mbox(he_dev, period, CS_TGRLD0 + reg);
		rate -= delta;
	}

	if (he_is622(he_dev))
	{
		/* table 5.2 (4 cells per lbuf) */
		he_writel_mbox(he_dev, 0x000800fa, CS_ERTHR0);
		he_writel_mbox(he_dev, 0x000c33cb, CS_ERTHR1);
		he_writel_mbox(he_dev, 0x0010101b, CS_ERTHR2);
		he_writel_mbox(he_dev, 0x00181dac, CS_ERTHR3);
		he_writel_mbox(he_dev, 0x00280600, CS_ERTHR4);

		/* table 5.3, 5.4, 5.5, 5.6, 5.7 */
		he_writel_mbox(he_dev, 0x023de8b3, CS_ERCTL0);
		he_writel_mbox(he_dev, 0x1801, CS_ERCTL1);
		he_writel_mbox(he_dev, 0x68b3, CS_ERCTL2);
		he_writel_mbox(he_dev, 0x1280, CS_ERSTAT0);
		he_writel_mbox(he_dev, 0x68b3, CS_ERSTAT1);
		he_writel_mbox(he_dev, 0x14585, CS_RTFWR);

		he_writel_mbox(he_dev, 0x4680, CS_RTATR);

		/* table 5.8 */
		he_writel_mbox(he_dev, 0x00159ece, CS_TFBSET);
		he_writel_mbox(he_dev, 0x68b3, CS_WCRMAX);
		he_writel_mbox(he_dev, 0x5eb3, CS_WCRMIN);
		he_writel_mbox(he_dev, 0xe8b3, CS_WCRINC);
		he_writel_mbox(he_dev, 0xdeb3, CS_WCRDEC);
		he_writel_mbox(he_dev, 0x68b3, CS_WCRCEIL);

		/* table 5.9 */
		he_writel_mbox(he_dev, 0x5, CS_OTPPER);
		he_writel_mbox(he_dev, 0x14, CS_OTWPER);
	}
	else
	{
		/* table 5.1 (4 cells per lbuf) */
		he_writel_mbox(he_dev, 0x000400ea, CS_ERTHR0);
		he_writel_mbox(he_dev, 0x00063388, CS_ERTHR1);
		he_writel_mbox(he_dev, 0x00081018, CS_ERTHR2);
		he_writel_mbox(he_dev, 0x000c1dac, CS_ERTHR3);
		he_writel_mbox(he_dev, 0x0014051a, CS_ERTHR4);

		/* table 5.3, 5.4, 5.5, 5.6, 5.7 */
		he_writel_mbox(he_dev, 0x0235e4b1, CS_ERCTL0);
		he_writel_mbox(he_dev, 0x4701, CS_ERCTL1);
		he_writel_mbox(he_dev, 0x64b1, CS_ERCTL2);
		he_writel_mbox(he_dev, 0x1280, CS_ERSTAT0);
		he_writel_mbox(he_dev, 0x64b1, CS_ERSTAT1);
		he_writel_mbox(he_dev, 0xf424, CS_RTFWR);

		he_writel_mbox(he_dev, 0x4680, CS_RTATR);

		/* table 5.8 */
		he_writel_mbox(he_dev, 0x000563b7, CS_TFBSET);
		he_writel_mbox(he_dev, 0x64b1, CS_WCRMAX);
		he_writel_mbox(he_dev, 0x5ab1, CS_WCRMIN);
		he_writel_mbox(he_dev, 0xe4b1, CS_WCRINC);
		he_writel_mbox(he_dev, 0xdab1, CS_WCRDEC);
		he_writel_mbox(he_dev, 0x64b1, CS_WCRCEIL);

		/* table 5.9 */
		he_writel_mbox(he_dev, 0x6, CS_OTPPER);
		he_writel_mbox(he_dev, 0x1e, CS_OTWPER);

	}

	he_writel_mbox(he_dev, 0x8, CS_OTTLIM);

	for(reg = 0; reg < 0x8; ++reg)
		he_writel_mbox(he_dev, 0x0, CS_HGRRT0 + reg);

}

static void __init
he_init_cs_block_rcm(struct he_dev *he_dev)
{
	unsigned rategrid[16][16];
	unsigned rate, delta;
	int i, j, reg;

	unsigned rate_atmf, exp, man;
	unsigned long long rate_cps;

	/* initialize rate grid group table */

	for (reg = 0x0; reg < 0xff; ++reg)
		he_writel_rcm(he_dev, 0x0, CONFIG_RCMABR + reg);

	/* initialize rate controller groups */

	for (reg = 0x100; reg < 0x1ff; ++reg)
		he_writel_rcm(he_dev, 0x0, CONFIG_RCMABR + reg);
	
	/* initialize tNrm lookup table */

	/* the manual makes reference to a routine in a sample driver
	   for proper configuration; fortunately, we only need this
	   in order to support abr connection */
	
	/* initialize rate to group table */

	rate = he_dev->atm_dev->link_rate;
	delta = rate / 32;

	/*
	 * 2.4 transmit internal functions
	 * 
	 * we construct a copy of the rate grid used by the scheduler
	 * in order to construct the rate to group table below
	 */

	for (j = 0; j < 16; j++)
	{
		rategrid[0][j] = rate;
		rate -= delta;
	}

	for (i = 1; i < 16; i++)
		for (j = 0; j < 16; j++)
			if (i > 14)
				rategrid[i][j] = rategrid[i - 1][j] / 4;
			else
				rategrid[i][j] = rategrid[i - 1][j] / 2;

	/*
	 * 2.4 transmit internal function
	 *
	 * this table maps the upper 5 bits of exponent and mantissa
	 * of the atm forum representation of the rate into an index
	 * on rate grid  
	 */

	rate_atmf = 0;
	while (rate_atmf < 0x400)
	{
		man = (rate_atmf & 0x1f) << 4;
		exp = rate_atmf >> 5;

		/* 
			instead of '/ 512', use '>> 9' to prevent a call
			to divdu3 on x86 platforms
		*/
		rate_cps = (unsigned long long) (1 << exp) * (man + 512) >> 9;

		if (rate_cps < 10) rate_cps = 10;
				/* 2.2.1 minimum payload rate is 10 cps */

		for (i = 255; i > 0; i--)
			if (rategrid[i/16][i%16] >= rate_cps) break;
				/* pick nearest rate instead? */

		/*
		 * each table entry is 16 bits (rate grid index, 8 bits,
		 * and possibly a buffer limit, 8 bits)
		 * there are two table entries in each 32-bit register
		 */

		reg = (reg<<16) | ((i<<8) | 0x04 /* ??? */);

#define RTGTBL_OFFSET 0x400
	  
		if (rate_atmf & 0x1)
			he_writel_rcm(he_dev, reg,
				CONFIG_RCMABR + RTGTBL_OFFSET + (rate_atmf>>1));

		++rate_atmf;
	}
}


static int __init
he_init_group(struct he_dev *he_dev, int group)
{
	int i;
	struct he_buff *hbuf;


	/* small buffer pool */

#ifdef rbps_support
	he_dev->rbps_p = kmalloc((CONFIG_RBPS_SIZE * sizeof(struct he_rbp))
				+ RBPS_ALIGNMENT, GFP_DMA);
	if (he_dev->rbps_p == NULL)
	{
		hprintk1("failed to alloc rbps\n");
		return -ENOMEM;
	}
	he_dev->rbps_base = (struct he_rbp *)
				ALIGN_ADDRESS(he_dev->rbps_p, RBPS_ALIGNMENT);
	memset(he_dev->rbps_base, 0, CONFIG_RBPS_SIZE * sizeof(struct he_rbp));

	for (i = 0; i < CONFIG_RBPS_SIZE; ++i)
	{
		void *data;

		data = kmalloc(CONFIG_RBPS_BUFSIZE + sizeof(struct he_buff)
						+ HBUF_ALIGNMENT, GFP_DMA);
		if (data == NULL)
		{
			hprintk1("failed to alloc rbps entry\n");
			return -ENOMEM;
		}
		hbuf = (struct he_buff *)
				ALIGN_ADDRESS(data + CONFIG_RBPS_BUFSIZE,
								HBUF_ALIGNMENT);
		hbuf->len = CONFIG_RBPS_BUFSIZE;
		hbuf->loaned = 1;
		hbuf->data = data;

		he_dev->rbps_base[i].virt = virt_to_bus(hbuf);
		he_dev->rbps_base[i].phys = virt_to_bus(hbuf->data);
	}
	hbuf->loaned = 0;

	he_dev->rbps_tail = &he_dev->rbps_base[CONFIG_RBPS_SIZE-1];

	he_writel(he_dev, virt_to_bus(he_dev->rbps_base),
						G0_RBPS_S + (group * 32));
	he_writel(he_dev, RBPS_MASK(he_dev->rbps_tail),
						G0_RBPS_T + (group * 32));
	he_writel(he_dev, CONFIG_RBPS_BUFSIZE/4,
						G0_RBPS_BS + (group * 32));
	he_writel(he_dev,
			RBP_THRESH(CONFIG_RBPS_THRESH) |
			RBP_QSIZE(CONFIG_RBPS_SIZE-1) |
			RBP_INT_ENB,
						G0_RBPS_QI + (group * 32));
#else
	he_writel(he_dev, 0x0, G0_RBPS_S + (group * 32));
	he_writel(he_dev, 0x0, G0_RBPS_T + (group * 32));
	he_writel(he_dev, 0x0, G0_RBPS_QI + (group * 32));
	he_writel(he_dev, RBP_THRESH(0x1) | RBP_QSIZE(0x0),
						G0_RBPS_BS + (group * 32));

#endif

	/* large buffer pool */

	he_dev->rbpl_p = kmalloc((CONFIG_RBPL_SIZE * sizeof(struct he_rbp))
				+ RBPL_ALIGNMENT, GFP_DMA);
	if (he_dev->rbpl_p == NULL)
	{
		hprintk1("failed to alloc rbpl\n");
		return -ENOMEM;
	}
	he_dev->rbpl_base = (struct he_rbp *)
				ALIGN_ADDRESS(he_dev->rbpl_p, RBPL_ALIGNMENT);
	memset(he_dev->rbpl_base, 0, CONFIG_RBPL_SIZE * sizeof(struct he_rbp));

	for (i = 0; i < CONFIG_RBPL_SIZE; ++i)
	{
		void *base;

		base = kmalloc(sizeof(struct he_buff) + HBUF_ALIGNMENT,
								GFP_KERNEL);
		if (base == NULL)
		{
			hprintk1("failed to alloc rbpl entry\n");
			return -ENOMEM;
		}
		hbuf = (struct he_buff *) ALIGN_ADDRESS(base, HBUF_ALIGNMENT);
		hbuf->base = base;

		hbuf->data = kmalloc(CONFIG_RBPL_BUFSIZE, GFP_DMA);
		if (hbuf->data == NULL)
		{
			hprintk1("failed to alloc rbpl buffer\n");
			return -ENOMEM;
		}
		hbuf->len = CONFIG_RBPL_BUFSIZE;
		hbuf->loaned = 1;

		he_dev->rbpl_base[i].virt = virt_to_bus(hbuf);
		hbuf->mapping = pci_map_single(he_dev->pci_dev, hbuf->data,
						hbuf->len, PCI_DMA_FROMDEVICE);
		he_dev->rbpl_base[i].phys = hbuf->mapping;
	}
	hbuf->loaned = 0;

	he_dev->rbpl_tail = &he_dev->rbpl_base[CONFIG_RBPL_SIZE-1];

	he_writel(he_dev, virt_to_bus(he_dev->rbpl_base),
						G0_RBPL_S + (group * 32));
	he_writel(he_dev, RBPL_MASK(he_dev->rbpl_tail),
						G0_RBPL_T + (group * 32));
	he_writel(he_dev, CONFIG_RBPL_BUFSIZE/4,
						G0_RBPL_BS + (group * 32));
	he_writel(he_dev,
			RBP_THRESH(CONFIG_RBPL_THRESH) |
			RBP_QSIZE(CONFIG_RBPL_SIZE-1) |
			RBP_INT_ENB,
						G0_RBPL_QI + (group * 32));

	/* rx buffer ready queue */

	he_dev->rbrq_p = (struct he_rbrq *) kmalloc(CONFIG_RBRQ_SIZE *
			sizeof(struct he_rbrq) + RBRQ_ALIGNMENT, GFP_DMA);
	if (he_dev->rbrq_p == NULL)
	{
		hprintk1("failed to allocate rbrq\n");
		return -ENOMEM;
	}
	he_dev->rbrq_base = (struct he_rbrq *)
				ALIGN_ADDRESS(he_dev->rbrq_p, RBRQ_ALIGNMENT);
	memset(he_dev->rbrq_base, 0, CONFIG_RBRQ_SIZE * sizeof(struct he_rbrq));

	he_dev->rbrq_head = he_dev->rbrq_base;
	he_writel(he_dev, virt_to_bus(he_dev->rbrq_base), G0_RBRQ_ST + (group * 16));
	he_writel(he_dev, 0, G0_RBRQ_H + (group * 16));
	he_writel(he_dev,
		RBRQ_THRESH(CONFIG_RBRQ_THRESH) | RBRQ_SIZE(CONFIG_RBRQ_SIZE-1),
						G0_RBRQ_Q + (group * 16));
#ifdef notdef
	he_writel(he_dev, RBRQ_TIME(10) | RBRQ_COUNT(5),
						G0_RBRQ_I + (group * 16));
#else
	he_writel(he_dev, RBRQ_TIME(0) | RBRQ_COUNT(1),
						G0_RBRQ_I + (group * 16));
#endif


	/* tx buffer ready queue */

	he_dev->tbrq_p = kmalloc(CONFIG_TBRQ_SIZE * sizeof(struct he_tbrq)
				+ TBRQ_ALIGNMENT, GFP_DMA);
	if (he_dev->tbrq_p == NULL)
	{
		hprintk1("failed to allocate tbrq\n");
		return -ENOMEM;
	}
	he_dev->tbrq_base = (struct he_tbrq *)
				ALIGN_ADDRESS(he_dev->tbrq_p, TBRQ_ALIGNMENT);
	memset(he_dev->tbrq_base, 0, CONFIG_TBRQ_SIZE * sizeof(struct he_tbrq));

	he_dev->tbrq_head = he_dev->tbrq_base;

	he_writel(he_dev, virt_to_bus(he_dev->tbrq_base),
						G0_TBRQ_B_T + (group * 16));
	he_writel(he_dev, 0, G0_TBRQ_H + (group * 16));
	he_writel(he_dev, CONFIG_TBRQ_SIZE - 1, G0_TBRQ_S + (group * 16));
	he_writel(he_dev, CONFIG_TBRQ_THRESH, G0_TBRQ_THRESH + (group * 16));

	return 0;
}

static int __init
he_init_irq(struct he_dev *he_dev)
{
	int i;

	/* 2.9.3.5  tail offset for each interrupt queue is located after the
		    end of the interrupt queue */

	he_dev->irq_p = kmalloc((CONFIG_IRQ_SIZE * sizeof(struct he_irq))
				+ sizeof(unsigned) + IRQ_ALIGNMENT, GFP_DMA);
	if (he_dev->irq_p == NULL)
	{
		hprintk1("failed to allocate irq\n");
		return -ENOMEM;
	}
	he_dev->irq_base = (struct he_irq *)
				ALIGN_ADDRESS(he_dev->irq_p, IRQ_ALIGNMENT);
	he_dev->irq_tailoffset = (unsigned *)
					&he_dev->irq_base[CONFIG_IRQ_SIZE];
	*he_dev->irq_tailoffset = 0;
	he_dev->irq_head = he_dev->irq_base;
	he_dev->irq_tail = he_dev->irq_base;

	for(i=0; i < CONFIG_IRQ_SIZE; ++i)
		he_dev->irq_base[i].isw = ITYPE_INVALID;

	he_writel(he_dev, virt_to_bus(he_dev->irq_base), IRQ0_BASE);
	he_writel(he_dev,
		IRQ_SIZE(CONFIG_IRQ_SIZE) | IRQ_THRESH(CONFIG_IRQ_THRESH),
								IRQ0_HEAD);
	he_writel(he_dev, IRQ_INT_A | IRQ_TYPE_LINE, IRQ0_CNTL);
	he_writel(he_dev, 0x0, IRQ0_DATA);

	he_writel(he_dev, 0x0, IRQ1_BASE);
	he_writel(he_dev, 0x0, IRQ1_HEAD);
	he_writel(he_dev, 0x0, IRQ1_CNTL);
	he_writel(he_dev, 0x0, IRQ1_DATA);

	he_writel(he_dev, 0x0, IRQ2_BASE);
	he_writel(he_dev, 0x0, IRQ2_HEAD);
	he_writel(he_dev, 0x0, IRQ2_CNTL);
	he_writel(he_dev, 0x0, IRQ2_DATA);

	he_writel(he_dev, 0x0, IRQ3_BASE);
	he_writel(he_dev, 0x0, IRQ3_HEAD);
	he_writel(he_dev, 0x0, IRQ3_CNTL);
	he_writel(he_dev, 0x0, IRQ3_DATA);

	/* 2.9.3.2 interrupt queue mapping registers */

	he_writel(he_dev, 0x0, GRP_10_MAP);
	he_writel(he_dev, 0x0, GRP_32_MAP);
	he_writel(he_dev, 0x0, GRP_54_MAP);
	he_writel(he_dev, 0x0, GRP_76_MAP);

	if (request_irq(he_dev->pci_dev->irq, he_irq_handler, SA_INTERRUPT|SA_SHIRQ, DEV_LABEL, he_dev))
	{
		hprintk("irq %d already in use\n", he_dev->pci_dev->irq);
		return -EINVAL;
        }   

	he_dev->irq = he_dev->pci_dev->irq;

	return 0;
}

static int __init
he_start(struct atm_dev *dev)
{
        struct he_dev *he_dev;
        struct pci_dev *pci_dev;

	u16 command;
	u32 gen_cntl_0, host_cntl, lb_swap;
	
	unsigned err;
	unsigned int status, reg;
	int i, group;

        he_dev = HE_DEV(dev);
        pci_dev = he_dev->pci_dev;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,3)
	he_dev->membase = pci_dev->resource[0].start;
#else
	he_dev->membase = pci_dev->base_address[0] & PCI_BASE_ADDRESS_MEM_MASK;
#endif
	HPRINTK("membase = 0x%x  irq = %d.\n", he_dev->membase, pci_dev->irq);

	/*
	 * pci bus controller initialization 
	 */

	/* 4.3 pci bus controller-specific initialization */
	if (pci_read_config_dword(pci_dev, GEN_CNTL_0, &gen_cntl_0) != 0)
	{
		hprintk1("can't read GEN_CNTL_0\n");
		return -EINVAL;
	}
	gen_cntl_0 |= (MRL_ENB | MRM_ENB | IGNORE_TIMEOUT);
	if (pci_write_config_dword(pci_dev, GEN_CNTL_0, gen_cntl_0) != 0)
	{
		hprintk1("can't write GEN_CNTL_0.\n");
		return -EINVAL;
	}

	if (pci_read_config_word(pci_dev, PCI_COMMAND, &command) != 0)
	{
		hprintk1("can't read PCI_COMMAND.\n");
		return -EINVAL;
	}

	command |= (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_INVALIDATE);
	if (pci_write_config_word(pci_dev, PCI_COMMAND, command) != 0)
	{
		hprintk1("can't enable memory.\n");
		return -EINVAL;
	}

	if (!(he_dev->membase = (unsigned long) ioremap(he_dev->membase, HE_REGMAP_SIZE))) {
		hprintk1("can't set up page mapping\n");
		return -EINVAL;
	}
	      
	/* 4.4 card reset */
	he_writel(he_dev, 0x0, RESET_CNTL);
	he_writel(he_dev, 0xff, RESET_CNTL);

	udelay(16*1000);	/* 16 ms */
	status = he_readl(he_dev, RESET_CNTL);
	if ((status & BOARD_RST_STATUS) == 0)
	{
		hprintk1("reset failed\n");
		return -EINVAL;
	}

	/* 4.5 set bus width */
	host_cntl = he_readl(he_dev, HOST_CNTL);
	if (host_cntl & PCI_BUS_SIZE64)
		gen_cntl_0 |= ENBL_64;
	else
		gen_cntl_0 &= ~ENBL_64;

	if (disable64 == 1)
	{
		hprintk1("disabling 64-bit pci bus transfers\n");
		gen_cntl_0 &= ~ENBL_64;
	}

	if (gen_cntl_0 & ENBL_64) hprintk1("64-bit transfers enabled\n");

	pci_write_config_dword(pci_dev, GEN_CNTL_0, gen_cntl_0);

	/* 4.7 read prom contents */
	for(i=0; i<PROD_ID_LEN; ++i)
		he_dev->prod_id[i] = read_prom_byte(he_dev, PROD_ID + i);

	he_dev->media = read_prom_byte(he_dev, MEDIA);

	for(i=0; i<6; ++i)
		dev->esi[i] = read_prom_byte(he_dev, MAC_ADDR + i);

	hprintk("%s%s, %x:%x:%x:%x:%x:%x\n",
				he_dev->prod_id,
					he_dev->media & 0x40 ? "SM" : "MM",
						dev->esi[0],
						dev->esi[1],
						dev->esi[2],
						dev->esi[3],
						dev->esi[4],
						dev->esi[5]);
	he_dev->atm_dev->link_rate = he_is622(he_dev) ?
						ATM_OC12_PCR : ATM_OC3_PCR;

	/* 4.6 set host endianess */
	lb_swap = he_readl(he_dev, LB_SWAP);
	if (he_is622(he_dev))
		lb_swap &= ~XFER_SIZE;		/* 4 cells */
	else
		lb_swap |= XFER_SIZE;		/* 8 cells */
#ifdef __BIG_ENDIAN
	lb_swap |= DESC_WR_SWAP | INTR_SWAP | BIG_ENDIAN_HOST;
#else
	lb_swap &= ~(DESC_WR_SWAP | INTR_SWAP | BIG_ENDIAN_HOST |
			DATA_WR_SWAP | DATA_RD_SWAP | DESC_RD_SWAP);
#endif /* __BIG_ENDIAN */
	he_writel(he_dev, lb_swap, LB_SWAP);

	/* 4.8 sdram controller initialization */
	he_writel(he_dev, he_is622(he_dev) ? LB_64_ENB : 0x0, SDRAM_CTL);

	/* 4.9 initialize rnum value */
	lb_swap |= SWAP_RNUM_MAX(0xf);
	he_writel(he_dev, lb_swap, LB_SWAP);

	/* 4.10 initialize the interrupt queues */
	if ((err = he_init_irq(he_dev)) != 0) return err;

#ifdef USE_TASKLET
	tasklet_init(&he_dev->tasklet, he_tasklet, (unsigned long) he_dev);
#endif
	spin_lock_init(&he_dev->global_lock);
	spin_lock_init(&he_dev->tpdrq_lock);

	/*
	 * atm network controller initialization
	 */

	/* 5.1.1 generic configuration state */

	/*
	 *		local (cell) buffer memory map
	 *                    
	 *             HE155                          HE622
	 *                                                      
	 *        0 ____________1023 bytes  0 _______________________2047 bytes
	 *         |            |            |                   |   |
	 *         |  utility   |            |        rx0        |   |
	 *        5|____________|         255|___________________| u |
	 *        6|            |         256|                   | t |
	 *         |            |            |                   | i |
	 *         |    rx0     |     row    |        tx         | l |
	 *         |            |            |                   | i |
	 *         |            |         767|___________________| t |
	 *      517|____________|         768|                   | y |
	 * row  518|            |            |        rx1        |   |
	 *         |            |        1023|___________________|___|
	 *         |            |
	 *         |    tx      |
	 *         |            |
	 *         |            |
	 *     1535|____________|
	 *     1536|            |
	 *         |    rx1     |
	 *     2047|____________|
	 *
	 */

	he_dev->vcibits = CONFIG_VCIBITS;	/* 4096 connections */
	he_dev->vpibits = CONFIG_VPIBITS;

	if (he_is622(he_dev))
	{
		he_dev->cells_per_row = 40;
		he_dev->bytes_per_row = 2048;
		he_dev->r0_numrows = 256;
		he_dev->tx_numrows = 512;
		he_dev->r1_numrows = 256;
		he_dev->r0_startrow = 0;
		he_dev->tx_startrow = 256;
		he_dev->r1_startrow = 768;
	}
	else
	{
		he_dev->cells_per_row = 20;
		he_dev->bytes_per_row = 1024;
		he_dev->r0_numrows = 512;
		he_dev->tx_numrows = 1018;
		he_dev->r1_numrows = 512;
		he_dev->r0_startrow = 6;
		he_dev->tx_startrow = 518;
		he_dev->r1_startrow = 1536;
	}

	he_dev->cells_per_lbuf = 4;
	he_dev->buffer_limit = 4;
	he_dev->r0_numbuffs = he_dev->r0_numrows *
				he_dev->cells_per_row / he_dev->cells_per_lbuf;
	if (he_dev->r0_numbuffs > 2560) he_dev->r0_numbuffs = 2560;

	he_dev->r1_numbuffs = he_dev->r1_numrows *
				he_dev->cells_per_row / he_dev->cells_per_lbuf;
	if (he_dev->r1_numbuffs > 2560) he_dev->r1_numbuffs = 2560;

	he_dev->tx_numbuffs = he_dev->tx_numrows *
				he_dev->cells_per_row / he_dev->cells_per_lbuf;
	if (he_dev->tx_numbuffs > 5120) he_dev->tx_numbuffs = 5120;

	/* 5.1.2 configure hardware dependent registers */

	he_writel(he_dev, 
		SLICE_X(0x2) | ARB_RNUM_MAX(0xf) | TH_PRTY(0x3) |
		RH_PRTY(0x3) | TL_PRTY(0x2) | RL_PRTY(0x1) |
		(he_is622(he_dev) ? BUS_MULTI(0x28) : BUS_MULTI(0x46)) |
		(he_is622(he_dev) ? NET_PREF(0x50) : NET_PREF(0x8c)),
								LBARB);

	he_writel(he_dev, BANK_ON |
		(he_is622(he_dev) ? (REF_RATE(0x384) | WIDE_DATA) : REF_RATE(0x150)),
								SDRAMCON);

	he_writel(he_dev,
		(he_is622(he_dev) ? RM_BANK_WAIT(1) : RM_BANK_WAIT(0)) |
						RM_RW_WAIT(1), RCMCONFIG);
	he_writel(he_dev,
		(he_is622(he_dev) ? TM_BANK_WAIT(2) : TM_BANK_WAIT(1)) |
						TM_RW_WAIT(1), TCMCONFIG);

	he_writel(he_dev, he_dev->cells_per_lbuf * ATM_CELL_PAYLOAD, LB_CONFIG);

	he_writel(he_dev, 
		(he_is622(he_dev) ? UT_RD_DELAY(8) : UT_RD_DELAY(0)) |
		(he_is622(he_dev) ? RC_UT_MODE(0) : RC_UT_MODE(1)) |
		RX_VALVP(he_dev->vpibits) |
		RX_VALVC(he_dev->vcibits),			 RC_CONFIG);

	he_writel(he_dev, DRF_THRESH(0x20) |
		(he_is622(he_dev) ? TX_UT_MODE(0) : TX_UT_MODE(1)) |
		TX_VCI_MASK(he_dev->vcibits) |
		LBFREE_CNT(he_dev->tx_numbuffs), 		TX_CONFIG);

	he_writel(he_dev, 0x0, TXAAL5_PROTO);

	he_writel(he_dev, PHY_INT_ENB |
			he_is622(he_dev) ? PTMR_PRE(0x41) : PTMR_PRE(0x31),
								RH_CONFIG);

	/* 5.1.3 initialize connection memory */

	for(i=0; i < TCM_MEM_SIZE; ++i)
		he_writel_tcm(he_dev, 0, i);

	for(i=0; i < RCM_MEM_SIZE; ++i)
		he_writel_rcm(he_dev, 0, i);


	/*
	 *	transmit connection memory map
	 *
	 *                  tx memory
	 *          0x0 ___________________
	 *             |                   |
	 *             |                   |
	 *             |       TSRa        |
	 *             |                   |
	 *             |                   |
	 *       0x8000|___________________|
	 *             |                   |
	 *             |       TSRb        |
	 *       0xc000|___________________|
	 *             |                   |
	 *             |       TSRc        |
	 *       0xe000|___________________|
	 *             |       TSRd        |
	 *       0xf000|___________________|
	 *             |       tmABR       |
	 *      0x10000|___________________|
	 *             |                   |
	 *             |       tmTPD       |
	 *             |___________________|
	 *             |                   |
	 *                      ....
	 *      0x1ffff|___________________|
	 *
	 *
	 */

	he_writel(he_dev, CONFIG_TSRB, TSRB_BA);
	he_writel(he_dev, CONFIG_TSRC, TSRC_BA);
	he_writel(he_dev, CONFIG_TSRD, TSRD_BA);
	he_writel(he_dev, 0x0f000, TMABR_BA);
	he_writel(he_dev, 0x10000, TPD_BA);


	/*
	 *	receive connection memory map
	 *
	 *          0x0 ___________________
	 *             |                   |
	 *             |                   |
	 *             |       RSRa        |
	 *             |                   |
	 *             |                   |
	 *       0x8000|___________________|
	 *             |                   |
	 *             |             rx0/1 |
	 *             |       LBM         |   link lists of local
	 *             |             tx    |   buffer memory 
	 *             |                   |
	 *       0xd000|___________________|
	 *             |                   |
	 *             |      rmABR        |
	 *       0xe000|___________________|
	 *             |                   |
	 *             |       RSRb        |
	 *             |___________________|
	 *             |                   |
	 *                      ....
	 *       0xffff|___________________|
	 */

	he_writel(he_dev, 0x08000, RCMLBM_BA);
	he_writel(he_dev, 0x0e000, RCMRSRB_BA);
	he_writel(he_dev, 0x0d800, RCMABR_BA);

	/* 5.1.4 initialize local buffer free pools linked lists */

	he_init_rx_lbfp0(he_dev);
	he_init_rx_lbfp1(he_dev);

	he_writel(he_dev, 0x0, RLBC_H);
	he_writel(he_dev, 0x0, RLBC_T);
	he_writel(he_dev, 0x0, RLBC_H2);

	he_writel(he_dev, 512, RXTHRSH);	/* 10% of r0+r1 buffers */
	he_writel(he_dev, 256, LITHRSH); 	/* 5% of r0+r1 buffers */

	he_init_tx_lbfp(he_dev);

	he_writel(he_dev, he_is622(he_dev) ? 0x104780 : 0x800, UBUFF_BA);

	/* 5.1.5 initialize intermediate receive queues */

	if (he_is622(he_dev))
	{
		he_writel(he_dev, 0x000f, G0_INMQ_S);
		he_writel(he_dev, 0x200f, G0_INMQ_L);

		he_writel(he_dev, 0x001f, G1_INMQ_S);
		he_writel(he_dev, 0x201f, G1_INMQ_L);

		he_writel(he_dev, 0x002f, G2_INMQ_S);
		he_writel(he_dev, 0x202f, G2_INMQ_L);

		he_writel(he_dev, 0x003f, G3_INMQ_S);
		he_writel(he_dev, 0x203f, G3_INMQ_L);

		he_writel(he_dev, 0x004f, G4_INMQ_S);
		he_writel(he_dev, 0x204f, G4_INMQ_L);

		he_writel(he_dev, 0x005f, G5_INMQ_S);
		he_writel(he_dev, 0x205f, G5_INMQ_L);

		he_writel(he_dev, 0x006f, G6_INMQ_S);
		he_writel(he_dev, 0x206f, G6_INMQ_L);

		he_writel(he_dev, 0x007f, G7_INMQ_S);
		he_writel(he_dev, 0x207f, G7_INMQ_L);
	}
	else
	{
		he_writel(he_dev, 0x0000, G0_INMQ_S);
		he_writel(he_dev, 0x0008, G0_INMQ_L);

		he_writel(he_dev, 0x0001, G1_INMQ_S);
		he_writel(he_dev, 0x0009, G1_INMQ_L);

		he_writel(he_dev, 0x0002, G2_INMQ_S);
		he_writel(he_dev, 0x000a, G2_INMQ_L);

		he_writel(he_dev, 0x0003, G3_INMQ_S);
		he_writel(he_dev, 0x000b, G3_INMQ_L);

		he_writel(he_dev, 0x0004, G4_INMQ_S);
		he_writel(he_dev, 0x000c, G4_INMQ_L);

		he_writel(he_dev, 0x0005, G5_INMQ_S);
		he_writel(he_dev, 0x000d, G5_INMQ_L);

		he_writel(he_dev, 0x0006, G6_INMQ_S);
		he_writel(he_dev, 0x000e, G6_INMQ_L);

		he_writel(he_dev, 0x0007, G7_INMQ_S);
		he_writel(he_dev, 0x000f, G7_INMQ_L);
	}

	/* 5.1.6 application tunable parameters */

	he_writel(he_dev, 0x0, MCC);
	he_writel(he_dev, 0x0, OEC);
	he_writel(he_dev, 0x0, DCC);
	he_writel(he_dev, 0x0, CEC);
	
	/* 5.1.7 cs block initialization */

	he_init_cs_block(he_dev);

	/* 5.1.8 cs block connection memory initialization */
	
	he_init_cs_block_rcm(he_dev);

	/* 5.1.10 initialize host structures */

	he_init_tpdrq(he_dev);

	if (he_init_group(he_dev, 0) != 0)
		return -ENOMEM;

	for (group = 1; group < HE_NUM_GROUPS; ++group)
	{
		he_writel(he_dev, 0x0, G0_RBPS_S + (group * 32));
		he_writel(he_dev, 0x0, G0_RBPS_T + (group * 32));
		he_writel(he_dev, 0x0, G0_RBPS_QI + (group * 32));
		he_writel(he_dev, RBP_THRESH(0x1) | RBP_QSIZE(0x0),
						G0_RBPS_BS + (group * 32));

		he_writel(he_dev, 0x0, G0_RBPL_S + (group * 32));
		he_writel(he_dev, 0x0, G0_RBPL_T + (group * 32));
		he_writel(he_dev, RBP_THRESH(0x1) | RBP_QSIZE(0x0),
						G0_RBPL_QI + (group * 32));
		he_writel(he_dev, 0x0, G0_RBPL_BS + (group * 32));

		he_writel(he_dev, 0x0, G0_RBRQ_ST + (group * 16));
		he_writel(he_dev, 0x0, G0_RBRQ_H + (group * 16));
		he_writel(he_dev, RBRQ_THRESH(0x1) | RBRQ_SIZE(0x0),
						G0_RBRQ_Q + (group * 16));
		he_writel(he_dev, 0x0, G0_RBRQ_I + (group * 16));

		he_writel(he_dev, 0x0, G0_TBRQ_B_T + (group * 16));
		he_writel(he_dev, 0x0, G0_TBRQ_H + (group * 16));
		he_writel(he_dev, TBRQ_THRESH(0x1),
						G0_TBRQ_THRESH + (group * 16));
		he_writel(he_dev, 0x0, G0_TBRQ_S + (group * 16));
	}

	/* host status page */

	he_dev->hsp_p = kmalloc(sizeof(struct he_hsp) + HSP_ALIGNMENT, GFP_DMA);
	if (he_dev->hsp_p == NULL)
	{
		hprintk1("failed to allocate host status page\n");
		return -ENOMEM;
	}
	he_dev->hsp = (struct he_hsp *)
				ALIGN_ADDRESS(he_dev->hsp_p, HSP_ALIGNMENT);
	memset(he_dev->hsp, 0, sizeof(struct he_hsp));
	he_writel(he_dev, virt_to_bus(he_dev->hsp), HSP_BA);

	/* initialize framer */

#ifdef USE_SUNI
	suni_init(he_dev->atm_dev);
	if (he_dev->atm_dev->phy && he_dev->atm_dev->phy->start)
		he_dev->atm_dev->phy->start(he_dev->atm_dev);
#endif /* USE_SUNI */

	/* 4.11 enable pci bus controller state machines */
	host_cntl |= (OUTFF_ENB | CMDFF_ENB |
				QUICK_RD_RETRY | QUICK_WR_RETRY | PERR_INT_ENB);
	he_writel(he_dev, host_cntl, HOST_CNTL);

	gen_cntl_0 |= INT_PROC_ENBL|INIT_ENB;
	pci_write_config_dword(pci_dev, GEN_CNTL_0, gen_cntl_0);


	/* 5.1.12 enable transmit and receive */

	reg = he_readl_mbox(he_dev, CS_ERCTL0);
	reg |= TX_ENABLE|ER_ENABLE;
	he_writel_mbox(he_dev, reg, CS_ERCTL0);

	reg = he_readl(he_dev, RC_CONFIG);
	reg |= RX_ENABLE;
	he_writel(he_dev, reg, RC_CONFIG);


	he_dev->he_vcc_table = kmalloc(sizeof(struct he_vcc_table) * 
			(2 << (he_dev->vcibits + he_dev->vpibits)), GFP_KERNEL);
	if (he_dev->he_vcc_table == NULL)
	{
		hprintk1("failed to alloc he_vcc_table\n");
		return -ENOMEM;
	}
	memset(he_dev->he_vcc_table, 0, sizeof(struct he_vcc_table) *
				(2 << (he_dev->vcibits + he_dev->vpibits)));

	for (i = 0; i < HE_NUM_CS_STPER; ++i)
	{
		he_dev->cs_stper[i].inuse = 0;
		he_dev->cs_stper[i].pcr = -1;
	}
	he_dev->total_bw = 0;


	/* atm linux initialization */

	he_dev->atm_dev->ci_range.vpi_bits = he_dev->vpibits;
	he_dev->atm_dev->ci_range.vci_bits = he_dev->vcibits;

	he_dev->irq_peak = 0;
	he_dev->rbrq_peak = 0;
        he_dev->rbpl_peak = 0;
        he_dev->tbrq_peak = 0;

	hprintk1("hell bent for leather!\n");

	return 0;
}

static void
he_stop(struct he_dev *he_dev)
{
	u16 command;
	u32 gen_cntl_0, reg;
	struct pci_dev *pci_dev;
	int i;

	pci_dev = he_dev->pci_dev;

	/* disable interrupts */

	if (he_dev->membase)
	{
		pci_read_config_dword(pci_dev, GEN_CNTL_0, &gen_cntl_0);
		gen_cntl_0 &= ~(INT_PROC_ENBL | INIT_ENB);
		pci_write_config_dword(pci_dev, GEN_CNTL_0, gen_cntl_0);

#ifdef USE_TASKLET
		tasklet_disable(&he_dev->tasklet);
#endif

		/* disable recv and transmit */

		reg = he_readl_mbox(he_dev, CS_ERCTL0);
		reg |= ~(TX_ENABLE|ER_ENABLE);
		he_writel_mbox(he_dev, reg, CS_ERCTL0);

		reg = he_readl(he_dev, RC_CONFIG);
		reg |= ~(RX_ENABLE);
		he_writel(he_dev, reg, RC_CONFIG);
	}

	if (he_dev->irq)
		free_irq(he_dev->irq, he_dev);

	if (he_dev->irq_p)
		kfree(he_dev->irq_p);

	if (he_dev->hsp_p)
		kfree(he_dev->hsp_p);

	if (he_dev->rbpl_p)
	{
		for (i=0; i<CONFIG_RBPL_SIZE; ++i)
		{
			struct he_buff *hbuf = bus_to_virt(he_dev->rbpl_base[i].virt);

			pci_unmap_single(he_dev->pci_dev, hbuf->mapping, hbuf->len, PCI_DMA_FROMDEVICE);
			if (hbuf->data) kfree(hbuf->data);
			if (hbuf->base) kfree(hbuf->base);
		}
		kfree(he_dev->rbpl_p);
	}

#ifdef rbps_support
	if (he_dev->rbps_p)
	{
		for (i=0; i<CONFIG_RBPS_SIZE; ++i)
			kfree(he_dev->rbps_base[i].hbuf->data);
		kfree(he_dev->rbps_p);
	}
#endif

	if (he_dev->rbrq_p)
		kfree(he_dev->rbrq_p);

	if (he_dev->tbrq_p)
		kfree(he_dev->tbrq_p);

	if (he_dev->tpdrq_p)
		kfree(he_dev->tpdrq_p);

	if (he_dev->he_vcc_table)
		kfree(he_dev->he_vcc_table);

	if (he_dev->pci_dev)
	{
		pci_read_config_word(he_dev->pci_dev, PCI_COMMAND, &command);
		command &= ~(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER);
		pci_write_config_word(he_dev->pci_dev, PCI_COMMAND, command);
	}
	
	if (he_dev->membase) iounmap((void *) he_dev->membase);
}

static __inline__ struct he_tpd *
he_alloc_tpd(void)
{
	void *tpd_base;
	struct he_tpd *tpd;

	tpd_base = kmalloc(sizeof(struct he_tpd) + TPD_ALIGNMENT,
						GFP_ATOMIC | GFP_DMA);
	if (tpd_base == NULL)
		return NULL;

	tpd = (struct he_tpd *) ALIGN_ADDRESS(tpd_base, TPD_ALIGNMENT);
	memset(tpd, 0, sizeof(struct he_tpd));
	tpd->base = tpd_base;
	tpd->status = BUF_ADDR(virt_to_bus(tpd));

	return tpd;
}


#define AAL5_LEN(buf,len) 						\
			((((unsigned char *)(buf))[(len)-6]<<8) |	\
				(((unsigned char *)(buf))[(len)-5]))

static int
he_service_rbrq(struct he_dev *he_dev, int group)
{
	struct he_rbrq *rbrq_tail = (struct he_rbrq *)
				((unsigned long)he_dev->rbrq_base |
					he_dev->hsp->group[group].rbrq_tail);
	struct he_buff *hbuf;
	unsigned cid;
	unsigned buf_len;
	struct sk_buff *skb;
	struct atm_vcc *vcc;
	struct he_vcc *he_vcc;
	struct iovec *iov;
	int pdus_assembled = 0;
	int updated = 0;

	while (he_dev->rbrq_head != rbrq_tail)
	{
		++updated;

		HPRINTK("%p rbrq%d 0x%x len=%d cid=0x%x %s%s%s%s%s%s\n",
			he_dev->rbrq_head, group,
			RBRQ_ADDR(he_dev->rbrq_head),
			RBRQ_BUFLEN(he_dev->rbrq_head),
			RBRQ_CID(he_dev->rbrq_head),
			RBRQ_CRC_ERR(he_dev->rbrq_head) ? " CRC_ERR" : "",
			RBRQ_LEN_ERR(he_dev->rbrq_head) ? " LEN_ERR" : "",
			RBRQ_END_PDU(he_dev->rbrq_head) ? " END_PDU" : "",
			RBRQ_AAL5_PROT(he_dev->rbrq_head) ? " AAL5_PROT" : "",
			RBRQ_CON_CLOSED(he_dev->rbrq_head) ? " CON_CLOSED" : "",
			RBRQ_HBUF_ERR(he_dev->rbrq_head) ? " HBUF_ERR" : "");

		hbuf = (struct he_buff *)
				bus_to_virt(RBRQ_ADDR(he_dev->rbrq_head));
		buf_len = RBRQ_BUFLEN(he_dev->rbrq_head) * 4;
		cid = RBRQ_CID(he_dev->rbrq_head);

		vcc = HE_LOOKUP_VCC(he_dev, cid);
		if (vcc == NULL)
		{
			hprintk("vcc == NULL  (cid 0x%x)\n", cid);
			if (!RBRQ_HBUF_ERR(he_dev->rbrq_head)) hbuf->loaned = 0;
			goto next_rbrq_entry;
		}

		he_vcc = HE_VCC(vcc);
		if (vcc == NULL)
		{
			hprintk("he_vcc == NULL  (cid 0x%x)\n", cid);
			if (!RBRQ_HBUF_ERR(he_dev->rbrq_head)) hbuf->loaned = 0;
			goto next_rbrq_entry;
		}

		if (RBRQ_HBUF_ERR(he_dev->rbrq_head))
		{
			hprintk("HBUF_ERR!  (cid 0x%x)\n", cid);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
				++vcc->stats->rx_drop;
#else
				atomic_inc(&vcc->stats->rx_drop);
#endif
			goto return_host_buffers;
		}

		he_vcc->iov_tail->iov_base = hbuf;
		he_vcc->iov_tail->iov_len = buf_len;
		he_vcc->pdu_len += buf_len;
		++he_vcc->iov_tail;

		if (RBRQ_CON_CLOSED(he_dev->rbrq_head))
		{
			HPRINTK("wake_up rx_waitq  (cid 0x%x)\n", cid);
			wake_up(&he_vcc->rx_waitq);
			goto return_host_buffers;
		}

#ifdef notdef
		if (he_vcc->iov_tail - he_vcc->iov_head > 32)
		{
			hprintk("iovec full!  cid 0x%x\n", cid);
			goto return_host_buffers;
		}
#endif
		if (!RBRQ_END_PDU(he_dev->rbrq_head)) goto next_rbrq_entry;

		if (RBRQ_LEN_ERR(he_dev->rbrq_head)
				|| RBRQ_CRC_ERR(he_dev->rbrq_head))
		{
			HPRINTK("%s%s (%d.%d)\n",
				RBRQ_CRC_ERR(he_dev->rbrq_head)
							? "CRC_ERR " : "",
				RBRQ_LEN_ERR(he_dev->rbrq_head)
							? "LEN_ERR" : "",
							vcc->vpi, vcc->vci);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
			++vcc->stats->rx_err;
#else
			atomic_inc(&vcc->stats->rx_err);
#endif
			goto return_host_buffers;
		}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,15)
		skb = atm_alloc_charge(vcc, he_vcc->pdu_len,
							GFP_ATOMIC);
#else

		if (!atm_charge(vcc, atm_pdu2truesize(he_vcc->pdu_len)))
			skb = NULL;
		else
		{
			skb = alloc_skb(he_vcc->pdu_len, GFP_ATOMIC);
			if (!skb) atm_return(vcc,
				atm_pdu2truesize(he_vcc->pdu_len));
		}
#endif
		if (!skb)
		{
			hprintk("charge failed (%d.%d)\n",
						vcc->vpi, vcc->vci);
			goto return_host_buffers;
		}

		skb->stamp = xtime;

		for(iov = he_vcc->iov_head;
				iov < he_vcc->iov_tail; ++iov)
		{
			struct he_buff *hbuf = iov->iov_base;

			pci_dma_sync_single(he_dev->pci_dev,
				hbuf->mapping, hbuf->len, PCI_DMA_FROMDEVICE);

			memcpy(skb_put(skb, iov->iov_len),
					hbuf->data, iov->iov_len);
		}

		switch(vcc->qos.aal)
		{
			case ATM_AAL0:
				/* 2.10.1.5 raw cell receive */
				skb->len = ATM_AAL0_SDU;
				break;
			case ATM_AAL5:
				/* 2.10.1.2 aal5 receive
				 *
				 * the aal5 trailer could be split
				 * across two buffers
				 */

				skb->len = AAL5_LEN(skb->data, he_vcc->pdu_len);
#ifdef notdef
				skb->ip_summed = CHECKSUM_UNNECESSARY;
#endif
				break;
		}

#ifdef notdef
		if (skb->len > vcc->qos.rxtp.max_sdu)
			hprintk("pdu_len (%d) > vcc->qos.rxtp.max_sdu (%d)!  cid 0x%x\n", skb->len, vcc->qos.rxtp.max_sdu, cid);
#endif

		ATM_SKB(skb)->iovcnt = 0;
		ATM_SKB(skb)->vcc = vcc;
		vcc->push(vcc, skb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
		++vcc->stats->rx;
#else
		atomic_inc(&vcc->stats->rx);
#endif

return_host_buffers:
		++pdus_assembled;

		for(iov = he_vcc->iov_head;
				iov < he_vcc->iov_tail; ++iov)
		{
			struct he_buff *hbuf = iov->iov_base;
			hbuf->loaned = 0;
		}

		he_vcc->iov_tail = he_vcc->iov_head;
		he_vcc->pdu_len = 0;

next_rbrq_entry:
		he_dev->rbrq_head = (struct he_rbrq *)
				((unsigned long) he_dev->rbrq_base |
					RBRQ_MASK(++he_dev->rbrq_head));

	}

	if (updated)
	{
		if (updated > he_dev->rbrq_peak) he_dev->rbrq_peak = updated;

		he_writel(he_dev, RBRQ_MASK(he_dev->rbrq_head),
						G0_RBRQ_H + (group * 16));
	}

	return pdus_assembled;
}

static void
he_service_tbrq(struct he_dev *he_dev, int group)
{
	struct he_tbrq *tbrq_tail = (struct he_tbrq *)
				((unsigned long)he_dev->tbrq_base |
					he_dev->hsp->group[group].tbrq_tail);
	struct he_tpd *tpd;
	int updated = 0;

	/* 2.1.6 transmit buffer return queue */

	while (he_dev->tbrq_head != tbrq_tail)
	{
		++updated;

		HPRINTK("tbrq%d 0x%x%s%s\n",
			group,
			TBRQ_TPD(he_dev->tbrq_head), 
			TBRQ_EOS(he_dev->tbrq_head) ? " EOS" : "",
			TBRQ_MULTIPLE(he_dev->tbrq_head) ? " MULTIPLE" : "");

		tpd = (struct he_tpd *) bus_to_virt(TBRQ_TPD(he_dev->tbrq_head));
		if (TBRQ_EOS(he_dev->tbrq_head))
		{
			HPRINTK("wake_up(tx_waitq) cid 0x%x\n",
				he_mkcid(he_dev, tpd->vcc->vpi, tpd->vcc->vci));
			if (tpd->vcc)
				wake_up(&HE_VCC(tpd->vcc)->tx_waitq);

			goto next_tbrq_entry;
		}

		if (tpd->skb)	/* && !TBRQ_MULTIPLE(he_dev->tbrq_head) */
		{
			pci_unmap_single(he_dev->pci_dev, tpd->address1, tpd->skb->len, PCI_DMA_TODEVICE);
			if (tpd->vcc && tpd->vcc->pop)
				tpd->vcc->pop(tpd->vcc, tpd->skb);
			else
				dev_kfree_skb_any(tpd->skb);
		}


next_tbrq_entry:
		kfree(tpd->base);

		he_dev->tbrq_head = (struct he_tbrq *)
				((unsigned long) he_dev->tbrq_base |
					TBRQ_MASK(++he_dev->tbrq_head));
	}

	if (updated)
	{
		if (updated > he_dev->tbrq_peak) he_dev->tbrq_peak = updated;

		he_writel(he_dev, TBRQ_MASK(he_dev->tbrq_head),
						G0_TBRQ_H + (group * 16));
	}
}


static void
he_service_rbpl(struct he_dev *he_dev, int group)
{
	struct he_rbp *new_tail = he_dev->rbpl_tail;
	struct he_rbp *rbpl_head;
	struct he_buff *hbuf = (struct he_buff *) bus_to_virt(new_tail->virt);
	int moved = 0;

	rbpl_head = bus_to_virt(he_readl(he_dev, G0_RBPL_S));

	while(hbuf->loaned == 0)
	{
		new_tail = (struct he_rbp *)
					((unsigned long)he_dev->rbpl_base |
							RBPL_MASK(++new_tail));

		/* table 3.42 -- rbpl_tail should never be set to rbpl_head */
		if (new_tail == rbpl_head) break;

		hbuf->loaned = 1;
		he_dev->rbpl_tail = new_tail;
		hbuf = (struct he_buff *) bus_to_virt(new_tail->virt);

		++moved;

	}

	if (moved)
	{
		he_writel(he_dev, RBPL_MASK(he_dev->rbpl_tail), G0_RBPL_T);
	}
}

#ifdef rbps_support
static void
he_service_rbps(struct he_dev *he_dev, int group)
{
	struct he_rbp *new_tail = he_dev->rbps_tail;
	struct he_rbp *rbps_head;
	struct he_buff *hbuf = (struct he_buff *) bus_to_virt(new_tail->virt);
	int moved = 0;

	rbps_head = bus_to_virt(he_readl(he_dev, G0_RBPS_S));

	while(hbuf->loaned == 0)
	{
		new_tail = (struct he_rbp *)
					((unsigned long)he_dev->rbps_base |
							RBPS_MASK(++new_tail));

		/* table 3.42 -- rbps_tail should never be set to rbps_head */
		if (new_tail == rbps_head) break;

		hbuf->loaned = 1;
		he_dev->rbps_tail = new_tail;
		hbuf = (struct he_buff *) bus_to_virt(new_tail->virt);

		++moved;

	}

	if (moved)
	{
		he_writel(he_dev, RBPS_MASK(he_dev->rbps_tail), G0_RBPS_T);
	}
}
#endif

static void
he_tasklet(unsigned long data)
{
#ifdef USE_TASKLET
	unsigned long flags;
#endif
	struct he_dev *he_dev = (struct he_dev *) data;
	unsigned group, type;
	int updated = 0;

	HPRINTK("tasklet (0x%lx)\n", data);
#ifdef USE_TASKLET
	he_spin_lock(&he_dev->global_lock, flags);
#endif

	while(he_dev->irq_head != he_dev->irq_tail)
	{
		++updated;

		type = ITYPE_TYPE(he_dev->irq_head->isw);
		group = ITYPE_GROUP(he_dev->irq_head->isw);

		switch (type)
		{
			case ITYPE_RBRQ_THRESH:
				hprintk("rbrq%d threshold\n", group);
			case ITYPE_RBRQ_TIMER:
				if (he_service_rbrq(he_dev, group))
				{
					he_service_rbpl(he_dev, group);
#ifdef rbps_support
					he_service_rbps(he_dev, group);
#endif
				}
				break;
			case ITYPE_TBRQ_THRESH:
				hprintk("tbrq%d threshold\n", group);
			case ITYPE_TPD_COMPLETE:
				he_service_tbrq(he_dev, group);
				break;
			case ITYPE_RBPL_THRESH:
				he_service_rbpl(he_dev, group);
				break;
			case ITYPE_RBPS_THRESH:
#ifdef rbps_support
				he_service_rbps(he_dev, group);
#endif
				break;
			case ITYPE_PHY:
				hprintk1("phy interrupt\n");
				break;
			case ITYPE_OTHER:
				switch (type|group)
				{
					case ITYPE_PARITY:
						hprintk1("parity error\n");
						break;
					case ITYPE_ABORT:
						hprintk("abort 0x%x\n",
						  he_readl(he_dev, ABORT_ADDR));
						break;
				}
				break;
			default:
				if (he_dev->irq_head->isw == ITYPE_INVALID)
				{
					/* see 8.1.1 -- check all queues */

					HPRINTK("isw not updated 0x%x\n",
						he_dev->irq_head->isw);

					he_service_rbrq(he_dev, 0);
					he_service_rbpl(he_dev, 0);
#ifdef rbps_support
					he_service_rbps(he_dev, 0);
#endif
					he_service_tbrq(he_dev, 0);
				}
				else
					hprintk("bad isw = 0x%x?\n",
						he_dev->irq_head->isw);
		}

		he_dev->irq_head->isw = ITYPE_INVALID;

		he_dev->irq_head = (struct he_irq *) NEXT_ENTRY(he_dev->irq_base, he_dev->irq_head, IRQ_MASK);
	}

	if (updated)
	{
		if (updated > he_dev->irq_peak) he_dev->irq_peak = updated;

		he_writel(he_dev,
			IRQ_SIZE(CONFIG_IRQ_SIZE) |
			IRQ_THRESH(CONFIG_IRQ_THRESH) |
			IRQ_TAIL(he_dev->irq_tail), IRQ0_HEAD);
		(void) he_readl(he_dev, INT_FIFO); /* 8.1.2 controller errata */
	}
#ifdef USE_TASKLET
	he_spin_unlock(&he_dev->global_lock, flags);
#endif
}

static void
he_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	unsigned long flags;
	struct he_dev *he_dev = (struct he_dev * )dev_id;

	if (he_dev == NULL) return;

	he_spin_lock(&he_dev->global_lock, flags);

	he_dev->irq_tail = (struct he_irq *) (((unsigned long)he_dev->irq_base) |
						(*he_dev->irq_tailoffset << 2));

	if (he_dev->irq_tail == he_dev->irq_head)
	{
		HPRINTK1("tailoffset not updated?\n");
		he_dev->irq_tail = (struct he_irq *) ((unsigned long)he_dev->irq_base |
			((he_readl(he_dev, IRQ0_BASE) & IRQ_MASK) << 2));
		(void) he_readl(he_dev, INT_FIFO);	/* 8.1.2 controller errata */
	}

#ifdef notdef
	if (he_dev->irq_head == he_dev->irq_tail /* && !IRQ_PENDING */)
		hprintk1("spurious interrupt?\n");
#endif

	if (he_dev->irq_head != he_dev->irq_tail)
	{
#ifdef USE_TASKLET
		tasklet_schedule(&he_dev->tasklet);
#else
		he_tasklet((unsigned long) he_dev);
#endif
		he_writel(he_dev, INT_CLEAR_A, INT_FIFO);
							/* clear interrupt */
	}

	he_spin_unlock(&he_dev->global_lock, flags);
}

static int
he_open(struct atm_vcc *vcc, short vpi, int vci)
{
	struct he_dev *he_dev = HE_DEV(vcc->dev);
	struct he_vcc *he_vcc;
	int err = 0;
	unsigned cid, rsr0, rsr1, rsr4, tsr0, period, reg, clock;
	unsigned long flags;

	if ((err = atm_find_ci(vcc, &vpi, &vci)))
	{
		hprintk("atm_find_ci err = %d\n", err);
		return err;
	}
	if (vci == ATM_VCI_UNSPEC || vpi == ATM_VPI_UNSPEC) return 0;
	vcc->vpi = vpi;
	vcc->vci = vci;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,1)
	vcc->flags |= ATM_VF_ADDR;
#else
	set_bit(ATM_VF_ADDR, &vcc->flags);
#endif

	cid = he_mkcid(he_dev, vpi, vci);

	he_vcc = (struct he_vcc *) kmalloc(sizeof(struct he_vcc), GFP_KERNEL);
	if (he_vcc == NULL)
	{
		hprintk1("unable to allocate he_vcc during open\n");
		return -ENOMEM;
	}

	he_vcc->iov_tail = he_vcc->iov_head;
	he_vcc->pdu_len = 0;
	he_vcc->rc_index = -1;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,1)
	init_waitqueue(&he_vcc->rx_waitq);
	init_waitqueue(&he_vcc->tx_waitq);
#else
	init_waitqueue_head(&he_vcc->rx_waitq);
	init_waitqueue_head(&he_vcc->tx_waitq);
#endif

	HE_VCC(vcc) = he_vcc;

	if (vcc->qos.txtp.traffic_class != ATM_NONE)
	{
		int pcr_goal;

                pcr_goal = atm_pcr_goal(&vcc->qos.txtp);
                if (pcr_goal == 0)
                        pcr_goal = he_dev->atm_dev->link_rate;
                if (pcr_goal < 0)	/* means round down, technically */
                        pcr_goal = -pcr_goal;

		HPRINTK("open tx cid 0x%x pcr_goal %d\n", cid, pcr_goal);

		/* no transmit support for AAL0 -- FIXME */

		if (vcc->qos.aal != ATM_AAL5)
		{
			err = -EINVAL;
			goto open_failed;
		}

		he_spin_lock(&he_dev->global_lock, flags);
		tsr0 = he_readl_tsr0(he_dev, cid);
		he_spin_unlock(&he_dev->global_lock, flags);

		if (TSR0_CONN_STATE(tsr0) != 0)
		{
			hprintk("cid 0x%x not idle (tsr0 = 0x%x)\n", cid, tsr0);
			err = -EBUSY;
			goto open_failed;
		}

		switch(vcc->qos.txtp.traffic_class)
		{
			case ATM_UBR:
				/* 2.3.3.1 open connection ubr */

				tsr0 = TSR0_UBR | TSR0_GROUP(0) | TSR0_AAL5 |
					TSR0_USE_WMIN | TSR0_UPDATE_GER;
				break;

			case ATM_CBR:
				/* 2.3.3.2 open connection cbr */

				clock = he_is622(he_dev) ? 66667000 : 50000000;
				period = clock / pcr_goal;
				
				/* find an unused cs_stper register */

				for(reg = 0; reg < HE_NUM_CS_STPER; ++reg)
					if (he_dev->cs_stper[reg].inuse == 0 || 
						he_dev->cs_stper[reg].pcr == pcr_goal)
					break;

				if (reg == HE_NUM_CS_STPER)
				{
					err = -EBUSY;
					goto open_failed;
				}

				/* 8.2.3 cbr scheduler wrap problem */
				if ((he_dev->total_bw + pcr_goal)
					> (he_dev->atm_dev->link_rate * 10 / 9))
				{
					err = -EBUSY;
					goto open_failed;
				}
				he_dev->total_bw += pcr_goal;

				he_vcc->rc_index = reg;
				++he_dev->cs_stper[reg].inuse;
				he_dev->cs_stper[reg].pcr = pcr_goal;

				HPRINTK("rc_index = %d period = %d\n",
								reg, period);

				he_spin_lock(&he_dev->global_lock, flags);
				he_writel_mbox(he_dev, rate_to_atmf(period/2),
							CS_STPER0 + reg);
				he_spin_unlock(&he_dev->global_lock, flags);

				tsr0 = TSR0_CBR | TSR0_GROUP(0) | TSR0_AAL5 |
							TSR0_RC_INDEX(reg);

				break;
			default:
				err = -EINVAL;
				goto open_failed;
		}

		he_spin_lock(&he_dev->global_lock, flags);

		he_writel_tsr1(he_dev, TSR1_MCR(rate_to_atmf(0)) |
					TSR1_PCR(rate_to_atmf(pcr_goal)), cid);
		he_writel_tsr2(he_dev, TSR2_ACR(rate_to_atmf(pcr_goal)), cid);
		he_writel_tsr3(he_dev, 0x0, cid);
		he_writel_tsr5(he_dev, 0x0, cid);
		he_writel_tsr6(he_dev, 0x0, cid);
		he_writel_tsr7(he_dev, 0x0, cid);
		he_writel_tsr8(he_dev, 0x0, cid);
		he_writel_tsr10(he_dev, 0x0, cid);
		he_writel_tsr11(he_dev, 0x0, cid);
		he_writel_tsr12(he_dev, 0x0, cid);
		he_writel_tsr13(he_dev, 0x0, cid);
		he_writel_tsr14(he_dev, 0x0, cid);
		he_writel_tsr4(he_dev, TSR4_AAL5 | 1, cid);
		he_writel_tsr9(he_dev, TSR9_OPEN_CONN, cid);
		he_writel_tsr0(he_dev, tsr0, cid);

		he_spin_unlock(&he_dev->global_lock, flags);
	}

	if (vcc->qos.rxtp.traffic_class != ATM_NONE)
	{
		unsigned aal;

		HPRINTK("open rx cid 0x%x (rx_waitq %p)\n", cid,
		 				&HE_VCC(vcc)->rx_waitq);

		switch (vcc->qos.aal)
		{
			case ATM_AAL5:
				aal = RSR0_AAL5;
				break;
			case ATM_AAL0:
				aal = RSR0_RAWCELL;
				break;
			default:
				err = -EINVAL;
				goto open_failed;
		}

		he_spin_lock(&he_dev->global_lock, flags);

		rsr0 = he_readl_rsr0(he_dev, cid);
		if (rsr0 & RSR0_OPEN_CONN)
		{
			he_spin_unlock(&he_dev->global_lock, flags);

			hprintk("cid 0x%x not idle (rsr0 = 0x%x)\n", cid, rsr0);
			err = -EBUSY;
			goto open_failed;
		}

#ifdef notdef
		rsr1 = RSR1_GROUP(0);
		rsr4 = RSR4_GROUP(0);
#else
		rsr1 = RSR1_GROUP(0)|RSR1_RBPL_ONLY;
		rsr4 = RSR4_GROUP(0)|RSR4_RBPL_ONLY;
#endif
		rsr0 = vcc->qos.rxtp.traffic_class == ATM_UBR ? 
				(RSR0_EPD_ENABLE|RSR0_PPD_ENABLE) : 0;

		he_writel_rsr1(he_dev, rsr1, cid);
		he_writel_rsr4(he_dev, rsr4, cid);
		he_writel_rsr0(he_dev,
			rsr0 | RSR0_START_PDU | RSR0_OPEN_CONN | aal, cid);

		he_spin_unlock(&he_dev->global_lock, flags);

		HE_LOOKUP_VCC(he_dev, cid) = vcc;
	}

open_failed:

	if (err)
	{
		HE_LOOKUP_VCC(he_dev, cid) = NULL;
		if (he_vcc) kfree(he_vcc);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,1)
		vcc->flags &= ~ATM_VF_ADDR;
#else
		clear_bit(ATM_VF_ADDR, &vcc->flags);
#endif
	}
	else
	{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,1)
		vcc->flags |= ATM_VF_READY;
#else
		set_bit(ATM_VF_READY, &vcc->flags);
#endif
	}

	return err;
}

static __inline__ void
he_enqueue_tpd(struct he_dev *he_dev, struct he_tpd *tpd, unsigned cid)
{
	unsigned long flags;
	struct he_tpdrq *new_tail;

	HPRINTK("tpdrq %p cid 0x%x -> tpdrq_tail %p\n",
					tpd, cid, he_dev->tpdrq_tail);

	spin_lock_irqsave(&he_dev->tpdrq_lock, flags);

	/* 2.1.5 transmit packet descriptor ready queue */

	he_dev->tpdrq_tail->tpd = virt_to_bus(tpd);
	he_dev->tpdrq_tail->cid = cid;
	wmb();

	new_tail = (struct he_tpdrq *) ((unsigned long) he_dev->tpdrq_base |
					RBRQ_MASK(++he_dev->tpdrq_tail));

	/*
	 * check to see if we are about to set the tail == head
	 * if true, update the head pointer from the adapter
	 * to see if this is really the case (reading the queue
	 * head for every transmit would be unnecessarily slow)
	 */

	if (new_tail == he_dev->tpdrq_head)
	{
		he_dev->tpdrq_head = (struct he_tpdrq *)
			(((unsigned long)he_dev->tpdrq_base) |
				TPDRQ_MASK(he_readl(he_dev, TPDRQ_B_H)));

		if (new_tail == he_dev->tpdrq_head)
		{
			hprintk("tpdrq full (cid 0x%x)\n", cid);
			/*
			 * FIXME
			 * push tpd onto a transmit backlog queue
			 * after service_tbrq, service the backlog
			 * for now, we just drop the pdu
			 */
			if (tpd->vcc->pop)
				tpd->vcc->pop(tpd->vcc, tpd->skb);
			else
				dev_kfree_skb_any(tpd->skb);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
			++tpd->vcc->stats->tx_err;
#else
			atomic_inc(&tpd->vcc->stats->tx_err);
#endif
			kfree(tpd->base);
			spin_unlock_irqrestore(&he_dev->tpdrq_lock, flags);
			return;
		}
	}
	he_dev->tpdrq_tail = new_tail;

	he_writel(he_dev, TPDRQ_MASK(he_dev->tpdrq_tail), TPDRQ_T);
	spin_unlock_irqrestore(&he_dev->tpdrq_lock, flags);
}


static void
he_close(struct atm_vcc *vcc)
{
	unsigned long flags;
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,1)
	DECLARE_WAITQUEUE(wait, current);
#else
	struct wait_queue wait = { current, NULL };
#endif
	struct he_dev *he_dev = HE_DEV(vcc->dev);
	struct he_tpd *tpd;
	unsigned cid, retry;
	struct he_vcc *he_vcc = HE_VCC(vcc);
	int tx_inuse;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,1)
	vcc->flags &= ~ATM_VF_READY;
#else
	clear_bit(ATM_VF_READY, &vcc->flags);
#endif
	cid = he_mkcid(he_dev, vcc->vpi, vcc->vci);

	he_spin_lock(&he_dev->global_lock, flags);

	if (vcc->qos.txtp.traffic_class != ATM_NONE)
	{
		volatile unsigned tsr4, tsr0;
		int timeout;

		HPRINTK("close tx cid 0x%x\n", cid);

		/* 2.3.1.1 generic close operations with flush */

		he_writel_tsr4_upper(he_dev, TSR4_FLUSH_CONN, cid);
					/* also clears TSR4_SESSION_ENDED */

		switch(vcc->qos.txtp.traffic_class)
		{
			case ATM_UBR:
				he_writel_tsr1(he_dev, 
					TSR1_MCR(rate_to_atmf(200000))
					| TSR1_PCR(rate_to_atmf(he_dev->atm_dev->link_rate)),
									cid);
				break;
			case ATM_CBR:
				he_writel_tsr14_upper(he_dev, TSR14_DELETE, cid);
				break;
		}

		/* wait for last outstanding tpd to be returned? */

		retry = 0;
		while ((tx_inuse = atomic_read(&vcc->tx_inuse)) > 0)
		{
			++retry;
			if ((retry % 10) == 0)
				hprintk("close tx cid 0x%x tx_inuse = %d)\n", cid, tx_inuse);
			HPRINTK("vcc->tx_inuse = %d\n", tx_inuse);

			if (retry == 1)
			{
				he_spin_unlock(&he_dev->global_lock, flags);
				udelay(100);
				he_spin_lock(&he_dev->global_lock, flags);
			}
			else
			{
				set_current_state(TASK_UNINTERRUPTIBLE);
				he_spin_unlock(&he_dev->global_lock, flags);
				(void) schedule_timeout(5);
				he_spin_lock(&he_dev->global_lock, flags);
				set_current_state(TASK_RUNNING);
			}
		}

		tpd = he_alloc_tpd();
		if (tpd == NULL)
		{
			hprintk("close tx he_alloc_tpd failed cid 0x%x\n", cid);
			goto close_tx_incomplete;
		}
		tpd->status |= TPD_EOS | TPD_INT;
		wmb();
		tpd->vcc = vcc;

		add_wait_queue(&he_vcc->tx_waitq, &wait);
		set_current_state(TASK_UNINTERRUPTIBLE);
		he_enqueue_tpd(he_dev, tpd, cid);
		he_spin_unlock(&he_dev->global_lock, flags);
		timeout = schedule_timeout(5*HZ);
		he_spin_lock(&he_dev->global_lock, flags);
		remove_wait_queue(&he_vcc->tx_waitq, &wait);
		set_current_state(TASK_RUNNING);

		if (timeout == 0)
		{
			hprintk("close tx timeout cid 0x%x\n", cid);
			goto close_tx_incomplete;
		}

		while (!((tsr4 = he_readl_tsr4(he_dev, cid))
							& TSR4_SESSION_ENDED))
		{
			HPRINTK("close tx cid 0x%x !TSR4_SESSION_ENDED (tsr4 = 0x%x)\n", cid, tsr4);
			udelay(100);
		}

		while (TSR0_CONN_STATE(tsr0 = he_readl_tsr0(he_dev, cid)) != 0)
		{
			HPRINTK("close tx cid 0x%x TSR0_CONN_STATE != 0 (tsr0 = 0x%x)\n", cid, tsr0);
			udelay(100);
		}

close_tx_incomplete:

		if (vcc->qos.txtp.traffic_class == ATM_CBR)
		{
			int reg = he_vcc->rc_index;

			HPRINTK("cs_stper reg = %d\n", reg);

			if (he_dev->cs_stper[reg].inuse == 0)
				hprintk("cs_stper[%d].inuse = 0!\n", reg);
			else
				--he_dev->cs_stper[reg].inuse;

			he_dev->total_bw -= he_dev->cs_stper[reg].pcr;
		}

		HPRINTK("close tx cid 0x%x complete\n", cid);
	}

	if (vcc->qos.rxtp.traffic_class != ATM_NONE)
	{
		int timeout;

		HPRINTK("close rx cid 0x%x\n", cid);

		/* 2.7.2.2 close receive operation */

		/* wait for previous close (if any) to finish */

		while(he_readl(he_dev, RCC_STAT) & RCC_BUSY)
		{
			HPRINTK("close cid 0x%x RCC_BUSY\n", cid);
			udelay(100);
		}

		add_wait_queue(&he_vcc->rx_waitq, &wait);
		set_current_state(TASK_UNINTERRUPTIBLE);

		he_writel_rsr0(he_dev, RSR0_CLOSE_CONN, cid);
		he_writel_mbox(he_dev, cid, RXCON_CLOSE);

		he_spin_unlock(&he_dev->global_lock, flags);
		timeout = schedule_timeout(5*HZ);
		he_spin_lock(&he_dev->global_lock, flags);
		remove_wait_queue(&he_vcc->rx_waitq, &wait);
		set_current_state(TASK_RUNNING);

		if (timeout == 0)
			hprintk("rx close timeout (cid 0x%x)\n", cid);

		HE_LOOKUP_VCC(he_dev, cid) = NULL;
		kfree(he_vcc);

		HPRINTK("close rx cid 0x%x complete\n", cid);
	}

	he_spin_unlock(&he_dev->global_lock, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,1)
	vcc->flags &= ~ATM_VF_ADDR;
#else
	clear_bit(ATM_VF_ADDR, &vcc->flags);
#endif
}

static int
he_sg_send(struct atm_vcc *vcc, unsigned long start, unsigned long size)
{
	return 0;
}

static int
he_send(struct atm_vcc *vcc, struct sk_buff *skb)
{
	unsigned long flags;
	struct he_dev *he_dev = HE_DEV(vcc->dev);
	unsigned cid;
	struct he_tpd *tpd;

#define HE_TPD_BUFSIZE 0xffff

	HPRINTK("send %d.%d\n", vcc->vpi, vcc->vci);

	if (skb->len > HE_TPD_BUFSIZE )
	{
		hprintk("buffer too large (%d bytes)\n", skb->len );
		if (vcc->pop)
			vcc->pop(vcc, skb);
		else
			dev_kfree_skb_any(skb);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
		++vcc->stats->tx_err;
#else
		atomic_inc(&vcc->stats->tx_err);
#endif
		return -EINVAL;
	}

	if (ATM_SKB(skb)->iovcnt != 0)
	{
		/* we should be able to support iovcnt <= 3 easily but... */
		hprintk1("scatter/gather not supported.");
		if (vcc->pop)
			vcc->pop(vcc, skb);
		else
			dev_kfree_skb_any(skb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
		++vcc->stats->tx_err;
#else
		atomic_inc(&vcc->stats->tx_err);
#endif
		return -EINVAL;
	}

	tpd = he_alloc_tpd();
	if (tpd == NULL)
	{
		if (vcc->pop)
			vcc->pop(vcc, skb);
		else
			dev_kfree_skb_any(skb);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
		++vcc->stats->tx_err;
#else
		atomic_inc(&vcc->stats->tx_err);
#endif
		return -ENOMEM;
	}

	tpd->status |= TPD_USERCELL | TPD_INT;
	tpd->address1 = pci_map_single(he_dev->pci_dev, skb->data, skb->len, PCI_DMA_TODEVICE);
	tpd->length1 = skb->len | TPD_LST;
	wmb();
	tpd->vcc = vcc;
	tpd->skb = skb;
	ATM_SKB(skb)->vcc = vcc;

	cid = he_mkcid(he_dev, vcc->vpi, vcc->vci);

	he_spin_lock(&he_dev->global_lock, flags);
	he_enqueue_tpd(he_dev, tpd, cid);
	he_spin_unlock(&he_dev->global_lock, flags);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,99)
	++vcc->stats->tx;
#else
	atomic_inc(&vcc->stats->tx);
#endif

	return 0;
}

static int
he_ioctl(struct atm_dev *atm_dev, unsigned int cmd, void *arg)
{
	struct he_dev *he_dev = HE_DEV(atm_dev);
	struct he_ioctl_reg reg;

	switch (cmd)
	{
		case HE_GET_REG:
			if (!capable(CAP_NET_ADMIN)) return -EPERM;

			copy_from_user(&reg, (struct he_ioctl_reg *) arg,
						sizeof(struct he_ioctl_reg));
			switch (reg.type)
			{
				case HE_REGTYPE_PCI:
					if (reg.addr < RESET_CNTL || reg.addr > 0x80c00)
						return -EINVAL;
					reg.val = he_readl(he_dev, reg.addr);
					break;
				case HE_REGTYPE_RCM:
					reg.val =
						he_readl_rcm(he_dev, reg.addr);
					break;
				case HE_REGTYPE_TCM:
					reg.val =
						he_readl_tcm(he_dev, reg.addr);
					break;
				case HE_REGTYPE_MBOX:
					reg.val =
						he_readl_mbox(he_dev, reg.addr);
					break;
				default:
					return -EINVAL;
			}
			copy_to_user((struct he_ioctl_reg *) arg, &reg,
						sizeof(struct he_ioctl_reg));
			break;
		default:
#ifdef USE_SUNI
			if (atm_dev->phy && atm_dev->phy->ioctl)
				return atm_dev->phy->ioctl(atm_dev, cmd, arg);
			else
#endif /* USE_SUNI */
				return -EINVAL;
	}

	return 0;
}

static void
he_phy_put(struct atm_dev *atm_dev, unsigned char val, unsigned long addr)
{
	struct he_dev *he_dev = HE_DEV(atm_dev);
	unsigned long flags;

	he_spin_lock(&he_dev->global_lock, flags);
        he_writel(he_dev, val, FRAMER + addr);
	he_spin_unlock(&he_dev->global_lock, flags);
}
 
        
static unsigned char
he_phy_get(struct atm_dev *atm_dev, unsigned long addr)
{ 
	struct he_dev *he_dev = HE_DEV(atm_dev);
	unsigned long reg, flags;

	he_spin_lock(&he_dev->global_lock, flags);
        reg = he_readl(he_dev, FRAMER + addr);
	he_spin_unlock(&he_dev->global_lock, flags);
	return reg;
}

static int
he_proc_read(struct atm_dev *dev, loff_t *pos, char *page)
{
	unsigned long flags;
	struct he_dev *he_dev = HE_DEV(dev);
	int left, i;
#ifdef notdef
	struct he_rbrq *rbrq_tail;
	struct he_tpdrq *tpdrq_head;
        int rbpl_head, rbpl_tail;
#endif
	static long mcc = 0, oec = 0, dcc = 0, cec = 0;


	left = *pos;
	if (!left--)
		return sprintf(page, "%s%s\n\n",
			he_dev->prod_id, he_dev->media & 0x40 ? "SM" : "MM");

	if (!left--)
		return sprintf(page, "Mismatched Cells  VPI/VCI Not Open  Dropped Cells  RCM Dropped Cells\n");

	he_spin_lock(&he_dev->global_lock, flags);
	mcc += he_readl(he_dev, MCC);
	oec += he_readl(he_dev, OEC);
	dcc += he_readl(he_dev, DCC);
	cec += he_readl(he_dev, CEC);
	he_spin_unlock(&he_dev->global_lock, flags);

	if (!left--)
		return sprintf(page, "%16ld  %16ld  %13ld  %17ld\n\n", 
							mcc, oec, dcc, cec);

	if (!left--)
		return sprintf(page, "irq_size = %d  inuse = ?  peak = %d\n",
				CONFIG_IRQ_SIZE, he_dev->irq_peak);

	if (!left--)
		return sprintf(page, "tpdrq_size = %d  inuse = ?\n",
						CONFIG_TPDRQ_SIZE);

	if (!left--)
		return sprintf(page, "rbrq_size = %d  inuse = ?  peak = %d\n",
				CONFIG_RBRQ_SIZE, he_dev->rbrq_peak);

	if (!left--)
		return sprintf(page, "tbrq_size = %d  peak = %d\n",
					CONFIG_TBRQ_SIZE, he_dev->tbrq_peak);


#ifdef notdef
        rbpl_head = RBPL_MASK(he_readl(he_dev, G0_RBPL_S));
        rbpl_tail = RBPL_MASK(he_readl(he_dev, G0_RBPL_T));

	inuse = rbpl_head - rbpl_tail;
	if (inuse < 0) inuse += CONFIG_RBPL_SIZE * sizeof(struct he_rbp);
	inuse /= sizeof(struct he_rbp);

	if (!left--)
		return sprintf(page, "rbpl_size = %d  inuse = %d\n\n",
						CONFIG_RBPL_SIZE, inuse);
#endif

	if (!left--)
		return sprintf(page, "rate controller periods (cbr)\n                 pcr  #vc\n");

	for (i = 0; i < HE_NUM_CS_STPER; ++i)
		if (!left--)
			return sprintf(page, "cs_stper%-2d  %8ld  %3d\n", i,
						he_dev->cs_stper[i].pcr,
						he_dev->cs_stper[i].inuse);

	if (!left--)
		return sprintf(page, "total bw (cbr): %d  (limit %d)\n",
			he_dev->total_bw, he_dev->atm_dev->link_rate * 10 / 9);

	return 0;
}

/* eeprom routines  -- see 4.7 */

u8
read_prom_byte(struct he_dev *he_dev, int addr)
{
	u32 val = 0, tmp_read = 0;
	int i, j = 0;
	u8 byte_read = 0;

	val = readl(he_dev->membase + HOST_CNTL);
	val &= 0xFFFFE0FF;
       
	/* Turn on write enable */
	val |= 0x800;
	writel(val, he_dev->membase + HOST_CNTL);
       
	/* Send READ instruction */
	for (i=0; i<sizeof(readtab)/sizeof(readtab[0]); i++) {
		writel(val | readtab[i], he_dev->membase + HOST_CNTL);
		udelay(EEPROM_DELAY);
	}
       
        /* Next, we need to send the byte address to read from */
	for (i=7; i>=0; i--) {
		writel(val | clocktab[j++] | (((addr >> i) & 1) << 9),
			he_dev->membase + HOST_CNTL);
		udelay(EEPROM_DELAY);
		writel(val | clocktab[j++] | (((addr >> i) & 1) << 9),
			he_dev->membase + HOST_CNTL);
		udelay(EEPROM_DELAY);
	}
       
	j=0;

	val &= 0xFFFFF7FF;      /* Turn off write enable */
	writel(val, he_dev->membase + HOST_CNTL);
       
	/* Now, we can read data from the EEPROM by clocking it in */
	for (i=7; i>=0; i--) {
		writel(val | clocktab[j++], he_dev->membase + HOST_CNTL);
	        udelay(EEPROM_DELAY);
	        tmp_read = readl(he_dev->membase + HOST_CNTL);
	        byte_read |= (unsigned char)
	                        ((tmp_read & ID_DOUT)
	                         >> ID_DOFFSET << i);
		writel(val | clocktab[j++],
		he_dev->membase + HOST_CNTL);
		udelay(EEPROM_DELAY);
	}
       
	writel(val | clocktab[j++], he_dev->membase + HOST_CNTL);
	udelay(EEPROM_DELAY);

        return (byte_read);
}

MODULE_AUTHOR("chas williams <chas@cmf.nrl.navy.mil>");
MODULE_DESCRIPTION("ForeRunnerHE ATM Adapter driver");
MODULE_PARM(disable64, "h");
MODULE_PARM_DESC(disable64, "disable 64-bit pci bus transfers");

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,3,1)
static struct pci_device_id he_pci_tbl[] __devinitdata = {
	{ PCI_VENDOR_ID_FORE, PCI_DEVICE_ID_FORE_HE, PCI_ANY_ID, PCI_ANY_ID,
	  0, 0, 0 },
        { 0, }
};

static struct pci_driver he_driver = {
	name:		"he",
	probe:		he_init_one,
	remove:		he_remove_one,
	id_table:	he_pci_tbl,
};

static int __init he_init(void)
{
        return pci_module_init(&he_driver);
}

static void __exit he_cleanup(void)
{
        pci_unregister_driver(&he_driver);
}

module_init(he_init);
module_exit(he_cleanup);
#else
static int __init
he_init()
{
	if (!pci_present())
		return -EIO;

	pci_dev = NULL;
	while ((pci_dev = pci_find_device(PCI_VENDOR_ID_FORE,
					PCI_DEVICE_ID_FORE_HE, pci_dev)) != NULL)
		if (he_init_one(pci_dev, NULL) == 0)
			++ndevs;

	return (ndevs ? 0 : -ENODEV);
}

static void __devexit
he_cleanup(void)
{
	while (he_devs)
	{
		atm_dev_deregister(he_devs->atm_dev);
		he_stop(he_devs);
		kfree(he_devs);

		he_devs = he_devs->next;
	}

}

int init_module(void)
{
	return he_init();
}

void cleanup_module(void)
{
	he_cleanup();
}
#endif
