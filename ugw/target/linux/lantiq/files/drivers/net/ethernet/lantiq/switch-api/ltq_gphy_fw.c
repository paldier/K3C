/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/pm.h>
#include <linux/export.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/dma-mapping.h>
#include <linux/firmware.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <asm/reboot.h>
#include <linux/kthread.h>
#include <lantiq_soc.h>
//#include <ifx_ethsw_kernel_api.h>
#include <switch-api/lantiq_ethsw_api.h>
static struct proc_dir_entry *g_proc_gphy_dir;

static int dev_id; 
static struct device_node *gphy_node;
static struct device *gphy_dev;
const char *g_gphy_fw_mode = NULL;
static int g_no_phys, pw_save_mode;
static dma_addr_t g_dev_addr = 0;
static dma_addr_t f_dev_addr = 0;
static void *g_fw_addr;
static void *f_fw_addr;
#define XWAY_GPHY_FW_ALIGN	(16 * 1024)
#define XWAY_GPHY_FW_ALIGN_MASK (XWAY_GPHY_FW_ALIGN - 1)
static u8 *g_gphy_fw_dma = NULL;
static u8 *f_gphy_fw_dma = NULL;
#define	IH_MAGIC_NO	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/
#define VERSIONLEN	16

/** Image header format*/
typedef struct gphy_image_header {
	uint32_t	immagic;	/*  Magic Number	*/
	uint32_t	imhcrc;	/* Checksum	*/
	uint32_t	imtime;	/* Timestamp	*/
	uint32_t	imsize;	/* Data Size		*/
	uint32_t	imload;	/* load  address		*/
	uint32_t	imep;		/* Entry Pointer */
	uint32_t	imdcrc;	/* Data Checksum	*/
	uint8_t		imos;		/* OS*/
	uint8_t		imarch;	/* architecture	of cpu	*/
	uint8_t		imtype;	/* type of Image			*/
	uint8_t		imcomp;	/* type of	compression 	*/
	uint8_t		imname[IH_NMLEN];	/* Name of Image */
#ifdef CONFIG_LANTIQ_IMAGE_EXTRA_CHECKS
	uint8_t     ihvendor[IH_NMLEN]; /* 32 char Vendor Name String */
	uint8_t     ihboard[IH_NMLEN]; /* 32 char Board Name String */
	uint8_t     ihboardVer[VERSIONLEN]; /* Board Version 16 char str */
	uint8_t     ihchip[IH_NMLEN]; /* 32 char Chip Name String */
	uint8_t     ihchipVer[VERSIONLEN]; /* Chip Version16 char string */
	uint8_t     ihswVer[VERSIONLEN]; /* Software Version-16 char string*/
#endif /* CONFIG_LANTIQ_IMAGE_EXTRA_CHECKS */
} gphy_image_header_t;
LTQ_ETHSW_API_HANDLE swithc_api_fd;
struct task_struct *gphy_pw_id;
static int gphy_pw_save_thread (void *arg);
/* Signal Related */
wait_queue_head_t gphy_pw_wait;
unsigned int phy_port_nos[6];
unsigned int phy_fw_type[6];
static unsigned int on_interval = 4;
static unsigned int off_interval = 5;
#define NUM_OF_PORTS 6
static int gphy_link_status[NUM_OF_PORTS] = {0};
static int gphy_power_down[NUM_OF_PORTS] = {0};
static unsigned int power_down_cnt[NUM_OF_PORTS] = {0};
static int ssd_err_mode = 0;
static unsigned int ssd_interval = 4;
/*#define PHY_SSD_ERROR	0 */
#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
struct task_struct *gphy_ssd_id;
wait_queue_head_t gphy_ssd_wait;
static int gphy_ssd_err_thread (void *arg);
static unsigned int ssd_cnt[NUM_OF_PORTS] = {0};
#endif /* PHY_SSD_ERROR */

/* reset request register */
#define RCU_RST_REQ		0x0010

/* reboot bit */
#define RCU_RD_GPHY0_XRX200	BIT(31)
#define RCU_RD_SRST		BIT(30)
#define RCU_RD_GPHY1_XRX200	BIT(29)
/* vr9 gphy registers */
#define RCU_GFS_ADD0_XRX200	0x0020
#define RCU_GFS_ADD1_XRX200	0x0068

/* xRX300 gphy registers */
#define RCU_GFS_ADD0_XRX300	0x0020
#define RCU_GFS_ADD1_XRX300	0x0058
#define RCU_GFS_ADD2_XRX300	0x00AC

/* xRX330 gphy registers */
#define RCU_GFS_ADD0_XRX330	0x0020
#define RCU_GFS_ADD1_XRX330	0x0058
#define RCU_GFS_ADD2_XRX330	0x00AC
#define RCU_GFS_ADD3_XRX330	0x0264

/* xRX300 bits */
#define RCU_RD_GPHY0_XRX300	BIT(31)
#define RCU_RD_GPHY1_XRX300	BIT(29)
#define RCU_RD_GPHY2_XRX300	BIT(28)

/* xRX330 bits */
#define RCU_RD_GPHY0_XRX330	BIT(31)
#define RCU_RD_GPHY1_XRX330	BIT(29)
#define RCU_RD_GPHY2_XRX330	BIT(28)
#define RCU_RD_GPHY3_XRX330	BIT(10)

/* xRX300 gphy registers */
#define RCU_GFS_ADD0_XRX300	0x0020
#define RCU_GFS_ADD1_XRX300	0x0058
#define RCU_GFS_ADD2_XRX300	0x00AC

/* xRX330 gphy registers */
#define RCU_GFS_ADD0_XRX330	0x0020
#define RCU_GFS_ADD1_XRX330	0x0058
#define RCU_GFS_ADD2_XRX330	0x00AC
#define RCU_GFS_ADD3_XRX330	0x0264

