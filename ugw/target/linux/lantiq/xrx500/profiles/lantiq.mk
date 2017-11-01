# Lantiq SoC xRX500 Family/Reference Boards

define Profile/None
  NAME:=Generic Profile
endef

define Profile/None/Description
	Basic profile
endef
$(eval $(call Profile,None))

define Profile/HAPS
  NAME:=LANTIQ GRX500 HAPS 
endef

define Profile/HAPS/Description
	Lantiq GRX500 HAPS Platform
endef
$(eval $(call Profile,HAPS))

define Profile/grx500_evm
  NAME:=LANTIQ GRX500 EVM Boards
endef

define Profile/grx500_evm/Description
	Lantiq GRX500 EVM Board
endef
$(eval $(call Profile,grx500_evm))

define Profile/easy350_anywan
  NAME:=EASY350 ANYWAN 600MHz Board
endef

define Profile/easy350_anywan/Description
        EASY350 ANYWAN Board
endef
$(eval $(call Profile,easy350_anywan))

define Profile/easy350_anywan_800m
  NAME:=EASY350 ANYWAN 800Mhz Board
endef

define Profile/easy350_anywan_800m/Description
        EASY350 ANYWAN 800Mhz DT
endef
$(eval $(call Profile,easy350_anywan_800m))

define Profile/easy350_anywan_router_800m
  NAME:=EASY350 ANYWAN 800Mhz Router Board
endef

define Profile/easy350_anywan_router_800m/Description
        EASY350 ANYWAN 800Mhz Router DT
endef
$(eval $(call Profile,easy350_anywan_router_800m))


define Profile/easy350_m
  NAME:=EASY350-M Board with GPON and G.Fast
endef

define Profile/easy350_m/Description
        EASY350-M Board with GPON and G.Fast
endef
$(eval $(call Profile,easy350_m))

define Profile/easy550_anywan_gw
  NAME:=EASY550 ANYWAN 1GHz Gateway Board
endef

define Profile/easy550_anywan_gw/Description
        EASY550 ANYWAN Gateway Board
endef
$(eval $(call Profile,easy550_anywan_gw))

define Profile/easy350_anywan_mpe
  NAME:=EASY350 ANYWAN Board for MPE (600MHz/800MHz)
endef

define Profile/easy350_anywan_mpe/Description
        EASY350 ANYWAN Board DT for MPE
endef
$(eval $(call Profile,easy350_anywan_mpe))

define Profile/easy550_anywan_router
  NAME:=EASY550 ANYWAN Router Board
endef

define Profile/easy550_anywan_router/Description
        EASY550 ANYWAN Router DT
endef
$(eval $(call Profile,easy550_anywan_router))

