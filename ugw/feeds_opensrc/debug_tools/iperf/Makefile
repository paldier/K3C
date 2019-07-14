#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iperf
PKG_VERSION:=2.0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/$(PKG_NAME)
PKG_MD5SUM:=44b5536b67719f4250faed632a3cd016

include $(INCLUDE_DIR)/package.mk

define Package/iperf/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:= +libstdcpp
  TITLE:=Internet Protocol bandwidth measuring tool
  URL:=http://sourceforge.net/projects/iperf/
endef

define Package/iperf/Default/description
 Iperf is a modern alternative for measuring TCP and UDP bandwidth
 performance, allowing the tuning of various parameters and
 characteristics.
endef

define Package/iperf
$(call Package/iperf/Default)
  TITLE+= (with single thread support)
endef

define Package/iperf/description
$(call Package/iperf/Default/description)
 This package is built with single thread support.
endef

define Package/iperf-mt
$(call Package/iperf/Default)
  DEPENDS+= +libpthread
  TITLE+= (with multithread support)
endef

define Package/iperf-mt/description
$(call Package/iperf/Default/description)
 This package is built with multithread support.
endef

define Build/Template

$(STAMP_BUILT)-$(2): $(STAMP_PREPARED)
	-$(MAKE) -C $(PKG_BUILD_DIR) clean
	$(call Build/Configure/Default,$(3))
	$(MAKE) -C $(PKG_BUILD_DIR)
	( cd $(PKG_BUILD_DIR)/src; mv -f iperf iperf-$(2) )
	touch $$@

$(STAMP_BUILT): $(STAMP_BUILT)-$(2)

define Package/$(1)/install
	$(INSTALL_DIR) $$(1)/usr/bin
	$(INSTALL_BIN) $$(PKG_BUILD_DIR)/src/iperf-$(2) $$(1)/usr/bin/iperf
endef

endef

CONFIGURE_ARGS += \
	--disable-multicast \

CONFIGURE_VARS += \
	CXXFLAGS="$$$$CXX_CFLAGS -fno-rtti" \

ifneq ($(SDK)$(CONFIG_PACKAGE_iperf),)
  define Build/singlethread
    $(call Build/Template,iperf,singlethread, \
	--disable-threads \
    )
  endef
endif
$(eval $(Build/singlethread))

ifneq ($(SDK)$(CONFIG_PACKAGE_iperf-mt),)
  define Build/multithread
    $(call Build/Template,iperf-mt,multithread, \
	--enable-threads=posix \
	LDFLAGS="$(TARGET_LDFLAGS) -lpthread" \
	ac_cv_func_pthread_cancel=no \
    )
  endef
endif
$(eval $(Build/multithread))

$(eval $(call BuildPackage,iperf))
$(eval $(call BuildPackage,iperf-mt))