/* reset / boot a gphy */
static struct ltq_xrx200_gphy_reset {
	u32 rd;
	u32 addr;
} xrx200_gphy[] = {
	{RCU_RD_GPHY0_XRX200, RCU_GFS_ADD0_XRX200},
	{RCU_RD_GPHY1_XRX200, RCU_GFS_ADD1_XRX200},
};

/* reset / boot a gphy */
static struct ltq_xrx300_gphy_reset {
	u32 rd;
	u32 addr;
} xrx300_gphy[] = {
	{RCU_RD_GPHY0_XRX300, RCU_GFS_ADD0_XRX300},
	{RCU_RD_GPHY1_XRX300, RCU_GFS_ADD1_XRX300},
	{RCU_RD_GPHY2_XRX300, RCU_GFS_ADD2_XRX300},
};

/* reset / boot a gphy */
static struct ltq_xrx330_gphy_reset {
	u32 rd;
	u32 addr;
} xrx330_gphy[] = {
	{RCU_RD_GPHY0_XRX330, RCU_GFS_ADD0_XRX330},
	{RCU_RD_GPHY1_XRX330, RCU_GFS_ADD1_XRX330},
	{RCU_RD_GPHY2_XRX330, RCU_GFS_ADD2_XRX330},
	{RCU_RD_GPHY3_XRX330, RCU_GFS_ADD3_XRX330},
};

#define LTQ_DRV_MODULE_NAME			"ltq_xrx_gphy"
#define LTQ_PHY_DRV_VERSION      "2.0.1"
static char version[] = LTQ_DRV_MODULE_NAME ": V" LTQ_PHY_DRV_VERSION "";

/* Proc File  */
static int proc_file_create(void);
static void proc_file_delete(void);
static int proc_read_ver(struct seq_file *, void *);
static int proc_read_phy_fw(struct seq_file *, void *);
static ssize_t proc_write_phy_fw(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_ver_open(struct inode *, struct file *);
static int proc_read_phy_fw_open(struct inode *, struct file *);

static int proc_read_phy_off_open(struct inode *, struct file *);
static int proc_read_phy_on_open(struct inode *, struct file *);
static int proc_read_phy_on(struct seq_file *, void *);
static int proc_read_phy_off(struct seq_file *, void *);
static ssize_t proc_write_phy_on(struct file *, const char __user *, size_t, loff_t *);
static ssize_t proc_write_phy_off(struct file *, const char __user *, size_t, loff_t *);
static void lq_ethsw_mdio_data_write(unsigned int phyAddr, unsigned int regAddr,unsigned int data);


//For VRX220 SW control PHY LED
#include <linux/sched.h>
#include <linux/of_gpio.h>
struct task_struct *gphy_rmon_poll_thread_id;
#define LED_OFF	0
#define LED_ON		1
#define LED_FLASH	2
#define GPIO_VRX200_OFFSET  200
enum gphy_gpio_mapping {
	GPHY_2_GPIO = 33 + GPIO_VRX200_OFFSET,
	GPHY_3_GPIO = 11 + GPIO_VRX200_OFFSET,
	GPHY_4_GPIO = 12 + GPIO_VRX200_OFFSET,
	GPHY_5_GPIO = 15 + GPIO_VRX200_OFFSET,
};
int gphy_led_state[NUM_OF_PORTS] = {0,0,0,0,0,0};	/* 0: OFF, 1: ON, 2: flash */
int gphy_led_status_on[NUM_OF_PORTS] = {0,0,0,0,0,0};
//end For VRX220 SW control PHY LED

static unsigned short lq_ethsw_mdio_data_read(unsigned int phyAddr, unsigned int regAddr);
static unsigned short gsw_mmd_data_read(unsigned int phyAddr, unsigned int regAddr);
static dma_addr_t xway_gphy_load(unsigned char *pData);
static int ltq_gphy_firmware_config (void);
static int ltq_mix_firmware_config (u8 *fw_ptr, int);
static struct file_operations file_phy_fw_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_phy_fw_open,
    .read       = seq_read,
    .write      = proc_write_phy_fw,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_phy_fw_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_phy_fw, NULL);
}

