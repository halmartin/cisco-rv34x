/*
 * testApp.c - sample code for the ucimap/libuci library
 */
#include "ip6tables-uci.h"
#include "xtables.h"
#include "libiptc/libip6tc.h"
#include "ip6tables-multi.h"

struct list_head aclRules;
struct list_head allIfaces;

/* global new argv and argc */
char *mynewargv[255];
int mynewargc;
static char * tablename="filter";
static int enable_debug=0;
unsigned int IfaceStatsId=0;

void sig_handler(int received_signal)
{
	int a=0;
	printf("Received signal Num is:%d\r\n",received_signal);
	printf("The parsed ACL rule record that is in progress when the signal received is:\r\n");

	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");

}

/* function adding one argument to newargv, updating mynewargc
 * returns true if argument added, false otherwise */
int add_argv(char *what) {
	if (what && mynewargc + 1 < ARRAY_SIZE(mynewargv)) {
		mynewargv[mynewargc] = strdup(what);
		mynewargc++;
		return 1;
	} else
		return 0;
}

void free_argv(void) {
	int i;

	for (i = 0; i < mynewargc; i++)
		free(mynewargv[i]);

	mynewargc = 0;
}

struct iface_data * search_iface(char * iface_name)
{
	struct list_head *p;
	struct list_head *found=NULL;
	struct iface_data *data;
	int foundAt=0;
	int length=0;

	list_for_each(p, &allIfaces) {
		data = list_entry (p, struct iface_data, list);

	//	printf("iface:%s status:%d ip4addr:%s ip4subnet:%d l2ifname:%s l3ifname:%s \r\n",data->ifname,
	//			data->status, data->ipaddr, data->subnet, data->l2ifname, data->l3ifname);

		if(strcmp(data->ifname,iface_name)==0)
		{
			found = p;
			//printf("Found at the length:%d\r\n",length);
			foundAt = length;
			break;
		}
		length++;
	}

	if(foundAt>6)
	{ /*This is under an assumption that the nodes that are requested for be at near vicinity from head node.
		But why 6: Assuming wan1, wan16, wan2, wan26, vlan1 and probably a new vlan2. So total all including 6 */	
		list_del(found);
		list_add(found,&allIfaces);
	}

	if (found)
		return list_entry (found, struct iface_data, list);

	printf("found is NULL here. But this should not occur.\r\n");
	//return found; //It will be NULL here. But this should not occur at all.
	return NULL; //It will be NULL here. But this should not occur at all.
}

void print_usage()
{
	fprintf(stderr, "USGAE: ip6tables-uci [ <List of modules to be reloaded> ] fileID:<Iface-File-ID>\n"
			"	Iface-File-ID: This file ID is PID number of process who invoke this application.\n"
			"			This ID is also the file extension of Iface stats file.\n"
			"	List of modules to reload can be:\n"
			"		acl basicsettings\n"
			"		debug\n"
			"	Example: ip6tables-uci debug acl fileID:4399\r\n"
		);
	exit(1);
}

void print_allIfaces()
{
	struct list_head *p;
	struct iface_data *data;

	printf("Here are the elements after reorganisation:\r\n");
	list_for_each(p, &allIfaces) {
		data = list_entry (p, struct iface_data, list);

		printf("iface:%s status:%d ip4addr:%s ip4subnet:%d l2ifname:%s l3ifname:%s \r\n",data->ifname,
				data->status, data->ipaddr, data->subnet, data->l2ifname, data->l3ifname);
	}
}

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
			"	src_prefix:	%d\n"
			"	src_ip_end:	%s\n"
			"	dest:		%s\n"
			"	dest_ip:	%s\n"
			"	dest_prefix:    %d\n"
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
			acl->src_prefix,
			acl->src_ip_end,
			acl->dest,
			acl->dest_ip,
			acl->dest_prefix,
			acl->dest_ip_end,
			acl->schedule);
}

