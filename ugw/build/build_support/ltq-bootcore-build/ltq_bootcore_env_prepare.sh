#!/bin/sh
BASEDIR=$1/build_dir/xRX500_BootCore
CURDIR=$1
TOOLCHAIN_DIR=$2
PKG_BUILD_DIR=$3

error() {
	echo "$0: ERROR: $*"
	exit 1
}

env_link_config() {
	OPENWRT_BUILD=0
  export OPENWRT_BUILD
	local MODEL=`grep CONFIG_XRX500_BOOTCORE_MODEL $CURDIR/.config|cut -d '=' -f2|sed "s/\"//g"`
	local CONFIG_PATH=`cat $CURDIR/other_config_path`
	local CONFIG_LINK=`echo $CONFIG_PATH|cut -d / -f2`	
	local TOS_SIZE=`grep "UBOOT_CONFIG_TOS.*=y$" $CURDIR/.config`
	local DDR_SIZE=`grep "UBOOT_CONFIG_DDR.*=y$" $CURDIR/.config`
	if [ ! -h "$CURDIR/$CONFIG_LINK" ];then
		`ln -s $CURDIR/$CONFIG_LINK $BASEDIR/$CONFIG_LINK`
	fi
	cd $BASEDIR
	local NAME=`find $CONFIG_PATH/ -name $MODEL -type d -print0`
  [ -f "$NAME/.config" ] || error "$NAME or $BASEDIR/invalid environment directory"
  rm -f "$BASEDIR/.config"
  rm -Rf "$BASEDIR/files"
  cp "$NAME/.config" "$BASEDIR/.config" || error "Failed to copy environment configuration"
	cd -
  echo "Apply config $NAME" 
	echo "CONFIG_EXTERNAL_TOOLCHAIN=y" >> $BASEDIR/.config
  local VAR=`grep CONFIG_EXTERNAL_TOOLCHAIN=y $CURDIR/.config`
  if [ "$VAR" != "" ]; then
    local VAR2=`grep CONFIG_TOOLCHAIN_ROOT $CURDIR/.config`
    echo $VAR2 >> $BASEDIR/.config
  else
		echo "CONFIG_TOOLCHAIN_ROOT=\"$TOOLCHAIN_DIR\"" >> $BASEDIR/.config
  fi
	if [ "$TOS_SIZE" = "CONFIG_UBOOT_CONFIG_TOS_16M=y" ];then
	sed -i s/"[#].*CONFIG_.*-tos-size-.*"/"CONFIG_PACKAGE_kmod-lantiq-tos-size-16=y"/   $BASEDIR/.config
	fi
	if [ "$TOS_SIZE" = "CONFIG_UBOOT_CONFIG_TOS_32M=y" ];then
	sed -i s/"[#].*CONFIG_.*-tos-size-.*"/"CONFIG_PACKAGE_kmod-lantiq-tos-size-32=y"/   $BASEDIR/.config
	fi
	if [ "$TOS_SIZE" = "CONFIG_UBOOT_CONFIG_TOS_64M=y" ];then
	sed -i s/"[#].*CONFIG_.*-tos-size-.*"/"CONFIG_PACKAGE_kmod-lantiq-tos-size-64=y"/   $BASEDIR/.config
	fi
	if [ "$TOS_SIZE" = "CONFIG_UBOOT_CONFIG_TOS_128M=y" ];then
	sed -i s/"[#].*CONFIG_.*-tos-size-.*"/"CONFIG_PACKAGE_kmod-lantiq-tos-size-128=y"/   $BASEDIR/.config
	fi
	sed -i s/"CONFIG_UBOOT_CONFIG_TOS.*=y$"/"$TOS_SIZE"/   $BASEDIR/.config
	sed -i s/"CONFIG_UBOOT_CONFIG_DDR.*=y$"/"$DDR_SIZE"/  $BASEDIR/.config

   make -C $BASEDIR -s defconfig 
   `rm -rf $BASEDIR/staging_dir/host`
  `mkdir -p $BASEDIR/staging_dir`
  `ln -s $CURDIR/staging_dir/host $BASEDIR/staging_dir/host`
  [ -d "$NAME/files" ] && {
    cp -Rf "$NAME/files" "$BASEDIR/files" || error "Failed to copy environment files"
    chmod -R u+wr "$BASEDIR/files" || error "Failed to change the protection"
  }
  echo $NAME > "$BASEDIR/active_config"
	if [ -d $BASEDIR ]; then 
    rm -Rf $PKG_BUILD_DIR
    echo "Preparing Custom Source Directory link: $BASEDIR"
    ln -snf $BASEDIR $PKG_BUILD_DIR
  else 
    error "Custom Source Directory $BASEDIR is invalid"
  fi

}

if [ ! -d "$BASEDIR" ]; then
`mkdir $BASEDIR`
`ln -s $CURDIR/BSDmakefile $BASEDIR/BSDmakefile`
`ln -s $CURDIR/Config.in $BASEDIR/Config.in`
`ln -s $CURDIR/LICENSE $BASEDIR/LICENSE`
`ln -s $CURDIR/Makefile $BASEDIR/Makefile`
`ln -s $CURDIR/README $BASEDIR/README`
`ln -s $CURDIR/docs $BASEDIR/docs`
`ln -s $CURDIR/feeds.conf.default $BASEDIR/feeds.conf.default`
`ln -s $CURDIR/include $BASEDIR/include`
`ln -s $CURDIR/package $BASEDIR/package`
`ln -s $CURDIR/rules.mk $BASEDIR/rules.mk`
`ln -s $CURDIR/scripts $BASEDIR/scripts`
`ln -s $CURDIR/target $BASEDIR/target`
`ln -s $CURDIR/toolchain $BASEDIR/toolchain`
`ln -s $CURDIR/tools $BASEDIR/tools`
`ln -s $CURDIR/feeds.conf $BASEDIR/feeds.conf`
`ln -s $CURDIR/other_config_path $BASEDIR/other_config_path`
`ln -s $CURDIR/ugw_version $BASEDIR/ugw_version`
`ln -s $CURDIR/config $BASEDIR/config`
`ln -s $CURDIR/version $BASEDIR/version`
`ln -s $CURDIR/feeds $BASEDIR/feeds`
`mkdir -p $CURDIR/dl`
`mkdir -p $BASEDIR/build_dir`
`ln -s $CURDIR/build_dir/host $BASEDIR/build_dir/host`
`ln -s $CURDIR/dl $BASEDIR/dl`
fi
env_link_config 
