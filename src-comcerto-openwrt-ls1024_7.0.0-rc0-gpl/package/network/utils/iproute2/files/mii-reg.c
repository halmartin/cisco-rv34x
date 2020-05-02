/*
 * mii-reg.c - Tim Harvey <tharvey@gateworks.com>
 *
 * Simple example of how to read/write MII registers using
 * SIOCGMIIREG/SIOCSMIIREG ioctl
 *
 * Note that your network device driver must support mdio_read/mdio_write
 *
 * Usage:
 *   mii-reg <iface> <reg> [<value>]
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/mii.h>
#include <linux/sockios.h>

int main(int argc, char **argv)
{
	char *iface;
	int sock;
	struct ifreq ifr;
	struct mii_ioctl_data *mii = (struct mii_ioctl_data*)(&ifr.ifr_data);
	int reg;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <iface> <reg> [<value>]\n",
			argv[0]);
		exit(1);
	}
	iface = argv[1];
	reg = strtol(argv[2], NULL, 0);

	/* open socket */
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(1);
	}

	/* get the details of the iface */
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, iface, IFNAMSIZ);
	if (ioctl(sock, SIOCGMIIPHY, &ifr) < 0) {
		perror("SIOCGMIIPHY");
		exit(1);
	}

	mii->reg_num = reg;
	if (argc > 3) {
		mii->val_in = strtol(argv[3], NULL, 0);
		if (ioctl(sock, SIOCSMIIREG, &ifr) < 0) {
			perror("SIOCSMIIREG");
			exit(1);
		}
	}
	mii->val_in = 0;
	mii->val_out = 0;
	if (ioctl(sock, SIOCGMIIREG, &ifr) < 0) {
		perror("SIOCGMIIREG");
		exit(1);
	}
	printf("%s phyid=0x%02x reg=0x%02x val=0x%04x\n",
		iface, mii->phy_id, mii->reg_num, mii->val_out);

	/* read */
	close(sock);

	return 0;
}
