#ifndef __LTQ_MPE_API_H__201308_05_1913__
#define __LTQ_MPE_API_H__201308_05_1913__

/*******************************************************************************
 **
 ** FILE NAME    : ltq_mpe_api.h
 ** PROJECT      : MPE FW
 ** MODULES      : MPE (Routing/Bridging Acceleration )
 **
 ** DATE         : 5 Aug 2013
 ** AUTHOR       : Shao Guohua
 ** DESCRIPTION  : MPE Acceration Header File
 ** COPYRIGHT    :              Copyright (c) 2009
 **                          Lantiq Deutschland GmbH
 **                   Am Campeon 3; 85579 Neubiberg, Germany
 **
 **   For licensing information, see the file 'LICENSE' in the root folder of
 **   this software module.
 **
 ** HISTORY
 ** $Date        $Author                $Comment
 ** 5 Aug 2013   Shao Guohua            Initiate Version
 *******************************************************************************/

/************************Below is Common Macro Definition *************************************/
/** \addtogroup  MPE_HEADER_MACRO */
/*@{*/
/**  \brief MPE Common Macro Definition    */
#define UNUSED(x) ((void)(x))

#define MAX_TM_NUM                    2      /*!< Maximum TM threads(TC) supported in MPE FW */
#define MAX_WORKER_NUM                6      /*!< Maximum works thread supported in MPE FW */
#define MAX_DL_NUM                	  2      /*!< Maximum dl thread supported in MPE FW */
#define MAX_MPE_TC_NUM     ( MAX_TM_NUM + MAX_WORKER_NUM + MAX_DL_NUM)     /*!< Maximum Threads supported in MPE FW */
#define MAX_ITF_PER_SESSION           8      /*!< Maximum Interface mib counter supported in one accelerated session */
#define MAX_CMP_TABLE                 8      /*!< Maximum compare table supported in MPE */
#define MAX_PMAC_PORT                 16     /*!< Maximum PMAC port number */
#define MAX_MC_VAP_PORT               8      /*!< Maximum PMAC port number which will support VAP for multicast (port 7-14) */
#define MAX_FW_SESSION_NUM            32767  /*!< Maximum FW acceleration sessions supported per table, ie, 32K-1 */
#define MAX_HW_SESSION_NUM            4096   /*!< Maximum HW acceleration sessions supported*/
#define MAX_VERSION_DESC_LEN          20     /*!< Maximum FW version description string length*/
#define MAX_MIB_ITF_NUM               128    /*!< Maximum MIB interfaces supported */
#define MAX_DISPATCH_BUF_NUM          64     /*!< Maximum DMA Descriptor number supported in Dispatch queue */
#define MAX_TLB_NUM                   32     /*!< Maximum TLB entry number supported */
#define MAX_DUMP_BYTE_LEN				  256    /*!< Maximum number of bytes when dumping a packet supported currently */
#define MAX_DMA_DESC_DUP					24		/*!< Maximum number of packet duplication in multicast */

#define TLB_PAGE_SIZE                 ( 16 * 1024 ) /*!< Default TLB page size 16K used in MPE FW*/
#define TC_SP_BUFFER_SIZE             ( 2 * 1024 ) /*!< Maximum Buffer needed for stack only in bytes*/

#define MAX_MC_NUM                    128  /*!< Maximum multicast session supported. Must match with HW capability */
#define MAX_VAP_PER_PORT              16   /*!< Maximum VAP supported per PMAC port*/
#define MAX_WK_MSG_CNT				  100  /*!< Maximum worker special message loop limit */

#define AC_RADIO_NUMBER               1   /*!< Number of Radios for Directlink */
#define MAX_EP2RADIO_ENTRY        16	/*!< Maximum number of EP to Radio ID mapping entries*/

#define IPSEC_TUN_MAX 			16	/*!< MAX IPSEC Tunnel number supported. Limitation is in DMA Descriptor */
#define EIP97_CD_SIZE  			6	/*!< CD(Command Descriptor) size: 6 DWORDS */
#define EIP97_ACD_MAX_SIZE  200	/*!< MAX ACD (Additional Command Descriptor) size in bytes */
#define EIP97_CTX_MAX_SIZE  200	/*!< MAX CTX (Additional Context Record) size in bytes */

/**
  \brief Supported TC type
 */
enum TC_TYPE {
	TYPE_WORKER = 0, /*!< Thread type: Worker */
	TYPE_TM,         /*!< Thread type: TM */
	TYPE_DIRECTLINK  /*!< Thread type: DirectLink */
};

/**
  \brief Supported TC state
 */
enum TC_STATE {
	STATE_INACTIVE = 0, /*!< TC  state: Inactive */
	STATE_IN_PROGRESS,  /*!< TC  state: In Progress */
	STATE_TERMINATED,	/*!< TC  state: Terminated */
	STATE_PAUSE,	/*!< TC  state: Pause */
	STATE_RESUME,       /*!< TC  state: Resume */
};

#define MAX_SEARCH_ITRN                15    /*!< the Max number of Iterations to be done to find a valid match for a Search Command */
#define GENCONF_OFFSET_POS             8     /*!< offset for genconf in the MPE FW BIN file. It is mainly for chiptest only */
#define TLB_IFNO_OFFSET_POS            12     /*!< offset for TLB INFO in the MPE FW BIN file */

#define VPE_NUM                       4     /*!<VPE Num supported */

/**
  \brief Supported Feature List0
 */
enum FEATURE_LIST0 {
	FEATURE_IPV4 = 0x1,	/*!<Support IPV4 Acceleration */
	FEATURE_IPV6 = 0x2,	/*!<Support IPV6 Acceleration */
	FEATURE_MC = 0x4,		/*!<Support Multicast Acceleration */
	FEATURE_DL = 0x8,		/*!<Support Directlink Acceleration */
	FEATURE_TUNNEL_6RD = 0x100,  	/*!<Support 6rd Acceleration */
	FEATURE_TUNNEL_DSLITE = 0x200,/*!<Support dslite Acceleration */
	FEATURE_TUNNEL_L2TP = 0x400,	/*!<Support dslite Acceleration */
	FEATURE_TUNNEL_GRE = 0x0800,	/*!<Support GRE Acceleration (IP/Ethernet over GRE*/
	FEATURE_TUNNEL_ESP = 0x1000	/*!<Support IPSec in tunnel mode */
};
/**
  \brief Supported Feature List1
 */
enum FEATURE_LIST1 {
	FEATURE_DUMMY1 = 0,  /*!< Feature list dummy 1 */
};
/**
  \brief Supported Feature List2
 */
enum FEATURE_LIST2 {
	FEATURE_DUMMY2 = 0, /*!< Feature list dummy 2 */
};
/**
  \brief Supported Feature List3
 */
enum FEATURE_LIST3 {
	FEATURE_DUMMY3 = 0,  /*!< Feature list dummy 3 */
};

/**
  \brief TC Current state
 */

enum TC_CUR_STATE {
	UNKNOWN = 0,	/*!< Unknown TC state */
	TM_YD_WAIT,	/*!< TM Yield Wait */
	TM_YD_WKUP,	/*!<  TM Yield wake up */
	WK_YD_WAIT,	/*!<  Worker Yield Wait */
	WK_YD_WKUP,	/*!<  Worker Yield Wake up */
	DL_YD_WAIT,	/*!<  Directlink Yield Wait */
	DL_YD_WKUP,	/*!<  Directlink Yield Wake up */
	MCPY_YD_WAIT,	/*!<  Mem copy Yield Wait */
	MCPY_YD_WKUP,	/*!<  Mem copy Yield Wake up */
	EIP97_YD_WAIT,	/*!<  Eip97 Yield Wait */
	EIP97_YD_WKUP,	/*!<  Eip97 Yield Wake up */
	MPECTRL_ENQ_YD_WAIT,	/*!<  MPE controller Enqueue Yield Wait */
	MPECTRL_ENQ_YD_WKUP,	/*!<  MPE controller Enqueue Yield Wake Up */
	MPECTRL_DEQ_YD_WAIT,	/*!<  MPE controller Dequeue Yield Wait */
	MPECTRL_DEQ_YD_WKUP,	/*!<  MPE controller Dequeue Yield Wake Up */
	CBM_ENQ_YD_WAIT,	/*!<  CBM controller Enqueue Yield Wait */
	CBM_ENQ_YD_WKUP,	/*!<  CBM controller Enqueue Yield Wake Up */
	CBM_DEQ_YD_WAIT,	/*!<  CBM controller Dequeue Yield Wait */
	CBM_DEQ_YD_WKUP,	/*!<  CBM controller Dequeue Yield Wake Up */
	SEM_DISP_Q_WAIT,	/*!<  Semaphore Dispatch Queue Wait */
	SEM_FREELIST_WAIT,	/*!<  Semaphore free list Wait */
	SEM_CBMALLOC_WAIT,	/*!<  Semaphore CBM alloc Wait */
	SEM_DISP_Q_CNT_WAIT,	/*!<  Semaphore Dispatch Queue count Wait */
	SEM_DISP_Q_WKUP,	/*!<  Semaphore Dispatch Queue Wake Up */
	SEM_FREELIST_WKUP,	/*!<  Semaphore free list Wake Up */
	SEM_CBMALLOC_WKUP,	/*!<  Semaphore CBM alloc Wake Up */
	SEM_DISP_Q_CNT_WKUP,	/*!<  Semaphore Dispatch Queue count Wake Up */
	SEM_DL_FREELIST_WAIT,	/*!<  Semaphore Directlink freelist Wait */
	SEM_DL_FREELIST_WKUP,	/*!<  Semaphore Directlink freelist Wake Up */
	SEM_DL_DISP_Q_CNT_WAIT,	/*!<  Semaphore Directlink Queue count Wait */
	SEM_DL_DISP_Q_CNT_WKUP,	/*!<  Semaphore Directlink Queue count Wake Up */
	PRIVDATA_INIT_START,		/*!<  Private Data Init start */
	PRIVDATA_INIT_COMPLETED,	/*!<  Private Data Init Completed */
};

/**
  \brief FW debug flag
 */
enum FW_DBG_FLAG {
// defined in MPE FW: start.S
	FW_DBG_FLAG_ENTER_STARTS = 0,	/*!<  Firmware debug flag enter starts */
	FW_DBG_FLAG_EVASEG_CFG_DONE,	/*!<  Firmware debug flag EVA segment cfg done */
	FW_DBG_FLAG_TLB_CFG_DONE,	/*!<  Firmware debug flag TLB cfg done */
	FW_DBG_FLAG_LEAVE_STARTS,   /*!<  Firmware debug flag leave starts */
// in tm_init.c
	FW_DBG_FLAG_CHECK_EVA_DONE,	/*!<  Firmware debug flag check EVA done */
	FW_DBG_FLAG_TM_INIT_FREELIST,	/*!<  Firmware debug flag TM init freelist */
	FW_DBG_FLAG_VMB_STARTED_WK,	/*!<  Firmware debug flag VMB started worker */
	FW_DBG_FLAG_VMB_IPI_ACKED,	/*!<  Firmware debug flag VMB IPI acked */
	FW_DBG_FLAG_DL_INIT_FREELIST	/*!<  Firmware debug flag Directlink init freelist */
};

/**
  \brief FW profiling macro
 */

enum PROFILING_MACRO {
	PROFILE_START = 1,		/*!<  Firmware profile starts */
	PROFILE_STOP  = 2,		/*!<  Firmware profile stop */
	PROFILE_SHOW  = 3,		/*!<  Firmware profile results show */
};

typedef unsigned long long              uint64_t;  /*!< typedef for uint64_t */
typedef unsigned int                    uint32_t;  /*!< typedef for uint32_t */
typedef unsigned short                  uint16_t;  /*!< typedef for uint16_t */
typedef unsigned char                   uint8_t;   /*!< typedef for uint8_t */

/* For Bin Endian case, Linux is using macro CONFIG_CPU_BIG_ENDIAN,
 ** But for Chiptest and MPE FW, they are using CONFIG_BIG_ENDIAN.
 ** So add below workaround to be compatible with Chiptest and Linux two cases.
 */
#if !defined(CONFIG_BIG_ENDIAN) && !defined(CONFIG_LITTLE_ENDIAN)
#ifdef CONFIG_CPU_BIG_ENDIAN
#define CONFIG_BIG_ENDIAN	/*!< Big Endian configuration*/
#else
#define CONFIG_LITTLE_ENDIAN	/*!< Little Endian configuration*/
#endif
#endif

/**
  \brief Tunnel type
 */
