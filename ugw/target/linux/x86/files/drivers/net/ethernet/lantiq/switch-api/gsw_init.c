/******************************************************************************

                         Copyright (c) 2012, 2014, 2015
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/export.h>
#include <linux/clk.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/version.h>

#ifndef CONFIG_X86_INTEL_CE2700
#include <lantiq.h>
#include <lantiq_soc.h>
#else
#include <linux/netip_subsystem.h>
#include <linux/avalanche/generic/modphy_mrpc_api.h>

extern int DWC_ETH_QOS_mdio_read_direct(int bus_number, bool c45, int phyaddr, int mmd, int phyreg, int *phydata);
extern int DWC_ETH_QOS_mdio_write_direct(int bus_number, bool c45, int phyaddr, int mmd, int phyreg, int phydata);

#define MDIO_ADDR_LANTIQ 31
#define C45_ENABLED 0
#define MMD_DISABLED 32
#define MDIO_BUS_NUMBER_0 0

/* netip-subsystem gpio value set register*/
#define DATA_OUT_SET_REG_OFFSET 0x14
/* netip-subsystem gpio value clear register*/
#define DATA_OUT_CLEAR_REG_OFFSET 0x18
/* netip-subsystem gpio directio set register*/
#define OE_SET_REG_OFFSET 0x20

#define NETSS_GPIO_17 (1 << 17)
#define NETSS_GPIO_19 (1 << 19)
#endif /*CONFIG_X86_INTEL_CE2700*/

#include "gsw_init.h"

void __iomem		*addr_gswl;
void __iomem		*addr_gswr;
void __iomem		*addr_gsw;

#define sw_w32(x, y)	ltq_w32((x), (addr_gswl + (y)))
#define sw_r32(x)			ltq_r32(addr_gswl + (x))

#define gsw1_w32(x, y)	ltq_w32((x), ((y)))
#define gsw1_r32(x)			ltq_r32((x))

#if 0
void gsw_reg_w32(uint32_t val, uint32_t reg_off)
{
	sw_w32(val, reg_off);
}
EXPORT_SYMBOL_GPL(gsw_reg_w32);
#endif

#define GSW_API_MAJOR_NUMBER	81
#define LTQ_INT_GSWITCH			0
#define LTQ_EXT_GSWITCH			1

extern ltq_lowlevel_fkts_t ltq_flow_fkt_tbl;
extern ltq_lowlevel_fkts_t ltq_rt_fkt_tbl;
ioctl_wrapper_init_t ioctlinit;
ioctl_wrapper_ctx_t *pioctlctl = NULL;
ethsw_api_dev_t *pEDev0 = NULL, *pEDev1 = NULL;

#ifdef CONFIG_X86_INTEL_CE2700
int GSW_SMDIO_DataRead(void *cdev, GSW_MDIO_data_t *pPar)
{
	u32 data;
	int ret;

	ret = DWC_ETH_QOS_mdio_read_direct(MDIO_BUS_NUMBER_0, C45_ENABLED, 
						MDIO_ADDR_LANTIQ, MMD_DISABLED, pPar->nAddressReg & 0x1F, &data);
	pPar->nData = data & 0xFFFF;
	return ret;
}

int GSW_SMDIO_DataWrite(void *cdev, GSW_MDIO_data_t *pPar)
{
	return DWC_ETH_QOS_mdio_write_direct(MDIO_BUS_NUMBER_0, C45_ENABLED, 
						MDIO_ADDR_LANTIQ, MMD_DISABLED, pPar->nAddressReg & 0x1F, pPar->nData & 0xFFFF);
}

/** read the gswitch register */
void gsw_r32(void *cdev, short offset, short shift, short size, u32 *value)
{
	u32 rvalue, mask;
	ethsw_api_dev_t *pethdev = (ethsw_api_dev_t *)cdev;

	GSW_MDIO_data_t mdio_data;
	mdio_data.nAddressDev = 0x1F;
	mdio_data.nAddressReg = 0x1F;
	if ((offset & 0xD000) == 0xD000)
		mdio_data.nData = (offset);
	else
		mdio_data.nData = (offset | 0xE000);
	GSW_SMDIO_DataWrite(cdev, &mdio_data);
	mdio_data.nAddressDev = 0x1F;
	mdio_data.nAddressReg = 0x00;
	mdio_data.nData = 0;
	GSW_SMDIO_DataRead(cdev, &mdio_data);
	rvalue = mdio_data.nData;

	mask = (1 << size) - 1;
	rvalue = (rvalue >> shift);
	*value = (rvalue & mask);
}

