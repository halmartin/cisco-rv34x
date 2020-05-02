#include "xtables.h"
#include "libiptc/libip6tc.h"
#include "ip6tables-multi.h"
#include "ip6tables-uci.h"

extern char *mynewargv[255];
extern int mynewargc;
extern struct list_head allIfaces;

void insert_udp_flood6(char *interface,struct ip6tc_handle * handle)
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
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }
}

void insert_syn_flood6(char *interface,struct ip6tc_handle * handle)
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
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    free_argv();
}

void insert_icmp_flood6(char *interface, struct ip6tc_handle * handle)
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

    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
            printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    free_argv();
}

void insert_ping_of_death6(char *interface,struct ip6tc_handle * handle)
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
    add_argv("ipv6-icmp");
    add_argv("--icmpv6-type");
    add_argv("echo-request");
    add_argv("--jump");
    add_argv("ping_of_death_rules");
    add_argv("--in-interface");
    add_argv(interface);

    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }   

    free_argv();
}

static int insert_flood_rules6(struct uci_context *ctx,struct ip6tc_handle * handle)
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
                    && (strncmp(findIface->ifname,"wan16",5)==0
                    || strncmp(findIface->ifname,"wan26",5)==0
                    || strncmp(findIface->ifname,"usb",3)==0
                    || strncmp(findIface->l3ifname,"6in4",4)==0
                    || strncmp(findIface->l3ifname,"6rd",3)==0) 
                )
            { 

                if(strcmp(findIface->l3ifname,"null") != 0)
                {
            insert_udp_flood6(findIface->l3ifname,handle);
            insert_syn_flood6(findIface->l3ifname,handle);
            insert_ping_of_death6(findIface->l3ifname,handle);
            insert_icmp_flood6(findIface->l3ifname,handle);
                }
            }
        }
    }

    return 0;
}

