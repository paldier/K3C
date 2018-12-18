#
# Copyright (C) 2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

LANTIQ_MENU:=Lantiq
INTEL_MENU:=Intel
SUBTARGET_SUFFIX:=$(shell echo $(subst .,_,$(subst -,_,$(subst /,_,$(SUBTARGET)))) | cut -d_ -f2-)
CPE_NAME:=$(shell echo $(SUBTARGET) | awk '{ print toupper($$1) }')

define KernelPackage/lantiq-deu
  TITLE:=Lantiq data encryption unit
  SUBMENU:=$(CRYPTO_MENU)
  DEPENDS:=@TARGET_lantiq
  KCONFIG:=CONFIG_CRYPTO_DEV_LANTIQ \
	   CONFIG_CRYPTO_HW=y \
	   CONFIG_CRYPTO_DEV_LANTIQ_AES=y \
	   CONFIG_CRYPTO_DEV_LANTIQ_DES=y \
	   CONFIG_CRYPTO_DEV_LANTIQ_MD5=y \
	   CONFIG_CRYPTO_DEV_LANTIQ_SHA1=y
  $(call AddDepends/crypto)
endef

define KernelPackage/lantiq-deu/description
  Kernel support for the Lantiq crypto HW
endef

$(eval $(call KernelPackage,lantiq-deu))

define KernelPackage/lantiq-pcie
  TITLE:=Lantiq PCIe Bus Support
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS:=@TARGET_lantiq
  KCONFIG:=\
	CONFIG_PCI=y \
	CONFIG_PCI_MSI=y \
	CONFIG_PCI_DOMAINS=y \
	CONFIG_PCIEPORTBUS=y \
	CONFIG_PCIE_LANTIQ=y \
	CONFIG_LANTIQ_PCIE_1ST_CORE=n \
	CONFIG_LANTIQ_PCIE_HW_SWAP=n
endef

define KernelPackage/lantiq-pcie-25mhz
  $(call KernelPackage/lantiq-pcie)
  TITLE += 25MHZ
  DEPENDS += @!PACKAGE_kmod-lantiq-pcie-36mhz-ssc
  KCONFIG += \
	CONFIG_LANTIQ_PCIE_PHY_25MHZ_MODE=y
endef

define KernelPackage/lantiq-pcie-25mhz/description
  Kernel support for the Lantiq PCIe bus 25mhz mode
endef

$(eval $(call KernelPackage,lantiq-pcie-25mhz))

define KernelPackage/lantiq-pcie-36mhz-ssc
  $(call KernelPackage/lantiq-pcie)
  TITLE += 36MHZ SSC
  KCONFIG += \
	CONFIG_LANTIQ_PCIE_PHY_36MHZ_SSC_MODE=y
endef

define KernelPackage/lantiq-pcie-36mhz-ssc/description
  Kernel support for the Lantiq PCIe bus 36MHZ SSC mode
endef

$(eval $(call KernelPackage,lantiq-pcie-36mhz-ssc))

define KernelPackage/lantiq-pcie-vrxmei
  TITLE:=PCIe Support for MEI VRX or VRX318
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS += @PACKAGE_kmod-lantiq-pcie-25mhz||PACKAGE_kmod-lantiq-pcie-36mhz-ssc
  KCONFIG:=\
	CONFIG_LANTIQ_VRX318=y
endef

define KernelPackage/lantiq-pcie-vrxmei/description
  Kernel support for the Lantiq PCIe bus
endef

$(eval $(call KernelPackage,lantiq-pcie-vrxmei))

define pcie_core
define KernelPackage/lantiq-pcie-$(if $(1),$(1)-)$(2)
  TITLE:=Lantiq PCIe $(if $(1),$(1)-)$(2)
  SUBMENU:=$$(LANTIQ_MENU)
  DEPENDS:=@PACKAGE_kmod-lantiq-pcie-25mhz||PACKAGE_kmod-lantiq-pcie-36mhz-ssc $(if $(1),kmod-lantiq-pcie-$(1))
  KCONFIG:=\
	CONFIG_LANTIQ_$(3)=y
endef

define KernelPackage/lantiq-pcie-$(if $(1),$(1)-)$(2)/description
  Lantiq PCIe bus enable $(if $(1),$(1)-)$(2)
endef

$$(eval $$(call KernelPackage,lantiq-pcie-$(if $(1),$(1)-)$(2)))
endef

$(eval $(call pcie_core,,1st-core,PCIE_1ST_CORE))
$(eval $(call pcie_core,1st-core,hw-swap,PCIE_HW_SWAP))
$(eval $(call pcie_core,1st-core,inbound-no-hw-swap,PCIE_INBOUND_NO_HW_SWAP))
$(eval $(call pcie_core,1st-core,rst-ep-active-high,PCIE_RST_EP_ACTIVE_HIGH))

$(eval $(call pcie_core,,2nd-core,PCIE_2ND_CORE))
$(eval $(call pcie_core,2nd-core,hw-swap,PCIE1_HW_SWAP))
$(eval $(call pcie_core,2nd-core,inbound-no-hw-swap,PCIE1_INBOUND_NO_HW_SWAP))
$(eval $(call pcie_core,2nd-core,rst-ep-active-high,PCIE1_RST_EP_ACTIVE_HIGH))

$(eval $(call pcie_core,,3rd-core,PCIE_3RD_CORE))
$(eval $(call pcie_core,3rd-core,hw-swap,PCIE2_HW_SWAP))
$(eval $(call pcie_core,3rd-core,inbound-no-hw-swap,PCIE2_INBOUND_NO_HW_SWAP))
$(eval $(call pcie_core,3rd-core,rst-ep-active-high,PCIE2_RST_EP_ACTIVE_HIGH))


USB_MENU:=USB Support

define KernelPackage/usb-xhci
  TITLE:=USB 3.0 XHCI support
  SUBMENU:=$(USB_MENU)
  DEPENDS+=+kmod-usb-storage
  KCONFIG:= \
	CONFIG_USB_XHCI_HCD=m \
	CONFIG_USB_SUPPORT=y \
	CONFIG_USB_ARCH_HAS_HCD=y \
	CONFIG_USB=y \
	CONFIG_USB_PHY=y \
	CONFIG_LTQ_DWC3_PHY=y
  FILES:= \
		$(LINUX_DIR)/drivers/usb/host/xhci-hcd.$(LINUX_KMOD_SUFFIX) \
		$(LINUX_DIR)/drivers/usb/dwc3/ltq-usb-oc.ko
  AUTOLOAD:=$(call AutoLoad,50,xhci-hcd)
endef

define KernelPackage/usb-xhci/description
  Kernel support USB 3.0 XHCI support XWAY
