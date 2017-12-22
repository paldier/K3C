
#ifndef __VR9_BONDING_FW_DATA_STRUCTURE_H_
#define __VR9_BONDING_FW_DATA_STRUCTURE_H_

	#ifdef __BIG_ENDIAN

	struct CDMA_RX_DES {

		unsigned int own:1;
		unsigned int c:1;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int res0:3;
		unsigned int byte_off:2;
		unsigned int qos:4;
		unsigned int res1:3;
		unsigned int data_len:16;

		unsigned int res2:2;
		unsigned int data_ptr:30;

	} ;

	struct CDMA_TX_DES {

		unsigned int own:1;
		unsigned int c:1;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int byte_off:5;
		unsigned int res0:7;
		unsigned int data_len:16;

		unsigned int res1:2;
		unsigned int data_ptr:30;

	} ;

	struct US_BOND_PKT_DES {

		unsigned int own:1;
		unsigned int c:1;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int byte_off:5;
		unsigned int last_frag:1;
		unsigned int frag_num:6;
		unsigned int data_len:16;

		unsigned int res0:2;
		unsigned int data_ptr:30;

	} ;

	struct US_E1_FRAG_DES {

		unsigned int own:1;
		unsigned int c:1;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int byte_off:5;
		unsigned int qid:4;
		unsigned int res0:3;
		unsigned int data_len:16;

		unsigned int res1:2;
		unsigned int data_ptr:30;

	} ;

	struct US_E1_FRAG_DES_BP {

		unsigned int frag_header:16;
		unsigned int status:1;
		unsigned int pkt_des_ptr:15;

	} ;

	struct DS_E1_FRAG_DES {

		unsigned int own:1;
		unsigned int c:1;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int byte_off:5;
		unsigned int gid:2;
		unsigned int res0:5;
		unsigned int data_len:16;

		unsigned int res1:2;
		unsigned int data_ptr:30;

	} ;

	struct DS_BOND_GIF_LL_DES {

		unsigned int next_des_ptr:16;
		unsigned int data_len:16;

		unsigned int res1:2;
		unsigned int data_ptr:30;

	} ;

	struct CURR_TIME_STAMP {

		unsigned int time;

	} ;

	struct US_E1_FRAG_DESBA {

		unsigned int bp_desba:16;
		unsigned int desba:16;

	} ;

	struct DS_E1_FRAG_DESBA {

		unsigned int _res0:16;
		unsigned int desba:16;

	} ;

	struct US_BG_CTXT{

		unsigned int sid:14;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int pkt_status:1;
		unsigned int des_addr:15;

		unsigned int res0:1;
		unsigned int data_ptr:31;

		unsigned int res1:9;
		unsigned int qid:4;
		unsigned int gif_id:3;
		unsigned int rem_len:16;

		unsigned int _dw_res0;

	} ;
	

	struct DS_BOND_GIF_LL_CTXT{

		unsigned int tail_ptr:16;
		unsigned int head_ptr:16;

		unsigned int sid:14;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int fh_valid:1;
		unsigned int des_num:15;

		unsigned int max_delay;

		unsigned int timeout;

	} ;
	

	

	struct DS_BOND_GIF_MIB {

		unsigned int total_rx_frag_cnt;

		unsigned int total_rx_byte_cnt;

		unsigned int overflow_frag_cnt;

		unsigned int overflow_byte_cnt;

		unsigned int out_of_range_frag_cnt;

		unsigned int missing_frag_cnt;

		unsigned int timeout_frag_cnt;

		unsigned int _dw_res0[9];

	} ;

	struct DS_BG_MIB {

		unsigned int conform_pkt_cnt;

		unsigned int conform_frag_cnt;

		unsigned int conform_byte_cnt;

		unsigned int no_sop_pkt_cnt;

		unsigned int no_sop_frag_cnt;

		unsigned int no_sop_byte_cnt;

		unsigned int no_eop_pkt_cnt;

		unsigned int no_eop_frag_cnt;

		unsigned int no_eop_byte_cnt;

		unsigned int oversize_pkt_cnt;

		unsigned int oversize_frag_cnt;

		unsigned int oversize_byte_cnt;

		unsigned int noncosec_pkt_cnt;

		unsigned int noncosec_frag_cnt;

		unsigned int noncosec_byte_cnt;

		unsigned int _dw_res0;

	} ;

	struct DS_BG_CTXT {

		unsigned int link_state_chg:1;
		unsigned int res0:15;
		unsigned int expected_sid:14;
		unsigned int last_sop:1;
		unsigned int last_eop:1;

		unsigned int res1:22;
		unsigned int bg_pkt_state:2;
		unsigned int res2:3;
		unsigned int noncosec_flag:1;
		unsigned int oversize_flag:1;
		unsigned int no_eop_flag:1;
		unsigned int no_sop_flag:1;
		unsigned int no_err_flag:1;

		unsigned int curr_pkt_frag_cnt;

		unsigned int curr_pkt_byte_cnt;

		unsigned int tail_ptr:16;
		unsigned int head_ptr:16;

		unsigned int sid:14;
		unsigned int sop:1;
		unsigned int eop:1;
		unsigned int fh_valid:1;
		unsigned int des_num:15;

		unsigned int _dw_res0[2];

	} ;

	#else

	struct CDMA_RX_DES {

		unsigned int data_len:16;
		unsigned int res1:3;
		unsigned int qos:4;
		unsigned int byte_off:2;
		unsigned int res0:3;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int c:1;
		unsigned int own:1;

		unsigned int data_ptr:30;
		unsigned int res2:2;

	} ;

	struct CDMA_TX_DES {

		unsigned int data_len:16;
		unsigned int res0:7;
		unsigned int byte_off:5;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int c:1;
		unsigned int own:1;

		unsigned int data_ptr:30;
		unsigned int res1:2;

	} CDMA_TX_DES;

	struct US_BOND_PKT_DES {

		unsigned int data_len:16;
		unsigned int frag_num:6;
		unsigned int last_frag:1;
		unsigned int byte_off:5;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int c:1;
		unsigned int own:1;

		unsigned int data_ptr:30;
		unsigned int res0:2;

	} ;

	struct US_E1_FRAG_DES {

		unsigned int data_len:16;
		unsigned int res0:3;
		unsigned int qid:4;
		unsigned int byte_off:5;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int c:1;
		unsigned int own:1;

		unsigned int data_ptr:30;
		unsigned int res1:2;

	} ;

	struct US_E1_FRAG_DES_BP {

		unsigned int pkt_des_ptr:15;
		unsigned int status:1;
		unsigned int frag_header:16;

	} ;

	struct DS_E1_FRAG_DES {

		unsigned int data_len:16;
		unsigned int res0:5;
		unsigned int gid:2;
		unsigned int byte_off:5;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int c:1;
		unsigned int own:1;

		unsigned int data_ptr:30;
		unsigned int res1:2;

	} ;

	struct DS_BOND_GIF_LL_DES {

		unsigned int data_len:16;
		unsigned int next_des_ptr:16;

		unsigned int data_ptr:30;
		unsigned int res1:2;

	} ;

	struct FW_VER_ID {

		unsigned int minor:8;
		unsigned int major:8;
		unsigned int fw_mode:4;
		unsigned int interface:4;
		unsigned int fw_type:4;
		unsigned int family:4;

	} ;

	struct BOND_CONF {

		unsigned int d5_b1_en:1;
		unsigned int d5_acc_dis:1;
		unsigned int e1_bond_en:1;
		unsigned int bond_mode:1;
		unsigned int bg_num:3;
		unsigned int rx_buf_size:9;
		unsigned int max_frag_size:16;

	} ;

	struct US_BG_QMAP {

		unsigned int queue_map0:8;
		unsigned int queue_map1:8;
		unsigned int queue_map2:8;
		unsigned int queue_map3:8;

	} ;

	struct US_BG_GMAP {

		unsigned int gif_map0:8;
		unsigned int gif_map1:8;
		unsigned int gif_map2:8;
		unsigned int gif_map3:8;

	} ;

	struct DS_BG_GMAP {

		unsigned int gif_map0:8;
		unsigned int gif_map1:8;
		unsigned int gif_map2:8;
		unsigned int gif_map3:8;

	} ;

	struct CURR_TIME_STAMP {

		unsigned int time;

	} ;

	struct US_E1_FRAG_DESBA {

		unsigned int desba:16;
		unsigned int bp_desba:16;

	} ;

	struct DS_E1_FRAG_DESBA {

		unsigned int desba:16;
		unsigned int _res0:16;

	} ;

	struct US_BG_CTXT {

		unsigned int des_addr:15;
		unsigned int pkt_status:1;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int sid:14;

		unsigned int data_ptr:31;
		unsigned int res0:1;

		unsigned int rem_len:16;
		unsigned int gif_id:3;
		unsigned int qid:4;
		unsigned int res1:9;

		unsigned int _dw_res0;

	} ;

	struct DS_BOND_GIF_LL_CTXT {

		unsigned int head_ptr:16;
		unsigned int tail_ptr:16;

		unsigned int des_num:15;
		unsigned int fh_valid:1;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int sid:14;

		unsigned int max_delay;

		unsigned int timeout;

	} ;

	struct DS_BOND_GIF_MIB {

		unsigned int total_rx_frag_cnt;

		unsigned int total_rx_byte_cnt;

		unsigned int overflow_frag_cnt;

		unsigned int overflow_byte_cnt;

		unsigned int out_of_range_frag_cnt;

		unsigned int missing_frag_cnt;

		unsigned int timeout_frag_cnt;

		unsigned int _dw_res0[9];

	} ;

	struct DS_BG_MIB {

		unsigned int conform_pkt_cnt;

		unsigned int conform_frag_cnt;

		unsigned int conform_byte_cnt;

		unsigned int no_sop_pkt_cnt;

		unsigned int no_sop_frag_cnt;

		unsigned int no_sop_byte_cnt;

		unsigned int no_eop_pkt_cnt;

		unsigned int no_eop_frag_cnt;

		unsigned int no_eop_byte_cnt;

		unsigned int oversize_pkt_cnt;

		unsigned int oversize_frag_cnt;

		unsigned int oversize_byte_cnt;

		unsigned int noncosec_pkt_cnt;

		unsigned int noncosec_frag_cnt;

		unsigned int noncosec_byte_cnt;

		unsigned int _dw_res0;

	} ;

	struct DS_BG_CTXT {

		unsigned int last_eop:1;
		unsigned int last_sop:1;
		unsigned int expected_sid:14;
		unsigned int res0:15;
		unsigned int link_state_chg:1;

		unsigned int no_err_flag:1;
		unsigned int no_sop_flag:1;
		unsigned int no_eop_flag:1;
		unsigned int oversize_flag:1;
		unsigned int noncosec_flag:1;
		unsigned int res2:3;
		unsigned int bg_pkt_state:2;
		unsigned int res1:22;

		unsigned int curr_pkt_frag_cnt;

		unsigned int curr_pkt_byte_cnt;

		unsigned int head_ptr:16;
		unsigned int tail_ptr:16;

		unsigned int des_num:15;
		unsigned int fh_valid:1;
		unsigned int eop:1;
		unsigned int sop:1;
		unsigned int sid:14;

		unsigned int _dw_res0[2];

	} ;

	#endif

#endif