enum TUNL_TYPE {
	TUNL_NULL = 0,			 /*!< Not Tunnel  */
	TUNL_6RD,      		/*!< 6rd Tunnel  */
	TUNL_DSLITE,  			/*!< DSLITE Tunnel  */
	TUNL_CAPWAP,   		/*!< CAPWAP Tunnel  */
	TUNL_L2TP,     		/*!< L2TP Tunnel  */
	TUNL_EOGRE,    		/*!< Ethernet Over GRE Tunnel  */
	TUNL_IPOGRE,   		/*!< IP Over GRE Tunnel  */
	TUNL_ESP,				/*!< IPSEC Tunnel ESP */
	TUNL_ESP_NATT,			/*!< IPSEC Tunnel ESP with NAT-T */
	TUNL_MAX       		/*!< Not Valid Tunnel type  */
};

/**
  \brief Outer IP DSCP mode
 */
enum OUTER_DSCP_MODE {
	NO_MARKING = 0,    /*!< No marking, FW has no action  */
	USE_INNER_DSCP,    /*!< Use DSCP value from inner IP */
	USE_ACTION_DSCP,   /*!< Use new inner DSCP value from session_action  */
	RESV,              /*!< Reserved  */
};

/*@}*/ /* MPE_HEADER_MACRO */

/*Struct declarion for compilation dependence */
struct session_action;
struct session_mib;

/** \addtogroup  VMB_OTHERS */
/*@{*/
/**  \brief MPE Other Structures  */

/*!
  \brief This is the structure for PMAC Port TYPE
 */
enum PMAC_PORT_TYPE {
	PMAC_DIRECTPATH_DEV,  /*!< Type: DIRECTPATH */
	PMAC_PORT_ETH = 1  /*!< Type: Ethernte */

};

/*!
  \brief This is the structure for PMAC Port Info
 */
struct port_info {
	uint32_t port_type   : 3; /*!< Ethernet/DIRECTPATH/  */
	uint32_t pmac_len_out: 4; /*!< PMAC header length to enqueue back to CBM, Possible value is 0 or 8 bytes. \n
                                0-no PMAC needed            \n
                                8-with standard PMAC header \n
                               */
	uint32_t pmac_len_in : 4; /*!< PMAC header length after dequeue from CBM. Possible value is 0 or 8 bytes. But currently only support 8 bytes*/
	uint32_t reserved    : 21; /*!< Reserve for future*/
};

/*!
  \brief This is the structure for PMAC SubInterface
 */
struct port_subif {
	uint32_t len: 8;  /*!< PMAC header length of this egress port */
	uint32_t port: 8; /*!< egress port id i.e DMA descriptor EP*/
	uint32_t vap: 16; /*!< vap info: [12] MC_flag [11:8] VAP [7] Grp_flag [6:0] VAP_index */
};

/*!
  \brief This is TLB entry elements required.
 */
struct tlb_entry {
	uint32_t pagemask;  /*!< TLB pagemask value*/
	uint32_t entryhi;   /*!< TLB entryhi value */
	uint32_t entrylo0;  /*!< TLB entrylo0 value */
	uint32_t entrylo1;  /*!< TLB entrylo1 value */
} ;

/*!
  \brief This is the structure for passing the arguments to firmware via VMB. It is used in Chiptest
 */
struct tlb_auguments {
	uint32_t tlb_entry_num;        /*!< the 4K page based tlb entry number. Its maximum element number is \ref MAX_TLB_NUM. */
	struct tlb_entry tlb[MAX_TLB_NUM]; /*!< the tlb point address which contain detail tlb configuration   */
	uint32_t mpe_instance;         /*!< first MPE instance is 0, and second MPE instance is 1, and so on */
	uint32_t tcs_num;              /*!< The thread context (TC) num which was assigned already by all previous VPE instance  */
	uint32_t genconf_addr;         /*!< the genconf address which contain genconf configuration */
} ;

/*!
  \brief This is the structure for TLB setting
 */
struct page_mask_offset {
	uint32_t pagemask;  /*!< page mask */
	uint32_t offset;    /*!< offset */
	uint32_t offset_mask; /*!< offset mask */
};

/*!
  \brief This is the structure for tlb register entry_hi
 */
struct tlb_entryhi_t {
#ifdef CONFIG_BIG_ENDIAN
	union {
		struct {
			volatile uint32_t vpn2     : 19; /*!< vpn2 */
			volatile uint32_t resv     : 5;  /*!< reserved */
			volatile uint32_t asid     : 8;  /*!< asid per tc */
		} field; /*!< tlb entryhi fileld */
		volatile uint32_t all; /*!< tlb entryhi all */
	} reg;
#else
	union {
		struct {
			volatile uint32_t asid     : 8; /*!< asid per tc */
			volatile uint32_t resv     : 5; /*!< reserved */
			volatile uint32_t vpn2     : 19; /*!< vpn2 */
		} field; /*!< tlb entryhi fileld */
		volatile uint32_t all; /*!< tlb entryhi all */
	} reg; /*!< tlb_entryhi_t */
#endif

} ;

/*!
  \brief This is the structure for tlb register entry_lo
 */
struct tlb_entrylo_t {
#ifdef CONFIG_BIG_ENDIAN
	union {
		struct {
			volatile uint32_t pfn: 26; /*!< The "Physical Frame Number" represents bits 31:12 of the physical address. \n
                                        The 20 bits of PFN, together with 12 bits of in-page address, make up a 32-bit \n
                                        physical address. The MIPS32® Architecture permits the PFN to be as large as \n
                                        24 bits. The interAptiv core supports a 32-bit physical address bus. */
			volatile uint32_t c: 3;   /*!<  Coherency attribute of the page. */
			volatile uint32_t d: 1;    /*!< The "Dirty" flag. Indicates that the page has been written, and/or is writable. If \n
                                        this bit is a one, stores to the page are permitted. If this bit is a zero, stores to the \n
                                        page cause a TLB Modified exception.*/
			volatile uint32_t v: 1;    /*!< The “Valid” flag. Indicates that the TLB entry, and thus the virtual page mapping, \n
                                        are valid. If this bit is a set, accesses to the page are permitted. If this bit is \n
                                        a zero, accesses to the page cause a TLB Invalid exception.\n
                                        This bit can be used to make just one of a pair of pages valid. */
			volatile uint32_t g: 1;    /*!<  The “Global” bit. On a TLB write, the logical AND of the G bits in both the \n
                                        Entry 0 and Entry 1 registers become the G bit in the TLB entry. If the TLB \n
                                        entry G bit is a one, then the ASID comparisons are ignored during TLB \n
                                        matches. On a read from a TLB entry, the G bits of both Entry 0 and Entry 1 \n
                                        reflect the state of the TLB G bit.*/
		} field; /*!< tlb tlb_entrylo_t fileld */
		volatile uint32_t all; /*!< tlb tlb_entrylo_t all */
	} reg;
#else
	union {
		struct {
			volatile uint32_t g: 1;   /*!< The “Global” bit. On a TLB write, the logical AND of the G bits in both the \n
                                       Entry 0 and Entry 1 registers become the G bit in the TLB entry. If the TLB \n
                                       entry G bit is a one, then the ASID comparisons are ignored during TLB \n
                                       matches. On a read from a TLB entry, the G bits of both Entry 0 and Entry 1 \n
                                       reflect the state of the TLB G bit.*/
			volatile uint32_t v: 1;  /*!< The “Valid” flag. Indicates that the TLB entry, and thus the virtual page mapping, \n
                                      are valid. If this bit is a set, accesses to the page are permitted. If this bit is \n
                                      a zero, accesses to the page cause a TLB Invalid exception.\n
                                      This bit can be used to make just one of a pair of pages valid. */
			volatile uint32_t d: 1;  /*!< The "Dirty" flag. Indicates that the page has been written, and/or is writable. If \n
                                      this bit is a one, stores to the page are permitted. If this bit is a zero, stores to the \n
                                      page cause a TLB Modified exception.*/
			volatile uint32_t c: 3; /*!< Coherency attribute of the page. */
			volatile uint32_t pfn: 26;  /*!< The "Physical Frame Number" represents bits 31:12 of the physical address. \n
                                         The 20 bits of PFN, together with 12 bits of in-page address, make up a 32-bit \n
                                         physical address. The MIPS32® Architecture permits the PFN to be as large as \n
                                         24 bits. The interAptiv core supports a 32-bit physical address bus. */
		} field; /*!< tlb tlb_entrylo_t fileld */
		volatile uint32_t all;  /*!< tlb tlb_entrylo_t all */
	} reg; /*!< tlb_entryhi_t */
#endif
} ;

/*!
  \brief This is the structure for DMA Descriptor DW0.
 */
#ifdef CONFIG_BIG_ENDIAN
struct dma_desc_0 {
	uint32_t res: 3;  /*!< reserved */
	uint32_t tunnel_id: 4; /*!< tunnel id */
	uint32_t flow_id: 8;  /*!< flow id */
	uint32_t eth_type: 2; /*!< ethernet type*/
	uint32_t des_sub_if_id: 15; /*!< dest sub if id */
} ;

/*!
  \brief This is the structure for DMA Descriptor DW1.
 */

struct dma_desc_1	{
	uint32_t session_id: 12;  /*!< session id */
	uint32_t tcp_err: 1;  /*!< tcp error flag: 1 means tcp error */
	uint32_t nat: 1;  /*!< nat flag: 1 means for complementary mode */
	uint32_t dec: 1;  /*!< dec flag */
	uint32_t enc: 1;  /*!< enc flag */
	uint32_t mpe2: 1;  /*!< MPE2 flag */
	uint32_t mpe1: 1;  /*!< MPE1 flag */
	uint32_t color: 2; /*!< color flag */
	uint32_t ep: 4;  /*!< egress port */
	uint32_t res: 4;  /*!< reserved */
	uint32_t classid: 4; /*!< traffic class */
} ;

/*!
  \brief This is the structure for DMA Descriptor DW2.
 */
struct dma_desc_2	{
	uint32_t dram_location; /*!< buffer address */
} ;

/*!
  \brief This is the structure for DMA Descriptor DW3.
 */

struct dma_desc_3 {
	uint32_t own: 1;  /*!< ownership: */
	uint32_t c: 1;    /*!< c flag */
	uint32_t sop: 1;  /*!< sop flag */
	uint32_t eop: 1;  /*!< eop flag */
	uint32_t dic: 1;  /*!< dic flag: if it is set, cbm will directly drop the packet during CBM enqueue */
	uint32_t pdu_type: 1; /*!< pdu type */
	uint32_t byte_offset: 3; /*!< byte offset */
	uint32_t res: 7;  /*!< reserved */
	uint32_t data_length: 16; /*!< data length in the dma descriptor's buffer */
} ;

#else	//CONFIG_LITTLE_ENDIAN
/*!
  \brief This is the structure for DMA Descriptor DW0.
 */
struct dma_desc_0 {
	uint32_t des_sub_if_id: 15;/*!< dest sub if id */
	uint32_t eth_type: 2; /*!< ethernet type*/
	uint32_t flow_id: 8; /*!< flow id */
	uint32_t tunnel_id: 4; /*!< tunnel id */
	uint32_t res: 3; /*!< reserved */
} ;

/*!
  \brief This is the structure for DMA Descriptor DW1.
 */

struct dma_desc_1	{
	uint32_t classid: 4;/*!< traffic class */
	uint32_t res: 4; /*!< reserved */
	uint32_t ep: 4; /*!< egress port */
	uint32_t color: 2; /*!< color flag */
	uint32_t mpe1: 1; /*!< MPE1 flag */
	uint32_t mpe2: 1; /*!< MPE2 flag */
	uint32_t enc: 1;/*!< enc flag */
	uint32_t dec: 1; /*!< dec flag */
	uint32_t nat: 1; /*!< nat flag: 1 means for complementary mode */
	uint32_t tcp_err: 1; /*!< tcp error flag: 1 means tcp error */
	uint32_t session_id: 12; /*!< session id */
} ;

/*!
  \brief This is the structure for DMA Descriptor DW2.
 */

struct dma_desc_2 {
	uint32_t dram_location; /*!< buffer address */
} ;

/*!
  \brief This is the structure for DMA Descriptor DW3.
 */
struct dma_desc_3 {
	uint32_t data_length: 16; /*!< data length in the dma descriptor's buffer */
	uint32_t res: 7; /*!< reserved */
	uint32_t byte_offset: 3; /*!< byte offset */
	uint32_t pdu_type: 1; /*!< pdu type */
	uint32_t dic: 1;  /*!< dic flag: if it is set, cbm will directly drop the packet during CBM enqueue */
	uint32_t eop: 1;  /*!< eop flag */
	uint32_t sop: 1;  /*!< sop flag */
	uint32_t c: 1;   /*!< c flag */
	uint32_t own: 1;  /*!< ownership: */
} ;

#endif

/*!
  \brief This is the structure for DMA Descriptor DW0.
 */
