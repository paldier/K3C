#ifndef PPA_DATAPATH_H
#define PPA_DATAPATH_H



/******************************************************************************
**
** FILE NAME    : ppa_datapath.h
** PROJECT      : UEIP
** MODULES      : Acceleration Package (PPA A4/D4/A5/D5)
**
** DATE         : 2 SEP 2009
** AUTHOR       : Xu Liang
** DESCRIPTION  : Acceleration Package Data Path Header File
** COPYRIGHT    :   Copyright (c) 2006
**          Infineon Technologies AG
**          Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
**  2 SEP 2009  Xu Liang        Initiate Version
*******************************************************************************/



#include <net/ppa_ppe_hal.h>
#include <linux/version.h>
#ifdef CONFIG_LTQ_ETHSW_API
#include <net/lantiq_ethsw.h>
#endif
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
#include <net/datapath_api.h>
#include <net/ltq_mpe_hal.h>
#include <net/ltq_tmu_hal_api.h>
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 8, 13)

#include <asm/io.h>
#ifdef CONFIG_LANTIQ
#include <lantiq_irq.h>
#endif

#define INT_NUM_IM0_IRL24               (INT_NUM_IRQ0 + 24)

#define INT_NUM_IM1_IRL29               (INT_NUM_IM1_IRL0 + 29)

#define INT_NUM_IM2_IRL0                (INT_NUM_IRQ0 + 64)
#define INT_NUM_IM2_IRL23               (INT_NUM_IM2_IRL0 + 23)
#define INT_NUM_IM2_IRL24               (INT_NUM_IM2_IRL0 + 24)


/*
 *  find first 1 from MSB in a 32-bit word
 *  if all ZERO, return -1
 *  e.g. 0x10000000 => 28
 */
static inline __s32 clz(__u32 x)
{
    __asm__ (
    "       .set    push                                    \n"
    "       .set    mips32                                  \n"
    "       clz     %0, %1                                  \n"
    "       .set    pop                                     \n"
    : "=r" (x)
    : "r" (x));

    return 31 - (__s32)x;
}


/*
 *  Register Operation
 */
#define IFX_REG_R32(_r)                    __raw_readl((volatile unsigned int *)(_r))
#define IFX_REG_W32(_v, _r)               __raw_writel((_v), (volatile unsigned int *)(_r))
#define IFX_REG_W32_MASK(_clr, _set, _r)   IFX_REG_W32((IFX_REG_R32((_r)) & ~(_clr)) | (_set), (_r))
#define IFX_REG_R16(_r)                    __raw_readw((_r))
#define IFX_REG_W16(_v, _r)               __raw_writew((_v), (_r))
#define IFX_REG_W16_MASK(_clr, _set, _r)   IFX_REG_W16((IFX_REG_R16((_r)) & ~(_clr)) | (_set), (_r))
#define IFX_REG_R8(_r)                     __raw_readb((_r))
#define IFX_REG_W8(_v, _r)                __raw_writeb((_v), (_r))
#define IFX_REG_W8_MASK(_clr, _set, _r)    IFX_REG_W8((IFX_REG_R8((_r)) & ~(_clr)) | (_set), (_r))

/*
 * Register manipulation macros that expect bit field defines
 * to follow the convention that an _S suffix is appended for
 * a shift count, while the field mask has no suffix. Or can use
 * _M as suffix
 */

/* Shift first, then mask, usually for write operation */
#define SM(_v, _f)  (((_v) << _f##_S) & (_f))

/* Mask first , then shift, usually for read operation */
#define MS(_v, _f)  (((_v) & (_f)) >> _f##_S)

#define IFX_REG_RMW32(_set, _clr, _r)    \
    IFX_REG_W32((IFX_REG_R32((_r)) & ~(_clr)) | (_set), (_r))