endef

$(eval $(call KernelPackage,usb-xhci))

define KernelPackage/usb-ltqhcd
  TITLE:=LTQHCD usb driver
  SUBMENU:=$(USB_MENU)
  DEPENDS+=+kmod-usb-storage
  KCONFIG:=CONFIG_USB_HOST_LTQ \
	CONFIG_USB_HOST_LTQ_FORCE_USB11=n \
	CONFIG_USB_HOST_LTQ_WITH_HS_ELECT_TST=n \
	CONFIG_USB_HOST_LTQ_WITH_ISO=n \
	CONFIG_USB_HOST_LTQ_UNALIGNED_ADJ=y
  FILES:=$(LINUX_DIR)/drivers/usb/host/ltqusb_host.ko
  AUTOLOAD:=$(call AutoLoad,50,ltqusb_host)
endef

define KernelPackage/usb-ltqhcd/description
  Kernel support for Synopsis USB on XWAY
endef

$(eval $(call KernelPackage,usb-ltqhcd))

I2C_LANTIQ_MODULES:= \
  CONFIG_I2C_LANTIQ:drivers/i2c/busses/i2c-lantiq

define KernelPackage/i2c-lantiq
  TITLE:=Lantiq I2C controller
  $(call i2c_defaults,$(I2C_LANTIQ_MODULES),52)
  DEPENDS:=kmod-i2c-core
endef

define KernelPackage/i2c-lantiq/description
  Kernel support for the Lantiq I2C controller
endef

$(eval $(call KernelPackage,i2c-lantiq))


I2C_LANTIQ_MODULES:= \
  CONFIG_I2C_LANTIQ:drivers/i2c/busses/i2c-lantiq

define KernelPackage/i2c-lantiq-build-in
  TITLE:=Lantiq I2C controller
  SUBMENU:=I2C support
  KCONFIG:= \
	  CONFIG_I2C=y \
	  CONFIG_I2C_LANTIQ=y
endef

define KernelPackage/i2c-lantiq/description
  Kernel support for the Lantiq I2C controller. Build-In driver
endef

$(eval $(call KernelPackage,i2c-lantiq-build-in))


define KernelPackage/lantiq-vpe
  TITLE:=Lantiq VPE extensions
  SUBMENU:=Lantiq
  DEPENDS:=@TARGET_lantiq +kmod-vpe
  KCONFIG:= \
	  CONFIG_LTQ_VPE_EXT=y \
	  CONFIG_SOFT_WATCHDOG_VPE=y \
	  CONFIG_MTSCHED=y \
	  CONFIG_PERFCTRS=n
endef

define KernelPackage/lantiq-vpe/description
  Kernel extensions for the Lantiq SoC
endef

$(eval $(call KernelPackage,lantiq-vpe))

define KernelPackage/smvp
  TITLE:=SMVP configuration
  SUBMENU:=Lantiq
  DEPENDS:=@TARGET_lantiq @!(PACKAGE_kmod-vpe)
  KCONFIG:= \
        CONFIG_MTSCHED=y \
        CONFIG_MIPS_MT_SMP=y \
        CONFIG_SCHED_SMT=y \
        CONFIG_SYS_SUPPORTS_SCHED_SMT=y \
        CONFIG_LTQ_MIPS_MT_SMP_IRQAFF=y
endef

define KernelPackage/smvp/description
  Enable SMVP related kernel configuration
endef

$(eval $(call KernelPackage,smvp))

define KernelPackage/grx500-emu
  TITLE:=Enable GRX500 SOC based Emulator Config (HAPS)
  SUBMENU:=Lantiq
  KCONFIG:= \
	CONFIG_USE_EMULATOR=y \
	CONFIG_USE_HAPS=y
endef

define KernelPackage/grx500-emu/description
  Enable GRX500 SOC based Emulator Config (HAPS)  
endef

$(eval $(call KernelPackage,grx500-emu))

define KernelPackage/lantiq-nf
  TITLE:=Lantiq NF extensions
  SUBMENU:=Lantiq
  DEPENDS:=@TARGET_lantiq
  KCONFIG:=CONFIG_NF_CONNTRACK_EVENTS=y
endef

define KernelPackage/lantiq-nf/description
  Netfilter extensions for the Lantiq SoC
endef

$(eval $(call KernelPackage,lantiq-nf))

define KernelPackage/lantiq_sata_ahci
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Lantiq SATA AHCI support
  DEPENDS:=@TARGET_lantiq
  KCONFIG:= \
    CONFIG_ATA=y \
    CONFIG_SATA_AHCI=m \
    CONFIG_PCI=y
 
  FILES:=\
	$(LINUX_DIR)/drivers/ata/ahci.$(LINUX_KMOD_SUFFIX) \
	$(LINUX_DIR)/drivers/ata/libahci.$(LINUX_KMOD_SUFFIX)
endef
 
define KernelPackage/lantiq_sata_ahci/description
Kernel module for Lantiq SATA AHCI package
endef
 
$(eval $(call KernelPackage,lantiq_sata_ahci))

define KernelPackage/fs-ext3
  SUBMENU:=Filesystems
  TITLE:=EXT3 filesystem support
  KCONFIG:= \
        CONFIG_EXT3_FS \
        CONFIG_JBD
  FILES:= \
        $(LINUX_DIR)/fs/ext3/ext3.ko \
        $(LINUX_DIR)/fs/jbd/jbd.ko \
        $(LINUX_DIR)/fs/mbcache.ko
endef

define KernelPackage/fs-ext3/description
 Kernel module for EXT3 filesystem support
endef

$(eval $(call KernelPackage,fs-ext3))

define KernelPackage/ubi_mtd
 SUBMENU:=Lantiq
 TITLE:=UBI Support
 DEPENDS:=@TARGET_lantiq
 KCONFIG:= \
	CONFIG_MTD_UBI=y \
	CONFIG_MTD_UBI_BEB_RESERVE=1 \
	CONFIG_MTD_UBI_GLUEBI=y \
	CONFIG_MTD_UBI_GLUEBI_ROOTFS_DEV=y \
	CONFIG_MTD_UBI_DEBUG=n
endef
# CONFIG_MTD_UBI_WL_THRESHOLD is moved to platformX/configs

define KernelPackage/ubi_mtd/description
  Kernel Built-in support for ubi and gluebi mtd
endef

$(eval $(call KernelPackage,ubi_mtd))

