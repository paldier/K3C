/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_objmsg.h                                         *	
 *         Description  : Common Helper Library contains functions, 		*
 *			  defines, structs, and enums used across the modules 	*
 *			  like CAL, CAPI, CSD, Servd, and Management Entities 	*	
 *  *****************************************************************************/


/*! \file help_objmsg.h
\brief File contains the Constants, enumerations, related Data
    structures and API's common for all modules in LQ software.
*/

/** \addtogroup LIBHELP */
/* @{ */

#ifndef _HELP_OBJMSG_H
#define _HELP_OBJMSG_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "help_proto.h"
#include "help_debug.h"
#include "help_logging.h"
#include "help_defs.h"
#include "help_error.h"


#define var(z) Obj##z		/*!< To get unified variable name on every declaration */
#define decl(x) var(x)         /*!< Define varible with line number */
#define OBJ  decl(__LINE__)    /*!< To get line number */


/*!
	\brief Allocates memory for given number of bytes
	\param[in] unBytes Number of bytes to allocate
 	\return On Successful returns number of bytes allocated ptr
*/
#define HELP_MALLOC(unBytes) help_calloc(1,unBytes,__FILE__,__LINE__);

/*!
	\brief Allocates memory for given number of bytes
	\param[in] *pSrcPtr Pointer allocate to reallocate.
	\param[in] unBytes Number of bytes to allocate
 	\return On Successful returns number of bytes allocated ptr
*/
#define HELP_REALLOC(pSrcPtr, unBytes) help_realloc(pSrcPtr,unBytes,__FILE__,__LINE__);


/*!
	\brief Allocates memory for given number of bytes
	\param[in] unBytes Number of bytes to allocate
	\param[in] unNum Number of elementes to allocate
 	\return On Successful returns number of bytes allocated ptr
*/
#define HELP_CALLOC(unNum, unBytes) help_calloc(unNum,unBytes,__FILE__,__LINE__);


/*!
	\brief Frees up the memory allocated using calloc, malloc
	\param[in] ptr Pointer to free
 	\return On Successful allocated memory gets freed
*/
#define HELP_FREE(ptr) help_free(ptr,__FILE__,__LINE__);


/*!
	\brief  Dumps the allocated and freed memory details
 	\return On Successful prints memory information
*/
static inline void HELP_DUMP_MEMINFO(void)
{
	help_memInfo();
}

static inline void * HELP_CREATE_OBJ(IN uint32_t unSubOper); 

/*!
	\brief Macro to create object list based on the sub-operation type
	\param[in] unSubOper Differentiates the objlist type
	\return Void ptr(it can be either objlist, objattrlist, objacslist) returned
*/
static inline void * HELP_CREATE_OBJ(IN uint32_t unSubOper) 
{
	if (OBJLIST(unSubOper))
	{ 
		ObjList *__pxTmpObj; 
		__pxTmpObj = HELP_MALLOC(sizeof(ObjList));
		if(!__pxTmpObj)
		{
			LOGF_LOG_CRITICAL(" malloc failed\n");
			return NULL;
		}
		INIT_LIST_HEAD(&(__pxTmpObj->xOlist)); 
		return (void*)__pxTmpObj;
	}
	else if (OBJATTRLIST(unSubOper))
	{
		ObjAttrList *__pxTmpAttrObj; 
		__pxTmpAttrObj = HELP_MALLOC(sizeof(ObjAttrList));
		if(!__pxTmpAttrObj)
		{
			LOGF_LOG_CRITICAL(" malloc failed\n");
			return NULL;
		}
		INIT_LIST_HEAD(&(__pxTmpAttrObj->xOlist)); 
		return (void*)__pxTmpAttrObj;

	}
	else if (IS_SOPT_OBJACSATTR(unSubOper))
	{
		ObjACSList *__pxTmpAcsObj; 
		__pxTmpAcsObj = HELP_MALLOC(sizeof(ObjACSList));
		if(!__pxTmpAcsObj)
		{
			LOGF_LOG_CRITICAL(" malloc failed\n");
			return NULL;
		}
		INIT_LIST_HEAD(&(__pxTmpAcsObj->xOlist)); 
		return (void*)__pxTmpAcsObj;
	}
	else
	{
		LOGF_LOG_ERROR(" Pass Proper Objlist Flag (i.e SOPT_OBJVALUE, SOPT_OBJATTR, SOPT_OBJACSATTR\n");
		return NULL;
	}
}

