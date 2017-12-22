/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _LTQ_FLOW_CORE_H_
#define _LTQ_FLOW_CORE_H_

#define	PCE_ASSERT(t)	if ((t)) { 	\
	pr_err("%s:%s:%d (" # t ")\n", __FILE__, __func__, __LINE__); \
	return -1; }

#ifndef IFX_RETURN_PCE
   #define IFX_RETURN_PCE \
      {	\
         printk("ERROR:\n\tFile %s\n\tLine %d\n", __FILE__, __LINE__);	\
         return (-1);	\
      }
#endif

#define PORT_STATE_LISTENING			0
#define PORT_STATE_RX_ENABLE_TX_DISABLE	1
#define PORT_STATE_RX_DISABLE_TX_ENABLE	2
#define PORT_STATE_LEARNING		4
#define PORT_STATE_FORWARDING			7
#define RMON_EXTEND_TRAFFIC_FLOW_COUNT_1	0x28

#define MAX_PORT_NUMBER						12
#define VLAN_ACTIVE_TABLE_SIZE		64
#define MAC_TABLE_SIZE						2048
#define MULTI_SW_TABLE_SIZE				64
#define MULTI_HW_TABLE_SIZE				64
#define LTQ_SOC_CPU_PORT					6
#define LTQ_SOC_CAP_SEGMENT				256
#define VRX_PLATFORM_CAP_FID			64
#define RMON_COUNTER_OFFSET				64
#define LTQ_SOC_TOP_REG_OFFSET		0xC40
#define MAX_PACKET_LENGTH					9600
#define PCE_PKG_LNG_TBL_SIZE			16
#define PCE_DASA_MAC_TBL_SIZE			64
#define PCE_APPL_TBL_SIZE					64
#define PCE_IP_DASA_MSB_TBL_SIZE	16
#define PCE_IP_DASA_LSB_TBL_SIZE	64
#define PCE_PTCL_TBL_SIZE					32
#define PCE_PPPOE_TBL_SIZE				16
#define PCE_VLAN_ACT_TBL_SIZE			64
#define PCE_TABLE_SIZE 						64
#define PCE_MICRO_TABLE_SIZE			64
/* Pore redirect PCE rules set or port 0 (30), port 1 (31),
*	port 2 (32), port 3(33), port 4(34) and port 5(35) */
#define PORT_REDIRECT_PCE_RULE_INDEX		30
#define EAPOL_PCE_RULE_INDEX	60
#define BPDU_PCE_RULE_INDEX		61
#define MULTI_PCE_RULES_INDEX	40
#define LTQ_GSWIP_2_0					0x100
#define LTQ_GSWIP_2_1					0x021
#define LTQ_GSWIP_2_2					0x122
#define LTQ_GSWIP_2_2_ETC			0x022

/*PHY Reg 0x4 */
#define PHY_AN_ADV_10HDX			0x20
#define PHY_AN_ADV_10FDX			0x40
#define PHY_AN_ADV_100HDX			0x80
#define PHY_AN_ADV_100FDX			0x100
/*PHY Reg 0x9 */
#define PHY_AN_ADV_1000HDX		0x100
#define PHY_AN_ADV_1000FDX		0x200

#define DEFAULT_AGING_TIMEOUT	300
/* Define Aging Counter Mantissa Value */
#define AGETIMER_1_DAY				0xFB75
#define AGETIMER_1_HOUR				0xA7BA
#define AGETIMER_300_SEC			0xDF84
#define AGETIMER_10_SEC				0x784
#define AGETIMER_1_SEC				0xBF

/* Define Duplex Mode */
#define DUPLEX_AUTO				0
#define DUPLEX_EN					1
#define DUPLEX_DIS				3

