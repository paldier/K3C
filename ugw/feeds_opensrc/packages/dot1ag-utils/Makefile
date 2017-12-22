include $(TOPDIR)/rules.mk

PKG_NAME:=dot1ag-utils
PKG_VERSION:=1.0.1
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/vnrick/dot1ag-utils.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=01a24e5de960318c4cc8a97212cc33b38d19e3e2
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_MD5SUM:=xxx
# PKG_MIRROR_MD5SUM:=
# CMAKE_INSTALL:=1

PKG_LICENSE:=BSD
PKG_LICENSE_FILES:=

include $(INCLUDE_DIR)/package.mk


define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=dot1ag-utils
  DEPENDS:=+libpcap
endef

define Package/$(PKG_NAME)/description
	The $(PKG_NAME) is an update to dot1ag-utils software package which is an Open Source (new BSD License) implementation of the IEEE 802.1ag protocol.
	It provides several debugging tools that interact with IEEE 802.1ag enabled routers and switches. The package implements a loopback message initiator (ethping), a link trace message initiator (ethtrace) and a daemon (ltq-ethoamd) that responds to loopback messages and link trace messages. Sending and receiving of continuity check messages is implemented in the dot1ag_ccd daemon.
endef


TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include

define Package/dot1ag-utils/install
	$(INSTALL_DIR) $(1)/usr/sbin 
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/dot1agd $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/{ethtrace,ethping,dot1ag_ccd} $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,dot1ag-utils))
