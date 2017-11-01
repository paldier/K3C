/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "ltq_ethsw_init.h"

#define PCE_MC_M(val, msk, ns, out, len, type, flags, ipv4_len) \
    { val, msk, (ns<<10 | out<<4 | len>>1), (len&1)<<15 | type<<13 | flags<<9 | ipv4_len<<8 }
const PCE_MICROCODE pce_mc_max_ifx_tag_m = {
    /*------------------------------------------------------------------------------------------
    **                value    mask   ns  out_fields   L  type     flags       ipv4_len
    ------------------------------------------------------------------------------------------*/
	PCE_MC_M(0x88c3 , 0xFFFF , 1 , OUT_ITAG0 , 4 , INSTR , FLAG_ITAG , 0),
	PCE_MC_M(0x8100 , 0xFFFF , 2 , OUT_VTAG0 , 2 , INSTR , FLAG_VLAN , 0),
	PCE_MC_M(0x88A8 , 0xFFFF , 1 , OUT_VTAG0 , 2 , INSTR , FLAG_VLAN , 0),
	PCE_MC_M(0x8100 , 0xFFFF , 1 , OUT_VTAG0 , 2 , INSTR , FLAG_VLAN , 0),
	PCE_MC_M(0x8864 , 0xFFFF , 17 , OUT_ETHTYP , 1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0800 , 0xFFFF , 21 , OUT_ETHTYP ,	1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x86DD , 0xFFFF , 22 , OUT_ETHTYP ,	1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x8863 , 0xFFFF , 16 , OUT_ETHTYP , 1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0xF800 , 10 , OUT_NONE , 0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 40 , OUT_ETHTYP ,	1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0600 , 0x0600 , 40 , OUT_ETHTYP ,	1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 12 , OUT_NONE , 1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0xAAAA , 0xFFFF , 14 , OUT_NONE , 1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0300 , 0xFF00 , 41 , OUT_NONE ,		0 , INSTR , FLAG_SNAP , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_DIP7 ,		3 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 18 , OUT_DIP7 , 3 , INSTR , FLAG_PPPOE , 0),
	PCE_MC_M(0x0021 , 0xFFFF , 21 , OUT_NONE ,		1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0057 , 0xFFFF , 22 , OUT_NONE ,		1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 40 , OUT_NONE ,		0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x4000 , 0xF000 , 24 , OUT_IP0 ,		4 , INSTR , FLAG_IPV4 , 1),
	PCE_MC_M(0x6000 , 0xF000 , 27 , OUT_IP0 ,		3 , INSTR , FLAG_IPV6 , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 25 , OUT_IP3 ,		2 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 26 , OUT_SIP0 ,		4 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 40 , OUT_NONE ,		0 , LENACCU , FLAG_NO , 0),
	PCE_MC_M(0x1100 , 0xFF00 , 39 , OUT_PROT ,		1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0600 , 0xFF00 , 39 , OUT_PROT ,		1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0xFF00 , 33 , OUT_IP3 ,		17, INSTR , FLAG_HOP , 0),
	PCE_MC_M(0x2B00 , 0xFF00 , 33 , OUT_IP3 ,		17, INSTR , FLAG_NN1 , 0),
	PCE_MC_M(0x3C00 , 0xFF00 , 33 , OUT_IP3 ,		17, INSTR , FLAG_NN2 , 0),
	PCE_MC_M(0x0000 , 0x0000 , 39 , OUT_PROT ,		1 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x00E0 , 35 , OUT_NONE , 0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 40 , OUT_NONE , 0 , INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0xFF00 , 33 , OUT_NONE ,		0 , IPV6 ,  FLAG_HOP , 0),
	PCE_MC_M(0x2B00 , 0xFF00 , 33 , OUT_NONE ,		0 , IPV6 ,  FLAG_NN1 , 0),
	PCE_MC_M(0x3C00 , 0xFF00 , 33 , OUT_NONE ,		0 , IPV6 ,  FLAG_NN2 , 0),
	PCE_MC_M(0x0000 , 0x0000 , 40 , OUT_PROT ,		1 , IPV6 ,  FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 40 , OUT_SIP0 ,		16, INSTR , FLAG_NO , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_APP0 ,		4 , INSTR , FLAG_IGMP , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
	PCE_MC_M(0x0000 , 0x0000 , 41 , OUT_NONE ,		0 , INSTR , FLAG_END , 0),
};

/* Static Function Declaration */
static int tbl_write(void *pTblStart, u16 *pRefCnt, void *pPar, u32 TblEntrySize, u32 TblEntryNum)
{
	int i;
	/* search if the entry is already available and can be re-used */
	for (i = 0; i < TblEntryNum; i++) {
		if (pRefCnt[i] > 0) {
			/* entry is used, check if the entry content fits */
			if (memcmp(((char *)pTblStart) + i * TblEntrySize, pPar, (u8)TblEntrySize) == 0) {
				/* content is the same, increment reference counter and return the index*/
				pRefCnt[i]++;
				return i;
			}
		}
	}
	/* find an empty entry and add information */
	for (i = 0; i < TblEntryNum; i++) {
		if (pRefCnt[i] == 0) {
			memcpy(((char *)pTblStart) + i * TblEntrySize, pPar, (u8)TblEntrySize);
			pRefCnt[i]++;
			return i;
		}
	}
	/* table is full, return an error */
	pr_err("ERROR:\n\tFile %s\n\tLine %d\n", __FILE__, __LINE__);
	return -1;
}

static int tbl_idx_delete(u16 *pRefCnt, u32 index, u32 TblSize)
{
	PCE_ASSERT(index >= TblSize);
	if (pRefCnt[index] > 0)
		pRefCnt[index]--;
	return LTQ_SUCCESS;
}

ethsw_ret_t xwayflow_pce_table_write(void *pDevCtx, pce_table_prog_t *pData)
{
	u32 value;
	u16 udata;
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET,	\
			PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	} while (value);
	gsw_w32(PCE_TBL_ADDR_ADDR_OFFSET,	\
		PCE_TBL_ADDR_ADDR_SHIFT, PCE_TBL_ADDR_ADDR_SIZE, pData->table_index);
	udata = pData->table;
	gsw_w32(PCE_TBL_CTRL_ADDR_OFFSET,	\
		PCE_TBL_CTRL_ADDR_SHIFT, PCE_TBL_CTRL_ADDR_SIZE, udata);
	gsw_w32(PCE_TBL_CTRL_OPMOD_OFFSET,	\
		PCE_TBL_CTRL_OPMOD_SHIFT, PCE_TBL_CTRL_OPMOD_SIZE, TABLE_ACCESS_OP_MODE_ADWR);
	gsw_w32(PCE_TBL_KEY_8_KEY8_OFFSET,	\
		PCE_TBL_KEY_8_KEY8_SHIFT, PCE_TBL_KEY_8_KEY8_SIZE, pData->key[8]);
	gsw_w32(PCE_TBL_KEY_7_KEY7_OFFSET,	\
		PCE_TBL_KEY_7_KEY7_SHIFT, PCE_TBL_KEY_7_KEY7_SIZE, pData->key[7]);
	gsw_w32(PCE_TBL_KEY_6_KEY6_OFFSET,	\
		PCE_TBL_KEY_6_KEY6_SHIFT, PCE_TBL_KEY_6_KEY6_SIZE, pData->key[6]);
	gsw_w32(PCE_TBL_KEY_5_KEY5_OFFSET,	\
		PCE_TBL_KEY_5_KEY5_SHIFT, PCE_TBL_KEY_5_KEY5_SIZE, pData->key[5]);
	gsw_w32(PCE_TBL_KEY_4_KEY4_OFFSET,	\
		PCE_TBL_KEY_4_KEY4_SHIFT, PCE_TBL_KEY_4_KEY4_SIZE, pData->key[4]);
	gsw_w32(PCE_TBL_KEY_3_KEY3_OFFSET,	\
		PCE_TBL_KEY_3_KEY3_SHIFT, PCE_TBL_KEY_3_KEY3_SIZE, pData->key[3]);
	gsw_w32(PCE_TBL_KEY_2_KEY2_OFFSET,	\
		PCE_TBL_KEY_2_KEY2_SHIFT, PCE_TBL_KEY_2_KEY2_SIZE, pData->key[2]);
	gsw_w32(PCE_TBL_KEY_1_KEY1_OFFSET,	\
		PCE_TBL_KEY_1_KEY1_SHIFT, PCE_TBL_KEY_1_KEY1_SIZE, pData->key[1]);
	gsw_w32(PCE_TBL_KEY_0_KEY0_OFFSET,	\
		PCE_TBL_KEY_0_KEY0_SHIFT, PCE_TBL_KEY_0_KEY0_SIZE, pData->key[0]);

	gsw_w32(PCE_TBL_MASK_0_MASK0_OFFSET,	\
		PCE_TBL_MASK_0_MASK0_SHIFT, PCE_TBL_MASK_0_MASK0_SIZE, pData->mask);

	gsw_w32(PCE_TBL_VAL_6_VAL6_OFFSET,	\
		PCE_TBL_VAL_6_VAL6_SHIFT, PCE_TBL_VAL_6_VAL6_SIZE, pData->val[6]);
	gsw_w32(PCE_TBL_VAL_5_VAL5_OFFSET,	\
		PCE_TBL_VAL_5_VAL5_SHIFT, PCE_TBL_VAL_5_VAL5_SIZE, pData->val[5]);
	gsw_w32(PCE_TBL_VAL_4_VAL4_OFFSET,	\
		PCE_TBL_VAL_4_VAL4_SHIFT, PCE_TBL_VAL_4_VAL4_SIZE, pData->val[4]);
	gsw_w32(PCE_TBL_VAL_3_VAL3_OFFSET,	\
		PCE_TBL_VAL_3_VAL3_SHIFT, PCE_TBL_VAL_3_VAL3_SIZE, pData->val[3]);
	gsw_w32(PCE_TBL_VAL_2_VAL2_OFFSET,	\
		PCE_TBL_VAL_2_VAL2_SHIFT, PCE_TBL_VAL_2_VAL2_SIZE, pData->val[2]);
	gsw_w32(PCE_TBL_VAL_1_VAL1_OFFSET,	\
		PCE_TBL_VAL_1_VAL1_SHIFT, PCE_TBL_VAL_1_VAL1_SIZE, pData->val[1]);
	gsw_w32(PCE_TBL_VAL_0_VAL0_OFFSET,	\
		PCE_TBL_VAL_0_VAL0_SHIFT, PCE_TBL_VAL_0_VAL0_SIZE, pData->val[0]);
	gsw_w32(PCE_TBL_CTRL_TYPE_OFFSET,	\
		PCE_TBL_CTRL_TYPE_SHIFT, PCE_TBL_CTRL_TYPE_SIZE, pData->type);
	gsw_w32(PCE_TBL_CTRL_VLD_OFFSET,	\
		PCE_TBL_CTRL_VLD_SHIFT, PCE_TBL_CTRL_VLD_SIZE, pData->valid);
	gsw_w32(PCE_TBL_CTRL_GMAP_OFFSET,	\
		PCE_TBL_CTRL_GMAP_SHIFT, PCE_TBL_CTRL_GMAP_SIZE, pData->group);
	gsw_w32(PCE_TBL_CTRL_BAS_OFFSET,	\
		PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, LTQ_TRUE);
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET,	\
			 PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_r32(PCE_TBL_CTRL_BAS_OFFSET, 0, 15, &value);
    return LTQ_SUCCESS;
}

