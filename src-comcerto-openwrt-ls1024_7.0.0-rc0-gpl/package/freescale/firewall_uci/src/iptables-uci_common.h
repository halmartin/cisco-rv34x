#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ucimap.h>
#include <signal.h>
#include "list.h"
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define WAN1_PHY_IFACE "eth2"
#define WAN2_PHY_IFACE "eth0"
#define WAN1_VLAN_PHY_IFACE "eth2.%s" // %s is needed to further replace it with the VLAN ID.
#define WAN2_VLAN_PHY_IFACE "eth0.%s" // %s is needed to further replace it with the VLAN ID.
#define LAN_VLAN_PHY_IFACE "eth3.%s" // %s is needed to further replace it with the VLAN ID.
#define TMP_FWIFACESTATS "/tmp/fwifacestats"

#ifdef IPTABLES_1_4_21
#	define do_command(a,b,c,d) do_command4(a,b,c,d,true)
//#	define do_command6(a,b,c,d) do_command6(a,b,c,d,true)
#endif

char gWan1_phy_iface[24];   //These represent only the physical ifnames.
char gLan_phy_iface[24];

struct iface_data {
	struct list_head list;

	char ifname[24];
	int status;
	char ipaddr[48];
	int subnet;
	char l2ifname[24];
	char l3ifname[24];
};

struct iface_data *search_iface(char * iface_name);
void readIfaceFile(void);
void print_allIfaces(void);

int add_argv(char *what);
void free_argv(void);
void print_argv(void);

void sig_handler(int received_signal);
