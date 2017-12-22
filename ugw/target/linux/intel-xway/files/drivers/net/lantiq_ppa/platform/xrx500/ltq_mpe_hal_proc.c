/*******************************************************************************
 **
 ** FILE NAME    : ltq_mpe_hal.c
 ** PROJECT      : MPE HAL
 ** MODULES      : MPE (Routing/Bridging Acceleration )
 **
 ** DATE         : 20 Mar 2014
 ** AUTHOR       : Purnendu Ghosh
 ** DESCRIPTION  : MPE HAL Layer
 ** COPYRIGHT    :              Copyright (c) 2009
 **                          Lantiq Deutschland GmbH
 **                   Am Campeon 3; 85579 Neubiberg, Germany
 **
 **   For licensing information, see the file 'LICENSE' in the root folder of
 **   this software module.
 **
 ** HISTORY
 ** $Date        $Author                $Comment
 ** 20 Mar 2014  Purnendu Ghosh         Initiate Version
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
#include <linux/firmware.h>
#include <linux/platform_device.h>
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
#include <linux/proc_fs.h>
#include <linux/seq_file.h>


/*
 *  Chip Specific Head File
 */
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ltq_mpe_api.h"
#include "mpe_debug_hdr.h"
#include "ltq_mpe_hal.h"
#include "mpe_fw_be.h"
#include <asm/ltq_vmb.h>
#include <net/lantiq_cbm.h>
#include <net/lantiq_cbm_api.h>


#define PARAM_NUM 5
#define MPE_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

#define set_ltq_mpe_dbg_flag(v, e, f) do {;\
        if (e > 0)\
                v |= (uint32_t)(f);\
        else\
                v &= (uint32_t) (~f); } \
        while (0)

uint32_t mpe_dbg_flag = 0;

char *mpe_dbg_flag_str[] = {
    "enable_debug",      /* to align with MPE FW misc.h enum */
    "enable_error",
    "enable_assert",
    "dbg_tm",
    "wk_rx_data", /* DUMP_RX_DATA */
    "wk_accel", /* DBG_WK_ACCEL */
    "wk_mcast", /* DBG_WK_MCAST */
    "wk_parser", /* DBG_WK_PARSER */    
    "wk_tx_data", /* DUMP_TX_DATA */
    "wk_tx_desc", /* DUMP_TX_DESCRIPTOR */
    "wk_tx_pmac", /* DUMP_TX_PMAC */
    "dummy_4",
};



/*
* ####################################
*             Declaration
* ####################################
*/

static int proc_read_genconf_seq_open(struct inode *, struct file *);
static int proc_read_tc_full_dbg_seq_open(struct inode *, struct file *);

static int proc_read_fwHdr_seq_open(struct inode *inode, struct file *file);