#define INSTR				0
#define IPV6				1
#define LENACCU			2
/* parser's microcode output field type */
enum {
	OUT_MAC0 = 0,
	OUT_MAC1,
	OUT_MAC2,
	OUT_MAC3,
	OUT_MAC4,
	OUT_MAC5,
	OUT_ETHTYP,
	OUT_VTAG0,
	OUT_VTAG1,
	OUT_ITAG0,
	OUT_ITAG1,	/*10 */
	OUT_ITAG2,
	OUT_ITAG3,
	OUT_IP0,
	OUT_IP1,
	OUT_IP2,
	OUT_IP3,
	OUT_SIP0,
	OUT_SIP1,
	OUT_SIP2,
	OUT_SIP3,	/*20*/
	OUT_SIP4,
	OUT_SIP5,
	OUT_SIP6,
	OUT_SIP7,
	OUT_DIP0,
	OUT_DIP1,
	OUT_DIP2,
	OUT_DIP3,
	OUT_DIP4,
	OUT_DIP5,	/*30*/
	OUT_DIP6,
	OUT_DIP7,
	OUT_SESID,
	OUT_PROT,
	OUT_APP0,
	OUT_APP1,
	OUT_IGMP0,
	OUT_IGMP1,
	OUT_IPOFF,	/*39*/
	OUT_NONE	=	63
};

/* parser's microcode flag type */
enum {
	FLAG_ITAG = 0,
	FLAG_VLAN,
	FLAG_SNAP,
	FLAG_PPPOE,
	FLAG_IPV6,
	FLAG_IPV6FL,
	FLAG_IPV4,
	FLAG_IGMP,
	FLAG_TU,
	FLAG_HOP,
	FLAG_NN1,	/*10 */
	FLAG_NN2,
	FLAG_END,
	FLAG_NO,	/*13*/
};

typedef struct {
   u16 val_3;
   u16 val_2;
   u16 val_1;
   u16 val_0;
} pce_uc_row_t;

typedef pce_uc_row_t PCE_MICROCODE[PCE_MICRO_TABLE_SIZE];
/** Provides the address of the configured/fetched lookup table. */
typedef enum {
	/** Parser microcode table */
	PCE_PARS_INDEX				= 0x00,
	PCE_ACTVLAN_INDEX			= 0x01,
	PCE_VLANMAP_INDEX			= 0x02,
	PCE_PPPOE_INDEX				= 0x03,
	PCE_PROTOCOL_INDEX		= 0x04,
	PCE_APPLICATION_INDEX	= 0x05,
	PCE_IP_DASA_MSB_INDEX	= 0x06,
	PCE_IP_DASA_LSB_INDEX	= 0x07,
	PCE_PACKET_INDEX			= 0x08,
	PCE_PCP_INDEX					= 0x09,
	PCE_DSCP_INDEX				= 0x0A,
	PCE_MAC_BRIDGE_INDEX	= 0x0B,
	PCE_MAC_DASA_INDEX		= 0x0C,
	PCE_MULTICAST_SW_INDEX = 0x0D,
	PCE_MULTICAST_HW_INDEX = 0x0E,
	PCE_TFLOW_INDEX				= 0x0F,
	PCE_REMARKING_INDEX		= 0x10,
	PCE_QUEUE_MAP_INDEX		= 0x11,
	PCE_METER_INS_0_INDEX	= 0x12,
	PCE_METER_INS_1_INDEX	= 0x13,
	PCE_IPDALSB_INDEX			= 0x14,
	PCE_IPSALSB_INDEX			= 0x15,
	PCE_MACDA_INDEX				= 0x16,
	PCE_MACSA_INDEX				= 0x17,
	PCE_FLOWPTR_INDEX			= 0x18,
	PCE_PARS_INDIVIDUAL_INDEX	= 0x19,
	PCE_SPCP_INDEX				= 0x1A,
	PCE_MSTP_INDEX				= 0x1B,
	PCE_EGREMARK_INDEX		= 0x1C,
} lookup_table_address_t;

