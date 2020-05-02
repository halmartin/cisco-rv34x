#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <sys/statvfs.h>
#include "cJSON.h"
#include "request.h"
#include "asdcmn.h"
#include "asdclient.h"

/* Macros */
#define ASD_FIRMWARE_TYPE "firmware"
#define ASD_USBDRIVERS_TYPE "drivers"
#define ASD_SIGNATURES_TYPE "signatures"
#define SW_UPDATE_INFO_FILE "/mnt/configcert/config/swupdateinfo"
#define SW_UPDATE_INFO_TMP_FILE "/mnt/configcert/config/swupdateinfo.tmp"
#define SW_UPDATE_ALERT_TMP_FILE "/tmp/asd_email_content"
#define SW_DOWNLOAD_STATUS_TMP_FILE "/tmp/asd_download_status"
#define FRM_AVAILABLE_VERSION "frm_available_version"
#define FRM_LAST_CHECK_TIME "frm_last_check_time"
#define USB_AVAILABLE_VERSION "usb_available_version"
#define USB_LAST_CHECK_TIME "usb_last_check_time"
#define SIG_AVAILABLE_VERSION "sig_available_version"
#define SIG_LAST_CHECK_TIME "sig_last_check_time"
#define CHK_VALID "Valid"
#define CHK_IN_VALID "Invalid"
#define SH_CHEKSCRIPT "lcsh license status"
#define FRM_APPLY_SCRIPT "sh /usr/bin/rv340_fw_unpack.sh"
#define SIG_APPLY_SCRIPT "sh /usr/bin/lcsig.sh"
#define USB_APPLY_SCRIPT "sh /usr/bin/usb_apply_script.sh"
#define SEND_EMAIL_SCRIPT "sh /usr/bin/sendAsdmail"
#define SWUPDATE_INFO_SCRIPT "sh /usr/bin/updateswinfo.sh"
#define NEW_IS_GREATER  1
#define NEW_IS_NOT_GREATER  -1
#define MAX_VER_ARR_SIZE 4
#define SW_VERSION_CHK_FILE "/etc/verchk"

#define VALIDATE_SW_VER

/* Global Variable Declarations */

AccessTokn_t sAccessTok_g;
char cHeaderStr[100];


/*
 * Prototypes
 */

void  updateDownloadStatus(int status, char *pPath);
char *trim_string(char *string_p);
void getObjectValue(cJSON * pObj, char **pOutVal);
int parseJSONText(char *text, cJSON ** pJsonObj);
void parseJSON(char *text);
void parse_object(cJSON * root);
Metadata_t *newMetaData(void);
int parseMetadata(char *pData, Metadata_t ** pMetadata);
int parseDownloadInfo(char *pData, DownLoadInfo_t ** pDownLoadInfo);
void freeMetadata(Metadata_t * pMetadata);
void freeDownloadInfo(DownLoadInfo_t * pDownLoadInfo);

int getASDParamsByPid(char *pPid, ASDParams_t * pASDParams,
		      swtype_e eSWType);
int getAccessToken(char *pPid);
int getMetaData(ASDParams_t * pASDParams, Metadata_t ** pMetadat);
int getDownloadUrl(ASDParams_t * pASDParams, Metadata_t * pMetadat,
		   DownLoadInfo_t ** pDownLoadInfo);
int validatelicense(void);
int downloadActualFile(ASDParams_t * pASDParams, Metadata_t * pMetadat,
		       DownLoadInfo_t * pDownLoadInfo);
int applyDownloadedFile(ASDParams_t * pASDParams,
			char *pInScript, DownLoadInfo_t * pDownLoadInfo);
char *getTimeStamp();
int updateswinfo(char *pVersion, swtype_e eSWType);

#ifdef VALIDATE_SW_VER
int toArray(char *pInStr, char *pTok, int iOutArry[]);
int validateVersion(char *pOldVer, char *pNewVer);
#endif				/* VALIDATE_SW_VER */
int checkVersionOpt(void);

int checkUpdates(ASDParams_t * pASDParams, swtype_e eSWType);
void  emailAlert(char *alertMesg);
int checkAvailableSpace(ASDParams_t * pASDParams, Metadata_t * pMetadat);

static int bvalidate = 0;
static char swtypent[20] = {0};
static char alertStr[120] = {0};

