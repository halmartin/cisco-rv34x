/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

struct list_head aclRules;
struct list_head allIfaces;
struct list_head ifaceNATs;
struct list_head portForwards;
struct list_head portTriggerRules;
struct list_head dmzhosts;

struct list_head allowurlRules;
struct list_head allowkeywordRules;
struct list_head blockurlRules;
struct list_head blockkeywordRules;

/* global new argv and argc */
char *mynewargv[255];
int mynewargc;
static char * tablename="filter";
static char * nat_tablename="nat";
int enable_debug=0;
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

void print_argv(void)
{
    int a=0;

    printf("Details of argc and argv passed to do_command are:\r\n");
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

	if(foundAt>6)
	{ /*This is under an assumption that the nodes that are requested for be at near vicinity from head node.
		But why 6: Assuming wan1, wan16, wan2, wan26, vlan1 and probably a new vlan2. So total all including 6 */	
		list_del(found);
		list_add(found,&allIfaces);
	}

	if (found)
		return list_entry (found, struct iface_data, list);

	printf("found is NULL here. But this should not occur for interface:%s\r\n",iface_name);
	return NULL; //It will be NULL here. But this should not occur at all.
}

void print_allIfaces()
{
	struct list_head *p;
	struct iface_data *data;

	printf("Here are the elements after reorganisation:\r\n");
	list_for_each(p, &allIfaces) {
		data = list_entry (p, struct iface_data, list);

		printf("iface:%s status:%d ipaddr:%s ipsubnet:%d l2ifname:%s l3ifname:%s \r\n",data->ifname,
				data->status, data->ipaddr, data->subnet, data->l2ifname, data->l3ifname);
	}
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
        if(enable_debug)
        {
		    printf("iface:%s status:%d ip4addr:%s ip4subnet:%d ip6addr:%s ip6mask:%d l2ifname:%s l3ifname:%s\r\n",
						ifname,status,ipaddr,subnet,ip6addr,ip6mask,l2ifname,l3ifname);
        }

		strcpy(newIface->ifname,ifname);
		newIface->status=status;
		strcpy(newIface->l2ifname,l2ifname);
		strcpy(newIface->l3ifname,l3ifname);
		if(strncmp(newIface->ifname,"wan16",5)==0 || strncmp(newIface->ifname,"wan26",5)==0 || 
                strncmp(newIface->l3ifname,"6in4",4)==0 || strncmp(newIface->l3ifname,"6rd",3)==0 ) //V6 type
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

	fclose(fp);
}

