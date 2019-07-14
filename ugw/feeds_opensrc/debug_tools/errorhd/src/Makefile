PKG_NAME := errorhd

CFLAGS := $(filter-out -DPACKAGE_ID=\"errorhd\" -DLOGGING_ID="errorhd",$(CFLAGS)) -I./

#opt_strip=1

bins := errorhd errorhd_cmd errorhd_test

errorhd_sources := errorhd.c errorhd_spawn.c
errorhd_cflags := -I./ -DPACKAGE_ID=\"ERRORHD\" -DLOGGING_ID="errorhd"
ifneq (,$(findstring UTS_MODE,$(CFLAGS)))
    errorhd_ldflags := -ldl -lpthread -Llibconfig -lconfig -ltelemetry
else
    errorhd_ldflags := -ldl -lpthread -Llibconfig -lconfig
endif

errorhd_cmd_sources := errorhd_cmd.c
errorhd_cmd_ldflags := -ldl

errorhd_test_sources := errorhd_test.c
errorhd_test_ldflags := -ldl -lpthread

include make.inc