/** Description */
typedef enum {
  TABLE_ACCESS_OP_MODE_ADRD = 0,
  TABLE_ACCESS_OP_MODE_ADWR = 1,
  TABLE_ACCESS_OP_MODE_KSRD = 2,
  TABLE_ACCESS_OP_MODE_KSWR = 3
} table_access_op_mode_t;

typedef enum {
	LTQ_FLOW_DEV_INT	= 0,
	LTQ_FLOW_DEV_MAX
} gsw_devType_t;


typedef struct {
	u16		key[16];
	u16		mask;
	u16		val[16];
	u16		table;
	u16		table_index;
	u16		op_mode;
	u16		type:1;
	u16		valid:1;
	u16		group:4;
} pce_table_prog_t;


typedef struct {
	u16				pkg_lng;
	u16				pkg_lng_rng;
}	pce_pkt_length_t;

typedef struct {
	u8				mac[6];
	u16				mac_mask;
}	pce_dasa_prog_t;

typedef struct {
	u16				appl_data;
	u16				mask_range;
	u8				mask_range_type;
} pce_app_prog_t;

/* IP DA/SA MSB Table */
typedef struct {
	u8				ip_msb[8];
	u16				mask;
} pce_dasa_msb_t;

/* IP DA/SA LSB Table */
typedef struct {
   u8		ip_lsb[8];
   u16	mask;
} pce_dasa_lsb_t;

/* programme the Protocol Table */
typedef struct {
	union {
		u16			ethertype;
		struct {
			u16		protocol:8;
			u16		protocol_flags:8;
		} prot;
	}	key;
	union {
		u16			ethertype_mask;
		struct {
			u16		res:12;
			u16		protocol_mask:2;
			u16		protocol_flag_mask:2;
		} prot;
	} mask;
} pce_protocol_tbl_t;

/* PPPoE Table  */
typedef struct {
	u16	sess_id;
} pce_ppoe_tbl_t;

typedef struct {
	u16		dscp:7;
	u16		pcp:4;
	u16		stag_pcp_dei;
	u16		pkt_lng_idx:5;
	u16		dst_mac_addr_idx:8;
	u16		src_mac_addr_idx:8;
	u16		dst_appl_fld_idx:8;
	u16		src_appl_fld_idx:8;
	u16		dip_msb_idx:8;
	u16		dip_lsb_idx:8;
	u16		sip_msb_idx:8;
	u16		sip_lsb_idx:8;
	u16		ip_prot_idx:8;
	u16		ethertype_idx:8;
	u16		pppoe_idx:5;
	u16		vlan_idx:7;
	u16		svlan_idx:7;
	u16		port_id:8;
} pce_table_t;

typedef struct {
	/* table reference counter */
	u16		pkg_lng_tbl_cnt[PCE_PKG_LNG_TBL_SIZE];
	u16		dasa_mac_tbl_cnt[PCE_DASA_MAC_TBL_SIZE];
	u16		appl_tbl_cnt[PCE_APPL_TBL_SIZE];
	u16		ip_dasa_msb_tbl_cnt[PCE_IP_DASA_MSB_TBL_SIZE];
	u16		ip_dasa_lsb_tbl_cnt[PCE_IP_DASA_LSB_TBL_SIZE];
	u16		ptcl_tbl_cnt[PCE_PTCL_TBL_SIZE];
	u16		pppoe_tbl_cnt[PCE_PPPOE_TBL_SIZE];
	u16		vlan_act_tbl_cnt[PCE_VLAN_ACT_TBL_SIZE];
	/* cached tables */
	pce_pkt_length_t	pkg_lng_tbl[PCE_PKG_LNG_TBL_SIZE];
	pce_dasa_prog_t		dasa_mac_tbl[PCE_DASA_MAC_TBL_SIZE];
	pce_app_prog_t		appl_tbl[PCE_APPL_TBL_SIZE];
	pce_dasa_msb_t		ip_dasa_msb_tbl[PCE_IP_DASA_MSB_TBL_SIZE];
	pce_dasa_lsb_t		ip_dasa_lsb_tbl[PCE_IP_DASA_LSB_TBL_SIZE];
	pce_protocol_tbl_t	ptcl_tbl[PCE_PTCL_TBL_SIZE];
	pce_ppoe_tbl_t		pppoe_tbl[PCE_PPPOE_TBL_SIZE];
} pce_table_handle_t;

