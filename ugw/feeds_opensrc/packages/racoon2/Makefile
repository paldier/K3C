#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=racoon2
PKG_VERSION:=20100526a
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://pkgs.fedoraproject.org/repo/pkgs/racoon2/racoon2-20100526a.tgz/md5/2fa33abff1ccd6fc22876a23db77aaa8/
PKG_MD5SUM:=2fa33abff1ccd6fc22876a23db77aaa8

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/racoon2
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=@!LINUX_2_4 +libopenssl
  TITLE:=racoon2
  URL:=http://www.racoon2.wide.ad.jp/w/
endef

CONFIGURE_ARGS += \
	--disable-kinkd \
	--enable-shared \
	--enable-static \
	--sysconfdir=/etc

MAKE_FLAGS+=\
	CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS) $(TARGET_CPPFLAGS) $(EXTRA_CPPFLAGS) -DHAVE_STDARG_H -DHAVE_NSSWITCH_CONF -DHAVE_INTTYPES_H -DHAVE_STDINT_H -D__USE_BSD"

define Build/Prepare
	$(call Build/Prepare/Default)
	chmod -R u+w $(PKG_BUILD_DIR)
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); touch \
		configure.ac \
		aclocal.m4 \
		Makefile.in \
		config.h.in \
		configure \
	);
	$(call Build/Configure/Default)
	echo "#undef HAVE_SHADOW_H" >> $(PKG_BUILD_DIR)/config.h
endef

define Package/racoon2/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/* $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/*.conf.* $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/hook
	$(INSTALL_DIR) $(1)/etc/hook/ikesa-up.d
	$(INSTALL_DIR) $(1)/etc/hook/child-up.d
	$(CP) $(PKG_INSTALL_DIR)/etc/hook/*/ $(1)/etc/hook/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/etc/init.d/* $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/etc
	$(CP) files/uci_to_ipsec_config.sh $(1)/$(VENDOR_PATH)/etc/
	$(INSTALL_DIR) $(1)/etc/config
	$(CP) files/ipsec $(1)/etc/config/
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/www
	$(CP) files/view_xmls/* $(1)/$(VENDOR_PATH)/www/
endef

$(eval $(call BuildPackage,racoon2))
