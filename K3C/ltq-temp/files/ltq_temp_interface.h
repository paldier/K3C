/******************************************************************************

			Copyright (c) 2010
			Lantiq Deutschland GmbH
			Am Campeon 3; 85579 Neubiberg, Germany

	For licensing information, see the file 'LICENSE' in the root folder
	of this software module.

******************************************************************************/
/**
	\file drv_temp_interface.h
	This file holds the kernel API function header.
*/
#ifndef _ltq_temp_interface_h
#define _ltq_temp_interface_h

#define TS_TEMP_OOR	999999

enum {	SHOW_TEMP1 =	0,
	SHOW_TEMP2 =	1,
	SHOW_TEMP3 =	2,
	SHOW_TEMP4 =	3,
	SHOW_TEMP5 =	4,
	SHOW_TEMP6 =	5,
	SHOW_TEMP7 =	6,
	SHOW_TEMP8 =	7,
	SHOW_NAME  =	8,
	SHOW_CRIT  =	9
};

/* kernel API functions */
extern int ts_get_current_temperature(unsigned long *cur_temp,
							int sensor_select);

#endif /* _ltq_temp_interface_h */
