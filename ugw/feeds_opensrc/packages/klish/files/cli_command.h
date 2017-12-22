/*******************************************************************************
 
        Copyright (c) 2015
        Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
        For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : cli_command.h                                         *
 *         Description  : This file declares various MACROs, structures, 
 *							global variables & functions required for CLI 		*
 *							Framework back-end                                	*
 *                                                                              *
 *  *****************************************************************************/
/*!	\file cli_command.h
	\brief This file declares various MACROs, structures, global variables &
	functions required for CLI Framework back-end
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include "./clish/shell/private.h"
#include "./clish/pargv/private.h"
#include "./clish/private.h"
#include "./clish/shell.h"
#include "./clish/param/private.h"
#include "./clish/command/private.h"
#include "./clish/view/private.h"
#include "cal.h"
#include "ulogging.h"

/************************************************/
/*              LOGGING_FRAMEWORK				*/
/************************************************/
#ifndef LOG_LEVEL
uint16_t LOGLEVEL = SYS_LOG_DEBUG + 1;
#else
uint16_t LOGLEVEL = LOG_LEVEL + 1;
#endif

#ifndef LOG_TYPE
uint16_t LOGTYPE = SYS_LOG_TYPE_FILE;
#else
uint16_t LOGTYPE = LOG_TYPE;
#endif

/***********************************************/
/*              MACRO Definitions				*/
/***********************************************/

/*!
	\brief
*/
#define MAX_LEN_COMMAND			128	/*!< macro specifying the maximum length of a CLI Command */
/*!
	\brief
*/
#define MAX_DEPTH_COMMAND		16	/*!< macro specifying the maximum depth/(number of fields) of a Command, eg: "show wan interface all" has a depth of 4, as it has 4 fields */
/*!
	\brief
*/
#define MAX_LEN_ARG_LIST 		512	/*!< macro specifying the maximum length of all Command Line Arguments */
/*!
	\brief
*/
#define MAX_LEN_TR181OBJ_PATH	128	/*!< macro specifying the maximum length of a TR181 Path for an object/a parameter */
/*!
	\brief
*/
#define MAX_LEN_SERVICE_NAME	16	/*!< macro specifying the maximum length of a Service/Subsystem Name */

/*!
	\brief
*/
#define OPERATION_TYPE_OFFSET	0	/*!< macro specifying the offset of Operation type(show/set/add/del) in a command */
/*!
	\brief
*/
#define SERVICE_NAME_OFFSET		1	/*!< macro specifying the offset of Service Name(lan/wan/..) in a show command */
/*!
	\brief
*/
#define DISPLAY_TAG_OFFSET		2	/*!< macro specifying the offset of first tag to be used in Display output of a Show command */

/*!
	\brief
*/
#define MAX_LEN_COMMAND_TYPE	4	/*!< macro specifying the string length of operation type i.e. strlen(show), strlen(set/add/del) */

/*!
	\brief
*/
#define DISPLAY_MODE_LIST			1	/*!< Display output of Show command in List format */
/*!
	\brief
*/
#define DISPLAY_MODE_TABLE			2	/*!< Display output of Show command in Table format */
/*!
	\brief
*/
#define DISPLAY_MODE_JSON			3	/*!< Display output of Show command in JSON format */

/*!
	\brief
*/
#define DEBUG_OFF					0	/*!< Debug Mode is Disabled */
/*!
	\brief
*/
#define DEBUG_ON_ERROR_LEVEL		1	/*!< Debug Mode is Ebabled to show Error level logs only */
/*!
	\brief
*/
#define DEBUG_ON_WARNING_LEVEL		2	/*!< Debug Mode is Ebabled to show Error,Warning level logs only */
/*!
	\brief
*/
#define DEBUG_ON_INFO_LEVEL			3	/*!< Debug Mode is Ebabled to show Error,Warning,Information level logs only */
/*!
	\brief
*/
#define DEBUG_ON_DEBUG_LEVEL		4	/*!< Debug Mode is Ebabled to show Error,Warning,Information & Debug level logs */

/*!
	\brief
*/
#define CHAR_DIRECTION				'>'	/*!< Macro for '>' (greater than) character */
/*!
	\brief
*/
#define CHAR_COMMA					','	/*!< Macro for ',' (Comma) character */
/*!
	\brief
*/
#define CHAR_ASTERISK				'*'	/*!< Macro for '*' (Asterisk) character */
/*!
	\brief
*/
#define CHAR_COLON					':'	/*!< Macro for ':' (Colon) character */
/*!
	\brief
*/
#define CHAR_DOT					'.'	/*!< Macro for '.' (full-stop) character */
/*!
	\brief
*/
#define CHAR_QUOTE					'\''	/*!< Macro for '\'' (SingleQuote) character */
/*!
	\brief
*/
#define CHAR_SPACE					' '	/*!< Macro for ' ' (space) character */
/*!
	\brief
*/
#define CHAR_OPEN_BRACE				'{'	/*!< Macro for '{' (Opening Brace) character */
/*!
	\brief
*/
#define CHAR_CLOSE_BRACE			'}'	/*!< Macro for '}' (Closing Brace) character */
/*!
	\brief
*/
#define CHAR_TAB					'\t'	/*!< Macro for '\t' (Tab) character */
/*!
	\brief
*/
#define CHAR_BACKSLASH				'\\'	/*!< Macro for '\' (Backslash) character */
/*!
	\brief
*/
#define CHAR_ENTER					'\n'	/*!< Macro for '\n' (Enter/NewLine) character */
/*!
	\brief
*/
#define CHAR_HASH					'#'	/*!< Macro for '#' (Hash/Sharp) character */

