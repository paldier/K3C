# Copyright (C) 2018 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Luci Support for BonusCloud
LUCI_DEPENDS:=+libc +luci-lib-jsonc +wget +curl +liblzo +libcurl +libopenssl +libstdcpp +libltdl +ca-certificates +ca-bundle +ip6tables +kmod-ip6tables +kmod-ip6tables-extra +kmod-nf-ipt6 +ip6tables-mod-nat +ip6tables-extra +ip6tables-mod-nat +kmod-tun
LUCI_PKGARCH:=all
PKG_VERSION:=0.2.2-6o
PKG_RELEASE:=

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature


