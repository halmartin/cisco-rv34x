# !/bin/sh

## Install wifi driver
#insmod ./wl.ko
#sleep 2

## Config wireless interface
# 2.4G
wl -i wl0 down
wl -i wl0 country US/0
wl -i wl0 band b
wl -i wl0 ap 1
wl -i wl0 wme 1
wl -i wl0 bw_cap 2g 3
wl -i wl0 chanspec 9u
wl -i wl0 frameburst 1
wl -i wl0 vht_features 3
wl -i wl0 obss_coex 0
wl -i wl0 ack_ratio 250
wl -i wl0 2g_rate auto
# for 256QAM
# wl -i wl0 2g_rate -v 9x3 -g -b 40
wl -i wl0 ampdu_mpdu 64
wl -i wl0 up
wl -i wl0 ssid rv260w_2g
ifconfig wl0 up

# 5G
wl -i wl1 down
wl -i wl1 country US/0
wl -i wl1 band a
wl -i wl1 ap 1
wl -i wl1 bss_maxassoc 64
wl -i wl1 mbss 1
wl -i wl1 mpc 0
wl -i wl1 spect 0
wl -i wl1 nar 0
wl -i wl1 clk 1
wl -i wl1 wme 1
wl -i wl1 amsdu 1
wl -i wl1 bw_cap 5g 7
wl -i wl1 chanspec 36/80
# for 1024 QAM
wl -i wl1 vht_features 6
wl -i wl1 5g_rate auto
#wl -i wl1 5g_rate -v 9x3 -g -b 80
wl -i wl1 frameburst 1
wl -i wl1 rx_amsdu_in_ampdu 1
wl -i wl1 amsdu_aggsf 2
wl -i wl1 ack_ratio 250
wl -i wl1 ampdu_mpdu 64
wl -i wl1 up
wl -i wl1 ssid rv260w_5g
ifconfig wl1 up

sleep 2

## Setup bridge
ifconfig eth0 0 up
ifconfig eth2 0 up
brctl addbr vlan1
brctl addif vlan1 eth0
brctl addif vlan1 eth2
brctl addif vlan1 wl0
brctl addif vlan1 wl1
ifconfig vlan1 192.168.1.1/24 up

# Enable fast path forwarding
sh ./opt_rv260w.sh

