#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12175 2008-08-05 22:04:10Z florian $

include $(TOPDIR)/rules.mk

PKG_NAME:=zebra
PKG_VERSION:=0.94
PKG_RELEASE:=1
PKG_MD5SUM:=ff1633f1ac026b720fa37b1856be3f48
PKG_FIXUP=libtool
PKG_NO_AUTORECONF=y

PKG_SOURCE_URL:=http://ftp.ripe.net/mirrors/sites/ftp.zebra.org/pub/zebra/

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_zebra

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=The zebra Software Routing Suite
  URL:=ftp://ftp.zebra.org
  MENU:=1
endef

define Package/$(PKG_NAME)/description
	GNU Zebra is free software that manages TCP/IP based routing protocols
endef

define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	config FEATURE_RIP
		default y
		depends on PACKAGE_$(PKG_NAME)
		bool 'RIP Support'
endef

CONFIGURE_ARGS  += --disable-vtysh --disable-ipv6 \
		--enable-ripd_standalone --disable-debug_ripd_standalone \
		--disable-bgpd --disable-ripngd --disable-ospfd --disable-ospf6d \
		--disable-bgp-announce --disable-bgpd --disable-ripngd \
		--disable-ospfd --disable-ospf6d --disable-bgp-announce \
		--disable-zebra IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)" \

CONFIG_FULL_PACKAGE=y
IFX_CFLAGS  +=-DVTY_REMOVE -DRIPD_MEMORY_FIX

define Build/Prepare
	$(call Build/Prepare/Default)
	-find $(PKG_BUILD_DIR)/ -name Makefile.in | xargs rm -f
	-rm -rf $(PKG_BUILD_DIR)/{aclocal.m4,autom4te.cache,config.guess,config.h.in,config.sub,configure}
	-rm -rf $(PKG_BUILD_DIR)/{depcomp,INSTALL,install-sh,ltmain.sh,missing,ylwrap}
endef

define Build/Configure
	# prevent autoheader invocation
	touch $(PKG_BUILD_DIR)/stamp-h.in
	(cd $(PKG_BUILD_DIR)/; autoreconf -i)
	$(call Build/Configure/Default)
endef

define Build/Compile	
	$(MAKE) -C $(PKG_BUILD_DIR) IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)" all 
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ripd/ripd $(1)/usr/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
