#!/bin/sh

igmp()
{
	echo ""
	echo " =========================== cat /proc/net/igmp =========================== "
	cat /proc/net/igmp
}
igmp6()
{
	echo ""
        echo " =========================== cat /proc/net/igmp6 =========================== "
        cat /proc/net/igmp6
}
vif()
{
	echo ""
	echo " ========================== cat /proc/net/ip_mr_vif ======================= "
	cat /proc/net/ip_mr_vif
}
vif6()
{
	echo ""
        echo " ========================== cat /proc/net/ip6_mr_vif ======================= "
        cat /proc/net/ip6_mr_vif
}
cache()
{
	echo ""
	echo " ======================== cat /proc/net/ip_mr_cache ====================== "
	cat /proc/net/ip_mr_cache
}
cache6()
{
	echo ""
        echo " ======================== cat /proc/net/ip6_mr_cache ====================== "
        cat /proc/net/ip6_mr_cache
}
ppa()
{
	echo ""
	echo " ========================== ppacmd getmcgroups =========================== "
	ppacmd getmcgroups
}
mib()
{
	echo ""
	echo " ========================= cat /proc/eth/mib ============================ "
	cat /proc/eth/mib
}
mc()
{
	echo ""
	echo " ======================== cat /proc/eth/mc ============================== "
	cat /proc/eth/mc
}
switch()
{
	echo ""
	echo " =============== switch_cli IFX_ETHSW_MULTICAST_SNOOP_CFG_GET =================== "
	switch_cli IFX_ETHSW_MULTICAST_SNOOP_CFG_GET   
	echo ""
	echo " =============== switch_cli IFX_ETHSW_MULTICAST_ROUTER_PORT_READ =================== "
        switch_cli IFX_ETHSW_MULTICAST_ROUTER_PORT_READ
	echo ""
	echo " =============== switch_cli IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ =================== "
        switch_cli IFX_ETHSW_MULTICAST_TABLE_ENTRY_READ
}


case $1 in 
	"igmp") igmp ;;
        "igmp6") igmp6 ;;
        "vif")  vif ;;
        "vif6")  vif6 ;;
        "cache") cache ;;
        "cache6") cache6 ;;
        "ppa") ppa ;;
        "mib") mib ;;
        "switch") switch ;;
        "mc") mc ;;
        *)

	echo ""
	echo " ======================== version.sh ============================== "
	version.sh
	echo ""
	echo " =========================== ps ============================ "
	ps

	igmp
	igmp6
	vif
	vif6
	cache
	cache6
	ppa
	mc
	mib
	switch
;;
	*)
;;
esac	
