#include "iptables-uci.h"
#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"
/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;
struct list_head staticnatRules;

char * incrementIpAddr (char * ipAddr)
{
    struct sockaddr_in resAddr;
    struct in_addr tmpAddr, oneIp;
    char * result_str;

    tmpAddr.s_addr=inet_addr(ipAddr);
    oneIp.s_addr=inet_addr("0.0.0.1");

    resAddr.sin_addr.s_addr=(tmpAddr.s_addr)+(oneIp.s_addr);

    result_str = inet_ntoa(resAddr.sin_addr);
    return result_str;
}

void print_staticnat(struct uci_staticnatRule *stNAT)
{
    int a=0;

    printf("Details of argc and argv passed to do_command are:\r\n");
    for (a = 0; a < mynewargc; a++)
        printf("argv[%u]: %s\t", a, mynewargv[a]);
    printf("\r\n");

    if(stNAT)
        printf("Complete firewall static NAT record is '%s'\n"
                "   status:     %s\n"
                "   privateip:  %s\n"
                "   publicip:   %s\n"
                "   range_length:   %d\n"
                "   protocol:   %s\n"
                "   port_start: %d\n"
                "   port_end:   %d\n"
                "   interface:  %s\n",
                stNAT->name,
                (stNAT->status ? "Yes" : "No"),
                stNAT->privateip,
                stNAT->publicip,
                stNAT->range_length,
                stNAT->protocol,
                stNAT->port_start,
                stNAT->port_end,
                stNAT->interface);
}

