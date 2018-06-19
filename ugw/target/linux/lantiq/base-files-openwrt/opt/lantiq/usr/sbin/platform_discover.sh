#!/bin/sh

if [ -r /etc/rc.d/config.sh ]; then
  . /etc/rc.d/config.sh 2> /dev/null
      plat_form=${CONFIG_BUILD_SUFFIX%%_*}
      platform=`echo $plat_form |tr '[:lower:]' '[:upper:]'`
      echo $platform > /tmp/platform.txt
fi
