/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>

struct list_head allIfaces;
struct list_head policynatRules;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;

static int
firewall_policynat_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
    struct uci_policynatRule *policynat = section;

    INIT_LIST_HEAD(&policynat->list);
    policynat->name = s->e.name;
    policynat->test = -1;
    return 0;
}

static int
firewall_policynat_add_rule(struct uci_map *map, void *section)
{
    struct uci_policynatRule *policynat = section;

    list_add_tail(&policynat->list, &policynatRules);

    return 0;
}

static struct ucimap_section_data *
firewall_policynat_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
    struct uci_policynatRule *p = malloc(sizeof(struct uci_policynatRule));
    memset(p, 0, sizeof(struct uci_policynatRule));
    return &p->map;
}

struct my_optmap {
    struct uci_optmap map;
    int test;
};


static struct my_optmap firewall_policynat_options[] = {

                     {
                        .map = {
                                    UCIMAP_OPTION(struct uci_policynatRule, status),
                                    .type = UCIMAP_BOOL,
                                    .name = "status",
                              }
                    },

                    {
                        .map = {
                                    UCIMAP_OPTION(struct uci_policynatRule, src_interface),
                                    .type = UCIMAP_STRING,
                                    .name = "src_interface",
                                }
                    },

                    {
                        .map = { 
                                    UCIMAP_OPTION(struct uci_policynatRule, dest_interface),
                                    .type = UCIMAP_STRING,
                                    .name = "dest_interface",
                                }   
                    }, 

                    {
                        .map = { 
                                    UCIMAP_OPTION(struct uci_policynatRule, orig_src_ip),
                                    .type = UCIMAP_STRING,
                                    .name = "orig_src_ip",
                                }   
                    }, 

                    {
                        .map = { 
                                    UCIMAP_OPTION(struct uci_policynatRule, trans_src_ip),
                                    .type = UCIMAP_STRING,
                                    .name = "trans_src_ip",
                                }   
                    },

                    {
                        .map = { 
                                    UCIMAP_OPTION(struct uci_policynatRule, orig_dest_ip),
                                    .type = UCIMAP_STRING,
                                    .name = "orig_dest_ip",
                                }   
                    }, 

                    {
                        .map = { 
                                UCIMAP_OPTION(struct uci_policynatRule, trans_dest_ip),
                                .type = UCIMAP_STRING,
                                .name = "trans_dest_ip",
                                }   
                    },

                    {
                        .map = { 
                                UCIMAP_OPTION(struct uci_policynatRule, protocol),
                                .type = UCIMAP_STRING,
                                .name = "protocol",
                             }   
                   },  

                    {
                        .map = { 
                                UCIMAP_OPTION(struct uci_policynatRule, orig_port_start),
                                .type = UCIMAP_INT,
                                .name = "orig_port_start",
                               }   
                    },  
                    {
                        .map = { 
                                 UCIMAP_OPTION(struct uci_policynatRule, orig_port_end),
                                 .type = UCIMAP_INT,
                                 .name = "orig_port_end",
                               }   
                   },  

                    {
                        .map = { 
                                 UCIMAP_OPTION(struct uci_policynatRule, trans_port_start),
                                 .type = UCIMAP_INT,
                                 .name = "trans_port_start",
                               }   
                    },  
                    {
                          .map = { 
                                   UCIMAP_OPTION(struct uci_policynatRule, trans_port_end),
                                   .type = UCIMAP_INT,
                                   .name = "trans_port_end",
                                 }   
                   },  



};

static struct uci_sectionmap firewall_policynatRules = {
    UCIMAP_SECTION(struct uci_policynatRule, map),
    .type = "firewall_policy_nat",
    .alloc = firewall_policynat_allocate,
    .init = firewall_policynat_init_rule,
    .add = firewall_policynat_add_rule,
    .options = &firewall_policynat_options[0].map,
    .n_options = ARRAY_SIZE(firewall_policynat_options),
    .options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap *firewall_policynat_smap[] = {
            &firewall_policynatRules,
};

static struct uci_map firewall_policynat_map = {
            .sections = firewall_policynat_smap,
            .n_sections = ARRAY_SIZE(firewall_policynat_smap),
};

void print_policynat(struct uci_policynatRule *pn)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");

	if(pn)
		printf("Complete firewall policynat record is '%s'\n"
			"	enabled:	%s\n"
			"	protocol:	%s\n"
			"	original Port start:%d\n"
            "   original Port end:%d\n"
			"	translated Port start:%d\n"
            "   translated Port end:%d\n"
			"	original source ip:%s\n"
			"	translated source ip:%s\n"
            "   original destination ip:%s"
            "   translated destination ip:%s"
			"	source interface:%s\n"
			"	destination interface: %s\n",
			pn->name,
			(pn->status ? "Yes" : "No"),
			pn->protocol,
			pn->orig_port_start,
            pn->orig_port_end,
			pn->trans_port_start,
            pn->trans_port_end,
			pn->orig_src_ip,
			pn->trans_src_ip,
            pn->orig_dest_ip,
            pn->trans_dest_ip,
			pn->src_interface,
			pn->dest_interface);
}

