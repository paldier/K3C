#ifndef __VR9_BONDING_ADDR_DEF_INC
#define __VR9_BONDING_ADDR_DEF_INC

//=============================================================================
// Bonding Related Definitions
//=============================================================================

//--------------
//PDMA Registers
//--------------
#define __PDMA_BAR0                      0x7A0D
#define __PDMA_BAR1                      0x7A0E
	
//--------------------
//Constant Definitions
//--------------------
#define PKT_REASSEMBLING	0
#define PKT_ASSEMBLY_DONE	1
#define PKT_FLUSHING        2

//Number of Bonding Groups in Upstream
#define __NUM_US_BG                         4
//Number of Gamma Interfaces in Upstream
#define __NUM_US_GIFS                       8
//Number of Bonding Groups in Downstream
#define __NUM_DS_BG                         4
//Number of Gamma Interfaces in Downstream
#define __NUM_DS_GIFS                       8

#define __MAX_PKT_SIZE          1536
#define __MAX_FRAG_NUM_PER_PKT  30

//---------------------
//General Configuration
//---------------------
//Addresses for Data Structures defined in Section 5.3.2 in Spec
// TX fragment avaiable mailbox pdma address for bonding board 0 and 1
// if address <= 0xffff, it is consider a PP32 address
// if address >  0xffff, it is a PDMA address (cross PCI)
#define __TFA_MAILBOX_PDMA_ADDR0                0x2004
#define __TFA_MAILBOX_PDMA_ADDR1                0x2005

#define __BOND_CONF                             0x2008
#define __US_BG_QMAP                            0x2009
#define __US_BG_GMAP                            0x200A
#define __DS_BG_GMAP                            0x200B
#define __CURR_TIME_STAMP                       0x200C
#define __E1_DES_PDMA_BAR                       0x200D
#define __B1_DES_PDMA_BAR                       0x200E
#define __DATA_PTR_PDMA_PREFIX_CFG              0x200F

#define __US_CDMA_RX_DES_PDMA_BAR               0x2020
#define __US_CDMA_RX_DES_BASE                   0x2021

#define __US_E1_FRAG_DESBA_CFG                  0x202A
#define __DS_E1_FRAG_DESBA_CFG                  0x202B

//Addresses for Data Structures defined in Section 5.3.3 in Spec
//Already defined in E1/D5

//Addresses for Data Structures defined in Section 5.3.4 in Spec
//Used only by E1 FW
//Commented since they are defined in E1
//#define __US_E1_FRAG_DESBA_CFG                  0x202A
//#define __DS_E1_FRAG_DESBA_CFG                  0x202B

//---------------------
//Descriptors Addresses
//---------------------
//E1 TX Fragment Descriptor Base Address (Non-Bonding Part)
//Section 5.2.4 (Non-Bonding Part)
#define __US_E1_FRAG_DESBA                      0x3800
#define __US_E1_FRAG_DES_NUM                    128
#define __US_E1_FRAG_DES_SIZE                   2
#define __US_E1_FRAG_DES_TOTAL_SIZE             256
//__US_E1_FRAG_DES_NUM_PER_GIF = __US_E1_FRAG_DES_NUM/__NUM_US_GIFS
#define __US_E1_FRAG_DES_NUM_PER_GIF            16
//__US_E1_FRAG_DES_TOTAL_SIZE_PER_GIF = __US_E1_FRAG_DES_TOTAL_SIZE/__NUM_US_GIFS
#define __US_E1_FRAG_DES_TOTAL_SIZE_PER_GIF     32

//E1 TX Fragment Descriptor Base Address (Bonding Part)
//Section 5.2.4 (Bonding Part)
#define __US_E1_FRAG_BP_DESBA                   0x3F60
#define __US_E1_FRAG_BP_DES_NUM                 128
#define __US_E1_FRAG_BP_DES_SIZE                1
#define __US_E1_FRAG_BP_DES_TOTAL_SIZE          128
//__US_E1_FRAG_BP_DES_NUM_PER_GIF = __US_E1_FRAG_BP_DES_NUM/__NUM_US_GIFS
#define __US_E1_FRAG_BP_DES_NUM_PER_GIF         16
//__US_E1_FRAG_BP_DES_TOTAL_SIZE_PER_GIF = __US_E1_FRAG_BP_DES_TOTAL_SIZE/__NUM_US_GIFS
#define __US_E1_FRAG_BP_DES_TOTAL_SIZE_PER_GIF  16

//System Master E1 RX Fragment Descriptor Base Address
#define __DS_SM_E1_FRAG_DESBA                   0x3D00
#define __DS_SM_E1_FRAG_DES_NUM                 32
#define __DS_SM_E1_FRAG_DES_SIZE                2
#define __DS_SM_E1_FRAG_DES_TOTAL_SIZE          64

