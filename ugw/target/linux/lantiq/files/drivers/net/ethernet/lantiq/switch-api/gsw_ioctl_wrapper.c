/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#include "gsw_init.h"

ioctl_wrapper_ctx_t *ioctlwrapctx = NULL;

#define ETHSW_API_DEV_NAME "switch_api"

static spinlock_t swapi_sem;

static int gsw_api_open(struct inode *inode, struct file *filp);
static int gsw_api_release(struct inode *inode, struct file *filp);
static long gsw_api_ioctl(struct file *filp, u32 cmd, unsigned long arg);

int gsw_spi_lock_init(void)
{
	spin_lock_init(&swapi_sem);
	return 0;
}

int gsw_api_lock(void)
{
	if (unlikely(in_irq())) {
		pr_err("Not allowed to call gsw_api_lock in_irq mode\n");
		return -1;
	}
	raw_spin_lock_bh(&swapi_sem.rlock);
	return 0;
}

int gsw_api_unlock(void)
{
	if (unlikely(in_irq())) {
		pr_err("Not allowed to call gsw_api_unlock in_irq mode\n");
		return -1;
	}
	raw_spin_unlock_bh(&swapi_sem.rlock);
	return 0;
}

ioctl_cmd_handle_t *gsw_api_alloc_cmd_handle(void)
{
	gfp_t flags;
	ioctl_cmd_handle_t *cmd_handle;
	
	if (!ioctlwrapctx) {
		pr_err("ioctlwrapctx not initilized\n");
		return NULL;
	}
	if ( in_atomic() || in_interrupt() )
		flags = GFP_ATOMIC;
	else
		flags = GFP_KERNEL;
	cmd_handle = kmalloc(sizeof(ioctl_cmd_handle_t), flags);
	if (cmd_handle) {
		cmd_handle->pLlTable = ioctlwrapctx->pIoctlHandle->pLlTable;
		cmd_handle->default_handler = ioctlwrapctx->pIoctlHandle->default_handler;
	}
	return cmd_handle;
}


/** searching for Switch API IOCTL  command */
int gsw_command_search(void *phandle, u32 command,
	u32 arg, ethsw_api_type_t apitype)
{
	int retvalue;
	ioctl_cmd_handle_t *pdrv = (ioctl_cmd_handle_t *) phandle;
	ltq_lowlevel_fkts_t *pLlTable = pdrv->pLlTable;
	/*  attempt to acquire the semaphore ...*/
 	
/* This table contains the low-level function for the */
/* IOCTL	commands with the same MAGIC-Type numer. */
	while (pLlTable != NULL) {
		if (_IOC_TYPE(command) == pLlTable->nType) {
			LTQ_ll_fkt fkt;
			u32 size;
			u32 cmdnr = _IOC_NR(command);
	/* Number out of range. No function available */
	/* for	this command number. */
			if (cmdnr >= pLlTable->nNumFkts)
				goto fail;
			fkt = pLlTable->pFkts[cmdnr];
			/* No low-level function given for this command. */
			if (fkt == NULL) {
				pr_err("ERROR %s[%d]: cmdnr=%d, nNumFkts=%d\n",
				__func__, __LINE__, cmdnr, pLlTable->nNumFkts);
				goto fail;
			}
			/* Copy parameter from userspace. */
			size = _IOC_SIZE(command);
	/* Local temporary buffer to store the parameter is to small. */
			if (size > PARAM_BUFFER_SIZE) {
				pr_err("ERROR %s[%d]: cmdnr=%d, nNumFkts=%d\n",
				__func__, __LINE__, cmdnr, pLlTable->nNumFkts);
				goto fail;
			}
			if (apitype == ETHSW_API_USERAPP) {
				copy_from_user((void *)(pdrv->paramBuffer),
				(const void __user *)arg, (unsigned long)size);
	/* Now call the low-level function with the right low-level context */
	/* handle and the local copy of the parameter structure of 'arg'. */
				if (gsw_api_lock())
					return -1;
				retvalue = fkt(pdrv->pLlHandle,
				(u32)pdrv->paramBuffer);
				gsw_api_unlock();
		/* Copy parameter to userspace. */
		/* Only copy back to userspace if really required */
				if (_IOC_DIR(command) & _IOC_READ) {
					copy_to_user((void __user *)arg,
					(const void *)(pdrv->paramBuffer),
					(unsigned long)size);
				}
			} else {
				memcpy((void *)(pdrv->paramBuffer),
				(const void *) arg, (unsigned long)size);
				if (gsw_api_lock())
					return -1;
				retvalue = fkt(pdrv->pLlHandle,
				(u32)pdrv->paramBuffer);
				gsw_api_unlock();
				memcpy((void *)arg,
				(const void *)(pdrv->paramBuffer),
				(unsigned long)size);
			}
/*	pr_err(" %s[%d]: cmdnr=%d, nNumFkts=%d, retvalue:%d\n",*/
	/*__func__, __LINE__, cmdnr, pLlTable->nNumFkts, retvalue);*/
			return retvalue;
		}
	/* If command was not found in the current table index, */
	/* look for the next linked table. Search till it is found */
	/* or we run out of tables.*/
		pLlTable = pLlTable->pNext;
	}
	if (pdrv->default_handler != NULL) {
		if(gsw_api_lock())
			return -1;
		retvalue = pdrv->default_handler(pdrv->pLlHandle, command, arg);
/*pr_err(" %s[%d]:  retvalue:%d\n", __func__, __LINE__, retvalue);*/
		gsw_api_unlock();
		return retvalue;
	}
fail:
	/*  release the given semaphore */
//	up(&swapi_sem);
   /* No supported command low-level function found.*/
	return -1;
}
/**The driver callbacks that will be registered with the kernel*/
/*static*/
const struct file_operations swapi_fops = {
	owner:THIS_MODULE,
	unlocked_ioctl : gsw_api_ioctl,
	open : gsw_api_open,
	release : gsw_api_release
};

