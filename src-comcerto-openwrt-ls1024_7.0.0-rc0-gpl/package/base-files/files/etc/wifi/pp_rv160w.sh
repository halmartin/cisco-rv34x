# !/bin/sh
# Script to bring up DNI's Broadcom 2.4/5G Wi-Fi Module on RV160W

## Install wifi driver
#insmod ./wl.ko

## 5G
wl -i wl0 down
wl -i wl0 country US/0
wl -i wl0 band a
wl -i wl0 ap 1
wl -i wl0 mbss 1
wl -i wl0 mpc 0
wl -i wl0 spect 0
wl -i wl0 nar 0
wl -i wl0 clk 1
wl -i wl0 wme 1
wl -i wl0 bss_maxassoc 64
wl -i wl0 bw_cap 5g 7
wl -i wl0 chanspec 36/80
wl -i wl0 ack_ratio 250
wl -i wl0 5g_rate auto
#wl -i wl0 5g_rate -v 9x2 
wl -i wl0 amsdu 1
wl -i wl0 frameburst 1
wl -i wl0 amsdu_aggsf 4
wl -i wl0 ampdu_mpdu 48
wl -i wl0 rx_amsdu_in_ampdu 1
wl -i wl0 up
wl -i wl0 ssid rv160w_5g
ifconfig wl0 up

# 2.4G
wl -i wl1 down
wl -i wl1 country US/0
wl -i wl1 band b 
wl -i wl1 ap 1
wl -i wl1 infra 1
wl -i wl1 bw_cap 2g 3
wl -i wl1 chanspec 9u
wl -i wl1 frameburst 1
wl -i wl1 obss_coex 0
wl -i wl1 ack_ratio 250
wl -i wl1 2g_rate auto
#wl -i wl1 2g_rate -h 15
wl -i wl1 ampdu_mpdu 48
wl -i wl1 up
wl -i wl1 ssid rv160w_2g
ifconfig wl1 up

sleep 1
ifconfig eth0 0 up
ifconfig eth2 0 up
brctl addbr vlan1
brctl addif vlan1 eth0
brctl addif vlan1 eth2
brctl addif vlan1 wl0
brctl addif vlan1 wl1
ifconfig vlan1 192.168.1.1 up
sleep 2
sh ./opt_rv160w.sh