#define IFX_REG_RMW32_FILED(_f, _v, _r) \
    IFX_REG_W32(\
        (IFX_REG_R32((_r)) &~ (_f)) | (((_v) << (_f##_S)) & (_f)), (_r))

#define IFX_REG_SET_BIT(_f, _r) \
    IFX_REG_W32((IFX_REG_R32((_r)) &~ (_f)) | (_f), (_r))

#define IFX_REG_CLR_BIT(_f, _r) \
    IFX_REG_W32(IFX_REG_R32((_r)) &~ (_f), (_r))

#define IFX_REG_IS_BIT_SET(_f, _r) \
    ((IFX_REG_R32((_r)) & (_f)) != 0)

/*
 *  Bits Operation
 */
#define GET_BITS(x, msb, lsb)               \
    (((x) >> (lsb)) & ((1 << ((msb) + 1 - (lsb))) - 1))
#define SET_BITS(x, msb, lsb, value)        \
    (((x) & ~(((1 << ((msb) + 1)) - 1) ^ ((1 << (lsb)) - 1))) | (((value) & ((1 << (1 + (msb) - (lsb))) - 1)) << (lsb)))

#endif


#ifdef CONFIG_LTQ_PPA_PORT_SEPARATION
#ifdef CONFIG_NETWORK_EXTMARK
#define  LAN_SKB_MARK(skb,header) SET_DATA_FROM_MARK_OPT(skb->mark,LAN_PORT_SEP_MASK,LAN_PORT_SEP_START_BIT_POS,(header->sppid + 1));
#else
#define LAN_SKB_MARK(skb,header) skb->mark = (header->sppid + 1 ) << 29
#endif
#define WAN_SKB_MARK(skb,header)  /* Do nothing */

/* The driver increments skb mark by 1 ( to avoid confuson between port 0 and packet coming from non-switch ). Hence decrease by 1. */
#ifdef CONFIG_NETWORK_EXTMARK
#define SET_LAN_SWITCH_PORT_MAP(skb,pkth) { \
						pkth.port_map_en	= 1; \
						GET_DATA_FROM_MARK_OPT(skb->mark,LAN_PORT_SEP_MASK,LAN_PORT_SEP_START_BIT_POS,pkth.port_map); \
					}
#else
#define SET_LAN_SWITCH_PORT_MAP(skb,pkth) { \
						pkth.port_map_en	= 1; \
						pkth.port_map		= 1 << ((skb->mark >> 29) -1); \
					}
#endif
#define SET_WAN_SWITCH_PORT_MAP(skb,pkth) /* Do nothing */
#define PPA_PORT_SEPARATION_TX(skb, pkth, lan_port_seperate_enabled, wan_port_seperate_enabled, port ) { \
											if( lan_port_seperate_enabled && port == 0  && ((skb->mark >> LAN_PORT_SEP_START_BIT_POS) != 0)) { \
												SET_LAN_SWITCH_PORT_MAP(skb,pkth); \
											} else if( wan_port_seperate_enabled && port == 1   )  { \
												SET_WAN_SWITCH_PORT_MAP(skb,pkth); \
											} \
										}
#define PPA_PORT_SEPARATION_RX(header,skb,lan_port_seperate_enabled,wan_port_seperate_enabled) { \
												    if ( header->src_itf == 0 ) { \
													if ( lan_port_seperate_enabled ) \
														LAN_SKB_MARK(skb,header); \
												    } else { \
													if ( wan_port_seperate_enabled ) \
														WAN_SKB_MARK(skb,header); \
												    } \
												}
#define PPA_PORT_SEPARATION_COPY_MARK(skb,new_skb) new_skb->mark = skb->mark
#else
#define PPA_PORT_SEPARATION_TX(skb, pkth, lan_port_seperate_enabled, wan_port_seperate_enabled, port) 
#define PPA_PORT_SEPARATION_RX(header,skb,lan_port_seperate_enabled,wan_port_seperate_enabled) { \
												skb->mark = header->sppid << 29; \
												}
#define PPA_PORT_SEPARATION_COPY_MARK(skb,new_skb)
#endif

/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  ATM ioctl Command
 */
#define PPE_ATM_IOC_MAGIC               'o'
#define PPE_ATM_MIB_CELL                _IOW(PPE_ATM_IOC_MAGIC,  0, atm_cell_ifEntry_t)
#define PPE_ATM_MIB_AAL5                _IOW(PPE_ATM_IOC_MAGIC,  1, atm_aal5_ifEntry_t)
#define PPE_ATM_MIB_VCC                 _IOWR(PPE_ATM_IOC_MAGIC, 2, atm_aal5_vcc_x_t)
#define PPE_ATM_MAP_PKT_PRIO_TO_Q       _IOR(PPE_ATM_IOC_MAGIC,  3, struct ppe_prio_q_map)
#define PPE_ATM_TX_Q_OP                 _IOR(PPE_ATM_IOC_MAGIC,  4, struct tx_q_op)
#define PPE_ATM_GET_MAP_PKT_PRIO_TO_Q   _IOWR(PPE_ATM_IOC_MAGIC, 5, struct ppe_prio_q_map_all)
#define PPE_ATM_IOC_MAXNR               6

#define PPE_ATM_TX_Q_OP_CHG_MASK        0x01
#define PPE_ATM_TX_Q_OP_ADD             0x02

/*
 *  PTM ioctl Command
 */