/*!
	\brief
*/
#define IF_ERR_RETURNED_GOTO_END(ret, fmt, ...) if(ret <= UGW_FAILURE) { LOGF_LOG_ERROR(fmt, ##__VA_ARGS__ ); goto end;}
/*!
	\brief
*/
#define PRINT_TO_CONSOLE	printf

/*!
	\brief
*/
#define MAP_FILE_DIRECTORY	"/opt/lantiq/cli/map-files/"	/*!< Macro specifying the Path for Parameter Map files */
/*!
	\brief
*/
#define MAP_FILE_SUFFIX		"_param.map"	/*!< Macro specifying the Parameter Map filename's suffix */
/*!
	\brief
*/
#define MAX_LEN_FILE_NAME	64	/*!< Macro specifying the maximum length of Parameter Map file name */
/*!
	\brief
*/
#define ALIAS_PARAM_NAME	"Alias"	/*!< Macro for "Alias" Parameter which is set during treatment of "ADD" requests */
/*!
	\brief
*/
#define DEFAULT_ALIAS_SUFFIX	1	/*!< Macro for Default Alias Suffix, i.e. the number used at the end of the Alias parameter value */

/*!
	\brief
*/
#define STR_DISPLAY_COMMAND	"display"	/*!< Macro for "display" string */
/*!
	\brief
*/
#define STR_DEBUG_COMMAND	"debug"	/*!< Macro for "debug" string */
/*!
	\brief
*/
#define STR_TRANSACTION_MODE_COMMAND	"transactionmode"	/*!< Macro for "transactionmode" string */
/*!
	\brief
*/
#define MAX_ERROR_CODES		48	/*!< Macro for specifying maximum number of Error Codes */
/*!
	\brief
*/
#define MIN_LEN_MAP_FILE_ENTRY		16	/*!< Macro for specifying minimum number of octets in Map file entry eg: {"Device.IP.","ip"} */

/************************************************/
/*   Structures & Enumerations Definitions		*/
/************************************************/
/*! \brief In the below declarations, many variables have PreAsterisk and PostAsterisk, ObjectName etc.
	For a new programmer to this code, it would be confusing. I tried explain with below example.
	"Device.IP.Interface.*.IPv4Address.*.IPAddress"
	When the above string is split, 
	ParameterName = "IPAddress"
	ObjectName = "Device.IP.Interface.*.IPv4Address.*."
	PreAsterisk = "Device.IP.Interface."
	PostAsterisk = ".IPv4Address.*."
*/

/*!	
	\brief Structure to make a list of nodes, which contains Parameter Map info i.e. user_define_name and corresponding TR181-Path. 
	All parameters of a service would be a list.
*/
typedef struct ParamMapList_s {
	char sUserDefParamName[MAX_LEN_PARAM_NAME];	/*!< String to hold User defined parameter name */
	char sTR181ObjPath[MAX_LEN_TR181OBJ_PATH];	/*!< String to hold a TR181-Path name */
	struct ParamMapList_s *pxParamNext;	/*!< Pointer to next parameter */
} ParamMapList_t;

/*!	
	\brief Structure to make a list of nodes, which contains Service Info i.e. servicename & Head of Parameter of the service. 
	The services form a horizontal list and parameters in each Service node forms a vertical list.
*/
typedef struct ServiceParamMapList_s {
	char sServiceName[MAX_LEN_OBJNAME];	/*!< String to hold Service Name */
	ParamMapList_t *pxParamHead;	/*!< HeadNode of Parameters list of this service */
	struct ServiceParamMapList_s *pxServiceNext;	/*!< Pointer to next Service */
} ServiceParamMapList_t;

/*!	
	\brief Structure to make a list of nodes, which contains the linking and linked parameter's/object's information.
*/
typedef struct LinkObject_s {
	char sLinkingObj[MAX_LEN_OBJNAME];	/*!< String to hold linking Object Name */
	char sLinkingObjPreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold linking Object PreAsterisk */
	char sLinkingObjPostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold linking Object PostAsterisk */
	char sLinkingObjParam[MAX_LEN_PARAM_NAME];	/*!< String to hold linking Parameter */
	char sLinkedObj[MAX_LEN_OBJNAME];	/*!< String to hold linked Object Name */
	char sLinkedObjPreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold linked Object PreAsterisk */
	char sLinkedObjPostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold linked Object PostAsterisk */
	char sLinkedObjParam[MAX_LEN_PARAM_NAME];	/*!< String to hold linked Parameter */
	struct LinkObject_s *pxLinkNext;	/*!< Pointer to next node of "links" list; useful for forward navigation */
	struct LinkObject_s *pxLinkPrev;	/*!< Pointer to previous node of "links" list; useful for backward navigation */

} LinkObject_t;

