#
# Copyright (C) 2007-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=k3c-gpio-led
PKG_VERSION:=1.0
PKG_RELEASE:=2
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Lantiq
  TITLE:=Lantiq GPIO Leds
endef

define Package/$(PKG_NAME)/description
  A simple script to prepare the board gpio leds
endef

define Package/$(PKG_NAME)/config
   menu "configuration"
   depends on PACKAGE_$(PKG_NAME)
       source "$(SOURCE)/config/Config.in"
   endmenu
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/ltq_gpio_led.sh $(1)/etc/init.d/
	$(INSTALL_BIN) ./files/k3cled $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,k3c-gpio-led))
