#ifdef BB2_IMAGE

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#define SIZE_PID 11  /* Max 11 Character Product ID */
#define SIZE_VID  3  /* Max  3 Character H/W Version */
#define SIZE_S_N 11  /* Max 11 Character Board Serial Number */
#define SIZE_P_N 12  /* Max 11 Character Board Serial Number */
#define SIZE_PROD_NAME 60  /* Max 11 Character Board Serial Number */
#define SIZE_PROD_SERIES 16  /* Max 11 Character Board Serial Number */
#define COUNT_MAC        3 /* 3 MAC-IDs one each for LAN, WAN-1, WAN-2 */
#define TOTAL_MAC_OCTET  6 /* No. of MAC Octets in MAC ID */

#define KEY_FILE "/tmp/serial_key.txt"

#define KEY_CR 0x0D  /* Enter Key Press */

#define OFFSET_ALPHA 0x37
#define OFFSET_DIGIT 0x30

#define RU_IMAGE_TYPE 1

typedef struct c2krv340_board_info {
  unsigned char pid[16];             /* type 1byte length 1byte data 12 Product ID */
  unsigned char vid[8];              /* H/W Version */
  unsigned char s_n[16];             /* Board Serial Number */
  unsigned char mac[3][8];           /* MAC Information */
  unsigned char p_n[16];             /* Board Serial Number */
  unsigned char prod_name[64];       /* Board Product Name */
  unsigned char prod_series[16];     /* Board Product Series */
}  __attribute__ ((__packed__)) c2krv340;

char *filename = "/dev/mtd16" ;

static int boardinfo_cipher( )
{
  int fd = -1, n_read = 0, i = 0;
  char out_str[18] = {0}, ifname[6] = {0};
  c2krv340 *board_data = NULL;
  int count;
  char buf_boarddata[512];
  int img_type=0;
  int pid_length;
  int vid_length;
  int serial_length = 0, partno_length = 0, prodseries_length = 0, prodname_length = 0, swdesc_length = 0, swobjid_length = 0;
  int j,mac_length;
  char cmd[256] = {'\0'};
  char PidTmp[128] = {0};
  char *p1 = NULL;
  char *p2 = NULL;
  char *tmpPid = NULL;

  fd = open(filename, O_RDONLY, 0);
  if (fd < 0) {
    printf("could not open %s!\n", filename);
    return -1;
  }
  board_data = (c2krv340 *)malloc(sizeof(c2krv340));
  if ( board_data == NULL )
  {
    close(fd);
    printf("malloc failed\n");
    return -1;
  }
  memset((char *)buf_boarddata, 0x00, sizeof(buf_boarddata));
  if ( (n_read = read(fd, (char *)buf_boarddata, sizeof(buf_boarddata))) < 0 )
  {
    free(board_data);
		printf("Reading failed\n");
    close(fd);
    return -1;
  }

  close(fd);

  count=2;
  pid_length = buf_boarddata[1];
  for (i=0;i<pid_length;i++)
  {
    *((char *)board_data->pid + i) = buf_boarddata[count++];
  }

  vid_length = buf_boarddata [count + 1];
  count = count+2;
  for (i=0;i<vid_length;i++)
  {
    *((char *)board_data->vid + i) = buf_boarddata[count++];
  }

  serial_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<serial_length;i++)
  {
    *((char *)board_data->s_n + i) = buf_boarddata[count++];
  }

  sprintf(cmd,"echo %s > %s",board_data->s_n,KEY_FILE);
  system(cmd);

  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_pid.txt",board_data->pid,board_data->s_n);
  system(cmd);

  strcpy (PidTmp, board_data->pid);

  sprintf(cmd,"uci set systeminfo.sysinfo.fullpid=%s",PidTmp);
  system(cmd);

  p1=p2=PidTmp;

  if (((p2 = strstr (PidTmp, "-K9")) != NULL) ||
      ((p2 = strstr (PidTmp, "-K8")) != NULL) )
  {
	p2=p2+3;
	PidTmp[p2-p1]='\0';
  }

  sprintf(cmd,"uci set systeminfo.sysinfo.pid=%s",PidTmp);
  system(cmd);

