#!/usr/bin/env bash

#
# Copyright (C) 2015-2016 Intel Corporation
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

##
## Expecting "The partition hash doesn't match the OS manifest's hash" warning while playing with appcpu kernel uimage.
##

set -x 
[ $# -eq 5 ] || {
    echo "SYNTAX: $0 <kernel_image_with_path> <tmp directory> <rootfs directory> <image_prefix> <host staging dir>"
    exit 1
}

TARGET_IMG_DIR="`dirname $4`"

BZIMAGE="$1"
WORKDIR="$2"
KERNEL_SIZE="9437184"
ROOTFS_GUID="6FAEF15A-C0F4-4AD2-8619-77649F22DB8D"
UIMAGE_NAME="appcpuImage"
UIMAGE_IMAGE="${WORKDIR}/${UIMAGE_NAME}"
FINAL_UIMAGE_IMAGE="$4-${UIMAGE_NAME}"
ROOTFS_IMAGE_NAME="appcpuRootfs.img"
FINAL_ROOTFS_IMAGE="$4-${ROOTFS_IMAGE_NAME}"
UGW_ROOTFS="rootfs.img"
UGW_KERNEL="uImage"
UGW_FULLIMAGE="fullimage.img"
TARGETFS_NAME="$3"
TARGETFS="${WORKDIR}/${TARGETFS_NAME}"
EXTRA_BOOT_LINE_PARAMETERS="do_overlay vmalloc=382M memmap=4M\$0x00C00000 cma=78M"
UIMAGE_VERSION="3"
FLASH_BLOCK_SIZE="64k"

#Secure image directory
SEC_IMAGE_DIR="${TARGET_IMG_DIR}/sec_img_temp"
KEYS_DIR="$5/keys"
PRIV_PEM_KEY="key_slot0.priv.pem"
PUB_PEM_KEY="key_slot0.pub.pem"

## ATOM images
ROOTFS_IMAGE="${WORKDIR}/${ROOTFS_IMAGE_NAME}"
KERNEL_IMAGE="$4-appcpuKernelfs.img"
FINAL_UGW_ROOTFS_IMAGE="${TARGET_IMG_DIR}/atom_${UGW_ROOTFS}"
FINAL_UGW_KERNEL_IMAGE="${TARGET_IMG_DIR}/atom_${UGW_KERNEL}"
FINAL_UGW_FULL_IMAGE="${TARGET_IMG_DIR}/atom_${UGW_FULLIMAGE}"

## ARM images
ARM_ROOTFS_IMAGE="${TARGET_IMG_DIR}/arm_images/rootfs.squashfs"
ARM_KERNEL_IMAGE="${TARGET_IMG_DIR}/arm_images/zImage"
FINAL_ARM_ROOTFS_IMAGE="${TARGET_IMG_DIR}/arm_${UGW_ROOTFS}"
FINAL_ARM_KERNEL_IMAGE="${TARGET_IMG_DIR}/arm_${UGW_KERNEL}"
FINAL_ARM_FULL_IMAGE="${TARGET_IMG_DIR}/arm_${UGW_FULLIMAGE}"

## BIOS image
BIOS_IMAGE="${TARGET_IMG_DIR}/bios_images/BIOS_Update.bin"
FINAL_BIOS_IMAGE="${TARGET_IMG_DIR}/bios.img"

## Combile Image
FINAL_CONBINE_IMAGE="${TARGET_IMG_DIR}/${UGW_FULLIMAGE}"

## Total Image
FINAL_TOTAL_IMAGE="${TARGET_IMG_DIR}/totalimage.img"

## GWFS image
GWFS_IMAGE="${WORKDIR}/GWFS.bin"
FINAL_GWFS_IMAGE="${TARGET_IMG_DIR}/GWFS.uimg"

## USB image
FINAL_USB_IMAGE="${TARGET_IMG_DIR}/USB_image"

TEMP_DIR=`mktemp -d`
EFI_SCRIPT_FILE=`mktemp`
ARM_BOOT='mm 0x400d0003 3'
ATOM_BOOT='fs%kernel_aid%:\EFI\BOOT\\bzImage console=ttyS0,115200n8 loglevel=8 root=%rootfs% rootwait'
ATOM_BOOT="${ATOM_BOOT} ${EXTRA_BOOT_LINE_PARAMETERS}"

