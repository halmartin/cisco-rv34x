#!/bin/sh

LOG_DIR="/tmp/log"
LOG="$LOG_DIR/messages"
STATS_DIR="/tmp/stats/"
TECH_REPORT_TMP_DIR="/tmp/techreport"
CONFD_LOGS_DIR="/tmp/confd"
DNS_LOOKUP="/tmp/utility-dnslookup"
PING="/tmp/utility-ping"
TRACE_ROUTE="/tmp/utility-traceroute"
CONFIG_DIRECTORY="/tmp/configuration"
IFACE_STATS=interface-stats
IPTABLES_CMM_ENTRIES=iptables-cmm-entries
VAR_STATE_DIR=/var/state
CONFIG_DIR=/tmp/etc/config
WIRELESS_COUNTERS=wireless-counters
PORT_STATISTICS=port_statics
CPU_MEM_STATISTICS=cpu_mem_stats
DPI_MODULE=dpi


pid=`uci get systeminfo.sysinfo.pid`
current_time=`date +%Y%m%d`
dev_pid=`echo $pid | cut -f 1 -d -`

tech_report_file1="TechReport_$pid"
tech_report_file2="_$current_time"
#TECH_REPORT_FILE="$tech_report_file1$tech_report_file2"
TECH_REPORT_FILE=$1

if [ ! -d "$LOG_DIR" ];then
        mkdir $LOG_DIR
fi

if [ ! -d "$TECH_REPORT_TMP_DIR" ];then
        mkdir $TECH_REPORT_TMP_DIR
	touch $TECH_REPORT_TMP_DIR/$PORT_STATISTICS
fi

if [ -f "$PING" ]; then
        cp  -rf $PING $TECH_REPORT_TMP_DIR
fi

if [ -f "$TRACE_ROUTE" ]; then
        cp  -rf $TRACE_ROUTE $TECH_REPORT_TMP_DIR
fi

if [ -f "$DNS_LOOKUP" ]; then
        cp  -rf $DNS_LOOKUP $TECH_REPORT_TMP_DIR
fi

cp -rf $STATS_DIR $TECH_REPORT_TMP_DIR
cp -rf $CONFD_LOGS_DIR/confd.log $TECH_REPORT_TMP_DIR
cp -rf $CONFD_LOGS_DIR/devel.log $TECH_REPORT_TMP_DIR
rm -rf $TECH_REPORT_TMP_DIR/confd/cdb

