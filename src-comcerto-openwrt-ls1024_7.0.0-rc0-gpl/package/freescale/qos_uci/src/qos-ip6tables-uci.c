/*
 * qos-ip6tables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "qos-ip6tables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

/*ip6tables -A egress_chain -t mangle $protocol$sport$dport$sip$dip$rcvint$outint$dscpval-m comment --comment "$1$2" -j QOSCONNMARK --set-xmark $queue/0xf0f0000000ffffff*/
#define EGRESS_IPTABLE_RULE(rcviface,outiface) do{\
		memset(commandBuff, '\0', sizeof(commandBuff));\
		sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s%s%s-m comment --comment %s%s"\
				" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,\
				 rcviface, outiface, dscp, class_name, flow_name, queue_value);\
		tokenize_and_docommand(commandBuff, "mangle", handle);\
	}while(0)

#define EGRESS_IPTABLE_RULE_WITH_TCP_UDP(protocol,rcviface,outiface) do{\
		memset(commandBuff, '\0', sizeof(commandBuff));\
		sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s%s%s-m comment --comment %s%s"\
				" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,\
				 rcviface, outiface, dscp, class_name, flow_name, queue_value);\
		tokenize_and_docommand(commandBuff, "mangle", handle);\
	}while(0)

struct list_head allIfaces;

/* global new argv and argc */
char *newargv_g[255];
int newargc_g;
static char * tablename="filter";
static char * nat_tablename="nat";
static char * mangle_table="mangle";

int enable_debug=0;
unsigned int IfaceStatsId=0;
bool wifiboard=0;
char this_board[8]="";

void sig_handler(int received_signal)
{
	int a=0;
	printf("Received signal Num is:%d\r\n",received_signal);
	printf("The parsed ACL rule record that is in progress when the signal received is:\r\n");

	for (a = 0; a < newargc_g; a++)
		printf("argv[%u]: %s\t", a, newargv_g[a]);
	printf("\r\n");
	exit(1);
}

/* function adding one argument to newargv, updating newargc_g
 * returns true if argument added, false otherwise */
int add_argv(char *what) {
	if (what && newargc_g + 1 < ARRAY_SIZE(newargv_g)) {
		newargv_g[newargc_g] = strdup(what);
		newargc_g++;
		return 1;
	} else
		return 0;
}

void free_argv(void) {
	int i;

	for (i = 0; i < newargc_g; i++)
		free(newargv_g[i]);
}

struct iface_data *search_iface(char * iface_name)
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

	if(foundAt>8)
	{ /*This is under an assumption that the nodes that are requested for be at near vicinity from head node.
		But why 8: Assuming wan1, wan16, wan2, wan26, usb1, usb2, vlan1 and probably a new vlan2. So total 8 */	
		list_del(found);
		list_add(found,&allIfaces);
	}

	if (found)
		return list_entry (found, struct iface_data, list);

	printf("found is NULL here. But this should not occur for interface:%s\r\n",iface_name);
	//return found; //It will be NULL here. But this should not occur at all.
	return NULL;
}

void print_allIfaces()
{
	struct list_head *p;
	struct iface_data *data;

	printf("Here are the elements after reorganisation:\r\n");
	list_for_each(p, &allIfaces) {
		data = list_entry (p, struct iface_data, list);

		printf("iface:%s status:%d l2ifname:%s l3ifname:%s \r\n",data->ifname,
				data->status, data->l2ifname, data->l3ifname);
	}
}

