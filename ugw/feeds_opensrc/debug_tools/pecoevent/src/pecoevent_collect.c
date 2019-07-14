/********************************************************************************
* Copyright (c) 2015                                                            *
* LANTIQ BETEILIGUNGS GMBH & CO KG                                              *
* Lilienthalstrasse 15, 85579 Neubiberg, Germany                                *
* For licensing information, see the file 'LICENSE' in the root folder of       *
* this software module.                                                         *
********************************************************************************/
 
/* ******************************************************************************
*  Description : Contains all common functions which is used                    *
*                Across the system.                                             *
********************************************************************************/
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/klogging.h>

/*asm header files*/
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

/*pecostat header file*/
#include "pecostat.h"
#include "fapi_processorstat.h"
//#include "fapi_sys_common.h"

/*Definition of variables*/

#define EXL     1<<0	/*count events in exception mode*/
#define K       1<<1	/*count events in kernel mode*/
#define S       1<<2	/*count events in supervisor mode*/
#define U       1<<3	/*count events in user mode*/
#define IE      1<<4	/*extended mode*/
#define ALL_EVENTS (EXL|K|S|U|IE)
#define EVENT_SHIFT 5
#define I$_EVENT 37	/*0i$stall */
#define INST_EVENT 1	/*1 */
#define D$_EVENT 37	/*4d$stall odd */
#define ITLB_EVENT 5	/*1ITLB misses */
#define DTLB_EVENT 6	/*1 DTLB misses */
#define CYCLE_EVENT 0	/*0 Cycles */

enum {
                HW_PLATFORM_NONE=0,
                HW_PLATFORM_GRX350,
                HW_PLATFORM_GRX500,
                HW_PLATFORM_GRX750,
                HW_PLATFORM_xRX330,
                HW_PLATFORM_xRX300,
                HW_PLATFORM_xRX220,
}platform_t;

unsigned int uiWriteLength;	/*Data length to be written to driver*/
unsigned int uiReadLength;	/*Dat length to be read from driver */
long long *puiReadptr;		/*Read ptr to read from driver*/
extern short Nvp;
extern short Npc;
extern unsigned char CCRes[4];	/*Multiplication factor for 4 VPE's*/
extern int Device_Open;
int ngFlag = 0;

PECOSTAT_INFO *xPecoInfo,*xPecoInfo1,*xPecoInfo2;	/*Info to fill for write*/

/*Exported functions from pecostat kernel module*/
ssize_t pecostat_device_write(struct file *,const char *,size_t ,loff_t *);
ssize_t pecostat_device_read(struct file *, char *, size_t, loff_t *);

static int pecostat_open(struct inode *i, struct file *f)
{
	if(Device_Open)
		return -EBUSY;

        /*Increment Device_Open to prevent others from opening driver*/
        Device_Open++;

	try_module_get(THIS_MODULE);
	return 0;
}

static int pecostat_close(struct inode *i, struct file *f)
{

	/* Decrement Device open to allow others to use pecostat driver*/
	Device_Open--;
	module_put(THIS_MODULE);
	return 0;
}

