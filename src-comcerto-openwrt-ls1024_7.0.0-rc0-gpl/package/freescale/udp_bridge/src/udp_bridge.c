/*
 *  Copyright (c) 2011, 2014 Freescale Semiconductor, Inc.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 */

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/byteorder/generic.h>            /* ntohs(X) */
#include <linux/if_ether.h>                     /* ETH_P_IP, IP packet */

#define DEBUG_ENA                   (0)
#define ETH_P_UDP_ENCAPS            (0x0800)    /* UDP packet */

static char *control_interface =    "eth1"; 
static char *phy_interface =        "eth0"; 

module_param(phy_interface, charp, 0);
MODULE_PARM_DESC(phy_interface, 
    "Select the physical interface that receives UDP packet (LAN, or WAN)");

static char *host_macaddress_string = "00:AA:BB:CC:DD:EE";

module_param(host_macaddress_string, charp, 0);
MODULE_PARM_DESC(host_macaddress_string, "sets the host mac address");

static char *support_ip = "no";
module_param(support_ip, charp, 0);
MODULE_PARM_DESC(support_ip, "allows additiolally to handle IP packets");

static char ctrl_macaddress[ETH_ALEN];
static char phy_macaddress[ETH_ALEN];
static char host_macaddress[ETH_ALEN];

#define ETHER_ADDR_LEN  (6)

typedef struct _ETH_PKT_ {
    unsigned char       dst[ETHER_ADDR_LEN];    /* Destination node */
    unsigned char       src[ETHER_ADDR_LEN];    /* Source node */
    unsigned short      type;                   /* Protocol or length */
    unsigned char       payload[1];    
}ETH_PKT, *PETH_PKT;

#define IP_ADDR_LEN     (4)                     /* Len of logical IP address */
static char msp_ip[IP_ADDR_LEN]= {192, 168, 31, 222};

typedef struct _IP_PKT_ {
    unsigned char       ip_hl_v;                /* header length and version */
    unsigned char       ip_tos;                 /* type of service */
    unsigned short      ip_len;                 /* total length */
    unsigned short      ip_id;                  /* identification */
    unsigned short      ip_off;                 /* fragment offset field */
    unsigned char       ip_ttl;                 /* time to live */
    unsigned char       ip_p;                   /* protocol */
    unsigned short      ip_sum;                 /* checksum */
    unsigned char       ip_src[IP_ADDR_LEN];    /* Source IP address */
    unsigned char       ip_dst[IP_ADDR_LEN];    /* Destination IP address */
    unsigned char       payload[1];    
} IP_PKT, *PIP_PKT;

/* Internet Protocol v4 (IPv4) header */
typedef struct {
    unsigned char       ip_hl_v;                /* header length and version */
    unsigned char       ip_tos;                 /* type of service */
    unsigned short      ip_len;                 /* total length */
    unsigned short      ip_id;                  /* identification */
    unsigned short      ip_off;                 /* fragment offset field */
    unsigned char       ip_ttl;                 /* time to live */
    unsigned char       ip_p;                   /* protocol */
    unsigned short      ip_sum;                 /* checksum */
    unsigned char       ip_src[IP_ADDR_LEN];    /* Source IP address */
    unsigned char       ip_dst[IP_ADDR_LEN];    /* Destination IP address */
} IP_t;

#define IP_HDR_SIZE     (sizeof (IP_t))

unsigned int sys_checksum(unsigned short * ptr, int len)
{
    unsigned long   xsum;
    unsigned short *p = (unsigned short *)ptr;

    xsum = 0;
    while (len-- > 0) {
        xsum += *p++;
    }
    
    xsum = (xsum & 0xffff) + (xsum >> 16);
    xsum = (xsum & 0xffff) + (xsum >> 16);
    
    return (xsum & 0xffff);
}

