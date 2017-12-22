#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lxml
PKG_VERSION:=3.7.2
PKG_RELEASE:=1							 

#PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=http://lxml.de/files/
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:=1cd159ef1fa6c13f9f21ddf9d69385b686b384a1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL:=1

HOST_BUILD_DEPENDS:=libxml2/host libxslt/host

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk

HOST_CFLAGS += -I$(STAGING_DIR_HOST)/include/libxml2 -I$(STAGING_DIR_HOST)/include/libexslt

export HOST_CFLAGS

define Package/lxml
  SECTION:=libs
  CATEGORY:=Libraries
  URL:=http://www.lxml.de/
  TITLE:=lxml 3.7.2
endef

define Package/libxml2/description
  A library for parsing XML files
endef

define Host/Compile
	cd $(HOST_BUILD_DIR) && \
		$(HOST_CONFIGURE_VARS) python setup.py build
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/lib/
	$(CP) $(HOST_BUILD_DIR)/build/lib.*/lxml $(STAGING_DIR_HOST)/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,lxml))
