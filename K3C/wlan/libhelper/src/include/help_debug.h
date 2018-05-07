/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_debug.h                                          *
 *         Description  : Memory debugging utility				* 
 *  *****************************************************************************/

#ifndef __HELP_DEBUG_API_H 
#define __HELP_DEBUG_API_H
#include <string.h>
#include <stdlib.h>
#include "help_defs.h"

/*! \file help_debug.h
 \brief File contains structs, defines, macros related to memory debugging utility
*/

/** \addtogroup LIBHELP */
/* @{ */


/*! 
 *     \brief Keeps track of allocated and freed memory details
 */
typedef struct DEBUG 
{
	void *vId;	/*!< Id */
	uint32_t iSize;    /*!< Size in bytes */
	char acFile[100]; /*!< File name */
	uint32_t iLine;	  /*!< Line number */
	struct Dbg *pxNext; /*!< Next node */ 
}MemDbgUtil;

/*!  \brief  Function to dump allocated and freed memory details
  \return Dumps memory information in table format on successful
*/
void help_memInfo(void);

/*!  \brief  Function to allocate the memory with calloc
  \param[in] unNum Number of memory blocks to be allocated
  \param[in] unSize Number of bytes to be allocated
  \param[in] pcFile __File__ info keep track mem info
  \param[in] unLine  __LINE__ info keep track mem info
  \return  Allocated memory as requested on success, else failure
*/
OUT void* help_calloc(IN uint32_t unNum, IN uint32_t unSize, IN char *pcFile, IN uint32_t unLine);

/*!  \brief  Function to allocate the memory with malloc
  \param[in] unSize Number of bytes to be allocated
  \param[in] pcFile __File__ info keep track mem info
  \param[in] unLine  __LINE__ info keep track mem info
  \return Allocated memory as requested on success, else failure
*/
OUT void* help_malloc(IN uint32_t unSize, IN char *pcFile, IN uint32_t unLine);


/*!  \brief  Function is used to allocate the memory with realloc
  \param[in] ptr Previously allocated ptr to be realloced
  \param[in] unSize Number of bytes to be allocated
  \param[in] pcFile __File__ info keep track mem info
  \param[in] unLine  __LINE__ info keep track mem info
  \return Allocated memory as requested on success, else failure
*/
OUT void* help_realloc(IN void *ptr, IN uint32_t unSize, IN char *pcFile, IN uint32_t unLine);

/*!  \brief  Function to free the memory with free
  \param[in] ptr to free
  \param[in] pcFile __file__ info keep track mem info
  \param[in] unLine  __LINE__ info keep track mem info
  \return 
*/
int help_free(IN void *ptr, IN char *pcFile, IN uint32_t unLine);

/* @} */
#endif  /* __HELP_DEBUG_API_H */
