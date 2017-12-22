#ifndef __VRX218_PPE_ATM_DS_H__
#define __VRX218_PPE_ATM_DS_H__

    //ATM FW Data-Structure Definitions -- START
    typedef struct {
        /*  0h  */
        unsigned int    new_vlan            :16;
        unsigned int    res1                :3;
        unsigned int    vlan_ins            :1;
        unsigned int    mpoa_type           :2; //  0: EoA without FCS, 1: EoA with FCS, 2: PPPoA, 3:IPoA
        unsigned int    ip_ver              :1; //  0: IPv4, 1: IPv6
        unsigned int    mpoa_mode           :1; //  0: VCmux, 1: LLC
        unsigned int    res2                :8;
        /*  1h  */
        unsigned int    oversize            :16;
        unsigned int    undersize           :16;
        /*  2h  */
        unsigned int    res3                :16;
        unsigned int    mfs                 :16;
        /*  3h  */
        unsigned int    uumask              :8;
        unsigned int    cpimask             :8;
        unsigned int    uuexp               :8;
        unsigned int    cpiexp              :8;
    } wrx_queue_config_t;

    typedef struct {
        unsigned int    res1                :27;
        unsigned int    qid                 :4;
        unsigned int    qsben               :1;
    } wtx_port_config_t;

    typedef struct {
        /*  0h  */
        unsigned int    res0                :16;
        unsigned int    same_vc_qmap        :16; //  e.g., TX Q0, Q2, Q4 is VCID1, config TX Q0, value is binary 0000000000010100. Set all queue in this VC with 1 except this queue.
        /*  1h  */
        unsigned int    uu                  :8;
        unsigned int    cpi                 :8;
        unsigned int    res1                :9;
        unsigned int    sbid                :1;
        unsigned int    qsb_vcid            :4; //  Which QSB queue (VCID) does this TX queue map to.
        unsigned int    mpoa_mode           :1; //  0: VCmux, 1: LLC
        unsigned int    qsben               :1; //  reserved in A5
        /*  2h  */
        unsigned int    atm_header          :32;
    } wtx_queue_config_t;
    //ATM FW Data-Structure Definitions -- END

#endif	// __VRX218_PPE_ATM_DS_H__