//Bonding Master E1 RX Fragment Descriptor Base Address
#define __DS_BM_E1_FRAG_DESBA                   0x3D40
#define __DS_BM_E1_FRAG_DES_NUM                 32
#define __DS_SM_E1_FRAG_DES_SIZE                2
#define __DS_SM_E1_FRAG_DES_TOTAL_SIZE          64

//DS Bonding GIF Link List Descriptor Base Address
#define __DS_BOND_GIF_LL_DESBA                  0x4800
#define __DS_BOND_GIF_LL_DES_NUM                256

//DMA TX Channel 1 Descriptor List
#define __DMA_TX_CH1_DESBA						0x2600
#define __DMA_TX_CH1_DES_NUM					64
#define __DMA_TX_CH1_DES_TOTAL_SIZE				128

//----------------
//FW Context & MIB
//----------------

//US Bonding Priority Queue Descriptor Configuraton Table
//Section 5.4 in Spec - Already defined in E1

//US Bonding Priority Queue Shaping & Scheduling Configuration Table
//Section 5.5 in Spec - Already defined in E1

//US Bonding Priority Queue MIB Counters
//Section 5.6 in Spec - Already defined in E1

//US Bonding Group Context
//Section 5.7 in Spec
#define __US_BG_CTXT                        0x3F00
#define __US_BG_CTXT_SIZE                   16
//__US_BG_CTXT_SIZE_PER_BG = __US_BG_CTXT_SIZE/__NUM_US_BG
#define __US_BG_CTXT_SIZE_PER_BG            4

//DS Bonding Gamma Interface Linked-List Context
//Section 5.8 in Spec
#define __DS_BOND_FREE_LL_CTXT              0x26DC
#define __DS_BOND_FREE_LL_CTXT_SIZE         4

#define __DS_BOND_GIF_LL_CTXT               0x26E0
#define __DS_BOND_GIF_LL_CTXT_SIZE          32
//__DS_BOND_GIF_LL_CTXT_SIZE_PER_GIF = __DS_BOND_GIF_LL_CTXT_SIZE/__NUM_DS_GIFS
#define __DS_BOND_GIF_LL_CTXT_SIZE_PER_GIF  4

//DS Bonding Gamma Interface MIB
//Section 5.9 in Spec
#define __DS_BOND_GIF_MIB                   0x2E80
#define __DS_BOND_GIF_MIB_SIZE              128
//__DS_BG_GIF_MIB_SIZE_PER_BG = __DS_BG_GIF_MIB_SIZE/__NUM_DS_GIFS
#define __DS_BOND_GIF_MIB_SIZE_PER_BG       16

//DS Bonding Group MIB
//Section 5.10 in Spec
#define __DS_BG_MIB                         0x3F20
#define __DS_BG_MIB_SIZE                    64
//__DS_BG_MIB_SIZE_PER_BG = __DS_BG_MIB_SIZE/__NUM_DS_BG
#define __DS_BG_MIB_SIZE_PER_BG             16

//DS Bonding Group Context
//Section 5.11 in Spec
#define __DS_BG_CTXT                        0x3FE0
#define __DS_BG_CTXT_SIZE                   32
//__DS_BG_CTXT_SIZE_PER_BG = __DS_BG_CTXT_SIZE/__NUM_DS_BG
#define __DS_BG_CTXT_SIZE_PER_BG            8

//---------------------
//FW Internal Variables
//---------------------

//DS Internal Variables
#define __DS_SM_E1_FRAG_DES_RDIDX           0x5180
#define __DS_BM_E1_FRAG_DES_RDIDX           0x5181
#define __RX_FRAG_DISP_RDIDX                __DS_SM_E1_FRAG_DES_RDIDX

#define __DS_BOND_GIF_LL_NONEMPTY_FLAG      0x5182

#define __IS_PKT_FLUSHING					0x5183
#define __DMA_PKT_FLUSHING_CURR_BG			0x5184
#define __DMA_TX_CH1_DES_WRPTR				0x5185

//US Internal Variables
#define __US_E1_FRAG_Q_FRAG_MIB				0x5100
#define __US_E1_FRAG_Q_NON_EMPTY_FLAG		0x5108	

#define __US_E1_FRAG_Q_DES_WRIDX			0x5110
#define __US_E1_FRAG_Q_DES_RDIDX			0x5118
#define __US_E1_FRAG_Q_FREE_FRAG_CNT		0x5120

//US Macros definitions

//for US_E1_FRAG_Q descriptor fields
#define FRAG_TX_DESC_OWNER_E1_DMA_TX 		1
#define FRAG_TX_DESC_OWNER_B1				0

#define FRAG_TX_DESC_STATUS_RELEASED		0
#define FRAG_TX_DESC_STATUS_NOT_RELEASED	1

#define	FRAG_TX_DESC_RELEASED		0
#define FRAG_TX_DESC_NOT_RELEASED	1

//For BG_CTXT fields
#define PKT_STATUS_IN_PROCESS		1
#define PKT_STATUS_NOT_IN_PROCESS	0



#endif