unsigned int found_magic = 0, found_img = 0, first_block =0, fw_len=0, rcv_size = 0, second_block = 0;
int fmode = 0, update_ge = 0, update_fe = 0;
static ssize_t proc_write_phy_fw(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	gphy_image_header_t header;
	int len = 0;
	char local_buf[4096] = {0};
	memset(&header, 0, sizeof(gphy_image_header_t));
	len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
	len = len - copy_from_user((local_buf), buf, len);

	if (!found_img) {
		memcpy(&header, (local_buf + (found_magic * sizeof(gphy_image_header_t))), sizeof(gphy_image_header_t));
		if (header.immagic == IH_MAGIC_NO) {
			found_magic++;
			first_block = 0;
			second_block = 0;
			fmode = 0;
			update_ge = 0;
			update_fe = 0;
		}
	}
	if (!found_img) {
		if (strcmp(g_gphy_fw_mode, "11G-FW") == 0) {
/*			dev_info(gphy_dev, "%s: header:%s, size:%d  \n", __func__,header.imname, header.imsize);*/
			if (((strncmp(header.imname, "VR9 V1.1 GPHY GE", sizeof(header.imname)) == 0)	\
				|| (strncmp(header.imname, "AR10 V1.1 GPHY GE", sizeof(header.imname)) == 0))	\
				&& (dev_id == 0) ) {
				first_block = 1;
				fw_len = header.imsize;
				found_img = 1;
				fmode = 1;
				dev_info(gphy_dev, "%s:   Found:%s  FW \n", __func__,header.imname);
			} else if (((strncmp(header.imname, "VR9 V1.2 GPHY GE", sizeof(header.imname)) == 0)	\
				|| (strncmp(header.imname, "AR10 V1.2 GPHY GE", sizeof(header.imname)) == 0))	\
				&& (dev_id == 1))	{
				first_block = 1;
				fw_len = header.imsize;
				found_img = 1;
				fmode = 1;
				dev_info(gphy_dev, "%s:   Found:%s  FW \n", __func__,header.imname);
			}
		} else if (strcmp(g_gphy_fw_mode, "22F-FW") == 0) {
			if(((strncmp(header.imname, "VR9 V1.1 GPHY FE", sizeof(header.imname)) == 0)	\
				 || (strncmp(header.imname, "AR10 V1.1 GPHY FE", sizeof(header.imname)) == 0))	\
				 && (dev_id == 0)) {
				first_block = 1;
				fw_len = header.imsize;
				found_img = 1;
				fmode = 2;
				dev_info(gphy_dev, "%s:   Found:%s  FW \n", __func__,header.imname);
			} else if (((strncmp(header.imname, "VR9 V1.2 GPHY FE", sizeof(header.imname)) == 0)	\
				|| (strncmp(header.imname, "AR10 V1.2 GPHY FE", sizeof(header.imname)) == 0 ))	\
				&& (dev_id == 1)) {
				first_block = 1;
				fw_len = header.imsize;
				found_img = 1;
				fmode = 2;
				dev_info(gphy_dev, "%s:  Found:%s  FW \n", __func__,header.imname);
			}
		} else if (strcmp(g_gphy_fw_mode, "11G22F-FW") == 0) {
			dev_info(gphy_dev, "%s: header:%s, size:%d  \n", __func__,header.imname, header.imsize);
			if (((strncmp(header.imname, "VR9 V1.2 GPHY GE", sizeof(header.imname)) == 0)	\
				|| (strncmp(header.imname, "AR10 V1.2 GPHY GE", sizeof(header.imname)) == 0))	\
				&& (dev_id == 1))	{
				first_block = 1;
				fw_len = header.imsize;
				found_img = 1;
				fmode = 1;
				dev_info(gphy_dev, "%s:   Found:%s  FW \n", __func__,header.imname);
			}
			if (((strncmp(header.imname, "VR9 V1.2 GPHY FE", sizeof(header.imname)) == 0)	\
				|| (strncmp(header.imname, "AR10 V1.2 GPHY FE", sizeof(header.imname)) == 0 ))	\
				&& (dev_id == 1)) {
				first_block = 1;
				fw_len = header.imsize;
				found_img = 1;
				fmode = 2;
				dev_info(gphy_dev, "%s:  Found:%s  FW \n", __func__,header.imname);
			}
		}
	}
	if ((strcmp(g_gphy_fw_mode, "11G-FW") == 0) || (strcmp(g_gphy_fw_mode, "22F-FW") == 0)) {
		if ((first_block == 1) && (!second_block) && found_img) {
			g_gphy_fw_dma = (u8*) kmalloc (fw_len * sizeof (unsigned char), GFP_KERNEL);
			memset(g_gphy_fw_dma, 0, fw_len);
			rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
			memcpy(g_gphy_fw_dma, local_buf+(found_magic * sizeof(gphy_image_header_t)), rcv_size);
			second_block = 1;
		} else if ((second_block == 1) && found_img) {
			if (rcv_size < (fw_len) ) {
				if ((rcv_size + len) >= fw_len) {
					memcpy(g_gphy_fw_dma+rcv_size, local_buf, (fw_len-rcv_size));
					first_block = 0;
					found_img = 0;
					second_block = 0;
					found_magic = 0;
					ltq_gphy_firmware_config();
				} else {
					memcpy(g_gphy_fw_dma+rcv_size, local_buf, (len));
					rcv_size += len;
				}
			} else {
				first_block = 0;
				found_img = 0;
				second_block = 0;
				found_magic = 0;
				ltq_gphy_firmware_config();
			}
		}
	} else if (strcmp(g_gphy_fw_mode, "11G22F-FW") == 0) {
		if ((first_block == 1) && (!second_block) && found_img) {
			if (fmode == 1 ) {
				g_gphy_fw_dma = (u8*) kmalloc (fw_len * sizeof (unsigned char), GFP_KERNEL);
				if (g_gphy_fw_dma == NULL )
					return -1;
				memset(g_gphy_fw_dma, 0, fw_len);
				rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
				memcpy(g_gphy_fw_dma, local_buf+(found_magic * sizeof(gphy_image_header_t)), rcv_size);
			}
			if (fmode == 2 ) {
				f_gphy_fw_dma = (u8*) kmalloc (fw_len * sizeof (unsigned char), GFP_KERNEL);
				if (f_gphy_fw_dma == NULL )
					return -1;
				memset(f_gphy_fw_dma, 0, fw_len);
				rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
				memcpy(f_gphy_fw_dma, local_buf+(found_magic * sizeof(gphy_image_header_t)), rcv_size);
			}
			second_block = 1;
		} else if ((second_block == 1) && found_img) {
			if (rcv_size < (fw_len) ) {
				if ((rcv_size + len) >= fw_len) {
					if (fmode == 1 ) {
						if ((len <= 4096) &&  (len > 64)) {
							update_ge = 1;
							memcpy(g_gphy_fw_dma+rcv_size, local_buf, (fw_len-rcv_size));
							rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
							memcpy(&header, (local_buf + (found_magic * sizeof(gphy_image_header_t))), sizeof(gphy_image_header_t));
						}
					}
					if (fmode == 2 ) {
						if ((len <= 4096) &&  (len > 64)) {
							update_fe = 1;
							memcpy(f_gphy_fw_dma+rcv_size, local_buf, (fw_len-rcv_size));
							rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
							memcpy(&header, (local_buf + (found_magic * sizeof(gphy_image_header_t))), sizeof(gphy_image_header_t));
						}
					}
					if (((strncmp(header.imname, "VR9 V1.2 GPHY GE", sizeof(header.imname)) == 0)	\
						|| (strncmp(header.imname, "AR10 V1.2 GPHY GE", sizeof(header.imname)) == 0))	\
						&& (dev_id == 1))	{
							first_block = 1;
							fw_len = header.imsize;
							found_img = 1;
							fmode = 1;
							dev_info(gphy_dev, "%s:   Found:%s  FW \n", __func__,header.imname);
							g_gphy_fw_dma = (u8*) kmalloc (fw_len * sizeof (unsigned char), GFP_KERNEL);
							if (g_gphy_fw_dma == NULL ) {
								pr_err("ERROR: Line: %d: %s:   Found:%s  FW \n", __LINE__, __func__,header.imname);
								return -1;
							}
							memset(g_gphy_fw_dma, 0, fw_len);
							found_magic++;
							rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
							memcpy(g_gphy_fw_dma, local_buf+(found_magic * sizeof(gphy_image_header_t)), rcv_size);
					}
					if (((strncmp(header.imname, "VR9 V1.2 GPHY FE", sizeof(header.imname)) == 0)	\
						|| (strncmp(header.imname, "AR10 V1.2 GPHY FE", sizeof(header.imname)) == 0 ))	\
						&& (dev_id == 1)) {
							first_block = 1;
							fw_len = header.imsize;
							found_img = 1;
							fmode = 2;
							found_magic++;
							dev_info(gphy_dev, "%s:  Found:%s  FW \n", __func__,header.imname);
							f_gphy_fw_dma = (u8*) kmalloc (fw_len * sizeof (unsigned char), GFP_KERNEL);
							if (f_gphy_fw_dma == NULL ) {
								pr_err("ERROR: Line: %d: %s:   Found:%s  FW \n", __LINE__, __func__,header.imname);
								return -1;
							}
							memset(f_gphy_fw_dma, 0, fw_len);
							rcv_size = (len - (found_magic * sizeof(gphy_image_header_t)));
							memcpy(f_gphy_fw_dma, local_buf+(found_magic * sizeof(gphy_image_header_t)), rcv_size);
					}
					if ( update_ge == 1) {
						if (g_gphy_fw_dma) {
							ltq_mix_firmware_config(g_gphy_fw_dma, 1);
							update_ge = 0;
						}
					}
					if ( update_fe == 1) {
						if (f_gphy_fw_dma) {
							ltq_mix_firmware_config(f_gphy_fw_dma, 2);
							update_fe = 0;
						}
					}
				} else {
					if (fmode == 1 ) {
						memcpy(g_gphy_fw_dma+rcv_size, local_buf, (len));
					}
					if (fmode == 2 ) {
						memcpy(f_gphy_fw_dma+rcv_size, local_buf, (len));
					}
					rcv_size += len;
				}
			} else {
				first_block = 0;
				found_img = 0;
				second_block = 0;
				found_magic = 0;
				if ( update_ge == 1) {
					if (g_gphy_fw_dma)
						ltq_mix_firmware_config(g_gphy_fw_dma, 1);
				}
				if ( update_fe == 1) {
					if (f_gphy_fw_dma)
						ltq_mix_firmware_config(f_gphy_fw_dma, 2);
				}
			}
		}
	}
	return len;
}

