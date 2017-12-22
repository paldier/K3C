/*******************************************************************************
 **
 ** FILE NAME    : ltq_tmu_hal_proc_changes.c
 ** PROJECT      : TMU HAL
 ** MODULES      : TMU (QoS Engine )
 **
 ** DATE         : 8 Aug 2014
 ** AUTHOR       : Purnendu Ghosh
 ** DESCRIPTION  : TMU HAL Layer
 ** COPYRIGHT    :              Copyright (c) 2009
 **                          Lantiq Deutschland GmbH
 **                   Am Campeon 3; 85579 Neubiberg, Germany
 **
 **   For licensing information, see the file 'LICENSE' in the root folder of
 **   this software module.
 **
 ** HISTORY
 ** $Date        $Author                $Comment
 ** 8 Aug 2014  Purnendu Ghosh         Initiate Version
 *******************************************************************************/


/*
 *  Common Header File
 */
//#include <linux/autoconf.h>
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
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <net/checksum.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 13)
#include <net/ipip.h>
#else
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>
#include <net/ip_tunnels.h>
#endif
#include <linux/if_arp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <net/ip6_tunnel.h>
#include <net/ipv6.h>


/*
 *  Chip Specific Head File
 */
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include <net/lantiq_cbm_api.h>
//#include <net/drv_tmu_reg.h>
#include <net/drv_tmu_ll.h>
#include <net/datapath_api.h>
//#include "ltq_tmu_hal.h"
#include "ltq_tmu_hal_debug.h"

#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))
#if 0
#define TMU_DEBUG_ERR (1<< 0)
#define TMU_DEBUG_TRACE (1<<1)
#define TMU_DEBUG_DEFAULT (1<<2)
#define TMU_ENABLE_ALL_DEBUG (~0)
extern uint32_t g_tmu_dbg;
#endif
//#if TMU_TEST_HASAN
extern struct proc_dir_entry *g_ppa_proc_dir ;
extern int tmu_hal_get_queue_info(struct net_device *netdev);
extern int tmu_hal_get_queue_map(struct net_device *netdev);
extern int tmu_hal_get_detailed_queue_map(uint32_t queue_index);
static int proc_read_queue_info_seq_open(struct inode *, struct file *);
static int proc_read_queue_info(struct seq_file *, void *);
static int proc_read_tc_map_seq_open(struct inode *, struct file *);
static int proc_read_tc_map(struct seq_file *, void *);
static int proc_read_queue_map_seq_open(struct inode *, struct file *);
static int proc_read_queue_map(struct seq_file *, void *);
static int proc_read_tc_group_seq_open(struct inode *, struct file *);
static int proc_read_tc_group(struct seq_file *, void *);

static int proc_read_dbg(struct seq_file *seq, void *v);
static ssize_t proc_write_queue_info(struct file *file, const char __user *buf, size_t count, loff_t *data);
static ssize_t proc_write_tc_map(struct file *file, const char __user *buf, size_t count, loff_t *data);
static ssize_t proc_write_queue_map(struct file *file, const char __user *buf, size_t count, loff_t *data);
static ssize_t proc_write_dbg(struct file *file, const char __user *buf, size_t count, loff_t *data);
static ssize_t proc_write_tc_group(struct file *file, const char __user *buf, size_t count, loff_t *data);
static struct proc_dir_entry *g_ppa_tmu_hal_proc_dir = NULL;
static int g_ppa_tmu_hal_proc_dir_flag = 0;

extern uint32_t high_prio_q_limit ;
extern uint32_t g_tmu_dbg ;

static int strincmp(const char *p1, const char *p2, int n)
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


int Atoi(char *str)
{
    int res = 0;  // Initialize result
    int sign = 1;  // Initialize sign as positive
    int i = 0;  // Initialize index of first digit
     
    // If number is negative, then update sign
    if (str[0] == '-')
    {
        sign = -1; 
        i++;  // Also update index of first digit
    }
     
    // Iterate through all digits and update the result
    for (; str[i] != '\0'; ++i)
        res = res*10 + str[i] - '0';
   
    // Return result with sign
    return sign*res;
}


int return_val( char *p, char *str)
{
        char *temp;
	char buf[30];
	strcpy(buf, p);
//	printk("buff=%s str=%s\n",buf,str);
        if ((temp =strstr(buf, str)) != NULL ){
                while(*temp != ' ' && *temp ) {
                        temp++;
                }
                str = ++temp;
//		printk("str=%s\n", str);
                while(*temp != ' ' && *temp ) {
                        temp++;
                }

                *temp = '\0';
//		printk("str=%s\n", str);
        }
        return Atoi(str);

}