ethsw_ret_t xwayflow_pce_table_read(void *pDevCtx, pce_table_prog_t *pData)
{
	u32 value;
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET, PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	}	while (value != 0);
	gsw_w32(PCE_TBL_ADDR_ADDR_OFFSET, PCE_TBL_ADDR_ADDR_SHIFT,	\
		PCE_TBL_ADDR_ADDR_SIZE, pData->table_index);
	gsw_w32(PCE_TBL_CTRL_ADDR_OFFSET, PCE_TBL_CTRL_ADDR_SHIFT,	\
		PCE_TBL_CTRL_ADDR_SIZE, pData->table);
	gsw_w32(PCE_TBL_CTRL_OPMOD_OFFSET, PCE_TBL_CTRL_OPMOD_SHIFT,	\
		PCE_TBL_CTRL_OPMOD_SIZE, TABLE_ACCESS_OP_MODE_ADRD);
	gsw_w32(PCE_TBL_CTRL_BAS_OFFSET,	PCE_TBL_CTRL_BAS_SHIFT,	\
		PCE_TBL_CTRL_BAS_SIZE, LTQ_TRUE);
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET, PCE_TBL_CTRL_BAS_SHIFT,	\
			PCE_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_r32(PCE_TBL_KEY_8_KEY8_OFFSET, PCE_TBL_KEY_8_KEY8_SHIFT, PCE_TBL_KEY_8_KEY8_SIZE, &value);
	pData->key[8] = value;
	gsw_r32(PCE_TBL_KEY_7_KEY7_OFFSET, PCE_TBL_KEY_7_KEY7_SHIFT, PCE_TBL_KEY_7_KEY7_SIZE, &value);
	pData->key[7] = value;
	gsw_r32(PCE_TBL_KEY_6_KEY6_OFFSET, PCE_TBL_KEY_6_KEY6_SHIFT, PCE_TBL_KEY_6_KEY6_SIZE, &value);
	pData->key[6] = value;
	gsw_r32(PCE_TBL_KEY_5_KEY5_OFFSET, PCE_TBL_KEY_5_KEY5_SHIFT, PCE_TBL_KEY_5_KEY5_SIZE, &value);
	pData->key[5] = value;
	gsw_r32(PCE_TBL_KEY_4_KEY4_OFFSET, PCE_TBL_KEY_4_KEY4_SHIFT, PCE_TBL_KEY_4_KEY4_SIZE, &value);
	pData->key[4] = value;
	gsw_r32(PCE_TBL_KEY_3_KEY3_OFFSET, PCE_TBL_KEY_3_KEY3_SHIFT, PCE_TBL_KEY_3_KEY3_SIZE, &value);
	pData->key[3] = value;
	gsw_r32(PCE_TBL_KEY_2_KEY2_OFFSET, PCE_TBL_KEY_2_KEY2_SHIFT, PCE_TBL_KEY_2_KEY2_SIZE, &value);
	pData->key[2] = value;
	gsw_r32(PCE_TBL_KEY_1_KEY1_OFFSET, PCE_TBL_KEY_1_KEY1_SHIFT, PCE_TBL_KEY_1_KEY1_SIZE, &value);
	pData->key[1] = value;
	gsw_r32(PCE_TBL_KEY_0_KEY0_OFFSET, PCE_TBL_KEY_0_KEY0_SHIFT, PCE_TBL_KEY_0_KEY0_SIZE, &value);
	pData->key[0] = value;
	gsw_r32(PCE_TBL_VAL_6_VAL6_OFFSET, PCE_TBL_VAL_6_VAL6_SHIFT, PCE_TBL_VAL_6_VAL6_SIZE, &value);
	pData->val[6] = value;
	gsw_r32(PCE_TBL_VAL_5_VAL5_OFFSET, PCE_TBL_VAL_5_VAL5_SHIFT, PCE_TBL_VAL_5_VAL5_SIZE, &value);
	pData->val[5] = value;
	gsw_r32(PCE_TBL_VAL_4_VAL4_OFFSET, PCE_TBL_VAL_4_VAL4_SHIFT, PCE_TBL_VAL_4_VAL4_SIZE, &value);
	pData->val[4] = value;
	gsw_r32(PCE_TBL_VAL_3_VAL3_OFFSET, PCE_TBL_VAL_3_VAL3_SHIFT, PCE_TBL_VAL_3_VAL3_SIZE, &value);
	pData->val[3] = value;
	gsw_r32(PCE_TBL_VAL_2_VAL2_OFFSET, PCE_TBL_VAL_2_VAL2_SHIFT, PCE_TBL_VAL_2_VAL2_SIZE, &value);
	pData->val[2] = value;
	gsw_r32(PCE_TBL_VAL_1_VAL1_OFFSET, PCE_TBL_VAL_1_VAL1_SHIFT, PCE_TBL_VAL_1_VAL1_SIZE, &value);
	pData->val[1] = value;
	gsw_r32(PCE_TBL_VAL_0_VAL0_OFFSET, PCE_TBL_VAL_0_VAL0_SHIFT, PCE_TBL_VAL_0_VAL0_SIZE, &value);
	pData->val[0] = value;
	gsw_r32(PCE_TBL_MASK_0_MASK0_OFFSET, PCE_TBL_MASK_0_MASK0_SHIFT, PCE_TBL_MASK_0_MASK0_SIZE, &value);
	pData->mask = value;
	gsw_r32(PCE_TBL_CTRL_TYPE_OFFSET, PCE_TBL_CTRL_TYPE_SHIFT, PCE_TBL_CTRL_TYPE_SIZE, &value);
	pData->type = value;
	gsw_r32(PCE_TBL_CTRL_VLD_OFFSET, PCE_TBL_CTRL_VLD_SHIFT, PCE_TBL_CTRL_VLD_SIZE, &value);
	pData->valid = value;
	gsw_r32(PCE_TBL_CTRL_GMAP_OFFSET, PCE_TBL_CTRL_GMAP_SHIFT, PCE_TBL_CTRL_GMAP_SIZE, &value);
	pData->group = value;
	return LTQ_SUCCESS;
}

ethsw_ret_t xwayflow_pce_table_key_read(void *pDevCtx, pce_table_prog_t *pData)
{
	u32 value;
	do {
	gsw_r32(PCE_TBL_CTRL_BAS_OFFSET, PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	}	while (value != 0);

	gsw_w32(PCE_TBL_CTRL_ADDR_OFFSET, PCE_TBL_CTRL_ADDR_SHIFT,	\
		PCE_TBL_CTRL_ADDR_SIZE, pData->table);
	gsw_w32(PCE_TBL_KEY_8_KEY8_OFFSET, PCE_TBL_KEY_8_KEY8_SHIFT,	\
		PCE_TBL_KEY_8_KEY8_SIZE, pData->key[8]);
	gsw_w32(PCE_TBL_KEY_7_KEY7_OFFSET, PCE_TBL_KEY_7_KEY7_SHIFT,	\
		PCE_TBL_KEY_7_KEY7_SIZE, pData->key[7]);
	gsw_w32(PCE_TBL_KEY_6_KEY6_OFFSET,	\
			PCE_TBL_KEY_6_KEY6_SHIFT, PCE_TBL_KEY_6_KEY6_SIZE, pData->key[6]);
	gsw_w32(PCE_TBL_KEY_5_KEY5_OFFSET,	\
			PCE_TBL_KEY_5_KEY5_SHIFT, PCE_TBL_KEY_5_KEY5_SIZE, pData->key[5]);
	gsw_w32(PCE_TBL_KEY_4_KEY4_OFFSET,	\
			PCE_TBL_KEY_4_KEY4_SHIFT, PCE_TBL_KEY_4_KEY4_SIZE, pData->key[4]);
	gsw_w32(PCE_TBL_KEY_3_KEY3_OFFSET,	\
			PCE_TBL_KEY_3_KEY3_SHIFT, PCE_TBL_KEY_3_KEY3_SIZE, pData->key[3]);
	gsw_w32(PCE_TBL_KEY_2_KEY2_OFFSET,	\
			PCE_TBL_KEY_2_KEY2_SHIFT, PCE_TBL_KEY_2_KEY2_SIZE, pData->key[2]);
	gsw_w32(PCE_TBL_KEY_1_KEY1_OFFSET,	\
			PCE_TBL_KEY_1_KEY1_SHIFT, PCE_TBL_KEY_1_KEY1_SIZE, pData->key[1]);
	gsw_w32(PCE_TBL_KEY_0_KEY0_OFFSET,	\
			PCE_TBL_KEY_0_KEY0_SHIFT, PCE_TBL_KEY_0_KEY0_SIZE, pData->key[0]);
	gsw_w32(PCE_TBL_CTRL_OPMOD_OFFSET,	\
		PCE_TBL_CTRL_OPMOD_SHIFT, PCE_TBL_CTRL_OPMOD_SIZE, TABLE_ACCESS_OP_MODE_KSRD);
	gsw_w32(PCE_TBL_CTRL_BAS_OFFSET,	\
    PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, LTQ_TRUE);
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET,	\
			PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);

	gsw_r32(PCE_TBL_VAL_6_VAL6_OFFSET,	\
		PCE_TBL_VAL_6_VAL6_SHIFT, PCE_TBL_VAL_6_VAL6_SIZE, &value);
	pData->val[6] = value;
	gsw_r32(PCE_TBL_VAL_5_VAL5_OFFSET,	\
		PCE_TBL_VAL_5_VAL5_SHIFT, PCE_TBL_VAL_5_VAL5_SIZE, &value);
	pData->val[5] = value;

	gsw_r32(PCE_TBL_VAL_4_VAL4_OFFSET,	\
		PCE_TBL_VAL_4_VAL4_SHIFT, PCE_TBL_VAL_4_VAL4_SIZE, &value);
	pData->val[4] = value;
	gsw_r32(PCE_TBL_VAL_3_VAL3_OFFSET,	\
		PCE_TBL_VAL_3_VAL3_SHIFT, PCE_TBL_VAL_3_VAL3_SIZE, &value);
	pData->val[3] = value;
	gsw_r32(PCE_TBL_VAL_2_VAL2_OFFSET,	\
		PCE_TBL_VAL_2_VAL2_SHIFT, PCE_TBL_VAL_2_VAL2_SIZE, &value);
	pData->val[2] = value;
	gsw_r32(PCE_TBL_VAL_1_VAL1_OFFSET,	\
		PCE_TBL_VAL_1_VAL1_SHIFT, PCE_TBL_VAL_1_VAL1_SIZE, &value);
	pData->val[1] = value;
	gsw_r32(PCE_TBL_VAL_0_VAL0_OFFSET,	\
		PCE_TBL_VAL_0_VAL0_SHIFT, PCE_TBL_VAL_0_VAL0_SIZE, &value);
	pData->val[0] = value;
	gsw_r32(PCE_TBL_MASK_0_MASK0_OFFSET,	\
		PCE_TBL_MASK_0_MASK0_SHIFT, PCE_TBL_MASK_0_MASK0_SIZE, &value);
	pData->mask = value;
	gsw_r32(PCE_TBL_CTRL_TYPE_OFFSET,	\
		PCE_TBL_CTRL_TYPE_SHIFT, PCE_TBL_CTRL_TYPE_SIZE, &value);
    pData->type = value;
	gsw_r32(PCE_TBL_CTRL_VLD_OFFSET,	\
    PCE_TBL_CTRL_VLD_SHIFT, PCE_TBL_CTRL_VLD_SIZE, &value);
    pData->valid = value;
	gsw_r32(PCE_TBL_CTRL_GMAP_OFFSET,	\
    PCE_TBL_CTRL_GMAP_SHIFT, PCE_TBL_CTRL_GMAP_SIZE, &value);
    pData->group = value;
	return LTQ_SUCCESS;
}
ethsw_ret_t xwayflow_pce_table_key_write(void *pDevCtx, pce_table_prog_t *pData)
{
    u32 value;
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET, \
			PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_w32(PCE_TBL_CTRL_ADDR_OFFSET,	\
		PCE_TBL_CTRL_ADDR_SHIFT, PCE_TBL_CTRL_ADDR_SIZE, pData->table);
	gsw_w32(PCE_TBL_CTRL_OPMOD_OFFSET,	\
		PCE_TBL_CTRL_OPMOD_SHIFT, PCE_TBL_CTRL_OPMOD_SIZE, TABLE_ACCESS_OP_MODE_KSWR);
	gsw_w32(PCE_TBL_KEY_8_KEY8_OFFSET,	\
		PCE_TBL_KEY_8_KEY8_SHIFT, PCE_TBL_KEY_8_KEY8_SIZE, pData->key[8]);
	gsw_w32(PCE_TBL_KEY_7_KEY7_OFFSET,	\
		PCE_TBL_KEY_7_KEY7_SHIFT, PCE_TBL_KEY_7_KEY7_SIZE, pData->key[7]);
	gsw_w32(PCE_TBL_KEY_6_KEY6_OFFSET,	\
		PCE_TBL_KEY_6_KEY6_SHIFT, PCE_TBL_KEY_6_KEY6_SIZE, pData->key[6]);
	gsw_w32(PCE_TBL_KEY_5_KEY5_OFFSET,	\
		PCE_TBL_KEY_5_KEY5_SHIFT, PCE_TBL_KEY_5_KEY5_SIZE, pData->key[5]);
	gsw_w32(PCE_TBL_KEY_4_KEY4_OFFSET,	\
		PCE_TBL_KEY_4_KEY4_SHIFT, PCE_TBL_KEY_4_KEY4_SIZE, pData->key[4]);
	gsw_w32(PCE_TBL_KEY_3_KEY3_OFFSET,	\
		PCE_TBL_KEY_3_KEY3_SHIFT, PCE_TBL_KEY_3_KEY3_SIZE, pData->key[3]);
	gsw_w32(PCE_TBL_KEY_2_KEY2_OFFSET,	\
		PCE_TBL_KEY_2_KEY2_SHIFT, PCE_TBL_KEY_2_KEY2_SIZE, pData->key[2]);
	gsw_w32(PCE_TBL_KEY_1_KEY1_OFFSET,	\
		PCE_TBL_KEY_1_KEY1_SHIFT, PCE_TBL_KEY_1_KEY1_SIZE, pData->key[1]);
	gsw_w32(PCE_TBL_KEY_0_KEY0_OFFSET,	\
		PCE_TBL_KEY_0_KEY0_SHIFT, PCE_TBL_KEY_0_KEY0_SIZE, pData->key[0]);
	gsw_w32(PCE_TBL_MASK_0_MASK0_OFFSET,	\
		PCE_TBL_MASK_0_MASK0_SHIFT, PCE_TBL_MASK_0_MASK0_SIZE, pData->mask);

	gsw_w32(PCE_TBL_VAL_6_VAL6_OFFSET,	\
		PCE_TBL_VAL_6_VAL6_SHIFT, PCE_TBL_VAL_6_VAL6_SIZE, pData->val[6]);
	gsw_w32(PCE_TBL_VAL_5_VAL5_OFFSET,	\
		PCE_TBL_VAL_5_VAL5_SHIFT, PCE_TBL_VAL_5_VAL5_SIZE, pData->val[5]);
	gsw_w32(PCE_TBL_VAL_4_VAL4_OFFSET,	\
		PCE_TBL_VAL_4_VAL4_SHIFT, PCE_TBL_VAL_4_VAL4_SIZE, pData->val[4]);
	gsw_w32(PCE_TBL_VAL_3_VAL3_OFFSET,	\
		PCE_TBL_VAL_3_VAL3_SHIFT, PCE_TBL_VAL_3_VAL3_SIZE, pData->val[3]);
	gsw_w32(PCE_TBL_VAL_2_VAL2_OFFSET,	\
		PCE_TBL_VAL_2_VAL2_SHIFT, PCE_TBL_VAL_2_VAL2_SIZE, pData->val[2]);
	gsw_w32(PCE_TBL_VAL_1_VAL1_OFFSET,	\
		PCE_TBL_VAL_1_VAL1_SHIFT, PCE_TBL_VAL_1_VAL1_SIZE, pData->val[1]);
	gsw_w32(PCE_TBL_VAL_0_VAL0_OFFSET,	\
		PCE_TBL_VAL_0_VAL0_SHIFT, PCE_TBL_VAL_0_VAL0_SIZE, pData->val[0]);
	gsw_w32(PCE_TBL_CTRL_TYPE_OFFSET,	\
		PCE_TBL_CTRL_TYPE_SHIFT, PCE_TBL_CTRL_TYPE_SIZE, pData->type);
	gsw_w32(PCE_TBL_CTRL_VLD_OFFSET,	\
		PCE_TBL_CTRL_VLD_SHIFT, PCE_TBL_CTRL_VLD_SIZE, pData->valid);
	gsw_w32(PCE_TBL_CTRL_GMAP_OFFSET,	\
		PCE_TBL_CTRL_GMAP_SHIFT, PCE_TBL_CTRL_GMAP_SIZE, pData->group);
	gsw_w32(PCE_TBL_CTRL_BAS_OFFSET,	\
		PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, LTQ_TRUE);
	do {
		gsw_r32(PCE_TBL_CTRL_BAS_OFFSET, \
			PCE_TBL_CTRL_BAS_SHIFT, PCE_TBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	return LTQ_SUCCESS;
}

/* Packet Length Table write */
static int pce_tm_pkg_lng_tbl_write(pce_table_handle_t *pTmHandle, pce_pkt_length_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index;

	table_index = tbl_write(pTmHandle->pkg_lng_tbl, pTmHandle->pkg_lng_tbl_cnt,	\
		pPar, sizeof(pce_pkt_length_t), PCE_PKG_LNG_TBL_SIZE);
	PCE_ASSERT(table_index < 0);
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table	= PCE_PACKET_INDEX;
	pcetable.table_index = table_index;
	pcetable.key[0]	= pPar->pkg_lng;
	pcetable.mask		= pPar->pkg_lng_rng;
	pcetable.valid	= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}

/* Packet Length Table delete */
static int pce_tm_pkg_lng_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_PKG_LNG_TBL_SIZE);
	if (pTmHandle->pkg_lng_tbl_cnt[index] > 0)
		pTmHandle->pkg_lng_tbl_cnt[index]--;

	if (pTmHandle->pkg_lng_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->pkg_lng_tbl) + (index * sizeof(pce_pkt_length_t))),	\
		 0, sizeof(pce_pkt_length_t));
		/* initialize the data structure before using it */
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table = PCE_PACKET_INDEX;
		pcetable.table_index = index;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

