/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_proto.h                                      	*	
 *         Description  : Helper Library contains list manipulation function    *
 *			  prototypes used across the system. 			*
 *  *****************************************************************************/

/*! \file help_proto.h
 \brief File contains common API prototypes used across all modules the software
*/

/** \addtogroup LIBHELP */
/* @{ */


#ifndef _HELP_PROTO_H
#define _HELP_PROTO_H


#include "help_structs.h"

/*! 
        \brief API to store objlist value to tmp file for later use
	\param[in] pxObj Objlist ptr
	\param[in] pcPath Path where to create a tmp file
	\return  UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
int help_storeLocalDB(IN ObjList *pxObj, 
					IN const char *pcPath);

/*! 
        \brief API to construct objlist from tmp file
	\param[out] pxObj Objlist ptr
	\param[in] pcPath Path where to fetch the values from
	\return  ObjList on successful / UGW_FAILURE on failure
*/
int help_loadLocalDB(OUT ObjList *pxObj, 
						IN const char *pcPath);


/*!  \brief  API to add an add object node to the head node
  \param[in] pxObjList Pointer to the List head object node
  \param[in] pcObjName Name of the Object
  \param[in] unSid Service Id corresponding to the Object
  \param[in] unOid Parameter Id
  \param[in] unSubOper Suboperation at object level(add, del, modify)
  \param[in] unObjFlag Identify access, dynamic, etc
  \return  
*/

ObjList *  help_addObjList(IN ObjList *pxObjList,
							IN const char *pcObjName,
							IN uint16_t unSid,
							IN uint16_t unOid,
							IN uint32_t unSubOper,
							IN uint32_t unObjFlag);

/*!  \brief  API to add a add param node to Objlist node
  \param[in] pxObjList Pointer to the List head object node
  \param[in] pcParamName Name of the Parameter
  \param[in] unParamId Parameter Id
  \param[in] pcParamValue Parameter Value
  \param[in] unFlag Holds masks type, dynamic, modified information in bit
  \return Values in paramList on successful / Proper code on failure
*/
ParamList * help_addParamList(IN ObjList *pxObjList,
							IN const char *pcParamName, 
							IN uint16_t unParamId,
							IN const char *pcParamValue,
							IN uint32_t unFlag);

/*!  \brief  API to dump/print the Object Info from the provided Object List
  \param[in] pxObj Void ptr 
  \param[in] unSubOper Suboperation to print objects(objlist, attrlist, acslist)
  \return  
*/
void help_printObj(IN void *pxObj,
				 IN uint32_t unSubOper);

/*!  \brief  API to print paramlist information 
  \param[in] pxParamList ParamList node ptr
  \return
*/
void help_printParamList(IN ParamList *pxParamList);

/*!  \brief  API to free the object info from the Object List structure
  \param[in] pObj Pointer to the List head object node
  \param[in] unSubOper  Suboperation to identify object type
  \param[in] unFlag EMPTY_OBJLIST - empty objlist for reuse, FREE_OBJLIST - free objlist completely
  \return  
*/
void help_delObj(IN void *pObj,
				IN uint32_t unSubOper,
				IN uint32_t unFlag);

/*!  \brief  API to free the object info from the Object List structure
  \param[in] pObj Pointer to the List head object node
  \param[in] pcObjName Object name
  \param[in] unFlag Flag to identify object type
  \return  
*/
int help_delCurObj(IN void *pObj, 
				IN const char *pcObjName,  
				IN uint32_t unFlag);
/*!  \brief  API to free the paramlist 
  \param[in] pParam Pointer paramlist ptr
  \param[in] unFlag Flag to identify paramlist type type
  \return  
*/
void help_delParam(IN void *pParam,
				IN uint32_t unFlag);

/*!  \brief  API to get object count 
  \param[in] pxObjList Pointer to the objlist
  \param[in] pnObjCnt Total number of object node count in given list
  \return  
*/
void help_getObjListCnt(IN ObjList *pxObjList, 
					OUT int32_t *pnObjCnt);


/*!  \brief  API to get parameter count 
  \param[in] pxObjList Pointer to the objlist
  \param[in] pcObjName Object name under which parmater node count to get 
  \param[in] pParamCnt Total number of parameter count
  \return  
*/
void help_getParamListCnt( IN ObjList *pxObjList,
					IN char8_t *pcObjName,
					OUT int32_t *pParamCnt);

/*!  \brief  Function to copy src object "as is" to dst object
  \param[in] pSrcObj Void ptr
  \param[in] unFlag Flag to define object type(objlist, attrobjlist, acsobjlist)
  \param[out] pDstObj Void ptr
  \return
*/
void help_copyObjList(OUT void *pDstObj ,
						IN uint32_t unFlag,
						IN void *pSrcObj); 

