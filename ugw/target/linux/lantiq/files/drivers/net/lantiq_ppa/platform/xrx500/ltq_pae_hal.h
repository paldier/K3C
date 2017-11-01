#ifndef __LTQ_PAE_HAL_H__201407_25_1520__
#define __LTQ_PAE_HAL_H__201407_25_1520__

/******************************************************************************
**
** FILE NAME    : ltq_pae_hal.h
** PROJECT      : GRX500
** MODULES      : PPA PAE HAL 
**
** DATE         : 27 MAY 2014
** AUTHOR       : Kamal Eradath
** DESCRIPTION  : PAE Hardware Abstraction Layer
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 27 MAY 2014  Kamal Eradath   Initiate Version
*******************************************************************************/
/*
 * ####################################
 *              Definition
 * ####################################
 */

/*
 *  Compilation Switch
 */

/*
 * PAE MAX entries
 */ 

#define MAX_PAE_PORTS	16
#define MAX_SUBIF_IDS	16

#define MAX_BRIDGING_ENTRIES                    512 
#define MAX_ROUTING_ENTRIES                    	4096 
#define MAX_WAN_MC_ENTRIES                      128

#define MAX_PPPOE_ENTRIES                       16 
#define MAX_MTU_ENTRIES                         8
#define MAX_MAC_ENTRIES                         512
#define MAX_VLAN_ENTRIES                  	256
#define MAX_IP_ENTRIES				2048

#define MAX_PCE_ENTRIES              		512
#define MAX_RTP_ENTRIES               		16
#define	MAX_LRO_ENTRIES				8

#define PPA_DEST_LIST_CPU0	0x01    //0000000000000001
#define PPA_DEST_LIST_ETH0	0x02    //0000000000000010
#define PPA_DEST_LIST_ETH0_1	0x04	//0000000000000100
#define PPA_DEST_LIST_ETH0_2	0x08	//0000000000001000
#define PPA_DEST_LIST_ETH0_3	0x010	//0000000000010000
#define PPA_DEST_LIST_ETH0_4	0x020	//0000000000100000
#define PPA_DEST_LIST_ATM	0x2000	//0010000000000000
#define PPA_DEST_LIST_ETH1	0x8000  //1000000000000000

#define  IP_PROTO_UDP                           0x11
#define  IP_PROTO_TCP                           0x06 
#define  IP_PROTO_ESP                           50 

#define PCE_CPU_FLOW_RULES_START    0	    //32
#define PCE_FILTER_RULES_START	    32	    //32
#define PCE_LRO_RULES_START	    48	    //16
#define PCE_RT_RULE_START	    80      //16
#define PCE_TUN_DECAP_RULE_START    96	    //16
#define PCE_BRIDGING_FID_RULE_START 112	    //16
#define PCE_QOS_CLASS_RULE_START    128	    //128~
#define PCE_RULE_MAX		    511

#define GSWL_PCE_MAX 64
#define GSWR_PCE_MAX 256    // Max number of PCE rules supported is 512 but for optimum speed it is recomended to use rules less than 256

#define RT_EXTID_TCP	    0
#define RT_EXTID_UDP	    100
#define RT_EXTID_IPSEC	    0
#define RMON_TCP_CNTR	    1
#define RMON_UDP_CNTR	    2
#define RMON_MCAST_CNTR	    3
#define RMON_6RD_CNTR	    4
#define RMON_DSLITE_CNTR    5
#define RMON_L2TP_CNTR	    6
#define RMON_CAPWAP_CNTR    7
#define RMON_GRE_CNTR         8
#define RMON_IPSEC_CNTR         9
#define RMON_SMALL_SIZE_CNTR	21	
#define RMON_CPU_ING_CNTR	22
#define RMON_CPU_EG_CNTR	23

#define PCE_PROTO_ESP 		50	
#define PCE_PARSER_MSB_LRO_EXCEP 			0x8000
#define PCE_PARSER_MSB_L2TP_DATA			0x4000
#define PCE_PARSER_MSB_UDP_HDR_AFTR_SEC_IP_HDR		0x2000
#define PCE_PARSER_MSB_INNR_IPV6_WITH_EXTN_HDR		0x1000
#define PCE_PARSER_MSB_EAPOL				0x0800
#define PCE_PARSER_MSB_IP_FRAGMT			0x0400
#define PCE_PARSER_MSB_TCP_ACK				0x0200
#define PCE_PARSER_MSB_OUTR_IPV6_WITH_EXTN_HDR		0x0100
#define PCE_PARSER_MSB_IPV4_OPTNS			0x0080
#define PCE_PARSER_MSB_IGMP				0x0040
#define PCE_PARSER_MSB_UDP_HDR_AFTR_FST_IP_HDR		0x0020
#define PCE_PARSER_MSB_TCP				0x0010
#define PCE_PARSER_MSB_RT_EXCEP				0x0008
#define PCE_PARSER_MSB_INNR_IPV6			0x0004
#define PCE_PARSER_MSB_INNR_IPV4			0x0002
#define PCE_PARSER_MSB_OUTR_IPV6			0x0001

