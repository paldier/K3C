ARCH:=mips
SUBTARGET:=xrx500_4kec
BOARDNAME:=Lantiq xRX500 BOOTCORE
KERNELNAME:="vmlinux.srec"

DEFAULT_PACKAGES+=LTQBASE

define Target/Description
	Lantiq xRX500 Boot Core
endef
