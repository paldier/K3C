#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/skbuff.h>
#include <linux/atmdev.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <net/xfrm.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pcie.h>
#include <asm/ifx/ifx_led.h>
#else
//#include <ltq_regs.h>
//#include <common_routines.h>
//#include <ltq_pmu.h>
//#include <asm/mach-ltqcpe/ltq_pcie.h>
//#include <asm/mach-ltqcpe/ltq_ledc.h>
#include <lantiq_dma.h>
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>
#include <lantiq_pcie.h>

#endif



#include <net/ppa_api.h>
#include <net/ppa_stack_al.h>
#include "../../ppa_datapath.h"

#include "vrx218_common.h"
#include "vrx218_a1plus_addr_def.h"
#include "vrx218_ppe_atm_ds.h"
#include "unified_qos_ds_be.h"
#include "vrx218_atm_common.h"


static int qsb_tau   = 1;                      	/*  QSB cell delay variation due to concurrency     */
static int qsb_srvm  = 0x0F;                    /*  QSB scheduler burst length                      */
static int qsb_tstep = 4 ;                      /*  QSB time step, all legal values are 1, 2, 4     */
static int atm_qos;				/*  ATM QoS: Disable: 0/ Enable: 1  */


#if defined(CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT)
static int aal5r_max_packet_size   = 0x0686;    /*  Max frame size for RX                           */
static int aal5r_min_packet_size   = 0x0000;    /*  Min frame size for RX                           */
static int aal5s_max_packet_size   = 0x0686;    /*  Max frame size for TX                           */
#else
static int aal5r_max_packet_size   = 0x0630;    /*  Max frame size for RX                           */
static int aal5r_min_packet_size   = 0x0000;    /*  Min frame size for RX                           */
static int aal5s_max_packet_size   = 0x0630;    /*  Max frame size for TX                           */
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
  #define MODULE_PARM_ARRAY(a, b)   module_param_array(a, int, NULL, 0)
  #define MODULE_PARM(a, b)         module_param(a, int, 0)
#else
  #define MODULE_PARM_ARRAY(a, b)   MODULE_PARM(a, b)
#endif

MODULE_PARM(qsb_tau,"i");
MODULE_PARM_DESC(qsb_tau, "Cell delay variation. Value must be > 0");
MODULE_PARM(qsb_srvm, "i");
MODULE_PARM_DESC(qsb_srvm, "Maximum burst size");
MODULE_PARM(qsb_tstep, "i");
MODULE_PARM_DESC(qsb_tstep, "n*32 cycles per sbs cycles n=1,2,4");

MODULE_PARM(aal5r_max_packet_size, "i");
MODULE_PARM_DESC(aal5r_max_packet_size, "Max packet size in byte for downstream AAL5 frames");
MODULE_PARM(aal5r_min_packet_size, "i");
MODULE_PARM_DESC(aal5r_min_packet_size, "Min packet size in byte for downstream AAL5 frames");
MODULE_PARM(aal5s_max_packet_size, "i");
MODULE_PARM_DESC(aal5s_max_packet_size, "Max packet size in byte for upstream AAL5 frames");
module_param(atm_qos, int, 0444);
MODULE_PARM_DESC(atm_qos, "ATM QoS Enable/Disable. if Enable, only support up to 8 PVCs.");



#define ENABLE_CONFIGURABLE_DSL_VLAN            1
#define ENABLE_STATS_ON_VCC_BASIS               1

#if defined(ENABLE_STATS_ON_VCC_BASIS) && ENABLE_STATS_ON_VCC_BASIS
  #define UPDATE_VCC_STAT(conn, field, num)     do { g_atm_priv_data.connection[conn].field += (num); } while (0)
#else
  #define UPDATE_VCC_STAT(conn, field, num)
#endif


#define CPU_TO_WAN_TX_DESC_BASE                 ((volatile struct tx_descriptor *)KSEG1ADDR(SOC_US_CPUPATH_DES_BASE))
#define CPU_TO_WAN_TX_DESC_NUM                  SOC_US_CPUPATH_DES_NUM

#define WRX_QUEUE_CONFIG(i)                     ((volatile wrx_queue_config_t *)SOC_ACCESS_VRX218_SB(__WRX_QUEUE_CONFIG + (i) * 10, g_atm_priv_data.vrx218_dev.phy_membase)) /* i < 16 */
#define WTX_QUEUE_CONFIG(i)                     ((volatile wtx_queue_config_t *)SOC_ACCESS_VRX218_SB(__WTX_QUEUE_CONFIG + (i) * 25, g_atm_priv_data.vrx218_dev.phy_membase)) /* i < 16 */

#define HTU_ENTRY(i)                            ((volatile struct htu_entry *)  SOC_ACCESS_VRX218_SB(__HTU_ENTRY_TABLE + (i),  g_atm_priv_data.vrx218_dev.phy_membase)) /*  i < 24  */
#define HTU_MASK(i)                             ((volatile struct htu_mask *)   SOC_ACCESS_VRX218_SB(__HTU_MASK_TABLE + (i),   g_atm_priv_data.vrx218_dev.phy_membase)) /*  i < 24  */
#define HTU_RESULT(i)                           ((volatile struct htu_result *) SOC_ACCESS_VRX218_SB(__HTU_RESULT_TABLE + (i), g_atm_priv_data.vrx218_dev.phy_membase)) /*  i < 24  */

#define DSL_WAN_MIB_TABLE                       ((volatile struct dsl_wan_mib_table *)SOC_ACCESS_VRX218_SB(0x4EF0, g_atm_priv_data.vrx218_dev.phy_membase))

/*
 *  Mailbox IGU0 Registers
 */
#define MBOX_IGU0_ISRS                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0200), g_atm_priv_data.vrx218_dev.phy_membase)
#define MBOX_IGU0_ISRC                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0201), g_atm_priv_data.vrx218_dev.phy_membase)
#define MBOX_IGU0_ISR                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0202), g_atm_priv_data.vrx218_dev.phy_membase)
#define MBOX_IGU0_IER                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0203), g_atm_priv_data.vrx218_dev.phy_membase)

/*
 *  Mailbox IGU1 Registers
 */
#define MBOX_IGU1_ISRS                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0204), g_atm_priv_data.vrx218_dev.phy_membase)
#define MBOX_IGU1_ISRC                          SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0205), g_atm_priv_data.vrx218_dev.phy_membase)
#define MBOX_IGU1_ISR                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0206), g_atm_priv_data.vrx218_dev.phy_membase)
#define MBOX_IGU1_IER                           SOC_ACCESS_VRX218_ADDR(PPE_REG_ADDR(0x0207), g_atm_priv_data.vrx218_dev.phy_membase)

#define QSB_CONF_REG(addr)                      SOC_ACCESS_VRX218_ADDR(QSB_CONF_REG_ADDR(addr), g_atm_priv_data.vrx218_dev.phy_membase)

/*
 *  QSB Internal Cell Delay Variation Register
 */
#define QSB_ICDV                                QSB_CONF_REG(0x0007)

#define QSB_ICDV_TAU                            GET_BITS(*QSB_ICDV, 5, 0)

#define QSB_ICDV_TAU_SET(value)                 SET_BITS(0, 5, 0, value)

/*
 *  QSB Scheduler Burst Limit Register
 */
#define QSB_SBL                                 QSB_CONF_REG(0x0009)

#define QSB_SBL_SBL                             GET_BITS(*QSB_SBL, 3, 0)

#define QSB_SBL_SBL_SET(value)                  SET_BITS(0, 3, 0, value)

/*
 *  QSB Configuration Register
 */
#define QSB_CFG                                 QSB_CONF_REG(0x000A)

#define QSB_CFG_TSTEPC                          GET_BITS(*QSB_CFG, 1, 0)

#define QSB_CFG_TSTEPC_SET(value)               SET_BITS(0, 1, 0, value)

/*
 *  QSB RAM Transfer Table Register
 */
#define QSB_RTM                                 QSB_CONF_REG(0x000B)

#define QSB_RTM_DM                              (*QSB_RTM)

#define QSB_RTM_DM_SET(value)                   ((value) & 0xFFFFFFFF)

/*
 *  QSB RAM Transfer Data Register
 */
#define QSB_RTD                                 QSB_CONF_REG(0x000C)

#define QSB_RTD_TTV                             (*QSB_RTD)

#define QSB_RTD_TTV_SET(value)                  ((value) & 0xFFFFFFFF)

/*
 *  QSB RAM Access Register
 */
#define QSB_RAMAC                               QSB_CONF_REG(0x000D)

#define QSB_RAMAC_RW                            (*QSB_RAMAC & (1 << 31))
#define QSB_RAMAC_TSEL                          GET_BITS(*QSB_RAMAC, 27, 24)
#define QSB_RAMAC_LH                            (*QSB_RAMAC & (1 << 16))
#define QSB_RAMAC_TESEL                         GET_BITS(*QSB_RAMAC, 9, 0)

#define QSB_RAMAC_RW_SET(value)                 ((value) ? (1 << 31) : 0)
#define QSB_RAMAC_TSEL_SET(value)               SET_BITS(0, 27, 24, value)
#define QSB_RAMAC_LH_SET(value)                 ((value) ? (1 << 16) : 0)
#define QSB_RAMAC_TESEL_SET(value)              SET_BITS(0, 9, 0, value)

/*
 *  CGU Register
 */
#define CGU_CLKFSR                              SOC_ACCESS_VRX218_ADDR(0x1E003010, g_atm_priv_data.vrx218_dev.phy_membase)

/*
 *  Constant Definition
 */
#define DMA_PACKET_SIZE                         1600    //  ((1518 + 8 <2 VLAN> + 62 <PPE FW> + 8 <SW Header>) + 31) & ~31
#define DMA_ALIGNMENT                           32

/*
 *  ATM Info
 */
#define CELL_SIZE                               ATM_AAL0_SDU

#define ATM_PORT_NUMBER                         2
#define ATM_PVC_NUMBER                          15
#define ATM_SW_TX_QUEUE_NUMBER                  16
#define QSB_QUEUE_NUMBER_BASE                   1

#define DEFAULT_RX_HUNT_BITTH                   4

/*
 *  QSB Queue Scheduling and Shaping Definitions
 */
#define QSB_WFQ_NONUBR_MAX                      0x3f00
#define QSB_WFQ_UBR_BYPASS                      0x3fff
#define QSB_TP_TS_MAX                           65472
#define QSB_TAUS_MAX                            64512
#define QSB_GCR_MIN                             18

/*
 *  QSB Command Set
 */
#define QSB_RAMAC_RW_READ                       0
#define QSB_RAMAC_RW_WRITE                      1

#define QSB_RAMAC_TSEL_QPT                      0x01
#define QSB_RAMAC_TSEL_SCT                      0x02
#define QSB_RAMAC_TSEL_SPT                      0x03
#define QSB_RAMAC_TSEL_VBR                      0x08

#define QSB_RAMAC_LH_LOW                        0
#define QSB_RAMAC_LH_HIGH                       1

#define QSB_QPT_SET_MASK                        0x0
#define QSB_QVPT_SET_MASK                       0x0
#define QSB_SET_SCT_MASK                        0x0
#define QSB_SET_SPT_MASK                        0x0
#define QSB_SET_SPT_SBVALID_MASK                0x7FFFFFFF

#define QSB_SPT_SBV_VALID                       (1 << 31)
#define QSB_SPT_PN_SET(value)                   (((value) & 0x01) ? (1 << 16) : 0)
#define QSB_SPT_INTRATE_SET(value)              SET_BITS(0, 13, 0, value)

/*
 *  OAM Definitions
 */
#define OAM_HTU_ENTRY_NUMBER                    3
#define OAM_F4_SEG_HTU_ENTRY                    0
#define OAM_F4_TOT_HTU_ENTRY                    1
#define OAM_F5_HTU_ENTRY                        2
#define OAM_F4_CELL_ID                          0
#define OAM_F5_CELL_ID                          15


struct uni_cell_header {
    unsigned int    gfc :4;
    unsigned int    vpi :8;
    unsigned int    vci :16;
    unsigned int    pti :3;
    unsigned int    clp :1;
};

struct atm_port {
    struct atm_dev                 *dev;

    unsigned int                    tx_max_cell_rate;       /*  maximum cell rate                       */
    unsigned int                    tx_used_cell_rate;      /*  currently used cell rate                */
};

struct atm_pvc {
    struct atm_vcc                 *vcc;                    /*  opened VCC                              */
    struct timespec                 access_time;            /*  time when last F4/F5 user cell arrives  */

    int                             prio_queue_map[8];
    unsigned int                    prio_tx_packets[8];

    unsigned int                    rx_packets;
    unsigned int                    rx_bytes;
    unsigned int                    rx_errors;
    unsigned int                    rx_dropped;
    unsigned int                    tx_packets;
    unsigned int                    tx_bytes;
    unsigned int                    tx_errors;
    unsigned int                    tx_dropped;

    unsigned int                    port;                   /*  to which port the connection belongs    */
    unsigned int                    sw_tx_queue_table;      /*  software TX queues used for this        */
                                                            /*  connection                              */
};

struct atm_priv_data {
    ifx_pcie_ep_dev_t               vrx218_dev;             /*  end point device                        */

    struct atm_port                 port[ATM_PORT_NUMBER];
    struct atm_pvc                  connection[ATM_PVC_NUMBER];
    unsigned int                    pvc_table;              /*  PVC opened status, every bit stands for */
                                                            /*  one connection as well as the QSB queue */
                                                            /*  used by this PVC                        */
    unsigned int                    sw_tx_queue_table;      /*  software TX queue occupations status    */

//    ppe_u64_t                       wrx_total_byte;         /*  bit-64 extention of MIB table member    */
//    ppe_u64_t                       wtx_total_byte;         /*  bit-64 extention of MIB talbe member    */

    unsigned int                    wrx_pdu;                /*  successfully received AAL5 packet       */
    unsigned int                    wrx_drop_pdu;           /*  AAL5 packet dropped by driver on RX     */
    unsigned int                    wtx_pdu;
    unsigned int                    wtx_err_pdu;            /*  error AAL5 packet                       */
    unsigned int                    wtx_drop_pdu;           /*  AAL5 packet dropped by driver on TX     */

//    struct dsl_wan_mib_table        prev_mib;
};

struct rx_descriptor {
    //  0 - 3h
    unsigned int    own         :1;
    unsigned int    c           :1;
    unsigned int    sop         :1;
    unsigned int    eop         :1;
    unsigned int    res0        :3;
    unsigned int    byte_off    :2;
    unsigned int    qid         :2;
    unsigned int    res1        :5;
    unsigned int    data_len    :16;
    //  4 - 7h
    unsigned int    res2        :3;
    unsigned int    dataptr     :29;
};

struct tx_descriptor {
    //  0 - 3h
    unsigned int    own                 :1; //  0: MIPS, 1: PPE
    unsigned int    c                   :1;
    unsigned int    sop                 :1;
    unsigned int    eop                 :1;
    unsigned int    dic                 :1;
    unsigned int    pdu_type            :1; //  0: AAL5, 1: Non-AAL5 cell
    unsigned int    byteoff             :3;
    unsigned int    qid                 :4;
    unsigned int    mpoa_pt             :1; //  0: MPoA is determinated in FW, 1: MPoA is transparent to FW.
    unsigned int    mpoa_type           :2; //  0: EoA without FCS, 1: reserved, 2: PPPoA, 3: IPoA
    unsigned int    datalen             :16;
    //  4 - 7h
    unsigned int    dataptr             :32;
};


/*
 *  QSB Queue Parameter Table Entry and Queue VBR Parameter Table Entry
 */
union qsb_queue_parameter_table {
    struct {
      unsigned int  res1    :1;
      unsigned int  vbr     :1;
      unsigned int  wfqf    :14;
      unsigned int  tp      :16;
    }             bit;
    unsigned int  dword;
};

union qsb_queue_vbr_parameter_table {
    struct {
      unsigned int  taus    :16;
      unsigned int  ts      :16;
    }             bit;
    unsigned int  dword;
};


