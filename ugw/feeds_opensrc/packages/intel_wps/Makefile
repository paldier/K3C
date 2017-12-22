#
# Openwrt Makefile for intel_wps
#

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ###############################
PKG_NAME:=WscRefImpl_Intel
PKG_VERSION:=1_0_6
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_VERSION)
PKG_STAGING_DIR:=$(STAGING_DIR)/target-mips_uClibc-0.9.29_arx_gw_he_adsl
PKG_SOURCE_URL:=ftp://download.intel.com/software/wsc
PKG_BUILD_DEPENDS:=libopenssl

include $(INCLUDE_DIR)/package.mk

STL_LIBDIR=${STAGING_DIR}/usr/lib

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Intel-WPS
  URL:=http://www.intel
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  Intel WSC package, implementing WPS functionality
endef

#### Directory Paths ############################
OPENSSL_DIR:=$(wildcard $(BUILD_DIR)/openssl-*)

IFX_CFLAGS += -I$(OPENSSL_DIR)/include
IFX_LDFLAGS += -Bdynamic -L$(OPENSSL_DIR)/ -L$(STL_LIBDIR)

#### Target Rules ###############################
define Build/Prepare
	$(call Build/Prepare/Default)
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(PKG_BUILD_DIR)/src \
	IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/linux/wsccmd $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/linux/hostapd.eap_user $(1)/bin
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg-$(BOARD); fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