/*!
	\brief Macro to delete object list based on the sub-operation type
	\param[in] pxObj Msg head ptr
	\param[in] unSubOper Identify the objlist structure.
	\param[in] unFlag 0-empty objlist for re-use 1-free objlist completely
	\return 
*/
#define HELP_DELETE_OBJ(pxObj, unSubOper, unFlag) \
					help_delObj(pxObj,unSubOper,unFlag);\
					if(unFlag == FREE_OBJLIST) \
						pxObj = NULL; 
/*!
	\brief Macro to delete current object list based on the sub-operation type
	\param[in] pxMsgObj Complete msg on where to remove the current object
	\param[in] pcObjName Objname based on the objname remove obj node from the list
	\param[in] pxObj Current object for reference
	\param[in] unSubOper Object list type
	\return Removes requested objnode from list
*/
#define  HELP_DELETE_CURRENT_OBJ(pxMsgObj, pcObjName, pxObj, unSubOper) \
	if (OBJLIST(unSubOper)) \
	{ \
		void *pTmp;\
		pTmp = list_entry(pxObj->xOlist.next,ObjList,xOlist); /*TODO*/ \
		help_delCurObj(pxMsgObj,pcObjName,unSubOper); \
		pxObj = pTmp; \
	} \

/*!
	\brief Macro to create param list for notification
	\param[in] unSubOper Differentiates the objlist type
	\return Parameter list ptr 
*/
static inline void * HELP_CREATE_PARAM(IN uint32_t unSubOper) 
{
	if (OBJLIST(unSubOper))
	{ 
		ParamList *__pxTmpParam; 
		__pxTmpParam = HELP_MALLOC(sizeof(ParamList));
		if(!__pxTmpParam)
		{
			LOGF_LOG_CRITICAL(" malloc failed\n");
			return NULL;
		}
		INIT_LIST_HEAD(&(__pxTmpParam->xPlist)); 
		return (void*)__pxTmpParam;
	}
	else
	{
		LOGF_LOG_INFO(" Attribute list handling\n");
	}
	return UGW_SUCCESS;
}

/*!
	\brief Macro to delete paramlist 
	\param[in] pxParam Paramlist ptr
	\param[in] unSubOper
	\return 
*/
#define HELP_DELETE_PARAM(pxParam, unSubOper) \
			help_delParam(pxParam,unSubOper);\
			pxParam = NULL;

/*!
	\brief Fills the objlist for get request on objects
	\param[in] pxObj Msg void ptr
	\param[in] pcObjName Objname
	\param[in] unSubOper Objlist type (objlist, attrlist, acslist)
	\return Void ptr which is used in paramlist addition
*/

static inline OUT void * HELP_OBJECT_GET(IN void *pxObj, IN const char *pcObjName, IN uint32_t unSubOper)
{
	if(OBJLIST(unSubOper))
	{
		return help_addObjList(pxObj,pcObjName,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE);
	}
	else if (OBJATTRLIST(unSubOper))
	{
		return help_addObjAttrList(pxObj,pcObjName,NO_ARG_VALUE,NO_ARG_VALUE);
	}
	else
	{
		LOGF_LOG_ERROR("Error in Objlist Construction, Pass proper flag(i.e SOPT_OBJVALUE, SOPT_OBJATTR\n");
		return NULL;
	}
}	

/*!
	\brief Fills the paramlist which is part of objlist to get request on parameters
	\param[in] pxObj Void ptr
	\param[in] pcParamName Parameter name 
	\param[in] unSubOper Objlist type (objlist, attrlist, acslist)
	\return 
*/

static inline void  HELP_PARAM_GET(IN void *pxObj, IN const char *pcParamName, IN uint32_t unSubOper)
{
	if(OBJLIST(unSubOper))
	{
		help_addParamList(pxObj,pcParamName,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE);
	}
	else if (OBJATTRLIST(unSubOper))
	{
		help_addParamAttrList(pxObj,pcParamName,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE,
						  NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE,NO_ARG_VALUE);
	}
	else
	{
		LOGF_LOG_ERROR("Error in Objlist Construction, Pass proper flag(i.e SOPT_OBJVALUE, SOPT_OBJATTR\n");
	}
}

