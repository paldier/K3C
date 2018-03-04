#!/bin/sh

target_path=$1

# Print one section from mtidlc, the new header line must be supplied, because its value must contain the new index.
# The search string is the first line of the section we are looking for, after the header
strip_section () {
	header=$1
	section_id=$2
	echo [$header]
	
	awk -v section_id=$section_id 'BEGIN {found=0; search_str="^"section_id} {if ($0 ~ search_str) found=1; if ($0 ~ /\[.*\]/) found=0; if (found==1) print $0}'  $target_path/mtdump.mtidlc
}

# Usage:
strip_section mtidl_enum_0 'binary_type=mtlk_wssa_peer_vendor_t' > $target_path/mtdump.mtidlc_PeerCaps
strip_section mtidl_bitfield_0 'binary_type=mtlk_wssa_net_modes_supported_e' >> $target_path/mtdump.mtidlc_PeerCaps
strip_section mtidl_item_0 'binary_type=mtlk_wssa_drv_peer_capabilities_t' >> $target_path/mtdump.mtidlc_PeerCaps

strip_section mtidl_item_0 'binary_type=mtlk_wssa_drv_peer_stats_t' > $target_path/mtdump.mtidlc_PeerFlowStat

strip_section mtidl_item_0 'binary_type=mtlk_wssa_drv_wlan_stats_t' >> $target_path/mtdump.mtidlc_WLANFlowStatus



