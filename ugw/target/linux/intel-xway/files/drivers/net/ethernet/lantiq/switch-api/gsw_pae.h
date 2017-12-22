/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _GSW_ROUTER_H_
#define _GSW_ROUTER_H_

#define RT_SESSION_TABLE_SIZE 4096
#define	RT_IP_TBL_SIZE	2048
#define	RT_MAC_TBL_SIZE	512
#define	RT_PPPOE_TBL_SIZE	16
#define	RT_TUNNEL_TBL_SIZE	16
#define	RT_RTP_TBL_SIZE	64
#define	RT_MTU_TBL_SIZE	8

/** Provides the address of the configured/fetched lookup table. */
typedef enum {
	/** Parser microcode table */
	PCE_R_SESSION_INDEX	= 0x00,
	PCE_R_IP_INDEX			= 0x01,
	PCE_R_MAC_INDEX			= 0x02,
	PCE_R_PPPOE_INDEX		= 0x03,
	PCE_R_TUNNEL_INDEX	= 0x04,
	PCE_R_RTP_INDEX			= 0x05,
	PCE_R_MTU_INDEX			= 0x06,
} routing_table_address_t;

/** Description */
typedef enum {
	OPMOD_ADDRESS_READ	=	0,
	OPMOD_ADDRESS_WRITE	=	1,
	OPMOD_KEY_READ	=	2,
	OPMOD_KEY_WRITE	=	3,
} rt_opmod_access_t;

/** Description */
typedef enum {
	OPMOD_RT_SESSION_READ = 0,
	OPMOD_RT_SESSION_WRITE = 1,
	OPMOD_RT_SESSION_NEXT = 2,
	OPMOD_RT_SESSION_HIT_STATUS = 3,
} rt_opmod_session_t;

typedef enum {
	OPMOD_IPv4_READ = 0,
	OPMOD_IPv4_WRITE = 1,
	/*OPMOD_IPv6_READ = 2,*/
	/*OPMOD_IPv6_WRITE = 3,*/
} ip_opmod_access_t;

typedef struct {
	union {
		u8 i4addr[4];
		u16 i6addr[8];
	} iaddr;
	u8 itype:2;
	u8 valid:1;
} riptbl_t;

typedef struct  {
	u8 mdata[6];
	ltq_bool_t valid;
} rt_mac_tbl_t;

/* RT PPPoE Table  */
typedef struct {
	u16	psesid;
	ltq_bool_t valid;
} rt_ppoe_tbl_t;

/* RT PPPoE Table  */
typedef struct {
	u16	mtsize;
	ltq_bool_t valid;
} rt_mtu_tbl_t;

typedef struct {
	u16 rtpseqnum;
	u16 rtpsespcnt;
} rt_rtp_tbl_t;

typedef struct {
	ltq_bool_t hwvalid;
	int hwnextptr;
	/* GSW_ROUTE_session_t HWSession;*/
} rt_hw_tbl_t;

typedef struct {
	ltq_bool_t vflag;
	ltq_bool_t fflag;
	ltq_bool_t prio;
	u8 nventries;
	int pprt;
	int nptr;
	int hval;
	/* GSW_ROUTE_Session_pattern_t    pattern; */
} rt_ses_nod_tbl_t;

typedef struct {
	int nfentries;
	int ffptr;
	int lfptr;
	int nuentries;
	rt_ses_nod_tbl_t node[RT_SESSION_TABLE_SIZE];
	rt_hw_tbl_t hw_table[RT_SESSION_TABLE_SIZE];
} rt_session_tbl_t;

typedef struct {
	/* Routing IP Table */
	u16 rt_ip_tbl_cnt[RT_IP_TBL_SIZE];
	riptbl_t rt_ip_tbl[RT_IP_TBL_SIZE];
	/* table reference counter */
	u16 rt_mac_tbl_cnt[RT_MAC_TBL_SIZE];
	/* Routing MAC Table */
	rt_mac_tbl_t rt_mac_tbl[RT_MAC_TBL_SIZE];
	/* table reference counter */
	u16 rt_ppoe_tbl_cnt[RT_PPPOE_TBL_SIZE];
	/* Routing MAC Table */
	rt_ppoe_tbl_t rt_ppoe_tbl[RT_PPPOE_TBL_SIZE];
	/* table reference counter */
	u16 rt_mtu_tbl_cnt[RT_MTU_TBL_SIZE];
	/* Routing MAC Table */
	rt_mtu_tbl_t rt_mtu_tbl[RT_MTU_TBL_SIZE];
	/* table reference counter */
/*	u16 rt_rtp_tbl_cnt[RT_RTP_TBL_SIZE]; */
	/* Routing MAC Table */
/*	rt_rtp_tbl_t rt_rtp_tbl[RT_RTP_TBL_SIZE]; */
} rt_table_handle_t;


typedef struct {
	/* Parameter for the sub-tables */
	rt_table_handle_t rt_sub_tbl;
	rt_session_tbl_t rstbl;
} ltq_rt_table_t;

int rt_table_init(void);
int gsw_r_init(void);

#endif /* _GSW_ROUTER_H_ */
