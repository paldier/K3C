# 
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
PKG_VERSION:=1.17.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.busybox.net/downloads \
		http://distfiles.gentoo.org/distfiles/
PKG_MD5SUM:=c7fe7533b7fc4018b0b49a05ee0ee601
PKG_BUILD_DEPENDS:=PACKAGE_ltq-mapi:ltq-mapi
include $(INCLUDE_DIR)/package.mk

IFX_APIS_DIR:=$(wildcard $(BUILD_DIR)/ltq-mapi*)
IFX_HTTPD_DIR:=$(wildcard $(BUILD_DIR)/ltq-webcgi*)

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) grep '^CONFIG_BUSYBOX_' $(TOPDIR)/.config | md5s)
endif

init-y :=
init-$(CONFIG_BUSYBOX_CONFIG_HTTPD) += httpd
init-$(CONFIG_BUSYBOX_CONFIG_CROND) += cron
#init-$(CONFIG_BUSYBOX_CONFIG_TELNETD) += telnet

define Package/busybox
  SECTION:=base
  CATEGORY:=Base system
  MAINTAINER:=Nicolas Thill <nico@openwrt.org>
  TITLE:=Core utilities for embedded Linux
  URL:=http://busybox.net/
  DEPENDS:=+BUSYBOX_CONFIG_FEATURE_HAVE_RPC:librpc
  MENU:=1
endef

define Package/busybox/description
 The Swiss Army Knife of embedded Linux.
 It slices, it dices, it makes Julian Fries.
endef

define Package/busybox/config
	menu "Configuration"
		depends on PACKAGE_busybox
		source "$(SOURCE)/config/Config.in"
	endmenu
endef

BB_CONFIG:=$(PKG_BUILD_DIR)/.config
define enable_config
	sed -i "/$(1)/d" $(BB_CONFIG); \
	echo "$(1)=y" >> $(BB_CONFIG) 
endef

define disable_config
	sed -i "/$(1)/d" $(BB_CONFIG); \
	echo "# $(1) is not set" >> $(BB_CONFIG) 
endef

define remove_config
	sed -i "/$(1)/d" $(BB_CONFIG);\
	sed -i "/# $(1) is not set/d" $(BB_CONFIG)
endef

define set_config
	$(call remove_config,$(1)); \
	echo "$(1)=\"$(2)"\" >> $(BB_CONFIG) 
endef

define set_config_val
        $(call remove_config,$(1)); \
        echo $(1)=$(2) >> $(BB_CONFIG)
endef