static int insert_staticnat_dnatmark_rule(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip,char *interface)
{


    char buffer[256]={'\0'};
    char *tablename="nat";

    int needudp=0;
    int ret=0;
    mynewargc=0;
    //printf("dnatmark rule\n");
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-I");
    add_argv("staticnat_dnat");
    add_argv("--jump");
    add_argv("MARK");
    //add_argv("--set-mark");
    //add_argv("0xdfaf");

    if(!strcmp(staticnat->protocol,"all") && !staticnat->port_start && !staticnat->port_end)
    {
        add_argv("--protocol");
        add_argv("all");
    }
    else if(!strcmp(staticnat->protocol,"icmp"))
    {
        if(staticnat->icmp_type)
        {
            add_argv("--protocol");
            add_argv("icmp");
            add_argv("--icmp-type");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->icmp_type);
            add_argv(buffer);
        }
        else
        {
            add_argv("--protocol");
            add_argv("icmp");
        }

    }
    else if(!strcmp(staticnat->protocol,"ip"))
    {
        add_argv("--protocol");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->protocol_number);
        add_argv(buffer);
    }

    else if(staticnat->port_start == staticnat->port_end)
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->port_start);
        add_argv(buffer);
        if(!strcmp(staticnat->protocol,"all"))
            needudp=1;

    }
    else
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
        add_argv(buffer);
        if(!strcmp(staticnat->protocol,"all"))
            needudp=1;

    }


    add_argv("-i");
    add_argv(interface);
    add_argv("--destination");
    sprintf(buffer,"%s",privateip);
    add_argv(buffer);
    add_argv("--set-mark");
    add_argv("0xdfaf");



    if(enable_debug)
    {
       //printf("Excuting staticnat dnat mark rule\n");
       print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(needudp == 1)
    {
        strcpy(mynewargv[8],"udp");

        if(enable_debug)
        {
            printf("Excuting staticnat dnat udp mark rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

    }

    free_argv();

    return 0;
}
static int insert_staticnat_forwardmark_rule(struct uci_staticnatRule *staticnat,struct iptc_handle *filter_handle,char *publicip,char *privateip,char *interface)
{

    char buffer[256]={'\0'};
    char *tablename="filter";

    int needudp=0;
    int ret=0;
    mynewargc=0;
    //printf("forward mark rule\n");
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-I");
    add_argv("staticnat");
    add_argv("--jump");
    add_argv("DROP");


    if(!strcmp(staticnat->protocol,"all") && !staticnat->port_start && !staticnat->port_end)
    {
        add_argv("--protocol");
        add_argv("all");
    }
    else if(!strcmp(staticnat->protocol,"icmp"))
    {
        if(staticnat->icmp_type)
        {
            add_argv("--protocol");
            add_argv("icmp");
            add_argv("--icmp-type");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->icmp_type);
            add_argv(buffer);
        }
        else
        {
            add_argv("--protocol");
            add_argv("icmp");
        }

    }
    else if(!strcmp(staticnat->protocol,"ip"))
    {
        add_argv("--protocol");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->protocol_number);
        add_argv(buffer);
    }
    else if(staticnat->port_start == staticnat->port_end)
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->port_start);
        add_argv(buffer);

    }
    else
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            add_argv("tcp");
            needudp=1;
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
        add_argv(buffer);

    }


    add_argv("-i");
    add_argv(interface);
    add_argv("--destination");
    sprintf(buffer,"%s",privateip);
    add_argv(buffer);

    add_argv("-m");
    add_argv("mark");
    add_argv("--mark");
    add_argv("0xdfaf");

    if(enable_debug)
    {
        printf("Excuting staticnat forward mark rule\n");
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(needudp == 1)
    {
        strcpy(mynewargv[8],"udp");

        if(enable_debug)
        {
            printf("Excuting staticnat udp forward mark rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

    }

    free_argv();

    return 0;



}

static int insert_staticnat_forward_rule(struct uci_staticnatRule *staticnat,struct iptc_handle *filter_handle,char *publicip,char *privateip,char *interface)
{

    char buffer[256]={'\0'};
    char *tablename="filter";

    int needudp=0;
    int ret=0;
    mynewargc=0;
    //printf("forward_rule\n");
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("filter");
    add_argv("-A");
    add_argv("staticnat");
    add_argv("--jump");
    add_argv("ACCEPT");


    if(!strcmp(staticnat->protocol,"all") && !staticnat->port_start && !staticnat->port_end)
    {
        add_argv("--protocol");
        add_argv("all");
    }
    else if(!strcmp(staticnat->protocol,"icmp"))
    {
        if(staticnat->icmp_type)
        {
            add_argv("--protocol");
            add_argv("icmp");
            add_argv("--icmp-type");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->icmp_type);
            add_argv(buffer);
        }
        else
        {
            add_argv("--protocol");
            add_argv("icmp");
        }

    }
    else if(!strcmp(staticnat->protocol,"ip"))
    {
        add_argv("--protocol");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->protocol_number);
        add_argv(buffer);
    }
    else if(staticnat->port_start == staticnat->port_end)
    {
        add_argv("--protocol");

        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->port_start);
        add_argv(buffer);

    }
    else
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            add_argv("tcp");
            needudp=1;
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
        add_argv(buffer);

    }


    add_argv("-i");
    add_argv(interface);
    add_argv("--destination");
    sprintf(buffer,"%s",privateip);
    add_argv(buffer);

    if(enable_debug)
    {
        printf("Excuting staticnat forward rule\n");
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(needudp == 1)
    {
        strcpy(mynewargv[8],"udp");

        if(enable_debug)
        {
            printf("Excuting staticnat forward udp rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &filter_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

    }

    free_argv();

    return 0;


}

static int insert_staticnat_dnat_rule(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip,char *interface)
{
    char buffer[256]={'\0'};
    char *tablename="nat";

    int needudp=0;
    int ret=0;
    mynewargc=0;
    //printf("dnat rule\n");
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("staticnat_dnat");
    add_argv("--jump");
    add_argv("DNAT");

    if(!strcmp(staticnat->protocol,"all") && !staticnat->port_start && !staticnat->port_end)
    {
        add_argv("--protocol");
        add_argv("all");
    }
    else if(!strcmp(staticnat->protocol,"icmp"))
    {
        if(staticnat->icmp_type)
        {
            add_argv("--protocol");
            add_argv("icmp");
            add_argv("--icmp-type");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->icmp_type);
            add_argv(buffer);
        }
        else
        {
            add_argv("--protocol");
            add_argv("icmp");
        }

    }
    else if(!strcmp(staticnat->protocol,"ip"))
    {
        add_argv("--protocol");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->protocol_number);
        add_argv(buffer);
    }
    else if(staticnat->port_start == staticnat->port_end)
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->port_start);
        add_argv(buffer);

    }
    else
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
        add_argv(buffer);

    }


    add_argv("-i");
    add_argv(interface);
    add_argv("--destination");
    sprintf(buffer,"%s",publicip);
    add_argv(buffer);
    add_argv("--to-destination");
    sprintf(buffer,"%s",privateip);
    add_argv(buffer);

    if(enable_debug)
    {
        printf("Excuting staticnat dnat rule\n");
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(needudp == 1)
    {
        strcpy(mynewargv[8],"udp");

        if(enable_debug)
        {
            printf("Excuting staticnat dnat udp rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

    }

    free_argv();

    return 0;
}

static int insert_staticnat_rule(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip,char *interface)
{

    char buffer[256]={'\0'};
    char *tablename="nat";

    int needudp=0;
    int ret=0;
    mynewargc=0;
    //printf("staticnat rule\n");
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("staticnat");
    add_argv("--jump");
    add_argv("SNAT");

    if(!strcmp(staticnat->protocol,"all") && !staticnat->port_start && !staticnat->port_end)
    {
        add_argv("--protocol");
        add_argv("all");
    }
    else if(!strcmp(staticnat->protocol,"icmp"))
    {
        if(staticnat->icmp_type)
        {
            add_argv("--protocol");
            add_argv("icmp");
            add_argv("--icmp-type");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->icmp_type);
            add_argv(buffer);
        }
        else
        {
            add_argv("--protocol");
            add_argv("icmp");
        }

    }
    else if(!strcmp(staticnat->protocol,"ip"))
    {
        add_argv("--protocol");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->protocol_number);
        add_argv(buffer);
    }
    else if(staticnat->port_start == staticnat->port_end)
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            add_argv("tcp");
            needudp=1;
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d",staticnat->port_start);
        add_argv(buffer);

    }
    else
    {
        add_argv("--protocol");
        if(!strcmp(staticnat->protocol,"all"))
        {
            needudp=1;
            add_argv("tcp");
        }
        else
            add_argv(staticnat->protocol);

        add_argv("--dport");
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
        add_argv(buffer);
        if(!strcmp(staticnat->protocol,"all"))
            needudp=1;

    }

    add_argv("-o");
    add_argv(interface);
    add_argv("--source");
    sprintf(buffer,"%s",privateip);
    add_argv(buffer);
    add_argv("--to-source");
    sprintf(buffer,"%s",publicip);
    add_argv(buffer);

    if(enable_debug)
    {
        printf("Excuting staticnat  rule\n");
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(needudp == 1)
    {
        strcpy(mynewargv[8],"udp");

        if(enable_debug)
        {
            printf("Excuting staticnat udp rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }

    }

    free_argv();


    return 0;
}

int staticnat_reflection_in_dnat(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip)
{

    char buffer[256]={'\0'};
    char final_lan_iface[16];
    char *tablename="nat";
    int ret=0,need_udp_rule=0;

    sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("staticnat_reflection_in");
    add_argv("--in-interface");
    add_argv(final_lan_iface);
    add_argv("--destination");
    add_argv(publicip);
    add_argv("--jump");
    add_argv("DNAT");
    add_argv("--to-destination");
    add_argv(privateip);

    if(strcmp(staticnat->protocol,"all") != 0)
    {

        if(strcmp(staticnat->protocol,"icmp") == 0)
        {

            if(staticnat->icmp_type)
            {
                add_argv("--protocol");
                add_argv("icmp");
                add_argv("--icmp-type");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d",staticnat->icmp_type);
                add_argv(buffer);
            }
            else
            {
                add_argv("--protocol");
                add_argv(staticnat->protocol);
            }

        }
        else if(!strcmp(staticnat->protocol,"ip"))
        {
            add_argv("--protocol");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->protocol_number);
            add_argv(buffer);
        }
        else if((staticnat->port_start) == (staticnat->port_end))
        {
            add_argv("--protocol");
            add_argv(staticnat->protocol);
            add_argv("--dport");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->port_start);
            add_argv(buffer);

        }
        else
        {
            add_argv("--protocol");
            add_argv(staticnat->protocol);
            add_argv("--dport");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
            add_argv(buffer);

        }

    }
    else 
    {
        if((staticnat->port_start) && (staticnat->port_end))
        {
            if((staticnat->port_start) == (staticnat->port_end))
            {
                add_argv("--protocol");
                add_argv("tcp");
                add_argv("--dport");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d",staticnat->port_start);
                add_argv(buffer);
                need_udp_rule=1;
            }
            else
            {
                add_argv("--protocol");
                add_argv("tcp");
                add_argv("--dport");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
                add_argv(buffer);
                need_udp_rule=1;
            }
        
        }
    }

    if(enable_debug)
    {
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(need_udp_rule)
    {
        strcpy(mynewargv[14],"udp");
        if(enable_debug)
        {
            printf("Excuting staticnat udp rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }
        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }
    }

    strcpy(mynewargv[6],"br-vlan+");
    if(enable_debug)
    {
        printf("Excuting staticnat udp rule for TCP&UDP\n");
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(need_udp_rule)
    {
        strcpy(mynewargv[14],"tcp");
        if(enable_debug)
        {
            printf("Excuting staticnat udp rule for TCP&UDP\n");
            print_staticnat(staticnat); //This atleast prints the args
        }
        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }
    }

    free_argv();

    return 0;
}

int staticnat_reflection_in(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip)
{
    char buffer[256]={'\0'};
    char final_lan_iface[16];
    char *tablename="nat";
    int ret=0,need_udp_rule=0;

    sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
    mynewargc=0;
    
    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-I");
    add_argv("staticnat_reflection_in");
    add_argv("--in-interface");
    add_argv(final_lan_iface);
    add_argv("-j");
    add_argv("MARK");
    add_argv("--set-mark");
    add_argv("0xcc00");
    add_argv("--destination");
    add_argv(publicip);

    if(strcmp(staticnat->protocol,"all") != 0)
    {

        if(strcmp(staticnat->protocol,"icmp") == 0)
        {

            if(staticnat->icmp_type)
            {
                add_argv("--protocol");
                add_argv("icmp");
                add_argv("--icmp-type");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d",staticnat->icmp_type);
                add_argv(buffer);
            }
            else
            {
                add_argv("--protocol");
                add_argv(staticnat->protocol);
            }

        }
        else if(!strcmp(staticnat->protocol,"ip"))
        {
            add_argv("--protocol");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->protocol_number);
            add_argv(buffer);
        }
        else if((staticnat->port_start) == (staticnat->port_end)) 
        {
            add_argv("--protocol");
            add_argv(staticnat->protocol);
            add_argv("--dport");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->port_start);
            add_argv(buffer);

        }
        else
        {
            add_argv("--protocol");
            add_argv(staticnat->protocol);
            add_argv("--dport");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
            add_argv(buffer);

        } 

    }
    else
    {
        if((staticnat->port_start) && (staticnat->port_end))
        {
            if((staticnat->port_start) == (staticnat->port_end))
            {
                add_argv("--protocol");
                add_argv("tcp");
                add_argv("--dport");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d",staticnat->port_start);
                add_argv(buffer);
                need_udp_rule=1;
            }
            else
            {
                add_argv("--protocol");
                add_argv("tcp");
                add_argv("--dport");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
                add_argv(buffer);
                need_udp_rule=1;
            }
       } 
    }


    if(enable_debug)
    {
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(need_udp_rule)
    {
        strcpy(mynewargv[14],"udp");
        if(enable_debug)
        {
            print_staticnat(staticnat); //This atleast prints the args
                                                                    }

            ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
            if (!ret) {
                    printf("do_command failed.string:%s\n",iptc_strerror(errno));
            }
    }

    strcpy(mynewargv[6],"br-vlan+");
        if(enable_debug)
        {
            print_staticnat(staticnat); //This atleast prints the args
        }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }


    if(need_udp_rule)
    {
        strcpy(mynewargv[14],"tcp");
        if(enable_debug)
        {
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }
    
    }

    free_argv();  
    staticnat_reflection_in_dnat(staticnat,nat_handle,publicip,privateip);
    return 0;
}

