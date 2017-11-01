obj-y += ppa_hook.o ppe_drv_wrapper.o ppa_stack_al.o ppa_api_tools.o ppa_datapath_wrapper.o
obj-y += ppa_stack_tnl_al.o
obj-$(CONFIG_LTQ_PPA_API) += ppa_api.o
obj-$(CONFIG_LTQ_PPA_HAL_SELECTOR) += ppa_hal_wrapper.o

ppa_api-objs = 


ppa_api-objs += ppa_api_mfe.o

ifeq ($(CONFIG_LTQ_PPA_QOS),y)
ppa_api-objs += ppa_api_qos.o
endif

ppa_api-objs += ppa_api_mib.o ppa_api_core.o ppa_api_misc.o ppa_api_netif.o 

ifeq ($(CONFIG_LTQ_PPA_HANDLE_CONNTRACK_SESSIONS),y)
ppa_api-objs += ppa_api_session_limit.o
endif

ppa_api-objs += ppa_api_session.o
ppa_api-objs += ppa_api_sess_helper.o
ifeq ($(CONFIG_LTQ_PMCU),y)
   ppa_api-objs += ppa_api_pwm.o ppa_api_pwm_logic.o
endif

ifeq ($(CONFIG_LTQ_CPU_FREQ),y)
   ppa_api-objs += ppa_api_pwm.o ppa_api_pwm_logic.o
endif

ifeq ($(CONFIG_LTQ_PPA_API_DIRECTPATH),y)
   ppa_api-objs += ppa_api_directpath.o
ifeq ($(CONFIG_PPA_PUMA7),y)
   ppa_api-objs += lpdp_api.o
endif
endif
ifeq ($(CONFIG_LTQ_PPA_COC_SUPPORT),y)
   ppa_api-objs += ppa_api_cpu_freq.o
endif

ifeq ($(CONFIG_LTQ_PPA_API_PROC),y)
  ppa_api-objs += ppa_api_proc.o
endif

ifeq ($(CONFIG_LTQ_PPA_HAL_SELECTOR),y)
    ppa_api-objs += ppa_api_hal_selector.o
endif

ifeq ($(CONFIG_LTQ_PPA_API_PROC),m)
  obj-$(CONFIG_LTQ_PPA_API_PROC) += ppa_api_proc.o
endif


# CUT_HERE_FOR_CD
# Note: Do not add any thing below this line. Following the lines 
#       "CUT_HERE_FOR_CD" will be removed during CD creation.
ifeq ($(CONFIG_LTQ_PPA_API_SW_FASTPATH),y)
   obj-m += swa_stack_al.o
   ppa_api-objs += ppa_sae_hal.o
   obj-m += ppa_api_sw_accel_mod.o
   ppa_api_sw_accel_mod-objs = ppa_api_sw_accel.o ppa_api_sw_accel_module.o 
endif

ifeq ($(CONFIG_LTQ_PPA_MPE_HAL),m)
  obj-m += ppa_api_tmplbuf.o
  ppa_api_tmplbuf-objs = ppa_api_tmpl_buf.o ppa_api_tmplbuf_module.o
endif
