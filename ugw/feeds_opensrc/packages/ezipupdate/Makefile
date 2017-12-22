#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12305 2008-08-14 12:03:55Z blogic $

include $(TOPDIR)/rules.mk

PKG_NAME:=ez-ipupdate
PKG_VERSION:=3.0.11b8
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).orig.tar.gz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

####
PKG_SOURCE_URL:=https://downloads.openwrt.org/sources/
#PKG_MD5SUM:=f5f491564812db5d8783daa538c49186
PKG_NO_AUTORECONF=y
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_ez_ipupdate
PKG_BUILD_DEPENDS:= +libugwhelper

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk
####
define Package/$(PKG_NAME)
  SUBMENU:=DDNS 
  SECTION:=net
  CATEGORY:=Network
  TITLE:=ez-ipupdate (ipupdate service)
  DEPENDS:= 
endef

define Package/$(PKG_NAME)/description
      ez-ipupdate for updating ipaddress using DDNS servers
endef
#
define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
#	config FEATURE_DDNS
#           bool
#      	   default y
#              depends on PACKAGE_ez-ipupdate
endef
#
#ifeq ($(PACKAGE_ez-ipupdate),y)
#	export CONFIG_FEATURE_DDNS=y
#endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ez-ipupdate $(1)/usr/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
