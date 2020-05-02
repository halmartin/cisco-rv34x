/*
 * iptables-uci.c - sample code for the ucimap/libuci library
 * 
 */
#include "iptables-uci.h"

#include "xtables.h"
#include "libiptc/libiptc.h"
#include "iptables-multi.h"

struct list_head allIfaces;
struct list_head portForwards;

/* global new argv and argc */
extern char *mynewargv[255];
extern int mynewargc;
extern int enable_debug;

void print_portForward(struct uci_portForward *pf)
{
	int a=0;

	printf("Details of argc and argv passed to do_command are:\r\n");
	for (a = 0; a < mynewargc; a++)
		printf("argv[%u]: %s\t", a, mynewargv[a]);
	printf("\r\n");

	if(pf)
		printf("Complete firewall port forward record is '%s'\n"
			"	enabled:	%s\n"
			"	protocol:	%s\n"
			"	external Port-start:%d\n"
			"	external Port-end:%d\n"
			"	internal Port-start:%d\n"
			"	internal Port-end:%d\n"
			"	interface_name:	%s\n"
			"	destination IP:	%s\n",
			pf->name,
			(pf->status ? "Yes" : "No"),
			pf->protocol,
			pf->ext_port_start,
			pf->ext_port_end,
			pf->int_port_start,
			pf->int_port_end,
			pf->interface_name,
			pf->dest_ip);
}