/*!	
	\brief Structure to make a list of nodes, which contains the name of object being created and it's corresponding details.
*/
typedef struct DepObject_s {
	char sObjName[MAX_LEN_OBJNAME];	/*!< String to hold Name of Object being created/deleted */
	char sObjNamePreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PreAsterisk of Object being created/deleted */
	char sObjNamePostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PostAsterisk of Object being created/deleted */
	char sAliasPrefix[MAX_LEN_PARAM_NAME];	/*!< String to hold Alias Prefix of Object being created/deleted */
	int nAliasNumber;	/*!< String to hold Number used in Alias for distinguition of same type of objects */
	struct DepObject_s *pxDepNext;	/*!< Pointer to next node of "depends" list */
	struct DepObject_s *pxDepParent;	/*!< Pointer to Parent node in "depends" list; Useful in Hierarchical object creation */
	bool bParentUnderConstr;	/*!< Boolean to indicate that parent is also being created at the same time;  Useful in Hierarchical object creation */
} DepObject_t;

/*!	
	\brief Structure to make a list of nodes, which contains details of Input (Command Line Arguments) & Output (Data to display)
*/
typedef struct ArgParameter_s {
	bool bFilled;		/*!< Boolean to indicate that this node is useful. Mainly used in statically declared ArgParameter_t variables */
	char sArgName[MAX_LEN_PARAM_NAME];	/*!< String to hold User defined Parameter Name as we see in XMLs/Command line */
	char sArgValue[MAX_LEN_PARAM_VALUE];	/*!< String to hold value of User given values as we see in Command line */
	char sObjName[MAX_LEN_OBJNAME];	/*!< String to hold ObjectName of Parameter */
	char sPreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PreAsterisk of Parameter */
	char sPostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PostAsterisk of Parameter */
	char sParameter[MAX_LEN_PARAM_NAME];	/*!< String to hold ParameterName of Parameter */
	struct ArgParameter_s *pxArgNext;	/*!< Pointer to next node in Parameter list */
	struct ArgParameter_s *pxArgNextRow;	/*!< Pointer to next row node in Parameter list; useful when we organize the parameters to be displayed as two dimensions  */
} ArgParameter_t;

/*!	
	\brief Structure to make a list of nodes, which contains details names of parameter to be displayed.
	The list contains just the names of parameter to be displayed, as received from XML. In Show command output, 
	this list will be used as reference to extract data of various objects. Subsequently the data is 
	stored in  rows & columns fashion using the ArgParameter_t structure. 
*/
typedef struct DisplayParameter_s {
	/* bool bFilled; */
	char sObjName[MAX_LEN_OBJNAME];	/*!< String to hold ObjectName of Display Parameter */
	char sPreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PreAsterisk of Display Parameter */
	char sPostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PostAsterisk of Display Parameter */
	char sParamName[MAX_LEN_PARAM_NAME];	/*!< String to hold ParameterName of Display Parameter */
	struct DisplayParameter_s *pxDisplayNext;	/*!< Pointer to next node in Display Parameter list  */
} DisplayParameter_t;

/*!	
	\brief Structure to make a list of nodes, which contains default/hidden values to be given to certain parameters.
	When hidden parameter was planned, it was given provision to assign a fixed value or value read from some DB Object.
	Hence one would see "left" and "right" in the below variables. When we assign fixed value we given in single quotes. eg: xx_enable:'true'
	In such a case, right side object name, PreAsterisk & PostAsterisk are not used. Currently the usage is only in fixed-value cases
*/
typedef struct HiddenParam_s {
	char sLeftObjName[MAX_LEN_OBJNAME];	/*!< String to hold ObjectName of Left Parameter */
	char sLeftPreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PreAsterisk of Left Parameter */
	char sLeftPostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PostAsterisk of Left Parameter */
	char sLeftParamName[MAX_LEN_PARAM_NAME];	/*!< String to hold ParamName of Left Parameter */
	char sRightObjName[MAX_LEN_OBJNAME];	/*!< String to hold ObjectName of right Parameter */
	char sRightPreAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PreAsterisk of right Parameter */
	char sRightPostAsterisk[MAX_LEN_OBJNAME];	/*!< String to hold PostAsterisk of right Parameter */
	char sRightParamName[MAX_LEN_PARAM_NAME];	/*!< String to hold ParamName of right Parameter */
	bool bFixedValue;	/*!< boolean to indicate fixed-value assignment */
	struct HiddenParam_s *pxHiddenNext;	/*!< Pointer to next node in Hidden Parameter list  */
} HiddenParam_t;

/*!	
	\brief Structure contains heads of various linked lists and data related to the command
*/
typedef struct InputData_s {
	char sCommandName[MAX_LEN_COMMAND];	/*!< String to hold Command string */
	char sCmdLineArgs[MAX_LEN_ARG_LIST];	/*!< String to hold Arguments given from Command line */
	LinkObject_t *pxLinkObjList;	/*!< Pointer to the Head of "links" list  */
	LinkObject_t *pxLinkObjListTail;	/*!< Pointer to the Tail of "links" list  */
	DepObject_t *pxDependsList;	/*!< Pointer to the Head of "Depends" list  */
	ArgParameter_t *pxArgList;	/*!< Pointer to the Head of "Arguments" list  */
	ArgParameter_t xFilterParam;	/*!< Variable to hold Filtering Parameter */
	DisplayParameter_t *pxDisplayList;	/*!< Pointer to the Head of "Display" list  */
	ServiceParamMapList_t *pxCurrentService;	/*!< Pointer to the Service related to current command */
	HiddenParam_t *pxHiddenParamList;	/*!< Pointer to the Head of "Hidden" list  */
	clish_pargv_t *pArgv;	/*!< Pointer to the list of Clish internal Arguments list */
	char sDisplayTag[MAX_LEN_COMMAND];	/*!< String to hold Display tag used in Show commands */
} InputData_t;