static int proc_read_phy_fw(struct seq_file *seq, void *v)
{
    return 0;
}
/*
static void space_ignore(char **p, int *len)
{
	while ( *len && (**p <= ' ' || **p == ':' || **p == '.' || **p == ',') ) {
		(*p)++;
		(*len)--;
	}
}
*/
static int get_number(char **p, int *len, int is_hex)
{
	int ret = 0, n = 0;
	if ( (*p)[0] == '0' && (*p)[1] == 'x' ) {
		is_hex = 1;
		(*p) += 2;
		(*len) -= 2;
	}
	if ( is_hex ) {
		while ( *len && ((**p >= '0' && **p <= '9')	\
			|| (**p >= 'a' && **p <= 'f') || (**p >= 'A' && **p <= 'F'))) {
			if ( **p >= '0' && **p <= '9' )
				n = **p - '0';
			else if ( **p >= 'a' && **p <= 'f' )
				n = **p - 'a' + 10;
			else if ( **p >= 'A' && **p <= 'F' )
				n = **p - 'A' + 10;
			ret = (ret << 4) | n;
			(*p)++;
			(*len)--;
		}
	} else {
		while ( *len && **p >= '0' && **p <= '9' ) {
			n = **p - '0';
			ret = ret * 10 + n;
			(*p)++;
			(*len)--;
		}
	}
	return ret;
}

#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
static int proc_read_phy_ssd_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_phy_on, NULL);
}

static ssize_t proc_write_phy_ssd(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	int len, rlen, num;
	char local_buf[512] = {0};
	char *p;
	len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
	rlen = len - copy_from_user((local_buf), buf, len);
	while ( rlen && local_buf[rlen - 1] <= ' ' )
		rlen--;
	local_buf[rlen] = 0;
	for ( p = local_buf; *p && *p <= ' '; p++, rlen-- );
	if ( !*p )
		return 0;
	num = get_number(&p, &rlen, 0);
	ssd_interval = ( num < 2 )? 1 : num;
	return len;
}
static struct file_operations file_phy_ssd_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_phy_ssd_open,
    .read       = seq_read,
    .write      = proc_write_phy_ssd,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#endif /* PHY_SSD_ERROR */

static struct file_operations file_phy_on_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_phy_on_open,
    .read       = seq_read,
    .write      = proc_write_phy_on,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static ssize_t proc_write_phy_on(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	int len, rlen, num;
	char local_buf[512] = {0};
	char *p;
	len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
	rlen = len - copy_from_user((local_buf), buf, len);
	while ( rlen && local_buf[rlen - 1] <= ' ' )
		rlen--;
	local_buf[rlen] = 0;
	for ( p = local_buf; *p && *p <= ' '; p++, rlen-- );
	if ( !*p )
		return 0;
	num = get_number(&p, &rlen, 0);
	on_interval = ( num < 4 )? 3 : num;
	return len;
}

static int proc_read_phy_on_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_phy_on, NULL);
}

static int proc_read_phy_on(struct seq_file *seq, void *v)
{
    return 0;
}

