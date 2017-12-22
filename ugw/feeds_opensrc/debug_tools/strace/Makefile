#
# Openwrt Makefile for strace     
#
#
#### Includes ###################################
include $(TOPDIR)/rules.mk

#### Package Info ###############################
PKG_NAME:=strace
PKG_VERSION:=4.5.19
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://localhost

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=Strace
	URL:=http://sourceforge.net/projects/strace/ 
endef

define Package/$(PKG_NAME)/description
	This package contains strace utility.
endef

#### Directory Paths ############################

### Menuconfig ##################################

### Configurations depends on Menuconfig ########

#### Export Section for Features & Others #######

#### Target Rules ###############################
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/strace $(1)/usr/bin  
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
