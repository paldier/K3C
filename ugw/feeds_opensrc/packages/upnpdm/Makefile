#
# Copyright (C) 2007-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=upnpdm
#PKG_VERSION:=0.12.2
PKG_VERSION:=1.2.0.17
PKG_RELEASE:=1

PKG_BASE_NAME:=upnpdm
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_URL:=http://sourceforge.net/projects/upnpdm/
#PKG_MD5SUM:=89b999d4a28078e1b67c7dbedd14c7b5
PKG_CAT:=zcat
PKG_SOURCE_URL:=http://localhost

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/trunk

PKG_NO_AUTORECONF:=y
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/upnp-md/Default
  SECTION:=net
  CATEGORY:=Network
  TITLE:=An UPnP DM implementation
  URL:=http://sourceforge.net/
endef

define Package/upnp-md/Default/description
 An UPnP Device Management implementation (daemon).
 It provides a common solution through defining standard management actions
 and data models, which can be implemented in device running different
 execution environments. UPnP DM also allows defining new data models
 for specific device usage.
endef

define Package/upnpdm
  $(call Package/upnp-md/Default)
  SUBMENU:=upnp-md
  DEPENDS:=+libupnp +libpthread +ltq-mapi +ltq-hn
  TITLE+= (daemon)
endef

define Package/upnpdm/description
$(call Package/upnp-md/Default/description)
 .
 This package contains an UPnP Managable Device daemon.
endef

define Package/upnpdm/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	menu "UPnP-DM features"
		depends on PACKAGE_$(PKG_NAME)
		source "$(SOURCE)/config/Config.in"
	endmenu
endef

### Configurations depends on Menuconfig ########
LTQ_CFLAGS_y:=
LTQ_LDFLAGS_y:=

ifeq ($(CONFIG_FEATURE_IFX_HOST_ENVIR),y)
  LTQ_CFLAGS_y += -g -DHOSTENV -Wall -fno-stack-protector -DBUILD_FROM_LTQ_APPS
else
  LTQ_CFLAGS_y += $(TARGET_CFLAGS) -DBUILD_FROM_LTQ_APPS
  LTQ_LDFLAGS_y += $(TARGET_LDFLAGS)
endif

ifeq ($(CONFIG_FEATURE_GHN_DATA_MODEL),y)
  LTQ_CFLAGS_y += -DCONFIG_FEATURE_GHN_DATA_MODEL
endif

LTQ_CFLAGS_y += -I$(STAGING_DIR)/usr/include
LTQ_CFLAGS_y += -I$(STAGING_DIR)/usr/include/ifx_IFXAPIs
LTQ_CFLAGS_y += -I$(STAGING_DIR)/usr/include/upnp

LTQ_LDFLAGS_y += -L$(STAGING_DIR)/usr/lib
LTQ_LDFLAGS_y += -lIFXAPIs -lifx_common -lifx_osa -loamapi
LTQ_LDFLAGS_y += -lhn_mapi -lhn
LTQ_LDFLAGS_y += -lupnp -lixml -lthreadutil

LTQ_CFLAGS:=$(LTQ_CFLAGS_y)
LTQ_LDFLAGS:=$(LTQ_LDFLAGS_y)

define Build/Compile
	$(CP) ./files/DMconf.xml $(PKG_BUILD_DIR)/conf/
	#$(CP) ./files/web/*.xml $(PKG_BUILD_DIR)/web/
	#$(MAKE) -C $(PKG_BUILD_DIR)/xml2C4MD all
	#cd $(PKG_BUILD_DIR); ./buildconfig pupnp; cd -

	$(MAKE) LINUX_DIR="$(LINUX_DIR)" $(TARGET_CONFIGURE_OPTS) -C $(PKG_BUILD_DIR)/ all LTQ_CFLAGS="$(LTQ_CFLAGS)" LTQ_LDFLAGS="$(LTQ_LDFLAGS)"

	$(STRIP) $(PKG_BUILD_DIR)/upnp-md-pupnp/upnp-md
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/upnp-md-pupnp/upnp-md $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/ramdisk_copy/etc/upnp-md/web
	$(INSTALL_DATA) ./files/upnp-md.conf $(1)/ramdisk_copy/etc/upnp-md/
	$(INSTALL_DATA) ./files/DMconf.xml $(1)/ramdisk_copy/etc/upnp-md/
	$(INSTALL_DATA) ./files/web/*.xml $(1)/ramdisk_copy/etc/upnp-md/web/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/upnpdm.init $(1)/etc/init.d/upnpdm
	$(RM) $(1)/etc/upnp-md
	cd $(1); ln -s ../ramdisk/etc/upnp-md etc/upnp-md; cd -
endef

define Build/Clean
	$(MAKE) LINUX_DIR="$(LINUX_DIR)" $(TARGET_CONFIGURE_OPTS) -C $(PKG_BUILD_DIR)/ clean LTQ_CFLAGS="$(LTQ_CFLAGS)" LTQ_LDFLAGS="$(LTQ_LDFLAGS)"

	#$(MAKE) -C $(PKG_BUILD_DIR)/xml2C4MD clean
endef

$(eval $(call BuildPackage,upnpdm))
