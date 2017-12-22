##############################################
include $(TOPDIR)/rules.mk
# Name and release number of this package
PKG_NAME:=lsqlite3
PKG_VERSION:=fsl09b
PKG_RELEASE:=1

PKG_SOURCE_URL:=http://lua.sqlite.org/index.cgi/zip
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).zip
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=lsqlite3 -- prints a snarky message
	DEPENDS:=+libsqlite3
endef

define Package/$(PKG_NAME)/description
 LuaSQLite 3 is a thin wrapper around the public domain SQLite3 database engine.
endef

define Package/$(PKG_NAME)/extra_provides
	echo 'libsqlite3.so.0'; \
	echo 'libc.so.6';
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.so $(1)/usr/lib/lua
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