void readIfaceFile()
{ //This reads the file and get the data structure ready for processing.
	FILE *fp;
	char buff[256];
	char ifname[24];
	int status=0;
	char ipaddr[48];
	char ip6addr[48];
	int subnet=0;
	int ip6mask=0;
	char l2ifname[24];
	char l3ifname[24];
	char finalFileName[32]="";

	sprintf(finalFileName,"%s.%u",TMP_FWIFACESTATS,IfaceStatsId);
	if (enable_debug)
		printf("Final Stats filename is:%s\r\n",finalFileName);
	fp=fopen(finalFileName,"r");

	while(fgets(buff,256,fp))
	{
		struct iface_data *newIface = malloc(sizeof(struct iface_data));
		memset(newIface, 0, sizeof(struct iface_data));
		INIT_LIST_HEAD(&newIface->list);

		sscanf (buff,"iface:%s status:%d ip4addr:%s ip4subnet:%d ip6addr:%s ip6mask:%d l2ifname:%s l3ifname:%s",
						ifname,&status,ipaddr,&subnet,ip6addr,&ip6mask,l2ifname,l3ifname);
//		printf("iface:%s status:%d ip4addr:%s ip4subnet:%d ip6addr:%s ip6mask:%d l2ifname:%s l3ifname:%s\r\n",
//						ifname,status,ipaddr,subnet,ip6addr,ip6mask,l2ifname,l3ifname);

		strcpy(newIface->ifname,ifname);
		newIface->status=status;
		strcpy(newIface->l2ifname,l2ifname);
		strcpy(newIface->l3ifname,l3ifname);
		if(strncmp(newIface->ifname,"wan16",5)==0 || strncmp(newIface->ifname,"wan26",5)==0
                || (strncmp(newIface->l3ifname,"6in4",4)==0) || (strncmp(newIface->l3ifname,"6rd",3)==0)) //V6 type
		{
			strcpy(newIface->ipaddr,ip6addr);
			newIface->subnet=ip6mask;
		}
		else
		{
			strcpy(newIface->ipaddr,ipaddr);
			newIface->subnet=subnet;
		}

		list_add_tail(&newIface->list, &allIfaces);

		if (strcmp(ifname,"wan1") == 0) //l2ifname consists of the physical ifname.
			strcpy(gWan1_phy_iface, l2ifname);
		else if (strcmp(ifname,"vlan1") == 0)
		{
			strncpy(gLan_phy_iface, l2ifname, 4);  //Eg: only 4chars of eth3.200 = eth3
			gLan_phy_iface[4] = '\0';
		}
	}

	//printf("#### wan1 and lan physical device are %s and %s respectively.\r\n",gWan1_phy_iface, gLan_phy_iface);
	fclose(fp);
}

struct ip6tc_handle *create6_handle(const char *fortablename)
{
	struct ip6tc_handle *newhandle;

	newhandle = ip6tc_init(fortablename);

	if (!newhandle) {
		/* try to insmod the module if iptc_init failed */
		printf("Trying to insmode the details\r\n");
		xtables_load_ko(xtables_modprobe_program, false);
		newhandle = ip6tc_init(fortablename);
	}

	if (!newhandle) {
		xtables_error(PARAMETER_PROBLEM, "unable to initialize "
			"table '%s'\n", fortablename);
		exit(1);
	}

	return newhandle;
}