static struct file_operations file_phy_off_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_phy_off_open,
    .read       = seq_read,
    .write      = proc_write_phy_off,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static ssize_t proc_write_phy_off(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	int len, rlen, num;
	char local_buf[512] = {0};
	char *p;
	len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
	rlen = len - copy_from_user((local_buf), buf, len);
	while ( rlen && local_buf[rlen - 1] <= ' ' )
		rlen--;
	local_buf[rlen] = 0;
	for ( p = local_buf; *p && *p <= ' '; p++, rlen-- );
	if ( !*p )
		return 0;
	num = get_number(&p, &rlen, 0);
	off_interval = ( num < 4 )? 5 : num;
	return len;
}
static int proc_read_phy_off_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_phy_off, NULL);
}
static int proc_read_phy_off(struct seq_file *seq, void *v)
{
    return 0;
}
static struct file_operations file_ver_fops = {
	.owner	= THIS_MODULE,
	.open	= proc_read_ver_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release	= single_release,
};

static int proc_read_ver_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_read_ver, NULL);
}

static int proc_read_ver(struct seq_file *seq, void *v)
{
	seq_printf(seq, "LTQ GPHY driver %s, version %s, on:%d, off:%d, pw_mode:%d, ssd_interval:%d\n",	\
		g_gphy_fw_mode, version, on_interval, off_interval, pw_save_mode, ssd_interval);
	return 0;
}

static int proc_file_create(void)
{
	struct proc_dir_entry *entry;

	g_proc_gphy_dir = proc_mkdir("driver/ltq_gphy", NULL);
	if (!g_proc_gphy_dir)
		return -ENOMEM;
	entry  = proc_create ("version", S_IRUGO, g_proc_gphy_dir, &file_ver_fops);
	if (!entry)
		goto Err0;
	entry = proc_create ("phyfirmware", S_IRUGO|S_IWUSR, g_proc_gphy_dir, &file_phy_fw_fops);
	if (!entry)
		goto Err2;
	entry = proc_create ("on_delay", S_IRUGO|S_IWUSR, g_proc_gphy_dir, &file_phy_on_fops);
	if (!entry)
		goto Err3;
	entry = proc_create ("off_delay", S_IRUGO|S_IWUSR, g_proc_gphy_dir, &file_phy_off_fops);
	if (!entry)
		goto Err4;
#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
	entry = proc_create ("ssd_delay", S_IRUGO|S_IWUSR, g_proc_gphy_dir, &file_phy_ssd_fops);
	if (!entry)
		goto Err5;
#endif /* PHY_SSD_ERROR */
	return 0;
#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
Err5:
	remove_proc_entry("off_delay", g_proc_gphy_dir);
#endif /* PHY_SSD_ERROR */
Err4:
	remove_proc_entry("on_delay", g_proc_gphy_dir);
Err3:
	remove_proc_entry("phyfirmware", g_proc_gphy_dir);
Err2:
	remove_proc_entry("version", g_proc_gphy_dir);
Err0:
	remove_proc_entry("driver/ltq_gphy", NULL);
	return -ENOMEM;
}

static void proc_file_delete(void)
{
	if (!g_proc_gphy_dir)
		return;
	remove_proc_entry("version", g_proc_gphy_dir);
	remove_proc_entry("phyfirmware", g_proc_gphy_dir);
	remove_proc_entry("driver/ltq_gphy", NULL);
}

static dma_addr_t xway_gphy_load (unsigned char *pData)
{
	dma_addr_t dev_addr = 0;
	void *fw_addr;
	size_t size;
	/*
	 * GPHY cores need the firmware code in a persistent and contiguous
	 * memory area with a 16 kB boundary aligned start address
	 */
	size = fw_len + XWAY_GPHY_FW_ALIGN;
	if (g_fw_addr) {
		dma_free_coherent(gphy_dev, size, g_fw_addr, g_dev_addr);
		g_fw_addr = NULL;
	}
	fw_addr = dma_alloc_coherent(gphy_dev, size, &dev_addr, GFP_KERNEL);
	if (fw_addr) {
		fw_addr = PTR_ALIGN(fw_addr, XWAY_GPHY_FW_ALIGN);
		dev_addr = ALIGN(dev_addr, XWAY_GPHY_FW_ALIGN);
		memcpy(fw_addr, pData, fw_len);
		g_fw_addr = fw_addr;
		g_dev_addr = dev_addr;
	} else {
		dev_err(gphy_dev, "failed to alloc firmware memory\n");
	}
	return dev_addr;
}

static dma_addr_t xway_fphy_load (unsigned char *pData)
{
	dma_addr_t dev_addr = 0;
	void *fw_addr;
	size_t size;
	/*
	 * GPHY cores need the firmware code in a persistent and contiguous
	 * memory area with a 16 kB boundary aligned start address
	 */
	size = fw_len + XWAY_GPHY_FW_ALIGN;
	if (f_fw_addr) {
		dma_free_coherent(gphy_dev, size, f_fw_addr, f_dev_addr);
		f_fw_addr = NULL;
	}
	fw_addr = dma_alloc_coherent(gphy_dev, size, &dev_addr, GFP_KERNEL);
	if (fw_addr) {
		fw_addr = PTR_ALIGN(fw_addr, XWAY_GPHY_FW_ALIGN);
		dev_addr = ALIGN(dev_addr, XWAY_GPHY_FW_ALIGN);
		memcpy(fw_addr, pData, fw_len);
		f_fw_addr = fw_addr;
		f_dev_addr = dev_addr;
	} else {
		dev_err(gphy_dev, "failed to alloc firmware memory\n");
	}
	return dev_addr;
}

