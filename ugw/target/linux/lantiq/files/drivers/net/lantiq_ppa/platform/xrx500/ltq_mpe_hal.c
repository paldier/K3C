/*******************************************************************************
 **
 ** FILE NAME    : ltq_mpe_hal.c
 ** PROJECT      : MPE HAL
 ** MODULES      : MPE (Routing/Bridging Acceleration )
 **
 ** DATE         : 20 Mar 2014
 ** AUTHOR       : Purnendu Ghosh
 ** DESCRIPTION  : MPE HAL Layer
 ** COPYRIGHT    :              Copyright (c) 2009
 **                          Lantiq Deutschland GmbH
 **                   Am Campeon 3; 85579 Neubiberg, Germany
 **
 **   For licensing information, see the file 'LICENSE' in the root folder of
 **   this software module.
 **
 ** HISTORY
 ** $Date        $Author                $Comment
 ** 20 Mar 2014  Purnendu Ghosh         Initiate Version
 *******************************************************************************/


/*
 *  Common Header File
 */
//#include <linux/autoconf.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/inet.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 8, 13)
#include <net/ipip.h>
#else
#include <lantiq.h>
#include <lantiq_soc.h>
#include <linux/clk.h>
#include <net/ip_tunnels.h>
#endif
//#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/in.h>
#include <asm/uaccess.h>
#include <net/ip6_tunnel.h>
#include <net/ipv6.h>


/*
 *  Chip Specific Head File
 */
#include <net/ppa_api.h>
#include "../ppa_datapath.h"
#include <net/ppa_ppe_hal.h>
#include <net/ltq_mpe_api.h>
#include "mpe_debug_hdr.h"
#include "ltq_mpe_hal.h"
//#include "mpe_fw_be.h"
#include <asm/ltq_vmb.h>
#include <asm/ltq_itc.h>
#include <asm/gic.h>
#include <net/lantiq_cbm.h>
#include <net/lantiq_cbm_api.h>
#include <net/datapath_api.h>

#include <linux/platform_device.h>
#include <linux/of.h>
#include "mpe_fw_ver.h"

//#define CONFIG_LTQ_PPA_MPE_IP97
#ifdef CONFIG_LTQ_PPA_MPE_IP97
#include "ltq_tmu_hal_dp_connectivity.h"
#include <net/datapath_api.h>
#include <net/lantiq_cbm.h>
#include <net/esp.h>
#include <uapi/linux/xfrm.h>
extern ppa_tunnel_entry*        g_tunnel_table[MAX_TUNNEL_ENTRIES];
#include <crypto/ltq_ipsec_ins.h>
#endif
#define FIRMWARE   "mpe_fw_be.img" //Firmware Name
#define FIRMWARE_REQUEST 1

//#define MPE_HAL_TEST
//#define NO_FW_HDR 
#define dbg printk
#define MAX_VPE_NUM                    2      /*!< Maximum number VPE's that MPE FW supports */
#define MPE_TC_STACK_SIZE           0x4000    /*!< TC Stack pointer size */
#define MPE_FILE_PATH "/opt/lantiq/bin/"
#define MPE_PAGE_SIZE  0x1000
#define VIR_TO_PHY(addr) ((addr & 0x7FFFFFFF) | 0x20000000)


//#define GIC_SH_WEDGE1 0x0280
#define GIC_BASE_ADDR1         GIC_BASE_ADDR | KSEG1  //0xb2320000  // KSEG0 address address of the GIC
#define GIC_SH_WEDGE_REG   *((volatile u32*) (GIC_SH_WEDGE_OFS | GIC_BASE_ADDR1))
//#define GIC_SH_MAP0_VPE31_0 0x2000
//#define GIC_SH_MAP_SPACER 0x20
#define MPE_HAL_NO_FREE_ENTRY 0x8000
#define MAX_CMP_TABLE_SUPPORTED 2
#define MAX_ENTRY_ERROR 0x0001

//#define GIC_SH_WEDGE_REG   *((volatile u32*) (0xb2320280))

extern void mpe_hal_proc_destroy(void);
extern void mpe_hal_proc_create(void);

//MPE version
char *g_mpe_version;

struct platform_device *pdev;
const struct firmware *fw_entry;

int32_t sem_id_log[MAX_ITC_SEMID]={0};
#ifdef CONFIG_LTQ_PPA_MPE_IP97
#define MAX_RING 4
int32_t ring_id_pool[MAX_RING] = {-1}; 
#endif

struct tlb_auguments tlb_info __attribute__((aligned(4)));
uint32_t g_MPELaunch_CPU=0;
char * g_MpeFw_load_addr = NULL;
char * g_MpeFw_stack_addr = NULL;
//uint32_t g_MpeFw_load_addr = NULL;
int32_t gImage_size=0;
struct genconf *g_GenConf = NULL;
struct fw_hdr g_MpeFwHdr; 
struct buffer_free_list dispatch_buffer[MAX_DISPATCH_BUF_NUM];
#if 0
extern int (*mpe_hal_feature_start_fn)(
                enum MPE_Feature_Type mpeFeature,
                uint32_t port_id,
                uint32_t * featureCfgBase,
                uint32_t flags);
#endif
//#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
struct buffer_free_list dl_dispatch_buffer[MAX_DISPATCH_BUF_NUM];
uint32_t dl_free_list_semid; /*!<MIPS itc semaphore address for free list of DMA descriptor */
uint32_t dl_disp_q_cnt_semid; /*!< MIPS itc semaphore address for buffer allocation from cbm */
//#endif
uint32_t display_action=0;
static int session_counter = 0;

uint8_t logic_tc_mapping[MAX_MPE_TC_NUM];
char str[INET6_ADDRSTRLEN];

uint32_t g_dl1_pmac_port;
uint32_t g_MPE_accl_mode;
typedef enum
{
	TMASK0=0,   // No mask applied, all fields are used to calculate hash 
	TMASK1,     // Mask routing extension
	TMASK2,     // Mask ports and routing extension
	TMASK3      // Mask source IP, ports and routing extension
} eTMASK;

uint32_t g_HAL_State =0;
typedef enum 
{
	MPE_HAL_FW_NOT_LOADED = 0,
	MPE_HAL_FW_LOADED,
	MPE_HAL_FW_RESOURCE_ALLOC,
	MPE_HAL_FW_RUNNING
} e_HAL_FW_STATE;

typedef struct
{
	uint8_t ucState; /*!<The VPE state, 0 - VPE_FREE 1 - VPE_RUNNING */
	uint8_t ucActualVpeNo;  
	uint8_t ucNoOfAssocTc;  
} mpe_hal_vpe_t;

typedef struct
{
	mpe_hal_vpe_t  vpe[MAX_VPE_NUM];
} mpe_hal_vpe_info_t;

mpe_hal_vpe_info_t g_VpeInfo;

typedef struct 
{
	uint32_t mpe_tc_id;
} mpe_hal_dl_tc_info;

mpe_hal_dl_tc_info dl_tc_info[AC_RADIO_NUMBER];

#define get_tm_from_vpe(vpe)        (vpe % 2)

extern uint32_t ppa_drv_generic_hal_register(uint32_t hal_id, ppa_generic_hook_t generic_hook);
extern void ppa_drv_generic_hal_deregister(uint32_t hal_id);

extern uint32_t ppa_drv_register_cap(PPA_API_CAPS cap, uint8_t wt, PPA_HAL_ID hal_id);
extern uint32_t ppa_drv_deregister_cap(PPA_API_CAPS cap, PPA_HAL_ID hal_id);

//extern void mpe_tlb_setting_init(uint32_t physical_addr, uint32_t v_addr, uint32_t pages, struct tlb_auguments *tlb);
//extern void mpe_tlb_set(struct tlb_auguments *tlb);

static int mpe_hal_load_fw(char* filename);
static int mpe_hal_run_fw(uint8_t ucCpu, uint8_t ucNum_worker);
static int mpe_hal_stop_fw(uint8_t ucCpu);

int32_t mpe_hal_add_routing_entry(PPE_ROUTING_INFO *route);
int32_t mpe_hal_del_routing_entry(PPE_ROUTING_INFO *route);
uint32_t mpe_hal_add_hw_session(PPE_ROUTING_INFO *route);
uint32_t mpe_hal_del_hw_session(PPE_ROUTING_INFO *route);

int32_t mpe_hal_add_wan_mc_entry(PPE_MC_INFO *mc);
int32_t mpe_hal_del_wan_mc_entry(PPE_MC_INFO *mc);

#ifdef CONFIG_LTQ_PPA_MPE_IP97
int32_t mpe_hal_add_ipsec_tunl_entry(PPE_ROUTING_INFO *route);
int32_t mpe_hal_del_ipsec_tunl_entry(PPE_ROUTING_INFO *route);

int32_t mpe_hal_dump_ipsec_tunnel_info(int32_t tun_id);
int32_t mpe_hal_alloc_cdr_rdr(void);
int32_t mpe_hal_free_cdr_rdr(void);

int32_t mpe_hal_get_ipsec_tunnel_mib(IPSEC_TUNNEL_MIB_INFO *route);
#endif

int32_t mpe_hal_test_and_clear_hit_stat(PPE_ROUTING_INFO *route);
int32_t mpe_hal_get_session_acc_bytes(PPE_ROUTING_INFO *route);

static int32_t mpe_hal_deregister_caps(void);
/*================================================================================================ */