void insert_remote_management_forward_rules6(struct ip6tc_handle *filter_handle,int remote_http,int remote_https,int port,
                char *remote_ipaddress,char *remote_end,struct iface_data *findIface)
{
    char *tablename="filter";
    char buffer[256]={'\0'};
    char cmdbuf[256]={'\0'};
    char ifbuf[256]={'\0'};
    char ifcmd[256]={'\0'};
    FILE *fp=NULL;
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
    
    sprintf(buffer,"%d",port);
    add_argv(buffer);

    add_argv("--jump");
    add_argv("ACCEPT");

    if(remote_ipaddress != NULL)
    {
        if(strcmp(remote_ipaddress,"any"))
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
    if(strncmp(findIface->l3ifname,"ppoe",4) == 0)
    {
        sprintf(ifcmd,"ifconfig %s |grep Scope:Global |awk -F ' ' '{print $3}' |awk -F '/' '{print $1}'",findIface->l3ifname);
        fp=popen(ifcmd,"r");
        if(fgets(ifbuf, sizeof(ifbuf), fp) != NULL)
        {
            strcpy (findIface->ipaddr, ifbuf);
        }
    
        findIface->ipaddr[strlen(findIface->ipaddr)-1]='\0';
        pclose(fp);
    }
    add_argv("--destination");
    add_argv(findIface->ipaddr);
    ret = do_command6(mynewargc, mynewargv, &tablename, &filter_handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    if (strncmp(findIface->ipaddr,"0::0",4)!=0)
    if(remote_http == 1)
        sprintf(cmdbuf,"echo \"listen [%s]:%d;\" >> /var/nginx/conf.d/wan.http.conf",findIface->ipaddr,port);
    else if(remote_https == 1)
        sprintf(cmdbuf,"echo \"listen [%s]:%d ssl;\" >> /var/nginx/conf.d/wan.https.conf",findIface->ipaddr,port);
    system(cmdbuf);
    free_argv();
}

int insert_remote_management6(struct uci_context *ctx,struct ip6tc_handle *filter_handle)
{

    struct uci_ptr ptr;
    char *strptr=strdup("firewall.global_configuration.remote_management");
    int remote_management=0;
    FILE *fp=NULL;
    char cmdbuf[256]={'\0'};

    if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
        printf("Error in getting info about:%s\r\n",strptr);
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

        char *strptr4=strdup("firewall.global_configuration.remote_v6_ipaddress");
        if (uci_lookup_ptr(ctx, &ptr, strptr4, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr4);
            return 1;
        }

        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {
        if(ptr.o->v.string)
            strcpy(remote_ipaddress,ptr.o->v.string);
        }
        char *strptr5=strdup("firewall.global_configuration.remote_v6_end");
        if (uci_lookup_ptr(ctx, &ptr, strptr5, true) != UCI_OK) {
            printf("Error in getting info about:%s\r\n",strptr4);
            return 1;
        }
        
        if(ptr.flags & UCI_LOOKUP_COMPLETE)
        {

            if(ptr.o->v.string)
                strcpy(remote_end,ptr.o->v.string);
        }


        if(strcmp(remote_ipaddress,"") == 0)
        {
            return 1;
        }

        struct iface_data *findIface;
        struct list_head *p;

        list_for_each(p, &allIfaces) {
            findIface = list_entry (p, struct iface_data, list);
            if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
                    && (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
                    && (strncmp(findIface->ifname,"wan16",5)==0
                        || strncmp(findIface->ifname,"wan26",5)==0
                        || strncmp(findIface->ifname,"usb",3)==0
                        || strncmp(findIface->l3ifname,"6in4",4)==0
                        || strncmp(findIface->l3ifname,"6rd",3)==0)
              )
            {

               if(strcmp(findIface->l3ifname,"null") !=0)
               {
                insert_remote_management_forward_rules6(filter_handle,remote_http,remote_https,remote_management_port,
                        remote_ipaddress,remote_end,findIface);
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

        int isboot=atoi(cmdbuf);
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

void insert_wan_netconf6_input(struct ip6tc_handle *filter_handle,struct iface_data *findIface,int netconf_port)
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

        ret = do_command6(mynewargc, mynewargv, &tablename, &filter_handle,true);
        if (!ret) {
            printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
        }

        free_argv();


}

void insert_lan_netconf6_input(struct ip6tc_handle *handle,int netconf_port)
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


    ret = do_command6(mynewargc, mynewargv, &tablename, &handle,true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
    strcpy(mynewargv[12],"RETURN");
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle,true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    strcpy(mynewargv[10],"br-vlan+");
    /*
     *  When netconf is disabled on LAN, a drop rule is added to restrict access from the host 
    */
    strcpy(mynewargv[12],"DROP");
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle,true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    free_argv();

}

int insert_netconf6(struct uci_context *ctx,struct ip6tc_handle *filter_handle)
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
                        && (strncmp(findIface->ifname,"wan16",5)==0
                        || strncmp(findIface->ifname,"wan26",5)==0
                        || strncmp(findIface->ifname,"usb",3)==0
                        || strncmp(findIface->l3ifname,"6in4",4)==0
                        || strncmp(findIface->l3ifname,"6rd",3)==0)
                      )
                    {
                        insert_wan_netconf6_input(filter_handle,findIface,netconf_port); 
                    }                                
            }
        }

        if(!netconf_lan)
        {
            insert_lan_netconf6_input(filter_handle,netconf_port);
        }

        free(strptr2);
        free(strptr3);
        free(strptr4);
    }
    free(strptr1);
}

void insert_lanvpn_http_rules6(int port,struct ip6tc_handle *handle)
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

    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    //strcpy(mynewargv[10],"eth3.4094");
    sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
    strcpy(mynewargv[12],"RETURN");
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    strcpy(mynewargv[10],"br-vlan+");
    strcpy(mynewargv[12],"ACCEPT");
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    free_argv();

}
void insert_lanvpn_https_rules6(int port,struct ip6tc_handle *handle)
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

    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }
    
    //strcpy(mynewargv[10],"eth3.4094");
    sprintf(mynewargv[10],"%s.4094",gLan_phy_iface);
    strcpy(mynewargv[12],"RETURN");
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }   

    strcpy(mynewargv[10],"br-vlan+");
    strcpy(mynewargv[12],"ACCEPT");
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    free_argv();
}
int insert_lanvpn_management6(struct uci_context *ctx,struct ip6tc_handle *filter_handle)
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

            insert_lanvpn_http_rules6(lan_http_port,filter_handle);

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

            insert_lanvpn_https_rules6(lan_https_port,filter_handle);
        }


    free(strptr);
    free(strptr1);
	free(strptr2);
	free(strptr3);
    return 0;
}
void insert_block_wan_rules6(struct iface_data *iface,struct ip6tc_handle *handle)
{

    char *tablename="filter";
    int ret;
    char ifbuf[128]={'\0'};
    char ifcmd[128]={'\0'};
    FILE *fp=NULL;

    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("input_rule");
    add_argv("--protocol");
    add_argv("ipv6-icmp");
    add_argv("--icmpv6-type");
    add_argv("ping");
    add_argv("--jump");
    add_argv("ACCEPT");
    add_argv("--in-interface");
    add_argv(iface->l3ifname);
    if(strncmp(iface->l3ifname,"ppoe",4) == 0)
    {
        sprintf(ifcmd,"ifconfig %s |grep Scope:Global |awk -F ' ' '{print $3}' |awk -F '/' '{print $1}'",iface->l3ifname);
        fp=popen(ifcmd,"r");
        if(fgets(ifbuf, sizeof(ifbuf), fp) != NULL)
        {
            strcpy (iface->ipaddr, ifbuf);
        }

        iface->ipaddr[strlen(iface->ipaddr)-1]='\0';
        pclose(fp);
    }

    add_argv("--destination");
    add_argv(iface->ipaddr);
    ret = do_command6(mynewargc, mynewargv, &tablename, &handle, true);
    if (!ret) {
        printf("do_command failed.string:%s\n",ip6tc_strerror(errno));
    }

    free_argv();
}

