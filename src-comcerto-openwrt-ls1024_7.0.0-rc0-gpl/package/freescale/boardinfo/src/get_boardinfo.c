#include <stdio.h>
#include <string.h>
#include "rv_boardinfo_api.h"

int main(int argc, char *argv[])
{
    int r;
    unsigned char out[256];
    if(argc != 2){
        printf("Usage: get_boardinfo serial|pid|vid|part|hwver|prodname\n");
        return 0;
    }
    if(!strcmp(argv[1],"serial")){
        if(r=boardinfo_getserial(out))
            return r;
        printf("%s", out);
    }
    if(!strcmp(argv[1],"pid")){
        if(r=boardinfo_getpid(out))
            return r;
        printf("%s", out);
    }
    if(!strcmp(argv[1],"vid")){
        if(r=boardinfo_getvid(out))
            return r;
        printf("%s", out);
    }
    if(!strcmp(argv[1],"part")){
        if(r=boardinfo_getpart(out))
            return r;
        printf("%s", out);
    }
    if(!strcmp(argv[1],"hwver")){
        if(r=boardinfo_gethwver(out))
            return r;
        printf("%s", out);
    }
    if(!strcmp(argv[1],"prodname")){
        if(r=boardinfo_getprodname(out))
            return r;
        printf("%s", out);
    }

    return 0;
}
