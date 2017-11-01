# Lantiq SoC xRX330 Family/Reference Boards

define Profile/EASY389_BOND
  NAME:=EASY389 BOND Reference Board (600MHz)
  PACKAGES:=
endef

$(eval $(call Profile,EASY389_BOND))

define Profile/EASY330
 NAME:=EASY330 Singleline DSL with Power Monitoring INA219
endef

$(eval $(call Profile,EASY330))

define Profile/EASY300_BOND
  NAME:=EASY300 VDSL BOND Board (600MHz)
  PACKAGES:=
endef

$(eval $(call Profile,EASY300_BOND))

define Profile/EASY330_GFAST
  NAME:=EASY330 G.Fast Board (720MHz)
  PACKAGES:=
endef

$(eval $(call Profile,EASY330_GFAST))

define Profile/EASY330_BOND
  NAME:=EASY330 VDSL BOND Board (720MHz)
  PACKAGES:=
endef

$(eval $(call Profile,EASY330_BOND))

define Profile/EASY300_AC1200
  NAME:=EASY300 AC1200 Board (600MHz)
  PACKAGES:=
endef

$(eval $(call Profile,EASY300_AC1200))