int main(int argc, char **argv)
{
    ASDParams_t ASDParams;
    Metadata_t *pMetadata;
    DownLoadInfo_t *pDownLoadInfo;
    swtype_e eSWType;


    bSyslog_g = ENABLE;

    memset(&ASDParams, 0, sizeof(ASDParams_t));

    if (argc < 8) {
	printf
	    ("\nUsage: asdclient <pid> <vid> <sno> <current_version> <firmware/drivers/signatures> <store_file_path> <check/download>\n");
	return ASD_FAILURE;
    }
    ASDEBUG("\n %s():LINE:%d : arg1= %s , arg2=%s , arg3=%s arg4=%s arg5=%s arg6=%s arg7=%s \n",
             __FUNCTION__, __LINE__, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);

    memset(ASDParams.cPId, 0, sizeof(ASDParams.cPId));
    memset(ASDParams.cVId, 0, sizeof(ASDParams.cVId));
    memset(ASDParams.cSNo, 0, sizeof(ASDParams.cSNo));
    memset(ASDParams.cCurVer, 0, sizeof(ASDParams.cCurVer));

    strcpy(ASDParams.cPId, argv[1]);
    strcpy(ASDParams.cVId, argv[2]);
    strcpy(ASDParams.cSNo, argv[3]);
    strcpy(ASDParams.cCurVer, argv[4]);

	 
    strcpy(ASDParams.cPath, argv[6]);
    strcpy(ASDParams.cType, argv[7]);


    if (strcmp(argv[5], ASD_FIRMWARE_TYPE) == 0) {
	eSWType = FIRMWARE;
    } else if (strcmp(argv[5], ASD_USBDRIVERS_TYPE) == 0) {
	eSWType = USBDRIVER;
    } else if (strcmp(argv[5], ASD_SIGNATURES_TYPE) == 0) {
	eSWType = SIGNATURES;
    }

    swu_syslog(LOG_INFO, " ASD Client started %s for %s \r\n", argv[7], argv[5]);

    strcpy(swtypent, argv[5]);
    if (getASDParamsByPid(ASDParams.cPId, &ASDParams, eSWType) !=
	ASD_SUCCESS) {
	ASDEBUG("\n No such product.\n");
        swu_syslog(LOG_ERR, " ASD Client: No such product: %s \r\n", ASDParams.cPId);
        if (strcmp((char *) ASDParams.cType, ASD_CMD_TYPE_DOWNLOAD) == 0) {
            updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                ASDParams.cType, STATUS_ERROR_PERCENT);
        }
	return ASD_FAILURE;
    }


    ASDEBUG("\n%s(): cMDFId= %s,  cSWTId= %s,  cCurVer= %s \n",
	    __FUNCTION__, ASDParams.cMDFId,
	    ASDParams.cSWTId, ASDParams.cCurVer);

    bvalidate = checkVersionOpt();

    if (getAccessToken(ASDParams.cPId) != ASD_SUCCESS) {
	ASDEBUG("\nASD Client failed to get access token.\n");
        swu_syslog(LOG_ERR, " ASD Client failed to get access token.\r\n");
        if (strcmp((char *) ASDParams.cType, ASD_CMD_TYPE_DOWNLOAD) == 0) {
            updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                ASDParams.cType, STATUS_ERROR_PERCENT);
        }
	return ASD_FAILURE;
    }
    memset(cHeaderStr, 0, sizeof(cHeaderStr));
    // Header:  "Authorization: Bearer " + AccessToken;
    sprintf(cHeaderStr, "%s%s",
	    METADATA_HEADER_PREFIX_STR, sAccessTok_g.cAccessToken);

    if (strcmp((char *) ASDParams.cType, ASD_CMD_TYPE_CHECK) == 0) {
	// Just check for updates
	if (checkUpdates(&ASDParams, eSWType) != ASD_SUCCESS)
        {
            swu_syslog(LOG_ERR, " ASD Client failed to check updates.\r\n");
	    return ASD_FAILURE;
        }
    } else if (strcmp((char *) ASDParams.cType, ASD_CMD_TYPE_DOWNLOAD) == 0) {


	// Check for update and download
	if (getMetaData(&ASDParams, &pMetadata) != ASD_SUCCESS)
        {
            updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                NULL, STATUS_ERROR_PERCENT);
	    return ASD_FAILURE;
        }
         ASDEBUG("\n%s(): bvalidate =%d .\n", __FUNCTION__, bvalidate);
        // validate versions
        if(bvalidate  == 1)
        {
            if (validateVersion(ASDParams.cCurVer, pMetadata->pRelease_version)
	        == NEW_IS_NOT_GREATER) {
		ASDEBUG("\n Current version is already latest %s\n", ASDParams.cCurVer);
                swu_syslog(LOG_INFO, " ASD Client: %s version is already latest %s . \r\n", swtypent, ASDParams.cCurVer);
                if(strcmp(swtypent,"firmware") == 0 )
                {
                    updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
		    freeMetadata(pMetadata);
                    return ER_FRM_VER_CURRENT_IS_LATEST;
                }
                if(strcmp(swtypent, "drivers") == 0 )
                {
                    updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
		    freeMetadata(pMetadata);
                    return ER_USBDRV_VER_CURRENT_IS_LATEST;
                }
                if(strcmp(swtypent, "signatures") == 0 )
                {
                    updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
		    freeMetadata(pMetadata);
                    return ER_SIG_VER_CURRENT_IS_LATEST;
                }
            }
        }
	// For signatures validate license.
	if (eSWType == SIGNATURES) {
	    ASDEBUG("\n Validate License.\n");
            swu_syslog(LOG_INFO, "Validate License.\r\n");
	    // Validate license before download
	    if (validatelicense() != ASD_SUCCESS) {
		ASDEBUG("\n Invalid License.\n");
                swu_syslog(LOG_ERR, " ASD Client: Invalid License.\r\n");
                updateDownloadStatus(ER_INVALID_LICENSE_SIG_DOWNLOAD_FAIL, "failed");
                updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
		freeMetadata(pMetadata);
		return ER_INVALID_LICENSE_SIG_DOWNLOAD_FAIL;
	    }
	}
	
	// get download URL
	if (getDownloadUrl(&ASDParams, pMetadata, &pDownLoadInfo) !=
	    ASD_SUCCESS) {
            swu_syslog(LOG_ERR, " ASD Client failed to get download URL.\r\n");
            updateDownloadStatus(ER_DOWNLOAD_FAILED, "failed");
            updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
	    freeMetadata(pMetadata);
	    return ER_DOWNLOAD_FAILED;
	}
        if(checkAvailableSpace(&ASDParams, pMetadata) != ASD_SUCCESS)
        {
            swu_syslog(LOG_ERR, "ASD Client not enough space to download %s file.\r\n",pMetadata->pImage_name);
            sprintf(alertStr,  "Not enough space to download %s file.\n",pMetadata->pImage_name);
            emailAlert(alertStr);
            updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
            //updateDownloadStatus(ASD_FAILURE, "failed");
            updateDownloadStatus(ER_INSUFFICIENT_MEMORY, "failed");
	    ASDEBUG("\n Error downloading file: not enough space.\n");
	    freeMetadata(pMetadata);
	    freeDownloadInfo(pDownLoadInfo);
	    return ER_INSUFFICIENT_MEMORY;
	}

	// download file from URL.
	if (downloadActualFile(&ASDParams, pMetadata, pDownLoadInfo) !=
	    ASD_SUCCESS) {
            swu_syslog(LOG_ERR, " ASD Client failed to download file.\r\n");
            updateDownloadStatus(ASD_FAILURE, "failed");
	    ASDEBUG("\n Error downloading file.\n");
            updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                           pMetadata->pImage_name, STATUS_ERROR_PERCENT);
	    freeMetadata(pMetadata);
	    freeDownloadInfo(pDownLoadInfo);
	    return ER_DOWNLOAD_FAILED;
	}
        

	freeDownloadInfo(pDownLoadInfo);
	freeMetadata(pMetadata);
    } else {
	// Invalid command type
	printf
	    ("\nUsage: asdclient <pid> <vid> <sno> <current_version> <firmware/drivers/signatures> <store_file_path> <check/download>\n");
    }

    return ASD_SUCCESS;
}