struct mpe_dma_desc {
	struct dma_desc_0 desc0;  /*!<DMA descriptor 0 */
	struct dma_desc_1 desc1;  /*!<DMA descriptor 1 */
	struct dma_desc_2 desc2;  /*!<DMA descriptor 2 */
	struct dma_desc_3 desc3;  /*!<DMA descriptor 3 */
} ;

/*!
  \brief This is the enum for DMA descriptor node status
 */
enum node_status {
	FREE_NODE = 0, /*!< This node is just taken from \ref buffer_free_list */
	WAITING_NODE,	/*!< This node is dispatched from TM to WK, waiting for processing */
	READY_NODE		/*! <This node is processed and waiting to enqueue back (send) to CBM */
};

/*!
  \brief This is the structure for DMA Descriptor free node
 */
struct buffer_free_list {
	struct mpe_dma_desc desc[MAX_DMA_DESC_DUP]; /*!< list of DMA descriptors is equal to \ref cnt*/
	uint8_t 	idx;  	/*!< free list id */
	uint8_t	cnt;  	/*!< packet duplicate times */
	uint8_t	proto;	/*!< inner transport protocol after packet parsing */
	uint8_t 	stat;		/*!< node status, refer to \ref node_status */
	struct buffer_free_list *next; /*!< pointer to next free node */
};

/*@}*/ /* VMB_OTHERS */

/** \addtogroup  MPE_FW_HDR */
/*@{*/

/**
  \brief Basic TC Info
 */
struct TC_INFO  { /*make sure strucutre size is in DWORD */
	/*DWORD */
	char    name[20]; /*!< tc name */

	uint16_t  type;          /*!< TC type: \n
                               0 -- Normal Worker. refer to \ref TYPE_WORKER  \n
                               1 -- Normal TM. refer to \ref TYPE_TM  \n
                               others--For future  */
	uint8_t  min_tc_num;       /*!< Minimal TC requiried */
	uint8_t  max_tc_num;       /*!< Maximum TC supported */

	uint32_t  start_addr;  /*!< start address, its value will be like 0xCxxx,xxxx. Filled by compiler */
	uint32_t features_list[4]; /*!< feature list: refer to genconf */
} ;

/*!
  \brief DL buffer Info
 */

struct dl_buf_info {
	uint32_t tx_cfg_ctxt_buf_size; /*!< TX config context buffer size */
	uint32_t tx_cfg_ctxt_buf_base; /*!< base address for TX config context buffer */
	uint32_t rx_cfg_ctxt_buf_size; /*!< RX config context buffer size */
	uint32_t rx_cfg_ctxt_buf_base; /*!< base address for RX config context buffer */
	uint32_t rx_msgbuf_size; /*!< RX message buffer size */
	uint32_t rx_msgbuf_base; /*!< base address for RX message buffer */
	uint32_t comm_buf_size; /*!< communication buffer size */
	uint32_t comm_buf_base; /*!< communication buffer base */
	uint32_t uncached_addr_size;	/*!< Uncached address size */
	uint32_t uncached_addr_base;	/*!< Uncached address base */
	uint32_t DlCommmIpi; /*!< DirectLink communication IPI */
	uint32_t resv[2]; /*!< reserved */
};

/**  \brief MPE FW Header File Structure. Its contect is in big endian mode */

struct fw_hdr { /*Note: all filelds in this structure should follow fw_endian setting */

	uint8_t  fw_endian;      /*!< FW Endian : 0--Little Endian, 1--Big Endian. It is set during compilation */
	uint8_t  res[3];    /*!< Reserve */

	uint32_t compatible_id;  /*!< MPE FW and MPE HAL both should maintain its compitability id.\n
                               FW's compatible_id is set during compilation.\n
                               If FW's compatible_id not equal to MPE HAL's, it means MPE FW and MPE HAL are not compatible.\n
                               In this case, MPE HAL or MPE FW have to be updated.\n
                               For FW, normally compatible_id needs to be increased by 1 if any change in FW will cause incompatibility,\n
                               for example, genconf/session action changes.\n
                               For MPE HAL, it needs to check FW's compatible_id before starting MPE FW since FW is release in binary file.\n
                               It is readonly to MPE FW/HAL.
                              */
	uint8_t family;  /*!< FW family, refer to genconf. It is set during compilation */
	uint8_t package; /*!< FW package, refer to genconf. It is set during compilation  */
	uint8_t v_maj;   /*!< FW v_maj, refer to genconf. It is set during compilation  */
	uint8_t v_mid;   /*!< FW v_mid, refer to genconf. It is set during compilation  */

	uint8_t v_min;   /*!< FW v_min, refer to genconf. It is set during compilation */
	uint8_t v_tag;   /*!< FW v_tag, refer to genconf. It is set during compilation*/
	uint8_t res2[2];    /*!< FW family, refer to genconf  */

	uint8_t v_desc[MAX_VERSION_DESC_LEN];  /*!< FW v_desc, refer to genconf. It is set during compilation*/
	uint32_t genconf_offset;         /*!< Genconf offset based on fw code address. It is set by post compilation script */

	uint32_t hdr_size;      /*!< FW Header size. It is set during compilation*/
	uint32_t tlb_page_size;   /*!<  Passing TLB PAGE SIZE info for TLB setting on CP0 */
	uint32_t fw_code_size;  /*!< FW code size in bytes got from its elf. It is set by post compilation script. \n
                              MPE FW need to allocate MPE FW buffer for MPE FW code/data/bss.\n
                              The buffer size is \ref fw_code_size + \ref fw_data_size + \ref fw_bss_size.\n
                              This buffer must be aglined as specifed as \ref fw_code_align.
                             */
	uint32_t fw_data_size;  /*!< FW Data Size got from its elf. It is set by post compilation script*/
	uint32_t fw_bss_size;   /*!< FW BSS  Size got from its elf. It is set by post compilation script*/
	uint32_t fw_stack_priv_data_size;/*!< FW Per TC's Private Data Size got from its elf. It is set by post compilation script\n
                                       It is the multiples of TLB page size \ref TLB_PAGE_SIZE.\n
                                       MPE HAL should allocate this buffer for MPE FW's stack and private data section.\n
                                       The private data base address is ( its TC's sp - \ref fw_stack_priv_data_size).\n
                                       This private data base address should be aligned as specified by \ref fw_priv_data_align*/
	uint32_t fw_priv_data_size;/*!< Per TC's Private Data Size got from its elf. It is set by post compilation script\n
                                 it is the real priv_data section size according to the elf. \n
                                 It is used only for information, no much use at all.*/
	uint32_t fw_code_align; /*!< The memory alignment in bytes for MPE FW code. It is set during compilation*/
	uint32_t fw_priv_data_align; /*!< The memory alignment in bytes for MPE FW code. It is set during compilation*/

	uint32_t fw_priv_data_mapped_addr; /*!< TC's Private Data section's mapped address got from its elf. It is set by post compilation script.\n

Note: All TC shared this same virtual mapped address.\n
Stack and private data share same buffer.\n
Stack grow from high address to low address while private data grows from low to high address.\n
It is used for MPE FW do TLB setting*/

	struct dl_buf_info dl_buf_info_base[AC_RADIO_NUMBER];	/*!< Directlink baseaddr */

	struct TC_INFO tm_info; /*!< Normal TM information */
	struct TC_INFO worker_info;   /*!< Normal Worker information */
	struct TC_INFO dl_info;   /*!< Normal Directlink information */

} ;

/*@}*/ /* MPE_FW_HDR */

/************************Below is Compare/Action table related structure Definition *************************************/

/** \addtogroup  MPE_COMPARE_ACTION */
/*@{*/
/**  \brief MPE Common Structure Definition
 */

/*!
  \brief This is ipv4 key for hash-auto based compare table. It has 4 DWORD
 */
#ifdef CONFIG_BIG_ENDIAN
struct ipv4_hash_auto_key {
	uint32_t rsvd: 24;     /*!< reserved bit for future */
	uint32_t extn: 8;      /*!< routing extention: currently only bit 0 is used for tcp/udp protocol differentiation */
	uint32_t srcport: 16;  /*!< TCP/UDP source port */
	uint32_t dstport: 16;  /*!< TCP/UDP destination port */
	uint32_t dstip;        /*!< destination IP address */
	uint32_t srcip;        /*!< source IP address */
} ;
#else
struct ipv4_hash_auto_key {
	uint32_t extn: 8;     /*!< routing extention: currently only bit 0 is used for tcp/udp protocol differentiation */
	uint32_t rsvd: 24;    /*!< reserved bit for future */
	uint32_t dstport: 16; /*!< TCP/UDP destination port */
	uint32_t srcport: 16; /*!< TCP/UDP source port */
	uint32_t dstip;  /*!< destination IP address */
	uint32_t srcip; /*!< source IP address */
} ;
#endif

/*!
  \brief This is ipv6 key for hash-auto based compare table. It has 10 DWORD
 */
#ifdef CONFIG_BIG_ENDIAN
struct ipv6_hash_auto_key {
	uint32_t rsvd: 24;     /*!< reserved bit for future */
	uint32_t extn: 8;     /*!< routing extention: currently only bit 0 is used for tcp/udp protocol differentiation */
	uint32_t srcport: 16; /*!< TCP/UDP source port */
	uint32_t dstport: 16; /*!< TCP/UDP destination port */
	uint32_t dstip[4];    /*!< destination IP address */
	uint32_t srcip[4];    /*!< source IP address */
} ;
#else
struct ipv6_hash_auto_key {
	uint32_t extn: 8;      /*!< routing extention: currently only bit 0 is used for tcp/udp protocol differentiation */
	uint32_t rsvd: 24;     /*!< reserved bit for future */
	uint32_t dstport: 16;  /*!< TCP/UDP destination port */
	uint32_t srcport: 16;  /*!< TCP/UDP source port */
	uint32_t dstip[4];    /*!< destination IP address */
	uint32_t srcip[4];    /*!< source IP address */
} ;
#endif

/*!
  \brief This is manual key for hash based compare table. It has 14 DWORD
 */
struct hash_manual_key {
	uint32_t key[14]; /*!< 14 DWORD manual key. It is flexible to define the key */
} ;

/*!
  \brief This is MPE Hash based IPV4 Compare table definitioin( 6 DWORD )
 */

#ifdef CONFIG_BIG_ENDIAN
struct fw_compare_hash_auto_ipv4 {
	/* 1H */
	/* nxt_ptr and first_ptr are null in the linear table */
	uint32_t nxt_ptr : 15;       /*!< next compare entry index in this hash index */
	uint32_t resv	 : 1;    /*!< reserve bit */
	uint32_t first_ptr	 : 15;   /*!< first valid compare entry index for this hash index*/
	uint32_t valid	 : 1;   /*!< valid bit: 1 means valid, 0-not valid */
	/* 2H - 5H */
	struct ipv4_hash_auto_key key;  /*!< IPV4 hash-auto key ( 4DWORD) */
	/* 6H */
	uint32_t act;    /*!< session action pointer ( 1 DWORD ) */
} ;
#else
struct fw_compare_hash_auto_ipv4 {
	/* 1H */
	/* nxt_ptr and first_ptr are null in the linear table */
	uint32_t valid	 : 1;  /*!< valid bit: 1 means valid, 0-not valid */
	uint32_t first_ptr  : 15; /*!< first valid compare entry index for this hash index*/
	uint32_t resv         : 1; /*!< reserve bit */
	uint32_t nxt_ptr : 15;    /*!< next compare entry index in this hash index */
	/* 2H - 5H */
	struct ipv4_hash_auto_key key; /*!< IPV4 hash-auto key ( 4DWORD) */
	/* 6H */
	uint32_t act;  /*!< session action pointer ( 1 DWORD ) */
} ;
#endif

/*!
  \brief This is MPE Hash based IPV6 Compare table definitioin (12 DWORD)
 */
#ifdef CONFIG_BIG_ENDIAN
struct fw_compare_hash_auto_ipv6 {
	/* 1H */
	/* nxt_ptr and first_ptr are null in the linear table */
	uint32_t nxt_ptr : 15;        /*!< next compare entry index in this hash index */
	uint32_t resv	 : 1;    /*!< reserve bit */
	uint32_t first_ptr	: 15;    /*!< first valid compare entry index for this hash index*/
	uint32_t valid	 : 1; /*!< valid bit: 1 means valid, 0-not valid */
	/* 2H - 11H*/
	struct ipv6_hash_auto_key key;  /*!< IPV6 hash-atuo key ( 10 DWORD) */
	/* 12H */
	uint32_t act;		/*!< session action pointer ( 1 DWORD ) */
} ;
#else
struct fw_compare_hash_auto_ipv6 {
	/* 1H */
	/* nxt_ptr and first_ptr are null in the linear table */
	uint32_t valid	 : 1; /*!< valid bit: 1 means valid, 0-not valid */
	uint32_t first_ptr	 : 15; /*!< first valid compare entry index for this hash index*/
	uint32_t resv	 : 1; /*!< reserve bit */
	uint32_t nxt_ptr : 15;  /*!< next compare entry index in this hash index */
	/* 2H - 11H*/
	struct ipv6_hash_auto_key key; /*!< IPV6 hash-atuo key ( 10 DWORD) */
	/* 12H */
	uint32_t act; /*!< session action pointer ( 1 DWORD ) */
} ;
#endif

