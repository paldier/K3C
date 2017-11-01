/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "ltq_ethsw_init.h"

#ifdef LTQ_ETHSW_API_COC
	#include <ltq_ethsw_pm.h>
#endif

/* Global Variables */
ethsw_api_dev_t *pCoreDev[LTQ_FLOW_DEV_MAX];
u8 VERSION_NAME[] = GSW_API_MODULE_NAME " for GSWITCH Platform";
u8 VERSION_NUMBER[] = GSW_API_DRV_VERSION;
u8 MICRO_CODE_VERSION_NAME[] = " GSWIP macro revision ID";
u8 MICRO_CODE_VERSION_NUMBER[] = "0x"MICRO_CODE_VERSION;
static u8 PortRedirectFlag;

/* Local Macros & Definitions */
static port_stp_state_t port_state_table[] = {
	{ IFX_ETHSW_STP_PORT_STATE_FORWARD,	\
		IFX_ETHSW_8021X_PORT_STATE_AUTHORIZED,	\
		1, PORT_STATE_FORWARDING, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_FORWARD,	\
		IFX_ETHSW_8021X_PORT_STATE_UNAUTHORIZED,	\
		1, PORT_STATE_LISTENING, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_FORWARD,	\
		IFX_ETHSW_8021X_PORT_STATE_RX_AUTHORIZED,	\
		1, PORT_STATE_RX_ENABLE_TX_DISABLE, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_FORWARD,	\
		IFX_ETHSW_8021X_PORT_STATE_TX_AUTHORIZED,	\
		1, PORT_STATE_RX_DISABLE_TX_ENABLE, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_DISABLE,	\
		IFX_ETHSW_8021X_PORT_STATE_AUTHORIZED,	\
		0, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_DISABLE,	\
		IFX_ETHSW_8021X_PORT_STATE_UNAUTHORIZED,	\
		0, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_DISABLE,	\
		IFX_ETHSW_8021X_PORT_STATE_RX_AUTHORIZED,	\
		0, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_DISABLE,	\
		IFX_ETHSW_8021X_PORT_STATE_TX_AUTHORIZED,	\
		0, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_LEARNING,	\
		IFX_ETHSW_8021X_PORT_STATE_AUTHORIZED,	\
		1, PORT_STATE_LEARNING, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_LEARNING,	\
		IFX_ETHSW_8021X_PORT_STATE_UNAUTHORIZED,	\
		1, PORT_STATE_LEARNING, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_LEARNING,	\
		IFX_ETHSW_8021X_PORT_STATE_RX_AUTHORIZED,	\
		1, PORT_STATE_LEARNING, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_LEARNING,	\
		IFX_ETHSW_8021X_PORT_STATE_TX_AUTHORIZED,	\
		1, PORT_STATE_LEARNING, 1 },
	{ IFX_ETHSW_STP_PORT_STATE_BLOCKING,	\
		IFX_ETHSW_8021X_PORT_STATE_AUTHORIZED,	\
		1, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_BLOCKING,	\
		IFX_ETHSW_8021X_PORT_STATE_UNAUTHORIZED,	\
		1, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_BLOCKING,	\
		IFX_ETHSW_8021X_PORT_STATE_RX_AUTHORIZED,	\
		1, PORT_STATE_LISTENING, 0 },
	{ IFX_ETHSW_STP_PORT_STATE_BLOCKING,	\
		IFX_ETHSW_8021X_PORT_STATE_TX_AUTHORIZED,	\
		1, PORT_STATE_LISTENING, 0 }
};

static gsw_CapDesc_t CAP_Description[] = {
	{ IFX_ETHSW_CAP_TYPE_PORT,	\
		"Number of physical Ethernet ports"},
	{ IFX_ETHSW_CAP_TYPE_VIRTUAL_PORT,	\
		"Number of virtual Ethernet ports"},
	{ IFX_ETHSW_CAP_TYPE_BUFFER_SIZE,	\
		"Size of internal packet memory [in Bytes]"},
	{ IFX_ETHSW_CAP_TYPE_SEGMENT_SIZE,	\
		"Number of Segment size per device"},
	{ IFX_ETHSW_CAP_TYPE_PRIORITY_QUEUE,	\
		"Number of priority queues per device"},
	{ IFX_ETHSW_CAP_TYPE_METER,	\
		"Number of meter instances"},
	{ IFX_ETHSW_CAP_TYPE_RATE_SHAPER,	\
		"Number of rate shaper instances"},
	{ IFX_ETHSW_CAP_TYPE_VLAN_GROUP,	\
		"Number of VLAN groups that can be configured on the switch hardware"},
	{ IFX_ETHSW_CAP_TYPE_FID,	\
		"Number of Forwarding database IDs [FIDs]"},
	{ IFX_ETHSW_CAP_TYPE_MAC_TABLE_SIZE,	\
		"Number of MAC table entries"},
	{ IFX_ETHSW_CAP_TYPE_MULTICAST_TABLE_SIZE,	\
		"Number of multicast level 3 hardware table entries"},
	{ IFX_ETHSW_CAP_TYPE_PPPOE_SESSION,	\
		"Number of supported PPPoE sessions"},
	{ IFX_ETHSW_CAP_TYPE_LAST,	\
		"Last Capability Index"}
};

/*****************/
/* Function Body */
/*****************/
static void ltq_ethsw_port_cfg_init(void *pDevCtx)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 i, value;
	gsw_r32(ETHSW_CAP_1_PPORTS_OFFSET, ETHSW_CAP_1_PPORTS_SHIFT,	\
		ETHSW_CAP_1_PPORTS_SIZE, &value);
	pEthDev->nPortNumber = value;
	for (i = 0; i < pEthDev->nPortNumber; i++) {
		memset(&pEthDev->PortConfig[i], 0 , sizeof(port_config_t));
		pEthDev->PortConfig[i].nLearningLimit = 0xFF;
		pEthDev->PortConfig[i].bPortEnable = 1;
	}
	pEthDev->STP_8021x_Config.eForwardPort	\
		= IFX_ETHSW_PORT_FORWARD_DEFAULT;
	pEthDev->STP_8021x_Config.n8021X_ForwardPortId = pEthDev->nCPU_Port;
	gsw_r32(ETHSW_CAP_1_VPORTS_OFFSET,	\
		ETHSW_CAP_1_VPORTS_SHIFT,	\
		ETHSW_CAP_1_VPORTS_SIZE, &value);
	pEthDev->nTotalPortNumber = value + pEthDev->nPortNumber;
}
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
static void reset_multicast_sw_table(void *pDevCtx)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 i, value;
	gsw_r32(ETHSW_CAP_8_MCAST_OFFSET,	\
		ETHSW_CAP_8_MCAST_SHIFT,	\
		ETHSW_CAP_8_MCAST_SIZE, &value);
	memset(&pEthDev->IGMP_Flags, 0 , sizeof(gsw_igmp_t));
	pEthDev->IGMP_Flags.nSwTblSize = value;
	for (i = 0; i < pEthDev->IGMP_Flags.nSwTblSize; i++) {
		pEthDev->IGMP_Flags.multi_sw_table[i].SrcIp_LSB_Index = 0x7F;
		pEthDev->IGMP_Flags.multi_sw_table[i].DisIp_LSB_Index = 0x7F;
		pEthDev->IGMP_Flags.multi_sw_table[i].SrcIp_MSB_Index = 0x1F;
		pEthDev->IGMP_Flags.multi_sw_table[i].DisIp_MSB_Index = 0x1F;
	}
}

/* Multicast Software Table Include/Exclude Add function */
static int ifx_multicast_sw_table_write(void *pDevCtx,	\
	IFX_ETHSW_multicastTable_t *pPar)
{
	ethsw_api_dev_t *pDEVHandle = (ethsw_api_dev_t *)pDevCtx;
	int	i, j, exclude_rule = LTQ_FALSE;
	int	Dip_lsb_tab_index = 0x7F, Dip_msb_tab_index = 0x7F;
	int Sip_lsb_tab_index = 0x7F, Sip_msb_tab_index = 0x7F;
	int dip_lsb_ind = 0, dip_msb_ind = 0, sip_lsb_ind = 0, sip_msb_ind = 0;
	ltq_bool_t new_entry = LTQ_FALSE;
	void *pDev = NULL;
	ltq_pce_table_t *pIPTmHandle = &pDEVHandle->PCE_Handler;
	gsw_igmp_t *pIGMPTbHandle = &pDEVHandle->IGMP_Flags;
	pce_table_prog_t pData;
	pce_dasa_lsb_t	dasa_lsb_tbl;
	pce_dasa_msb_t	dasa_msb_tbl;

	memset(&pData, 0 , sizeof(pce_table_prog_t));
	memset(&dasa_lsb_tbl, 0, sizeof(pce_dasa_lsb_t));
	memset(&dasa_msb_tbl, 0, sizeof(pce_dasa_msb_t));
	if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_INCLUDE)	\
		|| (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE)) {
		if (pDEVHandle->IGMP_Flags.bIGMPv3 != LTQ_TRUE) {
			pr_err("%s:%s:%d(bIGMPv3 need to be enable)\n",	\
			__FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
	}
	if ((pPar->eIPVersion != IFX_ETHSW_IP_SELECT_IPV4)	\
		&& (pPar->eIPVersion != IFX_ETHSW_IP_SELECT_IPV6)) {
		pr_err("%s:%s:%d (IPv4/IPV6 need to enable) \n",	\
		__FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV4) {
		for (i = 0; i < 4 ; i++)
			dasa_lsb_tbl.ip_lsb[i] = ((pPar->uIP_Gda.nIPv4 >> (i * 8)) & 0xFF);
		dasa_lsb_tbl.mask = 0xFF00;
	}
	if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6) {
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_msb_tbl.ip_msb[j-1] = (pPar->uIP_Gda.nIPv6[i] & 0xFF);
			dasa_msb_tbl.ip_msb[j] = ((pPar->uIP_Gda.nIPv6[i] >> 8) & 0xFF);
		}
		dasa_msb_tbl.mask = 0;
		Dip_msb_tab_index = find_software_msb_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_msb_tbl);
		if (Dip_msb_tab_index == 0xFF) {
			Dip_msb_tab_index = pce_tm_ip_dasa_msb_tbl_write(&pIPTmHandle->pce_sub_tbl, &dasa_msb_tbl);
			dip_msb_ind = 1;
		}
		if (Dip_msb_tab_index < 0) {
			pr_err("%s:%s:%d (IGMP Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_lsb_tbl.ip_lsb[j-1] = (pPar->uIP_Gda.nIPv6[i+4] & 0xFF);
			dasa_lsb_tbl.ip_lsb[j]	= ((pPar->uIP_Gda.nIPv6[i+4] >> 8) & 0xFF);
		}
		dasa_lsb_tbl.mask = 0;/* DIP LSB Nibble Mask */
	}
	Dip_lsb_tab_index = find_software_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_lsb_tbl);
	if (Dip_lsb_tab_index == 0xFF) {
		Dip_lsb_tab_index = pce_tm_ip_dasa_lsb_tbl_write(&pIPTmHandle->pce_sub_tbl, &dasa_lsb_tbl);
		dip_lsb_ind = 1;
	}
	if (Dip_lsb_tab_index < 0) {
		pr_err("%s:%s:%d (IGMP Table full) \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_INCLUDE) || 	\
		(pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE)) {
		if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV4) {
			for (i = 0; i < 4 ; i++)
				dasa_lsb_tbl.ip_lsb[i] = ((pPar->uIP_Gsa.nIPv4 >> (i * 8)) & 0xFF);
			/* DIP LSB Nibble Mask */
			dasa_lsb_tbl.mask = 0xFF00;
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
				if (dasa_lsb_tbl.ip_lsb[3] == 0 && dasa_lsb_tbl.ip_lsb[2] == 0 &&	\
					dasa_lsb_tbl.ip_lsb[1] == 0 && dasa_lsb_tbl.ip_lsb[0] == 0) {
						pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard) \n", __FILE__, __func__, __LINE__);
						return LTQ_ERROR;
				}
			}
		}
		if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6) {
			int src_zero = 0;
				/* First, search for DIP in the DA/SA table (DIP MSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				dasa_msb_tbl.ip_msb[j-1] = (pPar->uIP_Gsa.nIPv6[i] & 0xFF);
				dasa_msb_tbl.ip_msb[j]	= ((pPar->uIP_Gsa.nIPv6[i] >> 8) & 0xFF);
			}
			dasa_msb_tbl.mask = 0;/* DIP MSB Nibble Mask */
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
				if (dasa_msb_tbl.ip_msb[0] == 0 && dasa_msb_tbl.ip_msb[1] == 0 &&	\
					dasa_msb_tbl.ip_msb[2] == 0 && dasa_msb_tbl.ip_msb[3] == 0 && 	\
					dasa_msb_tbl.ip_msb[4] == 0 && dasa_msb_tbl.ip_msb[5] == 0 && 	\
					dasa_msb_tbl.ip_msb[6] == 0 && dasa_msb_tbl.ip_msb[7] == 0) {
						src_zero = 1;
				}
			}
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				dasa_lsb_tbl.ip_lsb[j-1] = (pPar->uIP_Gsa.nIPv6[i+4] & 0xFF);
				dasa_lsb_tbl.ip_lsb[j] = ((pPar->uIP_Gsa.nIPv6[i+4] >> 8) & 0xFF);
			}
			dasa_lsb_tbl.mask = 0;/* DIP LSB Nibble Mask */
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
				if (dasa_lsb_tbl.ip_lsb[0] == 0 && dasa_lsb_tbl.ip_lsb[1] == 0 &&	\
					dasa_lsb_tbl.ip_lsb[2] == 0 && dasa_lsb_tbl.ip_lsb[3] == 0 && 	\
					dasa_lsb_tbl.ip_lsb[4] == 0 && dasa_lsb_tbl.ip_lsb[5] == 0 && 	\
					dasa_lsb_tbl.ip_lsb[6] == 0 && dasa_lsb_tbl.ip_lsb[7] == 0) {
					if (src_zero) {
						pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard) \n", __FILE__, __func__, __LINE__);
						return LTQ_ERROR;
					}
				}
			}
			Sip_msb_tab_index = find_software_msb_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_msb_tbl);
			if (Sip_msb_tab_index == 0xFF) {
				Sip_msb_tab_index = pce_tm_ip_dasa_msb_tbl_write(&pIPTmHandle->pce_sub_tbl, &dasa_msb_tbl);
				sip_msb_ind = 1;
			}
			if (Sip_msb_tab_index < 0) {
				pr_err("%s:%s:%d (IGMP Table full) \n", __FILE__, __func__, __LINE__);
				return LTQ_ERROR;
			}
		}
		Sip_lsb_tab_index = find_software_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (Sip_lsb_tab_index == 0xFF) {
			Sip_lsb_tab_index = pce_tm_ip_dasa_lsb_tbl_write(&pIPTmHandle->pce_sub_tbl, &dasa_lsb_tbl);
			sip_lsb_ind = 1;
		}
		if (Sip_lsb_tab_index < 0) {
			pr_err("%s:%s:%d (IGMP Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
	}
	/* update the entry for another port number if already exists*/
	for (i = 0; i < pDEVHandle->IGMP_Flags.nSwTblSize; i++) {
		/* Check if port was already exist */
		if ((pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index == Dip_lsb_tab_index) &&	\
			(pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index == Dip_msb_tab_index) &&
			(pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index == Sip_lsb_tab_index) &&	\
			(pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index == Sip_msb_tab_index) &&
			(pIGMPTbHandle->multi_sw_table[i].valid == LTQ_TRUE)) {
			if (((pIGMPTbHandle->multi_sw_table[i].PortMap >> pPar->nPortId) & 0x1) == 1)
				return LTQ_SUCCESS;
			switch (pIGMPTbHandle->multi_sw_table[i].eModeMember) {
			case IFX_ETHSW_IGMP_MEMBER_DONT_CARE:
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index]++;
					if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6)
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index]++;
					/* Add the port */
					pIGMPTbHandle->multi_sw_table[i].PortMap |= (1 << pPar->nPortId);
					break;
			case IFX_ETHSW_IGMP_MEMBER_EXCLUDE:
					exclude_rule = LTQ_TRUE;
			case IFX_ETHSW_IGMP_MEMBER_INCLUDE:
					/* Add the port */
					pIGMPTbHandle->multi_sw_table[i].PortMap |= (1 << pPar->nPortId);
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index]++;
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Sip_lsb_tab_index]++;
					if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6) {
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index]++;
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Sip_msb_tab_index]++;
					}
					break;
			} /* end switch */
			/* Now, we write into Multicast SW Table */
			memset(&pData, 0 , sizeof(pce_table_prog_t));
			pData.table = PCE_MULTICAST_SW_INDEX;
			pData.table_index = i;
			pData.key[1] = (pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index << 8)	\
				| pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index;
			pData.key[0] = (pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index << 8)	\
				| pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE)
				pData.val[0] = (0 << pPar->nPortId);
			else
				pData.val[0] = pIGMPTbHandle->multi_sw_table[i].PortMap;
			pData.valid = pIGMPTbHandle->multi_sw_table[i].valid;
			xwayflow_pce_table_write(pDev, &pData);
			new_entry = LTQ_TRUE;
			if (exclude_rule == LTQ_FALSE)
				return LTQ_SUCCESS;
		}
	}

	/* wildcard entry for EXCLUDE rule for  port number if already exists*/
	if ((exclude_rule == LTQ_TRUE) && (new_entry == LTQ_TRUE)) {
		for (i = 0; i < pDEVHandle->IGMP_Flags.nSwTblSize; i++) {
			/* Check if port was already exist */
			if ((pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index == Dip_lsb_tab_index) &&	\
				(pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index == Dip_msb_tab_index) &&
				(pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index == 0x7F) &&	\
				(pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index == 0x7F) &&
				(pIGMPTbHandle->multi_sw_table[i].valid == LTQ_TRUE)) {
				if (((pIGMPTbHandle->multi_sw_table[i].PortMap >> pPar->nPortId) & 0x1) == 1) {
					return LTQ_SUCCESS;
				} else {
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index]++;
					if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6)
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index]++;
					/* Add the port */
					pIGMPTbHandle->multi_sw_table[i].PortMap |= (1 << pPar->nPortId);
				}
				pIGMPTbHandle->multi_sw_table[i].eModeMember = IFX_ETHSW_IGMP_MEMBER_DONT_CARE;
				memset(&pData, 0 , sizeof(pce_table_prog_t));
				pData.table = PCE_MULTICAST_SW_INDEX;
				pData.table_index = i;
				pData.key[1] = (pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index << 8)	\
					| pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index;
				pData.key[0] = (pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index << 8)	\
					| pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
				pData.val[0] = pIGMPTbHandle->multi_sw_table[i].PortMap;
				pData.valid = pIGMPTbHandle->multi_sw_table[i].valid;
				xwayflow_pce_table_write(pDev, &pData);
				return LTQ_SUCCESS;
			}
		}
	}
/* Create the new DstIP & SrcIP entry */
	if (new_entry == LTQ_FALSE) {
		if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_INCLUDE) ||	\
				(pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE))	{
			i = 0;
			while (i < pDEVHandle->IGMP_Flags.nSwTblSize) {
				/* Find a new empty entry to add */
				if (pIGMPTbHandle->multi_sw_table[i].valid == LTQ_FALSE)
					break;
				i++;
			}
		} else if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_DONT_CARE) {
			i = 63;
			while (i > 0) {
				/* Find a new empty entry to add */
				if (pIGMPTbHandle->multi_sw_table[i].valid == LTQ_FALSE)
					break;
				i--;
			}
		}
		if (i >= 0 && i < pDEVHandle->IGMP_Flags.nSwTblSize) {
			pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index = Dip_lsb_tab_index;
			pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index = Dip_msb_tab_index;
			pIGMPTbHandle->multi_sw_table[i].PortMap |= (1 << pPar->nPortId);
			if (dip_lsb_ind)
				pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] = 1;
			else
				pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index]++;
			if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6) {
				if (dip_msb_ind)
					pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] = 1;
				else
					pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index]++;
			}
			pIGMPTbHandle->multi_sw_table[i].valid = LTQ_TRUE;
			pIGMPTbHandle->multi_sw_table[i].eModeMember = pPar->eModeMember;
			if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_INCLUDE) ||	\
				(pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE)) {
				pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index = Sip_lsb_tab_index;
				pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index = Sip_msb_tab_index;
				if (sip_lsb_ind)
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Sip_lsb_tab_index] = 1;
				else
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Sip_lsb_tab_index]++;
				if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6) {
					if (sip_msb_ind)
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Sip_msb_tab_index] = 1;
					else
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Sip_msb_tab_index]++;
				}
			} else if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_DONT_CARE) {
				pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index = 0x7F;
				pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index = 0x7F;
			}
		}
		memset(&pData, 0 , sizeof(pce_table_prog_t));
		/* Now, we write into Multicast SW Table */
		pData.table = PCE_MULTICAST_SW_INDEX;
		pData.table_index = i;
		pData.key[1] = (pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index;
		pData.key[0] = (pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
		if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE))
			pData.val[0] = (0 << pPar->nPortId);
		else
			pData.val[0] = pIGMPTbHandle->multi_sw_table[i].PortMap;
		pData.valid = pIGMPTbHandle->multi_sw_table[i].valid ;
		xwayflow_pce_table_write(pDev, &pData);

		if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE)) {
			for (i = 0; i < pDEVHandle->IGMP_Flags.nSwTblSize; i++) {
				/* Check if port was already exist */
				if ((pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index == Dip_lsb_tab_index) &&	\
					(pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index == Dip_msb_tab_index) &&
					(pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index == 0x7F) &&	\
					(pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index == 0x7F) &&
					(pIGMPTbHandle->multi_sw_table[i].valid == LTQ_TRUE)) {
					if (((pIGMPTbHandle->multi_sw_table[i].PortMap >> pPar->nPortId) & 0x1) == 1)
						return LTQ_SUCCESS;
					pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index]++;
					if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6)
						pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index]++;
					pIGMPTbHandle->multi_sw_table[i].eModeMember = IFX_ETHSW_IGMP_MEMBER_DONT_CARE;
					/* Add the port */
					pIGMPTbHandle->multi_sw_table[i].PortMap |= (1 << pPar->nPortId);
					memset(&pData, 0 , sizeof(pce_table_prog_t));
					pData.table = PCE_MULTICAST_SW_INDEX;
					pData.table_index = i;
					pData.key[1] = (pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index << 8)	\
						| pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index;
					pData.key[0] = (pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index << 8)	\
						| pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
					pData.val[0] = pIGMPTbHandle->multi_sw_table[i].PortMap;
					pData.valid = pIGMPTbHandle->multi_sw_table[i].valid ;
					xwayflow_pce_table_write(pDev, &pData);
					return LTQ_SUCCESS;
				}
			}
			i = 63;
			while (i > 0) {
				/* Find a new empty entry to add */
				if (pIGMPTbHandle->multi_sw_table[i].valid == LTQ_FALSE)
					break;
				i--;
			}
		if (i >= 0 && i < pDEVHandle->IGMP_Flags.nSwTblSize) {
				/* Now, we write into Multicast SW Table */
			pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index = Dip_lsb_tab_index;
			pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index = Dip_msb_tab_index;
			pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index = 0x7F;
			pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index = 0x7F;
			pIGMPTbHandle->multi_sw_table[i].PortMap |= (1 << pPar->nPortId);
			pIGMPTbHandle->multi_sw_table[i].eModeMember = IFX_ETHSW_IGMP_MEMBER_DONT_CARE;
			pIGMPTbHandle->multi_sw_table[i].valid = LTQ_TRUE;
			pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index]++;
			if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6)
				pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index]++;
			memset(&pData, 0 , sizeof(pce_table_prog_t));
			pData.table = PCE_MULTICAST_SW_INDEX;
			pData.table_index = i;
			pData.key[1] = (pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index;
			pData.key[0] = (pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
			pData.val[0] = pIGMPTbHandle->multi_sw_table[i].PortMap;
			pData.valid = pIGMPTbHandle->multi_sw_table[i].valid ;
			xwayflow_pce_table_write(pDev, &pData);
			} else {
				pr_err("%s:%s:%d (IGMP Table full) \n", __FILE__, __func__, __LINE__);
			}
		}
	}
	/* Debug */
	return LTQ_SUCCESS;
}

/* Multicast Software Table Include/Exclude Remove function */
static int ifx_multicast_sw_table_remove(void *pDevCtx, IFX_ETHSW_multicastTable_t *pPar)
{
	ethsw_api_dev_t *pDEVHandle = (ethsw_api_dev_t *)pDevCtx;
	u8	i, j;
	ltq_bool_t MATCH = LTQ_FALSE;
	void *pDev = NULL;
	ltq_pce_table_t *pIPTmHandle = &pDEVHandle->PCE_Handler;
	gsw_igmp_t *pIGMPTbHandle = &pDEVHandle->IGMP_Flags;
	pce_table_prog_t pData;
	pce_dasa_lsb_t dasa_lsb_tbl;
	pce_dasa_msb_t dasa_msb_tbl;
	int Dip_lsb_tab_index = 0x7F, Dip_msb_tab_index = 0x7F,	\
		Sip_lsb_tab_index = 0x7F, Sip_msb_tab_index = 0x7F;
	memset(&pData, 0 , sizeof(pce_table_prog_t));
	memset(&dasa_lsb_tbl, 0, sizeof(pce_dasa_lsb_t));
	memset(&dasa_msb_tbl, 0, sizeof(pce_dasa_msb_t));
	if ((pPar->eIPVersion != IFX_ETHSW_IP_SELECT_IPV4)	\
		&& (pPar->eIPVersion != IFX_ETHSW_IP_SELECT_IPV6)) {
		pr_err("%s:%s:%d (IPv4/IPV6 need to enable!!!) \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_INCLUDE) &&	\
		(pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) &&	\
		(pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_DONT_CARE)) {
		pr_err("%s:%s:%d (!!!) \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV4 /* IPv4 */) {
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0; i < 4 ; i++)
			dasa_lsb_tbl.ip_lsb[i] = ((pPar->uIP_Gda.nIPv4 >> (i * 8)) & 0xFF);
		/* DIP LSB Nibble Mask */
		dasa_lsb_tbl.mask = 0xFF00;
	}
	if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6 /* IPv6 */) {
		/* First, search for DIP in the DA/SA table (DIP MSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_msb_tbl.ip_msb[j-1] = (pPar->uIP_Gda.nIPv6[i] & 0xFF);
			dasa_msb_tbl.ip_msb[j]	= ((pPar->uIP_Gda.nIPv6[i] >> 8) & 0xFF);
		}
		dasa_msb_tbl.mask = 0;/* DIP MSB Nibble Mask */
		Dip_msb_tab_index = find_software_msb_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_msb_tbl);
		if (Dip_msb_tab_index == 0xFF) {
			pr_err("%s:%s:%d (IGMP Entry not found) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_lsb_tbl.ip_lsb[j-1] = (pPar->uIP_Gda.nIPv6[i+4] & 0xFF);
			dasa_lsb_tbl.ip_lsb[j] = ((pPar->uIP_Gda.nIPv6[i+4] >> 8) & 0xFF);
		}
		dasa_lsb_tbl.mask = 0;/* DIP LSB Nibble Mask */
	}
	Dip_lsb_tab_index = find_software_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_lsb_tbl);
	if (Dip_lsb_tab_index == 0xFF) {
		pr_err("%s:%s:%d (IGMP Entry not found) \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if ((pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_INCLUDE) ||	\
		(pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE)) {
		if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV4 /* IPv4 */) {
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0; i < 4; i++)
				dasa_lsb_tbl.ip_lsb[i] = ((pPar->uIP_Gsa.nIPv4 >> (i * 8)) & 0xFF);
			/* DIP LSB Nibble Mask */
			dasa_lsb_tbl.mask = 0xFF00;
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
				if (dasa_lsb_tbl.ip_lsb[3] == 0 && dasa_lsb_tbl.ip_lsb[2] == 0 &&	\
					dasa_lsb_tbl.ip_lsb[1] == 0 && dasa_lsb_tbl.ip_lsb[0] == 0) {
					pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard) \n", __FILE__, __func__, __LINE__);
					return LTQ_ERROR;
				}
			}
		}
		if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6) {
			int src_zero = 0;
			/* First, search for DIP in the DA/SA table (DIP MSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				dasa_msb_tbl.ip_msb[j-1] = (pPar->uIP_Gsa.nIPv6[i] & 0xFF);
				dasa_msb_tbl.ip_msb[j] = ((pPar->uIP_Gsa.nIPv6[i] >> 8) & 0xFF);
			}
			dasa_msb_tbl.mask = 0;/* DIP MSB Nibble Mask */
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
				if (dasa_msb_tbl.ip_msb[0] == 0 && dasa_msb_tbl.ip_msb[1] == 0 &&	\
					dasa_msb_tbl.ip_msb[2] == 0 && dasa_msb_tbl.ip_msb[3] == 0 && 	\
					dasa_msb_tbl.ip_msb[4] == 0 && dasa_msb_tbl.ip_msb[5] == 0 && 	\
					dasa_msb_tbl.ip_msb[6] == 0 && dasa_msb_tbl.ip_msb[7] == 0)
						src_zero = 1;
				}
			Sip_msb_tab_index = find_software_msb_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_msb_tbl);
			if (Sip_msb_tab_index == 0xFF) {
				pr_err("%s:%s:%d (IGMP Entry not found) \n", __FILE__, __func__, __LINE__);
				return LTQ_ERROR;
			}
			/* First, search for DIP in the DA/SA table (DIP LSB) */
			for (i = 0, j = 7; i < 4; i++, j -= 2) {
				dasa_lsb_tbl.ip_lsb[j-1] = (pPar->uIP_Gsa.nIPv6[i+4] & 0xFF);
				dasa_lsb_tbl.ip_lsb[j] = ((pPar->uIP_Gsa.nIPv6[i+4] >> 8) & 0xFF);
			}
			dasa_lsb_tbl.mask = 0;/* DIP LSB Nibble Mask */
			if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
				if (dasa_lsb_tbl.ip_lsb[0] == 0 && dasa_lsb_tbl.ip_lsb[1] == 0 && \
					dasa_lsb_tbl.ip_lsb[2] == 0 && dasa_lsb_tbl.ip_lsb[3] == 0 && \
					dasa_lsb_tbl.ip_lsb[4] == 0 && dasa_lsb_tbl.ip_lsb[5] == 0 && \
					dasa_lsb_tbl.ip_lsb[6] == 0 && dasa_lsb_tbl.ip_lsb[7] == 0) {
						if (src_zero) {
							pr_err("%s:%s:%d (Exclude Rule Source IP is Wildcard) \n", __FILE__, __func__, __LINE__);
							return LTQ_ERROR;
						}
				}
			}
		}
		Sip_lsb_tab_index = find_software_tbl_entry(&pIPTmHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (Sip_lsb_tab_index == 0xFF) {
			pr_err("%s:%s:%d (IGMP Entry not found) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
	}
	for (i = 0; i < pDEVHandle->IGMP_Flags.nSwTblSize; i++) {
		if ((pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index == Dip_lsb_tab_index) &&	\
			(pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index == Sip_lsb_tab_index) &&	\
			(pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index == Dip_msb_tab_index) &&	\
			(pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index == Sip_msb_tab_index) &&	\
			(pIGMPTbHandle->multi_sw_table[i].valid == LTQ_TRUE)) {
			switch (pIGMPTbHandle->multi_sw_table[i].eModeMember) {
			case IFX_ETHSW_IGMP_MEMBER_DONT_CARE:
					if (((pIGMPTbHandle->multi_sw_table[i].PortMap >> pPar->nPortId) & 0x1) == 1) {
						pIGMPTbHandle->multi_sw_table[i].PortMap &= ~(1 << pPar->nPortId);
						if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] > 0) {
							pce_tm_ip_dasa_lsb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Dip_lsb_tab_index);
							if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] == 0) {
								/* Delet the sub table */
								pce_tm_ip_dasa_lsb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Dip_lsb_tab_index);
							}
						}
						if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] > 0) {
							pce_tm_ip_dasa_msb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Dip_msb_tab_index);
							if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] == 0) {
							/* Delet the sub table */
							if (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6 /* IPv6 */)
								pce_tm_ip_dasa_msb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Dip_msb_tab_index);
							}
						}
						/* Check the port map status */
						if (pIGMPTbHandle->multi_sw_table[i].PortMap == 0) {
							/* Delet the entry from Multicast sw Table */
							pIGMPTbHandle->multi_sw_table[i].valid = LTQ_FALSE;
						}
						MATCH = LTQ_TRUE;
					}
						break;
			case IFX_ETHSW_IGMP_MEMBER_INCLUDE:
			case IFX_ETHSW_IGMP_MEMBER_EXCLUDE:
					if (((pIGMPTbHandle->multi_sw_table[i].PortMap >> pPar->nPortId) & 0x1) == 1) {
						pIGMPTbHandle->multi_sw_table[i].PortMap &= ~(1 << pPar->nPortId);
						if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] > 0) {
							pce_tm_ip_dasa_lsb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Dip_lsb_tab_index);
							if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] == 0) {
								/* Delet the sub table */
								pce_tm_ip_dasa_lsb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Dip_lsb_tab_index);
							}
						}
						if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] > 0) {
							pce_tm_ip_dasa_msb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Dip_msb_tab_index);
							if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] == 0) {
							/* Delet the sub table */
								pce_tm_ip_dasa_msb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Dip_msb_tab_index);
							}
						}
						if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Sip_lsb_tab_index] > 0) {
							pce_tm_ip_dasa_lsb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Sip_lsb_tab_index);
							if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Sip_lsb_tab_index] == 0) {
								/* Delet the sub table */
								pce_tm_ip_dasa_lsb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Sip_lsb_tab_index);
							}
						}
						if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Sip_msb_tab_index] > 0) {
							pce_tm_ip_dasa_msb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Sip_msb_tab_index);
							if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Sip_msb_tab_index] == 0) {
							/* Delet the sub table */
								pce_tm_ip_dasa_msb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Sip_msb_tab_index);
							}
						}
						/* Check the port map status */
						if (pIGMPTbHandle->multi_sw_table[i].PortMap == 0) {
							/* Delet the entry from Multicast sw Table */
							pIGMPTbHandle->multi_sw_table[i].valid = LTQ_FALSE;
						}
						MATCH = LTQ_TRUE;
						if (pPar->eModeMember == IFX_ETHSW_IGMP_MEMBER_EXCLUDE) {
							for (j = 0; j < pDEVHandle->IGMP_Flags.nSwTblSize; j++) {
								if ((pIGMPTbHandle->multi_sw_table[j].DisIp_LSB_Index == Dip_lsb_tab_index) &&	\
									(pIGMPTbHandle->multi_sw_table[j].SrcIp_LSB_Index == 0x7F) &&	\
									(pIGMPTbHandle->multi_sw_table[j].DisIp_MSB_Index == Dip_msb_tab_index) &&	\
									(pIGMPTbHandle->multi_sw_table[j].SrcIp_MSB_Index == 0x7F) &&	\
									(pIGMPTbHandle->multi_sw_table[j].valid == LTQ_TRUE)) {
									if (((pIGMPTbHandle->multi_sw_table[j].PortMap >> pPar->nPortId) & 0x1) == 1) {
										pIGMPTbHandle->multi_sw_table[j].PortMap &= ~(1 << pPar->nPortId);
										if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] > 0) {
											pce_tm_ip_dasa_lsb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Dip_lsb_tab_index);
											if (pIPTmHandle->pce_sub_tbl.ip_dasa_lsb_tbl_cnt[Dip_lsb_tab_index] == 0) {
												/* Delet the sub table */
												pce_tm_ip_dasa_lsb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Dip_lsb_tab_index);
											}
										}
									if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] > 0) {
										pce_tm_ip_dasa_msb_tbl_idx_delete(&pIPTmHandle->pce_sub_tbl, Dip_msb_tab_index);
										if (pIPTmHandle->pce_sub_tbl.ip_dasa_msb_tbl_cnt[Dip_msb_tab_index] == 0) {
											/* Delet the sub table */
											pce_tm_ip_dasa_msb_tbl_delete(&pIPTmHandle->pce_sub_tbl, Dip_msb_tab_index);
										}
									}
									/* Check the port map status */
									if (pIGMPTbHandle->multi_sw_table[j].PortMap == 0) {
										/* Delet the entry from Multicast sw Table */
										pIGMPTbHandle->multi_sw_table[j].valid = LTQ_FALSE;
										pIGMPTbHandle->multi_sw_table[i].valid = LTQ_FALSE;
									}
									memset(&pData, 0 , sizeof(pce_table_prog_t));
									pData.table = PCE_MULTICAST_SW_INDEX;
									pData.table_index = j;
									pData.key[1] = ((0x7F << 8) | 0x7F);
									pData.key[0] = (pIGMPTbHandle->multi_sw_table[j].DisIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
									pData.val[0] = pIGMPTbHandle->multi_sw_table[j].PortMap;
									pData.valid = pIGMPTbHandle->multi_sw_table[j].valid;
									xwayflow_pce_table_write(pDev, &pData);
								}
							}
						}
					}
				}
					break;
				}
			if (MATCH == LTQ_TRUE) {
				memset(&pData, 0 , sizeof(pce_table_prog_t));
				pData.table = PCE_MULTICAST_SW_INDEX;
				pData.table_index = i;
				pData.key[1] = (pIGMPTbHandle->multi_sw_table[i].SrcIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].SrcIp_LSB_Index;
				pData.key[0] = (pIGMPTbHandle->multi_sw_table[i].DisIp_MSB_Index << 8) | pIGMPTbHandle->multi_sw_table[i].DisIp_LSB_Index;
				pData.val[0] = pIGMPTbHandle->multi_sw_table[i].PortMap;
				pData.valid	= pIGMPTbHandle->multi_sw_table[i].valid;
				xwayflow_pce_table_write(pDev, &pData);
			}
		}
	}
	if (MATCH == LTQ_FALSE)
		pr_err("The GIP/SIP not found\n");
   return LTQ_SUCCESS;
}
#endif /*CONFIG_LTQ_MULTICAST */
#if  ((defined(CONFIG_LTQ_STP) && CONFIG_LTQ_STP) || (defined(CONFIG_LTQ_8021X) && CONFIG_LTQ_8021X))
/*
 *  Function:    Internal function to program the registers when 802.1x and STP API are called.
 *  Description: Referene the matrix table to program the LRNLIM, PSTATE and PEN bit
 *               according to the Software architecture spec design.
 */