static int32_t mpe_hal_generic_hook(PPA_GENERIC_HOOK_CMD cmd, void *buffer, uint32_t flag)
{
    //printk("mpe_hal_generic_hook cmd %d\n", cmd );
    switch (cmd)  {
    case PPA_GENERIC_HAL_INIT: //init HAL
        {
          uint32_t res = PPA_SUCCESS;	

	  if((res = ppa_drv_register_cap(SESS_IPV4, 2, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability SESS_IPV4!!!\n");	
		//goto P_HAL_FAIL;
	  }
          if((res = ppa_drv_register_cap(SESS_IPV6, 2, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability SESS_IPV6!!!\n");	
		//goto P_HAL_FAIL;
	  }
          if((res = ppa_drv_register_cap(TUNNEL_L2TP_US, 1, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability TUNNEL_L2TP_US,!!!\n");	
		//goto P_HAL_FAIL;
	  }
	  if((res = ppa_drv_register_cap(TUNNEL_GRE_US, 1, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability TUNNEL_L2TP_US,!!!\n");	
		//goto P_HAL_FAIL;
	  }
          if((res = ppa_drv_register_cap(SESS_MC_DS_VAP, 1, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability SESS_MC_DS_VAP,!!!\n");	
		//goto P_HAL_FAIL;
	  }
#ifdef CONFIG_LTQ_PPA_MPE_IP97
	  if((res = ppa_drv_register_cap(TUNNEL_IPSEC_US, 1, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability TUNNEL_IPSEC_US!!!\n");	
		//goto P_HAL_FAIL;
	  }
          if((res = ppa_drv_register_cap(TUNNEL_IPSEC_DS, 1, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability TUNNEL_IPSEC_US!!!\n");	
		//goto P_HAL_FAIL;
	  }
          if((res = ppa_drv_register_cap(TUNNEL_IPSEC_MIB, 1, MPE_HAL)) != PPA_SUCCESS) {
		printk("ppa_drv_register_cap returned failure for capability TUNNEL_IPSEC_MIB!!!\n");	
		//goto P_HAL_FAIL;
	  }
#endif
		printk("Init Success\n");
            return PPA_SUCCESS;
        }

    case PPA_GENERIC_HAL_EXIT: //EXIT HAL
        {
            return mpe_hal_deregister_caps();
        } 
    
    case PPA_GENERIC_HAL_GET_HAL_VERSION:
        {
            PPA_VERSION *v = (PPA_VERSION *)buffer;
            strcpy(v->version, "1.0.1"); 
	    //v->family =
	    //v->type =
	    //v->itf =
	    //v->mode =
	    //v->major = g_MpeFwHdr.hdr_ver_maj;
	    //v->mid = g_MpeFwHdr.hdr_ver_mid;
	    //v->minor = g_MpeFwHdr.hdr_ver_min;
            return PPA_SUCCESS;
        }

    case PPA_GENERIC_HAL_GET_PPE_FW_VERSION:
        {
            PPA_VERSION *v=(PPA_VERSION *)buffer;
			   
			//v->index =
            v->family = g_GenConf->fw_hdr.family;
            v->type = g_GenConf->fw_hdr.package;
            v->major = g_GenConf->fw_hdr.v_maj;
	    v->mid = g_GenConf->fw_hdr.v_mid;
	    v->minor = g_GenConf->fw_hdr.v_min         ;
            return PPA_SUCCESS;
        }  

    case PPA_GENERIC_HAL_ADD_ROUTE_ENTRY:
        {
            PPE_ROUTING_INFO *route=(PPE_ROUTING_INFO *)buffer;
	    return mpe_hal_add_routing_entry(route);

        }

    case PPA_GENERIC_HAL_DEL_ROUTE_ENTRY:
        {
            PPE_ROUTING_INFO *route=(PPE_ROUTING_INFO *)buffer;
            return mpe_hal_del_routing_entry( route );
        }

    case PPA_GENERIC_HAL_ADD_COMPLEMENT_ENTRY:
        {
            PPE_ROUTING_INFO *route=(PPE_ROUTING_INFO *)buffer;
	    return mpe_hal_add_hw_session(route);

        }
    case PPA_GENERIC_HAL_DEL_COMPLEMENT_ENTRY:
        {
            PPE_ROUTING_INFO *route=(PPE_ROUTING_INFO *)buffer;
	    return mpe_hal_del_hw_session(route);

        }
	case PPA_GENERIC_HAL_ADD_MC_ENTRY:
        {

            PPE_MC_INFO *mc = (PPE_MC_INFO *)buffer;

            return mpe_hal_add_wan_mc_entry(mc);
        }

    case PPA_GENERIC_HAL_DEL_MC_ENTRY:
        {
            PPE_MC_INFO *mc = (PPE_MC_INFO *)buffer;

            mpe_hal_del_wan_mc_entry(mc);
            return PPA_SUCCESS;
        }


    case PPA_GENERIC_HAL_GET_ITF_MIB:
        {
            //PPE_ITF_MIB_INFO *mib=(PPE_ITF_MIB_INFO *)buffer;
            //get_itf_mib( mib->itf, &mib->mib);
            return PPA_SUCCESS;
        }

    case PPA_GENERIC_HAL_TEST_CLEAR_ROUTE_HIT_STAT:  //check whether a routing entry is hit or not
        {
            PPE_ROUTING_INFO *route=(PPE_ROUTING_INFO *)buffer;
            mpe_hal_test_and_clear_hit_stat(route);
            return PPA_SUCCESS;
        }
    case PPA_GENERIC_HAL_GET_ROUTE_ACC_BYTES:  //Get accelerated bytes of the session
        {
            PPE_ROUTING_INFO *route=(PPE_ROUTING_INFO *)buffer;
            mpe_hal_get_session_acc_bytes(route);
            return PPA_SUCCESS;
        }
#ifdef CONFIG_LTQ_PPA_MPE_IP97
    case PPA_GENERIC_HAL_GET_IPSEC_TUNNEL_MIB:  //Get accelerated bytes of the session
        {
            IPSEC_TUNNEL_MIB_INFO *mib =(IPSEC_TUNNEL_MIB_INFO *)buffer;
            mpe_hal_get_ipsec_tunnel_mib(mib);
            return PPA_SUCCESS;
        }
#endif

    default:
        printk("ppa_hal_generic_hook not support cmd 0x%x\n", cmd );
        return PPA_FAILURE;
    }

    return PPA_FAILURE;
}


/* ======================================================================================================= */
char *inet_ntoa(u32 in)
{
    static char b[18];
    register char *p;

    p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
    (void)snprintf(b, sizeof(b),
            "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
    return (b);
}

static char * inet_ntop4(const unsigned char *src, char *dst, u32 size) {
    static const char fmt[] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];
    int l;

    l = snprintf(tmp, sizeof(tmp), fmt, src[0], src[1], src[2], src[3]);
    if (l <= 0 ||  l >= size) {
        return (NULL);
    }
    strlcpy(dst, tmp, size);
    return (dst);
}

static char * inet_ntop6(const unsigned char *src, char *dst, u32 size) {
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
    struct { int base, len; } best, cur;
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2
    u_int words[NS_IN6ADDRSZ / NS_INT16SZ];
    int i;

    memset(words, '\0', sizeof words);
    for (i = 0; i < NS_IN6ADDRSZ; i++)
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    best.len = 0;
    cur.base = -1;
    cur.len = 0;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
        if (words[i] == 0) {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        } else {
            if (cur.base != -1) {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1) {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;
    tp = tmp;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base &&
                i < (best.base + best.len)) {
            if (i == best.base)
                *tp++ = ':';
            continue;
        }	
        if (i != 0)
            *tp++ = ':';
        if (i == 6 && best.base == 0 && (best.len == 6 ||
                    (best.len == 7 && words[7] != 0x0001) ||
                    (best.len == 5 && words[5] == 0xffff))) {
            if (!inet_ntop4(src+12, tp, sizeof tmp - (tp - tmp)))
                return (NULL);
            tp += strlen(tp);
            break;
        }
        tp += sprintf(tp, "%x", words[i]);
    }
    if (best.base != -1 && (best.base + best.len) ==
            (NS_IN6ADDRSZ / NS_INT16SZ))
        *tp++ = ':';
    *tp++ = '\0';
    if ((tp - tmp) > size) {
        return (NULL);
    }
    strcpy(dst, tmp);
    return (dst);
}


uint16_t mpe_hal_crcmsb(const uint8_t *data, unsigned int len) 
{
    uint16_t crc = 0;
    int i;
    if (len) do {
    crc ^= ((*data)<<8);
    data++;
    for (i=0; i<8; i++) {
        if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
        else crc <<= 1;
        crc=crc&0xFFFF;
        }
    } while (--len);
    return crc;
}

uint16_t mpe_cal_hash(struct ipv4_hash_auto_key *p4, struct ipv6_hash_auto_key *p6, eTMASK tm) 
{
    uint8_t  b[13];
    uint16_t b2 = 0;
    uint32_t b4, b6[4];
    uint16_t crc;
        
    memset(b, 0, sizeof (b));
    // Extract the structure into bytes
    // Arrange MSB first for CRC16 calculation
#ifdef CONFIG_BIG_ENDIAN // tested with EB mode MIPS
    if (p4) {
        b4 = (((TMASK0==tm) || (TMASK1==tm) || (TMASK2==tm)) ? p4->srcip:0x00000000);
        //if(TMASK3==tm) p->srcip.ip = 0;
        //b4 = p->srcip.ip;
        b[0]  = ((uint8_t *)&b4)[0]; // (b4 & 0xff000000)>>24;
        b[1]  = ((uint8_t *)&b4)[1]; // (b4 & 0x00ff0000)>>16;
        b[2]  = ((uint8_t *)&b4)[2]; // (b4 & 0x0000ff00)>> 8;
        b[3]  = ((uint8_t *)&b4)[3]; // (b4 & 0x000000ff)>> 0;

        b4 = p4->dstip;
        b[4]  = ((uint8_t *)&b4)[0];
        b[5]  = ((uint8_t *)&b4)[1];
        b[6]  = ((uint8_t *)&b4)[2];
        b[7]  = ((uint8_t *)&b4)[3]; 
    } else if (p6) {
        //b4 = (((TMASK0==tm) || (TMASK1==tm) || (TMASK2==tm)) ? 
        //        b6[0]^b6[1]^b6[2]^ b6[3] : 0x00000000)

        b6[0] = ((TMASK3==tm) ? 0: p6->srcip[0]);
        b6[1] = ((TMASK3==tm) ? 0: p6->srcip[1]);
        b6[2] = ((TMASK3==tm) ? 0: p6->srcip[2]);
        b6[3] = ((TMASK3==tm) ? 0: p6->srcip[3]);
        b4    = b6[0] ^ b6[1] ^ b6[2] ^ b6[3];
        b[0]  = ((uint8_t *)&b4)[0];
        b[1]  = ((uint8_t *)&b4)[1];
        b[2]  = ((uint8_t *)&b4)[2];
        b[3]  = ((uint8_t *)&b4)[3];

        b6[0] = p6->dstip[0];
        b6[1] = p6->dstip[1];
        b6[2] = p6->dstip[2];
        b6[3] = p6->dstip[3];
        b4    = b6[0] ^ b6[1] ^ b6[2] ^ b6[3];
        b[4]  = ((uint8_t *)&b4)[0];
        b[5]  = ((uint8_t *)&b4)[1];
        b[6]  = ((uint8_t *)&b4)[2];
        b[7]  = ((uint8_t *)&b4)[3];
    } 

    if(p4)      b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p4->srcport:0x0000);
    else if(p6) b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p6->srcport:0x0000);
    //if(((TMASK2==tm) || (TMASK3==tm))) p->srcport = 0;
    //b2 = p->srcport;                         
    b[8]  = ((uint8_t *)&b2)[0];
    b[9]  = ((uint8_t *)&b2)[1];

    if(p4)      b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p4->dstport:0x0000); 
    else if(p6) b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p6->dstport:0x0000);  
    //if((TMASK2==tm) || (TMASK3==tm)) p->dstport = 0;
    //b2 = p->dstport;
    b[10] = ((uint8_t *)&b2)[0];
    b[11] = ((uint8_t *)&b2)[1];    

    //b1 = ((TMASK0==tm) ? p->extn:0x00);
    //if (TMASK0!=tm) p->extn &= 1; // FLAGS_UDP is unmaskable
    if(p4)      b[12] = ((TMASK0==tm) ? p4->extn:(p4->extn &= 1)); 
    else if(p6) b[12] = ((TMASK0==tm) ? p6->extn:(p6->extn &= 1)); 

#else    
    if (p4) {
        b4 = (((TMASK0==tm) || (TMASK1==tm) || (TMASK2==tm)) ? p4->srcip:0x00000000);
        //if(TMASK3==tm) p->srcip.ip = 0;
        //b4 = p->srcip.ip;
        b[0]  = ((uint8_t *)&b4)[3]; // (b4 & 0xff000000)>>24;
        b[1]  = ((uint8_t *)&b4)[2]; // (b4 & 0x00ff0000)>>16;
        b[2]  = ((uint8_t *)&b4)[1]; // (b4 & 0x0000ff00)>> 8;
        b[3]  = ((uint8_t *)&b4)[0]; // (b4 & 0x000000ff)>> 0;

        b4 = p4->dstip;
        b[4]  = ((uint8_t *)&b4)[3];
        b[5]  = ((uint8_t *)&b4)[2];
        b[6]  = ((uint8_t *)&b4)[1];
        b[7]  = ((uint8_t *)&b4)[0]; 
    } else if (p6) {
        //b4 = (((TMASK0==tm) || (TMASK1==tm) || (TMASK2==tm)) ? 
        //        b6[0]^b6[1]^b6[2]^ b6[3] : 0x00000000)

        b6[0] = ((TMASK3==tm) ? 0: p6->srcip[0]);
        b6[1] = ((TMASK3==tm) ? 0: p6->srcip[1]);
        b6[2] = ((TMASK3==tm) ? 0: p6->srcip[2]);
        b6[3] = ((TMASK3==tm) ? 0: p6->srcip[3]);
        b4    = b6[0] ^ b6[1] ^ b6[2] ^ b6[3];
        b[0]  = ((uint8_t *)&b4)[3];
        b[1]  = ((uint8_t *)&b4)[2];
        b[2]  = ((uint8_t *)&b4)[1];
        b[3]  = ((uint8_t *)&b4)[0];

        b6[0] = p6->dstip[0];
        b6[1] = p6->dstip[1];
        b6[2] = p6->dstip[2];
        b6[3] = p6->dstip[3];
        b4    = b6[0] ^ b6[1] ^ b6[2] ^ b6[3];
        b[4]  = ((uint8_t *)&b4)[3];
        b[5]  = ((uint8_t *)&b4)[2];
        b[6]  = ((uint8_t *)&b4)[1];
        b[7]  = ((uint8_t *)&b4)[0];
    } 

    if(p4)      b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p4->srcport:0x0000);
    else if(p6) b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p6->srcport:0x0000);
    //if(((TMASK2==tm) || (TMASK3==tm))) p->srcport = 0;
    //b2 = p->srcport;                         
    b[8]  = ((uint8_t *)&b2)[1];
    b[9]  = ((uint8_t *)&b2)[0];

    if(p4)      b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p4->dstport:0x0000); 
    else if(p6) b2 = (((TMASK0==tm) || (TMASK1==tm)) ? p6->dstport:0x0000);  
    //if((TMASK2==tm) || (TMASK3==tm)) p->dstport = 0;
    //b2 = p->dstport;
    b[10] = ((uint8_t *)&b2)[1];
    b[11] = ((uint8_t *)&b2)[0];    

    //b1 = ((TMASK0==tm) ? p->extn:0x00);
    //if (TMASK0!=tm) p->extn &= 1; // FLAGS_UDP is unmaskable
    if(p4)      b[12] = ((TMASK0==tm) ? p4->extn:(p4->extn &= 1)); 
    else if(p6) b[12] = ((TMASK0==tm) ? p6->extn:(p6->extn &= 1)); 
#endif    
    crc = mpe_hal_crcmsb(b, sizeof(b));
    return (crc);
}


uint16_t mpe_hal_calculate_hash(void *pKey, uint32_t type)
{
	uint32_t len;

        if(type == 0) {
		len = sizeof(struct ipv4_hash_auto_key);
	} else
		len = sizeof(struct ipv6_hash_auto_key);

	mpe_cal_hash(pKey, NULL, 0); 
	//return mpe_hal_crcmsb(pKey, len);
	return PPA_SUCCESS;
}



uint32_t mpe_hal_find_free_index(void *pCmpTbl,uint32_t type)
{
        uint32_t count=0;
        struct fw_compare_hash_auto_ipv4 *pV4;
        struct fw_compare_hash_auto_ipv6 *pV6;
        if(type == 0){
		while(count < g_GenConf->fw_sess_tbl_num[0])
        	{
                	pV4  = (struct fw_compare_hash_auto_ipv4 *)(pCmpTbl + (count * sizeof(struct fw_compare_hash_auto_ipv4))); 
                	if(pV4->valid == 0) {
			//	printk("<%s> free index found : %d\n",__FUNCTION__,count);
                        	return count;
                	}
                	count++;
        	}
	}else {
		while(count < g_GenConf->fw_sess_tbl_num[1])
        	{
                	pV6  = (struct fw_compare_hash_auto_ipv6 *)(pCmpTbl + (count * sizeof(struct fw_compare_hash_auto_ipv6))); 
                	if(pV6->valid == 0) {
                        	return count;
                	}
                	count++;
        	}
	}
        
        return MPE_HAL_NO_FREE_ENTRY;
}

static inline void dump_pkt(u32 len, char *pData){
        int i;
        for(i=0;i<len;i++){
                printk("%2.2x ",(u8)(pData[i]));
                if (i % 16 == 15)
                        printk("\n");
        }
        printk("\n");
}



uint32_t mpe_hal_add_session_ipv4(struct fw_compare_hash_auto_ipv4 *pIp4CmpTbl, struct ipv4_hash_auto_key *p_key, uint32_t sess_action)
{
	uint32_t free_index=0;
	uint32_t count=0;
	uint32_t current_index;
	uint32_t hash_index =0;
	uint32_t tmp;
	struct fw_compare_hash_auto_ipv4  *temp_ptr;

//	dump_pkt(sizeof(struct ipv4_hash_auto_key), (char *)p_key);
	hash_index = mpe_cal_hash(p_key, NULL, 0); 
	//hash_index = mpe_hal_calculate_hash((void *)p_key, 0);
//	printk("\n CRC Calculation %x\n",hash_index);
	hash_index &= (g_GenConf->fw_sess_tbl_num[0] - 1);
//	printk("\n<%s> Hash Index %d\n",__FUNCTION__,hash_index);
	
#if 0
	int32_t i=0;
	for(i = 0; i < g_GenConf->fw_sess_tbl_num[0]; i++ ) {
		printk("Next Entry %d address 0x%x\n",i,((pIp4CmpTbl + (i ))));
		printk("Valid: %d\n",(pIp4CmpTbl + (i ))->valid);
		printk("Next Pointer: %d\n",(pIp4CmpTbl + (i ))->nxt_ptr);
		printk("First Pointer: %d\n",(pIp4CmpTbl + (i ))->first_ptr);
		printk("Action Pointer: %d\n",(pIp4CmpTbl + (i ))->act);
		printk("====================\n");
	}

#endif
	
	if(pIp4CmpTbl == NULL){
		printk("No IPV4 Compare Table is allocated\n");
		return PPA_FAILURE;
	}
	if(((struct session_action *)(sess_action)) == NULL){
		//printk("No Session Action Pointer\n");
		return PPA_FAILURE;
	}

        tmp  = ((pIp4CmpTbl + (hash_index))->first_ptr);
	while(tmp != (pIp4CmpTbl + tmp)->nxt_ptr)
        {
                tmp=(pIp4CmpTbl + tmp)->nxt_ptr;
                count++;
        }
//	printk("<%s>Iteration count : %d\n",__FUNCTION__,count);
	if( count > MAX_SEARCH_ITRN ) {
                printk("\nNumber of entries for hash=%X is %d \n", hash_index, count);
                return PPA_FAILURE;
        }

	current_index = ((pIp4CmpTbl + hash_index)->first_ptr);
//	printk("current index %d\n",current_index);
	if( current_index == g_GenConf->fw_sess_tbl_num[0] -1 )  //not valid first_ptr. it means there is no session yet in this hash_index
	{
		temp_ptr= pIp4CmpTbl + hash_index;
		if(temp_ptr->valid) {
			if((free_index = mpe_hal_find_free_index(pIp4CmpTbl,0)) == MPE_HAL_NO_FREE_ENTRY){
				printk("\n No Free Entry\n");
				return PPA_FAILURE;
			}
			else {
			}

//				printk("\n free_index[%d]\n",free_index);
		}
		else {
			free_index = hash_index;
//			printk("\n Index is not a valid entry ---> free_index=hash_index[%d]\n",free_index);
		}
	}
	else /*some session already exists in hash_index*/
	{
		temp_ptr = (pIp4CmpTbl + current_index);
		if(temp_ptr->valid) {
			if((free_index = mpe_hal_find_free_index(pIp4CmpTbl,0)) == MPE_HAL_NO_FREE_ENTRY){
				printk(" No Free Entry\n");
				return PPA_FAILURE;
			}
			else {
			}
	//			printk("\n free_index[%d]\n",free_index);
		}
		else
			free_index = current_index;
	}
	temp_ptr= pIp4CmpTbl + free_index;
	temp_ptr->nxt_ptr = (pIp4CmpTbl + hash_index)->first_ptr;
	(pIp4CmpTbl + hash_index)->first_ptr = free_index;
	if(temp_ptr->nxt_ptr == g_GenConf->fw_sess_tbl_num[0] -1 )
		temp_ptr->nxt_ptr = free_index;

	temp_ptr->valid        = 1;
	temp_ptr->key.srcip     = p_key->srcip;
	temp_ptr->key.dstip     = p_key->dstip;
	temp_ptr->key.srcport   = p_key->srcport;
	temp_ptr->key.dstport   = p_key->dstport;
	temp_ptr->key.extn      = p_key->extn;
	temp_ptr->act           = sess_action;
//	printk("Action Pointer: %x \n",temp_ptr->act);
//	printk("<%s> returning Free Index: %d \n",__FUNCTION__,free_index);

#if 0
	int32_t i=0;
	for(i = 0; i < g_GenConf->fw_sess_tbl_num[0]; i++ ) {
		printk("Next Entry %d address 0x%x\n",i, (uint32_t)(pIp4CmpTbl + i ));
		printk("Valid: %d\n",(pIp4CmpTbl + (i ))->valid);
		printk("Next Pointer: %d\n",(pIp4CmpTbl + (i ))->nxt_ptr);
		printk("First Pointer: %d\n",(pIp4CmpTbl + (i ))->first_ptr);
		printk("Action Pointer: %x\n",(pIp4CmpTbl + (i ))->act);

		printk("SrcIp: %s\n",inet_ntoa((pIp4CmpTbl + (i ))->key.srcip));
		printk("DstIp: %s\n",inet_ntoa((pIp4CmpTbl + (i ))->key.dstip));
		printk("SrcPort: %d\n",(pIp4CmpTbl + (i ))->key.srcport);
		printk("DstPort: %d\n",(pIp4CmpTbl + (i ))->key.dstport);
		printk("====================\n");
	}

#endif
	
	/* Set the MIB and Hit counter for this session index */
	((struct session_action *)(temp_ptr->act))->sess_mib_ix_en = 1;
	((struct session_action *)(temp_ptr->act))->sess_mib_hit_en = 1;
	((struct session_action *)(temp_ptr->act))->sess_mib_ix = free_index;

	return free_index;
}

uint32_t mpe_hal_add_session_ipv6(struct fw_compare_hash_auto_ipv6 *pIp6CmpTbl, struct ipv6_hash_auto_key *p_key, uint32_t sess_action)
{
	uint32_t free_index=0;
	uint32_t count=0;
	uint32_t current_index;
	uint32_t hash_index;
	uint32_t tmp;
	struct fw_compare_hash_auto_ipv6  *temp_ptr;

	//dump_pkt(sizeof(struct ipv6_hash_auto_key), (char *)p_key);
	hash_index = mpe_cal_hash( NULL, p_key, 0); 
	//printk("\n CRC Calculation %x\n",hash_index);
	hash_index &= (g_GenConf->fw_sess_tbl_num[1] - 1);
	//printk("\n<%s> Hash Index %d\n",__FUNCTION__,hash_index);


#if 0
	int32_t i=0;
	for(i = 0; i < g_GenConf->fw_sess_tbl_num[1]; i++ ) {
		printk("Next Entry %d address 0x%x\n",i,((pIp6CmpTbl + (i ))));
		printk("Valid: %d\n",(pIp6CmpTbl + (i ))->valid);
		printk("Next Pointer: %d\n",(pIp6CmpTbl + (i ))->nxt_ptr);
		printk("First Pointer: %d\n",(pIp6CmpTbl + (i ))->first_ptr);
		printk("Action Pointer: %d\n",(pIp6CmpTbl + (i ))->act);
	}

#endif
	
	if(pIp6CmpTbl == NULL){
		printk("No IPV6 Compare Table is allocated\n");
		return PPA_FAILURE;
	}
	if(((struct session_action *)(sess_action)) == NULL){
		//printk("No Session Action Pointer\n");
		return PPA_FAILURE;
	}

	tmp  = (pIp6CmpTbl + hash_index)->first_ptr;
	while(tmp != (pIp6CmpTbl + tmp)->nxt_ptr)
        {
                tmp=(pIp6CmpTbl + tmp)->nxt_ptr;
                count++;
        }
	if( count > MAX_SEARCH_ITRN ) {
                printk("\nNumber of entries for hash=%X is %d \n", hash_index, count);
                return PPA_FAILURE;
        }

	current_index = (pIp6CmpTbl + hash_index)->first_ptr;
	if( current_index == g_GenConf->fw_sess_tbl_num[1] - 1 )  //not valid first_ptr. it means there is no session yet in this hash_index
	{
		temp_ptr= (pIp6CmpTbl + hash_index);
		if(temp_ptr->valid) {
			if((free_index = mpe_hal_find_free_index(pIp6CmpTbl,1)) == MPE_HAL_NO_FREE_ENTRY){
				printk("\n No Free Entry !!!\n");
				return PPA_FAILURE;
			}
			else
				printk("\n free_index[%d]\n",free_index);
		}
		else
			free_index = hash_index;
	}
	else /*valid first_ptr with some session already in hash_index*/
	{
		temp_ptr = (pIp6CmpTbl + current_index);
		if(temp_ptr->valid) {
			if((free_index = mpe_hal_find_free_index(pIp6CmpTbl,1)) == MPE_HAL_NO_FREE_ENTRY){
				printk(" No Free Entry !!!\n");
				return PPA_FAILURE;
			}
			else
				printk("\n free_index[%d]\n",free_index);
		}
		else
			free_index = current_index;
	}
	temp_ptr= pIp6CmpTbl + free_index;
	temp_ptr->nxt_ptr = (pIp6CmpTbl + hash_index)->first_ptr;
	(pIp6CmpTbl + hash_index)->first_ptr = free_index;
	if(temp_ptr->nxt_ptr == g_GenConf->fw_sess_tbl_num[1] -1 )
		temp_ptr->nxt_ptr = free_index;

	temp_ptr->valid        = 1;
#if 1 //khalil
	temp_ptr->key.srcip[3] = p_key->srcip[0];
	temp_ptr->key.srcip[2] = p_key->srcip[1];
	temp_ptr->key.srcip[1] = p_key->srcip[2];
	temp_ptr->key.srcip[0] = p_key->srcip[3];
	temp_ptr->key.dstip[3] = p_key->dstip[0];
	temp_ptr->key.dstip[2] = p_key->dstip[1];
	temp_ptr->key.dstip[1] = p_key->dstip[2];
	temp_ptr->key.dstip[0] = p_key->dstip[3];
#else
	temp_ptr->key.srcip[0] = p_key->srcip[0];
	temp_ptr->key.srcip[1] = p_key->srcip[1];
	temp_ptr->key.srcip[2] = p_key->srcip[2];
	temp_ptr->key.srcip[3] = p_key->srcip[3];
	temp_ptr->key.dstip[0] = p_key->dstip[0];
	temp_ptr->key.dstip[1] = p_key->dstip[1];
	temp_ptr->key.dstip[2] = p_key->dstip[2];
	temp_ptr->key.dstip[3] = p_key->dstip[3];
#endif
	temp_ptr->key.srcport  = p_key->srcport;
	temp_ptr->key.dstport  = p_key->dstport;
	temp_ptr->key.extn     = p_key->extn;
	temp_ptr->act          = sess_action;
	//printk("Action Pointer: %x \n",temp_ptr->act);


	/* Set the MIB and Hit counter for this session index */
	((struct session_action *)(temp_ptr->act))->sess_mib_ix_en = 1;
	((struct session_action *)(temp_ptr->act))->sess_mib_hit_en = 1;
	((struct session_action *)(temp_ptr->act))->sess_mib_ix = free_index;
	return free_index;
}




uint32_t mpe_hal_del_session_ipv4(struct fw_compare_hash_auto_ipv4 *pIp4CmpTbl, struct ipv4_hash_auto_key *p_key, uint32_t hash_index)
{
	uint32_t index=0;
	uint32_t counter = 0;
	uint32_t previous;
	uint32_t hash_start=0;
	uint32_t start, next;

//	printk("<%s> Delete Index: %d\n",__FUNCTION__,hash_index);
	//hash_start = mpe_hal_calculate_hash((void *)p_key, 0);
	hash_start = mpe_cal_hash(p_key, NULL, 0); 
//	printk("\n CRC %x\n",hash_start);
	//printk("\n IPv4 table size  %d\n",(g_GenConf->fw_sess_tbl_num[0]));
	hash_start &= ((g_GenConf->fw_sess_tbl_num[0]) - 1);
//	printk("\n<%s> Hash Index %x\n",__FUNCTION__,hash_start);

        if(hash_index >= g_GenConf->fw_sess_tbl_num[0])
        {
                return MAX_ENTRY_ERROR;
        }

        if((pIp4CmpTbl + hash_index)->valid)
        {
		previous=hash_start;
                index=((pIp4CmpTbl + previous)->first_ptr);
                while(hash_index!= index)
                {
                        if(counter > MAX_SEARCH_ITRN)
                                return MAX_ENTRY_ERROR;
                        previous=index;
                        index=(pIp4CmpTbl + index)->nxt_ptr;
                        counter++;
                }
		start = (pIp4CmpTbl + previous)->first_ptr;
                next = (pIp4CmpTbl + index)->nxt_ptr;

                if(hash_index == start) /* delete first entry */
                {
                         if(hash_index==hash_start && next==hash_start )
                                (pIp4CmpTbl + hash_start)->first_ptr= g_GenConf->fw_sess_tbl_num[0]-1;
                         else
                                (pIp4CmpTbl + previous)->first_ptr=(pIp4CmpTbl + index)->nxt_ptr;

                }
		else if(hash_index == next) /* delete last entry */
                {
                         (pIp4CmpTbl + previous)->nxt_ptr=previous;
                }
                else /* delete middle entry */
                {
                        (pIp4CmpTbl + previous)->nxt_ptr= (pIp4CmpTbl + index)->nxt_ptr;
                }
                (pIp4CmpTbl + index)->nxt_ptr = g_GenConf->fw_sess_tbl_num[0]-1;
                (pIp4CmpTbl + index)->valid=0;


	}
        else
        {
                printk("No Item to Delete!!!!!!!\n");
        }
	
	return index;

}

uint32_t mpe_hal_del_session_ipv6(struct fw_compare_hash_auto_ipv6 *pIp6CmpTbl, struct ipv6_hash_auto_key *p_key, uint32_t hash_index)
{
	uint32_t index=0;
	uint32_t counter = 0;
	uint32_t previous;
	uint32_t hash_start=0;
	uint32_t start, next;

	hash_start = mpe_cal_hash(NULL, p_key, 0); 
	//printk("\nHash Index %x\n",hash_start);
	hash_start &= ((g_GenConf->fw_sess_tbl_num[1]) - 1);
	//printk("\n<%s>Hash Start Index %x\n",__FUNCTION__,hash_start);

        if(hash_index >= g_GenConf->fw_sess_tbl_num[1])
        {
                return MAX_ENTRY_ERROR;
        }

        if((pIp6CmpTbl + hash_index)->valid)
        {
		previous=hash_start;
                index=((pIp6CmpTbl + previous)->first_ptr);
                while(hash_index!= index)
                {
                        if(counter > MAX_SEARCH_ITRN)
                                return MAX_ENTRY_ERROR;
                        previous=index;
                        index=(pIp6CmpTbl + index)->nxt_ptr;
                        counter++;
                }
		start = (pIp6CmpTbl + previous)->first_ptr;
                next = (pIp6CmpTbl + index)->nxt_ptr;

                if(hash_index == start) /* delete first entry */
                {
                         if(hash_index==hash_start && next==hash_start )
                                (pIp6CmpTbl + hash_start)->first_ptr= g_GenConf->fw_sess_tbl_num[1]-1;
                         else
                                (pIp6CmpTbl + previous)->first_ptr=(pIp6CmpTbl + index)->nxt_ptr;

                }
		else if(hash_index == next) /* delete last entry */
                {
                         (pIp6CmpTbl + previous)->nxt_ptr=previous;
                }
                else /* delete middle entry */
                {
                        (pIp6CmpTbl + previous)->nxt_ptr= (pIp6CmpTbl + index)->nxt_ptr;
                }
                (pIp6CmpTbl + index)->nxt_ptr = g_GenConf->fw_sess_tbl_num[1]-1;
                (pIp6CmpTbl + index)->valid=0;


	}
        else
        {
                printk("No Item to Delete!!!!!!!\n");
        }
	
	return index;

}



#if 1

uint8_t src_mac_ipv4[6] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50};
uint8_t dst_mac_ipv4[6] = {0x00, 0x10, 0x20, 0x30, 0x40, 0x50};

struct session_action paction = {0};

#if 1
void create_action()
{
	paction.entry_vld = 1;
	paction.pkt_len_delta = 4;
	paction.templ_len = 18;
	paction.tunnel_type = 0;
	paction.pppoe_offset_en = 0;
	paction.outer_dscp_mode = 0;
	paction.dst_pmac_port_list[0] = 15;
	paction.in_eth_iphdr_offset = 30;
	paction.new_dst_ip_en = 1;
	paction.new_dst_ip.ip4.ip = 0x64640A01;
	paction.protocol = 1;
	paction.new_dst_port = 0x5000;
}
#endif
int32_t create_hw_action(struct session_action *paction)
{
	paction->entry_vld = 1;
	paction->pkt_len_delta = 4;
	paction->templ_len = 18;
	paction->tunnel_type = 0;
	paction->pppoe_offset_en = 0;
	paction->outer_dscp_mode = 0;
	paction->dst_pmac_port_num = 1;
	paction->dst_pmac_port_list[0] = 15;
	paction->in_eth_iphdr_offset = 30;
	paction->new_dst_ip_en = 1;
	paction->new_dst_ip.ip4.ip = 0x64640A01;
	paction->new_src_ip.ip4.ip = 0x65650B02;
	paction->protocol = 1;
	paction->new_dst_port = 0x5000;
	return PPA_SUCCESS;
}

uint32_t  create_templ_buffer(unsigned char *buffer)
{

	unsigned char *buf = buffer;
	uint32_t buf_len=0;
	memcpy(buf, dst_mac_ipv4,sizeof(dst_mac_ipv4));
        buf += sizeof(dst_mac_ipv4);
        memcpy(buf, src_mac_ipv4,sizeof(src_mac_ipv4));
        buf += sizeof(src_mac_ipv4);

	*buf='\0';
	buf_len = (unsigned int)buf - (unsigned int)buffer;
        return buf_len;
}

#endif

int32_t mpe_hal_add_routing_entry(PPE_ROUTING_INFO *route)
{
	struct fw_compare_hash_auto_ipv4 hIpv4;
	struct fw_compare_hash_auto_ipv6 hIpv6;
	int32_t ret = PPA_SUCCESS;
#if 0
	switch (route->tnnl_info.tunnel_type) {
    		case TUNNEL_TYPE_L2TP:
			break;
    		case TUNNEL_TYPE_IPOGRE:
			break;
    	default:
		break;
#endif
//	printk("<%s> <-------- Enter ---------->\n",__FUNCTION__);
	if(route->src_ip.f_ipv6) {
		memset(&hIpv6, 0, sizeof(struct fw_compare_hash_auto_ipv6));
		if(route->f_is_tcp) {
        		hIpv6.key.extn = 0; 
    		} else {
			hIpv6.key.extn = 1;
    		}
		ppa_memcpy(hIpv6.key.srcip,route->src_ip.ip.ip6,sizeof(uint16_t)*8);
		ppa_memcpy(hIpv6.key.dstip,route->dst_ip.ip.ip6,sizeof(uint16_t)*8);
    		hIpv6.key.srcport = route->src_port;
    		hIpv6.key.dstport = route->dst_port;
		if(g_GenConf->fw_cmp_tbl_base[1])
			return mpe_hal_add_session_ipv6((struct fw_compare_hash_auto_ipv6 *)g_GenConf->fw_cmp_tbl_base[1], 
							(struct ipv6_hash_auto_key *) &hIpv6.key, 
							route->sessionAction);
    	} else {
		memset(&hIpv4, 0, sizeof(struct fw_compare_hash_auto_ipv4));
		if(route->f_is_tcp) {
        		hIpv4.key.extn = 0; 
    		} else {
			hIpv4.key.extn = 1;
    		}

		hIpv4.key.rsvd = 0;
		hIpv4.key.srcip = route->src_ip.ip.ip;
		hIpv4.key.dstip = route->dst_ip.ip.ip;
    		hIpv4.key.srcport = route->src_port;
    		hIpv4.key.dstport = route->dst_port;
//                printk("key :srcip     = %s\n",inet_ntoa(hIpv4.key.srcip));
 //               printk("key :dstip     = %s\n",inet_ntoa(hIpv4.key.dstip));
   //             printk("key :dstport   = %d\n",hIpv4.key.srcport);
     //           printk("key :srcport   = %d\n",hIpv4.key.dstport);
#if 0
		create_action();
		create_templ_buffer(&paction.templ_buf);
    		action = (uint32_t)&paction;
#endif
		if(g_GenConf->fw_cmp_tbl_base[0]) {
			if((ret = mpe_hal_add_session_ipv4((struct fw_compare_hash_auto_ipv4 *)g_GenConf->fw_cmp_tbl_base[0],
							(struct ipv4_hash_auto_key *) &hIpv4.key, 
							route->sessionAction)) != PPA_FAILURE) {
				route->entry = ret;
				ret = PPA_SUCCESS;
			}
		}
				
    	}
	
//	printk("<%s> <-------- Exit ---------->\n",__FUNCTION__);
	return ret;
}


int32_t mpe_hal_del_routing_entry(PPE_ROUTING_INFO *route)
{
	struct fw_compare_hash_auto_ipv4 hIpv4;
	struct fw_compare_hash_auto_ipv6 hIpv6;

	//printk("<%s> <---- Enter ---->\n",__FUNCTION__);
	if(route->src_ip.f_ipv6) {
		memset(&hIpv6, 0, sizeof(struct fw_compare_hash_auto_ipv6));
		if(route->f_is_tcp) {
        		hIpv6.key.extn = 0; 
    		} else {
			hIpv6.key.extn = 1;
    		}
		ppa_memcpy(hIpv6.key.srcip,route->src_ip.ip.ip6,sizeof(uint16_t)*8);
		ppa_memcpy(hIpv6.key.dstip,route->dst_ip.ip.ip6,sizeof(uint16_t)*8);
    		hIpv6.key.srcport = route->src_port;
    		hIpv6.key.dstport = route->dst_port;
		if(g_GenConf->fw_cmp_tbl_base[1])
			return mpe_hal_del_session_ipv6((struct fw_compare_hash_auto_ipv6 *)g_GenConf->fw_cmp_tbl_base[1],
							(struct ipv6_hash_auto_key *)&hIpv6.key, 
							route->entry);
    	} else {
		memset(&hIpv4, 0, sizeof(struct fw_compare_hash_auto_ipv4));
		if(route->f_is_tcp) {
        		hIpv4.key.extn = 0; 
    		} else {
			hIpv4.key.extn = 1;
    		}
		hIpv4.key.srcip = route->src_ip.ip.ip;
		hIpv4.key.dstip = route->dst_ip.ip.ip;
    		hIpv4.key.srcport = route->src_port;
    		hIpv4.key.dstport = route->dst_port;
	//	printk("key :srcip     = %s\n",inet_ntoa(hIpv4.key.srcip));
          //      printk("key :dstip     = %s\n",inet_ntoa(hIpv4.key.dstip));
            //    printk("key :dstport   = %d\n",hIpv4.key.srcport);
              //  printk("key :srcport   = %d\n",hIpv4.key.dstport);
	//	printk("delete Entry: %d \n",route->entry);

		if(g_GenConf->fw_cmp_tbl_base[0])
			return mpe_hal_del_session_ipv4((struct fw_compare_hash_auto_ipv4 *)g_GenConf->fw_cmp_tbl_base[0],
							(struct ipv4_hash_auto_key *)&hIpv4.key, 
							route->entry);
				
    	}

	return PPA_SUCCESS;
}

uint32_t mpe_hal_add_hw_session(PPE_ROUTING_INFO *route)
{
	struct hw_act_ptr *hw_index;

	if(route->entry > g_GenConf->hw_act_num ){
		printk("Not a valid entry\n");
		return PPA_FAILURE;
	}

#ifdef CONFIG_LTQ_PPA_MPE_IP97
	if(route->tnnl_info.tunnel_type == TUNNEL_TYPE_IPSEC)
	{
		printk("<%s>%d\n",__FUNCTION__,__LINE__);
		if(
			((route->entry == 0) && (route->f_is_lan)) //outbound
				|| 
			(!route->f_is_lan) //inbound 
		  )
			return mpe_hal_add_ipsec_tunl_entry(route);

	}
#endif

	if(((struct session_action *)(route->sessionAction)) == NULL){
		printk("Entry %d has null action pointer\n",route->entry);
		return PPA_FAILURE;
	}

	printk("<%s>Routing session index is : %d\n",__FUNCTION__,route->entry);
	hw_index =(struct hw_act_ptr *)(g_GenConf->hw_act_tbl_base + (route->entry * sizeof(struct hw_act_ptr)) );
	hw_index->act = route->sessionAction;

	printk("HW index base %x\n",g_GenConf->hw_act_tbl_base);
	printk("HW Entry %d base %p\n",route->entry, hw_index);

	printk("Template buffer len        = %d\n",((struct session_action *)(hw_index->act))->templ_len);
	printk("pkt_len_delta              = %d\n",((struct session_action *)(hw_index->act))->pkt_len_delta);

#ifdef CONFIG_LTQ_PPA_MPE_IP97
	if(route->tnnl_info.tunnel_type == TUNNEL_TYPE_IPSEC)
	{
		((struct session_action *)(hw_index->act))->rx_itf_mib_num = 1;
		((struct session_action *)(hw_index->act))->tx_itf_mib_num = 1;
		((struct session_action *)(hw_index->act))->tx_itf_mib_ix[0] = ((struct session_action *)(hw_index->act))->tunnel_id * 2;
		((struct session_action *)(hw_index->act))->rx_itf_mib_ix[0] = ((struct session_action *)(hw_index->act))->tunnel_id * 2 + 1;
	}
#endif

	return PPA_SUCCESS;
}

uint32_t mpe_hal_add_hw_session_test(void)
{

	uint32_t action = 0;
	PPE_ROUTING_INFO route;
	struct session_action sess_action = {0};
	

	ppa_memset( &route, 0, sizeof(route));
	printk("mpe_hal_add_hw_session_test\n");
	create_hw_action(&sess_action);

	sess_action.templ_buf = ppa_malloc(sess_action.templ_len);	
	ppa_memset(sess_action.templ_buf, 0, sess_action.templ_len);

	create_templ_buffer((unsigned char *)&sess_action.templ_buf);
    	action = (uint32_t)&sess_action;
	route.sessionAction = action;
	route.entry = 2;
	mpe_hal_add_hw_session(&route);
	//ppa_free(&sess_action.templ_buf);
	return PPA_SUCCESS;
}


uint32_t mpe_hal_del_hw_session(PPE_ROUTING_INFO *route)
{
	struct hw_act_ptr *hw_index;

	if(route->entry > g_GenConf->hw_act_num )
		return PPA_FAILURE;

	printk("<%s>Routing session index is : %d\n",__FUNCTION__,route->entry);
#ifdef CONFIG_LTQ_PPA_MPE_IP97
	if(route->tnnl_info.tunnel_type == TUNNEL_TYPE_IPSEC) //outbound
	{
		if(((route->entry == 0) && (route->f_is_lan)) 
				|| 
			(!route->f_is_lan) ) //inbound
			return mpe_hal_del_ipsec_tunl_entry(route);
	}
#endif

	hw_index =(struct hw_act_ptr *)(g_GenConf->hw_act_tbl_base + (route->entry * sizeof(struct hw_act_ptr)) );
	hw_index->act = 0;
	return PPA_SUCCESS;
}

int32_t mpe_hal_add_wan_mc_entry(PPE_MC_INFO *mc)
{
	uint32_t i, k, port;
	struct hw_act_ptr *hw_index;
    	struct vap_entry *ve;
	if(mc->p_entry > g_GenConf->hw_act_num ){
		printk("Not a valid entry\n");
		return PPA_FAILURE;
	}

	if(((struct session_action *)(mc->sessionAction)) == NULL){
		printk("Entry %d has null action pointer\n",mc->p_entry);
		return PPA_FAILURE;
	}

	printk("<%s>Routing session index is : %d\n",__FUNCTION__,mc->p_entry);
	hw_index =(struct hw_act_ptr *)(g_GenConf->hw_act_tbl_base + (mc->p_entry * sizeof(struct hw_act_ptr)) );
	hw_index->act = mc->sessionAction;

	printk("HW index base %x\n",g_GenConf->hw_act_tbl_base);
	printk("HW Entry %d base %p\n",mc->p_entry, hw_index);

	printk("Template buffer len        = %d\n",((struct session_action *)(hw_index->act))->templ_len);
	printk("pkt_len_delta              = %d\n",((struct session_action *)(hw_index->act))->pkt_len_delta);
	printk("<%s> mc->group_id=%d mc->dest_list=%d \n", __FUNCTION__, mc->group_id,  mc->dest_list);
#if 1
	/** Now populate the VAP table */
    	for(i=0; i<MAX_PMAC_PORT; i++) {
		uint32_t vap;
		port = (1 << i) & (mc->dest_list) ;
		printk("Port %d i=%d\n",port,i);
		if(port) {
			port = i;
			ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[port] + (sizeof(struct vap_entry) * mc->group_id));
    			for(k=0; k<MAX_PMAC_PORT; k++) {
				vap = (1 << k) & (mc->dest_subif[port]) ;
				if(vap) {
					vap =k;
					ve->vap_list[ve->num] = mc->group_id;
					ve->vap_list[ve->num] |= 1 << 7; // group_flag
					ve->vap_list[ve->num] |= (vap & 0xF) << 8 ;// VAP[8:11]
					ve->vap_list[ve->num] |= 1 << 14 ;// MC bit set for multicast
					ve->num++;
				}
			}
		}
		if(port) {
			for(k=0; k<ve->num ;k++) {
                		printk("%04x ", ve->vap_list[k]);
            		}
		}
	}
            
#endif
	return PPA_SUCCESS;
}

int32_t mpe_hal_del_wan_mc_entry(PPE_MC_INFO *mc)
{
	uint32_t i, port;
	struct hw_act_ptr *hw_index;
    	struct vap_entry *ve;
	if(mc->p_entry > g_GenConf->hw_act_num )
		return PPA_FAILURE;

	printk("<%s>Routing session index is : %d\n",__FUNCTION__,mc->p_entry);
	hw_index =(struct hw_act_ptr *)(g_GenConf->hw_act_tbl_base + (mc->p_entry * sizeof(struct hw_act_ptr)) );
	hw_index->act = 0;
#if 1
	/** Now populate the VAP table */
    	for(i=0; i<MAX_PMAC_PORT; i++) {
		port = (1 << i) & (mc->dest_list) ;
		printk("Port %d i=%d\n",port,i);
		if(port) {
			port = i;
			ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[port] + (sizeof(struct vap_entry) * mc->group_id));
    			memset(ve, 0, sizeof(struct vap_entry));	
		}
		
	}
            
#endif


	return PPA_SUCCESS;
}


#ifdef CONFIG_LTQ_PPA_MPE_IP97

#define MPE_HAL_MASK_CTX_SIZE 0x0000FF00
#define MPE_HAL_MASK_CTX_FETCH_MODE 0x80000000

struct ltq_crypto_ipsec_params crypto_ipsec_params_in[IPSEC_TUN_MAX];
struct ltq_crypto_ipsec_params crypto_ipsec_params_out[IPSEC_TUN_MAX];

int32_t mpe_hal_alloc_cdr_rdr(void)
{
	int32_t i, j;
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			g_GenConf->e97_cdr_in[i][j] = (uint32_t ) kmalloc((EIP97_CD_SIZE * 4), GFP_DMA);
			if (!g_GenConf->e97_cdr_in[i][j])
			{
				printk("Failed to allocate memory for e97_cdr_in table for Worker %d and tunnel index %d\n",i,j);
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->e97_cdr_in[i][j], 0, (EIP97_CD_SIZE * 4));
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			g_GenConf->e97_cdr_out[i][j] = (uint32_t ) kmalloc((EIP97_CD_SIZE * 4), GFP_DMA);
			if (!g_GenConf->e97_cdr_out[i][j])
			{
				printk("Failed to allocate memory for e97_cdr_out table for Worker %d and tunnel index %d\n",i,j);
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->e97_cdr_out[i][j], 0, (EIP97_CD_SIZE * 4));
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			g_GenConf->e97_acd_in[i][j] = (uint32_t ) kmalloc((EIP97_ACD_MAX_SIZE), GFP_DMA);
			if (!g_GenConf->e97_acd_in[i][j])
			{
				printk("Failed to allocate memory for e97_acd_in table for Worker %d and tunnel index %d\n",i,j);
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->e97_acd_in[i][j], 0, (EIP97_ACD_MAX_SIZE));
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			g_GenConf->e97_acd_out[i][j] = (uint32_t ) kmalloc((EIP97_ACD_MAX_SIZE), GFP_DMA);
			if (!g_GenConf->e97_acd_out[i][j])
			{
				printk("Failed to allocate memory for e97_acd_out table for Worker %d and tunnel index %d\n",i,j);
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->e97_acd_out[i][j], 0, (EIP97_ACD_MAX_SIZE));
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		g_GenConf->e97_rdr[i] = (uint32_t ) kmalloc((EIP97_CD_SIZE * 4), GFP_DMA);
		if (!g_GenConf->e97_rdr[i])
		{
			printk("Failed to allocate memory for e97_rdr table for Worker %d \n",i);
			return PPA_FAILURE;
		}
		memset((void *)g_GenConf->e97_rdr[i], 0, (EIP97_CD_SIZE * 4));
	}

	memset(&crypto_ipsec_params_in[0], 0, sizeof(struct ltq_crypto_ipsec_params ) * IPSEC_TUN_MAX );
	memset(&crypto_ipsec_params_out[0], 0, sizeof(struct ltq_crypto_ipsec_params ) * IPSEC_TUN_MAX );
	return PPA_SUCCESS;
}

int32_t mpe_hal_free_cdr_rdr(void) 
{
	int32_t i, j;
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			if (g_GenConf->e97_cdr_in[i][j])
				g_GenConf->e97_cdr_in[i][j] = 0;
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			if (g_GenConf->e97_cdr_out[i][j])
				g_GenConf->e97_cdr_out[i][j] = 0;
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			if (g_GenConf->e97_acd_in[i][j])
				g_GenConf->e97_acd_in[i][j] = 0;
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		for(j=0; j< IPSEC_TUN_MAX; j++) {
			if (g_GenConf->e97_acd_out[i][j])
				g_GenConf->e97_acd_out[i][j] = 0;
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		if (g_GenConf->e97_rdr[i])
			g_GenConf->e97_rdr[i] = 0;
	}
	return PPA_SUCCESS;
}
int32_t mpe_hal_construct_cdr_rdr(struct ltq_crypto_ipsec_params *eip97_params, struct ipsec_info *ipsec_info)
{
	printk("<%s> Entry \n",__FUNCTION__);

	ipsec_info->cd_info.dw0.acd_size = eip97_params->tokenwords;
	ipsec_info->cd_info.dw0.first_seg = 1;
	ipsec_info->cd_info.dw0.last_seg = 1;
	ipsec_info->cd_info.dw0.buf_size = 0;

	ipsec_info->cd_info.dw2.acd_ptr = (uint32_t)eip97_params->token_buffer;

	memcpy(&ipsec_info->cd_info.dw3 ,&eip97_params->token_headerword, sizeof(uint32_t));

	ipsec_info->cd_info.dw4.res = 0;
	ipsec_info->cd_info.dw4.app_id = 0;


	ipsec_info->cd_info.dw5.ctx_ptr = (uint32_t)eip97_params->SA_buffer;

	ipsec_info->pad_instr_offset = (eip97_params->pad_offs[0] * 4);
	ipsec_info->pad_en = 1;
	ipsec_info->crypto_instr_offset = (eip97_params->crypto_offs * 4);

	ipsec_info->cd_size = EIP97_CD_SIZE;
	ipsec_info->iv_len = 16;
	ipsec_info->icv_len = 12;
	ipsec_info->blk_size = 16;
	return PPA_SUCCESS;
}

int32_t mpe_hal_construct_ipsec_buffer(int32_t dir, int32_t tunlId, struct ltq_crypto_ipsec_params *eip97_params)
{

	printk("<%s> Entry \n",__FUNCTION__);
	if(dir == LTQ_SAB_DIRECTION_INBOUND ){
		if(g_GenConf->ipsec_input_flag[tunlId] != 1){
			printk("<%s> ipsec Buffer for inbound tunnel %d\n",__FUNCTION__, tunlId);
			memset(&g_GenConf->ipsec_input[tunlId], 0, sizeof(struct ipsec_info));
			mpe_hal_construct_cdr_rdr(eip97_params, &g_GenConf->ipsec_input[tunlId]);
			g_GenConf->ipsec_input_flag[tunlId] = 1;	
		}

	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND) {
		if(g_GenConf->ipsec_output_flag[tunlId] != 1) {
			printk("<%s> ipsec Buffer for outbound tunnel %d\n",__FUNCTION__, tunlId);
			memset(&g_GenConf->ipsec_output[tunlId], 0, sizeof(struct ipsec_info));
			mpe_hal_construct_cdr_rdr(eip97_params, &g_GenConf->ipsec_output[tunlId]);
			g_GenConf->ipsec_output_flag[tunlId] = 1;	
		}
	} else {
		printk("Wrong SA Direction !!!\n");
		return PPA_FAILURE;
	}
	return PPA_SUCCESS;
}

int32_t mpe_hal_get_auth_size(char *algo)
{
	if(!strcmp(algo, "digest_null"))
		return 0 ;
	if(!strcmp(algo, "hash(md5)"))
		return 96;
	if(!strcmp(algo, "hmac(md5)"))
		return 128;
	if(!strcmp(algo, "hmac(sha1)"))
		return 160 ;
	if(!strcmp(algo, "hmac(sha256)"))
		return 256;
	if(!strcmp(algo, "hmac(sha384)"))
		return 384;
	if(!strcmp(algo, "hmac(sha512)"))
		return 512;

	return PPA_FAILURE;
}

int32_t mpe_hal_get_auth_algo(char *algo)
{
	if(!strcmp(algo, "digest_null"))
		return LTQ_SAB_AUTH_NULL;
	if(!strcmp(algo, "hash(md5)"))
		return LTQ_SAB_AUTH_HASH_MD5;
	if(!strcmp(algo, "hmac(md5)"))
		return LTQ_SAB_AUTH_HMAC_MD5;
	if(!strcmp(algo, "hmac(sha1)"))
		return LTQ_SAB_AUTH_HMAC_SHA1;
	if(!strcmp(algo, "hmac(sha256)"))
		return LTQ_SAB_AUTH_HMAC_SHA2_256;
	if(!strcmp(algo, "hmac(sha384)"))
		return LTQ_SAB_AUTH_HMAC_SHA2_384;
	if(!strcmp(algo, "hmac(sha512)"))
		return LTQ_SAB_AUTH_HMAC_SHA2_512;

	return PPA_FAILURE;
}
int32_t mpe_hal_get_crypto_algo(char *algo)
{

	if(!strcmp(algo, "cipher_null"))
		return LTQ_SAB_CRYPTO_NULL;
	if(!strcmp(algo, "des"))
		return LTQ_SAB_CRYPTO_DES;
	if(!strcmp(algo, "3des"))
		return LTQ_SAB_CRYPTO_3DES;
	if(!strcmp(algo, "aes"))
		return LTQ_SAB_CRYPTO_AES;
	if(!strcmp(algo, "arcfour"))
		return LTQ_SAB_CRYPTO_ARCFOUR;

	return PPA_FAILURE;
}
int32_t mpe_hal_get_crypto_mode(char *mode)
{

	if(!strcmp(mode, "ecb"))
		return LTQ_SAB_CRYPTO_MODE_ECB;
	if(!strcmp(mode, "cbc"))
		return LTQ_SAB_CRYPTO_MODE_CBC;
	if(!strcmp(mode, "ofb"))
		return LTQ_SAB_CRYPTO_MODE_OFB;
	if(!strcmp(mode, "cfb"))
		return LTQ_SAB_CRYPTO_MODE_CFB;
	if(!strcmp(mode, "cfb1"))
		return LTQ_SAB_CRYPTO_MODE_CFB1;
	if(!strcmp(mode, "cfb8"))
		return LTQ_SAB_CRYPTO_MODE_CFB8;
	if(!strcmp(mode, "ctr"))
		return LTQ_SAB_CRYPTO_MODE_CTR;
	if(!strcmp(mode, "icm"))
		return LTQ_SAB_CRYPTO_MODE_ICM;
	if(!strcmp(mode, "ccm"))
		return LTQ_SAB_CRYPTO_MODE_CCM;
	if(!strcmp(mode, "gcm"))
		return LTQ_SAB_CRYPTO_MODE_GCM;
	if(!strcmp(mode, "gmac"))
		return LTQ_SAB_CRYPTO_MODE_GMAC;

	return PPA_FAILURE;

}
int32_t mpe_hal_get_crypto_mode_algo(char *alg, char *mode, char *algo )
{
        int32_t i=0;
        uint8_t alg1[64] = "";
        uint8_t *dash = strchr(alg, '(');
	
	if(dash == NULL)
		return PPA_FAILURE;

        for(i=0; *(alg+i) != '('; i++)
        {
                alg1[i] = *(alg+i);
                //printk("i=%d : %c\n",i, *(alg+i));
        }

        alg1[i] = '\0';
	strcpy(mode, alg1);
	printk("len1:%d len2:%d\n",(strlen(dash+1)), strlen(alg));
        strncpy(algo, dash+1, (strlen(dash+1)-1));
        printk("<%s> mode %s algo %s\n",__FUNCTION__, mode, algo);

        return PPA_SUCCESS;
}
int32_t mpe_hal_get_ipsec_spi(uint32_t tunnel_index, uint32_t dir, uint32_t *spi)
{
	ppa_tunnel_entry *t_entry = NULL;
	PPA_XFRM_STATE *xfrm_sa = NULL;

	t_entry = g_tunnel_table[tunnel_index];
	if ( t_entry == NULL ) 
		return PPA_FAILURE;

	if ((tunnel_index < 0) || (tunnel_index > IPSEC_TUN_MAX))
		return PPA_FAILURE;

	if (dir == LTQ_SAB_DIRECTION_INBOUND ) {
		xfrm_sa = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.inbound;
	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND) {
		xfrm_sa = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.outbound;
	}
	if(xfrm_sa != NULL) {
		*spi = xfrm_sa->id.spi;
		printk("<%s> SPI: %x\n",__FUNCTION__,*spi);
		return PPA_SUCCESS;
	}

	return PPA_FAILURE;
}

int32_t mpe_hal_dump_ipsec_xfrm_sa(uint32_t tunnel_index)
{
	ppa_tunnel_entry *t_entry = NULL;
	PPA_XFRM_STATE *xfrm_sa_in;
	PPA_XFRM_STATE *xfrm_sa_out;
	uint32_t key_len, i;

	t_entry = g_tunnel_table[tunnel_index];
	if ( t_entry == NULL ) 
		return PPA_FAILURE;

	if ((tunnel_index < 0) || (tunnel_index > IPSEC_TUN_MAX))
		return PPA_FAILURE;

	xfrm_sa_in = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.inbound;
	xfrm_sa_out = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.outbound;

	if(xfrm_sa_in != NULL) {
		printk("Inbound\n");
		printk("esp mode: %d  SPI: %x\n",xfrm_sa_in->props.mode, xfrm_sa_in->id.spi);

		if(xfrm_sa_in->ealg->alg_name != NULL) {
			key_len = (xfrm_sa_in->ealg->alg_key_len + 7)/8 ;
			printk("Crypto Key length: %d\n",key_len);
			//printk("E: %s  %s\n",xfrm_sa_in->ealg->alg_name, xfrm_sa_in->ealg->alg_key[0]);
			for (i=0; i< key_len; i++) {
				printk("%2x",(xfrm_sa_in->ealg->alg_key[i]) & 0xFF);
				if( i != 0 && i%8 == 0)
					printk("  ");
			}
			printk("\n");
		}

		if(xfrm_sa_in->aalg->alg_name != NULL) {
			key_len = (xfrm_sa_in->aalg->alg_key_len + 7)/8 ;
			printk("Auth Key length: %d\n",key_len);
			//printk("A: %s  %s\n",xfrm_sa_in->aalg->alg_name, xfrm_sa_in->aalg->alg_key[0]);
			for (i=0; i< key_len; i++) {
				printk("%2x",(xfrm_sa_in->aalg->alg_key[i]) & 0xFF);
				if( i != 0 && i%8 == 0)
					printk("  ");
			}
			printk("\n");
		}

	}

	if(xfrm_sa_out != NULL) {
		printk("Outbound\n");
		printk("esp mode: %d  SPI: %x\n",xfrm_sa_out->props.mode, xfrm_sa_out->id.spi);
		if(xfrm_sa_out->ealg->alg_name !=NULL) {
			key_len = (xfrm_sa_out->ealg->alg_key_len + 7)/8 ;
			//printk("E: %s  %s\n",xfrm_sa_out->ealg->alg_name, xfrm_sa_out->ealg->alg_key[0]);
			printk("Crypto Key length: %d\n",key_len);
			for (i=0; i< key_len; i++) {
				printk("%2x",(xfrm_sa_out->ealg->alg_key[i]) & 0xFF);
				if( i != 0 && i%8 == 0)
					printk("  ");
			}
			printk("\n");
		}

		if(xfrm_sa_out->aalg->alg_name != NULL) {
			key_len = (xfrm_sa_out->aalg->alg_key_len + 7)/8 ;
			//printk("A: %s  %s\n",xfrm_sa_out->aalg->alg_name, xfrm_sa_out->aalg->alg_key[0]);
			printk("Auth Key length: %d\n",key_len);
			for (i=0; i< key_len; i++) {
				printk("%2x",(xfrm_sa_out->aalg->alg_key[i]) & 0xFF);
				if( i != 0 && i%8 == 0)
					printk("  ");
			}
			printk("\n");

		}
	}

	return PPA_SUCCESS;
}

int32_t mpe_hal_dump_ipsec_eip97_params_info(struct ltq_crypto_ipsec_params *eip97_params)
{
	int32_t i=0;
	if(eip97_params->ipad[0] !=0)
	{
		printk("<ipad> ");
		for(i=0; i < 64; i++) {
			printk(" [%2x]",eip97_params->ipad[i]);
			if( i != 0 && i%16 == 0)
				printk("\n\t ");
		}
		printk("\n");
	}
	if(eip97_params->opad[0] !=0)
	{
		printk("<opad> ");
		for(i=0; i < 64; i++) {
			printk(" [%2x]",eip97_params->opad[i]);
			if( i != 0 && i%16 == 0)
				printk("\n          ");
		}
		printk("\n");
	}
	if(eip97_params->nonce[0] !=0)
	{
		printk("<nonce> ");
		for(i=0; i < MAX_NONCE_LEN; i++)
			printk(" [%2x]",eip97_params->nonce[i]);
		printk("\n");
	}
	if(eip97_params->IV[0] !=0)
	{
		printk("<IV> ");
		for(i=0; i < MAX_IV_LEN; i++) {
			printk(" [%2x]",eip97_params->IV[i]);
			if( i != 0 && i%8 == 0)
				printk("\n ");
		}
		printk("\n");
	}
	printk("tokenheaderword %x\n",eip97_params->token_headerword);
	printk("tokenword length: [%d] Context Buffer length:[%d]\n",eip97_params->tokenwords, eip97_params->SAWords);

	if(eip97_params->SAWords != 0)
	{
		printk("<Context> ");
		for(i=0; i < eip97_params->SAWords; i++)
			printk(" [%2x]\n",eip97_params->SA_buffer[i]);
		printk("\n");
	}
	if(eip97_params->tokenwords != 0)
	{
		printk("<Token> ");
		for(i=0; i < eip97_params->tokenwords; i++)
			printk(" [%2x]\n",eip97_params->token_buffer[i]);
		printk("\n");
	}
	return PPA_SUCCESS;
}

int32_t mpe_hal_dump_ipsec_eip97_params(int32_t tunnel_index)
{

	mpe_hal_dump_ipsec_eip97_params_info(&crypto_ipsec_params_out[tunnel_index]);
	mpe_hal_dump_ipsec_eip97_params_info(&crypto_ipsec_params_in[tunnel_index]);
	return PPA_SUCCESS;
}
int32_t mpe_hal_add_ipsec_tunl_entry(PPE_ROUTING_INFO *route)
{
	int32_t tunnel_index, dir, i;
	struct ltq_crypto_ipsec_params ipsec_eip97_params;
	PPA_XFRM_STATE *xfrm_sa;
	char emode[64] = "", ealgo[64]= "";

	tunnel_index = route->tnnl_info.tunnel_idx;

	if ((tunnel_index < 0) || (tunnel_index > IPSEC_TUN_MAX))
		return PPA_FAILURE;
	
	printk("<%s> Tunnel Index: %d\n",__FUNCTION__,tunnel_index);
	memset(&ipsec_eip97_params, 0, sizeof(struct ltq_crypto_ipsec_params));

	dir = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.dir;
	printk("<%s> Direction: %d\n",__FUNCTION__, dir);

	if(dir == LTQ_SAB_DIRECTION_INBOUND ) {
		xfrm_sa = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.inbound;
	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND) {
		xfrm_sa = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.outbound;
	}
	
	ipsec_eip97_params.ipsec_mode = LTQ_SAB_IPSEC_ESP; //xfrm_sa->id.proto;
	ipsec_eip97_params.ipsec_tunnel = LTQ_SAB_IPSEC_TUNNEL;
	ipsec_eip97_params.direction = dir;
	ipsec_eip97_params.spi = xfrm_sa->id.spi;
	ipsec_eip97_params.ip_type = LTQ_SAB_IPSEC_IPV4;
	ipsec_eip97_params.ring_no = 1;

	mpe_hal_get_crypto_mode_algo(xfrm_sa->ealg->alg_name, emode, ealgo);
	ipsec_eip97_params.crypto_mode = mpe_hal_get_crypto_mode(emode);
	ipsec_eip97_params.crypto_algo = mpe_hal_get_crypto_algo(ealgo);
	printk("Crypto -> Algo:%d Mode:%d\n",ipsec_eip97_params.crypto_algo, ipsec_eip97_params.crypto_mode);
	ipsec_eip97_params.crypto_keysize = (xfrm_sa->ealg->alg_key_len + 7)/8;
	ipsec_eip97_params.crypto_key = &xfrm_sa->ealg->alg_key[0];


	ipsec_eip97_params.auth_algo = mpe_hal_get_auth_algo(xfrm_sa->aalg->alg_name);
	printk("Auth Algo: %d\n",ipsec_eip97_params.auth_algo);
	ipsec_eip97_params.authsize =(xfrm_sa->aalg->alg_key_len + 7)/8; // mpe_hal_get_auth_size(xfrm_sa->aalg->alg_name);
	ipsec_eip97_params.auth_keysize = (xfrm_sa->aalg->alg_key_len + 7)/8;
	ipsec_eip97_params.auth_key = &xfrm_sa->aalg->alg_key[0];

	if(dir == LTQ_SAB_DIRECTION_INBOUND ) {

		ipsec_eip97_params.cryptosize = 80; //xfrm_sa->ealg->alg_key_len;
		ipsec_eip97_params.data_len = 80; //xfrm_sa->ealg->alg_key_len;
	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND ) {

		ipsec_eip97_params.cryptosize = 16; //xfrm_sa->ealg->alg_key_len;
		ipsec_eip97_params.data_len = 16; //xfrm_sa->ealg->alg_key_len;
	}

#if 1
	printk("Crypto Key length: %d\n",ipsec_eip97_params.crypto_keysize );
                        for (i=0; i< ipsec_eip97_params.crypto_keysize ; i++) {
                                printk("%2x",(ipsec_eip97_params.crypto_key[i]) & 0xFF);
                                if( i != 0 && i%8 == 0)
                                        printk("  ");
                        }

	if(ltq_ipsec_setkey(&ipsec_eip97_params) != 0 ) {
		printk("<%s> SetKey failed!!!\n",__FUNCTION__);
		return PPA_FAILURE;
	}
	if(ltq_get_ipsec_token(&ipsec_eip97_params) != 0) {
		printk("<%s> Get Ipsec Token failed !!!\n",__FUNCTION__);
		mpe_hal_dump_ipsec_eip97_params_info(&ipsec_eip97_params);
		return PPA_FAILURE;
	}
#endif

	mpe_hal_dump_ipsec_eip97_params_info(&ipsec_eip97_params);
	if(dir == LTQ_SAB_DIRECTION_INBOUND ) {
		memcpy(&crypto_ipsec_params_in[tunnel_index], &ipsec_eip97_params, sizeof(struct ltq_crypto_ipsec_params));
	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND ) {
		memcpy(&crypto_ipsec_params_out[tunnel_index], &ipsec_eip97_params, sizeof(struct ltq_crypto_ipsec_params));
	}
	printk("Context %x Token %x\n",ipsec_eip97_params.SA_buffer, ipsec_eip97_params.token_buffer);
	printk("Crypto Offs:%d Pad Offs:%d\n",ipsec_eip97_params.crypto_offs,ipsec_eip97_params.pad_offs[0]);
	return mpe_hal_construct_ipsec_buffer(dir, tunnel_index, &ipsec_eip97_params);

}

int32_t mpe_hal_ipsec_active_tunnel_get()
{
	int32_t i, count=0;

	for(i=0; i< IPSEC_TUN_MAX; i++)
	{
		if((g_GenConf->ipsec_output_flag[i] == 1) && (g_GenConf->ipsec_input_flag[i] == 1))
         		count++;	

	}
	return count;
}

int32_t mpe_hal_delete_ipsec_buffer(int32_t dir, int32_t tunlId)
{

	if(dir == LTQ_SAB_DIRECTION_INBOUND ){
		g_GenConf->ipsec_input_flag[tunlId] = 0;	
		memset(&g_GenConf->ipsec_input[tunlId], 0, sizeof(struct ipsec_info));

	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND) {
		g_GenConf->ipsec_output_flag[tunlId] = 0;	
		memset(&g_GenConf->ipsec_output[tunlId], 0, sizeof(struct ipsec_info));
	} else {
		printk("Wrong SA Direction !!!\n");
		return PPA_FAILURE;
	}
	return PPA_SUCCESS;
}

int32_t mpe_hal_del_ipsec_tunl_entry(PPE_ROUTING_INFO *route)
{
	int32_t tunnel_index, dir;

	tunnel_index = route->tnnl_info.tunnel_idx;

	printk("<%s> <%d> Tunnel Index %d\n",__FUNCTION__,__LINE__, tunnel_index);
	if ((tunnel_index < 0) || (tunnel_index > IPSEC_TUN_MAX))
		return PPA_FAILURE;

	dir = g_tunnel_table[tunnel_index]->tunnel_info.ipsec_hdr.dir;

	mpe_hal_delete_ipsec_buffer(dir, tunnel_index);
	if(dir == LTQ_SAB_DIRECTION_INBOUND ) {
		ltq_destroy_ipsec_sa(&crypto_ipsec_params_in[tunnel_index]);
	} else if(dir == LTQ_SAB_DIRECTION_OUTBOUND ) {
		ltq_destroy_ipsec_sa(&crypto_ipsec_params_out[tunnel_index]);
	}

	//if(mpe_hal_ipsec_active_tunnel_get() == 0)
	return PPA_SUCCESS;
}

struct module module_loopback;
static struct net_device *loop_dev;
struct loopdev_priv {
	struct module           *owner;
	dp_subif_t              dp_subif;
	int32_t                 dev_port;
	int32_t                 id;
};
static int loopdev_init(struct net_device *dev)
{
	return 0;
}
static struct net_device_ops loopdev_ops = {
        .ndo_init           = loopdev_init,

};

static void loop_setup(struct net_device *dev)
{
	ether_setup(dev);/*  assign some members */
	return;
}
static int32_t mpe_hal_set_ipsec_loopback_connectivity(void)
{
	char ifname[IFNAMSIZ];
	uint32_t dp_port_id;
	dp_cb_t cb={0};
	struct loopdev_priv *priv;
#if 0
	cbm_queue_map_entry_t q_map = {0};
#endif
	
	printk("<%s> Enter\n",__FUNCTION__);
	snprintf(ifname, sizeof(ifname), "loopdev%d", 0);

        loop_dev =
                alloc_netdev(sizeof(struct loopdev_priv), ifname, loop_setup);

        if (loop_dev == NULL) {
                printk("alloc_netdev fail\n");
                return -1;
        }

	loop_dev->dev_addr[0] = 0x00;
	loop_dev->dev_addr[1] = 0x00;
	loop_dev->dev_addr[2] = 0x00;
	loop_dev->dev_addr[3] = 0x00;
	loop_dev->dev_addr[4] = 0x00;
	loop_dev->dev_addr[5] = 0x92;
	priv = netdev_priv(loop_dev);

	loop_dev->netdev_ops = &loopdev_ops;
	if (register_netdev(loop_dev)) {
		free_netdev(loop_dev);
		loop_dev = NULL;
		printk("register device \"%s\" fail ??\n", ifname);
	} else {
		printk("add \"%s\" successfully\n", ifname);
		//priv = netdev_priv(loop_dev);
	}


	priv->owner = &module_loopback; 
	priv->id =1;
	snprintf(priv->owner->name, sizeof(priv->owner->name), "modloop%d", 0);
	
	dp_port_id  = dp_alloc_port(priv->owner, loop_dev, 0, 0, NULL, DP_F_LOOPBACK);
	if(dp_port_id == DP_FAILURE) {
		printk("%s: dp_alloc_port failed \n", __func__);
		return -ENODEV;
	}
	if (dp_register_dev (priv->owner,  dp_port_id, &cb, 0) != DP_SUCCESS) {

		printk("%s: dp_register_dev failed \n", __func__);
		return -ENODEV;
	}
	priv->dev_port = dp_port_id;
	tmu_hal_setup_dp_ingress_connectivity(loop_dev, dp_port_id);

	printk("Redirect Port %d is configured\n",dp_port_id);

#if 0
	/** Configure the QMAP table to connect to the  CPU MPE Port*/
	memset(&q_map, 0, sizeof(cbm_queue_map_entry_t));

	q_map.ep = dp_port_id;

	q_map_mask |= 
		CBM_QUEUE_MAP_F_EN_DONTCARE |
		CBM_QUEUE_MAP_F_DE_DONTCARE |
		CBM_QUEUE_MAP_F_MPE1_DONTCARE |
		CBM_QUEUE_MAP_F_MPE2_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_H_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_L_DONTCARE |
		CBM_QUEUE_MAP_F_TC_DONTCARE ;
	cbm_queue_map_set(tmu_res->tmu_q, &q_map, q_map_mask);
#endif

	g_GenConf->tunnel_redir_port = dp_port_id;
	return PPA_SUCCESS;
}

static int32_t mpe_hal_remove_ipsec_loopback_connectivity(void)
{
	uint32_t dp_port_id;
	dp_cb_t cb={0};

	printk("Remove IPSEC loopback connectivity for the interface %s: redirect port %d\n",module_loopback.name,g_GenConf->tunnel_redir_port);
	if (dp_register_dev (&module_loopback, g_GenConf->tunnel_redir_port, &cb, DP_F_DEREGISTER) != DP_SUCCESS) {

		printk("%s: dp_register_dev failed \n", __func__);
		return -ENODEV;
	}
	if(dp_alloc_port(&module_loopback, loop_dev, g_GenConf->tunnel_redir_port, 0, NULL, DP_F_DEREGISTER) != DP_SUCCESS) {

		printk("dp_unregister_dev failed for port_id %d", g_GenConf->tunnel_redir_port);
	}
	tmu_hal_remove_dp_ingress_connectivity(loop_dev, g_GenConf->tunnel_redir_port);

	unregister_netdev(loop_dev);
	free_netdev(loop_dev);

	g_GenConf->tunnel_redir_port = 0;
	return PPA_SUCCESS;
}

uint32_t mpe_hal_get_seq_num_offset_from_ctx(uint32_t *sa_buffer, uint32_t buf_len, uint32_t spi)
{
	int32_t i=0;
	for(i=0; i<buf_len; i++)
        {
                //printk("SA[%d] = %x\n",i,sa_buffer[i]);
		if(sa_buffer[i] == spi)
			break;
        }
	return i+1;
}

void mpe_hal_dump_context(uint32_t *sa_buffer, uint32_t buf_len)
{
	int32_t i=0;
	for(i=0; i<buf_len; i++)
        {
                printk("SA[%d] = %x\n",i,sa_buffer[i]);
        }
	printk("=============\n");
}

void mpe_hal_dump_token(uint32_t *token_buffer, uint32_t buf_len)
{
	int32_t i=0;
	for(i=0; i<buf_len; i++)
        {
                printk("Token[%d] = %x\n",i,token_buffer[i]);
        }
	printk("=============\n");
}

int32_t mpe_hal_dump_ipsec_tunnel_info(int32_t tun_id)
{
	uint32_t ctx_size;
	uint32_t *ctx = NULL;
	uint32_t spi =0, seq_offset=0;

	printk("Tuunnel Id: %d\n",tun_id);
	if(g_GenConf->ipsec_output_flag[tun_id] == 1)
	{
		printk("Outbound:\n");
		printk("Token length: %d\n",g_GenConf->ipsec_output[tun_id].cd_info.dw0.acd_size);
		if(g_GenConf->ipsec_output[tun_id].cd_info.dw0.acd_size !=0)
			mpe_hal_dump_token(g_GenConf->ipsec_output[tun_id].cd_info.dw2.acd_ptr, 
						g_GenConf->ipsec_output[tun_id].cd_info.dw0.acd_size);
		ctx = (g_GenConf->ipsec_output[tun_id].cd_info.dw5.ctx_ptr);
		if(ctx != NULL) {
			ctx_size = (ctx[0] & MPE_HAL_MASK_CTX_SIZE);
			ctx_size = ctx_size >> 8;
			printk("Context Size: %d\n",ctx_size+2);
			if(ctx_size != 0)
				mpe_hal_dump_context(g_GenConf->ipsec_input[tun_id].cd_info.dw5.ctx_ptr, ctx_size+2);
			printk("crypto_instr_offset: %d  pad_instr_offset:%d \n",
				g_GenConf->ipsec_output[tun_id].crypto_instr_offset,g_GenConf->ipsec_output[tun_id].pad_instr_offset);
			mpe_hal_get_ipsec_spi( tun_id, LTQ_SAB_DIRECTION_INBOUND, &spi);
			seq_offset = mpe_hal_get_seq_num_offset_from_ctx(g_GenConf->ipsec_output[tun_id].cd_info.dw5.ctx_ptr, ctx_size+2, spi );
			printk("Sequence number Offset %d\n", seq_offset);
		}
		printk("DW3: ");
		printk("len:[%d] ip:[%d] cp:[%d] ct:[%d] rc:[%d] too:[%d] c:[%d] iv:[%d] u:[%d] type:[%d]\n", 
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.len,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.ip,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.cp,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.ct,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.rc,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.too,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.c,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.iv,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.u,
				g_GenConf->ipsec_output[tun_id].cd_info.dw3.type);
	}

	if(g_GenConf->ipsec_input_flag[tun_id] == 1)
	{
		printk("Inbound:\n");
		printk("Token length: %d\n",g_GenConf->ipsec_input[tun_id].cd_info.dw0.acd_size);
		if(g_GenConf->ipsec_input[tun_id].cd_info.dw0.acd_size !=0)
			mpe_hal_dump_token(g_GenConf->ipsec_input[tun_id].cd_info.dw2.acd_ptr, 
						g_GenConf->ipsec_input[tun_id].cd_info.dw0.acd_size);
		ctx = (g_GenConf->ipsec_input[tun_id].cd_info.dw5.ctx_ptr);
		if(ctx != NULL) {
			ctx_size = (ctx[0] & MPE_HAL_MASK_CTX_SIZE);
			//printk("CW0 : %x Context Size: %d\n",ctx[0], ctx_size);
			ctx_size = ctx_size >> 8;
			printk("Context Size: %d\n",ctx_size+2);
			if(ctx_size != 0)
				mpe_hal_dump_context(g_GenConf->ipsec_input[tun_id].cd_info.dw5.ctx_ptr, ctx_size+2);
			printk("crypto_instr_offset: %d  pad_instr_offset:%d \n",
				g_GenConf->ipsec_input[tun_id].crypto_instr_offset,g_GenConf->ipsec_input[tun_id].pad_instr_offset);
			mpe_hal_get_ipsec_spi( tun_id, LTQ_SAB_DIRECTION_INBOUND, &spi);
			seq_offset = mpe_hal_get_seq_num_offset_from_ctx(g_GenConf->ipsec_input[tun_id].cd_info.dw5.ctx_ptr, ctx_size+2, spi );
			printk("Sequence number Offset %d\n", seq_offset);
		}
		printk("DW3: ");
		printk("len:[%d] ip:[%d] cp:[%d] ct:[%d] rc:[%d] too:[%d] c:[%d] iv:[%d] u:[%d] type:[%d]\n", 
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.len,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.ip,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.cp,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.ct,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.rc,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.too,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.c,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.iv,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.u,
				g_GenConf->ipsec_input[tun_id].cd_info.dw3.type);

	}
	return PPA_SUCCESS;
}

int32_t mpe_hal_get_ring_id(int32_t *id)
{
	int32_t i=0;
	for(i=0; i<MAX_RING;i++)
	{
		//printk("<%s> i: %d\n",__FUNCTION__,i);
		if(ring_id_pool[i] == -1)
		{
			*id = i;
			//printk("<%s> i:%d Ring Id: %d\n",__FUNCTION__, i, *id);
			ring_id_pool[i] = i;
			return PPA_SUCCESS;
		}
	}
	return PPA_FAILURE;
}

int32_t mpe_hal_free_ring_id(int32_t id)
{
	ring_id_pool[id] == -1;
	return PPA_SUCCESS;
}

int32_t mpe_hal_ipsec_test()
{

	int32_t dir;
	PPA_XFRM_STATE xfrm_sa;
	PPA_XFRM_STATE *xfrm, *xfrm_out;
	PPE_ROUTING_INFO route;
	struct xfrm_algo_auth   *xfrm_auth = NULL;
        struct xfrm_algo        *xfrm_crypt=NULL;
  	ppa_tunnel_entry *t_entry = NULL;


	memset(&route, 0, sizeof(PPE_ROUTING_INFO));
	memset(&xfrm_sa, 0, sizeof(PPA_XFRM_STATE));


	xfrm = (PPA_XFRM_STATE *) ppa_malloc(sizeof(PPA_XFRM_STATE));
	xfrm_crypt = (struct xfrm_algo *) ppa_malloc(sizeof(struct xfrm_algo));
	xfrm_auth = (struct xfrm_algo_auth *) ppa_malloc(sizeof(struct xfrm_algo_auth));

	route.entry = 0;
	route.f_is_lan = 1;
	route.tnnl_info.tunnel_idx = 2;
	route.tnnl_info.tunnel_type = TUNNEL_TYPE_IPSEC;


	xfrm->props.mode = LTQ_SAB_IPSEC_TUNNEL;
	xfrm->id.spi = 0x40002016;
	xfrm->id.proto = 50;

	strcpy(xfrm_crypt->alg_name, "cbc(aes)");
	xfrm_crypt->alg_key_len = 128; // 4 * 4;
	xfrm_crypt->alg_key[0] = "c994ab76 99a0a6d2 7667ab17 2b00f96f";

	xfrm->ealg = xfrm_crypt;

	strcpy(xfrm_auth->alg_name, "hmac(sha1)");
	xfrm_auth->alg_key_len = 160; //5 * 4;
	xfrm_auth->alg_key[0] = "3b3b34bb 069198a7 a63a1b90 a45d1c27 9c3f4e30";

	xfrm->aalg = xfrm_auth;

	t_entry = (ppa_tunnel_entry *) ppa_malloc(sizeof(ppa_tunnel_entry));
    	if (t_entry) {

		t_entry->tunnel_type = TUNNEL_TYPE_IPSEC;
		t_entry->hal_buffer = NULL;
		g_tunnel_table[route.tnnl_info.tunnel_idx] = t_entry;
	}


	t_entry->tunnel_info.ipsec_hdr.dir = LTQ_SAB_DIRECTION_INBOUND;
	t_entry->tunnel_info.ipsec_hdr.inbound = xfrm;
	
	mpe_hal_add_ipsec_tunl_entry(&route);

	xfrm_out = (PPA_XFRM_STATE *) ppa_malloc(sizeof(PPA_XFRM_STATE));
	xfrm_out->props.mode = LTQ_SAB_IPSEC_TUNNEL;
	xfrm_out->id.spi = 0x069f2b79;
	xfrm_out->id.proto = 50;

	strcpy(xfrm_crypt->alg_name, "cbc(aes)");
	xfrm_crypt->alg_key_len = 128; // 4 * 4;
	xfrm_crypt->alg_key[0] = "0a3a5878 1831add8 5e5ee250 12569e35";

	xfrm_out->ealg = xfrm_crypt;

	strcpy(xfrm_auth->alg_name, "hmac(sha1)");
	xfrm_auth->alg_key_len = 160; //5 * 4;
	xfrm_auth->alg_key[0] = "79f4877b aabe4bf4 b72c0a33 b7c11924 8d0075d3";

	xfrm_out->aalg = xfrm_auth;

	t_entry->tunnel_info.ipsec_hdr.dir = LTQ_SAB_DIRECTION_OUTBOUND;
	t_entry->tunnel_info.ipsec_hdr.outbound = xfrm_out;
	
	mpe_hal_add_ipsec_tunl_entry(&route);

	return PPA_SUCCESS;
}
#endif

int32_t mpe_hal_get_session_bytes(uint32_t cmb_tbl_typ, uint32_t sess_index, uint32_t * f_bytes)
{
	int j;
	unsigned long long sum;
	struct session_mib * bases;		
	
	sum = 0;
	for(j=0;j < MAX_WORKER_NUM;j++) {
		bases = (struct session_mib *)(g_GenConf->fw_sess_mib_tbl_base[cmb_tbl_typ][j] + sess_index * sizeof(struct session_mib));
		sum+=bases->mib.bytes;
	}
       // printk("Bytes:  %012llu\n", sum);

	*f_bytes = sum;
	return PPA_SUCCESS;
}

int32_t mpe_hal_get_session_hit_cnt(uint32_t cmb_tbl_typ, uint32_t sess_index, uint32_t * f_hit)
{
	int j;
	uint8_t *baseh= NULL;
	unsigned char sum;
	unsigned char hn[MAX_WORKER_NUM] = {0};
	sum = 0;
	*f_hit = 0;
	for(j=0;j < MAX_WORKER_NUM;j++) {
		baseh = (uint8_t *)(g_GenConf->fw_sess_hit_tbl_base[cmb_tbl_typ][j] + sess_index * sizeof(uint8_t));
		hn[j] = *baseh;
		sum += hn[j];
	}
//	printk("Total Hit count %d\n",sum);
	if(sum)
		*f_hit = 1;

	//Now clear it.
	for(j=0;j < MAX_WORKER_NUM;j++) 
		memset((void *)g_GenConf->fw_sess_hit_tbl_base[cmb_tbl_typ][j] + sess_index * sizeof(uint8_t), 0, sizeof(uint8_t));

	return PPA_SUCCESS;
}

int32_t mpe_hal_get_session_acc_bytes(PPE_ROUTING_INFO *route)
{
	uint32_t fHit;
	uint32_t bytes;
//	printk("Get the Hit status for session index %d of IP type %d\n",route->entry,route->src_ip.f_ipv6);
	if(route->src_ip.f_ipv6) {
		mpe_hal_get_session_hit_cnt(1, route->entry, &fHit);
		mpe_hal_get_session_bytes(1, route->entry, &bytes);
	} else {
		mpe_hal_get_session_hit_cnt(0, route->entry, &fHit);
		mpe_hal_get_session_bytes(0, route->entry, &bytes);
	}
	route->f_hit = fHit;
	route->bytes = bytes;
	return PPA_SUCCESS;
}

int32_t mpe_hal_test_and_clear_hit_stat(PPE_ROUTING_INFO *route)
{
	printk("mpe_hal_test_and_clear_hit_stat\n");	
	if(route->src_ip.f_ipv6) {
		mpe_hal_get_session_hit_cnt(1, route->entry, &route->f_hit);
	} else {
		uint32_t fHit;
		mpe_hal_get_session_hit_cnt(0, route->entry, &fHit);
		route->f_hit = fHit;
	}
	return PPA_SUCCESS;
}

int add_route_entry(uint8_t *wanSrcMAC, uint8_t *wanDstMAC, uint8_t f_ipv6,
		  IP_ADDR *Src_IP, IP_ADDR *Dst_IP, uint8_t f_tcp, uint32_t SrcPort, uint32_t DstPort,
		  uint8_t rt_type, IP_ADDR *NATIPaddr, uint32_t TcpUdpPort,
		  uint8_t f_islan, uint32_t mtu, uint32_t portmap, uint16_t subifid,
		  uint8_t pppoe_mode, uint32_t pppoe_sessionid)
{
    int ret = PPA_SUCCESS;
    PPE_ROUTING_INFO route;

    ppa_memset( &route, 0, sizeof(route));
   
    route.route_type = rt_type;

    route.new_ip.f_ipv6 = f_ipv6; 
    memcpy(&route.new_ip.ip,NATIPaddr,sizeof(IP_ADDR)); 
    route.new_port = TcpUdpPort; 
    
    route.pppoe_mode = pppoe_mode;
    route.pppoe_info.pppoe_session_id = pppoe_sessionid;  
 
    route.mtu_info.mtu = mtu;
    
    route.dest_list = portmap; 
    route.dest_subif_id = subifid;
   
    route.f_is_lan = f_islan; 
 
    memcpy(route.src_mac.mac,wanSrcMAC,sizeof(uint8_t)*PPA_ETH_ALEN);
    memcpy(route.new_dst_mac,wanDstMAC,sizeof(uint8_t)*PPA_ETH_ALEN); 

    route.dst_port = DstPort;
    route.src_port = SrcPort;
    route.src_ip.f_ipv6 = f_ipv6;
    route.dst_ip.f_ipv6 = f_ipv6;
    memcpy(&route.src_ip.ip,Src_IP, sizeof(IP_ADDR));
    memcpy(&route.dst_ip.ip,Dst_IP, sizeof(IP_ADDR));
    route.f_is_tcp = f_tcp; 
    

    if((ret = mpe_hal_add_routing_entry(&route)) < PPA_SUCCESS) {
 	printk(KERN_INFO "add_routing_entry returned Failure %d\n", ret);                                  
    }
   
    printk(KERN_INFO "add_routing_entry Success sip=%u dip=%u sp=%u dp=%u\n", Src_IP->ip, Dst_IP->ip, SrcPort, DstPort);                                  
    return ret;
}


int ipv4_routing_test(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *lanip = "192.168.1.20";
    char *brip = "192.168.1.1";
    char *wanip = "100.100.105.1";
    char *gwip = "100.100.100.17";

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, br0_IP, eth1_IP, gw_IP;

    uint32_t lanPort =5000;
    uint32_t wanPort = 80;
    uint32_t TcpUdpPort = 4000;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_NAPT; 
    uint32_t mtu = 1500;

    lanpc_IP.ip = in_aton(lanip);
    br0_IP.ip	= in_aton(brip);
    eth1_IP.ip	= in_aton(wanip);
    gw_IP.ip	= in_aton(gwip);

    // LAN to WAN session
    printk("Add LAN to WAN session\n");
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 1;
    portmap = 0x8000;
    session_index = add_route_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &eth1_IP, TcpUdpPort, f_islan, mtu, portmap, 0, 0, 0); 

    //routing_entry_read(session_index);
    
    // WAN to LAN session
    printk("Add WAN to LAN session\n");
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 0;
    portmap = 0x04; // lan port 2
    session_index = add_route_entry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &eth1_IP, f_tcp, wanPort, TcpUdpPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, mtu, portmap, 0, 0, 0);

    lanPort =5001;
    wanPort = 100;
    TcpUdpPort = 4001;

    // LAN to WAN session
    printk("Add LAN to WAN session\n");
    f_ipv6 = 0;
    f_tcp = 1;
    f_islan = 1;
    portmap = 0x8000;
    session_index = add_route_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &eth1_IP, TcpUdpPort, f_islan, mtu, portmap, 0, 0, 0); 
    return 0;
}
#if 1
int ipv6_routing_test(void)
{
    int session_index = -1;

    u8 lanpcMAC[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x20}; // lan pc interface mac
    u8 br0MAC[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac
    u8 eth1MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xee}; // eth1 interface mac
    u8 gwMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x16}; // smb wan port mac

    char *v6lan = "3ffe:507:0:1:200:86ff:fe05:80da";
    char *v6wan = "3ffe:501:410:0:2c0:dfff:fe47:33e";  

    uint8_t f_ipv6, f_tcp, f_islan;
    uint32_t portmap;
    
    IP_ADDR lanpc_IP, gw_IP;

    uint32_t lanPort =1022;
    uint32_t wanPort = 22;
    uint32_t TcpUdpPort = 0;
    
    uint8_t rt_type = PPA_ROUTE_TYPE_IPV4; 
    uint32_t mtu = 1500;
    const char *end;

    in6_pton(v6lan,-1,(void*)&lanpc_IP.ip6,-1,&end);
    in6_pton(v6wan,-1,(void*)&gw_IP.ip6,-1,&end);
    
    // LAN to WAN session
    f_ipv6 = 1;
    f_tcp = 1;
    f_islan = 1;
    portmap = 1 << 15;
    session_index = add_route_entry(eth1MAC, gwMAC, f_ipv6, &lanpc_IP, &gw_IP, f_tcp, lanPort, wanPort, 
							rt_type, &lanpc_IP, TcpUdpPort, f_islan, mtu, portmap, 0, 0, 0); 

    //routing_entry_read(session_index);
    
    // WAN to LAN session
    f_islan = 0;
    portmap = 1 << 2;

    session_index = add_route_entry(br0MAC, lanpcMAC, f_ipv6, &gw_IP, &lanpc_IP, f_tcp, wanPort, lanPort, 
						    rt_type, &lanpc_IP, lanPort, f_islan, mtu, portmap, 0, 0, 0);
    //routing_entry_read(session_index);
    printk(KERN_INFO "ipv6 route added.. !\n");
	
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

int multicast_routing_test(void)
{
    PPE_MC_INFO mc;
    int session_index = -1;

    u8 mcMAC[6] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x10}; // multicast mac
    u8 br0MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}; // br0 port mac

    char *mcastip = "235.0.10.10";
    char *gwip = "100.100.100.1";

    uint8_t f_ipv6;
    uint32_t portmap=0, groupid=1, numvap=0, subifid=0;

    IP_ADDR gw_IP, mc_IP;

    mc_IP.ip    = in_aton(mcastip);
    gw_IP.ip    = in_aton(gwip);

    // LAN to WAN session
    f_ipv6 = 0;
    portmap = 1;
    portmap |= (1 << 2);
    subifid = 0;
    numvap = 0;

    memset(&mc, 0, sizeof(PPE_MC_INFO));
    mc.p_entry = 32;
    mc.group_id = 5;
    mc.dest_list |= 1 << 8;
    mc.dest_list |= 1 << 9;

    mc.dest_subif[8] |= 1 << 2;
    mc.dest_subif[8] |= 1 << 3;
    mc.dest_subif[8] |= 1 << 4;


    mc.dest_subif[9] |= 1 << 6;
    mc.dest_subif[9] |= 1 << 12;

    mc.sessionAction = (uint32_t) kmalloc(sizeof(struct session_action), GFP_KERNEL);

    mpe_hal_add_wan_mc_entry(&mc);


    printk(KERN_INFO "multicast route added.. !\n");
    return 0;    // Non-zero return means that the module couldn't be loaded.
}

