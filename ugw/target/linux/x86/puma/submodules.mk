#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/puma_ppa
 SUBMENU:=Lantiq
 TITLE:=PPA Support for PUMA
 DEPENDS:=@TARGET_x86
 
 KCONFIG:= \
	CONFIG_PPA_PUMA7=y \
	CONFIG_LTQ_PPA=y \
	CONFIG_LTQ_PPA_API=y \
	CONFIG_LTQ_PPA_DATAPATH=y\
	CONFIG_LTQ_PPA_HAL_SELECTOR=y\
	CONFIG_LTQ_PPA_API_DIRECTPATH=n \
	CONFIG_LTQ_PPA_QOS=y \
	CONFIG_LTQ_PPA_API_PROC=y \
	CONFIG_PPA_PUMA_HAL

endef

define KernelPackage/puma_ppa/description
 Kernel support for PPA stack 
endef

$(eval $(call KernelPackage,puma_ppa))

define KernelPackage/mrpc
 SUBMENU:=Intel
 TITLE:=RPC Over HW Mailbox Support
 DEPENDS:=@TARGET_x86
 KCONFIG:=CONFIG_MRPC=y
endef

define KernelPackage/mrpc/description
 Intel RPC Over HW Mailbox (mrpc) support
endef

$(eval $(call KernelPackage,mrpc))

define KernelPackage/mrpc_examples
 SUBMENU:=Intel
 TITLE:=RPC Over HW Mailbox (mrpc) examples
 DEPENDS:=@TARGET_x86 +kmod-mrpc
 KCONFIG:= \
       CONFIG_MRPC_EXAMPLES=y \
       CONFIG_MRPC_CLIENT_EXAMPLE=m \
       CONFIG_MRPC_SERVER_EXAMPLE=m
 FILES:= \
       $(LINUX_DIR)/drivers/mrpc/examples/client_example.ko \
       $(LINUX_DIR)/drivers/mrpc/examples/server_example.ko
endef

define KernelPackage/mrpc_examples/description
 Intel RPC Over HW Mailbox (mrpc) client and server examples
endef

$(eval $(call KernelPackage,mrpc_examples))

define KernelPackage/mrpc_modphy
 SUBMENU:=Intel
 TITLE:=mrpc modphy client
 DEPENDS:=@TARGET_x86 +kmod-mrpc
 KCONFIG:=CONFIG_MRPC_MODPHY_CLIENT=y
endef

define KernelPackage/mrpc_modphy/description
 mrpc modphy client
endef

$(eval $(call KernelPackage,mrpc_modphy))

define KernelPackage/mrpc_pp
 SUBMENU:=Intel
 TITLE:=mrpc pp client
 DEPENDS:=@TARGET_x86 +kmod-mrpc
 KCONFIG:=CONFIG_MRPC_PP_CLIENT=y
endef

define KernelPackage/mrpc_pp/description
 mrpc pp client
endef

$(eval $(call KernelPackage,mrpc_pp))

define KernelPackage/mrpc_cppi
 SUBMENU:=Intel
 TITLE:=mrpc cppi client
 DEPENDS:=@TARGET_x86 +kmod-mrpc
 KCONFIG:=CONFIG_MRPC_CPPI_CLIENT=y
endef

define KernelPackage/mrpc_cppi/description
 mrpc cppi client
endef

$(eval $(call KernelPackage,mrpc_cppi))

define KernelPackage/puma_vrx320_ep
 SUBMENU:=Lantiq
 TITLE:=VRX320 EP Support for PUMA
 DEPENDS:=@TARGET_x86
 KCONFIG:= \
        CONFIG_NET_VENDOR_LANTIQ=y \
        CONFIG_LANTIQ_VRX320=y \
        CONFIG_PCI_GODIRECT=n \
        CONFIG_PCI_GOANY=y \
        CONFIG_PCI_BIOS=y \
        CONFIG_PCI_DIRECT=y \
        CONFIG_PCI_MMCONFIG=y \
        CONFIG_PCI_DOMAINS=y
endef

define KernelPackage/puma_vrx320_ep/description
  VRX320 endpoint driver
endef

$(eval $(call KernelPackage,puma_vrx320_ep))

define KernelPackage/atm_stack_on_puma
  SUBMENU:=Lantiq
  TITLE:=ATM stack
  DEPENDS:=@TARGET_x86
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

$(eval $(call KernelPackage,atm_stack_on_puma))