/*!
	\brief Constructs objlist for get/set request
	\param[in] pxObj Msg void ptr
	\param[in] pcObjName Objname
	\param[in] unOper Operation (ADD, DEL, MODIFY)
	\param[in] unFlag Flag
	\return Void ptr which is used to create ACS paramlist
*/
static inline void * HELP_ACS_OBJ_CONSTRUCT(IN void *pxObj, IN const char *pcObjName, IN uint32_t unOper, IN uint32_t unObjFlag)
{
	return help_addAcsObjList(pxObj, pcObjName, unOper, unObjFlag);
}	

/*!
	\brief  Fills paramlist for get request on parameters
	\param[in] pxObj Void ptr
	\param[in] pcParamName Parameter name 
	\param[in] pcParamValue Parameter value 
	\param[in] unSubOper Objlist type (objlist,attrlist,acslist)
	\return 
*/
static inline void *  HELP_ACS_PARAM_CONSTRUCT(IN void *pxObj, IN const char *pcParamName, IN const char *pcParamValue, IN uint32_t unParamFlag)
{
	return help_addAcsParamList(pxObj, pcParamName, pcParamValue, unParamFlag);
}

/*!
	\brief Fills objlist for set request on objects
	\param[in] pxObj Msg void ptr
	\param[in] pcObjName Objname
	\param[in] unSid Service id number
	\param[in] unOper Operation (ADD, DEL, MODIFY)
	\param[in] unSubOper Objlist type (objlist, attrlist, acslist)
	\return Void ptr which is used to create paramlist
*/
static inline void * HELP_OBJECT_SET(IN void *pxObj, IN const char *pcObjName, IN uint32_t unSid, IN uint32_t unOper,IN uint32_t unSubOper)
{
	(void)unSubOper;
	return help_addObjList(pxObj,pcObjName,unSid,NO_ARG_VALUE,unOper,NO_ARG_VALUE);

}	

/*!
	\brief Fills paramlist for get request on parameters
	\param[in] pxObj Void ptr
	\param[in] pcParamName Parameter name 
	\param[in] pcParamValue Parameter value 
	\param[in] unSubOper Objlist type (objlist, attrlist, acslist)
	\return 
*/
static inline void *  HELP_PARAM_SET(IN void *pxObj, IN const char *pcParamName, IN const char *pcParamValue, IN uint32_t unSubOper)
{
	(void)unSubOper;
	return help_addParamList(pxObj,pcParamName,NO_ARG_VALUE,pcParamValue,NO_ARG_VALUE);
}

/*!
	\brief Fills the paramlist on notification
	\param[in] pxParam Void ptr
	\param[in] pcParamName Parameter name
	\param[in] pcParamValue Parameter value
	\param[in] unSubOper Objlist type (objlist, attrlist, acslist)
	\return 
*/

static inline void  HELP_ONLY_PARAM_SET(IN void *pxParam, IN const char *pcParamName, IN const char *pcParamValue, IN uint32_t unSubOper)
{
	(void)unSubOper;
	help_paramListOnly(pxParam,pcParamName,NO_ARG_VALUE,pcParamValue,NO_ARG_VALUE);
}
/*!
	\brief Copies the objlist from one list to another
	\param[in] pSrc Source objlist ptr
	\param[in] unSubOper Objtype
	\param[in] pDst Destination objlist ptr
	\param[in] nFlag Set to 0(copy complete object == COPY_COMPLETE_OBJ), 1(current object = COPY_SINGLE_OBJ)
	\return Pdst copies the obj with values, on successful 
*/
static inline void  HELP_COPY_OBJ(OUT void *pDst, IN void *pSrc, IN uint32_t unSubOper, IN uint8_t nFlag)
{
	if (nFlag == COPY_SINGLE_OBJ)
	 	help_copyObjList(pDst,unSubOper,pSrc);
	else
	 	help_copyCompleteObjList(pDst,unSubOper,pSrc);
}
/*!
	\brief Copies the paramlist from one list to another
	\param[in] pSrc Source paramlist ptr
	\param[in] pDst Destination objlist ptr
	\return Pdst copies the paramlist with values
*/
static inline void  HELP_COPY_PARAM(OUT void *pDst, IN void *pSrc)
{
	help_copyParamList(pDst,pSrc);
}

