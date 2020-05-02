/**************************************************************************
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
 ***************************************************************************/
/*
 * File:        nbvpn_core.c
 *
 * Description: NBVPN packet processing
 *
 * Authors:     Sridhar Pothuganti <sridhar.pothuganti@nxp.com>
 *
 */
/* History
 *  Version     Date            Author                  Change Description
 *    1.0       19/07/2015      Sridhar Pothuganti      Initial Development
 *    1.1       22/07/2015      Chaitanya Sakinam       Full functionality implimentation
 *    1.2       26/12/2017      Sridhar Pothuganti      Added functionality to forward
 *							Decrypted NetBIOS packets
*/
/****************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/udp.h>
#include <net/route.h>
#include <net/xfrm.h>
#include <net/netfilter/nf_queue.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/inetdevice.h>

#include "nbvpn_core.h"

extern spinlock_t nbvpn_list_lock;
extern struct list_head nbvpn_list_head;
static struct nf_hook_ops netfilter_ops_pre;

int forward_bcst_to_interface(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *new_skb;
	struct rtable *rt;
	int err=0;
	struct flowi4 fl4;
	struct iphdr *iph;
	struct udphdr *udph;

	new_skb = pskb_copy(skb, GFP_ATOMIC);
	if(new_skb == NULL)
		return -ENOMEM;

	new_skb->pkt_type=PACKET_HOST;
	iph = ip_hdr(new_skb);
	/*Setting UDP checksum to Zero, otherwise Windows dropping these packets*/
	udph = (struct udphdr *)skb_transport_header(new_skb);
	udph->check = 0;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0)
	if(new_skb->destructor)
		skb_scrub_packet(new_skb, 1);
	else
		skb_scrub_packet(new_skb, 0);

	skb_clear_hash(new_skb);

	memset(&fl4, 0, sizeof(fl4));
	fl4.daddr = iph->daddr;
	fl4.flowi4_mark = 0;
	fl4.flowi4_tos = RT_TOS(iph->tos);
	fl4.flowi4_proto = IPPROTO_UDP;

	rt = ip_route_output_key(&init_net, &fl4);
#else
	rt = ip_route_output(&init_net, iph->daddr, 0, 0, 0);
#endif
	if (IS_ERR(rt)) {
		printk("ip_route_output_key returned err\r\n");
		kfree_skb(new_skb);
		return rt;
	}

	new_skb->sk = NULL;
	skb_dst_set(new_skb, &rt->dst);
	memset(IPCB(new_skb), 0, sizeof(*IPCB(new_skb)));

	err = ip_local_out(new_skb);

	return err;
}

int sendBcsttoRemote(struct sk_buff *skb, unsigned int bcstaddr, struct nf_queue_entry *q_entry)
{
	struct sk_buff *new_skb;
	struct iphdr *iph;

	new_skb = pskb_copy(skb, GFP_ATOMIC);
	new_skb->pkt_type=PACKET_HOST;

	q_entry->skb = new_skb;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
	if (q_entry->indev)
		dev_hold(q_entry->indev);
	if (q_entry->outdev)
		dev_hold(q_entry->outdev);
#else
	nf_queue_entry_get_refs(q_entry);
#endif
	iph = ip_hdr(new_skb);
	iph->daddr = bcstaddr;
	iph->check = 0;
	iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);

	nf_reinject(q_entry,NF_ACCEPT);
	return T_SUCCESS;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
unsigned int Hook_func(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn) (struct sk_buff *))
#else
unsigned int Hook_func (const struct nf_hook_ops *ops,
                               struct sk_buff *skb,
                               const struct nf_hook_state *state)
