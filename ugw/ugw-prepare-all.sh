#!/bin/bash

# This path must be inside clone path.
TARGET_DIR="ugw"

_help ()
{
	echo -en "Usage:-\n\t$0  [Options]\n"
	echo -en "\t	No argument: Prepares UGW clone.\n"
	echo -en "\t	This applies buildsystem patches, "`[ -n "$use_ugw_file" ] && echo -n "setup repo symlinks under 'ugw/' folder,\n\t\t"`
	echo -en "setup symlinks in the buildsystem and install all feeds defined in 'feeds.conf' file.\n"
	echo -en "Options:-\n"
	echo -en "\t-o  To clean-up UGW clone.\n"
	echo -en "\t-i  To re-install feeds.\n"
	[ -n "$use_ugw_file" ] && echo -en "\t-u  To update '$TARGET_DIR' repo structure. This deletes '$TARGET_DIR' and re-creates again.\n"
	echo -en "\t-p  Apply buildsystem patches only and exclude setting up symlinks and installing feeds.\n"
	exit $1;
}

customize_openwrt_clone()
{
	# replace_in_owrt	<path inside openwrt> # The path must be identical to the path found in ./ugw/ folder
	replace_in_owrt 	target/linux/lantiq
	replace_in_owrt		target/linux/intel-xway
	replace_in_owrt		target/linux/intel-x86
	replace_in_owrt		target/linux/x86/patches-3.12
	replace_in_owrt		target/linux/x86/puma
	replace_in_owrt		target/linux/x86/base-files.mk
	replace_in_owrt		target/linux/x86/base-files-ugw-newframework
	replace_in_owrt		target/linux/x86/base-files-openwrt
	replace_in_owrt		target/linux/x86/image/Config.in
	replace_in_owrt		target/linux/x86/files
	replace_in_owrt		target/ugw-sdk
	replace_in_owrt		feeds.conf

	echo "./$TARGET_DIR/config" > other_config_path
	[ -n "$UGW_VERSION" ] && {
		echo "$UGW_VERSION" > ugw_version
		echo "$UGW_VERSION" > ugw/ugw_version
	} || {
		echo "7.1.1" > ugw_version
		echo "7.1.1" > ugw/ugw_version
	}
}

install_target_repos()
{
	. $THIS_SCRIPT_PATH/ugw-repo-structure.inc
}

_arg_parse()
{
	opts=""
	optstr=""

	while [ $# != 0 ]; do
		[ -n "$opts" ] && _help 1; # Donot use more than 1 option. If so help and exit 1;
		case "$1" in
			"-o") opts=1;;
			"-i") opts=2;;
			"-u") [ -n "$use_ugw_file" ] && opts=3 || _help 1;;
			"-p") opts=4;;
			"-h"|--help) _help 0;;
			*) optstr=$(echo $1|awk '{ print tolower($1) }');;
		esac
		shift
	done
}

_init_local()
{
	# Check if Openwrt standard buildsystem folders exist or not.
	[ -d target/linux/ -a -f ./package/Makefile -a -d ./scripts -a -d ./include -a -f ./Makefile ] || {
		echo "$0: Error, Run this script inside an Openwrt clone!!"
		exit 1;
	}

	# TOPDIR path
	TOPDIR=`pwd`;

	# TARGET PATH. Example "`pwd`/ugw/"
	TARGET_PATH=$TOPDIR/$TARGET_DIR
	[ -d ../core -a -d ../../openwrt ] && {
		cd ../../
		REPOBASE=`pwd`;
		cd - >/dev/null;
	}

	# This script name and path	
	THIS_SCRIPT=`basename $0`;
	THIS_SCRIPT_PATH=$(cd `dirname $0` && pwd)

	if [ -f $THIS_SCRIPT_PATH/ugw-repo-structure.inc ]; then
		use_ugw_file=1
	elif [ ! -f "$TARGET_DIR/$THIS_SCRIPT" ]; then
		[ -d "$TARGET_DIR" ] || {
			echo "ERROR: unable to find target source code folder '$TARGET_DIR'!!"
			exit 1;
		}
		echo "ERROR: This script is not from '$TARGET_DIR'. Please execute the preparation script under $TARGET_DIR/"
		exit 1;
	fi
}

install_buildsystem_patches()
{
	local patch_guards="ugw"
	cd ../;
	../install.pl
	cd - >/dev/null
	hg qselect $patch_guards
	hg qpop -a >/dev/null 2>/dev/null
	hg qpush -a
}

install_link()
{
	mkdir -p `dirname $TARGET_PATH/$1`;
	ln -sf $REPOBASE/$2 $TARGET_PATH/$1
}

replace_in_owrt()
{
	rm -rf $TOPDIR/$1
	mkdir -p `dirname $TOPDIR/$1`;
	ln -sf $TARGET_PATH/$1 $TOPDIR/$1
}

install_pkgsymlink()
{
	mkdir -p $TARGET_PATH/$1/
	cd $TARGET_PATH/$1/ && {
		ln -sf $REPOBASE/$2;
		cd - >/dev/null
	}
}

install_feedsymlink()
{
	local _dir;
	local _repo_dir="$REPOBASE/$2"
	mkdir -p $TARGET_PATH/$1
	cd $TARGET_PATH/$1/ && {
		for _dir in `ls $_repo_dir/*/ -d 2>/dev/null`; do
			ln -sf $_dir;
		done
		[ -d "$_repo_dir/updates" ] && {
			rm -f updates
			for _dir in `ls $_repo_dir/updates/*/ -d 2>/dev/null`; do
				ln -sf $_dir;
			done
		}
		cd - >/dev/null
	}
}

remove_dir()
{
	[ -n "$1" ] && {
		if [ -d $TARGET_DIR/$1/ ]; then
			rm -rf $TARGET_DIR/$1;
		fi
	}
}