define KernelPackage/ubifs
  SUBMENU:=Lantiq
  TITLE:=UBI Filesystem Support
  DEPENDS:=@TARGET_lantiq +kmod-ubi_mtd
  KCONFIG:= \
	CONFIG_UBIFS_FS=y \
	CONFIG_UBIFS_FS_XATTR=n \
	CONFIG_UBIFS_FS_ADVANCED_COMPR=y \
	CONFIG_UBIFS_FS_LZO=y \
	CONFIG_UBIFS_FS_ZLIB=n \
	CONFIG_UBIFS_FS_DEBUG=n
endef

define KernelPackage/ubifs/description
  Kernel Built-in support for ubi filesystem
endef

$(eval $(call KernelPackage,ubifs))

define KernelPackage/atm_stack
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=ATM stack
  DEPENDS:=@TARGET_lantiq
  KCONFIG:= \
	CONFIG_ATM=y \
	CONFIG_ATM_CLIP=y \
	CONFIG_ATM_DRIVERS=y \
	CONFIG_ATM_BR2684=y \
	CONFIG_ATM_MPOA=y \
	CONFIG_PPPOATM=y \
	CONFIG_ATM_LANE=y 
endef

define KernelPackage/atm_stack/description
 Kernel built-in support for ATM stack
endef

$(eval $(call KernelPackage,atm_stack))

define KernelPackage/lantiq_atm_builtin
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=LTQ ATM Support (built-in)
  DEPENDS:=@!PACKAGE_kmod-lantiq_atm_mod @TARGET_lantiq +kmod-atm_stack
  KCONFIG:= \
	CONFIG_LTQ_ATM=y \
	CONFIG_LTQ_ATM_TASKLET=y
endef

define KernelPackage/lantiq_atm_builtin/description
 Kernel support for LTQ ATM - built-in
endef

$(eval $(call KernelPackage,lantiq_atm_builtin))

define KernelPackage/lantiq_atm_retx
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=LTQ ATM ReTransmission Support
  DEPENDS:=@PACKAGE_kmod-lantiq_atm_builtin||PACKAGE_kmod-lantiq_atm_mod
  KCONFIG:= \
	CONFIG_LTQ_ATM_RETX=y
endef

define KernelPackage/lantiq_atm_retx/description
 Kernel support for LTQ ATM ReTransmission
endef

$(eval $(call KernelPackage,lantiq_atm_retx))

define KernelPackage/lantiq_eth_drv_builtin
 SUBMENU:=$(LANTIQ_MENU)
 TITLE:= LTQ Ethernet Driver (Built-in Support)
 DEPENDS:=@TARGET_lantiq
 KCONFIG:= \
	CONFIG_LANTIQ_ETH_DRV=y
endef

define KernelPackage/lantiq_eth_drv_builtin/description
 Kernel support for LTQ Ethernet Driver (Built-in Support)
endef

$(eval $(call KernelPackage,lantiq_eth_drv_builtin))

define KernelPackage/intel_eth_drv_xrx500_module
 SUBMENU:=$(INTEL_MENU)
 TITLE:= Intel Ethernet Driver for xRX500 (Module Support)
 DEPENDS:=@TARGET_lantiq_xrx500
 KCONFIG:= \
	CONFIG_LTQ_ETH_XRX500
 AUTOLOAD:=$(call AutoLoad,41,ltq_eth_drv_xrx500)
 FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/lantiq/ltq_eth_drv_xrx500.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/intel_eth_drv_xrx500_module/description
 Intel Ethernet Driver (Module Support)
endef

$(eval $(call KernelPackage,intel_eth_drv_xrx500_module))

define KernelPackage/lantiq_mini_jumbo_frame
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Mini Jumbo Frames Support
  KCONFIG:= \
		CONFIG_LTQ_MINI_JUMBO_FRAME_SUPPORT=y
endef

define KernelPackage/lantiq_mini_jumbo_frame/description
	Support for mini jumbo frames (also called as baby jumbo frames).
endef

$(eval $(call KernelPackage,lantiq_mini_jumbo_frame))

define KernelPackage/lantiq_log
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Target Support for Log 
  KCONFIG:= \
	CONFIG_NETFILTER_XT_TARGET_LOG=y 
endef

define KernelPackage/lantiq_log/description
  Target support for LOG 
endef

$(eval $(call KernelPackage,lantiq_log))

define KernelPackage/lantiq_imq
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Support for IMQ 
  KCONFIG:= \
	CONFIG_IMQ=y \
	CONFIG_IMQ_BEHAVIOR_AA=y \
	CONFIG_IMQ_NUM_DEVS=3 \
	CONFIG_NETFILTER_XT_TARGET_IMQ=y \
	CONFIG_IFB=y
  FILES:=$(LINUX_DIR)/drivers/net/imq.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/lantiq_imq/description
  Kernel support for IMQ
endef

$(eval $(call KernelPackage,lantiq_imq))

define KernelPackage/lantiq_qos
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Lantiq support for QoS
  DEPENDS:=@TARGET_lantiq
  KCONFIG:= \
	CONFIG_LANTIQ_IPQOS=y \
	CONFIG_LANTIQ_ALG_QOS=y \
	CONFIG_LANTIQ_IPQOS_MARK_SKBPRIO=y \
	CONFIG_LANTIQ_IPQOS_CLASS_ACCELERATION_DISABLE=y \
	CONFIG_NET_SCHED=y \
	CONFIG_NET_SCH_FIFO=y \
	CONFIG_NET_SCH_CLK_GETTIMEOFDAY=y \
	CONFIG_NET_SCH_CBQ=y \
	CONFIG_NET_SCH_HTB=y \
	CONFIG_NET_SCH_RED=y \
	CONFIG_NET_SCH_DSMARK=y \
	CONFIG_NET_CLS=y \
	CONFIG_NET_CLS_FW=y \
	CONFIG_NET_CLS_U32=y \
	CONFIG_NETFILTER_XT_MATCH_U32=y \
	CONFIG_LTQ_ALG_QOS=y \
	CONFIG_IP_NF_TARGET_TOS=y \
	CONFIG_NETFILTER_XT_TARGET_DSCP=y \
	CONFIG_BRIDGE_NETFILTER=n \
	CONFIG_NET_SCH_ESFQ=n \
	CONFIG_NET_SCH_TEQL=n \
	CONFIG_NET_SCH_HFSC=n \
	CONFIG_VLAN_8021Q=y \
	CONFIG_LTQ_IPQOS_BRIDGE_EBT_IMQ=y
endef

define KernelPackage/lantiq_qos/description
  Kernel Support for QoS. This package enables classifier and queuing disciplines (HTB, CBQ, FIFO etc) in Kernel configuration.
endef

$(eval $(call KernelPackage,lantiq_qos))

define KernelPackage/lantiq_vlan_qos
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Lantiq support for QoS based on LAN VLANs
  DEPENDS:=@!PACKAGE_kmod-qinq-support +kmod-lantiq_qos
  KCONFIG:= \
	CONFIG_VLAN_8021Q_UNTAG=y
