/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
//#include <linux/sched.h>
#include <linux/jiffies.h>
#include <linux/timer.h>

#ifndef CONFIG_X86_INTEL_CE2700
#include <lantiq_soc.h>
#endif

#include "gsw_init.h"

//#define CONFIG_USE_EMULATOR 1
/* Global Variables */
ethsw_api_dev_t *ecoredev[LTQ_FLOW_DEV_MAX];
u8 VERSION_NAME[] = GSW_API_MODULE_NAME " for GSWITCH Platform";
u8 VERSION_NUMBER[] = GSW_API_DRV_VERSION;
u8 MICRO_CODE_VERSION_NAME[] = " GSWIP macro revision ID";
u8 MICRO_CODE_VERSION_NUMBER[] = "0x"MICRO_CODE_VERSION;
/* Port redirect flag */
static u8 prdflag;
static void get_gsw_hw_cap(void *cdev);

/* Local Macros & Definitions */
static pstpstate_t pstpstate[] = {
	{ GSW_STP_PORT_STATE_FORWARD, GSW_8021X_PORT_STATE_AUTHORIZED,
		1, PORT_STATE_FORWARDING, 1 },
	{ GSW_STP_PORT_STATE_FORWARD, GSW_8021X_PORT_STATE_UNAUTHORIZED,
		1, PORT_STATE_LISTENING, 1 },
	{ GSW_STP_PORT_STATE_FORWARD, GSW_8021X_PORT_STATE_RX_AUTHORIZED,
		1, PS_RENABLE_TDISABLE, 1 },
	{ GSW_STP_PORT_STATE_FORWARD, GSW_8021X_PORT_STATE_TX_AUTHORIZED,
		1, PS_RDISABLE_TENABLE, 1 },
	{ GSW_STP_PORT_STATE_DISABLE, GSW_8021X_PORT_STATE_AUTHORIZED,
		0, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_DISABLE, GSW_8021X_PORT_STATE_UNAUTHORIZED,
		0, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_DISABLE, GSW_8021X_PORT_STATE_RX_AUTHORIZED,
		0, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_DISABLE, GSW_8021X_PORT_STATE_TX_AUTHORIZED,
		0, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_LEARNING, GSW_8021X_PORT_STATE_AUTHORIZED,
		1, PORT_STATE_LEARNING, 1 },
	{ GSW_STP_PORT_STATE_LEARNING, GSW_8021X_PORT_STATE_UNAUTHORIZED,
		1, PORT_STATE_LEARNING, 1 },
	{ GSW_STP_PORT_STATE_LEARNING, GSW_8021X_PORT_STATE_RX_AUTHORIZED,
		1, PORT_STATE_LEARNING, 1 },
	{ GSW_STP_PORT_STATE_LEARNING, GSW_8021X_PORT_STATE_TX_AUTHORIZED,
		1, PORT_STATE_LEARNING, 1 },
	{ GSW_STP_PORT_STATE_BLOCKING, GSW_8021X_PORT_STATE_AUTHORIZED,
		1, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_BLOCKING, GSW_8021X_PORT_STATE_UNAUTHORIZED,
		1, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_BLOCKING, GSW_8021X_PORT_STATE_RX_AUTHORIZED,
		1, PORT_STATE_LISTENING, 0 },
	{ GSW_STP_PORT_STATE_BLOCKING, GSW_8021X_PORT_STATE_TX_AUTHORIZED,
		1, PORT_STATE_LISTENING, 0 }
};

static gsw_capdesc_t capdes[] = {
	{ GSW_CAP_TYPE_PORT, "Number of Ethernet ports"},
	{ GSW_CAP_TYPE_VIRTUAL_PORT, "Number of virtual ports"},
	{ GSW_CAP_TYPE_BUFFER_SIZE, "Pcket buffer size[in Bytes]:"},
	{ GSW_CAP_TYPE_SEGMENT_SIZE, "Buffer Segment size:"},
	{ GSW_CAP_TYPE_PRIORITY_QUEUE, "Number of queues:"},
	{ GSW_CAP_TYPE_METER, "Number of traffic meters:"},
	{ GSW_CAP_TYPE_RATE_SHAPER, "Number of rate shapers:"},
	{ GSW_CAP_TYPE_VLAN_GROUP, "Number of VLAN groups:"},
	{ GSW_CAP_TYPE_FID, "Number of FIDs:"},
	{ GSW_CAP_TYPE_MAC_TABLE_SIZE, "Number of MAC entries:"},
	{ GSW_CAP_TYPE_MULTICAST_TABLE_SIZE, "Number of multicast entries"},
	{ GSW_CAP_TYPE_PPPOE_SESSION, "Number of PPPoE sessions:"},
	{ GSW_CAP_TYPE_SVLAN_GROUP, "Number of STAG VLAN groups:"},
	{ GSW_CAP_TYPE_PMAC, "Number of PMACs:"},
	{ GSW_CAP_TYPE_PAYLOAD, "Number of entries in Payload Table size:"},
	{ GSW_CAP_TYPE_IF_RMON, "Number of IF RMON Counters:"},
	{ GSW_CAP_TYPE_EGRESS_VLAN, "Number of Egress VLAN Entries:"},
	{ GSW_CAP_TYPE_RT_SMAC, "Number of Routing Source-MAC Entries:"},
	{ GSW_CAP_TYPE_RT_DMAC, "Number of Routing Dest-MAC Entries:"},
	{ GSW_CAP_TYPE_RT_PPPoE, "Number of Routing-PPPoE Entries:"},
	{ GSW_CAP_TYPE_RT_NAT, "Number of Routing-NAT Entries:"},
	{ GSW_CAP_TYPE_RT_MTU, "Number of MTU Entries:"},
	{ GSW_CAP_TYPE_RT_TUNNEL, "Number of Tunnel Entries:"},
	{ GSW_CAP_TYPE_RT_RTP, "Number of RTP Entries:"},
	{ GSW_CAP_TYPE_LAST, "Last Capability Index"}
};

/*****************/
/* Function Body */
/*****************/
#ifndef CONFIG_X86_INTEL_CE2700
static void __iomem		*gswr_bm_addr = (void __iomem *) (KSEG1 | 0x1a000114);;
#endif
#define GAP_MAX	50
#define GAP_MAX1	1000
#define MAX_READ 20
#define REPEAT_L 10
#define REPEAT_M 10

static int calc_credit_value(u32 *rxcnt, u8 *mv, u8 *locv)
{
	u8 s,c,i, m, loc = 0, crd[MAX_READ] = {0};
	for ( s = 0; s < MAX_READ - 1; s++) {
		for ( c = s + 1; c < MAX_READ; c++) {
			if ((rxcnt[s] <= rxcnt[c]) && (rxcnt[c] <= (rxcnt[s] + (c-s) * GAP_MAX))) {
				crd[s]++; crd[c]++;
			}
		}
	}
	m = crd[0];
	for ( i = 1; i < MAX_READ; i++) {
		if (crd[i] > m) {
			m = crd[i];
			loc = i;
		}
	}
	*mv = m;
	*locv = loc;
	return 0;
}

static int calc_credit_byte_value(u32 *rxcnt, u8 *mv, u8 *locv)
{
	u8 s,c,i, m, loc = 0, crd[MAX_READ] = {0};
	for ( s = 0; s < MAX_READ - 1; s++) {
		for ( c = s + 1; c < MAX_READ; c++) {
			if ((rxcnt[s] <= rxcnt[c]) && (rxcnt[c] <= (rxcnt[s] + (c-s) * GAP_MAX1))) {
				crd[s]++; crd[c]++;
			}
		}
	}
	m = crd[0];
	for ( i = 1; i < MAX_READ; i++) {
		if (crd[i] > m) {
			m = crd[i];
			loc = i;
		}
	}
	*mv = m;
	*locv = loc;
	return 0;
}

#ifdef CONFIG_X86_INTEL_CE2700
static void cport_sgmii_config(void *cdev)
{
	gsw_w32(cdev, 0xfa01, 0, 16, 0x0030);
	gsw_w32(cdev, 0xfa01, 0, 14, 0x0010);
	gsw_w32(cdev, 0xD009, 0, 16, 0x0009);
	gsw_w32(cdev, 0xD004, 0, 16, 0x053A);
	/*Reset of TBI FSM's*/
	gsw_w32(cdev, 0xD305, 0, 16, 0x0033);
	/*Release  TBI FSM's*/
	gsw_w32(cdev, 0xD305, 0, 16, 0x0032);
	/*  TX Buffer and pointers are initialized */
	gsw_w32(cdev, 0xD404, 0, 16, 0x0003);
	/* TX Buffer and pointers are normally operating */
	gsw_w32(cdev, 0xD404, 0, 16, 0x0001);
	/*  RX Buffer and pointers are initialized */
	gsw_w32(cdev, 0xD401, 0, 16, 0x0003);
	/*  RX Buffer and pointers are normally operating */
	gsw_w32(cdev, 0xD401, 0, 16, 0x0001);
	gsw_w32(cdev, 0xD300, 0, 16, 0x0018);
	gsw_w32(cdev, 0xD301, 0, 16, 0x0001);
	/*  Restart Autonegotiation */
	gsw_w32(cdev, 0xD304, 0, 16, 0x803c);
	/*SGMII_PHY SGMII PHY mode */
	gsw_w32(cdev, 0xD304, 0, 16, 0x80b4);
}
#endif

static void ltq_ethsw_port_cfg_init(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 i, value;
	gsw_r32(cdev, ETHSW_CAP_1_PPORTS_OFFSET,
		ETHSW_CAP_1_PPORTS_SHIFT,
		ETHSW_CAP_1_PPORTS_SIZE, &value);
	gswdev->pnum = value;
	for (i = 0; i < gswdev->pnum; i++) {
		memset(&gswdev->pconfig[i], 0, sizeof(port_config_t));
		gswdev->pconfig[i].llimit = 0xFF;
		gswdev->pconfig[i].penable = 1;
	}
	gswdev->stpconfig.sfport = GSW_PORT_FORWARD_DEFAULT;
	gswdev->stpconfig.fpid8021x = gswdev->cport;
	gsw_r32(cdev, ETHSW_CAP_1_VPORTS_OFFSET, ETHSW_CAP_1_VPORTS_SHIFT,
		ETHSW_CAP_1_VPORTS_SIZE, &value);
	gswdev->tpnum = value + gswdev->pnum;
}
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
static void reset_multicast_sw_table(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 i;
	memset(&gswdev->iflag, 0, sizeof(gsw_igmp_t));
	gswdev->iflag.itblsize = gswdev->mctblsize;
	for (i = 0; i < gswdev->iflag.itblsize; i++) {
		gswdev->iflag.mctable[i].slsbindex = 0x7F;
		gswdev->iflag.mctable[i].dlsbindex = 0x7F;
		gswdev->iflag.mctable[i].smsbindex = 0x1F;
		gswdev->iflag.mctable[i].dmsbindex = 0x1F;
	}
}

/* Multicast Software Table Include/Exclude Add function */
static int gsw2x_msw_table_wr(void *cdev, GSW_multicastTable_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	int	i, j, exclude_rule = 0;
	int	dlix = 0x7F, dmix = 0x7F;
	int slix = 0x7F, smix = 0x7F;
	int dlflag = 0, dmflag = 0, slflag = 0, smflag = 0;
	ltq_bool_t new_entry = 0;
	ltq_pce_table_t *hpctbl = &gswdev->phandler;
	gsw_igmp_t *hitbl = &gswdev->iflag;
	pctbl_prog_t ptdata;
	pce_dasa_lsb_t	ltbl;
	pce_dasa_msb_t	mtbl;
	memset(&ptdata, 0, sizeof(pctbl_prog_t));
	memset(&ltbl, 0, sizeof(pce_dasa_lsb_t));
	memset(&mtbl, 0, sizeof(pce_dasa_msb_t));
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
		|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
		if (gswdev->iflag.igv3 != 1) {
			pr_err("%s:%s:%d(bIGMPv3 need to be enable)\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
	}
	if ((parm->eIPVersion != GSW_IP_SELECT_IPV4)
		&& (parm->eIPVersion != GSW_IP_SELECT_IPV6)) {
		pr_err("%s:%s:%d (IPv4/IPV6 need to enable)\n",
		__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
		for (i = 0; i < 4; i++)
			ltbl.ilsb[i] =
			((parm->uIP_Gda.nIPv4 >> (i * 8)) & 0xFF);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			ltbl.mask[0] = 0; ltbl.mask[1] = 0;
			ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
		} else {
			ltbl.mask[0] = 0xFF00;
		}
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			mtbl.imsb[j-1] = (parm->uIP_Gda.nIPv6[i] & 0xFF);
			mtbl.imsb[j] = ((parm->uIP_Gda.nIPv6[i] >> 8) & 0xFF);
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			mtbl.mask[0] = 0; mtbl.mask[1] = 0;
			mtbl.mask[2] = 0; mtbl.mask[3] = 0;
		} else {
			mtbl.mask[0] = 0;
		}
		dmix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl, &mtbl);
		if (dmix == 0xFF) {
			dmix = pce_dasa_msb_tbl_write(cdev,
				&hpctbl->pce_sub_tbl, &mtbl);
			dmflag = 1;
		}
		if (dmix < 0) {
			pr_err("%s:%s:%d (IGMP Table full)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			ltbl.ilsb[j-1] = (parm->uIP_Gda.nIPv6[i+4] & 0xFF);
			ltbl.ilsb[j] = ((parm->uIP_Gda.nIPv6[i+4] >> 8) & 0xFF);
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			ltbl.mask[0] = 0; ltbl.mask[1] = 0;
			ltbl.mask[2] = 0; ltbl.mask[3] = 0;
		} else {
			ltbl.mask[0] = 0;/* DIP LSB Nibble Mask */
		}
	}
	dlix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl, &ltbl);
	if (dlix == 0xFF) {
		dlix = pce_dasa_lsb_tbl_write(cdev,
			&hpctbl->pce_sub_tbl, &ltbl);
		dlflag = 1;
	}
	if (dlix < 0) {
		pr_err("%s:%s:%d (IGMP Table full)\n",
			__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
		|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
		if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
			for (i = 0; i < 4; i++)
				ltbl.ilsb[i] =
				((parm->uIP_Gsa.nIPv4 >> (i * 8)) & 0xFF);
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				ltbl.mask[0] = 0x0; ltbl.mask[1] = 0x0;
				ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
			} else {
				/* DIP LSB Nibble Mask */
				ltbl.mask[0] = 0xFF00;
			}
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((ltbl.ilsb[3] == 0) &&
					(ltbl.ilsb[2] == 0) &&
					(ltbl.ilsb[1] == 0) &&
					(ltbl.ilsb[0] == 0)) {
						pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
						return GSW_statusErr;
				}
			}
		}
		if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
			int src_zero = 0;
		/*First, search for DIP in the DA/SA table (DIP MSB)*/
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				mtbl.imsb[j-1] =
				(parm->uIP_Gsa.nIPv6[i] & 0xFF);
				mtbl.imsb[j] =
				((parm->uIP_Gsa.nIPv6[i] >> 8) & 0xFF);
			}
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				mtbl.mask[0] = 0; mtbl.mask[1] = 0;
				mtbl.mask[2] = 0; mtbl.mask[3] = 0;
			} else {
				mtbl.mask[0] = 0;/* DIP MSB Nibble Mask */
			}
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((mtbl.imsb[0] == 0) &&
					(mtbl.imsb[1] == 0) &&
					(mtbl.imsb[2] == 0) &&
					(mtbl.imsb[3] == 0) &&
					(mtbl.imsb[4] == 0) &&
					(mtbl.imsb[5] == 0) &&
					(mtbl.imsb[6] == 0) &&
					(mtbl.imsb[7] == 0)) {
						src_zero = 1;
				}
			}
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				ltbl.ilsb[j-1] =
				(parm->uIP_Gsa.nIPv6[i+4] & 0xFF);
				ltbl.ilsb[j] =
				((parm->uIP_Gsa.nIPv6[i+4] >> 8) & 0xFF);
			}
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				ltbl.mask[0] = 0; ltbl.mask[1] = 0;
				ltbl.mask[2] = 0; ltbl.mask[3] = 0;
			} else {
				ltbl.mask[0] = 0;/* DIP LSB Nibble Mask */
			}
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((ltbl.ilsb[0] == 0) &&
					(ltbl.ilsb[1] == 0) &&
					(ltbl.ilsb[2] == 0) &&
					(ltbl.ilsb[3] == 0) &&
					(ltbl.ilsb[4] == 0) &&
					(ltbl.ilsb[5] == 0) &&
					(ltbl.ilsb[6] == 0) &&
					(ltbl.ilsb[7] == 0)) {
					if (src_zero) {
						pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
						return GSW_statusErr;
					}
				}
			}
			smix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl, &mtbl);
			if (smix == 0xFF) {
				smix = pce_dasa_msb_tbl_write(cdev,
					&hpctbl->pce_sub_tbl, &mtbl);
				smflag = 1;
			}
			if (smix < 0) {
				pr_err("%s:%s:%d (IGMP Table full)\n",
					__FILE__, __func__, __LINE__);
				return GSW_statusErr;
			}
		}
		slix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl, &ltbl);
		if (slix == 0xFF) {
			slix = pce_dasa_lsb_tbl_write(cdev,
				&hpctbl->pce_sub_tbl, &ltbl);
			slflag = 1;
		}
		if (slix < 0) {
			pr_err("%s:%s:%d (IGMP Table full)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
	}
	/* update the entry for another port number if already exists*/
	for (i = 0; i < gswdev->iflag.itblsize; i++) {
		/* Check if port was already exist */
		if ((hitbl->mctable[i].dlsbindex == dlix) &&
			(hitbl->mctable[i].dmsbindex == dmix) &&
			(hitbl->mctable[i].slsbindex == slix) &&
			(hitbl->mctable[i].smsbindex == smix) &&
			(hitbl->mctable[i].valid == 1)) {
			if (((hitbl->mctable[i].pmap >> parm->nPortId)
				& 0x1) == 1)
				return GSW_statusOk;
			switch (hitbl->mctable[i].mcmode) {
			case GSW_IGMP_MEMBER_DONT_CARE:
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
				if (parm->eIPVersion == GSW_IP_SELECT_IPV6)
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
				/* Add the port */
				hitbl->mctable[i].pmap |= (1 << parm->nPortId);
				break;
			case GSW_IGMP_MEMBER_EXCLUDE:
				if (gswdev->gipver == LTQ_GSWIP_3_0)
					exclude_rule = 0;
				else
					exclude_rule = 1;

			case GSW_IGMP_MEMBER_INCLUDE:
				/* Add the port */
				hitbl->mctable[i].pmap |= (1 << parm->nPortId);
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
				hpctbl->pce_sub_tbl.iplsbtcnt[slix]++;
				if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
					hpctbl->pce_sub_tbl.ipmsbtcnt[smix]++;
				}
				break;
			} /* end switch */
			/* Now, we write into Multicast SW Table */
			memset(&ptdata, 0, sizeof(pctbl_prog_t));
			ptdata.table = PCE_MULTICAST_SW_INDEX;
			ptdata.pcindex = i;
			ptdata.key[1] = (hitbl->mctable[i].smsbindex << 8)
				| hitbl->mctable[i].slsbindex;
			ptdata.key[0] = (hitbl->mctable[i].dmsbindex << 8)
				| hitbl->mctable[i].dlsbindex;
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				ptdata.val[0] = hitbl->mctable[i].pmap;
				ptdata.val[1] =
				(parm->nSubIfId & 0xFFFF)	<< 3;
				ptdata.key[2] = parm->nFID & 0x3F;
				if (parm->eModeMember ==
					GSW_IGMP_MEMBER_EXCLUDE) {
					ptdata.key[2] |= (1 << 15);
			/*ptdata.key[2] |= (parm->bExclSrcIP & 1) << 15; */
				} else {
					ptdata.key[2] &= ~(1 << 15);
				}
			} else {
				if (parm->eModeMember ==
					GSW_IGMP_MEMBER_EXCLUDE)
					ptdata.val[0] = (0 << parm->nPortId);
				else
					ptdata.val[0] = hitbl->mctable[i].pmap;
			}
			ptdata.valid = hitbl->mctable[i].valid;
			gsw_pce_table_write(cdev, &ptdata);
			new_entry = 1;
			if (exclude_rule == 0)
				return GSW_statusOk;
		}
	}

	/* wildcard entry for EXCLUDE rule for  port number if already exists*/
/*if (gswdev->gipver != LTQ_GSWIP_3_0) {*/
	if ((exclude_rule == 1) && (new_entry == 1)) {
		for (i = 0; i < gswdev->iflag.itblsize; i++) {
			/* Check if port was already exist */
			if ((hitbl->mctable[i].dlsbindex == dlix) &&
			(hitbl->mctable[i].dmsbindex == dmix) &&
			(hitbl->mctable[i].slsbindex == 0x7F) &&
			(hitbl->mctable[i].smsbindex == 0x7F) &&
			(hitbl->mctable[i].valid == 1)) {
				if (((hitbl->mctable[i].pmap >>
					parm->nPortId) & 0x1) == 1) {
					return GSW_statusOk;
				} else {
					hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
					if (parm->eIPVersion ==
					GSW_IP_SELECT_IPV6)
						hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
					/* Add the port */
					hitbl->mctable[i].pmap |=
					(1 << parm->nPortId);
				}
				hitbl->mctable[i].mcmode =
				GSW_IGMP_MEMBER_DONT_CARE;
				memset(&ptdata, 0, sizeof(pctbl_prog_t));
				ptdata.table = PCE_MULTICAST_SW_INDEX;
				ptdata.pcindex = i;
				ptdata.key[1] =
				((hitbl->mctable[i].smsbindex << 8)
					| (hitbl->mctable[i].slsbindex));
				ptdata.key[0] =
				((hitbl->mctable[i].dmsbindex << 8)
					| (hitbl->mctable[i].dlsbindex));
				ptdata.val[0] = hitbl->mctable[i].pmap;
				ptdata.valid = hitbl->mctable[i].valid;
				if (gswdev->gipver == LTQ_GSWIP_3_0) {
					ptdata.val[1] =
					(parm->nSubIfId & 0xFFFF)	<< 3;
					ptdata.key[2] = parm->nFID & 0x3F;
					ptdata.key[2] |=
					(parm->bExclSrcIP & 1) << 15;
				}
				gsw_pce_table_write(cdev, &ptdata);
				return GSW_statusOk;
			}
		}
	}
/*	}*/
/* Create the new DstIP & SrcIP entry */
	if (new_entry == 0) {
		if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
			|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
			i = 0;
			while (i < gswdev->iflag.itblsize) {
				/* Find a new empty entry to add */
				if (hitbl->mctable[i].valid == 0)
					break;
				i++;
			}
		} else if (parm->eModeMember == GSW_IGMP_MEMBER_DONT_CARE) {
			i = 63;
			while (i > 0) {
				/* Find a new empty entry to add */
				if (hitbl->mctable[i].valid == 0)
					break;
				i--;
			}
		}
		if (i >= 0 && i < gswdev->iflag.itblsize) {
			hitbl->mctable[i].dlsbindex = dlix;
			hitbl->mctable[i].dmsbindex = dmix;
			hitbl->mctable[i].pmap |= (1 << parm->nPortId);
			if (dlflag)
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix] = 1;
			else
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
			if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
				if (dmflag)
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] = 1;
				else
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
			}
			hitbl->mctable[i].valid = 1;
			hitbl->mctable[i].mcmode = parm->eModeMember;
			if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
			|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
				hitbl->mctable[i].slsbindex = slix;
				hitbl->mctable[i].smsbindex = smix;
				if (slflag)
					hpctbl->pce_sub_tbl.iplsbtcnt[slix] = 1;
				else
					hpctbl->pce_sub_tbl.iplsbtcnt[slix]++;
				if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
					if (smflag)
						hpctbl->pce_sub_tbl.ipmsbtcnt[smix] = 1;
					else
						hpctbl->pce_sub_tbl.ipmsbtcnt[smix]++;
				}
			} else if (parm->eModeMember ==
					GSW_IGMP_MEMBER_DONT_CARE) {
				hitbl->mctable[i].slsbindex = 0x7F;
				hitbl->mctable[i].smsbindex = 0x7F;
			}
		}
		memset(&ptdata, 0, sizeof(pctbl_prog_t));
		/* Now, we write into Multicast SW Table */
		ptdata.table = PCE_MULTICAST_SW_INDEX;
		ptdata.pcindex = i;
		ptdata.key[1] = ((hitbl->mctable[i].smsbindex << 8)
			| hitbl->mctable[i].slsbindex);
		ptdata.key[0] = ((hitbl->mctable[i].dmsbindex << 8)
			| hitbl->mctable[i].dlsbindex);

		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			ptdata.val[0] = hitbl->mctable[i].pmap;
		} else {
			if ((parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE))
				ptdata.val[0] = (0 << parm->nPortId);
			else
				ptdata.val[0] = hitbl->mctable[i].pmap;
		}
		ptdata.valid = hitbl->mctable[i].valid;
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			ptdata.val[1] = (parm->nSubIfId & 0xFFFF)	<< 3;
			ptdata.key[2] = parm->nFID & 0x3F;
			ptdata.key[2] |= (parm->bExclSrcIP & 1) << 15;
		}
		gsw_pce_table_write(cdev, &ptdata);

		if ((parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
			for (i = 0; i < gswdev->iflag.itblsize; i++) {
				/* Check if port was already exist */
				if ((hitbl->mctable[i].dlsbindex == dlix) &&
				(hitbl->mctable[i].dmsbindex == dmix) &&
				(hitbl->mctable[i].slsbindex == 0x7F) &&
				(hitbl->mctable[i].smsbindex == 0x7F) &&
				(hitbl->mctable[i].valid == 1)) {
					if (((hitbl->mctable[i].pmap >>
						parm->nPortId) & 0x1) == 1)
						return GSW_statusOk;
					hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
					if (parm->eIPVersion ==
						GSW_IP_SELECT_IPV6)
						hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
					hitbl->mctable[i].mcmode =
					GSW_IGMP_MEMBER_DONT_CARE;
					/* Add the port */
					hitbl->mctable[i].pmap |=
					(1 << parm->nPortId);
					memset(&ptdata, 0, sizeof(pctbl_prog_t));
					ptdata.table = PCE_MULTICAST_SW_INDEX;
					ptdata.pcindex = i;
					ptdata.key[1] =
					((hitbl->mctable[i].smsbindex << 8)
					| (hitbl->mctable[i].slsbindex));
					ptdata.key[0] =
					((hitbl->mctable[i].dmsbindex << 8)
					| (hitbl->mctable[i].dlsbindex));
					ptdata.val[0] = hitbl->mctable[i].pmap;
					ptdata.valid = hitbl->mctable[i].valid;
					gsw_pce_table_write(cdev, &ptdata);
					return GSW_statusOk;
				}
			}
			i = 63;
			while (i > 0) {
				/* Find a new empty entry to add */
				if (hitbl->mctable[i].valid == 0)
					break;
				i--;
			}
		if (i >= 0 && i < gswdev->iflag.itblsize) {
				/* Now, we write into Multicast SW Table */
			hitbl->mctable[i].dlsbindex = dlix;
			hitbl->mctable[i].dmsbindex = dmix;
			hitbl->mctable[i].slsbindex = 0x7F;
			hitbl->mctable[i].smsbindex = 0x7F;
			hitbl->mctable[i].pmap |= (1 << parm->nPortId);
			hitbl->mctable[i].mcmode = GSW_IGMP_MEMBER_DONT_CARE;
			hitbl->mctable[i].valid = 1;
			hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
			if (parm->eIPVersion == GSW_IP_SELECT_IPV6)
				hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
			memset(&ptdata, 0, sizeof(pctbl_prog_t));
			ptdata.table = PCE_MULTICAST_SW_INDEX;
			ptdata.pcindex = i;
			ptdata.key[1] = ((hitbl->mctable[i].smsbindex << 8)
				| hitbl->mctable[i].slsbindex);
			ptdata.key[0] = ((hitbl->mctable[i].dmsbindex << 8)
				| hitbl->mctable[i].dlsbindex);
			ptdata.val[0] = hitbl->mctable[i].pmap;
			ptdata.valid = hitbl->mctable[i].valid;
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				ptdata.val[1] =
				(parm->nSubIfId & 0xFFFF)	<< 3;
				ptdata.key[2] = parm->nFID & 0x3F;
				ptdata.key[2] |= (parm->bExclSrcIP & 1) << 15;
			}
			gsw_pce_table_write(cdev, &ptdata);
			} else {
				pr_err("%s:%s:%d (IGMP Table full)\n",
					__FILE__, __func__, __LINE__);
			}
		}
	}
	/* Debug */
	return GSW_statusOk;
}

/* Multicast Software Table Include/Exclude Add function */
static int gsw3x_msw_table_wr(void *cdev, GSW_multicastTable_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	int	i, j, exclude_rule = 0;
	int	dlix = 0x7F, dmix = 0x7F;
	int slix = 0x7F, smix = 0x7F;
	int dlflag = 0, dmflag = 0, slflag = 0, smflag = 0;
	ltq_bool_t new_entry = 0;
	ltq_pce_table_t *hpctbl = &gswdev->phandler;
	gsw_igmp_t *hitbl = &gswdev->iflag;
	pctbl_prog_t ptdata;
	pce_dasa_lsb_t	ltbl;
	pce_dasa_msb_t	mtbl;
	memset(&ptdata, 0, sizeof(pctbl_prog_t));
	memset(&ltbl, 0, sizeof(pce_dasa_lsb_t));
	memset(&mtbl, 0, sizeof(pce_dasa_msb_t));
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
		|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
		if (gswdev->iflag.igv3 != 1) {
			pr_err("%s:%s:%d(bIGMPv3 need to be enable)\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
	}
	if ((parm->eIPVersion != GSW_IP_SELECT_IPV4)
		&& (parm->eIPVersion != GSW_IP_SELECT_IPV6)) {
		pr_err("%s:%s:%d (IPv4/IPV6 need to enable)\n",
		__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
		for (i = 0; i < 4; i++)
			ltbl.ilsb[i] = ((parm->uIP_Gda.nIPv4 >> (i * 8))
				& 0xFF);
			ltbl.mask[0] = 0; ltbl.mask[1] = 0;
			ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			mtbl.imsb[j-1] = (parm->uIP_Gda.nIPv6[i] & 0xFF);
			mtbl.imsb[j] = ((parm->uIP_Gda.nIPv6[i] >> 8) & 0xFF);
		}
		mtbl.mask[0] = 0; mtbl.mask[1] = 0;
		mtbl.mask[2] = 0; mtbl.mask[3] = 0;
		dmix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl,
			&mtbl);
		if (dmix == 0xFF) {
			dmix = pce_dasa_msb_tbl_write(cdev,
				&hpctbl->pce_sub_tbl, &mtbl);
			dmflag = 1;
		}
		if (dmix < 0) {
			pr_err("%s:%s:%d (IGMP Table full)\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			ltbl.ilsb[j-1] = (parm->uIP_Gda.nIPv6[i+4] & 0xFF);
			ltbl.ilsb[j] = ((parm->uIP_Gda.nIPv6[i+4] >> 8) & 0xFF);
		}
		ltbl.mask[0] = 0; ltbl.mask[1] = 0;
		ltbl.mask[2] = 0; ltbl.mask[3] = 0;
	}
	dlix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl,
		&ltbl);
	if (dlix == 0xFF) {
		dlix = pce_dasa_lsb_tbl_write(cdev,
			&hpctbl->pce_sub_tbl, &ltbl);
		dlflag = 1;
	}
	if (dlix < 0) {
		pr_err("%s:%s:%d (IGMP Table full)\n",
			__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE) ||
		(parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
		if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
			for (i = 0; i < 4; i++)
				ltbl.ilsb[i] = ((parm->uIP_Gsa.nIPv4 >> (i * 8))
				& 0xFF);
			ltbl.mask[0] = 0x0; ltbl.mask[1] = 0x0;
			ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((ltbl.ilsb[3] == 0) &&
				(ltbl.ilsb[2] == 0) &&
				(ltbl.ilsb[1] == 0) &&
				(ltbl.ilsb[0] == 0)) {
					pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
						return GSW_statusErr;
				}
			}
		}
		if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
			int src_zero = 0;
	/* First, search for DIP in the DA/SA table (DIP MSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				mtbl.imsb[j-1] = (parm->uIP_Gsa.nIPv6[i]
					& 0xFF);
				mtbl.imsb[j] = ((parm->uIP_Gsa.nIPv6[i] >> 8)
					& 0xFF);
			}
			mtbl.mask[0] = 0; mtbl.mask[1] = 0;
			mtbl.mask[2] = 0; mtbl.mask[3] = 0;
			if (parm->eModeMember ==
				GSW_IGMP_MEMBER_EXCLUDE) {
				if (mtbl.imsb[0] == 0 &&
				mtbl.imsb[1] == 0 &&
				mtbl.imsb[2] == 0 &&
				mtbl.imsb[3] == 0 &&
				mtbl.imsb[4] == 0 &&
				mtbl.imsb[5] == 0 &&
				mtbl.imsb[6] == 0 &&
				mtbl.imsb[7] == 0) {
					src_zero = 1;
				}
			}
	/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				ltbl.ilsb[j-1] = (parm->uIP_Gsa.nIPv6[i+4]
					& 0xFF);
				ltbl.ilsb[j] = ((parm->uIP_Gsa.nIPv6[i+4] >> 8)
					& 0xFF);
			}
			ltbl.mask[0] = 0; ltbl.mask[1] = 0;
			ltbl.mask[2] = 0; ltbl.mask[3] = 0;
			if (parm->eModeMember ==
				GSW_IGMP_MEMBER_EXCLUDE) {
				if ((ltbl.ilsb[0] == 0) &&
					(ltbl.ilsb[1] == 0) &&
					(ltbl.ilsb[2] == 0) &&
					(ltbl.ilsb[3] == 0) &&
					(ltbl.ilsb[4] == 0) &&
					(ltbl.ilsb[5] == 0) &&
					(ltbl.ilsb[6] == 0) &&
					(ltbl.ilsb[7] == 0)) {
					if (src_zero) {
						pr_err("%s:%s:%d (Exclude rule SIP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
						return GSW_statusErr;
					}
				}
			}
			smix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl,
				&mtbl);
			if (smix == 0xFF) {
				smix = pce_dasa_msb_tbl_write(cdev,
					&hpctbl->pce_sub_tbl, &mtbl);
				smflag = 1;
			}
			if (smix < 0) {
				pr_err("%s:%s:%d (IGMP Table full)\n",
					__FILE__, __func__, __LINE__);
				return GSW_statusErr;
			}
		}
		slix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl,
			&ltbl);
		if (slix == 0xFF) {
			slix = pce_dasa_lsb_tbl_write(cdev,
				&hpctbl->pce_sub_tbl, &ltbl);
			slflag = 1;
		}
		if (slix < 0) {
			pr_err("%s:%s:%d (IGMP Table full)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
	}
	/* update the entry for another port number if already exists*/
	for (i = 0; i < gswdev->iflag.itblsize; i++) {
		/* Check if port was already exist */
		if ((hitbl->mctable[i].dlsbindex == dlix) &&
			(hitbl->mctable[i].dmsbindex == dmix) &&
			(hitbl->mctable[i].slsbindex == slix) &&
			(hitbl->mctable[i].smsbindex == smix) &&
			(hitbl->mctable[i].valid == 1) &&
			(hitbl->mctable[i].fid == parm->nFID)) {
			if (((hitbl->mctable[i].pmap >> parm->nPortId)
				& 0x1) == 1)
				return GSW_statusOk;
			exclude_rule = 0;
			switch (hitbl->mctable[i].mcmode) {
			case GSW_IGMP_MEMBER_DONT_CARE:
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
				if (parm->eIPVersion == GSW_IP_SELECT_IPV6)
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
				/* Add the port */
				hitbl->mctable[i].pmap |= (1 << parm->nPortId);
				break;
			case GSW_IGMP_MEMBER_EXCLUDE:
				exclude_rule = 1;
/*		hitbl->mctable[i].exclude = 1; */
/*		ptdata.key[2] |= (parm->bExclSrcIP & 1) << 15; */
			case GSW_IGMP_MEMBER_INCLUDE:
				/* Add the port */
				hitbl->mctable[i].pmap |= (1 << parm->nPortId);
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
				hpctbl->pce_sub_tbl.iplsbtcnt[slix]++;
				if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
					hpctbl->pce_sub_tbl.ipmsbtcnt[smix]++;
				}
				break;
			} /* end switch */
			/* Now, we write into Multicast SW Table */
			memset(&ptdata, 0, sizeof(pctbl_prog_t));
			ptdata.table = PCE_MULTICAST_SW_INDEX;
			ptdata.pcindex = i;
			ptdata.key[1] = ((hitbl->mctable[i].smsbindex << 8)
				| (hitbl->mctable[i].slsbindex));
			ptdata.key[0] = ((hitbl->mctable[i].dmsbindex << 8)
				| (hitbl->mctable[i].dlsbindex));
			ptdata.key[2] = ((hitbl->mctable[i].fid) & 0x3F);
			ptdata.key[2] |= ((hitbl->mctable[i].exclude) << 15);
			ptdata.val[0] = hitbl->mctable[i].pmap;
			ptdata.val[1] = ((hitbl->mctable[i].subifid)	<< 3);
			ptdata.valid = hitbl->mctable[i].valid;
			gsw_pce_table_write(cdev, &ptdata);
			new_entry = 1;
		}
	}

/* Create the new DstIP & SrcIP entry */
	if (new_entry == 0) {
		if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE) ||
			(parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
			i = 0;
			while (i < gswdev->iflag.itblsize) {
				/* Find a new empty entry to add */
				if (hitbl->mctable[i].valid == 0)
					break;
				i++;
			}
		} else if (parm->eModeMember == GSW_IGMP_MEMBER_DONT_CARE) {
			i = 63;
			while (i > 0) {
				/* Find a new empty entry to add */
				if (hitbl->mctable[i].valid == 0)
					break;
				i--;
			}
		}
		if (i >= 0 && i < gswdev->iflag.itblsize) {
			hitbl->mctable[i].dlsbindex = dlix;
			hitbl->mctable[i].dmsbindex = dmix;
			hitbl->mctable[i].pmap |= (1 << parm->nPortId);
			if (dlflag)
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix] = 1;
			else
				hpctbl->pce_sub_tbl.iplsbtcnt[dlix]++;
			if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
				if (dmflag)
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] = 1;
				else
					hpctbl->pce_sub_tbl.ipmsbtcnt[dmix]++;
			}
			hitbl->mctable[i].valid = 1;
			hitbl->mctable[i].mcmode = parm->eModeMember;
			if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
				|| (parm->eModeMember ==
				GSW_IGMP_MEMBER_EXCLUDE)) {
				hitbl->mctable[i].slsbindex = slix;
				hitbl->mctable[i].smsbindex = smix;
				if (slflag)
					hpctbl->pce_sub_tbl.iplsbtcnt[slix] = 1;
				else
					hpctbl->pce_sub_tbl.iplsbtcnt[slix]++;
				if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
					if (smflag)
						hpctbl->pce_sub_tbl.ipmsbtcnt[smix] = 1;
					else
						hpctbl->pce_sub_tbl.ipmsbtcnt[smix]++;
				}
			} else if (parm->eModeMember ==
				GSW_IGMP_MEMBER_DONT_CARE) {
				hitbl->mctable[i].slsbindex = 0x7F;
				hitbl->mctable[i].smsbindex = 0x7F;
			}
		}
		hitbl->mctable[i].fid = (parm->nFID & 0x3F);
		hitbl->mctable[i].subifid = (parm->nSubIfId & 0xFFFF);
		memset(&ptdata, 0, sizeof(pctbl_prog_t));
		/* Now, we write into Multicast SW Table */
		ptdata.table = PCE_MULTICAST_SW_INDEX;
		ptdata.pcindex = i;
		ptdata.key[1] = ((hitbl->mctable[i].smsbindex << 8)
			| (hitbl->mctable[i].slsbindex));
		ptdata.key[0] = ((hitbl->mctable[i].dmsbindex << 8)
			| (hitbl->mctable[i].dlsbindex));

		ptdata.val[0] = hitbl->mctable[i].pmap;
		ptdata.valid = hitbl->mctable[i].valid;
		ptdata.val[1] = (parm->nSubIfId & 0xFFFF)	<< 3;
		ptdata.key[2] |= parm->nFID & 0x3F;
		if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
			/* ptdata.key[2] |= (parm->bExclSrcIP & 1) << 15; */
			ptdata.key[2] |= (1 << 15);
			hitbl->mctable[i].exclude = 1;
		}
		gsw_pce_table_write(cdev, &ptdata);
	}
	return GSW_statusOk;
}

/* Multicast Software Table Include/Exclude Remove function */
static int gsw3x_msw_table_rm(void *cdev, GSW_multicastTable_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8	i, j;
	ltq_bool_t MATCH = 0;
	ltq_pce_table_t *hpctbl = &gswdev->phandler;
	gsw_igmp_t *hitbl = &gswdev->iflag;
	pctbl_prog_t ptdata;
	pce_dasa_lsb_t ltbl;
	pce_dasa_msb_t mtbl;
	int dlix = 0x7F, dmix = 0x7F, slix = 0x7F, smix = 0x7F;
	memset(&ptdata, 0, sizeof(pctbl_prog_t));
	memset(&ltbl, 0, sizeof(pce_dasa_lsb_t));
	memset(&mtbl, 0, sizeof(pce_dasa_msb_t));
	if ((parm->eIPVersion != GSW_IP_SELECT_IPV4)
		&& (parm->eIPVersion != GSW_IP_SELECT_IPV6)) {
		pr_err("%s:%s:%d (IPv4/IPV6 need to enable!!!)\n",
			__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE) &&
		(parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) &&
		(parm->eModeMember == GSW_IGMP_MEMBER_DONT_CARE)) {
		pr_err("%s:%s:%d (!!!)\n", __FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0; i < 4; i++)
			ltbl.ilsb[i] = ((parm->uIP_Gda.nIPv4 >> (i * 8))
				& 0xFF);
		ltbl.mask[0] = 0x0; ltbl.mask[1] = 0x0;
		ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV6 /* IPv6 */) {
		/* First, search for DIP in the DA/SA table (DIP MSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			mtbl.imsb[j-1] = (parm->uIP_Gda.nIPv6[i] & 0xFF);
			mtbl.imsb[j] = ((parm->uIP_Gda.nIPv6[i] >> 8) & 0xFF);
		}
		mtbl.mask[0] = 0; mtbl.mask[1] = 0;
		mtbl.mask[2] = 0; mtbl.mask[3] = 0;
		dmix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl,
			&mtbl);
		if (dmix == 0xFF) {
			pr_err("%s:%s:%d (IGMP Entry not found)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			ltbl.ilsb[j-1] = (parm->uIP_Gda.nIPv6[i+4] & 0xFF);
			ltbl.ilsb[j] = ((parm->uIP_Gda.nIPv6[i+4] >> 8) & 0xFF);
		}
		ltbl.mask[0] = 0; ltbl.mask[1] = 0;
		ltbl.mask[2] = 0; ltbl.mask[3] = 0;
	}
	dlix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl,
		&ltbl);
	if (dlix == 0xFF) {
		pr_err("%s:%s:%d (IGMP Entry not found)\n",
			__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
		|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
		if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0; i < 4; i++)
				ltbl.ilsb[i] = ((parm->uIP_Gsa.nIPv4 >> (i * 8))
				& 0xFF);
			ltbl.mask[0] = 0x0; ltbl.mask[1] = 0x0;
			ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if (ltbl.ilsb[3] == 0 &&
					(ltbl.ilsb[2] == 0) &&
					(ltbl.ilsb[1] == 0) &&
					(ltbl.ilsb[0] == 0)) {
					pr_err("%s:%s:%d (Exclude SIP is Wildcard)\n",
					__FILE__, __func__, __LINE__);
					return GSW_statusErr;
				}
			}
		}
		if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
			int src_zero = 0;
			/* First, search for DIP in the DA/SA table (DIP MSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				mtbl.imsb[j-1] = (parm->uIP_Gsa.nIPv6[i]
					& 0xFF);
				mtbl.imsb[j] = ((parm->uIP_Gsa.nIPv6[i] >> 8)
					& 0xFF);
			}
			mtbl.mask[0] = 0; mtbl.mask[1] = 0;
			mtbl.mask[2] = 0; mtbl.mask[3] = 0;
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((mtbl.imsb[0] == 0) &&
					(mtbl.imsb[1] == 0) &&
					(mtbl.imsb[2] == 0) &&
					(mtbl.imsb[3] == 0) &&
					(mtbl.imsb[4] == 0) &&
					(mtbl.imsb[5] == 0) &&
					(mtbl.imsb[6] == 0) &&
					(mtbl.imsb[7] == 0))
					src_zero = 1;
			}
			smix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl,
				&mtbl);
			if (smix == 0xFF) {
				pr_err("%s:%s:%d (IGMP Entry not found)\n",
				__FILE__, __func__, __LINE__);
				return GSW_statusErr;
			}
	/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				ltbl.ilsb[j-1] = (parm->uIP_Gsa.nIPv6[i+4]
					& 0xFF);
				ltbl.ilsb[j] = ((parm->uIP_Gsa.nIPv6[i+4] >> 8)
					& 0xFF);
			}
			ltbl.mask[0] = 0; ltbl.mask[1] = 0;
			ltbl.mask[2] = 0; ltbl.mask[3] = 0;
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((ltbl.ilsb[0] == 0) &&
					(ltbl.ilsb[1] == 0) &&
					(ltbl.ilsb[2] == 0) &&
					(ltbl.ilsb[3] == 0) &&
					(ltbl.ilsb[4] == 0) &&
					(ltbl.ilsb[5] == 0) &&
					(ltbl.ilsb[6] == 0) &&
					(ltbl.ilsb[7] == 0)) {
					if (src_zero) {
						pr_err("%s:%s:%d (Exclude SIP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
						return GSW_statusErr;
					}
				}
			}
		}
		slix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl,
			&ltbl);
		if (slix == 0xFF) {
			pr_err("%s:%s:%d (IGMP Entry not found)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
	}
	for (i = 0; i < gswdev->iflag.itblsize; i++) {
		if ((hitbl->mctable[i].dlsbindex == dlix) &&
		(hitbl->mctable[i].slsbindex == slix) &&
		(hitbl->mctable[i].dmsbindex == dmix) &&
		(hitbl->mctable[i].smsbindex == smix) &&
		(hitbl->mctable[i].valid == 1) &&
		(hitbl->mctable[i].fid == parm->nFID)) {
			switch (hitbl->mctable[i].mcmode) {
			case GSW_IGMP_MEMBER_DONT_CARE:
				if (((hitbl->mctable[i].pmap >> parm->nPortId) & 0x1) == 1) {
					hitbl->mctable[i].pmap &= ~(1 << parm->nPortId);
					if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] > 0) {
						ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, dlix);
						if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] == 0)
							/* Delet the sub table */
							ip_dasa_lsb_tbl_del(cdev,
									&hpctbl->pce_sub_tbl, dlix);
					}
			/* Delet the sub table */
					if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] > 0) {
						ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, dmix);
						if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] == 0) {
							if (parm->eIPVersion == GSW_IP_SELECT_IPV6)
								ip_dasa_msb_tbl_del(cdev,
									&hpctbl->pce_sub_tbl, dmix);
						}
					}
			/* Check the port map status */
			/* Delet the entry from Multicast sw Table */
					if (hitbl->mctable[i].pmap == 0)
						hitbl->mctable[i].valid = 0;
					MATCH = 1;
				}
				break;
			case GSW_IGMP_MEMBER_INCLUDE:
			case GSW_IGMP_MEMBER_EXCLUDE:
				if (((hitbl->mctable[i].pmap >> parm->nPortId) & 0x1) == 1) {
					hitbl->mctable[i].pmap &= ~(1 << parm->nPortId);
					if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] > 0) {
						ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, dlix);
			/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] == 0) {
							ip_dasa_lsb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, dlix);
						}
					}
					if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] > 0) {
						ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, dmix);
		/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] == 0) {
							ip_dasa_msb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, dmix);
						}
					}
					if (hpctbl->pce_sub_tbl.iplsbtcnt[slix] > 0) {
						ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, slix);
						/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.iplsbtcnt[slix] == 0) {
							ip_dasa_lsb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, slix);
						}
					}
					if (hpctbl->pce_sub_tbl.ipmsbtcnt[smix] > 0) {
						ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, smix);
						/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.ipmsbtcnt[smix] == 0) {
							ip_dasa_msb_tbl_del(cdev,
									&hpctbl->pce_sub_tbl, smix);
						}
					}
					/* Check the port map status */
					if (hitbl->mctable[i].pmap == 0) {
						/* Delet the entry from Multicast sw Table */
						hitbl->mctable[i].valid = 0;
					}
					MATCH = 1;
				}
				break;
			}
			if (MATCH == 1) {
				memset(&ptdata, 0, sizeof(pctbl_prog_t));
				ptdata.table = PCE_MULTICAST_SW_INDEX;
				ptdata.pcindex = i;
				ptdata.key[1] = ((hitbl->mctable[i].smsbindex << 8)
					| (hitbl->mctable[i].slsbindex));
				ptdata.key[0] = ((hitbl->mctable[i].dmsbindex << 8)
					| (hitbl->mctable[i].dlsbindex));
				ptdata.key[2] = (hitbl->mctable[i].fid) & 0x3F;
				ptdata.key[2] |= (hitbl->mctable[i].exclude) << 15;
				ptdata.val[0] = hitbl->mctable[i].pmap;
				ptdata.val[1] = (hitbl->mctable[i].subifid)	<< 3;
				ptdata.valid = hitbl->mctable[i].valid;
				gsw_pce_table_write(cdev, &ptdata);
			}
		}
	}
	if (MATCH == 0)
		pr_err("The GIP/SIP not found\n");
	return GSW_statusOk;
}

/* Multicast Software Table Include/Exclude Remove function */
static int gsw2x_msw_table_rm(void *cdev, GSW_multicastTable_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8	i, j;
	ltq_bool_t MATCH = 0;
	ltq_pce_table_t *hpctbl = &gswdev->phandler;
	gsw_igmp_t *hitbl = &gswdev->iflag;
	pctbl_prog_t ptdata;
	pce_dasa_lsb_t ltbl;
	pce_dasa_msb_t mtbl;
	int dlix = 0x7F, dmix = 0x7F, slix = 0x7F, smix = 0x7F;
	memset(&ptdata, 0, sizeof(pctbl_prog_t));
	memset(&ltbl, 0, sizeof(pce_dasa_lsb_t));
	memset(&mtbl, 0, sizeof(pce_dasa_msb_t));
	if ((parm->eIPVersion != GSW_IP_SELECT_IPV4)
		&& (parm->eIPVersion != GSW_IP_SELECT_IPV6)) {
		pr_err("%s:%s:%d (IPv4/IPV6 need to enable!!!)\n",
			__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
		&& (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)
		&& (parm->eModeMember == GSW_IGMP_MEMBER_DONT_CARE)) {
		pr_err("%s:%s:%d (!!!)\n", __FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0; i < 4; i++)
			ltbl.ilsb[i] = ((parm->uIP_Gda.nIPv4 >> (i * 8)) & 0xFF);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			ltbl.mask[0] = 0x0; ltbl.mask[1] = 0x0;
			ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
		} else {
			/* DIP LSB Nibble Mask */
			ltbl.mask[0] = 0xFF00;
		}
	}
	if (parm->eIPVersion == GSW_IP_SELECT_IPV6 /* IPv6 */) {
		/* First, search for DIP in the DA/SA table (DIP MSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			mtbl.imsb[j-1] = (parm->uIP_Gda.nIPv6[i] & 0xFF);
			mtbl.imsb[j] = ((parm->uIP_Gda.nIPv6[i] >> 8) & 0xFF);
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			mtbl.mask[0] = 0; mtbl.mask[1] = 0;
			mtbl.mask[2] = 0; mtbl.mask[3] = 0;
		} else {
			mtbl.mask[0] = 0;/* DIP MSB Nibble Mask */
		}
		dmix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl, &mtbl);
		if (dmix == 0xFF) {
			pr_err("%s:%s:%d (IGMP Entry not found)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			ltbl.ilsb[j-1] = (parm->uIP_Gda.nIPv6[i+4] & 0xFF);
			ltbl.ilsb[j] = ((parm->uIP_Gda.nIPv6[i+4] >> 8) & 0xFF);
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			ltbl.mask[0] = 0; ltbl.mask[1] = 0;
			ltbl.mask[2] = 0; ltbl.mask[3] = 0;
		} else {
			ltbl.mask[0] = 0;/* DIP LSB Nibble Mask */
		}
	}
	dlix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl, &ltbl);
	if (dlix == 0xFF) {
		pr_err("%s:%s:%d (IGMP Entry not found)\n",
			__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->eModeMember == GSW_IGMP_MEMBER_INCLUDE)
		|| (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE)) {
		if (parm->eIPVersion == GSW_IP_SELECT_IPV4) {
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0; i < 4; i++)
				ltbl.ilsb[i] = ((parm->uIP_Gsa.nIPv4 >> (i * 8)) & 0xFF);
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				ltbl.mask[0] = 0x0; ltbl.mask[1] = 0x0;
				ltbl.mask[2] = 0xFFFF; ltbl.mask[3] = 0xFFFF;
			} else {
				/* DIP LSB Nibble Mask */
				ltbl.mask[0] = 0xFF00;
			}
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if (ltbl.ilsb[3] == 0 && ltbl.ilsb[2] == 0
					&& ltbl.ilsb[1] == 0 && ltbl.ilsb[0] == 0) {
					pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
					return GSW_statusErr;
				}
			}
		}
		if (parm->eIPVersion == GSW_IP_SELECT_IPV6) {
			int src_zero = 0;
			/* First, search for DIP in the DA/SA table (DIP MSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				mtbl.imsb[j-1] = (parm->uIP_Gsa.nIPv6[i] & 0xFF);
				mtbl.imsb[j] = ((parm->uIP_Gsa.nIPv6[i] >> 8) & 0xFF);
			}
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				mtbl.mask[0] = 0; mtbl.mask[1] = 0;
				mtbl.mask[2] = 0; mtbl.mask[3] = 0;
			} else {
				mtbl.mask[0] = 0;/* DIP MSB Nibble Mask */
			}
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((mtbl.imsb[0] == 0) &&
					(mtbl.imsb[1] == 0) &&
					(mtbl.imsb[2] == 0) &&
					(mtbl.imsb[3] == 0) &&
					(mtbl.imsb[4] == 0) &&
					(mtbl.imsb[5] == 0) &&
					(mtbl.imsb[6] == 0) &&
					(mtbl.imsb[7] == 0))
					src_zero = 1;
			}
			smix = find_msb_tbl_entry(&hpctbl->pce_sub_tbl,
				&mtbl);
			if (smix == 0xFF) {
				pr_err("%s:%s:%d (IGMP Entry not found)\n",
					__FILE__, __func__, __LINE__);
				return GSW_statusErr;
			}
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				ltbl.ilsb[j-1] = (parm->uIP_Gsa.nIPv6[i+4] & 0xFF);
				ltbl.ilsb[j] = ((parm->uIP_Gsa.nIPv6[i+4] >> 8) & 0xFF);
			}
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				ltbl.mask[0] = 0; ltbl.mask[1] = 0;
				ltbl.mask[2] = 0; ltbl.mask[3] = 0;
			} else {
				ltbl.mask[0] = 0;/* DIP LSB Nibble Mask */
			}
			if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
				if ((ltbl.ilsb[0] == 0) &&
					(ltbl.ilsb[1] == 0) &&
					(ltbl.ilsb[2] == 0) &&
					(ltbl.ilsb[3] == 0) &&
					(ltbl.ilsb[4] == 0) &&
					(ltbl.ilsb[5] == 0) &&
					(ltbl.ilsb[6] == 0) &&
					(ltbl.ilsb[7] == 0)) {
					if (src_zero) {
						pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard)\n",
						__FILE__, __func__, __LINE__);
						return GSW_statusErr;
					}
				}
			}
		}
		slix = find_dasa_tbl_entry(&hpctbl->pce_sub_tbl,
			&ltbl);
		if (slix == 0xFF) {
			pr_err("%s:%s:%d (IGMP Entry not found)\n",
				__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
	}
	for (i = 0; i < gswdev->iflag.itblsize; i++) {
		if ((hitbl->mctable[i].dlsbindex == dlix) &&
			(hitbl->mctable[i].slsbindex == slix) &&
			(hitbl->mctable[i].dmsbindex == dmix) &&
			(hitbl->mctable[i].smsbindex == smix) &&
			(hitbl->mctable[i].valid == 1)) {

			switch (hitbl->mctable[i].mcmode) {
			case GSW_IGMP_MEMBER_DONT_CARE:
				if (((hitbl->mctable[i].pmap >> parm->nPortId) & 0x1) == 1) {
					hitbl->mctable[i].pmap &= ~(1 << parm->nPortId);
					if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] > 0) {
						ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, dlix);
						if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] == 0) {
							/* Delet the sub table */
							ip_dasa_lsb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, dlix);
						}
					}
		/* Delet the sub table */
					if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] > 0) {
						ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, dmix);
						if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] == 0) {
							if (parm->eIPVersion == GSW_IP_SELECT_IPV6)
								ip_dasa_msb_tbl_del(cdev,
									&hpctbl->pce_sub_tbl, dmix);
						}
					}
			/* Check the port map status */
			/* Delet the entry from Multicast sw Table */
					if (hitbl->mctable[i].pmap == 0)
						hitbl->mctable[i].valid = 0;
					MATCH = 1;
				}
				break;
			case GSW_IGMP_MEMBER_INCLUDE:
			case GSW_IGMP_MEMBER_EXCLUDE:
				if (((hitbl->mctable[i].pmap >> parm->nPortId) & 0x1) == 1) {
					hitbl->mctable[i].pmap &= ~(1 << parm->nPortId);
					if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] > 0) {
						ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, dlix);
					/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] == 0) {
							ip_dasa_lsb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, dlix);
						}
					}
					if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] > 0) {
						ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, dmix);
		/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] == 0) {
							ip_dasa_msb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, dmix);
						}
					}
					if (hpctbl->pce_sub_tbl.iplsbtcnt[slix] > 0) {
						ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, slix);
		/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.iplsbtcnt[slix] == 0) {
							ip_dasa_lsb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, slix);
						}
					}
					if (hpctbl->pce_sub_tbl.ipmsbtcnt[smix] > 0) {
						ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, smix);
		/* Delet the sub table */
						if (hpctbl->pce_sub_tbl.ipmsbtcnt[smix] == 0) {
							ip_dasa_msb_tbl_del(cdev,
								&hpctbl->pce_sub_tbl, smix);
						}
					}
			/* Check the port map status */
			/* Delet the entry from Multicast sw Table */
					if (hitbl->mctable[i].pmap == 0)
						hitbl->mctable[i].valid = 0;

					MATCH = 1;
					if (parm->eModeMember == GSW_IGMP_MEMBER_EXCLUDE) {
						for (j = 0; j < gswdev->iflag.itblsize; j++) {
							if ((hitbl->mctable[j].dlsbindex == dlix) &&
								(hitbl->mctable[j].slsbindex == 0x7F) &&
								(hitbl->mctable[j].dmsbindex == dmix) &&
								(hitbl->mctable[j].smsbindex == 0x7F) &&
								(hitbl->mctable[j].valid == 1)) {
								if (((hitbl->mctable[j].pmap >> parm->nPortId) & 0x1) == 1) {
									hitbl->mctable[j].pmap &= ~(1 << parm->nPortId);
									if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] > 0) {
										ipdslsb_tblidx_del(&hpctbl->pce_sub_tbl, dlix);
										if (hpctbl->pce_sub_tbl.iplsbtcnt[dlix] == 0) {
						/* Delet the sub table */
											ip_dasa_lsb_tbl_del(cdev,
												&hpctbl->pce_sub_tbl, dlix);
										}
									}
								if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] > 0) {
									ipdsmsb_tblidx_del(&hpctbl->pce_sub_tbl, dmix);
									if (hpctbl->pce_sub_tbl.ipmsbtcnt[dmix] == 0) {
							/* Delet the sub table */
										ip_dasa_msb_tbl_del(cdev,
											&hpctbl->pce_sub_tbl, dmix);
									}
								}
								/* Check the port map status */
								if (hitbl->mctable[j].pmap == 0) {
									/* Delet the entry from Multicast sw Table */
									hitbl->mctable[j].valid = 0;
									hitbl->mctable[i].valid = 0;
								}
								memset(&ptdata, 0, sizeof(pctbl_prog_t));
								ptdata.table = PCE_MULTICAST_SW_INDEX;
								ptdata.pcindex = j;
								ptdata.key[1] = ((0x7F << 8) | 0x7F);
								ptdata.key[0] = ((hitbl->mctable[j].dmsbindex << 8)
									| (hitbl->mctable[i].dlsbindex));
								ptdata.val[0] = hitbl->mctable[j].pmap;
								ptdata.valid = hitbl->mctable[j].valid;
								gsw_pce_table_write(cdev, &ptdata);
							}
						}
					}
				}
			}
			break;
			}
			if (MATCH == 1) {
				memset(&ptdata, 0, sizeof(pctbl_prog_t));
				ptdata.table = PCE_MULTICAST_SW_INDEX;
				ptdata.pcindex = i;
				ptdata.key[1] = ((hitbl->mctable[i].smsbindex << 8)
					| (hitbl->mctable[i].slsbindex));
				ptdata.key[0] = ((hitbl->mctable[i].dmsbindex << 8)
					| (hitbl->mctable[i].dlsbindex));
				ptdata.val[0] = hitbl->mctable[i].pmap;
				ptdata.valid = hitbl->mctable[i].valid;
				gsw_pce_table_write(cdev, &ptdata);
			}
		}
	}
	if (MATCH == 0)
		pr_err("The GIP/SIP not found\n");
	return GSW_statusOk;
}
#endif /*CONFIG_LTQ_MULTICAST */
#if	((defined(CONFIG_LTQ_STP) && CONFIG_LTQ_STP) ||  (defined(CONFIG_LTQ_8021X) && CONFIG_LTQ_8021X))

/* Function: Internal function to program the registers */
/* when 802.1x and STP API are called. */
/* Description: Referene the matrix table to program the  */
/* LRNLIM, PSTATE and PEN bit*/
/* according to the Software architecture spec design.*/
static void set_port_state(void *cdev, u32 pid,
	u32 stpstate, u32 st8021)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 i;

	for (i = 0; i < sizeof(pstpstate)/sizeof(pstpstate_t); i++) {
		pstpstate_t *pststate = &pstpstate[i];
		if ((pststate->psstate == stpstate) &&
			(pststate->ps8021x == st8021)) {
			gswdev->pconfig[pid].penable = pststate->pen_reg;
			gswdev->pconfig[pid].ptstate = pststate->pstate_reg;
			/* Learning Limit */
			if (pststate->lrnlim == 0) {
				gsw_w32(cdev, (PCE_PCTRL_1_LRNLIM_OFFSET
					+ (0xA * pid)),
					PCE_PCTRL_1_LRNLIM_SHIFT,
					PCE_PCTRL_1_LRNLIM_SIZE, 0);
			} else {
				gsw_w32(cdev, (PCE_PCTRL_1_LRNLIM_OFFSET
					+ (0xA * pid)),
					PCE_PCTRL_1_LRNLIM_SHIFT,
					PCE_PCTRL_1_LRNLIM_SIZE,
					gswdev->pconfig[pid].llimit);
			}
			/* Port State */
			gsw_w32(cdev, (PCE_PCTRL_0_PSTATE_OFFSET + (0xA * pid)),
				PCE_PCTRL_0_PSTATE_SHIFT,
				PCE_PCTRL_0_PSTATE_SIZE,
				gswdev->pconfig[pid].ptstate);
			/* Port Enable */
			gsw_w32(cdev, (SDMA_PCTRL_PEN_OFFSET + (0xA * pid)),
				SDMA_PCTRL_PEN_SHIFT,
				SDMA_PCTRL_PEN_SIZE,
				gswdev->pconfig[pid].penable);
		}
	}
}
#endif /* CONFIG_LTQ_STP / CONFIG_LTQ_8021X */
#if defined(CONFIG_LTQ_QOS) && CONFIG_LTQ_QOS

/* Function: Internal function to calculate the mrate when */
/* Shaper and Meter API is called.*/
/*  Description: Calculate the mrate by input Ibs, Exp and Mant.*/
/* The algorithm designed based on software architecture spec.*/
static u32 mratecalc(u32 ibsid, u32 expont, u32 mants)
{
	static const u16 ibs_table[] = {8*8, 32*8, 64*8, 96*8};
	u16 ibs;
	u32 mrate = 0;

	if ((ibsid == 0) && (expont == 0) && (mants == 0))
		return 0;
	ibs = ibs_table[ibsid];
	if(mants)
		mrate = ((ibs * 25000) >> expont) / mants;
	return mrate;
}

/* Function: Internal function to calculate the Token when */
/* Shaper and Meter API is called.*/
/*  Description: Calculate the Token by input mrate, Ibs, Exp and Mant.*/
/* The algorithm designed based on software architecture spec. */
static int calc_mtoken(u32 mrate, u32 *ibsid, u32 *expont, u32 *mants)
{
	static const u16 ibs_table[] = {8*8, 32*8, 64*8, 96*8};
	u8 i;

	for (i = 3; i >= 0; i--) {
		u32 exp;
		u16 ibs = ibs_table[i];
		/* target is to get the biggest mantissa value */
	/* that can be used for the 10-Bit register */
		for (exp = 0; exp < 16; exp++) {
			u32 mant = ((ibs * 25000) >> exp) / mrate;
			if (mant < (1 << 10))  {
				*ibsid = i;
				*expont = exp;
				*mants = mant;
				return 0;
			}
		}
	}
}
#endif /*CONFIG_LTQ_QOS */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
static void reset_vlan_sw_table(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u16 i;
	for (i = 0; i < gswdev->avlantsz /* VLAN_ACTIVE_TABLE_SIZE */; i++)
		memset(&gswdev->avtable[i], 0, sizeof(avlan_tbl_t));
}
#endif /*CONFIG_LTQ_VLAN*/

#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
u8 find_active_vlan_index(void *cdev, u16 vid)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 i, index = 0xFF;
	for (i = 0; i < gswdev->avlantsz; i++) {
		if ((vid == gswdev->avtable[i].vid)
			&& (gswdev->avtable[i].valid == 1)) {
			index = i;
			break;
		}
	}
	return index;
}

u8 fempty_avlan_index_table(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 i, index = 0xFF;
	for (i = 1; i < gswdev->avlantsz; i += 1) {
		if (gswdev->avtable[i].valid == 0) {
			index = i;
			break;
		}
	}
	if ((index == 0xFF) &&
		(gswdev->avtable[0].valid == 0))
		return 0;
	return index;
}

static void vlan_entry_set(void *cdev, u8 index,
	avlan_tbl_t *avlan_entry)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;

	gswdev->avtable[index].valid = avlan_entry->valid;
	gswdev->avtable[index].reserved	= avlan_entry->reserved;
	gswdev->avtable[index].vid = avlan_entry->vid;
	gswdev->avtable[index].fid = avlan_entry->fid;
	gswdev->avtable[index].pm = avlan_entry->pm;
	gswdev->avtable[index].tm = avlan_entry->tm;
}

static void get_vlan_sw_table(void *cdev, u8 pcindex,
	avlan_tbl_t *avlan_entry)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;

	avlan_entry->valid = gswdev->avtable[pcindex].valid;
	avlan_entry->reserved = gswdev->avtable[pcindex].reserved;
	avlan_entry->vid = gswdev->avtable[pcindex].vid;
	avlan_entry->fid = gswdev->avtable[pcindex].fid;
	avlan_entry->pm = gswdev->avtable[pcindex].pm;
	avlan_entry->tm = gswdev->avtable[pcindex].tm;

}
#endif /* CONFIG_LTQ_VLAN */
static void get_gsw_hw_cap(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 reg_val;
	gsw_r32(cdev, ETHSW_CAP_1_PPORTS_OFFSET,
		ETHSW_CAP_1_PPORTS_SHIFT,
		ETHSW_CAP_1_PPORTS_SIZE, &reg_val);
	gswdev->pnum = reg_val;
	pr_err("%s:%s:%d (pnum:%d)\n", __FILE__,
		__func__, __LINE__, gswdev->pnum);
	gsw_r32(cdev, ETHSW_CAP_1_VPORTS_OFFSET,
		ETHSW_CAP_1_VPORTS_SHIFT,
		ETHSW_CAP_1_VPORTS_SIZE, &reg_val);
	gswdev->tpnum = reg_val + gswdev->pnum;
/*	pr_err("%s:%s:%d (tpnum:%d)\n", __FILE__,
		__func__, __LINE__, gswdev->tpnum);*/
	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET,
		ETHSW_CAP_1_QUEUE_SHIFT,
		ETHSW_CAP_1_QUEUE_SIZE, &reg_val);
	gswdev->num_of_queues = reg_val;
	gsw_r32(cdev, ETHSW_CAP_3_METERS_OFFSET,
		ETHSW_CAP_3_METERS_SHIFT,
		ETHSW_CAP_3_METERS_SIZE, &reg_val);
	gswdev->num_of_meters = reg_val;
	gsw_r32(cdev, ETHSW_CAP_3_SHAPERS_OFFSET,
		ETHSW_CAP_3_SHAPERS_SHIFT,
		ETHSW_CAP_3_SHAPERS_SIZE, &reg_val);
	gswdev->num_of_shapers = reg_val;
	gsw_r32(cdev, ETHSW_CAP_4_PPPOE_OFFSET,
		ETHSW_CAP_4_PPPOE_SHIFT,
		ETHSW_CAP_4_PPPOE_SIZE, &reg_val);
	gswdev->num_of_pppoe = reg_val;
	gsw_r32(cdev, ETHSW_CAP_4_VLAN_OFFSET,
		ETHSW_CAP_4_VLAN_SHIFT,
		ETHSW_CAP_4_VLAN_SIZE, &reg_val);
	gswdev->avlantsz = reg_val;
	gsw_r32(cdev, ETHSW_CAP_5_IPPLEN_OFFSET,
		ETHSW_CAP_5_IPPLEN_SHIFT,
		ETHSW_CAP_5_IPPLEN_SIZE, &reg_val);
	gswdev->ip_pkt_lnt_size = reg_val;
	gsw_r32(cdev, ETHSW_CAP_5_PROT_OFFSET,
		ETHSW_CAP_5_PROT_SHIFT,
		ETHSW_CAP_5_PROT_SIZE, &reg_val);
	gswdev->prot_table_size = reg_val;
	gsw_r32(cdev, ETHSW_CAP_6_MACDASA_OFFSET,
		ETHSW_CAP_6_MACDASA_SHIFT,
		ETHSW_CAP_6_MACDASA_SIZE, &reg_val);
	gswdev->mac_dasa_table_size = reg_val;
	gsw_r32(cdev, ETHSW_CAP_6_APPL_OFFSET,
		ETHSW_CAP_6_APPL_SHIFT,
		ETHSW_CAP_6_APPL_SIZE, &reg_val);
	gswdev->app_table_size = reg_val;
	gsw_r32(cdev, ETHSW_CAP_7_IPDASAM_OFFSET,
		ETHSW_CAP_7_IPDASAM_SHIFT,
		ETHSW_CAP_7_IPDASAM_SIZE, &reg_val);
	gswdev->idsmtblsize = reg_val;
	gsw_r32(cdev, ETHSW_CAP_7_IPDASAL_OFFSET,
		ETHSW_CAP_7_IPDASAL_SHIFT,
		ETHSW_CAP_7_IPDASAL_SIZE, &reg_val);
	gswdev->idsltblsize = reg_val;
	gsw_r32(cdev, ETHSW_CAP_8_MCAST_OFFSET,
		ETHSW_CAP_8_MCAST_SHIFT,
		ETHSW_CAP_8_MCAST_SIZE, &reg_val);
	gswdev->mctblsize = reg_val;
	gsw_r32(cdev, ETHSW_CAP_9_FLAGG_OFFSET,
		ETHSW_CAP_9_FLAGG_SHIFT,
		ETHSW_CAP_9_FLAGG_SIZE, &reg_val);
	gswdev->tftblsize = reg_val;
	gsw_r32(cdev, ETHSW_CAP_10_MACBT_OFFSET,
		ETHSW_CAP_10_MACBT_SHIFT,
		ETHSW_CAP_10_MACBT_SIZE, &reg_val);
	gswdev->mactblsize = reg_val;
	gsw_r32(cdev, ETHSW_VERSION_REV_ID_OFFSET,
		ETHSW_VERSION_REV_ID_SHIFT, 16, &reg_val);
	gswdev->gipver = reg_val;
	pr_err("%s:%s:%d (gswdev->gipver:%d)\n",
		__FILE__, __func__, __LINE__, gswdev->gipver);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, ETHSW_CAP_13_PMAC_OFFSET,
			ETHSW_CAP_13_PMAC_SHIFT,
			ETHSW_CAP_13_PMAC_SIZE, &reg_val);
		gswdev->num_of_pmac = reg_val;
		gsw_r32(cdev, ETHSW_CAP_13_PAYLOAD_OFFSET,
			ETHSW_CAP_13_PAYLOAD_SHIFT,
			ETHSW_CAP_13_PAYLOAD_SIZE, &reg_val);
		gswdev->pdtblsize = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_13_INTRMON_OFFSET,
			ETHSW_CAP_13_INTRMON_SHIFT,
			ETHSW_CAP_13_INTRMON_SIZE, &reg_val);
		gswdev->num_of_ifrmon = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_13_EVLAN_OFFSET,
			ETHSW_CAP_13_EVLAN_SHIFT,
			ETHSW_CAP_13_EVLAN_SIZE, &reg_val);
		gswdev->num_of_egvlan = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_14_SMAC_OFFSET,
			ETHSW_CAP_14_SMAC_SHIFT,
			ETHSW_CAP_14_SMAC_SIZE, &reg_val);
		gswdev->num_of_rt_smac = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_14_DMAC_OFFSET,
			ETHSW_CAP_14_DMAC_SHIFT,
			ETHSW_CAP_14_DMAC_SIZE, &reg_val);
		gswdev->num_of_rt_dmac = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_14_PPPoE_OFFSET,
			ETHSW_CAP_14_PPPoE_SHIFT,
			ETHSW_CAP_14_PPPoE_SIZE, &reg_val);
		gswdev->num_of_rt_ppoe = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_14_NAT_OFFSET,
			ETHSW_CAP_14_NAT_SHIFT,
			ETHSW_CAP_14_NAT_SIZE, &reg_val);
		gswdev->num_of_rt_nat = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_15_MTU_OFFSET,
			ETHSW_CAP_15_MTU_SHIFT,
			ETHSW_CAP_15_MTU_SIZE, &reg_val);
		gswdev->num_of_rt_mtu = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_15_TUNNEL_OFFSET,
			ETHSW_CAP_15_TUNNEL_SHIFT,
			ETHSW_CAP_15_TUNNEL_SIZE, &reg_val);
		gswdev->num_of_rt_tunnel = (1 << reg_val);
		gsw_r32(cdev, ETHSW_CAP_15_RTP_OFFSET,
			ETHSW_CAP_15_RTP_SHIFT,
			ETHSW_CAP_15_RTP_SIZE, &reg_val);
		gswdev->num_of_rt_rtp = (1 << reg_val);
		gswdev->cport = GSW_3X_SOC_CPU_PORT;
	} else {
		gswdev->cport = GSW_2X_SOC_CPU_PORT;
	}

}

/*	This is the switch core layer init function.*/
/*	\param ethcinit This parameter is a pointer */
/* to the switch core context. */
/*	\return Return value as follows: */
/*	cdev: if successful */
void *ethsw_api_core_init(ethsw_core_init_t *ethcinit)
{
	int j;
	ethsw_api_dev_t *cdev;
	cdev = (ethsw_api_dev_t *)kmalloc(sizeof(ethsw_api_dev_t), GFP_KERNEL);
	if (!cdev) {
		pr_err("%s:%s:%d (memory allocation failed)\n",
		__FILE__, __func__, __LINE__);
		return cdev;
	}
	memset(cdev, 0, sizeof(ethsw_api_dev_t));
	cdev->raldev = ethcinit->ecdev;
	ecoredev[ethcinit->sdev] = cdev;
	cdev->sdev = ethcinit->sdev;
	cdev->gsw_base = ethcinit->gsw_base_addr;
	get_gsw_hw_cap(cdev);
	/* -----  end switch  ----- */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
	reset_vlan_sw_table(cdev);
#endif /* CONFIG_LTQ_VLAN */
	ltq_ethsw_port_cfg_init(cdev);
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
	reset_multicast_sw_table(cdev);
#endif /*CONFIG_LTQ_MULTICAST*/
	pce_table_init(&cdev->phandler);
	cdev->rst = 0;
	cdev->hwinit = 0;
	cdev->matimer = DEFAULT_AGING_TIMEOUT;
/*	pr_err("Switch API: PCE MicroCode loaded !!\n");*/
	gsw_pmicro_code_init(cdev);
	/* Set the the source MAC register of pause frame  */
	gsw_w32(cdev, MAC_PFSA_0_PFAD_OFFSET,
		MAC_PFSA_0_PFAD_SHIFT,
		MAC_PFSA_0_PFAD_SIZE, 0x0000);
	gsw_w32(cdev, MAC_PFSA_1_PFAD_OFFSET,
		MAC_PFSA_1_PFAD_SHIFT,
		MAC_PFSA_1_PFAD_SIZE, 0x9600);
	gsw_w32(cdev, MAC_PFSA_2_PFAD_OFFSET,
		MAC_PFSA_2_PFAD_SHIFT,
		MAC_PFSA_2_PFAD_SIZE, 0xAC9A);
	if (cdev->gipver == LTQ_GSWIP_3_0) {
#if defined(CONFIG_USE_EMULATOR) && CONFIG_USE_EMULATOR
		if (cdev->gipver == LTQ_GSWIP_3_0) {
			if (cdev->sdev == LTQ_FLOW_DEV_INT_R)
				gsw_r_init();
	/* Set Auto-Polling of connected PHYs - For all ports */
				gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
					GSWT_MDCCFG_0_PEN_1_SHIFT, 6, 0x0);
			} else {
	/* Set Auto-Polling of connected PHYs - For all ports */
				gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET
					+ GSW_TREG_OFFSET),
					MDC_CFG_0_PEN_0_SHIFT, 6, 0x0);
			}
		for (j = 0; j < cdev->pnum-1; j++) {
			u32 reg_value;
			if (cdev->gipver == LTQ_GSWIP_3_0) {
				gsw_w32(cdev,
					(FDMA_PCTRL_EN_OFFSET + (j * 0x6)),
					FDMA_PCTRL_EN_SHIFT,
					FDMA_PCTRL_EN_SIZE, 1);
				gsw_w32(cdev,
					(SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),
					SDMA_PCTRL_PEN_SHIFT,
					SDMA_PCTRL_PEN_SIZE, 1);
				gsw_w32(cdev,
					(BM_PCFG_CNTEN_OFFSET + (j * 2)),
					BM_PCFG_CNTEN_SHIFT,
					BM_PCFG_CNTEN_SIZE, 1);
				gsw_w32(cdev,
					(MAC_CTRL_0_FDUP_OFFSET + (0xC * j)),
					MAC_CTRL_0_FDUP_SHIFT,
					MAC_CTRL_0_FDUP_SIZE, 1);
				gsw_w32(cdev,
					(MAC_CTRL_0_GMII_OFFSET + (0xC * j)),
					MAC_CTRL_0_GMII_SHIFT,
					MAC_CTRL_0_GMII_SIZE, 2);

				gsw_w32(cdev,
				((GSWT_PHY_ADDR_1_SPEED_OFFSET + (j * 4))
				+ GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_SPEED_SHIFT,
				GSWT_PHY_ADDR_1_SPEED_SIZE, 2);
				gsw_w32(cdev,
					((GSWT_PHY_ADDR_1_FDUP_OFFSET
					+ (j * 4))
					+ GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_FDUP_SHIFT,
					GSWT_PHY_ADDR_1_FDUP_SIZE, 1);
				gsw_w32(cdev,
					((GSWT_PHY_ADDR_1_LNKST_OFFSET
					+ (j * 4))
					+ GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_LNKST_SHIFT,
					GSWT_PHY_ADDR_1_LNKST_SIZE, 1);
				gsw_r32(cdev,
					((GSWT_PHY_ADDR_1_LNKST_OFFSET
					+ (j * 4))
					+ GSW30_TOP_OFFSET),
					0, 16, &reg_value);
			}
		}
#else
	/* Configure the MDIO Clock 97.6 Khz */
		gsw_w32(cdev, (GSWT_MDCCFG_1_FREQ_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_FREQ_SHIFT, GSWT_MDCCFG_1_FREQ_SIZE, 0xFF);
		gsw_w32(cdev, (GSWT_MDCCFG_1_MCEN_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_MCEN_SHIFT, GSWT_MDCCFG_1_MCEN_SIZE, 1);
		/* MDIO Hardware Reset */
		gsw_w32(cdev, (GSWT_MDCCFG_1_RES_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_RES_SHIFT, GSWT_MDCCFG_1_RES_SIZE, 1);
		if (cdev->sdev == LTQ_FLOW_DEV_INT_R) {
			gsw_r_init();
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, 0x1);
/*			gsw_w32(cdev, (GSWT_PHY_ADDR_1_LNKST_OFFSET	+ GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_LNKST_SHIFT, GSWT_PHY_ADDR_1_LNKST_SIZE, 1);*/
		} else {
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
				+ GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, 0x1e);
		}
		for (j = 0; j < cdev->pnum-1; j++) {
			if (cdev->gipver == LTQ_GSWIP_3_0) {
				gsw_w32(cdev,
					(FDMA_PCTRL_EN_OFFSET + (j * 0x6)),
					FDMA_PCTRL_EN_SHIFT,
					FDMA_PCTRL_EN_SIZE, 1);
				gsw_w32(cdev,
					(SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),
					SDMA_PCTRL_PEN_SHIFT,
					SDMA_PCTRL_PEN_SIZE, 1);
				gsw_w32(cdev,
					(BM_PCFG_CNTEN_OFFSET + (j * 2)),
					BM_PCFG_CNTEN_SHIFT,
					BM_PCFG_CNTEN_SIZE, 1);
			}
		}
		for (j = 0; j < cdev->pnum-1; j++) {
			gsw_w32(cdev,
				((GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_OFFSET
				+ (4 * j)) + GSW30_TOP_OFFSET),
				GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_SHIFT,
				GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_SIZE, 0x3);
		}
#endif
	} else {
		/* Configure the MDIO Clock 97.6 Khz */
		gsw_w32(cdev, (MDC_CFG_1_FREQ_OFFSET
			+ GSW_TREG_OFFSET),
			MDC_CFG_1_FREQ_SHIFT,
			MDC_CFG_1_FREQ_SIZE, 0xFF);
		/* EEE auto negotiation overides:  */
		/*clock disable (ANEG_EEE_0.CLK_STOP_CAPABLE)  */
		for (j = 0; j < cdev->pnum-1; j++) {
			gsw_w32(cdev,
			((ANEG_EEE_0_CLK_STOP_CAPABLE_OFFSET+j)
			+ GSW_TREG_OFFSET),
			ANEG_EEE_0_CLK_STOP_CAPABLE_SHIFT,
			ANEG_EEE_0_CLK_STOP_CAPABLE_SIZE, 0x3);
		}
	}
	if (cdev->gipver == LTQ_GSWIP_3_0) {
		for (j = 0; j < cdev->tpnum; j++) {
			gsw_w32(cdev, (BM_PCFG_CNTEN_OFFSET + (j * 2)), BM_PCFG_CNTEN_SHIFT,
				BM_PCFG_CNTEN_SIZE, 1);
			gsw_w32(cdev, (BM_RMON_CTRL_BCAST_CNT_OFFSET + (j * 0x2)),
				BM_RMON_CTRL_BCAST_CNT_SHIFT, BM_RMON_CTRL_BCAST_CNT_SIZE, 1);
		}
		if (cdev->sdev == LTQ_FLOW_DEV_INT_R) {
			gsw_w32(cdev, PCE_TFCR_NUM_NUM_OFFSET, PCE_TFCR_NUM_NUM_SHIFT,
				PCE_TFCR_NUM_NUM_SIZE, 0x80);
		}
	}

#ifdef CONFIG_X86_INTEL_CE2700
	cport_sgmii_config(cdev);
	GSW_Enable(cdev);
#endif /*CONFIG_X86_INTEL_CE2700*/

	return cdev;
}

/**	This is the switch core layer cleanup function.*/
/*	\return Return value as follows: */
/*	GSW_statusOk: if successful */
void gsw_corecleanup(void)
{
	u8 i;
	ethsw_api_dev_t *cdev;
	for (i = 0; i < LTQ_FLOW_DEV_MAX; i++) {
		cdev = (ethsw_api_dev_t *)ecoredev[i];
		if (cdev) {
			kfree(cdev);
			cdev = NULL;
		}
	}
}

GSW_return_t GSW_MAC_TableClear(void *cdev)
{
	/*  flush all entries from the MAC table */
	gsw_w32(cdev, PCE_GCTRL_0_MTFL_OFFSET,
		PCE_GCTRL_0_MTFL_SHIFT,
		PCE_GCTRL_0_MTFL_SIZE, 1);
	return GSW_statusOk;
}

GSW_return_t GSW_MAC_TableEntryAdd(void *cdev,
	GSW_MAC_tableAdd_t *parm)
{
	pctbl_prog_t tbl_prog;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->nPortId >= gswdev->tpnum) &&
		(!(parm->nPortId & 0x80000000)))
		return GSW_statusErr;
	memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	tbl_prog.key[0] = parm->nMAC[4] << 8 | parm->nMAC[5];
	tbl_prog.key[1] = parm->nMAC[2] << 8 | parm->nMAC[3];
	tbl_prog.key[2] = parm->nMAC[0] << 8 | parm->nMAC[1];
	tbl_prog.key[3] = parm->nFId;
	if (gswdev->gipver == LTQ_GSWIP_2_2_ETC)
		tbl_prog.val[1] = ((parm->nSubIfId & 0xFFF) << 4);
/*	tbl_prog.val[1] = ((parm->nSVLAN_Id & 0xFFF) << 4); */
	if (gswdev->gipver == LTQ_GSWIP_3_0)
		tbl_prog.val[1] = ((parm->nSubIfId & 0x1FFF) << 3);

	tbl_prog.val[1] |= (1 << 1); /* Valid Entry */
	tbl_prog.valid = 1;
	if (parm->bStaticEntry) {
		if (parm->nPortId & 0x80000000) { /*Port Map */
			tbl_prog.val[0] = (parm->nPortId & 0x7FFF);
		} else {
			tbl_prog.val[0] = (1 << parm->nPortId);
		}
		tbl_prog.val[1] |= 1;
	} else {
		tbl_prog.val[0] = (((parm->nPortId & 0xF) << 4)
			| (parm->nAgeTimer & 0xF));
	}
	gsw_pce_table_key_write(cdev, &tbl_prog);
#if 0
	/* To ensure MAC is updated in the table */
	memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	tbl_prog.key[0] = parm->nMAC[4] << 8 | parm->nMAC[5];
	tbl_prog.key[1] = parm->nMAC[2] << 8 | parm->nMAC[3];
	tbl_prog.key[2] = parm->nMAC[0] << 8 | parm->nMAC[1];
	tbl_prog.key[3] = parm->nFId;
	gsw_pce_table_key_read(cdev, &tbl_prog);
	if (tbl_prog.valid != 1)
		pr_warn("(MAC Table is full) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
#endif
	return GSW_statusOk;
}

GSW_return_t GSW_MAC_TableEntryRead(void *cdev,
	GSW_MAC_tableRead_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t tbl_prog;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
	if (parm->bInitial == 1) {
		gswdev->mac_rd_index = 0; /*Start from the index 0 */
		parm->bInitial = 0;
	}
	if (gswdev->mac_rd_index >= gswdev->mactblsize) {
		memset(parm, 0, sizeof(GSW_MAC_tableRead_t));
		parm->bLast = 1;
		gswdev->mac_rd_index = 0;
		return 0;
	}
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	do {
		tbl_prog.pcindex = gswdev->mac_rd_index;
		gsw_pce_table_read(cdev, &tbl_prog);
		gswdev->mac_rd_index++;
		if (tbl_prog.valid != 0)
			break;
	} while (gswdev->mac_rd_index < gswdev->mactblsize);
	if (tbl_prog.valid == 1) {
		parm->nFId = tbl_prog.key[3] & 0x3F;
		parm->bStaticEntry = (tbl_prog.val[1] & 0x1);
		if (parm->bStaticEntry == 1) {
			parm->nAgeTimer = 0;
			parm->nPortId = tbl_prog.val[0];
		} else {
			u32 mant, timer = 300;
			/* Aging Counter Mantissa Value */
			gsw_r32(cdev, PCE_AGE_1_MANT_OFFSET,
				PCE_AGE_1_MANT_SHIFT,
				PCE_AGE_1_MANT_SIZE, &mant);
			switch (mant) {
			case AGETIMER_1_DAY:
				timer = 86400;
				break;
			case AGETIMER_1_HOUR:
				timer = 3600;
				break;
			case AGETIMER_300_SEC:
				timer = 300;
				break;
			case AGETIMER_10_SEC:
				timer = 10;
				break;
			case AGETIMER_1_SEC:
				timer = 1;
				break;
			}
			parm->nAgeTimer =	tbl_prog.val[0] & 0xF;
			parm->nAgeTimer =	(timer * parm->nAgeTimer)/0xF;
			parm->nPortId =	(tbl_prog.val[0] >> 4) & 0xF;
		}
		parm->nMAC[0] = tbl_prog.key[2] >> 8;
		parm->nMAC[1] = tbl_prog.key[2] & 0xFF;
		parm->nMAC[2] = tbl_prog.key[1] >> 8;
		parm->nMAC[3] = tbl_prog.key[1] & 0xFF;
		parm->nMAC[4] = tbl_prog.key[0] >> 8;
		parm->nMAC[5] = tbl_prog.key[0] & 0xFF;
		if (gswdev->gipver == LTQ_GSWIP_2_2_ETC)
			parm->nSubIfId = (tbl_prog.val[1] >> 4 & 0xFFF);
/*	parm->nSVLAN_Id = (tbl_prog.val[1] >> 4 & 0xFFF); */

		if (gswdev->gipver == LTQ_GSWIP_3_0)
			parm->nSubIfId = (tbl_prog.val[1] >> 3 & 0x1FFF);

		parm->bInitial = 0;
		parm->bLast = 0;
	} else {
		memset(parm, 0, sizeof(GSW_MAC_tableRead_t));
		parm->bLast = 1;
	}
	return 0;
}

GSW_return_t GSW_MAC_TableEntryQuery(void *cdev,
	GSW_MAC_tableQuery_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t tbl_prog;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	parm->bFound = 0;
	memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	tbl_prog.key[0] = parm->nMAC[4] << 8 | parm->nMAC[5];
	tbl_prog.key[1] = parm->nMAC[2] << 8 | parm->nMAC[3];
	tbl_prog.key[2] = parm->nMAC[0] << 8 | parm->nMAC[1];
	tbl_prog.key[3] = parm->nFId;
	gsw_pce_table_key_read(cdev, &tbl_prog);
	if (tbl_prog.valid == 1) {
		parm->bFound = 1;
		parm->bStaticEntry = (tbl_prog.val[1] & 0x1);
		if (gswdev->gipver == LTQ_GSWIP_2_2_ETC)
			parm->nSubIfId = ((tbl_prog.val[1] >> 4) & 0xFFF);
/*	parm->nSVLAN_Id = ((tbl_prog.val[1] >> 4) & 0xFFF); */

		if (gswdev->gipver == LTQ_GSWIP_3_0)
			parm->nSubIfId = ((tbl_prog.val[1] >> 3) & 0x1FFF);

		if ((tbl_prog.val[1] & 0x1) == 1) {
			parm->nAgeTimer = 0;
			parm->nPortId = (tbl_prog.val[0]);
		} else {
			u32 mant, timer = 300;
			/* Aging Counter Mantissa Value */
			gsw_r32(cdev, PCE_AGE_1_MANT_OFFSET,
				PCE_AGE_1_MANT_SHIFT,
				PCE_AGE_1_MANT_SIZE, &mant);
			switch (mant) {
			case AGETIMER_1_DAY:
				timer = 86400;
				break;
			case AGETIMER_1_HOUR:
				timer = 3600;
				break;
			case AGETIMER_300_SEC:
				timer = 300;
				break;
			case AGETIMER_10_SEC:
				timer = 10;
				break;
			case AGETIMER_1_SEC:
				timer = 1;
				break;
			}
			parm->nAgeTimer = tbl_prog.val[0] & 0xF;
			parm->nAgeTimer = (timer * parm->nAgeTimer)/0xF;
			parm->nPortId = (tbl_prog.val[0] >> 4) & 0xF;
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MAC_TableEntryRemove(void *cdev,
	GSW_MAC_tableRemove_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t tbl_prog;
	u32 index, value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Learning Limit Port Lock */
	gsw_r32(cdev, PCE_GCTRL_0_MTFL_OFFSET,
		PCE_GCTRL_0_MTFL_SHIFT,
		PCE_GCTRL_0_MTFL_SIZE, &value);
/*if value is 1 means, flush all entries from the MAC table */
	if (!value) {
		if ((gswdev->gipver == LTQ_GSWIP_2_0) ||
			(gswdev->gipver == LTQ_GSWIP_2_1)) {
			for (index = 0; index < gswdev->mactblsize; index++) {
				memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
				tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
				tbl_prog.pcindex = index;
				gsw_pce_table_read(cdev, &tbl_prog);
				if ((parm->nMAC[0] == (tbl_prog.key[2] >> 8)) &&
				(parm->nMAC[1] == (tbl_prog.key[2] & 0xFF)) &&
				(parm->nMAC[2] == (tbl_prog.key[1] >> 8)) &&
				(parm->nMAC[3] == (tbl_prog.key[1] & 0xFF)) &&
				(parm->nMAC[4] == (tbl_prog.key[0] >> 8)) &&
				(parm->nMAC[5] == (tbl_prog.key[0] & 0xFF)) &&
				(parm->nFId == (tbl_prog.key[3] & 0x3F))) {
					memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
					tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
					tbl_prog.pcindex = index;
					gsw_pce_table_write(cdev, &tbl_prog);
					break;
				}
			}
		} else {
			memset(&tbl_prog, 0, sizeof(pctbl_prog_t));
			tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
			tbl_prog.key[0] = parm->nMAC[4] << 8 | parm->nMAC[5];
			tbl_prog.key[1] = parm->nMAC[2] << 8 | parm->nMAC[3];
			tbl_prog.key[2] = parm->nMAC[0] << 8 | parm->nMAC[1];
			tbl_prog.key[3] = parm->nFId;
			gsw_pce_table_key_read(cdev, &tbl_prog);
			if (tbl_prog.valid == 1) {
				pctbl_prog_t tbl_cl_prog;
				memset(&tbl_cl_prog, 0, sizeof(pctbl_prog_t));
				tbl_cl_prog.table = PCE_MAC_BRIDGE_INDEX;
				tbl_cl_prog.pcindex = tbl_prog.pcindex;
				gsw_pce_table_write(cdev, &tbl_cl_prog);
			}
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PortCfgGet(void *cdev, GSW_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value, monrx = 0, montx = 0, PEN, EN;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	/* See if PORT enable or not */
	gsw_r32(cdev, (SDMA_PCTRL_PEN_OFFSET + (0x6 * pidx)),
		SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, &PEN);
	gsw_r32(cdev, (FDMA_PCTRL_EN_OFFSET + (0x6 * pidx)),
		FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, &EN);
	/* Port Enable feature only support 6 port */
	 if (pidx >= gswdev->pnum) {
		parm->eEnable = 1;
	} else {
		if ((PEN == 1) && (EN == 1))
			parm->eEnable = GSW_PORT_ENABLE_RXTX;
		else if ((PEN == 1) && (EN == 0))
			parm->eEnable = GSW_PORT_ENABLE_RX;
		else if ((PEN == 0) && (EN == 1))
			parm->eEnable = GSW_PORT_ENABLE_TX;
		else
			parm->eEnable = GSW_PORT_DISABLE;
	}
	/* Learning Limit */
	gsw_r32(cdev, (PCE_PCTRL_1_LRNLIM_OFFSET + (0xA * pidx)),
		PCE_PCTRL_1_LRNLIM_SHIFT,
		PCE_PCTRL_1_LRNLIM_SIZE, &value);
	parm->nLearningLimit = value;

	/* Learning Limit Port Lock */
	gsw_r32(cdev, (PCE_PCTRL_0_PLOCK_OFFSET + (0xA * pidx)),
		PCE_PCTRL_0_PLOCK_SHIFT,
		PCE_PCTRL_0_PLOCK_SIZE, &value);
	parm->bLearningMAC_PortLock = value;
	/* Aging */
	gsw_r32(cdev, PCE_PCTRL_0_AGEDIS_OFFSET + (0xA * pidx),
		PCE_PCTRL_0_AGEDIS_SHIFT,
		PCE_PCTRL_0_AGEDIS_SIZE, &value);
	parm->bAging = value;

	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		/** MAC address table learning on the port specified. */
		gsw_r32(cdev, (PCE_PCTRL_3_LNDIS_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_LNDIS_SHIFT,
			PCE_PCTRL_3_LNDIS_SIZE, &parm->bLearning);
		/** MAC spoofing detection. */
		gsw_r32(cdev, (PCE_PCTRL_0_SPFDIS_OFFSET + (0xA * pidx)),
			PCE_PCTRL_0_SPFDIS_SHIFT,
			PCE_PCTRL_0_SPFDIS_SIZE, &parm->bMAC_SpoofingDetection);
	}
	/* UnicastUnknownDrop */
	gsw_r32(cdev, PCE_PMAP_3_UUCMAP_OFFSET,
		PCE_PMAP_3_UUCMAP_SHIFT,
		PCE_PMAP_3_UUCMAP_SIZE, &value);
	/* UnicastUnknownDrop feature  support  */
	if ((value & (1 << pidx)) == 0)
		parm->bUnicastUnknownDrop = 1;
	else
		parm->bUnicastUnknownDrop = 0;
	/* MulticastUnknownDrop */
	gsw_r32(cdev, PCE_PMAP_2_DMCPMAP_OFFSET,
		PCE_PMAP_2_DMCPMAP_SHIFT,
		PCE_PMAP_2_DMCPMAP_SIZE, &value);
	/* MulticastUnknownDrop feature  support  */
	if ((value & (1 << pidx)) == 0) {
		parm->bMulticastUnknownDrop = 1;
		parm->bBroadcastDrop = 1;
	} else {
		parm->bMulticastUnknownDrop = 0;
		parm->bBroadcastDrop = 0;
	}
	/* Require to check later - 3M */
	parm->bReservedPacketDrop = 0;
	/* Port Monitor */
	gsw_r32(cdev, (PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_RXVMIR_SHIFT,
		PCE_PCTRL_3_RXVMIR_SIZE, &monrx);
	gsw_r32(cdev, (PCE_PCTRL_3_TXMIR_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_TXMIR_SHIFT,
		PCE_PCTRL_3_TXMIR_SIZE, &montx);
	if ((monrx == 1) && (montx == 1))
		parm->ePortMonitor = GSW_PORT_MONITOR_RXTX;
	else if ((monrx == 1) && (montx == 0))
		parm->ePortMonitor = GSW_PORT_MONITOR_RX;
	else if ((monrx == 0) && (montx == 1))
		parm->ePortMonitor = GSW_PORT_MONITOR_TX;
	else
		parm->ePortMonitor = GSW_PORT_MONITOR_NONE;

	gsw_r32(cdev, (PCE_PCTRL_3_VIO_2_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_VIO_2_SHIFT,
		PCE_PCTRL_3_VIO_2_SIZE, &monrx);
	if (monrx == 1)
		parm->ePortMonitor |= GSW_PORT_MONITOR_VLAN_UNKNOWN;

	gsw_r32(cdev, (PCE_PCTRL_3_VIO_4_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_VIO_4_SHIFT,
		PCE_PCTRL_3_VIO_4_SIZE, &monrx);
	if (monrx == 1)
		parm->ePortMonitor |= GSW_PORT_MONITOR_VLAN_MEMBERSHIP;

	gsw_r32(cdev, (PCE_PCTRL_3_VIO_5_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_VIO_5_SHIFT,
		PCE_PCTRL_3_VIO_5_SIZE, &monrx);
	if (monrx == 1)
		parm->ePortMonitor |= GSW_PORT_MONITOR_PORT_STATE;

	gsw_r32(cdev, (PCE_PCTRL_3_VIO_6_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_VIO_6_SHIFT,
		PCE_PCTRL_3_VIO_6_SIZE, &monrx);
	if (monrx == 1)
		parm->ePortMonitor |= GSW_PORT_MONITOR_LEARNING_LIMIT;

		gsw_r32(cdev, (PCE_PCTRL_3_VIO_7_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_VIO_7_SHIFT,
		PCE_PCTRL_3_VIO_7_SIZE, &monrx);
	if (monrx == 1)
		parm->ePortMonitor |= GSW_PORT_MONITOR_PORT_LOCK;
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, (BM_RMON_CTRL_IFRMONFST_OFFSET + (0x2 * pidx)),
			BM_RMON_CTRL_IFRMONFST_SHIFT,
			BM_RMON_CTRL_IFRMONFST_SIZE, &parm->nIfCountStartIdx);
		if (parm->nIfCountStartIdx)
			parm->bIfCounters = 1;
		gsw_r32(cdev, (BM_RMON_CTRL_IFRMONMD_OFFSET + (0x2 * pidx)),
			BM_RMON_CTRL_IFRMONMD_SHIFT,
			BM_RMON_CTRL_IFRMONMD_SIZE, &parm->eIfRMONmode);
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			if (parm->nPortId == 15) {
/*				pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);*/
				gsw_r32(cdev, MAC_PSTAT_TXPAUEN_OFFSET,
					MAC_PSTAT_TXPAUEN_SHIFT, 2, &value);
			}
		} else {
			if ((parm->nPortId |= 0 ) && (parm->nPortId < (gswdev->pnum - 1))) {
				gsw_r32(cdev, (MAC_PSTAT_TXPAUEN_OFFSET + (0xC * (parm->nPortId - 1 ))),
					MAC_PSTAT_TXPAUEN_SHIFT, 2, &value);
			}
		}
	} else {
		gsw_r32(cdev, (MAC_PSTAT_TXPAUEN_OFFSET + (0xC * pidx)),
			MAC_PSTAT_TXPAUEN_SHIFT, 2, &value);
	}
	switch (value) {
	case 0:
			parm->eFlowCtrl = GSW_FLOW_OFF;
		break;
	case 1:
			parm->eFlowCtrl = GSW_FLOW_TX;
		break;
	case 3:
			parm->eFlowCtrl = GSW_FLOW_RXTX;
		break;
	case 2:
			parm->eFlowCtrl = GSW_FLOW_RX;
		break;
	default:
		parm->eFlowCtrl = GSW_FLOW_AUTO;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PortCfgSet(void *cdev, GSW_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 paddr;
	u32 value, EN, PEN, PACT;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	/* Learning Limit Port Lock */
	gsw_w32(cdev, (PCE_PCTRL_0_PLOCK_OFFSET + (0xA * pidx)),
		PCE_PCTRL_0_PLOCK_SHIFT,
		PCE_PCTRL_0_PLOCK_SIZE, parm->bLearningMAC_PortLock);
	/* Learning Limit Action */
	if (parm->nLearningLimit == 0)
		value = 0;
	else if (parm->nLearningLimit == 0xFFFF)
		value = 0xFF;
	else
		value = parm->nLearningLimit;
	gswdev->pconfig[parm->nPortId].llimit = value;
	/* Learning Limit */
	gsw_w32(cdev, (PCE_PCTRL_1_LRNLIM_OFFSET + (0xA * pidx)),
		PCE_PCTRL_1_LRNLIM_SHIFT,
		PCE_PCTRL_1_LRNLIM_SIZE, value);
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		/** MAC address table learning on the port specified */
		gsw_w32(cdev, (PCE_PCTRL_3_LNDIS_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_LNDIS_SHIFT,
			PCE_PCTRL_3_LNDIS_SIZE, parm->bLearning);
		/** MAC spoofing detection. */
		gsw_w32(cdev, (PCE_PCTRL_0_SPFDIS_OFFSET + (0xA * pidx)),
			PCE_PCTRL_0_SPFDIS_SHIFT,
			PCE_PCTRL_0_SPFDIS_SIZE, parm->bMAC_SpoofingDetection);
	}
	/* Aging */
	gsw_w32(cdev, PCE_PCTRL_0_AGEDIS_OFFSET + (0xA * pidx),
		PCE_PCTRL_0_AGEDIS_SHIFT,
		PCE_PCTRL_0_AGEDIS_SIZE, parm->bAging);
	/* UnicastUnknownDrop Read first */
	gsw_r32(cdev, PCE_PMAP_3_UUCMAP_OFFSET,
		PCE_PMAP_3_UUCMAP_SHIFT,
		PCE_PMAP_3_UUCMAP_SIZE, &value);
	if (parm->bUnicastUnknownDrop == 1)
		value &= ~(1 << pidx);
	else
		value |= 1 << pidx;
	/* UnicastUnknownDrop write back */
	gsw_w32(cdev, PCE_PMAP_3_UUCMAP_OFFSET,
		PCE_PMAP_3_UUCMAP_SHIFT,
		PCE_PMAP_3_UUCMAP_SIZE, value);
	/* MulticastUnknownDrop */
	gsw_r32(cdev, PCE_PMAP_2_DMCPMAP_OFFSET,
		PCE_PMAP_2_DMCPMAP_SHIFT,
		PCE_PMAP_2_DMCPMAP_SIZE, &value);
	if (parm->bMulticastUnknownDrop == 1)
		value &= ~(1 << pidx);
	else
		value |= 1 << pidx;
	/* MulticastUnknownDrop */
	gsw_w32(cdev, PCE_PMAP_2_DMCPMAP_OFFSET,
		PCE_PMAP_2_DMCPMAP_SHIFT,
		PCE_PMAP_2_DMCPMAP_SIZE, value);
	/* Flow Control */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if ((pidx == 0 /*GSW_3X_SOC_CPU_PORT*/)) {
			PEN = 0;
			PACT = 0;
		} else {
			if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
				if (parm->nPortId == 15) {
					gsw_r32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
						(GSWT_MDCCFG_0_PEN_1_SHIFT), GSWT_MDCCFG_0_PEN_1_SIZE, &PEN);
					gsw_r32(cdev, (GSWT_MDIO_STAT_1_PACT_OFFSET + GSW30_TOP_OFFSET),
						GSWT_MDIO_STAT_1_PACT_SHIFT, GSWT_MDIO_STAT_1_PACT_SIZE, &PACT);
					gsw_r32(cdev, (GSWT_PHY_ADDR_1_ADDR_OFFSET  + GSW30_TOP_OFFSET),
							GSWT_PHY_ADDR_1_ADDR_SHIFT, GSWT_PHY_ADDR_1_ADDR_SIZE, &paddr);
				} else {
					PEN = 0;
					PACT = 0;
				}
			} else {
				if (pidx < gswdev->pnum ) {
					gsw_r32(cdev, (GSWT_MDCCFG_0_PEN_0_OFFSET + GSW30_TOP_OFFSET),
						(GSWT_MDCCFG_0_PEN_0_SHIFT + pidx),	GSWT_MDCCFG_0_PEN_0_SIZE, &PEN);
					gsw_r32(cdev, ((GSWT_MDIO_STAT_1_PACT_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
						GSWT_MDIO_STAT_1_PACT_SHIFT, GSWT_MDIO_STAT_1_PACT_SIZE, &PACT);
					gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((parm->nPortId - 1) * 4)) + GSW30_TOP_OFFSET),
							GSWT_PHY_ADDR_1_ADDR_SHIFT, GSWT_PHY_ADDR_1_ADDR_SIZE, &paddr);
					} else {
						PEN = 0;
						PACT = 0;
					}
				}
			}
	} else {
		if (pidx < gswdev->pnum ) {
			gsw_r32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
			(MDC_CFG_0_PEN_0_SHIFT + pidx), MDC_CFG_0_PEN_0_SIZE, &PEN);
			gsw_r32(cdev, (MDIO_STAT_0_PACT_OFFSET + GSW_TREG_OFFSET + pidx),
			MDIO_STAT_0_PACT_SHIFT, MDIO_STAT_0_PACT_SIZE, &PACT);
			gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &paddr);
		} else {
			PEN = 0;
			PACT = 0;
		}
	}
	/* PHY polling statemachine (of the MAC) is activated and */
/* an external PHY reacts on the MDIO accesses. */
/* Therefore update the MDIO register of the attached PHY.*/
	if ((PEN == 1) && (PACT == 1)) {
		GSW_MDIO_data_t mddata;
		/* Write directly to MDIO register */
		mddata.nAddressDev = paddr;
		mddata.nAddressReg = 0x4;
		GSW_MDIO_DataRead(gswdev, &mddata);
		mddata.nData &= ~(0xC00);
		switch (parm->eFlowCtrl) {
		case GSW_FLOW_OFF:
			break;
		case GSW_FLOW_TX:
			mddata.nData |= 0x800;
			break;
		case GSW_FLOW_RXTX:
			mddata.nData |= 0x400;
			break;
		case GSW_FLOW_RX:
		case GSW_FLOW_AUTO:
			mddata.nData |= 0xC00;
			break;
		}
		GSW_MDIO_DataWrite(gswdev, &mddata);
		/* Restart Auto negotiation */
		mddata.nAddressReg = 0x0;
		GSW_MDIO_DataRead(gswdev, &mddata);
		mddata.nData |= 0x1200;
		GSW_MDIO_DataWrite(gswdev, &mddata);
	} else {
		u32 RX = 0, TX = 0;
		switch (parm->eFlowCtrl) {
		case GSW_FLOW_OFF:
			RX = 3; TX = 3;
			break;
		case GSW_FLOW_TX:
			RX = 3; TX = 2;
			break;
		case GSW_FLOW_RXTX:
			RX = 2; TX = 2;
			break;
		case GSW_FLOW_RX:
			RX = 2; TX = 3;
			break;
		case GSW_FLOW_AUTO:
			RX = 0; TX = 0;
			break;
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			if (pidx != 0) {
				if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
					if (pidx == 15) {
						gsw_w32(cdev, MAC_CTRL_0_FCON_OFFSET, MAC_CTRL_0_FCON_SHIFT,
							MAC_CTRL_0_FCON_SIZE, parm->eFlowCtrl);
						gsw_w32(cdev, (GSWT_PHY_ADDR_1_FCONTX_OFFSET+ GSW30_TOP_OFFSET),
							GSWT_PHY_ADDR_1_FCONTX_SHIFT, GSWT_PHY_ADDR_1_FCONTX_SIZE, TX);
						gsw_w32(cdev, (GSWT_PHY_ADDR_1_FCONRX_OFFSET + GSW30_TOP_OFFSET),
							GSWT_PHY_ADDR_1_FCONRX_SHIFT,	GSWT_PHY_ADDR_1_FCONRX_SIZE, RX);
					}
				} else {
					if (pidx < gswdev->pnum ) {
						gsw_w32(cdev, (MAC_CTRL_0_FCON_OFFSET + (0xC * (pidx-1))),
							MAC_CTRL_0_FCON_SHIFT, MAC_CTRL_0_FCON_SIZE, parm->eFlowCtrl);
						gsw_w32(cdev, ((GSWT_PHY_ADDR_1_FCONTX_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
							GSWT_PHY_ADDR_1_FCONTX_SHIFT, GSWT_PHY_ADDR_1_FCONTX_SIZE, TX);
						gsw_w32(cdev, ((GSWT_PHY_ADDR_1_FCONRX_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
							GSWT_PHY_ADDR_1_FCONRX_SHIFT, GSWT_PHY_ADDR_1_FCONRX_SIZE, RX);
					}
				}
			}
		} else {
			if (pidx < gswdev->pnum ) {
				gsw_w32(cdev, (MAC_CTRL_0_FCON_OFFSET + (0xC * pidx)),
					MAC_CTRL_0_FCON_SHIFT, MAC_CTRL_0_FCON_SIZE, parm->eFlowCtrl);
				gsw_w32(cdev, (PHY_ADDR_0_FCONTX_OFFSET - (0x1 * pidx)),
					PHY_ADDR_0_FCONTX_SHIFT, PHY_ADDR_0_FCONTX_SIZE , TX);
				gsw_w32(cdev, (PHY_ADDR_0_FCONRX_OFFSET - (0x1 * pidx)),
					PHY_ADDR_0_FCONRX_SHIFT, PHY_ADDR_0_FCONRX_SIZE, RX);
			}
		}
	}
	switch (parm->ePortMonitor) {
	case GSW_PORT_MONITOR_NONE:
		gsw_w32(cdev,
			(PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_RXVMIR_SHIFT,
			PCE_PCTRL_3_RXVMIR_SIZE, 0);
		gsw_w32(cdev,
			(PCE_PCTRL_3_TXMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_TXMIR_SHIFT,
			PCE_PCTRL_3_TXMIR_SIZE, 0);
		gsw_w32(cdev,
			(PCE_PCTRL_3_VIO_2_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_2_SHIFT,
			PCE_PCTRL_3_VIO_2_SIZE, 0);
		gsw_w32(cdev,
			(PCE_PCTRL_3_VIO_4_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_4_SHIFT,
			PCE_PCTRL_3_VIO_4_SIZE, 0);
		gsw_w32(cdev,
			(PCE_PCTRL_3_VIO_5_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_5_SHIFT,
			PCE_PCTRL_3_VIO_5_SIZE, 0);
		gsw_w32(cdev,
			(PCE_PCTRL_3_VIO_6_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_6_SHIFT,
			PCE_PCTRL_3_VIO_6_SIZE, 0);
		gsw_w32(cdev,
			(PCE_PCTRL_3_VIO_7_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_7_SHIFT,
			PCE_PCTRL_3_VIO_7_SIZE, 0);
		break;
	case GSW_PORT_MONITOR_RX:
		gsw_w32(cdev, (PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_RXVMIR_SHIFT,
			PCE_PCTRL_3_RXVMIR_SIZE, 1);
		gsw_w32(cdev, (PCE_PCTRL_3_TXMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_TXMIR_SHIFT,
			PCE_PCTRL_3_TXMIR_SIZE, 0);
		break;
	case GSW_PORT_MONITOR_TX:
		gsw_w32(cdev, (PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_RXVMIR_SHIFT,
			PCE_PCTRL_3_RXVMIR_SIZE, 0);
		gsw_w32(cdev, (PCE_PCTRL_3_TXMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_TXMIR_SHIFT,
			PCE_PCTRL_3_TXMIR_SIZE, 1);
		break;
	case GSW_PORT_MONITOR_RXTX:
		gsw_w32(cdev, (PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_RXVMIR_SHIFT,
			PCE_PCTRL_3_RXVMIR_SIZE, 1);
		gsw_w32(cdev, (PCE_PCTRL_3_TXMIR_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_TXMIR_SHIFT,
			PCE_PCTRL_3_TXMIR_SIZE, 1);
		break;
	case GSW_PORT_MONITOR_VLAN_UNKNOWN:
		gsw_w32(cdev, (PCE_PCTRL_3_VIO_2_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_2_SHIFT,
			PCE_PCTRL_3_VIO_2_SIZE, 1);
		break;
	case GSW_PORT_MONITOR_VLAN_MEMBERSHIP:
		gsw_w32(cdev, (PCE_PCTRL_3_VIO_4_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_4_SHIFT,
			PCE_PCTRL_3_VIO_4_SIZE, 1);
		break;
	case GSW_PORT_MONITOR_PORT_STATE:
		gsw_w32(cdev, (PCE_PCTRL_3_VIO_5_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_5_SHIFT,
			PCE_PCTRL_3_VIO_5_SIZE, 1);
		break;
	case GSW_PORT_MONITOR_LEARNING_LIMIT:
		gsw_w32(cdev, (PCE_PCTRL_3_VIO_6_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_6_SHIFT,
			PCE_PCTRL_3_VIO_6_SIZE, 1);
		break;
	case GSW_PORT_MONITOR_PORT_LOCK:
		gsw_w32(cdev, (PCE_PCTRL_3_VIO_7_OFFSET + (0xA * pidx)),
			PCE_PCTRL_3_VIO_7_SHIFT,
			PCE_PCTRL_3_VIO_7_SIZE, 1);
		break;
	}
	if (parm->eEnable == GSW_PORT_ENABLE_RXTX) {
		PEN = 1;
		EN = 1;
	} else if (parm->eEnable == GSW_PORT_ENABLE_RX) {
		PEN = 1;
		EN = 0;
	} else if (parm->eEnable == GSW_PORT_ENABLE_TX) {
		PEN = 0;
		EN = 1;
	} else {
		PEN = 0;
		EN = 0;
	}
	/* Set SDMA_PCTRL_PEN PORT enable */
	gsw_w32(cdev, (SDMA_PCTRL_PEN_OFFSET + (6 * pidx)),
		SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, PEN);
	/* Set FDMA_PCTRL_EN PORT enable  */
	gsw_w32(cdev, (FDMA_PCTRL_EN_OFFSET + (0x6 * pidx)),
		FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, EN);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (parm->bIfCounters == 1) {
			gsw_w32(cdev, (BM_RMON_CTRL_IFRMONFST_OFFSET + (0x2 * pidx)),
				BM_RMON_CTRL_IFRMONFST_SHIFT, BM_RMON_CTRL_IFRMONFST_SIZE, parm->nIfCountStartIdx);
		}
		gsw_w32(cdev, (BM_RMON_CTRL_IFRMONMD_OFFSET + (0x2 * pidx)),
			BM_RMON_CTRL_IFRMONMD_SHIFT, BM_RMON_CTRL_IFRMONMD_SIZE, parm->eIfRMONmode);
	}
	return GSW_statusOk;
}
#if defined(CONFIG_LTQ_STP) && CONFIG_LTQ_STP
GSW_return_t GSW_STP_BPDU_RuleGet(void *cdev,
	GSW_STP_BPDU_Rule_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	stp8021x_t *scfg;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	scfg = &gswdev->stpconfig;
	parm->eForwardPort = scfg->spstate;
	parm->nForwardPortId = scfg->stppid;
	return GSW_statusOk;
}

GSW_return_t GSW_STP_BPDU_RuleSet(void *cdev,
	GSW_STP_BPDU_Rule_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	stp8021x_t *scfg = &gswdev->stpconfig;
	GSW_PCE_rule_t pcrule;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	scfg->spstate = parm->eForwardPort;
	scfg->stppid = parm->nForwardPortId;
	memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
	/* Attached the PCE rule for BPDU packet */
	pcrule.pattern.nIndex	= BPDU_PCE_RULE_INDEX;
	pcrule.pattern.bEnable = 1;
	pcrule.pattern.bMAC_DstEnable	= 1;
	pcrule.pattern.nMAC_Dst[0] = 0x01;
	pcrule.pattern.nMAC_Dst[1] = 0x80;
	pcrule.pattern.nMAC_Dst[2] = 0xC2;
	pcrule.pattern.nMAC_Dst[3] = 0x00;
	pcrule.pattern.nMAC_Dst[4] = 0x00;
	pcrule.pattern.nMAC_Dst[5] = 0x00;
	pcrule.action.eCrossStateAction	= GSW_PCE_ACTION_CROSS_STATE_CROSS;
	if ((scfg->spstate < 4) &&
		(scfg->spstate > 0))
		pcrule.action.ePortMapAction = scfg->spstate + 1;
	else
		pr_warn("(Incorrect forward port action) %s:%s:%d\n",
		__FILE__, __func__, __LINE__);
	pcrule.action.nForwardPortMap = (1 << scfg->stppid);
	/* We prepare everything and write into PCE Table */
	if (0 != pce_rule_write(gswdev, &gswdev->phandler, &pcrule))
		return GSW_statusErr;
	return GSW_statusOk;
}

GSW_return_t GSW_STP_PortCfgGet(void *cdev, GSW_STP_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	parm->ePortState = gswdev->pconfig[parm->nPortId].pcstate;
	return GSW_statusOk;
}

GSW_return_t GSW_STP_PortCfgSet(void *cdev, GSW_STP_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	gswdev->pconfig[parm->nPortId].pcstate = parm->ePortState;
	/* Config the Table */
	set_port_state(cdev,
		parm->nPortId, gswdev->pconfig[parm->nPortId].pcstate,
		gswdev->pconfig[parm->nPortId].p8021xs);
	return GSW_statusOk;
}

GSW_return_t GSW_TrunkingCfgGet(void *cdev, GSW_trunkingCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	/* Supported for GSWIP 2.2 and newer and returns */
	/*with an error for older hardware revisions. */
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
		/* Destination IP Mask */
		gsw_r32(cdev, PCE_TRUNK_CONF_DIP_OFFSET,
			PCE_TRUNK_CONF_DIP_SHIFT,
			PCE_TRUNK_CONF_DIP_SIZE, &parm->bIP_Dst);
		/* 'Source IP Mask */
		gsw_r32(cdev, PCE_TRUNK_CONF_SIP_OFFSET,
			PCE_TRUNK_CONF_SIP_SHIFT,
			PCE_TRUNK_CONF_SIP_SIZE, &parm->bIP_Src);
		/* Destination MAC Mask */
		gsw_r32(cdev, PCE_TRUNK_CONF_DA_OFFSET,
			PCE_TRUNK_CONF_DA_SHIFT,
			PCE_TRUNK_CONF_DA_SIZE, &parm->bMAC_Dst);
		/* 'Source MAC Mask */
		gsw_r32(cdev, PCE_TRUNK_CONF_SA_OFFSET,
			PCE_TRUNK_CONF_SA_SHIFT,
			PCE_TRUNK_CONF_SA_SIZE, &parm->bMAC_Src);
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

GSW_return_t GSW_TrunkingCfgSet(void *cdev,
	GSW_trunkingCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	/* Supported for GSWIP 2.2 and newer and returns */
	/* with an error for older hardware revisions. */
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
		/* Destination IP Mask */
		if (parm->bIP_Dst == 1) {
			gsw_w32(cdev, PCE_TRUNK_CONF_DIP_OFFSET,
				PCE_TRUNK_CONF_DIP_SHIFT,
				PCE_TRUNK_CONF_DIP_SIZE, 1);
		} else {
			gsw_w32(cdev, PCE_TRUNK_CONF_DIP_OFFSET,
				PCE_TRUNK_CONF_DIP_SHIFT,
				PCE_TRUNK_CONF_DIP_SIZE, 0);
		}
		/* 'Source IP Mask */
		if (parm->bIP_Src == 1) {
			gsw_w32(cdev, PCE_TRUNK_CONF_SIP_OFFSET,
				PCE_TRUNK_CONF_SIP_SHIFT,
				PCE_TRUNK_CONF_SIP_SIZE, 1);
		} else {
			gsw_w32(cdev, PCE_TRUNK_CONF_SIP_OFFSET,
				PCE_TRUNK_CONF_SIP_SHIFT,
				PCE_TRUNK_CONF_SIP_SIZE, 0);
		}
		/* Destination MAC Mask */
		if (parm->bMAC_Dst == 1) {
			gsw_w32(cdev, PCE_TRUNK_CONF_DA_OFFSET,
				PCE_TRUNK_CONF_DA_SHIFT,
				PCE_TRUNK_CONF_DA_SIZE, 1);
		} else {
			gsw_w32(cdev, PCE_TRUNK_CONF_DA_OFFSET,
				PCE_TRUNK_CONF_DA_SHIFT,
				PCE_TRUNK_CONF_DA_SIZE, 0);
		}
		/* 'Source MAC Mask */
		if (parm->bMAC_Src == 1) {
			gsw_w32(cdev, PCE_TRUNK_CONF_SA_OFFSET,
				PCE_TRUNK_CONF_SA_SHIFT,
				PCE_TRUNK_CONF_SA_SIZE, 1);
		} else {
			gsw_w32(cdev, PCE_TRUNK_CONF_SA_OFFSET,
				PCE_TRUNK_CONF_SA_SHIFT,
				PCE_TRUNK_CONF_SA_SIZE, 0);
		}
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

GSW_return_t GSW_TrunkingPortCfgGet(void *cdev,
	GSW_trunkingPortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= (gswdev->tpnum))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns with */
	/* an error for older hardware revisions. */
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
/** Ports are aggregated. the 'nPortId' and the */
/* 'nAggrPortId' ports form an aggregated link. */
		gsw_r32(cdev, (PCE_PTRUNK_EN_OFFSET + (parm->nPortId * 0x2)),
			PCE_PTRUNK_EN_SHIFT, PCE_PTRUNK_EN_SIZE, &value);
		 parm->bAggregateEnable = value;
		gsw_r32(cdev, (PCE_PTRUNK_PARTER_OFFSET + (parm->nPortId * 0x2)),
			PCE_PTRUNK_PARTER_SHIFT, PCE_PTRUNK_PARTER_SIZE, &value);
		parm->nAggrPortId = value;
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

GSW_return_t GSW_TrunkingPortCfgSet(void *cdev, GSW_trunkingPortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= (gswdev->tpnum))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns */
	/* with an error for older hardware revisions. */
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
		 /** Ports are aggregated. the 'nPortId' and the */
		 /* 'nAggrPortId' ports form an aggregated link.*/
		if (parm->bAggregateEnable == 1) {
			gsw_w32(cdev, (PCE_PTRUNK_EN_OFFSET + (parm->nPortId * 0x2)),
				PCE_PTRUNK_EN_SHIFT, 	PCE_PTRUNK_EN_SIZE, 1);
			gsw_w32(cdev, (PCE_PTRUNK_PARTER_OFFSET + (parm->nPortId * 0x2)),
				PCE_PTRUNK_PARTER_SHIFT, PCE_PTRUNK_PARTER_SIZE, (parm->nAggrPortId & 0xF));
		}
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

GSW_return_t GSW_TimestampTimerSet(void *cdev, GSW_TIMESTAMP_Timer_t *parm)
{
	u32 value;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Supported for GSWIP 2.2 and newer and returns with */
	/* an error for older hardware revisions. */
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
		/** Second. Absolute second timer count. */
		gsw_w32(cdev, (TIMER_SEC_LSB_SECLSB_OFFSET),
			TIMER_SEC_LSB_SECLSB_SHIFT,
			TIMER_SEC_LSB_SECLSB_SIZE, (parm->nSec & 0xFFFF));
		gsw_w32(cdev, (TIMER_SEC_MSB_SECMSB_OFFSET),
			TIMER_SEC_MSB_SECMSB_SHIFT,
			TIMER_SEC_MSB_SECMSB_SIZE,
			((parm->nSec >> 16) & 0xFFFF));
		/** Nano Second. Absolute nano second timer count.*/
		gsw_w32(cdev, (TIMER_NS_LSB_NSLSB_OFFSET),
			TIMER_NS_LSB_NSLSB_SHIFT,
			TIMER_NS_LSB_NSLSB_SIZE,
			(parm->nNanoSec & 0xFFFF));
		gsw_w32(cdev, (TIMER_NS_MSB_NSMSB_OFFSET),
			TIMER_NS_MSB_NSMSB_SHIFT,
			TIMER_NS_MSB_NSMSB_SIZE,
			((parm->nNanoSec >> 16) & 0xFFFF));
/** Fractional Nano Second. Absolute fractional nano */
/* second timer count. This counter specifis a */
/* 2^32 fractional 'nNanoSec'. */
		gsw_w32(cdev, (TIMER_FS_LSB_FSLSB_OFFSET),
			TIMER_FS_LSB_FSLSB_SHIFT,
			TIMER_FS_LSB_FSLSB_SIZE,
			(parm->nFractionalNanoSec & 0xFFFF));
		gsw_w32(cdev, (TIMER_FS_MSB_FSMSB_OFFSET),
			TIMER_FS_MSB_FSMSB_SHIFT,
			TIMER_FS_MSB_FSMSB_SIZE,
			((parm->nFractionalNanoSec >> 16) & 0xFFFF));
		value = 1;
		gsw_w32(cdev, (TIMER_CTRL_WR_OFFSET),
			TIMER_CTRL_WR_SHIFT,
			TIMER_CTRL_WR_SIZE, value);
		do {
			gsw_r32(cdev, (TIMER_CTRL_WR_OFFSET),
				TIMER_CTRL_WR_SHIFT,
				TIMER_CTRL_WR_SIZE, &value);
		} while (value == 1);
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

GSW_return_t GSW_TimestampTimerGet(void *cdev,
	GSW_TIMESTAMP_Timer_t *parm)
{
	u32 value;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	value = 1;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Supported for GSWIP 2.2 and newer and returns */
	/* with an error for older hardware revisions. */
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
		gsw_w32(cdev, (TIMER_CTRL_RD_OFFSET),
			TIMER_CTRL_RD_SHIFT,
			TIMER_CTRL_RD_SIZE, value);
		do {
			gsw_r32(cdev, (TIMER_CTRL_RD_OFFSET),
				TIMER_CTRL_RD_SHIFT,
				TIMER_CTRL_RD_SIZE, &value);
		} while (value == 1);
		/** Second. Absolute second timer count. */
		gsw_r32(cdev, (TIMER_SEC_LSB_SECLSB_OFFSET),
			TIMER_SEC_LSB_SECLSB_SHIFT,
			TIMER_SEC_LSB_SECLSB_SIZE, &value);
		parm->nSec = value & 0xFFFF;
		gsw_r32(cdev, (TIMER_SEC_MSB_SECMSB_OFFSET),
			TIMER_SEC_MSB_SECMSB_SHIFT,
			TIMER_SEC_MSB_SECMSB_SIZE, &value);
		parm->nSec |= (value & 0xFFFF << 16);
		/** Nano Second. Absolute nano second timer count. */
		gsw_r32(cdev, (TIMER_NS_LSB_NSLSB_OFFSET),
			TIMER_NS_LSB_NSLSB_SHIFT,
			TIMER_NS_LSB_NSLSB_SIZE, &value);
		parm->nNanoSec = value & 0xFFFF;
		gsw_r32(cdev, (TIMER_NS_MSB_NSMSB_OFFSET),
			TIMER_NS_MSB_NSMSB_SHIFT,
			TIMER_NS_MSB_NSMSB_SIZE, &value);
		parm->nNanoSec |= (value & 0xFFFF << 16);
	/** Fractional Nano Second. Absolute fractional */
	/* nano second timer count. */
/*	This counter specifis a 2^32 fractional 'nNanoSec'. */
		gsw_r32(cdev, (TIMER_FS_LSB_FSLSB_OFFSET),
			TIMER_FS_LSB_FSLSB_SHIFT,
			TIMER_FS_LSB_FSLSB_SIZE, &value);
		parm->nFractionalNanoSec = value & 0xFFFF;
		gsw_r32(cdev, (TIMER_FS_MSB_FSMSB_OFFSET),
			TIMER_FS_MSB_FSMSB_SHIFT,
			TIMER_FS_MSB_FSMSB_SIZE, &value);
		parm->nFractionalNanoSec |= (value & 0xFFFF << 16);
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}
GSW_return_t GSW_TimestampPortRead(void *cdev,
	GSW_TIMESTAMP_PortRead_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 tstamp0, tstamp1;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= ((gswdev->tpnum - 1)))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns */
	/* with an error for older hardware revisions. */
	if (gswdev->gipver != LTQ_GSWIP_2_0) {
		/** Second. Absolute second timer count. */
		gsw_r32(cdev, (FDMA_TSTAMP0_TSTL_OFFSET +
			(parm->nPortId * 0x6)),
			FDMA_TSTAMP0_TSTL_SHIFT,
			FDMA_TSTAMP0_TSTL_SIZE, &tstamp0);
		gsw_r32(cdev, (FDMA_TSTAMP1_TSTH_OFFSET +
			(parm->nPortId * 0x6)),
			FDMA_TSTAMP1_TSTH_SHIFT,
			FDMA_TSTAMP1_TSTH_SIZE, &tstamp1);
		parm->nEgressSec = ((tstamp0 | (tstamp1 << 16))) >> 30;
		parm->nEgressNanoSec = (((tstamp0 | (tstamp1 << 16)))
			& 0x7FFFFFFF);
		/** Nano Second. Absolute nano second timer count. */
		gsw_r32(cdev, (SDMA_TSTAMP0_TSTL_OFFSET +
			(parm->nPortId * 0x6)),
			SDMA_TSTAMP0_TSTL_SHIFT,
			SDMA_TSTAMP0_TSTL_SIZE, &tstamp0);
		gsw_r32(cdev, (SDMA_TSTAMP1_TSTH_OFFSET +
			(parm->nPortId * 0x6)),
			SDMA_TSTAMP1_TSTH_SHIFT,
			SDMA_TSTAMP1_TSTH_SIZE, &tstamp1);
		parm->nIngressSec = ((tstamp0 | (tstamp1 << 16))) >> 30;
		parm->nIngressNanoSec = (((tstamp0 | (tstamp1 << 16)))
			& 0x7FFFFFFF);
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

#endif /* CONFIG_LTQ_STP */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
GSW_return_t GSW_VLAN_Member_Init(void *cdev,
	GSW_VLAN_memberInit_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		pctbl_prog_t pcetable;
		u16 pcindex;
		for (pcindex = 0; pcindex < 4096; pcindex++) {
			memset(&pcetable, 0, sizeof(pctbl_prog_t));
			pcetable.pcindex = pcindex;
			pcetable.table = PCE_VLANMAP_INDEX;
			gsw_pce_table_read(cdev, &pcetable);
			pcetable.pcindex = pcindex;
			pcetable.table = PCE_VLANMAP_INDEX;
			pcetable.val[1] = (parm->nPortMemberMap & 0xFFFF);
			pcetable.val[2] = (parm->nEgressTagMap & 0xFFFF);
			gsw_pce_table_write(cdev, &pcetable);
		}
		return GSW_statusOk;
	} else {
		return GSW_statusNoSupport;
	}
}

GSW_return_t GSW_VLAN_IdCreate(void *cdev,
	GSW_VLAN_IdCreate_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = parm->nVId;
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.val[0] = (parm->nFId & 0x3F);
		gsw_pce_table_write(cdev, &pcetable);
	} else {
		u8 pcindex;
		avlan_tbl_t avlantbl;
		if (find_active_vlan_index(gswdev, parm->nVId) != 0xFF) {
			pr_err("This vid exists\n");
			return GSW_statusErr;
		}
		pcindex = fempty_avlan_index_table(gswdev);
		if (pcindex == 0xFF) {
			pr_err("There is no table entry avariable\n");
			return GSW_statusErr;
		}
		memset(&avlantbl, 0, sizeof(avlan_tbl_t));
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		avlantbl.valid = 1;
		avlantbl.vid = parm->nVId;
		avlantbl.fid = parm->nFId;
		if (pcindex >= 64)
			return GSW_statusValueRange;
		vlan_entry_set(gswdev, pcindex, &avlantbl);
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = pcindex;
		pcetable.table = PCE_ACTVLAN_INDEX;
		pcetable.key[0] = parm->nVId;
		pcetable.val[0] = parm->nFId;
		pcetable.valid = 1;
		gsw_pce_table_write(cdev, &pcetable);
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.val[0] = parm->nVId;
		pcetable.val[1] = 0;
		pcetable.val[2] = 0;
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_IdDelete(void *cdev,
	GSW_VLAN_IdDelete_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = parm->nVId;
		pcetable.table = PCE_VLANMAP_INDEX;
		gsw_pce_table_write(cdev, &pcetable);
	} else {
		u8 pcindex;
		avlan_tbl_t avlantbl;
		ltq_pce_table_t *pcvtbl = &gswdev->phandler;
		memset(&avlantbl, 0, sizeof(avlan_tbl_t));
		pcindex = find_active_vlan_index(gswdev, parm->nVId);
		if (pcindex == 0xFF) {
			pr_err("(VID not exists) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (gavlan_tbl_index(&pcvtbl->pce_sub_tbl,
			pcindex) != GSW_statusOk) {
			pr_err("(VID: 0x%0x used by flow table) %s:%s:%d\n",
			parm->nVId, __FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		vlan_entry_set(gswdev, pcindex, &avlantbl);
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = pcindex;
		pcetable.table = PCE_ACTVLAN_INDEX;
		pcetable.valid = 0;
		gsw_pce_table_write(cdev, &pcetable);
		pcetable.table = PCE_VLANMAP_INDEX;
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_IdGet(void *cdev, GSW_VLAN_IdGet_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = parm->nVId;
		pcetable.table = PCE_VLANMAP_INDEX;
		gsw_pce_table_read(cdev, &pcetable);
		parm->nFId = pcetable.val[0] & 0x3F;
	} else {
		u8 pcindex;
		avlan_tbl_t avlantbl;
		pcindex = find_active_vlan_index(gswdev, parm->nVId);
		if (pcindex == 0xFF) {
			pr_err("(VID not exists) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		get_vlan_sw_table(gswdev, pcindex, &avlantbl);
		parm->nFId = avlantbl.fid;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_PortCfgGet(void *cdev,
	GSW_VLAN_portCfg_t *parm)
{
	u32 value;
	int pcindex;
	ltq_bool_t uvr, vimr, vemr;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	avlan_tbl_t avlantbl;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	gsw_r32(cdev, (PCE_DEFPVID_PVID_OFFSET + (10 * parm->nPortId)),
		PCE_DEFPVID_PVID_SHIFT, PCE_DEFPVID_PVID_SIZE, &value);
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		parm->nPortVId = value;
	} else {
		pcindex = value;
		if (pcindex >= 64)
			return GSW_statusValueRange;
		get_vlan_sw_table(gswdev, pcindex, &avlantbl);
		parm->nPortVId = avlantbl.vid;
	}
	gsw_r32(cdev, (PCE_VCTRL_UVR_OFFSET + (10 * parm->nPortId)),
		PCE_VCTRL_UVR_SHIFT, PCE_VCTRL_UVR_SIZE, &value);
	uvr = value;
	if (uvr == 1)
		parm->bVLAN_UnknownDrop = 1;
	else
		parm->bVLAN_UnknownDrop = 0;
	gsw_r32(cdev, (PCE_VCTRL_VSR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_VSR_SHIFT,
		PCE_VCTRL_VSR_SIZE, &value);
	parm->bVLAN_ReAssign = value;
	gsw_r32(cdev, (PCE_VCTRL_VIMR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_VIMR_SHIFT,
		PCE_VCTRL_VIMR_SIZE, &value);
	vimr = value;

	gsw_r32(cdev, (PCE_VCTRL_VEMR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_VEMR_SHIFT,
		PCE_VCTRL_VEMR_SIZE, &value);
	vemr = value;
	if (vimr == 0 && vemr == 0)
		parm->eVLAN_MemberViolation = GSW_VLAN_MEMBER_VIOLATION_NO;
	else if (vimr == 1 && vemr == 0)
		parm->eVLAN_MemberViolation = GSW_VLAN_MEMBER_VIOLATION_INGRESS;
	else if (vimr == 0 && vemr == 1)
		parm->eVLAN_MemberViolation = GSW_VLAN_MEMBER_VIOLATION_EGRESS;
	else if (vimr == 1 && vemr == 1)
		parm->eVLAN_MemberViolation = GSW_VLAN_MEMBER_VIOLATION_BOTH;

	gsw_r32(cdev, (PCE_VCTRL_VINR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_VINR_SHIFT,
		PCE_VCTRL_VINR_SIZE, &value);
	switch (value) {
	case 0:
		parm->eAdmitMode = GSW_VLAN_ADMIT_ALL;
		break;
	case 1:
		parm->eAdmitMode = GSW_VLAN_ADMIT_TAGGED;
		break;
	case 2:
		parm->eAdmitMode = GSW_VLAN_ADMIT_UNTAGGED;
		break;
	default:
			break;
	}
	gsw_r32(cdev, (PCE_PCTRL_0_TVM_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_0_TVM_SHIFT,
		PCE_PCTRL_0_TVM_SIZE, &value);
	parm->bTVM = value;
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_PortCfgSet(void *cdev,
	GSW_VLAN_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	u8 pcindex;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		value = parm->nPortVId;
	} else {
		pcindex = find_active_vlan_index(gswdev, parm->nPortVId);
		if (pcindex == 0xFF) {
			pr_err("(VID not exists) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		value = pcindex;
	}
	gsw_w32(cdev, (PCE_DEFPVID_PVID_OFFSET +
		(10 * parm->nPortId)),
		PCE_DEFPVID_PVID_SHIFT,
		PCE_DEFPVID_PVID_SIZE, value);
	value = 0;
	if (parm->bVLAN_UnknownDrop == 1)
		value = 1;

	gsw_w32(cdev, (PCE_VCTRL_UVR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_UVR_SHIFT,
		PCE_VCTRL_UVR_SIZE, value);
	value = parm->bVLAN_ReAssign;
	gsw_w32(cdev, (PCE_VCTRL_VSR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_VSR_SHIFT,
		PCE_VCTRL_VSR_SIZE, value);
	switch (parm->eVLAN_MemberViolation) {
	case GSW_VLAN_MEMBER_VIOLATION_NO:
		gsw_w32(cdev, (PCE_VCTRL_VIMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VIMR_SHIFT,
			PCE_VCTRL_VIMR_SIZE, 0);
		gsw_w32(cdev, (PCE_VCTRL_VEMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VEMR_SHIFT,
			PCE_VCTRL_VEMR_SIZE, 0);
		break;
	case GSW_VLAN_MEMBER_VIOLATION_INGRESS:
		gsw_w32(cdev, (PCE_VCTRL_VIMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VIMR_SHIFT,
			PCE_VCTRL_VIMR_SIZE, 1);
		gsw_w32(cdev, (PCE_VCTRL_VEMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VEMR_SHIFT,
			PCE_VCTRL_VEMR_SIZE, 0);
		break;
	case GSW_VLAN_MEMBER_VIOLATION_EGRESS:
		gsw_w32(cdev, (PCE_VCTRL_VIMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VIMR_SHIFT,
			PCE_VCTRL_VIMR_SIZE, 0);
		gsw_w32(cdev, (PCE_VCTRL_VEMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VEMR_SHIFT,
			PCE_VCTRL_VEMR_SIZE, 1);
		break;
	case GSW_VLAN_MEMBER_VIOLATION_BOTH:
		gsw_w32(cdev, (PCE_VCTRL_VIMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VIMR_SHIFT,
			PCE_VCTRL_VIMR_SIZE, 1);
		gsw_w32(cdev, (PCE_VCTRL_VEMR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_VEMR_SHIFT,
			PCE_VCTRL_VEMR_SIZE, 1);
		break;
	default:
		pr_err("WARNING:(eVLAN_MemberViolation) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
	}
	switch (parm->eAdmitMode) {
	case GSW_VLAN_ADMIT_ALL:
		value = 0;
		break;
	case GSW_VLAN_ADMIT_TAGGED:
		value = 1;
		break;
	case GSW_VLAN_ADMIT_UNTAGGED:
		value = 2;
		break;
	default:
		value = 0;
		pr_err("%s:%s:%d (eAdmitMode)\n",
			__FILE__, __func__, __LINE__);
	}
	gsw_w32(cdev, (PCE_VCTRL_VINR_OFFSET +
		(10 * parm->nPortId)),
		PCE_VCTRL_VINR_SHIFT,
		PCE_VCTRL_VINR_SIZE, value);
	value = 0;
	if (parm->bTVM == 1)
		value = 1;

	gsw_w32(cdev, (PCE_PCTRL_0_TVM_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_0_TVM_SHIFT,
		PCE_PCTRL_0_TVM_SIZE, value);
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_PortMemberAdd(void *cdev,
	GSW_VLAN_portMemberAdd_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->nPortId >= gswdev->tpnum) &&
		(!(parm->nPortId & 0x80000000)))
		return GSW_statusErr;
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		u16  portmap, tagmap, val0;
		if (parm->nVId > 4096) {
			pr_err("ERROR: %s:%s:%d, (VID:%d)\n",
			__FILE__, __func__, __LINE__, parm->nVId);
			return GSW_statusErr;
		}
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = parm->nVId;
		gsw_pce_table_read(cdev, &pcetable);
		portmap	= (pcetable.val[1]);
		tagmap = (pcetable.val[2]);
		val0 = (pcetable.val[0]);
	/*  Support  portmap information. */
	/*  To differentiate between port index and portmap, */
	/* the MSB (highest data bit) should be 1.*/
		if (parm->nPortId & 0x80000000) { /*Port Map */
			portmap |= ((parm->nPortId) & 0xFFFF);
			if (parm->bVLAN_TagEgress)
				tagmap |= ((parm->nPortId) & 0xFFFF);
			else
				tagmap &= ~((parm->nPortId) & 0xFFFF);
		} else {
			portmap |= 1 << parm->nPortId;
			if (parm->bVLAN_TagEgress)
				tagmap |= 1 << parm->nPortId;
			else
				tagmap &= ~(1 << parm->nPortId);
		}
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = parm->nVId;
		pcetable.val[0] = val0;
		pcetable.val[1] = portmap;
		pcetable.val[2] = tagmap;
		gsw_pce_table_write(cdev, &pcetable);
	} else {
		u8 pcindex;
		avlan_tbl_t avlantbl;
		pcindex = find_active_vlan_index(gswdev,
			parm->nVId);
		if (pcindex == 0xFF) {
			pr_err("(VID not exists) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		get_vlan_sw_table(gswdev, pcindex, &avlantbl);
		if (avlantbl.reserved == 1) {
			pr_err("(VID was already reserved) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (parm->nPortId & 0x80000000) { /*Port Map */
			avlantbl.pm |= ((parm->nPortId) & 0x7FFF);
			if (parm->bVLAN_TagEgress)
				avlantbl.tm |= ((parm->nPortId) & 0x7FFF);
			else
				avlantbl.tm &= ~((parm->nPortId) & 0x7FFF);
		} else {
			avlantbl.pm |= 1 << parm->nPortId;
			if (parm->bVLAN_TagEgress)
				avlantbl.tm |= 1 << parm->nPortId;
			else
				avlantbl.tm &= ~(1 << parm->nPortId);
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		vlan_entry_set(gswdev, pcindex, &avlantbl);
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = pcindex;
		gsw_pce_table_read(cdev, &pcetable);
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = pcindex;
		pcetable.val[1] = avlantbl.pm;
		pcetable.val[2] = avlantbl.tm;
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_PortMemberRead(void *cdev,
	GSW_VLAN_portMemberRead_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->bInitial == 1) {
		/*Start from the index 0 */
		gswdev->vlan_rd_index = 0;
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = gswdev->vlan_rd_index;
		gsw_pce_table_read(cdev, &pcetable);
		parm->nVId = gswdev->vlan_rd_index;
		/* Port Map */
		parm->nPortId = (pcetable.val[1] | 0x80000000);
		parm->nTagId = (pcetable.val[2] | 0x80000000);
		parm->bInitial = 0;
		parm->bLast = 0;
	}
	if (parm->bLast != 1) {
		if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
			(gswdev->gipver == LTQ_GSWIP_3_0)) {
			if (gswdev->vlan_rd_index < 4096) {
				gswdev->vlan_rd_index++;
				pcetable.table = PCE_VLANMAP_INDEX;
				pcetable.pcindex = gswdev->vlan_rd_index;
				gsw_pce_table_read(cdev, &pcetable);
				parm->nVId = gswdev->vlan_rd_index;
				/* Port Map */
				parm->nPortId = (pcetable.val[1] | 0x80000000);
				parm->nTagId = (pcetable.val[2] | 0x80000000);
			} else {
				parm->bLast = 1;
				gswdev->vlan_rd_index = 0;
			}
		} else {
			if (gswdev->vlan_rd_index < gswdev->avlantsz) {
				gswdev->vlan_rd_index++;
				pcetable.table = PCE_VLANMAP_INDEX;
				pcetable.pcindex = gswdev->vlan_rd_index;
				gsw_pce_table_read(cdev, &pcetable);
				parm->nVId = (pcetable.val[0] & 0xFFF);
				/* Port Map */
				parm->nPortId = (pcetable.val[1] | 0x80000000);
				parm->nTagId = (pcetable.val[2] | 0x80000000);
			} else {
				parm->bLast = 1;
				gswdev->vlan_rd_index = 0;
			}
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_PortMemberRemove(void *cdev,
	GSW_VLAN_portMemberRemove_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->nPortId >= gswdev->tpnum) &&
		(!(parm->nPortId & 0x80000000)))
		return GSW_statusErr;

	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		u16  portmap, tagmap , val0;
		if (parm->nVId > 4096) {
			pr_err("ERROR: %s:%s:%d, (VID:%d)\n",
			__FILE__, __func__, __LINE__, parm->nVId);
			return GSW_statusErr;
		}
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = parm->nVId;
		gsw_pce_table_read(cdev, &pcetable);
		portmap	= (pcetable.val[1]);
		tagmap = (pcetable.val[2]);
		val0 = (pcetable.val[0]);
		if (parm->nPortId & 0x80000000)
			portmap  &= ~((parm->nPortId) & 0x7FFF);
		else
			portmap &= ~(1 << parm->nPortId);

		tagmap &= ~(1 << parm->nPortId);
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = parm->nVId;
		pcetable.val[0] = val0;
		pcetable.val[1] = portmap;
		pcetable.val[2] = tagmap;
		gsw_pce_table_write(cdev, &pcetable);
	} else {
		avlan_tbl_t avlantbl;
		u8 pcindex;
		pcindex = find_active_vlan_index(gswdev,
			parm->nVId);
		if (pcindex == 0xFF) {
			pr_err("This vid doesn't exists\n");
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		get_vlan_sw_table(gswdev, pcindex, &avlantbl);
		if (parm->nPortId & 0x80000000)
			avlantbl.pm  &= ~((parm->nPortId) & 0x7FFF);
		else
			avlantbl.pm &= ~(1 << parm->nPortId);

		avlantbl.tm &= ~(1 << parm->nPortId);
		if (pcindex >= 64)
			return GSW_statusValueRange;
		vlan_entry_set(gswdev, pcindex, &avlantbl);
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = pcindex;
		gsw_pce_table_read(cdev, &pcetable);
		if (parm->nPortId & 0x80000000) {
			pcetable.val[1] &= ~((parm->nPortId) & 0x7FFF);
			pcetable.val[2] &= ~((parm->nPortId) & 0x7FFF);
		} else {
			pcetable.val[1] &= ~(1 << parm->nPortId);
			pcetable.val[2] &= ~(1 << parm->nPortId);
		}
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_ReservedAdd(void *cdev,
	GSW_VLAN_reserved_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		if (parm->nVId > 4096) {
			pr_err("ERROR: %s:%s:%d,(VID:%d)\n",
			__FILE__, __func__, __LINE__, parm->nVId);
			return GSW_statusErr;
		}
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = parm->nVId;
		gsw_pce_table_read(cdev, &pcetable);
		pcetable.val[0] |= (1 << 8);
		gsw_pce_table_write(cdev, &pcetable);
	} else {
		u8 pcindex;
		avlan_tbl_t avlantbl;
		pcindex = find_active_vlan_index(gswdev,
			parm->nVId);
		if (pcindex == 0xFF) {
			pr_err("(VID not exist, create VID) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		get_vlan_sw_table(gswdev, pcindex, &avlantbl);
		if (avlantbl.pm != 0) {
			pr_err("(Added to member & can't be reserve %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (avlantbl.tm != 0) {
			pr_err("(Added to member & can't be reserve %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		avlantbl.reserved = 1;
		if (pcindex >= 64)
			return GSW_statusValueRange;
		vlan_entry_set(gswdev, pcindex, &avlantbl);
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = pcindex;
		pcetable.table = PCE_ACTVLAN_INDEX;
		pcetable.val[0] |= (1 << 8);
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_VLAN_ReservedRemove(void *cdev,
	GSW_VLAN_reserved_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		if (parm->nVId > 4096) {
			pr_err("ERROR: %s:%s:%d,(VID:%d)\n",
			__FILE__, __func__, __LINE__, parm->nVId);
			return GSW_statusErr;
		}
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.pcindex = parm->nVId;
		gsw_pce_table_read(cdev, &pcetable);
		pcetable.val[0] &= ~(1 << 8);
		gsw_pce_table_write(cdev, &pcetable);
	} else {
		u8 pcindex;
		avlan_tbl_t avlantbl;
		pcindex = find_active_vlan_index(gswdev, parm->nVId);
		if (pcindex == 0xFF) {
			pr_err("This vid doesn't exists, create VID first\n");
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		get_vlan_sw_table(gswdev, pcindex, &avlantbl);
		if (avlantbl.pm != 0) {
			pr_err("(Added to member & can't be remove the reserve %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (avlantbl.tm != 0) {
			pr_err("(Added to member & can't be remove the reserve %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}
		if (pcindex >= 64)
			return GSW_statusValueRange;
		if (avlantbl.reserved == 0) {
			pr_err("This VID was not reserve, reserve it first\n");
			return GSW_statusErr;
		} else {
			avlantbl.reserved = 0;
			vlan_entry_set(gswdev, pcindex, &avlantbl);
		}
		vlan_entry_set(gswdev, pcindex, &avlantbl);
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = pcindex;
		pcetable.table = PCE_ACTVLAN_INDEX;
		pcetable.val[0] &= ~(1 << 8);
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PCE_EG_VLAN_CfgSet(void *cdev,
	GSW_PCE_EgVLAN_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (parm->eEgVLANmode ==
			GSW_PCE_EG_VLAN_SUBIFID_BASED) {
			gsw_w32(cdev, (PCE_EVLANCFG_EGVMD_OFFSET
				+ (0x10 * parm->nPortId)),
				PCE_EVLANCFG_EGVMD_SHIFT,
				PCE_EVLANCFG_EGVMD_SIZE, 1);
		} else {
			gsw_w32(cdev, (PCE_EVLANCFG_EGVMD_OFFSET
				+ (0x10 * parm->nPortId)),
				PCE_EVLANCFG_EGVMD_SHIFT,
				PCE_EVLANCFG_EGVMD_SIZE, 0);
		}
		gsw_w32(cdev, (PCE_EVLANCFG_EGVFST_OFFSET
			+ (0x10 * parm->nPortId)),
			PCE_EVLANCFG_EGVFST_SHIFT,
			PCE_EVLANCFG_EGVFST_SIZE,
			parm->nEgStartVLANIdx);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PCE_EG_VLAN_CfgGet(void *cdev,
	GSW_PCE_EgVLAN_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	if (parm->nPortId >= gswdev->tpnum) */
/*		return GSW_statusErr; */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, (PCE_EVLANCFG_EGVMD_OFFSET
			+ (0x10 * parm->nPortId)),
			PCE_EVLANCFG_EGVMD_SHIFT,
			PCE_EVLANCFG_EGVMD_SIZE, &parm->eEgVLANmode);
		gsw_r32(cdev, (PCE_EVLANCFG_EGVFST_OFFSET
			+ (0x10 * parm->nPortId)),
			PCE_EVLANCFG_EGVFST_SHIFT,
			PCE_EVLANCFG_EGVFST_SIZE, &value);
		parm->nEgStartVLANIdx = value;
	}

	return GSW_statusOk;
}

GSW_return_t GSW_PCE_EG_VLAN_EntryWrite(void *cdev,
	GSW_PCE_EgVLAN_Entry_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	if (parm->nPortId >= gswdev->tpnum) */
/*		return GSW_statusErr;*/
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = (parm->nIndex & 0xFF);
		if (parm->bEgVLAN_Action == 1)
			pcetable.val[0] |= (1 << 0);

		if (parm->bEgSVidRem_Action == 1)
			pcetable.val[0] |= (1 << 2);

		if (parm->bEgSVidIns_Action == 1)
			pcetable.val[0] |= (1 << 3);

		pcetable.val[0] |= ((parm->nEgSVid & 0xFFF) << 4);
		if (parm->bEgCVidRem_Action == 1)
			pcetable.val[1] |= (1 << 2);

		if (parm->bEgCVidIns_Action == 1)
			pcetable.val[1] |= (1 << 3);

		pcetable.val[1] |= ((parm->nEgCVid & 0xFFF) << 4);
		pcetable.table = PCE_EG_VLAN_INDEX;
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PCE_EG_VLAN_EntryRead(void *cdev,
	GSW_PCE_EgVLAN_Entry_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/*if (parm->nPortId >= gswdev->tpnum) */
	/*	return GSW_statusErr; */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.pcindex = (parm->nIndex & 0xFF);
		pcetable.table = PCE_EG_VLAN_INDEX;
		gsw_pce_table_read(cdev, &pcetable);
		parm->bEgVLAN_Action = pcetable.val[0] & 0x1;
		parm->bEgSVidRem_Action = (pcetable.val[0] >> 2) & 0x1;
		parm->bEgSVidIns_Action = (pcetable.val[0] >> 3) & 0x1;
		parm->nEgSVid = (pcetable.val[0] >> 4) & 0xFFF;
		parm->bEgCVidRem_Action = (pcetable.val[1] >> 2) & 0x1;
		parm->bEgCVidIns_Action = (pcetable.val[1] >> 3) & 0x1;
		parm->nEgCVid = (pcetable.val[1] >> 4) & 0xFFF;
	}
	gsw_w32(cdev, PCE_TBL_CTRL_ADDR_OFFSET, 0, 16, 0);

	return GSW_statusOk;
}

GSW_return_t GSW_SVLAN_CfgGet(void *cdev,
	GSW_SVLAN_cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 reg_val;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC)
		|| (gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, (FDMA_SVTETYPE_OFFSET),
			FDMA_SVTETYPE_ETYPE_SHIFT,
			FDMA_SVTETYPE_ETYPE_SIZE, &reg_val);
		parm->nEthertype = reg_val;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_SVLAN_CfgSet(void *cdev,
	GSW_SVLAN_cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 reg_val;
	reg_val = parm->nEthertype & 0xFFFF;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
	(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_w32(cdev, (FDMA_SVTETYPE_OFFSET),
			FDMA_SVTETYPE_ETYPE_SHIFT,
			FDMA_SVTETYPE_ETYPE_SIZE, reg_val);
		gsw_w32(cdev, (MAC_VLAN_ETYPE_1_INNER_OFFSET),
			MAC_VLAN_ETYPE_1_INNER_SHIFT,
			MAC_VLAN_ETYPE_1_INNER_SIZE, reg_val);
	/* ToDo: Update the Micro code based on SVAN*/
	}
	return GSW_statusOk;
}

GSW_return_t GSW_SVLAN_PortCfgGet(void *cdev,
	GSW_SVLAN_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	ltq_bool_t svimr, svemr;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
/** nPortVId: retrieve the corresponding VLAN ID */
/* from the Active VLAN Table*/
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
	(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, (PCE_DEFPSVID_PVID_OFFSET +
			(2 * parm->nPortId)),
			PCE_DEFPSVID_PVID_SHIFT,
			PCE_DEFPSVID_PVID_SIZE, &value);
		parm->nPortVId = value;
    /* bSVLAN_TagSupport */
		gsw_r32(cdev, (PCE_VCTRL_STEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_STEN_SHIFT,
			PCE_VCTRL_STEN_SIZE, &value);
		parm->bSVLAN_TagSupport = value;

		/** bVLAN_ReAssign */
		gsw_r32(cdev, (PCE_VCTRL_SVSR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_SVSR_SHIFT,
			PCE_VCTRL_SVSR_SIZE, &value);
		parm->bVLAN_ReAssign = value;

    /** bVlanMemberViolationIngress */
		gsw_r32(cdev, (PCE_VCTRL_SVIMR_OFFSET
			+ (10 * parm->nPortId)),
			PCE_VCTRL_SVIMR_SHIFT,
			PCE_VCTRL_SVIMR_SIZE, &value);
		svimr = value;
		/** bVlanMemberViolationEgress */
		gsw_r32(cdev, (PCE_VCTRL_SVEMR_OFFSET
			+ (10 * parm->nPortId)),
			PCE_VCTRL_SVEMR_SHIFT,
			PCE_VCTRL_SVEMR_SIZE, &value);
		svemr = value;
		if (svimr == 0 && svemr == 0)
			parm->eVLAN_MemberViolation =
			GSW_VLAN_MEMBER_VIOLATION_NO;
		if (svimr == 1 && svemr == 0)
			parm->eVLAN_MemberViolation =
			GSW_VLAN_MEMBER_VIOLATION_INGRESS;
		if (svimr == 0 && svemr == 1)
			parm->eVLAN_MemberViolation =
			GSW_VLAN_MEMBER_VIOLATION_EGRESS;
		if (svimr == 1 && svemr == 1)
			parm->eVLAN_MemberViolation =
			GSW_VLAN_MEMBER_VIOLATION_BOTH;
		/* eAdmitMode:  */
		gsw_r32(cdev, (PCE_VCTRL_SVINR_OFFSET
			+ (10 * parm->nPortId)),
			PCE_VCTRL_SVINR_SHIFT,
			PCE_VCTRL_SVINR_SIZE, &value);
		switch (value) {
		case 0:
			parm->eAdmitMode = GSW_VLAN_ADMIT_ALL;
			break;
		case 1:
			parm->eAdmitMode = GSW_VLAN_ADMIT_TAGGED;
			break;
		case 2:
			parm->eAdmitMode = GSW_VLAN_ADMIT_UNTAGGED;
			break;
		default:
				break;
		} /* -----  end switch  ----- */
		/** bSVLAN_MACbasedTag */
		gsw_r32(cdev, (PCE_VCTRL_MACEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_MACEN_SHIFT,
			PCE_VCTRL_MACEN_SIZE, &value);
		parm->bSVLAN_MACbasedTag = value;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_SVLAN_PortCfgSet(void *cdev,
	GSW_SVLAN_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;

	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		value = parm->nPortVId;
		gsw_w32(cdev, (PCE_DEFPSVID_PVID_OFFSET +
			(2 * parm->nPortId)),
			PCE_DEFPSVID_PVID_SHIFT,
			PCE_DEFPSVID_PVID_SIZE, value);
		/* bSVLAN_TagSupport */
		value = parm->bSVLAN_TagSupport;
		gsw_w32(cdev, (PCE_VCTRL_STEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_STEN_SHIFT,
			PCE_VCTRL_STEN_SIZE, value);
		if (parm->bSVLAN_TagSupport == 1) {
				gsw_w32(cdev, (FDMA_PCTRL_SVLANMOD_OFFSET +
					(6 * parm->nPortId)),
					FDMA_PCTRL_SVLANMOD_SHIFT,
					FDMA_PCTRL_SVLANMOD_SIZE, 3);
			} else {
				gsw_w32(cdev, (FDMA_PCTRL_SVLANMOD_OFFSET +
					(6 * parm->nPortId)),
					FDMA_PCTRL_SVLANMOD_SHIFT,
					FDMA_PCTRL_SVLANMOD_SIZE, 0);
			}
		/** bVLAN_ReAssign */
		value = parm->bVLAN_ReAssign;
		gsw_w32(cdev, (PCE_VCTRL_SVSR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_SVSR_SHIFT,
			PCE_VCTRL_SVSR_SIZE, value);
		/** eVLAN_MemberViolation  */
		switch (parm->eVLAN_MemberViolation) {
		case GSW_VLAN_MEMBER_VIOLATION_NO:
			gsw_w32(cdev, (PCE_VCTRL_SVIMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVIMR_SHIFT,
				PCE_VCTRL_SVIMR_SIZE, 0);
			gsw_w32(cdev, (PCE_VCTRL_SVEMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVEMR_SHIFT,
				PCE_VCTRL_SVEMR_SIZE, 0);
			break;
		case GSW_VLAN_MEMBER_VIOLATION_INGRESS:
			gsw_w32(cdev, (PCE_VCTRL_SVIMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVIMR_SHIFT,
				PCE_VCTRL_SVIMR_SIZE, 1);
			gsw_w32(cdev, (PCE_VCTRL_SVEMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVEMR_SHIFT,
				PCE_VCTRL_SVEMR_SIZE, 0);
			break;
		case GSW_VLAN_MEMBER_VIOLATION_EGRESS:
			gsw_w32(cdev, (PCE_VCTRL_SVIMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVIMR_SHIFT,
				PCE_VCTRL_SVIMR_SIZE, 0);
			gsw_w32(cdev, (PCE_VCTRL_SVEMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVEMR_SHIFT,
				PCE_VCTRL_SVEMR_SIZE, 1);
			break;
		case GSW_VLAN_MEMBER_VIOLATION_BOTH:
			gsw_w32(cdev, (PCE_VCTRL_SVIMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVIMR_SHIFT,
				PCE_VCTRL_SVIMR_SIZE, 1);
			gsw_w32(cdev, (PCE_VCTRL_SVEMR_OFFSET +
				(10 * parm->nPortId)),
				PCE_VCTRL_SVEMR_SHIFT,
				PCE_VCTRL_SVEMR_SIZE, 1);
			break;
		} /* -----  end switch  ----- */
		/** eAdmitMode */
		switch (parm->eAdmitMode) {
		case GSW_VLAN_ADMIT_ALL:
			value = 0;
			break;
		case GSW_VLAN_ADMIT_TAGGED:
			value = 1;
			break;
		case GSW_VLAN_ADMIT_UNTAGGED:
			value = 2;
			break;
		default:
			value = 0;
		} /* -----  end switch  ----- */
		gsw_w32(cdev, (PCE_VCTRL_SVINR_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_SVINR_SHIFT,
			PCE_VCTRL_SVINR_SIZE, value);
		/** bSVLAN_MACbasedTag */
		value = parm->bSVLAN_MACbasedTag;
		gsw_w32(cdev, (PCE_VCTRL_MACEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_VCTRL_MACEN_SHIFT,
			PCE_VCTRL_MACEN_SIZE, value);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_SVLAN_ClassPCP_PortGet(void *cdev,
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value, dei;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		for (value = 0; value < 16; value++) {
			memset(&pcetable, 0, sizeof(pctbl_prog_t));
			pcetable.pcindex = (((parm->nPortId & 0xF) << 4)
				| (value));
			pcetable.table = PCE_EGREMARK_INDEX;
			gsw_pce_table_read(cdev, &pcetable);
			parm->nDSCP[value] = (pcetable.val[0] & 0x3F);
			parm->nCPCP[value] = ((pcetable.val[0] >> 8) & 0x7);
			parm->nSPCP[value] = ((pcetable.val[1] >> 8) & 0x7);
			dei = ((pcetable.val[1]) & 0x1);
			parm->nSPCP[value]	|= (dei << 7);
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_SVLAN_ClassPCP_PortSet(void *cdev,
	GSW_QoS_SVLAN_ClassPCP_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	u8 cpcp, dscp, spcp, dei;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;

	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		for (value = 0; value < 16; value++) {
			memset(&pcetable, 0, sizeof(pctbl_prog_t));
			pcetable.pcindex = (((parm->nPortId & 0xF) << 4)
				| (value));
			dscp = parm->nDSCP[value] & 0x3F;
			spcp = parm->nSPCP[value] & 0x7;
			cpcp = parm->nCPCP[value] & 0x7;
			dei = ((parm->nSPCP[value] >> 7) & 1);
			pcetable.val[1] = ((spcp << 8) | dei);
			pcetable.val[0] = (dscp | (cpcp << 8));
			pcetable.table = PCE_EGREMARK_INDEX;
			gsw_pce_table_write(cdev, &pcetable);
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_SVLAN_PCP_ClassGet(void *cdev,
	GSW_QoS_SVLAN_PCP_ClassCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t ptbl;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		memset(&ptbl, 0, sizeof(pctbl_prog_t));
		for (value = 0; value < 16; value++) {
			ptbl.table = PCE_SPCP_INDEX;
			ptbl.pcindex = value;
			gsw_pce_table_read(cdev, &ptbl);
			parm->nTrafficClass[value] = ptbl.val[0] & 0xF;
			parm->nTrafficColor[value] = ((ptbl.val[0] >> 6) & 0x3);
			parm->nPCP_Remark_Enable[value] =
				((ptbl.val[0] >> 4) & 0x1);
			parm->nDEI_Remark_Enable[value] =
				((ptbl.val[0] >> 5) & 0x1);
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_SVLAN_PCP_ClassSet(void *cdev,
	GSW_QoS_SVLAN_PCP_ClassCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		pctbl_prog_t pcetable;
		u32  value;
		for (value = 0; value < 16; value++) {
			memset(&pcetable, 0, sizeof(pctbl_prog_t));
			pcetable.table = PCE_SPCP_INDEX;
			pcetable.pcindex = value;
			pcetable.val[0] = parm->nTrafficClass[value] & 0xF;
			pcetable.val[0] |=
				(parm->nTrafficColor[value] & 0x3) << 6;
			pcetable.val[0] |=
				(parm->nPCP_Remark_Enable[value] & 0x1) << 4;
			pcetable.val[0] |=
				(parm->nDEI_Remark_Enable[value] & 0x1) << 5;
			gsw_pce_table_write(cdev, &pcetable);
		}
	}
	return GSW_statusOk;
}
#endif /*CONFIG_LTQ_VLAN */
#if defined(CONFIG_LTQ_QOS) && CONFIG_LTQ_QOS
GSW_return_t GSW_QoS_MeterCfgGet(void *cdev,
	GSW_QoS_meterCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 mid = parm->nMeterId, value, exp, mant, ibs;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (mid > gswdev->num_of_meters)
			return GSW_statusErr;
		gsw_w32(cdev, GSW_INST_SEL_INST_OFFSET,
			GSW_INST_SEL_INST_SHIFT,
			GSW_INST_SEL_INST_SIZE, mid);
		/* Enable/Disable the meter shaper */
		gsw_r32(cdev, GSW_PCE_TCM_CTRL_TCMEN_OFFSET,
			GSW_PCE_TCM_CTRL_TCMEN_SHIFT,
			GSW_PCE_TCM_CTRL_TCMEN_SIZE, &value);
		parm->bEnable = value;
		/* Committed Burst Size */
		gsw_r32(cdev, GSW_PCE_TCM_CBS_CBS_OFFSET,
			GSW_PCE_TCM_CBS_CBS_SHIFT,
			GSW_PCE_TCM_CBS_CBS_SIZE, &value);
		parm->nCbs = (value * 64);
		/* Excess Burst Size (EBS [bytes]) */
		gsw_r32(cdev, GSW_PCE_TCM_EBS_EBS_OFFSET,
			GSW_PCE_TCM_EBS_EBS_SHIFT,
			GSW_PCE_TCM_EBS_EBS_SIZE, &value);
		parm->nEbs = (value * 64);
		/* Rate Counter Exponent */
		gsw_r32(cdev, GSW_PCE_TCM_CIR_EXP_EXP_OFFSET,
			GSW_PCE_TCM_CIR_EXP_EXP_SHIFT,
			GSW_PCE_TCM_CIR_EXP_EXP_SIZE, &exp);
		/* Rate Counter Mantissa */
		gsw_r32(cdev, GSW_PCE_TCM_CIR_MANT_MANT_OFFSET,
			GSW_PCE_TCM_CIR_MANT_MANT_SHIFT,
			GSW_PCE_TCM_CIR_MANT_MANT_SIZE, &mant);
	   /* Rate Counter iBS */
		gsw_r32(cdev, GSW_PCE_TCM_IBS_IBS_OFFSET,
			GSW_PCE_TCM_IBS_IBS_SHIFT,
			GSW_PCE_TCM_IBS_IBS_SIZE, &ibs);
		/* calc the Rate */
		parm->nRate = mratecalc(ibs, exp, mant);
		/* Rate Counter Exponent */
		gsw_r32(cdev, GSW_PCE_TCM_PIR_EXP_EXP_OFFSET,
			GSW_PCE_TCM_PIR_EXP_EXP_SHIFT,
			GSW_PCE_TCM_PIR_EXP_EXP_SIZE, &exp);
		/* Rate Counter Mantissa */
		gsw_r32(cdev, GSW_PCE_TCM_PIR_MANT_MANT_OFFSET,
			GSW_PCE_TCM_PIR_MANT_MANT_SHIFT,
			GSW_PCE_TCM_PIR_MANT_MANT_SIZE, &mant);
	   /* Rate Counter iBS */
		gsw_r32(cdev, GSW_PCE_TCM_IBS_IBS_OFFSET,
			GSW_PCE_TCM_IBS_IBS_SHIFT,
			GSW_PCE_TCM_IBS_IBS_SIZE, &ibs);
		/* calc the Rate */
		parm->nPiRate = mratecalc(ibs, exp, mant);
		/* parm->nPbs=??? how to calculate it*/
		gsw_r32(cdev, GSW_PCE_TCM_CTRL_TMOD_OFFSET,
			GSW_PCE_TCM_CTRL_TMOD_SHIFT,
			GSW_PCE_TCM_CTRL_TMOD_SIZE, &parm->eMtrType);
	} else {
		if (mid > 7)
			return GSW_statusErr;
		/* Enable/Disable the meter shaper */
		gsw_r32(cdev, (PCE_TCM_CTRL_TCMEN_OFFSET + (mid * 7)),
			PCE_TCM_CTRL_TCMEN_SHIFT,
			PCE_TCM_CTRL_TCMEN_SIZE, &value);
		parm->bEnable = value;
		/* Committed Burst Size */
		gsw_r32(cdev, (PCE_TCM_CBS_CBS_OFFSET + (mid * 7)),
			PCE_TCM_CBS_CBS_SHIFT,
			PCE_TCM_CBS_CBS_SIZE, &value);
		parm->nCbs = (value * 64);
		/* Excess Burst Size (EBS [bytes]) */
		gsw_r32(cdev, (PCE_TCM_EBS_EBS_OFFSET + (mid * 7)),
			PCE_TCM_EBS_EBS_SHIFT,
			PCE_TCM_EBS_EBS_SIZE, &value);
		parm->nEbs = (value * 64);
		/* Rate Counter Exponent */
		gsw_r32(cdev, (PCE_TCM_CIR_EXP_EXP_OFFSET + (mid * 7)),
			PCE_TCM_CIR_EXP_EXP_SHIFT,
			PCE_TCM_CIR_EXP_EXP_SIZE, &exp);
		/* Rate Counter Mantissa */
		gsw_r32(cdev, (PCE_TCM_CIR_MANT_MANT_OFFSET + (mid * 7)),
			PCE_TCM_CIR_MANT_MANT_SHIFT,
			PCE_TCM_CIR_MANT_MANT_SIZE, &mant);
	   /* Rate Counter iBS */
		gsw_r32(cdev, (PCE_TCM_IBS_IBS_OFFSET + (mid * 7)),
			PCE_TCM_IBS_IBS_SHIFT,
			PCE_TCM_IBS_IBS_SIZE, &ibs);
		/* calc the Rate */
		parm->nRate = mratecalc(ibs, exp, mant);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_MeterCfgSet(void *cdev,
	GSW_QoS_meterCfg_t *parm)
{
	u32 mid, cbs, ebs, exp = 0, mant = 0, rate, ibs = 0;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	mid = parm->nMeterId;
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (mid > gswdev->num_of_meters)
			return GSW_statusErr;
		gsw_w32(cdev, GSW_INST_SEL_INST_OFFSET,
			GSW_INST_SEL_INST_SHIFT,
			GSW_INST_SEL_INST_SIZE, mid);
		/* Committed Burst Size */
		if (parm->nCbs > 0xFFC0)
			cbs = 0x3FF;
		else
			cbs = ((parm->nCbs + 63) / 64);
		gsw_w32(cdev, GSW_PCE_TCM_CBS_CBS_OFFSET,
			GSW_PCE_TCM_CBS_CBS_SHIFT,
			GSW_PCE_TCM_CBS_CBS_SIZE, cbs);
		/* Excess Burst Size (EBS [bytes]) */
		if (parm->nEbs > 0xFFC0)
			ebs = 0x3FF;
		else
			ebs = ((parm->nEbs + 63) / 64);
		gsw_w32(cdev, GSW_PCE_TCM_EBS_EBS_OFFSET,
			GSW_PCE_TCM_EBS_EBS_SHIFT,
			GSW_PCE_TCM_EBS_EBS_SIZE, ebs);
		/* Calc the Rate and convert to MANT and EXP*/
		rate = parm->nRate;
		if (rate)
			calc_mtoken(rate, &ibs, &exp, &mant);
		/* Rate Counter Exponent */
		gsw_w32(cdev, GSW_PCE_TCM_CIR_EXP_EXP_OFFSET,
			GSW_PCE_TCM_CIR_EXP_EXP_SHIFT,
			GSW_PCE_TCM_CIR_EXP_EXP_SIZE, exp);
		/* Rate Counter Mantissa */
		gsw_w32(cdev, GSW_PCE_TCM_CIR_MANT_MANT_OFFSET,
			GSW_PCE_TCM_CIR_MANT_MANT_SHIFT,
			GSW_PCE_TCM_CIR_MANT_MANT_SIZE, mant);
		/* Rate Counter iBS */
		gsw_w32(cdev, GSW_PCE_TCM_IBS_IBS_OFFSET,
			GSW_PCE_TCM_IBS_IBS_SHIFT,
			GSW_PCE_TCM_IBS_IBS_SIZE, ibs);

		/* Calc the Rate and convert to MANT and EXP*/
		rate = parm->nPiRate;
		if (rate)
			calc_mtoken(rate, &ibs, &exp, &mant);
		else {
			ibs = 0; exp = 0; mant = 0;
		}
		/* Rate Counter Exponent */
		gsw_w32(cdev, GSW_PCE_TCM_PIR_EXP_EXP_OFFSET,
			GSW_PCE_TCM_PIR_EXP_EXP_SHIFT,
			GSW_PCE_TCM_PIR_EXP_EXP_SIZE, exp);
		/* Rate Counter Mantissa */
		gsw_w32(cdev, GSW_PCE_TCM_PIR_MANT_MANT_OFFSET,
			GSW_PCE_TCM_PIR_MANT_MANT_SHIFT,
			GSW_PCE_TCM_PIR_MANT_MANT_SIZE, mant);
		/* Rate Counter iBS */
/*		gsw_w32(cdev, GSW_PCE_TCM_IBS_IBS_OFFSET, */
/*			GSW_PCE_TCM_IBS_IBS_SHIFT,*/
/*			GSW_PCE_TCM_IBS_IBS_SIZE, ibs);*/
		gsw_w32(cdev, GSW_PCE_TCM_CTRL_TMOD_OFFSET,
			GSW_PCE_TCM_CTRL_TMOD_SHIFT,
			GSW_PCE_TCM_CTRL_TMOD_SIZE, parm->eMtrType);
		/* Enable/Disable the meter shaper */
		gsw_w32(cdev, GSW_PCE_TCM_CTRL_TCMEN_OFFSET,
			GSW_PCE_TCM_CTRL_TCMEN_SHIFT,
			GSW_PCE_TCM_CTRL_TCMEN_SIZE, parm->bEnable);
	} else {
		if (mid > 7)
			return GSW_statusErr;
		/* Committed Burst Size */
		if (parm->nCbs > 0xFFC0)
			cbs = 0x3FF;
		else
			cbs = ((parm->nCbs + 63) / 64);
		gsw_w32(cdev, (PCE_TCM_CBS_CBS_OFFSET +
			(mid * 7)),
			PCE_TCM_CBS_CBS_SHIFT,
			PCE_TCM_CBS_CBS_SIZE, cbs);
		/* Excess Burst Size (EBS [bytes]) */
		if (parm->nEbs > 0xFFC0)
			ebs = 0x3FF;
		else
			ebs = ((parm->nEbs + 63) / 64);
		gsw_w32(cdev, (PCE_TCM_EBS_EBS_OFFSET +
			(mid * 7)),
			PCE_TCM_EBS_EBS_SHIFT,
			PCE_TCM_EBS_EBS_SIZE, ebs);
		/* Calc the Rate and convert to MANT and EXP*/
		rate = parm->nRate;
		if (rate)
			calc_mtoken(rate, &ibs, &exp, &mant);
		/* Rate Counter Exponent */
		gsw_w32(cdev, (PCE_TCM_CIR_EXP_EXP_OFFSET +
			(mid * 7)),
			PCE_TCM_CIR_EXP_EXP_SHIFT,
			PCE_TCM_CIR_EXP_EXP_SIZE, exp);
		/* Rate Counter Mantissa */
		gsw_w32(cdev, (PCE_TCM_CIR_MANT_MANT_OFFSET +
			(mid * 7)),
			PCE_TCM_CIR_MANT_MANT_SHIFT,
			PCE_TCM_CIR_MANT_MANT_SIZE, mant);
		/* Rate Counter iBS */
		gsw_w32(cdev, (PCE_TCM_IBS_IBS_OFFSET +
			(mid * 7)),
			PCE_TCM_IBS_IBS_SHIFT,
			PCE_TCM_IBS_IBS_SIZE, ibs);
		/* Enable/Disable the meter shaper */
		gsw_w32(cdev, (PCE_TCM_CTRL_TCMEN_OFFSET +
			(mid * 7)),
			PCE_TCM_CTRL_TCMEN_SHIFT,
			PCE_TCM_CTRL_TCMEN_SIZE, parm->bEnable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_MeterPortAssign(void *cdev,
	GSW_QoS_meterPort_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32 eport, iport, value1, value2;
	ltq_bool_t eftbl = 0, nempftbl = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	memset(&pcetable, 0, sizeof(pctbl_prog_t));
	value1 = 0;
	while (value1 < 8) {
		pcetable.table = PCE_METER_INS_0_INDEX;
		pcetable.pcindex = value1;
		gsw_pce_table_read(cdev, &pcetable);
		if (pcetable.valid == 1) {
			iport = pcetable.key[0] & 0xF;
			eport = (pcetable.key[0] >> 8) & 0xF;
			if ((eport == parm->nPortEgressId) &&
			(iport == parm->nPortIngressId)) {
				eftbl = 1;
				value2 = 0;
				while (value2 < 8) {
					pcetable.table = PCE_METER_INS_1_INDEX;
					pcetable.pcindex = value2;
					gsw_pce_table_read(cdev, &pcetable);
					if (pcetable.valid == 1) {
						iport = pcetable.key[0] & 0xF;
						eport = ((pcetable.key[0] >> 8) & 0xF);
						if ((eport == parm->nPortEgressId) &&
						(iport == parm->nPortIngressId))
							return GSW_statusErr;
					}
					value2++;
				}
			}
		}
		value1++;
	}
	/*  Not in the original table, write a new one */
	if (eftbl == 0) {
		value1 = 0;
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		/* Search for whole Table 1*/
		while (value1 < 8) {
			pcetable.table = PCE_METER_INS_0_INDEX;
			pcetable.pcindex = value1;
			gsw_pce_table_read(cdev, &pcetable);
			/* We found the empty one */
			if (pcetable.valid == 0) {
				switch (parm->eDir) {
				case GSW_DIRECTION_BOTH:
					pcetable.key[0] =
					(((parm->nPortEgressId & 0xF) << 8)
					| (parm->nPortIngressId & 0xF));
					pcetable.mask[0] = 0;
					break;
				case GSW_DIRECTION_EGRESS:
					pcetable.key[0] =
					(((parm->nPortEgressId & 0xF) << 8)
					| 0xF);
					pcetable.mask[0] = 1;
					break;
				case GSW_DIRECTION_INGRESS:
					pcetable.key[0] = (0xF00 |
						(parm->nPortIngressId & 0xF));
					pcetable.mask[0] = 4;
					break;
				default:
					pcetable.key[0] = 0;
					pcetable.mask[0] = 5;
				}
				pcetable.val[0] = parm->nMeterId & 0x3F;
				pcetable.valid = 1;
				gsw_pce_table_write(cdev, &pcetable);
				return GSW_statusOk;
			}
			value1++;
		}
		if (value1 >= 8)
			nempftbl = 1;
	}

	/* The Table 1 is full, We go search table 2 */
	if ((nempftbl == 1) || (eftbl == 1)) {
		value2 = 0;
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		while (value2 < 8) {
			pcetable.table = PCE_METER_INS_1_INDEX;
			pcetable.pcindex = value2;
			gsw_pce_table_read(cdev, &pcetable);
			/* We found the empty one */
			if (pcetable.valid == 0) {
				switch (parm->eDir) {
				case GSW_DIRECTION_BOTH:
					pcetable.key[0] =
					(((parm->nPortEgressId & 0xF) << 8) |
					(parm->nPortIngressId & 0xF));
					pcetable.mask[0] = 0;
					break;
				case GSW_DIRECTION_EGRESS:
					pcetable.key[0] =
					(((parm->nPortEgressId & 0xF) << 8)
					| 0xF);
					pcetable.mask[0] = 1;
					break;
				case GSW_DIRECTION_INGRESS:
					pcetable.key[0] = (0xF00 |
					(parm->nPortIngressId & 0xF));
					pcetable.mask[0] = 4;
					break;
				default:
					pcetable.key[0] = 0;
					pcetable.mask[0] = 5;
				}
				pcetable.val[0] = parm->nMeterId & 0x3F;
				pcetable.valid = 1;
				gsw_pce_table_write(cdev, &pcetable);
				nempftbl = 0;
				return GSW_statusOk;
			}
			value2++;
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_MeterPortDeassign(void *cdev,
	GSW_QoS_meterPort_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  eport, iport, mid, i, j;
	ltq_bool_t eftbl = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	for (i = 0; i < 2; i++) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		if (i == 0)
			pcetable.table = PCE_METER_INS_0_INDEX;
		else
			pcetable.table = PCE_METER_INS_1_INDEX;
		for (j = 0; j < 8; j++) {
			pcetable.pcindex = j;
			gsw_pce_table_read(cdev, &pcetable);
			if (pcetable.valid == 1) {
				iport = pcetable.key[0] & 0xF;
				eport = (pcetable.key[0] >> 8) & 0xF;
				mid = pcetable.val[0] & 0x1F;
				if ((eport == parm->nPortEgressId) &&
					(iport == parm->nPortIngressId) &&
					(mid == parm->nMeterId)) {
					if (i == 0)
						pcetable.table =
						PCE_METER_INS_0_INDEX;
					else
						pcetable.table =
						PCE_METER_INS_1_INDEX;
					pcetable.key[0] = 0;
					pcetable.val[0] = 0;
					pcetable.valid = 0;
					gsw_pce_table_write(cdev, &pcetable);
					eftbl = 1;
					pr_err("Found the entry, delet it\n");
				}

			}
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_MeterPortGet(void *cdev,
	GSW_QoS_meterPortGet_t *parm)
{
	pctbl_prog_t pcetable;
	u32 value = 0;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	memset(&pcetable, 0, sizeof(pctbl_prog_t));
/*	gsw_r32(cdev, ETHSW_CAP_3_METERS_OFFSET,*/
/*		ETHSW_CAP_3_METERS_SHIFT,*/
/*		ETHSW_CAP_3_METERS_SIZE, &value); */
	value = gswdev->num_of_meters;
	if (parm->bInitial == 1) {
		gswdev->meter_cnt = 0;
		parm->bInitial = 0;
	} else {
		if (gswdev->meter_cnt > (value * 2))
			parm->bLast = 1;
	}

	if (gswdev->meter_cnt > value)
		pcetable.table = PCE_METER_INS_1_INDEX;
	else
		pcetable.table = PCE_METER_INS_0_INDEX;

	pcetable.pcindex = gswdev->meter_cnt;
	gsw_pce_table_read(cdev, &pcetable);
	if (pcetable.valid) {
		parm->nMeterId = (pcetable.val[0] & 0x3F);
		parm->nPortEgressId = ((pcetable.key[0] >> 8) & 0xF);
		parm->nPortIngressId = (pcetable.key[0] & 0xF);
		if ((pcetable.mask[0] & 0x5) == 0)
				parm->eDir = GSW_DIRECTION_BOTH;
		else if ((pcetable.mask[0] & 0x5) == 1)
			parm->eDir = GSW_DIRECTION_EGRESS;
		else if ((pcetable.mask[0] & 0x5) == 4)
			parm->eDir = GSW_DIRECTION_INGRESS;
		else
			parm->eDir = GSW_DIRECTION_NONE;
	}
	gswdev->meter_cnt++;
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_DSCP_ClassGet(void *cdev,
	GSW_QoS_DSCP_ClassCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	for (value = 0; value <= 63; value++) {
		pcetable.table = PCE_DSCP_INDEX;
		pcetable.pcindex = value;
		gsw_pce_table_read(cdev, &pcetable);
		parm->nTrafficClass[value] = (pcetable.val[0] & 0xF);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_DSCP_ClassSet(void *cdev,
	GSW_QoS_DSCP_ClassCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
#if 0
	memset(&pcetable, 0, sizeof(pctbl_prog_t));
	pcetable.table = PCE_DSCP_INDEX;
	/* index of the DSCP configuration */
	pcetable.pcindex = parm->nDSCP;
	gsw_pce_table_read(cdev, &pcetable);
	pcetable.val[0] &= ~(0xFF);
	pcetable.val[0] |= (parm->nTrafficClass[parm->nDSCP] & 0xFF);
	gsw_pce_table_write(cdev, &pcetable);
#else
	for (value = 0; value <= 63; value++) {
		pcetable.table = PCE_DSCP_INDEX;
		pcetable.pcindex = value;
		gsw_pce_table_read(cdev, &pcetable);
		pcetable.val[0] &= ~(0xF);
		pcetable.val[0] |= (parm->nTrafficClass[value] & 0xF);
		gsw_pce_table_write(cdev, &pcetable);
	}
#endif
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ClassDSCP_Get(void *cdev,
	GSW_QoS_ClassDSCP_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver != LTQ_GSWIP_3_0) {
		for (value = 0; value < 16; value++) {
			pcetable.table = PCE_REMARKING_INDEX;
			pcetable.pcindex = value;
			gsw_pce_table_read(cdev, &pcetable);
			parm->nDSCP[value] = pcetable.val[0] & 0x3F;
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ClassDSCP_Set(void *cdev,
	GSW_QoS_ClassDSCP_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  dscp, pcp, value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver != LTQ_GSWIP_3_0) {
		for (value = 0; value < 16; value++) {
			pcetable.table = PCE_REMARKING_INDEX;
			pcetable.pcindex = value;
			gsw_pce_table_read(cdev, &pcetable);
			pcp = (pcetable.val[0] >> 8) & 0x7;
			dscp	= parm->nDSCP[value] & 0x3F;
			pcetable.val[0] = ((pcp << 8) | dscp);
			gsw_pce_table_write(cdev, &pcetable);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgGet(void *cdev,
	GSW_QoS_DSCP_DropPrecedenceCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	for (value = 0; value <= 63; value++) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.table = PCE_DSCP_INDEX;
		pcetable.pcindex = value;
		gsw_pce_table_read(cdev, &pcetable);
		parm->nDSCP_DropPrecedence[value]	=
			((pcetable.val[0] >> 4) & 0x3);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_DSCP_DropPrecedenceCfgSet(void *cdev,
	GSW_QoS_DSCP_DropPrecedenceCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
#if 0
	memset(&pcetable, 0, sizeof(pctbl_prog_t));
	pcetable.table = PCE_DSCP_INDEX;
	/* index of the DSCP configuration*/
	pcetable.pcindex = parm->nDSCP;
	gsw_pce_table_read(cdev, &pcetable);
	pcetable.val[0] &= ~(0x3 << 4);
	pcetable.val[0] |=
	((parm->nDSCP_DropPrecedence[parm->nDSCP] & 0x3) << 4);
	gsw_pce_table_write(cdev, &pcetable);
#else
	for (value = 0; value <= 63; value++) {
		memset(&pcetable, 0, sizeof(pctbl_prog_t));
		pcetable.table = PCE_DSCP_INDEX;
		pcetable.pcindex = value;
		gsw_pce_table_read(cdev, &pcetable);
		pcetable.val[0] &= ~(0x3 << 4);
		pcetable.val[0] |=
		((parm->nDSCP_DropPrecedence[value] & 0x3) << 4);
		gsw_pce_table_write(cdev, &pcetable);
	}
#endif
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_PortRemarkingCfgGet(void *cdev,
	GSW_QoS_portRemarkingCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32  value, vclpen = 0, vdpen = 0, vdscpmod = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;

	gsw_r32(cdev, (PCE_PCTRL_0_CLPEN_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_0_CLPEN_SHIFT,
		PCE_PCTRL_0_CLPEN_SIZE, &vclpen);
	gsw_r32(cdev, (PCE_PCTRL_0_DPEN_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_0_DPEN_SHIFT,
		PCE_PCTRL_0_DPEN_SIZE, &vdpen);
	gsw_r32(cdev, (PCE_PCTRL_2_DSCPMOD_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_2_DSCPMOD_SHIFT,
		PCE_PCTRL_2_DSCPMOD_SIZE, &vdscpmod);

	if ((vclpen == 0) && (vdpen == 0)
		&& (vdscpmod == 0))
		parm->eDSCP_IngressRemarkingEnable =
		GSW_DSCP_REMARK_DISABLE;
	else if ((vclpen == 1) && (vdpen == 0)
		&& (vdscpmod == 1))
		parm->eDSCP_IngressRemarkingEnable =
		GSW_DSCP_REMARK_TC6;
	else if ((vclpen == 1) && (vdpen == 1) &&
		(vdscpmod == 1))
		parm->eDSCP_IngressRemarkingEnable =
		GSW_DSCP_REMARK_TC3;
	else if ((vclpen == 0) && (vdpen == 1) &&
		(vdscpmod == 1))
		parm->eDSCP_IngressRemarkingEnable =
		GSW_DSCP_REMARK_DP3;
	else if ((vclpen == 1) && (vdpen == 1) &&
		(vdscpmod == 1))
		parm->eDSCP_IngressRemarkingEnable =
		GSW_DSCP_REMARK_DP3_TC3;

	gsw_r32(cdev, (PCE_PCTRL_0_PCPEN_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_0_PCPEN_SHIFT,
		PCE_PCTRL_0_PCPEN_SIZE, &value);
	parm->bPCP_IngressRemarkingEnable = value;
	gsw_r32(cdev, (FDMA_PCTRL_DSCPRM_OFFSET +
		(6 * parm->nPortId)),
		FDMA_PCTRL_DSCPRM_SHIFT,
		FDMA_PCTRL_DSCPRM_SIZE, &value);
	parm->bDSCP_EgressRemarkingEnable = value;
	gsw_r32(cdev, (FDMA_PCTRL_VLANMOD_OFFSET +
		(6 * parm->nPortId)),
		FDMA_PCTRL_VLANMOD_SHIFT,
		FDMA_PCTRL_VLANMOD_SIZE, &value);
	if (value == 3)
		parm->bPCP_EgressRemarkingEnable = 1;
	else
		parm->bPCP_EgressRemarkingEnable = 0;

	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, (PCE_PCTRL_2_SPCPEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_SPCPEN_SHIFT,
			PCE_PCTRL_2_SPCPEN_SIZE,
			&parm->bSTAG_PCP_IngressRemarkingEnable);
		gsw_r32(cdev, (PCE_PCTRL_2_SDEIEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_SDEIEN_SHIFT,
			PCE_PCTRL_2_SDEIEN_SIZE,
			&parm->bSTAG_DEI_IngressRemarkingEnable);
		gsw_w32(cdev, (FDMA_PCTRL_SVLANMOD_OFFSET +
			(6 * parm->nPortId)),
			FDMA_PCTRL_SVLANMOD_SHIFT,
			FDMA_PCTRL_SVLANMOD_SIZE, value);
		if (value == 3)
			parm->bSTAG_PCP_DEI_EgressRemarkingEnable = 1;
		else
			parm->bSTAG_PCP_DEI_EgressRemarkingEnable = 0;
		}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_PortRemarkingCfgSet(void *cdev,
	GSW_QoS_portRemarkingCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32  value, vclpen = 0, vdpen = 0, vdscpmod = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	switch (parm->eDSCP_IngressRemarkingEnable) {
	case GSW_DSCP_REMARK_DISABLE:
		vclpen = 0; vdpen = 0; vdscpmod = 0;
		break;
	case GSW_DSCP_REMARK_TC6:
		vclpen = 1; vdpen = 0; vdscpmod = 1;
		break;
	case GSW_DSCP_REMARK_TC3:
		vclpen = 1; vdpen = 1; vdscpmod = 1;
		break;
	case GSW_DSCP_REMARK_DP3:
		vclpen = 0; vdpen = 1; vdscpmod = 1;
		break;
	case GSW_DSCP_REMARK_DP3_TC3:
		vclpen = 1; vdpen = 1;
		vdscpmod = 1;
		break;
	}

	if (parm->eDSCP_IngressRemarkingEnable == 0) {
		vclpen = 0; vdpen = 0; vdscpmod = 0;
	}
	gsw_w32(cdev, PCE_PCTRL_0_CLPEN_OFFSET +
		(10 * parm->nPortId),
		PCE_PCTRL_0_CLPEN_SHIFT,
		PCE_PCTRL_0_CLPEN_SIZE, vclpen);
	gsw_w32(cdev, PCE_PCTRL_0_DPEN_OFFSET +
		(10 * parm->nPortId),
		PCE_PCTRL_0_DPEN_SHIFT,
		PCE_PCTRL_0_DPEN_SIZE, vdpen);
	gsw_w32(cdev, PCE_PCTRL_2_DSCPMOD_OFFSET +
		(10 * parm->nPortId),
		PCE_PCTRL_2_DSCPMOD_SHIFT,
		PCE_PCTRL_2_DSCPMOD_SIZE, vdscpmod);
	if (parm->bDSCP_EgressRemarkingEnable > 0)
		value = parm->bDSCP_EgressRemarkingEnable;
	else
		value = 0;
	gsw_w32(cdev, (FDMA_PCTRL_DSCPRM_OFFSET +
		(6 * parm->nPortId)),
		FDMA_PCTRL_DSCPRM_SHIFT,
		FDMA_PCTRL_DSCPRM_SIZE, value);
	if (parm->bPCP_IngressRemarkingEnable > 0)
		value = parm->bPCP_IngressRemarkingEnable;
	else
		value = 0;
	gsw_w32(cdev, (PCE_PCTRL_0_PCPEN_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_0_PCPEN_SHIFT,
		PCE_PCTRL_0_PCPEN_SIZE, value);
	if (parm->bPCP_EgressRemarkingEnable > 0)
		value = 3;
	else
		value = 0;
	gsw_w32(cdev, (FDMA_PCTRL_VLANMOD_OFFSET +
		(6 * parm->nPortId)),
		FDMA_PCTRL_VLANMOD_SHIFT,
		FDMA_PCTRL_VLANMOD_SIZE, value);
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_w32(cdev, (PCE_PCTRL_2_SPCPEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_SPCPEN_SHIFT,
			PCE_PCTRL_2_SPCPEN_SIZE,
			parm->bSTAG_PCP_IngressRemarkingEnable);
		gsw_w32(cdev, (PCE_PCTRL_2_SDEIEN_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_SDEIEN_SHIFT,
			PCE_PCTRL_2_SDEIEN_SIZE,
			parm->bSTAG_DEI_IngressRemarkingEnable);
		if (parm->bSTAG_PCP_DEI_EgressRemarkingEnable > 0)
			value = 3;
		else
			value = 0;
/*		gsw_w32(cdev, (FDMA_PCTRL_DEIMOD_OFFSET + */
/*			(6 * parm->nPortId)), */
/*			FDMA_PCTRL_DEIMOD_SHIFT,*/
/*			FDMA_PCTRL_DEIMOD_SIZE,*/
/*			parm->bSTAG_PCP_DEI_EgressRemarkingEnable);	*/
		gsw_w32(cdev, (FDMA_PCTRL_SVLANMOD_OFFSET +
			(6 * parm->nPortId)),
			FDMA_PCTRL_SVLANMOD_SHIFT,
			FDMA_PCTRL_SVLANMOD_SIZE, value);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ClassPCP_Get(void *cdev,
	GSW_QoS_ClassPCP_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver != LTQ_GSWIP_3_0) {
		for (value = 0; value < 16; value++) {
			pcetable.table = PCE_REMARKING_INDEX;
			pcetable.pcindex = value;
			gsw_pce_table_read(cdev, &pcetable);
			parm->nPCP[value] = (pcetable.val[0] >> 8) & 0x7;
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ClassPCP_Set(void *cdev,
	GSW_QoS_ClassPCP_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  dscp, pcp, value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver != LTQ_GSWIP_3_0) {
		for (value = 0; value < 16; value++) {
			pcetable.table = PCE_REMARKING_INDEX;
			pcetable.pcindex = value;
			gsw_pce_table_read(cdev, &pcetable);
			dscp = pcetable.val[0] & 0x3F;
			pcp = parm->nPCP[value] & 0x7;
			pcetable.val[0] = (pcp << 8) | dscp;
			gsw_pce_table_write(cdev, &pcetable);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_PCP_ClassGet(void *cdev,
	GSW_QoS_PCP_ClassCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	for (value = 0; value < 8; value++) {
		pcetable.table = PCE_PCP_INDEX;
		pcetable.pcindex = value;
		gsw_pce_table_read(cdev, &pcetable);
		parm->nTrafficClass[value] = (pcetable.val[0] & 0xF);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_PCP_ClassSet(void *cdev,
	GSW_QoS_PCP_ClassCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	for (value = 0; value < 8; value++) {
		pcetable.table = PCE_PCP_INDEX;
		pcetable.pcindex = value;
		pcetable.val[0] = (parm->nTrafficClass[value] & 0xF);
		gsw_pce_table_write(cdev, &pcetable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_PortCfgGet(void *cdev,
	GSW_QoS_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32  value, dscp, cpcp, spcp = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;

	gsw_r32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_2_DSCP_SHIFT,
		PCE_PCTRL_2_DSCP_SIZE, &dscp);
	gsw_r32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_2_PCP_SHIFT,
		PCE_PCTRL_2_PCP_SIZE, &cpcp);
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_SPCP_SHIFT,
			PCE_PCTRL_2_SPCP_SIZE, &spcp);
	}
	if ((dscp == 0) && (cpcp == 0) && (spcp == 0))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_NO;
	else if ((dscp == 2) && (cpcp == 0) && (spcp == 0))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_DSCP;
	else if ((dscp == 0) && (cpcp == 1) && (spcp == 0))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_PCP;
	else if ((dscp == 2) && (cpcp == 1) && (spcp == 0))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_DSCP_PCP;
	else if ((dscp == 1) && (cpcp == 1) && (spcp == 0))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_PCP_DSCP;
	else if ((dscp == 0) && (cpcp == 0) && (spcp == 1))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_SPCP;
	else if ((dscp == 1) && (cpcp == 0) && (spcp == 1))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_SPCP_DSCP;
	else if ((dscp == 2) && (cpcp == 0) && (spcp == 1))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_DSCP_SPCP;
	else if ((dscp == 0) && (cpcp == 1) && (spcp == 1))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_SPCP_PCP;
	else if ((dscp == 1) && (cpcp == 1) && (spcp == 1))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_SPCP_PCP_DSCP;
	else if ((dscp == 2) && (cpcp == 1) && (spcp == 1))
		parm->eClassMode = GSW_QOS_CLASS_SELECT_DSCP_SPCP_PCP;
	else
		parm->eClassMode = GSW_QOS_CLASS_SELECT_NO;

	gsw_r32(cdev, (PCE_PCTRL_2_PCLASS_OFFSET +
		(10 * parm->nPortId)),
		PCE_PCTRL_2_PCLASS_SHIFT,
		PCE_PCTRL_2_PCLASS_SIZE, &value);
	parm->nTrafficClass = value;
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_PortCfgSet(void *cdev,
	GSW_QoS_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	switch (parm->eClassMode) {
	case GSW_QOS_CLASS_SELECT_NO:
		gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
		(10 * parm->nPortId)),
			PCE_PCTRL_2_DSCP_SHIFT,
			PCE_PCTRL_2_DSCP_SIZE, 0);
		gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_PCP_SHIFT,
			PCE_PCTRL_2_PCP_SIZE, 0);
		if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC)
			|| (gswdev->gipver == LTQ_GSWIP_3_0)) {
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET
				+ (10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 0);
		}
		break;
	case GSW_QOS_CLASS_SELECT_DSCP:
		gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_DSCP_SHIFT,
			PCE_PCTRL_2_DSCP_SIZE, 2);
		gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_PCP_SHIFT,
			PCE_PCTRL_2_PCP_SIZE, 0);
		if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
			(gswdev->gipver == LTQ_GSWIP_3_0)) {
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 0);
		}
		break;
	case GSW_QOS_CLASS_SELECT_PCP:
		gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_DSCP_SHIFT,
			PCE_PCTRL_2_DSCP_SIZE, 0);
		gsw_w32(cdev, PCE_PCTRL_2_PCP_OFFSET +
			(10 * parm->nPortId),
			PCE_PCTRL_2_PCP_SHIFT,
			PCE_PCTRL_2_PCP_SIZE, 1);
		if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
			(gswdev->gipver == LTQ_GSWIP_3_0)) {
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 0);
		}
			break;
	case GSW_QOS_CLASS_SELECT_DSCP_PCP:
		gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_DSCP_SHIFT,
			PCE_PCTRL_2_DSCP_SIZE, 2);
		gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_PCP_SHIFT,
			PCE_PCTRL_2_PCP_SIZE, 1);
		if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
			(gswdev->gipver == LTQ_GSWIP_3_0)) {
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 0);
		}
		break;
	case GSW_QOS_CLASS_SELECT_PCP_DSCP:
		gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_DSCP_SHIFT,
			PCE_PCTRL_2_DSCP_SIZE, 1);
		gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
			(10 * parm->nPortId)),
			PCE_PCTRL_2_PCP_SHIFT,
			PCE_PCTRL_2_PCP_SIZE, 1);
		if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
			(gswdev->gipver == LTQ_GSWIP_3_0)) {
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 0);
		}
	break;
	if ((gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		case GSW_QOS_CLASS_SELECT_SPCP:
			gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_DSCP_SHIFT,
				PCE_PCTRL_2_DSCP_SIZE, 0);
			gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_PCP_SHIFT,
				PCE_PCTRL_2_PCP_SIZE, 0);
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 1);
			break;
		case GSW_QOS_CLASS_SELECT_SPCP_DSCP:
			gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_DSCP_SHIFT,
				PCE_PCTRL_2_DSCP_SIZE, 1);
			gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_PCP_SHIFT,
				PCE_PCTRL_2_PCP_SIZE, 0);
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 1);
			break;
		case GSW_QOS_CLASS_SELECT_DSCP_SPCP:
			gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_DSCP_SHIFT,
				PCE_PCTRL_2_DSCP_SIZE, 2);
			gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_PCP_SHIFT,
				PCE_PCTRL_2_PCP_SIZE, 0);
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 1);
			break;
		case GSW_QOS_CLASS_SELECT_SPCP_PCP:
			gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_DSCP_SHIFT,
				PCE_PCTRL_2_DSCP_SIZE, 0);
			gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_PCP_SHIFT,
				PCE_PCTRL_2_PCP_SIZE, 1);
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 1);
			break;
		case GSW_QOS_CLASS_SELECT_SPCP_PCP_DSCP:
			gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_DSCP_SHIFT,
				PCE_PCTRL_2_DSCP_SIZE, 1);
			gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_PCP_SHIFT,
				PCE_PCTRL_2_PCP_SIZE, 1);
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 1);
			break;
		case GSW_QOS_CLASS_SELECT_DSCP_SPCP_PCP:
			gsw_w32(cdev, (PCE_PCTRL_2_DSCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_DSCP_SHIFT,
				PCE_PCTRL_2_DSCP_SIZE, 2);
			gsw_w32(cdev, (PCE_PCTRL_2_PCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_PCP_SHIFT,
				PCE_PCTRL_2_PCP_SIZE, 1);
			gsw_w32(cdev, (PCE_PCTRL_2_SPCP_OFFSET +
				(10 * parm->nPortId)),
				PCE_PCTRL_2_SPCP_SHIFT,
				PCE_PCTRL_2_SPCP_SIZE, 1);
			break;
		}
	}
	if (parm->nTrafficClass > 0xF)
		return GSW_statusErr;
	else
		gsw_w32(cdev, PCE_PCTRL_2_PCLASS_OFFSET
			+ (10 * parm->nPortId),
			PCE_PCTRL_2_PCLASS_SHIFT,
			PCE_PCTRL_2_PCLASS_SIZE,
			parm->nTrafficClass);
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_QueuePortGet(void *cdev,
	GSW_QoS_queuePort_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d\n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	memset(&pcetable, 0, sizeof(pctbl_prog_t));
	pcetable.table = PCE_QUEUE_MAP_INDEX;
	pcetable.pcindex = ((((parm->nPortId << 4) & 0xF0)
		| (parm->nTrafficClassId & 0xF)));
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (parm->bRedirectionBypass == 1)
			pcetable.pcindex |= (1 << 8);
	}
	gsw_pce_table_read(cdev, &pcetable);
	parm->nQueueId = (pcetable.val[0] & 0x3F);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		u32  value;
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE, parm->nQueueId);
		/* Specify the Table Address */
		gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
			BM_RAM_CTRL_ADDR_SHIFT,
			BM_RAM_CTRL_ADDR_SIZE, 0xE);
		/* Assign Read operation */
		gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
			BM_RAM_CTRL_OPMOD_SHIFT,
			BM_RAM_CTRL_OPMOD_SIZE, 0);
		value = 1;
		/* Active */
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			 BM_RAM_CTRL_BAS_SHIFT,
			 BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);

		gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE, &value);
		parm->nRedirectPortId = (value & 0x7);
		parm->nRedirectPortId |= (((value >> 4) & 1) << 3);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_QueuePortSet(void *cdev,
	GSW_QoS_queuePort_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t pcetable;
	u32  eport = 0, value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	memset(&pcetable, 0, sizeof(pctbl_prog_t));
	pcetable.table = PCE_QUEUE_MAP_INDEX;
	pcetable.pcindex = (((parm->nPortId << 4) & 0xF0)
		| (parm->nTrafficClassId & 0xF));
	if (parm->bRedirectionBypass == 1)
		pcetable.pcindex |= (1 << 8);
	else
		pcetable.pcindex &= ~(1 << 8);
	pcetable.val[0] = (parm->nQueueId & 0x1F);
	gsw_pce_table_write(cdev, &pcetable);
	/* Assign the Egress Port Id and Enable the Queue */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		eport = (parm->nPortId & 0xF) << 8;
		eport |= (parm->nRedirectPortId & 0x7);
		eport |= (((parm->nRedirectPortId >> 3) & 0x1) << 4);
	} else {
		eport = (parm->nPortId & 0x7);
		eport |= (((parm->nPortId >> 3) & 0x1) << 4);
	}
	gsw_w32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
		BM_RAM_VAL_0_VAL0_SHIFT,
		BM_RAM_VAL_0_VAL0_SIZE, eport);
	gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
		BM_RAM_ADDR_ADDR_SHIFT,
		BM_RAM_ADDR_ADDR_SIZE, parm->nQueueId);
	/* Specify the Table Address */
	gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
		BM_RAM_CTRL_ADDR_SHIFT,
		BM_RAM_CTRL_ADDR_SIZE, 0xE);
	/* Assign Write operation */
	gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
		BM_RAM_CTRL_OPMOD_SHIFT,
		BM_RAM_CTRL_OPMOD_SIZE, 1);
	value = 1;
	/* Active */
	gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
		 BM_RAM_CTRL_BAS_SHIFT,
		 BM_RAM_CTRL_BAS_SIZE, value);
	do {
		gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, &value);
	} while (value);
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_SchedulerCfgGet(void *cdev,
	GSW_QoS_schedulerCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	int qid = parm->nQueueId;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET,*/
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (qid >= gswdev->num_of_queues)
		return GSW_statusErr;
	gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
		BM_RAM_ADDR_ADDR_SHIFT,
		BM_RAM_ADDR_ADDR_SIZE, qid);
	gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
		BM_RAM_CTRL_ADDR_SHIFT,
		BM_RAM_CTRL_ADDR_SIZE, 0x8);
	/* Table Access Operation Mode read */
	gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
		BM_RAM_CTRL_OPMOD_SHIFT,
		BM_RAM_CTRL_OPMOD_SIZE, 0x0);
	gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
		BM_RAM_CTRL_BAS_SHIFT,
		BM_RAM_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, &value);
	} while (value);
	gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
		BM_RAM_VAL_0_VAL0_SHIFT,
		BM_RAM_VAL_0_VAL0_SIZE, &value);
	parm->nWeight = value;
	if (value == 0xFFFF || value == 0x1800)
		parm->eType = GSW_QOS_SCHEDULER_STRICT;
	else
		parm->eType = GSW_QOS_SCHEDULER_WFQ;
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_SchedulerCfgSet(void *cdev,
	GSW_QoS_schedulerCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	int qid = parm->nQueueId;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (qid >= gswdev->num_of_queues)
		return GSW_statusErr;
	if ((parm->eType == GSW_QOS_SCHEDULER_WFQ) &&
		(parm->nWeight == 0))
		return GSW_statusErr;
	gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
		BM_RAM_ADDR_ADDR_SHIFT,
		BM_RAM_ADDR_ADDR_SIZE, qid);
	gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
		BM_RAM_CTRL_ADDR_SHIFT,
		BM_RAM_CTRL_ADDR_SIZE, 0x8);
	if (parm->eType == GSW_QOS_SCHEDULER_STRICT)
		value = 0xFFFF;
	else
		value = parm->nWeight;
	gsw_w32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
		BM_RAM_VAL_0_VAL0_SHIFT,
		BM_RAM_VAL_0_VAL0_SIZE, value);
	/* Table Access Operation Mode Write */
	gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
		BM_RAM_CTRL_OPMOD_SHIFT,
		BM_RAM_CTRL_OPMOD_SIZE, 0x1);
	/* Active */
	gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
		BM_RAM_CTRL_BAS_SHIFT,
		BM_RAM_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, &value);
	} while (value);
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ShaperCfgGet(void *cdev,
	GSW_QoS_ShaperCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	int rshid = parm->nRateShaperId;
	u32  value, exp, mant, ibs;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

/*	gsw_r32(cdev, ETHSW_CAP_3_SHAPERS_OFFSET, */
/*		ETHSW_CAP_3_SHAPERS_SHIFT, */
/*		ETHSW_CAP_3_SHAPERS_SIZE, &value); */
	if (rshid >= gswdev->num_of_shapers)
		return GSW_statusErr;
	/* Enable/Disable the rate shaper  */
	gsw_r32(cdev, RS_CTRL_RSEN_OFFSET + (rshid * 0x5),
		RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, &value);
	parm->bEnable = value;
	/* Committed Burst Size (CBS [bytes]) */
	gsw_r32(cdev, RS_CBS_CBS_OFFSET + (rshid * 0x5),
		RS_CBS_CBS_SHIFT, RS_CBS_CBS_SIZE, &value);
	parm->nCbs = (value * 64);
	/** Rate [Mbit/s] */
	gsw_r32(cdev, RS_CIR_EXP_EXP_OFFSET + (rshid * 0x5),
		RS_CIR_EXP_EXP_SHIFT, RS_CIR_EXP_EXP_SIZE, &exp);
	gsw_r32(cdev, RS_CIR_MANT_MANT_OFFSET + (rshid * 0x5),
		RS_CIR_MANT_MANT_SHIFT, RS_CIR_MANT_MANT_SIZE, &mant);
	gsw_r32(cdev, RS_IBS_IBS_OFFSET + (rshid * 0x5),
		RS_IBS_IBS_SHIFT, RS_IBS_IBS_SIZE, &ibs);
	/* calc the Rate */
	parm->nRate = mratecalc(ibs, exp, mant);
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, RS_CTRL_RSMOD_OFFSET + (rshid * 0x5),
			RS_CTRL_RSMOD_SHIFT, RS_CTRL_RSMOD_SIZE, &parm->bAVB);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ShaperCfgSet(void *cdev,
	GSW_QoS_ShaperCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	int rshid = parm->nRateShaperId;
	u32  value, exp = 0, mant = 0, rate = 0, ibs = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

/*	gsw_r32(cdev, ETHSW_CAP_3_SHAPERS_OFFSET, */
/*		ETHSW_CAP_3_SHAPERS_SHIFT, */
/*		ETHSW_CAP_3_SHAPERS_SIZE, &value); */
	if (rshid >= gswdev->num_of_shapers)
		return GSW_statusErr;
	/* Committed Burst Size */
	if (parm->nCbs > 0xFFC0)
		value = 0x3FF;
	else
		value = ((parm->nCbs + 63) / 64);
	/* Committed Burst Size (CBS [bytes]) */
	gsw_w32(cdev, RS_CBS_CBS_OFFSET + (rshid * 0x5),
		RS_CBS_CBS_SHIFT, RS_CBS_CBS_SIZE, value);
	/* Rate [kbit/s] */
	/* Calc the Rate */
	rate = parm->nRate;
	if (rate)
		calc_mtoken(rate, &ibs, &exp, &mant);
	gsw_w32(cdev, RS_CIR_EXP_EXP_OFFSET + (rshid * 0x5),
		 RS_CIR_EXP_EXP_SHIFT, RS_CIR_EXP_EXP_SIZE, exp);
	gsw_w32(cdev, RS_CIR_MANT_MANT_OFFSET + (rshid * 0x5),
		RS_CIR_MANT_MANT_SHIFT, RS_CIR_MANT_MANT_SIZE, mant);
	gsw_w32(cdev, RS_IBS_IBS_OFFSET + (rshid * 0x5),
		RS_IBS_IBS_SHIFT, RS_IBS_IBS_SIZE, ibs);
	/* Enable/Disable the rate shaper  */
	gsw_w32(cdev, RS_CTRL_RSEN_OFFSET + (rshid * 0x5),
		RS_CTRL_RSEN_SHIFT,
		RS_CTRL_RSEN_SIZE, parm->bEnable);
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_w32(cdev, RS_CTRL_RSMOD_OFFSET + (rshid * 0x5),
			RS_CTRL_RSMOD_SHIFT,
			RS_CTRL_RSMOD_SIZE, parm->bAVB);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ShaperQueueAssign(void *cdev,
	GSW_QoS_ShaperQueue_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 rshid = parm->nRateShaperId, qid = parm->nQueueId;
	u32 value1_RS, value1_ShaperId, value2_RS, value2_ShaperId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value1_RS); */
	if (qid >= gswdev->num_of_queues) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	gsw_r32(cdev, ETHSW_CAP_3_SHAPERS_OFFSET, */
/*		ETHSW_CAP_3_SHAPERS_SHIFT, */
/*		ETHSW_CAP_3_SHAPERS_SIZE, &value1_RS); */
	if (rshid >= gswdev->num_of_shapers) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Check Rate Shaper 1 Enable  */
	gsw_r32(cdev, PQM_RS_EN1_OFFSET + (qid * 2),
		PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, &value1_RS);
	gsw_r32(cdev, PQM_RS_RS1_OFFSET + (rshid * 2),
		PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, &value1_ShaperId);
	/* Check Rate Shaper 2 Enable  */
	gsw_r32(cdev, PQM_RS_EN2_OFFSET + (qid * 2),
		PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, &value2_RS);
	gsw_r32(cdev, PQM_RS_RS2_OFFSET + (rshid * 2),
		PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, &value2_ShaperId);
	if ((value1_RS == 1) && (value1_ShaperId == rshid))
		return GSW_statusOk;
	else if ((value2_RS == 1) &&
		(value2_ShaperId == rshid))
		return GSW_statusOk;
	else if (value1_RS == 0) {
		gsw_w32(cdev, PQM_RS_RS1_OFFSET + (qid * 2),
			PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, rshid);
		gsw_w32(cdev, PQM_RS_EN1_OFFSET + (qid * 2),
			PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, 0x1);
		gsw_w32(cdev, RS_CTRL_RSEN_OFFSET + (rshid * 0x5),
			RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, 0x1);
	} else if (value2_RS == 0) {
		gsw_w32(cdev, PQM_RS_RS2_OFFSET + (qid * 2),
			PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, rshid);
		gsw_w32(cdev, PQM_RS_EN2_OFFSET + (qid * 2),
			PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, 0x1);
		gsw_w32(cdev, RS_CTRL_RSEN_OFFSET + (rshid * 0x5),
			RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, 0x1);
	} else {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ShaperQueueDeassign(void *cdev,
	GSW_QoS_ShaperQueue_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 rshid = parm->nRateShaperId, qid = parm->nQueueId;
	u32 value1, value2, value3, value4;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value1); */
	if (qid >= gswdev->num_of_queues)
		return GSW_statusErr;
/*	gsw_r32(cdev, ETHSW_CAP_3_SHAPERS_OFFSET, */
/*		ETHSW_CAP_3_SHAPERS_SHIFT, */
/*		ETHSW_CAP_3_SHAPERS_SIZE, &value1); */
	if (rshid >= gswdev->num_of_shapers)
		return GSW_statusErr;
	/* Rate Shaper 1 Read  */
	gsw_r32(cdev, PQM_RS_EN1_OFFSET + (qid * 2),
		PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, &value1);
	gsw_r32(cdev, PQM_RS_RS1_OFFSET + (qid * 2),
		PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, &value2);
	/* Rate Shaper 2 Read  */
	gsw_r32(cdev, PQM_RS_EN2_OFFSET + (qid * 2),
		PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, &value3);
	gsw_r32(cdev, PQM_RS_RS2_OFFSET + (qid * 2),
		PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, &value4);
	if ((value1 == 1) && (value2 == rshid)) {
		gsw_w32(cdev, PQM_RS_EN1_OFFSET + (qid * 2),
			PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, 0);
		gsw_w32(cdev, PQM_RS_RS1_OFFSET + (qid * 2),
			PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, 0);
		return GSW_statusOk;
	} else if ((value3 == 1) && (value4 == rshid)) {
		gsw_w32(cdev, PQM_RS_EN2_OFFSET + (qid * 2),
			PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, 0);
		gsw_w32(cdev, PQM_RS_RS2_OFFSET + (qid * 2),
			PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, 0);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
	if ((value1 == 0) && (value3 == 0)) {
		gsw_w32(cdev, RS_CTRL_RSEN_OFFSET + (rshid * 0x5),
			RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, 0);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_ShaperQueueGet(void *cdev,
	GSW_QoS_ShaperQueueGet_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 rsh, value;
	u8 qid = parm->nQueueId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &rsh); */
	if (qid >= gswdev->num_of_queues)
		return GSW_statusErr;

	parm->bAssigned = 0;
	parm->nRateShaperId = 0;
	gsw_r32(cdev, PQM_RS_EN1_OFFSET + (qid * 2),
		PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, &rsh);
	if (rsh == 1) {
		parm->bAssigned = 1;
		gsw_r32(cdev, PQM_RS_RS1_OFFSET + (qid * 2),
			PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, &value);
		parm->nRateShaperId = value;
	}
	gsw_r32(cdev, PQM_RS_EN2_OFFSET + (qid * 2),
		PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, &rsh);
	if (rsh == 1) {
		gsw_r32(cdev, PQM_RS_RS2_OFFSET + (qid * 2),
			PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, &value);
		parm->nRateShaperId = value;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_StormCfgSet(void *cdev,
	GSW_QoS_stormCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 SCONBC, SCONMC, SCONUC, mid;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((parm->bBroadcast == 0) &&
		(parm->bMulticast == 0)	&&
		(parm->bUnknownUnicast == 0)) {
		/*  Storm Control Mode  */
		gsw_w32(cdev, PCE_GCTRL_0_SCONMOD_OFFSET,
			PCE_GCTRL_0_SCONMOD_SHIFT,
			PCE_GCTRL_0_SCONMOD_SIZE, 0);
		/* Meter instances used for broadcast traffic */
		gsw_w32(cdev, PCE_GCTRL_0_SCONBC_OFFSET,
			PCE_GCTRL_0_SCONBC_SHIFT,
			PCE_GCTRL_0_SCONBC_SIZE, 0);
		/* Meter instances used for multicast traffic */
		gsw_w32(cdev, PCE_GCTRL_0_SCONMC_OFFSET,
			PCE_GCTRL_0_SCONMC_SHIFT,
			PCE_GCTRL_0_SCONMC_SIZE, 0);
	/* Meter instances used for unknown unicast traffic */
		gsw_w32(cdev, PCE_GCTRL_0_SCONUC_OFFSET,
			PCE_GCTRL_0_SCONUC_SHIFT,
			PCE_GCTRL_0_SCONUC_SIZE, 0);
	}
	/*  Meter ID */
	gsw_r32(cdev, PCE_GCTRL_0_SCONMET_OFFSET,
		PCE_GCTRL_0_SCONMET_SHIFT,
		PCE_GCTRL_0_SCONMET_SIZE, &mid);
	/* Meter instances used for broadcast traffic */
	gsw_r32(cdev, PCE_GCTRL_0_SCONBC_OFFSET,
		PCE_GCTRL_0_SCONBC_SHIFT,
		PCE_GCTRL_0_SCONBC_SIZE, &SCONBC);
	/* Meter instances used for multicast traffic */
	gsw_r32(cdev, PCE_GCTRL_0_SCONMC_OFFSET,
		PCE_GCTRL_0_SCONMC_SHIFT,
		PCE_GCTRL_0_SCONMC_SIZE, &SCONMC);
	/* Meter instances used for unknown unicast traffic */
	gsw_r32(cdev, PCE_GCTRL_0_SCONUC_OFFSET,
		PCE_GCTRL_0_SCONUC_SHIFT,
		PCE_GCTRL_0_SCONUC_SIZE, &SCONUC);

	if ((SCONBC == 1) || (SCONMC == 1) || (SCONMC == 1)) {
		if (parm->nMeterId == (mid + 1)) {
			/*  Storm Control Mode  */
			gsw_w32(cdev, PCE_GCTRL_0_SCONMOD_OFFSET,
				PCE_GCTRL_0_SCONMOD_SHIFT,
				PCE_GCTRL_0_SCONMOD_SIZE, 3);
		} else if (parm->nMeterId != mid)
			return GSW_statusErr;
	} else {
		/*  Storm Control Mode */
		gsw_w32(cdev, PCE_GCTRL_0_SCONMOD_OFFSET,
			PCE_GCTRL_0_SCONMOD_SHIFT,
			PCE_GCTRL_0_SCONMOD_SIZE, 1);
		gsw_w32(cdev, PCE_GCTRL_0_SCONMET_OFFSET,
			PCE_GCTRL_0_SCONMET_SHIFT,
			PCE_GCTRL_0_SCONMET_SIZE, parm->nMeterId);
	}
	/* Meter instances used for broadcast traffic */
	gsw_w32(cdev, PCE_GCTRL_0_SCONBC_OFFSET,
		PCE_GCTRL_0_SCONBC_SHIFT,
		PCE_GCTRL_0_SCONBC_SIZE, parm->bBroadcast);
	/* Meter instances used for multicast traffic */
	gsw_w32(cdev, PCE_GCTRL_0_SCONMC_OFFSET,
		PCE_GCTRL_0_SCONMC_SHIFT,
		PCE_GCTRL_0_SCONMC_SIZE, parm->bMulticast);
	/* Meter instances used for unknown unicast traffic */
	gsw_w32(cdev, PCE_GCTRL_0_SCONUC_OFFSET,
		PCE_GCTRL_0_SCONUC_SHIFT,
		PCE_GCTRL_0_SCONUC_SIZE, parm->bUnknownUnicast);
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_StormCfgGet(void *cdev,
	GSW_QoS_stormCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/*  Storm Control Mode  */
	gsw_r32(cdev, PCE_GCTRL_0_SCONMOD_OFFSET,
		PCE_GCTRL_0_SCONMOD_SHIFT,
		PCE_GCTRL_0_SCONMOD_SIZE, &value);
	if (value == 0) {
		parm->nMeterId = 0;
		parm->bBroadcast = 0;
		parm->bMulticast = 0;
		parm->bUnknownUnicast =  0;
	} else {
		gsw_r32(cdev, PCE_GCTRL_0_SCONMET_OFFSET,
			PCE_GCTRL_0_SCONMET_SHIFT,
			PCE_GCTRL_0_SCONMET_SIZE, &value);
		parm->nMeterId = value;
		/* Meter instances used for broadcast traffic */
		gsw_r32(cdev, PCE_GCTRL_0_SCONBC_OFFSET,
			PCE_GCTRL_0_SCONBC_SHIFT,
			PCE_GCTRL_0_SCONBC_SIZE, &value);
		parm->bBroadcast = value;
		/* Meter instances used for multicast traffic */
		gsw_r32(cdev, PCE_GCTRL_0_SCONMC_OFFSET,
			PCE_GCTRL_0_SCONMC_SHIFT,
			PCE_GCTRL_0_SCONMC_SIZE, &value);
		parm->bMulticast = value;
		/* Meter instances used for unknown unicast traffic */
		gsw_r32(cdev, PCE_GCTRL_0_SCONUC_OFFSET,
			PCE_GCTRL_0_SCONUC_SHIFT,
			PCE_GCTRL_0_SCONUC_SIZE, &value);
		parm->bUnknownUnicast = value;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_WredCfgGet(void *cdev,
	GSW_QoS_WRED_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Description: 'Drop Probability Profile' */
	gsw_r32(cdev, BM_QUEUE_GCTRL_DPROB_OFFSET,
		BM_QUEUE_GCTRL_DPROB_SHIFT,
		BM_QUEUE_GCTRL_DPROB_SIZE, &parm->eProfile);
	/*Automatic/Manual - Adaptive Watermark Type */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, BM_QUEUE_GCTRL_BUFMOD_OFFSET,
			BM_QUEUE_GCTRL_BUFMOD_SHIFT,
			BM_QUEUE_GCTRL_BUFMOD_SIZE, &parm->eMode);
	}
	/* Get the Local/Global threshold */
	gsw_r32(cdev, BM_QUEUE_GCTRL_GL_MOD_OFFSET,
		BM_QUEUE_GCTRL_GL_MOD_SHIFT,
		BM_QUEUE_GCTRL_GL_MOD_SIZE, &parm->eThreshMode);
	/* WRED Red Threshold - Minimum */
	gsw_r32(cdev, BM_WRED_RTH_0_MINTH_OFFSET,
		BM_WRED_RTH_0_MINTH_SHIFT,
		BM_WRED_RTH_0_MINTH_SIZE, &parm->nRed_Min);
	/* WRED Red Threshold - Maximum */
	gsw_r32(cdev, BM_WRED_RTH_1_MAXTH_OFFSET,
		BM_WRED_RTH_1_MAXTH_SHIFT,
		BM_WRED_RTH_1_MAXTH_SIZE, &parm->nRed_Max);
	/* WRED Yellow Threshold - Minimum */
	gsw_r32(cdev, BM_WRED_YTH_0_MINTH_OFFSET,
		BM_WRED_YTH_0_MINTH_SHIFT,
		BM_WRED_YTH_0_MINTH_SIZE, &parm->nYellow_Min);
	/* WRED Yellow Threshold - Maximum */
	gsw_r32(cdev, BM_WRED_YTH_1_MAXTH_OFFSET,
		BM_WRED_YTH_1_MAXTH_SHIFT,
		BM_WRED_YTH_1_MAXTH_SIZE, &parm->nYellow_Max);
	/* WRED Green Threshold - Minimum */
	gsw_r32(cdev, BM_WRED_GTH_0_MINTH_OFFSET,
		BM_WRED_GTH_0_MINTH_SHIFT,
		BM_WRED_GTH_0_MINTH_SIZE, &parm->nGreen_Min);
	/* WRED Green Threshold - Maximum */
	gsw_r32(cdev, BM_WRED_GTH_1_MAXTH_OFFSET,
		BM_WRED_GTH_1_MAXTH_SHIFT,
		BM_WRED_GTH_1_MAXTH_SIZE, &parm->nGreen_Max);
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_WredCfgSet(void *cdev,
	GSW_QoS_WRED_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Description: 'Drop Probability Profile' */
	gsw_w32(cdev, BM_QUEUE_GCTRL_DPROB_OFFSET,
		BM_QUEUE_GCTRL_DPROB_SHIFT,
		BM_QUEUE_GCTRL_DPROB_SIZE, parm->eProfile);
	/*Automatic/Manual - Adaptive Watermark Type */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, BM_QUEUE_GCTRL_BUFMOD_OFFSET,
			BM_QUEUE_GCTRL_BUFMOD_SHIFT,
			BM_QUEUE_GCTRL_BUFMOD_SIZE, parm->eMode);
	}
	switch (parm->eThreshMode) {
	case GSW_QOS_WRED_Local_Thresh:
		/* Set the Local threshold */
		gsw_w32(cdev, BM_QUEUE_GCTRL_GL_MOD_OFFSET,
			BM_QUEUE_GCTRL_GL_MOD_SHIFT,
			BM_QUEUE_GCTRL_GL_MOD_SIZE, 0x0);
		break;
	case GSW_QOS_WRED_Global_Thresh:
		/* Set the global threshold */
		gsw_w32(cdev, BM_QUEUE_GCTRL_GL_MOD_OFFSET,
			BM_QUEUE_GCTRL_GL_MOD_SHIFT,
			BM_QUEUE_GCTRL_GL_MOD_SIZE, 0x1);
		break;
	default:
		/* Set the Local threshold */
		gsw_w32(cdev, BM_QUEUE_GCTRL_GL_MOD_OFFSET,
			BM_QUEUE_GCTRL_GL_MOD_SHIFT,
			BM_QUEUE_GCTRL_GL_MOD_SIZE, 0x0);
	}
	/* WRED Red Threshold - Minimum */
	gsw_w32(cdev, BM_WRED_RTH_0_MINTH_OFFSET,
		BM_WRED_RTH_0_MINTH_SHIFT,
		BM_WRED_RTH_0_MINTH_SIZE, parm->nRed_Min);
	/* WRED Red Threshold - Maximum */
	gsw_w32(cdev, BM_WRED_RTH_1_MAXTH_OFFSET,
		BM_WRED_RTH_1_MAXTH_SHIFT,
		BM_WRED_RTH_1_MAXTH_SIZE, parm->nRed_Max);
	/* WRED Yellow Threshold - Minimum */
	gsw_w32(cdev, BM_WRED_YTH_0_MINTH_OFFSET,
		BM_WRED_YTH_0_MINTH_SHIFT,
		BM_WRED_YTH_0_MINTH_SIZE, parm->nYellow_Min);
	/* WRED Yellow Threshold - Maximum */
	gsw_w32(cdev, BM_WRED_YTH_1_MAXTH_OFFSET,
		BM_WRED_YTH_1_MAXTH_SHIFT,
		BM_WRED_YTH_1_MAXTH_SIZE, parm->nYellow_Max);
	/* WRED Green Threshold - Minimum */
	gsw_w32(cdev, BM_WRED_GTH_0_MINTH_OFFSET,
		BM_WRED_GTH_0_MINTH_SHIFT,
		BM_WRED_GTH_0_MINTH_SIZE, parm->nGreen_Min);
	/* WRED Green Threshold - Maximum */
	gsw_w32(cdev, BM_WRED_GTH_1_MAXTH_OFFSET,
		BM_WRED_GTH_1_MAXTH_SHIFT,
		BM_WRED_GTH_1_MAXTH_SIZE, parm->nGreen_Max);
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_WredQueueCfgGet(void *cdev,
	GSW_QoS_WRED_QueueCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 qid = parm->nQueueId, value, addr, data0, data1;
	u8 crcode;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (qid >= gswdev->num_of_queues)
		return GSW_statusErr;

	/* For different color 0(not drop) 1(Green) 2(Yellow) 3(Red) */
	for (crcode = 1; crcode < 4; crcode++) {
		addr = ((qid << 3) | crcode);
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE, addr);
		gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
			BM_RAM_CTRL_ADDR_SHIFT,
			BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Write */
		gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
			BM_RAM_CTRL_OPMOD_SHIFT,
			BM_RAM_CTRL_OPMOD_SIZE, 0x0);
		value = 1;
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE, &data0);
		gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
			BM_RAM_VAL_1_VAL1_SHIFT,
			BM_RAM_VAL_1_VAL1_SIZE, &data1);
		switch (crcode) {
		case 3:
			parm->nRed_Max = data1;
			parm->nRed_Min = data0;
			break;
		case 2:
			parm->nYellow_Max = data1;
			parm->nYellow_Min = data0;
			break;
		case 1:
			parm->nGreen_Max = data1;
			parm->nGreen_Min = data0;
			break;
		case 0:
			break;
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_WredQueueCfgSet(void *cdev,
	GSW_QoS_WRED_QueueCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 qid = parm->nQueueId, value, addr, data0 = 0, data1 = 0;
	u8 crcode;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (qid >= gswdev->num_of_queues)
		return GSW_statusErr;

	/* For different color 0(not drop) 1(Green) 2(Yellow) 3(Red) */
	for (crcode = 1; crcode < 4; crcode++) {
		addr = (((qid << 3) & 0xF8) | crcode);
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE, addr);
		/* Specify the PQMCTXT = 0x9 */
		gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
			BM_RAM_CTRL_ADDR_SHIFT,
			BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Write */
		gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
			BM_RAM_CTRL_OPMOD_SHIFT,
			BM_RAM_CTRL_OPMOD_SIZE, 0x1);
		switch (crcode) {
		case 3:
			data1 = parm->nRed_Max;
			data0 = parm->nRed_Min;
			break;
		case 2:
			data1 = parm->nYellow_Max;
			data0 = parm->nYellow_Min;
			break;
		case 1:
			data1 = parm->nGreen_Max;
			data0 = parm->nGreen_Min;
				break;
		case 0:
			data0 = 0; data1 = 0;
			break;
		}
		gsw_w32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE, data0);
		gsw_w32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
			BM_RAM_VAL_1_VAL1_SHIFT,
			BM_RAM_VAL_1_VAL1_SIZE, data1);
		value = 1;
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
	}
	/* Set the local threshold */
/*	gsw_w32(cdev, BM_QUEUE_GCTRL_GL_MOD_OFFSET, */
/*		BM_QUEUE_GCTRL_GL_MOD_SHIFT, */
/*		BM_QUEUE_GCTRL_GL_MOD_SIZE, 0); */
	return GSW_statusOk;
}

GSW_return_t GSW_QoS_WredPortCfgGet(void *cdev,
	GSW_QoS_WRED_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= (gswdev->tpnum - 1))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns with*/
	/*  an error for older hardware revisions. */
	/** WRED Red Threshold Min [number of segments].*/
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, (BM_PWRED_RTH_0_MINTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_RTH_0_MINTH_SHIFT,
			BM_PWRED_RTH_0_MINTH_SIZE, &parm->nRed_Min);
	/** WRED Red Threshold Max [number of segments].*/
		gsw_r32(cdev, (BM_PWRED_RTH_1_MAXTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_RTH_1_MAXTH_SHIFT,
			BM_PWRED_RTH_1_MAXTH_SIZE, &parm->nRed_Max);
	/* WRED Yellow Threshold - Minimum */
		gsw_r32(cdev, (BM_PWRED_YTH_0_MINTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_YTH_0_MINTH_SHIFT,
			BM_PWRED_YTH_0_MINTH_SIZE, &parm->nYellow_Min);
	/* WRED Yellow Threshold - Maximum */
		gsw_r32(cdev, (BM_PWRED_YTH_1_MAXTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_YTH_1_MAXTH_SHIFT,
			BM_PWRED_YTH_1_MAXTH_SIZE, &parm->nYellow_Max);
	/* WRED Green Threshold - Minimum */
		gsw_r32(cdev, (BM_PWRED_GTH_0_MINTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_GTH_0_MINTH_SHIFT,
			BM_PWRED_GTH_0_MINTH_SIZE, &parm->nGreen_Min);
	/* WRED Green Threshold - Maximum */
		gsw_r32(cdev, (BM_PWRED_GTH_1_MAXTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_GTH_1_MAXTH_SHIFT,
			BM_PWRED_GTH_1_MAXTH_SIZE, &parm->nGreen_Max);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_WredPortCfgSet(void *cdev,
	GSW_QoS_WRED_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= (gswdev->tpnum - 1))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns */
	/* with an error for older hardware revisions. */
	/** WRED Red Threshold Min [number of segments]. */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_w32(cdev, (BM_PWRED_RTH_0_MINTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_RTH_0_MINTH_SHIFT,
			BM_PWRED_RTH_0_MINTH_SIZE, parm->nRed_Min);
	/** WRED Red Threshold Max [number of segments]. */
		gsw_w32(cdev, (BM_PWRED_RTH_1_MAXTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_RTH_1_MAXTH_SHIFT,
			BM_PWRED_RTH_1_MAXTH_SIZE, parm->nRed_Max);
	/* WRED Yellow Threshold - Minimum */
		gsw_w32(cdev, (BM_PWRED_YTH_0_MINTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_YTH_0_MINTH_SHIFT,
			BM_PWRED_YTH_0_MINTH_SIZE, parm->nYellow_Min);
	/* WRED Yellow Threshold - Maximum */
		gsw_w32(cdev, (BM_PWRED_YTH_1_MAXTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_YTH_1_MAXTH_SHIFT,
			BM_PWRED_YTH_1_MAXTH_SIZE, parm->nYellow_Max);
	/* WRED Green Threshold - Minimum */
		gsw_w32(cdev, (BM_PWRED_GTH_0_MINTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_GTH_0_MINTH_SHIFT,
			BM_PWRED_GTH_0_MINTH_SIZE, parm->nGreen_Min);
	/* WRED Green Threshold - Maximum */
		gsw_w32(cdev, (BM_PWRED_GTH_1_MAXTH_OFFSET
			+ (parm->nPortId * 0x6)),
			BM_PWRED_GTH_1_MAXTH_SHIFT,
			BM_PWRED_GTH_1_MAXTH_SIZE, parm->nGreen_Max);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_FlowctrlCfgGet(void *cdev,
	GSW_QoS_FlowCtrlCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Supported for GSWIP 2.2 and newer and returns */
	/* with an error for older hardware revisions. */
	/** Global Buffer Non Conforming Flow Control */
	/*  Threshold Minimum [number of segments]. */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, (SDMA_FCTHR1_THR1_OFFSET),
			SDMA_FCTHR1_THR1_SHIFT,
			SDMA_FCTHR1_THR1_SIZE,
			&parm->nFlowCtrlNonConform_Min);
	/** Global Buffer Non Conforming Flow Control */
	/* Threshold Maximum [number of segments]. */
		gsw_r32(cdev, (SDMA_FCTHR2_THR2_OFFSET),
			SDMA_FCTHR2_THR2_SHIFT,
			SDMA_FCTHR2_THR2_SIZE,
			&parm->nFlowCtrlNonConform_Max);
	/** Global Buffer Conforming Flow Control Threshold */
	/*  Minimum [number of segments]. */
		gsw_r32(cdev, (SDMA_FCTHR3_THR3_OFFSET),
			SDMA_FCTHR3_THR3_SHIFT,
			SDMA_FCTHR3_THR3_SIZE,
			&parm->nFlowCtrlConform_Min);
	/** Global Buffer Conforming Flow Control */
	/* Threshold Maximum [number of segments]. */
		gsw_r32(cdev, (SDMA_FCTHR4_THR4_OFFSET),
			SDMA_FCTHR4_THR4_SHIFT,
			SDMA_FCTHR4_THR4_SIZE,
			&parm->nFlowCtrlConform_Max);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_FlowctrlCfgSet(void *cdev,
	GSW_QoS_FlowCtrlCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Supported for GSWIP 2.2 and newer and returns with */
	/* an error for older hardware revisions. */
	/** Global Buffer Non Conforming Flow Control */
	/* Threshold Minimum [number of segments]. */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_w32(cdev, (SDMA_FCTHR1_THR1_OFFSET),
			SDMA_FCTHR1_THR1_SHIFT,
			SDMA_FCTHR1_THR1_SIZE,
			parm->nFlowCtrlNonConform_Min);
	/** Global Buffer Non Conforming Flow Control */
	/* Threshold Maximum [number of segments]. */
		gsw_w32(cdev, (SDMA_FCTHR2_THR2_OFFSET),
			SDMA_FCTHR2_THR2_SHIFT,
			SDMA_FCTHR2_THR2_SIZE,
			parm->nFlowCtrlNonConform_Max);
	/** Global Buffer Conforming Flow Control Threshold */
	/*  Minimum [number of segments]. */
		gsw_w32(cdev, (SDMA_FCTHR3_THR3_OFFSET),
			SDMA_FCTHR3_THR3_SHIFT,
			SDMA_FCTHR3_THR3_SIZE,
			parm->nFlowCtrlConform_Min);
	/** Global Buffer Conforming Flow Control Threshold */
	/*  Maximum [number of segments]. */
		gsw_w32(cdev, (SDMA_FCTHR4_THR4_OFFSET),
			SDMA_FCTHR4_THR4_SHIFT,
			SDMA_FCTHR4_THR4_SIZE,
			parm->nFlowCtrlConform_Max);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_FlowctrlPortCfgGet(void *cdev,
	GSW_QoS_FlowCtrlPortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= (gswdev->tpnum - 1))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns with */
	/* an error for older hardware revisions. */
	/** Ingress Port occupied Buffer Flow Control */
	/* Threshold Minimum [number of segments]. */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC)) {
		gsw_r32(cdev, (SDMA_PFCTHR8_THR8_OFFSET22
			+ (parm->nPortId * 0x4)),
			SDMA_PFCTHR8_THR8_SHIFT,
			SDMA_PFCTHR8_THR8_SIZE, &parm->nFlowCtrl_Min);
	/** Ingress Port occupied Buffer Flow Control */
	/* Threshold Maximum [number of segments]. */
		gsw_r32(cdev, (SDMA_PFCTHR9_THR9_OFFSET22
			+ (parm->nPortId * 0x4)),
			SDMA_PFCTHR9_THR9_SHIFT,
			SDMA_PFCTHR9_THR9_SIZE, &parm->nFlowCtrl_Max);
		return GSW_statusOk;
	} else if (gswdev->gipver == LTQ_GSWIP_3_0) {
	/** Ingress Port occupied Buffer Flow Control */
	/* Threshold Minimum [number of segments]. */
		gsw_r32(cdev, (SDMA_PFCTHR8_THR8_OFFSET30
			+ (parm->nPortId * 0x2)),
			SDMA_PFCTHR8_THR8_SHIFT,
			SDMA_PFCTHR8_THR8_SIZE, &parm->nFlowCtrl_Min);
	/** Ingress Port occupied Buffer Flow Control */
	/* Threshold Maximum [number of segments]. */
		gsw_r32(cdev, (SDMA_PFCTHR9_THR9_OFFSET30
			+ (parm->nPortId * 0x2)),
			SDMA_PFCTHR9_THR9_SHIFT,
			SDMA_PFCTHR9_THR9_SIZE,
			&parm->nFlowCtrl_Max);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_FlowctrlPortCfgSet(void *cdev,
	GSW_QoS_FlowCtrlPortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= (gswdev->tpnum -1 ))
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns with */
	/*  an error for older hardware revisions. */
	/** Ingress Port occupied Buffer Flow Control */
	/* Threshold Minimum [number of segments]. */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC)) {
		gsw_w32(cdev, (SDMA_PFCTHR8_THR8_OFFSET22
			+ (parm->nPortId * 0x4)),
			SDMA_PFCTHR8_THR8_SHIFT,
			SDMA_PFCTHR8_THR8_SIZE, parm->nFlowCtrl_Min);
	/** Ingress Port occupied Buffer Flow Control Threshold */
	/* Maximum [number of segments]. */
		gsw_w32(cdev, (SDMA_PFCTHR9_THR9_OFFSET22
			+ (parm->nPortId * 0x4)),
			SDMA_PFCTHR9_THR9_SHIFT,
			SDMA_PFCTHR9_THR9_SIZE, parm->nFlowCtrl_Max);
		return GSW_statusOk;
	} else if (gswdev->gipver == LTQ_GSWIP_3_0) {
	/** Ingress Port occupied Buffer Flow Control Threshold*/
	/*  Minimum [number of segments]. */
		gsw_w32(cdev, (SDMA_PFCTHR8_THR8_OFFSET30
			+ (parm->nPortId * 0x2)),
			SDMA_PFCTHR8_THR8_SHIFT,
			SDMA_PFCTHR8_THR8_SIZE, parm->nFlowCtrl_Min);
	/** Ingress Port occupied Buffer Flow Control Threshold */
	/*  Maximum [number of segments]. */
		gsw_w32(cdev, (SDMA_PFCTHR9_THR9_OFFSET30
			+ (parm->nPortId * 0x2)),
			SDMA_PFCTHR9_THR9_SHIFT,
			SDMA_PFCTHR9_THR9_SIZE, parm->nFlowCtrl_Max);
		return GSW_statusOk;
	}
	 else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_QueueBufferReserveCfgGet(void *cdev,
	GSW_QoS_QueueBufferReserveCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32  value, addr;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (parm->nQueueId >= gswdev->num_of_queues)
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns with an error */
	/*  for older hardware revisions. */
	/* Colourcode = 0  and fixed offset = 0 */
	/* Set the BM RAM ADDR  */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		addr = (((parm->nQueueId << 3) & 0xF8));
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE, addr);
		/* Specify the PQMCTXT = 0x9 */
		gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
			BM_RAM_CTRL_ADDR_SHIFT,
			BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Read */
		gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
			BM_RAM_CTRL_OPMOD_SHIFT,
			BM_RAM_CTRL_OPMOD_SIZE, 0x0);
		value = 1;
		/* Active */
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);

		gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE, &parm->nBufferReserved);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}

GSW_return_t GSW_QoS_QueueBufferReserveCfgSet(void *cdev,
	GSW_QoS_QueueBufferReserveCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32  value, addr;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	gsw_r32(cdev, ETHSW_CAP_1_QUEUE_OFFSET, */
/*		ETHSW_CAP_1_QUEUE_SHIFT, */
/*		ETHSW_CAP_1_QUEUE_SIZE, &value); */

	if (parm->nQueueId >= gswdev->num_of_queues)
		return GSW_statusErr;
	/* Supported for GSWIP 2.2 and newer and returns with an error */
	/* for older hardware revisions. */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		/* Colourcode = 0  and fixed offset = 0 */
		/* Set the BM RAM ADDR  */
		addr = (((parm->nQueueId << 3) & 0xF8));
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE, addr);
		gsw_w32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE,
			parm->nBufferReserved);
		/* Specify the PQMCTXT = 0x9 */
		gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
			BM_RAM_CTRL_ADDR_SHIFT,
			BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Write */
		gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
			BM_RAM_CTRL_OPMOD_SHIFT,
			BM_RAM_CTRL_OPMOD_SIZE, 0x1);
		value = 1;
		/* Active */
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		return GSW_statusOk;
	} else {
		return GSW_statusErr;
	}
}
GSW_return_t GSW_QoS_Meter_Act(void *cdev,
	GSW_QoS_mtrAction_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32  value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (parm->nCpuUserId >= 2)
			return GSW_statusErr;
		value = parm->nMeterId;
		gsw_w32(cdev, (PCE_CPUMETER_MID0_MID_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_MID0_MID_SHIFT,
			PCE_CPUMETER_MID0_MID_SIZE, value);
		gsw_w32(cdev, (PCE_CPUMETER_CTRL_MT0EN_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_CTRL_MT0EN_SHIFT,
			PCE_CPUMETER_CTRL_MT0EN_SIZE, parm->bMeterEna);

		value = parm->nSecMeterId;
		gsw_w32(cdev, (PCE_CPUMETER_MID1_MID_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_MID1_MID_SHIFT,
			PCE_CPUMETER_MID1_MID_SIZE, value);
		gsw_w32(cdev, (PCE_CPUMETER_CTRL_MT1EN_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_CTRL_MT1EN_SHIFT,
			PCE_CPUMETER_CTRL_MT1EN_SIZE, parm->bSecMeterEna);

		value = parm->pktLen;
		gsw_w32(cdev, (PCE_CPUMETER_SIZE_SIZE_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_SIZE_SIZE_SHIFT,
			PCE_CPUMETER_SIZE_SIZE_SIZE, value);

		gsw_w32(cdev, (PCE_CPUMETER_CTRL_PRECOL_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_CTRL_PRECOL_SHIFT,
			PCE_CPUMETER_CTRL_PRECOL_SIZE, parm->ePreColor);
/*
		gsw_w32(cdev, (PCE_CPUMETER_CTRL_REQ_OFFSET
			+ (parm->nCpuUserId * 0x8)),
			PCE_CPUMETER_CTRL_REQ_SHIFT,
			PCE_CPUMETER_CTRL_REQ_SIZE, 1);

		do {
			gsw_r32(cdev, (PCE_CPUMETER_CTRL_REQ_OFFSET
				+ (parm->nCpuUserId * 0x8)),
				PCE_CPUMETER_CTRL_REQ_SHIFT,
				PCE_CPUMETER_CTRL_REQ_SIZE, &value);
		} while(value);

		gsw_w32(cdev, PCE_CPUMETER_CTRL_AFTCOL_OFFSET
			+ (parm->nCpuUserId * 0x8),
			PCE_CPUMETER_CTRL_AFTCOL_SHIFT,
			PCE_CPUMETER_CTRL_AFTCOL_SIZE, parm->eOutColor);
*/
	}
	return 0;
}
#endif  /* CONFIG_LTQ_QOS */
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
GSW_return_t GSW_MulticastRouterPortAdd(void *cdev,
	GSW_multicastRouter_t *parm)
{
	ethsw_api_dev_t	*gswdev = (ethsw_api_dev_t *)cdev;
	gsw_igmp_t *hitbl = &gswdev->iflag;
	u32 value;
	u8 pidx = parm->nPortId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	/* Read the Default Router Port Map - DRPM */
	gsw_r32(cdev, PCE_IGMP_DRPM_DRPM_OFFSET,
		PCE_IGMP_DRPM_DRPM_SHIFT,
		PCE_IGMP_DRPM_DRPM_SIZE, &value);
	if (((value >> pidx) & 0x1) == 1) {
		pr_err("Error: the prot was already in the member\n");
	} else {
		value = (value | (1 << pidx));
		/* Write the Default Router Port Map - DRPM  */
		gsw_w32(cdev, PCE_IGMP_DRPM_DRPM_OFFSET,
			PCE_IGMP_DRPM_DRPM_SHIFT,
			PCE_IGMP_DRPM_DRPM_SIZE, value);
	}
	if (hitbl->igmode == GSW_MULTICAST_SNOOP_MODE_FORWARD) {
		GSW_PCE_rule_t pcrule;
		int i;
		for (i = 0; i < 2; i++) {
			memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
			pcrule.pattern.bEnable = 1;
			pcrule.pattern.bProtocolEnable = 1;
			switch (i) {
			case 0:
	/*	Management port remaining IGMP packets (forwarding */
	/* them to Router Ports) */
				pcrule.pattern.nIndex = MPCE_RULES_INDEX;
				pcrule.pattern.nProtocol = 0x2; /* for IPv4 */
				pcrule.pattern.bAppMaskRangeMSB_Select = 1;
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.nAppDataMSB = 0x1200;
				pcrule.pattern.nAppMaskRangeMSB	= 0x1DFF;
				break;
			case 1:
	/* Management Port ICMPv6 Multicast Listerner Report */
	/* & Leave (Avoiding Loopback abd Discard) */
				pcrule.pattern.nIndex = MPCE_RULES_INDEX + 3;
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.bAppMaskRangeMSB_Select = 1;
				pcrule.pattern.nAppDataMSB = 0x8300;
				pcrule.pattern.nAppMaskRangeMSB	= 0x1FF;
				pcrule.pattern.nProtocol = 0x3A;  /*for IPv6*/
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
				pcrule.action.nForwardPortMap = value;
				break;
			}
			/* Router portmap */
			pcrule.action.ePortMapAction =
			GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
			pcrule.action.nForwardPortMap = value;
			if (hitbl->igcos != 0) {
				pcrule.action.eTrafficClassAction = 1;
				pcrule.action.nTrafficClassAlternate = gswdev->iflag.igcos;
			}
			/*  Set eForwardPort */
			pcrule.pattern.bPortIdEnable = 1;
			if (hitbl->igfport == GSW_PORT_FORWARD_PORT)
				pcrule.pattern.nPortId = hitbl->igfpid;
			else if (hitbl->igfport == GSW_PORT_FORWARD_CPU)
				pcrule.pattern.nPortId = (gswdev->cport);

			if (hitbl->igxvlan)
				pcrule.action.eVLAN_CrossAction =
				GSW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				pcrule.action.eVLAN_CrossAction =
				GSW_PCE_ACTION_CROSS_VLAN_DISABLE;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(gswdev,
				&gswdev->phandler, &pcrule))
				return GSW_statusErr;
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MulticastRouterPortRead(void *cdev,
	GSW_multicastRouterRead_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value_1, value_2;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->bInitial == 1) {
		/* Read the Default Router Port Map - DRPM*/
		gsw_r32(cdev, PCE_IGMP_DRPM_DRPM_OFFSET,
			PCE_IGMP_DRPM_DRPM_SHIFT,
			PCE_IGMP_DRPM_DRPM_SIZE, &value_1);
		/* Read the Default Router Port Map - IGPM */
		gsw_r32(cdev, PCE_IGMP_STAT_IGPM_OFFSET,
			PCE_IGMP_STAT_IGPM_SHIFT,
			PCE_IGMP_STAT_IGPM_SIZE, &value_2);
		gswdev->iflag.igrport = (value_1 | value_2);
		parm->bInitial = 0;
		gswdev->mrtpcnt = 0;
	}
	if (parm->bLast == 0) {
		/* Need to clarify the different between DRPM & IGPM */
		while (((gswdev->iflag.igrport >>
			gswdev->mrtpcnt) & 0x1) == 0) {
			gswdev->mrtpcnt++;
			if (gswdev->mrtpcnt > (gswdev->tpnum-1)) {
				parm->bLast = 1;
				return GSW_statusOk;
			}
		}
		parm->nPortId = gswdev->mrtpcnt;
		if (gswdev->mrtpcnt < gswdev->tpnum)
			gswdev->mrtpcnt++;
		else
			parm->bLast = 1;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MulticastRouterPortRemove(void *cdev,
	GSW_multicastRouter_t *parm)
{
	ethsw_api_dev_t	*gswdev = (ethsw_api_dev_t *)cdev;
	gsw_igmp_t *hitbl	= &gswdev->iflag;
	u32 value_1, value_2;
	u8 pidx = parm->nPortId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	/* Read the Default Router Port Map - DRPM */
	gsw_r32(cdev, PCE_IGMP_DRPM_DRPM_OFFSET,
		PCE_IGMP_DRPM_DRPM_SHIFT,
		PCE_IGMP_DRPM_DRPM_SIZE, &value_1);
	/* Read the Default Router Port Map - IGPM */
	gsw_r32(cdev, PCE_IGMP_STAT_IGPM_OFFSET,
		PCE_IGMP_STAT_IGPM_SHIFT,
		PCE_IGMP_STAT_IGPM_SIZE, &value_2);
	if (((value_1 >> pidx) & 0x1) == 0) {
		pr_err("Error: the port was not in the member\n");
		return GSW_statusOk;
	} else {
	value_1 = (value_1 & ~(1 << pidx));
	/* Write the Default Router Port Map - DRPM*/
	gsw_w32(cdev, PCE_IGMP_DRPM_DRPM_OFFSET,
		PCE_IGMP_DRPM_DRPM_SHIFT,
		PCE_IGMP_DRPM_DRPM_SIZE, value_1);
	}
	if ((hitbl->igmode ==
	GSW_MULTICAST_SNOOP_MODE_FORWARD) & value_1) {
		GSW_PCE_rule_t pcrule;
		int i;
		for (i = 0; i < 2; i++) {
			memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
			pcrule.pattern.bEnable = 1;
			pcrule.pattern.bProtocolEnable = 1;
			switch (i) {
			case 0:
	/*	Management port remaining IGMP packets */
	/*(forwarding them to Router Ports) */
				pcrule.pattern.nIndex = MPCE_RULES_INDEX;
				/* for IPv4 */
				pcrule.pattern.nProtocol = 0x2;
				pcrule.pattern.bAppMaskRangeMSB_Select = 1;
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.nAppDataMSB = 0x1200;
				pcrule.pattern.nAppMaskRangeMSB	= 0x1DFF;
				break;
			case 1:
	/* Management Port ICMPv6 Multicast Listerner Report */
	/* & Leave (Avoiding Loopback abd Discard) */
				pcrule.pattern.nIndex = MPCE_RULES_INDEX+3;
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.bAppMaskRangeMSB_Select = 1;
				pcrule.pattern.nAppDataMSB = 0x8300;
				pcrule.pattern.nAppMaskRangeMSB	= 0x1FF;
				/*for IPv6*/
				pcrule.pattern.nProtocol = 0x3A;
				break;
			}
			/* Router portmap */
			pcrule.action.ePortMapAction =
			GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
			pcrule.action.nForwardPortMap = value_1;
			if (hitbl->igcos != 0) {
				pcrule.action.eTrafficClassAction = 1;
				pcrule.action.nTrafficClassAlternate =
					gswdev->iflag.igcos;
			}
			/*  Set eForwardPort */
			pcrule.pattern.bPortIdEnable = 1;
			if (hitbl->igfport == GSW_PORT_FORWARD_PORT)
				pcrule.pattern.nPortId = hitbl->igfpid;
			else if (hitbl->igfport == GSW_PORT_FORWARD_CPU)
				pcrule.pattern.nPortId = (gswdev->cport);
			if (hitbl->igxvlan)
				pcrule.action.eVLAN_CrossAction =
					GSW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				pcrule.action.eVLAN_CrossAction =
					GSW_PCE_ACTION_CROSS_VLAN_DISABLE;
	/* We prepare everything and write into PCE Table */
			if (pce_rule_write(gswdev,
				&gswdev->phandler, &pcrule) != 0)
				return GSW_statusErr;
		}
	} else if ((hitbl->igmode ==
		GSW_MULTICAST_SNOOP_MODE_FORWARD) & !value_1){
		GSW_PCE_rule_t pcrule;
		int i;
		for (i = 0; i < 2; i++) {
			memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
			switch (i) {
			case 0:
				pcrule.pattern.nIndex = MPCE_RULES_INDEX;
				break;
			case 1:
	/* Management Port ICMPv6 Multicast Listerner Report */
	/* & Leave (Avoiding Loopback abd Discard) */
				pcrule.pattern.nIndex =
				MPCE_RULES_INDEX + 3;
				break;
				}
			if (pce_pattern_delete(cdev, &gswdev->phandler, pcrule.pattern.nIndex) != 0)
				return GSW_statusErr;
		}
	}

	return GSW_statusOk;
}

GSW_return_t GSW_MulticastSnoopCfgGet(void *cdev,
	GSW_multicastSnoopCfg_t *parm)
{
	u32 data_1, data_2, value;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	parm->eIGMP_Mode = gswdev->iflag.igmode;
	parm->bIGMPv3 = gswdev->iflag.igv3;
	parm->bCrossVLAN = gswdev->iflag.igxvlan;
	parm->eForwardPort = gswdev->iflag.igfport;
	parm->nForwardPortId = gswdev->iflag.igfpid;
	parm->nClassOfService = gswdev->iflag.igcos;
	gsw_r32(cdev, PCE_IGMP_CTRL_ROB_OFFSET,
		PCE_IGMP_CTRL_ROB_SHIFT,
		PCE_IGMP_CTRL_ROB_SIZE, &value);
	parm->nRobust = value;
	gsw_r32(cdev, PCE_IGMP_CTRL_DMRT_OFFSET,
		PCE_IGMP_CTRL_DMRT_SHIFT,
		PCE_IGMP_CTRL_DMRT_SIZE, &value);
	parm->nQueryInterval = value;
	gsw_r32(cdev, PCE_IGMP_CTRL_REPSUP_OFFSET,
		PCE_IGMP_CTRL_REPSUP_SHIFT,
		PCE_IGMP_CTRL_REPSUP_SIZE, &data_1);
	gsw_r32(cdev, PCE_IGMP_CTRL_JASUP_OFFSET,
		PCE_IGMP_CTRL_JASUP_SHIFT,
		PCE_IGMP_CTRL_JASUP_SIZE, &data_2);
	if (data_1 == 0 && data_2 == 0)
		parm->eSuppressionAggregation = GSW_MULTICAST_TRANSPARENT;
	else if (data_1 == 1 && data_2 == 0)
		parm->eSuppressionAggregation = GSW_MULTICAST_REPORT;
	else if (data_1 == 1 && data_2 == 1)
		parm->eSuppressionAggregation = GSW_MULTICAST_REPORT_JOIN;
	else
		parm->eSuppressionAggregation = GSW_MULTICAST_TRANSPARENT;

	gsw_r32(cdev, PCE_IGMP_CTRL_FLEAVE_OFFSET,
		PCE_IGMP_CTRL_FLEAVE_SHIFT,
		PCE_IGMP_CTRL_FLEAVE_SIZE, &value);
	if (value == 1)
		parm->bFastLeave = 1;
	else
		parm->bFastLeave = 0;
	gsw_r32(cdev, PCE_IGMP_CTRL_SRPEN_OFFSET,
		PCE_IGMP_CTRL_SRPEN_SHIFT,
		PCE_IGMP_CTRL_SRPEN_SIZE, &value);
	parm->bLearningRouter = value;
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, PCE_GCTRL_1_UKIPMC_OFFSET,
			PCE_GCTRL_1_UKIPMC_SHIFT,
			PCE_GCTRL_1_UKIPMC_SIZE,
			&parm->bMulticastUnknownDrop);
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, PCE_GCTRL_1_MKFIDEN_OFFSET,
			PCE_GCTRL_1_MKFIDEN_SHIFT,
			PCE_GCTRL_1_MKFIDEN_SIZE,
			&parm->bMulticastFIDmode);
	}

	return GSW_statusOk;
}

GSW_return_t GSW_MulticastSnoopCfgSet(void *cdev,
	GSW_multicastSnoopCfg_t *parm)
{
	u32 i, data_1 = 0, data_2 = 0, pmcindex = MPCE_RULES_INDEX;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	GSW_PCE_rule_t pcrule;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Choose IGMP Mode */
	switch (parm->eIGMP_Mode) {
	case GSW_MULTICAST_SNOOP_MODE_DISABLED:
		/* Snooping of Router Port Disable */
		gsw_w32(cdev, PCE_IGMP_CTRL_SRPEN_OFFSET,
			PCE_IGMP_CTRL_SRPEN_SHIFT,
			PCE_IGMP_CTRL_SRPEN_SIZE, 0);
		gsw_w32(cdev, PCE_GCTRL_0_IGMP_OFFSET,
			PCE_GCTRL_0_IGMP_SHIFT,
			PCE_GCTRL_0_IGMP_SIZE, 0);
		for (i = 0; i <= gswdev->tpnum; i++) {
			gsw_w32(cdev, PCE_PCTRL_0_MCST_OFFSET + (0xA * i),
				PCE_PCTRL_0_MCST_SHIFT,
				PCE_PCTRL_0_MCST_SIZE, 0);
		}
		break;
	case GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING:
		/* Snooping of Router Port Enable */
		gsw_w32(cdev, PCE_GCTRL_0_IGMP_OFFSET,
			PCE_GCTRL_0_IGMP_SHIFT,
			PCE_GCTRL_0_IGMP_SIZE, 0);
		gsw_w32(cdev, PCE_IGMP_CTRL_SRPEN_OFFSET,
			PCE_IGMP_CTRL_SRPEN_SHIFT,
			PCE_IGMP_CTRL_SRPEN_SIZE, 1);
		for (i = 0; i <= gswdev->tpnum; i++) {
			gsw_w32(cdev, PCE_PCTRL_0_MCST_OFFSET + (0xA * i),
				PCE_PCTRL_0_MCST_SHIFT,
				PCE_PCTRL_0_MCST_SIZE, 1);
		}
		break;
	case GSW_MULTICAST_SNOOP_MODE_FORWARD:
		/* Snooping of Router Port Forward */
		gsw_w32(cdev, PCE_IGMP_CTRL_SRPEN_OFFSET,
			PCE_IGMP_CTRL_SRPEN_SHIFT,
			PCE_IGMP_CTRL_SRPEN_SIZE, 0);
		gsw_w32(cdev, PCE_GCTRL_0_IGMP_OFFSET,
			PCE_GCTRL_0_IGMP_SHIFT,
			PCE_GCTRL_0_IGMP_SIZE, 1);
		for (i = 0; i <= gswdev->tpnum; i++) {
			gsw_w32(cdev, PCE_PCTRL_0_MCST_OFFSET + (0xA * i),
				PCE_PCTRL_0_MCST_SHIFT,
				PCE_PCTRL_0_MCST_SIZE, 1);
		}
		break;
	default:
			pr_err("This Mode doesn't exists\n");
			return GSW_statusErr;
	}
	/* Set the Flag for eIGMP_Mode flag*/
	gswdev->iflag.igmode = parm->eIGMP_Mode;
	/* Set bIGMPv3 flag*/
	gswdev->iflag.igv3 =  parm->bIGMPv3;
	/* Set bCrossVLAN flag*/
	gswdev->iflag.igxvlan = parm->bCrossVLAN;
	/* Set eForwardPort flag */
	gswdev->iflag.igfport = parm->eForwardPort;
	/* Set nForwardPortId */
	if (parm->eForwardPort == GSW_PORT_FORWARD_CPU)
		gswdev->iflag.igfpid = (1 << gswdev->cport);
	else
		gswdev->iflag.igfpid = parm->nForwardPortId;
	gswdev->iflag.igcos = parm->nClassOfService;
/* If IGMP mode set to AutoLearning then the following Rule have to add it */
	if (parm->eIGMP_Mode ==
		GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		for (i = pmcindex; i <= (pmcindex + 7); i++) {
			memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
			pcrule.pattern.nIndex = i;
			pcrule.pattern.bEnable = 1;
			pcrule.pattern.bAppDataMSB_Enable = 1;
			if ((i == pmcindex + 0) ||
				(i == pmcindex + 1) ||
				(i == pmcindex + 2))
				pcrule.pattern.nAppDataMSB = 0x1100;
			else if (i == pmcindex + 3)
				pcrule.pattern.nAppDataMSB = 0x1200;
			else if (i == pmcindex + 4)
				pcrule.pattern.nAppDataMSB = 0x1600;
			else if (i == pmcindex + 5)
				pcrule.pattern.nAppDataMSB = 0x1700;
			else if (i == pmcindex + 6)
				pcrule.pattern.nAppDataMSB = 0x3100;
			else if (i == pmcindex + 7)
				pcrule.pattern.nAppDataMSB = 0x3000;

			pcrule.pattern.bAppMaskRangeMSB_Select = 0;
/*			if (gswdev->gipver == LTQ_GSWIP_3_0) { */
/*				pcrule.pattern.nAppMaskRangeMSB = 0x0; */
/*			} else { */
				pcrule.pattern.nAppMaskRangeMSB = 0x3;
	/*		} */
			if ((i == pmcindex + 0) ||
				(i == pmcindex + 1) ||
				(i == pmcindex + 6) ||
				(i == pmcindex + 7))
				pcrule.pattern.eDstIP_Select = 1;
			if ((i == pmcindex + 0) ||
				(i == pmcindex + 1))
				pcrule.pattern.nDstIP.nIPv4 = 0xE0000001;
			else if (i == pmcindex + 6)
				pcrule.pattern.nDstIP.nIPv4 = 0xE0000002;
			else if (i == pmcindex + 7)
				pcrule.pattern.nDstIP.nIPv4 = 0xE00000A6;
			pcrule.pattern.nDstIP_Mask = 0xFF00;
			if (i == pmcindex + 1)
				pcrule.pattern.eSrcIP_Select = 1;
			else
				pcrule.pattern.eSrcIP_Select = 0;
			if (i == pmcindex + 1)
				pcrule.pattern.nSrcIP_Mask = 0xFF00;
			else
				pcrule.pattern.nSrcIP_Mask = 0xFFFF;
			pcrule.pattern.bProtocolEnable = 1;
			pcrule.pattern.nProtocol = 0x2;
			if (gswdev->iflag.igcos == 0) {
				pcrule.action.eTrafficClassAction = 0;
				pcrule.action.nTrafficClassAlternate = 0;
			} else {
				pcrule.action.eTrafficClassAction = 1;
				pcrule.action.nTrafficClassAlternate =
					gswdev->iflag.igcos;
			}
			if (i == pmcindex + 0)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_QUERY;
			else if (i == pmcindex + 1)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_QUERY_NO_ROUTER;
			else if (i == pmcindex + 2)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_QUERY_GROUP;
			else if (i == pmcindex + 3)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_REPORT;
			else if (i == pmcindex + 4)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_REPORT;
			else if (i == pmcindex + 5)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_LEAVE;
			else if (i == pmcindex + 6)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_AD;
			else if (i == pmcindex + 7)
				pcrule.action.eSnoopingTypeAction =
				GSW_PCE_ACTION_IGMP_SNOOP_AD;
			pcrule.action.ePortMapAction =
			GSW_PCE_ACTION_PORTMAP_MULTICAST_ROUTER;
			if (parm->bCrossVLAN)
				pcrule.action.eVLAN_CrossAction =
				GSW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				pcrule.action.eVLAN_CrossAction =
				GSW_PCE_ACTION_CROSS_VLAN_DISABLE;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(gswdev,
				&gswdev->phandler, &pcrule))
				return GSW_statusErr;
		}
	}
	/* If IGMP mode set to forwarding then the */
	/* following Rule have to add it */
	if (parm->eIGMP_Mode == GSW_MULTICAST_SNOOP_MODE_FORWARD) {
		for (i = pmcindex; i <= (pmcindex + 7); i++) {
			memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
			pcrule.pattern.nIndex = i;
			pcrule.pattern.bEnable = 1;
			pcrule.pattern.bProtocolEnable = 1;
			switch (i - pmcindex) {
/*		case 0: */
/*Rule added by Router port ADD function based on router port for IPv4*/
/*					break; */
			case 1:
/*	Avoid IGMP Packets Redirection when seen on Management Port */
				pcrule.pattern.nProtocol = 0x2; /* for IPv4 */
				pcrule.pattern.bPortIdEnable = 1;
	/* Action Enabled, no redirection (default portmap) */
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_REGULAR;
				break;
			case 2:
				/* IGMPv1/2/3 IPv4 */
				pcrule.pattern.nProtocol = 0x2; /* for IPv4 */
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
				break;
/*		case 3: */
	/*Rules added by Router port ADD function */
	/* based on router port for IPv6 */
/*			break; */
			case 4:
	/*	Managemnt Port Remaining ICMPv6/MLD packets */
	/* (Avoiding Loopback and Disacard) */
				pcrule.pattern.bPortIdEnable = 1;
				pcrule.pattern.nPortId = parm->nForwardPortId;
				pcrule.pattern.nProtocol = 0x3A;  /*for IPv6*/
				pcrule.pattern.bPortIdEnable = 1;
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_REGULAR;
				break;
			case 5:
	/* ICMPv6 Multicast Listener Query/Report/Done(Leave) */
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.bAppMaskRangeMSB_Select = 1;
				pcrule.pattern.nAppDataMSB = 0x8200;
				pcrule.pattern.nAppMaskRangeMSB	= 0x2FF;
				pcrule.pattern.nProtocol = 0x3A;  /*for IPv6*/
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
				break;
			case 6:
	/* ICMPv6 Multicast Listener Report */
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.nAppDataMSB = 0x8F00;
				pcrule.pattern.nAppMaskRangeMSB = 0x3;
				pcrule.pattern.nProtocol = 0x3A;  /*for IPv6*/
				pcrule.action.ePortMapAction =
					GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
				break;
			case 7:
	/* ICMPv6 Multicast Router Advertisement/Solicitation/Termination */
				pcrule.pattern.bAppDataMSB_Enable	= 1;
				pcrule.pattern.bAppMaskRangeMSB_Select = 1;
				pcrule.pattern.nAppDataMSB = 0x9700;
				pcrule.pattern.nAppMaskRangeMSB	= 0x2FF;
				pcrule.pattern.nProtocol = 0x3A;  /*for IPv6*/
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
				break;
			default:
				continue;
			}
			if (gswdev->iflag.igcos != 0) {
				pcrule.action.eTrafficClassAction = 1;
				pcrule.action.nTrafficClassAlternate =
				gswdev->iflag.igcos;
			}
			/*  Set eForwardPort */
			if (parm->eForwardPort == GSW_PORT_FORWARD_PORT) {
				pcrule.action.nForwardPortMap =
				(1 << parm->nForwardPortId);
				pcrule.pattern.nPortId = parm->nForwardPortId;
			} else if (parm->eForwardPort == GSW_PORT_FORWARD_CPU) {
				pcrule.action.nForwardPortMap =
				(1 << gswdev->cport);
				pcrule.pattern.nPortId = gswdev->cport;
			}
			if (parm->bCrossVLAN)
				pcrule.action.eVLAN_CrossAction =
				GSW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				pcrule.action.eVLAN_CrossAction =
				GSW_PCE_ACTION_CROSS_VLAN_DISABLE;
			/* We prepare everything and write into PCE Table */
			if (pce_rule_write(gswdev,
				&gswdev->phandler, &pcrule) != 0)
				return GSW_statusErr;
		}

	}
	if (parm->eIGMP_Mode ==
		GSW_MULTICAST_SNOOP_MODE_DISABLED) {
		pmcindex = MPCE_RULES_INDEX;
		for (i = pmcindex; i <= (pmcindex + 7); i++) {
			pcrule.pattern.nIndex = i;
			pcrule.pattern.bEnable = 0;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_pattern_delete(cdev, &gswdev->phandler, i))
				return GSW_statusErr;
		}
	}
	if (parm->nRobust < 4) {
		gsw_w32(cdev, PCE_IGMP_CTRL_ROB_OFFSET,
			PCE_IGMP_CTRL_ROB_SHIFT,
			PCE_IGMP_CTRL_ROB_SIZE, parm->nRobust);
	} else {
		pr_err("The Robust time would only support 0..3\n");
		return GSW_statusErr;
	}
	gsw_w32(cdev, PCE_IGMP_CTRL_DMRTEN_OFFSET,
		PCE_IGMP_CTRL_DMRTEN_SHIFT,
		PCE_IGMP_CTRL_DMRTEN_SIZE, 1);
	gsw_w32(cdev, PCE_IGMP_CTRL_DMRT_OFFSET,
		PCE_IGMP_CTRL_DMRT_SHIFT,
		PCE_IGMP_CTRL_DMRT_SIZE,
		parm->nQueryInterval);

	if (parm->eIGMP_Mode ==
		GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		switch (parm->eSuppressionAggregation) {
		case GSW_MULTICAST_REPORT_JOIN:
			data_2 = 1;	data_1 = 1;
			break;
		case GSW_MULTICAST_REPORT:
			data_2 = 0;	data_1 = 1;
			break;
		case GSW_MULTICAST_TRANSPARENT:
			data_2 = 0;	data_1 = 0;
			break;
		}
		gsw_w32(cdev, PCE_IGMP_CTRL_REPSUP_OFFSET,
			PCE_IGMP_CTRL_REPSUP_SHIFT,
			PCE_IGMP_CTRL_REPSUP_SIZE, data_1);
		gsw_w32(cdev, PCE_IGMP_CTRL_JASUP_OFFSET,
			PCE_IGMP_CTRL_JASUP_SHIFT,
			PCE_IGMP_CTRL_JASUP_SIZE, data_2);
	}

	if (parm->eIGMP_Mode ==
		GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		gsw_w32(cdev, PCE_IGMP_CTRL_SRPEN_OFFSET,
			PCE_IGMP_CTRL_SRPEN_SHIFT,
			PCE_IGMP_CTRL_SRPEN_SIZE,
			parm->bLearningRouter);
		if (parm->bFastLeave == 1)
			gsw_w32(cdev, PCE_IGMP_CTRL_FLEAVE_OFFSET,
				PCE_IGMP_CTRL_FLEAVE_SHIFT,
				PCE_IGMP_CTRL_FLEAVE_SIZE, 1);
		else
			gsw_w32(cdev, PCE_IGMP_CTRL_FLEAVE_OFFSET,
				PCE_IGMP_CTRL_FLEAVE_SHIFT,
				PCE_IGMP_CTRL_FLEAVE_SIZE, 0);
	} else {
		gsw_w32(cdev, PCE_IGMP_CTRL_FLEAVE_OFFSET,
			PCE_IGMP_CTRL_FLEAVE_SHIFT,
			PCE_IGMP_CTRL_FLEAVE_SIZE, 0);
	}
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC) ||
		(gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_w32(cdev, PCE_GCTRL_1_UKIPMC_OFFSET,
			PCE_GCTRL_1_UKIPMC_SHIFT,
			PCE_GCTRL_1_UKIPMC_SIZE,
			parm->bMulticastUnknownDrop);
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, PCE_GCTRL_1_MKFIDEN_OFFSET,
			PCE_GCTRL_1_MKFIDEN_SHIFT,
			PCE_GCTRL_1_MKFIDEN_SIZE,
			parm->bMulticastFIDmode);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MulticastTableEntryAdd(void *cdev,
	GSW_multicastTable_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	gsw_igmp_t	*hitbl = &gswdev->iflag;
	u8 pidx = parm->nPortId;
	pctbl_prog_t ptdata;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	memset(&ptdata, 0, sizeof(pctbl_prog_t));
	if (hitbl->igmode == GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		u32 index = 0, i, available = 0;
		if ((gswdev->iflag.igv3 == 1) ||
			(parm->eIPVersion == GSW_IP_SELECT_IPV6))
			return GSW_statusErr;
		/* Read Out all of the HW Table */
		for (i = 0; i < gswdev->mctblsize; i++) {
			ptdata.table = PCE_MULTICAST_HW_INDEX;
			ptdata.pcindex = i;
			gsw_pce_table_read(cdev, &ptdata);
			if (ptdata.valid) {
				if ((ptdata.key[0] ==
				(parm->uIP_Gda.nIPv4 & 0xFFFF)) &&
				(ptdata.key[1] ==
				((parm->uIP_Gda.nIPv4 >> 16) & 0xFFFF))) {
					index = i;
					available = 1;
					break;
				}
			}
		}
		ptdata.table = PCE_MULTICAST_HW_INDEX;
		if (available == 0) {
			index = gswdev->mctblsize;
			for (i = 0; i < gswdev->mctblsize; i++) {
				ptdata.pcindex = i;
				gsw_pce_table_read(cdev, &ptdata);
				if (ptdata.valid == 0) {
					index = i;  /* Free index */
					break;
				}
			}
		}
		if (index < gswdev->mctblsize) {
			ptdata.table = PCE_MULTICAST_HW_INDEX;
			ptdata.pcindex = index;
			ptdata.key[1] = ((parm->uIP_Gda.nIPv4 >> 16) & 0xFFFF);
			ptdata.key[0] = (parm->uIP_Gda.nIPv4 & 0xFFFF);
			ptdata.val[0] |= (1 << pidx);
			ptdata.val[4] |= (1 << 14);
			ptdata.valid = 1;
			gsw_pce_table_write(gswdev, &ptdata);
		} else {
			pr_err("Error: (IGMP HW Table is full) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
			return GSW_statusErr;
		}

	} else if (hitbl->igmode == GSW_MULTICAST_SNOOP_MODE_FORWARD) {
		/* Program the Multicast SW Table */
		if (gswdev->gipver == LTQ_GSWIP_3_0)
			gsw3x_msw_table_wr(cdev, parm);
		else
			gsw2x_msw_table_wr(cdev, parm);
	} else {
		/* Disable All Multicast SW Table */
		pr_err("Please Select the IGMP Mode through Multicast Snooping Configuration API\n");
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MulticastTableEntryRead(void *cdev,
	GSW_multicastTableRead_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	gsw_igmp_t	*hitbl = &gswdev->iflag;
	pctbl_prog_t ptdata;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (hitbl->igmode == GSW_MULTICAST_SNOOP_MODE_DISABLED) {
		pr_err("Error: (IGMP snoop is not enabled) %s:%s:%d\n",
		__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (hitbl->igmode == GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		if (parm->bInitial == 1) {
			gswdev->mhw_rinx = 0; /*Start from the index 0 */
			parm->bInitial = 0;
		}
		if (gswdev->mhw_rinx >= gswdev->mctblsize) {
			memset(parm, 0, sizeof(GSW_multicastTableRead_t));
			parm->bLast = 1;
			gswdev->mhw_rinx = 0;
			return GSW_statusOk;
		}

		do {
			memset(&ptdata, 0, sizeof(pctbl_prog_t));
			ptdata.table = PCE_MULTICAST_HW_INDEX;
			ptdata.pcindex = gswdev->mhw_rinx;
			gsw_pce_table_read(cdev, &ptdata);
			gswdev->mhw_rinx++;
			if (ptdata.valid != 0)
				break;
		} while (gswdev->mhw_rinx < gswdev->mctblsize);
		if (ptdata.valid != 0) {
			parm->nPortId = ptdata.val[0] | 0x80000000;
			parm->uIP_Gda.nIPv4 =
			((ptdata.key[1] << 16) |
			ptdata.key[0]);
			parm->uIP_Gsa.nIPv4 = 0;
			parm->eModeMember = GSW_IGMP_MEMBER_DONT_CARE;
			parm->eIPVersion = GSW_IP_SELECT_IPV4;
			parm->bInitial = 0;
			parm->bLast = 0;
		} else {
			memset(parm, 0, sizeof(GSW_multicastTableRead_t));
			parm->bLast = 1;
			gswdev->mhw_rinx = 0;
		}
	}
	/*Snooping in Forward mode */
	if (hitbl->igmode == GSW_MULTICAST_SNOOP_MODE_FORWARD) {
		u32 dlsbid, slsbid, dmsbid, smsbid;
		if (parm->bInitial == 1) {
			gswdev->msw_rinx = 0; /*Start from the index 0 */
			parm->bInitial = 0;
		}
		if (gswdev->msw_rinx >= gswdev->mctblsize) {
			memset(parm, 0, sizeof(GSW_multicastTableRead_t));
			parm->bLast = 1;
			gswdev->msw_rinx = 0;
			return GSW_statusOk;
		}

		do {
			memset(&ptdata, 0, sizeof(pctbl_prog_t));
			ptdata.table = PCE_MULTICAST_SW_INDEX;
			ptdata.pcindex = gswdev->msw_rinx;
			gsw_pce_table_read(cdev, &ptdata);
			gswdev->msw_rinx++;
			if (ptdata.valid != 0)
				break;
		} while (gswdev->msw_rinx < gswdev->mctblsize);
		if (ptdata.valid == 1) {
			pctbl_prog_t iptbl;
			parm->nPortId = ptdata.val[0] | 0x80000000;
			if (gswdev->gipver == LTQ_GSWIP_3_0) {
				parm->nSubIfId = (ptdata.val[1] >> 3) & 0xFFFF;
				parm->nFID = (ptdata.key[2] & 0x3F);
				parm->bExclSrcIP =
				((ptdata.key[2] >> 15) & 0x1);
			}
			dlsbid = ptdata.key[0] & 0xFF;
			dmsbid = (ptdata.key[0] >> 8) & 0xFF;
			slsbid = ptdata.key[1] & 0xFF;
			smsbid = (ptdata.key[1] >> 8) & 0xFF;
			if (dlsbid <= 0x3F) {
				memset(&iptbl, 0, sizeof(pctbl_prog_t));
				iptbl.table = PCE_IP_DASA_LSB_INDEX;
				/* Search the DIP */
				iptbl.pcindex = dlsbid;
				gsw_pce_table_read(cdev, &iptbl);
				if (iptbl.valid == 1) {
					if (gswdev->gipver == LTQ_GSWIP_3_0) {
						if ((iptbl.mask[0] == 0x0) &&
						((iptbl.mask[1] == 0x0)) &&
						((iptbl.mask[2] == 0xFFFF)) &&
						((iptbl.mask[3] == 0xFFFF))) {
							parm->uIP_Gda.nIPv4 =
							((iptbl.key[1] << 16)
							| (iptbl.key[0]));
							parm->eIPVersion =
							GSW_IP_SELECT_IPV4;
						} else if ((iptbl.mask[0] == 0x0) &&
							((iptbl.mask[1] == 0x0)) &&
							((iptbl.mask[2] == 0x0)) &&
							((iptbl.mask[3] == 0x0))) {
							parm->uIP_Gda.nIPv6[4] =
								(iptbl.key[3]);
							parm->uIP_Gda.nIPv6[5] =
								(iptbl.key[2]);
							parm->uIP_Gda.nIPv6[6] =
								(iptbl.key[1]);
							parm->uIP_Gda.nIPv6[7] =
								(iptbl.key[0]);
							parm->eIPVersion =
							GSW_IP_SELECT_IPV6;
						}
					} else {
						if (iptbl.mask[0] == 0xFF00) {
							parm->uIP_Gda.nIPv4 =
							((iptbl.key[1] << 16)
							| (iptbl.key[0]));
							parm->eIPVersion =
							GSW_IP_SELECT_IPV4;
						} else if (iptbl.mask[0] == 0x0) {
							parm->uIP_Gda.nIPv6[4] =
								(iptbl.key[3]);
							parm->uIP_Gda.nIPv6[5] =
								(iptbl.key[2]);
							parm->uIP_Gda.nIPv6[6] =
								(iptbl.key[1]);
							parm->uIP_Gda.nIPv6[7] =
								(iptbl.key[0]);
							parm->eIPVersion =
							GSW_IP_SELECT_IPV6;
						}
					}
				}
			}
			if (slsbid <= 0x3F) {
				memset(&iptbl, 0, sizeof(pctbl_prog_t));
				iptbl.table = PCE_IP_DASA_LSB_INDEX;
				/* Search the SIP */
				iptbl.pcindex = slsbid;
				gsw_pce_table_read(cdev, &iptbl);
				if (iptbl.valid == 1) {
					if (gswdev->gipver == LTQ_GSWIP_3_0) {
						if ((iptbl.mask[0] == 0x0) &&
						((iptbl.mask[1] == 0x0)) &&
						((iptbl.mask[2] == 0xFFFF)) &&
						((iptbl.mask[3] == 0xFFFF))) {
							parm->uIP_Gsa.nIPv4 =
							((iptbl.key[1] << 16)
							| (iptbl.key[0]));
							parm->eIPVersion =
							GSW_IP_SELECT_IPV4;
						} else if ((iptbl.mask[0] == 0x0) &&
							((iptbl.mask[1] == 0x0)) &&
							((iptbl.mask[2] == 0x0)) &&
							((iptbl.mask[3] == 0x0))) {
							parm->uIP_Gsa.nIPv6[4] =
							(iptbl.key[3]);
							parm->uIP_Gsa.nIPv6[5] =
							(iptbl.key[2]);
							parm->uIP_Gsa.nIPv6[6] =
							(iptbl.key[1]);
							parm->uIP_Gsa.nIPv6[7] =
							(iptbl.key[0]);
						}
					} else {
						if (iptbl.mask[0] == 0xFF00) {
							parm->uIP_Gsa.nIPv4 =
							((iptbl.key[1] << 16)
							| (iptbl.key[0]));
							parm->eIPVersion =
							GSW_IP_SELECT_IPV4;
						} else if (iptbl.mask == 0x0) {
							parm->uIP_Gsa.nIPv6[4] =
								(iptbl.key[3]);
							parm->uIP_Gsa.nIPv6[5] =
								(iptbl.key[2]);
							parm->uIP_Gsa.nIPv6[6] =
								(iptbl.key[1]);
							parm->uIP_Gsa.nIPv6[7] =
								(iptbl.key[0]);
						}
					}
				}
			}
			if (dmsbid <= 0xF) {
				memset(&iptbl, 0, sizeof(pctbl_prog_t));
				iptbl.table = PCE_IP_DASA_MSB_INDEX;
				/* Search the DIP */
				iptbl.pcindex = dmsbid;
				gsw_pce_table_read(cdev, &iptbl);
				if (iptbl.valid == 1) {
					if (gswdev->gipver == LTQ_GSWIP_3_0) {
						if ((iptbl.mask[0] == 0) &&
						((iptbl.mask[1] == 0)) &&
						((iptbl.mask[2] == 0)) &&
						((iptbl.mask[3] == 0))) {
							parm->uIP_Gda.nIPv6[0] =
								(iptbl.key[3]);
							parm->uIP_Gda.nIPv6[1] =
								(iptbl.key[2]);
							parm->uIP_Gda.nIPv6[2] =
								(iptbl.key[1]);
							parm->uIP_Gda.nIPv6[3] =
								(iptbl.key[0]);
						}
					} else {
						if (iptbl.mask  == 0) {
							parm->uIP_Gda.nIPv6[0] =
								(iptbl.key[3]);
							parm->uIP_Gda.nIPv6[1] =
								(iptbl.key[2]);
							parm->uIP_Gda.nIPv6[2] =
								(iptbl.key[1]);
							parm->uIP_Gda.nIPv6[3] =
								(iptbl.key[0]);
						}
					}
				}
			}
			if (smsbid <= 0xF) {
				memset(&iptbl, 0, sizeof(pctbl_prog_t));
				iptbl.table = PCE_IP_DASA_MSB_INDEX;
				/* Search the DIP */
				iptbl.pcindex = smsbid;
				gsw_pce_table_read(cdev, &iptbl);
				if (iptbl.valid == 1) {
					if (gswdev->gipver == LTQ_GSWIP_3_0) {
						if ((iptbl.mask[0] == 0) &&
						((iptbl.mask[1] == 0)) &&
						((iptbl.mask[2] == 0)) &&
						((iptbl.mask[3] == 0))) {
							parm->uIP_Gsa.nIPv6[0] =
								(iptbl.key[3]);
							parm->uIP_Gsa.nIPv6[1] =
								(iptbl.key[2]);
							parm->uIP_Gsa.nIPv6[2] =
								(iptbl.key[1]);
							parm->uIP_Gsa.nIPv6[3] =
								(iptbl.key[0]);
						}
					} else {
						if (iptbl.mask == 0) {
							parm->uIP_Gsa.nIPv6[0] =
								(iptbl.key[3]);
							parm->uIP_Gsa.nIPv6[1] =
								(iptbl.key[2]);
							parm->uIP_Gsa.nIPv6[2] =
								(iptbl.key[1]);
							parm->uIP_Gsa.nIPv6[3] =
								(iptbl.key[0]);
						}
					}
				}
			}
			parm->eModeMember =
				hitbl->mctable[gswdev->msw_rinx-1].mcmode;
			parm->bInitial = 0;
			parm->bLast = 0;
		} else {
			memset(parm, 0, sizeof(GSW_multicastTableRead_t));
			parm->bLast = 1;
			gswdev->msw_rinx = 0;
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MulticastTableEntryRemove(void *cdev,
	GSW_multicastTable_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	gsw_igmp_t	*hitbl = &gswdev->iflag;
	u8 pidx = parm->nPortId;
	pctbl_prog_t ptdata;
	ltq_bool_t dflag = 0;
	u32 port = 0, i;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	memset(&ptdata, 0, sizeof(pctbl_prog_t));
	if (hitbl->igmode ==
		GSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		if (gswdev->iflag.igv3 == 1)
			return GSW_statusErr;
		/* Read Out all of the HW Table */
		for (i = 0; i < gswdev->mctblsize; i++) {
			memset(&ptdata, 0, sizeof(pctbl_prog_t));
			ptdata.table = PCE_MULTICAST_HW_INDEX;
			ptdata.pcindex = i;
			gsw_pce_table_read(cdev, &ptdata);
			/* Fill into Structure */
			if (((ptdata.val[0] >> pidx) & 0x1) == 1) {
				if (parm->uIP_Gda.nIPv4 ==
					((ptdata.key[1] << 16)
						| (ptdata.key[0]))) {
					port = (ptdata.val[0] & (~(1 << pidx)));
					if (port == 0) {
						ptdata.val[0] = 0;
						ptdata.key[1] = 0;
						ptdata.val[4] = 0;
					} else {
						ptdata.val[0] &= ~(1 << pidx);
					}
					dflag = 1;
					gsw_pce_table_write(cdev, &ptdata);
				}
			}
		}
		if (dflag == 0)
			pr_err("The input did not found\n");
	} else if (hitbl->igmode ==
		GSW_MULTICAST_SNOOP_MODE_FORWARD) {
		if (gswdev->gipver == LTQ_GSWIP_3_0)
			gsw3x_msw_table_rm(gswdev, parm);
		else
			gsw2x_msw_table_rm(gswdev, parm);
	}
	return GSW_statusOk;
}
#endif /*CONFIG_LTQ_MULTICAST*/
GSW_return_t GSW_CapGet(void *cdev, GSW_cap_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value, data1, data2;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nCapType >= GSW_CAP_TYPE_LAST)
		return GSW_statusErr;
	else
		strcpy(parm->cDesc, capdes[parm->nCapType].desci);
	/* As request, attached the code in the next version*/
	switch (parm->nCapType) {
	case GSW_CAP_TYPE_PORT:
		gsw_r32(cdev, ETHSW_CAP_1_PPORTS_OFFSET,
			ETHSW_CAP_1_PPORTS_SHIFT,
			ETHSW_CAP_1_PPORTS_SIZE, &value);
		parm->nCap = value;
		break;
	case GSW_CAP_TYPE_VIRTUAL_PORT:
		gsw_r32(cdev, ETHSW_CAP_1_VPORTS_OFFSET,
			ETHSW_CAP_1_VPORTS_SHIFT,
			ETHSW_CAP_1_VPORTS_SIZE, &value);
		parm->nCap = value;
		break;
	case GSW_CAP_TYPE_BUFFER_SIZE:
		gsw_r32(cdev, ETHSW_CAP_11_BSIZEL_OFFSET,
			ETHSW_CAP_11_BSIZEL_SHIFT,
			ETHSW_CAP_11_BSIZEL_SIZE, &data1);
		gsw_r32(cdev, ETHSW_CAP_12_BSIZEH_OFFSET,
			ETHSW_CAP_12_BSIZEH_SHIFT,
			ETHSW_CAP_12_BSIZEH_SIZE, &data2);
		parm->nCap = (data2 << 16 | data1);
		break;
	case GSW_CAP_TYPE_SEGMENT_SIZE:
		/* This is Hard coded */
		parm->nCap = LTQ_SOC_CAP_SEGMENT;
		break;
	case GSW_CAP_TYPE_PRIORITY_QUEUE:
		parm->nCap = gswdev->num_of_queues;
		break;
	case GSW_CAP_TYPE_METER:
		parm->nCap = gswdev->num_of_meters;
		break;
	case GSW_CAP_TYPE_RATE_SHAPER:
		parm->nCap = gswdev->num_of_shapers;
		break;
	case GSW_CAP_TYPE_VLAN_GROUP:
		parm->nCap = gswdev->avlantsz;
		break;
	case GSW_CAP_TYPE_FID:
		/* This is Hard coded */
		parm->nCap = VRX_PLATFORM_CAP_FID;
		break;
	case GSW_CAP_TYPE_MAC_TABLE_SIZE:
		parm->nCap = gswdev->mactblsize;
		break;
	case GSW_CAP_TYPE_MULTICAST_TABLE_SIZE:
		parm->nCap = gswdev->mctblsize;
		break;
	case GSW_CAP_TYPE_PPPOE_SESSION:
		parm->nCap = gswdev->num_of_pppoe;
		break;
	case GSW_CAP_TYPE_SVLAN_GROUP:
		parm->nCap = gswdev->avlantsz;
		break;
	case GSW_CAP_TYPE_PMAC:
		parm->nCap = gswdev->num_of_pmac;
		break;
	case GSW_CAP_TYPE_PAYLOAD:
		parm->nCap = gswdev->pdtblsize;
		break;
	case GSW_CAP_TYPE_IF_RMON:
		parm->nCap = gswdev->num_of_ifrmon;
		break;
	case GSW_CAP_TYPE_EGRESS_VLAN:
		parm->nCap = gswdev->num_of_egvlan;
		break;
	case GSW_CAP_TYPE_RT_SMAC:
		parm->nCap = gswdev->num_of_rt_smac;
		break;
	case GSW_CAP_TYPE_RT_DMAC:
		parm->nCap = gswdev->num_of_rt_dmac;
		break;
	case GSW_CAP_TYPE_RT_PPPoE:
		parm->nCap = gswdev->num_of_rt_ppoe;
		break;
	case GSW_CAP_TYPE_RT_NAT:
		parm->nCap = gswdev->num_of_rt_nat;
		break;
	case GSW_CAP_TYPE_RT_MTU:
		parm->nCap = gswdev->num_of_rt_mtu;
		break;
	case GSW_CAP_TYPE_RT_TUNNEL:
		parm->nCap = gswdev->num_of_rt_tunnel;
		break;
	case GSW_CAP_TYPE_RT_RTP:
		parm->nCap = gswdev->num_of_rt_rtp;
		break;
	case GSW_CAP_TYPE_LAST:
		parm->nCap = GSW_CAP_TYPE_LAST;
		break;
	default:
		parm->nCap = 0;
		break;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_CfgGet(void *cdev, GSW_cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value, data2;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Aging Counter Mantissa Value */
	gsw_r32(cdev, PCE_AGE_1_MANT_OFFSET,
		PCE_AGE_1_MANT_SHIFT,
		PCE_AGE_1_MANT_SIZE, &data2);
	if (data2 == AGETIMER_1_DAY)
		parm->eMAC_TableAgeTimer = GSW_AGETIMER_1_DAY;
	else if (data2 == AGETIMER_1_HOUR)
		parm->eMAC_TableAgeTimer = GSW_AGETIMER_1_HOUR;
	else if (data2 == AGETIMER_300_SEC)
		parm->eMAC_TableAgeTimer = GSW_AGETIMER_300_SEC;
	else if (data2 == AGETIMER_10_SEC)
		parm->eMAC_TableAgeTimer = GSW_AGETIMER_10_SEC;
	else if (data2 == AGETIMER_1_SEC)
		parm->eMAC_TableAgeTimer = GSW_AGETIMER_1_SEC;
	else
		parm->eMAC_TableAgeTimer = 0;

	gsw_r32(cdev, MAC_FLEN_LEN_OFFSET,
		MAC_FLEN_LEN_SHIFT, MAC_FLEN_LEN_SIZE, &value);
	parm->nMaxPacketLen = value;
	gsw_r32(cdev, MAC_PFAD_CFG_SAMOD_OFFSET,
		MAC_PFAD_CFG_SAMOD_SHIFT,
		MAC_PFAD_CFG_SAMOD_SIZE, &value);
	parm->bPauseMAC_ModeSrc = value;
	gsw_r32(cdev, PCE_GCTRL_0_VLAN_OFFSET,
		PCE_GCTRL_0_VLAN_SHIFT,
		PCE_GCTRL_0_VLAN_SIZE, &value);
	parm->bVLAN_Aware = value;
	/* MAC Address Learning Limitation Mode */
	gsw_r32(cdev, PCE_GCTRL_0_PLIMMOD_OFFSET ,
		PCE_GCTRL_0_PLIMMOD_SHIFT,
		PCE_GCTRL_0_PLIMMOD_SIZE, &value);
	parm->bLearningLimitAction = value;
/*Accept or discard MAC spoofing and port MAC locking violation packets */
	if ((gswdev->gipver == LTQ_GSWIP_2_2) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC)
		|| (gswdev->gipver == LTQ_GSWIP_3_0)) {
		gsw_r32(cdev, PCE_GCTRL_0_PLCKMOD_OFFSET ,
			PCE_GCTRL_0_PLCKMOD_SHIFT,
			PCE_GCTRL_0_PLCKMOD_SIZE,
			&parm->bMAC_SpoofingAction);
	}
	gsw_r32(cdev, MAC_PFSA_0_PFAD_OFFSET,
		MAC_PFSA_0_PFAD_SHIFT, MAC_PFSA_0_PFAD_SIZE, &value);
	parm->nPauseMAC_Src[5] = value & 0xFF;
	parm->nPauseMAC_Src[4] = (value >> 8) & 0xFF;
	gsw_r32(cdev, MAC_PFSA_1_PFAD_OFFSET,
		MAC_PFSA_1_PFAD_SHIFT, MAC_PFSA_1_PFAD_SIZE, &value);
	parm->nPauseMAC_Src[3] = value & 0xFF;
	parm->nPauseMAC_Src[2] = (value >> 8) & 0xFF;
	gsw_r32(cdev, MAC_PFSA_2_PFAD_OFFSET,
		MAC_PFSA_2_PFAD_SHIFT, MAC_PFSA_2_PFAD_SIZE, &value);
	parm->nPauseMAC_Src[1] = value & 0xFF;
	parm->nPauseMAC_Src[0] = (value >> 8) & 0xFF;
	return GSW_statusOk;
}

GSW_return_t GSW_CfgSet(void *cdev, GSW_cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 MANT = 0, EXP = 0, value, i;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	switch (parm->eMAC_TableAgeTimer) {
	case GSW_AGETIMER_1_SEC:
		MANT = AGETIMER_1_SEC; EXP = 0x2;
		gswdev->matimer = 1;
		break;
	case GSW_AGETIMER_10_SEC:
		MANT = AGETIMER_10_SEC; EXP = 0x2;
		gswdev->matimer = 10;
		break;
	case GSW_AGETIMER_300_SEC:
		MANT = AGETIMER_300_SEC; EXP = 0x2;
		gswdev->matimer = 300;
		break;
	case GSW_AGETIMER_1_HOUR:
		MANT = AGETIMER_1_HOUR; EXP = 0x6;
		gswdev->matimer = 3600;
		break;
	case GSW_AGETIMER_1_DAY:
		MANT = AGETIMER_1_DAY; EXP = 0xA;
		gswdev->matimer = 86400;
		break;
	default:
		MANT = AGETIMER_300_SEC; EXP = 0x2;
		gswdev->matimer = 300;
	}

	/* Aging Counter Exponent Value */
	gsw_w32(cdev, PCE_AGE_0_EXP_OFFSET,
		PCE_AGE_0_EXP_SHIFT, PCE_AGE_0_EXP_SIZE, EXP);
	/* Aging Counter Mantissa Value */
	gsw_w32(cdev, PCE_AGE_1_MANT_OFFSET,
		PCE_AGE_1_MANT_SHIFT, PCE_AGE_1_MANT_SIZE, MANT);
	/* Maximum Ethernet packet length */
	if (parm->nMaxPacketLen < 0xFFFF)
		value = parm->nMaxPacketLen;
	else
		value = MAX_PACKET_LENGTH;
	gsw_w32(cdev, MAC_FLEN_LEN_OFFSET,
		MAC_FLEN_LEN_SHIFT, MAC_FLEN_LEN_SIZE, value);
	if (parm->nMaxPacketLen > 0x5EE) {
		for (i = 0; i < 6; i++) {
			gsw_w32(cdev, (MAC_CTRL_2_MLEN_OFFSET + (i * 0xC)),
				MAC_CTRL_2_MLEN_SHIFT,
				MAC_CTRL_2_MLEN_SIZE, 1);
		}
	}
	/* MAC Address Learning Limitation Mode */
	gsw_w32(cdev, PCE_GCTRL_0_PLIMMOD_OFFSET,
		PCE_GCTRL_0_PLIMMOD_SHIFT,
		PCE_GCTRL_0_PLIMMOD_SIZE, parm->bLearningLimitAction);
		/*Accept or discard MAC spoofing and port */
		/* MAC locking violation packets */
	if ((gswdev->gipver == LTQ_GSWIP_2_2)
		|| (gswdev->gipver == LTQ_GSWIP_3_0) ||
		(gswdev->gipver == LTQ_GSWIP_2_2_ETC)) {
		gsw_w32(cdev, PCE_GCTRL_0_PLCKMOD_OFFSET ,
			PCE_GCTRL_0_PLCKMOD_SHIFT,
			PCE_GCTRL_0_PLCKMOD_SIZE, parm->bMAC_SpoofingAction);
	}
	/* VLAN-aware Switching           */
	gsw_w32(cdev, PCE_GCTRL_0_VLAN_OFFSET,
		PCE_GCTRL_0_VLAN_SHIFT,
		PCE_GCTRL_0_VLAN_SIZE, parm->bVLAN_Aware);
	/* MAC Source Address Mode */
	if (parm->bPauseMAC_ModeSrc == 1) {
		gsw_w32(cdev, MAC_PFAD_CFG_SAMOD_OFFSET,
			MAC_PFAD_CFG_SAMOD_SHIFT,
			MAC_PFAD_CFG_SAMOD_SIZE, parm->bPauseMAC_ModeSrc);
		value = parm->nPauseMAC_Src[4] << 8 | parm->nPauseMAC_Src[5];
		gsw_w32(cdev, MAC_PFSA_0_PFAD_OFFSET,
			MAC_PFSA_0_PFAD_SHIFT, MAC_PFSA_0_PFAD_SIZE, value);
		value = parm->nPauseMAC_Src[2] << 8 | parm->nPauseMAC_Src[3];
		gsw_w32(cdev, MAC_PFSA_1_PFAD_OFFSET,
			MAC_PFSA_1_PFAD_SHIFT, MAC_PFSA_1_PFAD_SIZE, value);
		value = parm->nPauseMAC_Src[0] << 8 | parm->nPauseMAC_Src[1];
		gsw_w32(cdev, MAC_PFSA_2_PFAD_OFFSET,
			MAC_PFSA_2_PFAD_SHIFT,
			MAC_PFSA_2_PFAD_SIZE, value);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_HW_Init(void *cdev, GSW_HW_Init_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 j;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Reset the Switch via Switch IP register*/
	j = 1;
	gsw_w32(cdev, ETHSW_SWRES_R0_OFFSET,
		ETHSW_SWRES_R0_SHIFT, ETHSW_SWRES_R0_SIZE, j);
	do {
		udelay(100);
		gsw_r32(cdev, ETHSW_SWRES_R0_OFFSET,
			ETHSW_SWRES_R0_SHIFT, ETHSW_SWRES_R0_SIZE, &j);
	} while (j);
#if defined(CONFIG_USE_EMULATOR) && CONFIG_USE_EMULATOR
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		/* Set Auto-Polling of connected PHYs - For all ports */
		gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MDCCFG_0_PEN_1_SHIFT, 6, 0x0);
	} else {
		/* Set Auto-Polling of connected PHYs - For all ports */
		gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET
			+ GSW_TREG_OFFSET),
			MDC_CFG_0_PEN_0_SHIFT, 6, 0x0);
	}
#else
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			gsw_r_init();
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, 0x1);
		} else {
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, 0x1e);
		}
	}
#endif  /* CONFIG_USE_EMULATOR */
/*	platform_device_init(gswdev); */
	gswdev->hwinit = 1;
/*	get_gsw_hw_cap (gswdev); */
	/* Software Table Init */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
	reset_vlan_sw_table(gswdev);
#endif /*CONFIG_LTQ_VLAN */
	ltq_ethsw_port_cfg_init(gswdev);
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
	reset_multicast_sw_table(gswdev);
#endif /*CONFIG_LTQ_MULTICAST*/
	pce_table_init(&gswdev->phandler);
	/* HW Init */
	gsw_pmicro_code_init(gswdev);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			rt_table_init();
			gsw_w32(cdev, PCE_TFCR_NUM_NUM_OFFSET, PCE_TFCR_NUM_NUM_SHIFT,
				PCE_TFCR_NUM_NUM_SIZE, 0x80);
		}
		/* EEE auto negotiation overides:*/
		/*  clock disable (ANEG_EEE_0.CLK_STOP_CAPABLE)  */
		for (j = 0; j < gswdev->pnum-1; j++) {
			gsw_w32(cdev,
			((GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_OFFSET
			+ (4 * j)) + GSW30_TOP_OFFSET),
			GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_SHIFT,
			GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_SIZE, 0x3);
		}
	} else {
		/* Configure the MDIO Clock 97.6 Khz */
		gsw_w32(cdev, (MDC_CFG_1_FREQ_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_1_FREQ_SHIFT,
			MDC_CFG_1_FREQ_SIZE, 0xFF);
		for (j = 0; j < gswdev->pnum-1; j++) {
			gsw_w32(cdev, ((ANEG_EEE_0_CLK_STOP_CAPABLE_OFFSET+j)
				+ GSW_TREG_OFFSET),
				ANEG_EEE_0_CLK_STOP_CAPABLE_SHIFT,
				ANEG_EEE_0_CLK_STOP_CAPABLE_SIZE, 0x3);
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MDIO_CfgGet(void *cdev, GSW_MDIO_cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, (GSWT_MDCCFG_1_FREQ_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_FREQ_SHIFT,
			GSWT_MDCCFG_1_FREQ_SIZE, &value);
		parm->nMDIO_Speed = value & 0xFF;
		gsw_r32(cdev, (GSWT_MDCCFG_1_MCEN_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_MCEN_SHIFT,
			GSWT_MDCCFG_1_MCEN_SIZE, &value);
		parm->bMDIO_Enable = value;
	} else {
		gsw_r32(cdev, (MDC_CFG_1_FREQ_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_1_FREQ_SHIFT,
			MDC_CFG_1_FREQ_SIZE, &value);
		parm->nMDIO_Speed = value & 0xFF;
		gsw_r32(cdev, (MDC_CFG_1_MCEN_OFFSET
			+ GSW_TREG_OFFSET),
			MDC_CFG_1_MCEN_SHIFT,
			MDC_CFG_1_MCEN_SIZE, &value);
		parm->bMDIO_Enable = value;
	}
	if (value == 1)
		parm->bMDIO_Enable = 1;
	else
		parm->bMDIO_Enable = 0;
	return GSW_statusOk;
}

GSW_return_t GSW_MDIO_CfgSet(void *cdev, GSW_MDIO_cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	pr_err("**********%s:%s:%d*************\n",__FILE__, __func__, __LINE__);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, (GSWT_MDCCFG_1_FREQ_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_FREQ_SHIFT, GSWT_MDCCFG_1_FREQ_SIZE, parm->nMDIO_Speed);
		if (parm->bMDIO_Enable)
			value = 0x3F;
		else
			value = 0;
		/* Set Auto-Polling of connected PHYs - For all ports */
		gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDCCFG_0_PEN_1_SHIFT, 6, value);
		gsw_w32(cdev, (GSWT_MDCCFG_1_MCEN_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDCCFG_1_MCEN_SHIFT, GSWT_MDCCFG_1_MCEN_SIZE, parm->bMDIO_Enable);
	} else {
		gsw_w32(cdev, (MDC_CFG_1_FREQ_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_1_FREQ_SHIFT, MDC_CFG_1_FREQ_SIZE, parm->nMDIO_Speed);
		if (parm->bMDIO_Enable)
			value = 0x3F;
		else
			value = 0;
		/* Set Auto-Polling of connected PHYs - For all ports */
		gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_0_PEN_0_SHIFT, 6, value);
		gsw_w32(cdev, (MDC_CFG_1_MCEN_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_1_MCEN_SHIFT,
			MDC_CFG_1_MCEN_SIZE, parm->bMDIO_Enable);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MDIO_DataRead(void *cdev, GSW_MDIO_data_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		do {
			udelay(1);
			gsw_r32(cdev, (GSWT_MDCTRL_MBUSY_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCTRL_MBUSY_SHIFT, GSWT_MDCTRL_MBUSY_SIZE, &value);
		} while (value);
		value = ((0x2 << 10) | ((parm->nAddressDev & 0x1F) << 5)
			| (parm->nAddressReg & 0x1F));
		/* Special write command, becouse we need to write */
		/* "MDIO Control Register" once at a time */
		gsw_w32(cdev, (GSWT_MDCTRL_MBUSY_OFFSET + GSW30_TOP_OFFSET), 0, 16, value);
		do {
			udelay(1);
			gsw_r32(cdev, (GSWT_MDCTRL_MBUSY_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCTRL_MBUSY_SHIFT, GSWT_MDCTRL_MBUSY_SIZE, &value);
		} while (value);
		gsw_r32(cdev, (GSWT_MDREAD_RDATA_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDREAD_RDATA_SHIFT, GSWT_MDREAD_RDATA_SIZE, &value);
		parm->nData = (value & 0xFFFF);
	} else {
		do {
			gsw_r32(cdev, (MDIO_CTRL_MBUSY_OFFSET + GSW_TREG_OFFSET),
				MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
		} while (value);
		value = ((0x2 << 10) | ((parm->nAddressDev & 0x1F) << 5)
			| (parm->nAddressReg & 0x1F));
		/* Special write command, becouse we need to write */
		/* "MDIO Control Register" once at a time */
			gsw_w32(cdev, (MDIO_CTRL_MBUSY_OFFSET + GSW_TREG_OFFSET), 0, 16, value);
		do {
			gsw_r32(cdev, (MDIO_CTRL_MBUSY_OFFSET + GSW_TREG_OFFSET),
				MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
		} while (value);
		gsw_r32(cdev, (MDIO_READ_RDATA_OFFSET + GSW_TREG_OFFSET),
			MDIO_READ_RDATA_SHIFT, MDIO_READ_RDATA_SIZE, &value);
		parm->nData = (value & 0xFFFF);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MDIO_DataWrite(void *cdev, GSW_MDIO_data_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		do {
			udelay(1);
			gsw_r32(cdev, (GSWT_MDCTRL_MBUSY_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCTRL_MBUSY_SHIFT, GSWT_MDCTRL_MBUSY_SIZE, &value);
		} while (value);
		value = parm->nData & 0xFFFF;
		gsw_w32(cdev, (GSWT_MDWRITE_WDATA_OFFSET + GSW30_TOP_OFFSET),
			GSWT_MDWRITE_WDATA_SHIFT, GSWT_MDWRITE_WDATA_SIZE, value);
		value = ((0x1 << 10) | ((parm->nAddressDev & 0x1F) << 5)
			| (parm->nAddressReg & 0x1F));
		/* Special write command, becouse we need to write*/
		/* "MDIO Control Register" once at a time */
		gsw_w32(cdev, (GSWT_MDCTRL_MBUSY_OFFSET + GSW30_TOP_OFFSET), 0, 16, value);
		do {
			udelay(1);
			gsw_r32(cdev, (GSWT_MDCTRL_MBUSY_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCTRL_MBUSY_SHIFT, GSWT_MDCTRL_MBUSY_SIZE, &value);
		} while (value);
	} else {
		do {
				gsw_r32(cdev, (MDIO_CTRL_MBUSY_OFFSET + GSW_TREG_OFFSET),
					MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
		} while (value);
		value = parm->nData & 0xFFFF;
		gsw_w32(cdev, (MDIO_WRITE_WDATA_OFFSET + GSW_TREG_OFFSET),
			MDIO_WRITE_WDATA_SHIFT, MDIO_WRITE_WDATA_SIZE, value);
		value = ((0x1 << 10) | ((parm->nAddressDev & 0x1F) << 5)
			| (parm->nAddressReg & 0x1F));
		/* Special write command, becouse we need to write*/
		/* "MDIO Control Register" once at a time */
		gsw_w32(cdev, (MDIO_CTRL_MBUSY_OFFSET + GSW_TREG_OFFSET), 0, 16, value);
		do {
			gsw_r32(cdev, (MDIO_CTRL_MBUSY_OFFSET + GSW_TREG_OFFSET),
				MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
		} while (value);
	}
	return GSW_statusOk;
}

static inline void ltq_mdelay(int delay)
{
	int i;
	for (i = delay; i > 0; i--)
		udelay(1000);
}

static int force_to_configure_phy_settings(void *cdev, u8 pidx, u8 link_status)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 mdio_stat_reg, phy_addr_reg = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (pidx >= (gswdev->pnum-1))
		return GSW_statusErr;
//	if ((pidx == GSW_3X_SOC_CPU_PORT) || ((pidx == GSW_2X_SOC_CPU_PORT)))
//		return 1;
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET
			+ ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
			GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, &phy_addr_reg);
		gsw_r32(cdev, ((GSWT_MDIO_STAT_1_TXPAUEN_OFFSET
			+ ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
			GSWT_MDIO_STAT_1_TXPAUEN_SHIFT, 16, &mdio_stat_reg);
	} else {
		gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx)
			+ GSW_TREG_OFFSET),
			PHY_ADDR_0_ADDR_SHIFT, 16, &phy_addr_reg);
		gsw_r32(cdev, (MDIO_STAT_0_TXPAUEN_OFFSET
			+ GSW_TREG_OFFSET + pidx),
			MDIO_STAT_0_TXPAUEN_SHIFT, 16, &mdio_stat_reg);
	}
	if (link_status) {
		/* PHY active Status */
		if ((mdio_stat_reg >> 6) & 0x1) {
			u32 temp = 0;
			/* Link Status */
			if ((mdio_stat_reg >> 5) & 0x1) {
				phy_addr_reg &= ~(0xFFE0);
				phy_addr_reg |= (1 << 13); /* Link up */
				temp = ((mdio_stat_reg >> 3) & 0x3); /*Speed */
				phy_addr_reg |= (temp << 11); /*Speed */
				if ((mdio_stat_reg >> 2) & 0x1) /*duplex */ {
					phy_addr_reg |= (0x1 << 9); /*duplex */
				} else {
					phy_addr_reg |= (0x3 << 9);
				}
				/*Receive Pause Enable Status */
				if ((mdio_stat_reg >> 1) & 0x1) {
					/*Receive Pause Enable Status */
					phy_addr_reg |= (0x1 << 5);
				} else {
					phy_addr_reg |= (0x3 << 5);
				}
				/*Transmit Pause Enable Status */
				if ((mdio_stat_reg >> 0) & 0x1) {
					/*Transmit Pause Enable Status */
					phy_addr_reg |= (0x1 << 7);
				} else {
					phy_addr_reg |= (0x3 << 7);
				}
				if (gswdev->gipver == LTQ_GSWIP_3_0) {
					if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
					//Reddy
				} else {
				}
					gsw_w32(cdev,
					((GSWT_PHY_ADDR_1_ADDR_OFFSET
						+ ((pidx - 1) * 4))
						+ GSW30_TOP_OFFSET),
						GSWT_PHY_ADDR_1_ADDR_SHIFT,
						16, phy_addr_reg);
				} else {
					gsw_w32(cdev,
					((PHY_ADDR_0_ADDR_OFFSET - pidx)
						+ GSW_TREG_OFFSET),
						PHY_ADDR_0_ADDR_SHIFT,
						16, phy_addr_reg);
				}
			}
		}
	} else {
		phy_addr_reg &= ~(0xFFE0);
		phy_addr_reg |= (0x3 << 11);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
					//Reddy
				} else {
				}
			gsw_w32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET
				+ ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_ADDR_SHIFT,
				16, phy_addr_reg);
		} else {
			gsw_w32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx)
				+ GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, 16, phy_addr_reg);
		}
	}
	return 1;
}

GSW_return_t GSW_MmdDataRead(void *cdev, GSW_MMD_data_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	GSW_MDIO_data_t mmd_data;
	u32 found = 0, rphy, rmdc, phy_addr, mdc_reg, dev, pidx;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			if ((parm->nAddressDev == 1)) {
				gsw_r32(cdev, (GSWT_PHY_ADDR_1_ADDR_OFFSET + GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_ADDR_SHIFT, GSWT_PHY_ADDR_1_ADDR_SIZE, &phy_addr);
				gsw_r32(cdev, (GSWT_PHY_ADDR_1_ADDR_OFFSET + GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, &rphy);
				found = 1;
				pidx = 1;
			} else {
					return GSW_statusErr;
				}
		} else {
			for (pidx = 1; pidx < gswdev->pnum; pidx++) {
				gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_ADDR_SHIFT, GSWT_PHY_ADDR_1_ADDR_SIZE, &phy_addr);
				if (phy_addr == parm->nAddressDev) {
					found = 1;
					gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
						GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, &rphy);
					break;
				}
			}
		}
	} else {
		for (pidx = 0; pidx < (gswdev->pnum-1); pidx++) {
			gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &phy_addr);
			if (phy_addr == parm->nAddressDev) {
				found = 1;
				gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, 16, &rphy);
				break;
			}
		}
	}
	if (found) {
		force_to_configure_phy_settings(cdev, pidx, 1);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_r32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, &mdc_reg);
		} else {
			gsw_r32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
				MDC_CFG_0_PEN_0_SHIFT, 6, &mdc_reg);
		}
		rmdc = mdc_reg;
		mdc_reg &= ~(1 << pidx);
		dev = ((parm->nAddressReg >> 16) & 0x1F);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, mdc_reg);
		} else {
			gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
				MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
		}
		ltq_mdelay(20);
		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = dev;
		GSW_MDIO_DataWrite(gswdev, &mmd_data);

		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		mmd_data.nData = parm->nAddressReg & 0xFFFF;
		GSW_MDIO_DataWrite(gswdev, &mmd_data);

		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = ((0x4000) | dev);
		GSW_MDIO_DataWrite(gswdev, &mmd_data);

		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		GSW_MDIO_DataRead(gswdev, &mmd_data);
		parm->nData = mmd_data.nData;

//		mdc_reg |= (1 << pidx);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, rphy);
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, rmdc);
		} else {
			gsw_w32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, 16, rphy);
			gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
				MDC_CFG_0_PEN_0_SHIFT, 6, rmdc);
		}
		ltq_mdelay(100);
		force_to_configure_phy_settings(cdev, pidx, 0);
	} else {
		pr_err("(Invalid PHY Address) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_MmdDataWrite(void *cdev, GSW_MMD_data_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	GSW_MDIO_data_t mmd_data;
	u32 found = 0, rphy, rmdc, phy_addr, mdc_reg, dev, pidx;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			if ((parm->nAddressDev == 1)) {
				gsw_r32(cdev, (GSWT_PHY_ADDR_1_ADDR_OFFSET + GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_ADDR_SHIFT, GSWT_PHY_ADDR_1_ADDR_SIZE, &phy_addr);
				gsw_r32(cdev, (GSWT_PHY_ADDR_1_ADDR_OFFSET + GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, &rphy);
				found = 1;
				pidx = 1;
			} else {
					return GSW_statusErr;
				}
		} else {
			for (pidx = 1; pidx < gswdev->pnum; pidx++) {
				gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
					GSWT_PHY_ADDR_1_ADDR_SHIFT, GSWT_PHY_ADDR_1_ADDR_SIZE, &phy_addr);
				if (phy_addr == parm->nAddressDev) {
					found = 1;
					gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
						GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, &rphy);
					break;
				}
			}
		}
	} else {
		for (pidx = 0; pidx < (gswdev->pnum-1); pidx++) {
			gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &phy_addr);
			if (phy_addr == parm->nAddressDev) {
				found = 1;
				gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, 16, &rphy);
				break;
			}
		}
	}

	if (found) {
		force_to_configure_phy_settings(cdev, pidx, 1);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_r32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
				+ GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, &mdc_reg);
		} else {
			gsw_r32(cdev, (MDC_CFG_0_PEN_0_OFFSET
				+ GSW_TREG_OFFSET),
				MDC_CFG_0_PEN_0_SHIFT, 6, &mdc_reg);
		}
		rmdc = mdc_reg;
		mdc_reg &= ~(1 << pidx);
		dev = ((parm->nAddressReg >> 16) & 0x1F);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
				+ GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, mdc_reg);
		} else {
			gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET
				+ GSW_TREG_OFFSET),
				MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
		}
		ltq_mdelay(20);
		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = dev;
		GSW_MDIO_DataWrite(gswdev, &mmd_data);

		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		mmd_data.nData = parm->nAddressReg & 0xFFFF;
		GSW_MDIO_DataWrite(gswdev, &mmd_data);

		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = ((0x4000) | dev);
		GSW_MDIO_DataWrite(gswdev, &mmd_data);

		mmd_data.nAddressDev = parm->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		mmd_data.nData = parm->nData;
		GSW_MDIO_DataWrite(gswdev, &mmd_data);
//		mdc_reg |= (1 << pidx);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_ADDR_SHIFT, 16, rphy);
			gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET + GSW30_TOP_OFFSET),
				GSWT_MDCCFG_0_PEN_1_SHIFT, 6, rmdc);
		} else {
			gsw_w32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT, 16, rphy);
			gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
				MDC_CFG_0_PEN_0_SHIFT, 6, rmdc);
		}
		ltq_mdelay(100);
		force_to_configure_phy_settings(cdev, pidx, 0);
	} else {
		pr_err("(Invalid PHY Address)  %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
	}
	return GSW_statusOk;
}

static GSW_return_t GSW_MMD_MDIO_DataWrite(void *cdev,
	GSW_MDIO_data_t *parm, u8 pidx, u8 dev)
{
	GSW_MDIO_data_t mmd_data;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 mdc_reg;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (pidx >= (gswdev->pnum-1))
		return GSW_statusErr;
	force_to_configure_phy_settings(cdev, pidx, 1);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MDCCFG_0_PEN_1_SHIFT, 6, &mdc_reg);
	} else {
		gsw_r32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_0_PEN_0_SHIFT, 6, &mdc_reg);
	}

	mdc_reg &= ~(1 << pidx);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MDCCFG_0_PEN_1_SHIFT, 6, mdc_reg);
	} else {
		gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
	}
	ltq_mdelay(20);

	mmd_data.nAddressDev = parm->nAddressDev;
	mmd_data.nAddressReg = 0xd;
	mmd_data.nData = dev;
	GSW_MDIO_DataWrite(gswdev, &mmd_data);

	mmd_data.nAddressDev = parm->nAddressDev;
	mmd_data.nAddressReg = 0xe;
	mmd_data.nData = parm->nAddressReg;
	GSW_MDIO_DataWrite(gswdev, &mmd_data);

	mmd_data.nAddressDev = parm->nAddressDev;
	mmd_data.nAddressReg = 0xd;
	mmd_data.nData = ((0x4000) | (dev & 0x1F));
	GSW_MDIO_DataWrite(gswdev, &mmd_data);

	mmd_data.nAddressDev = parm->nAddressDev;
	mmd_data.nAddressReg = 0xe;
	mmd_data.nData = parm->nData;
	GSW_MDIO_DataWrite(gswdev, &mmd_data);

	mdc_reg |= (1 << pidx);
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, (GSWT_MDCCFG_0_PEN_1_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MDCCFG_0_PEN_1_SHIFT, 6, mdc_reg);
	} else {
		gsw_w32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
			MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
	}
	ltq_mdelay(100);
	force_to_configure_phy_settings(cdev, pidx, 0);
	return GSW_statusOk;
}

GSW_return_t GSW_MonitorPortCfgGet(void *cdev, GSW_monitorPortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	/* Get PCE Port Map 1 */
	gsw_r32(cdev, PCE_PMAP_1_MPMAP_OFFSET ,
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, &value);
	if (((value & (1 << pidx)) >> pidx) == 1)
		parm->bMonitorPort = 1;
	else
		parm->bMonitorPort = 0;
	return GSW_statusOk;
}

GSW_return_t GSW_MonitorPortCfgSet(void *cdev, GSW_monitorPortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	gsw_r32(cdev, PCE_PMAP_1_MPMAP_OFFSET ,
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, &value);
	if (parm->bMonitorPort == 1)
		value |= (parm->bMonitorPort << pidx);
	else
		value = (value & ~(1 << pidx));
	gsw_w32(cdev, PCE_PMAP_1_MPMAP_OFFSET ,
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, value);
	return GSW_statusOk;
}

GSW_return_t GSW_PortLinkCfgGet(void *cdev, GSW_portLinkCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	u8 pidx = parm->nPortId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			if (pidx == 15)
				pidx = 0;
			else
				return GSW_statusErr;
		} else {
			if (!parm->nPortId || (parm->nPortId > (gswdev->pnum - 1)))
				return GSW_statusErr;
			pidx = pidx - 1;
		}
	} else {
		if (parm->nPortId > (gswdev->pnum-1))
			return GSW_statusErr;
	}
	gsw_r32(cdev, (MAC_PSTAT_FDUP_OFFSET + (0xC * pidx)),
		MAC_PSTAT_FDUP_SHIFT, MAC_PSTAT_FDUP_SIZE , &value);
	if (value)
		parm->eDuplex = GSW_DUPLEX_FULL;
	else
		parm->eDuplex = GSW_DUPLEX_HALF;
	gsw_r32(cdev, (MAC_PSTAT_GBIT_OFFSET + (0xC * pidx)),
		MAC_PSTAT_GBIT_SHIFT, MAC_PSTAT_GBIT_SIZE , &value);
	if (value) {
		parm->eSpeed = GSW_PORT_SPEED_1000;
	} else {
		gsw_r32(cdev, (MAC_PSTAT_MBIT_OFFSET + (0xC * pidx)),
			MAC_PSTAT_MBIT_SHIFT, MAC_PSTAT_MBIT_SIZE , &value);
		if (value)
			parm->eSpeed = GSW_PORT_SPEED_100;
		else
			parm->eSpeed = GSW_PORT_SPEED_10;
	}
	/* Low-power Idle Mode  configuration*/
	gsw_r32(cdev, (MAC_CTRL_4_LPIEN_OFFSET + (0xC * pidx)),
		MAC_CTRL_4_LPIEN_SHIFT, MAC_CTRL_4_LPIEN_SIZE, &value);
	parm->bLPI = value;
	gsw_r32(cdev, (MAC_PSTAT_LSTAT_OFFSET + (0xC * pidx)),
		MAC_PSTAT_LSTAT_SHIFT, MAC_PSTAT_LSTAT_SIZE , &value);
	if (value)
		parm->eLink = GSW_PORT_LINK_UP;
	else
		parm->eLink = GSW_PORT_LINK_DOWN;
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, ((GSWT_PHY_ADDR_1_LNKST_OFFSET + ((pidx) * 4)) + GSW30_TOP_OFFSET),
			GSWT_PHY_ADDR_1_LNKST_SHIFT, GSWT_PHY_ADDR_1_LNKST_SIZE, &value);
	} else {
		gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				PHY_ADDR_5_LNKST_SHIFT, PHY_ADDR_5_LNKST_SIZE, &value);
	}
	if ((value == 1) || (value == 2)) {
		parm->bLinkForce = 1;
	}

	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, (GSWT_MII_CFG_1_MIIMODE_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_MII_CFG_1_MIIMODE_SHIFT,
			GSWT_MII_CFG_1_MIIMODE_SIZE, &value);
	} else {
		gsw_r32(cdev, (MII_CFG_0_MIIMODE_OFFSET + (0x2 * pidx)
			+ GSW_TREG_OFFSET),
			MII_CFG_0_MIIMODE_SHIFT,
			MII_CFG_0_MIIMODE_SIZE, &value);
	}
	switch (value) {
	case 0:
		parm->eMII_Mode = GSW_PORT_HW_MII;
		parm->eMII_Type = GSW_PORT_PHY;
		break;
	case 1:
		parm->eMII_Mode = GSW_PORT_HW_MII;
		parm->eMII_Type = GSW_PORT_MAC;
		break;
	case 2:
		parm->eMII_Mode = GSW_PORT_HW_RMII;
		parm->eMII_Type = GSW_PORT_PHY;
		break;
	case 3:
		parm->eMII_Mode = GSW_PORT_HW_RMII;
		parm->eMII_Type = GSW_PORT_MAC;
		break;
	case 4:
		parm->eMII_Mode = GSW_PORT_HW_RGMII;
		parm->eMII_Type = GSW_PORT_MAC;
		break;
	}
	parm->eClkMode = GSW_PORT_CLK_NA;
	if (parm->eMII_Mode == GSW_PORT_HW_RMII) {
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_r32(cdev, (GSWT_MII_CFG_1_RMII_OFFSET
				+ GSW30_TOP_OFFSET),
				GSWT_MII_CFG_1_RMII_SHIFT,
				GSWT_MII_CFG_1_RMII_SIZE, &value);
		} else {
			gsw_r32(cdev, (MII_CFG_0_RMII_OFFSET + (0x2 * pidx)
				+ GSW_TREG_OFFSET),
				MII_CFG_0_RMII_SHIFT,
				MII_CFG_0_RMII_SIZE, &value);
		}
		if (value == 1)
			parm->eClkMode = GSW_PORT_CLK_MASTER;
		else
			parm->eClkMode = GSW_PORT_CLK_SLAVE;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PortLinkCfgSet(void *cdev, GSW_portLinkCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 data, phy_addr, phyreg, phy_ctrl = 0, duplex, PEN = 0, PACT = 0;
	u8 pidx = parm->nPortId;
	GSW_MDIO_data_t mddata;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			if (pidx == 15)
				pidx = 1;
			else
				return GSW_statusErr;
		} else {
			if (!parm->nPortId || (parm->nPortId > (gswdev->pnum - 1)))
				return GSW_statusErr;
		}
	} else {
		if (parm->nPortId > (gswdev->pnum-1))
			return GSW_statusErr;
	}

	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, (GSWT_MDCCFG_0_PEN_0_OFFSET + GSW30_TOP_OFFSET),
			(GSWT_MDCCFG_0_PEN_0_SHIFT + pidx),	GSWT_MDCCFG_0_PEN_0_SIZE, &PEN);
		gsw_r32(cdev, ((GSWT_MDIO_STAT_1_PACT_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
			GSWT_MDIO_STAT_1_PACT_SHIFT, GSWT_MDIO_STAT_1_PACT_SIZE, &PACT);
		gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
			0, 16, &phyreg);
	} else {
		gsw_r32(cdev, (MDC_CFG_0_PEN_0_OFFSET + GSW_TREG_OFFSET),
			(MDC_CFG_0_PEN_0_SHIFT + pidx), MDC_CFG_0_PEN_0_SIZE, &PEN);
		gsw_r32(cdev, (MDIO_STAT_0_PACT_OFFSET + GSW_TREG_OFFSET + pidx),
			MDIO_STAT_0_PACT_SHIFT, MDIO_STAT_0_PACT_SIZE, &PACT);
		gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				0, 16, &phyreg);
	}
	/*pr_err("%s:%s:%d PEN:%d, PACT:%d,  phyreg:0x%08x\n",
		__FILE__, __func__, __LINE__,PEN, PACT,phyreg); */
	if ((PEN == 1) && (PACT == 1)) {
		if (parm->bDuplexForce == 1) {
			if (parm->eDuplex == GSW_DUPLEX_FULL) {
				data = DUPLEX_EN;	duplex = DUPLEX_EN;
			} else {
				data = DUPLEX_DIS; duplex = DUPLEX_DIS;
			}
		} else {
			data = DUPLEX_AUTO;
			duplex = DUPLEX_AUTO;
		}
		data = 3; /*default value*/
		if (parm->bSpeedForce == 1) {
			switch (parm->eSpeed) {
			case GSW_PORT_SPEED_10:
				data = 0;
				if (duplex == DUPLEX_DIS)
					phy_ctrl = PHY_AN_ADV_10HDX;
				else
					phy_ctrl = PHY_AN_ADV_10FDX;
				break;
			case GSW_PORT_SPEED_100:
				data = 1;
				if (duplex == DUPLEX_DIS)
					phy_ctrl = PHY_AN_ADV_100HDX;
				else
					phy_ctrl = PHY_AN_ADV_100FDX;
				break;
			case GSW_PORT_SPEED_200:
				return GSW_statusErr;
			case GSW_PORT_SPEED_1000:
				data = 2;
				if (duplex == DUPLEX_DIS)
					phy_ctrl = PHY_AN_ADV_1000HDX;
				else
					phy_ctrl = PHY_AN_ADV_1000FDX;
				break;
			}
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET
				+ ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_ADDR_SHIFT,
				GSWT_PHY_ADDR_1_ADDR_SIZE, &phy_addr);
		} else {
			gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx)
				+ GSW_TREG_OFFSET),
				PHY_ADDR_0_ADDR_SHIFT,
				PHY_ADDR_0_ADDR_SIZE, &phy_addr);
		}
		mddata.nAddressDev = phy_addr;
		GSW_MDIO_DataRead(gswdev, &mddata);
		if ((data == 0) || (data == 1)) {
			mddata.nAddressReg = 4;
			GSW_MDIO_DataRead(gswdev, &mddata);
			mddata.nData &= ~(PHY_AN_ADV_10HDX
				| PHY_AN_ADV_10FDX
				| PHY_AN_ADV_100HDX
				| PHY_AN_ADV_100FDX);
			mddata.nData |= phy_ctrl;
			GSW_MDIO_DataWrite(gswdev, &mddata);
			mddata.nAddressReg = 9;
			GSW_MDIO_DataRead(gswdev, &mddata);
			mddata.nData &= ~(PHY_AN_ADV_1000HDX | PHY_AN_ADV_1000FDX);
			GSW_MDIO_DataWrite(gswdev, &mddata);
		}
		if (data == 2) {
			mddata.nAddressReg = 9;
			GSW_MDIO_DataRead(gswdev, &mddata);
			mddata.nData &= ~(PHY_AN_ADV_1000HDX | PHY_AN_ADV_1000FDX);
			mddata.nData |= phy_ctrl;
			GSW_MDIO_DataWrite(gswdev, &mddata);
			mddata.nAddressReg = 4;
			GSW_MDIO_DataRead(gswdev, &mddata);
			mddata.nData &= ~(PHY_AN_ADV_10HDX
				| PHY_AN_ADV_10FDX
				| PHY_AN_ADV_100HDX
				| PHY_AN_ADV_100FDX);
			GSW_MDIO_DataWrite(gswdev, &mddata);
		}
		if (data == 3) {
			mddata.nAddressReg = 4;
			GSW_MDIO_DataRead(gswdev, &mddata);
			if (duplex == DUPLEX_DIS) {
				mddata.nData &= ~(PHY_AN_ADV_10HDX
					| PHY_AN_ADV_10FDX
					| PHY_AN_ADV_100HDX
					| PHY_AN_ADV_100FDX);
				mddata.nData |= (PHY_AN_ADV_10HDX
					| PHY_AN_ADV_100HDX);
			} else {
				mddata.nData |= (PHY_AN_ADV_10HDX
					| PHY_AN_ADV_10FDX
					| PHY_AN_ADV_100HDX
					| PHY_AN_ADV_100FDX);
			}
			GSW_MDIO_DataWrite(gswdev, &mddata);
			mddata.nAddressReg = 9;
			GSW_MDIO_DataRead(gswdev, &mddata);
			if (duplex == DUPLEX_DIS) {
				mddata.nData &= ~(PHY_AN_ADV_1000HDX
						| PHY_AN_ADV_1000FDX);
				mddata.nData |= (PHY_AN_ADV_1000HDX);
			} else {
				mddata.nData |= (PHY_AN_ADV_1000HDX
						| PHY_AN_ADV_1000FDX);
			}
			GSW_MDIO_DataWrite(gswdev, &mddata);
		}
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
					gsw_w32(cdev, (MAC_CTRL_4_LPIEN_OFFSET + (0xC * (pidx - 1))),
				MAC_CTRL_4_LPIEN_SHIFT,
				MAC_CTRL_4_LPIEN_SIZE, parm->bLPI);
			/* LPI Wait Time for 1G -- 50us */
			gsw_w32(cdev, (MAC_CTRL_4_GWAIT_OFFSET + (0xC * (pidx - 1))),
				MAC_CTRL_4_GWAIT_SHIFT,
				MAC_CTRL_4_GWAIT_SIZE, 0x32);
			/* LPI Wait Time for 100M -- 21us */
			gsw_w32(cdev, (MAC_CTRL_4_WAIT_OFFSET + (0xC * (pidx - 1))),
				MAC_CTRL_4_WAIT_SHIFT,
				MAC_CTRL_4_WAIT_SIZE, 0x15);
		} else {
			gsw_w32(cdev, (MAC_CTRL_4_LPIEN_OFFSET + (0xC * pidx)),
				MAC_CTRL_4_LPIEN_SHIFT,
				MAC_CTRL_4_LPIEN_SIZE, parm->bLPI);
			/* LPI Wait Time for 1G -- 50us */
			gsw_w32(cdev, (MAC_CTRL_4_GWAIT_OFFSET + (0xC * pidx)),
				MAC_CTRL_4_GWAIT_SHIFT,
				MAC_CTRL_4_GWAIT_SIZE, 0x32);
			/* LPI Wait Time for 100M -- 21us */
			gsw_w32(cdev, (MAC_CTRL_4_WAIT_OFFSET + (0xC * pidx)),
				MAC_CTRL_4_WAIT_SHIFT,
				MAC_CTRL_4_WAIT_SIZE, 0x15);
		}
			/* LPI request controlled by data available for  port */
		gsw_w32(cdev, (FDMA_CTRL_LPI_MODE_OFFSET),
			FDMA_CTRL_LPI_MODE_SHIFT,
			FDMA_CTRL_LPI_MODE_SIZE, 0x4);
		mddata.nAddressDev = phy_addr;
		mddata.nAddressReg = 0x3C;
		if (parm->bLPI == 1) {
			mddata.nData = 0x6;
			GSW_MMD_MDIO_DataWrite(gswdev, &mddata, pidx, 0x07);
		} else {
			mddata.nData = 0x0;
			GSW_MMD_MDIO_DataWrite(gswdev, &mddata, pidx, 0x07);
		}
		mddata.nAddressDev = phy_addr;
		mddata.nAddressReg = 0;
		GSW_MDIO_DataRead(gswdev, &mddata);
		mddata.nData = 0x1200;
		data = 0;
		if (parm->bLinkForce == 1) {
			if (parm->eLink == GSW_PORT_LINK_UP) {
				data = 1;
			} else {
				data = 2;
				mddata.nData = 0x800;
			}
		}
		GSW_MDIO_DataWrite(gswdev, &mddata);
	} else {
		if (parm->bDuplexForce == 1) {
			phyreg &= ~(3 << 9);
			if (parm->eDuplex == GSW_DUPLEX_FULL) {
				phyreg |= (1 << 9);
			} else {
				phyreg |= (3 << 9);
			}
		}
		if (parm->bLinkForce == 1) {
			phyreg &= ~(3 << 13);
			if (parm->eLink == GSW_PORT_LINK_UP) {
				phyreg |= (1 << 13);
			} else {
				phyreg |= (2 << 13);
			}
		}
		if (parm->bSpeedForce == 1) {
			phyreg &= ~(3 << 11);
			switch (parm->eSpeed) {
			case GSW_PORT_SPEED_10:
				break;
			case GSW_PORT_SPEED_100:
				phyreg |= (1 << 11);
				break;
			case GSW_PORT_SPEED_200:
				return GSW_statusErr;
			case GSW_PORT_SPEED_1000:
				phyreg |= (2 << 11);
				break;
			}
		}
/*		pr_err("%s:%s:%d PEN:%d, PACT:%d,  phyreg:0x%08x\n",
			__FILE__, __func__, __LINE__,PEN, PACT,phyreg);*/
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET + ((parm->nPortId - 1) * 4)) + GSW30_TOP_OFFSET),
			0, 16, phyreg);
		} else {
			gsw_w32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx) + GSW_TREG_OFFSET),
				0, 16, phyreg);
		}
	}

	data = 4; /*default mode */
	switch (parm->eMII_Mode) {
	case GSW_PORT_HW_MII:
		data = 1;
		if (parm->eMII_Type == GSW_PORT_PHY)
			data = 0;
		break;
	case GSW_PORT_HW_RMII:
		data = 3;
		if (parm->eMII_Type == GSW_PORT_PHY)
			data = 2;
		break;
	case GSW_PORT_HW_RGMII:
		data = 4;
		break;
	case GSW_PORT_HW_GMII:
/*	data = 1; */
		break;
	}
	if (!(parm->eMII_Mode == GSW_PORT_HW_GMII)) {
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, (GSWT_MII_CFG_1_MIIMODE_OFFSET
				+ GSW30_TOP_OFFSET),
				GSWT_MII_CFG_1_MIIMODE_SHIFT,
				GSWT_MII_CFG_1_MIIMODE_SIZE, data);
		} else {
			gsw_w32(cdev, (MII_CFG_0_MIIMODE_OFFSET + (0x2 * pidx)
				+ GSW_TREG_OFFSET),
				MII_CFG_0_MIIMODE_SHIFT,
				MII_CFG_0_MIIMODE_SIZE, data);
		}
	}
	data = 0;
	if (parm->eMII_Mode == GSW_PORT_HW_RMII) {
		if (parm->eClkMode == GSW_PORT_CLK_MASTER)
			data = 1;
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev, (GSWT_MII_CFG_1_RMII_OFFSET
				+ GSW30_TOP_OFFSET),
				GSWT_MII_CFG_1_RMII_SHIFT,
				GSWT_MII_CFG_1_RMII_SIZE, data);
		} else {
			gsw_w32(cdev, (MII_CFG_0_RMII_OFFSET + (0x2 * pidx)
				+ GSW_TREG_OFFSET),
				MII_CFG_0_RMII_SHIFT,
				MII_CFG_0_RMII_SIZE, data);
			}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PortPHY_AddrGet(void *cdev, GSW_portPHY_Addr_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId, num_ports;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (!parm->nPortId)
				return GSW_statusErr;
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			num_ports = gswdev->tpnum;
			if (parm->nPortId == 15)
				pidx = 1;
			else
				return GSW_statusErr;
		}
		else
			num_ports = (gswdev->pnum -1);
		if (parm->nPortId > (num_ports))
			return GSW_statusErr;
		gsw_r32(cdev, ((GSWT_PHY_ADDR_1_ADDR_OFFSET
			+ ((pidx - 1) * 4)) + GSW30_TOP_OFFSET),
			GSWT_PHY_ADDR_1_ADDR_SHIFT,
			GSWT_PHY_ADDR_1_ADDR_SIZE, &value);
	} else {
		if (parm->nPortId >= (gswdev->pnum - 1))
			return GSW_statusErr;
		gsw_r32(cdev, ((PHY_ADDR_0_ADDR_OFFSET - pidx)
			+ GSW_TREG_OFFSET),
			PHY_ADDR_0_ADDR_SHIFT,
			PHY_ADDR_0_ADDR_SIZE, &value);
	}
	parm->nAddressDev = value;
	return GSW_statusOk;
}

GSW_return_t GSW_PortPHY_Query(void *cdev, GSW_portPHY_Query_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (!parm->nPortId || (parm->nPortId > (gswdev->pnum)))
			return GSW_statusErr;
	} else {
		if (parm->nPortId >= (gswdev->pnum - 1))
			return GSW_statusErr;
	}
	gsw_r32(cdev, (MAC_PSTAT_PACT_OFFSET + (0xC * pidx)),
		MAC_PSTAT_PACT_SHIFT, MAC_PSTAT_PACT_SIZE , &value);
	parm->bPHY_Present = value;
	return GSW_statusOk;
}

GSW_return_t GSW_PortRGMII_ClkCfgGet(void *cdev, GSW_portRGMII_ClkCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (!parm->nPortId || (parm->nPortId > (gswdev->pnum)))
			return GSW_statusErr;
	} else {
		if (parm->nPortId >= (gswdev->pnum-1))
			return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, GSWT_PCDU_1_RXDLY_OFFSET
			+ GSW30_TOP_OFFSET,
			GSWT_PCDU_1_RXDLY_SHIFT,
			GSWT_PCDU_1_RXDLY_SIZE, &value);
		parm->nDelayRx = value;
		gsw_r32(cdev, GSWT_PCDU_1_TXDLY_OFFSET
			+ GSW30_TOP_OFFSET,
			GSWT_PCDU_1_TXDLY_SHIFT,
			GSWT_PCDU_1_TXDLY_SIZE, &value);
		parm->nDelayTx = value;
	} else {
		gsw_r32(cdev, PCDU_0_RXDLY_OFFSET + (0x2 * parm->nPortId)
			+ GSW_TREG_OFFSET,
			PCDU_0_RXDLY_SHIFT,
			PCDU_0_RXDLY_SIZE, &value);
		parm->nDelayRx = value;
		gsw_r32(cdev, PCDU_0_TXDLY_OFFSET + (0x2 * parm->nPortId)
			+ GSW_TREG_OFFSET,
			PCDU_0_TXDLY_SHIFT,
			PCDU_0_TXDLY_SIZE, &value);
		parm->nDelayTx = value;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PortRGMII_ClkCfgSet(void *cdev, GSW_portRGMII_ClkCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (!parm->nPortId || (parm->nPortId > (gswdev->pnum)))
			return GSW_statusErr;
	} else {
		if (parm->nPortId >= (gswdev->pnum-1))
			return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, (GSWT_PCDU_1_RXDLY_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_PCDU_1_RXDLY_SHIFT,
			GSWT_PCDU_1_RXDLY_SIZE, parm->nDelayRx);
		gsw_w32(cdev, (GSWT_PCDU_1_TXDLY_OFFSET
			+ GSW30_TOP_OFFSET),
			GSWT_PCDU_1_TXDLY_SHIFT,
			GSWT_PCDU_1_TXDLY_SIZE, parm->nDelayTx);
	} else {
		gsw_w32(cdev, (PCDU_0_RXDLY_OFFSET + (0x2 * parm->nPortId)
			+ GSW_TREG_OFFSET),
			PCDU_0_RXDLY_SHIFT,
			PCDU_0_RXDLY_SIZE, parm->nDelayRx);
		gsw_w32(cdev, (PCDU_0_TXDLY_OFFSET + (0x2 * parm->nPortId)
			+ GSW_TREG_OFFSET),
			PCDU_0_TXDLY_SHIFT,
			PCDU_0_TXDLY_SIZE, parm->nDelayTx);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_PortRedirectGet(void *cdev, GSW_portRedirectCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	gsw_r32(cdev, (PCE_PCTRL_3_EDIR_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_EDIR_SHIFT,
		PCE_PCTRL_3_EDIR_SIZE, &value);
	parm->bRedirectEgress = value;
	if (prdflag > 0)
		parm->bRedirectIngress = 1;
	else
		parm->bRedirectIngress = 0;
	return GSW_statusOk;
}

GSW_return_t GSW_PortRedirectSet(void *cdev, GSW_portRedirectCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	GSW_PCE_rule_t pcrule;
	u8 pidx = parm->nPortId;
	u32 rdport = 0, value = 0, i;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	value |= (1 << gswdev->mpnum);
	rdport = value;
	gsw_w32(cdev, PCE_PMAP_1_MPMAP_OFFSET ,
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, value);
	value = parm->bRedirectEgress;
	gsw_w32(cdev, (PCE_PCTRL_3_EDIR_OFFSET + (0xA * pidx)),
		PCE_PCTRL_3_EDIR_SHIFT,
		PCE_PCTRL_3_EDIR_SIZE, value);
	if (parm->bRedirectIngress == 1)
		prdflag |= (1 << parm->nPortId);
	else
		prdflag &= ~(1 << parm->nPortId);
	for (i = 0; i < gswdev->pnum; i++) {
		if (((prdflag >> i) & 0x1) == 1) {
			memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
			pcrule.pattern.nIndex = (PRD_PRULE_INDEX + i);
			pcrule.pattern.bEnable = 1;
			pcrule.pattern.bPortIdEnable = 1;
			pcrule.pattern.nPortId = i;
			if (parm->bRedirectIngress == 1)
				pcrule.action.ePortMapAction =
				GSW_PCE_ACTION_PORTMAP_ALTERNATIVE;
			pcrule.action.nForwardPortMap = rdport;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(gswdev,
				&gswdev->phandler, &pcrule))
				return GSW_statusErr;
		}  else {
			if (0 != pce_pattern_delete(cdev,
				&gswdev->phandler, PRD_PRULE_INDEX + i))
				return GSW_statusErr;
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_RMON_Clear(void *cdev, GSW_RMON_clear_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 index, num_ports, pidx = parm->nRmonId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->sdev == LTQ_FLOW_DEV_INT_R)
		num_ports = gswdev->tpnum;
	else
		num_ports = gswdev->pnum;

	switch (parm->eRmonType) {
	case GSW_RMON_ALL_TYPE:
		/* Software Reset for All Interface RMON RAM */
		gsw_w32(cdev, BM_RMON_GCTRL_ALLITF_RES_OFFSET,
			BM_RMON_GCTRL_ALLITF_RES_SHIFT,
			BM_RMON_GCTRL_ALLITF_RES_SIZE, 0x1);
		/* Software Reset for PMAC RMON RAM*/
		gsw_w32(cdev, BM_RMON_GCTRL_PMAC_RES_OFFSET,
			BM_RMON_GCTRL_PMAC_RES_SHIFT,
			BM_RMON_GCTRL_PMAC_RES_SIZE, 0x1);
		/*  Software Reset for Meter RMON RAM */
		gsw_w32(cdev, BM_RMON_GCTRL_METER_RES_OFFSET,
			BM_RMON_GCTRL_METER_RES_SHIFT,
			BM_RMON_GCTRL_METER_RES_SIZE, 0x1);
		/*  Software Reset for Redirection RMON RAM */
		gsw_w32(cdev, BM_RMON_GCTRL_RED_RES_OFFSET,
			BM_RMON_GCTRL_RED_RES_SHIFT,
			BM_RMON_GCTRL_RED_RES_SIZE, 0x1);
		/* Reset all port based RMON counter */
		for (index = 0; index < num_ports; index++) {
			gsw_w32(cdev,
				BM_RMON_CTRL_RAM1_RES_OFFSET + (index * 2),
				BM_RMON_CTRL_RAM1_RES_SHIFT,
				BM_RMON_CTRL_RAM1_RES_SIZE, 0x1);
			gsw_w32(cdev,
				BM_RMON_CTRL_RAM2_RES_OFFSET + (index * 2),
				BM_RMON_CTRL_RAM2_RES_SHIFT,
				BM_RMON_CTRL_RAM2_RES_SIZE, 0x1);
			/*  Software Reset for Routing RMON RAM */
			gsw_w32(cdev,
				BM_RMON_CTRL_ROUT_RES_OFFSET + (index * 2),
				BM_RMON_CTRL_ROUT_RES_SHIFT,
				BM_RMON_CTRL_ROUT_RES_SIZE, 0x1);
		}
		break;
	case GSW_RMON_PMAC_TYPE:
		/* Software Reset for PMAC RMON RAM*/
		gsw_w32(cdev, BM_RMON_GCTRL_PMAC_RES_OFFSET,
			BM_RMON_GCTRL_PMAC_RES_SHIFT,
			BM_RMON_GCTRL_PMAC_RES_SIZE, 0x1);
		break;
	case GSW_RMON_PORT_TYPE:
		if (pidx >= num_ports)
			return GSW_statusErr;
		/* Reset all RMON counter */
		gsw_w32(cdev,
			BM_RMON_CTRL_RAM1_RES_OFFSET + (pidx * 2),
			BM_RMON_CTRL_RAM1_RES_SHIFT,
			BM_RMON_CTRL_RAM1_RES_SIZE, 0x1);
		gsw_w32(cdev,
			BM_RMON_CTRL_RAM2_RES_OFFSET + (pidx * 2),
			BM_RMON_CTRL_RAM2_RES_SHIFT,
			BM_RMON_CTRL_RAM2_RES_SIZE, 0x1);
		break;
	case GSW_RMON_METER_TYPE:
		/*  Software Reset for Meter RMON RAM */
		gsw_w32(cdev, BM_RMON_GCTRL_METER_RES_OFFSET,
			BM_RMON_GCTRL_METER_RES_SHIFT,
			BM_RMON_GCTRL_METER_RES_SIZE, 0x1);
		break;
	case GSW_RMON_IF_TYPE:
		/* Interface ID to be Reset */
		gsw_w32(cdev, BM_RMON_GCTRL_ITFID_OFFSET,
			BM_RMON_GCTRL_ITFID_SHIFT,
			BM_RMON_GCTRL_ITFID_SIZE, pidx);
		/*  Software Reset for a Single Interface RMON RAM */
		gsw_w32(cdev, BM_RMON_GCTRL_INT_RES_OFFSET,
			BM_RMON_GCTRL_INT_RES_SHIFT,
			BM_RMON_GCTRL_INT_RES_SIZE, 0x1);
		break;
	case GSW_RMON_ROUTE_TYPE:
		/*  Software Reset for Routing RMON RAM */
		gsw_w32(cdev,
			BM_RMON_CTRL_ROUT_RES_OFFSET + (pidx * 2),
			BM_RMON_CTRL_ROUT_RES_SHIFT,
			BM_RMON_CTRL_ROUT_RES_SIZE, 0x1);
		break;
	case GSW_RMON_REDIRECT_TYPE:
		/*  Software Reset for Redirection RMON RAM */
		gsw_w32(cdev, BM_RMON_GCTRL_RED_RES_OFFSET,
			BM_RMON_GCTRL_RED_RES_SHIFT,
			BM_RMON_GCTRL_RED_RES_SIZE, 0x1);
		break;
	default:
		break;
	}
	return GSW_statusOk;
}

GSW_return_t GSW_RMON_Port_Get(void *cdev, GSW_RMON_Port_cnt_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx, i;
	u32 data, data0, data1, value, bcast_cnt;
	u32 r_frame = 0, r_unicast = 0, r_multicast = 0,
		t_frame = 0, t_unicast = 0, t_multicast = 0;
	u32 rgbcl = 0, rbbcl = 0, tgbcl = 0;
	u64 rgbch = 0, rbbch = 0, tgbch = 0;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	pidx = parm->nPortId;
	if (gswdev) {
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			if (parm->nPortId >= gswdev->tpnum)
				return GSW_statusErr;
		} else {
			if (parm->nPortId >= gswdev->pnum)
				return GSW_statusErr;
		}
	}
	gsw_w32(cdev, BM_PCFG_CNTEN_OFFSET + (pidx * 2),
		BM_PCFG_CNTEN_SHIFT, BM_PCFG_CNTEN_SIZE, 1);
	memset(parm, 0, sizeof(GSW_RMON_Port_cnt_t));
	parm->nPortId = pidx;
	gsw_r32(cdev, BM_RMON_CTRL_BCAST_CNT_OFFSET + (pidx * 2),
		BM_RMON_CTRL_BCAST_CNT_SHIFT,
		BM_RMON_CTRL_BCAST_CNT_SIZE, &bcast_cnt);
	for (i = 0; i < RMON_COUNTER_OFFSET; i++) {
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE, i);
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			if (pidx >= 8)
				value = (pidx + 8);
			else
				value = parm->nPortId;
			gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
				BM_RAM_CTRL_ADDR_SHIFT,
				BM_RAM_CTRL_ADDR_SIZE, value);
		} else {
			gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
				BM_RAM_CTRL_ADDR_SHIFT,
				BM_RAM_CTRL_ADDR_SIZE, pidx);
		}
	gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
		BM_RAM_CTRL_OPMOD_SHIFT,
		BM_RAM_CTRL_OPMOD_SIZE, 0);
		value = 1;
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE, &data0);
		gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
			BM_RAM_VAL_1_VAL1_SHIFT,
			BM_RAM_VAL_1_VAL1_SIZE, &data1);
		data = (data1 << 16 | data0);
		switch (i) {
		case 0x1F: /* Receive Frme Count */
			if (bcast_cnt == 1)
				parm->nRxBroadcastPkts = data;
			else
				parm->nRxGoodPkts = data;
			r_frame = data;
			break;
		case 0x23: /* Receive Unicast Frame Count */
			parm->nRxUnicastPkts = data;
			r_unicast = data;
			break;
		case 0x22: /* Receive Multicast Frame Count1 */
			parm->nRxMulticastPkts = data;
			r_multicast = data;
			break;
		case 0x21: /* Receive CRC Errors Count */
			parm->nRxFCSErrorPkts = data;
			break;
		case 0x1D: /* Receive Undersize Good Count */
			parm->nRxUnderSizeGoodPkts = data;
			break;
		case 0x1B: /* Receive Oversize Good Count */
			parm->nRxOversizeGoodPkts = data;
			break;
		case 0x1E: /* Receive Undersize Bad Count */
			parm->nRxUnderSizeErrorPkts = data;
			break;
		case 0x20: /* Receive Pause Good Count */
			parm->nRxGoodPausePkts = data;
			break;
		case 0x1C: /* Receive Oversize Bad Count */
			parm->nRxOversizeErrorPkts = data;
			break;
		case 0x1A: /* Receive Alignment Errors Count */
			parm->nRxAlignErrorPkts = data;
			break;
		case 0x12: /* Receive Size 64 Frame Count1 */
			parm->nRx64BytePkts = data;
			break;
		case 0x13: /* Receive Size 65-127 Frame Count */
			parm->nRx127BytePkts = data;
			break;
		case 0x14: /* Receive Size 128-255 Frame Count */
			parm->nRx255BytePkts = data;
			break;
		case 0x15: /* Receive Size 256-511 Frame Count */
			parm->nRx511BytePkts = data;
			break;
		case 0x16: /* Receive Size 512-1023 Frame Count */
			parm->nRx1023BytePkts = data;
			break;
		case 0x17: /* Receive Size Greater 1023 Frame Count */
			parm->nRxMaxBytePkts = data;
			break;
		case 0x18: /* Receive Discard (Tail-Drop) Frame Count */
			parm->nRxDroppedPkts = data;
			break;
		case 0x19: /* Receive Drop (Filter) Frame Count */
			parm->nRxFilteredPkts = data;
			break;
		case 0x24: /* Receive Good Byte Count (Low) */
			rgbcl = data;
			break;
		case 0x25: /* Receive Good Byte Count (High) */
			rgbch = data;
			break;
		case 0x26: /* Receive Bad Byte Count (Low) */
			rbbcl = data;
			break;
		case 0x27: /* Receive Bad Byte Count (High) */
			rbbch = data;
			break;
		case 0x0C: /* Transmit Frame Count */
			if (bcast_cnt == 1)
				parm->nTxBroadcastPkts = data;
			else
				parm->nTxGoodPkts = data;
			t_frame = data;
			break;
		case 0x06: /* Transmit Unicast Frame Count */
			parm->nTxUnicastPkts = data;
			t_unicast = data;
			break;
		case 0x07: /* Transmit Multicast Frame Count1 */
			parm->nTxMulticastPkts = data;
			t_multicast = data;
			break;
		case 0x00: /* Transmit Size 64 Frame Count */
			parm->nTx64BytePkts = data;
			break;
		case 0x01: /* Transmit Size 65-127 Frame Count */
			parm->nTx127BytePkts = data;
			break;
		case 0x02: /* Transmit Size 128-255 Frame Count */
			parm->nTx255BytePkts = data;
			break;
		case 0x03: /* Transmit Size 256-511 Frame Count */
			parm->nTx511BytePkts = data;
			break;
		case 0x04: /* Transmit Size 512-1023 Frame Count */
			parm->nTx1023BytePkts = data;
			break;
		case 0x05: /* Transmit Size Greater 1024 Frame Count */
			parm->nTxMaxBytePkts = data;
			break;
		case 0x08: /* Transmit Single Collision Count. */
			parm->nTxSingleCollCount = data;
			break;
		case 0x09: /* Transmit Multiple Collision Count */
			parm->nTxMultCollCount = data;
			break;
		case 0x0A: /* Transmit Late Collision Count */
			parm->nTxLateCollCount = data;
			break;
		case 0x0B: /* Transmit Excessive Collision.*/
			parm->nTxExcessCollCount = data;
			break;
		case 0x0D: /* Transmit Pause Frame Count */
			parm->nTxPauseCount = data;
			break;
		case 0x10: /* Transmit Drop Frame Count */
			parm->nTxDroppedPkts = data;
			break;
		case 0x0E: /* Transmit Good Byte Count (Low) */
			tgbcl = data;
			break;
		case 0x0F: /* Transmit Good Byte Count (High) */
			tgbch = data;
			break;
		case 0x11:
/* Transmit Dropped Packet Cound, based on Congestion Management.*/
			parm->nTxAcmDroppedPkts = data;
			break;
		}
	}
	if (bcast_cnt == 1) {
		parm->nRxGoodPkts = r_frame + r_unicast + r_multicast;
		parm->nTxGoodPkts = t_frame + t_unicast + t_multicast;
	} else {
		/* Receive Broadcase Frme Count */
		parm->nRxBroadcastPkts = r_frame - r_unicast - r_multicast;
		/* Transmit Broadcase Frme Count */
		parm->nTxBroadcastPkts = t_frame - t_unicast - t_multicast;
	}
	/* Receive Good Byte Count */
	parm->nRxGoodBytes = (u64)(((rgbch & 0xFFFFFFFF) << 32) | (rgbcl & 0xFFFFFFFF));
	/* Receive Bad Byte Count */
	parm->nRxBadBytes = (u64)(((rbbch & 0xFFFFFFFF) << 32) | (rbbcl & 0xFFFFFFFF));
	/* Transmit Good Byte Count */
	parm->nTxGoodBytes = (u64)(((tgbch & 0xFFFFFFFF)<< 32) | (tgbcl & 0xFFFFFFFF) );
	return GSW_statusOk;
}

GSW_return_t GSW_RMON_Mode_Set(void *cdev, GSW_RMON_mode_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_3_0) && (gswdev->sdev == LTQ_FLOW_DEV_INT_R)) {
		switch (parm->eRmonType) {
		case GSW_RMON_ALL_TYPE:
			break;
		case GSW_RMON_PMAC_TYPE:
			break;
		case GSW_RMON_PORT_TYPE:
			break;
		case GSW_RMON_METER_TYPE:
			if (parm->eCountMode == GSW_RMON_COUNT_BYTES)
				gsw_w32(cdev, BM_RMON_GCTRL_MRMON_OFFSET, BM_RMON_GCTRL_MRMON_SHIFT, \
					BM_RMON_GCTRL_MRMON_SIZE, 1);
			else
				gsw_w32(cdev, BM_RMON_GCTRL_MRMON_OFFSET, BM_RMON_GCTRL_MRMON_SHIFT, \
					BM_RMON_GCTRL_MRMON_SIZE, 0);
			break;
		case GSW_RMON_IF_TYPE:
			if (parm->eCountMode == GSW_RMON_DROP_COUNT)
				gsw_w32(cdev, BM_RMON_GCTRL_INTMON_OFFSET, BM_RMON_GCTRL_INTMON_SHIFT, \
					BM_RMON_GCTRL_INTMON_SIZE, 1);
			else
				gsw_w32(cdev, BM_RMON_GCTRL_INTMON_OFFSET, BM_RMON_GCTRL_INTMON_SHIFT, \
					BM_RMON_GCTRL_INTMON_SIZE, 0);
			break;
		case GSW_RMON_ROUTE_TYPE:
			if (parm->eCountMode == GSW_RMON_COUNT_BYTES)
				gsw_w32(cdev, PCE_GCTRL_1_RSCNTMD_OFFSET, PCE_GCTRL_1_RSCNTMD_SHIFT, \
					PCE_GCTRL_1_RSCNTMD_SIZE, 1);
			else
				gsw_w32(cdev, PCE_GCTRL_1_RSCNTMD_OFFSET, PCE_GCTRL_1_RSCNTMD_SHIFT, \
					PCE_GCTRL_1_RSCNTMD_SIZE, 0);
			break;
		case GSW_RMON_REDIRECT_TYPE:
			break;
		default:
			break;
		}
	} else {
		return GSW_statusErr;
	}
	return 0;
}

GSW_return_t GSW_RMON_Meter_Get(void *cdev, GSW_RMON_Meter_cnt_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_3_0) && (gswdev->sdev == LTQ_FLOW_DEV_INT_R)) {
		u8 index, addr;
		u32 data, data0, data1, value;
		u8 i, j, m, rpt, loc, hc, crd[MAX_READ] = {0};
		u32 rdcount[MAX_READ], br;
		u8 meterid = parm->nMeterId;
		if (parm->nMeterId >= gswdev->num_of_meters)
			return GSW_statusErr;
		for (index = 0; index <= 3; index++) {
			rpt = 0; hc = 0; br = 0;
			addr = (meterid | (index << 6));
			gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
				BM_RAM_ADDR_ADDR_SHIFT,
				BM_RAM_ADDR_ADDR_SIZE, addr);
		repeat:
			rpt++;
			for ( i = 0; i < MAX_READ; i++)
				crd[i] = 0;
				loc = 0;
			for ( j = 0; j < MAX_READ; j++) {
#ifndef CONFIG_X86_INTEL_CE2700
				ltq_w32(0x8019, gswr_bm_addr);
				ltq_w32(0x0019, gswr_bm_addr);
	//			asm("SYNC");
				ltq_w32(0x8019, gswr_bm_addr);
#endif
				do {
					gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET, BM_RAM_CTRL_BAS_SHIFT,
						BM_RAM_CTRL_BAS_SIZE, &value);
				} while (value);
				for ( i = 0; i < 4; i++) {
					gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
						BM_RAM_VAL_0_VAL0_SHIFT,
						BM_RAM_VAL_0_VAL0_SIZE, &data0);
					gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
						BM_RAM_VAL_1_VAL1_SHIFT,
						BM_RAM_VAL_1_VAL1_SIZE, &data1);
				}
				data = (data1 << 16 | data0);
				switch (index) {
				case 0:
					rdcount[j] = data;
					break;
				case 1:
					rdcount[j] = data;
					break;
				case 2:
					rdcount[j] = data;
					break;
				case 3:
					rdcount[j] = data;
					break;
				}
			}
			switch (index) {
			case 0:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nResCount = br;
			break;
			case 1:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nGreenCount = br;
			break;
			case 2:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nYellowCount = br;
			break;
			case 3:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nRedCount = br;
			break;
			}
		}
	} else {
		return GSW_statusErr;
	}
	return 0;
}


GSW_return_t GSW_RMON_Redirect_Get(void *cdev, GSW_RMON_Redirect_cnt_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_3_0) && (gswdev->sdev == LTQ_FLOW_DEV_INT_R)) {
		u8 index;
		u8 i, j, m, rpt, loc, hc, crd[MAX_READ] = {0};
		u32 rdcount[MAX_READ], br;
		u32 data, data0, data1, value;
		u64  rxbytes_l = 0, rxbytes_h = 0;
		u64 txbytes_l = 0, txbytes_h = 0;
		for (index = 0; index < 8; index++) {
			rpt = 0; hc = 0; br = 0;
			gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET, BM_RAM_ADDR_ADDR_SHIFT,
				BM_RAM_ADDR_ADDR_SIZE, index);
		repeat:
			rpt++;
			for ( i = 0; i < MAX_READ; i++)
				crd[i] = 0;
				loc = 0;
			for ( j = 0; j < MAX_READ; j++) {
#ifndef CONFIG_X86_INTEL_CE2700
				ltq_w32(0x8018, gswr_bm_addr);
				ltq_w32(0x0018, gswr_bm_addr);
	//			asm("SYNC");
				ltq_w32(0x8018, gswr_bm_addr);
#endif
				do {
					gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET, BM_RAM_CTRL_BAS_SHIFT,
						BM_RAM_CTRL_BAS_SIZE, &value);
				} while (value);
				for ( i = 0; i < 4; i++) {
					gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
						BM_RAM_VAL_0_VAL0_SHIFT,
						BM_RAM_VAL_0_VAL0_SIZE, &data0);
					gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
						BM_RAM_VAL_1_VAL1_SHIFT,
						BM_RAM_VAL_1_VAL1_SIZE, &data1);
				}
				data = (data1 << 16 | data0);
				switch (index) {
				case 0:
					rdcount[j] = data;
					break;
				case 1:
					rdcount[j] = data;
					break;
				case 2:
					rdcount[j] = data;
					break;
				case 3:
					rdcount[j] = data;
					break;
				case 4:
					rdcount[j] = data;
					break;
				case 5:
					rdcount[j] = data;
					break;
				case 6:
					rdcount[j] = data;
					break;
				case 7:
					rdcount[j] = data;
					break;
				}
			}
			switch (index) {
			case 0:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nRxPktsCount = br;
				break;
			case 1:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nRxDiscPktsCount = br;
				break;
			case 2:
				calc_credit_byte_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				rxbytes_l = br;
				break;
			case 3:
				calc_credit_byte_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				rxbytes_h = br;
				break;
			case 4:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nTxPktsCount = br;
				break;
			case 5:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nTxDiscPktsCount = br;
				break;
			case 6:
				calc_credit_byte_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				txbytes_l = br;
		/*		parm->nTxBytesCount = br; */
				break;
			case 7:
				calc_credit_byte_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				txbytes_h = br;
		/*		parm->nTxBytesCount |= (br << 32); */
				break;
			}
		}
		parm->nRxBytesCount = (u64)(((rxbytes_h & 0xFFFFFFFF) << 32) | (rxbytes_l & 0xFFFFFFFF));
		parm->nTxBytesCount = (u64)(((txbytes_h & 0xFFFFFFFF) << 32) | (txbytes_l & 0xFFFFFFFF));
	} else {
		return GSW_statusErr;
	}
	return 0;
}

GSW_return_t GSW_RMON_If_Get(void *cdev, GSW_RMON_If_cnt_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_3_0) && (gswdev->sdev == LTQ_FLOW_DEV_INT_R)) {
		u8 index, i, j, m, rpt, loc, hc, crd[MAX_READ] = {0};
		u32 data, data0, data1, value, addr, intmon;
		u32 rdcount[MAX_READ], br;
		u8 ifid = parm->nIfId;
		if (parm->nIfId >= 256)  /* ETHSW_CAP_13 register*/
			return GSW_statusErr;
		gsw_r32(cdev, BM_RMON_GCTRL_INTMON_OFFSET,
			BM_RMON_GCTRL_INTMON_SHIFT,
			BM_RMON_GCTRL_INTMON_SIZE, &intmon);
		if (intmon)
			parm->countMode = GSW_RMON_DROP_COUNT;
		else
			parm->countMode = GSW_RMON_COUNT_BYTES;
		
		for (index = 0; index <= 3; index++) {
			rpt = 0; hc = 0; br = 0;
			addr = (ifid | (index << 8));
			gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET, BM_RAM_ADDR_ADDR_SHIFT,
				BM_RAM_ADDR_ADDR_SIZE, addr);
		repeat:
			rpt++;
			for ( i = 0; i < MAX_READ; i++)
				crd[i] = 0;
				loc = 0;
			for ( j = 0; j < MAX_READ; j++) {
#ifndef CONFIG_X86_INTEL_CE2700
				ltq_w32(0x801A, gswr_bm_addr);
				ltq_w32(0x001A, gswr_bm_addr);
	//			asm("SYNC");
				ltq_w32(0x801A, gswr_bm_addr);
#endif
				do {
						gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
							BM_RAM_CTRL_BAS_SHIFT,
							BM_RAM_CTRL_BAS_SIZE, &value);
					} while (value);
				for ( i = 0; i < 4; i++) {
					gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
						BM_RAM_VAL_0_VAL0_SHIFT,
						BM_RAM_VAL_0_VAL0_SIZE, &data0);
					gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
						BM_RAM_VAL_1_VAL1_SHIFT,
						BM_RAM_VAL_1_VAL1_SIZE, &data1);
				}
				data = (data1 << 16 | data0);
				switch (index) {
				case 0:
					rdcount[j] = data;
					break;
				case 1:
					rdcount[j] = data;
					break;
				case 2:
					rdcount[j] = data;
					break;
				case 3:
					rdcount[j] = data;
					break;
				}
			}
			switch (index) {
				case 0:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nRxPktsCount = br;
				break;
				case 1:
				if (intmon)
					calc_credit_value(rdcount, &m, &loc);
				else
					calc_credit_byte_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				if (intmon) {
						parm->nRxDiscPktsCount = br;
						parm->nRxBytesCount = 0;
					} else {
						parm->nRxBytesCount = br;
						parm->nRxDiscPktsCount = 0;
					}
				break;
				case 2:
				calc_credit_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				parm->nTxPktsCount = br;
				break;
				case 3:
				if (intmon)
					calc_credit_value(rdcount, &m, &loc);
				else
					calc_credit_byte_value(rdcount, &m, &loc);
				if ( m > hc) {
					hc = m;
					br = rdcount[loc];
				}
				if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
					goto repeat;
				}
				if (intmon) {
					parm->nTxDiscPktsCount = br;
					parm->nTxBytesCount = 0;
				} else {
					parm->nTxBytesCount = br;
					parm->nTxDiscPktsCount = 0;
				}
				break;
			}
		}
	} else {
		return GSW_statusErr;
	}
	return 0;
}

GSW_return_t GSW_RMON_Route_Get(void *cdev, GSW_RMON_Route_cnt_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_3_0) && (gswdev->sdev == LTQ_FLOW_DEV_INT_R)) {
		u8 index, addr;
		u8 i, j, m, rpt, loc, hc, crd[MAX_READ] = {0};
		u32 rdcount[MAX_READ], br;
		u32 data, data0, data1, value;
		u8 rid = parm->nRoutedPortId;
		if (parm->nRoutedPortId >= 16 /* gswdev->tpnum*/)
			return GSW_statusErr;
		for (index = 0; index <= 13; index++) {
			addr = (rid | (index << 4));
			gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET, BM_RAM_ADDR_ADDR_SHIFT,
				BM_RAM_ADDR_ADDR_SIZE, addr);
			rpt = 0; hc = 0; br = 0;
		repeat:
			rpt++;
			for ( i = 0; i < MAX_READ; i++)
				crd[i] = 0;
				loc = 0;
			for ( j = 0; j < MAX_READ; j++) {
#ifndef CONFIG_X86_INTEL_CE2700
				ltq_w32(0x801B, gswr_bm_addr);
				ltq_w32(0x001B, gswr_bm_addr);
	//			asm("SYNC");
				ltq_w32(0x801B, gswr_bm_addr);
#endif
				do {
					gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET, BM_RAM_CTRL_BAS_SHIFT,
						BM_RAM_CTRL_BAS_SIZE, &value);
				} while (value);
				for ( i = 0; i < 4; i++) {
					gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
						BM_RAM_VAL_0_VAL0_SHIFT,
						BM_RAM_VAL_0_VAL0_SIZE, &data0);
					gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
						BM_RAM_VAL_1_VAL1_SHIFT,
						BM_RAM_VAL_1_VAL1_SIZE, &data1);
				}
				data = (data1 << 16 | data0);
				switch (index) {
				case 0:
					rdcount[j] = data;
					break;
				case 1:
					rdcount[j] = data;
					break;
				case 2:
					rdcount[j] = data;
					break;
				case 3:
					rdcount[j] = data;
					break;
				case 4:
					rdcount[j] = data;
					break;
				case 5:
					rdcount[j] = data;
					break;
				case 6:
					rdcount[j] = data;
					break;
				case 7:
					rdcount[j] = data;
					break;
				case 8:
					rdcount[j] = data;
					break;
				case 9:
					rdcount[j] = data;
					break;
				case 0x0A:
					rdcount[j] = data;
					break;
				case 0x0B:
					rdcount[j] = data;
					break;
				case 0x0C:
					rdcount[j] = data;
					break;
				case 0x0D:
					rdcount[j] = data;
					break;
				}
			}
			switch (index) {
				case 0:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxUCv4UDPPktsCount = br;
					break;
				case 1:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxUCv4TCPPktsCount = br;
					break;
				case 2:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxMCv4PktsCount = br;
					break;
				case 3:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxUCv6UDPPktsCount = br;
					break;
				case 4:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxUCv6TCPPktsCount = br;
					break;
				case 5:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxMCv6PktsCount = br;
					break;
				case 6:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxIPv4BytesCount = br;
					break;
				case 7:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxIPv6BytesCount = br;
					break;
				case 8:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxCpuPktsCount = br;
					break;
				case 9:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxCpuBytesCount = br;
					break;
				case 0x0A:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxPktsDropCount = br;
					break;
				case 0x0B:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nRxBytesDropCount = br;
					break;
				case 0x0C:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nTxPktsCount = br;
					break;
				case 0x0D:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nTxBytesCount = br;
					break;
			}
		}
	} else {
		return GSW_statusErr;
	}
	return 0;
}

int GSW_PMAC_CountGet(void *cdev, GSW_PMAC_Cnt_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((gswdev->gipver == LTQ_GSWIP_3_0) && (gswdev->sdev == LTQ_FLOW_DEV_INT_R)) {
		u8 index;
		u8 i, j, m, rpt, loc, hc, crd[MAX_READ] = {0};
		u32 rdcount[MAX_READ], br;
		u32 data0, data1, data, value;
		if (parm->nTxDmaChanId >= 16)
			return GSW_statusErr;
		for (index = 0; index < 4; index++) {
			value = ((index << 4) | (parm->nTxDmaChanId & 0xF));
			gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET, BM_RAM_ADDR_ADDR_SHIFT,
				BM_RAM_ADDR_ADDR_SIZE, value);
			rpt = 0; hc = 0; br = 0;
		repeat:
			rpt++;
			for ( i = 0; i < MAX_READ; i++)
				crd[i] = 0;
			loc = 0;
			for ( j = 0; j < MAX_READ; j++) {
#ifndef CONFIG_X86_INTEL_CE2700
				ltq_w32(0x801C, gswr_bm_addr);
				ltq_w32(0x001C, gswr_bm_addr);
//				asm("SYNC");
				ltq_w32(0x801C, gswr_bm_addr);
#endif
				do {
					gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET, BM_RAM_CTRL_BAS_SHIFT,
						BM_RAM_CTRL_BAS_SIZE, &value);
				} while (value);
				for ( i = 0; i < 4; i++) {
					gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
						BM_RAM_VAL_0_VAL0_SHIFT,
						BM_RAM_VAL_0_VAL0_SIZE, &data0);
					gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
						BM_RAM_VAL_1_VAL1_SHIFT,
						BM_RAM_VAL_1_VAL1_SIZE, &data1);
				}
				data = (data1 << 16 | data0);
				switch (index) {
				case 0:
					rdcount[j] = data;
					break;
				case 1:
					rdcount[j] = data;
					break;
				case 2:
					rdcount[j] = data;
					break;
				case 3:
					rdcount[j] = data;
					break;
				}
			}
			switch (index) {
				case 0:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nDiscPktsCount = br;
					break;
				case 1:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nDiscBytesCount = br;
					break;
				case 2:
					calc_credit_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nChkSumErrPktsCount = br;
					break;
				case 3:
					calc_credit_byte_value(rdcount, &m, &loc);
					if ( m > hc) {
						hc = m;
						br = rdcount[loc];
					}
					if ((m < REPEAT_L) && (rpt < REPEAT_M)) {
						goto repeat;
					}
					parm->nChkSumErrBytesCount = br;
					break;
			}
		}
	} else {
		return GSW_statusErr;
	}
	return 0;
}

GSW_return_t GSW_PMAC_GLBL_CfgSet(void *cdev, GSW_PMAC_Glbl_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_w32(cdev, PMAC_CTRL_0_PADEN_OFFSET, PMAC_CTRL_0_PADEN_SHIFT,
			PMAC_CTRL_0_PADEN_SIZE, parm->bPadEna);
		gsw_w32(cdev, PMAC_CTRL_0_VPADEN_OFFSET, PMAC_CTRL_0_VPADEN_SHIFT,
			PMAC_CTRL_0_VPADEN_SIZE, parm->bVPadEna);
		gsw_w32(cdev, PMAC_CTRL_0_VPAD2EN_OFFSET, PMAC_CTRL_0_VPAD2EN_SHIFT,
			PMAC_CTRL_0_VPAD2EN_SIZE, parm->bSVPadEna);
		gsw_w32(cdev, PMAC_CTRL_0_FCS_OFFSET, PMAC_CTRL_0_FCS_SHIFT,
			PMAC_CTRL_0_FCS_SIZE, parm->bTxFCSDis);
		gsw_w32(cdev, PMAC_CTRL_0_CHKREG_OFFSET, PMAC_CTRL_0_CHKREG_SHIFT,
			PMAC_CTRL_0_CHKREG_SIZE, parm->bIPTransChkRegDis);
		gsw_w32(cdev, PMAC_CTRL_0_CHKVER_OFFSET, PMAC_CTRL_0_CHKVER_SHIFT,
			PMAC_CTRL_0_CHKVER_SIZE, parm->bIPTransChkVerDis);
		if (parm->bJumboEna == 1) {
			gsw_w32(cdev, PMAC_CTRL_2_MLEN_OFFSET, PMAC_CTRL_2_MLEN_SHIFT,
				PMAC_CTRL_2_MLEN_SIZE, 1);
			gsw_w32(cdev, PMAC_CTRL_3_JUMBO_OFFSET, PMAC_CTRL_3_JUMBO_SHIFT,
				PMAC_CTRL_3_JUMBO_SIZE, parm->nMaxJumboLen);
		} else {
			gsw_w32(cdev, PMAC_CTRL_2_MLEN_OFFSET, PMAC_CTRL_2_MLEN_SHIFT,
				PMAC_CTRL_2_MLEN_SIZE, 0);
			gsw_w32(cdev, PMAC_CTRL_3_JUMBO_OFFSET, PMAC_CTRL_3_JUMBO_SHIFT,
				PMAC_CTRL_3_JUMBO_SIZE, 0x8F8);
		}
		gsw_w32(cdev, PMAC_CTRL_2_LCHKL_OFFSET, PMAC_CTRL_2_LCHKL_SHIFT,
			PMAC_CTRL_2_LCHKL_SIZE, parm->bLongFrmChkDis);
		switch(parm->eShortFrmChkType) {
			case GSW_PMAC_SHORT_LEN_DIS:
				gsw_w32(cdev, PMAC_CTRL_2_LCHKS_OFFSET, PMAC_CTRL_2_LCHKS_SHIFT,
					PMAC_CTRL_2_LCHKS_SIZE, 0);
				break;
			case GSW_PMAC_SHORT_LEN_ENA_UNTAG:
				gsw_w32(cdev, PMAC_CTRL_2_LCHKS_OFFSET, PMAC_CTRL_2_LCHKS_SHIFT,
					PMAC_CTRL_2_LCHKS_SIZE, 1);
				break;
			case GSW_PMAC_SHORT_LEN_ENA_TAG:
				gsw_w32(cdev, PMAC_CTRL_2_LCHKS_OFFSET, PMAC_CTRL_2_LCHKS_SHIFT,
					PMAC_CTRL_2_LCHKS_SIZE, 2);
				break;
			case GSW_PMAC_SHORT_LEN_RESERVED:
				gsw_w32(cdev, PMAC_CTRL_2_LCHKS_OFFSET, PMAC_CTRL_2_LCHKS_SHIFT,
					PMAC_CTRL_2_LCHKS_SIZE, 3);
				break;
		}
		gsw_w32(cdev, PMAC_CTRL_4_FLAGEN_OFFSET, PMAC_CTRL_4_FLAGEN_SHIFT,
			PMAC_CTRL_4_FLAGEN_SIZE, parm->bProcFlagsEgCfgEna);
	}

	return 0;
}

GSW_return_t GSW_PMAC_GLBL_CfgGet(void *cdev, GSW_PMAC_Glbl_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 regval;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		gsw_r32(cdev, PMAC_CTRL_0_PADEN_OFFSET, PMAC_CTRL_0_PADEN_SHIFT,
			PMAC_CTRL_0_PADEN_SIZE, &parm->bPadEna);
		gsw_r32(cdev, PMAC_CTRL_0_VPADEN_OFFSET, PMAC_CTRL_0_VPADEN_SHIFT,
			PMAC_CTRL_0_VPADEN_SIZE, &parm->bVPadEna);
		gsw_r32(cdev, PMAC_CTRL_0_VPAD2EN_OFFSET, PMAC_CTRL_0_VPAD2EN_SHIFT,
			PMAC_CTRL_0_VPAD2EN_SIZE, &parm->bSVPadEna);
		gsw_r32(cdev, PMAC_CTRL_0_FCS_OFFSET, PMAC_CTRL_0_FCS_SHIFT,
			PMAC_CTRL_0_FCS_SIZE, &parm->bTxFCSDis);
		gsw_r32(cdev, PMAC_CTRL_0_CHKREG_OFFSET, PMAC_CTRL_0_CHKREG_SHIFT,
			PMAC_CTRL_0_CHKREG_SIZE, &parm->bIPTransChkRegDis);
		gsw_r32(cdev, PMAC_CTRL_0_CHKVER_OFFSET, PMAC_CTRL_0_CHKVER_SHIFT,
			PMAC_CTRL_0_CHKVER_SIZE, &parm->bIPTransChkVerDis);
		gsw_r32(cdev, PMAC_CTRL_2_MLEN_OFFSET, PMAC_CTRL_2_MLEN_SHIFT,
			PMAC_CTRL_2_MLEN_SIZE, &parm->bJumboEna);
		gsw_r32(cdev, PMAC_CTRL_3_JUMBO_OFFSET, PMAC_CTRL_3_JUMBO_SHIFT,
			PMAC_CTRL_3_JUMBO_SIZE, &regval);
		parm->nMaxJumboLen = regval;
		gsw_r32(cdev, PMAC_CTRL_2_LCHKL_OFFSET, PMAC_CTRL_2_LCHKL_SHIFT,
			PMAC_CTRL_2_LCHKL_SIZE, &parm->bLongFrmChkDis);
		gsw_r32(cdev, PMAC_CTRL_2_LCHKS_OFFSET, PMAC_CTRL_2_LCHKS_SHIFT,
			PMAC_CTRL_2_LCHKS_SIZE, &regval);
		parm->eShortFrmChkType = regval;
		gsw_r32(cdev, PMAC_CTRL_4_FLAGEN_OFFSET, PMAC_CTRL_4_FLAGEN_SHIFT,
			PMAC_CTRL_4_FLAGEN_SIZE, &parm->bProcFlagsEgCfgEna);
	}
	return 0;
}

int xwayflow_pmac_table_read(void *cdev, pmtbl_prog_t *ptdata)
{
	u32 value;
	do {
		gsw_r32(cdev, PMAC_TBL_CTRL_BAS_OFFSET,
			PMAC_TBL_CTRL_BAS_SHIFT,
			PMAC_TBL_CTRL_BAS_SIZE, &value);
	}	while (value != 0);
	gsw_w32(cdev, PMAC_TBL_ADDR_ADDR_OFFSET,
		PMAC_TBL_ADDR_ADDR_SHIFT,
		PMAC_TBL_ADDR_ADDR_SIZE, ptdata->ptaddr);
	gsw_w32(cdev, PMAC_TBL_CTRL_ADDR_OFFSET,
		PMAC_TBL_CTRL_ADDR_SHIFT,
		PMAC_TBL_CTRL_ADDR_SIZE, ptdata->ptcaddr);
	gsw_w32(cdev, PMAC_TBL_CTRL_OPMOD_OFFSET,
		PMAC_TBL_CTRL_OPMOD_SHIFT,
		PMAC_TBL_CTRL_OPMOD_SIZE, 0 /* ptdata->op_mode */);
	gsw_w32(cdev, PMAC_TBL_CTRL_BAS_OFFSET,
		PMAC_TBL_CTRL_BAS_SHIFT,
		PMAC_TBL_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(cdev, PMAC_TBL_CTRL_BAS_OFFSET,
			PMAC_TBL_CTRL_BAS_SHIFT,
			PMAC_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_r32(cdev, PMAC_TBL_VAL_4_VAL4_OFFSET,
		PMAC_TBL_VAL_4_VAL4_SHIFT,
		PMAC_TBL_VAL_4_VAL4_SIZE, &value);
	ptdata->val[4] = value;
	gsw_r32(cdev, PMAC_TBL_VAL_3_VAL3_OFFSET,
		PMAC_TBL_VAL_3_VAL3_SHIFT,
		PMAC_TBL_VAL_3_VAL3_SIZE, &value);
	ptdata->val[3] = value;
	gsw_r32(cdev, PMAC_TBL_VAL_2_VAL2_OFFSET,
		PMAC_TBL_VAL_2_VAL2_SHIFT,
		PMAC_TBL_VAL_2_VAL2_SIZE, &value);
	ptdata->val[2] = value;
	gsw_r32(cdev, PMAC_TBL_VAL_1_VAL1_OFFSET,
		PMAC_TBL_VAL_1_VAL1_SHIFT,
		PMAC_TBL_VAL_1_VAL1_SIZE, &value);
	ptdata->val[1] = value;
	gsw_r32(cdev, PMAC_TBL_VAL_0_VAL0_OFFSET,
		PMAC_TBL_VAL_0_VAL0_SHIFT,
		PMAC_TBL_VAL_0_VAL0_SIZE, &value);
	ptdata->val[0] = value;
	return 0;
}

int xwayflow_pmac_table_write(void *cdev, pmtbl_prog_t *ptdata)
{
	u32 value;
	do {
		gsw_r32(cdev, PMAC_TBL_CTRL_BAS_OFFSET,
			PMAC_TBL_CTRL_BAS_SHIFT,
			PMAC_TBL_CTRL_BAS_SIZE, &value);
	} while (value);
	gsw_w32(cdev, PMAC_TBL_ADDR_ADDR_OFFSET,
		PMAC_TBL_ADDR_ADDR_SHIFT,
		PMAC_TBL_ADDR_ADDR_SIZE, ptdata->ptaddr);
	gsw_w32(cdev, PMAC_TBL_CTRL_ADDR_OFFSET,
		PMAC_TBL_CTRL_ADDR_SHIFT,
		PMAC_TBL_CTRL_ADDR_SIZE, ptdata->ptcaddr);
	gsw_w32(cdev, PMAC_TBL_CTRL_OPMOD_OFFSET,
		PMAC_TBL_CTRL_OPMOD_SHIFT,
	PMAC_TBL_CTRL_OPMOD_SIZE, 1 /* ptdata->op_mode */);
	gsw_w32(cdev, PMAC_TBL_VAL_4_VAL4_OFFSET,
		PMAC_TBL_VAL_4_VAL4_SHIFT,
		PMAC_TBL_VAL_4_VAL4_SIZE, ptdata->val[4]);
	gsw_w32(cdev, PMAC_TBL_VAL_3_VAL3_OFFSET,
		PMAC_TBL_VAL_3_VAL3_SHIFT,
		PMAC_TBL_VAL_3_VAL3_SIZE, ptdata->val[3]);
	gsw_w32(cdev, PMAC_TBL_VAL_2_VAL2_OFFSET,
		PMAC_TBL_VAL_2_VAL2_SHIFT,
		PMAC_TBL_VAL_2_VAL2_SIZE, ptdata->val[2]);
	gsw_w32(cdev, PMAC_TBL_VAL_1_VAL1_OFFSET,
		PMAC_TBL_VAL_1_VAL1_SHIFT,
		PMAC_TBL_VAL_1_VAL1_SIZE, ptdata->val[1]);
	gsw_w32(cdev, PMAC_TBL_VAL_0_VAL0_OFFSET,
		PMAC_TBL_VAL_0_VAL0_SHIFT,
		PMAC_TBL_VAL_0_VAL0_SIZE, ptdata->val[0]);
	gsw_w32(cdev, PMAC_TBL_CTRL_BAS_OFFSET,
		PMAC_TBL_CTRL_BAS_SHIFT,
		PMAC_TBL_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(cdev, PMAC_TBL_CTRL_BAS_OFFSET,
			PMAC_TBL_CTRL_BAS_SHIFT,
			PMAC_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	return 0;
}

int GSW_PMAC_BM_CfgSet(void *cdev, GSW_PMAC_BM_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pmtbl_prog_t pmtbl;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pmtbl, 0, sizeof(pmtbl));
		pmtbl.ptaddr = (parm->nTxDmaChanId & 0x0F);
		/* RX Port Congestion Mask (bit 15:0) */
		pmtbl.val[0] = (parm->rxPortMask & 0xFFFF);
		/* TX Queue Congestion Mask (bit 15:0) */
		pmtbl.val[1] = (parm->txQMask & 0xFFFF);
		/* TX Queue Congestion Mask (bit 31:16)  */
		pmtbl.val[2] = ((parm->txQMask >> 16) & 0xFFFF);
		pmtbl.ptcaddr = PMAC_BPMAP_INDEX;
		xwayflow_pmac_table_write(cdev, &pmtbl);
	}

	return 0;
}

int GSW_PMAC_BM_CfgGet(void *cdev, GSW_PMAC_BM_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pmtbl_prog_t pmtbl;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pmtbl, 0, sizeof(pmtbl));

		pmtbl.ptaddr = (parm->nTxDmaChanId & 0x0F);
		pmtbl.ptcaddr	= PMAC_BPMAP_INDEX;
		xwayflow_pmac_table_read(cdev, &pmtbl);

		/* RX Port Congestion Mask (bit 15:0) */
		parm->rxPortMask = (pmtbl.val[0]);
		/* TX Queue Congestion Mask (bit 15:0) */
		parm->txQMask = (pmtbl.val[1]);
		/* TX Queue Congestion Mask (bit 31:16)  */
		parm->txQMask |= (pmtbl.val[2]) << 16;
	}

	return 0;
}

int GSW_PMAC_IG_CfgSet(void *cdev, GSW_PMAC_Ig_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pmtbl_prog_t pmtbl;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pmtbl, 0, sizeof(pmtbl));
		pmtbl.ptaddr = (parm->nTxDmaChanId & 0x0F);

		/* Default PMAC Header Bytes */
		pmtbl.val[0] = (((parm->defPmacHdr[0] & 0xFF) << 8)
			| (parm->defPmacHdr[1] & 0xFF));
		pmtbl.val[1] = (((parm->defPmacHdr[2] & 0xFF) << 8)
			| (parm->defPmacHdr[3] & 0xFF));
		pmtbl.val[2] = (((parm->defPmacHdr[4] & 0xFF) << 8)
			| (parm->defPmacHdr[5] & 0xFF));
		pmtbl.val[3] = (((parm->defPmacHdr[6] & 0xFF) << 8)
			| (parm->defPmacHdr[7] & 0xFF));

		/* Packet has PMAC header or not */
		if (parm->bPmacPresent)
			pmtbl.val[4] |= (1 << 0);

		/* Source Port Id from default PMAC header */
		if (parm->bSpIdDefault)
			pmtbl.val[4] |= (1 << 1);

		/* Sub_Interface Id  Info from default PMAC header  */
		if (parm->bSubIdDefault)
			pmtbl.val[4] |= (1 << 2);

		/* Class Enable info from default PMAC header  */
		if (parm->bClassEna)
			pmtbl.val[4] |= (1 << 3);

		/* Port Map Enable info from default PMAC header */
		if (parm->bPmapEna)
			pmtbl.val[4] |= (1 << 4);

		/* Class Info from default PMAC header */
		if (parm->bClassDefault)
			pmtbl.val[4] |= (1 << 5);

		/* Port Map info from default PMAC header  */
		if (parm->bPmapDefault)
			pmtbl.val[4] |= (1 << 6);

		/* Error set packets to be discarded  */
		if (parm->bErrPktsDisc)
			pmtbl.val[4] |= (1 << 7);

		pmtbl.ptcaddr	= PMAC_IGCFG_INDEX;
		xwayflow_pmac_table_write(cdev, &pmtbl);
	}

	return 0;
}


int GSW_PMAC_IG_CfgGet(void *cdev, GSW_PMAC_Ig_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pmtbl_prog_t pmtbl;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pmtbl, 0, sizeof(pmtbl));
		pmtbl.ptaddr = (parm->nTxDmaChanId & 0x0F);
		pmtbl.ptcaddr	= PMAC_IGCFG_INDEX;
		xwayflow_pmac_table_read(cdev, &pmtbl);

		/* Default PMAC Header Bytes */
		parm->defPmacHdr[0] = ((pmtbl.val[0] >> 8) & 0xFF);
		parm->defPmacHdr[1] = (pmtbl.val[0] & 0xFF);
		parm->defPmacHdr[2] = ((pmtbl.val[1] >> 8) & 0xFF);
		parm->defPmacHdr[3] = (pmtbl.val[1] & 0xFF);
		parm->defPmacHdr[4] = ((pmtbl.val[2] >> 8) & 0xFF);
		parm->defPmacHdr[5] = (pmtbl.val[2] & 0xFF);
		parm->defPmacHdr[6] = ((pmtbl.val[3] >> 8) & 0xFF);
		parm->defPmacHdr[7] = (pmtbl.val[3] & 0xFF);

		/* Packet has PMAC header or not */
		if ((pmtbl.val[4] >> 0) & 0x1)
			parm->bPmacPresent = 1;

		/* Source Port Id from default PMAC header */
		if ((pmtbl.val[4] >> 1) & 0x1)
			parm->bSpIdDefault = 1;

		/* Sub_Interface Id  Info from default PMAC header  */
		if ((pmtbl.val[4] >> 2) & 0x1)
			parm->bSubIdDefault = 1;

		/* Class Enable info from default PMAC header  */
		if ((pmtbl.val[4] >> 3) & 0x1)
			parm->bClassEna = 1;

		/* Port Map Enable info from default PMAC header */
		if ((pmtbl.val[4] >> 4) & 0x1)
			parm->bPmapEna = 1;

		/* Class Info from default PMAC header */
		if ((pmtbl.val[4] >> 5) & 0x1)
			parm->bClassDefault = 1;

		/* Port Map info from default PMAC header  */
		if ((pmtbl.val[4] >> 6) & 0x1)
			parm->bPmapDefault = 1;

		/* Error set packets to be discarded  */
		if ((pmtbl.val[4] >> 7) & 0x1)
			parm->bErrPktsDisc = 1;
	}

	return 0;
}

int GSW_PMAC_EG_CfgSet(void *cdev, GSW_PMAC_Eg_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pmtbl_prog_t pmtbl;
	u32 regval;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pmtbl, 0, sizeof(pmtbl));
		pmtbl.ptaddr = parm->nDestPortId & 0x0F;
		pmtbl.ptaddr |= (parm->nFlowIDMsb & 0x03) << 8;
		gsw_r32(cdev, PMAC_CTRL_4_FLAGEN_OFFSET, PMAC_CTRL_4_FLAGEN_SHIFT,
			PMAC_CTRL_4_FLAGEN_SIZE, &regval);
/*		if (parm->bTCEnable == 1) { */
		if ((parm->bProcFlagsSelect == 0) && (regval == 0)) {
			pmtbl.ptaddr |= (parm->nTrafficClass & 0x0F) << 4;
		} else if ((parm->bProcFlagsSelect == 1) && (regval == 1)) {
			/* MPE-1 Marked Flag  */
			if (parm->bMpe1Flag)
				pmtbl.ptaddr |= (1 << 4);
			/* MPE-2 Marked Flag  */
			if (parm->bMpe2Flag)
				pmtbl.ptaddr |= (1 << 5);
			/* Cryptography Encryption Action Flag  */
			if (parm->bEncFlag)
				pmtbl.ptaddr |= (1 << 6);
			/* Cryptography Decryption Action Flag  */
			if (parm->bDecFlag)
				pmtbl.ptaddr |= (1 << 7);
		} else {
			return GSW_statusNoSupport;
		}
		/*  nRes2DW0 -- DW0 (bit 14 to 13),*/
		/*nRes1DW0 -- DW0 (bit 31 to 29), nResDW1 -- DW1 (bit 7 to 4)*/
		pmtbl.val[0] = ((parm->nRes2DW0 & 0x3) | ((parm->nRes1DW0 & 0x3) << 2)
			| ((parm->nResDW1 & 0x0F) << 5));
		/* Receive DMA Channel Identifier (0..7) */
		pmtbl.val[1] = (parm->nRxDmaChanId & 0x0F);

		/* Packet has PMAC */
		if (parm->bPmacEna == 1)
			pmtbl.val[2] |= (1 << 0);

		/* Packet has FCS */
		if (parm->bFcsEna == 1)
			pmtbl.val[2] |= (1 << 1);

		/* To remove L2 header & additional bytes  */
		if (parm->bRemL2Hdr == 1) {
			pmtbl.val[2] |= (1 << 2);
		/* No. of bytes to be removed after Layer-2 Header,*/
			/*valid when bRemL2Hdr is set  */
			pmtbl.val[2] |= ((parm->numBytesRem & 0xFF) << 8);
		}
		pmtbl.ptcaddr	= PMAC_EGCFG_INDEX;
		xwayflow_pmac_table_write(cdev, &pmtbl);
	}
	return 0;
}

int GSW_PMAC_EG_CfgGet(void *cdev, GSW_PMAC_Eg_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	pmtbl_prog_t pmtbl;
	u32 regval;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		memset(&pmtbl, 0, sizeof(pmtbl));
		pmtbl.ptaddr = parm->nDestPortId & 0x0F;
		pmtbl.ptaddr |= (parm->nFlowIDMsb & 0x03) << 8;
		gsw_r32(cdev, PMAC_CTRL_4_FLAGEN_OFFSET, PMAC_CTRL_4_FLAGEN_SHIFT,
			PMAC_CTRL_4_FLAGEN_SIZE, &regval);
/*		if (parm->bTCEnable == 1) { */
		if ((parm->bProcFlagsSelect == 0) && (regval == 0)) {
			pmtbl.ptaddr |= (parm->nTrafficClass & 0x0F) << 4;
		} else if ((parm->bProcFlagsSelect == 1) && (regval == 1)) {
			/* MPE-1 Marked Flag  */
			if (parm->bMpe1Flag)
				pmtbl.ptaddr |= (1 << 4);
			/* MPE-2 Marked Flag  */
			if (parm->bMpe2Flag)
				pmtbl.ptaddr |= (1 << 5);
			/* Cryptography Encryption Action Flag  */
			if (parm->bEncFlag)
				pmtbl.ptaddr |= (1 << 6);
			/* Cryptography Decryption Action Flag  */
			if (parm->bDecFlag)
				pmtbl.ptaddr |= (1 << 7);
		} else {
			return GSW_statusNoSupport;
		}
		pmtbl.ptcaddr	= PMAC_EGCFG_INDEX;
		xwayflow_pmac_table_read(cdev, &pmtbl);
		/*  nRes2DW0 -- DW0 (bit 14 to 13),*/
		/*nRes1DW0 -- DW0 (bit 31 to 29), nResDW1 -- DW1 (bit 7 to 4)*/
		parm->nRes2DW0 = pmtbl.val[0] & 0x3;
		parm->nRes1DW0 = (pmtbl.val[0] >> 2) & 0x3;
		parm->nResDW1 = (pmtbl.val[0] >> 5) & 0x0F;
		/* Receive DMA Channel Identifier (0..7) */
		parm->nRxDmaChanId = (pmtbl.val[1] & 0x0F);

		/* Packet has PMAC */
		if ((pmtbl.val[2] >> 0) & 0x1)
			parm->bPmacEna = 1;

		/* Packet has FCS */
		if ((pmtbl.val[2] >> 1) & 0x1)
			parm->bFcsEna = 1;

		/* To remove L2 header & additional bytes  */
		if ((pmtbl.val[2] >> 2) & 0x1) {
			parm->bRemL2Hdr = 1;
		/* No. of bytes to be removed after Layer-2 Header */
		/* valid when bRemL2Hdr is set  */
			parm->numBytesRem = ((pmtbl.val[2] >> 8) & 0xFF);
		}
	}
	return 0;
}

#if defined(CONFIG_LTQ_8021X) && CONFIG_LTQ_8021X
GSW_return_t GSW_8021X_EAPOL_RuleGet(void *cdev, GSW_8021X_EAPOL_Rule_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	stp8021x_t *scfg = &gswdev->stpconfig;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	parm->eForwardPort = scfg->sfport;
	parm->nForwardPortId = scfg->fpid8021x;
	return GSW_statusOk;
}

GSW_return_t GSW_8021X_EAPOL_RuleSet(void *cdev, GSW_8021X_EAPOL_Rule_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	stp8021x_t *scfg = &gswdev->stpconfig;
	GSW_PCE_rule_t pcrule;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	scfg->sfport = parm->eForwardPort;
	scfg->fpid8021x = parm->nForwardPortId;
	memset(&pcrule, 0, sizeof(GSW_PCE_rule_t));
	pcrule.pattern.nIndex = EAPOL_PCE_RULE_INDEX;
	pcrule.pattern.bEnable = 1;
	pcrule.pattern.bMAC_DstEnable	= 1;
	pcrule.pattern.nMAC_Dst[0] = 0x01;
	pcrule.pattern.nMAC_Dst[1] = 0x80;
	pcrule.pattern.nMAC_Dst[2] = 0xC2;
	pcrule.pattern.nMAC_Dst[3] = 0x00;
	pcrule.pattern.nMAC_Dst[4] = 0x00;
	pcrule.pattern.nMAC_Dst[5] = 0x03;
	pcrule.pattern.nMAC_Src[5] = 0;
	pcrule.pattern.bEtherTypeEnable	= 1;
	pcrule.pattern.nEtherType	= 0x888E;
	pcrule.action.eCrossStateAction = GSW_PCE_ACTION_CROSS_STATE_CROSS;
	if ((scfg->sfport < 4) && (scfg->sfport > 0))
		pcrule.action.ePortMapAction = scfg->sfport + 1;
	else
		pr_warn("(Incorrect forward port action) %s:%s:%d\n",
			__FILE__, __func__, __LINE__);
	pcrule.action.nForwardPortMap = (1 << parm->nForwardPortId);
	/* We prepare everything and write into PCE Table */
	if (0 != pce_rule_write(gswdev, &gswdev->phandler, &pcrule))
		return GSW_statusErr;
	return GSW_statusOk;
}

GSW_return_t GSW_8021X_PortCfgGet(void *cdev, GSW_8021X_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev	= (ethsw_api_dev_t *)cdev;
	port_config_t *pcfg;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;

	pcfg = &gswdev->pconfig[parm->nPortId];
	parm->eState = pcfg->p8021xs;
	return GSW_statusOk;
}

GSW_return_t GSW_8021X_PortCfgSet(void *cdev, GSW_8021X_portCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	port_config_t *pcfg;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;

	pcfg = &gswdev->pconfig[parm->nPortId];
	pcfg->p8021xs	= parm->eState;
	set_port_state(cdev, parm->nPortId, pcfg->pcstate,
		pcfg->p8021xs);
	return GSW_statusOk;
}

#endif /*CONFIG_LTQ_8021X  */
GSW_return_t GSW_CPU_PortCfgGet(void *cdev, GSW_CPU_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->pnum)
		return GSW_statusErr;
	if (pidx ==  gswdev->cport)
		parm->bCPU_PortValid = 1;
	else
		parm->bCPU_PortValid = 0;
	/* Special Tag Egress*/
	gsw_r32(cdev, (FDMA_PCTRL_STEN_OFFSET + (0x6 * pidx)),
		FDMA_PCTRL_STEN_SHIFT, FDMA_PCTRL_STEN_SIZE, &value);
	parm->bSpecialTagEgress = value;
	/* Special Tag Igress*/
	gsw_r32(cdev, (PCE_PCTRL_0_IGSTEN_OFFSET + (0xa * pidx)),
		PCE_PCTRL_0_IGSTEN_SHIFT, PCE_PCTRL_0_IGSTEN_SIZE, &value);
	parm->bSpecialTagIngress = value;
	/* FCS Check */
	gsw_r32(cdev, (SDMA_PCTRL_FCSIGN_OFFSET + (0x6 * pidx)),
		SDMA_PCTRL_FCSIGN_SHIFT,
		SDMA_PCTRL_FCSIGN_SIZE, &value);
	parm->bFcsCheck = value;
	/* FCS Generate */
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		if (pidx != 0) {
			gsw_r32(cdev, (MAC_CTRL_0_FCS_OFFSET + (0xC * (pidx - 1))),
				MAC_CTRL_0_FCS_SHIFT, MAC_CTRL_0_FCS_SIZE, &value);
			parm->bFcsGenerate = value;
		}
		gsw_r32(cdev, FDMA_PASR_CPU_OFFSET,	FDMA_PASR_CPU_SHIFT,
			FDMA_PASR_CPU_SIZE, &value);
		parm->eNoMPEParserCfg = value;
		gsw_r32(cdev, FDMA_PASR_MPE1_OFFSET,	FDMA_PASR_MPE1_SHIFT,
			FDMA_PASR_MPE1_SIZE, &value);
		parm->eMPE1ParserCfg = value;
		gsw_r32(cdev, FDMA_PASR_MPE2_OFFSET,	FDMA_PASR_MPE2_SHIFT,
			FDMA_PASR_MPE2_SIZE, &value);
		parm->eMPE2ParserCfg = value;
		gsw_r32(cdev, FDMA_PASR_MPE3_OFFSET,	FDMA_PASR_MPE3_SHIFT,
			FDMA_PASR_MPE3_SIZE, &value);
		parm->eMPE1MPE2ParserCfg = value;
	} else {
		gsw_r32(cdev, (MAC_CTRL_0_FCS_OFFSET + (0xC * pidx)),
			MAC_CTRL_0_FCS_SHIFT, MAC_CTRL_0_FCS_SIZE, &value);
		parm->bFcsGenerate = value;
	}
	gsw_r32(cdev, (FDMA_PCTRL_ST_TYPE_OFFSET + (0x6 * pidx)),
		FDMA_PCTRL_ST_TYPE_SHIFT, FDMA_PCTRL_ST_TYPE_SIZE, &value);
	 parm->bSpecialTagEthType = value;
	return GSW_statusOk;
}

GSW_return_t GSW_CPU_PortCfgSet(void *cdev, GSW_CPU_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 RST, AS, AST, RXSH;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->pnum)
		return GSW_statusErr;
	if (pidx == gswdev->cport)
		parm->bCPU_PortValid = 1;
	else
		parm->bCPU_PortValid = 0;
	gswdev->mpnum = pidx;
	/* Special Tag Egress*/
	gsw_w32(cdev, (FDMA_PCTRL_STEN_OFFSET + (0x6 * pidx)),
		FDMA_PCTRL_STEN_SHIFT,
		FDMA_PCTRL_STEN_SIZE, parm->bSpecialTagEgress);
	/* xRX CPU port */
	if ((pidx == gswdev->cport) && !(gswdev->gipver == LTQ_GSWIP_3_0)) {
		if (parm->bSpecialTagEgress == 0) {
			RST = 1; AS = 0;
		} else {
			RST = 0; AS = 1;
		}
		gsw_w32(cdev, (PMAC_HD_CTL_RST_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_RST_SHIFT, PMAC_HD_CTL_RST_SIZE, RST);
		gsw_w32(cdev, (PMAC_HD_CTL_AS_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_AS_SHIFT, PMAC_HD_CTL_AS_SIZE, AS);
	}
	/* Special Tag Igress*/
	gsw_w32(cdev, (PCE_PCTRL_0_IGSTEN_OFFSET + (0xa * pidx)),
		PCE_PCTRL_0_IGSTEN_SHIFT,
		PCE_PCTRL_0_IGSTEN_SIZE, parm->bSpecialTagIngress);
	if ((pidx == gswdev->cport) && !(gswdev->gipver == LTQ_GSWIP_3_0)) {
		if (parm->bSpecialTagIngress == 0) {
			AST = 0; RXSH = 0;
		} else {
			AST = 1; RXSH = 1;
		}
		gsw_w32(cdev, (PMAC_HD_CTL_AST_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_AST_SHIFT, PMAC_HD_CTL_AST_SIZE, AST);
		gsw_w32(cdev, (PMAC_HD_CTL_RXSH_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_RXSH_SHIFT, PMAC_HD_CTL_RXSH_SIZE, RXSH);
		/* FCS Generate */
		gsw_w32(cdev, (MAC_CTRL_0_FCS_OFFSET + (0xC * pidx)),
			MAC_CTRL_0_FCS_SHIFT, MAC_CTRL_0_FCS_SIZE, parm->bFcsGenerate);
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		/* FCS Generate */
		if (pidx != 0) {
			gsw_w32(cdev, (MAC_CTRL_0_FCS_OFFSET + (0xC * (pidx - 1))),
			MAC_CTRL_0_FCS_SHIFT, MAC_CTRL_0_FCS_SIZE, parm->bFcsGenerate);
		}
		switch(parm->eNoMPEParserCfg) {
		case GSW_CPU_PARSER_NIL:
			gsw_w32(cdev, FDMA_PASR_CPU_OFFSET,	FDMA_PASR_CPU_SHIFT,
				FDMA_PASR_CPU_SIZE, 0);
			break;
		case GSW_CPU_PARSER_FLAGS:
			gsw_w32(cdev, FDMA_PASR_CPU_OFFSET,	FDMA_PASR_CPU_SHIFT,
				FDMA_PASR_CPU_SIZE, 1);
			break;
		case GSW_CPU_PARSER_OFFSETS_FLAGS:
			gsw_w32(cdev, FDMA_PASR_CPU_OFFSET,	FDMA_PASR_CPU_SHIFT,
				FDMA_PASR_CPU_SIZE, 2);
			break;
		case GSW_CPU_PARSER_RESERVED:
			gsw_w32(cdev, FDMA_PASR_CPU_OFFSET,	FDMA_PASR_CPU_SHIFT,
				FDMA_PASR_CPU_SIZE, 3);
			break;
		}
		switch(parm->eMPE1ParserCfg) {
		case GSW_CPU_PARSER_NIL:
			gsw_w32(cdev, FDMA_PASR_MPE1_OFFSET,	FDMA_PASR_MPE1_SHIFT,
				FDMA_PASR_MPE1_SIZE, 0);
			break;
		case GSW_CPU_PARSER_FLAGS:
			gsw_w32(cdev, FDMA_PASR_MPE1_OFFSET,	FDMA_PASR_MPE1_SHIFT,
				FDMA_PASR_MPE1_SIZE, 1);
			break;
		case GSW_CPU_PARSER_OFFSETS_FLAGS:
			gsw_w32(cdev, FDMA_PASR_MPE1_OFFSET,	FDMA_PASR_MPE1_SHIFT,
				FDMA_PASR_MPE1_SIZE, 2);
			break;
		case GSW_CPU_PARSER_RESERVED:
			gsw_w32(cdev, FDMA_PASR_MPE1_OFFSET,	FDMA_PASR_MPE1_SHIFT,
				FDMA_PASR_MPE1_SIZE, 3);
			break;
		}
		switch(parm->eMPE2ParserCfg) {
		case GSW_CPU_PARSER_NIL:
			gsw_w32(cdev, FDMA_PASR_MPE2_OFFSET,	FDMA_PASR_MPE2_SHIFT,
				FDMA_PASR_MPE2_SIZE, 0);
			break;
		case GSW_CPU_PARSER_FLAGS:
			gsw_w32(cdev, FDMA_PASR_MPE2_OFFSET,	FDMA_PASR_MPE2_SHIFT,
				FDMA_PASR_MPE2_SIZE, 1);
			break;
		case GSW_CPU_PARSER_OFFSETS_FLAGS:
			gsw_w32(cdev, FDMA_PASR_MPE2_OFFSET,	FDMA_PASR_MPE2_SHIFT,
				FDMA_PASR_MPE2_SIZE, 2);
			break;
		case GSW_CPU_PARSER_RESERVED:
			gsw_w32(cdev, FDMA_PASR_MPE2_OFFSET,	FDMA_PASR_MPE2_SHIFT,
				FDMA_PASR_MPE2_SIZE, 3);
			break;
		}
		switch(parm->eMPE1MPE2ParserCfg) {
		case GSW_CPU_PARSER_NIL:
			gsw_w32(cdev, FDMA_PASR_MPE3_OFFSET,	FDMA_PASR_MPE3_SHIFT,
				FDMA_PASR_MPE3_SIZE, 0);
			break;
		case GSW_CPU_PARSER_FLAGS:
			gsw_w32(cdev, FDMA_PASR_MPE3_OFFSET,	FDMA_PASR_MPE3_SHIFT,
				FDMA_PASR_MPE3_SIZE, 1);
			break;
		case GSW_CPU_PARSER_OFFSETS_FLAGS:
			gsw_w32(cdev, FDMA_PASR_MPE3_OFFSET,	FDMA_PASR_MPE3_SHIFT,
				FDMA_PASR_MPE3_SIZE, 2);
			break;
		case GSW_CPU_PARSER_RESERVED:
			gsw_w32(cdev, FDMA_PASR_MPE3_OFFSET,	FDMA_PASR_MPE3_SHIFT,
				FDMA_PASR_MPE3_SIZE, 3);
			break;
		}
	}
	/* FCS Check */
	gsw_w32(cdev, (SDMA_PCTRL_FCSIGN_OFFSET + (0x6 * pidx)),
		SDMA_PCTRL_FCSIGN_SHIFT,
		SDMA_PCTRL_FCSIGN_SIZE, parm->bFcsCheck);

	if (parm->bSpecialTagEthType == GSW_CPU_ETHTYPE_FLOWID) {
		gsw_w32(cdev, (FDMA_PCTRL_ST_TYPE_OFFSET + (0x6 * pidx)),
			FDMA_PCTRL_ST_TYPE_SHIFT, FDMA_PCTRL_ST_TYPE_SIZE, 1);
	} else {
		gsw_w32(cdev, (FDMA_PCTRL_ST_TYPE_OFFSET + (0x6 * pidx)),
			FDMA_PCTRL_ST_TYPE_SHIFT, FDMA_PCTRL_ST_TYPE_SIZE, 0);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_CPU_PortExtendCfgGet(void *cdev, GSW_CPU_PortExtendCfg_t *parm)
{
	u32  value, value_add, value_vlan;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		return GSW_statusOk;
	} else {
		gsw_r32(cdev,
			(PMAC_HD_CTL_ADD_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_ADD_SHIFT,
			PMAC_HD_CTL_ADD_SIZE, &value_add);
		gsw_r32(cdev,
			(PMAC_HD_CTL_TAG_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_TAG_SHIFT,
			PMAC_HD_CTL_TAG_SIZE, &value_vlan);
	}
	if (value_add == 0 && value_vlan == 0)
		parm->eHeaderAdd = 0;
	else if (value_add == 1 && value_vlan == 0)
		parm->eHeaderAdd = 1;
	else if (value_add == 1 && value_vlan == 1)
		parm->eHeaderAdd = 2;
	else
		parm->eHeaderAdd = 0;
	gsw_r32(cdev, (PMAC_HD_CTL_RL2_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_RL2_SHIFT, PMAC_HD_CTL_RL2_SIZE, &value);
	parm->bHeaderRemove = value;
	memset(&parm->sHeader, 0, sizeof(GSW_CPU_Header_t));
	if (value_add == 1) {
		/* Output the Src MAC */
		gsw_r32(cdev,
			(PMAC_SA3_SA_15_0_OFFSET + GSW_TREG_OFFSET),
			PMAC_SA3_SA_15_0_SHIFT,
			PMAC_SA3_SA_15_0_SIZE, &value);
		parm->sHeader.nMAC_Src[0] = value & 0xFF;
		parm->sHeader.nMAC_Src[1] = ((value >> 8) & 0xFF);
		gsw_r32(cdev,
			(PMAC_SA2_SA_31_16_OFFSET + GSW_TREG_OFFSET),
			PMAC_SA2_SA_31_16_SHIFT,
			PMAC_SA2_SA_31_16_SIZE, &value);
		parm->sHeader.nMAC_Src[2] = value & 0xFF;
		parm->sHeader.nMAC_Src[3] = ((value >> 8) & 0xFF);
		gsw_r32(cdev,
			(PMAC_SA1_SA_47_32_OFFSET + GSW_TREG_OFFSET),
			PMAC_SA1_SA_47_32_SHIFT,
			PMAC_SA1_SA_47_32_SIZE, &value);
		parm->sHeader.nMAC_Src[4] = value & 0xFF;
		parm->sHeader.nMAC_Src[5] = ((value >> 8) & 0xFF);
		/* Output the Dst MAC */
		gsw_r32(cdev,
			(PMAC_DA3_DA_15_0_OFFSET + GSW_TREG_OFFSET),
			PMAC_DA3_DA_15_0_SHIFT, PMAC_DA3_DA_15_0_SIZE, &value);
		parm->sHeader.nMAC_Dst[0] = value & 0xFF;
		parm->sHeader.nMAC_Dst[1] = ((value >> 8) & 0xFF);
		gsw_r32(cdev,
			(PMAC_DA2_DA_31_16_OFFSET + GSW_TREG_OFFSET),
			PMAC_DA2_DA_31_16_SHIFT,
			PMAC_DA2_DA_31_16_SIZE, &value);
		parm->sHeader.nMAC_Dst[2] = value & 0xFF;
		parm->sHeader.nMAC_Dst[3] = ((value >> 8) & 0xFF);
		gsw_r32(cdev,
			(PMAC_DA1_SA_47_32_OFFSET + GSW_TREG_OFFSET),
			PMAC_DA1_SA_47_32_SHIFT,
			PMAC_DA1_SA_47_32_SIZE, &value);
		parm->sHeader.nMAC_Dst[4] = value & 0xFF;
		parm->sHeader.nMAC_Dst[5] = ((value >> 8) & 0xFF);
		/* Input the Ethernet Type */
		gsw_r32(cdev,
			(PMAC_TL_TYPE_LEN_OFFSET + GSW_TREG_OFFSET),
			PMAC_TL_TYPE_LEN_SHIFT, PMAC_TL_TYPE_LEN_SIZE, &value);
		parm->sHeader.nEthertype = value;
	}
	if (value_vlan == 1) {
		gsw_r32(cdev, (PMAC_VLAN_PRI_OFFSET + GSW_TREG_OFFSET),
			PMAC_VLAN_PRI_SHIFT, PMAC_VLAN_PRI_SIZE, &value);
		parm->sHeader.nVLAN_Prio = value;
		gsw_r32(cdev, (PMAC_VLAN_CFI_OFFSET + GSW_TREG_OFFSET),
			PMAC_VLAN_CFI_SHIFT, PMAC_VLAN_CFI_SIZE, &value);
		parm->sHeader.nVLAN_CFI = value;
		gsw_r32(cdev,
			(PMAC_VLAN_VLAN_ID_OFFSET + GSW_TREG_OFFSET),
			PMAC_VLAN_VLAN_ID_SHIFT,
			PMAC_VLAN_VLAN_ID_SIZE, &value);
		parm->sHeader.nVLAN_ID = value;
	}
	gsw_r32(cdev, (PMAC_HD_CTL_FC_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_FC_SHIFT, PMAC_HD_CTL_FC_SIZE, &value);
	parm->ePauseCtrl = value;
	gsw_r32(cdev, (PMAC_HD_CTL_RC_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_RC_SHIFT, PMAC_HD_CTL_RC_SIZE, &value);
	parm->bFcsRemove = value;
	gsw_r32(cdev, (PMAC_EWAN_EWAN_OFFSET + GSW_TREG_OFFSET),
		PMAC_EWAN_EWAN_SHIFT, PMAC_EWAN_EWAN_SIZE, &value);
	parm->nWAN_Ports = value;
	return GSW_statusOk;
}

GSW_return_t GSW_CPU_PortExtendCfgSet(void *cdev, GSW_CPU_PortExtendCfg_t *parm)
{
	u32 value_add = 0, value_vlan = 0;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0)
		return GSW_statusOk;
	switch (parm->eHeaderAdd) {
	case GSW_CPU_HEADER_NO:
		value_add = 0; value_vlan = 0;
		break;
	case GSW_CPU_HEADER_MAC:
		value_add = 1; value_vlan = 0;
		break;
	case GSW_CPU_HEADER_VLAN:
		value_add = 1; value_vlan = 1;
		break;
	}
	if ((parm->bHeaderRemove == 1)
		&& (parm->eHeaderAdd != GSW_CPU_HEADER_NO)) {
		pr_err("The Header Can't be remove because the Header Add parameter is not 0");
		return GSW_statusErr;
	}  else {
		gsw_w32(cdev, (PMAC_HD_CTL_RL2_OFFSET + GSW_TREG_OFFSET),
			PMAC_HD_CTL_RL2_SHIFT,
			PMAC_HD_CTL_RL2_SIZE, parm->bHeaderRemove);
	}
	gsw_w32(cdev, (PMAC_HD_CTL_ADD_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_ADD_SHIFT, PMAC_HD_CTL_ADD_SIZE, value_add);
	gsw_w32(cdev, (PMAC_HD_CTL_TAG_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_TAG_SHIFT, PMAC_HD_CTL_TAG_SIZE, value_vlan);
	if (parm->eHeaderAdd == GSW_CPU_HEADER_MAC) {
		u32 macdata;
		/* Input the Src MAC */
		macdata = ((parm->sHeader.nMAC_Src[0])
		| (parm->sHeader.nMAC_Src[1] << 8));
		gsw_w32(cdev,
			(PMAC_SA3_SA_15_0_OFFSET + GSW_TREG_OFFSET),
			PMAC_SA3_SA_15_0_SHIFT,
			PMAC_SA3_SA_15_0_SIZE, macdata);
		macdata = (parm->sHeader.nMAC_Src[2]
			| parm->sHeader.nMAC_Src[3] << 8);
		gsw_w32(cdev,
			(PMAC_SA2_SA_31_16_OFFSET + GSW_TREG_OFFSET),
			PMAC_SA2_SA_31_16_SHIFT,
			PMAC_SA2_SA_31_16_SIZE, macdata);
		macdata = (parm->sHeader.nMAC_Src[4]
				| parm->sHeader.nMAC_Src[5] << 8);
		gsw_w32(cdev,
			(PMAC_SA1_SA_47_32_OFFSET + GSW_TREG_OFFSET),
			PMAC_SA1_SA_47_32_SHIFT,
			PMAC_SA1_SA_47_32_SIZE, macdata);
		/* Input the Dst MAC */
		macdata = (parm->sHeader.nMAC_Dst[0]
				| parm->sHeader.nMAC_Dst[1] << 8);
		gsw_w32(cdev,
			(PMAC_DA3_DA_15_0_OFFSET + GSW_TREG_OFFSET),
			PMAC_DA3_DA_15_0_SHIFT,
			PMAC_DA3_DA_15_0_SIZE, macdata);
		macdata = (parm->sHeader.nMAC_Dst[2]
				| parm->sHeader.nMAC_Dst[3] << 8);
		gsw_w32(cdev,
			(PMAC_DA2_DA_31_16_OFFSET + GSW_TREG_OFFSET),
			PMAC_DA2_DA_31_16_SHIFT,
			PMAC_DA2_DA_31_16_SIZE, macdata);
		macdata = ((parm->sHeader.nMAC_Dst[4])
			| (parm->sHeader.nMAC_Dst[5] << 8));
		gsw_w32(cdev,
			(PMAC_DA1_SA_47_32_OFFSET + GSW_TREG_OFFSET),
			PMAC_DA1_SA_47_32_SHIFT,
			PMAC_DA1_SA_47_32_SIZE, macdata);
		/* Input the Ethernet Type */
		gsw_w32(cdev,
			(PMAC_TL_TYPE_LEN_OFFSET + GSW_TREG_OFFSET),
			PMAC_TL_TYPE_LEN_SHIFT,
			PMAC_TL_TYPE_LEN_SIZE, parm->sHeader.nEthertype);
	}
	if (parm->eHeaderAdd == GSW_CPU_HEADER_VLAN) {
		gsw_w32(cdev, (PMAC_VLAN_PRI_OFFSET + GSW_TREG_OFFSET),
			PMAC_VLAN_PRI_SHIFT,
			PMAC_VLAN_PRI_SIZE, parm->sHeader.nVLAN_Prio);
	gsw_w32(cdev, (PMAC_VLAN_CFI_OFFSET + GSW_TREG_OFFSET),
		PMAC_VLAN_CFI_SHIFT,
		PMAC_VLAN_CFI_SIZE, parm->sHeader.nVLAN_CFI);
	gsw_w32(cdev, (PMAC_VLAN_VLAN_ID_OFFSET + GSW_TREG_OFFSET),
		PMAC_VLAN_VLAN_ID_SHIFT,
		PMAC_VLAN_VLAN_ID_SIZE, parm->sHeader.nVLAN_ID);
	}
	gsw_w32(cdev, (PMAC_HD_CTL_FC_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_FC_SHIFT, PMAC_HD_CTL_FC_SIZE, parm->ePauseCtrl);
	gsw_w32(cdev, (PMAC_HD_CTL_RC_OFFSET + GSW_TREG_OFFSET),
		PMAC_HD_CTL_RC_SHIFT, PMAC_HD_CTL_RC_SIZE, parm->bFcsRemove);
	gsw_w32(cdev, (PMAC_EWAN_EWAN_OFFSET + GSW_TREG_OFFSET),
		PMAC_EWAN_EWAN_SHIFT, PMAC_EWAN_EWAN_SIZE, parm->nWAN_Ports);
	return GSW_statusOk;
}

#if defined(CONFIG_LTQ_WOL) && CONFIG_LTQ_WOL
GSW_return_t GSW_WoL_CfgGet(void *cdev, GSW_WoL_Cfg_t *parm)
{
	u32 value;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	gsw_r32(cdev, WOL_GLB_CTRL_PASSEN_OFFSET, WOL_GLB_CTRL_PASSEN_SHIFT,
		WOL_GLB_CTRL_PASSEN_SIZE, &value);
	parm->bWolPasswordEnable = value;
	gsw_r32(cdev, WOL_DA_2_DA2_OFFSET, WOL_DA_2_DA2_SHIFT,
		WOL_DA_2_DA2_SIZE, &value);
	parm->nWolMAC[0] = (value >> 8 & 0xFF);
	parm->nWolMAC[1] = (value & 0xFF);
	gsw_r32(cdev, WOL_DA_1_DA1_OFFSET, WOL_DA_1_DA1_SHIFT,
		WOL_DA_1_DA1_SIZE, &value);
	parm->nWolMAC[2] = (value >> 8 & 0xFF);
	parm->nWolMAC[3] = (value & 0xFF);
	gsw_r32(cdev, WOL_DA_0_DA0_OFFSET, WOL_DA_0_DA0_SHIFT,
		WOL_DA_0_DA0_SIZE, &value);
	parm->nWolMAC[4] = (value >> 8 & 0xFF);
	parm->nWolMAC[5] = (value & 0xFF);
	gsw_r32(cdev, WOL_PW_2_PW2_OFFSET, WOL_PW_2_PW2_SHIFT,
		WOL_PW_2_PW2_SIZE, &value);
	parm->nWolPassword[0] = (value >> 8 & 0xFF);
	parm->nWolPassword[1] = (value & 0xFF);
	gsw_r32(cdev, WOL_PW_1_PW1_OFFSET, WOL_PW_1_PW1_SHIFT,
		WOL_PW_1_PW1_SIZE, &value);
	parm->nWolPassword[2] = (value >> 8 & 0xFF);
	parm->nWolPassword[3] = (value & 0xFF);
	gsw_r32(cdev, WOL_PW_0_PW0_OFFSET, WOL_PW_0_PW0_SHIFT,
		WOL_PW_0_PW0_SIZE, &value);
	parm->nWolPassword[4] = (value >> 8 & 0xFF);
	parm->nWolPassword[5] = (value & 0xFF);
	return GSW_statusOk;
}

GSW_return_t GSW_WoL_CfgSet(void *cdev, GSW_WoL_Cfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	gsw_w32(cdev, WOL_GLB_CTRL_PASSEN_OFFSET, WOL_GLB_CTRL_PASSEN_SHIFT,
		WOL_GLB_CTRL_PASSEN_SIZE, parm->bWolPasswordEnable);
	gsw_w32(cdev, WOL_DA_2_DA2_OFFSET,
		WOL_DA_2_DA2_SHIFT, WOL_DA_2_DA2_SIZE,
		(((parm->nWolMAC[0] & 0xFF) << 8)
		| (parm->nWolMAC[1] & 0xFF)));
	gsw_w32(cdev, WOL_DA_1_DA1_OFFSET,
		WOL_DA_1_DA1_SHIFT, WOL_DA_1_DA1_SIZE,
		(((parm->nWolMAC[2] & 0xFF) << 8)
		| (parm->nWolMAC[3] & 0xFF)));
	gsw_w32(cdev, WOL_DA_0_DA0_OFFSET,
		WOL_DA_0_DA0_SHIFT, WOL_DA_0_DA0_SIZE,
		(((parm->nWolMAC[4] & 0xFF) << 8)
		| (parm->nWolMAC[5] & 0xFF)));
	gsw_w32(cdev, WOL_PW_2_PW2_OFFSET,
		WOL_PW_2_PW2_SHIFT, WOL_PW_2_PW2_SIZE,
		(((parm->nWolPassword[0] & 0xFF) << 8)
		| (parm->nWolPassword[1] & 0xFF)));
	gsw_w32(cdev, WOL_PW_1_PW1_OFFSET,
		WOL_PW_1_PW1_SHIFT, WOL_PW_1_PW1_SIZE,
		(((parm->nWolPassword[2] & 0xFF) << 8)
		| (parm->nWolPassword[3] & 0xFF)));
	gsw_w32(cdev, WOL_PW_0_PW0_OFFSET,
		WOL_PW_0_PW0_SHIFT, WOL_PW_0_PW0_SIZE,
		(((parm->nWolPassword[4] & 0xFF) << 8)
		| (parm->nWolPassword[5] & 0xFF)));
	return GSW_statusOk;
}

GSW_return_t GSW_WoL_PortCfgGet(void *cdev, GSW_WoL_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	gsw_r32(cdev, (WOL_CTRL_PORT_OFFSET + (0xA * parm->nPortId)),
		WOL_CTRL_PORT_SHIFT, WOL_CTRL_PORT_SIZE, &value);
	parm->bWakeOnLAN_Enable = value;
	return GSW_statusOk;
}

GSW_return_t GSW_WoL_PortCfgSet(void *cdev, GSW_WoL_PortCfg_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	gsw_w32(cdev, (WOL_CTRL_PORT_OFFSET + (0xA * parm->nPortId)),
		WOL_CTRL_PORT_SHIFT,
		WOL_CTRL_PORT_SIZE, parm->bWakeOnLAN_Enable);
	return GSW_statusOk;
}

#endif /* CONFIG_LTQ_WOL */
GSW_return_t GSW_RegisterGet(void *cdev, GSW_register_t *parm)
{
	u32 rvalue, raddr = parm->nRegAddr;
	gsw_r32(cdev, raddr,	0, 16, &rvalue);
	parm->nData = rvalue;
	return GSW_statusOk;
}

GSW_return_t GSW_RegisterSet(void *cdev, GSW_register_t *parm)
{
	u32 rvalue = parm->nData, raddr = parm->nRegAddr;
	gsw_w32(cdev, raddr, 0, 16, rvalue);
	return GSW_statusOk;
}

GSW_return_t GSW_IrqGet(void *cdev, GSW_irq_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	/* ToDo: Require future clarify for how to display */
	return GSW_statusOk;
}

GSW_return_t GSW_IrqMaskGet(void *cdev, GSW_irq_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	if (parm->eIrqSrc == GSW_IRQ_WOL) {
		gsw_r32(cdev, (PCE_PIER_WOL_OFFSET + (0xA * pidx)),
			PCE_PIER_WOL_SHIFT, PCE_PIER_WOL_SIZE, &value);
	} else if (parm->eIrqSrc == GSW_IRQ_LIMIT_ALERT) {
		gsw_r32(cdev, (PCE_PIER_LOCK_OFFSET + (0xA * pidx)),
			PCE_PIER_LOCK_SHIFT, PCE_PIER_LOCK_SIZE, &value);
	} else if (parm->eIrqSrc == GSW_IRQ_LOCK_ALERT) {
		gsw_r32(cdev, (PCE_PIER_LIM_OFFSET + (0xA * pidx)),
			PCE_PIER_LIM_SHIFT, PCE_PIER_LIM_SIZE, &value);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_IrqMaskSet(void *cdev, GSW_irq_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	if (parm->eIrqSrc == GSW_IRQ_WOL) {
		gsw_w32(cdev, (PCE_PIER_WOL_OFFSET + (0xA * pidx)),
			PCE_PIER_WOL_SHIFT, PCE_PIER_WOL_SIZE, 1);
	} else if (parm->eIrqSrc == GSW_IRQ_LIMIT_ALERT) {
		gsw_w32(cdev, (PCE_PIER_LOCK_OFFSET + (0xA * pidx)),
			PCE_PIER_LOCK_SHIFT, PCE_PIER_LOCK_SIZE, 1);
	} else if (parm->eIrqSrc == GSW_IRQ_LOCK_ALERT) {
		gsw_w32(cdev, (PCE_PIER_LIM_OFFSET + (0xA * pidx)),
			PCE_PIER_LIM_SHIFT, PCE_PIER_LIM_SIZE, 1);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_IrqStatusClear(void *cdev, GSW_irq_t *parm)
{
	/* ToDo: Request future clarify */
	return GSW_statusOk;
}

GSW_return_t GSW_PceRuleRead(void *cdev, GSW_PCE_rule_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (0 != pce_rule_read(gswdev, &gswdev->phandler, parm))
		return GSW_statusErr;
	return GSW_statusOk;
}

GSW_return_t GSW_PceRuleWrite(void *cdev, GSW_PCE_rule_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (0 != pce_rule_write(gswdev, &gswdev->phandler, parm))
		return GSW_statusErr;
	return GSW_statusOk;
}

GSW_return_t GSW_PceRuleDelete(void *cdev, GSW_PCE_ruleDelete_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	value = parm->nIndex;
	if (0 != pce_pattern_delete(cdev, &gswdev->phandler, value))
		return GSW_statusErr;
	if (0 != pce_action_delete(cdev, &gswdev->phandler, value))
		return GSW_statusErr;
	return GSW_statusOk;
}

GSW_return_t GSW_RMON_ExtendGet(void *cdev, GSW_RMON_extendGet_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 pidx = parm->nPortId, i;
	u32 value, data0, data1;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nPortId >= gswdev->tpnum)
		return GSW_statusErr;
	memset(parm, 0, sizeof(GSW_RMON_extendGet_t));
	for (i = 0; i < GSW_RMON_EXTEND_NUM; i++) {
		gsw_w32(cdev, BM_RAM_ADDR_ADDR_OFFSET,
			BM_RAM_ADDR_ADDR_SHIFT,
			BM_RAM_ADDR_ADDR_SIZE,
			(i+REX_TFLOW_CNT_1));
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			if (pidx >= 8)
				value = (pidx + 8);
			else
				value = pidx;
			gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
				BM_RAM_CTRL_ADDR_SHIFT,
				BM_RAM_CTRL_ADDR_SIZE, value);
		} else {
			gsw_w32(cdev, BM_RAM_CTRL_ADDR_OFFSET,
				BM_RAM_CTRL_ADDR_SHIFT,
				BM_RAM_CTRL_ADDR_SIZE, pidx);
		}
		gsw_w32(cdev, BM_RAM_CTRL_OPMOD_OFFSET,
			BM_RAM_CTRL_OPMOD_SHIFT,
			BM_RAM_CTRL_OPMOD_SIZE, 0);
		value = 1;
		gsw_w32(cdev, BM_RAM_CTRL_BAS_OFFSET,
			BM_RAM_CTRL_BAS_SHIFT,
			BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(cdev, BM_RAM_CTRL_BAS_OFFSET,
				BM_RAM_CTRL_BAS_SHIFT,
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		gsw_r32(cdev, BM_RAM_VAL_0_VAL0_OFFSET,
			BM_RAM_VAL_0_VAL0_SHIFT,
			BM_RAM_VAL_0_VAL0_SIZE, &data0);
		gsw_r32(cdev, BM_RAM_VAL_1_VAL1_OFFSET,
			BM_RAM_VAL_1_VAL1_SHIFT,
			BM_RAM_VAL_1_VAL1_SIZE, &data1);
		parm->nTrafficFlowCnt[i] = (data1 << 16 | data0);
	}
	return GSW_statusOk;
}

GSW_return_t GSW_Reset(void *cdev, GSW_reset_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u32 value;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	gswdev->rst = 1;
	gswdev->hwinit = 0;
/* Reset the Switch via Switch IP register*/
	value = 1;
	gsw_w32(cdev, ETHSW_SWRES_R0_OFFSET,
		ETHSW_SWRES_R0_SHIFT, ETHSW_SWRES_R0_SIZE, value);
	do {
		udelay(100);
		gsw_r32(cdev, ETHSW_SWRES_R0_OFFSET,
			ETHSW_SWRES_R0_SHIFT, ETHSW_SWRES_R0_SIZE, &value);
	} while (value);
	return GSW_statusOk;
}

GSW_return_t GSW_VersionGet(void *cdev, GSW_version_t *parm)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if (parm->nId == 0) {
		memcpy(parm->cName, VERSION_NAME, sizeof(VERSION_NAME));
		memcpy(parm->cVersion, VERSION_NUMBER, sizeof(VERSION_NUMBER));
	} else if (parm->nId == 1) {
		memcpy(parm->cName, MICRO_CODE_VERSION_NAME,
			sizeof(MICRO_CODE_VERSION_NAME));
		memcpy(parm->cVersion, MICRO_CODE_VERSION_NUMBER,
			sizeof(MICRO_CODE_VERSION_NUMBER));
	} else {
		memcpy(parm->cName, "", 0);
		memcpy(parm->cVersion, "", 0);
	}
	return GSW_statusOk;
}

#if 0
int platform_device_init(void *cdev)
{
	u32 reg_val, phy_reg;
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	if (gswdev->gipver == LTQ_GSWIP_2_0) {
		/* Set Port 6 RBUF_BYPASS mode */
		gsw_r32(cdev,
			(MAC_LPITMER0_TMLSB_OFFSET + (6 * 0xC)),
			PHY_ADDR_4_ADDR_SHIFT,
			PHY_ADDR_4_ADDR_SIZE, &phy_reg);
		phy_reg |= (1 << 6);
		gsw_w32(cdev,
			(MAC_LPITMER0_TMLSB_OFFSET + (6 * 0xC)),
			PHY_ADDR_4_ADDR_SHIFT,
			PHY_ADDR_4_ADDR_SIZE, phy_reg);
		gsw_w32(cdev,
			(PMAC_RX_IPG_IPG_CNT_OFFSET + GSW_TREG_OFFSET),
			PMAC_RX_IPG_IPG_CNT_SHIFT,
			PMAC_RX_IPG_IPG_CNT_SIZE, 0xB);
	}
	return GSW_statusOk;
}
#endif /* if 0*/

GSW_return_t GSW_Enable(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 j;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
/*	if (gswdev->hwinit == 0) */
/*		platform_device_init(gswdev); */
	for (j = 0; j < gswdev->pnum; j++) {
		gsw_w32(cdev, (FDMA_PCTRL_EN_OFFSET + (j * 0x6)),
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 1);
		gsw_w32(cdev, (SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 1);
		gsw_w32(cdev, (BM_PCFG_CNTEN_OFFSET + (j * 2)),
			BM_PCFG_CNTEN_SHIFT, BM_PCFG_CNTEN_SIZE, 1);
#if defined(CONFIG_USE_EMULATOR) && CONFIG_USE_EMULATOR
		if (gswdev->gipver == LTQ_GSWIP_3_0) {
			gsw_w32(cdev,
				(MAC_CTRL_0_FDUP_OFFSET + (0xC * j)),
				MAC_CTRL_0_FDUP_SHIFT, MAC_CTRL_0_FDUP_SIZE, 1);
			gsw_w32(cdev,
				(MAC_CTRL_0_GMII_OFFSET + (0xC * j)),
				MAC_CTRL_0_GMII_SHIFT, MAC_CTRL_0_GMII_SIZE, 2);
			gsw_w32(cdev,
				((GSWT_PHY_ADDR_1_SPEED_OFFSET
				+ (j * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_SPEED_SHIFT,
				GSWT_PHY_ADDR_1_SPEED_SIZE, 2);
			gsw_w32(cdev,
				((GSWT_PHY_ADDR_1_FDUP_OFFSET
				+ (j * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_FDUP_SHIFT,
				GSWT_PHY_ADDR_1_FDUP_SIZE, 1);
			gsw_w32(cdev,
				((GSWT_PHY_ADDR_1_LNKST_OFFSET
				+ (j * 4)) + GSW30_TOP_OFFSET),
				GSWT_PHY_ADDR_1_LNKST_SHIFT,
				GSWT_PHY_ADDR_1_LNKST_SIZE, 1);
		} else {
			gsw_w32(cdev,
				((PHY_ADDR_0_SPEED_OFFSET - j)
				+ GSW_TREG_OFFSET),
				PHY_ADDR_0_SPEED_SHIFT,
				PHY_ADDR_0_SPEED_SIZE, 2);
			gsw_w32(cdev,
				((PHY_ADDR_0_FDUP_OFFSET - j)
				+ GSW_TREG_OFFSET),
				PHY_ADDR_0_FDUP_SHIFT,
				PHY_ADDR_0_FDUP_SIZE, 1);
			gsw_w32(cdev,
				((PHY_ADDR_0_LNKST_OFFSET - j)
				+ GSW_TREG_OFFSET),
				PHY_ADDR_0_LNKST_SHIFT,
				PHY_ADDR_0_LNKST_SIZE, 1);
		}
#endif
	}
	if (gswdev->gipver == LTQ_GSWIP_3_0) {
		for (j = 0; j < gswdev->tpnum; j++) {
			gsw_w32(cdev, (BM_PCFG_CNTEN_OFFSET + (j * 2)),
				BM_PCFG_CNTEN_SHIFT, BM_PCFG_CNTEN_SIZE, 1);
			gsw_w32(cdev, (BM_RMON_CTRL_BCAST_CNT_OFFSET + (j * 0x2)),
				BM_RMON_CTRL_BCAST_CNT_SHIFT, BM_RMON_CTRL_BCAST_CNT_SIZE, 1);
		}
		if (gswdev->sdev == LTQ_FLOW_DEV_INT_R) {
			gsw_w32(cdev, PCE_TFCR_NUM_NUM_OFFSET,
				PCE_TFCR_NUM_NUM_SHIFT,
				PCE_TFCR_NUM_NUM_SIZE, 0x80);
		}
	}
	return GSW_statusOk;
}

GSW_return_t GSW_Disable(void *cdev)
{
	ethsw_api_dev_t *gswdev = (ethsw_api_dev_t *)cdev;
	u8 j;
	if (gswdev == NULL) {
		pr_err("%s:%s:%d",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	/* Disable all physical port  */
	for (j = 0; j < gswdev->pnum; j++) {
		gsw_w32(cdev, (FDMA_PCTRL_EN_OFFSET + (j * 0x6)),
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 0);
		gsw_w32(cdev, (SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 0);
	}
	return GSW_statusOk;
}
