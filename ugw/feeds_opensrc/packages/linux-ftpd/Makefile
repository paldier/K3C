#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=linux-ftpd
PKG_VERSION:=0.17
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://ftp.linux.hr/gentoo/distfiles/distfiles
PKG_MD5SUM:=f5f491564812db5d8783daa538c49186
PKG_NO_AUTORECONF=y
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_linux_ftpd
PKG_BUILD_DEPENDS:=ltq-mapi

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Linux FTP Daemon for file transfer
  URL:=ftp://ftp.linux.hr/gentoo/distfiles/distfiles/
endef

define Package/$(PKG_NAME)/description
  Daemon for FTP (File Transfer Protocol)
endef

define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	config FEATURE_FTP_SERVER
		default y
		depends on PACKAGE_linux-ftpd
		bool 'linux_ftpd'
endef

CONFIGURE_ARGS = \
    --prefix=/usr --exec-prefix=/usr --with-c-compiler=$(TARGET_CC) \
    --disable-shadow --disable-libinet6 --enable-ipv6

IFX_INCLUDE = -I$(STAGING_DIR)/usr/include
IFX_INCLUDE += -DIFX_SMALL_FOOTPRINT -DIFX_SKIP_API_INCL
IFX_APIS_DIR:=$(wildcard $(BUILD_DIR)/ltq-mapi*)

ifeq ($(CONFIG_PACKAGE_ltq-mapi),y)
  ifeq ($(CONFIG_PACKAGE_ifx-oam),y)
    IFX_INCLUDE += -I$(IFX_APIS_DIR)/include -DIFX_FTP_UPGRADE 
    IFX_LDFLAGS = -L$(IFX_APIS_DIR)/lib -Wl,-rpath-link,$(IFX_APIS_DIR)/lib -L$(STAGING_DIR)/usr/lib -loamapi -lIFXAPIs -lifx_common
  endif
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) IFX_CFLAGS="$(IFX_CFLAGS) $(IFX_INCLUDE)" IFX_LDFLAGS="$(IFX_LDFLAGS)" all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) IFX_CFLAGS="$(IFX_CFLAGS) $(IFX_INCLUDE)" IFX_LDFLAGS="$(IFX_LDFLAGS)" INSTALLROOT=$(1) install
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/ftpd/ftpd $(1)/usr/sbin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