/*!
  \brief This is MPE Hash based IPV6 Compare table definitioin (12 DWORD)
 */
#ifdef CONFIG_BIG_ENDIAN
struct fw_compare_hash_manual {
	/* 1H */
	uint32_t nxt_ptr : 15;    /*!< next compare entry index in this hash index */
	uint32_t resv    : 1;     /*!< reserve bit */
	uint32_t first_ptr  : 15; /*!< first valid compare entry index for this hash index*/
	uint32_t valid   : 1;     /*!< valid bit: 1 means valid, 0-not valid */
	/* 2H - 15H*/
	struct hash_manual_key key;  /*!< IPV6 hash-atuo key ( 14 DWORD) */
	/* 16H */
	uint32_t act;	    /*!< session action pointer ( 1 DWORD ) */
} ;
#else
struct fw_compare_hash_manual {
	uint32_t valid   : 1;	  /*!< valid bit: 1 means valid, 0-not valid */
	uint32_t first_ptr  : 15; /*!< first valid compare entry index for this hash index*/
	uint32_t resv	  : 1;	  /*!< reserve bit */
	uint32_t nxt_ptr : 15;	  /*!< next compare entry index in this hash index */
	/* 2H - 15H*/
	struct hash_manual_key key;  /*!< IPV6 hash-atuo key ( 14 DWORD) */
	/* 16H */
	uint32_t act;		/*!< session action pointer ( 1 DWORD ) */

};
#endif

/*!
  \brief This is MPE Common Compare table definitioin
 */
union fw_compare_table {
	struct fw_compare_hash_auto_ipv4 ipv4;   /*!< ipv4 hash-auto  */
	struct fw_compare_hash_auto_ipv6 ipv6;   /*!< ipv6 hash-auto*/
	struct fw_compare_hash_manual manual;   /*!< manual hash */
};

/*!
  \brief IPV4 Address
 */
struct ipv4_addr {
	uint32_t ip;   /*!< IPV4 address entry in the ipv4_tbl */
} ;

/*!
  \brief IPV6 Address
 */
struct ipv6_addr {
	uint32_t ip[4]; /*!< IPV6 address entry in the ipv6_tbl */
} ;

/*!
  \brief This is the data structure for basic IPV4/IPV6 address
 */
union ip_addr {
	struct ipv4_addr ip4;   /*!< ipv4 address */
	struct ipv6_addr ip6;   /*!< ipv6 address */
};

/*!
  \brief This is the data structure for VAP entry
 */
struct vap_entry {
	uint8_t num;    /*!< the number of vap which joinined this mc group. It should meet: num <= \ref MAX_VAP_PER_PORT */
	uint8_t res[3]; /*!< Reserved*/
	uint16_t vap_list[MAX_VAP_PER_PORT]; /*!< VAP List */
} ;

/*!
  \brief This is enum for search engine mask type for hash index calculation
 */
enum cast_mask {
	NO_MASK = 0,
	MASK_ROUT_EXT,
	IP_ONLY,
	DSTIP_ONLY,
};

/**
  \brief key value for GRE and IPSEC in the future
 */
union mpe_key {
	uint32_t k; /*!< key:currently only for GRE downstream traffic */
};

/*!
  \brief This is Session Action definition with template buffer support.
 */
struct session_action {
	/*1st byte of DWORD */
	uint32_t entry_vld           : 1; /*!< Entry valid flag set by MPE HAL, used by MPE FW: 0-not valid. 1-valid */
	uint32_t protocol            : 1; /*!< Protocol: 1: TCP, 0: UDP*/
	uint32_t routing_flag        : 1; /*!< routing flag: 1-routing, 0-bridging. mainly for CAPWAP briging case*/
	uint32_t new_src_ip_en       : 1; /*!<  new_src_ip_en flag: 1-\ref new_src_ip is valid, 0-not valid*/
	uint32_t new_dst_ip_en       : 1; /*!<  new_dst_ip_en flag: 1-\ref new_dst_ip is valid, 0-not valid*/
	uint32_t new_inner_dscp_en   : 1; /*!< new_inner_dscp_en flag: 1-\ref new_inner_dscp is valid, 0-not valid */
	uint32_t pppoe_offset_en     : 1; /*!< pppoe_offset_en flag: 1- \ref pppoe_offset is valid, 0-not valid */
	uint32_t tunnel_ip_offset_en : 1; /*!< tunnel_ip_offset_en flag: 1-\ref tunnel_ip_offset is valid, 0-not valid */

	/*2st byte of DWORD */
	uint32_t tunnel_udp_offset_en: 1;  /*!< tunnel_udp_offset_en flag: 1-\ref tunnel_udp_offset is valid, 0-not valid */
	uint32_t in_eth_iphdr_offset_en: 1; /*!< in_iphdr_offset_en flag: 1-\ref tunnel_ip_offset is valid, 0-not valid */
	uint32_t sess_mib_ix_en      : 1; /*!< sess_mib_ix_en flag: 1-\ref sess_mib_ix is valid. 0--not valid*/
	uint32_t new_traffic_class_en: 1; /*!< traffic_class_en for DMA Descriptor: 1- \ref traffic_class is valid and copy to DMA Descriptor. 0-not valid */
	uint32_t tunnel_rm_en        : 1; /*!< tunnel header remove flag: \n
	                                        	* 1-remove tunnel header included in \ref templ_buf and and MPE FW need update \ref pkt_len_delta accordingly. \n
                                       	 	* 0-no need to remove tunnel header \n
                                       		* Note -- it is for tunnel downstream traffic only. \n
                                       		*/
	uint32_t meter_id1_en        : 1; /*!< the meter instance ID1 enable/disable flag. For meter_id0, it is always enabled here.*/
	uint32_t key_en              : 1; /*!< key flag: 1-enabled, 0-disabled. \ref key is valid if key_en 1. It is used for GRE Downstream traffic now */
	uint32_t sess_mib_hit_en		: 1; /*!< session MIB hit table selector: 1- hit info is valid, 0-Not valid */

	/*3rd byte of DWORD */
	uint32_t tunnel_id: 4;		/*!< ipsec tunnel ID, currently valid from 0 ~ 15 > */

	/*4th byte of DWORD */
	uint32_t reserve2: 3;		/*!< reserved > */
	uint32_t mc_way: 1;        /* Duplication method used for mcast: 1- always use new buffer, 0- depend on ingress PMAC setting \ref pmac_len_out */

	/*1st byte of DWORD */
	uint32_t outer_dscp_mode     : 2; /*!< For tunnel header's DSCP if necessary: 0-no marking, 1-from packet's Inner DSCP, 2-from action's inner DSCP, 3-reserved */
	uint32_t reserve3            : 6; /*!< for alignment */

	/*2nd byte of DWORD */
	uint32_t meter_id0          : 4; /*!< the meter instance ID of meter0. Maximum support 16 meter in HW now. As define. meter0 is enable by default*/
	uint32_t meter_id1          : 4; /*!< the meter instance ID. Maximum support 16 meter in HW now */

	/*3rd byte of DWORD */
	uint32_t new_inner_dscp      : 6; /*!< new DSCP value. Valid only if \ref new_inner_dscp_en == 1*/
	uint32_t reserve4            : 2; /*!< resrved for alignment  */

	/*4th byte of DWORD */
	uint32_t tunnel_type        : 4; /*!< refer to \ref TUNL_TYPE */
	uint32_t dst_pmac_port_num   : 4;  /*!<  Number of destination PMAC Port  configured in the dst_pmac_port_list. \n
                                         * For unicast it is 1 and for multicast the maximum value is \ref MAX_PMAC_PORT \n
                                        */

	/*DWORDs */
	union ip_addr new_src_ip;        /*!< new Source IP address. Valid only if \ref new_src_ip_en == 1.
                                      */
	union ip_addr new_dst_ip;        /*!< new Destination IP address. Valid only if \ref new_dst_ip_en == 1 */

	/*DWORD */
	uint32_t new_src_port: 16; /*!< New Source TCP/UDP port. 0 means no change/action*/
	uint32_t new_dst_port: 16; /*!< New Destination TCP/UDP port. 0 means no change/action */

	uint8_t mc_index;		  /*!< To get 7 bits sub interface MC index even without PAE support.\n
					* Applicable only for multicast application in chiptest and real case.\n
					* Its value should be set similar to incoming DMA descriptor sub interface index in real case.
					*/

	/*1st byte of DWORD */
	uint32_t traffic_class: 4; /*!< For TMU queue selection. In DMA descriptor, it is defined to be traffic class; */

	/*DWORDS*/
	uint8_t *templ_buf;  /*!< Pointer to template buffer per session. */

	/*1st byte of DWORD */
	char   pkt_len_delta;	/*!< Header length delta after editing. It is signed variable\n
                                             * The header data should be continuous data: MAC hdr, VLAN hdr,\n
                                             * PPPOE hdr, Tunnel hdr, original original IP hdr's. \n
                                             * it does not include removed tunnel header length since PPA does not know it. \n
                                             * Instead, MPE FW need to minus removed the removed real tunnel header after parsing parser header info.
                                            */
	/*2nd byte of DWORD */
	uint8_t  templ_len;	/*!< data length in template buffer */

	/*3rd byte of DWORD */
	uint8_t  pppoe_offset;	/*!< PPPOE header offset in the template buffer. Valid if \ref pppoe_offset_en ==1 \n
                                               * for updating payload length */
	/*4th byte of DWORD */
	uint8_t  tunnel_ip_offset;	/*!< tunnel's ip header offset in the template buffer. valid if \ref tunnel_ip_offset_en == 1 */

	/*1st byte of DWORD */
	uint8_t  in_eth_iphdr_offset;	/*!< offset for inner ip header starting address according to template buffer\n. */
	/*2nd byte of DWORD */
	uint8_t  tunnel_udp_offset;	/*!< tunnel udp offset based on its ip header. Its value is IP HLEN >> 2 */

	/*3rd byte of DWORD */
	uint8_t rx_itf_mib_num;	/*!< rx_inf number configured in \ref rx_itf_mib_ix */

	/*4th byte of DWORD */
	uint8_t tx_itf_mib_num;	/*!< tx_itf_num number configured in \ref tx_itf_mib_ix */

	union mpe_key  key;	/*!< gre/ipsec key. It is valid only if \ref key_en is valid */

	/* 1st word of DWORD  */
	uint16_t sess_mib_ix;	/*!< Session mib counter index point to session mib table. Valid only if \ref sess_mib_ix_en == 1 \n
                                            * It can be disabled in order to save system memory of session mib buffer  */
	/* 2nd word of DWORD  */
	uint16_t mtu;	/*!< Maximum Transfer Unit(MTU) does not include any link layer protocol overhead, ie, Maximum packet size supported by MPE FW for editing.*/

	/* DWORDS: */
	uint8_t dst_pmac_port_list[MAX_PMAC_PORT];    /*!< Destination PMAC Port ID: \n
                                                    * for unicast only read index 0. \n
                                                    * 0   --CPU  \n
                                                    * 1-6 --Ethernet LAN Ports \n
                                                    * 7-14--VRX318(DSL)/WIFI/LTE/CAPWAP other directpath's port \n
                                                    * 15  --Ethernet WAN ports \n
                                                    * for mcast read the whole list i.e the port list which join this mcast. Valid only if mc_flag is 1.
                                                   */

	uint16_t uc_vap_list[MAX_PMAC_PORT];    /*!< Unicast VAP list */

	/*DWORDs  */
	/* Interface mib counter*/
	uint8_t rx_itf_mib_ix[MAX_ITF_PER_SESSION]; /*!< virtual interface index, maximum support \ref MAX_ITF_PER_SESSION interface list per session*/
	uint8_t tx_itf_mib_ix[MAX_ITF_PER_SESSION]; /*!< virtual interface index, maximum support \ref MAX_ITF_PER_SESSION interface list per session*/

} ;

/*!
  \brief This is HW acceleration action definition.
 */
struct hw_act_ptr {
	uint32_t act;    /*!< action table's pointer address */
} ;

/*@}*/ /* MPE_COMPARE_ACTION */

/************************Below is MIB/RMON Structure Definition *************************************/
/** \addtogroup  MPE_MIB */
/*@{*/

