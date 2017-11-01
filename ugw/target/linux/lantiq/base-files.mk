#
# Lantiq UGW rootfs addon for OpenWRT basefiles
#

ifeq ($(CONFIG_LANTIQ_OPENWRT_FIRMWARE),y)
define Package/base-files/install-target
	if [ -d $(PLATFORM_DIR)/base-files-openwrt/ ]; then \
		$(CP) -a $(PLATFORM_DIR)/base-files-openwrt/* $(1)/; \
	fi

	mkdir -p $(1)/overlay
	mkdir -p $(1)/mnt/data
	$(if $(CONFIG_TARGET_DATAFS_JFFS2),
		echo jffs2:$(CONFIG_TARGET_ROOTFS_DATAFS_SIZE) > $(1)/mnt/data/fs)
	$(if $(CONFIG_TARGET_DATAFS_UBIFS),
		echo ubifs:$(CONFIG_TARGET_ROOTFS_DATAFS_SIZE) > $(1)/mnt/data/fs)
	$(if $(CONFIG_TARGET_x86_puma_puma7), mkdir -p $(1)/nvram)
endef

else
PKG_FILE_DEPENDS += $(PLATFORM_DIR)/base-files-ugw/

define Package/base-files/install-target
	rm -f $(1)/var
	rm -f $(1)/etc/{hosts,fstab,services}
	rm -rf $(1)/tmp
	rm -rf $(1)/etc/init.d

ifdef CONFIG_NEW_FRAMEWORK
	if [ -d $(PLATFORM_DIR)/base-files-ugw-newframework/ ]; then \
		$(CP) $(PLATFORM_DIR)/base-files-ugw-newframework/* $(1)/;\
	fi
else
	rm -f $(1)/etc/passwd
	if [ -d $(PLATFORM_DIR)/base-files-ugw/ ]; then \
		$(CP) $(PLATFORM_DIR)/base-files-ugw/* $(1)/; \
	fi
endif
	
	if [ -d $(PLATFORM_SUBDIR)/base-files/. ]; then \
		$(CP) $(PLATFORM_SUBDIR)/base-files/* $(1)/; \
	fi

	if [ -d $(PLATFORM_SUBDIR)/$(PROFILE)/base-files/. ]; then \
		$(CP) $(PLATFORM_SUBDIR)/$(PROFILE)/base-files/* $(1)/; \
	fi

	if [ -z "$(CONFIG_NEW_FRAMEWORK)" ]; then \
		rm -rf $(1)/etc/profile.d/; \
	fi

	mkdir -p $(1)/{usr/bin,usr/lib,etc/dsl_api,etc/ppp,etc/init.d,etc/rc.d,proc,root,ramdisk,ramdisk_copy,mnt/overlay}
	mkdir -p $(1)/ramdisk/{tmp,var,flash}
	mkdir -p $(1)/ramdisk_copy/{var/tmp,var/run,var/log,var/lock,etc/Wireless,etc/dnrd,etc/ppp/peers,tftp_upload,usb}
	$(if $(CONFIG_TARGET_x86_puma_puma7), mkdir -p $(1)/nvram)

	-sed -i -e '/\/etc\/passwd/d' -e '/\/etc\/hosts/d' $(1)/CONTROL/conffiles

	mkdir -p $(1)/mnt/data; \
	$(if $(CONFIG_TARGET_DATAFS_JFFS2), echo jffs2 > $(1)/mnt/data/fs)
	$(if $(CONFIG_TARGET_DATAFS_UBIFS),	echo ubifs > $(1)/mnt/data/fs)
	$(if $(CONFIG_TARGET_UBI_MTD_SUPPORT),, rm -f $(1)/usr/sbin/vol_mgmt)

	cd $(1); \
		mkdir -p lib/firmware/$(LINUX_VERSION); \
		ln -sf lib/firmware/$(LINUX_VERSION) firmware; \
		ln -sf ramdisk/var var; \
		ln -sf ramdisk/flash flash; \
		ln -sf ramdisk/tmp tmp

	mkdir -p $(STAGING_DIR)/usr/include/
	$(if $(CONFIG_PACKAGE_ifx-utilities),cp -f $(PLATFORM_DIR)/base-files-ugw/etc/rc.conf $(STAGING_DIR)/usr/include/)

	mkdir -p $(1)/etc
	cd $(1)/etc; \
		rm -f rc.conf; \
		ln -sf ../ramdisk/etc/dnrd dnrd; \
		ln -sf ../ramdisk/etc/hostapd.conf hostapd.conf; \
		ln -sf ../ramdisk/etc/hosts hosts; \
		ln -sf ../ramdisk/etc/ilmid ilmid; \
		ln -sf ../ramdisk/etc/inetd.conf inetd.conf; \
		ln -sf ../ramdisk/etc/ipsec.conf ipsec.conf; \
		ln -sf ../ramdisk/etc/ipsec.secrets ipsec.secrets; \
		ln -sf ../proc/mounts mtab; \
		ln -sf ../ramdisk/flash/rc.conf rc.conf; \
		ln -sf ../ramdisk/etc/ripd.conf ripd.conf; \
		ln -sf ../ramdisk/etc/snmp snmp; \
		ln -sf ../ramdisk/etc/tr69 tr69; \
		ln -sf ../ramdisk/etc/udhcpd.conf udhcpd.conf; \
		ln -sf ../ramdisk/etc/zebra.conf zebra.conf; \
		$(if $(CONFIG_NEW_FRAMEWORK),\
			rm -f TZ resolv.conf; \
		,\
			ln -sf ../flash/passwd passwd; \
			ln -sf ../ramdisk/etc/resolv.conf resolv.conf; \
			ln -sf ../ramdisk/etc/TZ TZ; \
		) \
		ln -sf ../ramdisk/etc/services services;

	$(if $(CONFIG_NEW_FRAMEWORK),,\
	mkdir -p $(1)/etc/ppp; \
	cd $(1)/etc/ppp; \
		ln -sf ../../ramdisk/etc/ppp/options options; \
		ln -sf ../../ramdisk/etc/ppp/peers peers; \
		ln -sf ../../ramdisk/etc/ppp/resolv.conf resolv.conf)

	mkdir -p $(1)/etc/rc.d
	cd $(1)/etc/rc.d; \
		ln -sf ../init.d init.d; \
		ln -sf ../config.sh config.sh

	mkdir -p $(1)/mnt
	cd $(1)/mnt; \
		ln -sf ../ramdisk/usb usb

	cd $(1)/usr/sbin; \
		rm -f mini_fo.sh; ln -sf overlayfs.sh mini_fo.sh

	echo "#<< switch_ports" > $(1)/etc/switchports.conf
	echo "switch_mii1_port=\""$(CONFIG_SWITCH_MII1_PORT)"\"" >> $(1)/etc/switchports.conf
	echo "switch_lan_ports=\""$(CONFIG_SWITCH_LAN_PORTS)"\"" >> $(1)/etc/switchports.conf
	echo "#>> switch_ports" >> $(1)/etc/switchports.conf

	$(if $(CONFIG_PACKAGE_quantenna), \
	echo "#<< switch_ports" > $(1)/etc/switchports.conf ;\
	echo "switch_mii1_port=\""$(CONFIG_PACKAGE_QUANTENNA_ETH_WAN_PORT)"\"" >> $(1)/etc/switchports.conf ;\
	echo "switch_lan_ports=\""$(CONFIG_PACKAGE_QUANTENNA_ETH_LAN_PORTS)"\"" >> $(1)/etc/switchports.conf ;\
	echo "#>> switch_ports" >> $(1)/etc/switchports.conf ;\
	)

	(echo "#";echo "# Automatically generated script for extended mark bits configuration.";echo "# Do not edit manually.";echo "#";) > $(1)/etc/extmark.sh
	grep ^#define $(LINUX_DIR)/include/linux/extmark.h|sed -e 's/#define //' -e 's/ /=/'|grep -v ^_|awk '{ print "export "$$$$0 }' >> $(1)/etc/extmark.sh

endef
endif
