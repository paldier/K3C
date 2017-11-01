/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _LTQ_GSW30_SOC_TOP_H_
#define _LTQ_GSW30_SOC_TOP_H_

/* ----------------------------------------------- */
#define GSW30_TOP_OFFSET	0xF00	/* 0x3C00 */
/* ----------------------------------------------- */
/* Register: 'Global Control Register0' */
/* Bit: 'SE' */
/* Description: 'Global Switch Macro Enable' */
#define GSWT_GCTRL_SE_OFFSET	0x000
#define GSWT_GCTRL_SE_SHIFT	15
#define GSWT_GCTRL_SE_SIZE	1
/* Bit: 'HWRES' */
/* Description: 'Global Hardware Reset' */
#define GSWT_GCTRL_HWRES_OFFSET	0x000
#define GSWT_GCTRL_HWRES_SHIFT	1
#define GSWT_GCTRL_HWRES_SIZE		1
/* Bit: 'SWRES' */
/* Description: 'Global Software Reset' */
#define GSWT_GCTRL_SWRES_OFFSET	0x000
#define GSWT_GCTRL_SWRES_SHIFT	0
#define GSWT_GCTRL_SWRES_SIZE		1
/* ----------------------------------------------- */
/* Register: 'MDIO Control Register' */
/* Bit: 'MBUSY' */
/* Description: 'MDIO Busy' */
#define GSWT_MDCTRL_MBUSY_OFFSET	0x004
#define GSWT_MDCTRL_MBUSY_SHIFT		12
#define GSWT_MDCTRL_MBUSY_SIZE		1
/* Bit: 'OP' */
/* Description: 'Operation Code' */
#define GSWT_MDCTRL_OP_OFFSET		0x004
#define GSWT_MDCTRL_OP_SHIFT		10
#define GSWT_MDCTRL_OP_SIZE			2
/* Bit: 'PHYAD' */
/* Description: 'PHY Address' */
#define GSWT_MDCTRL_PHYAD_OFFSET	0x004
#define GSWT_MDCTRL_PHYAD_SHIFT		5
#define GSWT_MDCTRL_PHYAD_SIZE		5
/* Bit: 'REGAD' */
/* Description: 'Register Address' */
#define GSWT_MDCTRL_REGAD_OFFSET	0x004
#define GSWT_MDCTRL_REGAD_SHIFT		0
#define GSWT_MDCTRL_REGAD_SIZE		5
/* ----------------------------------------------- */
/* Register: 'MDIO Read Data Register' */
/* Bit: 'RDATA' */
/* Description: 'Read Data' */
#define GSWT_MDREAD_RDATA_OFFSET	0x005
#define GSWT_MDREAD_RDATA_SHIFT		0
#define GSWT_MDREAD_RDATA_SIZE		16
/* ----------------------------------------------- */
/* Register: 'MDIO Write Data Register' */
/* Bit: 'WDATA' */
/* Description: 'Write Data' */
#define GSWT_MDWRITE_WDATA_OFFSET	0x006
#define GSWT_MDWRITE_WDATA_SHIFT	0
#define GSWT_MDWRITE_WDATA_SIZE		16
/* ----------------------------------------------- */
/* Register: 'MDC Clock ConfigurationRegister 0' */
/* Bit: 'PEN_6' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_6_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_6_SHIFT		6
#define GSWT_MDCCFG_0_PEN_6_SIZE		1
/* Bit: 'PEN_5' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_5_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_5_SHIFT		5
#define GSWT_MDCCFG_0_PEN_5_SIZE		1
/* Bit: 'PEN_4' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_4_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_4_SHIFT		4
#define GSWT_MDCCFG_0_PEN_4_SIZE		1
/* Bit: 'PEN_3' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_3_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_3_SHIFT		3
#define GSWT_MDCCFG_0_PEN_3_SIZE		1
/* Bit: 'PEN_2' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_2_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_2_SHIFT		2
#define GSWT_MDCCFG_0_PEN_2_SIZE		1
/* Bit: 'PEN_1' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_1_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_1_SHIFT		1
#define GSWT_MDCCFG_0_PEN_1_SIZE		1
/* Bit: 'PEN_0' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_0_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_0_SHIFT		0
#define GSWT_MDCCFG_0_PEN_0_SIZE		1
/* Bit: 'PEN_0~PEN_5' */
/* Description: 'Polling State Machine Enable' */
#define GSWT_MDCCFG_0_PEN_ALL_OFFSET	0x007
#define GSWT_MDCCFG_0_PEN_ALL_SHIFT		1
#define GSWT_MDCCFG_0_PEN_ALL_SIZE		6
/* ----------------------------------------------- */
/* Register: 'MDC Clock ConfigurationRegister 1' */
/* Bit: 'RES' */
/* Description: 'MDIO Hardware Reset' */
#define GSWT_MDCCFG_1_RES_OFFSET	0x008
#define GSWT_MDCCFG_1_RES_SHIFT		15
#define GSWT_MDCCFG_1_RES_SIZE		1
/* Bit: 'GAP' */
/* Description: 'Autopolling Gap' */
#define GSWT_MDCCFG_1_GAP_OFFSET	0x008
#define GSWT_MDCCFG_1_GAP_SHIFT		9
#define GSWT_MDCCFG_1_GAP_SIZE		6
/* Bit: 'MCEN' */
/* Description: 'Management Clock Enable' */
#define GSWT_MDCCFG_1_MCEN_OFFSET	0x008
#define GSWT_MDCCFG_1_MCEN_SHIFT	8
#define GSWT_MDCCFG_1_MCEN_SIZE		1
/* Bit: 'FREQ' */
/* Description: 'MDIO Interface Clock Rate' */
#define GSWT_MDCCFG_1_FREQ_OFFSET	0x008
#define GSWT_MDCCFG_1_FREQ_SHIFT	0
#define GSWT_MDCCFG_1_FREQ_SIZE		8
/* ----------------------------------------------- */
/* Register: 'xMII Port 1 ConfigurationRegister' */
/* Bit: 'RES' */
/* Description: 'Hardware Reset' */
#define GSWT_MII_CFG_1_RES_OFFSET	0x09
#define GSWT_MII_CFG_1_RES_SHIFT	15
#define GSWT_MII_CFG_1_RES_SIZE		1
/* Bit: 'EN' */
/* Description: 'xMII Interface Enable' */
#define GSWT_MII_CFG_1_EN_OFFSET		0x09
#define GSWT_MII_CFG_1_EN_SHIFT	14
#define GSWT_MII_CFG_1_EN_SIZE	1
/* Bit: 'ISOL' */
/* Description: 'ISOLATE xMII Interface' */
#define GSWT_MII_CFG_1_ISOL_OFFSET		0x09
#define GSWT_MII_CFG_1_ISOL_SHIFT	13
#define GSWT_MII_CFG_1_ISOL_SIZE	1
/* Bit: 'LDCLKDIS' */
/* Description: 'Link Down Clock Disable' */
#define GSWT_MII_CFG_1_LDCLKDIS_OFFSET	0x09
#define GSWT_MII_CFG_1_LDCLKDIS_SHIFT	12
#define GSWT_MII_CFG_1_LDCLKDIS_SIZE	1
/* Bit: 'CRS' */
/* Description: 'CRS Sensitivity Configuration' */
#define GSWT_MII_CFG_1_CRS_OFFSET	0x09
#define GSWT_MII_CFG_1_CRS_SHIFT	9
#define GSWT_MII_CFG_1_CRS_SIZE		2
/* Bit: 'RGMII_IBS' */
/* Description: 'RGMII In Band Status' */
#define GSWT_MII_CFG_1_RGMII_IBS_OFFSET	0x09
#define GSWT_MII_CFG_1_RGMII_IBS_SHIFT	8
#define GSWT_MII_CFG_1_RGMII_IBS_SIZE		1
/* Bit: 'RMII' */
/* Description: 'RMII Reference Clock Direction of the Port' */
#define GSWT_MII_CFG_1_RMII_OFFSET	0x09
#define GSWT_MII_CFG_1_RMII_SHIFT	7
#define GSWT_MII_CFG_1_RMII_SIZE	1
/* Bit: 'MIIRATE' */
/* Description: 'xMII Port Interface Clock Rate' */
#define GSWT_MII_CFG_1_MIIRATE_OFFSET	0x09
#define GSWT_MII_CFG_1_MIIRATE_SHIFT	4
#define GSWT_MII_CFG_1_MIIRATE_SIZE		3
/* Bit: 'MIIMODE' */
/* Description: 'xMII Interface Mode' */
#define GSWT_MII_CFG_1_MIIMODE_OFFSET	0x09
#define GSWT_MII_CFG_1_MIIMODE_SHIFT	0
#define GSWT_MII_CFG_1_MIIMODE_SIZE		4
/* ----------------------------------------------- */
/* Register: 'Configuration of ClockDelay for Port 1' */
/* Bit: 'RXLOCK' */
/* Description: 'Lock Status MDL of Receive PCDU' */
#define GSWT_PCDU_1_RXLOCK_OFFSET	0x0A
#define GSWT_PCDU_1_RXLOCK_SHIFT	15
#define GSWT_PCDU_1_RXLOCK_SIZE		1
/* Bit: 'TXLOCK' */
/* Description: 'Lock Status of MDL of Transmit PCDU' */
#define GSWT_PCDU_1_TXLOCK_OFFSET	0x0A
#define GSWT_PCDU_1_TXLOCK_SHIFT	14
#define GSWT_PCDU_1_TXLOCK_SIZE		1
/* Bit: 'DELMD' */
/* Description: 'PCDU Setting Mode' */
#define GSWT_PCDU_1_DELMD_OFFSET	0x0A
#define GSWT_PCDU_1_DELMD_SHIFT	10
#define GSWT_PCDU_1_DELMD_SIZE	1
/* Bit: 'RXDLY' */
/* Description: 'Configure Receive Clock Delay' */
#define GSWT_PCDU_1_RXDLY_OFFSET	0x0A
#define GSWT_PCDU_1_RXDLY_SHIFT	7
#define GSWT_PCDU_1_RXDLY_SIZE	3
/* Bit: 'TXDLY' */
/* Description: 'Configure Transmit PCDU' */
#define GSWT_PCDU_1_TXDLY_OFFSET	0x0A
#define GSWT_PCDU_1_TXDLY_SHIFT	0
#define GSWT_PCDU_1_TXDLY_SIZE	3
/* ----------------------------------------------- */
/* Register: 'Receive Buffer ControlRegister for Port 1' */
/* Bit: 'RBUF_UFL' */
/* Description: 'Receive Buffer Underflow Indicator' */
#define GSWT_RXB_CTL_1_RBUF_UFL_OFFSET	0x0B
#define GSWT_RXB_CTL_1_RBUF_UFL_SHIFT	15
#define GSWT_RXB_CTL_1_RBUF_UFL_SIZE	1
/* Bit: 'RBUF_OFL' */
/* Description: 'Receive Buffer Overflow Indicator' */
#define GSWT_RXB_CTL_1_RBUF_OFL_OFFSET	0x0B
#define GSWT_RXB_CTL_1_RBUF_OFL_SHIFT	14
#define GSWT_RXB_CTL_1_RBUF_OFL_SIZE	1
/* Bit: 'RBUF_DLY_WP' */
/* Description: 'Delay' */
#define GSWT_RXB_CTL_1_RBUF_DLY_WP_OFFSET	0x0B
#define GSWT_RXB_CTL_1_RBUF_DLY_WP_SHIFT		0
#define GSWT_RXB_CTL_1_RBUF_DLY_WP_SIZE		3
/* ----------------------------------------------- */
/* Register: 'Configure TX K value' */
/* Bit: 'KVAL' */
/* Description: 'K Value for TX Delay Path' */
#define GSWT_PCDU1_TX_KVAL_KVAL_OFFSET	0x0C
#define GSWT_PCDU1_TX_KVAL_KVAL_SHIFT	0
#define GSWT_PCDU1_TX_KVAL_KVAL_SIZE	16
/* ----------------------------------------------- */
/* Register: 'Configure TX M Blank' */
/* Bit: 'MBLK' */
/* Description: 'M Required for TX Delay Path' */
#define GSWT_PCDU1_TX_MBLK_MBLK_OFFSET	0x0D
#define GSWT_PCDU1_TX_MBLK_MBLK_SHIFT	0
#define GSWT_PCDU1_TX_MBLK_MBLK_SIZE	16
/* ----------------------------------------------- */
/* Register: 'Configure TX M Required' */
/* Bit: 'MREQ' */
/* Description: 'M Required for TX Delay Path' */
#define GSWT_PCDU1_TX_MREQ_MREQ_OFFSET	0x0E
#define GSWT_PCDU1_TX_MREQ_MREQ_SHIFT	0
#define GSWT_PCDU1_TX_MREQ_MREQ_SIZE	16
/* ----------------------------------------------- */
/* Register: 'Configure TX Delay Length' */
/* Bit: 'DEL_LEN' */
/* Description: 'Delay Length for TX Delay Path ' */
#define GSWT_PCDU1_TX_DELLEN_DEL_LEN_OFFSET	0x0F
#define GSWT_PCDU1_TX_DELLEN_DEL_LEN_SHIFT	0
#define GSWT_PCDU1_TX_DELLEN_DEL_LEN_SIZE	16
/* ----------------------------------------------- */
/* Register: 'Configure RX K value' */
/* Bit: 'KVAL' */
/* Description: 'K Value for RX Delay Path' */
#define GSWT_PCDU1_RX_KVAL_KVAL_OFFSET	0x10
#define GSWT_PCDU1_RX_KVAL_KVAL_SHIFT	0
#define GSWT_PCDU1_RX_KVAL_KVAL_SIZE	16
/* ----------------------------------------------- */
/* Register: 'Configure RX M Required' */
/* Bit: 'MREQ' */
/* Description: 'M Required for RX Delay Path' */
#define GSWT_PCDU1_RX_MREQ_MREQ_OFFSET	0x011
#define GSWT_PCDU1_RX_MREQ_MREQ_SHIFT	0
#define GSWT_PCDU1_RX_MREQ_MREQ_SIZE	16
/* ----------------------------------------------- */
/* Register: 'Configure RX M Blank' */
/* Bit: 'MBLK' */
/* Description: 'M Required for RX Delay Path' */
#define GSWT_PCDU1_RX_MBLK_MBLK_OFFSET	0x012
#define GSWT_PCDU1_RX_MBLK_MBLK_SHIFT	0
#define GSWT_PCDU1_RX_MBLK_MBLK_SIZE		16
/* ----------------------------------------------- */
/* Register: 'Configure RX Delay Length' */
/* Bit: 'DEL_LEN' */
/* Description: 'Delay Length for RX Delay Path ' */
#define GSWT_PCDU1_RX_DELLEN_DEL_LEN_OFFSET	0x013
#define GSWT_PCDU1_RX_DELLEN_DEL_LEN_SHIFT		0
#define GSWT_PCDU1_RX_DELLEN_DEL_LEN_SIZE		16
/* ----------------------------------------------- */
/* Bits: FSLSB	*/
/* Description: Timer Fractional Nano Second LSB Value */
#define GSWT_TIMER_FS_LSB_FSLSB_OFFSET	0x020
#define GSWT_TIMER_FS_LSB_FSLSB_SHIFT	0
#define GSWT_TIMER_FS_LSB_FSLSB_SIZE		16
/* --------------------------------------------------- */
/* Bits: FSMSB	*/
/* Description: Timer Fractional Nano Second MSB Value  */
#define GSWT_TIMER_FS_MSB_FSMSB_OFFSET	0x021
#define GSWT_TIMER_FS_MSB_FSMSB_SHIFT	0
#define GSWT_TIMER_FS_MSB_FSMSB_SIZE		16
/* --------------------------------------------------- */
/* Bits: NSLSB	*/
/* Description: Timer Nano Second LSB Register  */
#define GSWT_TIMER_NS_LSB_NSLSB_OFFSET	0x022
#define GSWT_TIMER_NS_LSB_NSLSB_SHIFT	0
#define GSWT_TIMER_NS_LSB_NSLSB_SIZE		16
/* --------------------------------------------------- */
/* Bits: NSMSB	*/
/* Description: Timer Nano Second MSB Register  */
#define GSWT_TIMER_NS_MSB_NSMSB_OFFSET	0x023
#define GSWT_TIMER_NS_MSB_NSMSB_SHIFT	0
#define GSWT_TIMER_NS_MSB_NSMSB_SIZE		16
/* --------------------------------------------------- */
/* Bits: SECLSB	*/
/* Description: Timer Second LSB Register  */
#define GSWT_TIMER_SEC_LSB_SECLSB_OFFSET	0x024
#define GSWT_TIMER_SEC_LSB_SECLSB_SHIFT	0
#define GSWT_TIMER_SEC_LSB_SECLSB_SIZE		16
/* --------------------------------------------------- */
/* Bits: SECMSB	*/
/* Description: Timer Second MSB Register  */
#define GSWT_TIMER_SEC_MSB_SECMSB_OFFSET	0x025
#define GSWT_TIMER_SEC_MSB_SECMSB_SHIFT	0
#define GSWT_TIMER_SEC_MSB_SECMSB_SIZE		16
/* --------------------------------------------------- */
/* Bit: WR	*/
/* Description: Write to Timer Command  (LTQ_GSWIP_2_2) */
#define GSWT_TIMER_CTRL_WR_OFFSET	0x026
#define GSWT_TIMER_CTRL_WR_SHIFT		15
#define GSWT_TIMER_CTRL_WR_SIZE		1
/* Bit: RD	*/
/* Description: Read from Timer Command  */
#define GSWT_TIMER_CTRL_RD_OFFSET	0x026
#define GSWT_TIMER_CTRL_RD_SHIFT		14
#define GSWT_TIMER_CTRL_RD_SIZE		1
/* Bit: ADJ	*/
/* Description: Adjust Timer Command  */
#define GSWT_TIMER_CTRL_ADJ_OFFSET	0x026
#define GSWT_TIMER_CTRL_ADJ_SHIFT	13
#define GSWT_TIMER_CTRL_ADJ_SIZE		1
/* --------------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define GSWT_PHY_ADDR_1_LNKST_OFFSET	0x044
#define GSWT_PHY_ADDR_1_LNKST_SHIFT	13
#define GSWT_PHY_ADDR_1_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_PHY_ADDR_1_SPEED_OFFSET	0x044
#define GSWT_PHY_ADDR_1_SPEED_SHIFT	11
#define GSWT_PHY_ADDR_1_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define GSWT_PHY_ADDR_1_FDUP_OFFSET	0x044
#define GSWT_PHY_ADDR_1_FDUP_SHIFT		9
#define GSWT_PHY_ADDR_1_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define GSWT_PHY_ADDR_1_FCONTX_OFFSET	0x044
#define GSWT_PHY_ADDR_1_FCONTX_SHIFT		7
#define GSWT_PHY_ADDR_1_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define GSWT_PHY_ADDR_1_FCONRX_OFFSET	0x044
#define GSWT_PHY_ADDR_1_FCONRX_SHIFT		5
#define GSWT_PHY_ADDR_1_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define GSWT_PHY_ADDR_1_ADDR_OFFSET	0x044
#define GSWT_PHY_ADDR_1_ADDR_SHIFT		0
#define GSWT_PHY_ADDR_1_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define GSWT_MDIO_STAT_1_CLK_STOP_CAPABLE_OFFSET	0x045
#define GSWT_MDIO_STAT_1_CLK_STOP_CAPABLE_SHIFT	8
#define GSWT_MDIO_STAT_1_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define GSWT_MDIO_STAT_1_EEE_CAPABLE_OFFSET	0x045
#define GSWT_MDIO_STAT_1_EEE_CAPABLE_SHIFT		7
#define GSWT_MDIO_STAT_1_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define GSWT_MDIO_STAT_1_PACT_OFFSET	0x045
#define GSWT_MDIO_STAT_1_PACT_SHIFT	6
#define GSWT_MDIO_STAT_1_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define GSWT_MDIO_STAT_1_LSTAT_OFFSET	0x045
#define GSWT_MDIO_STAT_1_LSTAT_SHIFT		5
#define GSWT_MDIO_STAT_1_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_MDIO_STAT_1_SPEED_OFFSET	0x045
#define GSWT_MDIO_STAT_1_SPEED_SHIFT		3
#define GSWT_MDIO_STAT_1_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define GSWT_MDIO_STAT_1_FDUP_OFFSET	0x045
#define GSWT_MDIO_STAT_1_FDUP_SHIFT	2
#define GSWT_MDIO_STAT_1_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define GSWT_MDIO_STAT_1_RXPAUEN_OFFSET	0x045
#define GSWT_MDIO_STAT_1_RXPAUEN_SHIFT		1
#define GSWT_MDIO_STAT_1_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define GSWT_MDIO_STAT_1_TXPAUEN_OFFSET	0x045
#define GSWT_MDIO_STAT_1_TXPAUEN_SHIFT		0
#define GSWT_MDIO_STAT_1_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 1*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_OFFSET	0x046
#define GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_SHIFT		2
#define GSWT_ANEG_EEE_1_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define GSWT_ANEG_EEE_1_EEE_CAPABLE_OFFSET	0x046
#define GSWT_ANEG_EEE_1_EEE_CAPABLE_SHIFT	0
#define GSWT_ANEG_EEE_1_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define GSWT_PHY_ADDR_2_LNKST_OFFSET	0x048
#define GSWT_PHY_ADDR_2_LNKST_SHIFT	13
#define GSWT_PHY_ADDR_2_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_PHY_ADDR_2_SPEED_OFFSET	0x048
#define GSWT_PHY_ADDR_2_SPEED_SHIFT	11
#define GSWT_PHY_ADDR_2_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define GSWT_PHY_ADDR_2_FDUP_OFFSET	0x048
#define GSWT_PHY_ADDR_2_FDUP_SHIFT		9
#define GSWT_PHY_ADDR_2_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define GSWT_PHY_ADDR_2_FCONTX_OFFSET	0x048
#define GSWT_PHY_ADDR_2_FCONTX_SHIFT		7
#define GSWT_PHY_ADDR_2_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define GSWT_PHY_ADDR_2_FCONRX_OFFSET	0x048
#define GSWT_PHY_ADDR_2_FCONRX_SHIFT		5
#define GSWT_PHY_ADDR_2_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define GSWT_PHY_ADDR_2_ADDR_OFFSET	0x048
#define GSWT_PHY_ADDR_2_ADDR_SHIFT		0
#define GSWT_PHY_ADDR_2_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define GSWT_MDIO_STAT_2_CLK_STOP_CAPABLE_OFFSET	0x049
#define GSWT_MDIO_STAT_2_CLK_STOP_CAPABLE_SHIFT	8
#define GSWT_MDIO_STAT_2_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define GSWT_MDIO_STAT_2_EEE_CAPABLE_OFFSET	0x049
#define GSWT_MDIO_STAT_2_EEE_CAPABLE_SHIFT		7
#define GSWT_MDIO_STAT_2_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define GSWT_MDIO_STAT_2_PACT_OFFSET	0x049
#define GSWT_MDIO_STAT_2_PACT_SHIFT	6
#define GSWT_MDIO_STAT_2_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define GSWT_MDIO_STAT_2_LSTAT_OFFSET	0x049
#define GSWT_MDIO_STAT_2_LSTAT_SHIFT		5
#define GSWT_MDIO_STAT_2_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_MDIO_STAT_2_SPEED_OFFSET	0x049
#define GSWT_MDIO_STAT_2_SPEED_SHIFT		3
#define GSWT_MDIO_STAT_2_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define GSWT_MDIO_STAT_2_FDUP_OFFSET	0x049
#define GSWT_MDIO_STAT_2_FDUP_SHIFT	2
#define GSWT_MDIO_STAT_2_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define GSWT_MDIO_STAT_2_RXPAUEN_OFFSET	0x049
#define GSWT_MDIO_STAT_2_RXPAUEN_SHIFT		1
#define GSWT_MDIO_STAT_2_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define GSWT_MDIO_STAT_2_TXPAUEN_OFFSET	0x049
#define GSWT_MDIO_STAT_2_TXPAUEN_SHIFT		0
#define GSWT_MDIO_STAT_2_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 2*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define GSWT_ANEG_EEE_2_CLK_STOP_CAPABLE_OFFSET	0x04A
#define GSWT_ANEG_EEE_2_CLK_STOP_CAPABLE_SHIFT		2
#define GSWT_ANEG_EEE_2_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define GSWT_ANEG_EEE_2_EEE_CAPABLE_OFFSET	0x04A
#define GSWT_ANEG_EEE_2_EEE_CAPABLE_SHIFT	0
#define GSWT_ANEG_EEE_2_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define GSWT_PHY_ADDR_3_LNKST_OFFSET	0x04C
#define GSWT_PHY_ADDR_3_LNKST_SHIFT	13
#define GSWT_PHY_ADDR_3_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_PHY_ADDR_3_SPEED_OFFSET	0x04C
#define GSWT_PHY_ADDR_3_SPEED_SHIFT	11
#define GSWT_PHY_ADDR_3_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define GSWT_PHY_ADDR_3_FDUP_OFFSET	0x04C
#define GSWT_PHY_ADDR_3_FDUP_SHIFT		9
#define GSWT_PHY_ADDR_3_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define GSWT_PHY_ADDR_3_FCONTX_OFFSET	0x04C
#define GSWT_PHY_ADDR_3_FCONTX_SHIFT		7
#define GSWT_PHY_ADDR_3_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define GSWT_PHY_ADDR_3_FCONRX_OFFSET	0x04C
#define GSWT_PHY_ADDR_3_FCONRX_SHIFT		5
#define GSWT_PHY_ADDR_3_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define GSWT_PHY_ADDR_3_ADDR_OFFSET	0x04C
#define GSWT_PHY_ADDR_3_ADDR_SHIFT		0
#define GSWT_PHY_ADDR_3_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define GSWT_MDIO_STAT_3_CLK_STOP_CAPABLE_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_CLK_STOP_CAPABLE_SHIFT	8
#define GSWT_MDIO_STAT_3_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define GSWT_MDIO_STAT_3_EEE_CAPABLE_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_EEE_CAPABLE_SHIFT		7
#define GSWT_MDIO_STAT_3_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define GSWT_MDIO_STAT_3_PACT_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_PACT_SHIFT	6
#define GSWT_MDIO_STAT_3_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define GSWT_MDIO_STAT_3_LSTAT_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_LSTAT_SHIFT		5
#define GSWT_MDIO_STAT_3_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_MDIO_STAT_3_SPEED_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_SPEED_SHIFT		3
#define GSWT_MDIO_STAT_3_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define GSWT_MDIO_STAT_3_FDUP_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_FDUP_SHIFT	2
#define GSWT_MDIO_STAT_3_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define GSWT_MDIO_STAT_3_RXPAUEN_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_RXPAUEN_SHIFT		1
#define GSWT_MDIO_STAT_3_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define GSWT_MDIO_STAT_3_TXPAUEN_OFFSET	0x04D
#define GSWT_MDIO_STAT_3_TXPAUEN_SHIFT		0
#define GSWT_MDIO_STAT_3_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 3*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define GSWT_ANEG_EEE_3_CLK_STOP_CAPABLE_OFFSET	0x04E
#define GSWT_ANEG_EEE_3_CLK_STOP_CAPABLE_SHIFT		2
#define GSWT_ANEG_EEE_3_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define GSWT_ANEG_EEE_3_EEE_CAPABLE_OFFSET	0x04E
#define GSWT_ANEG_EEE_3_EEE_CAPABLE_SHIFT	0
#define GSWT_ANEG_EEE_3_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define GSWT_PHY_ADDR_4_LNKST_OFFSET	0x050
#define GSWT_PHY_ADDR_4_LNKST_SHIFT	13
#define GSWT_PHY_ADDR_4_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_PHY_ADDR_4_SPEED_OFFSET	0x050
#define GSWT_PHY_ADDR_4_SPEED_SHIFT	11
#define GSWT_PHY_ADDR_4_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define GSWT_PHY_ADDR_4_FDUP_OFFSET	0x050
#define GSWT_PHY_ADDR_4_FDUP_SHIFT		9
#define GSWT_PHY_ADDR_4_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define GSWT_PHY_ADDR_4_FCONTX_OFFSET	0x050
#define GSWT_PHY_ADDR_4_FCONTX_SHIFT		7
#define GSWT_PHY_ADDR_4_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define GSWT_PHY_ADDR_4_FCONRX_OFFSET	0x050
#define GSWT_PHY_ADDR_4_FCONRX_SHIFT		5
#define GSWT_PHY_ADDR_4_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define GSWT_PHY_ADDR_4_ADDR_OFFSET	0x050
#define GSWT_PHY_ADDR_4_ADDR_SHIFT		0
#define GSWT_PHY_ADDR_4_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define GSWT_MDIO_STAT_4_CLK_STOP_CAPABLE_OFFSET	0x051
#define GSWT_MDIO_STAT_4_CLK_STOP_CAPABLE_SHIFT	8
#define GSWT_MDIO_STAT_4_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define GSWT_MDIO_STAT_4_EEE_CAPABLE_OFFSET	0x051
#define GSWT_MDIO_STAT_4_EEE_CAPABLE_SHIFT		7
#define GSWT_MDIO_STAT_4_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define GSWT_MDIO_STAT_4_PACT_OFFSET	0x051
#define GSWT_MDIO_STAT_4_PACT_SHIFT	6
#define GSWT_MDIO_STAT_4_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define GSWT_MDIO_STAT_4_LSTAT_OFFSET	0x051
#define GSWT_MDIO_STAT_4_LSTAT_SHIFT		5
#define GSWT_MDIO_STAT_4_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_MDIO_STAT_4_SPEED_OFFSET	0x051
#define GSWT_MDIO_STAT_4_SPEED_SHIFT		3
#define GSWT_MDIO_STAT_4_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define GSWT_MDIO_STAT_4_FDUP_OFFSET	0x051
#define GSWT_MDIO_STAT_4_FDUP_SHIFT	2
#define GSWT_MDIO_STAT_4_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define GSWT_MDIO_STAT_4_RXPAUEN_OFFSET	0x051
#define GSWT_MDIO_STAT_4_RXPAUEN_SHIFT		1
#define GSWT_MDIO_STAT_4_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define GSWT_MDIO_STAT_4_TXPAUEN_OFFSET	0x051
#define GSWT_MDIO_STAT_4_TXPAUEN_SHIFT		0
#define GSWT_MDIO_STAT_4_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 4*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define GSWT_ANEG_EEE_4_CLK_STOP_CAPABLE_OFFSET	0x052
#define GSWT_ANEG_EEE_4_CLK_STOP_CAPABLE_SHIFT		2
#define GSWT_ANEG_EEE_4_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define GSWT_ANEG_EEE_4_EEE_CAPABLE_OFFSET	0x052
#define GSWT_ANEG_EEE_4_EEE_CAPABLE_SHIFT	0
#define GSWT_ANEG_EEE_4_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define GSWT_PHY_ADDR_5_LNKST_OFFSET	0x054
#define GSWT_PHY_ADDR_5_LNKST_SHIFT	13
#define GSWT_PHY_ADDR_5_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_PHY_ADDR_5_SPEED_OFFSET	0x054
#define GSWT_PHY_ADDR_5_SPEED_SHIFT	11
#define GSWT_PHY_ADDR_5_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define GSWT_PHY_ADDR_5_FDUP_OFFSET	0x054
#define GSWT_PHY_ADDR_5_FDUP_SHIFT		9
#define GSWT_PHY_ADDR_5_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define GSWT_PHY_ADDR_5_FCONTX_OFFSET	0x054
#define GSWT_PHY_ADDR_5_FCONTX_SHIFT		7
#define GSWT_PHY_ADDR_5_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define GSWT_PHY_ADDR_5_FCONRX_OFFSET	0x054
#define GSWT_PHY_ADDR_5_FCONRX_SHIFT		5
#define GSWT_PHY_ADDR_5_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define GSWT_PHY_ADDR_5_ADDR_OFFSET	0x054
#define GSWT_PHY_ADDR_5_ADDR_SHIFT		0
#define GSWT_PHY_ADDR_5_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define GSWT_MDIO_STAT_5_CLK_STOP_CAPABLE_OFFSET	0x055
#define GSWT_MDIO_STAT_5_CLK_STOP_CAPABLE_SHIFT	8
#define GSWT_MDIO_STAT_5_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define GSWT_MDIO_STAT_5_EEE_CAPABLE_OFFSET	0x055
#define GSWT_MDIO_STAT_5_EEE_CAPABLE_SHIFT		7
#define GSWT_MDIO_STAT_5_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define GSWT_MDIO_STAT_5_PACT_OFFSET	0x055
#define GSWT_MDIO_STAT_5_PACT_SHIFT	6
#define GSWT_MDIO_STAT_5_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define GSWT_MDIO_STAT_5_LSTAT_OFFSET	0x055
#define GSWT_MDIO_STAT_5_LSTAT_SHIFT		5
#define GSWT_MDIO_STAT_5_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_MDIO_STAT_5_SPEED_OFFSET	0x055
#define GSWT_MDIO_STAT_5_SPEED_SHIFT		3
#define GSWT_MDIO_STAT_5_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define GSWT_MDIO_STAT_5_FDUP_OFFSET	0x055
#define GSWT_MDIO_STAT_5_FDUP_SHIFT	2
#define GSWT_MDIO_STAT_5_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define GSWT_MDIO_STAT_5_RXPAUEN_OFFSET	0x055
#define GSWT_MDIO_STAT_5_RXPAUEN_SHIFT		1
#define GSWT_MDIO_STAT_5_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define GSWT_MDIO_STAT_5_TXPAUEN_OFFSET	0x055
#define GSWT_MDIO_STAT_5_TXPAUEN_SHIFT		0
#define GSWT_MDIO_STAT_5_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 5*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define GSWT_ANEG_EEE_5_CLK_STOP_CAPABLE_OFFSET	0x056
#define GSWT_ANEG_EEE_5_CLK_STOP_CAPABLE_SHIFT		2
#define GSWT_ANEG_EEE_5_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define GSWT_ANEG_EEE_5_EEE_CAPABLE_OFFSET	0x056
#define GSWT_ANEG_EEE_5_EEE_CAPABLE_SHIFT	0
#define GSWT_ANEG_EEE_5_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define GSWT_PHY_ADDR_6_LNKST_OFFSET	0x058
#define GSWT_PHY_ADDR_6_LNKST_SHIFT	13
#define GSWT_PHY_ADDR_6_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_PHY_ADDR_6_SPEED_OFFSET	0x058
#define GSWT_PHY_ADDR_6_SPEED_SHIFT	11
#define GSWT_PHY_ADDR_6_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define GSWT_PHY_ADDR_6_FDUP_OFFSET	0x058
#define GSWT_PHY_ADDR_6_FDUP_SHIFT		9
#define GSWT_PHY_ADDR_6_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define GSWT_PHY_ADDR_6_FCONTX_OFFSET	0x058
#define GSWT_PHY_ADDR_6_FCONTX_SHIFT		7
#define GSWT_PHY_ADDR_6_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define GSWT_PHY_ADDR_6_FCONRX_OFFSET	0x058
#define GSWT_PHY_ADDR_6_FCONRX_SHIFT		5
#define GSWT_PHY_ADDR_6_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define GSWT_PHY_ADDR_6_ADDR_OFFSET	0x058
#define GSWT_PHY_ADDR_6_ADDR_SHIFT		0
#define GSWT_PHY_ADDR_6_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define GSWT_MDIO_STAT_6_CLK_STOP_CAPABLE_OFFSET	0x059
#define GSWT_MDIO_STAT_6_CLK_STOP_CAPABLE_SHIFT	8
#define GSWT_MDIO_STAT_6_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define GSWT_MDIO_STAT_6_EEE_CAPABLE_OFFSET	0x059
#define GSWT_MDIO_STAT_6_EEE_CAPABLE_SHIFT		7
#define GSWT_MDIO_STAT_6_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define GSWT_MDIO_STAT_6_PACT_OFFSET	0x059
#define GSWT_MDIO_STAT_6_PACT_SHIFT	6
#define GSWT_MDIO_STAT_6_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define GSWT_MDIO_STAT_6_LSTAT_OFFSET	0x059
#define GSWT_MDIO_STAT_6_LSTAT_SHIFT		5
#define GSWT_MDIO_STAT_6_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define GSWT_MDIO_STAT_6_SPEED_OFFSET	0x059
#define GSWT_MDIO_STAT_6_SPEED_SHIFT		3
#define GSWT_MDIO_STAT_6_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define GSWT_MDIO_STAT_6_FDUP_OFFSET	0x059
#define GSWT_MDIO_STAT_6_FDUP_SHIFT	2
#define GSWT_MDIO_STAT_6_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define GSWT_MDIO_STAT_6_RXPAUEN_OFFSET	0x059
#define GSWT_MDIO_STAT_6_RXPAUEN_SHIFT		1
#define GSWT_MDIO_STAT_6_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define GSWT_MDIO_STAT_6_TXPAUEN_OFFSET	0x059
#define GSWT_MDIO_STAT_6_TXPAUEN_SHIFT		0
#define GSWT_MDIO_STAT_6_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 6*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define GSWT_ANEG_EEE_6_CLK_STOP_CAPABLE_OFFSET	0x05A
#define GSWT_ANEG_EEE_6_CLK_STOP_CAPABLE_SHIFT	2
#define GSWT_ANEG_EEE_6_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define GSWT_ANEG_EEE_6_EEE_CAPABLE_OFFSET	0x05A
#define GSWT_ANEG_EEE_6_EEE_CAPABLE_SHIFT	0
#define GSWT_ANEG_EEE_6_EEE_CAPABLE_SIZE	2
/* ----------------------------------------------- */
/* Register: 'GPHY2 General Pin Strapping Configuration Register*/
/* Bit: 'CFG' */
/* Description: 'GPHY General Pin Strapping Configuration' */
#define GSWT_GPHY2_CFG_CFG_OFFSET	0x088
#define GSWT_GPHY2_CFG_CFG_SHIFT	0
#define GSWT_GPHY2_CFG_CFG_SIZE		8
/* ----------------------------------------------- */
/* Register: 'GPHY2 Base Frequency Deviation Configuration Register*/
/* Bit: 'BFDEV' */
/* Description: 'Base Frequency Deviation' */
#define GSWT_GPHY2_BFDEV_BFDEV_OFFSET	0x089
#define GSWT_GPHY2_BFDEV_BFDEV_SHIFT	0
#define GSWT_GPHY2_BFDEV_BFDEV_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY2 Firmware Base Address LSB Register */
/* Bit: 'BASELSB' */
/* Description: 'GPHY Firmware Base Address LSB (bit 15 to 0) ' */
#define GSWT_GPHY2_LBADR_BASELSB_OFFSET	0x08A
#define GSWT_GPHY2_LBADR_BASELSB_SHIFT	0
#define GSWT_GPHY2_LBADR_BASELSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY2 Firmware Base Address MSB Register */
/* Bit: 'BASEMSB' */
/* Description: 'GPHY Firmware Base Address MSB (bit 31 to 16) ' */
#define GSWT_GPHY2_MBADR_BASEMSB_OFFSET	0x08B
#define GSWT_GPHY2_MBADR_BASEMSB_SHIFT	0
#define GSWT_GPHY2_MBADR_BASEMSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY3 General Pin Strapping Configuration Register*/
/* Bit: 'CFG' */
/* Description: 'GPHY General Pin Strapping Configuration' */
#define GSWT_GPHY3_CFG_CFG_OFFSET	0x08C
#define GSWT_GPHY3_CFG_CFG_SHIFT	0
#define GSWT_GPHY3_CFG_CFG_SIZE		8
/* ----------------------------------------------- */
/* Register: 'GPHY3 Base Frequency Deviation Configuration Register*/
/* Bit: 'BFDEV' */
/* Description: 'Base Frequency Deviation' */
#define GSWT_GPHY3_BFDEV_BFDEV_OFFSET	0x08D
#define GSWT_GPHY3_BFDEV_BFDEV_SHIFT	0
#define GSWT_GPHY3_BFDEV_BFDEV_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY3 Firmware Base Address LSB Register */
/* Bit: 'BASELSB' */
/* Description: 'GPHY Firmware Base Address LSB (bit 15 to 0) ' */
#define GSWT_GPHY3_LBADR_BASELSB_OFFSET	0x08E
#define GSWT_GPHY3_LBADR_BASELSB_SHIFT	0
#define GSWT_GPHY3_LBADR_BASELSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY3 Firmware Base Address MSB Register */
/* Bit: 'BASEMSB' */
/* Description: 'GPHY Firmware Base Address MSB (bit 31 to 16) ' */
#define GSWT_GPHY3_MBADR_BASEMSB_OFFSET	0x08F
#define GSWT_GPHY3_MBADR_BASEMSB_SHIFT	0
#define GSWT_GPHY3_MBADR_BASEMSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY4 General Pin Strapping Configuration Register*/
/* Bit: 'CFG' */
/* Description: 'GPHY General Pin Strapping Configuration' */
#define GSWT_GPHY4_CFG_CFG_OFFSET	0x090
#define GSWT_GPHY4_CFG_CFG_SHIFT	0
#define GSWT_GPHY4_CFG_CFG_SIZE		8
/* ----------------------------------------------- */
/* Register: 'GPHY4 Base Frequency Deviation Configuration Register*/
/* Bit: 'BFDEV' */
/* Description: 'Base Frequency Deviation' */
#define GSWT_GPHY4_BFDEV_BFDEV_OFFSET	0x091
#define GSWT_GPHY4_BFDEV_BFDEV_SHIFT	0
#define GSWT_GPHY4_BFDEV_BFDEV_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY4 Firmware Base Address LSB Register */
/* Bit: 'BASELSB' */
/* Description: 'GPHY Firmware Base Address LSB (bit 15 to 0) ' */
#define GSWT_GPHY4_LBADR_BASELSB_OFFSET	0x092
#define GSWT_GPHY4_LBADR_BASELSB_SHIFT	0
#define GSWT_GPHY4_LBADR_BASELSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY4 Firmware Base Address MSB Register */
/* Bit: 'BASEMSB' */
/* Description: 'GPHY Firmware Base Address MSB (bit 31 to 16) ' */
#define GSWT_GPHY4_MBADR_BASEMSB_OFFSET	0x093
#define GSWT_GPHY4_MBADR_BASEMSB_SHIFT	0
#define GSWT_GPHY4_MBADR_BASEMSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY5 General Pin Strapping Configuration Register*/
/* Bit: 'CFG' */
/* Description: 'GPHY General Pin Strapping Configuration' */
#define GSWT_GPHY5_CFG_CFG_OFFSET	0x094
#define GSWT_GPHY5_CFG_CFG_SHIFT	0
#define GSWT_GPHY5_CFG_CFG_SIZE		8
/* ----------------------------------------------- */
/* Register: 'GPHY5 Base Frequency Deviation Configuration Register*/
/* Bit: 'BFDEV' */
/* Description: 'Base Frequency Deviation' */
#define GSWT_GPHY5_BFDEV_BFDEV_OFFSET	0x095
#define GSWT_GPHY5_BFDEV_BFDEV_SHIFT	0
#define GSWT_GPHY5_BFDEV_BFDEV_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY5 Firmware Base Address LSB Register */
/* Bit: 'BASELSB' */
/* Description: 'GPHY Firmware Base Address LSB (bit 15 to 0) ' */
#define GSWT_GPHY5_LBADR_BASELSB_OFFSET	0x096
#define GSWT_GPHY5_LBADR_BASELSB_SHIFT	0
#define GSWT_GPHY5_LBADR_BASELSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY5 Firmware Base Address MSB Register */
/* Bit: 'BASEMSB' */
/* Description: 'GPHY Firmware Base Address MSB (bit 31 to 16) ' */
#define GSWT_GPHY5_MBADR_BASEMSB_OFFSET	0x097
#define GSWT_GPHY5_MBADR_BASEMSB_SHIFT	0
#define GSWT_GPHY5_MBADR_BASEMSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY6F General Pin Strapping Configuration Register*/
/* Bit: 'CFG' */
/* Description: 'GPHY General Pin Strapping Configuration' */
#define GSWT_GPHY6F_CFG_CFG_OFFSET	0x098
#define GSWT_GPHY6F_CFG_CFG_SHIFT		0
#define GSWT_GPHY6F_CFG_CFG_SIZE		8
/* ----------------------------------------------- */
/* Register: 'GPHY6F Base Frequency Deviation Configuration Register*/
/* Bit: 'BFDEV' */
/* Description: 'Base Frequency Deviation' */
#define GSWT_GPHY6F_BFDEV_BFDEV_OFFSET	0x099
#define GSWT_GPHY6F_BFDEV_BFDEV_SHIFT		0
#define GSWT_GPHY6F_BFDEV_BFDEV_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY6F Firmware Base Address LSB Register */
/* Bit: 'BASELSB' */
/* Description: 'GPHY Firmware Base Address LSB (bit 15 to 0) ' */
#define GSWT_GPHY6F_LBADR_BASELSB_OFFSET	0x09A
#define GSWT_GPHY6F_LBADR_BASELSB_SHIFT		0
#define GSWT_GPHY6F_LBADR_BASELSB_SIZE		16
/* ----------------------------------------------- */
/* Register: 'GPHY6F Firmware Base Address MSB Register */
/* Bit: 'BASEMSB' */
/* Description: 'GPHY Firmware Base Address MSB (bit 31 to 16) ' */
#define GSWT_GPHY6F_MBADR_BASEMSB_OFFSET	0x09B
#define GSWT_GPHY6F_MBADR_BASEMSB_SHIFT		0
#define GSWT_GPHY6F_MBADR_BASEMSB_SIZE		16
/* ----------------------------------------------- */
#endif /*  _LTQ_GSW30_SOC_TOP_H_ */