/*!
  \brief This is basic MIB info definition.
 */
struct mib_info {
	unsigned long long bytes ;   /*!< session bytes mib counter */
	uint32_t pkt;            /*!< session packet mib counter for matched sessions\n
                               UDP/TCP packet mib counter for non-matched packet ( For CPU path mib counter )
                              */
	uint32_t non_acc_pkt;  /*!< non accelerated packet mib counter ( as fragmented, too big pkt, TTL 1/0) for matched sessions\n
                             non-UDP/TCP packet mib counter for non-matched packet( For CPU path mib counter ).
                            */
} ;

/*!
  \brief This is Session MIB definition.
 */
struct session_mib {
	struct mib_info mib;   /*!< session mib */
} ;

/*!
  \brief This is basic MIB per TC info definition. Updated by MPE FW and can be clear/read by HAL
 */
struct mib_tc {
	uint32_t deq_cnt; /*!< Dequeue counter per TC. It will be set by MPE FW on the fly: \n
                        Increment by one for each dequeued packet. Total of acc_pkt_cnt and \n
                        nona_pkt_cnt
                       */
	uint32_t enq_cnt; /*!< Enqueue counter per TC. It will be set by MPE FW on the fly: \n
                        Increment by one (ucast) or more (mcast) accelerated packets. \n
                        For ucast traffic only it has maximum value = deq_cnt, where all packets accelerated.
                       */
	uint32_t acc_pkt_cnt; /*!< Worker TC based rx accelerated packet counter: \n
                            Increment by one for each dequeued packet for accelerated case.
                           */
	uint32_t nona_pkt_cnt; /*!< Worker TC based rx non-accelerated packet counter: \n
                             Increment by one for each dequeued packet  for non-accelerated case.
                            */
	uint32_t pkt_cnt_in[MAX_PMAC_PORT]; /*!< Worker TC based dequeue packet counter for each PMAC port (sppid). \n
                            Increment by one for each dequeued packet from dispacth queue.
                            */

	uint32_t dec_pkt[IPSEC_TUN_MAX]; /*!< Worker TC based, good decrypted packet counter for each IPSEC tunnel. \n
                            Increment by one for each good decrypted packet to EIP97 HW.
                            */

	uint32_t dec_pkt_err[IPSEC_TUN_MAX]; /*!< Worker TC based, bad decrypted packet counter for each IPSEC tunnel. \n
                            Increment by one for each decrypted packet with specific error from EIP97 HW.
                            */

	uint32_t dl_deq_cnt; /*!< Dequeue counter per TC. It will be set by MPE FW on the fly: \n
                           Increment by one for each dequeued packet. Total of acc_pkt_cnt and \n
                           nona_pkt_cnt
                          */
	uint32_t dl_enq_cnt; /*!< Enqueue counter per TC. It will be set by MPE FW on the fly: \n
                           Increment by one (ucast) or more (mcast) accelerated packets. \n
                           For ucast traffic only it has maximum value = deq_cnt, where all packets accelerated.
                          */
};

/*!
  \brief This is Virtual Interface based mib counter
 */
struct mpe_itf_mib {
	struct mib_info rx_mib;   /*!< interface based mib counter */
	struct mib_info tx_mib;   /*!< interface based mib counter */
} ;

/*@}*/ /* MPE_MIB */

/** \addtogroup  MPE_HEADER_HW_RES */
/*@{*/
/*!
  \brief This is the structure for passing the HW resource to firmware.
 */
struct tc_hw_res {
	uint32_t flag: 1;       	/*!< HAL CONF: flag: 1-this HW resurce is assigned to some TC already. 0 --this resource is free */
	uint32_t res: 31;       	/*!< Reserve for futher */
	uint32_t logic_mpe_tc_id; 	/*!< ID value assignment:  \n
                                  1) 0 ~ (MAX_WORKER_NUM-1): used for worker_tc. First worker_tc's logic_mpe_tc_id is 0, then 1, and ...\n
                                  2) MAX_WORKER_NUM ~ (MAX_WORKER_NUM+MAX_TM_NUM-1):used for TM. First tm's logic_mpe_tc_id is MAX_WORKER_NUM.
                                 */
	uint32_t TcQid;      		/*!< HAL CONF: TC's Qid */
	uint32_t yield;     		/*!< HAL CONF: yield value, not yield ID. The yield value is like 1, 2, 4, 8, 16, ... */
	uint32_t CpuNum;			/*!< HAL CONF: CpuNum ( ie, vpc index) will be set by MPE HAL */
	uint32_t TcNum;				/*!< Real TC Num on which TM/WORKER/DL in a core works  */
	uint32_t VmbFwIpi;    		/*!< HAL CONF: TC's IPI interrupt Incoming from VMB to Mpe FW */
	uint32_t FwVmbIpi;    		/*!< HAL CONF: TC's IPI to be acked back to Mpe FW to VMB   */
	uint32_t tcType;    		/*!< HAL CONF: The TC could behave as TM / TC(worker), 0 -TYPE_TM 1 -TYPE_WORKER */
	uint32_t state;     		/*!< Two states define - STATE_IN_WORK - 0 STATE_HALTED- 1 */
	uint32_t MpeCmdReqReg; 		/*!< MPE CONF: MPE Ctrl request command register address */
	uint32_t MpeCmdReqRegIrq; 	/*!< MPE CONF: MPE Ctrl request command irq*/
	uint32_t MpeCmdRespReg;    	/*!< MPE CONF: MPE Ctrl respond command register address */
	uint32_t MpeCmdRespRegIrq;	/*!< MPE CONF: MPE Ctrl respond command irq*/
	uint32_t McpyPort; 			/*!< HAL CONF: DMA memcpy port ID*/
	uint32_t McpyTxDescBase;	/*!< MPE CONF: DMA memcpy Tx port desc base*/
	uint32_t McpyRxDescBase;	/*!< MPE CONF: DMA memcpy Rx port desc base*/
	uint32_t McpyTxCh;			/*!< MPE CONF: DMA memcpy Rx channel*/
	uint32_t McpyRxCh;			/*!< MPE CONF: DMA memcpy Tx channel*/
	uint32_t McpyCmdReg;  		/*!< MPE CONF: DMA memcpy port's comamnd register address */
	uint32_t McpyRespReg; 		/*!< MPE CONF: DMA memcpy port's respond register address */
	uint32_t McpyIrq;     		/*!< MPE CONF: DMA memcpy port's irq */
	uint32_t CbmDeQPort;  		/*!< HAL CONF: cbm dequeue port id*/
	uint32_t CbmDeQPortReg; 	/*!< MPE CONF: cbm dequeue port register address */
	uint32_t CbmDeQPortRegIrq;  /*!< MPE CONF: ccbm dequeue port's irq */
	uint32_t CbmEnQPort; 		/*!< HAL CONF: cbm enqueue port id*/
	uint32_t CbmEnQPortReg; 	/*!< MPE CONF: cbm enqueue port register address*/
	uint32_t CbmEnQPortRegIrq; 	/*!< MPE CONF: cbm enqueue port irq */
	uint32_t CbmEnQIrnenReg;   	/*!< MPE CONF: cbm enqueue CPU Ingress Port IRN Interrupt Enable Register */
	uint32_t CbmEnQIrncrReg;   	/*!< MPE CONF: cbm enqueue CPU Ingress Port IRN Interrupt Capture Register */
	uint32_t CbmIgpIrnenReg;   	/*!< MPE CONF: cbm Ingress Port IRN Interrupt Enable Register */
	uint32_t CbmLsStatusReg;   	/*!< MPE CONF: cbm LS status Register */
	uint32_t MpeDispatchQid;  	/*!< HAL CONF: acceleration dispatch queue id*/
	uint32_t MpeDispatchCmdReg; /*!< MPE CONF: acceleration dispatch cmd register address */
	uint32_t MpeDispatchCmdRegIrq; 	/*!< MPE CONF: acceleration dispatch cmd register irq */
	uint32_t MpeDispatchRespReg;  	/*!< MPE CONF: acceleration dispatch respond register address */
	uint32_t MpeDispatchRespRegIrq; /*!< MPE CONF: acceleration dispatch respond register irq */
	uint32_t MpeDebugQid; 		/*!< HAL CONF: debug queue id */
	uint32_t MpeDebugCmdReg; 	/*!< MPE CONF: acceleration Debug cmd register address */
	uint32_t MpeDebugCmdRegIrq; /*!< MPE CONF: acceleration Debug cmd register irq */
	uint32_t MpeDebugRespReg;  	/*!< MPE CONF: acceleration Debug respond register address */
	uint32_t MpeDebugRespRegIrq;/*!< MPE CONF: acceleration Debug respond register irq */
	uint32_t itc_view;			/*!< HAL CONF: MIPS itc view */
	uint32_t disp_q_semid;  	/*!< HAL CONF: MIPS itc semaphore address for dequeue packet from dispatch queue */
	uint32_t free_list_semid; 	/*!< HAL CONF: MIPS itc semaphore address for free list of DMA descriptor */
	uint32_t cbm_alloc_semid; 	/*!< HAL CONF: MIPS itc semaphore address for buffer allocation from cbm */
	uint32_t dispatch_q_cnt_semid; /*!<  HAL CONF: MIPS itc semaphore address for TM dispatch queue counter */
	uint32_t MpeSearchQid; 		/*!< MPE CONF: search queue id */
	uint32_t MpeMeterQid;  		/*!< MPE CONF: meter queue id */
	uint32_t private_ddr_addr; 	/*!< HAL CONF: For Per tc based private data. should be at least in 4K bytes alignment for TLB purpose */
	uint32_t e97_ring_id;		/*!< HAL_CONF: Eip97 Ring ID for IPSEC implementation */
	uint32_t e97CdrRingBaseReg;	/*!< MPE_CONF: Eip97 CDR Ring Base Address register */
	uint32_t e97CdrRingSizeReg;	/*!< MPE_CONF: Eip97 CDR Ring Size register */
	uint32_t e97CdrDescSizeReg;	/*!< MPE_CONF: Eip97 CDR Descriptor Size register */
	uint32_t e97CdrCfgReg;		/*!< MPE_CONF: Eip97 CDR Configuration register */
	uint32_t e97CdrPrepCntReg;	/*!< MPE_CONF: Eip97 CDR Prepared Count register */
	uint32_t e97CdrProcPntReg;	/*!< MPE_CONF: Eip97 CDR Processed Pointer register */
	uint32_t e97CdrPrepPntReg;	/*!< MPE_CONF: Eip97 CDR Prepared Pointer register */
	uint32_t e97RdrThreshReg;	/*!< MPE_CONF: Eip97 RDR threshold register */
	uint32_t e97RdrStsReg;		/*!< MPE_CONF: Eip97 RDR Status register */
	uint32_t e97RdrProcCntReg;	/*!< MPE_CONF: Eip97 RDR Processed Count register */
	uint32_t e97RdrPrepCntReg;	/*!< MPE_CONF: Eip97 RDR Prepared Count register */
	uint32_t e97RdrProcPntReg;	/*!< MPE_CONF: Eip97 RDR Processed Pointer register */
	uint32_t e97RdrPrepPntReg;	/*!< MPE_CONF: Eip97 RDR Prepared Pointer register */
	uint32_t e97RingIrq;		/*!< MPE_CONF: Eip97 Ring Irq */

	uint32_t DlMpeDispatchQid;  		/*!< HAL CONF: Directlink dispatch queue id*/
	uint32_t DlMpeDispatchCmdReg; 		/*!< MPE CONF: Directlink dispatch cmd register address */
	uint32_t DlMpeDispatchCmdRegIrq; 	/*!< MPE CONF: Directlink dispatch cmd register irq */
	uint32_t DlMpeDispatchRespReg;  	/*!< MPE CONF: Directlink dispatch respond register address */
	uint32_t DlMpeDispatchRespRegIrq; 	/*!< MPE CONF: Directlink dispatch respond register irq */

	uint32_t DlCbmDeQPort;  			/*!< HAL CONF: Directlink cbm dequeue port id*/
	uint32_t DlCbmDeQPortReg; 			/*!< MPE CONF: Directlink cbm dequeue port register address */
	uint32_t DlCbmDeQPortRegIrq;  		/*!< MPE CONF: Directlink ccbm dequeue port's irq */
	uint32_t DlCbmLsStatusReg;			/*!< MPE CONF: DL cbm LS status Register */

	uint32_t DlCbmFreeMemPort; 			/*!< MPE CONF: Directlink cbm dequeue port to free CBM buffer port */
	uint32_t DlCbmFreeMemPortReg; 		/*!< MPE CONF: Directlink cbm dequeue port to free CBM buffer port register address */

	uint32_t DlCommIpi;    /*!< Directlink TC's IPI  from Linux */

