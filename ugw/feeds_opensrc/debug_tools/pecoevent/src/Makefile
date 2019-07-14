# ******************************************************************************** #
#       Copyright (c) 2015                                                         #
#       Lantiq Beteiligungs-GmbH & Co. KG                                          #
#       Lilienthalstrasse 15, 85579 Neubiberg, Germany                             #
#       For licensing information, see the file 'LICENSE' in the root folder of    #
#        this software module.                                                     #
# *******************************************************************************  #

obj-m += pecoevent_collect.o

ifndef STAGING_DIR1
  export TOPDIR:=$(PKG_BUILD_DIR)/../../../
  export CFLAGS=
  export OPENWRT_VERBOSE=s
  include $(TOPDIR)/rules.mk
  include $(INCLUDE_DIR)/kernel.mk
  THISMODULE=$(PKG_BUILD_DIR)
  export CONFIG_DEBUG_SECTION_MISMATCH=1
all:
	make ARCH=$(LINUX_KARCH) $(TARGET_CONFIGURE_OPTS) -C $(LINUX_DIR) M=$(THISMODULE) modules
clean:
	make ARCH=$(LINUX_KARCH) $(TARGET_CONFIGURE_OPTS) -C $(LINUX_DIR) M=$(THISMODULE) clean
else
all:
	make ARCH=$(LINUX_KARCH) $(TARGET_CONFIGURE_OPTS) -C $(LINUX_DIR) M=$(THISMODULE) modules
clean:
	rm -f *.o *.ko *.mod.* modules.order Module.symvers
endif
