#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ucimap.h>
#include "list.h"
#include "iptables.h"
#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"
#include "list.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include "iptables-uci.h"

extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;
extern struct list_head allIfaces;

void print_basicsettings()
{
    int a=0;

    printf("Details of argc and argv passed to do_command are:\r\n");
    for (a = 0; a < mynewargc; a++)
        printf("argv[%u]: %s\t", a, mynewargv[a]);
    printf("\r\n");

}

void insert_udp_flood(char *interface,struct iptc_handle * handle)
{
    char *tablename="filter";
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("udp_flood");
    add_argv("--protocol");
    add_argv("udp");
    add_argv("--jump");
    add_argv("udp_flood_rules");
    add_argv("--in-interface");
    add_argv(interface);

    if(enable_debug)
    {
        printf("executing udp flood rules\n");
        print_basicsettings();
    }
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }
    

}
void insert_syn_flood(char *interface,struct iptc_handle * handle)
{
    char *tablename="filter";
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("syn_flood");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--jump");
    add_argv("syn_flood_rules");
    add_argv("--in-interface");
    add_argv(interface);

    if(enable_debug)
    {
        printf("executing syn flood rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();

}

void insert_icmp_flood(char *interface, struct iptc_handle * handle)
{
    char *tablename="filter";
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("icmp_flood");
    add_argv("--protocol");
    add_argv("icmp");
    add_argv("--jump");
    add_argv("icmp_flood_rules");
    add_argv("--in-interface");
    add_argv(interface);

    if(enable_debug)
    {
        printf("executing icmp flood rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();

}

void insert_ping_of_death(char *interface,struct iptc_handle * handle)
{
    char *tablename="filter";
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("ping_of_death");
    add_argv("--protocol");
    add_argv("icmp");
    add_argv("--icmp-type");
    add_argv("echo-request");
    add_argv("--jump");
    add_argv("ping_of_death_rules");
    add_argv("--in-interface");
    add_argv(interface);

    if(enable_debug)
    {
        printf("executing ping of death rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }   

    free_argv();
}

static int insert_flood_rules(struct uci_context *ctx,struct iptc_handle * handle)
{
    struct uci_ptr ptr;
    int dos_attack=0;
    char *strptr=strdup("firewall.global_configuration.dos_attack");

    if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
        printf("Error in getting info about:%s\r\n",strptr);
        return 1;
    }

    dos_attack = atoi(ptr.o->v.string);
    if(dos_attack)
    {
        struct iface_data *findIface;
        struct list_head *p;
        
        list_for_each(p, &allIfaces) {
            findIface = list_entry (p, struct iface_data, list);
            if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                    && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
                    && (strncmp(findIface->ifname,"wan16",5)!=0
                        && strncmp(findIface->ifname,"wan26",5)!=0)) 
            { 

                if((findIface) && (strcmp(findIface->l3ifname,"null")!=0) && (strncmp(findIface->l3ifname,"6in4",4) != 0)
                            && (strncmp(findIface->l3ifname,"6rd",3) != 0))
                {

                    insert_udp_flood(findIface->l3ifname,handle);
                    insert_syn_flood(findIface->l3ifname,handle);
                    insert_ping_of_death(findIface->l3ifname,handle);
                    insert_icmp_flood(findIface->l3ifname,handle);
                }
            }
        }
    }

    return 0;
}

void insert_wan_restconf_input(struct iptc_handle *filter_handle,struct iface_data *findIface,int restconf_port)
{
    char *tablename="filter";
    char buffer[16]={'\0'};
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buffer,"%d",restconf_port);
    add_argv(buffer);
    add_argv("--jump");
    add_argv("ACCEPT");

    add_argv("--in-interface");
    add_argv(findIface->l3ifname);
    add_argv("--destination");
    add_argv(findIface->ipaddr);
    if(enable_debug)
    {
        printf("executing remote management forward rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
}

void insert_wan_restconf_dnat(struct iptc_handle *nat_handle,struct iface_data *findIface,int restconf_port)
{
    char *tablename="nat";
    char buffer[16]={'\0'};
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("remote_management_dnat");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buffer,"%d",restconf_port);
    add_argv(buffer);
    add_argv("--jump");
    add_argv("ACCEPT");

    add_argv("--in-interface");
    add_argv(findIface->l3ifname);
    add_argv("--destination");
    add_argv(findIface->ipaddr);

    if(enable_debug)
    {
        printf("executing remote management dnat rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
}

void insert_wan_netconf_dnat(struct iptc_handle *nat_handle,struct iface_data *findIface,int netconf_port)
{
    char *tablename="nat";
    char buffer[16]={'\0'};
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("remote_management_dnat");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buffer,"%d",netconf_port);
    add_argv(buffer);
    add_argv("--jump");
    add_argv("ACCEPT");

    add_argv("--in-interface");
    add_argv(findIface->l3ifname);
    add_argv("--destination");
    add_argv(findIface->ipaddr);

    if(enable_debug)
    {
        printf("executing remote management dnat rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
}

void insert_wan_netconf_input(struct iptc_handle *filter_handle,struct iface_data *findIface,int netconf_port)
{
    char *tablename="filter";
    char buffer[16]={'\0'};
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buffer,"%d",netconf_port);
    add_argv(buffer);
    add_argv("--jump");
    add_argv("ACCEPT");

    add_argv("--in-interface");
    add_argv(findIface->l3ifname);
    add_argv("--destination");
    add_argv(findIface->ipaddr);
    if(enable_debug)
    {
        printf("executing remote management forward rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
}
void insert_remote_management_dnat_rules(struct iptc_handle *nat_handle,
                                            int remote_http,int remote_https,int port,char *remote_ipaddress,char *remote_end,struct iface_data *findIface)
{
    char *tablename="nat";
    char buffer[256]={'\0'};
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("remote_management_dnat");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    if(remote_http && port)
    {
        sprintf(buffer,"%d",port);
        add_argv(buffer);

    }
    else if(remote_https && port)
    {
        sprintf(buffer,"%d",port);
        add_argv(buffer);
    }

    add_argv("--jump");
    add_argv("ACCEPT");

    if(remote_ipaddress != NULL)
    {
        if(remote_ipaddress && strcmp(remote_ipaddress,"any"))
        {
            if(remote_end && (strcmp(remote_end,"")) && (strcmp(remote_ipaddress,remote_end)))
            {
                add_argv("--match");
                add_argv("iprange");
                add_argv("--src-range");
                sprintf(buffer,"%s-%s",remote_ipaddress,remote_end);
                add_argv(buffer);
            }
            else
            {
                add_argv("--source");
                sprintf(buffer,"%s",remote_ipaddress);
                add_argv(buffer);
            }

        }

    }
    add_argv("--in-interface");
    add_argv(findIface->l3ifname);
    add_argv("--destination");
    add_argv(findIface->ipaddr);

    if(enable_debug)
    {
        printf("executing remote management dnat rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();

}

void insert_remote_management_forward_rules(struct iptc_handle *filter_handle,int remote_http,int remote_https,int port,
                char *remote_ipaddress,char *remote_end,struct iface_data *findIface)
{
    char *tablename="filter";
    char buffer[256]={'\0'};
    char cmdbuf[256]={'\0'};
    int ret;

    mynewargc=0;

    printf("port in forward_rules %d\n",port);
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    if(remote_http && port)
    {
        sprintf(buffer,"%d",port);
        add_argv(buffer);

    }
    else if(remote_https && port)
    {
        sprintf(buffer,"%d",port);
        add_argv(buffer);
    }

    add_argv("--jump");
    add_argv("ACCEPT");

    if(remote_ipaddress != NULL)
    {
        if(strcmp(remote_ipaddress,"any"))
        {
            if((remote_end) && (strcmp(remote_end,"")) && (strcmp(remote_ipaddress,remote_end)))
            {
                add_argv("--match");
                add_argv("iprange");
                add_argv("--src-range");
                sprintf(buffer,"%s-%s",remote_ipaddress,remote_end);
                add_argv(buffer);
            }
            else
            {
                add_argv("--source");
                sprintf(buffer,"%s",remote_ipaddress);
                add_argv(buffer);
            }

        }

    }

    add_argv("--in-interface");
    add_argv(findIface->l3ifname);
    add_argv("--destination");
    add_argv(findIface->ipaddr);
    if(enable_debug)
    {
        printf("executing remote management forward rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    printf("port in %d\n",port);
    if(remote_http == 1)
        sprintf(cmdbuf,"echo \"listen %s:%d;\" >> /var/nginx/conf.d/wan.http.conf",findIface->ipaddr,port);
    else if(remote_https == 1)
        sprintf(cmdbuf,"echo \"listen %s:%d ssl;\" >> /var/nginx/conf.d/wan.https.conf",findIface->ipaddr,port);

    system(cmdbuf);
    free_argv();
}

int insert_remote_management(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle)
{

    struct uci_ptr ptr;
    char *strptr=strdup("firewall.global_configuration.remote_management");
    int remote_management=0;
    char cmdbuf[256]={'\0'};
    FILE *fp=NULL;
    int range=2;
    char remote_range[64];
    char remote_str[64]={'\0'};

    if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
        printf(" Error in getting info about:%s\r\n",strptr);
        return 1;
    }

    if(ptr.o->v.string)
        remote_management=atoi(ptr.o->v.string);


    if(remote_management)
    {
        int remote_http=0,remote_https=0,remote_management_port=0;
        char remote_ipaddress[256]={'\0'};
        char remote_end[256]={'\0'};

        char *strptr1=strdup("firewall.global_configuration.remote_http");
        if (uci_lookup_ptr(ctx, &ptr, strptr1, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr1);
            return 1;
        }

        if(ptr.o->v.string)
            remote_http=atoi(ptr.o->v.string);

        char *strptr2=strdup("firewall.global_configuration.remote_https");
        if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr2);
            return 1;
        }   

        if(ptr.o->v.string)
            remote_https=atoi(ptr.o->v.string);

        char *strptr3=strdup("firewall.global_configuration.remote_management_port");
        if (uci_lookup_ptr(ctx, &ptr, strptr3, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr3);
            return 1;
        }   

        if(ptr.o->v.string)
            remote_management_port=atoi(ptr.o->v.string);

        char *strptr4=strdup("firewall.global_configuration.remote_ipaddress");
        if (uci_lookup_ptr(ctx, &ptr, strptr4, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr4);
            return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
        if(ptr.o->v.string)
            strcpy(remote_ipaddress,ptr.o->v.string);
        }
        char *strptr5=strdup("firewall.global_configuration.remote_end");
        if (uci_lookup_ptr(ctx, &ptr, strptr5, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr4);
            return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
        if(ptr.o->v.string)
            strcpy(remote_end,ptr.o->v.string);
        }



        struct iface_data *findIface;
        struct list_head *p;

        sprintf(cmdbuf,"echo -n "" > /var/nginx/conf.d/wan.http.conf");
        system(cmdbuf);
        sprintf(cmdbuf,"echo -n "" > /var/nginx/conf.d/wan.https.conf");
        system(cmdbuf); 

        if(strcmp(remote_ipaddress,"") == 0)
        {
            return 1;
        }
        list_for_each(p, &allIfaces) {
            findIface = list_entry (p, struct iface_data, list);
            if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                    && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
                    && (strncmp(findIface->ifname,"wan16",5)!=0
                        && strncmp(findIface->ifname,"wan26",5)!=0))
            {
                if((findIface) && (strcmp(findIface->l3ifname,"null") != 0) && (strncmp(findIface->l3ifname,"6in4",4) != 0) 
                        && (strncmp(findIface->l3ifname,"6rd",3) != 0))
                {
			do {
				insert_remote_management_forward_rules(filter_handle,remote_http,remote_https,remote_management_port,
					remote_ipaddress,remote_end,findIface);
				insert_remote_management_dnat_rules(nat_handle,remote_http,remote_https,remote_management_port,
					remote_ipaddress,remote_end,findIface);

				sprintf(remote_str,"firewall.global_configuration.remote_v4_range%d",range);
				if (uci_lookup_ptr(ctx, &ptr, remote_str, true) != UCI_OK)
					return 1;
				if(ptr.flags & UCI_LOOKUP_COMPLETE)
				{
					if(ptr.o->v.string)
					{
						char *temp_ptr=NULL;
						strcpy(remote_range,ptr.o->v.string);
						temp_ptr=strtok(remote_range,"-");
						strcpy(remote_ipaddress,temp_ptr);
						temp_ptr = strtok(NULL,"-");
						strcpy(remote_end,temp_ptr);
					}
				}

				else
					break;
				range++;
				if(range == 7)
					break;
			} while(1);

                }
            }
        }
                    
        fp=popen("uci get -p /var/state/ system.core.booted","r");

        if(fgets((char *)cmdbuf, sizeof(cmdbuf), fp) == NULL)
        {
            printf("fgets failed\n");
            pclose(fp);
            return -1;
        }
        pclose(fp);
        
        printf("checking is boot %s\n",cmdbuf);
        int isboot=atoi(cmdbuf);
        printf("isboot %d\n",isboot);
        if(isboot)
        {
            printf("nginx is reloaded\n");
            system("/etc/init.d/nginx reload");
        }

        free(strptr1);
        free(strptr2);
        free(strptr3); 
        free(strptr4);
        free(strptr5);

    }
    free(strptr);

    return 0;
}


void insert_lan_netconf_input(struct iptc_handle *handle,int netconf_port)
{
    char final_lan_iface[16];
    char *tablename="filter";
    char buffer[16]={'\0'};
    int ret;

    sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-I");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buffer,"%d",netconf_port);
    add_argv(buffer);
    add_argv("--in-interface");
    add_argv(final_lan_iface);
    add_argv("--jump");
    add_argv("DROP");

    if(enable_debug)
    {
        printf("executing lan vpn http rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
    strcpy(mynewargv[12],"RETURN");
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    strcpy(mynewargv[10],"br-vlan+");
    //strcpy(mynewargv[12],"ACCEPT");
/*
When netconf is disabled on LAN in RV16W/RV26W, a drop rule is added to restrict access from the host 
*/
    strcpy(mynewargv[12],"DROP");
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();

}


void insert_lan_netconf_dnat(struct iptc_handle *handle,int netconf_port,struct iface_data *findIface)
{

    char buffer[1024]="iptables-uci -t nat -I remote_management_dnat";



    sprintf(buffer,"%s -p tcp -i %s --dport %d -d %s -j ACCEPT",buffer,findIface->l3ifname,netconf_port,findIface->ipaddr);
    tokenize_and_docommand(buffer, "nat", handle);


    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"iptables-uci -t nat -I remote_management_dnat -p tcp -i %s.4094 --dport %d -j RETURN",gLan_phy_iface,netconf_port,findIface->ipaddr);
    tokenize_and_docommand(buffer, "nat", handle);
}

int insert_netconf(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle)
{
    struct uci_ptr ptr;
    char *strptr1=strdup("firewall.global_configuration.netconf");
    int netconf=0,netconf_lan=0,netconf_wan=0,netconf_port=0;

        if (uci_lookup_ptr(ctx, &ptr, strptr1, true) != UCI_OK) {
                printf("Error in getting info about:%s\r\n",strptr1);
                return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
            if(ptr.o->v.string)
                netconf=atoi(ptr.o->v.string);
        }

        if(netconf)
        {
            char *strptr2=strdup("firewall.global_configuration.netconf_lan");
            
            if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
                    printf("Error in getting info about:%s\r\n",strptr2);
                    return 1;
            }

            if(ptr.flags & UCI_LOOKUP_COMPLETE)
            {
                if(ptr.o->v.string)
                    netconf_lan=atoi(ptr.o->v.string);
            }

            char *strptr3=strdup("firewall.global_configuration.netconf_wan");
            if (uci_lookup_ptr(ctx, &ptr, strptr3, true) != UCI_OK) {
                printf("Error in getting info about:%s\r\n",strptr3);
                return 1;
            }

            if(ptr.flags & UCI_LOOKUP_COMPLETE)
            {
                if(ptr.o->v.string)
                    netconf_wan=atoi(ptr.o->v.string);
            }

            char *strptr4=strdup("firewall.global_configuration.netconf_port");
            if (uci_lookup_ptr(ctx, &ptr, strptr4, true) != UCI_OK) {
                printf("Error in getting info about:%s\r\n",strptr4);
                return 1;
            }

            if(ptr.flags & UCI_LOOKUP_COMPLETE)
            {
                if(ptr.o->v.string)
                    netconf_port=atoi(ptr.o->v.string);
            }
            
            if(netconf_wan)
            {
                struct iface_data *findIface;
                struct list_head *p;

                list_for_each(p, &allIfaces) {
                    findIface = list_entry (p, struct iface_data, list);
                        if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                            && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
                            && (strncmp(findIface->ifname,"wan16",5)!=0
                            && strncmp(findIface->ifname,"wan26",5)!=0))
                        {

                            if((findIface) && (strcmp(findIface->l3ifname,"null") != 0) && (strncmp(findIface->l3ifname,"6in4",4) != 0)
                                && (strncmp(findIface->l3ifname,"6rd",3) != 0))
                            {
                            insert_wan_netconf_input(filter_handle,findIface,netconf_port);
                            insert_wan_netconf_dnat(nat_handle,findIface,netconf_port);
                            }
                        }

                }
            }
            if(!netconf_lan)
            {
                struct iface_data *findIface;
                struct list_head *p;

                insert_lan_netconf_input(filter_handle,netconf_port);
                list_for_each(p, &allIfaces) {
                    findIface = list_entry (p, struct iface_data, list);
                         if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                            && (strncmp(findIface->ifname,"vlan",3)==0))
                            {

                                insert_lan_netconf_dnat(nat_handle,netconf_port,findIface);
                            }
                }
            
            }
            free(strptr2);
            free(strptr3);
            free(strptr4); 

    }

        free(strptr1);
}

void insert_lanvpn_dnat(int port,struct iptc_handle *handle_nat)
{

    char commandBuff[1024];
    char *tablename="nat";
    struct iface_data *findIface;
    struct list_head *p;

    
    
    list_for_each(p, &allIfaces) {
        findIface = list_entry (p, struct iface_data, list);
        if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
            && (strncmp(findIface->ifname,"vlan",3)==0))
        {
	    memset(commandBuff,0,sizeof(commandBuff));
            sprintf(commandBuff,"iptables-uci -t nat -I remote_management_dnat -j ACCEPT -p tcp -i %s --dport %d -d %s",findIface->l3ifname,port,findIface->ipaddr);
            tokenize_and_docommand(commandBuff, "nat", handle_nat);
                
        }

    }


}
void insert_lanvpn_http_rules(int port,struct iptc_handle *handle)
{
    char final_lan_iface[16];
    char *tablename="filter";
    char buf[16]={0};
    int ret;

    sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-I");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buf,"%d",port);
    add_argv(buf); 
    add_argv("--in-interface");
    add_argv(final_lan_iface);
    add_argv("--jump");
    add_argv("ACCEPT");

    if(enable_debug)
    {
        printf("executing lan vpn http rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    //strcpy(mynewargv[10],"eth3.4094");
    sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
    strcpy(mynewargv[12],"RETURN");
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    strcpy(mynewargv[10],"br-vlan+");
    strcpy(mynewargv[12],"ACCEPT");
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();

}

void insert_lan_restconf_input(struct iptc_handle *handle,int restconf_port)
{
    char final_lan_iface[16];
    char *tablename="filter";
    char buffer[16]={'\0'};
    int ret;

        sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
        mynewargc=0;

        add_argv("newTestApp");
        add_argv("-t");
        add_argv("filter");
        add_argv("-I");
        add_argv("input_rule");
        add_argv("--protocol");
        add_argv("tcp");
        add_argv("--destination-port");
        sprintf(buffer,"%d",restconf_port);
        add_argv(buffer);
        add_argv("--in-interface");
        add_argv(final_lan_iface);
        add_argv("--jump");
        add_argv("ACCEPT");

        if(enable_debug)
        {
            printf("executing lan vpn http rules\n");
            print_basicsettings();
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

        sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
        strcpy(mynewargv[12],"RETURN");
        ret = do_command(mynewargc, mynewargv, &tablename, &handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

        strcpy(mynewargv[10],"br-vlan+");
        strcpy(mynewargv[12],"ACCEPT");
        ret = do_command(mynewargc, mynewargv, &tablename, &handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

        free_argv();
}

void insert_lan_restconf_dnat(struct iptc_handle *handle,int restconf_port,struct iface_data *findIface)
{

    char buffer[1024]="iptables-uci -t nat -I remote_management_dnat";

    sprintf(buffer,"%s -p tcp -i %s --dport %d -d %s -j ACCEPT",buffer,findIface->l3ifname,restconf_port,findIface->ipaddr);
    tokenize_and_docommand(buffer, "nat", handle);

    memset(buffer,0,sizeof(buffer));
    sprintf(buffer,"iptables-uci -t nat -I remote_management_dnat -p tcp -i %s.4094 --dport %d -j RETURN",gLan_phy_iface,restconf_port,findIface->ipaddr);
    tokenize_and_docommand(buffer, "nat", handle);

}

int insert_restconf(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle)
{
    struct uci_ptr ptr;
    char *strptr1=strdup("firewall.global_configuration.restconf");
    int restconf=0,restconf_lan=0,restconf_wan=0,restconf_port=0;

    if (uci_lookup_ptr(ctx, &ptr, strptr1, true) != UCI_OK) {
        printf("Error in getting info about:%s\r\n",strptr1);
        return 1;
    }

    if(ptr.flags & UCI_LOOKUP_COMPLETE)
    {
        if(ptr.o->v.string)
            restconf=atoi(ptr.o->v.string);
    }

    if(restconf)
    {
        char *strptr2=strdup("firewall.global_configuration.restconf_lan");

        if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr2);
            return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
            if(ptr.o->v.string)
                restconf_lan=atoi(ptr.o->v.string);
        }

        char *strptr3=strdup("firewall.global_configuration.restconf_wan");
        if (uci_lookup_ptr(ctx, &ptr, strptr3, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr3);
            return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
            if(ptr.o->v.string)
                restconf_wan=atoi(ptr.o->v.string);
        }

        char *strptr4=strdup("firewall.global_configuration.restconf_port");
        if (uci_lookup_ptr(ctx, &ptr, strptr4, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr4);
            return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
            if(ptr.o->v.string)
               restconf_port=atoi(ptr.o->v.string);
        }

        if(restconf_wan)
        {
            struct iface_data *findIface;
            struct list_head *p;

            list_for_each(p, &allIfaces) {
                findIface = list_entry (p, struct iface_data, list);
                if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                    && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
                    && (strncmp(findIface->ifname,"wan16",5)!=0
                    && strncmp(findIface->ifname,"wan26",5)!=0))
                {
                    if((findIface) && (strcmp(findIface->l3ifname,"null") != 0) && (strncmp(findIface->l3ifname,"6in4",4) != 0)
                        && (strncmp(findIface->l3ifname,"6rd",3) != 0))
                    {
                      insert_wan_restconf_input(filter_handle,findIface,restconf_port);
                      insert_wan_restconf_dnat(nat_handle,findIface,restconf_port);
                    }
                }
            }
        }
        if(restconf_lan)
        {
            struct iface_data *findIface;
            struct list_head *p;

            insert_lan_restconf_input(filter_handle,restconf_port);
            list_for_each(p, &allIfaces) {
                findIface = list_entry (p, struct iface_data, list);
                if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                 && (strncmp(findIface->ifname,"vlan",3)==0))
                {
                    insert_lan_restconf_dnat(nat_handle,restconf_port,findIface);
                }
            }
        }
        free(strptr2);
        free(strptr3);
        free(strptr4);
   }

   free(strptr1);


}

void insert_lanvpn_https_rules(int port,struct iptc_handle *handle)
{
    char final_lan_iface[16];
    char *tablename="filter";
    char buf[16]={0};
    int ret;

    sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-I");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("tcp");
    add_argv("--destination-port");
    sprintf(buf,"%d",port);
    add_argv(buf);
    add_argv("--in-interface");
    add_argv(final_lan_iface);
    add_argv("--jump");
    add_argv("ACCEPT");

    if(enable_debug)
    {
        printf("executing lan vpn https rules\n");
        print_basicsettings();
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    //strcpy(mynewargv[10],"eth3.4094");
    sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
    strcpy(mynewargv[12],"RETURN");
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }   

    strcpy(mynewargv[10],"br-vlan+");
    strcpy(mynewargv[12],"ACCEPT");
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
}
int insert_lanvpn_management(struct uci_context *ctx,struct iptc_handle *filter_handle,struct iptc_handle *nat_handle)
{
    struct uci_ptr ptr;
    int lan_vpn_http=0,lan_vpn_https=0,lan_http_port=80,lan_https_port=443;
    char *strptr=strdup("firewall.global_configuration.lan_vpn_http");
    char *strptr1=strdup("firewall.global_configuration.lan_vpn_https");
    char *strptr2=strdup("firewall.global_configuration.lan_http_port");
    char *strptr3=strdup("firewall.global_configuration.lan_https_port");

        if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr);
            return 1;
        }

        lan_vpn_http = atoi(ptr.o->v.string);

        if (uci_lookup_ptr(ctx, &ptr, strptr1, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr);
            return 1;
        }   

        lan_vpn_https = atoi(ptr.o->v.string);

        if(lan_vpn_http)
        {
		if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
                        printf("Error in getting info about:%s\r\n",strptr2);
                        return 1;
                }
                if(ptr.flags & UCI_LOOKUP_COMPLETE)
                {
                        if(ptr.o->v.string) 
                                lan_http_port = atoi(ptr.o->v.string);
                }

            insert_lanvpn_http_rules(lan_http_port,filter_handle);
            insert_lanvpn_dnat(lan_http_port,nat_handle);

        }
        if(lan_vpn_https)
        {
		if (uci_lookup_ptr(ctx, &ptr, strptr3, true) != UCI_OK) {
                        printf("Error in getting info about:%s\r\n",strptr2);
                        return 1;
                }
                if(ptr.flags & UCI_LOOKUP_COMPLETE)
                {
                        if(ptr.o->v.string)
                                lan_https_port = atoi(ptr.o->v.string);
                }

            insert_lanvpn_https_rules(lan_https_port,filter_handle);
            insert_lanvpn_dnat(lan_https_port,nat_handle);
        }


    free(strptr);
    free(strptr1);
    free(strptr2);
    free(strptr3);

    return 0;
}
void insert_block_wan_rules(struct iface_data *iface,struct iptc_handle *handle)
{

    char *tablename="filter";
    int ret;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("icmp");
    add_argv("--icmp-type");
    add_argv("ping");
    add_argv("--jump");
    add_argv("ACCEPT");
    add_argv("--in-interface");
    add_argv(iface->l3ifname);
    add_argv("--destination");
    add_argv(iface->ipaddr);

    if(enable_debug)
    {
        printf("executing block wan rules\n");
        print_basicsettings();
    }

    if(!iface->l3ifname)
    {
        free_argv();
        return;
    }
    ret = do_command(mynewargc, mynewargv, &tablename, &handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    free_argv();
}

int insert_block_wan(struct uci_context *ctx,struct iptc_handle *filter_handle)
{

    struct uci_ptr ptr;
    int block_wan=0;
    char *strptr=strdup("firewall.global_configuration.block_wan_ping_req");

    if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
        printf("Error in getting info about:%s\r\n",strptr);
        return 1;
    }

    block_wan = atoi(ptr.o->v.string);

    if(!block_wan)
    {

        struct iface_data *findIface;
        struct list_head *p;

        list_for_each(p, &allIfaces) {
            findIface = list_entry (p, struct iface_data, list);
            if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                    && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
                    && (strncmp(findIface->ifname,"wan16",5)!=0
                        && strncmp(findIface->ifname,"wan26",5)!=0))
            {

                if((findIface) && (strcmp(findIface->l3ifname,"null") !=0) && (strncmp(findIface->l3ifname,"6in4",4) != 0)
                                    && (strncmp(findIface->l3ifname,"6rd",3) != 0))
                {
                    insert_block_wan_rules(findIface,filter_handle);
                }

            }
        }

    }
        return 0;
}

void flush_basicsettings(struct iptc_handle *filter_handle,struct iptc_handle *nat_handle)
{


    if(iptc_is_chain(strdup("input_rule"), filter_handle))
    {
        if(!iptc_flush_entries(strdup("input_rule"),filter_handle))
            printf("Failed in flushing rules of input_rule\r\n");
    }
    else
    {
        printf("No chain detected with input_rule name\r\n");
    }

    if(iptc_is_chain(strdup("syn_flood"), filter_handle))
    {
        if(!iptc_flush_entries(strdup("syn_flood"),filter_handle))
            printf("Failed in flushing rules of syn_flood\r\n");
    }
    else
    {
        printf("No chain detected with syn_flood name\r\n");
    }

    if(iptc_is_chain(strdup("udp_flood"), filter_handle))
    {
        if(!iptc_flush_entries(strdup("udp_flood"),filter_handle))
            printf("Failed in flushing rules of udp_flood\r\n");
    }
    else
    {
        printf("No chain detected with udp_flood name\r\n");
    }

    if(iptc_is_chain(strdup("icmp_flood"), filter_handle))
    {   
        if(!iptc_flush_entries(strdup("icmp_flood"),filter_handle))
            printf("Failed in flushing rules of icmp_flood\r\n");
    }   
    else
    {   
        printf("No chain detected with icmp_flood name\r\n");
    }   

    if(iptc_is_chain(strdup("ping_of_death"), filter_handle))
    {   
        if(!iptc_flush_entries(strdup("ping_of_death"),filter_handle))
            printf("Failed in flushing rules of ping_of_death\r\n");
    }   
    else
    {   
        printf("No chain detected with ping_of_death name\r\n");
    }   

    if(iptc_is_chain(strdup("remote_management_dnat"), nat_handle))
    {

        if(!iptc_flush_entries(strdup("remote_management_dnat"),nat_handle))
            printf("Failed in flushing rules of remote_management_dnat\r\n");
    }
    else
    {
        printf("No chain detected with remote_management_dnat name\r\n");
    }


}
int config_basicsettings(struct uci_context *ctx)
{

    struct iptc_handle *filter_handle = NULL;
    struct iptc_handle *nat_handle=NULL;

    /*Create handle for filter table*/
    filter_handle = create_handle("filter");

    /*Create handle for nat*/
    nat_handle = create_handle("nat");
    
    flush_basicsettings(filter_handle,nat_handle);

    insert_remote_management(ctx,filter_handle,nat_handle);
    insert_flood_rules(ctx,filter_handle);
    insert_lanvpn_management(ctx,filter_handle,nat_handle);
    insert_block_wan(ctx,filter_handle);
    insert_netconf(ctx,filter_handle,nat_handle);
    insert_restconf(ctx,filter_handle,nat_handle);

    int y = iptc_commit(filter_handle);
    if (!y)
    {
	if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
	{
		sleep(1);
		y = iptc_commit(filter_handle);
		if (!y)
			printf("Error commiting data: %s errno:%d in the context of BasicSettings(filter table) even after retry.\n",
				iptc_strerror(errno),errno);
	}
	else
		printf("Error commiting data: %s errno:%d in the context of BasicSettings(filter table).\n",
			iptc_strerror(errno),errno);
        return -1;
    }

    y = iptc_commit(nat_handle);
    if (!y)
    {
	if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
	{
		sleep(1);
		y = iptc_commit(nat_handle);
		if (!y)
			printf("Error commiting data: %s errno:%d in the context of BasicSettings(nat table) even after retry.\n",
				iptc_strerror(errno),errno);
	}
	else
		printf("Error commiting data: %s errno:%d in the context of BasicSettings(nat table).\n",
			iptc_strerror(errno),errno);
        return -1;
    }

    iptc_free(filter_handle);
    iptc_free(nat_handle);

    return 0;
}