int checkAvailableSpace(ASDParams_t * pASDParams, Metadata_t * pMetadat)
{
    unsigned long long dskSpace;
    unsigned long long fileSize;

    struct statvfs stat;

    if (statvfs(pASDParams->cPath, &stat) != 0) {
      // error happens, just quits here
        return ASD_FAILURE;
    }

    // the available size is f_bsize * f_bavail
    dskSpace = stat.f_bsize * stat.f_bavail;
    fileSize = strtoull(pMetadat->pImage_size, NULL, 10);

    //swu_syslog(LOG_ERR, " Available space=%llu bytes, Required space=%llu bytes .\r\n",dskSpace,fileSize);
    
    if(fileSize > dskSpace)
        return ASD_FAILURE;

    return ASD_SUCCESS;
}

int checkVersionOpt()
{
    int ret = 0;
    FILE *fp;
    char cLine[10] = {0};

    fp = fopen(SW_VERSION_CHK_FILE, "r");
    if (fp == NULL) {
	return ASD_FAILURE;
    }
    while (fgets(cLine, sizeof(cLine), fp) != NULL) {
        ret = atoi(cLine);
        ASDEBUG("\n proc entry = %d\n", ret);
    }

    fclose(fp);
    return ret;
}

/*
 * Function to validate license
 */
int validatelicense(void)
{
    FILE *in;
    extern FILE *popen();
    char buff[512];
    char *pBuff;
    int iRes = ASD_FAILURE;

    if (!(in = popen(SH_CHEKSCRIPT, "r"))) {
	ASDEBUG("\n%s(): %s command failed .\n", __FUNCTION__,
		SH_CHEKSCRIPT);
	pclose(in);
	return ASD_FAILURE;
    }

    while (fgets(buff, sizeof(buff), in) != NULL) {
	//printf("\n read buff= %s\n", buff);
	if (strstr(buff, CHK_VALID) != NULL) {
	    iRes = ASD_SUCCESS;
	} else if (strstr(buff, CHK_IN_VALID) != NULL) {
	    iRes = ASD_FAILURE;
	}

    }
    pclose(in);

    pBuff = trim_string(buff);
    //printf("\n ***   pBuff = %s", pBuff);
    if (iRes == ASD_SUCCESS || strstr(pBuff, CHK_VALID) != NULL)
	return ASD_SUCCESS;
    else if (iRes == ASD_FAILURE || strstr(pBuff, CHK_IN_VALID) != NULL)
	return ASD_FAILURE;

    return ASD_FAILURE;
}

int getASDParamsByPid(char *pPid, ASDParams_t * pASDParams,
		      swtype_e eSWType)
{
    int i = 0;
    //int len = 0;
    int retVal = ASD_FAILURE;

   // for (len = 0; pASDParams->cCurVer[len] != '\0'; len++);

    for (i = 0; i < ASD_PRODINFO_PARAM_COUNT; i++) {
	if (strcmp(Product_params[i].cPId, pPid) == 0
	    && Product_params[i].eSWType == eSWType) {
	    ASDEBUG("\n%s(): cMDFId= %s,  cSWTId= %s,  cCurVer= %s \n",
		    __FUNCTION__, Product_params[i].cMDFId,
		    Product_params[i].cSWTId, Product_params[i].cCurVer);
	    strcpy(pASDParams->cMDFId, Product_params[i].cMDFId);
	    strcpy(pASDParams->cSWTId, Product_params[i].cSWTId);

	    //if (len == 0)
	    //	strcpy(pASDParams->cCurVer, Product_params[i].cCurVer);
	    retVal = ASD_SUCCESS;
	    break;
	}
    }
    return retVal;
}

/*
 * Function to check for software updates.
 */
int checkUpdates(ASDParams_t * pASDParams, swtype_e eSWType)
{
    Metadata_t *pMetadata;
    int retval = 0;
    retval = getMetaData(pASDParams, &pMetadata);
    if (retval != ASD_SUCCESS) {
	ASDEBUG("\n%s(): Failed to get Metadata: error code= %d \n", __FUNCTION__, retval);
        if (retval == ASD_MTDATA_EXCEPTION)
            updateswinfo("N/A", eSWType);

	return ASD_FAILURE;
    }
    ASDEBUG("\n%s(): Update configuration parameters.\n", __FUNCTION__);

#ifdef VALIDATE_SW_VER
    // validate versions
    ASDEBUG("\n%s(): bvalidate =%d .\n", __FUNCTION__, bvalidate);
    if(bvalidate  == 1)
    {
        if (validateVersion(pASDParams->cCurVer, pMetadata->pRelease_version)
	    == NEW_IS_NOT_GREATER)
        {
            ASDEBUG("\n Current version is already latest %s\n", pASDParams->cCurVer);
            swu_syslog(LOG_INFO, " ASD Client: %s version is already latest %s . \r\n", swtypent, pASDParams->cCurVer);
        }
    }

#endif
    // Update configuration parameters.
    updateswinfo(pMetadata->pRelease_version, eSWType);

    freeMetadata(pMetadata);
    return ASD_SUCCESS;
}

#ifdef VALIDATE_SW_VER
int validateVersion(char *pOldVer, char *pNewVer)
{
    int iRet = NEW_IS_NOT_GREATER;
    int iOldVerArry[MAX_VER_ARR_SIZE];
    int iNewVerArry[MAX_VER_ARR_SIZE];
    int i = 0;
    char copyOldVer[20] = {0};
    char copyNewVer[20] = {0};

    strcpy(copyOldVer, pOldVer);
    strcpy(copyNewVer, pNewVer);
    ASDEBUG("\noldVer=%s newVer=%s\n", pOldVer, pNewVer);
    toArray(copyOldVer, ".", iOldVerArry);
    toArray(copyNewVer, ".", iNewVerArry);

    for (i = 0; i < 4; ++i)
    {
       ASDEBUG("old=%d  : new= %d\n", iOldVerArry[i], iNewVerArry[i]);
    }
    for (i=0;i<4; ++i)
    {
      if(iNewVerArry[i] == iOldVerArry[i])
        continue;
      if(iNewVerArry[i] > iOldVerArry[i])
      {
        iRet = NEW_IS_GREATER;
        break;
      }
      else
      {
        iRet = NEW_IS_NOT_GREATER;
        break;
      }
    }
    return iRet;
}

int toArray(char *pInStr, char *pTok, int iOutArry[])
{
    char *p;
    int i = 0;
    p = strtok(pInStr, pTok);
    while (p != NULL) {
	iOutArry[i++] = atoi(p);
	p = strtok(NULL, pTok);
    }

    return 0;
}
#endif				/* VALIDATE_SW_VER */

