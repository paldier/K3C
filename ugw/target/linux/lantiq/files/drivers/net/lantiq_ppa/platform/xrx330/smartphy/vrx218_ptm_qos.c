/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>

/*
 *  Chip Specific Head File
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pcie.h>
#else
#include <linux/version.h>
#include <lantiq_dma.h>
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>
#include <lantiq_pcie.h>

#endif



#include <net/ppa_api.h>

#include <net/ppa_ppe_hal.h>
#include "../../ppa_datapath.h"


#include "vrx218_common.h"
#include "unified_qos_ds_be.h"
#include "vrx218_e1_addr_def.h"




/*
 * ####################################
 *              Definition
 * ####################################
 */
//4
#define MAX_PTM_QOS_PORT_NUM                1  //4
#define MAX_MBR_QOS_SHAPER_NUM              2
#define MAX_SHAPER_IDX                      20
#define DEFAULT_L2_SHAPER_IDX               17


#ifdef CONFIG_LTQ_PPA_QOS
  #define PPE_MAX_ETH1_QOS_QUEUE            16
  #define IFX_PPA_DRV_QOS_RATESHAPE_bitrate_2_R( bitrate_kbps, T, basic_tick)   ( (bitrate_kbps) * (T) * (basic_tick) /8/1000 )
  #define IFX_PPA_DRV_QOS_RATESHAPE_R_2_bitrate( R, T, basic_tick)              ( (R) * 8 * 1000 / (T) / (basic_tick) )
 #ifdef CONFIG_LTQ_PPA_QOS_WFQ
  #define IFX_PPA_DRV_QOS_WFQ_WLEVEL_2_W    2000
 #endif
#endif



/*
 * ####################################
 *            Address Mapping
 * ####################################
 */

#define SHAPING_CFG                         ((volatile shaping_cfg_t *)SOC_ACCESS_VRX218_SB(__SHAPING_CFG, g_us_vrx218_dev->phy_membase))
#define WFQ_CFG                             ((volatile wfq_cfg_t *)SOC_ACCESS_VRX218_SB(__WFQ_CFG, g_us_vrx218_dev->phy_membase))
#define TX_QOS_WFQ_RELOAD_MAP               (WFQ_CFG->wfq_force_reload_map)

#define QOS_CFG                             ((volatile qos_cfg_t *)SOC_ACCESS_VRX218_SB(__QOS_CFG, g_us_vrx218_dev->phy_membase))
#define WAN_TX_DESC_NUM_TOTAL                   512
#define __ETH_WAN_TX_QUEUE_NUM                  8
#define WAN_QOS_DESC_BASE                        0x2800
#define __ETH_WAN_TX_QUEUE_LEN                  (WAN_TX_DESC_NUM_TOTAL / __ETH_WAN_TX_QUEUE_NUM)
#define ETH_WAN_TX_DESC_NUM(i)                  __ETH_WAN_TX_QUEUE_LEN
#define __ETH_WAN_TX_DESC_BASE(i)               (0x2800 + (i) * 2 * __ETH_WAN_TX_QUEUE_LEN)
#define WTX_QOS_Q_DESC_CFG(i)               ((volatile qosq_cfg_ctxt_t *)SOC_ACCESS_VRX218_SB(__QOSQ_CFG_CTXT_BASE + (i) * __QOSQ_CFG_CTXT_SIZE, g_us_vrx218_dev->phy_membase)) /*  i < 16  */
#define SHAPING_WFQ_CFG(i)                  ((volatile shaping_wfq_cfg_t *)SOC_ACCESS_VRX218_SB(__SHAPING_WFQ_CFG_BASE + (i) * __SHAPING_WFQ_CFG_SIZE, g_us_vrx218_dev->phy_membase))   /*  i: 0~15 - QoS Q, 16~19 - L2 Shaper, 20 - L3 Shaper  */
//#define WTX_EG_Q_PORT_SHAPING_CFG(i)        SHAPING_WFQ_CFG(20 + (i))   /*  i < 1   */
#define WTX_PTM_EG_Q_PORT_SHAPING_CFG(i)    SHAPING_WFQ_CFG(20 + (i))   /*  i < 1   */
#define WTX_EG_Q_SHAPING_CFG(i)             SHAPING_WFQ_CFG(i)          /*  i < 16  */
#define OUTQ_ID_FROM_SHAPERID(i)            ((i)-DEFAULT_L2_SHAPER_IDX + 1) /*  18 < i < 20 *//*Default MBR shaper is 19.  available MBR shaper is 17 and 18 */ 
#define SHAPERID_FROM_OUTQID(i)             ((i)+DEFAULT_L2_SHAPER_IDX )    /*  0 <= i < 2 */ /*shaper id 17-18 */ 

#define OUTQ_QOS_CFG_CTXT(i)                ((volatile outq_qos_cfg_ctxt_t *)SOC_ACCESS_VRX218_SB(__OUTQ_QOS_CFG_CTXT_BASE + (i) * 4, g_us_vrx218_dev->phy_membase))            /*  i < 4   */

#define WAN_TX_MIB_TABLE(i)                 ((volatile struct wan_tx_mib_table *)SOC_ACCESS_VRX218_SB(__QOSQ_MIB_BASE + (i) * __QOSQ_MIB_SIZE, g_us_vrx218_dev->phy_membase))   /*  i < 16  */
#define ETH_WAN_TX_MIB_TABLE(i)             WAN_TX_MIB_TABLE(i)

#define CGU_CLKFSR(base)                    SOC_ACCESS_VRX218_ADDR(0x1E003010, base)



/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *  WAN QoS MODE List
 */
enum{
   PPA_WAN_QOS_ETH0 = 0,
   PPA_WAN_QOS_ETH1 = 1,
   PPA_WAN_QOS_PTM0 = 7,
};

struct wan_tx_mib_table {
    unsigned int    wrx_total_pdu;
    unsigned int    wrx_total_bytes;
    unsigned int    wtx_total_pdu;
    unsigned int    wtx_total_bytes;

    unsigned int    wtx_cpu_drop_small_pdu;
    unsigned int    wtx_cpu_drop_pdu;
    unsigned int    wtx_fast_drop_small_pdu;
    unsigned int    wtx_fast_drop_pdu;
};



/*
 * ####################################
 *             Declaration
 * ####################################
 */

extern ifx_pcie_ep_dev_t * const g_us_vrx218_dev;

#ifdef CONFIG_LTQ_PPA_QOS
#if defined(MBR_CONFIG) && MBR_CONFIG
   static int32_t set_qos_shaper(int32_t shaperid, uint32_t rate, uint32_t burst, uint32_t flag);
   static int32_t get_qos_rate( uint32_t portid, uint32_t queueid, int32_t *shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag);
   static int32_t reset_qos_shaper(int32_t shaperid, uint32_t flag);
   static void init_mbr_q_shaper_mapping(void);
   static void reset_qmap(void);
