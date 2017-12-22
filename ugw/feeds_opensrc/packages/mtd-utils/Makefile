# 
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_REV:=36d8de81049c9c908740b690c664b5bd3703ccd6
PKG_VERSION:=1.4.6
PKG_RELEASE:=1
PKG_INSTALL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=git://git.infradead.org/mtd-utils.git
PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=http://localhost/
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

include $(INCLUDE_DIR)/package.mk

FILES:= \
	ubiupdatevol \
	flash_otp_info \
	nandtest \
	ubimkvol \
	ubigen \
	serve_image \
	recv_image \
	ubinfo \
	ubirename \
	mkpfi \
	nandwrite \
	flash_eraseall \
	mkbootenv \
	ftl_format \
	nand2bin \
	ubiattach \
	nftl_format \
	nanddump \
	ubinize \
	docfdisk \
	ubimirror \
	ubicrc32.pl \
	ftl_check \
	flashcp \
	jffs2dump \
	ubiformat \
	ubidetach \
	flash_lock \
	bin2nand \
	pfiflash \
	mtd_debug \
	pddcustomize \
	flash_unlock \
	doc_loadbios \
	nftldump \
	mkfs.jffs2 \
	ubicrc32 \
	ubirmvol \
	unubi \
	pfi2bin \
	mkfs.ubifs \
	sumtool \
	flash_erase \
	rfddump \
	flash_otp_dump \
	rfdformat

#flash_info

define PartGen
define Package/mtd-utils-$(subst _,-,$(1))
  TITLE:=MTD $(1)
  URL:=http://www.linux-mtd.infradead.org/
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=mtd-utils +zlib +libuuid
endef
endef
#+liblzo

define Package/mtd-utils
  TITLE:=Utilities for flash info/debug
  SECTION:=utils
  CATEGORY:=Utilities
  MENU:=1
endef

define Package/mtd-utils/description
  Utilities for manipulating memory technology devices.
endef

define Package/mtd-utils/install
	true
endef

$(foreach file,$(FILES),$(eval $(call PartGen,$(file))))

MAKE_FLAGS += \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	BUILDDIR="$(PKG_BUILD_DIR)" \
	LDLIBS+="$(LIBGCC_S) -L$(PKG_BUILD_DIR)/lib -I$(PKG_BUILD_DIR)/include -lmtd" \
	WITHOUT_LZO=1 WITHOUT_XATTR=1

define PartInstall
define Package/mtd-utils-$(subst _,-,$(1))/install
	$(INSTALL_DIR) \
		$$(1)/usr/sbin

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/$(1) \
		$$(1)/usr/sbin/
endef
endef

$(foreach file,$(FILES),$(eval $(call PartInstall,$(file))))

$(eval $(call BuildPackage,mtd-utils))
$(foreach file,$(FILES),$(eval $(call BuildPackage,mtd-utils-$(subst _,-,$(file)))))
