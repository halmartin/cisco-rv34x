#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
//#include "md5.h"

int md5_checksum_validate(char *pInSum, char *filename);

int md5_checksum_validate(char *pInSum, char *filename)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    char chksm[MD5_DIGEST_LENGTH +1];
    
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[1024];

    if (inFile == NULL) {
        printf ("%s can't be opened.\n", filename);
	return -1;
    }
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, 1024, inFile)) != 0)
        MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    memset(chksm, 0, sizeof(chksm));
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) sprintf(chksm+(i*2),"%02x", c[i]);

    fclose (inFile);
    
    //printf ("\n chksm = %s\n", chksm);
    //printf ("\n pInSum = %s\n", pInSum);

    if(strcmp(pInSum, chksm) !=0)
      return -1;

    return 0;
}
