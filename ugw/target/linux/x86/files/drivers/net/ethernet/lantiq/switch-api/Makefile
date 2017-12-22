#
# Makefile for the Switch API
#
#EXTRA_CFLAGS := -I./arch/mips/include/asm/mach-lantiq/switch-api
obj-$(CONFIG_LTQ_ETHSW_API)	+= drv_switch_api.o


ifeq ($(CONFIG_SOC_XWAY),y)
	drv_switch_api-objs	:= ltq_ethsw_ioctl_wrapper.o
	drv_switch_api-objs	+= ltq_ethsw_init.o
	drv_switch_api-objs += ltq_gphy_fw.o

  drv_switch_api-objs   += ltq_flow_core.o
  drv_switch_api-objs   += ltq_flow_core_ll_table.o
  drv_switch_api-objs   += ltq_flow_pce.o
  drv_switch_api-objs   += ltq_flow_ral.o
endif

ifeq ($(CONFIG_SOC_GRX500),y)
	drv_switch_api-objs	:= gsw_ioctl_wrapper.o
	drv_switch_api-objs	+= gsw_init.o
	drv_switch_api-objs += gsw_gphy_fw.o
	
  drv_switch_api-objs   += gsw_flow_core.o
  drv_switch_api-objs   += gsw_ll_table.o
  drv_switch_api-objs   += gsw_flow_pce.o
  drv_switch_api-objs   += gsw_pae.o
endif

ifeq ($(CONFIG_X86_INTEL_CE2700),y)
EXTRA_CFLAGS := -I./arch/mips/include/asm/mach-lantiq/

	drv_switch_api-objs	:= gsw_ioctl_wrapper.o
	drv_switch_api-objs	+= gsw_init.o

  drv_switch_api-objs   += gsw_flow_core.o
  drv_switch_api-objs   += gsw_ll_table.o
  drv_switch_api-objs   += gsw_flow_pce.o
  drv_switch_api-objs   += gsw_pae.o
endif
