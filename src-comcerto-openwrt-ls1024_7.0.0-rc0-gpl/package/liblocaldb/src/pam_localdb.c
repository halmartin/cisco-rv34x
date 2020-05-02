/**************************************************************************
 *   Copyright 2010-2015, Freescale Semiconductor, Inc. All rights reserved.
 ***************************************************************************/

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>

#define PAM_SM_AUTH
#define PAM_SM_ACCOUNT
#include <security/pam_modules.h>
#include <security/_pam_macros.h>
#include <security/pam_ext.h>

#define MODULE_NAME "pam_localdb"

#ifndef PAM_EXTERN
#  define PAM_EXTERN
#endif


/* Callback function used to free the saved return value for pam_setcred. */
void _int_free(pam_handle_t * pamh, void *x, int error_status)
{
                free(x);
}

PAM_EXTERN int
pam_sm_authenticate (pam_handle_t *pamh, int flags ,
		     int argc, const char **argv)
{
    int ret = PAM_SUCCESS;
    const char *user;
    const char *password = NULL;
    const struct pam_conv *conv;
    struct pam_message msg[1],*pmsg[1];
    struct pam_response *resp=NULL;
    FILE *fp = NULL;
    char buffer[256]={'\0'};
    char cmd[256]={'\0'};
    
    pmsg[0] = &msg[0];
    msg[0].msg_style = PAM_PROMPT_ECHO_OFF;    
    msg[0].msg = "Password: ";


    ret = pam_get_user(pamh, &user, NULL);   
    if(ret != PAM_SUCCESS)
        return ret;

    /* grab the password */
    ret = pam_get_item(pamh, PAM_CONV, (const void **) &conv);    
    if (ret == PAM_SUCCESS)
    {
        ret = conv->conv (1,
                (const struct pam_message **) pmsg,
                &resp, conv->appdata_ptr);
    }  
    else
        return ret;

    if (resp != NULL)
    {
        if ((flags & PAM_DISALLOW_NULL_AUTHTOK) && resp[0].resp == NULL)
        {
            free (resp);
            return PAM_AUTH_ERR;
        }

        password = resp[0].resp;
        resp[0].resp = NULL;
    }
    else
    {
        return PAM_CONV_ERR;
    }

    free (resp);

    sprintf(cmd, "/usr/bin/userauth authenticate '%s' '%s'", user,password);
    fp = popen(cmd,"r");
    if (fp == NULL)
    {
      syslog(LOG_ERR, "Localdb:popen Failed.");
      return PAM_AUTH_ERR;
    }

    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
      if (strcmp(buffer, "PAM_AUTH_ERR") ==0 )
      {
         free(password);
         pclose(fp);
         return PAM_AUTH_ERR;
      }
      else
      {
         pam_set_data(pamh, "groupattr0", strdup(buffer) , _int_free);
         free(password);
         pclose(fp);
         return PAM_SUCCESS;
      }
    }

    pclose(fp);
    return PAM_AUTH_ERR;

}

PAM_EXTERN int
pam_sm_setcred (pam_handle_t *pamh , int flags ,
		int argc , const char **argv )
{
	return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
	return PAM_SUCCESS;;
}

PAM_EXTERN int
pam_sm_open_session (pam_handle_t *pamh, int flags,
		     int argc, const char **argv)
{
	return PAM_SUCCESS;;
}

PAM_EXTERN int
pam_sm_close_session (pam_handle_t *pamh, int flags,
		      int argc, const char **argv)
{
	return PAM_SUCCESS;
}

PAM_EXTERN int
pam_sm_chauthtok (pam_handle_t *pamh, int flags,
		  int argc, const char **argv)
{
	return PAM_SUCCESS;;
}

#ifdef PAM_STATIC

/* static module data */

struct pam_module _pam_localdb_modstruct = {
     "pam_localdb",
     pam_sm_authenticate,
     pam_sm_setcred,
     pam_sm_acct_mgmt,
     pam_sm_open_session,
     pam_sm_close_session,
     pam_sm_chauthtok
};

#endif