int staticnat_reflection_out(struct uci_staticnatRule *staticnat,struct iptc_handle *nat_handle,char *publicip,char *privateip)
{
    char buffer[256]={'\0'};
    char final_lan_iface[16];
    char *tablename="nat";
    int ret,need_udp_rule=0;

    sprintf(final_lan_iface,"%s.+",gLan_phy_iface);
    mynewargc=0;

    add_argv("newTestApp");
    add_argv("-t");
    add_argv("nat");
    add_argv("-A");
    add_argv("staticnat_reflection_out");
    add_argv("--out-interface");
    add_argv(final_lan_iface);
    add_argv("-j");
    add_argv("MASQUERADE");
    add_argv("-m");
    add_argv("mark");
    add_argv("--mark");
    add_argv("0xcc00");
    add_argv("--destination");
    add_argv(privateip);


    if(strcmp(staticnat->protocol,"all") != 0)
    {

        if(strcmp(staticnat->protocol,"icmp") == 0)
        {

            if(staticnat->icmp_type)
            {
                add_argv("--protocol");
                add_argv("icmp");
                add_argv("--icmp-type");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d",staticnat->icmp_type);
                add_argv(buffer);
            }
            else
            {
                add_argv("--protocol");
                add_argv(staticnat->protocol);
            }

        }
        else if(!strcmp(staticnat->protocol,"ip"))
        {
            add_argv("--protocol");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->protocol_number);
            add_argv(buffer);
        }
        else if((staticnat->port_start) == (staticnat->port_end))
        {
            add_argv("--protocol");
            add_argv(staticnat->protocol);
            add_argv("--dport");
            memset(buffer,0,sizeof(buffer));
            sprintf(buffer,"%d",staticnat->port_start);
            add_argv(buffer);

        }
        else
        {
            add_argv("--protocol");
            add_argv(staticnat->protocol);
            add_argv("--dport");
            sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
            add_argv(buffer);

        }

    }
    else
    {
        if((staticnat->port_start) && (staticnat->port_end))
        {
            if((staticnat->port_start) == (staticnat->port_end))
            {
                add_argv("--protocol");
                add_argv("tcp");
                add_argv("--dport");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d",staticnat->port_start);
                add_argv(buffer);
                need_udp_rule=1;
            }
            else
            {
                add_argv("--protocol");
                add_argv("tcp");
                add_argv("--dport");
                memset(buffer,0,sizeof(buffer));
                sprintf(buffer,"%d:%d",staticnat->port_start,staticnat->port_end);
                add_argv(buffer);
                need_udp_rule=1;
            }
        }
    }


    if(enable_debug)
    {
        printf("Excuting staticnat udp rule for TCP&UDP\n");
        print_staticnat(staticnat); //This atleast prints the args
    }

    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }

    if(need_udp_rule)
    {
        strcpy(mynewargv[16],"udp");
        if(enable_debug)
        {
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
                printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }
    }

    strcpy(mynewargv[6],"br-vlan+");
    ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
    if (!ret) {
        printf("do_command failed.string:%s\n",iptc_strerror(errno));
    }


    if(need_udp_rule)
    {
        strcpy(mynewargv[16],"tcp");
        if(enable_debug)
        {
            print_staticnat(staticnat); //This atleast prints the args
        }

        ret = do_command(mynewargc, mynewargv, &tablename, &nat_handle);
        if (!ret) {
            printf("do_command failed.string:%s\n",iptc_strerror(errno));
        }
    }

    free_argv();

    return 0;
}

