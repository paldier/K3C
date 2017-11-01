ARCH:=mips
SUBTARGET:=xrx330
BOARDNAME:=Lantiq xRX330
FEATURES:=squashfs jffs2 atm
CFLAGS+= -mtune=34kc

DEFAULT_PACKAGES+=LTQBASE kmod-pppoa ppp-mod-pppoa linux-atm atm-tools br2684ctl kmod-ltq-dsl-vr9 ltq-dsl-app swconfig kmod-lantiq_ppa_xrx330 \
		kmod-vpe kmod-lantiq-vpe ppp-mod-pppol2tp

define Target/Description
	Lantiq xRX330
endef