#endif

#ifdef CONFIG_LTQ_PPA_MPE_IP97
int32_t  IsValidRingId(uint32_t ring_id)
{
	if((ring_id > 0) || (ring_id <= MAX_RING))
		return 1;
	else
		return 0;
}

int32_t mpe_hal_get_ipsec_tunnel_mib(IPSEC_TUNNEL_MIB_INFO *mib_info)
{
	unsigned int i, rx_pkt_sum, tx_pkt_sum, tx_byte, rx_byte;
	struct mpe_itf_mib * base;
        unsigned int rxp[MAX_WORKER_NUM] = {0};
        unsigned int txp[MAX_WORKER_NUM] = {0};

	printk("Get tunnel mib for tunnel id %d\n",mib_info->tunnel_id);
	rx_pkt_sum=0;
	tx_pkt_sum=0;
	tx_byte=0;
	rx_byte=0;
#if 1
	for(i=0; i<MAX_WORKER_NUM; i++) {
		base = (struct mpe_itf_mib *)(g_GenConf->mib_itf_tbl_base[i] + (mib_info->tunnel_id * 2)*sizeof(struct mpe_itf_mib));
		//rxp[i] = base->rx_mib.pkt;
		txp[i] = base->tx_mib.pkt;
		//rx_pkt_sum += rxp[i];
		tx_pkt_sum += txp[i];
		tx_byte+=base->tx_mib.bytes;
		//rx_byte+=base->rx_mib.bytes;
        }
#endif
        for(i=0;i < MAX_MPE_TC_NUM;i++) {
		if(!g_GenConf->hw_res[i].flag)
			continue;			
		if(IsValidRingId(g_GenConf->hw_res[i].e97_ring_id)) {
			rxp[i] = g_GenConf->tc_mib[i].dec_pkt[mib_info->tunnel_id];
			//txp[i] = g_GenConf->tc_mib[i].enc_pkt[mib_info->tunnel_id];
			rx_pkt_sum += rxp[i];
			//tx_pkt_sum += txp[i];
		}
        }
	mib_info->rx_pkt_count = rx_pkt_sum;
	mib_info->tx_pkt_count = tx_pkt_sum;
	mib_info->tx_byte_count = tx_byte;
	mib_info->rx_byte_count = rx_pkt_sum;
	printk("<%s> rx_pkt_count [%d] tx_pkt_count:[%d] tx_byte_count: [%d] rx_byte_count: [%d]\n",
			__FUNCTION__,mib_info->rx_pkt_count,mib_info->tx_pkt_count,mib_info->tx_byte_count,mib_info->rx_byte_count);
	return PPA_SUCCESS;
}
#endif