/** read and update the GSWIP register */
void gsw_w32(void *cdev, short offset, short shift, short size, u32 value)
{
	u32 rvalue, mask;
	ethsw_api_dev_t *pethdev = (ethsw_api_dev_t *)cdev;

	GSW_MDIO_data_t mdio_data;
	mdio_data.nAddressDev = 0x1F;
	mdio_data.nAddressReg = 0x1F;
	if ((offset & 0xD000) == 0xD000)
		mdio_data.nData = (offset);
	else
		mdio_data.nData = (offset | 0xE000);
	GSW_SMDIO_DataWrite(cdev, &mdio_data);
	if (size != 16) {
		mdio_data.nAddressDev = 0x1F;
		mdio_data.nAddressReg = 0x00;
		mdio_data.nData = 0;
		GSW_SMDIO_DataRead(cdev, &mdio_data);
		rvalue = mdio_data.nData;

		/* Prepare the mask	*/
		mask = (1 << size) - 1 ;
		mask = (mask << shift);
		/* Shift the value to the right place and mask the rest of the bit*/
		value = ( value << shift ) & mask;
		/*  Mask out the bit field from the read register and place in the new value */
		value = ( rvalue & ~mask ) | value ;

		mdio_data.nAddressDev = 0x1F;
		mdio_data.nAddressReg = 0x1F;
		if ((offset & 0xD000) == 0xD000)
			mdio_data.nData = (offset);
		else
			mdio_data.nData = (offset | 0xE000);
		GSW_SMDIO_DataWrite(cdev, &mdio_data);
	}
	mdio_data.nAddressDev = 0x1F;
	mdio_data.nAddressReg = 0x0;
	mdio_data.nData = value;
	GSW_SMDIO_DataWrite(cdev, &mdio_data);
}
#else
/** read the gswitch register */
void gsw_r32(void *cdev, short offset, short shift, short size, u32 *value)
{
	u32 rvalue, mask;
	ethsw_api_dev_t *pethdev = (ethsw_api_dev_t *)cdev;
	if (pethdev->gsw_base != 0) {
		rvalue = gsw1_r32(pethdev->gsw_base + (offset * 4));
		mask = (1 << size) - 1;
		rvalue = (rvalue >> shift);
		*value = (rvalue & mask);
	} else {
		pr_err("%s:%s:%d,(ERROR)\n", __FILE__, __func__, __LINE__);
	}
}

/** read and update the GSWIP register */
void gsw_w32(void *cdev, short offset, short shift, short size, u32 value)
{
	u32 rvalue, mask;
	ethsw_api_dev_t *pethdev = (ethsw_api_dev_t *)cdev;
	if (pethdev->gsw_base != 0) {
		rvalue = gsw1_r32(pethdev->gsw_base + (offset * 4));
		mask = (1 << size) - 1;
		mask = (mask << shift);
		value = ((value << shift) & mask);
		value = ((rvalue & ~mask) | value);
		gsw1_w32(value, (pethdev->gsw_base + (offset * 4)));
	} else {
		pr_err("%s:%s:%d,(ERROR)\n", __FILE__, __func__, __LINE__);
	}
}
#endif

#ifdef CONFIG_X86_INTEL_CE2700
void inline gsw_p7_netss_write(void *base, unsigned int off, unsigned int val)
{
	*((volatile unsigned long *)(base + off)) = cpu_to_be32(val);
}

void gsw_p7_netss_unmap(void *base)
{
	iounmap(base);
}

