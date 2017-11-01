ARCH:=mips
SUBTARGET:=ar9
BOARDNAME:=Lantiq xRX100
FEATURES:=squashfs jffs2 atm
CFLAGS+= -mtune=mips32r2

DEFAULT_PACKAGES+=LTQBASE kmod-pppoa ppp-mod-pppoa linux-atm atm-tools br2684ctl kmod-ltq-dsl-ar9 ltq-dsl-app \
	kmod-input-gpio-keys-polled kmod-ledtrig-netdev kmod-leds-gpio kmod-button-hotplug \
	swconfig wpad-mini kmod-vpe kmod-lantiq-vpe

define Target/Description
	Lantiq xRX100
endef
