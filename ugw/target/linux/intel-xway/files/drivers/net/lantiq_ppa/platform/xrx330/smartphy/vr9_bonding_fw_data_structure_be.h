
#ifndef __VR9_BONDING_FW_DATA_STRUCTURE_BE_H_
#define __VR9_BONDING_FW_DATA_STRUCTURE_BE_H_

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int res0:3;
        unsigned int byte_off:2;
        unsigned int qos:4;
        unsigned int res1:3;
        unsigned int data_len:16;

        unsigned int data_ptr:32;

    } cdma_rx_des_t;

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int byte_off:5;
        unsigned int res0:7;
        unsigned int data_len:16;

        unsigned int data_ptr:32;

    } cdma_tx_des_t;

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int byte_off:5;
        unsigned int last_frag:1;
        unsigned int frag_num:6;
        unsigned int data_len:16;

        unsigned int data_ptr:32;

    } us_bond_pkt_des_t;

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int byte_off:5;
        unsigned int qid:4;
        unsigned int res0:3;
        unsigned int data_len:16;

        unsigned int data_ptr:32;

    } us_e1_frag_des_t;

    typedef struct {

        unsigned int frag_header:16;
        unsigned int status:1;
        unsigned int pkt_des_ptr:15;

    } us_e1_frag_des_bp_t;

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int byte_off:5;
        unsigned int gid:2;
        unsigned int res0:5;
        unsigned int data_len:16;

        unsigned int data_ptr:32;

    } ds_e1_frag_des_t;

    typedef struct {

        unsigned int next_des_ptr:16;
        unsigned int data_len:16;

        unsigned int data_ptr:32;

    } ds_bond_gif_ll_des_t;

    typedef struct {

        unsigned int family:4;
        unsigned int fw_type:4;
        unsigned int interface:4;
        unsigned int fw_mode:4;
        unsigned int major:8;
        unsigned int minor:8;

    } fw_ver_id_t;

    typedef struct {

        unsigned int max_frag_size:16;
        unsigned int polling_ctrl_cnt:8;
        unsigned int dplus_fp_fcs_en:1;
        unsigned int bg_num:3;
        unsigned int bond_mode:1;
        unsigned int e1_bond_en:1;
        unsigned int d5_acc_dis:1;
        unsigned int d5_b1_en:1;

    } bond_conf_t;

    typedef struct {

        unsigned int queue_map3:8;
        unsigned int queue_map2:8;
        unsigned int queue_map1:8;
        unsigned int queue_map0:8;

    } us_bg_qmap_t;

    typedef struct {

        unsigned int gif_map3:8;
        unsigned int gif_map2:8;
        unsigned int gif_map1:8;
        unsigned int gif_map0:8;

    } us_bg_gmap_t;

    typedef struct {

        unsigned int gif_map3:8;
        unsigned int gif_map2:8;
        unsigned int gif_map1:8;
        unsigned int gif_map0:8;

    } ds_bg_gmap_t;

    typedef struct {

        unsigned int time;

    } curr_time_stamp_t;

    typedef struct {

        unsigned int bp_desba:16;
        unsigned int desba:16;

    } us_e1_frag_desba_t;

    typedef struct {

        unsigned int dma_des_ba:16;
        unsigned int desba:16;

    } ds_e1_frag_desba_t;

    typedef struct {

        unsigned int prefix:16;
        unsigned int mask:16;

    } data_ptr_pdma_prefix_cfg_t;

    typedef struct {

        unsigned int sid:14;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int pkt_status:1;
        unsigned int des_addr:15;

        unsigned int data_ptr:32;

        unsigned int res1:9;
        unsigned int qid:4;
        unsigned int gif_id:3;
        unsigned int rem_len:16;

        unsigned int _res0:16;
        unsigned int desq_cfg_ctxt_ptr:16;

    } us_bg_ctxt_t;

    typedef struct {

        unsigned int tail_ptr:16;
        unsigned int head_ptr:16;

        unsigned int sid:14;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int fh_valid:1;
        unsigned int des_num:15;

        unsigned int max_des_num:8;
        unsigned int to_buff_thres:8;
        unsigned int max_delay:16;

        unsigned int timeout;

    } ds_bond_gif_ll_ctxt_t;

    typedef struct {

        unsigned int total_rx_frag_cnt;

        unsigned int total_rx_byte_cnt;

        unsigned int overflow_frag_cnt;

        unsigned int overflow_byte_cnt;

        unsigned int out_of_range_frag_cnt;

        unsigned int missing_frag_cnt;

        unsigned int timeout_frag_cnt;

        unsigned int _dw_res0[9];

    } ds_bond_gif_mib_t;

    typedef struct {

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

    } ds_bg_mib_t;

    typedef struct {

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

    } ds_bg_ctxt_t;

    typedef struct {

        unsigned int byte_cnt:10;
        unsigned int int_mem_addr_off:11;
        unsigned int res0:6;
        unsigned int eop:1;
        unsigned int pdma:1;
        unsigned int sar:1;
        unsigned int bc:2;

        unsigned int res1:1;
        unsigned int ext_mem_addr:29;
        unsigned int release:1;
        unsigned int pdma_cmd_type:1;

    } pdma_dira_cmd_t;

#endif