//Utility function to tokenize and add the string to the given handle with do_command function
void tokenize_and_docommand(char *cmdBuff,char *table, struct iptc_handle *handle)
{
	char *token;
	int ret=0;
	char tempBuff[2048]="";

	strcpy(tempBuff,cmdBuff);

	/* get the first token */
	token = strtok(tempBuff, " ");

	mynewargc = 0; //Just to double ensure

	/* walk through other tokens */
	while(token != NULL)
	{
		add_argv(token);
		token = strtok(NULL, " ");
	}

	if(enable_debug)
	{
		printf("Received string for tokenize is:\n%s\n",cmdBuff);
	    //This atleast prints the args
        print_argv();
	}
		

	ret = do_command(mynewargc, mynewargv, &table, &handle);
	if (!ret)
		printf("do_command failed inside tokenize_and_docommand.string:%s\n",iptc_strerror(errno));

	free_argv();
	mynewargc = 0;
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


struct iptc_handle *create_handle(const char *fortablename)
{
	struct iptc_handle *newhandle;

	newhandle = iptc_init(fortablename);

	if (!newhandle) {
		/* try to insmod the module if iptc_init failed */
		printf("Trying to insmode the details\r\n");
		xtables_load_ko(xtables_modprobe_program, false);
		newhandle = iptc_init(fortablename);
	}

	if (!newhandle) {
		xtables_error(PARAMETER_PROBLEM, "unable to initialize "
			"table '%s'\n", fortablename);
		exit(1);
	}

	return newhandle;
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
firewall_allocate_acl(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_aclRule *p = malloc(sizeof(struct uci_aclRule));
	memset(p, 0, sizeof(struct uci_aclRule));
	return &p->map;
}

//For IFACE NAT records.
static int
firewall_init_ifaceNAT(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_ifaceNAT *ifNAT = section; 

	INIT_LIST_HEAD(&ifNAT->list);
	ifNAT->name = s->e.name;
	ifNAT->test = -1;
	return 0;
}

static int
firewall_add_ifaceNAT(struct uci_map *map, void *section)
{
	struct uci_ifaceNAT *ifNAT = section; 

	list_add_tail(&ifNAT->list, &ifaceNATs);

	return 0;
}

static struct ucimap_section_data *
firewall_allocate_ifaceNAT(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_ifaceNAT *p = malloc(sizeof(struct uci_ifaceNAT));
	memset(p, 0, sizeof(struct uci_ifaceNAT));
	return &p->map;
}

//For Port Forward rules.
static int
firewall_init_portForward(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_portForward *pf = section; 

	INIT_LIST_HEAD(&pf->list);
	pf->name = s->e.name;
	pf->test = -1;
	return 0;
}

static int
firewall_add_portForward(struct uci_map *map, void *section)
{
	struct uci_portForward *pf = section; 

	list_add_tail(&pf->list, &portForwards);

	return 0;
}

static struct ucimap_section_data *
firewall_allocate_portForward(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_portForward *p = malloc(sizeof(struct uci_portForward));
	memset(p, 0, sizeof(struct uci_portForward));
	return &p->map;
}

//For Port trigger rules.
static int
firewall_init_portTrigger(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_portTriggerRule *pt = section; 

	INIT_LIST_HEAD(&pt->list);
	pt->name = s->e.name;
	pt->test = -1;
	return 0;
}

static int
firewall_add_portTrigger(struct uci_map *map, void *section)
{
	struct uci_portTriggerRule *pt = section; 

	list_add_tail(&pt->list, &portTriggerRules);

	return 0;
}

static struct ucimap_section_data *
firewall_allocate_portTrigger(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_portTriggerRule *p = malloc(sizeof(struct uci_portTriggerRule));
	memset(p, 0, sizeof(struct uci_portTriggerRule));
	return &p->map;
}

//For Block URL
static int
firewall_blockurl_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_blockurlRule *blockurl = section;

	INIT_LIST_HEAD(&blockurl->list);
	blockurl->name = s->e.name;
	blockurl->test = -1;
	return 0;
}

static int
firewall_blockurl_add_rule(struct uci_map *map, void *section)
{
	struct uci_blockurlRule *blockurl = section;

	list_add_tail(&blockurl->list, &blockurlRules);

	return 0;
}

static struct ucimap_section_data *
firewall_blockurl_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_blockurlRule *p = malloc(sizeof(struct uci_blockurlRule));
	memset(p, 0, sizeof(struct uci_blockurlRule));
	return &p->map;
}

//For Block Keyword
static int
firewall_blockkeyword_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_blockkeywordRule *blockkeyword = section;

	INIT_LIST_HEAD(&blockkeyword->list);
	blockkeyword->name = s->e.name;
	blockkeyword->test = -1;
	return 0;
}

static int
firewall_blockkeyword_add_rule(struct uci_map *map, void *section)
{
	struct uci_blockkeywordRule *blockkeyword = section;

	list_add_tail(&blockkeyword->list, &blockkeywordRules);

	return 0;
}

static struct ucimap_section_data *
firewall_blockkeyword_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_blockkeywordRule *p = malloc(sizeof(struct uci_blockkeywordRule));
	memset(p, 0, sizeof(struct uci_blockkeywordRule));
	return &p->map;
}

//For allow keyword
static int
firewall_allowkeyword_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_allowkeywordRule *allowkeyword = section;

	INIT_LIST_HEAD(&allowkeyword->list);
	allowkeyword->name = s->e.name;
	allowkeyword->test = -1;
	return 0;
}

static int
firewall_allowkeyword_add_rule(struct uci_map *map, void *section)
{
	struct uci_allowkeywordRule *allowkeyword = section;

	list_add_tail(&allowkeyword->list, &allowkeywordRules);

	return 0;
}

static struct ucimap_section_data *
firewall_allowkeyword_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_allowkeywordRule *p = malloc(sizeof(struct uci_allowkeywordRule));
	memset(p, 0, sizeof(struct uci_allowkeywordRule));
	return &p->map;
}

//For allow URL
static int
firewall_allowurl_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_allowurlRule *allowurl = section;

	INIT_LIST_HEAD(&allowurl->list);
	allowurl->name = s->e.name;
	allowurl->test = -1;
	return 0;
}

static int
firewall_allowurl_add_rule(struct uci_map *map, void *section)
{
	struct uci_allowurlRule *allowurl = section;

	list_add_tail(&allowurl->list, &allowurlRules);

	return 0;
}

