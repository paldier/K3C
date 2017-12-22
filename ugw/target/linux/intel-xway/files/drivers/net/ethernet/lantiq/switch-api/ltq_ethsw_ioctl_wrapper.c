/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "ltq_ethsw_init.h"
#include <linux/interrupt.h>

ioctl_wrapper_ctx_t *pIOCTL_Wrapper_Ctx;

#define ETHSW_API_DEV_NAME "switch_api"


static spinlock_t swapi_sem;
static int gsw_api_open(struct inode *inode, struct file *filp);
static int gsw_api_release(struct inode *inode, struct file *filp);
static long gsw_api_ioctl(struct file *filp,\
	u32 nCmd, unsigned long nArg);

/** searching for Switch API IOCTL  command */
int gsw_command_search(void *pHandle, int command, int arg, ethsw_api_type_t apiType)
{
	int retvalue;
	ioctl_cmd_handle_t *pDrv = (ioctl_cmd_handle_t *)pIOCTL_Wrapper_Ctx->pIoctlHandle;
	ltq_lowlevel_fkts_t *pLlTable = pDrv->pLlTable;
	char *pParamBuffer;
	
	while (pLlTable != NULL) {
		if (_IOC_TYPE(command) == pLlTable->nType) {
	/* This table contains the low-level function for the IOCTL	\
	commands with the same MAGIC-Type numer. */
			LTQ_ll_fkt fkt;
			u32 size;
			u32 cmdNr = _IOC_NR(command);
	/* Number out of range. No function available for	\
		this command number. */
			if (cmdNr >= pLlTable->nNumFkts)
				return -1;
			fkt = pLlTable->pFkts[cmdNr];
			/* No low-level function given for this command. */
			if (fkt == NULL) {
				pr_err("ERROR %s[%d]: cmdNr=%d, nNumFkts=%d \n",\
					__func__, __LINE__, cmdNr, pLlTable->nNumFkts);
				return -1;
			}
			/* Copy parameter from userspace. */
			size = _IOC_SIZE(command);
	/* Local temporary buffer to store the parameter is to small. */
			if (size > PARAM_BUFFER_SIZE) {
				pr_err("ERROR %s[%d]: cmdNr=%d, nNumFkts=%d \n",	\
				__func__, __LINE__, cmdNr, pLlTable->nNumFkts);
				return -1;
			}
			pParamBuffer = kzalloc(size,GFP_KERNEL);
			if(pParamBuffer == NULL){
				pr_err("ERROR %s[%d]: Buffer allocation failed in switch api user ioctl \n",	\
				__func__, __LINE__);
				return -1;	
			}
			if (apiType == ETHSW_API_USERAPP) {
				copy_from_user((void *)(pParamBuffer), (const void __user *)arg, (unsigned long)size);
				/*Take spinlock*/
				raw_spin_lock_bh(&swapi_sem.rlock);
				/* Now call the low-level function with the right low-level context \
				handle and the local copy of the parameter structure of 'arg'. */
				retvalue = fkt(pHandle, (u32)pParamBuffer);
				/*Release spinlock*/
				raw_spin_unlock_bh(&swapi_sem.rlock);
				/* Copy parameter to userspace. */
				if (_IOC_DIR(command) & _IOC_READ) {
					/* Only copy back to userspace if really required */
					copy_to_user((void __user *)arg, (const void *)(pParamBuffer), (unsigned long)size);
				}
			} else {
				memcpy((void *)(pParamBuffer), (const void *) arg, (unsigned long)size);
				/*Take spinlock*/
				raw_spin_lock_bh(&swapi_sem.rlock);
				retvalue = fkt(pHandle, (u32)pParamBuffer);
				/*Release spinlock*/
				raw_spin_unlock_bh(&swapi_sem.rlock);
				memcpy((void *)arg, (const void *)(pParamBuffer), (unsigned long)size);
			}
			kfree(pParamBuffer);
			return retvalue;
		}
		/* If command was not found in the current table index, look for the next \
		linked table. Search till it is found or we run out of tables. */
		pLlTable = pLlTable->pNext;
	}
	if (pDrv->default_handler != NULL) {
		/*Take spinlock*/
		raw_spin_lock_bh(&swapi_sem.rlock);
		retvalue = pDrv->default_handler(pHandle, command, arg);
		/*Release spinlock*/
		raw_spin_unlock_bh(&swapi_sem.rlock);
		return retvalue;
	}
   /* No supported command low-level function found. */
   return -1;
}
/** The driver callbacks that will be registered with the kernel*/
/*static*/
const struct file_operations swapi_fops = {
	owner: THIS_MODULE,
	unlocked_ioctl : gsw_api_ioctl,
	open : gsw_api_open,
	release : gsw_api_release
};

static long gsw_api_ioctl(struct file *filp, u32 nCmd, unsigned long nArg)
{
	dev_minor_num_t *p;
	int ret;
	ioctl_wrapper_ctx_t *pDev;
	void *pLlHandle;

	p = filp->private_data;
	pDev = pIOCTL_Wrapper_Ctx;
	if (!p->minor_number) {
		if (pDev->bInternalSwitch == LTQ_TRUE)
			pLlHandle = pDev->pEthSWDev[0];
		else {
			pr_err("\nNot support internal switch\n\n");
			return -1;
		}
	} else {
		if (p->minor_number <= pDev->nExternalSwitchNum) {
			pLlHandle = pDev->pEthSWDev[p->minor_number];
		} else {
			pr_err("(Not support external switch number: %d) %s:%s:%d \n",	\
				p->minor_number, __FILE__, __func__, __LINE__);
			return -1;
		}
	}
	ret = gsw_command_search(pLlHandle, nCmd, nArg, ETHSW_API_USERAPP);
	return ret;
}