/*
 *  ATM Operations
 */
static int ppe_ioctl(struct atm_dev *, unsigned int, void *);
static int ppe_open(struct atm_vcc *);
static void ppe_close(struct atm_vcc *);
static int ppe_send(struct atm_vcc *, struct sk_buff *);
static int ppe_send_oam(struct atm_vcc *, void *, int);
static int ppe_change_qos(struct atm_vcc *, struct atm_qos *, int);

/*
 *  RX data handling
 */
static int atm_push(struct sk_buff *skb, struct flag_header *, unsigned int ifid);
static void do_oam_tasklet(unsigned long);

/*
 *  ATM Upper Layer Hook Function
 */
static void mpoa_setup(struct atm_vcc *, int, int);

/* DSL showtime function */
/* static int in_showtime(void); */


/*
 *  DSL led flash function
 */
#ifdef CONFIG_IFX_LED
  static void dsl_led_flash(void);
  static void dsl_led_polling(unsigned long);
#endif

/*
 *  DSL PVC Multiple Queue/Priority Operation
 */
static int sw_tx_queue_add(int);
static int sw_tx_queue_del(int);
static int sw_tx_prio_to_queue(int, int, int, int *);

/*
 *  QSB & HTU setting functions
 */
static void set_qsb(struct atm_vcc *, struct atm_qos *, unsigned int);
static void set_htu_entry(unsigned int, unsigned int, unsigned int, int);
static void clear_htu_entry(unsigned int);
static unsigned int get_qsb_clk(void);

/*
 *  QSB & HTU init functions
 */
static void qsb_global_set(void);
static void setup_oam_htu_entry(void);
static void validate_oam_htu_entry(void);
static void invalidate_oam_htu_entry(void);

/*
 *  look up for connection ID
 */
static int find_vpi(unsigned int);
static int find_vpivci(unsigned int, unsigned int);
static int find_vcc(struct atm_vcc *);

/*
 *  Buffer manage functions
 */
#if 0
static struct sk_buff *alloc_skb_rx(void);
#endif
static struct sk_buff *alloc_skb_tx_aligned(struct sk_buff *, int);
static struct sk_buff *alloc_skb_tx(int);
static struct sk_buff* skb_break_away_from_protocol(struct sk_buff *);
static struct sk_buff* atm_alloc_tx(struct atm_vcc *, unsigned int);
static void atm_free_tx_skb_vcc(struct sk_buff *skb);

static irqreturn_t mailbox_irq_handler(int irq, void *dev_id);

/*
 *  Debug functions
 */
static void dump_skb(struct sk_buff *, unsigned int, char *, int, int, int, int);
static int swap_mac(unsigned char *);

/*
 *  External Functions
 */
#if defined(CONFIG_LTQ_OAM) || defined(CONFIG_LTQ_OAM_MODULE)
  extern void ifx_push_oam(struct atm_vcc *atmvcc, unsigned char *);
#else
  static inline void ifx_push_oam(unsigned char *cell)
  {
    unsigned long sys_flag;
    int i;

    local_irq_save(sys_flag);
    printk("ifx_push_oam\n");
    for ( i = 0; i < CELL_SIZE; i++ ) {
        if ( i % 8 == 0 )
            printk(i == 0 ? "  cell:" : "\n       ");
        printk(" %02x", (unsigned int)cell[i]);
    }
    if ( i % 8 != 0 )
        printk("\n");
    local_irq_restore(sys_flag);
  }
#endif



static int g_stop_datapath = 0;

static DECLARE_TASKLET(g_oam_tasklet, do_oam_tasklet, 0);
static int g_oam_desc_pos = 0;

static DEFINE_SPINLOCK(g_mailbox_lock);

static int g_cpu_to_wan_tx_desc_pos = 0;
static DEFINE_SPINLOCK(g_cpu_to_wan_tx_desc_lock);

static struct atm_priv_data g_atm_priv_data = {{0}};

static struct atmdev_ops g_ppe_atm_ops = {
    owner:      THIS_MODULE,
    open:       ppe_open,
    close:      ppe_close,
    ioctl:      ppe_ioctl,
    send:       ppe_send,
    send_oam:   ppe_send_oam,
    change_qos: ppe_change_qos,
};

#if defined(ENABLE_CONFIGURABLE_DSL_VLAN) && ENABLE_CONFIGURABLE_DSL_VLAN
  static unsigned short g_dsl_vlan_qid_map[ATM_PVC_NUMBER * 2] = {0};
#endif

#define LINE_NUMBER  1
static int g_showtime[LINE_NUMBER] = {0};

#ifdef CONFIG_IFX_LED
  static u32 g_dsl_wrx_correct_pdu = 0;
  static u32 g_dsl_wtx_total_pdu = 0;
  static struct timer_list g_dsl_led_polling_timer;
  static void *g_data_led_trigger = NULL;
#endif

static uint32_t g_pcie_reset_sig = 0;

int atm_txq_num(void)
{
	return (atm_qos ? (ATM_SW_TX_QUEUE_NUMBER / 2) : ATM_SW_TX_QUEUE_NUMBER);
}

int atm_pvc_num(void)
{
	return (atm_qos ? (ATM_SW_TX_QUEUE_NUMBER / 2) : ATM_PVC_NUMBER);
}

static int ppe_ioctl(struct atm_dev *dev, unsigned int cmd, void *arg)
{
    int ret = 0;
    atm_cell_ifEntry_t mib_cell;
//    atm_aal5_ifEntry_t mib_aal5;
    atm_aal5_vcc_x_t mib_vcc;
//    unsigned int value;
    int conn;

    if ( _IOC_TYPE(cmd) != PPE_ATM_IOC_MAGIC
        || _IOC_NR(cmd) >= PPE_ATM_IOC_MAXNR )
        return -ENOTTY;

    if ( _IOC_DIR(cmd) & _IOC_READ )
        ret = !access_ok(VERIFY_WRITE, arg, _IOC_SIZE(cmd));
    else if ( _IOC_DIR(cmd) & _IOC_WRITE )
        ret = !access_ok(VERIFY_READ, arg, _IOC_SIZE(cmd));
    if ( ret )
        return -EFAULT;

    switch ( cmd )
    {
    case PPE_ATM_MIB_CELL:  /*  cell level  MIB */
        /*  These MIB should be read at ARC side, now put zero only.    */
        mib_cell.ifHCInOctets_h = 0;
        mib_cell.ifHCInOctets_l = 0;
        mib_cell.ifHCOutOctets_h = 0;
        mib_cell.ifHCOutOctets_l = 0;
        mib_cell.ifInErrors = 0;
        mib_cell.ifInUnknownProtos = 0;//DSL_WAN_MIB_TABLE->wrx_drophtu_cell;
        mib_cell.ifOutErrors = 0;

        ret = sizeof(mib_cell) - copy_to_user(arg, &mib_cell, sizeof(mib_cell));
        break;
    case PPE_ATM_MIB_AAL5:  /*  AAL5 MIB    */
#if 0
        value = DSL_WAN_MIB_TABLE->wrx_total_byte;
        u64_add_u32(g_atm_priv_data.wrx_total_byte, value - g_atm_priv_data.prev_mib.wrx_total_byte, &g_atm_priv_data.wrx_total_byte);
        g_atm_priv_data.prev_mib.wrx_total_byte = value;
        mib_aal5.ifHCInOctets_h = g_atm_priv_data.wrx_total_byte.h;
        mib_aal5.ifHCInOctets_l = g_atm_priv_data.wrx_total_byte.l;

        value = DSL_WAN_MIB_TABLE->wtx_total_byte;
        u64_add_u32(g_atm_priv_data.wtx_total_byte, value - g_atm_priv_data.prev_mib.wtx_total_byte, &g_atm_priv_data.wtx_total_byte);
        g_atm_priv_data.prev_mib.wtx_total_byte = value;
        mib_aal5.ifHCOutOctets_h = g_atm_priv_data.wtx_total_byte.h;
        mib_aal5.ifHCOutOctets_l = g_atm_priv_data.wtx_total_byte.l;

        mib_aal5.ifInUcastPkts  = g_atm_priv_data.wrx_pdu;
        mib_aal5.ifOutUcastPkts = DSL_WAN_MIB_TABLE->wtx_total_pdu;
        mib_aal5.ifInErrors     = DSL_WAN_MIB_TABLE->wrx_err_pdu;
        mib_aal5.ifInDiscards   = DSL_WAN_MIB_TABLE->wrx_dropdes_pdu + g_atm_priv_data.wrx_drop_pdu;
        mib_aal5.ifOutErros     = g_atm_priv_data.wtx_err_pdu;
        mib_aal5.ifOutDiscards  = g_atm_priv_data.wtx_drop_pdu;

        ret = sizeof(mib_aal5) - copy_to_user(arg, &mib_aal5, sizeof(mib_aal5));
#endif
        break;
    case PPE_ATM_MIB_VCC:   /*  VCC related MIB */
        copy_from_user(&mib_vcc, arg, sizeof(mib_vcc));
        conn = find_vpivci(mib_vcc.vpi, mib_vcc.vci);
        if ( conn >= 0 )
        {
//            unsigned int sw_tx_queue_table;
//            int sw_tx_queue;

            memset(&mib_vcc.mib_vcc, 0, sizeof(mib_vcc.mib_vcc));
#if 0
            //mib_vcc.mib_vcc.aal5VccCrcErrors     = 0;   //  no support any more, g_atm_priv_data.connection[conn].aal5_vcc_crc_err;
            //mib_vcc.mib_vcc.aal5VccOverSizedSDUs = 0;   //  no support any more, g_atm_priv_data.connection[conn].aal5_vcc_oversize_sdu;
            //mib_vcc.mib_vcc.aal5VccSarTimeOuts   = 0;   //  no timer support
            mib_vcc.mib_vcc.aal5VccRxPDU    = DSL_QUEUE_RX_MIB_TABLE(conn)->pdu;
            mib_vcc.mib_vcc.aal5VccRxBytes  = DSL_QUEUE_RX_MIB_TABLE(conn)->bytes;
            for ( sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, sw_tx_queue = 0;
                  sw_tx_queue_table != 0;
                  sw_tx_queue_table >>= 1, sw_tx_queue++ )
                if ( (sw_tx_queue_table & 0x01) )
                {
                    mib_vcc.mib_vcc.aal5VccTxPDU        += DSL_QUEUE_TX_MIB_TABLE(sw_tx_queue)->pdu;
                    mib_vcc.mib_vcc.aal5VccTxBytes      += DSL_QUEUE_TX_MIB_TABLE(sw_tx_queue)->bytes;
                    mib_vcc.mib_vcc.aal5VccTxDroppedPDU += DSL_QUEUE_TX_DROP_MIB_TABLE(sw_tx_queue)->pdu;
                }
#endif
            ret = sizeof(mib_vcc) - copy_to_user(arg, &mib_vcc, sizeof(mib_vcc));
        }
        else
            ret = -EINVAL;
        break;
    case PPE_ATM_MAP_PKT_PRIO_TO_Q:
        {
            struct ppe_prio_q_map cmd;

            if ( copy_from_user(&cmd, arg, sizeof(cmd)) )
                return -EFAULT;

            if ( cmd.qid < 0 || cmd.pkt_prio < 0 || cmd.pkt_prio >= 8 )
                return -EINVAL;

            conn = find_vpivci(cmd.vpi, cmd.vci);
            if ( conn < 0 )
                return -EINVAL;

            return sw_tx_prio_to_queue(conn, cmd.pkt_prio, cmd.qid, NULL);
        }
        break;
    case PPE_ATM_TX_Q_OP:
        {
            struct tx_q_op cmd;

            if ( copy_from_user(&cmd, arg, sizeof(cmd)) )
                return -EFAULT;

            if ( !(cmd.flags & PPE_ATM_TX_Q_OP_CHG_MASK) )
                return -EFAULT;

            conn = find_vpivci(cmd.vpi, cmd.vci);
            if ( conn < 0 )
                return -EINVAL;

            if ( (cmd.flags & PPE_ATM_TX_Q_OP_ADD) )
                return sw_tx_queue_add(conn);
            else
                return sw_tx_queue_del(conn);
        }
        break;
    case PPE_ATM_GET_MAP_PKT_PRIO_TO_Q:
        {
            struct ppe_prio_q_map_all cmd;
            unsigned int sw_tx_queue_table;
            int sw_tx_queue_to_virt_queue_map[ATM_SW_TX_QUEUE_NUMBER] = {0};
            int sw_tx_queue;
            int virt_queue;
            int i;

            if ( copy_from_user(&cmd, arg, sizeof(cmd)) )
                return -EFAULT;

            conn = find_vpivci(cmd.vpi, cmd.vci);
            if ( conn >= 0 )
            {
                for ( sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, sw_tx_queue = virt_queue = 0;
                      sw_tx_queue_table != 0;
                      sw_tx_queue_table >>= 1, sw_tx_queue++ )
                    if ( (sw_tx_queue_table & 0x01) )
                    {
                        sw_tx_queue_to_virt_queue_map[sw_tx_queue] = virt_queue++;
                    }

                cmd.total_queue_num = virt_queue;
                for ( i = 0; i < 8; i++ )
                {
                    cmd.pkt_prio[i] = i;
                    cmd.qid[i] = sw_tx_queue_to_virt_queue_map[g_atm_priv_data.connection[conn].prio_queue_map[i]];
                }

                ret = sizeof(cmd) - copy_to_user(arg, &cmd, sizeof(cmd));
            }
            else
                ret = -EINVAL;
        }
        break;
    default:
        ret = -ENOIOCTLCMD;
    }

    return ret;
}

