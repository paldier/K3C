#
# Openwrt Makefile for tinytcl
#

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ###############################
PKG_NAME:=tinytcl
PKG_VERSION:=680
PKG_RELEASE:=1
PKG_BASE_NAME:= tinytcl
PKG_SOURCE:=tinytcl$(PKG_VERSION).zip
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_BASE_NAME)
PKG_SOURCE_URL:=http://downloads.sourceforge.net/project/tinytcl/tinytcl/TinyTcl%206.8.0%20for%20embedded%20and%20native%20DOS

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Utilities
  TITLE:=tinytcl(680)
  URL:=http://tinytcl.sourceforge.net
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  Rommable, minimal Tcl for embedded applications
endef

#### Target Rules ###############################
define Build/Prepare
	$(call Build/Prepare/Default)
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(PKG_BUILD_DIR) \
	IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tclsh $(1)/bin
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg-$(BOARD); fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
