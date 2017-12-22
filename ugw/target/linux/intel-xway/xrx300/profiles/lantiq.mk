# Lantiq SoC xRX300 Family/Reference Boards

define Profile/EASY388
  NAME:=EASY388 Family Board
  PACKAGES:=
endef

$(eval $(call Profile,EASY388))

define Profile/EASY388_ina219
 NAME:=EASY388 Family Board with Power Monitoring INA219
endef

$(eval $(call Profile,EASY388_ina219))

define Profile/EASY388_VRX318
  NAME:=EASY388 Family Board with VRX318 Addon
endef

$(eval $(call Profile,EASY388_VRX318))

define Profile/EASY362
  NAME:=EASY362 Family Board
  PACKAGES:=
endef

$(eval $(call Profile,EASY362))

define Profile/EASY382
  NAME:=EASY382 Family Board
  PACKAGES:=
endef

$(eval $(call Profile,EASY382))

define Profile/EASY388_BOND
  NAME:=EASY388 BOND Reference Board
  PACKAGES:=
endef

$(eval $(call Profile,EASY388_BOND))

