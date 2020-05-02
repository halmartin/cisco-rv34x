#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <linux/rtnetlink.h>

#include "list.h"

/*Global Declarations */
#define NL_BUFSIZE 4096
#define ADDRESS_SIZE 54
#define LABEL_SIZE 24

int WAN1_status = 0; //Assume 0 is DOWN and 1 is UP.
int WAN2_status = 0; //Assume 0 is DOWN and 1 is UP.

struct iface_ip_data {
	struct list_head list;

	char address[ADDRESS_SIZE];
	int prefix;
	char label[LABEL_SIZE];
};

struct list_head iface_ip_list;
void handle_address_update(int is_add, char *address, int prefix, char *label);
struct iface_ip_data *search_iface_ip_data(char *address, int prefix, char *label);
void parse_if_message (struct nlmsghdr *nl_msg);
void parse_if_message_link (struct nlmsghdr *nl_msg, int status);
void parse_route_message (struct nlmsghdr *nl_msg, int numArgs);
void print_complete_list(void);
/*Function declarations and prototyping*/

void print_complete_list(void)
{
	struct list_head *p;
	struct iface_ip_data *data;

	syslog(LOG_INFO, "Here are the elements after reorganisation:\r\n");
	list_for_each(p, &iface_ip_list) {
		data = list_entry (p, struct iface_ip_data, list);

		syslog(LOG_INFO, "Record with data: Address:%s prefix:%d label:%s !\r\n",
				data->address, data->prefix, data->label);
	}
}

void handle_address_update(int is_add, char *address, int prefix, char *label)
{
	if(is_add == 1)
	{ //Add case.
		if (!search_iface_ip_data(address, prefix, label))
		{ //record not found case.
			/*
			 * 1. Add new record.
			 * 2. send update if it needs to. 
			 */

			//1. Add new record.
			struct iface_ip_data *new_rec = malloc(sizeof(struct iface_ip_data));
			memset(new_rec, 0, sizeof(struct iface_ip_data));
			INIT_LIST_HEAD(&new_rec->list);

			strcpy(new_rec->address, address);
			new_rec->prefix = prefix;
			strcpy(new_rec->label, label);

			list_add_tail(&new_rec->list, &iface_ip_list);

			//Check if both v4 and v6 is sharing same session
			char ppoe_v6[32]={'\0'};
			if ((strncmp(label,"ppoe-wan1p",10) == 0) || (strncmp(label,"ppoe-wan2p",10) == 0) )
			{
				FILE *fp=NULL;
				char ppoe_cmd[48]={'\0'};
				sprintf(ppoe_cmd,"uci get network.%s.ipv6",label+5);
				fp=popen(ppoe_cmd,"r");
				if(fgets(ppoe_v6, sizeof(ppoe_v6), fp) == NULL)
				{
					printf("fgets failed\n");
					pclose(fp);
				}
				else
				{
					ppoe_v6[strlen(ppoe_v6)-1]='\0';
					pclose(fp);
				}
			}

			//2. Send update to netifd if needed.
			//This technique is needed only for WAN v6 when it is in pppoe mode.
			if ((strncmp(label, "ppoe-wan16p", 11)==0 || strncmp(label, "ppoe-wan26p", 11)==0
						|| ((ppoe_v6) && (strncmp(ppoe_v6,"1",1) == 0)))
						&& strncmp(address, "fe80", 4)!=0)
			{
				//printf("****Need for us to call NETIFD update****\r\n");
				char commandbuf[256];

				memset(commandbuf, '\0', sizeof(commandbuf));
				sprintf(commandbuf, "sh /sbin/pppoe-v6-update %s %s %d", label, address, prefix);
				//printf("EXECUTING COMMAND: %s\r\n",commandbuf);
				syslog(LOG_INFO, "PPPoE v6 interface update with data: Iface:%s Address:%s/%d\n",label, address, prefix);
				system(commandbuf);
			}
		}
		/*else
		{
			printf("Record with data: Address:%s prefix:%d label:%s already present. Hence ignoring!\r\n", \
				address, prefix, label);
		}*/
	}
	else
	{ //Del case.
		struct iface_ip_data *new_rec=NULL;
		new_rec = search_iface_ip_data(address, prefix, label);
		if(new_rec)
			list_del(new_rec);
	}

	//Debug
	//print_complete_list();
}

