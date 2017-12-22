#!/bin/sh

if [ $# -lt 18 ] ; then 

	echo -en "Usage: \n\n"
	echo -en "\$1 = enable/disable(1/0)\n"
	echo -en "\$2 = conn name\n"
	echo -en "\$3 = tunnel name\n"
	echo -en "\$4 = preshared key\n"
	echo -en "\$5 = kmp settings (ikev1/ikev2)\n"
	echo -en "\$6 = remote ip\n"
	echo -en "\$7 = src ip with prefix\n"
	echo -en "\$8 = dst ip with prefix\n"
	echo -en "\$9 = kmp encryption alg (aes128/aes192/aes256/des/3des)\n"
	echo -en "\$10 = kmp hash alg (md5/sha1/sha256/sha384/sha512)\n"
	echo -en "\$11 = kmp dh group (modp768/modp1024/modp1536/modp20148/modp4096)\n"
	echo -en "\$12 = kmp prf alg (prfd5/prfsha1)\n"
	echo -en "\$13 = encryption alg(aes128/aes192/aes256/des/3des) \n"
	echo -en "\$14 = hash alg(md5/sha1/sha256/sha384/sha512)\n"
	echo -en "\$15 = encryption dh grp (modp768/modp1024/modp1536/modp20148/modp4096)\n"
	echo -en "\$16 = life time\n"
	echo -en "\$17 = wan ip\n"
	echo -en "\$18 = wan interface name \n"
	echo -en "\$19 = instance number, if not set then its add operation \n"
	
	echo -en "\nExample: \n\n"
	echo -en "$0 1 pppoe-wan10 tunnel1 abcde ikev2 10.10.200.2 192.168.1.0/24 172.16.12.0/24 aes192 sha1 modp1536 prfsha1 aes192 sha1 modp1536 60 100.100.34.2 wan10 0 \n \n"

	exit
fi 


i=0
if [ -z "${19}" ]; then 

	val=$(uci add ipsec remote)
	uci set ipsec.$val.enabled=$1
	uci set ipsec.$val.conn_ifname=$2
	uci set ipsec.$val.tunnel_name=$3
	uci set ipsec.$val.pre_shared_key=$4
	uci set ipsec.$val.acceptable_kmp=$5
	uci set ipsec.$val.remote_ip=$6
	uci set ipsec.$val.src=$7
	uci set ipsec.$val.dst=$8
	uci set ipsec.$val.kmp_enc_alg=$9
	uci set ipsec.$val.kmp_hash_alg=${10}
	uci set ipsec.$val.kmp_dh_group=${11}
	if [ "$5" = "ikev2" ] ; then
		uci set ipsec.$val.kmp_prf_alg=${12}
	fi
	uci set ipsec.$val.encryption_algorithm=${13}
	uci set ipsec.$val.hash_algorithm=${14}
	uci set ipsec.$val.enc_dh_group=${15}
	uci set ipsec.$val.ipsec_sa_lifetime_time=${16}
	uci set ipsec.$val.my_ip=${17}
else
	i=${19}
	uci set ipsec.@remote[$i].enabled=$1
	uci set ipsec.@remote[$i].conn_ifname=$2
	uci set ipsec.@remote[$i].tunnel_name=$3
	uci set ipsec.@remote[$i].pre_shared_key=$4
	uci set ipsec.@remote[$i].acceptable_kmp=$5
	uci set ipsec.@remote[$i].remote_ip=$6
	uci set ipsec.@remote[$i].src=$7
	uci set ipsec.@remote[$i].dst=$8
	uci set ipsec.@remote[$i].kmp_enc_alg=$9
	uci set ipsec.@remote[$i].kmp_hash_alg=${10}
	uci set ipsec.@remote[$i].kmp_dh_group=${11}
	if [ "$5" = "ikev2" ] ; then
		uci set ipsec.@remote[$i].kmp_prf_alg=${12}
	fi
	uci set ipsec.@remote[$i].encryption_algorithm=${13}
	uci set ipsec.@remote[$i].hash_algorithm=${14}
	uci set ipsec.@remote[$i].enc_dh_group=${15}
	uci set ipsec.@remote[$i].ipsec_sa_lifetime_time=${16}
	uci set ipsec.@remote[$i].my_ip=${17}
fi


uci commit
/opt/lantiq/etc/uci_to_ipsec_config_owrt.sh restart ${18}
