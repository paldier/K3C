#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

VPE_MENU:=MIPS VPE support

define KernelPackage/vpe
  SUBMENU:=$(OTHER_MENU)
  DEPENDS:=@mips
  TITLE:=Virtual processing engine support (in-kernel)
  KCONFIG:= CONFIG_MIPS_VPE_LOADER=y \
	  CONFIG_MIPS_VPE_APSP_API=n \
	  CONFIG_MIPS_VPE_LOADER_TOM=y \
	  CONFIG_MIPS_MT=y
endef

define KernelPackage/vpe/description
 Virtual processing engine support.
endef

$(eval $(call KernelPackage,vpe))