/*!
	\brief Checks if the given object is empty or not
	\param[in] pxObj Objlist  ptr
	\return UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
static inline int  HELP_IS_EMPTY_OBJ(IN ObjList *pxObj)
{
	 return help_isEmptyObj(pxObj);
}

/*!
	\brief Checks if the given object includes parameters or not
	\param[in] pxObj Objlist  ptr
	\return UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
static inline int  HELP_IS_EMPTY_OBJ_PARAM(IN ObjList *pxObj)
{
	return help_isEmptyObjParam(pxObj);
}

/*!
	\brief Prints recieved or constructed objlist
	\param[in] pxObj Objlist ptr
	\param[in] unSubOper Get objtype 
	\return 
*/
static inline void  HELP_PRINT_OBJ(IN void *pxObj, IN uint32_t unSubOper)
{
	 help_printObj(pxObj,unSubOper);
}

/*!
	\brief Prints recieved or constructed objlist
	\param[in] pxParam Param list ptr
	\param[in] unSubOper Get objtype
	\return 
*/
static inline void  HELP_PRINT_PARAM(IN void *pxParam, IN uint32_t unSubOper)
{
	 (void)unSubOper;
	 help_printParamList(pxParam);
}

/*! \brief  Updates the particular parameter node in the given objlist if param node found, else adds new param node
        \param[in] pxDstObjList Objlist list ptr where parameter values need to be updated
	\param[in] pcObjname Object name 
	\param[in] pcParamName Parameter name
	\param[in] pcParamValue Parameter value to update
	\param[in] unParamId Parameter id
	\param[in] unParamFlag Parameter flag
        \return Destination objlist parameter value updated on successful / ugw_failure on failure
*/
static inline int HELP_EDIT_NODE (INOUT ObjList *pxDstObjList, IN char *pcObjname, IN char *pcParamName, IN char *pcParamValue, 
										IN uint32_t unParamId, IN uint32_t unParamFlag)
{
	return help_editNode(pxDstObjList,pcObjname,pcParamName,pcParamValue,unParamId,unParamFlag);
}

/*! \brief  Updates particular parameter node in given object node if param node found, else adds new param node
        \param[in] pxDstObjList Objlist list ptr where parameter values need to be updated
	\param[in] pcObjname Object name 
	\param[in] pcParamName Parameter name
	\param[in] pcParamValue Parameter value to update
	\param[in] unParamId Parameter id
	\param[in] unParamFlag Parameter flag
        \return Destination objlist parameter value updated on successful / ugw_failure on failure
*/
static inline int HELP_EDIT_SELF_NODE (INOUT ObjList *pxDstObjList, IN char *pcObjname, IN char *pcParamName, IN char *pcParamValue, 
										IN uint32_t unParamId, IN uint32_t unParamFlag)
{
	return help_editSelfNode(pxDstObjList,pcObjname,pcParamName,pcParamValue,unParamId,unParamFlag);
}

/*! 
        \brief  Moves object from one list to another and removes from the original list
        \param[in] pxDstObj dst Objlist ptr
        \param[in] pxSrcObj src Objlist ptr 
        \param[in] pcObjName Name to move objlist from src to dst
        \param[in] unFlag Flag to identify objlist
        \return  ObjList on successful / UGW_FAILURE on failure
*/
static inline int HELP_MOVEOBJLIST(INOUT ObjList *pxDstObj,
                                                        IN ObjList *pxSrcObj,
                                                        IN const char * pcObjName,
                                                        IN uint32_t unFlag)
{
	return help_moveObjList(pxDstObj, pxSrcObj, pcObjName, unFlag);
}

