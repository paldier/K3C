#ifndef __VRX218_COMMON_H__
#define __VRX218_COMMON_H__



#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#ifdef CONFIG_IFX_PPA_VRX218_A1
  #include <linux/atmdev.h>
#endif
#include "vrx218_fw_prereq.h"
#include <net/ppa_stack_al.h>
//#include "../../ppa_datapath.h"



/*
 * ####################################
 *           Compilation Flags
 * ####################################
 */

#define DEBUG_SKB_SWAP                          0

#define ENABLE_DEBUG                            1

#define ENABLE_ASSERT                           1

#define DEBUG_DUMP_SKB                          1

#define DEBUG_SWAP_MAC                          1

#define DEBUG_QOS                               1

#define ENABLE_PARTIAL_RESET_PPE                1

#if defined(CONFIG_DSL_MEI_CPE_DRV) && !defined(CONFIG_IFXMIPS_DSL_CPE_MEI)
  #define CONFIG_IFXMIPS_DSL_CPE_MEI            1
#endif

#ifndef NUM_ENTITY
#define NUM_ENTITY(x)                           (sizeof(x) / sizeof(*(x)))
#endif

/*
 * ####################################
 *              Definition
 * ####################################
 */

#define err(format, arg...)                     do { if ( (g_dbg_enable & DBG_ENABLE_MASK_ERR) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )

#if defined(ENABLE_DEBUG_PRINT) && ENABLE_DEBUG_PRINT
  #undef  dbg
  #define dbg(format, arg...)                   do { if ( (g_dbg_enable & DBG_ENABLE_MASK_DEBUG_PRINT) ) printk(KERN_WARNING __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #if !defined(dbg)
    #define dbg(format, arg...)
  #endif
#endif

#if defined(ENABLE_ASSERT) && ENABLE_ASSERT
  #define ASSERT(cond, format, arg...)          do { if ( (g_dbg_enable & DBG_ENABLE_MASK_ASSERT) && !(cond) ) printk(KERN_ERR __FILE__ ":%d:%s: " format "\n", __LINE__, __FUNCTION__, ##arg); } while ( 0 )
#else
  #define ASSERT(cond, format, arg...)
#endif

#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
  #define DUMP_SKB_LEN                          ~0
#endif

/*
 *  Debug Print Mask
 */
#define DBG_ENABLE_MASK_ERR                     (1 << 0)
#define DBG_ENABLE_MASK_DEBUG_PRINT             (1 << 1)
#define DBG_ENABLE_MASK_ASSERT                  (1 << 2)
#define DBG_ENABLE_MASK_DUMP_SKB_RX             (1 << 8)
#define DBG_ENABLE_MASK_DUMP_SKB_TX             (1 << 9)
#define DBG_ENABLE_MASK_DUMP_FLAG_HEADER        (1 << 10)
#define DBG_ENABLE_MASK_DUMP_INIT               (1 << 11)
#define DBG_ENABLE_MASK_DUMP_QOS                (1 << 12)
#define DBG_ENABLE_MASK_MAC_SWAP                (1 << 16)
#define DBG_ENABLE_MASK_ALL                     (DBG_ENABLE_MASK_ERR | DBG_ENABLE_MASK_DEBUG_PRINT | DBG_ENABLE_MASK_ASSERT \
                                                | DBG_ENABLE_MASK_DUMP_SKB_RX | DBG_ENABLE_MASK_DUMP_SKB_TX                 \
                                                | DBG_ENABLE_MASK_DUMP_FLAG_HEADER | DBG_ENABLE_MASK_DUMP_INIT              \
                                                | DBG_ENABLE_MASK_DUMP_QOS | DBG_ENABLE_MASK_MAC_SWAP)


/*
 * ####################################
 *            Address Mapping
 * ####################################
 */

/*
 *  VRX218 FPI Configuration Bus Register and Memory Address Mapping
 */
