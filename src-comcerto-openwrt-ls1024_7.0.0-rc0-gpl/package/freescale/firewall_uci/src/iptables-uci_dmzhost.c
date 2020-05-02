/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

struct list_head allIfaces;
struct list_head dmzhosts;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;
//static char * tablename="filter";
//static char * nat_tablename="nat";

static int insert_dmzhost_rules(struct uci_dmzhost *dh, struct uci_context *ctx, struct iptc_handle *handle_filter,struct iptc_handle *handle_nat)
{
	struct iface_data *findIface;
	struct list_head *p;
	char natRule[2048]="";
	char filterRule[2048]="";
    char dmzhost_reflection[256]="";

	if(dh->status == 1)
	{
		list_for_each(p, &allIfaces) {
			findIface = list_entry (p, struct iface_data, list);

            if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
            {
                printf("DMZ HOST: for the interface:%s is not configured since the interface is not active/UP\r\n",findIface->ifname);
                //return 1;
                continue;
            }
                                
			//printf("iface:%s status:%d ip4addr:%s ip4subnet:%d l2ifname:%s l3ifname:%s \r\n",data->ifname,
			//		data->status, data->ipaddr, data->subnet, data->l2ifname, data->l3ifname);
			if(findIface->status /*If status is 1, only then process it because it will have a valid IP and l3ifname*/
				&& (strncmp(findIface->ifname,"wan",3)==0 || strncmp(findIface->ifname,"usb",3)==0) /*A WAN or USB interface*/
				&& (strncmp(findIface->ifname,"wan16",5)!=0 
					&& strncmp(findIface->ifname,"wan26",5)!=0
                    && strncmp(findIface->l3ifname,"6in4",4)!=0
                    && strncmp(findIface->l3ifname,"6rd",3)!=0) 
				) //A Valid active WAN interface
			{

                /*Rule to skip pptp packets */
                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p tcp --dport 1723"
                                    ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p udp --dport 500"
                                    ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p udp --dport 4500"
                                ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p esp"
                                            ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p ah"
                            ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p 47"
                                ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p 108"
                                    ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

                memset(natRule, '\0', sizeof(natRule));
                sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -i %s -d %s -j RETURN -p udp --dport 161"
                                        ,findIface->l3ifname,findIface->ipaddr);
                tokenize_and_docommand(natRule,"nat",handle_nat);

#ifdef OPENVPN_SUPPORT
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

                    //printf("nagesh %s %s\n",protocol,ptr.o->v.string);
                    char *strptr2=strdup("openvpn.global.port");
                    if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
                        printf("Error in getting info about:%s\r\n",strptr2);
                    }

                    if(ptr.o->v.string)
                        port=atoi(ptr.o->v.string);
                
                    memset(natRule, '\0', sizeof(natRule));
                    sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -j RETURN -p %s --dport %d -i %s -d %s"
                                        ,protocol,port,findIface->l3ifname,findIface->ipaddr);
                    tokenize_and_docommand(natRule,"nat",handle_nat);
                }
#else
                struct uci_ptr ptr;
                char *strptr=strdup("sslvpn.general.enable");
                int sslvpn=0,port=0;
                char protocol[256]={'\0'};
                if (uci_lookup_ptr(ctx, &ptr, strptr, true) != UCI_OK) {
                    printf(" Error in getting info about:%s\r\n",strptr);
                }

                if(ptr.o->v.string)
                    sslvpn=atoi(ptr.o->v.string);

                if(sslvpn)
                {
		            /* In BB2 SSLVPN always uses TCP */
		            strcpy(protocol,"tcp");
                    char *strptr2=strdup("sslvpn.general.gwport");
                    if (uci_lookup_ptr(ctx, &ptr, strptr2, true) != UCI_OK) {
                        printf("Error in getting info about:%s\r\n",strptr2);
                    }

                    if(ptr.o->v.string)
			            port=atoi(ptr.o->v.string);

                    memset(natRule, '\0', sizeof(natRule));
                    sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -j RETURN -p %s --dport %d -i %s -d %s"
                                        ,protocol,port,findIface->l3ifname,findIface->ipaddr);
                    tokenize_and_docommand(natRule,"nat",handle_nat);
                }

