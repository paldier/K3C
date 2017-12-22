#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>

#include "dsl_dfe_ar10_loopback.h"

/***********************************************************************/
#define BSP_REG32(addr)		   *((volatile u32 *)(addr))
#define BSP_REG16(addr)        *((volatile u16 *)(addr))
#define BSP_REG8(addr)         *((volatile u8  *)(addr))
#define REG32(addr) 		   *((volatile u32 *)(addr))

#define asc_puts                printk

/***********************************************************************/

/***********************************************
 *
 * Memory Space Description for ADSL DFE
 *
 *
 **********************************************/

#ifdef CACHE
#define DFE_BASE_ADDR         0x9E116000
#else
#define DFE_BASE_ADDR         0xBE116000   // This is for the MEI Interface
#endif

#define mei_dataxfr_c         (DFE_BASE_ADDR + 0x00000000)
#define mei_version_c         (DFE_BASE_ADDR + 0x00000004)
#define mei_arc_gp_c          (DFE_BASE_ADDR + 0x00000008)
#define mei_dataxfr_stat_c    (DFE_BASE_ADDR + 0x0000000C)
#define mei_xfraddr_c         (DFE_BASE_ADDR + 0x00000010)
#define mei_max_wait_c        (DFE_BASE_ADDR + 0x00000014)
#define mei_2_arcint_c        (DFE_BASE_ADDR + 0x00000018)
#define mei_fr_arcint_c       (DFE_BASE_ADDR + 0x0000001C)
#define mei_fr_arcimsk_c      (DFE_BASE_ADDR + 0x00000020)
#define mei_dbg_waddr_c       (DFE_BASE_ADDR + 0x00000024)
#define mei_dbg_raddr_c       (DFE_BASE_ADDR + 0x00000028)
#define mei_dbg_data_c        (DFE_BASE_ADDR + 0x0000002C)
#define mei_dbg_deco_c        (DFE_BASE_ADDR + 0x00000030)
#define mei_config_c          (DFE_BASE_ADDR + 0x00000034)
#define mei_rst_ctrl_c        (DFE_BASE_ADDR + 0x00000038)
#define mei_dbg_master_c      (DFE_BASE_ADDR + 0x0000003C)
#define mei_clk_ctrl_c        (DFE_BASE_ADDR + 0x00000040)
#define mei_bist_ctrl_c       (DFE_BASE_ADDR + 0x00000044)
#define mei_bist_stat_c       (DFE_BASE_ADDR + 0x00000048)
#define mei_xdata_base_sh_c   (DFE_BASE_ADDR + 0x0000004C)
#define mei_xdata_base_c      (DFE_BASE_ADDR + 0x00000050)
#define mei_xmem_bar0_c       (DFE_BASE_ADDR + 0x00000054)
#define mei_xmem_bar1_c       (DFE_BASE_ADDR + 0x00000058)
#define mei_xmem_bar2_c       (DFE_BASE_ADDR + 0x0000005C)
#define mei_xmem_bar3_c       (DFE_BASE_ADDR + 0x00000060)
#define mei_xmem_bar4_c       (DFE_BASE_ADDR + 0x00000064)
#define mei_xmem_bar5_c       (DFE_BASE_ADDR + 0x00000068)
#define mei_xmem_bar6_c       (DFE_BASE_ADDR + 0x0000006C)
#define mei_xmem_bar7_c       (DFE_BASE_ADDR + 0x00000070)
#define mei_xmem_bar8_c       (DFE_BASE_ADDR + 0x00000074)
#define mei_xmem_bar9_c       (DFE_BASE_ADDR + 0x00000078)
#define mei_xmem_bar10_c      (DFE_BASE_ADDR + 0x0000007C)
#define mei_xmem_bar11_c      (DFE_BASE_ADDR + 0x00000080)
#define mei_xmem_bar12_c      (DFE_BASE_ADDR + 0x00000084)
#define mei_xmem_bar13_c      (DFE_BASE_ADDR + 0x00000088)
#define mei_xmem_bar14_c      (DFE_BASE_ADDR + 0x0000008C)
#define mei_xmem_bar15_c      (DFE_BASE_ADDR + 0x00000090)
#define mei_xmem_bar16_c      (DFE_BASE_ADDR + 0x00000094)