int get_ipaddress(char *ipgroup,char *ipaddr,int *type)
{

    char ipgrpstr[128]={'\0'};
    struct uci_ptr ptr;
    struct uci_context *grpctx;
    struct uci_element *e;
    int group_type=0;                              
    grpctx = uci_alloc_context ();

    sprintf(ipgrpstr,"ipgroup.%s.ipv4_addr",ipgroup);                                    
    
    if (uci_lookup_ptr(grpctx, &ptr,ipgrpstr, true) != UCI_OK) {
        printf("Error in getting info about:%s\r\n",ipgrpstr);
        return 1;
    }
   
   if(!(ptr.flags & UCI_LOOKUP_COMPLETE))
   {
        sprintf(ipgrpstr,"ipgroup.%s.ipv4_subnet",ipgroup);
        if (uci_lookup_ptr(grpctx, &ptr,ipgrpstr, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",ipgrpstr);
            return 1;
        }

        if(!(ptr.flags & UCI_LOOKUP_COMPLETE)) 
        {
            sprintf(ipgrpstr,"ipgroup.%s.ipv4_range",ipgroup);
            if (uci_lookup_ptr(grpctx, &ptr,ipgrpstr, true) != UCI_OK) {
                printf("Error in getting info about:%s\r\n",ipgrpstr);
                return 1;
            }  
           /*This indicates ipaddress group type is range*/ 
            group_type = 1;
       
        }
        else 
        {
            /*This indicates ipaddress group type is subnet*/
            group_type = 2;
        }

      if(!(ptr.flags & UCI_LOOKUP_COMPLETE))
      {
        return 1;
      }  
   }

    struct uci_option *o=ptr.o;
    switch(o->type) {
        case UCI_TYPE_LIST:
            uci_foreach_element(&o->v.list, e) {
                strcpy(ipaddr,e->name);
            }
        break;
        default:
            printf("<unknown>\n");
            break;
    }

    if(group_type == 1)
        *type=1;/*This indicates ipaddress group type is range*/
    else if(group_type == 2)
        *type=2;/*This indicates ipaddress group type is subnet*/

    uci_free_context (grpctx);
    return 0;
}

int get_range_from_subnet(char*ip,int prefix_length,char * start_ip,char* end_ip)
{

    struct in_addr in;
    unsigned long fullmask = 0xFFFFFFFF;
    unsigned  long start=0,end=0;

    inet_aton(ip,&in);
    start = (htonl(in.s_addr) & (fullmask << (32-prefix_length))) | 1;
    end = (ntohl(in.s_addr) & (fullmask << (32-prefix_length))) | (~((fullmask << (32-prefix_length)))-1);

    in.s_addr = htonl(start);
    strcpy(start_ip,inet_ntoa(in));
    in.s_addr = htonl(end);

    strcpy(end_ip,inet_ntoa(in));

    return 0;
}
int insert_policynat_forward(struct uci_policynatRule *pn,struct uci_context *ctx,struct iptc_handle *handle_filter)
{
    struct iface_data *findIface;
    char forward_rule[2048] = {'\0'}; 
    int need_udp_rule=0;
    char udp_rule[2048]="";

    if(((pn->trans_dest_ip) || (pn->trans_port_start)))
        sprintf(forward_rule,"iptables-uci -t nat -A policynat -j ACCEPT -m mark --mark 0xFFef");
    else
        sprintf(forward_rule,"iptables-uci -t nat -A policynat -j ACCEPT");
    
    if((pn->protocol) && (strcmp(pn->protocol,"all") != 0))
    {
        sprintf(forward_rule,"%s -p %s",forward_rule,pn->protocol);
        if(pn->trans_port_start)
        {
            if(pn->trans_port_start == pn->trans_port_end)
            {
                sprintf(forward_rule,"%s --dport %d",forward_rule,pn->trans_port_start);
            }
            else
            {
                sprintf(forward_rule,"%s --dport %d:%d",forward_rule,pn->trans_port_start,pn->trans_port_end);
            }
        }
        else if (pn->orig_port_start)
        {
            if(pn->orig_port_start == pn->orig_port_end)
            {
                sprintf(forward_rule,"%s --dport %d",forward_rule,pn->orig_port_start);
            }
            else
            {
                sprintf(forward_rule,"%s --dport %d:%d",forward_rule,pn->orig_port_start,pn->orig_port_end);
            }
        
        }
    }
    else if(pn->protocol && pn->trans_port_start && pn->trans_port_end)
    {
        need_udp_rule=1;
        sprintf(forward_rule,"%s --protocol tcp",forward_rule);
        if(pn->trans_port_start == pn->trans_port_end)
        {
            sprintf(forward_rule,"%s --dport %d",forward_rule,pn->trans_port_start);
        }
        else
        {
            sprintf(forward_rule,"%s --dport %d:%d",forward_rule,pn->trans_port_start,pn->trans_port_end);
        }


    }
    else if(pn->protocol && pn->orig_port_start && pn->orig_port_end)
    {
        need_udp_rule=1;
        sprintf(forward_rule,"%s --protocol tcp",forward_rule);
        if(pn->orig_port_start == pn->orig_port_end)
        {
            sprintf(forward_rule,"%s --dport %d",forward_rule,pn->orig_port_start);
        }
        else
        {
            sprintf(forward_rule,"%s --dport %d:%d",forward_rule,pn->orig_port_start,pn->orig_port_end);
        }
    
    }

   
    if((pn->orig_src_ip) && (strcmp(pn->orig_src_ip,"any")))
    {
            char orig_srcip[64]={'\0'};

            int iprange=0;

            get_ipaddress(pn->orig_src_ip,orig_srcip,&iprange);
            if(iprange == 1)
            {
                sprintf(forward_rule,"%s -m iprange --src-range %s",forward_rule,orig_srcip);
            }
            else
                sprintf(forward_rule,"%s -s %s",forward_rule,orig_srcip);

    }
    if((pn->trans_dest_ip) && (strcmp(pn->trans_dest_ip,"any")))
    {
            char trans_destip[64]={'\0'};
            int iprange=0;
            get_ipaddress(pn->trans_dest_ip,trans_destip,&iprange);
            if(iprange == 1)
                sprintf(forward_rule,"%s -m iprange --dst-range %s",forward_rule,trans_destip);
            else
                sprintf(forward_rule,"%s -d %s",forward_rule,trans_destip);
    }
    else if((pn->orig_dest_ip) && (strcmp(pn->orig_dest_ip,"any")))
    {
        char orig_destip[64]={'\0'};
        int iprange=0;

        get_ipaddress(pn->orig_dest_ip,orig_destip,&iprange);

        if(iprange == 1)
            sprintf(forward_rule,"%s -m iprange --dst-range %s",forward_rule,orig_destip);
        else
            sprintf(forward_rule,"%s -d %s",forward_rule,orig_destip);
    
    }

    if(strcmp(pn->src_interface,"any"))
    {
        findIface = search_iface(pn->src_interface);
        if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
        {
            printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
            return 1;
        }
        sprintf(forward_rule,"%s -i %s",forward_rule,findIface->l3ifname);

    }

    if(strcmp(pn->dest_interface,"any"))
    {
        findIface = search_iface(pn->dest_interface);
        if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
        {
            printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->dest_interface);
            return 1;
        }
        sprintf(forward_rule,"%s -o %s",forward_rule,findIface->l3ifname);

    }

    tokenize_and_docommand(forward_rule, "filter", handle_filter);

    if(need_udp_rule)
    {
        char udp_rule[2048]="";
        strreplace(udp_rule, forward_rule, "protocol tcp", "protocol udp");     
        tokenize_and_docommand(udp_rule, "filter", handle_filter);   
    }
    
    return 0;
}
int insert_policynat_dnat(struct uci_policynatRule *pn,struct uci_context *ctx,struct iptc_handle *handle_nat,char *orig_range_ip,char *trans_range_ip,char *subnet_startip,char *subnet_endip,int subnet_prefix)
{
    struct iface_data *findIface;
    char dnat_rule[2048] = {'\0'};
    char dnat_markrule[2048] = "iptables-uci -t nat -I policynat_dnat -j  MARK --set-mark 0xFFef";
    char loopback_rule[256]={'\0'};
    int need_udp_rule=0;

    /*if policynat orig source and trans destination prefix length is same and if translated destination port is zero
     * then use NETMAP otherwise use DNAT.NATMAP is not applicable for port translation*/
    if(subnet_prefix && ((pn->trans_port_start) == 0))
        sprintf(dnat_rule,"iptables-uci -t nat -A policynat_dnat -j NETMAP");
    else
        sprintf(dnat_rule,"iptables-uci -t nat -A policynat_dnat -j DNAT");

	if(!(pn->trans_dest_ip) && !(pn->trans_port_start))
    {
        return;
    }
  if((pn->protocol) && (strcmp(pn->protocol,"all") != 0))
    {
        
        sprintf(dnat_rule,"%s -p %s",dnat_rule,pn->protocol);
        sprintf(dnat_markrule,"%s -p %s",dnat_markrule,pn->protocol);
        if(pn->orig_port_start == pn->orig_port_end)
        {
            sprintf(dnat_rule,"%s --dport %d",dnat_rule,pn->orig_port_start);
            sprintf(dnat_markrule,"%s --dport %d",dnat_markrule,pn->orig_port_start);
        }
        else
        {
            sprintf(dnat_rule,"%s --dport %d:%d",dnat_rule,pn->orig_port_start,pn->orig_port_end);
            sprintf(dnat_markrule,"%s --dport %d:%d",dnat_markrule,pn->orig_port_start,pn->orig_port_end); 
        }
    
    }
  else if((pn->protocol) && (pn->orig_port_start) && (pn->orig_port_end))
    { 
        need_udp_rule=1;
        sprintf(dnat_rule,"%s --protocol tcp",dnat_rule);
        sprintf(dnat_markrule,"%s --protocol tcp",dnat_markrule);
        if(pn->orig_port_start == pn->orig_port_end)
        {
            sprintf(dnat_rule,"%s --dport %d",dnat_rule,pn->orig_port_start);
            sprintf(dnat_markrule,"%s --dport %d",dnat_markrule,pn->orig_port_start);
        }
        else
        {
            sprintf(dnat_rule,"%s --dport %d:%d",dnat_rule,pn->orig_port_start,pn->orig_port_end);
            sprintf(dnat_markrule,"%s --dport %d:%d",dnat_markrule,pn->orig_port_start,pn->orig_port_end);
        }
        
    }

    
    /*This condition checks original source ip, if it is not any it will add DNAT rule considering source*/
    if((pn->orig_src_ip) && (strcmp(pn->orig_src_ip,"any")))
    {
        char orig_srcip[64]={'\0'};
        int iprange=0;

        get_ipaddress(pn->orig_src_ip,orig_srcip,&iprange);
        if(iprange == 1)
        {
            sprintf(dnat_rule,"%s -m iprange --src-range %s",dnat_rule,orig_srcip);
            sprintf(dnat_markrule,"%s -m iprange --src-range %s",dnat_markrule,orig_srcip);
        }
        else
        {
            sprintf(dnat_rule,"%s -s %s",dnat_rule,orig_srcip);
            sprintf(dnat_markrule,"%s -s %s",dnat_markrule,orig_srcip);
        }
    }



  if((pn->orig_dest_ip) && (strcmp(pn->orig_dest_ip,"any")))
  {
          if(orig_range_ip && trans_range_ip)
          {
          
            sprintf(dnat_rule,"%s -d %s",dnat_rule,orig_range_ip);
            sprintf(dnat_markrule,"%s -d %s",dnat_markrule,orig_range_ip);

            if((strcmp(pn->src_interface,"any")) && (pn->trans_dest_ip))
            {
                findIface = search_iface(pn->src_interface);
                if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
                {
                    printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
                    return 1;
                }
                add_snatip(orig_range_ip,findIface);
            }
          }
          else
          {
          char orig_destip[64]={'\0'};
            int iprange=0;

          get_ipaddress(pn->orig_dest_ip,orig_destip,&iprange);
          if(iprange == 1)
          {
            sprintf(dnat_rule,"%s -m iprange --dst-range %s",dnat_rule,orig_destip);
            sprintf(dnat_markrule,"%s -m iprange --dst-range %s",dnat_markrule,orig_destip);
            /*This code adds alias interface and assign original destination ip for  the interface for range of  address*/
            if((strcmp(pn->src_interface,"any")) && (pn->trans_dest_ip))
            {
                char *orig_startip=NULL;
                char orig_tempIP[64]={'\0'};
                char *orig_endip=NULL;
                int range_end=0;

                findIface = search_iface(pn->src_interface);
                if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
                {
                    printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
                    return 1;
                }

                orig_startip=strtok(orig_destip,"-");
                orig_endip=strtok(NULL,"-");
                strcpy(orig_tempIP,orig_startip);

                while (!range_end)
                {
                    if(!strcmp(orig_tempIP,orig_endip))
                        range_end=1;

                    add_snatip(orig_tempIP,findIface);
                    strcpy(orig_tempIP,incrementIpAddr(orig_tempIP));

                }
            }
          }
          else
          {
            sprintf(dnat_rule,"%s -d %s",dnat_rule,orig_destip);
            sprintf(dnat_markrule,"%s -d %s",dnat_markrule,orig_destip);
            /*This code adds alias interface and assign original destination ip for  the interface if  ipaddress is single*/
            if((strcmp(pn->src_interface,"any")) && (pn->trans_dest_ip))
            {
                findIface = search_iface(pn->src_interface);
                if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
                {
                    printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
                    return 1;
                }

                if(iprange == 2)
                {
                    char subnet_start_temp[64]={'\0'};
                    int subnet_range_end=0;
                    int length=0;

                    strcpy(subnet_start_temp,subnet_startip);
                    
                    while ((!subnet_range_end) && (length < 255))
                    {
                        add_snatip(subnet_start_temp,findIface);

                        if(!strcmp(subnet_start_temp,subnet_endip))
                           subnet_range_end=1;
                        
                        length++;
                        
                        strcpy(subnet_start_temp,incrementIpAddr(subnet_start_temp));
                     }
                }
                else
                {
                    add_snatip(orig_destip,findIface);
                }
            }
          }
        }
  }
  if((pn->trans_dest_ip) && (strcmp(pn->trans_dest_ip,"any")))
  {
            char trans_destip[64]={'\0'};
            int iprange=0;

            get_ipaddress(pn->trans_dest_ip,trans_destip,&iprange);

            if ((iprange == 2) && (subnet_prefix) && ((pn->trans_port_start) == 0))
            {
                sprintf(dnat_rule,"%s --to %s",dnat_rule,trans_destip);            
            }
            else if((iprange == 1) && (orig_range_ip && trans_range_ip))
            {
                sprintf(dnat_rule,"%s --to-destination %s",dnat_rule,trans_range_ip);            
            }
            else if (iprange == 2)
            {
                sprintf(dnat_rule,"%s --to-destination %s-%s",dnat_rule,subnet_startip,subnet_endip);
            }
            else
            {
            sprintf(dnat_rule,"%s --to-destination %s",dnat_rule,trans_destip);
            }
  }

  if(((pn->protocol) && (strcmp(pn->protocol,"all") != 0)) || ((pn->protocol) && (pn->trans_port_start) && (pn->trans_port_end)))
    {
        if((pn->trans_port_start) != 0)
        {
        if(pn->trans_port_start == pn->trans_port_end)
        {
            if(pn->trans_dest_ip)
                sprintf(dnat_rule,"%s:%d",dnat_rule,pn->trans_port_start);
            else
                sprintf(dnat_rule,"%s --to :%d",dnat_rule,pn->trans_port_start);
        }
        else
        {
            if(pn->trans_dest_ip)
                sprintf(dnat_rule,"%s:%d-%d",dnat_rule,pn->trans_port_start,pn->trans_port_end);
            else
                sprintf(dnat_rule,"%s --to :%d-%d",dnat_rule,pn->trans_port_start,pn->trans_port_end);
        }
        }
    }

  if(strcmp(pn->src_interface,"any"))
  {
        findIface = search_iface(pn->src_interface);
        if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
        {
            printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
            return 1;
        }
        sprintf(dnat_rule,"%s -i %s",dnat_rule,findIface->l3ifname);
        sprintf(dnat_markrule,"%s -i %s",dnat_markrule,findIface->l3ifname);

        tokenize_and_docommand(dnat_rule, "nat", handle_nat);
        tokenize_and_docommand(dnat_markrule, "nat", handle_nat);
        if(need_udp_rule)
        {
            char udp_rule[2048]="";
            strreplace(udp_rule, dnat_rule, "protocol tcp", "protocol udp");
            tokenize_and_docommand(udp_rule, "nat", handle_nat);
            memset(udp_rule,'\0',sizeof(udp_rule));
            strreplace(udp_rule, dnat_markrule, "protocol tcp", "protocol udp");
            tokenize_and_docommand(udp_rule, "nat", handle_nat);
            
        }
  }
  else
  {
        tokenize_and_docommand(dnat_rule, "nat", handle_nat);
        tokenize_and_docommand(dnat_markrule, "nat", handle_nat); 

        if(need_udp_rule)
        {
            char udp_rule[2048]="";
            strreplace(udp_rule, dnat_rule, "protocol tcp", "protocol udp");
            tokenize_and_docommand(udp_rule, "nat", handle_nat);
            memset(udp_rule,'\0',sizeof(udp_rule));
            strreplace(udp_rule, dnat_markrule, "protocol tcp", "protocol udp");
            tokenize_and_docommand(udp_rule, "nat", handle_nat);
        }
        
        memset(loopback_rule,'\0',sizeof(loopback_rule));
        sprintf(loopback_rule,"iptables-uci -t nat -I policynat_dnat -i lo -j RETURN");
        tokenize_and_docommand(loopback_rule, "nat", handle_nat);
  }

  return 0;
}

