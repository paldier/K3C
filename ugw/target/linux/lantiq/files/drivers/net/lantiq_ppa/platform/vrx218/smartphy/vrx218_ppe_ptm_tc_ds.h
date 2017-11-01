#ifndef __TC_DS__H__
#define __TC_DS__H__

#define VRX218_US_BONDING_MASTER    0
#define VRX218_DS_BONDING_MASTER    1


struct cfg_std_data_len {
    unsigned int res1                   :14;
    unsigned int byte_off               :2;     //  byte offset in RX DMA channel
    unsigned int data_len               :16;    //  data length for standard size packet buffer
};

struct tx_qos_cfg {
    unsigned int time_tick              :16;    //  number of PP32 cycles per basic time tick
    unsigned int overhd_bytes           :8;     //  number of overhead bytes per packet in rate shaping
    unsigned int eth1_eg_qnum           :4;     //  number of egress QoS queues (< 8);
    unsigned int eth1_burst_chk         :1;     //  always 1, more accurate WFQ
    unsigned int eth1_qss               :1;     //  1: FW QoS, 0: HW QoS
    unsigned int shape_en               :1;     //  1: enable rate shaping, 0: disable
    unsigned int wfq_en                 :1;     //  1: WFQ enabled, 0: strict priority enabled
};

struct eg_bwctrl_cfg {
    unsigned int fdesc_wm               :16;    //  if free descriptors in QoS/Swap channel is less than this watermark, large size packets are discarded
    unsigned int class_len              :16;    //  if packet length is not less than this value, the packet is recognized as large packet
};

struct test_mode {
    unsigned int res1           :30;
    unsigned int mib_clear_mode :1;     //  1: MIB counter is cleared with TPS-TC software reset, 0: MIB counter not cleared
    unsigned int test_mode      :1;     //  1: test mode, 0: normal mode
};

struct rx_bc_cfg {
    unsigned int res1           :14;
    unsigned int local_state    :2;     //  0: local receiver is "Looking", 1: local receiver is "Freewheel Sync False", 2: local receiver is "Synced", 3: local receiver is "Freewheel Sync Truee"
    unsigned int res2           :15;
    unsigned int remote_state   :1;     //  0: remote receiver is "Out-of-Sync", 1: remote receiver is "Synced"
    unsigned int to_false_th    :16;    //  the number of consecutive "Miss Sync" for leaving "Freewheel Sync False" to "Looking" (default 3)
    unsigned int to_looking_th  :16;    //  the number of consecutive "Miss Sync" for leaving "Freewheel Sync True" to "Freewheel Sync False" (default 7)
    /*
     *  firmware use only (total 30 dwords)
     */
    unsigned int rx_cw_rdptr;
    unsigned int cw_cnt;
    unsigned int missed_sync_cnt;
    unsigned int bitmap_last_cw[3];         //  Bitmap of the Last Codeword
    unsigned int bitmap_second_last_cw[3];  //  Bitmap of the Second Last Codeword
    unsigned int bitmap_third_last_cw[3];   //  Bitmap of the Third Last Codeword
    unsigned int looking_cw_cnt;
    unsigned int looking_cw_th;
    unsigned int byte_shift_cnt;
    unsigned int byte_shift_val;
    unsigned int res_word1[14];
};

struct rx_gamma_itf_cfg {
    unsigned int res1           :31;
    unsigned int receive_state  :1;     //  0: "Out-of-Fragment", 1: "In-Fragment"
    unsigned int res2           :16;
    unsigned int rx_min_len     :8;     //  min length of packet, padding if packet length is smaller than this value
    unsigned int rx_pad_en      :1;     //  0:  padding disabled, 1: padding enabled
    unsigned int res3           :2;
    unsigned int rx_eth_fcs_ver_dis :1; //  0: ETH FCS verification is enabled, 1: disabled
    unsigned int rx_rm_eth_fcs      :1; //  0: ETH FCS field is not removed, 1: ETH FCS field is removed
    unsigned int rx_tc_crc_ver_dis  :1; //  0: TC CRC verification enabled, 1: disabled
    unsigned int rx_tc_crc_size     :2; //  0: 0-bit, 1: 16-bit, 2: 32-bit
    unsigned int rx_eth_fcs_result;     //  if the ETH FCS result matches this magic number, then the packet is valid packet
    unsigned int rx_tc_crc_result;      //  if the TC CRC result matches this magic number, then the packet is valid packet
    unsigned int rx_crc_cfg     :16;    //  TC CRC config, please check the description of SAR context data structure in the hardware spec
    unsigned int res4           :16;
    unsigned int rx_eth_fcs_init_value; //  ETH FCS initialization value
    unsigned int rx_tc_crc_init_value;  //  TC CRC initialization value
    unsigned int res_word1;
    unsigned int rx_max_len_sel :1;     //  0: normal, the max length is given by MAX_LEN_NORMAL, 1: fragment, the max length is given by MAX_LEN_FRAG
    unsigned int res5           :2;
    unsigned int rx_edit_num2   :4;     //  number of bytes to be inserted/removed
    unsigned int rx_edit_pos2   :7;     //  first byte position to be edited
    unsigned int rx_edit_type2  :1;     //  0: remove, 1: insert
    unsigned int rx_edit_en2    :1;     //  0: disable insertion or removal of data, 1: enable
    unsigned int res6           :3;
    unsigned int rx_edit_num1   :4;     //  number of bytes to be inserted/removed
    unsigned int rx_edit_pos1   :7;     //  first byte position to be edited
    unsigned int rx_edit_type1  :1;     //  0: remove, 1: insert
    unsigned int rx_edit_en1    :1;     //  0: disable insertion or removal of data, 1: enable
    unsigned int res_word2[2];
    unsigned int rx_inserted_bytes_1l;
    unsigned int rx_inserted_bytes_1h;
    unsigned int rx_inserted_bytes_2l;
    unsigned int rx_inserted_bytes_2h;
    int rx_len_adj;                     //  the packet length adjustment, it is sign integer
    unsigned int res_word3[16];
};

