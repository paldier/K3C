/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#include "gsw_init.h"

ltq_rt_table_t *rthandler = NULL;

#define	RT_ASSERT(t)	{if ((t)) {\
	return -1; }	}

#define RT_DEBUG 0
int route_table_read(void *cdev, pctbl_prog_t *rdata)
{
	u32 value;
	do {
		gsw_r32(cdev, PCE_RTBL_CTRL_BAS_OFFSET,
			PCE_RTBL_CTRL_BAS_SHIFT,
			PCE_RTBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_w32(cdev, PCE_TBL_ADDR_ADDR_OFFSET,
		PCE_TBL_ADDR_ADDR_SHIFT,
		PCE_TBL_ADDR_ADDR_SIZE, rdata->pcindex);
	gsw_w32(cdev, PCE_RTBL_CTRL_ADDR_OFFSET,
		PCE_RTBL_CTRL_ADDR_SHIFT,
		PCE_RTBL_CTRL_ADDR_SIZE, rdata->table);
	gsw_w32(cdev, PCE_RTBL_CTRL_OPMOD_OFFSET,
		PCE_RTBL_CTRL_OPMOD_SHIFT,
		PCE_RTBL_CTRL_OPMOD_SIZE, rdata->op_mode);
	gsw_w32(cdev, PCE_RTBL_CTRL_BAS_OFFSET,
		PCE_RTBL_CTRL_BAS_SHIFT,
		PCE_RTBL_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(cdev, PCE_RTBL_CTRL_BAS_OFFSET,
			PCE_RTBL_CTRL_BAS_SHIFT,
			PCE_RTBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_r32(cdev, PCE_TBL_KEY_15_KEY15_OFFSET,
		PCE_TBL_KEY_15_KEY15_SHIFT,
		PCE_TBL_KEY_15_KEY15_SIZE, &value);
	rdata->key[15] = value;
	gsw_r32(cdev, PCE_TBL_KEY_14_KEY14_OFFSET,
		PCE_TBL_KEY_14_KEY14_SHIFT,
		PCE_TBL_KEY_14_KEY14_SIZE, &value);
	rdata->key[14] = value;
	gsw_r32(cdev, PCE_TBL_KEY_13_KEY13_OFFSET,
		PCE_TBL_KEY_13_KEY13_SHIFT,
		PCE_TBL_KEY_13_KEY13_SIZE, &value);
	rdata->key[13] = value;
	gsw_r32(cdev, PCE_TBL_KEY_12_KEY12_OFFSET,
		PCE_TBL_KEY_12_KEY12_SHIFT,
		PCE_TBL_KEY_12_KEY12_SIZE, &value);
	rdata->key[12] = value;
	gsw_r32(cdev, PCE_TBL_KEY_11_KEY11_OFFSET,
		PCE_TBL_KEY_11_KEY11_SHIFT,
		PCE_TBL_KEY_11_KEY11_SIZE, &value);
	rdata->key[11] = value;
	gsw_r32(cdev, PCE_TBL_KEY_10_KEY10_OFFSET,
		PCE_TBL_KEY_10_KEY10_SHIFT,
		PCE_TBL_KEY_10_KEY10_SIZE, &value);
	rdata->key[10] = value;
	gsw_r32(cdev, PCE_TBL_KEY_9_KEY9_OFFSET,
		PCE_TBL_KEY_9_KEY9_SHIFT,
		PCE_TBL_KEY_9_KEY9_SIZE, &value);
	rdata->key[9] = value;
	gsw_r32(cdev, PCE_TBL_KEY_8_KEY8_OFFSET,
		PCE_TBL_KEY_8_KEY8_SHIFT,
		PCE_TBL_KEY_8_KEY8_SIZE, &value);
	rdata->key[8] = value;
	gsw_r32(cdev, PCE_TBL_KEY_7_KEY7_OFFSET,
		PCE_TBL_KEY_7_KEY7_SHIFT,
		PCE_TBL_KEY_7_KEY7_SIZE, &value);
	rdata->key[7] = value;
	gsw_r32(cdev, PCE_TBL_KEY_6_KEY6_OFFSET,
		PCE_TBL_KEY_6_KEY6_SHIFT,
		PCE_TBL_KEY_6_KEY6_SIZE, &value);
	rdata->key[6] = value;
	gsw_r32(cdev, PCE_TBL_KEY_5_KEY5_OFFSET,
		PCE_TBL_KEY_5_KEY5_SHIFT,
		PCE_TBL_KEY_5_KEY5_SIZE, &value);
	rdata->key[5] = value;
	gsw_r32(cdev, PCE_TBL_KEY_4_KEY4_OFFSET,
		PCE_TBL_KEY_4_KEY4_SHIFT,
		PCE_TBL_KEY_4_KEY4_SIZE, &value);
	rdata->key[4] = value;
	gsw_r32(cdev, PCE_TBL_KEY_3_KEY3_OFFSET,
		PCE_TBL_KEY_3_KEY3_SHIFT,
		PCE_TBL_KEY_3_KEY3_SIZE, &value);
	rdata->key[3] = value;
	gsw_r32(cdev, PCE_TBL_KEY_2_KEY2_OFFSET,
		PCE_TBL_KEY_2_KEY2_SHIFT,
		PCE_TBL_KEY_2_KEY2_SIZE, &value);
	rdata->key[2] = value;
	gsw_r32(cdev, PCE_TBL_KEY_1_KEY1_OFFSET,
		PCE_TBL_KEY_1_KEY1_SHIFT,
		PCE_TBL_KEY_1_KEY1_SIZE, &value);
	rdata->key[1] = value;
	gsw_r32(cdev, PCE_TBL_KEY_0_KEY0_OFFSET,
		PCE_TBL_KEY_0_KEY0_SHIFT,
		PCE_TBL_KEY_0_KEY0_SIZE, &value);
	rdata->key[0] = value;

	gsw_r32(cdev, PCE_TBL_VAL_15_VAL15_OFFSET,
		PCE_TBL_VAL_15_VAL15_SHIFT,
		PCE_TBL_VAL_15_VAL15_SIZE, &value);
	rdata->val[15] = value;
	gsw_r32(cdev, PCE_TBL_VAL_14_VAL14_OFFSET,
		PCE_TBL_VAL_14_VAL14_SHIFT,
		PCE_TBL_VAL_14_VAL14_SIZE, &value);
	rdata->val[14] = value;
	gsw_r32(cdev, PCE_TBL_VAL_13_VAL13_OFFSET,
		PCE_TBL_VAL_13_VAL13_SHIFT,
		PCE_TBL_VAL_13_VAL13_SIZE, &value);
	rdata->val[13] = value;
	gsw_r32(cdev, PCE_TBL_VAL_12_VAL12_OFFSET,
		PCE_TBL_VAL_12_VAL12_SHIFT,
		PCE_TBL_VAL_12_VAL12_SIZE, &value);
	rdata->val[12] = value;
	gsw_r32(cdev, PCE_TBL_VAL_11_VAL11_OFFSET,
		PCE_TBL_VAL_11_VAL11_SHIFT,
		PCE_TBL_VAL_11_VAL11_SIZE, &value);
	rdata->val[11] = value;
	gsw_r32(cdev, PCE_TBL_VAL_10_VAL10_OFFSET,
		PCE_TBL_VAL_10_VAL10_SHIFT,
		PCE_TBL_VAL_10_VAL10_SIZE, &value);
	rdata->val[10] = value;
	gsw_r32(cdev, PCE_TBL_VAL_9_VAL9_OFFSET,
		PCE_TBL_VAL_9_VAL9_SHIFT,
		PCE_TBL_VAL_9_VAL9_SIZE, &value);
	rdata->val[9] = value;
	gsw_r32(cdev, PCE_TBL_VAL_8_VAL8_OFFSET,
		PCE_TBL_VAL_8_VAL8_SHIFT,
		PCE_TBL_VAL_8_VAL8_SIZE, &value);
	rdata->val[8] = value;
	gsw_r32(cdev, PCE_TBL_VAL_7_VAL7_OFFSET,
		PCE_TBL_VAL_7_VAL7_SHIFT,
		PCE_TBL_VAL_7_VAL7_SIZE, &value);
	rdata->val[7] = value;
	gsw_r32(cdev, PCE_TBL_VAL_6_VAL6_OFFSET,
		PCE_TBL_VAL_6_VAL6_SHIFT,
		PCE_TBL_VAL_6_VAL6_SIZE, &value);
	rdata->val[6] = value;
	gsw_r32(cdev, PCE_TBL_VAL_5_VAL5_OFFSET,
		PCE_TBL_VAL_5_VAL5_SHIFT,
		PCE_TBL_VAL_5_VAL5_SIZE, &value);
	rdata->val[5] = value;
	gsw_r32(cdev, PCE_TBL_VAL_4_VAL4_OFFSET,
		PCE_TBL_VAL_4_VAL4_SHIFT,
		PCE_TBL_VAL_4_VAL4_SIZE, &value);
	rdata->val[4] = value;
	gsw_r32(cdev, PCE_TBL_VAL_3_VAL3_OFFSET,
		PCE_TBL_VAL_3_VAL3_SHIFT,
		PCE_TBL_VAL_3_VAL3_SIZE, &value);
	rdata->val[3] = value;
	gsw_r32(cdev, PCE_TBL_VAL_2_VAL2_OFFSET,
		PCE_TBL_VAL_2_VAL2_SHIFT,
		PCE_TBL_VAL_2_VAL2_SIZE, &value);
	rdata->val[2] = value;
	gsw_r32(cdev, PCE_TBL_VAL_1_VAL1_OFFSET,
		PCE_TBL_VAL_1_VAL1_SHIFT,
		PCE_TBL_VAL_1_VAL1_SIZE, &value);
	rdata->val[1] = value;
	gsw_r32(cdev, PCE_TBL_VAL_0_VAL0_OFFSET,
		PCE_TBL_VAL_0_VAL0_SHIFT,
		PCE_TBL_VAL_0_VAL0_SIZE, &value);
	rdata->val[0] = value;
	gsw_r32(cdev, PCE_TBL_MASK_0_MASK0_OFFSET,
		PCE_TBL_MASK_0_MASK0_SHIFT,
		PCE_TBL_MASK_0_MASK0_SIZE, &value);
	rdata->mask[0] = value;
	gsw_r32(cdev, PCE_RTBL_CTRL_VLD_OFFSET,
		PCE_RTBL_CTRL_VLD_SHIFT,
		PCE_RTBL_CTRL_VLD_SIZE, &value);
	rdata->valid = value;
	gsw_w32(cdev, PCE_TBL_CTRL_ADDR_OFFSET, 0, 16, 0);
	return GSW_statusOk;
}

int route_table_write(void *cdev, pctbl_prog_t *rdata)
{
	u32 value;
	u16 udata;
	do {
		gsw_r32(cdev, PCE_RTBL_CTRL_BAS_OFFSET,
			PCE_RTBL_CTRL_BAS_SHIFT,
			PCE_RTBL_CTRL_BAS_SIZE, &value);
	} while (value);
	gsw_w32(cdev, PCE_TBL_ADDR_ADDR_OFFSET,
		PCE_TBL_ADDR_ADDR_SHIFT, PCE_TBL_ADDR_ADDR_SIZE,
		rdata->pcindex);

	udata = rdata->table;
	gsw_w32(cdev, PCE_RTBL_CTRL_ADDR_OFFSET,
		PCE_RTBL_CTRL_ADDR_SHIFT,
		PCE_RTBL_CTRL_ADDR_SIZE, udata);

	gsw_w32(cdev, PCE_RTBL_CTRL_OPMOD_OFFSET,
		PCE_RTBL_CTRL_OPMOD_SHIFT,
		PCE_RTBL_CTRL_OPMOD_SIZE, rdata->op_mode);

	gsw_w32(cdev, PCE_TBL_KEY_15_KEY15_OFFSET,
		PCE_TBL_KEY_15_KEY15_SHIFT,
		PCE_TBL_KEY_15_KEY15_SIZE, rdata->key[15]);
	gsw_w32(cdev, PCE_TBL_KEY_14_KEY14_OFFSET,
		PCE_TBL_KEY_14_KEY14_SHIFT,
		PCE_TBL_KEY_14_KEY14_SIZE, rdata->key[14]);
	gsw_w32(cdev, PCE_TBL_KEY_13_KEY13_OFFSET,
		PCE_TBL_KEY_13_KEY13_SHIFT,
		PCE_TBL_KEY_13_KEY13_SIZE, rdata->key[13]);
	gsw_w32(cdev, PCE_TBL_KEY_12_KEY12_OFFSET,
		PCE_TBL_KEY_12_KEY12_SHIFT,
		PCE_TBL_KEY_12_KEY12_SIZE, rdata->key[12]);
	gsw_w32(cdev, PCE_TBL_KEY_11_KEY11_OFFSET,
		PCE_TBL_KEY_11_KEY11_SHIFT,
		PCE_TBL_KEY_11_KEY11_SIZE, rdata->key[11]);
	gsw_w32(cdev, PCE_TBL_KEY_10_KEY10_OFFSET,
		PCE_TBL_KEY_10_KEY10_SHIFT,
		PCE_TBL_KEY_10_KEY10_SIZE, rdata->key[10]);
	gsw_w32(cdev, PCE_TBL_KEY_9_KEY9_OFFSET,
		PCE_TBL_KEY_9_KEY9_SHIFT,
		PCE_TBL_KEY_9_KEY9_SIZE, rdata->key[9]);
	gsw_w32(cdev, PCE_TBL_KEY_8_KEY8_OFFSET,
		PCE_TBL_KEY_8_KEY8_SHIFT,
		PCE_TBL_KEY_8_KEY8_SIZE, rdata->key[8]);
	gsw_w32(cdev, PCE_TBL_KEY_7_KEY7_OFFSET,
		PCE_TBL_KEY_7_KEY7_SHIFT,
		PCE_TBL_KEY_7_KEY7_SIZE, rdata->key[7]);
	gsw_w32(cdev, PCE_TBL_KEY_6_KEY6_OFFSET,
		PCE_TBL_KEY_6_KEY6_SHIFT,
		PCE_TBL_KEY_6_KEY6_SIZE, rdata->key[6]);
	gsw_w32(cdev, PCE_TBL_KEY_5_KEY5_OFFSET,
		PCE_TBL_KEY_5_KEY5_SHIFT,
		PCE_TBL_KEY_5_KEY5_SIZE, rdata->key[5]);
	gsw_w32(cdev, PCE_TBL_KEY_4_KEY4_OFFSET,
		PCE_TBL_KEY_4_KEY4_SHIFT,
		PCE_TBL_KEY_4_KEY4_SIZE, rdata->key[4]);
	gsw_w32(cdev, PCE_TBL_KEY_3_KEY3_OFFSET,
		PCE_TBL_KEY_3_KEY3_SHIFT,
		PCE_TBL_KEY_3_KEY3_SIZE, rdata->key[3]);
	gsw_w32(cdev, PCE_TBL_KEY_2_KEY2_OFFSET,
		PCE_TBL_KEY_2_KEY2_SHIFT,
		PCE_TBL_KEY_2_KEY2_SIZE, rdata->key[2]);
	gsw_w32(cdev, PCE_TBL_KEY_1_KEY1_OFFSET,
		PCE_TBL_KEY_1_KEY1_SHIFT,
		PCE_TBL_KEY_1_KEY1_SIZE, rdata->key[1]);
	gsw_w32(cdev, PCE_TBL_KEY_0_KEY0_OFFSET,
		PCE_TBL_KEY_0_KEY0_SHIFT,
		PCE_TBL_KEY_0_KEY0_SIZE, rdata->key[0]);

	gsw_w32(cdev, PCE_TBL_MASK_0_MASK0_OFFSET,
		PCE_TBL_MASK_0_MASK0_SHIFT,
		PCE_TBL_MASK_0_MASK0_SIZE, rdata->mask[0]);

	gsw_w32(cdev, PCE_TBL_VAL_15_VAL15_OFFSET,
		PCE_TBL_VAL_15_VAL15_SHIFT,
		PCE_TBL_VAL_15_VAL15_SIZE, rdata->val[15]);
	gsw_w32(cdev, PCE_TBL_VAL_14_VAL14_OFFSET,
		PCE_TBL_VAL_14_VAL14_SHIFT,
		PCE_TBL_VAL_14_VAL14_SIZE, rdata->val[14]);
	gsw_w32(cdev, PCE_TBL_VAL_13_VAL13_OFFSET,
		PCE_TBL_VAL_13_VAL13_SHIFT,
		PCE_TBL_VAL_13_VAL13_SIZE, rdata->val[13]);
	gsw_w32(cdev, PCE_TBL_VAL_12_VAL12_OFFSET,
		PCE_TBL_VAL_12_VAL12_SHIFT,
		PCE_TBL_VAL_12_VAL12_SIZE, rdata->val[12]);
	gsw_w32(cdev, PCE_TBL_VAL_11_VAL11_OFFSET,
		PCE_TBL_VAL_11_VAL11_SHIFT,
		PCE_TBL_VAL_11_VAL11_SIZE, rdata->val[11]);
	gsw_w32(cdev, PCE_TBL_VAL_10_VAL10_OFFSET,
		PCE_TBL_VAL_10_VAL10_SHIFT,
		PCE_TBL_VAL_10_VAL10_SIZE, rdata->val[10]);
	gsw_w32(cdev, PCE_TBL_VAL_9_VAL9_OFFSET,
		PCE_TBL_VAL_9_VAL9_SHIFT,
		PCE_TBL_VAL_9_VAL9_SIZE, rdata->val[9]);
	gsw_w32(cdev, PCE_TBL_VAL_8_VAL8_OFFSET,
		PCE_TBL_VAL_8_VAL8_SHIFT,
		PCE_TBL_VAL_8_VAL8_SIZE, rdata->val[8]);
	gsw_w32(cdev, PCE_TBL_VAL_7_VAL7_OFFSET,
		PCE_TBL_VAL_7_VAL7_SHIFT,
		PCE_TBL_VAL_7_VAL7_SIZE, rdata->val[7]);
	gsw_w32(cdev, PCE_TBL_VAL_6_VAL6_OFFSET,
		PCE_TBL_VAL_6_VAL6_SHIFT,
		PCE_TBL_VAL_6_VAL6_SIZE, rdata->val[6]);
	gsw_w32(cdev, PCE_TBL_VAL_5_VAL5_OFFSET,
		PCE_TBL_VAL_5_VAL5_SHIFT,
		PCE_TBL_VAL_5_VAL5_SIZE, rdata->val[5]);
	gsw_w32(cdev, PCE_TBL_VAL_4_VAL4_OFFSET,
		PCE_TBL_VAL_4_VAL4_SHIFT,
		PCE_TBL_VAL_4_VAL4_SIZE, rdata->val[4]);
	gsw_w32(cdev, PCE_TBL_VAL_3_VAL3_OFFSET,
		PCE_TBL_VAL_3_VAL3_SHIFT,
		PCE_TBL_VAL_3_VAL3_SIZE, rdata->val[3]);
	gsw_w32(cdev, PCE_TBL_VAL_2_VAL2_OFFSET,
		PCE_TBL_VAL_2_VAL2_SHIFT,
		PCE_TBL_VAL_2_VAL2_SIZE, rdata->val[2]);
	gsw_w32(cdev, PCE_TBL_VAL_1_VAL1_OFFSET,
		PCE_TBL_VAL_1_VAL1_SHIFT,
		PCE_TBL_VAL_1_VAL1_SIZE, rdata->val[1]);
	gsw_w32(cdev, PCE_TBL_VAL_0_VAL0_OFFSET,
		PCE_TBL_VAL_0_VAL0_SHIFT,
		PCE_TBL_VAL_0_VAL0_SIZE, rdata->val[0]);
	gsw_w32(cdev, PCE_RTBL_CTRL_VLD_OFFSET,
		PCE_RTBL_CTRL_VLD_SHIFT,
		PCE_RTBL_CTRL_VLD_SIZE, rdata->valid);
	gsw_w32(cdev, PCE_RTBL_CTRL_BAS_OFFSET,
		PCE_RTBL_CTRL_BAS_SHIFT,
		PCE_RTBL_CTRL_BAS_SIZE, 1);
	do {
		gsw_r32(cdev, PCE_RTBL_CTRL_BAS_OFFSET,
			PCE_RTBL_CTRL_BAS_SHIFT,
			PCE_RTBL_CTRL_BAS_SIZE, &value);
	} while (value != 0);
	gsw_w32(cdev, PCE_TBL_CTRL_ADDR_OFFSET, 0, 16, 0);
	return GSW_statusOk;
}

/* Static Function Declaration */
static int rt_tbl_write(void *tstart, u16 *rcnt,
	void *rpar, u32 ts, u32 tentry)
{
	int i;
/* search if the entry is already available and can be re-used */
	for (i = 0; i < tentry; i++) {
		/* entry is used, check if the entry content fits */
		if (rcnt[i] > 0) {
			if (memcmp((((char *)tstart) + i * ts),
				rpar, (u8)ts) == 0) {
				rcnt[i]++;
				return i;
			}
		}
	}
	/* find an empty entry and add information */
	for (i = 0; i < tentry; i++) {
		if (rcnt[i] == 0) {
			memcpy((((char *)tstart) + i * ts), rpar, (u8)ts);
			rcnt[i]++;
			return i;
		}
	}
	/* table is full, return an error */
	return -1;
}

static int sw_ip_tbl_write(void *tstart, u16 *rcnt,
	void *rpar, u32 ts, u32 tnum, int start)
{
	int i = 0;
	/* search if the entry is already available and can be re-used */
	if (start == 0) {
		for (i = 0; i < tnum; i++) {
			if ((rcnt[i] > 0)) {
				if (memcmp((((char *)tstart) + i * ts),
					rpar, (u8)ts) == 0) {
					rcnt[i]++;
					return i;
				}
			}
		}
		/* find an empty entry and add information */
		for (i = 0; i < tnum;) {
			char *abc = ((char *)tstart + (i * sizeof(riptbl_t)));
			riptbl_t *itbl = (riptbl_t *)abc;
			if (((i % 4) == 0) && (itbl->itype == GSW_RT_IP_V6)
				&& (itbl->valid == 1)) {
				i += 4;
			} else {
				if (rcnt[i] == 0) {
					memcpy((((char *)tstart) + i * ts),
					rpar, (u8)ts);
					rcnt[i]++;
					return i;
				} else {
					i++;
				}
			}
		}
	} else if (start == 1) {
		for (i = 0; i < tnum/4; i += 4) {
			/* entry is used, check if the entry content fits */
			if (rcnt[i] > 0) {
				if (memcmp((((char *)tstart) + i * ts),
					rpar, (u8)ts) == 0) {
					rcnt[i]++;
					return i;
				}
			}
		}
		/* find an empty entry and add information */
		for (i = 0; i < tnum/4; i += 4) {
			char *abc = ((char *)tstart + (i * sizeof(riptbl_t)));
			riptbl_t *itbl = (riptbl_t *)abc;
			if (((i % 4) == 0) && (itbl->itype == GSW_RT_IP_V4)
				&& (itbl->valid == 1)) {
				continue;
			} else {
				if (rcnt[i] == 0) {
					memcpy((((char *)tstart) + i * ts),
					rpar, (u8)ts);
					rcnt[i]++;
					return i;
				}
			}
		}
	}
	/* table is full, return an error */
	return -1;
}

static int sw_mac_tbl_write(void *tstart, u16 *rcnt,
	void *rpar, u32 ts, u32 tentry)
{
	int i;
	/* search if the entry is already available and can be re-used */
	for (i = 1; i < tentry; i++) {
		if (rcnt[i] > 0) {
			/* entry is used, check if the entry content fits */
			if (memcmp((((char *)tstart) + i * ts),
				rpar, (u8)ts) == 0) {
				rcnt[i]++;
				return i;
			}
		}
	}
	/* find an empty entry and add information */
	for (i = 1; i < tentry; i++) {
		if (rcnt[i] == 0) {
			memcpy((((char *)tstart) + i * ts), rpar, (u8)ts);
			rcnt[i]++;
			return i;
		}
	}
	/* table is full, return an error */
	return -1;
}

static int get_rt_tbl_index(void *tstart, void *rpar,
	u32 ts, u32 tentry)
{
	int i;
	/* search if the entry is already available and can be re-used */
	for (i = 0; i < tentry; i++) {
		/* entry is used, check if the entry content fits */
		if (memcmp(((char *)tstart) + i * ts, rpar, (u8)ts) == 0)
			return i;
	}
	return 0x7FFF;
}

/* Static Function Declaration */
int find_ip_tbl_entry(rt_table_handle_t *tm, riptbl_t *rpar)
{
	return get_rt_tbl_index(tm->rt_ip_tbl, rpar,
		sizeof(riptbl_t), RT_IP_TBL_SIZE);
}

static int rt_iptable_rd(void *cdev,
	int index, riptbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;
	int i;
	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_IP_INDEX;
	ptable.pcindex	= index;
	ptable.op_mode = OPMOD_IPv4_READ;
	route_table_read(cdev, &ptable);
	if ((ptable.val[8] >> 15) & 0x1) {
		rpar->itype = GSW_RT_IP_V6;
		for (i = 0; i < 8; i++)
			rpar->iaddr.i6addr[i] = (ptable.val[i] & 0xFFFF);
	} else {
		rpar->itype = GSW_RT_IP_V4;
		rpar->iaddr.i4addr[0] =  (ptable.val[0] & 0xFF);
		rpar->iaddr.i4addr[1] =  ((ptable.val[0] >> 8) & 0xFF);
		rpar->iaddr.i4addr[2] =  (ptable.val[1] & 0xFF);
		rpar->iaddr.i4addr[3] =  ((ptable.val[1] >> 8) & 0xFF);
	}
	return GSW_statusOk;
}

static int rt_iptable_wr(void *cdev,
	int index, riptbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;
	int i, j;
	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_IP_INDEX;
	ptable.pcindex	= index;
	if (rpar->itype == GSW_RT_IP_V4) {
		ptable.op_mode = OPMOD_IPv4_WRITE;
		ptable.val[0] = ((rpar->iaddr.i4addr[0])
			| (rpar->iaddr.i4addr[1] << 8));
		ptable.val[1] = ((rpar->iaddr.i4addr[2])
			| (rpar->iaddr.i4addr[3] << 8));
		ptable.val[8] &= ~(1 << 15);
	} else if (rpar->itype == GSW_RT_IP_V6) {
		ptable.pcindex = index & 0xFFFC;
		ptable.op_mode = OPMOD_IPv4_WRITE;
		for (i = 0, j = 7; i < 8; i++, j--)
			ptable.val[i] = (rpar->iaddr.i6addr[j] & 0xFFFF);
		ptable.val[8] &= ~(1 << 15);
		ptable.val[8] |= (1 << 15);
	}
	route_table_write(cdev, &ptable);
	return GSW_statusOk;
}

/* Routing IP Table */
static int rt_ip_tbl_write(void *cdev,
	rt_table_handle_t *rtbl, riptbl_t *rpar)
{
	int pcindex = -1;
	if (rpar->itype == GSW_RT_IP_V4) {
		pcindex = sw_ip_tbl_write(rtbl->rt_ip_tbl,
			rtbl->rt_ip_tbl_cnt, rpar,
			sizeof(riptbl_t), RT_IP_TBL_SIZE, 0);
	} else if (rpar->itype == GSW_RT_IP_V6) {
		pcindex = sw_ip_tbl_write(rtbl->rt_ip_tbl,
			rtbl->rt_ip_tbl_cnt, rpar,
			sizeof(riptbl_t), RT_IP_TBL_SIZE, 1);
	}
	if (pcindex < 0)
		return -1;
	rt_iptable_wr(cdev, pcindex, rpar);
	return pcindex;
}

/* IP DA/SA lsb Table delete */
static int rt_ip_tbl_delete(void *cdev,
	rt_table_handle_t *rtbl, int index)
{
	pctbl_prog_t ptable;

	RT_ASSERT(index >= RT_IP_TBL_SIZE);
	if (rtbl->rt_ip_tbl_cnt[index] > 0)
		rtbl->rt_ip_tbl_cnt[index]--;
	if (rtbl->rt_ip_tbl_cnt[index] == 0) {
		memset((((char *)rtbl->rt_ip_tbl) + (index * sizeof(riptbl_t))),
		0, sizeof(riptbl_t));
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_IP_INDEX;
		ptable.op_mode = OPMOD_IPv4_WRITE;
		ptable.pcindex = index;
		route_table_write(cdev, &ptable);
	}
	return 0;
}

static int rt_mtutable_wr(void *cdev, u16 index,
	rt_mtu_tbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;
	memset(&ptable, 0, sizeof(ptable));
	ptable.table = PCE_R_MTU_INDEX;
	ptable.pcindex = index;
	ptable.op_mode = OPMOD_ADDRESS_WRITE;
	ptable.val[0] = rpar->mtsize;
	route_table_write(cdev, &ptable);
	return GSW_statusOk;
}

/* MTU Table Write */
static int rt_mtu_tbl_write(void *cdev,
	rt_table_handle_t *rtbl, rt_mtu_tbl_t *rpar)
{
	int pcindex;
	pcindex = rt_tbl_write(rtbl->rt_mtu_tbl, rtbl->rt_mtu_tbl_cnt,
		rpar,	sizeof(rt_mtu_tbl_t), RT_MTU_TBL_SIZE);
	if (pcindex < 0)
		return -1;
	rt_mtutable_wr(cdev, pcindex, rpar);
	return pcindex;
}

/* MTU Table Delete */
static int rt_mtu_tbl_delete(void *cdev,
	rt_table_handle_t *rtbl, u32 index)
{
	pctbl_prog_t ptable;

	RT_ASSERT(index >= RT_MTU_TBL_SIZE);
	if (rtbl->rt_mtu_tbl_cnt[index] > 0)
		rtbl->rt_mtu_tbl_cnt[index]--;
	if (rtbl->rt_mtu_tbl_cnt[index] == 0) {
		memset((((char *)rtbl->rt_mtu_tbl) +
		(index * sizeof(rt_mtu_tbl_t))),
		0, sizeof(rt_mtu_tbl_t));
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_MTU_INDEX;
		ptable.op_mode = OPMOD_ADDRESS_WRITE;
		ptable.pcindex = index;
		route_table_write(cdev, &ptable);
	}
	return 0;
}

static int rt_mtutable_rd(void *cdev,
	u16 index, rt_mtu_tbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;
	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_MTU_INDEX;
	ptable.pcindex	= index;
	ptable.op_mode = OPMOD_ADDRESS_READ;
	route_table_read(cdev, &ptable);
	rpar->mtsize = ptable.val[0] & 0x3FFF;
	return GSW_statusOk;
}

static int rt_mactable_wr(void *cdev, int index,
	rt_mac_tbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;
	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_MAC_INDEX;
	ptable.pcindex	= index;
	ptable.op_mode = OPMOD_ADDRESS_WRITE;
	ptable.val[0] = rpar->mdata[4] << 8 | rpar->mdata[5];
	ptable.val[1] = rpar->mdata[2] << 8 | rpar->mdata[3];
	ptable.val[2] = rpar->mdata[0] << 8 | rpar->mdata[1];
	route_table_write(cdev, &ptable);
	return GSW_statusOk;
}

/* MAC DA/SA Table index write */
static int rt_mac_tbl_write(void *cdev,
	rt_table_handle_t *rtbl, rt_mac_tbl_t *rpar)
{
	int pcindex;

	pcindex = sw_mac_tbl_write(rtbl->rt_mac_tbl,
		rtbl->rt_mac_tbl_cnt, rpar,
		sizeof(rt_mac_tbl_t), RT_MAC_TBL_SIZE);
	if (pcindex < 0)
		return -1;
	rt_mactable_wr(cdev, pcindex, rpar);
	return pcindex;
}

/* MAC DA/SA Table delete */
static int rt_mac_tbl_delete(void *cdev,
	rt_table_handle_t *rtbl, u32 index)
{
	pctbl_prog_t ptable;

	RT_ASSERT(index >= RT_MAC_TBL_SIZE);
	if (rtbl->rt_mac_tbl_cnt[index] > 0)
		rtbl->rt_mac_tbl_cnt[index]--;

	if (rtbl->rt_mac_tbl_cnt[index] == 0) {
		memset((((char *)rtbl->rt_mac_tbl) +
		(index * sizeof(rt_mac_tbl_t))),
		0, sizeof(rt_mac_tbl_t));
		/* initialize the data structure before using it */
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_MAC_INDEX;
		ptable.op_mode = OPMOD_ADDRESS_WRITE;
		ptable.pcindex = index;
		route_table_write(cdev, &ptable);
	}
	return 0;
}

static int rt_mactable_rd(void *cdev,
	int index, rt_mac_tbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;
	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_MAC_INDEX;
	ptable.pcindex	= index;
	ptable.op_mode = OPMOD_ADDRESS_READ;
	route_table_read(cdev, &ptable);
	rpar->mdata[0] = ptable.val[2] >> 8;
	rpar->mdata[1] = ptable.val[2] & 0xFF;
	rpar->mdata[2] = ptable.val[1] >> 8;
	rpar->mdata[3] = ptable.val[1] & 0xFF;
	rpar->mdata[4] = ptable.val[0] >> 8;
	rpar->mdata[5] = ptable.val[0] & 0xFF;
	return GSW_statusOk;
}

static int rt_ppoetable_rd(void *cdev,
	int index, rt_ppoe_tbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;

	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_PPPOE_INDEX;
	ptable.pcindex	= index;
	ptable.op_mode = OPMOD_ADDRESS_READ;
	route_table_read(cdev, &ptable);
	rpar->psesid = ptable.val[0];
	return GSW_statusOk;
}

static int rt_ppoetable_wr(void *cdev,
	int index, rt_ppoe_tbl_t *rpar)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;

	memset(&ptable, 0, sizeof(ptable));
	ptable.table = PCE_R_PPPOE_INDEX;
	ptable.pcindex = index;
	ptable.op_mode = OPMOD_ADDRESS_WRITE;
	ptable.val[0]	= rpar->psesid;
	ptable.valid = 1;
	route_table_write(cdev, &ptable);
	return GSW_statusOk;
}

/* PPPoE Table Write */
static int rt_pppoe_tbl_write(void *cdev,
	rt_table_handle_t *rtbl, rt_ppoe_tbl_t *rpar)
{
	int pcindex;
	pcindex = rt_tbl_write(rtbl->rt_ppoe_tbl,
		rtbl->rt_ppoe_tbl_cnt,
		rpar,	sizeof(rt_ppoe_tbl_t), RT_PPPOE_TBL_SIZE);
	if (pcindex < 0)
		return -1;
	rt_ppoetable_wr(cdev, pcindex, rpar);
	return pcindex;
}

/* PPPoE Table Delete */
static int rt_pppoe_tbl_delete(void *cdev,
	rt_table_handle_t *rtbl, u32 index)
{
	pctbl_prog_t ptable;

	RT_ASSERT(index >= RT_PPPOE_TBL_SIZE);
	if (rtbl->rt_ppoe_tbl_cnt[index] > 0)
		rtbl->rt_ppoe_tbl_cnt[index]--;
	if (rtbl->rt_ppoe_tbl_cnt[index] == 0) {
		memset((((char *)rtbl->rt_ppoe_tbl) +
		(index * sizeof(rt_ppoe_tbl_t))),
		0, sizeof(rt_ppoe_tbl_t));
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_PPPOE_INDEX;
		ptable.op_mode = OPMOD_ADDRESS_WRITE;
		ptable.pcindex	= index;
		route_table_write(cdev, &ptable);
	}
	return 0;
}

#if 0
static int rt_rtptable_wr(void *cdev, int index,
	rt_rtp_tbl_t *rtptableentry)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;

	memset(&ptable, 0, sizeof(ptable));
	ptable.table = PCE_R_RTP_INDEX;
	ptable.pcindex = index;
	ptable.op_mode = OPMOD_ADDRESS_WRITE;
	ptable.val[0] = rtptableentry->rtpseqnum;
	ptable.val[1] = rtptableentry->rtpsespcnt;
	route_table_write(cdev, &ptable);
	return GSW_statusOk;
}
/* RTP Table Write */
static int rt_rtp_tbl_write(void *cdev,
	rt_table_handle_t *rtbl, rt_rtp_tbl_t *rpar)
{
	int pcindex;
	pcindex = rt_tbl_write(rtbl->rt_rtp_tbl,
		rtbl->rt_rtp_tbl_cnt,
		rpar,	sizeof(rt_rtp_tbl_t), RT_RTP_TBL_SIZE);
	if (pcindex < 0)
		return -1;
	rt_rtptable_wr(cdev, pcindex, rpar);
	return pcindex;
}

/* RTP Table Delete */
static int rt_rtp_tbl_delete(void *cdev,
	rt_table_handle_t *rtbl, u32 index)
{
	pctbl_prog_t ptable;

	RT_ASSERT(index >= RT_RTP_TBL_SIZE);
	if (rtbl->rt_rtp_tbl_cnt[index] > 0)
		rtbl->rt_rtp_tbl_cnt[index]--;
	if (rtbl->rt_rtp_tbl_cnt[index] == 0) {
		memset((((char *)rtbl->rt_rtp_tbl) +
		(index * sizeof(rt_rtp_tbl_t))),
		0, sizeof(rt_rtp_tbl_t));
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_RTP_INDEX;
		ptable.op_mode = OPMOD_ADDRESS_WRITE;
		ptable.pcindex = index;
		route_table_write(cdev, &ptable);
	}
	return 0;
}
#endif

static int rt_rtptable_rd(void *cdev, int index,
	rt_rtp_tbl_t *rtptableentry)
{
	/*ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;*/
	pctbl_prog_t ptable;

	memset(&ptable, 0, sizeof(ptable));
	ptable.table	= PCE_R_RTP_INDEX;
	ptable.pcindex	= index;
	ptable.op_mode = OPMOD_ADDRESS_READ;
	route_table_read(cdev, &ptable);
	rtptableentry->rtpseqnum = ptable.val[0];
	rtptableentry->rtpsespcnt = ptable.val[1];
	return GSW_statusOk;
}

int calhash(const u8 *data, size_t len)
{
	int crc = 0, i;
	if (len)
		do {
			crc ^= ((*data)<<8);
			data++;
			for (i = 0; i < 8; i++) {
				if (crc & 0x8000)
					crc = (crc << 1) ^ 0x1021;
				else
					crc <<= 1;
				crc = crc & 0xFFFF;
			}
		} while (--len);
	return crc;
}

int pae_hash_index(GSW_ROUTE_Entry_t *rpar)
{
	u8 hdata[13] = {0};
	int i, j, hashc;
	if (rpar->routeEntry.pattern.eIpType == GSW_RT_IP_V4) {
		for (i = 0; i < 4; i++)
			hdata[i] =
			((rpar->routeEntry.pattern.nSrcIP.nIPv4 >>
			((3-i) * 8)) & 0xFF);
		for (i = 0; i < 4; i++)
			hdata[i + 4] =
			((rpar->routeEntry.pattern.nDstIP.nIPv4 >>
			((3 - i) * 8)) & 0xFF);
	} else if (rpar->routeEntry.pattern.eIpType == GSW_RT_IP_V6) {
		u32 ip6data[4];
		u32 xdata;
		for (i = 0, j = 0; i < 4; i++, j += 2) {
			ip6data[i] =
			((rpar->routeEntry.pattern.nSrcIP.nIPv6[j] << 16) |
			((rpar->routeEntry.pattern.nSrcIP.nIPv6[j+1])));
		}
		xdata = (ip6data[0] ^ ip6data[1] ^ ip6data[2] ^ ip6data[3]);
		for (i = 0; i < 4; i++)
			hdata[i] = ((xdata >> ((3 - i) * 8)) & 0xFF);

		for (i = 0, j = 0; i < 4; i++, j += 2) {
			ip6data[i] =
			((rpar->routeEntry.pattern.nDstIP.nIPv6[j] << 16) |
			((rpar->routeEntry.pattern.nDstIP.nIPv6[j+1])));
		}
		xdata = (ip6data[0] ^ ip6data[1] ^ ip6data[2] ^ ip6data[3]);
		for (i = 0; i < 4; i++)
			hdata[i + 4] = ((xdata >> ((3 - i) * 8)) & 0xFF);
	}
	for (i = 0; i < 2; i++)
		hdata[i + 8] =
		((rpar->routeEntry.pattern.nSrcPort >> ((1-i) * 8)) & 0xFF);
	for (i = 0; i < 2; i++)
		hdata[i + 10] =
		((rpar->routeEntry.pattern.nDstPort >> ((1-i) * 8)) & 0xFF);
	hdata[12] = rpar->routeEntry.pattern.nRoutExtId & 0xFF;

	hashc = calhash(hdata, sizeof(hdata));
	return hashc & 0xFFF;
}

int rt_free_entry(void *cdev, int free_index)
{
	int temp1, temp2;
	pctbl_prog_t ptable;
	memset(&ptable, 0, sizeof(pctbl_prog_t));
	rthandler->rstbl.node[free_index].fflag = 1;
	rthandler->rstbl.nfentries++;
	if ((rthandler->rstbl.node[free_index].pprt == -1) &&
		(rthandler->rstbl.node[free_index].nptr != -1)) {
		temp2 = rthandler->rstbl.node[free_index].nptr;
		rthandler->rstbl.node[temp2].pprt = -1;
	} else if ((rthandler->rstbl.node[free_index].pprt != -1)
		&& (rthandler->rstbl.node[free_index].nptr == -1)) {
		temp1 = rthandler->rstbl.node[free_index].pprt;
		rthandler->rstbl.node[temp1].nptr = -1;
		rthandler->rstbl.hw_table[temp1].hwnextptr = temp1;
//		rthandler->rstbl.hw_table[temp1].hwvalid = 1;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table	= PCE_R_SESSION_INDEX;
		ptable.pcindex	= temp1;
		ptable.op_mode = OPMOD_ADDRESS_READ;
		route_table_read(cdev, &ptable);

		ptable.table	= PCE_R_SESSION_INDEX;
		ptable.pcindex = temp1;
		ptable.valid = 1;
		ptable.key[5] = rthandler->rstbl.hw_table[temp1].hwnextptr;
		ptable.key[4] &= ~(1 << 15);
		ptable.key[4] |= ((rthandler->rstbl.hw_table[temp1].hwvalid) << 15);
		ptable.op_mode = OPMOD_RT_SESSION_NEXT;
		route_table_write(cdev, &ptable);
	} else if ((rthandler->rstbl.node[free_index].pprt != -1)
			&& (rthandler->rstbl.node[free_index].nptr != -1)) {
		temp1 = rthandler->rstbl.node[free_index].pprt;
		temp2 = rthandler->rstbl.node[free_index].nptr;
		rthandler->rstbl.node[temp2].pprt = temp1;
		rthandler->rstbl.node[temp1].nptr = temp2;
		rthandler->rstbl.hw_table[temp1].hwnextptr = temp2;
//		rthandler->rstbl.hw_table[temp1].hwvalid = 1;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table	= PCE_R_SESSION_INDEX;
		ptable.pcindex	= temp1;
		ptable.op_mode = OPMOD_ADDRESS_READ;
		route_table_read(cdev, &ptable);
		
		ptable.table	= PCE_R_SESSION_INDEX;
		ptable.pcindex = temp1;
		ptable.valid = 1;
		ptable.key[5] = rthandler->rstbl.hw_table[temp1].hwnextptr;
		ptable.key[4] &= ~(1 << 15);
		ptable.key[4] |= ((rthandler->rstbl.hw_table[temp1].hwvalid) << 15);
		ptable.op_mode = OPMOD_RT_SESSION_NEXT;
		route_table_write(cdev, &ptable);
	}
	if (rthandler->rstbl.nfentries <= 1) {
		rthandler->rstbl.ffptr = free_index;
		rthandler->rstbl.lfptr = free_index;
		rthandler->rstbl.node[free_index].nptr = -1;
		rthandler->rstbl.node[free_index].pprt = -1;
	} else {
		temp1 = rthandler->rstbl.lfptr;
		rthandler->rstbl.lfptr = free_index;
		rthandler->rstbl.node[free_index].pprt = temp1;
		rthandler->rstbl.node[temp1].nptr = free_index;
		rthandler->rstbl.node[free_index].nptr = -1;
	}
	rthandler->rstbl.hw_table[free_index].hwnextptr = free_index;
	rthandler->rstbl.hw_table[free_index].hwvalid = 0;
	memset(&ptable, 0, sizeof(pctbl_prog_t));
	memset(&ptable, 0, sizeof(pctbl_prog_t));
	ptable.table	= PCE_R_SESSION_INDEX;
	ptable.pcindex	= free_index;
	ptable.op_mode = OPMOD_ADDRESS_READ;
	route_table_read(cdev, &ptable);
	ptable.table	= PCE_R_SESSION_INDEX;
	ptable.pcindex = free_index;
	ptable.valid = 0;
	ptable.key[5] = rthandler->rstbl.hw_table[free_index].hwnextptr;
	ptable.key[4] &= ~(1 << 15);
	ptable.key[4] |= ((rthandler->rstbl.hw_table[free_index].hwvalid) << 15);
	ptable.op_mode = OPMOD_RT_SESSION_NEXT;
	route_table_write(cdev, &ptable);
	return GSW_statusOk;
}

int GSW_ROUTE_SessionEntryAdd(void *cdev, GSW_ROUTE_Entry_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	pctbl_prog_t ptable;
	int hindex, tab_index, i, index, retval = GSW_statusErr;
	int sindex, dindex, aindex, smindex, dmindex, ppindex, tuindex,rrindex, mtindex = 0xFF;
	riptbl_t  dstip, srcip;
	rt_mtu_tbl_t mtval;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)	&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		if (rpar->nHashVal < 0) {
			/* calculate hash index */
			hindex = pae_hash_index(rpar);
		} else {
			hindex = rpar->nHashVal;
		}
		index = hindex;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		tab_index = 0x7FFF;
		sindex = tab_index;
		dindex = tab_index;
		aindex = tab_index;
		if (rpar->routeEntry.pattern.eIpType == GSW_RT_IP_V4) {
			memset(&dstip, 0, sizeof(riptbl_t));
			memset(&srcip, 0, sizeof(riptbl_t));
			dstip.itype = GSW_RT_IP_V4;
			srcip.itype = GSW_RT_IP_V4;
			for (i = 0; i < 4; i++) {
				dstip.iaddr.i4addr[i] =
				((rpar->routeEntry.pattern.nDstIP.nIPv4 >> (i * 8)) & 0xFF);
				srcip.iaddr.i4addr[i] =
				((rpar->routeEntry.pattern.nSrcIP.nIPv4 >> (i * 8)) & 0xFF);
			}
			dstip.valid = 1;
			tab_index = rt_ip_tbl_write(cdev, &rthandler->rt_sub_tbl, &dstip);
			if (tab_index < 0)
				return GSW_ROUTE_ERROR_IP_FULL;
			else
				sindex = tab_index;

			ptable.key[0] = (tab_index & 0x7FF);

			srcip.valid = 1;
			tab_index = rt_ip_tbl_write(cdev, &rthandler->rt_sub_tbl, &srcip);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_IP_FULL;
				goto errexit1;
			}
			else
				dindex = tab_index;

			ptable.key[1] = (tab_index & 0x7FF);
		} else if (rpar->routeEntry.pattern.eIpType == GSW_RT_IP_V6) {
			memset(&dstip, 0, sizeof(riptbl_t));
			memset(&srcip, 0, sizeof(riptbl_t));
			dstip.itype = GSW_RT_IP_V6;
			srcip.itype = GSW_RT_IP_V6;
			for (i = 0;  i < 8; i++) {
				dstip.iaddr.i6addr[i] = (rpar->routeEntry.pattern.nDstIP.nIPv6[i]);
				srcip.iaddr.i6addr[i] = (rpar->routeEntry.pattern.nSrcIP.nIPv6[i]);
			}
			dstip.valid = 1;
			tab_index = rt_ip_tbl_write(cdev, &rthandler->rt_sub_tbl, &dstip);
			if (tab_index < 0)
				return GSW_ROUTE_ERROR_IP_FULL;
			else
				sindex = tab_index;

			ptable.key[0] = (tab_index & 0x7FF);
			srcip.valid = 1;
			tab_index = rt_ip_tbl_write(cdev, &rthandler->rt_sub_tbl, &srcip);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_IP_FULL;
				goto errexit1;
			}
			else
				dindex = tab_index;
			ptable.key[1] = (tab_index & 0x7FF);
		} else {
			ptable.key[0] = (tab_index & 0x7FF);
			ptable.key[1] = (tab_index & 0x7FF);
		}
		ptable.key[2] = rpar->routeEntry.pattern.nDstPort;
		ptable.key[3] = rpar->routeEntry.pattern.nSrcPort;
		ptable.key[4] = rpar->routeEntry.pattern.nRoutExtId & 0xFF;
		/* Destination Port Map  */
		ptable.val[0] = (rpar->routeEntry.action.nDstPortMap & 0xFFFF);
		/*Destination Sub-Interface ID*/
		ptable.val[1] = ((rpar->routeEntry.action.nDstSubIfId & 0x1FFF) << 3);

		/* New IP Address Index */
		tab_index = 0x7FFF;
		if (rpar->routeEntry.action.eIpType == GSW_RT_IP_V4) {
			memset(&dstip, 0, sizeof(riptbl_t));
			dstip.itype = GSW_RT_IP_V4;
			for (i = 0; i < 4; i++)
				dstip.iaddr.i4addr[i] =
				((rpar->routeEntry.action.nNATIPaddr.nIPv4 >> (i * 8)) & 0xFF);
			dstip.valid = 1;
			tab_index = rt_ip_tbl_write(cdev, &rthandler->rt_sub_tbl, &dstip);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_IP_FULL;
				goto errexit2;
			}
			else
				aindex = tab_index;
		} else if (rpar->routeEntry.action.eIpType == GSW_RT_IP_V6) {
			memset(&dstip, 0, sizeof(riptbl_t));
			dstip.itype = GSW_RT_IP_V6;
			for (i = 0; i < 8; i++)
				dstip.iaddr.i6addr[i] = (rpar->routeEntry.action.nNATIPaddr.nIPv6[i]);
			dstip.valid = 1;
			tab_index = rt_ip_tbl_write(cdev, &rthandler->rt_sub_tbl, &dstip);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_IP_FULL;
				goto errexit2;
			}
			else
				aindex = tab_index;
		}
		ptable.val[2] = tab_index & 0x7FF;

		/* New UDP/TCP Port */
		ptable.val[3] = rpar->routeEntry.action.nTcpUdpPort;

		/* MTU Index */
		memset(&mtval, 0, sizeof(rt_mtu_tbl_t));
		/*Programme +1 for MTU value, due to < MTU check in HW instead of <= MTU*/
		mtval.mtsize = (rpar->routeEntry.action.nMTUvalue + 1);
		mtval.valid = 1;
		mtval.mtsize = (rpar->routeEntry.action.nMTUvalue + 1);
		tab_index = rt_mtu_tbl_write(cdev, &rthandler->rt_sub_tbl, &mtval);
		if (tab_index < 0) {
			retval = GSW_ROUTE_ERROR_MTU_FULL;
			goto errexit3;
		}
		else
			mtindex = tab_index;
		ptable.val[4] |= (tab_index & 0x7);

	/* New Source MAC Address Index */
		if (rpar->routeEntry.action.bMAC_SrcEnable == 1) {
			rt_mac_tbl_t src_mac_tbl;
			memset(&src_mac_tbl, 0, sizeof(rt_mac_tbl_t));
		/* Destination MAC address */
			for (i = 0; i < 6; i++)
				src_mac_tbl.mdata[i] =
				rpar->routeEntry.action.nSrcMAC[i];
			src_mac_tbl.valid = 1;
			tab_index = rt_mac_tbl_write(cdev, &rthandler->rt_sub_tbl, &src_mac_tbl);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_MAC_FULL;
				goto errexit4;
			}
			else
				smindex = tab_index;
			ptable.val[4] |= ((tab_index & 0x3FF) << 8);
		} else {
			ptable.val[4] |= (0 << 8);
			smindex = 0;
		}

	/* New Destination MAC Address Index */
		if (rpar->routeEntry.action.bMAC_DstEnable == 1) {
			rt_mac_tbl_t dst_mac_tbl;
			memset(&dst_mac_tbl, 0, sizeof(rt_mac_tbl_t));
		/* Destination MAC address */
			for (i = 0; i < 6; i++)
				dst_mac_tbl.mdata[i] =
				rpar->routeEntry.action.nDstMAC[i];
			dst_mac_tbl.valid = 1;
			tab_index = rt_mac_tbl_write(cdev, &rthandler->rt_sub_tbl, &dst_mac_tbl);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_MAC_FULL;
				goto errexit5;
			}
			else
				dmindex = tab_index;
			ptable.val[5] = ((tab_index) & 0x3FF);
		} else {
			ptable.val[5] = 0;
			dmindex = 0;
		}

		ptable.val[8] |= rpar->routeEntry.action.nFID & 0x3F;
		ptable.val[8] |= ((rpar->routeEntry.action.nFlowId & 0xFF) << 8);
		if (rpar->routeEntry.action.bInnerDSCPRemark == 1)
			ptable.val[9] |= rpar->routeEntry.action.nDSCP & 0x3F;
		if (rpar->routeEntry.action.bTCremarking == 1)
			ptable.val[9] |= ((rpar->routeEntry.action.nTrafficClass & 0xF) << 8);