#else
  static int32_t get_qos_rate( uint32_t portid, uint32_t queueid, uint32_t *rate, uint32_t *burst, uint32_t flag);
#endif
 #ifdef CONFIG_LTQ_PPA_QOS_WFQ
  static int32_t get_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t *weight_level, uint32_t flag);
 #endif
#endif


/*
 * ####################################
 *            Extern Variable
 * ####################################
 */
extern int g_queue_gamma_map[4];


/*
 * ####################################
 *            Local Variable
 * ####################################
 */


#ifdef CONFIG_LTQ_PPA_QOS
  static uint32_t qos_queue_portid = PPA_WAN_QOS_PTM0;
 #ifdef CONFIG_LTQ_PPA_QOS_WFQ
  static uint32_t wfq_multiple = IFX_PPA_DRV_QOS_WFQ_WLEVEL_2_W;
  static uint32_t wfq_strict_pri_weight = 0x7FFFFF;
 #endif
 #ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
  //static uint32_t  default_qos_rateshaping_burst = 6000;
  static uint32_t  default_qos_rateshaping_burst = 12000; 
 #endif

 static int32_t g_qos_q_shaper_mapping[PPE_MAX_ETH1_QOS_QUEUE];

 
#endif



/*
 * ####################################
 *            Global Function
 * ####################################
 */

#ifdef CONFIG_LTQ_PPA_QOS

struct bitrate_2_t_kbps
{
    uint32_t bitrate_kbps;
    uint32_t T; //Time ticks
};

static struct bitrate_2_t_kbps bitrate_2_t_kbps_table[]={
    {  100000,   1},
    {   10000,  10},
    {    1000, 100},
    {       0, 250}
};

static inline int32_t cgu_get_pp32_clock(void)
{
    if ( (*CGU_CLKFSR(g_us_vrx218_dev->phy_membase) & 0x00070000) == 0x00010000 )
        return 432000000;
    else
        return 288000000;
}

static int32_t get_basic_time_tick(void)
{
    return QOS_CFG->time_tick / (cgu_get_pp32_clock() /1000000);
}

static inline uint32_t set_qos_port_id(void)
{
    return PPA_SUCCESS;
}

/*!
    \brief This is to set the qos queue for vrx318
    \param[in] qos_queue_len  the array of qos setting
    \return returns nothing
*/
void init_qos_queue(int* qos_queue_len)
{
	volatile qosq_cfg_ctxt_t wtx_qos_q_desc_cfg[PPE_MAX_ETH1_QOS_QUEUE];
    int i;
    int des_num = WAN_TX_DESC_NUM_TOTAL;
    int q_len = WAN_TX_DESC_NUM_TOTAL;
    int equal_div = 0;
    
    for(i = 0; i < __ETH_WAN_TX_QUEUE_NUM; i++){
        if(q_len < qos_queue_len[i] || (q_len - qos_queue_len[i]) < (__ETH_WAN_TX_QUEUE_NUM - i - 1)){
            q_len = -1;
            err("total qos queue descriptor number is large than max number(%d)", des_num);
            err("q_len: %d, queue len: %d, qid: %d", q_len, qos_queue_len[i], i);
            break;
        }
        q_len -= qos_queue_len[i];
    }
    
    if(q_len == -1){
        for ( i = 0; i < __ETH_WAN_TX_QUEUE_NUM; i++ )
        {
        	wtx_qos_q_desc_cfg[i] = *WTX_QOS_Q_DESC_CFG(i);
            wtx_qos_q_desc_cfg[i].des_num = ETH_WAN_TX_DESC_NUM(i);
            wtx_qos_q_desc_cfg[i].des_base_addr   = __ETH_WAN_TX_DESC_BASE(i);
            *WTX_QOS_Q_DESC_CFG(i) = wtx_qos_q_desc_cfg[i];
        }
    }else{      
        q_len = 0;
        for ( i = 0; i < __ETH_WAN_TX_QUEUE_NUM; i++){
			wtx_qos_q_desc_cfg[i] = *WTX_QOS_Q_DESC_CFG(i);
            if(!equal_div && qos_queue_len[i] == 0){
                equal_div = (des_num - q_len)/(__ETH_WAN_TX_QUEUE_NUM - i) < 256 ? (des_num - q_len)/(__ETH_WAN_TX_QUEUE_NUM - i) : 255; 
            }

            if(!equal_div){
                wtx_qos_q_desc_cfg[i].des_num = qos_queue_len[i] < 256 ? qos_queue_len[i] : 255;
            }else{
                wtx_qos_q_desc_cfg[i].des_num = equal_div;
            }
            wtx_qos_q_desc_cfg[i].des_base_addr   = WAN_QOS_DESC_BASE + (q_len << 1);
            q_len += wtx_qos_q_desc_cfg[i].des_num;
            *WTX_QOS_Q_DESC_CFG(i) = wtx_qos_q_desc_cfg[i];
        }
    }

    return;
}

/*!
    \brief This is to get the maximum queue number supported on specified port
    \param[in] portid the physical port id which support qos queue
    \param[in] flag reserved for future
    \return returns the queue number supported on this port.
*/
static int32_t get_qos_qnum( uint32_t portid, uint32_t flag)
{
    if( set_qos_port_id() != PPA_SUCCESS ) return 0;
    if ( qos_queue_portid != portid )
        return 0;

    return QOS_CFG->qosq_num;
}