## removed files for the unified image
REMOVED_FILE_LIST="\
	boot \
	usr/grub \
	usr/lib/grub \
	usr/lib/xtables \
	usr/lib/X11 \
	usr/lib/vfs \
	usr/lib/tc \
	usr/lib/pdb \
	bin/qemu \
	bin/qemu-io \
	bin/qemu-nbd \
	bin/qemu-img \
	usr/bin/libvirt_samples \
	usr/bin/mtp-* \
	usr/bin/grub-* \
	usr/sbin/grub-* \
	usr/sbin/iDTSRmtSrv \
	usr/sbin/libvirtd \
	usr/sbin/ubiattach \
	usr/sbin/ubicrc32 \
	usr/sbin/ubidetach \
	usr/sbin/ubiformat \
	usr/sbin/ubimkvol \
	usr/sbin/ubinfo \
	usr/sbin/ubinize \
	usr/sbin/ubirename \
	usr/sbin/ubirmvol \
	usr/sbin/ubirsvol \
	usr/sbin/ubiupdatevol \
	usr/sbin/mkfs.ubifs \
	usr/lib/gconv \
	usr/bin/gdb \
	usr/bin/minicom \
	usr/bin/virsh \
	usr/bin/virt-pki-validate \
	usr/bin/virt-xml-validate \
	usr/bin/localedef \
	usr/bin/locale \
	sbin/ldconfig \
	sbin/sln \
	usr/dtsbin \
	bin/anim_app  \
	bin/audio_player  \
	bin/audio_setup_outputs  \
	bin/cl_app  \
	bin/dvr_app  \
	bin/gdl_samples  \
	bin/renderer_driven_clock_player  \
	bin/tsout_streamer  \
	bin/video_buffer  \
	lib/libgdl.so  \
	lib/libgdl.so.1  \
	lib/libgdl.so.1.0.0  \
	lib/libismd_audio.so  \
	lib/libismd_audio.so.0  \
	lib/libismd_audio.so.0.0.0  \
	lib/libismd_avcap_shim.so  \
	lib/libismd_avcap_shim.so.0  \
	lib/libismd_avcap_shim.so.0.0.0  \
	lib/libismd_bufmon.so  \
	lib/libismd_bufmon.so.0  \
	lib/libismd_bufmon.so.0.0.0  \
	lib/libismd_mux.so  \
	lib/libismd_mux.so.0  \
	lib/libismd_mux.so.0.0.0  \
	lib/libismd_remux.so  \
	lib/libismd_remux.so.0  \
	lib/libismd_remux.so.0.0.0  \
	lib/libismd_tsout.so  \
	lib/libismd_tsout.so.0  \
	lib/libismd_tsout.so.0.0.0  \
	lib/libismd_viddec.so  \
	lib/libismd_viddec.so.0  \
	lib/libismd_viddec.so.0.0.0  \
	lib/libismd_videnc.so  \
	lib/libismd_videnc.so.0  \
	lib/libismd_videnc.so.0.0.0  \
	lib/libismd_vidpproc.so  \
	lib/libismd_vidpproc.so.0  \
	lib/libismd_vidpproc.so.0.0.0  \
	lib/libismd_vidrend.so  \
	lib/libismd_vidrend.so.0  \
	lib/libismd_vidrend.so.0.0.0  \
	lib/libismd_vidsink_direct.so  \
	lib/libismd_vidsink_direct.so.1  \
	lib/libismd_vidsink_direct.so.1.0.0  \
	lib/libpipeline_library.so  \
	lib/libpipeline_library.so.0  \
	lib/libpipeline_library.so.0.0.0  \
	lib/libpsi_handler.so  \
	lib/libpsi_handler.so.0  \
	lib/libpsi_handler.so.0.0.0  \
	lib/libpsi_parser.so  \
	lib/libpsi_parser.so.0  \
	lib/libpsi_parser.so.0.0.0  \
	lib/modules/3.12.17/extra/ismdbufmon.ko  \
	lib/modules/ismdbufmon.ko \
	usr/bin/bison \
	usr/bin/flex \
	usr/bin/m4 \
	usr/sbin/dhcp6r \
	usr/sbin/dhcp6s \
	bin/compile_et \
	bin/mk_cmds \
	lib/libcom_err.so \
	lib/libcom_err.so.2 \
	lib/libcom_err.so.2.1 \
	lib/libe2p.so \
	lib/libe2p.so.2 \
	lib/libe2p.so.2.3 \
	lib/libext2fs.so \
	lib/libext2fs.so.2 \
	lib/libext2fs.so.2.4 \
	lib/libss.so \
	lib/libss.so.2 \
	lib/libss.so.2.0 \
	sbin/badblocks \
	sbin/debugfs \
	sbin/dumpe2fs \
	sbin/e2freefrag \
	sbin/e2image \
	sbin/e2label \
	sbin/e2undo \
	sbin/e4defrag \
	sbin/filefrag \
	sbin/logsave \
	sbin/mklost+found \
	sbin/resize2fs \
	sbin/vigr \
	sbin/vipw \
	usr/bin/chage \
	usr/bin/chattr \
	usr/bin/chfn \
	usr/bin/chsh \
	usr/bin/compile_et \
	usr/bin/expiry \
	usr/bin/faillog \
	usr/bin/lastlog \
	usr/bin/lsattr \
	usr/bin/mk_cmds \
	usr/bin/sg \
	usr/lib/charset/CP437.so \
	usr/lib/charset/CP850.so \
	usr/lib/e2initrd_helper \
	usr/sbin/chgpasswd \
	usr/sbin/chpasswd \
	usr/sbin/groupadd \
	usr/sbin/groupdel \
	usr/sbin/groupmems \
	usr/sbin/groupmod \
	usr/sbin/grpck \
	usr/sbin/grpconv \
	usr/sbin/grpunconv \
	usr/sbin/newusers \
	usr/sbin/pwck \
	usr/sbin/pwconv \
	usr/sbin/pwunconv \
	usr/sbin/useradd \
	usr/sbin/userdel \
	usr/sbin/usermod \
	bin/update-alternatives \
	usr/bin/update-alternatives \
	usr/lib/opkg \
	usr/lib/libxml2.so.2 \
	usr/lib/libxml2.so.2.9.1 \
"

