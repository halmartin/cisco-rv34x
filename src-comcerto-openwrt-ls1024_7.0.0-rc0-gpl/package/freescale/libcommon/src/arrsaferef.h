/**************************************************************************
 * Copyright 2017, NXP Semiconductors
 ***************************************************************************/
/*
 * File:        saferef.h
 *
 * Description: Safe Reference module header file defines
 *
 * Authors:     	ganesh.reddy@nxp.com
 * Contributors/Review	kumara.ganji@nxp.com
 * Validation		siva.meduri@nxp.com
 *
 */

/* History
 *  Version     Date            Author                  Change Description
 *    1.0       10/01/2018      Ganesh Reddy 		Initial Development
*/
/****************************************************************************/
#ifndef _ARRSAFEREF_H_
#define _ARRSAFEREF_H_

#define SF_SUCCESS 0
#define SF_FAILURE -1

#define SF_TRUE 1
#define SF_FALSE 0

#define SF_USED 1
#define SF_NOT_USED 0

//#define SF_DBG

#ifdef SF_DBG
#define sfdbg(...)    printf(__VA_ARGS__)
#else
#define sfdbg(...)
#endif

//#define SF_DBG_SYSLOG

#ifdef SF_DBG_SYSLOG
#define sfsfdbg_syslog(x, ...) syslog(x, __VA_ARGS__)
#else
#define sfsfdbg_syslog(x, ...)
#endif



typedef struct safeRef_s {
    void *pNode;
    unsigned int ulMagic;
    unsigned int bUsed;
} safeRef_t;

typedef struct safeRef_entr_s {
    unsigned int ulMagic;
    unsigned int uIndex;
} safeRef_entr_t;

static inline unsigned int getMagic();
static inline int InsertToSafeRefArray(safeRef_t * pArr, int size,
				       void *pNode);
static inline int RemoveFromSafeRefArray(safeRef_t * pArr, int size,
					 void *pNode);
static inline int GetSafeRef(safeRef_t * pArr, int size,
			     void *pNode, safeRef_entr_t * out);
static inline void *validateSafeRefAndGetOpaque(safeRef_t * pArr,
						int size,
						safeRef_entr_t * pSafeRef);
static inline int isValidateSafeRef(safeRef_t * pArr, int size,
				    safeRef_entr_t * pSafeRef);
static inline unsigned int getMagic()
{
    static int xx = 256;
    // return random value or jiffies value or some random number
    return ++xx;
}

/*
Input: pArr, size, pNode
Output: None
Return Val: SF_SUCCESS/FAIL
*/
static inline int
InsertToSafeRefArray(safeRef_t * pArr, int size, void *pNode)
{
    int ii = 0;

    for (ii = 0; ii < size; ii++) {
	if (!pArr[ii].bUsed) {
	    pArr[ii].bUsed = SF_USED;
	    pArr[ii].pNode = pNode;
	    pArr[ii].ulMagic = getMagic();

	    sfsfdbg_syslog(LOG_WARNING,
			   "SAFEREF: Insert %p to Safe Ref with Magic %x at index %d successfully\r\n",
			   pNode, pArr[ii].ulMagic, ii);
	    sfdbg
		("SAFEREF: Insert %p to Safe Ref with Magic %x at index %d successfully\r\n",
		 pNode, pArr[ii].ulMagic, ii);
	    return SF_SUCCESS;
	}
    }
    sfsfdbg_syslog(LOG_WARNING,
		   "SAFEREF: Failed to add to SafeRefArr\r\n");
    sfdbg("SAFEREF: Failed to add to SafeRefArr\r\n");
    return SF_FAILURE;
}

/*
Input: pArr, size, pNode
Output: None
Return Val: SF_SUCCESS/FAIL
*/
static inline int
RemoveFromSafeRefArray(safeRef_t * pArr, int size, void *pNode)
{
    int ii = 0;

    for (ii = 0; ii < size; ii++) {
	if (pArr[ii].bUsed) {
	    if (pArr[ii].pNode == pNode) {
		sfsfdbg_syslog
		    (LOG_WARNING,
		     "SAFEREF: Removed %p from Safe Ref with Magic %x from index %d successfully\r\n",
		     pNode, pArr[ii].ulMagic, ii);
		sfdbg
		    ("SAFEREF: Removed %p from Safe Ref with Magic %x from index %d successfully\r\n",
		     pNode, pArr[ii].ulMagic, ii);
		pArr[ii].bUsed = 0;
		pArr[ii].pNode = NULL;
		pArr[ii].ulMagic = 0x0;

		return SF_SUCCESS;
	    }
	}
    }
    sfsfdbg_syslog(LOG_WARNING,
		   " SAFEREF: Remove from SafeRefArr failed\r\n");
    sfdbg(" SAFEREF: Remove from SafeRefArr failed\r\n");
    return SF_FAILURE;
}


