/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_error.h                                          * 
 *         Description  : Common Library contains functions, defines,		*
 *			  structs, enums used across modules like CAL, CAPI,	*
 *			  CSD, Servd, and Management Entities			*
 *  *****************************************************************************/

/*! \file help_error.h
\brief File contains the error code returned with respect to Framework, SL, and ME
*/
/* @{ */


/*! CAUTION: While adding the new error code
	The comment section for that error code is must, else that error message will not be handle by web. 
	String within the comment section in the same line of error code enum will be displayed as error message on the web.
	Format of the comment section should follow doxygen format comment as below.  
*/


#ifndef _HELP_ERROR_H
#define _HELP_ERROR_H

#define UGW_SUCCESS 0		/*!< Macro to define success status */
#define UGW_FAILURE -1		/*!< Macro to define failure status */

/*! \enum FrameWorkErrCode
    \brief Enum containing the type of error code which can be returned from the framework 
     modules like servd, csd, and cal. Buffer error code -200 to -240
*/
typedef enum {
	ERR_INVALID_OBJNAME_REQUEST = -200,	/*!< Error Requested Object is Invalid */
	ERR_INVALID_PARAMETER_REQUEST = -201,	/*!< Error Requested Parameter is Invalid */
	ERR_INVALID_INSTANCE_REQUEST = -202,	/*!< Error Invalid Instance Request */
	ERR_INVALID_XML_FILE = -203,	/*!< Invalid XML File request */
	ERR_FILE_NOT_FOUND = -204,	/*!< File not found, Cannot perform the operation */
	ERR_FILE_LOCK_FAILED = -205,	/*!< File lock failed */
	ERR_FILE_WRITE_FAILED = -206,	/*!< File write failed, Cannot perform the operation */
	ERR_FILE_TRUNCATE_FAILED = -207,	/*!< File truncate failed, Cannot perform the operation */
	ERR_NO_CONTENT = -208,	/*!< No content in recived msg */
	ERR_INVALID_UBUS_ARG = -209,	/*!< Invalid ubus arguments */
	ERR_UBUSMSG_OVERLOAD = -210,	/*!< Ubus msg overloaded */
	ERR_UBUSD_NOT_RUNNING = -211,	/*!< Ubusd daemon not running */
	ERR_MEMORY_ALLOC_FAILED = -212,	/*!< Memory Allocation failed */
	ERR_UPDATE_FAILED = -213,	/*!< Update failed it can be anything like (tree update, servd internal update, csd internal update, etc) */
	ERR_MAX_INSTANCE_EXCEEDED = -214,	/*!< Add operation failed, max limit reached */
	ERR_MIN_INSTANCE_REACHED = -215,	/*!< Delete operation failed, system expects this entry  */
	ERR_MERGE_FAILED = -216,	/*!< Merge Failed */
	ERR_VALIDATION_FAILED = -217,	/*!< Validataion Failed, Cannot perform the operation */
	ERR_NON_INSTANCEABLE = -218,	/*!< Add Operation Failed, due to non-instancable property of the object */
	ERR_DEFAULT_LOAD_FAILURE = -219,	/*!< Default value get failed */
	ERR_PATH_NOT_FOUND = -220,	/*!< File directory path not found */
	ERR_RECEIVER_NOT_RUNNING = -221,	/*!< Requested server not running */
	ERR_ADD_OBJECT_FAILED = -222,	/*!< ADD object failed */
	ERR_DEL_OBJECT_FAILED = -223,	/*!< DEL object failed */
	ERR_MIN_VAL_REACHED = -224,	/*!< Value is not number or less than expected value*/
	ERR_MAX_VAL_EXCEEDED = -225,	/*!< Value is not number or greater than expected value*/
	ERR_MIN_LEN_REACHED = -226,	/*!< String length is less than expected length*/
	ERR_MAX_LEN_EXCEEDED = -227,	/*!< String length is greater than expected length */
	ERR_BOOLEAN_FAILED = -228,	/*!< Boolean Failed */
	ERR_DATETIME_FAILED = -229,	/*!< Date Time Failed */
	ERR_READ_ONLY = -230,	/*!< Read only parameter, Cannot perform the operation */
	ERR_GET_ID_FAILED = -231,	/*!< Get Id Failed */
	ERR_STRING_NOT_FOUND = -232,	/*!< String not found or String match failed */
	ERR_BAD_FD = -233,	/*!< Bad File descriptor */
	ERR_IOCTL_FAILED = -234,	/*!< IOCTL failed */
	ERR_INVALID_PARAMETER_VALUE_REQUEST = -235,	/*!< Invalid parameter value request */
	ERR_ALIAS_REPLACEMENT_FAILED = -236,	/*!< Invalid parameter value request */
	ERR_UBUS_TIME_OUT = -237,	/*!< ubus timeout happend on get/set request */
	ERR_SL_TIME_OUT = -238,	/*!< SL failed to complete the operation or timed out */
	ERR_EXPECTED_NUM = -239	/*!< Strings are not allowed, Only numbers are allowed */
} FrameWorkErrCode;

