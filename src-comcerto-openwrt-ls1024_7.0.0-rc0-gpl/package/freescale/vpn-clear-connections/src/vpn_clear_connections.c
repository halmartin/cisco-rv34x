#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <libnetfilter_conntrack/libnetfilter_conntrack.h>

/*cth is used for getting connections and dth is used for delete connections.*/
struct nfct_handle *cth,*dth;
unsigned int left_subnet, left_subnet_mask, right_subnet, right_subnet_mask;
unsigned int counter;
//make this variable to 1 through gdb and get debug info
int debug_gdb=0;

void print_usage()
{
	fprintf(stderr, "Usage: vpn_clear_connections <Left-Subnet/Left-Subnet-mask> <Right-Subnet/Right-Subnet-mask>\n"
			"	Left-Subnet/Left-Subnet-mask: Local security group subnet and its mask. Similar for remote sec group.\n"
			"Example: vpn_clear_connections 192.168.1.0/24 192.168.2.0/24\n");
}

void convert(char *x, char *host_ip, char *finalmask)
{
	int i;
	char netmask[10];
	struct in_addr host;
	struct in_addr tmp;
	strcpy(host_ip, strtok(x, "/"));
	strcpy(netmask, strtok(NULL, "/"));

	int p = 0xFFFFFFFF;
	i = atoi(netmask);
	int mask1 = (p << (32 - i));
	tmp.s_addr = htonl(mask1);

	inet_ntop(AF_INET, &tmp, finalmask, INET_ADDRSTRLEN);
}

static int delete_cb(enum nf_conntrack_msg_type type, struct nf_conntrack *ct, void *data)
{
	int ret;
	char buf[1024];
	
	/* For a connection to be deleted it has to fall into the leftsubnet and rightsubnet of the tunnel*/

	if (!( (left_subnet == (nfct_get_attr_u32(ct, ATTR_IPV4_SRC) & left_subnet_mask))
		&& (right_subnet == (nfct_get_attr_u32(ct, ATTR_IPV4_DST) & right_subnet_mask))))
	{
		if (debug_gdb)
		{
			printf("This conneciton is NOT cleared:\r\n");
			nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);
			printf("%s\n", buf);
		}
		return NFCT_CB_CONTINUE;
	}

	ret = nfct_query(dth, NFCT_Q_DESTROY, ct);
	if (ret < 0)
	{
		perror("nfct_query failed for destroy connection");
		return NFCT_CB_CONTINUE;
	}

	if (debug_gdb)
	{
		printf("This connection is CLEARED:\r\n");
		nfct_snprintf(buf, sizeof(buf), ct, NFCT_T_UNKNOWN, NFCT_O_DEFAULT, NFCT_OF_SHOW_LAYER3);
		printf("%s\n", buf);
	}

	counter++;
	return NFCT_CB_CONTINUE;
}

int main(int argc, char **argv)
{
	int ret;
	u_int8_t family = AF_INET;
	char host_ip[16], netmask[16];
	struct in_addr tmphost, tmpmask;

	if (argc != 3)
	{
		print_usage();
		return -1;
	}

	//Handling for Leftsubnet/mask
	convert(argv[1], host_ip, netmask);
	inet_pton(AF_INET, host_ip, &(tmphost));
	left_subnet = tmphost.s_addr;
	inet_pton(AF_INET, netmask, &(tmpmask));
	left_subnet_mask = tmpmask.s_addr;
	
	//Handling for Rightsubnet/mask
	memset(host_ip, 0, 16);
	memset(netmask, 0, 16);
	memset(&tmphost, 0, sizeof(struct in_addr));
	memset(&tmpmask, 0, sizeof(struct in_addr));
	convert(argv[2], host_ip, netmask);
	inet_pton(AF_INET, host_ip, &(tmphost));
	right_subnet = tmphost.s_addr;
	inet_pton(AF_INET, netmask, &(tmpmask));
	right_subnet_mask = tmpmask.s_addr;

	dth = nfct_open(CONNTRACK, 0);
	cth = nfct_open(CONNTRACK, 0);

	if (!cth || !dth) {
		perror("nfct_open failed");
		return -1;
	}

	nfct_callback_register(cth, NFCT_T_ALL, delete_cb, NULL);
	ret = nfct_query(cth, NFCT_Q_DUMP, &family);

	printf("Clear VPN connections: ");
	if (ret == -1)
		printf("(%d)(%s)\n", ret, strerror(errno));
	else
		printf("(OK)\n");

	printf("Cleared %d matching VPN connections\r\n", counter);

	nfct_close(cth);
	nfct_close(dth);

	ret == -1 ? exit(EXIT_FAILURE) : exit(EXIT_SUCCESS);
}
