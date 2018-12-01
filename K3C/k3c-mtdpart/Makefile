#
# Openwrt feed Makefile for k3c
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=k3c-mtdpart
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk


CAL_IMAGE	:=calibration.bin
DEVINFO_IMAGE	:=dev_info.bin
PROINFO_IMAGE	:=pro_info.bin


define Package/$(PKG_NAME)
  SECTION:=firmware
  CATEGORY:=Lantiq
  TITLE:=k3c mtd part
  URL:=http://k3c.paldier.com/
  MAINTAINER:=paldier
  DEPENDS:=
  MENU:=1
endef

define Package/$(PKG_NAME)/description
	Lantiq GPHY Firmware
endef

define Build/Prepare
	cp -f ./files/* $(PKG_BUILD_DIR)
endef

define Build/Configure
endef


define Build/Compile
endef

define Package/$(PKG_NAME)/install
	mkdir -p $(TARGET_IMG_DIR)
	cp -f $(PKG_BUILD_DIR)/$(CAL_IMAGE) $(TARGET_IMG_DIR)/
	cp -f $(PKG_BUILD_DIR)/$(DEVINFO_IMAGE) $(TARGET_IMG_DIR)/
	cp -f $(PKG_BUILD_DIR)/$(PROINFO_IMAGE) $(TARGET_IMG_DIR)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