static int proc_read_tc_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_tc(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_hw_res_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_hw_res(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_session_mib_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_session_mib(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_tc_mib_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_tc_mib(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_itf_mib_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_itf_mib(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_hit_mib_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_hit_mib(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_fw_dbg_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_fw_dbg(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_ipv4_sessions_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_ipv4_sessions(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_ipv6_sessions_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_ipv6_sessions(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_test_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_test(struct file *file, const char __user *buf, size_t count, loff_t *data);


static int proc_read_session_action_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_session_action(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_accel_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_accel(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_fw_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_fw(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_multicast_vap_list_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_multicast_vap_list(struct file *file, const char __user *buf, size_t count, loff_t *data);


#if CONFIG_LTQ_PPA_MPE_IP97
static int proc_read_tunnel_info_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_tunnel_info(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_xfrm_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_xfrm(struct file *file, const char __user *buf, size_t count, loff_t *data);

static int proc_read_eip97_seq_open(struct inode *inode, struct file *file);
static ssize_t proc_write_eip97(struct file *file, const char __user *buf, size_t count, loff_t *data);
#endif
static int proc_read_version_seq_open(struct inode *inode, struct file *file);

static int proc_read_session_count_seq_open(struct inode *inode, struct file *file);

//extern void mpe_hal_dump_tc_hw_res(uint32_t tc_num, struct seq_file *seq);
extern void mpe_hal_dump_fw_header(struct seq_file *seq);
extern void mpe_hal_dump_genconf_offset(struct seq_file *seq);
extern void mpe_hal_dump_tc_hw_res_all(void); 
extern int32_t mpe_hal_pause_tc(uint8_t ucCpu, uint8_t ucTc);
extern int32_t  mpe_hal_resume_tc(uint8_t ucCpu, uint8_t ucTc);
extern int32_t  mpe_hal_add_tc(uint8_t ucCpu, uint32_t tc_type);
extern int32_t  mpe_hal_delete_tc(uint8_t ucCpu, uint8_t ucTc);
extern void mpe_hal_dump_session_mib_cntr(struct seq_file *seq);
extern void mpe_hal_dump_tc_mib(struct seq_file *seq);
extern void mpe_hal_clear_tc_mib(void);
extern void mpe_hal_debug_cfg(uint32_t ucDbg);
extern void mpe_hal_dump_itf_mib_cntr(struct seq_file *seq);
extern void mpe_hal_dump_hit_mib(struct seq_file *seq);
extern void mpe_hal_clear_hit_mib(void);
extern void mpe_hal_clear_tc_mib(void);
extern void mpe_hal_clear_session_mib(void);
extern void mpe_hal_clear_itf_mib(void);
extern void mpe_hal_dump_ipv4_cmp_table_entry(struct seq_file *seq);
extern void mpe_hal_dump_ipv6_cmp_table_entry(struct seq_file *seq);
extern void mpe_hal_dump_mpe_detailed_dbg(struct seq_file *seq);
extern void mpe_hal_test(uint32_t testcase);
extern void mpe_hal_display_session_action(uint32_t tbl, uint32_t current_ptr);
extern void mpe_hal_config_accl_mode(uint32_t mode);
extern int32_t mpe_hal_fw_load(void);
extern int32_t mpe_hal_fw_unload(void);
extern void dump_mpe_version(struct seq_file *seq);
extern void mpe_session_count(struct seq_file *seq);
extern void mpe_hal_dump_vap_list(void);

#if CONFIG_LTQ_PPA_MPE_IP97
extern int32_t mpe_hal_dump_ipsec_tunnel_info(int32_t tun_id);
extern int32_t mpe_hal_dump_ipsec_xfrm_sa(uint32_t tunnel_index);
extern int32_t mpe_hal_dump_ipsec_eip97_params(int32_t tunnel_index);
#endif
extern struct proc_dir_entry *g_ppa_proc_dir ;
extern uint32_t g_MPE_accl_mode;
static struct proc_dir_entry *g_ppa_mpe_proc_dir = NULL;
static int32_t g_ppa_mpe_proc_dir_flag = 0;
static struct proc_dir_entry *g_ppa_mpe_ipsec_proc_dir = NULL;
static int32_t  g_ppa_mpe_ipsec_proc_dir_flag = 0;

void remove_leading_whitespace(char **p, int *len)
{
	while (*len && ((**p == ' ') || (**p == '\r') || (**p == '\r'))) {
		(*p)++;
		(*len)--;
	}
}


int ltq_split_buffer(char *buffer, char *array[], int max_param_num)
{
	int i, set_copy = 0;
	int res = 0;
	int len;

	for (i = 0; i < max_param_num; i++)
		array[i] = NULL;
	if (!buffer)
		return 0;
	len = strlen(buffer);
	for (i = 0; i < max_param_num;) {
		remove_leading_whitespace(&buffer, &len);
		for (;
		     *buffer != ' ' && *buffer != '\0' && *buffer != '\r'
		     && *buffer != '\n' && *buffer != '\t'; buffer++, len--) {
			/*Find first valid charactor */
			set_copy = 1;
			if (!array[i])
				array[i] = buffer;
		}

		if (set_copy == 1) {
			i++;
			if (*buffer == '\0' || *buffer == '\r'
			    || *buffer == '\n') {
				*buffer = 0;
				break;
			}
			*buffer = 0;
			buffer++;
			len--;
			set_copy = 0;

		} else {
			if (*buffer == '\0' || *buffer == '\r'
			    || *buffer == '\n')
				break;
			buffer++;
			len--;
		}
	}
	res = i;

	return res;
}


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
//      printk("buff=%s str=%s\n",buf,str);
        if ((temp =strstr(buf, str)) != NULL ){
                while(*temp != ' ' && *temp ) {
                        temp++;
                }
                str = ++temp;
//              printk("str=%s\n", str);
                while(*temp != ' ' && *temp ) {
                        temp++;
                }

                *temp = '\0';
//              printk("str=%s\n", str);
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


/* Proc function for /proc/ppa/mpe/genconf */
static struct file_operations g_proc_file_genconf_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_genconf_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_genconf(struct seq_file *seq, void *v)
{
    mpe_hal_dump_genconf_offset( seq );
    return 0;
}

static int proc_read_genconf_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_genconf, NULL);
}



/* Proc function for /proc/ppa/mpe/fwHdr */
static struct file_operations g_proc_file_fwHdr_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_fwHdr_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_fwHdr(struct seq_file *seq, void *v)
{
    mpe_hal_dump_fw_header( seq );
    return 0;
}


static int proc_read_fwHdr_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_fwHdr, NULL);
}


/* Proc function for /proc/ppa/mpe/tc */
static struct file_operations g_proc_file_tc_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_tc_seq_open,
    .read       = seq_read,
    .write	= proc_write_tc,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_tc(struct seq_file *seq, void *v)
{
    seq_printf(seq, "echo start <vpe-no> <tc-type> > /proc/ppa/mpe/tc\n");
    seq_printf(seq, "echo stop/pause/resume <vpe-no> <tc-no> > /proc/ppa/mpe/tc\n");
    return 0;
}

static ssize_t proc_write_tc(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len, num=0, vpe_num=0, tcNum=0;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;

        if ( strstr(p, "help"))
        {
    		printk( "echo start <vpe-no> <tc-type> > /proc/ppa/mpe/tc\n");
    		printk( "echo stop/pause/resume <vpe-no> <tc-no> > /proc/ppa/mpe/tc\n");
                return count;

	} else {
		num = ltq_split_buffer(p, param_list, PARAM_NUM);

		if (strincmp(param_list[0], "start", 5) == 0) {
			vpe_num = Atoi(param_list[1]);
			if(param_list[2] && !strincmp(param_list[2], "DL", 2))
				mpe_hal_add_tc(vpe_num,TYPE_DIRECTLINK);
			else
				mpe_hal_add_tc(vpe_num,TYPE_WORKER);
		}
		else if ((strincmp(param_list[0], "stop", 4) == 0) && param_list[1] && param_list[2]) {
			vpe_num = Atoi(param_list[1]);
			tcNum = Atoi(param_list[2]);
			mpe_hal_delete_tc(vpe_num,tcNum);
		}
		else if ((strincmp(param_list[0], "pause", 5) == 0) && param_list[1] && param_list[2]) {
			vpe_num = Atoi(param_list[1]);
			tcNum = Atoi(param_list[2]);
			mpe_hal_pause_tc(vpe_num, tcNum);
		}   
		else if ((strincmp(param_list[0], "resume", 6) == 0) && param_list[1] && param_list[2]) {
			vpe_num = Atoi(param_list[1]);
			tcNum = Atoi(param_list[2]);
			mpe_hal_resume_tc(vpe_num,tcNum);
		}   
		else {
			printk("Wrong Parameter : try \n");
			printk("echo help > /proc/ppa/mpe/tc\n");
			printk("echo start/stop/pause/resume cpu_num <opt: DL> <opt: TC number> > /proc/ppa/mpe/tc\n");
		}

	}

//	printk("cmd=%s, vpe=%d tc=%d\n",cmd, vpe, tc);
	return count;

}

static int proc_read_tc_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_tc, NULL);
}

/* Proc function for /proc/ppa/mpe/hw_res */
static struct file_operations g_proc_file_hw_res_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_hw_res_seq_open,
    .read       = seq_read,
    .write	= proc_write_hw_res,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_hw_res(struct seq_file *seq, void *v)
{
    seq_printf(seq, "Specific TC:\n\techo <tc-no> > /proc/ppa/mpe/hw_res\n");
    seq_printf(seq, "All TC:\n\techo -1 > /proc/ppa/mpe/hw_res\n");
    return 0;
}


static ssize_t proc_write_hw_res(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo <tc-num>/-1(for all TC) > /proc/ppa/mpe/tc\n");
                return count;

        } else {
		int32_t tc = Atoi(p);
		printk("tc= %d\n", tc);
		if (tc == -1 ) {
			mpe_hal_dump_tc_hw_res_all();			
		} else {
//			mpe_hal_dump_tc_hw_res(tc);
		}
	}
	return count ;
}

static int proc_read_hw_res_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_hw_res, NULL);
}


/* Proc function for /proc/ppa/mpe/session_mib */
static struct file_operations g_proc_file_session_mib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_session_mib_seq_open,
    .read       = seq_read,
    .write	= proc_write_session_mib,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_session_mib(struct seq_file *seq, void *v)
{
    /*    int test = 20;
    seq_printf(seq, "Specific TC:\n\techo <tc-no> > /proc/ppa/mpe/hw_res\n");
    seq_printf(seq, "All TC:\n\techo -1 > /proc/ppa/mpe/hw_res\n");*/
    mpe_hal_dump_session_mib_cntr(seq); 
    return 0;
}


static ssize_t proc_write_session_mib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo <tc-num> > /proc/ppa/mpe/session_mib\n");
                return count;

        } else {
		printk("cmd=%s\n", p);
		mpe_hal_clear_session_mib();
	}
	return count ;
}


static int proc_read_session_mib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_session_mib, NULL);
}

/* Proc function for /proc/ppa/mpe/tc_mib */
static struct file_operations g_proc_file_tc_mib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_tc_mib_seq_open,
    .read       = seq_read,
    .write	= proc_write_tc_mib,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_tc_mib(struct seq_file *seq, void *v)
{
    /*    int test = 20;
    seq_printf(seq, "Specific TC:\n\techo <tc-no> > /proc/ppa/mpe/hw_res\n");
    seq_printf(seq, "All TC:\n\techo -1 > /proc/ppa/mpe/hw_res\n");*/
    mpe_hal_dump_tc_mib(seq);
    return 0;
}


static ssize_t proc_write_tc_mib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo clear > /proc/ppa/mpe/tc_mib\n");
                return count;

        } else {
		printk("p=%s\n", p);
		mpe_hal_clear_tc_mib();	
	}
	return count ;
}


