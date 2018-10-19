include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk

PKG_NAME:=ltq-temp
PKG_VERSION:=1.1.2
PKG_RELEASE:=1

PKG_MAINTAINER:=paldier <paldier@hotmail.com>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk


define Package/$(PKG_NAME)
  SECTION:=power
  CATEGORY:=Lantiq
  TITLE:=Temperature Sensor Driver
  URL:=http://www.lantiq.com/
  MAINTAINER:=Lantiq Deutschland GmbH
  DEPENDS:=+kmod-hwmon-core
  MENU:=1
  SUBMENU:= 
endef

define Package/$(PKG_NAME)/description
	Temerature Sensor for XRX200, XRX300, GRX330 and GRX350 platforms.
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/{lib,include} 
	$(CP) ./files/ltq_temp_interface.h $(1)/usr/include/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/ltq_temp.sh $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION)/
	$(INSTALL_BIN) ./files/ltq_temp.ko $(1)/lib/modules/$(LINUX_VERSION)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