/* ======================================================================================================= */

void inline gic_disable_edge(uint32_t irq_no)
{
	GIC_SH_WEDGE_REG = irq_no;  // Clear this interrupt edge
	//GIC_SH_WEDGE_CLR(irq_no);
	//gic_disable_interrupt(irq_no);
	return;
}


static irqreturn_t print_content(void)
{
	struct tc_hw_res *hw_res = &g_GenConf->hw_res[0];
	struct mperesp_itc *reg = (struct mperesp_itc *)(hw_res->MpeDebugRespReg);
	struct mperesp_itc resp_itc;

	if(CHECK_BIT(REG32(MPE_RESP_INT_STS), hw_res->MpeDebugQid))
	{	
		resp_itc.reg0 = reg->reg0;
		resp_itc.reg1 = reg->reg1;

		REG32(MPE_RESP_INT_STS) = (1<<hw_res->MpeDebugQid);

		gic_disable_edge(hw_res->MpeDebugRespRegIrq);
		if(g_GenConf->g_mpe_dbg_enable)
			printk("%s",(char *)(resp_itc.reg1.fdf));

		cbm_buffer_free(0,resp_itc.reg1.fdf,0);

		reg->reg0.vld = MPE_FW; // clear the valid bit				
	}	
	return IRQ_HANDLED;
}


static int mpe_hal_to_vmb_callback_hdlr(uint32_t status)
{
  switch (status)  {
    case FW_RESET:
        {//MPE FW hangs. So VMB has already configured the VPE in bootloader mode.
		 // Start the MPE FW again.
	  //if(mpe_hal_run_fw( MAX_CPU, g_MpeFwHdr.worker_info.min_num) == PPA_FAILURE)
	  if(mpe_hal_run_fw( MAX_CPU, g_MpeFwHdr.worker_info.min_tc_num) == PPA_FAILURE)
          {
            printk("Cannot run MPE FW.\n");
            return PPA_FAILURE;
          }   		 
        }
    default:
        printk("mpe_hal_to_vmb_callback_hdlr not support status 0x%x\n", status );
        return PPA_FAILURE;
    }

  return PPA_SUCCESS;
}

static void mpe_hal_prepare_tc_launch(uint8_t tc_num, uint8_t tcType, struct tc_hw_res *tc_priv, TC_launch_t *tc_launch)
{
	tc_launch->tc_num = tc_num;
	tc_launch->mt_group = 1 ;
	if(tcType == TYPE_DIRECTLINK)
		tc_launch->start_addr = (uint32_t)g_GenConf->fw_hdr.dl_info.start_addr;
	else
		tc_launch->start_addr = (uint32_t)g_GenConf->fw_hdr.worker_info.start_addr;
	//printk("start address of tc %d is %x\n",tc_num,tc_launch->start_addr);
	//printk("logic mpe tc id %d \n",tc_priv->logic_mpe_tc_id);
	tc_launch->sp = (uint32_t)(g_MpeFw_stack_addr + g_GenConf->fw_hdr.fw_stack_priv_data_size * (tc_priv->logic_mpe_tc_id +1));
	//tc_launch->gp = ;
	//tc_launch->a0 =;
	tc_launch->state = TC_INACTIVE;
	tc_launch->priv_info =(uint32_t) tc_priv->logic_mpe_tc_id;	
	return;
}
static int mpe_hal_get_yield(uint8_t cpu_num, uint8_t tc_num)
{
	/* VMB manager will provide the API */
	return vmb_yr_get(cpu_num, 1);
}

static int mpe_hal_free_yield(uint8_t cpu_num, uint8_t tc_num)
{
	/* VMB manager will provide the API */
	vmb_yr_free(cpu_num, 1);
	return 0;
}

static int mpe_hal_get_ipi(uint8_t cpu)
{
	//return (FW_VMB_IPI1 - MIPS_GIC_IRQ_BASE);
	return fw_vmb_get_irq(cpu);
}

static int mpe_hal_get_vmbfwipi(uint8_t cpu)
{
	int32_t ret=0;
  	switch (cpu){
		case 1 :
			ret = VMB_CPU_IPI1;
			break;
		case 2 :
			ret = VMB_CPU_IPI2;
			break;
		case 3 :
			ret = VMB_CPU_IPI3;
			break;
		default :
			printk ("[%s]:[%d] Invalid CPU number !!!\n", __FUNCTION__, __LINE__);

	}
  	return ret;
}

static int32_t mpe_hal_get_semid(uint8_t cpu_num, uint8_t tc_num)
{
	int32_t semid = 0;
    	uint32_t sem_addr ;
	/* VMB manager will provide the API */
	semid= vmb_itc_sem_get(cpu_num, tc_num);
	//printk("<%s>sem id %d\n",__FUNCTION__,semid);
	if(semid != -VMB_EBUSY) {
		//printk("<%s>sem id %d\n",__FUNCTION__,semid);
		sem_addr = itc_sem_addr(semid);
    		sem_id_log[semid] = sem_addr; //keep for free later
		return sem_addr;
	}
		
	return PPA_FAILURE; 
}

uint32_t mpe_hal_free_semid(uint32_t semid_ptr)
{
    int i;
    if( !semid_ptr ) return 0;

    for(i=0; i<MAX_ITC_SEMID; i++ ) {
        if( sem_id_log[i] == semid_ptr ) {
            sem_id_log[i] = 0;
            break;
        }
    }

    if( i>= MAX_ITC_SEMID) /*not found*/
        return -1;

    vmb_itc_sem_free(i);

    return PPA_SUCCESS;
}


/*static int mpe_hal_get_qid(uint8_t cpu_num, uint8_t tc_num)
{
  return 1;
}*/

static void mpe_hal_config_pmac_port_len(void)
{
	int32_t i=0;
	for(i=0; i<MAX_PMAC_PORT;i++) {
		/*
		* Add PMCA to Port 1-6 and port 15. Rest of the port should have no PMAC
		*/
		if((( i > 0 && i < 7)) || (i == 15)) {

			g_GenConf->port[i].pmac_len_out=8;
			g_GenConf->port[i].port_type = PMAC_PORT_ETH;
		}	
		else
		{
			g_GenConf->port[i].pmac_len_out=0;
			g_GenConf->port[i].port_type = PMAC_DIRECTPATH_DEV;
		}
	}

}
static int32_t mpe_hal_set_fw_connectivity(void)
{
	int32_t q_map_mask = 0;
	uint32_t nof_of_tmu_ports;
	cbm_tmu_res_t *tmu_res = NULL;
        dp_subif_t dp_subif={0};
	cbm_queue_map_entry_t q_map = {0};

	if(cbm_dp_port_resources_get(&dp_subif.port_id, &nof_of_tmu_ports, &tmu_res, DP_F_MPE_ACCEL) != 0)
		return PPA_FAILURE;

	printk("<%s> CPU TMU resources for MPE are Port:%d ,SB:%d, Q:%d \n",__FUNCTION__,tmu_res->tmu_port, tmu_res->tmu_sched, tmu_res->tmu_q);

	/** Configure the QMAP table to connect to the  CPU MPE Port*/
	memset(&q_map, 0, sizeof(cbm_queue_map_entry_t));

	q_map.ep = 0;
	q_map.mpe1 = 1;
	q_map.mpe2 = 0;
	//q_map.flowid = 0;

	q_map_mask |= 
		CBM_QUEUE_MAP_F_EN_DONTCARE |
		CBM_QUEUE_MAP_F_DE_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_H_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_L_DONTCARE |
		CBM_QUEUE_MAP_F_TC_DONTCARE ;
	cbm_queue_map_set(tmu_res->tmu_q, &q_map, q_map_mask);

#if 0
	memset(&q_map, 0, sizeof(cbm_queue_map_entry_t));

	q_map.ep = 0;
	q_map.mpe1 = 1;
	q_map.mpe2 = 0;
	//q_map.flowid = 1;

	q_map_mask |= 
		CBM_QUEUE_MAP_F_EN_DONTCARE |
		CBM_QUEUE_MAP_F_DE_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_DONTCARE |
		CBM_QUEUE_MAP_F_TC_DONTCARE ;
	cbm_queue_map_set(tmu_res->tmu_q, &q_map, q_map_mask);
#endif
	g_MPE_accl_mode = 1;
	kfree(tmu_res);
	return PPA_SUCCESS;
}


static int32_t mpe_hal_remove_fw_connectivity(void)
{
	int32_t num_entries , i=0;
	uint32_t nof_of_tmu_ports;
	cbm_tmu_res_t *tmu_res = NULL;
        dp_subif_t dp_subif={0};
	cbm_queue_map_entry_t *q_map_get = NULL;

	if(cbm_dp_port_resources_get(&dp_subif.port_id, &nof_of_tmu_ports, &tmu_res, DP_F_MPE_ACCEL) != 0)
		return PPA_FAILURE;

	printk("<%s> TMU resources for MPE are Port:%d ,SB:%d, Q:%d \n",__FUNCTION__,tmu_res->tmu_port, tmu_res->tmu_sched, tmu_res->tmu_q);

	cbm_queue_map_get(
		tmu_res->tmu_q,
		&num_entries, &q_map_get, 0);
	printk("<%s> MPE FW Queue %d has mapped entries %d \n",__FUNCTION__, tmu_res->tmu_q, num_entries);

	kfree(tmu_res);
	tmu_res = NULL;
	memset(&dp_subif, 0, sizeof(dp_subif_t));
	if(cbm_dp_port_resources_get(&dp_subif.port_id, &nof_of_tmu_ports, &tmu_res, 0) != 0) {
		printk("<%s> Fail to get port resource info !!!",__FUNCTION__);
		kfree(q_map_get);
		return PPA_FAILURE;
	}

	printk("<%s> CPU TMU resources are Port:%d ,SB:%d, Q:%d \n",__FUNCTION__,tmu_res->tmu_port, tmu_res->tmu_sched, tmu_res->tmu_q);
	printk("<%s> Remap to CPU Q: %d\n",__FUNCTION__,tmu_res->tmu_q);
	for(i = 0; i < num_entries; i++) {
		cbm_queue_map_set(tmu_res->tmu_q, &q_map_get[i], 0);;
	}

	g_MPE_accl_mode = 0;
	kfree(q_map_get);
	kfree(tmu_res);

	return PPA_SUCCESS;
}

#if 0
static void mpe_hal_add_dl_lookup(uint32_t ep)
{
	cbm_queue_map_entry_t q_map;
	int32_t q_map_mask = 0;

	uint32_t nof_of_tmu_ports;
	cbm_tmu_res_t *tmu_res;
        dp_subif_t dp_subif={0};

	dp_subif.port_id = ep;
	if(cbm_dp_port_resources_get(&dp_subif.port_id, &nof_of_tmu_ports, &tmu_res, DP_F_DIRECTLINK) != 0)
		return PPA_FAILURE;

	printk("<%s> PMAC port %d :TMU resources for DL are Port:%d ,SB:%d, Q:%d \n",
			__FUNCTION__,ep, tmu_res->tmu_port, tmu_res->tmu_sched, tmu_res->tmu_q);

	/** Configure the QMAP table to connect to the DL CPU Port*/
	memset(&q_map, 0, sizeof(cbm_queue_map_entry_t));

	q_map.ep = ep;
	q_map.dec = 0;
	q_map.enc = 0;
	q_map.mpe1 = 1;
	q_map.mpe2 = 0;

	q_map_mask |= 
		CBM_QUEUE_MAP_F_FLOWID_L_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_H_DONTCARE |
		CBM_QUEUE_MAP_F_TC_DONTCARE ;
	cbm_queue_map_set(tmu_res->tmu_q, &q_map, q_map_mask);

	kfree(tmu_res);
	return;
}

static void mpe_hal_remove_dl_lookup(uint32_t ep)
{
	cbm_queue_map_entry_t q_map;
	int32_t q_map_mask = 0;

	uint32_t nof_of_tmu_ports;
	cbm_tmu_res_t *tmu_res;
        dp_subif_t dp_subif={0};

	dp_subif.port_id = ep;
	if(cbm_dp_port_resources_get(&dp_subif.port_id, &nof_of_tmu_ports, &tmu_res, DP_F_DIRECTLINK) != 0)
		return PPA_FAILURE;

	printk("<%s> PAMC port %d :TMU resources for DL are Port:%d ,SB:%d, Q:%d \n",__FUNCTION__,ep, tmu_res->tmu_port, tmu_res->tmu_sched, tmu_res->tmu_q);


	/** Configure the QMAP table to connect to the DL CPU Port*/
	memset(&q_map, 0, sizeof(cbm_queue_map_entry_t));

	q_map.ep = ep;
	q_map.dec = 0;
	q_map.enc = 0;
	q_map.mpe1 = 1;
	q_map.mpe2 = 0;

	q_map_mask |= 
		CBM_QUEUE_MAP_F_FLOWID_L_DONTCARE |
		CBM_QUEUE_MAP_F_FLOWID_H_DONTCARE |
		CBM_QUEUE_MAP_F_TC_DONTCARE ;
	cbm_queue_map_set(tmu_res->tmu_q, &q_map, q_map_mask);

	kfree(tmu_res);
	return;
}
#endif


static int32_t mpe_hal_get_cbm_deq_port(uint8_t tcType)
{
	cbm_dq_port_res_t deq_port_res = {0};
	int32_t deq_port= -1;
	//dp_subif_t dp_subif={0};

	//printk("<%s> Get Dequeue Port resource for TC type %d\n",__FUNCTION__,tcType);
	if(tcType == TYPE_DIRECTLINK)
	{
		//printk("<%s> Get Dequeue Port resource for PMAC port %d\n",__FUNCTION__,g_dl1_pmac_port);
		if(cbm_dequeue_port_resources_get(g_dl1_pmac_port, &deq_port_res, DP_F_DIRECTLINK) != 0)
			return PPA_FAILURE;
	} else {
		if(cbm_dequeue_port_resources_get(0, &deq_port_res, DP_F_MPE_ACCEL) != 0)
			return PPA_FAILURE;
	}
	//printk("<%s> Dequeue Port is %d\n",__FUNCTION__,deq_port_res.deq_info->port_no);
	//deq_port_res.deq_info->port_no = 3;
	
	//printk("<%s> Dequeue Port is %d\n",__FUNCTION__,deq_port_res.deq_info->port_no);
	deq_port = deq_port_res.deq_info->port_no;
	kfree(deq_port_res.deq_info);
	return deq_port;
}



/****************DUMP for PROC ENTRIES**********************************************************/
int32_t mpe_hal_test(uint32_t testcase)
{
	switch(testcase) {
	case 1:
	{
		ipv4_routing_test();
		break;
	}
	case 2:
	{
		ipv6_routing_test();
		break;
	}
	case 3:
	{
		mpe_hal_add_hw_session_test();
		return PPA_SUCCESS;
	}
	case 4:
	{
		multicast_routing_test();
		return PPA_SUCCESS;
	}
	case 5:
	{

#ifdef CONFIG_LTQ_PPA_MPE_IP97
		mpe_hal_ipsec_test();
#endif
		return PPA_SUCCESS;
	}
	default:
		break;

	}
	return 0;

}