#ifdef RU_IMAGE
  if (strstr(board_data->pid, "-RU")) {
    /* RV340W-A-K8-RU */
    img_type = RU_IMAGE_TYPE;
    sprintf(cmd,"uci set systeminfo.sysinfo.region=%s","RU");
  system(cmd);
  }
#endif

  tmpPid=strtok(board_data->pid,"-");
  if(img_type == RU_IMAGE_TYPE)
  {
    /* RU Image type description */
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV340") == 0))
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV340 Dual WAN Gigabit Router'");
        system(cmd);
    }
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV345P") == 0))
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV345P Dual WAN Gigabit Router with PoE'");
        system(cmd);
    }
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV345") == 0))
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV345 Dual WAN Gigabit Router'");
        system(cmd);
    }
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV340W")) == 0)
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV340W Dual WAN Gigabit Router with Wireless'");
        system(cmd);
    }
  }
  else
  {
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV340") == 0))
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV340 Dual WAN Gigabit VPN Router'");
        system(cmd);
    }
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV345P") == 0))
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV345P Dual WAN Gigabit VPN Router with PoE'");
        system(cmd);
    }
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV345") == 0))
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV345 Dual WAN Gigabit VPN Router'");
        system(cmd);
    }
    if((tmpPid != NULL) && (strcmp(tmpPid,"RV340W")) == 0)
    {
        sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='RV340W Dual WAN Gigabit VPN Router with Wireless'");
        system(cmd);
    }
  }
  if(strncmp(board_data->pid,"RV3",3) == 0)
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_series='RV300 Series'");
      system(cmd);
  }  
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_vid.txt",board_data->vid,board_data->s_n);
  system(cmd);
  sprintf(cmd,"uci set systeminfo.sysinfo.vid=%s",board_data->vid);
  system(cmd);
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_serial.txt",board_data->s_n,board_data->s_n);
  system(cmd);
  sprintf(cmd,"uci set systeminfo.sysinfo.serial_number=%s",board_data->s_n);
  system(cmd);

  for (j = 0; j < COUNT_MAC ; j++)
  {
    mac_length=buf_boarddata[count+1];
    count = count+2;
    for (i=0;i<mac_length;i++)
    {
      board_data->mac[j][i] = buf_boarddata[count++];
    }
    memset(out_str, 0x00, sizeof(out_str));
    sprintf(out_str,"%02X:%02X:%02X:%02X:%02X:%02X",\
    (unsigned char)board_data->mac[j][0],\
    (unsigned char)board_data->mac[j][1],\
    (unsigned char)board_data->mac[j][2],\
    (unsigned char)board_data->mac[j][3],\
    (unsigned char)board_data->mac[j][4],\
    (unsigned char)board_data->mac[j][5]);
    sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s >> /tmp/boardinfo_mac.txt",out_str,board_data->s_n);
    system(cmd);
    sprintf(cmd,"uci set systeminfo.sysinfo.%s=%s",  j == 0 ? "maclan" : (j == 1 ? "macwan1" : "macwan2"), out_str);
    system(cmd);
  }

  system("uci commit systeminfo");

  partno_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<partno_length;i++)
  {
    *((char *)board_data->p_n + i) = buf_boarddata[count++];
  }
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_partno.txt",board_data->p_n,board_data->s_n);
  system(cmd);

  prodname_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<prodname_length;i++)
  {
    *((char *)board_data->prod_name + i) = buf_boarddata[count++];
  }
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_prodname.txt",board_data->prod_name,board_data->s_n);
  system(cmd);

  prodseries_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<prodseries_length;i++)
  {
    *((char *)board_data->prod_series + i) = buf_boarddata[count++];
  }
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_prodseries.txt",board_data->prod_series,board_data->s_n);
  system(cmd);

  free(board_data);
  return 0;
}