static int proc_read_tc_mib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_tc_mib, NULL);
}

/* Proc function for /proc/ppa/mpe/itf_mib */
static struct file_operations g_proc_file_itf_mib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_itf_mib_seq_open,
    .read       = seq_read,
    .write	= proc_write_itf_mib,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_itf_mib(struct seq_file *seq, void *v)
{
    mpe_hal_dump_itf_mib_cntr(seq);
    return 0;
}


static ssize_t proc_write_itf_mib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo clear > /proc/ppa/mpe/itf_mib\n");
                return count;

        } else {
		printk("cmd=%s\n", p);
		mpe_hal_clear_itf_mib();
	}
	return count ;
}


static int proc_read_itf_mib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_itf_mib, NULL);
}


/* Proc function for /proc/ppa/mpe/hit_mib */
static struct file_operations g_proc_file_hit_mib_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_hit_mib_seq_open,
    .read       = seq_read,
    .write	= proc_write_hit_mib,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_hit_mib(struct seq_file *seq, void *v)
{
    mpe_hal_dump_hit_mib(seq);
    return 0;
}


static ssize_t proc_write_hit_mib(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo clear > /proc/ppa/mpe/hit_mib\n");
                return count;

        } else {
		printk("cmd=%s\n", p);
		mpe_hal_clear_hit_mib();
	}
	return count ;
}