#define IFX_PPE                                 0x1E200000
#define PP32_DEBUG_REG_ADDR(i, x)               (IFX_PPE + (((x) + 0x000000 + (i) * 0x00010000) << 2))
#define CDM_CODE_MEMORY(i, x)                   (IFX_PPE + (((x) + 0x001000 + (i) * 0x00010000) << 2))
#define CDM_DATA_MEMORY(i, x)                   (IFX_PPE + (((x) + 0x004000 + (i) * 0x00010000) << 2))
#define SB_RAM0_ADDR(x)                         (IFX_PPE + (((x) + 0x008000) << 2))
#define SB_RAM1_ADDR(x)                         (IFX_PPE + (((x) + 0x009000) << 2))
#define SB_RAM2_ADDR(x)                         (IFX_PPE + (((x) + 0x00A000) << 2))
#define SB_RAM3_ADDR(x)                         (IFX_PPE + (((x) + 0x00B000) << 2))
#define SB_RAM4_ADDR(x)                         (IFX_PPE + (((x) + 0x00C000) << 2))
#define PPE_REG_ADDR(x)                         (IFX_PPE + (((x) + 0x00D000) << 2))
#define QSB_CONF_REG_ADDR(x)                    (IFX_PPE + (((x) + 0x00E000) << 2))
#define SB_RAM6_ADDR(x)                         (IFX_PPE + (((x) + 0x018000) << 2))

/*  DWORD-Length of Memory Blocks   */
#define PP32_DEBUG_REG_DWLEN                    0x0030
#define CDM_CODE_MEMORYn_DWLEN(n)               ((n) == 0 ? 0x1000 : 0x0800)
#define CDM_DATA_MEMORY_DWLEN                   CDM_CODE_MEMORYn_DWLEN(1)
#define SB_RAM0_DWLEN                           0x1000
#define SB_RAM1_DWLEN                           0x1000
#define SB_RAM2_DWLEN                           0x1000
#define SB_RAM3_DWLEN                           0x1000
#define SB_RAM4_DWLEN                           0x1000
#define SB_RAM6_DWLEN                           0x8000
#define QSB_CONF_REG_DWLEN                      0x0100

#define SB_BUFFER(__sb_addr)                    ((((__sb_addr) >= 0x0000) && ((__sb_addr) <= 0x1FFF)) ? PPE_REG_ADDR((__sb_addr)) :          \
                                                 (((__sb_addr) >= 0x2000) && ((__sb_addr) <= 0x2FFF)) ? SB_RAM0_ADDR((__sb_addr) - 0x2000) : \
                                                 (((__sb_addr) >= 0x3000) && ((__sb_addr) <= 0x3FFF)) ? SB_RAM1_ADDR((__sb_addr) - 0x3000) : \
                                                 (((__sb_addr) >= 0x4000) && ((__sb_addr) <= 0x4FFF)) ? SB_RAM2_ADDR((__sb_addr) - 0x4000) : \
                                                 (((__sb_addr) >= 0x5000) && ((__sb_addr) <= 0x5FFF)) ? SB_RAM3_ADDR((__sb_addr) - 0x5000) : \
                                                 (((__sb_addr) >= 0x6000) && ((__sb_addr) <= 0x6FFF)) ? SB_RAM4_ADDR((__sb_addr) - 0x6000) : \
                                                 (((__sb_addr) >= 0x7000) && ((__sb_addr) <= 0x7FFF)) ? PPE_REG_ADDR((__sb_addr) - 0x7000) : \
                                                 (((__sb_addr) >= 0x8000) && ((__sb_addr) <= 0xFFFF)) ? SB_RAM6_ADDR((__sb_addr) - 0x8000) : \
                                                 0)

#define SOC_ACCESS_VRX218_ADDR(addr, base)      ((volatile unsigned int *)KSEG1ADDR((base) + ((unsigned int)(addr) & 0x007FFFFF)))
#define SOC_ACCESS_VRX218_SB(addr, base)        SOC_ACCESS_VRX218_ADDR(SB_BUFFER(addr), base)
#define SOC_ACCESS_VRX218_CFG_SPACE(addr, base) SOC_ACCESS_VRX218_ADDR(addr, (base) | 0x00700000)

