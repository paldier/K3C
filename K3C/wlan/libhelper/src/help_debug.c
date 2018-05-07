/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/* ****************************************************************************** 
 *         File Name    : ugw_debug.c                                           *
 *         Description  : Memory Debug Library to check Memory Leaks		*
 * ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "help_objlist.h"

#ifdef MEM_DEBUG
#define OFFSET 0

MemDbgUtil *pxHead;

/*  =============================================================================
 *   Function Name 	: help_storeMemInfo					*
 *   Description 	: Function to updated internal list for every allocation* 
 *   			  and free						*
 *  ============================================================================*/
static MemDbgUtil* help_storeMemInfo(void *vId,uint32_t iSize,char *pcFile, uint32_t iLine)
{
	MemDbgUtil *pxNew, *pxPrev, *pxCurr;
	if((pxNew = calloc(1,sizeof(MemDbgUtil))) != NULL)
	{
		pxNew->vId = vId;
		pxNew->iSize = iSize;
		pxNew->iLine = iLine;
		strcpy(pxNew->acFile,pcFile+OFFSET);
		pxNew->pxNext = NULL;
	}
	else
	{
		LOGF_LOG_CRITICAL("Memory allocation failed\n");		
	}
	if(pxHead == NULL)
	{
		pxHead = pxNew;
	}
	else
	{
		pxPrev = pxCurr = pxHead;
		while((pxCurr!=NULL) && (pxCurr->pxNext != NULL))
		{
			pxPrev = pxCurr;
			pxCurr = pxCurr->pxNext;
		}
		if(pxCurr != NULL)
		{
			pxCurr->pxNext = pxNew;
		}
		else
		{
			pxPrev->pxNext = pxNew;
		}
	}
	return pxNew;
}

/*  =============================================================================
 *   function name 	: help_locatememinfo					*
 *   description 	: function to get memory information from internal list *
 *  ============================================================================*/
static MemDbgUtil* help_locateMemInfo(void *vId, MemDbgUtil **pxPrevNode)
{
	MemDbgUtil *pxCurr, *pxPrev=NULL;
	pxCurr = pxHead;
	while((pxCurr!=NULL) && (pxCurr->vId != vId))
	{
		pxPrev = pxCurr;
		pxCurr = pxCurr->pxNext;
	}
	if(pxPrevNode != NULL)
	{
		*pxPrevNode = pxPrev;
	}
	return pxCurr;
}

/*  =============================================================================
 *   function name 	: help_bytesAllocated					*
 *   description 	: function to get number of allocated bytes  		*
 *  ============================================================================*/
static int32_t help_bytesAllocated(int32_t *piNumBytes)
{
	int32_t iCount=0,iNumAllocs=0;
	MemDbgUtil *pxCurr;
	pxCurr = pxHead;
	while(pxCurr != NULL)
	{
		iCount += pxCurr->iSize;
		iNumAllocs++;
		pxCurr = pxCurr->pxNext;
	}
	if(piNumBytes != NULL)
	{
		*piNumBytes = iCount;
	}
	return iNumAllocs;	
}

#endif

/*  =============================================================================
 *   function name 	: help_free						*
 *   description 	: function to free the allocated bytes  		*
 *  ============================================================================*/
int help_free(void* pcFree, char *pcFile, uint32_t iLine)
{
#ifdef MEM_DEBUG
	MemDbgUtil *pxCurr, *pxPrev;
	if((pxCurr = help_locateMemInfo(pcFree,&pxPrev)) != NULL)
	{
		if(pxPrev != NULL)
		{
			pxPrev->pxNext = pxCurr->pxNext;
		}
		else
		{
			pxHead = pxCurr->pxNext;
		}
		free(pxCurr);
		pxCurr = NULL;
	}
	else{
		LOGF_LOG_DEBUG("Already Freed %p Line: %d File: %s\n",pcFree,iLine,pcFile+OFFSET);			
	}

	if(pcFree)
	{
		int iSize1, iNumAllocs;
		iNumAllocs=help_bytesAllocated(&iSize1);
		LOGF_LOG_DEBUG("Freed %p Line: %d  File: %s TotalAllocs = %d\n",pcFree,iLine,pcFile+OFFSET,iNumAllocs);			
		free(pcFree);	
		pcFree = NULL;
	}
	return UGW_SUCCESS;
#else
	UNUSED_VAR(pcFile);
	UNUSED_VAR(iLine);
	if(pcFree)
        {
                free(pcFree);
		pcFree = NULL;
        }
        return UGW_SUCCESS;
#endif
}

