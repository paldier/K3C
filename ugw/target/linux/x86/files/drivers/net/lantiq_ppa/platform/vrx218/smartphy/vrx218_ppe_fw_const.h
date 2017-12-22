#ifndef VRX218_Bonding_const_h__
#define VRX218_Bonding_const_h__

//----------------------------------------------------------------------
// DES_SYNC_READ number of descriptor per read command config
//
//      set DES_SYNC_READ_DES_NUM to 4 or 8
//
//
// for us stream, we will not issue more des_read unless
//      to_read_num >= US_DES_SYNC_READ_GUARD
// where
//      to_read_num            = desc_num - (sync_rd_cnt - sync_wr_cnt)
//      US_DES_SYNC_READ_GUARD = DES_SYNC_READ_DES_NUM + 4
#if defined(DES_SYNC_READ_DES_NUM) && (DES_SYNC_READ_DES_NUM == 8)

    #define DES_SYNC_READ_DW_NUM    16
    #define US_DES_SYNC_READ_GUARD  12

#else

    #define DES_SYNC_READ_DES_NUM   4
    #define DES_SYNC_READ_DW_NUM    8
    #define US_DES_SYNC_READ_GUARD  8

#endif

//----------------------------------------------------------------------

// typedef EDMA_LLE.transfer_type
#define PKT_WRITE               0
#define PKT_READ                1
#define DES_WRITE_DW0           2
#define DES_WRITE_DW1           3
#define DES_READ                4

// typedef DES_SYNC_CFG_CTXT.sync_type
#define US_READ_WRITE_SYNC      0
#define DS_WRITE_READ_SYNC      1

// typedef DES_SYNC_CFG_CTXT.sync_rd_status
#define SYNC_RD_IDLE            0
#define SYNC_RD_CMD_READY       1
#define SYNC_RD_CMD_ISSUED      2
#define SYNC_RD_CMD_DONE        3

// typedef DES_SYNC_CFG_CTXT.sync_wr_status
#define SYNC_WR_IDLE            0
#define SYNC_WR_CMD_READY       1
#define SYNC_WR_CMD_DW1_ISSUED  2
#define SYNC_WR_CMD_DW0_ISSUED  3
#define SYNC_WR_CMD_DONE        4

// typedef EDMA_RD_CNT_SM.state
#define ERCS_IDLE               0
#define ERCS_WRITING_IDX        1
#define ERCS_READING_LL_PTR     2

// typedef EDMA_CH_CTXT.edma_ch_type
// typedef EDMA_RD_CNT_SM.ch_id
#define EDMA_WRITE_CH           0
#define EDMA_READ_CH            1

#define SIZE_OF_EDMA_LLE            6
#define SIZE_OF_EDMA_LLE_EXT        SIZE_OF_EDMA_LLE
#define SIZE_OF_DES_SYNC_CFG_CTXT   32
#define SIZE_OF_EDMA_CH_CTXT        8
#define SIZE_OF_EDMA_COPY_CH_CFG    1

#define MIN_FREE_LLE_NUM            2
#define MIN_FREE_DMAL_FIFO_SLOT     6

#define FPI_FOR_SB0x2000            0x1E220000
#define FPI_FOR_SB0x2000_HI         0x1E22
#define FPI_FOR_SB0x2000_LO         0x0

#define PKT_REASSEMBLING                    0
#define PKT_ASSEMBLY_DONE                   1
#define PKT_FLUSHING                        2

// CDMA copy channel des own bit definition
#define CDMA_DES_MIPS_OWN                0
#define CDMA_DES_DMA_OWN                 1

#define EDMA_STOPPED                        3
#define EDMA_RUNNING                        1

//Number of Bonding Groups in Upstream
#define __NUM_US_BG                         4
//Number of Gamma Interfaces in Upstream
#define __NUM_US_GIFS                       8
//Number of Bonding Groups in Downstream
#define __NUM_DS_BG                         4
//Number of Gamma Interfaces in Downstream
#define __NUM_DS_GIFS                       8

#define __MAX_PKT_SIZE                      1604
#define __MAX_FRAG_NUM_PER_PKT              30

//US Macros definitions

//for US_E1_FRAG_Q descriptor fields
#define FRAG_TX_DESC_OWNER_E1_DMA_TX        1
#define FRAG_TX_DESC_OWNER_B1               0

#define FRAG_TX_DESC_STATUS_RELEASED        0
#define FRAG_TX_DESC_STATUS_NOT_RELEASED    1

#define FRAG_TX_DESC_RELEASED               0
#define FRAG_TX_DESC_NOT_RELEASED           1

//For BG_CTXT fields
#define PKT_STATUS_IN_PROCESS               1
#define PKT_STATUS_NOT_IN_PROCESS           0

#define __US_E1_FRAG_DES_NUM_PER_GIF            16

// constant
// PDMA maximum burst is 8 dwords
// set maximum read number to be 4
#define XPCI_DES_READ_NUM                   4
#define XPCI_DES_WRITE_MAX_NUM              8

// to keep des_read_ptr and des_write_ptr from reach
// each other too close
#define XPCI_DES_RW_WINDOW_SIZE             52
#define XPCI_DES_WRITE_GUARD_DIST           4

// when segment a fragment, there should be at least
// US_SEG_GUARD_DIST free descriptor to avoid read/write clash
#define US_SEG_GUARD_DIST                   2

#define XPCI_DES_RW_NONE                    0
#define XPCI_DES_RW_CMD_READY               1
#define XPCI_DES_RW_CMD_INCMDBUF            2
#define XPCI_DES_RW_CMD_DONE                3

#define DMA_OWN                             0
#define PPE_OWN                             1

//Constants used by "Unified QoS" Module
#define __QOS_DISPATCH_OWN      0
#define __SHAPING_WFQ_SCHED_OWN 1

//Constants used by "PPE DSL Notifications" Module (ppe_dsl_notifications.asm)
#define S_44K_OWN_DSL   0
#define S_44K_OWN_PPE   1

#define __DREG_SIGNATURE_VAL_HWORD  0xA5A5

//Constants used by "Bonding - Descriptor Synchronization" Module
#define __BOND_DES_SYNC_IDLE_STATE      0
#define __BOND_DES_SYNC_CDMA_READ_STATE 1

#define READ_WRITE_SYNC     0
#define WRITE_READ_SYNC     1

#define UPSTREAM            0
#define DOWNSTREAM          1

#endif
