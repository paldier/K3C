#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include "ifxmips_vectoring_stub.h"


static int proc_read_vectoring_seq_open(struct inode *, struct file *);

static unsigned char g_ebr_block[4096] = {0};


static int proc_read_vectoring(struct seq_file *seq, void *data)
{

    seq_printf(seq, "mei_dsm_cb_func_hook = %p\n", mei_dsm_cb_func_hook);
    seq_printf(seq, "g_ebr_block = %p\n", g_ebr_block);

    return 0;
}

static void dump(void *buffer, unsigned int size)
{
    unsigned char *p;
    unsigned int precede;
    unsigned int i;

    precede = (unsigned int)buffer & 0x07;
    size += precede;
    p = (unsigned char *)((unsigned int)buffer & ~0x07);

    for ( i = 0; i < precede; i++ )
    {
        if ( (i & 0x07) == 0 )
            printk("%p:", &p[i]);
        if ( i >= precede )
            printk(" %02X", (unsigned int)p[i]);
        if ( (i & 0x07) == 7 )
            printk("\n");
    }
}

static int proc_write_vectoring(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char *p;
    int len;
    char local_buf[1024];

    unsigned long pkt_len;
    int ret;
    unsigned long sys_flag;
    unsigned long start, end;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p = local_buf;
    if ( strncmp(p, "send", 4) == 0 )
    {
        pkt_len = simple_strtoul(p + 5, NULL, 0);
        *(unsigned long *)g_ebr_block = pkt_len + 14;
        *(unsigned short *)&g_ebr_block[16] = (unsigned short)pkt_len;
        printk("SIZE = %lu\n", *(unsigned long *)g_ebr_block);
        if ( mei_dsm_cb_func_hook != NULL )
        {
            printk("before process...\n");
            dump(g_ebr_block, 32);
            local_irq_save(sys_flag);
            start = read_c0_count();
            ret = mei_dsm_cb_func_hook((unsigned int *)g_ebr_block);
            end = read_c0_count();
            local_irq_restore(sys_flag);
            printk("after process... ret = %d\n", ret);
            dump(g_ebr_block, 32);
            printk("total cycles: %lu\n", (end - start) << 1);
        }
    }
    else
        printk("echo send <size> > /proc/eth/vectoring\n");

    return count;
}

static struct file_operations g_proc_file_vectoring_seq_fops = {
    .owner      = THIS_MODULE,
    .open       = proc_read_vectoring_seq_open,
    .read       = seq_read,
    .write      = proc_write_vectoring,
    .llseek     = seq_lseek,
    .release    = single_release,
};

static int proc_read_vectoring_seq_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_read_vectoring, NULL);
}
static struct proc_dir_entry *g_proc_dir;
static __init void proc_file_create(void)
{
    struct proc_dir_entry *res;

//    g_proc_dir = proc_mkdir("eth", NULL);

    res = proc_create("eth/vectoring",
                            S_IRUGO|S_IWUSR,
                            g_proc_dir,
			&g_proc_file_vectoring_seq_fops);
}

static __exit void proc_file_delete(void)
{
    remove_proc_entry("vectoring",
                      g_proc_dir);

    remove_proc_entry("eth", NULL);
}



static int __init vectoring_test_init(void)
{
    unsigned char mac[] = {0xAC, 0x9A, 0x96, 0x00, 0x0C, 0x01, 0xAC, 0x9A, 0x96, 0x00, 0x0D, 0x02};
    int i;

    memcpy(g_ebr_block + 4, mac, sizeof(mac));
    for ( i = 31; i < sizeof(g_ebr_block); i++ )
        g_ebr_block[i] = (unsigned char)(i - 31);

    proc_file_create();

    return 0;
}

static void __exit vectoring_test_exit(void)
{
    proc_file_delete();
}

module_init(vectoring_test_init);
module_exit(vectoring_test_exit);
