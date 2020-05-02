
#ifndef ASD_CMN_H
#define ASD_CMN_H

/* Debug macro */
//#define ASD_DEBUG
#ifdef ASD_DEBUG
#define ASDEBUG  printf
#else                                 
#define ASDEBUG(format,args...)
#endif


/* Syslog macro */
#define DISABLE     0
#define ENABLE      1

int bSyslog_g;

#define swu_syslog(x, ...) \
{\
   if(bSyslog_g) \
       syslog(x, __VA_ARGS__);\
}

/* ASD Error codes */
#define ASD_SUCCESS 0
#define ASD_FAILURE -1
#define ER_INSUFFICIENT_MEMORY  10

//#"ASD Client Checksum failed for file"
#define ER_MD5SUM_CHECK_FAILED  13
//#"ASD Client failed to download file."
//ASD Client failed to get download URL.
#define ER_DOWNLOAD_FAILED  19

//#"Current firmware version is already latest"
#define ER_FRM_VER_CURRENT_IS_LATEST  21

//#"Current USB driver version is already latest"
#define ER_USBDRV_VER_CURRENT_IS_LATEST  22

//#"Current Signature version is already latest"
#define ER_SIG_VER_CURRENT_IS_LATEST  23

//#"Invalid License, Signature download failed"
#define ER_INVALID_LICENSE_SIG_DOWNLOAD_FAIL 24


#define MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL     3
#define MAX_STR_LEN 512
#define SWDOWNLOAD_STATUS_TMP_FILE "/mnt/configcert/config/downloadstatus"

 /*
  *  Download Status error codes
  *  0: Success
  *  1: In progress
  * -1: Error-Generic
  * -2: Error-File not found
  * -3: Error-Network issue
  */

#define STATUS_DOWNLOAD_SUCCESS 0
#define STATUS_IN_PROGRESS 1
#define STATUS_ERROR_GENERIC -1
#define STATUS_ERROR_FILE_NOT_FOUND -2
#define STATUS_ERROR_NETWORK_ISSUE -3
#define STATUS_ERROR_PERCENT 0
#define STATUS_SUCCESS_PERCENT 100


void  updateSwDownloadStatus(int status, char *pFile, int percent);

#endif /* ASD_CMN_H */
