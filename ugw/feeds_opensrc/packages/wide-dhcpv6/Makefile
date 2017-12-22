# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 6582 2007-03-16 20:21:39Z nbd $

include $(TOPDIR)/rules.mk
include $(TOPDIR)/include/kernel.mk

PKG_NAME:=wide-dhcpv6
PKG_VERSION:=20080615
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://sourceforge.net/projects/wide-dhcpv6/
#PKG_MD5SUM:=9903d06ba899838e2d3928dccb9d2d0c
PKG_MD5SUM:=1011e165850fe40d3e332dad2bfd30b3

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
IFX_APIS_DIR:=$(wildcard $(BUILD_DIR)/ltq-mapi*)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=DHCPv6 Client and Server
  DEPENDS:=+!NEW_FRAMEWORK:ltq-mapi
  URL:=http://sourceforge.net/projects/wide-dhcpv6/
endef

ifdef CONFIG_NEW_FRAMEWORK
DEP_FLAGS:="-DCONFIG_NEW_FRAMEWORK"
endif

ifndef CONFIG_NEW_FRAMEWORK
ifeq ($(CONFIG_PACKAGE_ltq-mapi),y)
DEP_FLAGS:="-I$(IFX_APIS_DIR)/include -DRCCONF_CHANGE"
DEP_LDFLAGS:="-L$(IFX_APIS_DIR)/lib/ -Wl,-rpath-link,$(IFX_APIS_DIR)/lib/,-rpath-link,$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib -loamapi -lIFXAPIs -lifx_common"
endif
endif

define Build/Prepare
	$(call Build/Prepare/Default)
endef

define Build/Configure
	$(call Build/Configure/Default)
endef


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) DEP_FLAGS=$(DEP_FLAGS) DEP_LDFLAGS=$(DEP_LDFLAGS)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dhcp6c $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dhcp6s $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ipv6helper $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