/*!  \brief  API to copy src object complete objlist "as is" to dst object
  \param[in] pDst Void ptr
  \param[in] unFlag Flag to define object type(objlist, attrobjlist, acsobjlist)
  \param[out] pSrc Void ptr
  \return
*/
void help_copyCompleteObjList(OUT void *pDst , 
						IN uint32_t unFlag , 
						IN void *pSrc);
/*!  \brief  API to add and traverse paramlist alone, used for notification 
  \param[in] pxSrcParam Void ptr
  \param[in] pcParamName Parameter name
  \param[in] unParamId Parameter Id
  \param[in] pcParamValue Parameter Value
  \param[in] unFlag Holds masks type, dynamic, modified information in bit
  \return
*/
OUT ParamList * help_paramListOnly(IN ParamList *pxSrcParam ,
					IN const char *pcParamName, 
					IN uint16_t unParamId,
					IN const char *pcParamValue,
					IN uint32_t unFlag);


/*!  \brief API to construct attrlist for object with requested attribute values
  \param[in] pxObjAttrList Attrobj head node ptr 
  \param[in] pcObjName Object name
  \param[in] pcWebName Webname for requested object
  \param[in] unObjAttrFlag  Holds access, multiinst information in bits
  \return  
*/
ObjAttrList * help_addObjAttrList(IN ObjAttrList *pxObjAttrList,
								IN const char *pcObjName,
								IN const char *pcWebName,
								IN uint32_t unObjAttrFlag);

/*!  \brief API to construct attrlist for parameter with requested attribute values
  \param[in] pxObjAttrList Attrobj head node ptr 
  \param[in] pcParamName Parameter name
  \param[in] pcParamProfile Parameter profile
  \param[in] pcParamWebName Parameter webname
  \param[in] pcParamValidVal Validation values with comma separated
  \param[in] pcParamDefaultVal Default value of the parameter
  \param[in] unMinVal Minimum value if the parameter type is int 
  \param[in] unMaxVal Maximum value if the parameter type is int
  \param[in] unMinLen Minimum length if the parameter type is string
  \param[in] unMaxLen Maximum length if the parameter type is string
  \param[in] unParamFlag  Holds access, syntax, permit information in bits
  \return  
*/

OUT ParamAttrList * help_addParamAttrList(IN ObjAttrList *pxObjAttrList,
							IN const char *pcParamName,
							IN const char *pcParamProfile,
							IN const char *pcParamWebName,
							IN const char *pcParamValidVal,
							IN const char *pcParamDefaultVal,
							IN uint32_t unMinVal,                      
							IN uint32_t unMaxVal,                      
							IN uint32_t unMinLen,                      
							IN uint32_t unMaxLen,                      
							IN uint32_t unParamFlag);      

/*!  \brief API to construct ACS attrlist for object with requested attribute values
  \param[in] pxObjACSList Object list with ACS attribute supported
  \param[in] sObjName Object name
  \param[in] unObjOper Suboperation for the given object MODIFY
  \param[in] unObjFlag Flag
  \return  
*/
OUT ObjACSList*  help_addAcsObjList(IN ObjACSList *pxObjACSList,
								IN const char sObjName[MAX_LEN_OBJNAME], 
								IN uint32_t unObjOper,             
								IN uint32_t unObjFlag);             

/*!  \brief API to construct ACS attrlist for parameter with requested attribute values
  \param[in] pxObjACSList Object list with ACS attribute supported
  \param[in] sParamName Parameter name
  \param[in] sParamValue Parameter value
  \param[in] nParamFlag  Holds syntax, InformToAcs, SaveToFlash, Notification, AccessControl, ChangeFlag information in bits
  \return  
*/
OUT ParamACSList * help_addAcsParamList(IN ObjACSList *pxObjACSList,
							IN const char sParamName[MAX_LEN_PARAM_NAME],
							IN const char sParamValue[MAX_LEN_PARAM_VALUE],
							IN uint32_t nParamFlag);

/*! \brief  API to update particular parameter node in given objlist
        \param[in] pxDstObjList Objlist list ptr where parameter values needed to update
	\param[in] pcObjname Object name 
	\param[in] pcParamName Parameter name
	\param[in] pcParamValue Parameter value to update
	\param[in] unParamId Parameter id to update
	\param[in] unParamFlag Parameter flag to update
        \return Destination objlist parameter value updated on successful
*/
uint32_t help_editNode (INOUT ObjList *pxDstObjList, 
						IN char *pcObjname, 
						IN char *pcParamName, 
						IN char *pcParamValue,
						IN uint32_t unParamId,
						IN uint32_t unParamFlag);

/*! \brief  API to update particular parameter node in given objlist ptr
        \param[in] pxDstObjList Objlist list ptr where parameter values need to be updated
	\param[in] pcObjname Object name 
	\param[in] pcParamName Parameter name
	\param[in] pcParamValue Parameter value to update
	\param[in] unParamId Parameter id to update
	\param[in] unParamFlag Parameter flag to update
        \return Destination objlist parameter value updated on successful
*/
uint32_t help_editSelfNode (INOUT ObjList *pxDstObjList, 
						IN char *pcObjname, 
						IN char *pcParamName, 
						IN char *pcParamValue,
						IN uint32_t unParamId,
						IN uint32_t unParamFlag);