struct iface_ip_data *search_iface_ip_data(char *address, int prefix, char *label)
{
	struct list_head *p;
	struct list_head *found=NULL;
	struct iface_ip_data *data;
	int foundAt=0;
	int length=0;

	list_for_each(p, &iface_ip_list) {
		data = list_entry (p, struct iface_ip_data, list);

		//printf("Current record is Address:%s prefix:%d label:%s\r\n",data->address, data->prefix, data->label);
		if(strcmp(data->address, address)==0 
			&& data->prefix == prefix
			&& strcmp(data->label, label)==0)
		{
			found = p;
			foundAt = length;
			break;
		}
		length++;
	}

	if(foundAt > 6)
	{ /*This is under an assumption that the nodes that are requested for be at near vicinity from head node.
	   * But why 6: 3VLANs * 2ips = 6 + 1 for WAN v4. 
	   */
		list_del(found);
		list_add(found,&iface_ip_list);
	}

	if (found)
		return list_entry (found, struct iface_ip_data, list);
	else
		return NULL;
}

void parse_if_message (struct nlmsghdr *nl_msg)
{
	struct ifaddrmsg *if_msg;
	struct rtattr    *attrib;
	int len;

	char address[ADDRESS_SIZE];
	char label[LABEL_SIZE];

	if_msg = (struct ifaddrmsg *) NLMSG_DATA (nl_msg);

	//printf("Inside the function:parse_if_message. FAMILY:%d AF_INET:%d AF_INET6:%d\r\n", if_msg->ifa_family, AF_INET, AF_INET6);

	switch (if_msg->ifa_family)
	{
		case AF_INET:
		case AF_INET6:
			break;
		default:
			return;
	}

	memset(address, '\0', sizeof(address));
	memset(label, '\0', sizeof(label));

	len = IFA_PAYLOAD (nl_msg);
	for (attrib = IFA_RTA (if_msg); RTA_OK (attrib, len);attrib = RTA_NEXT (attrib, len))
	{
		switch (attrib->rta_type)
		{
			case IFA_ADDRESS:
			case IFA_LOCAL:
				inet_ntop (if_msg->ifa_family, RTA_DATA (attrib), address, sizeof (address));
				break;

			case IFA_LABEL:
				strcpy(label, (char *) RTA_DATA (attrib));
				break;

			default:
				/* ignore all other attributes */
				//printf("Unhandled attribute:%d IFA_ADDRESS:%d\r\n",attrib->rta_type, IFA_ADDRESS);
				break;
		}
	}

	if(!label[0])
	{
		if_indextoname(if_msg->ifa_index, label);
	}

	/* if we got both a label and an IP address */
	if (label[0] && address[0])
	{
		switch (nl_msg->nlmsg_type)
		{
			case RTM_NEWADDR:
				//fprintf (stderr, "ROUTE UPDATE: ADD: %s/%d to %s\n", address, if_msg->ifa_prefixlen, label);
				handle_address_update(1, address, if_msg->ifa_prefixlen, label);
				break;
			case RTM_DELADDR:
				//fprintf (stderr, "ROUTE UPDATE: DEL: %s/%d from %s\n", address, if_msg->ifa_prefixlen, label);
				handle_address_update(0, address, if_msg->ifa_prefixlen, label);
				break;
			default:
				/* ignore all other messages */
				break;
		}
	}

	return;
}