/*! \brief  Updates particular parameter node in given objlist
        \param[in] pxObjDst Objlist list ptr where parameter values need to be updated
        \param[in] pxObjSrc Source objlist ptr
        \return Source objlist merged with destinition on successful / err_merge_failed on failure
*/
static inline int HELP_MERGE_OBJLIST (INOUT ObjList *pxObjDst,IN ObjList *pxObjSrc)
{
	return help_mergeObjList(pxObjDst,pxObjSrc);
}

/*! 
        \brief Macro to get the values from the given objlist structure 
	\param[in] pxObj Objlist ptr
	\param[in] unOid ObjectId
	\param[in] unInstance Instance number
	\param[in] unParamId Parameter ID
	\param[in] pcVal Parameter value
	\return Pcval value of the parameter on successful
*/
static inline int HELP_SL_GET (IN ObjList *pxObj, IN uint32_t unOid, IN uint32_t unInstance, IN uint32_t unParamId, OUT char *pcVal) 
{
	return help_getValue(pxObj,unOid,unInstance,unParamId,pcVal);
}

/*! 
        \brief Macro to get value of specfic parameter from the given objlist structure through object/parameter name match
	\param[in] pxObj Objlist ptr
	\param[in] pcObjName Object name
	\param[in] unInstance Instance number
	\param[in] pcParamname Parameter name
	\param[out] pcVal Parameter value
	\return value of the parameter when successful
*/
static inline int HELP_SL_GET_NAME_BASED (IN ObjList *pxObj, IN char *pcObjName, IN uint32_t unInstance, IN char *pcParamName, OUT char *pcVal)
{
	return help_getValueNameBased(pxObj, pcObjName ,unInstance, pcParamName, pcVal);
}

/*! 
        \brief Macro to set the values in the given objlist structure 
	\param[in] pxObj Objlist ptr
	\param[in] unOid ObjectId
	\param[in] unInstance Instance number
	\param[in] unParamId Parameter ID
	\param[in] pcVal Parameter value \ 
	\return Updated objlist on successful 
*/
static inline int HELP_SL_SET (INOUT ObjList *pxObj, IN uint32_t unOid, IN uint32_t unInstance, IN uint32_t unParamId, IN char *pcVal) 
{
	return help_setValue(pxObj,unOid,unInstance,unParamId,pcVal);
}

/*! 
        \brief Macro to set value of specfic parameter to the given objlist structure through object/parameter name match
	\param[in] pxObj Objlist ptr
	\param[in] unOid ObjectId
	\param[in] unInstance Instance number
	\param[in] unParamId Parameter ID
	\param[in] pcVal Parameter value
	\return Updated objlist on successful 
*/
static inline int HELP_SL_SET_NAME_BASED (INOUT ObjList *pxObj, IN char *pcObjName, IN uint32_t unInstance, IN char *pcParamName, IN char *pcVal) 
{
	return help_setValueNameBased(pxObj, pcObjName, unInstance, pcParamName, pcVal);
}

/*! 
        \brief Stores objlist value to tmp file for later use
	\param[in] pxObj Objlist ptr
	\param[in] pcPath Path to create a tmp file
	\return UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
static inline int HELP_STORELOCALDB(IN ObjList *pxObj, IN const char *pcPath)
{
	return help_storeLocalDB(pxObj, pcPath);
}

/*! 
        \brief Constructs objlist from tmp file
	\param[out] pxObj Objlist ptr
	\param[in] pcPath Path to fetch the values
	\return Returns ObjList on successful / UGW_FAILURE on failure
*/
static inline int HELP_LOADLOCALDB(OUT ObjList *pxObj,  IN const char *pcPath)
{
	return help_loadLocalDB(pxObj, pcPath);
}

/*! 
        \brief Function to objlist to get the objlist ptr based on the condition
	\param[in] pxObj Objlist ptr
	\param[in] paramName Parameter name to match 
	\param[in] paramValue Parameter value name to value
	\return  Returns ObjList on successful / UGW_FAILURE on failure
*/
static inline OUT ObjList* HELP_GETOBJPTR(IN ObjList *pxObj, IN const char *paramName, IN const char *paramValue)
{
	return help_getObjPtr(pxObj, paramName, paramValue);
}