/*!	
	\brief Structure defines a node that contains CommandName & various arguments as given by user; It is used to 
	display list of commands in Explicit transaction mode
*/
typedef struct Command_s {
	char sCommandWithArgs[MAX_LEN_COMMAND + MAX_LEN_ARG_LIST];	/*!< String to hold CommandName & Command line arguments */
	struct Command_s *pxCmdNext;	/*!< Pointer to next command */
} Command_t;

/*!	
	\brief Structure to used to make a list of nodes, which contains a separate MsgHeader instance for every Object type.
*/
typedef struct GetMsgList_s {
	char sObjName[MAX_LEN_OBJNAME];	/*!< String to hold ObjName for which we are going to make CAL-GET */
	MsgHeader xGetObjMsgHead;	/*!< Variable to hold Object List of CAL-GET request/result */
	int nCalGetStatus;	/*!< Variable to hold return value of CAL-GET request */
	struct GetMsgList_s *pxGetMsgNext;	/*!< Pointer to next ObjectType for which CAL-GET will be made */
} GetMsgsList_t;

/*!	
	\brief Structure defines array elements which contain error codes and theur default error strings.
*/
typedef struct ErrorResponseCode_s {
	char sDefaultErrorString[MAX_LEN_PARAM_VALUE];	/*!< String to hold Default String corresponding to Error response code */
	int nErrorCode;		/*!< Integer variable to hold Error response code */
} ErrorResponseCode_t;

/************************************************/
/*       Global Variable Declarations			*/
/************************************************/
ServiceParamMapList_t *pxServiceList = NULL;	/*This needs to be Global as it is freed in a different place; TODO to free it in the context of cli_command.c */
MsgHeader gxSetObjMsgHead;	/*This needs to be Global as it spans across many commands */
bool gbTransactionMode = false;	/*This needs to be Global as it has to be checked for Transaction mode across many commands */
Command_t *gpxCmdListToCommit = NULL;	/*This needs to be Global as it contains list of commands */
int gnDebugMode = DEBUG_OFF;	/*This needs to be Global as it spans across many commands */
/*int gnDebugMode = DEBUG_ON_DEBUG_LEVEL; */
int gnDisplayMode = DISPLAY_MODE_LIST;	/*This needs to be Global as it spans across many commands */

/*!	\brief This array defines default error strings corresponding to each error code.
	In case if no error string comes from Subsystems, then this array is read to give a mimimum clue to user 
*/
ErrorResponseCode_t ErrorCodeArray[MAX_ERROR_CODES] = {
	{"ERR_INVALID_OBJNAME_REQUEST", -200},	/*!< Invalid object name request */
	{"ERR_INVALID_PARAMETER_REQUEST", -201},	/*!< Invalid parameter name request */
	{"ERR_INVALID_INSTANCE_REQUEST", -202},	/*!< Invalid instance request */
	{"ERR_INVALID_XML_FILE", -203},	/*!< Invalid xml file */
	{"ERR_FILE_NOT_FOUND", -204},	/*!< File not found */
	{"ERR_FILE_LOCK_FAILED", -205},	/*!< File lock failed */
	{"ERR_FILE_WRITE_FAILED", -206},	/*!< File write failed */
	{"ERR_FILE_TRUNCATE_FAILED", -207},	/*!< File truncate failed */
	{"ERR_NO_CONTENT", -208},	/*!< No content in recived msg */
	{"ERR_INVALID_UBUS_ARG", -209},	/*!< Invalid ubus arguments */
	{"ERR_UBUSMSG_OVERLOAD", -210},	/*!< Ubus msg overloaded */
	{"ERR_UBUSD_NOT_RUNNING", -211},	/*!< Ubusd daemon not running */
	{"ERR_MEMORY_ALLOC_FAILED", -212},	/*!< Memory Allocation failed */
	{"ERR_UPDATE_FAILED", -213},	/*!< Update failed it can be anything like(tree update}, servd internal update}, csd internal update}, etc) */
	{"ERR_MAX_INSTANCE_EXCEEDED", -214},	/*!< Maximum instance count reached */
	{"ERR_MIN_INSTANCE_REACHED", -215},	/*!< Minimum instance count reached */
	{"ERR_MERGE_FAILED", -216},	/*!< Merge failed */
	{"ERR_VALIDATION_FAILED", -217},	/*!< validataion failed */
	{"ERR_NON_INSTANCEABLE", -218},	/*!< Add failed */
	{"ERR_DEFAULT_LOAD_FAILURE", -219},	/*!< Default value get failed */
	{"ERR_PATH_NOT_FOUND", -220},	/*!< File directory path not found */
	{"ERR_RECEIVER_NOT_RUNNING", -221},	/*!< Requested server not running */
	{"ERR_ADD_OBJECT_FAILED", -222},	/*!< ADD object failed */
	{"ERR_DEL_OBJECT_FAILED", -223},	/*!< DEL object failed */
	{"ERR_MIN_VAL_REACHED", -224},	/*!< MIN VAL Reached */
	{"ERR_MAX_VAL_EXCEEDED", -225},	/*!< MAX VAL Exceeded */
	{"ERR_MIN_LEN_REACHED", -226},	/*!< MIN LEN Reached */
	{"ERR_MAX_LEN_EXCEEDED", -227},	/*!< MAX LEN Exceeded */
	{"ERR_BOOLEAN_FAILED", -228},	/*!< Boolean failed */
	{"ERR_DATETIME_FAILED", -229},	/*!< Date Time failed */
	{"ERR_READ_ONLY", -230},	/*!< Read only parameter */
	{"ERR_GET_ID_FAILED", -231},	/*!< Get Id failed */
	{"ERR_STRING_NOT_FOUND", -232},	/*!< String match failed */
	{"ERR_BAD_FD", -233},	/*!< Bad File descriptor */
	{"ERR_IOCTL_FAILED", -234},	/*!< IOCTL failed */
	{"ERR_INVALID_PARAMETER_VALUE_REQUEST", -235},	/*!< Invalid parameter value request */
	{"ERR_ALIAS_REPLACEMENT_FAILED", -236},	/*!< Invalid parameter value request */
	{"ERR_RULE_MODIFY_NOT_ALLOWED", -240},	/*!< Modification of Rule is not allowed */
	{"ERR_INVALID_SL", -241},	/*!< TODO dummy need to replace actual error code */
	{"ERR_INVALID_SET_PARAM_REQ", -242},	/*!< Invalid SET Parameter request eg: SET of ReadOnly Parameter */
	{"ERR_INVALID_OPERATION", -243},	/*!< Invalid Options */
	{"ERR_INVALID_IPADDRESS", -244},	/*!< Wrong format of IPAddress */
	{"ERR_INVALID_IP_POOL_RANGE", -245},	/*!< IP Address out of allowed range */
	{"ERR_INTERNAL", -246},	/*!< Internal Error */
	{"ERR_EXISTING_ATM_WAN", -247},	/*!< Existing ATM WAN connections */
	{"ERR_MULTIPLE_IPOA_PPPOA_WAN", -248},	/*!< Multiple IPOA/EOA WAN */
	{"ERR_MULTIPLE_BRIDGED_WAN", -249},	/*!< Multiple Beidged WAN */
	{"ERR_IPOA_PPPOA_WAN_ON_VLAN", -250}	/*!< IPoA/PPPoA WAN on VLAN */
};

