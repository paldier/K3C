/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_enums.h	                                        *	
 *         Description  : Common Library contains functions, defines,		*
 *			  structs, enums used across modules like CAL, CAPI,	*
 *			  CSD, Servd, and Management Entities			*
 *  *****************************************************************************/


/*! \file help_enums.h
\brief File contains all enumerations for the common library 
*/

#ifndef _HELP_ENUMS_H
#define _HELP_ENUMS_H

/** \addtogroup LIBHELP */

/* @{ */

/*! \enum SubOper
    \brief Enum containing the type of operation which includes ID_ONLY, SUB_TREE etc
    \ for per transaction 
*/
typedef enum
{
   SOPT_ID = 0x1,/*!< SET/GET_ID_ONLY */
   SOPT_OBJNAME = 0x2,/*!< SET/GET_SUB_TREE */
   SOPT_OBJVALUE = 0x4,/*!< SET/GET_COMPLETE */
   SOPT_OBJATTR = 0x8,/*!< SET/GET_ATTR_VALUE */
   SOPT_OBJACSATTR = 0x10,/*!< SET/GET_ATTR_VALUE */
   SOPT_VALIDATION = 0x20,/*!< SET/GET_VALID_VALUES */
   SOPT_DEFVAL = 0x40,/*!< SET/GET_DEFAULT_VALUE */
   SOPT_NOSUBOPER= 0x80,/*!< NO_SUBOPER */
   SOPT_LEAFNODE = 0x100,/*!< GET LEAF NODE ONLY */
   SOPT_ORGOWN = 0x200/*!< IDENTIFY ORGINAL OWNER */
}SubOper;

/*! \enum MiscType
    \brief Enum containing the misc type for various requirements
*/
typedef enum 
{
   COPY_COMPLETE_OBJ=0, /*!< Copy complete objlist objlist */ 
   COPY_SINGLE_OBJ=1, /*!< Copy single objlist  */ 
   EMPTY_OBJLIST, /*!< Empty objlist */
   FREE_OBJLIST  /*!< Free objlist */
}MiscType;

/*! \enum ACSATTR
    \brief Enum containing the attribute list for ACS
*/
typedef enum 
{
   ATTR_NOTSAVETOFLASH = 0x400,  /*!< Save To Flash (true if the not attribute is not present and false if the not attribute is present) */
   ATTR_INFORMTOACS = 0x800, /*!< Inform to ACS */
   ATTR_ACTIVENOTI = 0x1000, /*!< Active Notification */
   ATTR_PASSIVENOTI = 0x2000, /*!< Passive Notificaton */ 
   ATTR_NOTIDISABLED = 0x4000, /*!< Disabled Notification */ 
   ATTR_CHANGEFLAG  = 0x8000, /*!< Change flag */
   ATTR_CANDENY = 0x20000000  /*!< CanDeny flag */
}AcsAttr;

/*! \enum Owner
    \brief Enum containing the Owner Type whose try to set/get for ex:web.tr69 etc
*/
typedef enum
{
   OWN_TR69 = 0x1,/*!< DeVM */
   OWN_WEB = 0x2,/*!< WEB */
   OWN_SERVD = 0x4,/*!< SERVD */
   OWN_CLI = 0x8,/*!< CLI  */
   OWN_POLLD = 0x10,/*!< POLLD  */
   OWN_OTHER = 0x20/*!< OTHER */
}Owner;


/* @} */

#endif // #ifndef _HELP_ENUMS_H