static int
firewall_staticnat_init_rule(struct uci_map *map, void *section, struct uci_section *s)
{
        struct uci_staticnatRule *staticnat = section;

            INIT_LIST_HEAD(&staticnat->list);
            staticnat->name = s->e.name;
            staticnat->test = -1;
            return 0;
}

static int
firewall_staticnat_add_rule(struct uci_map *map, void *section)
{
        struct uci_staticnatRule *staticnat = section;

        list_add_tail(&staticnat->list, &staticnatRules);

         return 0;
}

static struct ucimap_section_data *
firewall_staticnat_allocate(struct uci_map *map, struct uci_sectionmap *sm, struct uci_section *s)
{
        struct uci_staticnatRule *p = malloc(sizeof(struct uci_staticnatRule));
            memset(p, 0, sizeof(struct uci_staticnatRule));
            return &p->map;
}

struct my_optmap {
        struct uci_optmap map;
        int test;
};

static struct my_optmap firewall_staticnat_options[] = {

                 {
                  .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, status),
                                     .type = UCIMAP_BOOL,
                                     .name = "status",
                                 }
                 },
                 {
                    .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, interface),
                            .type = UCIMAP_STRING,
                            .name = "interface",
                           }
                },
                {
                    .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, privateip),
                            .type = UCIMAP_STRING,
                            .name = "privateip",
                            }
                },
                {
                    .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, publicip),
                            .type = UCIMAP_STRING,
                            .name = "publicip",
                            }
                },
                {
                    .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, range_length),
                            .type = UCIMAP_INT,
                            .name = "range_length",
                            }
                },
                {
                    .map = { 
                            UCIMAP_OPTION(struct uci_staticnatRule, protocol),
                            .type = UCIMAP_STRING,
                            .name = "protocol",
                            }
                },   
                {
                    .map = { 
                            UCIMAP_OPTION(struct uci_staticnatRule, port_start),
                            .type = UCIMAP_INT,
                            .name = "port_start",
                            }
                },   
                {
                    .map = { 
                            UCIMAP_OPTION(struct uci_staticnatRule, port_end),
                            .type = UCIMAP_INT,
                            .name = "port_end",
                            }
                },  
                {
                    .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, icmp_type),
                            .type = UCIMAP_INT,
                            .name = "icmp_type",
                            }
                },
                {
                    .map = {
                            UCIMAP_OPTION(struct uci_staticnatRule, protocol_number),
                            .type = UCIMAP_INT,
                            .name = "protocol_number"
                            }
                }


};