/* Packet Length Table read */
static int pce_tm_pkg_lng_tbl_read(pce_table_handle_t *pTmHandle, int index, pce_pkt_length_t *pPar)
{
	PCE_ASSERT(index >= PCE_PKG_LNG_TBL_SIZE);
	memcpy(pPar, &pTmHandle->pkg_lng_tbl[index], sizeof(pce_pkt_length_t));
	return LTQ_SUCCESS;
}

/* MAC DA/SA Table index write */
static int pce_tm_dasa_mac_tbl_write(pce_table_handle_t *pTmHandle, pce_dasa_prog_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index;

	table_index = tbl_write(pTmHandle->dasa_mac_tbl, pTmHandle->dasa_mac_tbl_cnt, pPar, \
		sizeof(pce_dasa_prog_t), PCE_DASA_MAC_TBL_SIZE);
	PCE_ASSERT(table_index < 0);
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table = PCE_MAC_DASA_INDEX;
	pcetable.table_index = table_index;
	pcetable.key[0]		= (pPar->mac[4] << 8 | pPar->mac[5]);
	pcetable.key[1]		= (pPar->mac[2] << 8 | pPar->mac[3]);
	pcetable.key[2]		= (pPar->mac[0] << 8 | pPar->mac[1]);
	pcetable.mask			= pPar->mac_mask;
	pcetable.valid		= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}

/* MAC DA/SA Table delete */
static int pce_tm_dasa_mac_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_DASA_MAC_TBL_SIZE);
	if (pTmHandle->dasa_mac_tbl_cnt[index] > 0)
		pTmHandle->dasa_mac_tbl_cnt[index]--;

	if (pTmHandle->dasa_mac_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->dasa_mac_tbl) + (index * sizeof(pce_dasa_prog_t))),	\
		0, sizeof(pce_dasa_prog_t));
		/* initialize the data structure before using it */
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table				= PCE_MAC_DASA_INDEX;
		pcetable.table_index	= index;;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
   }
	return LTQ_SUCCESS;
}

/* MAC DA/SA Table Read */
static int pce_tm_dasa_mac_tbl_read(pce_table_handle_t *pTmHandle, int index,	\
		pce_dasa_prog_t *pPar)
{
	PCE_ASSERT(index >= PCE_DASA_MAC_TBL_SIZE);
	memcpy(pPar, &pTmHandle->dasa_mac_tbl[index], sizeof(pce_dasa_prog_t));
	return LTQ_SUCCESS;
}

/* Application Table write */
static int pce_tm_appl_tbl_write(pce_table_handle_t *pTmHandle, pce_app_prog_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index;

	table_index = tbl_write(pTmHandle->appl_tbl, pTmHandle->appl_tbl_cnt, pPar,	\
		sizeof(pce_app_prog_t), PCE_APPL_TBL_SIZE);
	PCE_ASSERT(table_index < 0);
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table	= PCE_APPLICATION_INDEX;
	pcetable.table_index	= table_index;
	pcetable.key[0]	= pPar->appl_data;
	pcetable.mask		= pPar->mask_range;
	pcetable.type		= pPar->mask_range_type;
	pcetable.valid	= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}

/* Application Table Delete */
static int pce_tm_appl_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_APPL_TBL_SIZE);
	if (pTmHandle->appl_tbl_cnt[index] > 0)
		pTmHandle->appl_tbl_cnt[index]--;

	if (pTmHandle->appl_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->appl_tbl) + (index * sizeof(pce_app_prog_t))),	\
		0, sizeof(pce_app_prog_t));
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table	= PCE_APPLICATION_INDEX;
		pcetable.table_index = index;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

/* Application Table Read */
static int pce_tm_appl_tbl_read(pce_table_handle_t *pTmHandle, int index, pce_app_prog_t *pPar)
{
	PCE_ASSERT(index >= PCE_APPL_TBL_SIZE);
	memcpy(pPar, &pTmHandle->appl_tbl[index], sizeof(pce_app_prog_t));
	return LTQ_SUCCESS;
}

/* IP DA/SA msb Table write */
int pce_tm_ip_dasa_msb_tbl_write(pce_table_handle_t *pTmHandle, pce_dasa_msb_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index, i;

	table_index = tbl_write(pTmHandle->ip_dasa_msb_tbl, pTmHandle->ip_dasa_msb_tbl_cnt, pPar,	\
		sizeof(pce_dasa_msb_t), PCE_IP_DASA_MSB_TBL_SIZE);
	if (table_index < 0)
		return table_index;

	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table				= PCE_IP_DASA_MSB_INDEX;
	pcetable.table_index	= table_index;
	for (i = 0; i < 4; i++)
		pcetable.key[i] = ((pPar->ip_msb[((i*2)+1)] << 8) | pPar->ip_msb[(i*2)]);
	pcetable.mask			= pPar->mask;
	pcetable.valid		= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}

/* IP DA/SA msb Table delete */
int pce_tm_ip_dasa_msb_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_IP_DASA_MSB_TBL_SIZE);
	if (pTmHandle->ip_dasa_msb_tbl_cnt[index] > 0)
		pTmHandle->ip_dasa_msb_tbl_cnt[index]--;

	if (pTmHandle->ip_dasa_msb_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->ip_dasa_msb_tbl) + (index * sizeof(pce_dasa_msb_t))),	\
		 0, sizeof(pce_dasa_msb_t));
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table				= PCE_IP_DASA_MSB_INDEX;
		pcetable.table_index	= index;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

/* IP DA/SA msb Table read */
static int pce_tm_ip_dasa_msb_tbl_read(pce_table_handle_t *pTmHandle, int index,	\
		pce_dasa_msb_t *pPar)
{
	PCE_ASSERT(index >= PCE_IP_DASA_MSB_TBL_SIZE);
	memcpy(pPar, &pTmHandle->ip_dasa_msb_tbl[index], sizeof(pce_dasa_msb_t));
	return LTQ_SUCCESS;
}

static int get_tbl_index(void *pTblStart, void *pPar, u32 TblEntrySize, u32 TblEntryNum)
{
	int i;
	/* search if the entry is already available and can be re-used */
	for (i = 0; i < TblEntryNum; i++) {
		/* entry is used, check if the entry content fits */
		if (memcmp(((char *)pTblStart) + i * TblEntrySize, pPar, (u8)TblEntrySize) == 0)
			return i;
	}
	return 0xFF;
}

/* Static Function Declaration */
int find_software_tbl_entry(pce_table_handle_t *pTmHandle, pce_dasa_lsb_t *pPar)
{
	return get_tbl_index(pTmHandle->ip_dasa_lsb_tbl, pPar,	\
		sizeof(pce_dasa_lsb_t), PCE_IP_DASA_LSB_TBL_SIZE);
}

/* Static Function Declaration */
int find_software_msb_tbl_entry(pce_table_handle_t *pTmHandle, pce_dasa_msb_t *pPar)
{
	return get_tbl_index(pTmHandle->ip_dasa_msb_tbl, pPar,	\
		sizeof(pce_dasa_msb_t), PCE_IP_DASA_MSB_TBL_SIZE);
}

/* IP DA/SA lsb Table Write */
int pce_tm_ip_dasa_lsb_tbl_write(pce_table_handle_t *pTmHandle, pce_dasa_lsb_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index, i;

	table_index = tbl_write(pTmHandle->ip_dasa_lsb_tbl, pTmHandle->ip_dasa_lsb_tbl_cnt, pPar,	\
		sizeof(pce_dasa_lsb_t), PCE_IP_DASA_LSB_TBL_SIZE);
	PCE_ASSERT(table_index < 0);
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table				= PCE_IP_DASA_LSB_INDEX;
	pcetable.table_index	= table_index;
	for (i = 0 ; i < 4 ; i++)
		pcetable.key[i]	= ((pPar->ip_lsb[((i*2)+1)] << 8) | pPar->ip_lsb[(i*2)]);
	pcetable.mask					= pPar->mask;
	pcetable.valid				= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}

/* IP DA/SA lsb Table delete */
int pce_tm_ip_dasa_lsb_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_IP_DASA_LSB_TBL_SIZE);
	if (pTmHandle->ip_dasa_lsb_tbl_cnt[index] > 0)
		pTmHandle->ip_dasa_lsb_tbl_cnt[index]--;
	if (pTmHandle->ip_dasa_lsb_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->ip_dasa_lsb_tbl) + (index * sizeof(pce_dasa_lsb_t))),	\
		0, sizeof(pce_dasa_lsb_t));
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table				= PCE_IP_DASA_LSB_INDEX;
		pcetable.table_index	= index;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

/* IP DA/SA lsb Table index delete */
int pce_tm_ip_dasa_lsb_tbl_idx_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	return tbl_idx_delete(pTmHandle->ip_dasa_lsb_tbl_cnt,	\
		index, PCE_IP_DASA_LSB_TBL_SIZE);
}

