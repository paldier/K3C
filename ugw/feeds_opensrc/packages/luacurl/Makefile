#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luacurl
PKG_VERSION:=20140207
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/msva/lua-curl.git
PKG_SOURCE_VERSION:=047ac31ecd18c2526eed84e349e5016de2633fac
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/luacurl
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Lua-cURL
  URL:=http://msva.github.io/lua-curl/
  DEPENDS:=+liblua +libcurl
endef

define Package/luacurl/description
 Lua-cURL is aiming for a full-fledged libcurl binding (easy/multi/share
 interface) to the functionality of Lua
endef

CMAKE_OPTIONS += \
	-DUSE_LUA=ON

define Package/luacurl/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cURL.so $(1)/usr/lib/lua/
endef

$(eval $(call BuildPackage,luacurl))
