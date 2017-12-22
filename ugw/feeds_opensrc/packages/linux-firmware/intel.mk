Package/iwl-firmware = $(call Package/firmware-default,Intel wireless firmware)
define Package/iwl-firmware/config
  if PACKAGE_iwl-firmware
	config IWL8000_FW
		bool "Intel Wi-Fi 8000 Series Firmware"
		default n
		help
		  Download and install firmware for:
		    Intel Wi-Fi Series 8265
	config IBT8000_FW
		bool "Intel BT 8000 Series Firmware"
		default n
		help
		  Download and install firmware for:
		    Intel BT Series 8265
  endif
endef
define Package/iwl-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DIR) $(1)/lib/firmware/intel
ifneq ($(CONFIG_IWL8000_FW),)
	$(INSTALL_DATA) ./fw/iwlwifi-8265-27.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IBT8000_FW),)
	$(INSTALL_DATA) ./fw/ibt-11-16.sfi $(1)/lib/firmware/intel
	$(INSTALL_DATA) ./fw/ibt-11-16.ddc $(1)/lib/firmware/intel
endif
endef
$(eval $(call BuildPackage,iwl-firmware))
