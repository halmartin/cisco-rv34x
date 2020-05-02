#!/bin/sh

runcmd()
{
  local cmd="$1"
  #echo "cmd=$cmd"
{ confd_cli -u cisco <<EOF
configure
$cmd
exit
EOF
}

}

TRPTGT=`runcmd "run show all SNMP-TARGET-MIB snmpTargetAddrTable"`

keyparam=$(echo "$TRPTGT" | grep "snmpTargetAddrParams" | awk '{ print $2 }' | tr -d ";" )
trapsrvaddr=$(echo "$TRPTGT" | grep "snmpTargetAddrTAddress" | awk '{ print $2 }' | tr -d ";" | cut -d'.' -f'1-4')

#echo "trapsrvaddr=$trapsrvaddr"
#echo "keyparam=$keyparam"

trpcom=`confd_cmd -c "mget /SNMP-COMMUNITY-MIB/snmpCommunityTable/snmpCommunityEntry{3}/snmpCommunityName"`
#echo "trpcom=$trpcom"
echo "1,$trapsrvaddr,$trpcom"