static int ppe_open(struct atm_vcc *vcc)
{
    int ret;
    short vpi = vcc->vpi;
    int   vci = vcc->vci;
    struct atm_port *port = &g_atm_priv_data.port[(int)vcc->dev->dev_data];
    int sw_tx_queue;
    int conn;
    int f_enable_irq = 0;
    wrx_queue_config_t wrx_queue_config = {0};
    wtx_queue_config_t wtx_queue_config = {0};
    struct uni_cell_header *cell_header;
    int i;

    if ( vcc->qos.aal != ATM_AAL5 && vcc->qos.aal != ATM_AAL0 )
        return -EPROTONOSUPPORT;

    /*  check bandwidth */
    if ( (vcc->qos.txtp.traffic_class == ATM_CBR && vcc->qos.txtp.max_pcr > (port->tx_max_cell_rate - port->tx_used_cell_rate))
      || (vcc->qos.txtp.traffic_class == ATM_VBR_RT && vcc->qos.txtp.max_pcr > (port->tx_max_cell_rate - port->tx_used_cell_rate))
#if defined(DISABLE_VBR) && DISABLE_VBR
      || (vcc->qos.txtp.traffic_class == ATM_VBR_NRT && vcc->qos.txtp.pcr > (port->tx_max_cell_rate - port->tx_used_cell_rate))
#else
      || (vcc->qos.txtp.traffic_class == ATM_VBR_NRT && vcc->qos.txtp.scr > (port->tx_max_cell_rate - port->tx_used_cell_rate))
#endif
      || (vcc->qos.txtp.traffic_class == ATM_UBR_PLUS && vcc->qos.txtp.min_pcr > (port->tx_max_cell_rate - port->tx_used_cell_rate)) )
    {
        err("exceed TX line rate");
        ret = -EINVAL;
        goto PPE_OPEN_EXIT;
    }

    /*  check existing vpi,vci  */
    conn = find_vpivci(vpi, vci);
    if ( conn >= 0 )
    {
        err("existing PVC (%d.%d)", (int)vpi, vci);
        ret = -EADDRINUSE;
        goto PPE_OPEN_EXIT;
    }

    /*  check whether it need to enable irq */
    if ( g_atm_priv_data.pvc_table == 0 )
        f_enable_irq = 1;

    /*  allocate software TX queue  */
    for ( sw_tx_queue = 0; sw_tx_queue < atm_txq_num(); sw_tx_queue++ )
        if ( !(g_atm_priv_data.sw_tx_queue_table & (1 << sw_tx_queue)) )
            break;
    if ( sw_tx_queue >= atm_txq_num() )
    {
        err("no free TX queue");
        ret = -EINVAL;
        goto PPE_OPEN_EXIT;
    }

    /*  allocate PVC    */
    for ( conn = 0; conn < atm_pvc_num(); conn++ )
        if ( !(g_atm_priv_data.pvc_table & (1 << conn)) )
        {
            g_atm_priv_data.pvc_table |= 1 << conn;
            g_atm_priv_data.sw_tx_queue_table |= 1 << sw_tx_queue;
            g_atm_priv_data.connection[conn].vcc = vcc;
            g_atm_priv_data.connection[conn].port = (unsigned int)vcc->dev->dev_data;
            g_atm_priv_data.connection[conn].sw_tx_queue_table = 1 << sw_tx_queue;
            for ( i = 0; i < 8; i++ )
                g_atm_priv_data.connection[conn].prio_queue_map[i] = sw_tx_queue;
            break;
        }
    if ( conn >= atm_pvc_num() )
    {
        err("exceed PVC limit");
        ret = -EINVAL;
        goto PPE_OPEN_EXIT;
    }

    /*  reserve bandwidth   */
    switch ( vcc->qos.txtp.traffic_class )
    {
    case ATM_CBR:
    case ATM_VBR_RT:
        port->tx_used_cell_rate += vcc->qos.txtp.max_pcr;
        break;
    case ATM_VBR_NRT:
#if defined(DISABLE_VBR) && DISABLE_VBR
        port->tx_used_cell_rate += vcc->qos.txtp.pcr;
#else
        port->tx_used_cell_rate += vcc->qos.txtp.scr;
#endif
        break;
    case ATM_UBR_PLUS:
        port->tx_used_cell_rate += vcc->qos.txtp.min_pcr;
        break;
    }

    /*  setup RX queue cfg and TX queue cfg */
    wrx_queue_config.new_vlan   = 0x0FF0 | conn;//  use vlan to differentiate PVCs
    wrx_queue_config.vlan_ins   = 1;            //  use vlan to differentiate PVCs, please use outer VLAN in routing table, and do remember to configure switch to handle bridging case
    wrx_queue_config.mpoa_type  = 3;            //  set IPoA as default
    wrx_queue_config.ip_ver     = 0;            //  set IPv4 as default
    wrx_queue_config.mpoa_mode  = 0;            //  set VC-mux as default
    wrx_queue_config.oversize   = aal5r_max_packet_size;
    wrx_queue_config.undersize  = aal5r_min_packet_size;
    wrx_queue_config.mfs        = aal5r_max_packet_size;

    wtx_queue_config.same_vc_qmap   = 0x00;     //  only one TX queue is assigned now, use ioctl to add other TX queues
    wtx_queue_config.sbid           = g_atm_priv_data.connection[conn].port;
    wtx_queue_config.qsb_vcid       = conn + QSB_QUEUE_NUMBER_BASE; //  qsb qid = firmware qid + 1
    wtx_queue_config.mpoa_mode      = 0;        //  set VC-mux as default
    wtx_queue_config.qsben          = 1;        //  reserved in A4, however put 1 for backward compatible

    cell_header = (struct uni_cell_header *)((unsigned int *)&wtx_queue_config + 2);
    cell_header->clp    = (vcc->atm_options & ATM_ATMOPT_CLP) ? 1 : 0;
    cell_header->pti    = ATM_PTI_US0;
    cell_header->vci    = vci;
    cell_header->vpi    = vpi;
    cell_header->gfc    = 0;

    *WRX_QUEUE_CONFIG(conn) = wrx_queue_config;
    *WTX_QUEUE_CONFIG(sw_tx_queue) = wtx_queue_config;

    /*  set qsb */
    set_qsb(vcc, &vcc->qos, conn);

    /*  update atm_vcc structure    */
    vcc->itf = (int)vcc->dev->dev_data;
    set_bit(ATM_VF_ADDR, &vcc->flags);
    set_bit(ATM_VF_READY, &vcc->flags);

    if ( f_enable_irq )
    {
        ifx_atm_alloc_tx = atm_alloc_tx;
        turn_on_dma_rx(31);
        validate_oam_htu_entry();

#ifdef CONFIG_IFX_LED
        g_dsl_wrx_correct_pdu = DSL_WAN_MIB_TABLE->wrx_correct_pdu;
        g_dsl_wtx_total_pdu = DSL_WAN_MIB_TABLE->wtx_total_pdu;
        g_dsl_led_polling_timer.expires = jiffies + HZ / 10;    //  100ms
        add_timer(&g_dsl_led_polling_timer);
#endif
    }

    /*  set htu entry   */
    set_htu_entry(vpi, vci, conn, vcc->qos.aal == ATM_AAL5 ? 1 : 0);

    dbg("ppe_open(%d.%d): conn = %d", vcc->vpi, vcc->vci, conn);
    return 0;

PPE_OPEN_EXIT:
    return ret;
}

static void ppe_close(struct atm_vcc *vcc)
{
    int conn;
    struct atm_port *port;
    struct atm_pvc *connection;
    int i;

    if ( vcc == NULL )
        return;

    /*  get connection id   */
    conn = find_vcc(vcc);
    if ( conn < 0 )
    {
        err("can't find vcc");
        goto PPE_CLOSE_EXIT;
    }
    connection = &g_atm_priv_data.connection[conn];
    port = &g_atm_priv_data.port[connection->port];

    clear_bit(ATM_VF_READY, &vcc->flags);
    clear_bit(ATM_VF_ADDR, &vcc->flags);

    /*  clear htu   */
    clear_htu_entry(conn);

    /*  release connection  */
    g_atm_priv_data.pvc_table &= ~(1 << conn);
    g_atm_priv_data.sw_tx_queue_table &= ~(connection->sw_tx_queue_table);
    memset(connection, 0, sizeof(*connection));

    /*  disable irq */
    if ( g_atm_priv_data.pvc_table == 0 )
    {
#ifdef CONFIG_IFX_LED
        del_timer(&g_dsl_led_polling_timer);
#endif

        invalidate_oam_htu_entry();
        turn_off_dma_rx(31);
        ifx_atm_alloc_tx = NULL;
    }

    /*  release bandwidth   */
    switch ( vcc->qos.txtp.traffic_class )
    {
    case ATM_CBR:
    case ATM_VBR_RT:
        port->tx_used_cell_rate -= vcc->qos.txtp.max_pcr;
        break;
    case ATM_VBR_NRT:
#if defined(DISABLE_VBR) && DISABLE_VBR
        port->tx_used_cell_rate -= vcc->qos.txtp.pcr;
#else
        port->tx_used_cell_rate -= vcc->qos.txtp.scr;
#endif
        break;
    case ATM_UBR_PLUS:
        port->tx_used_cell_rate -= vcc->qos.txtp.min_pcr;
        break;
    }

    /*  idle for a while to let parallel operation finish   */
    udelay(100);

#if defined(ENABLE_CONFIGURABLE_DSL_VLAN) && ENABLE_CONFIGURABLE_DSL_VLAN
    for ( i = 0; i < ARRAY_SIZE(g_dsl_vlan_qid_map); i += 2 )
        if ( g_dsl_vlan_qid_map[i] == WRX_QUEUE_CONFIG(conn)->new_vlan )
        {
            g_dsl_vlan_qid_map[i] = 0;
            break;
        }
#endif

PPE_CLOSE_EXIT:
    return;
}

static int ppe_send(struct atm_vcc *vcc, struct sk_buff *skb)
{
    int ret;
    int conn;
    unsigned long sys_flag;
    volatile struct tx_descriptor *desc;
    struct tx_descriptor reg_desc;
    struct sk_buff *skb_to_free;
    int byteoff;
    unsigned int priority;

    if ( vcc == NULL || skb == NULL )
        return -EINVAL;

    if ( skb->len > aal5s_max_packet_size )
        return -EOVERFLOW;

    ATM_SKB(skb)->vcc = vcc;

    if ( g_stop_datapath != 0 )
    {
        atm_free_tx_skb_vcc(skb);
        return -EBUSY;
    }

    skb_get(skb);
    atm_free_tx_skb_vcc(skb);
    ATM_SKB(skb)->vcc = NULL;

    conn = find_vcc(vcc);
    if ( conn < 0 )
    {
        err("not find vcc");
        ret = -EINVAL;
        goto FIND_VCC_FAIL;
    }

#if 0 //dsl still have 2 fw running if it was build in bonding project that will cause driver always think it's not in showtime
    if ( !in_showtime() )
    {
        err("not in showtime");
        ret = -EIO;
        goto CHECK_SHOWTIME_FAIL;
    }
#endif

    dump_skb(skb, DUMP_SKB_LEN, "ppe_send - raw", 7, reg_desc.qid, 1, 0);
    swap_mac(skb->data + 10);   //  ignore AAL5 header
    dump_skb(skb, DUMP_SKB_LEN, "ppe_send - swap", 7, reg_desc.qid, 1, 0);

    priority = skb->priority;
    if ( priority >= 8 )
        priority = 7;

    /*  reserve space to put pointer in skb */
    byteoff = (unsigned int)skb->data & (DMA_ALIGNMENT - 1);
    if ( byteoff || skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff	|| skb_shared(skb) || skb_cloned(skb) 
		|| (unsigned int)skb->end - (unsigned int)skb->data < DMA_PACKET_SIZE)
    {
        //  this should be few case

        struct sk_buff *new_skb;

        //dbg("skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff");
        ASSERT(skb_headroom(skb) >= sizeof(struct sk_buff *) + byteoff, "skb_headroom(skb) < sizeof(struct sk_buff *) + byteoff");
        ASSERT(skb->end - skb->data >= DMA_PACKET_SIZE, "skb->end - skb->data < 1518 + 22 + 10 + 6 + 4 x 2");

        new_skb = alloc_skb_tx(skb->len < DMA_PACKET_SIZE ? DMA_PACKET_SIZE : skb->len);    //  may be used by RX fastpath later
        if ( !new_skb )
        {
            err("no memory");
            ret = -ENOMEM;
            goto ALLOC_SKB_TX_FAIL;
        }
        skb_put(new_skb, skb->len);
        memcpy(new_skb->data, skb->data, skb->len);
        atm_free_tx_skb_vcc(skb);
        skb = new_skb;
        byteoff = (unsigned int)skb->data & (DMA_ALIGNMENT - 1);
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((unsigned int)skb->data, skb->len);
#endif
    }
    else
    {
        *(struct sk_buff **)((unsigned int)skb->data - byteoff - sizeof(struct sk_buff *)) = skb;
#ifndef CONFIG_MIPS_UNCACHED
        /*  write back to physical memory   */
        dma_cache_wback((unsigned int)skb->data - byteoff - sizeof(struct sk_buff *), skb->len + byteoff + sizeof(struct sk_buff *));
#endif
    }

    /*  allocate descriptor */
    spin_lock_irqsave(&g_cpu_to_wan_tx_desc_lock, sys_flag);
    desc = CPU_TO_WAN_TX_DESC_BASE + g_cpu_to_wan_tx_desc_pos;
    if ( desc->own )    //  PPE hold
    {
        spin_unlock_irqrestore(&g_cpu_to_wan_tx_desc_lock, sys_flag);
        ret = -EIO;
        err("PPE hold");
        goto NO_FREE_DESC;
    }
    if ( ++g_cpu_to_wan_tx_desc_pos == CPU_TO_WAN_TX_DESC_NUM )
        g_cpu_to_wan_tx_desc_pos = 0;
    spin_unlock_irqrestore(&g_cpu_to_wan_tx_desc_lock, sys_flag);

    /*  load descriptor from memory */
    reg_desc = *desc;

    /*  free previous skb   */
    ASSERT((reg_desc.dataptr & (DMA_ALIGNMENT - 1)) == 0, "reg_desc.dataptr (0x%#x) must be 8 DWORDS aligned", reg_desc.dataptr);
    skb_to_free = get_skb_pointer(reg_desc.dataptr);
    dev_kfree_skb_any(skb_to_free);

    /*  detach from protocol    */
    skb_to_free = skb;
    skb = skb_break_away_from_protocol(skb);
    dev_kfree_skb_any(skb_to_free);

    put_skb_to_dbg_pool(skb);

#if defined(DEBUG_MIRROR_PROC) && DEBUG_MIRROR_PROC
    if ( g_mirror_netdev != NULL )
    {
        if ( (WRX_QUEUE_CONFIG(conn)->mpoa_type & 0x02) == 0 )  //  handle EoA only, TODO: IPoA/PPPoA
        {
            struct sk_buff *new_skb = skb_copy(skb, GFP_ATOMIC);

            if ( new_skb != NULL )
            {
                if ( WRX_QUEUE_CONFIG(conn)->mpoa_mode )    //  LLC
                    skb_pull(new_skb, 10);  //  remove EoA header
                else                                        //  VC mux
                    skb_pull(new_skb, 2);   //  remove EoA header
                if ( WRX_QUEUE_CONFIG(conn)->mpoa_type == 1 )   //  remove FCS
                    skb_trim(new_skb, new_skb->len - 4);
                new_skb->dev = g_mirror_netdev;
                dev_queue_xmit(new_skb);
            }
        }
    }
#endif

    /*  update descriptor   */
    reg_desc.dataptr    = (unsigned int)skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1));  //  byte address
    reg_desc.byteoff    = byteoff;
    reg_desc.datalen    = skb->len;
    reg_desc.mpoa_pt    = 1;
    reg_desc.mpoa_type  = 0;    //  this flag should be ignored by firmware
    reg_desc.pdu_type   = vcc->qos.aal == ATM_AAL5 ? 0 : 1;
    reg_desc.qid        = g_atm_priv_data.connection[conn].prio_queue_map[priority];
    reg_desc.own        = 1;
    reg_desc.c          = 0;
    reg_desc.sop = reg_desc.eop = 1;

    /*  update MIB  */
    UPDATE_VCC_STAT(conn, tx_packets, 1);
    UPDATE_VCC_STAT(conn, tx_bytes, skb->len);

    if ( vcc->stats )
        atomic_inc(&vcc->stats->tx);
    if ( vcc->qos.aal == ATM_AAL5 )
        g_atm_priv_data.wtx_pdu++;

    g_atm_priv_data.connection[conn].prio_tx_packets[priority]++;

    /*  write discriptor to memory and write back cache */
    *((volatile unsigned int *)desc + 1) = *((unsigned int *)&reg_desc + 1);
    *(volatile unsigned int *)desc = *(unsigned int *)&reg_desc;

    return 0;

FIND_VCC_FAIL:
    if ( vcc->stats )
    {
        atomic_inc(&vcc->stats->tx_err);
    }
    if ( vcc->qos.aal == ATM_AAL5 )
    {
        g_atm_priv_data.wtx_err_pdu++;
        g_atm_priv_data.wtx_drop_pdu++;
    }
    atm_free_tx_skb_vcc(skb);
    return ret;

/* CHECK_SHOWTIME_FAIL: */
ALLOC_SKB_TX_FAIL:
    if ( vcc->stats )
        atomic_inc(&vcc->stats->tx_err);
    if ( vcc->qos.aal == ATM_AAL5 )
        g_atm_priv_data.wtx_drop_pdu++;
    UPDATE_VCC_STAT(conn, tx_dropped, 1);
    atm_free_tx_skb_vcc(skb);
    return ret;

NO_FREE_DESC:
    if ( vcc->stats )
        atomic_inc(&vcc->stats->tx_err);
    if ( vcc->qos.aal == ATM_AAL5 )
        g_atm_priv_data.wtx_drop_pdu++;
    UPDATE_VCC_STAT(conn, tx_dropped, 1);
    atm_free_tx_skb_vcc(skb);
    return ret;
}