#define VRX218_ACCESS_VRX218_SB(addr)           SB_BUFFER(addr)
#define VRX218_ACCESS_SOC_ADDR(addr)            CPHYSADDR(addr) //  VRX218 use physical address to access SoC


//Maximum 64 Descriptors (0x1E1A4000 - 0x1E1A4200)
#define SOC_US_FASTPATH_DES_BASE    g_host_desc_base.us_fastpath_des_base   //  chiptest: (0x1E180000 + (0x9000 * 4))
#define SOC_US_FASTPATH_DES_NUM     32          //  max: 64
//Maximum 64 Descriptors (0x1E1A4200 - 0x1E1A4400)
#define SOC_US_CPUPATH_DES_BASE     g_host_desc_base.us_cpupath_des_base    //  chiptest: (0x1E180000 + (0x9080 * 4))
#define SOC_US_CPUPATH_DES_NUM      32          //  max: 64
//Maximum 64 Descriptors (0x1E1A4400 - 0x1E1A4600)
#define SOC_DS_DES_BASE             g_host_desc_base.ds_des_base            //  chiptest: (0x1E180000 + (0x9100 * 4))
#define SOC_DS_DES_NUM              32          //  max: 32
#define SOC_DS_OAM_DES_BASE         g_host_desc_base.ds_oam_des_base        //  chiptest: (0x1E180000 + (0x8980 * 4)
#define SOC_DS_OAM_DES_NUM          32          //  max: 32

#define VRX218_OUTBOUND_ADDR_BASE               0x20000000
#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
#define DMA_PACKET_SIZE  1632
#define PDBRAM_PKT_SIZE  1632
#else
#define DMA_PACKET_SIZE  1600
#define PDBRAM_PKT_SIZE  1600
#endif

/*
 * ####################################
 *              Data Type
 * ####################################
 */

struct flag_header {
    //  0 - 3h
    unsigned int    ipv4_rout_vld       :1;
    unsigned int    ipv4_mc_vld         :1;
    unsigned int    proc_type           :1; // 0: routing, 1: bridging
    unsigned int    res1                :1;
    unsigned int    tcpudp_err          :1; //  reserved in A4
    unsigned int    tcpudp_chk          :1; //  reserved in A4
    unsigned int    is_udp              :1;
    unsigned int    is_tcp              :1;
    unsigned int    res2                :1;
    unsigned int    ip_inner_offset     :7; //offset from the start of the Ethernet frame to the IP field(if there's more than one IP/IPv6 header, it's inner one)
    unsigned int    is_pppoes           :1; //  2h
    unsigned int    is_inner_ipv6       :1;
    unsigned int    is_inner_ipv4       :1;
    unsigned int    is_vlan             :2; //  0: nil, 1: single tag, 2: double tag, 3: reserved
    unsigned int    rout_index          :11;

    //  4 - 7h
    unsigned int    dest_list           :8;
    unsigned int    src_itf             :3; //  7h
    unsigned int    tcp_rstfin          :1; //  7h
    unsigned int    qid                 :4; //  for fast path, indicate destination priority queue, for CPU path, QID determined by Switch
    unsigned int    temp_dest_list      :8; //  only for firmware use
    unsigned int    src_dir             :1; //  0: LAN, 1: WAN
    unsigned int    acc_done            :1;
    unsigned int    res3                :2;
    unsigned int    is_outer_ipv6       :1; //if normal ipv6 packet, only is_inner_ipv6 is set
    unsigned int    is_outer_ipv4       :1;
    unsigned int    is_tunnel           :2; //0-1 reserved, 2: 6RD, 3: Ds-lite

