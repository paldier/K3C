################includes##############################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
######################################################

PKG_NAME:=electric-fence
PKG_SUBVER:=2.1.13
PKG_RELEASE:=0.1
PKG_VERSION:=$(PKG_SUBVER)-$(PKG_RELEASE)
PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://perens.com/FreeSoftware/ElectricFence/
PKG_MD5SUM:=59e4e7817a30aff52d8971ce00e1ad35
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SUBVER)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=Electric_Fence
  URL:=http://perens.com/FreeSoftware/ElectricFence/ 
endef

define Package/$(PKG_NAME)/description
Electric Fence helps you detect two common programming bugs: software that overruns the boundaries \
of a malloc() memory allocation, and software that touches a memory allocation that has been released by free().
endef

MAKE_OPTS:= \
        LINUX_DIR="$(LINUX_DIR)" \
        ARCH="$(LINUX_ARCH)" \
        CROSS_COMPILE="$(TARGET_CROSS)" \
        SUBDIRS="$(PKG_BUILD_DIR)" \
        EXTRA_CFLAGS="$(EXTRA_CFLAGS)"


####################target rules ########################

define Build/Prepare
        $(call Build/Prepare/Default)
endef

#define Build/Compile
#$(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_OPTS) PKG_BUILD_DIR=$(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libefence.a $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libefence.a $(STAGING_DIR)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

