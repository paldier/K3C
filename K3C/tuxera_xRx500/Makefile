#
# Openwrt feed for tuxera-ntfs
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=tuxera-file-systems
PKG_VERSION:=3016.7-grx350_merlin
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://tuxera.com/download/lantiq/
PKG_MD5SUM:=257b648accc71ccc57a23352987c8858

TUXERA_FS:=ntfs fat hfsplus
ntfs_tools:=mkntfs ntfsck ntfsdebug ntfsinfo ntfslabel
fat_tools:=fatdebug fatdump fatfsck fatinfo fatlabel mkfatfs
hfsplus_tools:=fsck_hfs newfs_hfs thfslabel

PKG_CONFIG_DEPENDS:=\
	$(foreach tfs,$(TUXERA_FS),CONFIG_PACKAGE_tuxera_xRx500-$(tfs) \
	$(foreach tops,$($(tfs)_tools),CONFIG_PACKAGE_tuxera_xRx500-$(tfs)_TOOL_$(shell echo $(tops)|tr a-z A-Z)))

include $(INCLUDE_DIR)/package.mk

define config_gen
  config PACKAGE_tuxera_xRx500-$(2)_TOOL_$(shell echo $(1)|tr a-z A-Z)
	depends on PACKAGE_tuxera_xRx500-$(2)
	bool '$(1)'

endef

define tuxeraPkg
define Package/tuxera_xRx500-$(1)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=Tuxera $(1) for Lantiq XRX500 platforms
  URL:=http://www.tuxera.com
  CATEGORY:=Utilities
  DEPENDS:=+kmod-nls-utf8 +kmod-nls-cp437 +libpthread @TARGET_lantiq_xrx500
  MENU:=1
endef

define Package/tuxera_xRx500-$(1)/description
  This package contains pre-compiled binaries and tools of Tuxera $(1)
  tuned for Lantiq XRX500 platforms.
  Supported tools: $(subst $(space),$(strip ,) ,$($(1)_tools))
endef

define Package/tuxera_xRx500-$(1)/config
  $(foreach file,$($(1)_tools),$(call config_gen,$(file),$(1)))
endef

define Package/tuxera_xRx500-$(1)/install
	$(INSTALL_DIR) $$(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1)/kernel-module/*.ko $$(1)/lib/modules/$(LINUX_VERSION)/

	$(INSTALL_DIR) $$(1)/usr/sbin
	$(foreach tbin, $($(1)_tools), \
		$(if $(CONFIG_PACKAGE_tuxera_xRx500-$(1)_TOOL_$(shell echo $(tbin)|tr a-z A-Z)), \
			$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1)/tools/$(tbin) $$(1)/usr/sbin/;) \
	)
endef
endef

define Build/Compile
	echo -n;
endef

$(foreach tfile,$(TUXERA_FS),$(eval $(call tuxeraPkg,$(tfile))))
$(foreach tfile,$(TUXERA_FS),$(eval $(call BuildPackage,tuxera_xRx500-$(tfile))))
