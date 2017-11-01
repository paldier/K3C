#ifndef __11AC_ACC_TX_ADDR_DEF_CFG_INC
#define __11AC_ACC_TX_ADDR_DEF_CFG_INC

#define __EMA_CMD_BUF                           				0x3000   //0x3000-307F
#define __EMA_DATA_BUF                          				0x3080   //0x3080-0x317F

//EMA accessible config/ctxt
//0x31A8-0x31AF
#define __CPU_CE4_READ_INDEX                              0x31AB
#define __CPU_CE4_WRITE_INDEX                             0x31AC 

//context
#define __LOCAL_CE4_READ_INDEX                            0x31a8
#define __LOCAL_CE4_WRITE_INDEX                           0x31a9
#define __CPU_CE4_PPE_READ_INDEX                          0x31aa
#define __LOCAL_CHIP_ID_ADDR                              0x31ad
#define __CPU_CE4_SW_IDX                                  0x31ae

#define __D6_HTT_TX_DES_BASE                            	0x36D0  //change from 0x3180
#define __D6_HTT_TX_DES_SIZE                            	128        // 8 * 16 entries
#define __D6_SUPPORT_VAP_NUM															16

#define __D6_PER_VAP_MIB_BASE                             0x4F00
#define __D6_PER_VAP_MIB_SIZE                             156      // 16 * 16 (VAP) 

#if 1
//__D6_TX_CFG_BASE     0x5F00-0x5F1F                             //change from 0x5Bxx
#define __CFG_DIRLINK_INT                                 0x5F00
#define __CFG_SIZE_TXPB                                   0x5F01
#define __CFG_INT2VAP_MAP                                 0x5F02
#define __CFG_BADR_TXPB                                   0x5F03  
#define __CFG_SIZE_TXHEADER                               0x5F04
#define __CFG_NUM_CE4SRCDES                               0x5F05 
#define __CFG_CE4DES_LOW                                  0x5F06 
#define __CFG_CE4DES_FULL                                 0x5F07
#define __CFG_BADR_CE4SRCDES                              0x5F08
#define __CFG_QCAMEM_BASE                                 0x5F09
#define __CFG_SIZE_CPUSRCDES                              0x5F0a

//supprt BEELINER
#define __CFG_BOARD_TYPE                                  0x5F0B
#define __BEELINER_FRAG_DES_BASE                          0x5EF0


//__D6_TX_CTX_BASE     0x5F20=>0x5F3F
#define __DMA_RXDES_INDEX                                 0x5F20  
#define __FREE_NUM_TXPB                                   0x5F21

#define __LOCAL_CE4_READ_INDEX_REQ                        0x5F22 

#define __CE4_READ_REQ_PERIOD_CFG                         0x5F23
#define __CE4_READ_REQ_PERIOD_CNT                         0x5F24
#define __FREE_PB_PTR_ADDR                                0x5F25


#define __D6_TX_PB_STATUS_BASE                            0x5F40
#define __D6_TX_PB_STATUS_SIZE                            133  // 1 + 4 + 128 to maintain 4K bytes packet buffer
#define __FREE_PB_GROUP1024_STATUS                        0x5F40
#define __FREE_PB_GROUP32_STATUS_BASE                     0x5F41
#define __FREE_PB_GROUP32_STATUS_SIZE                     4
#define __FREE_PB_STATUS_BASE                             0x5F45  
#define __FREE_PB_STATUS_SIZE                             128

#define __D6_RXDMA7_DES_BASE                              0x5FD0
#define __D6_RXDMA7_DES_SIZE                              16  // 2 * 8 entries

#else
//__D6_TX_CFG_BASE     0x5F00-0x5F1F
#define __CFG_DIRLINK_INT                                 0x5B00
#define __CFG_SIZE_TXPB                                   0x5B01
#define __CFG_INT2VAP_MAP                                 0x5B02
#define __CFG_BADR_TXPB                                   0x5B03  
#define __CFG_SIZE_TXHEADER                               0x5B04
#define __CFG_NUM_CE4SRCDES                               0x5B05 
#define __CFG_CE4DES_LOW                                  0x5B06 
#define __CFG_CE4DES_FULL                                 0x5B07
#define __CFG_BADR_CE4SRCDES                              0x5B08
#define __CFG_QCAMEM_BASE                                 0x5B09
#define __CFG_SIZE_CPUSRCDES                              0x5B0a

//__D6_TX_CTX_BASE     0x5F20=>0x5F3F
#define __DMA_RXDES_INDEX                                 0x5b20  
#define __FREE_NUM_TXPB                                   0x5b21

#define __LOCAL_CE4_READ_INDEX_REQ                        0x5b22 

#define __CE4_READ_REQ_PERIOD_CFG                         0x5b23
#define __CE4_READ_REQ_PERIOD_CNT                         0x5b24
#define __FREE_PB_PTR_ADDR                                0x5b25

#define __D6_TX_PB_STATUS_BASE                            0x5B40
#define __D6_TX_PB_STATUS_SIZE                            133  // 1 + 4 + 128 to maintain 4K bytes packet buffer
#define __FREE_PB_GROUP1024_STATUS                        0x5B40
#define __FREE_PB_GROUP32_STATUS_BASE                     0x5B41
#define __FREE_PB_GROUP32_STATUS_SIZE                     4
#define __FREE_PB_STATUS_BASE                             0x5B45  
#define __FREE_PB_STATUS_SIZE                             128

#define __D6_RXDMA7_DES_BASE                              0x5BD0
#define __D6_RXDMA7_DES_SIZE                              16  // 2 * 8 entries

#endif 

#define __D6_TXPB_MSG0_BASE                               0x5C00
#define __D6_TXPB_MSG0_SIZE                               129

#define __D6_TXPB_MSG1_BASE                               0x5C90
#define __D6_TXPB_MSG1_SIZE                               129

//  CPU CE4 Descriptor Queue buffer 64*2 DWs	
	#define __CPU_CE4_BUF_BASE              					0x5D40

#endif
