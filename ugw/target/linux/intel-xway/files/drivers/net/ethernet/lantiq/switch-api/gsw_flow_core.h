/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#ifndef _LTQ_FLOW_CORE_H_
#define _LTQ_FLOW_CORE_H_

#define	PCE_ASSERT(t)	if ((t)) { \
	pr_err("%s:%s:%d (" # t ")\n", __FILE__, __func__, __LINE__); \
	return -1; }

#ifndef GSW_RETURN_PCE
	#define GSW_RETURN_PCE	\
		{	\
			printk("ERROR:\n\tFile %s\n\tLine %d\n", __FILE__, __LINE__);	\
			return -1; \
		}
#endif

#define PORT_STATE_LISTENING 0
#define PS_RENABLE_TDISABLE	1
#define PS_RDISABLE_TENABLE	2
#define PORT_STATE_LEARNING 4
#define PORT_STATE_FORWARDING 7
#define REX_TFLOW_CNT_1	0x28

#define MAX_PORT_NUMBER 12
#define VLAN_ACTIVE_TABLE_SIZE 64
/* #define MAC_TABLE_SIZE 2048 */
#define MC_PC_SIZE 64
#define GSW_2X_SOC_CPU_PORT 6
#define GSW_3X_SOC_CPU_PORT 0
#define LTQ_SOC_CAP_SEGMENT 256
#define VRX_PLATFORM_CAP_FID 64
#define RMON_COUNTER_OFFSET 64
#define GSW_TREG_OFFSET 0xC40
#define MAX_PACKET_LENGTH 9600
#define PCE_PKG_LNG_TBL_SIZE 16
#define PCE_DASA_MAC_TBL_SIZE 64
#define PCE_APPL_TBL_SIZE 64
#define PCE_FLAGS_TBL_SIZE 32
#define PCE_PAYLOAD_TBL_SIZE 64
#define IP_DASA_PC_MSIZE 16
#define IP_DASA_PC_LSIZE 64
#define PCE_PTCL_TBL_SIZE 32
#define PCE_PPPOE_TBL_SIZE 16
#define PCE_VLAN_ACT_TBL_SIZE 64
#define PCE_TABLE_SIZE 512
/*256 -- GSWIP3.0*/ /* 64 -- GSWIP2.2*/
#define PCE_MICRO_TABLE_SIZE 256
/* Pore redirect PCE rules set or port 0 (30), port 1 (31),*/
/*	port 2 (32), port 3(33), port 4(34) and port 5(35) */
#define PRD_PRULE_INDEX 30
#define EAPOL_PCE_RULE_INDEX	60
#define BPDU_PCE_RULE_INDEX 61
#define MPCE_RULES_INDEX	10
#define LTQ_GSWIP_2_0 0x100
#define LTQ_GSWIP_2_1 0x021
#define LTQ_GSWIP_2_2 0x122
#define LTQ_GSWIP_2_2_ETC	0x023
#define LTQ_GSWIP_3_0 0x030


/*PHY Reg 0x4 */
#define PHY_AN_ADV_10HDX 0x20
#define PHY_AN_ADV_10FDX 0x40
#define PHY_AN_ADV_100HDX 0x80
#define PHY_AN_ADV_100FDX 0x100
/*PHY Reg 0x9 */
#define PHY_AN_ADV_1000HDX 0x100
#define PHY_AN_ADV_1000FDX 0x200

#define DEFAULT_AGING_TIMEOUT	300
/* Define Aging Counter Mantissa Value */
#define AGETIMER_1_DAY 0xFB75
#define AGETIMER_1_HOUR 0xA7BA
#define AGETIMER_300_SEC 0xDF84
#define AGETIMER_10_SEC 0x784
#define AGETIMER_1_SEC 0xBF

/* Define Duplex Mode */
#define DUPLEX_AUTO 0
#define DUPLEX_EN 1
#define DUPLEX_DIS 3

#define INSTR 0
#define IPV6 1
#define LENACCU 2