/*
 * Function to get current time stamp.
 */
char *getTimeStamp()
{
    time_t rawtime;
    struct tm *timeinfo;
    char *timstr;
    char timestamp[30 + 1];
    char *pos;
    char *pOutTimeStr = NULL;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    memset(timestamp, 0, sizeof(timestamp));
    timstr = asctime(timeinfo);
    if ((pos = strchr(timstr, '\n')) != NULL)
	*pos = '\0';
    sprintf(timestamp, "\"%s\"", timstr);
    //pOutTimeStr = strdup(timestamp);
    pOutTimeStr = (char*) malloc(strlen(timestamp)+1);
    if(!pOutTimeStr)
       return NULL;
    strcpy(pOutTimeStr, timestamp);

    return pOutTimeStr;
}

/*
 * Function to alert by email.
 * <alert string>
 */
void  emailAlert(char *alertMesg)
{
    FILE *fpw;

    fpw = fopen(SW_UPDATE_ALERT_TMP_FILE, "w");
    if (fpw == NULL) {
	return;
    }

    fprintf(fpw, "%s\n", alertMesg);

    fclose(fpw);

    system(SEND_EMAIL_SCRIPT);
}
/*
 * Function to update /tmp/asd_download_status file.
 * 0 <file absolute path>
 * or
 * 1 failed
 */
void  updateDownloadStatus(int status, char *pPath)
{
    FILE *fpw;

    fpw = fopen(SW_DOWNLOAD_STATUS_TMP_FILE, "w");
    if (fpw == NULL) {
	return;
    }

    if(status == ASD_SUCCESS)
    {
      fprintf(fpw, "0 %s\n", pPath);
    //} else if(status == ASD_FAILURE)
    } else
    {
      fprintf(fpw, "%d failed\n", status);
    }
    fclose(fpw);
}

/*
 * Function to update swupdateinfo config file.
 */
int updateswinfo(char *pVersion, swtype_e eSWType)
{
    FILE *fp;
    FILE *fpw;
    char *time = NULL;
    char cLine[100];
    char cNewLine[100];
    char *cLinesArr[20];
    int records = 0, i = 0;
    int bupdate =  0;
    int retval =  0;

    ASDEBUG("\n%s(): pVersion=%s .\n", __FUNCTION__, pVersion);
    switch (eSWType) {
	    case FIRMWARE:
		    swu_syslog(LOG_INFO, " Firmware Available Version: %s \r\n", pVersion);
                    sprintf(cLine, "%s %s %s", SWUPDATE_INFO_SCRIPT, "firmware", pVersion);
		    break;
	    case USBDRIVER:
                    swu_syslog(LOG_INFO, " USB Dongle Driver Available Version: %s \r\n", pVersion);
                    sprintf(cLine, "%s %s %s", SWUPDATE_INFO_SCRIPT, "drivers", pVersion);
		    break;
	    case SIGNATURES:
                    swu_syslog(LOG_INFO, " Signature Available Version: %s \r\n", pVersion);
                    sprintf(cLine, "%s %s %s", SWUPDATE_INFO_SCRIPT, "signatures", pVersion);
		    break;
    }
    if(system(cLine) != ASD_SUCCESS)
    {
       swu_syslog(LOG_INFO, " Failed to execute script: %s \r\n", cLine);
       ASDEBUG("\nFailed to execute script: %s \r\n", cLine);
    }
#if 0
    memset(cLine, 0, sizeof(cLine));
    memset(cNewLine, 0, sizeof(cNewLine));

    ASDEBUG("\n%s(): LINE = %d\n", __FUNCTION__, __LINE__);

    fp = fopen(SW_UPDATE_INFO_FILE, "r");
    if (fp == NULL) {
	return ASD_FAILURE;
    }

    while (fgets(cLine, sizeof(cLine), fp) != NULL) {
	switch (eSWType) {
	case FIRMWARE:
	    if (strstr(cLine, FRM_AVAILABLE_VERSION) != NULL) {
		swu_syslog(LOG_INFO, " Firmware Available Version: %s \r\n", pVersion);
		memset(cNewLine, 0, sizeof(cNewLine));
		sprintf(cNewLine, "%s=\"%s\"\n", FRM_AVAILABLE_VERSION, pVersion);
		bupdate =  1;
	    }
	    if (strstr(cLine, FRM_LAST_CHECK_TIME) != NULL) {
		memset(cNewLine, 0, sizeof(cNewLine));
		time = getTimeStamp();
		if(time)
		{
		    sprintf(cNewLine, "%s=%s\n", FRM_LAST_CHECK_TIME, time);
		    swu_syslog(LOG_INFO, " Firmware Last Check Time: %s \r\n", time);
		    free(time);
		    bupdate =  1;
                }
	    }
	    break;
	case USBDRIVER:
	    if (strstr(cLine, USB_AVAILABLE_VERSION) != NULL) {
                swu_syslog(LOG_INFO, " USB Dongle Driver Available Version: %s \r\n", pVersion);
		memset(cNewLine, 0, sizeof(cNewLine));
		sprintf(cNewLine, "%s=\"%s\"\n", USB_AVAILABLE_VERSION, pVersion);
		bupdate =  1;
	    }
	    if (strstr(cLine, USB_LAST_CHECK_TIME) != NULL) {
		memset(cNewLine, 0, sizeof(cNewLine));
		time = getTimeStamp();
		if(time)
		{
		    sprintf(cNewLine, "%s=%s\n", USB_LAST_CHECK_TIME, time);
		    swu_syslog(LOG_INFO, " USB Dongle Driver Last Check Time: %s \r\n", time);
		    free(time);
		    bupdate =  1;
		}
	    }
	    break;
	case SIGNATURES:
	    if (strstr(cLine, SIG_AVAILABLE_VERSION) != NULL) {
                swu_syslog(LOG_INFO, " Signature Available Version: %s \r\n", pVersion);
		memset(cNewLine, 0, sizeof(cNewLine));
		sprintf(cNewLine, "%s=\"%s\"\n", SIG_AVAILABLE_VERSION, pVersion);
		bupdate =  1;
	    }
	    if (strstr(cLine, SIG_LAST_CHECK_TIME) != NULL) {
		memset(cNewLine, 0, sizeof(cNewLine));
		time = getTimeStamp();
		if(time)
		{
		    sprintf(cNewLine, "%s=%s\n", SIG_LAST_CHECK_TIME, time);
		    swu_syslog(LOG_INFO, " Signature Last Check Time: %s \r\n", time);
		    free(time);
		    bupdate =  1;
		}
	    }
	    break;
	}

        if(bupdate ==  1)
	    cLinesArr[records] = (char*) malloc(strlen(cNewLine) + 1);
        else
	    cLinesArr[records] = (char*) malloc(strlen(cLine) + 1);

	if (cLinesArr[records] == NULL) {
	    for (i = 0; i < records; i++) {
		if (cLinesArr[i])
		    free(cLinesArr[i]);
	    }
	    return ASD_FAILURE;
	}
        if(bupdate ==  1)
	    strcpy(cLinesArr[records], cNewLine);
        else
	    strcpy(cLinesArr[records], cLine);

	ASDEBUG( " cLinesArr[%d]=%s \n", records, cLinesArr[records]);
	//memset(cNewLine, 0, sizeof(cNewLine));
	//memset(cNewLine, 0, sizeof(cLine));
	records++;
	bupdate=0;
    }
    fclose(fp);

    ASDEBUG( " records=[%d] \n", records);

    fpw = fopen(SW_UPDATE_INFO_TMP_FILE, "w");
    if (fpw == NULL) {
	for (i = 0; i < records; i++) {
	    if (cLinesArr[i])
		free(cLinesArr[i]);
	}
	return ASD_FAILURE;
    }

    for (i = 0; i < records; i++)
    {
	fprintf(fpw, "%s", cLinesArr[i]);
    }
    fclose(fpw);

    remove(SW_UPDATE_INFO_FILE);
    rename(SW_UPDATE_INFO_TMP_FILE, SW_UPDATE_INFO_FILE);

    for (i = 0; i < records; i++) {
	if (cLinesArr[i])
	    free(cLinesArr[i]);
    }
#endif

    return ASD_SUCCESS;
}