#define IFX_PTM_MIB_CW_GET              SIOCDEVPRIVATE + 1
#define IFX_PTM_MIB_FRAME_GET           SIOCDEVPRIVATE + 2
#define IFX_PTM_CFG_GET                 SIOCDEVPRIVATE + 3
#define IFX_PTM_CFG_SET                 SIOCDEVPRIVATE + 4
#define IFX_PTM_MAP_PKT_PRIO_TO_Q       ETH_MAP_PKT_PRIO_TO_Q

/*
 *  ethernet ioctl Command
 */
#define SET_VLAN_COS                    SIOCDEVPRIVATE + 0
#define SET_DSCP_COS                    SIOCDEVPRIVATE + 1
#define ENABLE_VLAN_CLASSIFICATION      SIOCDEVPRIVATE + 2
#define DISABLE_VLAN_CLASSIFICATION     SIOCDEVPRIVATE + 3
#define VLAN_CLASS_FIRST                SIOCDEVPRIVATE + 4
#define VLAN_CLASS_SECOND               SIOCDEVPRIVATE + 5
#define ENABLE_DSCP_CLASSIFICATION      SIOCDEVPRIVATE + 6
#define DISABLE_DSCP_CLASSIFICATION     SIOCDEVPRIVATE + 7
#define PASS_UNICAST_PACKETS            SIOCDEVPRIVATE + 8
#define FILTER_UNICAST_PACKETS          SIOCDEVPRIVATE + 9
#define KEEP_BROADCAST_PACKETS          SIOCDEVPRIVATE + 10
#define DROP_BROADCAST_PACKETS          SIOCDEVPRIVATE + 11
#define KEEP_MULTICAST_PACKETS          SIOCDEVPRIVATE + 12
#define DROP_MULTICAST_PACKETS          SIOCDEVPRIVATE + 13
#define ETH_MAP_PKT_PRIO_TO_Q           SIOCDEVPRIVATE + 14



/*
 * ####################################
 *              Data Type
 * ####################################
 */

/*
 *  ATM MIB
 */
typedef struct {
        __u32   ifHCInOctets_h;
        __u32   ifHCInOctets_l;
        __u32   ifHCOutOctets_h;
        __u32   ifHCOutOctets_l;
        __u32   ifInErrors;
        __u32   ifInUnknownProtos;
        __u32   ifOutErrors;
} atm_cell_ifEntry_t;

typedef struct {
        __u32   ifHCInOctets_h;
        __u32   ifHCInOctets_l;
        __u32   ifHCOutOctets_h;
        __u32   ifHCOutOctets_l;
        __u32   ifInUcastPkts;
        __u32   ifOutUcastPkts;
        __u32   ifInErrors;
        __u32   ifInDiscards;
        __u32   ifOutErros;
        __u32   ifOutDiscards;
} atm_aal5_ifEntry_t;

typedef struct {
        __u32   aal5VccCrcErrors;
        __u32   aal5VccSarTimeOuts;//no timer support yet
        __u32   aal5VccOverSizedSDUs;

        __u32   aal5VccRxPDU;
        __u32   aal5VccRxBytes;
        __u32   aal5VccRxCell;      //  reserved
        __u32   aal5VccRxOAM;       //  reserved
        __u32   aal5VccTxPDU;
        __u32   aal5VccTxBytes;
        __u32   aal5VccTxDroppedPDU;
        __u32   aal5VccTxCell;      //  reserved
        __u32   aal5VccTxOAM;       //  reserved
} atm_aal5_vcc_t;

/*
 *  Data Type Used to Call ATM ioctl
 */
typedef struct {
    int             vpi;
    int             vci;
    atm_aal5_vcc_t  mib_vcc;
} atm_aal5_vcc_x_t;

struct ppe_prio_q_map {     //  also used in ethernet ioctl
    int             pkt_prio;
    int             qid;
    int             vpi;    //  ignored in eth interface
    int             vci;    //  ignored in eth interface
};

struct tx_q_op {
    int             vpi;
    int             vci;
    unsigned int    flags;
};

struct ppe_prio_q_map_all {
    int             vpi;
    int             vci;
    int             total_queue_num;
    int             pkt_prio[8];
    int             qid[8];
};

/*
 *  Data Type Used to Call PTM ioctl
 */
typedef struct ptm_cw_ifEntry_t {
    uint32_t    ifRxNoIdleCodewords;    /*!< output, number of ingress user codeword */
    uint32_t    ifRxIdleCodewords;      /*!< output, number of ingress idle codeword */
    uint32_t    ifRxCodingViolation;    /*!< output, number of error ingress codeword */
    uint32_t    ifTxNoIdleCodewords;    /*!< output, number of egress user codeword */
    uint32_t    ifTxIdleCodewords;      /*!< output, number of egress idle codeword */
} PTM_CW_IF_ENTRY_T;