endef

define KernelPackage/lantiq_vlan_qos/description
  Kernel Support for ingress vlan based QoS.
endef

$(eval $(call KernelPackage,lantiq_vlan_qos))

define KernelPackage/lantiq_layer7
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=LTQ layer7 support
  KCONFIG:= CONFIG_NETFILTER_XT_MATCH_LAYER7=m
  FILES:=$(LINUX_DIR)/net/netfilter/xt_layer7.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/lantiq_layer7/description
  Kernel Support for layer7. This package enables layer7 Kernel configuration.
endef
$(eval $(call KernelPackage,lantiq_layer7))

define KernelPackage/lantiq_session_limiter
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=LTQ Session Limiter
  KCONFIG:= CONFIG_LTQ_HANDLE_CONNTRACK_SESSIONS=y
endef

define KernelPackage/lantiq_session_limiter/description
  Kernel Support for session limiting.. This package enables session limiter Kernel configuration.
endef
$(eval $(call KernelPackage,lantiq_session_limiter))

define KernelPackage/lantiq_extmark
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=LTQ Extmark
  KCONFIG:= CONFIG_NF_CONNTRACK_EXTMARK=y \
	    CONFIG_NETFILTER_XT_TARGET_CONNEXTMARK=m
  FILES:=$(LINUX_DIR)/net/netfilter/xt_CONNEXTMARK.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/lantiq_extmark/description
  Kernel Support for extended mark
endef
$(eval $(call KernelPackage,lantiq_extmark))

define KernelPackage/ipv6-kernel
  SUBMENU:=Netfilter Extensions
  TITLE:=Extra IPv6 kernel options
  KCONFIG:= \
	CONFIG_IPV6=y \
	CONFIG_IPV6_ROUTER_PREF=y \
	CONFIG_IPV6_ROUTE_INFO=y \
	CONFIG_NF_CONNTRACK_IPV6=y \
	CONFIG_NET_IP_TUNNEL=m \
	CONFIG_IP6_NF_QUEUE=n
  FILES:=$(LINUX_DIR)/net/ipv4/ip_tunnel.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/ipv6-kernel/description
  Enable extra IPv6 kernel options
endef

$(eval $(call KernelPackage,ipv6-kernel))

define KernelPackage/ipv6-firewall
  SUBMENU:=Netfilter Extensions
  TITLE:=IPv6 firewall options
  DEPENDS:= +kmod-ipv6-kernel
  KCONFIG:= \
	CONFIG_IP6_NF_IPTABLES=m \
	CONFIG_IP6_NF_MATCH_AH=n \
	CONFIG_IP6_NF_MATCH_EUI64=m \
	CONFIG_IP6_NF_MATCH_FRAG=m \
	CONFIG_IP6_NF_MATCH_OPTS=m \
	CONFIG_IP6_NF_MATCH_HL=n \
	CONFIG_IP6_NF_MATCH_IPV6HEADER=m \
	CONFIG_IP6_NF_MATCH_MH=m \
	CONFIG_IP6_NF_MATCH_RT=m \
	CONFIG_IP6_NF_TARGET_HL=n \
	CONFIG_IP6_NF_TARGET_LOG=m \
	CONFIG_IP6_NF_FILTER=m \
	CONFIG_IP6_NF_TARGET_REJECT=m \
	CONFIG_IP6_NF_MANGLE=m \
	CONFIG_IP6_NF_RAW=n \
	CONFIG_IP6_NF_SECURITY=n
endef

define KernelPackage/ipv6-firewall/description
  Enable  IPv6 firewall specific options
endef

$(eval $(call KernelPackage,ipv6-firewall))


define KernelPackage/lan-port-sep
  SUBMENU:=Lantiq
  TITLE:=LAN Port Seperation
  KCONFIG:= \
	CONFIG_LTQ_ETHSW=m \
	CONFIG_LTQ_PPA_PORT_SEPARATION=y
  AUTOLOAD:=$(call AutoLoad,10,lantiq_ethsw)
  FILES:=$(LINUX_DIR)/drivers/net/lantiq_ethsw.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/lan-port-sep/description
  Enable support for LAN port seperation driver. The driver creates virtual interface for each of switch LAN ports.
endef

$(eval $(call KernelPackage,lan-port-sep))

define KernelPackage/ltq_portbinding
 SUBMENU:=Lantiq
 DEPENDS:=@PACKAGE_kmod-lan-port-sep
 TITLE:=Lantiq PortBinding 
 KCONFIG:= \
       CONFIG_IP_ADVANCED_ROUTER=y \
       CONFIG_IP_MULTIPLE_TABLES=y \
       CONFIG_BRIDGE_NETFILTER=y \
       CONFIG_NETFILTER_XT_MATCH_PHYSDEV=y
endef

define KernelPackage/ltq_portbinding//description
 kernel support for PortBinding 
endef

$(eval $(call KernelPackage,ltq_portbinding))

define KernelPackage/lantiq_ebtables
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS:=@!PACKAGE_kmod-ebtables
  TITLE:=Bridge firewalling modules
  KCONFIG:= \
          CONFIG_BRIDGE_NF_EBTABLES=y \
          CONFIG_BRIDGE_EBT_BROUTE=y \
          CONFIG_BRIDGE_EBT_T_FILTER=y \
          CONFIG_BRIDGE_EBT_802_3=n \
          CONFIG_BRIDGE_EBT_AMONG=n \
          CONFIG_BRIDGE_EBT_LIMIT=n \
          CONFIG_BRIDGE_EBT_MARK=y \
          CONFIG_BRIDGE_EBT_EXTMARK=y \
          CONFIG_BRIDGE_EBT_PKTTYPE=y \
          CONFIG_BRIDGE_EBT_STP=n \
          CONFIG_BRIDGE_EBT_VLAN=y \
          CONFIG_BRIDGE_EBT_MARK_T=y \
          CONFIG_BRIDGE_EBT_EXTMARK_T=y \
          CONFIG_BRIDGE_EBT_REDIRECT=y
endef

define KernelPackage/lantiq_ebtables/description
  ebtables is a general, extensible frame/packet identification
  framework. It provides you to do Ethernet
  filtering/NAT/brouting on the Ethernet bridge.
endef

$(eval $(call KernelPackage,lantiq_ebtables))


