#include <linux/module.h>    
#include <linux/kernel.h>    // included for KERN_INFO
#include <linux/init.h>     
#include <linux/inet.h>

#include <net/ppa_api.h>
#include <xway/switch-api/lantiq_gsw_api.h>
#include <xway/switch-api/lantiq_gsw_flow.h>

#define RCU_IFMUX_CFG 0x16080120
#define RCU_RST_REQ 0x16000010


static int enable_hgu_trunking(void)
{
    int ret = 0;
#if defined(CONFIG_HGU_BONDING) && CONFIG_HGU_BONDING
    
    GSW_API_HANDLE gswipr=0, gswipl=0;

    GSW_portLinkCfg_t linkcfg;
    GSW_portCfg_t portcfg;
    GSW_register_t regcfg;
    GSW_PCE_rule_t pcecfg;

    //open the GSWIP-R device
    gswipr = gsw_api_kopen("/dev/switch_api/1");
    if (gswipr == 0) {
	printk(KERN_INFO "%s: Open SWAPI device FAILED !!\n", __func__ );
	goto cleanup_ret;	
    }

    //open the GSIP-L device
    gswipl = gsw_api_kopen("/dev/switch_api/0");
    if (gswipl == 0) {
	printk(KERN_INFO "%s: Open SWAPI device FAILED !!\n", __func__ );
	goto cleanup_ret;	
    }

    //printk(KERN_INFO"Enabling Ethernet bonding between port 15 and port %d\n",CONFIG_HGU_BOND_PORT); 
#if defined(CONFIG_LTQ_PPA_ETHWAN_RGMII) && CONFIG_LTQ_PPA_ETHWAN_RGMII
    //Step1: Change the port 15 to RGMII mode
    //mem -s 0x16080120 -w 0x00100800 -u #(associate RGMII6F with GSWIP-R)
    __raw_writel(0x00100800,phys_to_virt(RCU_IFMUX_CFG));    
//    set_current_state(TASK_INTERRUPTIBLE);
//    schedule_timeout(1*HZ);
#endif

    //Step2: Force link up to 1Gbps on port 15 and CONFIG_HGU_BOND_PORT(on GSWIP-L)
    //switch_cli dev=1 GSW_PORT_LINK_CFG_SET nPortId=15 bDuplexForce=1 eDuplex=0 bLinkForce=1 eLink=0 bSpeedForce=1 eSpeed=1000 eMII_Mode=3
    memset(&linkcfg,0,sizeof(linkcfg));

    linkcfg.nPortId = 15;

    if((ret=gsw_api_kioctl(gswipr, GSW_PORT_LINK_CFG_GET, (unsigned int)&linkcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PORT_LINK_CFG_GET returned failure on GSWIPR %d\n",ret);
	goto cleanup_ret;	
    }

    linkcfg.bDuplexForce = 1;
    linkcfg.eDuplex = GSW_DUPLEX_FULL;
    linkcfg.bLinkForce = 1;
    linkcfg.eLink = GSW_PORT_LINK_UP;
    linkcfg.bSpeedForce = 1;
    linkcfg.eSpeed = GSW_PORT_SPEED_1000;
    linkcfg.eMII_Mode= GSW_PORT_HW_RGMII; 
       
    if((ret=gsw_api_kioctl(gswipr, GSW_PORT_LINK_CFG_SET, (unsigned int)&linkcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PORT_LINK_CFG_SET returned failure on GSWIPR %d\n",ret);
	goto cleanup_ret;	
    }
    
    //switch_cli dev=0 GSW_PORT_LINK_CFG_SET nPortId=1 bDuplexForce=1 eDuplex=0 bLinkForce=1 eLink=0 bSpeedForce=1 eSpeed=1000 eMII_Mode=3
    memset(&linkcfg,0,sizeof(linkcfg));

    linkcfg.nPortId = CONFIG_HGU_BOND_PORT;
    
    if((ret=gsw_api_kioctl(gswipl, GSW_PORT_LINK_CFG_GET, (unsigned int)&linkcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PORT_LINK_CFG_GET returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }
    
    linkcfg.bDuplexForce = 1;
    linkcfg.eDuplex = GSW_DUPLEX_FULL;
    linkcfg.bLinkForce = 1;
    linkcfg.eLink = GSW_PORT_LINK_UP;
    linkcfg.bSpeedForce = 1;
    linkcfg.eSpeed = GSW_PORT_SPEED_1000;
    linkcfg.eMII_Mode= GSW_PORT_HW_RGMII; 
   
    if((ret=gsw_api_kioctl(gswipl, GSW_PORT_LINK_CFG_SET, (unsigned int)&linkcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PORT_LINK_CFG_SET returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }

    //Enable RX GSWIPR bond port
    //switch_cli GSW_PORT_CFG_SET dev=1 nPortId=1 eEnable=1
    memset(&portcfg,0,sizeof(portcfg)); 
    
    portcfg.nPortId = CONFIG_HGU_BOND_PORT;
    
    if((ret=gsw_api_kioctl(gswipr, GSW_PORT_CFG_GET, (unsigned int)&portcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PORT_CFG_GET returned failure on GSWIPR %d\n",ret);
	goto cleanup_ret;	
    }

    portcfg.eEnable = GSW_PORT_ENABLE_RXTX;
    
    if((ret=gsw_api_kioctl(gswipr, GSW_PORT_CFG_SET, (unsigned int)&portcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PORT_CFG_SET returned failure on GSWIPR %d\n",ret);
	goto cleanup_ret;	
    }

    //Step3: In GSWIP-L Remove port 1 from unknown unicast and unknown multicast portmap.
    //switch_cli dev=0 REGISTER_SET nRegAddr=0x454 nData=0x7D

    memset(&regcfg,0,sizeof(regcfg));

    regcfg.nRegAddr = 0x454; 

    if((ret=gsw_api_kioctl(gswipl, GSW_REGISTER_GET, (unsigned int)&regcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_REGISTER_GET returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }

    regcfg.nData &= ~( 1 << CONFIG_HGU_BOND_PORT);

    if((ret=gsw_api_kioctl(gswipl, GSW_REGISTER_SET, (unsigned int)&regcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_REGISTER_SET returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }

    //switch_cli dev=0 REGISTER_SET nRegAddr=0x455 nData=0x7D 

    memset(&regcfg,0,sizeof(regcfg));

    regcfg.nRegAddr = 0x455; 

    if((ret=gsw_api_kioctl(gswipl, GSW_REGISTER_GET, (unsigned int)&regcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_REGISTER_GET returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }

    regcfg.nData &= ~( 1 << CONFIG_HGU_BOND_PORT);

    if((ret=gsw_api_kioctl(gswipl, GSW_REGISTER_SET, (unsigned int)&regcfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_REGISTER_SET returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }    

    //Step4: GSWIP-L Forward all traffic from port 1 to port 0, learning is disabled
    //switch_cli dev=0 GSW_PCE_RULE_WRITE pattern.nIndex=10 pattern.bEnable=1 pattern.bPortIdEnable=1 pattern.nPortId=1 
    //action.ePortMapAction=4 action.nForwardPortMap=0x01 action.bPortBitMapMuxControl=0x0 action.eLearningAction=2

    memset(&pcecfg, 0, sizeof(pcecfg));

    pcecfg.pattern.nIndex = 10;
    pcecfg.pattern.bEnable = 1;
    pcecfg.pattern.bPortIdEnable = 1;   
    pcecfg.pattern.nPortId = CONFIG_HGU_BOND_PORT;
    pcecfg.action.ePortMapAction = GSW_PCE_ACTION_PORTMAP_ALTERNATIVE; 
    pcecfg.action.nForwardPortMap= 0x01; //CPU port
    pcecfg.action.bPortBitMapMuxControl = 0x0;
    pcecfg.action.eLearningAction = GSW_PCE_ACTION_LEARNING_FORCE_NOT;

    if((ret=gsw_api_kioctl(gswipl, GSW_PCE_RULE_WRITE, (unsigned int)&pcecfg)) < GSW_statusOk) {
	printk(KERN_INFO "GSW_PCE_RULE_WRITE returned failure on GSWIPL %d\n",ret);
	goto cleanup_ret;	
    }    

    printk(KERN_INFO"Ethernet bonding between port 15 and port %d enabled...!\n",CONFIG_HGU_BOND_PORT); 

cleanup_ret:
    gsw_api_kclose(gswipl);
    gsw_api_kclose(gswipr);
#endif
return ret;
}

static int __init eth_bond_init(void)
{
    return enable_hgu_trunking();
}

static void __exit eth_bond_cleanup(void)
{
    printk(KERN_INFO "Cleaning up module.\n");
}
            
module_init(eth_bond_init);
module_exit(eth_bond_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lantiq co. ltd");
MODULE_DESCRIPTION("Ethernet Bonding datapath setup Kernel Module");


