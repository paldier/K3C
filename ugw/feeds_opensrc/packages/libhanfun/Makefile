#
# ULE Alliance's HAN-FUN OpenWrt package
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hanfun
PKG_VERSION:=1.4.3
PKG_RELEASE:=1
PKG_INSTALL:=0
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_VERSION:= v1.4.3
PKG_SOURCE_VERSION:= bf7c446c93463d43bd2226724bf817b29f1b97e9
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.xz
#PKG_SOURCE_URL:=http://www.ulealliance.org
PKG_SOURCE_URL:=https://github.com/ULE-Alliance/hanfun.git
#PKG_CAT=zcat
#PKG_MD5SUM:=28828832fb3fa3771165665e5aa1440b
PKG_MD5SUM:= 5bf341879dc2429681d8207367d9d65d
#PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(PKG_SOURCE)

TARGET_CFLAGS += -DHF_APP_CONFIG_FILE="\\\"/flash/HF_rc.conf\\\"" -DHF_LOG_LEVEL=HF_LOG_LEVEL_DEBUG

CMAKE_OPTIONS:=-DHF_BASE_LIB=ON -DHF_APP_EXT_REG=ON

include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libhanfun
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Home Area Network FUNctional protocol
  URL:=http://www.ulealliance.org
  MAINTAINER:=Bithium
  MENU:=1
  DEPENDS:=+libstdcpp +libc +libgcc
endef

define Package/libhanfun/description
  Implementation of the HAN-FUN application specification.
endef

define Build/InstallDev
  pushd $(PKG_BUILD_DIR) && make DESTDIR=$(1) install && popd
endef

$(eval $(call BuildPackage,libhanfun))