void print_flowEntry(struct qos_flow *flow)
{
	printf("Received flow entry is: proto:%s icmptype:%s sport:%s dport:%s sip:%s dip:%s dscp:%s"
			"rcvinterface:%s mark_dscp:%d ip_version:%d queue:%s\n",
			flow->proto,flow->icmptype,flow->sport,flow->dport,flow->sip,flow->dip,flow->dscp,
			flow->rcvinterface,flow->mark_dscp,flow->ip_version,flow->queue);
}
void readIfaceFile()
{ //This reads the file and get the data structure ready for processing.
	FILE *fp = NULL;
	char buff[256];
	char ifname[24];
	int status=0;
/*	char ipaddr[48];
	char ip6addr[48];
	int subnet=0;
	int ip6mask=0;*/
	char l2ifname[24];
	char l3ifname[24];
	char finalFileName[32]="";

	sprintf(finalFileName,"%s.%u", TMP_QOSIFACESTATS, IfaceStatsId);
	if (enable_debug)
		printf("Final Stats filename is:%s\r\n",finalFileName);
	fp=fopen(finalFileName,"r");

	if(NULL == fp)
	{
		printf("Failed to open file:%s\r\n",finalFileName);
	}

	while(fgets(buff,256,fp))
	{
		struct iface_data *newIface = malloc(sizeof(struct iface_data));
		memset(newIface, 0, sizeof(struct iface_data));
		INIT_LIST_HEAD(&newIface->list);

		sscanf (buff,"iface:%s status:%d l2ifname:%s l3ifname:%s", ifname, &status, l2ifname, l3ifname);
		if (enable_debug)
			printf("iface:%s status:%d l2ifname:%s l3ifname:%s\r\n", ifname, status, l2ifname,l3ifname);

		strcpy(newIface->ifname,ifname);
		newIface->status=status;
		strcpy(newIface->l2ifname,l2ifname);
		strcpy(newIface->l3ifname,l3ifname);

		list_add_tail(&newIface->list, &allIfaces);

		if (strcmp(ifname,"wan1") == 0) //l2ifname consists of the physical ifname.
			strcpy(gWan1_phy_iface, l2ifname);
		else if (strcmp(ifname,"wan2") == 0)
			strcpy(gWan2_phy_iface, l2ifname);
		else if (strcmp(ifname,"vlan1") == 0)
		{
			strncpy(gLan_phy_iface, l2ifname, 4);  //only 4chars of eth3.XYZ = eth3
			gLan_phy_iface[4] = '\0';
		}
	}

	printf("#### wan1, wan2 and lan physical device are %s, %s and %s respectively.\r\n",
					gWan1_phy_iface, gWan2_phy_iface, gLan_phy_iface);
	fclose(fp);
}

//Utility function to tokenize and add the string to the given handle with do_command function
void tokenize_and_docommand(char *cmdBuff,char *table, struct ip6tc_handle *handle)
{
	char *token;
	int ret=0;
	char tempBuff[2048]="";

	strcpy(tempBuff,cmdBuff);

	/* get the first token */
	token = strtok(tempBuff, " ");

	newargc_g = 0; //Just to double ensure

	/* walk through other tokens */
	while(token != NULL)
	{
		add_argv(token);
		token = strtok(NULL, " ");
	}

	if(enable_debug)
	{
		printf("Received string for tokenize is:\n%s\n",cmdBuff);
	}
		

	ret = do_command6(newargc_g, newargv_g, &table, &handle, true);
	if (!ret)
		printf("do_command failed inside tokenize_and_docommand.string:%s\n",ip6tc_strerror(errno));

	free_argv();
	newargc_g = 0;
}

int strreplace(char *outputbuff,char * original_buff,char *search_string,char * replace_with)
{
	char *occurence = strstr(original_buff,search_string);

	if(!occurence)
	{
		printf("Returning because of non-occurence of search_string:%s\r\n",search_string);
		return -1;
	}

	strncpy(outputbuff,original_buff, (occurence - original_buff));
	sprintf(outputbuff,"%s%s%s",outputbuff,replace_with,occurence + strlen(search_string));
	return(0);
}

struct ip6tc_handle *create6_handle(const char *fortablename)
{
	struct ip6tc_handle *newhandle;

	newhandle = ip6tc_init(fortablename);

