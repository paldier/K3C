#!/bin/bash
#
# UGW (CDs) Creation script
# Just execute this under your UGW/openwrt/core/ path 
# For more information execute script with -h option.
#

OWRT_GIT_REV="ac8dd1d3f9c3d2f192aa42d06945cee9464a0dd6"
OWRT_GIT_REV_1="30a6f4a50919667bfd350e5523e801d460569a42"
BUILD_PATH=$2
PROJ_NAME="UGW-`cat ugw/ugw_version`"
SW_CD_NAME="${PROJ_NAME}-SW-CD"
OPENSW_CD_NAME="${PROJ_NAME}-OPENSRC-CD"
CUR_PATH=`pwd -P` #from where script executed
SDK_DESCRIPTION="Lantiq Universal Gateway $PROJ_NAME Software CD"
CDS_COMMON_PATH="${PROJ_NAME}-CDs"

error_print()
{
	echo "ERROR: $@"
	exit 1;
}

pkg_3rdparty="tuxera-ntfs tuxera_xRx500-ntfs eip97 eip123"
thirdparty()
{
	[ "$1" = "install" ] && {
		./scripts/feeds update thirdparty_sw
		for tpkg in $pkg_3rdparty
		do
			./scripts/feeds $1 -f $tpkg
		done 
	} || {
		rm -rf $CUR_PATH/package/feeds/thirdparty_sw
		rm -rf $CUR_PATH/feeds/thirdparty_sw*
	}
}