/* IP DA/SA msb Table index delete */
int pce_tm_ip_dasa_msb_tbl_idx_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	return tbl_idx_delete(pTmHandle->ip_dasa_msb_tbl_cnt,	\
		index, PCE_IP_DASA_MSB_TBL_SIZE);
}

/* IP DA/SA lsb Table read */
int pce_tm_ip_dasa_lsb_tbl_read(pce_table_handle_t *pTmHandle, int index,	\
		pce_dasa_lsb_t *pPar)
{
	PCE_ASSERT(index >= PCE_IP_DASA_LSB_TBL_SIZE);
	memcpy(pPar, &pTmHandle->ip_dasa_lsb_tbl[index], sizeof(pce_dasa_lsb_t));
	return LTQ_SUCCESS;
}

/* Protocal Table write */
static int pce_tm_ptcl_tbl_write(pce_table_handle_t *pTmHandle, pce_protocol_tbl_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index;

	table_index = tbl_write(pTmHandle->ptcl_tbl, pTmHandle->ptcl_tbl_cnt, pPar,	\
		sizeof(pce_protocol_tbl_t), PCE_PTCL_TBL_SIZE);
	PCE_ASSERT(table_index < 0);
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table				= PCE_PROTOCOL_INDEX;
	pcetable.table_index	= table_index;
	pcetable.key[0]				= pPar->key.ethertype;
	pcetable.mask					= pPar->mask.ethertype_mask;
	pcetable.valid				= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}
/* Get the vlan flow table index*/
int get_pce_tm_vlan_act_tbl_index(pce_table_handle_t *pTmHandle, u8 index)
{
	PCE_ASSERT(index >= PCE_VLAN_ACT_TBL_SIZE);
	if (pTmHandle->vlan_act_tbl_cnt[index] == 0)
		return LTQ_SUCCESS;
	else
		return LTQ_ERROR;
}

/* Protocal Table delete */
static int pce_tm_ptcl_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_PTCL_TBL_SIZE);
	if (pTmHandle->ptcl_tbl_cnt[index] > 0)
		pTmHandle->ptcl_tbl_cnt[index]--;

	if (pTmHandle->ptcl_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->ptcl_tbl) + (index * sizeof(pce_protocol_tbl_t))),	\
		0, sizeof(pce_protocol_tbl_t));
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table				= PCE_PROTOCOL_INDEX;
		pcetable.table_index	= index;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

/* Protocal Table Read */
static int pce_tm_ptcl_tbl_read(pce_table_handle_t *pTmHandle, int index, pce_protocol_tbl_t *pPar)
{
	PCE_ASSERT(index >= PCE_PTCL_TBL_SIZE);
	memcpy(pPar, &pTmHandle->ptcl_tbl[index], sizeof(pce_protocol_tbl_t));
	return LTQ_SUCCESS;
}

/* PPPoE Table Write */
static int pce_tm_pppoe_tbl_write(pce_table_handle_t *pTmHandle, pce_ppoe_tbl_t *pPar)
{
	void *pDevCtx = NULL;
	pce_table_prog_t pcetable;
	int table_index;

	table_index = tbl_write(pTmHandle->pppoe_tbl, pTmHandle->pppoe_tbl_cnt,	\
		pPar,	sizeof(pce_ppoe_tbl_t), PCE_PPPOE_TBL_SIZE);
	if (table_index < 0)
		return table_index;
	/* initialize the data structure before using it */
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	pcetable.table				= PCE_PPPOE_INDEX;
	pcetable.table_index	= table_index;
	pcetable.key[0]				= pPar->sess_id;
	pcetable.valid				= 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return table_index;
}

/* PPPoE Table Delete */
static int pce_tm_pppoe_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;

	PCE_ASSERT(index >= PCE_PPPOE_TBL_SIZE);
	if (pTmHandle->pppoe_tbl_cnt[index] > 0)
		pTmHandle->pppoe_tbl_cnt[index]--;
	if (pTmHandle->pppoe_tbl_cnt[index] == 0) {
		memset((((char *)pTmHandle->pppoe_tbl) + (index * sizeof(pce_ppoe_tbl_t))),	\
		0, sizeof(pce_ppoe_tbl_t));
		memset(&pcetable, 0, sizeof(pce_table_prog_t));
		pcetable.table				= PCE_PPPOE_INDEX;
		pcetable.table_index	= index;
		xwayflow_pce_table_write(pDevCtx, &pcetable);
	}
	return LTQ_SUCCESS;
}

/* PPPoE Table Read */
static int pce_tm_pppoe_tbl_read(pce_table_handle_t *pTmHandle,	\
	int index, pce_ppoe_tbl_t *pPar)
{
	PCE_ASSERT(index >= PCE_PPPOE_TBL_SIZE);
	memcpy(pPar, &pTmHandle->pppoe_tbl[index], sizeof(pce_ppoe_tbl_t));
	return LTQ_SUCCESS;
}

/* VLAN Table Delete */
static int pce_tm_vlan_act_tbl_delete(pce_table_handle_t *pTmHandle, u32 index)
{
	PCE_ASSERT(index >= PCE_VLAN_ACT_TBL_SIZE);
	if (pTmHandle->vlan_act_tbl_cnt[index] > 0)
		pTmHandle->vlan_act_tbl_cnt[index]--;
	return LTQ_SUCCESS;
}

static u32 ltq_active_VLAN_IdCreate(void *pDevCtx, u16 nVId)
{
	pce_table_prog_t pcetable;
	u32	index, vid_index = 0x7F;
	for (index = 0; index < PCE_VLAN_ACT_TBL_SIZE; index++) {
		memset(&pcetable, 0 , sizeof(pce_table_prog_t));
		pcetable.table		= PCE_ACTVLAN_INDEX;
		pcetable.table_index = index;
		xwayflow_pce_table_read(pDevCtx, &pcetable);
		if (pcetable.valid == LTQ_FALSE) {
			pcetable.table_index	= index;
			vid_index				= index;
			pcetable.table	= PCE_ACTVLAN_INDEX;
			pcetable.key[0]	= nVId;
			pcetable.valid	= 1;
			xwayflow_pce_table_write(pDevCtx, &pcetable);
			break;
		}
	}
	return vid_index;
}

static int pce_tm_find_vlan_id_index(pce_table_handle_t *pTmHandle, u16 vid)
{
	pce_table_prog_t pcetable_vlan;
	int	index, vid_index = 0x7F;
	void *pDevCtx = NULL;

	for (index = 0; index < PCE_VLAN_ACT_TBL_SIZE; index++) {
		pcetable_vlan.table				= PCE_ACTVLAN_INDEX;
		pcetable_vlan.table_index	= index;
		xwayflow_pce_table_read(pDevCtx, &pcetable_vlan);
		if (pcetable_vlan.valid == LTQ_TRUE) {
			if (pcetable_vlan.key[0] == vid) {
				vid_index = index;
				pTmHandle->vlan_act_tbl_cnt[index]++;
				break;
			}
		}
	}
	return vid_index;
}

static int pce_tm_find_vlan_id_fd_index(pce_table_handle_t *pTmHandle, \
	vlan_active_table_t *VLAN_Table_Entry)
{
	pce_table_prog_t pcetable_vlan;
	int	index, vid_index = 0x7F;
	void *pDevCtx = NULL;

	for (index = 0; index < PCE_VLAN_ACT_TBL_SIZE; index++) {
		pcetable_vlan.table = PCE_ACTVLAN_INDEX;
		pcetable_vlan.table_index	= index;
		xwayflow_pce_table_read(pDevCtx, &pcetable_vlan);
		if (pcetable_vlan.valid	== LTQ_TRUE) {
			if (pcetable_vlan.key[0]	== VLAN_Table_Entry->vid) {
				vid_index							= index;
				VLAN_Table_Entry->fid	= (pcetable_vlan.val[0] & 0xFF);
				pTmHandle->vlan_act_tbl_cnt[index]++;
				break;
			}
		}
	}
	return vid_index;
}

/* PCE Table Init routine */
int pce_table_init(ltq_pce_table_t *pPCEHandle)
{
	int i;
	PCE_ASSERT(pPCEHandle == NULL);
	memset(&pPCEHandle->pce_sub_tbl, 0, sizeof(pce_table_handle_t));
	memset(&pPCEHandle->pce_tbl, 0, sizeof(pce_table_t));
	memset(&pPCEHandle->pce_act, 0, sizeof(IFX_FLOW_PCE_action_t));
	for (i = 0; i < PCE_TABLE_SIZE; i++)
		pPCEHandle->pce_tbl_used[i] = 0;
	return LTQ_SUCCESS;
}

/* PCE Table Micro Code Init routine */
ethsw_ret_t ethsw_pce_micro_code_init(void *pDevCtx)
{
	ethsw_api_dev_t *pEthSWDev = (ethsw_api_dev_t *)pDevCtx;
	pce_table_prog_t tbl_entry;
	u8 i, j;

	/* Disable all physical port  */
	for (j = 0; j < pEthSWDev->nPortNumber; j++) {
		gsw_w32((FDMA_PCTRL_EN_OFFSET + (j * 0x6)),	\
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 0);
		gsw_w32((SDMA_PCTRL_PEN_OFFSET + (j * 0x6)),	\
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 0);
	}
	gsw_w32((GLOB_CTRL_SE_OFFSET + 0xC40),	\
		GLOB_CTRL_SE_SHIFT, GLOB_CTRL_SE_SIZE, 1);
	/* Download the microcode  */
	for (i = 0; i < PCE_MICRO_TABLE_SIZE; i++) {
		memset(&tbl_entry, 0, sizeof(tbl_entry));
		tbl_entry.val[3] = pce_mc_max_ifx_tag_m[i].val_3;
		tbl_entry.val[2] = pce_mc_max_ifx_tag_m[i].val_2;
		tbl_entry.val[1] = pce_mc_max_ifx_tag_m[i].val_1;
		tbl_entry.val[0] = pce_mc_max_ifx_tag_m[i].val_0;
		tbl_entry.table_index = i;
		tbl_entry.table = PCE_PARS_INDEX;
		xwayflow_pce_table_write(pEthSWDev, &tbl_entry);
	}
	gsw_w32(PCE_GCTRL_0_MC_VALID_OFFSET,	\
		PCE_GCTRL_0_MC_VALID_SHIFT, PCE_GCTRL_0_MC_VALID_SIZE, 0x1);
	gsw_w32(PCE_PMAP_2_DMCPMAP_OFFSET,	\
		PCE_PMAP_2_DMCPMAP_SHIFT, PCE_PMAP_2_DMCPMAP_SIZE, 0x7F);
	gsw_w32(PCE_PMAP_3_UUCMAP_OFFSET,	\
		PCE_PMAP_3_UUCMAP_SHIFT, PCE_PMAP_3_UUCMAP_SIZE, 0x7F);
	/* Enable RMON Counter for all ports */
	for (j = 0; j < pEthSWDev->nPortNumber; j++) {
		gsw_w32((BM_PCFG_CNTEN_OFFSET + (j * 0x2)),	\
			BM_PCFG_CNTEN_SHIFT, BM_PCFG_CNTEN_SIZE, 1);
	}
	gsw_w32(BM_QUEUE_GCTRL_GL_MOD_OFFSET,	\
		BM_QUEUE_GCTRL_GL_MOD_SHIFT, BM_QUEUE_GCTRL_GL_MOD_SIZE, 0);
	return LTQ_SUCCESS;
}


int pce_action_delete(ltq_pce_table_t *pHandle, u32 index)
{
	pce_table_prog_t pcetable;
	void *pDevCtx = NULL;
	PCE_ASSERT(index >= PCE_TABLE_SIZE);
	memset(&pHandle->pce_act[index], 0, sizeof(IFX_FLOW_PCE_action_t));
	/* REMOVE RULE ACTION FROM HARDWARE DEVICE */
	pcetable.table_index = index;    /*index of the Traffic Flow Table index configuration */
	pcetable.table = PCE_TFLOW_INDEX;
	xwayflow_pce_table_read(pDevCtx, &pcetable);
	if (pcetable.valid == LTQ_TRUE) {
		if (((pcetable.val[0] >> 1) & 0x1) == LTQ_TRUE) {
			u32 index = (pcetable.val[2] & 0x3F) ;
			if (pHandle->pce_sub_tbl.vlan_act_tbl_cnt[index] > 0)
				pHandle->pce_sub_tbl.vlan_act_tbl_cnt[index]--;
		}
	}
	pcetable.valid = 0;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
	return LTQ_SUCCESS;
}