/************************************************/
/*            Function Declarations				*/
/************************************************/

/*!	\brief This function parses various data coming from XML and updates various Linked lists in  xInData
	\param[in] clish_context: pointer to KLISH's internal context	
	\param[out] pcServiceName: pointer where the service name would be stored
	\param[out] peMainOp: pointer where the main operation type would be stored
	\param[out] pxInData: pointer where the sub operation type would be stored
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_parseDataFromXML(clish_context_t * clish_context, char *pcServiceName, MainOper * peMainOp, ObjOper * peSubOper, InputData_t * pxInData);

/*!	\brief This function parses command line arguments attribute received from XML. For all parameters to 
				be set, the arguments taken in name-value pairs. The parameter to be used as Filter would 
				come directly. The filtering is done based on first parameter.
	\param[in] pArgv: pointer to KLISH's internal structures of command line arguments
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_parseCmdLineArguments(clish_pargv_t * pArgv, InputData_t * pxInData);

/*!	\brief This function Parses "links" attribute coming from XML. The purpose of this attribute is :
				To convey the relation between various objects.
	\param[in] pcLink: pointer to "links" attribute string
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_parseLinks(char *pcLink, InputData_t * pxInData);

/*!	\brief Parses "display" attribute received from XML. To clarify a new user, "display" attribute versus
			"dislay mode": "display" parameter comes from XML to convey what are all the parameters to be 
			displayed in the o/p, whereas the "display mode" is in what fashion(list, json, table) the output 
			has to be displayed.
	\param[in] pcDisplay: pointer to "display" attribute string
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_parseDisplay(char *pcDisplay, InputData_t * pxInData);

/*!	\brief This function parses "depends" attribute received from XML. This attribute's purpose is:
				to convey the list of objects to be created/deleted.
	\param[in] pcDepends: pointer to "depends" attribute string
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_parseDepends(char *pcDepends, InputData_t * pxInData);

/*!	\brief This function parses "hidden" attribute received from XML. This attribute's purpose is:
				To parse default values to few of the parameters needed during configuration
	\param[in] pcHidden: pointer to "hidden" attribute string
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_parseHidden(char *pcHidden, InputData_t * pxInData);

/*!	\brief This function frees various dynamically allocated structures created while parsing XML datas
	\param[out] pxInData: pointer to Input Data's Structure
	\return void
*/
static void cli_freeGlobalStructures(InputData_t * pxInData);

