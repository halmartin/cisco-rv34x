/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

struct list_head allIfaces;
struct list_head ifaceNATs;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;

void print_ifaceNAT(struct uci_ifaceNAT *ifNAT)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");
	if(ifNAT)
		printf("Complete firewall iface NAT record is '%s'\n"
			"	enabled:	%s\n"
			"	interface-name:	%s\n",
			ifNAT->name,
			(ifNAT->enable ? "Yes" : "No"),
			ifNAT->interfacename);
}

static int insert_iface_nat(struct uci_ifaceNAT *ifNAT, struct uci_context *ctx, struct iptc_handle *handle)
{
	int ret;
	char tuple[32]="";
	struct uci_ptr ptr;
	struct iface_data *findIface;
	char * nat_tablename="nat";
	
	findIface = search_iface(ifNAT->interfacename);
	if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
	{
		//printf("Iface NAT for the interface:%s is not configured since the interface is not active/UP\r\n",ifNAT->interfacename);
		return 1;
	}

	mynewargc = 0;

	if(strstr(findIface->l3ifname,"pptp") || strstr(findIface->l3ifname,"l2tp"))
	{//Is a ppp interface.
		add_argv("iptables-uci");
		add_argv("-t");
		add_argv("nat");
		add_argv("-A");
		add_argv("ifacenat");

		add_argv("-o");
		sprintf(tuple,"network.%s.ifname",ifNAT->interfacename);
		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("SKC: Error in getting info about:%s\r\n",tuple);
			//cli_perror();
			//return 1;
		}
		add_argv(ptr.o->v.string); //The physical interface on which this rule should be applied.
		if (ptr.p)
			uci_unload(ctx, ptr.p);

		add_argv("-d");
		sprintf(tuple,"network.%s.server",findIface->l3ifname + 5); //pptp- or l2tp- is skipped.
		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("SKC: Error in getting info about:%s\r\n",tuple);
			//cli_perror();
			//return 1;
		}
		add_argv(ptr.o->v.string);/*pptp/l2tp server IP*/
		if (ptr.p)
			uci_unload(ctx, ptr.p);

		add_argv("-j");
		add_argv("MASQUERADE");

		if(enable_debug)
			print_ifaceNAT(ifNAT);

		ret = do_command(mynewargc, mynewargv, &nat_tablename, &handle);
		if (!ret) {
			printf("do_command failed.string:%s\n",iptc_strerror(errno));
			print_ifaceNAT(ifNAT);
		}	
		free_argv();
		mynewargc = 0;
	}

	add_argv("iptables-uci");
	add_argv("-t");
	add_argv("nat");
	add_argv("-A");
	add_argv("ifacenat");

	add_argv("-o");
	add_argv(findIface->l3ifname);
	add_argv("-s");
	add_argv("0.0.0.0/0");
	add_argv("-d");
	add_argv("0.0.0.0/0");
	add_argv("-j");
	add_argv("MASQUERADE");
		

	if(enable_debug)
		print_ifaceNAT(ifNAT);

	ret = do_command(mynewargc, mynewargv, &nat_tablename, &handle);
	if (!ret) {
		printf("do_command failed.string:%s\n",iptc_strerror(errno));
		print_ifaceNAT(ifNAT);
	}
	free_argv();
	return 0;
}

void config_ifaceNAT(struct uci_context *ctx)
{
	struct list_head *p;
	struct uci_ifaceNAT *ifNAT;
	int y=0;
	struct iptc_handle *handle = NULL;

	handle = create_handle("nat");

	if(iptc_is_chain(strdup("ifacenat"), handle))
	{
		if(!iptc_flush_entries(strdup("ifacenat"),handle))
			printf("Failed in flushing rules of ifacenat in iptables\r\n");
	}
	else
		printf("No chain detected with the name ifacenat for IPv4\r\n");

	list_for_each(p, &ifaceNATs) {

		ifNAT = list_entry(p, struct uci_ifaceNAT, list);

		if (ifNAT->enable)
			insert_iface_nat(ifNAT, ctx, handle);
		else
			printf("Rule with the name:%s is disabled\n",ifNAT->name);
	}

	y = iptc_commit(handle);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of ifaceNAT rules(nat table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of ifaceNAT rules(nat table).\n",
				iptc_strerror(errno),errno);

		//return -1;
	}

	iptc_free(handle);
}

