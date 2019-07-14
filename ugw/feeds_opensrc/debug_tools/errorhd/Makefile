#
# Openwrt Makefile for errorhd
#
#

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ############################### 
PKG_NAME:=errorhd
PKG_VERSION:=1.0.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(shell pwd)/src

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Intel
  TITLE:=UGW Error Handler
  URL:=http://www.lantiq.com
  MAINTAINER:=Lantiq Beteiligungs-GmbH & Co. KG
  DEPENDS:= +libconfig +libpthread +PACKAGE_telemetrics:telemetrics
  MENU:=1
endef

define Package/$(PKG_NAME)/description
	UGW Error Handler Daemon
endef

### Menuconfig ##################################
define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
$(call Package/$(PKG_NAME)/config_debug_level)
endef

#### Exports for Source Code ####################
#export $(PKG_CONFIG_DEPENDS)

ifeq ($(CONFIG_PACKAGE_telemetrics),y)
TARGET_CFLAGS +=-DUTS_MODE
endif

#### Target Rules ###############################
define Build/Prepare
	$(call Build/Prepare/Default)
	$(if $(CONFIG_$(PKG_NAME)_USE_CUSTOM_SOURCE_DIR),,$(CP) -L ./src/* $(PKG_BUILD_DIR)/)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/errorhd.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/usr/sbin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/errorhd $(1)/$(VENDOR_PATH)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/errorhd_cmd $(1)/$(VENDOR_PATH)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/errorhd_test $(1)/$(VENDOR_PATH)/usr/sbin/
	$(INSTALL_BIN) ./files/errorhd.init $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/errorhd.cfg $(1)/opt/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg-$(BOARD); fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