/*!	\brief This function creates "links" and does minimal sorting as user is expected to send in sorted way.
	\param[in] pcLinking: pointer to linking parameter string
	\param[in] pcLinked: pointer to linked object string
	\param[out] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_createAndSortLinkObjs(InputData_t * pxInData, char *pcLinking, char *pcLinked);

/*!	\brief This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on "links" attribute.
	\param[in] pxInData: pointer to Input Data's Structure
	\param[in] bShowMode: boolean to indicates if this function is called in "show" context
	\param[out] pxGetObjList: pointer to Get ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateGetObjListBasedOnLinkObjs(ObjList * pObjType, InputData_t * pxInData, bool bShowMode);

/*!	\brief This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on "depends" attribute.
	\param[in] pxFilterArg: pointer to Filter Parameter structure
	\param[out] pxDepObj: pointer to Depends list	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateGetObjListBasedOnDepObjs(ObjList * pObjType, DepObject_t * pxDepObj);

/*!	\brief This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on "display" parameters.
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxGetObjList: pointer to Get ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateGetObjListBasedOnDisplay(ObjList * pxGetObjList, InputData_t * pxInData);

/*!	\brief This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on input Arguments.
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxGetObjList: pointer to Get ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateGetObjListBasedOnArgList(ObjList * pxGetObjList, InputData_t * pxInData);

/*!	\brief This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on filter parameter.
	\param[in] pxFilterArg: pointer to Filter Parameter structure
	\param[out] pxGetObjList: pointer to Get ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateGetObjListBasedOnFilterParam(ObjList * pxGetObjList, ArgParameter_t * pxFilterArg);

/*!	\brief This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on Hidden Parameters.
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxGetObjList: pointer to Get ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateGetObjListBasedOnHiddenParams(ObjList * pxGetObjList, InputData_t * pxInData);

/*!	\brief This function updates Object List for Add scenario. The list is updated based on "depends", 
			"links" & Arguments received from XML.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxSetObjList: pointer to Set ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateSetObjListForAddBasedOnDepObjs(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData);

/*!	\brief This function based on "hidden" attribute, updates Object List to be configured using CAL-SET.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxSetObjList: pointer to Set ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateSetObjListBasedOnHiddenParam(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData);

/*!	\brief This function prepares list of Objects to be deleted.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxSetObjList: pointer to Set ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateSetObjListForDelBasedOnDepObjs(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData);

/*!	\brief This function updates the ObjectList that has to be configured in Database using CAL-SET. Here 
				the update happens based on input Arguments.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[in] bFilterApplied: boolean that indicates if a specific object's filter has been applied
	\param[out] pxSetObjList: pointer to Set ObjList	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateSetObjListForModifyBasedOnArgs(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData, bool bFilterApplied);

/*!	\brief This function handles Generic commands (start, commit or cancel)
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_genericCommandHandler(InputData_t * pxInData);

/*!	\brief This function handles Show commands starting from CAL-GET to displaying the data to user.
	\param[in] pxInData: pointer to Input Data's Structure
	\param[in] pxCalSetObjList: pointer to ObjList returned by CAL-SET operation
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_showCommandHandler(InputData_t * pxInData, ObjList * pxCalSetObjList);

/*!	\brief This function is top-level function for various configuration commands. It further branches to
				specific handler based on sub operation type.
	\param[in] eSubOp: SubOperation type
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_configureCommandHandler(ObjOper eSubOp, InputData_t * pxInData);

/*!	\brief This function handles ADD commands
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxGetMsgHead: pointer to GetMsgHead which contains GetObjList
	\param[out] pxSetMsgHead: pointer to SetMsgHead which contains SetObjList
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_addCommandHandler(MsgHeader * pxGetMsgHead, MsgHeader * pxSetMsgHead, InputData_t * pxInData);

/*!	\brief This function handles DEL commands
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxGetMsgHead: pointer to GetMsgHead which contains GetObjList
	\param[out] pxSetMsgHead: pointer to SetMsgHead which contains SetObjList
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_deleteCommandHandler(MsgHeader * pxGetMsgHead, MsgHeader * pxSetMsgHead, InputData_t * pxInData);

/*!	\brief This function handles Modify/SET requests.
	\param[in] pxInData: pointer to Input Data's Structure
	\param[out] pxGetMsgHead: pointer to GetMsgHead which contains GetObjList
	\param[out] pxSetMsgHead: pointer to SetMsgHead which contains SetObjList
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_modifyCommandHandler(MsgHeader * pxGetMsgHead, MsgHeader * pxSetMsgHead, InputData_t * pxInData);

/*!	\brief Sets the display mode. Default mode is "list" mode.
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_setDisplayCommandHandler(InputData_t * pxInData);

/*!	\brief This function enables or disables Debug mode. Also sets the level of debug. Default level is 
				Error level.
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_setDebugCommandHandler(InputData_t * pxInData);

/*!	\brief This function displays current transaction mode and also various commands to be committed in 
				explicit trnasaction mode
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_showTransactionModeCommandHandler(void);

/*!	\brief This function returns the unused instance number, which will be used to form Alias suffix
			(number part).
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxSetObjList: pointer to Set ObjList	
	\param[in] pcObjName: pointer to Object name string for which we need to find unused instance number.	
	\return The unused instance number  of corresponding ObjectType or 1 if no such object exists in DB
*/
static int cli_getUnusedInstanceofObjname(GetMsgsList_t * pxGetMsgList, ObjList * pxSetObjList, char *pcObjName);

/*!	\brief This function returns the display mode.
	\param[in] pnMode: pointer to display mode
	\return UGW_SUCCESS / UGW_FAILURE 	
*/
static int cli_getDisplayMode(int *pnMode);