for f in /tmp/*.core
do
   cp -v "$f" $TECH_REPORT_TMP_DIR
done

#Export running configuration
config_mgmt.sh export config-running "$CONFIG_DIRECTORY"/running-config.xml
cp "$CONFIG_DIRECTORY"/running-config.xml $TECH_REPORT_TMP_DIR

#Export startup configuration
config_mgmt.sh export config-startup "$CONFIG_DIRECTORY"/startup-config.xml
cp "$CONFIG_DIRECTORY"/startup-config.xml $TECH_REPORT_TMP_DIR

#IPTABLES
touch $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n##################output of iptables -L -nv ##############\n" > $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
iptables -L -nv >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

echo -e "\n##################output of iptables -L -nv -t nat ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
iptables -L -nv -t nat >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

echo -e "\n##################output of iptables -L -nv -t mangle ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
iptables -L -nv -t mangle >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

#CMM ENTRIES
echo -e "\n################## nf_conntract entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cat /proc/net/nf_conntrack >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query sa" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query sa >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "##### xfrm state" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
ip xfrm state >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "##### xfrm polocy" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
ip xfrm policy >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query connections" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query connections >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query tunnels" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query tunnels >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query pppoe" ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query pppoe >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query qm " ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query qm >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c query swqos " ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c query swqos >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
echo -e "\n################## output of "cmm -c show stat ipsec query " ##############\n" >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES
cmm -c show stat ipsec query >> $TECH_REPORT_TMP_DIR/$IPTABLES_CMM_ENTRIES

# PoE related infomration
if [ "$dev_pid" == "RV345P" ]
then
cp /proc/poe/showallreg -f $TECH_REPORT_TMP_DIR/PoEPortReg
cp /proc/poe/showallstats -f $TECH_REPORT_TMP_DIR/PoEStatistics
fi

#INTERFACE STATS
echo -e "\n################## inface statistics ##############\n" > $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## ifconfig entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
/sbin/ifconfig >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /proc/net/dev entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /proc/net/dev >>$TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip -s link entries ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip -s link >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /etc/hosts ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /etc/hosts >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /etc/resolv.conf ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /etc/resolv.conf >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /tmp/dhcp.leases ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /tmp/dhcp.leases >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of route -n ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
route -n >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip route show ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip route show  >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip route show table 200 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip route show table 220 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of mwan3 status ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
mwan3 status >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of arp -n ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
arp -n >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip neigh show ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip neigh show >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of /etc/nsswitch.conf ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
cat /etc/nsswitch.conf >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ip addr show ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ip addr show >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ps -w ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ps -w >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ubus list ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ubus list >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus wan1 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus wan1 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus wan2 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus wan2 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus usb1 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus usb1 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ifstatus usb1_4 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ifstatus usb1_4 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ethtool -d eth2 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ethtool -d eth2 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ethtool -d eth0 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ethtool -d eth0 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ethtool -d eth3 ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ethtool -d eth3 >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
echo -e "\n################## out put of ipsec statusall ##############\n" >> $TECH_REPORT_TMP_DIR/$IFACE_STATS
ipsec statusall >> $TECH_REPORT_TMP_DIR/$IFACE_STATS

#WIRELESS STATS
if [ "$dev_pid" == "RV340W" ]
then
echo -e "\n################## out put of wl -i wl0 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 assoclist ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 assoclist >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 ssid ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 ssid >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 staus | grep -i "Primary channel" ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 status | grep -i "Primary channel" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 channels ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 channels >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 phy_rssi_ant ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 phy_rssi_ant >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of wl -i wl0 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.1 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.1 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.2 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.2 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl0.3 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0.3 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.1 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.1 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.2 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.2 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## out put of wl -i wl1.3 status ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1.3 status >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
fi
echo -e "\n################## out put of nvram show ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
nvram show >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of uci show wireless ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
uci show wireless >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## out put of uci show lobby ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
uci show lobby >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

echo -e "\n################## output of wl -i wl0 dump all ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl0 dump all 2&>1 >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
echo -e "\n################## output of wl -i wl1 dump all ##############\n" >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS
wl -i wl1 dump all 2&>1 >> $TECH_REPORT_TMP_DIR/$WIRELESS_COUNTERS

#LAN PORT STATS
#/usr/sbin/bcmssdk -Z 1 -d 1 &

cp -rf $VAR_STATE_DIR $TECH_REPORT_TMP_DIR
cp -rf $CONFIG_DIR  $TECH_REPORT_TMP_DIR
cp -rf /tmp/update.sh $TECH_REPORT_TMP_DIR
cp -rf /tmp/etc/ipsec.conf $TECH_REPORT_TMP_DIR
cp -rf /etc/strongswan.d/charon.conf $TECH_REPORT_TMP_DIR
kill -SIGUSR1 `ps | grep vpnTimer | grep -v grep | awk '{print $1}'`

touch $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
echo -e "#######dmesg -c\n" >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
dmesg -c  >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS

echo -e "\ncpu:\n"  >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
mpstat -P ALL >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
top -n 1 >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS

echo -e "\nmemory:\n" >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
free >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS

echo -e "\nupdate time:" >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
cat /proc/uptime >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS

echo -e "current processes" >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS
ps -w >> $TECH_REPORT_TMP_DIR/$CPU_MEM_STATISTICS

if [ -f "$LOG" ]; then
        cp  -f $LOG $TECH_REPORT_TMP_DIR
        cp  -f $LOG_DIR/messagesBK $TECH_REPORT_TMP_DIR
fi

touch $TECH_REPORT_TMP_DIR/$DPI_MODULE
echo -e "lionic dpi module data:\n" >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh show all >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh bundle show >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh iface dump >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh ips psd show >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh decomp show >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh dns show >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh dm dump >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
/usr/bin/lcsh dev dump >> $TECH_REPORT_TMP_DIR/$DPI_MODULE

echo -e "\nlionic dpi module data copying sqldb:\n" >> $TECH_REPORT_TMP_DIR/$DPI_MODULE
cp /tmp/stat.db $TECH_REPORT_TMP_DIR/

#sh /usr/bin/guiReport.sh >/dev/null 2>&1
#mv $LOG_DIR/guiReport* $TECH_REPORT_TMP_DIR

cd $LOG_DIR

tar czf $TECH_REPORT_FILE.tar.gz $TECH_REPORT_TMP_DIR

# Encrypting with password protection
openssl enc -aes-256-cbc -salt -in $TECH_REPORT_FILE.tar.gz -out $TECH_REPORT_FILE.bin -pass pass:C$bTsrd@T^
 
# Decrypting with password
#openssl enc -aes-256-cbc -d -in TechReport_RV160W-I-K9_20180330.bin -out TechReport_RV160W-I-K9_20180330.tar.gz -pass pass:C$bTsrd@T^

rm -f $TECH_REPORT_FILE.tar.gz
rm -rf $TECH_REPORT_TMP_DIR