	uint32_t Dlfree_list_semid; 	/*!< HAL CONF: MIPS itc semaphore address for Directlink free list of DMA descriptor */
	uint32_t Dldispatch_q_cnt_semid; /*!<  HAL CONF: MIPS itc semaphore address for Directlink TM dispatch queue counter */
} ;

/*@}*/ /* MPE_PROFILING */

/** \addtogroup  MPE_PROFILING */
/*@{*/
/*!
  \brief This is the structure for api level and full profiling.
 */

struct mpe_profiling {
	uint32_t wk_api_profile_start;		/*!< Profiling start for wk_api's */
	uint32_t wk_profile_start;			/*!< wk full profiling start */
	uint32_t wk_deq_dispatch_q_start;	/*!< Profiling start for deq_dispatch_q */
	uint32_t tm_api_profile_start;		/*!< Profiling start for tm_api's */

	uint64_t cacheinval_end;			/*!< Profiling end for cache invalidating */
	uint64_t ddr2c_end;					/*!< Profiling end for ddr to L2$ copy */
	uint64_t wk_collect_pkinfo_end;		/*!< Profiling end for wk_collect_pkinfo() */
	uint64_t wk_get_ses_act_ptr_end;	/*!< Profiling end for wk_get_ses_act_ptr() */
	uint64_t pmac_and_tbuf_mcpy_end;	/*!< Profiling end for PMAC header copy and template buffer copy/gathering */
	uint64_t pppoe_edit_end;			/*!< Profiling end for PPPOE edit, if applicable */
	uint64_t wk_update_tunnel_hdr_end;	/*!< Profiling end for wk_update_tunnel_hdr(), if applicable */
	uint64_t wk_update_ip_port_end;		/*!< Profiling end for wk_update_ip_port_en() */
	uint64_t wk_itf_mib_counter_end;	/*!< Profiling end for wk_itf_mib_counter_en(), if applicable */
	uint64_t wk_sess_mib_counter_end;	/*!< Profiling end for wk_sess_mib_counter(), if applicable */
	uint64_t wk_hit_mib_counter_end;	/*!< Profiling end for wk_hit_mib_counter(), if applicable */
	uint64_t wk_edit_loop_end;			/*!< Profiling end for wk_edit_loop() */
	uint64_t wk_pmac_basic_edit_end;	/*!< Profiling end for wk_pmac_basic_edit() */
	uint64_t cache_flush_end;			/*!< Profiling end for cache_flushing */
	uint64_t dma_desc_update_end;		/*!< Profiling end for DMA descriptor update */
	uint64_t wk_send_to_tm_end;			/*!< Profiling end for wk_send_to_tm() */
	uint64_t tm_dispatch_pkt_end;		/*!< Profiling end for tm_dispatch_pkt() */
	uint64_t tm_process_tc_q_end;		/*!< Profiling end for tm_process_tc_q() */
	uint64_t wk_process_pkt_end;		/*!< Profiling end for wk_process_pkt() */
	uint64_t wk_deq_dispatch_q_end;		/*!< Profiling end for dequeue from dispatch Q */

	uint64_t wk_profile_end;			/*!< wk full profiling end */

	uint32_t profile_flag;				/*!< wk full profiling control flag */
	uint32_t pkt;						/*!< no: of acc pkts for profiling */
	uint32_t old_pkt_cnt;				/*!< temp variable to check only accelerated packets used for profiling */
};

/*@}*/ /* MPE_HEADER_HW_RES */

enum e97_ring_idx {
	EIP97_RING0 = 0,
	EIP97_RING1,
	EIP97_RING2,
	EIP97_RING3,
	EIP97_INVALID_RING
};

#ifdef CONFIG_BIG_ENDIAN
/*!
  \brief This is the structure for EIP97 Command Descriptor DW0
 */
struct e97_cdw_0	{
	uint32_t acd_size: 8;	/*!< additional control data size */
	uint32_t first_seg: 1;	/*!< first segment indicator */
	uint32_t last_seg: 1;	/*!< last segment indicator */
	uint32_t res: 5;			/*!< reserved */
	uint32_t buf_size: 17;	/*!< size of input data buffer in bytes */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW1
 */
struct e97_cdw_1 {
	uint32_t buf_ptr;	/*!< data buffer pointer */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW2
 */
struct e97_cdw_2 {
	uint32_t acd_ptr;	/*!< additional control data pointer */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW3
 */
struct e97_cdw_3	{
	uint32_t type: 2;	/*!< type of token, reserved*/
	uint32_t u: 1;		/*!< upper layer header from token */
	uint32_t iv: 3;	/*!< IV??? */
	uint32_t c: 1;		/*!< context control words present in token */
	uint32_t too: 3;	/*!< type of output */
	uint32_t rc: 2;	/*!< reuse context */
	uint32_t ct: 1;	/*!< context type, reserved */
	uint32_t cp: 1;	/*!< context pointer 64bit */
	uint32_t ip: 1;	/*!< EIP97 Mode */
	uint32_t len: 17;	/*!< input packet length */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW4
 */
struct e97_cdw_4 {
	uint32_t res: 8;		/*!< reserved */
	uint32_t app_id: 24;	/*!< application id */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW5
 */
struct e97_cdw_5 {
	uint32_t ctx_ptr/*: 30*/;	/*!< The pointer to the offset of the Context in Host memory */
	//uint32_t refresh: 2;	/*!< Refresh flags to update out-of-date context */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW0
 */
struct e97_rdw_0	{
	uint32_t result_size: 8;/*!< number of result data words written to this result descriptor */
	uint32_t first_seg: 1;	/*!< first segment indicator */
	uint32_t last_seg: 1;	/*!< last segment indicator */
	uint32_t buf_ovf: 1;		/*!< buffer overflow error indicator */
	uint32_t desc_ovf: 1;		/*!< buffer overflow error indicator */
	uint32_t res: 3;			/*!< reserved */
	uint32_t buf_size: 17;	/*!< size of the output data segment buffer in bytes */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW1
 */
struct e97_rdw_1	{
	uint32_t buf_ptr;	/*!< data buffer pointer */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW2
 */
struct e97_rdw_2	{
	uint32_t err_code: 15;	/*!<  the error code (E14..E0) that is returned by the EIP-96 Packet Engine */
	uint32_t pkt_len: 17;		/*!<  the length of the output packet in bytes; \n
										excluding the appended result fields */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW3
 */
struct e97_rdw_3	{
	uint32_t L: 1;	/*!< Length field appended at the end of the packet data flag */
	uint32_t N: 1;	/*!< Next Header field appended at the end of the packet data flag */
	uint32_t C: 1;	/*!< Checksum field appended at the end of the packet data flag */
	uint32_t B: 1;	/*!< Generic Bytes field appended at the end of the packet data flag */
	uint32_t hash_len: 6;		/*!< The number of appended hash bytes at the end of the packet data. \n
										Valid when H = 1 */
	uint32_t H: 1;				/*!< Hash Byte field appended at the end of the packet data flag */
	uint32_t res: 16;			/*!< reserved */
	uint32_t E15: 1;			/*!< the error code (E15) that is returned by the EIP-96 Packet Engine */
	uint32_t bypass_len: 4;	/*!< the number of DWORDs of Bypass_Token_Words in the Result Descriptor */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW4
 */
struct e97_rdw_4 {
	uint32_t res: 8;		/*!< reserved */
	uint32_t app_id: 24;	/*!< application id */
};
/*!
  \brief This is the structure for EIP97 Result Descriptor DW5
 */
struct e97_rdw_5	{
	uint32_t res: 16;			/*!< reserved*/
	uint32_t pad_len: 8;		/*!<  number of detected (and removed) padding bytes */
	uint32_t next_hdr: 8;	/*!< next header result value from IPSec trailer */
};

/**************************************************************/
#else	// CONFIG_LITTLE_ENDIAN
/**************************************************************/
/*!
  \brief This is the structure for EIP97 Command Descriptor DW0
 */
struct e97_cdw_0	{
	uint32_t buf_size: 17;	/*!< size of input data buffer in bytes */
	uint32_t res: 5;			/*!< reserved */
	uint32_t last_seg: 1;	/*!< last segment indicator */
	uint32_t first_seg: 1;	/*!< first segment indicator */
	uint32_t acd_size: 8;	/*!< additional control data size */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW1
 */
struct e97_cdw_1 {
	uint32_t buf_ptr;	/*!< data buffer pointer */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW2
 */
struct e97_cdw_2 {
	uint32_t acd_ptr;	/*!< additional control data pointer */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW3
 */
struct e97_cdw_3	{
	uint32_t len: 17; /*!< input packet length */
	uint32_t ip: 1;	/*!< EIP97 Mode */
	uint32_t cp: 1;	/*!< context pointer 64bit */
	uint32_t ct: 1;	/*!< context type, reserved */
	uint32_t rc: 2;	/*!< reuse context */
	uint32_t too: 3;	/*!< type of output */
	uint32_t c: 1;		/*!< context control words present in token */
	uint32_t iv: 3;	/*!< IV??? */
	uint32_t u: 1;		/*!< upper layer header from token */
	uint32_t type: 2;	/*!< type of token, reserved*/
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW4
 */
struct e97_cdw_4 {
	uint32_t app_id: 24;	/*!< application id */
	uint32_t res: 8;		/*!< reserved */
};

/*!
  \brief This is the structure for EIP97 Command Descriptor DW5
 */
struct e97_cdw_5 {
	//uint32_t refresh: 2;	/*!< Refresh flags to update out-of-date context */
	uint32_t ctx_ptr/*: 30*/;	/*!< The pointer to the offset of the Context in Host memory */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW0
 */
struct e97_rdw_0	{
	uint32_t buf_size: 17;	/*!< size of the output data segment buffer in bytes */
	uint32_t res: 3;			/*!< reserved */
	uint32_t desc_ovf: 1;		/*!< buffer overflow error indicator */
	uint32_t buf_ovf: 1;		/*!< buffer overflow error indicator */
	uint32_t last_seg: 1;	/*!< last segment indicator */
	uint32_t first_seg: 1;	/*!< first segment indicator */
	uint32_t result_size: 8;/*!< number of result data words written to this result descriptor */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW1
 */
struct e97_rdw_1	{
	uint32_t buf_ptr;	/*!< data buffer pointer */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW2
 */
struct e97_rdw_2	{
	uint32_t pkt_len: 17;		/*!<  the length of the output packet in bytes; \n
										excluding the appended result fields */
	uint32_t err_code: 15;	/*!<  the error code (E14..E0) that is returned by the EIP-96 Packet Engine */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW3
 */
struct e97_rdw_3	{
	uint32_t bypass_len: 4;	/*!< the number of DWORDs of Bypass_Token_Words in the Result Descriptor */
	uint32_t E15: 1;			/*!< the error code (E15) that is returned by the EIP-96 Packet Engine */
	uint32_t res: 16;			/*!< reserved */
	uint32_t H: 1;				/*!< Hash Byte field appended at the end of the packet data flag */
	uint32_t hash_len: 6;		/*!< The number of appended hash bytes at the end of the packet data. \n
										Valid when H = 1 */
	uint32_t B: 1;	/*!< Generic Bytes field appended at the end of the packet data flag */
	uint32_t C: 1; /*!< Checksum field appended at the end of the packet data flag */
	uint32_t N: 1;	/*!< Next Header field appended at the end of the packet data flag */
	uint32_t L: 1;	/*!< Length field appended at the end of the packet data flag */
};

/*!
  \brief This is the structure for EIP97 Result Descriptor DW4
 */
struct e97_rdw_4 {
	uint32_t app_id: 24;	/*!< application id */
	uint32_t res: 8;		/*!< reserved */
};
/*!
  \brief This is the structure for EIP97 Result Descriptor DW5
 */
struct e97_rdw_5	{
	uint32_t next_hdr: 8;	/*!< next header result value from IPSec trailer */
	uint32_t pad_len: 8;		/*!<  number of detected (and removed) padding bytes */
	uint32_t res: 16;			/*!< reserved*/
};

#endif

/*!
  \brief This is the structure for Command Descriptor (CD) words.
 */
struct e97_cdw {
	struct e97_cdw_0 dw0;	/*!<Command Descriptor word 0 */
	struct e97_cdw_1 dw1;	/*!<Command Descriptor word 1 */
	struct e97_cdw_2 dw2;	/*!<Command Descriptor word 2 */
	struct e97_cdw_3 dw3;	/*!<Command Descriptor word 3 */
	struct e97_cdw_4 dw4;	/*!<Command Descriptor word 4 */
	struct e97_cdw_5 dw5;	/*!<Command Descriptor word 5 */
} ;

/*!
  \brief This is the structure for Result Descriptor (RD) words.
 */
struct e97_rdw {
	struct e97_rdw_0 dw0;	/*!<Result Descriptor word 0 */
	struct e97_rdw_1 dw1;	/*!<Result Descriptor word 1 */
	struct e97_rdw_2 dw2;	/*!<Result Descriptor word 2 */
	struct e97_rdw_3 dw3;	/*!<Result Descriptor word 3 */
	struct e97_rdw_4 dw4;	/*!<Result Descriptor word 4 */
	struct e97_rdw_5 dw5;	/*!<Result Descriptor word 5 */
} ;

/*!
  \brief This is the structure for IPSec info shared between MPE HAL and MPE FW
 */
struct ipsec_info {
	/*EIP97 specific */
	struct e97_cdw 	cd_info; /*!< EIP97 CD(Command Descriptor Information)*/
	uint8_t  cd_size; 		 /*!< Command Descriptor Size in DWORD (4 bytes). If follow this implementation proposal, its size
								    is EIP97_CD_SIZE ,*/

