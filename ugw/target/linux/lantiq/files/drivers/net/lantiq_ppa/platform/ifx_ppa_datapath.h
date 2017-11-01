#ifndef IFX_PPA_DATAPATH_H
#define IFX_PPA_DATAPATH_H



/******************************************************************************
**
** FILE NAME    : ifx_ppa_datapath.h
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



#include <net/ifx_ppa_ppe_hal.h>

#ifdef CONFIG_IFX_PPA_PORT_SEPARATION
#define LAN_SKB_MARK(skb,header) skb->mark = (header->sppid + 1 ) << 29
#define WAN_SKB_MARK(skb,header)  /* Do nothing */
#define SKB_MARK_CHECK(skb) (skb->mark >> 29)

/* The driver increments skb mark by 1 ( to avoid confuson between port 0 and packet coming from non-switch ). Hence decrease by 1. */
#define SET_LAN_SWITCH_PORT_MAP(skb,pkth) { \
						pkth.port_map_en	= 1; \
						pkth.port_map		= 1 << ((skb->mark >> 29) -1); \
					}
#define SET_WAN_SWITCH_PORT_MAP(skb,pkth) /* Do nothing */
#define PPA_PORT_SEPARATION_TX(skb, pkth, lan_port_seperate_enabled, wan_port_seperate_enabled, port ) { \ 
										if ( SKB_MARK_CHECK(skb) ) { \
											if( lan_port_seperate_enabled && port == 0   ) { \
												SET_LAN_SWITCH_PORT_MAP(skb,pkth); \
											} else if( wan_port_seperate_enabled && port == 1   )  { \
												SET_WAN_SWITCH_PORT_MAP(skb,pkth); \
											} \
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

  //extern void ifx_atm_set_cell_rate(int, u32);
  //extern int IFX_ATM_LED_Callback_Register(void (*)(void));
  //extern int IFX_ATM_LED_Callback_Unregister( void (*)(void));

  #if defined(CONFIG_AMAZON_SE)
    extern unsigned int ephy_read_mdio_reg(int, int);
    extern int ephy_write_mdio_reg (int, int, u32);
    extern int ephy_auto_negotiate(int);
  #endif

//import API from ifx_ppa_api_hook.c--start
extern  int (*ifx_ppa_drv_get_dslwan_qid_with_vcc_hook)(PPA_VCC *);
extern  int (*ifx_ppa_drv_get_atm_qid_with_pkt_hook)(struct sk_buff *, void *, int);
extern  int (*ifx_ppa_drv_get_netif_qid_with_pkt_hook)(struct sk_buff *, void *, int);
extern  int (*ifx_ppa_drv_ppe_clk_change_hook)(unsigned int, unsigned int);
extern  int (*ifx_ppa_drv_ppe_pwm_change_hook)(unsigned int, unsigned int); //  arg1 - parameter, arg2 - 1: clock gating on/off, 2: power gating on/off

#ifdef CONFIG_IFX_PPA_API_DIRECTPATH
extern struct ppe_directpath_data *ifx_ppa_drv_g_ppe_directpath_data;
extern  int (*ifx_ppa_drv_directpath_send_hook)(uint32_t, PPA_BUF *, int32_t, uint32_t);
extern  int (*ifx_ppa_drv_directpath_rx_stop_hook)(uint32_t, uint32_t);
extern  int (*ifx_ppa_drv_directpath_rx_start_hook)(uint32_t, uint32_t);
#endif

extern int32_t (*ifx_ppa_drv_datapath_generic_hook)(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag);
extern int32_t (*ifx_ppa_drv_datapath_mac_entry_setting)(uint8_t  *mac, uint32_t fid, uint32_t portid, uint32_t agetime, uint32_t st_entry , uint32_t action) ;
#endif  // IFX_PPA_DATAPATH_H

#endif
