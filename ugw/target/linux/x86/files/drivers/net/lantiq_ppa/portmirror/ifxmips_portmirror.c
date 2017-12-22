/******************************************************************************
**
** FILE NAME    : ifxmips_portmirror.c
** PROJECT      : IFX UEIP
** MODULES      : IFX port mirror driver
** DATE         : 03 January  2012
** AUTHOR       : Santosh Gowda
** DESCRIPTION  : IFX port mirror driver
** COPYRIGHT    :       Copyright (c) 2012
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date                $Author                 $Comment
** 03 January 2012         Santosh Gowda      	Initial release
******************************************************************************/


/*
 *  Common Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/atmdev.h>
#include <linux/init.h>
#include <linux/etherdevice.h>  /*  eth_type_trans  */
#include <linux/ethtool.h>      /*  ethtool_cmd     */
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <net/xfrm.h>

#if !defined(DISABLE_INLINE) || !DISABLE_INLINE
  #define INLINE                                inline
#else
  #define INLINE
#endif

/*
 *  Proc File
 */
static INLINE void proc_file_create(void);
static INLINE void proc_file_delete(void);
static int proc_read_port_mirror(struct seq_file *, void *);
static ssize_t proc_write_port_mirror(struct file *, const char __user *, size_t, loff_t *);
static int proc_read_port_mirror_seq_open(struct inode *, struct file *);

static struct net_device *get_mirror_netdev(void);
static uint32_t is_device_type_wireless (void);

/*
 *  Global declaration
 */
static struct net_device *g_mirror_netdev = NULL;
static struct proc_dir_entry *net_proc = NULL;

struct net_device* (*ifx_get_mirror_netdev)(void) = NULL;
EXPORT_SYMBOL(ifx_get_mirror_netdev);

uint32_t (*ifx_is_device_type_wireless) (void) = NULL;
EXPORT_SYMBOL(ifx_is_device_type_wireless);


static INLINE int stricmp(const char *p1, const char *p2)
{
    int c1, c2;

    while ( *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
    }

    return *p1 - *p2;
}

static INLINE int strincmp(const char *p1, const char *p2, int n)
{
    int c1 = 0, c2;

    while ( n && *p1 && *p2 )
    {
        c1 = *p1 >= 'A' && *p1 <= 'Z' ? *p1 + 'a' - 'A' : *p1;
        c2 = *p2 >= 'A' && *p2 <= 'Z' ? *p2 + 'a' - 'A' : *p2;
        if ( (c1 -= c2) )
            return c1;
        p1++;
        p2++;
        n--;
    }

    return n ? *p1 - *p2 : c1;
}

static struct net_device *get_mirror_netdev(void)
{
    return g_mirror_netdev;
}

static uint32_t is_device_type_wireless(void)
{
    if ( strincmp(g_mirror_netdev->name, "wlan" , 4) == 0 )
        return 1;

    return 0;
}

static struct file_operations g_proc_file_mirror_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_port_mirror_seq_open,
    .read       = seq_read,
    .write      = proc_write_port_mirror,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_port_mirror_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_port_mirror, NULL);
}

static INLINE void proc_file_create(void)
{
    struct proc_dir_entry *res = NULL;

    res = proc_create("mirror", S_IRUGO|S_IWUSR,net_proc,&g_proc_file_mirror_fops);
}

static INLINE void proc_file_delete(void)
{
    remove_proc_entry("mirror",
                      net_proc);
}

static int proc_read_port_mirror(struct seq_file *seq, void *v)
{
    int len = 0;
    char devname[IFNAMSIZ + 1] = {0};

    if ( g_mirror_netdev == NULL )
        strcpy(devname, "disable");
    else
        strncpy(devname, g_mirror_netdev->name, IFNAMSIZ);

    seq_printf(seq, "mirror: %s\n", devname);

    return len;
}

static ssize_t proc_write_port_mirror(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char str[2048];
    char *p;
    int len, rlen;

    len = count < sizeof(str) ? count : sizeof(str) - 1;
    rlen = len - copy_from_user(str, buf, len);
    while ( rlen && str[rlen - 1] <= ' ' )
        rlen--;
    str[rlen] = 0;
    for ( p = str; *p && *p <= ' '; p++, rlen-- );
    if ( !*p )
        return count;

    if ( stricmp(p, "help") == 0 )
    {
        printk("echo <disable|port> > /proc/mirror\n");
	    printk("  disable: port mirroring\n");
	    printk("  port:    mirroring interface name like eth0, wlan0, etc..\n");
	    return count;
    }

    if ( g_mirror_netdev != NULL )
        g_mirror_netdev = NULL;
    if ( stricmp(p, "disable") == 0 )
        printk("disable port mirror\n");
    else
    {
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
        g_mirror_netdev = dev_get_by_name(p);
  #else
        g_mirror_netdev = dev_get_by_name(&init_net, p);
  #endif
        if ( g_mirror_netdev != NULL )
        {
            printk("mirror: %s\n", p);
            dev_put(g_mirror_netdev);
	    }
        else
            printk("mirror: none, can't find device \"%s\"\n", p);
    }
    return count;
}


static int __init port_mirror_init(void)
{
    proc_file_create();
    ifx_get_mirror_netdev = get_mirror_netdev;
    ifx_is_device_type_wireless = is_device_type_wireless;

    return 0;
}

static void __exit port_mirror_exit(void)
{
    ifx_get_mirror_netdev = NULL;
    ifx_is_device_type_wireless = NULL;
    proc_file_delete();
}

module_init(port_mirror_init);
module_exit(port_mirror_exit);

MODULE_AUTHOR("Santosh Gowda");
MODULE_DESCRIPTION("Port mirroring implementation");
MODULE_LICENSE("GPL");
