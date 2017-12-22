#
# Copyright (C) 2015-2016 Intel Corporation
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dpdk
PKG_VERSION:=16.04
PKG_RELEASE:=1

# Disabling parallel build for easier debugging when experimenting.
PKG_BUILD_PARALLEL:=0 
#PKG_BUILD_PARALLEL:=1 

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://dpdk.org/browse/dpdk/snapshot/
PKG_MD5SUM:=b8c234a5d5669876291a2efce0523eae

PKG_MAINTAINER:=www.intel.com
PKG_LICENSE:=BSD LGPLv2 GPLv2
PKG_LICENSE_FILES:=LICENSE.GPL LICENSE.LGPL

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/image.mk

define Package/$(PKG_NAME)
	SECTION:=IntelBaseComponents
	CATEGORY:=Intel(R) GRX750 base components
	SUBMENU:=Packet Processing
	TITLE:=Intel(R) Data Plane Development Kit
	URL:=http://dpdk.org
	MAINTAINER:=Intel Corporation

	DEPENDS:=@TARGET_x86_puma +libpcap +python-subprocess +librt +libpthread +USE_UCLIBC:libubacktrace +USE_GLIBC:libbacktrace
endef

define Package/$(PKG_NAME)/description
	The Intel(r) Data Plane Development Kit (DPDK) is a set of libraries
	and drivers for fast packet processing.
endef

# TODO: Need to determine an approach to remove libubacktrace dependency.
define Package/libubacktrace
  SECTION:=libs
  CATEGORY:=Base system
  URL:=http://gcc.gnu.org/
  TITLE:=ubacktrace support library from GCC
endef

export RTE_TARGET=i686-native-linuxapp-gcc
export RTE_SDK=$(PKG_BUILD_DIR)

DEFCONFIG_RTE_TARGET_PATH:=$(PKG_BUILD_DIR)/config/defconfig_$(RTE_TARGET)

define Build/Prepare
	$(call Build/Prepare/Default)

	#clean dpdk sdk
	echo "dpdk= $(STAGING_DIR)/usr/src/dpdk/"
	# TODO: Add to "clean" target.
	rm -rf $(STAGING_DIR)/usr/src/dpdk/

	# Set target architechtore to Atom
	echo "CONFIG_RTE_MACHINE=\"atm\"">>$(DEFCONFIG_RTE_TARGET_PATH)

	# We don't support RAW sockets
	echo "CONFIG_RTE_LIBRTE_PMD_AF_PACKET=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# Dont care about these either
	echo "CONFIG_RTE_LIBRTE_PMD_BOND=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_PMD_NULL=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# Test PMD app depends on BOND PMD so disable it too
	echo "CONFIG_RTE_TEST_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# We don't need support of these
	echo "CONFIG_RTE_LIBRTE_EM_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_IGB_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_E1000_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# Or these
	echo "CONFIG_RTE_LIBRTE_IXGBE_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_I40E_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_FM10K_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_MLX4_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_ENIC_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_VIRTIO_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_VMXNET3_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_BNX2X_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_CXGBE_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_NFP_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_ENA_PMD=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	
	# We don't expose kernel space interface yet
	echo "CONFIG_RTE_LIBRTE_KNI=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_KNI_KMOD=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# No need for VHOST at this time
	echo "CONFIG_RTE_LIBRTE_VHOST=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# Disable CRYPTODEV
	echo "CONFIG_RTE_LIBRTE_CRYPTODEV=n">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_LIBRTE_PMD_NULL_CRYPTO=n">>$(DEFCONFIG_RTE_TARGET_PATH)

	# Need PMD PCAP for pre-SI 	
	echo "CONFIG_RTE_LIBRTE_PMD_PCAP=y">>$(DEFCONFIG_RTE_TARGET_PATH)
	echo "CONFIG_RTE_PORT_PCAP=y">>$(DEFCONFIG_RTE_TARGET_PATH)

	# We don't expect too many concurrent DPDK apps / threads
	# Smaller value helps with memory footprint (per-lcore structures)
	echo "CONFIG_RTE_MAX_LCORE=64">>$(DEFCONFIG_RTE_TARGET_PATH)

	# Need PMD PCAP for pre-SI 
	echo "CONFIG_RTE_LOG_HISTORY=128">>$(DEFCONFIG_RTE_TARGET_PATH)
endef

define Build/Configure
	$(call Build/Configure/Default)
	( cd $(RTE_SDK); make config T=$(RTE_TARGET) O=$(RTE_SDK)/$(RTE_TARGET) )

	# TODO: Check if this is still required.
	$(LN) -s $(TOPDIR)/build_dir/$(TOOLCHAIN_DIR_NAME)/uClibc-0.9.33.2/include/execinfo.h \
	 $(RTE_SDK)/$(RTE_TARGET)/include
endef

define Build/Compile
	( cd $(RTE_SDK)/$(RTE_TARGET); \
	make 'RTE_KERNELDIR=$(LINUX_DIR)' \
	'EXTRA_CLFAGS = "-D_GNU_SOURCE"' \
	'CROSS="$(TARGET_CROSS)"' )
endef

# TODO: Need to determine an approach to remove libubacktrace dependency.
define Package/libubacktrace/install
	$(INSTALL_DIR) $(1)/lib
	$(if $(CONFIG_EXTERNAL_TOOLCHAIN),
		$(INSTALL_BIN) $(call qstrip,$(CONFIG_LIBC_ROOT_DIR))/lib/libubacktrace.so* $(1)/lib/
	,
		$(INSTALL_BIN) $(TOOLCHAIN_DIR)/lib/libubacktrace.so* $(1)/lib/
	)
endef

define Package/$(PKG_NAME)/install
	#
	# Create a copy of dpdk src and compiled binaries/libaries so
	# that components dependent on DPDK can be successfully compiled.
	#
	$(INSTALL_DIR) $(STAGING_DIR)/usr/src/dpdk/
	(cd ${RTE_SDK}; \
	find . -depth -not -path "./.*" -print0 | \
	cpio --null --preserve-modification --pass-through --make-directories --unconditional \
	$(STAGING_DIR)/usr/src/dpdk)

	#TODO: need to be installed to avoid missing dependency issue. need to fix it
	$(INSTALL_DIR) $1/lib
	$(if $(CONFIG_EXTERNAL_TOOLCHAIN),
		-$(INSTALL_BIN) $(call qstrip,$(CONFIG_LIBC_ROOT_DIR))/lib/libubacktrace.so* $(1)/lib/
	,
		-$(INSTALL_BIN) $(TOOLCHAIN_DIR)/lib/libubacktrace.so* $(1)/lib/
	)
endef

# TODO: Need to determine an approach to remove libubacktrace dependency.
$(eval $(call BuildPackage,libubacktrace))
$(eval $(call BuildPackage,$(PKG_NAME)))