int getAccessToken(char *pPid)
{
    char *data;
    char *pPostData;
    char *headers[] =
	{ "Content-Type: application/x-www-form-urlencoded" };
    cJSON *pTokenType = NULL;
    cJSON *pExpiresIn = NULL;
    cJSON *pAccessToken = NULL;

    cJSON *pJson = NULL;

    memset(&sAccessTok_g, 0, sizeof(AccessTokn_t));

	ASDEBUG("\%s(): LINE: %d .\n", __FUNCTION__, __LINE__);


    if(strcmp(pPid, "RV340") == 0
        || strcmp(pPid, "RV340W") == 0
        || strcmp(pPid, "RV345") == 0
        || strcmp(pPid, "RV345P") == 0)
    {
        pPostData = BB2_ACCESS_TOKEN_POST_DATA;
    }
    if(strcmp(pPid, "RV160") == 0
        || strcmp(pPid, "RV160W") == 0
        || strcmp(pPid, "R260") == 0
        || strcmp(pPid, "RV260W") == 0
        || strcmp(pPid, "RV260P") == 0)
    {
        pPostData = PP_ACCESS_TOKEN_POST_DATA;
    }
    if (sendRequest(URL_ACCESS_TOKEN_REQ, headers, pPostData, &data) !=
       ASD_SUCCESS) {
	ASDEBUG("\n%s(): sendRequest for Metadata failed.\n",
		__FUNCTION__);
	return ASD_FAILURE;
    }
	ASDEBUG("\%s(): LINE: %d .\n", __FUNCTION__, __LINE__);

    if (parseJSONText(data, &pJson) != ASD_SUCCESS) {
	ASDEBUG("\nFailed to parse and get Tokens...\n");
	free(data);
	return ASD_FAILURE;
    }
	ASDEBUG("\%s(): LINE: %d .\n", __FUNCTION__, __LINE__);

    if (pJson) {
	ASDEBUG("\%s(): LINE: %d .\n", __FUNCTION__, __LINE__);

	pTokenType = cJSON_GetObjectItem(pJson, "token_type");
	pExpiresIn = cJSON_GetObjectItem(pJson, "expires_in");
	pAccessToken = cJSON_GetObjectItem(pJson, "access_token");

	if (pTokenType == NULL && pExpiresIn == NULL
	    && pAccessToken == NULL) {
	    ASDEBUG("\nFailed to parse and get Tokens...\n");
	    cJSON_Delete(pJson);
	    free(data);
	    return ASD_FAILURE;
	}
	ASDEBUG("\%s(): LINE: %d .\n", __FUNCTION__, __LINE__);

	strcpy(sAccessTok_g.cTokenType, pTokenType->valuestring);
	sAccessTok_g.pExpiresIn = pExpiresIn->valueint;
	strcpy(sAccessTok_g.cAccessToken, pAccessToken->valuestring);
	cJSON_Delete(pJson);
    }
	ASDEBUG("\%s(): LINE: %d .\n", __FUNCTION__, __LINE__);
    free(data);
    return ASD_SUCCESS;
}

void freeMetadata(Metadata_t * pMetadata)
{
    ASDEBUG("\nCalled: %s \n", __FUNCTION__);
    if (pMetadata) {
	if (pMetadata->pRelease_version)
	    free(pMetadata->pRelease_version);
	if (pMetadata->pRelease_fcs_date)
	    free(pMetadata->pRelease_fcs_date);
	if (pMetadata->pImage_guid)
	    free(pMetadata->pImage_guid);
	if (pMetadata->pImage_name)
	    free(pMetadata->pImage_name);
	if (pMetadata->pImage_size)
	    free(pMetadata->pImage_size);
	if (pMetadata->pMd5_checksum)
	    free(pMetadata->pMd5_checksum);
	if (pMetadata->pMetadata_trans_id)
	    free(pMetadata->pMetadata_trans_id);
	free(pMetadata);
    }
}

void freeDownloadInfo(DownLoadInfo_t * pDownLoadInfo)
{
    ASDEBUG("\nCalled: %s \n", __FUNCTION__);
    if (pDownLoadInfo) {
	if (pDownLoadInfo->pDownloadUrl)
	    free(pDownLoadInfo->pDownloadUrl);
	if (pDownLoadInfo->pImageFullName)
	    free(pDownLoadInfo->pImageFullName);
	free(pDownLoadInfo);
    }
}

