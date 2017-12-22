#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=klish
PKG_VERSION:=2.1.3
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
#PKG_SOURCE_URL:=http://klish.googlecode.com/files
PKG_SOURCE_URL:=http://sources.lede-project.org/
PKG_MD5SUM:=7dfe46d474c02c86946c1d7a461ae549
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/ugw-loglevel.mk

define Package/klish/default
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Kommand Line Interface SHell ($(1))
  URL:=http://code.google.com/p/klish/
endef

define Package/klish
$(call Package/klish/default,main tool)
  DEPENDS:=+libstdcpp +libexpat +libcal
endef

define Package/konf
$(call Package/klish/default,konf tool)
  DEPENDS:=klish
endef

define Package/klish/description
 The klish is a framework for implementing a CISCO-like CLI on a UNIX
 systems. It is configurable by XML files. The KLISH stands for Kommand
 Line Interface Shell.
 The klish is a fork of clish 0.7.3 developed by Graeme McKerrell.
 It defines new features but it's compatible (as much as possible) with
 clish's XML configuration files.
 klish is able to run using clish XML configuration files although
 current clish users may expect some changes in behavior.
endef

define Package/konf/description
 The klish is a framework for implementing a CISCO-like CLI on a UNIX
 systems. It is configurable by XML files. The KLISH stands for Kommand
 Line Interface Shell.
 Konf and konfd are klish utilities that are used to store configuration
 informations in a way which is similar to what's found on CISCO devices.
 More information about these tools is to be found on the klish web site.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) -L ./files/*.c $(PKG_BUILD_DIR)/plugins/clish/
	$(CP) -L ./files/*.h $(PKG_BUILD_DIR)/plugins/clish/
endef
define Package/klish/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/clish $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Package/konf/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/konf $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/konfd $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libkonf.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblub.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,klish))
$(eval $(call BuildPackage,konf))

define Package/klish-xml-files
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=klish
  TITLE:=klish sample XML files
  URL:=http://code.google.com/p/klish/
endef

define Package/klish-xml-files/description
  This is a set of sample XML files for klish. This specific sample set
  is compatible with the original clish.
endef

define Package/klish-xml-files/install
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/cli
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/cli/map-files/
	$(INSTALL_BIN) ./files/xmls/common.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_BIN) ./files/xmls/configure-mode.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_BIN) ./files/xmls/enable-mode.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_BIN) ./files/xmls/startup.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_BIN) ./files/xmls/template.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_BIN) ./files/xmls/transaction.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_BIN) ./files/xmls/types.xml $(1)/$(VENDOR_PATH)/cli/xmls/
endef

$(eval $(call BuildPackage,klish-xml-files))