static void set_port_state(void *pDevCtx, u32 PortID, u32 ifx_stp_state, u32 ifx_8021_state)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 i;

	for (i = 0; i < sizeof(port_state_table)/sizeof(port_stp_state_t); i++) {
		port_stp_state_t *pTable = &port_state_table[i];
		if ((pTable->ifx_stp_state == ifx_stp_state) && (pTable->ifx_8021_state == ifx_8021_state)) {
			pEthDev->PortConfig[PortID].bPortEnable = pTable->pen_reg;
			pEthDev->PortConfig[PortID].nPortState = pTable->pstate_reg;
			/* Learning Limit */
			if (pTable->lrnlim == 0) {
				gsw_w32((PCE_PCTRL_1_LRNLIM_OFFSET + (0xA * PortID)),
					PCE_PCTRL_1_LRNLIM_SHIFT, PCE_PCTRL_1_LRNLIM_SIZE, 0);
			} else {
				gsw_w32((PCE_PCTRL_1_LRNLIM_OFFSET + (0xA * PortID)),	\
					PCE_PCTRL_1_LRNLIM_SHIFT, PCE_PCTRL_1_LRNLIM_SIZE,	\
					pEthDev->PortConfig[PortID].nLearningLimit);
			}
			/* Port State */
			gsw_w32((PCE_PCTRL_0_PSTATE_OFFSET + (0xA * PortID)),	\
				PCE_PCTRL_0_PSTATE_SHIFT, PCE_PCTRL_0_PSTATE_SIZE,	\
				pEthDev->PortConfig[PortID].nPortState);
			/* Port Enable */
			gsw_w32((SDMA_PCTRL_PEN_OFFSET + (0xA * PortID)),	\
				SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE,	\
				pEthDev->PortConfig[PortID].bPortEnable);
		}
	}
}
#endif /* CONFIG_LTQ_STP / CONFIG_LTQ_8021X */
#if defined(CONFIG_LTQ_QOS) && CONFIG_LTQ_QOS
/*
 *  Function:    Internal function to calculate the Rate when Shaper and Meter API is called.
 *  Description: Calculate the Rate by input Ibs, Exp and Mant.
 *               The algorithm designed based on software architecture spec.
 */
static u32 RateCalc(u32 pIbsIdx, u32 pExp, u32 pMant)
{
	static const u16 ibs_table[] = {8*8, 32*8, 64*8, 96*8};
	u16 ibs;
	u32 Rate;

	if ((pIbsIdx == 0) && (pExp == 0) && (pMant == 0))
		return 0;
	ibs = ibs_table[pIbsIdx];
	Rate = ((ibs * 25000) >> pExp) / pMant;
	return Rate;
}

/*
 *  Function:    Internal function to calculate the Token when Shaper and Meter API is called.
 *  Description: Calculate the Token by input Rate, Ibs, Exp and Mant.
 *               The algorithm designed based on software architecture spec.
 */
static ethsw_ret_t calcToken(u32 Rate, u32 *pIbsIdx, u32 *pExp, u32 *pMant)
{
	static const u16 ibs_table[] = {8*8, 32*8, 64*8, 96*8};
	u8 i;

	for (i = 3; i >= 0; i--) {
		u32 exp;
		u16 ibs = ibs_table[i];
		for (exp = 0; exp < 16; exp++) {
			u32 mant =  ((ibs * 25000) >> exp) / Rate ;
			if (mant < (1 << 10))  {
				/* target is to get the biggest mantissa value that can be used for the 10-Bit register */
				*pIbsIdx = i;
				*pExp = exp;
				*pMant = mant;
				return LTQ_SUCCESS;
			}
		}
	}
}
#endif /*CONFIG_LTQ_QOS */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
static void reset_vlan_sw_table(void *pDevCtx)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u16 i;
	for (i = 0; i < VLAN_ACTIVE_TABLE_SIZE; i++)
		memset(&pEthDev->VLAN_Table[i], 0, sizeof(vlan_active_table_t));
}
#endif /*CONFIG_LTQ_VLAN*/

#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
u8 find_active_vlan_index(void *pDevCtx, u16 vid)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 i, index = 0xFF;
	for (i = 0; i < VLAN_ACTIVE_TABLE_SIZE; i++) {
		if (vid == pEthDev->VLAN_Table[i].vid) {
			index = i;
			break;
		}
	}
	return index;
}

u8 find_empty_active_vlan_index_table(void *pDevCtx)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 i, index = 0xFF;
	for (i = 1; i < VLAN_ACTIVE_TABLE_SIZE; i += 1) {
		if (pEthDev->VLAN_Table[i].valid == LTQ_FALSE) {
			index = i;
			break;
		}
	}
	if ((index == 0xFF) && (pEthDev->VLAN_Table[0].valid == LTQ_FALSE))
		return 0;
	return index;
}

static void vlan_entry_set(void *pDevCtx, u8 index,	vlan_active_table_t *pTable_Entry)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pEthDev->VLAN_Table[index].valid	= pTable_Entry->valid;
	pEthDev->VLAN_Table[index].reserved	= pTable_Entry->reserved;
	pEthDev->VLAN_Table[index].vid = pTable_Entry->vid;
	pEthDev->VLAN_Table[index].fid = pTable_Entry->fid;
	pEthDev->VLAN_Table[index].pm = pTable_Entry->pm;
	pEthDev->VLAN_Table[index].tm = pTable_Entry->tm;
}

static void get_vlan_sw_table(void *pDevCtx, u8 table_index, vlan_active_table_t *pTable_Entry)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;

	pTable_Entry->valid = pEthDev->VLAN_Table[table_index].valid;
	pTable_Entry->reserved = pEthDev->VLAN_Table[table_index].reserved;
	pTable_Entry->vid = pEthDev->VLAN_Table[table_index].vid;
	pTable_Entry->fid = pEthDev->VLAN_Table[table_index].fid;
	pTable_Entry->pm = pEthDev->VLAN_Table[table_index].pm;
	pTable_Entry->tm = pEthDev->VLAN_Table[table_index].tm;

}
#endif /* CONFIG_LTQ_VLAN */
/**
*	This is the switch core layer init function.
*	\param pInit This parameter is a pointer to the switch core context.
*	\return Return value as follows:
*	pDev: if successful */
void *ethsw_api_core_init(ethsw_core_init_t *pInit)
{
	int j;
	u32 reg_val;
	ethsw_api_dev_t *pDev;
	pDev = (ethsw_api_dev_t *)kmalloc(sizeof(ethsw_api_dev_t), GFP_KERNEL);
	if (!pDev) {
		pr_err("%s:%s:%d (memory allocation failed) \n", __FILE__, __func__, __LINE__);
		return pDev;
	}
	memset(pDev, 0, sizeof(ethsw_api_dev_t));
	pDev->pRAL_Dev = pInit->pDev;
	pCoreDev[pInit->eDev] = pDev;
	pDev->eDev = pInit->eDev;
	/* -----  end switch  ----- */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
	reset_vlan_sw_table(pDev);
#endif /* CONFIG_LTQ_VLAN */
	ltq_ethsw_port_cfg_init(pDev);
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
	reset_multicast_sw_table(pDev);
#endif /*CONFIG_LTQ_MULTICAST*/
	pce_table_init(&pDev->PCE_Handler);
	pDev->bResetCalled = LTQ_FALSE;
	pDev->bHW_InitCalled = LTQ_FALSE;
	pDev->MAC_AgeTimer = DEFAULT_AGING_TIMEOUT;
	pr_err("Switch API: PCE MicroCode loaded !!\n");
	ethsw_pce_micro_code_init(pDev);
#ifdef LTQ_ETHSW_API_COC
	pDev->pPMCtx = IFX_ETHSW_PM_powerManegementInit(pDev, IFX_ETHSW_PM_MODULENR_GSWIP);
	if (pDev->pPMCtx == NULL)
		pr_err("%s:%s:%d (IFX_ETHSW_PM_powerManegementInit failed) \n", __FILE__, __func__, __LINE__);
#endif
	/* Configure the MDIO Clock 97.6 Khz */
	gsw_w32((MDC_CFG_1_FREQ_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_1_FREQ_SHIFT, MDC_CFG_1_FREQ_SIZE, 0xFF);
	/* EEE auto negotiation overides:  clock disable (ANEG_EEE_0.CLK_STOP_CAPABLE)  */
	for (j = 0; j < pDev->nPortNumber-1; j++) {
		gsw_w32(((ANEG_EEE_0_CLK_STOP_CAPABLE_OFFSET+j) + LTQ_SOC_TOP_REG_OFFSET),	\
		ANEG_EEE_0_CLK_STOP_CAPABLE_SHIFT, ANEG_EEE_0_CLK_STOP_CAPABLE_SIZE, 0x3);
	}
	/* Set the the source MAC register of pause frame  */
	gsw_w32(MAC_PFSA_0_PFAD_OFFSET, MAC_PFSA_0_PFAD_SHIFT, \
		MAC_PFSA_0_PFAD_SIZE, 0x0000);
	gsw_w32(MAC_PFSA_1_PFAD_OFFSET, MAC_PFSA_1_PFAD_SHIFT, \
		MAC_PFSA_1_PFAD_SIZE, 0x9600);
	gsw_w32(MAC_PFSA_2_PFAD_OFFSET, MAC_PFSA_2_PFAD_SHIFT, \
		MAC_PFSA_2_PFAD_SIZE, 0xAC9A);
	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	ETHSW_CAP_1_QUEUE_SHIFT,	\
		ETHSW_CAP_1_QUEUE_SIZE, &reg_val);
	pDev->num_of_queues = reg_val;
	gsw_r32(ETHSW_CAP_3_METERS_OFFSET,	ETHSW_CAP_3_METERS_SHIFT,	\
		ETHSW_CAP_3_METERS_SIZE, &reg_val);
	pDev->num_of_meters = reg_val;
	gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET, ETHSW_CAP_3_SHAPERS_SHIFT,	\
		ETHSW_CAP_3_SHAPERS_SIZE, &reg_val);
	pDev->num_of_shapers = reg_val;
	gsw_r32(ETHSW_CAP_4_PPPOE_OFFSET, ETHSW_CAP_4_PPPOE_SHIFT,	\
		ETHSW_CAP_4_PPPOE_SIZE, &reg_val);
	pDev->num_of_pppoe = reg_val;
	gsw_r32(ETHSW_CAP_4_VLAN_OFFSET, ETHSW_CAP_4_VLAN_SHIFT,	\
		ETHSW_CAP_4_VLAN_SIZE, &reg_val);
	pDev->avlan_table_size = reg_val;
	gsw_r32(ETHSW_CAP_5_IPPLEN_OFFSET, ETHSW_CAP_5_IPPLEN_SHIFT,	\
		ETHSW_CAP_5_IPPLEN_SIZE, &reg_val);
	pDev->ip_pkt_lnt_size = reg_val;
	gsw_r32(ETHSW_CAP_5_PROT_OFFSET, ETHSW_CAP_5_PROT_SHIFT,	\
		ETHSW_CAP_5_PROT_SIZE, &reg_val);
	pDev->prot_table_size = reg_val;
	gsw_r32(ETHSW_CAP_6_MACDASA_OFFSET, ETHSW_CAP_6_MACDASA_SHIFT,	\
		ETHSW_CAP_6_MACDASA_SIZE, &reg_val);
	pDev->mac_dasa_table_size = reg_val;
	gsw_r32(ETHSW_CAP_6_APPL_OFFSET, ETHSW_CAP_6_APPL_SHIFT,	\
		ETHSW_CAP_6_APPL_SIZE, &reg_val);
	pDev->app_table_size = reg_val;
	gsw_r32(ETHSW_CAP_7_IPDASAM_OFFSET, ETHSW_CAP_7_IPDASAM_SHIFT,	\
		ETHSW_CAP_7_IPDASAM_SIZE, &reg_val);
	pDev->ip_dasa_msb_table_size = reg_val;
	gsw_r32(ETHSW_CAP_7_IPDASAL_OFFSET, ETHSW_CAP_7_IPDASAL_SHIFT,	\
		ETHSW_CAP_7_IPDASAL_SIZE, &reg_val);
	pDev->ip_dasa_lsb_table_size = reg_val;
	gsw_r32(ETHSW_CAP_8_MCAST_OFFSET, ETHSW_CAP_8_MCAST_SHIFT,	\
		ETHSW_CAP_8_MCAST_SIZE, &reg_val);
	pDev->multi_table_size = reg_val;
	gsw_r32(ETHSW_CAP_9_FLAGG_OFFSET, ETHSW_CAP_9_FLAGG_SHIFT,	\
		ETHSW_CAP_9_FLAGG_SIZE, &reg_val);
	pDev->flow_table_size = reg_val;

	gsw_r32(ETHSW_CAP_10_MACBT_OFFSET, ETHSW_CAP_10_MACBT_SHIFT,	\
		ETHSW_CAP_10_MACBT_SIZE, &reg_val);
	pDev->mac_table_size = reg_val;

	gsw_r32(ETHSW_VERSION_REV_ID_OFFSET, ETHSW_VERSION_REV_ID_SHIFT, 16, &reg_val);
	pDev->GSWIP_Ver = reg_val;
	return pDev;
}

/**
*	This is the switch core layer cleanup function.
*	\return Return value as follows:
*	LTQ_SUCCESS: if successful */
void gsw_CoreCleanUP(void)
{
	u8 i;
	ethsw_api_dev_t *pDev;
	for (i = 0; i < LTQ_FLOW_DEV_MAX; i++) {
		pDev = (ethsw_api_dev_t *)pCoreDev[i];
		if (pDev) {
#ifdef LTQ_ETHSW_API_COC
			if (pDev->pPMCtx != NULL)
				IFX_ETHSW_PM_powerManegementCleanUp(pDev->pPMCtx);
#endif
			kfree(pDev);
			pDev = NULL;
		}
	}
}