static struct ucimap_section_data *
firewall_allowurl_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_allowurlRule *p = malloc(sizeof(struct uci_allowurlRule));
	memset(p, 0, sizeof(struct uci_allowurlRule));
	return &p->map;
}

//For DMZ 
static int
firewall_dmzhost_init(struct uci_map *map, void *section, struct uci_section *s)
{
	struct uci_dmzhost *dh = section;

	INIT_LIST_HEAD(&dh->list);
	dh->name = s->e.name;
	dh->test = -1;
	return 0;
}

static int
firewall_dmzhost_add(struct uci_map *map, void *section)
{
	struct uci_dmzhost *dh = section;

	list_add_tail(&dh->list, &dmzhosts);

	return 0;
}

static struct ucimap_section_data *
firewall_dmzhost_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
	struct uci_dmzhost *p = malloc(sizeof(struct uci_dmzhost));
	memset(p, 0, sizeof(struct uci_dmzhost));
	return &p->map;
}


struct my_optmap {
	struct uci_optmap map;
	int test;
};

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
			UCIMAP_OPTION(struct uci_aclRule, src_subnet),
			.type = UCIMAP_INT,
			.name = "src_subnet"
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
			UCIMAP_OPTION(struct uci_aclRule, dest_subnet),
			.type = UCIMAP_INT,
			.name = "dest_subnet"
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

static struct my_optmap firewall_ifaceNAT_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_ifaceNAT, enable),
			.type = UCIMAP_BOOL,
			.name = "enable",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_ifaceNAT, interfacename),
			.type = UCIMAP_STRING,
			.name = "interfacename",
			.data.s.maxlen = 32,
		}
	}
};

static struct my_optmap firewall_dmzhost_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_dmzhost, status),
			.type = UCIMAP_BOOL,
			.name = "status",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_dmzhost, ipaddr),
			.type = UCIMAP_STRING,
			.name = "ipaddr",
			.data.s.maxlen = 32,
		}
	}
};

static struct my_optmap firewall_portForward_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, status),
			.type = UCIMAP_BOOL,
			.name = "status",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, protocol),
			.type = UCIMAP_STRING,
			.name = "protocol",
			.data.s.maxlen = 32,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, ext_port_start),
			.type = UCIMAP_INT,
			.name = "ext_port_start"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, ext_port_end),
			.type = UCIMAP_INT,
			.name = "ext_port_end",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, int_port_start),
			.type = UCIMAP_INT,
			.name = "int_port_start",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, int_port_end),
			.type = UCIMAP_INT,
			.name = "int_port_end",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, interface_name),
			.type = UCIMAP_STRING,
			.name = "interface_name",
			.data.s.maxlen = 32,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portForward, dest_ip),
			.type = UCIMAP_STRING,
			.name = "dest_ip",
			.data.s.maxlen = 32,
		}
	}
};

static struct my_optmap firewall_portTrigger_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, status),
			.type = UCIMAP_BOOL,
			.name = "status",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, interface),
			.type = UCIMAP_STRING,
			.name = "interface",
			.data.s.maxlen = 32,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, protocol),
			.type = UCIMAP_STRING,
			.name = "protocol",
			.data.s.maxlen = 32,
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, trigger_port_start),
			.type = UCIMAP_INT,
			.name = "trigger_port_start"
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, trigger_port_end),
			.type = UCIMAP_INT,
			.name = "trigger_port_end",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, int_port_start),
			.type = UCIMAP_INT,
			.name = "int_port_start",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, int_port_end),
			.type = UCIMAP_INT,
			.name = "int_port_end",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_portTriggerRule, int_protocol),
			.type = UCIMAP_STRING,
			.name = "int_protocol",
			.data.s.maxlen = 32,
		}
	}
};

static struct my_optmap firewall_blockurl_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_blockurlRule, domain_name),
			.type = UCIMAP_STRING,
			.name = "domain_name",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_blockurlRule, schedule),
			.type = UCIMAP_STRING,
			.name = "schedule",
		}
	}
};

static struct my_optmap firewall_blockkeyword_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_blockkeywordRule, keyword),
			.type = UCIMAP_STRING,
			.name = "keyword",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_blockkeywordRule, schedule),
			.type = UCIMAP_STRING,
			.name = "schedule",
		}
	}
};