static int ppe_send_oam(struct atm_vcc *vcc, void *cell, int flags)
{
    int conn;
    struct uni_cell_header *uni_cell_header = (struct uni_cell_header *)cell;
    struct sk_buff *skb;
    unsigned long sys_flag;
    volatile struct tx_descriptor *desc;
    struct tx_descriptor reg_desc;
    struct sk_buff *skb_to_free;

    if ( ((uni_cell_header->pti == ATM_PTI_SEGF5 || uni_cell_header->pti == ATM_PTI_E2EF5)
        && find_vpivci(uni_cell_header->vpi, uni_cell_header->vci) < 0)
        || ((uni_cell_header->vci == 0x03 || uni_cell_header->vci == 0x04)
        && find_vpi(uni_cell_header->vpi) < 0) )
        return -EINVAL;

#if 0
    if ( !in_showtime() )
    {
        err("not in showtime");
        return -EIO;
    }
#endif

    /*  find queue ID   */
    conn = find_vcc(vcc);
    if ( conn < 0 )
    {
        err("OAM not find queue");
        return -EINVAL;
    }

    /*  allocate sk_buff    */
    skb = alloc_skb_tx(DMA_PACKET_SIZE);    //  may be used by RX fastpath later
    if ( skb == NULL )
        return -ENOMEM;

    /*  copy data   */
    skb_put(skb, CELL_SIZE);
    memcpy(skb->data, cell, CELL_SIZE);

#ifndef CONFIG_MIPS_UNCACHED
    /*  write back to physical memory   */
    dma_cache_wback((unsigned int)skb->data, skb->len);
#endif

    /*  allocate descriptor */
    spin_lock_irqsave(&g_cpu_to_wan_tx_desc_lock, sys_flag);
    desc = CPU_TO_WAN_TX_DESC_BASE + g_cpu_to_wan_tx_desc_pos;
    if ( desc->own )    //  PPE hold
    {
        spin_unlock_irqrestore(&g_cpu_to_wan_tx_desc_lock, sys_flag);
        err("OAM not alloc tx connection");
        return -EIO;
    }
    if ( ++g_cpu_to_wan_tx_desc_pos == CPU_TO_WAN_TX_DESC_NUM )
        g_cpu_to_wan_tx_desc_pos = 0;
    spin_unlock_irqrestore(&g_cpu_to_wan_tx_desc_lock, sys_flag);

    /*  load descriptor from memory */
    reg_desc = *desc;

    /*  free previous skb   */
    ASSERT((reg_desc.dataptr & (DMA_ALIGNMENT - 1)) == 0, "reg_desc.dataptr (0x%#x) must be 8 DWORDS aligned", reg_desc.dataptr);
    skb_to_free = get_skb_pointer(reg_desc.dataptr);
    dev_kfree_skb_any(skb_to_free);
    put_skb_to_dbg_pool(skb);

    /*  setup descriptor    */
    reg_desc.dataptr    = (unsigned int)skb->data & (0x1FFFFFFF ^ (DMA_ALIGNMENT - 1));  //  byte address
    reg_desc.byteoff    = (unsigned int)skb->data & (DMA_ALIGNMENT - 1);
    reg_desc.datalen    = skb->len;
    reg_desc.mpoa_pt    = 1;
    reg_desc.mpoa_type  = 0;    //  this flag should be ignored by firmware
    reg_desc.pdu_type   = 1;    //  cell
    reg_desc.qid        = g_atm_priv_data.connection[conn].prio_queue_map[7];   //  expect priority '7' is highest priority
    reg_desc.own        = 1;
    reg_desc.c          = 0;
    reg_desc.sop = reg_desc.eop = 1;

    dump_skb(skb, DUMP_SKB_LEN, "ppe_send_oam", 7, reg_desc.qid, 1, 0);

    /*  write discriptor to memory and write back cache */
    *((volatile unsigned int *)desc + 1) = *((unsigned int *)&reg_desc + 1);
    *(volatile unsigned int *)desc = *(unsigned int *)&reg_desc;

    return 0;
}

static int ppe_change_qos(struct atm_vcc *vcc, struct atm_qos *qos, int flags)
{
    int conn;

    if ( vcc == NULL || qos == NULL )
        return -EINVAL;

    conn = find_vcc(vcc);
    if ( conn < 0 )
        return -EINVAL;

    set_qsb(vcc, qos, conn);

    return 0;
}

static int atm_get_pvc_id(struct sk_buff *skb, unsigned int ifid)
{
    int conn;

    if ( ifid == 6 )
        conn = skb->data[11] & 0x3F;    //  QId: 6bits
    else
    {
        int i;

        ASSERT(skb->data[12] == 0x81 && skb->data[13] == 0x00, "EoA traffic: no outer VLAN for PVCs differentiation");  //  outer VLAN
#if defined(ENABLE_CONFIGURABLE_DSL_VLAN) && ENABLE_CONFIGURABLE_DSL_VLAN
        if ( skb->data[14] != 0x0F || (skb->data[15] & 0xF0) != 0xF0 )
        {
            unsigned short vlan_id = ((unsigned short)skb->data[14] << 8) | (unsigned short)skb->data[15];
            for ( i = 0; i < ARRAY_SIZE(g_dsl_vlan_qid_map); i += 2 )
                if ( g_dsl_vlan_qid_map[i] == vlan_id )
                {
                    skb->data[15] = (unsigned char)g_dsl_vlan_qid_map[i + 1];
                    break;
                }
            ASSERT(i < ARRAY_SIZE(g_dsl_vlan_qid_map), "search DSL VLAN table fail");
        }
#endif
        conn = skb->data[15] & 0x0F;
        //  remove outer VLAN tag
        for ( i = 11; i >= 0; i-- )
            skb->data[i + 4] = skb->data[i];
        skb_pull(skb, 4);
    }

    return conn;
}

static void atm_encapsulate_frame(struct sk_buff *skb, struct flag_header *header, int conn)
{
    /*  ETH packet, need recover ATM encapsulation  */
    if ( WRX_QUEUE_CONFIG(conn)->mpoa_mode )
    {
        unsigned int proto_type;

        /*  LLC */
        switch ( WRX_QUEUE_CONFIG(conn)->mpoa_type )
        {
        case 0: //  EoA w/o FCS
            ASSERT(skb_headroom(skb) >= 10, "not enough skb headroom (LLC EoA w/o FCS)");
            ASSERT(((u32)skb->data & 0x03) == 2, "not aligned data pointer (LLC EoA w/o FCS)");
            skb->data -= 10;
            skb->len  += 10;
            ((u32 *)skb->data)[0] = 0xAAAA0300;
            ((u32 *)skb->data)[1] = 0x80C20007;
            ((u16 *)skb->data)[4] = 0x0000;
            break;
        case 1: //  EoA w FCS
            ASSERT(skb_headroom(skb) >= 10, "not enough skb headroom (LLC EoA w FCS)");
            ASSERT(((u32)skb->data & 0x03) == 2, "not aligned data pointer (LLC EoA w FCS)");
            skb->data -= 10;
            skb->len  += 10;
            ((u32 *)skb->data)[0] = 0xAAAA0300;
            ((u32 *)skb->data)[1] = 0x80C20001;
            ((u16 *)skb->data)[4] = 0x0000;
            break;
        case 2: //  PPPoA
            switch ( (proto_type = ntohs(*(u16 *)(skb->data + 12))) )
            {
            case 0x0800: proto_type = 0x0021; break;
            case 0x86DD: proto_type = 0x0057; break;
            }
            ASSERT(header->ip_inner_offset - 6 >= 0 || skb_headroom(skb) >= 6 - header->ip_inner_offset, "not enough skb headroom (LLC PPPoA)");
            skb->data += 0x0E - 6;
            skb->len  -= 0x0E - 6;
            ASSERT(((u32)skb->data & 0x03) == 2, "not aligned data pointer (LLC PPPoA)");
            ((u32 *)skb->data)[0] = 0xFEFE03CF;
            ((u16 *)skb->data)[2] = (u16)proto_type;
            break;
        case 3: //  IPoA
            ASSERT(header->ip_inner_offset - 8 >= 0 || skb_headroom(skb) >= 8 - header->ip_inner_offset, "not enough skb headroom (LLC IPoA)");
            skb->data += header->ip_inner_offset - 8;
            skb->len  -= header->ip_inner_offset - 8;
            ASSERT(((u32)skb->data & 0x03) == 0, "not aligned data pointer (LLC IPoA)");
            ((u32 *)skb->data)[0] = 0xAAAA0300;
            ((u16 *)skb->data)[2] = 0x0000;
            break;
        }
    }
    else
    {
        unsigned int proto_type;

        /*  VC-mux  */
        switch ( WRX_QUEUE_CONFIG(conn)->mpoa_type )
        {
        case 0: //  EoA w/o FCS
            ASSERT(skb_headroom(skb) >= 2, "not enough skb headroom (VC-mux EoA w/o FCS)");
            ASSERT(((u32)skb->data & 0x03) == 2, "not aligned data pointer (VC-mux EoA w/o FCS)");
            skb->data -= 2;
            skb->len  += 2;
            *(u16 *)skb->data = 0x0000;
            break;
        case 1: //  EoA w FCS
            ASSERT(skb_headroom(skb) >= 2, "not enough skb headroom (VC-mux EoA w FCS)");
            ASSERT(((u32)skb->data & 0x03) == 2, "not aligned data pointer (VC-mux EoA w FCS)");
            skb->data -= 2;
            skb->len  += 2;
            *(u16 *)skb->data = 0x0000;
            break;
        case 2: //  PPPoA
            switch ( (proto_type = ntohs(*(u16 *)(skb->data + 12))) )
            {
            case 0x0800: proto_type = 0x0021; break;
            case 0x86DD: proto_type = 0x0057; break;
            }
            ASSERT(header->ip_inner_offset - 2 >= 0 || skb_headroom(skb) >= 2 - header->ip_inner_offset, "not enough skb headroom (VC-mux PPPoA)");
            skb->data += 0x0E - 2;
            skb->len  -= 0x0E - 2;
            ASSERT(((u32)skb->data & 0x03) == 2, "not aligned data pointer (VC-mux PPPoA)");
            *(u16 *)skb->data = (u16)proto_type;
            break;
        case 3: //  IPoA
            skb->data += header->ip_inner_offset;
            skb->len  -= header->ip_inner_offset;
            break;
        }
    }
    dump_skb(skb, DUMP_SKB_LEN, "atm_encapsulate_frame AAL5 encapsulated", header->src_itf, 0, 0, 0);
}

static int atm_push(struct sk_buff *skb, struct flag_header *header, unsigned int ifid)
{
    if ( ifid == 6 /* IPoA/PPPoA */ || ifid == 7 /* EoA */ )
    {
        int conn;
        struct atm_vcc *vcc = NULL;

        conn = atm_get_pvc_id(skb, ifid);
        if ( conn < 0 || conn >= atm_pvc_num() || (vcc = g_atm_priv_data.connection[conn].vcc) != NULL )
        {
            if ( atm_charge(vcc, skb->truesize) )
            {
                ATM_SKB(skb)->vcc = vcc;

                atm_encapsulate_frame(skb, header, conn);

                g_atm_priv_data.connection[conn].access_time = current_kernel_time();

                vcc->push(vcc, skb);

                if ( vcc->stats )
                    atomic_inc(&vcc->stats->rx);

                if ( vcc->qos.aal == ATM_AAL5 )
                    g_atm_priv_data.wrx_pdu++;

                UPDATE_VCC_STAT(conn, rx_packets, 1);
                UPDATE_VCC_STAT(conn, rx_bytes, skb->len);

                return 0;
            }
            else
            {
                dbg("inactive qid %d", conn);

                if ( vcc->stats )
                    atomic_inc(&vcc->stats->rx_drop);

                if ( vcc->qos.aal == ATM_AAL5 )
                    g_atm_priv_data.wrx_drop_pdu++;

                UPDATE_VCC_STAT(conn, rx_dropped, 1);
            }
        }
        dev_kfree_skb_any(skb);

        return 0;
    }
    else{
        dev_kfree_skb_any(skb);
        return -EINVAL;
    }
}

int get_dslwan_qid_with_vcc(struct atm_vcc *vcc)
{
    int conn = find_vcc(vcc);
    u32 sw_tx_queue_table;
    int sw_tx_queue;

    if ( conn >= 0 )
    {
        for ( sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, sw_tx_queue = 0;
              sw_tx_queue_table != 0;
              sw_tx_queue_table >>= 1, sw_tx_queue++ )
            if ( (sw_tx_queue_table & 0x01) )
                break;

        conn = (conn << 8) | sw_tx_queue;
    }

    return conn;
}

int get_atm_qid_with_pkt(struct sk_buff *skb, void *arg, int is_atm_vcc)
{
    if ( is_atm_vcc )
    {
        int conn = find_vcc((struct atm_vcc *)arg);

        if ( conn < 0 )
            return -1;

        return (conn << 8) | g_atm_priv_data.connection[conn].prio_queue_map[skb->priority > 7 ? 7 : skb->priority];
    }
    

    return -1;
}

int get_mpoa_type(uint32_t dslwan_qid, uint32_t *mpoa_type)
{
    int ret = -1;
    int qid = (dslwan_qid >> 8) & 0xFF;

    if(qid >= 0 && qid < 16){
        ret = 0;
        *mpoa_type = WRX_QUEUE_CONFIG(qid)->mpoa_type;
    }

    return ret;
}

static void do_oam_tasklet(unsigned long arg)
{
    //const static unsigned int oam_sb_addr[10] = { 0x3BC0, 0x3BE0, 0x3C00, 0x3C20, 0x3C40, 0x3C60, 0x3C80, 0x3CA0, 0x3CC0, 0x3CE0 };
    unsigned long sys_flag;
    volatile struct rx_descriptor *pdesc;
    struct uni_cell_header *header;
    int conn;
    struct atm_vcc *vcc;
    volatile desq_cfg_ctxt_t *desq_oam_cfg_ctxt;
    volatile desq_cfg_ctxt_t *lclq_oam_cfg_ctxt;

    desq_oam_cfg_ctxt = (volatile desq_cfg_ctxt_t *)SOC_ACCESS_VRX218_SB(__DS_OAM_DESQ_CFG_CTXT, g_atm_priv_data.vrx218_dev.phy_membase);
    lclq_oam_cfg_ctxt = (volatile desq_cfg_ctxt_t *)SOC_ACCESS_VRX218_SB(__DS_TC_OAM_LOCAL_Q_CFG_CTXT, g_atm_priv_data.vrx218_dev.phy_membase);

    while ( 1 ) {
        //pdesc = (volatile struct rx_descriptor *)SOC_ACCESS_VRX218_SB(__DS_TC_LOCAL_OAMQ_DES_LIST_BASE + (g_oam_desc_pos * 2), g_atm_priv_data.vrx218_dev.phy_membase);
        //if ( pdesc->own ) {
        pdesc = (volatile struct rx_descriptor *)KSEG1ADDR(SOC_DS_OAM_DES_BASE) + g_oam_desc_pos;
        if ( !pdesc->own ) {
            *MBOX_IGU1_ISRC = 0x02;
            //if ( pdesc->own ) {
            if ( !pdesc->own ) {
                spin_lock_irqsave(&g_mailbox_lock, sys_flag);
                *MBOX_IGU1_IER |= 0x02;
                spin_unlock_irqrestore(&g_mailbox_lock, sys_flag);

                if(lclq_oam_cfg_ctxt->enq_pkt_cnt > desq_oam_cfg_ctxt->enq_pkt_cnt ||
                      desq_oam_cfg_ctxt->enq_pkt_cnt > desq_oam_cfg_ctxt->deq_pkt_cnt){
                      tasklet_schedule(&g_oam_tasklet);
                }
                break;
            }
        }

        //header = (struct uni_cell_header *)SOC_ACCESS_VRX218_SB(oam_sb_addr[g_oam_desc_pos], g_atm_priv_data.vrx218_dev.phy_membase);
        header = (struct uni_cell_header *)KSEG1ADDR(pdesc->dataptr);

        if ( header->pti == ATM_PTI_SEGF5 || header->pti == ATM_PTI_E2EF5 )
            conn = find_vpivci(header->vpi, header->vci);
        else if ( header->vci == 0x03 || header->vci == 0x04 )
            conn = find_vpi(header->vpi);
        else
            conn = -1;

        if ( conn >= 0 && (vcc = g_atm_priv_data.connection[conn].vcc) != NULL ) {
            g_atm_priv_data.connection[conn].access_time = current_kernel_time();

            if ( vcc->push_oam != NULL )
                vcc->push_oam(vcc, header);
            else
                ifx_push_oam(vcc, (unsigned char *)header);

            UPDATE_VCC_STAT(conn, rx_packets, 1);
            UPDATE_VCC_STAT(conn, rx_bytes, 52);
        }

        pdesc->c = 0;
        //pdesc->own = 1;
        pdesc->own = 0;

        desq_oam_cfg_ctxt->deq_pkt_cnt++;

        //if ( ++g_oam_desc_pos == ARRAY_SIZE(oam_sb_addr) )
        if ( ++g_oam_desc_pos == SOC_DS_OAM_DES_NUM )
            g_oam_desc_pos = 0;
    }
}