/*!	\brief This function reads data from received Objlist from CAL-GET, and prepares data to be displayed.
				It prepares the display data as rows & columns fashion, where each row corresponds to one set 
				of display parameters.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[OUT] pxOutList: pointer to the output nodes that are used to display data
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_extractDataToDisplayFromGetObjList(GetMsgsList_t * pxGetMsgList, InputData_t * pxInData, ArgParameter_t ** pxOutList);

/*!	\brief This function prints Output in specified Display mode.
	\param[in] pxOutList: pointer to the output nodes that are used to display data
	\param[in] nMode: display mode
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_printOutput(ArgParameter_t * pxOutList, int nMode, InputData_t * pxInData);

/*!	\brief This function displays parameters in tabular form. Upto 6 parameters, each of 20 char length are 
			shown cleanly.
	\param[in] pxOutList: pointer to the output nodes that are used to display data
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_printInTabularForm(ArgParameter_t * pxOutList, InputData_t * pxInData);

/*!	\brief This function displays parameters in JSON form. 
	\param[in] pxOutList: pointer to the output nodes that are used to display data
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_printInJsonForm(ArgParameter_t * pxOutList, InputData_t * pxInData);

/*!	\brief This function is used to perform specific action based on subsystem/command type. At the 
				moment this function filter unwanted entries of "show wan interface all". This function is 
				a not clean way to achieve this. The better way would be to make use of "hidden" parameter.
	\param[in] pxOutList: pointer to the output nodes that are used to display data
	\param[in] pxInData: pointer to Input Data's Structure
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_specificActionsonDisplayData(ArgParameter_t ** pxOutList, InputData_t * pxInData);

/*!	\brief This function free output structures, which are organized as rows and columns.
	\param[in] pxOutList: pointer to the output nodes that were used to display data
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_freeOutputStructures(ArgParameter_t * pxOutList);

/*!	\brief This function updates specific node(parameter) in output display list with value read from 
			corresponding Reference object. It takes care if the concerned object is in hierarchy of RefObj.
	\param[in] pxDispTrav: pointer to the display parameter node	
	\param[in] pxTopRefObj: Pointer to top level reference Object's name
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[OUT] pxOutTrav: pointer to the output node that has to be updated with data to be displayed
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateDisplayParameterNodeWithRefObj(ArgParameter_t * pxOutTrav, DisplayParameter_t * pxDispTrav, ObjList * pxTopRefObj, GetMsgsList_t * pxGetMsgList);

/*!	\brief This function searches the received Object from CALGET to find a linked object instance. 
				It traverses in the lists in backward direction (bottom-to-top direction) starting from 
				Initial Reference(filter criterion) Object based on "links".
	\param[in] pcInitRefObj: Specific Initial reference object	
	\param[in] pcExpectObj: Pointer to expected Object's name
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[OUT] pxRetObj: ObjectList to capture the retrieved objects
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_getBackwardLinkedObj(char *pcInitRefObj, char *pcExpectObj, GetMsgsList_t * pxGetMsgList, ObjList ** pxRetObj, InputData_t * pxInData);

/*!	\brief This function searches the received Object from CALGET to find a linked object instance. 
				It traverses in the lists in forward direction (top-to-bottom direction) starting from 
				Initial Reference(filter criterion) Object based on "links".
	\param[in] pcInitRefObj: Specific Initial reference object	
	\param[in] pcExpectObj: Pointer to expected Object's name
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pxInData: pointer to Input Data's Structure
	\param[OUT] pxRetObj: ObjectList to capture the retrieved objects
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_getForwardLinkedObj(char *pcInitRefObj, char *pcExpectObj, GetMsgsList_t * pxGetMsgList, ObjList ** pxRetObj, InputData_t * pxInData);

/*!	\brief This function searches received object from CALGET & extracts the object Instance based 
			on the filter.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[in] pcPreAsterisk: Pointer to PreAsterisk string
	\param[in] pcObjName: Pointer to Object name string
	\param[out] pcInitRefObj: Specific Initial reference object	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_getInitRefObjectBasedOnFilter(char *pcInitRefObj, GetMsgsList_t * pxGetMsgList, char *pcPreAsterisk, char *pcObjName);

/*!	\brief This function sets the ObjList for CAL-SET based on a specific Argument.
	\param[in] pxArgTrav: pointer to structure of Argument that has to be set 
	\param[in] pcRefObjName: Specific Object name that has to be set using CALSET
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\param[out] pxSetObjList: ObjectList that is set for CAL SET operations
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_updateSetObjListBasedOnSpecificArg(ObjList * pxSetObjList, ArgParameter_t * pxArgTrav, char *pcRefObjName, GetMsgsList_t * pxGetMsgList);

/*!	\brief This function checks if an Object of given name is already in the specific ObjectList
	\param[in] pxObjList: ObjectList containing various objects
	\param[in] pcObjName: Specific Object name that has to checked in the object list
	\param[out] pxObjOut: ObjectList pointer will be returned in this if specific object is found in the list	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_checkObjDuplicationInObjList(ObjList * pxObjList, char *pcObjName, ObjList ** pxObjOut);

/*!	\brief This function splits a TR181 string to Parameter part(ParamName), Object Name (ObjExtract),
			top level instantiated object (PreAsterisk), Child object Name(PostAsterisk)
	\param[in] pcTR181fullpath: TR181 Parameter string with full path
	\param[out] pcParamName: Parameter name of TR181 string
	\param[out] pcObjExtract: Object name of TR181 string
	\param[out] pcObjNamePreAsterisk: PreAsterisk Object name of TR181 string
	\param[out] pcObjNamePostAsterisk: PostAsterisk Object name of TR181 string
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_splitTR181String(char *pcTR181fullpath, char *pcParamName, char *pcObjExtract, char *pcObjNamePreAsterisk, char *pcObjNamePostAsterisk);

/*!	\brief This function gets the TR181-Param name based on user defined parameter name. Just opposite to 
				cli_getUserDefinedParamName()
	\param[in] pxServParamMap: Pointer to ServiceParamList corresponding to this service
	\param[in] pcCmd: user defined parameter string	
	\param[out] pcTR181param: TR181 Parameter string with full path
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_getTR181MappingParam(ServiceParamMapList_t * pxServParamMap, char *pcCmd, char *pcTR181param);

/*!	\brief This function gets the user defined parameter name based on TR181-Param name. Just opposite to 
				cli_getTR181MappingParam()
	\param[in] pxServParamMap: Pointer to ServiceParamList corresponding to this service
	\param[in] pcTR181param: TR181 Parameter string with full path
	\param[out] pcUserparam: user defined parameter string	
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_getUserDefinedParamName(ServiceParamMapList_t * pxServParamMap, char *pcTR181param, char *pcUserparam);

/*!	\brief Reads command and determines if it is a Show command or Configure command. If Configure command,
			it further determines if it as add/delete/modify operation. Also if it is Show command, it reads
			service name as well.
	\param[in] pcCmd: Command string 
	\param[in] pcService: Service name 
	\param[out] peMainOp: pointer that points to Main operation value
	\param[out] peSubOper: pointer that points to sub operation value
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_analyzeCommand(char *pcCmd, MainOper * peMainOp, ObjOper * peSubOper, char *pcService);

/*!	\brief Based on Services name, it reads parameter map file and updates Service Parameter map
	\param[in] pcServName: Service name 
	\param[out] pxServList: Pointer to Head of ServiceParamMapList list
	\return Pointer to ServiceParamList corresponding to this service
*/
static ServiceParamMapList_t *cli_populateServiceParameters(ServiceParamMapList_t ** pxServList, char *pcServName);