static int proc_read_hit_mib_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_hit_mib, NULL);
}


/* Proc function for /proc/ppa/mpe/IPv4_sessions */
static struct file_operations g_proc_file_ipv4_sessions_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_ipv4_sessions_seq_open,
    .read       = seq_read,
    .write	= proc_write_ipv4_sessions,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_ipv4_sessions(struct seq_file *seq, void *v)
{
//    mpe_hal_dump_table_hashidx_entry(void *phtable,uint32_t hashidx, 0);
      mpe_hal_dump_ipv4_cmp_table_entry(seq);
    return 0;
}


static ssize_t proc_write_ipv4_sessions(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo clear > /proc/ppa/mpe/IPv4_sessions\n");
                return count;

        } else {
		printk("p=%s\n", p);
	}
	return count ;
}


static int proc_read_ipv4_sessions_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ipv4_sessions, NULL);
}


/* Proc function for /proc/ppa/mpe/IPv6_sessions */
static struct file_operations g_proc_file_ipv6_sessions_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_ipv6_sessions_seq_open,
    .read       = seq_read,
    .write	= proc_write_ipv6_sessions,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_ipv6_sessions(struct seq_file *seq, void *v)
{
//    mpe_hal_dump_table_hashidx_entry(void *phtable,uint32_t hashidx, 1);
      mpe_hal_dump_ipv6_cmp_table_entry( seq );
    return 0;
}


static ssize_t proc_write_ipv6_sessions(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo clear > /proc/ppa/mpe/IPv6_sessions\n");
                return count;

        } else {
		printk("p=%s\n", p);
	}
	return count ;
}


static int proc_read_ipv6_sessions_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_ipv6_sessions, NULL);
}