	/*padding variables*/
	uint8_t pad_en; 				/*!< flag whether pad needed or not. For block ciphers , it must be set to 1, otherwise 0*/
	uint8_t pad_instr_offset;	/*!< the insert instruction index in the ACD for padding. valid only if pad_en is 1.
												MPE FW need to update it accordingly if pad_en equal 1*/
	uint8_t crypto_instr_offset;/*!< the insert instruction index in the ACD for crypto to encrypt/decrypt real data */
	uint8_t blk_size; 		/*!< crypto block size for calculating padding length in bytes */

	/* Integrity Check Value (ICV) */
	uint8_t icv_len; 			/*!< ICV length in bytes. Used by FW for MTU sanity checking */

	/* instruction vectors (IV), check its source i.e from token, context record, or hardware generated randomly */
	uint8_t iv_len; 			/*!< IV length in bytes. Used by FW for MTU sanity checking */

	/*For debugging */
	uint8_t key_len; 			/*!< the key length in DWORDS */
	uint8_t key_offset; 		/*!< It is the offset based on context base. Valid only if key_len is non_zero*/
	uint8_t digest0_len; 	/*!< the digest0 length  in DWORDS */
	uint8_t digest0_offset;	/*!< It is the offset based on context base. Valid only if digest0_len is non_zero */
	uint8_t digest1_len; 	/*!< the digest1 length  in DWORDS */
	uint8_t digest1_offset; /*!< It is the offset based on context base. Valid only if digest1_len is non_zero */
	uint8_t spi_offset; 		/*!< It is the offset based on context base. */
	uint8_t seq_num_len; 	/*!< the sequence number length  in DWORDS */
	uint8_t seq_num_offset;	/*!< It is the offset based on context base. Valid only if seq_num_len is non_zero*/
	uint8_t seq_mask_len; 	/*!< the sequence mask length  in DWORDS */
	uint8_t seq_mask_offset;/*!< It is the offset based on context base. Valid only if seq_mask_len is non_zero*/
	uint8_t iv_offset; 		/*!< It is the offset based on acd_ptr or ctx_ptr (as base). Valid only if iv_len is non_zero*/

};

struct ipsec_tunl_setup {
	uint8_t id;
	uint32_t dstip[4];
	uint32_t spi;
};

/************************Below is General Configuration (Genconf ) Definition *************************************/

/** \addtogroup  MPE_GENERAL_CONFIG */
/*@{*/
/**  \brief MPE General Configuration Structure Definition. MPE HAL no need to allocate memory for this configuration */
struct genconf {
	/******************************************************************************
	  Note -- must put eva_config_flag/eva_SegCtl0/eva_SegCtl1/eva_SegCtl2
	  at the begining of the geoncof structure
	  so as assemly code can easily access it with fixed offset 0.
	 *******************************************************************************/
	uint32_t eva_config_flag;/*!< EVA Setting Flag set by MPE HAL: one bit is for one register. Its value definitiion : \n
                               &nbsp;&nbsp; 0  -- this register value not valid and no need to set.\n
                               &nbsp;&nbsp; 1  -- this register value valid and need to set \n
Note:\n
&nbsp;&nbsp; bit0 is for eva_SegCtl0 register \n
&nbsp;&nbsp; bit1 is for eva_SegCtl1 register \n
&nbsp;&nbsp; bit2 is for eva_SegCtl2 register\n
                              */

	uint32_t eva_SegCtl0;   /*!< EVA SegCtl0 for cfg0 and cfg1 if bit0 of \ref eva_config_flag is 1. It is set by MPE HAL*/
	uint32_t eva_SegCtl1;   /*!< EVA SegCtl0 for cfg0 and cfg1 if bit1 of \ref eva_config_flag is 1. It is set by MPE HAL*/
	uint32_t eva_SegCtl2;   /*!< EVA SegCtl0 for cfg0 and cfg1 if bit2 of \ref eva_config_flag is 1. It is set by MPE HAL*/
	uint32_t ddr_address;	/*!< Used to convert between PHY and VIR address in EVA mode. HW dependent. */
	uint32_t vmb_fw_msg_base[VPE_NUM]; /*!< VMB msg base address (from VMB to FW message) per VPE. It is set by MPE HAL*/
	uint32_t fw_vmb_msg_base[VPE_NUM]; /*!< FW msg base address (from FW to VMB reply message) per VPE. It is set by MPE HAL*/
	uint32_t fw_dbg_flag[2]; /*!< FW debug flags accessible by kernel. Set by MPE FW, read only to MPE HAL. */

	struct fw_hdr fw_hdr; /*!< MPE FW header. It is set during compilation or by post scripts. It is readonly to MPE FW/HAL*/

	uint32_t features_list[4];  /*!< features_list[0] bit defition as below.
                                  features_list[0]: \n
                                  &nbsp;&nbsp; bit 0: ipv4 routing, \n
                                  &nbsp;&nbsp; bit 1: ipv6 routing, \n
                                  &nbsp;&nbsp; bit 2: multicast\n
                                  &nbsp;&nbsp; For details, refer to \ref FEATURE_LIST0 \ref FEATURE_LIST1,
                                  \ref FEATURE_LIST2, \ref FEATURE_LIST3 \n.
                                  It is set during compilation and readonly to MPE FW/HAL.
                                 */
	uint32_t dbg_mpe_q_id  : 8;  /*!< Debug queue ID for MPE HAL to receive event/msg from MPE FW.\n
                                   Valid only if \ref dbg_mpe_q_id_en == 1 \n
                                   It is set by MPE HAL.
                                  */
	uint32_t dbg_mpe_q_id_en: 1; /*!< Debug queue ID enable flag: 1-\ref dbg_mpe_q_id is valid. 0-not valid\n
                                   It is set by MPE HAL.
                                  */
	uint32_t mc_cmp_mode  : 2;  /*!< Multicast compare mask as MPE Acceleration table defined.\n
                                  &nbsp;&nbsp; 0-No Mask(Normally for unicast session), \n
                                  &nbsp;&nbsp; 1-Rtng Ext Mask (Mask Routing extention), \n
                                  &nbsp;&nbsp; 2-IP only (Normally for IGMP V3) \n
                                  &nbsp;&nbsp; 3-Dest IP only (Normally for IGMP V2)\n
                                  It is set by MPE HAL.
                                 */
	uint32_t dispatch_q_buf_num: 8;  /*!< The dispatch queue's buffer number/length. Maximum up to \ref MAX_DISPATCH_BUF_NUM \n
                                       Note, the buffer is allocated in MPE FW, not by MPE HAL*/

	uint8_t fw_sess_tbl_type[MAX_CMP_TABLE];  /*!< Session table type configured by MPE HAL. \n
                                                &nbsp;&nbsp; 0 -- Not defined table type. If 0, the talbe can not be used by MPE FW\n
                                                &nbsp;&nbsp; 1 -- IPV4  \n
                                                &nbsp;&nbsp; 2 -- IPV6  \n
                                                It is set by MPE HAL and Readonly to MPE HAL/FW once MPE activated \n
                                               */
	uint8_t fw_sess_tbl_iterate[MAX_CMP_TABLE];  /*!< Max number of Iterations to be done to find a valid match for a Search Command. \n
                                                   Minimal is 1 and maximum is \ref MAX_SEARCH_ITRN.
                                                  */

	uint16_t fw_sess_tbl_num[MAX_CMP_TABLE];  /*!< Session table size (up to \ref MAX_FW_SESSION_NUM) per MPE table configured by MPE HAL. \n
                                                It is Readonly to MPE HAL/FW once MPE FW is activated  */

	/*MIB */
	uint32_t fw_sess_hit_tbl_base[MAX_CMP_TABLE][MAX_WORKER_NUM]; /*!< hit counter table base address per fw table per tc set by MPE HAL\n
                                                                    each byte represent one session hit flag: \n
                                                                    &nbsp;&nbsp; 0 no hit, \n
                                                                    &nbsp;&nbsp; 1 hit.  \n
                                                                    The session hit counter is set by MPE FW once the session is hit.
                                                                    It is cleared by PPA/MPE HAL periodically. \n
                                                                    Its memory size per table is ( fw_session_num[i]) + 1 bytes,  maximum up to ( \ref MAX_FW_SESSION_NUM) + 1 bytes per table \n
                                                                    It is per worker based in order to avoid lock between different workers.\n
                                                                   */
	uint32_t fw_sess_mib_tbl_base[MAX_CMP_TABLE][MAX_WORKER_NUM];  /*!< mib counter table base address per fw table per TC set by MPE HAL. \n
                                                                     The memory is allocated by MPE HAL.\n
                                                                     Its memory size per table is ( \ref fw_sess_tbl_num[i] * sizeof(struct \ref session_mib)) bytes. \n
                                                                     maximumum up to ( \ref MAX_FW_SESSION_NUM * sizeof(struct \ref session_mib) ) bytes per table.\n
                                                                     Maximum \ref MAX_WORKER_NUM TC are supported \n
                                                                     It is per worker based in order to avoid lock between different workers.\n
                                                                    */
	uint16_t mib_itf_num;      /*!< maximum MIB Interface configured by MPE HAL ( up to \ref MAX_MIB_ITF_NUM )\n
                                 Readonly to MPE HAL once MPE activated
                                */
	uint16_t hw_act_num;      /*!< Maximum HW session number set by MPE HAL for complemantry mode. \n
                                It is up to \ref MAX_HW_SESSION_NUM.\n
                                It should match HW PAE configuration.*/
	uint16_t dump_byte_len;	/*!< Number of bytes to dump during MPE FW acceleration debugging. Maximum is MAX_DUMP_BYTE_LEN */

	uint32_t hw_act_tbl_base;    /*!< HW session action table base for Complementary mode set by MPE HAL.\n
                                   Its memory size is ( \ref hw_act_num * sizeof(struct \ref hw_act_ptr)) bytes.\n
                                   maximumum up to ( \ref MAX_MIB_ITF_NUM * sizeof(struct \ref hw_act_ptr) )\n
                                  */
	uint32_t mib_itf_tbl_base[MAX_WORKER_NUM];  /*!< MIB interface table base address per TC set by MPE HAL.
                                                  The memory is allocated by MPE HAL.\n
                                                  Each table's memory size per table is ( its \ref mib_itf_num * sizeof(struct \ref mpe_itf_mib)) bytes. \n
                                                  It is up to ( \ref MAX_MIB_ITF_NUM * sizeof(struct \ref mpe_itf_mib) )\n
                                                  MPE FW access the base address via hw_res->logic_mpe_tc_id, ie, \ref mib_itf_tbl_base[hw_res->logic_mpe_tc_id]\n
                                                 */

	/* MPE FW Acceleration Extension */
	uint32_t fw_cmp_tbl_base[MAX_CMP_TABLE]; /*!< FW compare table base address per table set by MPE HAL\n
                                               each table size is \ref fw_sess_tbl_num[i], maximium up to \ref MAX_FW_SESSION_NUM (32K) sessions per table.\n
                                               Each compare entry size in the table can be :\n
                                               &nbsp;&nbsp; 1) sizeof(struct \ref fw_compare_hash_auto_ipv4)\n
                                               &nbsp;&nbsp; 2) sizeof(struct \ref fw_compare_hash_auto_ipv6)\n
                                               &nbsp;&nbsp; 3) sizeof(struct \ref fw_compare_hash_manual)\n
                                              */

	uint32_t max_tm_deq_pkt_num : 8;  /*!< Maximum packets can be dequeued from CBM per time before it switches to handle other events.\n
                                        It is set by MPE HAL*/
	uint32_t max_works_pkt_num : 8;  /*!< Maximum packets can be handled by worker TC before it switches to handle other events.\n
                                       It is set by MPE HAL*/