/*! \enum SLErrCode
    \brief Enum containing the type of error code which can be returned from the Service Layer. 
     Buffer error code -241 to -280
*/
typedef enum {
	ERR_INVALID_SL = -241,	/*!< Required Service Library doesnot Exist !! */
	ERR_INVALID_SET_PARAM_REQ = -242, /*!< Set Parameter Request Failed */
	ERR_INVALID_OPERATION = -243, /*!< Invalid Operation Requested */
	ERR_INVALID_IPADDRESS = -244, /*!< Invalid IpAddress */
	ERR_INVALID_IP_POOL_RANGE = -245, /*!< Pool Range Specified in Invalid */
	ERR_INTERNAL = -246, /*!< Internal Error. Please correct the entered data */
	ERR_EXISTING_ATM_WAN = -247, /*!< Can't delete ATM Link, already a connection exists with this link */
	ERR_MULTIPLE_IPOA_PPPOA_WAN = -248, /*!< Creation of a New or Modification of existing IPoA/PPPoA Connection not supported. Kindly delete the existing connection and Re-create a new connection */
	ERR_MULTIPLE_BRIDGED_WAN = -249, /*!< Creation of multiple bridged WAN connection is not permitted */
	ERR_IPOA_PPPOA_WAN_ON_VLAN = -250, /*!< Creation of IPoA/PPPoA connection on a VLAN interface is not permitted */
	ERR_DB_READ_FAILED = -251, /*!< DataBase Read Failed */
	ERR_RULE_MODIFY_NOT_ALLOWED = -252, /*!< This rule cannot be modified/deleted */
	ERR_REJECT_BRIDGED_WAN_CONN = -253, /*!< WAN exists on concerned wan interface, Creation of new Bridged WAN Connection is not permitted */
	ERR_REJECT_PPPOE_WAN_CONN = -254, /*!< Bridged WAN exists on concerned wan interface, Creation of new PPP WAN Connection is not permitted */
	ERR_REJECT_STATIC_DHCP_WAN_CONN = -255, /*!< Static/DHCP/Bridged WAN exists on concerned wan interface, Creation of new Static/DHCP WAN Connection is not permitted */
	ERR_USER_ADD = -256, /*!< Failed to add user, Check system logs for proper error message */
	ERR_USER_REMOVE = -257, /*!< Failed to remove user, Check system logs for proper error message */
	ERR_BRIDGED_LANVLAN_DELETE = -258, /*!< LAN VLAN Interface cannot be deleted as it is part of bridge. Please remove it from bridge and try again */ 
	ERR_PRIMARY_BRIDGE_DELETE = -259, /*!< Deletion of Primary LAN Bridge (br-lan) is not Allowed */
	ERR_DEFAULT_STATIC_ROUTE = -260, /*!< Adding static route against 0.0.0.0 network is not allowed */
	ERR_BRIDGE_WITOUT_LANPORT = -261, /*!< Bridge (br-lan) should have atleast one lan port */
	ERR_MACADDRESS_POOL_EXHAUSTED = -262 /*!< MAC Address pool exhausted. Rejecting WAN connection */
} SLErrCode;

/*! \enum MEErrCode (Management Entity)
    \brief Enum containing the type of error code which can be returned from the ME(web, cwmp, upnp, cli, etc).
     Buffer error code -281 to -320
*/
typedef enum {
	ERR_402_NOT_FOUND = -281	/*!< ME Error */
} MEErrCode;

#endif				//#ifndef _HELP_ERROR_H

/* @} */