/* Proc function for /proc/ppa/mpe/fw_dbg */
static struct file_operations g_proc_file_fw_dbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_fw_dbg_seq_open,
    .read       = seq_read,
    .write	= proc_write_fw_dbg,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_fw_dbg(struct seq_file *seq, void *v)
{
    seq_printf(seq, "echo enable/disable > /proc/ppa/mpe/fw_dbg\n");
    /*    int test = 20;
    se11q_printf(seq, "Specific TC:\n\techo <tc-no> > /proc/ppa/mpe/hw_res\n");
    seq_printf(seq, "All TC:\n\techo -1 > /proc/ppa/mpe/hw_res\n");*/
    return 0;
}

static ssize_t proc_write_fw_dbg(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
	char str[50];
	char *p;
	char *param_list[20];
	int f_enable, i, j, num;

        len = min(count, sizeof(str) - 1);
	len -= ppa_copy_from_user(str, buf, len);
	while ( len && str[len - 1] <= ' ' )
		len--;
	str[len] = 0;
	for ( p = str; *p && *p <= ' '; p++, len-- );
	if ( !*p )
		return count;


	if ( strstr(p, "help"))
	{
		printk("echo enable/disable > /proc/ppa/mpe/fw_dbg\n");
		return count;

	} else {
		printk("p=%s\n", p);

		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));

		if (strincmp(param_list[0], "enable", 6) == 0)
			f_enable = 1;
		else if (strincmp(param_list[0], "disable", 7) == 0)
			f_enable = -1;
		else {
			printk("echo <enable/disable> ");
			for (i = 0; i < MPE_ARRAY_SIZE(mpe_dbg_flag_str); i++)
				printk("%s ", mpe_dbg_flag_str[i]);
			printk(" > /proc/mpe/cfg_mpe_dbg\n");
			return count;	
		}

		if (!param_list[1]) {   /*no parameter after enable or disable: set/clear all debug flags */
			set_ltq_mpe_dbg_flag(mpe_dbg_flag, f_enable, -1);
		} else {
			for (i = 1; i < num; i++) {
				for (j = 0; j < MPE_ARRAY_SIZE(mpe_dbg_flag_str); j++)
					if (strincmp(param_list[i], mpe_dbg_flag_str[j], strlen(param_list[i])) == 0) {
						set_ltq_mpe_dbg_flag(mpe_dbg_flag,  f_enable, (1 << j));
						break;
					}
			}
		}

	}
	printk("<-------- mpe_dbg_flag = %d\n",mpe_dbg_flag);
	mpe_hal_debug_cfg(mpe_dbg_flag );
	return count ;
}


static int proc_read_fw_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_fw_dbg, NULL);
}


/* Proc function for /proc/ppa/mpe/tc_full_dbg */
static struct file_operations g_proc_file_tc_full_dbg_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_tc_full_dbg_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_tc_full_dbg(struct seq_file *seq, void *v)
{
    mpe_hal_dump_mpe_detailed_dbg(seq);
    return 0;
}

static int proc_read_tc_full_dbg_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_tc_full_dbg, NULL);
}



/* Proc function for /proc/ppa/mpe/test */
static struct file_operations g_proc_file_test_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_test_seq_open,
    .read       = seq_read,
    .write	= proc_write_test,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_test(struct seq_file *seq, void *v)
{
    seq_printf(seq, "echo <val> > /proc/ppa/mpe/test");
    return 0;
}


static ssize_t proc_write_test(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo clear > /proc/ppa/mpe/IPv6_sessions\n");
                return count;

        } else {
		int32_t val=0 ;
		val = Atoi(p);
		printk("p=%s, val=%d\n", p, val);
		mpe_hal_test(val);
	}
	return count ;
}


static int proc_read_test_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_test, NULL);
}


/* Proc function for /proc/ppa/mpe/session_action */
static struct file_operations g_proc_file_session_action_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_session_action_seq_open,
    .read       = seq_read,
    .write	= proc_write_session_action,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_session_action(struct seq_file *seq, void *v)
{
    seq_printf(seq, "echo <Tbl typ> <idx> > /proc/ppa/mpe/session_action\n");
    seq_printf(seq, "Tbl typ:\n\t1 - IPv4 Table\n\t2 - IPv6 Table\n\t3 - Hardware Action Table\n");
    return 0;
}


static ssize_t proc_write_session_action(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo <table type> <idx> > /proc/ppa/mpe/session_action\n");
                return count;

        } else {
		int32_t idx=0 , tbl=0, num=0;
		 num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		idx = Atoi(param_list[1]);
		tbl = Atoi(param_list[0]);
		printk("p=%s, idx=%d tbl=%d\n", p, idx, tbl);
		mpe_hal_display_session_action(tbl, idx);
	}
	return count ;
}