	uint32_t pmac_len_for_csum_offload: 4;	/*!< Default value is 8 to give 8 bytes offset (PMAC header length) for PAE checksum support */
	uint32_t loop_test: 1; /*!< To force input packets looped back to same GSWIP-L port without checking its real session action. \n
                             Mainly used for testing purpose in chiptest*/
	uint32_t dic_flag: 1; /*!< TX DMA descriptor DIC field to discard not-hit-by-rule packet from CPU queue. It is for testing only in chiptest */

	struct mib_tc tc_mib[MAX_MPE_TC_NUM]; /*!< per TC based rx MIB counter: \n
                                            It is accessed via hw_res->logic_mpe_tc_id
                                           */

	uint16_t mc_vap_num[MAX_PMAC_PORT]; /*!< maximum multicast session number per port set by MPE HAL, up to \ref MAX_MC_NUM. \n
                                          If it is Zero, it means no MC support for this port */
	uint32_t mc_vap_tbl_base[MAX_PMAC_PORT]; /*!< per port based Multicast VAP table base address set by MPE HAL.\n
                                               Each entry in the table is a strcut \ref vap_entry  .\n
                                               each port's VAP table's memory size is \ref mc_vap_num[i] * sizeof(struct \ref vap_entry) bytes\n
                                               each port's VAP table's memory size is up to \ref MAX_MC_NUM * sizeof(struct \ref vap_entry) bytes\n
Note: each mc session's maximum supported VAP number is \ref MAX_VAP_PER_PORT
                                              */

	struct port_info port[MAX_PMAC_PORT]; /*!< per pmac port information set by MPE HAL*/

	uint32_t buffer_free_tbl_base;   /*!<Dispatch Q list base address set by MPE HAL.\n
                                       Its memory size should be sizeof( struct \ref buffer_free_list ) * \ref MAX_DISPATCH_BUF_NUM  ) */

	/*Put it at the end of the structure for its variable size */
	struct tc_hw_res  hw_res[MAX_MPE_TC_NUM + 1];         /*!< Hardware resource per TC set by MPE HAL.\n
                                                          It is accessed via hw_res->logic_mpe_tc_id
                                                         */
	uint32_t g_mpe_dbg_enable;                        /*!< Used to enable/disable debugging message support set by MPE HAL.\n
                                                        It can be enabled/disabled on the fly*/

	uint32_t cbm_buffer_req_port;     /*!< CBM buffer request port for MPE FW to allocate a free buffer. It is set by MPE FW*/

	uint32_t eva_cfg_pa;		/*!< Physical base address according to EVA. Set by MPE FW, read only to MPE HAL.  */

	uint32_t eva_cfg_va;		/*!< Virtual base address according to EVA. Set by MPE FW, read only to MPE HAL.  */

	uint32_t tc_cur_state[MAX_MPE_TC_NUM]; /*!< TC Current state for debugging */

	uint32_t dispatch_q_cnt;	/*!< Current packets pending in dispatch Q */

	uint32_t ls_q_cnt; /*!< Load spreader Q count to avoid multiple register read */

	struct mpe_profiling mprof;	/*!< Used for MPE cycle estimation/profiling. Strictly for lab testing by MPE FW team */

	uint32_t ibl_loadaddr; /*!<  IBL loadaddr */

	uint32_t dl_dispatch_q_cnt;	/*!< Current packets pending in dispatch Q */

	uint32_t dl_ls_q_cnt; /*!< Load spreader Q count to avoid multiple register read */
	uint32_t dl_buffer_free_tbl_base;	 /*!< Dispatch Q list base address set by MPE HAL for Directlink.\n
									Its memory size should be sizeof( struct \ref buffer_free_list )* \ref MAX_DISPATCH_BUF_NUM	) */
	uint32_t max_tm_dl_deq_pkt_num : 8;  /*!< Maximum packets can be dequeued from CBM to DL per time before it switches to handle other events.\n
										  It is set by MPE HAL*/
	uint32_t dl_ep2radioId_base[MAX_EP2RADIO_ENTRY];	/*!< EP to Radio ID mapping array */
	uint32_t dl_dispatch_q_buf_num;  /*!< The dispatch queue's buffer number/length. Maximum up to \ref MAX_DISPATCH_BUF_NUM \n
														 Note, the buffer is allocated in MPE FW, not by MPE HAL*/

	uint8_t e97_init_flag;	/*!< whether EIP97 is initialized by Linux Driver or not. 1– Initialized successfully */

	uint8_t tunnel_esp_test;	/*!<  MPE FW workaround to set DEC=1, ENC=0, and tunnel_id in RX DMA descriptor : 0: real FW, 1: testing only*/
	uint8_t tunnel_redir_port;	/*!<  source port id in ingress PMAC after a tunnel ESP packet decrypted : valid id from 7 - 14 */

	/* IPSec testing tunnel setup for decryption */
	struct ipsec_tunl_setup ips_tunl[IPSEC_TUN_MAX + IPSEC_TUN_MAX];	/*!<  for MPE FW  workaround to set tunnel_id in RX DMA descriptor testing. \n
	                                                                                                            It consists 16 tunnels for IPv4 and 16 tunnels for IPv6. Only valid when \n
	                                                                                                            tunnel_esp_test = EN */

	/*IPSEC Tunnel Info */
	struct ipsec_info ipsec_input[IPSEC_TUN_MAX]; 	/*!< IPSEC Tunnel input direction info for Decryption */
	struct ipsec_info ipsec_output[IPSEC_TUN_MAX];	/*!< IPSEC Tunnel output direction info for Encryption */

	/*IPSEC Tunnel Flag */
	uint8_t ipsec_input_flag[IPSEC_TUN_MAX]; 	/*!< Tunnel flag: 0 –its configuration not valid, 1 –valid 2—For MPE use */
	uint8_t ipsec_output_flag[IPSEC_TUN_MAX]; /*!< Tunnel flag: 0 –its configuration not valid, 1 --valid */

	/*For MPE HAL to construct CDR/RDR. It is Per Worker based. MPE HAL need to allocate the buffer */
	uint32_t e97_cdr_in[MAX_WORKER_NUM][IPSEC_TUN_MAX];	/*!< MPE HAL alloc per CDR input buffer EIP97_CD_SIZE * 4 */
	uint32_t e97_cdr_out[MAX_WORKER_NUM][IPSEC_TUN_MAX];	/*!< MPE HAL alloc per CDR input buffer EIP97_CD_SIZE * 4. */;
	uint32_t e97_acd_in[MAX_WORKER_NUM][IPSEC_TUN_MAX]; 	/*!< MPE HAL alloc per ACD input buffer EIP97_ACD_MAX_SIZE */
	uint32_t e97_acd_out[MAX_WORKER_NUM][IPSEC_TUN_MAX]; 	/*!< MPE HAL alloc per ACD output buffer EIP97_ACD_MAX_SIZE */
	uint32_t e97_rdr[MAX_WORKER_NUM]; 							/*!< MPE HAL alloc per RDR buffer e97_rdw)  */;

} ;

/*!
  \brief Structure for Directlink driver address
 */
struct dl_drv_address_map {
	uint32_t dl_buf_info_addr;	/*!< Directlink buffer info address */
	uint32_t dl_ep_2radio_addr;	/*!< Directlink EP to radio id address */
};

/*@}*/ /* MPE_GENERAL_CONFIG */

/** \addtogroup  VMB_FW_INTERFACE */
/*@{*/
/**  \brief MPE FW Header File Structure */
#define MAX_IPI_TC_NUM  4  /*!< Maximum TC_NUM supported in VMB Interface*/

/**  \brief VMB Message ID*/
enum MSG_ID {
	VMB_CPU_START = 0x1, /*!< Start the firmware in the VPE/CPU */
	VMB_CPU_STOP = 0x2, /*!< Stop the firmware in the VPE/CPU */
	VMB_TC_START = 0x4, /*!< Start the firmware for the TC */
	VMB_TC_STOP = 0x8,  /*!< Start the firmware for the TC */
	VMB_TC_PAUSE = 0x10, /*!< Start the firmware for the TC */
	VMB_TC_RESUME = 0x20 /*!< Start the firmware for the TC */
};

/**  \brief VMB ACK Status */
enum FW_ACK_STS {
	FW_VMB_ACK = 0x1,  /*!< ACK */
	FW_VMB_NACK = 0x2, /*!< NACK */
	FW_RESET = 0x4,    /*!< RESET */
	IBL_IN_WAIT = 0x8, /*!< IN_WAIT */
	FW_VMB_PRIV_INFO = 0x10 /*!< PRIV_INFO */
};

/* ERROR codes */
#define VMB_SUCCESS     1  /*!< VMB error code: success */
#define VMB_ERROR       2  /*!< VMB error code: ERROR */
#define VMB_EBUSY       3  /*!< VMB error code: BUSY */
#define VMB_EAVAIL      4  /*!< VMB error code: EAVAIL */
#define VMB_ERESET      5  /*!< VMB error code: ERESET */
#define VMB_ETIMEOUT    6  /*!< VMB error code: ETIMEOUT */
#define VMB_ENACK       7  /*!< VMB error code: ETIMEOUT */
#define VMB_ENOPRM      8  /*!< VMB error code: ENOPRM */

/**  \brief CPU Launch structure */
struct cpu_launch_t {
	uint32_t start_addr;   /*!< Start Address of the firmware */
	uint32_t sp;           /*!< stack pointer */
	uint32_t gp;           /*!< global pointer */
	uint32_t a0;           /*!< argument that needs to be passed to firmware/Linux secondary CPU */
	uint32_t eva;          /*!< indicates whether firmware will run in legacy or in EVA mode */
	uint32_t mt_group;     /*!< mt_group */
	uint32_t yield_res;    /*!< Initial State of the TC. For MPE FW, it is pointer to its HW_RES */
	uint32_t priv_info;          /*!< reserve */
} ;

/**  \brief TC Launch structure */
struct tc_Launch_t {
	uint32_t tc_num;       /*!< Number of the TC on which firmware needs to be started */
	uint32_t mt_group;     /*!< MT priority group number to which the TC should be assigned to  */
	uint32_t start_addr;   /*!< Start Address of the firmware */
	uint32_t sp;           /*!< stack pointer */
	uint32_t gp;           /*!< global pointer */
	uint32_t a0;           /*!< argument that needs to be passed to firmware/Linux secondary CPU */
	uint32_t state;         /*!< Initial State of the TC */
	uint32_t priv_info;  /*!< Point to struct tc_hw_res */
} ;

/**  \brief Data structure used to pass messages from VMB to FW*/
struct vmb_fw_msg_t {
	uint32_t msg_id; /*!< Message Identifier for interAptiv-BL or firmware
                       0x1- VMB_CPU_START - Start the firmware using the information in
                       CPU_Launch structure
                       0x2 - VMB_CPU_STOP - Stop the firmware graciously. This shall stop
                       all TCs that are associated with the CPU.
                       0x4 - VMB_TC_START - Start the TC using the information in the
                       TC_Launch structure
                       0x8 - VMB_TC_STOP - Stop the firmware running on the TC. TCNum
                       contains the number of the TC to be stopped
                       0x10 - VMB_TC_PAUSE - Halt the firmware running on the TC
                       0x20 - VMB_TC_RESUME - Resume the firmware that is halted
                       previously.
                       0x40 - VMB_FW_PRIV_INFO - Use to exchange just the private
                       nformation from driver to the corresponding firmware via VMB (either
                       the priv_info field in cpu_launch_t or in tc_launch_t can be used). */
	struct cpu_launch_t cpu_launch; /*!< Data structure that contains all info to start a firmware on a CPU */
	struct tc_Launch_t  tc_launch[MAX_IPI_TC_NUM]; /*!< Data structure that contains all info to start a firmware on a TC.
                                                     MAX_TCS is 4 since 1 TC is always default on both VPEs, and we have
                                                     6TCs are there per core */
	uint32_t tc_num;  /*!< Number of the TC to be stopped/paused/resumed */
} ;

/*!
  \brief Data Structure used to pass messages from firmware to VMB
 */
struct fw_vmb_msg_t {
	uint32_t status; /*!< Status Information passed from firmware/interAptiv-BL to VMB
                       0x1 - FW_VMB_ACK - request was handled successfully
                       0x2 - FW_VMB_NACK - request handling was unsuccessful
                       0x4 - FW_RESET - firmware is restarting either due to NMI reset or
                       some internal issue
                       0x8 - IBL_IN_WAIT - interAptiv-BL is initialised and is waiting for
                       requests from VMB
                       0x10 - FW_VMB_PRIV_INFO - additional information available in
                       priv_info */
	uint32_t priv_info; /*!< Additional info that can be communicated by firmware to corresponding
                          driver on linux */
} ;

/*@}*/ /* VMB_FW_INTERFACE */

#endif  /*__LTQ_MPE_API_H__201308_05_1913__ */