/* reset and boot a gphy. these phys only exist on xrx200 SoC */
int xway_gphy_rcu_config(unsigned int id, dma_addr_t dev_addr)
{
	if (of_machine_is_compatible("lantiq,vr9") || of_machine_is_compatible("lantiq,xrx220") ) {
		struct clk *clk;
		clk = clk_get_sys("1f203000.rcu", "gphy");
		if (IS_ERR(clk))
			return PTR_ERR(clk);
		clk_enable(clk);
	}

	if (of_machine_is_compatible("lantiq,vr9") || of_machine_is_compatible("lantiq,xrx220") ) {
		if (id > 1) {
			dev_info(gphy_dev, "%u is an invalid gphy id\n", id);
			return -EINVAL;
		}
		ltq_rcu_w32_mask(0, xrx200_gphy[id].rd, RCU_RST_REQ);
		ltq_rcu_w32(dev_addr, xrx200_gphy[id].addr);
		ltq_rcu_w32_mask(xrx200_gphy[id].rd, 0,  RCU_RST_REQ);
		dev_info(gphy_dev, "booting GPHY%u firmware at %X for VR9\n", id, dev_addr);
	} else if (of_machine_is_compatible("lantiq,ar10")) {
		if (id > 2) {
			dev_info(gphy_dev, "%u is an invalid gphy id\n", id);
			return -EINVAL;
		}
		ltq_rcu_w32_mask(0, xrx300_gphy[id].rd, RCU_RST_REQ);
		ltq_rcu_w32(dev_addr, xrx300_gphy[id].addr);
		ltq_rcu_w32_mask(xrx300_gphy[id].rd, 0,  RCU_RST_REQ);
		dev_info(gphy_dev, "booting GPHY%u firmware at %X for AR10\n", id, dev_addr);
	} else if (of_machine_is_compatible("lantiq,grx390")) {
		if (id > 3) {
			dev_info(gphy_dev, "%u is an invalid gphy id\n", id);
			return -EINVAL;
		}
		ltq_rcu_w32_mask(0, xrx330_gphy[id].rd, RCU_RST_REQ);
		ltq_rcu_w32(dev_addr, xrx330_gphy[id].addr);
		ltq_rcu_w32_mask(xrx330_gphy[id].rd, 0,  RCU_RST_REQ);
		dev_info(gphy_dev, "booting GPHY%u firmware at %X for GRX390\n", id, dev_addr);
	}
	return 0;
}

static int ltq_gphy_firmware_config()
{
	int i;
	dma_addr_t data_ptr;
	data_ptr = xway_gphy_load(g_gphy_fw_dma);
	if (g_gphy_fw_dma)
		kfree(g_gphy_fw_dma);
	if (!data_ptr)
		return -ENOMEM;
	for (i = 0; i < g_no_phys; i++)
		xway_gphy_rcu_config(i,data_ptr);
	pr_err("%s: fw_mode:%s, no of phys:%d,data_ptr:%X\n", __func__, g_gphy_fw_mode, g_no_phys,data_ptr);
	return 0;
}

static int ltq_mix_firmware_config (u8 *fw_ptr, int mix_mode)
{
	int i;
	dma_addr_t data_ptr = 0;
	if (mix_mode == 1)
		data_ptr = xway_gphy_load(fw_ptr);
	if (mix_mode == 2)
		data_ptr = xway_fphy_load(fw_ptr);
	if (fw_ptr)
		kfree(fw_ptr);
	if (!data_ptr)
		return -ENOMEM;
	for (i = 0; i < g_no_phys; i++) {
		if ((mix_mode == 1) && (phy_fw_type[i] == 1))
			xway_gphy_rcu_config(i,data_ptr);
		if ((mix_mode == 2) && (phy_fw_type[i] == 2))
			xway_gphy_rcu_config(i,data_ptr);
	}
	pr_err("%s: fw_mode:%s, no of phys:%d,data_ptr:%X,mix_mode:%d \n", __func__, g_gphy_fw_mode, g_no_phys,data_ptr, mix_mode);
	return 0;
}

static void lq_ethsw_mdio_data_write(unsigned int phyAddr, unsigned int regAddr,unsigned int data)
{
	IFX_ETHSW_MDIO_data_t mdio_data;
	memset(&mdio_data, 0, sizeof(IFX_ETHSW_MDIO_data_t));
	mdio_data.nAddressDev = phyAddr;
	mdio_data.nAddressReg = regAddr;
	mdio_data.nData = data;
	ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_MDIO_DATA_WRITE, (unsigned int)&mdio_data);
    return ;
}

static unsigned short lq_ethsw_mdio_data_read(unsigned int phyAddr, unsigned int regAddr)
{
	IFX_ETHSW_MDIO_data_t mdio_data;
	memset(&mdio_data, 0, sizeof(IFX_ETHSW_MDIO_data_t));
	mdio_data.nAddressDev = phyAddr;
	mdio_data.nAddressReg = regAddr;
	ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_MDIO_DATA_READ, (unsigned int)&mdio_data);
    return (mdio_data.nData);
}

static unsigned short gsw_mmd_data_read(unsigned int phyAddr, unsigned int regAddr)
{
	IFX_ETHSW_MMD_data_t mmd_data;
	memset(&mmd_data, 0, sizeof(IFX_ETHSW_MMD_data_t));
	mmd_data.nAddressDev = phyAddr;
	mmd_data.nAddressReg = regAddr;
	ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_MMD_DATA_READ, (unsigned int)&mmd_data);
    return (mmd_data.nData);
}

