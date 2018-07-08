#
# Openwrt Makefile for ltq-wlan-wave_5_x
####################################

# Includes
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk


KERNEL_PKG_NAME:=lantiq-wlan-wave-support_5_x
PKG_NAME:=ltq-wlan-wave_5_x
LINUX_ENV_NAME:=ugw-linux310
PKG_BASE_NAME:=ltq-wave300
PKG_BUILD_DEPENDS_EXTRA:=kmod-lantiq-cpufreq
DEPENDS_EXTRA:=@+PACKAGE_iw \
		+libnl3 +libpthread +librt

# Package Info
PKG_VERSION:=05.04.00.49
DRV_VERSION:=05.04.00.00.31.e4a866ab51e9
FW_VERSION:=FW_5.4.0_r13858
TX_SENDER_VERSION:=TxSender_2.1_r2165
HOST_INTERFACE_VERSION:=HostInterface_2.1_r2174
RX_HANDLER_VERSION:=RxHandler_2.1_r2223

TX_SENDER_GEN5B_VERSION:=TxSender_3.4_r3053
HOST_INTERFACE_GEN5B_VERSION:=HostInterface_3.3_r2947
RX_HANDLER_GEN5B_VERSION:=RxHandler_3.4_r2992

PROGMODEL_VERSION_AR10:=2856
PROGMODEL_VERSION_WAVE500:=2721
PROGMODEL_VERSION_GEN5B:=2870

FPGA_SIM_VERSION:=wave500_mac_npu_2_2_26_3_G5_G5_6MHz_async
WAVE_PSD_VER:=TODO_UPDATE_VER_STRING
PKG_RELEASE:=1
PKG_SOURCE_VERSION:=$(DRV_VERSION).gpl.wls.src
PKG_SOURCE_NAME:=$(DRV_VERSION).wls.src


DRV_RFLIB_PKG_NAME:=$(PKG_NAME)-rflib
DRV_RFLIB_PKG_VERSION:=05.04.00.49
DRV_RFLIB_PKG_SOURCE:=lq-wave-300-$(DRV_VERSION).rflib.wls.src.tar.bz2
DRV_RFLIB_PKG_BIN:=lq-wave-300-$(DRV_VERSION).rflib.wls.bin.tar.bz2
KERNEL_PKG_NAME_BCKPRT_SPPRT:=$(KERNEL_PKG_NAME)_backport_support

BINARY_PKG:=$(if $(wildcard $(TOPDIR)/dl/$(DRV_RFLIB_PKG_BIN)),y)
UGW_MODEL_NAME:=$(shell echo $(CONFIG_BUILD_SUFFIX)|tr a-z A-Z)

ifndef CONFIG_$(PKG_NAME)_USE_CUSTOM_SOURCE_DIR
  PKG_SOURCE_DIR:=lq-wave-300-$(PKG_SOURCE_NAME)
  PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_BASE_NAME)-$(PKG_VERSION)
  PKG_FULL_DIR:=$(PKG_BUILD_DIR)/$(PKG_SOURCE_DIR)
else
  PKG_FULL_DIR:=$(CONFIG_lq-wave-300_5_x_CUSTOM_SOURCE_DIR)
endif

PKG_BUILD_DEPENDS:=libnl3 $(DRV_RFLIB_PKG_NAME) $(PKG_BUILD_DEPENDS_EXTRA)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(shell pwd)/src
PKG_NO_AUTORECONF:=y

COPY_SCRIPTS:=
SCRIPTS_FOLDER:=opt/lantiq/wave/scripts
BIN_FOLDER:=opt/lantiq/bin
BINS_FOLDER:=opt/lantiq/wave/images
IMAGES_FOLDER:=files/root/mtlk/images
DRIVER_FOLDER:=opt/lantiq/lib/modules/$(LINUX_VERSION)/net
SBIN_FOLDER:=opt/lantiq/sbin


ifeq ($(CONFIG_FEATURE_LTQ_WAVE_500),y)
 FW_FOLDER:=real_phy_wrx500
 COPY_WAVE500:=y
 WAVE_CFG_SFX:=-vbg500
 ifeq ($(CONFIG_FEATURE_LTQ_WAVE_FGPA_DUMMY_PHY),y)
  FW_FOLDER:=dummy_phy
 endif
endif

# AR10 must be before WAVE500_PCIE, because the last will override what driver config file to use
# and wave500 supports both AR10 and Wave500 PCIE
ifeq ($(CONFIG_FEATURE_LTQ_WAVE_AR10_SUPPORT),y)
 COPY_AR10:=y
 WAVE_CFG_SFX:=-wave400
endif
ifeq ($(CONFIG_FEATURE_LTQ_WAVE_STA),y)
 COPY_STA:=y
endif