static struct uci_sectionmap firewall_staticnatRules = {
      UCIMAP_SECTION(struct uci_staticnatRule, map),
      .type = "staticNat",
      .alloc = firewall_staticnat_allocate,
      .init = firewall_staticnat_init_rule,
      .add = firewall_staticnat_add_rule,
      .options = &firewall_staticnat_options[0].map,
      .n_options = ARRAY_SIZE(firewall_staticnat_options),
      .options_size = sizeof(struct my_optmap)
};

static struct uci_sectionmap *firewall_staticnat_smap[] = {
        &firewall_staticnatRules,
};

static struct uci_map firewall_staticnat_map = {
        .sections = firewall_staticnat_smap,
        .n_sections = ARRAY_SIZE(firewall_staticnat_smap),
};

void add_publicip(char *publicip,char *interface)
{
    char cmdBuff[256]={'0'};

    sprintf(cmdBuff,"ip address add %s dev %s label %s:1 2>/dev/null",publicip, interface, interface);
    system(cmdBuff);

}

void flush_staticnat(struct iptc_handle *nat_handle,struct iptc_handle *filter_handle)
{

    if(iptc_is_chain(strdup("staticnat"), nat_handle))
    {
        if(!iptc_flush_entries(strdup("staticnat"),nat_handle))
            printf("Failed in flushing rules of ifacenat\r\n");
    }
    else
        printf("No chain detected with that name\r\n");

    if(iptc_is_chain(strdup("staticnat_dnat"), nat_handle))
    {
        if(!iptc_flush_entries(strdup("staticnat_dnat"),nat_handle))
            printf("Failed in flushing rules of ifacenat\r\n");
    }
    else
        printf("No chain detected with that name\r\n");

    if(iptc_is_chain(strdup("staticnat_reflection_in"), nat_handle))
    {
        if(!iptc_flush_entries(strdup("staticnat_reflection_in"),nat_handle))
            printf("Failed in flushing rules of ifacenat\r\n");
    }
    else
        printf("No chain detected with that name\r\n");

    if(iptc_is_chain(strdup("staticnat_reflection_out"), nat_handle))
    {
        if(!iptc_flush_entries(strdup("staticnat_reflection_out"),nat_handle))
            printf("Failed in flushing rules of ifacenat\r\n");
    }
    else
        printf("No chain detected with that name\r\n");

    if(iptc_is_chain(strdup("staticnat"), filter_handle))
    {
        if(!iptc_flush_entries(strdup("staticnat"),filter_handle))
            printf("Failed in flushing rules of ifacenat\r\n");
    }
    else
        printf("No chain detected with that name\r\n");

}

