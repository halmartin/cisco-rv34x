/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

struct list_head allIfaces;
struct list_head portTriggerRules;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;

void print_portTrigger(struct uci_portTriggerRule *pt)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");

	if(pt)
		printf("Complete firewall port trigger record is '%s'\n"
			"	enabled:	%s\n"
			"	interface:	%s\n"
			"	protocol:	%s\n"
			"	Trigger Port-start:%d\n"
			"	Trigger Port-end:%d\n"
			"	internal Port-start:%d\n"
			"	internal Port-end:%d\n"
			"	Internal protocol:%s\n",
			pt->name,
			(pt->status ? "Yes" : "No"),
			pt->interface,
			pt->protocol,
			pt->trigger_port_start,
			pt->trigger_port_end,
			pt->int_port_start,
			pt->int_port_end,
			pt->int_protocol);
}

static int insert_port_Triggering(struct uci_portTriggerRule *pt, struct uci_context *ctx, struct iptc_handle *handle_filter, struct iptc_handle *handle_nat)
{
	struct iface_data *findIface;
//	struct list_head *p;
	char natRule[2048]="";
	char filterRule[2048]="";


	findIface = search_iface(pt->interface);
	if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
	{
		printf("port trigger for the interface:%s is not configured since the interface is not active/UP\r\n",pt->interface);
		return 1;
	}

	if(strcmp(pt->int_protocol,"all") == 0)
	{
		sprintf(natRule,"iptables-uci -t nat -A port_trigger_nat -i %s -d %s -p tcp --dport %d:%d "
					"-j TRIGGER --trigger-type dnat",findIface->l3ifname,findIface->ipaddr,pt->int_port_start, pt->int_port_end);
		tokenize_and_docommand(natRule,"nat",handle_nat);
		memset(natRule, '\0', sizeof(natRule));
		sprintf(natRule,"iptables-uci -t nat -A port_trigger_nat -i %s -d %s -p udp --dport %d:%d "
					"-j TRIGGER --trigger-type dnat",findIface->l3ifname,findIface->ipaddr,pt->int_port_start, pt->int_port_end);
		tokenize_and_docommand(natRule,"nat",handle_nat);

		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i %s -p tcp --dport %d:%d "
					"-j TRIGGER --trigger-type in",findIface->l3ifname,pt->int_port_start, pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
		memset(natRule, '\0', sizeof(natRule));
		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i %s -p udp --dport %d:%d "
					"-j TRIGGER --trigger-type in",findIface->l3ifname,pt->int_port_start, pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
	}
	else
	{
		sprintf(natRule,"iptables-uci -t nat -A port_trigger_nat -i %s -d %s -p %s --dport %d:%d "
			"-j TRIGGER --trigger-type dnat",findIface->l3ifname,findIface->ipaddr,pt->int_protocol,pt->int_port_start, pt->int_port_end);
		tokenize_and_docommand(natRule,"nat",handle_nat);

		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i %s -p %s --dport "
					"%d:%d -j TRIGGER --trigger-type in",findIface->l3ifname,pt->int_protocol,pt->int_port_start, pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
	}

	if(strcmp(pt->protocol,"all") == 0)
	{
		memset(filterRule, '\0', sizeof(filterRule));
        	sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i %s.+ -o %s -p tcp --dport %d:%d -j TRIGGER "
					"--trigger-type out --trigger-proto all --trigger-match %d-%d --trigger-relate %d-%d", gLan_phy_iface, 
					findIface->l3ifname,pt->trigger_port_start,pt->trigger_port_end,pt->int_port_start,pt->int_port_end,
					pt->int_port_start,pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
		memset(filterRule, '\0', sizeof(filterRule));
		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i %s.+ -o %s -p udp --dport %d:%d -j TRIGGER "
					"--trigger-type out --trigger-proto all --trigger-match %d-%d --trigger-relate %d-%d", gLan_phy_iface,
					findIface->l3ifname,pt->trigger_port_start,pt->trigger_port_end,pt->int_port_start,pt->int_port_end,
					pt->int_port_start,pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);

		//For br-vlan
		memset(filterRule, '\0', sizeof(filterRule));
		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i br-vlan+ -o %s -p tcp --dport %d:%d -j TRIGGER "
					"--trigger-type out --trigger-proto all --trigger-match %d-%d --trigger-relate %d-%d",
					findIface->l3ifname,pt->trigger_port_start,pt->trigger_port_end,pt->int_port_start,pt->int_port_end,
					pt->int_port_start,pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
		memset(filterRule, '\0', sizeof(filterRule));
		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i br-vlan+ -o %s -p udp --dport %d:%d -j TRIGGER "
					"--trigger-type out --trigger-proto all --trigger-match %d-%d --trigger-relate %d-%d",
					findIface->l3ifname,pt->trigger_port_start,pt->trigger_port_end,pt->int_port_start,pt->int_port_end,
					pt->int_port_start,pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
	}
	else
	{
		memset(filterRule, '\0', sizeof(filterRule));
		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i %s.+ -o %s -p %s --dport %d:%d -j TRIGGER "
					"--trigger-type out --trigger-proto all --trigger-match %d-%d --trigger-relate %d-%d", gLan_phy_iface,
					findIface->l3ifname,pt->protocol,pt->trigger_port_start,pt->trigger_port_end,pt->int_port_start,
					pt->int_port_end,pt->int_port_start,pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);

		//For br-vlan
		memset(filterRule, '\0', sizeof(filterRule));
		sprintf(filterRule,"iptables-uci -t filter -A port_trigger_fwd -i br-vlan+ -o %s -p %s --dport %d:%d -j TRIGGER "
					"--trigger-type out --trigger-proto all --trigger-match %d-%d --trigger-relate %d-%d",
					findIface->l3ifname,pt->protocol,pt->trigger_port_start,pt->trigger_port_end,pt->int_port_start,
					pt->int_port_end,pt->int_port_start,pt->int_port_end);
		tokenize_and_docommand(filterRule,"filter", handle_filter);
	}
	return 0;
}