/*ptable.val[10] = rpar->routeEntry.action.nSessionCtrs & 0xFFFF;*/
/*ptable.val[11] = (rpar->routeEntry.action.nSessionCtrs >> 16) & 0xFFFF;*/

		ptable.val[10] = 0;
		ptable.val[11] = 0;
		if (rpar->routeEntry.action.eSessDirection == 1)
			ptable.val[12] |= (1 << 0);
		else
			ptable.val[12] &= ~(1 << 0);

		if (rpar->routeEntry.action.bPPPoEmode == 1) {
			/* New PPPoE Index Index */
			rt_ppoe_tbl_t sesid;
			sesid.psesid = rpar->routeEntry.action.nPPPoESessId;
			tab_index = rt_pppoe_tbl_write(cdev, &rthandler->rt_sub_tbl, &sesid);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_PPPOE_FULL;
				goto errexit6;
			}
			else
				ppindex = tab_index;
			ptable.val[6] |= (tab_index & 0xF);
			ptable.val[12] |= (1 << 1);
		} else {
			ptable.val[12] &= ~(1 << 1);
			ptable.val[6] |= 0;
			ppindex = 0xFF;
		}
		if (rpar->routeEntry.action.bRTPMeasEna == 1) {
			/*  Routing RTP Table Index */
#if 1
			ptable.val[7] |= ((rpar->routeEntry.action.nRTPSeqNumber & 0x3F) << 8);
#else
			rt_rtp_tbl_t rtp_tbl;
			rtp_tbl.rtpseqnum =
			rpar->routeEntry.action.nRTPSeqNumber;
			rtp_tbl.rtpsespcnt =
			rpar->routeEntry.action.nRTPSessionPktCnt;
			tab_index = rt_rtp_tbl_write(cdev, &rthandler->rt_sub_tbl, &rtp_tbl);
			if (tab_index < 0) {
				retval = GSW_ROUTE_ERROR_RTP_FULL;
				goto errexit;
			}
			else
				rrindex = tab_index;
			ptable.val[7] |= (tab_index << 8);
#endif
			ptable.val[12] |= (1 << 11);
		} else {
			ptable.val[12] &= ~(1 << 11);
			ptable.val[7] |= (0x3F << 8);
			rrindex = 0x3F;
		}

		switch (rpar->routeEntry.action.eSessRoutingMode) {
		ptable.val[12] &= ~(3 << 2);
		case GSW_ROUTE_MODE_NULL:
			ptable.val[12] |= (0 << 2);
			break;
		case GSW_ROUTE_MODE_ROUTING:
			ptable.val[12] |= (1 << 2);
			break;
		case GSW_ROUTE_MODE_NAT:
			ptable.val[12] |= (2 << 2);
			break;
		case GSW_ROUTE_MODE_NAPT:
			ptable.val[12] |= (3 << 2);
			break;
		default:
			ptable.val[12] |= (0 << 2);
		}

		/* Tunnel Index */
		if (rpar->routeEntry.action.bTunnel_Enable == 1) {
			ptable.val[12] &= ~(3 << 4);
			switch (rpar->routeEntry.action.eTunType) {
			case GSW_ROUTE_TUNL_NULL:
				ptable.val[12] |= (0 << 4);
				break;
			case GSW_ROUTE_TUNL_6RD:
				ptable.val[12] |= (1 << 4);
				break;
			case GSW_ROUTE_TUNL_DSLITE:
				ptable.val[12] |= (2 << 4);
				break;
			case GSW_ROUTE_TUNL_L2TP:
				if (rpar->routeEntry.pattern.eIpType == GSW_RT_IP_V4) {
					ptable.val[12] |= (2 << 4);
				} else if (rpar->routeEntry.pattern.eIpType == GSW_RT_IP_V6) {
					ptable.val[12] |= (1 << 4);
				}
				break;
			case GSW_ROUTE_TUNL_IPSEC:
				ptable.val[12] |= (3 << 4);
				break;
			default:
				ptable.val[12] |= (0 << 4);
			}
			ptable.val[6] |=
			((rpar->routeEntry.action.nTunnelIndex & 0xF) << 8);
			tuindex = rpar->routeEntry.action.nTunnelIndex;
		} else {
			ptable.val[6] |= (0 << 8);
			tuindex = 0xFF;
		}

		switch (rpar->routeEntry.action.eOutDSCPAction) {
		ptable.val[12] &= ~(3 << 6);
		case GSW_ROUTE_OUT_DSCP_NULL:
			ptable.val[12] |= (0 << 6);
			break;
		case GSW_ROUTE_OUT_DSCP_INNER:
			ptable.val[12] |= (1 << 6);
			break;
		case GSW_ROUTE_OUT_DSCP_SESSION:
			ptable.val[12] |= (2 << 6);
			break;
		case GSW_ROUTE_OUT_DSCP_RES:
			ptable.val[12] |= (3 << 6);
			break;
		default:
			ptable.val[12] |= (0 << 6);
		}

		if (rpar->routeEntry.action.bInnerDSCPRemark == 1)
			ptable.val[12] |= (1 << 8);
		else
			ptable.val[12] &= ~(1 << 8);
		if (rpar->routeEntry.action.bTCremarking == 1)
			ptable.val[12] |= (1 << 9);
		else
			ptable.val[12] &= ~(1 << 9);
		if (rpar->routeEntry.action.bMeterAssign == 1) {
			ptable.val[12] |= (1 << 10);
			ptable.val[7] = rpar->routeEntry.action.nMeterId & 0x3F;
		} else
			ptable.val[12] &= ~(1 << 10);

		if (rpar->routeEntry.action.bTTLDecrement == 1)
			ptable.val[12] |= (1 << 12);
		else
			ptable.val[12] &= ~(1 << 12);

		if (rpar->routeEntry.action.bHitStatus == 1)
			ptable.val[12] |= (1 << 15);
		else
			ptable.val[12] &= ~(1 << 15);

	{
		int  step = 0, success = 0;
start:
		if (rthandler->rstbl.node[index].vflag == 1) {
			/* If entry is  valid , check its next */
			/* entry in the search link list*/
			if (rthandler->rstbl.node[index].nptr == -1) {
				/* This entry is the last node of the */
				/* search link list*/
				if (rthandler->rstbl.nfentries == 0) {
					/* if no free entry, cannot add this session. */
					if (rpar->bPrio == 1) {
						goto errexit;
						retval = GSW_ROUTE_F_SWAP_OUT_ERR /*GSW_ROUTE_F_SWAP_OUT*/;
					} else {
						goto errexit;
						retval = GSW_ROUTE_ERROR_RT_SESS_FULL;
					}
				} else {
					int temp;
					success = 1;
	/* Get free entry from the head of the free link list , add this session. */
					rthandler->rstbl.node[hindex].nventries++;
					temp = index;
					index = rthandler->rstbl.ffptr;
					rthandler->rstbl.node[index].hval = hindex; /* Store the hash value*/
					rthandler->rstbl.node[index].vflag = 1;
		/* TODO: Session info add for a index */
					rthandler->rstbl.nfentries--;
					rthandler->rstbl.nuentries++;
					rthandler->rstbl.ffptr = rthandler->rstbl.node[index].nptr;
					rthandler->rstbl.node[index].fflag = 0;
					rthandler->rstbl.node[index].prio = rpar->bPrio;
					rthandler->rstbl.node[temp].nptr = index;
					rthandler->rstbl.node[index].pprt = temp;
					rthandler->rstbl.node[index].nptr = -1;
					/* Update the HW table */
	/* Add this session to hardware table. It is last node of*/
	/* this search link list. */
					rthandler->rstbl.hw_table[index].hwvalid = 1;
					rthandler->rstbl.hw_table[index].hwnextptr = index;
					ptable.table = PCE_R_SESSION_INDEX;
					ptable.pcindex = index;
					ptable.valid = rthandler->rstbl.hw_table[index].hwvalid;
					ptable.key[5] = rthandler->rstbl.hw_table[index].hwnextptr;
					ptable.key[4] &= ~(1 << 15);
					ptable.key[4] |= ((rthandler->rstbl.hw_table[index].hwvalid) << 15);
					ptable.op_mode = OPMOD_ADDRESS_WRITE;
					route_table_write(cdev, &ptable);
					rpar->nRtIndex = index;
			/* Update the HW temp entry table */
			/* Link previous entry to this new entry. */
			/*rthandler->rstbl.hw_table[temp].hwvalid = 1;*/
					rthandler->rstbl.hw_table[temp].hwnextptr = index;
					memset(&ptable, 0, sizeof(pctbl_prog_t));
					ptable.table	= PCE_R_SESSION_INDEX;
					ptable.pcindex	= temp;
					ptable.op_mode = OPMOD_ADDRESS_READ;
					route_table_read(cdev, &ptable);

					ptable.table	= PCE_R_SESSION_INDEX;
					ptable.pcindex = temp; //rthandler->rstbl.hw_table[temp].hwnextptr;
					ptable.valid = rthandler->rstbl.hw_table[temp].hwvalid;
					ptable.key[5] = rthandler->rstbl.hw_table[temp].hwnextptr;
					ptable.key[4] &= ~(1 << 15);
					ptable.key[4] |= ((rthandler->rstbl.hw_table[temp].hwvalid) << 15);
					ptable.op_mode = OPMOD_RT_SESSION_NEXT;
					route_table_write(cdev, &ptable);
					if (index == -1)
						pr_err(" *SW BUG*** %s:%s:%d \n", __FILE__, __func__, __LINE__);
/*					rpar->nRtIndex = index;*/
					return success;
				}
			} else {
	/* Move to the next node of the search link list until the last node*/
		/*or search step is greater than 15.*/
				step++;
				if (step >= 15) {
					if (rpar->bPrio == 1) {
						goto errexit;
						retval = GSW_ROUTE_F_SWAP_OUT_ERR /*GSW_ROUTE_F_SWAP_OUT*/;
					} else {
						goto errexit;
						retval = GSW_ROUTE_ERROR_RT_COLL_FULL;
					}
				} else {
					index = rthandler->rstbl.node[index].nptr;
					goto start;
				}
			}
		} else {
			success = 1;
	/* If entry is not valid, can add the new session to this entry. */
			rthandler->rstbl.node[hindex].nventries++;
			rthandler->rstbl.node[index].hval = hindex; /* Store the hash value*/
			rthandler->rstbl.node[index].vflag = 1;
			rthandler->rstbl.node[index].prio = rpar->bPrio;
			rthandler->rstbl.nuentries++;
			/* TODO: Add Pattern */
			if (rthandler->rstbl.node[index].fflag == 1) {
				/* If this entry is free entry, */
				/* remove it from free link list. */
				rthandler->rstbl.node[index].fflag = 0;
				rthandler->rstbl.nfentries--;
				if (index == rthandler->rstbl.ffptr) {
					rthandler->rstbl.ffptr = rthandler->rstbl.node[index].nptr;
				} else {
					if (index == rthandler->rstbl.lfptr) {
						rthandler->rstbl.lfptr = rthandler->rstbl.node[index].pprt;
					} else {
						int temp1, temp2;
						temp1 = rthandler->rstbl.node[index].pprt;
						temp2 = rthandler->rstbl.node[index].nptr;
						rthandler->rstbl.node[temp2].pprt = temp1;
						rthandler->rstbl.node[temp1].nptr = temp2;
					}
				}
				/* This entry is the single node for this hash. */
				rthandler->rstbl.node[index].pprt = -1;
				rthandler->rstbl.node[index].nptr = -1;
	/*	Update the hardware data structure. This is the single node for */
	/*	this search link list add this session to hardware table. */
	/*	It is last node of this search link list. */
				rthandler->rstbl.hw_table[index].hwvalid = 1;
				rthandler->rstbl.hw_table[index].hwnextptr = index;
				/* TODO: Session update*/
				ptable.table	= PCE_R_SESSION_INDEX;
				ptable.pcindex	= index;
				ptable.valid = rthandler->rstbl.hw_table[index].hwvalid;
				ptable.key[5] = rthandler->rstbl.hw_table[index].hwnextptr;
				ptable.key[4] &= ~(1 << 15);
	/*ptable.key[4] |= ((rpar->routeEntry.pattern.bValid & 1) << 15); */
				ptable.key[4] |= ((rthandler->rstbl.hw_table[index].hwvalid) << 15);
				ptable.op_mode = OPMOD_ADDRESS_WRITE;
				route_table_write(cdev, &ptable);
				if (index == -1)
					pr_err(" *SW BUG*** %s:%s:%d \n", __FILE__, __func__, __LINE__);
				rpar->nRtIndex = index;
				return success;
			} else {
	/* If this entry is not free entry, just update hardware data structure */
	/*to add this session.Next pointer in hardware will be unchanged since */
	/*there are more entries in this search link list. */
	/* Update the HW table */
	/* Add this session to hardware table. It is last node */
	/* of this search link list. */
				rthandler->rstbl.hw_table[index].hwvalid = 1;
		/*rthandler->rstbl.hw_table[index].hwnextptr = unchanged */;
				/* TODO*/
				ptable.table	= PCE_R_SESSION_INDEX;
				ptable.pcindex	= index;
				ptable.valid = rthandler->rstbl.hw_table[index].hwvalid;
				ptable.key[5] = rthandler->rstbl.hw_table[index].hwnextptr;
				ptable.key[4] &= ~(1 << 15);
				ptable.key[4] |= ((rthandler->rstbl.hw_table[index].hwvalid) << 15);
				ptable.op_mode = OPMOD_ADDRESS_WRITE;
				route_table_write(cdev, &ptable);
				if (index == -1)
					pr_err(" * SW BUG*** %s:%s:%d \n", __FILE__, __func__, __LINE__);
				rpar->nRtIndex = index;
				return success;
			}
		}
	}
	return GSW_statusOk;
