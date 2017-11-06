#
# Add menu configurable log level options to a package.
# This also converts configured log levels to corresponding compile flags
# for the sources.
#

ifeq ($(PKG_IS_KERNEL_MODULE),y)
PKG_NAME_PREF:=kmod-
endif

define config_add_global_debug_level
config FEATURE_UGW_GLOBAL_DEBUG_LEVEL
	depends on PACKAGE_$(PKG_NAME_PREF)$(PKG_NAME)
	int 'Specifiy Debug level from 0 - 7 for UGW Software'
	range 0 7
	default '4'
	help
	  Add global debug level for UGW Software (ranges: 0 - 7)
endef

define Package/$(PKG_NAME_PREF)$(PKG_NAME)/config_debug_level
config $(PKG_NAME_PREF)$(PKG_NAME)_DEBUG_LEVEL
	string 'Specifiy Debug level from 0 - 7.'
	depends on PACKAGE_$(PKG_NAME_PREF)$(PKG_NAME)
	default ""
	help
	  Compiles out all the debug levels greater than the selected level 'X' (0 <= X <= 7).
	  Allows control over debug prints and code foot print.

config $(PKG_NAME_PREF)$(PKG_NAME)_DEBUG_TYPE
	int 'Specifiy Debug Type from 0 - 3.'
	depends on PACKAGE_$(PKG_NAME_PREF)$(PKG_NAME)
	range 0 3
	default 1
	help
	  Specify where the log needs to be redirected.
	  0 - None
	  1 - File
	  2 - Console
	  3 - Both File and Console
endef

TARGET_CFLAGS += -DPACKAGE_ID=\\\"$(PKG_NAME_PREF)$(PKG_NAME)\\\" -DLOGGING_ID=\"$(subst -,_,$(PKG_NAME_PREF)$(PKG_NAME))\" \
	$(if $(call qstrip,$(CONFIG_$(PKG_NAME_PREF)$(PKG_NAME)_DEBUG_LEVEL)),\
		-DLOG_LEVEL=$(CONFIG_$(PKG_NAME_PREF)$(PKG_NAME)_DEBUG_LEVEL),-DLOG_LEVEL=$(CONFIG_FEATURE_UGW_GLOBAL_DEBUG_LEVEL)) \
	$(if $(call qstrip,$(CONFIG_$(PKG_NAME_PREF)$(PKG_NAME)_DEBUG_TYPE)),\
		-DLOG_TYPE=$(CONFIG_$(PKG_NAME_PREF)$(PKG_NAME)_DEBUG_TYPE))

