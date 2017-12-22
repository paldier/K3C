#ifndef MPE_DEBUG_HDR_H
#define MPE_DEBUG_HDR_H

#define MPE_RESP_INT_STS  0xE1000420
#define MPE_CMD_INT_STS  0xE1000424
#define MPE_MCPY_INT_STAT	0xE1300090

#define CHECK_BIT(var, pos) (((var) & (1<<(pos)))?1:0)
#define MPE_SET_BIT(var, pos) (var |= (1 << pos))
#define MPECTRL_RES_IRQ(qid)            (236+qid)       // Total 12 q
#define MAP_TO_PIN 31

#define CBM_IRNCR_PORT(port)		(0xbe710000+0x0+((port)*0x40))
#define CBM_IRNEN_PORT(port)		(0xbe710000+0x8+((port)*(0x40)))
#define CBM_LS_INT_STS				(0xbe760000+0x910)
#define CBM_LS_INT_EN				(0xbe760000+0x918)
#define CBM_LS_STATUS_PORT(port)	(0xbe760000+0x14+((port)*(0x114-0x14)))
// ENQ Interrupt regs
#define CBM_ENQ_IRNCR_PORT(port)	(0xbe780000+0x10020+((port) * 0x1000))
#define CBM_ENQ_IRNEN_PORT(port)	(0xbe780000+0x10028+((port) * 0x1000))


#define MPE_DMA4_BASE	 0xbe400000

//DMA4
#define DMA4_CLC 		(MPE_DMA4_BASE + 0x0)
#define DMA4_ID 		(MPE_DMA4_BASE + 0x8)
#define DMA4_CTRL 		(MPE_DMA4_BASE + 0x10)
#define DMA4_CPOLL 		(MPE_DMA4_BASE + 0x14)
#define DMA4_CS 		(MPE_DMA4_BASE + 0x18)
#define DMA4_CCTRL 		(MPE_DMA4_BASE + 0x1c)
#define DMA4_CDBA 		(MPE_DMA4_BASE + 0x20)
#define DMA4_CDLEN 		(MPE_DMA4_BASE + 0x24)
#define DMA4_CIS 		(MPE_DMA4_BASE + 0x28)
#define DMA4_CIE 		(MPE_DMA4_BASE + 0x2c)
#define DMA4_PS 		(MPE_DMA4_BASE + 0x40)
#define DMA4_PCTRL 		(MPE_DMA4_BASE + 0x44)


#define DMA4_IRNEN 		(MPE_DMA4_BASE + 0xF4)
#define DMA4_IRNCR 		(MPE_DMA4_BASE + 0xF8)
#define DMA4_IRNICR 	(MPE_DMA4_BASE + 0xFC)


#if 0
enum tc_cur_state_info {
    UNKNOWN=0,
    TM_YD_WAIT,
    TM_YD_WKUP,
    WK_YD_WAIT,
    WK_YD_WKUP,
    MCPY_YD_WAIT,
    MCPY_YD_WKUP,
    MPECTRL_ENQ_YD_WAIT,
    MPECTRL_ENQ_YD_WKUP,
    MPECTRL_DEQ_YD_WAIT,
    MPECTRL_DEQ_YD_WKUP,
    CBM_ENQ_YD_WAIT,
    CBM_ENQ_YD_WKUP,
    CBM_DEQ_YD_WAIT,
    CBM_DEQ_YD_WKUP,
    SEM_DISP_Q_WAIT,
    SEM_FREELIST_WAIT,
    SEM_CBMALLOC_WAIT,
    SEM_DISP_Q_CNT_WAIT,
    SEM_DISP_Q_WKUP,
    SEM_FREELIST_WKUP,
    SEM_CBMALLOC_WKUP,
    SEM_DISP_Q_CNT_WKUP,
};
#endif

#ifdef CONFIG_BIG_ENDIAN

struct dispatch_q_reg1_itc	{    
    uint32_t reorder_qid :8;
    uint32_t res         :8;
    uint32_t desc_idx    :16;    
}__attribute__ ((packed));


/* Response ITC */
// Response register 0 ITC
struct mperesp_reg0_itc	{
    uint32_t vld:1;			// Determines whether the contents are valid or not, HW Enqueue starts after setting this bit
    uint32_t na:2;	
    uint32_t fdf:24;
    uint32_t na1:5;			// Target QID in the queuing control
}__attribute__ ((packed));


// CBM LS Status register
struct cbm_ls_status	{
    uint32_t cnt_val:16;			
    uint32_t res:2;
    uint32_t queue_empty:1;
    uint32_t queue_full:1;
    uint32_t res1:1;
    uint32_t queue_len:4;	
    uint32_t res2:7;
};


#else

struct dispatch_q_reg1_itc	{    
    uint32_t desc_idx    :16;    
    uint32_t res         :8;	
    uint32_t reorder_qid :8;
}__attribute__ ((packed));


/* Response ITC */
// Response register 0 ITC
struct mperesp_reg0_itc	{
    uint32_t na1:5;			// Target QID in the queuing control
    uint32_t fdf:24;    
    uint32_t na:2;	
    uint32_t vld:1;			// Determines whether the contents are valid or not, HW Enqueue starts after setting this bit
}__attribute__ ((packed));

// CBM LS Status register
struct cbm_ls_status	{
    uint32_t res2:7;
    uint32_t queue_len:4;	
    uint32_t res1:1;
    uint32_t queue_full:1;
    uint32_t queue_empty:1;
    uint32_t res:2;
    uint32_t cnt_val:16;			
};


#endif

// Response register 1 ITC
struct mperesp_reg1_itc	{
    union {
        struct dispatch_q_reg1_itc dispatch;   
        uint32_t fdf;
    };
}__attribute__ ((packed));

struct mperesp_itc {
    struct mperesp_reg0_itc reg0;
    struct mperesp_reg1_itc reg1;
}__attribute__ ((packed));


enum _valid
{
    MPE_FW=0,
    MPE_HW
};  

#endif