#endif
				memset(natRule, '\0', sizeof(natRule));
				sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -j MARK --set-mark 0xdfaf -i %s -d %s"
					,findIface->l3ifname,dh->ipaddr);
				tokenize_and_docommand(natRule,"nat",handle_nat);

				memset(filterRule, '\0', sizeof(filterRule));
				sprintf(filterRule,"iptables-uci -t filter -A dmzhost_forward -m mark --mark 0xdfaf -i %s -d %s -j DROP"
					,findIface->l3ifname,dh->ipaddr);
				tokenize_and_docommand(filterRule,"filter", handle_filter);

				memset(natRule, '\0', sizeof(natRule));
				sprintf(natRule,"iptables-uci -t nat -A dmzhost_dnat -j DNAT -i %s -d %s --to-destination %s"
					,findIface->l3ifname,findIface->ipaddr,dh->ipaddr);
				tokenize_and_docommand(natRule,"nat",handle_nat);

				memset(filterRule, '\0', sizeof(filterRule));
				sprintf(filterRule,"iptables-uci -t filter -A dmzhost_forward -j ACCEPT -i %s -d %s"
					,findIface->l3ifname,dh->ipaddr);
				tokenize_and_docommand(filterRule,"filter",handle_filter);

                memset(dmzhost_reflection, '\0', sizeof(dmzhost_reflection));
                sprintf(dmzhost_reflection,"iptables-uci -t nat -I dmzhost_reflection_in -j MARK --set-mark 0xdc00 -i %s.+ "
                                        "-d %s", gLan_phy_iface, findIface->ipaddr);
                tokenize_and_docommand(dmzhost_reflection, "nat", handle_nat);

                memset(dmzhost_reflection, '\0', sizeof(dmzhost_reflection));
                sprintf(dmzhost_reflection,"iptables-uci -t nat -A dmzhost_reflection_in -i %s.+ -d %s "
                                            "-j DNAT --to-destination %s", gLan_phy_iface, findIface->ipaddr,dh->ipaddr);
                tokenize_and_docommand(dmzhost_reflection,"nat",handle_nat);

                memset(dmzhost_reflection, '\0', sizeof(dmzhost_reflection));
                sprintf(dmzhost_reflection,"iptables-uci -t nat -A dmzhost_reflection_out -m mark --mark 0xdc00 -o %s.+ -d %s "
                                "-j MASQUERADE", gLan_phy_iface, dh->ipaddr);
                tokenize_and_docommand(dmzhost_reflection, "nat", handle_nat);

                memset(dmzhost_reflection, '\0', sizeof(dmzhost_reflection));
                sprintf(dmzhost_reflection,"iptables-uci -t nat -I dmzhost_reflection_in -j MARK --set-mark 0xdc00 -i br-vlan+ "
                                "-d %s", findIface->ipaddr);
                tokenize_and_docommand(dmzhost_reflection, "nat", handle_nat);

                memset(dmzhost_reflection, '\0', sizeof(dmzhost_reflection));
                sprintf(dmzhost_reflection,"iptables-uci -t nat -A dmzhost_reflection_in -i br-vlan+ -d %s "
                                "-j DNAT --to-destination %s", findIface->ipaddr,dh->ipaddr);
                tokenize_and_docommand(dmzhost_reflection,"nat",handle_nat);

                memset(dmzhost_reflection, '\0', sizeof(dmzhost_reflection));
                sprintf(dmzhost_reflection,"iptables-uci -t nat -A dmzhost_reflection_out -m mark --mark 0xdc00 -o br-vlan+ -d %s "
                                "-j MASQUERADE",dh->ipaddr);
                tokenize_and_docommand(dmzhost_reflection, "nat", handle_nat);

			}
		}
	}
	return 0;
}

void config_dmzhost(struct uci_context *ctx)
{
	struct list_head *p;
	struct uci_dmzhost *dh;
	int y=0;
	struct iptc_handle *handle_filter = NULL;
	struct iptc_handle *handle_nat = NULL;

	handle_filter = create_handle("filter");
	handle_nat = create_handle("nat");
	
	if(iptc_is_chain(strdup("dmzhost_forward"), handle_filter)
		&& iptc_is_chain(strdup("dmzhost_dnat"), handle_nat) 
        && iptc_is_chain(strdup("dmzhost_reflection_in"), handle_nat) 
        && iptc_is_chain(strdup("dmzhost_reflection_out"), handle_nat))
	{
		if(!iptc_flush_entries(strdup("dmzhost_forward"),handle_filter))
			printf("Failed in flushing rules of dmzhost_forward iptables\r\n");

		if(!iptc_flush_entries(strdup("dmzhost_dnat"), handle_nat))
			printf("Failed in flushing rules of dmzhost_dnat in iptables\r\n");

        if(!iptc_flush_entries(strdup("dmzhost_reflection_in"), handle_nat))
                        printf("Failed in flushing rules of dmzhost_reflection_in in iptables\r\n");

        if(!iptc_flush_entries(strdup("dmzhost_reflection_out"), handle_nat))
                        printf("Failed in flushing rules of dmzhost_reflection_out in iptables\r\n");
	}
	else
		printf("No chain detected with the name dmzhost_dnat/dmzhost_forward for IPv4\r\n");

	list_for_each(p, &dmzhosts) {

		dh = list_entry(p, struct uci_dmzhost, list);

		if (dh->status)
			insert_dmzhost_rules(dh, ctx, handle_filter, handle_nat);
/*		else
			printf("dmzhost with the name:%s is disabled\n",dh->name);*/
	}

	y = iptc_commit(handle_filter);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle_filter);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of DMZ host(filter table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of DMZ host(filter table).\n",
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
				printf("Error commiting data: %s errno:%d in the context of DMZ host(nat table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of DMZ host(nat table).\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	iptc_free(handle_filter);
	iptc_free(handle_nat);
}
