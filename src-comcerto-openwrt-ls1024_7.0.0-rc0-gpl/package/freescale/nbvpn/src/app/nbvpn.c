/**************************************************************************
 * Copyright 2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/
/*
 * File:        nbvpn.c
 *
 * Description: NBVPN user space application
 *
 * Authors:     Sridhar Pothuganti <sridhar.pothuganti@freescale.com>
 *
 */
/* History
 *  Version     Date            Author                  Change Description
 *    1.0       19/07/2015      Sridhar Pothuganti      Initial Development
 *    1.1       22/07/2015      Chaitanya Sakinam       Full functionality implimentation
*/
/****************************************************************************/
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "sbr_ioctl.h"
#include "nbvpn_ctrl.h"

void help(char *program)
{
	printf("Help:\r\n");
	printf
	    ("%s <add/del>  <left-subnet>/<left-subnetmask>\n <right-subnet>/<right-subnetmask>\n",
	     program);
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

int main(int argc, char *argv[])
{
	nbvpn_ctrl_t Node;
	int i = 0;
	char cmd[8];
	char host_ip[16], netmask[16];
	struct in_addr tmphost, tmpmask, tmpbroadcast;

	if ((argc != 4) && (argc != 2)){
		help(argv[0]);
		return 1;
	}
	if((argc == 2) && strcasecmp("list", argv[1])){
		help(argv[0]);
		return 1;
	}

	for (i = 1; i < argc; i++) {
		switch (i) {
		case 1:
			strcpy(cmd, argv[i]);
			break;
		case 2:
			convert(argv[i], host_ip, netmask);

			inet_pton(AF_INET, host_ip, &(tmphost));
			Node.left_subnet = tmphost.s_addr;
			inet_pton(AF_INET, netmask, &(tmpmask));
			Node.left_subnet_mask = tmpmask.s_addr;
			char tmp[16], tmp2[16];
			break;
		case 3:
			memset(host_ip, 0, 16);
			memset(netmask, 0, 16);
			memset(&tmphost, 0, sizeof(struct in_addr));
			memset(&tmpmask, 0, sizeof(struct in_addr));
			convert(argv[i], host_ip, netmask);
			inet_pton(AF_INET, host_ip, &(tmphost));
			Node.right_subnet = tmphost.s_addr;
			inet_pton(AF_INET, netmask, &(tmpmask));
			Node.right_subnet_mask = tmpmask.s_addr;
			Node.right_bcst = tmphost.s_addr | ~tmpmask.s_addr;
			break;
		default:
			printf("This should not occur\r\n");
			break;
		}
	}

	int fd = open("/dev/sbr_dev", O_RDWR);
	if (fd == -1) {
		printf("Error in opening the driver\r\n");
		exit(1);
	}

	if (!strcasecmp("add", cmd)) {
		ioctl(fd, SBR_ADD_NBVPN_REC, &Node);
	} else if (!strcasecmp("del", cmd)) {
		ioctl(fd, SBR_DEL_NBVPN_REC, &Node);
	} else {
		ioctl(fd, SBR_LIST_NBVPN_REC, &Node);
	}
	close(fd);
	return 0;
}