static int32_t get_qos_status( PPA_QOS_STATUS *stat)
{
    shaping_wfq_cfg_t qos_cfg;
    struct wan_tx_mib_table  qos_queue_mib;
    volatile qos_cfg_t tx_qos_cfg = *QOS_CFG;
    volatile qosq_cfg_ctxt_t qos_q_desc_cfg[PPE_MAX_ETH1_QOS_QUEUE];

    if( !stat ) return PPA_FAILURE;

    stat->qos_queue_portid = qos_queue_portid;

    if( set_qos_port_id() != PPA_SUCCESS )
    {
        return PPA_SUCCESS;
    }

    stat->eth1_qss = tx_qos_cfg.qos_en;                     //tx_qos_cfg.eth1_qss;
    stat->wfq_en = WFQ_CFG->wfq_reload_enable_map;          //tx_qos_cfg.wfq_en;
    stat->shape_en = SHAPING_CFG->qosq_shaper_enable_map;   //tx_qos_cfg.shape_en;
    stat->time_tick= tx_qos_cfg.time_tick;
    stat->overhd_bytes = OUTQ_QOS_CFG_CTXT(0)->overhd_bytes;//tx_qos_cfg.overhd_bytes;
    stat->eth1_eg_qnum = tx_qos_cfg.qosq_num;
    stat->tx_qos_cfg_addr = (uint32_t )QOS_CFG;

    stat->pp32_clk = cgu_get_pp32_clock();
    stat->basic_time_tick = get_basic_time_tick();



#ifdef CONFIG_LTQ_PPA_QOS_WFQ
    stat->wfq_multiple = wfq_multiple;
    stat->wfq_strict_pri_weight_addr = (uint32_t )&wfq_multiple;
#endif

    if ( tx_qos_cfg.qosq_num )
    {
        uint32_t bit_rate_kbps=0;
        uint32_t weight_level=0;
        int32_t  shaperid       = 0;
        int i;

        for ( i = 0; i < PPE_MAX_ETH1_QOS_QUEUE && i<=stat->max_buffer_size; i++ )
        {
            qos_cfg = *WTX_EG_Q_SHAPING_CFG(i);
#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
   #if defined(MBR_CONFIG) && MBR_CONFIG
            get_qos_rate( qos_queue_portid, i, &shaperid, &bit_rate_kbps, NULL,0);
   #else
            get_qos_rate( qos_queue_portid, i, &bit_rate_kbps, NULL,0);
   #endif
            stat->queue_internal[i].bit_rate_kbps = bit_rate_kbps;
#endif
#ifdef CONFIG_LTQ_PPA_QOS_WFQ
            get_qos_wfq( qos_queue_portid, i, &weight_level, 0);
            stat->queue_internal[i].weight_level = weight_level;
#endif

            stat->queue_internal[i].t = qos_cfg.t;
            stat->queue_internal[i].r = qos_cfg.r;
            stat->queue_internal[i].s = qos_cfg.s;

            stat->queue_internal[i].w = qos_cfg.w;
            stat->queue_internal[i].d = qos_cfg.d;
            stat->queue_internal[i].tick_cnt = qos_cfg.tick_cnt;
            stat->queue_internal[i].b = qos_cfg.b;

            stat->queue_internal[i].reg_addr = (uint32_t )WTX_EG_Q_SHAPING_CFG(i);
        }
#if 0
        //QOS Note: For ethernat wan mode only. For E5 ptm mode, it is not necessary since there is no port based rate shaping
        {
            qos_cfg = *WTX_EG_Q_PORT_SHAPING_CFG(0);

#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
   #if defined(MBR_CONFIG) && MBR_CONFIG
            get_qos_rate( qos_queue_portid, i, &shaperid, &bit_rate_kbps, NULL,0);    
   #else
            get_qos_rate( qos_queue_portid, i, &bit_rate_kbps, NULL,0);
   #endif
            stat->qos_port_rate_internal.bit_rate_kbps = bit_rate_kbps;
#endif

            stat->qos_port_rate_internal.t = qos_cfg.t;
            stat->qos_port_rate_internal.r = qos_cfg.r;
            stat->qos_port_rate_internal.s = qos_cfg.s;

            stat->qos_port_rate_internal.w = qos_cfg.w;
            stat->qos_port_rate_internal.d = qos_cfg.d;
            stat->qos_port_rate_internal.tick_cnt = qos_cfg.tick_cnt;
            stat->qos_port_rate_internal.b = qos_cfg.b;

            stat->qos_port_rate_internal.reg_addr = (uint32_t)WTX_EG_Q_PORT_SHAPING_CFG(0);
        }
#endif

        if (qos_queue_portid == PPA_WAN_QOS_PTM0){
            for( i = 0; i < MAX_PTM_QOS_PORT_NUM; i ++){
                qos_cfg = *WTX_PTM_EG_Q_PORT_SHAPING_CFG(i);
#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
    #if defined(MBR_CONFIG) && MBR_CONFIG
                get_qos_rate( qos_queue_portid, ~i, &shaperid, &bit_rate_kbps, NULL,0);    
    #else
                get_qos_rate( qos_queue_portid, ~i, &bit_rate_kbps, NULL,0);
    #endif
                stat->ptm_qos_port_rate_shaping[i].bit_rate_kbps = bit_rate_kbps;
#endif
                stat->ptm_qos_port_rate_shaping[i].t = qos_cfg.t;
                stat->ptm_qos_port_rate_shaping[i].r = qos_cfg.r;
                stat->ptm_qos_port_rate_shaping[i].s = qos_cfg.s;

                stat->ptm_qos_port_rate_shaping[i].w = qos_cfg.w;
                stat->ptm_qos_port_rate_shaping[i].d = qos_cfg.d;
                stat->ptm_qos_port_rate_shaping[i].tick_cnt = qos_cfg.tick_cnt;
                stat->ptm_qos_port_rate_shaping[i].b = qos_cfg.b;

                stat->ptm_qos_port_rate_shaping[i].reg_addr = (uint32_t)WTX_PTM_EG_Q_PORT_SHAPING_CFG(0);
            }
        }
        for ( i = 0; i < PPE_MAX_ETH1_QOS_QUEUE && i<=stat->max_buffer_size; i++ )
        {
            qos_queue_mib = *ETH_WAN_TX_MIB_TABLE(i);
            stat->mib[i].mib.total_rx_pkt= qos_queue_mib.wrx_total_pdu;
            stat->mib[i].mib.total_rx_bytes  = qos_queue_mib.wrx_total_bytes;
            stat->mib[i].mib.total_tx_pkt  = qos_queue_mib.wtx_total_pdu;
            stat->mib[i].mib.total_tx_bytes  = qos_queue_mib.wtx_total_bytes;
            stat->mib[i].mib.cpu_path_small_pkt_drop_cnt  = qos_queue_mib.wtx_cpu_drop_small_pdu;
            stat->mib[i].mib.cpu_path_total_pkt_drop_cnt  = qos_queue_mib.wtx_cpu_drop_pdu;
            stat->mib[i].mib.fast_path_small_pkt_drop_cnt  = qos_queue_mib.wtx_fast_drop_small_pdu;
            stat->mib[i].mib.fast_path_total_pkt_drop_cnt  = qos_queue_mib.wtx_fast_drop_pdu;

            stat->mib[i].reg_addr = (uint32_t)ETH_WAN_TX_MIB_TABLE(i);
        }


        //QOS queue descriptor
        for(i=0;  i < PPE_MAX_ETH1_QOS_QUEUE && i<=stat->max_buffer_size; i++)
        {
            qos_q_desc_cfg[i] = *WTX_QOS_Q_DESC_CFG(i);

            stat->desc_cfg_interanl[i].threshold = qos_q_desc_cfg[i].threshold;
            stat->desc_cfg_interanl[i].length = qos_q_desc_cfg[i].des_num;
            stat->desc_cfg_interanl[i].addr = qos_q_desc_cfg[i].des_base_addr;
            stat->desc_cfg_interanl[i].rd_ptr = qos_q_desc_cfg[i].deq_idx;
            stat->desc_cfg_interanl[i].wr_ptr = qos_q_desc_cfg[i].enq_idx;

            stat->desc_cfg_interanl[i].reg_addr = (uint32_t)WTX_QOS_Q_DESC_CFG(i);
        }
    }

    return PPA_SUCCESS;
}

