/*
 * This file holds ASD/BSD Client related structures and API
 */

#ifndef ASD_CLIENT_H
#define ASD_CLIENT_H


#define BB2_ACCESS_TOKEN_POST_DATA  "client_id=ehdazkv97u66uwhgatxa6nnt&client_secret=uuvVXby8nU3nTpfnYSxT34mR&grant_type=client_credentials";
#define PP_ACCESS_TOKEN_POST_DATA   "client_id=gp47x4ghscyrwt3qy3bp5ztk&client_secret=skWMAgMg9RYEHDJZQY4Y7PbN&grant_type=client_credentials";

#define URL_ACCESS_TOKEN_REQ "https://cloudsso.cisco.com/as/token.oauth2"
#define URL_METADATA_REQ "https://api.cisco.com/software/v2.0/metadata/udi"
//#define URL_DOWNLOADURL_REQ "https://api.cisco.com/software/v2.0/downloads/urls/base_pid"
#define URL_DOWNLOADURL_REQ "https://api.cisco.com/software/v2.0/downloads/urls/udi"
#define METADATA_HEADER_PREFIX_STR "Authorization: Bearer "

#define ASD_CMD_TYPE_CHECK "check"
#define ASD_CMD_TYPE_DOWNLOAD "download"

#define MAX_ACCTKEN_STR_LEN 150

#define ASD_SUCCESS 0
#define ASD_FAILURE -1
#define ASD_MTDATA_EXCEPTION -2



typedef struct ASDParams {
/* <pid> <vid> <sno> <mdf_id> <swt_id> <current_release> <store_file_path>
PID: RV130-K9-NA
VID: V10
MDF ID: 285026141
SWT ID: 282465789
SN: DNI12345678 
Current Release: 1.0.0.21 
*/
  char cPId[20];
  char cVId[20];
  char cSNo[20];
  char cMDFId[20];
  char cSWTId[20];
  char cCurVer[20];
  char cPath[200];
  char cType[10];
} ASDParams_t;

typedef enum __swtype_e
{
  FIRMWARE = 0,
  SIGNATURES,
  USBDRIVER
} swtype_e;

typedef struct Productinfo {
  char cPId[20];
  char cMDFId[20];
  char cSWTId[20];
  char cCurVer[20];
  swtype_e eSWType;
} ProductInfo_t;

/*

Device MDF ID     SWT ID
RV340  286287791  282465789  Other SWTs: 286289041, 286261054, 286277447, 282465797
RV340W 286289157  282465789  Other SWTs: 286289041, 286261054, 286277447, 282465797
RV345  286288298  282465789  Other SWTs: 286289041, 286261054, 286277447, 282465797
RV345P 286289211  282465789  Other SWTs: 286289041, 286261054, 286277447, 282465797

*/

ProductInfo_t Product_params[] = {
/*{"cPId", "cMDFId", "cSWTId" , "cCurVer",   swtype_e} */
  {"RV130-K9-NA", "285026141", "282465789" , "1.0.0.21", FIRMWARE},
#ifdef RU_IMAGE
  {"RV340", "286287791", "286315066" , "0.0.0.1", FIRMWARE},
#else
  {"RV340", "286287791", "282465789" , "0.0.0.1", FIRMWARE},
#endif
  {"RV340", "286287791", "286289041" , "0.0.0.1", SIGNATURES},
  {"RV340", "286287791", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV340-K9-NA", "286287791", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV340-K9-NA", "286287791", "286289041" , "0.0.0.1", SIGNATURES},
  {"RV340-K9-NA", "286287791", "286261054" , "0.0.0.1", USBDRIVER},

#ifdef RU_IMAGE
  {"RV340W", "286289157", "286315066" , "0.0.0.1", FIRMWARE},
#else
  {"RV340W", "286289157", "282465789" , "0.0.0.1", FIRMWARE},
#endif
  {"RV340W", "286289157", "286289041" , "0.0.0.1", SIGNATURES},
  {"RV340W", "286289157", "286261054" , "0.0.0.1", USBDRIVER},

#ifdef RU_IMAGE
  {"RV345", "286288298", "286315066" , "0.0.0.1", FIRMWARE},
#else
  {"RV345", "286288298", "282465789" , "0.0.0.1", FIRMWARE},
#endif
  {"RV345", "286288298", "286289041" , "0.0.0.1", SIGNATURES},
  {"RV345", "286288298", "286261054" , "0.0.0.1", USBDRIVER},

#ifdef RU_IMAGE
  {"RV345P", "286289211", "286315066" , "0.0.0.1", FIRMWARE},
#else
  {"RV345P", "286289211", "282465789" , "0.0.0.1", FIRMWARE},
#endif
  {"RV345P", "286289211", "286289041" , "0.0.0.1", SIGNATURES},
  {"RV345P", "286289211", "286261054" , "0.0.0.1", USBDRIVER},

#if 0
  { "RV160", "21230748", "282465789" , "0.0.0.1", FIRMWARE},
  { "RV160", "21230748", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV160W", "21360157", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV160W", "21360157", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV260", "21360181", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV260", "21360181", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV260P", "21360190", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV260P", "21360190", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV260W", "21360198", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV260W", "21360198", "286261054" , "0.0.0.1", USBDRIVER}
#endif
  { "RV160", "286315556", "282465789" , "0.0.0.1", FIRMWARE},
  { "RV160", "286315556", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV160W", "286316464", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV160W", "286316464", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV260", "286316476", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV260", "286316476", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV260P", "286316489", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV260P", "286316489", "286261054" , "0.0.0.1", USBDRIVER},

  {"RV260W", "286316501", "282465789" , "0.0.0.1", FIRMWARE},
  {"RV260W", "286316501", "286261054" , "0.0.0.1", USBDRIVER}

};

#define ASD_PRODINFO_PARAM_COUNT (sizeof(Product_params)/ sizeof(ProductInfo_t))

typedef struct AccessTokn {
  char cTokenType[100];
  int pExpiresIn;
  char cAccessToken[100];
} AccessTokn_t;

typedef struct Metadata {
  char *pRelease_version;
  char *pRelease_fcs_date;
  char *pImage_guid;
  char *pImage_name;
  char *pImage_size;
  char *pMd5_checksum;
  char *pMetadata_trans_id;
} Metadata_t;

typedef struct DownLoadInfo {
  char *pDownloadUrl;
  char *pImageFullName;
} DownLoadInfo_t;

#endif /* ASD_CLIENT_H */