error(){
	>&2 echo "$@"
	do_clear
	exit 1
}

## shared part of strip the rootfs
strip_rootfs_common(){
	touch ${TARGETFS}/etc/uimage
##	for i in ${REMOVED_FILE_LIST} ; do rm -rf ${TARGETFS}/$i ; done
	find ${TARGETFS} -name lib*.a | xargs rm -f
	find ${TARGETFS} -name lib*.la | xargs rm -f
	find ${TARGETFS}/bin -type f -exec chmod 755 {} \;
	find ${TARGETFS}/sbin -type f -exec chmod 755 {} \;
	find ${TARGETFS}/usr/bin -type f -exec chmod 755 {} \;
	find ${TARGETFS}/usr/sbin -type f -exec chmod 755 {} \;
	mkdir -p "${TARGETFS}/lib/modules/3.12.17/extra"
	find ${TARGETFS}/lib/modules/3.12.17/extra -iname "*ko" -exec strip --strip-debug {} \;

}

create_usb_image(){
	local appcpu_kernel=$1
	local appcpu_rootfs=$2

	rm -rf ${FINAL_USB_IMAGE}
	mkdir -p ${FINAL_USB_IMAGE}

	echo "Add startup script for USB"
	cp ${appcpu_kernel} ${FINAL_USB_IMAGE}/appcpu_kernel
	cp `dirname $0`/puma7_usb_image.sh ${FINAL_USB_IMAGE}/create_usb.sh
	echo "Plug-in empty (working) pendrive to Linux machine (prefer ubuntu/debian)"			> ${FINAL_USB_IMAGE}/README
	echo "Extract USB_image.tar.gz tar ball & switch to USB_image directory" 				>> ${FINAL_USB_IMAGE}/README
	echo "Execute: sudo ./create_usb.sh /dev/sdX"											>> ${FINAL_USB_IMAGE}/README
	echo -e "\tReplace \"X\" with appropiate device...can be found dmesg or 'sudo fdisk -l'">> ${FINAL_USB_IMAGE}/README
	echo -e "\nNOTE:"																		>> ${FINAL_USB_IMAGE}/README
	echo "Make sure, you have fdisk => 2.20 or greater version"								>> ${FINAL_USB_IMAGE}/README
	echo -e "\nBooting Process:"															>> ${FINAL_USB_IMAGE}/README
	echo "Plug USB drive to Puma DUT & reset the board"										>> ${FINAL_USB_IMAGE}/README
	echo "Press ESC and stop on EFI shell"													>> ${FINAL_USB_IMAGE}/README
	echo "Check the map & switch to the USB EFI partition EFI\\BOOT directory"				>> ${FINAL_USB_IMAGE}/README
	echo -e "\tIn most of case, it will be FS2. So, commands will be:"						>> ${FINAL_USB_IMAGE}/README
	echo -e "\t\tmode 80 50  => To switch to compatible mode"								>> ${FINAL_USB_IMAGE}/README
	echo -e "\t\tcd FS2:\\\EFI\\\BOOT"														>> ${FINAL_USB_IMAGE}/README
	echo -e "\t\tstartup.nsh"																>> ${FINAL_USB_IMAGE}/README

	>${EFI_SCRIPT_FILE}
	echo 'npcpu start'														>> "${EFI_SCRIPT_FILE}"
	echo 'if not %lasterror% == 0 then'										>> "${EFI_SCRIPT_FILE}"
	echo "${ARM_BOOT}"														>> "${EFI_SCRIPT_FILE}"
	echo 'endif'															>> "${EFI_SCRIPT_FILE}"
	echo -n "bzImage "														>> "${EFI_SCRIPT_FILE}"
	echo -n "%kernel_cmd_line% "											>> "${EFI_SCRIPT_FILE}"
	echo "root=/dev/sda2 ${EXTRA_BOOT_LINE_PARAMETERS}"						>> "${EFI_SCRIPT_FILE}"

	# Add startup.nsh to the EFI partition image
	mcopy -n -o -i "${FINAL_USB_IMAGE}/appcpu_kernel" "${EFI_SCRIPT_FILE}" "::EFI/BOOT/startup.nsh"

	cp ${appcpu_rootfs} ${FINAL_USB_IMAGE}/appcpu_rootfs
	tar czf ${FINAL_USB_IMAGE}.tar.gz -C `dirname ${FINAL_USB_IMAGE}` `basename ${FINAL_USB_IMAGE}`
	rm -rf ${FINAL_USB_IMAGE}
}