void parse_if_message_link (struct nlmsghdr *nl_msg, int status)
{
	int len = nl_msg->nlmsg_len - sizeof(*nl_msg);
	struct ifinfomsg *ifi;
	char buffer[256]="";  //Potential BUG
	
	if (sizeof(*ifi) > (size_t) len) {
		//fprintf (stderr, "Got a short RTM_NEWLINK message\n");
		syslog(LOG_ERR, "Got a short RTM_NEWLINK message\n");
		return;
	}
	
	ifi = (struct ifinfomsg *)NLMSG_DATA(nl_msg);
	if ((ifi->ifi_flags & IFF_LOOPBACK) != 0) {
		//fprintf(stderr,"Message about loopback. Don't bother");
		return;
	}
	
	struct rtattr *rta = (struct rtattr *)
	((char *) ifi + NLMSG_ALIGN(sizeof(*ifi)));
	len = NLMSG_PAYLOAD(nl_msg, sizeof(*ifi));
	
	while(RTA_OK(rta, len)) {
		switch(rta->rta_type) {
			case IFLA_IFNAME:
				//NOTE: This is applicable only for PP. Even if it is a VLAN on WAN this should work.
				if (strncmp((char *) RTA_DATA(rta), "eth0", 4) == 0)
				{
					if (status == 1 && WAN2_status == 0)
					{
						WAN2_status = 1;
						snprintf(buffer, sizeof(buffer), "sh /sbin/link-hotplug %s UP &",(char *) RTA_DATA(rta));
						system(buffer);
					}
					if (status == 0 && WAN2_status == 1)
					{
						WAN2_status = 0;
					}
				}
				else if (strncmp((char *) RTA_DATA(rta), "eth2", 4) == 0)
				{
					if (status == 1 && WAN1_status == 0)
					{
						WAN1_status = 1;
						snprintf(buffer, sizeof(buffer), "sh /sbin/link-hotplug %s UP &",(char *) RTA_DATA(rta));
						system(buffer);
					}
					if (status == 0 && WAN1_status == 1)
					{
						WAN1_status = 0;
					}
				}
				//syslog(LOG_INFO, "TESTING: WAN1_status:%d WAN2_status:%d status:%d ifname:%s\n", WAN1_status, WAN2_status, status, (char *) RTA_DATA(rta));
			break;
		}
		rta = RTA_NEXT(rta, len);
	}
}

void parse_route_message (struct nlmsghdr *nl_msg, int numArgs)
{
	struct	rtmsg *route_entry;
	struct	rtattr *route_attribute;
	int	route_attribute_len = 0;
	unsigned char route_netmask = 0;
	char	destination_address[32];
	char 	gateway_address[32];
	char 	routebuf[256];

	bzero(destination_address, sizeof(destination_address));
	bzero(gateway_address, sizeof(gateway_address));
	bzero(routebuf, sizeof(routebuf));
	/* Get the route data */
	route_entry = (struct rtmsg *) NLMSG_DATA(nl_msg);

	/* We are just intrested in main routing table or table 220*/
	if (route_entry->rtm_table != RT_TABLE_MAIN && route_entry->rtm_table != 220)
		return;

#if 1
	/* Get attributes of route_entry */
	route_attribute = (struct rtattr *) RTM_RTA(route_entry);
	route_netmask = route_entry->rtm_dst_len;
	/* Get the route atttibutes len */
	route_attribute_len = RTM_PAYLOAD(nl_msg);
	/* Loop through all attributes */
	for ( ; RTA_OK(route_attribute, route_attribute_len); \
		route_attribute = RTA_NEXT(route_attribute, route_attribute_len))
	{
		/* Get the destination address */
		if (route_attribute->rta_type == RTA_DST)
		{
			inet_ntop(AF_INET, RTA_DATA(route_attribute), \
				destination_address, sizeof(destination_address));
		}
		/* Get the gateway (Next hop) */
		if (route_attribute->rta_type == RTA_GATEWAY)
		{
			inet_ntop(AF_INET, RTA_DATA(route_attribute), \
				gateway_address, sizeof(gateway_address));
		}
	}
#endif

	/* Now we can dump the routing attributes */
	if (nl_msg->nlmsg_type == RTM_DELROUTE)
	{
		if (route_entry->rtm_table == 220)
			return;
		/* handle default routes for rip*/
		if ( strcmp(destination_address,"") == 0 || route_netmask == 0 )
		{
			snprintf(routebuf, sizeof(routebuf), "sh /sbin/route-del");
			system(routebuf);
			return;
		}

		if(numArgs == 1)
		{
			snprintf(routebuf, sizeof(routebuf), "iptables -w -D mwan3_connected -t " \
				"mangle -d %s/%d -j MARK --set-xmark 0xff00/0xff00", \
				destination_address, route_netmask);
			system(routebuf);
		}
		/*fprintf(stdout, "Deleting route to destination --> %s netmask --> %d and gateway %s\n", \
			destination_address, route_netmask, gateway_address);
		system ("/etc/init.d/mwan3 route_update"); */
	}
	else if (nl_msg->nlmsg_type == RTM_NEWROUTE)
	{
		if (route_entry->rtm_table == 220)
		{
			snprintf(routebuf,sizeof(routebuf), "ip route del default table 220");
			system(routebuf);
			return;
		}
		/* handle default routes for rip*/
		if ( strcmp(destination_address,"") == 0 || route_netmask == 0 )
		{
			snprintf(routebuf, sizeof(routebuf), "vtysh -c \"conf t\" -c \"router rip\" " \
				"-c \"default-information originate\" -c \"exit\" -c \"exit\" -c \"exit\"");
			system(routebuf);
			return;
		}

		if(numArgs == 1)
		{
			snprintf(routebuf, sizeof(routebuf), "iptables -w -A mwan3_connected -t mangle -d %s/%d" \
				" -j MARK --set-xmark 0xff00/0xff00", destination_address, route_netmask);
			system(routebuf);
		}
		/*fprintf(stdout,"Adding route to destination --> %s netmask --> %d and gateway %s\n", \
			destination_address, route_netmask, gateway_address);
		system ("/etc/init.d/mwan3 route_update");*/
	}
}