static int gphy_pw_save_thread (void *arg)
{
	allow_signal(SIGKILL);	
	while(!kthread_should_stop ()) {
		u8 index;
		IFX_ETHSW_portLinkCfg_t param;
		memset(&param, 0, sizeof(IFX_ETHSW_portLinkCfg_t));
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current))
			break;
		/* Get the port Link Status  */
		for (index = 0; index < NUM_OF_PORTS; index++) {
			if ((gphy_power_down[index] == 1) && (gphy_link_status[index] == 0)) {
				u16 reg0_val;
				reg0_val = lq_ethsw_mdio_data_read(index, 0x0);
				reg0_val &= ~(0x800);
				lq_ethsw_mdio_data_write(index, 0x0, reg0_val);
			}
		}
		interruptible_sleep_on_timeout(&gphy_pw_wait, (on_interval * HZ) );
		for (index = 0; index < NUM_OF_PORTS; index++) {
			param.nPortId = index;
			ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_PORT_LINK_CFG_GET, (unsigned int)&param);
			if (param.eLink == 0 ) {
				gphy_link_status[index] = 1;
				power_down_cnt[index] = 0;
			} else {
				gphy_link_status[index] = 0;
				power_down_cnt[index]++;
				if ((power_down_cnt[index] % 6) != 0) {
					u16 reg0_val;
					reg0_val = lq_ethsw_mdio_data_read(index, 0x0);
					reg0_val |= (0x800);
					lq_ethsw_mdio_data_write(index, 0x0, reg0_val);
				}
				gphy_power_down[index] = 1;
			}
		}
		/*poll again  once configured time is up */
		interruptible_sleep_on_timeout(&gphy_pw_wait, (off_interval * HZ) );
	}
	return 0;
}
#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
static int gphy_ssd_err_thread (void *arg)
{
	allow_signal(SIGKILL);	
	while(!kthread_should_stop ()) {
		u8 index;
		IFX_ETHSW_portLinkCfg_t param;
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current))
			break;
		/* Get the port Link Status  */
		for (index = 0; index < NUM_OF_PORTS; index++) {
			memset(&param, 0, sizeof(IFX_ETHSW_portLinkCfg_t));
			param.nPortId = index;
			ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_PORT_LINK_CFG_GET, (unsigned int)&param);
			if ((param.eLink == 0) && (gphy_link_status[index] == 0)) {
				gphy_link_status[index] = 1;
				ssd_cnt[index] = 0;
			} else {
				gphy_link_status[index] = 0;
				ssd_cnt[index] = 0;
			}
		}
		for (index = 0; index < NUM_OF_PORTS; index++) {
			if ((gphy_link_status[index] == 1)) {
				u16 reg0_val;
				ssd_cnt[index]++;
				if ((ssd_cnt[index] % 3) == 0 ) {
					ssd_cnt[index] = 0;
					reg0_val = gsw_mmd_data_read(index, 0x1F07be);
					if ((reg0_val >> 1) & 0x1) {
						u16 reg1_val;
						reg1_val = lq_ethsw_mdio_data_read(index, 0x0);
						reg1_val |= (1 << 9);
						lq_ethsw_mdio_data_write(index, 0x0, reg1_val);
					}
				}
			}
		}
		interruptible_sleep_on_timeout(&gphy_ssd_wait, (ssd_interval * HZ) );
	}
	return 0;
}
#endif /* PHY_SSD_ERROR */
//For VRX220 SW control PHY LED
/* Switches on the LED */
/* Input:   port
 *		:	on_off */
 /* Process: Use the GPIO to ON/OFF the LED 
 */
 void gphy_data_led_on_off (int port, int on_off)
 {
 	u32 gpio_pin = GPHY_2_GPIO;
 	switch (port) {
 		case 2:
 			gpio_pin = GPHY_2_GPIO;
 			break;
 		case 3:
 			gpio_pin = GPHY_3_GPIO;
			break;
		case 4:
			gpio_pin = GPHY_4_GPIO;
			break;	
		case 5:
			gpio_pin = GPHY_5_GPIO;
			break;
	}
	if (on_off) {
		gpio_set_value(gpio_pin, 1);
	} else {
		gpio_set_value(gpio_pin, 0);
	}
}

static int gphy_rmon_poll_thread (void *arg)
{
	int port;
	int port_rx[NUM_OF_PORTS];
	int port_rx_prev[NUM_OF_PORTS] = {0,0,0,0,0,0};
	int port_tx[NUM_OF_PORTS];
	int port_tx_prev[NUM_OF_PORTS] = {0,0,0,0,0,0};
	IFX_ETHSW_RMON_cnt_t param;
	IFX_ETHSW_portLinkCfg_t param_link;
	printk (KERN_INFO "start %p ..\n", current);
	allow_signal(SIGKILL);
	while(!kthread_should_stop ()) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (signal_pending(current))
			break;
		swithc_api_fd = ltq_ethsw_api_kopen("/dev/switch_api/0");	 
		for (port = 2; port < NUM_OF_PORTS; port++) {
			memset(&param, 0, sizeof(IFX_ETHSW_RMON_cnt_t));
			memset(&param_link, 0, sizeof(IFX_ETHSW_portLinkCfg_t));
			param.nPortId = port;
			param_link.nPortId = port;
			ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_RMON_GET,(unsigned int)&param);
			port_rx[port] = param.nRxGoodPkts;
			port_tx[port] = param.nTxGoodPkts;
			if ((port_rx[port] != port_rx_prev[port]) ||(port_tx[port] != port_tx_prev[port])) {
				if (gphy_led_status_on[port] == 0) {
					gphy_led_status_on[port] = 1;
					gphy_data_led_on_off(port, LED_ON);
				} else {
					gphy_led_status_on[port] = 0;
					gphy_data_led_on_off(port, LED_OFF);
				}
				port_rx_prev[port] = port_rx[port];
				port_tx_prev[port] = port_tx[port];
			} else {
				ltq_ethsw_api_kioctl(swithc_api_fd, IFX_ETHSW_PORT_LINK_CFG_GET, (unsigned int)&param_link);
				if (param_link.eLink == 0 ) {
					gphy_led_status_on[port] = 1;
					gphy_data_led_on_off (port, LED_ON);
				} else {
					gphy_led_status_on[port] = 0;
					gphy_data_led_on_off (port, LED_OFF);
				}
			}
		}
		ltq_ethsw_api_kclose(swithc_api_fd);
		msleep(30);
	}
	return 0;
}

