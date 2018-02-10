#
# Openwrt Makefile for libwlan
#
#

#### Includes ###################################
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

#### Package Info ############################### 
PKG_NAME:=libwlan
PKG_VERSION:=1.1.0.1
PKG_RELEASE:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_DEFAULT_CUSTOM_SOURCE_DIR:=$(shell pwd)/src

#PKG_CONFIG_DEPENDS:=\
#	CONFIG_FEATURE_WLAN

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/package-version-override.mk
include $(INCLUDE_DIR)/ugw-loglevel.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Lantiq
  SUBMENU:=UGW Software
  TITLE:=Lantiq WLAN SL
  URL:=http://www.lantiq.com
  MAINTAINER:=Lantiq Beteiligungs-GmbH & Co. KG
  DEPENDS:= +libugwhelper +libcal +csd +servd +scapi +fapi_wlan_common
  MENU:=1
endef

define Package/$(PKG_NAME)/description
	Lantiq SL for WLAN Support
endef

### Menuconfig ##################################
define Package/$(PKG_NAME)/config
$(call Package/$(PKG_NAME)/override_version)
$(call Package/$(PKG_NAME)/override_source_path)
$(call Package/$(PKG_NAME)/config_debug_level)
endef

#### Target Rules ###############################
define Build/Prepare

endef

# To Staging Dir
define Build/InstallDev
	$(INSTALL_DIR) $(1)/xml
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/www
	$(if $(CONFIG_TARGET_lantiq_xrx200), \
			$(INSTALL_DATA) ./files/vrx220_id_added_xml/*.xml $(1)/xml/;\
			$(INSTALL_DATA) ./files/vrx220_id_added_xml/*.json $(1)/$(VENDOR_PATH)/www/;\
						rm -f $(1)/$(VENDOR_PATH)/www/wifi2.4_debug_menu.json;\
						rm -f $(1)/$(VENDOR_PATH)/www/wifi5_debug_menu.json;\
						rm -f $(1)/$(VENDOR_PATH)/www/wifi5_2_debug_menu.json;\
			,\
			$(if $(CONFIG_BEEROCKS_MODE_IRE), \
				$(INSTALL_DATA) ./files/id_added_xml/WiFi_ire_data.xml $(1)/xml/WiFi_data.xml;\
			,\
				$(INSTALL_DATA) ./files/id_added_xml/WiFi_gw_data.xml $(1)/xml/WiFi_data.xml;\
			)
			$(INSTALL_DATA) ./files/id_added_xml/WiFi_control.xml $(1)/xml/;\
			$(INSTALL_DATA) ./files/id_added_xml/*.json $(1)/$(VENDOR_PATH)/www/;\
	)
endef

# To ipkg / Rootfs
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/wave/db
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/www
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/www/languages/en
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/www/languages/cn
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/servd/lib
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/xml
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/cli
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_DIR) $(1)/$(VENDOR_PATH)/cli/map-files/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libwlan.so $(1)/$(VENDOR_PATH)/servd/lib/
	cd $(1)/usr/lib && ln -sf ../../$(VENDOR_PATH)/servd/lib/libwlan.so
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sl_wlan_test $(1)/usr/sbin
	$(if $(CONFIG_TARGET_lantiq_xrx200), \
			$(INSTALL_DATA) ./files/vrx220_id_added_xml/*.xml $(1)/$(VENDOR_PATH)/xml/; \
			$(INSTALL_DATA) ./files/vrx220_id_added_xml/*.json $(1)/$(VENDOR_PATH)/www/; \
             , \
			$(if $(CONFIG_BEEROCKS_MODE_IRE), \
				$(INSTALL_DATA) ./files/id_added_xml/WiFi_ire_data.xml $(1)/$(VENDOR_PATH)/xml/WiFi_data.xml; \
			,\
				$(INSTALL_DATA) ./files/id_added_xml/WiFi_gw_data.xml $(1)/$(VENDOR_PATH)/xml/WiFi_data.xml; \
			)
			$(INSTALL_DATA) ./files/id_added_xml/WiFi_control.xml $(1)/$(VENDOR_PATH)/xml/; \
			$(INSTALL_DATA) ./files/id_added_xml/*.json $(1)/$(VENDOR_PATH)/www/;\
	)
	$(INSTALL_DATA) ./files/cli-xml/*.xml $(1)/$(VENDOR_PATH)/cli/xmls/
	$(INSTALL_DATA) ./files/cli-map/*.map $(1)/$(VENDOR_PATH)/cli/map-files/
	#$(INSTALL_DATA) ./files/view_xml/*.xml $(1)/$(VENDOR_PATH)/www/
	python3 ./files/permutation/xmljsonparser.py -i files/permutation/Permutation.xml -o ./files/www/
	$(INSTALL_DATA) ./files/www/*.xml $(1)/$(VENDOR_PATH)/www/
	$(INSTALL_DATA) ./files/www/*.json $(1)/$(VENDOR_PATH)/www/
	$(if $(CONFIG_TARGET_lantiq_xrx200), \
		rm -f $(1)/$(VENDOR_PATH)/www/wifi2.4_debug_menu.json;\
		rm -f $(1)/$(VENDOR_PATH)/www/wifi5_debug_menu.json;\
		rm -f $(1)/$(VENDOR_PATH)/www/wifi5_2_debug_menu.json;\
	)

	$(INSTALL_DATA) ./files/www/languages/en/*.json $(1)/$(VENDOR_PATH)/www/languages/en
	$(INSTALL_DATA) ./files/www/languages/cn/*.json $(1)/$(VENDOR_PATH)/www/languages/cn

	$(INSTALL_DATA) ./files/www/*.html $(1)/$(VENDOR_PATH)/www/
	$(if $(CONFIG_BEEROCKS_MODE_IRE), \
		$(INSTALL_DATA) ./files/db_xml/WiFi_ire_data.xml $(1)/$(VENDOR_PATH)/wave/db/WiFi_data.xml; \
	,\
		$(INSTALL_DATA) ./files/db_xml/WiFi_gw_data.xml $(1)/$(VENDOR_PATH)/wave/db/WiFi_data.xml; \
	)
	$(INSTALL_DATA) ./files/db_xml/WiFi_control.xml $(1)/$(VENDOR_PATH)/wave/db

endef

define Build/Clean
	if [ -d $(PKG_BUILD_DIR) ]; then $(MAKE) -C $(PKG_BUILD_DIR) clean; \
	$(RM) -r $(PKG_BUILD_DIR)/ipkg-$(BOARD); fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