static long gsw_api_ioctl(struct file *filp, u32 cmd, unsigned long arg)
{
	dev_minor_num_t *p;
	int ret;
	ioctl_wrapper_ctx_t *pdev;
	ioctl_cmd_handle_t *cmd_handle;

	if (!ioctlwrapctx) {
		pr_err("ioctlwrapctx not initilized\n");
		return -1;
	}
	p = filp->private_data;
	pdev = ioctlwrapctx;
	cmd_handle = gsw_api_alloc_cmd_handle();
	if(!cmd_handle) return -1;
	if (!p->minor_number) {
		if (pdev->bInternalSwitch == 1)
			cmd_handle->pLlHandle = pdev->pEthSWDev[0];
		else {
			pr_err("%s[%d]: Not support internal switch\n\n",
			__func__, __LINE__);
			kfree(cmd_handle);
			return -1;
		}
	} else {
		if (p->minor_number <= pdev->nExternalSwitchNum) {
			cmd_handle->pLlHandle = pdev->pEthSWDev[p->minor_number];
		} else {
			pr_err("(Not support external switch number: %d) %s:%s:%d\n",
				p->minor_number, __FILE__, __func__, __LINE__);
			kfree(cmd_handle);
			return -1;
		}
	}
	ret = gsw_command_search(cmd_handle, cmd,
		arg, ETHSW_API_USERAPP);
	kfree(cmd_handle);
	return ret;
}

static int gsw_api_open(struct inode *inode, struct file *filp)
{
	u32 minornum, majornum;
	dev_minor_num_t *p;

	minornum = MINOR(inode->i_rdev);
	majornum = MAJOR(inode->i_rdev);
	p = kmalloc(sizeof(dev_minor_num_t), GFP_KERNEL);
	if (!p) {
		pr_err("%s[%d]: memory allocation failed !!\n",
		__func__, __LINE__);
		return -ENOMEM;
	}
	p->minor_number = minornum;
	filp->private_data = p;
	return 0;
}

static int gsw_api_release(struct inode *inode,
	struct file *filp)
{
	if (filp->private_data) {
		kfree(filp->private_data);
		filp->private_data = NULL;
	}
	return 0;
}

int gsw_api_drv_register(u32 major)
{
	int result;
	result = register_chrdev(major, ETHSW_API_DEV_NAME, &swapi_fops);
	if (result < 0) {
		pr_err("SWAPI: Register Char Dev failed with %d !!!\n", result);
		return result;
	}
	pr_info("SWAPI: Registered char device [%s] with major no [%d]\n",
	ETHSW_API_DEV_NAME, major);
	return 0;
}

int gsw_api_drv_unregister(u32 major)
{
	unregister_chrdev(major, ETHSW_API_DEV_NAME);
	return 0;
}

