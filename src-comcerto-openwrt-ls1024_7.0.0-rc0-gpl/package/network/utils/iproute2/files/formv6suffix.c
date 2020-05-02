#include <stdio.h>
#include <netinet/in.h>

//Example: formv6suffix 64 ffff 56
//output: 0:0:0:ff:0:0:0:0
union final32
{
	uint32_t __u6_addr32;
	uint16_t __u6_addr16[2];
};

unsigned int power(unsigned int a, unsigned int b)
{
	if (b == 0)
		return 1;
	else if (b%2 == 0)
		return power(a, b/2) * power(a, b/2);
	else
		return a * power(a, b/2) * power(a, b/2);
}

int main(int argc, char* argv[])
{
	struct in6_addr finaladdr={0};
	unsigned int feed;
	unsigned int prefix; //128 - 68(actual prefix)
	unsigned int andWith;
	unsigned int suffix;


	if (argc != 4)
	{
		printf("Help: %s <prefix> <data> <suffix>\nprefix in decimal format\ndata in hex format\nsuffix in decimal format\n",argv[0]);
		return 1;
	}
	prefix = 128 - strtol(argv[1], NULL, 10);
	suffix = 128 - strtol(argv[3], NULL, 10);

	unsigned int a = power(2 ,(suffix % 32)) - 1;
	unsigned int b = power(2 ,(prefix % 32)) - 1;
	andWith = a - b;
	//printf("andwith:%x\n", andWith);

	//int remainder = prefix % 16;
	//int which16 = prefix / 16;
	union final32 final;

	feed = strtol(argv[2], NULL, 16);

	//if (remainder >= 0) 
		final.__u6_addr32 = (feed << (prefix % 16));

	final.__u6_addr32 = (final.__u6_addr32 & andWith);

	//printf("final.__u6_addr32 :%x\n", final.__u6_addr32);

	//printf("feed:%d \nremainder:%d \nfinal:%d \nfinalpart1:%d \nfinalpart2:%d \nwhich16:%d\n", feed, remainder, final.__u6_addr32, final.__u6_addr16[1], final.__u6_addr16[0], which16);

	//finaladdr.__in6_u.__u6_addr16[which16] |= final.__u6_addr16[0];
	//finaladdr.__in6_u.__u6_addr16[which16+1] |= final.__u6_addr16[1];


	finaladdr.__in6_u.__u6_addr16[prefix / 16] |= final.__u6_addr16[0];
	finaladdr.__in6_u.__u6_addr16[(prefix/16) + 1] |= final.__u6_addr16[1];

	//printf("Final V6 address: %x:%x:%x:%x:%x:%x:%x:%x\n",finaladdr.__in6_u.__u6_addr16[7],
	printf("%x:%x:%x:%x:%x:%x:%x:%x\n",finaladdr.__in6_u.__u6_addr16[7],
								finaladdr.__in6_u.__u6_addr16[6], finaladdr.__in6_u.__u6_addr16[5], finaladdr.__in6_u.__u6_addr16[4],
								finaladdr.__in6_u.__u6_addr16[3],finaladdr.__in6_u.__u6_addr16[2],finaladdr.__in6_u.__u6_addr16[1],finaladdr.__in6_u.__u6_addr16[0]);

	return 0;
}