errexit:
		if (ppindex != 0xFF)
			rt_pppoe_tbl_delete(cdev, &rthandler->rt_sub_tbl, ppindex);
errexit6:
		if (dmindex != 0x0)
			rt_mac_tbl_delete(cdev, &rthandler->rt_sub_tbl, dmindex);
errexit5:
		if (smindex != 0x0)
			rt_mac_tbl_delete(cdev, &rthandler->rt_sub_tbl, smindex);
errexit4:
		if (mtindex != 0xFF)
			rt_mtu_tbl_delete(cdev, &rthandler->rt_sub_tbl, mtindex);
errexit3:
		if (aindex != 0x7FFF)
			rt_ip_tbl_delete(cdev, &rthandler->rt_sub_tbl, aindex);
errexit2:
		if (dindex != 0x7FFF)
			rt_ip_tbl_delete(cdev, &rthandler->rt_sub_tbl, dindex);
errexit1:
		if (sindex != 0x7FFF)
			rt_ip_tbl_delete(cdev, &rthandler->rt_sub_tbl, sindex);
		return retval;
	} else {
		return GSW_statusErr;
	}
}

int GSW_ROUTE_SessionEntryDel(void *cdev, GSW_ROUTE_Entry_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	int hindex,  index;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		index = rpar->nRtIndex;
		if ((index < 0) || (index >= 4096)) 
			return GSW_statusErr;
		hindex = rthandler->rstbl.node[index].hval;
		if (rthandler->rstbl.node[index].vflag == 1) {
		rthandler->rstbl.node[index].vflag = 0;
		if (rthandler->rstbl.node[hindex].nventries == 0)
			pr_err(" ****SW BUG ****: %s:%s:%d, hindex:%d, index:%d\n",__FILE__, __func__,
				__LINE__,hindex, index);
			rthandler->rstbl.node[hindex].nventries--;
			rthandler->rstbl.nuentries--;
			rthandler->rstbl.node[index].vflag = 0;
			rthandler->rstbl.node[index].prio = 0;
/*			rthandler->rstbl.node[index].fflag = 0;*/
			/* Update the HW table*/
			rthandler->rstbl.hw_table[index].hwvalid = 0;
/*	rthandler->rstbl.hw_table[index].hwnextptr = unchanged; */
			ptable.table = PCE_R_SESSION_INDEX;
			ptable.pcindex = index;
			ptable.op_mode = OPMOD_ADDRESS_READ;
			route_table_read(cdev, &ptable);
			if (((ptable.key[4] >> 15) & 1) == 1) {
				pctbl_prog_t rptable;
				int pcindex;
				memset(&rptable, 0, sizeof(pctbl_prog_t));
				rptable.table	= PCE_R_SESSION_INDEX;
				rptable.pcindex	= index;
				rptable.op_mode = OPMOD_ADDRESS_WRITE;
				rptable.valid = 0;
				rptable.key[5] = rthandler->rstbl.hw_table[index].hwnextptr;
	/*rptable.key[4] &= ~((rthandler->rstbl.hw_table[temp].hwvalid) << 15);*/
				route_table_write(cdev, &rptable);
				pcindex = (ptable.key[0] & 0xFFF);
				if (pcindex != 0xFFF)
					rt_ip_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);
				pcindex = (ptable.key[1] & 0x7FF);
				if (pcindex != 0xFFF)
					rt_ip_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);
				pcindex = (ptable.val[2] & 0xFFF);
				if (pcindex != 0xFFF)
					rt_ip_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);

				pcindex = (ptable.val[4] & 0x7);
				if (pcindex != 0xF)
					rt_mtu_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);

				pcindex = ((ptable.val[4] >> 8) & 0xFF);
				if (pcindex != 0x0)
					rt_mac_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);
				pcindex = (ptable.val[5] & 0x1FF);
				if (pcindex != 0x0)
					rt_mac_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);
				if ((ptable.val[12] >> 1) & 0x1) {
					pcindex = (ptable.val[6] & 0xF);
					rt_pppoe_tbl_delete(cdev, &rthandler->rt_sub_tbl, pcindex);
				}
			}
			if ((rthandler->rstbl.node[hindex].nventries == 0) && (rthandler->rstbl.node[hindex].vflag == 0)) {
				rt_free_entry(cdev, hindex);
			}
			if ((rthandler->rstbl.node[index].nventries == 0) && (index != hindex)) {
				rt_free_entry(cdev, index);
			}
		} else {
			pr_err(" No entry in the HW:%s:%s:%d, hindex:%d\n",__FILE__, __func__,
	__LINE__,rthandler->rstbl.node[index].hval);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_SessionEntryRead(void *cdev,
	GSW_ROUTE_Entry_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	u16 index;
	int i;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}

	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		int hindex, rindex;
		riptbl_t  iptbl;
		rt_mtu_tbl_t mvalue;
		rt_mac_tbl_t mentry;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		memset(&iptbl, 0, sizeof(riptbl_t));
		memset(&mvalue, 0, sizeof(rt_mtu_tbl_t));

		ptable.table	= PCE_R_SESSION_INDEX;
		ptable.pcindex	= rpar->nRtIndex;
		ptable.op_mode = OPMOD_ADDRESS_READ;
		route_table_read(cdev, &ptable);
		rindex = rpar->nRtIndex;
		hindex = rthandler->rstbl.node[rindex].hval;
#if defined(RT_DEBUG) && RT_DEBUG
		pr_err("%s:%s:%d rindex:%d,hindex:%d,\
		\nrthandler->rstbl.node[rindex].vflag:%d, \
		\nrthandler->rstbl.node[rindex].fflag:%d, \
		\nrthandler->rstbl.node[rindex].nventries:%d \
		\nrthandler->rstbl.ffptr:%d \
		\nrthandler->rstbl.nfentries:%d, \
		\nrthandler->rstbl.nuentries:%d, \
		\nrthandler->rstbl.node[rindex].nptr:%d, \
		\nrthandler->rstbl.node[rindex].pprt:%d, \
		\nrthandler->rstbl.lfptr:%d \
		\nrthandler->rstbl.hw_table[rindex].hwnextptr:%d \
		\nrthandler->rstbl.hw_table[rindex].hwvalid:%d \n", __FILE__, __func__, __LINE__, rindex, hindex, \
		rthandler->rstbl.node[rindex].vflag, \
		rthandler->rstbl.node[rindex].fflag, \
		rthandler->rstbl.node[rindex].nventries, \
		rthandler->rstbl.ffptr, \
		rthandler->rstbl.nfentries, \
		rthandler->rstbl.nuentries, \
		rthandler->rstbl.node[rindex].nptr, \
		rthandler->rstbl.node[rindex].pprt, \
		rthandler->rstbl.lfptr, \
		rthandler->rstbl.hw_table[rindex].hwnextptr, \
		rthandler->rstbl.hw_table[rindex].hwvalid);
#endif /* RT_DEBUG */
		if (((ptable.key[4] >> 15) & 1) == 1) {
			index = (ptable.key[0] & 0x7FF);
			if (index != 0x7ff)
				rt_iptable_rd(cdev, index, &iptbl);
			rpar->routeEntry.pattern.eIpType = iptbl.itype;
			if (rpar->routeEntry.pattern.eIpType ==
				GSW_RT_IP_V6) {
				for (i = 0;  i < 8; i++)
					rpar->routeEntry.pattern.nDstIP.nIPv6[i] =
					(iptbl.iaddr.i6addr[7 - i] & 0xFFFF);
			}	else if (rpar->routeEntry.pattern.eIpType ==
				GSW_RT_IP_V4) {
				rpar->routeEntry.pattern.nDstIP.nIPv4 =
					(iptbl.iaddr.i4addr[0] |
					(iptbl.iaddr.i4addr[1] << 8) |
					(iptbl.iaddr.i4addr[2] << 16) |
					(iptbl.iaddr.i4addr[3] << 24));
			}
			memset(&iptbl, 0, sizeof(riptbl_t));
			index = (ptable.key[1] & 0x7FF);
			if (index != 0x7ff)
				rt_iptable_rd(cdev, index, &iptbl);
			rpar->routeEntry.pattern.eIpType = iptbl.itype;
			if (rpar->routeEntry.pattern.eIpType ==
				GSW_RT_IP_V6) {
				for (i = 0;  i < 8; i++)
					rpar->routeEntry.pattern.nSrcIP.nIPv6[i] =
					(iptbl.iaddr.i6addr[7 - i] & 0xFFFF);
			} else {
				rpar->routeEntry.pattern.nSrcIP.nIPv4 =
					(iptbl.iaddr.i4addr[0] |
					(iptbl.iaddr.i4addr[1] << 8) |
					(iptbl.iaddr.i4addr[2] << 16) |
					(iptbl.iaddr.i4addr[3] << 24));
			}

			rpar->routeEntry.pattern.nDstPort = (ptable.key[2]);
			rpar->routeEntry.pattern.nSrcPort = (ptable.key[3]);
			rpar->routeEntry.pattern.nRoutExtId =
				(ptable.key[4] & 0xFF);
			rpar->routeEntry.pattern.bValid =
				((ptable.key[4] >> 15) & 0x1);

			rpar->routeEntry.action.nDstPortMap = (ptable.val[0]);
			rpar->routeEntry.action.nDstSubIfId =
				((ptable.val[1] >> 3) & 0x1FFF);

			memset(&iptbl, 0, sizeof(riptbl_t));
			index = (ptable.val[2] & 0x7FF);
			if (index != 0x7ff)
				rt_iptable_rd(cdev, index, &iptbl);
			rpar->routeEntry.action.eIpType = iptbl.itype;

			if (rpar->routeEntry.action.eIpType == GSW_RT_IP_V6) {
				for (i = 0;  i < 8; i++)
					rpar->routeEntry.action.nNATIPaddr.nIPv6[i] =
					(iptbl.iaddr.i6addr[7 - i] & 0xFFFF);
			} else if (rpar->routeEntry.action.eIpType ==
				GSW_RT_IP_V4) {
				rpar->routeEntry.action.nNATIPaddr.nIPv4 =
				(iptbl.iaddr.i4addr[0] |
				(iptbl.iaddr.i4addr[1] << 8) |
				(iptbl.iaddr.i4addr[2] << 16) |
				(iptbl.iaddr.i4addr[3] << 24));
			}

			rpar->routeEntry.action.nTcpUdpPort = (ptable.val[3]);

			index = (ptable.val[4] & 0x7);
			rt_mtutable_rd(cdev, index, &mvalue);
			rpar->routeEntry.action.nMTUvalue = mvalue.mtsize;

			memset(&mentry, 0, sizeof(rt_mac_tbl_t));
			index = ((ptable.val[4] >> 8) & 0xFF);
			if (index != 0) {
				rt_mactable_rd(cdev, index, &mentry);
				for (i = 0; i < 6; i++)
					rpar->routeEntry.action.nSrcMAC[i] =
					mentry.mdata[i];
				rpar->routeEntry.action.bMAC_SrcEnable = 1;
			}
			index = ((ptable.val[5]) & 0x1FF);
			memset(&mentry, 0, sizeof(rt_mac_tbl_t));
			if (index != 0) {
				rt_mactable_rd(cdev, index, &mentry);
				for (i = 0; i < 6; i++)
					rpar->routeEntry.action.nDstMAC[i] =
					mentry.mdata[i];
				rpar->routeEntry.action.bMAC_DstEnable = 1;
			}

			rpar->routeEntry.action.nFID = (ptable.val[8] & 0x3F);
			rpar->routeEntry.action.nFlowId =
			((ptable.val[8] >> 8) & 0xFF);
			rpar->routeEntry.action.nDSCP =
			(ptable.val[9] & 0x3F);
			rpar->routeEntry.action.nTrafficClass =
			((ptable.val[9] >> 8) & 0x3F);
			rpar->routeEntry.action.nSessionCtrs =
			(ptable.val[10]);
			rpar->routeEntry.action.nSessionCtrs |=
			((ptable.val[11] << 16));
			if ((ptable.val[12] >> 0) & 0x1)
				rpar->routeEntry.action.eSessDirection =
				GSW_ROUTE_DIRECTION_UPSTREAM;
			else
				rpar->routeEntry.action.eSessDirection =
				GSW_ROUTE_DIRECTION_DNSTREAM;

			if ((ptable.val[12] >> 1) & 0x1) {
				rt_ppoe_tbl_t sesid;
				index = (ptable.val[6] & 0xF);
				rt_ppoetable_rd(cdev, index, &sesid);
				rpar->routeEntry.action.nPPPoESessId =
				sesid.psesid;
				rpar->routeEntry.action.bPPPoEmode = 1;
			} else
				rpar->routeEntry.action.bPPPoEmode = 0;

			switch ((ptable.val[12] >> 2) & 0x3) {
			case 0:
				rpar->routeEntry.action.eSessRoutingMode =
					GSW_ROUTE_MODE_NULL;
					break;
			case 1:
				rpar->routeEntry.action.eSessRoutingMode =
					GSW_ROUTE_MODE_ROUTING;
					break;
			case 2:
				rpar->routeEntry.action.eSessRoutingMode =
					GSW_ROUTE_MODE_NAT;
					break;
			case 3:
				rpar->routeEntry.action.eSessRoutingMode =
					GSW_ROUTE_MODE_NAPT;
				break;
			}
			switch ((ptable.val[12] >> 4) & 0x3) {
			case 0:
				rpar->routeEntry.action.eTunType =
					GSW_ROUTE_TUNL_NULL;
					break;
			case 1:
				rpar->routeEntry.action.eTunType =
					GSW_ROUTE_TUNL_6RD;
					break;
			case 2:
				rpar->routeEntry.action.eTunType =
					GSW_ROUTE_TUNL_DSLITE;
					break;
			case 3:
				rpar->routeEntry.action.eTunType =
					GSW_ROUTE_TUNL_IPSEC;
					break;
			}
			rpar->routeEntry.action.nTunnelIndex =
			((ptable.val[6] >> 8) & 0xF);

			switch ((ptable.val[12] >> 6) & 0x3) {
			case 0:
				rpar->routeEntry.action.eOutDSCPAction =
					GSW_ROUTE_OUT_DSCP_NULL;
					break;
			case 1:
				rpar->routeEntry.action.eOutDSCPAction =
					GSW_ROUTE_OUT_DSCP_INNER;
					break;
			case 2:
				rpar->routeEntry.action.eOutDSCPAction =
					GSW_ROUTE_OUT_DSCP_SESSION;
					break;
			case 3:
				rpar->routeEntry.action.eOutDSCPAction =
					GSW_ROUTE_OUT_DSCP_RES;
					break;
			}
			if ((ptable.val[12] >> 8) & 0x1)
				rpar->routeEntry.action.bInnerDSCPRemark = 1;
			else
				rpar->routeEntry.action.bInnerDSCPRemark = 0;
			if ((ptable.val[12] >> 9) & 0x1)
				rpar->routeEntry.action.bTCremarking = 1;
			else
				rpar->routeEntry.action.bTCremarking = 0;
			if ((ptable.val[12] >> 10) & 0x1) {
				rpar->routeEntry.action.bMeterAssign = 1;
				rpar->routeEntry.action.nMeterId =
				ptable.val[7] & 0x3F;
			} else
				rpar->routeEntry.action.bMeterAssign = 0;

			if ((ptable.val[12] >> 11) & 0x1) {
				rt_rtp_tbl_t rtp_tbl;
				index = ((ptable.val[7] >> 8) & 0x3F);
				rt_rtptable_rd(cdev, index, &rtp_tbl);
				rpar->routeEntry.action.nRTPSeqNumber =
				rtp_tbl.rtpseqnum;
				rpar->routeEntry.action.nRTPSessionPktCnt =
				rtp_tbl.rtpsespcnt;
				rpar->routeEntry.action.bRTPMeasEna = 1;
			} else {
				rpar->routeEntry.action.bRTPMeasEna = 0;
			}
			if ((ptable.val[12] >> 12) & 0x1)
				rpar->routeEntry.action.bTTLDecrement = 1;
			else
				rpar->routeEntry.action.bTTLDecrement = 0;
			if ((ptable.val[12] >> 15) & 0x1)
				rpar->routeEntry.action.bHitStatus = 1;
			else
				rpar->routeEntry.action.bHitStatus = 0;
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_TunnelEntryAdd(void *cdev,
	GSW_ROUTE_Tunnel_Entry_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	int i, j;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		u16 data[20] = { 0 };
		if (rpar->tunnelEntry.eTunnelType == GSW_ROUTE_TUNL_6RD) {
			data[0] = 0x0045; data[1] = 0x5678;
			data[2] = 0x90ab; data[3] = 0x0000;
			data[4] = 0x293f; data[5] = 0x3344;
			data[6] =
			((rpar->tunnelEntry.t.tun6RD.nSrcIP4Addr.nIPv4 >> 24)
			& 0xFF);
			data[6] |=
			(((rpar->tunnelEntry.t.tun6RD.nSrcIP4Addr.nIPv4 >> 16)
			& 0xFF) << 8);
			data[7] =
			((rpar->tunnelEntry.t.tun6RD.nSrcIP4Addr.nIPv4 >> 8)
			& 0xFF);
			data[7] |=
			((rpar->tunnelEntry.t.tun6RD.nSrcIP4Addr.nIPv4
			& 0xFF) << 8);
			data[8] =
			((rpar->tunnelEntry.t.tun6RD.nDstIP4Addr.nIPv4 >> 24)
			& 0xFF);
			data[8] |=
			(((rpar->tunnelEntry.t.tun6RD.nDstIP4Addr.nIPv4 >> 16)
			& 0xFF) << 8);
			data[9] =
			((rpar->tunnelEntry.t.tun6RD.nDstIP4Addr.nIPv4 >> 8)
			& 0xFF);
			data[9] |=
			((rpar->tunnelEntry.t.tun6RD.nDstIP4Addr.nIPv4
			& 0xFF) << 8);
		} else if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_DSLITE) {
			data[0] = 0x0060; data[1] = 0x0000;
			data[2] = 0x0000; data[3] = 0xff04;
			for (i = 0; i < 8; i++) {
				data[i+4] =
				((rpar->tunnelEntry.t.tunDSlite.nSrcIP6Addr.nIPv6[i] >> 8)
				& 0xFF);
				data[i+4] |=
				((rpar->tunnelEntry.t.tunDSlite.nSrcIP6Addr.nIPv6[i]
				& 0xFF) << 8);
			}
			for (i = 0; i < 8; i++) {
				data[i+12] =
				((rpar->tunnelEntry.t.tunDSlite.nDstIP6Addr.nIPv6[i] >> 8)
				& 0xFF);
				data[i+12] |=
				((rpar->tunnelEntry.t.tunDSlite.nDstIP6Addr.nIPv6[i]
				& 0xFF) << 8);
			}
		} else if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_L2TP) {
			data[1] =
			(rpar->tunnelEntry.t.nTunL2TP & 0xFFFF);
			data[0] =
			((rpar->tunnelEntry.t.nTunL2TP >> 16) & 0xFFFF);
		} else if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_IPSEC) {
			data[1] = (rpar->tunnelEntry.t.nTunIPsec & 0xFFFF);
			data[0] =
			((rpar->tunnelEntry.t.nTunIPsec >> 16) & 0xFFFF);
		}
		for (j = 0; j < 5; j++) {
			memset(&ptable, 0, sizeof(pctbl_prog_t));
			for (i = 0; i < 4; i++)
				ptable.val[i] = data[(j*4)+i];
			ptable.table = PCE_R_TUNNEL_INDEX;
			ptable.pcindex = ((rpar->nTunIndex * 5) + j);
			ptable.valid = 1;
			ptable.op_mode = OPMOD_ADDRESS_WRITE;
			route_table_write(cdev, &ptable);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_TunnelEntryRead(void *cdev,
	GSW_ROUTE_Tunnel_Entry_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		int i, j;
		u16 data[20] = { 0 };

		for (j = 0; j < 5; j++) {
			memset(&ptable, 0, sizeof(pctbl_prog_t));
			ptable.table	= PCE_R_TUNNEL_INDEX;
			ptable.pcindex	= ((rpar->nTunIndex * 5) + j);
			ptable.op_mode = OPMOD_ADDRESS_READ;
			route_table_read(cdev, &ptable);
			for (i = 0; i < 4; i++)
				data[(j * 4) + i] = ptable.val[i];
		}
		if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_6RD) {
			rpar->tunnelEntry.t.tun6RD.nSrcIP4Addr.nIPv4 =
				((data[6] << 16) | (data[7]));
			rpar->tunnelEntry.t.tun6RD.nDstIP4Addr.nIPv4 =
				((data[8] << 16) | (data[9]));
		} else if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_DSLITE) {
			for (i = 0; i < 8; i++)
				rpar->tunnelEntry.t.tunDSlite.nSrcIP6Addr.nIPv6[i] =
				data[i+4];
			for (i = 0; i < 8; i++)
				rpar->tunnelEntry.t.tunDSlite.nDstIP6Addr.nIPv6[i] =
				data[i+12];
		} else if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_L2TP) {
			rpar->tunnelEntry.t.nTunIPsec =
			((data[0] << 16) | data[1]);
		} else if (rpar->tunnelEntry.eTunnelType ==
			GSW_ROUTE_TUNL_IPSEC) {
			rpar->tunnelEntry.t.nTunIPsec =
				((data[0] << 16) | data[1]);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_TunnelEntryDel(void *cdev,
	GSW_ROUTE_Tunnel_Entry_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		int j;
		for (j = 0; j < 5; j++) {
			memset(&ptable, 0, sizeof(pctbl_prog_t));
			ptable.table = PCE_R_TUNNEL_INDEX;
			ptable.pcindex = ((rpar->nTunIndex * 5) + j);
			ptable.op_mode = OPMOD_ADDRESS_WRITE;
			route_table_write(cdev, &ptable);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_L2NATCfgWrite(void *cdev,
	GSW_ROUTE_EgPort_L2NAT_Cfg_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		u16 data;
		if (rpar->nEgPortId >= ethdev->tpnum)
			return GSW_statusErr;
		gsw_w32(cdev,
			(PCE_PCTRL_2_L2NAT_OFFSET + (rpar->nEgPortId * 0xA)),
			PCE_PCTRL_2_L2NAT_SHIFT,
			PCE_PCTRL_2_L2NAT_SIZE, rpar->bL2NATEna);

		data = rpar->nNatMAC[4] << 8 | rpar->nNatMAC[5];
		gsw_w32(cdev,
			(PCE_L2NAT_MAC0_MAC0_OFFSET + (rpar->nEgPortId * 0x10)),
			PCE_L2NAT_MAC0_MAC0_SHIFT,
			PCE_L2NAT_MAC0_MAC0_SIZE, data);

		data = rpar->nNatMAC[2] << 8 | rpar->nNatMAC[3];
		gsw_w32(cdev,
			(PCE_L2NAT_MAC1_MAC1_OFFSET + (rpar->nEgPortId * 0x10)),
			PCE_L2NAT_MAC1_MAC1_SHIFT,
			PCE_L2NAT_MAC1_MAC1_SIZE, data);

		data = rpar->nNatMAC[0] << 8 | rpar->nNatMAC[1];
		gsw_w32(cdev,
			(PCE_L2NAT_MAC2_MAC2_OFFSET + (rpar->nEgPortId * 0x10)),
			PCE_L2NAT_MAC2_MAC2_SHIFT,
			PCE_L2NAT_MAC2_MAC2_SIZE, data);
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_L2NATCfgRead(void *cdev,
	GSW_ROUTE_EgPort_L2NAT_Cfg_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		u32 data;
		if (rpar->nEgPortId >= ethdev->tpnum)
			return GSW_statusErr;

		gsw_r32(cdev,
			(PCE_PCTRL_2_L2NAT_OFFSET + (rpar->nEgPortId * 0xA)),
			PCE_PCTRL_2_L2NAT_SHIFT,
			PCE_PCTRL_2_L2NAT_SIZE, &rpar->bL2NATEna);

		gsw_r32(cdev,
			(PCE_L2NAT_MAC0_MAC0_OFFSET + (rpar->nEgPortId * 0x10)),
			PCE_L2NAT_MAC0_MAC0_SHIFT,
			PCE_L2NAT_MAC0_MAC0_SIZE, &data);
		rpar->nNatMAC[5] = (data & 0xFF);
		rpar->nNatMAC[4] = ((data >> 8) & 0xFF);

		gsw_r32(cdev,
			(PCE_L2NAT_MAC1_MAC1_OFFSET + (rpar->nEgPortId * 0x10)),
			PCE_L2NAT_MAC1_MAC1_SHIFT,
			PCE_L2NAT_MAC1_MAC1_SIZE, &data);
		rpar->nNatMAC[3] = (data & 0xFF);
		rpar->nNatMAC[2] = ((data >> 8) & 0xFF);

		gsw_r32(cdev,
			(PCE_L2NAT_MAC2_MAC2_OFFSET + (rpar->nEgPortId * 0x10)),
			PCE_L2NAT_MAC2_MAC2_SHIFT,
			PCE_L2NAT_MAC2_MAC2_SIZE, &data);
		rpar->nNatMAC[1] = (data & 0xFF);
		rpar->nNatMAC[0] = ((data >> 8) & 0xFF);
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_SessHitOp(void *cdev,
	GSW_ROUTE_Session_Hit_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_SESSION_INDEX;
		ptable.pcindex = rpar->nRtIndex;
		ptable.op_mode = OPMOD_ADDRESS_READ;
		route_table_read(cdev, &ptable);
		if (((ptable.key[4] >> 15) & 1) == 1) {
			switch (rpar->eHitOper) {
			case GSW_ROUTE_HIT_READ:
				if ((ptable.val[12] >> 15) & 0x1)
					rpar->bHitStatus = 1;
				else
					rpar->bHitStatus = 0;
				break;
			case GSW_ROUTE_HIT_CLEAR:
				ptable.val[12] &= ~(1 << 15);
				ptable.table = PCE_R_SESSION_INDEX;
				ptable.pcindex = rpar->nRtIndex;
				ptable.op_mode = OPMOD_RT_SESSION_HIT_STATUS;
				route_table_write(cdev, &ptable);
				break;
			case GSW_ROUTE_HIT_N_CNTR_READ:
				if ((ptable.val[12] >> 15) & 0x1)
					rpar->bHitStatus = 1;
				else
					rpar->bHitStatus = 0;
				rpar->nSessCntr = (ptable.val[10]);
				rpar->nSessCntr |= ((ptable.val[11] << 16));
				break;
			case GSW_ROUTE_HIT_N_CNTR_CLEAR:
				if ((ptable.val[12] >> 15) & 0x1)
					rpar->bHitStatus = 1;
				else
					rpar->bHitStatus = 0;
				ptable.val[12] &= ~(1 << 15);
				rpar->nSessCntr = (ptable.val[10]);
				rpar->nSessCntr |= ((ptable.val[11] << 16));
				ptable.val[10] = 0;
				ptable.val[11] = 0;
				ptable.table = PCE_R_SESSION_INDEX;
				ptable.pcindex = rpar->nRtIndex;
				ptable.op_mode = OPMOD_RT_SESSION_WRITE;
				route_table_write(cdev, &ptable);
				break;
			}
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

int GSW_ROUTE_SessDestModify(void *cdev,
	GSW_ROUTE_Session_Dest_t *rpar)
{
	ethsw_api_dev_t *ethdev = (ethsw_api_dev_t *)cdev;
	if (ethdev == NULL) {
		pr_err("%s:%s:%d \n",__FILE__, __func__, __LINE__);
		return GSW_statusErr;
	}
	if ((ethdev->gipver == LTQ_GSWIP_3_0)
		&& ((ethdev->gsw_dev == LTQ_FLOW_DEV_INT_R))) {
		pctbl_prog_t ptable;
		memset(&ptable, 0, sizeof(pctbl_prog_t));
		ptable.table = PCE_R_SESSION_INDEX;
		ptable.pcindex = rpar->nRtIdx;
		ptable.op_mode = OPMOD_ADDRESS_READ;
		route_table_read(cdev, &ptable);
		if (((ptable.key[4] >> 15) & 1) == 1) {
			/* Destination Port Map  */
			ptable.val[0] = (rpar->nDstPortMap & 0xFFFF);
			/*Destination Sub-Interface ID*/
			ptable.val[1] = ((rpar->nDstSubIfId & 0x1FFF) << 3);
			ptable.table = PCE_R_SESSION_INDEX;
			ptable.pcindex = rpar->nRtIdx;
			ptable.op_mode = OPMOD_ADDRESS_WRITE;
			route_table_write(cdev, &ptable);
		}
	} else {
		return GSW_statusErr;
	}
	return GSW_statusOk;
}

/* RT Table Init routine */
int rt_table_init()
{
	int index;
	PCE_ASSERT(rthandler == NULL);
	memset(&rthandler->rt_sub_tbl, 0, sizeof(rt_table_handle_t));
	memset(&rthandler->rstbl, 0, sizeof(rt_session_tbl_t));
	rthandler->rstbl.nfentries = 4096;
	rthandler->rstbl.nuentries = 0;
	rthandler->rstbl.ffptr = 0;
	rthandler->rstbl.lfptr = 4095;
	for (index = 0; index < rthandler->rstbl.nfentries; index++) {
		rthandler->rstbl.node[index].vflag = 0;
		rthandler->rstbl.node[index].pprt = (index == 0) ? (-1) : (index-1);
		rthandler->rstbl.node[index].nptr = (index == 4095)? (-1) : (index + 1);
		rthandler->rstbl.node[index].nventries = 0;
		rthandler->rstbl.node[index].fflag = 1;
		rthandler->rstbl.node[index].prio = 0;
		rthandler->rstbl.node[index].hval = 0;

		rthandler->rstbl.hw_table[index].hwvalid = 0;
		rthandler->rstbl.hw_table[index].hwnextptr = index;
	}
	return GSW_statusOk;
}

int gsw_r_init()
{
	int index;
/*	rthandler = (ltq_rt_table_t *) kmalloc(sizeof(ltq_rt_table_t),*/
/*		GFP_KERNEL);*/
	if (rthandler)
		kfree(rthandler);
	rthandler = kmalloc(sizeof(ltq_rt_table_t), GFP_KERNEL);
	PCE_ASSERT(rthandler == NULL);
	memset(&rthandler->rt_sub_tbl, 0, sizeof(rt_table_handle_t));
	memset(&rthandler->rstbl, 0, sizeof(rt_session_tbl_t));
	rthandler->rstbl.nfentries = 4096;
	rthandler->rstbl.nuentries = 0;
	rthandler->rstbl.ffptr = 0;
	rthandler->rstbl.lfptr = 4095;
	for (index = 0; index < rthandler->rstbl.nfentries; index++) {
		rthandler->rstbl.node[index].vflag = 0;
		rthandler->rstbl.node[index].pprt = (index == 0) ? (-1) : (index-1);
		rthandler->rstbl.node[index].nptr = (index == 4095)? (-1) : (index + 1);
		rthandler->rstbl.node[index].nventries = 0;
		rthandler->rstbl.node[index].fflag = 1;
		rthandler->rstbl.node[index].prio = 0;
		rthandler->rstbl.node[index].hval = 0;
		rthandler->rstbl.hw_table[index].hwvalid = 0;
		rthandler->rstbl.hw_table[index].hwnextptr = index;
	}
	return GSW_statusOk;
}