# TODO: Need to revisit the FW_FOLDER parameter when driver supports both.
# e.g. what to do when GRX350 is wrx300 and Wave500 is wrx500
ifeq ($(CONFIG_FEATURE_LTQ_WAVE_500_PCIE),y)
 COPY_WAVE500_PCIE:=y
 WAVE_CFG_SFX:=-wave500
 ifeq ($(CONFIG_TARGET_x86),y)
  WAVE_CFG_SFX:=-wave500-x86
  LINUX_ENV_NAME:=ugw-x86-linux312
 endif
 ifeq ($(COPY_AR10),y)
  WAVE_CFG_SFX:=-wave400_wave500
  ifeq ($(CONFIG_TARGET_lantiq_xrx330_EASY300_AC1200),y)
   WAVE_CFG_SFX:=-wave400_wave500_ac1200
  endif
  ifeq ($(CONFIG_TARGET_lantiq_xrx330_EASY330_BOND),y)
   WAVE_CFG_SFX:=-wave400_wave500_ac1200
  endif
 endif
endif

ifeq ($(CONFIG_PACKAGE_kmod-mac80211),y)
 WAVE_CFG_SFX:=-wave400_wave500_backport
endif

ifeq ($(CONFIG_FEATURE_LTQ_WAVE_500B),y)
 COPY_WAVE500B:=y
endif

ifneq ($(CONFIG_FEATURE_LTQ_WAVE_DEBUG_TFTP_BINS),y)
 REMOVE_TFTP_BINS_SCRIPT:=y
endif

ifeq ($(COPY_SCRIPTS),y)
 WAVE_SCRIPTS_VER:=$(shell hg summ > /dev/null 2>&1 && hg summ | head -n1 | awk '{print $$2}')
endif

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
		+@!NEW_FRAMEWORK:FEATURE_WIRELESS \
		+@!NEW_FRAMEWORK:FEATURE_WIRELESS_WAVE300 \
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
		@+PACKAGE_crda
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

### Menuconfig #############################
define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
      menu "$(PKG_NAME) features"
      depends on PACKAGE_$(PKG_NAME)
      source "$(SOURCE)/config/Config.in"
      endmenu
endef


#### Target Rules ##########################

