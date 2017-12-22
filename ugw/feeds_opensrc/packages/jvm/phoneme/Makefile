#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=phoneme
PKG_VERSION:=advanced-mr2-dev-b167
#PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://localhost
PKG_MD5SUM:=a56639b5f8a8569d18e04c025070c82b
PKG_CAT:=zcat
#PKG_BUILD_DEPENDS:=

include $(INCLUDE_DIR)/package.mk

define Package/phoneme
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A Java Virtual Machine
  #DESCRIPTION:=
  URL:=http://davy.preuveneers.be/phoneme/?q=node/10#armgnueabi
  DEPENDS:=@TARGET_ltqcpe_platform_vr9 @PACKAGE_kmod-ltqcpe_nand
endef

define Build/Prepare
$(call Build/Prepare/Default)
	$(CP) ./files/* $(PKG_BUILD_DIR)/.
endef

#export J2ME_CLASSLIB=foundation
#export TARGET_CROSS
#$(MAKE) -C $(PKG_BUILD_DIR)/cdc/build/linux-mips-openwrt/ J2ME_CLASSLIB=foundation
define Build/Compile
	:
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/bin/cvm  $(1)/usr/bin/
	$(CP) $(PKG_BUILD_DIR)/lib/*  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,phoneme))
