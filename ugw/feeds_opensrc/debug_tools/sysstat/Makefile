#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sysstat
PKG_VERSION:=9.0.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://pagesperso-orange.fr/sebastien.godard
PKG_MD5SUM:=e069daf192d259de905d7f8cdd85dc4e


include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) grep '^CONFIG_FEATURE_SYSSTAT' $(TOPDIR)/.config | md5s)
endif

define Package/sysstat
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Sysstat performance monitoring tools
  URL:=http://pagesperso-orange.fr/sebastien.godard/index.html
  MENU:=1
endef

define Package/sysstat/description
  The sysstat utilities are a collection of performance monitoring tools for
  Linux. These include sar, sadf, mpstat, iostat, pidstat and sa tools.
endef

define Package/$(PKG_NAME)/config
  config FEATURE_SYSSTAT_IOSTAT
  depends on PACKAGE_$(PKG_NAME)
    bool 'iostat'

  config FEATURE_SYSSTAT_MPSTAT
    depends on PACKAGE_$(PKG_NAME)
    bool 'mpstat'
    default y

  config FEATURE_SYSSTAT_PIDSTAT
    depends on PACKAGE_$(PKG_NAME)
    bool 'pidstat'

  config FEATURE_SYSSTAT_SADF
    depends on PACKAGE_$(PKG_NAME)
    bool 'sadf'

  config FEATURE_SYSSTAT_SAR
    depends on PACKAGE_$(PKG_NAME)
    bool 'sar'

endef

ifeq ($(CONFIG_FEATURE_SYSSTAT_IOSTAT),y)
  SYSSTAT_BINS += iostat
endif
ifeq ($(CONFIG_FEATURE_SYSSTAT_MPSTAT),y)
  SYSSTAT_BINS += mpstat
endif
ifeq ($(CONFIG_FEATURE_SYSSTAT_PIDSTAT),y)
  SYSSTAT_BINS += pidstat
endif
ifeq ($(CONFIG_FEATURE_SYSSTAT_SADF),y)
  SYSSTAT_BINS += sadf
endif
ifeq ($(CONFIG_FEATURE_SYSSTAT_SAR),y)
  SYSSTAT_BINS += sar
endif

CONFIGURE_ARGS+= \
	sa_lib_dir="/usr/lib/sysstat" \
	sa_dir="/var/log/sysstat" \
	conf_dir="/etc/sysstat"

CONFIGURE_ARGS+= \
	--disable-documentation

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) $(SYSSTAT_BINS)
endef

define install_sysstat_bin
	for sysstatbin in $(SYSSTAT_BINS); do \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/$$sysstatbin $(1)/usr/bin/; \
	done
endef

define Package/sysstat/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(RM) $(1)/usr/bin/*
	$$(call install_sysstat_bin,$(1))
endef

$(eval $(call BuildPackage,sysstat))