/*! 
        \brief  Deletes object from given objlist
	\param[in] pxHeadObj Head Objlist ptr
	\param[in] pxObj Object to be deleted from the list
	\return  UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
static inline int HELP_DELOBJPTR(INOUT ObjList *pxHeadObj, IN ObjList *pxObj)
{
	return help_delObjPtr(pxHeadObj, pxObj);
}

/*! 
        \brief Macro to loop through objlist to get object node information
	\param[in] __pxMsgObj Msg header objlist pointer 
	\param[in] __pxObj Objlist pointer used to copy the actual objlist values
*/
#define FOR_EACH_OBJ(__pxMsgObj,__pxObj)\
		ObjList * OBJ; \
		OBJ = __pxMsgObj; \
		list_for_each_entry(__pxObj,&(OBJ->xOlist),xOlist) \
/*! 
        \brief FOR_EACH_ATTROBJ macro is used to loop through objAttrlist to get object node information
	\param[in] __pxMsgObj msg header objattrlist pointer 
	\param[in] __pxObj objlist pointer used to copy the actual objlist values
*/
#define FOR_EACH_OBJATTR(__pxMsgObj,__pxObj)\
		ObjAttrList * OBJ; \
		OBJ = __pxMsgObj; \
		list_for_each_entry(__pxObj,&(OBJ->xOlist),xOlist) \

/*! 
        \brief Macro to loop through ACS objAttrlist to get object node information
	\param[in] __pxMsgObj Msg header ACS objattrlist pointer 
	\param[in] __pxObj ACS objlist pointer used to copy the actual objlist values
*/
#define FOR_EACH_OBJ_ACS_ATTR(__pxMsgObj,__pxObj)\
		ObjACSList * OBJ; \
		OBJ = __pxMsgObj; \
		list_for_each_entry(__pxObj,&(OBJ->xOlist),xOlist) \


/*! 
        \brief Macro to loop through paramlist
        \param[in] __pxObj Objlist pointer 
        \param[in] __pxParam Paramlist  pointer used to copy the actual paramlist values
*/
#define  FOR_EACH_PARAM(__pxObj,__pxParam)\
		list_for_each_entry(__pxParam,&(__pxObj->xParamList.xPlist),xPlist) \

/*! 
        \brief Macro to loop through paramlist
        \param[in] __pxObj ObjiAttrlist pointer 
        \param[in] __pxParam Paramlist  pointer used to copy the actual paramattrlist values
*/
#define  FOR_EACH_PARAM_ATTR(__pxObj,__pxParam)\
		list_for_each_entry(__pxParam,&(__pxObj->xParamAttrList.xPlist),xPlist) \

/*! 
        \brief Macro to loop through paramlist
        \param[in] __pxObj ObjAcsAttrlist pointer 
        \param[in] __pxParam Paramlist  pointer used to copy the actual paramattrlist values
*/
#define  FOR_EACH_PARAM_ACS_ATTR(__pxObj,__pxParam)\
		list_for_each_entry(__pxParam,&(__pxObj->xParamAcsList.xPlist),xPlist) \

/*! 
        \brief Macro to loop through paramlist used in case of notification
        \param[in] __pxSrc Paramlist pointer 
        \param[in] __pxParam Paramlist  pointer used to copy the actual paramlist values
*/
#define  FOR_EACH_PARAM_ONLY(__pxSrc,__pxParam)\
		list_for_each_entry(__pxParam,&(__pxSrc->xPlist),xPlist) \

/*! 
        \brief Function to get objname from objlist 
	\param[in] __pxObjName Objnode ptr
	\return Returns objname on successful 
*/
static inline char * GET_OBJ_NAME(IN ObjList * __pxObjName)
{
	return  __pxObjName->sObjName;
}

/*! 
        \brief Function to get sid from objlist 
	\param[in] __pxObjSid Objnode ptr
	\return Returns sid on successful 
*/
static inline int16_t GET_OBJ_SID(IN ObjList * __pxObjSid)
{
	return  (int16_t)__pxObjSid->unSid;
}

/*! 
        \brief Function to get oid from objlist 
	\param[in] __pxObjOid Objnode ptr
	\return Returns oid on successful 
*/

#define GET_OBJ_OID(__pxObjOid) \
		(int16_t)__pxObjOid->unOid

