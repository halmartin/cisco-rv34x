
/**************************************************************************
 *  * Copyright 2010-2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
/** PAM authentication routine */
#define PAM_SM_AUTH

/* PAM session management */
#define PAM_SM_SESSION

/* PAM password changing routine */
#define PAM_SM_PASSWORD

/* PAM authorization routine */
#define PAM_SM_ACCOUNT

#include <security/pam_appl.h>
#include <security/pam_modules.h>

#ifndef PAM_EXTERN
#define PAM_EXTERN
#endif

#include <syslog.h>

#define PAM_AUTH_RESP_ATTRIBUTES  0x1000
#define PAM_AUTH_CONNECTION_NAME  0x2000

#define PPP_USER_ACCOUNTING_FILE	"/tmp/ppp_user_accounting.log"
#define SINGLE_QUOTE "'\"'\"'"

void log_user_accounting(char *user)
{
	int fd, num;
	time_t now = time(0);
	char buf[296];
	struct tm *ltm = localtime(&now);

	memset(buf, 0, 296);

	fd = open(PPP_USER_ACCOUNTING_FILE, O_CREAT|O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
	if(fd < 0) {
		syslog(LOG_ERR,"open() failed with error [%s]\n",strerror(errno));
		return;
	}

	sprintf(buf, "User:%s PID:%d Time:%d;%d;%d;%d;%d;%d\n",user, getpid(), (1900+ltm->tm_year),(1+ltm->tm_mon),ltm->tm_mday,ltm->tm_hour,ltm->tm_min,ltm->tm_sec);
	num = write(fd, buf, 296);
	if(num <= 0) {
		syslog(LOG_ERR,"write() failed with error [%s]\n",strerror(errno));
	}
	close(fd);
}

/* expected hook */
PAM_EXTERN int pam_sm_setcred( pam_handle_t *pamh, int flags, int argc, const char **argv ) {
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) 
{

    char * groupname, *mod_data;

    const struct pam_conv *conv;
    struct pam_message *resp_msg = NULL;
    const struct pam_message *msg[256];
    struct pam_message tmpmsg[1],*ptmpmsg[1];
    struct pam_response *resp = NULL;
    int retval = PAM_AUTH_ERR;
    //struct sockaddr_in addr;  
    //int rsock;
    char *service_name;
    char *connection_name = NULL;
    //char *ptmp = NULL;
    int n_service = 1;
    int i, ii=0;
    int action;
    char *user;
    
    FILE *fp = NULL;
    char buffer[256]={'\0'};
    char cmd[512]={'\0'};
    int sp_conname = 0;
    char *sp_ssid = NULL ,*ret = NULL;
    int j = 0, sp_ssid_len = 0;

    /*Get the group name*/
    retval = pam_get_data( pamh, "groupattr0", (void **) &groupname );
    if(retval != PAM_SUCCESS)
    {
        syslog(LOG_ERR, "Localdb:authorization failed as group is NULL");
        return retval;
    }

    /*Get the service name*/
    retval = pam_get_item(pamh, PAM_SERVICE, &service_name);
    if(retval != PAM_SUCCESS)
    {
        syslog(LOG_ERR, "Localdb:authorization failed  as service name is NULL");
        return retval;
    }
    /* Get the converastion pointer */
    retval = pam_get_item(pamh, PAM_CONV, (const void **) &conv);
    if (retval != PAM_SUCCESS)
    {
        syslog(LOG_ERR, "Localdb:authorization failed as conv is NULL group:%s service:%s",
                         groupname,service_name);
        return retval;
    }
    if(strcmp(service_name,"weblogin") == 0)
      {
        resp_msg = (struct pam_message *)malloc(sizeof (struct pam_message));
        resp_msg->msg_style = PAM_AUTH_RESP_ATTRIBUTES;
        resp_msg->msg=groupname;
        msg[0] = resp_msg;
        retval = conv->conv(1, msg, &resp,conv->appdata_ptr);
        if (retval != PAM_SUCCESS)
          {
             syslog(LOG_ERR, "Localdb:authorization failed grooup:%s service:%s",
                         groupname,service_name);
             return PAM_PERM_DENIED;
          }
        free(resp_msg);
       return PAM_SUCCESS;
     }


     if((strcmp(service_name,"s2s-vpn") == 0) || (strcmp(service_name,"ezvpn") == 0) || 
               (strcmp(service_name,"3rdparty") == 0) || (strcmp(service_name, "ssid") == 0) ||
			   (strcmp(service_name,"captive-portal") == 0))
     {
        ptmpmsg[0] = &tmpmsg[0];
        tmpmsg[0].msg_style = PAM_AUTH_CONNECTION_NAME;
        tmpmsg[0].msg = "Connection: ";

        /*Get the connection name*/
        retval = pam_get_item(pamh, PAM_CONV, (const void **) &conv);
        if (retval == PAM_SUCCESS)
        {
             retval = conv->conv (1,
                     (const struct pam_message **) ptmpmsg,
                     &resp, conv->appdata_ptr);
        }
        else
          return retval;

          if (resp != NULL)
           {
             if ((flags & PAM_DISALLOW_NULL_AUTHTOK) && resp[0].resp == NULL)
             {
                free (resp);
                return PAM_AUTH_ERR;
             }

             connection_name = strdup(resp[0].resp);
	     free(resp[0].resp);
             resp[0].resp = NULL;
           }
           else
           {
             free (resp);
             return PAM_CONV_ERR;
           }
          free (resp);
     }

       do
       {
	 mod_data = (char *) calloc(1, strlen("groupattr")+2);
         sprintf(mod_data,"groupattr%d",ii++);
	        /*Get the group name*/
       	 retval = pam_get_data( pamh, mod_data, (void **) &groupname);
       	 if(retval != PAM_SUCCESS)
       	 {
       	     syslog(LOG_ERR, "Localdb:authorization failed as group is NULL");
             free(mod_data);
             free(connection_name);
       	     return PAM_PERM_DENIED;
       	 }

        free(mod_data);

       if((strcmp(service_name,"s2s-vpn") == 0) || (strcmp(service_name,"ezvpn") == 0) || 
                 (strcmp(service_name,"3rdparty") == 0) || (strcmp(service_name, "ssid") == 0) ||
				 (strcmp(service_name,"captive-portal") == 0))
       {
           sp_conname = 0;
           if(strcmp(service_name,"ssid") == 0)
           {
               ret = strchr(connection_name,'\'');
               if (ret)
               {
                   sp_conname = 1;
                   sp_ssid =(char*)malloc(256);
                   if (!sp_ssid)
                   {
                       syslog(LOG_ERR, "Localdb:Insufficient memory");
                       return PAM_AUTH_ERR;
                   }
                   sp_ssid_len = 0;
                   for(j=0 ;connection_name[j] != '\0';j++)
                   {
                       if(connection_name[j] == '\'')
                       {
                           sp_ssid_len += sprintf ((sp_ssid + sp_ssid_len),"%s",SINGLE_QUOTE);
                           continue;
                       }
                       sp_ssid[sp_ssid_len++] = connection_name[j];
                   }
                   sp_ssid[sp_ssid_len] = '\0';
                   sprintf(cmd, "/usr/bin/userauth authorize '%s' '%s' '%s'",
                           service_name, groupname,sp_ssid);
                   if(sp_ssid)
                       free(sp_ssid);
               }
           }
           if (sp_conname == 0)
           {
               sprintf(cmd, "/usr/bin/userauth authorize '%s' '%s' '%s'",
                       service_name, groupname,connection_name);
           }
       }
       else
       {
           sprintf(cmd, "/usr/bin/userauth authorize '%s' '%s'", service_name, groupname);
       }
        fp = popen(cmd,"r");
        if (fp == NULL)
        {
          syslog(LOG_ERR, "Localdb:popen Failed.");
          free(connection_name);
          return PAM_AUTH_ERR;
        }

        while(fgets(buffer, sizeof(buffer), fp) != NULL)
        {
          if (strcmp(buffer, "PAM_AUTH_ERR") ==0 )
          {
             pclose(fp);
             free(connection_name);
             return PAM_AUTH_ERR;
          }
          else if(strcmp(buffer, "PAM_SUCCESS") ==0 )
          {
	     if((strcmp(service_name,"pptp")==0) || (strcmp(service_name,"l2tp")==0) || 
                  (strcmp(service_name,"ppp")==0))
             {
		retval = pam_get_user(pamh, &user, NULL);
		if(retval != PAM_SUCCESS) {
                    pclose(fp);
                    free(connection_name);
		    return retval;
		}
		log_user_accounting(user);
             }
             pclose(fp);
             free(connection_name);
             return PAM_SUCCESS;
          }
          else // service is of type anyconnect-vpn
          {
             resp_msg = (struct pam_message *)malloc(sizeof (struct pam_message));
             resp_msg->msg_style = PAM_AUTH_RESP_ATTRIBUTES;
             resp_msg->msg = buffer;
             msg[i] = resp_msg;

             action = conv->conv(n_service, msg, &resp,conv->appdata_ptr);
             if (action == PAM_SUCCESS)
             {
                 syslog(LOG_ERR, "Localdb:authorization sucess grooup:%s service:%s",
                                        groupname,service_name);
               free(resp_msg);
               pclose(fp);
               return PAM_SUCCESS;   
             }
             free(resp_msg);
          }
        }
        pclose(fp);

      } while(groupname != NULL);

    if (connection_name != NULL)
        free(connection_name);

    return PAM_AUTH_ERR;
}

/* expected hook, this is where custom stuff happens */
PAM_EXTERN int pam_sm_authenticate( pam_handle_t *pamh, int flags,int argc, const char **argv ) {

	return PAM_SUCCESS;
}
