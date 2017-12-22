
#ifndef __SMARTPHY_DS_BE_H_
#define __SMARTPHY_DS_BE_H_

    typedef struct {

        unsigned int ctxt_ptr:16;
        unsigned int transfer_type:4;
        unsigned int _res0:7;
        unsigned int rie:1;
        unsigned int lie:1;
        unsigned int llp:1;
        unsigned int tcb:1;
        unsigned int cb:1;

        unsigned int transfer_size;

        unsigned int sar_low;

        unsigned int sar_high;

        unsigned int dar_low;

        unsigned int dar_high;

    } edma_lle_t;

    typedef struct {

        unsigned int srcq_ctxt_ptr:16;
        unsigned int src_des_ptr:16;

        unsigned int dstq_ctxt_ptr:16;
        unsigned int dst_des_ptr:16;

        unsigned int src_des_bp_ptr:16;
        unsigned int dst_des_bp_ptr:16;

        unsigned int ctxt_ptr:16;
        unsigned int transfer_type:4;
        unsigned int _res0:4;
        unsigned int des_sync_size:8;

        unsigned int _dw_res0[2];

    } edma_lle_ext_t;

    typedef struct {

        unsigned int sync_type:1;
        unsigned int us_des_polling_needed:1;
        unsigned int running_cnt:4;
        unsigned int polling_intv:4;
        unsigned int max_polling_intv:4;
        unsigned int _res0:1;
        unsigned int soc_des_own_val:1;
        unsigned int desq_cfg_ctxt:16;

        unsigned int sync_rd_status:3;
        unsigned int sync_rd_size:13;
        unsigned int sync_rd_idx:16;

        unsigned int rd_cmd_sar;

        unsigned int rd_cmd_dar;

        unsigned int sync_wr_status:3;
        unsigned int sync_wr_size:13;
        unsigned int sync_wr_idx:16;

        unsigned int wr_cmd_sar;

        unsigned int wr_cmd_dar;

        unsigned int ext_desc_base_addr;

        unsigned int ext_bp_des_base_addr;

        unsigned int _dw_res0;

        unsigned int cdma_dst_des_dw0;

        unsigned int cdma_dst_des_dw1;

        unsigned int sync_rd_cmd_cnt;

        unsigned int sync_wr_cmd_cnt;

        unsigned int sync_rd_cnt;

        unsigned int sync_wr_cnt;

        unsigned int rd_des_buf[16];

    } des_sync_cfg_ctxt_t;

    typedef struct {

        unsigned int des_idx:8;
        unsigned int _res0:3;
        unsigned int dir:1;
        unsigned int _res1:2;
        unsigned int state:1;
        unsigned int sync_type:1;
        unsigned int desq_cfg_ctxt:16;

        unsigned int soc_sync_addr;

        unsigned int enq_pkt_cnt;

        unsigned int deq_pkt_cnt;

        unsigned int cdma_tx_des_dw0;

        unsigned int cdma_tx_des_dw1;

        unsigned int cdma_rx_des_dw0;

        unsigned int cdma_rx_des_dw1;

    } bond_des_sync_cfg_ctxt_t;

    typedef struct {

        unsigned int edma_ch_type:1;
        unsigned int edma_pcs:1;
        unsigned int edma_lle_num:6;
        unsigned int edma_lle_sb_size:8;
        unsigned int edma_lle_sb_base:16;

        unsigned int edma_lle_ext_sb_base:16;
        unsigned int fw_pp_idx:8;
        unsigned int fw_wr_idx:8;

        unsigned int edma_lle_fpi_base;

        unsigned int edma_rd_idx:8;
        unsigned int fw_pp_cnt:8;
        unsigned int edma_rd_cnt:8;
        unsigned int fw_wr_cnt:8;

        unsigned int edma_extra_db_cnt:24;
        unsigned int edma_zero_complete_cnt:4;
        unsigned int _res0:2;
        unsigned int edma_ch_status:2;

        unsigned int edma_lle_pending_cnt;

        unsigned int edma_lle_pending_wr_ptr;

        unsigned int edma_lle_pending_wr_dw0;

    } edma_ch_ctxt_t;

    typedef struct {

        unsigned int srcq_ctxt_ptr:16;
        unsigned int dstq_ctxt_ptr:16;

    } edma_copy_ch_cfg_t;

    typedef struct {

        unsigned int _res0:29;
        unsigned int ch_id:1;
        unsigned int state:2;

        unsigned int _dw_res0;

        unsigned int wch_idx_write_pdma_cmd0;

        unsigned int wch_idx_write_pdma_cmd1;

        unsigned int wch_ll_ptr_read_pdma_cmd0;

        unsigned int wch_ll_ptr_read_pdma_cmd1;

        unsigned int rch_idx_write_pdma_cmd0;

        unsigned int rch_idx_write_pdma_cmd1;

        unsigned int rch_ll_ptr_read_pdma_cmd0;

        unsigned int rch_ll_ptr_read_pdma_cmd1;

        unsigned int pdma_bc3_rd_cmd_issue_cnt;

        unsigned int pdma_bc3_rd_cmd_finish_cnt;

        unsigned int _dw_res1[4];

    } edma_rd_cnt_sm_t;

    typedef struct {

        unsigned int pp32_core_id:1;
        unsigned int us_bonding_master:1;
        unsigned int us_segment_en:1;
        unsigned int us_buf_release_en:1;
        unsigned int profiling_en:1;
        unsigned int event_monitor_en:1;
        unsigned int _res0:2;
        unsigned int ds_bonding_master:1;
        unsigned int ds_pkt_dispatch_en:1;
        unsigned int ds_pkt_reconstruct_en:1;
        unsigned int ds_pkt_flush_en:1;
        unsigned int _res1:2;
        unsigned int us_bonding_des_sync:1;
        unsigned int ds_bonding_des_sync:1;
        unsigned int tc_us_en:1;
        unsigned int tc_ds_en:1;
        unsigned int _res2:5;
        unsigned int des_sync_en:1;
        unsigned int edma_write_cnt_update_en:1;
        unsigned int edma_read_cnt_update_en:1;
        unsigned int edma_write_lle_gen_en:1;
        unsigned int edma_read_lle_gen_en:1;
        unsigned int edma_post_proc_en:1;
        unsigned int qos_wfq_shaping_en:1;
        unsigned int qos_dispatch_en:1;
        unsigned int qos_replenish_en:1;

    } task_cfg_t;

    typedef struct {

        unsigned int enter_time;

        unsigned int total_call_num;

        unsigned int idle_call_num;

        unsigned int working_call_num;

        unsigned int idle_call_time_hi;

        unsigned int idle_call_time_lo;

        unsigned int working_call_time_hi;

        unsigned int working_call_time_lo;

    } module_perf_statistics_t;

    typedef struct {

        unsigned int _res0:12;
        unsigned int pp32_1_main_loop:1;
        unsigned int pp32_0_main_loop:1;
        unsigned int _res1:1;
        unsigned int ds_bonding_flush:1;
        unsigned int ds_bonding_pkt_reconst:1;
        unsigned int ds_bonding_pkt_dispatch:1;
        unsigned int ds_bonding_link_state_check:1;
        unsigned int us_bonding_buf_release:1;
        unsigned int us_bonding_segment:1;
        unsigned int tc_update_link_state:1;
        unsigned int tc_rx_fifo_proc:1;
        unsigned int tc_rx_main:1;
        unsigned int tc_tx_main:1;
        unsigned int edma_rd_cnt_update:1;
        unsigned int edma_rwch_pp:1;
        unsigned int edma_wch_lle_prod:1;
        unsigned int edma_rch_lle_prod:1;
        unsigned int us_wfq_shaping:1;
        unsigned int us_qos_dispatch:1;
        unsigned int des_sync:1;

    } module_index_t;

    typedef struct {

        unsigned int srcq_ctxt_ptr:16;
        unsigned int dstq_ctxt_ptr:16;

    } cdma_copy_ch_cfg_t;

    typedef struct {

        unsigned int _res0:31;
        unsigned int grx500:1;

    } soc_family_t;

#endif

