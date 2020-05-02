#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <syslog.h>
#include "request.h"
#include "asdcmn.h"


struct string {
  char *ptr;
  size_t len;
};

struct myprogress {
  double lastruntime;
  int    dwldPercent;
  CURL *curl;
  char  filename[MAX_STR_LEN];
};

static int progress_func(void *p,
                          double dltotal, double dlnow,
                          double ultotal, double ulnow);
static struct curl_slist * getiCurlHeaderList(char *headers[]);


size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
  size_t new_len = s->len + size*nmemb;
  s->ptr = realloc(s->ptr, new_len+1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr+s->len, ptr, size*nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size*nmemb;
}

static struct curl_slist * getiCurlHeaderList(char *headers[])
{
   int iLen = 0, i = 0;
   struct curl_slist *list = NULL;

   iLen = sizeof(headers)/sizeof(char*);

   for(i = 0; i < iLen; i++)
     list = curl_slist_append(list, headers[i]);

   return list;
}

int sendRequest(char *pURL, char *headers[], char *pPostData, char **pOutData)
{
  
  CURL *curl;
  CURLcode res;
  struct curl_slist *pHeaders=NULL;
  int iRet = 0;

  curl = curl_easy_init();
  if(curl) {
    struct string s;

    init_string(&s);

    if(headers != NULL)
    {
       pHeaders = getiCurlHeaderList(headers);
       curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);
    }

    if(pPostData != NULL)
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pPostData);

    
    curl_easy_setopt(curl, CURLOPT_URL, pURL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
      iRet = -1;
    else
      *pOutData = s.ptr;
 
    /* always cleanup */
    if(pHeaders != NULL)
       curl_slist_free_all(pHeaders);
    curl_easy_cleanup(curl);
  }
  return iRet;
}

void encodeData(char *pStr, int len, char **pEncodData)
{
  CURL *curl;

  curl = curl_easy_init();
  char *output=NULL;
  char *lclout=NULL;
  // encode URL
  //*pEncodData = curl_easy_escape(curl, pStr, len);
  output = curl_easy_escape(curl, pStr, len);
  if(output)
  {
    lclout=(char *)malloc(strlen(output)+1);
    if(!lclout)
      pEncodData = NULL;
    else
    {
      strcpy(lclout,  output);
      *pEncodData =lclout;
    }

  }
  curl_easy_cleanup(curl);
  curl_free(output);
}

void freeEncodedData(char *pData)
{
  curl_free(pData);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/* for libcurl older than 7.32.0 (CURLOPT_PROGRESSFUNCTION) */
static int progress_func(void *p,
                          double dltotal, double dlnow,
                          double ultotal, double ulnow)
{
    struct myprogress *myp = (struct myprogress *)p;
    CURL *curl = myp->curl;
    double curtime = 0;
    int percentval = 0;
    char percntchr[2];
    strcpy(percntchr, "%");

    curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &curtime);

    /* under certain circumstances it may be desirable for certain functionality
     to only run every N seconds, in order to do this the transaction time can
     be used */
    if((curtime - myp->lastruntime) >= MINIMAL_PROGRESS_FUNCTIONALITY_INTERVAL) {
        myp->lastruntime = curtime;
        //swu_syslog(LOG_INFO, "TOTAL TIME: %f \r\n", curtime);
    }

    percentval = (dlnow/dltotal) * 100;
    if(percentval > myp->dwldPercent)
    {
        myp->dwldPercent = percentval;
        //printf("\n dlnow= %f dltotal= %f percentval= %d \n", dlnow, dltotal, percentval);
        if((percentval % 5 == 0) || (percentval % 10 == 0))
        {
            ASDEBUG("\n %s():LINE= %d : percent= %d%s \n", __FUNCTION__, __LINE__, percentval, percntchr);
            updateSwDownloadStatus(STATUS_IN_PROGRESS, myp->filename, percentval);
            swu_syslog(LOG_INFO, "%s file download percentage: %d%s",
                                         myp->filename, percentval, percntchr);
        }
    }
    return 0;
}