build_efi_partition_image() {
	echo "Building EFI partition image"

	echo Temporary process -- package EFI partition image instead of just
	echo the kernel image

	dd if=/dev/zero of="${KERNEL_IMAGE}" bs=${KERNEL_SIZE} count=1
	mkfs.vfat "${KERNEL_IMAGE}"

	echo Create directory structure
	mmd -i "${KERNEL_IMAGE}" "EFI"
	mmd -i "${KERNEL_IMAGE}" "EFI/BOOT"

	echo Add kernel image
	mcopy -i "${KERNEL_IMAGE}" "${BZIMAGE}" "::EFI/BOOT/bzImage.efi"

	echo Add startup script

	>${EFI_SCRIPT_FILE}
	echo 'npcpu start'                            >> "${EFI_SCRIPT_FILE}"
	echo 'if not %lasterror% == 0 then'           >> "${EFI_SCRIPT_FILE}"
	echo "${ARM_BOOT}"                            >> "${EFI_SCRIPT_FILE}"
	echo 'endif'                                  >> "${EFI_SCRIPT_FILE}"
	echo 'bootkernel -c %kernel_cmd_line% root=%rootfs% mtdparts= phram.phram= '  "${EXTRA_BOOT_LINE_PARAMETERS}" >> "${EFI_SCRIPT_FILE}"
	echo 'if not %lasterror% == 0 then'           >> "${EFI_SCRIPT_FILE}"
	# Add Atom boot command
	echo "${ATOM_BOOT}"                           >> "${EFI_SCRIPT_FILE}"
	echo 'endif'                                  >> "${EFI_SCRIPT_FILE}"

	# Add startup.nsh to the EFI partition image
	mcopy -i "${KERNEL_IMAGE}" "${EFI_SCRIPT_FILE}" "::EFI/BOOT/startup.nsh"
}