	if (!newhandle) {
		/* try to insmod the module if ip6tc_init failed */
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

void print_usage()
{
	fprintf(stderr, "USGAE: qos-ip6tables-uci <[ -df ]> <classname.flowname>\n"
			"			-d to enable debug\n"
			"			-f <fileID> which is the PID number of process who invoke this application.\n"
			"				This ID is also the file extension of Iface stats file.\n"
		);
	exit(1);
}

int get_queues_configured(struct queue_config *queueconf, char *policy, char *class_name, struct uci_context *ctx)
{
	char tuple[64]="";
	struct uci_ptr ptr;
	struct uci_element *e = NULL;

	sprintf(tuple,"qos.%s",policy);

	if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
		printf("Error in getting info about:%s\r\n",tuple);
		return -1;
	}

	uci_foreach_element(&(ptr.s)->options, e) {
		struct uci_option *o = uci_to_option(e);

		if(strcmp(o->e.name,"queue1") == 0 )
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue1 = 1;
		}
		else if (strcmp(o->e.name,"queue2") == 0 )
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue2 = 1;
		}
		else if (strcmp(o->e.name,"queue3") == 0)
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue3 = 1;
		}
		else if (strcmp(o->e.name,"queue4") == 0)
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue4 = 1;
		}
		else if (strcmp(o->e.name,"queue5") == 0)
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue5 = 1;
		}
		else if (strcmp(o->e.name,"queue6") == 0)
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue6 = 1;
		}
		else if (strcmp(o->e.name,"queue7") == 0)
		{
			if(strcmp(o->v.string,class_name) == 0)
				queueconf->queue7 = 1;
		}
		else
			printf("Unhandled option:%s\r\n",o->v.string);
	}

	return 0;
}