define busybox_always_on_config
	$(if $(CONFIG_FEATURE_INETD), \
                $(call enable_config,CONFIG_INETD); \
                $(call disable_config,CONFIG_FEATURE_INETD_SUPPORT_BUILTIN_ECHO); \
                $(call disable_config,CONFIG_FEATURE_INETD_SUPPORT_BUILTIN_DISCARD); \
                $(call disable_config,CONFIG_FEATURE_INETD_SUPPORT_BUILTIN_TIME); \
                $(call disable_config,CONFIG_FEATURE_INETD_SUPPORT_BUILTIN_DAYTIME); \
                $(call disable_config,CONFIG_FEATURE_INETD_SUPPORT_BUILTIN_CHARGEN); \
                $(call enable_config,CONFIG_FEATURE_INETD_CLASS_SUPPORT); \
                $(call enable_config,CONFIG_FEATURE_IPV6); \
	, \
                $(call disable_config,CONFIG_INETD))
	$(if $(CONFIG_FEATURE_DHCP_SERVER), \
                $(call enable_config,CONFIG_UDHCPD); \
                $(call enable_config,CONFIG_DUMPLEASES); \
                $(call enable_config,CONFIG_FEATURE_DHCP_USER_CLASS); \
                $(call enable_config,CONFIG_FEATURE_DHCP_TR111_DEVICE_ASSOCIATION); \
                $(call enable_config,CONFIG_FEATURE_DHCP_TR69_HOSTS_OBJ); \
                $(call enable_config,CONFIG_FEATURE_UDHCPD_WRITE_LEASES_EARLY); \
                $(call enable_config,CONFIG_FEATURE_UDHCPD_RELAY); \
		$(call set_config,CONFIG_DHCPD_LEASES_FILE,"/var/lib/misc/udhcpd.leases"); \
	, \
                $(call disable_config,CONFIG_UDHCPD))
	$(if $(CONFIG_FEATURE_DHCP_CLIENT), \
                $(call enable_config,CONFIG_UDHCPC); \
                $(call enable_config,CONFIG_FEATURE_UDHCPC_ARPING); \
		$(call set_config,CONFIG_UDHCPC_DEFAULT_SCRIPT,"/etc/rc.d/udhcpc.script"); \
		$(call set_config_val,CONFIG_UDHCPC_SLACK_FOR_BUGGY_SERVERS,924); \
		$(call enable_config,CONFIG_FEATURE_UDHCP_RFC5969); \
	, \
                $(call disable_config,CONFIG_UDHCPC))
	$(if $(CONFIG_FEATURE_TELNET_SERVER), \
                $(call enable_config,CONFIG_TELNETD); \
                $(call enable_config,CONFIG_FEATURE_TELNETD_STANDALONE); \
                $(call enable_config,CONFIG_FEATURE_TELNETD_INETD_WAIT); \
                $(call enable_config,CONFIG_FEATURE_TELNETD_INACTIVE_TIMEOUT); \
		$(call enable_config,CONFIG_FEATURE_DEVPTS); \
		, \
                $(call disable_config,CONFIG_TELNETD))
	$(if $(CONFIG_FEATURE_TFTP_SERVER), \
                $(call enable_config,CONFIG_TFTP); \
                $(call enable_config,CONFIG_TFTPD); \
                $(call enable_config,CONFIG_FEATURE_TFTP_UPGRADE_IMAGE); \
                $(call enable_config,CONFIG_FEATURE_TFTP_GET); \
                $(call enable_config,CONFIG_FEATURE_TFTP_PUT); \
		$(call disable_config,CONFIG_TFTP_DEBUG); \
		, \
                $(call disable_config,CONFIG_TFTPD))
	$(if $(CONFIG_FEATURE_DHCP_RELAY), \
                $(call enable_config,CONFIG_DHCPRELAY); \
	, \
                $(call disable_config,CONFIG_DHCPRELAY))
	$(if $(CONFIG_FEATURE_DEVM_DEVICE_ASSOCIATION), \
                $(call enable_config,CONFIG_FEATURE_DHCP_TR111_DEVICE_ASSOCIATION); \
	, \
                $(call disable_config,CONFIG_FEATURE_DHCP_TR111_DEVICE_ASSOCIATION))
	$(if $(CONFIG_FEATURE_DEVM_HOSTS_OBJ), \
                $(call enable_config,CONFIG_FEATURE_DHCP_TR69_HOSTS_OBJ); \
	, \
                $(call disable_config,CONFIG_FEATURE_DHCP_TR69_HOSTS_OBJ))
	$(if $(CONFIG_FEATURE_SYSTEM_LOG), \
                $(call enable_config,CONFIG_FEATURE_ROTATE_LOGFILE); \
                $(call enable_config,CONFIG_FEATURE_REMOTE_LOG); \
		$(call set_config_val,CONFIG_FEATURE_SYSLOGD_READ_BUFFER_SIZE,256); \
		$(call disable_config,CONFIG_SYSLOGD); \
                $(call disable_config,CONFIG_FEATURE_SYSLOGD_DUP); \
                $(call disable_config,CONFIG_LOGREAD); \
                $(call disable_config,CONFIG_FEATURE_IPC_SYSLOG); \
                $(call disable_config,CONFIG_KLOGD); \
                $(call disable_config,CONFIG_LOGGER); \
	, \
		$(call disable_config,CONFIG_SYSLOGD); \
		$(call disable_config,CONFIG_LOGGER); \
                $(call disable_config,CONFIG_FEATURE_UDHCP_SYSLOG))
	$(call set_config_val,CONFIG_FEATURE_EDITING_HISTORY,15)
	$(call disable_config,CONFIG_FEATURE_PRESERVE_HARDLINKS)
	$(call enable_config,CONFIG_ASH)
	$(call disable_config,CONFIG_ASH_BUILTIN_ECHO)
	$(call enable_config,CONFIG_ASH_BASH_COMPAT)
	$(call enable_config,CONFIG_ASH_JOB_CONTROL)
	$(call enable_config,CONFIG_SH_MATH_SUPPORT)
	$(call enable_config,CONFIG_ASH_CMDCMD)
	$(call enable_config,CONFIG_ASH_BUILTIN_PRINTF)
	$(call enable_config,CONFIG_ASH_BUILTIN_TEST)
	$(call enable_config,CONFIG_FEATURE_TEST_64)
	$(call enable_config,CONFIG_ASH_OPTIMIZE_FOR_SIZE)
	$(call enable_config,CONFIG_ASH_EXPAND_PRMT)
	$(call enable_config,CONFIG_ASH_ALIAS)
	$(call disable_config,CONFIG_SH_MATH_SUPPORT_64)
	$(call disable_config,CONFIG_ASH_GETOPTS)
	$(call disable_config,CONFIG_ASH_MAIL)
	$(call disable_config,CONFIG_ASH_RANDOM_SUPPORT)
	$(call disable_config,CONFIG_FEATURE_SH_EXTRA_QUIET)
	$(call disable_config,CONFIG_FEATURE_SH_STANDALONE)
	$(call disable_config,CONFIG_FEATURE_EDITING_SAVEHISTORY)
	$(call enable_config,CONFIG_FEATURE_SH_IS_ASH)
	$(call disable_config,CONFIG_FEATURE_SH_IS_NONE)
	$(call enable_config,CONFIG_FEATURE_BASH_IS_NONE)
	
	$(call enable_config,CONFIG_FEATURE_EDITING)
	$(call set_config_val,CONFIG_FEATURE_EDITING_MAX_LEN,512)
	$(call enable_config,CONFIG_FEATURE_EDITING_VI)
	$(call set_config_val,CONFIG_FEATURE_EDITING_HISTORY,10)
	$(call disable_config,CONFIG_FEATURE_EDITING_SAVEHISTORY)
	$(call enable_config,CONFIG_FEATURE_EDITING_FANCY_PROMPT)
	$(call disable_config,CONFIG_FEATURE_EDITING_ASK_TERMINAL)
	$(call enable_config,CONFIG_FEATURE_TAB_COMPLETION)
	$(call disable_config,CONFIG_FEATURE_USERNAME_COMPLETION)
	$(call enable_config,CONFIG_DF)
	$(call enable_config,CONFIG_FEATURE_PIDFILE)
	$(call enable_config,CONFIG_SHOW_USAGE)
	$(call enable_config,CONFIG_FEATURE_VERBOSE_USAGE)