char * return_string( char *buf, char *str)
{
	char *temp;
        if ((temp =strstr(buf, str)) != NULL ){
                while(*temp != ' ' && *temp ) {
                        temp++;
                }
                str = ++temp;
                while(*temp != ' ' && *temp ) {
                        temp++;
                }

                *temp = '\0';
	}
	return str ;
}


static struct file_operations g_proc_file_queue_info_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_queue_info_seq_open,
    .read       = seq_read,
    .write	= proc_write_queue_info,
    .llseek     = seq_lseek,
    .release    = single_release,
};

#if 1
static int proc_read_queue_info_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_queue_info, NULL);
}

static int proc_read_queue_info(struct seq_file *seq, void *v)
{
	seq_printf(seq, "echo <interface> > /proc/ppa/tmu/queue_info\n");
        return 0;
}
#endif



static ssize_t proc_write_queue_info(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	int len;
	char str[64];
	char *p ;
	struct net_device *netdev ;
	uint32_t l_tmu_dbg;

	len = min(count, (unsigned long)sizeof(str) - 1);
	len -= ppa_copy_from_user(str, buf, len);
	while ( len && str[len - 1] <= ' ' )
		len--;
	str[len] = 0;
	for ( p = str; *p && *p <= ' '; p++, len-- );
	if ( !*p )
		return count;

//	printk("<%s> data= %s\n", __FUNCTION__,p);

	if ( strstr(p, "help"))
        {
                printk("echo <interface>  > /proc/ppa/tmu/queue_info\n");
                return count;
        }

	if ( (netdev = dev_get_by_name(&init_net, p)) == NULL ) {
                printk("Invalid interface name\n");
                return PPA_FAILURE;
        }
	l_tmu_dbg = g_tmu_dbg;
	g_tmu_dbg = TMU_DEBUG_TRACE; 
	tmu_hal_get_queue_info(netdev);
	g_tmu_dbg = l_tmu_dbg;
	return count;
}
//#endif
static int proc_read_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_dbg, NULL);
}


static struct file_operations g_proc_file_dbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_dbg_seq_open,
    .read       = seq_read,
    .write	= proc_write_dbg,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_dbg(struct seq_file *seq, void *v)
{
    int i;
	seq_printf(seq, "\t====================================================\n");
	seq_printf(seq,"\t|     CMD        |    Discription    |     Status    |\n");
	seq_printf(seq,"\t----------------------------------------------------\n");
	for(i=0;  i < NUM_ENTITY(dbg_enable_mask_str) -1; i++) {
		seq_printf(seq,"\t|%9s       |",dbg_enable_mask_str[i].cmd);
		seq_printf(seq,"%9s       |",dbg_enable_mask_str[i].des);
		seq_printf(seq,"%9s       |\n",(g_tmu_dbg & dbg_enable_mask_str[i].flag)  ? "enabled" : "disabled");
		seq_printf(seq,"\t---------------------------------------------------- \n");
	}
#if 0	
    for ( i = 0; i < NUM_ENTITY(dbg_enable_mask_str) -1; i++ )  //skip -1
    {
        seq_printf(seq, "%-10s(%-40s):        %-5s\n", dbg_enable_mask_str[i].cmd, dbg_enable_mask_str[i].des, (g_tmu_dbg & dbg_enable_mask_str[i].flag)  ? "enabled" : "disabled");
    }
#endif
    return 0;
}