typedef struct {
	/* Parameter for the sub-tables */
	pce_table_handle_t	pce_sub_tbl;
	pce_table_t			pce_tbl[PCE_TABLE_SIZE];
	IFX_FLOW_PCE_action_t	pce_act[PCE_TABLE_SIZE];
	u8	pce_tbl_used[PCE_TABLE_SIZE];
} ltq_pce_table_t;

typedef struct {
	IFX_ETHSW_capType_t		Cap_Type;
	/* Description String */
	char	Desci[IFX_ETHSW_CAP_STRING_LEN];
} gsw_CapDesc_t;

typedef struct {
	IFX_ETHSW_STP_PortState_t		ifx_stp_state;
	IFX_ETHSW_8021X_portState_t	ifx_8021_state;
	u8	pen_reg;
	u8	pstate_reg;
	u8	lrnlim;
} port_stp_state_t;

typedef struct {
	/* Port Enable */
	ltq_bool_t		bPortEnable;
	/** Transparent Mode */
	ltq_bool_t		bTVM;
	/* Learning Limit Action */
	ltq_bool_t		bLearningLimitAction;
	/* Automatic MAC address table learning locking */
	ltq_bool_t		bLearningPortLocked;
	/* Automatic MAC address table learning limitation */
	u16						nLearningLimit;
	/* Port State */
	u16						nPortState;
	/* Port State for STP */
	IFX_ETHSW_STP_PortState_t	ifx_stp_state;
	/* Port State for 8021.x */
	IFX_ETHSW_8021X_portState_t	ifx_8021x_state;
} port_config_t;

typedef struct {
	/* 8021x Port Forwarding State */
	IFX_ETHSW_portForward_t		eForwardPort;
		/* STP port State */
	IFX_ETHSW_portForward_t	eSTPPortState;
	/* 8021X Forwarding Port ID*/
	u8	n8021X_ForwardPortId;
	/* STP Port ID */
	u16	nSTP_PortID;
} ltq_stp_8021x_t;

typedef struct {
	ltq_bool_t	valid;
	u16		vid;
	u32		fid;
	u16		pm;
	u16		tm;
	ltq_bool_t	reserved;
} vlan_active_table_t;

typedef struct {
	u16		SrcIp_MSB_Index;
	u16		DisIp_MSB_Index;
	u16		SrcIp_LSB_Index;
	u16		DisIp_LSB_Index;
	u16		PortMap;
	u16		eModeMember;
	ltq_bool_t		valid;
} multi_sw_table_t;


typedef struct {
	u16	eIGMP_Mode;
	ltq_bool_t	bIGMPv3;
	u16	eForwardPort;
	u8	nForwardPortId;
	ltq_bool_t	bCrossVLAN;
	u8	nClassOfService;
	multi_sw_table_t	multi_sw_table[MULTI_SW_TABLE_SIZE];
	u16	eRouterPort;
	u8	nSwTblSize;
} gsw_igmp_t;

