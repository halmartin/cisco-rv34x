#!/bin/sh


rm -f /tmp/stats/dhcp_bind_stats

echo `cut -f1,2,3,4 -d' ' /tmp/dhcp.leases > /tmp/tmpresult`

convertsecs() {
    h=`expr $1 / 3600`
    m=`expr $1  % 3600 / 60`
    s=`expr $1 % 60`
    printf "%02d hours %02d minutes %02d seconds\n" $h $m $s
}


while read first second third forth
do

oIFS="$IFS";IFS="-"
set $forth
IFS="$oIFS"

if [ $1 = "static" ]
then
forth="0"
else
forth="1"
fi

cursecs=`date +%s`
remsecs=`expr $first - $cursecs`
echo $third-$second-$(convertsecs $remsecs)-$forth >> /tmp/stats/dhcp_bind_stats
done < /tmp/tmpresult

rm -f /tmp/tmpresult