/*!
    \brief This is to get the mib couter for specified port and queue
    \param[in] portid the physical port id
    \param[in] queueid the queueid for the mib counter to get
    \param[out] mib the buffer for mib counter
    \param[in] flag reserved for future
    \return returns the queue number supported on this port.
*/
static int32_t get_qos_mib(uint32_t portid, uint32_t queueid, PPA_QOS_MIB *mib, uint32_t flag)
{
    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( !mib )
        return PPA_FAILURE;

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;

    if ( queueid >= QOS_CFG->qosq_num )
        return PPA_FAILURE;

    {
        struct wan_tx_mib_table qos_queue_mib;

        qos_queue_mib = *ETH_WAN_TX_MIB_TABLE(queueid);

        mib->total_rx_pkt   = qos_queue_mib.wrx_total_pdu;
        mib->total_rx_bytes = qos_queue_mib.wrx_total_bytes;
        mib->total_tx_pkt   = qos_queue_mib.wtx_total_pdu;
        mib->total_tx_bytes = qos_queue_mib.wtx_total_bytes;

        mib->cpu_path_small_pkt_drop_cnt    = qos_queue_mib.wtx_cpu_drop_small_pdu;
        mib->cpu_path_total_pkt_drop_cnt    = qos_queue_mib.wtx_cpu_drop_pdu;
        mib->fast_path_small_pkt_drop_cnt   = qos_queue_mib.wtx_fast_drop_small_pdu;
        mib->fast_path_total_pkt_drop_cnt   = qos_queue_mib.wtx_fast_drop_pdu;
    }

    return PPA_SUCCESS;
}

#ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
/*!
    \brief This is to eanble/disable Rate Shaping feature
    \param[in] portid the phisical port id which support qos queue
    \param[in] enabled 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t set_ctrl_qos_rate( uint32_t portid, uint32_t enable, uint32_t flag)
{
#if 0
    qos_cfg_t tx_qos_cfg = {0};

    if( set_qos_port_id() != PPA_SUCCESS )  return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;
    tx_qos_cfg = *QOS_CFG;
    // if ( !tx_qos_cfg.eth1_qss ) return PPA_FAILURE;  //Note, For E5 ptm wan mode, the eth1_qss is disabled.

    if ( enable )
    {
        tx_qos_cfg.shape_en = 1;
        tx_qos_cfg.overhd_bytes = 24; //add 24 bytes preamble and inter-frame gape
    }
    else
    {
        tx_qos_cfg.shape_en = 0;

        if ( !tx_qos_cfg.shape_en && !tx_qos_cfg.wfq_en )
        {
            //tx_qos_cfg.eth1_qss = 1;
            //tx_qos_cfg.qosq_num = 0;
        }
    }

    *QOS_CFG = tx_qos_cfg;
#else
    int i;

    if( set_qos_port_id() != PPA_SUCCESS )  return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;

    if ( enable )
    {
        SHAPING_CFG->qosq_shaper_enable_map = (1 << QOS_CFG->qosq_num) - 1;
        SHAPING_CFG->outq_shaper_enable_map = 0xF; // Only enable all, old outq 0
        SHAPING_CFG->port_shaper_enable_map = 1;
        QOS_CFG->qos_en = 1;
        for ( i = 0; i < 4; i++ )
            //if ( OUTQ_QOS_CFG_CTXT(i)->qmap )
            {
                OUTQ_QOS_CFG_CTXT(i)->qos_en = 1;
                OUTQ_QOS_CFG_CTXT(i)->shape_en = 1;
            }
    }
    else
    {
        SHAPING_CFG->qosq_shaper_enable_map = 0x0000;
        SHAPING_CFG->outq_shaper_enable_map = 0;
        SHAPING_CFG->port_shaper_enable_map = 0;

        for ( i = 0; i < 4; i++ )
            OUTQ_QOS_CFG_CTXT(i)->shape_en = 0;
        if ( WFQ_CFG->wfq_reload_enable_map == 0 )
        {
            QOS_CFG->qos_en = 0;
            for ( i = 0; i < 4; i++ )
                OUTQ_QOS_CFG_CTXT(i)->qos_en = 0;
        }
    }
#endif

    return PPA_SUCCESS;
}

/*!
    \brief This is to get Rate Shaping feature status: enabled or disabled
    \param[in] portid the phisical port id which support qos queue
    \param[out] enabled 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t get_ctrl_qos_rate( uint32_t portid, uint32_t *enable, uint32_t flag)
{
    if( set_qos_port_id() != PPA_SUCCESS )  return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;

    if ( enable )
    {
//        if ( QOS_CFG->shape_en )
        if ( SHAPING_CFG->qosq_shaper_enable_map )
            *enable = 1;
        else
            *enable =0;
    }

    return PPA_SUCCESS;
}

/*!
    \brief This is to set Rate Shaping for one specified port and queue
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set rate shaping. \n
              If queue id bigger than muximum queue id, it will be regarded as port based rate shaping.
    \param[in] rate the maximum rate limit in kbps
    \param[in] burst the maximun burst in bytes
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#if defined(MBR_CONFIG) && MBR_CONFIG
static int32_t set_qos_rate( uint32_t portid, uint32_t queueid, int32_t shaperid, uint32_t rate, uint32_t burst, uint32_t flag)
#else
static int32_t set_qos_rate( uint32_t portid, uint32_t queueid, uint32_t rate, uint32_t burst, uint32_t flag)
#endif
{
    int i;
    shaping_wfq_cfg_t qos_cfg = {0};
    volatile shaping_wfq_cfg_t *p_qos_cfg = NULL;
    uint32_t qos_portid;//port rateshaping port id
    int32_t shaper_id = -1;
    volatile outq_qos_cfg_ctxt_t *outq_qos_cfg_ctxt;
    int32_t cur_shaperid;
    if( set_qos_port_id() != PPA_SUCCESS ){
		return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom
    	}

    if ( qos_queue_portid!= portid ){
        return PPA_FAILURE;
    }

#if defined(MBR_CONFIG) && MBR_CONFIG
    shaper_id = shaperid;
#endif

	if(shaper_id == -1){
        if ( queueid >= QOS_CFG->qosq_num )  //regard it as port based rate shaping
        {
            if( qos_queue_portid == PPA_WAN_QOS_PTM0){//ptm_wan_port;
                qos_portid = ~ queueid;
                if(qos_portid >= MAX_PTM_QOS_PORT_NUM){
                    return PPA_FAILURE;
                }
				qos_cfg = *WTX_PTM_EG_Q_PORT_SHAPING_CFG(qos_portid);
                p_qos_cfg = WTX_PTM_EG_Q_PORT_SHAPING_CFG(qos_portid);
            }else{//eth_wan_port
                //qos_portid = 0;
                //qos_cfg = *WTX_EG_Q_PORT_SHAPING_CFG(qos_portid);
                //p_qos_cfg = WTX_EG_Q_PORT_SHAPING_CFG(qos_portid);
                return PPA_FAILURE;
            }
        }
        else
        {
            qos_cfg = *WTX_EG_Q_SHAPING_CFG(queueid);
            p_qos_cfg = WTX_EG_Q_SHAPING_CFG(queueid);
        }

        if ( rate >= bitrate_2_t_kbps_table[0].bitrate_kbps )
        {
            qos_cfg.t = bitrate_2_t_kbps_table[0].T;
        }
        else
        {
            for( i = 0; i < sizeof(bitrate_2_t_kbps_table) / sizeof(bitrate_2_t_kbps_table[0]) - 1; i++ )
            {
                if ( rate < bitrate_2_t_kbps_table[i].bitrate_kbps && rate >= bitrate_2_t_kbps_table[i+1].bitrate_kbps )
                {
                    qos_cfg.t = bitrate_2_t_kbps_table[i+1].T;
                    break;
                }
            }
        }
        if ( qos_cfg.t == 0 )
        {
            return PPA_FAILURE;
        }
        if( burst == 0 )
        {
            burst = default_qos_rateshaping_burst;
        }

        qos_cfg.r = IFX_PPA_DRV_QOS_RATESHAPE_bitrate_2_R( rate, qos_cfg.t, get_basic_time_tick() );
        qos_cfg.s = burst;

        *p_qos_cfg = qos_cfg;
#if defined(MBR_CONFIG) && MBR_CONFIG
        //shaperid -1 is used to detach the shaper and remap to default shaper
        if ( queueid < QOS_CFG->qosq_num )  //regard it as port based rate shaping
        {
	        cur_shaperid = g_qos_q_shaper_mapping[queueid];
	        shaper_id = DEFAULT_L2_SHAPER_IDX + 2;

	        outq_qos_cfg_ctxt = OUTQ_QOS_CFG_CTXT(OUTQ_ID_FROM_SHAPERID(cur_shaperid));
	        outq_qos_cfg_ctxt->qmap &= ~(1 << queueid);
	        outq_qos_cfg_ctxt = OUTQ_QOS_CFG_CTXT(OUTQ_ID_FROM_SHAPERID(shaper_id));
	        outq_qos_cfg_ctxt->qmap |= (1 << queueid);
	        g_qos_q_shaper_mapping[queueid] = shaper_id;
        }
#endif
	}
#if defined(MBR_CONFIG) && MBR_CONFIG
    else{
        int cur_shaperid;

        if(shaper_id < 0 || shaper_id >= MAX_MBR_QOS_SHAPER_NUM){
            err("VRX318 only support %d shapers: shaper_id: %d\n",MAX_MBR_QOS_SHAPER_NUM, shaper_id);
            return PPA_FAILURE;
        }
        if ( queueid < QOS_CFG->qosq_num ){
            cur_shaperid = g_qos_q_shaper_mapping[queueid];
            shaper_id = SHAPERID_FROM_OUTQID(shaper_id) ;

            outq_qos_cfg_ctxt = OUTQ_QOS_CFG_CTXT(OUTQ_ID_FROM_SHAPERID(cur_shaperid));
            outq_qos_cfg_ctxt->qmap &= ~(1 << queueid);

            outq_qos_cfg_ctxt = OUTQ_QOS_CFG_CTXT(OUTQ_ID_FROM_SHAPERID(shaper_id));
            outq_qos_cfg_ctxt->qmap |= (1 << queueid);
            g_qos_q_shaper_mapping[queueid] = shaper_id;
        }
    }
#endif


    return PPA_SUCCESS;
}

/*!
    \brief This is to get Rate Shaping settings for one specified port and queue
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set rate shaping \n
              If queue id bigger than muximum queue id, it will be regarded as port based rate shaping.
    \param[out] rate the maximum rate limit in kbps
    \param[out] burst the maximun burst in bytes
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
#if defined(MBR_CONFIG) && MBR_CONFIG
static int32_t get_qos_rate( uint32_t portid, uint32_t queueid, int32_t *shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag)
#else
static int32_t get_qos_rate( uint32_t portid, uint32_t queueid, uint32_t *rate, uint32_t *burst, uint32_t flag)
#endif
{
    shaping_wfq_cfg_t qos_cfg = {0};
    int32_t shaper_id         = -1;
    uint32_t port_id          = 0;

    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //A5 Ethernet WAN mode only QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;
	

    if ( queueid >= QOS_CFG->qosq_num )
    {
        if( qos_queue_portid == PPA_WAN_QOS_PTM0) {
			if( queueid != QOS_CFG->qosq_num ) //user provided queueid as -1
                port_id = ~queueid;
			else
				port_id = 0; //user provided queueid as number of queue
			
            if(port_id >= MAX_PTM_QOS_PORT_NUM){//only support up to 1 port rateshaping
                memset(&qos_cfg,0,sizeof(qos_cfg));
            }else{
                qos_cfg = *WTX_PTM_EG_Q_PORT_SHAPING_CFG(port_id);
            }
        }else{
            //otherwise it is E5 ethernet wan mode, so regard it as port based rate shaping, not queue based rate shaping
            //qos_cfg = *WTX_EG_Q_PORT_SHAPING_CFG(0);
            err("VRX318 not supporting NON-PTM QoS WAN mode ");
            return PPA_FAILURE;
        }
    }
    else{
		if(queueid < PPE_MAX_ETH1_QOS_QUEUE){
            
#if defined(MBR_CONFIG) && MBR_CONFIG
            if(*shaperid == -1){
                shaper_id = g_qos_q_shaper_mapping[queueid];        
                *shaperid = OUTQ_ID_FROM_SHAPERID(shaper_id) < 1 ?  -1 : OUTQ_ID_FROM_SHAPERID(shaper_id)-1;
                qos_cfg   = *SHAPING_WFQ_CFG(shaper_id);
                if(*shaperid >= MAX_MBR_QOS_SHAPER_NUM)
                    *shaperid = -1; // used for ppacmd to print correct shaper id
            }else{
                qos_cfg = *WTX_EG_Q_SHAPING_CFG(queueid); //queue based rate shaping
            }
#else
            qos_cfg = *WTX_EG_Q_SHAPING_CFG(queueid); //queue based rate shaping       
#endif
         }else{
              err("Queue id should be less than 16");
              return PPA_FAILURE;
         }
    }

    if ( qos_cfg.t != 0 )   //not set yet
    {
        if ( rate )
            *rate = IFX_PPA_DRV_QOS_RATESHAPE_R_2_bitrate(qos_cfg.r, qos_cfg.t, get_basic_time_tick());
    }
    else
    {
        if ( rate )
            *rate = 0;
    }
    if ( burst )
        *burst = qos_cfg.s;

    return PPA_SUCCESS;
}

/*!
    \brief This is to reset Rate Shaping for one specified port and queue (
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set rate shaping
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t reset_qos_rate( uint32_t portid, uint32_t queueid, uint32_t flag )
{
    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE;
    if ( qos_queue_portid != portid )
        return PPA_FAILURE;

#if defined(MBR_CONFIG) && MBR_CONFIG
    set_qos_rate(portid, queueid, -1, 1000000, default_qos_rateshaping_burst, flag);
#else
    set_qos_rate(portid, queueid, 1000000, default_qos_rateshaping_burst, flag);
#endif

    return PPA_SUCCESS;
}

static int32_t init_qos_rate(void)
{
    int i;

	
#if defined(MBR_CONFIG) && MBR_CONFIG
    init_mbr_q_shaper_mapping();
    reset_qmap();
#endif

    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    for ( i = 0; i <= get_qos_qnum(qos_queue_portid, 0); i++ )  //here we purposely use <= to set port based rate sahping also
        reset_qos_rate(qos_queue_portid, i, 0);

    if(qos_queue_portid == PPA_WAN_QOS_PTM0){
        for( i = 0; i < MAX_PTM_QOS_PORT_NUM; i ++){
            reset_qos_rate(qos_queue_portid, ~i, 0); //set ~i to indicate it is not a queueid
        }
    }

#if defined(MBR_CONFIG) && MBR_CONFIG
    for(i = 0; i < 4; i ++){
        reset_qos_shaper(i,1);
    }

    //init_mbr_q_shaper_mapping();
    //reset_qmap();
    
#endif

    return PPA_SUCCESS;
}

#if defined(MBR_CONFIG) && MBR_CONFIG
 
 static void init_mbr_q_shaper_mapping(void)
 {
     int i;
     for(i = 0; i < PPE_MAX_ETH1_QOS_QUEUE; i ++){
         g_qos_q_shaper_mapping[i] = DEFAULT_L2_SHAPER_IDX+2;
     }
 }
 
 static void reset_qmap(void)
 {
     volatile outq_qos_cfg_ctxt_t *outq_qos_cfg_ctxt;
 
     outq_qos_cfg_ctxt       = OUTQ_QOS_CFG_CTXT(0);
	 outq_qos_cfg_ctxt->qmap = g_queue_gamma_map[3];
     outq_qos_cfg_ctxt       = OUTQ_QOS_CFG_CTXT(1);
	 outq_qos_cfg_ctxt->qmap = g_queue_gamma_map[2];
     outq_qos_cfg_ctxt       = OUTQ_QOS_CFG_CTXT(2);
	 outq_qos_cfg_ctxt->qmap = g_queue_gamma_map[1];
     outq_qos_cfg_ctxt       = OUTQ_QOS_CFG_CTXT(3);
	 outq_qos_cfg_ctxt->qmap = g_queue_gamma_map[0];
     
 }
 
 static int32_t reset_qos_shaper(int32_t shaperid, uint32_t flag)
 {
     
     if(shaperid < 0 || shaperid >= 4)
         return PPA_FAILURE;
 
     return set_qos_shaper(shaperid,1000000,default_qos_rateshaping_burst,flag);
     
 }
 
 /*!
      \brief This is to set Rate Shaper
      \param[in] shaperid the shaper instance id 
      \param[in] rate the maximum rate limit in kbps
      \param[in] burst the maximun burst in bytes
      \param[in] flag reserved for future
      \return The return value can be any one of the following:  \n
                 - IFX_SUCCESS on success \n
                 - IFX_FAILURE on error \n
 */
 int32_t set_qos_shaper( int32_t shaperid, uint32_t rate, uint32_t burst, uint32_t flag)
 {
     shaping_wfq_cfg_t qos_cfg = {0};
     int i;
     volatile shaping_wfq_cfg_t *p_qos_cfg = NULL;
 
     if( set_qos_port_id() != PPA_SUCCESS ) 
     	{
         return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom
     	}
 
     if(flag == 1){
         qos_cfg = *SHAPING_WFQ_CFG(shaperid + 16);
         p_qos_cfg = SHAPING_WFQ_CFG(shaperid + 16);
     }else if(shaperid >= 0 && shaperid < MAX_MBR_QOS_SHAPER_NUM){
         qos_cfg = *SHAPING_WFQ_CFG(SHAPERID_FROM_OUTQID(shaperid));
         p_qos_cfg = SHAPING_WFQ_CFG(SHAPERID_FROM_OUTQID(shaperid));
     }else{
         return PPA_FAILURE;
     }
 
     if ( rate >= bitrate_2_t_kbps_table[0].bitrate_kbps ){
         qos_cfg.t = bitrate_2_t_kbps_table[0].T;
     }
     else
     {
         for( i = 0; i < sizeof(bitrate_2_t_kbps_table) / sizeof(bitrate_2_t_kbps_table[0]) - 1; i++ ){
             if ( rate < bitrate_2_t_kbps_table[i].bitrate_kbps && rate >= bitrate_2_t_kbps_table[i+1].bitrate_kbps ){
                 qos_cfg.t = bitrate_2_t_kbps_table[i+1].T;
                 break;
             }
         }
     }
     if ( qos_cfg.t == 0 )
     {
         return PPA_FAILURE;
     }
     if( burst == 0 )
     {
         burst = default_qos_rateshaping_burst;
     }
 
     qos_cfg.r = IFX_PPA_DRV_QOS_RATESHAPE_bitrate_2_R( rate, qos_cfg.t, get_basic_time_tick() );
     qos_cfg.s = burst;
 
     *p_qos_cfg = qos_cfg;
 
  
      return PPA_SUCCESS;
 }
  
  /*!
      \brief This is to get Rate Shaper
      \param[in] shaperid the shaper id need to set rate shaping \n
      \param[out] rate the maximum rate limit in kbps
      \param[out] burst the maximun burst in bytes
      \param[in] flag reserved for future
      \return The return value can be any one of the following:  \n
                 - IFX_SUCCESS on success \n
                 - IFX_FAILURE on error \n
  */
 int32_t get_qos_shaper( int32_t shaperid, uint32_t *rate, uint32_t *burst, uint32_t flag)
 {
     shaping_wfq_cfg_t qos_cfg = {0};
 
     if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //A5 Ethernet WAN mode only QOS Note: Different condition with different mode/platfrom
 
     if(shaperid < 0 || shaperid >= MAX_MBR_QOS_SHAPER_NUM)
         return PPA_FAILURE;
     
     qos_cfg = *SHAPING_WFQ_CFG(SHAPERID_FROM_OUTQID(shaperid)); 
     if ( qos_cfg.t != 0 )   //not set yet
     {
         if ( rate )
             *rate = IFX_PPA_DRV_QOS_RATESHAPE_R_2_bitrate(qos_cfg.r, qos_cfg.t, get_basic_time_tick());
     }
     else
     {
         if ( rate )
             *rate = 0;
     }
     if ( burst )
         *burst = qos_cfg.s;
 
     return PPA_SUCCESS;
 }