int AR10_F2_GPHY_LED_init(void)
{
	if (!gpio_request(GPHY_2_GPIO, "SW-LED"))
		gpio_direction_output(GPHY_2_GPIO, 1);  //set gpio direction as  output
	else
		printk(KERN_EMERG "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (!gpio_request(GPHY_3_GPIO, "SW-LED"))
		gpio_direction_output(GPHY_3_GPIO, 1);  //set gpio direction as  output
	else
		return 1;
	if (!gpio_request(GPHY_4_GPIO, "SW-LED"))
		gpio_direction_output(GPHY_4_GPIO, 1);  //set gpio direction as  output
	else
		return 1;
	if (!gpio_request(GPHY_5_GPIO, "SW-LED"))
		gpio_direction_output(GPHY_5_GPIO, 1);  //set gpio direction as  output
	else
		return 1;
	gphy_rmon_poll_thread_id = kthread_create(gphy_rmon_poll_thread, NULL, \
		"gphy_rmon_poll_thread");
	if (!IS_ERR(gphy_rmon_poll_thread_id)) {
		printk (KERN_EMERG "GPHY RMON poll thread created..\n");
		wake_up_process(gphy_rmon_poll_thread_id);
	}
	return 0;
}
//end For VRX220 SW control PHY LED

static int __init gphy_driver_init (struct platform_device *pdev)
{	
	const __be32 *no_phys;
	gphy_dev = &pdev->dev;
	gphy_node = pdev->dev.of_node;

	off_interval = 5;
	on_interval = 3;
	pw_save_mode = 0;
	ssd_err_mode = 0;
#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
	ssd_err_mode = 1;
	ssd_interval = 4;
#endif /* PHY_SSD_ERROR */
	dev_id = 0;
	if (of_property_read_string(pdev->dev.of_node, "fw-mode", &g_gphy_fw_mode)) {
		dev_err(&pdev->dev, "failed to read  firmware mode\n");
		return 0;
	}
	no_phys = of_get_property(gphy_node, "no_of_phys", NULL);
	if (!no_phys) {
		dev_err(&pdev->dev, "failed to get maximum number of internal gphys ports\n");
		return 0;
	}
	g_no_phys = (*no_phys);
	
	if (of_machine_is_compatible("lantiq,vr9") || of_machine_is_compatible("lantiq,xrx220") ) {
		int type = ltq_get_soc_type();
		if (type == SOC_TYPE_VR9) /*SOC_TYPE_VR9_2*/
			dev_id = 0;
		else
			dev_id = 1;
	} else if (of_machine_is_compatible("lantiq,ar10")) {
		dev_id = 1;
		no_phys = of_get_property(gphy_node, "pw_save_mode", NULL);
		if (no_phys)
			pw_save_mode = (*no_phys);
	} else if (of_machine_is_compatible("lantiq,grx390")) {
		dev_id = 1;
		no_phys = of_get_property(gphy_node, "pw_save_mode", NULL);
		if (no_phys)
			pw_save_mode = (*no_phys);
	}
	if (of_machine_is_compatible("lantiq,xrx220") ) {
		int i;
		of_property_read_u32_array(gphy_node, "phy_port_nos", phy_port_nos, g_no_phys);
		of_property_read_u32_array(gphy_node, "phy_fw_type", phy_fw_type, g_no_phys);
		for (i = 0; i < g_no_phys; i++) {
			pr_err("phy_port_nos[%d]:%d, phy_fw_type[%d]:%d\n", i, phy_port_nos[i], i, phy_fw_type[i]);
		}
	}
	proc_file_create();
	pr_err("%s: fw_mode:%s, no of phys:%d, mode:%d\n", \
		__func__, g_gphy_fw_mode, g_no_phys, pw_save_mode);
	if (of_machine_is_compatible("lantiq,grx390")) {
		if ( (pw_save_mode == 1) ||  (ssd_err_mode == 1) ) {
			swithc_api_fd = ltq_ethsw_api_kopen("/dev/switch_api/0");
			if (swithc_api_fd == 0) {
				pr_err("%s: Open SWAPI device FAILED !!\n", __func__ );
				return -EIO;
			}
			if ( pw_save_mode == 1 ) {
				init_waitqueue_head(&gphy_pw_wait);
				gphy_pw_id = kthread_create(gphy_pw_save_thread, NULL, "gphy_pw_save");
				if (!IS_ERR(gphy_pw_id))
					wake_up_process(gphy_pw_id);
			}
	#if defined(PHY_SSD_ERROR) && PHY_SSD_ERROR
			if ( ssd_err_mode == 1 ) {
				init_waitqueue_head(&gphy_ssd_wait);
				gphy_ssd_id = kthread_create(gphy_ssd_err_thread, NULL, "gphy_ssd_wait");
				if (!IS_ERR(gphy_ssd_id))
					wake_up_process(gphy_ssd_id);
			}
	#endif /* PHY_SSD_ERROR */
		}
	}
        //For VRX220 SW control PHY LED
	if (of_machine_is_compatible("lantiq,xrx220"))
	    AR10_F2_GPHY_LED_init();
	return 0;
}

static int __init gphy_driver_exit (struct platform_device *pdev)
{
	proc_file_delete();
	if ( pw_save_mode == 1 )
		ltq_ethsw_api_kclose(swithc_api_fd);
	return 0;
}

static int xway_gphy_fw_probe(struct platform_device *pdev)
{
	gphy_driver_init(pdev);
	return 0;
}

static int xway_gphy_fw_remove(struct platform_device *pdev)
{
	gphy_driver_exit(pdev);
	return 0;
}

static const struct of_device_id xway_gphy_fw_match[] = {
	{ .compatible = "lantiq,xway-phy-fw" },
	{},
};
MODULE_DEVICE_TABLE(of, xway_gphy_fw_match);

static struct platform_driver xway_gphy_driver = {
	.probe = xway_gphy_fw_probe,
	.remove = xway_gphy_fw_remove,
	.driver = {
		.name = "gphy-fw",
		.owner = THIS_MODULE,
		.of_match_table = xway_gphy_fw_match,
	},
};

module_platform_driver(xway_gphy_driver);

MODULE_AUTHOR("Reddy Mallikarjuna <reddy.mallikarjun@lantiq.com>");
MODULE_DESCRIPTION("Lantiq GPHY PHY Firmware Loader");
MODULE_LICENSE("GPL");


