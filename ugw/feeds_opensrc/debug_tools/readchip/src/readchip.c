#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

//#include "chipid.h"

typedef int u32;
typedef short u16;
typedef char u8;

#define CHIPID_ID_CFG 0x1F107350u
#define CHIPID_MPS_CHIPID 0x1F107344u
#define CHIPID_MPS_CHIP_LOC0 0x1F10734Cu
#define CHIPID_MPS_CHIP_LOC1 0x1F10736Cu
#define CHIPID_FAB_LOT_ID0 0x1F107354u
#define CHIPID_FAB_LOT_ID1 0x1F107358u
#define CHIPID_MPS_MANID 0x1F107340u

int mem_dump (int start_address);

u32 grx500_check_efuse() {
	u32 id, config_id, dev_configid, chipid;
	
	dev_configid = mem_dump(CHIPID_ID_CFG);
	chipid = mem_dump(CHIPID_MPS_CHIPID);

	id= ((chipid&0xffff000)>>12);
	switch(id) {
		case 0x0020 : 	
				config_id = 0x40010000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0021 : 	
				config_id = 0x40010000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0022 : 	
				config_id = 0x40092000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0023 : 	
				config_id = 0x40010000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0024 : 	
				config_id = 0x40010000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0025 : 	
				config_id = 0x40010000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0026 : 	
				config_id = 0x40130000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		case 0x0028 : 	
				config_id = 0x40130000;
				if(dev_configid!= config_id) 
					printf("\tWrong config fusing (0x%08x) vs (0x%08x) for id (0x%04x)\n",dev_configid,config_id,id) ;
				break;
		default :	printf("Error -> Not a GRX350/550 series device!!!\n");
				break ;
	}	

}



int main(int argc, char** argv) {
    	u16 wafer_nbr, coordinate_x, coordinate_y;
	u8 year, month, day;

	int retValue;
	int manid;
	int chipid;
	int chiploc;
	int chiploc1;
	int configid;
	int lotid0;
	int lotid1;

	int temp, i;
	char id[8];

	manid = mem_dump(CHIPID_MPS_MANID);

	chipid = mem_dump(CHIPID_MPS_CHIPID);
	if((chipid == 0xffffffff) || (chipid == 0x00000000)) {
		printf("\tDEVICE is not FUSED\n");
		return;
	} else {
		printf("\tPart Number\t: ");
		switch((chipid&0xffff000)>>12) {
			case 0x0020 : 	printf("G583 Pxb4583el-1600");
				break;
			case 0x0021 : 	printf("G582 Pxb4582el-1600");
				break;
			case 0x0022 : 	printf("G562 Pxb4562el-1600");
				break;
			case 0x0023 : 	printf("G582 Pxb4582el-2000");
				break;
			case 0x0024 : 	printf("G583 Pxb4583el-2000");
				break;
			case 0x0025 : 	printf("G583 Pxb4583el-2400");
				break;
			case 0x0026 : 	printf("G350-6 Pxb4395el-1200");
				break;
			case 0x0028 : 	printf("G350-8 Pxb4395el-1600");
				break;
			default :	printf("Unknown");
				break ;
		}
		
		printf("\n\tVersion\t\t: ");
		switch(((chipid)>>28)&7) {
			case 0x00 : 	printf("UnFuse");
				break;
			case 0x01 : 	printf("V11");
				break;
			case 0x02 : 	printf("V12");
				break;
			case 0x03 : 	printf("3");
				break;
			case 0x04 : 	printf("4");
				break;
			case 0x05 : 	printf("5");
				break;
			case 0x06 : 	printf("6");
				break;
			default :	printf("Unknown");
				break ;
		}

		printf("\n");
	}

	chiploc = mem_dump(CHIPID_MPS_CHIP_LOC0);
	chiploc1 = mem_dump(CHIPID_MPS_CHIP_LOC1);
	temp = chiploc;
	for(i=0; i<8; i++) {
		id[7-i] = (char)((temp &0x3f)+48);
		temp = temp >> 6;
		if(i == 3)
			temp = temp | (chiploc1 <<6);
	}
	printf("\tWafer Lot Code\t: ");
	for(i=0;i<8;i++) printf("%c",id[i]);
	printf("\n");

	configid = mem_dump(CHIPID_ID_CFG);
	lotid0 = mem_dump(CHIPID_FAB_LOT_ID0);
	lotid1 = mem_dump(CHIPID_FAB_LOT_ID1);

	wafer_nbr = (u16)(chiploc1 >> 27);
	coordinate_y = (u8)((lotid1 >> 7)&0x7f);
	coordinate_x = (u8)((lotid1)&0x7f);
	year  = (u8) ((lotid0>>26)&0x1f);
	month = (u8) ((lotid0>>22)&0xf);
	day   = (u8) ((lotid0>>17)&0x1f);
	printf("\tChip Wafer Date\t: YEAR:%d MONTH:%d DAY:%d\n", (2000+year), month, day);
	printf("\tChip Wafer ID\t: 0x%X\n", wafer_nbr); 
	printf("\tX,Y LOC\t\t: 0x%X,0x%X\n", coordinate_x, coordinate_y);

	grx500_check_efuse();
#if 0
	printf("\tChip ID Reg: 0x%08x\n", chipid);
	printf("\tMan ID Reg: 0x%08x\n", manid);
	printf("\tConfig Reg: 0x%08x\n", configid);
	printf("\tChip XY LOC: 0x%08x\n", chiploc);
#endif

}
