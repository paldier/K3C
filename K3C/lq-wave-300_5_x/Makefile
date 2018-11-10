#
# Openwrt Makefile for ltq-wlan-wave_5_x
####################################

# Includes
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

KERNEL_PKG_NAME:=lantiq-wlan-wave-support_5_x
PKG_NAME:=ltq-wlan-wave_5_x
PKG_BASE_NAME:=ltq-wave300
PKG_BUILD_DEPENDS_EXTRA:=kmod-lantiq-cpufreq
DEPENDS_EXTRA:=@+PACKAGE_iw \
		+libnl3 +libpthread +librt

# Package Info
PKG_VERSION:=05.04.00.49
PKG_RELEASE:=1
PKG_SOURCE_NAME:=$(PKG_VERSION).wls.src

DRV_RFLIB_PKG_NAME:=$(PKG_NAME)-rflib
KERNEL_PKG_NAME_BCKPRT_SPPRT:=$(KERNEL_PKG_NAME)_backport_support

PKG_SOURCE_DIR:=lq-wave-300-$(PKG_SOURCE_NAME)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_BASE_NAME)-$(PKG_VERSION)
PKG_FULL_DIR:=$(PKG_BUILD_DIR)/$(PKG_SOURCE_DIR)

PKG_BUILD_DEPENDS:=libnl3 $(DRV_RFLIB_PKG_NAME) $(PKG_BUILD_DEPENDS_EXTRA)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(shell pwd)/src
PKG_NO_AUTORECONF:=y

BIN_FOLDER:=opt/lantiq/bin
BINS_FOLDER:=opt/lantiq/wave/images
IMAGES_FOLDER:=files/root/mtlk/images
DRIVER_FOLDER:=opt/lantiq/lib/modules/$(LINUX_VERSION)/net
SBIN_FOLDER:=opt/lantiq/sbin

PKG_CONFIG_DEPENDS:=CONFIG_PACKAGE_kmod-lantiq-cpufreq
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk


define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Lantiq
	TITLE:=Wave scripts/binaries
	DEPENDS:= \
		+PACKAGE_kmod-mac80211:kmod-$(KERNEL_PKG_NAME_BCKPRT_SPPRT) \
		+!PACKAGE_kmod-mac80211:kmod-$(KERNEL_PKG_NAME) \
		+$(DRV_RFLIB_PKG_NAME) \
		+PACKAGE_kmod-ltq_wave500_dp_drv:kmod-ltq_wave500_dp_drv \
		+PACKAGE_kmod-lantiq_directconnect_support:kmod-lantiq_directconnect_support \
		@+PACKAGE_ltq-devm:FEATURE_DEVM_WIFILAN \
		$(DEPENDS_EXTRA) \
		@+BUSYBOX_CONFIG_WC \
		@+BUSYBOX_CONFIG_WHICH \
		@+BUSYBOX_CONFIG_AWK \
		@+BUSYBOX_CONFIG_FEATURE_AWK_MATH \
		@+BUSYBOX_CONFIG_FIND \
		@+BUSYBOX_CONFIG_FEATURE_FIND_MTIME \
		@+BUSYBOX_CONFIG_FEATURE_FIND_PERM \
		@+BUSYBOX_CONFIG_FEATURE_FIND_TYPE \
		@+BUSYBOX_CONFIG_FEATURE_FIND_XDEV \
		@+BUSYBOX_CONFIG_FEATURE_FIND_NEWER \
		@+BUSYBOX_CONFIG_FEATURE_FIND_INUM \
		@+BUSYBOX_CONFIG_XARGS \
		@+BUSYBOX_CONFIG_LSMOD \
		@+PACKAGE_libnl3 \
		@+PACKAGE_libopenssl \
		@+PACKAGE_libuuid \
		@+PACKAGE_uuidgen \
		@+PACKAGE_hostapd_2_3_wave_5_1_1 \
		@+PACKAGE_udev \
		@+PACKAGE_crda \
		@+PACKAGE_wav500-images
	URL:=http://www.lantiq.com
	MAINTAINER:=Lantiq Technologies
	MENU:=1
endef

define Package/$(PKG_NAME)/description
 This package contains wave scripts/binaries
endef

define Package/$(DRV_RFLIB_PKG_NAME)
$(call Package/$(PKG_NAME)/Default)
 SECTION:=utils
 CATEGORY:=Lantiq
 TITLE:=RFLIB package for Wave400/Wave500 driver
endef

define Package/$(DRV_RFLIB_PKG_NAME)/description
 This package contains the wave RF library
