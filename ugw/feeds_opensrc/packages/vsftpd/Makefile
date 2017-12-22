#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vsftpd
PKG_VERSION:=3.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://security.appspot.com/downloads/
PKG_MD5SUM:=ad9fa952558c2c5b0426ccaccff0f972

include $(INCLUDE_DIR)/package.mk

define Package/vsftpd
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A fast and secure FTP server
  URL:=http://vsftpd.beasts.org/
  MAINTAINER:=Cezary Jackiewicz <cezary@eko.one.pl>
  DEPENDS:= +libopenssl
endef

define Package/vsftpd/conffiles
/etc/vsftpd.conf
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		vsftpd
endef

define Package/vsftpd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/$(PKG_NAME).filterusers.sh $(1)/etc/init.d/
	$(INSTALL_CONF) ./files/$(PKG_NAME).conf $(1)/etc/$(PKG_NAME).conf
	$(INSTALL_CONF) ./files/$(PKG_NAME).pem $(1)/etc/$(PKG_NAME).pem
	$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE), \
		$(INSTALL_CONF) ./files/$(PKG_NAME).users $(1)/etc/$(PKG_NAME).users; \
	, \
		$(INSTALL_DIR) $(1)/ramdisk_copy/flash; \
		$(INSTALL_CONF) ./files/$(PKG_NAME).users $(1)/ramdisk_copy/flash/$(PKG_NAME).users; \
		cd $(1); ln -s /ramdisk/flash/$(PKG_NAME).users etc/$(PKG_NAME).users; cd ->/dev/null; \
	)
endef
##$(INSTALL_DIR) $(1)/etc/init.d
##$(INSTALL_BIN) ./files/$(PKG_NAME).init $(1)/etc/init.d/$(PKG_NAME)

$(eval $(call BuildPackage,vsftpd))