remove_file()
{
	[ -n "$1" ] && {
		rm -f $TARGET_DIR/$1
	}
}

init_target_dir()
{
	[ -n "$use_ugw_file" ] && {
		mkdir -p $TARGET_DIR
		cd $TARGET_DIR/
		hg init
		cd - >/dev/null
	}
}

install_self()
{
	cd $TARGET_DIR/ && {
		ln -sf $THIS_SCRIPT_PATH/$THIS_SCRIPT
		cd $TOPDIR/
	}
}

install_preliminary()
{
	local _ii="$1"
	[ -d "$TARGET_DIR/" ] || {
		echo "Installing buildsystem patches.."
		install_buildsystem_patches
		echo "Initializing $TARGET_DIR .."
		init_target_dir
		_ii=1
	}
	[ -n "$_ii" ] && echo "Installing buildsystem patches completed!!"
}

revert_custom_changes()
{
	unset hg
	[ -h .hg/patches ] && {
		hg qgoto buildsystem/ugw-temperory-from-ugw-prepare.patch 2>/dev/null >/dev/null
		hg qpop 2>/dev/null >/dev/null
		hg qdelete buildsystem/ugw-temperory-from-ugw-prepare.patch 2>/dev/null >/dev/null
		[ -f ../patches/core/buildsystem/ugw-temperory-from-ugw-prepare.patch ] && {
			rm -f ../patches/core/buildsystem/ugw-temperory-from-ugw-prepare.patch
			sed -i '/buildsystem\/ugw-temperory-from-ugw-prepare.patch/d' ../patches/core/series
		}
	}
}

custom_changes_to_revision()
{
	[ -h .hg/patches ] && {
		unset hg
		hg addremove
		hg qnew buildsystem/ugw-temperory-from-ugw-prepare.patch -m "Temp_Commit_Donot_Push"
	}
}

remove_feeds()
{
	rm -rf tmp feeds package/feeds
}

install_all_feeds()
{
	remove_feeds
	./scripts/feeds update -a
	./scripts/feeds install -a 2>/dev/null
	rm -rf package/feeds/ltq_packages tmp/info
	./scripts/feeds install -a 2>/dev/null
	[ -n "$1" ] && {
		echo "Your existing model .config might have corrupted or changed due to the feed re-installation!!"
		echo "So please re-select the model by './scripts/ltq_change_environment.sh switch' for build."
	} || {
		echo "Now please do './scripts/ltq_change_environment.sh switch' to select a model for build."
	}
}

reinstall_feeds()
{
	local _inp;
	echo "WARNING: Re-installing feeds will corrupt your existing '.config' file. Please take a backup of '.config' file if needed."
	echo "After re-installation, please restore backup '.config' file or freshly select model by: './scripts/ltq_change_environment.sh switch'"
	echo -en "Do you want to continue? (y/N) "
	read _inp;
	([ -n "$_inp" ] && [ "$_inp" = "y" -o "$_inp" = "Y" ]) && {
		remove_feeds;
		install_all_feeds 1;
	}
}

clean_pvt_files()
{
        hg revert --all
        hg purge --all
}

reinstall_target_dir_repos()
{
	local _inp;
	echo "WARNING: This operation will delete '$TARGET_DIR' and will re-create again."
	echo "If you have any private files under '$TARGET_DIR', then please take a backup and retry."
	echo -en "Do you want to continue? (y/N) "
	read _inp;
	([ -n "$_inp" ] && [ "$_inp" = "y" -o "$_inp" = "Y" ]) && {
		rm -rf $TARGET_DIR
		remove_feeds
		install_target_repos
		install_self
		install_all_feeds 1
	}
}

clean_all()
{
	local _inp
	echo -en "WARNING: This operation will clean-up/purge all un-checkin files, build_dir, installed feeds etc. Do you want to Continue? (y/N) "
	read _inp;
	([ -n "$_inp" ] && [ "$_inp" = "y" -o "$_inp" = "Y" ]) && {
		rm -rf bin/ build_dir/ dl/ tmp/
		remove_feeds;
		[ -n "$use_ugw_file" ] && {
			rm -rf $TARGET_DIR
		}
		clean_pvt_files >/dev/null 2>/dev/null
		revert_custom_changes;
		../../install.pl -o
		echo "Clean-up of clone completed!!"
	}
}

_arg_process()
{
	if [ -z "$opts" ]; then
		install_preliminary;
		[ -f $TARGET_DIR/$THIS_SCRIPT ] || {
			if [ -n "$use_ugw_file" ]; then
				rm -rf $TARGET_DIR/*
				echo "Setting up repo symlinks under $TARGET_DIR .."
				install_target_repos
				install_self
			fi
		}
		[ -L target/linux/lantiq ] || {
			echo "Customizing openwrt clone.."
			customize_openwrt_clone
			custom_changes_to_revision
		}
		[ -d feeds -a -d package/feeds ] || {
			install_all_feeds;
		}

		echo "UGW clone preparation completed!!"
		echo "* For more options, execute '$0 -h'"
	elif [ "$opts" = "4" ]; then
		install_preliminary 1
	elif [ "$opts" = "3" -a -n "$use_ugw_file" ]; then
		[ -L target/linux/lantiq ] && {
			reinstall_target_dir_repos
		} || echo "Please prepare the clone once before using this option!!";
	elif [ "$opts" = "2" ]; then
		[ -f $TARGET_DIR/$THIS_SCRIPT -a -L target/linux/lantiq ] && {
			reinstall_feeds
		} || echo "Please prepare the clone once before using this option!!";
	elif [ "$opts" = "1" ]; then
		clean_all
	fi
}

_init_local
_arg_parse $@
_arg_process