define KernelPackage/lantiq_ebtables-ipv4
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS:=kmod-lantiq_ebtables
  TITLE:=ebtables: IPv4 support
  KCONFIG:= \
          CONFIG_BRIDGE_EBT_ARP=n \
          CONFIG_BRIDGE_EBT_IP=y \
          CONFIG_BRIDGE_EBT_ARPREPLY=n \
          CONFIG_BRIDGE_EBT_DNAT=n
  FILES:= $(LINUX_DIR)/net/bridge/netfilter/ebt_ip.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/lantiq_ebtables-ipv4/description
 This option adds the IPv4 support to ebtables, which allows basic
 IPv4 header field filtering, ARP filtering as well as SNAT, DNAT targets.
endef

$(eval $(call KernelPackage,lantiq_ebtables-ipv4))


define KernelPackage/lantiq_ebtables-ipv6
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS:=kmod-lantiq_ebtables
  TITLE:=ebtables: IPv6 support
  KCONFIG:= \
          CONFIG_BRIDGE_EBT_IP6=n
endef

define KernelPackagelantiq_ebtables-ipv6/description
 This option adds the IPv6 support to ebtables, which allows basic
 IPv6 header field filtering and target support.
endef

$(eval $(call KernelPackage,lantiq_ebtables-ipv6))

define KernelPackage/lantiq_sflash_support
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Lantiq SPI Flash support
  KCONFIG:= \
	CONFIG_LANTIQ_SPI=y \
	CONFIG_LANTIQ_SPI_DEBUG=y \
	CONFIG_LANTIQ_SPI_FLASH=y \
	CONFIG_SPI_BITBANG=n \
	CONFIG_SPI_GPIO=n

endef

define KernelPackage/lantiq_sflash_support/description
 This option is used to enable Lantiq SPI Flash support.
endef

$(eval $(call KernelPackage,lantiq_sflash_support))

define KernelPackage/lantiq_ipsec
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=IPSec Support
  KCONFIG:= \
	CONFIG_CRYPTO_HW=y \
	CONFIG_CRYPTO_DEV_LANTIQ_DEU=y \
	CONFIG_CRYPTO_DEV_PWR_SAVE_MODE=n \
	CONFIG_XFRM_USER=y \
	CONFIG_XFRM_SUB_POLICY=y \
	CONFIG_XFRM_IPCOMP=y \
	CONFIG_NET_KEY=y \
	CONFIG_INET_AH=y \
	CONFIG_INET_ESP=y \
	CONFIG_INET_IPCOMP=y \
	CONFIG_INET_XFRM_TUNNEL=y \
	CONFIG_INET_TUNNEL=y \
	CONFIG_INET_XFRM_MODE_TRANSPORT=y \
	CONFIG_INET_XFRM_MODE_TUNNEL=y \
	CONFIG_CRYPTO_AEAD=y \
	CONFIG_CRYPTO_AUTHENC=y \
	CONFIG_CRYPTO_HMAC=y \
	CONFIG_CRYPTO_DEFLATE=y \
	CONFIG_NETFILTER_XT_MATCH_POLICY=y \
	CONFIG_NET_IPIP=y \
	CONFIG_CRYPTO_RNG=y \
	CONFIG_CRYPTO_GF128MUL=y \
	CONFIG_CRYPTO_GCM=y \
	CONFIG_CRYPTO_SEQIV=y \
	CONFIG_CRYPTO_CTR=y \
	CONFIG_CRYPTO_ECB=y \
	CONFIG_CRYPTO_XCBC=y \
	CONFIG_CRYPTO_GHASH=y \
	CONFIG_CRYPTO_MD5=y \
	CONFIG_CRYPTO_BLOWFISH=y
endef

define KernelPackage/lantiq_ipsec/description
  IPSec Support
endef

$(eval $(call KernelPackage,lantiq_ipsec))

define KernelPackage/lantiq-cpufreq
  TITLE:=Lantiq CPUFREQ support
  SUBMENU:=Lantiq
  DEPENDS:= \
	+cpufrequtils \
	@+PACKAGE_ltq-wlan-wave&&FEATURE_LTQ_WAVE_AR10_SUPPORT:PMCU_SUPPORT \
	@+PACKAGE_ltq-voice-tapi:LTQ_VOICE_TAPI_PMC
  KCONFIG:= \
	CONFIG_CPU_FREQ=y \
	CONFIG_CPU_FREQ_GOV_USERSPACE=y \
	CONFIG_CPU_FREQ_GOV_LANTIQGOV=y \
	CONFIG_LTQ_CPUFREQ=y \
	CONFIG_LTQ_CPU_FREQ=y
endef

define KernelPackage/lantiq-cpufreq/description
  Cpu frequency scaling support (CPUFreq driver) for Lantiq SoC's.
endef

$(eval $(call KernelPackage,lantiq-cpufreq))

define KernelPackage/lantiq-dvs
  TITLE:=Lantiq DVS support
  SUBMENU:=Lantiq
  DEPENDS:= \
	+ltq-regulator
  KCONFIG:= \
	CONFIG_LTQ_CPUFREQ_DVS=y
endef

define KernelPackage/lantiq-dvs/description
  Dynamic Voltage Scaling support for digital core voltage in conjunction
  with cpu frequency scaling. Higher cpu clock rate -> higher voltage and vice
  versa.
endef

$(eval $(call KernelPackage,lantiq-dvs))

define KernelPackage/lantiq_vdsl_vectoring_support
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=PTM Vectoring Support
  KCONFIG += \
	CONFIG_PTM_VECTORING=y
endef

define KernelPackage/lantiq_vdsl_vectoring_support/description
  PTM Vectoring support
endef

$(eval $(call KernelPackage,lantiq_vdsl_vectoring_support))

define KernelPackage/usb-ncm
  TITLE:=Support for USB Network Control Model Device CDC NCM
  KCONFIG:= \
	CONFIG_USB_NET_CDC_NCM=m \
	CONFIG_USB_NET_CDCETHER=m
  FILES:= \
		$(LINUX_DIR)/drivers/net/usb/cdc_ncm.ko \
		$(LINUX_DIR)/drivers/net/usb/cdc_ether.ko
  SUBMENU:=$(USB_MENU)
  DEPENDS:= +kmod-usb-net
endef

define KernelPackage/usb-ncm/description
 Kernel support for USB Network Control Model Device CDC NCM
endef

$(eval $(call KernelPackage,usb-ncm))

define KernelPackage/usb-sierra-lte
  TITLE:=Support for USB Network Control Model Sierra Device
  KCONFIG:= \
	CONFIG_USB_LTE_SIERRA_HL7548
  FILES:= \
  	$(LINUX_DIR)/drivers/net/usb/sierra_lte_reset.ko
  SUBMENU:=$(USB_MENU)
  DEPENDS:= +kmod-usb-ncm
endef

define KernelPackage/usb-sierra-lte/description
 Kernel support for USB Network Control Model Sierra Device
endef

$(eval $(call KernelPackage,usb-sierra-lte))


define KernelPackage/jffs2
  SUBMENU:=Filesystems
  TITLE:=JFFS2 filesystem Support
  KCONFIG:= \
	CONFIG_JFFS2_FS=y
endef

define KernelPackage/jffs2/description
  JFFS2 filesystem Support
endef

$(eval $(call KernelPackage,jffs2))

FS_MENU:=Filesystems

define KernelPackage/fs-nfs-root
  SUBMENU:=$(FS_MENU)
  TITLE:=NFS root support
  KCONFIG:= \
	CONFIG_NFS_FS=y \
	CONFIG_NFS_V2=y \
	CONFIG_IP_PNP=y \
	CONFIG_IP_PNP_DHCP=y \
	CONFIG_IP_PNP_BOOTP=y \
	CONFIG_IP_PNP_RARP=y \
	CONFIG_LOCKD=y \
	CONFIG_ROOT_NFS=y \
	CONFIG_RPCSEC_GSS_KRB5=y \
	CONFIG_SUNRPC=y \
	CONFIG_SUNRPC_GSS=y

endef

define KernelPackage/fs-nfs-root/description
 Enables kernel settings for NFS root support
endef

$(eval $(call KernelPackage,fs-nfs-root))

define KernelPackage/lantiq-vmb
  SUBMENU:=Lantiq
  TITLE:=VPE Management Support (VMB) for CMP Framework
  KCONFIG:= \
        CONFIG_LTQ_VMB=y \
        CONFIG_NR_CPUS=4
endef

define KernelPackage/lantiq-vmb/description
  VPE Management Support (VMB) for CMP Framework
endef

$(eval $(call KernelPackage,lantiq-vmb))

define KernelPackage/lantiq-eva-2gb
  SUBMENU:=Lantiq
  TITLE:=Enhanced Virtual Addressing (EVA) 2GB support
  KCONFIG:= \
	CONFIG_EVA_2GB=y
endef

define KernelPackage/lantiq-eva-2gb/description
  Enables Enhanced Virtual Addressing (EVA) to access memory upto 2GB.
  Kernel's start address will change to 0x2000 0000 instead of 0x8000 0000.
endef

$(eval $(call KernelPackage,lantiq-eva-2gb))

define KernelPackage/lantiq-eva-1gb
  SUBMENU:=Lantiq
  TITLE:=Enhanced Virtual Addressing (EVA) 1GB support
  KCONFIG:= \
	CONFIG_EVA_1GB=y
endef

define KernelPackage/lantiq-eva-1gb/description
  Enables Enhanced Virtual Addressing (EVA) to access memory upto 1GB.
  Kernel's start address will change to 0x6000 0000 instead of 0x8000 0000.
endef

$(eval $(call KernelPackage,lantiq-eva-1gb))

define KernelPackage/lantiq-tos-size-16
  SUBMENU:=Lantiq
  TITLE:=TOS Size 16M
  DEPENDS:=@TARGET_lantiq_xrx500_4kec @UBOOT_CONFIG_TOS_16M
  KCONFIG:=CONFIG_TOS_SIZE_16M=y
endef

define KernelPackage/lantiq-tos-size-16/description
  Tos size 16
endef

$(eval $(call KernelPackage,lantiq-tos-size-16))

define KernelPackage/lantiq-tos-size-32
  SUBMENU:=Lantiq
  TITLE:=TOS Size 32M
  DEPENDS:=@TARGET_lantiq_xrx500_4kec @UBOOT_CONFIG_TOS_32M
  KCONFIG:=CONFIG_TOS_SIZE_32M=y
endef

define KernelPackage/lantiq-tos-size-32/description
  Tos size 32
endef

$(eval $(call KernelPackage,lantiq-tos-size-32))

define KernelPackage/lantiq-tos-size-64
  SUBMENU:=Lantiq
  TITLE:=TOS Size 64M
  DEPENDS:=@TARGET_lantiq_xrx500_4kec @UBOOT_CONFIG_TOS_64M
  KCONFIG:=CONFIG_TOS_SIZE_64M=y
endef

define KernelPackage/lantiq-tos-size-64/description
  Tos size 64
endef

$(eval $(call KernelPackage,lantiq-tos-size-64))

define KernelPackage/lantiq-tos-size-128
  SUBMENU:=Lantiq
  TITLE:=TOS Size 128M
  DEPENDS:=@TARGET_lantiq_xrx500_4kec @UBOOT_CONFIG_TOS_128M
  KCONFIG:=CONFIG_TOS_SIZE_128M=y
endef

define KernelPackage/lantiq-tos-size-128/description
  Tos size 128
endef

$(eval $(call KernelPackage,lantiq-tos-size-128))

define KernelPackage/lantiq-itc
  SUBMENU:=Lantiq
  TITLE:=Inter Thread Communication (ITC) for GRX500
  KCONFIG:= \
        CONFIG_LTQ_ITC=y
endef

define KernelPackage/lantiq-itc/description
  Inter Thread Communication (ITC) for GRX500 to support synchronisation 
  between multiple TCs. Currently ITC Cells are used as binary Semaphores.
endef

$(eval $(call KernelPackage,lantiq-itc))

define KernelPackage/lantiq-grx500iapwdt
  SUBMENU:=Lantiq
  TITLE:=Watchdog driver for InterAptiv cores
  KCONFIG:= \
	CONFIG_WATCHDOG=y \
	CONFIG_WATCHDOG_CORE=y \
	CONFIG_WATCHDOG_NOWAYOUT=y \
	CONFIG_GRX500_IAP_WDT=y
endef

define KernelPackage/lantiq-grx500iapwdt/description
  Enables Per-VPE Watchdog driver for InterAptiv cores
endef

$(eval $(call KernelPackage,lantiq-grx500iapwdt))

define KernelPackage/lantiq-bootcorewdt
  SUBMENU:=Lantiq
  TITLE:=Watchdog driver for Bootcore (MIPS4Kec) cores
  KCONFIG:= \
        CONFIG_WATCHDOG=y \
        CONFIG_WATCHDOG_CORE=y \
        CONFIG_WATCHDOG_NOWAYOUT=y \
        CONFIG_GRX500_BOOTCORE_WDT=y
endef

define KernelPackage/lantiq-bootcorewdt/description
  Enables Bootcore Watchdog Driver implementation .
endef

$(eval $(call KernelPackage,lantiq-bootcorewdt))

define KernelPackage/hwmon-ina2xx
  SUBMENU:=Hardware Monitoring Support
  TITLE:=TI power monitoring support
  KCONFIG:= \
		CONFIG_HWMON=y \
		CONFIG_SENSORS_INA2XX=y
  DEPENDS:=kmod-i2c-lantiq-build-in
