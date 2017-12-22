# 
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ragel
PKG_VERSION:=6.9
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.colm.net/files/ragel

PKG_BUILD_PARALLEL:=0
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
    SECTION:=puma7
    CATEGORY:=Intel
    TITLE:= ragel

    MAKE_FLAGS=$(HOST_MAKE_FLAGS)

    CONFIGURE_VARS=$(HOST_CONFIGURE_VARS)
    CONFIGURE_ARGS=$(HOST_CONFIGURE_ARGS)
endef

define Package/$(PKG_NAME)/description
    ragel host tool
endef

define Package/$(PKG_NAME)/install
    $(MAKE) -C $(PKG_BUILD_DIR) SHELL="$(BASH)" install
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
