/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/version.h>
#include <lantiq.h>
#include <lantiq_soc.h>

#include "ltq_ethsw_init.h"

void __iomem		*gsw_regbase;
#define sw_w32(x, y)	ltq_w32((x), (gsw_regbase + (y)))
#define sw_r32(x)			ltq_r32(gsw_regbase + (x))

#define GSW_API_MAJOR_NUMBER	81
#define LTQ_INT_GSWITCH				0
#define LTQ_EXT_GSWITCH				1

/*extern ltq_lowlevel_fkts_t ifx_flow_fkt_tbl; */
extern ltq_lowlevel_fkts_t ifx_flow_fkt_tbl;

/** read the gswitch register */
//void gsw_r32(void *pDevCtx, short Offset, short Shift, short Size, u32 *value)
void gsw_r32(short Offset, short Shift, short Size, u32 *value)
{
	u32 regValue, mask;
	regValue = sw_r32(Offset * 4);
	mask = (1 << Size) - 1 ;
	regValue = (regValue >> Shift);
	*value = (regValue & mask);
}

/** read and update the GSWIP register */
//void gsw_w32(void *pDevCtx, short Offset, short Shift, short Size, u32 value)
void gsw_w32(short Offset, short Shift, short Size, u32 value)
{
	u32 regValue, mask;
	regValue = sw_r32(Offset * 4);
	mask = (1 << Size) - 1;
	mask = (mask << Shift);
	value = ((value << Shift) & mask);
	value = ((regValue & ~mask) | value) ;
	sw_w32(value, (Offset * 4));
}

int ltq_ethsw_api_register(struct platform_device *pdev)
{
	int result;
	struct resource *res;
	ethsw_core_init_t core_init;
	ethsw_api_dev_t *pEthDev;
	ioctl_wrapper_ctx_t *pIoctlWrapper;
	ioctl_wrapper_init_t Ioctl_Wrapper_Init;
	struct clk *clk;

	/* Find and map our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");
		return -ENOENT;
	}
	gsw_regbase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gsw_regbase)) 
		return PTR_ERR(gsw_regbase);

	/* Register Char Device */
	result = gsw_api_drv_register(GSW_API_MAJOR_NUMBER);
	if (result != 0) {
		pr_err("%s:%s:%d (Register Char Device Failed) \n",	\
		 __FILE__, __func__, __LINE__);
		return result;
	}
	/*Enable Switch Power  */
	clk = clk_get_sys("1e108000.eth", NULL);
	clk_enable(clk);

	/* Init FLOW Switch Core Layer */
	core_init.eDev = LTQ_FLOW_DEV_INT;
/*	core_init.pDev = pRALDev; */
	pEthDev = ethsw_api_core_init(&core_init);
	if (pEthDev == NULL) {
		pr_err("%s:%s:%d (Init Failed) \n",	\
		 __FILE__, __func__, __LINE__);
		return -1;
	}
	pEthDev->nCPU_Port = LTQ_SOC_CPU_PORT;
	pEthDev->regbase = gsw_regbase;
	Ioctl_Wrapper_Init.pLlTable = &ifx_flow_fkt_tbl;
	Ioctl_Wrapper_Init.default_handler = NULL;
	pIoctlWrapper = ioctl_wrapper_init(&Ioctl_Wrapper_Init);
	if (pIoctlWrapper == NULL) {
		pr_err("%s:%s:%d (WrapperInit Failed) \n",	\
		__FILE__, __func__, __LINE__);
		return -1;
	}
	/* add Internal switch */
	ioctl_wrapper_dev_add(pIoctlWrapper, pEthDev, LTQ_INT_GSWITCH);
	return 0;
}

int ltq_ethsw_api_unregister(void)
{
	/* Free the device data block */
	gsw_api_drv_unregister(GSW_API_MAJOR_NUMBER);
	gsw_CoreCleanUP();
	gsw_api_ioctl_wrapper_cleanup();
	return 0;
}

/*
 * ltq_ethsw_api_init   the init function, called when the module is loaded.
 * Returns zero if successfully loaded, nonzero otherwise.
 */
static int __init ltq_ethsw_api_init(struct platform_device *pdev)
{
	/* Print Version Number */
	pr_info("LTQ ETH SWITCH API, Version %s.\n", GSW_API_DRV_VERSION);
	ltq_ethsw_api_register(pdev);
	return 0;
}

/*
 * ltq_etshw_api_exit  the exit function, called when the module is removed.
 */
static void __exit ltq_etshw_api_exit(void)
{
	ltq_ethsw_api_unregister();
}

static int ltq_switch_api_probe(struct platform_device *pdev)
{
	ltq_ethsw_api_init(pdev);
	return 0;
}

static int ltq_switch_api_remove(struct platform_device *pdev)
{
	ltq_etshw_api_exit();
	return 0;
}

static const struct of_device_id ltq_switch_api_match[] = {
	{ .compatible = "lantiq,xway-gsw2x" },
	{},
};
MODULE_DEVICE_TABLE(of, ltq_switch_api_match);

static struct platform_driver ltq_switch_api = {
	.probe = ltq_switch_api_probe,
	.remove = ltq_switch_api_remove,
	.driver = {
		.name = "xway-gsw2xapi",
		.of_match_table = ltq_switch_api_match,
		.owner = THIS_MODULE,
	},
};
module_platform_driver(ltq_switch_api);

MODULE_AUTHOR("LANTIQ");
MODULE_DESCRIPTION("LTQ ETHSW API");
MODULE_LICENSE("GPL");
MODULE_VERSION(GSW_API_DRV_VERSION);
