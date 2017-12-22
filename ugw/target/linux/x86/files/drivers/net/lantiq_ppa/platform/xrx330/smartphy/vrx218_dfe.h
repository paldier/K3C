#ifndef __VRX218_DFE_H
#define __VRX218_DFE_H

//	#include <lib.h>
//	#include <asc.h>

   	#include "vrx218_dsl_address_define.h"

	typedef struct {
		u32 addr;
		u32 data;
		u32 type;
	} ME_CFG_def_t;

	//Header file to set the Loopback rate to programmable!
	#include "progRate.h"

   	//ARC FW for ToD
//   	#include <ToD_dmem0.h>
//   	#include <ToD_pmem0.h>

	//ARC FW for icache test
//	#include <arc_icache_dmem0.h>
//	#include <arc_icache_pmem0.h>

	//Table for DFE-PPE DREG access
//	#include <dfe_ppe_dreg_tbl.h>

	//ARC -> AEC, DREG, (GLP is black-boxed)
//	#include <arc_glp_aec_dreg_pmem0.h>
//	#include <arc_glp_aec_dreg_dmem0.h>

	//ARC -> DFE MEM
//	#include <arc_dfe_mem_pmem0.h>
//	#include <arc_dfe_mem_dmem0.h>

	//ARC -> XYRAM
//	#include <arc_dfe_xyram_pmem0.h>
//	#include <arc_dfe_xyram_dmem0.h>

	//Global definition
    #define VRX218_RST_REQ   0x1E002010
//	#define VRX218_RST_STAT	 0x1E002014
    #define VRX218_RST_STAT  0x1E002000
	#define VRX218_PMU_PWDCR 0x1E00311C
	#define VRX218_PMU_SR 	 0x1E003120
	#define VRX218_RCU_DBGR  0x1E002050

    #define PCIE_ADDR(__pcie_node, __addr) ((__pcie_node)?(0xA0000000 + 0x18000000 + ((__addr) & 0x00FFFFFF)):(0xA0000000 + 0x1C000000 + ((__addr) & 0x00FFFFFF)))

#endif