/* ==========================================================================
* Function Name : sock_ioctl
* Description   : gets differnet cpu stats counter for a given interval 
		  of 1 sec it can read 2 different counter values.
* Input         : None
* OutPut        : None
* Returns       : SUCCESS/FAILURE
=============================================================================*/
static long sock_ioctl(struct file *file, unsigned int cmd, unsigned long tmp)
{
	int nRet = 0, iLoop = 0;
	unsigned long long *uIEvent, *uIEvent1, *uIEvent2;
	void  __user *args = (void __user *)tmp;
	ProcessorEvent xProcessorEvent;

	memset(&xProcessorEvent, 0, sizeof(ProcessorEvent));

	if (copy_from_user(&xProcessorEvent, (ProcessorEvent *)tmp, sizeof(ProcessorEvent))) {
		LOGF_KLOG_CRITICAL("pecostat event copy from user failed.");
		return -EFAULT;
	}
	if(ngFlag == 0)
	{
		if((xProcessorEvent.nPlatform == HW_PLATFORM_GRX500) || (xProcessorEvent.nPlatform == HW_PLATFORM_GRX350))
		{
			/*peco stat info + events as number of tc's multiplied by 2 +1 number of 4bytes*/
			uiWriteLength=sizeof(PECOSTAT_INFO) + (Nvp *(2+1))*sizeof(long long);
			uiReadLength=(Nvp*(2+1))*sizeof(long long);
		}
		else
		{
			uiWriteLength=sizeof(PECOSTAT_INFO) + (Npc*sizeof(long long));
			uiReadLength=((Npc+1)*sizeof(unsigned long long));
		}

		/*malloc pecostat structure and fill info and events as number of tc's multiplied by 2 words*/
		xPecoInfo=kmalloc(uiWriteLength,GFP_KERNEL);
		xPecoInfo1=kmalloc(uiWriteLength,GFP_KERNEL);
		xPecoInfo2=kmalloc(uiWriteLength,GFP_KERNEL);

		if((xPecoInfo == NULL) || (xPecoInfo1 == NULL) ||(xPecoInfo2 == NULL))
		{
			LOGF_KLOG_CRITICAL("pecostat event malloc failed..\n");
			nRet = -ENOMEM;
			return nRet;
		}

		/*memset peco stat info structure*/
		memset(xPecoInfo,0,uiWriteLength);
		memset(xPecoInfo1,0,uiWriteLength);
		memset(xPecoInfo2,0,uiWriteLength);

		/*Fill some basic variables*/
		xPecoInfo->VPpc=1;
		xPecoInfo->events_count=1;
		xPecoInfo->nvp=Nvp;

		if((xProcessorEvent.nPlatform == HW_PLATFORM_GRX500) || (xProcessorEvent.nPlatform == HW_PLATFORM_GRX350))
		{
			/*No.of counters=number of tcs multiplied by 2*/
			xPecoInfo->perf_count=Nvp*(2);
		}
		else
		{
			/*No.of counters=number of tcs multiplied by 2*/
			xPecoInfo->perf_count=Npc;
		}

		/*PM_COUNT*/
		memcpy(xPecoInfo1,xPecoInfo,uiWriteLength);
		memcpy(xPecoInfo2,xPecoInfo,uiWriteLength);

		/*point events to end of pecostat info structure*/
		uIEvent=(unsigned long long *)((char *)(xPecoInfo) + sizeof(PECOSTAT_INFO));
		uIEvent1=(unsigned long long *)((char *)(xPecoInfo1) + sizeof(PECOSTAT_INFO));
		uIEvent2=(unsigned long long *)((char *)(xPecoInfo2) + sizeof(PECOSTAT_INFO));

		if((xProcessorEvent.nPlatform == HW_PLATFORM_GRX500) || (xProcessorEvent.nPlatform == HW_PLATFORM_GRX350))
		{
			for(iLoop=0; iLoop<Nvp*(2+1); iLoop+=2)
			{
				uIEvent[iLoop] = ALL_EVENTS|(INST_EVENT << EVENT_SHIFT);
				uIEvent[iLoop+1] = ALL_EVENTS|(ITLB_EVENT << EVENT_SHIFT);
			}
			for(iLoop=0; iLoop<Nvp*(2+1); iLoop+=2)
			{
				uIEvent1[iLoop]=ALL_EVENTS|(I$_EVENT << EVENT_SHIFT);
				uIEvent1[iLoop+1]=ALL_EVENTS|(D$_EVENT << EVENT_SHIFT);
			}
			for(iLoop=0; iLoop<Nvp*(2+1); iLoop+=2) {
				uIEvent2[iLoop]=ALL_EVENTS|(CYCLE_EVENT << EVENT_SHIFT);
				uIEvent2[iLoop+1]=ALL_EVENTS|(DTLB_EVENT << EVENT_SHIFT);
			}
		}
		else
		{
			for(iLoop=0; iLoop<Npc; iLoop+=2) 
			{
				uIEvent[iLoop] = ALL_EVENTS|(INST_EVENT << EVENT_SHIFT);
				uIEvent[iLoop+1] = ALL_EVENTS|(ITLB_EVENT << EVENT_SHIFT);
			}
			for(iLoop=0; iLoop<Npc;iLoop+=2)
			{
				uIEvent1[iLoop]=ALL_EVENTS|(I$_EVENT << EVENT_SHIFT);
				uIEvent1[iLoop+1]=ALL_EVENTS|(D$_EVENT << EVENT_SHIFT);
			}
			for(iLoop=0; iLoop<Npc; iLoop+=2) {
				uIEvent2[iLoop]=ALL_EVENTS|(CYCLE_EVENT << EVENT_SHIFT);
				uIEvent2[iLoop+1]=ALL_EVENTS|(DTLB_EVENT << EVENT_SHIFT);
			}
		}

		/*malloc read ptr*/
		puiReadptr=kmalloc(uiReadLength,GFP_KERNEL);
                if(puiReadptr == NULL)
                {
                        LOGF_KLOG_CRITICAL("malloc failed for puiReadptr! \n");
                        nRet = -1;
                        return nRet;
                }

		ngFlag = 1;
	}

	switch(cmd)
	{
		case PROCESSOR_EVENT:

			/* pecostat INST and ICACHEMISSSTALLCYCLES event values for all VPE's */
			if((xProcessorEvent.flag & INTRUCTIONS) || (xProcessorEvent.flag & ICACHEMISSSTALLCYCLES))
			{
				/*pecostat writing*/
				pecostat_device_write(NULL, (char *)xPecoInfo, uiWriteLength, 0);

				/*memset the read ptr*/
				memset(puiReadptr,0,uiReadLength);

				/*set current state to interruptible*/
				__set_current_state(TASK_INTERRUPTIBLE);

				/*schedule timeout for 1 sec */
				schedule_timeout(HZ*1);

				/*Read from the pecostat driver*/
				pecostat_device_read(NULL,(char *)puiReadptr,uiReadLength,0);

				for(iLoop = 0; iLoop < 2; iLoop++)
				{
					if(iLoop == 0)
					{
						xProcessorEvent.uPicEvnt1 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt2 = puiReadptr[iLoop];
					}
				}
				for(iLoop = 3; iLoop < 5; iLoop++)
				{
					if(iLoop == 3)
					{
						xProcessorEvent.uPicEvnt3 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt4 = puiReadptr[iLoop];
					}

				}
				for(iLoop = 6; iLoop < 8; iLoop++)
				{
					if(iLoop == 7)
					{
						xProcessorEvent.uPicEvnt6 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt5 = puiReadptr[iLoop];
					}
				}

			}

			/* pecostat ICACHE and ITLB event values for all VPE's */
			if((xProcessorEvent.flag & DCACHEMISSSTALLCYCLES) || (xProcessorEvent.flag & ITLBMISSCYCLES))
			{
				/*pecostat writing*/
				pecostat_device_write(NULL,(char *)xPecoInfo1,uiWriteLength,0);

				/*memset the read ptr*/
				memset(puiReadptr,0,uiReadLength);

				/*set current state to interruptible*/
				__set_current_state(TASK_INTERRUPTIBLE);

				/*schedule timeout for 1 sec */
				schedule_timeout(HZ*1);

				/*Read from the pecostat driver*/
				pecostat_device_read(NULL,(char *)puiReadptr,uiReadLength,0);

				for(iLoop = 0; iLoop < 2; iLoop++)
				{
					if(iLoop == 0)
					{
						xProcessorEvent.uPicEvnt7 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt8 = puiReadptr[iLoop];
					}
				}
				for(iLoop = 3; iLoop < 5; iLoop++)
				{
					if(iLoop == 3)
					{
						xProcessorEvent.uPicEvnt9 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt10 = puiReadptr[iLoop];
					}
				}
				for(iLoop = 6; iLoop < 8; iLoop++)
				{
					if(iLoop == 7)
					{
						xProcessorEvent.uPicEvnt12 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt11 = puiReadptr[iLoop];
					}
				}

			}

			/* pecostat DTLB event and cycle event values for all VPE's */
			if((xProcessorEvent.flag & DTLBMISSCYCLES) || (xProcessorEvent.flag & CPUCYCLES))
			{
				/*pecostat writing*/
				pecostat_device_write(NULL,(char *)xPecoInfo2,uiWriteLength,0);

				/*memset the read ptr*/
				memset(puiReadptr,0,uiReadLength);

				/*set current state to interruptible*/
				__set_current_state(TASK_INTERRUPTIBLE);

				/*schedule timeout for 1 sec */
				schedule_timeout(HZ*1);

				/*Read from the pecostat driver*/
				pecostat_device_read(NULL,(char *)puiReadptr,uiReadLength,0);

				for(iLoop = 0; iLoop <=2; iLoop++)
				{
					if(iLoop == 0)
					{
						//puiReadptr[iLoop]*=CCRes[0];
						xProcessorEvent.uPicEvnt14 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt13 = puiReadptr[iLoop];
					}
				}
				for(iLoop = 3; iLoop <=5; iLoop++)
				{
					if(iLoop == 3)
					{
						//puiReadptr[iLoop]*=CCRes[0];
						xProcessorEvent.uPicEvnt16 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt15 = puiReadptr[iLoop];
					}
				}
				for(iLoop = 6; iLoop <=8; iLoop++)
				{
					if(iLoop == 6)
					{
						//puiReadptr[iLoop]*=CCRes[0];
						xProcessorEvent.uPicEvnt17 = puiReadptr[iLoop];
					}
					else
					{
						xProcessorEvent.uPicEvnt18 = puiReadptr[iLoop];
					}
				}

			}

			if(copy_to_user(args, &xProcessorEvent, sizeof(xProcessorEvent)))
			{
				LOGF_KLOG_CRITICAL("pecostat event copy to user structure failed.");
				return -EFAULT;
			}
			break;
		default:
			nRet = -EINVAL;
			break;
	}
	return nRet;
}

