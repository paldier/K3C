#
# Copyright (C) 2007-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=knopflerfish_osgi
PKG_VERSION:=3.2.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.knopflerfish.org/download.html
PKG_MD5SUM:=1b36ceb37512284fe637389a7e9367c6
PKG_CAT:=zcat

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=OSGi
  #DESCRIPTION:=
  URL:=http://www.knopflerfish.org/releases/3.2.0/
  DEPENDS:=@TARGET_ltqcpe_platform_vr9 @PACKAGE_kmod-ltqcpe_nand
endef

define Build/Compile
	:
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/opt/lantiq/kf_osgi/jars
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/console		$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/consoletty		$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/cm			$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/frameworkcommands	$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/log			$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/logcommands		$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/useradmin		$(1)/opt/lantiq/kf_osgi/jars/
	$(CP) $(PKG_BUILD_DIR)/osgi/jars/ltq_dect		$(1)/opt/lantiq/kf_osgi/jars/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/osgi/kf2		$(1)/opt/lantiq/kf_osgi/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/osgi/*.jar		$(1)/opt/lantiq/kf_osgi/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/osgi/*.xargs		$(1)/opt/lantiq/kf_osgi/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
