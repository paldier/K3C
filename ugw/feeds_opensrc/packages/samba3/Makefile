#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=samba3
PKG_VERSION:=3.0.24
PKG_RELEASE:=6
PKG_MD5SUM:=89273f67a6d8067cbbecefaa13747153

PKG_SOURCE_URL:=ftp://ftp.samba.org/pub/samba/old-versions/
PKG_SOURCE:=samba-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/samba-$(PKG_VERSION)/

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS+=$(if $(CONFIG_SAMBA3_DEBUG),-DSAMBA_DEBUG)

define Package/samba3
  SECTION:=net
  CATEGORY:=Network
  TITLE:=samba3
  DEPENDS:=+libpthread +librpc +@BUSYBOX_CONFIG_ADDUSER +@BUSYBOX_CONFIG_FEATURE_ADDUSER_LONG_OPTIONS +@BUSYBOX_CONFIG_DELUSER
  URL:=http://www.samba.org/
endef

define Package/samba3/Description
  Samba3 - made small with patches taken from AVM GPL releases and freetz 
endef

define Package/samba3/config
	source "$(SOURCE)/Config.in"
endef

CONFIGURE_ARGS += --enable-shared=ALL

define Build/Prepare/LFS
	cat ./files/config-lfs.h >> $(PKG_BUILD_DIR)/source/include/config.h
endef

define Build/Prepare
$(call Build/Prepare/Default)
$(call Build/Prepare/LFS)
endef

define Build/Compile
	CC="$(TARGET_CC)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="$(TARGET_CPPFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(MAKE) -C $(PKG_BUILD_DIR)/source \
		all bin/smbpasswd
endef

#User management is now handled by User Accounts management under system.
#So not copying ./files/update_user.sh
define Package/samba3/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE),etc,ramdisk_copy/flash)/samba
	$(INSTALL_DIR) $(1)/etc/{init.d,config}
	$(INSTALL_DIR) $(1)/etc/hotplug.d/{mount,iface}
	$(INSTALL_BIN) ./files/samba.init $(1)/etc/init.d/samba
	$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE),,$(INSTALL_DATA) ./files/functions.sh $(1)/etc/functions.sh)
	$(INSTALL_DATA) ./files/samba.config $(1)/etc/config/samba
	$(INSTALL_BIN) ./files/01-mount-event-samba $(1)/etc/hotplug.d/mount/
	$(INSTALL_BIN) ./files/03-iface-event-samba $(1)/etc/hotplug.d/iface/
	$(INSTALL_DATA) ./files/$(if $(CONFIG_TARGET_x86_puma),xRX750/)smb.conf.template $(1)/$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE),etc,ramdisk_copy/flash)/samba/
	$(INSTALL_DATA) ./files/$(if $(CONFIG_TARGET_x86_puma),xRX750/)smb.conf $(1)/$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE),etc,ramdisk_copy/flash)/samba/
	touch $(1)/$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE),etc,ramdisk_copy/flash)/samba/smbpasswd
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source/bin/smbpasswd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/source/bin/smbd $(1)/usr/sbin/
	cd $(1)/bin; ln -sf /usr/sbin/smbpasswd smbpasswd; cd -
	$(if $(CONFIG_LANTIQ_OPENWRT_FIRMWARE), \
		sed -i 's/#START/START/g' $(1)/etc/init.d/samba; \
		$(CP) ./files/uci-defaults $(1)/etc/; \
	, \
		$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/www; \
		cd $(1); ln -s /ramdisk/flash/samba etc/samba; cd -; \
		$(CP) ./files/www/* $(1)/$(VENDOR_PATH)/www/; \
	)
endef

$(eval $(call BuildPackage,samba3))
