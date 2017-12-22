# ******************************************************************************** #
#       Copyright (c) 2015                                                         #
#       Lantiq Beteiligungs-GmbH & Co. KG                                          #
#       Lilienthalstrasse 15, 85579 Neubiberg, Germany                             #
#       For licensing information, see the file 'LICENSE' in the root folder of    #
#        this software module.                                                     #
# *******************************************************************************  #

PKG_NAME := ltq-cgroups

bins := cgroup_notifier cgroup_daemon

ifeq ($(ARCH),i386)
cgroup_daemon_sources := cgroup_daemon_puma.c
else
cgroup_daemon_sources := cgroup_daemon.c
endif

include make.inc