/*! 
        \brief Function to get subOper from objlist 
	\param[in] __pxObjSubOper Objnode ptr
	\return Returns subOper(add, delete, modify) on successful 
*/
static inline uint32_t GET_OBJ_SUBOPER(IN ObjList * __pxObjSubOper)
{
	return  __pxObjSubOper->unObjOper;
}

/*! 
        \brief Function to get object flag from objlist 
	\param[in] __pxObjFlag Objnode ptr
	\return Flag(access, dynamic, etc) on successful 
*/
static inline uint32_t GET_OBJ_FLAG(IN ObjList * __pxObjFlag)
{
	return  __pxObjFlag->unObjFlag;
}

/*! 
        \brief Function to get paramname from paramlist 
	\param[in] __pxParamName Param Node ptr
	\return Param name on successful 
*/
static inline char * GET_PARAM_NAME(IN ParamList * __pxParamName)
{
	return  __pxParamName->sParamName;
}

/*! 
        \brief Function to get paramvalue from paramlist 
	\param[in] __pxParamValue Param Node ptr
	\return Param value on successful 
*/
static inline char * GET_PARAM_VALUE(IN ParamList * __pxParamValue)
{
	return  __pxParamValue->sParamValue;
}

/*! 
        \brief Function to get paramId from paramlist 
	\param[in] __pxParamId Param Node ptr
	\return ParamId on successful 
*/
#define GET_PARAM_ID(__pxParamId) \
		(int16_t)__pxParamId->unParamId

/*! 
        \brief Function to get paramFlag from paramlist 
	\param[in] __pxParamFlag Param Node ptr
	\return Param Flag(dynamic, type, modified, etc) on successful 
*/
static inline uint32_t GET_PARAM_FLAG(IN ParamList * __pxParamFlag)
{
	return  __pxParamFlag->unParamFlag;
}

/*! 
        \brief Function to get objname  from ObjAttrList
	\param[in] __pxAttrObjname ObjAttr Node ptr
	\return ObjAttr objname on successful
*/
static inline char * GET_ATTR_OBJNAME(IN ObjAttrList * __pxAttrObjname)
{
	return  __pxAttrObjname->sObjName;
}

/*! 
        \brief Function to get  web name  from ObjAttrList
	\param[in] __pxAttrWebName ObjAttr Node ptr
	\return ObjAttr web name on successful
*/
static inline char * GET_ATTR_WEBNAME(IN ObjAttrList * __pxAttrWebName)
{
	return  __pxAttrWebName->sObjWebName;
}

/*! 
        \brief Function to get  unObjAttr  from ObjAttrList
	\param[in] __pxAttrFlag ObjAttr Node ptr
	\return ObjAttr flag on successful
*/
#define GET_ATTR_FLAG(__pxAttrFlag) \
		(int32_t)__pxAttrFlag->unObjAttr

/*! 
        \brief Function to get paramname  from ParamAttrList
	\param[in] __pxParamAttrName ParamAttrlist Node ptr
	\return Param name on successful
*/
static inline char * GET_ATTR_PARAMNAME(IN ParamAttrList * __pxParamAttrName)
{
	return  __pxParamAttrName->sParamName;
}

/*! 
        \brief Function to get  param profile  from ParamAttrList
	\param[in] __pxParamAttrProfile ParamAttrlist Node ptr
	\return Param profile on successful
*/
static inline char * GET_ATTR_PARAMPROFILE(IN ParamAttrList * __pxParamAttrProfile)
{
	return  __pxParamAttrProfile->sParamProfile;
}

/*! 
        \brief Function to get  param webname  from ParamAttrList
	\param[in] __pxParamAttrWebName paramAttrlist Node ptr
	\return Param web name on successful
*/
static inline char * GET_ATTR_PARAMWEBNAME(IN ParamAttrList * __pxParamAttrWebName)
{
	return  __pxParamAttrWebName->sParamWebName;
}

/*! 
        \brief Function to get  param valid values  from ParamAttrList
	\param[in] __pxParamAttrValidValue ParamAttrlist Node ptr
	\return Param enums  on successful
*/
static inline char * GET_ATTR_VALIDVAL(IN ParamAttrList * __pxParamAttrValidValue)
{
	return  __pxParamAttrValidValue->sParamValidVal;
}