Metadata_t *newMetaData()
{
    ASDEBUG("\nCalled: %s \n", __FUNCTION__);
    Metadata_t *pMetadata;

    pMetadata = (Metadata_t *) malloc(sizeof(Metadata_t));
    pMetadata->pRelease_version = NULL;
    pMetadata->pRelease_fcs_date = NULL;
    pMetadata->pImage_guid = NULL;
    pMetadata->pImage_name = NULL;
    pMetadata->pImage_size = NULL;
    pMetadata->pMd5_checksum = NULL;
    pMetadata->pMetadata_trans_id = NULL;

    return pMetadata;
}

int getMetaData(ASDParams_t * pASDParams, Metadata_t ** pMetadat)
{
    char *data;
    char cUDI[100];		// from "PID: " + pid + ", VID: " + vid + ", SN: " + serialNo
    char cMetaDataReqStr[512];
    char *headers[] = { };
    char *pEncodUDI;
    Metadata_t *pMetadata;
    int retval = 0;

    memset(cMetaDataReqStr, 0, sizeof(cMetaDataReqStr));
    memset(cUDI, 0, sizeof(cUDI));

    sprintf(cUDI, "PID: %s, VID: %s, SN: %s",
	    pASDParams->cPId, pASDParams->cVId, pASDParams->cSNo);

    encodeData(cUDI, strlen(cUDI), &pEncodUDI);

    headers[0] = cHeaderStr;

    sprintf(cMetaDataReqStr,
	    "%s/%s/mdf_id/%s/software_type_id/%s/current_release/%s?output_release=LATEST",
	    URL_METADATA_REQ, pEncodUDI, pASDParams->cMDFId,
	    pASDParams->cSWTId, pASDParams->cCurVer);

    ASDEBUG("\nURL Request Str= %s\n", cMetaDataReqStr);

    if (sendRequest(cMetaDataReqStr, headers, NULL, &data) != ASD_SUCCESS) {
	ASDEBUG("\n%s(): sendRequest for Metadata failed.\n",
		__FUNCTION__);
        swu_syslog(LOG_ERR, " ASD Client send request for Metadata failed.\r\n");
	freeEncodedData(pEncodUDI);
	return ASD_FAILURE;
    }

    if (data) {
	ASDEBUG("\ndata= %s\n", data);

	//pMetadata = (Metadata_t *) malloc(sizeof(Metadata_t));
	pMetadata = newMetaData();
	if (!pMetadata) {
	    free(data);
	    freeEncodedData(pEncodUDI);
	    return ASD_FAILURE;
	}

	retval = parseMetadata(data, &pMetadata);
	if ( retval != ASD_SUCCESS) {
	    freeMetadata(pMetadata);
	    free(data);
	    freeEncodedData(pEncodUDI);
	    return retval;
	}

	ASDEBUG("\n%s(): pReleaseVer = %s\n", __FUNCTION__,
		pMetadata->pRelease_version);
	ASDEBUG("\n%s(): pReleaseDate = %s\n", __FUNCTION__,
		pMetadata->pRelease_fcs_date);
	ASDEBUG("\n%s(): pImage_guid = %s\n", __FUNCTION__,
		pMetadata->pImage_guid);
	ASDEBUG("\n%s(): pImage_name = %s\n", __FUNCTION__,
		pMetadata->pImage_name);
	ASDEBUG("\n%s(): pImage_size = %s\n", __FUNCTION__,
		pMetadata->pImage_size);
	ASDEBUG("\n%s(): pMd5_checksum = %s\n", __FUNCTION__,
		pMetadata->pMd5_checksum);
	ASDEBUG("\n%s(): pMetadataTransId = %s\n", __FUNCTION__,
		pMetadata->pMetadata_trans_id);
	*pMetadat = pMetadata;
    }

    free(data);
    freeEncodedData(pEncodUDI);

    return ASD_SUCCESS;
}

int getDownloadUrl(ASDParams_t * pASDParams, Metadata_t * pMetadat,
		   DownLoadInfo_t ** pDownLoadInfo)
{
    char *data;
    //char cUDI[100];		// from "PID: " + pid + ", VID: " + vid + ", SN: " + serialNo
    //char cDownloadUrlReq[512];
    char cUDI[512];		// from "PID: " + pid + ", VID: " + vid + ", SN: " + serialNo
    char cDownloadUrlReq[1024];
    char *headers[] = { };
    char *pEncodUDI;

    memset(cDownloadUrlReq, 0, sizeof(cDownloadUrlReq));
    memset(cUDI, 0, sizeof(cUDI));

    sprintf(cUDI, "PID: %s, VID: %s, SN: %s",
	    pASDParams->cPId, pASDParams->cVId, pASDParams->cSNo);
    encodeData(cUDI, strlen(cUDI), &pEncodUDI);

    headers[0] = cHeaderStr;

    sprintf(cDownloadUrlReq,
	    "%s/%s/mdf_id/%s/image_guid/%s/metadata_trans_id/%s",
	    URL_DOWNLOADURL_REQ, pEncodUDI, pASDParams->cMDFId,
	    pMetadat->pImage_guid, pMetadat->pMetadata_trans_id);

    ASDEBUG("\nURL Request Str= %s\n", cDownloadUrlReq);

    if (sendRequest(cDownloadUrlReq, headers, NULL, &data) != ASD_SUCCESS) {
	freeEncodedData(pEncodUDI);
	return ASD_FAILURE;
    }

    if (parseDownloadInfo(data, pDownLoadInfo) != ASD_SUCCESS) {
	freeDownloadInfo(*pDownLoadInfo);
	freeEncodedData(pEncodUDI);
	return ASD_FAILURE;
    }
    freeEncodedData(pEncodUDI);

    return ASD_SUCCESS;
}

/*
 * Function to apply downloaded file.
 */
int applyDownloadedFile(ASDParams_t * pASDParams,
			char *pInScript, DownLoadInfo_t * pDownLoadInfo)
{
    int fnameLen = 0;
    char *pOutFileName;
    FILE *in;
    extern FILE *popen();
    char buff[512];

    pOutFileName = (char *) malloc(strlen(pDownLoadInfo->pImageFullName)
				   + strlen(pASDParams->cPath)
				   + strlen(pInScript) + 3);

    if (pOutFileName == NULL)
	return ASD_FAILURE;

    sprintf(pOutFileName, "%s %s/%s", pInScript, pASDParams->cPath,
	    pDownLoadInfo->pImageFullName);

    ASDEBUG("\n %s(): Apply file %s \n", __FUNCTION__, pOutFileName);
    if (!(in = popen(pOutFileName, "r"))) {
	ASDEBUG("\n%s(): %s Command failed .\n", __FUNCTION__,
		pOutFileName);
	pclose(in);
	if (pOutFileName)
	    free(pOutFileName);
	return ASD_FAILURE;
    }
    while (fgets(buff, sizeof(buff), in) != NULL) {
	ASDEBUG("\n read buff= %s\n", buff);
    }
    pclose(in);

    if (pOutFileName) {
	free(pOutFileName);
    }
    return ASD_SUCCESS;
}

