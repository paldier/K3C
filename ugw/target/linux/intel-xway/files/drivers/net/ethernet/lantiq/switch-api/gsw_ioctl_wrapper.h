/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#ifndef _LTQ_ETHSW_API_LINUX_H_
#define _LTQ_ETHSW_API_LINUX_H_

#define PARAM_BUFFER_SIZE			2048
typedef enum {
	ETHSW_API_USERAPP = 0,
	ETHSW_API_KERNEL,
} ethsw_api_type_t;

/* general declaration fits for all low-level functions. */
typedef int (*LTQ_ll_fkt) (void *, u32);
typedef struct ltq_lowlevel_fkts_t ltq_lowlevel_fkts_t;
/* Switch API low-level function tables to map all supported IOCTL commands */
struct ltq_lowlevel_fkts_t {
	/*Some device have multiple tables to split the generic
	switch API features and the device specific switch API features.
	Additional tables, if exist,	can be found under this next pointer.
	Every table comes along with a different 'nType'
	parameter to differentiate.*/
	ltq_lowlevel_fkts_t	*pNext;
	/* IOCTL type of all commands listed in the table. */
	u16 nType;
	/* Number of low-level functions listed in the table. */
	u32 nNumFkts;
	/* Pointer to the first entry of the ioctl number table.
	This table is used to check if the given ioctl command fits
	the the found low-level function pointer under 'pFkts'.*/
	/* u32	*pIoctlCmds;*/
	/* Pointer to the first entry of the function table.
	Table size is given by the parameter 'nNumFkts'. */
	LTQ_ll_fkt	*pFkts;
};
/* function type declaration for the default IOCTL low-level function in
   case the command cannot be found in the low-level function table,
	or in case no low-level function table is provided.. */
typedef int (*ioctl_default_fkt) (void*, int, int);
/*typedef*/
typedef struct {
	ltq_lowlevel_fkts_t *pLlTable;
	void *pLlHandle;
	char paramBuffer[PARAM_BUFFER_SIZE];
	/** Default callback handler. This handler is called	\
	in case the command cannot be found in the low-level	\
	function table, or in case no low-level function table is provided.
	Provide a 'NULL' pointer in case no default handler is provided. */
	ioctl_default_fkt default_handler;
} ioctl_cmd_handle_t;

/*typedef*/
typedef struct {
	ltq_bool_t	bInternalSwitch;
	/** Number of similar Low Level External Switch Devices */
	u8 nExternalSwitchNum;
	ioctl_cmd_handle_t *pIoctlHandle;
	/** Array of pEthSWDev pointers associated with this driver context */
	void *pEthSWDev[LTQ_GSW_DEV_MAX];
} ioctl_wrapper_ctx_t;

/*typedef*/
typedef struct {
	ltq_lowlevel_fkts_t	*pLlTable;
	/** Default callback handler. This handler is called in case the command
	    cannot be found in the low-level function table, or in case no low-level
	    function table is provided.
	    Provide a 'NULL' pointer in case no default handler is provided. */
	ioctl_default_fkt		default_handler;
} ioctl_wrapper_init_t;

/*typedef*/
typedef struct {
	u8				minor_number;
} dev_minor_num_t;

int gsw_api_drv_register(u32 major);
int gsw_api_drv_unregister(u32 major);
void	*ioctl_wrapper_init(ioctl_wrapper_init_t *pinit);
int ioctl_wrapper_dev_add(ioctl_wrapper_ctx_t *pioctldev,
	void *pcoredev, u8 mnum);
int gsw_api_ioctl_wrapper_cleanup(void);

#endif    /* _LTQ_ETHSW_API_LINUX_H_ */