static struct my_optmap firewall_allowurl_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_allowurlRule, domain_name),
			.type = UCIMAP_STRING,
			.name = "domain_name",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_allowurlRule, schedule),
			.type = UCIMAP_STRING,
			.name = "schedule",
		}
	}
};

static struct my_optmap firewall_allowkeyword_options[] = {
	{
		.map = {
			UCIMAP_OPTION(struct uci_allowkeywordRule, keyword),
			.type = UCIMAP_STRING,
			.name = "keyword",
		}
	},
	{
		.map = {
			UCIMAP_OPTION(struct uci_allowkeywordRule, schedule),
			.type = UCIMAP_STRING,
			.name = "schedule",
		}
	}
};

static struct uci_sectionmap firewall_aclRules = {
	UCIMAP_SECTION(struct uci_aclRule, map),
	.type = "rule",
	.alloc = firewall_allocate_acl,
	.init = firewall_init_rule,
	.add = firewall_add_rule,
	.options = &firewall_aclRule_options[0].map,
	.n_options = ARRAY_SIZE(firewall_aclRule_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_dmzhost_rules = {
	UCIMAP_SECTION(struct uci_dmzhost, map),
	.type = "host",
	.alloc = firewall_dmzhost_allocate,
	.init = firewall_dmzhost_init,
	.add = firewall_dmzhost_add,
	.options = &firewall_dmzhost_options[0].map,
	.n_options = ARRAY_SIZE(firewall_dmzhost_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_ifaceNAT_rules = {
	UCIMAP_SECTION(struct uci_ifaceNAT, map),
	.type = "natmode",
	.alloc = firewall_allocate_ifaceNAT,
	.init = firewall_init_ifaceNAT,
	.add = firewall_add_ifaceNAT,
	.options = &firewall_ifaceNAT_options[0].map,
	.n_options = ARRAY_SIZE(firewall_ifaceNAT_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_portForward_rules = {
	UCIMAP_SECTION(struct uci_portForward, map),
	.type = "redirect",
	.alloc = firewall_allocate_portForward,
	.init = firewall_init_portForward,
	.add = firewall_add_portForward,
	.options = &firewall_portForward_options[0].map,
	.n_options = ARRAY_SIZE(firewall_portForward_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_portTrigger_rules = {
	UCIMAP_SECTION(struct uci_portTriggerRule, map),
	.type = "port_trigger",
	.alloc = firewall_allocate_portTrigger,
	.init = firewall_init_portTrigger,
	.add = firewall_add_portTrigger,
	.options = &firewall_portTrigger_options[0].map,
	.n_options = ARRAY_SIZE(firewall_portTrigger_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_blockurlRules = {
	UCIMAP_SECTION(struct uci_blockurlRule, map),
	.type = "blockURL",
	.alloc = firewall_blockurl_allocate,
	.init = firewall_blockurl_init_rule,
	.add = firewall_blockurl_add_rule,
	.options = &firewall_blockurl_options[0].map,
	.n_options = ARRAY_SIZE(firewall_blockurl_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_blockkeywordRules = {
	UCIMAP_SECTION(struct uci_blockkeywordRule, map),
	.type = "blockKeyword",
	.alloc = firewall_blockkeyword_allocate,
	.init = firewall_blockkeyword_init_rule,
	.add = firewall_blockkeyword_add_rule,
	.options = &firewall_blockkeyword_options[0].map,
	.n_options = ARRAY_SIZE(firewall_blockkeyword_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_allowurlRules = {
	UCIMAP_SECTION(struct uci_allowurlRule, map),
	.type = "allowURL",
	.alloc = firewall_allowurl_allocate,
	.init = firewall_allowurl_init_rule,
	.add = firewall_allowurl_add_rule,
	.options = &firewall_allowurl_options[0].map,
	.n_options = ARRAY_SIZE(firewall_allowurl_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap firewall_allowkeywordRules = {
	UCIMAP_SECTION(struct uci_allowkeywordRule, map),
	.type = "allowKeyword",
	.alloc = firewall_allowkeyword_allocate,
	.init = firewall_allowkeyword_init_rule,
	.add = firewall_allowkeyword_add_rule,
	.options = &firewall_allowkeyword_options[0].map,
	.n_options = ARRAY_SIZE(firewall_allowkeyword_options),
	.options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap *firewall_smap[] = {
	&firewall_aclRules,
	&firewall_ifaceNAT_rules,
	&firewall_portForward_rules,
	&firewall_portTrigger_rules,
	//&firewall_staticNAT_rules,
	&firewall_blockurlRules,
	&firewall_blockkeywordRules,
	&firewall_allowurlRules,
	&firewall_allowkeywordRules,
	&firewall_dmzhost_rules,
};

static struct uci_map firewall_map = {
	.sections = firewall_smap,
	.n_sections = ARRAY_SIZE(firewall_smap),
};

void print_usage()
{
	fprintf(stderr, "USGAE: iptables-uci [ <List of modules to be reloaded> ] fileID:<Iface-File-ID>\n"
			"	Iface-File-ID: This file ID is PID number of process who invoke this application.\n"
			"			This ID is also the file extension of Iface stats file.\n"
			"	List of modules to reload can be:\n"
			"		acl allowurl allowkeyword\n"
			"		blockurl blockkeyword basicsettings\n"
			"		contentfilter debug dmz ifacenat\n"
			"		portforward porttrigger\n"
			"		staticnat spoofrules trusteddomains\n"
			"		webfeatures policynat\n"
			"	Example: iptables-uci debug acl staticnat porttrigger fileID:4399\r\n"
		);
	exit(1);
}

int insert_spoof_rules(struct uci_context *ctx, struct iptc_handle *handle)
{
	struct uci_ptr ptr;
	char tuple[64]="firewall.global_configuration.firewall";
	struct iface_data *lanIface, *wanIface;
	struct list_head *p_lan, *p_wan;
	char commandBuff[2048]="";

	if (uci_lookup_ptr(ctx, &ptr, tuple, true) != UCI_OK) {
		printf("Error in getting info about firewall global configuration with string:%s\r\n",tuple);
		//cli_perror();
		return 1;
	}

	if(atoi(ptr.o->v.string)!= 0)
		return 1;//Which means firewall is enabled 

	if (ptr.p)
		uci_unload(ctx, ptr.p);

	list_for_each(p_lan, &allIfaces) {
		lanIface = list_entry (p_lan, struct iface_data, list);
		if(strncmp(lanIface->ifname,"vlan",4)!=0)
			continue;

		list_for_each(p_wan, &allIfaces) {
			wanIface = list_entry (p_wan, struct iface_data, list);
            if((!wanIface) || (strcmp(wanIface->l3ifname,"null")==0))
            {
                printf("Spoof rules:l3ifname is NULL/null\n");
                continue;
            }

			if(wanIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
				&& (strncmp(wanIface->ifname,"wan",3)==0 || strncmp(wanIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
				&& (strncmp(wanIface->ifname,"wan16",5)!=0 
					&& strncmp(wanIface->ifname,"wan26",5)!=0 && strncmp(wanIface->l3ifname,"6in4",4)!=0
                                        && strncmp(wanIface->l3ifname,"6rd",3)!=0) /*A v4 interface*/
				) //A Valid active WAN interface
			{
				sprintf(commandBuff,"iptables-uci -t filter -I firewall_disable -i %s -s %s/%d -j DROP",
							wanIface->l3ifname, lanIface->ipaddr, lanIface->subnet);
				tokenize_and_docommand(commandBuff, "filter", handle);

				memset(commandBuff, '\0', sizeof(commandBuff));
				sprintf(commandBuff,"iptables-uci -t filter -I firewall_disable -i %s -s 224.0.0.0/4 -j DROP",
							wanIface->l3ifname);
				tokenize_and_docommand(commandBuff, "filter", handle);
			}
			else
				continue; // NOT an active WAN interface.
		}
	}

	memset(commandBuff, '\0', sizeof(commandBuff));
	sprintf(commandBuff,"iptables-uci -t filter -A firewall_disable -j ACCEPT");
	tokenize_and_docommand(commandBuff, "filter", handle);
	return 0;
}

void config_spoof_rules(struct uci_context *ctx)
{
	struct iptc_handle *handle = NULL;
	int y=0;

	handle = create_handle("filter");

	if(iptc_is_chain(strdup("firewall_disable"), handle))
	{
		if(!iptc_flush_entries(strdup("firewall_disable"),handle))
			printf("Failed in flushing rules of firewall_disable in iptables\r\n");
	}
	else
		printf("No chain detected with that name for IPv4\r\n");

	insert_spoof_rules(ctx, handle);

	y = iptc_commit(handle);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of spoof rules addition, even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data: %s errno:%d in the context of spoof rules addition.\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	iptc_free(handle);
}

int main(int argc, char **argv)
{
	struct uci_package *pkg;
	struct uci_context *ctx;
	struct modules config_reload={0,0,0,0,0,0,0,0,0,0,0};
	//int i,a=0;
	int c=0;
	int argcount=1;

	if (argc > 1)
	while (argcount < argc)
	{
		//printf("Parsing arg is: %s\n",argv[argcount]);

		if (strcmp(argv[argcount],"acl")==0)
			config_reload.access_rules=1;
		else if (strcmp(argv[argcount],"basicsettings")==0)
			config_reload.basic_settings=1;
		else if (strcmp(argv[argcount],"webfeatures")==0)
			config_reload.allWebContentFilter=1;
		else if (strcmp(argv[argcount],"contentfilter")==0)
			config_reload.allWebContentFilter=1;
		else if (strcmp(argv[argcount],"blockurl")==0)
			config_reload.allWebContentFilter=1;
		else if (strcmp(argv[argcount],"blockkeyword")==0)
			config_reload.allWebContentFilter=1;
		else if (strcmp(argv[argcount],"staticnat")==0)
			config_reload.static_nat=1;
		else if (strcmp(argv[argcount],"ifacenat")==0)
			config_reload.iface_nat=1;
		else if (strcmp(argv[argcount],"portforward")==0)
			config_reload.port_forwarding=1;
		else if (strcmp(argv[argcount],"porttrigger")==0)
			config_reload.port_triggering=1;
		else if (strcmp(argv[argcount],"dmz")==0)
			config_reload.dmz=1;
		else if (strcmp(argv[argcount],"spoofrules")==0)
			config_reload.spoof_rules=1;
		else if (strcmp(argv[argcount],"trusteddomains")==0)
			config_reload.allWebContentFilter=1;
		else if (strcmp(argv[argcount],"allowurl")==0)
			config_reload.allWebContentFilter=1;
		else if (strcmp(argv[argcount],"allowkeyword")==0)
			config_reload.allWebContentFilter=1;
        else if (strcmp(argv[argcount],"policynat")==0)
            config_reload.policynat=1;
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

	//1.Very first activity is to ready your IFACE data structure and initialise all other lists.
	INIT_LIST_HEAD(&aclRules);
	INIT_LIST_HEAD(&ifaceNATs);
	INIT_LIST_HEAD(&portForwards);
	//INIT_LIST_HEAD(&staticNATs);
	INIT_LIST_HEAD(&allIfaces);
	INIT_LIST_HEAD(&portTriggerRules);
	INIT_LIST_HEAD(&dmzhosts);

	INIT_LIST_HEAD(&blockurlRules);
	INIT_LIST_HEAD(&blockkeywordRules);
	INIT_LIST_HEAD(&allowurlRules);
	INIT_LIST_HEAD(&allowkeywordRules);

	readIfaceFile();

	//2. Ready your iptables infra with basic initialization.
	iptables_globals.program_name = "iptables-uci";
	c = xtables_init_all(&iptables_globals, NFPROTO_IPV4);
	if (c<0)
	{
		printf("Failed to initialize xtables.%s\r\n",iptc_strerror(errno));
		exit(1);
	}
	init_extensions();
	init_extensions4();

	//3. Create UCI context and initialize all other things.
	ctx = uci_alloc_context();
	ucimap_init(&firewall_map);

	if (uci_load(ctx, "firewall", &pkg))
	{
		//uci_perror(state->uci, NULL);
		fprintf(stderr, "Error: Can't load firewall config");
		return 0;
	}

	//4. Issue your parse command.
	ucimap_parse(&firewall_map, pkg); //Complete data is been parsed all the data is available in respective lists.

	//5. Call appropriate functions to load all the relavent data.
	if (config_reload.basic_settings == 1)
		config_basicsettings(ctx);

	if (config_reload.allWebContentFilter == 1)
		config_allWebContentFiltering(ctx);

	if (config_reload.access_rules == 1)
		config_aclrule(ctx);

	if (config_reload.static_nat == 1)
		config_staticnat();

	if (config_reload.iface_nat == 1)
		config_ifaceNAT(ctx);

	if (config_reload.port_forwarding == 1)
		config_portForwarding(ctx);

	if (config_reload.dmz == 1)
		config_dmzhost(ctx);

	if (config_reload.port_triggering == 1)
		config_portTriggering(ctx);

	if (config_reload.spoof_rules == 1)
		config_spoof_rules(ctx);

    if (config_reload.policynat == 1)
    {
          config_policynat();
    }
	ucimap_cleanup(&firewall_map);
	uci_free_context(ctx);
	return 0;
}
