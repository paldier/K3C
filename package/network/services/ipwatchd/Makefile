# Copyright (C) 2009-2010 OpenWrt.org
# All rights reserved.

include $(TOPDIR)/rules.mk

PKG_NAME:=ipwatchd
PKG_VERSION:=1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://ipwatchd.sourceforge.net
PKG_MD5SUM:=8485c98d7539b2f8b5bad370d523fdc3

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=IP conflict detection tool for Linux
  DEPENDS:=+libnet1
  URL:=http://ipwatchd.sourceforge.net
endef

define Package/$(PKG_NAME)/description
  IP conflict detection tool for Linux
endef

define Package/$(PKG_NAME)/conffiles
/etc/$(PKG_NAME).conf
endef

define Build/Compile
	CC="$(TARGET_CROSS)gcc" \
	CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -I$(STAGING_DIR)/usr/lib/libnet-1.1.x/include -L$(STAGING_DIR)/usr/lib/libnet-1.1.x/lib" \
	$(MAKE) -C $(PKG_BUILD_DIR)/src
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin/ $(1)/etc/ $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_DATA) ./files/$(PKG_NAME).conf $(1)/etc/
	$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)
	$(INSTALL_DIR) $(1)/usr/share/$(PKG_NAME)
	$(INSTALL_BIN) ./files/$(PKG_NAME).script $(1)/usr/share/$(PKG_NAME)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