static ssize_t proc_write_dbg(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    int len;
    char str[64];
    char *p;

    int f_enable = 0;
    int i;
//	printk(" str=%s\n", str);
    len = min(count, (size_t)(sizeof(str) - 1));
    len -= ppa_copy_from_user(str, buf, len);
    while ( len && str[len - 1] <= ' ' )
        len--;
    str[len] = 0;
    for ( p = str; *p && *p <= ' '; p++, len-- );
    if ( !*p )
        return count;
//	printk("p=%s\n", p);
    if ( strincmp(p, "enable", 6) == 0 )
    {
//	printk("inside enable\n");
        p += 6 + 1;  //skip enable and one blank
        len -= 6 + 1;  //len maybe negative now if there is no other parameters
        f_enable = 1;
    }
    else if ( strincmp(p, "disable", 7) == 0 )
    {
//	printk("inside disable\n");
        p += 7 + 1;  //skip disable and one blank
        len -= 7 + 1; //len maybe negative now if there is no other parameters
        f_enable = -1;
    }
    else if ( strincmp(p, "help", 4) == 0 || *p == '?' )
    {
         printk("echo <enable/disable> [");
         for ( i = 0; i < NUM_ENTITY(dbg_enable_mask_str); i++ ) printk("%s/", dbg_enable_mask_str[i].cmd );
         printk("] > /proc/ppa/tmu/dbg\n");
    }
//	printk("p=%s\n", p);
    if ( f_enable )
    {
        if ( (len <= 0) || ( p[0] >= '0' && p[1] <= '9') )
        {
            if ( f_enable > 0 )
                g_tmu_dbg |= TMU_ENABLE_ALL_DEBUG;
            else
                g_tmu_dbg &= ~TMU_ENABLE_ALL_DEBUG;
        }
        else
        {
            do
            {
                for ( i = 0; i < NUM_ENTITY(dbg_enable_mask_str); i++ )
                    if ( strincmp(p, dbg_enable_mask_str[i].cmd, strlen(dbg_enable_mask_str[i].cmd) ) == 0 )
                    {
                        if ( f_enable > 0 ) {
			    int32_t j = 0;
#if 1
			    do {
				g_tmu_dbg |= 1 << j ;
			    } while(!(dbg_enable_mask_str[i].flag & 1 << j++) ); 
#endif
                        //    g_tmu_dbg |= dbg_enable_mask_str[i].flag;
                        } else {
                            g_tmu_dbg &= ~dbg_enable_mask_str[i].flag;
			}
                        p += strlen(dbg_enable_mask_str[i].cmd) + 1; //skip one blank
                        len -= strlen(dbg_enable_mask_str[i].cmd) + 1; //skip one blank. len maybe negative now if there is no other parameters

                        break;
                    }
            } while ( i < NUM_ENTITY(dbg_enable_mask_str) );
        }
    }
//	printk("test...\n");
	TMU_HAL_DEBUG_MSG(TMU_DEBUG_ERR,"TMU Err Debug is enable\n");
	TMU_HAL_DEBUG_MSG(TMU_DEBUG_HIGH,"TMU HIGH Debug is enable\n");
	TMU_HAL_DEBUG_MSG(TMU_DEBUG_TRACE,"TMU TRACE Debug is enable\n");

	return count;
}

static struct file_operations g_proc_file_tc_map_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_tc_map_seq_open,
    .read       = seq_read,
    .write	= proc_write_tc_map,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static int proc_read_tc_map_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_tc_map, NULL);
}

static int proc_read_tc_map(struct seq_file *seq, void *v)
{
	seq_printf(seq,"echo <interface> > /proc/ppa/tmu/tc_map\n");
        return 0;
}

static ssize_t proc_write_tc_map(struct file *file, const char __user *buf, size_t count, loff_t *data)
{

	uint32_t len;
	char str[50];
	char *p ;
	struct net_device *netdev ;
	uint32_t l_tmu_dbg;

	len = min(count, (unsigned long)sizeof(str) - 1);
	len -= ppa_copy_from_user(str, buf, len);
	while ( len && str[len - 1] <= ' ' )
		len--;
	str[len] = 0;
	for ( p = str; *p && *p <= ' '; p++, len-- );
	if ( !*p )
		return count;

//	printk("<%s> data= %s\n", __FUNCTION__,p);

	if ( strstr(p, "help"))
        {
                printk("echo <interface> > /proc/ppa/tmu/tc_map\n");
                return count;

        } else {
		if ( (netdev = dev_get_by_name(&init_net, p)) == NULL ) {
        	        printk("Invalid interface name\n");
                	return PPA_FAILURE;
        	}
//		printk("**********NEW******name=%s index=%d mtu=%u\n", netdev->name, netdev->ifindex, netdev->mtu);
		
		l_tmu_dbg = g_tmu_dbg;
		g_tmu_dbg = TMU_DEBUG_TRACE; 
		tmu_hal_get_queue_map(netdev);
		g_tmu_dbg = l_tmu_dbg;
		return count;
	}

}

static struct file_operations g_proc_file_queue_map_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_queue_map_seq_open,
    .read       = seq_read,
    .write	= proc_write_queue_map,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static int proc_read_queue_map_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_queue_map, NULL);
}

static int proc_read_queue_map(struct seq_file *seq, void *v)
{
	seq_printf(seq,"echo <QId> > /proc/ppa/tmu/queue_map\n");
        return 0;
}

static ssize_t proc_write_queue_map(struct file *file, const char __user *buf, size_t count, loff_t *data)
{

	uint32_t len;
	uint32_t l_tmu_dbg;
	char str[50];
	char *p ;
	int32_t queue_index;
	len = min(count, (unsigned long)sizeof(str) - 1);
	len -= ppa_copy_from_user(str, buf, len);
	while ( len && str[len - 1] <= ' ' )
		len--;
	str[len] = 0;
	for ( p = str; *p && *p <= ' '; p++, len-- );
	if ( !*p )
		return count;

//	printk("<%s> data= %s\n", __FUNCTION__,p);

	if ( strstr(p, "help"))
        {
                printk("echo <QId> > /proc/ppa/tmu/queue_map\n");
                return count;

        } else {
		queue_index = Atoi(p);		
		l_tmu_dbg = g_tmu_dbg;
		g_tmu_dbg = TMU_DEBUG_TRACE; 
		tmu_hal_get_detailed_queue_map(queue_index);
		g_tmu_dbg = l_tmu_dbg;
		return count;
	}

}