static int proc_read_session_action_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_session_action, NULL);
}


/* Proc function for /proc/ppa/mpe/accel */
static struct file_operations g_proc_file_accel_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_accel_seq_open,
    .read       = seq_read,
    .write	= proc_write_accel,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_accel(struct seq_file *seq, void *v)
{
    if (g_MPE_accl_mode) {
    	seq_printf(seq, "MPE accel : enable\n");
    } else {
    	seq_printf(seq, "MPE accel : disable\n");
    }
    return 0;
}


static ssize_t proc_write_accel(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo <enable/disable/start/stop> > /proc/ppa/mpe/accel\n");
                return count;

        } else {
		int32_t num=0, f_enable=0;
		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		if (num == 1) {
			if (strincmp(param_list[0], "enable", 6) == 0)
				f_enable = 1;
			else if (strincmp(param_list[0], "disable", 7) == 0)
				f_enable = 0;
			else if (strincmp(param_list[0], "start", 5) == 0)
				f_enable = 2;
			else if (strincmp(param_list[0], "stop", 4) == 0)
				f_enable = 3;
			else {
				printk("echo <enable/disable/start/stop> /proc/ppa/mpe/accel\n");
				return count;
			}
			mpe_hal_config_accl_mode(f_enable);

		} else {
			printk("echo help > /proc/ppa/mpe/accel\n");
		}
	}
	return count ;
}


static int proc_read_accel_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_accel, NULL);
}

/* Proc function for /proc/ppa/mpe/fw */
static struct file_operations g_proc_file_fw_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_fw_seq_open,
    .read       = seq_read,
    .write	= proc_write_fw,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_fw(struct seq_file *seq, void *v)
{
    seq_printf(seq, "echo <load/unload> > /proc/ppa/mpe/fw\n");
    return 0;
}


static ssize_t proc_write_fw(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("echo <load/unload> > /proc/ppa/mpe/fw\n");
                return count;

        } else {
		int32_t num=0 ;
		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		if (num == 1) {
			if (strincmp(param_list[0], "load", 4) == 0)
				mpe_hal_fw_load();
			else if (strincmp(param_list[0], "unload", 6) == 0)
				mpe_hal_fw_unload();
			else {
				printk("echo <load/unload> > /proc/ppa/mpe/fw\n");
				return count;
			}
		} else {
			printk("echo help > /proc/ppa/mpe/fw\n");
		}
	}
	return count ;
}


static int proc_read_fw_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_fw, NULL);
}

/* Proc function for /proc/ppa/mpe/multicast_vap_list */
static struct file_operations g_proc_file_multicast_vap_list_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_multicast_vap_list_seq_open,
    .read       = seq_read,
    .write	= proc_write_multicast_vap_list,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_multicast_vap_list(struct seq_file *seq, void *v)
{
    mpe_hal_dump_vap_list();
    seq_printf(seq, "echo clear > /proc/ppa/mpe/multicast_vap_list\n");
    return 0;
}


static ssize_t proc_write_multicast_vap_list(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("for clearing conter 'echo clear > /proc/ppa/mpe/multicast_vap_list\n'");
		printk("cat /proc/ppa/mpe/multicast_vap_list\n");
                return count;

        } else {
		int32_t num=0 ;
		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		if (num == 1) {
			if (strincmp(param_list[0], "clear", 5) == 0)
				printk("need to pass api for cleaning counter for multicast_vap_list\n");
			else {
				printk("echo clear > /proc/ppa/mpe/multicast_vap_list\n");
				return count;
			}
		} else {
			printk("echo help > /proc/ppa/mpe/multicast_vap_list\n");
		}
	}
	return count ;
}


static int proc_read_multicast_vap_list_seq_open(struct inode *inode, struct file *file)
{
    
    return single_open(file, proc_read_multicast_vap_list, NULL);
}


#if CONFIG_LTQ_PPA_MPE_IP97
/* Proc function for /proc/ppa/mpe/ipsec/tunnel_info */
static struct file_operations g_proc_file_tunnel_info_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_tunnel_info_seq_open,
    .read       = seq_read,
    .write	= proc_write_tunnel_info,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_tunnel_info(struct seq_file *seq, void *v)
{
    return 0;
}