static void mpoa_setup(struct atm_vcc *vcc, int mpoa_type, int f_llc)
{
    int conn;
    unsigned int sw_tx_queue_table;
    int sw_tx_queue;

    conn = find_vcc(vcc);
    if ( conn < 0 )
        return;

    WRX_QUEUE_CONFIG(conn)->mpoa_type = mpoa_type;
    WRX_QUEUE_CONFIG(conn)->mpoa_mode = f_llc;

    for ( sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, sw_tx_queue = 0;
          sw_tx_queue_table != 0;
          sw_tx_queue_table >>= 1, sw_tx_queue++ )
        if ( (sw_tx_queue_table & 0x01) )
            WTX_QUEUE_CONFIG(sw_tx_queue)->mpoa_mode = f_llc;
}

#ifdef CONFIG_LTQ_LED
static void dsl_led_flash(void)
{
    if ( g_data_led_trigger != NULL )
        ifx_led_trigger_activate(g_data_led_trigger);
}

static void dsl_led_polling(unsigned long arg)
{
    if ( DSL_WAN_MIB_TABLE->wrx_correct_pdu != g_dsl_wrx_correct_pdu || DSL_WAN_MIB_TABLE->wtx_total_pdu != g_dsl_wtx_total_pdu )
    {
        g_dsl_wrx_correct_pdu = DSL_WAN_MIB_TABLE->wrx_correct_pdu;
        g_dsl_wtx_total_pdu = DSL_WAN_MIB_TABLE->wtx_total_pdu;

        dsl_led_flash();
    }

    if ( ifx_atm_alloc_tx != NULL )
    {
        g_dsl_led_polling_timer.expires = jiffies + HZ / 10;    //  100ms
        add_timer(&g_dsl_led_polling_timer);
    }
}
#endif

//  add one lower priority queue
static int sw_tx_queue_add(int conn)
{
    int ret;
    int new_sw_tx_queue;
    u32 new_sw_tx_queue_table;
    int sw_tx_queue_to_virt_queue_map[ATM_SW_TX_QUEUE_NUMBER] = {0};
    int virt_queue_to_sw_tx_queue_map[8] = {0};
    int num_of_virt_queue;
    u32 sw_tx_queue_table;
    int sw_tx_queue;
    int virt_queue;
    wtx_queue_config_t wtx_queue_config;
    int f_got_wtx_queue_config = 0;
    int i;

    for ( new_sw_tx_queue = 0; new_sw_tx_queue < atm_txq_num(); new_sw_tx_queue++ )
        if ( !(g_atm_priv_data.sw_tx_queue_table & (1 << new_sw_tx_queue)) )
            break;
    if ( new_sw_tx_queue >= atm_txq_num() )
    {
        ret = -EINVAL;
        goto SW_TX_QUEUE_ADD_EXIT;
    }

    new_sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table | (1 << new_sw_tx_queue);

    for ( sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, sw_tx_queue = virt_queue = 0;
          sw_tx_queue_table != 0;
          sw_tx_queue_table >>= 1, sw_tx_queue++ )
        if ( (sw_tx_queue_table & 0x01) )
        {
            if ( !f_got_wtx_queue_config )
            {
                wtx_queue_config = *WTX_QUEUE_CONFIG(sw_tx_queue);
                f_got_wtx_queue_config = 1;
            }

            sw_tx_queue_to_virt_queue_map[sw_tx_queue] = virt_queue++;
            WTX_QUEUE_CONFIG(sw_tx_queue)->same_vc_qmap = new_sw_tx_queue_table & ~(1 << sw_tx_queue);
        }

    wtx_queue_config.same_vc_qmap = new_sw_tx_queue_table & ~(1 << new_sw_tx_queue);
    *WTX_QUEUE_CONFIG(new_sw_tx_queue) = wtx_queue_config;

    for ( num_of_virt_queue = 0, sw_tx_queue_table = new_sw_tx_queue_table, sw_tx_queue = 0;
          num_of_virt_queue < ARRAY_SIZE(virt_queue_to_sw_tx_queue_map) && sw_tx_queue_table != 0;
          sw_tx_queue_table >>= 1, sw_tx_queue++ )
        if ( (sw_tx_queue_table & 0x01) )
            virt_queue_to_sw_tx_queue_map[num_of_virt_queue++] = sw_tx_queue;

    g_atm_priv_data.sw_tx_queue_table |= 1 << new_sw_tx_queue;
    g_atm_priv_data.connection[conn].sw_tx_queue_table = new_sw_tx_queue_table;

    for ( i = 0; i < 8; i++ )
    {
        sw_tx_queue = g_atm_priv_data.connection[conn].prio_queue_map[i];
        virt_queue = sw_tx_queue_to_virt_queue_map[sw_tx_queue];
        sw_tx_queue = virt_queue_to_sw_tx_queue_map[virt_queue];
        g_atm_priv_data.connection[conn].prio_queue_map[i] = sw_tx_queue;
    }

    ret = 0;

SW_TX_QUEUE_ADD_EXIT:
    return ret;
}

//  delete lowest priority queue
static int sw_tx_queue_del(int conn)
{
    int ret;
    int rem_sw_tx_queue;
    u32 new_sw_tx_queue_table;
    u32 sw_tx_queue_table;
    int sw_tx_queue;
    int i;

    for ( sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, rem_sw_tx_queue = atm_txq_num() - 1;
          (sw_tx_queue_table & (1 << (atm_txq_num() - 1))) == 0;
          sw_tx_queue_table <<= 1, rem_sw_tx_queue-- );

    if ( g_atm_priv_data.connection[conn].sw_tx_queue_table == (1 << rem_sw_tx_queue) )
    {
        ret = -EIO;
        goto SW_TX_QUEUE_DEL_EXIT;
    }

    new_sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table & ~(1 << rem_sw_tx_queue);

    for ( sw_tx_queue_table = new_sw_tx_queue_table, sw_tx_queue = atm_txq_num() - 1;
          (sw_tx_queue_table & (1 << (atm_txq_num() - 1))) == 0;
          sw_tx_queue_table <<= 1, sw_tx_queue-- );

    for ( i = 0; i < 8; i++ )
        if ( g_atm_priv_data.connection[conn].prio_queue_map[i] == rem_sw_tx_queue )
            g_atm_priv_data.connection[conn].prio_queue_map[i] = sw_tx_queue;

    g_atm_priv_data.sw_tx_queue_table &= ~(1 << rem_sw_tx_queue);
    g_atm_priv_data.connection[conn].sw_tx_queue_table = new_sw_tx_queue_table;

    for ( sw_tx_queue_table = new_sw_tx_queue_table, sw_tx_queue = 0;
          sw_tx_queue_table != 0;
          sw_tx_queue_table >>= 1, sw_tx_queue++ )
        if ( (sw_tx_queue_table & 0x01) )
            WTX_QUEUE_CONFIG(sw_tx_queue)->same_vc_qmap = new_sw_tx_queue_table & ~(1 << sw_tx_queue);

    ret = 0;

SW_TX_QUEUE_DEL_EXIT:
    return ret;
}

static int sw_tx_prio_to_queue(int conn, int prio, int queue, int *p_num_of_virt_queue)
{
    int virt_queue_to_sw_tx_queue_map[8] = {0};
    int num_of_virt_queue;
    u32 sw_tx_queue_table;
    int sw_tx_queue;

    if ( conn < 0 )
        return -EINVAL;

    for ( num_of_virt_queue = 0, sw_tx_queue_table = g_atm_priv_data.connection[conn].sw_tx_queue_table, sw_tx_queue = 0;
          num_of_virt_queue < ARRAY_SIZE(virt_queue_to_sw_tx_queue_map) && sw_tx_queue_table != 0;
          sw_tx_queue_table >>= 1, sw_tx_queue++ )
        if ( (sw_tx_queue_table & 0x01) )
            virt_queue_to_sw_tx_queue_map[num_of_virt_queue++] = sw_tx_queue;

    if ( p_num_of_virt_queue != NULL )
        *p_num_of_virt_queue = num_of_virt_queue;

    if ( prio < 0 || queue < 0 || queue >= num_of_virt_queue )
        return -EINVAL;

    g_atm_priv_data.connection[conn].prio_queue_map[prio] = virt_queue_to_sw_tx_queue_map[queue];

    return 0;
}

/*
 *  Description:
 *    Setup QSB queue.
 *  Input:
 *    vcc        --- struct atm_vcc *, structure of an opened connection
 *    qos        --- struct atm_qos *, QoS parameter of the connection
 *    connection --- unsigned int, QSB queue ID, which is same as connection ID
 *  Output:
 *    none
 */
static void set_qsb(struct atm_vcc *vcc, struct atm_qos *qos, unsigned int connection)
{
    unsigned int qsb_clk = get_qsb_clk();
    union qsb_queue_parameter_table qsb_queue_parameter_table = {{0}};
    union qsb_queue_vbr_parameter_table qsb_queue_vbr_parameter_table = {{0}};
    unsigned int tmp;

    connection += QSB_QUEUE_NUMBER_BASE;    //  qsb qid = firmware qid + 1

#if defined(DEBUG_QOS) && DEBUG_QOS
    if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
    {
        static char *str_traffic_class[9] = {
            "ATM_NONE",
            "ATM_UBR",
            "ATM_CBR",
            "ATM_VBR",
            "ATM_ABR",
            "ATM_ANYCLASS",
            "ATM_VBR_RT",
            "ATM_UBR_PLUS",
            "ATM_MAX_PCR"
        };

        unsigned char traffic_class = qos->txtp.traffic_class;
        int max_pcr = qos->txtp.max_pcr;
        int pcr = qos->txtp.pcr;
        int min_pcr = qos->txtp.min_pcr;
  #if !defined(DISABLE_VBR) || !DISABLE_VBR
        int scr = qos->txtp.scr;
        int mbs = qos->txtp.mbs;
        int cdv = qos->txtp.cdv;
  #endif

        printk("TX QoS\n");

        printk("  traffic class : ");
        if ( traffic_class == (unsigned char)ATM_MAX_PCR )
            printk("ATM_MAX_PCR\n");
        else if ( traffic_class > ATM_UBR_PLUS )
            printk("Unknown Class\n");
        else
            printk("%s\n", str_traffic_class[traffic_class]);

        printk("  max pcr       : %d\n", max_pcr);
        printk("  desired pcr   : %d\n", pcr);
        printk("  min pcr       : %d\n", min_pcr);

  #if !defined(DISABLE_VBR) || !DISABLE_VBR
        printk("  sustained rate: %d\n", scr);
        printk("  max burst size: %d\n", mbs);
        printk("  cell delay var: %d\n", cdv);
  #endif
    }
#endif    //  defined(DEBUG_QOS) && DEBUG_QOS

    /*
     *  Peak Cell Rate (PCR) Limiter
     */
    if ( qos->txtp.max_pcr == 0 )
        qsb_queue_parameter_table.bit.tp = 0;   /*  disable PCR limiter */
    else
    {
        /*  peak cell rate would be slightly lower than requested [maximum_rate / pcr = (qsb_clock / 8) * (time_step / 4) / pcr] */
        tmp = ((qsb_clk * qsb_tstep) >> 5) / qos->txtp.max_pcr + 1;
        /*  check if overflow takes place   */
        qsb_queue_parameter_table.bit.tp = tmp > QSB_TP_TS_MAX ? QSB_TP_TS_MAX : tmp;
    }

    //  A funny issue. Create two PVCs, one UBR and one UBR with max_pcr.
    //  Send packets to these two PVCs at same time, it trigger strange behavior.
    //  In A1, RAM from 0x80000000 to 0x0x8007FFFF was corrupted with fixed pattern 0x00000000 0x40000000.
    //  In A4, PPE firmware keep emiting unknown cell and do not respond to driver.
    //  To work around, create UBR always with max_pcr.
    //  If user want to create UBR without max_pcr, we give a default one larger than line-rate.
    if ( qos->txtp.traffic_class == ATM_UBR && qsb_queue_parameter_table.bit.tp == 0 )
    {
        int port = g_atm_priv_data.connection[connection - QSB_QUEUE_NUMBER_BASE].port;
        unsigned int max_pcr = g_atm_priv_data.port[port].tx_max_cell_rate + 1000;

        tmp = ((qsb_clk * qsb_tstep) >> 5) / max_pcr + 1;
        if ( tmp > QSB_TP_TS_MAX )
            tmp = QSB_TP_TS_MAX;
        else if ( tmp < 1 )
            tmp = 1;
        qsb_queue_parameter_table.bit.tp = tmp;
    }

    /*
     *  Weighted Fair Queueing Factor (WFQF)
     */
    switch ( qos->txtp.traffic_class )
    {
    case ATM_CBR:
    case ATM_VBR_RT:
        /*  real time queue gets weighted fair queueing bypass  */
        qsb_queue_parameter_table.bit.wfqf = 0;
        break;
    case ATM_VBR_NRT:
    case ATM_UBR_PLUS:
        /*  WFQF calculation here is based on virtual cell rates, to reduce granularity for high rates  */
        /*  WFQF is maximum cell rate / garenteed cell rate                                             */
        /*  wfqf = qsb_minimum_cell_rate * QSB_WFQ_NONUBR_MAX / requested_minimum_peak_cell_rate        */
        if ( qos->txtp.min_pcr == 0 )
            qsb_queue_parameter_table.bit.wfqf = QSB_WFQ_NONUBR_MAX;
        else
        {
            tmp = QSB_GCR_MIN * QSB_WFQ_NONUBR_MAX / qos->txtp.min_pcr;
            if ( tmp == 0 )
                qsb_queue_parameter_table.bit.wfqf = 1;
            else if ( tmp > QSB_WFQ_NONUBR_MAX )
                qsb_queue_parameter_table.bit.wfqf = QSB_WFQ_NONUBR_MAX;
            else
                qsb_queue_parameter_table.bit.wfqf = tmp;
        }
        break;
    default:
    case ATM_UBR:
        qsb_queue_parameter_table.bit.wfqf = QSB_WFQ_UBR_BYPASS;
    }

    /*
     *  Sustained Cell Rate (SCR) Leaky Bucket Shaper VBR.0/VBR.1
     */
    if ( qos->txtp.traffic_class == ATM_VBR_RT || qos->txtp.traffic_class == ATM_VBR_NRT )
    {
#if defined(DISABLE_VBR) && DISABLE_VBR
        /*  disable shaper  */
        qsb_queue_vbr_parameter_table.bit.taus = 0;
        qsb_queue_vbr_parameter_table.bit.ts = 0;
#else
        if ( qos->txtp.scr == 0 )
        {
            /*  disable shaper  */
            qsb_queue_vbr_parameter_table.bit.taus = 0;
            qsb_queue_vbr_parameter_table.bit.ts = 0;
        }
        else
        {
            /*  Cell Loss Priority  (CLP)   */
            if ( (vcc->atm_options & ATM_ATMOPT_CLP) )
                /*  CLP1    */
                qsb_queue_parameter_table.bit.vbr = 1;
            else
                /*  CLP0    */
                qsb_queue_parameter_table.bit.vbr = 0;
            /*  Rate Shaper Parameter (TS) and Burst Tolerance Parameter for SCR (tauS) */
            tmp = ((qsb_clk * qsb_tstep) >> 5) / qos->txtp.scr + 1;
            qsb_queue_vbr_parameter_table.bit.ts = tmp > QSB_TP_TS_MAX ? QSB_TP_TS_MAX : tmp;
            tmp = (qos->txtp.mbs - 1) * (qsb_queue_vbr_parameter_table.bit.ts - qsb_queue_parameter_table.bit.tp) / 64;
            if ( tmp == 0 )
                qsb_queue_vbr_parameter_table.bit.taus = 1;
            else if ( tmp > QSB_TAUS_MAX )
                qsb_queue_vbr_parameter_table.bit.taus = QSB_TAUS_MAX;
            else
                qsb_queue_vbr_parameter_table.bit.taus = tmp;
        }
#endif
    }
    else
    {
        qsb_queue_vbr_parameter_table.bit.taus = 0;
        qsb_queue_vbr_parameter_table.bit.ts = 0;
    }

    /*  Queue Parameter Table (QPT) */
    *QSB_RTM   = QSB_RTM_DM_SET(QSB_QPT_SET_MASK);
    *QSB_RTD   = QSB_RTD_TTV_SET(qsb_queue_parameter_table.dword);
    *QSB_RAMAC = QSB_RAMAC_RW_SET(QSB_RAMAC_RW_WRITE) | QSB_RAMAC_TSEL_SET(QSB_RAMAC_TSEL_QPT) | QSB_RAMAC_LH_SET(QSB_RAMAC_LH_LOW) | QSB_RAMAC_TESEL_SET(connection);
#if defined(DEBUG_QOS) && DEBUG_QOS
    if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
        printk("QPT: QSB_RTM (%08X) = 0x%08X, QSB_RTD (%08X) = 0x%08X, QSB_RAMAC (%08X) = 0x%08X\n", (unsigned int)QSB_RTM, *QSB_RTM, (unsigned int)QSB_RTD, *QSB_RTD, (unsigned int)QSB_RAMAC, *QSB_RAMAC);
#endif
    /*  Queue VBR Paramter Table (QVPT) */
    *QSB_RTM   = QSB_RTM_DM_SET(QSB_QVPT_SET_MASK);
    *QSB_RTD   = QSB_RTD_TTV_SET(qsb_queue_vbr_parameter_table.dword);
    *QSB_RAMAC = QSB_RAMAC_RW_SET(QSB_RAMAC_RW_WRITE) | QSB_RAMAC_TSEL_SET(QSB_RAMAC_TSEL_VBR) | QSB_RAMAC_LH_SET(QSB_RAMAC_LH_LOW) | QSB_RAMAC_TESEL_SET(connection);
#if defined(DEBUG_QOS) && DEBUG_QOS
    if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
        printk("QVPT: QSB_RTM (%08X) = 0x%08X, QSB_RTD (%08X) = 0x%08X, QSB_RAMAC (%08X) = 0x%08X\n", (unsigned int)QSB_RTM, *QSB_RTM, (unsigned int)QSB_RTD, *QSB_RTD, (unsigned int)QSB_RAMAC, *QSB_RAMAC);
#endif

#if defined(DEBUG_QOS) && DEBUG_QOS
    if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
    {
        printk("set_qsb\n");
        printk("  qsb_clk = %lu\n", (unsigned long)qsb_clk);
        printk("  qsb_queue_parameter_table.bit.tp       = %d\n", (int)qsb_queue_parameter_table.bit.tp);
        printk("  qsb_queue_parameter_table.bit.wfqf     = %d (0x%08X)\n", (int)qsb_queue_parameter_table.bit.wfqf, (int)qsb_queue_parameter_table.bit.wfqf);
        printk("  qsb_queue_parameter_table.bit.vbr      = %d\n", (int)qsb_queue_parameter_table.bit.vbr);
        printk("  qsb_queue_parameter_table.dword        = 0x%08X\n", (int)qsb_queue_parameter_table.dword);
        printk("  qsb_queue_vbr_parameter_table.bit.ts   = %d\n", (int)qsb_queue_vbr_parameter_table.bit.ts);
        printk("  qsb_queue_vbr_parameter_table.bit.taus = %d\n", (int)qsb_queue_vbr_parameter_table.bit.taus);
        printk("  qsb_queue_vbr_parameter_table.dword    = 0x%08X\n", (int)qsb_queue_vbr_parameter_table.dword);
    }
#endif
}