int add_rule_to_chain(char class_name[128],char flow_name[128], struct uci_context *ctx, struct ip6tc_handle *handle)
{
	char tuple[128]="";
	struct qos_flow flow = {"any","8","","","","","","",65,6,""};
	struct queue_config queueconf = { 0,0,0,0,0,0,0};
	char protocol[64]="", sport[32]="", dport[32]="", sip[64]="", dip[64]="", dscp[32]="";
	char commandBuff[2048]="";
	struct uci_ptr ptr;
	struct uci_element *e = NULL;
	struct iface_data *findIface;
	char mark_dscp[8]="";
	int i=0;

	memset(commandBuff, '\0', sizeof(commandBuff));
	memset(protocol, '\0', sizeof(protocol));
	memset(sport, '\0', sizeof(sport));
	memset(dport, '\0', sizeof(dport));
	memset(sip, '\0', sizeof(sip));
	memset(dip, '\0', sizeof(dip));
	memset(dscp, '\0', sizeof(dscp));
	memset(tuple, '\0', sizeof(tuple));

	sprintf(tuple,"qos.%s",flow_name);
	
	if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
		printf("Error in getting info about:%s\r\n",tuple);
		return 1;
	}

	uci_foreach_element(&(ptr.s)->options, e) {
		struct uci_option *o = uci_to_option(e);
//printf("For option:%s valueis:%s\r\n",o->e.name,o->v.string);

		if(strcmp(o->e.name,"proto")==0)
			strcpy(flow.proto, o->v.string);
		else if (strcmp(o->e.name,"icmptype")==0)
			strcpy(flow.icmptype, o->v.string);
		else if (strcmp(o->e.name,"icmp_type")==0) //We mentioned that we need the data in icmp_type in QoS FS.
			strcpy(flow.icmptype, o->v.string);
		else if (strcmp(o->e.name,"sport")==0)
			strcpy(flow.sport, o->v.string);
		else if (strcmp(o->e.name,"dport")==0)
			strcpy(flow.dport, o->v.string);
		else if (strcmp(o->e.name,"sip")==0)
			strcpy(flow.sip, o->v.string);
		else if (strcmp(o->e.name,"dip")==0)
			strcpy(flow.dip, o->v.string);
		else if (strcmp(o->e.name,"dscp")==0)
			strcpy(flow.dscp, o->v.string);
		else if (strcmp(o->e.name,"rcvinterface")==0)
			strcpy(flow.rcvinterface, o->v.string);
		else if (strcmp(o->e.name,"mark_dscp")==0)
			flow.mark_dscp=atoi(o->v.string);
		else if (strcmp(o->e.name,"ip_version")==0)
		{
			if (strcmp(o->v.string, "ipv4")==0)
				flow.ip_version = 4;
			else if (strcmp(o->v.string, "ipv6")==0)
				flow.ip_version = 6;
		}
		else if (strcmp(o->e.name,"queue")==0)
			strcpy(flow.queue, o->v.string);
		else if (strcmp(o->e.name,"classname")==0)
			continue;
		else
			printf("Unhandled option-name:%s option-value:%s\r\n", o->e.name, o->v.string);
	}

	if (ptr.p)
		uci_unload(ctx, ptr.p);

	if(enable_debug)
		print_flowEntry(&flow);

	if(flow.ip_version == 4)
		return -1;   //This file is for only v6 handling. Hence the ip6tables rules are v6 only.

	/*Handling of common parameters*/
	//Handling protocol
	if(strcmp("icmp",flow.proto)==0)
		sprintf(protocol,"-p icmpv6 --icmpv6-type %s ",flow.icmptype);
	else if (strcmp("tcp",flow.proto)==0 || strcmp("udp",flow.proto)==0 || strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
	{
		if (strlen(flow.sport) && (strcmp("Any",flow.sport)!=0)) //Not any
			sprintf(sport,"--sport %s ",flow.sport);
		if (strlen(flow.dport) && (strcmp("Any",flow.dport)!=0)) //Not any
			sprintf(dport,"--dport %s ",flow.dport);

		if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
			sprintf(protocol,"-p tcp ");
		else
			sprintf(protocol,"-p %s ",flow.proto);
	}
/*	else
	{
	} Were already set to null/empty*/

	//Handling SIP
	if(strlen(flow.sip))
		sprintf(sip,"-s %s ",flow.sip);

	//Handling DIP
	if(strlen(flow.dip))
		sprintf(dip,"-d %s ",flow.dip);

	//Handling DSCP remark
	if(flow.mark_dscp != 65)
		sprintf(mark_dscp,"%02x", (flow.mark_dscp * 2)+1);
	else
		sprintf(mark_dscp,"00");

	//Handling DSCP
	if(strlen(flow.dscp))
		sprintf(dscp,"-m dscp --dscp %s ",flow.dscp);

	if (strncmp(flow_name,"ingress",7)==0)
	{
		if (!strlen(flow.queue))
			sprintf(flow.queue,"0");
		if (strncmp(flow.rcvinterface,"wan1",4)==0)
		{
			do
			{
			memset(commandBuff, '\0', sizeof(commandBuff));
			sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i %s+ %s-m comment --comment %s%s "
					"-j QOSCONNMARK --set-mark 0x80A%s%s0000000000/0xffffffff00000000",
					protocol, sport, dport, sip, dip, gWan1_phy_iface, dscp, class_name, flow_name, flow.queue, mark_dscp);
			tokenize_and_docommand(commandBuff, "mangle", handle);

			memset(commandBuff, '\0', sizeof(commandBuff));
			sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i ppoe-wan16+ %s-m comment --comment %s%s "
					"-j QOSCONNMARK --set-mark 0x80A%s%s0000000000/0xffffffff00000000",
					protocol, sport, dport, sip, dip, dscp, class_name, flow_name, flow.queue, mark_dscp);
			tokenize_and_docommand(commandBuff, "mangle", handle);
			if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
			{
				memset(protocol, '\0', sizeof(protocol));
				sprintf(protocol,"-p udp ");
				i+=1;
			}

			}while(i == 1);

		}
		else if (strncmp(flow.rcvinterface,"wan2",4)==0)
		{
			do
			{
			memset(commandBuff, '\0', sizeof(commandBuff));
			sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i %s+ %s-m comment --comment %s%s "
					"-j QOSCONNMARK --set-mark 0x80B%s%s0000000000/0xffffffff00000000",
					protocol, sport, dport, sip, dip, gWan2_phy_iface, dscp, class_name, flow_name, flow.queue, mark_dscp);
			tokenize_and_docommand(commandBuff, "mangle", handle);

			memset(commandBuff, '\0', sizeof(commandBuff));
			sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i ppoe-wan26+ %s-m comment --comment %s%s "
					"-j QOSCONNMARK --set-mark 0x80B%s%s0000000000/0xffffffff00000000",
					protocol, sport, dport, sip, dip, dscp, class_name, flow_name, flow.queue, mark_dscp);
			tokenize_and_docommand(commandBuff, "mangle", handle);
			if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
			{
				memset(protocol, '\0', sizeof(protocol));
				sprintf(protocol,"-p udp ");
				i+=1;
			}

			}while(i == 1);

		}
		else if (strcmp(flow.rcvinterface,"usb1")==0)
		{
			/*USB1_INTERFACE*/
			findIface = search_iface(flow.rcvinterface);
			if(findIface && strcmp(findIface->l3ifname,"null")!=0)
			{
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i %s %s-m comment --comment %s%s "
						"-j QOSCONNMARK --set-mark 0x80C%s%s0000000000/0xffffffff00000000",
						protocol, sport, dport, sip, dip, findIface->l3ifname, dscp, class_name, flow_name, flow.queue, mark_dscp);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i %s %s-m comment --comment %s%s "
						"-j QOSCONNMARK --set-mark 0x80C%s%s0000000000/0xffffffff00000000",
						"-p udp ", sport, dport, sip, dip, findIface->l3ifname, dscp, class_name, flow_name, flow.queue, mark_dscp);
					tokenize_and_docommand(commandBuff, "mangle", handle);
				}
			}
			else
				printf("Rule for the receive interface: %s, could not be configured since it is not ACTIVE.\r\n",flow.rcvinterface);
		}
		else if (strcmp(flow.rcvinterface,"usb2")==0)
		{
			/*USB2_INTERFACE*/
			findIface = search_iface(flow.rcvinterface);
			if(findIface && strcmp(findIface->l3ifname,"null")!=0)
			{
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i %s %s-m comment --comment %s%s "
						"-j QOSCONNMARK --set-mark 0x80D%s%s0000000000/0xffffffff00000000",
						protocol, sport, dport, sip, dip, findIface->l3ifname, dscp, class_name, flow_name, flow.queue, mark_dscp);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A ingress_chain -t mangle %s%s%s%s%s-i %s %s-m comment --comment %s%s "
						"-j QOSCONNMARK --set-mark 0x80D%s%s0000000000/0xffffffff00000000",
						"-p udp ", sport, dport, sip, dip, findIface->l3ifname, dscp, class_name, flow_name, flow.queue, mark_dscp);
					tokenize_and_docommand(commandBuff, "mangle", handle);
				}
			}
			else
				printf("Rule for the receive interface: %s, could not be configured since it is not ACTIVE.\r\n",flow.rcvinterface);
		}
	}
	else if (strncmp(flow_name,"egress",6)==0)
	{
		char final_rcv_iface[64]="", final_rcvwifi_iface[64]="";
		char policy1[128]="", policy2[128]="", policy3[128]="", policy4[128]="";
		int n=0;

//printf("final_rcv_iface:%s\r\n",final_rcv_iface);
		if(strcasecmp(flow.rcvinterface,"any")!=0)
		{
			sprintf(final_rcv_iface,"-i %s.%s ", gLan_phy_iface, flow.rcvinterface + 4);
			if (wifiboard)
				sprintf(final_rcvwifi_iface,"-i br-%s ",flow.rcvinterface);
		}

		//Adding egress_flow in class_chain
		memset(commandBuff, '\0', sizeof(commandBuff));
		sprintf(commandBuff,"qos-ip6tables-uci -A class_chain -t mangle %s%s%s%s%s%s-o outint %s-m comment --comment %s%s"
				" -j QOSCONNMARK --set-mark 0x80%s00", protocol, sport, dport, sip, dip,
				 final_rcv_iface, dscp, class_name, flow_name, mark_dscp);
		tokenize_and_docommand(commandBuff, "mangle", handle);
	
		if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
		{
			memset(commandBuff, '\0', sizeof(commandBuff));
			sprintf(commandBuff,"qos-ip6tables-uci -A class_chain -t mangle %s%s%s%s%s%s-o outint %s-m comment --comment %s%s"
				" -j QOSCONNMARK --set-mark 0x80%s00", "-p udp ", sport, dport, sip, dip,
				 final_rcv_iface, dscp, class_name, flow_name, mark_dscp);
			tokenize_and_docommand(commandBuff, "mangle", handle);
		}

		if( wifiboard && strlen(final_rcvwifi_iface))
		{
			memset(commandBuff, '\0', sizeof(commandBuff));
			sprintf(commandBuff,"qos-ip6tables-uci -A class_chain -t mangle %s%s%s%s%s%s-o outint %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark 0x80%s00", protocol, sport, dport, sip, dip,
					 final_rcvwifi_iface, dscp, class_name, flow_name, mark_dscp);
			tokenize_and_docommand(commandBuff, "mangle", handle);
			if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
			{
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A class_chain -t mangle %s%s%s%s%s%s-o outint %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark 0x80%s00", "-p udp ", sport, dport, sip, dip,
					 final_rcvwifi_iface, dscp, class_name, flow_name, mark_dscp);
				tokenize_and_docommand(commandBuff, "mangle", handle);
			}
		}


		//Adding egress_flow in egress_chain
	
		memset(tuple, '\0', sizeof(tuple));
		strcpy(tuple,"qos.scheduler.upstream");
		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("Error in getting info about qos.scheduler.upstream\r\n");
		}

		if(strcmp("rate-control",ptr.o->v.string)==0)
			n=8;
		else
			n=4;

		memset(tuple, '\0', sizeof(tuple));
		strcpy(tuple,"qos.outbound_policy.wan1");
		if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
			printf("Error in getting info about qos.outbound_policy.wan1\r\n");
		}
		strcpy(policy1,ptr.o->v.string);

		if (strcmp(this_board,"RV34") == 0) //Only RV34 series have wan2
		{
			memset(tuple, '\0', sizeof(tuple));
			strcpy(tuple,"qos.outbound_policy.wan2");
			if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
				printf("Error in getting info about qos.outbound_policy.wan2\r\n");
				memset(policy2,'\0', sizeof(policy2));
			}
			else //SUCCESS case.
				strcpy(policy2,ptr.o->v.string);
		}

		if(!(strcmp(this_board,"RV16") == 0)) //RV26 or RV34 
		{
			memset(tuple, '\0', sizeof(tuple));
			strcpy(tuple,"qos.outbound_policy.usb1");
			if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
				printf("Error in getting info about qos.outbound_policy.usb1\r\n");
				memset(policy3,'\0', sizeof(policy3));
			}
			else //SUCCESS case
				strcpy(policy3,ptr.o->v.string);
		}

		if (strcmp(this_board,"RV34") == 0) //Only RV34 series have USB2
		{
			memset(tuple, '\0', sizeof(tuple));
			strcpy(tuple,"qos.outbound_policy.usb2");
			if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
				printf("Error in getting info about qos.outbound_policy.usb2\r\n");
				memset(policy4,'\0', sizeof(policy4));
			}
			else //SUCCESS case.
				strcpy(policy4,ptr.o->v.string);
		}

		if ((strcmp(policy1,"Rate_Control_Default")!=0)
			&& (strcmp(policy1,"Low_Latency_Default")!=0)
			&& (strcmp(policy1,"Priority_Default")!=0))
		{//This policy is of WAN1 interface.
			char queue_value[128] = "";

			//Find out all the queue nos. for which the given class is configured.
			get_queues_configured(&queueconf,policy1,class_name,ctx);

			//NOTE: Hanlding only ipv4 part. Since v6 will be dealt in seperate file.
			if(queueconf.queue1)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-1)); //Since we are in queue1

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 		final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}

			if(queueconf.queue2)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-2)); //Since we are in queue2

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}

			if(queueconf.queue3)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-3)); //Since we are in queue3

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}

			if(queueconf.queue4)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-4)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}

			if(queueconf.queue5)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-5)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}

			if(queueconf.queue6)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-6)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}

			if(queueconf.queue7)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80A000000000%s0%d",mark_dscp,(n-7)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth2+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth2+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan16+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan16+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan1_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan16+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan16+ ");
				}
			}
		}//End of policy1 handling

		memset(&queueconf, 0, sizeof(struct queue_config));
		if ((strlen(policy2) > 0)
			&& (strcmp(policy2,"Rate_Control_Default")!=0)
			&& (strcmp(policy2,"Low_Latency_Default")!=0)
			&& (strcmp(policy2,"Priority_Default")!=0))
		{//This policy is of WAN2 interface.
			char queue_value[128] = "";

			//Find out all the queue nos. for which the given class is configured.
			get_queues_configured(&queueconf,policy2,class_name,ctx);

			//NOTE: Hanlding only ipv4 part. Since v6 will be dealt in seperate file.
			if(queueconf.queue1)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-1)); //Since we are in queue1

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}

			if(queueconf.queue2)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-2)); //Since we are in queue2

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}

			if(queueconf.queue3)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-3)); //Since we are in queue3

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}

			if(queueconf.queue4)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-4)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}

			if(queueconf.queue5)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-5)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}

			if(queueconf.queue6)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-6)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}

			if(queueconf.queue7)
			{
				memset(queue_value, '\0', sizeof(queue_value));
				sprintf(queue_value,"0x80B000000000%s0%d",mark_dscp,(n-7)); //Since we are in queue4

				//EGRESS_IPTABLE_RULE(final_rcv_iface,"-o eth0+ ");
				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
				tokenize_and_docommand(commandBuff, "mangle", handle);
				if(wifiboard && strlen(final_rcvwifi_iface))
				{
					//EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o eth0+ ");
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", protocol, sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
						" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff","-p udp ", sport, dport, sip, dip,
					 	final_rcvwifi_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					}
				}

				EGRESS_IPTABLE_RULE(final_rcv_iface,"-o ppoe-wan26+ ");
				if(wifiboard && strlen(final_rcvwifi_iface))
					EGRESS_IPTABLE_RULE(final_rcvwifi_iface,"-o ppoe-wan26+ ");
				if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
				{
					memset(commandBuff, '\0', sizeof(commandBuff));
					sprintf(commandBuff,"qos-ip6tables-uci -A egress_chain -t mangle %s%s%s%s%s%s-o %s+ %s-m comment --comment %s%s"
					" -j QOSCONNMARK --set-mark %s/0xf0f0000000ffffff", "-p udp ", sport, dport, sip, dip,
				 	final_rcv_iface, gWan2_phy_iface, dscp, class_name, flow_name, queue_value);
					tokenize_and_docommand(commandBuff, "mangle", handle);
					EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface,"-o ppoe-wan26+ ");
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface,"-o ppoe-wan26+ ");
				}
			}
		}//End of policy2 handling

		memset(&queueconf, 0, sizeof(struct queue_config));
		if ((strcmp(policy3,"Rate_Control_Default")!=0)
			&& (strcmp(policy3,"Low_Latency_Default")!=0)
			&& (strcmp(policy3,"Priority_Default")!=0))
		{//This policy is of USB1 interface.
			char queue_value[128] = "";

			findIface = search_iface("usb1");
			if(findIface && strcmp(findIface->l3ifname,"null")!=0)
			{
				char finalL3ifname[32]="";
				sprintf(finalL3ifname,"-o %s ",findIface->l3ifname);
				//Find out all the queue nos. for which the given class is configured.
				get_queues_configured(&queueconf,policy3,class_name,ctx);

				//NOTE: Hanlding only ipv4 part. Since v6 will be dealt in seperate file.
				if(queueconf.queue1)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000081%s00",mark_dscp); //Since we are in queue1

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue2)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000082%s00",mark_dscp); //Since we are in queue2

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue3)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000083%s00",mark_dscp); //Since we are in queue3

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue4)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000084%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue5)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000085%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue6)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000086%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue7)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80C000000087%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}
			} //End of active iface handling
		}//End of policy3 handling

		memset(&queueconf, 0, sizeof(struct queue_config));
		if ((strlen(policy4) > 0)
			&& (strcmp(policy4,"Rate_Control_Default")!=0)
			&& (strcmp(policy4,"Low_Latency_Default")!=0)
			&& (strcmp(policy4,"Priority_Default")!=0))
		{//This policy is of USB2 interface.
			char queue_value[128] = "";

			findIface = search_iface("usb2");
			if(findIface && strcmp(findIface->l3ifname,"null")!=0)
			{
				char finalL3ifname[32]="";
				sprintf(finalL3ifname,"-o %s ",findIface->l3ifname);
				//Find out all the queue nos. for which the given class is configured.
				get_queues_configured(&queueconf,policy4,class_name,ctx);

				//NOTE: Hanlding only ipv4 part. Since v6 will be dealt in seperate file.
				if(queueconf.queue1)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000091%s00",mark_dscp); //Since we are in queue1

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue2)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000092%s00",mark_dscp); //Since we are in queue2

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue3)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000093%s00",mark_dscp); //Since we are in queue3

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue4)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000094%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue5)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000095%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue6)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000096%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}

				if(queueconf.queue7)
				{
					memset(queue_value, '\0', sizeof(queue_value));
					sprintf(queue_value,"0x80D000000097%s00",mark_dscp); //Since we are in queue4

					EGRESS_IPTABLE_RULE(final_rcv_iface, finalL3ifname);
					if(wifiboard && strlen(final_rcvwifi_iface))
						EGRESS_IPTABLE_RULE(final_rcvwifi_iface, finalL3ifname);
					if(strcmp(FLOW_PROTO_TCP_UDP_KEYWORD,flow.proto)==0)
					{
						EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcv_iface, finalL3ifname);
						if(wifiboard && strlen(final_rcvwifi_iface))
							EGRESS_IPTABLE_RULE_WITH_TCP_UDP("-p udp ",final_rcvwifi_iface, finalL3ifname);
					}
				}
			} //End of handling active iface
		}//End of policy4 handling
	}
	else
		printf("This should not occur. A flow can be an ingress/egress. Flow:%s classname:%s\r\n", flow_name, class_name);

	return 0;
}

