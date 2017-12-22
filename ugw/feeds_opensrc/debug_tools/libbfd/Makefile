#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=binutils
PKG_REV:=2.24.0-2014.09
PKG_VERSION:=$(PKG_REV)
PKG_SOURCE_URL:=https://releases.linaro.org/14.09/components/toolchain/binutils-linaro/
PKG_SOURCE:=$(PKG_NAME)-linaro-$(PKG_REV).tar.xz
PKG_MD5SUM:=8f9b2b2e049d59b1b86ce9657802a353
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-linaro-$(PKG_REV)

PATCH_DIR:=$(TOPDIR)/toolchain/binutils/patches/$(PKG_VERSION)

PKG_FIXUP:=libtool
PKG_LIBTOOL_PATHS:=. gas bfd opcodes gprof binutils ld libiberty gold intl
PKG_REMOVE_FILES:=libtool.m4
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

#define Package/binutils
 # SECTION:=devel
 # CATEGORY:=Development
 # TITLE:=binutils
 # DEPENDS:=+objdump
#endef

define Package/objdump
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=objdump
  DEPENDS:=+zlib 
endef

#define Package/binutils/description
#  The Binutils package contains a linker, an assembler, and other tools for handling object files
#endef

CONFIGURE_ARGS = \
		--target=$(REAL_GNU_TARGET_NAME) \
		--host=$(REAL_GNU_TARGET_NAME) \
		--build=$(GNU_HOST_NAME) \
		--with-sysroot=$(PKG_INSTALL_DIR) \
		--prefix=/usr \
		--disable-multilib \
		--disable-werror \
		--disable-nls \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		$(call qstrip,$(CONFIG_EXTRA_BINUTILS_CONFIG_OPTIONS))

TARGET_CFLAGS += $(FPIC)

define Build/Configure
	$(call Build/Configure/Default)
	$(call Build/Compile/Default, \
		configure-bfd \
		configure-binutils \
		configure-etc \
		configure-gas \
		configure-gprof \
		configure-intl \
		configure-ld \
		configure-libiberty \
		configure-opcodes \
	)
	$(MAKE) CFLAGS="$(TARGET_CFLAGS)"-C $(PKG_BUILD_DIR)/bfd/po Makefile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include}
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_BUILD_DIR)/include/*.h \
		$(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
	rm -f $(1)/usr/include/gdbm.h
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/bfd/doc/ CFLAGS="-I$(PKG_BUILD_DIR)/include" chew
	$(call Build/Compile/Default)
endef

define Package/objdump/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/$(REAL_GNU_TARGET_NAME)/bin/objdump $(1)/usr/bin/
endef

define Package/binutils/install
	$(INSTALL_DIR) $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ $(1)/usr/
	$(CP) $(PKG_INSTALL_DIR)/usr/$(REAL_GNU_TARGET_NAME) $(1)/usr
	rm -f $(1)/usr/bin/objdump
endef

#$(eval $(call BuildPackage,binutils))
$(eval $(call BuildPackage,objdump))