/* parser's microcode output field type */
enum {
	OUT_MAC0 = 0,
	OUT_MAC1,
	OUT_MAC2,
	OUT_MAC3,
	OUT_MAC4,
	OUT_MAC5,
	OUT_ITAG0,
	OUT_ITAG1,
	OUT_ITAG2,
	OUT_ITAG3,
	OUT_1VTAG0, /* 10 */
	OUT_1VTAG1,
	OUT_2VTAG0,
	OUT_2VTAG1,
	OUT_3VTAG0,
	OUT_3VTAG1,
	OUT_4VTAG0,
	OUT_4VTAG1,
	OUT_ETYPE,
	OUT_PPPOE0,
	OUT_PPPOE1, /* 20 */
	OUT_PPPOE3,
	OUT_PPP,
	OUT_RES,
	OUT_1IP0,
	OUT_1IP1,
	OUT_1IP2,
	OUT_1IP3,
	OUT_1IP4,
	OUT_1IP5,
	OUT_1IP6, /* 30 */
	OUT_1IP7,
	OUT_1IP8,
	OUT_1IP9,
	OUT_1IP10,
	OUT_1IP11,
	OUT_1IP12,
	OUT_1IP13,
	OUT_1IP14,
	OUT_1IP15,
	OUT_1IP16, /* 40 */
	OUT_1IP17,
	OUT_1IP18,
	OUT_1IP19,
	OUT_2IP0,
	OUT_2IP1,
	OUT_2IP2,
	OUT_2IP3,
	OUT_2IP4,
	OUT_2IP5,
	OUT_2IP6, /* 50 */
	OUT_2IP7,
	OUT_2IP8,
	OUT_2IP9,
	OUT_2IP10,
	OUT_2IP11,
	OUT_2IP12,
	OUT_2IP13,
	OUT_2IP14,
	OUT_2IP15,
	OUT_2IP16, /* 60 */
	OUT_2IP17,
	OUT_2IP18,
	OUT_2IP19,
	OUT_APP0,
	OUT_APP1,
	OUT_APP2,
	OUT_APP3,
	OUT_APP4,
	OUT_APP5,
	OUT_APP6, /* 70 */
	OUT_APP7,
	OUT_APP8,
	OUT_APP9,
	OUT_1PL,
	OUT_2PL,
	OUT_1LNH,
	OUT_2LNH = 77,
	OUT_NONE = 127
};

/* parser's microcode flag type */
enum {
	FLAG_NO = 0,
	FLAG_END,
	FLAG_CAPWAP,
	FLAG_GRE,
	FLAG_LEN,
	FLAG_GREK,
	FLAG_NN1,
	FLAG_NN2,
	FLAG_ITAG,
	FLAG_1VLAN,
	FLAG_2VLAN,  /* 10 */
	FLAG_3VLAN,
	FLAG_4VLAN,
	FLAG_SNAP,
	FLAG_PPPOES,
	FLAG_1IPV4,
	FLAG_1IPV6,
	FLAG_2IPV4,
	FLAG_2IPV6,
	FLAG_ROUTEXP,
	FLAG_TCP,  /* 20 */
	FLAG_1UDP,
	FLAG_IGMP,
	FLAG_IPV4OPT,
	FLAG_1IPV6EXT,
	FLAG_TCPACK,
	FLAG_IPFRAG,
	FLAG_EAPOL,
	FLAG_2IPV6EXT,
	FLAG_2UDP,
	FLAG_L2TPNEXP,  /* 30 */
	FLAG_LROEXP,
	FLAG_L2TP,
	FLAG_GRE_VLAN1,
	FLAG_GRE_VLAN2,
	FLAG_GRE_PPPOE,
	FLAG_NN13,
	FLAG_NN14,
	FLAG_NN15,
	FLAG_NN16,
	FLAG_NN17, /* 40 */
	FLAG_NN18,
	FLAG_NN19,
	FLAG_NN20,
	FLAG_NN21,
	FLAG_NN22,
	FLAG_NN23,
	FLAG_NN24,
	FLAG_NN25,
	FLAG_NN26,
	FLAG_NN27, /* 50 */
	FLAG_NN28,
	FLAG_NN29,
	FLAG_NN30,
	FLAG_NN31,
	FLAG_NN32,
	FLAG_NN33,
	FLAG_NN34,
	FLAG_NN35,
	FLAG_NN36,
	FLAG_NN37, /* 60 */
	FLAG_NN38,
	FLAG_NN39,
	FLAG_NN40,
};