//PPE Address Define
#define PPE_BASE_ADDR			(0xA0000000 | 0x1E180000)
#define CDM_CODE_MEMORY(x)		((volatile u32*)(PPE_BASE_ADDR + ((x + 0x001000) << 2)))
#define CDM_DATA_MEMORY(x)		((volatile u32*)(PPE_BASE_ADDR + ((x + 0x005000) << 2)))
#define SB_MEMORY(i)			((volatile u32*)(PPE_BASE_ADDR + ((i + 0x006000) << 2)))
#define MACRO_PPE_REG_ADDR(x)	((volatile u32*)(PPE_BASE_ADDR + ((x + 0x004000) << 2)))
#define CDM_CFG					MACRO_PPE_REG_ADDR(0x0100)
#define SFSM_STATE0				MACRO_PPE_REG_ADDR(0x0410)
#define SFSM_STATE1				MACRO_PPE_REG_ADDR(0x0411)
#define PPE32_DMRX_DBA			MACRO_PPE_REG_ADDR(0x0612)
#define PPE32_DMRX_CBA			MACRO_PPE_REG_ADDR(0x0613)
#define PPE32_DMRX_CFG			MACRO_PPE_REG_ADDR(0x0614)
#define PPE32_DMRX_PGCNT		MACRO_PPE_REG_ADDR(0x0615)
#define PPE32_DMRX_PKTCNT		MACRO_PPE_REG_ADDR(0x0616)
#define PPE32_DSRX_DB			MACRO_PPE_REG_ADDR(0x0710)
#define PPE32_DSRX_CB			MACRO_PPE_REG_ADDR(0x0711)
#define PPE32_DSRX_CFG			MACRO_PPE_REG_ADDR(0x0712)
#define PPE32_DSRX_PGCNT		MACRO_PPE_REG_ADDR(0x0713)

#define PP32_FREEZE				((volatile u32*)(PPE_BASE_ADDR))
#define PPE_CONF				((volatile u32*)((0xA0000000 | 0x1F203000)+ 0x002C))

/************************************************************************
 *  Static variables
 ************************************************************************/

u32 mei = 0x1;
u32 jtag = 0x0;
u32 dmp_core_access = 0x2;

// Register Definitions
#define aux_access 0x0
#define dmp_access 0x1 // or 0x2
#define core_access 0x3

//* Registers in the ARC address space */
//* ARC auxiliary space definitions */
#define AUX_STATUS 0x0000
#define AUX_SEMAPHORE 0x0001
#define AUX_LP_START 0x0002
#define AUX_LP_END 0x0003
#define AUX_IDENTITY 0x0004
#define AUX_DEBUG 0x0005


//************************** Constants *************************/
//* ARC address spaces, visible to the MEI using the debug port.  */
//* ARC load/store space definitions */
#define IRAM0_BASE 0x00000
#define IRAM0_SIZE (16*1024)
#define IRAM1_BASE 0x04000
#define IRAM1_SIZE (22*1024)
#define IRAM2_BASE 0x08000    // to be removed
#define IRAM2_SIZE (8*1024)   // to be removed
//#define BRAM_BASE 0x40000   //VR9 
//#define BRAM_SIZE (152*1024)//VR9
#define BRAM_BASE 0x8000    
#define BRAM_SIZE (12*1024)
#define IIBRAM2_BASE 0x66000
#define IIBRAM2_SIZE (16*1024)

/* Start and end addresses of INBOX and OUTBOX */
#define IMBOX_BASE (BRAM_BASE + BRAM_SIZE - 0x40)
#define IMBOX_END  (BRAM_BASE + BRAM_SIZE -0x1)
#define OMBOX_BASE (IMBOX_BASE - 0x40)
#define OMBOX_END  (IMBOX_BASE - 0x1)

/*********************************************
 * structure declaration
 *********************************************/
#if defined(__KERNEL__)
    struct port_cell_info {
        unsigned int    port_num;
        unsigned int    tx_link_rate[2];
    };
#endif

/*********************************************
  * extern function declaration
  ********************************************/
extern int (*ifx_mei_atm_showtime_enter)(struct port_cell_info *, void *);
extern int (*ifx_mei_atm_showtime_exit)(void);

/************************************************************************
 *  Static function prototypes
 ************************************************************************/

void mei_master(u32 mode){
  BSP_REG32(mei_dbg_master_c) = mode; // MEI as the master
};

void poll_status(void){
  while( (REG32(mei_fr_arcint_c) & 0x20) != 0x20 ){};
  REG32(mei_fr_arcint_c) = 0x20;
};

void mei_aux_acc(void){
  BSP_REG32(mei_dbg_deco_c) = aux_access;
};

void mei_core_acc(void){
  BSP_REG32(mei_dbg_deco_c) = core_access;
};

void mei_dmp_acc(void){
  BSP_REG32(mei_dbg_deco_c) = dmp_access;
};

void mei_dmp_acc_core(void){
  BSP_REG32(mei_dbg_deco_c) = dmp_core_access;
};

u32 ReadARCreg(u32 addr){
  u32 data;
  mei_master(mei);
  mei_aux_acc();
  BSP_REG32(mei_dbg_raddr_c) = addr;
  poll_status();
  data = BSP_REG32(mei_dbg_data_c);
  mei_master(jtag);
  return data;
};

void WriteARCreg(u32 addr, u32 data){
  mei_master(mei);
  mei_aux_acc();
  BSP_REG32(mei_dbg_waddr_c) = addr;
  BSP_REG32(mei_dbg_data_c) = data;
  poll_status();
  mei_master(jtag);
};

void dfe_interrupt_unmask(void){
    mei_master(mei);
    mei_aux_acc();
    // Interrupt Mask activation
    BSP_REG32(mei_fr_arcimsk_c) = 0x1;
};