int pce_pattern_delete(ltq_pce_table_t *pHandle, u32 index)
{
	pce_table_t  *pFet;

	PCE_ASSERT(index >= PCE_TABLE_SIZE);
	/* Check if an entry is currently programmed and remove that one. */
	if (pHandle->pce_tbl_used[index] == 0)
		return LTQ_SUCCESS;
	pFet = &(pHandle->pce_tbl[index]);
#define IFX_PCE_TM_IDX_DELETE(x, y, z) if (x != y) \
	if (0 != z(&pHandle->pce_sub_tbl, y)) IFX_RETURN_PCE;
	/* Packet length */
	IFX_PCE_TM_IDX_DELETE(0x1F, pFet->pkt_lng_idx, pce_tm_pkg_lng_tbl_delete)
	/* Destination MAC address */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->dst_mac_addr_idx, pce_tm_dasa_mac_tbl_delete)
	/* Source MAC address */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->src_mac_addr_idx, pce_tm_dasa_mac_tbl_delete)
	/* Destination Application field */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->dst_appl_fld_idx, pce_tm_appl_tbl_delete)
	/* Source Application field */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->src_appl_fld_idx, pce_tm_appl_tbl_delete)
	/* DIP MSB */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->dip_msb_idx, pce_tm_ip_dasa_msb_tbl_delete)
	/* DIP LSB */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->dip_lsb_idx, pce_tm_ip_dasa_lsb_tbl_delete)
	/* SIP MSB */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->sip_msb_idx, pce_tm_ip_dasa_msb_tbl_delete)
	/* SIP LSB */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->sip_lsb_idx, pce_tm_ip_dasa_lsb_tbl_delete)
	/* IP protocol */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->ip_prot_idx, pce_tm_ptcl_tbl_delete)
	/* Ethertype */
	IFX_PCE_TM_IDX_DELETE(0xFF, pFet->ethertype_idx, pce_tm_ptcl_tbl_delete)
	/* PPPoE */
	IFX_PCE_TM_IDX_DELETE(0x1F, pFet->pppoe_idx, pce_tm_pppoe_tbl_delete)
   /* VLAN */
	IFX_PCE_TM_IDX_DELETE(0x7F, pFet->vlan_idx, pce_tm_vlan_act_tbl_delete)
	 /* SVLAN */
	IFX_PCE_TM_IDX_DELETE(0x7F, pFet->svlan_idx, pce_tm_vlan_act_tbl_delete)
	/* reset the flag that this rule line is used */
	pHandle->pce_tbl_used[index] = 0;
	/* reset the rule line */
	memset(pFet, 0xFF, sizeof(pce_table_t));
	/* WRITE DOWN THE NEW LINE TO THE HARDWARE */
	/* also remove the action for this rule */
	return pce_action_delete(pHandle, index);
}

int pce_rule_read(ltq_pce_table_t *pHandle, IFX_FLOW_PCE_rule_t *pPar)
{
	u32   i, idx = pPar->pattern.nIndex;
	pce_table_t  *pFet;

	PCE_ASSERT(idx >= PCE_TABLE_SIZE);
	/* initialize to zero the structure before writing the parameters */
	memset(pPar, 0, sizeof(IFX_FLOW_PCE_rule_t));
	pPar->pattern.nIndex = idx;
	if (pHandle->pce_tbl_used[idx] == 0)
		return LTQ_SUCCESS;
	else
		pPar->pattern.bEnable = LTQ_TRUE;

	pFet = &(pHandle->pce_tbl[idx]);
	/* Port ID */
	if (pFet->port_id == 0xFF)
		pPar->pattern.nPortId = 0;
	else
		pPar->pattern.nPortId = (u8)pFet->port_id;
	/* Port ID is assigned then the Port ID is Enable */
	if (pFet->port_id != 0xFF)
		pPar->pattern.bPortIdEnable = LTQ_TRUE;
	/* todo: check how pPar->bDscp should be used or is needed */
	/* DSCP value */
	if (pFet->dscp == 0x7F)
		pPar->pattern.nDSCP = 0;
	else
		pPar->pattern.nDSCP = (u8)pFet->dscp;
	/* DSCP is assigned and Enable */
	if (pFet->dscp != 0x7F)
		pPar->pattern.bDSCP_Enable = LTQ_TRUE;
	/* todo: check how pPar->bPcp should be used or is needed */
	/* PCP value */
	if (pFet->pcp == 0xF)
		pPar->pattern.nPCP = 0;
	else
		pPar->pattern.nPCP = (u8)pFet->pcp;
	/* PCP is assigned and Enable */
	if (pFet->pcp != 0xF)
		pPar->pattern.bPCP_Enable = LTQ_TRUE;
		/* ETC */
		/* nSTAG_PCP_DEI value */
	if (pFet->stag_pcp_dei == 0x1F)
		pPar->pattern.nSTAG_PCP_DEI = 0;
	else
		pPar->pattern.nSTAG_PCP_DEI = (u8)pFet->stag_pcp_dei;
		/* PCP is assigned and Enable */
	if (pFet->stag_pcp_dei != 0x1F)
		pPar->pattern.bSTAG_PCP_DEI_Enable = LTQ_TRUE;
	if (pFet->pkt_lng_idx != 0x1F) {
		pce_pkt_length_t pkg_lng_tbl;
		memset(&pkg_lng_tbl, 0 , sizeof(pce_pkt_length_t));
		/* Packet length used */
		pPar->pattern.bPktLngEnable = LTQ_TRUE;
		if (LTQ_SUCCESS != pce_tm_pkg_lng_tbl_read(&pHandle->pce_sub_tbl, pFet->pkt_lng_idx, &pkg_lng_tbl))
			IFX_RETURN_PCE;
		/* Packet length */
		pPar->pattern.nPktLng = pkg_lng_tbl.pkg_lng;
		/* Packet length Range */
		pPar->pattern.nPktLngRange = pkg_lng_tbl.pkg_lng_rng;
	}
	if (pFet->dst_mac_addr_idx != 0xFF) {
		pce_dasa_prog_t dasa_mac_tbl;
		memset(&dasa_mac_tbl, 0 , sizeof(pce_dasa_prog_t));
		if (LTQ_SUCCESS != pce_tm_dasa_mac_tbl_read(&pHandle->pce_sub_tbl, pFet->dst_mac_addr_idx, &dasa_mac_tbl))
			IFX_RETURN_PCE;
			/* Destination MAC address used */
			pPar->pattern.bMAC_DstEnable = LTQ_TRUE;

		/* Destination MAC address */
		for (i = 0; i < 6; i++)
			pPar->pattern.nMAC_Dst[i] = dasa_mac_tbl.mac[i];

      /* Destination MAC address mask */
		pPar->pattern.nMAC_DstMask = dasa_mac_tbl.mac_mask;
	}
	if (pFet->src_mac_addr_idx != 0xFF) {
		pce_dasa_prog_t dasa_mac_tbl;
		memset(&dasa_mac_tbl, 0 , sizeof(pce_dasa_prog_t));
		if (LTQ_SUCCESS != pce_tm_dasa_mac_tbl_read(&pHandle->pce_sub_tbl, pFet->src_mac_addr_idx, &dasa_mac_tbl))
			IFX_RETURN_PCE;
		/* Destination MAC address used */
		pPar->pattern.bMAC_SrcEnable = LTQ_TRUE;
		/* Destination MAC address */
		for (i = 0; i < 6; i++)
			pPar->pattern.nMAC_Src[i] = dasa_mac_tbl.mac[i];

		/* Destination MAC address mask */
		pPar->pattern.nMAC_SrcMask = dasa_mac_tbl.mac_mask;
	}
	if (pFet->dst_appl_fld_idx != 0xFF) {
		pce_app_prog_t appl_tbl;
		memset(&appl_tbl, 0 , sizeof(pce_app_prog_t));
		if (LTQ_SUCCESS != pce_tm_appl_tbl_read(&pHandle->pce_sub_tbl, pFet->dst_appl_fld_idx, &appl_tbl))
			IFX_RETURN_PCE;
		/* Destination Application used */
		pPar->pattern.bAppDataMSB_Enable = LTQ_TRUE;
		/* Destination Application field */
		pPar->pattern.nAppDataMSB = appl_tbl.appl_data;
		/* Destination Application mask/range used */
		pPar->pattern.bAppMaskRangeMSB_Select = appl_tbl.mask_range_type;
		/* Destination Application mask/range */
		pPar->pattern.nAppMaskRangeMSB = appl_tbl.mask_range;
	}
	if (pFet->src_appl_fld_idx != 0xFF) {
		pce_app_prog_t appl_tbl;
		memset(&appl_tbl, 0 , sizeof(pce_app_prog_t));
		if (LTQ_SUCCESS != pce_tm_appl_tbl_read(&pHandle->pce_sub_tbl, pFet->src_appl_fld_idx, &appl_tbl))
			IFX_RETURN_PCE;
		/* Source Application used */
		pPar->pattern.bAppDataLSB_Enable = LTQ_TRUE;
		/* Source Application field */
		pPar->pattern.nAppDataLSB = appl_tbl.appl_data;
		/* Destination Application mask/range used */
		pPar->pattern.bAppMaskRangeLSB_Select = appl_tbl.mask_range_type;
		/* Source Application mask/range */
		pPar->pattern.nAppMaskRangeLSB = appl_tbl.mask_range;
	}

	if ((pFet->dip_msb_idx != 0xFF) && (pFet->dip_lsb_idx != 0xFF)) { /*for IPv6  */
		pce_dasa_msb_t dasa_tbl;
		pce_dasa_lsb_t dasa_tbl_lsb;
		int i, j;
		memset(&dasa_tbl, 0 , sizeof(pce_dasa_msb_t));
		memset(&dasa_tbl_lsb, 0 , sizeof(pce_dasa_lsb_t));
		/* DIP MSB used */
		pPar->pattern.eDstIP_Select = IFX_FLOW_PCE_IP_V6;
		if (LTQ_SUCCESS != pce_tm_ip_dasa_msb_tbl_read(&pHandle->pce_sub_tbl, pFet->dip_msb_idx, &dasa_tbl))
			IFX_RETURN_PCE;
		/* First, search for DIP in the DA/SA table (DIP MSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2)
			pPar->pattern.nDstIP.nIPv6[i]	= (((dasa_tbl.ip_msb[j] & 0xFF) << 8) | (dasa_tbl.ip_msb[j-1] & 0xFF));

		/* DIP MSB Nibble Mask */
		pPar->pattern.nDstIP_Mask = (dasa_tbl.mask & 0xFFFF) << 16;
		/* DIP LSB used */
		pPar->pattern.eDstIP_Select = IFX_FLOW_PCE_IP_V6;
		if (LTQ_SUCCESS != pce_tm_ip_dasa_lsb_tbl_read(&pHandle->pce_sub_tbl, pFet->dip_lsb_idx, &dasa_tbl_lsb))
			IFX_RETURN_PCE;
		for (i = 0, j = 7; i < 4; i++, j -= 2)
			pPar->pattern.nDstIP.nIPv6[i+4]	= (((dasa_tbl_lsb.ip_lsb[j] & 0xFF) << 8) | (dasa_tbl_lsb.ip_lsb[j-1] & 0xFF));

		pPar->pattern.nDstIP_Mask |= dasa_tbl_lsb.mask & 0xFFFF;
	} else if (pFet->dip_lsb_idx != 0xFF) {  /*For IPv4 only */
		pce_dasa_lsb_t dasa_tbl;
		memset(&dasa_tbl, 0 , sizeof(pce_dasa_lsb_t));
		/* DIP LSB used */
		pPar->pattern.eDstIP_Select = IFX_FLOW_PCE_IP_V4;
		if (LTQ_SUCCESS != pce_tm_ip_dasa_lsb_tbl_read(&pHandle->pce_sub_tbl, pFet->dip_lsb_idx, &dasa_tbl))
			IFX_RETURN_PCE;
		/* DIP LSB */
		pPar->pattern.nDstIP.nIPv4 = (dasa_tbl.ip_lsb[0] | (dasa_tbl.ip_lsb[1] << 8) \
									 | (dasa_tbl.ip_lsb[2] << 16) | (dasa_tbl.ip_lsb[3] << 24));

      /* DIP LSB Nibble Mask */
		pPar->pattern.nDstIP_Mask = dasa_tbl.mask;
	}
	if ((pFet->sip_msb_idx != 0xFF) && (pFet->sip_lsb_idx != 0xFF)) { /* for IPv6 */
		pce_dasa_msb_t dasa_tbl;
		pce_dasa_lsb_t dasa_tbl_lsb;
		int i, j;
		memset(&dasa_tbl, 0 , sizeof(pce_dasa_msb_t));
		memset(&dasa_tbl_lsb, 0 , sizeof(pce_dasa_lsb_t));
		/* SIP MSB used */
		pPar->pattern.eSrcIP_Select = IFX_FLOW_PCE_IP_V6;
		if (LTQ_SUCCESS != pce_tm_ip_dasa_msb_tbl_read(&pHandle->pce_sub_tbl, pFet->sip_msb_idx, &dasa_tbl))
			IFX_RETURN_PCE;
		for (i = 0, j = 7; i < 4; i++, j -= 2)
			pPar->pattern.nSrcIP.nIPv6[i]	= (((dasa_tbl.ip_msb[j] & 0xFF) << 8) | (dasa_tbl.ip_msb[j-1] & 0xFF));

		/* SIP MSB Nibble Mask */
		pPar->pattern.nSrcIP_Mask = (dasa_tbl.mask & 0xFFFF) << 16;
		/* SIP LSB used */
		pPar->pattern.eSrcIP_Select = IFX_FLOW_PCE_IP_V6;
		if (LTQ_SUCCESS != pce_tm_ip_dasa_lsb_tbl_read(&pHandle->pce_sub_tbl, pFet->sip_lsb_idx, &dasa_tbl_lsb))
			IFX_RETURN_PCE;
		for (i = 0, j = 7; i < 4; i++, j -= 2)
			pPar->pattern.nSrcIP.nIPv6[i+4]	= (((dasa_tbl_lsb.ip_lsb[j] & 0xFF) << 8) | (dasa_tbl_lsb.ip_lsb[j-1] & 0xFF));

		/* SIP LSB Nibble Mask */
		pPar->pattern.nSrcIP_Mask |= dasa_tbl_lsb.mask & 0xFFFF;
	} else if (pFet->sip_lsb_idx != 0xFF) { /* for IPv4 only */
		pce_dasa_lsb_t dasa_tbl;
		memset(&dasa_tbl, 0 , sizeof(pce_dasa_lsb_t));
		/* SIP LSB used */
		pPar->pattern.eSrcIP_Select = IFX_FLOW_PCE_IP_V4;
		if (LTQ_SUCCESS != pce_tm_ip_dasa_lsb_tbl_read(&pHandle->pce_sub_tbl, pFet->sip_lsb_idx, &dasa_tbl))
			IFX_RETURN_PCE;
		/* SIP LSB */
		pPar->pattern.nSrcIP.nIPv4 = (dasa_tbl.ip_lsb[0] | (dasa_tbl.ip_lsb[1] << 8) \
									| (dasa_tbl.ip_lsb[2] << 16) | (dasa_tbl.ip_lsb[3] << 24));

      /* SIP LSB Nibble Mask */
      pPar->pattern.nSrcIP_Mask = dasa_tbl.mask;
   }
	if (pFet->ethertype_idx != 0xFF) {
		pce_protocol_tbl_t   pctl_tbl;
		memset(&pctl_tbl, 0 , sizeof(pce_protocol_tbl_t));
		/* Ethertype used */
		pPar->pattern.bEtherTypeEnable = LTQ_TRUE;
		if (LTQ_SUCCESS != pce_tm_ptcl_tbl_read(&pHandle->pce_sub_tbl, pFet->ethertype_idx, &pctl_tbl))
			IFX_RETURN_PCE;
		/* Ethertype */
		pPar->pattern.nEtherType = pctl_tbl.key.ethertype;
		/* Ethertype Mask */
		pPar->pattern.nEtherTypeMask = pctl_tbl.mask.ethertype_mask;
	}
	if (pFet->ip_prot_idx != 0xFF) {
		pce_protocol_tbl_t   pctl_tbl;
		memset(&pctl_tbl, 0 , sizeof(pce_protocol_tbl_t));
		/* IP protocol used */
		pPar->pattern.bProtocolEnable = LTQ_TRUE;
		if (LTQ_SUCCESS != pce_tm_ptcl_tbl_read(&pHandle->pce_sub_tbl, pFet->ip_prot_idx, &pctl_tbl))
			IFX_RETURN_PCE;
		/* IP protocol */
		pPar->pattern.nProtocol = (u8)pctl_tbl.key.prot.protocol;
		/* IP protocol Mask */
		pPar->pattern.nProtocolMask = (u8)pctl_tbl.mask.prot.protocol_mask;
	}
	if (pFet->pppoe_idx != 0x1F) {
		pce_ppoe_tbl_t pppoe_tbl;
		memset(&pppoe_tbl, 0 , sizeof(pce_ppoe_tbl_t));
		/* PPPoE used */
		pPar->pattern.bSessionIdEnable = LTQ_TRUE;
		if (LTQ_SUCCESS != pce_tm_pppoe_tbl_read(&pHandle->pce_sub_tbl, pFet->pppoe_idx, &pppoe_tbl))
			IFX_RETURN_PCE;
		/* PPPoE */
		pPar->pattern.nSessionId = pppoe_tbl.sess_id;
	}
	if (pFet->vlan_idx != 0x7F) {
		pce_table_prog_t pcetable_vlan;
		void *pDevCtx = NULL;
		memset(&pcetable_vlan, 0 , sizeof(pce_table_prog_t));
		/* VLAN used */
		pPar->pattern.bVid = LTQ_TRUE;
		pcetable_vlan.table = PCE_ACTVLAN_INDEX;
		pcetable_vlan.table_index = pFet->vlan_idx; /*index of the VLAN ID configuration */
		/* pcetable.key[0] = pPar->nVId; */
		xwayflow_pce_table_read(pDevCtx, &pcetable_vlan);
		if (pcetable_vlan.valid == LTQ_TRUE)
			pPar->pattern.nVid = pcetable_vlan.key[0] & 0xFFF;
	}
		if (pFet->svlan_idx != 0x7F) {
		pce_table_prog_t pcetable_vlan;
		void *pDevCtx = NULL;
		memset(&pcetable_vlan, 0 , sizeof(pce_table_prog_t));
		/* VLAN used */
		pPar->pattern.bSLAN_Vid = LTQ_TRUE;
		pcetable_vlan.table = PCE_ACTVLAN_INDEX;
		pcetable_vlan.table_index = pFet->svlan_idx; /*index of the VLAN ID configuration */
		/* pcetable.key[0] = pPar->nVId; */
		xwayflow_pce_table_read(pDevCtx, &pcetable_vlan);
		if (pcetable_vlan.valid == LTQ_TRUE)
			pPar->pattern.nSLAN_Vid = pcetable_vlan.key[0] & 0xFFF;
	}
	/* Copy whole action table into action structure */
	memcpy(&pPar->action, &pHandle->pce_act[idx], sizeof(IFX_FLOW_PCE_action_t));
	return LTQ_SUCCESS;
}