/*
 * Function to download actual file.
 */
int downloadActualFile(ASDParams_t * pASDParams,
		       Metadata_t * pMetaData,
		       DownLoadInfo_t * pDownLoadInfo)
{
    int fnameLen = 0;
    char *pOutFile;
    char *headers[] = { };

    headers[0] = cHeaderStr;
    pOutFile =
	(char *) calloc(1, strlen(pDownLoadInfo->pImageFullName) +
			strlen(pASDParams->cPath) + 2);

    if (pOutFile == NULL)
	return ASD_FAILURE;

    sprintf(pOutFile, "%s/%s", pASDParams->cPath,
	    pDownLoadInfo->pImageFullName);

    ASDEBUG("\n %s():LINE= %d : file %s \n", __FUNCTION__, __LINE__, pOutFile);

    swu_syslog(LOG_INFO, " ASD Client started downloading file %s .\r\n", pOutFile);

    ASDEBUG("\n %s():LINE= %d \n", __FUNCTION__, __LINE__);

    if (downloadFile(pDownLoadInfo->pDownloadUrl, headers, pOutFile) !=
	ASD_SUCCESS) {
	ASDEBUG("\n %s(): failed to download file %s \n", __FUNCTION__,
		pOutFile);
        swu_syslog(LOG_ERR, " ASD Client failed to download file %s .\r\n", pOutFile);
	free(pOutFile);
	return ER_DOWNLOAD_FAILED;
    }

    if (md5_checksum_validate(pMetaData->pMd5_checksum, pOutFile) !=
	ASD_SUCCESS) {
        swu_syslog(LOG_ERR, " ASD Client Checksum failed for file %s .\r\n", pOutFile);
	ASDEBUG("\n ASD Client Checksum failed for file %s \n", pOutFile);
	free(pOutFile);
	return ER_MD5SUM_CHECK_FAILED;
    }
    swu_syslog(LOG_INFO, " ASD Client successfully downloaded %s\r\n", pOutFile);
    updateDownloadStatus(ASD_SUCCESS, pOutFile);

    free(pOutFile);
    return ASD_SUCCESS;
}

int parseDownloadInfo(char *pData, DownLoadInfo_t ** pDownLoadInfo)
{
    DownLoadInfo_t *pDwnldInfo;

    cJSON *pJson = NULL;
    cJSON *pSubItem = NULL;
    cJSON *pItem = NULL;

    if (parseJSONText(pData, &pJson) != ASD_SUCCESS) {
	ASDEBUG("\n*** Failed to parse...\n");
	return ASD_FAILURE;
    }

    if (pJson) {
	pDwnldInfo = (DownLoadInfo_t *) malloc(sizeof(DownLoadInfo_t));

	if (pDwnldInfo == NULL) {
	    cJSON_Delete(pJson);
	    return ASD_FAILURE;
	}

	pItem = cJSON_GetObjectItem(pJson, "download_info_list");
	pSubItem = cJSON_GetArrayItem(pItem, 0);
	pItem = cJSON_GetObjectItem(pSubItem, "download_url");

	getObjectValue(pItem, &pDwnldInfo->pDownloadUrl);

	pItem = cJSON_GetObjectItem(pSubItem, "image_full_name");

	getObjectValue(pItem, &pDwnldInfo->pImageFullName);

	*pDownLoadInfo = pDwnldInfo;
    }
    cJSON_Delete(pJson);

    return ASD_SUCCESS;
}