static int insert_port_Forwardings(struct uci_portForward *pf, struct uci_context *ctx,struct iptc_handle *handle_filter, struct iptc_handle *handle_nat)
{
	struct iface_data *findIface;
	bool need_udp_rule=0;
	struct list_head *p;

	char commandBuff[2048]="iptables-uci -t nat -A portforward_dnat --protocol";
	char forwardRule[2048]="iptables-uci -t filter -A portforward --protocol";
	char dnat_markrule[2048]="iptables-uci -t nat -I portforward_dnat -j MARK --set-mark 0xdfaf --protocol";
	char forward_markrule[2048]="iptables-uci -t filter -I portforward -m mark --mark 0xdfaf --protocol";
	char nat_reflection[2048]="";

	sprintf(commandBuff,"%s %s",commandBuff,pf->protocol);
	sprintf(forwardRule,"%s %s",forwardRule,pf->protocol);
	sprintf(dnat_markrule,"%s %s",dnat_markrule,pf->protocol);
	sprintf(forward_markrule,"%s %s",forward_markrule,pf->protocol);

	if(pf->ext_port_start && pf->ext_port_end)
	{
		if(strcmp(pf->protocol,"all")==0)
		{
			memset(commandBuff,'\0',sizeof(commandBuff));
			memset(forwardRule,'\0',sizeof(forwardRule));
			memset(dnat_markrule,'\0',sizeof(dnat_markrule));
			memset(forward_markrule,'\0',sizeof(forward_markrule));
			strcpy(commandBuff,"iptables-uci -t nat -A portforward_dnat --protocol tcp");
			strcpy(forwardRule,"iptables-uci -t filter -A portforward --protocol tcp");
			strcpy(dnat_markrule,"iptables-uci -t nat -I portforward_dnat -j MARK --set-mark 0xdfaf --protocol tcp");
			strcpy(forward_markrule,"iptables-uci -t filter -I portforward -m mark --mark 0xdfaf --protocol tcp");
			need_udp_rule=1;
		}

		if(pf->ext_port_start == pf->ext_port_end)
			sprintf(commandBuff,"%s --dport %d",commandBuff,pf->ext_port_start);
		else
			sprintf(commandBuff,"%s --dport %d:%d",commandBuff,pf->ext_port_start,pf->ext_port_end);
	}

	sprintf(commandBuff,"%s -j DNAT --to-destination %s",commandBuff,pf->dest_ip);
	sprintf(forwardRule,"%s -j ACCEPT -d %s",forwardRule,pf->dest_ip);
	sprintf(dnat_markrule,"%s -d %s",dnat_markrule,pf->dest_ip);
	sprintf(forward_markrule,"%s -d %s -j DROP",forward_markrule,pf->dest_ip);
	
	if(pf->int_port_start && pf->int_port_end)
	{
		if (pf->int_port_start == pf->int_port_end)
		{
			sprintf(commandBuff,"%s:%d",commandBuff,pf->int_port_start);
			sprintf(forwardRule,"%s --dport %d",forwardRule,pf->int_port_start);
			sprintf(dnat_markrule,"%s --dport %d",dnat_markrule,pf->int_port_start);
			sprintf(forward_markrule,"%s --dport %d",forward_markrule,pf->int_port_start);
		}
		else
		{
			sprintf(commandBuff,"%s:%d-%d",commandBuff,pf->int_port_start,pf->int_port_end);
			sprintf(forwardRule,"%s --dport %d:%d",forwardRule,pf->int_port_start, pf->int_port_end);
			sprintf(dnat_markrule,"%s --dport %d:%d",dnat_markrule,pf->int_port_start, pf->int_port_end);
			sprintf(forward_markrule,"%s --dport %d:%d",forward_markrule,pf->int_port_start, pf->int_port_end);
		}
	}

	if(strcmp(pf->interface_name,"any"))
	{//Not any.
		findIface = search_iface(pf->interface_name);
		if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
		{
			printf("port forward for the interface:%s is not configured since the interface is not active/UP\r\n",pf->interface_name);
			return 1;
		}
		sprintf(commandBuff,"%s -i %s -d %s",commandBuff, findIface->l3ifname, findIface->ipaddr);
		sprintf(forwardRule,"%s -i %s",forwardRule,findIface->l3ifname);
		sprintf(dnat_markrule,"%s -i %s",dnat_markrule, findIface->l3ifname);
		sprintf(forward_markrule,"%s -i %s",forward_markrule, findIface->l3ifname);

// prototype:	tokenize_and_docommand(char *cmdBuff,char *table, struct iptc_handle *handle);
		tokenize_and_docommand(commandBuff, "nat", handle_nat);
		tokenize_and_docommand(forwardRule, "filter", handle_filter);
		tokenize_and_docommand(dnat_markrule, "nat", handle_nat);
		tokenize_and_docommand(forward_markrule, "filter", handle_filter);
		
		//UDP rule
		if(need_udp_rule)
		{
			char udprule[2048]="";
//prototype:	strreplace(char *outputbuff,char * original_buff,char *search_string,char * replace_with);
			strreplace(udprule, commandBuff, "protocol tcp", "protocol udp");
			tokenize_and_docommand(udprule, "nat", handle_nat);

			memset(udprule,'\0',sizeof(udprule));
			strreplace(udprule, forwardRule, "protocol tcp", "protocol udp");
			tokenize_and_docommand(udprule, "filter", handle_filter);

			memset(udprule,'\0',sizeof(udprule));
			strreplace(udprule, dnat_markrule, "protocol tcp", "protocol udp");
			tokenize_and_docommand(udprule, "nat", handle_nat);

			memset(udprule,'\0',sizeof(udprule));
			strreplace(udprule, forward_markrule, "protocol tcp", "protocol udp");
			tokenize_and_docommand(udprule, "filter", handle_filter);
		}

		//Nat reflection rules
		if(strcmp(pf->protocol,"all")!=0)
		{//Not all.
			sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
					"-d %s -p %s --dport %d:%d", gLan_phy_iface, findIface->ipaddr, pf->protocol, pf->ext_port_start,pf->ext_port_end);
			tokenize_and_docommand(nat_reflection, "nat", handle_nat);

			memset(nat_reflection, '\0', sizeof(nat_reflection));
			sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ -d %s -p %s --dport %d:%d "
					"-j DNAT --to-destination %s:%d-%d", gLan_phy_iface, findIface->ipaddr, pf->protocol, pf->ext_port_start,
								pf->ext_port_end, pf->dest_ip, pf->int_port_start, pf->int_port_end);
			tokenize_and_docommand(nat_reflection, "nat", handle_nat);

			memset(nat_reflection, '\0', sizeof(nat_reflection));
			sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -m mark --mark 0xee00 -o %s.+ -d %s -p %s "
					"--dport %d:%d -j MASQUERADE", gLan_phy_iface, pf->dest_ip, pf->protocol, pf->int_port_start,pf->int_port_end);
			tokenize_and_docommand(nat_reflection, "nat", handle_nat);

			//For br-vlan
			memset(nat_reflection, '\0', sizeof(nat_reflection));
			sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
					"-d %s -p %s --dport %d:%d", findIface->ipaddr, pf->protocol, pf->ext_port_start,pf->ext_port_end);
			tokenize_and_docommand(nat_reflection, "nat", handle_nat);

			memset(nat_reflection, '\0', sizeof(nat_reflection));
			sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ -d %s -p %s --dport %d:%d "
					"-j DNAT --to-destination %s:%d-%d", findIface->ipaddr, pf->protocol, pf->ext_port_start,pf->ext_port_end,
								pf->dest_ip, pf->int_port_start, pf->int_port_end);
			tokenize_and_docommand(nat_reflection, "nat", handle_nat);

			memset(nat_reflection, '\0', sizeof(nat_reflection));
			sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -m mark --mark 0xee00 -o br-vlan+ -d %s -p %s "
					"--dport %d:%d -j MASQUERADE", pf->dest_ip, pf->protocol, pf->int_port_start,pf->int_port_end);
			tokenize_and_docommand(nat_reflection, "nat", handle_nat);
		}
		else
		{
            if(!(pf->int_port_start) && !(pf->int_port_end))
            {
			    sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
					"-d %s -p tcp", gLan_phy_iface, findIface->ipaddr);
			    tokenize_and_docommand(nat_reflection, "nat", handle_nat);
			
			    memset(nat_reflection, '\0', sizeof(nat_reflection));
			    sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
					"-d %s -p udp", gLan_phy_iface, findIface->ipaddr);
			    tokenize_and_docommand(nat_reflection, "nat", handle_nat);

			    //For br-vlan
			    memset(nat_reflection, '\0', sizeof(nat_reflection));
			    sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
					"-d %s -p tcp", findIface->ipaddr);
			    tokenize_and_docommand(nat_reflection, "nat", handle_nat);
			
			    memset(nat_reflection, '\0', sizeof(nat_reflection));
			    sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
					"-d %s -p udp", findIface->ipaddr);
			    tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
					"-d %s -p tcp -j DNAT --to-destination %s", gLan_phy_iface, findIface->ipaddr, pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
					"-d %s -p tcp -j MASQUERADE", gLan_phy_iface, pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
					"-d %s -p udp -j DNAT --to-destination %s", gLan_phy_iface, findIface->ipaddr, pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
					"-d %s -p udp -j MASQUERADE", gLan_phy_iface, pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
			
				//For br-vlan
				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
					"-d %s -p tcp -j DNAT --to-destination %s", findIface->ipaddr, pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
					"-d %s -p tcp -j MASQUERADE", pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
					"-d %s -p udp -j DNAT --to-destination %s", findIface->ipaddr, pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
					"-d %s -p udp -j MASQUERADE", pf->dest_ip);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
			}
			else
			{
                sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
                                    "-d %s -p tcp --dport %d:%d", gLan_phy_iface, findIface->ipaddr, pf->ext_port_start,pf->ext_port_end);
                tokenize_and_docommand(nat_reflection, "nat", handle_nat);

                memset(nat_reflection, '\0', sizeof(nat_reflection));
                sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
                                        "-d %s -p udp --dport %d:%d", gLan_phy_iface, findIface->ipaddr, pf->ext_port_start,pf->ext_port_end);
                tokenize_and_docommand(nat_reflection, "nat", handle_nat);

                //For br-vlan
                memset(nat_reflection, '\0', sizeof(nat_reflection));
                sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
                                      "-d %s -p tcp --dport %d:%d", findIface->ipaddr, pf->ext_port_start,pf->ext_port_end);
                tokenize_and_docommand(nat_reflection, "nat", handle_nat);
                
                memset(nat_reflection, '\0', sizeof(nat_reflection));
                sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
                                    "-d %s -p udp --dport %d:%d", findIface->ipaddr, pf->ext_port_start,pf->ext_port_end);
                tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
					"-d %s -p tcp --dport %d:%d -j DNAT --to-destination %s:%d-%d", gLan_phy_iface,
					findIface->ipaddr,pf->ext_port_start,pf->ext_port_end,pf->dest_ip,pf->int_port_start,pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
					"-d %s -p tcp --dport %d:%d -j MASQUERADE", gLan_phy_iface, pf->dest_ip, pf->int_port_start, pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
					"-d %s -p udp --dport %d:%d -j DNAT --to-destination %s:%d-%d", gLan_phy_iface,
					findIface->ipaddr, pf->ext_port_start,pf->ext_port_end,pf->dest_ip, pf->int_port_start,pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
					"-d %s -p udp --dport %d:%d -j MASQUERADE", gLan_phy_iface, pf->dest_ip, pf->int_port_start, pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				//For br-vlan
				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
					"-d %s -p tcp --dport %d:%d -j DNAT --to-destination %s:%d-%d",
					findIface->ipaddr,pf->ext_port_start,pf->ext_port_end,pf->dest_ip,pf->int_port_start,pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
					"-d %s -p tcp --dport %d:%d -j MASQUERADE", pf->dest_ip, pf->int_port_start, pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
					"-d %s -p udp --dport %d:%d -j DNAT --to-destination %s:%d-%d",
					findIface->ipaddr, pf->ext_port_start,pf->ext_port_end,pf->dest_ip, pf->int_port_start,pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);

				memset(nat_reflection, '\0', sizeof(nat_reflection));
				sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
					"-d %s -p udp --dport %d:%d -j MASQUERADE", pf->dest_ip, pf->int_port_start, pf->int_port_end);
				tokenize_and_docommand(nat_reflection, "nat", handle_nat);
			}
		}
	}
	else
	{
		char commandBuff_orig[2048]="",forwardRule_orig[2048]="",dnat_markrule_orig[2048]="",forward_markrule_orig[2048]="";
		strcpy(commandBuff_orig,commandBuff);
		strcpy(forwardRule_orig,forwardRule);
		strcpy(dnat_markrule_orig,dnat_markrule);
		strcpy(forward_markrule_orig,forward_markrule);

		list_for_each(p, &allIfaces) {
			findIface = list_entry (p, struct iface_data, list);

            if((!findIface) || (strcmp(findIface->l3ifname,"null")==0))
            {
                printf("port forward for the interface:%s is not configured since the interface is not active/UP\r\n",findIface->ifname);
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
                )
					 /*A v4 interface*/
				 //A Valid active WAN interface
			{
				sprintf(commandBuff,"%s -i %s -d %s",commandBuff_orig, findIface->l3ifname, findIface->ipaddr);
				sprintf(forwardRule,"%s -i %s",forwardRule_orig,findIface->l3ifname);
				sprintf(dnat_markrule,"%s -i %s",dnat_markrule_orig, findIface->l3ifname);
				sprintf(forward_markrule,"%s -i %s",forward_markrule_orig, findIface->l3ifname);

				tokenize_and_docommand(commandBuff, "nat", handle_nat);
				tokenize_and_docommand(forwardRule, "filter", handle_filter);
				tokenize_and_docommand(dnat_markrule, "nat", handle_nat);
				tokenize_and_docommand(forward_markrule, "filter", handle_filter);

				if(need_udp_rule)
				{
					char udprule[2048]="";
//prototype:	strreplace(char *outputbuff,char * original_buff,char *search_string,char * replace_with);
					strreplace(udprule, commandBuff, "protocol tcp", "protocol udp");
					tokenize_and_docommand(udprule, "nat", handle_nat);

					memset(udprule, '\0', sizeof(udprule));
					strreplace(udprule, forwardRule, "protocol tcp", "protocol udp");
					tokenize_and_docommand(udprule, "filter", handle_filter);

					memset(udprule, '\0', sizeof(udprule));
					strreplace(udprule, dnat_markrule, "protocol tcp", "protocol udp");
					tokenize_and_docommand(udprule, "nat", handle_nat);

					memset(udprule, '\0', sizeof(udprule));
					strreplace(udprule, forward_markrule, "protocol tcp", "protocol udp");
					tokenize_and_docommand(udprule, "filter", handle_filter);
				}

				//Nat reflection rules
				if(strcmp(pf->protocol,"all") != 0)
				{//Not all.
					sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
						"-d %s -p %s --dport %d:%d", gLan_phy_iface, findIface->ipaddr, pf->protocol, 
						pf->ext_port_start,pf->ext_port_end);
					tokenize_and_docommand(nat_reflection, "nat", handle_nat);

					memset(nat_reflection, '\0', sizeof(nat_reflection));
					sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ -d %s -p %s --dport %d:%d "
						"-j DNAT --to-destination %s:%d-%d", gLan_phy_iface, findIface->ipaddr, pf->protocol, pf->ext_port_start,
						pf->ext_port_end, pf->dest_ip, pf->int_port_start, pf->int_port_end);
					tokenize_and_docommand(nat_reflection, "nat", handle_nat);

					memset(nat_reflection, '\0', sizeof(nat_reflection));
					sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -m mark --mark 0xee00 -o %s.+ -d %s -p %s "
						"--dport %d:%d -j MASQUERADE", gLan_phy_iface, pf->dest_ip, pf->protocol, 
						pf->int_port_start,pf->int_port_end);
					tokenize_and_docommand(nat_reflection, "nat", handle_nat);

					//For br-vlan
					memset(nat_reflection, '\0', sizeof(nat_reflection));
					sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
						"-d %s -p %s --dport %d:%d", findIface->ipaddr, pf->protocol, pf->ext_port_start,pf->ext_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

					memset(nat_reflection, '\0', sizeof(nat_reflection));
					sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ -d %s -p %s --dport %d:%d "
						"-j DNAT --to-destination %s:%d-%d", findIface->ipaddr, pf->protocol, pf->ext_port_start,pf->ext_port_end,
						pf->dest_ip, pf->int_port_start, pf->int_port_end);
					tokenize_and_docommand(nat_reflection, "nat", handle_nat);

					memset(nat_reflection, '\0', sizeof(nat_reflection));
					sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -m mark --mark 0xee00 -o br-vlan+ -d %s -p %s "
						"--dport %d:%d -j MASQUERADE", pf->dest_ip, pf->protocol, pf->int_port_start,pf->int_port_end);
					tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				}
				else
				{
					if(!pf->int_port_start && !pf->int_port_end)
					{
                        memset(nat_reflection, '\0', sizeof(nat_reflection));
                        sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
                                                "-d %s -p tcp", gLan_phy_iface, findIface->ipaddr);
                        tokenize_and_docommand(nat_reflection, "nat", handle_nat);

                        memset(nat_reflection, '\0', sizeof(nat_reflection));
                        sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
                                                "-d %s -p udp", gLan_phy_iface, findIface->ipaddr);
                        tokenize_and_docommand(nat_reflection, "nat", handle_nat);

                        memset(nat_reflection, '\0', sizeof(nat_reflection));
                        sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
                                                "-d %s -p tcp", findIface->ipaddr);
                        tokenize_and_docommand(nat_reflection, "nat", handle_nat);

                        memset(nat_reflection, '\0', sizeof(nat_reflection));
                        sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
                        "-d %s -p udp", findIface->ipaddr);
                        tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
							"-d %s -p tcp -j DNAT --to-destination %s", gLan_phy_iface, findIface->ipaddr, pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
							"-d %s -p tcp -j MASQUERADE", gLan_phy_iface, pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
							"-d %s -p udp -j DNAT --to-destination %s", gLan_phy_iface, findIface->ipaddr, pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
							"-d %s -p udp -j MASQUERADE", gLan_phy_iface, pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						//For br-vlan
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
							"-d %s -p tcp -j DNAT --to-destination %s", findIface->ipaddr, pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
							"-d %s -p tcp -j MASQUERADE", pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
							"-d %s -p udp -j DNAT --to-destination %s", findIface->ipaddr, pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);
				
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
							"-d %s -p udp -j MASQUERADE", pf->dest_ip);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);
					}
					else
					{
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
							"-d %s -p tcp --dport %d:%d", gLan_phy_iface, findIface->ipaddr, 
							pf->ext_port_start,pf->ext_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i %s.+ "
							"-d %s -p udp --dport %d:%d", gLan_phy_iface, findIface->ipaddr, 
							pf->ext_port_start,pf->ext_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
							"-d %s -p tcp --dport %d:%d", findIface->ipaddr, pf->ext_port_start,pf->ext_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -I nat_reflection_in -j MARK --set-mark 0xee00 -i br-vlan+ "
							"-d %s -p udp --dport %d:%d", findIface->ipaddr, pf->ext_port_start,pf->ext_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
							"-d %s -p tcp --dport %d:%d -j DNAT --to-destination %s:%d-%d", gLan_phy_iface, findIface->ipaddr,
							pf->ext_port_start, pf->ext_port_end, pf->dest_ip, pf->int_port_start, pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
							"-d %s -p tcp --dport %d:%d -j MASQUERADE", gLan_phy_iface, pf->dest_ip,
							pf->int_port_start,pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i %s.+ "
							"-d %s -p udp --dport %d:%d -j DNAT --to-destination %s:%d-%d", gLan_phy_iface, findIface->ipaddr,
							pf->ext_port_start,pf->ext_port_end, pf->dest_ip, pf->int_port_start, pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o %s.+ -m mark --mark 0xee00 "
							"-d %s -p udp --dport %d:%d -j MASQUERADE", gLan_phy_iface, pf->dest_ip,pf->int_port_start,
							pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						//For br-vlan
						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
							"-d %s -p tcp --dport %d:%d -j DNAT --to-destination %s:%d-%d", findIface->ipaddr,
							pf->ext_port_start, pf->ext_port_end, pf->dest_ip, pf->int_port_start, pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
							"-d %s -p tcp --dport %d:%d -j MASQUERADE", pf->dest_ip,pf->int_port_start,pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_in -i br-vlan+ "
							"-d %s -p udp --dport %d:%d -j DNAT --to-destination %s:%d-%d", findIface->ipaddr, \
							pf->ext_port_start,pf->ext_port_end, pf->dest_ip, pf->int_port_start, pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);

						memset(nat_reflection, '\0', sizeof(nat_reflection));
						sprintf(nat_reflection,"iptables-uci -t nat -A nat_reflection_out -o br-vlan+ -m mark --mark 0xee00 "
							"-d %s -p udp --dport %d:%d -j MASQUERADE", pf->dest_ip,pf->int_port_start,pf->int_port_end);
						tokenize_and_docommand(nat_reflection, "nat", handle_nat);
					}
				}
				//Nat reflection END
			}
		}
	}
	return 0;
}

