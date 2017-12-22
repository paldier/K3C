#******************************************************************************
#
#                         Copyright (c) 2015
#                        Lantiq Beteiligungs-GmbH & Co. KG
#
#  For licensing information, see the file 'LICENSE' in the root folder of
#  this software module.
#
#******************************************************************************

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/ugw-loglevel.mk

#### Package Info ###############################
PKG_NAME:=ltq-pecoevent
PKG_VERSION:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(shell pwd)/src
PKG_BUILD_DEPENDS:= PACKAGE_pecostat_interaptiv:pecostat_interaptiv PACKAGE_pecostat:pecostat +system_fapi

#export PKG_BUILD_DIR

define Package/$(PKG_NAME)
  SECTION:=ltq-pecoevent
  CATEGORY:=Lantiq
  SUBMENU:=UGW Software
  TITLE:=pecoevent is used get counter of performance counters from pecostat
  URL:=http://www.lantiq.com
  DEPENDS:=@PACKAGE_pecostat_interaptiv||@PACKAGE_pecostat +system_fapi
  MAINTAINER:= Lantiq Deutschland GmbH == www.lantiq.com
endef

define Package/$(PKG_NAME)/description
 This package contains an utility to get counter values from pecostat
endef

### Exports ##################################
export LINUX_KARCH TARGET_CONFIGURE_OPTS LINUX_DIR PKG_BUILD_DIR

#### Target Rules ###############################
define Build/Prepare
	$(CP) -L ./src/* $(PKG_BUILD_DIR)/
	$(call Build/Prepare/Default)
endef

# To ipkg / Rootfs
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pecoevent_collect.ko $(1)/lib/modules/$(LINUX_VERSION)/
endef

# To Staging Dir
define Build/InstallDev
#	$(INSTALL_DIR) $(1)/usr/include
#	$(CP) $(PKG_BUILD_DIR)/pecoevent.h $(1)/usr/include
endef

define Build/Clean
	$(MAKE) -C $(PKG_BUILD_DIR) clean
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
