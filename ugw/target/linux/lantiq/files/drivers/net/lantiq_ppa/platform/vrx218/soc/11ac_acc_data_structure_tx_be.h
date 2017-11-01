
#ifndef __11AC_ACC_DATA_STRUCTURE_TX_BE_H_
#define __11AC_ACC_DATA_STRUCTURE_TX_BE_H_

    typedef struct {

        unsigned int _res0:24;
        unsigned int int7:1;
        unsigned int int6:1;
        unsigned int int5:1;
        unsigned int int4:1;
        unsigned int int3:1;
        unsigned int int2:1;
        unsigned int int1:1;
        unsigned int int0:1;

    } cfg_dirlink_int_t;

    typedef struct {

        unsigned int size;

    } cfg_size_txpb_t;

    typedef struct {

        unsigned int num;

    } cfg_num_txpb_t;

    typedef struct {

        unsigned int addr;

    } cfg_badr_txpb_t;

    typedef struct {

        unsigned int size;

    } cfg_size_txheader_t;

    typedef struct {

        unsigned int num;

    } cfg_num_ce4des_t;

    typedef struct {

        unsigned int num;

    } cfg_ce4des_low_t;

    typedef struct {

        unsigned int num;

    } cfg_ce4des_full_t;

    typedef struct {

        unsigned int addr;

    } cfg_badr_ce4des_t;

    typedef struct {

        unsigned int index;

    } dma_rxdes_index_t;

    typedef struct {

        unsigned int num;

    } free_num_txpb_t;

    typedef struct {

        unsigned int index;

    } local_ce4_read_index_t;

    typedef struct {

        unsigned int req;

    } local_ce4_read_index_req_t;

    typedef struct {

        unsigned int _res0:15;
        unsigned int fraglen:17;

        unsigned int payloadlen:16;
        unsigned int flags:8;
        unsigned int endpointid:8;

        unsigned int _res1:16;
        unsigned int controlbyte1:8;
        unsigned int controlbyte0:8;

        unsigned int _res2:1;
        unsigned int cksum:2;
        unsigned int mib:1;
        unsigned int postponded:1;
        unsigned int exttid:5;
        unsigned int vdevid:6;
        unsigned int pkttype:3;
        unsigned int pktsubtype:5;
        unsigned int msgtype:8;

        unsigned int id:16;
        unsigned int len:16;

        unsigned int _dw_res0[3];

    } htt_txdes_t;

    typedef struct {

        unsigned int status;

    } free_tx_pb_status_t;

    typedef struct {

        unsigned int status;

    } free_tx_pb_group32_status_t;

    typedef struct {

        unsigned int status;

    } free_tx_pb_group1024_status_t;

    typedef struct {

        unsigned int _res0:8;
        unsigned int num_pb_pointers:8;
        unsigned int _res1:16;

        unsigned int free_pb_ptr[128];

    } tx_buf_release_msg0_t;

    typedef struct {

        unsigned int _res0:8;
        unsigned int num_pb_pointers:8;
        unsigned int _res1:16;

        unsigned int free_pb_ptr[128];

    } tx_buf_release_msg1_t;

    typedef struct {

        unsigned int txpdu;

        unsigned int txbytes;

        unsigned int rx_fwd_pdu;

        unsigned int rx_fwd_bytes;

        unsigned int rx_inspect_pdu;

        unsigned int rx_inspect_bytes;

        unsigned int rx_discard_pdu;

        unsigned int rx_discard_bytes;

        unsigned int txpdu_high; //20140704: new change to use 64 bit statistics , previous defined rx_pn_pdu high byte for txpdu

        unsigned int txbytes_high; //20140704: new change to use 64 bit statistics , previous rx_pn_bytes high byte for txbytes

        unsigned int rx_drop_pdu;

        unsigned int rx_drop_bytes;

        unsigned int rx_rcv_pdu;

        unsigned int rx_rcv_bytes;

        unsigned int _dw_res0[2];

    } mib_table_t;

    typedef struct {

        unsigned int src_buf_ptr;

        unsigned int metadata:14;
        unsigned int bytesw:1;
        unsigned int gather:1;
        unsigned int src_buf_len:16;

    } ce4_src_ring_desc_t;

    typedef struct {

        unsigned int own:1;
        unsigned int c:1;
        unsigned int sop:1;
        unsigned int eop:1;
        unsigned int byte_offset:5;
        unsigned int rx_sideband:4;
        unsigned int _res1:3;
        unsigned int data_len:16;

        unsigned int _res2:2;
        unsigned int dataptr:27;
        unsigned int _res3:3;

    } soc_dma_desc_t;

#endif

