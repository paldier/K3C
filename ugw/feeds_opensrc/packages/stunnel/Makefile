#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stunnel
PKG_VERSION:=4.55
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://pkgs.fedoraproject.org/repo/pkgs/stunnel/stunnel-4.55.tar.gz/md5/ebe6670368d30c6a01ca33b9c50cb239/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=ebe6670368d30c6a01ca33b9c50cb239

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/stunnel
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenssl +libwrap
  TITLE:=SSL TCP Wrapper
  URL:=http://www.stunnel.org/
  DEPENDS:=+libpthread +libopenssl +zlib
endef

define Package/stunnel/description
	Stunnel is a program that allows you to encrypt arbitrary TCP
	connections inside SSL (Secure Sockets Layer) available on both Unix
	and Windows. Stunnel can allow you to secure non-SSL aware daemons and
	protocols (like POP, IMAP, LDAP, etc) by having Stunnel provide the
	encryption, requiring no changes to the daemon's code.
endef

define Package/stunnel/conffiles
/etc/stunnel/stunnel.conf
endef

CONFIGURE_ARGS+= \
	--with-random=/dev/urandom \
	--with-threads=fork \
	--with-ssl=$(STAGING_DIR)/usr \

TARGET_CFLAGS += \
	$(if $(CONFIG_SSP_SUPPORT),,-fno-stack-protector)

define Build/Compile
	mkdir -p $(PKG_INSTALL_DIR)/etc/stunnel
	echo '#dummy' > $(PKG_INSTALL_DIR)/etc/stunnel/stunnel.pem
	$(call Build/Compile/Default)
endef

define Package/stunnel/install
	$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE), \
		$(INSTALL_DIR) $(1)/etc/stunnel; \
		$(INSTALL_DATA) ./files/stunnel.pem $(1)/etc/stunnel/; \
		$(INSTALL_DATA) ./files/stunnel.conf $(1)/etc/stunnel/; \
		chmod 777 $(1)/etc/stunnel/stunnel.conf; \
		chmod 600 $(1)/etc/stunnel/stunnel.pem; \
	, \
		$(INSTALL_DIR) $(1)/ramdisk_copy/etc/stunnel; \
		$(INSTALL_DATA) ./files/stunnel.pem $(1)/ramdisk_copy/etc/stunnel/; \
		$(INSTALL_DATA) ./files/stunnel.conf $(1)/ramdisk_copy/etc/stunnel/; \
		chmod 777 $(1)/ramdisk_copy/etc/stunnel/stunnel.conf; \
		chmod 600 $(1)/ramdisk_copy/etc/stunnel/stunnel.pem; \
		$(INSTALL_DIR) $(1)/etc; \
		cd $(1);ln -s /ramdisk/etc/stunnel etc/stunnel;cd -; \
	)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/stunnel $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/stunnel
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/stunnel/libstunnel.so $(1)/usr/lib/stunnel/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/stunnel.init $(1)/etc/init.d/stunnel
endef

$(eval $(call BuildPackage,stunnel))