static struct file_operations g_proc_file_tc_group_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_tc_group_seq_open,
    .read       = seq_read,
    .write	= proc_write_tc_group,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static int proc_read_tc_group_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_tc_group, NULL);
}

static int proc_read_tc_group(struct seq_file *seq, void *v)
{
	seq_printf(seq,"echo <tc group split Idx> > /proc/ppa/tmu/tc_group\n");
        return 0;
}

static ssize_t proc_write_tc_group(struct file *file, const char __user *buf, size_t count, loff_t *data)
{

	uint32_t len;
	uint32_t l_tmu_dbg;
	char str[50];
	char *p ;
	int32_t queue_index;

	len = min(count, (unsigned long)sizeof(str) - 1);
	len -= ppa_copy_from_user(str, buf, len);
	while ( len && str[len - 1] <= ' ' )
		len--;
	str[len] = 0;
	for ( p = str; *p && *p <= ' '; p++, len-- );
	if ( !*p )
		return count;

//	printk("<%s> data= %s\n", __FUNCTION__,p);

	if ( strstr(p, "help"))
        {
                printk("echo <tc group split Idx> > /proc/ppa/tmu/tc_group\n");
                return count;

        } else {
		high_prio_q_limit = Atoi(p);
		printk( "high_prio_q_limit = %d\n", high_prio_q_limit);		
		return count;
	}

}


int tmu_hal_proc_create(void)
{
	int32_t res;
	TMU_HAL_DEBUG_MSG(TMU_DEBUG_ERR,"TMU HAL Init.\n");
	g_ppa_tmu_hal_proc_dir = proc_mkdir("tmu", g_ppa_proc_dir);
	g_ppa_tmu_hal_proc_dir_flag = 1 ;

	res = proc_create("queue_info",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_queue_info_seq_fops);
	res = proc_create("dbg",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_dbg_seq_fops);
	res = proc_create("tc_map",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_tc_map_seq_fops);
	res = proc_create("queue_map",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_queue_map_seq_fops);
	res = proc_create("tc_group",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_tc_group_seq_fops);

	return 0;

}

void tmu_hal_proc_destroy(void)
{
	remove_proc_entry("queue_info",
                      g_ppa_tmu_hal_proc_dir);

	remove_proc_entry("dbg",
                      g_ppa_tmu_hal_proc_dir);
	
	remove_proc_entry("tc_map",
                      g_ppa_tmu_hal_proc_dir);

	remove_proc_entry("queue_map",
                      g_ppa_tmu_hal_proc_dir);

	remove_proc_entry("tc_group",
                      g_ppa_tmu_hal_proc_dir);

	if ( g_ppa_tmu_hal_proc_dir_flag )
    	{
	        remove_proc_entry("tmu",
                          g_ppa_proc_dir);
       		g_ppa_tmu_hal_proc_dir = NULL;
        	g_ppa_tmu_hal_proc_dir_flag = 0;
    	}

}

static int __init tmu_hal_proc_init(void)
{
#if 0
	int32_t res;
	TMU_HAL_DEBUG_MSG(TMU_DEBUG_ERR,"TMU HAL Init.\n");
	g_ppa_tmu_hal_proc_dir = proc_mkdir("tmu_hal", g_ppa_proc_dir);
	g_ppa_tmu_hal_proc_dir_flag = 1 ;

	res = proc_create("queue_info",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_queue_info_seq_fops);
	res = proc_create("dbg",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_dbg_seq_fops);
	res = proc_create("tc_map",
                            S_IRUGO|S_IWUSR,
                            g_ppa_tmu_hal_proc_dir,
                            &g_proc_file_tc_map_seq_fops);


	return 0;
#endif
}

static void __exit tmu_hal_proc_exit(void)
{

	tmu_hal_proc_destroy();
#if 0
	remove_proc_entry("queue_info",
                      g_ppa_tmu_hal_proc_dir);

	remove_proc_entry("dbg",
                      g_ppa_tmu_hal_proc_dir);
	
	remove_proc_entry("tc_map",
                      g_ppa_tmu_hal_proc_dir);
	if ( g_ppa_tmu_hal_proc_dir_flag )
    	{
	        remove_proc_entry("tmu_hal",
                          g_ppa_proc_dir);
       		g_ppa_tmu_hal_proc_dir = NULL;
        	g_ppa_tmu_hal_proc_dir_flag = 0;
    	}
#endif
}
#if 0
module_init(tmu_hal_proc_init);
module_exit(tmu_hal_proc_exit);
#endif
MODULE_LICENSE("GPL");