static ssize_t proc_write_tunnel_info(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("for specific tunnel 'echo <tunnel Id> > /proc/ppa/mpe/ipsec/tunnel_info\n'");
		printk("cat /proc/ppa/mpe/ipsec/tunnel_info\n");
                return count;

        } else {
		int32_t num=0 ;
		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		if (num == 1) {
			printk("tunnel Id=%d\n", Atoi(param_list[0]));
			mpe_hal_dump_ipsec_tunnel_info(Atoi(param_list[0]));
		} else {
			printk("echo help > /proc/ppa/mpe/multicast_vap_list\n");
		}
	}
	return count ;
}


static int proc_read_tunnel_info_seq_open(struct inode *inode, struct file *file)
{
    
    return single_open(file, proc_read_tunnel_info, NULL);
}

/* Proc function for /proc/ppa/mpe/ipsec/xfrm */
static struct file_operations g_proc_file_xfrm_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_xfrm_seq_open,
    .read       = seq_read,
    .write	= proc_write_xfrm,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_xfrm(struct seq_file *seq, void *v)
{
    return 0;
}


static ssize_t proc_write_xfrm(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("for specific tunnel 'echo <tunnel Id> > /proc/ppa/mpe/ipsec/xfrm\n'");
		printk("cat /proc/ppa/mpe/ipsec/xfrm\n");
                return count;

        } else {
		int32_t num=0 ;
		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		if (num == 1) {
			printk("tunnel Id=%d\n", Atoi(param_list[0]));
			mpe_hal_dump_ipsec_xfrm_sa(Atoi(param_list[0]));
		} else {
			printk("echo help > /proc/ppa/mpe/ipsec/xfrm\n");
		}
	}
	return count ;
}


static int proc_read_xfrm_seq_open(struct inode *inode, struct file *file)
{
    
    return single_open(file, proc_read_xfrm, NULL);
}


/* Proc function for /proc/ppa/mpe/ipsec/eip97 */
static struct file_operations g_proc_file_eip97_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_eip97_seq_open,
    .read       = seq_read,
    .write	= proc_write_eip97,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_eip97(struct seq_file *seq, void *v)
{
    return 0;
}


static ssize_t proc_write_eip97(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
	uint32_t len;
        char str[50];
        char *p;
	char *param_list[PARAM_NUM] = { 0 };

        len = min(count, sizeof(str) - 1);
        len -= ppa_copy_from_user(str, buf, len);
        while ( len && str[len - 1] <= ' ' )
                len--;
        str[len] = 0;
        for ( p = str; *p && *p <= ' '; p++, len-- );
        if ( !*p )
                return count;


        if ( strstr(p, "help"))
        {
                printk("for specific tunnel 'echo <tunnel Id> > /proc/ppa/mpe/ipsec/eip97\n'");
                return count;

        } else {
		int32_t num=0 ;
		num = ltq_split_buffer(p, param_list, MPE_ARRAY_SIZE(param_list));
		if (num == 1) {
			printk("tunnel Id=%d\n", Atoi(param_list[0]));
			mpe_hal_dump_ipsec_eip97_params(Atoi(param_list[0]));
		} else {
			printk("echo help > /proc/ppa/mpe/ipsec/eip97\n");
		}
	}
	return count ;
}


static int proc_read_eip97_seq_open(struct inode *inode, struct file *file)
{
    
    return single_open(file, proc_read_eip97, NULL);
}


#endif

/* Proc function for /proc/ppa/mpe/version */
static struct file_operations g_proc_file_version_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_version_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_version(struct seq_file *seq, void *v)
{
    dump_mpe_version(seq);
    return 0;
}


static int proc_read_version_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_version, NULL);
}




/* Proc function for /proc/ppa/mpe/session_count */
static struct file_operations g_proc_file_session_count_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_session_count_seq_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = seq_release,
};


static int proc_read_session_count(struct seq_file *seq, void *v)
{
    mpe_session_count(seq);
    return 0;
}


static int proc_read_session_count_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_session_count, NULL);
}


