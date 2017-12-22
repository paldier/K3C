#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ntfs-3g
PKG_RELEASE:=3

PKG_UCLIBC_VERSION:=$(call qstrip,$(CONFIG_UCLIBC_VERSION))
#ifneq (,$(or $(findstring !0.9.29,!$(PKG_UCLIBC_VERSION)),$(findstring !0.9.30.1!,!$(PKG_UCLIBC_VERSION)!) ))
PKG_VERSION:=1.616
#else
#	PKG_VERSION:=2010.3.6
#endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tgz
PKG_SOURCE_URL:=http://www.tuxera.com/opensource/

PKG_MD5SUM_2010.1.16:=e104c914e8d7d29ee83e63d46afbba25
PKG_MD5SUM_2010.3.6:=12ce21aa044c6068a4df6e8cbd3c5cfa
PKG_MD5SUM=$(PKG_MD5SUM_$(PKG_VERSION))

PKG_FIXUP:=libtool
#PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/ntfs-3g/common
	SECTION:=utils
	CATEGORY:=Utilities
	URL:=http://www.tuxera.com
	SUBMENU:=Filesystem
	TITLE:=Stable Read/Write NTFS Driver
	MAINTAINER:=Bud <wrt_buddhay@gmx.net>
endef

define Package/ntfs-3g
	$(call Package/ntfs-3g/common)
	DEPENDS:= +kmod-fuse +PACKAGE_NTFS-3G_USE_LIBFUSE:libfuse
	DEPENDS+= +libpthread
endef

define Package/ntfs-3g/description
Ntfs-3g is a NTFS driver, which can create, remove, rename,
move files, directories, hard links, and streams. It can read
and write files, including streams and sparse files. It can
handle special files like symbolic links, devices, and FIFOs.
Moreover it can also read transparently compressed files.

Contains:
 - ntfs-3g
 - ntfs-3g.probe

endef

define Package/ntfs-3g/config
config PACKAGE_NTFS-3G_USE_LIBFUSE
	bool "use external FUSE library (package libfuse)"
	depends on PACKAGE_ntfs-3g
	---help---
	Ntfs-3g by default uses a minimalized lite version of FUSE.
	If libfuse is part of your filesystem anyway (because of sshfs, owfs
	etc.) it makes sense to activate this option and save some kilobytes
	of space.
endef

define Package/ntfs-3g-utils
	$(call Package/ntfs-3g/common)
	TITLE:=ntfs-3g utilities (ntfs-3g.secaudit, ntfs-3g.usermap)
	DEFAULT:=n
	DEPENDS+= +ntfs-3g
endef

define Package/ntfs-3g-utils/description
Additional ntfs-3g utilities. Not included by default for size
considerations. All binaries except ntfs-3g, ntfs-3g.probe.

Currently:
 - ntfs-3g.secaudit
 - ntfs-3g.usermap

endef

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-ldconfig

# insert config with external libfuse
ifdef CONFIG_PACKAGE_NTFS-3G_USE_LIBFUSE
	CONFIGURE_ARGS += --with-fuse=external
	PKG_RELEASE:=$(PKG_RELEASE)-fuseext
else
	CONFIGURE_ARGS += --with-fuse=internal
	TARGET_CPPFLAGS:=-I../include/fuse-lite $(TARGET_CPPFLAGS) 
	PKG_RELEASE:=$(PKG_RELEASE)-fuseint
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/.libs/ntfs-3g $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libntfs-3g/.libs/libntfs-3g.{a,so*} $(1)/usr/lib/
endef

define Package/ntfs-3g/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/.libs/ntfs-3g $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libntfs-3g/.libs/libntfs-3g.so.* $(1)/usr/lib/
endef

define Package/ntfs-3g-utils/install
	$(INSTALL_DIR) $(1)/usr/bin; \
	/usr/bin/find $(PKG_BUILD_DIR)/ipkg-lantiq/ntfs-3g/usr/bin/ -type f ! -name ntfs-3g.probe ! -name ntfs-3g -exec $(INSTALL_BIN) {} $(1)/usr/bin/ \;
endef

$(eval $(call BuildPackage,ntfs-3g))
$(eval $(call BuildPackage,ntfs-3g-utils))