endef

define KernelPackage/$(KERNEL_PKG_NAME)
 SUBMENU:=Lantiq
 TITLE:=Lantiq Support for Wave WLAN
 DEPENDS:=@TARGET_ltqcpe||TARGET_lantiq||TARGET_x86

 KCONFIG:= \
 CONFIG_CRYPTO_SHA256=y \
 CONFIG_FW_LOADER=y \
 CONFIG_HOTPLUG=y \
 CONFIG_WIRELESS=y \
 CONFIG_CFG80211=y \
 CONFIG_CFG80211_WEXT=y \
 CONFIG_MAC80211=y
endef


define KernelPackage/$(KERNEL_PKG_NAME)/description
  Kernel Support for Wave WLAN
endef

define KernelPackage/$(KERNEL_PKG_NAME_BCKPRT_SPPRT)
 SUBMENU:=Lantiq
 TITLE:=Lantiq Support for Wave WLAN
 DEPENDS:=@TARGET_ltqcpe||TARGET_lantiq||TARGET_x86

 KCONFIG:= \
 CONFIG_CRYPTO_SHA256=y \
 CONFIG_FW_LOADER=y \
 CONFIG_HOTPLUG=y \
 CONFIG_WIRELESS=y \
 CONFIG_CFG80211_WEXT=y
endef

define KernelPackage/$(KERNEL_PKG_NAME_BCKPRT_SPPRT)/description
  Kernel Support for Wave WLAN
endef


#### Exports ###############################
export THISDIR = $(shell pwd)
export TARGET_CROSS

#### Target Rules ##########################

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -L ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) -p $(1)/$(DRIVER_FOLDER)/
	$(INSTALL_DIR) -p $(1)/$(BIN_FOLDER)/
	$(INSTALL_DIR) -p $(1)/$(SBIN_FOLDER)/
	$(INSTALL_DIR) -p $(1)/$(BINS_FOLDER)/
	$(INSTALL_DIR) -p $(1)/etc/hotplug/
	$(INSTALL_DIR) -p $(1)/etc/hotplug.d/
	$(INSTALL_DIR) -p $(1)/etc/udev/rules.d/

	$(LN) /dev/null $(1)/etc/udev/rules.d/75-persistent-net-generator.rules

	$(CP) ./src/mtlk.ko $(1)/$(DRIVER_FOLDER)/
	$(CP) ./src/mtlkroot.ko $(1)/$(DRIVER_FOLDER)/
	chmod 555 $(1)/$(DRIVER_FOLDER)/mtlk.ko
	chmod 555 $(1)/$(DRIVER_FOLDER)/mtlkroot.ko

	$(CP) ./src/*.mtidlc $(1)/$(BIN_FOLDER)/	|| echo "Skipping mtidlc from driver folder install"
	$(CP) ./src/drvhlpr $(1)/$(BIN_FOLDER)/	|| echo "Skipping drvhlpr install"
	$(CP) ./src/mtdump $(1)/$(BIN_FOLDER)/ || echo "Skipping mtdump install"
	$(CP) ./src/mtlk_cli $(1)/$(BIN_FOLDER)/ || echo "Skipping mtlk_cli install"
	$(CP) ./src/BclSockServer $(1)/$(BIN_FOLDER)/ || echo "Skipping BclSockServer install"
	$(CP) ./src/logserver $(1)/$(BIN_FOLDER)/ || echo "Skipping logserver install"
	$(CP) ./src/dutserver $(1)/$(BIN_FOLDER)/ || echo "Skipping Dutserver install"

	chmod 555 $(1)/$(BIN_FOLDER)/drvhlpr || true
	chmod 555 $(1)/$(BIN_FOLDER)/*.mtidlc* || true
	chmod 555 $(1)/$(BIN_FOLDER)/mtdump || true
	chmod 555 $(1)/$(BIN_FOLDER)/mtlk_cli || true
	chmod 555 $(1)/$(BIN_FOLDER)/BclSockServer || true
	chmod 555 $(1)/$(BIN_FOLDER)/logserver || true
	chmod 555 $(1)/$(BIN_FOLDER)/dutserver || true
	$(CP) ./files/etc/* $(1)/etc/
	$(CP) ./files/binaries/sbin/hotplug $(1)/$(SBIN_FOLDER)/
endef

$(eval $(call BuildPackage,$(DRV_RFLIB_PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call KernelPackage,$(KERNEL_PKG_NAME)))
$(eval $(call KernelPackage,$(KERNEL_PKG_NAME_BCKPRT_SPPRT)))
