#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>

#include "vrx218_dfe.h"

//////////////////////////////////////////////
//  here is simple wrapper for quick test   //
//////////////////////////////////////////////

#define dprint                                  printk
#define print                                   printk
#define REG32(x)                                (*(volatile unsigned int *)(x))

unsigned long dfe_pcie_base_addr = 0;
#undef PCIE_ADDR
#define PCIE_ADDR(__pcie_node, __addr)          KSEG1ADDR(dfe_pcie_base_addr + ((__addr) & 0x007FFFFF))
#undef MEI_OFFSET
#define MEI_OFFSET                              PCIE_ADDR(0, 0x1E116000)
#undef DSL_OFFSET
#define DSL_OFFSET                              PCIE_ADDR(0, 0x1E400000)


//////////////////////////////
//  start of chiptest code  //
//////////////////////////////

//---------------- Internal Functions -------------------//

static void vrx218_dfe_reset(u8 pcie_port) {
        dprint("REG32(PCIE_ADDR(%d, VRX318_RST_STAT)) -> 0x%08x\n", (int)pcie_port, (unsigned int)REG32(PCIE_ADDR(pcie_port, VRX218_RST_STAT)));

        dprint("[VRX318] RST_STAT: 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_RST_STAT)));
        dprint("[VRX318] RST_REQ : 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_RST_REQ)));
        print("Resetting VRX318 DFE ....");
        REG32(PCIE_ADDR(pcie_port, VRX218_RST_REQ)) = ((0x1 << 7) | (0x1 << 3));
        dprint("DONE!!\n");
        dprint("[VRX318] RST_STAT: 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_RST_STAT)));
        dprint("[VRX318] RST_REQ : 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_RST_REQ)));

        return;
}

#if 0
static void vrx218_dfe_init (u8 pcie_port)
{

	u32 read_data;

	dprint("[VRX218] PMU_PWDCR: 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_PMU_PWDCR)));
	dprint("[VRX218] PMU_SR   : 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_PMU_SR)));
	print("Enable DSL-DFE ....");
	read_data = REG32(PCIE_ADDR(pcie_port, VRX218_PMU_PWDCR));
	REG32(PCIE_ADDR(pcie_port, VRX218_PMU_PWDCR)) = read_data & 0xFFFFFDFF;
	dprint("DONE!!\n");
	dprint("[VRX218] PMU_PWDCR: 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_PMU_PWDCR)));
	dprint("[VRX218] PMU_SR   : 0x%08X\n", REG32(PCIE_ADDR(pcie_port, VRX218_PMU_SR)));

	return;
}

static void mei_master(u32 mode)
{
  	REG32(mei_dbg_master_c) = mode; // MEI as the master
};

static void arc_iram_download(u32 arc_code_length, u32 * start_address)
{
  	u32 count;
  	// Program the xfr addr to start of instruction mem
  	REG32(mei_xfraddr_c) = IRAM0_BASE;
  	for (count=0; count < arc_code_length; count++)
	{
    		REG32(mei_dataxfr_c) = * ( start_address + count );
  	};
};

static void arc_data_download(u32 arc_bulk_length, u32 * start_address, u32 offset)
{
  	u32 count;
  	// Program the xfr addr to start of bulk mem
  	REG32(mei_xfraddr_c) = BRAM_BASE + offset;
  	for (count=0; count < arc_bulk_length; count++)
	{
    		REG32(mei_dataxfr_c) = * ( start_address + count );
  	};
};

static void arc_code_data_download(u32 arc_code_length, u32 * start_address, u32 arc_data_length, u32 * start_data_address)
{
        u32 offset;
        offset = 0;
        // Code Page Download
        mei_master(S_MEI);
        me_dbg_port(0);
        // arc_prep();
        arc_iram_download(arc_code_length, start_address);
        arc_data_download(arc_data_length, start_data_address, offset);
        // arc_mode_start();
        mei_master(S_JTAG);
};
#endif

static void start_arc(u8 pcie_port) {

    u32  a;

    dprint("Starting ARC core\n");
	//---------------------------------------------
  	// Set MEI as Debug Master
 	//---------------------------------------------
  	me_dbg_master_on;
  	me_dbg_port(0);
    a = REG32(MEIAD(ME_DBG_DECODE));
    me_dbg_decode(ARC_AUX_REG_ADDR_SPACE);
    me_dbg_wr(ARC_STATUS32,0x0); // Release ARC from HALT
	me_dbg_decode(a);
    me_dbg_master_off;
    dprint("Arc started\n");

    return;
}

//void set_dfe_data_rate(u32 nBC_switches, u32 nBC0Bytes,  u32 nBC1Bytes, u32 nBC0ErrBytes,  u32 nBC1ErrBytes);
void set_dfe_data_rate(u8 pcie_port, u32 nBC_switches, u32 nBC0Bytes,  u32 nBC1Bytes, u32 numTimeSlots) {

	// number of BC switches for Tx to load into register ZT_R0
	me_dbg_wr(0x000542F4, nBC_switches);

	// number of BC switches for Rx to load into register ZR_R0
	me_dbg_wr(0x0005B94C, nBC_switches);

	// number of BC0 and BC1 bytes for Tx to load into register ZT_VBC_SIZE
	me_dbg_wr(0x00054308, (nBC1Bytes << 16) + nBC0Bytes);

	// number of BC0 and BC1 bytes for Rx to load into register ZR_VBC_SIZE
	me_dbg_wr(0x0005B960, (nBC1Bytes << 16) + nBC0Bytes);

	// number of BC0 and BC1 error bytes for Tx to load into register ZT_R12
	//me_dbg_wr(0x00054300, (nBC1ErrBytes << 16) + nBC0ErrBytes);
	me_dbg_wr(0x00054300, 0);

	// number of BC0 and BC1 error bytes for Rx to load into register ZR_R12
	//me_dbg_wr(0x0005B958, (nBC1ErrBytes << 16) + nBC0ErrBytes);
	me_dbg_wr(0x0005B958, 0);

	// kick of by writing to CRI registers
	//me_dbg_wr(0x0020c40c, 0x8007ffe1);
	me_dbg_wr(0x0020c40c, 0x8007ffe0 | numTimeSlots);
	me_dbg_wr(0x0020c49c, 0x00000078);

    return;
}

static void setup_zephyr(u8 pcie_port) {

//  	u32 reg_wr_rd_err=0x0;
  	u32 i, read_data;

    print("Setting Zephry via MEI: pcie_port = %d\n", (int)pcie_port);
    for (i=0;i<NUM_ME_VAR;i++) {
        switch(my_cfg_seq[i].type) {
            case ME_DBG_RD: read_data = REG32(PCIE_ADDR(pcie_port, my_cfg_seq[i].addr+0xbe116000));
                            break;
	  		case ME_DBG_WR: REG32(PCIE_ADDR(pcie_port, my_cfg_seq[i].addr+0xbe116000)) = my_cfg_seq[i].data;
                            break;
        }
    }

  	// Read back to make sure Zephry started
  	me_dbg_rd(0x00020c40c,read_data);
  	dprint("CRI_TSC_CTRL: 0x%08x\n",read_data);

  	me_dbg_rd(0x00020c4dc,read_data);
  	dprint("CRI_RXFFT_STALL_CTRL: 0x%08x\n",read_data);

  	me_dbg_rd(0x00020c4d8,read_data);
  	dprint("CRI_RXQT_STALL_CTRL: 0x%08x\n",read_data);

 	me_dbg_rd(0x00020c4d0,read_data);
  	dprint("CRI_RXPMS_CTRL: 0x%08x\n",read_data);

  	me_dbg_rd(0x00020c4b4,read_data);
  	dprint("CRI_TXPMS_CTRL: 0x%08x\n",read_data);

  	return;
}

void dfe_zephyr_lb_init(u8 pcie_port, u8 arc_mode) {

  	vrx218_dfe_reset(pcie_port);

  	if(arc_mode)
		start_arc(pcie_port);
  	else
		setup_zephyr(pcie_port);

    return;
}

#if 0
static void arc2meint_en()
{
        me_dbg_master_on;
        me_dbg_port(0);
        me_dbg_decode(ARC_LDST_ADDR_SPACE);
        REG32(MEIAD(ME_ARC2ME_MASK))=0x7;
        me_dbg_master_off;
}

u32 me_rd(u32 a)
{
        u32 rd, de;
        me_dbg_master_on;
        #if (ME_DEBUG_INFO)
                print("Setting the LDST Port to 0");
                print("\n");
        #endif
        me_dbg_port(0);
        de = REG32(MEIAD(ME_DBG_DECODE));
        #if (ME_DEBUG_INFO)
                print("Decoding the LDST address space");
                print("\n");
        #endif
        me_dbg_decode(ARC_LDST_ADDR_SPACE);
        #if (ME_DEBUG_INFO)
                print("Waiting for ARC2ME status");
                print("\n");
        #endif
        me_dbg_rd(a,rd);
        #if (ME_DEBUG_INFO)
                print("Got the ARC2ME status set, transaction completed");
                print("\n");
        #endif
        me_dbg_decode(de);
        me_dbg_master_off;
        return rd;
}

static void arc_putc()
{
        int j=0;
        char *s = "\0";
        do
        {
                *s =  (char) (me_rd(ilv_dmetric_base +((j++)<<2))) & 0xff;
                asc_putc(*s);
        } while (*s++ != 0);
        REG32(MEIAD(ME_ARC2ME_STAT)) =  0x7;
        ack_bsp_irq(32+23);
}

u32 grand(u32 *p) {
	*p = 1103515245*(*p) + 12345;
	return *p;
}

// ------------------ Testcases ------------------- //

static void access_vrx218_fpi_ldst_dfe_mem(u8 pcie_port, u8 mode) {

        print("\t*****************************************\n");
        print("\tAR10 MIPS FPI LDST access to DFE memories\n");
        print("\t*****************************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	// Program RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x2;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = (0x1 << 1);

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
    	unsigned int i, j, flag;

	if (mode == 0) {
		print("\t *** SINGLE ACCESS write, followed by read ***\n");
	} else {
		print("\t *** BURST ACCESS write, followed by read ***\n");
	}

	flag=1;
	// IRAM
	print("\n");
	print("*** IRAM : DSP Resident Program Memory ***\n");
	dprint("*** IRAM : 0x000000 - 0x0097FF 	      ***\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
		if (mode == 0) {
		// Single access
        		for (j=0; j<0x25FF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_iram_base+j*4)) = pattern[i];
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_iram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		} else {
		// Burst access
			for (j=0; j<0x25FF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_iram_base+j*4)) = pattern[i];
			}
			for (j=0; j<0x25FF; j++) {
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_iram_base+j*4));
				if (read_val != pattern[i]) {
					print ("Write-Read access 0x%08X Failed!!\n", (j));
					print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
					flag=0;
					break;
				}
				if ((j & 0x7FF) == 0x7FF)
					dprint (".");
			}
		}
                print("\n");
    	}

	// DCCM
	print("\n");
	print("*** DCCM : DSP Local Load Store Memory ***\n");
	dprint("*** DCCM : 0x010000 - 0x013FFF         ***\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
		if (mode==0) {
		// Single access
        		for (j=0; j<0xFFF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_dccm_base+j*4)) = pattern[i];
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_dccm_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		} else {
		// Burst access
			for (j=0; j<0xFFF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_dccm_base+j*4)) = pattern[i];
			}
			for (j=0; j<0xFFF; j++) {
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_dccm_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		}
                print("\n");
    	}

	// XRAM
	print("\n");
	print("*** XRAM : DSP X component of XY Memory ***\n");
	dprint("*** XRAM : 0x030000 - 0x030FFF 	       ***\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
		if (mode==0) {
		// Single access
        		for (j=0; j<0x3FF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_xram_base+j*4)) = pattern[i];
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_xram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
			}
		} else {
		// Burst access
			for (j=0; j<0x3FF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_xram_base+j*4)) = pattern[i];
			}
			for (j=0; j<0x3FF; j++) {
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_xram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
			}
        	}
                print("\n");
    	}

	// YRAM
	print("\n");
	print("*** YRAM : DSP Y component of XY Memory ***\n");
	dprint("*** YRAM : 0x034000 - 0x034FFF 	       ***\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
		if (mode==0) {
		// Single access
        		for (j=0; j<0x3FF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_yram_base+j*4)) = pattern[i];
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_yram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		} else {
		// Burst access
			for (j=0; j<0x3FF; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_yram_base+j*4)) = pattern[i];
			}
			for (j=0; j<0x3FF; j++) {
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_yram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		}
                print("\n");
    	}

	// IIBRAM
	print("\n");
	print("*** IIBRAM : DSP Interleave/Instruction/Bulk Data Memory ***\n");
	dprint("*** IIBRAM : 0x040000 - 0x071FFF  			***\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
		if (mode==0) {
		// Single access
        		for (j=0; j<0xC800; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_iibram_base+j*4)) = pattern[i];
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_iibram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		} else {
		// Burst access
			for (j=0; j<0xC800; j++) {
				// Write to DFE memories
				REG32(FPIAD(dsp_iibram_base+j*4)) = pattern[i];
			}
			for (j=0; j<0xC800; j++) {
				// Read from DFE memories
				read_val = REG32(FPIAD(dsp_iibram_base+j*4));
            			if (read_val != pattern[i]) {
                			print ("Write-Read access 0x%08X Failed!!\n", (j));
                			print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                			flag=0;
                                	break;
            			}
                        	if ((j & 0x7FF) == 0x7FF)
                                	dprint (".");
        		}
		}
                print("\n");
    	}

	// Memories not tested
	// DSP Load Store Registers (LDSTREG)
	// DSP External SDRAM (SDRAM)
   	// All memories are tested

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_bar015_iibram_ddr_access(u8 pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI DBG LDST BAR0-15 - IIBRAM <-> DDR\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Config BAR0 address to AR10 SDRAM
	dprint("Configure BAR0-15 register\n");
	REG32(MEIAD(ME_XMEM_BAR0)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR1)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR2)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR3)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR4)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR5)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR6)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR7)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR8)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR9)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR10)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR11)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR12)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR13)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR14)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR15)) = 0x20000000;

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
	int LDST_MEM_OFFSET;
    	unsigned int i, j, flag;
	int total_acc;

	LDST_MEM_OFFSET = 0x80000;
	flag=1;
	total_acc = 0x4000;
	// Testing 0x4000 locations
	print("\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
        	for (j=0; j<total_acc; j++) {
			// Write to AR10 DDR
			//print ("write..");
			me_dbg_wr(LDST_MEM_OFFSET+j*4,(pattern[i]+j));
			// Read from AR10 DDR
			//print ("read..");
			me_dbg_rd(LDST_MEM_OFFSET+j*4,read_val);
			//print ("compare..");
            		if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
            		}
                        if ((j & 0x1FF) == 0x1FF)
                                dprint (".");
        	}
                print("\n");
    	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_bar16_iibram_ddr_access(pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI DBG LDST BAR16 - IIBRAM <-> DDR\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Config BAR16 address to AR10 SDRAM
	dprint("Configure BAR16 register\n");
	REG32(MEIAD(ME_XMEM_BAR0+0x40))= (0x20000000 + 0x200000);

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
	int LDST_MEM_OFFSET;
    	unsigned int i, j, flag;
	int total_acc;

	//  8_0000 => to offset 512k
	// 10_0000 => to select XMEM_BAR16 (A20 = '1')
	LDST_MEM_OFFSET = 0x180000;
	flag=1;
	total_acc = 0xC800;
	// Testing 0xC800 locations
	print("\n");
    	for (i=0; i<6; i++) {
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
        	for (j=0; j<total_acc; j++) {
			// Write to AR10 DDR
			//print ("write..");
			me_dbg_wr(LDST_MEM_OFFSET+j*4,(pattern[i]+j));
			// Read from AR10 DDR
			//print ("read..");
			me_dbg_rd(LDST_MEM_OFFSET+j*4,read_val);
			//print ("compare..");
            		if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
            		}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
        	}
                print("\n");
    	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_xdma_bar015_iibram_ddr_access(pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI XDMA BAR0-15 - IIBRAM <-> DDR\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	print("Start the test...\n");
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Config BAR0 address to AR10 SDRAM
	dprint("Configure BAR0-15 register\n");
	REG32(MEIAD(ME_XMEM_BAR0)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR1)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR2)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR3)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR4)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR5)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR6)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR7)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR8)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR9)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR10)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR11)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR12)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR13)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR14)) = 0x20000000;
	REG32(MEIAD(ME_XMEM_BAR15)) = 0x20000000;

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
    	unsigned int i, j, flag;
	int total_dword, total_byte;

	flag=1;
	total_dword = 0x4000;
	total_byte = ((total_dword-1)*4) + 0x2;
	// Testing 0x4000 locations
	print("\n");
	print("XDMA transfer from IIBRAM to DDR\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to IIBRAM
			me_dbg_wr(dsp_iibram_base+(j*4),(pattern[i]+j));
		}
		// Start XDMA
		dprint("Start XDMA transfer from IIBRAM to DDR\n");
		me_dbg_wr(D_XDMA0_SA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_SA_E,dsp_iibram_base+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,0);
		me_dbg_wr(D_XDMA0_CTRL,3);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from DDR
			read_val = REG32(0xA0000000 + j*4);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

	// Testing 0x4000 locations
	print("\n");
	print("XDMA transfer from DDR to IIBRAM\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to DDR
			REG32(0xA0000000 + j*4) = pattern[i]+j;
		}
		// Start XDMA
		dprint("Start XDMA transfer from DDR to IIBRAM\n");
		me_dbg_wr(D_XDMA0_SA_S,0);
		me_dbg_wr(D_XDMA0_SA_E,total_byte);
		me_dbg_wr(D_XDMA0_DA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_CTRL,1);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from IIBRAM
			me_dbg_rd(dsp_iibram_base+j*4,read_val);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_xdma_bar16_iibram_ddr_access(pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI XDMA BAR16 - IIBRAM <-> DDR\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	print("Start the test...\n");
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Config BAR16 address to AR10 SDRAM
	dprint("Configure BAR16 register\n");
	REG32(MEIAD(ME_XMEM_BAR0+0x40))= (0x20000000 + 0x200000);

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
    	unsigned int i, j, flag;
	int total_dword, total_byte;

	flag=1;
	total_dword = 0xC800;
	total_byte = ((total_dword-1)*4) + 0x2;
	// Testing 0xC800 locations
	print("\n");
	print("XDMA transfer from IIBRAM to DDR\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to IIBRAM
			me_dbg_wr(dsp_iibram_base+(j*4),(pattern[i]+j));
		}
		// Start XDMA
		dprint("Start XDMA transfer from IIBRAM to DDR\n");
		me_dbg_wr(D_XDMA0_SA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_SA_E,dsp_iibram_base+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,0x00100000);
		me_dbg_wr(D_XDMA0_CTRL,3);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from DDR offset 0x20000
			read_val = REG32(0xA0200000 + j*4);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

	// Testing all locations
	print("\n");
	print("XDMA transfer from DDR to IIBRAM\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to DDR offset 0x20000
			REG32(0xA0200000 + j*4) = pattern[i]+j;
		}
		// Start XDMA
		dprint("Start XDMA transfer from DDR to IIBRAM\n");
		me_dbg_wr(D_XDMA0_SA_S,0x00100000);
		me_dbg_wr(D_XDMA0_SA_E,0x00100000+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_CTRL,1);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from IIBRAM
			me_dbg_rd(dsp_iibram_base+j*4,read_val);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_xdma_xdatabase_iibram_ddr_access(pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI XDMA XDATA BASE - IIBRAM <-> DDR\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	print("Start the test...\n");
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Config XDATA BASE address to AR10 SDRAM
	dprint("Configure XDATA_BASE register\n");
	REG32(MEIAD(ME_XDATA_BASE_SH))= 0x20000000;
  	REG32(MEIAD(ME_XDATA_BASE))= 0x20000000;

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
    	unsigned int i, j, flag;
	int total_dword, total_byte;

	flag=1;
	total_dword = 0xC800;
	total_byte = ((total_dword-1)*4) + 0x2;
	// Testing all locations
	print("\n");
	print("XDMA transfer from IIBRAM to DDR\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to IIBRAM
			me_dbg_wr(dsp_iibram_base+(j*4),(pattern[i]+j));
		}
		// Start XDMA
		dprint("Start XDMA transfer from IIBRAM to DDR\n");
		me_dbg_wr(D_XDMA0_SA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_SA_E,dsp_iibram_base+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,0);
		me_dbg_wr(D_XDMA0_CTRL,7);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from DDR
			read_val = REG32(0xA0000000 + j*4);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

	// Testing all locations
	print("\n");
	print("XDMA transfer from DDR to IIBRAM\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to DDR
			REG32(0xA0000000 + j*4) = pattern[i]+j;
		}
		// Start XDMA
		dprint("Start XDMA transfer from DDR to IIBRAM\n");
		me_dbg_wr(D_XDMA0_SA_S,0);
		me_dbg_wr(D_XDMA0_SA_E,total_byte);
		me_dbg_wr(D_XDMA0_DA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_CTRL,5);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from IIBRAM
			me_dbg_rd(dsp_iibram_base+j*4,read_val);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_xdma_bar015_iibram_pdbram_access(pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI XDMA BAR0-15 - IIBRAM <-> PDBRAM\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	print("Start the test...\n");
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Config BAR0 address to AR10 SDRAM
	dprint("Configure BAR0-15 register\n");
	REG32(MEIAD(ME_XMEM_BAR0)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR1)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR2)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR3)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR4)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR5)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR6)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR7)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR8)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR9)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR10)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR11)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR12)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR13)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR14)) = 0x1E080000;
	REG32(MEIAD(ME_XMEM_BAR15)) = 0x1E080000;

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
    	unsigned int i, j, flag;
	int total_dword, total_byte;

	flag=1;
	total_dword = 0x4000;
	total_byte = ((total_dword-1)*4) + 0x2;
	// Testing all locations
	print("\n");
	print("XDMA transfer from IIBRAM to PDBRAM\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to IIBRAM
			me_dbg_wr(dsp_iibram_base+(j*4),(pattern[i]+j));
		}
		// Start XDMA
		dprint("Start XDMA transfer from IIBRAM to PDBRAM\n");
		me_dbg_wr(D_XDMA0_SA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_SA_E,dsp_iibram_base+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,0);
		me_dbg_wr(D_XDMA0_CTRL,3);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from PDBRAM
			read_val = REG32(0xBC080000 + j*4);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

	// Testing all locations
	print("\n");
	print("XDMA transfer from PDBRAM to IIBRAM\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to PDBRAM
			REG32(0xBC080000 + j*4) = pattern[i]+j;
		}
		// Start XDMA
		dprint("Start XDMA transfer from PDBRAM to IIBRAM\n");
		me_dbg_wr(D_XDMA0_SA_S,0);
		me_dbg_wr(D_XDMA0_SA_E,total_byte);
		me_dbg_wr(D_XDMA0_DA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_CTRL,1);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from IIBRAM
			me_dbg_rd(dsp_iibram_base+j*4,read_val);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_dbg_ldst_xdma_bar16_default_sb_access(pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE MEI XDMA BAR16 - IIBRAM <-> PPE SB\n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	print("Start the test...\n");
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Default BAR16 point to 1E20_0000 : PPE base address
	// PPE SB RAM0 offset => 0x8000*4 => 0x2_0000
	// For BAR16, bit 20 should be set to '1' => 0x0010_0000
	// Offset => 0x0012_0000
	dprint("No configuration of BAR16 register is required\n");

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	int read_val;
    	unsigned int i, j, flag;
	int total_dword, total_byte;

	flag=1;
	total_dword = 0x4000;
	total_byte = ((total_dword-1)*4) + 0x2;
	// Testing 0xC800 locations
	print("\n");
	print("XDMA transfer from IIBRAM to PPE SB\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to IIBRAM
			me_dbg_wr(dsp_iibram_base+(j*4),(pattern[i]+j));
		}
		// Start XDMA
		dprint("Start XDMA transfer from IIBRAM to PPE SB\n");
		me_dbg_wr(D_XDMA0_SA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_SA_E,dsp_iibram_base+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,0x00120000);
		me_dbg_wr(D_XDMA0_CTRL,3);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from PPE SB RAM0
			read_val = REG32(0xBC220000 + j*4);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

	// Testing all locations
	print("\n");
	print("XDMA transfer from PPE SB to IIBRAM\n");
	for (i=0; i<6; i++) {
		for (j=0; j<total_dword; j++) {
			// Write to PPE SB
			REG32(0xBC220000 + j*4) = pattern[i]+j;
		}
		// Start XDMA
		dprint("Start XDMA transfer from PPE SB to IIBRAM\n");
		me_dbg_wr(D_XDMA0_SA_S,0x00120000);
		me_dbg_wr(D_XDMA0_SA_E,0x00120000+total_byte);
		me_dbg_wr(D_XDMA0_DA_S,dsp_iibram_base);
		me_dbg_wr(D_XDMA0_CTRL,1);

		// Polling for status bit
		dprint("Polling for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		while ((read_val & 0x00000001) != 0x00000000)
		{
	        	me_dbg_rd(D_XDMA0_CTRL,read_val);
		};
		print("Testing with pattern 0x%08X ..", pattern[i]);
		for (j=0; j<total_dword; j++) {
			// Read from IIBRAM
			me_dbg_rd(dsp_iibram_base+j*4,read_val);
			if (read_val != (pattern[i]+j)) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern[i]+j), read_val);
                		flag=0;
                                break;
			}
                        if ((j & 0x7FF) == 0x7FF)
                                dprint (".");
		}
		print("\n");

	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_mei_dx_single_iibram_access(u8 pcie_port) {

        print("\t*****************************************\n");
        print("\tAR10 MIPS MEI Single access to IIBRAM \n");
        print("\t*****************************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	// Program RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x2;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = (0x1 << 1);

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
    	u32 wr_data[51200], read_val[51200];
    	unsigned int i, j, k, flag;
	int total_acc;

	flag=1;
	total_acc = 0xC800;
	// IIBRAM
	print("\n");
	print("*** IIBRAM : DSP Interleave/Instruction/Bulk Data Memory ***\n");
	dprint("*** IIBRAM : 0x040000 - 0x071FFF  			***\n");
    	for (i=0; i<6; i++) {
		for (j=0; j<total_acc; j++) {
			wr_data[j] = pattern[i] + j;
		}
        	print ("Testing with pattern 0x%08X ..", pattern[i]);
		dprint ("\nPlease wait, it takes a while ...\n");
		// Write to DFE memories
		//print ("Writing..");
		me_dx_wr(dsp_iibram_base,wr_data,total_acc);
		// Read from DFE memories
		//print ("Reading..");
		me_dx_rd(dsp_iibram_base,read_val,total_acc);
		//print ("Comparing..");
        	for (j=0; j<total_acc; j++) {
            		if (read_val[j] != wr_data[j]) {
                		print ("Write-Read access 0x%08X Failed!!\n", (j));
                		print ("Write Value: %08X\tRead Value: %08X\n", wr_data[j], read_val[j]);
                		flag=0;
                                break;
            		}
                        if ((j & 0x1FF) == 0x1FF)
                                dprint (".");
        	}
                print("\n");
    	}

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_ppe_dreg_access(u8 pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE access to PPE DREG \n");
        print("\t*****************************************\n");

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(1);
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Access to PPE DREG
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
 	unsigned int i, j, flag;
	int read_data, write_data, exp_data;
	flag=1;
	print("\n");
	print("Start testing...\n");
	for (i=0; i<NUM_ppe_dreg_var; i++) {
		// Checking reset value
		dprint("Reading reset value\n");
		me_dbg_rd(ppe_dreg_var[i].addr, read_data);
		exp_data = ppe_dreg_var[i].resval & ppe_dreg_var[i].rmsk;
		if (read_data != exp_data) {
			print("Reset value check 0x%08X Failed!!\n", ppe_dreg_var[i].addr);
			print("Read value: %08X\tExpected value: %08X\n", read_data, exp_data);
			flag=0;
			break;
		}
		// Checking with all different data pattern
		for (j=0; j<6; j++) {
			dprint("Testing with pattern 0x%08X ..\n", pattern[j]);
			write_data = pattern[j] & ppe_dreg_var[i].wmsk;
			me_dbg_wr(ppe_dreg_var[i].addr, write_data);
			read_data = 0x0;
			me_dbg_rd(ppe_dreg_var[i].addr, read_data);
			exp_data = write_data & ppe_dreg_var[i].rmsk;
			if (read_data != exp_data) {
				print("Read write check 0x%08X Failed!!\n", ppe_dreg_var[i].addr);
				print("Read value: %08X\tExpected value: %08X\n", read_data, exp_data);
				flag=0;
				break;
			}
		}
		// Write with reset value
		dprint("Write with reset value\n");
		write_data = ppe_dreg_var[i].resval;
		me_dbg_wr(ppe_dreg_var[i].addr, write_data);
	}
	print("\n");

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;

}

static void dfe_mei_mgmt_registers(u8 pcie_port) {

        print("\t******************************************\n");
        print("\tDFE MEI Read Write check to MGMT registers \n");
        print("\t******************************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	//Predefined MGMT registers to be accessed
	int reg_addr[] = {ME_VERSION, ME_ARC2ME_MASK, ME_ME2ARC_INT,
			  ME_CLK_CTRL, ME_RST_CTRL, ME_CHIP_CONFIG,
			  ME_DBG_MASTER, ME_DBG_DECODE, ME_DBG_PORT_SEL, ME_DBG_RD_AD,
			  ME_DBG_WR_AD, ME_DX_PORT_SEL, ME_DX_AD,
			  ME_DX_MWS, ME_ARC_GP_STAT,
			  ME_XDATA_BASE_SH, ME_XMEM_BAR0, ME_XMEM_BAR1,
			  ME_XMEM_BAR2, ME_XMEM_BAR3, ME_XMEM_BAR4, ME_XMEM_BAR5,
			  ME_XMEM_BAR6, ME_XMEM_BAR7, ME_XMEM_BAR8, ME_XMEM_BAR9,
			  ME_XMEM_BAR10, ME_XMEM_BAR11, ME_XMEM_BAR12, ME_XMEM_BAR13,
			  ME_XMEM_BAR14, ME_XMEM_BAR15, ME_XMEM_BAR16, ME_XMEM_ARB_MISC};
	int reg_default[] = {ME_VERSION_DEFAULT,
			     ME_ARC2ME_MASK_DEFAULT, ME_ME2ARC_INT_DEFAULT,
			     ME_CLK_CTRL_DEFAULT,
			     ME_RST_CTRL_DEFAULT, ME_CHIP_CONFIG_DEFAULT,
			     ME_DBG_MASTER_DEFAULT, ME_DBG_DECODE_DEFAULT,
			     ME_DBG_PORT_SEL_DEFAULT, ME_DBG_RD_AD_DEFAULT,
			     ME_DBG_WR_AD_DEFAULT,
			     ME_DX_PORT_SEL_DEFAULT, ME_DX_AD_DEFAULT,
			     ME_DX_MWS_DEFAULT, ME_ARC_GP_STAT_DEFAULT,
			     ME_XDATA_BASE_SH_DEFAULT,
			     ME_XMEM_BAR0_DEFAULT, ME_XMEM_BAR1_DEFAULT,
			     ME_XMEM_BAR2_DEFAULT, ME_XMEM_BAR3_DEFAULT,
		 	     ME_XMEM_BAR4_DEFAULT, ME_XMEM_BAR5_DEFAULT,
			     ME_XMEM_BAR6_DEFAULT, ME_XMEM_BAR7_DEFAULT,
			     ME_XMEM_BAR8_DEFAULT, ME_XMEM_BAR9_DEFAULT,
			     ME_XMEM_BAR10_DEFAULT, ME_XMEM_BAR11_DEFAULT,
			     ME_XMEM_BAR12_DEFAULT, ME_XMEM_BAR13_DEFAULT,
			     ME_XMEM_BAR14_DEFAULT, ME_XMEM_BAR15_DEFAULT,
			     ME_XMEM_BAR16_DEFAULT, ME_XMEM_ARB_MISC_DEFAULT};
	int reg_wmask[] = {ME_VERSION_WMASK,
			   ME_ARC2ME_MASK_WMASK, ME_ME2ARC_INT_WMASK,
			   ME_CLK_CTRL_WMASK,
			   ME_RST_CTRL_WMASK, ME_CHIP_CONFIG_WMASK,
			   ME_DBG_MASTER_WMASK, ME_DBG_DECODE_WMASK,
			   ME_DBG_PORT_SEL_WMASK, ME_DBG_RD_AD_WMASK,
			   ME_DBG_WR_AD_WMASK,
			   ME_DX_PORT_SEL_WMASK, ME_DX_AD_WMASK,
			   ME_DX_MWS_WMASK, ME_ARC_GP_STAT_WMASK,
			   ME_XDATA_BASE_SH_WMASK,
			   ME_XMEM_BAR0_WMASK, ME_XMEM_BAR1_WMASK,
			   ME_XMEM_BAR2_WMASK, ME_XMEM_BAR3_WMASK,
		 	   ME_XMEM_BAR4_WMASK, ME_XMEM_BAR5_WMASK,
			   ME_XMEM_BAR6_WMASK, ME_XMEM_BAR7_WMASK,
			   ME_XMEM_BAR8_WMASK, ME_XMEM_BAR9_WMASK,
			   ME_XMEM_BAR10_WMASK, ME_XMEM_BAR11_WMASK,
			   ME_XMEM_BAR12_WMASK, ME_XMEM_BAR13_WMASK,
			   ME_XMEM_BAR14_WMASK, ME_XMEM_BAR15_WMASK,
			   ME_XMEM_BAR16_WMASK, ME_XMEM_ARB_MISC_WMASK};
	int reg_rmask[] = {ME_VERSION_MASK,
			   ME_ARC2ME_MASK_MASK, ME_ME2ARC_INT_MASK,
			   ME_CLK_CTRL_MASK,
			   ME_RST_CTRL_MASK, ME_CHIP_CONFIG_MASK,
			   ME_DBG_MASTER_MASK, ME_DBG_DECODE_MASK,
			   ME_DBG_PORT_SEL_MASK, ME_DBG_RD_AD_MASK,
			   ME_DBG_WR_AD_MASK,
			   ME_DX_PORT_SEL_MASK, ME_DX_AD_MASK,
			   ME_DX_MWS_MASK, ME_ARC_GP_STAT_MASK,
			   ME_XDATA_BASE_SH_MASK,
			   ME_XMEM_BAR0_MASK, ME_XMEM_BAR1_MASK,
			   ME_XMEM_BAR2_MASK, ME_XMEM_BAR3_MASK,
		 	   ME_XMEM_BAR4_MASK, ME_XMEM_BAR5_MASK,
			   ME_XMEM_BAR6_MASK, ME_XMEM_BAR7_MASK,
			   ME_XMEM_BAR8_MASK, ME_XMEM_BAR9_MASK,
			   ME_XMEM_BAR10_MASK, ME_XMEM_BAR11_MASK,
			   ME_XMEM_BAR12_MASK, ME_XMEM_BAR13_MASK,
			   ME_XMEM_BAR14_MASK, ME_XMEM_BAR15_MASK,
			   ME_XMEM_BAR16_MASK, ME_XMEM_ARB_MISC_MASK};
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
	unsigned int i, j, flag;
 	int addr, wdata, rdata, wmask, rmask, exp_data, defdata;
	int no_of_regs;
	flag=1;
	print("\n");
	print("Exclude from the list: \n");
	print("\tME_ARC2ME_STAT, ME_ME2ARC_STAT\n");
	print("\tME_DX_DATA, ME_DX_STAT\n");
	print("\tME_DBG_DATA, ME_XDATA_BASE\n");
	print("Start testing...\n");
	print("Checking default value, read-write with different patterns\n");
	no_of_regs = 34;
	for (i=0; i<no_of_regs; i++) {
		// Checking reset value
		dprint("Reading reset value\n");
		addr = reg_addr[i];
		rmask = reg_rmask[i];
		exp_data = reg_default[i];
		rdata = REG32(MEIAD(addr));
		if (rdata != exp_data) {
			print("Default value check 0x%08X Failed!!\n", addr);
			print("Read value: %08X\tExpected value: %08X\n", rdata, exp_data);
			flag=0;
			break;
		}
	}
	for (i=0; i<no_of_regs; i++) {
		// Checking will all different data patterns
		for (j=0; j<6; j++) {
			dprint("Testing with pattern 0x%08X ..\n", pattern[j]);
			addr = reg_addr[i];
			wmask = reg_wmask[i];
			rmask = reg_rmask[i];
			defdata = reg_default[i];
			wdata = pattern[j] & wmask;
			exp_data = (wdata & rmask) | (~wmask & defdata);
			REG32(MEIAD(addr)) = wdata;
			rdata = REG32(MEIAD(addr));
				if (rdata != exp_data) {
				print("Read write check 0x%08X Failed!!\n", addr);
				print("Write value: %08X\t\n", wdata);
				print("Read value: %08X\tExpected value: %08X\n", rdata, exp_data);
				flag=0;
				break;
			}
		}
	}
	for (i=0; i<no_of_regs; i++) {
		// Write back reset value
		dprint("Write back default value\n");
		addr = reg_addr[i];
		wdata = reg_default[i];
		REG32(MEIAD(addr)) = wdata;
	}
	print("\n");

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}
	return;

}

static void dfe_arc_fw_download_iibram(u8 pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE ARC FW download to IIBRAM \n");
        print("\t*****************************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	unsigned int i, j, flag;
	int read_data, exp_data;
	flag=1;

	print("\n");
	print("Start checking...\n");
	//Initialize DFE memories
	REG32(mei_xfraddr_c) = IRAM0_BASE;
	REG32(mei_dataxfr_c) = 0x0;
	REG32(mei_xfraddr_c) = BRAM_BASE;
	REG32(mei_dataxfr_c) = 0x0;

	//Checking DFE memories before ARC FW download
	//Code
	REG32(mei_xfraddr_c) = IRAM0_BASE;
 	read_data = REG32(mei_dataxfr_c);
	dprint("DFE Code memory value 0x%08X \n", read_data);

 	//Data
	REG32(mei_xfraddr_c) = BRAM_BASE;
	read_data = REG32(mei_dataxfr_c);
	dprint("DFE Data memory value 0x%08X \n", read_data);

  	//Download ARC code
  	print("Download ARC Code\n");
  	arc_code_data_download(4096, &ToD_arc_pmem0[0], 4096, &ToD_arc_dmem0[0]);

	//Checking DFE memories after ARC FW download
	//Code
	REG32(mei_xfraddr_c) = IRAM0_BASE;
 	read_data = REG32(mei_dataxfr_c);
	exp_data = 0x0F802020;
	if (read_data != exp_data) {
		flag=0;
	}
	dprint("DFE Code memory value 0x%08X \n", read_data);

 	//Data
	REG32(mei_xfraddr_c) = BRAM_BASE;
	read_data = REG32(mei_dataxfr_c);
	exp_data = 0x0020C400;
	if (read_data != exp_data) {
		flag=0;
	}
	dprint("DFE Data memory value 0x%08X \n", read_data);

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_arc_dfe_mem_access(u8 pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE ARC FW access to DFE memories \n");
        print("\t*****************************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	unsigned int i, j, flag;
	int read_data;

	print("\n");

        print("Download ARC Code\n");
	arc_code_data_download(4096, &arc_dfe_mem_pmem0[0], 4096, &arc_dfe_mem_dmem0[0]);

	print("Start ARC\n");
        start_arc(pcie_port);

	flag=1;
	read_data=REG32(MEIAD(ME_ARC2ME_STAT));
  	while(1)
	    	{
		// Wait for a message from ARC
	      	while((read_data&0x1)==0)
			read_data=REG32(MEIAD(ME_ARC2ME_STAT));
	      		// Check Message @ ME_ARC_GP_STAT
	      		read_data=REG32(MEIAD(ME_ARC_GP_STAT));
			// Print out DATECODE
			if (read_data==0x30032012) {
				print("Start Auto-checking by ARC\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			// Print out pattern
			if (read_data==0x12345678) {
				dprint("Pattern 0x12345678\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x5A5A5A5A) {
				dprint("Pattern 0x5A5A5A5A\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0xA5A5A5A5) {
				dprint("Pattern 0xA5A5A5A5\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0xFFFFFFFF) {
				dprint("Pattern 0xFFFFFFFF\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x00000000) {
				dprint("Pattern 0x00000000\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x87654321) {
				dprint("Pattern 0x87654321\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			// Print out different memories
			if (read_data==0x00babe00) {
				dprint("Testing IRAM\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x11babe11) {
				dprint("Testing DCCM\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x22babe22) {
				dprint("Testing XRAM\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x33babe33) {
				dprint("Testing YRAM\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			if (read_data==0x44babe44) {
				dprint("Testing IIBRAM\n");
				REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			}
			// Detecting end of simulation
			if ((read_data&0xFFFF0000)==0xbabe0000) {
		  	// This should end the sim
				print("Checking is completed\n");
		  		if ((read_data&0xffff)==0x600d) {
					print("PASS, Data read value 0x%08X \n", read_data);
					break;
		    		} else {
					flag=0;
					print("FAIL, Data read value 0x%08X \n", read_data);
					break;
		    		}
			}
			// Loop
	      		read_data=REG32(MEIAD(ME_ARC2ME_STAT));
	    	}

   	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_arc_aec_dreg_access(u8 pcie_port) {

        print("\t**************************************************\n");
        print("\tDFE ARC FW access to Registers AEC, PPE DREG \n");
        print("\t**************************************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	unsigned int i, j, flag;
	int read_data;

	print("\n");

        print("Download ARC Code\n");
	arc_code_data_download(4096, &arc_glp_aec_dreg_pmem0[0], 4096, &arc_glp_aec_dreg_dmem0[0]);

	print("Start ARC\n");
        start_arc(pcie_port);

	flag=1;
	print("Start testing\n");
	read_data=REG32(MEIAD(ME_ARC2ME_STAT));
  	while(1)
	    	{
		// Wait for a message from ARC
	      	while((read_data&0x1)==0)
			read_data=REG32(MEIAD(ME_ARC2ME_STAT));
	      	// Check Message @ ME_ARC_GP_STAT
	      	read_data=REG32(MEIAD(ME_ARC_GP_STAT));
	      	if ((read_data&0xFFFF0000)!=0xbabe0000) {
		// Simple stuff to be flag on the UART
		dprint("Data read value 0x%08X \n", read_data);
		REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
		} else {
		// This should end the sim
			print("Checking is completed\n");
			if ((read_data&0xffff)==0x600d) {
				print("PASS, Data read value 0x%08X \n", read_data);
				break;
			} else {
				flag=0;
				print("FAIL, Data read value 0x%08X \n", read_data);
				break;
			}
		}
	      	read_data=REG32(MEIAD(ME_ARC2ME_STAT));
	    	}

   	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_ToD_check(u8 pcie_port) {

        print("\t*****************************************\n");
        print("\tDFE ToD check \n");
        print("\t*****************************************\n");

  	//Set GPIO5 and GPIO14
  	//print("Set GPIO5 and GPIO14\n");
  	//gpio_port_0_cfg(5, OUT, 3);
  	//gpio_port_0_cfg(14, OUT, 1);

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

  	//Download ARC code
  	dprint("Download ARC Code\n");
  	arc_code_data_download(4096, &ToD_arc_pmem0[0], 4096, &ToD_arc_dmem0[0]);

  	dprint("Start ARC\n");
  	start_arc(pcie_port);

	print("Test is completed, waveform check is required\n");
	print("Check GPIO5 and GPIO14\n");
	return;
}

static void dfe_icache(u8 pcie_port, u8 mode) {

        print("\t*****************\n");
        print("\tDFE icache check \n");
        print("\t*****************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	unsigned int i, j, flag, dram_base_addr;
	u32 read_data, exp_data;
	u32 iteration, xbar_sel, tmp;
	u32 pc, pc2;
	u32 prand, prand2, prand3, prand4;

	int arc_code_pre[] = {
		0x7000264a,// 	nop
		0x7000264a,// 	nop
		0x7000264a,// 	nop
		0x7000264a,// 	nop
		0x7000264a,// 	nop
		0x7000264a,// 	nop
		0x7000264a// 	nop
	};

	int arc_code_template[] = {
		0x0f80200a,//mov r0, limm
		0x12345678
	};

	int arc_code_post[] = {
  		0x7000264a,//	nop
  		0x7000264a,//   nop
  		0x00402069,//   flag 1
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a,//   nop
  		0x7000264a//   nop
	};

	// Download ARC code
	dprint("Download ARC Code\n");
	arc_code_data_download(4096, &arc_icache_pmem0[0], 4096, &arc_icache_dmem0[0]);

	// ---------------------------------
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Generate random value
	prand = 0x1; // Temporary fixed to 1
	iteration = 0x2;
	flag=1;
	print("\n");
	dprint("prand2 :\t0x%08X\n", prand2);
	dprint("&prand2 :\t0x%08X\n", &prand2);
	dprint("prand4 :\t0x%08X\n", prand4);
	dprint("&prand4 :\t0x%08X\n", &prand4);
	print("Start checking...\n");
	for (j=0; j<iteration; j++) {
		print("Iteration %02d\n", j);
		dprint("prand :\t0x%08X\n",prand);
		dprint("&prand :\t0x%08X\n",&prand);
		// Selection correct xbar
		xbar_sel = grand(&prand) & 0xF;
		//print("xbar_sel :\t0x%08X\n", xbar_sel);
		read_data = xbar_sel+(j<<24);
		dprint("read_data :\t0x%08X\n", read_data);
		if (mode == 0) {
			// Set XMEM_BAR0 to PPE SB
			// PPE base address : 0x1E20_0000
			// PPE SB RAM0 => 0x1E20_0000 + 0x8000*4 => 0x1E22_0000
			dprint("Set XMEM_BAR0 to PPE SB\n");
			REG32(MEIAD(ME_XMEM_BAR0+((xbar_sel)<<2)))= 0x1E210000;
			REG32(MEIAD(ME_XMEM_BAR0+((xbar_sel+1)<<2)))= 0x1E220000;
			dram_base_addr = 0xBC210000;
		} else {
			// Set XMEM_BAR0 to PDBRAM
			// PDBRAM base address : 0x1E08_0000
			dprint("Set XMEM_BAR0 to PDBRAM\n");
			REG32(MEIAD(ME_XMEM_BAR0+((xbar_sel)<<2)))= 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR0+((xbar_sel+1)<<2)))= 0x1E090000;
			dram_base_addr = 0xBC080000;
		}
		pc=(grand(&prand)&0xFC)+0xFF00;
		pc2=pc+dram_base_addr;
		pc=pc+dsp_sdram_base+((xbar_sel)<<16)+0x1C;
		dprint("pc2 :\t0x%08X\n", pc2);
		dprint("pc :\t0x%08X\n", pc);
		//print("Start ARC icache configuration\n");
		// Start ARC
		me_dbg_decode(ARC_AUX_REG_ADDR_SPACE);
		// Halt ARC
		me_dbg_wr(0x5,0x2);
		// Start icache invalidation
		me_dbg_wr(0x10,0);
		// Write new ARC
		dprint("pc value : 0x%08X\n", pc);
		me_dbg_wr(0x6,pc);
		prand2 = prand;
		prand3 = prand;
		prand4 = prand;

		// Load ARC code
		print("Load arc_code_pre\n");
		for (i=0; i<7; i++) {
			REG32(pc2)=arc_code_pre[i];
			dprint("pc2 :\t0x%08X code : \t0x%08X\n", pc2, arc_code_pre[i]);
			pc2+=4;
		}
		prand3=prand3&0x1F;
		print("Load arc_code_template\n");
		for (i=0; i<10; i++) {
			dprint("pc2 value : 0x%08X\n", pc2);
			REG32(pc2)=arc_code_template[0]|((prand3&0x7)<<8)|((prand3&0x38)<<25);
			read_data = arc_code_template[0]|((prand3&0x7)<<8)|((prand3&0x38)<<25);
			dprint("pc2 value : 0x%08X code : \t0x%08X\n", pc2, read_data);
			prand3=(prand3+5)&0x1F;
			pc2+=4;
			tmp=grand(&prand);
			tmp=((tmp>>16)&0xFFFF)|((tmp&0xFFFF)<<16);
			REG32(pc2)=tmp;
			dprint("tmp :\t0x%08X\n", tmp);
			pc2+=4;
		}
		print("Load arc_code_post\n");
		for (i=0; i<15; i++) {
			REG32(pc2)=arc_code_post[i];
			dprint("pc2 value :0x%08X code : \t0x%08X\n", pc2, arc_code_post[i]);
			pc2+=4;
		}
		print("Restart ARC\n");
		me_dbg_wr(0xA, 0x0);
		// Wait for self halt
		read_data=0;
		while ((read_data&0x40000000)==0) {
			me_dbg_rd(0x5,read_data);
		}

		// Checking Core Reg
		print("Checking core registers\n");
		me_dbg_decode(ARC_CORE_REG_ADDR_SPACE);
		prand4=prand4&0x1F;
		dprint("prand4 :\t0x%08X\n", prand4);
		dprint("&prand4 :\t0x%08X\n", &prand4);
		for (i=0; i<10; i++) {
			me_dbg_rd(prand4,read_data);
			dprint("prand4 :\t0x%08X\n", prand4);
			dprint("&prand4 :\t0x%08X\n", &prand4);
			prand4=(prand4+5)&0x1F;
			dprint("prand4 :\t0x%08X\n", prand4);
			dprint("&prand4 :\t0x%08X\n", &prand4);
			dprint("prand2 :\t0x%08X\n", prand2);
			dprint("&prand2 :\t0x%08X\n", &prand2);
			exp_data = grand(&prand2);
			if (read_data!=exp_data) {
				print("Failed, Counter\t %02d\tprand4\t %02d\n", i, prand4);
				print("Read data\t 0x%08X\n", read_data);
				dprint("prand2\t 0x%08X\n", prand2);
				dprint("&prand2\t 0x%08X\n", &prand2);
				print("Expected data\t 0x%08X\n", exp_data);
				flag=0;
				//break;
			}
		}
		// Soft reset AHB logic
		print("Soft reset AHB logic\n");
		REG32(MEIAD(ME_RST_CTRL))=0x4;
		for (i=0; i<10; i++);
		REG32(MEIAD(ME_RST_CTRL))=0x0;
	}
	print("\n");

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_parallel_mips_arc(u8 pcie_port) {

        print("\t**************************\n");
        print("\tDFE parallel datapath \n");
	print("\t1. MIPS access DFE DCCM\n");
	print("\t2. ARC access to XYRAM\n");
        print("\t**************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
	unsigned int i, j, flag, iteration;
	int addr, read_data, read_status, read_val;
	int total_dword, total_byte;

	print("\n");

        print("Download ARC Code\n");
	arc_code_data_download(4096, &arc_dfe_xyram_pmem0[0], 4096, &arc_dfe_xyram_dmem0[0]);

	print("Start ARC\n");
        start_arc(pcie_port);

	flag=1;
	iteration=0xF;
	read_data=REG32(MEIAD(ME_ARC2ME_STAT));

	// Wait for a message from ARC
	//while(1) {
	print("Start testing: parallel access by MIPS, ARC\n");
	for(i=0;i<iteration;i++) {
		// ------------------------------------------------
		// ARC
		dprint("Start ARC auto-check\n");
		// Read twice in case ARC status is missed
		read_status=REG32(MEIAD(ME_ARC2ME_STAT));
		read_status=REG32(MEIAD(ME_ARC2ME_STAT));
		dprint("Read status 0x%08X\n", read_status);
		if ((read_status&0x1)==1) {
			// Check Message @ ME_ARC_GP_STAT
			read_data=REG32(MEIAD(ME_ARC_GP_STAT));
			dprint("Read data 0x%08X\n", read_data);
		}
		// Print out DATECODE
		if (read_data==0x30032012) {
			dprint("Start Auto-checking access to XYRAM by ARC\n");
			REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
		}
		if (read_data==0xbabe600d) {
			print("ARC auto-check passed\n");
			REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
		}
		if (read_data==0xbabebadd) {
			print("ARC auto-check failed\n");
			print("Iteration %05d\n", i);
			REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			flag=0;
			break;
		}
		read_data=REG32(MEIAD(ME_ARC_GP_STAT));
		// ------------------------------------------------
		// MIPS accessing DDCM
		// ---------------------------------
		// Program RCU_DBGR field FPI_LDST_SEL
		// Address : 0xBC00_2050
		//REG32(0xBC002050) = 0x2;
		dprint("Start MIPS accessing DCCM\n");
		REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = (0x1 << 1);
		for(j=0;j<6;j++) {
			// Write to DFE memories
			// DCCM
			addr = dsp_dccm_base + i*4;
			REG32(FPIAD(addr)) = pattern[i];
			// Read from DFE memories
			read_val = REG32(FPIAD(addr));
			if (j==5) {
				dprint("MIPS access to DCCM location 0x%08X\n", addr);
			}
			if (read_val != pattern[i]) {
				print ("Write-Read access 0x%08X Failed!!\n", addr);
				print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                		flag=0;
                                break;
			}
		}
		print("MIPS access to DCCM is completed\n");
		REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;
		print("Iteration %04d\n", i);
	}
	print("\n");

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_parallel_mips_xdma_arc(u8 pcie_port) {

        print("\t**************************\n");
        print("\tDFE parallel datapath \n");
	print("\t1. MIPS access DFE DCCM\n");
	print("\t2. XDMA transfer to PDBRAM\n");
	print("\t3. ARC access to XYRAM\n");
        print("\t**************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

	// Data pattern
    	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0xFFFFFFFF, 0x00000000, 0x87654321};
	unsigned int i, j, k, flag, iteration;
	int addr, read_data, read_status, read_val;
	int total_dword, total_byte;
	unsigned int xdma_program, xdma_start, xdma_end;
	int pattern_xdma;

	print("\n");

        print("Download ARC Code\n");
	arc_code_data_download(4096, &arc_dfe_xyram_pmem0[0], 4096, &arc_dfe_xyram_dmem0[0]);

	print("Start ARC\n");
        start_arc(pcie_port);

	flag=1;
	xdma_program=1;
	xdma_start=1;
	xdma_end=0;
	pattern_xdma=0x12345678;
	iteration=0xF;
	read_data=REG32(MEIAD(ME_ARC2ME_STAT));

	// Wait for a message from ARC
	//while(1) {
	print("Start testing: parallel access by MIPS, XDMA, ARC\n");
	for(i=0;i<iteration;i++) {
		// ------------------------------------------------
		// ARC
		dprint("Start ARC auto-check\n");
		// Read twice in case ARC status is missed
		read_status=REG32(MEIAD(ME_ARC2ME_STAT));
		read_status=REG32(MEIAD(ME_ARC2ME_STAT));
		dprint("Read status 0x%08X\n", read_status);
		if ((read_status&0x1)==1) {
			// Check Message @ ME_ARC_GP_STAT
			read_data=REG32(MEIAD(ME_ARC_GP_STAT));
			dprint("Read data 0x%08X\n", read_data);
		}
		// Print out DATECODE
		if (read_data==0x30032012) {
			dprint("Start Auto-checking access to XYRAM by ARC\n");
			REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
		}
		if (read_data==0xbabe600d) {
			print("ARC auto-check passed\n");
			REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
		}
		if (read_data==0xbabebadd) {
			print("ARC auto-check failed\n");
			print("Iteration %05d\n", i);
			REG32(MEIAD(ME_ARC2ME_STAT))=0x1;//Clear the interrupt
			flag=0;
			break;
		}
		read_data=REG32(MEIAD(ME_ARC_GP_STAT));
		// ------------------------------------------------
		// MIPS accessing DDCM
		// ---------------------------------
		// Program RCU_DBGR field FPI_LDST_SEL
		// Address : 0xBC00_2050
		//REG32(0xBC002050) = 0x2;
		dprint("Start MIPS accessing DCCM\n");
		REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = (0x1 << 1);
		for(j=0;j<6;j++) {
			// Write to DFE memories
			// DCCM
			addr = dsp_dccm_base + i*4;
			REG32(FPIAD(addr)) = pattern[i];
			// Read from DFE memories
			read_val = REG32(FPIAD(addr));
			if (j==5) {
				dprint("MIPS access to DCCM location 0x%08X\n", addr);
			}
			if (read_val != pattern[i]) {
				print ("Write-Read access 0x%08X Failed!!\n", addr);
				print ("Write Value: %08X\tRead Value: %08X\n", pattern[i], read_val);
                		flag=0;
                                break;
			}
		}
		print("MIPS access to DCCM is completed\n");
		REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;
		// ------------------------------------------------
		// XDMA data transfer
		// Set MEI as Debug Master
		dprint("Start XDMA data transfer\n");
		me_dbg_master_on;
		me_dbg_port(0);
		me_dbg_decode(ARC_LDST_ADDR_SPACE);
		if (xdma_program==1) {
			// Config BAR0 address to AR10 SDRAM
			dprint("Configure BAR0-15 register\n");
			REG32(MEIAD(ME_XMEM_BAR0)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR1)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR2)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR3)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR4)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR5)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR6)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR7)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR8)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR9)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR10)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR11)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR12)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR13)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR14)) = 0x1E080000;
			REG32(MEIAD(ME_XMEM_BAR15)) = 0x1E080000;
		}
		if (xdma_start==1) {
			total_dword = 0x20;
			total_byte = ((total_dword-1)*4) + 0x2;
			// Use IIBRAM offset 0x5000
			addr = dsp_iibram_base + 0x5000 + i*4;
			// Testing all locations
			dprint("Initialize IIBRAM for XDMA transfer\n");
			for (k=0; k<total_dword; k++) {
				// Write to IIBRAM
				me_dbg_wr(addr+(k*4),(pattern_xdma+k));
			}
			// Start XDMA
			dprint("Start XDMA transfer from IIBRAM to PDBRAM\n");
			me_dbg_wr(D_XDMA0_SA_S,addr);
			me_dbg_wr(D_XDMA0_SA_E,addr+total_byte);
			me_dbg_wr(D_XDMA0_DA_S,0);
			me_dbg_wr(D_XDMA0_CTRL,3);
		}

		// Check for status bit
		dprint("Checking for completion of transfer\n");
		me_dbg_rd(D_XDMA0_CTRL,read_val);
		if ((read_val & 0x00000001) == 0x00000001) {
			// Transfer is completed
			print("XDMA data transfer to PDBRAM is completed\n");
			// Transfer is not completed
			xdma_start = 1;
			xdma_end = 1;
		} else {
			// Transfer is not completed
			print("XDMA data transfer is ongoing\n");
			xdma_start = 0;
		}
		if (xdma_end==1) {
			dprint("Testing with pattern 0x%08X ..", pattern_xdma);
			for (k=0; k<total_dword; k++) {
				// Read from PDBRAM
				read_val = REG32(0xBC080000 + k*4);
				if (read_val != (pattern_xdma+k)) {
	                		print ("Write-Read access 0x%08X Failed!!\n", (j));
	                		print ("Write Value: %08X\tRead Value: %08X\n", (pattern_xdma+k), read_val);
	                		flag=0;
	                                break;
				}
	                        if ((k & 0x7FF) == 0x7FF)
	                                dprint (".");
			}
		}
		xdma_program = 0;
		me_dbg_master_off;
		print("Iteration %04d\n", i);
	}
	print("\n");

    	if (flag == 0) {
        	print("TEST FAIL\n\n");
    	} else {
        	print("TEST PASS\n\n");
    	}

	return;
}

static void dfe_mei_me_irq_to_msi(u8 pcie_port) {

        print("\t********************************\n");
        print("\tDFE MEI ME_IRQ interrupt to MSI \n");
        print("\t********************************\n");

	print("Please run PCIe - MSI Test - MSI default setting test first\n");
	print("This is to ensure all MSI related registers are configured\n");
	print("If MSI interrupt is received, specific print out will be displayed:\n");
	print("Please check for printout : irq 156  interrupt received\n\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	// ---------------------------------
	// Reset RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x0;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = 0x0;

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	int addr, read_data;

	print("\nStart ME_IRQ testing\n\n");
	REG32(MEIAD(ME_ARC2ME_MASK)) = REG32(MEIAD(ME_ARC2ME_MASK)) | 0x1;
	read_data = REG32(MEIAD(ME_ARC2ME_STAT));
	dprint("ME_IRQ Mask status 0x%08X\n", read_data);

	// ---------------------------------
	// Set MEI as Debug Master
	dprint("Set MEI as Debug Master\n");
	me_dbg_master_on;
	me_dbg_port(0);
	me_dbg_decode(ARC_LDST_ADDR_SPACE);

	// Set ME_IRQ interrupt
	dprint("Set ME_IRQ interrupt\n");
	me_dbg_wr(D_ARC2ME_INT, 0x1);

	read_data = REG32(MEIAD(ME_ARC2ME_STAT));
	dprint("ME_IRQ Interrupt status 0x%08X\n", read_data);

	print("\n");

	return;

}

static void dfe_arc_jtag_to_dfe(u8 pcie_port) {

        print("\t*******************************\n");
        print("\tDFE ARC JTAG acecss to DFE MEM \n");
        print("\t*******************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	// ---------------------------------
	// Program RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x2;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = (0x1 << 1);

	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0x87654321, 0x12341234};
	int i, total;

	total = 0x100;

	print("\nStart initializing some DFE memories locations\n");
	dprint("\nIRAM, initialize with pattern 0x%08X", pattern[0]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(dsp_iram_base+i*4)) = pattern[0];
	}
	dprint("\nDCCM, initialize with pattern 0x%08X", pattern[1]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(dsp_dccm_base+i*4)) = pattern[1];
	}
	dprint("\nXRAM, initialize with pattern 0x%08X", pattern[2]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(dsp_xram_base+i*4)) = pattern[2];
	}
	dprint("\nYRAM, initialize with pattern 0x%08X", pattern[3]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(dsp_yram_base+i*4)) = pattern[3];
	}
	dprint("\nIIBRAM, initialize with pattern 0x%08X", pattern[4]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(dsp_iibram_base+i*4)) = pattern[4];
	}

	print("\nARC JTAG is enabled by default\n");
	print("Please connect ARC JTAG and start testing\n");
	return;
}

static void dfe_arc_jtag_to_ppe(u8 pcie_port) {

        print("\t*******************************\n");
        print("\tDFE ARC JTAG acecss to PPE MEM \n");
        print("\t*******************************\n");

  	//reset dfe before init
	vrx218_dfe_reset(pcie_port);

	//Initialize DFE
  	dprint("Initialize DFE\n");
  	vrx218_dfe_init(pcie_port);

	// ---------------------------------
	// Program RCU_DBGR field FPI_LDST_SEL
	// Address : 0xBC00_2050
 	//REG32(0xBC002050) = 0x2;
	REG32(PCIE_ADDR(pcie_port, VRX218_RCU_DBGR)) = (0x1 << 1);

	int pattern[] = {0x12345678, 0x5A5A5A5A, 0xA5A5A5A5,
			 0x87654321, 0x12341234, 0x56785678};
	int i, addr, total;
	int ppe_cdm0_addr, ppe_cdm2_addr;
	int ppe_sbram0_addr, ppe_sbram1_addr, ppe_sbram2_addr, ppe_sbram3_addr;

	total = 0x100;
	ppe_cdm0_addr = 0xBC200000 + 0x1000*4;
	ppe_cdm2_addr = 0xBC200000 + 0x11000*4;
	ppe_sbram0_addr = 0xBC200000 + 0x8000*4;
	ppe_sbram1_addr = 0xBC200000 + 0x9000*4;
	ppe_sbram2_addr = 0xBC200000 + 0xA000*4;
	ppe_sbram3_addr = 0xBC200000 + 0xB000*4;

	// Start testing
	print("\nStart initializing some PPE memories locations\n");
	dprint("\nConfigure CDM CFG for CDM0 and CDM2 setting\n");
	addr = 0xBC200000 + 0xD100*4;
	REG32(addr) = 0xC8;

	dprint("\nPPE CDM0, initialize with pattern 0x%08X", pattern[0]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(ppe_cdm0_addr+i*4)) = pattern[0];
	}
	dprint("\nPPE CDM2, initialize with pattern 0x%08X", pattern[1]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(ppe_cdm2_addr+i*4)) = pattern[1];
	}
	dprint("\nPPE SB RAM0, initialize with pattern 0x%08X", pattern[2]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(ppe_sbram0_addr+i*4)) = pattern[2];
	}
	dprint("\nPPE SB RAM1, initialize with pattern 0x%08X", pattern[3]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(ppe_sbram1_addr+i*4)) = pattern[3];
	}
	dprint("\nPPE SB RAM2, initialize with pattern 0x%08X", pattern[4]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(ppe_sbram2_addr+i*4)) = pattern[4];
	}
	dprint("\nPPE SB RAM3, initialize with pattern 0x%08X", pattern[5]);
	for(i=0; i<total; i++) {
		REG32(FPIAD(ppe_sbram3_addr+i*4)) = pattern[5];
	}

	print("\nARC JTAG is enabled by default\n");
	print("Please connect ARC JTAG and start testing\n");
	return;
}
#endif