define KernelPackage/ipsec
 SUBMENU:=Intel
 TITLE:=IPSec Support for PUMA
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
        CONFIG_CRYPTO_AES_NI_INTEL=m \
        CONFIG_CRYTPO_ABLK_HELPER=m \
        CONFIG_CRYPTO_MD5=y \
        CONFIG_CRYPTO_BLOWFISH=y
 ifeq ($(CONFIG_TARGET_x86_puma),y)
 FILES:= \
        $(LINUX_DIR)/arch/x86/crypto/aesni-intel.ko \
        $(LINUX_DIR)/arch/x86/crypto/ablk_helper.ko
 endif
endef

define KernelPackage/ipsec/description
 IPSec Support for PUMA
endef

$(eval $(call KernelPackage,ipsec))

define KernelPackage/synopsys
 SUBMENU:= Lantiq
 TITLE:= synopsys ethernet module
 DEPENDS:= @TARGET_x86
 KCONFIG:= CONFIG_DWC_QOS=m
 FILES:= $(LINUX_DIR)/drivers/net/ethernet/synopsys/DWC_ETH_QOS.ko
endef

define KernelPackage/synopsys/description
  synopsys ethernet module
endef

$(eval $(call KernelPackage,synopsys))

define KernelPackage/pp_init
 SUBMENU:= Lantiq
 TITLE:= pp tx init kernel module
 DEPENDS:= @TARGET_x86
 KCONFIG:= CONFIG_PP_TX_INIT=m
 FILES:= $(LINUX_DIR)/arch/x86/pp_init/puma7_pp_init.ko
endef

define KernelPackage/pp_init/description
  pp tx init kernel module
endef

$(eval $(call KernelPackage,pp_init))

define KernelPackage/bonding
 SUBMENU:=Lantiq
 TITLE:=Bond driver for linux
 DEPENDS:=
 KCONFIG:= CONFIG_PACKAGE_kmod-bonding=m
 FILES:= $(LINUX_DIR)/drivers/net/bonding/bonding.ko
endef

define KernelPackage/bonding/description
  Enable kernel bonding driver
endef

$(eval $(call KernelPackage,bonding))

define KernelPackage/puma_switch_api
 SUBMENU:=Lantiq
 TITLE:=switch api module
 DEPENDS:=@TARGET_x86 +kmod-synopsys
 
 KCONFIG:= \
	CONFIG_LTQ_ETHSW_API=m

 FILES:= \
	$(LINUX_DIR)/drivers/net/ethernet/lantiq/switch-api/drv_switch_api.ko
endef

define KernelPackage/puma_switch_api/description
  switch api driver
endef

$(eval $(call KernelPackage,puma_switch_api))

define KernelPackage/lantiq_qos
  SUBMENU:=Lantiq
  TITLE:=Lantiq support for QoS
  #DEPENDS:= +ebtables +ebtables-utils 
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
	CONFIG_VLAN_8021Q_UNTAG=y \
	CONFIG_IFB=y \
	CONFIG_NETWORK_EXTMARK=y \
	CONFIG_NETFILTER_XT_TARGET_EXTMARK=y \
	CONFIG_NETFILTER_XT_MATCH_EXTMARK=y \
	CONFIG_LANTIQ_IPQOS=y \
	CONFIG_LANTIQ_IPQOS_MARK_SKBPRIO=y \
	CONFIG_LANTIQ_IPQOS_CLASS_ACCELERATION_DISABLE=y \
	CONFIG_VLAN_8021Q=y \
	CONFIG_VLAN_8021Q_UNTAG=y \
	CONFIG_LANTIQ_ALG_QOS=y \
	CONFIG_CLS_U32_EXTMARK=y

endef

define KernelPackage/lantiq_qos/description
  Kernel Support for QoS. This package enables classifier and queuing disciplines (HTB, CBQ, FIFO etc) in Kernel configuration.
endef

$(eval $(call KernelPackage,lantiq_qos))

define KernelPackage/lan_port_separation
 SUBMENU:=Lantiq
 TITLE:=LAN port separation for puma
 DEPENDS:=@TARGET_x86 +kmod-puma_switch_api +kmod-gphy_event
 
 KCONFIG:= \
	CONFIG_LTQ_VLAN_SWITCH_PORT_ISOLATION=y
endef

define KernelPackage/lan_port_separation/description
  Enable LAN port separation for puma
endef

$(eval $(call KernelPackage,lan_port_separation))

define KernelPackage/SFP_Phy_puma_driver
 SUBMENU:=Intel
 TITLE:=SFP phy driver for puma
 DEPENDS:=@TARGET_x86
 
 KCONFIG:= \
	CONFIG_SFP_PHY=y
endef

define KernelPackage/SFP_Phy_puma_driver/description
  SFP phy driver for puma
endef

$(eval $(call KernelPackage,SFP_Phy_puma_driver))

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
