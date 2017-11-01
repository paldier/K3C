# Lantiq SoC xRX500 4kec Family/Reference Boards

define Profile/HAPS_BOOTCORE
  NAME:=LANTIQ GRX500 HAPS BOOTCORE
endef

define Profile/HAPS_BOOTCORE/Description
	Lantiq GRX500 HAPS BOOTCORE Platform
endef
$(eval $(call Profile,HAPS_BOOTCORE))

define Profile/grx500_evm_bootcore
  NAME:=LANTIQ GRX500 EVM BOOTCORE
endef

define Profile/grx500_evm_bootcore/Description
	Lantiq GRX500 EVM BOOTCORE Reference Platform
endef
$(eval $(call Profile,grx500_evm_bootcore))

define Profile/easy350550_bootcore
  NAME:=LANTIQ EASY350/550 ANYWAN BOOTCORE
endef

define Profile/easy350550_bootcore/Description
	EASY350/550 ANYWAN BOOTCORE
endef
$(eval $(call Profile,easy350550_bootcore))