/**
 *  udp_bridge -
 *
 *
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14)
int udp_bridge( struct sk_buff *skb, 
                struct net_device *dev, 
                struct packet_type *pt, 
                struct net_device *orig_dev )
#else
int udp_bridge( struct sk_buff *skb, 
                struct net_device *dev, 
                struct packet_type *pt )
#endif
{
    struct ethhdr *eth_hdr;
    unsigned short protocol;
        
    if ((skb = skb_share_check(skb, GFP_ATOMIC)) == NULL) {
#if DEBUG_ENA        
        printk("UDP: skb_share_check == NULL\n");
#endif
        goto out;
    }
    
    if( (strcmp(dev->name,phy_interface) != 0) 
        && (strcmp(dev->name,control_interface) != 0) ) {    
#if DEBUG_ENA        
        printk("UDP: Unknown interface\n");
#endif
        goto drop;
    }
    
    // forward
    eth_hdr = (struct ethhdr *) skb_push(skb, ETH_HLEN);//eth_hdr(skb);
    protocol = ntohs (eth_hdr->h_proto);
    
#if DEBUG_ENA
    printk("UDP source mac: %x:%x:%x:%x:%x:%x\n",
        eth_hdr->h_source[0], eth_hdr->h_source[1], eth_hdr->h_source[2],
        eth_hdr->h_source[3], eth_hdr->h_source[4], eth_hdr->h_source[5]);
    printk("UDP destination mac: %x:%x:%x:%x:%x:%x\n",
        eth_hdr->h_dest[0], eth_hdr->h_dest[1], eth_hdr->h_dest[2],
        eth_hdr->h_dest[3], eth_hdr->h_dest[4], eth_hdr->h_dest[5]);
#endif

    if(strcmp(dev->name,phy_interface) == 0) {                
        if (ETH_P_UDP_ENCAPS == protocol) {
            uint i, flag;

            flag = 1;
            for (i=0; i<ETHER_ADDR_LEN; i++) {
                if (eth_hdr->h_source[i] != host_macaddress[i]) {
                    flag = 0;
                    break;
                }
            }

            if(flag) {
                for (i=0; i<ETHER_ADDR_LEN; i++) {
                    if (eth_hdr->h_dest[i] != phy_macaddress[i]) {
                        flag = 0;
                        break;
                    }
                }
                
                if (flag) {
                    //change IP addr here
                    PETH_PKT pEthPkt = (PETH_PKT)eth_hdr;
                    PIP_PKT pIpPkt = (PIP_PKT)&pEthPkt->payload[0];            

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
                     struct net_device *dest_dev = \
                        dev_get_by_name(control_interface);
#else
                    struct net_device *dest_dev = \
                        dev_get_by_name(&init_net, control_interface);
#endif
                    if (flag) {
                        for (i=0; i<IP_ADDR_LEN; i++) {
                            if (pIpPkt->ip_dst[i] != msp_ip[i]) {
                                flag = 0;
                                
                                break;
                            }
                        }

                        if (flag) {
#if DEBUG_ENA
                            static uint host2msp_cntr = 0;
#endif
                            memcpy(eth_hdr->h_dest, ctrl_macaddress, ETH_ALEN);

                            pIpPkt->ip_sum = 0;
                            pIpPkt->ip_sum = (unsigned short)(~(sys_checksum (
                                  (unsigned short *)pIpPkt, IP_HDR_SIZE / 2)));
#if DEBUG_ENA
                            host2msp_cntr++;

                            printk("UDP Host --> MSP (%x)[c=%d] [sum_%04x]: ",
                                protocol, host2msp_cntr, pIpPkt->ip_sum);
                            printk(" [src: %02x:%02x:%02x:%02x:%02x:%02x] ",
                                eth_hdr->h_source[0], eth_hdr->h_source[1],
                                eth_hdr->h_source[2], eth_hdr->h_source[3],
                                eth_hdr->h_source[4], eth_hdr->h_source[5]);
                            printk(" [dst: %02x:%02x:%02x:%02x:%02x:%02x]",
                                eth_hdr->h_dest[0], eth_hdr->h_dest[1],
                                eth_hdr->h_dest[2], eth_hdr->h_dest[3],
                                eth_hdr->h_dest[4], eth_hdr->h_dest[5]);
                            printk(" [ip src: %03d.%03d.%03d.%03d]",
                                pIpPkt->ip_src[0], pIpPkt->ip_src[1], 
                                pIpPkt->ip_src[2], pIpPkt->ip_src[3]);
                            printk(" [ip dst: %03d.%03d.%03d.%03d]\n",
                                pIpPkt->ip_dst[0], pIpPkt->ip_dst[1], 
                                pIpPkt->ip_dst[2], pIpPkt->ip_dst[3]);
#endif
                            skb->dev = dest_dev;
                            dev_queue_xmit(skb);

                            return NET_RX_SUCCESS;
                        }
                    }
                }
            }
        }
    } else if(strcmp(dev->name,control_interface) == 0) {
        if (ETH_P_UDP_ENCAPS == protocol) {
            uint i, flag;

            flag = 1;
            for (i=0; i<ETHER_ADDR_LEN; i++) {
                if (eth_hdr->h_dest[i] != host_macaddress[i]) {
                    flag = 0;
                    break;
                }
            }

            if(flag) {
                for (i=0; i<ETHER_ADDR_LEN; i++) {
                    if (eth_hdr->h_source[i] != ctrl_macaddress[i]) {
                        flag = 0;
                        break;
                    }
                }
                
                if (flag) {
                    //change IP addr here
                    PETH_PKT pEthPkt = (PETH_PKT)eth_hdr;
                    PIP_PKT pIpPkt = (PIP_PKT)&pEthPkt->payload[0];         

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
                    struct net_device *dest_dev = \
                        dev_get_by_name(phy_interface);
#else
                    struct net_device *dest_dev = \
                        dev_get_by_name(&init_net, phy_interface);
#endif
                    for (i=0; i<IP_ADDR_LEN; i++) {
                        if (pIpPkt->ip_src[i] != msp_ip[i]) {
                            flag = 0;
                            break;
                        }
                    }

                    if (flag) {
                        for (i=0; i<IP_ADDR_LEN; i++) {
                            if (pIpPkt->ip_dst[i] != msp_ip[i]) {
                                flag = 0;
                                break;
                            }
                        }

                        if (flag) {
#if DEBUG_ENA
                            static uint msp2host_cntr = 0;
#endif
                            memcpy(eth_hdr->h_source, phy_macaddress,ETH_ALEN);

                            pIpPkt->ip_sum = 0;
                            pIpPkt->ip_sum = (unsigned short)(~(sys_checksum (
                                (unsigned short *)pIpPkt, IP_HDR_SIZE / 2)));

#if DEBUG_ENA
                            msp2host_cntr++;

                            printk("UDP MSP --> Host (%x)[c=%d] [sum_%04x]: ",
                                protocol, msp2host_cntr, pIpPkt->ip_sum);
                            printk(" [src: %02x:%02x:%02x:%02x:%02x:%02x] ",
                                eth_hdr->h_source[0], eth_hdr->h_source[1],
                                eth_hdr->h_source[2], eth_hdr->h_source[3],
                                eth_hdr->h_source[4], eth_hdr->h_source[5]);
                            printk(" [dst: %02x:%02x:%02x:%02x:%02x:%02x]",
                                eth_hdr->h_dest[0], eth_hdr->h_dest[1],
                                eth_hdr->h_dest[2], eth_hdr->h_dest[3],
                                eth_hdr->h_dest[4], eth_hdr->h_dest[5]);
                            printk(" [ip src: %03d.%03d.%03d.%03d]",
                                pIpPkt->ip_src[0], pIpPkt->ip_src[1], 
                                pIpPkt->ip_src[2], pIpPkt->ip_src[3]);
                            printk(" [ip dst: %03d.%03d.%03d.%03d]\n",
                                pIpPkt->ip_dst[0], pIpPkt->ip_dst[1], 
                                pIpPkt->ip_dst[2], pIpPkt->ip_dst[3]);
#endif
                            skb->dev = dest_dev;
                            dev_queue_xmit(skb);

                            return NET_RX_SUCCESS;
                        }
                    }
                }
            }
        }
    }

drop:
    kfree_skb(skb);

out:
    return NET_RX_DROP;
}

struct packet_type udp_encaps_pt = {
    .type = __constant_htons(ETH_P_UDP_ENCAPS),
    .func = udp_bridge,
};

struct packet_type eth_pt = {
    .type = __constant_htons(ETH_P_IP),
    .func = udp_bridge,
};


int udp_bridge_init(void)
{
    struct net_device *orig_dev;
    int index =0;
    char *pchar;

    printk(" == UDP bridge init ==\n");
    
    //get ctrl interface mac address
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
    orig_dev = dev_get_by_name(control_interface);
#else
    orig_dev = dev_get_by_name(&init_net, control_interface);
#endif

    if(orig_dev) {
        memcpy(ctrl_macaddress,orig_dev->dev_addr,ETH_ALEN);
    } else  {
        printk("Error getting ctrl_macaddress\n");
        return -1;
    }

    printk(" -- CTRL MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            ctrl_macaddress[0], ctrl_macaddress[1], ctrl_macaddress[2],
            ctrl_macaddress[3], ctrl_macaddress[4], ctrl_macaddress[5]);
    
    //get phys interface mac address
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)
     orig_dev = dev_get_by_name(phy_interface);
#else
    orig_dev = dev_get_by_name(&init_net, phy_interface);
#endif

    if(orig_dev) {
        memcpy(phy_macaddress,orig_dev->dev_addr,ETH_ALEN);
    } else {
        printk("Error getting phy_macaddress %s\n",phy_macaddress);
        return -1;
    }

    printk(" -- PHY MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            phy_macaddress[0], phy_macaddress[1], phy_macaddress[2],
            phy_macaddress[3], phy_macaddress[4], phy_macaddress[5]);

    //get host macaddress
    index = 0;
    pchar = (unsigned char*) host_macaddress_string;
    while (pchar && (index < ETH_ALEN)) {
        if (pchar) {
            unsigned char tmp = simple_strtol(pchar, NULL, 16);
            host_macaddress[index++] = (unsigned char)tmp;
            pchar +=3;
        }
    }

    printk(" -- HOST MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
            host_macaddress[0], host_macaddress[1], host_macaddress[2],
            host_macaddress[3], host_macaddress[4], host_macaddress[5]);
    
    dev_add_pack(&udp_encaps_pt);
    
    if (strcmp(support_ip,"yes")  == 0) {
        dev_add_pack(&eth_pt);
        printk("IP protocol bridging enabled (MSP <--> CSP)\n");
    }
    
    return 0;
}
void udp_bridge_cleanup(void)
{
    printk("udp_intercept_cleanup\n");
    dev_remove_pack(&udp_encaps_pt);
    
    if (strcmp(support_ip,"yes") == 0) {
        dev_remove_pack(&eth_pt);
    }

    return;
}

int udp_bridge_init_module(void)
{
    return udp_bridge_init();
}

static void udp_bridge_cleanup_module(void)
{
    udp_bridge_cleanup();

    return;
}

module_init(udp_bridge_init_module);
module_exit(udp_bridge_cleanup_module);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Alex Winter <alex.winter@nxp.com>");

