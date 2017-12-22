#
# Copyright (C) 2015-2016 Intel Corporation
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hyperscan
PKG_VERSION:=4.0.0
PKG_RELEASE:=1

# Disabling parallel build for easier debugging when experimenting.
PKG_BUILD_PARALLEL:=0
#PKG_BUILD_PARALLEL:=1


PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/01org/hyperscan.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_VERSION:=0e5c4cbd1d6fde77a24ad00b1c1fa7925b39bbec 
PKG_MAINTAINER:=www.intel.com
PKG_LICENSE:=BSD

# If needed, apply OpenWRT workarounds for autotools issues.
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk
include $(INCLUDE_DIR)/target.mk

define Package/$(PKG_NAME)
	SECTION:=IntelBaseComponents
	CATEGORY:=Network
	TITLE:=Hyperscan high-performance regex matching library
	URL:=https://01.org/hyperscan
	MAINTAINER:=Intel Corporation

	DEPENDS:=@TARGET_x86_puma
	DEPENDS:=+boost +ragel +python-subprocess +libpcap +libstdc++
endef

define Package/$(PKG_NAME)/description
	Hyperscan is a software regular expression matching engine designed 
	with high performance and flexibility in mind.  It is implemented as 
	a library that exposes a straightforward C API.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	ln -s $(BUILD_DIR)/boost_1_58_0/boost $(PKG_BUILD_DIR)/include/boost 
endef

#define Build/Configure
#	$(call Build/Configure/Default)
#endef

#For fixing __m128i unknown type name issue
NEED_CFLAGS:= -msse -msse2 -msse3 -mssse3 -m32 -O2 -march=core2 -mtune=core2 -feliminate-unused-debug-types -fno-asynchronous-unwind-tables
TARGET_CFLAGS+= $(NEED_CFLAGS)
TARGET_CPPFLAGS+= $(NEED_CFLAGS)
TARGET_LDFLAGS+= -L$(STAGING_DIR)/user/lib 

#define Build/Compile
#	$(call Build/Compile/Default)
#endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/bin/hyperscan
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/pcapscan $1/$(VENDOR_PATH)/bin/hyperscan
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/patbench $1/$(VENDOR_PATH)/bin/hyperscan
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/simplegrep $1/$(VENDOR_PATH)/bin/hyperscan
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/unit-hyperscan $1/$(VENDOR_PATH)/bin/hyperscan
	$(INSTALL_DIR) $1/bin
	$(LN) -sf ..$(VENDOR_PATH)/bin/hyperscan $(1)/bin/hyperscan

	#TODO: need to be installed to avoid missing dependency issue. need to fix it
	$(INSTALL_DIR) $1/lib
	$(if $(CONFIG_EXTERNAL_TOOLCHAIN),
		$(INSTALL_BIN) $(call qstrip,$(CONFIG_LIBC_ROOT_DIR))/lib/libstdc++.so.6 $(1)/lib/
	,
		$(INSTALL_BIN) $(TOOLCHAIN_DIR)/lib/libstdc++.so.6 $(1)/lib/
	)

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