typedef struct {
	gsw_devType_t	eDev;
	port_config_t				PortConfig[MAX_PORT_NUMBER];
	vlan_active_table_t	VLAN_Table[VLAN_ACTIVE_TABLE_SIZE];
	ltq_stp_8021x_t			STP_8021x_Config;
	gsw_igmp_t			IGMP_Flags;
	ltq_pce_table_t			PCE_Handler;
	void		*pRAL_Dev;
	ltq_bool_t	bVLAN_Aware;
	u8		nPortNumber;
	u8		nTotalPortNumber;
	u8		nManagementPortNumber;
	u32		MAC_AgeTimer;
	ltq_bool_t	bResetCalled;
	ltq_bool_t	bHW_InitCalled;
	u8		vlan_table_index;
	u16		mac_table_index;
	u8		multi_hw_table_index;
	u8		multi_sw_table_index;
	u8		nCPU_Port;
	u8		routerport_cnt;
	u8		meter_cnt;
	u8		num_of_queues; /* Number of priority queues . */
	u8		num_of_meters;  /* Number of traffic meters */
	u8		num_of_shapers; /* Number of traffic shapers */
	u8		num_of_pppoe; /* PPPoE table size  */
	u8		avlan_table_size; /* Active VLAN table size */
	u8		ip_pkt_lnt_size; /* IP packet length table size */
	u8		prot_table_size; /* Protocol table size */
	u8		mac_dasa_table_size; /* MAC DA/SA table size */
	u8		app_table_size;	/* Application table size */
	u8		ip_dasa_msb_table_size;	/* IP DA/SA MSB table size */
	u8		ip_dasa_lsb_table_size;	/*  IP DA/SA LSB table size*/
	u8		multi_table_size;	/* Multicast table size */
	u8		flow_table_size; /* Flow Aggregation table size */
	u16		mac_table_size; /* MAC bridging table size */
	u16		GSWIP_Ver;
	void __iomem *regbase;
#ifdef LTQ_ETHSW_API_COC
	void	*pPMCtx;
#endif
} ethsw_api_dev_t;

u8 find_active_vlan_index(void *pDevCtx, u16 vid);
int find_software_msb_tbl_entry(pce_table_handle_t *pTmHandle,	\
	pce_dasa_msb_t *pPar);
int pce_tm_ip_dasa_msb_tbl_write(pce_table_handle_t *pTmHandle,	\
	pce_dasa_msb_t *pPar);
int find_software_tbl_entry(pce_table_handle_t *pTmHandle,	\
	pce_dasa_lsb_t *pPar);
int pce_tm_ip_dasa_lsb_tbl_write(pce_table_handle_t *pTmHandle,	\
	pce_dasa_lsb_t *pPar);
int pce_table_init(ltq_pce_table_t *pPCEHandle);
int pce_tm_ip_dasa_msb_tbl_delete(pce_table_handle_t *pTmHandle, u32 index);
int pce_tm_ip_dasa_lsb_tbl_delete(pce_table_handle_t *pTmHandle, u32 index);
int pce_tm_ip_dasa_msb_tbl_idx_delete(pce_table_handle_t *pTmHandle, u32 index);
int get_pce_tm_vlan_act_tbl_index(pce_table_handle_t *pTmHandle, u8 index);
int pce_pattern_delete(ltq_pce_table_t *pHandle, u32 index);
int pce_tm_ip_dasa_lsb_tbl_idx_delete(pce_table_handle_t *pTmHandle, u32 index);
int pce_action_delete(ltq_pce_table_t *pHandle, u32 index);
int pce_rule_read(ltq_pce_table_t *pHandle, IFX_FLOW_PCE_rule_t *pPar);
int pce_rule_write(ltq_pce_table_t *pHandle, IFX_FLOW_PCE_rule_t *pPar);
ethsw_ret_t xwayflow_pce_table_write(void *pDevCtx, pce_table_prog_t *pData);
ethsw_ret_t xwayflow_pce_table_read(void *pDevCtx, pce_table_prog_t *pData);
ethsw_ret_t xwayflow_pce_table_key_write(void *pDevCtx, \
	pce_table_prog_t *pData);
ethsw_ret_t xwayflow_pce_table_key_read(void *pDevCtx,	\
	pce_table_prog_t *pData);

#endif    /* _LTQ_FLOW_CORE_H_ */