int main (int argc, char *argv[])
{
	struct pollfd		pollfd;
	struct nlmsghdr		*nl_msg;
	struct sockaddr_nl	local;
	int 	fd, len, linkStatus;
	char 	buf[NL_BUFSIZE];

	// Standars i/p o/p error will be redirected to /dev/null automatically.
	daemon(1,0);
	syslog(LOG_INFO, "rtupd is now daemonized...!\r\n");
	/* Create Socket */
	if ((fd = socket (PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0)
	{
		perror ("Socket Creation: ");
		return -1;
	}

	memset (&local, 0, sizeof (local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_IFADDR;

	if (bind (fd, (struct sockaddr*) &local, sizeof (local)) < 0)
	{
		perror("Cannot bind netlink socket");
		return -1;
	}

	if (fcntl (fd, F_SETFL, O_NONBLOCK))
	{
		perror ("fcntl O_NONBLOCK");
		return -1;
	}

	memset (buf, 0, NL_BUFSIZE);
	nl_msg = (struct nlmsghdr *)buf;

	/* For getting interface addresses */
	nl_msg->nlmsg_len   = NLMSG_LENGTH (sizeof (struct ifaddrmsg));
	nl_msg->nlmsg_type  = RTM_GETADDR;
	nl_msg->nlmsg_flags = NLM_F_ROOT | NLM_F_REQUEST;
	nl_msg->nlmsg_pid   = getpid ();

	if (write (fd, nl_msg, nl_msg->nlmsg_len) < 0)
	{
		//fprintf (stderr, "Write To Socket Failed...\n");
		syslog(LOG_ERR, "Write To Socket Failed...\n");
		return -1;
	}

	INIT_LIST_HEAD(&iface_ip_list);

	while (1)
	{
		pollfd.fd = fd;
		pollfd.events = POLLIN;
		pollfd.revents = 0;

		if (poll (&pollfd, 1, 20000))
		{
			if ((len = recv (fd, buf, NL_BUFSIZE, 0)) < 0)
			{
				//fprintf (stderr, "Read From Socket Failed... %s \n", strerror(errno));
				syslog(LOG_ERR, "Read From Socket Failed... %s \n", strerror(errno));
				continue;
				//if (errno != EAGAIN)
				// return -1;
			}

			for (nl_msg = (struct nlmsghdr *) buf;
				NLMSG_OK (nl_msg, len);
				nl_msg = NLMSG_NEXT (nl_msg, len))
			{
				switch (nl_msg->nlmsg_type)
				{
				case RTM_NEWADDR:
				case RTM_DELADDR:
					parse_if_message (nl_msg);
					break;
				case RTM_NEWLINK:
					{
						struct ifinfomsg *ifi;
						ifi = (struct ifinfomsg *) NLMSG_DATA (nl_msg);
						if (ifi->ifi_flags & IFF_RUNNING)
						{
							//fprintf (stderr, "Link active\n");
							linkStatus=1;
						}
						else
						{
							//fprintf (stderr, "Link inactive\n");
							linkStatus=0;
						}
						parse_if_message_link (nl_msg,linkStatus);
					}
					break;
				case RTM_DELLINK:
					//fprintf (stderr, "Link down. This is usually for a complete interface deletion, like in pppX interfaces.\n");
					break;
				case NLMSG_DONE:
					break;
				case RTM_NEWROUTE:
				case RTM_DELROUTE:
					parse_route_message(nl_msg, argc);
					break;
				default:
					//fprintf (stderr, "unhandled message (%d)\n", nl_msg->nlmsg_type);
					break;
				}
			}
		}
	}

	close (fd);
	return 0;
}