#define PCE_PARSER_LSB_OUTR_IPV4			0x8000
#define PCE_PARSER_LSB_PPPOE				0x4000
#define PCE_PARSER_LSB_SNAP_ENCAP			0x2000
#define PCE_PARSER_LSB_4TH_VLAN				0x1000
#define PCE_PARSER_LSB_3RD_VLAN				0x0800
#define PCE_PARSER_LSB_2ND_VLAN				0x0400
#define PCE_PARSER_LSB_1ST_VLAN				0x0200
#define PCE_PARSER_LSB_SPL_TAG				0x0100
#define PCE_PARSER_LSB_RES1				0x0080
#define PCE_PARSER_LSB_RES2				0x0040
#define PCE_PARSER_LSB_RES3				0x0020
#define PCE_PARSER_LSB_LEN_ENCAP			0x0010
#define PCE_PARSER_LSB_GRE				0x0008
#define PCE_PARSER_LSB_CAPWAP				0x0004
#define PCE_PARSER_LSB_PARSE_ERROR			0x0002
#define PCE_PARSER_LSB_WOL				0x0001

#define PCE_IPV4_MCAST_MASK				0xFF7F
#define PCE_IPV6_MCAST_MASK				0x3FFFFFFF

//
//classification API: static configuration if categories and subcategories
//
#define GSWR_CAT_FILTER_START	16
#define GSWR_CAT_FILTER_MAX	GSWR_PCE_MAX / 8 // max 32 when GSWR_PCE_MAX is 256
#define GSWL_CAT_FILTER_START	4
#define GSWL_CAT_FILTER_MAX	GSWL_PCE_MAX / 2 // max 32 

#define GSWR_CAT_VLAN_START	32
#define GSWR_CAT_VLAN_MAX	GSWR_PCE_MAX / 16 // max 16 when GSWR_PCE_MAX is 256
#define GSWL_CAT_VLAN_START	8
#define GSWL_CAT_VLAN_MAX	GSWL_PCE_MAX / 4  // max 16

#define GSWR_CAT_FWD_START	80
#define GSWR_CAT_FWD_MAX	GSWR_PCE_MAX / 8  // max 32 when GSWR_PCE_MAX is 256
#define GSWL_CAT_FWD_START	16
#define GSWL_CAT_FWD_MAX	GSWL_PCE_MAX / 2  // max 32

#define GSWR_CAT_USQOS_START	112
#define GSWR_CAT_USQOS_MAX	GSWR_PCE_MAX / 4 // max 64 when GSWR_PCE_MAX is 256
#define GSWL_CAT_USQOS_START	16
#define GSWL_CAT_USQOS_MAX	GSWL_PCE_MAX / 2 // max 32

#define GSWR_CAT_DSQOS_START	112
#define GSWR_CAT_DSQOS_MAX	GSWR_PCE_MAX / 4 // max 64 when GSWR_PCE_MAX is 256
#define GSWL_CAT_DSQOS_START	16
#define GSWL_CAT_DSQOS_MAX	GSWL_PCE_MAX / 2 // max 32

#define GSWR_CAT_MGMT_START	48
#define GSWR_CAT_MGMT_MAX	GSWR_PCE_MAX / 8 // max 32 when GSWR_PCE_MAX is 256
#define GSWL_CAT_MGMT_START	32
#define GSWL_CAT_MGMT_MAX       GSWL_PCE_MAX / 2 // max 32

#define GSWR_CAT_LRO_START	32
#define GSWR_CAT_LRO_MAX	32 // 16 LRO sessions MAX 2 pce rule per session so 32

#define GSWR_CAT_TUN_START	64
#define GSWR_CAT_TUN_MAX	16 // MAX 16 tunnels supported in system


/*
 * ####################################
 *              Data Type
 * ####################################
 */
// classification API datastructures
typedef struct {
	uint16_t uid;
	uint16_t index;
} cat_idx_map_t;

typedef struct pce_catmap {
	ppa_class_category_t cat_id;
	int16_t cat_max;
	int16_t cat_used;
	int16_t cat_start_idx;
	int16_t cat_last_ordr;
	cat_idx_map_t *cat_idx_vect;
} pce_cat_map_t;

typedef struct pce_sub_catmap {
	ppa_class_category_t cat_id;
	ppa_class_sub_category_t subcat_id;
	uint16_t subcat_max;
	uint16_t subcat_used;
} pce_subcat_map_t;

typedef struct cat_odr_vect {
	ppa_class_category_t cat_id;
	ppa_class_sub_category_t subcat_id;
	uint8_t cat_ordr;
	uint8_t usage_flg;
} cat_ordr_vect_t;

struct pce_cat_conf {
    int16_t cat_max;
    int16_t cat_start_idx;
};

struct switch_dev_class {
	uint16_t tot_max;
	uint16_t tot_used;
	
	pce_cat_map_t cat_map[CAT_MAX]; // array of all the categories 
	pce_subcat_map_t subcat_map[SUBCAT_MAX]; // array of all sub categories 
	uint32_t *pce_tbl_bitmap;   // bitmap for bookkeeping.
	cat_ordr_vect_t *cat_ordr_vect;
};
// classification API datastructures ends

/*
 *  Host-PPE Communication Data Structure
 */
#if defined(__BIG_ENDIAN)
  struct proc_entry_cfg{
    char                    *parent_dir;
    char                    *name;
    unsigned int            is_dir;
    int (*proc_r_fn)(char*, char **, off_t , int , int*, void*);
    int (*proc_w_fn)(struct file*, const char*, unsigned long, void*);
    int                     is_enable;
    struct proc_dir_entry   *proc_dir;
  };
#endif
#endif /*__LTQ_PAE_HAL_H__201407_25_1520__*/

