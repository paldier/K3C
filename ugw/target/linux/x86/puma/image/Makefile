include $(TOPDIR)/rules.mk

UGW_VERSION:=UGW-$(shell cat $(TOPDIR)/ugw_version)
include $(INCLUDE_DIR)/image.mk
TIME_STAMP:=$(shell date +%s)

CPE_NAME:=$(shell echo $(SUBTARGET) | awk '{ print toupper($$1) }')
UGW_BANNER:=Lantiq UGW Software $(UGW_VERSION) on $(CPE_NAME) CPE

define put_ltq_banner
	b_str=" | $(UGW_BANNER) |"; \
	b_chars=`echo -n $$b_str | wc -m`; \
	(echo -n " +"; for ((i=2;i<$$b_chars;i++)); do echo -n "-"; done; echo "+"; \
	echo "$$b_str";echo -n " +";for ((i=2;i<$$b_chars;i++));do echo -n "-";done;echo -en "+\n\n") > $(1)
endef

define prepare_uts
	$(if $(BUILD_VERSION), \
		temp_suffix_tag=`echo $${BUILD_VERSION##built_}`;temp_branch=`echo $${temp_suffix_tag/_[0-9]*T[0-9]*/}`; \
		echo -en "BRANCH=$$temp_branch\nVERSION_ID=$$temp_suffix_tag\n" > $(TARGET_DIR)/etc/os-release \
	)
endef