static int gsw_api_open(struct inode *inode, struct file *filp)
{
	u32 minorNum, majorNum;
	dev_minor_num_t *p;

	minorNum = MINOR(inode->i_rdev);
	majorNum = MAJOR(inode->i_rdev);
	p = kmalloc(sizeof(dev_minor_num_t), GFP_KERNEL);
	if (!p) {
		pr_err("%s memory allocation failed !!\n", __func__);
		return -ENOMEM;
	}
	p->minor_number = minorNum;
	filp->private_data = p;
	return 0;
}

static int gsw_api_release(struct inode *inode, struct file *filp)
{
	if (filp->private_data) {
		kfree(filp->private_data);
		filp->private_data = NULL;
	}
	return 0;
}

int gsw_api_drv_register(u32 Major)
{
	int result;
	result = register_chrdev(Major, ETHSW_API_DEV_NAME, &swapi_fops);
	if (result < 0) {
		pr_err("SWAPI: Register Char Dev failed with %d !!!\n", result);
		return result;
	}
	pr_info("SWAPI: Registered char device [%s] with major no [%d]\n", ETHSW_API_DEV_NAME, Major);
	return 0;
}

int gsw_api_drv_unregister(u32 Major)
{
	unregister_chrdev(Major, ETHSW_API_DEV_NAME);
	return 0;
}

void *ioctl_wrapper_init(ioctl_wrapper_init_t *pInit)
{
	u8 i;
	ioctl_wrapper_ctx_t *pDev;
	pDev = (ioctl_wrapper_ctx_t *)kmalloc(sizeof(ioctl_wrapper_ctx_t), GFP_KERNEL);
	if (!pDev) {
		pr_err("%s memory allocation failed !!\n", __func__);
		return pDev;
	}
	pDev->bInternalSwitch = 0;  /* internal switch, the value is 0 */
	pDev->nExternalSwitchNum = 0;
	pDev->pIoctlHandle = (ioctl_cmd_handle_t *)kmalloc(sizeof(ioctl_cmd_handle_t), GFP_KERNEL);
	if (!pDev->pIoctlHandle) {
		pr_err("%s memory allocation failed !!\n", __func__);
		if (pDev)
			kfree(pDev);
		return pDev->pIoctlHandle;
	}
	pDev->pIoctlHandle->pLlTable = pInit->pLlTable;
	pDev->pIoctlHandle->default_handler = pInit->default_handler;
	for (i = 0; i < LTQ_GSW_DEV_MAX; i++)
		pDev->pEthSWDev[i] = NULL;
	pIOCTL_Wrapper_Ctx = pDev;	
	spin_lock_init(&swapi_sem);
	return pDev;
}

int ioctl_wrapper_dev_add(ioctl_wrapper_ctx_t *pIoctlDev, void *pCoreDev, u8 nMinorNum)
{
	if (nMinorNum > LTQ_GSW_DEV_MAX) {
		pr_err("(Device number: %d) %s:%s:%d \n",	\
			nMinorNum, __FILE__, __func__, __LINE__);
		return -1;
	}
	pIoctlDev->pEthSWDev[nMinorNum] = pCoreDev;
	if (!nMinorNum)
		pIoctlDev->bInternalSwitch = LTQ_TRUE;
	else /* other than 0 means external switch */
		pIoctlDev->nExternalSwitchNum++;
	return 0;
}

int gsw_api_ioctl_wrapper_cleanup(void)
{
	ioctl_wrapper_ctx_t *pDev = pIOCTL_Wrapper_Ctx;
	if (pDev != NULL) {
		if (pDev->pIoctlHandle != NULL) {
			kfree(pDev->pIoctlHandle);
			pDev->pIoctlHandle = NULL;
		}
		kfree(pDev);
		pDev = NULL;
	}
	return 0;
}

LTQ_ETHSW_API_HANDLE ltq_ethsw_api_kopen(char *name)
{
	ioctl_wrapper_ctx_t *pDev;
	/* process /dev/switch/minor string */
	char *needle = "/";
	char *buf = strstr(name, needle);
	pDev = pIOCTL_Wrapper_Ctx;
	name = buf+strlen(needle); /* pointer to dev */
	buf = strstr(name, needle);
	name = buf+strlen(needle); /* pointer to switch */
	buf = strstr(name, needle);
	name = buf+strlen(needle); /* pointer to minor */
	if (!strcmp(name, "0")) {
		if (pDev->bInternalSwitch == LTQ_TRUE)
			return (LTQ_ETHSW_API_HANDLE)(pDev->pEthSWDev[0]);
		else {
			pr_err("\nNot support internal switch\n\n");
			return 0;
		}
	} else if (!strcmp(name, "1")) {
		return (LTQ_ETHSW_API_HANDLE)(pDev->pEthSWDev[1]);
	} else {
		pr_err("\nNot support external switch number = %s\n\n", name);
		return 0;
	}
	return 0;
}
EXPORT_SYMBOL(ltq_ethsw_api_kopen);

int ltq_ethsw_api_kioctl(LTQ_ETHSW_API_HANDLE handle, u32 command, u32 arg)
{
	if (unlikely(in_irq())) {
    pr_err("Not allowed to call kioctl in_irq mode\n");
    return -1;
  }
 return gsw_command_search((void *)handle, command, arg, ETHSW_API_KERNEL);
}
EXPORT_SYMBOL(ltq_ethsw_api_kioctl);

int ltq_ethsw_api_kclose(LTQ_ETHSW_API_HANDLE handle)
{
	return 0;
}
EXPORT_SYMBOL(ltq_ethsw_api_kclose);
