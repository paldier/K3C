#!/bin/sh 
 
. /lib/functions.sh
. /usr/share/libubox/jshn.sh

WanUpFlag=0 
BaseFile="/var/racoon2.conf"

g_all_my_ip=""
g_ifname=""
g_wanstatus=0

TunnelConfig() {

  local ConfigFile="/var/ipsec_policies_$1"
  local PSKFile="/var/passwd_$1.psk"
  
  config_get_bool enabled 		"$1" enabled 0
  if [ $enabled -eq 0 ]; then
  	return 0;
  fi
  
  config_get my_ip			"$1" my_ip ""
  if [ -z "$my_ip" ];then
  	return 0;
  fi

  config_get name			"$1" tunnel_name 
  config_get acceptable_kmp     	"$1" acceptable_kmp
  config_get remote_ip			"$1" remote_ip
  config_get kmp_enc_alg		"$1" kmp_enc_alg
  config_get kmp_prf_alg		"$1" kmp_prf_alg
  config_get kmp_hash_alg		"$1" kmp_hash_alg
  config_get exchange_mode      	"$1" exchange_mode
  config_get kmp_dh_group      		"$1" kmp_dh_group
  config_get kmp_auth_method		"$1" kmp_auth_method
  config_get pre_shared_key     	"$1" pre_shared_key "abcde"
  config_get src			"$1" src
  config_get dst 			"$1" dst
  config_get encryption_algorithm 	"$1" encryption_algorithm
  config_get hash_algorithm 		"$1" hash_algorithm
  config_get ipsec_sa_lifetime_time	"$1" ipsec_sa_lifetime_time "60"

  kmp_auth_method="psk";  
  echo -n "$pre_shared_key" > $PSKFile;

  echo "remote $name {" > $ConfigFile
  echo "acceptable_kmp $acceptable_kmp;" >> $ConfigFile
  echo "  $acceptable_kmp {" >> $ConfigFile
  echo "  	my_id ipaddr $my_ip;" >> $ConfigFile
  echo "	peers_id ipaddr $remote_ip;" >> $ConfigFile
  echo "	peers_ipaddr $remote_ip;" >> $ConfigFile
  echo " 	kmp_enc_alg $kmp_enc_alg;" >> $ConfigFile
  if [ $acceptable_kmp == "ikev1" ]; then
  	echo "  	kmp_hash_alg $kmp_hash_alg;" >> $ConfigFile
  	echo "  	exchange_mode main;" >> $ConfigFile
  fi
  if [ $acceptable_kmp == "ikev2" ]; then
  	echo "  	kmp_prf_alg $kmp_prf_alg;" >> $ConfigFile
  	echo "  	kmp_hash_alg $kmp_hash_alg;" >> $ConfigFile
  fi
  echo "	kmp_dh_group $kmp_dh_group;" >> $ConfigFile
  echo "	kmp_auth_method $kmp_auth_method;" >> $ConfigFile
  echo "	pre_shared_key $PSKFile;" >> $ConfigFile
  echo "  };" >> $ConfigFile
  echo "  selector_index ${name}_out;" >> $ConfigFile
  echo "};" >> $ConfigFile
  
  echo "selector ${name}_out {" >> $ConfigFile
  echo "  direction outbound;" >> $ConfigFile
  echo "  src $src;" >> $ConfigFile
  echo "  dst $dst;" >> $ConfigFile
  echo "  upper_layer_protocol any;" >> $ConfigFile
  echo "  policy_index $name;" >> $ConfigFile
  echo "};" >> $ConfigFile
  
  echo "selector ${name}_in {" >> $ConfigFile
  echo "  direction inbound;" >> $ConfigFile
  echo "  src $dst;" >> $ConfigFile
  echo "  dst $src;" >> $ConfigFile
  echo "  upper_layer_protocol any;" >> $ConfigFile
  echo "  policy_index $name;" >> $ConfigFile
  echo "};" >> $ConfigFile

  echo "policy $name {" >> $ConfigFile
  echo "  action auto_ipsec;" >> $ConfigFile
  echo "  remote_index $name;" >> $ConfigFile
  echo "  ipsec_mode tunnel;" >> $ConfigFile
  echo "  ipsec_index ipsec_esp;" >> $ConfigFile
  echo "  ipsec_level require;" >> $ConfigFile
  echo "  peers_sa_ipaddr $remote_ip;" >> $ConfigFile
  echo "  my_sa_ipaddr $my_ip;" >> $ConfigFile
  echo "};" >> $ConfigFile
  
  echo "ipsec ipsec_esp {" >> $ConfigFile
  echo "  ipsec_sa_lifetime_time $ipsec_sa_lifetime_time min;" >> $ConfigFile
  echo "  sa_index esp_0;" >> $ConfigFile
  echo "};" >> $ConfigFile
  
  echo "sa esp_0 {" >> $ConfigFile
  echo "  sa_protocol esp;" >> $ConfigFile
  echo "  esp_enc_alg $encryption_algorithm;" >> $ConfigFile
  echo "  esp_auth_alg $hash_algorithm;" >> $ConfigFile
  echo "};" >> $ConfigFile
  
}
 