void add_snatip(char *ipaddress,struct iface_data *findIface)
{
        char cmdBuff[256]={'0'};
        unsigned long policynatip = 0;
        unsigned long ifaceip=0,netmask=0,prefix=0;
        struct in_addr inp;


        inet_aton(ipaddress,&inp);

        policynatip = htonl((inp.s_addr));

        inet_aton(findIface->ipaddr,&inp);

        ifaceip=htonl((inp.s_addr));
        prefix= findIface->subnet;

        if (prefix) {
                netmask=(~((1 << (32 - prefix)) - 1));
            }

        /*Create alias interface only when policynat ip and interface ip are in same subnet*/
        if((ifaceip != policynatip) && ((policynatip & netmask) == (ifaceip & netmask)))
        {   
        
            sprintf(cmdBuff,"ip address add %s dev %s label %s:2 2>/dev/null",ipaddress, findIface->l3ifname, findIface->l3ifname);
            system(cmdBuff);
        }

}

int insert_policynat_snat(struct uci_policynatRule *pn,struct uci_context *ctx,struct iptc_handle *handle_nat,char *orig_range_srcip,char *trans_range_srcip,char *subnet_start,char *subnet_end,int mark,int subnet_prefix)
{
    struct iface_data *findIface;
    char snat_rule[2048]={'0'};
    char loopback_rule[256]={'\0'};
    int need_udp_rule=0;

    if(((pn->trans_dest_ip) || (pn->trans_port_start) || (strcmp(pn->src_interface,"any") != 0)) && (pn->trans_src_ip))
    {
     
	if(subnet_prefix)
            sprintf(snat_rule,"iptables-uci -t nat -A policynat_snat -j NETMAP -m mark --mark 0x%x",mark);
        else
            sprintf(snat_rule,"iptables-uci -t nat -A policynat_snat -j SNAT -m mark --mark 0x%x",mark);
    }
    else if(((pn->trans_dest_ip) || (pn->trans_port_start)) && (!(pn->trans_src_ip)))
    {
        sprintf(snat_rule,"iptables-uci -t nat -A policynat_snat -j ACCEPT -m mark --mark 0x%x",mark);
    }
    else
    {
        if(subnet_prefix)
            sprintf(snat_rule,"iptables-uci -t nat -A policynat_snat -j NETMAP");
        else
            sprintf(snat_rule,"iptables-uci -t nat -A policynat_snat -j SNAT");
    }
    
    if((pn->protocol) && (strcmp(pn->protocol,"all") != 0))
        {
            sprintf(snat_rule,"%s -p %s",snat_rule,pn->protocol);
            if((pn->trans_port_start) && (pn->trans_port_end))
            {
            if(pn->trans_port_start == pn->trans_port_end)
                sprintf(snat_rule,"%s --dport %d",snat_rule,pn->trans_port_start);
            else
                sprintf(snat_rule,"%s --dport %d:%d",snat_rule,pn->trans_port_start,pn->trans_port_end);
            }
            else if((pn->orig_port_start) && (pn->orig_port_end))
            {
                if(pn->orig_port_start == pn->orig_port_end)
                    sprintf(snat_rule,"%s --dport %d",snat_rule,pn->orig_port_start);
                else
                    sprintf(snat_rule,"%s --dport %d:%d",snat_rule,pn->orig_port_start,pn->orig_port_end);
            }

        }
    else if((pn->protocol) && (pn->trans_port_start) && (pn->trans_port_end)) 
    {
        need_udp_rule=1;
        sprintf(snat_rule,"%s --protocol tcp",snat_rule);
        if(pn->trans_port_start == pn->trans_port_end)
                sprintf(snat_rule,"%s --dport %d",snat_rule,pn->trans_port_start);
        else
                sprintf(snat_rule,"%s --dport %d:%d",snat_rule,pn->trans_port_start,pn->trans_port_end);
    
    }
    else if((pn->protocol) && (pn->orig_port_start) && (pn->orig_port_end))
    {
        need_udp_rule=1;
        sprintf(snat_rule,"%s --protocol tcp",snat_rule);
        if(pn->orig_port_start == pn->orig_port_end)
            sprintf(snat_rule,"%s --dport %s",snat_rule,pn->orig_port_start);
        else
            sprintf(snat_rule,"%s --dport %d:%d",snat_rule,pn->orig_port_start,pn->orig_port_end);
    
    }

    if((pn->orig_src_ip) && (strcmp(pn->orig_src_ip,"any")))
    {
            if(orig_range_srcip && trans_range_srcip)
            {
                sprintf(snat_rule,"%s -s %s",snat_rule,orig_range_srcip);
            
            }
            else
            {
            char orig_srcip[64]={'\0'};
            int iprange=0;

            get_ipaddress(pn->orig_src_ip,orig_srcip,&iprange);
            if(iprange == 1)
            {
                sprintf(snat_rule,"%s -m iprange --src-range %s",snat_rule,orig_srcip);
            }
            else
                sprintf(snat_rule,"%s -s %s",snat_rule,orig_srcip);        
            }
        
    }
    if((pn->trans_dest_ip) && (strcmp(pn->trans_dest_ip,"any")))
    {
            char trans_destip[64]={'\0'};
            int iprange=0;
            get_ipaddress(pn->trans_dest_ip,trans_destip,&iprange);        
            if(iprange == 1)
                sprintf(snat_rule,"%s -m iprange --dst-range %s",snat_rule,trans_destip);
            else
                sprintf(snat_rule,"%s -d %s",snat_rule,trans_destip);
    }
    else if((pn->orig_dest_ip) && (strcmp(pn->orig_dest_ip,"any")))
    {

        char orig_destip[64]={'\0'};
        int iprange=0;
        get_ipaddress(pn->orig_dest_ip,orig_destip,&iprange);
        if(iprange == 1)
            sprintf(snat_rule,"%s -m iprange --dst-range %s",snat_rule,orig_destip);
        else
            sprintf(snat_rule,"%s -d %s",snat_rule,orig_destip);

    }

    if((pn->trans_src_ip) && (strcmp(pn->trans_src_ip,"any")))
    {
        if((strcmp(pn->trans_src_ip,"wan1") == 0) || (strncmp(pn->trans_src_ip,"usb",3) == 0))
        {
            findIface = search_iface(pn->trans_src_ip);
            if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
            {   
                printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
                return 1;
            }   

            sprintf(snat_rule,"%s --to-source %s",snat_rule,findIface->ipaddr);
        }
        else if(strncmp(pn->trans_src_ip,"vlan",3) == 0)
        {
        
            findIface = search_iface(pn->trans_src_ip);
            if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
            {
                printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->src_interface);
                return 1;
            }

            sprintf(snat_rule,"%s --to-source %s",snat_rule,findIface->ipaddr);
        }
        else
        {

            if(orig_range_srcip && trans_range_srcip)
            {
                sprintf(snat_rule,"%s --to-source %s",snat_rule,trans_range_srcip);
            }
            else
            {
                char trans_srcip[64]={'\0'};
                int iprange=0;

                get_ipaddress(pn->trans_src_ip,trans_srcip,&iprange);

                if((iprange ==2) && (subnet_prefix))
                {
                    sprintf(snat_rule,"%s --to %s",snat_rule,trans_srcip);
                }
                else if (iprange == 2)
                {
                    sprintf(snat_rule,"%s --to-source %s-%s",snat_rule,subnet_start,subnet_end);
                }
                else
                {
                    sprintf(snat_rule,"%s --to-source %s",snat_rule,trans_srcip); 
                }
        
            }
        }
    }
    
    if(strcmp(pn->dest_interface,"any"))
    {
        findIface = search_iface(pn->dest_interface);
        if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
        {
            printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->dest_interface);
            return 1;
        }
        sprintf(snat_rule,"%s -o %s",snat_rule,findIface->l3ifname);

        if(need_udp_rule)
        {
            char udp_rule[2048]="";
            strreplace(udp_rule, snat_rule, "protocol tcp", "protocol udp");
            tokenize_and_docommand(udp_rule, "nat", handle_nat);
        
        }
        tokenize_and_docommand(snat_rule, "nat", handle_nat);

        char trans_srcip[64]={'\0'};
        
        if((pn->trans_src_ip) && (strcmp(pn->trans_src_ip,"wan1") != 0) && (strncmp(pn->trans_src_ip,"vlan",3) != 0))
        {
            if(orig_range_srcip && trans_range_srcip)
            {
                add_snatip(trans_range_srcip,findIface);
            }
            else
            {
       
                int iprange=0;

                get_ipaddress(pn->trans_src_ip,trans_srcip,&iprange);

                if(iprange == 1)
                {
                    char *trans_startip=NULL;
                    char *trans_endip=NULL;
                    char trans_tempIP[64]={'\0'};
                    int range_end=0;

                    trans_startip=strtok(trans_srcip,"-");
                    trans_endip=strtok(NULL,"-");
                    strcpy(trans_tempIP,trans_startip);

                    while (!range_end)
                    {
                        if(!strcmp(trans_tempIP,trans_endip))
                            range_end=1;

                        add_snatip(trans_tempIP,findIface);
                        strcpy(trans_tempIP,incrementIpAddr(trans_tempIP));

                    }

                }
                else if (iprange == 2)
                {
                
                        char subnet_start_temp[64]={'\0'};
                        int subnet_range_end=0;
                        int length=0;

                        strcpy(subnet_start_temp,subnet_start);
                        while ((!subnet_range_end) && (length < 255))
                        {
                            add_snatip(subnet_start_temp,findIface);

                            if(!strcmp(subnet_start_temp,subnet_end))
                                  subnet_range_end=1;
                            length++;
                            strcpy(subnet_start_temp,incrementIpAddr(subnet_start_temp));
                        }
                }
                else
                {
                    add_snatip(trans_srcip,findIface);
                }
            }
        }
    }
  else
    {
        if(need_udp_rule)
        {
            char udp_rule[2048]="";
            strreplace(udp_rule, snat_rule, "protocol tcp", "protocol udp");
            tokenize_and_docommand(udp_rule, "nat", handle_nat);
        }
        
        tokenize_and_docommand(snat_rule, "nat", handle_nat);

        memset(loopback_rule,'\0',sizeof(loopback_rule));
        sprintf(loopback_rule,"iptables-uci -t nat -I policynat_snat -o lo -j RETURN");
        tokenize_and_docommand(loopback_rule, "nat", handle_nat);
    }
  
    
    if((pn->trans_src_ip) && (!(pn->trans_dest_ip)) && (strcmp(pn->src_interface,"any")))
    {
        char dnat_markrule[2048]={'\0'};
        findIface = search_iface(pn->src_interface);
        if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
        {
            printf("policy nat for the interface:%s is not configured since the interface is not active/UP\r\n",pn->dest_interface);
            return 1;
        }

        sprintf(dnat_markrule,"iptables-uci -t nat -I policynat_dnat -j MARK --set-mark 0x%x -i %s",mark,findIface->l3ifname);
        tokenize_and_docommand(dnat_markrule, "nat", handle_nat);        
                            
    }

  //add_snatip(pn->trans_src_ip,findIface->l3ifname);

  return 0;
}

