#
# Copyright (C) 2008-2014 The LuCI Team <luci@lists.subsignal.org>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Firewall and Portforwarding application
#Remove dependency with fw3 as this will have a conflict wth UGW sl_firewall. Explicitly enabled fw3 for OpenWrt models.
#LUCI_DEPENDS:=+firewall

include ../../luci.mk

# call BuildPackage - OpenWrt buildroot signature