int32_t mpe_hal_display_ipv4_session_action(uint32_t current_ptr)
{
	uint32_t size, i;
	struct fw_compare_hash_auto_ipv4 *phtable;
	int v =0,vap_entry_index=0,dst_pmac_port_no = 0, pmac_port_num =0;
	struct vap_entry *ve = NULL;

	if(current_ptr > MAX_HW_SESSION_NUM) {
		printk("Index is exceeding the table size \n");
		return PPA_FAILURE;
	}
	size = 1; //sizeof( struct fw_compare_hash_auto_ipv4);
	phtable = ( struct fw_compare_hash_auto_ipv4 *)g_GenConf->fw_cmp_tbl_base[0]; 

	if((phtable + (current_ptr * size))->act == 0){
		printk("NULL Action Pointer !!!\n");
		return PPA_FAILURE;
	}

	printk("\nAction Table:\n");
	printk("Valid		        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->entry_vld);
	printk("Template buffer len        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->templ_len);
	printk("pkt_len_delta              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->pkt_len_delta);
	printk("traffic_class              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->traffic_class);
	printk("mtu                        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->mtu);
	printk("protocol                   = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->protocol);
	printk("routing_flag               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->routing_flag );
	printk("new_src_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_src_ip_en);
	printk("new_dst_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_dst_ip_en);
	printk("new_inner_dscp_en          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp_en);
	printk("pppoe_offset_en            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->pppoe_offset_en);
	printk("tunnel_ip_offset_en        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset_en);
	printk("tunnel_udp_offset_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset_en);
	printk("in_eth_iphdr_offset_en     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset_en);
	printk("sess_mib_ix_en             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix_en);
	printk("new_traffic_class_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_traffic_class_en);
	printk("tunnel_rm_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_rm_en);
	printk("meter_id1_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->meter_id1_en);
	printk("outer_dscp_mode            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->outer_dscp_mode);
	printk("meter_id0                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->meter_id0);
	printk("meter_id1                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->meter_id1);
	printk("new_inner_dscp             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp);
	printk("tunnel_type                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_type);
	printk("pppoe_offset               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->pppoe_offset);
	printk("tunnel_ip_offset           = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset);
	printk("in_eth_iphdr_offset        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset);
	printk("tunnel_udp_offset          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset);
	printk("key_en                     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->key_en);
	printk("rx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_num);
	printk("tx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_num);
	printk("sess_mib_ix                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix);
	printk("dst_pmac_port_num          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num);

	pmac_port_num = ((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num;
	/*multicast*/
	vap_entry_index=((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->mc_index;
	if(vap_entry_index) 
	{
		printk("mc_index                   = %d\n",vap_entry_index);
		for(i=0;i < pmac_port_num;i++) 
		{
			printk("dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
			dst_pmac_port_no = ((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i];
			if(dst_pmac_port_no)
			{	
				ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[dst_pmac_port_no] + (sizeof(struct vap_entry) * vap_entry_index));
				printk("mc_vap_num                 = 0x%x \n",ve->num);
				for(v=0; v < ve->num ;v++)
				{
					printk("mc_vap_list[%d]            = 0x%x \n",v,ve->vap_list[v]);
				}
				printk("\n");
			}
		}
	}
	else
	{
		for(i=0;i < pmac_port_num;i++)
			printk("dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
		for(i=0;i < pmac_port_num;i++)
			printk("uc_vap_list[%d]             = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->uc_vap_list[i]);
	}
	for(i=0;i < MAX_ITF_PER_SESSION;i++)
		printk("rx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_ix[i]);
	for(i=0;i < MAX_ITF_PER_SESSION;i++)
		printk("tx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_ix[i]);
	printk("new_src_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip4.ip));
	printk("new_dst_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_dst_ip.ip4.ip));
	printk("new_src_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_src_port);
	printk("new_dst_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_dst_port);
	printk("Template buffer:\n");
	printk("\t");
	for (i=0;i < ((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->templ_len;i++)  {
		printk("%02x ",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->templ_buf[i]);
	}
	printk("\n\n");
	return 0;

}

int32_t mpe_hal_display_ipv6_session_action(uint32_t current_ptr)
{
	uint32_t size, i;
	struct fw_compare_hash_auto_ipv6 *phtable;
	int v =0,vap_entry_index=0,dst_pmac_port_no = 0, pmac_port_num =0;
	struct vap_entry *ve = NULL;

	if(current_ptr > MAX_HW_SESSION_NUM) {
		printk("Index is exceeding the table size \n");
		return PPA_FAILURE;
	}
	size = 1; //sizeof( struct fw_compare_hash_auto_ipv4);
	phtable = ( struct fw_compare_hash_auto_ipv6 *)g_GenConf->fw_cmp_tbl_base[1]; 

	if((phtable + (current_ptr * size))->act == 0){
		printk("NULL Action Pointer !!!\n");
		return PPA_FAILURE;
	}

	printk("\nAction Table:\n");
	printk("Valid		        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->entry_vld);
	printk("Template buffer len        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->templ_len);
	printk("pkt_len_delta              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->pkt_len_delta);
	printk("traffic_class              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->traffic_class);
	printk("mtu                        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->mtu);
	printk("protocol                   = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->protocol);
	printk("routing_flag               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->routing_flag );
	printk("new_src_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip_en);
	printk("new_dst_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_dst_ip_en);
	printk("new_inner_dscp_en          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp_en);
	printk("pppoe_offset_en            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->pppoe_offset_en);
	printk("tunnel_ip_offset_en        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset_en);
	printk("tunnel_udp_offset_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset_en);
	printk("in_eth_iphdr_offset_en     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset_en);
	printk("sess_mib_ix_en             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix_en);
	printk("new_traffic_class_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_traffic_class_en);
	printk("tunnel_rm_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_rm_en);
	printk("meter_id1_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->meter_id1_en);
	printk("outer_dscp_mode            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->outer_dscp_mode);
	printk("meter_id0                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->meter_id0);
	printk("meter_id1                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->meter_id1);
	printk("new_inner_dscp             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp);
	printk("tunnel_type                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_type);
	printk("pppoe_offset               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->pppoe_offset);
	printk("tunnel_ip_offset           = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset);
	printk("in_eth_iphdr_offset        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset);
	printk("tunnel_udp_offset          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset);
	printk("key_en                     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->key_en);
	printk("rx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_num);
	printk("tx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_num);
	printk("sess_mib_ix                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix);
	printk("dst_pmac_port_num          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num);

	pmac_port_num = ((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num;
	/*multicast*/
	vap_entry_index=((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->mc_index;
	if(vap_entry_index) 
	{
		printk("mc_index                   = %d\n",vap_entry_index);
		for(i=0;i < pmac_port_num;i++) 
		{
			printk("dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
			dst_pmac_port_no = ((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i];
			if(dst_pmac_port_no)
			{	
				ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[dst_pmac_port_no] + (sizeof(struct vap_entry) * vap_entry_index));
				printk("mc_vap_num                 = 0x%x \n",ve->num);
				for(v=0; v < ve->num ;v++)
				{
					printk("mc_vap_list[%d]            = 0x%x \n",v,ve->vap_list[v]);
				}
				printk("\n");
			}
		}
	}
	else
	{
		for(i=0;i < pmac_port_num;i++)
			printk("dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
		for(i=0;i < pmac_port_num;i++)
			printk("uc_vap_list[%d]             = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->uc_vap_list[i]);
	}
	for(i=0;i < MAX_ITF_PER_SESSION;i++)
		printk("rx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_ix[i]);
	for(i=0;i < MAX_ITF_PER_SESSION;i++)
		printk("tx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_ix[i]);
#if 1
	printk("new_src_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip4.ip));
	printk("new_dst_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_dst_ip.ip4.ip));
#else 
	printk("new_src_ip                 = %x:%x", (((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[3] & 0xFF), ((((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[3]) & 0xFF00) >> 16 );

	printk(":%x:%x", (((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[2] & 0xFF), ((((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[2]) & 0xFF00) >> 16 );

	printk(":%x:%x", (((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[1] & 0xFF), ((((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[1]) & 0xFF00) >> 16 );

	printk(":%x:%x", (((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[0] & 0xFF), ((((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6.ip[0]) & 0xFF00) >> 16 );
#endif
	printk("new_src_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_port);
	printk("new_dst_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_dst_port);
	printk("Template buffer:\n");
	printk("\t");
	for (i=0;i < ((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->templ_len;i++)  {
		printk("%02x ",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->templ_buf[i]);
	}
	printk("\n\n");
	return 0;

}


int32_t mpe_hal_display_hw_session_action(uint32_t current_ptr)
{
	uint32_t size, i;
	struct hw_act_ptr *phtable;
	int v =0,vap_entry_index=0,dst_pmac_port_no = 0, pmac_port_num =0;
	struct vap_entry *ve = NULL;

	if(current_ptr > MAX_HW_SESSION_NUM) {
                printk("Index is exceeding the table size \n");
                return PPA_FAILURE;
        }

	size =1; //sizeof( struct hw_act_ptr);
	phtable =(struct hw_act_ptr *) g_GenConf->hw_act_tbl_base; 

	if((phtable + (current_ptr * size))->act == 0) {
		printk("NULL Pointer\n");
		return PPA_FAILURE;
	}
	/*for (i=0 ; i< g_GenConf->hw_act_num; i++){
		if(((struct session_action*)((phtable + i)->act)) != NULL)
			printk("Template buffer len  = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->templ_len);
	}
		return PPA_SUCCESS;*/
	printk("\nAction Table:\n");
	printk("Valid    		   = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->entry_vld);
	printk("Template buffer len        = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->templ_len);
	printk("pkt_len_delta              = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->pkt_len_delta);
	printk("traffic_class              = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->traffic_class);
	printk("mtu                        = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->mtu);
	printk("protocol                   = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->protocol);
	printk("routing_flag               = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->routing_flag );
	printk("new_src_ip_en              = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_src_ip_en);
	printk("new_dst_ip_en              = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_dst_ip_en);
	printk("new_inner_dscp_en          = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_inner_dscp_en);
	printk("pppoe_offset_en            = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->pppoe_offset_en);
	printk("tunnel_ip_offset_en        = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset_en);
	printk("tunnel_udp_offset_en       = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset_en);
	printk("in_eth_iphdr_offset_en     = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset_en);
	printk("sess_mib_ix_en             = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->sess_mib_ix_en);
	printk("new_traffic_class_en       = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_traffic_class_en);
	printk("tunnel_rm_en               = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_rm_en);
	printk("meter_id1_en               = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->meter_id1_en);
	printk("outer_dscp_mode            = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->outer_dscp_mode);
	printk("meter_id0                  = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->meter_id0);
	printk("meter_id1                  = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->meter_id1);
	printk("new_inner_dscp             = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_inner_dscp);
	printk("tunnel_type                = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_type);
	printk("tunnel_id                  = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_id);
	printk("pppoe_offset               = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->pppoe_offset);
	printk("tunnel_ip_offset           = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset);
	printk("in_eth_iphdr_offset        = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset);
	printk("tunnel_udp_offset          = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset);
	printk("key_en                     = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->key_en);
	printk("rx_itf_mib_num             = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_num);
	printk("tx_itf_mib_num             = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_num);
	printk("sess_mib_ix                = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->sess_mib_ix);
	printk("dst_pmac_port_num          = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num);

	pmac_port_num = ((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num;
	/*multicast*/
	vap_entry_index=((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->mc_index;
	if(vap_entry_index) 
	{
		printk("mc_index                   = %d\n",vap_entry_index);
		for(i=0;i < pmac_port_num;i++) 
		{
			printk("dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
			dst_pmac_port_no = ((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i];
			if(dst_pmac_port_no)
			{	
				ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[dst_pmac_port_no] + (sizeof(struct vap_entry) * vap_entry_index));
				printk("mc_vap_num                 = 0x%x \n",ve->num);
				for(v=0; v < ve->num ;v++)
				{
					printk("mc_vap_list[%d]            = 0x%x \n",v,ve->vap_list[v]);
				}
				printk("\n");
			}
		}
	}
	else
	{
		for(i=0;i < pmac_port_num;i++)
			printk("dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
		for(i=0;i < pmac_port_num;i++)
			printk("uc_vap_list[%d]             = %d\n",i,((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->uc_vap_list[i]);
	}
	for(i=0;i < MAX_ITF_PER_SESSION;i++)
		printk("rx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_ix[i]);
	for(i=0;i < MAX_ITF_PER_SESSION;i++)
		printk("tx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_ix[i]);
	printk("new_src_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip4.ip));
	printk("new_dst_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_dst_ip.ip4.ip));
	printk("new_src_port               = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_src_port);
	printk("new_dst_port               = %d\n",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->new_dst_port);
	printk("Template buffer:\n");
	printk("\t");
	for (i=0;i < ((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->templ_len;i++)  {
		printk("%02x ",((struct session_action*)(((struct hw_act_ptr *)(phtable + (current_ptr * size)))->act))->templ_buf[i]);
	}
	printk("\n\n");
	return PPA_SUCCESS;

}
int32_t mpe_hal_display_session_action(uint32_t tbl, uint32_t current_ptr)
{
	int32_t ret = PPA_SUCCESS;
	if(tbl == 1){
		ret = mpe_hal_display_ipv4_session_action(current_ptr);
	} else if(tbl == 2) {
		ret = mpe_hal_display_ipv6_session_action(current_ptr);
	} else if (tbl == 3) {
		ret = mpe_hal_display_hw_session_action(current_ptr);
	}
	return ret;
}



#if 1


int32_t mpe_hal_dump_table_hashidx_entry(void *phtable,uint32_t hashidx,uint32_t type_flag, struct seq_file *seq)
{
    uint32_t size=0,counter=0,i=0;
    uint32_t current_ptr=0, previous_ptr=(MAX_HW_SESSION_NUM);
    struct ipv6_hash_auto_key ip6key;
    int v =0,vap_entry_index=0,dst_pmac_port_no = 0, pmac_port_num =0;
    struct vap_entry *ve = NULL;


    if(type_flag == 0)
        size=sizeof(struct fw_compare_hash_auto_ipv4);
    else
        size=sizeof(struct fw_compare_hash_auto_ipv6);

    //dp_mpe_fw_get_htable_lock();
    current_ptr = ((struct fw_compare_hash_auto_ipv4 *)(phtable + (hashidx * size)))->first_ptr;
    while(previous_ptr != current_ptr)
    {
        if(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->valid) {
            if(type_flag==0){
                seq_printf(seq,"\nHash Index = 0x%x : \n",hashidx);
                seq_printf(seq,"Session Id/Index     = 0x%x \n",current_ptr);
                seq_printf(seq,"MPE FW TABLE [IPV4]\n");
                seq_printf(seq,"key :srcip     = %s\n",inet_ntoa(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.srcip));
                seq_printf(seq,"key :dstip     = %s\n",inet_ntoa(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.dstip));
                seq_printf(seq,"key :dstport   = %d\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.dstport);
                seq_printf(seq,"key :srcport   = %d\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.srcport);

                if (((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn == 0x80)
		{
			seq_printf(seq,"key :extn      = %d GRE[TCP]\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn);
		}	
                else if (((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn == 0x81)
		{
			seq_printf(seq,"key :extn      = %d GRE[UDP]\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn);
		}	
                else if (((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn == 0x21)
		{
			seq_printf(seq,"key :extn      = %d L2TP[UDP]\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn);
		}	
                else if (((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn == 0x20)
		{
			seq_printf(seq,"key :extn      = %d L2TP[UDP]\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn);
		}	
		else if (((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn == 0x01)
		{
			seq_printf(seq,"key :extn      = %d UDP\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn);
		}
                else
                    seq_printf(seq,"key :extn      = %d TCP\n",((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->key.extn);

                if(display_action)
                {
                    seq_printf(seq,"\nAction Table:\n");
                    seq_printf(seq,"Template buffer len        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->templ_len);
                    seq_printf(seq,"pkt_len_delta              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->pkt_len_delta);
                    seq_printf(seq,"traffic_class              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->traffic_class);
                    seq_printf(seq,"mtu                        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->mtu);
                    seq_printf(seq,"protocol                   = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->protocol);
                    seq_printf(seq,"routing_flag               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->routing_flag );
                    seq_printf(seq,"new_src_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_src_ip_en);
                    seq_printf(seq,"new_dst_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_dst_ip_en);
                    seq_printf(seq,"new_inner_dscp_en          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp_en);
                    seq_printf(seq,"pppoe_offset_en            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->pppoe_offset_en);
                    seq_printf(seq,"tunnel_ip_offset_en        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset_en);
                    seq_printf(seq,"tunnel_udp_offset_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset_en);
                    seq_printf(seq,"in_eth_iphdr_offset_en     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset_en);
                    seq_printf(seq,"sess_mib_ix_en             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix_en);
                    seq_printf(seq,"new_traffic_class_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_traffic_class_en);
                    seq_printf(seq,"tunnel_rm_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_rm_en);
                    seq_printf(seq,"meter_id1_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->meter_id1_en);
                    seq_printf(seq,"outer_dscp_mode            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->outer_dscp_mode);
                    seq_printf(seq,"meter_id0                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->meter_id0);
                    seq_printf(seq,"meter_id1                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->meter_id1);
                    seq_printf(seq,"new_inner_dscp             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp);
                    seq_printf(seq,"tunnel_type                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_type);
                    seq_printf(seq,"pppoe_offset               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->pppoe_offset);
                    seq_printf(seq,"tunnel_ip_offset           = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset);
                    seq_printf(seq,"in_eth_iphdr_offset        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset);
                    seq_printf(seq,"tunnel_udp_offset          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset);
                    seq_printf(seq,"key_en                     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->key_en);
                    seq_printf(seq,"rx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_num);
                    seq_printf(seq,"tx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_num);
                    seq_printf(seq,"sess_mib_ix                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix);
                    seq_printf(seq,"dst_pmac_port_num          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num);

                    pmac_port_num = ((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num;
                    /*multicast*/
                    vap_entry_index=((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->mc_index;
                    if(vap_entry_index) 
                    {
                        seq_printf(seq,"mc_index                   = %d\n",vap_entry_index);
                        for(i=0;i < pmac_port_num;i++) 
                        {
                            seq_printf(seq,"dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
                            dst_pmac_port_no = ((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i];
                            if(dst_pmac_port_no)
                            {	
                                ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[dst_pmac_port_no] + (sizeof(struct vap_entry) * vap_entry_index));
                                seq_printf(seq,"mc_vap_num                 = 0x%x \n",ve->num);
                                for(v=0; v < ve->num ;v++)
                                {
                                    seq_printf(seq,"mc_vap_list[%d]            = 0x%x \n",v,ve->vap_list[v]);
                                }
                                seq_printf(seq,"\n");
                            }
                        }
                    }
                    else
                    {
                        for(i=0;i < pmac_port_num;i++)
                            seq_printf(seq,"dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
                        for(i=0;i < pmac_port_num;i++)
                            seq_printf(seq,"uc_vap_list[%d]             = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->uc_vap_list[i]);
                    }
                    for(i=0;i < MAX_ITF_PER_SESSION;i++)
                        seq_printf(seq,"rx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_ix[i]);
                    for(i=0;i < MAX_ITF_PER_SESSION;i++)
                        seq_printf(seq,"tx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_ix[i]);
                    seq_printf(seq,"new_src_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip4.ip));
                    seq_printf(seq,"new_dst_ip                 = %s\n",inet_ntoa(((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_dst_ip.ip4.ip));
                    seq_printf(seq,"new_src_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_src_port);
                    seq_printf(seq,"new_dst_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->new_dst_port);
                    seq_printf(seq,"Template buffer:\n");
                    seq_printf(seq,"\t");
                    for (i=0;i < ((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->templ_len;i++)  {
                        seq_printf(seq,"%02x ",((struct session_action*)(((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->act))->templ_buf[i]);
                    }
                    seq_printf(seq,"\n\n");
                }
            }
            else {
                seq_printf(seq,"\nHash Index = 0x%x : \n",hashidx);
                seq_printf(seq,"Session Id/Index     = 0x%x \n",current_ptr);
                seq_printf(seq,"MPE FW TABLE [IPV6]\n");
                memset(str,0,INET6_ADDRSTRLEN);
                ip6key.srcip[0] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.srcip[3];
                ip6key.srcip[1] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.srcip[2];
                ip6key.srcip[2] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.srcip[1];
                ip6key.srcip[3] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.srcip[0];
                seq_printf(seq,"key :src_ip   =%s \n",inet_ntop6((char *)&ip6key.srcip,str,INET6_ADDRSTRLEN));
                memset(str,0,INET6_ADDRSTRLEN);
                ip6key.dstip[0] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.dstip[3];
                ip6key.dstip[1] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.dstip[2];
                ip6key.dstip[2] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.dstip[1];
                ip6key.dstip[3] = ((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.dstip[0];
                seq_printf(seq,"key :dst_ip   =%s \n",inet_ntop6((char *)&ip6key.dstip,str,INET6_ADDRSTRLEN));
                seq_printf(seq,"key :dstport  = %d\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.dstport);
                seq_printf(seq,"key :srcport  = %d \n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.srcport);
                if (((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn == 0x81)
		{
			seq_printf(seq,"key :extn      = %d GRE[UDP]\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn);
		}	
                else if (((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn == 0x80)
		{
			seq_printf(seq,"key :extn      = %d GRE[TCP]\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn);
		}	
                else if (((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn == 0x21)
		{
			seq_printf(seq,"key :extn      = %d L2TP[UDP]\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn);
		}	
                else if (((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn == 0x20)
		{
			seq_printf(seq,"key :extn      = %d L2TP[TCP]\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn);
		}	
		else if (((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn == 0x01)
		{
			seq_printf(seq,"key :extn      = %d UDP\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn);
		}
                else
                    seq_printf(seq,"key :extn      = %d TCP\n",((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->key.extn);

                if(display_action)
                {
                    seq_printf(seq,"\nAction Table:\n");
                    seq_printf(seq,"Template buffer len        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->templ_len);
                    seq_printf(seq,"dst_pmac_port_num          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num);
                    seq_printf(seq,"dst_pmac_port_list[0]      = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[0]);
                    seq_printf(seq,"pkt_len_delta              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->pkt_len_delta);
                    seq_printf(seq,"traffic_class              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->traffic_class);
                    seq_printf(seq,"mtu                        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->mtu);
                    seq_printf(seq,"protocol                   = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->protocol);
                    seq_printf(seq,"routing_flag               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->routing_flag );
                    seq_printf(seq,"new_src_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip_en);
                    seq_printf(seq,"new_dst_ip_en              = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_dst_ip_en);
                    seq_printf(seq,"new_inner_dscp_en          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp_en);
                    seq_printf(seq,"pppoe_offset_en            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->pppoe_offset_en);
                    seq_printf(seq,"tunnel_ip_offset_en        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset_en);
                    seq_printf(seq,"tunnel_udp_offset_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset_en);
                    seq_printf(seq,"in_eth_iphdr_offset_en     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset_en);
                    seq_printf(seq,"sess_mib_ix_en             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix_en);
                    seq_printf(seq,"new_traffic_class_en       = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_traffic_class_en);
                    seq_printf(seq,"tunnel_rm_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_rm_en);
                    seq_printf(seq,"meter_id1_en               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->meter_id1_en);
                    seq_printf(seq,"outer_dscp_mode            = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->outer_dscp_mode);
                    seq_printf(seq,"meter_id0                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->meter_id0);
                    seq_printf(seq,"meter_id1                  = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->meter_id1);
                    seq_printf(seq,"new_inner_dscp             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_inner_dscp);
                    seq_printf(seq,"tunnel_type                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_type);
                    seq_printf(seq,"pppoe_offset               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->pppoe_offset);
                    seq_printf(seq,"tunnel_ip_offset           = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_ip_offset);
                    seq_printf(seq,"in_eth_iphdr_offset        = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->in_eth_iphdr_offset);
                    seq_printf(seq,"tunnel_udp_offset          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tunnel_udp_offset);
                    seq_printf(seq,"key_en                     = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->key_en);
                    seq_printf(seq,"rx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_num);
                    seq_printf(seq,"tx_itf_mib_num             = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_num);
                    seq_printf(seq,"sess_mib_ix                = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->sess_mib_ix);
                    seq_printf(seq,"dst_pmac_port_num          = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num);
                    pmac_port_num = ((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_num;

                    /*multicast*/
                    vap_entry_index=((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->mc_index;
                    if(vap_entry_index) 
                    {
                        seq_printf(seq,"mc_index                   = %d\n",vap_entry_index);
                        for(i=0;i < pmac_port_num;i++) 
                        {
                            seq_printf(seq,"dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
                            dst_pmac_port_no = ((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i];
                            if(dst_pmac_port_no)
                            {	
                                ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[dst_pmac_port_no] + (sizeof(struct vap_entry) * vap_entry_index));
                                seq_printf(seq,"mc_vap_num                 = 0x%x \n",ve->num);
                                for(v=0; v < ve->num ;v++)
                                {
                                    seq_printf(seq,"mc_vap_list[%d]            = 0x%x \n",v,ve->vap_list[v]);
                                }
                                seq_printf(seq,"\n");
                            }
                        }
                    }
                    else
                    {
                        for(i=0;i < pmac_port_num;i++)
                            seq_printf(seq,"dst_pmac_port_list[%d]      = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->dst_pmac_port_list[i]);
                        for(i=0;i < pmac_port_num;i++)
                            seq_printf(seq,"uc_vap_list[%d]             = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->uc_vap_list[i]);
                    }

                    for(i=0;i < MAX_ITF_PER_SESSION;i++)
                        seq_printf(seq,"rx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->rx_itf_mib_ix[i]);
                    for(i=0;i < MAX_ITF_PER_SESSION;i++)
                        seq_printf(seq,"tx_itf_mib_ix[%d]           = %d\n",i,((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->tx_itf_mib_ix[i]);
                    seq_printf(seq,"new_src_ip                 = " NIP6_FMT "\n",
                            NIP6(&((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_ip.ip6));
                    seq_printf(seq,"new_dst_ip                 = " NIP6_FMT "\n",
                            NIP6(&((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_dst_ip.ip6));
                    seq_printf(seq,"new_src_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_src_port);
                    seq_printf(seq,"new_dst_port               = %d\n",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->new_dst_port);

                    seq_printf(seq,"Template buffer:\n");
                    seq_printf(seq,"\t");
                    for (i=0;i < ((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->templ_len;i++) {
                        seq_printf(seq,"%02x ",((struct session_action*)(((struct fw_compare_hash_auto_ipv6 *)(phtable + (current_ptr * size)))->act))->templ_buf[i]);
                    }
                    seq_printf(seq,"\n\n");
                }		
            }
            session_counter++;
        }
        counter++;
        previous_ptr=current_ptr;
        current_ptr=((struct fw_compare_hash_auto_ipv4 *)(phtable + (current_ptr * size)))->nxt_ptr;
    }
    //dp_mpe_fw_release_htable_lock();
	return PPA_SUCCESS;
}

//int mpe_hal_dump_ipv4_cmp_table_entry(struct seq_file *seq, int pos)
int mpe_hal_dump_ipv4_cmp_table_entry(struct seq_file *seq)
{
	//mpe_hal_dump_table_hashidx_entry((void *)&g_GenConf->fw_cmp_tbl_base[0],g_GenConf->fw_sess_tbl_num[0], 0);

#if 1
	int32_t i=0;
	struct fw_compare_hash_auto_ipv4 * pIp4CmpTbl;
	pIp4CmpTbl = (struct fw_compare_hash_auto_ipv4 *)g_GenConf->fw_cmp_tbl_base[0];
	for(i = 0; i < g_GenConf->fw_sess_tbl_num[0]; i++ ) {
	//	seq_printf(seq,"Next Entry %d address 0x%x\n",i,((pIp4CmpTbl + (i ))));
		if(((pIp4CmpTbl + (i ))->valid) == 1) {
			seq_printf(seq,"\n");
			seq_printf(seq,"Index : %d\n",i);
			seq_printf(seq,"Valid: %d\n",(pIp4CmpTbl + (i ))->valid);
			seq_printf(seq,"Next Pointer: %d\n",(pIp4CmpTbl + (i ))->nxt_ptr);
			seq_printf(seq,"First Pointer: %d\n",(pIp4CmpTbl + (i ))->first_ptr);
			seq_printf(seq,"Action Pointer: %x\n",(pIp4CmpTbl + (i ))->act);

			seq_printf(seq,"SrcIp: %s\n",inet_ntoa((pIp4CmpTbl + (i ))->key.srcip));
			seq_printf(seq,"DstIp: %s\n",inet_ntoa((pIp4CmpTbl + (i ))->key.dstip));
			seq_printf(seq,"SrcPort: %d\n",(pIp4CmpTbl + (i ))->key.srcport);
			seq_printf(seq,"DstPort: %d\n",(pIp4CmpTbl + (i ))->key.dstport);
			seq_printf(seq,"DstPort: %d\n",(pIp4CmpTbl + (i ))->key.extn);
			seq_printf(seq,"====================\n");
		}
	}

#endif

#if 0
	u32 proc_mpe_ipv4_session_dump_start_id=0;
	u32 proc_mpe_ipv4_session_dump_stop_id= g_GenConf->fw_sess_tbl_num[0];

	//print_mpe_fw_table_hashidx_entry(htable_4, pos,IPVER4);
	mpe_hal_dump_table_hashidx_entry((void *)&g_GenConf->fw_cmp_tbl_base[0], pos, 0);
	pos++;
	if ((pos + proc_mpe_ipv4_session_dump_start_id >= g_GenConf->fw_sess_tbl_num[0]) ||
			(pos + proc_mpe_ipv4_session_dump_start_id > proc_mpe_ipv4_session_dump_stop_id)) {
		pos = -1;	/*end of the loop*/
		seq_printf(seq,"\nNumber of IPV4 Session added in MPE FW table = %d\n",session_counter);
		session_counter = 0;
	}
	return pos;
#endif
	return 0;
}

void mpe_hal_dump_ipv6_cmp_table_entry( struct seq_file *seq )
{
	//mpe_hal_dump_table_hashidx_entry((void *)&g_GenConf->fw_cmp_tbl_base[1],g_GenConf->fw_sess_tbl_num[1], 1, seq);
#if 1
	int32_t i=0;
	struct fw_compare_hash_auto_ipv6 * pIp6CmpTbl;
	pIp6CmpTbl = (struct fw_compare_hash_auto_ipv6 *)g_GenConf->fw_cmp_tbl_base[1];
	for(i = 0; i < g_GenConf->fw_sess_tbl_num[1]; i++ ) {
	//	seq_printf(seq,"Next Entry %d address 0x%x\n",i,((pIp4CmpTbl + (i ))));
		if(((pIp6CmpTbl + (i ))->valid) == 1) {
			seq_printf(seq,"\n");
			seq_printf(seq,"Index : %d\n",i);
			seq_printf(seq,"Valid: %d\n",(pIp6CmpTbl + (i ))->valid);
			seq_printf(seq,"Next Pointer: %d\n",(pIp6CmpTbl + (i ))->nxt_ptr);
			seq_printf(seq,"First Pointer: %d\n",(pIp6CmpTbl + (i ))->first_ptr);
			seq_printf(seq,"Action Pointer: %x\n",(pIp6CmpTbl + (i ))->act);
#if 1
			//seq_printf(seq,"SrcIp: %s\n",inet_ntoa((pIp6CmpTbl + (i ))->key.srcip));
			//seq_printf(seq,"DstIp: %s\n",inet_ntoa((pIp6CmpTbl + (i ))->key.dstip));
			seq_printf(seq,"SrcIp: %pI6\n",(pIp6CmpTbl + (i ))->key.srcip);
			seq_printf(seq,"DstIp: %pI6\n",(pIp6CmpTbl + (i ))->key.dstip);
#else
			seq_printf(seq,"1st16bits=%x 2nd16bits=%x\n", (((pIp6CmpTbl + (i ))->key.srcip[3] & 0xFF00) >> 16 ), ((pIp6CmpTbl + (i ))->key.srcip[3] & 0xFF));
			seq_printf(seq,"SrcIp: %x:%x:%x:%x", ((pIp6CmpTbl + (i ))->key.srcip[3] & 0xFF), ((pIp6CmpTbl + (i ))->key.srcip[3] & 0xFF00) >> 8, ((pIp6CmpTbl + (i ))->key.srcip[2] & 0xFF), ((pIp6CmpTbl + (i ))->key.srcip[2] & 0xFF00) >> 8);
			seq_printf(seq,"%x:%x:%x:%x\n", ((pIp6CmpTbl + (i ))->key.srcip[2] & 0xFF), ((pIp6CmpTbl + (i ))->key.srcip[2] & 0xFF00) >> 8, ((pIp6CmpTbl + (i ))->key.srcip[0] & 0xFF), ((pIp6CmpTbl + (i ))->key.srcip[0] & 0xFF00) >> 8);

			seq_printf(seq,"DstIp: %x:%x:%x:%x", ((pIp6CmpTbl + (i ))->key.dstip[3] & 0xFF), ((pIp6CmpTbl + (i ))->key.dstip[3] & 0xFF00) >> 8, ((pIp6CmpTbl + (i ))->key.dstip[2] & 0xFF), ((pIp6CmpTbl + (i ))->key.dstip[2] & 0xFF00) >> 8);
			seq_printf(seq,"%x:%x:%x:%x\n", ((pIp6CmpTbl + (i ))->key.dstip[1] & 0xFF), ((pIp6CmpTbl + (i ))->key.dstip[1] & 0xFF00) >> 8, ((pIp6CmpTbl + (i ))->key.dstip[0] & 0xFF), ((pIp6CmpTbl + (i ))->key.dstip[0] & 0xFF00) >> 8);

#endif
			seq_printf(seq,"SrcPort: %d\n",(pIp6CmpTbl + (i ))->key.srcport);
			seq_printf(seq,"DstPort: %d\n",(pIp6CmpTbl + (i ))->key.dstport);
			seq_printf(seq,"DstPort: %d\n",(pIp6CmpTbl + (i ))->key.extn);
			seq_printf(seq,"====================\n");
		}
	}

#endif

	return 0;
}
#endif


void mpe_hal_dump_vap_list(void)
{
    int i, j, k;
    struct vap_entry *ve;
    for(i=0; i<MAX_PMAC_PORT; i++) {
        for(j=0; j<g_GenConf->mc_vap_num[i]; j++) {
            ve = (struct vap_entry *)(g_GenConf->mc_vap_tbl_base[i] + (sizeof(struct vap_entry) * j));
            if(!ve->num) continue;
            	printk("Port%02u:%03u base(%x) num(%02u):\n", i, j, ve, ve->num);
            for(k=0; k<ve->num ;k++) {
                printk("%04x ", ve->vap_list[k]);
            }
            printk("\n");
        }
        //printk("==============================================\n");
    }
}


char *get_status_str(uint32_t tc_cur_state)
{
    if(tc_cur_state == UNKNOWN)
        return ("Unknown");
    else if(tc_cur_state == TM_YD_WAIT)
        return ("TM YIELD");
    else if(tc_cur_state == WK_YD_WAIT)
        return ("WK YIELD");
    else if(tc_cur_state == WK_YD_WKUP)
        return ("WK YIELD WAKE UP");
    else if (tc_cur_state == TM_YD_WKUP)
        return ("TM YIELD WAKE UP");
    else if (tc_cur_state == DL_YD_WAIT)
        return ("DL YIELD WAIT");
    else if (tc_cur_state == DL_YD_WKUP)
        return ("DL YIELD WAKE UP");
    else if (tc_cur_state == MCPY_YD_WAIT)
        return ("MCPY YIELD WAKE UP");
    else if (tc_cur_state == MCPY_YD_WKUP)
        return ("MCPY YIELD WAKE UP");	
    else if (tc_cur_state == MPECTRL_ENQ_YD_WAIT)
        return ("MPE CTRL ENQ YIELD");
    else if (tc_cur_state == MPECTRL_ENQ_YD_WKUP)
        return ("MPE CTRL ENQ YIELD WAKE UP");
    else if (tc_cur_state == MPECTRL_DEQ_YD_WAIT)
        return ("MPE CTRL DEQ YIELD");
    else if (tc_cur_state == MPECTRL_DEQ_YD_WKUP)
        return ("MPE CTRL DEQ YIELD WAKE UP");
    else if (tc_cur_state == CBM_ENQ_YD_WAIT)
        return ("CBM ENQ YIELD"); 	
    else if (tc_cur_state == CBM_ENQ_YD_WKUP)
        return ("CBM ENQ YIELD WAKE UP");		
    else if (tc_cur_state == CBM_DEQ_YD_WAIT)
        return ("CBM DEQ YIELD"); 	
    else if (tc_cur_state == CBM_DEQ_YD_WKUP)
        return ("CBM DEQ YIELD WAKE UP");		
    else if (tc_cur_state == SEM_DISP_Q_WAIT)
        return ("SEM TAKE DISP Q"); 	
    else if (tc_cur_state == SEM_FREELIST_WAIT)
        return ("SEM TAKE FREELIST");		
    else if (tc_cur_state == SEM_CBMALLOC_WAIT)
        return ("SEM TAKE CBM ALLOC"); 	
    else if (tc_cur_state == SEM_DISP_Q_CNT_WAIT)
        return ("SEM TAKE DISP Q COUNT"); 	
    else if (tc_cur_state == SEM_DISP_Q_WKUP)
        return ("SEM PUT DISP Q WAKE UP"); 	
    else if (tc_cur_state == SEM_FREELIST_WKUP)
        return ("SEM PUT FREELIST WAKE UP ");		
    else if (tc_cur_state == SEM_CBMALLOC_WKUP)
        return ("SEM PUT CBM ALLOC WAKE UP"); 	
    else if (tc_cur_state == SEM_DISP_Q_CNT_WKUP)
        return ("SEM PUT DISP Q COUNT WAKE UP"); 	

    printk("Error returning tc_cur_state %d\n",tc_cur_state);
    return ("Error");
}

char *get_tc_type(int idx)
{
    if(g_GenConf->hw_res[idx].tcType == TYPE_TM)
        return ("TM");
    else if(g_GenConf->hw_res[idx].tcType == TYPE_WORKER)
        return ("WK");
    else if(g_GenConf->hw_res[idx].tcType == TYPE_DIRECTLINK)
        return ("DL");
    else
        return ("Unknown");
}

char *get_tc_health_cond(int idx)
{
    if(g_GenConf->hw_res[idx].state == STATE_INACTIVE)
        return("INACTIVE");
    else if(g_GenConf->hw_res[idx].state == STATE_IN_PROGRESS)
        return("RUNNING");
    else if(g_GenConf->hw_res[idx].state == STATE_TERMINATED)
        return("TERMIND");
    else if(g_GenConf->hw_res[idx].state == STATE_PAUSE)
        return("PAUSE");
    else if(g_GenConf->hw_res[idx].state == STATE_RESUME)
        return("RESUME");
    else
        return("Unknown");	
}

void dump_tc_current_status()
{
    int i=0;

    if(!g_GenConf)
    {
        printk("Genconf is not valid\n");
        return;
    }
    printk("=========================TC STATUS========================\n");
    printk("Hw_res:   Health:   TcType:   TcNum:    Vpe:      TcState:\n");
    printk("==========================================================\n");
    for(i=0;i < MAX_MPE_TC_NUM;i++) { 
        if(!g_GenConf->hw_res[i].flag)
            continue;
        printk("%02d        %s     %s        %02d        %02d     %s  \n",
                i, 
                get_tc_health_cond(i),
                get_tc_type(i),
                g_GenConf->hw_res[i].TcNum,
                g_GenConf->hw_res[i].CpuNum,
		get_status_str(g_GenConf->tc_cur_state[i]));
    }
    printk("==========================================================\n");	
    printk("\n");

    return;

}

void mpe_hal_dump_fw_header(struct seq_file *seq)
{
    int i=0;
//    if(g_MpeFwHdr) {
        seq_printf(seq, "MPE FW HEADER :\n\n");
        seq_printf(seq, "fw_endian                %c 0x%x\n",'=',g_MpeFwHdr.fw_endian);
        seq_printf(seq, "compatible_id            %c 0x%x\n",'=',g_MpeFwHdr.compatible_id);
        seq_printf(seq, "family                   %c 0x%x\n",'=',g_MpeFwHdr.family);
        seq_printf(seq, "package                  %c 0x%x\n",'=',g_MpeFwHdr.package);
        seq_printf(seq, "v_maj                    %c 0x%x\n",'=',g_MpeFwHdr.v_maj);
        seq_printf(seq, "v_mid                    %c 0x%x\n",'=',g_MpeFwHdr.v_mid);
        seq_printf(seq, "v_min                    %c 0x%x\n",'=',g_MpeFwHdr.v_min);
        seq_printf(seq, "v_tag                    %c 0x%x\n",'=',g_MpeFwHdr.v_tag);
        seq_printf(seq, "genconf_offset           %c 0x%x\n",'=',g_MpeFwHdr.genconf_offset);
        seq_printf(seq, "hdr_size                 %c 0x%x\n",'=',g_MpeFwHdr.hdr_size);
        seq_printf(seq, "fw_code_size             %c 0x%x\n",'=',g_MpeFwHdr.fw_code_size);
        seq_printf(seq, "fw_data_size             %c 0x%x\n",'=',g_MpeFwHdr.fw_data_size);
        seq_printf(seq, "fw_bss_size              %c 0x%x\n",'=',g_MpeFwHdr.fw_bss_size);
        seq_printf(seq, "fw_stack_priv_data_size  %c 0x%x\n",'=',g_MpeFwHdr.fw_stack_priv_data_size);
        seq_printf(seq, "fw_priv_data_size        %c 0x%x\n",'=',g_MpeFwHdr.fw_priv_data_size);
        seq_printf(seq, "fw_code_align            %c 0x%x\n",'=',g_MpeFwHdr.fw_code_align);
        seq_printf(seq, "fw_priv_data_align       %c 0x%x\n",'=',g_MpeFwHdr.fw_priv_data_align);
        seq_printf(seq, "fw_priv_data_mapped_addr %c 0x%x\n",'=',g_MpeFwHdr.fw_priv_data_mapped_addr);
        seq_printf(seq, "\n");
	seq_printf(seq, " MPE firmware code start address        %c 0x%p \n",'=', g_MpeFw_load_addr);
        seq_printf(seq, "\n");
	for (i=0;i < MAX_MPE_TC_NUM;i++) {
		//seq_printf(seq, "TC %d private data start address        %c 0x%x\n",i,'=',fw_tc_priv_data_start_addr[i]);
		seq_printf(seq, "TC %d stack start address               %c 0x%p\n", i,'=',g_MpeFw_stack_addr + (g_GenConf->fw_hdr.fw_stack_priv_data_size * i));
		seq_printf(seq, "\n");
	}
   /* }
    else 
        seq_printf(seq, "g_MpeFwHdr = %x\n",(unsigned int)g_MpeFwHdr);
  */
}



void mpe_hal_dump_genconf_offset(struct seq_file *seq)
{
        seq_printf(seq, "EVA mode: %d\n",g_GenConf->eva_config_flag);
        seq_printf(seq, "Segment control registers SEG0 = 0:%.8x SEG1 = 1:%.8x SEG2 = 2:%.8x\n",g_GenConf->eva_SegCtl0,g_GenConf->eva_SegCtl1,g_GenConf->eva_SegCtl2);


	seq_printf(seq, "vmb_fw_msg_base %x fw_vmb_msg_base %x Cpunum: %d\n",
			g_GenConf->vmb_fw_msg_base[g_MPELaunch_CPU],g_GenConf->fw_vmb_msg_base[g_MPELaunch_CPU], g_MPELaunch_CPU);

        seq_printf(seq, "IPV4 Compare table base 0x%8x\n",g_GenConf->fw_cmp_tbl_base[0]);
        seq_printf(seq, "IPV6 Compare table base 0x%8x\n",g_GenConf->fw_cmp_tbl_base[1]);

	seq_printf(seq, "hw_act_num                %c 0x%x\n",'=',g_GenConf->hw_act_num);
        seq_printf(seq, "hw_act_tbl_base           %c 0x%x\n",'=',g_GenConf->hw_act_tbl_base);

        seq_printf(seq, "Session hit table base 0x%8p\n",g_GenConf->fw_sess_hit_tbl_base);
        seq_printf(seq, "Session MIB table base 0x%8p\n",g_GenConf->fw_sess_mib_tbl_base);
        seq_printf(seq, "Interface MIB table base 0x%8p\n",g_GenConf->mib_itf_tbl_base);
        seq_printf(seq, "Debug : %d\n",g_GenConf->g_mpe_dbg_enable);

	seq_printf(seq, "dbg_mpe_q_id              %c 0x%x\n",'=',g_GenConf->dbg_mpe_q_id);
        seq_printf(seq, "dbg_mpe_q_id_en           %c 0x%x\n",'=',g_GenConf->dbg_mpe_q_id_en);
        seq_printf(seq, "mc_cmp_mode               %c mpe_fw_len0x%x\n",'=',g_GenConf->mc_cmp_mode);
        seq_printf(seq, "dispatch_q_buf_num        %c 0x%x\n",'=',g_GenConf->dispatch_q_buf_num);

#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
	seq_printf(seq, "==========================================================\n");	
	seq_printf(seq, "GenConfig DirectLink information\n");
        seq_printf(seq, "    DL buffer_free_tbl_base %c 0x%x\n",'=',g_GenConf->dl_buffer_free_tbl_base);
        seq_printf(seq, "    dl_dispatch_q_cnt 		%c 0x%x\n",'=',g_GenConf->dl_dispatch_q_cnt);
        seq_printf(seq, "    dl_ls_q_cnt 			%c 0x%x\n",'=',g_GenConf->dl_ls_q_cnt);
        seq_printf(seq, "    max_tm_dl_deq_pkt_num	%c 0x%x\n",'=',g_GenConf->max_tm_dl_deq_pkt_num);
        seq_printf(seq, "    dl_dispatch_q_buf_num	%c 0x%x\n",'=',g_GenConf->dl_dispatch_q_buf_num);
	seq_printf(seq, "	TX Cfg Context Buffer:	0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].tx_cfg_ctxt_buf_base);
	seq_printf(seq, "				size:		0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].tx_cfg_ctxt_buf_size);

	seq_printf(seq, "	RX Cfg Context Buffer:	0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].rx_cfg_ctxt_buf_base);
	seq_printf(seq, "				size:		0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].rx_cfg_ctxt_buf_size);

	seq_printf(seq, "	RX Message Buffer:		0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].rx_msgbuf_base);
	seq_printf(seq, "				size:		0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].rx_msgbuf_size);

	seq_printf(seq, "	Communication Buffer:	0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].comm_buf_base);
	seq_printf(seq, "				size:	0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].comm_buf_size);
		
	seq_printf(seq, "	Uncached Buffer:		0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].uncached_addr_base);
	seq_printf(seq, "				size:		0x%x\n",g_GenConf->fw_hdr.dl_buf_info_base[0].uncached_addr_size);
#endif
#ifdef CONFIG_LTQ_PPA_MPE_IP97
	seq_printf(seq, "	CDR in base address:		0x%x\n",g_GenConf->e97_cdr_in[0][0]);
	seq_printf(seq, "	CDR out base address:		0x%x\n",g_GenConf->e97_cdr_out[0][0]);
	seq_printf(seq, "	ACD in base address:		0x%x\n",g_GenConf->e97_acd_in[0][0]);
	seq_printf(seq, "	ACD out base address:		0x%x\n",g_GenConf->e97_acd_out[0][0]);
	seq_printf(seq, "	RDR base address:		0x%x\n",g_GenConf->e97_rdr[0]);
	seq_printf(seq, "	Tunnel redir port:		%d\n",g_GenConf->tunnel_redir_port);
#endif
}




void mpe_hal_dump_tc_hw_res(struct tc_hw_res *tc_res)
{
	//struct tc_hw_res *tc_res;
	//tc_res = &g_GenConf->hw_res[tc_id];
	printk("=========================TC HW RES ========================\n");
	printk("Flag: %2d\n",tc_res->flag);
	printk("logic_mpe_tc_id: %2d\n",tc_res->logic_mpe_tc_id);
	printk("TC QID: %2d\n",tc_res->TcQid);
	printk("CPU Num: %2d TC Num : %2d\n",tc_res->CpuNum, tc_res->TcNum);
	printk("FwVmbIpi : %2d VmbFwIpi: %2d \n",tc_res->FwVmbIpi, tc_res->VmbFwIpi);
	printk("Yield : %2d\n",tc_res->yield);
	//printk("TC Type: %2d State: %2d\n",tc_res->tcType, tc_res->state);
	dump_tc_current_status();
	printk("MpeCmdReqReg: %2x MpeCmdReqRegIrq: %2d\n",tc_res->MpeCmdReqReg,tc_res->MpeCmdReqRegIrq);
	printk("MpeCmdRespReg: %2x MpeCmdRespRegIrq: %2d\n",tc_res->MpeCmdRespReg,tc_res->MpeCmdRespRegIrq);

	printk("disp_q_semid :%2x \n", tc_res->disp_q_semid);  	
	printk("free_list_semid :%2x \n", tc_res->free_list_semid);  	
	printk("cbm_alloc_semid :%2x \n", tc_res->cbm_alloc_semid);  	
	printk("dispatch_q_cnt_semid :%2x \n", tc_res->dispatch_q_cnt_semid);  	

	printk("Dequeue Port : %2d CbmDeQPortReg: %2x CbmDeQPortRegIrq: %2d\n",tc_res->CbmDeQPort,tc_res->CbmDeQPortReg,tc_res->CbmDeQPortRegIrq);
	printk("Enqueue Port : %2d CbmEnQPortReg: %2x CbmEnQPortRegIrq: %2d\n",tc_res->CbmEnQPort,tc_res->CbmEnQPortReg,tc_res->CbmEnQPortRegIrq);

#ifdef CONFIG_LTQ_PPA_MPE_IP97
	printk("Ring Id : %2d \n",tc_res->e97_ring_id);
#endif
	printk("================================================\n");
	return;
}

void mpe_hal_dump_tc_hw_res_all(void) 
{
	int i=0;

	if(!g_GenConf)
	{
		printk("Genconf is not valid\n");
		return;
	}

	printk("Hw_res:                 =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",i);
	printk("\n");
	printk("\n");
	printk("   Flag:                =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].flag));

	printk("\n");
	printk("   logic_mpe_tc_id:     =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].logic_mpe_tc_id));

	printk("\n");
	printk("   TcQid:               =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].TcQid));

	printk("\n");
	printk("   CpuNum:              =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].CpuNum));

	printk("\n");
	printk("   TcNum:               =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].TcNum));	

	printk("\n");
	printk("   yield:               =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].yield));

	printk("\n");
	printk("   VmbFwIpi:            =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].VmbFwIpi));

	printk("\n");
	printk("   FwVmbIpi:            =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].FwVmbIpi));

	printk("\n");	
	printk("   tcType:              =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].tcType));
	printk("\n");

	printk("   state:               =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].state));
	printk("\n");

	printk("   MpeCmdReqReg:        =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].MpeCmdReqReg));
	printk("\n");

	printk("   MpeCmdReqRegIrq:     =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeCmdReqRegIrq));
	printk("\n");

	printk("   MpeCmdRespReg:       =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].MpeCmdRespReg));
	printk("\n");

	printk("   MpeCmdRespRegIrq:    =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeCmdRespRegIrq));
	printk("\n");

	printk("   McpyPort:            =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].McpyPort));
	printk("\n");


	printk("   McpyTxDescBase:      =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].McpyTxDescBase));
	printk("\n");


	printk("   McpyRxDescBase:      =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].McpyRxDescBase));
	printk("\n");

	printk("   McpyTxCh:            =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].McpyTxCh));
	printk("\n");


	printk("   McpyRxCh:            =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].McpyRxCh));
	printk("\n");


	printk("   McpyCmdReg:          =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].McpyCmdReg));
	printk("\n");

	printk("   McpyRespReg:         =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].McpyRespReg));
	printk("\n");

	printk("   McpyIrq:             =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].McpyIrq));
	printk("\n");

	printk("   CbmDeQPort:          =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].CbmDeQPort));
	printk("\n");

	printk("   CbmDeQPortReg:       =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].CbmDeQPortReg));
	printk("\n");

	printk("   CbmDeQPortRegIrq:    =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].CbmDeQPortRegIrq));
	printk("\n");


	printk("   CbmEnQPort:          =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].CbmEnQPort));
	printk("\n");


	printk("   CbmEnQPortReg:       =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].CbmEnQPortReg));
	printk("\n");

	printk("   CbmEnQPortRegIrq:    =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].CbmEnQPortRegIrq));
	printk("\n");

	printk("   MpeDispatchQid:      =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeDispatchQid));
	printk("\n");


	printk("   MpeDispatchCmdReg:   =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].MpeDispatchCmdReg));
	printk("\n");


	printk("   MpeDispatchCmdRegIrq:=   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeDispatchCmdRegIrq));
	printk("\n");

	printk("   MpeDispatchRespReg:  =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].MpeDispatchRespReg));
	printk("\n");


	printk("   MpeDispatchRespRegIrq=   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeDispatchRespRegIrq));
	printk("\n");

	printk("   MpeDebugQid:         =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeDebugQid));
	printk("\n");

	printk("   MpeDebugCmdReg:      =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].MpeDebugCmdReg));
	printk("\n");

	printk("   MpeDebugCmdRegIrq:   =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeDebugCmdRegIrq));
	printk("\n");

	printk("   MpeDebugRespReg:     =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].MpeDebugRespReg));
	printk("\n");

	printk("   MpeDebugRespRegIrq:  =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeDebugRespRegIrq));
	printk("\n");

	printk("   itc_view:            =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].itc_view));
	printk("\n");

	printk("   disp_q_semid:        =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].disp_q_semid));
	printk("\n");

	printk("   free_list_semid:     =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		if(i < (MAX_TM_NUM + MAX_WORKER_NUM))
			printk("%10x",(g_GenConf->hw_res[i].free_list_semid));
		else
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
			printk("%10x",(g_GenConf->hw_res[i].Dlfree_list_semid));
#endif
	printk("\n");

	printk("   cbm_alloc_semid:     =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].cbm_alloc_semid));
	printk("\n");

	printk("   dispatch_q_cnt_semid:=   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		if(i < (MAX_TM_NUM + MAX_WORKER_NUM))
			printk("%10x",(g_GenConf->hw_res[i].dispatch_q_cnt_semid));
		else
#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
			printk("%10x",(g_GenConf->hw_res[i].Dldispatch_q_cnt_semid));
#endif
	printk("\n");

	printk("   MpeSearchQid:        =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeSearchQid));
	printk("\n");

	printk("   MpeMeterQid:         =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",(g_GenConf->hw_res[i].MpeMeterQid));
	printk("\n");

	printk("   private_ddr_addr:    =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10x",(g_GenConf->hw_res[i].private_ddr_addr));
	printk("\n");

#ifdef CONFIG_LTQ_PPA_MPE_IP97
	printk("   EIP97 Ring Id:       =   ");
	for(i=0;i < MAX_MPE_TC_NUM;i++) 
		printk("%10d",g_GenConf->hw_res[i].e97_ring_id);
#endif
	printk("\n");
}

void mpe_hal_dump_hit_mib(struct seq_file *seq)
{
    int i, j, k;
    uint8_t *baseh;
    unsigned char sum;

    seq_printf(seq, "Hit session(WK):  ");
    for(j=0;j < MAX_WORKER_NUM;j++) 
        seq_printf(seq, "%5u",j);

    seq_printf(seq, "   Total   \n");
    for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
        for(k=0; k< g_GenConf->fw_sess_tbl_num[i]; k++) {
            unsigned char hn[MAX_WORKER_NUM] = {0};
            sum = 0;
            for(j=0;j < MAX_WORKER_NUM;j++) {
                baseh = (uint8_t *)(g_GenConf->fw_sess_hit_tbl_base[i][j] + k * sizeof(uint8_t));
                hn[j] = *baseh;
                sum += hn[j];
            }

            if(sum) {
                seq_printf(seq, " tbl/sess %d/%d:   ", i, k);
                for(j=0; j<MAX_WORKER_NUM; j++)
                    seq_printf(seq, "%5u", hn[j]);
                seq_printf(seq, "%5u\n", sum);	                
            }
        }
    }    
}

void mpe_hal_clear_hit_mib(void)
{
	int i=0,j=0;
	if(g_GenConf)	{
		for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
			for(j=0; j< MAX_WORKER_NUM; j++) {
				memset((void *)g_GenConf->fw_sess_hit_tbl_base[i][j], 0, (g_GenConf->fw_sess_tbl_num[i] * sizeof(uint8_t)));
			}
		}
	}

}

void mpe_hal_dump_tc_mib( struct seq_file *seq)
{
    int i=0;
    if(g_GenConf)	{
        seq_printf(seq,"Hw_res:         =   ");
        for(i=0;i < MAX_MPE_TC_NUM;i++) {
			if(!g_GenConf->hw_res[i].flag)
				continue;
            seq_printf(seq,"%11d  ",i);
        }
        seq_printf(seq,"\n");
        seq_printf(seq,"-------------------------------------------------------------------------------------------------------------------------\n");        
        seq_printf(seq,"   acc_pkt      =   ");
        for(i=0;i < MAX_MPE_TC_NUM;i++) {
			if(!g_GenConf->hw_res[i].flag)
				continue;			
            seq_printf(seq,"%11u  ",(g_GenConf->tc_mib[i].acc_pkt_cnt));
        }
        seq_printf(seq," 	\n");
        seq_printf(seq,"   non_acc_pkt  =   ");
        for(i=0;i < MAX_MPE_TC_NUM;i++) {
			if(!g_GenConf->hw_res[i].flag)
				continue;			
            seq_printf(seq,"%11u  ",(g_GenConf->tc_mib[i].nona_pkt_cnt));
        }
        seq_printf(seq," 	\n");
        seq_printf(seq,"   deq_cnt      =   ");
        for(i=0;i < MAX_MPE_TC_NUM;i++) {
			if(!g_GenConf->hw_res[i].flag)
				continue;			
            seq_printf(seq,"%11u  ",(g_GenConf->tc_mib[i].deq_cnt));
        }
        seq_printf(seq," 	\n");
        seq_printf(seq,"   enq_cnt      =   ");
        for(i=0;i < MAX_MPE_TC_NUM;i++) {
			if(!g_GenConf->hw_res[i].flag)
				continue;			
            seq_printf(seq,"%11u  ",(g_GenConf->tc_mib[i].enq_cnt));
        }
        seq_printf(seq," 	\n");	  
    }
}

void mpe_hal_clear_tc_mib(void)
{
	int i=0;

	if(g_GenConf)	{
		for(i=0;i < MAX_MPE_TC_NUM;i++) {
			memset(&g_GenConf->tc_mib[i], 0, sizeof(struct mib_tc));
		}
	}

}


void mpe_hal_dump_session_mib_cntr(struct seq_file *seq) 
{
    int i, j, k;
    unsigned long long sum;
    unsigned int pkt_sum;
    struct session_mib * bases;		
	
    seq_printf(seq,"Session(WK):");
    for(j=0;j < MAX_WORKER_NUM;j++) 
        seq_printf(seq,"%12u  ",j);
    seq_printf(seq,"          Total            \n");

    for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
        for(k=0; k< g_GenConf->fw_sess_tbl_num[i]; k++) {
            int pn[MAX_WORKER_NUM] = {0}; // accelerated
            int qn[MAX_WORKER_NUM] = {0}; // non-accelerated
            pkt_sum = 0;
            for(j=0;j < MAX_WORKER_NUM;j++) {
                bases = (struct session_mib *)(g_GenConf->fw_sess_mib_tbl_base[i][j] + k * sizeof(struct session_mib));
                pn[j] = bases->mib.pkt;
                qn[j] = bases->mib.non_acc_pkt;
                if(pn[j] > 0 || qn[j] > 0) {
                    pkt_sum = 1;
                    break;
                }
            }

            if(pkt_sum) {
                seq_printf(seq,"tbl%d sess%d\n", i, k);
                sum = 0;
                seq_printf(seq,"    bytes : ");				
                for(j=0;j < MAX_WORKER_NUM;j++) {
                    bases = (struct session_mib *)(g_GenConf->fw_sess_mib_tbl_base[i][j] + k * sizeof(struct session_mib));
                    sum+=bases->mib.bytes;
                    seq_printf(seq,"  %012llu", bases->mib.bytes);					
                }
                seq_printf(seq,"  %012llu\n", sum);

                // accelerated 
                pkt_sum = 0;
                seq_printf(seq,"    pkts  : ");
                for(j=0; j<MAX_WORKER_NUM; j++) {
                    bases = (struct session_mib *)(g_GenConf->fw_sess_mib_tbl_base[i][j] + k * sizeof(struct session_mib));
                    pkt_sum+=bases->mib.pkt;
                    seq_printf(seq,"  %012u", bases->mib.pkt);
                }	
                seq_printf(seq,"  %012u\n", pkt_sum);	

                // non-accelerated
                pkt_sum = 0;
                seq_printf(seq,"nona_pkts  : ");
                for(j=0; j<MAX_WORKER_NUM; j++) {
                    bases = (struct session_mib *)(g_GenConf->fw_sess_mib_tbl_base[i][j] + k * sizeof(struct session_mib));
                    pkt_sum+=bases->mib.non_acc_pkt;
                    seq_printf(seq,"  %012u", bases->mib.non_acc_pkt);
                }	
                seq_printf(seq,"  %012u\n", pkt_sum);	                
            }
       }
    }
}

void mpe_hal_clear_session_mib(void)
{
	int i=0,j=0;
	if(g_GenConf)	{
		for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
			for(j=0; j< MAX_WORKER_NUM; j++) {
				memset((void *)g_GenConf->fw_sess_mib_tbl_base[i][j], 0, (g_GenConf->fw_sess_tbl_num[i] * sizeof(struct session_mib)));
			}
		}
	}

}


void mpe_hal_dump_itf_mib_cntr(struct seq_file *seq)
{
    int i, j;
    struct mpe_itf_mib * base;
    unsigned long long sum;
    unsigned int pkt_sum;

    seq_printf(seq,"Interface(WK):");
    for(i=0;i < MAX_WORKER_NUM;i++) 
        seq_printf(seq,"  %11u",i);
    seq_printf(seq,"          Total\n");

    for(j=0; j<g_GenConf->mib_itf_num; j++) {
        unsigned int rxp[MAX_WORKER_NUM] = {0};
        unsigned int txp[MAX_WORKER_NUM] = {0};

        // Collect RX interface counters
        pkt_sum=0;
        for(i=0; i<MAX_WORKER_NUM; i++) {
            base = (struct mpe_itf_mib *)(g_GenConf->mib_itf_tbl_base[i] + j*sizeof(struct mpe_itf_mib)); 
            rxp[i] = base->rx_mib.pkt;
            pkt_sum += rxp[i];
        }

        if(pkt_sum > 0) {
            seq_printf(seq,"[%3d] RX\n", j);
            sum=0;
            seq_printf(seq,"    bytes : ");
            for(i=0; i<MAX_WORKER_NUM; i++) {
                base = (struct mpe_itf_mib *)(g_GenConf->mib_itf_tbl_base[i] + j*sizeof(struct mpe_itf_mib));
                sum+=base->rx_mib.bytes;
                seq_printf(seq,"  %012llu", base->rx_mib.bytes);
            }	
            seq_printf(seq,"  %012llu\n", sum);

            seq_printf(seq,"    pkts  : ");
            for(i=0; i<MAX_WORKER_NUM; i++)
                seq_printf(seq,"  %012u", rxp[i]);
            seq_printf(seq,"  %012u\n", pkt_sum);            
        }

        // Collect TX interface counters
        pkt_sum=0;
        for(i=0; i<MAX_WORKER_NUM; i++) {
            base = (struct mpe_itf_mib *)(g_GenConf->mib_itf_tbl_base[i] + j*sizeof(struct mpe_itf_mib)); 
            txp[i] = base->tx_mib.pkt;
            pkt_sum += txp[i];
        }

        if(pkt_sum > 0) {
            seq_printf(seq,"[%3d] TX\n", j);
            sum=0;
            seq_printf(seq,"    bytes : ");
            for(i=0; i<MAX_WORKER_NUM; i++) {
                base = (struct mpe_itf_mib *)(g_GenConf->mib_itf_tbl_base[i] + j*sizeof(struct mpe_itf_mib));
                sum+=base->tx_mib.bytes;
                seq_printf(seq,"  %012llu", base->tx_mib.bytes);
            }	
            seq_printf(seq,"  %012llu\n", sum);

            seq_printf(seq,"    pkts  : ");
            for(i=0; i<MAX_WORKER_NUM; i++)
                seq_printf(seq,"  %012u", txp[i]);
            seq_printf(seq,"  %012u\n", pkt_sum);            
        }
    }

}

void mpe_hal_clear_itf_mib(void)
{
	int i=0;
	if(g_GenConf)	{
		for(i=0; i< g_GenConf->mib_itf_num; i++) {
			memset((void *)g_GenConf->mib_itf_tbl_base[i], 0, (g_GenConf->fw_sess_tbl_num[i] * sizeof(struct mpe_itf_mib)));
		}
	}

}

void mpe_hal_debug_cfg(uint32_t ucDbg) 
{

	printk("ucDbg:%d\n",ucDbg);
	g_GenConf->g_mpe_dbg_enable = ucDbg;

}

#if 1 
int gic_pend_reg(uint32_t irq_no)
{
    volatile uint32_t mask = (0xb2320480+((irq_no>>5)*0x4));
    irq_no = (irq_no - ((irq_no>>5)*32));	
    if(CHECK_BIT(REG32(mask), irq_no))
        return 1;

    return 0;
}

int inline gic_mask_reg(uint32_t irq_no)
{
    volatile uint32_t mask = (0xb2320400+((irq_no>>5)*0x4));
    irq_no = (irq_no - ((irq_no>>5)*32));	
    if(CHECK_BIT(REG32(mask), irq_no))
        return 1;

    return 0;
}

void dump_mpe_ctrl_q(struct seq_file *seq)
{
    int i=0;

    if(!g_GenConf)
    {
        seq_printf(seq, "Genconf is not valid\n");
        return;
    }
    seq_printf(seq,"========================MPE CTRL Q STATUS==============================\n");
    seq_printf(seq,"Hw_res:   CmdReg0:  CmdReg1:  RespReg0: RespReg1: CmdIntSts:RespIntSts:\n");
    seq_printf(seq,"=======================================================================\n");
    for(i=0;i < MAX_MPE_TC_NUM;i++) {
        if(!g_GenConf->hw_res[i].flag)
            continue;
        seq_printf(seq,"%02d        %08x  %08x  %08x  %08x  %s       %s\n",
                i, 
                REG32(g_GenConf->hw_res[i].MpeCmdReqReg),
                REG32(g_GenConf->hw_res[i].MpeCmdReqReg+4),
                REG32(g_GenConf->hw_res[i].MpeCmdRespReg),
                REG32(g_GenConf->hw_res[i].MpeCmdRespReg+4),
                ((REG32(MPE_CMD_INT_STS) & (1 << i))?"SET":"CLR"),
                ((REG32(MPE_RESP_INT_STS) & (1 << i))?"SET":"CLR"));
    }
    seq_printf(seq,"=======================================================================\n");	
    seq_printf(seq,"\n");

    return;

}

void dump_mcpy_reg(struct seq_file *seq)
{
    int i=0;

    if(!g_GenConf)
    {
        seq_printf(seq,"Genconf is not valid\n");
        return;
    }
    seq_printf(seq,"=========================MCPY REG STATUS=====================\n");
    seq_printf(seq,"Hw_res:   McpyReg0: McpyReg1: McpyReg2: McpyReg3: McpyIntSts:\n");
    seq_printf(seq,"=============================================================\n");
    for(i=0;i < MAX_MPE_TC_NUM;i++) {
        if(!g_GenConf->hw_res[i].flag)
            continue;
        seq_printf(seq,"%02d        %08x  %08x  %08x  %08x  %s  \n",
                i, 
                REG32(g_GenConf->hw_res[i].McpyCmdReg),
                REG32(g_GenConf->hw_res[i].McpyCmdReg+0x4),
                REG32(g_GenConf->hw_res[i].McpyCmdReg+0x8),
                REG32(g_GenConf->hw_res[i].McpyCmdReg+0xc),
                ((REG32(MPE_MCPY_INT_STAT) & (1 << i))?"SET":"CLR"));
    }
    seq_printf(seq,"=============================================================\n");	
    seq_printf(seq,"\n");

    return;

}

void dump_gic_pend_mask_reg(struct seq_file *seq)
{
    int i=0;

    if(!g_GenConf)
    {
        seq_printf(seq,"Genconf is not valid\n");
        return;
    }
    seq_printf(seq,"====================GIC STATUS=====================\n");
    seq_printf(seq,"Type:     PORT No:  IrqNo:    Pending:  Mask:      \n");
    seq_printf(seq,"===================================================\n");
    for(i=0;i<MAX_TM_NUM;i++) {
        if(!g_GenConf->hw_res[MAX_WORKER_NUM+i].flag)
            continue;
        seq_printf(seq,"CBM DEQ   %02d        %03d       %s       %s\n",
                g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort, 
                g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPortRegIrq, 
                (gic_pend_reg(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPortRegIrq)?"SET":"CLR"), 
                (gic_mask_reg(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPortRegIrq)?"SET":"CLR"));
        seq_printf(seq,"CBM ENQ   %02d        %03d       %s       %s\n",
                g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmEnQPort, 
                g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmEnQPortRegIrq, 
                (gic_pend_reg(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmEnQPortRegIrq)?"SET":"CLR"), 
                (gic_mask_reg(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmEnQPortRegIrq)?"SET":"CLR"));
    }
    seq_printf(seq,"\n");
    for(i=0;i<MAX_MPE_TC_NUM;i++) {
        if(!g_GenConf->hw_res[i].flag)
            continue;
        seq_printf(seq,"MCPY      %02d        %03d       %s       %s\n",
                g_GenConf->hw_res[i].McpyPort, 
                g_GenConf->hw_res[i].McpyIrq, 
                (gic_pend_reg(g_GenConf->hw_res[i].McpyIrq)?"SET":"CLR"), 
                (gic_mask_reg(g_GenConf->hw_res[i].McpyIrq)?"SET":"CLR"));	
    }
    seq_printf(seq,"\n");
    for(i=0;i<MAX_MPE_TC_NUM;i++) {
        if(!g_GenConf->hw_res[i].flag)
            continue;
        seq_printf(seq,"MPE CMD   %02d        %03d       %s       %s\n",
                g_GenConf->hw_res[i].TcQid, 
                g_GenConf->hw_res[i].MpeCmdReqRegIrq, 
                (gic_pend_reg(g_GenConf->hw_res[i].MpeCmdReqRegIrq)?"SET":"CLR"), 
                (gic_mask_reg(g_GenConf->hw_res[i].MpeCmdReqRegIrq)?"SET":"CLR"));	
    }
    for(i=0;i<MAX_MPE_TC_NUM;i++) {
        if(g_GenConf->hw_res[i].flag)
            break;
    }	
    seq_printf(seq,"MPE CMD   %02d        %03d       %s       %s\n",
            g_GenConf->hw_res[i].MpeDispatchQid, 
            g_GenConf->hw_res[i].MpeDispatchCmdRegIrq, 
            (gic_pend_reg(g_GenConf->hw_res[i].MpeDispatchCmdRegIrq)?"SET":"CLR"), 
            (gic_mask_reg(g_GenConf->hw_res[i].MpeDispatchCmdRegIrq)?"SET":"CLR"));	

    seq_printf(seq,"\n");
    for(i=0;i<MAX_MPE_TC_NUM;i++) {
        if(!g_GenConf->hw_res[i].flag)
            continue;
        seq_printf(seq,"MPE RESP  %02d        %03d       %s       %s\n",
                g_GenConf->hw_res[i].TcQid, 
                g_GenConf->hw_res[i].MpeCmdRespRegIrq, 
                (gic_pend_reg(g_GenConf->hw_res[i].MpeCmdRespRegIrq)?"SET":"CLR"), 
                (gic_mask_reg(g_GenConf->hw_res[i].MpeCmdRespRegIrq)?"SET":"CLR"));	
    }
    for(i=0;i<MAX_MPE_TC_NUM;i++) {
        if(g_GenConf->hw_res[i].flag)
            break;
    }
    seq_printf(seq,"MPE RESP  %02d        %03d       %s       %s\n",
            g_GenConf->hw_res[i].MpeDispatchQid, 
            g_GenConf->hw_res[i].MpeDispatchRespRegIrq, 
            (gic_pend_reg(g_GenConf->hw_res[i].MpeDispatchRespRegIrq)?"SET":"CLR"), 
            (gic_mask_reg(g_GenConf->hw_res[i].MpeDispatchRespRegIrq)?"SET":"CLR")); 

    seq_printf(seq,"===================================================\n");	
    seq_printf(seq,"\n");

    return;
}


void dump_cbm_reg(struct seq_file *seq)
{
    int i=0;
    struct cbm_ls_status *ls_status;

    if(!g_GenConf)
    {
        seq_printf(seq,"Genconf is not valid\n");
        return;
    }
    seq_printf(seq,"======CBM REG STS========\n");
    seq_printf(seq,"Type:        Status:     \n");
    seq_printf(seq,"=========================\n");
    for(i=0;i<MAX_TM_NUM;i++) {
        if(!g_GenConf->hw_res[MAX_WORKER_NUM+i].flag)
            continue;

        seq_printf(seq,"CBM_ENQ_IRNEN:   %08x\n", REG32(CBM_ENQ_IRNEN_PORT(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmEnQPort)));       
        seq_printf(seq,"CBM_ENQ_IRNCR:   %08x\n", REG32(CBM_ENQ_IRNCR_PORT(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmEnQPort)));
    }
    seq_printf(seq,"\n");
	
    for(i=0;i<MAX_TM_NUM;i++) {
        if(!g_GenConf->hw_res[MAX_WORKER_NUM+i].flag)
            continue;

        seq_printf(seq,"CBM_IRNEN:       %s\n", CHECK_BIT(REG32(CBM_IRNEN_PORT(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort+4)), 
                    (g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort+8))?"SET":"CLR");       
        seq_printf(seq,"CBM_IRNCR:       %s\n",CHECK_BIT(REG32(CBM_IRNCR_PORT(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort+4)), 
                    (g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort+8))?"SET":"CLR");
    }
    seq_printf(seq,"\n");

    for(i=0;i<MAX_TM_NUM;i++) {
        if(!g_GenConf->hw_res[MAX_WORKER_NUM+i].flag)
            continue;

        seq_printf(seq,"LS_INT_EN:       %s\n",CHECK_BIT(REG32(CBM_LS_INT_EN),
                    (16+(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort*2)))?"SET":"CLR");
        seq_printf(seq,"LS_INTSTS:       %s\n",CHECK_BIT(REG32(CBM_LS_INT_STS),
                    (16+(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort*2)))?"SET":"CLR");
        seq_printf(seq,"LS_REGSTS:       %08x\n",REG32(CBM_LS_STATUS_PORT(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort)));
        ls_status = (struct cbm_ls_status *)(CBM_LS_STATUS_PORT(g_GenConf->hw_res[MAX_WORKER_NUM+i].CbmDeQPort));
        seq_printf(seq,"LS_REGCNT:       %02d\n",ls_status->queue_len);		
    }
    seq_printf(seq,"\n");
    seq_printf(seq,"DISP_Q_CNT:  %02d\n",g_GenConf->dispatch_q_cnt);
    seq_printf(seq,"G_LS_Q_CNT:  %02d\n",g_GenConf->ls_q_cnt);

    seq_printf(seq,"=========================\n");	
    seq_printf(seq,"\n");	
    return;
}


void dump_dma_setting( struct seq_file *seq )
{

    if(!g_GenConf)
    {
        seq_printf(seq,"Genconf is not valid\n");
        return;
    }
    seq_printf(seq,"=========DMA REG SETTING==========\n");
    seq_printf(seq,"Name:        Addr:        Value:\n");
    seq_printf(seq,"==================================\n");
    seq_printf(seq,"DMA4_CLC     %08x     %08x\n",(DMA4_CLC),REG32(DMA4_CLC));
    seq_printf(seq,"DMA4_ID      %08x     %08x\n",(DMA4_ID),REG32(DMA4_ID));
    seq_printf(seq,"DMA4_CTRL    %08x     %08x\n",(DMA4_CTRL),REG32(DMA4_CTRL));
    seq_printf(seq,"DMA4_CPOLL   %08x     %08x\n",(DMA4_CPOLL),REG32(DMA4_CPOLL));	
    seq_printf(seq,"DMA4_CS      %08x     %08x\n",(DMA4_CS),REG32(DMA4_CS));		
    seq_printf(seq,"DMA4_CCTRL   %08x     %08x\n",(DMA4_CCTRL),REG32(DMA4_CCTRL));		
    seq_printf(seq,"DMA4_CDBA    %08x     %08x\n",(DMA4_CDBA),REG32(DMA4_CDBA));		
    seq_printf(seq,"DMA4_CDLEN   %08x     %08x\n",(DMA4_CDLEN),REG32(DMA4_CDLEN));		
    seq_printf(seq,"DMA4_CIE     %08x     %08x\n",(DMA4_CIE),REG32(DMA4_CIE));		
    seq_printf(seq,"DMA4_PS      %08x     %08x\n",(DMA4_PS),REG32(DMA4_PS));		
    seq_printf(seq,"DMA4_PCTRL   %08x     %08x\n",(DMA4_PCTRL),REG32(DMA4_PCTRL));		
    seq_printf(seq,"==================================\n");	
    seq_printf(seq,"\n");	

    return;
}
#endif

void dump_mpe_detailed_debug(struct seq_file *seq)
{
    // dump mpe ctrl registers
    dump_mpe_ctrl_q( seq );
    // dump mcpy registers
    dump_mcpy_reg( seq	);	
    // dump gic pending registers & mask registers
    dump_gic_pend_mask_reg( seq );
    // dump tc current status
    dump_tc_current_status( );	
    // cbm interrupt status
    dump_cbm_reg( seq );
    // dump dma setting
    dump_dma_setting( seq );
    // dump cause reg, epc, status of each tc
}

void mpe_hal_dump_mpe_detailed_dbg(struct seq_file *seq)
{
    dump_mpe_detailed_debug( seq );
}

int32_t mpe_hal_config_accl_mode(uint32_t mode)
{
	if(mode == 0) {
		printk(" Disable MPE Accl\n");
		mpe_hal_remove_fw_connectivity();
		dp_set_gsw_parser(3,0,0,0,0);
	} else if(mode == 1) {
		printk(" Enable MPE Accl\n");
		mpe_hal_set_fw_connectivity();
		dp_set_gsw_parser(3,2,2,0,0);
	} else if(mode == 2) {
		printk("Start MPE FW\n");
		mpe_hal_run_fw(MAX_CPU, g_MpeFwHdr.worker_info.min_tc_num);
	} else if(mode == 3) {
		printk("Stop MPE FW in CPU %d\n",g_MPELaunch_CPU);
		mpe_hal_stop_fw(g_MPELaunch_CPU);
		
	} else {
		printk("Wrong mode!!!\n");
	}
	return 0;
}

/**************************************************************************/

/*
     *  fill all the hardware resources for each TC
     *  for all the hardware resources in tc_hw_res, by default is already filled by MPE FW
     */
static void mpe_hal_update_tc_hw_info(uint8_t cpu_num, uint8_t tc_num, uint8_t tcType, struct tc_hw_res *tc_res)
{
	tc_res->flag = 1; // this hw resource is assigned to the TC
	tc_res->tcType = tcType;
	tc_res->TcNum = tc_num;
	tc_res->CpuNum = cpu_num;
	tc_res->state =  STATE_INACTIVE;
	tc_res->yield = mpe_hal_get_yield(cpu_num, tc_num);
	tc_res->FwVmbIpi = mpe_hal_get_ipi(cpu_num);
	tc_res->VmbFwIpi = mpe_hal_get_vmbfwipi(cpu_num);

	if(tcType != TYPE_TM){
		int32_t j=0;
		for (j=0; j< MAX_MPE_TC_NUM; j++)
		{
			if((g_GenConf->hw_res[j].tcType == TYPE_TM) && (g_GenConf->hw_res[j].CpuNum == cpu_num)){
				break;
			}
		}
		//printk("<%s> TM logic MPE Tc Id is : %d\n",__FUNCTION__,j);

		if(tcType == TYPE_DIRECTLINK) {
			tc_res->Dlfree_list_semid = g_GenConf->hw_res[j].Dlfree_list_semid; 	
			tc_res->Dldispatch_q_cnt_semid = g_GenConf->hw_res[j].Dldispatch_q_cnt_semid;
			tc_res->DlCbmDeQPort = mpe_hal_get_cbm_deq_port(tcType);
		} else {
			//tc_res->itc_view;			
			tc_res->disp_q_semid = g_GenConf->hw_res[j].disp_q_semid;  	
			tc_res->free_list_semid = g_GenConf->hw_res[j].free_list_semid; 	
			tc_res->cbm_alloc_semid = g_GenConf->hw_res[j].cbm_alloc_semid; 	
			tc_res->dispatch_q_cnt_semid = g_GenConf->hw_res[j].dispatch_q_cnt_semid;
		}
#ifdef CONFIG_LTQ_PPA_MPE_IP97
		int32_t id;
		if(mpe_hal_get_ring_id(&id)==PPA_SUCCESS)
		{
			tc_res->e97_ring_id = id;
		}
#endif

	} else { //TM 
		//tc_res->itc_view;			
		tc_res->disp_q_semid = mpe_hal_get_semid(cpu_num, tc_num);  	
		tc_res->free_list_semid = mpe_hal_get_semid(cpu_num, tc_num); 	
		tc_res->cbm_alloc_semid = mpe_hal_get_semid(cpu_num, tc_num); 	
		tc_res->dispatch_q_cnt_semid = mpe_hal_get_semid(cpu_num, tc_num);

//#if defined(CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)

		tc_res->Dlfree_list_semid = mpe_hal_get_semid(cpu_num, tc_num); 	
		tc_res->Dldispatch_q_cnt_semid = mpe_hal_get_semid(cpu_num, tc_num);
//#endif

	}

	tc_res->CbmDeQPort = mpe_hal_get_cbm_deq_port(tcType);
	tc_res->CbmEnQPort = tc_res->CbmDeQPort;
	//mpe_hal_dump_tc_hw_res(tc_res);
	return;
}

void ltq_strcat(char *p, char *q)
{
   while(*p)
      p++;
 
   while(*q)
   {
      *p = *q;
      q++;
      p++;
   }
   *p = '\0';
}

char *itoa(int32_t i )
{
  /* Room for INT_DIGITS digits, - and '\0' */
  static char buf[20];
  char *p = buf + 19;	/* points to terminating '\0' */
  if (i >= 0) {
    do {
      *--p = '0' + (i % 10);
      i /= 10;
    } while (i != 0);
    return p;
  }
  else {			/* i < 0 */
    do {
      *--p = '0' - (i % 10);
      i /= 10;
    } while (i != 0);
    *--p = '-';
  }
  return p;
}

void dump_mpe_version(struct seq_file *seq )
{
#if 1
	char *str;
//	seq_printf(seq, "MPE version:v.%d.%d.%d.%d(%s)\n", VER_MAJ, VER_MID, VER_MIN, VER_TAG, VER_DESC );
	g_mpe_version = kmalloc( sizeof(char) * 4, GFP_KERNEL );
	memset(g_mpe_version, '\0', sizeof(char) * 4);
	ltq_strcat(g_mpe_version, itoa(VER_MAJ));
	ltq_strcat(g_mpe_version, ".");
	ltq_strcat(g_mpe_version, itoa(VER_MID));
	ltq_strcat(g_mpe_version, ".");
	ltq_strcat(g_mpe_version, itoa(VER_MIN));
	ltq_strcat(g_mpe_version, ".");
	ltq_strcat(g_mpe_version, itoa(VER_TAG));
	ltq_strcat(g_mpe_version, "(");
	ltq_strcat(g_mpe_version, VER_DESC);
	ltq_strcat(g_mpe_version, ")");
#endif
	seq_printf(seq, "MPE Version: V.%s\n", g_mpe_version);
	return ;
}

void mpe_session_count ( struct seq_file *seq)
{
	int32_t i=0, count=0;
	struct fw_compare_hash_auto_ipv4 * pIp4CmpTbl;
	pIp4CmpTbl = (struct fw_compare_hash_auto_ipv4 *)g_GenConf->fw_cmp_tbl_base[0];

	struct fw_compare_hash_auto_ipv6 * pIp6CmpTbl;
	pIp6CmpTbl = (struct fw_compare_hash_auto_ipv6 *)g_GenConf->fw_cmp_tbl_base[1];

	for(i = 0; i < g_GenConf->fw_sess_tbl_num[0]; i++ ) {
	//	seq_printf(seq,"Next Entry %d address 0x%x\n",i,((pIp4CmpTbl + (i ))));
		if(((pIp4CmpTbl + (i ))->valid) == 1) {
			count++;
		}
	}
	seq_printf(seq, "IPv4 session count = %d\n", count);

	for(i = 0, count = 0; i < g_GenConf->fw_sess_tbl_num[1]; i++ ) {
	//	seq_printf(seq,"Next Entry %d address 0x%x\n",i,((pIp4CmpTbl + (i ))));
		if(((pIp6CmpTbl + (i ))->valid) == 1) {
			count++;	
		}
	}
	seq_printf(seq, "IPv6 session count = %d\n", count);
	
	return;
}

int mpe_hal_pause_tc(uint8_t ucCpu, uint8_t ucTc)
{
	printk("<%s> Pause CPU->TC %d->%d\n",__FUNCTION__,ucCpu,ucTc);
	if(vmb_tc_pause(ucCpu, ucTc) != VMB_SUCCESS)
		return PPA_FAILURE;
	
	return PPA_SUCCESS;

}

int mpe_hal_resume_tc(uint8_t ucCpu, uint8_t ucTc)
{
	printk("<%s> Resume CPU->TC %d->%d\n",__FUNCTION__,ucCpu,ucTc);
	vmb_tc_resume(ucCpu, ucTc);
	if(vmb_tc_resume(ucCpu, ucTc) != VMB_SUCCESS)
		return PPA_FAILURE;
	
	return PPA_SUCCESS;
}

int mpe_hal_delete_tc(uint8_t ucCpu, uint8_t ucTc)
{
	uint32_t j=0;
	printk("<%s> Delete CPU->TC %d->%d\n",__FUNCTION__,ucCpu,ucTc);
	for (j=0; j< MAX_MPE_TC_NUM; j++)
	{
		if((logic_tc_mapping[j] == ucTc) && (g_GenConf->hw_res[j].CpuNum == ucCpu))
		{
			vmb_tc_stop(ucCpu, ucTc);
			g_GenConf->hw_res[j].flag = 0; // Free this hardware resource
			g_GenConf->hw_res[j].state = STATE_INACTIVE;
			logic_tc_mapping[j] = 0;
			break;
		}	  
	}
	return PPA_SUCCESS;
}

int mpe_hal_add_tc(uint8_t ucCpu, uint32_t tc_type)
{
	uint32_t ret;
	int32_t tc_num = -1, j;
	TC_launch_t tc_launch;

	printk("<%s>Launch TC to CPU %d\n",__FUNCTION__,ucCpu);
	//tc_launch = (uint32_t) kmalloc(sizeof(TC_launch_t), GFP_KERNEL);
	/* Allocate a TC bound to a particular CPU */
	tc_num = vmb_tc_alloc(ucCpu);
	printk("<%s>Allocated TC num %d\n",__FUNCTION__,tc_num);

	if(tc_num == -VMB_ERROR){
		printk("Failed to allocate TC for the CPU %d\n", ucCpu );
		return PPA_FAILURE;
	}else {
		/*  Find the free hw resource for the TC */
		for (j=0; j< MAX_WORKER_NUM; j++)
		{
			if(g_GenConf->hw_res[j].flag == 0)
			{
				break;
			}	  
		}

		if(tc_type == TYPE_DIRECTLINK)
			j= MAX_TM_NUM + MAX_WORKER_NUM;

		printk("<%s>Free hardware resource Index is %d\n",__FUNCTION__, j );
		logic_tc_mapping[j] = tc_num;
		mpe_hal_update_tc_hw_info(ucCpu, tc_num, tc_type, &g_GenConf->hw_res[j]);
		mpe_hal_prepare_tc_launch(tc_num, tc_type, &g_GenConf->hw_res[j], &tc_launch);
		ret = vmb_tc_start(ucCpu, &tc_launch ,1);     
		if((ret == -VMB_ETIMEOUT) ||(ret == -VMB_ENACK))
		{
			printk("Failed to start TC for the CPU %d\n", ucCpu );
			return PPA_FAILURE;
		}  	 
	}
	//return tc_num;
	return j;

}

struct device * g_Mpedev;
int dl_irq_no = 0;
void mpe_hal_dl_enable_gic(int irq_no)
{
	//printk("-----------<%s> IRQ: %d----------------\n",__FUNCTION__,irq_no);
	GIC_SH_WEDGE_REG = (1<<31) + irq_no;
	//gic_send_ipi(irq_no);
	//pr_err("    value: 0x%x\n", GIC_SH_WEDGE_REG);
	if (dl_irq_no == 0)
		dl_irq_no = irq_no;
	//GIC_SH_WEDGE_REG = 0x8000009a;
	return; 

}
EXPORT_SYMBOL(mpe_hal_dl_enable_gic);


#if defined (CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)

dma_addr_t dma_phys;
dma_addr_t dma_phys1;
dma_addr_t dma_phys2;
dma_addr_t dma_phys3;
dma_addr_t dma_phys4;

struct device * mpe_hal_dl_get_dev(void)
{
	return g_Mpedev;
}
EXPORT_SYMBOL(mpe_hal_dl_get_dev);

int mpe_hal_dl_alloc_resource(
        enum MPE_Feature_Type mpeFeature,
        uint32_t *memAddr,
        uint32_t flags)
{
	struct dl_drv_address_map *dl_resource_addr;
	printk("<%s> Allocating resources for DL\n",__FUNCTION__);

	if(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_size != 0 &&
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base == NULL)
#if 0	
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base = (uint32_t)alloc_pages_exact(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_size, GFP_DMA); 
#else
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base = (uint32_t)dma_zalloc_coherent(g_Mpedev,
									g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_size, 
									&dma_phys1,GFP_ATOMIC); 
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base = 
				((g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base & 0x0fffffff) | KSEG0);

#endif
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base)
  	{
    		printk("Failed to allocate memory for tx_cfg_ctxt_buf_base\n");
    		goto CLEANUP;
	}
  
	if(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_size != 0 &&
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base == NULL)
#if 0
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base = (uint32_t)alloc_pages_exact(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_size, GFP_DMA); 
#else
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base = (uint32_t)dma_zalloc_coherent(g_Mpedev,
									g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_size, 
									&dma_phys2,GFP_ATOMIC); 

		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base = 
				((g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base & 0x0fffffff) | KSEG0);
#endif
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base)
  	{
    		printk("Failed to allocate memory for rx_cfg_ctxt_buf_base\n");
    		goto CLEANUP;
	}

	if(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_size != 0 &&
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base == NULL)
#if 0
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base = (uint32_t)alloc_pages_exact(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_size, GFP_DMA); 
#else
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base = (uint32_t)dma_zalloc_coherent(g_Mpedev,
									g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_size, 
									&dma_phys3,GFP_ATOMIC); 
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base = 
				((g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base & 0x0fffffff) | KSEG0);
#endif
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base)
  	{
    		printk("Failed to allocate memory for rx_msgbuf_base\n");
    		goto CLEANUP;
	}

	if(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_base == NULL &&
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_size != 0)
#if 0
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_base = (uint32_t)alloc_pages_exact(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_size, GFP_DMA); 
#else
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_base = (uint32_t)dma_zalloc_coherent(g_Mpedev,
									g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_size, 
									&dma_phys3,GFP_ATOMIC); 
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_base = 
				((g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_base & 0x0fffffff) | KSEG0);
#endif
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].uncached_addr_base)
  	{
    		printk("Failed to allocate memory for uncached_addr_base\n");
    		goto CLEANUP;
	}


	if(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_size != 0 &&
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_base == NULL)
		g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_base = (uint32_t)alloc_pages_exact(g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_size, 
									GFP_KERNEL | __GFP_ZERO); 
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_base)
  	{
    		printk("Failed to allocate memory for comm_buf_base\n");
    		goto CLEANUP;
	}
#if 1
	dl_resource_addr = memAddr;
	dl_resource_addr->dl_buf_info_addr =(uint32_t) &(g_GenConf->fw_hdr.dl_buf_info_base[0]);
	dl_resource_addr->dl_ep_2radio_addr = (uint32_t)&(g_GenConf->dl_ep2radioId_base);
	printk("<%s>: dl_buf_info_addr: 0x%x dl_ep_2radio_addr: 0x%x\n",
                __func__,
                dl_resource_addr->dl_buf_info_addr,
                dl_resource_addr->dl_ep_2radio_addr
                );
#endif
	g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].DlCommmIpi = g_GenConf->hw_res[8].DlCommIpi;
	printk("<%s> DlCommmIpi: %d\n",__FUNCTION__, g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].DlCommmIpi);

	//*memAddr = &g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature];
	return PPA_SUCCESS;
	
CLEANUP:
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base)
		free_pages_exact((void *)g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_base,  g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].tx_cfg_ctxt_buf_size);
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base)
		free_pages_exact((void *)g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_base, g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_cfg_ctxt_buf_size);
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base)
		free_pages_exact((void *)g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_base, g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].rx_msgbuf_size);
	if (!g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_base)
		free_pages_exact((void *)g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_base, g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].comm_buf_size);
	return PPA_FAILURE;

}
EXPORT_SYMBOL(mpe_hal_dl_alloc_resource);
#endif

int mpe_hal_feature_start(
		enum MPE_Feature_Type mpeFeature,
		uint32_t port_id,
		uint32_t * featureCfgBase,
		uint32_t flags)
{
	int32_t tc_id;
	printk("<%s> Flags: %d Mpe Feature: %d\n",__FUNCTION__,flags,mpeFeature);
	if((flags & F_FEATURE_START) == F_FEATURE_START)
	{
		if(mpeFeature == DL_TX_1 || mpeFeature == DL_TX_2)
		{
			if(port_id != 0) { 
				printk("<%s> Start DL Feature for port id %d\n",__FUNCTION__,port_id);
				g_dl1_pmac_port = port_id;
				tc_id = mpe_hal_add_tc(g_MPELaunch_CPU, TYPE_DIRECTLINK);
				if(tc_id != PPA_FAILURE) {
					printk("DL TC is %d: state %s\n",tc_id, get_tc_health_cond(tc_id));
					dl_tc_info[mpeFeature].mpe_tc_id = tc_id;
				} else {
					printk("ADD DL TC failed\n");
					return PPA_FAILURE;
				}

				//g_GenConf->fw_hdr.dl_buf_info_base[mpeFeature].DlCommmIpi = g_GenConf->hw_res[8].DlCommIpi;
				//mpe_hal_add_dl_lookup(port_id);
				if (dl_irq_no)
					mpe_hal_dl_enable_gic(dl_irq_no);
			}
		} else {
			tc_id = mpe_hal_add_tc(g_MPELaunch_CPU, TYPE_WORKER);
		}

	} else if((flags & F_FEATURE_STOP) == F_FEATURE_STOP) {
		if(mpeFeature == DL_TX_1 || mpeFeature == DL_TX_2)
			if(port_id !=0) {
				printk("<%s> Stop DL Feature for port id %d\n",__FUNCTION__,port_id);
				mpe_hal_delete_tc(g_GenConf->hw_res[dl_tc_info[mpeFeature].mpe_tc_id].CpuNum, 
						g_GenConf->hw_res[dl_tc_info[mpeFeature].mpe_tc_id].TcNum);
			}

	} else {
		printk("Wrong flag is passed!!");
		return PPA_FAILURE;
	}
	printk("<%s> EXIT\n",__FUNCTION__);
	return PPA_SUCCESS;
}
EXPORT_SYMBOL(mpe_hal_feature_start);

static int mpe_hal_read_fw_hdr(void)
{
#if FIRMWARE_REQUEST
	int count = 0 , ret = 0;
	pdev = platform_device_register_simple("MPE_FW", 0, NULL, 0); //
	if (IS_ERR(pdev)) {
                printk(KERN_ERR "Failed to register platform device.\n");
                return -EINVAL;
        }
	while (count < 5) {
		//printk("count=%d\n", count);
		if ((ret = request_firmware(&fw_entry, FIRMWARE, &pdev->dev)) == 0) {
			if ( (fw_entry->data != NULL) && (fw_entry->size) ) {
				//printk("******** Request Firmware Addr: 0x%p size: %d ********\n", fw_entry->data, fw_entry->size);
				break;
			} else {
				printk("******** Request Firmware Addr: 0x%p size: %d ********\n", fw_entry->data, fw_entry->size);
				printk("releasing firmware\n");
				release_firmware(fw_entry);
			}
		}		
		count= count + 1;
	}
	if (count == 5 ) {
      	     	printk(KERN_ERR " Firmware not  available error code=%d\n", ret);
               	return PPA_FAILURE ;
	}
       	
	//printk("******** Request Firmware Addr: 0x%p size: %d ********\n", fw_entry->data, fw_entry->size);
	gImage_size = fw_entry->size - sizeof(struct fw_hdr);
	//memcpy(g_MpeFw_load_addr, (fw_entry->data + sizeof(struct fw_hdr)), mpe_alloc_code_len);

    	memset(&g_MpeFwHdr, 0x00 , sizeof(struct fw_hdr));
    	memcpy(&g_MpeFwHdr, fw_entry->data, sizeof(struct fw_hdr));
    	//printk("<%s>Code Size: %d Data Size:%d BSS size:%d\n",__FUNCTION__,g_MpeFwHdr.fw_code_size,g_MpeFwHdr.fw_data_size,g_MpeFwHdr.fw_bss_size);
  	g_GenConf = (struct genconf *)((fw_entry->data + sizeof(struct fw_hdr)) + g_MpeFwHdr.genconf_offset);

	//printk("<%s>Header size %d genconf offset %p\n",__FUNCTION__,g_MpeFwHdr.hdr_size,g_GenConf);
#endif
 
  return PPA_SUCCESS;
}

static int mpe_hal_load_fw(char* filename)
{
	//uint32_t uiFileLen;
	int pages;
	int32_t ret = PPA_SUCCESS;
	unsigned int mpe_alloc_code_len;
	unsigned int mpe_alloc_stack_len;

	//printk("MPE HAL Load FW.\n");

	/* Allocate memory for firmware code 
	   Use GFP_DMA to allocate in the lower 128/256M memory location*/
	mpe_alloc_code_len = g_MpeFwHdr.fw_code_size + g_MpeFwHdr.fw_data_size + g_MpeFwHdr.fw_bss_size;
	//printk("<%s> mpe_alloc_code_len=%d\n",__FUNCTION__,mpe_alloc_code_len);

	if( (mpe_alloc_code_len % TLB_PAGE_SIZE) != 0)
		pages = (mpe_alloc_code_len / TLB_PAGE_SIZE) + 1;
	else
		pages = (mpe_alloc_code_len / TLB_PAGE_SIZE) ;

	//printk("<%s>MPE FW  allocated len:%d No of pages: %d\n",__FUNCTION__, mpe_alloc_code_len, pages);
	g_MpeFw_load_addr = (char *)kmalloc(((pages+2) * TLB_PAGE_SIZE), GFP_DMA);

	if (!g_MpeFw_load_addr)
	{ 
		printk("Can't allocate Memory for Code!");
		ret = PPA_FAILURE;
		goto CLEANUP;
	}
	// Align the buffer with fw_code_align
	g_MpeFw_load_addr = g_MpeFw_load_addr + g_MpeFwHdr.fw_code_align - ((unsigned int) g_MpeFw_load_addr) % g_MpeFwHdr.fw_code_align;
	//printk("<%s>CPU Launch start Virtual address %x\n",__FUNCTION__, g_MpeFw_load_addr);

	// Allocate memory for stack
	mpe_alloc_stack_len = MAX_MPE_TC_NUM * g_MpeFwHdr.fw_stack_priv_data_size;
	if( (mpe_alloc_stack_len % TLB_PAGE_SIZE) != 0)
		pages = (mpe_alloc_stack_len / TLB_PAGE_SIZE) + 1;
	else
		pages = (mpe_alloc_stack_len / TLB_PAGE_SIZE) ;

	g_MpeFw_stack_addr = (char *)kmalloc(((pages+2) * TLB_PAGE_SIZE), GFP_DMA);
	
	if (!g_MpeFw_stack_addr)
	{ 
		printk("Can't Allocate Stack Memory!");
		ret = PPA_FAILURE;
		goto CLEANUP;
	}
	//printk("<%s>CPU Launch stack Virtual address %x\n",__FUNCTION__, g_MpeFw_stack_addr);

	/* Copy the content from file to the memory location */
	//printk("*************************Request Firmware addr=0x%x size=%d\n", fw_entry->data, fw_entry->size);
	memcpy(g_MpeFw_load_addr, (fw_entry->data + sizeof(struct fw_hdr)), mpe_alloc_code_len);

	/*  Get the GenConf information */
	g_GenConf = (struct genconf *)(g_MpeFw_load_addr + g_MpeFwHdr.genconf_offset);

	//printk("MPE HAL g_GenConf->fw_hdr.v_desc=%s\n", g_GenConf->fw_hdr.v_desc);

	if(!strcmp(g_GenConf->fw_hdr.v_desc, "Silver"))
		printk("Genconf offset correct\n");
	else
	{
		printk("Genconf offset wrong !!!!! desc %s\n",g_GenConf->fw_hdr.v_desc);
		ret = PPA_FAILURE;
		goto CLEANUP;
	}

	g_HAL_State = MPE_HAL_FW_LOADED;
	//printk("MPE HAL Load FW Success.\n");
	//return PPA_SUCCESS;

CLEANUP:
	release_firmware(fw_entry);
	platform_device_unregister(pdev);	
	if(ret == PPA_FAILURE) {
		if (g_MpeFw_load_addr)
			kfree(g_MpeFw_load_addr);

		if (g_MpeFw_stack_addr)
			kfree(g_MpeFw_stack_addr);
	}
	return ret;
}

static int mpe_hal_allocate_fw_table(void)
{
	uint32_t i=0, j=0;
	//printk("MPE HAL Allocate FW Data.\n");

	g_GenConf->eva_config_flag = 7;
	//printk("segment control registers SEG0 = 0:%.8x SEG1 = 1:%.8x SEG2 = 2:%.8x\n",read_c0_segctl0(),read_c0_segctl1(),read_c0_segctl2());
	g_GenConf->eva_SegCtl0 = read_c0_segctl0();
	g_GenConf->eva_SegCtl1 = read_c0_segctl1();
	g_GenConf->eva_SegCtl2 = read_c0_segctl2();  

#ifdef CONFIG_IP_TABLE
	g_GenConf->ipv4_num = ;  
	g_GenConf->ipv4_tbl_base = (uint32_t)kmalloc(g_GenConf->ipv4_num * 4, GFP_KERNEL );
	if (!g_GenConf->ipv4_tbl_base)
	{       
		printk("Failed to allocate memory for IPv4 table\n");	
		return PPA_FAILURE;
	}

	g_GenConf->ipv6_num = ;
	g_GenConf->ipv6_tbl_base = (uint32_t)kmalloc(g_GenConf->ipv6_num * 16, GFP_KERNEL );
	if (!g_GenConf->ipv6_tbl_base)
	{
		printk("Failed to allocate memory for IPv6 table\n");
		return PPA_FAILURE;
	}
#endif

	/* Configuration required for Full Precessing */
	/* Compare Table for IPv4 session */
	g_GenConf->fw_sess_tbl_type[0] = 1; //IPv4 table
	g_GenConf->fw_sess_tbl_num[0] = MAX_HW_SESSION_NUM;
	g_GenConf->fw_sess_tbl_iterate[0] = MAX_SEARCH_ITRN;
	g_GenConf->fw_cmp_tbl_base[0] = (uint32_t)kmalloc(g_GenConf->fw_sess_tbl_num[0]* sizeof(struct fw_compare_hash_auto_ipv4), GFP_DMA);
	if (!g_GenConf->fw_cmp_tbl_base[0])
	{
		printk("Failed to allocate memory for IPv4 compare table\n");
		return PPA_FAILURE;
	} else { 
		//printk("IPv4 compare table address %x\n", g_GenConf->fw_cmp_tbl_base[0]);
		memset((void *)g_GenConf->fw_cmp_tbl_base[0], 0, g_GenConf->fw_sess_tbl_num[0]* sizeof(struct fw_compare_hash_auto_ipv4));
		for(i = 0; i < g_GenConf->fw_sess_tbl_num[0] ; i++ ) {
			//printk("Next Entry %d address 0x%x\n",i,((struct fw_compare_hash_auto_ipv4 *)(g_GenConf->fw_cmp_tbl_base[0] + (i * sizeof(struct fw_compare_hash_auto_ipv4)))));
			((struct fw_compare_hash_auto_ipv4 *)(g_GenConf->fw_cmp_tbl_base[0] + (i * sizeof(struct fw_compare_hash_auto_ipv4))))->valid=0;
			((struct fw_compare_hash_auto_ipv4 *)(g_GenConf->fw_cmp_tbl_base[0] + (i * sizeof(struct fw_compare_hash_auto_ipv4))))->nxt_ptr= g_GenConf->fw_sess_tbl_num[0] - 1;
			((struct fw_compare_hash_auto_ipv4 *)(g_GenConf->fw_cmp_tbl_base[0] + (i * sizeof(struct fw_compare_hash_auto_ipv4))))->first_ptr= g_GenConf->fw_sess_tbl_num[0] - 1;
			((struct fw_compare_hash_auto_ipv4 *)(g_GenConf->fw_cmp_tbl_base[0] + (i * sizeof(struct fw_compare_hash_auto_ipv4))))->act=0;

		}
	}


	/* Compare Table for IPv6 session */
	g_GenConf->fw_sess_tbl_type[1] = 2; //IPv6 table
	g_GenConf->fw_sess_tbl_num[1] = MAX_HW_SESSION_NUM;
	g_GenConf->fw_sess_tbl_iterate[1] = MAX_SEARCH_ITRN;
	g_GenConf->fw_cmp_tbl_base[1] = (uint32_t)kmalloc(g_GenConf->fw_sess_tbl_num[1]* sizeof(struct fw_compare_hash_auto_ipv6), GFP_DMA);
	if (!g_GenConf->fw_cmp_tbl_base[1])
	{
		printk("Failed to allocate memory for IPv6 compare table\n");
		return PPA_FAILURE;
	} else {
		//printk("IPv6 compare table address %x\n", g_GenConf->fw_cmp_tbl_base[1]);
		memset((void *)g_GenConf->fw_cmp_tbl_base[1], 0, g_GenConf->fw_sess_tbl_num[1]* sizeof(struct fw_compare_hash_auto_ipv6));
		for(i = 0; i < g_GenConf->fw_sess_tbl_num[1]; i++ ) {
			((struct fw_compare_hash_auto_ipv6 *)(g_GenConf->fw_cmp_tbl_base[1] + (i * sizeof(struct fw_compare_hash_auto_ipv6))))->valid=0;
			((struct fw_compare_hash_auto_ipv6 *)(g_GenConf->fw_cmp_tbl_base[1] + (i * sizeof(struct fw_compare_hash_auto_ipv6))))->nxt_ptr= g_GenConf->fw_sess_tbl_num[1] - 1;
			((struct fw_compare_hash_auto_ipv6 *)(g_GenConf->fw_cmp_tbl_base[1] + (i * sizeof(struct fw_compare_hash_auto_ipv6))))->first_ptr= g_GenConf->fw_sess_tbl_num[1] - 1;
			((struct fw_compare_hash_auto_ipv6 *)(g_GenConf->fw_cmp_tbl_base[1] + (i * sizeof(struct fw_compare_hash_auto_ipv6))))->act=0;
		}
	}

	/* Complementary table */  
	g_GenConf->hw_act_num = MAX_HW_SESSION_NUM;
	g_GenConf->hw_act_tbl_base = (uint32_t)kmalloc(g_GenConf->hw_act_num * sizeof(struct hw_act_ptr), GFP_DMA);
	if (!g_GenConf->hw_act_tbl_base)
	{
		printk("Failed to allocate memory for hardware complementary table\n");
		return PPA_FAILURE;
	}
	memset((void *)g_GenConf->hw_act_tbl_base, 0 , g_GenConf->hw_act_num * sizeof(struct hw_act_ptr));

	/* Multicast VAP table base */
	for(i=0; i<MAX_PMAC_PORT; i++ )
	{
		/* Fixed Port 0: CPU 1-6: LAN Port 15: WAN Port 
		   Port 7-14 are dynamic ports */	
		if(i <=6 || i == 15)
			g_GenConf->mc_vap_num[i] = 0 ; // No Multicast VAP support for this PMAC port
		else
		{
			g_GenConf->mc_vap_num[i] = MAX_MC_NUM;
			g_GenConf->mc_vap_tbl_base[i] = (uint32_t)kmalloc(g_GenConf->mc_vap_num[i] * sizeof(struct vap_entry), GFP_DMA);
			if (!g_GenConf->mc_vap_tbl_base[i])
			{
				printk("Failed to allocate memory for Multicast VAP table for PMAC port\n");
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->mc_vap_tbl_base[i], 0 , (g_GenConf->mc_vap_num[i] * sizeof(struct vap_entry)));
		}
	}

	/** Session Based Hit and MIB counters allocation */
	for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
		for(j=0; j< MAX_WORKER_NUM; j++) {
                        g_GenConf->fw_sess_hit_tbl_base[i][j] = (uint32_t ) kmalloc((g_GenConf->fw_sess_tbl_num[i] * sizeof(uint8_t)), GFP_DMA);
			if (!g_GenConf->fw_sess_hit_tbl_base[i][j])
			{
				printk("Failed to allocate memory for Session HIT table for CMP table index %d and Worker %d\n",i,j);
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->fw_sess_hit_tbl_base[i][j], 0, (g_GenConf->fw_sess_tbl_num[i] * sizeof(uint8_t)));
		}
	}

	for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
		for(j=0; j< MAX_WORKER_NUM; j++) {
                        g_GenConf->fw_sess_mib_tbl_base[i][j] = (uint32_t )kmalloc((g_GenConf->fw_sess_tbl_num[i] * sizeof(struct session_mib)), GFP_DMA);
			if (!g_GenConf->fw_sess_mib_tbl_base[i][j])
			{
				printk("Failed to allocate memory for Session MIB table for CMP table index %d and Worker %d\n",i,j);
				return PPA_FAILURE;
			}
			memset((void *)g_GenConf->fw_sess_mib_tbl_base[i][j], 0, (g_GenConf->fw_sess_tbl_num[i] * sizeof(struct session_mib)));
		}
	}

	/** MIB interface table base address per Worker TC  */
	for(i=0; i< MAX_WORKER_NUM; i++) {
		g_GenConf->mib_itf_tbl_base[i] = (uint32_t )kmalloc((g_GenConf->mib_itf_num * sizeof(struct mpe_itf_mib)), GFP_DMA);
		if (!g_GenConf->mib_itf_tbl_base[i])
		{
			printk("Failed to allocate memory for Session MIB table for Worker %d\n",i);
			return PPA_FAILURE;
		}
		memset((void *)g_GenConf->mib_itf_tbl_base[i], 0, g_GenConf->mib_itf_num * sizeof(struct mpe_itf_mib));
	}

	g_HAL_State = MPE_HAL_FW_RESOURCE_ALLOC;
	//printk("MPE HAL Allocate FW Data Success.\n");
	return PPA_SUCCESS;

}

static int32_t mpe_hal_free_fw_table(void)
{
	int32_t i=0, j=0;
	printk("<%s> ---> Enter <-----.\n",__FUNCTION__);
	for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
		if(g_GenConf->fw_cmp_tbl_base[i] != 0)
			g_GenConf->fw_cmp_tbl_base[i] = 0;
	}
	if(g_GenConf->hw_act_tbl_base)
		g_GenConf->hw_act_tbl_base = 0;

	for(i=0; i<MAX_PMAC_PORT; i++ )
	{
		if(i > 7 || i < 15)
			g_GenConf->mc_vap_tbl_base[i] = 0;
	}

	for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
		for(j=0; j< MAX_WORKER_NUM; j++) {
			if (g_GenConf->fw_sess_hit_tbl_base[i][j])
				g_GenConf->fw_sess_hit_tbl_base[i][j] = 0;
		}
	}
	for(i=0; i< MAX_CMP_TABLE_SUPPORTED; i++) {
		for(j=0; j< MAX_WORKER_NUM; j++) {
			if (g_GenConf->fw_sess_mib_tbl_base[i][j])
				g_GenConf->fw_sess_mib_tbl_base[i][j] = 0;
		}
	}
	for(i=0; i< MAX_WORKER_NUM; i++) {
		if (g_GenConf->mib_itf_tbl_base[i])
			g_GenConf->mib_itf_tbl_base[i] = 0;
	}
	g_HAL_State = MPE_HAL_FW_LOADED;
	printk("<%s> ---> Exit <-----.\n",__FUNCTION__);
	return PPA_SUCCESS;
}


static int mpe_hal_run_fw(uint8_t ucCpu, uint8_t ucNum_worker)
{
	int32_t ret = PPA_SUCCESS;
	uint8_t tc_num;
	int8_t ucCpuNum = -1;
	uint32_t i=0, j; 
	uint32_t status = 0;
	int pages;
	u32 phy_addr ;

	CPU_launch_t c_lunch;
	TC_launch_t *tc_launch=NULL;

	//printk("<%s> ---> Enter <-----.\n",__FUNCTION__);
	/* Get the CPU number from VMB */
	ucCpuNum = vmb_cpu_alloc(ucCpu, "MPEFW");
	//printk("<%s> CPU Number: %d\n",__FUNCTION__,ucCpuNum);
	if( (ucCpuNum == -VMB_EBUSY) || (ucCpuNum == -VMB_EAVAIL) )
	{
		printk("EBUSY !!!!\n");
		ucCpuNum = vmb_cpu_alloc(MAX_CPU, "MPEFW");
		if( (ucCpuNum == -VMB_EBUSY) || (ucCpuNum == -VMB_EAVAIL) )
		{
			printk("No CPU is free\n");
			ret = -VMB_EAVAIL;
			goto MPE_HAL_RUN_FW_FAILURE_HANDLER;
		}
	}
	g_MPELaunch_CPU = ucCpuNum;

	/*  Find the free hw resource(6-7) for the TC */
	for (j=MAX_WORKER_NUM; j< (MAX_WORKER_NUM+1); j++)
	{
		if(g_GenConf->hw_res[j].flag == 0)
		{
			break;
		}	  
	}

	g_GenConf->ddr_address = 0x20000000;
	g_GenConf->vmb_fw_msg_base[ucCpuNum]= (uint32_t ) VMB_get_msg_addr(ucCpuNum,0);
	g_GenConf->fw_vmb_msg_base[ucCpuNum]= (uint32_t ) VMB_get_msg_addr(ucCpuNum,1);
	g_GenConf->g_mpe_dbg_enable = 0;

	//printk("<%s>vmb_fw_msg_base %x fw_vmb_msg_base %x Cpunum: %d\n",__FUNCTION__,g_GenConf->vmb_fw_msg_base[ucCpuNum],g_GenConf->fw_vmb_msg_base[ucCpuNum],ucCpuNum);

	g_VpeInfo.vpe[j-MAX_WORKER_NUM].ucState = 1;
	g_VpeInfo.vpe[j-MAX_WORKER_NUM].ucActualVpeNo = ucCpuNum;
	tc_num = get_tm_from_vpe(ucCpuNum);

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_EIP97
	g_GenConf->e97_init_flag = 1;
#endif

#ifdef CONFIG_LTQ_PPA_MPE_IP97
	for(i=0; i<MAX_RING;i++)
	{
		ring_id_pool[i] = -1;
	}
	ring_id_pool[0] = 0; //used by Linux
	mpe_hal_alloc_cdr_rdr();
	g_GenConf->e97_init_flag = 1;
#endif
	/* Allocate a TC bound to a particular CPU */
	//j=6;  // TBR right now hard coded
	//printk("<%s> TM Logical index is: %d mapped to Core tc number: %d\n",__FUNCTION__, j, tc_num);  
	logic_tc_mapping[j] = tc_num;
	mpe_hal_update_tc_hw_info(ucCpuNum, tc_num, TYPE_TM, &g_GenConf->hw_res[j]);

	/*  Prepare the c_lunch */
	memset(&c_lunch, 0, sizeof(CPU_launch_t));

#ifndef FIRMWARE_REQUEST
	if( (mpe_fw_len % TLB_PAGE_SIZE) != 0)
		pages = (mpe_fw_len / TLB_PAGE_SIZE) + 1;
	else
		pages = (mpe_fw_len / TLB_PAGE_SIZE) ;
#else 
	if( (gImage_size % TLB_PAGE_SIZE) != 0)
		pages = (gImage_size / TLB_PAGE_SIZE) + 1;
	else
		pages = (gImage_size / TLB_PAGE_SIZE) ;
#endif
	//printk("No of pages ---- %d\n",pages);
	phy_addr = VIR_TO_PHY((uint32_t)g_MpeFw_load_addr);
	//mpe_tlb_setting_init(phy_addr, 0xC0000000, pages, &tlb_info);

	//mpe_tlb_set(&tlb_info);


//#if defined (CONFIG_ACCL_11AC) || defined(CONFIG_ACCL_11AC_MODULE)
	g_GenConf->dl_buffer_free_tbl_base = (uint32_t)&dl_dispatch_buffer[0];
	g_GenConf->max_tm_dl_deq_pkt_num = 1;
	//g_GenConf->dl_dispatch_q_buf_num = 10;
//#endif
	g_GenConf->buffer_free_tbl_base = (uint32_t)&dispatch_buffer[0];
	g_GenConf->max_tm_deq_pkt_num = 1;
	g_GenConf->dispatch_q_buf_num = 10;
	g_GenConf->pmac_len_for_csum_offload = 8;

	//if(g_GenConf->g_mpe_dbg_enable)
	{
		int err = 0;
		//printk("<%s> MPE FW Debug Enabled %d\n",__FUNCTION__,g_GenConf->hw_res[0].MpeDebugRespRegIrq);
		err = request_irq(g_GenConf->hw_res[0].MpeDebugRespRegIrq + 8, print_content, IRQF_DISABLED, "mpe_dbg", NULL);
		if (err)
			printk ("request_irq for IRQ mpe_dbg = %d failed !!! \n", g_GenConf->hw_res[0].MpeDebugRespRegIrq);
	}

	//c_lunch.start_addr = (unsigned long)g_MpeFw_load_addr; //g_MpeFwHdr.tm_info.start_addr;
	c_lunch.start_addr = CKSEG1ADDR((unsigned long)g_MpeFw_load_addr);

	c_lunch.sp = (uint32_t)(g_MpeFw_stack_addr + (g_GenConf->fw_hdr.fw_stack_priv_data_size * (j+1)));
	//printk("<%s>CPU Launch stack pointer %x\n",__FUNCTION__, c_lunch.sp);

	c_lunch.a0 =(uint32_t) &tlb_info;
	c_lunch.priv_info = j; //only the hw_res index has to be passed
	//printk("CPU Launch start private info %x\n",c_lunch.priv_info);

	//printk("<%s>Start VMB CPU %d start_addr  = %x, SP =%x , cpu_lainch =%p \n",__FUNCTION__,ucCpuNum, c_lunch.start_addr, c_lunch.sp, &c_lunch);
	//printk("<%s>a0  = %x, \n",__FUNCTION__,c_lunch.a0);
	//printk("<%s>TC Hw Res  = 0x%p IPI: %d\n",__FUNCTION__,&g_GenConf->hw_res[j], g_GenConf->hw_res[j].FwVmbIpi);

	tc_launch = (TC_launch_t *) kmalloc(ucNum_worker * sizeof(TC_launch_t), GFP_KERNEL);
	/* Find a free worker TC (0-5). For all the ucNum_worker worker TC's*/
	for (i=0; i< ucNum_worker; i++)
	{
		//printk("Worker  num %d\n",i);
		tc_num = vmb_tc_alloc(ucCpuNum);
		//printk("<%s>Allocated TC num %d\n",__FUNCTION__,tc_num);

		if(tc_num == -VMB_ERROR){
			printk("Failed to allocate TC for the CPU %d\n", ucCpu );
			ret = VMB_EAVAIL;
			goto MPE_HAL_RUN_FW_FAILURE_HANDLER;
		}else {
			logic_tc_mapping[i] = tc_num;
			//printk("TC Number: %d\n",tc_num);
			mpe_hal_update_tc_hw_info(ucCpuNum, tc_num, TYPE_WORKER, &g_GenConf->hw_res[i]);
			mpe_hal_prepare_tc_launch(tc_num, TYPE_WORKER, &g_GenConf->hw_res[i], &tc_launch[i]);
		}
	}

	/* Register the callback handler to VMB */
	vmb_register_callback(ucCpuNum, (void *)mpe_hal_to_vmb_callback_hdlr(status));

	ppa_memcpy((void *)c_lunch.start_addr, g_MpeFw_load_addr, (pages * TLB_PAGE_SIZE));

	//printk("eva_cfg_pa = %d eva_cfg_va=%d\n", g_GenConf->eva_cfg_pa, g_GenConf->eva_cfg_va);

	/* Start the CPU*/
	//ret = vmb_cpu_start(ucCpuNum, "MPEFW", c_lunch, tc_launch, ucNum_worker);     
	ret = vmb_cpu_start(ucCpuNum, c_lunch, tc_launch, ucNum_worker, 0);     
	/*if((ret == -VMB_ETIMEOUT) ||(ret == -VMB_ENACK))
	  {
	  printk("Start VMB CPU Failure !!!\n");
	  printk("eva_cfg_pa = %x eva_cfg_va=%x\n", g_GenConf->eva_cfg_pa, g_GenConf->eva_cfg_va);
	  goto MPE_HAL_RUN_FW_FAILURE_HANDLER;
	  } */

	if(ret == VMB_SUCCESS) {
		//printk("eva_cfg_pa = %x eva_cfg_va=%x\n", g_GenConf->eva_cfg_pa, g_GenConf->eva_cfg_va);
#ifdef CONFIG_SOC_GRX500_A21
		mpe_hal_set_fw_connectivity();
		dp_set_gsw_parser(3,2,2,0,0);
#endif
		mpe_hal_config_pmac_port_len();
		g_HAL_State = MPE_HAL_FW_RUNNING;
		printk("MPE HAL Run FW success .\n");
		kfree(tc_launch);
		return PPA_SUCCESS;
	}

MPE_HAL_RUN_FW_FAILURE_HANDLER:
	if(ret == -VMB_EAVAIL)
	{ 	
		printk("CPU is not available.!!!\n");
		if(ucCpuNum >=0)
			vmb_cpu_free(ucCpuNum);
	}
	if((ret == -VMB_ETIMEOUT) ||(ret == -VMB_ENACK))
	{
		int j;
		printk("CPU Start is failing.!!!\n");
		vmb_tc_free(ucCpuNum, -1); 
		vmb_cpu_free(ucCpuNum);

		/* For all the ucNum_worker worker TC's*/
		for (j=0; j< MAX_MPE_TC_NUM; j++)
		{
			logic_tc_mapping[j] = 0;
			g_GenConf->hw_res[j].flag = 0;
			g_GenConf->hw_res[j].state = STATE_INACTIVE;
		}
		printk("Start VMB CPU Failure !!!\n");
		printk("eva_cfg_pa = %x eva_cfg_va=%x\n", g_GenConf->eva_cfg_pa, g_GenConf->eva_cfg_va);
	}
	return PPA_FAILURE;
}

static int mpe_hal_stop_fw(uint8_t ucCpu)
{
	uint32_t j;
	/*  Free all the hardware resources which are assiciated to this CPU */
	for (j=0; j< MAX_MPE_TC_NUM; j++)
	{
		g_GenConf->hw_res[j].flag = 0; // Free this hardware resource
		g_GenConf->hw_res[j].state = STATE_INACTIVE;
		logic_tc_mapping[j] = 0;
		if((g_GenConf->hw_res[j].CpuNum == ucCpu) && (g_GenConf->hw_res[j].tcType == TYPE_WORKER))
		{
			printk("<%s> Delete CPU->TC %d->%d\n",__FUNCTION__,ucCpu, g_GenConf->hw_res[j].TcNum);
			mpe_hal_free_yield(ucCpu, 1);
			vmb_tc_stop(ucCpu, g_GenConf->hw_res[j].TcNum);
		} else  if((g_GenConf->hw_res[j].CpuNum == ucCpu) && (g_GenConf->hw_res[j].tcType == TYPE_TM)) {
			printk("Stopping the TM !!!\n");
			vmb_cpu_stop(ucCpu);
			mpe_hal_free_semid(g_GenConf->hw_res[j].disp_q_semid);
			mpe_hal_free_semid(g_GenConf->hw_res[j].free_list_semid);
			mpe_hal_free_semid(g_GenConf->hw_res[j].cbm_alloc_semid);
			mpe_hal_free_semid(g_GenConf->hw_res[j].dispatch_q_cnt_semid);
		}
	}
	for (j=0; j< MAX_VPE_NUM; j++)
	{
		if(g_VpeInfo.vpe[j].ucActualVpeNo == ucCpu)
		{
			g_VpeInfo.vpe[j].ucState = 0;      
		}	  
	}
	free_irq(g_GenConf->hw_res[0].MpeDebugRespRegIrq + 8, NULL);
	mpe_hal_remove_fw_connectivity();
	dp_set_gsw_parser(3,0,0,0,0);
	g_HAL_State = MPE_HAL_FW_RESOURCE_ALLOC;
	return PPA_SUCCESS;
}

static int32_t mpe_hal_deregister_caps(void)
{
        ppa_drv_deregister_cap(SESS_IPV4,MPE_HAL);
        ppa_drv_deregister_cap(SESS_IPV6,MPE_HAL);
        ppa_drv_deregister_cap(TUNNEL_L2TP_US,MPE_HAL);
        ppa_drv_deregister_cap(TUNNEL_GRE_US,MPE_HAL);
        ppa_drv_deregister_cap(SESS_MC_DS_VAP,MPE_HAL);
#ifdef CONFIG_LTQ_PPA_MPE_IP97
        ppa_drv_deregister_cap(TUNNEL_IPSEC_US,MPE_HAL);
        ppa_drv_deregister_cap(TUNNEL_IPSEC_DS,MPE_HAL);
        ppa_drv_deregister_cap(TUNNEL_IPSEC_MIB,MPE_HAL);
#endif

        return PPA_SUCCESS;
}



static void mpe_register_hal(void)
{
	//printk("Register MPE HAL to PPA.\n");
	ppa_drv_generic_hal_register(MPE_HAL, mpe_hal_generic_hook);
}

#if 1
static int mpe_xrx500_probe(struct platform_device *pdev)
{
	g_Mpedev = &(pdev->dev);
	return 0;
}

static int  mpe_xrx500_release(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id mpe_xrx500_match[] = {
    { .compatible = "lantiq,mpe-xrx500" },
    {},
};

static struct platform_driver mpe_xrx500_driver = {
    .probe = mpe_xrx500_probe,
    .remove = mpe_xrx500_release,
    .driver = {
                .name = "mpe-xrx500",
                .owner = THIS_MODULE,
                .of_match_table = mpe_xrx500_match,
        },
};

#endif

static int32_t hal_init(void) 
{
	platform_driver_register(&mpe_xrx500_driver);
	//printk("MPE HAL FW State : %d\n",g_HAL_State);

	if(g_HAL_State < MPE_HAL_FW_LOADED)
		goto LOAD_FW;
	else if (g_HAL_State < MPE_HAL_FW_RESOURCE_ALLOC)
		goto ALLOCATE_FW_DATA;
	else if (g_HAL_State < MPE_HAL_FW_RUNNING)
		goto RUN_FW;

LOAD_FW:
#ifndef NO_FW_HDR
	if(mpe_hal_read_fw_hdr() == PPA_FAILURE)
	{
		printk("File not found.\n");
		return PPA_FAILURE;
	}
#endif
	if(mpe_hal_load_fw( MPE_FILE_PATH) == PPA_FAILURE)
	{
		printk("Insufficient memory for MPE FW.\n");
		return PPA_FAILURE;
	}
ALLOCATE_FW_DATA:
	if(mpe_hal_allocate_fw_table() == PPA_FAILURE)
	{
		printk("Insufficient memory for MPE FW tables allocation\n");
		return PPA_FAILURE;
	}
RUN_FW:
#if 1
#ifndef NO_FW_HDR
	printk("Min TC:%d Max TC:%d\n",g_MpeFwHdr.worker_info.min_tc_num, g_MpeFwHdr.worker_info.max_tc_num);
	if(mpe_hal_run_fw( MAX_CPU, g_MpeFwHdr.worker_info.min_tc_num) == PPA_FAILURE)
#else
	if(mpe_hal_run_fw( MAX_CPU, 0) == PPA_FAILURE)
#endif
	{
		printk("Cannot run MPE FW.\n");
		return PPA_FAILURE;
	}      
#endif
	mpe_hal_feature_start_fn = mpe_hal_feature_start;	
	mpe_register_hal();
	return 0;

}

int32_t mpe_hal_fw_load(void)
{
	hal_init();
	return 0;
}

static int __init mpe_hal_init(void)
{
	//printk("MPE FW Init.\n");
	if(hal_init() == PPA_FAILURE)
	{
		printk("HAL Init Failed !!!!!\n");
		return PPA_FAILURE;
	}
	mpe_hal_proc_create();
		
#ifdef CONFIG_LTQ_PPA_MPE_IP97
	mpe_hal_set_ipsec_loopback_connectivity();
#endif

#ifdef MPE_HAL_TEST 
	int32_t tc;
	tc = mpe_hal_add_tc(g_MPELaunch_CPU, TYPE_WORKER); 
	printk("<%s> Added TC %d\n",__FUNCTION__,tc);
	mpe_hal_delete_tc(g_MPELaunch_CPU, tc);
#endif
	return 0;
}

static int32_t hal_uninit(void)
{
	printk("<%s> Unregister platform driver \n",__FUNCTION__);
	platform_driver_unregister(&mpe_xrx500_driver);
	printk("<%s> Deregister HAL driver \n",__FUNCTION__);
	ppa_drv_generic_hal_deregister(MPE_HAL);
	mpe_hal_feature_start_fn = NULL;	

	return 0;
}

int32_t mpe_hal_fw_unload(void)
{

	mpe_hal_free_fw_table();
	mpe_hal_stop_fw(g_MPELaunch_CPU);
#ifdef CONFIG_LTQ_PPA_MPE_IP97
	mpe_hal_free_cdr_rdr();
	mpe_hal_remove_ipsec_loopback_connectivity();
#endif
	hal_uninit();
	kfree(g_MpeFw_stack_addr);
	kfree(g_MpeFw_load_addr);

	g_HAL_State = MPE_HAL_FW_NOT_LOADED;
	return PPA_SUCCESS;
}

static void __exit mpe_hal_exit(void)
{
	hal_uninit();
	mpe_hal_proc_destroy();
}


module_init(mpe_hal_init);
module_exit(mpe_hal_exit);

MODULE_LICENSE("GPL");