/* GSWIP_2.X*/
enum {
	GOUT_MAC0 = 0,
	GOUT_MAC1,
	GOUT_MAC2,
	GOUT_MAC3,
	GOUT_MAC4,
	GOUT_MAC5,
	GOUT_ETHTYP,
	GOUT_VTAG0,
	GOUT_VTAG1,
	GOUT_ITAG0,
	GOUT_ITAG1,	/*10 */
	GOUT_ITAG2,
	GOUT_ITAG3,
	GOUT_IP0,
	GOUT_IP1,
	GOUT_IP2,
	GOUT_IP3,
	GOUT_SIP0,
	GOUT_SIP1,
	GOUT_SIP2,
	GOUT_SIP3,	/*20*/
	GOUT_SIP4,
	GOUT_SIP5,
	GOUT_SIP6,
	GOUT_SIP7,
	GOUT_DIP0,
	GOUT_DIP1,
	GOUT_DIP2,
	GOUT_DIP3,
	GOUT_DIP4,
	GOUT_DIP5,	/*30*/
	GOUT_DIP6,
	GOUT_DIP7,
	GOUT_SESID,
	GOUT_PROT,
	GOUT_APP0,
	GOUT_APP1,
	GOUT_IGMP0,
	GOUT_IGMP1,
	GOUT_IPOFF,	/*39*/
	GOUT_NONE	=	63,
};

/* parser's microcode flag type */
enum {
	GFLAG_ITAG = 0,
	GFLAG_VLAN,
	GFLAG_SNAP,
	GFLAG_PPPOE,
	GFLAG_IPV6,
	GFLAG_IPV6FL,
	GFLAG_IPV4,
	GFLAG_IGMP,
	GFLAG_TU,
	GFLAG_HOP,
	GFLAG_NN1,	/*10 */
	GFLAG_NN2,
	GFLAG_END,
	GFLAG_NO,	/*13*/
};

typedef struct {
	u16 val_3;
	u16 val_2;
	u16 val_1;
	u16 val_0;
} pce_uc_row_t;

typedef struct {
	u16 val[8];
	u16 ptaddr;
	u16 ptcaddr;
	u16 op_mode;
/*	u16 valid:1; */
} pmtbl_prog_t;

typedef enum {
	/** Parser microcode table */
	PMAC_BPMAP_INDEX = 0x00,
	PMAC_IGCFG_INDEX = 0x01,
	PMAC_EGCFG_INDEX = 0x02,
} pm_tbl_cmds_t;

/** Description */
typedef enum {
	PMAC_OPMOD_READ = 0,
	PMAC_OPMOD_WRITE = 1,
} pm_opcode_t;

typedef pce_uc_row_t PCE_MICROCODE[PCE_MICRO_TABLE_SIZE];
/** Provides the address of the configured/fetched lookup table. */
typedef enum {
	/** Parser microcode table */
	PCE_PARS_INDEX = 0x00,
	PCE_ACTVLAN_INDEX = 0x01,
	PCE_VLANMAP_INDEX = 0x02,
	PCE_PPPOE_INDEX = 0x03,
	PCE_PROTOCOL_INDEX = 0x04,
	PCE_APPLICATION_INDEX	= 0x05,
	PCE_IP_DASA_MSB_INDEX	= 0x06,
	PCE_IP_DASA_LSB_INDEX	= 0x07,
	PCE_PACKET_INDEX = 0x08,
	PCE_PCP_INDEX = 0x09,
	PCE_DSCP_INDEX = 0x0A,
	PCE_MAC_BRIDGE_INDEX	= 0x0B,
	PCE_MAC_DASA_INDEX = 0x0C,
	PCE_MULTICAST_SW_INDEX = 0x0D,
	PCE_MULTICAST_HW_INDEX = 0x0E,
	PCE_TFLOW_INDEX = 0x0F,
	PCE_REMARKING_INDEX = 0x10,
	PCE_QUEUE_MAP_INDEX = 0x11,
	PCE_METER_INS_0_INDEX	= 0x12,
	PCE_METER_INS_1_INDEX	= 0x13,
	PCE_IPDALSB_INDEX = 0x14,
	PCE_IPSALSB_INDEX = 0x15,
	PCE_MACDA_INDEX = 0x16,
	PCE_MACSA_INDEX = 0x17,
	PCE_PARSER_FLAGS_INDEX = 0x18,
	PCE_PARS_INDIVIDUAL_INDEX	= 0x19,
	PCE_SPCP_INDEX = 0x1A,
	PCE_MSTP_INDEX = 0x1B,
	PCE_EGREMARK_INDEX = 0x1C,
	PCE_PAYLOAD_INDEX = 0x1D,
	PCE_EG_VLAN_INDEX = 0x1E,
} ptbl_cmds_t;

