#
# Openwrt Makefile for procps
#
#
#### Includes ###################################
include $(TOPDIR)/rules.mk

#### Package Info ###############################
PKG_NAME:=procps
PKG_VERSION:=3.2.8
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://localhost

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=procps
	URL:=http://procps.sourceforge.net/
	DEPENDS:=+libncurses
endef

define Package/$(PKG_NAME)/description
	This package contains procps utilites.
endef

#### Directory Paths ############################
NCURSES_DIR:=$(wildcard $(BUILD_DIR)/ncurses*)

### Menuconfig ##################################

### Configurations depends on Menuconfig ########

#### Export Section for Features & Others #######

#### Target Rules ###############################
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) -I$(NCURSES_DIR)/include -I$(NCURSES_DIR)/ipkg-install/usr/include/" \
	CPPFLAGS="-I$(NCURSES_DIR)/include" \
	CURSES="-L$(NCURSES_DIR)/lib/ -lncurses"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/watch $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vmstat $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pgrep $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pkill $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/w $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/slabtop $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/skill $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pmap $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/uptime $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pwdx $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/proc/libproc-$(PKG_VERSION).so $(1)/usr/lib
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