int pce_rule_write(ltq_pce_table_t *pHandle, IFX_FLOW_PCE_rule_t *pPar)
{
	pce_table_prog_t pcetable;
	u32   idx = pPar->pattern.nIndex;
	void *pDevCtx = NULL;
	pce_table_t  *pFet;
	IFX_FLOW_PCE_action_t *pAct;
	IFX_FLOW_PCE_rule_t	key_current;
	int	tab_index, i, reg_val;

	PCE_ASSERT(idx >= PCE_TABLE_SIZE);

	if (pPar->pattern.bEnable == LTQ_FALSE) {
		/* Entry to delete. */
		return pce_pattern_delete(pHandle, idx);
	}
	/* first read the current value context from the table to compare if
	 the new configuration is the same. */
	key_current.pattern.nIndex = idx;
	/* This key is already programmed on the hardware, no need to update
	the current hardware programming. */
	if (pce_rule_read(pHandle, &key_current) != LTQ_SUCCESS)
		IFX_RETURN_PCE;
	if (!memcmp(&key_current, pPar, sizeof(IFX_FLOW_PCE_rule_t)))
		return 0;
	memset(&pcetable, 0, sizeof(pce_table_prog_t));
	gsw_r32(PCE_GCTRL_1_VLANMD_OFFSET,	\
				PCE_GCTRL_1_VLANMD_SHIFT,	PCE_GCTRL_1_VLANMD_SIZE, &reg_val);
	/* Delete the old entry before adding the new one. */
	if (pHandle->pce_tbl_used[idx] != 0) {
		if (pce_pattern_delete(pHandle, idx) != 0) IFX_RETURN_PCE;
	}
	pcetable.key[8] = 0xFFFF;
	/* Mark the entry as taken already and then program it. */
	pHandle->pce_tbl_used[idx] = 1;
	/* Now convert the parameter and add to the table. */
	pFet = &(pHandle->pce_tbl[idx]);
	/* Port ID */
	if (pPar->pattern.bPortIdEnable == 1)
		pFet->port_id = pPar->pattern.nPortId;
	else
		pFet->port_id = 0xFF;
	pcetable.key[0] = pFet->port_id;
	/* DSCP value used */
	if (pPar->pattern.bDSCP_Enable == LTQ_TRUE)
		pFet->dscp = pPar->pattern.nDSCP;
	else
		pFet->dscp = 0x7F;

	pcetable.key[6] |= (pFet->dscp << 8);
	/* PCP value used */
	if (pPar->pattern.bPCP_Enable == LTQ_TRUE)
		pFet->pcp = pPar->pattern.nPCP;
	else
		pFet->pcp = 0xF;
	pcetable.key[6] |=  pFet->pcp;
	/* ETC */
	/* PCP DEI value used */
	if (pPar->pattern.bSTAG_PCP_DEI_Enable == LTQ_TRUE) {
		/* PCP value */
		pFet->stag_pcp_dei = pPar->pattern.nSTAG_PCP_DEI;
	} else {
		pFet->stag_pcp_dei = 0x1F;
	}
	pcetable.key[8] &= ~(0x1F);
	pcetable.key[8] |=  pFet->stag_pcp_dei;

	/* Packet length used */
	if (pPar->pattern.bPktLngEnable == LTQ_TRUE) {
		pce_pkt_length_t   pkg_lng;
		pkg_lng.pkg_lng = pPar->pattern.nPktLng; /* Packet length */
		pkg_lng.pkg_lng_rng = pPar->pattern.nPktLngRange; /* Packet length range, in number of bytes */
		tab_index = pce_tm_pkg_lng_tbl_write(&pHandle->pce_sub_tbl, &pkg_lng);
		if (tab_index < 0)
			return tab_index ;
		pFet->pkt_lng_idx = tab_index;
	} else {
		pFet->pkt_lng_idx =  0x1F;
	}
	pcetable.key[7] = pFet->pkt_lng_idx;
	/* Destination MAC address used */
	if (pPar->pattern.bMAC_DstEnable == LTQ_TRUE) {
		pce_dasa_prog_t  dasa_mac_tbl;
		for (i = 0; i < 6; i++)
			dasa_mac_tbl.mac[i] = pPar->pattern.nMAC_Dst[i]; /* Destination MAC address */
		dasa_mac_tbl.mac_mask = pPar->pattern.nMAC_DstMask; /* Destination MAC address mask */
		tab_index = pce_tm_dasa_mac_tbl_write(&pHandle->pce_sub_tbl, &dasa_mac_tbl);
		if (tab_index < 0)
			return tab_index;
		pFet->dst_mac_addr_idx = tab_index;
	} else {
		pFet->dst_mac_addr_idx = 0xFF;
	}
	pcetable.key[5] |=  (pFet->dst_mac_addr_idx << 8);
	/* Source MAC address used */
	if (pPar->pattern.bMAC_SrcEnable == LTQ_TRUE) {
		pce_dasa_prog_t  dasa_mac_tbl;
		for (i = 0; i < 6; i++)
			dasa_mac_tbl.mac[i] = pPar->pattern.nMAC_Src[i];
		/* Source MAC address mask */
		dasa_mac_tbl.mac_mask = pPar->pattern.nMAC_SrcMask;
		tab_index = pce_tm_dasa_mac_tbl_write(&pHandle->pce_sub_tbl, &dasa_mac_tbl);
		if (tab_index < 0)
			return tab_index;

		pFet->src_mac_addr_idx = tab_index;
	}  else {
		pFet->src_mac_addr_idx = 0xFF;
	}
	pcetable.key[5] |= pFet->src_mac_addr_idx;
	/* Destination Application used */
	if (pPar->pattern.bAppDataMSB_Enable == LTQ_TRUE) {
		pce_app_prog_t   appl_tbl;
		appl_tbl.mask_range_type = pPar->pattern.bAppMaskRangeMSB_Select;
		appl_tbl.appl_data = pPar->pattern.nAppDataMSB; /* Destination Application field */
		appl_tbl.mask_range = pPar->pattern.nAppMaskRangeMSB;  /* Destination Application mask/range */
		tab_index = pce_tm_appl_tbl_write(&pHandle->pce_sub_tbl, &appl_tbl);
		if (tab_index < 0)
			return tab_index;
		pFet->dst_appl_fld_idx = tab_index;
	} else {
		pFet->dst_appl_fld_idx = 0xFF;
	}
	pcetable.key[4] |= pFet->dst_appl_fld_idx;
	/* Source Application field used */
	if (pPar->pattern.bAppDataLSB_Enable == LTQ_TRUE) {
		pce_app_prog_t   appl_tbl;
		appl_tbl.mask_range_type = pPar->pattern.bAppMaskRangeLSB_Select;
		appl_tbl.appl_data = pPar->pattern.nAppDataLSB; /* Source Application field */
		appl_tbl.mask_range = pPar->pattern.nAppMaskRangeLSB;  /* Source Application mask/range */
		tab_index = pce_tm_appl_tbl_write(&pHandle->pce_sub_tbl, &appl_tbl);
		if (tab_index < 0)
			return tab_index;
		pFet->src_appl_fld_idx = tab_index;
	} else {
		pFet->src_appl_fld_idx = 0xFF;
	}
	pcetable.key[4] |= (pFet->src_appl_fld_idx << 8);

	pFet->dip_msb_idx = 0xFF;
	pFet->dip_lsb_idx = 0xFF;
	/* DIP MSB used */
	if (pPar->pattern.eDstIP_Select == /*2*/IFX_FLOW_PCE_IP_V6) {
		pce_dasa_msb_t  dasa_msb_tbl;
		pce_dasa_lsb_t  dasa_lsb_tbl;
		int	j;
		/* First, search for DIP in the DA/SA table (DIP MSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_msb_tbl.ip_msb[j-1] = (pPar->pattern.nDstIP.nIPv6[i] & 0xFF);
			dasa_msb_tbl.ip_msb[j]	= ((pPar->pattern.nDstIP.nIPv6[i] >> 8) & 0xFF);
		}
		dasa_msb_tbl.mask = (u16) ((pPar->pattern.nDstIP_Mask >> 16) & 0xFFFF);
		tab_index = find_software_msb_tbl_entry(&pHandle->pce_sub_tbl, &dasa_msb_tbl);
		if (tab_index < 0) {
			pr_err("%s:%s:%d (DASA Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		if (tab_index == 0xFF)
			tab_index = pce_tm_ip_dasa_msb_tbl_write(&pHandle->pce_sub_tbl, &dasa_msb_tbl);

		pFet->dip_msb_idx = tab_index;

		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_lsb_tbl.ip_lsb[j-1] = (pPar->pattern.nDstIP.nIPv6[i+4] & 0xFF);
			dasa_lsb_tbl.ip_lsb[j]	= ((pPar->pattern.nDstIP.nIPv6[i+4] >> 8) & 0xFF);
		}
		dasa_lsb_tbl.mask = (u16) (pPar->pattern.nDstIP_Mask & 0xFFFF);
		tab_index = find_software_tbl_entry(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (tab_index < 0) {
			pr_err("%s:%s:%d (DASA Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		if (tab_index == 0xFF)
			tab_index = pce_tm_ip_dasa_lsb_tbl_write(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);

		pFet->dip_lsb_idx = (u16)tab_index;
	} else  if (pPar->pattern.eDstIP_Select == IFX_FLOW_PCE_IP_V4) { /* DIP LSB used */
		pce_dasa_lsb_t  dasa_lsb_tbl;

		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0; i < 4 ; i++)
			dasa_lsb_tbl.ip_lsb[i] = ((pPar->pattern.nDstIP.nIPv4 >> (i * 8)) & 0xFF);

		/* DIP LSB Nibble Mask */
		dasa_lsb_tbl.mask = (u16) (pPar->pattern.nDstIP_Mask | 0xFF00) & 0xFFFF;
		tab_index = find_software_tbl_entry(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (tab_index < 0) {
			pr_err("%s:%s:%d (DASA Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		if (tab_index == 0xFF)
			tab_index = pce_tm_ip_dasa_lsb_tbl_write(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);

		pFet->dip_lsb_idx = (u16)tab_index;

	}
	pcetable.key[3] |= (pFet->dip_msb_idx << 8);
	pcetable.key[3] |= pFet->dip_lsb_idx;
		/* SIP MSB used */
	pFet->sip_msb_idx = 0xFF;
	pFet->sip_lsb_idx = 0xFF;
	if (pPar->pattern.eSrcIP_Select == IFX_FLOW_PCE_IP_V6) {
		pce_dasa_msb_t  dasa_msb_tbl;
		pce_dasa_lsb_t  dasa_lsb_tbl;
		int	j;
		/* First, search for DIP in the DA/SA table (DIP MSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_msb_tbl.ip_msb[j-1] = (pPar->pattern.nSrcIP.nIPv6[i] & 0xFF);
			dasa_msb_tbl.ip_msb[j]	= ((pPar->pattern.nSrcIP.nIPv6[i] >> 8) & 0xFF);
		}
		dasa_msb_tbl.mask = (u16) ((pPar->pattern.nSrcIP_Mask >> 16) & 0xFFFF);
		tab_index = find_software_msb_tbl_entry(&pHandle->pce_sub_tbl, &dasa_msb_tbl);
		if (tab_index < 0) {
			pr_err("%s:%s:%d (DASA Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		if (tab_index == 0xFF)
			tab_index = pce_tm_ip_dasa_msb_tbl_write(&pHandle->pce_sub_tbl, &dasa_msb_tbl);

		pFet->sip_msb_idx = (u16)tab_index;
		/* First, search for DIP in the DA/SA table (DIP LSB) */
		for (i = 0, j = 7; i < 4; i++, j -= 2) {
			dasa_lsb_tbl.ip_lsb[j-1] = (pPar->pattern.nSrcIP.nIPv6[i+4] & 0xFF);
			dasa_lsb_tbl.ip_lsb[j]	= ((pPar->pattern.nSrcIP.nIPv6[i+4] >> 8) & 0xFF);
		}
		dasa_lsb_tbl.mask = (u16) (pPar->pattern.nSrcIP_Mask & 0xFFFF);
		tab_index = find_software_tbl_entry(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (tab_index == 0xFF)
			tab_index = pce_tm_ip_dasa_lsb_tbl_write(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (tab_index < 0) {
			pr_err("%s:%s:%d (DASA Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		pFet->sip_lsb_idx = (u16)tab_index;

	} else  if (pPar->pattern.eSrcIP_Select == IFX_FLOW_PCE_IP_V4) { /* SIP LSB used */
		pce_dasa_lsb_t  dasa_lsb_tbl;
	/* Second, search for SIP in the DA/SA table (SIP LSB) */
		for (i = 0; i < 4 ; i++)
			dasa_lsb_tbl.ip_lsb[i] = ((pPar->pattern.nSrcIP.nIPv4 >> (i * 8)) & 0xFF);
		/* DIP LSB Nibble Mask */
		dasa_lsb_tbl.mask = (u16) (pPar->pattern.nSrcIP_Mask | 0xFF00) & 0xFFFF;
		tab_index = find_software_tbl_entry(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (tab_index == 0xFF)
			tab_index = pce_tm_ip_dasa_lsb_tbl_write(&pHandle->pce_sub_tbl, &dasa_lsb_tbl);
		if (tab_index < 0) {
			pr_err("%s:%s:%d (DASA Table full) \n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
		pFet->sip_lsb_idx = (u16)tab_index;
	}
	pcetable.key[2] |= (pFet->sip_msb_idx << 8);
	pcetable.key[2] |= pFet->sip_lsb_idx;
	/* Ethertype used */
	if (pPar->pattern.bEtherTypeEnable == LTQ_TRUE) {
		pce_protocol_tbl_t   pctl_tbl;
		pctl_tbl.key.ethertype = pPar->pattern.nEtherType;
		pctl_tbl.mask.ethertype_mask = pPar->pattern.nEtherTypeMask;
		tab_index = pce_tm_ptcl_tbl_write(&pHandle->pce_sub_tbl, &pctl_tbl);
		if (tab_index < 0)
			return tab_index;
		pFet->ethertype_idx = (u16)tab_index;
	} else {
		pFet->ethertype_idx = 0xFF;
	}
	pcetable.key[1] |= pFet->ethertype_idx;
	/* IP protocol used */
	if (pPar->pattern.bProtocolEnable == LTQ_TRUE) {
		pce_protocol_tbl_t   pctl_tbl;
		pctl_tbl.key.prot.protocol = pPar->pattern.nProtocol;
		pctl_tbl.mask.prot.protocol_mask = pPar->pattern.nProtocolMask;
		/* todo: define API parameter for the flags. */
		pctl_tbl.key.prot.protocol_flags = 0xFF;
		/* todo: define API parameter for the flags. */
		pctl_tbl.mask.prot.protocol_flag_mask = 0x3;
		tab_index = pce_tm_ptcl_tbl_write(&pHandle->pce_sub_tbl, &pctl_tbl);
		if (tab_index < 0)
			return tab_index;
		pFet->ip_prot_idx = (u16)tab_index;
	} else {
		pFet->ip_prot_idx = 0xFF;
	}
	pcetable.key[1] |= (pFet->ip_prot_idx << 8);
	/* PPPoE used */
	if (pPar->pattern.bSessionIdEnable == LTQ_TRUE) {
		pce_ppoe_tbl_t  pppoe_tbl;
		pppoe_tbl.sess_id = pPar->pattern.nSessionId;
		tab_index = pce_tm_pppoe_tbl_write(&pHandle->pce_sub_tbl, (pce_ppoe_tbl_t *)pPar);
		if (tab_index < 0)
			return tab_index;
		pFet->pppoe_idx = (u16)tab_index;
	} else {
		pFet->pppoe_idx = 0x1F;
	}
	pcetable.key[7] |= (pFet->pppoe_idx << 8);
	/* VLAN used */
	tab_index = 0x7F;
	if (pPar->pattern.bVid == LTQ_TRUE) {
		tab_index = pce_tm_find_vlan_id_index(&pHandle->pce_sub_tbl, pPar->pattern.nVid);
		if (reg_val == 1) {  /*ETC*/
			if (tab_index == 0x7F)
				tab_index = ltq_active_VLAN_IdCreate(pDevCtx, pPar->pattern.nVid);
		} else {
			if (tab_index == 0x7F) {
				pr_err("%s:%s:%d (Create VID before use it) \n", __FILE__, __func__, __LINE__);
				return LTQ_ERROR;
			}
		}
	}
	pFet->vlan_idx = (u16)tab_index;
	pcetable.key[0] &= ~(0xFF << 8);
	pcetable.key[0] |= (pFet->vlan_idx << 8);

		tab_index = 0x7F;
/* ETC */
	if (reg_val == 1) {
		if (pPar->pattern.bSLAN_Vid == LTQ_TRUE) {
			tab_index = pce_tm_find_vlan_id_index(&pHandle->pce_sub_tbl, pPar->pattern.nSLAN_Vid);
			if (tab_index == 0x7F)
				tab_index = ltq_active_VLAN_IdCreate(pDevCtx, pPar->pattern.nSLAN_Vid);
		}
		pFet->svlan_idx = (u16)tab_index;
		pcetable.key[8] &= ~(0xFF << 8);
		pcetable.key[8] |= (pFet->svlan_idx << 8);
	}
	pAct = &(pHandle->pce_act[idx]);
	memcpy(pAct, &pPar->action, sizeof(IFX_FLOW_PCE_action_t));
	/* Fill the Action parameter into the pcetable.val[x] */
	/*	Action "Forwarding" Group. Port map action enable. This port
	forwarding configuration is ignored in case the action "IGMP Snooping"
	is enabled via the parameter 'nSnoopingTypeAction'. */
	if (pAct->ePortMapAction != IFX_FLOW_PCE_ACTION_PORTMAP_DISABLE) {
		if (pAct->eSnoopingTypeAction == IFX_FLOW_PCE_ACTION_IGMP_SNOOP_DISABLE) {
			pcetable.val[0] = 1;
			pcetable.val[4] |= (0x3 << 2);
			switch (pAct->ePortMapAction) {
			case IFX_FLOW_PCE_ACTION_PORTMAP_REGULAR:
					pcetable.val[4] &= ~(0x3 << 2);
					break;
			case IFX_FLOW_PCE_ACTION_PORTMAP_DISCARD:
					pcetable.val[1] = 0;
					break;
			case IFX_FLOW_PCE_ACTION_PORTMAP_CPU:
/*					pcetable.val[1] = 0x40; */ /* Port 6 */
					pcetable.val[1] = pAct->nForwardPortMap & 0xFFFF;
					pcetable.val[4] &= ~(0x3 << 2);
					pcetable.val[4] |= (0x1 << 2);
					break;
			case IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE:
					pcetable.val[1] = pAct->nForwardPortMap & 0xFFFF;
					break;
					/* To fix compilation warnings*/
			case IFX_FLOW_PCE_ACTION_PORTMAP_DISABLE:
			case IFX_FLOW_PCE_ACTION_PORTMAP_MULTICAST_ROUTER:
			case IFX_FLOW_PCE_ACTION_PORTMAP_MULTICAST_HW_TABLE:
			case IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE_VLAN:
			case IFX_FLOW_PCE_ACTION_PORTMAP_ALTERNATIVE_STAG_VLAN:
					break;
			}
		} else {
			switch (pAct->eSnoopingTypeAction) {
			case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_REPORT:
			case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_LEAVE:
					pcetable.val[0] = 1;
					pcetable.val[4] &= ~(0x3 << 2);
					/* Forwarding Action enabled. Select Multicast Router Port-Map */
					pcetable.val[4] |= (0x1 << 2);   /* Multicast router portmap */
					break;
			default:
					pcetable.val[0] = 1;
					pcetable.val[4] &= ~(0x3 << 2);
					pcetable.val[4] |= (0x0 << 2);  /* default port map */
					break;
				}
		}
	} else {
		pcetable.val[0] = 0;
		pcetable.val[1] = 0xFFFF;
		pcetable.val[4] &= ~(0x3 << 2);
	}
  /* Action "Flow ID". This flow table action and the 'bFlowID_Action' action can be used exclusively. */
	if (pAct->bFlowID_Action != LTQ_FALSE) {
		if (pAct->ePortMapAction == IFX_FLOW_PCE_ACTION_PORTMAP_DISABLE) {
			pcetable.val[4] |= (0x1 << 4);  /* enable Flow ID action */
			pcetable.val[1] = pAct->nFlowID & 0xFFFF;
		} else {
			pr_err("%s:%s:%d (PortMap & FlowID can be used exclusively)\n", __FILE__, __func__, __LINE__);
			return LTQ_ERROR;
		}
	}

	tab_index = 0x7F;
	/** Action "VLAN" Group. VLAN action enable */
	pcetable.val[2] = 0;
	pcetable.val[0] &= ~(1 << 13);  /* for IFX_FLOW_PCE_ACTION_VLAN_REGULAR  also*/
	if (pAct->eVLAN_Action != IFX_FLOW_PCE_ACTION_VLAN_DISABLE) {
		pcetable.val[0] |= (1 << 1);
		if (pAct->eVLAN_Action == IFX_FLOW_PCE_ACTION_VLAN_ALTERNATIVE) {
			if (reg_val == 1) {
				/*ETC */
				pcetable.val[5] |= ((pAct->nVLAN_Id & 0xFFF) << 4);
				pcetable.val[2] |= ((pAct->nFId & 0xFF) << 8);
				pcetable.val[5] |= (1 << 3); /* alternative CTAG VLAN ID and FID */
				pcetable.val[0] |= (1 << 13); /*Table action enabled */
			} else {
			vlan_active_table_t VLAN_Table_Entry;
			memset(&VLAN_Table_Entry, 0, sizeof(vlan_active_table_t));
			VLAN_Table_Entry.vid = pAct->nVLAN_Id;
			tab_index = pce_tm_find_vlan_id_fd_index(&pHandle->pce_sub_tbl, &VLAN_Table_Entry);
			if (tab_index != 0x7F) {
				pcetable.val[2] = (tab_index & 0xFF);
				pcetable.val[2] |= ((VLAN_Table_Entry.fid & 0xFF) << 8);
				pcetable.val[0] |= (1 << 13); /*Table action enabled */
			}
		}
	}
  }
	/* ETC */
	/** Action "SVLAN" Group. SVLAN action enable */
	if (reg_val == 1) {
		pcetable.val[6] = 0;
		if (pAct->eSVLAN_Action != IFX_FLOW_PCE_ACTION_VLAN_DISABLE) {
			pcetable.val[0] |= (1 << 1);
			pcetable.val[6] &= ~(1 << 3); /*  default CTAG VLAN ID and FID */
		if (pAct->eSVLAN_Action == IFX_FLOW_PCE_ACTION_VLAN_ALTERNATIVE) {
				pcetable.val[6] |= ((pAct->nSVLAN_Id & 0xFFF) << 4);
				pcetable.val[0] |= (1 << 13); /*Table action enabled */
				pcetable.val[6] |= (1 << 3); /*Table action enabled */
			}
		}
	}
	/** Action "Traffic Class" Group. Traffic class action enable */
	if (pAct->eTrafficClassAction != IFX_FLOW_PCE_ACTION_TRAFFIC_CLASS_DISABLE) {
      pcetable.val[0] |= (1 << 2);
		switch (pAct->eTrafficClassAction) {
		case IFX_FLOW_PCE_ACTION_TRAFFIC_CLASS_REGULAR:
				pcetable.val[0] &= ~(1 << 14);
				break;
		case IFX_FLOW_PCE_ACTION_TRAFFIC_CLASS_ALTERNATIVE:
				pcetable.val[0] |= (1 << 14);
				pcetable.val[3] &= ~(0xF << 8);
				pcetable.val[3] |= (pAct->nTrafficClassAlternate & 0xF) << 8;
				break;
		case IFX_FLOW_PCE_ACTION_TRAFFIC_CLASS_DISABLE:
				break;
		}
	} else {
		pcetable.val[0] &= ~((1 << 2) | (1 << 14));
		pcetable.val[3] |= (0xF << 8);
	}
	/** Action "Remarking" Group. Remarking action enable */
	if (pAct->bRemarkAction != LTQ_FALSE)
		pcetable.val[0] |= (1 << 3);
	else
		pcetable.val[0] &= ~(1 << 3);
	/** Action "Cross VLAN" Group. Cross VLAN action enable */
	if (pAct->eVLAN_CrossAction != IFX_FLOW_PCE_ACTION_CROSS_VLAN_DISABLE) {
		pcetable.val[0] |= (1 << 4);
		if (pAct->eVLAN_CrossAction == IFX_FLOW_PCE_ACTION_CROSS_VLAN_REGULAR)
			pcetable.val[3] &= ~(0 << 15);
		else
			pcetable.val[3] |= (1 << 15);
	} else {
		pcetable.val[0] &= ~(1 << 4);
		pcetable.val[3] &= ~(0 << 15);
	}
	/** Action "Cross State" Group. Cross state action control and enable */
	if (pAct->eCrossStateAction != IFX_FLOW_PCE_ACTION_CROSS_STATE_DISABLE) {
		pcetable.val[0] |= (1 << 5);
		if (pAct->eCrossStateAction == IFX_FLOW_PCE_ACTION_CROSS_STATE_CROSS)
			pcetable.val[4] |= (1 << 13);
		else
			pcetable.val[4] &= ~(1 << 13);
	} else {
		pcetable.val[4] &= ~(1 << 13);
		pcetable.val[0] &= ~(1 << 5);
	}
	/** Action "Critical Frames" Group. Critical Frame action control and enable */
	if (pAct->eCritFrameAction != IFX_FLOW_PCE_ACTION_CRITICAL_FRAME_DISABLE) {
		pcetable.val[0] |= (1 << 6);
		if (pAct->eCritFrameAction == IFX_FLOW_PCE_ACTION_CRITICAL_FRAME_CRITICAL)
			pcetable.val[4] |= (1 << 14);
		else
			pcetable.val[4] &= ~(1 << 14);
	} else {
		pcetable.val[0] &= ~(1 << 6);
		pcetable.val[4] &= ~(1 << 14);
	}
	/** Action "Timestamp" Group. Time stamp action control and enable */
	if (pAct->eTimestampAction != IFX_FLOW_PCE_ACTION_TIMESTAMP_DISABLE) {
		pcetable.val[0] |= (1 << 7);
		if (pAct->eTimestampAction == IFX_FLOW_PCE_ACTION_TIMESTAMP_STORED)
			pcetable.val[4] |= (1 << 15);
		else
			pcetable.val[4] &= ~(1 << 15);
	} else {
		pcetable.val[0] &= ~(1 << 7);
		pcetable.val[4] &= ~(1 << 15);
	}
	/** Action "Interrupt" Group. Interrupt action generate and enable */
	if (pAct->eIrqAction != IFX_FLOW_PCE_ACTION_IRQ_DISABLE) {
		pcetable.val[0] |= (1 << 8);
		if (pAct->eIrqAction == IFX_FLOW_PCE_ACTION_IRQ_EVENT)
			pcetable.val[0] |= (1 << 15);
		else
			pcetable.val[0] &= ~(1 << 15);
	} else {
		pcetable.val[0] &= ~((1 << 8) | (1 << 15));
	}
	/** Action "Learning" Group. Learning action control and enable */
	if (pAct->eLearningAction != IFX_FLOW_PCE_ACTION_LEARNING_DISABLE) {
		pcetable.val[0] |= (1 << 9);
		/* Todo: Learning Rule need to be check */
		switch (pAct->eLearningAction) {
		case IFX_FLOW_PCE_ACTION_LEARNING_REGULAR:
				pcetable.val[4] &= ~0x3;
				break;
		case IFX_FLOW_PCE_ACTION_LEARNING_FORCE_NOT:
				pcetable.val[4] = (pcetable.val[4] & ~0x3) | 0x2;
				break;
		case IFX_FLOW_PCE_ACTION_LEARNING_FORCE:
				pcetable.val[4] |= 0x3;
				break;
		case IFX_FLOW_PCE_ACTION_LEARNING_DISABLE:
				break;
		}
	} else {
		pcetable.val[0] &= ~(1 << 9);
		pcetable.val[4] &= ~0x3;
	}
	/** Action "IGMP Snooping" Group. */
	if (pAct->eSnoopingTypeAction != IFX_FLOW_PCE_ACTION_IGMP_SNOOP_DISABLE) {
		pcetable.val[0] |= (1 << 10);
		pcetable.val[4] &= ~(0x7 << 5);
		switch (pAct->eSnoopingTypeAction) {
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_REGULAR:
				pcetable.val[4] |= (0 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_REPORT:
				pcetable.val[4] |= (1 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_LEAVE:
				pcetable.val[4] |= (2 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_AD:
				pcetable.val[4] |= (3 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_QUERY:
				pcetable.val[4] |= (4 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_QUERY_GROUP:
				pcetable.val[4] |= (5 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_QUERY_NO_ROUTER:
				pcetable.val[4] |= (6 << 5);
				break;
		case IFX_FLOW_PCE_ACTION_IGMP_SNOOP_DISABLE:
				break;
		}
	} else {
		pcetable.val[0] &= ~(1 << 10);
		pcetable.val[4] &= ~(0x7 << 5);
	}
	/** Action "Meter" Group. Meter action control and enable. */
	if (pAct->eMeterAction != IFX_FLOW_PCE_ACTION_METER_DISABLE) {
		pcetable.val[0] |= (1 << 11);
		pcetable.val[3] = (pAct->nMeterId & 0x1F);
		switch (pAct->eMeterAction) {
		case IFX_FLOW_PCE_ACTION_METER_REGULAR:
				pcetable.val[3] |= 0 << 6;
				break;
		case IFX_FLOW_PCE_ACTION_METER_1:
				pcetable.val[3] |= 1 << 6;
				break;
/*	case IFX_FLOW_PCE_ACTION_METER_2:
				pcetable.val[3] |= 2 << 6;
				break; */
		case IFX_FLOW_PCE_ACTION_METER_1_2:
				pcetable.val[3] |= 3 << 6;
				break;
		case IFX_FLOW_PCE_ACTION_METER_DISABLE:
				break;
		}
	} else {
		pcetable.val[0] &= ~(1 << 11);
		pcetable.val[3] |= 0x1F;
	}
   /** Action "RMON" Group. RMON action enable */
	if (pAct->bRMON_Action != LTQ_FALSE) {
		pcetable.val[0] |= (1 << 12);
		pcetable.val[4] &= ~(0x1F << 8);
		if (pAct->nRMON_Id < 24)  {/*RMON_ID will support from 1 to 24 */
			pcetable.val[4] |= ((pAct->nRMON_Id + 1) << 8);
		} else  {
			return LTQ_ERROR;
		}
	} else {
		pcetable.val[0] &= ~(1 << 12);
		pcetable.val[4] &= ~(0x1F << 8);
	}
	pcetable.val[3] |= (0x7 << 12);
	if (pAct->bRemarkDSCP == LTQ_TRUE)
		pcetable.val[3] &= ~(1 << 12);
	if (pAct->bRemarkClass == LTQ_TRUE)
		pcetable.val[3] &= ~(1 << 13);
	if (pAct->bRemarkPCP == LTQ_TRUE)
		pcetable.val[3] &= ~(1 << 14);
/* ETC */
	if (pAct->bRemarkSTAG_PCP == LTQ_TRUE)
		pcetable.val[6] &= ~(1 << 1);
	else
		pcetable.val[6] |= (1 << 1);
	if (pAct->bRemarkSTAG_DEI == LTQ_TRUE)
		pcetable.val[6] &= ~(1 << 2);
	else
		pcetable.val[6] |= (1 << 2);
	if (pAct->bPortBitMapMuxControl == LTQ_TRUE)
		pcetable.val[6] |= (1 << 0);
	else
		pcetable.val[6] &= ~(1 << 0);
	/* Trunking action enable */
	if (pAct->bPortTrunkAction == LTQ_TRUE)
		pcetable.val[5] |= (1 << 0);
	else
		pcetable.val[5] &= ~(1 << 0);
		/* Port Link Selection control */
	if (pAct->bPortLinkSelection == LTQ_TRUE)
		pcetable.val[5] |= (1 << 1);
	else
		pcetable.val[5] &= ~(1 << 1);
	/*	CVLAN Ignore control */
	if (pAct->bCVLAN_Ignore_Control == LTQ_TRUE) {
		pcetable.val[5] |= (1 << 2);
			pcetable.val[0] |= (1 << 1);
	}	else {
		pcetable.val[5] &= ~(1 << 2);
	}
	pcetable.table_index = idx;
	pcetable.table = PCE_TFLOW_INDEX;
	pcetable.valid = 1;
	xwayflow_pce_table_write(pDevCtx, &pcetable);
   return LTQ_SUCCESS;
}
