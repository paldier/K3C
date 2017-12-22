include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

ifeq ($(CONFIG_LANTIQ_OPENWRT_FIRMWARE),y)
	include Makefile.openwrt
else
	include Makefile.ugw
endif
