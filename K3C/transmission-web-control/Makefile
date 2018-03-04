#
# Copyright (C) 2016-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=transmission-web-control
PKG_VERSION:=1.4.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ronggang/transmission-web-control
PKG_SOURCE_VERSION:=6028d412d7fb5bd9df995862793344dbb2ec79a7
PKG_SOURCE_SUBDIR ?= $(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE ?= $(PKG_SOURCE_SUBDIR).tar.xz

PKG_MAINTAINER:=ronggang

include $(INCLUDE_DIR)/package.mk

define Package/transmission-web-control
  SUBMENU:=BitTorrent
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@(PACKAGE_transmission-daemon-openssl||PACKAGE_transmission-daemon-mbedtls)
  TITLE:=Transmission Web Control
  URL:=https://github.com/ronggang/transmission-web-control
  PKGARCH:=all
endef

define Package/transmission-web-control/description
	Transmission Web Control is a custom web UI. The project began in Google Code.
endef

define Build/Compile
endef

define Package/transmission-web-control/install
	$(INSTALL_DIR) $(1)/usr/share/transmission/web
	$(CP) $(PKG_BUILD_DIR)/src/* $(1)/usr/share/transmission/web
endef

$(eval $(call BuildPackage,transmission-web-control))