/*
Input: pArr, size, pNode
Output: safeRef_entr_t*
Return Val: SF_SUCCESS/SF_FAILURE
*/
static inline int
GetSafeRef(safeRef_t * pArr, int size, void *pNode, safeRef_entr_t * out)
{
    int ii = 0;

    for (ii = 0; ii < size; ii++) {
	if (pArr[ii].bUsed) {
	    if (pArr[ii].pNode == pNode) {
		sfsfdbg_syslog(LOG_WARNING,
			       "SAFEREF: Returning with SafeRef - index %d, magic %x\r\n",
			       ii, pArr[ii].ulMagic);

		sfdbg
		    ("SAFEREF: Returning with SafeRef - index %d, magic %x\r\n",
		     ii, pArr[ii].ulMagic);
		out->uIndex = ii;
		out->ulMagic = pArr[ii].ulMagic;
		return SF_SUCCESS;
	    }
	}
    }
    sfsfdbg_syslog(LOG_WARNING,
		   "SAFEREF: No valid saference !!!!, failed\r\n");
    sfdbg("SAFEREF: No valid saference !!!!, failed\r\n");
    return SF_FAILURE;
}

/*
Input: pArr, size, pSafeRef
Output: None
Return Val: pNode/void *
*/
static inline void *validateSafeRefAndGetOpaque(safeRef_t * pArr,
						int size,
						safeRef_entr_t * pSafeRef)
{

    if (pSafeRef->uIndex >= size) {
	sfsfdbg_syslog(LOG_WARNING,
		       "SAFEREF: Requested SafeRef index is beyond DB \r\n");
	sfdbg("SAFEREF: Requested SafeRef index is beyond DB \r\n");
	return NULL;
    }
    if (pArr[pSafeRef->uIndex].bUsed) {
	if (pArr[pSafeRef->uIndex].ulMagic == pSafeRef->ulMagic) {
	    sfsfdbg_syslog
		(LOG_WARNING,
		 " SAFEREF: SafeReference is valid and magic matches successfully.\r\n");
	    sfdbg
		(" SAFEREF: SafeReference is valid and magic matches successfully.\r\n");

	    return pArr[pSafeRef->uIndex].pNode;
	} else {
	    sfsfdbg_syslog(LOG_WARNING,
			   "SAFEREF: Mismatch in magic  %x != %x at index %d \r\n",
			   pArr[pSafeRef->uIndex].ulMagic,
			   pSafeRef->ulMagic, pSafeRef->uIndex);
	    sfdbg("SAFEREF: Mismatch in magic  %x != %x at index %d \r\n",
		  pArr[pSafeRef->uIndex].ulMagic, pSafeRef->ulMagic,
		  pSafeRef->uIndex);

	}
    }
    sfsfdbg_syslog(LOG_WARNING,
		   "SAFEREF: No valid saference !!!! failed\r\n");
    sfdbg("SAFEREF: No valid saference !!!! failed\r\n");
    return NULL;
}

/*
Input: pArr, size, pSafeRef
Output: None
Return Val: SF_TRUE/SF_FALSE
*/
static inline int
isValidateSafeRef(safeRef_t * pArr, int size, safeRef_entr_t * pSafeRef)
{

    if (pSafeRef->uIndex >= size) {
	sfsfdbg_syslog(LOG_WARNING,
		       " SAFERF Requested SafeRef index is beyond DB \r\n");
	sfdbg(" SAFERF Requested SafeRef index is beyond DB \r\n");
	return SF_FALSE;
    }
    if (pArr[pSafeRef->uIndex].bUsed) {
	if (pArr[pSafeRef->uIndex].ulMagic == pSafeRef->ulMagic) {
	    sfsfdbg_syslog
		(LOG_WARNING,
		 "SAFEREF: SafeReference is valid and magic matches successfully.\r\n");
	    sfdbg
		("SAFEREF: SafeReference is valid and magic matches successfully.\r\n");

	    return SF_TRUE;
	} else {
	    sfsfdbg_syslog(LOG_WARNING,
			   "SAFEREF: Mismatch in magic  %x != %x at index %d \r\n",
			   pArr[pSafeRef->uIndex].ulMagic,
			   pSafeRef->ulMagic, pSafeRef->uIndex);

	}
    }
    sfsfdbg_syslog(LOG_WARNING,
		   "SAFEREF: No valid saference, !!!! failed\r\n");
    return SF_FALSE;
}
#endif
