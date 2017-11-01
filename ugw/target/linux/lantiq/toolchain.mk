#
# Lantiq toolchain addon for OpenWRT package/toolchain
# Updates to support mklibs when external toolchain used.
#

ifneq ($(CONFIG_EXTERNAL_TOOLCHAIN),)
  define Package/libc/install_lib
	$(CP) $(filter-out %/libdl_pic.a %/libpthread_pic.a %/libresolv_pic.a %/libnsl_pic.a,$(wildcard $(TOOLCHAIN_ROOT_DIR)/lib/lib*.a)) $(1)/lib/
	$(if $(wildcard $(TOOLCHAIN_ROOT_DIR)/lib/libc_so.a),$(CP) $(TOOLCHAIN_ROOT_DIR)/lib/libc_so.a $(1)/lib/libc_pic.a)
	$(if $(wildcard $(TOOLCHAIN_ROOT_DIR)/lib/gcc/*/*/libgcc.map), \
		$(CP) $(TOOLCHAIN_ROOT_DIR)/lib/gcc/*/*/libgcc_pic.a $(1)/lib/libgcc_s_pic.a; \
		$(CP) $(TOOLCHAIN_ROOT_DIR)/lib/gcc/*/*/libgcc.map $(1)/lib/libgcc_s_pic.map \
	)
  endef

  define Package/libpthread/install_lib
	$(if $(wildcard $(TOOLCHAIN_ROOT_DIR)/lib/libpthread_so.a),$(CP) $(TOOLCHAIN_ROOT_DIR)/lib/libpthread_so.a $(1)/lib/libpthread_pic.a)
  endef
endif