int insert_policynat_output(struct uci_policynatRule *pn, struct uci_context *ctx,struct iptc_handle *handle_filter,struct iptc_handle *handle_nat)
{
    char output_rule[2048]={'0'};

    sprintf(output_rule,"iptables-uci -t nat -I policynat_snat -j RETURN -m mark --mark 0x6fbb");
    tokenize_and_docommand(output_rule, "nat", handle_nat);

    memset(output_rule,'\0',sizeof(output_rule));
    sprintf(output_rule,"iptables-uci -I policynat_output -j MARK --set-mark 0x6fbb");
    tokenize_and_docommand(output_rule, "filter", handle_filter);

}

int insert_policynat(struct uci_policynatRule *pn, struct uci_context *ctx,struct iptc_handle *handle_filter, struct iptc_handle *handle_nat,int mark)
{

        if((pn->orig_dest_ip)  || (pn->orig_port_start))
        {
             if(pn->trans_dest_ip)
             {
                 char trans_ip_range[64]={'\0'};
                 int dest_iprange=0;

                get_ipaddress(pn->trans_dest_ip,trans_ip_range,&dest_iprange);
                if(dest_iprange == 1)
                {
             
                    char *trans_startip=NULL;
                    char *orig_startip=NULL;
                    char trans_tempIP[64]={'\0'};
                    char orig_tempIP[64]={'\0'};
                    char *trans_endip=NULL;
                    char *orig_endip=NULL;
                    char orig_ip_range[64]={'\0'};
                    int range_end=0;

                    trans_startip=strtok(trans_ip_range,"-");
                    trans_endip=strtok(NULL,"-");
                    strcpy(trans_tempIP,trans_startip);

                    get_ipaddress(pn->orig_dest_ip,orig_ip_range,&dest_iprange);

                    orig_startip=strtok(orig_ip_range,"-");
                    orig_endip=strtok(NULL,"-");
                    strcpy(orig_tempIP,orig_startip);

                    while (!range_end)
                    {

                        insert_policynat_dnat(pn,ctx,handle_nat,orig_tempIP,trans_tempIP,NULL,NULL,0);

                        if(!strcmp(trans_tempIP,trans_endip))
                            range_end=1;

                        strcpy(trans_tempIP,incrementIpAddr(trans_tempIP));
                        strcpy(orig_tempIP,incrementIpAddr(orig_tempIP));
                                                                        
                    }
                                                                    

                }
                else if(dest_iprange == 2)
                {
                    char *trans_dest_ip = NULL;
                    char *prefix=NULL;
                    char trans_start_ip[64]={'\0'};
                    char trans_end_ip[64]={'\0'};
                    int orig_range=0;
                    char orig_address_range[64]={'\0'};
                    char *orig_subnet=NULL;
                    char *orig_prefix=0;
                    int subnet_prefix=0;

                    trans_dest_ip=strtok(trans_ip_range,"/");
                    prefix=strtok(NULL,"/");
                    get_range_from_subnet(trans_dest_ip,atoi(prefix),trans_start_ip,trans_end_ip);
                    get_ipaddress(pn->orig_dest_ip,orig_address_range,&orig_range);
                    if(orig_range == 2)
                    {
                        orig_subnet=strtok(orig_address_range,"/");
                        orig_prefix=strtok(NULL,"/");
                        if(atoi(orig_prefix) == atoi(prefix))
                            subnet_prefix=1;
                    }

                    insert_policynat_dnat(pn,ctx,handle_nat,NULL,NULL,trans_start_ip,trans_end_ip,subnet_prefix);
               
                }
                else
                {
                    insert_policynat_dnat(pn,ctx,handle_nat,NULL,NULL,NULL,NULL,0);
                }
             }
             else
             {
                    insert_policynat_dnat(pn,ctx,handle_nat,NULL,NULL,NULL,NULL,0);
             }
        }

        if(pn->orig_src_ip)
        {
            char trans_srcip_range[64]={'\0'};
            int src_iprange=0;
            int src_subnet=0;
            get_ipaddress(pn->trans_src_ip,trans_srcip_range,&src_iprange);
           
            if(src_iprange == 1)
            {
                char *trans_src_startip=NULL;
                char *orig_src_startip=NULL;
                char trans_src_tempIP[64]={'\0'};
                char orig_src_tempIP[64]={'\0'};
                char *trans_src_endip=NULL;
                char *orig_src_endip=NULL;
                char orig_srcip_range[64]={'\0'};
                int src_range_end=0;
                int orig_src_iprange=0;

                trans_src_startip=strtok(trans_srcip_range,"-");
                trans_src_endip=strtok(NULL,"-");
                strcpy(trans_src_tempIP,trans_src_startip);

                get_ipaddress(pn->orig_src_ip,orig_srcip_range,&orig_src_iprange);

                if(orig_src_iprange == 1)
                {
                    orig_src_startip=strtok(orig_srcip_range,"-");
                    orig_src_endip=strtok(NULL,"-");
                    strcpy(orig_src_tempIP,orig_src_startip);


                    while (!src_range_end)
                    {

                        insert_policynat_snat(pn,ctx,handle_nat,orig_src_tempIP,trans_src_tempIP,NULL,NULL,mark,0);

                        if(!strcmp(trans_src_tempIP,trans_src_endip))
                            src_range_end=1;

                        strcpy(trans_src_tempIP,incrementIpAddr(trans_src_tempIP));
                        strcpy(orig_src_tempIP,incrementIpAddr(orig_src_tempIP));
                    }
                }
                else
                {
                    insert_policynat_snat(pn,ctx,handle_nat,NULL,NULL,NULL,NULL,mark,0);
                
                }
                 
            }
            else if(src_iprange == 2)
            {
                char *trans_src_ip = NULL;
                char *prefix=NULL;
                char trans_start_ip[64]={'\0'};
                char trans_end_ip[64]={'\0'};
                int orig_src_range=0;
                char orig_src_address_range[64]={'\0'};
                char *orig_src_subnet=NULL;
                char *orig_src_prefix=0;
                int subnet_src_prefix=0;

                trans_src_ip=strtok(trans_srcip_range,"/");
                prefix=strtok(NULL,"/");
                get_range_from_subnet(trans_src_ip,atoi(prefix),trans_start_ip,trans_end_ip);
                get_ipaddress(pn->orig_src_ip,orig_src_address_range,&orig_src_range);
                if(orig_src_range == 2)
                {
                    orig_src_subnet=strtok(orig_src_address_range,"/");
                    orig_src_prefix=strtok(NULL,"/");
                    if(atoi(orig_src_prefix) == atoi(prefix))
                        subnet_src_prefix=1;
			printf("orig prefix:%d subnet :%d\n",orig_src_prefix,subnet_src_prefix);
                }

                insert_policynat_snat(pn,ctx,handle_nat,NULL,NULL,trans_start_ip,trans_end_ip,mark,subnet_src_prefix);
            }
            else
            {
                insert_policynat_snat(pn,ctx,handle_nat,NULL,NULL,NULL,NULL,mark,0);
            }
        }
        else
        {
             insert_policynat_snat(pn,ctx,handle_nat,NULL,NULL,NULL,NULL,mark,0);
        }



        if(pn->trans_src_ip)
        {
            insert_policynat_output(pn,ctx,handle_filter,handle_nat);
        }

        insert_policynat_forward(pn,ctx,handle_filter);
}