int main(int argc,char *argv[])
{
  return boardinfo_cipher();
}

#else

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#define SIZE_PID 11  /* Max 11 Character Product ID */
#define SIZE_VID  3  /* Max  3 Character H/W Version */
#define SIZE_S_N 11  /* Max 11 Character Board Serial Number */
#define SIZE_P_N 12  /* Max 11 Character Board Serial Number */
#define SIZE_PROD_NAME 60  /* Max 11 Character Board Serial Number */
#define SIZE_PROD_SERIES 16  /* Max 11 Character Board Serial Number */
#define COUNT_MAC        2 /* 2 MAC-IDs one each for LAN and WAN */
#define TOTAL_MAC_OCTET  6 /* No. of MAC Octets in MAC ID */

#define KEY_FILE "/tmp/serial_key.txt"

#define KEY_CR 0x0D  /* Enter Key Press */

#define OFFSET_ALPHA 0x37
#define OFFSET_DIGIT 0x30

typedef struct c2krv16x_26x_board_info {
  unsigned char pid[16];             /* type 1byte length 1byte data 12 Product ID */
  unsigned char vid[8];              /* H/W Version */
  unsigned char s_n[16];             /* Board Serial Number */
  unsigned char mac[2][8];           /* MAC Information */
  unsigned char p_n[16];             /* Board Serial Number */
  unsigned char prod_name[64];       /* Board Product Name */
  unsigned char prod_series[16];     /* Board Product Series */
}  __attribute__ ((__packed__)) c2krv16x_26x;

char *filename = "/dev/mtd19" ;

