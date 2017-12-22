# 
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12693 2008-09-25 10:47:39Z nico $

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bridge-utils
PKG_RELEASE:=1
PKG_SOURCE_URL:=@SF/bridge
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_bridge_utils
PKG_NO_AUTORECONF=y

PKG_VERSION:=1.4
PKG_MD5SUM:=0182fcac3a2b307113bbec34e5f1c673

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=Ethernet bridging configuration utility
  URL:=http://bridge.sourceforge.net/
  MENU:=1
endef

define Package/$(PKG_NAME)/description
 Manage ethernet bridging: a way to connect networks together to 
 form a larger network.
endef

CONFIGURE_ARGS += \
	--with-lq-cflags="-DFEATURE_LQ_BRCTL_SHOWMACS_IFNAME" \
	--with-linux-headers="$(LINUX_DIR)" 

define Build/Prepare
$(call Build/Prepare/Default)
#	$(CP) $(FILES_DIR)/brctl/* $(PKG_BUILD_DIR)/brctl/;
#	$(CP) $(FILES_DIR)/libbridge/* $(PKG_BUILD_DIR)/libbridge/;
#	$(CP) $(FILES_DIR)/misc $(PKG_BUILD_DIR)/;
	( cd $(PKG_BUILD_DIR) ; \
		[ -f ./configure ] || { \
			ln -sf configure.in configure.ac ; \
			autoconf ; \
		} \
	)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brctl/brctl $(1)/usr/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
