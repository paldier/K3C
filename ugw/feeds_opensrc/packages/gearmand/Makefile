##############################################
# OpenWrt Makefile for gearmand program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
# Name and release number of this package
PKG_NAME:=gearmand
PKG_VERSION:=1.1.12
#PKG_RELEASE:=1
PKG_SOURCE_URL:=https://launchpad.net/gearmand/1.2/1.1.12/+download

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libpthread +librpc +librt +libsqlite3 +libubacktrace +libstdcpp +libevent +libevent2 +boost-program_options
	TITLE:=gearmand -- prints a snarky message
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
	If you can't figure out what this program does, you're probably
	brain-dead and need immediate medical attention.
endef

define Package/$(PKG_NAME)/extra_provides
	echo 'libevent-2.0.so.5'; \
	echo 'libuuid.so.1'; \
	echo 'librpc.so'; \
	echo 'libsqlite3.so.0'; \
	echo 'libubacktrace.so.0'; 
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default. The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.

#define Build/Prepare
#	mkdir -p $(PKG_BUILD_DIR)
#	$(CP) ./src/* $(PKG_BUILD_DIR)/
#endef


TARGET_LDFLAGS+=-L$(STAGING_DIR)/usr/lib
TARGET_LDFLAGS+= -pthread -lrt -lpthread -levent -luuid

GEARMAND_CONFIGURE:= \
        LDFLAGS="$(TARGET_LDFLAGS)" \
        ./configure \
	--includedir=$(STAGING_DIR)/usr/include \
	--oldincludedir=$(STAGING_DIR)/usr/include \
        --target=i486-openwrt-linux \
        --host=i486-openwrt-linux \
        --build=i686-linux-gnu \
        --with-boost-libdir=$(STAGING_DIR)/usr/lib \
	--with-sqlite3=$(STAGING_DIR)/usr/lib \
	CPPFLAGS=-I$(STAGING_DIR)/usr/include \
	CXXFLAGS=-I$(STAGING_DIR)/usr/include \



define Build/Configure
        ( cd $(PKG_BUILD_DIR); \
                $(GEARMAND_CONFIGURE) \
         )
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one

# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist. Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/libgearman-1.0
	$(INSTALL_DIR) $(STAGING_DIR)/usr/include/libgearman
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gearmand/gearmand $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/libgearman/.libs/*.so.* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libgearman/.libs/*.so.* $(STAGING_DIR)/lib
	$(LN) -sf  $(PKG_BUILD_DIR)/libgearman/.libs/libgearman.so.8 $(STAGING_DIR)/usr/lib/libgearman.so
	$(LN) -sf $(STAGING_DIR)/lib/libgearman.so.8 $(STAGING_DIR)/lib/libgearman.so
	$(CP) $(PKG_BUILD_DIR)/libgearman-1.0/* $(STAGING_DIR)/usr/include/libgearman-1.0
	$(CP) $(PKG_BUILD_DIR)/libgearman/* $(STAGING_DIR)/usr/include/libgearman
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call BuildPackage,$(PKG_NAME)))