int insert_policynat_bypass(struct uci_context *ctx,struct iptc_handle *handle_nat)
{

    struct iface_data *findIface;
    struct list_head *p;
    char natRule[2048]="";

    list_for_each(p, &allIfaces) {
        findIface = list_entry (p, struct iface_data, list);

        if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
        {
            printf("DMZ HOST: for the interface:%s is not configured since the interface is not active/UP\r\n",findIface->ifname);
            continue;
        }

        if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
            && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
            && (strncmp(findIface->ifname,"wan16",5)!=0
            && strncmp(findIface->ifname,"wan26",5)!=0
            && strncmp(findIface->l3ifname,"6in4",4)!=0
            && strncmp(findIface->l3ifname,"6rd",3)!=0)
          )
        {
            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p tcp --dport 1723"
                                        ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);

            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p udp --dport 500"
                                        ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);

            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p udp --dport 4500"
                                ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);

            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p esp"
                                    ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);
                                                                                                                                                                                     memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p ah"
                                     ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);
            
            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p 47"
                                    ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);
            
            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p 108"
                                    ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat); 

            memset(natRule, '\0', sizeof(natRule));
            sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i %s -d %s -j RETURN -p udp --dport 161"
                                    ,findIface->l3ifname,findIface->ipaddr);
            tokenize_and_docommand(natRule,"nat",handle_nat);

            struct uci_ptr ptr;
            char *strptr=strdup("openvpn.global.enable");
            int sslvpn=0,port=0;
            char protocol[256]={'\0'};
                    
                if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
                    printf(" Error in getting info about:%s\r\n",strptr);
                }

                if(ptr.o->v.string)
                    sslvpn=atoi(ptr.o->v.string);

                if(sslvpn)
                {

                    char *strptr1=strdup("openvpn.global.proto");
                    if (uci_lookup_ptr(ctx, &ptr, strptr1, true) != UCI_OK) {
                            printf("Error in getting info about:%s\r\n",strptr1);
                    }

                    if(ptr.o->v.string)
                        strcpy(protocol,ptr.o->v.string);

                    char *strptr2=strdup("openvpn.global.port");
                    if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
                        printf("Error in getting info about:%s\r\n",strptr2);
                    }

                    if(ptr.o->v.string)
                        port=atoi(ptr.o->v.string);

                    memset(natRule, '\0', sizeof(natRule));
                    sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -j RETURN -p %s --dport %d -i %s -d %s"
                                        ,protocol,port,findIface->l3ifname,findIface->ipaddr);
                    tokenize_and_docommand(natRule,"nat",handle_nat);
                }
        }
    }

    memset(natRule, '\0', sizeof(natRule));
    sprintf(natRule,"iptables-uci -t nat -I policynat_dnat -i eth2+ -j RETURN -p udp --dport 67");
    tokenize_and_docommand(natRule,"nat",handle_nat);    

}
void config_policynat(void)
{
	struct list_head *p;
	struct uci_policynatRule *pn;
	int y=0;
	struct iptc_handle *handle_filter = NULL;
	struct iptc_handle *handle_nat = NULL;
    struct uci_context *ctx;
    struct uci_package *pkg;
    int policynat_bypass=1,mark=0xFF01;

    INIT_LIST_HEAD(&policynatRules);

    ctx = uci_alloc_context();
    ucimap_init(&firewall_policynat_map);

    if (uci_load(ctx, "firewall", &pkg))
        {
            //uci_perror(state->uci, NULL);
            fprintf(stderr, "Error: Can't load config");
            return 0;
        }
    ucimap_parse(&firewall_policynat_map, pkg);
                                                    
	handle_filter = create_handle("filter");
	handle_nat = create_handle("nat");
	
	if(iptc_is_chain(strdup("policynat"), handle_filter)
		&& iptc_is_chain(strdup("policynat_dnat"), handle_nat)
        && iptc_is_chain(strdup("policynat_snat"), handle_nat))
	{
		if(!iptc_flush_entries(strdup("policynat"),handle_filter))
			printf("Failed in flushing rules of policynat in iptables\r\n");

		if(!iptc_flush_entries(strdup("policynat_dnat"), handle_nat))
			printf("Failed in flushing rules of policynat_dnat in iptables\r\n");
		
		if(!iptc_flush_entries(strdup("policynat_snat"), handle_nat))
			printf("Failed in flushing rules of policynat_snat in iptables\r\n");

        if(!iptc_flush_entries(strdup("policynat_output"), handle_filter))
            printf("Failed in flushing rules of policynat_output in iptables\r\n");

	}
	else
		printf("No chain detected with the name policynat/policynat_dnat/policynat_snat for IPv4\r\n");

	list_for_each(p, &policynatRules) {

		pn = list_entry(p, struct uci_policynatRule, list);

		if (pn->status)
        {
            if((pn->trans_src_ip) && (!((pn->trans_dest_ip) || (pn->trans_port_start))) && (strcmp(pn->src_interface,"any")))
            {
    			insert_policynat(pn, ctx, handle_filter, handle_nat,mark);
                mark=mark+1;
            }
            else
            {
                insert_policynat(pn, ctx, handle_filter, handle_nat,0xFFef);
            }
            if(policynat_bypass && ((pn->trans_dest_ip) || (pn->trans_port_start)))
            {
                insert_policynat_bypass(ctx,handle_nat);
                policynat_bypass=0;
            }
        }
		else
			printf("policy nat rule with the name:%s is disabled\n",pn->name);
	}

	y = iptc_commit(handle_filter);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle_filter);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of Policynat rules(filter table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of Policynat rules(filter table).\n",
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
				printf("Error commiting data: %s errno:%d in the context of Policynat rules(nat table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of policynat rules(nat table).\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	iptc_free(handle_filter);
	iptc_free(handle_nat);

    ucimap_cleanup(&firewall_policynat_map);
    uci_free_context(ctx);

}
