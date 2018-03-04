#
# Copyright (C) 2016-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yaaw
PKG_VERSION:=2017-08-28
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/aa65535/yaaw-zh-hans
PKG_SOURCE_VERSION:=6074435606cfed1da942c546b850b998f12ed400
PKG_VERSION ?= $(if $(PKG_SOURCE_DATE),$(PKG_SOURCE_DATE)-)$(call version_abbrev,$(PKG_SOURCE_VERSION))
PKG_SOURCE_SUBDIR ?= $(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE ?= $(PKG_SOURCE_SUBDIR).tar.xz

PKG_LICENSE:=LGPL-3.0
PKG_MAINTAINER:=Hsing-Wang Liao <kuoruan@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/yaaw
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Download Manager
  DEPENDS:=
  TITLE:=Yet another aria2 web frontend
  URL:=https://github.com/binux/yaaw
  PKGARCH:=all
endef

define Package/yaaw/description
	Yet Another Aria2 Web Frontend in pure HTML/CSS/Javascirpt.
endef

define Build/Compile
endef

define Package/yaaw/install
	$(INSTALL_DIR) $(1)/opt/lantiq/www/yaaw
	$(CP) \
		$(PKG_BUILD_DIR)/{css,img,js} \
		$(1)/opt/lantiq/www/yaaw
	$(CP) \
		$(PKG_BUILD_DIR)/{index.html,offline.appcache} \
		$(1)/opt/lantiq/www/yaaw
endef

$(eval $(call BuildPackage,yaaw))
