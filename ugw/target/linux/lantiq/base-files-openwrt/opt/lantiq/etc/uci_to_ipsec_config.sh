#!/bin/sh 
 
. /etc/functions.sh
. /usr/share/libubox/jshn.sh

WanUpFlag=0 
BaseFile="/etc/strongswan.conf" #can be coped statically, need to check if there is any var in this

tunel_no=1
g_all_my_ip=""
g_ifname=""
g_wanstatus=0
g_ver=4

TunnelConfig() {

  local ConfigFile="/etc/ipsec.conf"
  local PSKFile="/etc/ipsec.secrets"
  
  config_get_bool enabled 		"$1" enabled 0
  if [ $enabled -eq 0 ]; then
  	return 0;
  fi
  
  config_get my_ip			"$1" my_ip ""
  if [ -z "$my_ip" ];then
  	return 0;
  fi
 
  config_get acceptable_kmp     	"$1" acceptable_kmp
  config_get tunnel_name     		"$1" tunnel_name
  config_get remote_ip			"$1" remote_ip
  config_get kmp_enc_alg		"$1" kmp_enc_alg
  config_get kmp_prf_alg		"$1" kmp_prf_alg
  config_get kmp_hash_alg		"$1" kmp_hash_alg
  config_get exchange_mode      	"$1" exchange_mode
  config_get kmp_dh_group      		"$1" kmp_dh_group
  config_get enc_dh_group      		"$1" enc_dh_group
  config_get enc_esn           		"$1" enc_esn
  config_get kmp_auth_method		"$1" kmp_auth_method
  config_get pre_shared_key     	"$1" pre_shared_key "abcde"
  config_get src			"$1" src
  config_get dst 			"$1" dst
  config_get encryption_algorithm 	"$1" encryption_algorithm
  config_get hash_algorithm 		"$1" hash_algorithm
  config_get ipsec_sa_lifetime_time	"$1" ipsec_sa_lifetime_time "60"

  kmp_auth_method="psk";  
	esn=""
	enc_dh="-$enc_dh_group"

	if [ $enc_esn -eq 1  -a $acceptable_kmp = ikev2 ]; then
		esn="-esn"
	fi

	if [ $enc_dh_group = "none" ]; then
		enc_dh=""
	fi


  echo -n "$my_ip $remote_ip : PSK $pre_shared_key" > $PSKFile;

	echo "" >> $ConfigFile
	echo "conn tunnel${tunel_no}" >> $ConfigFile
	echo -n "	ikelifetime=$ipsec_sa_lifetime_time" >> $ConfigFile
	echo "m" >> $ConfigFile
	if [ $acceptable_kmp = ikev2 ]; then
		echo "	ike=$kmp_enc_alg-$kmp_hash_alg-$kmp_prf_alg-$kmp_dh_group!" >> $ConfigFile
	else
		echo "	ike=$kmp_enc_alg-$kmp_hash_alg-$kmp_dh_group!" >> $ConfigFile
	fi
	echo "	esp=$encryption_algorithm-$hash_algorithm$enc_dh$esn!" >> $ConfigFile
	echo "	keyexchange=$acceptable_kmp" >> $ConfigFile
	echo "	left=$my_ip" >> $ConfigFile
	echo "	leftsubnet=$src" >> $ConfigFile
	echo "	leftid=$my_ip" >> $ConfigFile
	echo "	leftfirewall=yes" >> $ConfigFile
	echo "	right=$remote_ip" >> $ConfigFile
	echo "	rightsubnet=$dst" >> $ConfigFile
	echo "	rightid=$remote_ip" >> $ConfigFile
	tunel_no=`expr ${tunel_no} + 1`
}