/*!	\brief Matches 2 Object Names, ignoring the CHAR_ASTERISK symbol in place of instance number.
	\param[in] pcAsteriskStr: Strings with Asterisk(s) 
	\param[in] pcInstancedStr: Strings with instance number(s)
	\return true/1 if strings matches, else false/0
*/
static bool cli_matchInstancedStrings(char *pcAsteriskStr, char *pcInstancedStr);

/*!	\brief This function searches an ObjectName within the CAL-GET received lists and return the  
			corresponding ObjList pointer.
	\param[in] pcObjName: Specific Object's name 
	\param[in] pcObjPreAsterisk: Specific Object's pre-asterisk string
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList 
	\return Pointer to ObjList corresponding to pcObjName
*/
static ObjList *cli_getSpecificObjectPointer(char *pcObjName, char *pcObjPreAsterisk, GetMsgsList_t * pxGetMsgList);

/*!	\brief This function prepares a separate MshHeaderfor for each Object type that has to be 
				fetched using CALGET. Otherwise the failure to obtain Objlist for one Object type, would 
				eventually cause failure for other Object types, though their Object list could be fetched 
				successfully when tried separately.
	\param[in] pxGetObjList: ObjList containing all the objects that has to be fetched from DB
	\param[in] pxInData: pointer to Input Data's Structure
	\param[in] bPostCalSet: Boolean to indicate that this function is called after CAL_SET
	\param[out] pxGetMsgList: pointer to pointer of Concatenated/forked Get ObjList
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_forkGetObjListForEachObject(ObjList * pxGetObjList, int nGetMode, InputData_t * pxInData, GetMsgsList_t ** pxGetMsgList, bool bPostCalSet);

/*!	\brief This function calls CALGET in a loop for various objects.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_executeConcatenatedCalGet(GetMsgsList_t * pxGetMsgList);

/*!	\brief This function frees various Object lists obtained in CALGET.
	\param[in] pxGetMsgList: pointer to Concatenated Get ObjList 
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_freeConcatenatedGetObjList(GetMsgsList_t * pxGetMsgList);

/*!	\brief This function appends the current command to the list of Commands awaiting "Commit" in 
			explicit transaction mode.
	\param[in] pxInData: pointer to Input Data's Strcuture 
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_appendCommandToCommitAwaitingList(InputData_t * pxInData);

/*!	\brief This function frees the list of Commands awaiting "Commit" in explicit transaction mode.
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_freeCommitAwaitingList(void);

/*!	\brief This function ignores spaces/tabs/slashes/CRs in Strings and returns a resultant string
	\param[in] pcInput: Character pointer pointing to the string containing attribute parsed from XML
	\param[out] pcResult: pointer to character pointer, which holds resultant attribute
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_ignoreSpacesInStrings(char **pcResult, char *pcInput);

/*!	\brief This function handles error response codes and displays error information to user
	\param[in] pxObjList: The ObjList returned by CAL_SET operations
	\return UGW_SUCCESS / UGW_FAILURE 
*/
static int cli_handleErrorResponseCode(ObjList * pxObjList);
