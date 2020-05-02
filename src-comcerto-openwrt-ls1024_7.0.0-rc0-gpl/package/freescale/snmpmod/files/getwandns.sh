#!/bin/sh

. /lib/functions/network.sh

count=0

network_active_wan_interfaces allWANiface
for waniface in $allWANiface             
do
    #echo "allWANiface = $allWANiface"
    local realDev                     
    count=$((count+1))                                          
    network_get_waniface realDev $waniface
    network_get_dnsserver dnsserver $waniface
    if [ -n "$dnsserver" ]; then
        index=0
        #echo "$waniface"
        wiface=`echo $waniface | awk '{ print toupper($0) }'`
        #echo "wiface=$wiface"
        ifindex=`confd_cmd -c "mget /interfaces-state/interface{$wiface}/if-index"`>/dev/null 2>&1
        for i in $dnsserver
        do
           echo "$index,$ifindex,$i"
           index=$((index+1))
        done
    fi
    #local wanipaddr=""                                               
    #network_get_wanip wanipaddr $waniface                            
done

