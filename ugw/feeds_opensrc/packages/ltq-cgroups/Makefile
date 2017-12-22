#
# Copyright (C) 2009-2010 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ltq-cgroups
PKG_VERSION:=2.0.1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/ltq-cgroups
  SECTION:=ltq-cgroups
  CATEGORY:=Lantiq
  TITLE:=ltq-cgroups is used for classifying the tasks based on the priority.
  DEPENDS:= +kmod-ltq-cgroup-support
  MAINTAINER:= Lantiq Deutschland GmbH
  MENU:=1
endef

define Package/$(PKG_NAME)/description
 This package contains an utility to classsify the tasks based on the priorities.
endef

define KernelPackage/ltq-cgroup-support
  SUBMENU:=Lantiq
  TITLE:=Lantiq Control Group Support
  KCONFIG:= \
        CONFIG_CGROUPS=y \
        CONFIG_RESOURCE_COUNTERS=y \
        CONFIG_MEMCG=y \
        CONFIG_MEMCG_KMEM=y \
        CONFIG_CGROUP_SCHED=y \
        CONFIG_FAIR_GROUP_SCHED=y \
        CONFIG_MM_OWNER=y \
        CONFIG_CONNECTOR=y \
        CONFIG_PROC_EVENTS=y

endef

define KernelPackage/ltq-cgroup-support/description
 Enables kernel settings for Control Group Support
endef

### Menuconfig ##################################
define Package/$(PKG_NAME)/config
	menu "CGroup affinity features"
		depends on PACKAGE_$(PKG_NAME)
		source "$(SOURCE)/config/Config.in"
	endmenu
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
endef

TARGET_CFLAGS += $(if $(CONFIG_FEATURE_CGROUP_AFFINITY),-DFEATURE_CGROUP_AFFINITY)
EXTRA_CFLAGS += $(if $(CONFIG_PACKAGE_ltq-voip),-DCONFIG_PACKAGE_LTQ_VOIP)

define Build/Prepare
	$(call Build/Prepare/Default)
	$(if $(CONFIG_$(PKG_NAME)_USE_CUSTOM_SOURCE_DIR),,$(CP) -L ./src/* $(PKG_BUILD_DIR)/)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) ./files/cgroups.conf $(1)/etc/
	$(if $(CONFIG_NEW_FRAMEWORK), \
		$(INSTALL_DIR) $(1)/etc/preinit.d; \
		$(INSTALL_BIN) ./files/ltq_cgroups_startup_procd.sh $(1)/etc/init.d/ltq_cgroups_startup.sh; \
		$(INSTALL_BIN) ./files/ltq_cgroups_preinit.sh $(1)/etc/preinit.d/; \
	, \
		$(INSTALL_BIN) ./files/ltq_cgroups_startup.sh $(1)/etc/init.d/; \
	)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cgroup_notifier $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cgroup_daemon $(1)/usr/bin/
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg-$(BOARD); fi
endef

$(eval $(call KernelPackage,ltq-cgroup-support))
$(eval $(call BuildPackage,$(PKG_NAME)))