int mpe_hal_proc_create(void)
{
        int32_t res;
        printk("MPE Proc Creation....!!!\n");
        g_ppa_mpe_proc_dir = proc_mkdir("mpe", g_ppa_proc_dir);
        g_ppa_mpe_proc_dir_flag = 1 ;
#if CONFIG_LTQ_PPA_MPE_IP97
        g_ppa_mpe_ipsec_proc_dir = proc_mkdir("ipsec", g_ppa_mpe_proc_dir);
        g_ppa_mpe_ipsec_proc_dir_flag = 1 ;
#endif
        res = (int32_t )proc_create("genconf",
                            S_IRUGO,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_genconf_seq_fops);
        res = (int32_t )proc_create("fwHdr",
                            S_IRUGO|S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_fwHdr_seq_fops);
        res = (int32_t)proc_create("tc",
                            S_IRUGO|S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_tc_seq_fops);
	res = (int32_t)proc_create("hw_res",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_hw_res_seq_fops );
	res = (int32_t)proc_create("session_mib",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_session_mib_seq_fops );
	res = (int32_t)proc_create("tc_mib",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_tc_mib_seq_fops );
	res = (int32_t)proc_create("itf_mib",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_itf_mib_seq_fops );
	res = (int32_t)proc_create("hit_mib",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_hit_mib_seq_fops );
	res = (int32_t)proc_create("IPv4_sessions",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_ipv4_sessions_seq_fops );
	res = (int32_t)proc_create("IPv6_sessions",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_ipv6_sessions_seq_fops );

	res = (int32_t)proc_create("fw_dbg",
			    S_IRUGO| S_IWUSR,
			    g_ppa_mpe_proc_dir,
			    &g_proc_file_fw_dbg_seq_fops );
        res = (int32_t)proc_create("tc_full_dbg",
                            S_IRUGO,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_tc_full_dbg_seq_fops);
        res = (int32_t)proc_create("test",
                            S_IRUGO | S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_test_seq_fops);
        res = (int32_t)proc_create("session_action",
                            S_IRUGO | S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_session_action_seq_fops);
	res = (int32_t)proc_create("accel",
                            S_IRUGO | S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_accel_seq_fops);
	res = (int32_t)proc_create("fw",
                            S_IRUGO | S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_fw_seq_fops);
	res = (int32_t)proc_create("multicast_vap_list",
                            S_IRUGO | S_IWUSR,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_multicast_vap_list_seq_fops);
	res = (int32_t)proc_create("session_count",
                            S_IRUGO,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_session_count_seq_fops);
	res = (int32_t)proc_create("version",
                            S_IRUGO,
                            g_ppa_mpe_proc_dir,
                            &g_proc_file_version_seq_fops);
#if CONFIG_LTQ_PPA_MPE_IP97
	res = (int32_t)proc_create("tunnel_info",
                            S_IRUGO,
                            g_ppa_mpe_ipsec_proc_dir,
                            &g_proc_file_tunnel_info_seq_fops);
	res = (int32_t)proc_create("xfrm",
                            S_IRUGO,
                            g_ppa_mpe_ipsec_proc_dir,
                            &g_proc_file_xfrm_seq_fops);
	res = (int32_t)proc_create("eip97",
                            S_IRUGO,
                            g_ppa_mpe_ipsec_proc_dir,
                            &g_proc_file_eip97_seq_fops);

#endif
        return 0;

}


void mpe_hal_proc_destroy(void)
{
	printk("MPE Proc Destroy...!!!\n");
        remove_proc_entry("genconf",
                      g_ppa_mpe_proc_dir);
        remove_proc_entry("fwHdr",
                      g_ppa_mpe_proc_dir);

        remove_proc_entry("tc",
                      g_ppa_mpe_proc_dir);
	remove_proc_entry("hw_res",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("session_mib",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("tc_mib",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("itf_mib",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("hit_mib",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("IPv4_sessions",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("IPv6_sessions",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("fw_dbg",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("tc_full_dbg",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("test",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("session_action",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("accel",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("fw",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("multicast_vap_list",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("session_count",
		      g_ppa_mpe_proc_dir);
	remove_proc_entry("version",
		      g_ppa_mpe_proc_dir);
        if ( g_ppa_mpe_proc_dir_flag )
        {
                remove_proc_entry("mpe",
                          g_ppa_proc_dir);
                g_ppa_mpe_proc_dir = NULL;
                g_ppa_mpe_proc_dir_flag = 0;
        }
#if CONFIG_LTQ_PPA_MPE_IP97
        if ( g_ppa_mpe_ipsec_proc_dir_flag )
        {
                remove_proc_entry("ipsec",
                          g_ppa_mpe_proc_dir);
                g_ppa_mpe_ipsec_proc_dir = NULL;
                g_ppa_mpe_ipsec_proc_dir_flag = 0;
        }
	remove_proc_entry("tunnel_info",
		      g_ppa_mpe_ipsec_proc_dir);
	remove_proc_entry("xfrm",
		      g_ppa_mpe_ipsec_proc_dir);
	remove_proc_entry("eip97",
		      g_ppa_mpe_ipsec_proc_dir);
#endif

}




