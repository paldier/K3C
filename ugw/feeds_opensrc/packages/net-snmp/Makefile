#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=net-snmp
PKG_VERSION:=5.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://localhost
PKG_MD5SUM:=14217471edb2b805b0e28c4c3cfd8c75
PKG_FIXUP=libtool
PKG_NO_AUTORECONF=y
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(TOPDIR)/../../../wlnsw/wireline_sw_linux26/open_snmp
PKG_BUILD_DEPENDS:=ifx-oam ltq-mapi 

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk

define Package/$(PKG_NAME)
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Net SNMP Support
  URL:=http://localhost
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  Net SNMP Support
endef

define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
	config FEATURE_SNMPv1
		default n
		depends on PACKAGE_$(PKG_NAME)
		bool 'Support for SNMPV1 and V2'
	config FEATURE_SNMPv3
		default n
		depends on PACKAGE_$(PKG_NAME)
		bool 'Support for SNMPV3'
endef
#	config FEATURE_SNMP_ATM_MIB
#		default y
#		depends on FEATURE_SNMPv1 || FEATURE_SNMPv3
#		bool 'ATM MIB Support'
#	config FEATURE_SNMP_ADSL_MIB
#		default n
#		depends on FEATURE_SNMPv1 || FEATURE_SNMPv3 
#		bool 'ADSL MIB Support'


ifeq ($(CONFIG_TARGET_ifxcpe_platform_amazon_se),"AMAZON")
	IFX_CFLAGS = -DCPU_AMAZON
endif

IFX_CFLAGS += -DBUILD_FROM_LTQ_APPS

IFX_CONFIG_MIB_MODULES = mibII
ifeq ($(CONFIG_FEATURE_SNMPv3),y)
	export CONFIG_FEATURE_SNMPv3=y
	IFX_CONFIG_MIB_MODULES += snmpv3mibs
endif

ifeq ($(CONFIG_FEATURE_SNMP_ATM_MIB),y)
	IFX_CONFIG_MIB_MODULES += atmMIB
	IFX_CFLAGS+= -DCONFIG_FEATURE_SNMP_ATM_MIB 
endif

ifeq ($(CONFIG_FEATURE_SNMP_ADSL_MIB),y)
	IFX_CONFIG_MIB_MODULES += adslMIB
	IFX_CFLAGS+= -DCONFIG_FEAURE_SNMP_ADSL_MIB
	IFX_CFLAGS+= -DAMAZON_MEI_MIB_RFC3440 
endif
	
#ifeq ($(CONFIG_IFX_SNMP_TRANSPORT_ATMPVC_DOMAIN),y)
#  ifeq ($(CONFIG_IFX_SNMP_TRANSPORT_EOC_DOMAIN),y)
#	IFX_CONFIG_SNMP_TRANSPORTS="${IFX_CONFIG_SNMP_TRANSPORTS} AAL5PVC EOC"
#  else  
#	IFX_CONFIG_SNMP_TRANSPORTS="${IFX_CONFIG_SNMP_TRANSPORTS} AAL5PVC"
#  endif
#endif
PKG_INST_DIR:=$(PKG_BUILD_DIR)/ipkg/$(PKG_NAME)/
CONFIGURE_VARS += \
	IFX_CFLAGS="$(IFX_CFLAGS)" IFX_LDFLAGS="$(IFX_LDFLAGS)" \
	BUILD_ROOTFS_DIR="$(PKG_INST_DIR)" IFX_CONFIG_SNMP_TRANSPORTS="$(IFX_CONFIG_SNMP_TRANSPORTS)" \
	IFX_CONFIG_MIB_MODULES="${IFX_CONFIG_MIB_MODULES}" 

ifeq ($(CONFIG_FEATURE_SNMPv1),y)
	CONFIGURE_ARGS += '--host=mips-hardhat-linux' CC=mips-linux-gcc 'CFLAGS=-Os -mips2 -mtune=r4600 -DAmazon' '--with-endianness=big' 'CPP=mips-linux-cpp' 'host_alias=mips-hardhat-linux' --disable-debugging --disable-mib-loading --with-out-mib-modules="snmpv3mibs agentx utilities agent_mibs ucd_snmp notification target" --with-default-snmp-version=1 --with-out-transports="Callback UDPIPv6 TCPIPv6 TCP Unix IPX" --disable-manuals --disable-scripts --disable-applications --disable-mibs --disable-privacy --prefix=  --with-install-prefix=${PKG_INST_DIR} --with-sys-contact="root@infineon" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp" --with-sys-location="/"
  ifeq ($(CONFIG_PACKAGE_ltq-mapi),y)
	CONFIGURE_ARGS += --with-mib-modules="${IFX_CONFIG_MIB_MODULES}" --with-transports="UDP ${IFX_CONFIG_SNMP_TRANSPORTS}"
  endif
