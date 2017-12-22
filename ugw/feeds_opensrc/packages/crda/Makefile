#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=crda
PKG_RELEASE:=1
PKG_VERSION:=1.1.3
PKG_SOURCE_URL:=https://www.kernel.org/pub/software/network/crda/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=29579185e06a75675507527243d28e5c

PKG_REGULATORY_NAME:=regulatory
PKG_REGULATORY_VERSION:=2015.10.13_WAVE05
#PKG_REGULATORY_SOURCE_URL:=http://wireless.kernel.org/download/wireless-regdb/regulatory.bins
PKG_REGULATORY_SOURCE:=$(PKG_REGULATORY_VERSION)-$(PKG_REGULATORY_NAME).bin
#PKG_REGULATORY_MD5SUM:=e0c8a5ca63fb8bf803213f9a0c90b50b


PKG_BUILD_DEPENDS:=libnl3

include $(INCLUDE_DIR)/package.mk

define Package/crda
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Central Regulatory Domain Agent (CRDA)
  DEPENDS:=+libnl3
  URL:=http://wireless.kernel.org/en/developers/Regulatory/CRDA
endef

define Download/wireless-regdb
  FILE:=$(PKG_REGULATORY_SOURCE)
  URL:=$(PKG_REGULATORY_SOURCE_URL)
  VERSION:=$(PKG_REGULATORY_VERSION)
  MD5SUM:=$(PKG_REGULATORY_MD5SUM)
endef
#$(eval $(call Download,wireless-regdb))

define Package/crda/description
 This is the Central Regulatory Domain Agent for Linux. It serves one
 purpose: tell Linux kernel what to enforce. In essence it is a udev
 helper for communication between the kernel and userspace. You only
 need to run this manually for debugging purposes. For manual changing
 of regulatory domains use iw (iw reg set) or wpa_supplicant (feature
 yet to be added).
endef

LIBNL_DIR:=$(wildcard $(BUILD_DIR)/libnl-3*)

TARGET_CPPFLAGS := \
	-I$(LIBNL_DIR)/include \
	-D_GNU_SOURCE \
	$(TARGET_CPPFLAGS)

#	-I$(STAGING_DIR)/usr/include/libnl 

MAKE_FLAGS += \
	NL1FOUND="" NL2FOUND="" NL3FOUND=Y \
	NLLIBNAME="libnl-3.0" \
	NLLIBS="-L$(LIBNL_DIR)/lib/.libs -lnl-3 -lnl-genl-3 -lm" \
	REG_BIN="./files/$(PKG_REGULATORY_SOURCE)" \
	crda regdbdump

define Package/crda/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/hotplug.d
	$(INSTALL_DIR) $(1)/etc/hotplug.d/platform
	$(INSTALL_DIR) $(1)/usr/lib/crda
	$(INSTALL_DIR) $(1)/etc/udev/rules.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/crda $(1)/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/regdbdump $(1)/sbin/
	$(INSTALL_DATA) ./files/hotplug.rule $(1)/etc/hotplug.d/platform/10-regulatory
	$(INSTALL_DATA) ./files/regulatory.rules $(1)/etc/udev/rules.d/regulatory.rules
	$(INSTALL_DATA) ./files/$(PKG_REGULATORY_SOURCE) $(1)/usr/lib/crda/regulatory.bin
endef

$(eval $(call BuildPackage,crda))