## strip the rootfs
strip_rootfs(){
	# Fix etc files.
	install -d  ${TARGETFS}/nvram/etc
	strip_rootfs_etc passwd
	strip_rootfs_etc group
	strip_rootfs_etc fstab
	strip_rootfs_etc shadow

	strip_rootfs_common
}

# Replace the specified etc configuration file with a symbolic link to one
# in the nvram partition. This creates a file whose contents duplicate the
# original file as '/etc/${FILE}.orig' and creates a symbolic link from
# '/etc/${FILE}' to '/nvram/etc/${FILE}'. This is to make up for the fact
# that '/etc/' is read-only on our system.
strip_rootfs_etc() {
	local file=${1}
	touch ${TARGETFS}/etc/${file}
	install -c  ${TARGETFS}/etc/${file} ${TARGETFS}/etc/${file}.orig
	install -c  ${TARGETFS}/etc/${file} ${TARGETFS}/nvram/etc/${file}
	chmod 644 ${TARGETFS}/etc/${file}.orig
	rm -f ${TARGETFS}/etc/${file}
	ln -sf /nvram/etc/${file} ${TARGETFS}/etc/${file}
}

build_gwfs_partition_image()
{
	echo "Building GWFS partition image"

	rm -f ${GWFS_IMAGE}

	## Generate 1 MB image
	dd if=/dev/zero of="${GWFS_IMAGE}" bs=1024 count=1024
}

gen_uimage()
{
	local files="$1"
	local name="$2"
	local target="$3"

	mkimage_grx750 --version ${UIMAGE_VERSION} --file "${files}" --name "${name}" --image "${target}"
	[ $? -ne 0 ] && { error "Failed: mkimage_grx750 $*"; }
	chmod 644 ${target}
}

gen_osmanifest()
{
	local img_name="$1"
	local img_type="$2"
	local img_location="$3"

	#Create app.kernel.unsignedmanifest:
	os_manifest_tool generate_unsigned_manifest -u=${SEC_IMAGE_DIR}/${img_name} -image=${img_location} -type=${img_type} -keyIndex=0 -version=0
	[ $? -ne 0 ] && { error "os_manifest_tool generate_unsigned_manifest failed"; }

	#Create app.kernel.signature:
	os_manifest_tool generate_signature -u=${SEC_IMAGE_DIR}/${img_name}.unsignedmanifest -private=${KEYS_DIR}/${PRIV_PEM_KEY} -sig=${SEC_IMAGE_DIR}/${img_name}
	[ $? -ne 0 ] && { error "os_manifest_tool generate_signature failed"; }

	#Create app.kernel.osmanifest:
	os_manifest_tool attach_signature_to_unsigned_manifest -u=${SEC_IMAGE_DIR}/${img_name}.unsignedmanifest -public=${KEYS_DIR}/${PUB_PEM_KEY} -sig=${SEC_IMAGE_DIR}/${img_name}.signature -c=${SEC_IMAGE_DIR}/${img_name}
	[ $? -ne 0 ] && { error "os_manifest_tool attach_signature_to_unsigned_manifest failed"; }
}

