#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=open_uboot
PKG_VERSION:=2010.06
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/u-boot-$(PKG_VERSION)
PKG_SOURCE:=u-boot-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.denx.de/pub/u-boot
PKG_MD5SUM:=cd42bc64b6edafa6930ce299a144503e
PKG_CAT:=bzcat
#PKG_TARGETS:=bin
PKG_BUILD_DEPENDS:=opkg/host PACKAGE_secure_boot:secure_boot

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) grep '^CONFIG_UBOOT_' $(TOPDIR)/.config | md5s)
endif
  
define Package/$(PKG_NAME)
  SECTION:=boot
  CATEGORY:=Boot Loaders
  DEPENDS:=@TARGET_ltqcpe||TARGET_lantiq||TARGET_intel_xway
  TITLE:=U-Boot for Lantiq SoC based platforms
  URL:=http://www.denx.de/wiki/UBoot/WebHome
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	cp -r ./files/* $(PKG_BUILD_DIR)
endef

#define Build/Configure
#endef

# Target Macros
ifeq ($(CONFIG_TARGET_$(BOARD)_platform_ar9)$(CONFIG_TARGET_$(BOARD)_ar9),y)
  UBOOT_PLATFORM:=ar9
endif

ifdef CONFIG_TARGET_ltqcpe_platform_vbg400
UBOOT_PLATFORM:=vbg400
endif

ifeq ($(CONFIG_TARGET_$(BOARD)_platform_danube)$(CONFIG_TARGET_$(BOARD)_danube),y)
  UBOOT_PLATFORM:=danube
endif

ifeq ($(CONFIG_TARGET_$(BOARD)_platform_amazon_se)$(CONFIG_TARGET_$(BOARD)_ase),y)
  UBOOT_PLATFORM:=amazon_se
endif

ifeq ($(CONFIG_TARGET_$(BOARD)_platform_vr9)$(CONFIG_TARGET_$(BOARD)_vr9)$(CONFIG_TARGET_$(BOARD)_xrx200),y)
  UBOOT_PLATFORM:=vr9
endif

ifeq ($(CONFIG_TARGET_$(BOARD)_platform_ar10)$(CONFIG_TARGET_$(BOARD)_ar10)$(CONFIG_TARGET_$(BOARD)_xrx300),y)
  ifdef CONFIG_TARGET_ltqcpe_platform_ar10_GRX390 
    UBOOT_PLATFORM:=grx390
  else
    UBOOT_PLATFORM:=ar10
  endif
endif

ifeq ($(CONFIG_TARGET_$(BOARD)_xrx330),y)
  UBOOT_PLATFORM:=grx390
endif

ifeq ($(CONFIG_TARGET_$(BOARD)_platform_hn1)$(CONFIG_TARGET_$(BOARD)_hn1),y)
  UBOOT_PLATFORM:=hn1
endif

ifdef CONFIG_TARGET_ifxcpe_platform_vbg400
UBOOT_PLATFORM:=vbg400
endif

ifdef CONFIG_TARGET_lantiq_xrx500
UBOOT_PLATFORM:=grx500
endif

ifdef CONFIG_TARGET_lantiq_xrx500_4kec
UBOOT_PLATFORM:=grx500
endif

ifdef CONFIG_TARGET_intel_xway_xrx500
UBOOT_PLATFORM:=grx500
endif

ifdef CONFIG_TARGET_intel_xway_xrx500_4kec
UBOOT_PLATFORM:=grx500
endif


UBOOT_IMAGE :=$(PKG_BUILD_DIR)/u-boot.ltq

ifdef CONFIG_UBOOT_CONFIG_BOOT_FROM_NAND
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/u-boot-nand.bin
ifeq ($(CONFIG_TARGET_$(BOARD)_platform_hn1)$(CONFIG_TARGET_$(BOARD)_hn1),y)
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/u-boot.uart
ifdef CONFIG_UBOOT_CONFIG_ENABLE_DDR_DCDC
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/dcdc_ddr_patch.uart
endif
ifndef CONFIG_UBOOT_CONFIG_HN1_GPHY_FW_EMBEDDED
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/gphy_firmware.img
endif
endif
endif


ifdef CONFIG_UBOOT_CONFIG_BUILD_ENV_BLOCK
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/ubootenv.img
endif



ifdef CONFIG_UBOOT_CONFIG_BOOT_FROM_SPI
ifeq ($(CONFIG_TARGET_$(BOARD)_platform_hn1)$(CONFIG_TARGET_$(BOARD)_hn1),y)
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/u-boot.uart
ifdef CONFIG_UBOOT_CONFIG_ENABLE_DDR_DCDC
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/dcdc_ddr_patch.uart
endif
ifndef CONFIG_UBOOT_CONFIG_HN1_GPHY_FW_EMBEDDED
UBOOT_IMAGE +=$(PKG_BUILD_DIR)/gphy_firmware.img
endif
endif
endif

define Package/open_uboot/config
        menu "Configuration"
                depends on PACKAGE_open_uboot
                source "$(SOURCE)/config/Kconfig"
        endmenu
endef


define Build/Configure
        rm -f $(PKG_BUILD_DIR)/.configured*
endef

define Build/Compile
	#ln -sdf $(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_uboot $(PKG_BUILD_DIR)/open_uboot
	cd $(PKG_BUILD_DIR);chmod a+x ./scripts_platform/*;make clean distclean;make $(UBOOT_PLATFORM)_config;\
        grep 'CONFIG_UBOOT_' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_UBOOT_\\(.*\\),\\1\\2,g" > $(PKG_BUILD_DIR)/.config;\
        make oldconfig CROSS_COMPILE=$(TARGET_CROSS);make clean all CROSS_COMPILE=$(TARGET_CROSS)
endef

define Package/$(PKG_NAME)/install
	mkdir -p $(TARGET_IMG_DIR)
	cp -f $(UBOOT_IMAGE) $(TARGET_IMG_DIR)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