/*! 
        \brief Function to get  param webname  from ParamAttrList
	\param[in] __pxParamAttrValue ParamAttrlist Node ptr
	\return Param default value on successful
*/
static inline char * GET_ATTR_PARAMVALUE(IN ParamAttrList * __pxParamAttrValue)
{
	return  __pxParamAttrValue->sParamValue;
}

/*! 
        \brief Function to get  param minvalue  from ParamAttrList
	\param[in] __pxParamAttrMinValue ParamAttrlist Node ptr
	\return Param Minimum value on successful
*/
static inline uint32_t  GET_ATTR_MINVAL(IN ParamAttrList * __pxParamAttrMinValue)
{
	return  __pxParamAttrMinValue->unMinVal;
}

/*! 
        \brief Function to get  param maxvalue  from ParamAttrList
	\param[in] __pxParamAttrMaxValue ParamAttrlist Node ptr
	\return Param Maximum value on successful
*/
static inline uint32_t  GET_ATTR_MAXVAL(IN ParamAttrList * __pxParamAttrMaxValue)
{
	return  __pxParamAttrMaxValue->unMaxVal;
}

/*! 
        \brief Function to get  param minlen  from ParamAttrList
	\param[in] __pxParamAttrMinLen ParamAttrList Node ptr
	\return Param Min Len on successful
*/
static inline uint32_t  GET_ATTR_MINLEN(IN ParamAttrList * __pxParamAttrMinLen)
{
	return  __pxParamAttrMinLen->unMinLen;
}

/*! 
        \brief Function to get  param maxlen  from ParamAttrList
	\param[in] __pxParamAttrMaxLen ParamAttrList Node ptr
	\return Param Max Len on successful
*/
static inline uint32_t  GET_ATTR_MAXLEN(IN ParamAttrList * __pxParamAttrMaxLen)
{
	return  __pxParamAttrMaxLen->unMaxLen;

}

/*! 
        \brief Function to get  param flag from ParamAttrList
	\param[in] __pxParamAttrFlag ParamAttrList Node ptr
	\return Param flag(which holds access, syntax, permit, etc) on successful
*/
#define GET_ATTR_PARAMFLAG(__pxParamAttrFlag) \
				(int32_t)__pxParamAttrFlag->unParamFlag

/*! 
        \brief Function to get acslist objname
	\param[in] __pxAcsObjName Objname ptr
	\return Objname on successful
*/
static inline char * GET_ACS_OBJNAME(IN ObjACSList * __pxAcsObjName)
{
	return  __pxAcsObjName->sObjName;
}

/*! 
        \brief Function to get acslist objper
	\param[in] __pxAcsObjOper ObjOper ptr
	\return ObjOper on successful
*/
static inline uint32_t GET_ACS_OBJOPER(IN ObjACSList * __pxAcsObjOper)
{
	return  __pxAcsObjOper->unObjOper;
}

/*! 
        \brief Function to get acslist objFlag
	\param[in] __pxAcsObjFlag ObjFlag ptr
	\return ObjFlag on successful
*/
#define GET_ACS_OBJFLAG(__pxAcsObjFlag) \
			(int32_t)__pxAcsObjFlag->unObjFlag

/*! 
        \brief Function to get acslist parmname
	\param[in] __pxAcsParamName objparm ptr
	\return on successfull return paramname 
*/
static inline char * GET_ACS_PARAMNAME(IN ParamACSList * __pxAcsParamName)
{
	return  __pxAcsParamName->sParamName;
}

/*! 
        \brief Function to get acslist param accesslist value
	\param[in] __pxAcsParam Access list value ptr
	\return Accesslist info on successful
*/
static inline char * GET_ACS_ACCESSLIST(IN ParamACSList * __pxAcsParam)
{
	return  __pxAcsParam->sParamValue;
}

/*! 
        \brief Function to get acslist ParamFlag
	\param[in] __pxAcsObjFlag ParamFlag ptr
	\return ParamFlag on successful
*/
#define GET_ACS_PARAMFLAG(__pxAcsParamFlag) \
			(int32_t)__pxAcsParamFlag->unParamFlag

#endif //_HELP_OBJMSG_H 
/* @} */
