#!/bin/sh

. /lib/functions/network.sh

prefix2mask ()
{
   # Number of args to shift, 255..255, first non-255 byte, zeroes
   set -- $(( 5 - ($1 / 8) )) 255 255 255 255 $(( (255 << (8 - ($1 % 8))) & 255 )) 0 0 0
   [ $1 -gt 1 ] && shift $1 || shift
   echo ${1-0}.${2-0}.${3-0}.${4-0}
}

getIfaceInfo()
{
  local oper=$1
  ifstatus=""
  ip=""
  mask4=""
  v4_phy_l2=""
  v4_phy_l3=""
  dns14=""
  dns16=""
  gw=""
  mac_addr=""

  __network_getall_stats_v4 $oper ifstatus ip mask4 v4_phy_l2 v4_phy_l3 dns14 dns16 gw

  mac_addr=`cat /sys/class/net/$v4_phy_l2/address`

  #echo v4,$oper,$ip4_count,$ip, $mask4,$gw,$v4_phy_l2,$v4_phy_l3, $mac_addr

  mask=$(prefix2mask $mask4)
  echo $oper,$ip,$mask,$mac_addr,$gw
}

opt=$1

case $opt in

    lan)
        getIfaceInfo vlan1
    ;;
    wan)
        getIfaceInfo wan1
    ;;
    *)
    ;;
esac