/** Description */
typedef enum {
	PCE_OP_MODE_ADRD = 0,
	PCE_OP_MODE_ADWR = 1,
	PCE_OP_MODE_KSRD = 2,
	PCE_OP_MODE_KSWR = 3
} ptbl_opcode_t;

typedef enum {
	LTQ_FLOW_DEV_INT	= 0,
	LTQ_FLOW_DEV_INT_R	= 1,
	LTQ_FLOW_DEV_MAX
} gsw_devtype_t;

typedef struct {
	u16 key[16];
	u16 mask[4];
	u16 val[16];
	u16 table;
	u16 pcindex;
	u16 op_mode:2;
	u16 extop:1;
	u16 kformat:1;
	u16 type:1;
	u16 valid:1;
	u16 group:4;
} pctbl_prog_t;

typedef struct {
	u16 pkg_lng;
	u16 pkg_lng_rng;
}	pce_pkt_length_t;

typedef struct {
	u8 mac[6];
	u16 mac_mask;
}	pce_sa_prog_t;

typedef struct {
	u8 mac[6];
	u16 mac_mask;
}	pce_da_prog_t;

typedef struct {
	u16 appl_data;
	u16 mask_range;
	u8 mask_range_type;
} app_tbl_t;

typedef struct {
	u16 parser_flag_data;
	u16 mask_value;
	u8 valid;
} flag_tbl_t;

typedef struct {
	u16 payload_data;
	u16 mask_range;
	u8 mask_range_type:1;
	u8 valid:1;
} payload_tbl_t;

/* IP DA/SA MSB Table */
typedef struct {
	u8 imsb[8];
	u16 mask[4];
	u16 nmask;
	ltq_bool_t valid;
} pce_dasa_msb_t;

/* IP DA/SA LSB Table */
typedef struct {
	u8 ilsb[8];
	u16	mask[4];
	u16 nmask;
	ltq_bool_t valid;
} pce_dasa_lsb_t;

/* programme the Protocol Table */
typedef struct {
	u16 ethertype;
	u16 emask;
} prtcol_tbl_t;

/* PPPoE Table  */
typedef struct {
	u16	sess_id;
} pce_ppoe_tbl_t;

typedef struct {
	u16 pkt_lng_idx:8;
	u16 dst_mac_addr_idx:8;
	u16 src_mac_addr_idx:8;
	u16 dst_appl_fld_idx:8;
	u16 src_appl_fld_idx:8;
	u16 dip_msb_idx:8;
	u16 dip_lsb_idx:8;
	u16 sip_msb_idx:8;
	u16 sip_lsb_idx:8;
	u16 inr_dip_msb_idx:8;
	u16 inr_dip_lsb_idx:8;
	u16 inr_sip_msb_idx:8;
	u16 inr_sip_lsb_idx:8;
	u16 ip_prot_idx:8;
	u16 ethertype_idx:8;
	u16 pppoe_idx:8;
	u16 vlan_idx:8;
	u16 svlan_idx:8;
	u16 payload1_idx:8;
	u16 payload2_idx:8;
	u16 ppp_prot_idx:8;
	u16 parse_lsb_idx:8;
	u16 parse_msb_idx:8;
} pce_table_t;