#endif //end of MBR_CONFIG

 #endif /*end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING*/

 #ifdef CONFIG_LTQ_PPA_QOS_WFQ

/*!
    \brief This is to eanble/disable WFQ feature
    \param[in] portid the phisical port id which support qos queue
    \param[in] enabled 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t set_ctrl_qos_wfq( uint32_t portid, uint32_t enable, uint32_t flag)
{
#if 0
    qos_cfg_t tx_qos_cfg = {0};

    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;
    tx_qos_cfg = *QOS_CFG;

    if ( enable )
    {
        tx_qos_cfg.wfq_en= 1;
        tx_qos_cfg.overhd_bytes = 24; //add 24 bytes preamble and inter-frame gape
        //tx_qos_cfg.eth1_qss = 1;

        if( flag != 0 )
            wfq_multiple= flag;
        else
            wfq_multiple= IFX_PPA_DRV_QOS_WFQ_WLEVEL_2_W;
    }
    else
    {
        tx_qos_cfg.wfq_en = 0;

        if ( !tx_qos_cfg.shape_en && !tx_qos_cfg.wfq_en )
        {
            //tx_qos_cfg.eth1_qss = 1;
            //tx_qos_cfg.qosq_num = 0;
        }
    }

    *QOS_CFG = tx_qos_cfg;
#else
    int i;

    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;

    if ( enable )
    {
        WFQ_CFG->wfq_reload_enable_map = (1 << QOS_CFG->qosq_num) - 1;
        QOS_CFG->qos_en = 1;
        for ( i = 0; i < 4; i++ )
            //if ( OUTQ_QOS_CFG_CTXT(i)->qmap ) //enable output queue directly
            {
                OUTQ_QOS_CFG_CTXT(i)->qos_en = 1;
                OUTQ_QOS_CFG_CTXT(i)->wfq_en = 1;
            }

        if ( flag != 0 )
            wfq_multiple = flag;
        else
            wfq_multiple = IFX_PPA_DRV_QOS_WFQ_WLEVEL_2_W;
    }
    else
    {
        WFQ_CFG->wfq_reload_enable_map = 0x0000;
        for ( i = 0; i < 4; i++ )
            OUTQ_QOS_CFG_CTXT(i)->wfq_en = 0;
        if ( SHAPING_CFG->qosq_shaper_enable_map == 0 )
        {
            QOS_CFG->qos_en = 0;
            for ( i = 0; i < 4; i++ )
                OUTQ_QOS_CFG_CTXT(i)->qos_en = 0;
        }
    }
#endif

    return PPA_SUCCESS;
}

/*!
    \brief This is to get WFQ feature status: enabled or disabled
    \param[in] portid the phisical port id which support qos queue
    \param[out] enabled 1:enable 0: disable
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t get_ctrl_qos_wfq( uint32_t portid, uint32_t *enable, uint32_t flag)
{
    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;
    if ( enable )
    {
//        if ( QOS_CFG->wfq_en )
        if ( WFQ_CFG->wfq_reload_enable_map )
            *enable = 1;
        else
            *enable =0;
    }

    return PPA_SUCCESS;
}

/*!
    \brief This is to set WFQ weight level for one specified port and queue
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set WFQ
    \param[in] weight_level the value should be 0 ~ 100. It will be mapped to internal PPE FW WFQ real weight
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t set_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t weight_level, uint32_t flag)
{
    shaping_wfq_cfg_t qos_cfg = {0};

    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;
    if ( queueid >= QOS_CFG->qosq_num )
        return PPA_FAILURE;

    qos_cfg = *WTX_EG_Q_SHAPING_CFG(queueid);

    if ( weight_level == 100 )
    {
        if( wfq_multiple != 1 )
            qos_cfg.w = wfq_strict_pri_weight;
        else
            qos_cfg.w = weight_level * wfq_multiple;
    }
    else
    {
        qos_cfg.w = weight_level * wfq_multiple;
    }

    *WTX_EG_Q_SHAPING_CFG(queueid) = qos_cfg;
    WFQ_CFG->wfq_force_reload_map |= 1 << queueid;
    if( weight_level < 100 ) { // WFQ
        WFQ_CFG->wfq_reload_enable_map |= 1 << queueid;
    }else{ //strict priority
        WFQ_CFG->wfq_reload_enable_map &= ~(1 << queueid);
    }
    return PPA_SUCCESS;
}

/*!
    \brief This is to get WFQ settings for one specified port and queue ( default value should be 0xFFFF)
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set WFQ
    \param[out] weight_level the value should be 0 ~ 100.
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t get_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t *weight_level, uint32_t flag)
{
    shaping_wfq_cfg_t qos_cfg = {0};

    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    if ( qos_queue_portid != portid )
        return PPA_FAILURE;
    if ( queueid >= QOS_CFG->qosq_num )
        return PPA_FAILURE;
    if ( !weight_level )
        return PPA_FAILURE;

    qos_cfg = *WTX_EG_Q_SHAPING_CFG(queueid);

    if ( qos_cfg.w == wfq_strict_pri_weight )
    {
        if( wfq_multiple != 1 )
            *weight_level = 100;
        else
            *weight_level = qos_cfg.w / wfq_multiple;

    }
    else
    {
        *weight_level = qos_cfg.w / wfq_multiple;
    }

    return PPA_SUCCESS;
}

/*!
    \brief This is to reset WFQ for one specified port and queue ( default value should be 0xFFFF)
    \param[in] portid the phisical port id which support qos queue
    \param[in] queueid the queue id need to set WFQ
    \param[in] flag reserved for future
    \return The return value can be any one of the following:  \n
               - PPA_SUCCESS on success \n
               - PPA_FAILURE on error \n
*/
static int32_t reset_qos_wfq( uint32_t portid, uint32_t queueid, uint32_t flag )
{
    return set_qos_wfq(portid, queueid, 100, flag);
}