/*! \brief  API to update particular parameter node in given objlist
        \param[in] pxObjDst Objlist list ptr where parameter values needed to update
        \param[in] pxObjSrc Source objlist ptr
        \return Source objlist is merged with destination objlist on successful
*/
uint32_t help_mergeObjList(INOUT ObjList *pxObjDst,
						IN ObjList *pxObjSrc);
/*!
	\brief API to copy paramlist from one list to another
	\param[in] pxSrc Source paramlist ptr
	\param[in] pxDst Destination objlist ptr
	\return  Paramlist with values is copied to pxdst on successful 
*/
void help_copyParamList(OUT ParamList *pxDst,
						OUT ParamList *pxSrc);


/*! 
        \brief API to get the values from the given objlist structure 
	\param[in] pxObj Objlist ptr
	\param[in] unOid ObjectId
	\param[in] unInstance Instance number
	\param[in] unParamId Parameter ID
	\param[in] pcVal Parameter value 
	\return Pcval Value of the parameter on successful / failure
*/
int help_getValue (IN ObjList *pxObj, IN uint32_t unOid, IN uint32_t unInstance, IN uint32_t unParamId, OUT char *pcVal);

/*! 
        \brief API to get value of specfic parameter from the given objlist structure through object/parameter name match
	\param[in] pxObj Objlist ptr
	\param[in] pcObjName Object name
	\param[in] unInstance Instance number
	\param[in] pcParamname Parameter name
	\param[out] pcVal Parameter value
	\return value of the parameter when successful
*/
int help_getValueNameBased(IN ObjList *pxObj, IN char *pcObjName, IN uint32_t unInstance, IN char *pcParamName, OUT char *pcVal);

/*! 
        \brief Macro to set the values in the given objlist structure 
	\param[in] pxObj Objlist ptr
	\param[in] unOid ObjectId
	\param[in] unInstance Instance number
	\param[in] unParamId Parameter ID
	\param[in] pcVal Parameter value
	\return  Updated objlist on successful / failure
*/
int help_setValue (INOUT ObjList *pxObj, IN uint32_t unOid, IN uint32_t unInstance, IN uint32_t unParamId, IN char *pcVal); 

/*! 
        \brief API to set value of specfic parameter to the given objlist structure through object/parameter name match
	\param[in] pxObj Objlist ptr
	\param[in] unOid ObjectId
	\param[in] unInstance Instance number
	\param[in] unParamId Parameter ID
	\param[in] pcVal Parameter value
	\return  Updated objlist on successful / failure
*/
int help_setValueNameBased(IN ObjList *pxObj, IN char *pcObjName, IN uint32_t unInstance, IN char *pcParamName, OUT char *pcVal);

/*! 
        \brief API to traverse objlist and get objlist ptr based on the defined condition
	\param[in] pxObj Objlist ptr
	\param[in] paramName Parameter name to match  
	\param[in] paramValue Parameter value to match
	\return ObjList on successful / UGW_FAILURE on failure
*/
OUT ObjList* help_getObjPtr(IN ObjList *pxObj, 
						IN const char *paramName, 
						IN const char *paramValue);
/*! 
        \brief  API to objlist to get the objlist ptr based on the condition
	\param[in] pxHeadObj Head Objlist ptr
	\param[in] pxObj Object to be deleted from the list
	\return  ObjList on successful / UGW_FAILURE on failure
*/
OUT int help_delObjPtr(INOUT ObjList *pxHeadObj, IN ObjList *pxObj);

/*! 
        \brief  API to move object from one list to another and remove from the original list
	\param[in] pxDstObj Destination(dst) Objlist ptr
	\param[in] pxSrcObj Source(src) Objlist ptr 
	\param[in] pcObjName Name to move objlist from src to dst
	\param[in] unFlag Flag to identify objlist
	\return ObjList on successful / UGW_FAILURE on failure
*/
OUT int help_moveObjList(INOUT ObjList *pxDstObj, 
							IN ObjList *pxSrcObj, 
							IN const char * pcObjName, 
							IN uint32_t unFlag);

/*! 	\brief  API to check if obj list ptr is empty or not
        \param[in] pxObj Obj ptr
        \return UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
OUT int help_isEmptyObj(IN ObjList *pxObj);

/*! 	\brief  API to check if obj list included parameters or not
        \param[in] pxObj Obj ptr
        \return UGW_SUCCESS on successful / UGW_FAILURE on failure
*/
OUT int help_isEmptyObjParam(IN ObjList *pxObj);

#endif  //#ifndef _HELP_PROTO_H
/* @} */
