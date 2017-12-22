#
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

BOARD:=intel-xway
BOARDNAME:=Intel GPON/XWAY/SVIP
FEATURES:=squashfs jffs2
DEFAULT_SUBTARGET:=danube
VENDOR_PATH="/opt/lantiq"

LINUX_VERSION:=3.10.104

CFLAGS=-Os -pipe -mips32r2 -mno-branch-likely

define Target/Description
	Build firmware images for Intel XWAY/GPON/SVIP SoCs
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))