static int32_t init_qos_wfq(void)
{
    int i;
    if( set_qos_port_id() != PPA_SUCCESS ) return PPA_FAILURE; //QOS Note: Different condition with different mode/platfrom

    for ( i = 0; i < get_qos_qnum(qos_queue_portid, 0); i++ )
        reset_qos_wfq(qos_queue_portid, i, 0);

    return PPA_SUCCESS;
}

 #endif /*end of CONFIG_LTQ_PPA_QOS_WFQ*/

int32_t vrx218_qos_ppe_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    switch (cmd) {
#ifdef CONFIG_LTQ_PPA_QOS
    case PPA_GENERIC_HAL_GET_QOS_STATUS:    //get QOS status
        {
            return get_qos_status((PPA_QOS_STATUS *)buffer);
        }
    case PPA_GENERIC_HAL_GET_QOS_QUEUE_NUM:  //get maximum QOS queue number
        {
            PPE_QOS_COUNT_CFG *count=(PPE_QOS_COUNT_CFG *)buffer;
            count->num = get_qos_qnum( count->portid, count->flags );
            return PPA_SUCCESS;
        }
    case PPA_GENERIC_HAL_GET_QOS_MIB:  //get maximum QOS queue number
        {
            PPE_QOS_MIB_INFO *mib_info=(PPE_QOS_MIB_INFO *)buffer;
            return get_qos_mib(mib_info->portid, mib_info->queueid, &mib_info->mib, mib_info->flag );
        }
 #ifdef CONFIG_LTQ_PPA_QOS_WFQ
    case PPA_GENERIC_HAL_SET_QOS_WFQ_CTRL:  //enable/disable WFQ
        {
            PPE_QOS_ENABLE_CFG *enable_cfg=(PPE_QOS_ENABLE_CFG *)buffer;
            return set_ctrl_qos_wfq( enable_cfg->portid, enable_cfg->f_enable, enable_cfg->flag );
        }
    case PPA_GENERIC_HAL_GET_QOS_WFQ_CTRL:  //get  WFQ status: enabeld/disabled
        {
            PPE_QOS_ENABLE_CFG *enable_cfg=(PPE_QOS_ENABLE_CFG *)buffer;
            return get_ctrl_qos_wfq( enable_cfg->portid, &enable_cfg->f_enable, enable_cfg->flag );
        }
    case PPA_GENERIC_HAL_SET_QOS_WFQ_CFG:  //set WFQ cfg
        {
            PPE_QOS_WFQ_CFG *cfg=(PPE_QOS_WFQ_CFG *)buffer;
            return set_qos_wfq( cfg->portid, cfg->queueid, cfg->weight_level, cfg->flag );
        }
    case PPA_GENERIC_HAL_RESET_QOS_WFQ_CFG:  //reset WFQ cfg
        {
            PPE_QOS_WFQ_CFG *cfg=(PPE_QOS_WFQ_CFG *)buffer;
            return reset_qos_wfq( cfg->portid, cfg->queueid, cfg->flag );
        }
    case PPA_GENERIC_HAL_GET_QOS_WFQ_CFG:  //get WFQ cfg
        {
            PPE_QOS_WFQ_CFG *cfg=(PPE_QOS_WFQ_CFG *)buffer;
            return get_qos_wfq( cfg->portid, cfg->queueid, &cfg->weight_level, cfg->flag );
        }
    case PPA_GENERIC_HAL_INIT_QOS_WFQ: // init QOS Rateshapping
        {
            return init_qos_wfq();
        }
 #endif //end of CONFIG_LTQ_PPA_QOS_WFQ
 #ifdef CONFIG_LTQ_PPA_QOS_RATE_SHAPING
    case PPA_GENERIC_HAL_SET_QOS_RATE_SHAPING_CTRL:  //enable/disable Rate shaping
        {
            PPE_QOS_ENABLE_CFG *enable_cfg=(PPE_QOS_ENABLE_CFG *)buffer;
            return set_ctrl_qos_rate( enable_cfg->portid, enable_cfg->f_enable, enable_cfg->flag );
        }
    case PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CTRL:  //get  Rateshaping status: enabeld/disabled
        {
            PPE_QOS_ENABLE_CFG *enable_cfg=(PPE_QOS_ENABLE_CFG *)buffer;
            return get_ctrl_qos_rate( enable_cfg->portid, &enable_cfg->f_enable, enable_cfg->flag );
        }
    case PPA_GENERIC_HAL_SET_QOS_RATE_SHAPING_CFG:  //set rate shaping
        {
            PPE_QOS_RATE_SHAPING_CFG*cfg=(PPE_QOS_RATE_SHAPING_CFG *)buffer;
#if defined(MBR_CONFIG) && MBR_CONFIG
            return set_qos_rate( cfg->portid, cfg->queueid, cfg->shaperid,cfg->rate_in_kbps, cfg->burst, cfg->flag );
#else
            return set_qos_rate( cfg->portid, cfg->queueid, cfg->rate_in_kbps, cfg->burst, cfg->flag );
#endif
        }
    case PPA_GENERIC_HAL_GET_QOS_RATE_SHAPING_CFG:  //get rate shaping cfg
        {
            PPE_QOS_RATE_SHAPING_CFG*cfg=(PPE_QOS_RATE_SHAPING_CFG *)buffer;
#if defined(MBR_CONFIG) && MBR_CONFIG
            return get_qos_rate( cfg->portid, cfg->queueid, &cfg->shaperid, &cfg->rate_in_kbps, &cfg->burst, cfg->flag );
#else
            return get_qos_rate( cfg->portid, cfg->queueid, &cfg->rate_in_kbps, &cfg->burst, cfg->flag );
#endif
        }
    case PPA_GENERIC_HAL_RESET_QOS_RATE_SHAPING_CFG:  //reset rate shaping cfg
        {
            PPE_QOS_RATE_SHAPING_CFG*cfg=(PPE_QOS_RATE_SHAPING_CFG *)buffer;
            return reset_qos_rate( cfg->portid, cfg->queueid, cfg->flag );
        }
    case PPA_GENERIC_HAL_INIT_QOS_RATE_SHAPING: // init QOS Rateshapping
        {
            return init_qos_rate();
        }
#if defined(MBR_CONFIG) && MBR_CONFIG
    case PPA_GENERIC_HAL_SET_QOS_SHAPER_CFG:  //set shaper 
        {
            PPE_QOS_RATE_SHAPING_CFG*cfg=(PPE_QOS_RATE_SHAPING_CFG *)buffer;
            return set_qos_shaper( cfg->shaperid, cfg->rate_in_kbps, cfg->burst, cfg->flag );
        }
    case PPA_GENERIC_HAL_GET_QOS_SHAPER_CFG:  //get shaper
        {
            PPE_QOS_RATE_SHAPING_CFG*cfg=(PPE_QOS_RATE_SHAPING_CFG *)buffer;
            return get_qos_shaper( cfg->shaperid, &cfg->rate_in_kbps, &cfg->burst, cfg->flag );
        }
    case PPA_GENERIC_HAL_RESET_QOS_SHAPER_CFG:
        {
            PPE_QOS_RATE_SHAPING_CFG *cfg = (PPE_QOS_RATE_SHAPING_CFG *)buffer;
            return reset_qos_shaper(cfg->shaperid,cfg->flag);
        }
#endif //end of MBR_CONFIG

 #endif //end of CONFIG_LTQ_PPA_QOS_RATE_SHAPING
#endif //end of CONFIG_LTQ_PPA_QOS
    default:
        break;
    }

    return PPA_FAILURE;
}
#endif  /*end of CONFIG_LTQ_PPA_QOS*/
