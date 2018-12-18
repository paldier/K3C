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

WAVE_TARGET:=k3c
ifeq ($(CONFIG_TARGET_lantiq_xrx500_k3c),y)
 WAVE_TARGET:=k3c
endif
ifeq ($(CONFIG_TARGET_lantiq_xrx500_k3cv2),y)
#k3c for B1/A1/B1G,k3cv2 for B2
 WAVE_TARGET:=k3cv2
endif

ifeq ($(CONFIG_TARGET_lantiq_xrx500_bluecave),y)
 WAVE_TARGET:=bluecave
endif
ifeq ($(CONFIG_TARGET_lantiq_xrx500_easy350_anywan_800m_owrt),y)
 WAVE_TARGET:=k3c
endif

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
	cp -f $(PKG_BUILD_DIR)/$WAVE_TARGET/$(CAL_IMAGE) $(TARGET_IMG_DIR)/
	cp -f $(PKG_BUILD_DIR)/$WAVE_TARGET/$(DEVINFO_IMAGE) $(TARGET_IMG_DIR)/
	cp -f $(PKG_BUILD_DIR)/$WAVE_TARGET/$(PROINFO_IMAGE) $(TARGET_IMG_DIR)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