define Build/Prepare
	$(call Build/Prepare/Default)
	#$(call Prepare/$(DRV_RFLIB_PKG_NAME))
	$(if $(CONFIG_$(PKG_NAME)_USE_CUSTOM_SOURCE_DIR),,$(CP) -L ./src/* $(PKG_BUILD_DIR)/)
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

	# if the /aligned/ exist we are in mixed mode 
	# the image directory is flat and we copy all to $(BINS_FOLDER)

ifneq (,$(wildcard ./$(IMAGES_FOLDER)/aligned/.))
	 $(CP) ./$(IMAGES_FOLDER)/aligned/* $(1)/$(BINS_FOLDER)/
else
	 @echo "aligned dir not exist->legacy mode only"
endif

	# if bin_files.list.mod not exist copy PSD.bin else echo

ifeq ($(wildcard ./$(IMAGES_FOLDER)/PSD/bin_files.list.mod),)
	 $(CP) ./$(IMAGES_FOLDER)/PSD/PSD.bin $(1)/$(BINS_FOLDER)/
	 #$(CP) ./$(IMAGES_FOLDER)/PSD/PSD.bin.bak $(1)/$(BINS_FOLDER)/
else
	@echo "PSD->bin_files-list-mod exist handled in auto mode"
endif

	$(LN) /dev/null $(1)/etc/udev/rules.d/75-persistent-net-generator.rules

	@echo "******* COPY_WAVE500B (y?)-> $(COPY_WAVE500B) ***********"

ifeq ($(COPY_WAVE500B),y)
	$(CP) ./$(IMAGES_FOLDER)/cal/cal_wlan0.bin $(1)/$(BINS_FOLDER)/cal_wlan0.bin
	$(CP) ./$(IMAGES_FOLDER)/cal/cal_wlan1.bin $(1)/$(BINS_FOLDER)/cal_wlan1.bin
	$(CP) ./$(IMAGES_FOLDER)/cal/cal_wlan2.bin $(1)/$(BINS_FOLDER)/cal_wlan2.bin
 ifeq ($(wildcard ./$(IMAGES_FOLDER)/progmodels/bin_files.list.mod),)
	$(CP) ./$(IMAGES_FOLDER)/progmodels/wave500/CHIP/* $(1)/$(BINS_FOLDER)/
	$(CP) ./$(IMAGES_FOLDER)/progmodels/wave500b/FPGA/* $(1)/$(BINS_FOLDER)/
	$(CP) ./$(IMAGES_FOLDER)/progmodels/wave500b/CHIP/* $(1)/$(BINS_FOLDER)/
	$(CP) ./$(IMAGES_FOLDER)/progmodels/wave500/FPGA/* $(1)/$(BINS_FOLDER)/
	$(CP) ./$(IMAGES_FOLDER)/progmodels/ar10/* $(1)/$(BINS_FOLDER)/
	@echo "COPY_WAVE500B->progmodels->wave500b in legacy mode"
 else
	@echo "COPY_WAVE500B->progmodels handled in auto mode"
 endif

else
	@echo "COPY_WAVE500B->N"
endif


	@echo "******* COPY images completed ***********"

	$(CP) ./src/mtlk.ko $(1)/$(DRIVER_FOLDER)/
	$(CP) ./src/mtlkroot.ko $(1)/$(DRIVER_FOLDER)/
	chmod 555 $(1)/$(DRIVER_FOLDER)/mtlk.ko
	chmod 555 $(1)/$(DRIVER_FOLDER)/mtlkroot.ko

	$(CP) ./$(IMAGES_FOLDER)/fw/fw_scd_file.scd $(1)/$(BIN_FOLDER)/ || echo "Skipping fw_scd_file.scd install"
	$(CP) ./src/*.mtidlc $(1)/$(BIN_FOLDER)/	|| echo "Skipping mtidlc from driver folder install"
	$(CP) ./src/drvhlpr $(1)/$(BIN_FOLDER)/	|| echo "Skipping drvhlpr install"
	$(CP) ./src/mtdump $(1)/$(BIN_FOLDER)/ || echo "Skipping mtdump install"
	$(CP) ./src/mtlk_cli $(1)/$(BIN_FOLDER)/ || echo "Skipping mtlk_cli install"
	$(CP) ./src/BclSockServer $(1)/$(BIN_FOLDER)/ || echo "Skipping BclSockServer install"
	$(CP) ./src/logserver $(1)/$(BIN_FOLDER)/ || echo "Skipping logserver install"
	$(CP) ./src/dutserver $(1)/$(BIN_FOLDER)/ || echo "Skipping Dutserver install"
	./files/config/mtidlc-generator.sh $(1)/$(BIN_FOLDER)/ || echo "Skipping generation of minimized mtidlc files"

	chmod 555 $(1)/$(BIN_FOLDER)/drvhlpr || true
	chmod 555 $(1)/$(BIN_FOLDER)/*.mtidlc* || true
	chmod 555 $(1)/$(BIN_FOLDER)/mtdump || true
	chmod 555 $(1)/$(BIN_FOLDER)/mtlk_cli || true
	chmod 555 $(1)/$(BIN_FOLDER)/BclSockServer || true
	chmod 555 $(1)/$(BIN_FOLDER)/logserver || true
	chmod 555 $(1)/$(BIN_FOLDER)/dutserver || true
	$(CP) ./files/binaries/sbin/* $(1)/$(SBIN_FOLDER)/
	$(CP) ./files/etc/* $(1)/etc/
	echo -e "wave_release_minor=\"$(PKG_VERSION)\"\nwave_mac_ver=\"$(FW_VERSION)\"\nwave_psd_ver=\"$(WAVE_PSD_VER)\"\nwave_driver_ver=\"$(DRV_VERSION)\"" > $(1)/etc/wave_components.ver
	$(if $(COPY_SCRIPTS),
		echo -e "wave_scripts_ver=\"$(WAVE_SCRIPTS_VER)\"" >> $(1)/etc/wave_components.ver)
	$(if $(COPY_WAVE500),
		echo -e "wave_fpga_sim_ver=\"$(FPGA_SIM_VERSION)\"\nwave500_progmodel_ver=\"$(PROGMODEL_VERSION_WAVE500)\"\nwave_tx_sender_ver=\"$(TX_SENDER_VERSION)\"\nwave_host_interface_ver=\"$(HOST_INTERFACE_VERSION)\"\nwave_rx_handler_ver=\"$(RX_HANDLER_VERSION)\"" >> $(1)/etc/wave_components.ver)
	$(if $(COPY_WAVE500_PCIE),
		echo -e "wave500_progmodel_ver=\"$(PROGMODEL_VERSION_WAVE500)\"\nwave_tx_sender_ver=\"$(TX_SENDER_VERSION)\"\nwave_host_interface_ver=\"$(HOST_INTERFACE_VERSION)\"\nwave_rx_handler_ver=\"$(RX_HANDLER_VERSION)\"" >> $(1)/etc/wave_components.ver)
	$(if $(COPY_WAVE500B),
		echo -e "wave500B_progmodel_ver=\"$(PROGMODEL_VERSION_GEN5B)\"\nwave_tx_sender_gen5b_ver=\"$(TX_SENDER_GEN5B_VERSION)\"\nwave_host_interface_gen5b_ver=\"$(HOST_INTERFACE_GEN5B_VERSION)\"\nwave_rx_handler_gen5b_ver=\"$(RX_HANDLER_GEN5B_VERSION)\"" >> $(1)/etc/wave_components.ver)
	$(if $(COPY_AR10),
		echo -e "wave_ar10_progmodel_ver=\"$(PROGMODEL_VERSION_AR10)\"" >> $(1)/etc/wave_components.ver)
endef

$(eval $(call BuildPackage,$(DRV_RFLIB_PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call KernelPackage,$(KERNEL_PKG_NAME)))
$(eval $(call KernelPackage,$(KERNEL_PKG_NAME_BCKPRT_SPPRT)))
