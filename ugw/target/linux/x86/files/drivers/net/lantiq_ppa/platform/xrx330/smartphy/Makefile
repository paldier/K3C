ifneq ($(CONFIG_LTQ_PPA_E1),)
obj-$(CONFIG_LTQ_PPA_DATAPATH)	+= ltqmips_vrx318_e1.o
endif
ifneq ($(CONFIG_LTQ_PPA_A1),)
obj-$(CONFIG_LTQ_PPA_DATAPATH)	+= ltqmips_vrx318_a1.o
endif
obj-y += vrx218_common.o 
ltqmips_vrx318_e1-objs = vrx218_dfe.o vrx218_ppe_ptm_init.o vrx218_ptm_datapath.o vrx218_ptm_main.o vrx218_ptm_qos.o
ltqmips_vrx318_a1-objs = vrx218_dfe.o vrx218_ppe_atm_init.o vrx218_atm_datapath.o vrx218_atm_main.o