int insert_block_wan6(struct uci_context *ctx,struct ip6tc_handle *filter_handle)
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
                    && (strncmp(findIface->ifname,"wan",3)!=0 || strncmp(findIface->ifname,"usb",3)!=0) /*A WAN or USB interface*/
                    && (strncmp(findIface->ifname,"wan16",5)==0
                        || strncmp(findIface->ifname,"wan26",5)==0
                        || strncmp(findIface->ifname,"usb",3)==0
                        || strncmp(findIface->l3ifname,"6in4",4)==0
                        || strncmp(findIface->l3ifname,"6rd",3)==0)
              )
            {

                if(strcmp(findIface->l3ifname,"null") != 0)
                {

                    insert_block_wan_rules6(findIface,filter_handle);
                }

            }
        }

    }
        return 0;
}

void flush_basicsettings6(struct ip6tc_handle *filter_handle)
{


    if(ip6tc_is_chain(strdup("input_rule"), filter_handle))
    {
        if(!ip6tc_flush_entries(strdup("input_rule"),filter_handle))
            printf("Failed in flushing rules of input_rule\r\n");
    }
    else
    {
        printf("No chain detected with input_rule name\r\n");
    }

    if(ip6tc_is_chain(strdup("syn_flood"), filter_handle))
    {
        if(!ip6tc_flush_entries(strdup("syn_flood"),filter_handle))
            printf("Failed in flushing rules of syn_flood\r\n");
    }
    else
    {
        printf("No chain detected with syn_flood name\r\n");
    }

    if(ip6tc_is_chain(strdup("udp_flood"), filter_handle))
    {
        if(!ip6tc_flush_entries(strdup("udp_flood"),filter_handle))
            printf("Failed in flushing rules of udp_flood\r\n");
    }
    else
    {
        printf("No chain detected with udp_flood name\r\n");
    }

    if(ip6tc_is_chain(strdup("icmp_flood"), filter_handle))
    {   
        if(!ip6tc_flush_entries(strdup("icmp_flood"),filter_handle))
            printf("Failed in flushing rules of icmp_flood\r\n");
    }   
    else
    {   
        printf("No chain detected with icmp_flood name\r\n");
    }   

    if(ip6tc_is_chain(strdup("ping_of_death"), filter_handle))
    {   
        if(!ip6tc_flush_entries(strdup("ping_of_death"),filter_handle))
            printf("Failed in flushing rules of ping_of_death\r\n");
    }   
    else
    {   
        printf("No chain detected with ping_of_death name\r\n");
    }   



}
int config_basicsettings6(struct uci_context *ctx)
{

    static struct ip6tc_handle *filter_handle6 = NULL;

    /*Create handle for filter table*/
    filter_handle6 = create6_handle("filter");

    flush_basicsettings6(filter_handle6);

    insert_remote_management6(ctx,filter_handle6);
    insert_flood_rules6(ctx,filter_handle6);
    insert_lanvpn_management6(ctx,filter_handle6);
    insert_block_wan6(ctx,filter_handle6);
    insert_netconf6(ctx,filter_handle6);

    int y = ip6tc_commit(filter_handle6);
    if (!y)
    {
        printf("Error commit: %s errno:%d\n", ip6tc_strerror(errno),errno);
        return -1;
    }

    ip6tc_free(filter_handle6);

    return 0;
}
