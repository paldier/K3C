/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _LTQ_ETHSW_SOC_TOP_H_
#define _LTQ_ETHSW_SOC_TOP_H_
/* ----------------------------------------------- */
/* Register: 'Global Control Register0' */
/* Bit: 'SE' */
/* Description: 'Global Switch Macro Enable' */
#define GLOB_CTRL_SE_OFFSET	0x000
#define GLOB_CTRL_SE_SHIFT	15
#define GLOB_CTRL_SE_SIZE		1
/* Bit: 'HWRES' */
/* Description: 'Global Hardware Reset' */
#define GLOB_CTRL_HWRES_OFFSET	0x000
#define GLOB_CTRL_HWRES_SHIFT		1
#define GLOB_CTRL_HWRES_SIZE		1
/* Bit: 'SWRES' */
/* Description: 'Global Software Reset' */
#define GLOB_CTRL_SWRES_OFFSET	0x000
#define GLOB_CTRL_SWRES_SHIFT		0
#define GLOB_CTRL_SWRES_SIZE		1
/* ----------------------------------------------- */
/* Register: 'MDIO Control Register' */
/* Bit: 'MBUSY' */
/* Description: 'MDIO Busy' */
#define MDIO_CTRL_MBUSY_OFFSET	0x008
#define MDIO_CTRL_MBUSY_SHIFT		12
#define MDIO_CTRL_MBUSY_SIZE		1
/* Bit: 'OP' */
/* Description: 'Operation Code' */
#define MDIO_CTRL_OP_OFFSET		0x008
#define MDIO_CTRL_OP_SHIFT		10
#define MDIO_CTRL_OP_SIZE			2
/* Bit: 'PHYAD' */
/* Description: 'PHY Address' */
#define MDIO_CTRL_PHYAD_OFFSET	0x008
#define MDIO_CTRL_PHYAD_SHIFT		5
#define MDIO_CTRL_PHYAD_SIZE		5
/* Bit: 'REGAD' */
/* Description: 'Register Address' */
#define MDIO_CTRL_REGAD_OFFSET	0x008
#define MDIO_CTRL_REGAD_SHIFT		0
#define MDIO_CTRL_REGAD_SIZE		5
/* ----------------------------------------------- */
/* Register: 'MDIO Read Data Register' */
/* Bit: 'RDATA' */
/* Description: 'Read Data' */
#define MDIO_READ_RDATA_OFFSET	0x009
#define MDIO_READ_RDATA_SHIFT		0
#define MDIO_READ_RDATA_SIZE		16
/* ----------------------------------------------- */
/* Register: 'MDIO Write Data Register' */
/* Bit: 'WDATA' */
/* Description: 'Write Data' */
#define MDIO_WRITE_WDATA_OFFSET	0x00A
#define MDIO_WRITE_WDATA_SHIFT	0
#define MDIO_WRITE_WDATA_SIZE		16
/* ----------------------------------------------- */
/* Register: 'MDC Clock ConfigurationRegister 0' */
/* Bit: 'PEN_5' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_5_OFFSET	0x00B
#define MDC_CFG_0_PEN_5_SHIFT		5
#define MDC_CFG_0_PEN_5_SIZE		1
/* Bit: 'PEN_4' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_4_OFFSET	0x00B
#define MDC_CFG_0_PEN_4_SHIFT		4
#define MDC_CFG_0_PEN_4_SIZE		1
/* Bit: 'PEN_3' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_3_OFFSET	0x00B
#define MDC_CFG_0_PEN_3_SHIFT		3
#define MDC_CFG_0_PEN_3_SIZE		1
/* Bit: 'PEN_2' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_2_OFFSET	0x00B
#define MDC_CFG_0_PEN_2_SHIFT		2
#define MDC_CFG_0_PEN_2_SIZE		1
/* Bit: 'PEN_1' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_1_OFFSET	0x00B
#define MDC_CFG_0_PEN_1_SHIFT		1
#define MDC_CFG_0_PEN_1_SIZE		1
/* Bit: 'PEN_0' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_0_OFFSET	0x00B
#define MDC_CFG_0_PEN_0_SHIFT		0
#define MDC_CFG_0_PEN_0_SIZE		1
/* Bit: 'PEN_0~PEN_5' */
/* Description: 'Polling State Machine Enable' */
#define MDC_CFG_0_PEN_ALL_OFFSET	0x00B
#define MDC_CFG_0_PEN_ALL_SHIFT		0
#define MDC_CFG_0_PEN_ALL_SIZE		6
/* ----------------------------------------------- */
/* Register: 'MDC Clock ConfigurationRegister 1' */
/* Bit: 'RES' */
/* Description: 'MDIO Hardware Reset' */
#define MDC_CFG_1_RES_OFFSET	0x00C
#define MDC_CFG_1_RES_SHIFT		15
#define MDC_CFG_1_RES_SIZE		1
/* Bit: 'MCEN' */
/* Description: 'Management Clock Enable' */
#define MDC_CFG_1_MCEN_OFFSET	0x00C
#define MDC_CFG_1_MCEN_SHIFT	8
#define MDC_CFG_1_MCEN_SIZE		1
/* Bit: 'FREQ' */
/* Description: 'MDIO Interface Clock Rate' */
#define MDC_CFG_1_FREQ_OFFSET	0x00C
#define MDC_CFG_1_FREQ_SHIFT	0
#define MDC_CFG_1_FREQ_SIZE		8
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 5' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define PHY_ADDR_5_LNKST_OFFSET	0x010
#define PHY_ADDR_5_LNKST_SHIFT	13
#define PHY_ADDR_5_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PHY_ADDR_5_SPEED_OFFSET	0x010
#define PHY_ADDR_5_SPEED_SHIFT	11
#define PHY_ADDR_5_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define PHY_ADDR_5_FDUP_OFFSET	0x010
#define PHY_ADDR_5_FDUP_SHIFT		9
#define PHY_ADDR_5_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define PHY_ADDR_5_FCONTX_OFFSET	0x010
#define PHY_ADDR_5_FCONTX_SHIFT		7
#define PHY_ADDR_5_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define PHY_ADDR_5_FCONRX_OFFSET	0x010
#define PHY_ADDR_5_FCONRX_SHIFT		5
#define PHY_ADDR_5_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define PHY_ADDR_5_ADDR_OFFSET	0x010
#define PHY_ADDR_5_ADDR_SHIFT		0
#define PHY_ADDR_5_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 4' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define PHY_ADDR_4_LNKST_OFFSET	0x011
#define PHY_ADDR_4_LNKST_SHIFT	13
#define PHY_ADDR_4_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PHY_ADDR_4_SPEED_OFFSET	0x011
#define PHY_ADDR_4_SPEED_SHIFT	11
#define PHY_ADDR_4_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define PHY_ADDR_4_FDUP_OFFSET	0x011
#define PHY_ADDR_4_FDUP_SHIFT		9
#define PHY_ADDR_4_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define PHY_ADDR_4_FCONTX_OFFSET	0x011
#define PHY_ADDR_4_FCONTX_SHIFT		7
#define PHY_ADDR_4_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define PHY_ADDR_4_FCONRX_OFFSET	0x011
#define PHY_ADDR_4_FCONRX_SHIFT		5
#define PHY_ADDR_4_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define PHY_ADDR_4_ADDR_OFFSET	0x011
#define PHY_ADDR_4_ADDR_SHIFT		0
#define PHY_ADDR_4_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 3' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define PHY_ADDR_3_LNKST_OFFSET	0x012
#define PHY_ADDR_3_LNKST_SHIFT	13
#define PHY_ADDR_3_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PHY_ADDR_3_SPEED_OFFSET	0x012
#define PHY_ADDR_3_SPEED_SHIFT	11
#define PHY_ADDR_3_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define PHY_ADDR_3_FDUP_OFFSET	0x012
#define PHY_ADDR_3_FDUP_SHIFT		9
#define PHY_ADDR_3_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define PHY_ADDR_3_FCONTX_OFFSET	0x012
#define PHY_ADDR_3_FCONTX_SHIFT		7
#define PHY_ADDR_3_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define PHY_ADDR_3_FCONRX_OFFSET	0x012
#define PHY_ADDR_3_FCONRX_SHIFT		5
#define PHY_ADDR_3_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define PHY_ADDR_3_ADDR_OFFSET	0x012
#define PHY_ADDR_3_ADDR_SHIFT		0
#define PHY_ADDR_3_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 2' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define PHY_ADDR_2_LNKST_OFFSET	0x013
#define PHY_ADDR_2_LNKST_SHIFT	13
#define PHY_ADDR_2_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PHY_ADDR_2_SPEED_OFFSET	0x013
#define PHY_ADDR_2_SPEED_SHIFT	11
#define PHY_ADDR_2_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define PHY_ADDR_2_FDUP_OFFSET	0x013
#define PHY_ADDR_2_FDUP_SHIFT		9
#define PHY_ADDR_2_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define PHY_ADDR_2_FCONTX_OFFSET	0x013
#define PHY_ADDR_2_FCONTX_SHIFT		7
#define PHY_ADDR_2_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define PHY_ADDR_2_FCONRX_OFFSET	0x013
#define PHY_ADDR_2_FCONRX_SHIFT		5
#define PHY_ADDR_2_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define PHY_ADDR_2_ADDR_OFFSET	0x013
#define PHY_ADDR_2_ADDR_SHIFT		0
#define PHY_ADDR_2_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 1' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define PHY_ADDR_1_LNKST_OFFSET	0x014
#define PHY_ADDR_1_LNKST_SHIFT	13
#define PHY_ADDR_1_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PHY_ADDR_1_SPEED_OFFSET	0x014
#define PHY_ADDR_1_SPEED_SHIFT	11
#define PHY_ADDR_1_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define PHY_ADDR_1_FDUP_OFFSET	0x014
#define PHY_ADDR_1_FDUP_SHIFT		9
#define PHY_ADDR_1_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define PHY_ADDR_1_FCONTX_OFFSET	0x014
#define PHY_ADDR_1_FCONTX_SHIFT		7
#define PHY_ADDR_1_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define PHY_ADDR_1_FCONRX_OFFSET	0x014
#define PHY_ADDR_1_FCONRX_SHIFT		5
#define PHY_ADDR_1_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define PHY_ADDR_1_ADDR_OFFSET	0x014
#define PHY_ADDR_1_ADDR_SHIFT		0
#define PHY_ADDR_1_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY Address RegisterPORT 0' */
/* Bit: 'LNKST' */
/* Description: 'Link Status Control' */
#define PHY_ADDR_0_LNKST_OFFSET	0x015
#define PHY_ADDR_0_LNKST_SHIFT	13
#define PHY_ADDR_0_LNKST_SIZE		2
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PHY_ADDR_0_SPEED_OFFSET	0x015
#define PHY_ADDR_0_SPEED_SHIFT	11
#define PHY_ADDR_0_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Control' */
#define PHY_ADDR_0_FDUP_OFFSET	0x015
#define PHY_ADDR_0_FDUP_SHIFT		9
#define PHY_ADDR_0_FDUP_SIZE		2
/* Bit: 'FCONTX' */
/* Description: 'Flow Control Mode TX' */
#define PHY_ADDR_0_FCONTX_OFFSET	0x015
#define PHY_ADDR_0_FCONTX_SHIFT		7
#define PHY_ADDR_0_FCONTX_SIZE		2
/* Bit: 'FCONRX' */
/* Description: 'Flow Control Mode RX' */
#define PHY_ADDR_0_FCONRX_OFFSET	0x015
#define PHY_ADDR_0_FCONRX_SHIFT		5
#define PHY_ADDR_0_FCONRX_SIZE		2
/* Bit: 'ADDR' */
/* Description: 'PHY Address' */
#define PHY_ADDR_0_ADDR_OFFSET	0x015
#define PHY_ADDR_0_ADDR_SHIFT		0
#define PHY_ADDR_0_ADDR_SIZE		5
/* ----------------------------------------------- */
/* Register: 'PHY MDIO PollingStatus per PORT' */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'PHY supports MAC turning of TX clk' */
#define MDIO_STAT_0_CLK_STOP_CAPABLE_OFFSET	0x016
#define MDIO_STAT_0_CLK_STOP_CAPABLE_SHIFT	8
#define MDIO_STAT_0_CLK_STOP_CAPABLE_SIZE		1
/* Bit: 'EEE_CAPABLE' */
/* Description: 'PHY and link partner support EEE for current speed' */
#define MDIO_STAT_0_EEE_CAPABLE_OFFSET	0x016
#define MDIO_STAT_0_EEE_CAPABLE_SHIFT		7
#define MDIO_STAT_0_EEE_CAPABLE_SIZE		1
/* Bit: 'PACT' */
/* Description: 'PHY Active Status' */
#define MDIO_STAT_0_PACT_OFFSET	0x016
#define MDIO_STAT_0_PACT_SHIFT	6
#define MDIO_STAT_0_PACT_SIZE		1
/* Bit: 'LSTAT' */
/* Description: 'Link Status' */
#define MDIO_STAT_0_LSTAT_OFFSET	0x016
#define MDIO_STAT_0_LSTAT_SHIFT		5
#define MDIO_STAT_0_LSTAT_SIZE		1
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define MDIO_STAT_0_SPEED_OFFSET	0x016
#define MDIO_STAT_0_SPEED_SHIFT		3
#define MDIO_STAT_0_SPEED_SIZE		2
/* Bit: 'FDUP' */
/* Description: 'Full Duplex Status' */
#define MDIO_STAT_0_FDUP_OFFSET	0x016
#define MDIO_STAT_0_FDUP_SHIFT	2
#define MDIO_STAT_0_FDUP_SIZE		1
/* Bit: 'RXPAUEN' */
/* Description: 'Receive Pause Enable Status' */
#define MDIO_STAT_0_RXPAUEN_OFFSET	0x016
#define MDIO_STAT_0_RXPAUEN_SHIFT		1
#define MDIO_STAT_0_RXPAUEN_SIZE		1
/* Bit: 'TXPAUEN' */
/* Description: 'Transmit Pause Enable Status' */
#define MDIO_STAT_0_TXPAUEN_OFFSET	0x016
#define MDIO_STAT_0_TXPAUEN_SHIFT		0
#define MDIO_STAT_0_TXPAUEN_SIZE		1
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 0*/
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define ANEG_EEE_0_CLK_STOP_CAPABLE_OFFSET	0x01C
#define ANEG_EEE_0_CLK_STOP_CAPABLE_SHIFT		2
#define ANEG_EEE_0_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define ANEG_EEE_0_EEE_CAPABLE_OFFSET	0x01C
#define ANEG_EEE_0_EEE_CAPABLE_SHIFT	0
#define ANEG_EEE_0_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 0 */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define ANEG_EEE_1_CLK_STOP_CAPABLE_OFFSET	0x01D
#define ANEG_EEE_1_CLK_STOP_CAPABLE_SHIFT		2
#define ANEG_EEE_1_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define ANEG_EEE_1_EEE_CAPABLE_OFFSET	0x01D
#define ANEG_EEE_1_EEE_CAPABLE_SHIFT	0
#define ANEG_EEE_1_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 2 */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define ANEG_EEE_2_CLK_STOP_CAPABLE_OFFSET	0x01E
#define ANEG_EEE_2_CLK_STOP_CAPABLE_SHIFT		2
#define ANEG_EEE_2_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define ANEG_EEE_2_EEE_CAPABLE_OFFSET	0x01E
#define ANEG_EEE_2_EEE_CAPABLE_SHIFT	0
#define ANEG_EEE_2_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 3 */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define ANEG_EEE_3_CLK_STOP_CAPABLE_OFFSET	0x01F
#define ANEG_EEE_3_CLK_STOP_CAPABLE_SHIFT		2
#define ANEG_EEE_3_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define ANEG_EEE_3_EEE_CAPABLE_OFFSET	0x01F
#define ANEG_EEE_3_EEE_CAPABLE_SHIFT	0
#define ANEG_EEE_3_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 4  */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define ANEG_EEE_4_CLK_STOP_CAPABLE_OFFSET	0x020
#define ANEG_EEE_4_CLK_STOP_CAPABLE_SHIFT		2
#define ANEG_EEE_4_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define ANEG_EEE_4_EEE_CAPABLE_OFFSET	0x020
#define ANEG_EEE_4_EEE_CAPABLE_SHIFT	0
#define ANEG_EEE_4_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'EEE auto negotiationoverides' for PORT 5 */
/* Bit: 'CLK_STOP_CAPABLE' */
/* Description: 'clk stop capable' */
#define ANEG_EEE_5_CLK_STOP_CAPABLE_OFFSET	0x021
#define ANEG_EEE_5_CLK_STOP_CAPABLE_SHIFT		2
#define ANEG_EEE_5_CLK_STOP_CAPABLE_SIZE		2
/* Bit: 'EEE_CAPABLE' */
/* Description: 'EEE capable' */
#define ANEG_EEE_5_EEE_CAPABLE_OFFSET	0x021
#define ANEG_EEE_5_EEE_CAPABLE_SHIFT	0
#define ANEG_EEE_5_EEE_CAPABLE_SIZE		2
/* ----------------------------------------------- */
/* Register: 'xMII Port 0 ConfigurationRegister' */
/* Bit: 'RES' */
/* Description: 'Hardware Reset' */
#define MII_CFG_0_RES_OFFSET	0x036
#define MII_CFG_0_RES_SHIFT		15
#define MII_CFG_0_RES_SIZE		1
/* Bit: 'EN' */
/* Description: 'xMII Interface Enable' */
#define MII_CFG_0_EN_OFFSET		0x036
#define MII_CFG_0_EN_SHIFT		14
#define MII_CFG_0_EN_SIZE			1
/* Bit: 'ISOL' */
/* Description: 'ISOLATE xMII Interface' */
#define MII_CFG_0_ISOL_OFFSET		0x036
#define MII_CFG_0_ISOL_SHIFT		13
#define MII_CFG_0_ISOL_SIZE			1
/* Bit: 'LDCLKDIS' */
/* Description: 'Link Down Clock Disable' */
#define MII_CFG_0_LDCLKDIS_OFFSET	0x036
#define MII_CFG_0_LDCLKDIS_SHIFT	12
#define MII_CFG_0_LDCLKDIS_SIZE		1
/* Bit: 'CRS' */
/* Description: 'CRS Sensitivity Configuration' */
#define MII_CFG_0_CRS_OFFSET	0x036
#define MII_CFG_0_CRS_SHIFT		9
#define MII_CFG_0_CRS_SIZE		2
/* Bit: 'RGMII_IBS' */
/* Description: 'RGMII In Band Status' */
#define MII_CFG_0_RGMII_IBS_OFFSET	0x036
#define MII_CFG_0_RGMII_IBS_SHIFT		8
#define MII_CFG_0_RGMII_IBS_SIZE		1
/* Bit: 'RMII' */
/* Description: 'RMII Reference Clock Direction of the Port' */
#define MII_CFG_0_RMII_OFFSET	0x036
#define MII_CFG_0_RMII_SHIFT	7
#define MII_CFG_0_RMII_SIZE		1
/* Bit: 'MIIRATE' */
/* Description: 'xMII Port Interface Clock Rate' */
#define MII_CFG_0_MIIRATE_OFFSET	0x036
#define MII_CFG_0_MIIRATE_SHIFT		4
#define MII_CFG_0_MIIRATE_SIZE		3
/* Bit: 'MIIMODE' */
/* Description: 'xMII Interface Mode' */
#define MII_CFG_0_MIIMODE_OFFSET	0x036
#define MII_CFG_0_MIIMODE_SHIFT		0
#define MII_CFG_0_MIIMODE_SIZE		4
/* ----------------------------------------------- */
/* Register: 'Configuration of ClockDelay for Port 0' */
/* Bit: 'RXLOCK' */
/* Description: 'Lock Status MDL of Receive PCDU' */
#define PCDU_0_RXLOCK_OFFSET	0x037
#define PCDU_0_RXLOCK_SHIFT		15
#define PCDU_0_RXLOCK_SIZE		1
/* Bit: 'TXLOCK' */
/* Description: 'Lock Status of MDL of Transmit PCDU' */
#define PCDU_0_TXLOCK_OFFSET	0x037
#define PCDU_0_TXLOCK_SHIFT		14
#define PCDU_0_TXLOCK_SIZE		1
/* Bit: 'RXDLY' */
/* Description: 'Configure Receive Clock Delay' */
#define PCDU_0_RXDLY_OFFSET		0x037
#define PCDU_0_RXDLY_SHIFT		7
#define PCDU_0_RXDLY_SIZE			3
/* Bit: 'TXDLY' */
/* Description: 'Configure Transmit PCDU' */
#define PCDU_0_TXDLY_OFFSET		0x037
#define PCDU_0_TXDLY_SHIFT		0
#define PCDU_0_TXDLY_SIZE			3
/* ----------------------------------------------- */
/* Register: 'xMII Port 1 ConfigurationRegister' */
/* Bit: 'RES' */
/* Description: 'Hardware Reset' */
#define MII_CFG_1_RES_OFFSET	0x038
#define MII_CFG_1_RES_SHIFT		15
#define MII_CFG_1_RES_SIZE		1
/* Bit: 'EN' */
/* Description: 'xMII Interface Enable' */
#define MII_CFG_1_EN_OFFSET		0x038
#define MII_CFG_1_EN_SHIFT		14
#define MII_CFG_1_EN_SIZE			1
/* Bit: 'ISOL' */
/* Description: 'ISOLATE xMII Interface' */
#define MII_CFG_1_ISOL_OFFSET	0x038
#define MII_CFG_1_ISOL_SHIFT	13
#define MII_CFG_1_ISOL_SIZE		1
/* Bit: 'LDCLKDIS' */
/* Description: 'Link Down Clock Disable' */
#define MII_CFG_1_LDCLKDIS_OFFSET	0x038
#define MII_CFG_1_LDCLKDIS_SHIFT	12
#define MII_CFG_1_LDCLKDIS_SIZE		1
/* Bit: 'CRS' */
/* Description: 'CRS Sensitivity Configuration' */
#define MII_CFG_1_CRS_OFFSET	0x038
#define MII_CFG_1_CRS_SHIFT		9
#define MII_CFG_1_CRS_SIZE		2
/* Bit: 'RGMII_IBS' */
/* Description: 'RGMII In Band Status' */
#define MII_CFG_1_RGMII_IBS_OFFSET	0x038
#define MII_CFG_1_RGMII_IBS_SHIFT		8
#define MII_CFG_1_RGMII_IBS_SIZE		1
/* Bit: 'RMII' */
/* Description: 'RMII Reference Clock Direction of the Port' */
#define MII_CFG_1_RMII_OFFSET	0x038
#define MII_CFG_1_RMII_SHIFT	7
#define MII_CFG_1_RMII_SIZE		1
/* Bit: 'MIIRATE' */
/* Description: 'xMII Port Interface Clock Rate' */
#define MII_CFG_1_MIIRATE_OFFSET	0x038
#define MII_CFG_1_MIIRATE_SHIFT		4
#define MII_CFG_1_MIIRATE_SIZE		3
/* Bit: 'MIIMODE' */
/* Description: 'xMII Interface Mode' */
#define MII_CFG_1_MIIMODE_OFFSET	0x038
#define MII_CFG_1_MIIMODE_SHIFT		0
#define MII_CFG_1_MIIMODE_SIZE		4
/* ----------------------------------------------- */
/* Register: 'Configuration of ClockDelay for Port 1' */
/* Bit: 'RXLOCK' */
/* Description: 'Lock Status MDL of Receive PCDU' */
#define PCDU_1_RXLOCK_OFFSET	0x039
#define PCDU_1_RXLOCK_SHIFT		15
#define PCDU_1_RXLOCK_SIZE		1
/* Bit: 'TXLOCK' */
/* Description: 'Lock Status of MDL of Transmit PCDU' */
#define PCDU_1_TXLOCK_OFFSET	0x039
#define PCDU_1_TXLOCK_SHIFT		14
#define PCDU_1_TXLOCK_SIZE		1
/* Bit: 'RXDLY' */
/* Description: 'Configure Receive Clock Delay' */
#define PCDU_1_RXDLY_OFFSET		0x039
#define PCDU_1_RXDLY_SHIFT		7
#define PCDU_1_RXDLY_SIZE			3
/* Bit: 'TXDLY' */
/* Description: 'Configure Transmit PCDU' */
#define PCDU_1_TXDLY_OFFSET		0x039
#define PCDU_1_TXDLY_SHIFT		0
#define PCDU_1_TXDLY_SIZE			3
/* ----------------------------------------------- */
/* Register: 'xMII Port 5 ConfigurationRegister' */
/* Bit: 'RES' */
/* Description: 'Hardware Reset' */
#define MII_CFG_5_RES_OFFSET	0x040
#define MII_CFG_5_RES_SHIFT		15
#define MII_CFG_5_RES_SIZE		1
/* Bit: 'EN' */
/* Description: 'xMII Interface Enable' */
#define MII_CFG_5_EN_OFFSET		0x040
#define MII_CFG_5_EN_SHIFT		14
#define MII_CFG_5_EN_SIZE			1
/* Bit: 'ISOL' */
/* Description: 'ISOLATE xMII Interface' */
#define MII_CFG_5_ISOL_OFFSET	0x040
#define MII_CFG_5_ISOL_SHIFT	13
#define MII_CFG_5_ISOL_SIZE		1
/* Bit: 'LDCLKDIS' */
/* Description: 'Link Down Clock Disable' */
#define MII_CFG_5_LDCLKDIS_OFFSET	0x040
#define MII_CFG_5_LDCLKDIS_SHIFT	12
#define MII_CFG_5_LDCLKDIS_SIZE		1
/* Bit: 'CRS' */
/* Description: 'CRS Sensitivity Configuration' */
#define MII_CFG_5_CRS_OFFSET	0x040
#define MII_CFG_5_CRS_SHIFT		9
#define MII_CFG_5_CRS_SIZE		2
/* Bit: 'RGMII_IBS' */
/* Description: 'RGMII In Band Status' */
#define MII_CFG_5_RGMII_IBS_OFFSET	0x040
#define MII_CFG_5_RGMII_IBS_SHIFT		8
#define MII_CFG_5_RGMII_IBS_SIZE		1
/* Bit: 'MIIRATE' */
/* Description: 'xMII Port Interface Clock Rate' */
#define MII_CFG_5_MIIRATE_OFFSET	0x040
#define MII_CFG_5_MIIRATE_SHIFT		4
#define MII_CFG_5_MIIRATE_SIZE		3
/* Bit: 'MIIMODE' */
/* Description: 'xMII Interface Mode' */
#define MII_CFG_5_MIIMODE_OFFSET	0x040
#define MII_CFG_5_MIIMODE_SHIFT		0
#define MII_CFG_5_MIIMODE_SIZE		4
/* ----------------------------------------------- */
/* Register: 'Configuration of ClockDelay for External Port 5' */
/* Bit: 'RXLOCK' */
/* Description: 'Lock Status MDL of Receive PCDU' */
#define PCDU_5_RXLOCK_OFFSET	0x041
#define PCDU_5_RXLOCK_SHIFT		15
#define PCDU_5_RXLOCK_SIZE		1
/* Bit: 'TXLOCK' */
/* Description: 'Lock Status of MDL of Transmit PCDU' */
#define PCDU_5_TXLOCK_OFFSET	0x041
#define PCDU_5_TXLOCK_SHIFT		14
#define PCDU_5_TXLOCK_SIZE		1
/* Bit: 'RXDLY' */
/* Description: 'Configure Receive Clock Delay' */
#define PCDU_5_RXDLY_OFFSET		0x041
#define PCDU_5_RXDLY_SHIFT		7
#define PCDU_5_RXDLY_SIZE			3
/* Bit: 'TXDLY' */
/* Description: 'Configure Transmit PCDU' */
#define PCDU_5_TXDLY_OFFSET		0x041
#define PCDU_5_TXDLY_SHIFT		0
#define PCDU_5_TXDLY_SIZE			3
/* ----------------------------------------------- */
/* Register: 'Receive Buffer ControlRegister for Port 0' */
/* Bit: 'RBUF_UFL' */
/* Description: 'Receive Buffer Underflow Indicator' */
#define RXB_CTL_0_RBUF_UFL_OFFSET	0x056
#define RXB_CTL_0_RBUF_UFL_SHIFT	15
#define RXB_CTL_0_RBUF_UFL_SIZE		1
/* Bit: 'RBUF_OFL' */
/* Description: 'Receive Buffer Overflow Indicator' */
#define RXB_CTL_0_RBUF_OFL_OFFSET	0x056
#define RXB_CTL_0_RBUF_OFL_SHIFT	14
#define RXB_CTL_0_RBUF_OFL_SIZE		1
/* Bit: 'RBUF_DLY_WP' */
/* Description: 'Delay' */
#define RXB_CTL_0_RBUF_DLY_WP_OFFSET	0x056
#define RXB_CTL_0_RBUF_DLY_WP_SHIFT		0
#define RXB_CTL_0_RBUF_DLY_WP_SIZE		3
/* ----------------------------------------------- */
/* Register: 'Receive Buffer ControlRegister External Port 1' */
/* Bit: 'RBUF_UFL' */
/* Description: 'Receive Buffer Underflow Indicator' */
#define RXB_CTL_1_RBUF_UFL_OFFSET	0x057
#define RXB_CTL_1_RBUF_UFL_SHIFT	15
#define RXB_CTL_1_RBUF_UFL_SIZE		1
/* Bit: 'RBUF_OFL' */
/* Description: 'Receive Buffer Overflow Indicator' */
#define RXB_CTL_1_RBUF_OFL_OFFSET	0x057
#define RXB_CTL_1_RBUF_OFL_SHIFT	14
#define RXB_CTL_1_RBUF_OFL_SIZE		1
/* Bit: 'RBUF_DLY_WP' */
/* Description: 'Delay' */
#define RXB_CTL_1_RBUF_DLY_WP_OFFSET	0x057
#define RXB_CTL_1_RBUF_DLY_WP_SHIFT		0
#define RXB_CTL_1_RBUF_DLY_WP_SIZE		3
/* ----------------------------------------------- */
/* Register: 'Receive Buffer ControlRegister External Port 5' */
/* Bit: 'RBUF_UFL' */
/* Description: 'Receive Buffer Underflow Indicator' */
#define RXB_CTL_5_RBUF_UFL_OFFSET	0x05B
#define RXB_CTL_5_RBUF_UFL_SHIFT	15
#define RXB_CTL_5_RBUF_UFL_SIZE		1
/* Bit: 'RBUF_OFL' */
/* Description: 'Receive Buffer Overflow Indicator' */
#define RXB_CTL_5_RBUF_OFL_OFFSET	0x05B
#define RXB_CTL_5_RBUF_OFL_SHIFT	14
#define RXB_CTL_5_RBUF_OFL_SIZE		1
/* Bit: 'RBUF_DLY_WP' */
/* Description: 'Delay' */
#define RXB_CTL_5_RBUF_DLY_WP_OFFSET	0x05B
#define RXB_CTL_5_RBUF_DLY_WP_SHIFT		0
#define RXB_CTL_5_RBUF_DLY_WP_SIZE		3
/* ----------------------------------------------- */
/* Register: 'Debug Control Register' */
/* Bit: 'DBG_EN' */
/* Description: 'Debug enable' */
#define DBG_CTL_DBG_EN_OFFSET	0x081
#define DBG_CTL_DBG_EN_SHIFT	14
#define DBG_CTL_DBG_EN_SIZE		1
/* Bit: 'DBG_SEL' */
/* Description: 'Debug select' */
#define DBG_CTL_DBG_SEL_OFFSET	0x081
#define DBG_CTL_DBG_SEL_SHIFT		0
#define DBG_CTL_DBG_SEL_SIZE		1
/* ----------------------------------------------- */
/* Register: 'PMAC Header ControlRegister' */
/* Bit: 'FC' */
/* Description: 'Enable Flow Control' */
#define PMAC_HD_CTL_FC_OFFSET	0x082
#define PMAC_HD_CTL_FC_SHIFT	10
#define PMAC_HD_CTL_FC_SIZE		1
/* Bit: 'CCRC' */
/* Description: 'Check CRC' */
#define PMAC_HD_CTL_CCRC_OFFSET	0x082
#define PMAC_HD_CTL_CCRC_SHIFT	9
#define PMAC_HD_CTL_CCRC_SIZE		1
/* Bit: 'RST' */
/* Description: 'Remove Special Tag' */
#define PMAC_HD_CTL_RST_OFFSET	0x082
#define PMAC_HD_CTL_RST_SHIFT		8
#define PMAC_HD_CTL_RST_SIZE		1
/* Bit: 'AST' */
/* Description: 'Add Special Tag' */
#define PMAC_HD_CTL_AST_OFFSET	0x082
#define PMAC_HD_CTL_AST_SHIFT		7
#define PMAC_HD_CTL_AST_SIZE		1
/* Bit: 'RXSH' */
/* Description: 'Status Header' */
#define PMAC_HD_CTL_RXSH_OFFSET	0x082
#define PMAC_HD_CTL_RXSH_SHIFT	6
#define PMAC_HD_CTL_RXSH_SIZE		1
/* Bit: 'RL2' */
/* Description: 'Remove Layer-2 Header' */
#define PMAC_HD_CTL_RL2_OFFSET	0x082
#define PMAC_HD_CTL_RL2_SHIFT		5
#define PMAC_HD_CTL_RL2_SIZE		1
/* Bit: 'RC' */
/* Description: 'Remove CRC' */
#define PMAC_HD_CTL_RC_OFFSET	0x082
#define PMAC_HD_CTL_RC_SHIFT	4
#define PMAC_HD_CTL_RC_SIZE		1
/* Bit: 'AS' */
/* Description: 'Add Status Header' */
#define PMAC_HD_CTL_AS_OFFSET	0x082
#define PMAC_HD_CTL_AS_SHIFT	3
#define PMAC_HD_CTL_AS_SIZE		1
/* Bit: 'AC' */
/* Description: 'Add CRC' */
#define PMAC_HD_CTL_AC_OFFSET	0x082
#define PMAC_HD_CTL_AC_SHIFT	2
#define PMAC_HD_CTL_AC_SIZE		1
/* Bit: 'TAG' */
/* Description: 'Add TAG' */
#define PMAC_HD_CTL_TAG_OFFSET	0x082
#define PMAC_HD_CTL_TAG_SHIFT		1
#define PMAC_HD_CTL_TAG_SIZE		1
/* Bit: 'ADD' */
/* Description: 'ADD Header' */
#define PMAC_HD_CTL_ADD_OFFSET	0x082
#define PMAC_HD_CTL_ADD_SHIFT		0
#define PMAC_HD_CTL_ADD_SIZE		1
/* ----------------------------------------------- */
/* Register: 'PMAC Type/Length Register' */
/* Bit: 'TYPE_LEN' */
/* Description: 'TYPE or Lenght Value' */
#define PMAC_TL_TYPE_LEN_OFFSET	0x083
#define PMAC_TL_TYPE_LEN_SHIFT	0
#define PMAC_TL_TYPE_LEN_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Source Address Register1' */
/* Bit: 'SA_47_32' */
/* Description: 'Source Address 47..32' */
#define PMAC_SA1_SA_47_32_OFFSET	0x084
#define PMAC_SA1_SA_47_32_SHIFT		0
#define PMAC_SA1_SA_47_32_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Source Address Register2' */
/* Bit: 'SA_31_16' */
/* Description: 'Source Address 31..16' */
#define PMAC_SA2_SA_31_16_OFFSET	0x085
#define PMAC_SA2_SA_31_16_SHIFT		0
#define PMAC_SA2_SA_31_16_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Source Address Register3' */
/* Bit: 'SA_15_0' */
/* Description: 'Source Address 15..0' */
#define PMAC_SA3_SA_15_0_OFFSET	0x086
#define PMAC_SA3_SA_15_0_SHIFT	0
#define PMAC_SA3_SA_15_0_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Destination AddressRegister 1' */
/* Bit: 'SA_47_32' */
/* Description: 'Destination Address 47..32' */
#define PMAC_DA1_SA_47_32_OFFSET	0x087
#define PMAC_DA1_SA_47_32_SHIFT		0
#define PMAC_DA1_SA_47_32_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Destination AddressRegister 2' */
/* Bit: 'DA_31_16' */
/* Description: 'Destination Address 31..16' */
#define PMAC_DA2_DA_31_16_OFFSET	0x088
#define PMAC_DA2_DA_31_16_SHIFT		0
#define PMAC_DA2_DA_31_16_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Destination AddressRegister 3' */
/* Bit: 'DA_15_0' */
/* Description: 'Destination Address 15..0' */
#define PMAC_DA3_DA_15_0_OFFSET	0x089
#define PMAC_DA3_DA_15_0_SHIFT	0
#define PMAC_DA3_DA_15_0_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC VLAN Register' */
/* Bit: 'PRI' */
/* Description: 'VLAN Priority' */
#define PMAC_VLAN_PRI_OFFSET	0x08A
#define PMAC_VLAN_PRI_SHIFT		13
#define PMAC_VLAN_PRI_SIZE		3
/* Bit: 'CFI' */
/* Description: 'Canonical Format Identifier' */
#define PMAC_VLAN_CFI_OFFSET	0x08A
#define PMAC_VLAN_CFI_SHIFT		12
#define PMAC_VLAN_CFI_SIZE		1
/* Bit: 'VLAN_ID' */
/* Description: 'VLAN ID' */
#define PMAC_VLAN_VLAN_ID_OFFSET	0x08A
#define PMAC_VLAN_VLAN_ID_SHIFT		0
#define PMAC_VLAN_VLAN_ID_SIZE		12
/* ----------------------------------------------- */
/* Register: 'PMAC Inter Packet Gapin RX Direction' */
/* Bit: 'REQ_DS_THRES' */
/* Description: 'Request Deassertion Threshold' */
#define PMAC_RX_IPG_REQ_DS_THRES_OFFSET	0x08B
#define PMAC_RX_IPG_REQ_DS_THRES_SHIFT	8
#define PMAC_RX_IPG_REQ_DS_THRES_SIZE		1
/* Bit: 'REQ_AS_THRES' */
/* Description: 'Request Assertion Threshold' */
#define PMAC_RX_IPG_REQ_AS_THRES_OFFSET	0x08B
#define PMAC_RX_IPG_REQ_AS_THRES_SHIFT	4
#define PMAC_RX_IPG_REQ_AS_THRES_SIZE		4
/* Bit: 'IPG_CNT' */
/* Description: 'IPG Counter' */
#define PMAC_RX_IPG_IPG_CNT_OFFSET	0x08B
#define PMAC_RX_IPG_IPG_CNT_SHIFT		0
#define PMAC_RX_IPG_IPG_CNT_SIZE		4
/* ----------------------------------------------- */
/* Register: 'PMAC Special Tag Ethertype' */
/* Bit: 'ST_ETYPE' */
/* Description: 'Special Tag Ethertype' */
#define PMAC_ST_ETYPE_ST_ETYPE_OFFSET	0x08C
#define PMAC_ST_ETYPE_ST_ETYPE_SHIFT	0
#define PMAC_ST_ETYPE_ST_ETYPE_SIZE		16
/* ----------------------------------------------- */
/* Register: 'PMAC Ethernet WAN Group' */
/* Bit: 'EWAN' */
/* Description: 'Ethernet WAN Group' */
#define PMAC_EWAN_EWAN_OFFSET	0x08D
#define PMAC_EWAN_EWAN_SHIFT	0
#define PMAC_EWAN_EWAN_SIZE		6
/* ----------------------------------------------- */
/* Register: 'PMAC Control Register' */
/* Bit: 'SPEED' */
/* Description: 'Speed Control' */
#define PMAC_CTL_SPEED_OFFSET	0x08E
#define PMAC_CTL_SPEED_SHIFT	1
#define PMAC_CTL_SPEED_SIZE		1
/* Bit: 'EN' */
/* Description: 'PMAC Enable' */
#define PMAC_CTL_EN_OFFSET	0x08E
#define PMAC_CTL_EN_SHIFT		0
#define PMAC_CTL_EN_SIZE		1
/* ----------------------------------------------- */
#endif /*  _LTQ_ETHSW_SOC_TOP_H_ */
