#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=oprofile
PKG_VERSION:=0.9.4
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/oprofile/
PKG_MD5SUM:=82b059379895cf125261d7d773465915

PKG_BUILD_DEPENDS:=binutils
PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define KernelPackage/oprofile
  SUBMENU:=Other modules
  TITLE:=OProfile profiling support
  KCONFIG:=CONFIG_OPROFILE @!LINUX_2_6
  FILES:=$(LINUX_DIR)/arch/$(LINUX_KARCH)/oprofile/oprofile.$(LINUX_KMOD_SUFFIX)
  DEPENDS:=@KERNEL_PROFILING
endef

define KernelPackage/oprofile/description
  Kernel module for support for oprofile system profiling.
endef

define Package/oprofile
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=OProfile System Profiler
  URL:=http://oprofile.sourceforge.net
  DEPENDS:=+libpopt +kmod-oprofile +libstdcpp +objdump @+BUSYBOX_CONFIG_DIRNAME @+BUSYBOX_CONFIG_ID @+BUSYBOX_CONFIG_SEQ
endef

define Package/oprofile/description
 A transparent low-overhead system-wide profiler.
endef

define Package/oprofile-utils
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=OProfile System Profiler (extra utilities)
  URL:=http://oprofile.sourceforge.net
  DEPENDS:=oprofile
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--with-kernel-support \
		--without-x \
	)
endef

TARGET_LDFLAGS := -L$(STAGING_DIR)/usr/lib $(TARGET_LDFLAGS)

define Package/oprofile-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/opannotate \
		$(PKG_INSTALL_DIR)/usr/bin/oparchive \
		$(PKG_INSTALL_DIR)/usr/bin/opgprof \
		$(1)/usr/bin
endef

define Package/oprofile/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(1)/usr/bin

	rm -f \
		$(1)/usr/bin/opannotate \
		$(1)/usr/bin/oparchive \
		$(1)/usr/bin/opgprof

	$(INSTALL_DIR) $(1)/usr/lib/oprofile
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/oprofile/*.so* \
		$(1)/usr/lib/oprofile/

	$(INSTALL_DIR) $(1)/usr/share/oprofile
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/oprofile/stl.pat \
		$(PKG_INSTALL_DIR)/usr/share/oprofile/$(LINUX_KARCH) \
		$(1)/usr/share/oprofile/

	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) ./files/load_oprofile_module.sh $(1)/etc/init.d/
	$(CP) $(STAGING_DIR)/usr/lib/libopcodes-2.19.1.so  $(1)/usr/lib/
endef

$(eval $(call BuildPackage,oprofile))
$(eval $(call BuildPackage,oprofile-utils))
$(eval $(call KernelPackage,oprofile))