    // 8 - 11h
    unsigned int    sppid               :3; //switch port id
    unsigned int    pkt_len             :13;//packet length
    unsigned int    pl_byteoff          :8; //bytes between flag header and fram payload
    unsigned int    mpoa_type           :2;
    unsigned int    ip_outer_offset     :6; //offset from the start of the Ethernet frame to the IP field

    // 12 - 15h
    unsigned int    tc                  :4; //switch traffic class
    unsigned int    res4                :28;
};

struct host_desc_mem {
    unsigned long   us_fastpath_des_base;
    unsigned int    us_fastpath_des_num;

    unsigned long   us_cpupath_des_base;
    unsigned int    us_cpupath_des_num;

    unsigned long   ds_des_base;
    unsigned int    ds_des_num;

    unsigned long   ds_oam_des_base;
    unsigned int    ds_oam_des_num;
};

struct fw_ver_id {//@2000
    //DWORD 0
    unsigned int    family              :4;
    unsigned int    package             :4;
    unsigned int    major               :8;
    unsigned int    middle              :8;
    unsigned int    minor               :8;

    //DWORD 1
    unsigned int    features;
};

struct psave_cfg {
    unsigned int res1                   :15;
    unsigned int start_state            :1;     //  1: start from partial PPE reset, 0: start from full PPE reset
    unsigned int res2                   :15;
    unsigned int sleep_en               :1;     //  1: enable sleep mode, 0: disable sleep mode
};

/*
struct ltq_mei_atm_showtime_info { 
    void *check_ptr; 
    void *enter_ptr; 
    void *exit_ptr; 
};
*/ 
/*
 * ####################################
 *         Variable Declaration
 * ####################################
 */
extern int g_dbg_enable;
extern int g_dump_cnt;

extern struct host_desc_mem g_host_desc_base;


/*
 * ####################################
 *         Function Declaration
 * ####################################
 */
extern int print_fw_ver(struct seq_file *, struct fw_ver_id ver);
extern int proc_buf_copy(char **pbuf, int size, off_t off, int *ppos, const char *str, int len);
extern int get_token(char **p1, char **p2, int *len, int *colon);
extern int get_number(char **, int *, int);
extern void ignore_space(char **p, int *len);
extern void dword_mem_write(void *__dst_addr, void *__src_addr, unsigned int no_of_bytes);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
extern int strncasecmp(const char *p1, const char *p2, int n);
extern int strcasecmp(const char *p1, const char *p2);
#endif

extern int ppa_callback_set(e_ltq_mei_cb_type type, void *func);
extern void* ppa_callback_get(e_ltq_mei_cb_type type);

/*
 * ####################################
 *        Exported from Host
 * ####################################
 */
//  common function
extern int vrx218_get_desc_mem_base(struct host_desc_mem *base);
extern void turn_on_dma_rx(int);
extern void turn_off_dma_rx(int);
extern void enable_vrx218_dma_rx(int enabled);
extern void enable_vrx218_dma_tx(int enabled);
extern void enable_vrx218_swap(int enabled, int is_atm, int is_bonding);
//  common function for debugging
extern void __get_skb_from_dbg_pool(struct sk_buff *, const char *, unsigned int);
#define get_skb_from_dbg_pool(skb)  __get_skb_from_dbg_pool(skb, __FUNCTION__, __LINE__)
extern struct sk_buff *__get_skb_pointer(unsigned int, const char *, unsigned int);
#define get_skb_pointer(dataptr)    __get_skb_pointer(dataptr, __FUNCTION__, __LINE__)
extern void __put_skb_to_dbg_pool(struct sk_buff *, const char *, unsigned int);
#define put_skb_to_dbg_pool(skb)    __put_skb_to_dbg_pool(skb, __FUNCTION__, __LINE__)
//  common variable
extern int g_smartphy_port_num;
extern int (*g_smartphy_push_fn)(struct sk_buff *, struct flag_header *, unsigned int);
extern void xet_phy_wan_port(uint32_t port, uint32_t *flags, uint32_t wanmode, int xmode);


#endif  //  __VRX218_COMMON_H__