#endif
{
	struct iphdr *iph;
	struct udphdr *uhead = NULL;
	unsigned short srcPort = 0, dstPort = 0;
	nbvpn_t *pTemp;
	struct nf_queue_entry *q_entry;

#if LINUX_VERSION_CODE > KERNEL_VERSION(4,1,0)
	unsigned int    hooknum;
	hooknum=ops->hooknum;
#endif

	iph = ip_hdr(skb);
	
	/* Be sure this is a UDP packet first */
	if (iph->protocol != IPPROTO_UDP)
		return NF_ACCEPT;

	uhead = (struct udphdr *)(skb->data + (iph->ihl * 4));
	srcPort = ntohs(uhead->source);
	dstPort = ntohs(uhead->dest);

	/* Now check the destination port */
	if ((dstPort == NB_NAME_SERVICE_PORT) ||
	    (dstPort == NB_DATAGRAM_SESSION_SERVICE) ||
	    (srcPort == NB_NAME_SERVICE_PORT) ||
	    (srcPort == NB_DATAGRAM_SESSION_SERVICE)) {

		//If skb->sp poiter is set, it is VPN decrypted packet
		if(skb->sp)
		{
			struct net_device *dev=first_net_device(&init_net);
			struct in_device *in_dev;
			struct in_ifaddr **ifap = NULL;
			struct in_ifaddr *ifa = NULL;
			int status;

			while(dev) {
				in_dev = in_dev_get(dev);
				for (ifap = &in_dev->ifa_list; (ifa = *ifap) != NULL;
					ifap = &ifa->ifa_next) {
//			printk("ifa_label:%s ifa_local:%u(%pI4) ifa_broadcast:%u(%pI4) ifa_address:%u(%pI4) ifa_mask:%u(%pI4) \r\n",ifa->ifa_label, ifa->ifa_local, &(ifa->ifa_local), ifa->ifa_broadcast, &(ifa->ifa_broadcast), ifa->ifa_address, &(ifa->ifa_address), ifa->ifa_mask, &(ifa->ifa_mask));
					if(iph->daddr == ifa->ifa_broadcast) {
						nbvpn_debug("Packet has to be sent on interface %s and existing dev is :%s\r\n",dev->name, skb->dev->name);
						status=forward_bcst_to_interface(skb, dev);
						nbvpn_debug("Status:%d \r\n",status);
					}
				}

				in_dev_put(in_dev);
				dev=next_net_device(dev);
			}
		}
		list_for_each_entry(pTemp, &nbvpn_list_head, list) {
			if ((pTemp->left_subnet & pTemp->left_subnet_mask) ==
			    (iph->saddr & pTemp->left_subnet_mask)) {
			nbvpn_debug("hook:%u src:%u(%pI4) dst:%u(%pI4) proto:%d\r\n",hooknum,iph->saddr, &(iph->saddr), iph->daddr, &(iph->daddr), iph->protocol);

			q_entry=kmalloc(sizeof(struct nf_queue_entry),GFP_ATOMIC);
	
			if(q_entry){
			q_entry->elem	= &netfilter_ops_pre;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)
			q_entry->pf     = NFPROTO_IPV4;
			q_entry->hook   = hooknum;
			q_entry->indev  = in;
			q_entry->outdev = out;
			q_entry->okfn   = okfn;
#else
			q_entry->state.pf     = NFPROTO_IPV4;
			q_entry->state.hook   = hooknum;
			q_entry->state.in  = state->in;
			q_entry->state.out = state->out;
			q_entry->state.okfn   = state->okfn;
			q_entry->state.sk  = state->sk;
#endif
			}
			else
			{
				printk("\n %s:%d unable to allocate nf_queue_entry \r\n",__FUNCTION__,__LINE__);
				return NF_DROP;
			}
	
				if (sendBcsttoRemote(skb, pTemp->right_bcst, q_entry))
					nbvpn_debug
					    ("Unable to send packet to %pI4\r\n",
					     &(pTemp->right_bcst));
			}
		}
	}
	return NF_ACCEPT;
}

int nbvpn_init_module(void)
{
	netfilter_ops_pre.hook = Hook_func;
	netfilter_ops_pre.hooknum = NF_INET_PRE_ROUTING;
	netfilter_ops_pre.pf = PF_INET;
	netfilter_ops_pre.priority = NF_IP_PRI_FIRST;

	nf_register_hook(&netfilter_ops_pre);

	spin_lock_init(&nbvpn_list_lock);
	printk("nbvpn module loaded successfully...\r\n");

	return 0;
}

void nbvpn_exit_module(void)
{
	nf_unregister_hook(&netfilter_ops_pre);
	printk("nbvpn module un-loaded successfully...\r\n");

}

module_init(nbvpn_init_module);
module_exit(nbvpn_exit_module);

MODULE_DESCRIPTION("Module to send NetBIOS packets to remote VPN subnet");
MODULE_AUTHOR("sridhar.pothuganti@freescale.com");
MODULE_LICENSE("GPL");
