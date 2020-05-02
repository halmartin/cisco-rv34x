/*
 * testApp.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "iptables.h"
#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

extern struct list_head aclRules;
extern struct list_head allIfaces;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;

void print_acl(struct uci_aclRule *acl)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");

	if(acl)
		printf("Complete firewall rule is '%s'\n"
			"	enabled:	%s\n"
			"	target: 	%s\n"
			"	family: 	%d\n"
			"	protocol:	%s\n"
			"	protocol_number:%d\n"
			"	port_start:	%d\n"
			"	port_end:	%d\n"
			"	icmp_type:	%d\n"
			"	log:		%s\n"
			"	src:		%s\n"
			"	src_ip:		%s\n"
			"	src_subnet:	%d\n"
			"	src_ip_end:	%s\n"
			"	dest:		%s\n"
			"	dest_ip:	%s\n"
			"	dest_subnet:	%d\n"
			"	dest_ip_end:	%s\n"
			"	schedule:	%s\n",
			acl->name,
			(acl->enable ? "Yes" : "No"),
			acl->target,
			acl->family,
			acl->protocol,
			acl->protocol_number,
			acl->port_start,
			acl->port_end,
			acl->icmp_type,
			(acl->log ? "Yes" : "No"),
			acl->src,
			acl->src_ip,
			acl->src_subnet,
			acl->src_ip_end,
			acl->dest,
			acl->dest_ip,
			acl->dest_subnet,
			acl->dest_ip_end,
			acl->schedule);
}

static int insert_rule(struct uci_aclRule *acl, struct uci_context *sched_ctx, struct iptc_handle *handle)
{ //Though the name of second arg is sched_ctx, this is not a new context. But it just resembles that it will be used for retriving schedule info  
	int range_loaded=0,ret;
	char final_src[16]="";
	char final_dest[16]="";
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	struct iface_data *findIface;
	char *tablename="filter";

	//Optimization: Convert below bools to a single integer variable and use each bit out of it for each use.
	bool need_udp_rule=0;

	mynewargc = 0;

	add_argv("iptables-uci");
	add_argv("-t");
	add_argv("filter -w");
	add_argv("-A");
	add_argv("forward_rule");

	//Handling SRC interface.
	if(strncmp(acl->src,"vlan",4)==0)
	{
		findIface = search_iface(acl->src);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",acl->src);
			free_argv();
			return 1;
		}	
		add_argv("-i");
		//add_argv(final_src);
		add_argv(findIface->l3ifname);
	}
	else if(strstr(acl->src,"_tun"))
	{//Is a tun kind of interface.
		findIface = search_iface(acl->src);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",acl->src);
			free_argv();
			return 1;
		}
		add_argv("-i");
		add_argv(findIface->l3ifname);
	}
	else if(strncmp(acl->src,"wan",3)==0)
	{ //Is some WAN interface
		if(acl->family==6)
		{ //We should search for the mapping l3device with the name wan1/2/etc appended with "6"
			strncpy(final_src,acl->src,4); //copied wanX
			sprintf(final_src,"%s6%s",final_src,acl->src+5);
			printf("final V6 string is :%s\r\n",final_src);  //DEBUG only.
			findIface = search_iface(final_src);
			if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
			{
				printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",final_src);
				free_argv();
				return 1;
			}
		}
		else
		{
			findIface = search_iface(acl->src);
			if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
			{
				printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",acl->src);
				free_argv();
				return 1;
			}
		}
		add_argv("-i");
		add_argv(findIface->l3ifname);
	}
	else if(strncmp(acl->src,"usb",3)==0)
	{
		findIface = search_iface(acl->src);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",acl->src);
			free_argv();
			return 1;
		}
		add_argv("-i");
		add_argv(findIface->l3ifname);
	}
	else if(strncmp(acl->src,"any",3)!=0)
	{ /*For an interface other than above mentioned and not any, try resolve it via search_iface function.*/
		findIface = search_iface(acl->src);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",acl->src);
			free_argv();
			return 1;
		}
		add_argv("-i");
		add_argv(findIface->l3ifname);
	}

	//Handling DEST interface.
	if(strncmp(acl->dest,"vlan",4)==0)
	{
		findIface = search_iface(acl->dest);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the dest interface:%s is not configured since the interface is not active/UP\r\n",acl->dest);
			free_argv();
			return 1;
		}
		add_argv("-o");
		add_argv(findIface->l3ifname);
	}
	else if(strstr(acl->dest,"_tun"))
	{//Is a tun kind of interface.
		findIface = search_iface(acl->dest);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the dest interface:%s is not configured since the interface is not active/UP\r\n",acl->dest);
			free_argv();
			return 1;
		}
		add_argv("-o");
		add_argv(findIface->l3ifname);
	}
	else if(strncmp(acl->dest,"wan",3)==0)
	{ //Is some WAN interface
		if(acl->family==6)
		{ //We should search for the mapping l3device with the name wan1/2/etc appended with "6"
			strncpy(final_dest,acl->dest,4); //copied wanX
			sprintf(final_dest,"%s6%s",final_dest,acl->dest+5);
			printf("final V6 string is :%s\r\n",final_dest);  //DEBUG only.
			findIface = search_iface(final_dest);
			if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
			{
				printf("insert_rule for the dest interface:%s is not configured since the interface is not active/UP\r\n",final_dest);
				free_argv();
				return 1;
			}
		}
		else
		{
			findIface = search_iface(acl->dest);
			if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
			{
				printf("insert_rule for the dest interface:%s is not configured since the interface is not active/UP\r\n",acl->dest);
				free_argv();
				return 1;
			}
		}
		add_argv("-o");
		add_argv(findIface->l3ifname);
	}
	else if(strncmp(acl->dest,"usb",3)==0)
	{
		findIface = search_iface(acl->dest);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the dest interface:%s is not configured since the interface is not active/UP\r\n",acl->dest);
			free_argv();
			return 1;
		}
		add_argv("-o");
		add_argv(findIface->l3ifname);
	}
	else if(strncmp(acl->dest,"any",3)!=0)
	{ /*For an interface other than above mentioned and not "any", try resolve it via search_iface function.*/
		findIface = search_iface(acl->dest);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("insert_rule for the src interface:%s is not configured since the interface is not active/UP\r\n",acl->dest);
			free_argv();
			return 1;
		}
		add_argv("-o");
		add_argv(findIface->l3ifname);
	}

	//Handling src IP
	if (strcmp(acl->src_ip,"any"))
	{// SRC IP is not ANY
		if(strlen(acl->src_ip)
		&& acl->src_ip_end
		&& strlen(acl->src_ip_end)
		&& strcmp(acl->src_ip, acl->src_ip_end))
		{//Handle IP Range here.
			range_loaded = 1;
			add_argv("--match");
			add_argv("iprange");
			add_argv("--src-range");
			char temp[24]="";
			sprintf(temp,"%s-%s",acl->src_ip,acl->src_ip_end);
			add_argv(temp);
		}
		else
		{
			add_argv("-s");
			if(acl->src_subnet)
			{
				char temp[24]="";
				sprintf(temp,"%s/%d",acl->src_ip,acl->src_subnet);
				add_argv(temp);
			}
			else
				add_argv(acl->src_ip);
		}
	
	}

	//Handling dest IP
	if (strcmp(acl->dest_ip,"any"))
	{//DEST IP is not ANY
		if(strlen(acl->dest_ip)
		&& acl->dest_ip_end
		&& strlen(acl->dest_ip_end)
		&& strcmp(acl->dest_ip, acl->dest_ip_end))
		{//Handle IP Range here.
			if (!range_loaded)
			{
				range_loaded = 1;
				add_argv("--match");
				add_argv("iprange");
			}
			add_argv("--dst-range");
			char temp[24]="";
			sprintf(temp,"%s-%s",acl->dest_ip,acl->dest_ip_end);
			add_argv(temp);
		}
		else
		{
			add_argv("-d");
			if(acl->dest_subnet)
			{
				char temp[24]="";
				sprintf(temp,"%s/%d",acl->dest_ip,acl->dest_subnet);
				add_argv(temp);
			}
			else
				add_argv(acl->dest_ip);
		}
	}

	//Handling protocol
	if(strcmp(acl->protocol,"icmp")==0)
	{
		if(acl->family==4)
		{
			if(acl->icmp_type==0)
			{
				add_argv("--protocol");
				add_argv("icmp");
			}
			else
			{
				char icmp_type[8]="";
				add_argv("--protocol");
				add_argv("icmp");
				add_argv("--icmp-type");
				sprintf(icmp_type,"%d",acl->icmp_type);
				add_argv(icmp_type);
			}
		}
		else
		{ //IPv6 family.
			if(acl->icmp_type==0)
			{//Special case. Not clear for us, contact nagesh. TODO
				add_argv("--protocol");
				add_argv("icmpv6");
			}
			else if (acl->icmp_type==8)
			{
				add_argv("--protocol");
				add_argv("icmpv6");
				add_argv("--icmpv6-type");
				add_argv("128");
			}
			else
			{
				char icmp_type[8]="";
				add_argv("--protocol");
				add_argv("icmpv6");
				add_argv("--icmpv6-type");
				sprintf(icmp_type,"%d",acl->icmp_type);
				add_argv(icmp_type);
			}
		}
	}
	else if (strcmp(acl->protocol,"ip")==0)
	{
		if(acl->protocol_number)
		{
			char protocol_number[8]="";
			add_argv("--protocol");
			sprintf(protocol_number,"%d",acl->protocol_number);
			add_argv(protocol_number);
		}
	}
	else if (strcmp(acl->protocol,"tcp")==0)
	{
		if (acl->port_start && acl->port_end)
		{
			add_argv("--protocol");
			add_argv("tcp");
			if(acl->port_start == acl->port_end)
			{
				char port_start[8]="";
				sprintf(port_start,"%d",acl->port_start);
				add_argv("--destination-port");
				add_argv(port_start);
			}
			else
			{
				char port_start_end[16]="";
				sprintf(port_start_end,"%d:%d",acl->port_start,acl->port_end);
				add_argv("--destination-port");
				add_argv(port_start_end);
			}
		}
	}
	else if (strcmp(acl->protocol,"udp")==0)
	{
		if (acl->port_start && acl->port_end)
		{
			add_argv("--protocol");
			add_argv("udp");
			if(acl->port_start == acl->port_end)
			{
				char port_start[8]="";
				sprintf(port_start,"%d",acl->port_start);
				add_argv("--destination-port");
				add_argv(port_start);
			}
			else
			{
				char port_start_end[16]="";
				sprintf(port_start_end,"%d:%d",acl->port_start,acl->port_end);
				add_argv("--destination-port");
				add_argv(port_start_end);
			}
		}
	}
	else if (strcmp(acl->protocol,"all")==0)
	{ //When protocol is "any", it means tcp+udp. So we need two iptable rules.
		//As of now, we will configure a TCP rule and set  a flag for an udp rule that
		//	is configured at the end.
		if (acl->port_start && acl->port_end)
		{
			need_udp_rule=1;
			add_argv("--protocol");
			add_argv("tcp");
			if(acl->port_start == acl->port_end)
			{
				char port_start[8]="";
				sprintf(port_start,"%d",acl->port_start);
				add_argv("--destination-port");
				add_argv(port_start);
			}
			else
			{
				char port_start_end[16]="";
				sprintf(port_start_end,"%d:%d",acl->port_start,acl->port_end);
				add_argv("--destination-port");
				add_argv(port_start_end);
			}
		}

	}
	
	if(strlen(acl->schedule))
	{
		char tuple[32]="";
		bool dayConfig=0;
		char dayString[16]="";

		sprintf(tuple,"schedule.%s",acl->schedule);

		if (uci_lookup_ptr(sched_ctx, &ptr, tuple, true) != UCI_OK) {
			printf("SKC: Error in getting info about:%s\r\n",tuple);
			return 1;
		}

		add_argv("--match");
		add_argv("time");
        add_argv("--kerneltz");

		uci_foreach_element(&(ptr.s)->options, e) {
			struct uci_option *o = uci_to_option(e);

			if(strcmp(o->e.name,"start_time")==0)
			{
				add_argv("--timestart");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"end_time")==0)
			{
				add_argv("--timestop");
				add_argv(o->v.string);
			}
			else if (strcmp(o->e.name,"sun")==0)
			{//During parsing Sun will come first.
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Su");
					else
					{
						sprintf(dayString,"Su");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"mon")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Mo");
					else
					{
						sprintf(dayString,"Mo");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"tue")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Tu");
					else
					{
						sprintf(dayString,"Tu");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"wed")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",We");
					else
					{
						sprintf(dayString,"We");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"thu")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Th");
					else
					{
						sprintf(dayString,"Th");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"fri")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Fr");
					else
					{
						sprintf(dayString,"Fr");
						dayConfig=1;
					}
				}
			}
			else if (strcmp(o->e.name,"sat")==0)
			{
				if(strcmp(o->v.string,"1")==0)
				{
					if(dayConfig)
						strcat(dayString,",Sa");
					else
					{
						sprintf(dayString,"Sa");
						dayConfig=1;
					}
				}
			}
		}

		if(dayConfig)
		{
			add_argv("--weekdays");
			add_argv(dayString);
		}
	
		if (ptr.p)
			uci_unload(sched_ctx, ptr.p);
	}

	if (!acl->log)
	{

		add_argv("-j");
		if(!strcmp(acl->target,"Allow"))
			add_argv("ACCEPT");
		else
			add_argv("DROP");
	}
	else
	{
		add_argv("-j");
		if(!strcmp(acl->target,"Allow"))
			add_argv("accept_log");
		else
			add_argv("drop_log");
	}

	if(enable_debug)
		print_acl(acl);

	if (acl->family==4)
	{
		ret = do_command(mynewargc, mynewargv, &tablename, &handle);
		//ret = do_command(mynewargc, mynewargv, "filter", &handle);
		if (!ret) {
			printf("do_command failed.string:%s\n",iptc_strerror(errno));
			print_acl(acl);
		}
	}
	free_argv();

	if(need_udp_rule)
	{
		struct uci_aclRule temp;
		memcpy(&temp,acl,sizeof(temp));

		temp.protocol="udp";
		insert_rule(&temp,sched_ctx, handle);
	}

	return 0;
}

void config_aclrule(struct uci_context *ctx)
{
	struct list_head *p;
	struct uci_aclRule *acl;
	int y=0;
	struct iptc_handle *handle = NULL;

	handle = create_handle("filter");

	if(iptc_is_chain(strdup("forward_rule"), handle))
	{
		if(!iptc_flush_entries(strdup("forward_rule"),handle))
			printf("Failed in flushing rules of forward_rule in iptables\r\n");
	}
	else
		printf("No chain detected with that name for IPv4\r\n");

	list_for_each(p, &aclRules) {

		acl = list_entry(p, struct uci_aclRule, list);

		if (acl->enable && acl->family==4)
			insert_rule(acl, ctx, handle);
		/*else
			printf("Rule with the name:%s is disabled\n",acl->name);*/
	}

	y = iptc_commit(handle);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of ACL v4 even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of ACL v4.\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	iptc_free(handle);
}