endef

define KernelPackage/hwmon-ina2xx/description
  Kernel module for TI power monitor chips.
endef

$(eval $(call KernelPackage,hwmon-ina2xx))

define KernelPackage/hwmon-xrx500-spdmon
  SUBMENU:=Hardware Monitoring Support
  TITLE:=LANTIQ XRX500 speed monitor
  KCONFIG:= \
		CONFIG_HWMON=y \
		CONFIG_LTQ_SPDMON=y
endef

define KernelPackage/hwmon-xrx500-spdmon/description
  Kernel module for Lantiq grx500 platform.
endef

$(eval $(call KernelPackage,hwmon-xrx500-spdmon))

define KernelPackage/regulator-tps65273
  SUBMENU:=Hardware Power Regulator Support
  TITLE:=TI power regulator support
  KCONFIG:= \
		CONFIG_REGULATOR=y \
		CONFIG_REGULATOR_TPS65273=y
  DEPENDS:=kmod-i2c-lantiq-build-in
endef

define KernelPackage/tps65273-regulator/description
  Kernel module for TI power regulator chips.
endef

$(eval $(call KernelPackage,regulator-tps65273))

define KernelPackage/memory-compaction
  SUBMENU:=Other modules
  TITLE:=Support for memory compaction
  KCONFIG:=CONFIG_COMPACTION=y
endef

define KernelPackage/memory-compaction/description
 Enables kernel settings for memory compaction support
endef

$(eval $(call KernelPackage,memory-compaction))

define KernelPackage/lantiq-new-pcie
  SUBMENU:=$(LANTIQ_MENU)
  TITLE:=Lantiq PCIe Bus Support (New Framework)
  DEPENDS:=@TARGET_lantiq
  KCONFIG:= \
	CONFIG_PCIE_LANTIQ=y \
	CONFIG_PCIE_LANTIQ_MSI=y \
	CONFIG_PCI_MSI=y \
	CONFIG_PCIEPORTBUS=y \
	CONFIG_PCIEASPM=n
endef

define KernelPackage/lantiq-new-pcie/description
 Enables PCIe Bus Support (New Framework)
endef

$(eval $(call KernelPackage,lantiq-new-pcie))

define KernelPackage/vrx318-dp-mod
  SUBMENU:=Lantiq
  TITLE:=VRX318 Datapath ATM/PTM modules (xRX500 SoC)
  KCONFIG:= \
	CONFIG_VRX318_DATAPATH=y \
	CONFIG_VRX318_TC=m
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/lantiq/vrx318/vrx318_tc.ko
endef

define KernelPackage/vrx318-dp-mod/description
   Enables VRX318 Datapath and builds ATM/PTM driver as Modules for xRX500 SoCs
endef

$(eval $(call KernelPackage,vrx318-dp-mod))

define KernelPackage/pcie-vrx518
  SUBMENU:=Lantiq
  TITLE:=VRX518 PCIe EP/ACA module
  KCONFIG:= \
	CONFIG_VRX518=y \
	CONFIG_NET_VENDOR_INTEL=y
endef

define KernelPackage/pcie-vrx518/description
   Enables VRX518 PCIe EP/ACA driver
endef

$(eval $(call KernelPackage,pcie-vrx518))

define KernelPackage/vrx518-dp-mod
  SUBMENU:=Lantiq
  TITLE:=VRX518 Datapath ATM/PTM modules (xRX500 SoC)
  KCONFIG:= \
	CONFIG_VRX518_TC=m
  FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/intel/vrx518/tc/vrx518_tc.ko
endef

define KernelPackage/vrx518-dp-mod/description
   Enables VRX518 Datapath and builds ATM/PTM driver as Modules for xRX500 SoCs
endef
$(eval $(call KernelPackage,vrx518-dp-mod))

define KernelPackage/pcie-switch-vrx518-bonding
  SUBMENU:=Lantiq
  TITLE:=VRX518 PCIe switch for bonding 
  DEPENDS+=@PACKAGE_kmod-vrx518-dp-mod
  KCONFIG:= \
        CONFIG_VRX518_PCIE_SWITCH_BONDING=y
endef

define KernelPackage/pcie-switch-vrx518-bonding/description
   Enables VRX518 PCIe switch for bonding
endef

$(eval $(call KernelPackage,pcie-switch-vrx518-bonding))


define KernelPackage/lantiq_directconnect_support
 SUBMENU:=$(LANTIQ_MENU)
 TITLE:=LTQ DirectConnect Datapath Driver
 DEPENDS:=@TARGET_lantiq_xrx500
 KCONFIG:= \
	CONFIG_LTQ_DIRECTCONNECT_DP=m
 FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/lantiq/directconnect_dp/ltq_directconnect_datapath.ko
endef

define KernelPackage/lantiq_directconnect_support/description
 Kernel support for LTQ DirectConnect Datapath Driver
endef

$(eval $(call KernelPackage,lantiq_directconnect_support))

define KernelPackage/ltq_hwmcpy
 SUBMENU:=$(LANTIQ_MENU)
 TITLE:=LTQ Hardware Memcopy Driver
 DEPENDS:=@TARGET_lantiq_xrx500
 KCONFIG:= \
	CONFIG_DMADEVICES=y \
	CONFIG_LTQ_HWMCPY=y
endef

define KernelPackage/ltq_hwmcpy/description
 Kernel support for LTQ Hardware Memcopy Engine Driver
endef

$(eval $(call KernelPackage,ltq_hwmcpy))

define KernelPackage/lantiq-csrc-gptu
  SUBMENU:=Lantiq
  TITLE:=GPTU Clocksource
  KCONFIG:= \
	CONFIG_CSRC_GPTC=y \
	CONFIG_CEVT_GPTC=y
endef

define KernelPackage/lantiq-csrc-gptu/description
  Enables GPTU as clocksource and for events
endef

$(eval $(call KernelPackage,lantiq-csrc-gptu))

define KernelPackage/kernel-config-support
  SUBMENU:=Other modules
  TITLE:=Support for kernel-config-support on target
  KCONFIG:= \
	CONFIG_IKCONFIG=y \
	CONFIG_IKCONFIG_PROC=y
endef

define KernelPackage/kernel-config-support/description
 Enables the complete linux kernel ".config" file contents to be saved in the kernel.Can be extracted from a running kernel by reading /proc/config.gz
endef

$(eval $(call KernelPackage,kernel-config-support))

