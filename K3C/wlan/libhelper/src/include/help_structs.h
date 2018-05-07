/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    :  help_structs.h                                        *
 *         Description  : Common Library contains functions, defines,		*
 *			  structs, enums used across the modules like CAL, CAPI,*
 *			  CSD, Servd, and Management Entities			*
 *  *****************************************************************************/


/*! \file help_structs.h
\brief File contains the data
    structures common across all modules in UGW software
*/

#ifndef _HELP_STRUCTS_H
#define _HELP_STRUCTS_H

#include "help_enums.h"
#include "help_defs.h"
#include "list.h"

/** \addtogroup LIBHELP */
/* @{ */

/*! 
 *     \brief Contains the parameters list(attribute values)
 *     */
typedef struct
{
	char sParamName[MAX_LEN_PARAM_NAME];    /*!< Parameter Name */
	char sParamProfile[MAX_LEN_PROFILENAME];        /*!< Profile name */
	char sParamWebName[MAX_LEN_WEBNAME];        /*!< Web name  */
	char sParamValidVal[MAX_LEN_VALID_VALUE];       /*!< Valid values */
	char sParamValue[MAX_LEN_PARAM_VALUE];       /*!< Default  values */
	uint32_t unMinVal;                      /*!< Minimum value  */
	uint32_t unMaxVal;                      /*!< Maximum value  */
	uint32_t unMinLen;                      /*!< Minimum length */
	uint32_t unMaxLen;                      /*!< Maximum length  */
	uint32_t unParamFlag;                    /*!< Parameter  ACCESS[READ, WRITE], SYNTAX[Int, string, boolean, hex, datetime, etc], 
						  PERMIT[SuperAdmin,Admin,Normal,Guest] */
	ListHead xPlist;                         /*!< Traverse List */
}ParamAttrList;

/*! 
 *     \brief Contains the (objname,attr list,paramlist)
 *     */
typedef struct
{
	char sObjName[MAX_LEN_OBJNAME]; /*!< Object Name */
	char sObjWebName[MAX_LEN_WEBNAME]; /*!< Web name mapping with respect to object */
	uint32_t unObjAttr;             /*!< ACCESS[READ,WRITE], MultiInst, attribute value */
	ParamAttrList xParamAttrList;       /*!< Attribute paramlist */
	ListHead xOlist;                /*!< Traverse List */
}ObjAttrList;


/*! 
 *     \brief Contains the parameters list(attribute values)
 *     */
typedef struct
{
	char sParamName[MAX_LEN_PARAM_NAME];    /*!< Parameter Name */
	char sParamValue[MAX_LEN_PARAM_VALUE];  	/*!< AccessList param value can be NULL/subscriber*/
	uint32_t unParamFlag;                    /*!< Parameter flag which holds Notification[active,passive,disabled] */
	ListHead xPlist;                         /*!< Traverse List */
}ParamACSList;

/*! 
 *     \brief Contains the (objname, attr list, paramlist)
 *     */
typedef struct
{
	char sObjName[MAX_LEN_OBJNAME]; /*!< Object Name */
	uint32_t unObjOper;             /*!< Object Operation MODIFY */
	uint32_t unObjFlag;             	/*!< Reserved */
	ParamACSList xParamAcsList;       /*!< Attribute paramlist */
	ListHead xOlist;                /*!< Traverse List */
}ObjACSList;


/*! 
  \brief Contains the parameters list(name, value, type) that are passed in related functions and Callbacks   
  */
typedef struct
{
	char sParamName[MAX_LEN_PARAM_NAME];    /*!< Parameter Name */	
	char sParamValue[MAX_LEN_PARAM_VALUE];  /*!< Parameter Value */
	uint16_t unParamId; 			 /*!< Parameter Id */
	uint32_t unParamFlag;			/*!< Parameter to mask type, dynamic, modified, etc info in bit */
	ListHead xPlist;			 /*!< Traverse List */ 
}ParamList;

/*! 
  \brief Contains the (objname, sid, oid, paramlist) that are passed in related functions and Callbacks   
  */
typedef struct 
{
	char sObjName[MAX_LEN_OBJNAME];	/*!< Object Name */ 
	uint16_t unSid;	  		/*!< Corresponding Service Id */
	uint16_t unOid; 			/*!< Object Id */
	uint32_t unObjOper; 		/*!< Object Operation ADD, DELETE, MODIFY, etc */ 
	uint32_t unObjFlag; 		/*!< Object flag for access, dynamic, etc */
	ParamList xParamList;		/*!< Name Value Pair */
	ListHead xOlist; 		/*!< Traverse List */
}ObjList;


/* @} */

#endif //#ifndef _HELP_STRUCTS_H



