#!/bin/bash

SW_AS_SOURCE=1

if [ ! -n "$1" ] ; then
	echo "Usage : create_customer_package.sh SOURCE_PATH"
	exit 0
fi

#
# Remove sources files which are not to be distributed 
#

[ ${SW_AS_SOURCE} -eq 0 ] && {
  rm -rf ppa_api/ppa_api_sw_accel.c
  rm -rf ppa_api/ppa_api_sw_accel_module.c
  rm -rf ppa_api/ppa_api_sw_accel_module.h
  rm -rf ppa_api/ppa_api_tmpl_buf.c
  rm -rf ppa_api/ppa_api_tmplbuf_module.c
  rm -rf ppa_api/swa_ipproto.h
#
# Copy required binary modules into CD
#
  if [ -f $1/drivers/net/lantiq_ppa/ppa_api/ppa_api_sw_accel_mod.ko ] ; then
    cp -f $1/drivers/net/lantiq_ppa/ppa_api/ppa_api_sw_accel_mod.ko ppa_api/ppa_api_sw_accel_mod.ko
  fi

  if [ -f $1/drivers/net/lantiq_ppa/ppa_api/ppa_api_tmplbuf.ko ] ; then
    cp -f $1/drivers/net/lantiq_ppa/ppa_api/ppa_api_tmplbuf.ko ppa_api/ppa_api_tmplbuf.ko
  fi

  if [ -x /bin/sed ] ; then
    str=`grep -n CUT_HERE_FOR_CD ppa_api/Makefile`
    n=${str%%:*}
    /bin/sed -i ''${n}',$d' ppa_api/Makefile
  else
    echo "ppa_api: /bin/sed utility not found"
    exit 1
  fi
} 

rm -rf create_customer_package.sh

