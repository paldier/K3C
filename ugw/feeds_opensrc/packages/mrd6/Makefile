include $(TOPDIR)/rules.mk

PKG_NAME:=mrd6
PKG_VERSION:=0.9.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.openwrt.org/sources/
#PKG_MD5SUM:=361322198dafcc5ffab5e8e23197012b
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
#PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux24/open_mrd6

CONFIG_IPV6=1
EXTERNAL_MODULES ?= bgp.so
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Base system
  TITLE:=MRD6
  URL:=http://en.sourceforge.jp/
  DEPENDS:=+libstdcpp
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  Package for MRD6 Support 
endef

ifeq ($(CONFIG_EXTERNAL_TOOLCHAIN),)
  TOOLCHAIN_PATH:=$(TOOLCHAIN_DIR)
else
  TOOLCHAIN_PATH:=$(TOOLCHAIN_ROOT_DIR)
endif

define Build/Prepare
	$(call Build/Prepare/Default)
endef


define Build/Compile
	rm -f $(PKG_BUILD_DIR)/libstdc++.a $(PKG_BUILD_DIR)/src/libstdc++.a
	ln -s $(TOOLCHAIN_PATH)/usr/lib/libstdc++.a $(PKG_BUILD_DIR)/src/
	ln -s $(TOOLCHAIN_PATH)/usr/lib/libstdc++.a $(PKG_BUILD_DIR)/
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) STATIC_STDCXX=yes PLATFORM=OS_LINUX
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/src/mrd $(1)/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; $(RM) $(PKG_BUILD_DIR)/src/libstdc++.a \
	$(PKG_BUILD_DIR)/libstdc++.a; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
