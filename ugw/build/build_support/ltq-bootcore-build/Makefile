#
# Openwrt Makefile for ltq-dect
#
#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ###############################
PKG_NAME:=ltq-xrx500-bootcore
PKG_VERSION:=0.0.0
PKG_RELEASE:=1
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(BUILD_DIR_BASE)/xRX500_BootCore
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Lantiq
	SUBMENU:=Enable xRX500 BootCore build
	TITLE:=Lantiq xRX500 BootCore environment build
	URL:=http://www.lantiq.com
	MAINTAINER:=Lantiq
	MENU:=1
	Hooks/Prepare/Pre := PackagePreReq 
endef

define Package/$(PKG_NAME)/description
	This package is for building xRX500 BootCore model.
endef


BOOTCORE_SUFFIX:=$(shell echo $(CONFIG_XRX500_BOOTCORE_MODEL) |tr '[:upper:]' '[:lower:]')

### Menuconfig ##################################
define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	menu "xRX500 Boot Core features"
		depends on PACKAGE_$(PKG_NAME)
		source "$(SOURCE)/config/Config.in"
	endmenu
endef


#### Target Rules ###############################
define PackagePreReq
	sh ltq_bootcore_env_prepare.sh $(TOPDIR) $(TOOLCHAIN_DIR) $(PKG_BUILD_DIR)
endef

define Build/Clean
	@rm -rf $(PKG_DEFAULT_CUSTOM_SOURCE_DIR) 
	@rm -rf $(PKG_BUILD_DIR)
	@rm -rf $(BUILD_DIR_BASE)/*$(BOOTCORE_SUFFIX)
	@rm -rf $(TOPDIR)/staging_dir/*$(BOOTCORE_SUFFIX)
endef

define Build/Compile
	sh ltq_bootcore_env_prepare.sh $(TOPDIR) $(TOOLCHAIN_DIR) $(PKG_BUILD_DIR)
	$(call Build/Compile/Default)
	rm -rf $(PKG_BUILD_DIR)/.configured*
endef

define Package/$(PKG_NAME)/install
	mkdir -p $(TARGET_IMG_DIR)
	cp -f $(PKG_DEFAULT_CUSTOM_SOURCE_DIR)/bin/$(BOARD)/$(BOOTCORE_SUFFIX)/uImage $(TARGET_IMG_DIR)/uImage_bootcore
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
