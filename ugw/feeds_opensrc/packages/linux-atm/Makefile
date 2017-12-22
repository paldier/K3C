# 
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 9906 2007-12-25 01:40:47Z nbd $

include $(TOPDIR)/rules.mk
include $(TOPDIR)/include/kernel.mk

PKG_NAME:=linux-atm
PKG_VERSION:=2.5.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://sourceforge.net/projects/linux-atm/files/linux-atm/2.5.0/
PKG_MD5SUM:=0b45a0e801fac7093ce4b0cadf419965
PKG_FIXUP=libtool
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_linux_atm
PKG_NO_AUTORECONF=y

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Linux ATM library
  URL:=http://linux-atm.sourceforge.net/
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  This package contains a library for accessing the Linux ATM subsystem.
endef

define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	config FEATURE_CLIP
		depends on PACKAGE_linux-atm
		bool 'CLIP'
endef

define Package/atm-tools
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=$(PKG_NAME)
  TITLE:=Linux ATM tools
  URL:=http://linux-atm.sourceforge.net/
endef

define Package/atm-tools/description
  This package contains the Linux ATM tools.
endef

#CONFIGURE_ARGS	+= --with-kernel-headers=$(LINUX_DIR) --enable-ifx-opt --program-transform-name=""
#MAKE_FLAGS += INCLUDES+="-I$(LINUX_DIR)/include" DEFS+="-DCONFIG_IFX"
CONFIGURE_ARGS	+= --enable-ifx-opt --program-transform-name=""
MAKE_FLAGS += DEFS+="-DCONFIG_IFX"

define Build/Prepare
	$(call Build/Prepare/Default)
	-find $(PKG_BUILD_DIR)/ -name Makefile.in | xargs rm -f
	-rm -rf $(PKG_BUILD_DIR)/{aclocal.m4,autom4te.cache,config.guess,config.h.in,config.sub,configure}
	-rm -rf $(PKG_BUILD_DIR)/{depcomp,INSTALL,install-sh,ltmain.sh,missing,ylwrap}
endef

define Build/Configure
	# prevent autoheader invocation
	touch $(PKG_BUILD_DIR)/stamp-h.in
	(cd $(PKG_BUILD_DIR)/; autoreconf -i)
	$(call Build/Configure/Default)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src/lib DESTDIR="$(PKG_INSTALL_DIR)" $(MAKE_FLAGS) libdir=/lib; \
	if [ A$(CONFIG_FEATURE_RFC2684) = Ay ];then \
		$(MAKE) -C $(PKG_BUILD_DIR)/src/br2684 DESTDIR="$(PKG_INSTALL_DIR)" $(MAKE_FLAGS) libdir=/lib; \
	fi; \
	if [ A$(CONFIG_FEATURE_CLIP) = Ay ];then \
		$(MAKE) -C $(PKG_BUILD_DIR)/src/arpd DESTDIR="$(PKG_INSTALL_DIR)" $(MAKE_FLAGS) install; \
		$(MAKE) -C $(PKG_BUILD_DIR)/src/maint DESTDIR="$(PKG_INSTALL_DIR)" $(MAKE_FLAGS) atmaddr; \
	fi; \
	$(MAKE) -C $(PKG_BUILD_DIR)/src/include DESTDIR="$(PKG_INSTALL_DIR)" $(MAKE_FLAGS) install
	$(MAKE) -C $(PKG_BUILD_DIR)/src/lib DESTDIR="$(PKG_INSTALL_DIR)/usr" $(MAKE_FLAGS) libdir=/lib install
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	$(CP)	$(PKG_INSTALL_DIR)/usr/include/atm{,d,sap}.h \
		$(1)/usr/include/
	mkdir -p $(1)/lib
	$(CP)	$(PKG_INSTALL_DIR)/usr/lib/libatm.{a,so*} \
		$(1)/lib/
	mkdir -p $(1)/usr/include/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libatm.so.* $(1)/lib/
endef

define Package/atm-tools/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	if [ A$(CONFIG_FEATURE_CLIP) = Ay ];then \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/atmarp{,d} $(1)/usr/sbin/; \
	fi
endef

define Build/Clean
        if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
        $(RM) $(PKG_BUILD_DIR)/ipk; \
	find $(PKG_BUILD_DIR) -name ".deps" | rm -rf; \
		$(RM) $(PKG_BUILD_DIR)/.[a-z]*; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,atm-tools))