BasicConfig() {
  local resolver
  local unix
  local spmd_password

  config_get resolver		"$1" resolver "off"
  config_get unix		"$1" unix
  config_get spmd_password	"$1" spmd_password
  
  echo "resolver"
  echo "{"
  echo "	resolver $resolver;"
  echo "};"
  echo "interface"
  echo "{"
  echo "	ike {"
  echo "		$g_all_my_ip"
  echo "	};"
  echo "	spmd{"
  echo "		unix $unix;"
  echo " 	};"
  echo "	spmd_password $spmd_password;"
  echo "};"
  echo "include \"/var/ipsec_policies_*\";"
  
}

getAllMyIp() {
  local my_ip
  local enabled
  local conn_ifname
  local uci_name

  config_get_bool enabled "$1" enabled  
  if [ $enabled -ne 1 ]; then
       	uci set ipsec.$1.my_ip=""
       	uci commit ipsec
  	return 0;
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
  json_load "$(ifstatus $uci_name)"
  
  json_get_var up "up"
  if [ $up -eq 1 ]; then
          json_get_var conn_ifname "l3_device"
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

  iptables -D INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 2>/dev/null
  iptables -I INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 
  iptables -t nat -D POSTROUTING -s $src -d $dst -m policy --dir out --pol  ipsec --proto esp --mode tunnel -j ACCEPT 2>/dev/null
  iptables -t nat -I POSTROUTING -s $src -d $dst -m policy --dir out --pol  ipsec --proto esp --mode tunnel -j ACCEPT 

}

RemoveIptableRules ()
{
  config_get my_ip                      "$1" my_ip ""
  config_get remote_ip                  "$1" remote_ip
  config_get src                        "$1" src
  config_get dst                        "$1" dst

  iptables -D INPUT -s $remote_ip/32 -d $my_ip/32 -j ACCEPT 2>/dev/null
  iptables -t nat -D POSTROUTING -s $src -d $dst -m policy --dir out --pol  ipsec --proto esp --mode tunnel -j ACCEPT 2>/dev/null
}

start() {
  
  config_load ipsec

  config_foreach getAllMyIp remote
  if [ $WanUpFlag -ne 1 ];then
	exit 0;
  fi

  config_load ipsec
  config_foreach BasicConfig ipsec > $BaseFile 
  config_foreach TunnelConfig remote

  cp -f /etc/spmd.pwd /var/spmd.pwd 2>/dev/null
  /usr/sbin/spmd -f /var/racoon2.conf
  sleep 2
  /usr/sbin/iked -f /var/racoon2.conf

  config_foreach SetIptableRules remote
}
 
stop() {
  rm -f /var/ipsec_policies_* 2>/dev/null
  rm -f /var/passwd_* 2>/dev/null
  rm -f /var/racoon2.conf 2>/dev/null
  rm -f /var/spmd.pwd 2>/dev/null
  
  killall -9 spmd  2>/dev/null
  killall -9 iked  2>/dev/null

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