void *gsw_p7_netss_map(netss_dev_t netss_subdevice)
{
	int ret;
	netss_dev_info_t pbase;
	volatile void *vbase = NULL;

	if (!netss_driver_ready()) {
		pr_err("%s:%s:%d (Netss Driver Not Ready)\n",
			__FILE__, __func__, __LINE__);
		return NULL;
	}

	if (netss_device_get_info(netss_subdevice, &pbase)) {
		pr_err("%s:%s:%d (Netss Get Info Failed)\n",
			__FILE__, __func__, __LINE__);
		return NULL;
	}

	vbase = ioremap_nocache(pbase.base, pbase.size);
	if (!vbase) {
		pr_err("%s:%s:%d (Virt_base Is Null)\n",
			__FILE__, __func__, __LINE__);
		return NULL;
	}

	return vbase;
}

int gsw_p7_reset_modphy_lane(void)
{

#ifndef EXT_SWITCH_SGMII0
	ModphyController_e client = MODPHY_SGMII1_2_5G_CLIENT_ID;
#else
	ModphyController_e client = MODPHY_SGMII0_2_5G_CLIENT_ID;
#endif
	modphy_reset_client(client);

	return 0;
}

int gsw_p7_power_on(void)
{
	void *netss_gpio_base = NULL;

	netss_gpio_base = gsw_p7_netss_map(NETSS_DEV_GPIO);
	if (!netss_gpio_base) {
		pr_err("%s:%s:%d (Gpio Base Map Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}

	/* set power gpio to output and value 1*/
	gsw_p7_netss_write(netss_gpio_base, OE_SET_REG_OFFSET, NETSS_GPIO_17);
	gsw_p7_netss_write(netss_gpio_base, DATA_OUT_SET_REG_OFFSET,
				NETSS_GPIO_17);
	mdelay(200);
	/* set reset gpio to output*/
	gsw_p7_netss_write(netss_gpio_base, OE_SET_REG_OFFSET, NETSS_GPIO_19);

	/* set reset gpio value to 0*/
	gsw_p7_netss_write(netss_gpio_base, DATA_OUT_CLEAR_REG_OFFSET,
				NETSS_GPIO_19);
	mdelay(200);
	/* set reset gpio value to 1*/
	gsw_p7_netss_write(netss_gpio_base, DATA_OUT_SET_REG_OFFSET,
				NETSS_GPIO_19);
	mdelay(1000);

	gsw_p7_netss_unmap(netss_gpio_base);

	return 0;
}

int ltq_ethsw_api_register_p7(struct platform_device *pdev)
{
	int result;
	ethsw_core_init_t core_init;
	ethsw_api_dev_t *pethdev;
	ioctl_wrapper_ctx_t *iowrap;
	ioctl_wrapper_init_t iowrapinit;

	/* Switch memory not mapped*/
	addr_gsw = 0;

	/* Enable Switch Power  */
	if (gsw_p7_power_on()) {
		pr_err("%s:%s:%d (Switch Power On Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}

	/* Init FLOW Switch Core Layer */
	core_init.sdev = LTQ_FLOW_DEV_INT;
	core_init.gsw_base_addr = addr_gsw;
/*	core_init.pDev = pRALDev; */
	pethdev = ethsw_api_core_init(&core_init);
	if (pethdev == NULL) {
		pr_err("%s:%s:%d (Init Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}
	pethdev->cport = GSW_2X_SOC_CPU_PORT;
	pethdev->gsw_base = addr_gsw;
	iowrapinit.pLlTable = &ltq_flow_fkt_tbl;
	iowrapinit.default_handler = NULL;
	iowrap = ioctl_wrapper_init(&iowrapinit);
	if (iowrap == NULL) {
		pr_err("%s:%s:%d (WrapperInit Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}

	/* add Internal switch */
	ioctl_wrapper_dev_add(iowrap, pethdev, LTQ_INT_GSWITCH);

	if (gsw_p7_reset_modphy_lane()) {
		pr_err("%s:%s:%d (Reset Modphy Lane Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}

	/* Register Char Device */
	result = gsw_api_drv_register(GSW_API_MAJOR_NUMBER);
	if (result != 0) {
		pr_err("%s:%s:%d (Register Char Device Failed)\n",
			__FILE__, __func__, __LINE__);
		return result;
	}

	return 0;
}
#endif /*CONFIG_X86_INTEL_CE2700*/

#ifdef CONFIG_SOC_GRX500
int ltq_gsw_api_register(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int result;
	u32 devid;
	struct resource *memres;
	ethsw_core_init_t core_init;
	struct clk *clk;

	/* Find and map our resources */
/* Switch device index */
	if (!of_property_read_u32(node, "lantiq,gsw-devid", &devid))
		pdev->id = devid;
	if (pdev->id < 0 || pdev->id >= 2)
		return -EINVAL;
	memres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (memres == NULL) {
		pr_err("%s:%s:%d (Failed)\n", __FILE__, __func__, __LINE__);
/*		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");*/
		return -ENOENT;
	}
	/*Enable Switch Power  */
	clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk))
		panic("Failed to get switch clock");
	clk_enable(clk);

	if (devid == 0) {
		addr_gswl = devm_ioremap_resource(&pdev->dev, memres);
		if (IS_ERR(addr_gswl))
			return PTR_ERR(addr_gswl);
/*		pr_err("%s:%s:%d (Register l base:0x%08x)\n",
			__FILE__, __func__, __LINE__, (u32)addr_gswl);*/
	}

	if (devid == 1) {
		addr_gswr = devm_ioremap_resource(&pdev->dev, memres);
		if (IS_ERR(addr_gswr))
			return PTR_ERR(addr_gswr);
/*		pr_err("%s:%s:%d (Register r base:0x%08x)\n",
			__FILE__, __func__, __LINE__, (u32)addr_gswr);*/
	}

	/* Register Char Device */
	if (devid == 0) {
		result = gsw_api_drv_register(GSW_API_MAJOR_NUMBER);
		if (result != 0) {
			pr_err("%s:%s:%d (Reg Char Device Failed)\n",
				__FILE__, __func__, __LINE__);
			return result;
		}
	}
	/*Enable Switch Power  */
#if 0
	clk = clk_get_sys("1c000000.eth", NULL);  /*GSWIP-L*/
	clk_enable(clk);
	clk = clk_get_sys("1a000000.eth", NULL);  /*GSWIP-R*/
	clk_enable(clk);
#endif
	if (devid == 0) {
		/* Init FLOW Switch Core Layer */
		core_init.sdev = LTQ_FLOW_DEV_INT;
		core_init.gsw_base_addr = addr_gswl;
		pEDev0 = ethsw_api_core_init(&core_init);
		if (pEDev0 == NULL) {
			pr_err("%s:%s:%d (Init Failed)\n",
				__FILE__, __func__, __LINE__);
			return -1;
		}
		pEDev0->cport = GSW_3X_SOC_CPU_PORT;
		pEDev0->gsw_dev = LTQ_FLOW_DEV_INT;
		pEDev0->gswl_base = addr_gswl;
		pEDev0->gsw_base = addr_gswl;
	}
	if (devid == 1) {
		/* Init FLOW Switch Core Layer */
		core_init.sdev = LTQ_FLOW_DEV_INT_R;
		core_init.gsw_base_addr = addr_gswr;
		pEDev1 = ethsw_api_core_init(&core_init);
		if (pEDev1 == NULL) {
			pr_err("%s:%s:%d (Init Failed)\n",
				__FILE__, __func__, __LINE__);
			return -1;
		}
		pEDev1->cport = GSW_3X_SOC_CPU_PORT;
		pEDev1->gsw_dev = LTQ_FLOW_DEV_INT_R;
		pEDev1->gswr_base = addr_gswr;
		pEDev1->gsw_base = addr_gswr;
	}
	if (devid == 0) {
		ioctlinit.pLlTable = &ltq_rt_fkt_tbl;
		ioctlinit.default_handler = NULL;
		pioctlctl = ioctl_wrapper_init(&ioctlinit);
		if (pioctlctl == NULL) {
			pr_err("%s:%s:%d (WrapperInit Failed)\n",
				__FILE__, __func__, __LINE__);
			return -1;
		}
	}
	/* add Internal switch */
	if ((devid == 0) && pioctlctl && pEDev0)
		ioctl_wrapper_dev_add(pioctlctl, pEDev0, LTQ_INT_GSWITCH);
	/* add Internal switch */
	if ((devid == 1) && pioctlctl && pEDev1)
		ioctl_wrapper_dev_add(pioctlctl, pEDev1, LTQ_EXT_GSWITCH);
	return 0;
}
#endif /* CONFIG_SOC_GRX500 */

int ltq_ethsw_api_register(struct platform_device *pdev)
{
	int result;
	struct resource *res;
	ethsw_core_init_t core_init;
	ethsw_api_dev_t *pethdev;
	ioctl_wrapper_ctx_t *iowrap;
	ioctl_wrapper_init_t iowrapinit;
	struct clk *clk;

	/* Find and map our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		pr_err("%s:%s:%d (Get IORESOURCE_MEM Failed)\n",
			__FILE__, __func__, __LINE__);
/*		dev_err(&pdev->dev, "Cannot get IORESOURCE_MEM\n");*/
		return -ENOENT;
	}
	addr_gsw = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(addr_gsw))
		return PTR_ERR(addr_gsw);

	/* Register Char Device */
	result = gsw_api_drv_register(GSW_API_MAJOR_NUMBER);
	if (result != 0) {
		pr_err("%s:%s:%d (Register Char Device Failed)\n",
			__FILE__, __func__, __LINE__);
		return result;
	}

	/*Enable Switch Power  */
	clk = clk_get_sys("1e108000.eth", NULL);
	clk_enable(clk);

	/* Init FLOW Switch Core Layer */
	core_init.sdev = LTQ_FLOW_DEV_INT;
	core_init.gsw_base_addr = addr_gsw;
/*	core_init.pDev = pRALDev; */
	pethdev = ethsw_api_core_init(&core_init);
	if (pethdev == NULL) {
		pr_err("%s:%s:%d (Init Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}
	pethdev->cport = GSW_2X_SOC_CPU_PORT;
	pethdev->gsw_base = addr_gsw;
	iowrapinit.pLlTable = &ltq_flow_fkt_tbl;
	iowrapinit.default_handler = NULL;
	iowrap = ioctl_wrapper_init(&iowrapinit);
	if (iowrap == NULL) {
		pr_err("%s:%s:%d (WrapperInit Failed)\n",
			__FILE__, __func__, __LINE__);
		return -1;
	}
	/* add Internal switch */
	ioctl_wrapper_dev_add(iowrap, pethdev, LTQ_INT_GSWITCH);
	return 0;
}

int ltq_ethsw_api_unregister(void)
{
	/* Free the device data block */
	gsw_api_drv_unregister(GSW_API_MAJOR_NUMBER);
	gsw_corecleanup();
	gsw_api_ioctl_wrapper_cleanup();
	return 0;
}

/*ltq_ethsw_api_init   the init function, called when the module is loaded.*/
 /*	Returns zero if successfully loaded, nonzero otherwise.*/
static int __init ltq_ethsw_api_init(struct platform_device *pdev)
{
	/* Print Version Number */
	pr_info("LTQ ETH SWITCH API, Version %s.\n", GSW_API_DRV_VERSION);
#ifdef CONFIG_SOC_GRX500
/*	if (of_machine_is_compatible("lantiq,grx500")) { */
		ltq_gsw_api_register(pdev);
#endif
/*	} else { */
#ifdef CONFIG_SOC_XWAY
		ltq_ethsw_api_register(pdev);
#endif

#ifdef CONFIG_X86_INTEL_CE2700
		ltq_ethsw_api_register_p7(pdev);
#endif
/*	} */
	return 0;
}

/*ltq_etshw_api_exit  the exit function, called when the module is removed.*/
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
#ifdef CONFIG_SOC_GRX500
static void __iomem		*gswl_addr;
static void __iomem		*gswr_addr;
/** read and update the GSWIP register */
static void ltq_gsw_w32(short offset, short shift, short size, u32 value)
{
	u32 rvalue, mask;
	if (gswl_addr != 0) {
		rvalue = gsw1_r32(gswl_addr + (offset * 4));
		mask = (1 << size) - 1;
		mask = (mask << shift);
		value = ((value << shift) & mask);
		value = ((rvalue & ~mask) | value);
/*		pr_info("writing %x to the address = %x \n", value, (u32) (gswl_addr + (offset * 4)));*/
		gsw1_w32(value, (gswl_addr + (offset * 4)));
	} else {
		pr_err("%s:%s:%d,(ERROR)\n", __FILE__, __func__, __LINE__);
	}
}

static void ltq_gsw_r_w32(short offset, short shift, short size, u32 value)
{
	u32 rvalue, mask;
	if (gswr_addr != 0) {
		rvalue = gsw1_r32(gswr_addr + (offset * 4));
		mask = (1 << size) - 1;
		mask = (mask << shift);
		value = ((value << shift) & mask);
		value = ((rvalue & ~mask) | value);
/*		pr_info("writing %x to the address = %x \n", value, (u32) (gswr_addr + (offset * 4)));*/
		gsw1_w32(value, (gswr_addr + (offset * 4)));
	} else {
		pr_err("%s:%s:%d,(ERROR)\n", __FILE__, __func__, __LINE__);
	}
}
void gsw_api_disable_switch_ports(void)
{
	int pidx;
	gswl_addr = (void __iomem *) (KSEG1 | 0x1c000000);
	gswr_addr = (void __iomem *) (KSEG1 | 0x1a000000);

	for(pidx = 2; pidx < 6; pidx++) {
		/* Set SDMA_PCTRL_PEN PORT disable */
		ltq_gsw_w32((SDMA_PCTRL_PEN_OFFSET + (6 * pidx)),
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 0);
		/* Set FDMA_PCTRL_EN PORT disable */
		ltq_gsw_w32((FDMA_PCTRL_EN_OFFSET + (0x6 * pidx)),
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 0);
	}
	for(pidx = 0; pidx < 16; pidx++) {
		/* Set SDMA_PCTRL_PEN PORT disable */
		ltq_gsw_r_w32((SDMA_PCTRL_PEN_OFFSET + (6 * pidx)),
			SDMA_PCTRL_PEN_SHIFT, SDMA_PCTRL_PEN_SIZE, 0);
		/* Set FDMA_PCTRL_EN PORT disable */
		ltq_gsw_r_w32((FDMA_PCTRL_EN_OFFSET + (0x6 * pidx)),
			FDMA_PCTRL_EN_SHIFT, FDMA_PCTRL_EN_SIZE, 0);
	}
}
EXPORT_SYMBOL(gsw_api_disable_switch_ports);
#endif

#ifdef CONFIG_SOC_GRX500
static const struct of_device_id ltq_switch_api_match[] = {
	{ .compatible = "lantiq,xway-gswapi" },
	{},
};
#endif /* CONFIG_SOC_GRX500  */
#ifdef CONFIG_SOC_XWAY
static const struct of_device_id ltq_switch_api_match[] = {
	{ .compatible = "lantiq,xway-gsw2x" },
	{},
};
#endif /* CONFIG_SOC_XWAY */
#ifdef CONFIG_X86_INTEL_CE2700
static const struct of_device_id ltq_switch_api_match[] = {
	{ .compatible = "lantiq,xway-gsw2x" },
	{},
};
#endif /* CONFIG_X86_INTEL_CE2700 */
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


#ifdef CONFIG_X86_INTEL_CE2700
static struct platform_device *ltq_switch_api_dev;

static int __init ltq_ethsw_api_init_p7(void)
{
	int ret;

	ret = platform_driver_register(&ltq_switch_api);
	if (ret < 0) {
		pr_err("%s:%s:%d switch_api driver register failed\n",
			__FILE__, __func__, __LINE__);
		return ret;
	}

	ltq_switch_api_dev =
		platform_device_register_simple("xway-gsw2xapi", -1, NULL, 0);

	if (IS_ERR(ltq_switch_api_dev)) {
		pr_err("%s:%s:%d switch_api device register failed\n",
			__FILE__, __func__, __LINE__);
		platform_driver_unregister(&ltq_switch_api);
		return PTR_ERR(ltq_switch_api_dev);
	}
	return 0;
}

static void __exit ltq_ethsw_api_exit_p7(void)
{
	platform_device_unregister(ltq_switch_api_dev);
	platform_driver_unregister(&ltq_switch_api);
}

module_init(ltq_ethsw_api_init_p7);
module_exit(ltq_ethsw_api_exit_p7);
#else
module_platform_driver(ltq_switch_api);
#endif /*CONFIG_X86_INTEL_CE2700*/

MODULE_AUTHOR("LANTIQ");
MODULE_DESCRIPTION("LTQ ETHSW API");
MODULE_LICENSE("GPL");
MODULE_VERSION(GSW_API_DRV_VERSION);