//static int insert_rule(struct uci_aclRule *acl)
static int insert_rule(struct uci_aclRule *acl, struct uci_context *sched_ctx, struct ip6tc_handle *handle6)
{
	int range_loaded=0,ret;
	char final_src[16]="";
	char final_dest[16]="";
	struct uci_element *e = NULL;
	struct uci_ptr ptr;
	struct iface_data *findIface;

	//Optimization: Convert below bools to a single integer variable and use each bit out of it for each use.
	bool need_udp_rule=0;

	mynewargc = 0;

	add_argv("ip6tables-uci");
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
            if(strlen(acl->src) > 4)
			    sprintf(final_src,"%s6%s",final_src,acl->src+4);
            else
                sprintf(final_src,"%s6",final_src);

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
/*	else if (strcmp(acl->src,"any")==0)
	{
		add_argv("-i");
		add_argv(acl->src);
	}*/

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
            if(strlen(acl->dest) > 4)
                sprintf(final_dest,"%s6%s",final_dest,acl->dest+4);
            else
                sprintf(final_dest,"%s6",final_dest);

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
/*	else
	{
		add_argv("-o");
		add_argv(acl->dest);
	}*/

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
			if(acl->src_prefix)
			{
				char temp[24]="";
				sprintf(temp,"%s/%d",acl->src_ip,acl->src_prefix);
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
			if(acl->dest_prefix)
			{
				char temp[24]="";
				sprintf(temp,"%s/%d",acl->dest_ip,acl->dest_prefix);
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
			//cli_perror();
			return 1;
		}

		add_argv("--match");
		add_argv("time");
        add_argv("--kerneltz");

		//e = ptr.last;
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

	if (acl->family==6)
	{
		ret = do_command6(mynewargc, mynewargv,&tablename,&handle6, true);
		if (!ret) {
			printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
			print_acl(acl);
		}
	}
	//printf("do_command call finished\r\n");
	free_argv();

	if(need_udp_rule)
	{
		struct uci_aclRule temp;
		memcpy(&temp,acl,sizeof(temp));

		temp.protocol="udp";
		insert_rule(&temp,sched_ctx, handle6);
	}

	return 0;
}

static int
firewall_parse_ip(void *section, struct uci_optmap *om, union ucimap_data *data, const char *str)
{
	unsigned char *target;
	int tmp[4];
	int i;

	if (sscanf(str, "%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3]) != 4)
		return -1;

	target = malloc(4);
	if (!target)
		return -1;

	data->ptr = target;
	for (i = 0; i < 4; i++)
		target[i] = (char) tmp[i];

	return 0;
}

static int
firewall_format_ip(void *section, struct uci_optmap *om, union ucimap_data *data, char **str)
{
	static char buf[16];
	unsigned char *ip = (unsigned char *) data->ptr;

	if (ip) {
		sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
		*str = buf;
	} else {
		*str = NULL;
	}

	return 0;
}

static void
firewall_free_ip(void *section, struct uci_optmap *om, void *ptr)
{
	free(ptr);
}

static int
firewall_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_aclRule *acl = section; 

	INIT_LIST_HEAD(&acl->list);
	acl->name = s->e.name;
	acl->test = -1;
	return 0;
}

static int
firewall_add_rule(struct uci_map *map, void *section)
{
	struct uci_aclRule *acl = section; 

	list_add_tail(&acl->list, &aclRules);

	return 0;
}

static struct ucimap_section_data *
firewall_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_aclRule *p = malloc(sizeof(struct uci_aclRule));
	memset(p, 0, sizeof(struct uci_aclRule));
	return &p->map;
}

struct my_optmap {
	struct uci_optmap map;
	int test;
};

static struct uci_sectionmap firewall_acls;

static struct my_optmap firewall_aclRule_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, enable),
			.type = UCIMAP_BOOL,
			.name = "enable",
		}
	},
	{

		.map = {
			UCIMAP_OPTION(struct uci_aclRule, target),
			.type = UCIMAP_STRING,
			.name = "target",
			.data.s.maxlen = 32,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, family),
			.type = UCIMAP_INT,
			.name = "family"
		}
	},
	{

		.map = {
			UCIMAP_OPTION(struct uci_aclRule, protocol),
			.type = UCIMAP_STRING,
			.name = "protocol",
			.data.s.maxlen = 32,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, protocol_number),
			.type = UCIMAP_INT,
			.name = "protocol_number"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, port_start),
			.type = UCIMAP_INT,
			.name = "port_start"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, port_end),
			.type = UCIMAP_INT,
			.name = "port_end"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, icmp_type),
			.type = UCIMAP_INT,
			.name = "icmp_type"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, log),
			.type = UCIMAP_BOOL,
			.name = "log",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, src),
			.type = UCIMAP_STRING,
			.name = "src"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, src_ip),
			.type = UCIMAP_STRING,
			.name = "src_ip"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, src_prefix),
			.type = UCIMAP_INT,
			.name = "src_prefix"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, src_ip_end),
			.type = UCIMAP_STRING,
			.name = "src_ip_end"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, dest),
			.type = UCIMAP_STRING,
			.name = "dest"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, dest_ip),
			.type = UCIMAP_STRING,
			.name = "dest_ip"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, dest_prefix),
			.type = UCIMAP_INT,
			.name = "dest_prefix"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, dest_ip_end),
			.type = UCIMAP_STRING,
			.name = "dest_ip_end"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_aclRule, schedule),
			.type = UCIMAP_STRING,
			.name = "schedule"
		}
	}
};

