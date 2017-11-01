define Profile/Generic
  NAME:=Generic - all boards
  PACKAGES:= \
	kmod-dm9000-nfs \
	kmod-i2c-core kmod-i2c-lantiq kmod-eeprom-at24 \
	kmod-spi-bitbang kmod-spi-gpio kmod-eeprom-at25 \
	gpon-dti-agent
endef

$(eval $(call Profile,Generic))

define Profile/EASY98000
  NAME:=EASY98000
  PACKAGES:= \
	kmod-dm9000-nfs \
	kmod-i2c-core kmod-i2c-lantiq kmod-eeprom-at24 \
	kmod-spi-bitbang kmod-spi-gpio kmod-eeprom-at25
endef

define Profile/EASY98000/Description
	Lantiq EASY98000 evalkit
endef

$(eval $(call Profile,EASY98000))

define Profile/EASY98020
  NAME:=EASY98020
endef

define Profile/EASY98020/Description
	Lantiq EASY98020 evalkit
endef

$(eval $(call Profile,EASY98020))

define Profile/EASY98020V18
  NAME:=EASY98020V18
endef

define Profile/EASY98020V18/Description
	Lantiq EASY98020 V1.8 evalkit
endef

$(eval $(call Profile,EASY98020V18))

define Profile/MDU
  NAME:=MDU
endef

define Profile/MDU/Description
	Lantiq MDU platform
endef

$(eval $(call Profile,MDU))

define Profile/SFP
  NAME:=SFP
  PACKAGES:= \
	gpon-sfp-i2c-drv gpon-sfp-i2c-drv-linux kmod-gpon-sfp-i2c-drv
endef

define Profile/SFP/Description
	Lantiq SFP platform
endef

$(eval $(call Profile,SFP))
