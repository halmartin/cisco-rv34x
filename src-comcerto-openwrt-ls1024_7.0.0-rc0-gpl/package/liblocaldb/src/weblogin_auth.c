#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
int my_conv(int num_msg, const struct pam_message** msg,
        struct pam_response** resp, void* appdata_ptr);

static struct pam_conv conv = {
    my_conv,
    NULL
};


char group[256]={'\0'};

//#define WEBLOGIN_LOGSUPPORT

#ifdef WEBLOGIN_LOGSUPPORT
#define wl_log syslog
#else
#define wl_log
#endif

void wl_openlog() {
#ifdef WEBLOGIN_LOGSUPPORT
    openlog ("weblogin", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
#endif
}

void wl_closelog() {
#ifdef WEBLOGIN_LOGSUPPORT
	closelog();
#endif
}

int main(int argc,char*argv[])
{
    pam_handle_t *pamh=NULL;
    int retval, i;
    char buffer[256]={'\0'};
    char *user=NULL;
    char *password=NULL;
    char *context=NULL;
    FILE *fp;

    wl_openlog();
    wl_log(LOG_INFO," Logging for weblogin");
    fp = popen("wc -c /tmp/etc/config/ldap /tmp/etc/config/radius /tmp/etc/config/ad | tail -n 1 | grep -Eo [0-9]*", "r");
    fscanf(fp, "%d", &retval);
    pclose(fp);
    if(retval<=3) {
	wl_closelog();
	exit(1);
    }
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strlen(buffer)-1]='\0';

    wl_log(LOG_INFO," buffer:%s",buffer);
    user=strtok(buffer,"]");
    user=strtok(user,"[");
    user=strtok(user,";");
    password = strtok(NULL,";");
    for(i=0;i<=2;i++)
    {
	context=strtok(NULL,";");
    }
    wl_log(LOG_INFO," logging user-%s,password-%s", user,password);
    wl_log(LOG_INFO," context:%s",context);
    

    if((!strcmp(context,"webui")) || (!strcmp(context,"netconf")) || (!strcmp(context,"rest"))) {
        
        wl_log(LOG_INFO," trying weblogin pam");

        conv.appdata_ptr = (void*) password;

        retval = pam_start("weblogin", user, &conv,  &pamh);

        if (retval == PAM_SUCCESS)
        {
            retval = pam_authenticate(pamh, 0);    
            if (retval == PAM_SUCCESS)
            {
                retval = pam_acct_mgmt(pamh, 0);    
            }
            else if ( retval == PAM_PERM_DENIED)
            {
                fputs("reject Bad password\n",stdout);
                wl_log(LOG_INFO," reject Bad password");
                if (pam_end(pamh,retval) != PAM_SUCCESS) {
                    pamh = NULL;
                    wl_closelog();
                    exit(1);
                }
                wl_closelog();
                return ( retval == PAM_SUCCESS ? 0:1 );
            }
            else
            {
                fputs("abort External auth rejected the login\n",stdout);
                wl_log(LOG_INFO," abort External auth rejected the login");
            }
        }

        if (pam_end(pamh,retval) != PAM_SUCCESS) {     /* close Linux-PAM */
            pamh = NULL;
            wl_closelog();
            exit(1);
        }
        sprintf(buffer,"accept %s 1000 500 /usr/bin\n",group);
        fputs(buffer,stdout);
        wl_log(LOG_INFO,"%s", buffer);
        wl_closelog();
        return ( retval == PAM_SUCCESS ? 0:1 );       /* indicate success */
    }
    else
    {
        wl_log(LOG_INFO," context not matching");
        wl_closelog();
        exit(1);

    }
}




#define PAM_AUTH_RESP_ATTRIBUTES  0x1000
int my_conv(int num_msg, const struct pam_message** msg,
        struct pam_response** resp, void* appdata_ptr)
{
    int i;
    char* password = NULL;
    struct pam_response* myresp = NULL;

    for(i = 0; i<num_msg; i++)
    {
        switch(msg[i]->msg_style)
        {
            case PAM_PROMPT_ECHO_OFF:
            case PAM_PROMPT_ECHO_ON:
                if(!strncmp(msg[0]->msg, "Password", strlen("Password")))
                {
                    myresp = (struct pam_response*) calloc(1,sizeof(struct pam_response));
                    if(!myresp)
                        return PAM_CRED_ERR;
                    password = (char*) calloc(1, strlen((char*)appdata_ptr)+1);
                    if(!password)
                    {
                        free(myresp);
                        return PAM_CRED_ERR;
                    }
                    strcpy(password, (char*)appdata_ptr);
                    myresp->resp = password;
                    myresp->resp_retcode = 0;
                    *resp = myresp;
                }
                break;
            case PAM_AUTH_RESP_ATTRIBUTES :
                strcpy(group,msg[i]->msg);
                break;
            case PAM_ERROR_MSG:
            case PAM_TEXT_INFO:
                /* We will add more here */
            default:
                return PAM_CRED_ERR;
        }
    }
    return PAM_SUCCESS;
}


