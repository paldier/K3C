#
# Written by Zenon Fortuna, zenon@mips.com, zenon@fortuna.org
# March 21, 2011
# Updated by Lantiq Communications 11, Feb 2013
#

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ###############################
PKG_NAME:=pecostat
PKG_VERSION=2.0.2
PKG_RELEASE:=1
PKG_MD5SUM:=594a8b9ded23ae0f6231f0526ffed58e
PKG_BASE_NAME:=pecostat34BE
PKG_SOURCE:=$(PKG_BASE_NAME).tgz
PKG_CAT:=zcat
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_BASE_NAME)/srcBE
PKG_SOURCE_URL:=http://localhost

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Access MIPS_34K and 24K HW performance counters
  DEPENDS:=+kmod-$(PKG_NAME)
  URL:=http://www.mips.com
  MAINTAINER:=MIPS.com
endef

define Package/$(PKG_NAME)/description
  This package contains pecostat which is used for accessing MIPS_34K and 24K HW performance counters.
endef

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Other modules
  SECTION:=kernel
  DEPENDS:=@!TARGET_lantiq_xrx500 
  TITLE:=pecostat Kernel Support
  FILES:=$(PKG_BUILD_DIR)/pecostat_noIRQ.$(LINUX_KMOD_SUFFIX)
endef

define KernelPackage/$(PKG_NAME)/description
  Kernel support to pecostat for accessing MIPS_34K and 24K HW performance counters
endef

MAKE_OPTS:= \
  LINUX_DIR="$(LINUX_DIR)" \
  ARCH="$(LINUX_KARCH)" \
  CROSS_COMPILE="$(TARGET_CROSS)" \
  SUBDIRS="$(PKG_BUILD_DIR)" \
  EXTRA_CFLAGS="$(EXTRA_CFLAGS) -fhonour-copts"

#### Target Rules ###############################
define Build/Compile
	$(MAKE_VARS) \
        $(MAKE) -C $(PKG_BUILD_DIR)/ \
                $(MAKE_FLAGS)

	$(MAKE) -C "$(LINUX_DIR)" \
                $(MAKE_OPTS) \
                modules
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/pecostat.h $(1)/usr/include
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pecostat $(1)/usr/sbin
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/pecostat.h $(1)/usr/include
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