copy_dbtool_jar_file()
{
	echo $1
	#root path
	cd $1/ugw/feeds_ugw/framework/DBTool/src/dist/lib > /dev/null 2>&1
	wget http://central.maven.org/maven2/javax/xml/jsr173/1.0/jsr173-1.0.jar >/dev/null 2>&1
	wget http://www.mmbase.org/maven2/com/bea/xml/jsr173-ri/1.0/jsr173-ri-1.0.jar >/dev/null 2>&1 
	wget http://central.maven.org/maven2/com/bea/xml/jsr173-ri/1.0/jsr173-ri-1.0.jari >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/apache/logging/log4j/log4j-api/2.2/log4j-api-2.2.jar >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/apache/logging/log4j/log4j-core/2.2/log4j-core-2.2.jar >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/python/jython-standalone/2.7-b4/jython-standalone-2.7-b4.jar >/dev/null 2>&1
	wget http://central.maven.org/maven2/net/java/dev/stax-utils/stax-utils/20070216/stax-utils-20070216.jar >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/swinglabs/swingx/swingx-all/1.6.5-1/swingx-all-1.6.5-1.jar >/dev/null 2>&1
	wget http://central.maven.org/maven2/xmlunit/xmlunit/1.5/xmlunit-1.5.jar >/dev/null 2>&1
	
	cd ../../frontend/lib >/dev/null
	cp ../../dist/lib/* . >/dev/null
	
	cd $1 >/dev/null
}

copy_system_anlyzer_jar_file()
{
	cd $1/ugw/feeds_ugw/debug/system-analyzer/src/systemanalyzer/lib/ > /dev/null 2>&1 
	wget http://central.maven.org/maven2/commons-codec/commons-codec/1.7/commons-codec-1.7.jar  >/dev/null 2>&1
	wget http://central.maven.org/maven2/net/lingala/zip4j/zip4j/1.3.2/zip4j-1.3.2.jar  >/dev/null 2>&1
	wget http://central.maven.org/maven2/junit/junit/4.10/junit-4.10.jar  >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/controlsfx/controlsfx/8.20.8/controlsfx-8.20.8.jar  >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/controlsfx/fxsampler/1.0.8/fxsampler-1.0.8.jar  >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/controlsfx/openjfx-dialogs/1.0.2/openjfx-dialogs-1.0.2.jar  >/dev/null 2>&1
	wget http://central.maven.org/maven2/org/reactfx/reactfx/1.4.1/reactfx-1.4.1.jar  >/dev/null 2>&1
	cd $1 >/dev/null

}

environment_prepare()
{

#cloning openwrt(chaos calmer) and updating to ugw revision

	which git > /dev/null || error_print "Error: Missing 'git' tool. Please install 'git' in your PC to continue."
	[ -d ./$PROJ_NAME ] || {
	        git clone https://git.openwrt.org/15.05/openwrt.git $PROJ_NAME || git clone https://github.com/openwrt/openwrt $PROJ_NAME || error_print "Unable to download check proxy settings."
	}

#updating to particualr revision
	cd $PROJ_NAME/
	[ "`git rev-parse HEAD`" = "$OWRT_GIT_REV" ] || {
		git reset --hard $OWRT_GIT_REV ||  git reset --hard $OWRT_GIT_REV_1 || error_print "Unable to update openwrt to a specified revision. Please try '$0 -o' to clean and re-try."
	}
	cd - >/dev/null

#setup patches link
	cd ugw/build/buildsystem_patches/core/
	patches_path=`pwd -P`
	cd - >/dev/null

	cd $PROJ_NAME/
	[ -L patches ] || {
		ln -s $patches_path patches
	}
	
	sed -e '/.*#+[btsfl].*/d' -e '/.*#+ugw-.*/d' -e '/.*ugw-temperory*/d' -e '/.*hgignore-*/d' -i patches/series
	

	quilt --version 2>/dev/null || error_print "Error: Missing 'quilt' tool. Please install 'quilt' in your PC to continue."
	quilt push -a

	mv ../patches.list . 2>/dev/null 
	mv .git .git-orig 2>/dev/null 
	cp -r ../ugw .
	cp -r ugw/dl .

	rm -rf target/linux/lantiq/
	cd target/linux/
	ln -s ../../ugw/target/linux/lantiq lantiq
	#cp -aLr  ../../ugw/target/linux/x86/* x86/
	cd - >/dev/null

	rm -f feeds.conf
        rm -rf config
        ln -sf ugw/feeds.conf
        echo "./ugw/config" > other_config_path
	ln -sf ugw/ugw_version 

        mkdir config
        cp ./ugw/config/Config* config/
	cp -r ./ugw/dl .

	#Download required jar files for DBTool re-compilation.
	copy_dbtool_jar_file `pwd -P`
	
	copy_system_anlyzer_jar_file `pwd -P`

        ./scripts/feeds update -a
        ./scripts/feeds install -a 2>/dev/null

	#Avoid feed install overwrite during opensrc pkg installation.
	rm -rf package/feeds/ltq_packages tmp/info
        ./scripts/feeds install -a 2>/dev/null
	
	rm -rf ./package/feeds/thirdparty_sw/
	./scripts/feeds install -f eip97 2>/dev/null
	./scripts/feeds install -f eip123 2>/dev/null

	chmod +x ./scripts/*
	echo -e "CD Environment Setup done...!"
	echo -e "Select model and compile ...!"
}

#List of lantiq/intel packages tarball to be copied in CD.
__TAR_LIST="drv_dsl_cpe_api_vrx* drv_mei_cpe* dsl_* sl_dsl_cpe* 
	   lib_ifxos* 
	   lib_dti* 
	   switch_cli*  lib_cli*
           ltq_voip_common* drv_event_logger* ltq_voip* coef_voip* fw_voip*
	   lq-wave*
	   ltq_fw_PHY_IP*
	   ltq_temp-*
	   ltq_pmcu-*
	   ltq_pm_util*
	   drv_sdd_mbx*
	   lib_tapi_nlt_cpe* ltq_dect* drv_vmmc* tapidemo* tapidump* drv_tapi* drv_kpi2udp* drv_palsys* drv_dspmod* lib_ccu* lib_slic220* drv_mxp_mem* lib_svca*
	   lib_mxp* drv_mxp_core* drv_telhal* drv_voice_ni* mxp_app* fw_voice_c55_dsp*
	   ltq_hanfun_agent*
	   ltq_regulator_cpufreq-* ltq_ppmd-*
	   gpon*
	   ltq_eip97* M5T-SceAgent* ltq_eip123* 
	   wss_gateway-* iotivity-hanfun* libltqhanfun*"

copy_tarbal()
{
	local _file _ii;
	mkdir $CUR_PATH/$SW_CD_NAME/ugw/dl
	for tarfile in $__TAR_LIST ; do
		cp -rf $CUR_PATH/dl/$tarfile $CUR_PATH/$SW_CD_NAME/ugw/dl 2>/dev/null
		#copy from other model pkg.
		[ -n "$OTHER_BUILD_DIR" ] && {	
			for i in $OTHER_BUILD_DIR 
			do
				_file="";
				_file="`ls $i/dl/$tarfile 2>/dev/null`"
				[ -n "$_file" ] && {
					for _ii in $_file; do
						cp -rf $_ii $CUR_PATH/$SW_CD_NAME/ugw/dl/
					done
				}
			done
		}
	done
	
	chmod 644 $CUR_PATH/$SW_CD_NAME/ugw/dl/*
}

__CONFIG_FILE="./ugw/config/GRX350_1200_MR_VDSL_LTE_GW_711
		./ugw/config/GRX350_1200_GW_HE_VDSL_LTE_OWRT_71_SAMPLE
		./ugw/config/GRX350_1600_MR_VDSL_LTE_SEC_GW_711
		./ugw/config/GRX350_1200_MR_GPON_GW_711
		./ugw/config/bootcore/GRX_350_550_BOOTCORE
		./ugw/config/bootcore/GRX_350_550_SECBOOT_BOOTCORE
		./ugw/config/bootcore/GRX550_2000_GW_HE_VDSL_LTE_SECBOOT_71
		./ugw/config/GRX220_EL_LTE_GW_711
		./ugw/config/GRX550_2000_MR_ETH_RT_711
		./ugw/config/GRX550_2000_MR_VDSL_LTE_SEC_GW_711
		./ugw/config/VRX220_EL_VDSL_GW_711
		./ugw/config/GRX350_1600_MR_ETH_RT_711
		./ugw/config/GRX350_1600_GW_HE_VDSL_LTE_QCA_71/"

copy_config_files()
{

cd $CUR_PATH
rm -rf $CUR_PATH/$SW_CD_NAME/ugw/config/
for __config_file in $__CONFIG_FILE ; do
	mkdir -p $CUR_PATH/$SW_CD_NAME/$__config_file 
	cp -rf $__config_file/.config $CUR_PATH/$SW_CD_NAME/$__config_file/ 		
done
cp -r ./config/Config* $CUR_PATH/$SW_CD_NAME/ugw/config/
cd - >/dev/null

}

#part binary packages list 
PART_BINARY="ugw-devm ltq-dect mcastd shgw"
pkg_build_path=
tar_name=

get_build_path()
{
  	cd $BUILD_PATH/$pkg*
	pkg_build_path=`pwd -P`
	cd - > /dev/null
}

part_binary()
{
for pkg in $PART_BINARY ; do

	cd $CUR_PATH
	tar_name=`./scripts/metadata.pl package_source ./tmp/.packageinfo | grep -w ^$pkg: | awk '{ print $2 }'`

	if [ "$pkg" = "ugw-devm" ] ; then
		get_build_path
		cd $CUR_PATH/$SW_CD_NAME/ugw/feeds_ugw/cwmp/ugw_devm/src/
		./create_customer_package.sh $pkg_build_path
		cd - >/dev/null
	fi
	
	if [ "$pkg" = "shgw" ] ; then
		get_build_path
		cd $CUR_PATH/$SW_CD_NAME/ugw/feeds_ugw/shgw/shgw/
		./create_customer_package.sh $pkg_build_path
		cd - >/dev/null
	fi

	if [ "$pkg" = "mcastd" ]; then
		get_build_path
		cd $CUR_PATH/$SW_CD_NAME/ugw/feeds_ugw/mcast/daemon_mcast/src	
		./create_customer_package.sh $pkg_build_path
		cd - >/dev/null
	fi

	if [ "$pkg" = "ltq-dect" ]; then
		
		cd $BUILD_PATH/ltq_dect*
		pkg_build_path=`pwd -P`
		cd - >/dev/null

		cd $CUR_PATH/$SW_CD_NAME/ugw/dl/
		tar -xzf $tar_name
		cd ltq_dect*
		./create_customer_package.sh $pkg_build_path
		cd - > /dev/null
		untar_pkg_name=`ls -d ltq_dect*/`
		tar -czf $tar_name $untar_pkg_name
		rm -rf $untar_pkg_name 
	fi
done

}

#thirdparty binary copy
thirdparty_tarbal_copy()
{
#cp -rf $CUR_PATH/$BUILD_PATH/$CDS_COMMON_PATH/$PROJ_NAME-MEDIATEK-CD/$PROJ_NAME/dl/mt7603e* $CUR_PATH/$SW_CD_NAME/ugw/dl/
#cp -rf $CUR_PATH/$BUILD_PATH/$CDS_COMMON_PATH/$PROJ_NAME-RTDOT1XD-CD/$PROJ_NAME/package/feeds/thirdparty_sw/rtdot1xd $CUR_PATH/$SW_CD_NAME/ugw/feeds_thirdparty/
#cp -rf $CUR_PATH/dl/tuxera* $CUR_PATH/$SW_CD_NAME/ugw/dl/
#cp -rf $CUR_PATH/$BUILD_PATH/$CDS_COMMON_PATH/$PROJ_NAME-EIP97-CD/$PROJ_NAME/dl/ltq_eip97* $CUR_PATH/$SW_CD_NAME/ugw/dl/
rm -rf $CUR_PATH/$SW_CD_NAME/ugw/feeds_opensrc/debug_tools/pecostat*
cp -rf $CUR_PATH/$BUILD_PATH/$CDS_COMMON_PATH/$PROJ_NAME-PECOSTAT-CD/$PROJ_NAME/dl/peco* $CUR_PATH/$SW_CD_NAME/ugw/dl/
cp -rf $CUR_PATH/$BUILD_PATH/$CDS_COMMON_PATH/$PROJ_NAME-PECOSTAT-CD/$PROJ_NAME/package/feeds/open_debug_tools/pecostat* $CUR_PATH/$SW_CD_NAME/ugw/feeds_opensrc/debug_tools/

rm -rf $CUR_PATH/$SW_CD_NAME/ugw/feeds_opensrc/debug_tools/pecostat/patches/101-ltq-cpu-utilization-print-app.patch 
rm -rf $CUR_PATH/$SW_CD_NAME/ugw/feeds_opensrc/debug_tools/pecostat_interaptiv/patches/002-ltq-cpu-utilization-print-app.patch
rm -rf $CUR_PATH/$BUILD_PATH/$CDS_COMMON_PATH/$PROJ_NAME-PECOSTAT-CD

}

cd_opensrc_start()
{
	[ ! -d "$OPENSW_CD_NAME" ] && {
		mkdir $OPENSW_CD_NAME	
	} || echo "Removing existing $OPENSW_CD_NAME" ; rm -rf $OPENSW_CD_NAME ;  mkdir $OPENSW_CD_NAME
	
	#copy ugw directory structure "AS IS"
	cp -Lr ugw/ $OPENSW_CD_NAME 2>/dev/null
	find $OPENSW_CD_NAME -follow -name ".hg*" | xargs rm -rf

	rm -rf $OPENSW_CD_NAME/ugw/feeds_ugw
	rm -rf $OPENSW_CD_NAME/ugw/feeds_thirdparty
	rm -rf $OPENSW_CD_NAME/ugw/config/*
	rm -rf $OPENSW_CD_NAME/ugw/target/x86/*

	cp -r $CUR_PATH/ugw/config/GRX350_GW_HE_VDSL_LTE_OWRT_SAMPLE $CUR_PATH/$OPENSW_CD_NAME/ugw/config/
	cp -r $CUR_PATH/ugw/config/bootcore/GRX_350_550_BOOTCORE $CUR_PATH/$OPENSW_CD_NAME/ugw/config/
	cp -r $CUR_PATH/config/Config* $CUR_PATH/$OPENSW_CD_NAME/ugw/config/

	cd $CUR_PATH
	cp "$(readlink -f $0)" $OPENSW_CD_NAME/ugw-prepare-all.sh
	cd -
}

#create software cd 
cd_sw_start()
{
	[ ! -d "$SW_CD_NAME" ] && {
		mkdir $SW_CD_NAME		
	} || echo "Removing existing $SW_CD_NAME" ; rm -rf $SW_CD_NAME ;  mkdir $SW_CD_NAME
	
	#copy ugw directory structure "AS IS"
	cp -Lr ugw/ $SW_CD_NAME 2>/dev/null
	find $SW_CD_NAME -follow -name ".hg*" | xargs rm -rf

	rm -rf $SW_CD_NAME/ugw/target/linux/x86
	rm -rf $SW_CD_NAME/ugw/feeds_ugw/puma_components
	rm -rf $SW_CD_NAME/ugw/feeds_ugw/puma_components/sepdk
	rm -rf $SW_CD_NAME/ugw/feeds_thirdparty/rtdot1xd/src
	rm -rf $SW_CD_NAME/ugw/feeds_thirdparty/rtdot1xd/create_customer_package.sh
	rm -rf $SW_CD_NAME/ugw/feeds_thirdparty/mt7603e/patches
	rm -rf $SW_CD_NAME/ugw/feeds_thirdparty/mt7603e/create_customer_package.sh
	rm -rf $SW_CD_NAME/ugw/feeds_thirdparty/secure-app
	rm -rf $SW_CD_NAME/ugw/ugw-prepare-all.sh
	rm -rf $SW_CD_NAME/ugw/feeds_ugw/wlan/wlan_feed/
	rm -rf $SW_CD_NAME/ugw/feeds_ugw/wlan/wlan_wave_feed/lq-wave-300/
	rm -rf $SW_CD_NAME/ugw/target/ugw-sdk/

	#copy intel/lantiq specific tarbal
	copy_tarbal
	copy_tmp_tarbal

	# create part binary packages
	part_binary

	#copy config files 
	copy_config_files

	#addon cpy
	thirdparty_tarbal_copy
	
	cd $CUR_PATH
	cp "$(readlink -f $0)" $SW_CD_NAME/ugw-prepare-all.sh
	cd -
}

create_installer()
{
	#preparing installer 
	cd $CUR_PATH
	cd $1
	mkdir -p $1/Sources/
	mv ugw* $1/Sources/
        cp ../target/ugw-sdk/license/wrapper/LICENSE .
	../target/ugw-sdk/tools/makeself.sh --gzip --notemp . install.sh "$SDK_DESCRIPTION"
	rm -rf $1
	rm -rf LICENSE
	mkdir $1
	mv install.sh $1
	echo "=======================" >> $1/README
	echo "Steps to build CD image" >> $1/README
	echo "=======================" >> $1/README
	echo "1. Execute ./install.sh and accept the terms and conditions by typing 'yes'" >> $1/README
	echo "2. cd UGW-X.X-SW-CD/Sources/ (Ex: UGW-7.1-SW-CD, version will be changed based on the release)" >> $1/README
	echo "3. Execute ./ugw-prepare-all.sh" >> $1/README
	echo "4. cd UGW_X(Ex:UGW-7.1, version will be changed based on the release)" >> $1/README
	echo "5. select model : ./scripts/ltq_change_environment.sh switch " >> $1/README
	echo "6. ../ugw-prepare-all.sh download ; make -j24 " >> $1/README
	echo "7. After successfull build image can be found under ./bin/lantiq/" >> $1/README
	echo "=======================" >> $1/README
	zip -r $1.zip *
	mkdir -p ../bin/$CDS_COMMON_PATH
	mkdir -p ../bin/lantiq/$CDS_COMMON_PATH
	cp -rf $1.zip ../bin/lantiq/$CDS_COMMON_PATH
	cp -rf $1.zip ../bin/$CDS_COMMON_PATH
	cd - >/dev/null
}

#to create cd
[ "$1" = "CD" ] && {

	[ -n "$2" ] && {
    	    BUILD_PATH=$2
	    pushd . > /dev/null
    	    cd_sw_start
	    create_installer $SW_CD_NAME
	    popd > /dev/null
	    #pushd . > /dev/null
    	    #cd_opensrc_start
	    #create_installer $OPENSW_CD_NAME
	    #popd > /dev/null
	    exit 0
   	} || echo -e " @@@@@@ Please provide build path @@@@@@" ; exit $?
} || [ "$1" = "-o" ] && {
	echo -e "@@@@@@@@ Removing Existing CD environment @@@@@@@@"  
	cp $PROJ_NAME/patches.list . 2>/dev/null
    	rm -rf $PROJ_NAME
	exit
} || [ "$1" = "-i" ] && {
	rm -rf tmp feeds package/feeds
        ./scripts/feeds update -a
        ./scripts/feeds install -a 2>/dev/null
	rm -rf ./package/feeds/thirdparty_sw/
	./scripts/feeds install -f eip97
	./scripts/feeds install -f eip123
        echo "Now please do './scripts/ltq_change_environment.sh switch' to select a model for build."
	exit
} || [ "$1" = "-h" ] && {
	echo -e "$0    <Without argument script will setup CD build enviornment>"
	echo -e "$0 -o <To clean the existing environment>"
	echo -e "$0 -i <To re-install packages>"
	echo -e "$0 CD <build_path> <To create CD from the existing environement>"
	echo -e "$0 install/uninstall  <package name> <to install or uninstall packages>"
	echo -e "$0 thirdparty install/uninstall  <package name> <to install or uninstall packages>"
	exit
} || [ "$1" = "download" ] && {
	make package/download
	make tools/download
	echo -e "\n@@@ now start the compilation with  make -j24  @@@\n"
	exit
} || [ "$1" = "install" ] && {
	[ -n "$2" ] && {
		./scripts/feeds update thirdparty_sw
		./scripts/feeds $1 -f $2
		exit $?
	} || error_print "provide feed name to install"
} || [ "$1" = "uninstall" ] && {
	[ -n "$2" ] && {
		./scripts/feeds $1 $2
		exit $?
	} || error_print "provide feed name to uninstall"
} || [ "$1" = "thirdparty" ] && {
	[ "$2" = "install" ] && {
		thirdparty $2
	} || {
		thirdparty $2 
	}
	exit $?
} || [ -n "$1" ] && {
	error_print "invalid option execute -h option"
}

echo -e "@@@@@@@@ Setting up enviroment for compilation @@@@@@@@"  
environment_prepare 