# add '|| exit 1' to python commands to throw errors instead of warnings
# example
# python $(STAGING_DIR_HOST)/bin/merge.py -v $(STAGING_DIR_HOST)/bin/target_validate.xsd *_control*.xml -o=control.xml || exit 1;
define gen_xml_db
        $(if $(wildcard $(STAGING_DIR)/xml/*_control.xml), \
                $(if $(wildcard $(STAGING_DIR)/xml/*_data.xml), \
                       touch $(TARGET_IMG_DIR)/metadata.txt; \
                       echo "" >> $(TARGET_IMG_DIR)/metadata.txt; \
                       echo "hw_model=$(CONFIG_HW_MODEL_NAME)" >> $(TARGET_IMG_DIR)/metadata.txt; \
                       echo "hw_version=$(CONFIG_HW_MODEL_VERSION)" >>  $(TARGET_IMG_DIR)/metadata.txt; \
                       echo "sw_timestamp=$(TIME_STAMP)" >> $(TARGET_IMG_DIR)/metadata.txt; \
                        cd $(STAGING_DIR)/xml/ && { \
                                $(STAGING_DIR_HOST)/bin/python $(STAGING_DIR_HOST)/bin/merge.py -v $(STAGING_DIR_HOST)/bin/target_validate.xsd *_control*.xml -o=control.xml; \
                                $(STAGING_DIR_HOST)/bin/openssl aes-256-cbc -base64 -in control.xml -out $(TARGET_DIR)/etc/ctrlcfg -pass pass:SEINTELCHK ; \
                                $(STAGING_DIR_HOST)/bin/python $(STAGING_DIR_HOST)/bin/merge.py -v $(STAGING_DIR_HOST)/bin/target_validate.xsd *_data*.xml -o=data.xml; \
                                sed -i "s/\(<HwModelName.*>\).*\(<.*\)/\1$(CONFIG_HW_MODEL_NAME)\2/" data.xml;\
                                sed -i "s/\(<HwVersion.*>\).*\(<.*\)/\1$(CONFIG_HW_MODEL_VERSION)\2/" data.xml;\
                                sed -i "s/\(<SwUpdtTimeStamp.*>\).*\(<.*\)/\1$(TIME_STAMP)\2/" data.xml;\
                                $(STAGING_DIR_HOST)/bin/openssl aes-256-cbc -base64 -in data.xml -out $(TARGET_DIR)/etc/datacfg -pass pass:SEINTELCHK ;\
                        } \
                )
        )
	-rm -f $(TARGET_DIR)/$(call qstrip,$(VENDOR_PATH))/xml/*
endef


# Function to strip config macros from /etc/config.sh. Reference file used: tmp/ifx_config.sh
# Scan path: etc/ opt/ ramdisk_copy/ bin/ usr/ lib/
define strip_config_macros
	if [ -f $(TMP_DIR)/ifx_config.sh ]; then \
		scan_dirs="etc/ opt/ ramdisk_copy/ bin/ usr/ lib/"; \
		rm -rf $(TMP_DIR)/stripmacros; mkdir -p $(TMP_DIR)/stripmacros; \
		echo -en "\nStripping config macros from 'etc/config.sh'\nScanning: $$scan_dirs\n"; \
		echo -en "#\n# Automatically generated config for system scripts. Don't edit.\n#\n" \
			> $(TMP_DIR)/stripmacros/config.sh; \
		tot_macros=$$(grep export $(TMP_DIR)/ifx_config.sh | wc -l); \
		found=0; inc=0; cd $(TARGET_DIR)/; \
		fls="$$(find $$scan_dirs -type f | xargs grep -lI 'CONFIG_' | grep -v "etc\/config.sh" | sort -u)"; \
		grep export $(TMP_DIR)/ifx_config.sh | \
		  ( while read cfg_macro; do \
			cfgm=$$(echo $$cfg_macro | sed -e 's/export[ ]*//g' -e 's/=.*//g'); \
			val="$$(grep -wl $$cfgm $$fls)"; \
			if [ -n "$$val" ]; then \
				found=$$(expr $$found + 1); \
				echo "$$cfg_macro" >> $(TMP_DIR)/stripmacros/config.sh; \
			fi; inc=$$(expr $$inc + 1); \
			echo -en "Processing: $$inc/$$tot_macros macros. Used macros: $$found\r"; \
		  done ); echo -en "\n\n"; \
		if [ -f $(TMP_DIR)/stripmacros/config.sh ]; then \
		$(INSTALL_BIN) $(TMP_DIR)/stripmacros/config.sh $(TARGET_DIR)/etc/config.sh; fi; \
	else echo -en "\nERROR: Unable to find $(TMP_DIR)/ifx_config.sh for macro stripping!!\n\n"; fi
endef

define Image/mkfs/prepare/platform
	- @$$(call prepare_uts)
	- echo "$(UGW_VERSION)" > $(TARGET_DIR)/etc/version
	- date +%H%M-%d%b%y > $(TARGET_DIR)/etc/timestamp
	- date +%Y-%m-%dT%H:%M:%S > $(TARGET_DIR)/etc/ntp_timestamp
	- @$$(call put_ltq_banner,$(TARGET_DIR)/etc/banner)
	- $(if $(wildcard $(TOPDIR)/patches.list), install -m644 $(TOPDIR)/patches.list $(TARGET_DIR)/etc/)
	- rm -rf $(TARGET_DIR)/tmp
	- cd $(TARGET_DIR); ln -sf ramdisk/tmp tmp
	- $(CP) $(TMP_DIR)/ifx_config.sh $(TARGET_DIR)/etc/config.sh
	- $(if $(CONFIG_EXTERNAL_TOOLCHAIN),$(CP) $(TOOLCHAIN_ROOT_DIR)/lib/librt-*.so $(TOOLCHAIN_ROOT_DIR)/lib/libutil-*.so $(TARGET_DIR)/lib, \
	$(CP) $(TOOLCHAIN_DIR)/lib/librt-*.so $(TOOLCHAIN_DIR)/lib/libutil-*.so $(TARGET_DIR)/lib)
	- cat $(TOPDIR)/patches.list > $(TARGET_DIR)/etc/ugwinfo
	@$$(call strip_config_macros)
	@$(if $(CONFIG_NEW_FRAMEWORK),$(call gen_xml_db))
#As service libraries starting the netifd daemon, removing the startup script from /etc/init.d directory
	@$(if $(CONFIG_NEW_FRAMEWORK), rm -f $(TARGET_DIR)/etc/init.d/network)
#As ltq uses syslog-ng, logd daemon provided by opnsource utility - ubox is not required
	@$(if $(CONFIG_NEW_FRAMEWORK), rm -f $(TARGET_DIR)/etc/init.d/log)
	mkdir -p $(TARGET_IMG_DIR)
endef


define Image/Build/Profile/puma7
	$(INSTALL_DIR) $(TARGET_DIR)/$(MODULES_SUBDIR)
	$(INSTALL_BIN) $(LINUX_DIR)/drivers/net/ethernet/lantiq/switch-api/drv_switch_api.ko $(TARGET_DIR)/$(MODULES_SUBDIR)
	mkdir -p $(TARGET_ROOTFS_DIR)/tmp/
	$(CP) -a $(TARGET_DIR) $(TARGET_ROOTFS_DIR)/tmp/
	$(TOPDIR)/target/linux/x86/puma/profiles/puma7_uimage.sh "$(KDIR)/bzImage" "$(TARGET_ROOTFS_DIR)/tmp" "root-$(BOARD)" "$(TARGET_IMG_DIR)/$(IMG_PREFIX)-$(PROFILE)" "$(STAGING_DIR_HOST)"
endef

define BuildImage_platform
rootfs:
	$(call Image/mkfs/prepare)
ifeq ($(CONFIG_TARGET_ROOTFS_SQUASHFS),y)
	$(call Image/Build,squashfs)
endif
ifeq ($(CONFIG_TARGET_ROOTFS_EXT4FS),y)
	$(call Image/Build,ext4)
endif

fullimage:
	$(call Image/Build/Profile/puma7)

endef

$(eval $(call BuildImage))
$(eval $(call BuildImage_platform))