static int boardinfo_cipher( )
{
  int fd = -1, n_read = 0, i = 0;
  char out_str[18] = {0}, ifname[6] = {0};
  c2krv16x_26x *board_data = NULL;
  int count;
  char buf_boarddata[512];
  int pid_length;
  int vid_length;
  int serial_length = 0, partno_length = 0, prodseries_length = 0, prodname_length = 0, swdesc_length = 0, swobjid_length = 0;
  int j,mac_length;
  char cmd[256] = {'\0'};
  char PidTmp[128] = {0};
  char *p1 = NULL;
  char *p2 = NULL;
  char *tmpPid = NULL;

  fd = open(filename, O_RDONLY, 0);
  if (fd < 0) {
    printf("could not open %s!\n", filename);
    return -1;
  }
  board_data = (c2krv16x_26x *)malloc(sizeof(c2krv16x_26x));
  if ( board_data == NULL )
  {
    close(fd);
    printf("malloc failed\n");
    return -1;
  }
  memset((char *)buf_boarddata, 0x00, sizeof(buf_boarddata));
  if ( (n_read = read(fd, (char *)buf_boarddata, sizeof(buf_boarddata))) < 0 )
  {
    free(board_data);
		printf("Reading failed\n");
    close(fd);
    return -1;
  }

  close(fd);

  count=2;
  pid_length = buf_boarddata[1];
  for (i=0;i<pid_length;i++)
  {
    *((char *)board_data->pid + i) = buf_boarddata[count++];
  }

  vid_length = buf_boarddata [count + 1];
  count = count+2;
  for (i=0;i<vid_length;i++)
  {
    *((char *)board_data->vid + i) = buf_boarddata[count++];
  }

  serial_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<serial_length;i++)
  {
    *((char *)board_data->s_n + i) = buf_boarddata[count++];
  }

  sprintf(cmd,"echo %s > %s",board_data->s_n,KEY_FILE);
  system(cmd);

  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_pid.txt",board_data->pid,board_data->s_n);
  system(cmd);

  strcpy (PidTmp, board_data->pid);

  sprintf(cmd,"uci set systeminfo.sysinfo.fullpid=%s",PidTmp);
  system(cmd);

  p1=p2=PidTmp;

  if ((p2 = strstr (PidTmp, "-K9")) != NULL)
  {
	p2=p2+3;
	PidTmp[p2-p1]='\0';
  }

  sprintf(cmd,"uci set systeminfo.sysinfo.pid=%s",PidTmp);
  system(cmd);
  tmpPid=strtok(board_data->pid,"-");
  if((tmpPid != NULL) && (strcmp(tmpPid,"RV160") == 0))
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='Cisco RV160 VPN Router'");
      system(cmd);
  }
  if((tmpPid != NULL) && (strcmp(tmpPid,"RV160W") == 0))
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='Cisco RV160W Wireless-AC VPN Router'");
      system(cmd);
  }
  if((tmpPid != NULL) && (strcmp(tmpPid,"RV260") == 0))
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='Cisco RV260 VPN Router'");
      system(cmd);
  }
  if((tmpPid != NULL) && (strcmp(tmpPid,"RV260P")) == 0)
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='Cisco RV260P PoE VPN Router'");
      system(cmd);
  }
  if((tmpPid != NULL) && (strcmp(tmpPid,"RV260W")) == 0)
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_desc='Cisco RV260W Wireless-AC VPN Router'");
      system(cmd);
  }
  
  if(strncmp(board_data->pid,"RV1",3) == 0)
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_series='RV100 Series'");
      system(cmd);
  }  
  if(strncmp(board_data->pid,"RV2",3) == 0)
  {
      sprintf(cmd,"uci set systeminfo.sysinfo.prod_series='RV200 Series'");
      system(cmd);
  }

  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_vid.txt",board_data->vid,board_data->s_n);
  system(cmd);
  sprintf(cmd,"uci set systeminfo.sysinfo.vid=%s",board_data->vid);
  system(cmd);
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_serial.txt",board_data->s_n,board_data->s_n);
  system(cmd);
  sprintf(cmd,"uci set systeminfo.sysinfo.serial_number=%s",board_data->s_n);
  system(cmd);

  for (j = 0; j < COUNT_MAC ; j++)
  {
    mac_length=buf_boarddata[count+1];
    count = count+2;
    for (i=0;i<mac_length;i++)
    {
      board_data->mac[j][i] = buf_boarddata[count++];
    }
    memset(out_str, 0x00, sizeof(out_str));
    sprintf(out_str,"%02X:%02X:%02X:%02X:%02X:%02X",\
    (unsigned char)board_data->mac[j][0],\
    (unsigned char)board_data->mac[j][1],\
    (unsigned char)board_data->mac[j][2],\
    (unsigned char)board_data->mac[j][3],\
    (unsigned char)board_data->mac[j][4],\
    (unsigned char)board_data->mac[j][5]);
    sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s >> /tmp/boardinfo_mac.txt",out_str,board_data->s_n);
    system(cmd);
    sprintf(cmd,"uci set systeminfo.sysinfo.%s=%s",  j == 0 ? "maclan" : "macwan1", out_str);
    system(cmd);
  }

  system("uci commit systeminfo");

  partno_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<partno_length;i++)
  {
    *((char *)board_data->p_n + i) = buf_boarddata[count++];
  }
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_partno.txt",board_data->p_n,board_data->s_n);
  system(cmd);

  prodname_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<prodname_length;i++)
  {
    *((char *)board_data->prod_name + i) = buf_boarddata[count++];
  }
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_prodname.txt",board_data->prod_name,board_data->s_n);
  system(cmd);

  prodseries_length=buf_boarddata[count+1];
  count = count+2;
  for (i=0;i<prodseries_length;i++)
  {
    *((char *)board_data->prod_series + i) = buf_boarddata[count++];
  }
  sprintf(cmd,"echo %s | openssl enc -aes-128-cbc -a -salt -pass pass:%s > /tmp/boardinfo_prodseries.txt",board_data->prod_series,board_data->s_n);
  system(cmd);

  free(board_data);
  return 0;
}

int main(int argc,char *argv[])
{
  return boardinfo_cipher();
}


#endif