static struct uci_sectionmap firewall_aclRules = {
	UCIMAP_SECTION(struct uci_aclRule, map),
	.type = "rule",
	.alloc = firewall_allocate,
	.init = firewall_init_rule,
	.add = firewall_add_rule,
	.options = &firewall_aclRule_options[0].map,
	.n_options = ARRAY_SIZE(firewall_aclRule_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap *firewall_smap[] = {
	&firewall_aclRules,
};

static struct uci_map firewall_map = {
	.sections = firewall_smap,
	.n_sections = ARRAY_SIZE(firewall_smap),
};

void config_aclrule6(struct uci_context *ctx)
{
	struct list_head *p;
	struct uci_aclRule *acl;
	int y=0;
	struct ip6tc_handle *handle6 = NULL;

	handle6 = create6_handle("filter");
	if(ip6tc_is_chain(strdup("forward_rule"), handle6))
	{
		if(!ip6tc_flush_entries(strdup("forward_rule"),handle6))
			printf("Failed in flushing rules of forward_rule in ip6tables\r\n");
	}
	else
		printf("No chain detected with that name for IPv6\r\n");

	list_for_each(p, &aclRules) {

		acl = list_entry(p, struct uci_aclRule, list);

		if (acl->enable && acl->family==6)
			insert_rule(acl,ctx,handle6);
		/*else
			printf("Rule with the name:%s is disabled/NOT v6\n",acl->name);*/
	}

	y = ip6tc_commit(handle6);
	if (!y)
	{
		printf("Error commiting data for IPv6: %s errno:%d\n", ip6tc_strerror(errno),errno);
		//return -1;
	}

	ip6tc_free(handle6);
}


int main(int argc, char **argv)
{
	struct uci_package *pkg;
	struct uci_context *ctx;
	struct modules config_reload={0,0,0,0};
	int c=0;
	int argcount=1;

	if(argc > 1)
		while (argcount < argc)
		{
			if (strcmp(argv[argcount],"acl")==0)
				config_reload.access_rules=1;
			else if (strcmp(argv[argcount],"basicsettings")==0)
				config_reload.basic_settings=1;
			else if (strcmp(argv[argcount],"spoofrules")==0)
				config_reload.spoof_rules=1;
			else if (strcmp(argv[argcount],"debug")==0)
				enable_debug=1;
			else if (strncmp(argv[argcount],"fileID:",7)==0)
				sscanf(argv[argcount],"fileID:%u",&IfaceStatsId);
			else //Dude, this exits the program as well. We may need to do clean up activities also if needed. TODO
				print_usage();

			argcount++;
		}

	if(IfaceStatsId <= 0)
	{
		printf("Iface Stats ID must be given and is a valid ID.\r\n");
		print_usage();
		exit(1);
	}

	//1. INIT DS.
	INIT_LIST_HEAD(&aclRules);
	INIT_LIST_HEAD(&allIfaces);
	readIfaceFile();

	//2. init iptables infra.
	ip6tables_globals.program_name = "iptables-uci";
	c = xtables_init_all(&ip6tables_globals, NFPROTO_IPV6);
	if (c<0)
	{
		printf("Failed to initialize xtables for IPv6.%s\r\n",ip6tc_strerror(errno));
		exit(1);
	}
	init_extensions();
	init_extensions6();

	//3. Create UCI context and init other things.
	ctx = uci_alloc_context();
	ucimap_init(&firewall_map);

	if (uci_load(ctx, "firewall", &pkg))
	{
		//uci_perror(state->uci, NULL);
		fprintf(stderr, "Error: Can't load firewall config");
		return 0;
	}

	//4. Parse API 
	ucimap_parse(&firewall_map, pkg);

	//5.Call functions to load all the relevant rules.

	if (config_reload.basic_settings == 1)
		config_basicsettings6(ctx);

	if (config_reload.access_rules == 1)
		config_aclrule6(ctx);


	ucimap_cleanup(&firewall_map);
	uci_free_context(ctx);

	return 0;
}