/*
 *  Description:
 *    Add one entry to HTU table.
 *  Input:
 *    vpi        --- unsigned int, virtual path ID
 *    vci        --- unsigned int, virtual channel ID
 *    connection --- unsigned int, connection ID
 *    aal5       --- int, 0 means AAL0, else means AAL5
 *  Output:
 *    none
 */
static void set_htu_entry(unsigned int vpi, unsigned int vci, unsigned int conn, int aal5)
{
    struct htu_entry htu_entry = {  res1:       0x00,
                                    pid:        g_atm_priv_data.connection[conn].port & 0x01,
                                    vpi:        vpi,
                                    vci:        vci,
                                    pti:        0x00,
                                    vld:        0x01};

    struct htu_mask htu_mask = {    set:        0x03,
                                    pid_mask:   0x02,
                                    vpi_mask:   0x00,
                                    vci_mask:   0x0000,
                                    pti_mask:   0x03,   //  0xx, user data
                                    clear:      0x00};

    struct htu_result htu_result = {res1:       0x00,
                                    cellid:     conn,
                                    res2:       0x00,
                                    type:       aal5 ? 0x00 : 0x01,
                                    ven:        0x01,
                                    res3:       0x00,
                                    qid:        conn};

    *HTU_RESULT(conn + OAM_HTU_ENTRY_NUMBER) = htu_result;
    *HTU_MASK(conn + OAM_HTU_ENTRY_NUMBER)   = htu_mask;
    *HTU_ENTRY(conn + OAM_HTU_ENTRY_NUMBER)  = htu_entry;
}

/*
 *  Description:
 *    Remove one entry from HTU table.
 *  Input:
 *    conn --- unsigned int, connection ID
 *  Output:
 *    none
 */
static void clear_htu_entry(unsigned int conn)
{
    HTU_ENTRY(conn + OAM_HTU_ENTRY_NUMBER)->vld = 0;
}

static unsigned int get_qsb_clk(void)
{
    unsigned int fpi_dvsn;
    unsigned int freq;

    fpi_dvsn = (*CGU_CLKFSR >> 28) & 0x03;
    freq = (*CGU_CLKFSR >> 16) & 0x03;

    if(freq == 2){ //freq 288 Mhz
        freq = 288000000 >> fpi_dvsn;
    }else if(freq == 1){ // freq 432 Mhz
        freq = 432000000 >> fpi_dvsn;
    }else { //default: use 288 Mhz
        freq = 288000000 >> fpi_dvsn;
    }
    return freq;
}

/*
 *  Description:
 *    Loop up for connection ID with virtual path ID.
 *  Input:
 *    vpi --- unsigned int, virtual path ID
 *  Output:
 *    int --- negative value: failed
 *            else          : connection ID
 */
static int find_vpi(unsigned int vpi)
{
    int i;
    struct atm_pvc *connection = g_atm_priv_data.connection;

    for ( i = 0; i < atm_pvc_num(); i++ )
        if ( (g_atm_priv_data.pvc_table & (1 << i))
            && connection[i].vcc != NULL
            && vpi == connection[i].vcc->vpi )
            return i;
    return -1;
}

/*
 *  Description:
 *    Setup QSB.
 *  Input:
 *    none
 *  Output:
 *    none
 */
static void qsb_global_set(void)
{
    u32 qsb_clk = get_qsb_clk();
    int i;
    u32 tmp1, tmp2, tmp3;

    *QSB_ICDV = QSB_ICDV_TAU_SET(qsb_tau);
    *QSB_SBL  = QSB_SBL_SBL_SET(qsb_srvm);
    *QSB_CFG  = QSB_CFG_TSTEPC_SET(qsb_tstep >> 1);
#if defined(DEBUG_QOS) && DEBUG_QOS
    if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
    {
        printk("qsb_clk = %u\n", qsb_clk);
        printk("QSB_ICDV (%08X) = %d (%d), QSB_SBL (%08X) = %d (%d), QSB_CFG (%08X) = %d (%d)\n", (u32)QSB_ICDV, *QSB_ICDV, QSB_ICDV_TAU_SET(qsb_tau), (u32)QSB_SBL, *QSB_SBL, QSB_SBL_SBL_SET(qsb_srvm), (u32)QSB_CFG, *QSB_CFG, QSB_CFG_TSTEPC_SET(qsb_tstep >> 1));
    }
#endif

    /*
     *  set SCT and SPT per port
     */
    for ( i = 0; i < ATM_PORT_NUMBER; i++ )
        if ( g_atm_priv_data.port[i].tx_max_cell_rate != 0 )
        {
            tmp1 = ((qsb_clk * qsb_tstep) >> 1) / g_atm_priv_data.port[i].tx_max_cell_rate;
            tmp2 = tmp1 >> 6;                   /*  integer value of Tsb    */
            tmp3 = (tmp1 & ((1 << 6) - 1)) + 1; /*  fractional part of Tsb  */
            /*  carry over to integer part (?)  */
            if ( tmp3 == (1 << 6) )
            {
                tmp3 = 0;
                tmp2++;
            }
            if ( tmp2 == 0 )
                tmp2 = tmp3 = 1;
            /*  1. set mask                                 */
            /*  2. write value to data transfer register    */
            /*  3. start the tranfer                        */
            /*  SCT (FracRate)  */
            *QSB_RTM   = QSB_RTM_DM_SET(QSB_SET_SCT_MASK);
            *QSB_RTD   = QSB_RTD_TTV_SET(tmp3);
            *QSB_RAMAC = QSB_RAMAC_RW_SET(QSB_RAMAC_RW_WRITE) | QSB_RAMAC_TSEL_SET(QSB_RAMAC_TSEL_SCT) | QSB_RAMAC_LH_SET(QSB_RAMAC_LH_LOW) | QSB_RAMAC_TESEL_SET(i & 0x01);
#if defined(DEBUG_QOS) && DEBUG_QOS
            if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
                printk("SCT: QSB_RTM (%08X) = 0x%08X, QSB_RTD (%08X) = 0x%08X, QSB_RAMAC (%08X) = 0x%08X\n", (u32)QSB_RTM, *QSB_RTM, (u32)QSB_RTD, *QSB_RTD, (u32)QSB_RAMAC, *QSB_RAMAC);
#endif
            /*  SPT (SBV + PN + IntRage)    */
            *QSB_RTM   = QSB_RTM_DM_SET(QSB_SET_SPT_MASK);
            *QSB_RTD   = QSB_RTD_TTV_SET(QSB_SPT_SBV_VALID | QSB_SPT_PN_SET(i & 0x01) | QSB_SPT_INTRATE_SET(tmp2));
            *QSB_RAMAC = QSB_RAMAC_RW_SET(QSB_RAMAC_RW_WRITE) | QSB_RAMAC_TSEL_SET(QSB_RAMAC_TSEL_SPT) | QSB_RAMAC_LH_SET(QSB_RAMAC_LH_LOW) | QSB_RAMAC_TESEL_SET(i & 0x01);
#if defined(DEBUG_QOS) && DEBUG_QOS
            if ( (g_dbg_enable & DBG_ENABLE_MASK_DUMP_QOS) )
                printk("SPT: QSB_RTM (%08X) = 0x%08X, QSB_RTD (%08X) = 0x%08X, QSB_RAMAC (%08X) = 0x%08X\n", (u32)QSB_RTM, *QSB_RTM, (u32)QSB_RTD, *QSB_RTD, (u32)QSB_RAMAC, *QSB_RAMAC);
#endif
        }
}

static void setup_oam_htu_entry(void)
{
    struct htu_entry htu_entry = {0};
    struct htu_result htu_result = {0};
    struct htu_mask htu_mask = {    set:        0x03,
                                    pid_mask:   0x00,
                                    vpi_mask:   0x00,
                                    vci_mask:   0x00,
                                    pti_mask:   0x00,
                                    clear:      0x00};
    int ven = 1;
    int i;

#ifdef CONFIG_VRX218_DSL_DFE_LOOPBACK
    ven = 0;
#endif

    /*
     *  HTU Tables
     */
    for ( i = 0; i < atm_pvc_num(); i++ )
    {
        htu_result.qid = (unsigned int)i;

        *HTU_ENTRY(i + OAM_HTU_ENTRY_NUMBER)  = htu_entry;
        *HTU_MASK(i + OAM_HTU_ENTRY_NUMBER)   = htu_mask;
        *HTU_RESULT(i + OAM_HTU_ENTRY_NUMBER) = htu_result;
    }
    /*  OAM HTU Entry   */
    htu_entry.vci     = 0x03;
    htu_mask.pid_mask = 0x03;
    htu_mask.vpi_mask = 0xFF;
    htu_mask.vci_mask = 0x0000;
    htu_mask.pti_mask = 0x07;
    htu_result.cellid = 0;  //  need confirm
    htu_result.type   = 1;
    htu_result.ven    = ven;
    htu_result.qid    = 0;  //  need confirm
    *HTU_RESULT(OAM_F4_SEG_HTU_ENTRY) = htu_result;
    *HTU_MASK(OAM_F4_SEG_HTU_ENTRY)   = htu_mask;
    *HTU_ENTRY(OAM_F4_SEG_HTU_ENTRY)  = htu_entry;
    htu_entry.vci     = 0x04;
    htu_result.cellid = 0;  //  need confirm
    htu_result.type   = 1;
    htu_result.ven    = ven;
    htu_result.qid    = 0;  //  need confirm
    *HTU_RESULT(OAM_F4_TOT_HTU_ENTRY) = htu_result;
    *HTU_MASK(OAM_F4_TOT_HTU_ENTRY)   = htu_mask;
    *HTU_ENTRY(OAM_F4_TOT_HTU_ENTRY)  = htu_entry;
    htu_entry.vci     = 0x00;
    htu_entry.pti     = 0x04;
    htu_mask.vci_mask = 0xFFFF;
    htu_mask.pti_mask = 0x01;
    htu_result.cellid = 0;  //  need confirm
    htu_result.type   = 1;
    htu_result.ven    = ven;
    htu_result.qid    = 0;  //  need confirm
    *HTU_RESULT(OAM_F5_HTU_ENTRY) = htu_result;
    *HTU_MASK(OAM_F5_HTU_ENTRY)   = htu_mask;
    *HTU_ENTRY(OAM_F5_HTU_ENTRY)  = htu_entry;
}

/*
 *  Description:
 *    Add HTU entries to capture OAM cell.
 *  Input:
 *    none
 *  Output:
 *    none
 */
static void validate_oam_htu_entry(void)
{
    HTU_ENTRY(OAM_F4_SEG_HTU_ENTRY)->vld = 1;
    HTU_ENTRY(OAM_F4_TOT_HTU_ENTRY)->vld = 1;
    HTU_ENTRY(OAM_F5_HTU_ENTRY)->vld = 1;
}

/*
 *  Description:
 *    Remove HTU entries which are used to capture OAM cell.
 *  Input:
 *    none
 *  Output:
 *    none
 */
static void invalidate_oam_htu_entry(void)
{
    HTU_ENTRY(OAM_F4_SEG_HTU_ENTRY)->vld = 0;
    HTU_ENTRY(OAM_F4_TOT_HTU_ENTRY)->vld = 0;
    HTU_ENTRY(OAM_F5_HTU_ENTRY)->vld = 0;
    /*  idle for a while to finish running HTU search   */
    udelay(10);
}

/*
 *  Description:
 *    Loop up for connection ID with virtual path ID and virtual channel ID.
 *  Input:
 *    vpi --- unsigned int, virtual path ID
 *    vci --- unsigned int, virtual channel ID
 *  Output:
 *    int --- negative value: failed
 *            else          : connection ID
 */