typedef struct {
	/* table reference counter */
	u16 pkg_lng_tbl_cnt[PCE_PKG_LNG_TBL_SIZE];
	u16 sa_mac_tbl_cnt[PCE_DASA_MAC_TBL_SIZE];
	u16 da_mac_tbl_cnt[PCE_DASA_MAC_TBL_SIZE];
	u16 appl_tbl_cnt[PCE_APPL_TBL_SIZE];
	u16 flags_tbl_cnt[PCE_FLAGS_TBL_SIZE];
	u16 payload_tbl_cnt[PCE_PAYLOAD_TBL_SIZE];
	u16 ipmsbtcnt[IP_DASA_PC_MSIZE];
	u16 iplsbtcnt[IP_DASA_PC_LSIZE];
	u16 ptcl_tbl_cnt[PCE_PTCL_TBL_SIZE];
	u16 pppoe_tbl_cnt[PCE_PPPOE_TBL_SIZE];
	u16 vlan_act_tbl_cnt[PCE_VLAN_ACT_TBL_SIZE];
	/* cached tables */
	pce_pkt_length_t	pkg_lng_tbl[PCE_PKG_LNG_TBL_SIZE];
	pce_sa_prog_t sa_mac_tbl[PCE_DASA_MAC_TBL_SIZE];
	pce_da_prog_t da_mac_tbl[PCE_DASA_MAC_TBL_SIZE];
	app_tbl_t appl_tbl[PCE_APPL_TBL_SIZE];
	flag_tbl_t flags_tbl[PCE_FLAGS_TBL_SIZE];
	payload_tbl_t payload_tbl[PCE_PAYLOAD_TBL_SIZE];
	pce_dasa_msb_t ip_dasa_msb_tbl[IP_DASA_PC_MSIZE];
	pce_dasa_lsb_t ip_dasa_lsb_tbl[IP_DASA_PC_LSIZE];
	prtcol_tbl_t ptcl_tbl[PCE_PTCL_TBL_SIZE];
	pce_ppoe_tbl_t pppoe_tbl[PCE_PPPOE_TBL_SIZE];
} pcetbl_prog_t /*pce_table_handle_t*/;

typedef struct {
	/* Parameter for the sub-tables */
	pcetbl_prog_t pce_sub_tbl;
	pce_table_t pce_tbl[PCE_TABLE_SIZE];
	GSW_PCE_action_t	pce_act[PCE_TABLE_SIZE];
	u8 ptblused[PCE_TABLE_SIZE];
} ltq_pce_table_t;

typedef struct {
	GSW_capType_t captype;
	/* Description String */
	char desci[GSW_CAP_STRING_LEN];
} gsw_capdesc_t;

typedef struct {
	GSW_STP_PortState_t psstate /*ifx_stp_state*/;
	GSW_8021X_portState_t	ps8021x /*ifx_8021_state*/;
	u8	pen_reg;
	u8	pstate_reg;
	u8	lrnlim;
} pstpstate_t;

typedef struct {
	/* Port Enable */
	ltq_bool_t penable;
	/* Learning Limit Action */
/*	ltq_bool_t laction; */
	/* Automatic MAC address table learning locking */
/*	ltq_bool_t lplock; */
	/* Automatic MAC address table learning limitation */
	u16 llimit;
	/* Port State */
	u16 ptstate;
	/* Port State for STP */
	GSW_STP_PortState_t	pcstate;
	/* Port State for 8021.x */
	GSW_8021X_portState_t	p8021xs;
} port_config_t;

typedef struct {
	/* 8021x Port Forwarding State */
	GSW_portForward_t sfport;
/* STP port State */
	GSW_portForward_t	spstate;
	/* 8021X Forwarding Port ID*/
	u8	fpid8021x;
	/* STP Port ID */
	u16	stppid;
} stp8021x_t;

typedef struct {
	ltq_bool_t	valid;
	u16 vid;
	u32 fid;
	u16 pm;
	u16 tm;
	ltq_bool_t	reserved;
} avlan_tbl_t;

typedef struct {
	u16 smsbindex;
	u16 dmsbindex;
	u16 slsbindex;
	u16 dlsbindex;
	u16 pmap;
	u16 subifid;
	u8 fid;
	u16 mcmode;
	ltq_bool_t valid;
	ltq_bool_t exclude;
} mcsw_table_t;


typedef struct {
	u16	igmode;
	ltq_bool_t igv3;
	u16	igfport;
	u8	igfpid;
	ltq_bool_t igxvlan;
	u8	igcos;
	mcsw_table_t mctable[MC_PC_SIZE];
	u16	igrport;
	u8	itblsize;
} gsw_igmp_t;