static struct file_operations file_ops =
{
	.owner = THIS_MODULE,
	.open = pecostat_open,
	.release = pecostat_close,
	.unlocked_ioctl = sock_ioctl
};


/* ==========================================================================
* Function Name : kpeco_init
* Description   : registers the PROCESSOREVENT_DEVICE with chardev
* Input         : None
* OutPut        : None
* Returns       : SUCCESS/FAILURE
=============================================================================*/
int __init kpeco_init(void)
{
	int nRet = 0;

	/* register the pecostat device as miscdevice */
	nRet = register_chrdev(PROCESSORSTAT_EVENT_MAJOR, PROCESSOREVENT_DEVICE, &file_ops);
	if(nRet < 0) { 
		LOGF_KLOG_INFO("register char device:  %s failed! \n",PROCESSOREVENT_DEVICE);
		return nRet;
	}
	LOGF_KLOG_INFO("pecostat event installed.. with major device number %d \n", PROCESSORSTAT_EVENT_MAJOR);

	return nRet;	
}

/* ==========================================================================
* Function Name : kpeco_exit
* Description   : unregisters the PROCESSOREVENT_DEVICE with chardev
* Input         : None
* OutPut        : None
* Returns       : SUCCESS/FAILURE
=============================================================================*/
void __exit kpeco_exit(void)
{
	kfree(puiReadptr);
	kfree(xPecoInfo);
	kfree(xPecoInfo1);
	kfree(xPecoInfo2);

	/* Unregister the pecostat event devices from miscdevice(s) */
	unregister_chrdev(PROCESSORSTAT_EVENT_MAJOR, PROCESSOREVENT_DEVICE);

	LOGF_KLOG_INFO("pecostat_event module unloaded\n");
}

/* init and cleanup functions */
module_init(kpeco_init);
module_exit(kpeco_exit);

/* module information */
MODULE_DESCRIPTION("kernel thread for collecting pecostat values"); 
MODULE_LICENSE("Dual BSD/GPL");