do_compile(){
	build_efi_partition_image
	build_gwfs_partition_image
	strip_rootfs
	mksquashfs4 ${TARGETFS} ${ROOTFS_IMAGE} -b ${FLASH_BLOCK_SIZE} -comp xz -noappend -root-owned

	# Generate os manifest files
	rm -rf ${SEC_IMAGE_DIR}
	mkdir -p ${SEC_IMAGE_DIR}
	gen_osmanifest "app.kernel"		"0"	"${BZIMAGE}"
	gen_osmanifest "app.rootfs"		"1"	"${ROOTFS_IMAGE}"
	gen_osmanifest "npcpu.kernel"	"3"	"${ARM_KERNEL_IMAGE}"
	gen_osmanifest "npcpu.rootfs"	"4"	"${ARM_ROOTFS_IMAGE}"

	## ATOM Images
	gen_uimage "${SEC_IMAGE_DIR}/app.kernel.osmanifest,os_manifest:${KERNEL_IMAGE},appcpu_kernel" "ATOM_${UGW_KERNEL}" "${FINAL_UGW_KERNEL_IMAGE}"
	gen_uimage "${SEC_IMAGE_DIR}/app.rootfs.osmanifest,os_manifest:${ROOTFS_IMAGE},appcpu_rootfs" "ATOM_${UGW_ROOTFS}" "${FINAL_UGW_ROOTFS_IMAGE}"
	gen_uimage "${SEC_IMAGE_DIR}/app.kernel.osmanifest,os_manifest:${KERNEL_IMAGE},appcpu_kernel:${SEC_IMAGE_DIR}/app.rootfs.osmanifest,os_manifest:${ROOTFS_IMAGE},appcpu_rootfs" "ATOM_${UGW_FULLIMAGE}" "${FINAL_UGW_FULL_IMAGE}"

	## ARM Images
	gen_uimage "${SEC_IMAGE_DIR}/npcpu.kernel.osmanifest,os_manifest:${ARM_KERNEL_IMAGE},npcpu_kernel" "ARM_${UGW_KERNEL}" "${FINAL_ARM_KERNEL_IMAGE}"
	gen_uimage "${SEC_IMAGE_DIR}/npcpu.rootfs.osmanifest,os_manifest:${ARM_ROOTFS_IMAGE},npcpu_rootfs" "ARM_${UGW_ROOTFS}" "${FINAL_ARM_ROOTFS_IMAGE}"
	gen_uimage "${SEC_IMAGE_DIR}/npcpu.kernel.osmanifest,os_manifest:${ARM_KERNEL_IMAGE},npcpu_kernel:${SEC_IMAGE_DIR}/npcpu.rootfs.osmanifest,os_manifest:${ARM_ROOTFS_IMAGE},npcpu_rootfs" "ARM_${UGW_FULLIMAGE}" "${FINAL_ARM_FULL_IMAGE}"

	## BIOS Image
	gen_uimage "${BIOS_IMAGE},uefi_bundle" "BIOS_Update.bin" "${FINAL_BIOS_IMAGE}"
	
	## SUIF Image
	#gen_uimage "${TARGET_IMG_DIR}/metadata.txt" "SUIF_Info" "${TARGET_IMG_DIR}/SUIF.img"
	#rm -rf ${TARGET_IMG_DIR}/metadata.txt

	## Combine Image (Fullimage)
	gen_uimage "${SEC_IMAGE_DIR}/app.kernel.osmanifest,os_manifest:${KERNEL_IMAGE},appcpu_kernel:${SEC_IMAGE_DIR}/app.rootfs.osmanifest,os_manifest:${ROOTFS_IMAGE},appcpu_rootfs:${SEC_IMAGE_DIR}/npcpu.kernel.osmanifest,os_manifest:${ARM_KERNEL_IMAGE},npcpu_kernel:${SEC_IMAGE_DIR}/npcpu.rootfs.osmanifest,os_manifest:${ARM_ROOTFS_IMAGE},npcpu_rootfs" "${UGW_FULLIMAGE}" "${FINAL_CONBINE_IMAGE}"

	## Total Image
	gen_uimage "${SEC_IMAGE_DIR}/app.kernel.osmanifest,os_manifest:${KERNEL_IMAGE},appcpu_kernel:${SEC_IMAGE_DIR}/app.rootfs.osmanifest,os_manifest:${ROOTFS_IMAGE},appcpu_rootfs:${SEC_IMAGE_DIR}/npcpu.kernel.osmanifest,os_manifest:${ARM_KERNEL_IMAGE},npcpu_kernel:${SEC_IMAGE_DIR}/npcpu.rootfs.osmanifest,os_manifest:${ARM_ROOTFS_IMAGE},npcpu_rootfs:${BIOS_IMAGE},uefi_bundle" "Puma7UGW_TotalImage" "${FINAL_TOTAL_IMAGE}"

	## GWFS Image
	gen_uimage "${GWFS_IMAGE},gwfs" "GWFSImage" "${FINAL_GWFS_IMAGE}"

	create_usb_image "${KERNEL_IMAGE}" "${ROOTFS_IMAGE}"
}

do_clear(){
	rm -rf ${SEC_IMAGE_DIR}
	rm -rf ${TARGETFS}
	rm -f ${UIMAGE_IMAGE} ${ROOTFS_IMAGE}
	rm -rf ${TEMP_DIR} ${EFI_SCRIPT_FILE}
	rm -rf ${FINAL_USB_IMAGE}
}

do_compile
do_clear