int config_staticnat(void)
{
    struct iptc_handle *nat_handle=NULL;
    struct iptc_handle *filter_handle=NULL;
    struct uci_staticnatRule *staticnat=NULL;
    struct list_head *p;
    char publicipstr[32]={'\0'};
    char privateipstr[32]={'\0'};
    int i=0;
    struct uci_context *ctx;
    struct uci_package *pkg;

    INIT_LIST_HEAD(&staticnatRules);

    ctx = uci_alloc_context();
    ucimap_init(&firewall_staticnat_map);

    if (uci_load(ctx, "firewall", &pkg))
        {
                        //uci_perror(state->uci, NULL);
            fprintf(stderr, "Error: Can't load config");
            return 0;
        }
                        
                        
    ucimap_parse(&firewall_staticnat_map, pkg);
                        
    nat_handle = create_handle("nat");
    filter_handle = create_handle("filter");

    flush_staticnat(nat_handle,filter_handle);

    list_for_each(p, &staticnatRules) {

        staticnat = list_entry(p, struct uci_staticnatRule, list);
        //mynewargc = 0;

        if (staticnat->status)
        {

            struct iface_data *findIface;
            findIface = search_iface(staticnat->interface);
            if((findIface) && (strcmp(findIface->l3ifname,"null")!=0))
            {

            strcpy(publicipstr,staticnat->publicip);
            strcpy(privateipstr,staticnat->privateip);
            for(i=1;i<=(staticnat->range_length);i++)
            {
                insert_staticnat_rule(staticnat,nat_handle,publicipstr,privateipstr,findIface->l3ifname);
                insert_staticnat_dnat_rule(staticnat,nat_handle,publicipstr,privateipstr,findIface->l3ifname);
                insert_staticnat_dnatmark_rule(staticnat,nat_handle,publicipstr,privateipstr,findIface->l3ifname);
                insert_staticnat_forward_rule(staticnat,filter_handle,publicipstr,privateipstr,findIface->l3ifname);
                insert_staticnat_forwardmark_rule(staticnat,filter_handle,publicipstr,privateipstr,findIface->l3ifname);
                staticnat_reflection_in(staticnat,nat_handle,publicipstr,privateipstr);
                staticnat_reflection_out(staticnat,nat_handle,publicipstr,privateipstr);
                add_publicip(publicipstr,findIface->l3ifname); 
                strcpy(publicipstr,incrementIpAddr(publicipstr));
                strcpy(privateipstr,incrementIpAddr(privateipstr));
            }
            }

        }
        else
            printf("Rule with the name:%s is disabled\n",staticnat->name);

    }

    int y = iptc_commit(filter_handle);
    if (!y)
    {
	if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
	{
		sleep(1);
		y = iptc_commit(filter_handle);
		if (!y)
			printf("Error commiting data: %s errno:%d in the context of Static NAT addition(filter table), even after retry.\n",
				iptc_strerror(errno),errno);
	}
	else
		printf("Error commiting data: %s errno:%d in the context of Static NAT addition(filter table).\n",
			iptc_strerror(errno),errno);
        //exit(errno);
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
			printf("Error commiting data: %s errno:%d in the context of Static NAT addition(nat table), even after retry.\n",
				iptc_strerror(errno),errno);
	}
	else
		printf("Error commiting data: %s errno:%d in the context of Static NAT addition(nat table).\n",
			iptc_strerror(errno),errno);

        //exit(errno);
        return -1;
    }

    iptc_free(nat_handle);
    iptc_free(filter_handle);
   
   ucimap_cleanup(&firewall_staticnat_map);
    uci_free_context(ctx); 
    return 0;
}

