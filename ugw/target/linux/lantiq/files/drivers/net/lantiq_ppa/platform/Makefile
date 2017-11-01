obj-$(CONFIG_AR9)	+= ar9/
ifneq ($(CONFIG_LTQ_PPA_E1),)
  obj-$(CONFIG_LTQ_PPA_XRX300)	+= xrx330/
  obj-$(CONFIG_LTQ_PPA_XRX330)	+= xrx330/
else
 ifneq ($(CONFIG_LTQ_PPA_A1),)
  obj-$(CONFIG_LTQ_PPA_XRX300)	+= xrx330/
  obj-$(CONFIG_LTQ_PPA_XRX330)	+= xrx330/
 else
  obj-$(CONFIG_LTQ_PPA_XRX200)	+= xrx200/
  obj-$(CONFIG_LTQ_PPA_XRX300)	+= xrx300/
  obj-$(CONFIG_LTQ_PPA_XRX330)	+= xrx330/
  obj-$(CONFIG_LTQ_PPA_GRX500)	+= xrx500/
 endif
endif

#obj-$(CONFIG_PPA_PUMA7) += puma/