ethsw_ret_t IFX_FLOW_MAC_TableClear(void *pDevCtx)
{
	/*  flush all entries from the MAC table */
	gsw_w32(PCE_GCTRL_0_MTFL_OFFSET,	PCE_GCTRL_0_MTFL_SHIFT,	\
		PCE_GCTRL_0_MTFL_SIZE, 1);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MAC_TableEntryAdd(void *pDevCtx, IFX_ETHSW_MAC_tableAdd_t *pPar)
{
	pce_table_prog_t tbl_prog;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;

	if ((pPar->nPortId >= pEthDev->nTotalPortNumber) && (!(pPar->nPortId & 0x80000000)))
		return LTQ_ERROR;
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	tbl_prog.key[0] = pPar->nMAC[4] << 8 | pPar->nMAC[5];
	tbl_prog.key[1] = pPar->nMAC[2] << 8 | pPar->nMAC[3];
	tbl_prog.key[2] = pPar->nMAC[0] << 8 | pPar->nMAC[1];
	tbl_prog.key[3] = pPar->nFId;
	tbl_prog.valid = 1;
	if (pPar->bStaticEntry) {
		if (pPar->nPortId & 0x80000000) { /*Port Map */
			tbl_prog.val[0] = (pPar->nPortId & 0x7FFF);
		} else {
			tbl_prog.val[0] = (1 << pPar->nPortId);
		}
		tbl_prog.val[1] = 1;
	} else {
		tbl_prog.val[0] =  (((pPar->nPortId & 0xF) << 4) | (pPar->nAgeTimer & 0xF));
	}
	xwayflow_pce_table_key_write(pDevCtx, &tbl_prog);
	/* To ensure MAC is updated in the table */
	memset(&tbl_prog, 0, sizeof(pce_table_prog_t));
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	tbl_prog.key[0] = pPar->nMAC[4] << 8 | pPar->nMAC[5];
	tbl_prog.key[1] = pPar->nMAC[2] << 8 | pPar->nMAC[3];
	tbl_prog.key[2] = pPar->nMAC[0] << 8 | pPar->nMAC[1];
	tbl_prog.key[3] = pPar->nFId;
	xwayflow_pce_table_key_read(pDevCtx, &tbl_prog);
	if (tbl_prog.valid != 1)
		pr_warning("(MAC Table is full) %s:%s:%d \n", __FILE__, __func__, __LINE__);

	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MAC_TableEntryRead(void *pDevCtx, IFX_ETHSW_MAC_tableRead_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t tbl_prog;
	if (pPar->bInitial == LTQ_TRUE) {
		pEthDev->mac_table_index = 0; /*Start from the index 0 */
		pPar->bInitial = LTQ_FALSE;
	}
	if (pEthDev->mac_table_index >= pEthDev->mac_table_size) {
		memset(pPar, 0, sizeof(IFX_ETHSW_MAC_tableRead_t));
		pPar->bLast = LTQ_TRUE;
		pEthDev->mac_table_index = 0;
		return LTQ_SUCCESS;
	}
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	do {
		tbl_prog.table_index = pEthDev->mac_table_index;
		xwayflow_pce_table_read(pDevCtx, &tbl_prog);
		pEthDev->mac_table_index++;
		if (tbl_prog.valid != 0)
			break;
	} while (pEthDev->mac_table_index < pEthDev->mac_table_size);
	if (tbl_prog.valid == 1) {
		pPar->nFId = tbl_prog.key[3] & 0x3F;
		pPar->bStaticEntry = (tbl_prog.val[1] & 0x1);
		if (pPar->bStaticEntry == 1) {
			pPar->nAgeTimer = 0;
			pPar->nPortId = tbl_prog.val[0] ;
		} else {
			u32 mant, timer = 300;
			/* Aging Counter Mantissa Value */
			gsw_r32(PCE_AGE_1_MANT_OFFSET,	PCE_AGE_1_MANT_SHIFT,	\
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
			pPar->nAgeTimer	=	tbl_prog.val[0] & 0xF;
			pPar->nAgeTimer	=	(timer * pPar->nAgeTimer)/0xF;
			pPar->nPortId		=	(tbl_prog.val[0] >> 4) & 0xF;
		}
		pPar->nMAC[0] = tbl_prog.key[2] >> 8;;
		pPar->nMAC[1] = tbl_prog.key[2] & 0xFF;
		pPar->nMAC[2] = tbl_prog.key[1] >> 8;
		pPar->nMAC[3] = tbl_prog.key[1] & 0xFF;
		pPar->nMAC[4] = tbl_prog.key[0] >> 8;
		pPar->nMAC[5] = tbl_prog.key[0] & 0xFF;
		pPar->bInitial = LTQ_FALSE;
		pPar->bLast = LTQ_FALSE;
	} else {
		memset(pPar, 0, sizeof(IFX_ETHSW_MAC_tableRead_t));
		pPar->bLast = LTQ_TRUE;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MAC_TableEntryQuery(void *pDevCtx, IFX_ETHSW_MAC_tableQuery_t *pPar)
{
	pce_table_prog_t tbl_prog;
	pPar->bFound = LTQ_FALSE;
	memset(&tbl_prog, 0, sizeof(pce_table_prog_t));
	tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
	tbl_prog.key[0] = pPar->nMAC[4] << 8 | pPar->nMAC[5];
	tbl_prog.key[1] = pPar->nMAC[2] << 8 | pPar->nMAC[3];
	tbl_prog.key[2] = pPar->nMAC[0] << 8 | pPar->nMAC[1];
	tbl_prog.key[3] = pPar->nFId;
	xwayflow_pce_table_key_read(pDevCtx, &tbl_prog);
	if (tbl_prog.valid == 1) {
		pPar->bFound = LTQ_TRUE;
		pPar->bStaticEntry = (tbl_prog.val[1] & 0x1);
		if ((tbl_prog.val[1] & 0x1) == 1) {
			pPar->nAgeTimer = 0;
			pPar->nPortId = (tbl_prog.val[0]);
		} else {
			u32 mant, timer = 300;
			/* Aging Counter Mantissa Value */
			gsw_r32(PCE_AGE_1_MANT_OFFSET, PCE_AGE_1_MANT_SHIFT,	\
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
			pPar->nAgeTimer = tbl_prog.val[0] & 0xF;
			pPar->nAgeTimer = (timer * pPar->nAgeTimer)/0xF;
			pPar->nPortId = (tbl_prog.val[0] >> 4) & 0xF;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MAC_TableEntryRemove(void *pDevCtx, IFX_ETHSW_MAC_tableRemove_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t tbl_prog;
	u32 index, value ;
	/* Learning Limit Port Lock */
	gsw_r32(PCE_GCTRL_0_MTFL_OFFSET, PCE_GCTRL_0_MTFL_SHIFT,	\
		PCE_GCTRL_0_MTFL_SIZE, &value);
	if (!value) { /*if value is 1 means, flush all entries from the MAC table */
		for (index = 0; index < pEthDev->mac_table_size; index++) {
			memset(&tbl_prog, 0 , sizeof(pce_table_prog_t));
			tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
			tbl_prog.table_index = index;
			xwayflow_pce_table_read(pDevCtx, &tbl_prog);
			if ((pPar->nMAC[0] == (tbl_prog.key[2] >> 8)) && (pPar->nMAC[1] == (tbl_prog.key[2] & 0xFF))	\
				&& (pPar->nMAC[2] == (tbl_prog.key[1] >> 8)) && (pPar->nMAC[3] == (tbl_prog.key[1] & 0xFF))	\
				&& (pPar->nMAC[4] == (tbl_prog.key[0] >> 8)) && (pPar->nMAC[5] == (tbl_prog.key[0] & 0xFF))	\
				&& (pPar->nFId == (tbl_prog.key[3] & 0x3F))) {
				memset(&tbl_prog, 0 , sizeof(pce_table_prog_t));
				tbl_prog.table = PCE_MAC_BRIDGE_INDEX;
				tbl_prog.table_index = index;
				xwayflow_pce_table_write(pDevCtx, &tbl_prog);
			}
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortCfgGet(void *pDevCtx, IFX_ETHSW_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value, Monitor_rx = 0, Monitor_tx = 0, PEN, EN;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	/* See if PORT enable or not */
	gsw_r32((SDMA_PCTRL_PEN_OFFSET + (0x6 * portIdx)),	\
		SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, &PEN);
	gsw_r32((FDMA_PCTRL_EN_OFFSET + (0x6 * portIdx)),	\
		FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, &EN);
	/* Port Enable feature only support 6 port */
	 if (portIdx >= pEthDev->nPortNumber) {
		pPar->eEnable = LTQ_ENABLE;
	} else {
		if ((PEN == 1) && (EN == 1))
			pPar->eEnable = IFX_ETHSW_PORT_ENABLE_RXTX;
		else if ((PEN == 1) && (EN == 0))
			pPar->eEnable = IFX_ETHSW_PORT_ENABLE_RX;
		else if ((PEN == 0) && (EN == 1))
			pPar->eEnable = IFX_ETHSW_PORT_ENABLE_TX;
		else
			pPar->eEnable = IFX_ETHSW_PORT_DISABLE;
	}
	/* Learning Limit */
	gsw_r32((PCE_PCTRL_1_LRNLIM_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_1_LRNLIM_SHIFT, PCE_PCTRL_1_LRNLIM_SIZE, &value);
	pPar->nLearningLimit = value;

	/* Learning Limit Port Lock */
	gsw_r32((PCE_PCTRL_0_PLOCK_OFFSET + (0xA * portIdx)) ,	\
		PCE_PCTRL_0_PLOCK_SHIFT, PCE_PCTRL_0_PLOCK_SIZE, &value);
	pPar->bLearningMAC_PortLock = value;
	/* Aging */
	gsw_r32(PCE_PCTRL_0_AGEDIS_OFFSET + (0xA * portIdx),	\
		PCE_PCTRL_0_AGEDIS_SHIFT, PCE_PCTRL_0_AGEDIS_SIZE, &value);
	pPar->bAging = value;

	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** MAC address table learning on the port specified. */
		gsw_r32((PCE_PCTRL_3_LNDIS_OFFSET + (0xA * portIdx)),	\
			PCE_PCTRL_3_LNDIS_SHIFT, PCE_PCTRL_3_LNDIS_SIZE, &pPar->bLearning);
		/** MAC spoofing detection. */
		gsw_r32((PCE_PCTRL_0_SPFDIS_OFFSET + (0xA * portIdx)), \
			PCE_PCTRL_0_SPFDIS_SHIFT, PCE_PCTRL_0_SPFDIS_SIZE, &pPar->bMAC_SpoofingDetection);
	}
	/* UnicastUnknownDrop */
	gsw_r32(PCE_PMAP_3_UUCMAP_OFFSET,	\
		PCE_PMAP_3_UUCMAP_SHIFT, PCE_PMAP_3_UUCMAP_SIZE, &value);
	/* UnicastUnknownDrop feature  support  */
	if ((value & (1 << portIdx)) == 0)
		pPar->bUnicastUnknownDrop = LTQ_ENABLE;
	else
		pPar->bUnicastUnknownDrop = 0;
	/* MulticastUnknownDrop */
	gsw_r32(PCE_PMAP_2_DMCPMAP_OFFSET,	\
		PCE_PMAP_2_DMCPMAP_SHIFT, PCE_PMAP_2_DMCPMAP_SIZE, &value);
	/* MulticastUnknownDrop feature  support  */
	if ((value & (1 << portIdx)) == 0) {
		pPar->bMulticastUnknownDrop = 1;
		pPar->bBroadcastDrop = 1;
	} else {
		pPar->bMulticastUnknownDrop = 0;
		pPar->bBroadcastDrop = 0;
	}
	/* Require to check later - 3M */
	pPar->bReservedPacketDrop = 0;
	/* Port Monitor */
	gsw_r32((PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_3_RXVMIR_SHIFT, PCE_PCTRL_3_RXVMIR_SIZE, &Monitor_rx);
	gsw_r32((PCE_PCTRL_3_TXMIR_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_3_TXMIR_SHIFT, PCE_PCTRL_3_TXMIR_SIZE, &Monitor_tx);
	if ((Monitor_rx == 1) && (Monitor_tx == 1))
		pPar->ePortMonitor = IFX_ETHSW_PORT_MONITOR_RXTX;
	else if ((Monitor_rx == 1) && (Monitor_tx == 0))
		pPar->ePortMonitor = IFX_ETHSW_PORT_MONITOR_RX;
	else if ((Monitor_rx == 0) && (Monitor_tx == 1))
		pPar->ePortMonitor = IFX_ETHSW_PORT_MONITOR_TX;
	else
		pPar->ePortMonitor = IFX_ETHSW_PORT_MONITOR_NONE;
	gsw_r32((MAC_PSTAT_TXPAUEN_OFFSET + (0xC * portIdx)),	\
		MAC_PSTAT_TXPAUEN_SHIFT, 2, &value);
	pPar->eFlowCtrl = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortCfgSet(void *pDevCtx, IFX_ETHSW_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value, EN, PEN, PACT, Monitor_rx, Monitor_tx;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	/* Learning Limit Port Lock */
	gsw_w32((PCE_PCTRL_0_PLOCK_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_0_PLOCK_SHIFT, PCE_PCTRL_0_PLOCK_SIZE, pPar->bLearningMAC_PortLock);
	/* Learning Limit Action */
	if (pPar->nLearningLimit == 0)
		value = 0;
	else if (pPar->nLearningLimit == 0xFFFF)
		value = 0xFF;
	else
		value = pPar->nLearningLimit;
	pEthDev->PortConfig[pPar->nPortId].nLearningLimit = value;
	/* Learning Limit */
	gsw_w32((PCE_PCTRL_1_LRNLIM_OFFSET + (0xA * portIdx)), \
		PCE_PCTRL_1_LRNLIM_SHIFT, PCE_PCTRL_1_LRNLIM_SIZE, value);
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** MAC address table learning on the port specified */
		gsw_w32((PCE_PCTRL_3_LNDIS_OFFSET + (0xA * portIdx)), \
			PCE_PCTRL_3_LNDIS_SHIFT, PCE_PCTRL_3_LNDIS_SIZE, pPar->bLearning);
		/** MAC spoofing detection. */
		gsw_w32((PCE_PCTRL_0_SPFDIS_OFFSET + (0xA * portIdx)),\
			PCE_PCTRL_0_SPFDIS_SHIFT, PCE_PCTRL_0_SPFDIS_SIZE, pPar->bMAC_SpoofingDetection);
	}
	/* Aging */
	gsw_w32(PCE_PCTRL_0_AGEDIS_OFFSET + (0xA * portIdx),	\
		PCE_PCTRL_0_AGEDIS_SHIFT, PCE_PCTRL_0_AGEDIS_SIZE, pPar->bAging);
	/* UnicastUnknownDrop Read first */
	gsw_r32(PCE_PMAP_3_UUCMAP_OFFSET,	\
		PCE_PMAP_3_UUCMAP_SHIFT, PCE_PMAP_3_UUCMAP_SIZE, &value);
	if (pPar->bUnicastUnknownDrop == 1)
		value &= ~(1 << portIdx);
	else
		value |= 1 << portIdx;
	/* UnicastUnknownDrop write back */
	gsw_w32(PCE_PMAP_3_UUCMAP_OFFSET,	\
		PCE_PMAP_3_UUCMAP_SHIFT, PCE_PMAP_3_UUCMAP_SIZE, value);
	/* MulticastUnknownDrop */
	gsw_r32(PCE_PMAP_2_DMCPMAP_OFFSET,	\
		PCE_PMAP_2_DMCPMAP_SHIFT, PCE_PMAP_2_DMCPMAP_SIZE, &value);
	if (pPar->bMulticastUnknownDrop == 1)
		value &= ~(1 << portIdx);
	else
		value |= 1 << portIdx;
	/* MulticastUnknownDrop */
	gsw_w32(PCE_PMAP_2_DMCPMAP_OFFSET,	\
		PCE_PMAP_2_DMCPMAP_SHIFT, PCE_PMAP_2_DMCPMAP_SIZE, value);
	/* Flow Control */
	if (portIdx < pEthDev->nPortNumber) {
		gsw_r32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			(MDC_CFG_0_PEN_0_SHIFT + portIdx), MDC_CFG_0_PEN_0_SIZE, &PEN);
		gsw_r32((MDIO_STAT_0_PACT_OFFSET + LTQ_SOC_TOP_REG_OFFSET + portIdx),	\
			MDIO_STAT_0_PACT_SHIFT, MDIO_STAT_0_PACT_SIZE, &PACT);

		if ((PEN == 1) && (PACT == 1)) {
			/* PHY polling statemachine (of the MAC) is activated and
				an external PHY reacts on the MDIO accesses.
			   Therefore update the MDIO register of the attached PHY. */
			IFX_ETHSW_MDIO_data_t mdio_data;
			/* Write directly to MDIO register */
			gsw_r32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
				PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &value);
			mdio_data.nAddressDev = value;
			mdio_data.nAddressReg = 0x4;
			IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
			mdio_data.nData &= ~(0xC00);
			switch (pPar->eFlowCtrl) {
			case IFX_ETHSW_FLOW_OFF:
					break;
			case IFX_ETHSW_FLOW_TX:
					mdio_data.nData |= 0x800;
					break;
			case IFX_ETHSW_FLOW_RXTX:
					mdio_data.nData |= 0x400;
					break;
			case IFX_ETHSW_FLOW_RX:
			case IFX_ETHSW_FLOW_AUTO:
					mdio_data.nData |= 0xC00;
					break;
			}
			IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
			/* Restart Auto negotiation */
			mdio_data.nAddressReg = 0x0;
			IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
			mdio_data.nData |= 0x1200;
			IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
		} else {
			/* Either PHY polling statemachine (of the MAC) is disable,
			   or the statemachine did not find any attached PHY. */
			u32 RX = 0, TX = 0;
			switch (pPar->eFlowCtrl) {
			case IFX_ETHSW_FLOW_AUTO:
			case IFX_ETHSW_FLOW_OFF:
					RX = 0; TX = 0;
					break;
			case IFX_ETHSW_FLOW_RXTX:
					RX = 1; TX = 1;
					break;
			case IFX_ETHSW_FLOW_RX:
					RX = 1; TX = 0;
					break;
			case IFX_ETHSW_FLOW_TX:
					RX = 0; TX = 1;
					break;
			}
			gsw_w32((MAC_CTRL_0_FCON_OFFSET + (0xC * portIdx)),	\
				MAC_CTRL_0_FCON_SHIFT, MAC_CTRL_0_FCON_SIZE, pPar->eFlowCtrl);
			gsw_w32((PHY_ADDR_0_FCONTX_OFFSET - (0x1 * portIdx)),	\
				PHY_ADDR_0_FCONTX_SHIFT, PHY_ADDR_0_FCONTX_SIZE , TX);
			gsw_w32((PHY_ADDR_0_FCONRX_OFFSET - (0x1 * portIdx)),	\
				PHY_ADDR_0_FCONRX_SHIFT, PHY_ADDR_0_FCONRX_SIZE, RX);
		}
	}
	/* Port Monitor */
	if (pPar->ePortMonitor == IFX_ETHSW_PORT_MONITOR_RXTX) {
		Monitor_rx = 1;
		Monitor_tx = 1;
	} else if (pPar->ePortMonitor == IFX_ETHSW_PORT_MONITOR_RX) {
		Monitor_rx = 1;
		Monitor_tx = 0;
	} else if (pPar->ePortMonitor == IFX_ETHSW_PORT_MONITOR_TX) {
		Monitor_rx = 0;
		Monitor_tx = 1;
	} else {
		Monitor_rx = 0;
		Monitor_tx = 0;
	}
	gsw_w32((PCE_PCTRL_3_RXVMIR_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_3_RXVMIR_SHIFT, PCE_PCTRL_3_RXVMIR_SIZE, Monitor_rx);
	gsw_w32((PCE_PCTRL_3_TXMIR_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_3_TXMIR_SHIFT, PCE_PCTRL_3_TXMIR_SIZE, Monitor_tx);
	if (pPar->eEnable == IFX_ETHSW_PORT_ENABLE_RXTX) {
		PEN = 1;
		EN = 1;
	} else if (pPar->eEnable == IFX_ETHSW_PORT_ENABLE_RX) {
		PEN = 1;
		EN = 0;
	} else if (pPar->eEnable == IFX_ETHSW_PORT_ENABLE_TX) {
		PEN = 0;
		EN = 1;
	} else {
		PEN = 0;
		EN = 0;
	}
	/* Set SDMA_PCTRL_PEN PORT enable */
	gsw_w32((SDMA_PCTRL_PEN_OFFSET + (6 * portIdx)),	\
		SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, PEN);
	/* Set FDMA_PCTRL_EN PORT enable  */
	gsw_w32((FDMA_PCTRL_EN_OFFSET + (0x6 * portIdx)),	\
		FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, EN);
	return LTQ_SUCCESS;
}
#if defined(CONFIG_LTQ_STP) && CONFIG_LTQ_STP
ethsw_ret_t IFX_FLOW_STP_BPDU_RuleGet(void *pDevCtx, IFX_ETHSW_STP_BPDU_Rule_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	ltq_stp_8021x_t *pStateHandle = &pEthDev->STP_8021x_Config;
	pPar->eForwardPort = pStateHandle->eSTPPortState;
	pPar->nForwardPortId = pStateHandle->nSTP_PortID;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_STP_BPDU_RuleSet(void *pDevCtx, IFX_ETHSW_STP_BPDU_Rule_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	ltq_stp_8021x_t *pStateHandle	= &pEthDev->STP_8021x_Config;
	IFX_FLOW_PCE_rule_t PCE_rule;

	pStateHandle->eSTPPortState = pPar->eForwardPort;
	pStateHandle->nSTP_PortID   = pPar->nForwardPortId;
	memset(&PCE_rule, 0, sizeof(IFX_FLOW_PCE_rule_t));
	/* Attached the PCE rule for BPDU packet */
	PCE_rule.pattern.nIndex		= BPDU_PCE_RULE_INDEX;
	PCE_rule.pattern.bEnable	= LTQ_TRUE;
	PCE_rule.pattern.bMAC_DstEnable	= LTQ_TRUE;
	PCE_rule.pattern.nMAC_Dst[0]	= 0x01;
	PCE_rule.pattern.nMAC_Dst[1]	= 0x80;
	PCE_rule.pattern.nMAC_Dst[2]	= 0xC2;
	PCE_rule.pattern.nMAC_Dst[3]	= 0x00;
	PCE_rule.pattern.nMAC_Dst[4]	= 0x00;
	PCE_rule.pattern.nMAC_Dst[5]	= 0x00;
	PCE_rule.action.eCrossStateAction	= IFX_FLOW_PCE_ACTION_CROSS_STATE_CROSS;
	if ((pStateHandle->eSTPPortState < 4) && (pStateHandle->eSTPPortState > 0))
		PCE_rule.action.ePortMapAction = pStateHandle->eSTPPortState + 1;
	else
		pr_warning("(Incorrect forward port action) %s:%s:%d \n", __FILE__, __func__, __LINE__);
	PCE_rule.action.nForwardPortMap = (1 << pStateHandle->nSTP_PortID);
	/* We prepare everything and write into PCE Table */
	if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
		return LTQ_ERROR;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_STP_PortCfgGet(void *pDevCtx, IFX_ETHSW_STP_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	pPar->ePortState = pEthDev->PortConfig[pPar->nPortId].ifx_stp_state;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_STP_PortCfgSet(void *pDevCtx, IFX_ETHSW_STP_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	pEthDev->PortConfig[pPar->nPortId].ifx_stp_state = pPar->ePortState;
	/* Config the Table */
	set_port_state(pDevCtx, pPar->nPortId, pEthDev->PortConfig[pPar->nPortId].ifx_stp_state,	\
		pEthDev->PortConfig[pPar->nPortId].ifx_8021x_state);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_TrunkingCfgGet(void *pDevCtx, IFX_ETHSW_trunkingCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/* Destination IP Mask */
		gsw_r32(PCE_TRUNK_CONF_DIP_OFFSET,	\
			PCE_TRUNK_CONF_DIP_SHIFT, PCE_TRUNK_CONF_DIP_SIZE, &pPar->bIP_Dst);
		/* 'Source IP Mask */
		gsw_r32(PCE_TRUNK_CONF_SIP_OFFSET,	\
			PCE_TRUNK_CONF_SIP_SHIFT, PCE_TRUNK_CONF_SIP_SIZE, &pPar->bIP_Src);
		/* Destination MAC Mask */
		gsw_r32(PCE_TRUNK_CONF_DA_OFFSET,	\
			PCE_TRUNK_CONF_DA_SHIFT, PCE_TRUNK_CONF_DA_SIZE, &pPar->bMAC_Dst);
		/* 'Source MAC Mask */
		gsw_r32(PCE_TRUNK_CONF_SA_OFFSET,	\
			PCE_TRUNK_CONF_SA_SHIFT, PCE_TRUNK_CONF_SA_SIZE, &pPar->bMAC_Src);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_TrunkingCfgSet(void *pDevCtx, IFX_ETHSW_trunkingCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/* Destination IP Mask */
		if (pPar->bIP_Dst == 1) {
			gsw_w32(PCE_TRUNK_CONF_DIP_OFFSET,	\
				PCE_TRUNK_CONF_DIP_SHIFT, PCE_TRUNK_CONF_DIP_SIZE, 1);
		} else {
			gsw_w32(PCE_TRUNK_CONF_DIP_OFFSET,	\
				PCE_TRUNK_CONF_DIP_SHIFT, PCE_TRUNK_CONF_DIP_SIZE, 0);
		}
		/* 'Source IP Mask */
		if (pPar->bIP_Src == 1) {
			gsw_w32(PCE_TRUNK_CONF_SIP_OFFSET,	\
				PCE_TRUNK_CONF_SIP_SHIFT, PCE_TRUNK_CONF_SIP_SIZE, 1);
		} else {
			gsw_w32(PCE_TRUNK_CONF_SIP_OFFSET,	\
				PCE_TRUNK_CONF_SIP_SHIFT, PCE_TRUNK_CONF_SIP_SIZE, 0);
		}
		/* Destination MAC Mask */
		if (pPar->bMAC_Dst == 1) {
			gsw_w32(PCE_TRUNK_CONF_DA_OFFSET,	\
				PCE_TRUNK_CONF_DA_SHIFT, PCE_TRUNK_CONF_DA_SIZE, 1);
		} else {
			gsw_w32(PCE_TRUNK_CONF_DA_OFFSET,	\
				PCE_TRUNK_CONF_DA_SHIFT, PCE_TRUNK_CONF_DA_SIZE, 0);
		}
		/* 'Source MAC Mask */
		if (pPar->bMAC_Src == 1) {
			gsw_w32(PCE_TRUNK_CONF_SA_OFFSET,	\
				PCE_TRUNK_CONF_SA_SHIFT, PCE_TRUNK_CONF_SA_SIZE, 1);
		} else {
			gsw_w32(PCE_TRUNK_CONF_SA_OFFSET,	\
				PCE_TRUNK_CONF_SA_SHIFT, PCE_TRUNK_CONF_SA_SIZE, 0);
		}
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_TrunkingPortCfgGet(void *pDevCtx, IFX_ETHSW_trunkingPortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;

	if (pPar->nPortId >= (pEthDev->nTotalPortNumber)) {
		return LTQ_ERROR;
	}
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** Ports are aggregated. the 'nPortId' and the 'nAggrPortId' ports form an aggregated link. */
		gsw_r32((PCE_PTRUNK_EN_OFFSET + (pPar->nPortId * 0x2)),	\
			PCE_PTRUNK_EN_SHIFT, PCE_PTRUNK_EN_SIZE, &value);
		 pPar->bAggregateEnable = value;
		gsw_r32((PCE_PTRUNK_PARTER_OFFSET + (pPar->nPortId * 0x2)),	\
			PCE_PTRUNK_PARTER_SHIFT, PCE_PTRUNK_PARTER_SIZE, &value);
		pPar->nAggrPortId = value;
		return LTQ_SUCCESS;
	} else {
		pr_warning("(version:0x%08x) %s:%s:%d \n", pEthDev->GSWIP_Ver, __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_TrunkingPortCfgSet(void *pDevCtx, IFX_ETHSW_trunkingPortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= (pEthDev->nTotalPortNumber))
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		 /** Ports are aggregated. the 'nPortId' and the 'nAggrPortId' ports form an aggregated link. */
		if (pPar->bAggregateEnable == 1) {
			gsw_w32((PCE_PTRUNK_EN_OFFSET + (pPar->nPortId * 0x2)),	\
				PCE_PTRUNK_EN_SHIFT, PCE_PTRUNK_EN_SIZE, 1);
			gsw_w32((PCE_PTRUNK_PARTER_OFFSET + (pPar->nPortId * 0x2)),	\
				PCE_PTRUNK_PARTER_SHIFT, PCE_PTRUNK_PARTER_SIZE, (pPar->nAggrPortId & 0xF));
		}
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_TimestampTimerSet(void *pDevCtx, IFX_FLOW_TIMESTAMP_Timer_t *pPar)
{
	u32 value;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** Second. Absolute second timer count. */
		gsw_w32((TIMER_SEC_LSB_SECLSB_OFFSET),	\
			TIMER_SEC_LSB_SECLSB_SHIFT, TIMER_SEC_LSB_SECLSB_SIZE, (pPar->nSec & 0xFFFF));
		gsw_w32((TIMER_SEC_MSB_SECMSB_OFFSET),	\
			TIMER_SEC_MSB_SECMSB_SHIFT, TIMER_SEC_MSB_SECMSB_SIZE, ((pPar->nSec >> 16) & 0xFFFF));
		/** Nano Second. Absolute nano second timer count. */
		gsw_w32((TIMER_NS_LSB_NSLSB_OFFSET),	\
			TIMER_NS_LSB_NSLSB_SHIFT, TIMER_NS_LSB_NSLSB_SIZE, (pPar->nNanoSec & 0xFFFF));
		gsw_w32((TIMER_NS_MSB_NSMSB_OFFSET),	\
			TIMER_NS_MSB_NSMSB_SHIFT, TIMER_NS_MSB_NSMSB_SIZE, ((pPar->nNanoSec >> 16) & 0xFFFF));
		/** Fractional Nano Second. Absolute fractional nano second timer count.
       This counter specifis a 2^32 fractional 'nNanoSec'. */
		gsw_w32((TIMER_FS_LSB_FSLSB_OFFSET),	\
			TIMER_FS_LSB_FSLSB_SHIFT, TIMER_FS_LSB_FSLSB_SIZE, (pPar->nFractionalNanoSec & 0xFFFF));
		gsw_w32((TIMER_FS_MSB_FSMSB_OFFSET),	\
			TIMER_FS_MSB_FSMSB_SHIFT, TIMER_FS_MSB_FSMSB_SIZE, ((pPar->nFractionalNanoSec >> 16) & 0xFFFF));
		value = 1;
		gsw_w32((TIMER_CTRL_WR_OFFSET),	\
			TIMER_CTRL_WR_SHIFT, TIMER_CTRL_WR_SIZE, value);
		do {
			gsw_r32((TIMER_CTRL_WR_OFFSET),	\
				TIMER_CTRL_WR_SHIFT, TIMER_CTRL_WR_SIZE, &value);
		} while (value == 1);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}
ethsw_ret_t IFX_FLOW_TimestampTimerGet(void *pDevCtx, IFX_FLOW_TIMESTAMP_Timer_t *pPar)
{
	u32 value;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	value = 1;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		gsw_w32((TIMER_CTRL_RD_OFFSET),	\
			TIMER_CTRL_RD_SHIFT, TIMER_CTRL_RD_SIZE, value);
		do {
			gsw_r32((TIMER_CTRL_RD_OFFSET),	\
				TIMER_CTRL_RD_SHIFT, TIMER_CTRL_RD_SIZE, &value);
		} while (value == 1);
		/** Second. Absolute second timer count. */
		gsw_r32((TIMER_SEC_LSB_SECLSB_OFFSET),	\
			TIMER_SEC_LSB_SECLSB_SHIFT, TIMER_SEC_LSB_SECLSB_SIZE, &value);
		pPar->nSec = value & 0xFFFF;
		gsw_r32((TIMER_SEC_MSB_SECMSB_OFFSET),	\
			TIMER_SEC_MSB_SECMSB_SHIFT, TIMER_SEC_MSB_SECMSB_SIZE, &value);
		pPar->nSec |= (value & 0xFFFF << 16);
		/** Nano Second. Absolute nano second timer count. */
		gsw_r32((TIMER_NS_LSB_NSLSB_OFFSET),	\
			TIMER_NS_LSB_NSLSB_SHIFT, TIMER_NS_LSB_NSLSB_SIZE, &value);
		pPar->nNanoSec = value & 0xFFFF;
		gsw_r32((TIMER_NS_MSB_NSMSB_OFFSET),	\
			TIMER_NS_MSB_NSMSB_SHIFT, TIMER_NS_MSB_NSMSB_SIZE, &value);
		pPar->nNanoSec |= (value & 0xFFFF << 16);
		/** Fractional Nano Second. Absolute fractional nano second timer count.
		This counter specifis a 2^32 fractional 'nNanoSec'. */
		gsw_r32((TIMER_FS_LSB_FSLSB_OFFSET),	\
			TIMER_FS_LSB_FSLSB_SHIFT, TIMER_FS_LSB_FSLSB_SIZE, &value);
		pPar->nFractionalNanoSec = value & 0xFFFF;
		gsw_r32((TIMER_FS_MSB_FSMSB_OFFSET),	\
			TIMER_FS_MSB_FSMSB_SHIFT, TIMER_FS_MSB_FSMSB_SIZE, &value);
		pPar->nFractionalNanoSec |= (value & 0xFFFF << 16);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}
ethsw_ret_t IFX_FLOW_TimestampPortRead(void *pDevCtx, IFX_FLOW_TIMESTAMP_PortRead_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 tstamp0, tstamp1;
	if (pPar->nPortId >= (pEthDev->nPortNumber))
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** Second. Absolute second timer count. */
		gsw_r32((FDMA_TSTAMP0_TSTL_OFFSET + (pPar->nPortId * 0x6)),	\
			FDMA_TSTAMP0_TSTL_SHIFT, FDMA_TSTAMP0_TSTL_SIZE, &tstamp0);
		gsw_r32((FDMA_TSTAMP1_TSTH_OFFSET + (pPar->nPortId * 0x6)),	\
			FDMA_TSTAMP1_TSTH_SHIFT, FDMA_TSTAMP1_TSTH_SIZE, &tstamp1);
		pPar->nEgressSec =  ((tstamp0 |  (tstamp1  << 16))) >> 30;
		pPar->nEgressNanoSec =  ((tstamp0 |  (tstamp1  << 16))) & 0x7FFFFFFF;
		/** Nano Second. Absolute nano second timer count. */
		gsw_r32((SDMA_TSTAMP0_TSTL_OFFSET + (pPar->nPortId * 0x6)),	\
			SDMA_TSTAMP0_TSTL_SHIFT, SDMA_TSTAMP0_TSTL_SIZE, &tstamp0);
		gsw_r32((SDMA_TSTAMP1_TSTH_OFFSET + (pPar->nPortId * 0x6)),	\
			SDMA_TSTAMP1_TSTH_SHIFT, SDMA_TSTAMP1_TSTH_SIZE, &tstamp1);
		pPar->nIngressSec =  ((tstamp0 |  (tstamp1  << 16))) >> 30;
		pPar->nIngressNanoSec =  ((tstamp0 |  (tstamp1  << 16))) & 0x7FFFFFFF;
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

#endif /* CONFIG_LTQ_STP */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
ethsw_ret_t IFX_FLOW_VLAN_IdCreate(void *pDevCtx, IFX_ETHSW_VLAN_IdCreate_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 table_index;
	vlan_active_table_t VLAN_Table_Entry;
	pce_table_prog_t pcetable;

	if (find_active_vlan_index(pEthDev, pPar->nVId) != 0xFF) {
		pr_err("This vid exists\n");
		return LTQ_ERROR;
	}
	table_index = find_empty_active_vlan_index_table(pEthDev);
	if (table_index == 0xFF) {
		pr_err("There is no table entry avariable\n");
		return LTQ_ERROR;
	}
	memset(&VLAN_Table_Entry, 0, sizeof(vlan_active_table_t));
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	VLAN_Table_Entry.valid = LTQ_TRUE;
	VLAN_Table_Entry.vid = pPar->nVId;
	VLAN_Table_Entry.fid = pPar->nFId;
	vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	pcetable.table_index = table_index;
	pcetable.table = PCE_ACTVLAN_INDEX;
	pcetable.key[0] = pPar->nVId;
	pcetable.val[0] = pPar->nFId;
	pcetable.valid = LTQ_TRUE;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	pcetable.table = PCE_VLANMAP_INDEX;
	pcetable.val[0] = pPar->nVId;
	pcetable.val[1] = 0;
	pcetable.val[2] = 0;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_IdDelete(void *pDevCtx, IFX_ETHSW_VLAN_IdDelete_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u8 table_index;
	vlan_active_table_t VLAN_Table_Entry;
	ltq_pce_table_t *pVlanTmHandle = &pEthDev->PCE_Handler;

	memset(&VLAN_Table_Entry, 0, sizeof(vlan_active_table_t));
	table_index = find_active_vlan_index(pEthDev, pPar->nVId);
	if (table_index == 0xFF) {
		pr_err("(VID not exists) %s:%s:%d\n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if (get_pce_tm_vlan_act_tbl_index(&pVlanTmHandle->pce_sub_tbl, table_index) != LTQ_SUCCESS) {
		pr_err("(VID: 0x%0x used by flow table) %s:%s:%d \n", pPar->nVId, __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	pcetable.table_index = table_index;
	pcetable.table = PCE_ACTVLAN_INDEX;
	pcetable.valid = LTQ_FALSE;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	pcetable.table = PCE_VLANMAP_INDEX;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_IdGet(void *pDevCtx, IFX_ETHSW_VLAN_IdGet_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 table_index;
	vlan_active_table_t VLAN_Table_Entry;
	table_index = find_active_vlan_index(pEthDev, pPar->nVId);
	if (table_index == 0xFF) {
		pr_err("(VID not exists) %s:%s:%d \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	get_vlan_sw_table(pEthDev, table_index, &VLAN_Table_Entry);
	pPar->nFId = VLAN_Table_Entry.fid;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_PortCfgGet(void *pDevCtx, IFX_ETHSW_VLAN_portCfg_t *pPar)
{
	u32 value;
	int table_index;
	ltq_bool_t bUVR, bVIMR, bVEMR;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	vlan_active_table_t VLAN_Table_Entry;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	gsw_r32(PCE_DEFPVID_PVID_OFFSET + (10 * pPar->nPortId),	\
		PCE_DEFPVID_PVID_SHIFT, PCE_DEFPVID_PVID_SIZE, &value);
	table_index = value;
	get_vlan_sw_table(pEthDev, table_index, &VLAN_Table_Entry);
	pPar->nPortVId = VLAN_Table_Entry.vid;
	gsw_r32(PCE_VCTRL_UVR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_UVR_SHIFT, PCE_VCTRL_UVR_SIZE, &value);
	bUVR = value;
	if (bUVR == 1)
		pPar->bVLAN_UnknownDrop = LTQ_TRUE;
	else
		pPar->bVLAN_UnknownDrop = LTQ_FALSE;
	gsw_r32(PCE_VCTRL_VSR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_VSR_SHIFT, PCE_VCTRL_VSR_SIZE, &value);
	pPar->bVLAN_ReAssign = value;
	gsw_r32(PCE_VCTRL_VIMR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_VIMR_SHIFT, PCE_VCTRL_VIMR_SIZE, &value);
	bVIMR = value;

	gsw_r32(PCE_VCTRL_VEMR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_VEMR_SHIFT, PCE_VCTRL_VEMR_SIZE, &value);
	bVEMR = value;
	if (bVIMR == LTQ_FALSE && bVEMR == LTQ_FALSE)
		pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_NO;
	else if (bVIMR == LTQ_TRUE && bVEMR == LTQ_FALSE)
		pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_INGRESS;
	else if (bVIMR == LTQ_FALSE && bVEMR == LTQ_TRUE)
		pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_EGRESS;
	else if (bVIMR == LTQ_TRUE && bVEMR == LTQ_TRUE)
		pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_BOTH;

	gsw_r32(PCE_VCTRL_VINR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_VINR_SHIFT, PCE_VCTRL_VINR_SIZE, &value);
	switch (value) {
	case 0:
			pPar->eAdmitMode = IFX_ETHSW_VLAN_ADMIT_ALL;
			break;
	case 1:
			pPar->eAdmitMode = IFX_ETHSW_VLAN_ADMIT_TAGGED;
			break;
	case 2:
			pPar->eAdmitMode = IFX_ETHSW_VLAN_ADMIT_UNTAGGED;
			break;
	default:
			break;
	}
	gsw_r32(PCE_PCTRL_0_TVM_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_TVM_SHIFT, PCE_PCTRL_0_TVM_SIZE, &value);
	pPar->bTVM = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_PortCfgSet(void *pDevCtx, IFX_ETHSW_VLAN_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	u8 table_index;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	table_index = find_active_vlan_index(pEthDev, pPar->nPortVId);
	if (table_index == 0xFF) {
		pr_err("(VID not exists) %s:%s:%d \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	value = table_index;
	gsw_w32(PCE_DEFPVID_PVID_OFFSET + (10 * pPar->nPortId),	\
		PCE_DEFPVID_PVID_SHIFT, PCE_DEFPVID_PVID_SIZE, value);
	value = 0;
	if (pPar->bVLAN_UnknownDrop == LTQ_TRUE)
		value = 1;

	gsw_w32(PCE_VCTRL_UVR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_UVR_SHIFT, PCE_VCTRL_UVR_SIZE, value);
	value = pPar->bVLAN_ReAssign;
	gsw_w32(PCE_VCTRL_VSR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_VSR_SHIFT, PCE_VCTRL_VSR_SIZE, value);
	switch (pPar->eVLAN_MemberViolation) {
	case IFX_ETHSW_VLAN_MEMBER_VIOLATION_NO:
			gsw_w32(PCE_VCTRL_VIMR_OFFSET + (10 * pPar->nPortId),	\
				PCE_VCTRL_VIMR_SHIFT, PCE_VCTRL_VIMR_SIZE, 0);
			gsw_w32(PCE_VCTRL_VEMR_OFFSET + (10 * pPar->nPortId),	\
				PCE_VCTRL_VEMR_SHIFT, PCE_VCTRL_VEMR_SIZE, 0);
			break;
	case IFX_ETHSW_VLAN_MEMBER_VIOLATION_INGRESS:
		gsw_w32(PCE_VCTRL_VIMR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_VIMR_SHIFT, PCE_VCTRL_VIMR_SIZE, 1);
		gsw_w32(PCE_VCTRL_VEMR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_VEMR_SHIFT, PCE_VCTRL_VEMR_SIZE, 0);
		break;
	case IFX_ETHSW_VLAN_MEMBER_VIOLATION_EGRESS:
		gsw_w32(PCE_VCTRL_VIMR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_VIMR_SHIFT, PCE_VCTRL_VIMR_SIZE, 0);
		gsw_w32(PCE_VCTRL_VEMR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_VEMR_SHIFT, PCE_VCTRL_VEMR_SIZE, 1);
		break;
	case IFX_ETHSW_VLAN_MEMBER_VIOLATION_BOTH:
			gsw_w32(PCE_VCTRL_VIMR_OFFSET + (10 * pPar->nPortId),	\
				PCE_VCTRL_VIMR_SHIFT, PCE_VCTRL_VIMR_SIZE, 1);
			gsw_w32(PCE_VCTRL_VEMR_OFFSET + (10 * pPar->nPortId),	\
				PCE_VCTRL_VEMR_SHIFT, PCE_VCTRL_VEMR_SIZE, 1);
		break;
	default:
		pr_err("WARNING: (eVLAN_MemberViolation) %s:%s:%d \n", __FILE__, __func__, __LINE__);
	}
	switch (pPar->eAdmitMode) {
	case IFX_ETHSW_VLAN_ADMIT_ALL:
			value = 0;
			break;
	case IFX_ETHSW_VLAN_ADMIT_TAGGED:
			value = 1;
			break;
	case IFX_ETHSW_VLAN_ADMIT_UNTAGGED:
			value = 2;
			break;
	default:
			value = 0;
			pr_err("WARNING: (eAdmitMode) %s:%s:%d \n", __FILE__, __func__, __LINE__);
	}
	gsw_w32(PCE_VCTRL_VINR_OFFSET + (10 * pPar->nPortId),	\
		PCE_VCTRL_VINR_SHIFT, PCE_VCTRL_VINR_SIZE, value);
	value = 0;
	if (pPar->bTVM == LTQ_TRUE)
		value = 1;

	gsw_w32(PCE_PCTRL_0_TVM_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_TVM_SHIFT, PCE_PCTRL_0_TVM_SIZE, value);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_PortMemberAdd(void *pDevCtx, IFX_ETHSW_VLAN_portMemberAdd_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u8 table_index;
	vlan_active_table_t VLAN_Table_Entry;

	if ((pPar->nPortId >= pEthDev->nTotalPortNumber) && (!(pPar->nPortId & 0x80000000)))
		return LTQ_ERROR;
	table_index = find_active_vlan_index(pEthDev, pPar->nVId);
	if (table_index == 0xFF) {
		pr_err("(VID not exists) %s:%s:%d\n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	get_vlan_sw_table(pEthDev, table_index, &VLAN_Table_Entry);
	if (VLAN_Table_Entry.reserved == LTQ_TRUE) {
		pr_err("(VID was already reserved) %s:%s:%d\n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if (pPar->nPortId & 0x80000000) { /*Port Map */
		VLAN_Table_Entry.pm |= ((pPar->nPortId) & 0x7FFF) ;
		if (pPar->bVLAN_TagEgress)
			VLAN_Table_Entry.tm |= ((pPar->nPortId) & 0x7FFF) ;
		else
			VLAN_Table_Entry.tm &= ~((pPar->nPortId) & 0x7FFF);
	} else {
		VLAN_Table_Entry.pm |= 1 << pPar->nPortId; /*  single port index */
		if (pPar->bVLAN_TagEgress)
			VLAN_Table_Entry.tm |= 1 << pPar->nPortId;
		else
			VLAN_Table_Entry.tm &= ~(1 << pPar->nPortId);
	}
	vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	pcetable.table = PCE_VLANMAP_INDEX;
	pcetable.table_index = table_index;
	xwayflow_pce_table_read(pDevCtx, &pcetable);
	pcetable.table = PCE_VLANMAP_INDEX;
	pcetable.table_index = table_index;
	pcetable.val[1] = VLAN_Table_Entry.pm;
	pcetable.val[2] = VLAN_Table_Entry.tm;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_PortMemberRead(void *pDevCtx, IFX_ETHSW_VLAN_portMemberRead_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;

	if (pPar->bInitial == LTQ_TRUE) {
		pEthDev->vlan_table_index = 0; /*Start from the index 0 */
		pcetable.table = PCE_VLANMAP_INDEX;
		pcetable.table_index = pEthDev->vlan_table_index;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pPar->nVId = (pcetable.val[0] & 0xFFF);
		pPar->nPortId = (pcetable.val[1] | 0x80000000); /* Port Map */
		pPar->nTagId			= (pcetable.val[2] | 0x80000000); /*Port Map */
		pPar->bInitial = LTQ_FALSE;
		pPar->bLast = LTQ_FALSE;
	}
	if (pPar->bLast != LTQ_TRUE) {
		if (pEthDev->vlan_table_index < VLAN_ACTIVE_TABLE_SIZE) {
			pEthDev->vlan_table_index++;
			pcetable.table = PCE_VLANMAP_INDEX;
			pcetable.table_index = pEthDev->vlan_table_index;
			xwayflow_pce_table_read(pDevCtx, &pcetable);
			pPar->nVId = (pcetable.val[0] & 0xFFF);
			pPar->nPortId = (pcetable.val[1] | 0x80000000);	/* Port Map */
			pPar->nTagId = (pcetable.val[2] | 0x80000000);	/* Port Map */
		} else {
			pPar->bLast = LTQ_TRUE;
			pEthDev->vlan_table_index = 0;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_PortMemberRemove(void *pDevCtx, IFX_ETHSW_VLAN_portMemberRemove_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	vlan_active_table_t VLAN_Table_Entry;
	u8 table_index;
	if ((pPar->nPortId >= pEthDev->nTotalPortNumber) && (!(pPar->nPortId & 0x80000000)))
		return LTQ_ERROR;
	table_index = find_active_vlan_index(pEthDev, pPar->nVId);
	if (table_index == 0xFF) {
		pr_err("This vid doesn't exists\n");
		return LTQ_ERROR;
	}
	get_vlan_sw_table(pEthDev, table_index, &VLAN_Table_Entry);
	if (pPar->nPortId & 0x80000000)
		VLAN_Table_Entry.pm  &= ~((pPar->nPortId) & 0x7FFF) ;
	else
		VLAN_Table_Entry.pm &= ~(1 << pPar->nPortId);

	VLAN_Table_Entry.tm &= ~(1 << pPar->nPortId);
	vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	pcetable.table = PCE_VLANMAP_INDEX;
	pcetable.table_index = table_index;
	xwayflow_pce_table_read(pDevCtx, &pcetable);
	if (pPar->nPortId & 0x80000000) {
		pcetable.val[1] &= ~((pPar->nPortId) & 0x7FFF);
		pcetable.val[2] &= ~((pPar->nPortId) & 0x7FFF);
	} else {
		pcetable.val[1] &= ~(1 << pPar->nPortId);
		pcetable.val[2] &= ~(1 << pPar->nPortId);
	}
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_ReservedAdd(void *pDevCtx, IFX_ETHSW_VLAN_reserved_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u8 table_index;
	vlan_active_table_t VLAN_Table_Entry;

	table_index = find_active_vlan_index(pEthDev, pPar->nVId);
	if (table_index == 0xFF) {
		pr_err("(VID not exist, Please create VID) %s:%s:%d \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	get_vlan_sw_table(pEthDev, table_index, &VLAN_Table_Entry);
	if (VLAN_Table_Entry.pm != 0) {
		pr_err("This VID was already add into the member, can not reserve\n");
		return LTQ_ERROR;
	}
	if (VLAN_Table_Entry.tm != 0) {
		pr_err("This VID was already add into the member, can not reserve\n");
		return LTQ_ERROR;
	}
	VLAN_Table_Entry.reserved = LTQ_TRUE;
	vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	pcetable.table_index = table_index;
	pcetable.table = PCE_ACTVLAN_INDEX;
	pcetable.val[0] |= (1 << 8);
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VLAN_ReservedRemove(void *pDevCtx, IFX_ETHSW_VLAN_reserved_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u8 table_index;
	vlan_active_table_t VLAN_Table_Entry;

	table_index = find_active_vlan_index(pEthDev, pPar->nVId);
	if (table_index == 0xFF) {
		pr_err("This vid doesn't exists, Please create VID first\n");
		return LTQ_ERROR;
	}
	get_vlan_sw_table(pEthDev, table_index, &VLAN_Table_Entry);
	if (VLAN_Table_Entry.pm != 0) {
		pr_err("This VID was already add into the member, can not remove the reserve\n");
		return LTQ_ERROR;
	}
	if (VLAN_Table_Entry.tm != 0) {
		pr_err("This VID was already add into the member, can not remove the reserve\n");
		return LTQ_ERROR;
	}
	if (VLAN_Table_Entry.reserved == LTQ_FALSE) {
		pr_err("This VID was not reserve, please reserve it first\n");
		return LTQ_ERROR;
	} else {
		VLAN_Table_Entry.reserved = LTQ_FALSE;
		vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	}
	vlan_entry_set(pEthDev, table_index, &VLAN_Table_Entry);
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	pcetable.table_index = table_index;
	pcetable.table = PCE_ACTVLAN_INDEX;
	pcetable.val[0] &= ~(1 << 8);
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_SVLAN_CfgGet(void *pDevCtx, IFX_ETHSW_SVLAN_cfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 reg_val;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		gsw_r32((FDMA_SVTETYPE_OFFSET),	FDMA_SVTETYPE_ETYPE_SHIFT,	\
			FDMA_SVTETYPE_ETYPE_SIZE, &reg_val);
		pPar->nEthertype = reg_val;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_SVLAN_CfgSet(void *pDevCtx, IFX_ETHSW_SVLAN_cfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 reg_val;
	reg_val = pPar->nEthertype & 0xFFFF;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
			gsw_w32((FDMA_SVTETYPE_OFFSET), FDMA_SVTETYPE_ETYPE_SHIFT,	\
				FDMA_SVTETYPE_ETYPE_SIZE, reg_val);
			gsw_w32((MAC_VLAN_ETYPE_1_INNER_OFFSET),	MAC_VLAN_ETYPE_1_INNER_SHIFT,	\
				MAC_VLAN_ETYPE_1_INNER_SIZE, reg_val);
			/* ToDo: Update the Micro code based on SVAN*/
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_SVLAN_PortCfgGet(void *pDevCtx, IFX_ETHSW_SVLAN_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	ltq_bool_t bSVIMR, bSVEMR;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		/** nPortVId: retrieve the corresponding VLAN ID from the Active VLAN Table */
		gsw_r32(PCE_DEFPSVID_PVID_OFFSET + (2 * pPar->nPortId), \
			PCE_DEFPSVID_PVID_SHIFT, PCE_DEFPSVID_PVID_SIZE, &value);
    pPar->nPortVId = value;
    /* bSVLAN_TagSupport */
		gsw_r32(PCE_VCTRL_STEN_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_STEN_SHIFT, PCE_VCTRL_STEN_SIZE, &value);
		pPar->bSVLAN_TagSupport = value;

		/** bVLAN_ReAssign */
		gsw_r32(PCE_VCTRL_SVSR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_SVSR_SHIFT, PCE_VCTRL_SVSR_SIZE, &value);
    pPar->bVLAN_ReAssign = value;

    /** bVlanMemberViolationIngress */
    gsw_r32(PCE_VCTRL_SVIMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVIMR_SHIFT, PCE_VCTRL_SVIMR_SIZE, &value);
		bSVIMR = value;
		/** bVlanMemberViolationEgress */
		gsw_r32(PCE_VCTRL_SVEMR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_SVEMR_SHIFT, PCE_VCTRL_SVEMR_SIZE, &value);
		bSVEMR = value;
		if (bSVIMR == LTQ_FALSE && bSVEMR == LTQ_FALSE)
			pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_NO;
		if (bSVIMR == LTQ_TRUE && bSVEMR == LTQ_FALSE)
			pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_INGRESS;
		if (bSVIMR == LTQ_FALSE && bSVEMR == LTQ_TRUE)
			pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_EGRESS;
		if (bSVIMR == LTQ_TRUE && bSVEMR == LTQ_TRUE)
			pPar->eVLAN_MemberViolation = IFX_ETHSW_VLAN_MEMBER_VIOLATION_BOTH;
		/* eAdmitMode:  */
		gsw_r32(PCE_VCTRL_SVINR_OFFSET + (10 * pPar->nPortId), \
			PCE_VCTRL_SVINR_SHIFT, PCE_VCTRL_SVINR_SIZE, &value);
		switch (value) {
		case 0:
				pPar->eAdmitMode = IFX_ETHSW_VLAN_ADMIT_ALL;
				break;
		case 1:
				pPar->eAdmitMode = IFX_ETHSW_VLAN_ADMIT_TAGGED;
				break;
		case 2:
				pPar->eAdmitMode = IFX_ETHSW_VLAN_ADMIT_UNTAGGED;
				break;
		default:
				break;
		} /* -----  end switch  ----- */
		/** bSVLAN_MACbasedTag */
		gsw_r32(PCE_VCTRL_MACEN_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_MACEN_SHIFT, PCE_VCTRL_MACEN_SIZE, &value);
		pPar->bSVLAN_MACbasedTag = value;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_SVLAN_PortCfgSet(void *pDevCtx, IFX_ETHSW_SVLAN_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;

	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		value = pPar->nPortVId;
		gsw_w32(PCE_DEFPSVID_PVID_OFFSET + (2 * pPar->nPortId),	\
			PCE_DEFPSVID_PVID_SHIFT, PCE_DEFPSVID_PVID_SIZE, value);
		/* bSVLAN_TagSupport */
		value = pPar->bSVLAN_TagSupport;
		gsw_w32(PCE_VCTRL_STEN_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_STEN_SHIFT, PCE_VCTRL_STEN_SIZE, value);
		if (pPar->bSVLAN_TagSupport == 1) {
				gsw_w32(FDMA_PCTRL_SVLANMOD_OFFSET + (6 * pPar->nPortId),	\
					FDMA_PCTRL_SVLANMOD_SHIFT, FDMA_PCTRL_SVLANMOD_SIZE, 3);
			} else {
				gsw_w32(FDMA_PCTRL_SVLANMOD_OFFSET + (6 * pPar->nPortId),	\
					FDMA_PCTRL_SVLANMOD_SHIFT, FDMA_PCTRL_SVLANMOD_SIZE, 0);
			}
		/** bVLAN_ReAssign */
		value = pPar->bVLAN_ReAssign;
		gsw_w32(PCE_VCTRL_SVSR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_SVSR_SHIFT, PCE_VCTRL_SVSR_SIZE, value);
		/** eVLAN_MemberViolation  */
		switch (pPar->eVLAN_MemberViolation) {
		case IFX_ETHSW_VLAN_MEMBER_VIOLATION_NO:
				gsw_w32(PCE_VCTRL_SVIMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVIMR_SHIFT, PCE_VCTRL_SVIMR_SIZE, 0);
				gsw_w32(PCE_VCTRL_SVEMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVEMR_SHIFT, PCE_VCTRL_SVEMR_SIZE, 0);
				break;
		case IFX_ETHSW_VLAN_MEMBER_VIOLATION_INGRESS:
				gsw_w32(PCE_VCTRL_SVIMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVIMR_SHIFT, PCE_VCTRL_SVIMR_SIZE, 1);
				gsw_w32(PCE_VCTRL_SVEMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVEMR_SHIFT, PCE_VCTRL_SVEMR_SIZE, 0);
				break;
		case IFX_ETHSW_VLAN_MEMBER_VIOLATION_EGRESS:
				gsw_w32(PCE_VCTRL_SVIMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVIMR_SHIFT, PCE_VCTRL_SVIMR_SIZE, 0);
				gsw_w32(PCE_VCTRL_SVEMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVEMR_SHIFT, PCE_VCTRL_SVEMR_SIZE, 1);
				break;
		case IFX_ETHSW_VLAN_MEMBER_VIOLATION_BOTH:
				gsw_w32(PCE_VCTRL_SVIMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVIMR_SHIFT, PCE_VCTRL_SVIMR_SIZE, 1);
				gsw_w32(PCE_VCTRL_SVEMR_OFFSET + (10 * pPar->nPortId),	\
					PCE_VCTRL_SVEMR_SHIFT, PCE_VCTRL_SVEMR_SIZE, 1);
				break;
		} /* -----  end switch  ----- */
		/** eAdmitMode */
		switch (pPar->eAdmitMode) {
		case IFX_ETHSW_VLAN_ADMIT_ALL:
				value = 0;
				break;
		case IFX_ETHSW_VLAN_ADMIT_TAGGED:
				value = 1;
				break;
		case IFX_ETHSW_VLAN_ADMIT_UNTAGGED:
				value = 2;
				break;
		default:
				value = 0;
		} /* -----  end switch  ----- */
		gsw_w32(PCE_VCTRL_SVINR_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_SVINR_SHIFT, PCE_VCTRL_SVINR_SIZE, value);
		/** bSVLAN_MACbasedTag */
		value = pPar->bSVLAN_MACbasedTag;
		gsw_w32(PCE_VCTRL_MACEN_OFFSET + (10 * pPar->nPortId),	\
			PCE_VCTRL_MACEN_SHIFT, PCE_VCTRL_MACEN_SIZE, value);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_SVLAN_ClassPCP_PortGet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u32  value, dei;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		for (value = 0; value < 16; value++) {
			memset(&pcetable, 0 , sizeof(pce_table_prog_t));
			pcetable.table_index = (((pPar->nPortId & 0xF) << 4) | (value));
			pcetable.table = PCE_EGREMARK_INDEX;
			xwayflow_pce_table_read(pDevCtx, &pcetable);
			pPar->nDSCP[value]	= (pcetable.val[0] & 0x3F);
			pPar->nCPCP[value]	= ((pcetable.val[0] >> 8) & 0x7);
			pPar->nSPCP[value]	= ((pcetable.val[1] >> 8) & 0x7);
			dei 								= ((pcetable.val[1])  & 0x1);  /*DEI */
			pPar->nSPCP[value]	|= (dei << 7);
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_SVLAN_ClassPCP_PortSet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_ClassPCP_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u32  value;
	u8 cpcp, dscp, spcp, dei ;

		if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;

	if ((pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC)) {
		for (value = 0; value < 16; value++) {
			memset(&pcetable, 0 , sizeof(pce_table_prog_t));
			pcetable.table_index =  (((pPar->nPortId & 0xF) << 4) | (value));
			dscp = pPar->nDSCP[value] & 0x3F;
			spcp = pPar->nSPCP[value] & 0x7;
			cpcp = pPar->nCPCP[value] & 0x7;
			dei = ((pPar->nSPCP[value] >> 7) & 1);
			pcetable.val[1] = ((spcp << 8) | dei);
			pcetable.val[0] = (dscp | (cpcp << 8));
			pcetable.table = PCE_EGREMARK_INDEX;
			xwayflow_pce_table_write(pDevCtx, &pcetable);
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_SVLAN_PCP_ClassGet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t pcetable;
	u32  value;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
			memset(&pcetable, 0 , sizeof(pce_table_prog_t));
			for (value = 0; value < 16; value++) {
				pcetable.table = PCE_SPCP_INDEX;
				pcetable.table_index = value;
				xwayflow_pce_table_read(pDevCtx, &pcetable);
				pPar->nTrafficClass[value] = pcetable.val[0] & 0xF;
				pPar->nTrafficColor[value] = ((pcetable.val[0] >> 6) & 0x3);
				pPar->nPCP_Remark_Enable[value] = ((pcetable.val[0] >> 4) & 0x1);
				pPar->nDEI_Remark_Enable[value] = ((pcetable.val[0] >> 5) & 0x1);
			}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_SVLAN_PCP_ClassSet(void *pDevCtx, IFX_ETHSW_QoS_SVLAN_PCP_ClassCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		pce_table_prog_t pcetable;
		u32  value;
		for (value = 0; value < 16; value++) {
			memset(&pcetable, 0 , sizeof(pce_table_prog_t));
			pcetable.table = PCE_SPCP_INDEX;
			pcetable.table_index = value;
			pcetable.val[0] = pPar->nTrafficClass[value] & 0xF;
			pcetable.val[0] |= (pPar->nTrafficColor[value] & 0x3) << 6;
			pcetable.val[0] |= (pPar->nPCP_Remark_Enable[value] & 0x1) << 4;
			pcetable.val[0] |= (pPar->nDEI_Remark_Enable[value] & 0x1) << 5;
			xwayflow_pce_table_write(pDevCtx, &pcetable);
		}
	}
	return LTQ_SUCCESS;
}
#endif /*CONFIG_LTQ_VLAN */
#if defined(CONFIG_LTQ_QOS) && CONFIG_LTQ_QOS
ethsw_ret_t IFX_FLOW_QoS_MeterCfgGet(void *pDevCtx, IFX_ETHSW_QoS_meterCfg_t *pPar)
{
	u32 nMeterId = pPar->nMeterId, value, exp, mant, ibs;
	if (nMeterId > 7)
		return LTQ_ERROR;
	/* Enable/Disable the meter shaper */
	gsw_r32(PCE_TCM_CTRL_TCMEN_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CTRL_TCMEN_SHIFT, PCE_TCM_CTRL_TCMEN_SIZE, &value);
	pPar->bEnable = value;
	/* Committed Burst Size */
	gsw_r32(PCE_TCM_CBS_CBS_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CBS_CBS_SHIFT, PCE_TCM_CBS_CBS_SIZE, &value);
	pPar->nCbs = (value * 64);
	/* Excess Burst Size (EBS [bytes]) */
	gsw_r32(PCE_TCM_EBS_EBS_OFFSET + (nMeterId * 7),	\
		PCE_TCM_EBS_EBS_SHIFT, PCE_TCM_EBS_EBS_SIZE, &value);
	pPar->nEbs = (value * 64);
	/* Rate Counter Exponent */
	gsw_r32(PCE_TCM_CIR_EXP_EXP_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CIR_EXP_EXP_SHIFT, PCE_TCM_CIR_EXP_EXP_SIZE, &exp);
	/* Rate Counter Mantissa */
	gsw_r32(PCE_TCM_CIR_MANT_MANT_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CIR_MANT_MANT_SHIFT, PCE_TCM_CIR_MANT_MANT_SIZE, &mant);
   /* Rate Counter iBS */
	gsw_r32(PCE_TCM_IBS_IBS_OFFSET + (nMeterId * 7),	\
		PCE_TCM_IBS_IBS_SHIFT, PCE_TCM_IBS_IBS_SIZE, &ibs);
	/* calc the Rate */
	pPar->nRate = RateCalc(ibs, exp, mant);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_MeterCfgSet(void *pDevCtx, IFX_ETHSW_QoS_meterCfg_t *pPar)
{
	u32 nMeterId, nCbs, nEbs, exp = 0, mant = 0, rate, ibs = 0;
	nMeterId = pPar->nMeterId;
	if (nMeterId > 7)
		return LTQ_ERROR;

	/* Committed Burst Size */
	if (pPar->nCbs > 0xFFC0)
		nCbs = 0x3FF;
	else
		nCbs = ((pPar->nCbs + 63) / 64);
	gsw_w32(PCE_TCM_CBS_CBS_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CBS_CBS_SHIFT, PCE_TCM_CBS_CBS_SIZE, nCbs);
	/* Excess Burst Size (EBS [bytes]) */
	if (pPar->nEbs > 0xFFC0)
		nEbs = 0x3FF;
	else
		nEbs = ((pPar->nEbs + 63) / 64);
	gsw_w32(PCE_TCM_EBS_EBS_OFFSET + (nMeterId * 7),	\
		PCE_TCM_EBS_EBS_SHIFT, PCE_TCM_EBS_EBS_SIZE, nEbs);
	/* Calc the Rate and convert to MANT and EXP*/
	rate = pPar->nRate;
	if (rate)
		calcToken(rate, &ibs, &exp, &mant);
	/* Rate Counter Exponent */
	gsw_w32(PCE_TCM_CIR_EXP_EXP_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CIR_EXP_EXP_SHIFT, PCE_TCM_CIR_EXP_EXP_SIZE, exp);
	/* Rate Counter Mantissa */
	gsw_w32(PCE_TCM_CIR_MANT_MANT_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CIR_MANT_MANT_SHIFT, PCE_TCM_CIR_MANT_MANT_SIZE, mant);
	/* Rate Counter iBS */
	gsw_w32(PCE_TCM_IBS_IBS_OFFSET + (nMeterId * 7),	\
		PCE_TCM_IBS_IBS_SHIFT, PCE_TCM_IBS_IBS_SIZE, ibs);
	/* Enable/Disable the meter shaper */
	gsw_w32(PCE_TCM_CTRL_TCMEN_OFFSET + (nMeterId * 7),	\
		PCE_TCM_CTRL_TCMEN_SHIFT, PCE_TCM_CTRL_TCMEN_SIZE, pPar->bEnable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_MeterPortAssign(void *pDevCtx, IFX_ETHSW_QoS_meterPort_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  EgressPortId, IngressPortId, value1, value2;
	ltq_bool_t EntryFound_Tbl = LTQ_FALSE, noEmptyEntry_Tb1 = LTQ_FALSE;
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	value1 = 0;
	while (value1 < 8) {
		pcetable.table = PCE_METER_INS_0_INDEX;
		pcetable.table_index = value1;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		if (pcetable.valid == 1) {
			IngressPortId = pcetable.key[0] & 0xF;
			EgressPortId = (pcetable.key[0] >> 8) & 0xF;
			if ((EgressPortId == pPar->nPortEgressId) && (IngressPortId == pPar->nPortIngressId)) {
				EntryFound_Tbl = LTQ_TRUE;
				value2 = 0;
				while (value2 < 8) {
					pcetable.table = PCE_METER_INS_1_INDEX;
					pcetable.table_index = value2;
					xwayflow_pce_table_read(pDevCtx, &pcetable);
					if (pcetable.valid == 1) {
						IngressPortId = pcetable.key[0] & 0xF;
						EgressPortId = (pcetable.key[0] >> 8) & 0xF;
						if ((EgressPortId == pPar->nPortEgressId) && (IngressPortId == pPar->nPortIngressId))
							return LTQ_ERROR;
					}
					value2++;
				}
			}
		}
		value1++;
	}
	/*  Not in the original table, write a new one */
	if (EntryFound_Tbl == LTQ_FALSE) {
		value1 = 0;
		memset(&pcetable, 0 , sizeof(pce_table_prog_t));
		/* Search for whole Table 1*/
		while (value1 < 8) {
			pcetable.table = PCE_METER_INS_0_INDEX;
			pcetable.table_index = value1;
			xwayflow_pce_table_read(pDevCtx, &pcetable);
			/* We found the empty one */
			if (pcetable.valid == 0) {
				switch (pPar->eDir) {
				case IFX_ETHSW_DIRECTION_BOTH:
						pcetable.key[0] = (((pPar->nPortEgressId & 0xF) << 8) |	\
							(pPar->nPortIngressId & 0xF));
						pcetable.mask = 0;
					break;
				case IFX_ETHSW_DIRECTION_EGRESS:
						pcetable.key[0] = (((pPar->nPortEgressId & 0xF) << 8) | 0xF);
						pcetable.mask = 1;
						break;
				case IFX_ETHSW_DIRECTION_INGRESS:
						pcetable.key[0] = (0xF00 | (pPar->nPortIngressId & 0xF));
						pcetable.mask = 4;
						break;
				default:
						pcetable.key[0] = 0;
						pcetable.mask = 5;
				}
				pcetable.val[0] = pPar->nMeterId & 0x3F;
				pcetable.valid = 1;
				xwayflow_pce_table_write(pDevCtx, &pcetable);
				return LTQ_SUCCESS;
			}
			value1++;
		}
		if (value1 >= 8)
			noEmptyEntry_Tb1 = LTQ_TRUE;
	}

	/* The Table 1 is full, We go search table 2 */
	if ((noEmptyEntry_Tb1 == LTQ_TRUE) || (EntryFound_Tbl == LTQ_TRUE)) {
		value2 = 0;
		memset(&pcetable, 0 , sizeof(pce_table_prog_t));
		while (value2 < 8) {
			pcetable.table = PCE_METER_INS_1_INDEX;
			pcetable.table_index = value2;
			xwayflow_pce_table_read(pDevCtx, &pcetable);
			/* We found the empty one */
			if (pcetable.valid == 0) {
				switch (pPar->eDir) {
				case IFX_ETHSW_DIRECTION_BOTH:
						pcetable.key[0] = (((pPar->nPortEgressId & 0xF) << 8) |	\
							(pPar->nPortIngressId & 0xF));
						pcetable.mask = 0;
					break;
				case IFX_ETHSW_DIRECTION_EGRESS:
						pcetable.key[0] = (((pPar->nPortEgressId & 0xF) << 8) | 0xF);
						pcetable.mask = 1;
						break;
				case IFX_ETHSW_DIRECTION_INGRESS:
						pcetable.key[0] = (0xF00 | (pPar->nPortIngressId & 0xF));
						pcetable.mask = 4;
						break;
				default:
						pcetable.key[0] = 0;
						pcetable.mask = 5;
				}
				pcetable.val[0] = pPar->nMeterId & 0x3F;
				pcetable.valid = 1;
				xwayflow_pce_table_write(pDevCtx, &pcetable);
				noEmptyEntry_Tb1 = LTQ_FALSE;
				return LTQ_SUCCESS;
			}
			value2++;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_MeterPortDeassign(void *pDevCtx, IFX_ETHSW_QoS_meterPort_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  EgressPortId, IngressPortId, MeterId, i, j;
	ltq_bool_t EntryFound_Tbl = LTQ_FALSE;

	for (i = 0; i < 2; i++) {
		memset(&pcetable, 0 , sizeof(pce_table_prog_t));
		if (i == 0)
			pcetable.table = PCE_METER_INS_0_INDEX;
		else
			pcetable.table = PCE_METER_INS_1_INDEX;
		for (j = 0; j < 8; j++) {
			pcetable.table_index = j;
			xwayflow_pce_table_read(pDevCtx, &pcetable);
			if (pcetable.valid == 1) {
				IngressPortId = pcetable.key[0] & 0xF;
				EgressPortId = (pcetable.key[0] >> 8) & 0xF;
				MeterId = pcetable.val[0] & 0x1F;
				if ((EgressPortId == pPar->nPortEgressId) &&	\
					(IngressPortId == pPar->nPortIngressId) && (MeterId == pPar->nMeterId)) {
					if (i == 0)
						pcetable.table = PCE_METER_INS_0_INDEX;
					else
						pcetable.table = PCE_METER_INS_1_INDEX;
					pcetable.key[0] = 0;
					pcetable.val[0] = 0;
					pcetable.valid = 0;
					xwayflow_pce_table_write(pDevCtx, &pcetable);
					EntryFound_Tbl = LTQ_TRUE;
					pr_err("Found the entry, delet it\n");
				}

			}
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_MeterPortGet(void *pDevCtx, IFX_ETHSW_QoS_meterPortGet_t *pPar)
{
	pce_table_prog_t pcetable;
	u32 value = 0;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
/*	gsw_r32(ETHSW_CAP_3_METERS_OFFSET,	\
		ETHSW_CAP_3_METERS_SHIFT, ETHSW_CAP_3_METERS_SIZE, &value); */
	value = pEthDev->num_of_meters;
	if (pPar->bInitial == LTQ_TRUE) {
		pEthDev->meter_cnt = 0;
		pPar->bInitial = LTQ_FALSE;
	} else {
		if (pEthDev->meter_cnt > (value * 2))
			pPar->bLast = LTQ_TRUE;
	}

	if (pEthDev->meter_cnt > value)
		pcetable.table = PCE_METER_INS_1_INDEX;
	else
		pcetable.table = PCE_METER_INS_0_INDEX;

	pcetable.table_index = pEthDev->meter_cnt;
	xwayflow_pce_table_read(pDevCtx, &pcetable);
	if (pcetable.valid) {
		pPar->nMeterId = (pcetable.val[0] & 0x3F);
		pPar->nPortEgressId = ((pcetable.key[0] >> 8) & 0xF);
		pPar->nPortIngressId = (pcetable.key[0] & 0xF);
		if ((pcetable.mask & 0x5) == 0)
				pPar->eDir = IFX_ETHSW_DIRECTION_BOTH;
		else if ((pcetable.mask & 0x5) == 1)
			pPar->eDir = IFX_ETHSW_DIRECTION_EGRESS;
		else if ((pcetable.mask & 0x5) == 4)
			pPar->eDir = IFX_ETHSW_DIRECTION_INGRESS;
		else
			pPar->eDir = IFX_ETHSW_DIRECTION_NONE;
	}
	pEthDev->meter_cnt++;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_DSCP_ClassGet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_ClassCfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value <= 63; value++) {
		pcetable.table				= PCE_DSCP_INDEX;
		pcetable.table_index	= value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pPar->nTrafficClass[value] = pcetable.val[0] & 0xF;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_DSCP_ClassSet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_ClassCfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value <= 63; value++) {
		pcetable.table				= PCE_DSCP_INDEX;
		pcetable.table_index	= value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pcetable.val[0]				&= ~(0xF);
		pcetable.val[0]				|= (pPar->nTrafficClass[value] & 0xF);
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ClassDSCP_Get(void *pDevCtx, IFX_ETHSW_QoS_ClassDSCP_Cfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value < 16; value++) {
		pcetable.table				= PCE_REMARKING_INDEX;
		pcetable.table_index	= value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pPar->nDSCP[value]		= pcetable.val[0] & 0x3F;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ClassDSCP_Set(void *pDevCtx, IFX_ETHSW_QoS_ClassDSCP_Cfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  nDSCP, nPCP, value;

	for (value = 0; value < 16; value++) {
		pcetable.table = PCE_REMARKING_INDEX;
		pcetable.table_index = value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		nPCP	= (pcetable.val[0] >> 8) & 0x7;
		nDSCP	= pPar->nDSCP[value] & 0x3F;
		pcetable.val[0]		= (nPCP << 8) | nDSCP;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_DSCP_DropPrecedenceCfgGet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value <= 63; value++) {
		memset(&pcetable, 0 , sizeof(pce_table_prog_t));
		pcetable.table = PCE_DSCP_INDEX;
		pcetable.table_index = value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pPar->nDSCP_DropPrecedence[value]	= (pcetable.val[0] >> 4) & 0x3;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_DSCP_DropPrecedenceCfgSet(void *pDevCtx, IFX_ETHSW_QoS_DSCP_DropPrecedenceCfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value <= 63; value++) {
		memset(&pcetable, 0 , sizeof(pce_table_prog_t));
		pcetable.table = PCE_DSCP_INDEX;
		pcetable.table_index = value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pcetable.val[0] &= ~(0x3 << 4);
		pcetable.val[0] |= ((pPar->nDSCP_DropPrecedence[value] & 0x3) << 4);
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_PortRemarkingCfgGet(void *pDevCtx, IFX_ETHSW_QoS_portRemarkingCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32  value, value_CLPEN = 0, value_DPEN = 0, value_DSCPMOD = 0;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;

	gsw_r32(PCE_PCTRL_0_CLPEN_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_CLPEN_SHIFT, PCE_PCTRL_0_CLPEN_SIZE, &value_CLPEN);
	gsw_r32(PCE_PCTRL_0_DPEN_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_DPEN_SHIFT, PCE_PCTRL_0_DPEN_SIZE, &value_DPEN);
	gsw_r32(PCE_PCTRL_2_DSCPMOD_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_2_DSCPMOD_SHIFT, PCE_PCTRL_2_DSCPMOD_SIZE, &value_DSCPMOD);

	if ((value_CLPEN == 0) && (value_DPEN == 0) && (value_DSCPMOD == 0))
		pPar->eDSCP_IngressRemarkingEnable = IFX_ETHSW_DSCP_REMARK_DISABLE;
	else if ((value_CLPEN == 1) && (value_DPEN == 0) && (value_DSCPMOD == 1))
		pPar->eDSCP_IngressRemarkingEnable = IFX_ETHSW_DSCP_REMARK_TC6;
	else if ((value_CLPEN == 1) && (value_DPEN == 1) && (value_DSCPMOD == 1))
		pPar->eDSCP_IngressRemarkingEnable = IFX_ETHSW_DSCP_REMARK_TC3;
	else if ((value_CLPEN == 0) && (value_DPEN == 1) && (value_DSCPMOD == 1))
		pPar->eDSCP_IngressRemarkingEnable = IFX_ETHSW_DSCP_REMARK_DP3;
	else if ((value_CLPEN == 1) && (value_DPEN == 1) && (value_DSCPMOD == 1))
		pPar->eDSCP_IngressRemarkingEnable = IFX_ETHSW_DSCP_REMARK_DP3_TC3;

	gsw_r32(PCE_PCTRL_0_PCPEN_OFFSET + (10 * pPar->nPortId),\
		 PCE_PCTRL_0_PCPEN_SHIFT, PCE_PCTRL_0_PCPEN_SIZE, &value);
	pPar->bPCP_IngressRemarkingEnable = value;
	gsw_r32(FDMA_PCTRL_DSCPRM_OFFSET + (6 * pPar->nPortId),	\
		FDMA_PCTRL_DSCPRM_SHIFT, FDMA_PCTRL_DSCPRM_SIZE, &value);
	pPar->bDSCP_EgressRemarkingEnable = value;
	gsw_r32(FDMA_PCTRL_VLANMOD_OFFSET + (6 * pPar->nPortId),	\
		FDMA_PCTRL_VLANMOD_SHIFT, FDMA_PCTRL_VLANMOD_SIZE, &value);
	if (value == 3)
		pPar->bPCP_EgressRemarkingEnable = LTQ_ENABLE;
	else
		pPar->bPCP_EgressRemarkingEnable = LTQ_DISABLE;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_PortRemarkingCfgSet(void *pDevCtx, IFX_ETHSW_QoS_portRemarkingCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32  value, value_CLPEN = 0, value_DPEN = 0, value_DSCPMOD = 0;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	switch (pPar->eDSCP_IngressRemarkingEnable) {
	case IFX_ETHSW_DSCP_REMARK_DISABLE:
			value_CLPEN = 0; value_DPEN = 0; value_DSCPMOD = 0;
			break;
	case IFX_ETHSW_DSCP_REMARK_TC6:
			value_CLPEN = 1; value_DPEN = 0; value_DSCPMOD = 1;
			break;
	case IFX_ETHSW_DSCP_REMARK_TC3:
			value_CLPEN = 1; value_DPEN = 1; value_DSCPMOD = 1;
			break;
	case IFX_ETHSW_DSCP_REMARK_DP3:
			value_CLPEN = 0; value_DPEN = 1; value_DSCPMOD = 1;
			break;
	case IFX_ETHSW_DSCP_REMARK_DP3_TC3:
			value_CLPEN = 1; value_DPEN = 1; value_DSCPMOD = 1;
			break;
	}

	if (pPar->eDSCP_IngressRemarkingEnable == 0) {
		value_CLPEN = 0; value_DPEN = 0; value_DSCPMOD = 0;
	}
	gsw_w32(PCE_PCTRL_0_CLPEN_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_CLPEN_SHIFT, PCE_PCTRL_0_CLPEN_SIZE, value_CLPEN);
	gsw_w32(PCE_PCTRL_0_DPEN_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_DPEN_SHIFT, PCE_PCTRL_0_DPEN_SIZE, value_DPEN);
	gsw_w32(PCE_PCTRL_2_DSCPMOD_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_2_DSCPMOD_SHIFT, PCE_PCTRL_2_DSCPMOD_SIZE, value_DSCPMOD);
	if (pPar->bDSCP_EgressRemarkingEnable > 0)
		value = pPar->bDSCP_EgressRemarkingEnable;
	else
		value = 0;
	gsw_w32(FDMA_PCTRL_DSCPRM_OFFSET + (6 * pPar->nPortId),	\
		FDMA_PCTRL_DSCPRM_SHIFT, FDMA_PCTRL_DSCPRM_SIZE, value);
	if (pPar->bPCP_IngressRemarkingEnable > 0)
		value = pPar->bPCP_IngressRemarkingEnable;
	else
		value = 0;
	gsw_w32(PCE_PCTRL_0_PCPEN_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_0_PCPEN_SHIFT, PCE_PCTRL_0_PCPEN_SIZE, value);
	if (pPar->bPCP_EgressRemarkingEnable > 0)
		value = 3;
	else
		value = 0;
	gsw_w32(FDMA_PCTRL_VLANMOD_OFFSET + (6 * pPar->nPortId),	\
		FDMA_PCTRL_VLANMOD_SHIFT, FDMA_PCTRL_VLANMOD_SIZE, value);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ClassPCP_Get(void *pDevCtx, IFX_ETHSW_QoS_ClassPCP_Cfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value < 16; value++) {
		pcetable.table				= PCE_REMARKING_INDEX;
		pcetable.table_index	= value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pPar->nPCP[value]			= (pcetable.val[0] >> 8) & 0x7;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ClassPCP_Set(void *pDevCtx, IFX_ETHSW_QoS_ClassPCP_Cfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  nDSCP, nPCP, value;

	for (value = 0; value < 16; value++) {
		pcetable.table = PCE_REMARKING_INDEX;
		pcetable.table_index		= value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		nDSCP	= pcetable.val[0] & 0x3F;
		nPCP	= pPar->nPCP[value] & 0x7;
		pcetable.val[0]	= (nPCP << 8) | nDSCP;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_PCP_ClassGet(void *pDevCtx, IFX_ETHSW_QoS_PCP_ClassCfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value < 8; value++) {
		pcetable.table				= PCE_PCP_INDEX;
		pcetable.table_index	= value;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		pPar->nTrafficClass[value]	=  (pcetable.val[0] & 0xF);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_PCP_ClassSet(void *pDevCtx, IFX_ETHSW_QoS_PCP_ClassCfg_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  value;
	for (value = 0; value < 8; value++) {
		pcetable.table				= PCE_PCP_INDEX;
		pcetable.table_index	= value;
		pcetable.val[0]				= (pPar->nTrafficClass[value] & 0xF);
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_PortCfgGet(void *pDevCtx, IFX_ETHSW_QoS_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32  value, value_DSCP, value_PCP, value_SPCP = 0;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;

	gsw_r32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, &value_DSCP);
	gsw_r32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, &value_PCP);
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		gsw_r32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
			PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, &value_SPCP);
	}
	if ((value_DSCP == 0) && (value_PCP == 0) && (value_SPCP == 0))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_NO;
	else if ((value_DSCP == 2) && (value_PCP == 0) && (value_SPCP == 0))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_DSCP;
	else if ((value_DSCP == 0) && (value_PCP == 1) && (value_SPCP == 0))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_PCP;
	else if ((value_DSCP == 2) && (value_PCP == 1) && (value_SPCP == 0))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_DSCP_PCP;
	else if ((value_DSCP == 1) && (value_PCP == 1) && (value_SPCP == 0))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_PCP_DSCP;
	else if ((value_DSCP == 0) && (value_PCP == 0) && (value_SPCP == 1))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_SPCP;
	else if ((value_DSCP == 1) && (value_PCP == 0) && (value_SPCP == 1))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_SPCP_DSCP;
	else if ((value_DSCP == 2) && (value_PCP == 0) && (value_SPCP == 1))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_DSCP_SPCP;
	else if ((value_DSCP == 0) && (value_PCP == 1) && (value_SPCP == 1))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_SPCP_PCP;
	else if ((value_DSCP == 1) && (value_PCP == 1) && (value_SPCP == 1))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_SPCP_PCP_DSCP;
	else if ((value_DSCP == 2) && (value_PCP == 1) && (value_SPCP == 1))
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_DSCP_SPCP_PCP;
	else
		pPar->eClassMode = IFX_ETHSW_QOS_CLASS_SELECT_NO;

	gsw_r32(PCE_PCTRL_2_PCLASS_OFFSET + (10 * pPar->nPortId),	\
		PCE_PCTRL_2_PCLASS_SHIFT, PCE_PCTRL_2_PCLASS_SIZE, &value);
	pPar->nTrafficClass = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_PortCfgSet(void *pDevCtx, IFX_ETHSW_QoS_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	switch (pPar->eClassMode) {
	case IFX_ETHSW_QOS_CLASS_SELECT_NO:
			gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 0);
			gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 0);
			if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 0);
			}
			break;
	case IFX_ETHSW_QOS_CLASS_SELECT_DSCP:
			gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 2);
			gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 0);
			if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 0);
			}
			break;
	case IFX_ETHSW_QOS_CLASS_SELECT_PCP:
			gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 0);
			gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 1);
			if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 0);
			}
			break;
	case IFX_ETHSW_QOS_CLASS_SELECT_DSCP_PCP:
			gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 2);
			gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 1);
			if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 0);
			}
			break;
	case IFX_ETHSW_QOS_CLASS_SELECT_PCP_DSCP:
			gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId), \
				PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 1);
			gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
				PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 1);
			if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 0);
			}
		break;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2_ETC) {
		case IFX_ETHSW_QOS_CLASS_SELECT_SPCP:
				gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 0);
				gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 0);
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 1);
				break;
		case IFX_ETHSW_QOS_CLASS_SELECT_SPCP_DSCP:
				gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 1);
				gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 0);
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 1);
				break;
		case IFX_ETHSW_QOS_CLASS_SELECT_DSCP_SPCP:
				gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 2);
				gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 0);
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 1);
				break;
		case IFX_ETHSW_QOS_CLASS_SELECT_SPCP_PCP:
				gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 0);
				gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 1);
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 1);
				break;
		case IFX_ETHSW_QOS_CLASS_SELECT_SPCP_PCP_DSCP:
				gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 1);
				gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 1);
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 1);
				break;
		case IFX_ETHSW_QOS_CLASS_SELECT_DSCP_SPCP_PCP:
				gsw_w32(PCE_PCTRL_2_DSCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_DSCP_SHIFT, PCE_PCTRL_2_DSCP_SIZE, 2);
				gsw_w32(PCE_PCTRL_2_PCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_PCP_SHIFT, PCE_PCTRL_2_PCP_SIZE, 1);
				gsw_w32(PCE_PCTRL_2_SPCP_OFFSET + (10 * pPar->nPortId),	\
					PCE_PCTRL_2_SPCP_SHIFT, PCE_PCTRL_2_SPCP_SIZE, 1);
				break;
		}
	}
	if (pPar->nTrafficClass > 0xF)
		return LTQ_ERROR;
	else
		gsw_w32(PCE_PCTRL_2_PCLASS_OFFSET + (10 * pPar->nPortId),	\
			PCE_PCTRL_2_PCLASS_SHIFT, PCE_PCTRL_2_PCLASS_SIZE, pPar->nTrafficClass);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_QueuePortGet(void *pDevCtx, IFX_ETHSW_QoS_queuePort_t *pPar)
{
	pce_table_prog_t pcetable;
	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	pcetable.table = PCE_QUEUE_MAP_INDEX;
	pcetable.table_index = (((pPar->nPortId << 4) & 0x70) | (pPar->nTrafficClassId & 0xF));
	xwayflow_pce_table_read(pDevCtx, &pcetable);
	pPar->nQueueId = (pcetable.val[0] & 0x1F);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_QueuePortSet(void *pDevCtx, IFX_ETHSW_QoS_queuePort_t *pPar)
{
	pce_table_prog_t pcetable;
	u32  EgressPortId = 0, value;

	memset(&pcetable, 0 , sizeof(pce_table_prog_t));
	pcetable.table = PCE_QUEUE_MAP_INDEX;
	pcetable.table_index = ((pPar->nPortId << 4) & 0x70) | (pPar->nTrafficClassId & 0xF);
	pcetable.val[0] = (pPar->nQueueId & 0x1F);
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	/* Assign the Egress Port Id and Enable the Queue */
	EgressPortId = (pPar->nPortId & 0x7);
	gsw_w32(BM_RAM_VAL_0_VAL0_OFFSET,	\
		BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, EgressPortId);
	gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
		BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, pPar->nQueueId);
	/* Specify the Table Address */
	gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
		BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0xE);
	/* Assign Write operation */
	gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
		BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 1);
	value = 1;
	/* Active */
	gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
		 BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
	do {
		gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
	} while (value);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_SchedulerCfgGet(void *pDevCtx, IFX_ETHSW_QoS_schedulerCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	int nQueueId = pPar->nQueueId;
	u32  value;
/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;
	gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
		BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, nQueueId);
	gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
		BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0x8);
	/* Table Access Operation Mode Read */
	gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
		BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0x0);
	gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
		BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
	} while (value);
	gsw_r32(BM_RAM_VAL_0_VAL0_OFFSET,	\
		BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, &value);
	pPar->nWeight = value;
	if (value == 0xFFFF || value == 0x1800)
		pPar->eType = IFX_ETHSW_QOS_SCHEDULER_STRICT;
	else
		pPar->eType = IFX_ETHSW_QOS_SCHEDULER_WFQ;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_SchedulerCfgSet(void *pDevCtx, IFX_ETHSW_QoS_schedulerCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	int nQueueId = pPar->nQueueId;
	u32  value;
/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;
	gsw_w32(BM_RAM_ADDR_ADDR_OFFSET, \
		BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, nQueueId);
	gsw_w32(BM_RAM_CTRL_ADDR_OFFSET, \
		BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0x8);
	if (pPar->eType == IFX_ETHSW_QOS_SCHEDULER_STRICT)
		value = 0xFFFF;
	else
		value = pPar->nWeight;
	gsw_w32(BM_RAM_VAL_0_VAL0_OFFSET,	\
		BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, value);
	/* Table Access Operation Mode Write */
	gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
		BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0x1);
	/* Active */
	gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
		BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
	} while (value);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ShaperCfgGet(void *pDevCtx, IFX_ETHSW_QoS_ShaperCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	int nRateShaperId = pPar->nRateShaperId;
	u32  value, exp, mant, ibs;

/*	gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET,	\
		ETHSW_CAP_3_SHAPERS_SHIFT, ETHSW_CAP_3_SHAPERS_SIZE, &value); */
	if (nRateShaperId >= pEthDev->num_of_shapers)
		return LTQ_ERROR;
	/* Enable/Disable the rate shaper  */
	gsw_r32(RS_CTRL_RSEN_OFFSET + (nRateShaperId * 0x5),	\
		RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, &value);
	pPar->bEnable = value;
	/* Committed Burst Size (CBS [bytes]) */
	gsw_r32(RS_CBS_CBS_OFFSET + (nRateShaperId * 0x5),	\
		RS_CBS_CBS_SHIFT, RS_CBS_CBS_SIZE, &value);
	pPar->nCbs = (value * 64);
	/** Rate [Mbit/s] */
	gsw_r32(RS_CIR_EXP_EXP_OFFSET + (nRateShaperId * 0x5),	\
		RS_CIR_EXP_EXP_SHIFT, RS_CIR_EXP_EXP_SIZE, &exp);
	gsw_r32(RS_CIR_MANT_MANT_OFFSET + (nRateShaperId * 0x5),	\
		RS_CIR_MANT_MANT_SHIFT, RS_CIR_MANT_MANT_SIZE, &mant);
	gsw_r32(RS_IBS_IBS_OFFSET + (nRateShaperId * 0x5),	\
		RS_IBS_IBS_SHIFT, RS_IBS_IBS_SIZE, &ibs);
	/* calc the Rate */
	pPar->nRate = RateCalc(ibs, exp, mant);
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		gsw_r32(RS_CTRL_RSMOD_OFFSET + (nRateShaperId * 0x5),	\
			RS_CTRL_RSMOD_SHIFT, RS_CTRL_RSMOD_SIZE, &pPar->bAVB);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ShaperCfgSet(void *pDevCtx, IFX_ETHSW_QoS_ShaperCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	int nRateShaperId = pPar->nRateShaperId;
	u32  value, exp = 0, mant = 0, rate = 0, ibs = 0;

/*	gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET,	\
		ETHSW_CAP_3_SHAPERS_SHIFT, ETHSW_CAP_3_SHAPERS_SIZE, &value); */
	if (nRateShaperId >= pEthDev->num_of_shapers)
		return LTQ_ERROR;
	/* Committed Burst Size */
	if (pPar->nCbs > 0xFFC0)
		value = 0x3FF;
	else
		value = ((pPar->nCbs + 63) / 64);
	/* Committed Burst Size (CBS [bytes]) */
	gsw_w32(RS_CBS_CBS_OFFSET + (nRateShaperId * 0x5),	\
		RS_CBS_CBS_SHIFT, RS_CBS_CBS_SIZE, value);
	/* Rate [kbit/s] */
	/* Calc the Rate */
	rate = pPar->nRate;
	if (rate)
		calcToken(rate, &ibs, &exp, &mant);
	gsw_w32(RS_CIR_EXP_EXP_OFFSET + (nRateShaperId * 0x5),	\
		 RS_CIR_EXP_EXP_SHIFT, RS_CIR_EXP_EXP_SIZE, exp);
	gsw_w32(RS_CIR_MANT_MANT_OFFSET + (nRateShaperId * 0x5),	\
		RS_CIR_MANT_MANT_SHIFT, RS_CIR_MANT_MANT_SIZE, mant);
	gsw_w32(RS_IBS_IBS_OFFSET + (nRateShaperId * 0x5),	\
		RS_IBS_IBS_SHIFT, RS_IBS_IBS_SIZE, ibs);
	/* Enable/Disable the rate shaper  */
	gsw_w32(RS_CTRL_RSEN_OFFSET + (nRateShaperId * 0x5),	\
		RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, pPar->bEnable);
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		gsw_w32(RS_CTRL_RSMOD_OFFSET + (nRateShaperId * 0x5),	\
			RS_CTRL_RSMOD_SHIFT, RS_CTRL_RSMOD_SIZE, pPar->bAVB);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ShaperQueueAssign(void *pDevCtx, IFX_ETHSW_QoS_ShaperQueue_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 nRateShaperId = pPar->nRateShaperId, nQueueId = pPar->nQueueId;
	u32 value1_RS, value1_ShaperId, value2_RS, value2_ShaperId;
/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value1_RS); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;
/*	gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET,	\
		ETHSW_CAP_3_SHAPERS_SHIFT, ETHSW_CAP_3_SHAPERS_SIZE, &value1_RS); */
	if (nRateShaperId >= pEthDev->num_of_shapers)
		return LTQ_ERROR;
	/* Check Rate Shaper 1 Enable  */
	gsw_r32(PQM_RS_EN1_OFFSET + (nQueueId * 2),	\
		PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, &value1_RS);
	gsw_r32(PQM_RS_RS1_OFFSET + (nRateShaperId * 2),	\
		PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, &value1_ShaperId);
	/* Check Rate Shaper 2 Enable  */
	gsw_r32(PQM_RS_EN2_OFFSET + (nQueueId * 2),	\
		PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, &value2_RS);
	gsw_r32(PQM_RS_RS2_OFFSET + (nRateShaperId * 2),	\
		PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, &value2_ShaperId);
	if ((value1_RS == 1) && (value1_ShaperId == nRateShaperId))
		return LTQ_SUCCESS;
	else if ((value2_RS == 1) && (value2_ShaperId == nRateShaperId))
		return LTQ_SUCCESS;
	else if (value1_RS == 0) {
		gsw_w32(PQM_RS_RS1_OFFSET + (nQueueId * 2),	\
			PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, nRateShaperId);
		gsw_w32(PQM_RS_EN1_OFFSET + (nQueueId * 2),	\
			PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, 0x1);
		gsw_w32(RS_CTRL_RSEN_OFFSET + (nRateShaperId * 0x5),	\
			RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, 0x1);
	} else if (value2_RS == 0) {
		gsw_w32(PQM_RS_RS2_OFFSET + (nQueueId * 2),	\
			PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, nRateShaperId);
		gsw_w32(PQM_RS_EN2_OFFSET + (nQueueId * 2),	\
			PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, 0x1);
		gsw_w32(RS_CTRL_RSEN_OFFSET + (nRateShaperId * 0x5),	\
			RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, 0x1);
	} else {
		return LTQ_ERROR;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ShaperQueueDeassign(void *pDevCtx, IFX_ETHSW_QoS_ShaperQueue_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 nRateShaperId = pPar->nRateShaperId, nQueueId = pPar->nQueueId;
	u32 value1, value2, value3, value4;

/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value1); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;
/*	gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET,	\
		ETHSW_CAP_3_SHAPERS_SHIFT, ETHSW_CAP_3_SHAPERS_SIZE, &value1); */
	if (nRateShaperId >= pEthDev->num_of_shapers)
		return LTQ_ERROR;
	/* Rate Shaper 1 Read  */
	gsw_r32(PQM_RS_EN1_OFFSET + (nQueueId * 2),	\
		PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, &value1);
	gsw_r32(PQM_RS_RS1_OFFSET + (nQueueId * 2),	\
		PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, &value2);
	/* Rate Shaper 2 Read  */
	gsw_r32(PQM_RS_EN2_OFFSET + (nQueueId * 2),	\
		PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, &value3);
	gsw_r32(PQM_RS_RS2_OFFSET + (nQueueId * 2),	\
		PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, &value4);
	if ((value1 == 1) && (value2 == nRateShaperId)) {
		gsw_w32(PQM_RS_EN1_OFFSET + (nQueueId * 2),	\
			PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, 0);
		gsw_w32(PQM_RS_RS1_OFFSET + (nQueueId * 2),	\
			PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, 0);
		return LTQ_SUCCESS;
	} else if ((value3 == 1) && (value4 == nRateShaperId)) {
		gsw_w32(PQM_RS_EN2_OFFSET + (nQueueId * 2),	\
			PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, 0);
		gsw_w32(PQM_RS_RS2_OFFSET + (nQueueId * 2),	\
			PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, 0);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
	if ((value1 == 0) && (value3 == 0)) {
		gsw_w32(RS_CTRL_RSEN_OFFSET + (nRateShaperId * 0x5),	\
			RS_CTRL_RSEN_SHIFT, RS_CTRL_RSEN_SIZE, 0);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_ShaperQueueGet(void *pDevCtx, IFX_ETHSW_QoS_ShaperQueueGet_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value_RS, value;
	u8 nQueueId = pPar->nQueueId;

/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value_RS); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;

	pPar->bAssigned = LTQ_FALSE;
	pPar->nRateShaperId = 0;
	gsw_r32(PQM_RS_EN1_OFFSET + (nQueueId * 2),	\
		PQM_RS_EN1_SHIFT, PQM_RS_EN1_SIZE, &value_RS);
	if (value_RS == 1) {
		pPar->bAssigned = LTQ_TRUE;
		gsw_r32(PQM_RS_RS1_OFFSET + (nQueueId * 2),	\
			PQM_RS_RS1_SHIFT, PQM_RS_RS1_SIZE, &value);
		pPar->nRateShaperId = value;
	}
	gsw_r32(PQM_RS_EN2_OFFSET + (nQueueId * 2),	\
		PQM_RS_EN2_SHIFT, PQM_RS_EN2_SIZE, &value_RS);
	if (value_RS == 1) {
		gsw_r32(PQM_RS_RS2_OFFSET + (nQueueId * 2),	\
			PQM_RS_RS2_SHIFT, PQM_RS_RS2_SIZE, &value);
		pPar->nRateShaperId = value;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_StormCfgSet(void *pDevCtx, IFX_ETHSW_QoS_stormCfg_t *pPar)
{
	u32 SCONBC, SCONMC, SCONUC, MeterID;
	if ((pPar->bBroadcast == LTQ_FALSE)	\
		 && (pPar->bMulticast == LTQ_FALSE)	\
		 && (pPar->bUnknownUnicast == LTQ_FALSE)) {
		/*  Storm Control Mode  */
		gsw_w32(PCE_GCTRL_0_SCONMOD_OFFSET,	\
			PCE_GCTRL_0_SCONMOD_SHIFT, PCE_GCTRL_0_SCONMOD_SIZE, 0);
		/* Meter instances used for broadcast traffic  */
		gsw_w32(PCE_GCTRL_0_SCONBC_OFFSET,	\
			PCE_GCTRL_0_SCONBC_SHIFT, PCE_GCTRL_0_SCONBC_SIZE, 0);
		/* Meter instances used for multicast traffic  */
		gsw_w32(PCE_GCTRL_0_SCONMC_OFFSET,	\
			PCE_GCTRL_0_SCONMC_SHIFT, PCE_GCTRL_0_SCONMC_SIZE, 0);
		/* Meter instances used for unknown unicast traffic  */
		gsw_w32(PCE_GCTRL_0_SCONUC_OFFSET,	\
			PCE_GCTRL_0_SCONUC_SHIFT, PCE_GCTRL_0_SCONUC_SIZE, 0);
	}
	/*  Meter ID */
	gsw_r32(PCE_GCTRL_0_SCONMET_OFFSET,	\
		PCE_GCTRL_0_SCONMET_SHIFT, PCE_GCTRL_0_SCONMET_SIZE, &MeterID);
	/* Meter instances used for broadcast traffic  */
	gsw_r32(PCE_GCTRL_0_SCONBC_OFFSET,	\
		PCE_GCTRL_0_SCONBC_SHIFT, PCE_GCTRL_0_SCONBC_SIZE, &SCONBC);
	/* Meter instances used for multicast traffic  */
	gsw_r32(PCE_GCTRL_0_SCONMC_OFFSET,	\
		PCE_GCTRL_0_SCONMC_SHIFT, PCE_GCTRL_0_SCONMC_SIZE, &SCONMC);
	/* Meter instances used for unknown unicast traffic  */
	gsw_r32(PCE_GCTRL_0_SCONUC_OFFSET,	\
		PCE_GCTRL_0_SCONUC_SHIFT, PCE_GCTRL_0_SCONUC_SIZE, &SCONUC);

	if ((SCONBC == 1) || (SCONMC == 1) || (SCONMC == 1)) {
		if (pPar->nMeterId == (MeterID + 1)) {
			/*  Storm Control Mode  */
			gsw_w32(PCE_GCTRL_0_SCONMOD_OFFSET,	\
				PCE_GCTRL_0_SCONMOD_SHIFT, PCE_GCTRL_0_SCONMOD_SIZE, 3);
		} else if (pPar->nMeterId != MeterID)
			return LTQ_ERROR;
	} else {
		/*  Storm Control Mode  */
		gsw_w32(PCE_GCTRL_0_SCONMOD_OFFSET,	\
			PCE_GCTRL_0_SCONMOD_SHIFT, PCE_GCTRL_0_SCONMOD_SIZE, 1);
		gsw_w32(PCE_GCTRL_0_SCONMET_OFFSET,	\
			PCE_GCTRL_0_SCONMET_SHIFT, PCE_GCTRL_0_SCONMET_SIZE, pPar->nMeterId);
	}
	/* Meter instances used for broadcast traffic  */
	gsw_w32(PCE_GCTRL_0_SCONBC_OFFSET,	\
		PCE_GCTRL_0_SCONBC_SHIFT, PCE_GCTRL_0_SCONBC_SIZE, pPar->bBroadcast);
	/* Meter instances used for multicast traffic  */
	gsw_w32(PCE_GCTRL_0_SCONMC_OFFSET,	\
		PCE_GCTRL_0_SCONMC_SHIFT, PCE_GCTRL_0_SCONMC_SIZE, pPar->bMulticast);
	/* Meter instances used for unknown unicast traffic  */
	gsw_w32(PCE_GCTRL_0_SCONUC_OFFSET,	\
		PCE_GCTRL_0_SCONUC_SHIFT, PCE_GCTRL_0_SCONUC_SIZE, pPar->bUnknownUnicast);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_StormCfgGet(void *pDevCtx, IFX_ETHSW_QoS_stormCfg_t *pPar)
{
	u32 value;
	/*  Storm Control Mode  */
	gsw_r32(PCE_GCTRL_0_SCONMOD_OFFSET,	\
		PCE_GCTRL_0_SCONMOD_SHIFT, PCE_GCTRL_0_SCONMOD_SIZE, &value);
	if (value == 0) {
		pPar->nMeterId = LTQ_FALSE;
		pPar->bBroadcast = LTQ_FALSE;
		pPar->bMulticast = LTQ_FALSE;
		pPar->bUnknownUnicast =  LTQ_FALSE;
	} else {
		gsw_r32(PCE_GCTRL_0_SCONMET_OFFSET,	\
			PCE_GCTRL_0_SCONMET_SHIFT, PCE_GCTRL_0_SCONMET_SIZE, &value);
		pPar->nMeterId = value;
		/* Meter instances used for broadcast traffic  */
		gsw_r32(PCE_GCTRL_0_SCONBC_OFFSET,	\
			PCE_GCTRL_0_SCONBC_SHIFT, PCE_GCTRL_0_SCONBC_SIZE, &value);
		pPar->bBroadcast = value;
		/* Meter instances used for multicast traffic  */
		gsw_r32(PCE_GCTRL_0_SCONMC_OFFSET,	\
			PCE_GCTRL_0_SCONMC_SHIFT, PCE_GCTRL_0_SCONMC_SIZE, &value);
		pPar->bMulticast = value;
		/* Meter instances used for unknown unicast traffic  */
		gsw_r32(PCE_GCTRL_0_SCONUC_OFFSET,	\
			PCE_GCTRL_0_SCONUC_SHIFT, PCE_GCTRL_0_SCONUC_SIZE, &value);
		pPar->bUnknownUnicast = value;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_WredCfgGet(void *pDevCtx, IFX_ETHSW_QoS_WRED_Cfg_t *pPar)
{
	/* Description: 'Drop Probability Profile' */
	gsw_r32(BM_QUEUE_GCTRL_DPROB_OFFSET,	\
		BM_QUEUE_GCTRL_DPROB_SHIFT, BM_QUEUE_GCTRL_DPROB_SIZE, &pPar->eProfile);
	/* WRED Red Threshold - Minimum */
	gsw_r32(BM_WRED_RTH_0_MINTH_OFFSET,	\
		BM_WRED_RTH_0_MINTH_SHIFT, BM_WRED_RTH_0_MINTH_SIZE, &pPar->nRed_Min);
	/* WRED Red Threshold - Maximum */
	gsw_r32(BM_WRED_RTH_1_MAXTH_OFFSET,	\
		BM_WRED_RTH_1_MAXTH_SHIFT, BM_WRED_RTH_1_MAXTH_SIZE, &pPar->nRed_Max);
	/* WRED Yellow Threshold - Minimum */
	gsw_r32(BM_WRED_YTH_0_MINTH_OFFSET,	\
		BM_WRED_YTH_0_MINTH_SHIFT, BM_WRED_YTH_0_MINTH_SIZE, &pPar->nYellow_Min);
	/* WRED Yellow Threshold - Maximum */
	gsw_r32(BM_WRED_YTH_1_MAXTH_OFFSET,	\
		BM_WRED_YTH_1_MAXTH_SHIFT, BM_WRED_YTH_1_MAXTH_SIZE, &pPar->nYellow_Max);
	/* WRED Green Threshold - Minimum */
	gsw_r32(BM_WRED_GTH_0_MINTH_OFFSET,	\
		BM_WRED_GTH_0_MINTH_SHIFT, BM_WRED_GTH_0_MINTH_SIZE, &pPar->nGreen_Min);
	/* WRED Green Threshold - Maximum */
	gsw_r32(BM_WRED_GTH_1_MAXTH_OFFSET,	\
		BM_WRED_GTH_1_MAXTH_SHIFT, BM_WRED_GTH_1_MAXTH_SIZE, &pPar->nGreen_Max);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_WredCfgSet(void *pDevCtx, IFX_ETHSW_QoS_WRED_Cfg_t *pPar)
{
	/* Set the global threshold */
	gsw_w32(BM_QUEUE_GCTRL_GL_MOD_OFFSET,	\
		BM_QUEUE_GCTRL_GL_MOD_SHIFT, BM_QUEUE_GCTRL_GL_MOD_SIZE, 0x1);
	/* Description: 'Drop Probability Profile' */
	gsw_w32(BM_QUEUE_GCTRL_DPROB_OFFSET,	\
		BM_QUEUE_GCTRL_DPROB_SHIFT, BM_QUEUE_GCTRL_DPROB_SIZE, pPar->eProfile);
	/* WRED Red Threshold - Minimum */
	gsw_w32(BM_WRED_RTH_0_MINTH_OFFSET,	\
		BM_WRED_RTH_0_MINTH_SHIFT, BM_WRED_RTH_0_MINTH_SIZE, pPar->nRed_Min);
	/* WRED Red Threshold - Maximum */
	gsw_w32(BM_WRED_RTH_1_MAXTH_OFFSET,	\
		BM_WRED_RTH_1_MAXTH_SHIFT, BM_WRED_RTH_1_MAXTH_SIZE, pPar->nRed_Max);
	/* WRED Yellow Threshold - Minimum */
	gsw_w32(BM_WRED_YTH_0_MINTH_OFFSET,	\
		BM_WRED_YTH_0_MINTH_SHIFT, BM_WRED_YTH_0_MINTH_SIZE, pPar->nYellow_Min);
	/* WRED Yellow Threshold - Maximum */
	gsw_w32(BM_WRED_YTH_1_MAXTH_OFFSET,	\
		BM_WRED_YTH_1_MAXTH_SHIFT, BM_WRED_YTH_1_MAXTH_SIZE, pPar->nYellow_Max);
	/* WRED Green Threshold - Minimum */
	gsw_w32(BM_WRED_GTH_0_MINTH_OFFSET,	\
		BM_WRED_GTH_0_MINTH_SHIFT, BM_WRED_GTH_0_MINTH_SIZE, pPar->nGreen_Min);
	/* WRED Green Threshold - Maximum */
	gsw_w32(BM_WRED_GTH_1_MAXTH_OFFSET,	\
		BM_WRED_GTH_1_MAXTH_SHIFT, BM_WRED_GTH_1_MAXTH_SIZE, pPar->nGreen_Max);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_WredQueueCfgGet(void *pDevCtx, IFX_ETHSW_QoS_WRED_QueueCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 nQueueId = pPar->nQueueId, value, addr, data0, data1;
	u8 ColorCode;

/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;

	/* For different color 0(not drop) 1(Green) 2(Yellow) 3(Red) */
	for (ColorCode = 0; ColorCode < 4; ColorCode++) {
		addr = ((nQueueId << 3) | ColorCode);
		gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
			BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, addr);
		gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
			BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Write */
		gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
			BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0x0);
		value = 1;
		gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
				BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		gsw_r32(BM_RAM_VAL_0_VAL0_OFFSET,	\
			BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, &data0);
		gsw_r32(BM_RAM_VAL_1_VAL1_OFFSET,	\
			BM_RAM_VAL_1_VAL1_SHIFT, BM_RAM_VAL_1_VAL1_SIZE, &data1);
		switch (ColorCode) {
		case 3:
				pPar->nRed_Max = data1;	pPar->nRed_Min = data0;
			break;
		case 2:
				pPar->nYellow_Max = data1; pPar->nYellow_Min = data0;
			break;
		case 1:
				pPar->nGreen_Max = data1;	pPar->nGreen_Min = data0;
			break;
		case 0:
			break;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_WredQueueCfgSet(void *pDevCtx, IFX_ETHSW_QoS_WRED_QueueCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 nQueueId = pPar->nQueueId, value, addr, data0 = 0, data1 = 0;
	u8 ColorCode;

/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;

	/* For different color 0(not drop) 1(Green) 2(Yellow) 3(Red) */
	for (ColorCode = 0; ColorCode < 4; ColorCode++) {
		addr = (((nQueueId << 3) & 0xF8) | ColorCode);
		gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
			BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, addr);
		/* Specify the PQMCTXT = 0x9 */
		gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
			BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Write */
		gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
			BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0x1);
		switch (ColorCode) {
		case 3:
				data1 = pPar->nRed_Max;	data0 = pPar->nRed_Min;
				break;
		case 2:
				data1 = pPar->nYellow_Max; data0 = pPar->nYellow_Min;
				break;
		case 1:
				data1 = pPar->nGreen_Max;	data0 = pPar->nGreen_Min;
				break;
		case 0:
				data0 = 0; data1 = 0;
				break;
		}
		gsw_w32(BM_RAM_VAL_0_VAL0_OFFSET,	\
			BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, data0);
		gsw_w32(BM_RAM_VAL_1_VAL1_OFFSET,	\
			BM_RAM_VAL_1_VAL1_SHIFT, BM_RAM_VAL_1_VAL1_SIZE, data1);
		value = 1;
		gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
				BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
	}
	/* Set the local threshold */
	gsw_w32(BM_QUEUE_GCTRL_GL_MOD_OFFSET,	\
		BM_QUEUE_GCTRL_GL_MOD_SHIFT, BM_QUEUE_GCTRL_GL_MOD_SIZE, 0);
   return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_QoS_WredPortCfgGet(void *pDevCtx, IFX_ETHSW_QoS_WRED_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= (pEthDev->nPortNumber))
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** WRED Red Threshold Min [number of segments]. */
		gsw_r32((BM_PWRED_RTH_0_MINTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_RTH_0_MINTH_SHIFT, BM_PWRED_RTH_0_MINTH_SIZE, &pPar->nRed_Min);
		 /** WRED Red Threshold Max [number of segments]. */
		gsw_r32((BM_PWRED_RTH_1_MAXTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_RTH_1_MAXTH_SHIFT, BM_PWRED_RTH_1_MAXTH_SIZE, &pPar->nRed_Max);
		/* WRED Yellow Threshold - Minimum */
		gsw_r32((BM_PWRED_YTH_0_MINTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_YTH_0_MINTH_SHIFT, BM_PWRED_YTH_0_MINTH_SIZE, &pPar->nYellow_Min);
		/* WRED Yellow Threshold - Maximum */
		gsw_r32((BM_PWRED_YTH_1_MAXTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_YTH_1_MAXTH_SHIFT, BM_PWRED_YTH_1_MAXTH_SIZE, &pPar->nYellow_Max);
		/* WRED Green Threshold - Minimum */
		gsw_r32((BM_PWRED_GTH_0_MINTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_GTH_0_MINTH_SHIFT, BM_PWRED_GTH_0_MINTH_SIZE, &pPar->nGreen_Min);
		/* WRED Green Threshold - Maximum */
		gsw_r32((BM_PWRED_GTH_1_MAXTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_GTH_1_MAXTH_SHIFT, BM_PWRED_GTH_1_MAXTH_SIZE, &pPar->nGreen_Max);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_WredPortCfgSet(void *pDevCtx, IFX_ETHSW_QoS_WRED_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= (pEthDev->nPortNumber))
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** WRED Red Threshold Min [number of segments]. */
		gsw_w32((BM_PWRED_RTH_0_MINTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_RTH_0_MINTH_SHIFT, BM_PWRED_RTH_0_MINTH_SIZE, pPar->nRed_Min);
		 /** WRED Red Threshold Max [number of segments]. */
		gsw_w32((BM_PWRED_RTH_1_MAXTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_RTH_1_MAXTH_SHIFT, BM_PWRED_RTH_1_MAXTH_SIZE, pPar->nRed_Max);
		/* WRED Yellow Threshold - Minimum */
		gsw_w32((BM_PWRED_YTH_0_MINTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_YTH_0_MINTH_SHIFT, BM_PWRED_YTH_0_MINTH_SIZE, pPar->nYellow_Min);
		/* WRED Yellow Threshold - Maximum */
		gsw_w32((BM_PWRED_YTH_1_MAXTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_YTH_1_MAXTH_SHIFT, BM_PWRED_YTH_1_MAXTH_SIZE, pPar->nYellow_Max);
		/* WRED Green Threshold - Minimum */
		gsw_w32((BM_PWRED_GTH_0_MINTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_GTH_0_MINTH_SHIFT, BM_PWRED_GTH_0_MINTH_SIZE, pPar->nGreen_Min);
		/* WRED Green Threshold - Maximum */
		gsw_w32((BM_PWRED_GTH_1_MAXTH_OFFSET + (pPar->nPortId * 0x6)),	\
			BM_PWRED_GTH_1_MAXTH_SHIFT, BM_PWRED_GTH_1_MAXTH_SIZE, pPar->nGreen_Max);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_FlowctrlCfgGet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
			/** Global Buffer Non Conforming Flow Control Threshold Minimum [number of segments]. */
		gsw_r32((SDMA_FCTHR1_THR1_OFFSET),	\
			SDMA_FCTHR1_THR1_SHIFT, SDMA_FCTHR1_THR1_SIZE, &pPar->nFlowCtrlNonConform_Min);
		/** Global Buffer Non Conforming Flow Control Threshold Maximum [number of segments]. */
		gsw_r32((SDMA_FCTHR2_THR2_OFFSET),	\
			SDMA_FCTHR2_THR2_SHIFT, SDMA_FCTHR2_THR2_SIZE, &pPar->nFlowCtrlNonConform_Max);
		/** Global Buffer Conforming Flow Control Threshold Minimum [number of segments]. */
		gsw_r32((SDMA_FCTHR3_THR3_OFFSET),	\
			SDMA_FCTHR3_THR3_SHIFT, SDMA_FCTHR3_THR3_SIZE, &pPar->nFlowCtrlConform_Min);
		/** Global Buffer Conforming Flow Control Threshold Maximum [number of segments]. */
		gsw_r32((SDMA_FCTHR4_THR4_OFFSET),	\
			SDMA_FCTHR4_THR4_SHIFT, SDMA_FCTHR4_THR4_SIZE, &pPar->nFlowCtrlConform_Max);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_FlowctrlCfgSet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** Global Buffer Non Conforming Flow Control Threshold Minimum [number of segments]. */
		gsw_w32((SDMA_FCTHR1_THR1_OFFSET),	\
			SDMA_FCTHR1_THR1_SHIFT, SDMA_FCTHR1_THR1_SIZE, pPar->nFlowCtrlNonConform_Min);
		/** Global Buffer Non Conforming Flow Control Threshold Maximum [number of segments]. */
		gsw_w32((SDMA_FCTHR2_THR2_OFFSET),	\
			SDMA_FCTHR2_THR2_SHIFT, SDMA_FCTHR2_THR2_SIZE, pPar->nFlowCtrlNonConform_Max);
		/** Global Buffer Conforming Flow Control Threshold Minimum [number of segments]. */
		gsw_w32((SDMA_FCTHR3_THR3_OFFSET),	\
			SDMA_FCTHR3_THR3_SHIFT, SDMA_FCTHR3_THR3_SIZE, pPar->nFlowCtrlConform_Min);
		/** Global Buffer Conforming Flow Control Threshold Maximum [number of segments]. */
		gsw_w32((SDMA_FCTHR4_THR4_OFFSET),	\
			SDMA_FCTHR4_THR4_SHIFT, SDMA_FCTHR4_THR4_SIZE, pPar->nFlowCtrlConform_Max);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_FlowctrlPortCfgGet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlPortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= (pEthDev->nPortNumber))
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** Ingress Port occupied Buffer Flow Control Threshold Minimum [number of segments]. */
		gsw_r32((SDMA_PFCTHR8_THR8_OFFSET + (pPar->nPortId * 0x4)),	\
			SDMA_PFCTHR8_THR8_SHIFT, SDMA_PFCTHR8_THR8_SIZE, &pPar->nFlowCtrl_Min);
		/** Ingress Port occupied Buffer Flow Control Threshold Maximum [number of segments]. */
		gsw_r32((SDMA_PFCTHR9_THR9_OFFSET + (pPar->nPortId * 0x4)),	\
			SDMA_PFCTHR9_THR9_SHIFT, SDMA_PFCTHR9_THR9_SIZE, &pPar->nFlowCtrl_Max);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_FlowctrlPortCfgSet(void *pDevCtx, IFX_ETHSW_QoS_FlowCtrlPortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= (pEthDev->nPortNumber))
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/** Ingress Port occupied Buffer Flow Control Threshold Minimum [number of segments]. */
		gsw_w32((SDMA_PFCTHR8_THR8_OFFSET + (pPar->nPortId * 0x4)),	\
			SDMA_PFCTHR8_THR8_SHIFT, SDMA_PFCTHR8_THR8_SIZE, pPar->nFlowCtrl_Min);
		/** Ingress Port occupied Buffer Flow Control Threshold Maximum [number of segments]. */
		gsw_w32((SDMA_PFCTHR9_THR9_OFFSET + (pPar->nPortId * 0x4)),	\
			SDMA_PFCTHR9_THR9_SHIFT, SDMA_PFCTHR9_THR9_SIZE, pPar->nFlowCtrl_Max);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_QueueBufferReserveCfgGet(void *pDevCtx, IFX_ETHSW_QoS_QueueBufferReserveCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32  value, addr;

/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value); */
	if (pPar->nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
			/* Colourcode = 0  and fixed offset = 0 */
			/* Set the BM RAM ADDR  */
		addr = (((pPar->nQueueId << 3) & 0xF8));
		gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
			BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, addr);
		/* Specify the PQMCTXT = 0x9 */
		gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
			BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Read */
		gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
			BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0x0);
		value = 1;
		/* Active */
		gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
				BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);

		gsw_r32(BM_RAM_VAL_0_VAL0_OFFSET,	\
			BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, &pPar->nBufferReserved);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}

ethsw_ret_t IFX_FLOW_QoS_QueueBufferReserveCfgSet(void *pDevCtx, IFX_ETHSW_QoS_QueueBufferReserveCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32  value, addr;

/*	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
		ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value); */

	if (pPar->nQueueId >= pEthDev->num_of_queues)
		return LTQ_ERROR;
	/* Supported for GSWIP 2.2 and newer and returns with an error for older hardware revisions. */
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/* Colourcode = 0  and fixed offset = 0 */
		/* Set the BM RAM ADDR  */
		addr = (((pPar->nQueueId << 3) & 0xF8));
		gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
			BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, addr);
		gsw_w32(BM_RAM_VAL_0_VAL0_OFFSET,	\
			BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, pPar->nBufferReserved);
		/* Specify the PQMCTXT = 0x9 */
		gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
			BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, 0x9);
		/* Table Access Operation Mode Write */
		gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
			BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0x1);
		value = 1;
		/* Active */
		gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
				BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		return LTQ_SUCCESS;
	} else {
		return LTQ_ERROR;
	}
}
#endif  /* CONFIG_LTQ_QOS */
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
ethsw_ret_t IFX_FLOW_MulticastRouterPortAdd(void *pDevCtx, IFX_ETHSW_multicastRouter_t *pPar)
{
	ethsw_api_dev_t	*pEthDev = (ethsw_api_dev_t *)pDevCtx;
	gsw_igmp_t	*pIGMPTbHandle	= &pEthDev->IGMP_Flags;
	u32 value;
	u8 portIdx = pPar->nPortId;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	/* Read the Default Router Port Map - DRPM */
	gsw_r32(PCE_IGMP_DRPM_DRPM_OFFSET,	\
		PCE_IGMP_DRPM_DRPM_SHIFT, PCE_IGMP_DRPM_DRPM_SIZE, &value);
	if (((value >> portIdx) & 0x1) == 1) {
		pr_err("Error: the prot was already in the member\n");
	} else {
		value = (value | (1 << portIdx));
		/* Write the Default Router Port Map - DRPM  */
		gsw_w32(PCE_IGMP_DRPM_DRPM_OFFSET,	\
			PCE_IGMP_DRPM_DRPM_SHIFT, PCE_IGMP_DRPM_DRPM_SIZE, value);
	}
	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD) {
		IFX_FLOW_PCE_rule_t PCE_rule;
		int i;
		for (i = 0; i < 2; i++) {
			memset(&PCE_rule, 0 , sizeof(IFX_FLOW_PCE_rule_t));
			PCE_rule.pattern.bEnable = LTQ_TRUE;
			PCE_rule.pattern.bProtocolEnable = LTQ_TRUE;
			switch (i) {
			case 0:
				/*	Management port remaining IGMP packets (forwarding them to Router Ports) */
					PCE_rule.pattern.nIndex = MULTI_PCE_RULES_INDEX;
					PCE_rule.pattern.nProtocol = 0x2; /* for IPv4 */
					PCE_rule.pattern.bAppMaskRangeMSB_Select	= LTQ_TRUE;
					PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
					PCE_rule.pattern.nAppDataMSB = 0x1200;
					PCE_rule.pattern.nAppMaskRangeMSB	= 0x1DFF;
					break;
			case 1:
					/* Management Port ICMPv6 Multicast Listerner Report & Leave (Avoiding Loopback abd Discard) */
					PCE_rule.pattern.nIndex = MULTI_PCE_RULES_INDEX + 3;
					PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
					PCE_rule.pattern.bAppMaskRangeMSB_Select = LTQ_TRUE;
					PCE_rule.pattern.nAppDataMSB = 0x8300;
					PCE_rule.pattern.nAppMaskRangeMSB	= 0x1FF;
					PCE_rule.pattern.nProtocol = 0x3A;  /*for IPv6*/
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
					PCE_rule.action.nForwardPortMap = value ;
					break;
			}
			/* Router portmap */
			PCE_rule.action.ePortMapAction	= IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
			PCE_rule.action.nForwardPortMap = value ;
			if (pIGMPTbHandle->nClassOfService != 0) {
				PCE_rule.action.eTrafficClassAction = 1;
				PCE_rule.action.nTrafficClassAlternate = pEthDev->IGMP_Flags.nClassOfService;
			}
			/*  Set eForwardPort */
			PCE_rule.pattern.bPortIdEnable	= LTQ_TRUE;
			if (pIGMPTbHandle->eForwardPort == IFX_ETHSW_PORT_FORWARD_PORT)
				PCE_rule.pattern.nPortId		= pIGMPTbHandle->nForwardPortId;
			else if (pIGMPTbHandle->eForwardPort == IFX_ETHSW_PORT_FORWARD_CPU)
				PCE_rule.pattern.nPortId		= (pEthDev->nCPU_Port);

			if (pIGMPTbHandle->bCrossVLAN)
				PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_DISABLE;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
				return LTQ_ERROR;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastRouterPortRead(void *pDevCtx, IFX_ETHSW_multicastRouterRead_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value_1, value_2;

	if (pPar->bInitial == LTQ_TRUE) {
		/* Read the Default Router Port Map - DRPM*/
		gsw_r32(PCE_IGMP_DRPM_DRPM_OFFSET,	\
			PCE_IGMP_DRPM_DRPM_SHIFT, PCE_IGMP_DRPM_DRPM_SIZE, &value_1);
		/* Read the Default Router Port Map - IGPM */
		gsw_r32(PCE_IGMP_STAT_IGPM_OFFSET,	\
			PCE_IGMP_STAT_IGPM_SHIFT, PCE_IGMP_STAT_IGPM_SIZE, &value_2);
		pEthDev->IGMP_Flags.eRouterPort = (value_1 | value_2);
		pPar->bInitial = LTQ_FALSE;
		pEthDev->routerport_cnt = 0;
	}
	if (pPar->bLast == LTQ_FALSE) {
		/* Need to clarify the different between DRPM & IGPM */
		while (((pEthDev->IGMP_Flags.eRouterPort >> pEthDev->routerport_cnt) & 0x1) == 0) {
			pEthDev->routerport_cnt++;
			if (pEthDev->routerport_cnt > (pEthDev->nTotalPortNumber-1)) {
				pPar->bLast = LTQ_TRUE;
				return LTQ_SUCCESS;
			}
		}
		pPar->nPortId = pEthDev->routerport_cnt;
		if (pEthDev->routerport_cnt < pEthDev->nTotalPortNumber)
			pEthDev->routerport_cnt++;
		else
			pPar->bLast = LTQ_TRUE;
	}
    return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastRouterPortRemove(void *pDevCtx, IFX_ETHSW_multicastRouter_t *pPar)
{
	ethsw_api_dev_t	*pEthDev = (ethsw_api_dev_t *)pDevCtx;
	gsw_igmp_t *pIGMPTbHandle	= &pEthDev->IGMP_Flags;
	u32 value_1, value_2;
	u8 portIdx = pPar->nPortId;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	/* Read the Default Router Port Map - DRPM */
	gsw_r32(PCE_IGMP_DRPM_DRPM_OFFSET,	\
		PCE_IGMP_DRPM_DRPM_SHIFT, PCE_IGMP_DRPM_DRPM_SIZE, &value_1);
	/* Read the Default Router Port Map - IGPM */
	gsw_r32(PCE_IGMP_STAT_IGPM_OFFSET,	\
		PCE_IGMP_STAT_IGPM_SHIFT, PCE_IGMP_STAT_IGPM_SIZE, &value_2);
	if (((value_1 >> portIdx) & 0x1) == 0) {
		pr_err("Error: the port was not in the member\n");
		return LTQ_SUCCESS;
	} else {
	value_1 = (value_1 & ~(1 << portIdx));
	/* Write the Default Router Port Map - DRPM*/
	gsw_w32(PCE_IGMP_DRPM_DRPM_OFFSET,	\
		PCE_IGMP_DRPM_DRPM_SHIFT, PCE_IGMP_DRPM_DRPM_SIZE, value_1);
	}
	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD) {
		IFX_FLOW_PCE_rule_t PCE_rule;
		int i;
		if (value_1) {
			for (i = 0; i < 2; i++) {
				memset(&PCE_rule, 0 , sizeof(IFX_FLOW_PCE_rule_t));
				PCE_rule.pattern.bEnable = LTQ_TRUE;
				PCE_rule.pattern.bProtocolEnable = LTQ_TRUE;
				switch (i) {
				case 0:
					/*	Management port remaining IGMP packets (forwarding them to Router Ports) */
						PCE_rule.pattern.nIndex = MULTI_PCE_RULES_INDEX;
						PCE_rule.pattern.nProtocol = 0x2; /* for IPv4 */
						PCE_rule.pattern.bAppMaskRangeMSB_Select = LTQ_TRUE;
						PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
						PCE_rule.pattern.nAppDataMSB = 0x1200;
						PCE_rule.pattern.nAppMaskRangeMSB	= 0x1DFF;
						break;
				case 1:
						/* Management Port ICMPv6 Multicast Listerner Report & Leave (Avoiding Loopback abd Discard) */
						PCE_rule.pattern.nIndex = MULTI_PCE_RULES_INDEX+3;
						PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
						PCE_rule.pattern.bAppMaskRangeMSB_Select = LTQ_TRUE;
						PCE_rule.pattern.nAppDataMSB = 0x8300;
						PCE_rule.pattern.nAppMaskRangeMSB	= 0x1FF;
						PCE_rule.pattern.nProtocol = 0x3A;  /*for IPv6*/
						break;
				}
				/* Router portmap */
				PCE_rule.action.ePortMapAction	= IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
				PCE_rule.action.nForwardPortMap = value_1 ;
				if (pIGMPTbHandle->nClassOfService != 0) {
					PCE_rule.action.eTrafficClassAction = 1;
					PCE_rule.action.nTrafficClassAlternate = pEthDev->IGMP_Flags.nClassOfService;
				}
				/*  Set eForwardPort */
				PCE_rule.pattern.bPortIdEnable	= LTQ_TRUE;
				if (pIGMPTbHandle->eForwardPort == IFX_ETHSW_PORT_FORWARD_PORT)
					PCE_rule.pattern.nPortId		= pIGMPTbHandle->nForwardPortId;
				else if (pIGMPTbHandle->eForwardPort == IFX_ETHSW_PORT_FORWARD_CPU)
					PCE_rule.pattern.nPortId		= (pEthDev->nCPU_Port);

				if (pIGMPTbHandle->bCrossVLAN)
					PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_CROSS;
				else
					PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_DISABLE;
				/* We prepare everything and write into PCE Table */
				if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
					return LTQ_ERROR;
			}
		} else {
			for (i = 0; i < 2; i++) {
				memset(&PCE_rule, 0, sizeof(IFX_FLOW_PCE_rule_t));
				switch (i) {
				case 0:
						PCE_rule.pattern.nIndex = MULTI_PCE_RULES_INDEX;
						break;
				case 1:
						/* Management Port ICMPv6 Multicast Listerner Report & Leave (Avoiding Loopback abd Discard) */
						PCE_rule.pattern.nIndex = MULTI_PCE_RULES_INDEX + 3;
						break;
					}
				if (0 != pce_pattern_delete(&pEthDev->PCE_Handler, PCE_rule.pattern.nIndex))
				return LTQ_ERROR;
			}
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastSnoopCfgGet(void *pDevCtx, IFX_ETHSW_multicastSnoopCfg_t *pPar)
{
	u32 data_1, data_2, value;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;

	pPar->eIGMP_Mode = pEthDev->IGMP_Flags.eIGMP_Mode;
	pPar->bIGMPv3 = pEthDev->IGMP_Flags.bIGMPv3;
	pPar->bCrossVLAN = pEthDev->IGMP_Flags.bCrossVLAN;
	pPar->eForwardPort = pEthDev->IGMP_Flags.eForwardPort;
	pPar->nForwardPortId = pEthDev->IGMP_Flags.nForwardPortId;
	pPar->nClassOfService = pEthDev->IGMP_Flags.nClassOfService;
	gsw_r32(PCE_IGMP_CTRL_ROB_OFFSET,	\
		PCE_IGMP_CTRL_ROB_SHIFT, PCE_IGMP_CTRL_ROB_SIZE, &value);
	pPar->nRobust = value;
	gsw_r32(PCE_IGMP_CTRL_DMRT_OFFSET,	\
		PCE_IGMP_CTRL_DMRT_SHIFT, PCE_IGMP_CTRL_DMRT_SIZE, &value);
	pPar->nQueryInterval = value;
	gsw_r32(PCE_IGMP_CTRL_REPSUP_OFFSET,	\
		PCE_IGMP_CTRL_REPSUP_SHIFT, PCE_IGMP_CTRL_REPSUP_SIZE, &data_1);
	gsw_r32(PCE_IGMP_CTRL_JASUP_OFFSET,	\
		PCE_IGMP_CTRL_JASUP_SHIFT, PCE_IGMP_CTRL_JASUP_SIZE, &data_2);
	if (data_1 == 0 && data_2 == 0)
		pPar->eSuppressionAggregation = IFX_ETHSW_MULTICAST_TRANSPARENT;
	else if (data_1 == 1 && data_2 == 0)
		pPar->eSuppressionAggregation = IFX_ETHSW_MULTICAST_REPORT;
	else if (data_1 == 1 && data_2 == 1)
		pPar->eSuppressionAggregation = IFX_ETHSW_MULTICAST_REPORT_JOIN;
	else
		pPar->eSuppressionAggregation = IFX_ETHSW_MULTICAST_TRANSPARENT;

	gsw_r32(PCE_IGMP_CTRL_FLEAVE_OFFSET,	\
		PCE_IGMP_CTRL_FLEAVE_SHIFT, PCE_IGMP_CTRL_FLEAVE_SIZE, &value);
	if (value == 1)
		pPar->bFastLeave = 1;
	else
		pPar->bFastLeave = 0;
	gsw_r32(PCE_IGMP_CTRL_SRPEN_OFFSET,	\
		PCE_IGMP_CTRL_SRPEN_SHIFT, PCE_IGMP_CTRL_SRPEN_SIZE, &value);
	pPar->bLearningRouter = value;
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		gsw_r32(PCE_GCTRL_1_UKIPMC_OFFSET,	\
			PCE_GCTRL_1_UKIPMC_SHIFT, PCE_GCTRL_1_UKIPMC_SIZE, &pPar->bMulticastUnknownDrop);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastSnoopCfgSet(void *pDevCtx, IFX_ETHSW_multicastSnoopCfg_t *pPar)
{
	u32 i, data_1 = 0, data_2 = 0, pce_table_index = MULTI_PCE_RULES_INDEX ;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	IFX_FLOW_PCE_rule_t PCE_rule;

	/* Choose IGMP Mode */
	switch (pPar->eIGMP_Mode) {
	case IFX_ETHSW_MULTICAST_SNOOP_MODE_DISABLED:
			/* Snooping of Router Port Disable */
			gsw_w32(PCE_IGMP_CTRL_SRPEN_OFFSET,	\
				PCE_IGMP_CTRL_SRPEN_SHIFT, PCE_IGMP_CTRL_SRPEN_SIZE, 0);
			gsw_w32(PCE_GCTRL_0_IGMP_OFFSET,	\
				PCE_GCTRL_0_IGMP_SHIFT, PCE_GCTRL_0_IGMP_SIZE, 0);
			for (i = 0; i <= pEthDev->nTotalPortNumber; i++) {
				gsw_w32(PCE_PCTRL_0_MCST_OFFSET + (0xA * i),	\
					PCE_PCTRL_0_MCST_SHIFT, PCE_PCTRL_0_MCST_SIZE, 0);
			}
			break;
	case IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING:
			/* Snooping of Router Port Enable */
			gsw_w32(PCE_GCTRL_0_IGMP_OFFSET,	\
				PCE_GCTRL_0_IGMP_SHIFT, PCE_GCTRL_0_IGMP_SIZE, 0);
			gsw_w32(PCE_IGMP_CTRL_SRPEN_OFFSET,	\
				PCE_IGMP_CTRL_SRPEN_SHIFT, PCE_IGMP_CTRL_SRPEN_SIZE, 1);
			for (i = 0; i <= pEthDev->nTotalPortNumber ; i++) {
				gsw_w32(PCE_PCTRL_0_MCST_OFFSET + (0xA * i),	\
					PCE_PCTRL_0_MCST_SHIFT, PCE_PCTRL_0_MCST_SIZE, 1);
			}
			break;
	case IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD:
			/* Snooping of Router Port Forward */
			gsw_w32(PCE_IGMP_CTRL_SRPEN_OFFSET,	\
				PCE_IGMP_CTRL_SRPEN_SHIFT, PCE_IGMP_CTRL_SRPEN_SIZE, 0);
			gsw_w32(PCE_GCTRL_0_IGMP_OFFSET,	\
				PCE_GCTRL_0_IGMP_SHIFT, PCE_GCTRL_0_IGMP_SIZE, 1);
			for (i = 0; i <= pEthDev->nTotalPortNumber ; i++) {
				gsw_w32(PCE_PCTRL_0_MCST_OFFSET + (0xA * i),	\
					PCE_PCTRL_0_MCST_SHIFT, PCE_PCTRL_0_MCST_SIZE, 1);
			}
			break;
	default:
			pr_err("This Mode doesn't exists\n");
			return LTQ_ERROR;
	}
	/* Set the Flag for eIGMP_Mode flag*/
	pEthDev->IGMP_Flags.eIGMP_Mode = pPar->eIGMP_Mode;
	/* Set bIGMPv3 flag*/
	pEthDev->IGMP_Flags.bIGMPv3 =  pPar->bIGMPv3;
	/* Set bCrossVLAN flag*/
	pEthDev->IGMP_Flags.bCrossVLAN = pPar->bCrossVLAN;
	/* Set eForwardPort flag */
	pEthDev->IGMP_Flags.eForwardPort = pPar->eForwardPort;
	/* Set nForwardPortId */
	if (pPar->eForwardPort == IFX_ETHSW_PORT_FORWARD_CPU)
		pEthDev->IGMP_Flags.nForwardPortId = (1 << pEthDev->nCPU_Port);
	else
		pEthDev->IGMP_Flags.nForwardPortId = pPar->nForwardPortId;
	pEthDev->IGMP_Flags.nClassOfService = pPar->nClassOfService;
	/* If IGMP mode set to AutoLearning then the following Rule have to add it */
	if (pPar->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		for (i = pce_table_index; i <= (pce_table_index + 7); i++) {
			memset(&PCE_rule, 0 , sizeof(IFX_FLOW_PCE_rule_t));
			PCE_rule.pattern.nIndex = i;
			PCE_rule.pattern.bEnable = LTQ_TRUE;
			PCE_rule.pattern.bAppDataMSB_Enable = LTQ_TRUE;
			if ((i == pce_table_index + 0) || (i == pce_table_index + 1) || (i == pce_table_index + 2))
				PCE_rule.pattern.nAppDataMSB = 0x1100;
			else if (i == pce_table_index + 3)
				PCE_rule.pattern.nAppDataMSB = 0x1200;
			else if (i == pce_table_index + 4)
				PCE_rule.pattern.nAppDataMSB = 0x1600;
			else if (i == pce_table_index + 5)
				PCE_rule.pattern.nAppDataMSB = 0x1700;
			else if (i == pce_table_index + 6)
				PCE_rule.pattern.nAppDataMSB = 0x3100;
			else if (i == pce_table_index + 7)
				PCE_rule.pattern.nAppDataMSB = 0x3000;

			PCE_rule.pattern.bAppMaskRangeMSB_Select = 0;
			PCE_rule.pattern.nAppMaskRangeMSB = 0x3;
			if ((i == pce_table_index + 0) || (i == pce_table_index + 1)	\
				 || (i == pce_table_index + 6) || (i == pce_table_index + 7))
				PCE_rule.pattern.eDstIP_Select = LTQ_TRUE;
			if ((i == pce_table_index + 0) || (i == pce_table_index + 1))
				PCE_rule.pattern.nDstIP.nIPv4 = 0xE0000001;
			else if (i == pce_table_index + 6)
				PCE_rule.pattern.nDstIP.nIPv4 = 0xE0000002;
			else if (i == pce_table_index + 7)
				PCE_rule.pattern.nDstIP.nIPv4 = 0xE00000A6;
			PCE_rule.pattern.nDstIP_Mask = 0xFF00;
			if (i == pce_table_index + 1)
				PCE_rule.pattern.eSrcIP_Select = LTQ_TRUE;
			else
				PCE_rule.pattern.eSrcIP_Select = 0;
			if (i == pce_table_index + 1)
				PCE_rule.pattern.nSrcIP_Mask = 0xFF00;
			else
				PCE_rule.pattern.nSrcIP_Mask = 0xFFFF;
			PCE_rule.pattern.bProtocolEnable = LTQ_TRUE;
			PCE_rule.pattern.nProtocol = 0x2;
			if (pEthDev->IGMP_Flags.nClassOfService == 0) {
				PCE_rule.action.eTrafficClassAction = 0;
				PCE_rule.action.nTrafficClassAlternate = 0;
			} else {
				PCE_rule.action.eTrafficClassAction = 1;
				PCE_rule.action.nTrafficClassAlternate = pEthDev->IGMP_Flags.nClassOfService;
			}
			if (i == pce_table_index + 0)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_QUERY;
			else if (i == pce_table_index + 1)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_QUERY_NO_ROUTER;
			else if (i == pce_table_index + 2)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_QUERY_GROUP;
			else if (i == pce_table_index + 3)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_REPORT;
			else if (i == pce_table_index + 4)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_REPORT;
			else if (i == pce_table_index + 5)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_LEAVE;
			else if (i == pce_table_index + 6)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_AD;
			else if (i == pce_table_index + 7)
				PCE_rule.action.eSnoopingTypeAction = IFX_FLOW_PCE_ACTION_IGMP_SNOOP_AD;
			PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_MULTICAST_ROUTER;
			if (pPar->bCrossVLAN)
				PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_DISABLE;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
				return LTQ_ERROR;
		}
	}
	/* If IGMP mode set to forwarding then the following Rule have to add it */
	if (pPar->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD) {
		for (i = pce_table_index; i <= (pce_table_index + 7); i++) {
			memset(&PCE_rule, 0 , sizeof(IFX_FLOW_PCE_rule_t));
			PCE_rule.pattern.nIndex = i;
			PCE_rule.pattern.bEnable = LTQ_TRUE;
			PCE_rule.pattern.bProtocolEnable = LTQ_TRUE;
			switch (i - pce_table_index) {
/*		case 0: */
					/*Rule added by Router port ADD function based on router port for IPv4*/
/*					break; */
			case 1:
					/*	Avoid IGMP Packets Redirection when seen on Management Port */
					PCE_rule.pattern.nProtocol = 0x2; /* for IPv4 */
					PCE_rule.pattern.bPortIdEnable = LTQ_TRUE;
					/* Action Enabled, no redirection (default portmap) */
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_REGULAR;
					break;
			case 2:
					/* IGMPv1/2/3 IPv4 */
					PCE_rule.pattern.nProtocol = 0x2; /* for IPv4 */
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
					break;
/*				case 3: */
					/*Rules added by Router port ADD function based on router port for IPv6 */
/*					break; */
			case 4:
					/*	Managemnt Port Remaining ICMPv6/MLD packets(Avoiding Loopback and Disacard) */
					PCE_rule.pattern.bPortIdEnable = LTQ_TRUE;
					PCE_rule.pattern.nPortId = pPar->nForwardPortId;
					PCE_rule.pattern.nProtocol = 0x3A;  /*for IPv6*/
					PCE_rule.pattern.bPortIdEnable = LTQ_TRUE;
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_REGULAR;
					break;
			case 5:
					/* ICMPv6 Multicast Listener Query/Report/Done(Leave) */
					PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
					PCE_rule.pattern.bAppMaskRangeMSB_Select = LTQ_TRUE;
					PCE_rule.pattern.nAppDataMSB = 0x8200;
					PCE_rule.pattern.nAppMaskRangeMSB	= 0x2FF;
					PCE_rule.pattern.nProtocol = 0x3A;  /*for IPv6*/
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
					break;
			case 6:
					/* ICMPv6 Multicast Listener Report */
					PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
					PCE_rule.pattern.nAppDataMSB = 0x8F00;
					PCE_rule.pattern.nAppMaskRangeMSB = 0x3;
					PCE_rule.pattern.nProtocol = 0x3A;  /*for IPv6*/
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
					break;
			case 7:
					/* ICMPv6 Multicast Router Advertisement/Solicitation/Termination */
					PCE_rule.pattern.bAppDataMSB_Enable	= LTQ_TRUE;
					PCE_rule.pattern.bAppMaskRangeMSB_Select = LTQ_TRUE;
					PCE_rule.pattern.nAppDataMSB = 0x9700;
					PCE_rule.pattern.nAppMaskRangeMSB	= 0x2FF;
					PCE_rule.pattern.nProtocol = 0x3A;  /*for IPv6*/
					PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
					break;
			default:
					continue;
			}
			if (pEthDev->IGMP_Flags.nClassOfService != 0) {
				PCE_rule.action.eTrafficClassAction = 1;
				PCE_rule.action.nTrafficClassAlternate = pEthDev->IGMP_Flags.nClassOfService;
			}
			/*  Set eForwardPort */
			if (pPar->eForwardPort == IFX_ETHSW_PORT_FORWARD_PORT) {
				PCE_rule.action.nForwardPortMap = (1 << pPar->nForwardPortId);
				PCE_rule.pattern.nPortId		= pPar->nForwardPortId;
			} else if (pPar->eForwardPort == IFX_ETHSW_PORT_FORWARD_CPU) {
				PCE_rule.action.nForwardPortMap = (1 << pEthDev->nCPU_Port);
				PCE_rule.pattern.nPortId = pEthDev->nCPU_Port;
			}
			if (pPar->bCrossVLAN)
				PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_CROSS;
			else
				PCE_rule.action.eVLAN_CrossAction = IFX_FLOW_PCE_ACTION_CROSS_VLAN_DISABLE;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
				return LTQ_ERROR;
		}

	}
	if (pPar->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_DISABLED) {
		pce_table_index = MULTI_PCE_RULES_INDEX;
		for (i = pce_table_index; i <= (pce_table_index + 7); i++) {
			PCE_rule.pattern.nIndex = i;
			PCE_rule.pattern.bEnable = LTQ_FALSE;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_pattern_delete(&pEthDev->PCE_Handler, i))
				return LTQ_ERROR;
		}
	}
	if (pPar->nRobust < 4) {
		gsw_w32(PCE_IGMP_CTRL_ROB_OFFSET,	\
			PCE_IGMP_CTRL_ROB_SHIFT, PCE_IGMP_CTRL_ROB_SIZE, pPar->nRobust);
	} else {
		pr_err("The Robust time would only support 0..3\n");
		return LTQ_ERROR;
	}
	gsw_w32(PCE_IGMP_CTRL_DMRTEN_OFFSET,	\
		PCE_IGMP_CTRL_DMRTEN_SHIFT, PCE_IGMP_CTRL_DMRTEN_SIZE, 1);
	gsw_w32(PCE_IGMP_CTRL_DMRT_OFFSET,	\
		PCE_IGMP_CTRL_DMRT_SHIFT, PCE_IGMP_CTRL_DMRT_SIZE, pPar->nQueryInterval);

	if (pPar->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		switch (pPar->eSuppressionAggregation) {
		case IFX_ETHSW_MULTICAST_REPORT_JOIN:
				data_2 = 1;	data_1 = 1;
				break;
		case IFX_ETHSW_MULTICAST_REPORT:
				data_2 = 0;	data_1 = 1;
				break;
		case IFX_ETHSW_MULTICAST_TRANSPARENT:
				data_2 = 0;	data_1 = 0;
				break;
		}
		gsw_w32(PCE_IGMP_CTRL_REPSUP_OFFSET,	\
			PCE_IGMP_CTRL_REPSUP_SHIFT, PCE_IGMP_CTRL_REPSUP_SIZE, data_1);
		gsw_w32(PCE_IGMP_CTRL_JASUP_OFFSET,	\
			PCE_IGMP_CTRL_JASUP_SHIFT, PCE_IGMP_CTRL_JASUP_SIZE, data_2);
	}

	if (pPar->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		gsw_w32(PCE_IGMP_CTRL_SRPEN_OFFSET,	\
			PCE_IGMP_CTRL_SRPEN_SHIFT, PCE_IGMP_CTRL_SRPEN_SIZE, pPar->bLearningRouter);
		if (pPar->bFastLeave == 1)
			gsw_w32(PCE_IGMP_CTRL_FLEAVE_OFFSET,	\
				PCE_IGMP_CTRL_FLEAVE_SHIFT, PCE_IGMP_CTRL_FLEAVE_SIZE, 1);
		else
			gsw_w32(PCE_IGMP_CTRL_FLEAVE_OFFSET,	\
				PCE_IGMP_CTRL_FLEAVE_SHIFT, PCE_IGMP_CTRL_FLEAVE_SIZE, 0);
	} else {
		gsw_w32(PCE_IGMP_CTRL_FLEAVE_OFFSET,	\
			PCE_IGMP_CTRL_FLEAVE_SHIFT, PCE_IGMP_CTRL_FLEAVE_SIZE, 0);
	}
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		gsw_w32(PCE_GCTRL_1_UKIPMC_OFFSET,	\
			PCE_GCTRL_1_UKIPMC_SHIFT, PCE_GCTRL_1_UKIPMC_SIZE, pPar->bMulticastUnknownDrop);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastTableEntryAdd(void *pDevCtx, IFX_ETHSW_multicastTable_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	gsw_igmp_t	*pIGMPTbHandle	= &pEthDev->IGMP_Flags;
	u8 portIdx = pPar->nPortId;
	pce_table_prog_t pData;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	memset(&pData, 0 , sizeof(pce_table_prog_t));
	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		u32 index = 0, i, available = 0;
		if ((pEthDev->IGMP_Flags.bIGMPv3 == 1) || (pPar->eIPVersion == IFX_ETHSW_IP_SELECT_IPV6))
			return LTQ_ERROR;
		/* Read Out all of the HW Table */
		for (i = 0; i < MULTI_HW_TABLE_SIZE; i++) {
			pData.table = PCE_MULTICAST_HW_INDEX;
			pData.table_index = i;
			xwayflow_pce_table_read(pEthDev, &pData);
			if (pData.valid) {
				if ((pData.key[0] == (pPar->uIP_Gda.nIPv4 & 0xFFFF))	\
					&&  (pData.key[1] == ((pPar->uIP_Gda.nIPv4 >> 16) & 0xFFFF))) {
					index = i;
					available = 1;
					break;
				}
			}
		}
		pData.table = PCE_MULTICAST_HW_INDEX;
		if (available == 0) {
			index = MULTI_HW_TABLE_SIZE;
			for (i = 0; i < MULTI_HW_TABLE_SIZE; i++) {
				pData.table_index = i;
				xwayflow_pce_table_read(pEthDev, &pData);
				if (pData.valid == 0) {
					index = i;  /* Free index */
					break;
				}
			}
		}
		if (index < MULTI_HW_TABLE_SIZE) {
			pData.table			= PCE_MULTICAST_HW_INDEX;
			pData.table_index	= index;
			pData.key[1] = ((pPar->uIP_Gda.nIPv4 >> 16) & 0xFFFF);
			pData.key[0] = (pPar->uIP_Gda.nIPv4 & 0xFFFF);
			pData.val[0] |= (1 << portIdx);
			pData.val[4] |= (1 << 14);
			pData.valid = 1;
			xwayflow_pce_table_write(pEthDev, &pData);
		} else {
			pr_err("Error: (IGMP HW Table is full) %s:%s:%d \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}

	} else if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD) {
		/* Program the Multicast SW Table */
		ifx_multicast_sw_table_write(pEthDev, pPar);
	} else {
		/* Disable All Multicast SW Table */
		pr_err("Please Select the IGMP Mode through Multicast Snooping Configuration API\n");
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastTableEntryRead(void *pDevCtx, IFX_ETHSW_multicastTableRead_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	gsw_igmp_t	*pIGMPTbHandle   = &pEthDev->IGMP_Flags;
	pce_table_prog_t pData;

	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_DISABLED) {
		pr_err("Error: (IGMP snoop is not enabled) %s:%s:%d \n", __FILE__, __func__, __LINE__);
		return LTQ_ERROR;
	}
	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		if (pPar->bInitial == LTQ_TRUE) {
			pEthDev->multi_hw_table_index = 0; /*Start from the index 0 */
			pPar->bInitial = LTQ_FALSE;
		}
		if (pEthDev->multi_hw_table_index >= MULTI_HW_TABLE_SIZE) {
			memset(pPar, 0, sizeof(IFX_ETHSW_multicastTableRead_t));
			pPar->bLast = LTQ_TRUE;
			pEthDev->multi_hw_table_index = 0;
			return LTQ_SUCCESS;
		}

		do {
			memset(&pData, 0 , sizeof(pce_table_prog_t));
			pData.table = PCE_MULTICAST_HW_INDEX;
			pData.table_index = pEthDev->multi_hw_table_index;
			xwayflow_pce_table_read(pDevCtx, &pData);
			pEthDev->multi_hw_table_index++;
			if (pData.valid != 0)
				break;
		} while (pEthDev->multi_hw_table_index < MULTI_HW_TABLE_SIZE);
		if (pData.valid != 0) {
			pPar->nPortId 		= pData.val[0] | 0x80000000;
			pPar->uIP_Gda.nIPv4 = ((pData.key[1] << 16) | pData.key[0]);
			pPar->uIP_Gsa.nIPv4 = 0;
			pPar->eModeMember 	= IFX_ETHSW_IGMP_MEMBER_DONT_CARE;
			pPar->eIPVersion 	= IFX_ETHSW_IP_SELECT_IPV4;
			pPar->bInitial = LTQ_FALSE;
			pPar->bLast = LTQ_FALSE;
		} else {
			memset(pPar, 0, sizeof(IFX_ETHSW_multicastTableRead_t));
			pPar->bLast = LTQ_TRUE;
			pEthDev->multi_hw_table_index = 0;
		}
	}
	/*Snooping in Forward mode */
	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD) {
		u32 Dest_lsb_index, Src_lsb_index, Dest_msb_index, Src_msb_index;
		if (pPar->bInitial == LTQ_TRUE) {
			pEthDev->multi_sw_table_index = 0; /*Start from the index 0 */
			pPar->bInitial = LTQ_FALSE;
		}
		if (pEthDev->multi_sw_table_index >= MULTI_SW_TABLE_SIZE) {
			memset(pPar, 0, sizeof(IFX_ETHSW_multicastTableRead_t));
			pPar->bLast = LTQ_TRUE;
			pEthDev->multi_sw_table_index = 0;
			return LTQ_SUCCESS;
		}

		do {
			memset(&pData, 0 , sizeof(pce_table_prog_t));
			pData.table = PCE_MULTICAST_SW_INDEX;
			pData.table_index = pEthDev->multi_sw_table_index;
			xwayflow_pce_table_read(pDevCtx, &pData);
			pEthDev->multi_sw_table_index++;
			if (pData.valid != 0)
				break;
		} while (pEthDev->multi_sw_table_index < MULTI_SW_TABLE_SIZE);
		if (pData.valid == 1) {
			pce_table_prog_t pData_IP_Table;
			pPar->nPortId 	= pData.val[0] | 0x80000000;
			Dest_lsb_index	= pData.key[0] & 0xFF;
			Dest_msb_index	= (pData.key[0] >> 8) & 0xFF;
			Src_lsb_index	= pData.key[1] & 0xFF;
			Src_msb_index	= (pData.key[1] >> 8) & 0xFF;
			if (Dest_lsb_index <= 0x3F) {
				memset(&pData_IP_Table, 0 , sizeof(pce_table_prog_t));
				pData_IP_Table.table = PCE_IP_DASA_LSB_INDEX;
				/* Search the DIP */
				pData_IP_Table.table_index = Dest_lsb_index;
				xwayflow_pce_table_read(pDevCtx, &pData_IP_Table);
				if (pData_IP_Table.valid == 1) {
					if (pData_IP_Table.mask  == 0xFF00) {
						pPar->uIP_Gda.nIPv4 = (pData_IP_Table.key[1] << 16) | (pData_IP_Table.key[0] & 0xFFFF);
						pPar->eIPVersion 	= IFX_ETHSW_IP_SELECT_IPV4;
					} else if (pData_IP_Table.mask  == 0x0) {
						pPar->uIP_Gda.nIPv6[4] = (pData_IP_Table.key[3] & 0xFFFF);
						pPar->uIP_Gda.nIPv6[5] = (pData_IP_Table.key[2] & 0xFFFF);
						pPar->uIP_Gda.nIPv6[6] = (pData_IP_Table.key[1] & 0xFFFF);
						pPar->uIP_Gda.nIPv6[7] = (pData_IP_Table.key[0] & 0xFFFF);
						pPar->eIPVersion 	= IFX_ETHSW_IP_SELECT_IPV6;
					}
				}
			}
			if (Src_lsb_index <= 0x3F) {
				memset(&pData_IP_Table, 0 , sizeof(pce_table_prog_t));
				pData_IP_Table.table = PCE_IP_DASA_LSB_INDEX;
				/* Search the SIP */
				pData_IP_Table.table_index = Src_lsb_index;
				xwayflow_pce_table_read(pDevCtx, &pData_IP_Table);
				if (pData_IP_Table.valid == 1) {
					if (pData_IP_Table.mask  == 0xFF00) {
						pPar->uIP_Gsa.nIPv4 = (pData_IP_Table.key[1] << 16) | (pData_IP_Table.key[0] & 0xFFFF);
						pPar->eIPVersion 	= IFX_ETHSW_IP_SELECT_IPV4;
					} else if (pData_IP_Table.mask  == 0x0) {
						pPar->uIP_Gsa.nIPv6[4] = (pData_IP_Table.key[3] & 0xFFFF);
						pPar->uIP_Gsa.nIPv6[5] = (pData_IP_Table.key[2] & 0xFFFF);
						pPar->uIP_Gsa.nIPv6[6] = (pData_IP_Table.key[1] & 0xFFFF);
						pPar->uIP_Gsa.nIPv6[7] = (pData_IP_Table.key[0] & 0xFFFF);
					}
				}
			}
			if (Dest_msb_index <= 0xF) {
				memset(&pData_IP_Table, 0 , sizeof(pce_table_prog_t));
				pData_IP_Table.table = PCE_IP_DASA_MSB_INDEX;
				/* Search the DIP */
				pData_IP_Table.table_index = Dest_msb_index;
				xwayflow_pce_table_read(pDevCtx, &pData_IP_Table);
				if (pData_IP_Table.valid == 1) {
					if (pData_IP_Table.mask  == 0) {
						pPar->uIP_Gda.nIPv6[0] = (pData_IP_Table.key[3] & 0xFFFF);
						pPar->uIP_Gda.nIPv6[1] = (pData_IP_Table.key[2] & 0xFFFF);
						pPar->uIP_Gda.nIPv6[2] = (pData_IP_Table.key[1] & 0xFFFF);
						pPar->uIP_Gda.nIPv6[3] = (pData_IP_Table.key[0] & 0xFFFF);
					}
				}
			}
			if (Src_msb_index <= 0xF) {
				memset(&pData_IP_Table, 0 , sizeof(pce_table_prog_t));
				pData_IP_Table.table = PCE_IP_DASA_MSB_INDEX;
				/* Search the DIP */
				pData_IP_Table.table_index = Src_msb_index;
				xwayflow_pce_table_read(pDevCtx, &pData_IP_Table);
				if (pData_IP_Table.valid == 1) {
					if (pData_IP_Table.mask  == 0) {
						pPar->uIP_Gsa.nIPv6[0] = (pData_IP_Table.key[3] & 0xFFFF);
						pPar->uIP_Gsa.nIPv6[1] = (pData_IP_Table.key[2] & 0xFFFF);
						pPar->uIP_Gsa.nIPv6[2] = (pData_IP_Table.key[1] & 0xFFFF);
						pPar->uIP_Gsa.nIPv6[3] = (pData_IP_Table.key[0] & 0xFFFF);
					}
				}
			}
			pPar->eModeMember = pIGMPTbHandle->multi_sw_table[pEthDev->multi_sw_table_index-1].eModeMember;
			pPar->bInitial = LTQ_FALSE;
			pPar->bLast = LTQ_FALSE;
		} else {

			memset(pPar, 0, sizeof(IFX_ETHSW_multicastTableRead_t));
			pPar->bLast = LTQ_TRUE;
			pEthDev->multi_sw_table_index = 0;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MulticastTableEntryRemove(void *pDevCtx, IFX_ETHSW_multicastTable_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	gsw_igmp_t	*pIGMPTbHandle = &pEthDev->IGMP_Flags;
	u8 portIdx = pPar->nPortId;
	pce_table_prog_t pData;
	ltq_bool_t DeletFlag = LTQ_FALSE;
	u32 port = 0, i;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	memset(&pData, 0 , sizeof(pce_table_prog_t));
	if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_AUTOLEARNING) {
		if (pEthDev->IGMP_Flags.bIGMPv3 == 1)
			return LTQ_ERROR;
		/* Read Out all of the HW Table */
		for (i = 0; i < MULTI_HW_TABLE_SIZE; i++) {
			memset(&pData, 0 , sizeof(pce_table_prog_t));
			pData.table = PCE_MULTICAST_HW_INDEX;
			pData.table_index = i;
			xwayflow_pce_table_read(pEthDev, &pData);
			/* Fill into Structure */
			if (((pData.val[0] >> portIdx) & 0x1) == 1) {
				if (pPar->uIP_Gda.nIPv4 == ((pData.key[1] << 16) | (pData.key[0] & 0xFFFF))) {
					port = (pData.val[0] & (~(1 << portIdx)));
					if (port == 0) {
						pData.val[0] = 0;
						pData.key[1] = 0;
						pData.val[4] = 0;
					} else {
						pData.val[0] &= ~(1 << portIdx);
					}
					DeletFlag = LTQ_TRUE;
					xwayflow_pce_table_write(pDevCtx, &pData);
				}
			}
		}
		if (DeletFlag == LTQ_FALSE)
			pr_err("The input did not found \n");
	} else if (pIGMPTbHandle->eIGMP_Mode == IFX_ETHSW_MULTICAST_SNOOP_MODE_FORWARD) {
		/* Program the Multicast SW Table */
		ifx_multicast_sw_table_remove(pEthDev, pPar);
	}
	return LTQ_SUCCESS;
}
#endif /*CONFIG_LTQ_MULTICAST*/
ethsw_ret_t IFX_FLOW_CapGet(void *pDevCtx, IFX_ETHSW_cap_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value, data1, data2;

	if (pPar->nCapType >= IFX_ETHSW_CAP_TYPE_LAST)
		return LTQ_ERROR;
	else
		strcpy(pPar->cDesc, CAP_Description[pPar->nCapType].Desci);
	/* As request, attached the code in the next version*/
	switch (pPar->nCapType) {
	case IFX_ETHSW_CAP_TYPE_PORT:
			gsw_r32(ETHSW_CAP_1_PPORTS_OFFSET,	\
				ETHSW_CAP_1_PPORTS_SHIFT, ETHSW_CAP_1_PPORTS_SIZE, &value);
			pPar->nCap = value;
			break;
	case IFX_ETHSW_CAP_TYPE_VIRTUAL_PORT:
			gsw_r32(ETHSW_CAP_1_VPORTS_OFFSET,	\
				ETHSW_CAP_1_VPORTS_SHIFT, ETHSW_CAP_1_VPORTS_SIZE, &value);
			pPar->nCap = value;
			break;
	case IFX_ETHSW_CAP_TYPE_BUFFER_SIZE:
			gsw_r32(ETHSW_CAP_11_BSIZEL_OFFSET,	\
				ETHSW_CAP_11_BSIZEL_SHIFT, ETHSW_CAP_11_BSIZEL_SIZE, &data1);
			gsw_r32(ETHSW_CAP_12_BSIZEH_OFFSET,	\
				ETHSW_CAP_12_BSIZEH_SHIFT, ETHSW_CAP_12_BSIZEH_SIZE, &data2);
			pPar->nCap = (data2 << 16 | data1);
			break;
	case IFX_ETHSW_CAP_TYPE_SEGMENT_SIZE:
			/* This is Hard coded */
			pPar->nCap = LTQ_SOC_CAP_SEGMENT;
			break;
	case IFX_ETHSW_CAP_TYPE_PRIORITY_QUEUE:
/*			gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	\
				ETHSW_CAP_1_QUEUE_SHIFT, ETHSW_CAP_1_QUEUE_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->num_of_queues;
			break;
	case IFX_ETHSW_CAP_TYPE_METER:
/*			gsw_r32(ETHSW_CAP_3_METERS_OFFSET,	\
				ETHSW_CAP_3_METERS_SHIFT, ETHSW_CAP_3_METERS_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->num_of_meters;
			break;
	case IFX_ETHSW_CAP_TYPE_RATE_SHAPER:
/*			gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET,	\
				ETHSW_CAP_3_SHAPERS_SHIFT, ETHSW_CAP_3_SHAPERS_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->num_of_shapers;
			break;
	case IFX_ETHSW_CAP_TYPE_VLAN_GROUP:
/*			gsw_r32(ETHSW_CAP_4_VLAN_OFFSET,	\
				ETHSW_CAP_4_VLAN_SHIFT, ETHSW_CAP_4_VLAN_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->avlan_table_size;
			break;
	case IFX_ETHSW_CAP_TYPE_FID:
			/* This is Hard coded */
			pPar->nCap = VRX_PLATFORM_CAP_FID;
			break;
	case IFX_ETHSW_CAP_TYPE_MAC_TABLE_SIZE:
/*			gsw_r32(ETHSW_CAP_10_MACBT_OFFSET,	\
				ETHSW_CAP_10_MACBT_SHIFT, ETHSW_CAP_10_MACBT_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->mac_table_size;
			break;
	case IFX_ETHSW_CAP_TYPE_MULTICAST_TABLE_SIZE:
/*			gsw_r32(ETHSW_CAP_8_MCAST_OFFSET,
				ETHSW_CAP_8_MCAST_SHIFT, ETHSW_CAP_8_MCAST_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->multi_table_size;
			break;
	case IFX_ETHSW_CAP_TYPE_PPPOE_SESSION:
/*			gsw_r32(ETHSW_CAP_4_PPPOE_OFFSET,	\
				ETHSW_CAP_4_PPPOE_SHIFT, ETHSW_CAP_4_PPPOE_SIZE, &value);
			pPar->nCap = value; */
			pPar->nCap = pEthDev->num_of_pppoe;
			break;
	case IFX_ETHSW_CAP_TYPE_LAST:
			pPar->nCap = 12;
			break;
	default:
			pPar->nCap = 0;
			break;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_CfgGet(void *pDevCtx, IFX_ETHSW_cfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value, data2;

	/* Aging Counter Mantissa Value */
	gsw_r32(PCE_AGE_1_MANT_OFFSET,	\
		PCE_AGE_1_MANT_SHIFT, PCE_AGE_1_MANT_SIZE, &data2);
   if (data2 == AGETIMER_1_DAY)
      pPar->eMAC_TableAgeTimer = IFX_ETHSW_AGETIMER_1_DAY;
   else if (data2 == AGETIMER_1_HOUR)
      pPar->eMAC_TableAgeTimer = IFX_ETHSW_AGETIMER_1_HOUR;
   else if (data2 == AGETIMER_300_SEC)
      pPar->eMAC_TableAgeTimer = IFX_ETHSW_AGETIMER_300_SEC;
   else if (data2 == AGETIMER_10_SEC)
      pPar->eMAC_TableAgeTimer = IFX_ETHSW_AGETIMER_10_SEC;
   else if (data2 == AGETIMER_1_SEC)
      pPar->eMAC_TableAgeTimer = IFX_ETHSW_AGETIMER_1_SEC;
   else
      pPar->eMAC_TableAgeTimer = 0;

	gsw_r32(MAC_FLEN_LEN_OFFSET,	\
		MAC_FLEN_LEN_SHIFT, MAC_FLEN_LEN_SIZE, &value);
	pPar->nMaxPacketLen = value;
	gsw_r32(MAC_PFAD_CFG_SAMOD_OFFSET,	\
		MAC_PFAD_CFG_SAMOD_SHIFT, MAC_PFAD_CFG_SAMOD_SIZE, &value);
	pPar->bPauseMAC_ModeSrc = value;
	gsw_r32(PCE_GCTRL_0_VLAN_OFFSET,	\
		PCE_GCTRL_0_VLAN_SHIFT, PCE_GCTRL_0_VLAN_SIZE, &value);
	pPar->bVLAN_Aware = value;
	/* MAC Address Learning Limitation Mode */
	gsw_r32(PCE_GCTRL_0_PLIMMOD_OFFSET ,	\
		PCE_GCTRL_0_PLIMMOD_SHIFT, PCE_GCTRL_0_PLIMMOD_SIZE, &value);
	pPar->bLearningLimitAction = value;

	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
	/*Accept or discard MAC spoofing and port MAC locking violation packets */
		gsw_r32(PCE_GCTRL_0_PLCKMOD_OFFSET ,	\
			PCE_GCTRL_0_PLCKMOD_SHIFT, PCE_GCTRL_0_PLCKMOD_SIZE, &pPar->bMAC_SpoofingAction);
	}
	gsw_r32(MAC_PFSA_0_PFAD_OFFSET,	\
		MAC_PFSA_0_PFAD_SHIFT, MAC_PFSA_0_PFAD_SIZE, &value);
	pPar->nPauseMAC_Src[5] = value & 0xFF;
	pPar->nPauseMAC_Src[4] = (value >> 8) & 0xFF;
	gsw_r32(MAC_PFSA_1_PFAD_OFFSET,	\
		MAC_PFSA_1_PFAD_SHIFT, MAC_PFSA_1_PFAD_SIZE, &value);
	pPar->nPauseMAC_Src[3] = value & 0xFF;
	pPar->nPauseMAC_Src[2] = (value >> 8) & 0xFF;
	gsw_r32(MAC_PFSA_2_PFAD_OFFSET,	\
		MAC_PFSA_2_PFAD_SHIFT, MAC_PFSA_2_PFAD_SIZE, &value);
	pPar->nPauseMAC_Src[1] = value & 0xFF;
	pPar->nPauseMAC_Src[0] = (value >> 8) & 0xFF;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_CfgSet(void *pDevCtx, IFX_ETHSW_cfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 MANT = 0, EXP = 0, value, i;

	switch (pPar->eMAC_TableAgeTimer) {
	case IFX_ETHSW_AGETIMER_1_SEC:
			MANT = AGETIMER_1_SEC; EXP = 0x2;
			pEthDev->MAC_AgeTimer = 1;
			break;
	case IFX_ETHSW_AGETIMER_10_SEC:
			MANT = AGETIMER_10_SEC; EXP = 0x2;
			pEthDev->MAC_AgeTimer = 10;
			break;
	case IFX_ETHSW_AGETIMER_300_SEC:
			MANT = AGETIMER_300_SEC; EXP = 0x2;
			pEthDev->MAC_AgeTimer = 300;
			break;
	case IFX_ETHSW_AGETIMER_1_HOUR:
			MANT = AGETIMER_1_HOUR; EXP = 0x6;
			pEthDev->MAC_AgeTimer = 3600;
			break;
	case IFX_ETHSW_AGETIMER_1_DAY:
			MANT = AGETIMER_1_DAY; EXP = 0xA;
			pEthDev->MAC_AgeTimer = 86400;
			break;
	default:
			MANT = AGETIMER_300_SEC; EXP = 0x2;
			pEthDev->MAC_AgeTimer = 300;
	}

	/* Aging Counter Exponent Value */
	gsw_w32(PCE_AGE_0_EXP_OFFSET,	\
		PCE_AGE_0_EXP_SHIFT, PCE_AGE_0_EXP_SIZE, EXP);
	/* Aging Counter Mantissa Value */
	gsw_w32(PCE_AGE_1_MANT_OFFSET,	\
		PCE_AGE_1_MANT_SHIFT, PCE_AGE_1_MANT_SIZE, MANT);
	/* Maximum Ethernet packet length */
	if (pPar->nMaxPacketLen < 0xFFFF)
		value = pPar->nMaxPacketLen;
	else
		value = MAX_PACKET_LENGTH;
	gsw_w32(MAC_FLEN_LEN_OFFSET,	\
		MAC_FLEN_LEN_SHIFT, MAC_FLEN_LEN_SIZE, value);
	if (pPar->nMaxPacketLen > 0x5EE) {
		for (i = 0; i < 6; i++) {
			gsw_w32((MAC_CTRL_2_MLEN_OFFSET + (i * 0xC)),	\
				MAC_CTRL_2_MLEN_SHIFT, MAC_CTRL_2_MLEN_SIZE, 1);
		}
	}
	/* MAC Address Learning Limitation Mode */
	gsw_w32(PCE_GCTRL_0_PLIMMOD_OFFSET ,	\
		PCE_GCTRL_0_PLIMMOD_SHIFT, PCE_GCTRL_0_PLIMMOD_SIZE, pPar->bLearningLimitAction);
	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_2) {
		/*Accept or discard MAC spoofing and port MAC locking violation packets */
		gsw_w32(PCE_GCTRL_0_PLCKMOD_OFFSET ,	\
			PCE_GCTRL_0_PLCKMOD_SHIFT, PCE_GCTRL_0_PLCKMOD_SIZE, pPar->bMAC_SpoofingAction);
	}
	/* VLAN-aware Switching           */
	gsw_w32(PCE_GCTRL_0_VLAN_OFFSET,	\
		PCE_GCTRL_0_VLAN_SHIFT, PCE_GCTRL_0_VLAN_SIZE, pPar->bVLAN_Aware);
	/* MAC Source Address Mode */
	if (pPar->bPauseMAC_ModeSrc == LTQ_TRUE) {
		gsw_w32(MAC_PFAD_CFG_SAMOD_OFFSET,	\
			MAC_PFAD_CFG_SAMOD_SHIFT, MAC_PFAD_CFG_SAMOD_SIZE, pPar->bPauseMAC_ModeSrc);
		value = pPar->nPauseMAC_Src[4] << 8 | pPar->nPauseMAC_Src[5];
		gsw_w32(MAC_PFSA_0_PFAD_OFFSET,	\
			MAC_PFSA_0_PFAD_SHIFT, MAC_PFSA_0_PFAD_SIZE, value);
		value = pPar->nPauseMAC_Src[2] << 8 | pPar->nPauseMAC_Src[3];
		gsw_w32(MAC_PFSA_1_PFAD_OFFSET,	\
			MAC_PFSA_1_PFAD_SHIFT, MAC_PFSA_1_PFAD_SIZE, value);
		value = pPar->nPauseMAC_Src[0] << 8 | pPar->nPauseMAC_Src[1];
		gsw_w32(MAC_PFSA_2_PFAD_OFFSET,	\
			MAC_PFSA_2_PFAD_SHIFT, MAC_PFSA_2_PFAD_SIZE, value);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_HW_Init(void *pDevCtx, IFX_ETHSW_HW_Init_t *pPar)
{
	ethsw_api_dev_t *pDev = (ethsw_api_dev_t *)pDevCtx;
	u32 reg_val, j;
/*	platform_device_init(pDev); */
	pDev->bHW_InitCalled = LTQ_TRUE;
	/* Software Table Init */
#if defined(CONFIG_LTQ_VLAN) && CONFIG_LTQ_VLAN
	reset_vlan_sw_table(pDev);
#endif /*CONFIG_LTQ_VLAN */
	ltq_ethsw_port_cfg_init(pDev);
#if defined(CONFIG_LTQ_MULTICAST) && CONFIG_LTQ_MULTICAST
	reset_multicast_sw_table(pDev);
#endif /*CONFIG_LTQ_MULTICAST*/
	pce_table_init(&pDev->PCE_Handler);
	/* HW Init */
	ethsw_pce_micro_code_init(pDev);
	/* Configure the MDIO Clock 97.6 Khz */
	gsw_w32((MDC_CFG_1_FREQ_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_1_FREQ_SHIFT, MDC_CFG_1_FREQ_SIZE, 0xFF);
	for (j = 0; j < pDev->nPortNumber-1; j++) {
		gsw_w32(((ANEG_EEE_0_CLK_STOP_CAPABLE_OFFSET+j) + LTQ_SOC_TOP_REG_OFFSET),	\
		ANEG_EEE_0_CLK_STOP_CAPABLE_SHIFT, ANEG_EEE_0_CLK_STOP_CAPABLE_SIZE, 0x3);
	}
	gsw_r32(ETHSW_CAP_1_QUEUE_OFFSET,	ETHSW_CAP_1_QUEUE_SHIFT,	\
		ETHSW_CAP_1_QUEUE_SIZE, &reg_val);
	pDev->num_of_queues = reg_val;
	gsw_r32(ETHSW_CAP_3_METERS_OFFSET,	ETHSW_CAP_3_METERS_SHIFT,	\
		ETHSW_CAP_3_METERS_SIZE, &reg_val);
	pDev->num_of_meters = reg_val;
	gsw_r32(ETHSW_CAP_3_SHAPERS_OFFSET, ETHSW_CAP_3_SHAPERS_SHIFT,	\
		ETHSW_CAP_3_SHAPERS_SIZE, &reg_val);
	pDev->num_of_shapers = reg_val;
	gsw_r32(ETHSW_CAP_4_PPPOE_OFFSET, ETHSW_CAP_4_PPPOE_SHIFT,	\
		ETHSW_CAP_4_PPPOE_SIZE, &reg_val);
	pDev->num_of_pppoe = reg_val;
	gsw_r32(ETHSW_CAP_4_VLAN_OFFSET, ETHSW_CAP_4_VLAN_SHIFT,	\
		ETHSW_CAP_4_VLAN_SIZE, &reg_val);
	pDev->avlan_table_size = reg_val;
	gsw_r32(ETHSW_CAP_5_IPPLEN_OFFSET, ETHSW_CAP_5_IPPLEN_SHIFT,	\
		ETHSW_CAP_5_IPPLEN_SIZE, &reg_val);
	pDev->ip_pkt_lnt_size = reg_val;
	gsw_r32(ETHSW_CAP_5_PROT_OFFSET, ETHSW_CAP_5_PROT_SHIFT,	\
		ETHSW_CAP_5_PROT_SIZE, &reg_val);
	pDev->prot_table_size = reg_val;
	gsw_r32(ETHSW_CAP_6_MACDASA_OFFSET, ETHSW_CAP_6_MACDASA_SHIFT,	\
		ETHSW_CAP_6_MACDASA_SIZE, &reg_val);
	pDev->mac_dasa_table_size = reg_val;
	gsw_r32(ETHSW_CAP_6_APPL_OFFSET, ETHSW_CAP_6_APPL_SHIFT,	\
		ETHSW_CAP_6_APPL_SIZE, &reg_val);
	pDev->app_table_size = reg_val;
	gsw_r32(ETHSW_CAP_7_IPDASAM_OFFSET, ETHSW_CAP_7_IPDASAM_SHIFT,	\
		ETHSW_CAP_7_IPDASAM_SIZE, &reg_val);
	pDev->ip_dasa_msb_table_size = reg_val;
	gsw_r32(ETHSW_CAP_7_IPDASAL_OFFSET, ETHSW_CAP_7_IPDASAL_SHIFT,	\
		ETHSW_CAP_7_IPDASAL_SIZE, &reg_val);
	pDev->ip_dasa_lsb_table_size = reg_val;
	gsw_r32(ETHSW_CAP_8_MCAST_OFFSET, ETHSW_CAP_8_MCAST_SHIFT,	\
		ETHSW_CAP_8_MCAST_SIZE, &reg_val);
	pDev->multi_table_size = reg_val;
	gsw_r32(ETHSW_CAP_9_FLAGG_OFFSET, ETHSW_CAP_9_FLAGG_SHIFT,	\
		ETHSW_CAP_9_FLAGG_SIZE, &reg_val);
	pDev->flow_table_size = reg_val;

	gsw_r32(ETHSW_CAP_10_MACBT_OFFSET,	\
		ETHSW_CAP_10_MACBT_SHIFT, ETHSW_CAP_10_MACBT_SIZE, &reg_val);
	 pDev->mac_table_size = reg_val;

	gsw_r32(ETHSW_VERSION_REV_ID_OFFSET,	\
		ETHSW_VERSION_REV_ID_SHIFT, 16, &reg_val);
	pDev->GSWIP_Ver = reg_val;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MDIO_CfgGet(void *pDevCtx, IFX_ETHSW_MDIO_cfg_t *pPar)
{
	u32 value;
	gsw_r32((MDC_CFG_1_FREQ_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_1_FREQ_SHIFT, MDC_CFG_1_FREQ_SIZE, &value);
	pPar->nMDIO_Speed = value & 0xFF;
	gsw_r32((MDC_CFG_1_MCEN_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_1_MCEN_SHIFT, MDC_CFG_1_MCEN_SIZE, &value);
	pPar->bMDIO_Enable = value;
	if (value == 1)
		pPar->bMDIO_Enable = LTQ_ENABLE;
	else
		pPar->bMDIO_Enable = LTQ_DISABLE;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MDIO_CfgSet(void *pDevCtx, IFX_ETHSW_MDIO_cfg_t *pPar)
{
	u32 value;
	gsw_w32((MDC_CFG_1_FREQ_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_1_FREQ_SHIFT, MDC_CFG_1_FREQ_SIZE, pPar->nMDIO_Speed);
	if (pPar->bMDIO_Enable)
		value = 0x3F;
	else
		value = 0;
	/* Set Auto-Polling of connected PHYs - For all ports */
	gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_0_PEN_0_SHIFT, 6, value);
	gsw_w32((MDC_CFG_1_MCEN_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_1_MCEN_SHIFT, MDC_CFG_1_MCEN_SIZE, pPar->bMDIO_Enable);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MDIO_DataRead(void *pDevCtx, IFX_ETHSW_MDIO_data_t *pPar)
{
	u32 value;

	do {
		gsw_r32((MDIO_CTRL_MBUSY_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
	} while (value);
	value = ((0x2 << 10) | ((pPar->nAddressDev & 0x1F) << 5) | (pPar->nAddressReg & 0x1F));
	/* Special write command, becouse we need to write "MDIO Control Register" once at a time */
	gsw_w32((MDIO_CTRL_MBUSY_OFFSET + LTQ_SOC_TOP_REG_OFFSET), 0, 16, value);
	do {
		gsw_r32((MDIO_CTRL_MBUSY_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
	} while (value);
	gsw_r32((MDIO_READ_RDATA_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDIO_READ_RDATA_SHIFT, MDIO_READ_RDATA_SIZE, &value);
	pPar->nData = (value & 0xFFFF);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MDIO_DataWrite(void *pDevCtx, IFX_ETHSW_MDIO_data_t *pPar)
{
	u32 value;
	do {
		gsw_r32((MDIO_CTRL_MBUSY_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
	} while (value);
	value = pPar->nData & 0xFFFF;
	gsw_w32((MDIO_WRITE_WDATA_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDIO_WRITE_WDATA_SHIFT, MDIO_WRITE_WDATA_SIZE, value);
	value = ((0x1 << 10) | ((pPar->nAddressDev & 0x1F) << 5) | (pPar->nAddressReg & 0x1F));
	/* Special write command, becouse we need to write "MDIO Control Register" once at a time */
	gsw_w32((MDIO_CTRL_MBUSY_OFFSET + LTQ_SOC_TOP_REG_OFFSET), 0, 16, value);
	do {
		gsw_r32((MDIO_CTRL_MBUSY_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDIO_CTRL_MBUSY_SHIFT, MDIO_CTRL_MBUSY_SIZE, &value);
	} while (value);
	return LTQ_SUCCESS;
}

static inline void ltq_mdelay(int delay)
{
	int i;
	for (i = delay; i > 0; i--)
		udelay(1000);
}

static int force_to_configure_phy_settings(void *pDevCtx, u8 portIdx, u8 link_status)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 mdio_stat_reg, phy_addr_reg = 0;
	if (portIdx >= (pEthDev->nPortNumber-1))
		return LTQ_ERROR;
	gsw_r32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_0_ADDR_SHIFT, 16, &phy_addr_reg);
	gsw_r32((MDIO_STAT_0_TXPAUEN_OFFSET + LTQ_SOC_TOP_REG_OFFSET + portIdx),	\
		MDIO_STAT_0_TXPAUEN_SHIFT, 16, &mdio_stat_reg);
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
				if ((mdio_stat_reg >> 1) & 0x1) /*Receive Pause Enable Status */ {
					phy_addr_reg |= (0x1 << 5); /*Receive Pause Enable Status */
				} else {
					phy_addr_reg |= (0x3 << 5);
				}
				if ((mdio_stat_reg >> 0) & 0x1) /*Transmit Pause Enable Status */ {
					phy_addr_reg |= (0x1 << 7); /*Transmit Pause Enable Status */
				} else {
					phy_addr_reg |= (0x3 << 7);
				}
				gsw_w32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
					PHY_ADDR_0_ADDR_SHIFT, 16, phy_addr_reg);
			}
		}
	} else {
		phy_addr_reg &= ~(0xFFE0);
		phy_addr_reg |= (0x3 << 11);
		gsw_w32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
			PHY_ADDR_0_ADDR_SHIFT, 16, phy_addr_reg);
	}
	return LTQ_TRUE;
}

ethsw_ret_t IFX_FLOW_MmdDataRead(void *pDevCtx, IFX_ETHSW_MMD_data_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	IFX_ETHSW_MDIO_data_t mmd_data;
	u32 found = 0, phy_addr, mdc_reg, dev, portIdx;
	for (portIdx = 0; portIdx < (pEthDev->nPortNumber-1); portIdx++) {
		gsw_r32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
			PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &phy_addr);
		if (phy_addr == pPar->nAddressDev) {
			found = 1;
			break;
		}
	}
	if (found) {
		force_to_configure_phy_settings(pDevCtx, portIdx, 1);
		gsw_r32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
				MDC_CFG_0_PEN_0_SHIFT, 6, &mdc_reg);
		mdc_reg &= ~(1 << portIdx);
		dev = ((pPar->nAddressReg >> 16) & 0x1F);
		gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
		ltq_mdelay(20);
		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = dev;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		mmd_data.nData = pPar->nAddressReg & 0xFFFF;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = ((0x4000) | dev);
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mmd_data);
		pPar->nData = mmd_data.nData;

		mdc_reg |= (1 << portIdx);
		gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
		ltq_mdelay(100);
		force_to_configure_phy_settings(pDevCtx, portIdx, 0);
	} else {
		pr_err("(Invalid PHY Address) %s:%s:%d\n", __FILE__, __func__, __LINE__);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MmdDataWrite(void *pDevCtx, IFX_ETHSW_MMD_data_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	IFX_ETHSW_MDIO_data_t mmd_data;
	u32 found = 0, phy_addr, mdc_reg, dev, portIdx;
	for (portIdx = 0; portIdx < (pEthDev->nPortNumber-1); portIdx++) {
		gsw_r32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &phy_addr);
		if (phy_addr == pPar->nAddressDev) {
			found = 1;
			break;
		}
	}
	if (found) {
		force_to_configure_phy_settings(pDevCtx, portIdx, 1);
		gsw_r32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDC_CFG_0_PEN_0_SHIFT, 6, &mdc_reg);
		mdc_reg &= ~(1 << portIdx);
		dev = ((pPar->nAddressReg >> 16) & 0x1F);
		gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
		ltq_mdelay(20);
		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = dev;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		mmd_data.nData = pPar->nAddressReg & 0xFFFF;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xd;
		mmd_data.nData = ((0x4000) | dev);
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

		mmd_data.nAddressDev = pPar->nAddressDev;
		mmd_data.nAddressReg = 0xe;
		mmd_data.nData = pPar->nData;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);
		mdc_reg |= (1 << portIdx);
		gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
		ltq_mdelay(100);
		force_to_configure_phy_settings(pDevCtx, portIdx, 0);
	} else {
		pr_err("(Invalid PHY Address)  %s:%s:%d\n", __FILE__, __func__, __LINE__);
	}
	return LTQ_SUCCESS;
}

static ethsw_ret_t IFX_FLOW_MMD_MDIO_DataWrite(void *pDevCtx,	\
	IFX_ETHSW_MDIO_data_t *pPar, u8 portIdx, u8 dev)
{
	IFX_ETHSW_MDIO_data_t mmd_data;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 mdc_reg;
	if (portIdx >= (pEthDev->nPortNumber-1))
		return LTQ_ERROR;
	force_to_configure_phy_settings(pDevCtx, portIdx, 1);
	gsw_r32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			MDC_CFG_0_PEN_0_SHIFT, 6, &mdc_reg);

	mdc_reg &= ~(1 << portIdx);
	gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
	ltq_mdelay(20);

	mmd_data.nAddressDev = pPar->nAddressDev;
	mmd_data.nAddressReg = 0xd;
	mmd_data.nData = dev;
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

	mmd_data.nAddressDev = pPar->nAddressDev;
	mmd_data.nAddressReg = 0xe;
	mmd_data.nData = pPar->nAddressReg;
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

	mmd_data.nAddressDev = pPar->nAddressDev;
	mmd_data.nAddressReg = 0xd;
	mmd_data.nData = ((0x4000) | (dev & 0x1F));
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

	mmd_data.nAddressDev = pPar->nAddressDev;
	mmd_data.nAddressReg = 0xe;
	mmd_data.nData = pPar->nData;
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mmd_data);

	mdc_reg |= (1 << portIdx);
	gsw_w32((MDC_CFG_0_PEN_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		MDC_CFG_0_PEN_0_SHIFT, 6, mdc_reg);
	ltq_mdelay(100);
	force_to_configure_phy_settings(pDevCtx, portIdx, 0);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MonitorPortCfgGet(void *pDevCtx, IFX_ETHSW_monitorPortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	/* Get PCE Port Map 1 */
	gsw_r32(PCE_PMAP_1_MPMAP_OFFSET ,	\
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, &value);
	if (((value & (1 << portIdx)) >> portIdx) == 1)
		pPar->bMonitorPort = LTQ_TRUE;
	else
		pPar->bMonitorPort = LTQ_FALSE;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_MonitorPortCfgSet(void *pDevCtx, IFX_ETHSW_monitorPortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	gsw_r32(PCE_PMAP_1_MPMAP_OFFSET ,	\
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, &value);
	if (pPar->bMonitorPort == 1)
		value |= (pPar->bMonitorPort << portIdx);
	else
		value = (value & ~(1 << portIdx));
	gsw_w32(PCE_PMAP_1_MPMAP_OFFSET ,	\
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, value);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortLinkCfgGet(void *pDevCtx, IFX_ETHSW_portLinkCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	u8 portIdx = pPar->nPortId;
	if (pPar->nPortId >= (pEthDev->nPortNumber-1))
		return LTQ_ERROR;
	gsw_r32((MAC_PSTAT_FDUP_OFFSET + (0xC * portIdx)),	\
		MAC_PSTAT_FDUP_SHIFT, MAC_PSTAT_FDUP_SIZE , &value);
	if (value)
		pPar->eDuplex = IFX_ETHSW_DUPLEX_FULL;
	else
		pPar->eDuplex = IFX_ETHSW_DUPLEX_HALF;
	gsw_r32((MAC_PSTAT_GBIT_OFFSET + (0xC * portIdx)),	\
		MAC_PSTAT_GBIT_SHIFT, MAC_PSTAT_GBIT_SIZE , &value);
	if (value) {
		pPar->eSpeed = IFX_ETHSW_PORT_SPEED_1000;
	} else {
		gsw_r32((MAC_PSTAT_MBIT_OFFSET + (0xC * portIdx)),	\
			MAC_PSTAT_MBIT_SHIFT, MAC_PSTAT_MBIT_SIZE , &value);
		if (value)
			pPar->eSpeed = IFX_ETHSW_PORT_SPEED_100;
		else
			pPar->eSpeed = IFX_ETHSW_PORT_SPEED_10;
	}
	/* Low-power Idle Mode  configuration*/
	gsw_r32((MAC_CTRL_4_LPIEN_OFFSET + (0xC * portIdx)),	\
		MAC_CTRL_4_LPIEN_SHIFT, MAC_CTRL_4_LPIEN_SIZE, &value);
	pPar->bLPI = value;
	gsw_r32((MAC_PSTAT_LSTAT_OFFSET + (0xC * portIdx)),	\
		MAC_PSTAT_LSTAT_SHIFT, MAC_PSTAT_LSTAT_SIZE , &value);
	if (value)
		pPar->eLink = IFX_ETHSW_PORT_LINK_UP;
	else
		pPar->eLink = IFX_ETHSW_PORT_LINK_DOWN;
	gsw_r32((MII_CFG_0_MIIMODE_OFFSET + (0x2 * portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
		MII_CFG_0_MIIMODE_SHIFT, MII_CFG_0_MIIMODE_SIZE, &value);
	switch (value) {
	case 0:
			pPar->eMII_Mode = IFX_ETHSW_PORT_HW_MII;
			pPar->eMII_Type = IFX_ETHSW_PORT_PHY;
			break;
	case 1:
			pPar->eMII_Mode = IFX_ETHSW_PORT_HW_MII ;
			pPar->eMII_Type = IFX_ETHSW_PORT_MAC;
			break;
	case 2:
			pPar->eMII_Mode = IFX_ETHSW_PORT_HW_RMII;
			pPar->eMII_Type = IFX_ETHSW_PORT_PHY;
			break;
	case 3:
			pPar->eMII_Mode = IFX_ETHSW_PORT_HW_RMII;
			pPar->eMII_Type = IFX_ETHSW_PORT_MAC;
			break;
	case 4:
			pPar->eMII_Mode = IFX_ETHSW_PORT_HW_RGMII;
			pPar->eMII_Type = IFX_ETHSW_PORT_MAC;
			break;
	}
	pPar->eClkMode = IFX_ETHSW_PORT_CLK_NA;
	if (pPar->eMII_Mode == IFX_ETHSW_PORT_HW_RMII) {
		gsw_r32((MII_CFG_0_RMII_OFFSET + (0x2 * portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
			MII_CFG_0_RMII_SHIFT, MII_CFG_0_RMII_SIZE, &value);
		if (value == 1)
			pPar->eClkMode = IFX_ETHSW_PORT_CLK_MASTER;
		else
			pPar->eClkMode = IFX_ETHSW_PORT_CLK_SLAVE;
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortLinkCfgSet(void *pDevCtx, IFX_ETHSW_portLinkCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 data = DUPLEX_AUTO, phy_addr = 0, phy_ctrl = 0, duplex = DUPLEX_AUTO;
	u8 portIdx = pPar->nPortId;
	IFX_ETHSW_MDIO_data_t mdio_data;

	if (pPar->nPortId > (pEthDev->nPortNumber-1))
		return LTQ_ERROR;
	if (pPar->bDuplexForce == LTQ_TRUE) {
		if (pPar->eDuplex == IFX_ETHSW_DUPLEX_FULL) {
			data = DUPLEX_EN;	duplex = DUPLEX_EN;
		} else {
			data = DUPLEX_DIS; 	duplex = DUPLEX_DIS;
		}
	} else {
		data = DUPLEX_AUTO;
	}
	data = 3; /*default value*/
	if (pPar->bSpeedForce == LTQ_TRUE) {
		switch (pPar->eSpeed) {
		case IFX_ETHSW_PORT_SPEED_10:
				data = 0;
				if (duplex == DUPLEX_DIS)
					phy_ctrl = PHY_AN_ADV_10HDX;
				else
					phy_ctrl = PHY_AN_ADV_10FDX;
				break;
		case IFX_ETHSW_PORT_SPEED_100:
				data = 1;
				if (duplex == DUPLEX_DIS)
					phy_ctrl = PHY_AN_ADV_100HDX;
				else
					phy_ctrl = PHY_AN_ADV_100FDX;
				break;
		case IFX_ETHSW_PORT_SPEED_200:
				return LTQ_ERROR;
		case IFX_ETHSW_PORT_SPEED_1000:
				data = 2;
				if (duplex == DUPLEX_DIS)
					phy_ctrl = PHY_AN_ADV_1000HDX;
				else
					phy_ctrl = PHY_AN_ADV_1000FDX;
				break;
		}
	}
	gsw_r32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &phy_addr);
	mdio_data.nAddressDev = phy_addr;
	IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
	if ((data == 0) || (data == 1)) {
		mdio_data.nAddressReg = 4;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		mdio_data.nData &= ~(PHY_AN_ADV_10HDX | PHY_AN_ADV_10FDX | PHY_AN_ADV_100HDX | PHY_AN_ADV_100FDX);
		mdio_data.nData |= phy_ctrl;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
		mdio_data.nAddressReg = 9;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		mdio_data.nData &= ~(PHY_AN_ADV_1000HDX | PHY_AN_ADV_1000FDX);
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
	}
	if (data == 2) {
		mdio_data.nAddressReg = 9;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		mdio_data.nData &= ~(PHY_AN_ADV_1000HDX | PHY_AN_ADV_1000FDX);
		mdio_data.nData |= phy_ctrl;
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
		mdio_data.nAddressReg = 4;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		mdio_data.nData &= ~(PHY_AN_ADV_10HDX | PHY_AN_ADV_10FDX | PHY_AN_ADV_100HDX | PHY_AN_ADV_100FDX);
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
	}
	if (data == 3) {
		mdio_data.nAddressReg = 4;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		if (duplex == DUPLEX_DIS) {
			mdio_data.nData &= ~(PHY_AN_ADV_10HDX | PHY_AN_ADV_10FDX | PHY_AN_ADV_100HDX | PHY_AN_ADV_100FDX);
			mdio_data.nData |= (PHY_AN_ADV_10HDX | PHY_AN_ADV_100HDX);
		} else {
			mdio_data.nData |= (PHY_AN_ADV_10HDX | PHY_AN_ADV_10FDX | PHY_AN_ADV_100HDX | PHY_AN_ADV_100FDX);
		}
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
		mdio_data.nAddressReg = 9;
		IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		if (duplex == DUPLEX_DIS) {
			mdio_data.nData &= ~(PHY_AN_ADV_1000HDX | PHY_AN_ADV_1000FDX);
			mdio_data.nData |= (PHY_AN_ADV_1000HDX);
		} else {
			mdio_data.nData |= (PHY_AN_ADV_1000HDX | PHY_AN_ADV_1000FDX);
		}
		IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
	}
	gsw_w32((MAC_CTRL_4_LPIEN_OFFSET + (0xC * portIdx)),	\
		MAC_CTRL_4_LPIEN_SHIFT, MAC_CTRL_4_LPIEN_SIZE, pPar->bLPI);
	/* LPI Wait Time for 1G -- 50us */
	gsw_w32((MAC_CTRL_4_GWAIT_OFFSET + (0xC * portIdx)),	\
		MAC_CTRL_4_GWAIT_SHIFT, MAC_CTRL_4_GWAIT_SIZE, 0x32);
	/* LPI Wait Time for 100M -- 21us */
	gsw_w32((MAC_CTRL_4_WAIT_OFFSET + (0xC * portIdx)),	\
		MAC_CTRL_4_WAIT_SHIFT, MAC_CTRL_4_WAIT_SIZE, 0x15);
		/* LPI request controlled by data available for  port */
	gsw_w32((FDMA_CTRL_LPI_MODE_OFFSET),	\
		FDMA_CTRL_LPI_MODE_SHIFT, FDMA_CTRL_LPI_MODE_SIZE, 0x4);
	mdio_data.nAddressDev = phy_addr;
	mdio_data.nAddressReg = 0x3C;
	if (pPar->bLPI == LTQ_TRUE) {
		mdio_data.nData = 0x6;
		IFX_FLOW_MMD_MDIO_DataWrite(pEthDev, &mdio_data, portIdx, 0x07);
	} else {
		mdio_data.nData = 0x0;
		IFX_FLOW_MMD_MDIO_DataWrite(pEthDev, &mdio_data, portIdx, 0x07);
	}
	mdio_data.nAddressDev = phy_addr;
	mdio_data.nAddressReg = 0;
	IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
	mdio_data.nData = 0x1200;
	data = 0;
	if (pPar->bLinkForce == LTQ_TRUE) {
		if (pPar->eLink == IFX_ETHSW_PORT_LINK_UP) {
			data = 1;
		} else {
			data = 2;
			mdio_data.nData = 0x800;
		}
#ifdef LTQ_ETHSW_API_COC
		IFX_ETHSW_PM_linkForceSet(pEthDev->pPMCtx, portIdx, LTQ_TRUE);
#endif
	}
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
	data = 4; /*default mode */
	switch (pPar->eMII_Mode) {
	case IFX_ETHSW_PORT_HW_MII:
			data = 1;
			if (pPar->eMII_Type == IFX_ETHSW_PORT_PHY)
				data = 0;
			break;
	case IFX_ETHSW_PORT_HW_RMII:
			data = 3;
			if (pPar->eMII_Type == IFX_ETHSW_PORT_PHY)
				data = 2;
			break;
	case IFX_ETHSW_PORT_HW_RGMII:
			data = 4;
			break;
	case IFX_ETHSW_PORT_HW_GMII:
/*			data = 1; */
			break;
	}
	if (!(pPar->eMII_Mode == IFX_ETHSW_PORT_HW_GMII))
		gsw_w32((MII_CFG_0_MIIMODE_OFFSET + (0x2 * portIdx) +	\
			LTQ_SOC_TOP_REG_OFFSET), MII_CFG_0_MIIMODE_SHIFT, MII_CFG_0_MIIMODE_SIZE, data);
	data = 0;
	if (pPar->eMII_Mode == IFX_ETHSW_PORT_HW_RMII) {
		if (pPar->eClkMode == IFX_ETHSW_PORT_CLK_MASTER)
			data = 1;
		gsw_w32((MII_CFG_0_RMII_OFFSET + (0x2 * portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
			MII_CFG_0_RMII_SHIFT, MII_CFG_0_RMII_SIZE, data);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortPHY_AddrGet(void *pDevCtx, IFX_ETHSW_portPHY_Addr_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;
	if (pPar->nPortId >= (pEthDev->nPortNumber - 1))
		return LTQ_ERROR;
	gsw_r32(((PHY_ADDR_0_ADDR_OFFSET - portIdx) + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_0_ADDR_SHIFT, PHY_ADDR_0_ADDR_SIZE, &value);
	pPar->nAddressDev = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortPHY_Query(void *pDevCtx, IFX_ETHSW_portPHY_Query_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;
	if (pPar->nPortId >= (pEthDev->nPortNumber - 1))
		return LTQ_ERROR;
	gsw_r32((MAC_PSTAT_PACT_OFFSET + (0xC * portIdx)),	\
		MAC_PSTAT_PACT_SHIFT, MAC_PSTAT_PACT_SIZE , &value);
	pPar->bPHY_Present = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortRGMII_ClkCfgGet(void *pDevCtx, IFX_ETHSW_portRGMII_ClkCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	if (pPar->nPortId >= (pEthDev->nPortNumber-1))
		return LTQ_ERROR;
	gsw_r32(PCDU_0_RXDLY_OFFSET + (0x2 * pPar->nPortId) + LTQ_SOC_TOP_REG_OFFSET,	\
		PCDU_0_RXDLY_SHIFT, PCDU_0_RXDLY_SIZE, &value);
	pPar->nDelayRx = value;
	gsw_r32(PCDU_0_TXDLY_OFFSET + (0x2 * pPar->nPortId) + LTQ_SOC_TOP_REG_OFFSET,	\
		PCDU_0_TXDLY_SHIFT, PCDU_0_TXDLY_SIZE, &value);
	pPar->nDelayTx = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortRGMII_ClkCfgSet(void *pDevCtx, IFX_ETHSW_portRGMII_ClkCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= (pEthDev->nPortNumber - 1))
		return LTQ_ERROR;
	gsw_w32((PCDU_0_RXDLY_OFFSET + (0x2 * pPar->nPortId) + LTQ_SOC_TOP_REG_OFFSET),	\
		PCDU_0_RXDLY_SHIFT, PCDU_0_RXDLY_SIZE, pPar->nDelayRx);
	gsw_w32((PCDU_0_TXDLY_OFFSET + (0x2 * pPar->nPortId) + LTQ_SOC_TOP_REG_OFFSET),	\
		PCDU_0_TXDLY_SHIFT, PCDU_0_TXDLY_SIZE, pPar->nDelayTx);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortRedirectGet(void *pDevCtx, IFX_ETHSW_portRedirectCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	gsw_r32((PCE_PCTRL_3_EDIR_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_3_EDIR_SHIFT, PCE_PCTRL_3_EDIR_SIZE, &value);
	pPar->bRedirectEgress = value;
	if (PortRedirectFlag > 0)
		pPar->bRedirectIngress = LTQ_TRUE;
	else
		pPar->bRedirectIngress = LTQ_FALSE;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PortRedirectSet(void *pDevCtx, IFX_ETHSW_portRedirectCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	IFX_FLOW_PCE_rule_t PCE_rule;
	u8 portIdx = pPar->nPortId;
	u32 RedirectPort = 0, value = 0, i;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	value |= (1 << pEthDev->nManagementPortNumber);
	RedirectPort = value;
	gsw_w32(PCE_PMAP_1_MPMAP_OFFSET ,	\
		PCE_PMAP_1_MPMAP_SHIFT, PCE_PMAP_1_MPMAP_SIZE, value);
	value = pPar->bRedirectEgress;
	gsw_w32((PCE_PCTRL_3_EDIR_OFFSET + (0xA * portIdx)),	\
		PCE_PCTRL_3_EDIR_SHIFT, PCE_PCTRL_3_EDIR_SIZE, value);
	if (pPar->bRedirectIngress == LTQ_TRUE)
		PortRedirectFlag |= (1 << pPar->nPortId);
	else
		PortRedirectFlag &= ~(1 << pPar->nPortId) ;
	for (i = 0; i < pEthDev->nPortNumber; i++) {
		if (((PortRedirectFlag >> i) & 0x1) == 1) {
			memset(&PCE_rule, 0 , sizeof(IFX_FLOW_PCE_rule_t));
			PCE_rule.pattern.nIndex = (PORT_REDIRECT_PCE_RULE_INDEX + i);
			PCE_rule.pattern.bEnable = LTQ_TRUE;
			PCE_rule.pattern.bPortIdEnable = LTQ_TRUE;
			PCE_rule.pattern.nPortId = i;
			if (pPar->bRedirectIngress == LTQ_TRUE)
				PCE_rule.action.ePortMapAction = IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE;
			PCE_rule.action.nForwardPortMap = RedirectPort;
			/* We prepare everything and write into PCE Table */
			if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
				return LTQ_ERROR;
		}  else {
			if (0 != pce_pattern_delete(&pEthDev->PCE_Handler,	\
				PORT_REDIRECT_PCE_RULE_INDEX + i))
				return LTQ_ERROR;
		}
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_RMON_Clear(void *pDevCtx, IFX_ETHSW_RMON_clear_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	if (pPar->nPortId >= pEthDev->nPortNumber)
		return LTQ_ERROR;
	/* Reset all RMON counter */
	gsw_w32(BM_RMON_CTRL_RAM1_RES_OFFSET + (portIdx * 2),	\
		BM_RMON_CTRL_RAM1_RES_SHIFT, BM_RMON_CTRL_RAM1_RES_SIZE, 0x1);
	gsw_w32(BM_RMON_CTRL_RAM2_RES_OFFSET + (portIdx * 2),	\
		BM_RMON_CTRL_RAM2_RES_SHIFT, BM_RMON_CTRL_RAM2_RES_SIZE, 0x1);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_RMON_Get(void *pDevCtx, IFX_ETHSW_RMON_cnt_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 data, data0, data1, value, bcast_cnt;
	u32 r_frame = 0, r_unicast = 0, r_multicast = 0, 	\
		t_frame = 0, t_unicast = 0, t_multicast = 0;
	u32 R_GoodByteCount_L = 0, R_BadByteCount_L = 0, T_GoodByteCount_L = 0;
	unsigned long long int  R_GoodByteCount_H = 0,	\
		R_BadByteCount_H = 0, T_GoodByteCount_H = 0;
	u8 i;
	if (pEthDev) {
		if (pPar->nPortId >= pEthDev->nPortNumber)
			return LTQ_ERROR;
	}
/*	gsw_w32(BM_PCFG_CNTEN_OFFSET + (portIdx * 2),	\
		BM_PCFG_CNTEN_SHIFT, BM_PCFG_CNTEN_SIZE, 1); */
	memset(pPar, 0 , sizeof(IFX_ETHSW_RMON_cnt_t));
	pPar->nPortId = portIdx;
	gsw_r32(BM_RMON_CTRL_BCAST_CNT_OFFSET + (portIdx * 2),	\
		BM_RMON_CTRL_BCAST_CNT_SHIFT, BM_RMON_CTRL_BCAST_CNT_SIZE, &bcast_cnt);
	for (i = 0; i < RMON_COUNTER_OFFSET; i++) {
		gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
			BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE, i);
		gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
			BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, portIdx);
	gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
		BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0);
		value = 1;
		gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
				BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		gsw_r32(BM_RAM_VAL_0_VAL0_OFFSET,	\
			BM_RAM_VAL_0_VAL0_SHIFT, BM_RAM_VAL_0_VAL0_SIZE, &data0);
		gsw_r32(BM_RAM_VAL_1_VAL1_OFFSET,	\
			BM_RAM_VAL_1_VAL1_SHIFT, BM_RAM_VAL_1_VAL1_SIZE, &data1);
		data = (data1 << 16 | data0);
		switch (i) {
		case 0x1F: /* Receive Frme Count */
				if (bcast_cnt == 1)
					pPar->nRxBroadcastPkts = data;
				else
					pPar->nRxGoodPkts = data;
				r_frame = data;
				break;
		case 0x23: /* Receive Unicast Frame Count */
				pPar->nRxUnicastPkts = data;
				r_unicast = data;
				break;
		case 0x22: /* Receive Multicast Frame Count1 */
				pPar->nRxMulticastPkts = data;
				r_multicast = data;
				break;
		case 0x21: /* Receive CRC Errors Count */
				pPar->nRxFCSErrorPkts = data;
				break;
		case 0x1D: /* Receive Undersize Good Count */
				pPar->nRxUnderSizeGoodPkts = data;
				break;
		case 0x1B: /* Receive Oversize Good Count */
				pPar->nRxOversizeGoodPkts = data;
				break;
		case 0x1E: /* Receive Undersize Bad Count */
				pPar->nRxUnderSizeErrorPkts = data;
				break;
		case 0x20: /* Receive Pause Good Count */
				pPar->nRxGoodPausePkts = data;
				break;
		case 0x1C: /* Receive Oversize Bad Count */
				pPar->nRxOversizeErrorPkts = data;
				break;
		case 0x1A: /* Receive Alignment Errors Count */
				pPar->nRxAlignErrorPkts = data;
				break;
		case 0x12: /* Receive Size 64 Frame Count1 */
				pPar->nRx64BytePkts = data;
				break;
		case 0x13: /* Receive Size 65-127 Frame Count */
				pPar->nRx127BytePkts = data;
				break;
		case 0x14: /* Receive Size 128-255 Frame Count */
				pPar->nRx255BytePkts = data;
				break;
		case 0x15: /* Receive Size 256-511 Frame Count */
				pPar->nRx511BytePkts = data;
				break;
		case 0x16: /* Receive Size 512-1023 Frame Count */
				pPar->nRx1023BytePkts = data;
				break;
		case 0x17: /* Receive Size Greater 1023 Frame Count */
				pPar->nRxMaxBytePkts = data;
				break;
		case 0x18: /* Receive Discard (Tail-Drop) Frame Count */
				pPar->nRxDroppedPkts = data;
				break;
		case 0x19: /* Receive Drop (Filter) Frame Count */
				pPar->nRxFilteredPkts = data;
				break;
		case 0x24: /* Receive Good Byte Count (Low) */
				R_GoodByteCount_L = data;
				break;
		case 0x25: /* Receive Good Byte Count (High) */
				R_GoodByteCount_H = data;
				break;
		case 0x26: /* Receive Bad Byte Count (Low) */
				R_BadByteCount_L = data;
				break;
		case 0x27: /* Receive Bad Byte Count (High) */
				R_BadByteCount_H = data;
				break;
		case 0x0C: /* Transmit Frame Count */
				if (bcast_cnt == 1)
					pPar->nTxBroadcastPkts = data;
				else
					pPar->nTxGoodPkts = data;
				t_frame = data;
				break;
		case 0x06: /* Transmit Unicast Frame Count */
				pPar->nTxUnicastPkts = data;
				t_unicast = data;
				break;
		case 0x07: /* Transmit Multicast Frame Count1 */
				pPar->nTxMulticastPkts = data;
				t_multicast = data;
				break;
		case 0x00: /* Transmit Size 64 Frame Count */
				pPar->nTx64BytePkts = data;
				break;
		case 0x01: /* Transmit Size 65-127 Frame Count */
				pPar->nTx127BytePkts = data;
				break;
		case 0x02: /* Transmit Size 128-255 Frame Count */
				pPar->nTx255BytePkts = data;
				break;
		case 0x03: /* Transmit Size 256-511 Frame Count */
				pPar->nTx511BytePkts = data;
				break;
		case 0x04: /* Transmit Size 512-1023 Frame Count */
				pPar->nTx1023BytePkts = data;
				break;
		case 0x05: /* Transmit Size Greater 1024 Frame Count */
				pPar->nTxMaxBytePkts = data;
				break;
		case 0x08: /* Transmit Single Collision Count. */
				pPar->nTxSingleCollCount = data;
				break;
		case 0x09: /* Transmit Multiple Collision Count */
				pPar->nTxMultCollCount = data;
				break;
		case 0x0A: /* Transmit Late Collision Count */
				pPar->nTxLateCollCount = data;
				break;
		case 0x0B: /* Transmit Excessive Collision.*/
				pPar->nTxExcessCollCount = data;
				break;
		case 0x0D: /* Transmit Pause Frame Count */
				pPar->nTxPauseCount = data;
				break;
		case 0x10: /* Transmit Drop Frame Count */
				pPar->nTxDroppedPkts = data;
				break;
		case 0x0E: /* Transmit Good Byte Count (Low) */
				T_GoodByteCount_L = data;
				break;
		case 0x0F: /* Transmit Good Byte Count (High) */
				T_GoodByteCount_H = data;
				break;
		case 0x11: /* Transmit Dropped Packet Cound, based on Congestion Management.*/
				pPar->nTxAcmDroppedPkts = data;
				break;
			}
	}
	if (bcast_cnt == 1) {
		pPar->nRxGoodPkts = r_frame + r_unicast + r_multicast;
		pPar->nTxGoodPkts = t_frame + t_unicast + t_multicast;
	} else {
		/* Receive Broadcase Frme Count */
		pPar->nRxBroadcastPkts = r_frame - r_unicast - r_multicast;
		/* Transmit Broadcase Frme Count */
		pPar->nTxBroadcastPkts = t_frame - t_unicast - t_multicast;
	}
	/* Receive Good Byte Count */
	pPar->nRxGoodBytes = (R_GoodByteCount_H << 32) | R_GoodByteCount_L;
	/* Receive Bad Byte Count */
	pPar->nRxBadBytes = (R_BadByteCount_H << 32) | R_BadByteCount_L;
	/* Transmit Good Byte Count */
	pPar->nTxGoodBytes = (T_GoodByteCount_H << 32) | T_GoodByteCount_L;
	return LTQ_SUCCESS;
}
#if defined(CONFIG_LTQ_8021X) && CONFIG_LTQ_8021X
ethsw_ret_t IFX_FLOW_8021X_EAPOL_RuleGet(void *pDevCtx, IFX_ETHSW_8021X_EAPOL_Rule_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	ltq_stp_8021x_t *pStateHandle = &pEthDev->STP_8021x_Config;
	pPar->eForwardPort = pStateHandle->eForwardPort;
	pPar->nForwardPortId = pStateHandle->n8021X_ForwardPortId;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_8021X_EAPOL_RuleSet(void *pDevCtx, IFX_ETHSW_8021X_EAPOL_Rule_t *pPar)
{
	ethsw_api_dev_t *pEthDev			= (ethsw_api_dev_t *)pDevCtx;
	ltq_stp_8021x_t *pStateHandle	= &pEthDev->STP_8021x_Config;
	IFX_FLOW_PCE_rule_t PCE_rule;

	pStateHandle->eForwardPort = pPar->eForwardPort;
	pStateHandle->n8021X_ForwardPortId = pPar->nForwardPortId;
	memset(&PCE_rule, 0 , sizeof(IFX_FLOW_PCE_rule_t));
	PCE_rule.pattern.nIndex = EAPOL_PCE_RULE_INDEX;
	PCE_rule.pattern.bEnable = LTQ_TRUE;
	PCE_rule.pattern.bMAC_DstEnable	= LTQ_TRUE;
	PCE_rule.pattern.nMAC_Dst[0]	= 0x01;
	PCE_rule.pattern.nMAC_Dst[1]	= 0x80;
	PCE_rule.pattern.nMAC_Dst[2]	= 0xC2;
	PCE_rule.pattern.nMAC_Dst[3]	= 0x00;
	PCE_rule.pattern.nMAC_Dst[4]	= 0x00;
	PCE_rule.pattern.nMAC_Dst[5]	= 0x03;
	PCE_rule.pattern.nMAC_Src[5]	= 0;
	PCE_rule.pattern.bEtherTypeEnable	= LTQ_TRUE;
	PCE_rule.pattern.nEtherType	= 0x888E;
	PCE_rule.action.eCrossStateAction = IFX_FLOW_PCE_ACTION_CROSS_STATE_CROSS;
	if ((pStateHandle->eForwardPort < 4) && (pStateHandle->eForwardPort > 0))
		PCE_rule.action.ePortMapAction = pStateHandle->eForwardPort + 1;
	else
		pr_warning("(Incorrect forward port action) %s:%s:%d \n", __FILE__, __func__, __LINE__);
	PCE_rule.action.nForwardPortMap = (1 << pPar->nForwardPortId);
	/* We prepare everything and write into PCE Table */
	if (0 != pce_rule_write(&pEthDev->PCE_Handler, &PCE_rule))
		return LTQ_ERROR;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_8021X_PortCfgGet(void *pDevCtx, IFX_ETHSW_8021X_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev	= (ethsw_api_dev_t *)pDevCtx;
	port_config_t *pPortHandle ;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	pPortHandle = &pEthDev->PortConfig[pPar->nPortId];
	pPar->eState = pPortHandle->ifx_8021x_state;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_8021X_PortCfgSet(void *pDevCtx, IFX_ETHSW_8021X_portCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	port_config_t *pPortHandle;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	pPortHandle	= &pEthDev->PortConfig[pPar->nPortId];
	pPortHandle->ifx_8021x_state	= pPar->eState;
	set_port_state(pDevCtx, pPar->nPortId, pPortHandle->ifx_stp_state,	\
		pPortHandle->ifx_8021x_state);
	return LTQ_SUCCESS;
}

#endif /*CONFIG_LTQ_8021X  */
ethsw_ret_t IFX_FLOW_CPU_PortCfgGet(void *pDevCtx, IFX_ETHSW_CPU_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;

	if (pPar->nPortId >= pEthDev->nPortNumber)
		return LTQ_ERROR;
	if (portIdx ==  pEthDev->nCPU_Port)
		pPar->bCPU_PortValid = LTQ_ENABLE;
	else
		pPar->bCPU_PortValid = LTQ_DISABLE;
	/* Special Tag Egress*/
	gsw_r32((FDMA_PCTRL_STEN_OFFSET + (0x6 * portIdx)),	\
		FDMA_PCTRL_STEN_SHIFT, FDMA_PCTRL_STEN_SIZE, &value);
	pPar->bSpecialTagEgress = value;
	/* Special Tag Igress*/
	gsw_r32((PCE_PCTRL_0_IGSTEN_OFFSET + (0xa * portIdx)),	\
		PCE_PCTRL_0_IGSTEN_SHIFT, PCE_PCTRL_0_IGSTEN_SIZE, &value);
	pPar->bSpecialTagIngress = value;
	/* FCS Check */
	gsw_r32((SDMA_PCTRL_FCSIGN_OFFSET + (0x6 * portIdx)),	\
		SDMA_PCTRL_FCSIGN_SHIFT, SDMA_PCTRL_FCSIGN_SIZE, &value);
	pPar->bFcsCheck = value;
	/* FCS Generate */
	gsw_r32((MAC_CTRL_0_FCS_OFFSET + (0xC * portIdx)),	\
		MAC_CTRL_0_FCS_SHIFT, MAC_CTRL_0_FCS_SIZE, &value);
	pPar->bFcsGenerate = value;
	gsw_r32((FDMA_PCTRL_ST_TYPE_OFFSET + (0x6 * portIdx)),	\
		FDMA_PCTRL_ST_TYPE_SHIFT, FDMA_PCTRL_ST_TYPE_SIZE, &value);
	 pPar->bSpecialTagEthType = value  ;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_CPU_PortCfgSet(void *pDevCtx, IFX_ETHSW_CPU_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 RST, AS, AST, RXSH;

	if (pPar->nPortId >= pEthDev->nPortNumber)
		return LTQ_ERROR;
	if (portIdx == pEthDev->nCPU_Port)
		pPar->bCPU_PortValid = LTQ_ENABLE;
	else
		pPar->bCPU_PortValid = LTQ_DISABLE;
	pEthDev->nManagementPortNumber = portIdx;
	/* Special Tag Egress*/
	gsw_w32((FDMA_PCTRL_STEN_OFFSET + (0x6 * portIdx)),	\
		FDMA_PCTRL_STEN_SHIFT, FDMA_PCTRL_STEN_SIZE, pPar->bSpecialTagEgress);
	/* VRX CPU port */
	if (portIdx == pEthDev->nCPU_Port) {
		if (pPar->bSpecialTagEgress == LTQ_FALSE) {
			RST = 1; AS = 0;
		} else {
			RST = 0; AS = 1;
		}
		gsw_w32((PMAC_HD_CTL_RST_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_HD_CTL_RST_SHIFT, PMAC_HD_CTL_RST_SIZE, RST);
		gsw_w32((PMAC_HD_CTL_AS_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_HD_CTL_AS_SHIFT, PMAC_HD_CTL_AS_SIZE, AS);
	}
	/* Special Tag Igress*/
	gsw_w32((PCE_PCTRL_0_IGSTEN_OFFSET + (0xa * portIdx)),	\
		PCE_PCTRL_0_IGSTEN_SHIFT, PCE_PCTRL_0_IGSTEN_SIZE, pPar->bSpecialTagIngress);
	if (pPar->bSpecialTagIngress == LTQ_FALSE) {
		AST = 0; RXSH = 0;
	} else {
		AST = 1; RXSH = 1;
	}
	gsw_w32((PMAC_HD_CTL_AST_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_AST_SHIFT, PMAC_HD_CTL_AST_SIZE, AST);
	gsw_w32((PMAC_HD_CTL_RXSH_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_RXSH_SHIFT, PMAC_HD_CTL_RXSH_SIZE, RXSH);
	/* FCS Check */
	gsw_w32((SDMA_PCTRL_FCSIGN_OFFSET + (0x6 * portIdx)),	\
		SDMA_PCTRL_FCSIGN_SHIFT, SDMA_PCTRL_FCSIGN_SIZE, pPar->bFcsCheck);
	/* FCS Generate */
	gsw_w32((MAC_CTRL_0_FCS_OFFSET + (0xC * portIdx)),	\
		MAC_CTRL_0_FCS_SHIFT, MAC_CTRL_0_FCS_SIZE, pPar->bFcsGenerate);
	if (pPar->bSpecialTagEthType == IFX_ETHSW_CPU_ETHTYPE_FLOWID) {
		gsw_w32((FDMA_PCTRL_ST_TYPE_OFFSET + (0x6 * portIdx)),	\
			FDMA_PCTRL_ST_TYPE_SHIFT, FDMA_PCTRL_ST_TYPE_SIZE, 1);
	} else {
		gsw_w32((FDMA_PCTRL_ST_TYPE_OFFSET + (0x6 * portIdx)),	\
			FDMA_PCTRL_ST_TYPE_SHIFT, FDMA_PCTRL_ST_TYPE_SIZE, 0);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_CPU_PortExtendCfgGet(void *pDevCtx, IFX_ETHSW_CPU_PortExtendCfg_t *pPar)
{
	u32  value, value_add, value_vlan;

	gsw_r32((PMAC_HD_CTL_ADD_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_ADD_SHIFT, PMAC_HD_CTL_ADD_SIZE, &value_add);
	gsw_r32((PMAC_HD_CTL_TAG_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_TAG_SHIFT, PMAC_HD_CTL_TAG_SIZE, &value_vlan);
	if (value_add == 0 && value_vlan == 0)
		pPar->eHeaderAdd = 0;
	else if (value_add == 1 && value_vlan == 0)
		pPar->eHeaderAdd = 1;
	else if (value_add == 1 && value_vlan == 1)
		pPar->eHeaderAdd = 2;
	else
		pPar->eHeaderAdd = 0;

	gsw_r32((PMAC_HD_CTL_RL2_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_RL2_SHIFT, PMAC_HD_CTL_RL2_SIZE, &value);
	pPar->bHeaderRemove = value;
	memset(&pPar->sHeader, 0 , sizeof(IFX_ETHSW_CPU_Header_t));
	if (value_add == 1) {
		/* Output the Src MAC */
		gsw_r32((PMAC_SA3_SA_15_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_SA3_SA_15_0_SHIFT, PMAC_SA3_SA_15_0_SIZE, &value);
		pPar->sHeader.nMAC_Src[0] = value & 0xFF;
		pPar->sHeader.nMAC_Src[1] = ((value >> 8) & 0xFF);
		gsw_r32((PMAC_SA2_SA_31_16_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_SA2_SA_31_16_SHIFT, PMAC_SA2_SA_31_16_SIZE, &value);
		pPar->sHeader.nMAC_Src[2] = value & 0xFF;
		pPar->sHeader.nMAC_Src[3] = ((value >> 8) & 0xFF);
		gsw_r32((PMAC_SA1_SA_47_32_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_SA1_SA_47_32_SHIFT, PMAC_SA1_SA_47_32_SIZE, &value);
		pPar->sHeader.nMAC_Src[4] = value & 0xFF;
		pPar->sHeader.nMAC_Src[5] = ((value >> 8) & 0xFF);
		/* Output the Dst MAC */
		gsw_r32((PMAC_DA3_DA_15_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_DA3_DA_15_0_SHIFT, PMAC_DA3_DA_15_0_SIZE, &value);
		pPar->sHeader.nMAC_Dst[0] = value & 0xFF;;
		pPar->sHeader.nMAC_Dst[1] = ((value >> 8) & 0xFF);
		gsw_r32((PMAC_DA2_DA_31_16_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_DA2_DA_31_16_SHIFT, PMAC_DA2_DA_31_16_SIZE, &value);
		pPar->sHeader.nMAC_Dst[2] = value & 0xFF;;
		pPar->sHeader.nMAC_Dst[3] = ((value >> 8) & 0xFF);
		gsw_r32((PMAC_DA1_SA_47_32_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_DA1_SA_47_32_SHIFT, PMAC_DA1_SA_47_32_SIZE, &value);
		pPar->sHeader.nMAC_Dst[4] = value & 0xFF;;
		pPar->sHeader.nMAC_Dst[5] = ((value >> 8) & 0xFF);
		/* Input the Ethernet Type */
		gsw_r32((PMAC_TL_TYPE_LEN_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_TL_TYPE_LEN_SHIFT, PMAC_TL_TYPE_LEN_SIZE, &value);
		pPar->sHeader.nEthertype = value;
	}
	if (value_vlan == 1) {
		gsw_r32((PMAC_VLAN_PRI_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_VLAN_PRI_SHIFT, PMAC_VLAN_PRI_SIZE, &value);
		pPar->sHeader.nVLAN_Prio = value;
		gsw_r32((PMAC_VLAN_CFI_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_VLAN_CFI_SHIFT, PMAC_VLAN_CFI_SIZE, &value);
		pPar->sHeader.nVLAN_CFI = value;
		gsw_r32((PMAC_VLAN_VLAN_ID_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_VLAN_VLAN_ID_SHIFT, PMAC_VLAN_VLAN_ID_SIZE, &value);
		pPar->sHeader.nVLAN_ID = value;
	}
	gsw_r32((PMAC_HD_CTL_FC_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_FC_SHIFT, PMAC_HD_CTL_FC_SIZE, &value);
	pPar->ePauseCtrl = value;
	gsw_r32((PMAC_HD_CTL_RC_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_RC_SHIFT, PMAC_HD_CTL_RC_SIZE, &value);
	pPar->bFcsRemove = value;
	gsw_r32((PMAC_EWAN_EWAN_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_EWAN_EWAN_SHIFT, PMAC_EWAN_EWAN_SIZE, &value);
	pPar->nWAN_Ports = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_CPU_PortExtendCfgSet(void *pDevCtx, IFX_ETHSW_CPU_PortExtendCfg_t *pPar)
{
	u32 value_add = 0, value_vlan = 0, nMAC;
	switch (pPar->eHeaderAdd) {
	case IFX_ETHSW_CPU_HEADER_NO:
			value_add = 0; value_vlan = 0;
			break;
	case IFX_ETHSW_CPU_HEADER_MAC:
			value_add = 1; value_vlan = 0;
			break;
	case IFX_ETHSW_CPU_HEADER_VLAN:
			value_add = 1; value_vlan = 1;
			break;
	}
	if ((pPar->bHeaderRemove == LTQ_TRUE) && (pPar->eHeaderAdd != IFX_ETHSW_CPU_HEADER_NO)) {
		pr_err("The Header Can't be remove because the Header Add parameter is not 0");
		return LTQ_ERROR;
	}  else {
		gsw_w32((PMAC_HD_CTL_RL2_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_HD_CTL_RL2_SHIFT, PMAC_HD_CTL_RL2_SIZE, pPar->bHeaderRemove);
	}
	gsw_w32((PMAC_HD_CTL_ADD_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_ADD_SHIFT, PMAC_HD_CTL_ADD_SIZE, value_add);
	gsw_w32((PMAC_HD_CTL_TAG_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_TAG_SHIFT, PMAC_HD_CTL_TAG_SIZE, value_vlan);
	if (pPar->eHeaderAdd == IFX_ETHSW_CPU_HEADER_MAC) {
		/* Input the Src MAC */
		nMAC = pPar->sHeader.nMAC_Src[0] | pPar->sHeader.nMAC_Src[1] << 8;
		gsw_w32((PMAC_SA3_SA_15_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_SA3_SA_15_0_SHIFT, PMAC_SA3_SA_15_0_SIZE, nMAC);
		nMAC = pPar->sHeader.nMAC_Src[2] | pPar->sHeader.nMAC_Src[3] << 8;
		gsw_w32((PMAC_SA2_SA_31_16_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_SA2_SA_31_16_SHIFT, PMAC_SA2_SA_31_16_SIZE, nMAC);
		nMAC = pPar->sHeader.nMAC_Src[4] | pPar->sHeader.nMAC_Src[5] << 8;
		gsw_w32((PMAC_SA1_SA_47_32_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_SA1_SA_47_32_SHIFT, PMAC_SA1_SA_47_32_SIZE, nMAC);
		/* Input the Dst MAC */
		nMAC = pPar->sHeader.nMAC_Dst[0] | pPar->sHeader.nMAC_Dst[1] << 8;
		gsw_w32((PMAC_DA3_DA_15_0_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_DA3_DA_15_0_SHIFT, PMAC_DA3_DA_15_0_SIZE, nMAC);
		nMAC = pPar->sHeader.nMAC_Dst[2] | pPar->sHeader.nMAC_Dst[3] << 8;
		gsw_w32((PMAC_DA2_DA_31_16_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_DA2_DA_31_16_SHIFT, PMAC_DA2_DA_31_16_SIZE, nMAC);
		nMAC = pPar->sHeader.nMAC_Dst[4] | pPar->sHeader.nMAC_Dst[5] << 8;
		gsw_w32((PMAC_DA1_SA_47_32_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_DA1_SA_47_32_SHIFT, PMAC_DA1_SA_47_32_SIZE, nMAC);
		/* Input the Ethernet Type */
		gsw_w32((PMAC_TL_TYPE_LEN_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_TL_TYPE_LEN_SHIFT, PMAC_TL_TYPE_LEN_SIZE, pPar->sHeader.nEthertype);
	}
	if (pPar->eHeaderAdd == IFX_ETHSW_CPU_HEADER_VLAN) {
		gsw_w32((PMAC_VLAN_PRI_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PMAC_VLAN_PRI_SHIFT, PMAC_VLAN_PRI_SIZE, pPar->sHeader.nVLAN_Prio);
	gsw_w32((PMAC_VLAN_CFI_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_VLAN_CFI_SHIFT, PMAC_VLAN_CFI_SIZE, pPar->sHeader.nVLAN_CFI);
	gsw_w32((PMAC_VLAN_VLAN_ID_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_VLAN_VLAN_ID_SHIFT, PMAC_VLAN_VLAN_ID_SIZE, pPar->sHeader.nVLAN_ID);
	}
	gsw_w32((PMAC_HD_CTL_FC_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_FC_SHIFT, PMAC_HD_CTL_FC_SIZE, pPar->ePauseCtrl);
	gsw_w32((PMAC_HD_CTL_RC_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_HD_CTL_RC_SHIFT, PMAC_HD_CTL_RC_SIZE, pPar->bFcsRemove);
	gsw_w32((PMAC_EWAN_EWAN_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PMAC_EWAN_EWAN_SHIFT, PMAC_EWAN_EWAN_SIZE, pPar->nWAN_Ports);
	return LTQ_SUCCESS;
}

#if defined(CONFIG_LTQ_WOL) && CONFIG_LTQ_WOL
ethsw_ret_t IFX_FLOW_WoL_CfgGet(void *pDevCtx, IFX_ETHSW_WoL_Cfg_t *pPar)
{
	u32 value;
	gsw_r32(WOL_GLB_CTRL_PASSEN_OFFSET, WOL_GLB_CTRL_PASSEN_SHIFT,	\
		WOL_GLB_CTRL_PASSEN_SIZE, &value);
	pPar->bWolPasswordEnable = value;
	gsw_r32(WOL_DA_2_DA2_OFFSET, WOL_DA_2_DA2_SHIFT,	\
		WOL_DA_2_DA2_SIZE, &value);
	pPar->nWolMAC[0] = (value >> 8 & 0xFF);
	pPar->nWolMAC[1] = (value & 0xFF);
	gsw_r32(WOL_DA_1_DA1_OFFSET, WOL_DA_1_DA1_SHIFT,	\
		WOL_DA_1_DA1_SIZE, &value);
	pPar->nWolMAC[2] = (value >> 8 & 0xFF);
	pPar->nWolMAC[3] = (value & 0xFF);
	gsw_r32(WOL_DA_0_DA0_OFFSET, WOL_DA_0_DA0_SHIFT,	\
		WOL_DA_0_DA0_SIZE, &value);
	pPar->nWolMAC[4] = (value >> 8 & 0xFF);
	pPar->nWolMAC[5] = (value & 0xFF);
	gsw_r32(WOL_PW_2_PW2_OFFSET, WOL_PW_2_PW2_SHIFT,	\
		WOL_PW_2_PW2_SIZE, &value);
	pPar->nWolPassword[0] = (value >> 8 & 0xFF);
	pPar->nWolPassword[1] = (value & 0xFF);
	gsw_r32(WOL_PW_1_PW1_OFFSET, WOL_PW_1_PW1_SHIFT,	\
		WOL_PW_1_PW1_SIZE, &value);
	pPar->nWolPassword[2] = (value >> 8 & 0xFF);
	pPar->nWolPassword[3] = (value & 0xFF);
	gsw_r32(WOL_PW_0_PW0_OFFSET, WOL_PW_0_PW0_SHIFT,	\
		WOL_PW_0_PW0_SIZE, &value);
	pPar->nWolPassword[4] = (value >> 8 & 0xFF);
	pPar->nWolPassword[5] = (value & 0xFF);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_WoL_CfgSet(void *pDevCtx, IFX_ETHSW_WoL_Cfg_t *pPar)
{
	gsw_w32(WOL_GLB_CTRL_PASSEN_OFFSET, WOL_GLB_CTRL_PASSEN_SHIFT,	\
		WOL_GLB_CTRL_PASSEN_SIZE, pPar->bWolPasswordEnable);
	gsw_w32(WOL_DA_2_DA2_OFFSET, WOL_DA_2_DA2_SHIFT, WOL_DA_2_DA2_SIZE,	\
		(((pPar->nWolMAC[0] & 0xFF) << 8) | (pPar->nWolMAC[1] & 0xFF)));
	gsw_w32(WOL_DA_1_DA1_OFFSET, WOL_DA_1_DA1_SHIFT, WOL_DA_1_DA1_SIZE,	\
		(((pPar->nWolMAC[2] & 0xFF) << 8) | (pPar->nWolMAC[3] & 0xFF)));
	gsw_w32(WOL_DA_0_DA0_OFFSET, WOL_DA_0_DA0_SHIFT, WOL_DA_0_DA0_SIZE,	\
		(((pPar->nWolMAC[4] & 0xFF) << 8) | (pPar->nWolMAC[5] & 0xFF)));
	gsw_w32(WOL_PW_2_PW2_OFFSET, WOL_PW_2_PW2_SHIFT, WOL_PW_2_PW2_SIZE,	\
		(((pPar->nWolPassword[0] & 0xFF) << 8) | (pPar->nWolPassword[1] & 0xFF)));
	gsw_w32(WOL_PW_1_PW1_OFFSET, WOL_PW_1_PW1_SHIFT, WOL_PW_1_PW1_SIZE,	\
		(((pPar->nWolPassword[2] & 0xFF) << 8) | (pPar->nWolPassword[3] & 0xFF)));
	gsw_w32(WOL_PW_0_PW0_OFFSET, WOL_PW_0_PW0_SHIFT, WOL_PW_0_PW0_SIZE,	\
		(((pPar->nWolPassword[4] & 0xFF) << 8) | (pPar->nWolPassword[5] & 0xFF)));
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_WoL_PortCfgGet(void *pDevCtx, IFX_ETHSW_WoL_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	gsw_r32((WOL_CTRL_PORT_OFFSET + (0xA * pPar->nPortId)),	\
		WOL_CTRL_PORT_SHIFT, WOL_CTRL_PORT_SIZE, &value);
	pPar->bWakeOnLAN_Enable = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_WoL_PortCfgSet(void *pDevCtx, IFX_ETHSW_WoL_PortCfg_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	gsw_w32((WOL_CTRL_PORT_OFFSET + (0xA * pPar->nPortId)),	\
		WOL_CTRL_PORT_SHIFT, WOL_CTRL_PORT_SIZE, pPar->bWakeOnLAN_Enable);
	return LTQ_SUCCESS;
}

#endif /* CONFIG_LTQ_WOL */
ethsw_ret_t IFX_FLOW_RegisterGet(void *pDevCtx, IFX_FLOW_register_t *pPar)
{
	u32 regValue, regAddr = pPar->nRegAddr;
	gsw_r32(regAddr,	0, 16, &regValue);
	pPar->nData = regValue;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_RegisterSet(void *pDevCtx, IFX_FLOW_register_t *pPar)
{
	u32 regValue = pPar->nData, regAddr = pPar->nRegAddr;
	gsw_w32(regAddr, 0, 16, regValue);
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_IrqGet(void *pDevCtx, IFX_FLOW_irq_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	/* ToDo: Require future clarify for how to display */
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_IrqMaskGet(void *pDevCtx, IFX_FLOW_irq_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;
	u32 value;
	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	if (pPar->eIrqSrc == IFX_FLOW_IRQ_WOL) {
		gsw_r32((PCE_PIER_WOL_OFFSET + (0xA * portIdx)),	\
			PCE_PIER_WOL_SHIFT, PCE_PIER_WOL_SIZE, &value);
	} else if (pPar->eIrqSrc == IFX_FLOW_IRQ_LIMIT_ALERT) {
		gsw_r32((PCE_PIER_LOCK_OFFSET + (0xA * portIdx)),	\
			PCE_PIER_LOCK_SHIFT, PCE_PIER_LOCK_SIZE, &value);
	} else if (pPar->eIrqSrc == IFX_FLOW_IRQ_LOCK_ALERT) {
		gsw_r32((PCE_PIER_LIM_OFFSET + (0xA * portIdx)),	\
			PCE_PIER_LIM_SHIFT, PCE_PIER_LIM_SIZE, &value);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_IrqMaskSet(void *pDevCtx, IFX_FLOW_irq_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId;

	if (pPar->nPortId >= pEthDev->nTotalPortNumber)
		return LTQ_ERROR;
	if (pPar->eIrqSrc == IFX_FLOW_IRQ_WOL) {
		gsw_w32((PCE_PIER_WOL_OFFSET + (0xA * portIdx)),	\
			PCE_PIER_WOL_SHIFT, PCE_PIER_WOL_SIZE, 1);
	} else if (pPar->eIrqSrc == IFX_FLOW_IRQ_LIMIT_ALERT) {
		gsw_w32((PCE_PIER_LOCK_OFFSET + (0xA * portIdx)),	\
			PCE_PIER_LOCK_SHIFT, PCE_PIER_LOCK_SIZE, 1);
	} else if (pPar->eIrqSrc == IFX_FLOW_IRQ_LOCK_ALERT) {
		gsw_w32((PCE_PIER_LIM_OFFSET + (0xA * portIdx)),	\
			PCE_PIER_LIM_SHIFT, PCE_PIER_LIM_SIZE, 1);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_IrqStatusClear(void *pDevCtx, IFX_FLOW_irq_t *pPar)
{
	/* ToDo: Request future clarify */
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PceRuleRead(void *pDevCtx, IFX_FLOW_PCE_rule_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (0 != pce_rule_read(&pEthDev->PCE_Handler, pPar))
		return LTQ_ERROR;
  return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PceRuleWrite(void *pDevCtx, IFX_FLOW_PCE_rule_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	if (0 != pce_rule_write(&pEthDev->PCE_Handler, pPar))
		return LTQ_ERROR;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_PceRuleDelete(void *pDevCtx,	\
	IFX_FLOW_PCE_ruleDelete_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u32 value;
	value = pPar->nIndex;
	if (0 != pce_pattern_delete(&pEthDev->PCE_Handler, value))
		return LTQ_ERROR;
	if (0 != pce_action_delete(&pEthDev->PCE_Handler, value))
		return LTQ_ERROR;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_RMON_ExtendGet(void *pDevCtx,	\
	IFX_FLOW_RMON_extendGet_t *pPar)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	u8 portIdx = pPar->nPortId, i;
	u32 value, data0, data1;

	if (pPar->nPortId >= pEthDev->nPortNumber)
		return LTQ_ERROR;

	memset(pPar, 0 , sizeof(IFX_FLOW_RMON_extendGet_t));
	for (i = 0; i < IFX_FLOW_RMON_EXTEND_NUM; i++) {
		gsw_w32(BM_RAM_ADDR_ADDR_OFFSET,	\
			BM_RAM_ADDR_ADDR_SHIFT, BM_RAM_ADDR_ADDR_SIZE,	\
			(i+RMON_EXTEND_TRAFFIC_FLOW_COUNT_1));
		gsw_w32(BM_RAM_CTRL_ADDR_OFFSET,	\
			BM_RAM_CTRL_ADDR_SHIFT, BM_RAM_CTRL_ADDR_SIZE, portIdx);
		gsw_w32(BM_RAM_CTRL_OPMOD_OFFSET,	\
			BM_RAM_CTRL_OPMOD_SHIFT, BM_RAM_CTRL_OPMOD_SIZE, 0);
		value = 1;
		gsw_w32(BM_RAM_CTRL_BAS_OFFSET,	\
			BM_RAM_CTRL_BAS_SHIFT, BM_RAM_CTRL_BAS_SIZE, value);
		do {
			gsw_r32(BM_RAM_CTRL_BAS_OFFSET,	\
				BM_RAM_CTRL_BAS_SHIFT,	\
				BM_RAM_CTRL_BAS_SIZE, &value);
		} while (value);
		gsw_r32(BM_RAM_VAL_0_VAL0_OFFSET,	\
			BM_RAM_VAL_0_VAL0_SHIFT,	\
			BM_RAM_VAL_0_VAL0_SIZE, &data0);
		gsw_r32(BM_RAM_VAL_1_VAL1_OFFSET,	\
			BM_RAM_VAL_1_VAL1_SHIFT,	\
			BM_RAM_VAL_1_VAL1_SIZE, &data1);
		pPar->nTrafficFlowCnt[i] = (data1 << 16 | data0);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_Reset(void *pDevCtx, IFX_FLOW_reset_t *pPar)
{
	ethsw_api_dev_t *pDev = (ethsw_api_dev_t *)pDevCtx;
	pDev->bResetCalled = LTQ_TRUE;
	pDev->bHW_InitCalled = LTQ_FALSE;
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_VersionGet(void *pDevCtx, IFX_ETHSW_version_t *pPar)
{
	if (pPar->nId == 0) {
		memcpy(pPar->cName, VERSION_NAME, sizeof(VERSION_NAME));
		memcpy(pPar->cVersion, VERSION_NUMBER, sizeof(VERSION_NUMBER));
	} else if (pPar->nId == 1) {
		memcpy(pPar->cName, MICRO_CODE_VERSION_NAME,	\
			sizeof(MICRO_CODE_VERSION_NAME));
		memcpy(pPar->cVersion, MICRO_CODE_VERSION_NUMBER,	\
			sizeof(MICRO_CODE_VERSION_NUMBER));
	} else {
		memcpy(pPar->cName, "", 0);
		memcpy(pPar->cVersion, "", 0);
	}
	return LTQ_SUCCESS;
}

#if 0
int platform_device_init(void *pDevCtx)
{
	u32 reg_val, phy_reg;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
/*	reg_val = GSWITCH_R32_ACCESS(GFMDIO_ADD); */
	reg_val = 0x0C4E51;
	gsw_r32((PHY_ADDR_2_ADDR_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
			PHY_ADDR_2_ADDR_SHIFT, PHY_ADDR_2_ADDR_SIZE, &phy_reg);
	phy_reg &= ~(0x1F);
	phy_reg |= (reg_val & 0x1F);
	gsw_w32((PHY_ADDR_2_ADDR_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_2_ADDR_SHIFT, PHY_ADDR_2_ADDR_SIZE, phy_reg);

	gsw_r32((PHY_ADDR_3_ADDR_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_3_ADDR_SHIFT, PHY_ADDR_3_ADDR_SIZE, &phy_reg);
	phy_reg &= ~(0x1F);
	phy_reg |= ((reg_val >> 5) & 0x1F);
	gsw_w32((PHY_ADDR_3_ADDR_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_3_ADDR_SHIFT, PHY_ADDR_3_ADDR_SIZE, phy_reg);

	gsw_r32((PHY_ADDR_4_ADDR_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_4_ADDR_SHIFT, PHY_ADDR_4_ADDR_SIZE, &phy_reg);
	phy_reg &= ~(0x1F);
	phy_reg |= ((reg_val >> 10) & 0x1F);
	gsw_w32((PHY_ADDR_4_ADDR_OFFSET + LTQ_SOC_TOP_REG_OFFSET),	\
		PHY_ADDR_4_ADDR_SHIFT, PHY_ADDR_4_ADDR_SIZE, phy_reg);

	if (pEthDev->GSWIP_Ver == LTQ_GSWIP_2_0) {
		/* Set Port 6 RBUF_BYPASS mode */
		gsw_r32((MAC_LPITMER0_TMLSB_OFFSET + (6 * 0xC)),	\
			PHY_ADDR_4_ADDR_SHIFT, PHY_ADDR_4_ADDR_SIZE, &phy_reg);
		phy_reg |= (1 << 6);
		gsw_w32((MAC_LPITMER0_TMLSB_OFFSET + (6 * 0xC)),	\
			PHY_ADDR_4_ADDR_SHIFT, PHY_ADDR_4_ADDR_SIZE, phy_reg);
		gsw_w32((PMAC_RX_IPG_IPG_CNT_OFFSET + LTQ_SOC_TOP_REG_OFFSET), \
			PMAC_RX_IPG_IPG_CNT_SHIFT,	\
			PMAC_RX_IPG_IPG_CNT_SIZE, 0xB);
	}
	return LTQ_SUCCESS;
}
#endif /* if 0*/

ethsw_ret_t IFX_FLOW_Enable(void *pDevCtx)
{
	ethsw_api_dev_t *pDev = (ethsw_api_dev_t *)pDevCtx;
	u8 j;
/*	if (pDev->bHW_InitCalled == LTQ_FALSE)
		platform_device_init(pDev); */
	for (j = 0; j < pDev->nPortNumber; j++) {
		gsw_w32((FDMA_PCTRL_EN_OFFSET + (j * 0x6)),	\
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 1);
		gsw_w32((SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),	\
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 1);
		gsw_w32((BM_PCFG_CNTEN_OFFSET + (j * 2)),	\
			BM_PCFG_CNTEN_SHIFT, BM_PCFG_CNTEN_SIZE, 1);
	}
	return LTQ_SUCCESS;
}

ethsw_ret_t IFX_FLOW_Disable(void *pDevCtx)
{
	ethsw_api_dev_t *pDev = (ethsw_api_dev_t *)pDevCtx;
	u8 j;
	/* Disable all physical port  */
	for (j = 0; j < pDev->nPortNumber; j++) {
		gsw_w32((FDMA_PCTRL_EN_OFFSET + (j * 0x6)),	\
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 0);
		gsw_w32((SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),	\
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 0);
	}
	return LTQ_SUCCESS;
}

#ifdef LTQ_ETHSW_API_COC
/** switch core layer function to set PHY PD bit (Phy register 0:bit11) */
ethsw_ret_t PHY_PDN_Set(void *pDevCtx, u8 nPHYAD)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	IFX_ETHSW_MDIO_data_t mdio_data;
	mdio_data.nAddressDev = nPHYAD;
	mdio_data.nAddressReg = 0;
	IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
	/* GSWIP_SET_BITS(x, msb, lsb, value) */
	/*  Forces the PHY device into Power Down */
	mdio_data.nData = GSWIP_SET_BITS(mdio_data.nData, 11, 11, 1);
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
	return LTQ_SUCCESS;
}

/** This is the switch core layer function to clear PHY PDN bit */
ethsw_ret_t PHY_PDN_Clear(void *pDevCtx, u8 nPHYAD)
{
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	IFX_ETHSW_MDIO_data_t mdio_data;
	mdio_data.nAddressDev = nPHYAD;
	mdio_data.nAddressReg = 0;
	IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
	/* GSWIP_SET_BITS(x, msb, lsb, value) */
	mdio_data.nData = GSWIP_SET_BITS(mdio_data.nData, 11, 11, 0);
	IFX_FLOW_MDIO_DataWrite(pEthDev, &mdio_data);
	return LTQ_SUCCESS;
}

/** This is the switch core layer function to get medium detect status */
ltq_bool_t PHY_mediumDetectStatusGet(void *pDevCtx, u8 nPortID)
{
	IFX_ETHSW_MDIO_data_t mdio_data;
	u32 value;
	ltq_bool_t bLinkStatus;
	ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
	mdio_data.nAddressDev = nPortID;
	mdio_data.nAddressReg = 0x16;
	IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
	value = GSWIP_GET_BITS(mdio_data.nData, 10, 10);
	if (value)
		bLinkStatus = LTQ_TRUE;
	else
		bLinkStatus = LTQ_FALSE;
	return bLinkStatus;
}

ltq_bool_t PHY_Link_Status_Get(void *pDevCtx, u8 nPortID)
{
	IFX_ETHSW_MDIO_data_t mdio_data;
    u32 value;
    ethsw_api_dev_t *pEthDev = (ethsw_api_dev_t *)pDevCtx;
    mdio_data.nAddressDev = nPortID;
    mdio_data.nAddressReg = 0x1;
    IFX_FLOW_MDIO_DataRead(pEthDev, &mdio_data);
		if (mdio_data.nData != 0xFFFF) {
			value = GSWIP_GET_BITS(mdio_data.nData, 2, 2);
			if (value)
				return LTQ_TRUE;
			else
				return LTQ_FALSE;
		} else
			return LTQ_FALSE;
}

#endif /* LTQ_ETHSW_API_COC */