endef

define install_busybox_new
	mkdir -p $(1)
	cd $(PKG_BUILD_DIR); \
	./applets/ifx_install.sh $(1)
endef

define Build/Configure
	rm -f $(PKG_BUILD_DIR)/.configured*
	grep 'CONFIG_BUSYBOX_' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_BUSYBOX_\\(.*\\),\\1\\2,g" > $(PKG_BUILD_DIR)/.config
	yes '' | $(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		ARCH="$(ARCH)" \
		oldconfig
	$(CP) $(BB_CONFIG) $(BB_CONFIG)_busybox
endef

ifdef CONFIG_GCC_VERSION_LLVM
  TARGET_CFLAGS += -fnested-functions
endif

ifdef CONFIG_GCC_VERSION_4_5_0
  TARGET_CFLAGS += -fno-tree-pta
endif

IFX_INCLUDE = -I$(STAGING_DIR)/usr/include

ifeq ($(CONFIG_FEATURE_DEVM_DEVICE_ASSOCIATION),y)
	IFX_INCLUDE += -I$(IFX_APIS_DIR)/include
endif

ifeq ($(CONFIG_FEATURE_DEVM_DEVICE_ASSOCIATION),y)
  TARGET_LDFLAGS += -ldl
endif
LDLIBS:=m crypt
ifdef CONFIG_BUSYBOX_CONFIG_FEATURE_HAVE_RPC
  TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
  export LDFLAGS=$(TARGET_LDFLAGS)
  LDLIBS += rpc
endif

define Build/Compile_split
	rm -rf $(PKG_INSTALL_DIR)
	$(FIND) $(PKG_BUILD_DIR) -lname "*busybox" -exec rm \{\} \;
	yes '' | $(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		ARCH="$(ARCH)" \
		allnoconfig
	$(busybox_always_on_config)
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		EXTRA_CFLAGS="$(TARGET_CFLAGS) $(IFX_INCLUDE)" \
		ARCH="$(ARCH)" \
		SKIP_STRIP=y \
		LDLIBS="$(LDLIBS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	$(CP) $(BB_CONFIG) $(BB_CONFIG)_smaller
	$(CP) $(PKG_BUILD_DIR)/busybox $(PKG_BUILD_DIR)/busybox2
	chmod +x $(PKG_BUILD_DIR)/applets/ifx_install.sh
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		EXTRA_CFLAGS="$(TARGET_CFLAGS) $(IFX_INCLUDE)" \
		ARCH="$(ARCH)" \
		SKIP_STRIP=y \
		LDLIBS="$(LDLIBS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		busybox.links
	$(call install_busybox_new,$(PKG_INSTALL_DIR))
	$(CP) $(BB_CONFIG)_busybox $(BB_CONFIG)
	-rm -f $(PKG_BUILD_DIR)/busybox.links
	yes '' | $(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		ARCH="$(ARCH)" \
		oldconfig
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		EXTRA_CFLAGS="$(TARGET_CFLAGS) $(IFX_INCLUDE)" \
		ARCH="$(ARCH)" \
		LDLIBS="$(LDLIBS)" \
		SKIP_STRIP=y \
		all
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS) $(IFX_INCLUDE)" \
		ARCH="$(ARCH)" \
		LDLIBS="$(LDLIBS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		install
endef

define Build/Compile_regular
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		SKIP_STRIP=y \
		LDLIBS="$(LDLIBS)" \
		$(BB_MAKE_VERBOSE) \
		all
	rm -rf $(PKG_INSTALL_DIR)
	$(FIND) $(PKG_BUILD_DIR) -lname "*busybox" -exec rm \{\} \;
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		LDLIBS="$(LDLIBS)" \
		$(BB_MAKE_VERBOSE) \
		install
endef

define Build/Compile
	$(if $(CONFIG_BUSYBOX_CONFIG_FEATURE_SPLIT),
		$(call Build/Compile_split),$(call Build/Compile_regular))
endef

define Package/busybox/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
	for tmp in $(init-y); do \
		$(INSTALL_BIN) ./files/$$$$tmp $(1)/etc/init.d/$$$$tmp; \
	done
	-rm -rf $(1)/lib64
endef

$(eval $(call BuildPackage,busybox))
