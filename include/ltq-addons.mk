
define generate_config_headers
	$(eval LTQ_CONF_H=$(TMP_DIR)/ifx_config.h)$(eval LTQ_CONF_SH=$(TMP_DIR)/ifx_config.sh) \
	if [ $(TOPDIR)/.config -nt $(LTQ_CONF_H) -o $(TOPDIR).config -nt $(LTQ_CONF_SH) ]; then \
		$(SCRIPT_DIR)/config/conf --defconfig=$(TOPDIR)/.config -w $(TOPDIR)/.config Config.in; \
		echo -en "/*\n * Automatically generated config for C files. Don't edit.\n */\n" > $(LTQ_CONF_H); \
		grep ^CONFIG_.* .config \
			| sed 's/=/ /' | awk '{ gsub("-","_",$$1);$$1=toupper($$1);gsub("y$$|m$$","1",$$2);print "#define " $$0 }' \
			>> $(LTQ_CONF_H); \
		if [ -f $(LTQ_CONF_H) ]; then mkdir -p $(STAGING_DIR)/usr/include/; cp -f $(LTQ_CONF_H) $(STAGING_DIR)/usr/include/; fi; \
		echo -en "#\n# Automatically generated config for startup scripts. Don't edit.\n#\n" > $(LTQ_CONF_SH); \
		grep ^#define $(LTQ_CONF_H) \
			| sed -e 's/#define //' -e 's/ /=/' \
			| awk '{ print "export "$$0 }' \
			>> $(LTQ_CONF_SH); \
	fi
endef

