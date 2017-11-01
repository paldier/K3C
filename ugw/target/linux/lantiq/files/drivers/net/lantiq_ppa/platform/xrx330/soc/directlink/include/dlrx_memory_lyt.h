
#ifndef __DLRX_MEMORY_LYT_H_
#define __DLRX_MEMORY_LYT_H_

#define DLRX_SRAM_PHY_ADDR      0x1F107400
//JAMES change from 0x1000 to 0x2000
#define DLRX_CFG_CTXT_MAX_SIZE  0x2000 //The maximum size is fixed for DLRX FW as 4Kbyte

//Definition of length for each Data Structure
#define GSWIP_DESC_NUM              254
#define WLAN_DESC_NUM               254
#define PROTO_DESC_NUM              254

#define CPU_CE5_DESC_RING_NUM       2
#define RX_PKT_BUF_REL_MSG_NUM      2
#define CE5_DEST_DESC_RING_NUM      512
#define CE5_DEST_MSG_BUF_NUM        CE5_DEST_DESC_RING_NUM
#define RX_PKT_BUF_PTR_RING_NUM     1024     //Must >= desc ring num 
#define RX_REORDER_MAIN_NUM         2048
#define RX_REORDER_DESC_LINK_NUM    4095

#define RX_PEER_ID_PEER_MAP         132
#define RX_PEER_TO_VAP_PN           128
#define RX_PEER_RESET               1
#define RX_VAP2INT_MAP1             1
#define RX_VAP2INT_MAP2             1

#define DLRX_CE5_DEST_BUF_SIZE 	    512
#define QCA_PEREGRINE_11AC_CFG_OFFSET_ATTEN   248    //BYTES
#define QCA_BEELINER_11AC_CFG_OFFSET_ATTEN   296 //BYTES


#define TRUE 1
#define FALSE 0

#define ZERO 0
#define ONE 1

#define VALID 1
#define INVALID 0

/* DMA  Related Definition */
#define DMA_BASE            0xBE104100
#define DMA_CS              (volatile u32*)(DMA_BASE + 0x18)
#define DMA_CCTRL           (volatile u32*)(DMA_BASE + 0x1C)
#define DMA_CDBA            (volatile u32*)(DMA_BASE + 0x20)
#define DMA_CDLEN           (volatile u32*)(DMA_BASE + 0x24)
#define DMA_CIS             (volatile u32*)(DMA_BASE + 0x28)
#define DMA_CIE             (volatile u32*)(DMA_BASE + 0x2C)
#define PPE_TX_CH_NO        3

typedef struct {
	uint32_t gswip_desc_num;
	uint32_t wlan_desc_num;
	uint32_t proto_desc_num;
	uint32_t cpu_ce5_desc_ring_num;
	uint32_t rx_pkt_buf_rel_msg_num;
	uint32_t ce5_dest_desc_ring_num;
	uint32_t ce5_dest_msg_buf_num;
	uint32_t rx_pkt_buf_ptr_ring_num;
	uint32_t rx_reorder_main_num;
	uint32_t rx_reorder_desc_link_num;
}dlrx_bufsize_t;


#endif