getAllMyIp() {
  local my_ip
  local enabled
  local conn_ifname
  local uci_name
  local remote_ip

  config_get_bool enabled "$1" enabled  
  if [ $enabled -ne 1 ]; then
       	uci set ipsec.$1.my_ip=""
       	uci commit ipsec
  	return 0;
  fi

	config_get remote_ip "$1" remote_ip
	echo $remote_ip | grep -q ':'
	if [ $? -eq 0 ]; then
		g_ver=6
	fi

  config_get conn_ifname "$1" conn_ifname
  json_init
  json_load "$(/opt/lantiq/usr/sbin/cgigetutil Device.X_LANTIQ_COM_NwHardware.WANConnection)"
  json_get_keys _Objects "Objects"
  len=${#_Objects}
  len=$(( $len / 2 ))
  len=$(( $len + 1 ))
  
  i=1
  while [ $i -lt $len ]; do
  	json_init
  	json_load "$(/opt/lantiq/usr/sbin/cgigetutil Device.X_LANTIQ_COM_NwHardware.WANConnection)"
  	json_get_keys _Objects "Objects"
  	if [ -n "$_Objects" ]; then
  		json_select "Objects"
  		json_select $i
  		json_get_var ObjName "ObjName"
  		json_get_keys _Params "Param"
  		if [ -n "$_Params" ]; then
  			json_select "Param"
  			json_select 1
  			json_get_var paramvalue "ParamValue"
  			if [ "$conn_ifname" = "$paramvalue" ]; then
  				json_select ..
  				json_select 2
  				json_get_var uci_name "ParamValue"
  			fi
  		fi
  	fi
  	i=$(( $i + 1 ))
  done
  
  json_init
  if [ $g_ver -eq 6 ]; then
  	json_load "$(ifstatus ${uci_name}v6)"
  else
  	json_load "$(ifstatus $uci_name)"
  fi
  
  json_get_var up "up"
  if [ $up -eq 1 ]; then
			json_get_var conn_ifname "l3_device"
  		if [ $g_ver -eq 6 ]; then
          json_get_keys _ipv6_address_entries "ipv6-address"
          if [ -n "$_ipv6_address_entries" ]; then
          	json_select "ipv6-address"
          	json_select 1
          	json_get_var ipaddr "address"
          	uci set ipsec.$1.my_ip=$ipaddr
          	uci commit ipsec
						eval g_all_my_ip="'$g_all_my_ip'$ipaddr\;"
						WanUpFlag=1;
          fi
			else
					json_get_keys _ipv4_address_entries "ipv4-address"
          if [ -n "$_ipv4_address_entries" ]; then
          	json_select "ipv4-address"
          	json_select 1
          	json_get_var ipaddr "address"
          	uci set ipsec.$1.my_ip=$ipaddr
          	uci commit ipsec
						eval g_all_my_ip="'$g_all_my_ip'$ipaddr\;"
						WanUpFlag=1;
          fi
			fi
  else
  	uci set ipsec.$1.my_ip=""
  	uci commit ipsec
  fi
  
}

SetIptableRules ()
{
  config_get_bool enabled               "$1" enabled 0
  if [ $enabled -eq 0 ]; then
        return 0;
  fi

  config_get my_ip                      "$1" my_ip ""
  if [ -z "$my_ip" ];then
        return 0;
  fi

  config_get remote_ip                  "$1" remote_ip
  config_get src			"$1" src
  config_get dst			"$1" dst

	if [ $g_ver -eq 6 ]; then
		ip6tables -D INPUT -s $remote_ip -d $my_ip -j ACCEPT 2>/dev/null
		ip6tables -I INPUT -s $remote_ip -d $my_ip -j ACCEPT 
	else
		iptables -D INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 2>/dev/null
		iptables -I INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 
		iptables -t nat -D POSTROUTING -s $src -d $dst -m policy --dir out --pol  ipsec --proto esp --mode tunnel -j ACCEPT 2>/dev/null
		iptables -t nat -I POSTROUTING -s $src -d $dst -m policy --dir out --pol  ipsec --proto esp --mode tunnel -j ACCEPT 
	fi

}

RemoveIptableRules ()
{
  config_get my_ip                      "$1" my_ip ""
  config_get remote_ip                  "$1" remote_ip
  config_get src                        "$1" src
  config_get dst                        "$1" dst

	if [ $g_ver -eq 6 ]; then
		ip6tables -D INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 2>/dev/null
	else
		iptables -D INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 2>/dev/null
		iptables -t nat -D POSTROUTING -s $src -d $dst -m policy --dir out --pol  ipsec --proto esp --mode tunnel -j ACCEPT 2>/dev/null
	fi
}

start() {
  
  config_load ipsec

  config_foreach getAllMyIp remote

  if [ $WanUpFlag -ne 1 ];then
		exit 0;
  fi

	local ConfigFile="/etc/ipsec.conf"

	#additional params
	keylife="20"
	rekeymargin="3"

	echo "config setup" >> $ConfigFile
	echo "conn %default" >> $ConfigFile
	echo -n "	keylife=$keylife" >> $ConfigFile
	echo "m" >> $ConfigFile
	echo -n "	rekeymargin=$rekeymargin" >> $ConfigFile
	echo "m" >> $ConfigFile
	echo "	keyingtries=1" >> $ConfigFile
	echo "	authby=secret" >> $ConfigFile #check hardcode
	echo "	replay_window=1024" >> $ConfigFile
	echo "	mobike=no" >> $ConfigFile
	echo "	auto=start" >> $ConfigFile

  config_load ipsec
  config_foreach TunnelConfig remote

	ipsec start --conf /etc/ipsec.conf

  config_foreach SetIptableRules remote
}
 
stop() {
	ipsec stop --conf /etc/ipsec.conf

  rm -f /etc/ipsec.conf 2>/dev/null
  rm -f /etc/ipsec.secrets 2>/dev/null
  
  config_load ipsec
  config_foreach RemoveIptableRules remote
}
 
restart() {
  stop
  start
}

checkwaniface() {
  config_get_bool enabled "$1" enabled 0
  if [ $enabled -eq 0 ]; then
  	return 0;
  fi
  
  config_get conn_ifname "$1" conn_ifname ""
  
  if [ "$conn_ifname" = "$g_ifname" ]; then
  	g_wanstatus=1;
  fi
}

wanstatus() {

  config_load ipsec

  config_foreach checkwaniface remote

  if [ $g_wanstatus -eq 1 ];then
  	restart
  fi
}

factoryreset() {
  uci delete ipsec.$1
  uci commit ipsec
}

reset() {
  config_load ipsec
  config_foreach factoryreset remote
}

if [ $1 = "wanstatus" ];then
  if [ -z "$2" ];then
        exit 0
  else
        g_ifname=$2
  fi
fi

$1
