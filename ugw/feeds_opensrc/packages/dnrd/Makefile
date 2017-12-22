include $(TOPDIR)/rules.mk

PKG_NAME:=dnrd
PKG_VERSION:=2.20.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://en.sourceforge.jp/projects/sfnet_dnrd/downloads/dnrd/2.20.3
PKG_MD5SUM:=41c9b070aae8ed403fc8c2aac7ab157c
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux24/open_dnrd

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=base
  CATEGORY:=Base system
  TITLE:=Support for DNS Relay
  URL:=http://en.sourceforge.jp/
  DEPENDS:=+libpthread
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  Package for DNS Relay Support 
endef

define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	config FEATURE_DNS_RELAY
		depends on PACKAGE_$(PKG_NAME)
		bool
		default 'y'
endef

#define Build/Prepare
#	$(call Build/Prepare/Default)
#endef

CONFIGURE_ARGS	+= IFX_CFLAGS="$(IFX_CFLAGS) -D__CYGWIN__" IFX_LDFLAGS="$(IFX_LDFLAGS)" --enable-master --prefix=/

define Build/Configure
	$(call Build/Configure/Default)
	# prevent autoheader invocation
	touch $(PKG_BUILD_DIR)/stamp-h.in
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) IFX_CFLAGS="$(IFX_CFLAGS)" 
		IFX_LDFLAGS="$(IFX_LDFLAGS)" 
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/dnrd $(1)/usr/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