typedef struct ptm_frame_mib_t {
    uint32_t    RxCorrect;      /*!< output, number of ingress packet */
    uint32_t    TC_CrcError;    /*!< output, number of egress packet with CRC error */
    uint32_t    RxDropped;      /*!< output, number of dropped ingress packet */
    uint32_t    TxSend;         /*!< output, number of egress packet */
} PTM_FRAME_MIB_T;

typedef struct ptm_cfg_t {
    uint32_t    RxEthCrcPresent;    /*!< input/output, ingress packet has ETH CRC */
    uint32_t    RxEthCrcCheck;      /*!< input/output, check ETH CRC of ingress packet */
    uint32_t    RxTcCrcCheck;       /*!< input/output, check TC CRC of ingress codeword */
    uint32_t    RxTcCrcLen;         /*!< input/output, length of TC CRC of ingress codeword */
    uint32_t    TxEthCrcGen;        /*!< input/output, generate ETH CRC for egress packet */
    uint32_t    TxTcCrcGen;         /*!< input/output, generate TC CRC for egress codeword */
    uint32_t    TxTcCrcLen;         /*!< input/output, length of TC CRC of egress codeword */
} IFX_PTM_CFG_T;

/*
 *  Data Type Used to Call ethernet ioctl
 */
struct vlan_cos_req {
    int     pri;
    int     cos_value;
};

struct dscp_cos_req {
    int     dscp;
    int     cos_value;
};


/*
 * ####################################
 *             Declaration
 * ####################################
 */

#if defined(__KERNEL__)
  struct port_cell_info {
    unsigned int    port_num;
    unsigned int    tx_link_rate[2];
  };

  //extern void atm_set_cell_rate(int, u32);
  //extern int IFX_ATM_LED_Callback_Register(void (*)(void));
  //extern int IFX_ATM_LED_Callback_Unregister( void (*)(void));

  #if defined(CONFIG_AMAZON_SE)
    extern unsigned int ephy_read_mdio_reg(int, int);
    extern int ephy_write_mdio_reg (int, int, u32);
    extern int ephy_auto_negotiate(int);
  #endif

//import API from ppa_api_hook.c--start
extern  int (*ppa_drv_get_dslwan_qid_with_vcc_hook)(PPA_VCC *);
extern  int (*ppa_drv_get_atm_qid_with_pkt_hook)(struct sk_buff *, void *, int);
extern  int (*ppa_drv_get_netif_qid_with_pkt_hook)(struct sk_buff *, void *, int);
extern  int (*ppa_drv_ppe_clk_change_hook)(unsigned int, unsigned int);
extern  int (*ppa_drv_ppe_pwm_change_hook)(unsigned int, unsigned int); //  arg1 - parameter, arg2 - 1: clock gating on/off, 2: power gating on/off

#ifdef CONFIG_LTQ_PPA_API_DIRECTPATH
extern struct ppe_directpath_data *ppa_drv_g_ppe_directpath_data;
extern  int (*ppa_drv_directpath_send_hook)(uint32_t, PPA_BUF *, int32_t, uint32_t);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
extern  int (*ppa_drv_directpath_register_hook)(PPA_SUBIF *, PPA_NETIF *, PPA_DIRECTPATH_CB *, int32_t*, uint32_t);
extern int (*mpe_hal_feature_start_fn)(
                enum MPE_Feature_Type mpeFeature,
                uint32_t port_id,
                uint32_t * featureCfgBase,
                uint32_t flags);
extern int32_t(*tmu_hal_get_qos_mib_hook_fn)(
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                struct tmu_hal_qos_stats *qos_mib,
                uint32_t flag);
extern int32_t (*tmu_hal_get_csum_ol_mib_hook_fn)(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag);
extern int32_t (*tmu_hal_clear_csum_ol_mib_hook_fn)(
                struct tmu_hal_qos_stats *csum_mib,
                uint32_t flag);
extern int32_t (*tmu_hal_clear_qos_mib_hook_fn)(
                struct net_device *netdev,
                dp_subif_t *subif_id,
                int32_t queueid,
                uint32_t flag);
#endif
extern  int (*ppa_drv_directpath_rx_stop_hook)(uint32_t, uint32_t);
extern  int (*ppa_drv_directpath_rx_start_hook)(uint32_t, uint32_t);
#endif

extern int32_t (*ppa_drv_datapath_generic_hook)(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);
extern int32_t (*ppa_drv_datapath_mac_entry_setting)(uint8_t  *mac, uint32_t fid, uint32_t portid, uint32_t agetime, uint32_t st_entry , uint32_t action) ;
#endif  // __KERNEL__ 
/*
struct ltq_mei_atm_showtime_info {
		void *check_ptr;
		void *enter_ptr;
		void *exit_ptr;
		};
*/
#endif  // PPA_DATAPATH_H