int main(int argc, char **argv)
{
	struct uci_context *ctx = NULL;
	struct ip6tc_handle *handle = NULL;
	//struct uci_package *pkg = NULL;
	struct uci_ptr ptr;
	char temp[]="systeminfo.sysinfo.pid";
	int i,c,y;

	//0. Process command line arguments
	while((c = getopt(argc, argv, "df:")) != -1) {
		switch(c) {
			case 'd':
				enable_debug = 1;
				break;
			case 'f':
				IfaceStatsId = atoi(optarg);
				break;
			default:
				print_usage();
				break;
		}
	}

	if (IfaceStatsId <= 0)
	{
		printf("Iface Stats ID must be given and is a valid ID.");
		print_usage();
		return 1;
	}

	//1. Init iptables related infra
	ip6tables_globals.program_name = "qos-ip6tables-uci";
	c = xtables_init_all(&ip6tables_globals, NFPROTO_IPV6);
	if (c<0)
	{
		printf("Failed to initialize xtables.%s\r\n",ip6tc_strerror(errno));
		exit(1);
	}
	init_extensions();
	init_extensions6();

	//2. Create UCI context and initialize all other things.
	ctx = uci_alloc_context();

	/* Figure out the board on which we are working. */
	if (uci_lookup_ptr(ctx, &ptr, temp, true) != UCI_OK) {
		printf("Error in getting info about DUT. Default to RV340\r\n");
	}

	if(strncmp("RV340W",ptr.o->v.string,6)==0
		|| strncmp("RV160W",ptr.o->v.string,6)==0
		|| strncmp("RV260W",ptr.o->v.string,6)==0)
		wifiboard=1;

	strncpy(this_board, ptr.o->v.string, 4); //Only RV34 or RV16 or RV26
	this_board[4]='\0';

	//3. Create handle
	handle = create6_handle("mangle");

	//4.Do the given command
	INIT_LIST_HEAD(&allIfaces);
	readIfaceFile();

	if(signal(SIGSEGV, sig_handler) == SIG_ERR)
		printf("Install of signal handler for SIGSEGV failed.\r\n");

	i = optind;
	while(i < argc)
	{
		char *class_flow[2];
		char *currentData = strdup(argv[i]);
		char *p = strtok(currentData,"."); 

		class_flow[0] = p;
		p = strtok(NULL,".");
		class_flow[1] = p;

		if(enable_debug)
			printf("Received Data: class:%s flow:%s\n", class_flow[0], class_flow[1]);

		i++;
		add_rule_to_chain(class_flow[0], class_flow[1], ctx, handle);
	}

	//5.Commit once all the rules are configured.
	y = ip6tc_commit(handle);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = ip6tc_commit(handle);
			if (!y)
				printf("Error commiting data: %s errno:%d after retry.\n",
					ip6tc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d.\n",
				ip6tc_strerror(errno),errno);
	}

	ip6tc_free(handle);
	uci_free_context(ctx);

	return 0;
}