endif

ifeq ($(CONFIG_FEATURE_SNMPv3),y)
	CONFIGURE_ARGS += '--host=mips-hardhat-linux' CC=mips-linux-gcc 'CFLAGS=-Os -mips2 -mtune=r4600 -DAmazon' '--with-endianness=big' 'CPP=mips-linux-cpp' 'host_alias=mips-hardhat-linux' --disable-debugging --disable-mib-loading --with-out-mib-modules="agentx utilities agent_mibs ucd_snmp notification target" --with-default-snmp-version=3 --with-out-transports="Callback UDPIPv6 TCPIPv6 TCP Unix IPX" --disable-manuals --disable-scripts --disable-applications --disable-mibs --prefix=  --with-install-prefix=${PKG_INST_DIR} --with-openssl="../openssl-0.9.8a" --with-sys-contact="root@infineon" --with-logfile="/var/log/snmpd.log" --with-persistent-directory="/var/net-snmp" --with-sys-location="/"
  ifeq ($(CONFIG_PACKAGE_ltq-mapi),y)
	CONFIGURE_ARGS += --with-mib-modules="${IFX_CONFIG_MIB_MODULES}" --with-transports="UDP ${IFX_CONFIG_SNMP_TRANSPORTS}"
  endif
	IFX_CFLAGS+=-DCONFIG_FEATURE_SNMPv3 
endif

define Build/Configure
	touch $(PKG_BUILD_DIR)/stamp-h.in
	(cd $(PKG_BUILD_DIR)/; $(CP) configure_full configure; $(CP) Makefile.rules.full Makefile.rules; chmod u+x configure)
	(cd $(PKG_BUILD_DIR)/; \
	if [ -x $(CONFIGURE_CMD) ]; then \
		$(CONFIGURE_VARS) \
		$(CONFIGURE_CMD) \
		$(CONFIGURE_ARGS); \
	fi; \
	)
endef


IFX_APIS_DIR:=$(wildcard $(BUILD_DIR)/ltq-mapi*)
IFX_DSL_CPE_API_INCLUDES:=$(STAGING_DIR)/usr/include/drv_dsl_cpe_api/

IFX_INCLUDE += -I$(IFX_APIS_DIR)/include  -I$(IFX_DSL_CPE_API_INCLUDES)

ifeq ($(CONFIG_PACKAGE_dsl-cpe-api-vrx),y)
  IFX_INCLUDE += -DINCLUDE_DSL_CPE_API_VRX
  IFX_INCLUDE += -DCONFIG_PACKAGE_DSL_CPE_API
else
  ifeq ($(CONFIG_PACKAGE_dsl-cpe-api-danube),y)
    IFX_INCLUDE += -DINCLUDE_DSL_CPE_API_DANUBE
    IFX_INCLUDE += -DCONFIG_PACKAGE_DSL_CPE_API
  endif
endif


export CONFIG_PACKAGE_ltq-mapi

ifeq ($(CONFIG_PACKAGE_ltq-mapi),y)
  EXTRA_LDFLAGS = -L$(IFX_APIS_DIR)/lib -Wl,-rpath-link,$(IFX_APIS_DIR)/lib -L$(STAGING_DIR)/usr/lib/ -loamapi -lIFXAPIs
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) IFX_CFLAGS="$(IFX_CFLAGS) $(IFX_INCLUDE)" IFX_LDFLAGS="$(IFX_LDFLAGS) $(EXTRA_LDFLAGS)" all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(MAKE) -C $(PKG_BUILD_DIR)/agent $(TARGET_CONFIGURE_OPTS) IFX_CFLAGS="$(IFX_CFLAGS) $(IFX_INCLUDE)" IFX_LDFLAGS="$(IFX_LDFLAGS)" install
	$(MAKE) -C $(PKG_BUILD_DIR)/snmplib $(TARGET_CONFIGURE_OPTS) IFX_CFLAGS="$(IFX_CFLAGS) $(IFX_INCLUDE)" IFX_LDFLAGS="$(IFX_LDFLAGS)" install
	$(CP) $(PKG_BUILD_DIR)/agent/snmpd $(1)/usr/sbin
	rm -rf $(PKG_INST_DIR)/usr/lib/libnetsnmp*
endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	rm -f ${PKG_INST_DIR}/usr/lib/libnetsnmp*; \
	rm -f ${PKG_INST_DIR}/usr/sbin/snmp*; \
	$(MAKE) -C $(PKG_BUILD_DIR) distclean; \
	$(RM) $(PKG_BUILD_DIR)/ipk; fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
