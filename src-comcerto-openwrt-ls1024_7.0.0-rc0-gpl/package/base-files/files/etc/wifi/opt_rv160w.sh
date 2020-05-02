# !/bin/sh

##### RV160W optimize script #####

### Assign PEE interrupt and HIF_NAPI to Core 1
### Here we should use the same core with 2.4G card as it has lower throughput and CPU consumption
echo 1 > /proc/irq/36/smp_affinity

### Assign wl0 (5G) to Core 1
echo 2 > /proc/irq/75/smp_affinity

### Assign wl1 (2.4G) to Core 0, as 5G card has more CPU consumtion
echo 1 > /proc/irq/76/smp_affinity

### Assign PFE VWD NAPI to Core 1  eth0(eth2) -> wl0
echo 1 > /sys/devices/platform/pfe.0/wl0/rx_cpu_affinity

### Assign PFE VWD NAPI to Core 0,  eth0(eth2) -> wl1
echo 0 > /sys/devices/platform/pfe.0/wl1/rx_cpu_affinity

##### Enable NCNB features in VWD driver for 5G card
echo 1 > /sys/devices/platform/pfe.0/wl0/custom_skb_enable

# Enable packet chaining in VWD driver
echo 1 > /sys/devices/platform/pfe.0/wl0/brcm_pktc_enable

# Enable Fast path in UL direction (wl0.rx -> vwd.tx)
echo 1 > /sys/devices/platform/pfe.0/wl0/direct_rx_path

# Enable Fast path in DL direction (vwd.rx -> wl0.tx)
echo 1 > /sys/devices/platform/pfe.0/wl0/direct_tx_path

##### Enable NCNB features in VWD driver for 2.4G card
echo 1 > /sys/devices/platform/pfe.0/wl1/custom_skb_enable

# Enable packet chaining in VWD driver
echo 1 > /sys/devices/platform/pfe.0/wl1/brcm_pktc_enable

# Enable Fast path in UL direction (wl1.rx -> vwd.tx)
echo 1 > /sys/devices/platform/pfe.0/wl1/direct_rx_path

# Enable Fast path in DL direction (vwd.rx -> wl1.tx)
echo 1 > /sys/devices/platform/pfe.0/wl1/direct_tx_path

### Enable fast bridge, disable route
echo 1 > /sys/devices/platform/pfe.0/vwd_bridge_hook_enable
echo 0 > /sys/devices/platform/pfe.0/vwd_route_hook_enable