int downloadFile(char *url, char *headers[], char *pOutFileName) 
{
    CURL *curl;
    FILE *fp;
    CURLcode res;
    struct curl_slist *pHeaders=NULL;
    int iRet = 0;
    struct myprogress prog;
    char errbuf[CURL_ERROR_SIZE];
    size_t len;

    ASDEBUG("\n %s():LINE= %d : start... \n", __FUNCTION__, __LINE__);

    curl = curl_easy_init();

    if (curl) {

       prog.lastruntime = 0;
       prog.dwldPercent = 0;
       prog.curl = curl;
       memset(prog.filename, 0, sizeof(prog.filename));

       strcpy(prog.filename, pOutFileName);

       if(headers != NULL)
       {
          ASDEBUG("\n %s():LINE= %d : get curl headers \n", __FUNCTION__, __LINE__);
          pHeaders = getiCurlHeaderList(headers);
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);
       }
          ASDEBUG("\n %s():LINE= %d : open file FD \n", __FUNCTION__, __LINE__);
       fp = fopen(pOutFileName,"wb");
       curl_easy_setopt(curl, CURLOPT_URL, url);
       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
       curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
       curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

       curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);
       /* pass the struct pointer into the progress function */ 
       curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &prog);

       curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
       /* provide a buffer to store errors in */
       curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
 
       /* set the error buffer as empty before performing a request */
       errbuf[0] = 0;

       /* complete within 60 seconds */
       //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
       curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1);
       curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 90L);

       ASDEBUG("\n %s():LINE= %d : call curl_easy_perform \n", __FUNCTION__, __LINE__);
       res = curl_easy_perform(curl);

       if(res != CURLE_OK)
       {
           len = strlen(errbuf);
           //printf("\nlibcurl: (%d) ", res);
           if(len)
           {
               swu_syslog(LOG_ERR, "%s%s", errbuf,
                  ((errbuf[len - 1] != '\n') ? "\n" : ""));
           }
           else
           {
               swu_syslog(LOG_ERR, "%s", curl_easy_strerror(res));
           }

           if(res == CURLE_OPERATION_TIMEDOUT)
           {
               swu_syslog(LOG_ERR, "Download operation timedout.");
           } 

           if(res == CURLE_REMOTE_FILE_NOT_FOUND)
           {
               updateSwDownloadStatus(STATUS_ERROR_FILE_NOT_FOUND, 
                                      pOutFileName, STATUS_ERROR_PERCENT);

           }
            else if((res == CURLE_OPERATION_TIMEDOUT) 
                 //|| (res == CURLE_NO_CONNECTION_AVAILABLE)
                 || (res == CURLE_SEND_ERROR)
                 || (res == CURLE_RECV_ERROR))
           {
               updateSwDownloadStatus(STATUS_ERROR_NETWORK_ISSUE,
                                      pOutFileName, STATUS_ERROR_PERCENT);
           }
           else
           {
               updateSwDownloadStatus(STATUS_ERROR_GENERIC,
                                      pOutFileName, STATUS_ERROR_PERCENT);
           }

           iRet = -1;
       }
       else 
       {
           updateSwDownloadStatus(STATUS_DOWNLOAD_SUCCESS,
                                  pOutFileName, STATUS_SUCCESS_PERCENT);
       }
       curl_slist_free_all(pHeaders);
       curl_easy_cleanup(curl);
       fclose(fp);
    }
    return iRet;
}


/*
 * Function to update /tmp/downloadstatus file.
 * Status:  (int)
 * 0: Success
 * 1: In progress
 * -1: Error-Generic 
 * -2: Error-File not found
 * -3: Error-Network issue
 * Filename (string)
 * Percentage (int)
 * 0-100
 */
void  updateSwDownloadStatus(int status, char *pFile, int percent)
{
    FILE *fpw;

    fpw = fopen(SWDOWNLOAD_STATUS_TMP_FILE, "w");
    if (fpw == NULL) {
	return;
    }
    fprintf(fpw, "Status:%d\n", status);
    if(pFile)
    fprintf(fpw, "Filename:%s\n", pFile);
    fprintf(fpw, "Percentage:%d\n", percent);

    fclose(fpw);
}
