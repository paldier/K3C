# ******************************************************************************** #
#       Copyright (c) 2015                                                         #
#       Lantiq Beteiligungs-GmbH & Co. KG                                          #
#       Lilienthalstrasse 15, 85579 Neubiberg, Germany                             #
#       For licensing information, see the file 'LICENSE' in the root folder of    #
#        this software module.                                                     #
# *******************************************************************************  #

PKG_NAME:= libhelper

CFLAGS := $(strip $(subst -DPACKAGE_ID=\"libhelper\",,$(CFLAGS))) -DPACKAGE_ID=\"libhelper\"
CFLAGS := $(strip $(subst -DLOGGING_ID="libhelper",,$(CFLAGS))) -DLOGGING_ID="libhelper"

opt_no_flags := -Wcast-qual

bins := libhelper.so 

libhelper.so_sources := $(wildcard *.c)
libhelper.so_cflags := -I./include/ 
libhelper.so_ldflags :=  
	
include make.inc