define KernelPackage/pmi-ngi
  SUBMENU:=$(OTHER_MENU)
  TITLE:=pmi ngi Support
  KCONFIG:=CONFIG_LTQ_PMINGI
  FILES:=$(LINUX_DIR)/drivers/char/pmi-ngi.ko
  AUTOLOAD:=$(call AutoLoad,240,pmi-ngi)
endef

define KernelPackage/pmi-ngi/description
  Lantiq Kernel module for pmi ngi support
endef

$(eval $(call KernelPackage,pmi-ngi))

define KernelPackage/grx500-a21
  SUBMENU:=Lantiq
  TITLE:=GRX500 A21 specific changes
  KCONFIG:= \
	CONFIG_SOC_GRX500_A21=y
endef

define KernelPackage/grx500-a21/description
  Compilation flag which will be used for GRX500 A21
  related changes.
endef

$(eval $(call KernelPackage,grx500-a21))

define KernelPackage/nand-sw-ecc
  SUBMENU:=Lantiq
  TITLE:=Enable Nand sw Ecc (BCH)
  DEPENDS:=@UBOOT_CONFIG_NAND_ECC_BCH
  KCONFIG := \
	CONFIG_MTD_NAND_XWAY=y \
	CONFIG_MTD_NAND_ECC_BCH=y \
	CONFIG_LTQ_BCH_MODE=y
endef

define KernelPackage/nand-sw-ecc/description
  Enable Nand sw-ecc, which will use BCH for ECC
endef

$(eval $(call KernelPackage,nand-sw-ecc))

define KernelPackage/lantiq-wrt-image
  TITLE:=Lantiq Openwrt Image
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS:=@TARGET_lantiq
  KCONFIG:= \
	CONFIG_MTD_SPLIT_FIRMWARE=y \
	CONFIG_MTD_SPLIT_FIRMWARE_NAME="firmwareA" \
	CONFIG_MTD_UIMAGE_SPLIT=y \
	CONFIG_MTD_ROOTFS_SPLIT=n
endef

define KernelPackage/lantiq-wrt-image/description
  Kernel support for the Lantiq openwrt firmware
endef

$(eval $(call KernelPackage,lantiq-wrt-image))

define KernelPackage/lantiq-stat-helper
  TITLE:=PPA STAT HELPER
  SUBMENU:=$(LANTIQ_MENU)
  DEPENDS:=@PACKAGE_kmod-lantiq_ppa_xrx200
  KCONFIG:= \
	CONFIG_LTQ_STAT_HELPER=y \
	CONFIG_LTQ_STAT_HELPER_XRX200=y
endef

define KernelPackage/lantiq-stat-helper/description
  kernel module for counter and statistics
endef

$(eval $(call KernelPackage,lantiq-stat-helper))

define KernelPackage/cpuload
 SUBMENU:=$(OTHER_MENU)
 TITLE:=LTQ cpuload tool
 KCONFIG:= \
        CONFIG_CPULOAD=m
 FILES:=$(LINUX_DIR)/drivers/char/cpuload.ko
endef

define KernelPackage/cpuload/description
 Kernel support for LTQ per-VPE cpuload tool
endef

$(eval $(call KernelPackage,cpuload))

define KernelPackage/benand
 SUBMENU:=$(OTHER_MENU)
 TITLE:=BENAND Support
 KCONFIG:= \
        CONFIG_MTD_NAND_BENAND=y \
        CONFIG_MTD_NAND_BENAND_ENABLE=y
endef

define KernelPackage/benand/description
 Kernel support for BENAND(Embedded ECC NAND) 
endef

$(eval $(call KernelPackage,benand))

define KernelPackage/drv-toe
 SUBMENU:=$(LANTIQ_MENU)
 TITLE:=TOE (TSO and LRO) Support
 KCONFIG:= \
	CONFIG_LTQ_TOE_DRIVER=y
endef

define KernelPackage/drv-toe/description
 Kernel support for TOE (TSO and LRO)
endef

$(eval $(call KernelPackage,drv-toe))

define KernelPackage/wavflow
 SUBMENU:=$(LANTIQ_MENU)
 TITLE:=Wave Flow Support
 KCONFIG:= \
        CONFIG_MAC80211=y \
        CONFIG_WLAN=y \
        CONFIG_IWLWIFI \
        CONFIG_IWLDVM \
        CONFIG_IWLMVM \
        CONFIG_WIDAN_NETFILTER
 FILES:= \
        $(LINUX_DIR)/drivers/net/widan_netf.ko \
        $(LINUX_DIR)/drivers/net/wireless/iwlwifi/iwlwifi.ko \
        $(LINUX_DIR)/drivers/net/wireless/iwlwifi/dvm/iwldvm.ko \
        $(LINUX_DIR)/drivers/net/wireless/iwlwifi/mvm/iwlmvm.ko \
        $(LINUX_DIR)/drivers/net/wireless/iwlwifi/mvm/iwlmvm.ko
endef

define KernelPackage/wavflow/description
 Kernel support for Waveflow support
endef

$(eval $(call KernelPackage,wavflow))

define KernelPackage/qinq-support
 SUBMENU:=$(LANTIQ_MENU)
 TITLE:=QinQ support in kernel
 DEPENDS:=@TARGET_lantiq_xrx500
 KCONFIG:= \
	CONFIG_BRIDGE_VLAN_FILTERING=y \
	CONFIG_VLAN_8021Q_UNTAG=n
endef

define KernelPackage/qinq-support/description
 Kernel support for enabling qinq configuration
endef

$(eval $(call KernelPackage,qinq-support))

define KernelPackage/tproxy
  SUBMENU:=Lantiq
  TITLE:=TPROXY modules in kernel
  KCONFIG := \
	CONFIG_NETFILTER_TPROXY=y \
	CONFIG_NETFILTER_XT_MATCH_SOCKET=y \
	CONFIG_NETFILTER_XT_TARGET_TPROXY=y
endef

define KernelPackage/tproxy/description
  TPROXY modules in kernel
endef

$(eval $(call KernelPackage,tproxy))

define KernelPackage/prio_mark
  SUBMENU:=Intel
  TITLE:=Prioritiy marking module
  KCONFIG:=CONFIG_BOOST_MARK=y
endef

define KernelPackage/prio_mark/description
  Kernel module to mark prioritiy of skb
endef

$(eval $(call KernelPackage,prio_mark))

define KernelPackage/l2nat
  SUBMENU:=Intel
  TITLE:=Layer 2 nat driver
  KCONFIG:=CONFIG_L2NAT=m
  FILES:=$(LINUX_DIR)/drivers/net/l2nat/l2nat.ko
endef

define KernelPackage/l2nat/description
 Kernel support for Layer 2 nat for bridged client mode
endef

$(eval $(call KernelPackage,l2nat))
