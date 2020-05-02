
#ifndef _ASD_REQUEST_H_
#define _ASD_REQUEST_H_

int sendRequest(char *pURL, char *headers[], char *pPostData, char **pOutData);
void encodeData(char *pStr, int len, char **pEncodData);
void freeEncodedData(char *pData);
int downloadFile(char *url, char *headers[], char *pOutFileName);

#endif /* _ASD_REQUEST_H_ */