void *ioctl_wrapper_init(ioctl_wrapper_init_t *pinit)
{
	u8 i;
	ioctl_wrapper_ctx_t *pdev;
	gsw_spi_lock_init();
	pdev = (ioctl_wrapper_ctx_t *)kmalloc(sizeof(ioctl_wrapper_ctx_t), GFP_KERNEL);
	if (!pdev) {
		pr_err("%s memory allocation failed !!\n", __func__);
		return pdev;
	}
	pdev->bInternalSwitch = 0;  /* internal switch, the value is 0 */
	pdev->nExternalSwitchNum = 0;
	pdev->pIoctlHandle = (ioctl_cmd_handle_t *)kmalloc(sizeof(ioctl_cmd_handle_t), GFP_KERNEL);
	if (!pdev->pIoctlHandle) {
		pr_err("%s memory allocation failed !!\n", __func__);
		if (pdev)
			kfree(pdev);
		return NULL; /*pdev->pIoctlHandle;*/
	}
	pdev->pIoctlHandle->pLlTable = pinit->pLlTable;
	pdev->pIoctlHandle->default_handler = pinit->default_handler;
	for (i = 0; i < LTQ_GSW_DEV_MAX; i++)
		pdev->pEthSWDev[i] = NULL;
	ioctlwrapctx = pdev;
	return pdev;
}

int ioctl_wrapper_dev_add(ioctl_wrapper_ctx_t *pioctldev,
	void *pcoredev, u8 mnum)
{
	if (mnum >= LTQ_GSW_DEV_MAX) {
		pr_err("(Device number: %d) %s:%s:%d\n", mnum,
			__FILE__, __func__, __LINE__);
		return -1;
	}
	pioctldev->pEthSWDev[mnum] = pcoredev;
	if (!mnum)
		pioctldev->bInternalSwitch = 1;
	else /* other than 0 means external switch */
		pioctldev->nExternalSwitchNum++;
	return 0;
}

int gsw_api_ioctl_wrapper_cleanup(void)
{
	ioctl_wrapper_ctx_t *pdev = ioctlwrapctx;
	if (pdev != NULL) {
		ioctlwrapctx = NULL;
		if (pdev->pIoctlHandle != NULL) {
			kfree(pdev->pIoctlHandle);
			pdev->pIoctlHandle = NULL;
		}
		kfree(pdev);
		pdev = NULL;
	}
	return 0;
}

GSW_API_HANDLE gsw_api_kopen(char *name)
{
	ioctl_wrapper_ctx_t *pdev;
	void *pLlHandle;
	/* process /dev/switch/minor string */
	char *needle = "/";
	char *buf = strstr(name, needle);

	if (!ioctlwrapctx) {
		pr_err("ioctlwrapctx not initilized\n");
		return 0;
	}
	pdev = ioctlwrapctx;
	name = buf+strlen(needle); /* pointer to dev */
	if (name != NULL) {
		buf = strstr(name, needle);
	}
	name = buf+strlen(needle); /* pointer to switch */
	if (name != NULL) {
		buf = strstr(name, needle);
	}
	name = buf+strlen(needle); /* pointer to minor */
	if (name == NULL)
		return 0;
	if (!strcmp(name, "0")) {
		if (pdev->bInternalSwitch == 1)
			pLlHandle = pdev->pEthSWDev[0];
		else {
			pr_err("\nNot support internal switch\n\n");
			return 0;
		}
	} else if (!strcmp(name, "1")) {
		pLlHandle = pdev->pEthSWDev[1];
	} else {
		pr_err("\nNot support external switch number = %s\n\n", name);
		return 0;
	}
	return (GSW_API_HANDLE)pLlHandle;
}
EXPORT_SYMBOL(gsw_api_kopen);

int gsw_api_kioctl(GSW_API_HANDLE handle, u32 command, u32 arg)
{
	ioctl_wrapper_ctx_t *pdev = NULL;
	ioctl_cmd_handle_t *cmd_handle;
	int ret;

	if (!ioctlwrapctx) {
		pr_err("ioctlwrapctx not initilized\n");
		return -1;
	}
	pdev = ioctlwrapctx;
	cmd_handle = gsw_api_alloc_cmd_handle();
	if(!cmd_handle) return -1;
	if (handle == (GSW_API_HANDLE)pdev->pEthSWDev[0]) {
			cmd_handle->pLlHandle = pdev->pEthSWDev[0];
	} else if (handle == (GSW_API_HANDLE)pdev->pEthSWDev[1]) {
		cmd_handle->pLlHandle = pdev->pEthSWDev[1];
	}	else {
		pr_err("ERROR:Provided wrong address ( Address:0x%08x) %s:%s:%d\n", 
			handle, __FILE__, __func__, __LINE__);
		kfree(cmd_handle);
		return -1;
	}
	ret = gsw_command_search(cmd_handle, command,
		arg, ETHSW_API_KERNEL);
	kfree(cmd_handle);
	return ret;
}
EXPORT_SYMBOL(gsw_api_kioctl);

int gsw_api_kclose(GSW_API_HANDLE handle)
{
	/* Nothing to do for kernel API's */
	return 0;
}
EXPORT_SYMBOL(gsw_api_kclose);