struct tx_bc_cfg {
    unsigned int fill_wm        :16;    //  default 2
    unsigned int uflw_wm        :16;    //  default 2
    /*
     *  firmware use only (total 31 dwords)
     */
    //  Reserved
    unsigned int res0;
    //  FW Internal Use
    unsigned int holding_pages;
    unsigned int ready_pages;
    unsigned int pending_pages;
    unsigned int cw_wrptr;              // TX codeword write pointer for e
    unsigned int res_word[26];
};

struct tx_gamma_itf_cfg {
    unsigned int res_word1;
    unsigned int res1           :8;
    unsigned int tx_len_adj     :4;     //  4 * (not TX_ETH_FCS_GEN_DIS) + TX_TC_CRC_SIZE
    unsigned int tx_crc_off_adj :4;     //  4 + TX_TC_CRC_SIZE
    unsigned int tx_min_len     :8;     //  min length of packet, if length is less than this value, packet is padded
    unsigned int res2           :3;
    unsigned int tx_eth_fcs_gen_dis :1; //  0: ETH FCS generation enabled, 1: disabled
    unsigned int res3           :2;
    unsigned int tx_tc_crc_size :2;     //  0: 0-bit, 1: 16-bit, 2: 32-bit
    unsigned int res4           :24;
    unsigned int queue_mapping  :8;     //  TX queue attached to this Gamma interface
    unsigned int res_word2;
    unsigned int tx_crc_cfg     :16;    //  TC CRC config, please check the description of SAR context data structure in the hardware spec
    unsigned int res5           :16;
    unsigned int tx_eth_fcs_init_value; //  ETH FCS initialization value
    unsigned int tx_tc_crc_init_value;  //  TC CRC initialization value
    unsigned int res_word3[9];
    /*
     *  firmware use only (total 25 dwords)
     */
    //  FW Internal Use
    unsigned int curr_qid;
    unsigned int fill_pkt_state;
    unsigned int post_pkt_state;
    unsigned int curr_pdma_context_ptr;
    unsigned int curr_sar_context_ptr;
    unsigned int des_addr;
    unsigned int des_qid;
    unsigned int rem_data;
    unsigned int rem_crc;
    //  bonding fields
    unsigned int rem_fh_len;
    unsigned int des_dw0;
    unsigned int des_dw1;
    unsigned int des_bp_dw;

    //MIB field
    unsigned int tx_pkt_cnt;
    unsigned int tx_byte_cnt;
    //  Reserved
    unsigned int res_word4;
};

struct gpio_mode {
    unsigned int res1           :3;
    unsigned int gpio_bit_bc1   :5;
    unsigned int res2           :3;
    unsigned int gpio_bit_bc0   :5;

    unsigned int res3           :7;
    unsigned int gpio_bc1_en    :1;

    unsigned int res4           :7;
    unsigned int gpio_bc0_en    :1;
};

struct gpio_wm_cfg {
    unsigned int stop_wm_bc1    :8;
    unsigned int start_wm_bc1   :8;
    unsigned int stop_wm_bc0    :8;
    unsigned int start_wm_bc0   :8;
};

struct wtx_qos_q_desc_cfg {
    unsigned int    threshold           :8;
    unsigned int    length              :8;
    unsigned int    addr                :16;
    unsigned int    rd_ptr              :16;
    unsigned int    wr_ptr              :16;
};

struct wan_rx_mib_table {
    unsigned int    res1[2];
    unsigned int    wrx_dropdes_pdu;
    unsigned int    wrx_total_bytes;
    unsigned int    res2[4];
    //  wrx_total_pdu is implemented with hardware counter (not used by PTM TC)
    //  check register "TC_RX_MIB_CMD"
    //  "HEC_INC" used to increase preemption Gamma interface (wrx_total_pdu)
    //  "AIIDLE_INC" used to increase normal Gamma interface (wrx_total_pdu)
};

struct SFSM_cfg {
    unsigned int    res                 :14;
    unsigned int    rlsync              :1;
    unsigned int    endian              :1;
    unsigned int    idlekeep            :1;
    unsigned int    sen                 :1;
    unsigned int    res1                :6;
    unsigned int    pnum                :8;
};

struct PTM_CW_CTRL {
    unsigned int    state               :1;
    unsigned int    bad                 :1;
    unsigned int    ber                 :9;
    unsigned int    spos                :7;
    unsigned int    ffbn                :7;
    unsigned int    shrt                :1;
    unsigned int    preempt             :1;
    unsigned int    cwer                :2;
    unsigned int    cwid                :3;
};

struct SFSM_dba {
    unsigned int    res                 :17;
    unsigned int    dbase               :15;
};

struct SFSM_cba {
    unsigned int    res                 :15;
    unsigned int    cbase               :17;
};

struct FFSM_dba {
    unsigned int    res                 :17;
    unsigned int    dbase               :15;
};

struct FFSM_cfg  {
    unsigned int    res                 :12;
    unsigned int    rstptr              :1;
    unsigned int    clvpage             :1;
    unsigned int    fidle               :1;
    unsigned int    endian              :1;
    unsigned int    res1                :8;
    unsigned int    pnum                :8;
};





#endif  //  __TC_DS__H__