typedef struct {
	gsw_devtype_t	sdev;
	port_config_t pconfig[MAX_PORT_NUMBER];
	avlan_tbl_t avtable[VLAN_ACTIVE_TABLE_SIZE];
	stp8021x_t stpconfig;
	gsw_igmp_t iflag;
	ltq_pce_table_t phandler;
	void *raldev;
	u8 pnum;
	u8 tpnum; /* Total number of ports including vitual ports*/
	u8 mpnum; /* ManagementPortNumber */
	u32 matimer;
	ltq_bool_t rst;
	ltq_bool_t hwinit;
	u16 vlan_rd_index; /* read VLAN table index */
	u16 mac_rd_index; /* read mac table index */
	u8 mhw_rinx;
	u8 msw_rinx;
	u8 cport;
	u8 gsw_dev;
	/* multicast router port count */
	u8 mrtpcnt;
	u8 meter_cnt;
	u8 num_of_queues; /* Number of priority queues . */
	u8 num_of_meters;  /* Number of traffic meters */
	u8 num_of_shapers; /* Number of traffic shapers */
	u8 num_of_pppoe; /* PPPoE table size  */
	u8 avlantsz; /* Active VLAN table size */
	u8 ip_pkt_lnt_size; /* IP packet length table size */
	u8 prot_table_size; /* Protocol table size */
	u8 mac_dasa_table_size; /* MAC DA/SA table size */
	u8 app_table_size;	/* Application table size */
	u8 idsmtblsize;	/* IP DA/SA MSB table size */
	u8 idsltblsize;	/*  IP DA/SA LSB table size*/
	u8 mctblsize;	/* Multicast table size */
	u8 tftblsize; /* Flow Aggregation table size */
	u16 mactblsize; /* MAC bridging table size */
	u8 num_of_pmac;	/* Number of PMAC */
	u16 pdtblsize;	/* Payload Table Size  */
	u16 num_of_ifrmon;	/* Interface RMON Counter Table Size */
	u16 num_of_egvlan;	/* Egress VLAN Treatment Table Size */
	u16 num_of_rt_smac;	/* Routing MAC Table Size for Source MAC */
	u16 num_of_rt_dmac;	/* Routing MAC Table Size for Destination MAC */
	u16 num_of_rt_ppoe;	/* Routing PPPoE Table Size  */
	u16 num_of_rt_nat;	/* Routing Session Table Size */
	u16 num_of_rt_mtu;	/* Routing MTU Table Size */
	u16 num_of_rt_tunnel;	/* Routing Tunnel Table Size  */
	u16 num_of_rt_rtp;	/* Routing RTP Table Size */
	u16 gipver;
	void __iomem *gswl_base;	/*Base address GSWIP-L */
	void __iomem *gswr_base;  /* Base address GSWIP-R */
	void __iomem *gsw_base;  /* Base address GSWITCH */
} ethsw_api_dev_t;

u8 find_active_vlan_index(void *cdev, u16 vid);
int find_msb_tbl_entry(pcetbl_prog_t *ptbl,
	pce_dasa_msb_t *parm);
int pce_dasa_msb_tbl_write(void *cdev, pcetbl_prog_t *ptbl,
	pce_dasa_msb_t *parm);
int find_dasa_tbl_entry(pcetbl_prog_t *ptbl,
	pce_dasa_lsb_t *parm);
int pce_dasa_lsb_tbl_write(void *cdev, pcetbl_prog_t *ptbl,
	pce_dasa_lsb_t *parm);
int pce_table_init(ltq_pce_table_t *pchndl);
int ip_dasa_msb_tbl_del(void *cdev, pcetbl_prog_t *ptbl, u32 index);
int ip_dasa_lsb_tbl_del(void *cdev, pcetbl_prog_t *ptbl, u32 index);
int ipdsmsb_tblidx_del(pcetbl_prog_t *ptbl, u32 index);
int gavlan_tbl_index(pcetbl_prog_t *ptbl, u8 index);
int pce_pattern_delete(void *cdev, ltq_pce_table_t *pthandle, u32 index);
int ipdslsb_tblidx_del(pcetbl_prog_t *ptbl, u32 index);
int pce_action_delete(void *cdev, ltq_pce_table_t *pthandle, u32 index);
int pce_rule_read(void *cdev, ltq_pce_table_t *pthandle,
	GSW_PCE_rule_t *parm);
int pce_rule_write(void *cdev, ltq_pce_table_t *pthandle,
	GSW_PCE_rule_t *parm);
int gsw_pce_table_write(void *cdev, pctbl_prog_t *ptdata);
int gsw_pce_table_read(void *cdev, pctbl_prog_t *ptdata);
int gsw_pce_table_key_write(void *cdev, pctbl_prog_t *ptdata);
int gsw_pce_table_key_read(void *cdev, pctbl_prog_t *ptdata);

#endif    /* _LTQ_FLOW_CORE_H_ */