void config_portForwarding(struct uci_context *ctx)
{
	struct list_head *p;
	struct uci_portForward *pf;
	int y=0;
	struct iptc_handle *handle_filter = NULL;
	struct iptc_handle *handle_nat = NULL;

	handle_filter = create_handle("filter");
	handle_nat = create_handle("nat");
	
	if(iptc_is_chain(strdup("portforward"), handle_filter)
		&& iptc_is_chain(strdup("portforward_dnat"), handle_nat)
		&& iptc_is_chain(strdup("nat_reflection_in"), handle_nat)
		&& iptc_is_chain(strdup("nat_reflection_out"), handle_nat))
	{
		if(!iptc_flush_entries(strdup("portforward"),handle_filter))
			printf("Failed in flushing rules of portforward in iptables\r\n");

		if(!iptc_flush_entries(strdup("portforward_dnat"), handle_nat))
			printf("Failed in flushing rules of portforward_dnat in iptables\r\n");
		
		if(!iptc_flush_entries(strdup("nat_reflection_in"), handle_nat))
			printf("Failed in flushing rules of nat_reflection_in in iptables\r\n");

		if(!iptc_flush_entries(strdup("nat_reflection_out"), handle_nat))
			printf("Failed in flushing rules of nat_reflection_out in iptables\r\n");
	}
	else
		printf("No chain detected with the name portforward/portforward_dnat/nat_reflection_in/nat_reflection_out for IPv4\r\n");

	list_for_each(p, &portForwards) {

		pf = list_entry(p, struct uci_portForward, list);

		if (pf->status)
			insert_port_Forwardings(pf, ctx, handle_filter, handle_nat);
		else
			printf("port forward rule with the name:%s is disabled\n",pf->name);
	}

	y = iptc_commit(handle_filter);
	if (!y)
	{
		if(errno == 11) //Identifyed that resource unavailable error's errno is 11. So we retry again after a sec and then giveup.
		{
			sleep(1);
			y = iptc_commit(handle_filter);
			if (!y)
				printf("Error commiting data: %s errno:%d in the context of PortForward rules(filter table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of PortForward rules(filter table).\n",
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
				printf("Error commiting data: %s errno:%d in the context of PortForward rules(nat table) even after retry.\n",
					iptc_strerror(errno),errno);
		}
		else
			printf("Error commiting data for IPv4: %s errno:%d in the context of PortForward rules(nat table).\n",
				iptc_strerror(errno),errno);
		//return -1;
	}

	iptc_free(handle_filter);
	iptc_free(handle_nat);
}
