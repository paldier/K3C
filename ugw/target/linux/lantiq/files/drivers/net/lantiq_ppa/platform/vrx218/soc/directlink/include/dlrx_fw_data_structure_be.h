
#ifndef __DLRX_FW_DATA_STRUCTURE_BE_H_
#define __DLRX_FW_DATA_STRUCTURE_BE_H_

    typedef struct {

        unsigned int msdu_load_status:1;
        unsigned int insig1:28;
        unsigned int mcast_bcast:1;
        unsigned int insig0:2;

        unsigned int insig3:8;
        unsigned int msdu_chain_num:8;
        unsigned int insig2:16;

        unsigned int insig5:4;
        unsigned int seqid:12;
        unsigned int insig4:16;

        unsigned int pn_31_0;

        unsigned int insig6:16;
        unsigned int pn_47_32:16;

        unsigned int insig7:18;
        unsigned int msdu_len:14;

        unsigned int insig8;

        unsigned int insig9;

        unsigned int insig10;

        unsigned int pn_63_48:16;
        unsigned int insig11:16;

        unsigned int pn_95_64;

        unsigned int pn_127_96;

        unsigned int insig13:16;
        unsigned int last_msdu:1;
        unsigned int first_msdu:1;
        unsigned int insig12:14;

    } dlrx_rxpb_hdr_peregrine_t;

    typedef struct {

        unsigned int msdu_load_status:1;
        unsigned int insig1:28;
        unsigned int mcast_bcast:1;
        unsigned int insig0:2;

        unsigned int insig3:8;
        unsigned int msdu_chain_num:8;
        unsigned int insig2:16;

        unsigned int insig5:4;
        unsigned int seqid:12;
        unsigned int insig4:16;

        unsigned int pn_31_0;

        unsigned int insig6:16;
        unsigned int pn_47_32:16;

        unsigned int insig7:18;
        unsigned int msdu_len:14;

        unsigned int insig8;

        unsigned int insig9;

        unsigned int insig10;

        unsigned int insig11;

        unsigned int pn_63_48:16;
        unsigned int insig12:16;

        unsigned int pn_95_64;

        unsigned int pn_127_96;

        unsigned int insig14:16;
        unsigned int last_msdu:1;
        unsigned int first_msdu:1;
        unsigned int insig13:14;

        unsigned int insig15;

        unsigned int insig16;

        unsigned int insig17;

        unsigned int insig19:19;
        unsigned int l3_header_padding:3;
        unsigned int insig18:10;

        unsigned int insig20;

    } dlrx_rxpb_hdr_beeliner_t;

    typedef struct {

        unsigned int rsvd3:4;
        unsigned int src_int:4;
        unsigned int rsvd2:4;
        unsigned int dest_int:4;
        unsigned int rsvd1:3;
        unsigned int fwd1:1;
        unsigned int vap:4;
        unsigned int rsvd0:6;
        unsigned int discard:1;
        unsigned int fwd:1;

    } dlrx_rxpb_pmac_hdr_t;

    typedef struct {

        unsigned int rsvd0:16;
        unsigned int mpdu_status:8;
        unsigned int fw_rx_desc_byte:8;

    } dlrx_rxpb_wlan_drv_hdr_t;

    typedef struct {

        unsigned int htt_hdr[2];

        unsigned int rsvd1:6;
        unsigned int peer_id:10;
        unsigned int rsvd0:1;
        unsigned int rv:1;
        unsigned int fv:1;
        unsigned int ext_tid:5;
        unsigned int msg_type:8;

        unsigned int mpdu_ranges_num:8;
        unsigned int release_end_seqid:6;
        unsigned int release_start_seqid:6;
        unsigned int flush_end_seqid:6;
        unsigned int flush_start_seqid:6;

        unsigned int rsvd2;

        unsigned int rsvd3;

        unsigned int rsvd4;

        unsigned int rsvd5;

        unsigned int rsvd6;

        unsigned int rsvd7;

        unsigned int rsvd8;

        unsigned int rsvd9;

        unsigned int rsvd10;

        unsigned int rsvd11:16;
        unsigned int fw_rx_desc_byte_num:16;

    } dlrx_ind_msg_t;

    typedef struct {

        unsigned int rsvd2_msdu3:2;
        unsigned int inspect_msdu3:1;
        unsigned int rsvd1_msdu3:3;
        unsigned int forward_msdu3:1;
        unsigned int discard_msdu3:1;
        unsigned int rsvd2_msdu2:2;
        unsigned int inspect_msdu2:1;
        unsigned int rsvd1_msdu2:3;
        unsigned int forward_msdu2:1;
        unsigned int discard_msdu2:1;
        unsigned int rsvd2_msdu1:2;
        unsigned int inspect_msdu1:1;
        unsigned int rsvd1_msdu1:3;
        unsigned int forward_msdu1:1;
        unsigned int discard_msdu1:1;
        unsigned int rsvd2_msdu0:2;
        unsigned int inspect_msdu0:1;
        unsigned int rsvd1_msdu0:3;
        unsigned int forward_msdu0:1;
        unsigned int discard_msdu0:1;

    } fw_rx_desc_byte_t;

    typedef struct {

        unsigned int mpdu_status_mpdu_range1:8;
        unsigned int mpdu_cnt_mpdu_range1:8;
        unsigned int mpdu_status_mpdu_range0:8;
        unsigned int mpdu_cnt_mpdu_range0:8;

    } ctxt_mpdu_t;

    typedef struct {

        unsigned int htt_hdr[2];

        unsigned int rsvd1:3;
        unsigned int ext_tid:5;
        unsigned int rsvd0:6;
        unsigned int peer_id:10;
        unsigned int msg_type:8;

        unsigned int rsvd4:2;
        unsigned int flush_end_seqid:6;
        unsigned int rsvd3:2;
        unsigned int flush_start_seqid:6;
        unsigned int mpdu_status:8;
        unsigned int rsvd2:8;

    } dlrx_flush_msg_t;

    typedef struct {

        unsigned int htt_hdr[2];

        unsigned int rsvd1:6;
        unsigned int peer_id:10;
        unsigned int rsvd0:2;
        unsigned int fv:1;
        unsigned int ext_tid:5;
        unsigned int msg_type:8;

        unsigned int rsvd2:20;
        unsigned int flush_end_seqid:6;
        unsigned int flush_start_seqid:6;

        unsigned int rsvd3:16;
        unsigned int fw_rx_desc_byte_num:16;

        unsigned int padding:24;
        unsigned int fw_rx_desc_byte_msdu0:8;

        unsigned int rsvd4:20;
        unsigned int rxpb_ptr_read_index:12;

    } dlrx_frag_ind_msg_t;

    typedef struct {

        unsigned int htt_hdr[2];

        unsigned int _res0:8;
        unsigned int pb_ptr_rel_num:8;
        unsigned int _res1:5;
        unsigned int status:3;
        unsigned int msg_type:8;

        unsigned int free_txpb_ptr[128];

    } dlrx_tx_cmpl_msg_t;

    typedef struct {

        unsigned int last_pn_dw0;

        unsigned int last_pn_dw1;

        unsigned int last_pn_dw2;

        unsigned int last_pn_dw3;

        unsigned int mcast_bcast:1;
        unsigned int msdu_num:15;
        unsigned int first_ptr:16;

        unsigned int _dw_res0[63];

    } dlrx_ro_mainlist_t;

    typedef struct {

        unsigned int pn_dw0;

        unsigned int pn_dw1;

        unsigned int pn_dw2;

        unsigned int pn_dw3;

        unsigned int next_ptr:12;
        unsigned int rsvd0:1;
        unsigned int inspect:1;
        unsigned int discard:1;
        unsigned int fwd:1;
        unsigned int msdu_len:16;

        unsigned int rxpb_ptr;

    } dlrx_ro_linklist_t;

    typedef struct {

        unsigned int rsvd1:9;
        unsigned int pb_ptr_rel_num:7;
        unsigned int rsvd0:16;

        unsigned int rxpb_ptr[127];

    } dlrx_rxpb_ptr_rel_msg_t;

    typedef struct {

        unsigned int txpdu_low;

        unsigned int txpdu_high;

        unsigned int txbytes_low;

        unsigned int txbytes_high;

        unsigned int txdrop_low;

        unsigned int txdrop_high;

        unsigned int rx_fwd_pdu_low;

        unsigned int rx_fwd_pdu_high;

        unsigned int rx_fwd_bytes_low;

        unsigned int rx_fwd_bytes_high;

        unsigned int rx_inspect_pdu_low;

        unsigned int rx_inspect_pdu_high;

        unsigned int rx_inspect_bytes_low;

        unsigned int rx_inspect_bytes_high;

        unsigned int rx_discard_pdu_low;

        unsigned int rx_discard_pdu_high;

        unsigned int rx_discard_bytes_low;

        unsigned int rx_discard_bytes_high;

        unsigned int rx_pn_pdu_low;

        unsigned int rx_pn_pdu_high;

        unsigned int rx_pn_bytes_low;

        unsigned int rx_pn_bytes_high;

        unsigned int rx_drop_pdu_low;

        unsigned int rx_drop_pdu_high;

        unsigned int rx_drop_bytes_low;

        unsigned int rx_drop_bytes_high;

        unsigned int rx_rcv_pdu_low;

        unsigned int rx_rcv_pdu_high;

        unsigned int rx_rcv_bytes_low;

        unsigned int rx_rcv_bytes_high;

        unsigned int _dw_res0[2];

    } vap_data_mib_t;

    typedef struct {

        unsigned int rx_gswip_packets_low;

        unsigned int rx_gswip_packets_high;

        unsigned int rx_gswip_bytes_low;

        unsigned int rx_gswip_bytes_high;

        unsigned int rx_wlan_packets_low;

        unsigned int rx_wlan_packets_high;

        unsigned int rx_wlan_bytes_low;

        unsigned int rx_wlan_bytes_high;

        unsigned int rx_protocol_stack_packets_low;

        unsigned int rx_protocol_stack_packets_high;

        unsigned int rx_protocol_stack_bytes_low;

        unsigned int rx_protocol_stack_bytes_high;

        unsigned int _dw_res0[20];

    } vap_data_misc_mib_t;

    typedef struct {

        unsigned int rx_success_mpdu;

        unsigned int rx_success_msdu;

        unsigned int rx_error2_mpdu;

        unsigned int rx_error2_msdu;

        unsigned int rx_error3_mpdu;

        unsigned int rx_error3_msdu;

        unsigned int rx_error4_mpdu;

        unsigned int rx_error4_msdu;

        unsigned int rx_error5_mpdu;

        unsigned int rx_error5_msdu;

        unsigned int rx_error6_mpdu;

        unsigned int rx_error6_msdu;

        unsigned int rx_error7_mpdu;

        unsigned int rx_error7_msdu;

        unsigned int rx_error8_mpdu;

        unsigned int rx_error8_msdu;

        unsigned int rx_error9_mpdu;

        unsigned int rx_error9_msdu;

        unsigned int rx_errora_mpdu;

        unsigned int rx_errora_msdu;

        unsigned int rx_drop_error5;

        unsigned int rx_drop_ro_linklist;

        unsigned int _dw_res0[10];

    } dlrx_data_mib_t;

    typedef struct {

        unsigned int total_ce4_cpu_msg;

        unsigned int total_ce5_cpu_msg;

        unsigned int total_rx_ind_msg;

        unsigned int total_rx_flush_msg;

        unsigned int total_tx_cmp_msg;

        unsigned int total_rx_ind_wlan_msg;

        unsigned int total_rx_flush_wlan_msg;

        unsigned int total_rx_frag_ind_msg;

        unsigned int total_rx_invalid_tid_msg;

        unsigned int _dw_res0[3];

    } dlrx_msg_mib_t;

    typedef struct {

        unsigned int total_chained_mpdu;

        unsigned int total_chained_msdu;

        unsigned int _dw_res0[14];

    } dlrx_misc_mib_t;

    typedef struct {

        unsigned int vld3:1;
        unsigned int peer3:7;
        unsigned int vld2:1;
        unsigned int peer2:7;
        unsigned int vld1:1;
        unsigned int peer1:7;
        unsigned int vld0:1;
        unsigned int peer0:7;

    } dlrx_cfg_peer_id_to_peer_map_t;

    typedef struct {

        unsigned int rsvd0:25;
        unsigned int acc_dis:1;
        unsigned int sec_type:2;
        unsigned int vap:4;

    } dlrx_cfg_peer_to_vap_pn_t;

    typedef struct {

        unsigned int req:1;
        unsigned int rsvd0:24;
        unsigned int peer:7;

    } dlrx_cfg_peer_reset_t;

    typedef struct {

        unsigned int req:1;
        unsigned int rsvd0:24;
        unsigned int peer:7;

    } dlrx_cfg_invalid_tid_t;

    typedef struct {

        unsigned int allreq:1;
        unsigned int vapreq:1;
        unsigned int rsvd0:26;
        unsigned int vap:4;

    } dlrx_cfg_mib_reset_t;

    typedef struct {

        unsigned int vap7:4;
        unsigned int vap6:4;
        unsigned int vap5:4;
        unsigned int vap4:4;
        unsigned int vap3:4;
        unsigned int vap2:4;
        unsigned int vap1:4;
        unsigned int vap0:4;

    } dlrx_cfg_vap2int_map1_t;

    typedef struct {

        unsigned int vap15:4;
        unsigned int vap14:4;
        unsigned int vap13:4;
        unsigned int vap12:4;
        unsigned int vap11:4;
        unsigned int vap10:4;
        unsigned int vap9:4;
        unsigned int vap8:4;

    } dlrx_cfg_vap2int_map2_t;

    typedef struct {

        unsigned int rxpb_ptr;

    } dlrx_rxpb_ptr_ring_t;

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int _res0:3;
        unsigned int byte_off:2;
        unsigned int _res1:7;
        unsigned int data_len:16;

        unsigned int _res2:2;
        unsigned int data_ptr:27;
        unsigned int _res3:2;
        unsigned int data_ptr_rel:1;

    } dlrx_dma_des_t;

    typedef struct {

        unsigned int cfg_badr_dma;

        unsigned int cfg_num_dma;

        unsigned int txdes_index;

        unsigned int rsvd0;

    } dlrx_cfg_ctxt_dma_t;

    typedef struct {

        unsigned int cfg_badr_ce5buf;

        unsigned int cfg_num_ce5buf;

        unsigned int cfg_size_ce5buf;

        unsigned int cfg_size_shift_ce5buf;

        unsigned int cfg_badr_target_ce5_read_index;

        unsigned int cfg_badr_target_ce5_write_index;

        unsigned int local_ce5_read_index;

        unsigned int local_ce5_parsing_index;

        unsigned int ce5_msg_type;

        unsigned int _dw_res0[3];

    } dlrx_cfg_ctxt_ce5buf_t;

    typedef struct {

        unsigned int cfg_badr_ce5des;

        unsigned int cfg_num_ce5des;

        unsigned int msg_len;

        unsigned int _dw_res0;

    } dlrx_cfg_ctxt_ce5des_t;

    typedef struct {

        unsigned int dest_ptr;

        unsigned int meta_data:14;
        unsigned int byte_swap:1;
        unsigned int gather:1;
        unsigned int nbytes:16;

    } dlrx_ce5des_format_t;

    typedef struct {

        unsigned int cfg_badr_cpu_ce5;

        unsigned int cfg_num_cpu_ce5;

        unsigned int cpu_ce5_read_index;

        unsigned int cpu_ce5_write_index;

        unsigned int cpu_ce5_msg_done;

        unsigned int _dw_res0[3];

    } dlrx_cfg_ctxt_cpu_ce5des_t;

    typedef struct {

        unsigned int cfg_badr_rxpb_ptr_ring;

        unsigned int cfg_num_rxpb_ptr_ring;

        unsigned int rxpb_ptr_write_index;

        unsigned int rxpb_ptr_read_index;

        unsigned int _dw_res0[4];

    } dlrx_cfg_ctxt_rxpb_ptr_ring_t;

    typedef struct {

        unsigned int cfg_size_rxpktdes;

        unsigned int cfg_offset_atten;

        unsigned int _dw_res0[2];

    } dlrx_cfg_ctxt_rxpb_t;

    typedef struct {

        unsigned int cfg_badr_ro_linklist;

        unsigned int cfg_num_ro_linklist;

        unsigned int free_num_ro_linklist;

        unsigned int cur_ro_des_ptr;

        unsigned int cur_ro_des_index;

        unsigned int prev_ro_des_index;

        unsigned int ro_des_free_head_index;

        unsigned int ro_des_free_tail_index;

        unsigned int _dw_res0[8];

    } dlrx_cfg_ctxt_ro_linklist_t;

    typedef struct {

        unsigned int cfg_badr_ro_mainlist;

        unsigned int cfg_num_ro_mainlist;

        unsigned int ro_mainlist_ptr;

        unsigned int _dw_res0[5];

    } dlrx_cfg_ctxt_ro_mainlist_t;

    typedef struct {

        unsigned int mpdu_cnt;

        unsigned int mpdu_status;

        unsigned int mpdu_range_index;

        unsigned int mpdu_index;

        unsigned int msdu_mpdu_index;

        unsigned int msdu_index;

        unsigned int peer;

        unsigned int ext_tid;

        unsigned int seqid;

        unsigned int total_seqid;

        unsigned int start_seqid;

        unsigned int vap;

        unsigned int sec_type;

        unsigned int pn_pass;

        unsigned int total_msdu;

        unsigned int check_rv_pending;

        unsigned int ext_ro_mainlist_ptr;

        unsigned int ext_msg_ptr;

        unsigned int peer_vld;

        unsigned int acc_dis;

        unsigned int _res0:26;
        unsigned int inspect:1;
        unsigned int _res1:3;
        unsigned int forward:1;
        unsigned int discard:1;

        unsigned int _dw_res0[27];

    } dlrx_ctxt_msg_t;

    typedef struct {

        unsigned int cfg_badr_rel_msgbuf;

        unsigned int cfg_num_rel_msgbuf;

        unsigned int _dw_res0[2];

    } dlrx_cfg_ctxt_rxpb_ptr_rel_msgbuf_t;

    typedef struct {

        unsigned int dltx_enable;

        unsigned int dlrx_enable;

        unsigned int dlrx_pcie_base;

        unsigned int dlrx_ddr_base;

        unsigned int dlrx_cfg_ctxt_base;

        unsigned int dlrx_cfg_ctxt_max_size;

        unsigned int fw_ver_id;

        unsigned int fw_feature;

        unsigned int debug_print_enable;

        unsigned int dlrx_cfg_unload;

        unsigned int dlrx_qca_hw;

        unsigned int _dw_res0[5];

    } dlrx_cfg_global_t;

    typedef struct {

        unsigned int cfg_peer_handler;

        unsigned int cfg_peer_count;

    } dlrx_cfg_ctxt_peer_handler_t;

#endif

