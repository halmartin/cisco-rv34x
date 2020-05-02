#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define PID_FILE "/tmp/boardinfo_pid.txt"
#define VID_FILE "/tmp/boardinfo_vid.txt"
#define SERIAL_FILE "/tmp/boardinfo_serial.txt"
#define MAC_FILE "/tmp/boardinfo_mac.txt"
#define PARTNO_FILE "/tmp/boardinfo_partno.txt"
#define PRODNAME_FILE "/tmp/boardinfo_prodname.txt"
#define PRODSERIES_FILE "/tmp/boardinfo_prodseries.txt"
#define KEY_FILE "/tmp/serial_key.txt"

int get_key(unsigned char *keyval)
{
 int fd;
 unsigned char buffer[16]={'\0'};
 int nbytes;

 fd = open(KEY_FILE,O_RDONLY);
 if (fd == -1)
  {
    printf("Failed to open the file\n");
    return -1;
  }

  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    printf("reading failed\n");
    close(fd);
    return -1;
  }

  buffer[nbytes-1] = '\0';
  strcpy(keyval,buffer);
  close(fd);
  return 0;
}

int boardinfo_getpid(unsigned char *pid)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp=NULL;
  unsigned char var[256] = {'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
    return -1;

  fd = open(PID_FILE,O_RDONLY);
  if (fd == -1)
  {
     printf("Failed to open the file\n");
     return -1;
  }

  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    printf("reading failed\n");
    close(fd);
    return -1;
  }
  buffer[nbytes-1] = '\0';

  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var, sizeof(var), fp) == NULL)
  {
    printf("fgets failed\n");
    close(fd);
    pclose(fp);
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(pid,var);
  return 0;
}

int boardinfo_getvid(unsigned char*vid)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp;
  unsigned char var[256]={'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret != 0 )
    return -1;

 fd  = open(VID_FILE,O_RDONLY);
 if(fd == -1)
  {
    printf("reading failed\n");
    return -1;
  }
  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    close(fd);
    printf("nbytes:%d",nbytes);
    return -1;
  }
  buffer[nbytes-1] = '\0';
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var,sizeof(var),fp) == NULL)
  {
    printf("fgets2 failed\n");
    close(fd);
    pclose(fp);
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(vid,var);
  return 0;
}

int boardinfo_getserial(unsigned char *serial)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp;
  unsigned char var[256]={'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
    return -1;

  fd  = open(SERIAL_FILE,O_RDONLY);
  if(fd == -1)
  {
    printf("reading failed\n");
    return -1;
  }
  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    printf("nbytes:%d",nbytes);
    close(fd);
    return -1;
  }
  buffer[nbytes-1] = '\0';
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var, sizeof(var), fp) == NULL)
  {
    printf("fgets3 failed\n");
    close(fd);
    pclose(fp);
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(serial,var);
  return 0;
}

int boardinfo_getmac(unsigned char **mac)
{
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp = NULL;
  FILE *fp1 = NULL;
  unsigned char var[256] = {'\0'};
  int i=0;
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
    return -1;

  fp1  = fopen(MAC_FILE,"r");
  if(fp1 == NULL)
  {
  printf("reading failed\n");
  return -1;
  }
  for(i=0;i<3;i++)
  {
    memset(buffer,0,sizeof(buffer));
    if(fgets((char *)buffer,sizeof(buffer),fp1) == NULL)
    {
        if (ferror(fp1))
            {
                printf("ERROR:reading failed\n");
                fclose(fp1);
                return -1;
            }
        else
            {
                //printf("end of file reached\n");
                fclose(fp1);
                return 0;
            }
    }
    buffer[strlen(buffer)-1] = '\0';
    sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
    fp=popen(cmd,"r");
    if(fgets((char *)var, sizeof(var), fp) == NULL)
    {
      pclose(fp);
      fclose(fp1);
      printf("fgets3 failed\n");
      return -1;
    }
    var[strlen(var) -1] = '\0';
    strncpy(mac[i],var,strlen(var));
    mac[i][strlen(var)]='\0';
    pclose(fp);
  }
  fclose(fp1);
  return 0;

}

int boardinfo_getpart(unsigned char *part)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp;
  unsigned char var[256]={'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
    return -1;

  fd  = open(PARTNO_FILE,O_RDONLY);
  if(fd == -1)
  {
    printf("reading failed\n");
    return -1;
  }
  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    close(fd);
    printf("nbytes:%d",nbytes);
    return -1;
  }
  buffer[nbytes-1] = '\0';
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var, sizeof(var), fp) == NULL)
  {
    close(fd);
    pclose(fp);
    printf("fgets3 failed\n");
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(part,var);
  return 0;
}

int boardinfo_gethwver(unsigned char *hwver)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp;
  unsigned char var[256]={'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
    return -1;

  fd  = open(VID_FILE,O_RDONLY);
  if(fd == -1)
  {
    printf("reading failed\n");
    return -1;
  }
  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    close(fd);
    printf("nbytes:%d",nbytes);
    return -1;
  }
  buffer[nbytes-1] = '\0';
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var, sizeof(var), fp) == NULL)
  {
    close(fd);
    pclose(fp);
    printf("fgets3 failed\n");
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(hwver,var);
  return 0;
}

int boardinfo_getprodname(unsigned char *prodname)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  unsigned char cmd[256]={'\0'};
  FILE *fp;
  unsigned char var[256]={'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
    return -1;

  fd  = open(PRODNAME_FILE,O_RDONLY);
  if(fd == -1)
  {
    printf("reading failed\n");
    return -1;
  }
  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    close(fd);
    printf("nbytes:%d",nbytes);
    return -1;
  }
  buffer[nbytes-1] = '\0';
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var, sizeof(var), fp) == NULL)
  {
    close(fd);
    pclose(fp);
    printf("fgets3 failed\n");
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(prodname,var);
  return 0;
}

int boardinfo_getprodser(unsigned char *prodser)
{
  int fd;
  int nbytes;
  unsigned char buffer[256]={'\0'};
  char cmd[256]={'\0'};
  FILE *fp;
  unsigned char var[256]={'\0'};
  unsigned char key[16]={'\0'};
  int ret;

  ret = get_key(key);
  if(ret !=0 )
  return -1;

  fd  = open(PRODSERIES_FILE,O_RDONLY);
  if(fd == -1)
  {
    printf("reading failed\n");
    return -1;
  }
  nbytes = read(fd,buffer,sizeof(buffer));
  if (nbytes < 0)
  {
    close(fd);
    printf("nbytes:%d",nbytes);
    return -1;
  }
  buffer[nbytes-1] = '\0';
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -d -salt -pass pass:%s",buffer,key);
  fp=popen(cmd,"r");
  if(fgets((char *)var, sizeof(var), fp) == NULL)
  {
    printf("fgets3 failed\n");
    close(fd);
    pclose(fp);
    return -1;
  }
  pclose(fp);
  close(fd);
  strcpy(prodser,var);
  return 0;
}