static int find_vpivci(unsigned int vpi, unsigned int vci)
{
    int i;
    struct atm_pvc *connection = g_atm_priv_data.connection;

    for ( i = 0; i < atm_pvc_num(); i++ ) {
        if ( (g_atm_priv_data.pvc_table & (1 << i))
            && connection[i].vcc != NULL
            && vpi == connection[i].vcc->vpi
            && vci == connection[i].vcc->vci )
            return i;
    }
    return -1;
}

/*
 *  Description:
 *    Loop up for connection ID with atm_vcc structure.
 *  Input:
 *    vcc --- struct atm_vcc *, atm_vcc structure of opened connection
 *  Output:
 *    int --- negative value: failed
 *            else          : connection ID
 */
static int find_vcc(struct atm_vcc *vcc)
{
    int i;
    struct atm_pvc *connection = g_atm_priv_data.connection;

    for ( i = 0; i < atm_pvc_num(); i++ )
        if ( (g_atm_priv_data.pvc_table & (1 << i))
            && connection[i].vcc == vcc )
            return i;
    return -1;
}

#if 0
static struct sk_buff *alloc_skb_rx(void)
{
    return alloc_skb_tx(DMA_PACKET_SIZE);
}
#endif

static struct sk_buff *alloc_skb_tx_aligned(struct sk_buff *skb, int len)
{
    if ( skb )
        dev_kfree_skb_any(skb);

    skb = dev_alloc_skb(len + DMA_ALIGNMENT * 2);
    if ( skb )
    {
        skb_reserve(skb, (~((unsigned int)skb->data + (DMA_ALIGNMENT - 1)) & (DMA_ALIGNMENT - 1)) + DMA_ALIGNMENT);
        ASSERT(((unsigned int)skb->data & (DMA_ALIGNMENT - 1)) == 0, "skb->data (%#x) is not 8 DWORDS aligned", (unsigned int)skb->data);
    }

    return skb;
}

static struct sk_buff *alloc_skb_tx(int len)
{
    struct sk_buff *skb;

    len = (len + DMA_ALIGNMENT - 1) & ~(DMA_ALIGNMENT - 1);

    skb = dev_alloc_skb(len);
    if ( skb )
    {
        if ( ((unsigned int)skb->data & (DMA_ALIGNMENT - 1)) != 0 && !(skb = alloc_skb_tx_aligned(skb, len)) )
            return NULL;
        *((unsigned int *)skb->data - 1) = (unsigned int)skb;
#ifndef CONFIG_MIPS_UNCACHED
        dma_cache_wback((unsigned int)skb->data - sizeof(unsigned int), sizeof(unsigned int));
#endif
        ATM_SKB(skb)->vcc = NULL;
    }

    return skb;
}

static struct sk_buff* skb_break_away_from_protocol(struct sk_buff *skb)
{
    struct sk_buff *new_skb;

    if ( skb_shared(skb) ) {
        new_skb = skb_clone(skb, GFP_ATOMIC);
        if ( new_skb == NULL )
            return NULL;
    }
    else
        new_skb = skb_get(skb);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dst_release(new_skb->dst);
    new_skb->dst = NULL;
#else
    skb_dst_drop(new_skb);
#endif
#ifdef CONFIG_XFRM
    secpath_put(new_skb->sp);
    new_skb->sp = NULL;
#endif
    if ( new_skb->destructor )
    {
        WARN_ON(in_irq());
        new_skb->destructor(new_skb);
        new_skb->destructor = NULL;
    }
#if defined(CONFIG_NETFILTER) || LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
    nf_conntrack_put(new_skb->nfct);
    new_skb->nfct = NULL;
  #if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
    nf_conntrack_put_reasm(new_skb->nfct_reasm);
    new_skb->nfct_reasm = NULL;
  #endif
  #ifdef CONFIG_BRIDGE_NETFILTER
    nf_bridge_put(new_skb->nf_bridge);
    new_skb->nf_bridge = NULL;
  #endif
#endif

    return new_skb;
}

static struct sk_buff* atm_alloc_tx(struct atm_vcc *vcc, unsigned int size)
{
    struct sk_buff *skb;

    /*  send buffer overflow    */
    if ( atomic_read(&sk_atm(vcc)->sk_wmem_alloc) && !atm_may_send(vcc, size) ) {
        err("send buffer overflow");
        return NULL;
    }

    skb = alloc_skb_tx(size < DMA_PACKET_SIZE ? DMA_PACKET_SIZE : size);
    if ( skb == NULL ) {
        err("sk buffer is used up");
        return NULL;
    }

    atomic_add(skb->truesize, &sk_atm(vcc)->sk_wmem_alloc);

    return skb;
}

static void atm_free_tx_skb_vcc(struct sk_buff *skb)
{
    struct atm_vcc* vcc;

    ASSERT((unsigned int)skb > 0x80000000, "atm_free_tx_skb_vcc: skb = %08X", (unsigned int)skb);

    vcc = ATM_SKB(skb)->vcc;

    if ( vcc != NULL && vcc->pop != NULL )
    {
        ASSERT(atomic_read(&skb->users) != 0, "atm_free_tx_skb_vcc(vcc->pop): skb->users == 0, skb = %08X", (unsigned int)skb);
        vcc->pop(vcc, skb);
    }
    else
    {
        ASSERT(atomic_read(&skb->users) != 0, "atm_free_tx_skb_vcc(dev_kfree_skb_any): skb->users == 0, skb = %08X", (unsigned int)skb);
        dev_kfree_skb_any(skb);
    }
}

static irqreturn_t mailbox_irq_handler(int irq, void *dev_id)
{
    u32 mailbox_signal = 0;

    spin_lock(&g_mailbox_lock);
    mailbox_signal = *MBOX_IGU1_ISR & *MBOX_IGU1_IER;
    *MBOX_IGU1_ISRC = mailbox_signal & ~0x02;
    spin_unlock(&g_mailbox_lock);

    if ( (mailbox_signal & 0x01) ) {
        spin_lock(&g_mailbox_lock);
        *MBOX_IGU1_IER &= ~0x01;
        spin_unlock(&g_mailbox_lock);
        enable_vrx218_dma_tx(1);
    }

    if ( (mailbox_signal & 0x02) ) {
        spin_lock(&g_mailbox_lock);
        *MBOX_IGU1_IER &= ~0x02;
        spin_unlock(&g_mailbox_lock);
        tasklet_schedule(&g_oam_tasklet);
    }

    if ( (mailbox_signal & 0x04) ) {
        /* TODO: swap queue */
    }

    if ( (mailbox_signal & 0x08) ) {//EDMA hang
        g_pcie_reset_sig = 1;
    }

    /* Clear TX interrupt at this moment.
     * Implement flow control mechansim if there is specific requirement.
     */
    spin_lock(&g_mailbox_lock);
    mailbox_signal = *MBOX_IGU0_ISR & *MBOX_IGU0_IER;
    *MBOX_IGU0_ISRC = mailbox_signal;
    spin_unlock(&g_mailbox_lock);

    return IRQ_HANDLED;
}

static void dump_skb(struct sk_buff *skb, unsigned int len, char *title, int port, int ch, int is_tx, int is_force)
{
#if defined(DEBUG_DUMP_SKB) && DEBUG_DUMP_SKB
    int i;

    if ( !is_force && !(g_dbg_enable & (is_tx ? DBG_ENABLE_MASK_DUMP_SKB_TX : DBG_ENABLE_MASK_DUMP_SKB_RX)) )
        return;

    if ( skb->len < len )
        len = skb->len;

    if ( len > DMA_PACKET_SIZE )
    {
        printk("too big data length: skb = %08x, skb->data = %08x, skb->len = %d\n", (unsigned int)skb, (unsigned int)skb->data, skb->len);
        return;
    }

    if ( ch >= 0 )
        printk("%s (port %d, ch %d)\n", title, port, ch);
    else
        printk("%s\n", title);
    printk("  skb->data = %08X, skb->tail = %08X, skb->len = %d\n", (unsigned int)skb->data, (unsigned int)skb->tail, (int)skb->len);
    for ( i = 1; i <= len; i++ )
    {
        if ( i % 16 == 1 )
            printk("  %4d:", i - 1);
        printk(" %02X", (int)(*((char*)skb->data + i - 1) & 0xFF));
        if ( i % 16 == 0 )
            printk("\n");
    }
    if ( (i - 1) % 16 != 0 )
        printk("\n");
#endif
}

static int swap_mac(unsigned char *data)
{
#if defined(DEBUG_SWAP_MAC) && DEBUG_SWAP_MAC
    int ret = 0;

    if ( (g_dbg_enable & DBG_ENABLE_MASK_MAC_SWAP) )
    {
        unsigned char tmp[8];
        unsigned char *p = data;

        if ( p[12] == 0x08 && p[13] == 0x06 ) { //  arp
            if ( p[14] == 0x00 && p[15] == 0x01 && p[16] == 0x08 && p[17] == 0x00 && p[20] == 0x00 && p[21] == 0x01 ) {
                //  dest mac
                memcpy(p, p + 6, 6);
                //  src mac
                p[6] = p[7] = 0;
                memcpy(p + 8, p + 38, 4);
                //  arp reply
                p[21] = 0x02;
                //  sender mac
                memcpy(p + 22, p + 6, 6);
                //  sender IP
                memcpy(tmp, p + 28, 4);
                memcpy(p + 28, p + 38, 4);
                //  target mac
                memcpy(p + 32, p, 6);
                //  target IP
                memcpy(p + 38, tmp, 4);

                ret = 42;
            }
        }
        else if ( !(p[0] & 0x01) ) {    //  bypass broadcast/multicast
            //  swap MAC
            memcpy(tmp, p, 6);
            memcpy(p, p + 6, 6);
            memcpy(p + 6, tmp, 6);
            p += 12;

            //  bypass VLAN
            while ( p[0] == 0x81 && p[1] == 0x00 )
                p += 4;

            //  IP
            if ( p[0] == 0x08 && p[1] == 0x00 ) {
                p += 14;
                memcpy(tmp, p, 4);
                memcpy(p, p + 4, 4);
                memcpy(p + 4, tmp, 4);
                p += 8;
            }

            ret = (int)((unsigned long)p - (unsigned long)data);
        }
    }

    return ret;
#else
    return 0;
#endif
}



int proc_read_prio(struct seq_file *seq, void *v)
{
    int len = 0;
    int i, j;

    //  skb->priority to firmware queue map (for LAN interface, QId is virtual one maitained by driver)
    seq_printf(seq,        "Priority to Queue Map:\n");
    seq_printf(seq,        "  prio\t\t:  0  1  2  3  4  5  6  7\n");
    for ( i = 0; i < atm_pvc_num(); i++ )
    {
        if ( g_atm_priv_data.connection[i].vcc != NULL )
        {
            int sw_tx_queue_to_virt_queue_map[ATM_SW_TX_QUEUE_NUMBER] = {0};
            u32 sw_tx_queue_table;
            int sw_tx_queue;
            int virt_queue;

            for ( sw_tx_queue_table = g_atm_priv_data.connection[i].sw_tx_queue_table, sw_tx_queue = virt_queue = 0;
                  sw_tx_queue_table != 0;
                  sw_tx_queue_table >>= 1, sw_tx_queue++ )
                if ( (sw_tx_queue_table & 0x01) )
                    sw_tx_queue_to_virt_queue_map[sw_tx_queue] = virt_queue++;

            seq_printf(seq,"  pvc %d.%d\t:", (int)g_atm_priv_data.connection[i].vcc->vpi, (int)g_atm_priv_data.connection[i].vcc->vci);
            for ( j = 0; j < 8; j++ )
                seq_printf(seq," %2d", sw_tx_queue_to_virt_queue_map[g_atm_priv_data.connection[i].prio_queue_map[j]]);
            seq_printf(seq,    "\n");
            seq_printf(seq,"      phys qid\t:");
            for ( j = 0; j < 8; j++ )
                seq_printf(seq," %2d", g_atm_priv_data.connection[i].prio_queue_map[j]);
            seq_printf(seq,    "\n");
        }
    }

    return len;
}

ssize_t proc_write_prio(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char local_buf[256];
    int len;
    char *p1, *p2;
    int colon = 1;
    int conn = -1;
    unsigned int vpi, vci;
    int prio = -1;
    int queue = -1;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( strcasecmp(p1, "help") == 0 )
        {
            printk("echo <pvc vpi.vci> prio xx queue xx [prio xx queue xx] > /proc/eth/vrx318/prio\n");
            printk("echo pvc vpi.vci <add/del> > /proc/eth/vrx318/prio\n");
            break;
        }
        else if ( strcasecmp(p1, "pvc") == 0 )
        {
            ignore_space(&p2, &len);
            vpi = get_number(&p2, &len, 0);
            ignore_space(&p2, &len);
            vci = get_number(&p2, &len, 0);
            conn = find_vpivci(vpi, vci);
            dbg("vpi = %u, vci = %u, conn = %d", vpi, vci, conn);
            prio = queue = -1;
        }
        else if ( conn >= 0 )
        {
            if ( strcasecmp(p1, "p") == 0 || strcasecmp(p1, "prio") == 0 )
            {
                ignore_space(&p2, &len);
                prio = get_number(&p2, &len, 0);
                dbg("prio = %d", prio);
                if ( conn >= 0 && prio >= 0 && prio < 8 )
                {
                    sw_tx_prio_to_queue(conn, prio, queue, NULL);
                }
                else
                {
                    err("prio (%d) is out of range 0 - %d", prio, 7);
                }
            }
            else if ( strcasecmp(p1, "q") == 0 || strcasecmp(p1, "queue") == 0 )
            {
                ignore_space(&p2, &len);
                queue = get_number(&p2, &len, 0);
                dbg("queue = %d", queue);
                if ( conn >= 0 )
                {
                    int num_of_virt_queue;

                    if ( sw_tx_prio_to_queue(conn, prio, queue, &num_of_virt_queue) != 0 )
                    {
                        err("queue (%d) is out of range 0 - %d", queue, num_of_virt_queue - 1);
                    }
                }
            }
            else if ( conn >= 0 && strcasecmp(p1, "add") == 0 )
            {
                int ret = sw_tx_queue_add(conn);

                if ( ret == 0 )
                {
                    dbg("add tx queue successfully");
                }
                else
                {
                    err("failed in adding tx queue: %d", ret);
                }
            }
            else if ( conn >= 0 && (strcasecmp(p1, "del") == 0 || strcasecmp(p1, "rem") == 0) )
            {
                int ret = sw_tx_queue_del(conn);

                if ( ret == 0 )
                {
                    dbg("remove tx queue successfully");
                }
                else
                {
                    err("failed in removing tx queue: %d", ret);
                }
            }
            else
            {
                err("unknown command (%s)", p1);
            }
        }
        else
        {
            err("unknown command (%s)", p1);
        }

        p1 = p2;
        colon = 1;
    }

    return count;
}

int proc_read_pcie_rst(struct seq_file *seq, void *v)
{
    int len = 0;

    seq_printf(seq, "%d\n", g_pcie_reset_sig);

    return len;
}

static int ppe_set_vlan(int conn, unsigned short vlan_tag)
{
    int candidate = -1;
    int i;

    for ( i = 0; i < NUM_ENTITY(g_dsl_vlan_qid_map); i += 2 )
    {
        if ( candidate < 0 && g_dsl_vlan_qid_map[i] == 0 )
            candidate = i;
        else if ( g_dsl_vlan_qid_map[i] == (unsigned short)vlan_tag )
        {
            if ( conn != g_dsl_vlan_qid_map[i + 1] )
            {
                err("VLAN tag 0x%04X is in use", vlan_tag);
                return -1;  //  quit (error), VLAN tag is used by other PVC
            }
            return 0;       //  quit (success), VLAN tag is assigned already
        }
    }
    //  candidate must be valid, because size of g_dsl_vlan_qid_map is equal to max number of PVCs
    g_dsl_vlan_qid_map[candidate + 1] = conn;
    g_dsl_vlan_qid_map[candidate] = vlan_tag;
    WRX_QUEUE_CONFIG(conn)->new_vlan = vlan_tag;

    return 0;
}