void arc_mode_halt(void){
  u32 data_reg;   // storage space
  // Read ARC aux_debug register and or the result with 0x02 to halt the DSP
  mei_master(mei);
  mei_aux_acc();
  data_reg = (ReadARCreg(AUX_DEBUG) | 0x02);
  WriteARCreg(AUX_DEBUG, data_reg);
  mei_master(jtag);
};

void arc_prep(void){
  mei_aux_acc();
  arc_mode_halt(); // check if arc is in HALT mode
};

void arc_iram_download(u32 arc_code_length, u32 * start_address){
  u32 count;
  // Program the xfr addr to start of instruction mem
  BSP_REG32(mei_xfraddr_c) = IRAM0_BASE;
  for (count=0; count < arc_code_length; count++){
    BSP_REG32(mei_dataxfr_c) = * ( start_address + count );
  };
};

void arc_code_page_download(u32 arc_code_length, u32 * start_address){
    // Code Page Download
    mei_master(mei);
    arc_prep();
    arc_iram_download(arc_code_length, start_address);
    // arc_mode_start();
    mei_master(jtag);
};

void WriteARCmem(u32 addr, u32 data){
  mei_master(mei);
  mei_dmp_acc();
  BSP_REG32(mei_dbg_waddr_c) = addr;
  BSP_REG32(mei_dbg_data_c) = data;
  poll_status();
  mei_master(jtag);
};

void arc_mode_start(void){
  u32 data_reg;   // storage space
  // Read ARC aux_debug register and or the result with 0x02 to halt the DSP
  mei_master(mei);
  mei_aux_acc();
  data_reg = (ReadARCreg(AUX_STATUS) & 0xfdffffff);
  WriteARCreg(AUX_STATUS, data_reg);
};


void dsl_powerup(void) 
{
	
	//BC0 frame size = 50
	#define TXFB_START0 0
	#define TXFB_END0   49
	#define RXFB_START0 0
	#define RXFB_END0   49
    
	// BC1 frame size = 60
	#define TXFB_START1 50
	#define TXFB_END1   109
	#define RXFB_START1 256
	#define RXFB_END1   315	  
  
	#define TIMER_DELAY 1024
	
	dfe_interrupt_unmask();

	//Download ARC Code
	arc_code_page_download(0x43E, &arc_eth_dsl_lpbk[0]);

	//Set loopback parameters in mailbox
	mei_master(mei);
	mei_aux_acc();
	REG32(mei_xfraddr_c) = IMBOX_BASE;
	REG32(mei_dataxfr_c) = TIMER_DELAY;
	//Set FB Params
	REG32(mei_dataxfr_c) = TXFB_START0;
	REG32(mei_dataxfr_c) = TXFB_END0;
	REG32(mei_dataxfr_c) = TXFB_START1;
	REG32(mei_dataxfr_c) = TXFB_END1;
	REG32(mei_dataxfr_c) = RXFB_START0;
	REG32(mei_dataxfr_c) = RXFB_END0;
	REG32(mei_dataxfr_c) = RXFB_START1;
	REG32(mei_dataxfr_c) = RXFB_END1;  
  
	WriteARCmem(0x32010, 0xf);

	//Start ARC
	arc_mode_start();	

	return;
}

void wait_ppe_sync_state(u32 state)
{
    int i = 1000;

    if(state == 0) {
        asc_puts("Checking BC0 SYNC Condition\n");
        while (BSP_REG32(SFSM_STATE0) != 1 && i-- > 1) { };
        if ( i != 0 )
            asc_puts("BC0 achieved SYNC!!!\n");
        else
            asc_puts("BC0 did not achieve SYNC!!!\n");
    }
    if(state == 1) {
        asc_puts("Checking BC1 SYNC Condition\n");
        while (BSP_REG32(SFSM_STATE1) != 1 && i-- > 1) { };
        if ( i != 0 )
            asc_puts("BC1 achieved SYNC!!!\n");
        else
            asc_puts("BC1 did not achieve SYNC!!!\n");
    }
}

static void ifx_dsl_dfe_setup(void)
{
	int i = 0;
    //init dfe
    dsl_powerup();
    for(i =0; i < 10000; i ++);
    wait_ppe_sync_state(0);
    for(i =0; i < 10000; i ++);
    wait_ppe_sync_state(1);
}

static int __init ifx_dfe_loopback_init(void)
{
    struct port_cell_info info = {0};

    ifx_dsl_dfe_setup();

    if ( ifx_mei_atm_showtime_enter ) {
        info.port_num = 2;
        info.tx_link_rate[0] = 3000;
        ifx_mei_atm_showtime_enter(&info, NULL);
    }

    return 0;
}

/*
 *  Description:
 *    Release memory, free IRQ, and deregister device.
 *  Input:
 *    none
 *  Output:
 *   none
 */
static void __exit ifx_dfe_loopback_exit(void)
{
    if ( ifx_mei_atm_showtime_exit ) {
        ifx_mei_atm_showtime_exit();
    }
}


module_init(ifx_dfe_loopback_init);
module_exit(ifx_dfe_loopback_exit);
MODULE_LICENSE("GPL");