int parseMetadata(char *pData, Metadata_t ** pMetadata)
{
    Metadata_t *pMtadata;

    cJSON *pJson = NULL;
    cJSON *pSubItem = NULL;
    cJSON *pItem = NULL;
    cJSON *pExceptionItem = NULL;
    cJSON *pExceptionCode = NULL;

    cJSON *pMetadataResp = NULL;

    cJSON *pReleaseVer = NULL;
    cJSON *pReleaseDate = NULL;
    cJSON *pMetadataTransId = NULL;
    cJSON *pImage_guid = NULL;
    cJSON *pImage_name = NULL;
    cJSON *pImage_size = NULL;
    cJSON *pMd5_checksum = NULL;
    int imgArrySize = 0, i = 0;
    char exceptionCode[50];

    ASDEBUG("\n%s(): Parse Metadata: LINE: %d ...\n", __FUNCTION__,
	    __LINE__);

    pMtadata = *pMetadata;

    if (parseJSONText(pData, &pJson) != ASD_SUCCESS) {
	ASDEBUG("\n%s(): failed to parse Metadata\n", __FUNCTION__);
	return ASD_FAILURE;
    }

    if (pJson) {
	pMetadataResp = cJSON_GetObjectItem(pJson, "metadata_response");
	if (pMetadataResp) {
	    pMetadataTransId =
		cJSON_GetObjectItem(pMetadataResp, "metadata_trans_id");
	    if (pMetadataTransId == NULL) {
		ASDEBUG("\n%s(): failed to parse metadata_trans_id.\n",
			__FUNCTION__);
		return ASD_FAILURE;
	    }
	    pItem = cJSON_GetObjectItem(pMetadataResp, "metadata_id_list");
	    if (pItem == NULL) {
		ASDEBUG("\n%s(): failed to parse metadata_id_list.\n",
			__FUNCTION__);
		return ASD_FAILURE;
	    }
            //ASDEBUG("\n%s(): ASD METADATA LST: %s .\n", __FUNCTION__, cJSON_Print(pItem));
	    pExceptionItem = cJSON_GetObjectItem(pItem, "asd_metadata_exception");
            ASDEBUG("\n%s(): ASD EXCP LST: %s .\n", __FUNCTION__, pExceptionItem->valuestring);
            if(pExceptionItem)
	    {
                pExceptionCode = cJSON_GetObjectItem(pExceptionItem, "exception_code");
                if(pExceptionCode) {
                    memset(exceptionCode, 0, sizeof(exceptionCode));
                    sprintf(exceptionCode, "%s",pExceptionCode->valuestring);
                    ASDEBUG("\n%s(): ASD Exception Code: %s .\n", __FUNCTION__, exceptionCode);
                    if(strstr(exceptionCode, "NO_DATA_FOUND") != NULL)
                    {
                        swu_syslog(LOG_ERR, " ASD Server returned NO_DATA_FOUND exception for Metadata request.\r\n");
		        return ASD_MTDATA_EXCEPTION;
                    }
                    else
                    {
		        return ASD_FAILURE;
                    }
                }
	    }
	    pItem = cJSON_GetObjectItem(pItem, "software_list");
	    if (pItem == NULL) {
		ASDEBUG("\n%s(): failed to parse software_list.\n",
			__FUNCTION__);
		return ASD_FAILURE;
	    }
	    pItem = cJSON_GetObjectItem(pItem, "platform_list");
	    if (pItem == NULL) {
		ASDEBUG("\n%s(): failed to parse platform_list.\n",
			__FUNCTION__);
		return ASD_FAILURE;
	    }

	    pSubItem = cJSON_GetArrayItem(pItem, 0);


	    pItem = cJSON_GetObjectItem(pSubItem, "release_list");

	    pSubItem = cJSON_GetArrayItem(pItem, 0);

	    pReleaseVer = cJSON_GetObjectItem(pSubItem, "release_version");
	    if (pReleaseVer == NULL) {
		ASDEBUG("\n%s(): failed to parse release_version.\n",
			__FUNCTION__);
		return ASD_FAILURE;
	    }
	    pReleaseDate =
		cJSON_GetObjectItem(pSubItem, "release_fcs_date");
	    pItem = cJSON_GetObjectItem(pSubItem, "image_details");

	    imgArrySize = cJSON_GetArraySize(pItem);

	    for (i = 0; i < imgArrySize; i++) {
		pSubItem = cJSON_GetArrayItem(pItem, i);

		pImage_guid = cJSON_GetObjectItem(pSubItem, "image_guid");

		if (pImage_guid->type == cJSON_NULL)
		    continue;
		pImage_name = cJSON_GetObjectItem(pSubItem, "image_name");
		pImage_size = cJSON_GetObjectItem(pSubItem, "image_size");
		pSubItem =
		    cJSON_GetObjectItem(pSubItem, "image_checksums");
		pMd5_checksum =
		    cJSON_GetObjectItem(pSubItem, "md5_checksum");

		getObjectValue(pReleaseVer, &pMtadata->pRelease_version);
		getObjectValue(pReleaseDate, &pMtadata->pRelease_fcs_date);
		getObjectValue(pImage_guid, &pMtadata->pImage_guid);
		getObjectValue(pImage_name, &pMtadata->pImage_name);
		getObjectValue(pImage_size, &pMtadata->pImage_size);
		getObjectValue(pMd5_checksum, &pMtadata->pMd5_checksum);
		getObjectValue(pMetadataTransId,
			       &pMtadata->pMetadata_trans_id);
	    }
            if(pMtadata->pRelease_version == NULL
               || pMtadata->pRelease_fcs_date == NULL
               || pMtadata->pImage_guid == NULL
               || pMtadata->pImage_name == NULL
               || pMtadata->pImage_size == NULL
               || pMtadata->pMd5_checksum == NULL)
            {
		return ASD_FAILURE;
            }

	    *pMetadata = pMtadata;
	}

	cJSON_Delete(pJson);
    }
    return ASD_SUCCESS;
}

void getObjectValue(cJSON * pObj, char **pOutVal)
{
    char *pVal;
    if (pObj->type != cJSON_NULL)
    {
        pVal = (char *) calloc(1, strlen(pObj->valuestring) + 1);
        if(!pVal)
        {
          *pOutVal = NULL;
          return;
        }
        strcpy(pVal, pObj->valuestring);
    }
    else
    {
	pVal = NULL;
    }
    *pOutVal = pVal;
}


/* Parse text to JSON and return cJson object */
int parseJSONText(char *text, cJSON ** pJsonObj)
{
    cJSON *json;
    json = cJSON_Parse(text);
    if (!json) {
	ASDEBUG("Error before: [%s]\n", cJSON_GetErrorPtr());
    } else {
	*pJsonObj = json;
	return ASD_SUCCESS;
    }
    return ASD_FAILURE;
}

/* Parse text to JSON, then render back to text, and print! */
void parseJSON(char *text)
{
    char *out;
    cJSON *json;

    json = cJSON_Parse(text);
    if (!json) {
	ASDEBUG("Error before: [%s]\n", cJSON_GetErrorPtr());
    } else {
	//parse_object(json);
	out = cJSON_Print(json);
	cJSON_Delete(json);
	ASDEBUG("\n %s():\n%s\n", __FUNCTION__, out);
	free(out);
    }
}

void parse_object(cJSON * root)
{
    cJSON *name = NULL;
    cJSON *index = NULL;
    cJSON *optional = NULL;

    int i;

    cJSON *item = cJSON_GetObjectItem(root, "error");
    cJSON *subitem = item->child;
    for (i = 0; i < cJSON_GetArraySize(subitem); i++) {
	cJSON *subitem = cJSON_GetArrayItem(item, i);
	name = cJSON_GetObjectItem(subitem, "error-type");
	index = cJSON_GetObjectItem(subitem, "error-tag");
	optional = cJSON_GetObjectItem(subitem, "error-message");
    }
}

/*
 * Function to remove leading and trailing spaces.
 */
char *trim_string(char *string_p)
{
    char *pInBuffer = string_p, *pOutBuff = string_p;
    int i = 0, count_i = 0;

    if (string_p) {
	for (pInBuffer = string_p; *pInBuffer && isspace(*pInBuffer);
	     ++pInBuffer);
	if (string_p != pInBuffer)
	    memmove(string_p, pInBuffer, pInBuffer - string_p);

	while (*pInBuffer) {
	    if (isspace(*pInBuffer) && count_i)
		pInBuffer++;
	    else {
		if (!isspace(*pInBuffer))
		    count_i = 0;
		else {
		    *pInBuffer = ' ';
		    count_i = 1;
		}
		pOutBuff[i++] = *pInBuffer++;
	    }
	}
	pOutBuff[i] = '\0';

	while (--i >= 0) {
	    if (!isspace(pOutBuff[i]))
		break;
	}
	pOutBuff[++i] = '\0';
    }
    return string_p;
}