void config_portTriggering(struct uci_context *ctx)
{
	struct list_head *p;
	struct uci_portTriggerRule *pt;
	int y=0;
	struct iptc_handle *handle_filter = NULL;
	struct iptc_handle *handle_nat = NULL;

	handle_filter = create_handle("filter");
	handle_nat = create_handle("nat");
	
	if(iptc_is_chain(strdup("port_trigger_nat"), handle_nat)
		&& iptc_is_chain(strdup("port_trigger_fwd"), handle_filter))
	{
		if(!iptc_flush_entries(strdup("port_trigger_nat"),handle_nat))
			printf("Failed in flushing rules of port_trigger_nat in iptables\r\n");

		if(!iptc_flush_entries(strdup("port_trigger_fwd"), handle_filter))
			printf("Failed in flushing rules of port_trigger_fwd in iptables\r\n");
	}
	else
		printf("No chain detected with the name portforward/portforward_dnat/nat_reflection_in/nat_reflection_out for IPv4\r\n");

	list_for_each(p, &portTriggerRules) {

		pt = list_entry(p, struct uci_portTriggerRule, list);

		if (pt->status)
			insert_port_Triggering(pt, ctx, handle_filter, handle_nat);
		else
			printf("port Trigger rule with the name:%s is disabled\n",pt->name);
	}

	y = iptc_commit(handle_filter);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle_filter);
			if (!y)
				printf("Error commiting data:%s errno:%d in the context of Port Trigger rules(filter table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of Port Trigger rules(filter table).\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	y = iptc_commit(handle_nat);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle_nat);
			if (!y)
				printf("Error commiting data:%s errno:%d in the context of Port Trigger rules(nat table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of Port Trigger rules(nat table).\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	iptc_free(handle_filter);
	iptc_free(handle_nat);
}