static int sw_set_vlan_bypass(int itf, unsigned int vlan_tag, int is_del)
{
    if ( itf == 1 )
    {
        err("does not support eth1 VLAN bypass!");
        return -1;
    }

    return 0;
}

int proc_read_dsl_vlan(struct seq_file *seq, void *v)
{
    int len = 0;
    int i;

    seq_printf(seq,     "PVC to DSL VLAN map:\n");
    for ( i = 0; i < atm_pvc_num(); i++ )
    {
        if ( HTU_ENTRY(i + OAM_HTU_ENTRY_NUMBER)->vld == 0 )
        {
            seq_printf(seq, "  %2d: invalid\n", i);
            continue;
        }

        seq_printf(seq, "  %2d: PVC %u.%u\tVLAN 0x%04X\n", i, (unsigned int)g_atm_priv_data.connection[i].vcc->vpi, (unsigned int)g_atm_priv_data.connection[i].vcc->vci, (unsigned int)WRX_QUEUE_CONFIG(i)->new_vlan);
    }
    seq_printf(seq,     "DSL VLAN to PVC map:\n");
    for ( i = 0; i < NUM_ENTITY(g_dsl_vlan_qid_map); i += 2 )
    {
        if ( g_dsl_vlan_qid_map[i] == 0 )
        {
            seq_printf(seq, "  %2d: invalid\n", i);
            continue;
        }
        seq_printf(seq, "  %2d: VLAN 0x%04X\tQID %d\n", i >> 1, (unsigned int)g_dsl_vlan_qid_map[i], (unsigned int)g_dsl_vlan_qid_map[i + 1]);
    }


    return len;
}

ssize_t proc_write_dsl_vlan(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char local_buf[128];
    int len;
    char *p1, *p2;
    int colon = 1;
    int conn = -1;
    unsigned int vpi, vci;
    unsigned int vlan_tag = ~0;
    int eth0 = 0, eth1 = 0;
    int bypass = 0, enable = 0;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) )
    {
        if ( strcasecmp(p1, "help") == 0 )
        {
            printk("echo <pvc vpi.vci> <vlan xx> [bypass [enable|disbable]] > /proc/eth/vrx318/dsl_vlan\n");
            printk("  vlan   - 16-bit vlan tag, including PCP, CFI, and VID.\n");
            printk("  bypass - traffic from ATM to MII0 is done by internal switch, no CPU involvement.\n");
            conn = -1;
            break;
        }
        else if ( bypass == 1 )
        {
            if ( strcasecmp(p1, "enable") == 0 )
                enable = 1;
            else if ( strcasecmp(p1, "disable") == 0 )
                enable = -1;
            else if ( strcasecmp(p1, "eth0") == 0 )
            {
                if ( eth0 == 0 )
                    eth0 = enable == 0 ? 1 : enable;
            }
            else if ( strcasecmp(p1, "eth1") == 0 )
            {
                if ( eth1 == 0 )
                    eth1 = enable == 0 ? 1 : enable;
            }
            else
            {
                printk("unknown command (%s)\n", p1);
                conn = -1;
                break;
            }
        }
        else
        {
            if ( bypass == 1 && eth0 == 0 )
                eth0 = enable >= 0 ? 1 : -1;

            bypass = 0;
            enable = 0;

            if ( strcasecmp(p1, "pvc") == 0 )
            {
                ignore_space(&p2, &len);
                vpi = get_number(&p2, &len, 0);
                ignore_space(&p2, &len);
                vci = get_number(&p2, &len, 0);
                conn = find_vpivci(vpi, vci);
                printk("vpi = %u, vci = %u, conn = %d\n", vpi, vci, conn);
            }
            else if ( vlan_tag == ~0 && strcasecmp(p1, "vlan") == 0 )
            {
                ignore_space(&p2, &len);
                vlan_tag = get_number(&p2, &len, 0) & 0xFFFF;
                printk("VLAN tag = 0x%04X\n", vlan_tag);
            }
            else if ( strcasecmp(p1, "bypass") == 0 )
                bypass = 1;
            else
            {
                printk("unknown command (%s)\n", p1);
                conn = -1;
                break;
            }
        }

        p1 = p2;
        colon = 1;
    }
    if ( bypass == 1 && eth0 == 0 )
        eth0 = enable >= 0 ? 1 : -1;

    if ( conn >= 0 && vlan_tag != ~0 )
    {
        if ( ppe_set_vlan(conn, (unsigned short)vlan_tag) != 0 )
            conn = -1;
    }
    if ( conn >= 0 )
    {
        vlan_tag = (unsigned int)WRX_QUEUE_CONFIG(conn)->new_vlan;
        if ( eth0 != 0 )
            sw_set_vlan_bypass(0, vlan_tag, eth0 < 0 ? 1 : 0);
        if ( eth1 != 0 )
            sw_set_vlan_bypass(1, vlan_tag, eth0 < 0 ? 1 : 0);
    }

    return count;
}

ssize_t proc_write_cell(struct file *file, const char __user *buf, size_t count, loff_t *data)
{
    char local_buf[128];
    int len;
    char *p1, *p2;
    int colon = 1;
    int conn = -1;
    unsigned int vpi = 0, vci = 0, gfc = 0, pti = 0, clp = 0;
    unsigned char cell[CELL_SIZE] = {0};
    struct uni_cell_header *header = (struct uni_cell_header *)cell;
    unsigned int pos = sizeof(*header);
    unsigned long sys_flag;
    int ret;
    int i;

    len = sizeof(local_buf) < count ? sizeof(local_buf) - 1 : count;
    len = len - copy_from_user(local_buf, buf, len);
    local_buf[len] = 0;

    p1 = local_buf;
    while ( get_token(&p1, &p2, &len, &colon) ) {
        if ( strcasecmp(p1, "help") == 0 ) {
            printk("echo <pvc vpi.vci> [pti] [gfc] [clp] <hex data> > /proc/eth/vrx318/cell\n");
            conn = -1;
            break;
        }
        else if ( strcasecmp(p1, "pvc") == 0 ) {
            ignore_space(&p2, &len);
            vpi = get_number(&p2, &len, 0);
            ignore_space(&p2, &len);
            vci = get_number(&p2, &len, 0);
            if ( vci == 0x03 || vci == 0x04 )
                conn = find_vpi(vpi);
            else
                conn = find_vpivci(vpi, vci);
            dbg("vpi = %u, vci = %u, conn = %d", vpi, vci, conn);
        }
        else if ( strcasecmp(p1, "pti") == 0 ) {
            ignore_space(&p2, &len);
            if ( strncasecmp(p2, "ATM_PTI_SEGF5", 13) == 0 ) {
                pti = ATM_PTI_SEGF5;
                p2 += 13;
                len -= 13;
            }
            else if ( strncasecmp(p2, "SEGF5", 5) == 0 ) {
                pti = ATM_PTI_SEGF5;
                p2 += 5;
                len -= 5;
            }
            else if ( strncasecmp(p2, "ATM_PTI_E2EF5", 13) == 0 ) {
                pti = ATM_PTI_E2EF5;
                p2 += 13;
                len -= 13;
            }
            else if ( strncasecmp(p2, "E2EF5", 5) == 0 ) {
                pti = ATM_PTI_E2EF5;
                p2 += 5;
                len -= 5;
            }
            else
                pti = get_number(&p2, &len, 0);
        }
        else if ( strcasecmp(p1, "gfc") == 0 ) {
            ignore_space(&p2, &len);
            gfc = get_number(&p2, &len, 0);
        }
        else if ( strcasecmp(p1, "clp") == 0 ) {
            ignore_space(&p2, &len);
            clp = get_number(&p2, &len, 0);
        }
        else if ( isxdigit(*p1) && pos < ARRAY_SIZE(cell) ) {
            cell[pos++] = simple_strtoul(p1, NULL, 16);
        }

        p1 = p2;
        colon = 1;
    }

    if ( conn >= 0 ) {
        header->gfc = gfc;
        header->vpi = vpi;
        header->vci = vci;
        header->pti = pti;
        header->clp = clp;

        ret = ppe_send_oam(g_atm_priv_data.connection[conn].vcc, cell, 0);

        local_irq_save(sys_flag);
        printk("ppe_send_oam\n");
        printk("  PVC %d.%d, TX QId %d\n", vpi, vci, conn);
        for ( i = 0; i < ARRAY_SIZE(cell); i++ ) {
            if ( i % 8 == 0 )
                printk(i == 0 ? "  cell:" : "\n       ");
            printk(" %02x", (unsigned int)cell[i]);
        }
        if ( i % 8 != 0 )
            printk("\n");
        printk("  return %d\n", ret);
        local_irq_restore(sys_flag);
    }

    return count;
}

/*
static int in_showtime(void)
{
    int i;

    for(i = 0; i < LINE_NUMBER; i ++){
        if(g_showtime[i]){
            return 1;
        }
    }

    return 0;
}
*/

static int atm_showtime_enter(const unsigned char line_idx,struct port_cell_info *port_cell, void *xdata_addr)
{
    int i, j;

    ASSERT(port_cell != NULL, "port_cell is NULL");
    ASSERT(xdata_addr != NULL, "xdata_addr is NULL");
    ASSERT(line_idx < LINE_NUMBER, "line_idx: %d large than max line num: %d", line_idx, LINE_NUMBER);

    for ( j = 0; j < ATM_PORT_NUMBER && j < port_cell->port_num; j++ )
        if ( port_cell->tx_link_rate[j] > 0 )
            break;
    for ( i = 0; i < ATM_PORT_NUMBER && i < port_cell->port_num; i++ )
        g_atm_priv_data.port[i].tx_max_cell_rate = port_cell->tx_link_rate[i] > 0 ? port_cell->tx_link_rate[i] : port_cell->tx_link_rate[j];

    qsb_global_set();

    for ( i = 0; i < atm_pvc_num(); i++ )
        if ( (g_atm_priv_data.pvc_table & (1 << i)) && g_atm_priv_data.connection[i].vcc != NULL )
            set_qsb(g_atm_priv_data.connection[i].vcc, &g_atm_priv_data.connection[i].vcc->qos, i);

    g_showtime[line_idx] = 1;

    dbg("line[%d]:enter showtime, cell rate: 0 - %d, 1 - %d, xdata addr: 0x%08x",line_idx,g_atm_priv_data.port[0].tx_max_cell_rate, g_atm_priv_data.port[1].tx_max_cell_rate, (unsigned int)g_xdata_addr);

    return PPA_SUCCESS;
}

static int atm_showtime_exit(const unsigned char line_idx)
{
    ASSERT(line_idx < LINE_NUMBER, "line_idx: %d large than max line num: %d", line_idx, LINE_NUMBER);
    
    if ( !g_showtime[line_idx] )
        return PPA_FAILURE;

    g_showtime[line_idx] = 0;

    dbg("line[%d]:leave showtime",line_idx);

    return PPA_SUCCESS;
}

static void check_showtime(void)
{
    int i;
    struct port_cell_info port_cell = {0};
    int showtime = 0;
    void *xdata_addr = NULL;
    int ret = -1;
    ltq_mei_atm_showtime_check_t ltq_mei_atm_showtime_check = NULL;

    ltq_mei_atm_showtime_check = (ltq_mei_atm_showtime_check_t)ppa_callback_get(LTQ_MEI_SHOWTIME_CHECK);
    if (!ltq_mei_atm_showtime_check) {
        return;
    }

    for(i = 0; i < LINE_NUMBER; i++){
        ret = ltq_mei_atm_showtime_check(i, &showtime, &port_cell, &xdata_addr);
        if(!ret && showtime){
            atm_showtime_enter(i,&port_cell, &xdata_addr);
        }
    }

    return;
}


int __init vrx218_atm_datapath_init(const ifx_pcie_ep_dev_t *p_vrx218_dev)
{
    int ret;
    int i;

    g_atm_priv_data.vrx218_dev = *p_vrx218_dev;

    for ( i = 0; i < ATM_PORT_NUMBER; i++ )
    {
        g_atm_priv_data.port[i].tx_max_cell_rate = 3200;    //  default cell rate
        g_atm_priv_data.port[i].dev = atm_dev_register("ltq_atm", NULL, &g_ppe_atm_ops, -1, 0UL);
        if ( !g_atm_priv_data.port[i].dev )
        {
            ret = -EIO;
            goto ATM_DEV_REGISTER_FAIL;
        }
        else
        {
            g_atm_priv_data.port[i].dev->ci_range.vpi_bits = 8;
            g_atm_priv_data.port[i].dev->ci_range.vci_bits = 16;
            g_atm_priv_data.port[i].dev->link_rate = 3200;  //  assume 3200 cell rate before real information passed in
            g_atm_priv_data.port[i].dev->dev_data = (void*)i;
        }
    }

    /* request irq (enable by default) */
    ret = request_irq(g_atm_priv_data.vrx218_dev.irq, mailbox_irq_handler, IRQF_DISABLED, "vrx318_ppe_isr", NULL);
    if ( ret != 0 ) {
        printk(KERN_ERR "Failed to request PCIe MSI irq %u\n", g_atm_priv_data.vrx218_dev.irq);
        goto REQUEST_IRQ_FAIL;
    }

    qsb_global_set();

    setup_oam_htu_entry();

    ppa_hook_mpoa_setup = mpoa_setup;

    enable_vrx218_swap(1, 1, 0);
    enable_vrx218_dma_rx(1);
    /* call enable_vrx218_dma_tx(1); in mailbox_irq_handler in vrx218_ptm_main.c */

    g_smartphy_push_fn  = atm_push;
    g_smartphy_port_num = 2;
    ppa_drv_get_dslwan_qid_with_vcc_hook = get_dslwan_qid_with_vcc;
    ppa_drv_get_atm_qid_with_pkt_hook    = get_atm_qid_with_pkt;
    ppa_drv_hal_get_mpoa_type_hook       = get_mpoa_type;
    
    check_showtime();
    ppa_callback_set(LTQ_MEI_SHOWTIME_ENTER, (void *)atm_showtime_enter);
    ppa_callback_set(LTQ_MEI_SHOWTIME_EXIT, (void *)atm_showtime_exit);

#ifdef CONFIG_IFX_LED
    setup_timer(&g_dsl_led_polling_timer, dsl_led_polling, 0);
    ifx_led_trigger_register("dsl_data", &g_data_led_trigger);
#endif

    return 0;

REQUEST_IRQ_FAIL:
ATM_DEV_REGISTER_FAIL:
    for ( i = 0; i < ATM_PORT_NUMBER; i++ )
        if ( g_atm_priv_data.port[i].dev )
            atm_dev_deregister(g_atm_priv_data.port[i].dev);
    return ret;
}

void __exit vrx218_atm_datapath_exit(void)
{
    int i;

    g_stop_datapath = 1;

#ifdef CONFIG_IFX_LED
    del_timer(&g_dsl_led_polling_timer);
    ifx_led_trigger_deregister(g_data_led_trigger);
    g_data_led_trigger = NULL;
#endif

    ppa_callback_set(LTQ_MEI_SHOWTIME_ENTER, (void *)NULL);
    ppa_callback_set(LTQ_MEI_SHOWTIME_EXIT, (void *)NULL);

    invalidate_oam_htu_entry();

    g_smartphy_port_num = 0;
    g_smartphy_push_fn = NULL;
    ppa_drv_get_dslwan_qid_with_vcc_hook = NULL;
    ppa_drv_get_atm_qid_with_pkt_hook    = NULL;
    ppa_drv_hal_get_mpoa_type_hook       = NULL;

    enable_vrx218_dma_tx(0);
    enable_vrx218_dma_rx(0);
    enable_vrx218_swap(0, 1, 0);

    ppa_hook_mpoa_setup = NULL;

    free_irq(g_atm_priv_data.vrx218_dev.irq, NULL);

    for ( i = 0; i < ATM_PORT_NUMBER; i++ )
        if ( g_atm_priv_data.port[i].dev )
            atm_dev_deregister(g_atm_priv_data.port[i].dev);
}
