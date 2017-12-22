# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 6582 2007-03-16 20:21:39Z nbd $

include $(TOPDIR)/rules.mk
include $(TOPDIR)/include/kernel.mk

PKG_NAME:=br2684ctl
PKG_VERSION:=20040226
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz
PKG_SOURCE_URL:=http://ftp.debian.org/debian/pool/main/b/br2684ctl
PKG_MD5SUM:=6eb4d8cd174e24a7c078eb4f594f5b69

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION).orig
PKG_BUILD_DEPENDS:=linux-atm
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_br2684ctl
PKG_NO_AUTORECONF=y
IFX_APIS_DIR:=$(wildcard $(BUILD_DIR)/ltq-mapi*)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=ATM Ethernet bridging configuration utility
  DEPENDS:=+linux-atm
  URL:=http://ftp.debian.org/debian/pool/main/b/br2684ctl
  MENU:=1
endef

### Menuconfig ##################################
define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	menu "$(PKG_NAME) features"
	depends on PACKAGE_$(PKG_NAME)
	source "$(SOURCE)/config/Config.in"
	endmenu
endef


#TARGET_LDFLAGS += -L$(IFX_APIS_DIR)/lib

ifeq ($(CONFIG_FEATURE_LTQ_VLAN),y)
	IFX_CFLAGS+= -DCONFIG_FEATURE_LTQ_VLAN -DCONFIG_WAN_VLAN_SUPPORT
endif

MAKE_FLAGS+= CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) $(IFX_CFLAGS) -I$(LINUX_DIR)/user_headers/include"

#define Build/Prepare
#	$(CP) $(FILES_DIR)/* $(PKG_BUILD_DIR)/.
#	$(call Build/Prepare/Default)
#endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME)d $(1)/usr/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
