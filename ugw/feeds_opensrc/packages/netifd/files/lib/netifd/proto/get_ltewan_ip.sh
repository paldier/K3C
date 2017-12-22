#!/bin/sh

#The variable names are important. Don't change them.
F="/tmp/lteip.sh"

get_lte_wan_details()
{
  
  if [ ! -f "$F" ]; then
    x=0
  else
    x=`cat $F`
    x=$((x+1));
    [ $x -ge 3 ] && x=0;
  fi
  echo $x > "$F"
  
  dev="/dev/ttyACM`cat $F`"
  echo "using $dev"
  ATCMD="at+cgcontrdp=" gcom -d $dev -s /etc/gcom/atcmd.gcom|xargs echo|sed -n -e 's/ OK /&/' -e s/"$1 "// -e 's/OK.*//p'
}
                    
get_lte_wan_ip()
{
  # not using the command but fetching details from file
  # containing the details
  #local pdp_details=`get_lte_wan_details`
  local pdp_details=`cat /tmp/at_ip`
                      
  if [ -n "$pdp_details" ]; then
    echo "$pdp_details" > /dev/console
    lte_wan_ip_mask=`echo $(echo $pdp_details|cut -d, -f4)`
    if [ -z "$lte_wan_ip_mask" ]; then
      return 1
    fi
    lte_wan_ip=`echo $lte_wan_ip_mask | grep -m 1 -o '[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}' | awk -F "\n" 'NR==1 {print $1}'`
    
    if [ "$lte_wan_ip" = "0.0.0.0" ] || [ -z "$lte_wan_ip" ]; then
      return 1
    else
      lte_wan_subnet=`echo $lte_wan_ip_mask | grep -m 1 -o '[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}' | awk -F "\n" 'NR==2 {print $1}'`
      lte_wan_gw=`echo $(echo $pdp_details|cut -d, -f5) | grep -m 1 -o '[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}' | awk -F "\n" 'NR==1 {print $1}'`
      lte_wan_dns1=`echo $(echo $pdp_details|cut -d, -f6) | grep -m 1 -o '[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}' | awk -F "\n" 'NR==1 {print $1}'`
      lte_wan_dns2=`echo $(echo $pdp_details|cut -d, -f7) | grep -m 1 -o '[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}' | awk -F "\n" 'NR==1 {print $1}'`
      return 0
    fi
  fi
  echo "Invalid params in response to AT Command"
  return 1
}
