#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minissdpd
PKG_VERSION:=1.5
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://miniupnp.free.fr/files/
PKG_MD5SUM:=be556df1550f49aedd39172ca0a68f48

include $(INCLUDE_DIR)/package.mk

define Package/minissdpd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Simple Service Discovery Protocol daemon
  URL:=http://miniupnp.free.fr/minissdpd.html
  DEPENDS:=+libnfnetlink
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
        $(TARGET_CONFIGURE_OPTS) \
        CFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE"
endef

define Package/minissdpd/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/etc/init.d $(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minissdpd $(1)/usr/bin/minissdpd
	$(INSTALL_BIN) ./files/minissdpd.init $(1)/etc/init.d/minissdpd
	$(INSTALL_CONF) ./files/minissdpd.config $(1)/etc/config/minissdpd
endef

$(eval $(call BuildPackage,minissdpd))