/*  =============================================================================
 *   function name 	: help_callocDbg						*
 *   description 	: function to  allocate memory   			*
 *  ============================================================================*/
void*  help_calloc(uint32_t uiNum, uint32_t iSize, char *pcFile, uint32_t iLine)
{
#ifdef MEM_DEBUG
	MemDbgUtil *pxMemDbg;
	int32_t iNumAllocs, iSize1=0;
	void *pTemp;

	pTemp = calloc(uiNum,iSize + 1);
	if(pTemp == NULL)
	{
		LOGF_LOG_CRITICAL("Memory allocation failed\n");	  
		return NULL;
	}

	if((pxMemDbg = help_storeMemInfo(pTemp,iSize,pcFile,iLine)) == NULL)
	{
		help_free(pTemp,pcFile,iLine);
		return NULL;
	}
	iNumAllocs=help_bytesAllocated(&iSize1);
	LOGF_LOG_DEBUG("Alloc at addr %p ,Bytes Alloc %d ,File %s line %d , TotalAllocs = %d\n",
			pTemp,(iSize*uiNum),pcFile+OFFSET,iLine,iNumAllocs);			
	memset(pTemp, 0, iSize);
	return pTemp;
#else
	UNUSED_VAR(pcFile);
	UNUSED_VAR(iLine);
	return calloc(uiNum,iSize);
#endif
}

/*  =============================================================================
 *   function name 	: help_malloc						*
 *   description 	: function to  allocate memory   			*
 *  ============================================================================*/
void*  help_malloc(uint32_t unSize, char *pcFile, uint32_t iLine)
{
#ifdef MEM_DEBUG
	return help_calloc(1,unSize,pcFile,iLine);
#else
	UNUSED_VAR(pcFile);
	UNUSED_VAR(iLine);
	return calloc(1,unSize);
#endif
}

/*  =============================================================================
 *   function name 	: help_realloc						*
 *   description 	: function to  allocate memory   			*
 *  ============================================================================*/
void* help_realloc(void *pPtr,uint32_t unSize, char *pcFile, uint32_t iLine)
{

#ifdef MEM_DEBUG
	MemDbgUtil *pxMemDbg;
	int32_t iNumAllocs, iSize1=0;
	void *pTemp;

	pTemp = realloc(pPtr,unSize);

	if(pTemp == NULL)
	{
		LOGF_LOG_CRITICAL("Memory allocation failed\n");	  
		return NULL;
	}

	if((pxMemDbg = help_storeMemInfo(pTemp,unSize,pcFile,iLine)) == NULL)
	{
		help_free(pTemp,pcFile,iLine);
		return NULL;
	}
	iNumAllocs=help_bytesAllocated(&iSize1);
	LOGF_LOG_DEBUG("Alloc at addr %p ,Bytes Alloc %d ,File %s line %d , TotalAllocs = %d\n",
			pTemp,(unSize*1),pcFile+OFFSET,iLine,iNumAllocs);			


	HELP_FREE(pPtr);

	return pTemp;
#else
	UNUSED_VAR(pcFile);
	UNUSED_VAR(iLine);
	void *pTmp;
	pTmp = realloc(pPtr,unSize);
	return pTmp;
#endif

}

/*  =============================================================================
 *   function name 	: help_memInfo						*
 *   description 	: function to  dump allocated memory details   		*
 *  ============================================================================*/
void help_memInfo()
{
#ifdef MEM_DEBUG
	MemDbgUtil *pxCurr;
	pxCurr = pxHead;
	fprintf(stderr,"\n---------------MEMORY INFO START--------------");
	fprintf(stderr,"\n----------------------------------------------");
	fprintf(stderr,"\n Address | Size  | Line | File name");
	fprintf(stderr,"\n----------------------------------------------");
	while(pxCurr != NULL){
		fprintf(stderr,"\n%8x | %5u | %4d | %s",(uint32_t)pxCurr->vId,pxCurr->iSize,pxCurr->iLine,
				pxCurr->acFile);
		pxCurr = pxCurr->pxNext;
	}
	fprintf(stderr,"\n----------------------------------------------");
	fprintf(stderr,"\n---------------MEMORY INFO END----------------\n");
#else
	printf("Debug not enabled \n");
#endif
}
