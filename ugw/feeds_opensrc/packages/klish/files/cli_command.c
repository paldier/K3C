/*******************************************************************************
 
        Copyright (c) 2015
        Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
        For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : cli_command.c                                         *
 *         Description  : This file implements various functions required  for  *
 *		                  CLI Framework back-end                                *
 *                                                                              *
 *  *****************************************************************************/

/*!	\file cli_command.c
	\brief This file implements various functions required  for CLI Framework back-end
*/

#include "cli_command.h"

/***********************************************************************************************************
Function: cli_cmdHandler
Description: This is entry point for UGW CLI Back-end framework
***********************************************************************************************************/
CLISH_PLUGIN_SYM(cli_cmdHandler)
{
	int nRet = UGW_SUCCESS;
	MainOper eMainOp = MOPT_NOOPER;
	ObjOper eSubOper = OBJOPT_MODIFY;
	char sServiceName[MAX_LEN_SERVICE_NAME] = { 0 };
	InputData_t xInData;

	memset(&xInData, 0, sizeof(InputData_t));

	/* Parse various data coming from XML to the structures contained by xInData */
	nRet = cli_parseDataFromXML(clish_context, sServiceName, &eMainOp, &eSubOper, &xInData);
	IF_ERR_RETURNED_GOTO_END(nRet, "Unable to parse Data from XML; Check input \n");

	if ((xInData.pxCurrentService == NULL) && (strlen(sServiceName) > 0)) {
		LOGF_LOG_ERROR("No Parameter Map available for %s service\n", sServiceName);
		nRet = UGW_FAILURE;
		goto end;
	}

	if (eMainOp == MOPT_GET) {
		nRet = cli_showCommandHandler(&xInData, NULL);
		IF_ERR_RETURNED_GOTO_END(nRet, "Failure in Show Command\n");
	} else if (eMainOp == MOPT_SET) {
		nRet = cli_configureCommandHandler(eSubOper, &xInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Failure in Configure Command\n");
	} else {
		nRet = cli_genericCommandHandler(&xInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Failure in Generic Command\n");
	}

 end:
	if (nRet <= UGW_FAILURE) {
		PRINT_TO_CONSOLE("Failure in executing \"%s %s\" \n", xInData.sCommandName, xInData.sCmdLineArgs);
	}
	cli_freeGlobalStructures(&xInData);
	return nRet;
}

/***********************************************************************************************************
Function: cli_parseDataFromXML
Description: This function parses various data coming from XML and updates various Linked lists in  xInData
***********************************************************************************************************/
int cli_parseDataFromXML(clish_context_t * clish_context, char *pcServiceName, MainOper * peMainOp, ObjOper * peSubOper, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	clish_command_t *pCommand = (clish_command_t *) clish_context->cmd;
	clish_pargv_t *pArgv = (clish_pargv_t *) (clish_context->pargv);
	clish_view_t *pView = (clish_view_t *) pCommand->pview;
	char *pcLinks = NULL;
	char *pcDepends = NULL;
	char *pcDisplay = NULL;
	char *pcHidden = NULL;

	if (strlen(pCommand->name) >= MAX_LEN_COMMAND) {
		LOGF_LOG_ERROR("Command Name %s is longer than %d\n", pCommand->name, MAX_LEN_COMMAND);
		nRet = UGW_FAILURE;
		goto end;
	}
	memset(pxInData->sCommandName, 0, MAX_LEN_COMMAND);
	strncpy(pxInData->sCommandName, pCommand->name, MAX_LEN_COMMAND - 1);

	if (pView->service != NULL) {
		if (strlen(pView->service) >= MAX_LEN_SERVICE_NAME) {
			LOGF_LOG_ERROR("Service Name %s is longer than %d\n", pView->service, MAX_LEN_SERVICE_NAME);
			nRet = UGW_FAILURE;
			goto end;
		}
		memset(pcServiceName, 0, MAX_LEN_SERVICE_NAME);
		strncpy(pcServiceName, pView->service, MAX_LEN_SERVICE_NAME - 1);
		pxInData->pxCurrentService = cli_populateServiceParameters(&pxServiceList, pcServiceName);
	}
	nRet = cli_analyzeCommand(pxInData->sCommandName, peMainOp, peSubOper, pcServiceName);
	IF_ERR_RETURNED_GOTO_END(nRet, "Unable to interpret the command %s\n", pxInData->sCommandName);
	/* Extract Command line arguments, either they are in name-value pair style or implicit style */
	/* The arguments are stored in respective linked-list, pxInData->pxArgList */
	if (pArgv->pargc > 0) {
		if (pxInData->pxCurrentService != NULL) {
			int nCnt = 0;
			for (nCnt = 0; nCnt < pArgv->pargc; nCnt++) {
				/* LOGF_LOG_DEBUG("\tparam Name: %s - Value: %s\n",pArgv->pargv[nCnt]->param->name, pArgv->pargv[nCnt]->value); */
				if ((strlen(pxInData->sCmdLineArgs) + strlen(pArgv->pargv[nCnt]->value) + 1) < MAX_LEN_ARG_LIST) {	/* +1 is to insert a space between parameters */
					strcat(pxInData->sCmdLineArgs, pArgv->pargv[nCnt]->value);
					strcat(pxInData->sCmdLineArgs, " ");
				} else {
					LOGF_LOG_WARNING("Arguments seems to be longer than %d. Please check \n", MAX_LEN_ARG_LIST);
				}
			}
			nRet = cli_parseCmdLineArguments(pArgv, pxInData);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error to parse Command Line Arguments\n");
		} else {
			pxInData->pArgv = pArgv;
		}
	}
	/* Extract links attribute and store in respective linked-list, pxInData->pxLinkObjList */
	if (pCommand->links) {
		nRet = cli_ignoreSpacesInStrings(&pcLinks, pCommand->links);
		if (nRet <= UGW_FAILURE) {
			LOGF_LOG_CRITICAL("Unable to allocate memory(%d bytes) for space-free links string\n", strlen(pCommand->links));
			goto end;
		}
	}
	if (pCommand->depends) {
		nRet = cli_ignoreSpacesInStrings(&pcDepends, pCommand->depends);
		if (nRet <= UGW_FAILURE) {
			LOGF_LOG_CRITICAL("Unable to allocate memory(%d bytes) for space-free depends string\n", strlen(pCommand->depends));
			goto end;
		}
	}
	if (pCommand->display) {
		nRet = cli_ignoreSpacesInStrings(&pcDisplay, pCommand->display);
		if (nRet <= UGW_FAILURE) {
			LOGF_LOG_CRITICAL("Unable to allocate memory(%d bytes) for space-free display string\n", strlen(pCommand->display));
			goto end;
		}
	}
	if (pCommand->hidden) {
		nRet = cli_ignoreSpacesInStrings(&pcHidden, pCommand->hidden);
		if (nRet <= UGW_FAILURE) {
			LOGF_LOG_CRITICAL("Unable to allocate memory(%d bytes) for space-free hidden string\n", strlen(pCommand->hidden));
			goto end;
		}
	}

	if (pxInData->pxCurrentService != NULL) {
		/* Extract links attribute and store in respective linked-list, pxInData->pxLinkObjList */
		if (pcLinks != NULL) {
			/* LOGF_LOG_DEBUG("\tThe \"links\" attribute is: %s\n", pcLinks); */
			nRet = cli_parseLinks(pcLinks, pxInData);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error in parsing Links attribute\n");
		}

		/* Extract depends attribute and store in respective linked-list, pxInData->pxDependsList */
		if (pcDepends != NULL) {
			/* LOGF_LOG_DEBUG("\tThe \"depends\" attribute is: %s\n", pcDepends); */
			nRet = cli_parseDepends(pcDepends, pxInData);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error in parsing Depends attribute\n");
		}

		/* Extract display attribute and store in respective linked-list, pxInData->pxDisplayList */
		if (pcDisplay != NULL) {
			/* LOGF_LOG_DEBUG("\tThe \"display\" attribute is: %s\n", pcDisplay); */
			nRet = cli_parseDisplay(pcDisplay, pxInData);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error in parsing Display attribute\n");
		}

		/* Extract hidden attribute and store in respective linked-list, pxInData->pxDisplayList */
		if (pcHidden != NULL) {
			/* LOGF_LOG_DEBUG("\tThe \"hidden\" attribute is: %s\n", pcHidden); */
			nRet = cli_parseHidden(pcHidden, pxInData);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error in parsing Hidden attribute\n");
		}
	}

	LOGF_LOG_DEBUG("Command: Name: %s,  View: Name: %s\n", pCommand->name, pView->name);
 end:
	if (pcLinks != NULL)
		HELP_FREE(pcLinks);
	if (pcDepends != NULL)
		HELP_FREE(pcDepends);
	if (pcDisplay != NULL)
		HELP_FREE(pcDisplay);
	if (pcHidden != NULL)
		HELP_FREE(pcHidden);
	return nRet;
}

/***********************************************************************************************************
Function:cli_checkObjDuplicationInObjList
Description: Checks if an Object of given name is already in the specific ObjectList
***********************************************************************************************************/
int cli_checkObjDuplicationInObjList(ObjList * pxObjList, char *pcObjName, ObjList ** pxObjOut)
{
	ObjList *pxObj = NULL;
	bool bDuplicate = 0;
	FOR_EACH_OBJ(pxObjList, pxObj) {
		if (strncmp(pxObj->sObjName, pcObjName, MAX_LEN_OBJNAME) == 0) {
			bDuplicate = 1;
			*pxObjOut = pxObj;
			break;
		}
	}
	return bDuplicate;
}

/***********************************************************************************************************
Function: cli_splitTR181String
Description: This function splits a TR181 string to Parameter part(ParamName), Object Name (ObjExtract),
			top level instantiated object (PreAsterisk), Child object Name(PostAsterisk)
***********************************************************************************************************/
int cli_splitTR181String(char *pcTR181fullpath, char *pcParamName, char *pcObjExtract, char *pcObjNamePreAsterisk, char *pcObjNamePostAsterisk)
{
	char *pLastDot = strrchr(pcTR181fullpath, CHAR_DOT);
	char *pWildcard = NULL;
	int nRet = UGW_SUCCESS;

	if (pLastDot != NULL) {
		int nOffset = pLastDot - pcTR181fullpath + 1;	/* +1 to consider the last CHAR_DOT itself */

		if (nOffset >= MAX_LEN_OBJNAME) {
			LOGF_LOG_ERROR("TR181 Object name %s is longer than %d\n", pcTR181fullpath, MAX_LEN_OBJNAME);
			nRet = UGW_FAILURE;
			goto end;
		}
		strncpy(pcObjExtract, pcTR181fullpath, nOffset);
		if (strlen(pLastDot + 1) >= MAX_LEN_PARAM_NAME) {
			LOGF_LOG_ERROR("TR181 Parameter Name %s is longer than %d\n", pcTR181fullpath, MAX_LEN_PARAM_NAME);
			nRet = UGW_FAILURE;
			goto end;
		}
		strncpy(pcParamName, (pLastDot + 1), MAX_LEN_PARAM_NAME - 1);	/* +1 to left out the last CHAR_DOT itself */
		/* LOGF_LOG_DEBUG("Extracted ObjName %s, Param Name %s\n", pcObjExtract, pcParamName); */

		pWildcard = strchr(pcObjExtract, CHAR_ASTERISK);
		if (pWildcard != NULL) {
			nOffset = pWildcard - pcObjExtract;
			strncpy(pcObjNamePreAsterisk, pcObjExtract, nOffset);
			strncpy(pcObjNamePostAsterisk, (pWildcard + 1), MAX_LEN_OBJNAME - 1);
		} else {
			/* Not a Multiinstance Object */
			strncpy(pcObjNamePreAsterisk, pcObjExtract, MAX_LEN_OBJNAME);
		}
	} else {
		LOGF_LOG_ERROR("The String %s doesn't seem to be a TR181 object\n", pcTR181fullpath);
		nRet = UGW_FAILURE;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_getUserDefinedParamName
Description: This function gets the user defined parameter name based on TR181-Param name. Just opposite to 
				cli_getTR181MappingParam()
***********************************************************************************************************/
int cli_getUserDefinedParamName(ServiceParamMapList_t * pxServParamMap, char *pcTR181param, char *pcUserparam)
{
	bool bMatchFound = false;
	ParamMapList_t *pxParamMapTrav = NULL;
	char sObjectName[MAX_LEN_OBJNAME] = { 0 };
	char sParamName[MAX_LEN_PARAM_NAME] = { 0 };
	char sObjectPreAst[MAX_LEN_OBJNAME] = { 0 };
	char sObjectPostAst[MAX_LEN_OBJNAME] = { 0 };
	char sMapObjectName[MAX_LEN_OBJNAME] = { 0 };
	char sMapParamName[MAX_LEN_PARAM_NAME] = { 0 };
	char sMapObjectPreAst[MAX_LEN_OBJNAME] = { 0 };
	char sMapObjectPostAst[MAX_LEN_OBJNAME] = { 0 };
	int nRet = UGW_SUCCESS;

	nRet = cli_splitTR181String(pcTR181param, sParamName, sObjectName, sObjectPreAst, sObjectPostAst);
	if(nRet != UGW_SUCCESS) {
		LOGF_LOG_INFO("Splitting %s using cli_splitTR181String failed\n", pcTR181param);
	}
	pxParamMapTrav = pxServParamMap->pxParamHead;
	while (pxParamMapTrav != NULL) {
		memset(sMapObjectName, 0, MAX_LEN_OBJNAME);
		memset(sMapParamName, 0, MAX_LEN_PARAM_NAME);
		memset(sMapObjectPreAst, 0, MAX_LEN_OBJNAME);
		memset(sMapObjectPostAst, 0, MAX_LEN_OBJNAME);
		nRet = cli_splitTR181String(pxParamMapTrav->sTR181ObjPath, sMapParamName, sMapObjectName, sMapObjectPreAst, sMapObjectPostAst);
		if(nRet != UGW_SUCCESS) {
			LOGF_LOG_INFO("Splitting %s using cli_splitTR181String failed\n", pxParamMapTrav->sTR181ObjPath);
		}
		/* if (cli_matchInstancedStrings(pxParamMapTrav->sTR181ObjPath, pcTR181param)) { */
		if (cli_matchInstancedStrings(sMapObjectName, sObjectName) && (strncmp(sMapParamName, sParamName, MAX_LEN_PARAM_NAME) == 0)) {
			bMatchFound = true;
			/* LOGF_LOG_DEBUG("\tMatch found for Tr181Param %s\n", pcTR181param); */
			memset(pcUserparam, 0, MAX_LEN_PARAM_NAME);
			strncpy(pcUserparam, pxParamMapTrav->sUserDefParamName, MAX_LEN_PARAM_NAME);
			break;
		}
		pxParamMapTrav = pxParamMapTrav->pxParamNext;
	}

	if (bMatchFound == false) {
		LOGF_LOG_ERROR("Unknown TR-181 Parameter %s\n", pcTR181param);
		return UGW_FAILURE;
	} else {
		return UGW_SUCCESS;
	}
}

/***********************************************************************************************************
Function: cli_getTR181MappingParam
Description: This function gets the TR181-Param name based on user defined parameter name. Just opposite to 
				cli_getUserDefinedParamName()
***********************************************************************************************************/
int cli_getTR181MappingParam(ServiceParamMapList_t * pxServParamMap, char *pcCmd, char *pcTR181param)
{
	bool bMatchFound = false;

	ParamMapList_t *pxParamMapTrav = NULL;

	/* if (strncasecmp(pcServName, pxServTrav->sServiceName, MAX_LEN_SS_NAME) == 0)  */
	{
		pxParamMapTrav = pxServParamMap->pxParamHead;
		while (pxParamMapTrav != NULL) {
			if (strncasecmp(pcCmd, pxParamMapTrav->sUserDefParamName, MAX_LEN_PARAM_NAME) == 0) {
				bMatchFound = true;
				/* LOGF_LOG_DEBUG("\tMatch found for CommandParam %s\n", pcCmd); */
				strncpy(pcTR181param, pxParamMapTrav->sTR181ObjPath, MAX_LEN_TR181OBJ_PATH);
				break;
			}
			pxParamMapTrav = pxParamMapTrav->pxParamNext;
		}
	}

	if (bMatchFound == false) {
		LOGF_LOG_ERROR("Unknown User Defined Parameter %s\n", pcCmd);
		return UGW_FAILURE;
	} else {
		return UGW_SUCCESS;
	}
}

/***********************************************************************************************************
Function: cli_getDisplayMode
Description: Returns the display mode.
***********************************************************************************************************/
int cli_getDisplayMode(int *pnMode)
{
	int nRet = UGW_SUCCESS;

	*pnMode = gnDisplayMode;

	return nRet;
}

/***********************************************************************************************************
Function: cli_analyzeCommand

Description: Reads command and determines if it is a Show command or Configure command. If Configure command,
			it further determines if it as add/delete/modify operation. Also if it is Show command, it reads
			service name as well.
***********************************************************************************************************/
int cli_analyzeCommand(char *pcCmd, MainOper * peMainOp, ObjOper * peSubOper, char *pcService)
{
	int nRet = UGW_SUCCESS;
	char *pcTmp = (char *)(pcCmd);
	char sStringVal[MAX_DEPTH_COMMAND][MAX_LEN_COMMAND];
	int nLen = 0, nPos = 0;

	if (strlen(pcCmd) == 0) {
		LOGF_LOG_ERROR("Invalid Command string, returning failure\n");
		return UGW_FAILURE;
	}
	memset(sStringVal, 0, (MAX_DEPTH_COMMAND * MAX_LEN_COMMAND));
	do {
		nLen = strcspn(pcTmp, " ");
		memset(sStringVal[nPos], 0, sizeof(sStringVal[nPos]));
		strncpy(sStringVal[nPos], pcTmp, nLen);
		pcTmp += nLen + 1;	/* +1 to consider space */
		nPos++;
	} while (pcTmp[-1]);

	if (strlen(sStringVal[OPERATION_TYPE_OFFSET]) > 0) {
		/* LOGF_LOG_DEBUG("The Operation is %s\n", sStringVal[OPERATION_TYPE_OFFSET]); */
		if (strncasecmp(pcCmd, "show", MAX_LEN_COMMAND_TYPE) == 0) {
			*peMainOp = MOPT_GET;
			if (strstr(pcCmd, STR_TRANSACTION_MODE_COMMAND) == NULL) {
				if (strlen(sStringVal[SERVICE_NAME_OFFSET]) > 0) {
					/* LOGF_LOG_DEBUG("The Service Name is %s\n", sStringVal[SERVICE_NAME_OFFSET]); */
					memset(pcService, 0, MAX_LEN_SERVICE_NAME);
					strncpy(pcService, sStringVal[SERVICE_NAME_OFFSET], MAX_LEN_SERVICE_NAME - 1);
				}
			}
		} else {

			if (strncasecmp(pcCmd, "set ", MAX_LEN_COMMAND_TYPE) == 0) {
				*peMainOp = MOPT_SET;
				*peSubOper = OBJOPT_MODIFY;
			} else if (strncasecmp(pcCmd, "add ", MAX_LEN_COMMAND_TYPE) == 0) {
				*peMainOp = MOPT_SET;
				*peSubOper = OBJOPT_ADD;
			} else if (strncasecmp(pcCmd, "del ", MAX_LEN_COMMAND_TYPE) == 0) {
				*peMainOp = MOPT_SET;
				*peSubOper = OBJOPT_DEL;
			} else {	/* TODO Diagnostic mode commands will be done later */
				/* LOGF_LOG_DEBUG(" %s is a generic Command\n", pcCmd); */
			}
		}
	}
	return nRet;
}

/***********************************************************************************************************
Function: cli_populateServiceParameters
Description: Based on Services name, it reads parameter map file and updates Service Parameter map
***********************************************************************************************************/
ServiceParamMapList_t *cli_populateServiceParameters(ServiceParamMapList_t ** ppxServList, char *pcServName)
{
	ServiceParamMapList_t *pxServTrav = *ppxServList;
	ServiceParamMapList_t *pxServTemp = NULL;
	int nNumOfServices = 0;
	int nServFound = 0;
	while (pxServTrav != NULL) {
		nNumOfServices++;
		if (strncasecmp(pcServName, pxServTrav->sServiceName, MAX_LEN_SERVICE_NAME) == 0) {
			nServFound = true;
			break;
		}
		pxServTemp = pxServTrav;
		pxServTrav = pxServTrav->pxServiceNext;
	}
	if (nServFound) {
		goto end;
	} else {
		FILE *pMapFile = NULL;
		char sFilename[MAX_LEN_FILE_NAME] = { 0 };

		LOGF_LOG_WARNING("Service Name Not Matched with any entry, Create new\n");

		if (nNumOfServices == 0) {
			*ppxServList = HELP_MALLOC(sizeof(ServiceParamMapList_t));
			pxServTrav = *ppxServList;
		} else {
			pxServTemp->pxServiceNext = HELP_MALLOC(sizeof(ServiceParamMapList_t));
			pxServTrav = pxServTemp->pxServiceNext;
		}
		if (pxServTrav == NULL) {
			LOGF_LOG_CRITICAL("Error in allocating Service Node\n");
			goto end;
		}

		strncpy(pxServTrav->sServiceName, pcServName, MAX_LEN_SERVICE_NAME);
		pxServTrav->pxServiceNext = NULL;

		snprintf(sFilename, MAX_LEN_FILE_NAME, "%s%s%s", MAP_FILE_DIRECTORY, pcServName, MAP_FILE_SUFFIX);
		pMapFile = fopen(sFilename, "r");
		if (pMapFile == NULL) {
			LOGF_LOG_CRITICAL("File %s open failure\n", sFilename);
			HELP_FREE(pxServTrav);
			if (nNumOfServices > 0) {
				pxServTemp->pxServiceNext = NULL;
			}
			pxServTrav = NULL;
			goto end;
		} else {

			int nLoop = 0;
			int nBytesRead = 0;
			char *pcLine = NULL;
			char *pcLineSpaces = NULL;
			unsigned int unLineSize = 200;	/* MAX_LINE_SIZE */
			char *pcOpenBrace = NULL;
			char *pcCloseBrace = NULL;
			char *pcSharp = NULL;
			char *pcComma = NULL;
			int nOffset = 0;
			ParamMapList_t *pxParamMapTrav = pxServTrav->pxParamHead;

			for (nLoop = 0; ((nBytesRead = getline(&pcLineSpaces, &unLineSize, pMapFile)) != -1); nLoop++) {

				if (nBytesRead < MIN_LEN_MAP_FILE_ENTRY) {
					continue;
				}
				cli_ignoreSpacesInStrings(&pcLine, pcLineSpaces);
				if (pcLine == NULL || (*pcLine == CHAR_HASH)) {
					HELP_FREE(pcLine);
					continue;
				}
				pcOpenBrace = strchr(pcLine, CHAR_OPEN_BRACE);
				pcCloseBrace = strrchr(pcLine, CHAR_CLOSE_BRACE);
				pcSharp = strrchr(pcLine, CHAR_HASH);
				if (pcOpenBrace != NULL) {
					pcComma = strchr(pcOpenBrace, CHAR_COMMA);
				}
				if ((pcOpenBrace == NULL) || (pcCloseBrace == NULL) || (pcComma == NULL) || ((pcSharp > pcOpenBrace) && (pcSharp < pcCloseBrace))) {
					LOGF_LOG_ERROR("Invalid format of Parameter Map %s\n", pcLine);
					HELP_FREE(pcLine);
					/* break; */
					continue;
				}
				if (pcSharp > pcCloseBrace) {
					/* LOGF_LOG_DEBUG("Commented after end of Map Info, Line %s\n", pcLine); */
				}
				if (pxServTrav->pxParamHead == NULL) {
					pxServTrav->pxParamHead = HELP_MALLOC(sizeof(ParamMapList_t));
					pxParamMapTrav = pxServTrav->pxParamHead;
				} else {
					pxParamMapTrav->pxParamNext = HELP_MALLOC(sizeof(ParamMapList_t));
					pxParamMapTrav = pxParamMapTrav->pxParamNext;
				}
				if (pxParamMapTrav == NULL) {
					LOGF_LOG_CRITICAL("Error in allocating Parameter Map node\n");
					/* pxServTrav = NULL; */
					HELP_FREE(pcLine);
					fclose(pMapFile);
					goto end;
				}
				nOffset = pcComma - (pcOpenBrace + 1 + 1);	/*  first +1 is to skip Open brace and second +1 is to skip open quote(") */
				strncpy(pxParamMapTrav->sUserDefParamName, (pcOpenBrace + 1 + 1), (nOffset - 1));	/* -1 to skip closing quote(") */
				nOffset = pcCloseBrace - (pcComma + 1 + 1);	/*  first +1 is to skip Comma and second +1 is to skip open quote(") */
				strncpy(pxParamMapTrav->sTR181ObjPath, (pcComma + 1 + 1), (nOffset - 1));	/* -1 to skip closing quote(") */
				HELP_FREE(pcLine);
			}
			fclose(pMapFile);

		}
	}

 end:

	return pxServTrav;
}

/***********************************************************************************************************
Function: cli_matchInstancedStrings
Description: Matches 2 Object Names, ignoring the CHAR_ASTERISK symbol in place of instance number.
***********************************************************************************************************/
bool cli_matchInstancedStrings(char *pcAsteriskStr, char *pcInstancedStr)
{
	char *pcAstStr = pcAsteriskStr;
	char *pcInstStr = pcInstancedStr;
	int nOffset = 0;
	bool bStrMatched = false;
	char *pcNextAst = NULL;
	char *pcNextDot = NULL;

	/* LOGF_LOG_DEBUG("cli_matchInstancedStrings %s - %s\n", pcAsteriskStr, pcInstancedStr); */

	while (strlen(pcAstStr) > 0) {
		/* LOGF_LOG_DEBUG("in While Loop %s - %s\n", pcAstStr, pcInstStr); */
		pcNextAst = strchr(pcAstStr, CHAR_ASTERISK);
		if (pcNextAst != NULL) {
			/* LOGF_LOG_DEBUG("Asterisk found \n"); */
			nOffset = pcNextAst - pcAstStr;
			if ((strncmp(pcAstStr, pcInstStr, nOffset) == 0)) {
				pcNextDot = strchr((pcAstStr + nOffset), CHAR_DOT);
				if (pcNextDot != NULL) {
					pcAstStr = pcNextDot;
				}
				pcNextDot = strchr((pcInstStr + nOffset), CHAR_DOT);
				if (pcNextDot != NULL) {
					pcInstStr = pcNextDot;
				}
				/* LOGF_LOG_DEBUG("Continue next iteration\n"); */
				continue;
			} else {
				/* bStrMatched = false; */
				/* LOGF_LOG_DEBUG("String NOT Matched \n"); */
				break;
			}
		} else {
			/* LOGF_LOG_DEBUG("Asterisk NOT found \n"); */
			if (strncmp(pcAstStr, pcInstStr, strlen(pcAstStr)) == 0) {
				/* if(strncmp(pcAstStr, pcInstStr, strlen(pcInstStr)) == 0) { */
				if ((strlen(pcInstStr) - strlen(pcAstStr)) < 4) {
					/* TODO to clear this workaround  */
					LOGF_LOG_DEBUG("Strings Matched %s - %s\n", pcAsteriskStr, pcInstancedStr);
					bStrMatched = true;
				}
			}
			break;
		}
	}
	return bStrMatched;
}

/***********************************************************************************************************
Function: cli_parseLinks
Description: This function Parses "links" attribute coming from XML. The purpose of this attribute is :
				To convey the relation between various objects.
***********************************************************************************************************/
int cli_parseLinks(char *pcLink, InputData_t * pxInData)
{
	int nRet = 1;
	char *pcDirection = NULL;	/* pointer pointing to CHAR_DIRECTION i.e "greater than" symbol */
	char *pcComma = NULL;	/* pointer pointing to CHAR_COMMA i.e Comma symbol */
	char sTempLinking[MAX_LEN_PARAM_NAME] = { 0 };
	char sTempLinked[MAX_LEN_PARAM_NAME] = { 0 };

	char *pcEnd = pcLink + strlen(pcLink);
	char *pcTrav = pcLink;
	while (pcTrav < pcEnd) {
		pcDirection = strchr(pcTrav, CHAR_DIRECTION);
		if (pcDirection != NULL) {
			if ((pcDirection - pcTrav) >= MAX_LEN_PARAM_NAME) {	/*Linking parameter length is compared here */
				LOGF_LOG_ERROR("Linking Param Name %s is longer than %d\n", pcTrav, MAX_LEN_PARAM_NAME);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(sTempLinking, pcTrav, (pcDirection - pcTrav));
		} else {
			LOGF_LOG_ERROR("There is error in links encoding; No \">\" symbol\n");
			nRet = UGW_FAILURE;
			goto end;
		}
		pcTrav = pcDirection + 1;	/* +1 to skip CHAR_DIRECTION symbol */
		pcComma = strchr(pcTrav, CHAR_COMMA);
		if (pcComma != NULL) {
			if ((pcComma - pcTrav) >= MAX_LEN_PARAM_NAME) {	/*Linked Object parameter length is compared here */
				LOGF_LOG_ERROR("Too long Linked Object Name %s\n", pcTrav);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(sTempLinked, pcTrav, (pcComma - pcTrav));
			pcTrav = pcComma + 1;	/* +1 to skip CHAR_COMMA symbol */
		} else {
			strncpy(sTempLinked, pcTrav, (MAX_LEN_PARAM_NAME - 1));
			pcTrav += strlen(pcTrav);
		}
		/* LOGF_LOG_DEBUG("\t %s >> %s\n", sTempLinking, sTempLinked); */
		nRet = cli_createAndSortLinkObjs(pxInData, sTempLinking, sTempLinked);
		memset(sTempLinking, 0, MAX_LEN_PARAM_NAME);
		memset(sTempLinked, 0, MAX_LEN_PARAM_NAME);
	}
	/* LOGF_LOG_DEBUG("Successfully reached end of Links loop\n"); */

	if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
		LinkObject_t *pxLinkTrav = pxInData->pxLinkObjList;
		int nLinks = 0;
		while (pxLinkTrav != NULL) {
			nLinks++;
			LOGF_LOG_DEBUG("\nLink %d\n", nLinks);
			LOGF_LOG_DEBUG("\tLinking Param %s Obj %s\n", pxLinkTrav->sLinkingObjParam, pxLinkTrav->sLinkingObj);
			LOGF_LOG_DEBUG("\tLinking Pre %s Post %s\n", pxLinkTrav->sLinkingObjPreAsterisk, pxLinkTrav->sLinkingObjPostAsterisk);
			LOGF_LOG_DEBUG("\tLinked Param %s Obj %s\n", pxLinkTrav->sLinkedObjParam, pxLinkTrav->sLinkedObj);
			LOGF_LOG_DEBUG("\tLinked Pre %s Post %s\n", pxLinkTrav->sLinkedObjPreAsterisk, pxLinkTrav->sLinkedObjPostAsterisk);
			LOGF_LOG_DEBUG("\tLink's Previous 0x%08x Next 0x%08x\n", (unsigned int)pxLinkTrav->pxLinkPrev, (unsigned int)pxLinkTrav->pxLinkNext);
			pxLinkTrav = pxLinkTrav->pxLinkNext;
		}
		LOGF_LOG_DEBUG("\n\tThe Links Head is 0x%08x Tail is: 0x%08x\n", (unsigned int)pxInData->pxLinkObjList, (unsigned int)pxInData->pxLinkObjListTail);
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_createAndSortLinkObjs
Description: This function creates "links" and does minimal sorting as user is expected to send in sorted way.
***********************************************************************************************************/
int cli_createAndSortLinkObjs(InputData_t * pxInData, char *pcLinking, char *pcLinked)
{
	int nRet = UGW_SUCCESS;
	LinkObject_t *pxLinkTrav = NULL;
	LinkObject_t *pxLinkTemp = NULL;
	LinkObject_t *pxLinkNew = NULL;

	LinkObject_t xLinkStruct;
	char sLinkingTR181Obj[MAX_LEN_TR181OBJ_PATH] = { 0 };
	char sLinkedTR181Obj[MAX_LEN_TR181OBJ_PATH] = { 0 };
	int nLinks = 0;
	bool bMatchFound = false;

	memset(&xLinkStruct, 0, sizeof(LinkObject_t));
	nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, pcLinking, sLinkingTR181Obj);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while getting TR181 name of Linking item %s\n", nRet, pcLinking);
	nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, pcLinked, sLinkedTR181Obj);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while getting TR181 name of Linked item %s\n", nRet, pcLinked);
	nRet = cli_splitTR181String(sLinkingTR181Obj, xLinkStruct.sLinkingObjParam, xLinkStruct.sLinkingObj, xLinkStruct.sLinkingObjPreAsterisk, xLinkStruct.sLinkingObjPostAsterisk);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while splitting TR181 name of Linking item %s\n", nRet, sLinkingTR181Obj);
	nRet = cli_splitTR181String(sLinkedTR181Obj, xLinkStruct.sLinkedObjParam, xLinkStruct.sLinkedObj, xLinkStruct.sLinkedObjPreAsterisk, xLinkStruct.sLinkedObjPostAsterisk);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while splitting TR181 name of Linked item %s\n", nRet, sLinkedTR181Obj);
	/* LOGF_LOG_DEBUG("%s  >  %s\n", sLinkingTR181Obj , sLinkedTR181Obj); */
	pxLinkTrav = pxInData->pxLinkObjList;
	while (pxLinkTrav != NULL) {
		nLinks++;
		/* LOGF_LOG_DEBUG("Link %d - IN\n", nLinks); */
		if (cli_matchInstancedStrings(pxLinkTrav->sLinkingObj, xLinkStruct.sLinkedObj)) {
			bMatchFound = true;
			pxLinkNew = HELP_MALLOC(sizeof(LinkObject_t));
			if (pxLinkNew == NULL) {
				LOGF_LOG_CRITICAL("Unable to allocated Memory for link object!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			memcpy(pxLinkNew, &xLinkStruct, sizeof(LinkObject_t));
			if (pxLinkTemp == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxLinkTemp!!\n");
				nRet = UGW_FAILURE;
				HELP_FREE(pxLinkNew);
				goto end;
			}
			pxLinkTemp->pxLinkNext = pxLinkNew;
			pxLinkNew->pxLinkNext = pxLinkTrav;
			pxLinkNew->pxLinkPrev = pxLinkTemp;
			pxLinkTrav->pxLinkPrev = pxLinkNew;
			break;
		}
		pxLinkTemp = pxLinkTrav;
		pxLinkTrav = pxLinkTrav->pxLinkNext;
	}
	if (bMatchFound == false) {
		if (nLinks == 0) {
			pxInData->pxLinkObjList = HELP_MALLOC(sizeof(LinkObject_t));
			pxLinkNew = pxInData->pxLinkObjList;
			if (pxLinkNew == NULL) {
				LOGF_LOG_CRITICAL("Unable to allocated Memory for link object!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			memcpy(pxLinkNew, &xLinkStruct, sizeof(LinkObject_t));
			pxInData->pxLinkObjList->pxLinkNext = NULL;
			pxInData->pxLinkObjList->pxLinkPrev = NULL;
			pxInData->pxLinkObjListTail = pxLinkNew;
			goto end;	/* SUCCESS case return */
		} else {
			if (pxLinkTemp == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxLinkTemp!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			pxLinkTemp->pxLinkNext = HELP_MALLOC(sizeof(LinkObject_t));
			pxLinkNew = pxLinkTemp->pxLinkNext;
			if (pxLinkNew == NULL) {
				LOGF_LOG_CRITICAL("Unable to allocated Memory for link object!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			memcpy(pxLinkNew, &xLinkStruct, sizeof(LinkObject_t));
			pxLinkNew->pxLinkNext = NULL;
			pxLinkNew->pxLinkPrev = pxLinkTemp;
			pxInData->pxLinkObjListTail = pxLinkNew;
			/* goto end; */
		}
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateGetObjListBasedOnLinkObjs
Description: This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on "links" attribute.
***********************************************************************************************************/
int cli_updateGetObjListBasedOnLinkObjs(ObjList * pObjType, InputData_t * pxInData, bool bShowMode)
{
	int nRet = UGW_SUCCESS;
	LinkObject_t *pxLinkTrav = pxInData->pxLinkObjList;
	ObjList *pxObj = NULL;
	int nDup = 0;

	while (pxLinkTrav != NULL) {
		/* LOGF_LOG_DEBUG("Adding %s & %s\n", pxLinkTrav->sLinkingObjPreAsterisk, pxLinkTrav->sLinkedObjPreAsterisk); */
		nDup = cli_checkObjDuplicationInObjList(pObjType, pxLinkTrav->sLinkingObjPreAsterisk, &pxObj);
		if (nDup == 0) {
			pxObj = HELP_OBJECT_GET(pObjType, pxLinkTrav->sLinkingObjPreAsterisk, SOPT_LEAFNODE);
			if (pxObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
				goto end;
			}
			if (bShowMode == false) {
				HELP_PARAM_GET(pxObj, ALIAS_PARAM_NAME, SOPT_LEAFNODE);
			}
		}
		if ((pxInData->xFilterParam.bFilled == true) && (strncmp(pxInData->xFilterParam.sPreAsterisk, pxLinkTrav->sLinkingObjPreAsterisk, MAX_LEN_OBJNAME) == 0)) {
			/* No need to add Linking parameter for Filtered Object, as we get the full object; */
		} else {
			if (bShowMode == false) {
				HELP_PARAM_GET(pxObj, pxLinkTrav->sLinkingObjParam, SOPT_LEAFNODE);
			}
		}
		nDup = cli_checkObjDuplicationInObjList(pObjType, pxLinkTrav->sLinkedObjPreAsterisk, &pxObj);
		if (nDup == 0) {
			pxObj = HELP_OBJECT_GET(pObjType, pxLinkTrav->sLinkedObjPreAsterisk, SOPT_LEAFNODE);
			if (pxObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
				goto end;
			}
			if (bShowMode == false) {
				HELP_PARAM_GET(pxObj, ALIAS_PARAM_NAME, SOPT_LEAFNODE);
			}
		}
		pxLinkTrav = pxLinkTrav->pxLinkNext;
	}
	LOGF_LOG_DEBUG("Get OBJLIST after updating List based on Links\n");
	/* if(gnDebugMode == DEBUG_ON_DEBUG_LEVEL) HELP_PRINT_OBJ(pObjType, SOPT_OBJVALUE); */
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateGetObjListBasedOnDepObjs
Description: This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on "depends" attribute.
***********************************************************************************************************/
int cli_updateGetObjListBasedOnDepObjs(ObjList * pObjType, DepObject_t * pxDepObj)
{
	int nRet = UGW_SUCCESS;
	DepObject_t *pxDepTrav = pxDepObj;
	ObjList *pxObj = NULL;
	int nDup = 0;

	while (pxDepTrav != NULL) {
		/* LOGF_LOG_DEBUG("Adding DepObj %s to Get List \n", pxDepTrav->sObjNamePreAsterisk); */
		nDup = cli_checkObjDuplicationInObjList(pObjType, pxDepTrav->sObjNamePreAsterisk, &pxObj);
		if (nDup == 0) {
			/* pxObj = HELP_OBJECT_GET(pObjType, pxDepTrav->sObjNamePreAsterisk, SOPT_OBJVALUE);   */
			/* HELP_PARAM_GET(pxObj, ALIAS_PARAM_NAME, SOPT_LEAFNODE);    */
			pxObj = HELP_OBJECT_GET(pObjType, pxDepTrav->sObjNamePreAsterisk, SOPT_LEAFNODE);
			if (pxObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
				goto end;
			}
			HELP_PARAM_GET(pxObj, ALIAS_PARAM_NAME, SOPT_LEAFNODE);
		}
		pxDepTrav = pxDepTrav->pxDepNext;
	}
	/* LOGF_LOG_DEBUG("Get OBJLIST after updating List based on Depends\n"); */
	/* if(gnDebugMode == DEBUG_ON_DEBUG_LEVEL) HELP_PRINT_OBJ(pObjType, SOPT_OBJVALUE); */

 end:
	return nRet;

}

/***********************************************************************************************************
Function: cli_updateSetObjListForAddBasedOnDepObjs
Description: This function updates Object List for Add scenario. The list is updated based on "depends", 
			"links" & Arguments received from XML.
***********************************************************************************************************/
int cli_updateSetObjListForAddBasedOnDepObjs(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData)
{
	LinkObject_t *pxLinkTrav = pxInData->pxLinkObjList;
	DepObject_t *pxDepTrav = pxInData->pxDependsList;
	ArgParameter_t *pxArgTrav = NULL;
	int nIndex = 0;
	char sNewAlias[MAX_LEN_PARAM_NAME] = { 0 };
	ObjList *pxObj = NULL;
	bool bMatchFound = false;
	int nRet = UGW_SUCCESS;

	while (pxDepTrav != NULL) {
		/* nIndex = cli_getUnusedInstanceofObjname(pxGetObjList, pxSetObjList, pxDepTrav->sObjNamePreAsterisk); */
		if (pxDepTrav->bParentUnderConstr == true) {
			nIndex = DEFAULT_ALIAS_SUFFIX;
		} else {
			nIndex = cli_getUnusedInstanceofObjname(pxGetMsgList, pxSetObjList, pxDepTrav->sObjNamePreAsterisk);
			if (nIndex < DEFAULT_ALIAS_SUFFIX) {
				LOGF_LOG_CRITICAL("Unexpected Instance Number %d!!\n", nIndex);
			}
		}
		pxDepTrav->nAliasNumber = nIndex;
		pxDepTrav = pxDepTrav->pxDepNext;
	}

	pxDepTrav = pxInData->pxDependsList;
	while (pxDepTrav != NULL) {
		snprintf(sNewAlias, MAX_LEN_PARAM_NAME, "%s-%d", pxDepTrav->sAliasPrefix, pxDepTrav->nAliasNumber);
		if (pxDepTrav->bParentUnderConstr == true) {
			char sChildObj[MAX_LEN_OBJNAME] = { 0 };
			char sChildPostAsterisk[MAX_LEN_OBJNAME] = { 0 };
			char *pcAstInChildObj = strchr(pxDepTrav->sObjNamePostAsterisk, CHAR_ASTERISK);
			/* LOGF_LOG_DEBUG("Hierarchical Object construction PreAst %s, PostAst %s\n", pxDepTrav->sObjNamePreAsterisk, pxDepTrav->sObjNamePostAsterisk); */
			if (pcAstInChildObj != NULL) {
				strncpy(sChildPostAsterisk, pxDepTrav->sObjNamePostAsterisk, (pcAstInChildObj - (pxDepTrav->sObjNamePostAsterisk)));
			} else {
				strncpy(sChildPostAsterisk, pxDepTrav->sObjNamePostAsterisk, MAX_LEN_OBJNAME);
			}
			snprintf(sChildObj, MAX_LEN_OBJNAME, "%s%s-%d%s", pxDepTrav->sObjNamePreAsterisk, pxDepTrav->pxDepParent->sAliasPrefix, pxDepTrav->pxDepParent->nAliasNumber,
				 sChildPostAsterisk);
			/* LOGF_LOG_DEBUG("The Obj Name in case of Hierarchical Object is %s\n", sChildObj); */
			if (pcAstInChildObj != NULL) {
				pxObj = HELP_OBJECT_SET(pxSetObjList, sChildObj, NO_ARG_VALUE, OBJOPT_ADD, SOPT_OBJVALUE);
				if (pxObj == NULL) {
					nRet = UGW_FAILURE;
					LOGF_LOG_CRITICAL("Unable to append a Object to SetObjList in ChildObj ADD scenario\n");
					goto end;
				}
				HELP_PARAM_SET(pxObj, ALIAS_PARAM_NAME, sNewAlias, SOPT_OBJVALUE);
			} else {
				pxObj = HELP_OBJECT_SET(pxSetObjList, sChildObj, NO_ARG_VALUE, OBJOPT_MODIFY, SOPT_OBJVALUE);
				if (pxObj == NULL) {
					nRet = UGW_FAILURE;
					LOGF_LOG_CRITICAL("Unable to append an Object to SetObjList in ChildObj Modify scenario\n");
					goto end;
				}
			}
		} else {
			pxObj = HELP_OBJECT_SET(pxSetObjList, pxDepTrav->sObjNamePreAsterisk, NO_ARG_VALUE, OBJOPT_ADD, SOPT_OBJVALUE);
			if (pxObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to SetObjList in ADD scenario\n");
				goto end;
			}
			HELP_PARAM_SET(pxObj, ALIAS_PARAM_NAME, sNewAlias, SOPT_OBJVALUE);
		}
		/* HELP_PARAM_SET(pxObj, ENABLE_PARAM_NAME, ENABLE_PARAM_SET, SOPT_OBJVALUE); */
		pxLinkTrav = pxInData->pxLinkObjList;
		while (pxLinkTrav) {
			if ((strncmp(pxDepTrav->sObjNamePreAsterisk, pxLinkTrav->sLinkingObjPreAsterisk, MAX_LEN_OBJNAME) == 0) && (strlen(pxDepTrav->sObjNamePostAsterisk) <= 1)) {
				char sMappedObjName[MAX_LEN_OBJNAME] = { 0 };
				if (strchr(pxLinkTrav->sLinkedObj, CHAR_ASTERISK) != NULL) {	/* If linkedObjName contains Instance number unknown */
					DepObject_t *pxDepSubTrav = pxInData->pxDependsList;
					while (pxDepSubTrav != NULL) {
						if (strncmp(pxDepSubTrav->sObjNamePreAsterisk, pxLinkTrav->sLinkedObjPreAsterisk, MAX_LEN_OBJNAME) == 0) {
							bMatchFound = true;
							snprintf(sMappedObjName, MAX_LEN_OBJNAME, "%s%s-%d.", pxDepSubTrav->sObjNamePreAsterisk, pxDepSubTrav->sAliasPrefix,
								 pxDepSubTrav->nAliasNumber);
							break;
						}
						pxDepSubTrav = pxDepSubTrav->pxDepNext;
					}
					if (bMatchFound == false) {	/* Case of Addition based on filter */
						if ((pxInData->xFilterParam.bFilled == true) && (pxInData->pxArgList != NULL)) {
							char sDBObject[MAX_LEN_OBJNAME] = { 0 };
							nRet = cli_getInitRefObjectBasedOnFilter(sDBObject, pxGetMsgList, pxInData->xFilterParam.sPreAsterisk, pxInData->xFilterParam.sObjName);
							if (strlen(sDBObject) == 0) {
								LOGF_LOG_ERROR("No Object available with the required filter Obj:%s - ParamName:%s - %s\n", pxInData->xFilterParam.sPreAsterisk,
									       pxInData->xFilterParam.sArgName, pxInData->xFilterParam.sArgValue);
								nRet = UGW_FAILURE;
								goto end;
							} else {
								if (cli_matchInstancedStrings(pxLinkTrav->sLinkedObj, sDBObject)) {
									/* LOGF_LOG_DEBUG("InitRefObject %s itself matched with LinkedObj %s\n", sDBObject, pxLinkTrav->sLinkedObj); */
									strncpy(sMappedObjName, sDBObject, MAX_LEN_OBJNAME);
								} else {
								}
							}
						}
						/* Else PRINT ERROR */
					}
				} else {
					strncpy(sMappedObjName, pxLinkTrav->sLinkedObj, MAX_LEN_OBJNAME);
				}
				HELP_PARAM_SET(pxObj, pxLinkTrav->sLinkingObjParam, sMappedObjName, SOPT_OBJVALUE);
			}
			pxLinkTrav = pxLinkTrav->pxLinkNext;
		}

		pxArgTrav = pxInData->pxArgList;
		while (pxArgTrav != NULL) {
			/* if( strncmp(pxDepTrav->sObjNamePreAsterisk, pxArgTrav->sPreAsterisk, MAX_LEN_OBJNAME) == 0) { */
			/* if(cli_matchInstancedStrings(pxDepTrav->sObjName, pxArgTrav->sObjName)) { */
			LOGF_LOG_DEBUG("Check  Arg %s-%s, against %s\n", pxArgTrav->sObjName, pxArgTrav->sParameter, pxDepTrav->sObjName);
			if (cli_matchInstancedStrings(pxDepTrav->sObjName, pxArgTrav->sObjName)) {
				HELP_PARAM_SET(pxObj, pxArgTrav->sParameter, pxArgTrav->sArgValue, SOPT_OBJVALUE);
			}
			pxArgTrav = pxArgTrav->pxArgNext;
		}
		pxDepTrav = pxDepTrav->pxDepNext;
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateSetObjListBasedOnHiddenParam
Description: This function based on "hidden" attribute, updates Object List to be configured using CAL-SET.
***********************************************************************************************************/
int cli_updateSetObjListBasedOnHiddenParam(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData)
{
	HiddenParam_t *pxHiddenTrav = pxInData->pxHiddenParamList;
	ObjList *pxObj = NULL;
	int nRet = UGW_SUCCESS;
	char sInstObjName[MAX_LEN_OBJNAME] = { 0 };
	bool bMatchFound = false;

	while (pxHiddenTrav != NULL) {
		bMatchFound = false;
		FOR_EACH_OBJ(pxSetObjList, pxObj) {
			memset(sInstObjName, 0, MAX_LEN_OBJNAME);
			snprintf(sInstObjName, MAX_LEN_OBJNAME, "%s*.", pxObj->sObjName);
			LOGF_LOG_DEBUG("Hidden Param case: Going to compare %s - %s\n", pxHiddenTrav->sLeftObjName, sInstObjName);
			if (cli_matchInstancedStrings(pxHiddenTrav->sLeftObjName, sInstObjName)) {
				LOGF_LOG_DEBUG("Match found Hidden Param LeftObject \n");
				if (pxHiddenTrav->bFixedValue == true) {
					/* LOGF_LOG_DEBUG("Hidden attribute - FixedValue case\n"); */
					HELP_PARAM_SET(pxObj, pxHiddenTrav->sLeftParamName, pxHiddenTrav->sRightParamName, SOPT_OBJVALUE);
					bMatchFound = true;
				}	/*else {

					   } */
				break;
			}
		}
		if ((bMatchFound == false) && (strncmp(pxHiddenTrav->sLeftObjName, pxHiddenTrav->sLeftPreAsterisk, MAX_LEN_OBJNAME) == 0)
		    && (pxHiddenTrav->bFixedValue == true)) {	/* Case of having no * in the ObjName */
			ObjList *pxNewObj = HELP_OBJECT_SET(pxSetObjList, pxHiddenTrav->sLeftObjName, NO_ARG_VALUE, OBJOPT_MODIFY, SOPT_OBJVALUE);
			LOGF_LOG_DEBUG("Append an Object to SetObjList for Hidden Parameter %s.%s\n", pxHiddenTrav->sLeftObjName, pxHiddenTrav->sLeftParamName);
			if (pxNewObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to SetObjList in MODIFY scenario\n");
				goto end;
			}	/*else {
				   TODO Link traversal 
				   } */
			HELP_PARAM_SET(pxNewObj, pxHiddenTrav->sLeftParamName, pxHiddenTrav->sRightParamName, SOPT_LEAFNODE);
		}
		pxHiddenTrav = pxHiddenTrav->pxHiddenNext;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_getUnusedInstanceofObjname
Description: This function returns the unused instance number, which will be used to form Alias suffix
			(number part).
***********************************************************************************************************/
int cli_getUnusedInstanceofObjname(GetMsgsList_t * pxGetMsgList, ObjList * pxSetObjList, char *pcObjName)
{
	ObjList *pxObj = NULL;
	int nHighInst = 0;
	char sInstNbr[10] = { 0 };
	ObjList *pxGetObjList = NULL;
	GetMsgsList_t *pxGetMsgTrav = pxGetMsgList;

	for (pxGetMsgTrav = pxGetMsgList; pxGetMsgTrav != NULL; pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext) {
		if (pxGetMsgTrav->nCalGetStatus <= UGW_FAILURE) {
			continue;
		}
		pxGetObjList = pxGetMsgTrav->xGetObjMsgHead.pObjType;

		FOR_EACH_OBJ(pxGetObjList, pxObj) {
			if (strncmp(pxObj->sObjName, pcObjName, strlen(pcObjName)) == 0) {
				char *pcInstNbr = pxObj->sObjName + strlen(pcObjName);
				char *pcNextDot = strchr(pcInstNbr, CHAR_DOT);
				if (pcNextDot != NULL) {
					strncpy(sInstNbr, pcInstNbr, (pcNextDot - pcInstNbr));
					nHighInst = atoi(sInstNbr);
				} else {
					char *pcNull = strchr(pcInstNbr, 0);
					strncpy(sInstNbr, pcInstNbr, (pcNull - pcInstNbr));
					nHighInst = atoi(sInstNbr);
				}
			}
		}
	}

	FOR_EACH_OBJ(pxSetObjList, pxObj) {
		if ((strncmp(pxObj->sObjName, pcObjName, strlen(pcObjName)) == 0) && (pxObj->unObjOper == OBJOPT_ADD)) {
			LOGF_LOG_DEBUG("There is a previous addition of the same ObjType %s\n", pcObjName);
			nHighInst++;
		}
	}

	/* [Scenario Eg: In batch mode, in Multiple add commands treatment, 2nd add shouldn't reuse the unused instance that was obtained in 1st add treatment] */
	if (nHighInst == 0) {
		LOGF_LOG_ERROR("There is no matching Object of name %s\n", pcObjName);
		nHighInst = DEFAULT_ALIAS_SUFFIX;
	} else {
		nHighInst++;
		LOGF_LOG_DEBUG("The unused instance of %s is %d\n", pcObjName, nHighInst);
	}
	return nHighInst;
}

/***********************************************************************************************************
Function: cli_updateSetObjListForDelBasedOnDepObjs
Description: This function prepares list of Objects to be deleted.
***********************************************************************************************************/
int cli_updateSetObjListForDelBasedOnDepObjs(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData)
{
	DepObject_t *pxDepTrav = pxInData->pxDependsList;
	/* xArgParam_t *pxArgTrav = pxArgList; */
	ObjList *pxDelObj = NULL;
	int nDup = 0;
	int nRet = UGW_SUCCESS;
	char sDBObject[MAX_LEN_OBJNAME] = { 0 };
	char sTempObjName[MAX_LEN_OBJNAME] = { 0 };

	nRet = cli_getInitRefObjectBasedOnFilter(sDBObject, pxGetMsgList, pxInData->xFilterParam.sPreAsterisk, pxInData->xFilterParam.sObjName);
	/* if(bMatchFound == false) { */
	if (strlen(sDBObject) == 0) {
		LOGF_LOG_ERROR("No Object available with the required filter Obj:%s - ParamName:%s - %s\n", pxInData->xFilterParam.sPreAsterisk, pxInData->xFilterParam.sArgName,
			       pxInData->xFilterParam.sArgValue);
		nRet = UGW_FAILURE;
		goto end;
	}

	pxDepTrav = pxInData->pxDependsList;
	while (pxDepTrav != NULL) {
		LOGF_LOG_DEBUG("\t\tObjName being deleted in Depends is : %s FilteredObj %s\n", pxDepTrav->sObjName, sDBObject);
		memset(sTempObjName, 0, MAX_LEN_OBJNAME);
		snprintf(sTempObjName, MAX_LEN_OBJNAME, "%s*.", pxDepTrav->sObjName);

		if (cli_matchInstancedStrings(sTempObjName, sDBObject)) {
			nDup = cli_checkObjDuplicationInObjList(pxSetObjList, sDBObject, &pxDelObj);
			if (nDup == 0) {
				pxDelObj = HELP_OBJECT_SET(pxSetObjList, sDBObject, NO_ARG_VALUE, OBJOPT_DEL, SOPT_OBJVALUE);
				if (pxDelObj == NULL) {
					nRet = UGW_FAILURE;
					LOGF_LOG_CRITICAL("Unable to append an Object to SetObjList in DELETE scenario");
					goto end;
				}
			}
		} else {
			if (pxInData->pxLinkObjList != NULL) {
				ObjList *pxRetrievedObj = NULL;
				ObjList *pxInstObj = NULL;
				nRet = cli_getForwardLinkedObj(sDBObject, pxDepTrav->sObjName, pxGetMsgList, &pxRetrievedObj, pxInData);
				if(nRet <= UGW_FAILURE) {
					if (pxRetrievedObj != NULL) {
						HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
					}
					LOGF_LOG_ERROR("Error in obtaining Forward linking object\n");
					goto end;
				}

				if (pxRetrievedObj == NULL) {	/* Probably that might be in Backward links */
					nRet = cli_getBackwardLinkedObj(sDBObject, pxDepTrav->sObjName, pxGetMsgList, &pxRetrievedObj, pxInData);
					if(nRet <= UGW_FAILURE) {
						if (pxRetrievedObj != NULL) {
							HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
						}
						LOGF_LOG_ERROR("Error in obtaining Backward linking object\n");
						goto end;
					}
				}

				if (pxRetrievedObj != NULL) {
					LOGF_LOG_DEBUG("Seems some %s Objects found in links\n", pxDepTrav->sObjName);
					FOR_EACH_OBJ(pxRetrievedObj, pxInstObj) {
						LOGF_LOG_DEBUG("One of the found Object to be deleted is %s\n", pxInstObj->sObjName);
						nDup = cli_checkObjDuplicationInObjList(pxSetObjList, pxInstObj->sObjName, &pxDelObj);
						if (nDup == 0) {
							pxDelObj = HELP_OBJECT_SET(pxSetObjList, pxInstObj->sObjName, NO_ARG_VALUE, OBJOPT_DEL, SOPT_OBJVALUE);
							if (pxDelObj == NULL) {
								nRet = UGW_FAILURE;
								LOGF_LOG_CRITICAL("Unable to append an Object to SetObjList in DELETE scenario");
								if (pxRetrievedObj != NULL) {
									HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
								}
								goto end;
							}
						}
					}
				}
				if (pxRetrievedObj != NULL) {
					HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
				}
				/* TODO Write Else case to check if the Object hasn't had any links, but different from the filtered/first object  */
			} else {
				/* TODO Search in GetObjList */
			}
		}
		pxDepTrav = pxDepTrav->pxDepNext;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateGetObjListBasedOnFilterParam
Description: This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on filter parameter.
***********************************************************************************************************/
int cli_updateGetObjListBasedOnFilterParam(ObjList * pxGetObjList, ArgParameter_t * pxFilterArg)
{
	int nRet = UGW_SUCCESS;
	ObjList *pxObj = NULL;
	LOGF_LOG_DEBUG("Filtering based on Object %s Param %s = %s\n", pxFilterArg->sObjName, pxFilterArg->sArgName, pxFilterArg->sArgValue);
	/* pxObj = HELP_OBJECT_GET(pxGetObjList, sPreAsterisk, SOPT_OBJVALUE); */
	pxObj = HELP_OBJECT_GET(pxGetObjList, pxFilterArg->sPreAsterisk, SOPT_LEAFNODE);
	if (pxObj == NULL) {
		nRet = UGW_FAILURE;
		LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
		goto end;
	}
	HELP_PARAM_SET(pxObj, pxFilterArg->sParameter, pxFilterArg->sArgValue, SOPT_LEAFNODE);
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_parseCmdLineArguments
Description: This function parses command line arguments attribute received from XML. For all parameters to 
				be set, the arguments taken in name-value pairs. The parameter to be used as Filter would 
				come directly. The filtering is done based on first parameter.
***********************************************************************************************************/
int cli_parseCmdLineArguments(clish_pargv_t * pArgv, InputData_t * pxInData)
{

	int nRet = UGW_SUCCESS;
	int nArg = 0;
	char sTR181string[MAX_LEN_TR181OBJ_PATH] = { 0 };
	ArgParameter_t xLocalArg;
	ArgParameter_t *pxArgTrav = NULL;

	for (nArg = 0; nArg < pArgv->pargc; nArg++) {
		memset(&xLocalArg, 0, sizeof(ArgParameter_t));
		/* if(strstr(pArgv->pargv[nArg]->param->name, "arg_name") != NULL) { */
		if (strstr(pArgv->pargv[nArg]->param->name, "arg_") != NULL) {
			if (strlen(pArgv->pargv[nArg]->value) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Argument Name %s is longer than %d\n", pArgv->pargv[nArg]->value, MAX_LEN_PARAM_NAME);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(xLocalArg.sArgName, pArgv->pargv[nArg]->value, MAX_LEN_PARAM_NAME);
			nArg++;
			if (nArg >= pArgv->pargc) {
				LOGF_LOG_ERROR("Something missed in parameter Name-Value pairs\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			/* if(strstr(pArgv->pargv[nArg]->param->name, "arg_value") == NULL) { */
			if (strstr(pArgv->pargv[nArg]->param->name, "arg_") == NULL) {
				LOGF_LOG_ERROR("Violation of Name-Value pair naming guideline near %s, Please correct !!\n", xLocalArg.sArgName);
			}
		} else {
			if (strlen(pArgv->pargv[nArg]->param->name) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Argument Name %s is longet than %d\n", pArgv->pargv[nArg]->param->name, MAX_LEN_PARAM_NAME);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(xLocalArg.sArgName, pArgv->pargv[nArg]->param->name, MAX_LEN_PARAM_NAME);
		}
		strncpy(xLocalArg.sArgValue, pArgv->pargv[nArg]->value, MAX_LEN_PARAM_VALUE);
		nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, xLocalArg.sArgName, sTR181string);
		IF_ERR_RETURNED_GOTO_END(nRet, "No matching Parameter/Object in Map for %s\n", xLocalArg.sArgName);

		nRet = cli_splitTR181String(sTR181string, xLocalArg.sParameter, xLocalArg.sObjName, xLocalArg.sPreAsterisk, xLocalArg.sPostAsterisk);
		IF_ERR_RETURNED_GOTO_END(nRet, "Unable to split the TR181 string %s\n", sTR181string);

		if (pxInData->pxArgList == NULL) {
			/* LOGF_LOG_DEBUG("First node of Argument List creation\n"); */
			pxInData->pxArgList = HELP_MALLOC(sizeof(ArgParameter_t));
			if (pxInData->pxArgList == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for Argument list node\n");
				goto end;
			}
			pxArgTrav = pxInData->pxArgList;
		} else {
			/* LOGF_LOG_DEBUG("Chained node of Argument List creation\n"); */
			if (pxArgTrav == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxArgTrav!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			pxArgTrav->pxArgNext = HELP_MALLOC(sizeof(ArgParameter_t));
			if (pxArgTrav->pxArgNext == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for Argument list node\n");
				goto end;
			}
			pxArgTrav = pxArgTrav->pxArgNext;
		}
		memcpy(pxArgTrav, &xLocalArg, sizeof(ArgParameter_t));
		pxArgTrav->pxArgNext = NULL;

		if (nArg == 0) {
			strncpy(pxInData->xFilterParam.sArgName, xLocalArg.sArgName, MAX_LEN_PARAM_NAME);
			strncpy(pxInData->xFilterParam.sArgValue, xLocalArg.sArgValue, MAX_LEN_PARAM_VALUE);
			strncpy(pxInData->xFilterParam.sObjName, xLocalArg.sObjName, MAX_LEN_OBJNAME);
			strncpy(pxInData->xFilterParam.sPreAsterisk, xLocalArg.sPreAsterisk, MAX_LEN_OBJNAME);
			strncpy(pxInData->xFilterParam.sPostAsterisk, xLocalArg.sPostAsterisk, MAX_LEN_OBJNAME);
			strncpy(pxInData->xFilterParam.sParameter, xLocalArg.sParameter, MAX_LEN_PARAM_NAME);
			pxInData->xFilterParam.bFilled = true;
		}
		memset(sTR181string, 0, MAX_LEN_TR181OBJ_PATH);
	}

	if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
		pxArgTrav = pxInData->pxArgList;
		nArg = 0;
		while (pxArgTrav != NULL) {
			nArg++;
			LOGF_LOG_DEBUG("\nArg %d\n", nArg);
			LOGF_LOG_DEBUG("\tArgName %s ArgValue %s Obj %s Param %s\n", pxArgTrav->sArgName, pxArgTrav->sArgValue, pxArgTrav->sObjName, pxArgTrav->sParameter);
			pxArgTrav = pxArgTrav->pxArgNext;
		}
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_parseDepends
Description: This function parses "depends" attribute received from XML. This attribute's purpose is:
				to convey the list of objects to be created/deleted.
***********************************************************************************************************/
int cli_parseDepends(char *pcDepends, InputData_t * pxInData)
{
	int nRet = 1;
	char *pcColon = NULL;	/* pointer pointing to CHAR_COLON i.e Colon symbol */
	char *pcComma = NULL;	/* pointer pointing to CHAR_COMMA i.e Comma symbol */
	char sUserObjName[MAX_LEN_PARAM_NAME] = { 0 };
	char sAliasPrefix[MAX_LEN_PARAM_NAME] = { 0 };
	char *pcEnd = pcDepends + strlen(pcDepends);
	char *pcTrav = pcDepends;
	DepObject_t *pxDepTrav = NULL;
	DepObject_t xDepStruct;
	char sTR181string[MAX_LEN_TR181OBJ_PATH] = { 0 };
	char sParam[MAX_LEN_PARAM_NAME] = { 0 };

	while (pcTrav < pcEnd) {
		memset(&xDepStruct, 0, sizeof(DepObject_t));
		pcColon = strchr(pcTrav, CHAR_COLON);
		if (pcColon != NULL) {
			if ((pcColon - pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Param Name %s of Depends attribute is longer than %d\n", pcTrav, MAX_LEN_PARAM_NAME);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(sUserObjName, pcTrav, (pcColon - pcTrav));
		} else {
			LOGF_LOG_ERROR("There is error in \"depends\" encoding; No \":\" symbol\n");
			nRet = UGW_FAILURE;
			goto end;
		}
		pcTrav = pcColon + 1;	/* +1 to skip CHAR_COLON symbol */
		pcComma = strchr(pcTrav, CHAR_COMMA);
		if (pcComma != NULL) {
			if ((pcComma - pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Alias Prefix %s is longer than %d\n", pcTrav, MAX_LEN_PARAM_NAME);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(xDepStruct.sAliasPrefix, pcTrav, (pcComma - pcTrav));
			pcTrav = pcComma + 1;	/* +1 to skip CHAR_COMMA symbol */
		} else {
			if (strlen(pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Alias Prefix %s is longer than %d\n", pcTrav, MAX_LEN_PARAM_NAME);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(xDepStruct.sAliasPrefix, pcTrav, MAX_LEN_PARAM_NAME);
			pcTrav += strlen(pcTrav);
		}
		/* LOGF_LOG_DEBUG("\tObject %s : Alias %s\n", sUserObjName, xDepStruct.sAliasPrefix); */

		nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, sUserObjName, sTR181string);
		IF_ERR_RETURNED_GOTO_END(nRet, "No matching Parameter/Object in Map for %s\n", sUserObjName);

		nRet = cli_splitTR181String(sTR181string, sParam, xDepStruct.sObjName, xDepStruct.sObjNamePreAsterisk, xDepStruct.sObjNamePostAsterisk);
		IF_ERR_RETURNED_GOTO_END(nRet, "Unable to split the TR181 string %s\n", sTR181string);

		if (pxInData->pxDependsList == NULL) {
			pxInData->pxDependsList = HELP_MALLOC(sizeof(DepObject_t));
			if (pxInData->pxDependsList == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for 1st node of Depending Obj List\n");
				goto end;
			}

			pxDepTrav = pxInData->pxDependsList;
			/* goto end; *//* SUCCESS case return */
		} else {
			if (pxDepTrav == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxDepTrav!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			pxDepTrav->pxDepNext = HELP_MALLOC(sizeof(DepObject_t));
			if (pxDepTrav->pxDepNext == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for a node of Depending Obj List\n");
				goto end;
			}
			pxDepTrav = pxDepTrav->pxDepNext;
			/* goto end; */
		}
		memcpy(pxDepTrav, &xDepStruct, sizeof(DepObject_t));
		pxDepTrav->pxDepNext = NULL;

		if (strlen(pxDepTrav->sObjNamePostAsterisk) > 1) {	/* 1 is needed since a . might exist */
			/* get Parent & Set flag and copy Parent */
			/* LOGF_LOG_DEBUG("DepTrav->PostAsterisk is %s\n", pxDepTrav->sObjNamePostAsterisk); */
			DepObject_t *pxDepSubTrav = pxInData->pxDependsList;
			while (pxDepSubTrav != NULL) {
				/* LOGF_LOG_DEBUG("Compare %s - %s to findout if Parent is under construction\n", pxDepSubTrav->sObjName, pxDepTrav->sObjNamePreAsterisk); */
				if (strncmp(pxDepSubTrav->sObjNamePreAsterisk, pxDepTrav->sObjNamePreAsterisk, MAX_LEN_OBJNAME) == 0) {
					if (strlen(pxDepSubTrav->sObjNamePostAsterisk) <= 1) {	/* 1 is needed since a . might exist */
						LOGF_LOG_DEBUG("Found that the Parent is Under construction\n");
						pxDepTrav->bParentUnderConstr = true;
						pxDepTrav->pxDepParent = pxDepSubTrav;
						break;
					}
				}
				pxDepSubTrav = pxDepSubTrav->pxDepNext;
			}
		}

		memset(sUserObjName, 0, MAX_LEN_PARAM_NAME);
		memset(sAliasPrefix, 0, MAX_LEN_PARAM_NAME);
		memset(sTR181string, 0, MAX_LEN_TR181OBJ_PATH);
	}

	if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
		DepObject_t *pxDepTrav = pxInData->pxDependsList;
		int nDeps = 0;
		while (pxDepTrav != NULL) {
			nDeps++;
			LOGF_LOG_DEBUG("\nDep %d\n", nDeps);
			LOGF_LOG_DEBUG("\tObjName %s AliasPrefix %s bParentUnderConstr %d\n", pxDepTrav->sObjName, pxDepTrav->sAliasPrefix, pxDepTrav->bParentUnderConstr);
			pxDepTrav = pxDepTrav->pxDepNext;
		}

	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_parseHidden
Description: This function parses "hidden" attribute received from XML. This attribute's purpose is:
				To parse default values to few of the parameters needed during configuration
***********************************************************************************************************/
int cli_parseHidden(char *pcHidden, InputData_t * pxInData)
{
	int nRet = 1;
	char *pcColon = NULL;	/* pointer pointing to CHAR_COLON i.e Colon symbol */
	char *pcComma = NULL;	/* pointer pointing to CHAR_COMMA i.e Comma symbol */
	char sLeftUserObjName[MAX_LEN_PARAM_NAME] = { 0 };
	char sRightUserObjName[MAX_LEN_PARAM_NAME] = { 0 };
	char *pcEnd = pcHidden + strlen(pcHidden);
	char *pcTrav = pcHidden;
	HiddenParam_t *pxHiddenTrav = NULL;
	HiddenParam_t xLocalHiddenParam;
	char sLeftTR181string[MAX_LEN_TR181OBJ_PATH] = { 0 };
	char sRightTR181string[MAX_LEN_TR181OBJ_PATH] = { 0 };

	while (pcTrav < pcEnd) {
		memset(&xLocalHiddenParam, 0, sizeof(HiddenParam_t));
		pcColon = strchr(pcTrav, CHAR_COLON);
		if (pcColon != NULL) {
			if ((pcColon - pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Too long Hidden Param Name %s\n", pcTrav);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(sLeftUserObjName, pcTrav, (pcColon - pcTrav));
		} else {
			LOGF_LOG_ERROR("There is error in \"Hidden\" encoding; No \":\" symbol\n");
			nRet = UGW_FAILURE;
			goto end;
		}
		pcTrav = pcColon + 1;	/* +1 to skip CHAR_COLON symbol */
		pcComma = strchr(pcTrav, CHAR_COMMA);
		if (pcComma != NULL) {
			if ((pcComma - pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Too long Right side %s\n", pcTrav);
				nRet = UGW_FAILURE;
				goto end;
			}
			if (*pcTrav == CHAR_QUOTE) {
				xLocalHiddenParam.bFixedValue = true;
				strncpy(xLocalHiddenParam.sRightParamName, pcTrav + 1, ((pcComma - pcTrav) - 2));	/* +1 to ignore opening quote and -2 to ignore closing quote */
			} else {
				xLocalHiddenParam.bFixedValue = false;
				strncpy(sRightUserObjName, pcTrav, (pcComma - pcTrav));
			}
			pcTrav = pcComma + 1;	/* +1 to skip CHAR_COMMA symbol */
		} else {
			if (strlen(pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Too long Right side Param %s\n", pcTrav);
				nRet = UGW_FAILURE;
				goto end;
			}
			if (*pcTrav == CHAR_QUOTE) {
				xLocalHiddenParam.bFixedValue = true;
				strncpy(xLocalHiddenParam.sRightParamName, pcTrav + 1, ((strlen(pcTrav)) - 2));	/* +1 to ignore opening quote and -2 to ignore closing quote */
			} else {
				xLocalHiddenParam.bFixedValue = false;
				strncpy(sRightUserObjName, pcTrav, (pcComma - pcTrav));
			}
			strncpy(sRightUserObjName, pcTrav, MAX_LEN_PARAM_NAME);
			pcTrav += strlen(pcTrav);
		}

		nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, sLeftUserObjName, sLeftTR181string);
		IF_ERR_RETURNED_GOTO_END(nRet, "No matching Parameter/Object in Map for %s\n", sLeftUserObjName);

		nRet =
		    cli_splitTR181String(sLeftTR181string, xLocalHiddenParam.sLeftParamName, xLocalHiddenParam.sLeftObjName, xLocalHiddenParam.sLeftPreAsterisk, xLocalHiddenParam.sLeftPostAsterisk);
		IF_ERR_RETURNED_GOTO_END(nRet, "Unable to split the TR181 string %s\n", sLeftTR181string);

		if (xLocalHiddenParam.bFixedValue == false) {
			nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, sRightUserObjName, sRightTR181string);
			IF_ERR_RETURNED_GOTO_END(nRet, "No matching Parameter/Object in Map for %s\n", sRightUserObjName);

			nRet =
			    cli_splitTR181String(sRightTR181string, xLocalHiddenParam.sRightParamName, xLocalHiddenParam.sRightObjName, xLocalHiddenParam.sRightPreAsterisk,
						 xLocalHiddenParam.sRightPostAsterisk);
			IF_ERR_RETURNED_GOTO_END(nRet, "Unable to split the TR181 string %s\n", sRightTR181string);
		}

		if (pxInData->pxHiddenParamList == NULL) {
			pxInData->pxHiddenParamList = HELP_MALLOC(sizeof(HiddenParam_t));
			if (pxInData->pxHiddenParamList == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for 1st node of Hidden param List\n");
				goto end;
			}
			pxHiddenTrav = pxInData->pxHiddenParamList;
			/* goto end; *//* SUCCESS case return */
		} else {
			if (pxHiddenTrav == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxHiddenTrav!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			pxHiddenTrav->pxHiddenNext = HELP_MALLOC(sizeof(HiddenParam_t));
			if (pxHiddenTrav->pxHiddenNext == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for a node of Hidden param List\n");
				goto end;
			}
			pxHiddenTrav = pxHiddenTrav->pxHiddenNext;
			/* goto end; */
		}
		memcpy(pxHiddenTrav, &xLocalHiddenParam, sizeof(HiddenParam_t));
		pxHiddenTrav->pxHiddenNext = NULL;

		memset(sLeftUserObjName, 0, MAX_LEN_PARAM_NAME);
		memset(sRightUserObjName, 0, MAX_LEN_PARAM_NAME);
		memset(sLeftTR181string, 0, MAX_LEN_TR181OBJ_PATH);
		memset(sRightTR181string, 0, MAX_LEN_TR181OBJ_PATH);
	}
	if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
		HiddenParam_t *pxHiddenTrav = pxInData->pxHiddenParamList;
		int nHidden = 0;
		while (pxHiddenTrav != NULL) {
			nHidden++;
			LOGF_LOG_DEBUG("\nHidden Param 1%d\n", nHidden);
			LOGF_LOG_DEBUG("\tLeftParam %s%s sRightValue %s%s Fixed: %d\n", pxHiddenTrav->sLeftObjName, pxHiddenTrav->sLeftParamName, pxHiddenTrav->sRightObjName,
				       pxHiddenTrav->sRightParamName, pxHiddenTrav->bFixedValue);
			pxHiddenTrav = pxHiddenTrav->pxHiddenNext;
		}

	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_parseDisplay
Description: Parses "display" attribute received from XML. To clarify a new user, "display" attribute versus
			"dislay mode": "display" parameter comes from XML to convey what are all the parameters to be 
			displayed in the o/p, whereas the "display mode" is in what fashion(list, json, table) the output 
			has to be displayed.
***********************************************************************************************************/
int cli_parseDisplay(char *pcDisplay, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	char *pcComma = NULL;
	char *pcEnd = NULL;
	char *pcTrav = pcDisplay;
	DisplayParameter_t xDisplayParam;
	DisplayParameter_t *pxDisplayTrav = NULL;
	char sShortString[MAX_LEN_PARAM_NAME] = { 0 };
	char sTR181string[MAX_LEN_PARAM_NAME] = { 0 };

	pcEnd = pcDisplay + strlen(pcDisplay);
	while (pcTrav < pcEnd) {
		memset(&xDisplayParam, 0, sizeof(DisplayParameter_t));
		pcComma = strchr(pcTrav, CHAR_COMMA);
		if (pcComma != NULL) {
			if ((pcComma - pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Too long name of Display Parameter: %s\n", pcTrav);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(sShortString, pcTrav, (pcComma - pcTrav));
			pcTrav = pcComma + 1;	/* +1 to skip CHAR_COMMA symbol */
		} else {
			if (strlen(pcTrav) >= MAX_LEN_PARAM_NAME) {
				LOGF_LOG_ERROR("Too long Linking Param Name %s\n", pcTrav);
				nRet = UGW_FAILURE;
				goto end;
			}
			strncpy(sShortString, pcTrav, MAX_LEN_PARAM_NAME);
			pcTrav += strlen(pcTrav);
		}

		nRet = cli_getTR181MappingParam(pxInData->pxCurrentService, sShortString, sTR181string);
		IF_ERR_RETURNED_GOTO_END(nRet, "No matching Parameter/Object in Map for %s\n", sShortString);

		nRet = cli_splitTR181String(sTR181string, xDisplayParam.sParamName, xDisplayParam.sObjName, xDisplayParam.sPreAsterisk, xDisplayParam.sPostAsterisk);
		IF_ERR_RETURNED_GOTO_END(nRet, "Unable to split the TR181 string %s\n", sTR181string);

		if (pxInData->pxDisplayList == NULL) {
			pxInData->pxDisplayList = HELP_MALLOC(sizeof(DisplayParameter_t));
			if (pxInData->pxDisplayList == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for 1st node of Display param List\n");
				goto end;
			}
			pxDisplayTrav = pxInData->pxDisplayList;
		} else {
			if (pxDisplayTrav == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxDisplayTrav!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			pxDisplayTrav->pxDisplayNext = HELP_MALLOC(sizeof(DisplayParameter_t));
			if (pxDisplayTrav->pxDisplayNext == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for a node of Display param List\n");
				goto end;
			}
			pxDisplayTrav = pxDisplayTrav->pxDisplayNext;
		}
		memcpy(pxDisplayTrav, &xDisplayParam, sizeof(DisplayParameter_t));
		pxDisplayTrav->pxDisplayNext = NULL;

		memset(sShortString, 0, MAX_LEN_PARAM_NAME);
		memset(sTR181string, 0, MAX_LEN_TR181OBJ_PATH);
		pcEnd = pcDisplay + strlen(pcDisplay);	/* TODO Check why the loop is failing without this line */
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_freeGlobalStructures
Description: This function frees various dynamically allocated structures created while parsing XML datas
***********************************************************************************************************/
void cli_freeGlobalStructures(InputData_t * pxInData)
{
	ArgParameter_t *pxArgTrav = pxInData->pxArgList;
	ArgParameter_t *pxArgTemp = NULL;
	LinkObject_t *pxLinkTrav = pxInData->pxLinkObjList;
	LinkObject_t *pxLinkTemp = NULL;
	DepObject_t *pxDepTrav = pxInData->pxDependsList;
	DepObject_t *pxDepTemp = NULL;
	DisplayParameter_t *pxDisplayTrav = pxInData->pxDisplayList;
	DisplayParameter_t *pxDisplayTemp = NULL;
	HiddenParam_t *pxHiddenTrav = pxInData->pxHiddenParamList;
	HiddenParam_t *pxHiddenTemp = NULL;

	while (pxArgTrav != NULL) {
		pxArgTemp = pxArgTrav;
		pxArgTrav = pxArgTrav->pxArgNext;
		HELP_FREE(pxArgTemp);
	}
	pxInData->pxArgList = NULL;
	/* memset(&(pxInData->xFilterParam), 0 , sizeof(ArgParameter_t)); */

	while (pxLinkTrav != NULL) {
		pxLinkTemp = pxLinkTrav;
		pxLinkTrav = pxLinkTrav->pxLinkNext;
		HELP_FREE(pxLinkTemp);
	}
	pxInData->pxLinkObjList = NULL;

	while (pxDepTrav != NULL) {
		pxDepTemp = pxDepTrav;
		pxDepTrav = pxDepTrav->pxDepNext;
		HELP_FREE(pxDepTemp);
	}
	pxInData->pxDependsList = NULL;

	while (pxDisplayTrav != NULL) {
		pxDisplayTemp = pxDisplayTrav;
		pxDisplayTrav = pxDisplayTrav->pxDisplayNext;
		HELP_FREE(pxDisplayTemp);
	}
	pxInData->pxDisplayList = NULL;

	while (pxHiddenTrav != NULL) {
		pxHiddenTemp = pxHiddenTrav;
		pxHiddenTrav = pxHiddenTrav->pxHiddenNext;
		HELP_FREE(pxHiddenTemp);
	}
	pxInData->pxHiddenParamList = NULL;
	LOGF_LOG_DEBUG("Successfully freed various Heap allocations\n");
}

/***********************************************************************************************************
Function: cli_configureCommandHandler
Description: This function is top-level function for various configuration commands. It further branches to
				specific handler based on sub operation type.
***********************************************************************************************************/
int cli_configureCommandHandler(ObjOper eSubOp, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	MsgHeader xGetObjMsgHead;

	if (strstr(pxInData->sCommandName, "display") != NULL) {
		nRet = cli_setDisplayCommandHandler(pxInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error in handling \"set display\" command");
		xGetObjMsgHead.pObjType = NULL;
		goto end;
	}
	if (strstr(pxInData->sCommandName, "debug") != NULL) {
		nRet = cli_setDebugCommandHandler(pxInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error in handling \"set debug\" command");
		xGetObjMsgHead.pObjType = NULL;
		goto end;
	}

	if (eSubOp == OBJOPT_MODIFY) {
		HELP_CREATE_MSG(&xGetObjMsgHead, MOPT_GET, SOPT_OBJVALUE, OWN_CLI, 1);
	} else {
		HELP_CREATE_MSG(&xGetObjMsgHead, MOPT_GET, SOPT_LEAFNODE, OWN_CLI, 1);
	}

	/* LOGF_LOG_DEBUG("MsgHead created in xGetObjMsgHead\n"); */
	if (xGetObjMsgHead.pObjType == NULL) {
		LOGF_LOG_ERROR("Get ObjectList is NULL \n");
		goto end;
	}

	if ((gbTransactionMode == false) || (gxSetObjMsgHead.pObjType == NULL)) {
		HELP_CREATE_MSG(&gxSetObjMsgHead, MOPT_SET, SOPT_OBJVALUE, OWN_CLI, 1);
		if (gxSetObjMsgHead.pObjType == NULL) {
			LOGF_LOG_CRITICAL("Set ObjectList is NULL \n");
			nRet = UGW_FAILURE;
			goto end;
		}
		LOGF_LOG_DEBUG("ObjList created in gxSetObjMsgHead.pObjType in Implicit transaction mode\n");
	} else {
		nRet = cli_appendCommandToCommitAwaitingList(pxInData);
	}

	if (eSubOp == OBJOPT_ADD) {
		nRet = cli_addCommandHandler(&xGetObjMsgHead, &gxSetObjMsgHead, pxInData);
	} else if (eSubOp == OBJOPT_DEL) {
		nRet = cli_deleteCommandHandler(&xGetObjMsgHead, &gxSetObjMsgHead, pxInData);
	} else if (eSubOp == OBJOPT_MODIFY) {
		nRet = cli_modifyCommandHandler(&xGetObjMsgHead, &gxSetObjMsgHead, pxInData);
	}

 end:
	if (xGetObjMsgHead.pObjType != NULL) {
		HELP_DELETE_OBJ(xGetObjMsgHead.pObjType, SOPT_OBJVALUE, FREE_OBJLIST);
		/* HELP_DELETE_MSG(&xGetObjMsgHead); */
	}
	if ((gbTransactionMode == false) && (gxSetObjMsgHead.pObjType != NULL)) {
		HELP_DELETE_OBJ(gxSetObjMsgHead.pObjType, SOPT_OBJVALUE, FREE_OBJLIST);
		/* HELP_DELETE_MSG(&gxSetObjMsgHead); */
	}
	return nRet;
}

/***********************************************************************************************************
Function: cli_showCommandHandler
Description: This function handles Show commands starting from CAL-GET to displaying the data to user.
***********************************************************************************************************/
int cli_showCommandHandler(InputData_t * pxInData, ObjList * pxCalSetObjList)
{
	int nRet = UGW_SUCCESS;
	int nDispMode = DISPLAY_MODE_JSON;
	MsgHeader xGetObjMsgHead;
	ArgParameter_t *pxOutData = NULL;
	GetMsgsList_t *pxGetMsgList = NULL;

	if (pxCalSetObjList == NULL) {
		if (strstr(pxInData->sCommandName, STR_TRANSACTION_MODE_COMMAND) != NULL) {
			nRet = cli_showTransactionModeCommandHandler();
			IF_ERR_RETURNED_GOTO_END(nRet, "Error in handling \"show transactionmode\" command");
			xGetObjMsgHead.pObjType = NULL;
			goto end;
		}
		/* HELP_CREATE_MSG(&xGetObjMsgHead, MOPT_GET, SOPT_LEAFNODE, OWN_CLI, 1); */
		HELP_CREATE_MSG(&xGetObjMsgHead, MOPT_GET, SOPT_OBJVALUE, OWN_CLI, 1);
		/* LOGF_LOG_DEBUG("MsgHead created in xGetObjMsgHead\n"); */
		if (xGetObjMsgHead.pObjType == NULL) {
			LOGF_LOG_ERROR("Get ObjectList is NULL in Show scenario \n");
			goto end;
		}
		LOGF_LOG_DEBUG("Inside Show Specific operation\n");
		if ((pxInData->xFilterParam.bFilled == true) && (pxInData->pxArgList != NULL)) {
			nRet = cli_updateGetObjListBasedOnFilterParam(xGetObjMsgHead.pObjType, &(pxInData->xFilterParam));
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d to update GetObjList based on Filter parameter\n", nRet);
		}
		if (pxInData->pxLinkObjList != NULL) {
			nRet = cli_updateGetObjListBasedOnLinkObjs(xGetObjMsgHead.pObjType, pxInData, true);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d to update GetObjList based on Link Object list\n", nRet);
		}
		if (pxInData->pxDisplayList != NULL) {
			nRet = cli_updateGetObjListBasedOnDisplay(xGetObjMsgHead.pObjType, pxInData);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d to update GetObjList based on Display parameter list\n", nRet);

			/* if(pxInData->xFilterParam.bFilled == false) {
			   } */
		}
		LOGF_LOG_DEBUG("Printing Objlist before CAL-GET in SHOW scenario\n");
		if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL)
			HELP_PRINT_OBJ(xGetObjMsgHead.pObjType, SOPT_OBJVALUE);
		nRet = cli_forkGetObjListForEachObject(xGetObjMsgHead.pObjType, SOPT_OBJVALUE, pxInData, &pxGetMsgList, false);

	} else {
		pxInData->xFilterParam.bFilled = false;
		nRet = cli_forkGetObjListForEachObject(pxCalSetObjList, SOPT_OBJVALUE, pxInData, &pxGetMsgList, true);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while preparing for forking of CAL-GET requests\n", nRet);
	}

	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while preparing for forking of CAL-GET requests\n", nRet);
	nRet = cli_executeConcatenatedCalGet(pxGetMsgList);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while forking CAL-GET requests\n", nRet);

	nRet = cli_extractDataToDisplayFromGetObjList(pxGetMsgList, pxInData, &pxOutData);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while extracting Display parameters from the received Object List\n", nRet);
	if (pxOutData == NULL) {
		LOGF_LOG_ERROR("Failure to extract Output Data in SHOW scenario\n");
		nRet = UGW_FAILURE;
		goto end;
	}
	nRet = cli_specificActionsonDisplayData(&pxOutData, pxInData);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while doing specific filtration on Display Data\n", nRet);
	nRet = cli_getDisplayMode(&nDispMode);
	nRet = cli_printOutput(pxOutData, nDispMode, pxInData);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while printing Display Data output\n", nRet);
	nRet = cli_freeOutputStructures(pxOutData);
	IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while freeing Output Data Structures\n", nRet);
 end:
	cli_freeConcatenatedGetObjList(pxGetMsgList);
	if (pxCalSetObjList == NULL) {
		if (xGetObjMsgHead.pObjType != NULL) {
			HELP_DELETE_OBJ(xGetObjMsgHead.pObjType, SOPT_OBJVALUE, FREE_OBJLIST);
			/* HELP_DELETE_MSG(&xGetObjMsgHead); */
		}
	}
	return nRet;
}

/***********************************************************************************************************
Function: cli_extractDataToDisplayFromGetObjList
Description: This function reads data from received Objlist from CAL-GET, and prepares data to be displayed.
				It prepares the display data as rows & columns fashion, where each row corresponds to one set 
				of display parameters.
***********************************************************************************************************/
int cli_extractDataToDisplayFromGetObjList(GetMsgsList_t * pxGetMsgList, InputData_t * pxInData, ArgParameter_t ** pxOutList)
{
	int nRet = UGW_SUCCESS;
	DisplayParameter_t *pxDispTrav = NULL;
	ArgParameter_t *pxOutTrav = NULL;
	ObjList *pxObj = NULL;
	ObjList *pxRefObj = NULL;
	char sObjToCheck[MAX_LEN_OBJNAME] = { 0 };
	char sDBObject[MAX_LEN_OBJNAME] = { 0 };
	char sObjToCheckWildcard[MAX_LEN_OBJNAME] = { 0 };
	bool bFirstNodeInRow = false;
	//bool bObjFound = false;
	ObjList *pxGetObjList = NULL;
	GetMsgsList_t *pxGetMsgTrav = NULL;

	if (pxInData->xFilterParam.bFilled == true) {
		strncpy(sObjToCheck, pxInData->xFilterParam.sPreAsterisk, MAX_LEN_OBJNAME);
		if (strlen(pxInData->pxDisplayList->sPostAsterisk) == 0) {	/* In case of Fixed Instances mentioned in Map files */
			strncpy(sObjToCheckWildcard, pxInData->xFilterParam.sPreAsterisk, MAX_LEN_OBJNAME);
		} else {
			snprintf(sObjToCheckWildcard, MAX_LEN_OBJNAME, "%s*.", pxInData->xFilterParam.sPreAsterisk);
		}
	} else {
		/* initialize sObjToCheck to pxDisplayList's first nodes sPreAsterisk */
		strncpy(sObjToCheck, pxInData->pxDisplayList->sPreAsterisk, MAX_LEN_OBJNAME);
		if (strlen(pxInData->pxDisplayList->sPostAsterisk) == 0) {	/* In case of Fixed Instances mentioned in Map files */
			strncpy(sObjToCheckWildcard, pxInData->pxDisplayList->sPreAsterisk, MAX_LEN_OBJNAME);
		} else {
			snprintf(sObjToCheckWildcard, MAX_LEN_OBJNAME, "%s*.", pxInData->pxDisplayList->sPreAsterisk);
		}

	}
	LOGF_LOG_DEBUG("Object chosen to filter is %s- %s\n", sObjToCheck, sObjToCheckWildcard);
	for (pxGetMsgTrav = pxGetMsgList; pxGetMsgTrav != NULL; pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext) {
		if (pxGetMsgTrav->nCalGetStatus <= UGW_FAILURE) {
			continue;
		}
		pxGetObjList = pxGetMsgTrav->xGetObjMsgHead.pObjType;
		FOR_EACH_OBJ(pxGetObjList, pxObj) {
			memset(sDBObject, 0, MAX_LEN_OBJNAME);
			if (pxObj->sObjName[(strlen(pxObj->sObjName) - 1)] == CHAR_DOT) {
				strncpy(sDBObject, pxObj->sObjName, MAX_LEN_OBJNAME);
			} else {
				snprintf(sDBObject, MAX_LEN_OBJNAME, "%s.", pxObj->sObjName);
			}
			/* snprintf(sDBObject, MAX_LEN_OBJNAME, "%s.", pxObj->sObjName); */
			//bObjFound = false;
			pxRefObj = NULL;
			/* LOGF_LOG_DEBUG("Comparing %s - %s\n", sObjToCheckWildcard, sDBObject); */
			if (cli_matchInstancedStrings(sObjToCheckWildcard, sDBObject)) {
				LOGF_LOG_DEBUG("Inside Match of %s- %s\n", sObjToCheckWildcard, sDBObject);
				if (*pxOutList == NULL) {
					/* LOGF_LOG_DEBUG("First Row creation\n"); */
					*pxOutList = HELP_MALLOC(sizeof(ArgParameter_t));
					if (*pxOutList == NULL) {
						nRet = UGW_FAILURE;
						LOGF_LOG_CRITICAL("Unable to allocate memory for 1st row of OutputDisplay Data\n");
						goto end;
					}
					pxOutTrav = *pxOutList;
				} else {
					ArgParameter_t *pxOutRowTrav = *pxOutList;
					ArgParameter_t *pxOutRowTemp = NULL;
					int nCnt = 1;
					while (pxOutRowTrav != NULL) {
						nCnt++;
						pxOutRowTemp = pxOutRowTrav;
						pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
					}
					/* LOGF_LOG_DEBUG("Row #%d creation\n", nCnt); */
					pxOutRowTemp->pxArgNextRow = HELP_MALLOC(sizeof(ArgParameter_t));
					if (pxOutRowTemp->pxArgNextRow == NULL) {
						nRet = UGW_FAILURE;
						LOGF_LOG_CRITICAL("Unable to allocate memory for a row of OutputDisplay Data\n");
						goto end;
					}
					pxOutTrav = pxOutRowTemp->pxArgNextRow;
				}
				bFirstNodeInRow = true;
				pxDispTrav = pxInData->pxDisplayList;
				while (pxDispTrav != NULL) {
					//bObjFound = false;
					LOGF_LOG_DEBUG("\tDisplay Object: %s, Parameter %s\n", pxDispTrav->sObjName, pxDispTrav->sParamName);

					if (bFirstNodeInRow == true) {
						bFirstNodeInRow = false;
						/* LOGF_LOG_DEBUG("FirstNode in Row scenario; No need to create structure; \n"); */
					} else {
						pxOutTrav->pxArgNext = HELP_MALLOC(sizeof(ArgParameter_t));
						if (pxOutTrav->pxArgNext == NULL) {
							nRet = UGW_FAILURE;
							LOGF_LOG_CRITICAL("Unable to allocate memory for a node of OutputDisplay Data\n");
							goto end;
						}
						pxOutTrav = pxOutTrav->pxArgNext;
						/* LOGF_LOG_DEBUG("Case of Chained Node in Row ;\n"); */
					}
					snprintf(pxOutTrav->sParameter, MAX_LEN_PARAM_NAME, "%s%s", pxDispTrav->sObjName, pxDispTrav->sParamName);
					if (cli_matchInstancedStrings(pxDispTrav->sPreAsterisk, sDBObject) == 0) {	/* NOT MATCHED */
						ObjList *pxRetrievedObj = NULL;
						ObjList *pxInstObj = NULL;

						nRet = cli_getForwardLinkedObj(sDBObject, pxDispTrav->sPreAsterisk, pxGetMsgList, &pxRetrievedObj, pxInData);
						if(nRet <= UGW_FAILURE) {
							if (pxRetrievedObj != NULL) {
								HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
							}
		                                        LOGF_LOG_ERROR("Error in obtaining Forward linking object\n");
                		                        goto end;
                                		}	

						if (pxRetrievedObj == NULL) {	/* Probably that might be in Backward links */
							nRet = cli_getBackwardLinkedObj(sDBObject, pxDispTrav->sPreAsterisk, pxGetMsgList, &pxRetrievedObj, pxInData);
							if(nRet <= UGW_FAILURE) {
								if (pxRetrievedObj != NULL) {
									HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
								}
	                                                        LOGF_LOG_ERROR("Error in obtaining Backward linking object\n");
                                                        	goto end;
                                                	}
						}
						if (pxRetrievedObj != NULL) {
							LOGF_LOG_DEBUG("Seems some %s Objects found in links\n", pxDispTrav->sPreAsterisk);
							GetMsgsList_t *pxGetMsgTempTrav = pxGetMsgList;
							ObjList *pxGetObjListTemp = NULL;
							ObjList *pxTempObj = NULL;
							FOR_EACH_OBJ(pxRetrievedObj, pxInstObj) {
								LOGF_LOG_DEBUG("One of the found Object is %s\n", pxInstObj->sObjName);
								for (pxGetMsgTempTrav = pxGetMsgList; pxGetMsgTempTrav != NULL; pxGetMsgTempTrav = pxGetMsgTempTrav->pxGetMsgNext) {
									if (pxGetMsgTempTrav->nCalGetStatus <= UGW_FAILURE) {
										continue;
									}
									/* if(cli_matchInstancedStrings(pxGetMsgTempTrav->sObjName, pxDispTrav->sPreAsterisk) == 0) { */
									if (cli_matchInstancedStrings(pxDispTrav->sPreAsterisk, pxGetMsgTempTrav->sObjName) == 0) {
										continue;
									}
									pxGetObjListTemp = pxGetMsgTempTrav->xGetObjMsgHead.pObjType;
									break;
								}
								if (pxGetObjListTemp == NULL) {
									nRet = UGW_FAILURE;
									LOGF_LOG_DEBUG("Unable to get pointer to retrieved object\n");
									if (pxRetrievedObj != NULL) {
										HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
									}
									goto end;
								}
								/* LOGF_LOG_DEBUG("Before FOR loop to get ObjPtr matching to %s\n", pxInstObj->sObjName); */
								FOR_EACH_OBJ(pxGetObjListTemp, pxTempObj) {
									if (strncmp(pxTempObj->sObjName, pxInstObj->sObjName, MAX_LEN_OBJNAME) == 0) {
										pxRefObj = pxTempObj;
										nRet = cli_updateDisplayParameterNodeWithRefObj(pxOutTrav, pxDispTrav, pxRefObj, pxGetMsgList);
										//bObjFound = true;
										break;
									}
								}
							}
						} else {
							LOGF_LOG_DEBUG("There is no link to %s object which is different from FilterObj %s\n", pxDispTrav->sObjName, sDBObject);
							if (strncmp(pxDispTrav->sObjName, pxDispTrav->sPreAsterisk, MAX_LEN_OBJNAME) == 0) {
//							if (cli_matchInstancedStrings(pxDispTrav->sObjName, pxDispTrav->sPreAsterisk)) {

								LOGF_LOG_DEBUG("Exact Object Name is given as Display parameter path\n");
								pxRefObj = cli_getSpecificObjectPointer(pxDispTrav->sObjName, pxDispTrav->sPreAsterisk, pxGetMsgList);
								if (pxRefObj != NULL) {
									LOGF_LOG_DEBUG("Going to update Display parameter\n");
									nRet = cli_updateDisplayParameterNodeWithRefObj(pxOutTrav, pxDispTrav, pxRefObj, pxGetMsgList);
									//bObjFound = true;
								}
							}
						}
						if (pxRetrievedObj != NULL) {
                                                	HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
                                        	}
						/* TODO Write Else case to check if the Object hasn't had any links, but different from the filtered/first object */
					} else {
						//bObjFound = true;
						pxRefObj = pxObj;
						LOGF_LOG_DEBUG("Top Level Reference Object found to Display parameter\n");
						nRet = cli_updateDisplayParameterNodeWithRefObj(pxOutTrav, pxDispTrav, pxRefObj, pxGetMsgList);
					}

					pxDispTrav = pxDispTrav->pxDisplayNext;
				}
			}
		}
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateDisplayParameterNodeWithRefObj
Description: This function updates specific node(parameter) in output display list with value read from 
			corresponding Reference object. It takes care if the concerned object is in hierarchy of RefObj.
***********************************************************************************************************/
int cli_updateDisplayParameterNodeWithRefObj(ArgParameter_t * pxOutTrav, DisplayParameter_t * pxDispTrav, ObjList * pxTopRefObj, GetMsgsList_t * pxGetMsgList)
{
	int nRet = UGW_SUCCESS;
	ObjList *pxRefObj = pxTopRefObj;
	ParamList *pxParam = NULL;

	if (strlen(pxDispTrav->sPostAsterisk) > 0) {
		ObjList *pxTempObj = NULL;
		char sTempObjName[MAX_LEN_OBJNAME] = { 0 };
		bool bChildObjFound = false;
		GetMsgsList_t *pxGetMsgTempTrav = pxGetMsgList;
		ObjList *pxGetObjListTemp = NULL;
		for (pxGetMsgTempTrav = pxGetMsgList; pxGetMsgTempTrav != NULL; pxGetMsgTempTrav = pxGetMsgTempTrav->pxGetMsgNext) {
			if (pxGetMsgTempTrav->nCalGetStatus <= UGW_FAILURE) {
				continue;
			}
			pxGetObjListTemp = pxGetMsgTempTrav->xGetObjMsgHead.pObjType;
			FOR_EACH_OBJ(pxGetObjListTemp, pxTempObj) {
				memset(sTempObjName, 0, MAX_LEN_OBJNAME);
				if (pxTempObj->sObjName[(strlen(pxTempObj->sObjName) - 1)] == CHAR_DOT) {
					strncpy(sTempObjName, pxTempObj->sObjName, MAX_LEN_OBJNAME);
				} else {
					snprintf(sTempObjName, MAX_LEN_OBJNAME, "%s.", pxTempObj->sObjName);
				}
				/* LOGF_LOG_DEBUG("TempObj %s - DbObj2Chk %s\n", sTempObjName, sDbObjToCheck); */
				if (cli_matchInstancedStrings(pxDispTrav->sObjName, sTempObjName) && strstr(sTempObjName, pxRefObj->sObjName)) {
					LOGF_LOG_DEBUG("Old RefObj %s, New RefObj %s\n", pxRefObj->sObjName, sTempObjName);
					pxRefObj = pxTempObj;
					bChildObjFound = true;
					break;
				}
			}
			if (bChildObjFound == true) {
				break;
			}
		}
	}
	LOGF_LOG_DEBUG("RefObjName %s\n", pxRefObj->sObjName);
	FOR_EACH_PARAM(pxRefObj, pxParam) {
		/* LOGF_LOG_DEBUG("\t\t\t\tFetching parameter if %s - %s matches\n",pxParam->sParamName,pxDispTrav->sParamName); */
		if (strncmp(pxParam->sParamName, pxDispTrav->sParamName, MAX_LEN_PARAM_NAME) == 0) {
			char sObjNameDotted[MAX_LEN_OBJNAME] = { 0 };
			if (pxRefObj->sObjName[(strlen(pxRefObj->sObjName) - 1)] == CHAR_DOT) {
				strncpy(sObjNameDotted, pxRefObj->sObjName, MAX_LEN_OBJNAME);
			} else {
				snprintf(sObjNameDotted, MAX_LEN_OBJNAME, "%s.", pxRefObj->sObjName);
			}
			LOGF_LOG_DEBUG("\t\tGot Parameter %s%s with value %s \n", sObjNameDotted, pxParam->sParamName, pxParam->sParamValue);
			if (strlen(pxOutTrav->sArgValue) > 0) {
				char sTempArgVal[MAX_LEN_PARAM_VALUE] = { 0 };
				strncpy(sTempArgVal, pxOutTrav->sArgValue, MAX_LEN_PARAM_VALUE);
				memset(pxOutTrav->sArgValue, 0, MAX_LEN_PARAM_VALUE);
				snprintf(pxOutTrav->sArgValue, MAX_LEN_PARAM_VALUE, "%s,%s", sTempArgVal, pxParam->sParamValue);
			} else {
				strncpy(pxOutTrav->sArgValue, pxParam->sParamValue, MAX_LEN_PARAM_VALUE);
			}
			/* if(strlen(pxDispTrav->sPostAsterisk) == 0) {
			   snprintf(pxOutTrav->sParameter, MAX_LEN_PARAM_NAME, "%s%s", pxRefObj->sObjName, pxParam->sParamName);
			   } else {
			   snprintf(pxOutTrav->sParameter, MAX_LEN_PARAM_NAME, "%s.%s", pxRefObj->sObjName, pxParam->sParamName);
			   } */
			snprintf(pxOutTrav->sParameter, MAX_LEN_PARAM_NAME, "%s%s", sObjNameDotted, pxParam->sParamName);

			pxOutTrav->pxArgNext = NULL;
			pxOutTrav->pxArgNextRow = NULL;
			break;
		}
	}
/* end: */
	return nRet;
}

/***********************************************************************************************************
Function: cli_printOutput
Description: This function prints Output in specified Display mode.
***********************************************************************************************************/
int cli_printOutput(ArgParameter_t * pxOutList, int nMode, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;

	if (nMode == DISPLAY_MODE_TABLE) {
		cli_printInTabularForm(pxOutList, pxInData);
	} else if (nMode == DISPLAY_MODE_JSON) {
		cli_printInJsonForm(pxOutList, pxInData);
	} else {		/*if (nMode == DISPLAY_MODE_LIST) */

		ArgParameter_t *pxOutRowTrav = pxOutList;
		ArgParameter_t *pxOutTrav = NULL;
		int nCnt = 0;
		int nNumberOfRows = 0;

		while (pxOutRowTrav != NULL) {
			nNumberOfRows++;
			pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
		}

		pxOutRowTrav = pxOutList;
		while (pxOutRowTrav != NULL) {
			nCnt++;
			if (nNumberOfRows > 1) {
				PRINT_TO_CONSOLE("%s %d:\n", pxInData->sDisplayTag, nCnt);
			} else {
				/*if(pxInData->pxArgList)
				   PRINT_TO_CONSOLE("%s %s:\n", pxInData->sDisplayTag, pxInData->pxArgList->sArgName);
				   else */
				PRINT_TO_CONSOLE("%s :\n", pxInData->sDisplayTag);
			}
			pxOutTrav = pxOutRowTrav;
			while (pxOutTrav != NULL) {
				nRet = cli_getUserDefinedParamName(pxInData->pxCurrentService, pxOutTrav->sParameter, pxOutTrav->sParameter);
				PRINT_TO_CONSOLE("\t%s : %s\n", pxOutTrav->sParameter, pxOutTrav->sArgValue);
				pxOutTrav = pxOutTrav->pxArgNext;
			}
			pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
		}
	}

	/* TODO print based nMode */
	return nRet;
}

/***********************************************************************************************************
Function: cli_freeOutputStructures
Description: This function free output structures, which are organized as rows and columns.
***********************************************************************************************************/
int cli_freeOutputStructures(ArgParameter_t * pxOutList)
{
	int nRet = UGW_SUCCESS;

	ArgParameter_t *pxOutRowTrav = pxOutList;
	ArgParameter_t *pxOutRowTemp = NULL;
	ArgParameter_t *pxOutTrav = NULL;
	ArgParameter_t *pxOutTemp = NULL;
	int nCnt = 0;

	while (pxOutRowTrav != NULL) {
		nCnt++;
		/* LOGF_LOG_DEBUG("Freeing Row Number %d:\n", nCnt); */
		pxOutRowTemp = pxOutRowTrav->pxArgNextRow;
		pxOutTrav = pxOutRowTrav;
		while (pxOutTrav != NULL) {
			pxOutTemp = pxOutTrav->pxArgNext;
			HELP_FREE(pxOutTrav);
			pxOutTrav = pxOutTemp;
		}
		pxOutRowTrav = pxOutRowTemp;
	}
	return nRet;
}

/***********************************************************************************************************
Function: cli_genericCommandHandler
Description: This function handles Generic commands (start, commit or cancel)
***********************************************************************************************************/
int cli_genericCommandHandler(InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;

	if (strncasecmp(pxInData->sCommandName, "start", MAX_LEN_COMMAND) == 0) {
		LOGF_LOG_WARNING("Start Transaction Mode\n");
		HELP_CREATE_MSG(&gxSetObjMsgHead, MOPT_SET, SOPT_OBJVALUE, OWN_CLI, 1);
		if (gxSetObjMsgHead.pObjType == NULL) {
			LOGF_LOG_ERROR("Set ObjectList is NULL \n");
			nRet = UGW_FAILURE;
			goto end;
		}
		gbTransactionMode = true;

	} else if (strncasecmp(pxInData->sCommandName, "commit", MAX_LEN_COMMAND) == 0) {
		LOGF_LOG_INFO("Confirm & Conclude Transaction Mode\n");
		/*if(bObjToSet == true) */
		{
			LOGF_LOG_DEBUG("Printing ObjectList Before CAL_SET\n");
			if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
				HELP_PRINT_MSG(&gxSetObjMsgHead);
			}
			nRet = cal_setValue(&gxSetObjMsgHead);
			if (nRet <= UGW_FAILURE) {
				PRINT_TO_CONSOLE("Failure of cal_set in COMMIT scenario\n");
				/* if(gnDebugMode == DEBUG_ON_DEBUG_LEVEL) HELP_PRINT_OBJ(gxSetObjMsgHead.pObjType, SOPT_OBJVALUE); */
				cli_handleErrorResponseCode(gxSetObjMsgHead.pObjType);
			} else {
				PRINT_TO_CONSOLE("Success of cal_set in COMMIT scenario\n");
			}
			LOGF_LOG_INFO("\n\ncal_set completed Successfully\n\n");
		}
		HELP_DELETE_OBJ(gxSetObjMsgHead.pObjType, SOPT_OBJVALUE, FREE_OBJLIST);
		cli_freeCommitAwaitingList();
		/* HELP_DELETE_MSG(&gxSetObjMsgHead); */
		gbTransactionMode = false;

	} else if (strncasecmp(pxInData->sCommandName, "cancel", MAX_LEN_COMMAND) == 0) {
		LOGF_LOG_INFO("Cancel & Conclude Transaction Mode\n");
		HELP_DELETE_OBJ(gxSetObjMsgHead.pObjType, SOPT_OBJVALUE, FREE_OBJLIST);
		cli_freeCommitAwaitingList();
		/* HELP_DELETE_MSG(&gxSetObjMsgHead); */
		gbTransactionMode = false;
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_deleteCommandHandler
Description: This function handles DEL commands
***********************************************************************************************************/
int cli_deleteCommandHandler(MsgHeader * pxGetMsgHead, MsgHeader * pxSetMsgHead, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	GetMsgsList_t *pxGetMsgList = NULL;
	{
		LOGF_LOG_DEBUG("Inside DEL Specific operation\n");
		nRet = cli_updateGetObjListBasedOnFilterParam(pxGetMsgHead->pObjType, &(pxInData->xFilterParam));
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Filter parameter\n", nRet);
		if (pxInData->pxLinkObjList != NULL) {
			nRet = cli_updateGetObjListBasedOnLinkObjs(pxGetMsgHead->pObjType, pxInData, false);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Link ObjectList\n", nRet);
		}
		nRet = cli_updateGetObjListBasedOnDepObjs(pxGetMsgHead->pObjType, pxInData->pxDependsList);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Dependant ObjectList\n", nRet);

		nRet = cli_forkGetObjListForEachObject(pxGetMsgHead->pObjType, SOPT_LEAFNODE, pxInData, &pxGetMsgList, false);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while preparing for forking of CAL_GETs\n", nRet);
		nRet = cli_executeConcatenatedCalGet(pxGetMsgList);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while forking of CAL_GETs in DELETE scenario\n", nRet);

		nRet = cli_updateSetObjListForDelBasedOnDepObjs(pxSetMsgHead->pObjType, pxGetMsgList, pxInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating SetObjList in DELETE scenario\n", nRet);
		LOGF_LOG_DEBUG("Printing Objlist before CAL-SET in DEL scenario\n");
		if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
			HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE);
		}
		if (gbTransactionMode == false) {
			nRet = cal_setValue(pxSetMsgHead);
			if (nRet <= UGW_FAILURE) {
				PRINT_TO_CONSOLE("Failure of cal_set in DEL scenario\n");
				if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
					HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE);
				}
				cli_handleErrorResponseCode(pxSetMsgHead->pObjType);
			} else {
				PRINT_TO_CONSOLE("Success of cal_set in DEL scenario\n");
			}
		}
	}
 end:
	cli_freeConcatenatedGetObjList(pxGetMsgList);
	return nRet;

}

/***********************************************************************************************************
Function: cli_addCommandHandler
Description: This function handles ADD commands
***********************************************************************************************************/

int cli_addCommandHandler(MsgHeader * pxGetMsgHead, MsgHeader * pxSetMsgHead, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	GetMsgsList_t *pxGetMsgList = NULL;

	{
		LOGF_LOG_DEBUG("Inside ADD Specific operation\n");
		if ((pxInData->xFilterParam.bFilled == true) && (pxInData->pxArgList != NULL)) {
			nRet = cli_updateGetObjListBasedOnFilterParam(pxGetMsgHead->pObjType, &(pxInData->xFilterParam));
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Filter parameter\n", nRet);
		}
		if (pxInData->pxLinkObjList != NULL) {
			nRet = cli_updateGetObjListBasedOnLinkObjs(pxGetMsgHead->pObjType, pxInData, false);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Link ObjectList\n", nRet);
		}

		nRet = cli_updateGetObjListBasedOnDepObjs(pxGetMsgHead->pObjType, pxInData->pxDependsList);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Dependant ObjectList\n", nRet);

		nRet = cli_forkGetObjListForEachObject(pxGetMsgHead->pObjType, SOPT_LEAFNODE, pxInData, &pxGetMsgList, false);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while preparing for forking of CAL_GETs\n", nRet);
		cli_executeConcatenatedCalGet(pxGetMsgList);
		/* IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while forking of CAL_GETs in ADD scenario\n", nRet); */

		nRet = cli_updateSetObjListForAddBasedOnDepObjs(pxSetMsgHead->pObjType, pxGetMsgList, pxInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating SetObjList in ADD scenario\n", nRet);
		cli_updateSetObjListBasedOnHiddenParam(pxSetMsgHead->pObjType, pxGetMsgList, pxInData);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating SetObjList based on Hidden in ADD scenario\n", nRet);

		LOGF_LOG_DEBUG("Printing Objlist before CAL-SET in ADD scenario\n");
		if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
			HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE);
		}
		if (gbTransactionMode == false) {
			nRet = cal_setValue(pxSetMsgHead);
			if (nRet <= UGW_FAILURE) {
				PRINT_TO_CONSOLE("Failure of cal_set in ADD scenario\n");
				if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
					HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE);
				}
				cli_handleErrorResponseCode(pxSetMsgHead->pObjType);
			} else {
				PRINT_TO_CONSOLE("Success of cal_set in ADD scenario\n");
				/* if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL)
				   HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE); */
				if (pxInData->pxDisplayList != NULL) {
					nRet = cli_showCommandHandler(pxInData, pxSetMsgHead->pObjType);
					IF_ERR_RETURNED_GOTO_END(nRet, "Failed to display the required parameters from CAL-SET successful return data\n");
				}
			}
		}
	}
 end:
	cli_freeConcatenatedGetObjList(pxGetMsgList);
	return nRet;
}

/***********************************************************************************************************
Function: cli_setDisplayCommandHandler
Description: Sets the display mode. Default mode is "list" mode.
***********************************************************************************************************/
int cli_setDisplayCommandHandler(InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	int nCnt = 0;

	if (strcasecmp(pxInData->pArgv->pargv[nCnt]->value, "json") == 0) {
		gnDisplayMode = DISPLAY_MODE_JSON;
	} else if (strcasecmp(pxInData->pArgv->pargv[nCnt]->value, "table") == 0) {
		gnDisplayMode = DISPLAY_MODE_TABLE;
	} else if (strcasecmp(pxInData->pArgv->pargv[nCnt]->value, "list") == 0) {
		gnDisplayMode = DISPLAY_MODE_LIST;
	}
	/* gnDisplayMode = DISPLAY_MODE_LIST;  */
	PRINT_TO_CONSOLE("The Selected Display mode is %s(%d)\n", pxInData->pArgv->pargv[nCnt]->value, gnDisplayMode);

	return nRet;
}

/***********************************************************************************************************
Function: cli_setDebugCommandHandler
Description: This function enables or disables Debug mode. Also sets the level of debug. Default level is 
				Error level.
***********************************************************************************************************/
int cli_setDebugCommandHandler(InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	int nCnt = 0;

	if (strcasecmp(pxInData->pArgv->pargv[nCnt]->value, "on") == 0) {
		gnDebugMode = DEBUG_ON_DEBUG_LEVEL;
		LOGLEVEL = 7;
		LOGTYPE = 3;
	} else if (strcasecmp(pxInData->pArgv->pargv[nCnt]->value, "off") == 0) {
		gnDebugMode = DEBUG_OFF;
		LOGLEVEL = 3;
		LOGTYPE = 1;
	}
	/* TODO : To implement Debug level */
	/*for(nCnt=0;nCnt<pxInData->pArgv->pargc;nCnt++) {
	   LOGF_LOG_DEBUG("Inside cli_setDebugCommandHandler  %s - %s\n", pxInData->pArgv->pargv[nCnt]->param->name, pxInData->pArgv->pargv[nCnt]->value);
	   } */
/* end: */
	return nRet;

}

/***********************************************************************************************************
Function: cli_showTransactionModeCommandHandler
Description: This function displays current transaction mode and also various commands to be committed in 
				explicit trnasaction mode
***********************************************************************************************************/
int cli_showTransactionModeCommandHandler(void)
{
	int nRet = UGW_SUCCESS;
	if (gbTransactionMode == true) {
		PRINT_TO_CONSOLE("You are in ExplicitTransaction Mode\n");
		if (gpxCmdListToCommit != NULL) {
			Command_t *pxCmdTrav = gpxCmdListToCommit;
			PRINT_TO_CONSOLE("Following commands are yet to be committed:\n");
			while (pxCmdTrav != NULL) {
				PRINT_TO_CONSOLE("\t%s\n", pxCmdTrav->sCommandWithArgs);
				pxCmdTrav = pxCmdTrav->pxCmdNext;
			}
		} else {
			PRINT_TO_CONSOLE("Currently no commands to \"commit\"\n");
		}
	} else {
		PRINT_TO_CONSOLE("You are in ImplicitTransaction/SingleCommand Mode\n");
	}
/* end: */
	return nRet;
}

/***********************************************************************************************************
Function: cli_modifyCommandHandler
Description: This function handles Modify/SET requests.
***********************************************************************************************************/
int cli_modifyCommandHandler(MsgHeader * pxGetMsgHead, MsgHeader * pxSetMsgHead, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	bool bFilterApplied = false;
	bool bLinksApplied = false;
	GetMsgsList_t *pxGetMsgList = NULL;
	{
		LOGF_LOG_DEBUG("Inside MODIFY Specific operation\n");
		if ((pxInData->xFilterParam.bFilled == true) && (pxInData->pxArgList != NULL)) {
			nRet = cli_updateGetObjListBasedOnFilterParam(pxGetMsgHead->pObjType, &(pxInData->xFilterParam));
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Filter parameter\n", nRet);
			bFilterApplied = true;
		}
		if (pxInData->pxLinkObjList != NULL) {
			/* nRet = cli_updateGetObjListBasedOnLinkObjs(pxGetMsgHead->pObjType, pxInData, false); */
			nRet = cli_updateGetObjListBasedOnLinkObjs(pxGetMsgHead->pObjType, pxInData, true);
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Link ObjectList\n", nRet);
			bLinksApplied = true;
		}
		if ((bFilterApplied == false) && (bLinksApplied == false)) {
			nRet = cli_updateGetObjListBasedOnArgList(pxGetMsgHead->pObjType, pxInData);	/*  Argument@pxInData->pxArgList is already taken care as part of pxInData->xFilterParam */
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Arguments\n", nRet);
		}
		if (pxInData->pxHiddenParamList != NULL) {
			nRet = cli_updateGetObjListBasedOnHiddenParams(pxGetMsgHead->pObjType, pxInData);	/*  Argument@pxInData->pxArgList is already taken care as part of pxInData->xFilterParam */
			IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating GetObjList based on Arguments\n", nRet);
		}
		nRet = cli_forkGetObjListForEachObject(pxGetMsgHead->pObjType, SOPT_OBJVALUE, pxInData, &pxGetMsgList, false);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while preparing for forking of CAL_GETs\n", nRet);
		nRet = cli_executeConcatenatedCalGet(pxGetMsgList);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while forking of CAL_GETs in MODIFY scenario\n", nRet);

		nRet = cli_updateSetObjListForModifyBasedOnArgs(pxSetMsgHead->pObjType, pxGetMsgList, pxInData, bFilterApplied);
		IF_ERR_RETURNED_GOTO_END(nRet, "Error %d while updating SetObjList in SET scenario\n", nRet);
		LOGF_LOG_DEBUG("Printing Objlist before CAL-SET in MODIFY scenario\n");
		if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
			HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE);
		}
		if (gbTransactionMode == false) {
			nRet = cal_setValue(pxSetMsgHead);
			if (nRet <= UGW_FAILURE) {
				PRINT_TO_CONSOLE("Failure of cal_set in MODIFY scenario\n");
				if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
					HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE);
				}
				cli_handleErrorResponseCode(pxSetMsgHead->pObjType);
			} else {
				PRINT_TO_CONSOLE("Success of cal_set in MODIFY scenario\n");
				/*if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL)
				   HELP_PRINT_OBJ(pxSetMsgHead->pObjType, SOPT_OBJVALUE); */
			}
		}
	}
 end:
	cli_freeConcatenatedGetObjList(pxGetMsgList);
	return nRet;

}

/***********************************************************************************************************
Function: cli_updateGetObjListBasedOnArgList
Description: This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on input Arguments.
***********************************************************************************************************/
int cli_updateGetObjListBasedOnArgList(ObjList * pxGetObjList, InputData_t * pxInData)
{
	ArgParameter_t *pxArgTrav = pxInData->pxArgList;
	ObjList *pxObj = NULL;
	int nDup = 0;
	int nRet = UGW_SUCCESS;
	while (pxArgTrav != NULL) {
		if ((pxInData->xFilterParam.bFilled == true) && (strncmp(pxInData->xFilterParam.sPreAsterisk, pxArgTrav->sObjName, MAX_LEN_OBJNAME) == 0)) {
			/* LOGF_LOG_DEBUG("No need to add Arguments for Filtered Object, as we get the full object\n"); */
		} else {

			nDup = cli_checkObjDuplicationInObjList(pxGetObjList, pxArgTrav->sObjName, &pxObj);
			if (nDup == 0) {
				pxObj = HELP_OBJECT_GET(pxGetObjList, pxArgTrav->sObjName, SOPT_OBJVALUE);
				/* pxObj = HELP_OBJECT_GET(pxGetObjList, pxArgTrav->sObjName, SOPT_LEAFNODE); */
				if (pxObj == NULL) {
					nRet = UGW_FAILURE;
					LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
					goto end;
				}
			}
			/* HELP_PARAM_GET(pxObj, pxArgTrav->sParameter, SOPT_LEAFNODE);  */
		}
		pxArgTrav = pxArgTrav->pxArgNext;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateGetObjListBasedOnHiddenParams
Description: This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on Hidden Parameters.
***********************************************************************************************************/
int cli_updateGetObjListBasedOnHiddenParams(ObjList * pxGetObjList, InputData_t * pxInData)
{
	HiddenParam_t *pxHiddenTrav = pxInData->pxHiddenParamList;
	ObjList *pxObj = NULL;
	int nDup = 0;
	int nRet = UGW_SUCCESS;
	while (pxHiddenTrav != NULL) {
		if ((pxInData->xFilterParam.bFilled == true) && (strncmp(pxInData->xFilterParam.sPreAsterisk, pxHiddenTrav->sLeftObjName, MAX_LEN_OBJNAME) == 0)) {
			/* LOGF_LOG_DEBUG("No need to add Arguments for Filtered Object, as we get the full object\n");  */
		} else {

			nDup = cli_checkObjDuplicationInObjList(pxGetObjList, pxHiddenTrav->sLeftPreAsterisk, &pxObj);
			if (nDup == 0) {
				pxObj = HELP_OBJECT_GET(pxGetObjList, pxHiddenTrav->sLeftPreAsterisk, SOPT_OBJVALUE);
				/* pxObj = HELP_OBJECT_GET(pxGetObjList, pxArgTrav->sObjName, SOPT_LEAFNODE);     */
				if (pxObj == NULL) {
					nRet = UGW_FAILURE;
					LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
					goto end;
				}
			}
			/* HELP_PARAM_GET(pxObj, pxArgTrav->sParameter, SOPT_LEAFNODE);        */
		}
		pxHiddenTrav = pxHiddenTrav->pxHiddenNext;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateSetObjListForModifyBasedOnArgs
Description: This function updates the ObjectList that has to be configured in Database using CAL-SET. Here 
				the update happens based on input Arguments.
***********************************************************************************************************/
int cli_updateSetObjListForModifyBasedOnArgs(ObjList * pxSetObjList, GetMsgsList_t * pxGetMsgList, InputData_t * pxInData, bool bFilterApplied)
{
	/* LinkObject_t *pxLinkTrav = pxInData->pxLinkObjList; */
	/* DepObject_t *pxDepTrav = pxDepObj; */
	ArgParameter_t *pxArgTrav = pxInData->pxArgList;
	int nRet = UGW_SUCCESS;
	char sObject[MAX_LEN_OBJNAME] = { 0 };
	char sDBObject[MAX_LEN_OBJNAME] = { 0 };
	char sPreAsterisk[MAX_LEN_OBJNAME] = { 0 };

	if ((pxInData->xFilterParam.bFilled == true)) {
		strncpy(sObject, pxInData->xFilterParam.sObjName, MAX_LEN_OBJNAME);
		strncpy(sPreAsterisk, pxInData->xFilterParam.sPreAsterisk, MAX_LEN_OBJNAME);
	} else if (pxInData->pxArgList != NULL) {
		strncpy(sObject, pxInData->pxArgList->sObjName, MAX_LEN_OBJNAME);
		strncpy(sPreAsterisk, pxInData->pxArgList->sPreAsterisk, MAX_LEN_OBJNAME);
	} else if (pxInData->pxHiddenParamList != NULL) {
		strncpy(sObject, pxInData->pxHiddenParamList->sLeftObjName, MAX_LEN_OBJNAME);
		strncpy(sPreAsterisk, pxInData->pxHiddenParamList->sLeftPreAsterisk, MAX_LEN_OBJNAME);
	}

	nRet = cli_getInitRefObjectBasedOnFilter(sDBObject, pxGetMsgList, sPreAsterisk, sObject);
	/* if(bMatchFound == false) { */
	if (strlen(sDBObject) == 0) {
		LOGF_LOG_INFO("No Object available with the required filter Obj:%s \n", sPreAsterisk);
		nRet = UGW_FAILURE;
		goto end;
	}

	if (pxInData->pxArgList != NULL) {
		if (bFilterApplied == true) {
			pxArgTrav = pxInData->pxArgList->pxArgNext;	/*  argument@pxArgList was already used to extract sFilteredObj */
		} else {
			pxArgTrav = pxInData->pxArgList;
		}
	}
	while (pxArgTrav != NULL) {
		LOGF_LOG_DEBUG("\t\tObjName being modified is : %s FilteredObj %s\n", pxArgTrav->sObjName, sDBObject);
		LOGF_LOG_DEBUG("\tParam to Set is Object: %s, Parameter %s\n", pxArgTrav->sObjName, pxArgTrav->sParameter);

		if (cli_matchInstancedStrings(pxArgTrav->sPreAsterisk, sDBObject) == 0) {	/* NOT MATCHED     */
			if (pxInData->pxLinkObjList != NULL) {
				ObjList *pxRetrievedObj = NULL;
				ObjList *pxInstObj = NULL;

				nRet = cli_getForwardLinkedObj(sDBObject, pxArgTrav->sPreAsterisk, pxGetMsgList, &pxRetrievedObj, pxInData);
				if(nRet <= UGW_FAILURE) {
					if (pxRetrievedObj != NULL) {
						HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
					}
                                        LOGF_LOG_ERROR("Error in obtaining Forward linking object\n");
                                        goto end;
                                }

				if (pxRetrievedObj == NULL) {	/* Probably that might be in Backward links */
					nRet = cli_getBackwardLinkedObj(sDBObject, pxArgTrav->sPreAsterisk, pxGetMsgList, &pxRetrievedObj, pxInData);
					if(nRet <= UGW_FAILURE) {
						if (pxRetrievedObj != NULL) {
							HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
						}
                                                LOGF_LOG_ERROR("Error in obtaining Backward linking object\n");
                                                goto end;
                                        }
				}
				if (pxRetrievedObj != NULL) {
					LOGF_LOG_DEBUG("Seems some %s Objects found in links\n", pxArgTrav->sPreAsterisk);
					FOR_EACH_OBJ(pxRetrievedObj, pxInstObj) {
						nRet = cli_updateSetObjListBasedOnSpecificArg(pxSetObjList, pxArgTrav, pxInstObj->sObjName, pxGetMsgList);
						if (nRet <= UGW_FAILURE) {
							HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
							goto end;
						}
					}
				}
				if (pxRetrievedObj != NULL) {
					HELP_DELETE_OBJ(pxRetrievedObj, SOPT_OBJVALUE, FREE_OBJLIST);
				}
			}	/* TODO Write Else case to check if the Object hasn't had any links, but different from the filtered/first object */
		} else {
			nRet = cli_updateSetObjListBasedOnSpecificArg(pxSetObjList, pxArgTrav, sDBObject, pxGetMsgList);
			if (nRet <= UGW_FAILURE) {
				goto end;
			}
		}
		pxArgTrav = pxArgTrav->pxArgNext;
	}

	nRet = cli_updateSetObjListBasedOnHiddenParam(pxSetObjList, pxGetMsgList, pxInData);

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateGetObjListBasedOnDisplay
Description: This function updates the list of objects that has to be fetched using CALGET. Here the 
				update happens based on "display" parameters.
***********************************************************************************************************/
int cli_updateGetObjListBasedOnDisplay(ObjList * pxGetObjList, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	DisplayParameter_t *pxDispTrav = pxInData->pxDisplayList;
	ObjList *pxObj = NULL;
	int nDup = 0;

	while (pxDispTrav != NULL) {
		LOGF_LOG_DEBUG("Adding DispObj %s to Get List \n", pxDispTrav->sPreAsterisk);
		nDup = cli_checkObjDuplicationInObjList(pxGetObjList, pxDispTrav->sPreAsterisk, &pxObj);
		if (nDup == 0) {
			pxObj = HELP_OBJECT_GET(pxGetObjList, pxDispTrav->sPreAsterisk, SOPT_LEAFNODE);
			if (pxObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
				goto end;
			}
		}
		/* HELP_PARAM_GET(pxObj, pxDispTrav->sParamName, SOPT_LEAFNODE);  */
		pxDispTrav = pxDispTrav->pxDisplayNext;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_forkGetObjListForEachObject
Description: This function prepares a separate MshHeaderfor for each Object type that has to be 
				fetched using CALGET. Otherwise the failure to obtain Objlist for one Object type, would 
				eventually cause failure for other Object types, though their Object list could be fetched 
				successfully when tried separately.
***********************************************************************************************************/
int cli_forkGetObjListForEachObject(ObjList * pxObjList, int nGetMode, InputData_t * pxInData, GetMsgsList_t ** pxGetMsgList, bool bPostCalSet)
{
	ObjList *pxObj = NULL;
	ObjList *pxNewGetObj = NULL;
	GetMsgsList_t *pxGetMsgTrav = NULL;
	int nRet = UGW_SUCCESS;
	ParamList *pxParam = NULL;

	FOR_EACH_OBJ(pxObjList, pxObj) {
		if (*pxGetMsgList == NULL) {
			/* LOGF_LOG_DEBUG("First node of GetMsgList creation\n"); */
			*pxGetMsgList = HELP_MALLOC(sizeof(GetMsgsList_t));
			if (*pxGetMsgList == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for 1st node of GetMsgList\n");
				goto end;
			}
			pxGetMsgTrav = *pxGetMsgList;
		} else {
			/* LOGF_LOG_DEBUG("Chained node of GetMsgList creation\n"); */
			if (pxGetMsgTrav == NULL) {
				LOGF_LOG_CRITICAL("Unexpected NULL value in pxGetMsgTrav!!\n");
				nRet = UGW_FAILURE;
				goto end;
			}
			pxGetMsgTrav->pxGetMsgNext = HELP_MALLOC(sizeof(GetMsgsList_t));
			if (pxGetMsgTrav->pxGetMsgNext == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to allocated memory for a node of GetMsgList\n");
				goto end;
			}
			pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext;
		}
		strncpy(pxGetMsgTrav->sObjName, pxObj->sObjName, MAX_LEN_OBJNAME);
		HELP_CREATE_MSG(&(pxGetMsgTrav->xGetObjMsgHead), MOPT_GET, nGetMode, OWN_CLI, 1);
		/* LOGF_LOG_DEBUG("MsgHead created in xGetObjMsgHead\n"); */
		if (pxGetMsgTrav->xGetObjMsgHead.pObjType == NULL) {
			LOGF_LOG_CRITICAL("ERROR!! ObjectList is NULL :-( \n");
			nRet = UGW_FAILURE;
			goto end;
		}

		pxNewGetObj = HELP_OBJECT_GET(pxGetMsgTrav->xGetObjMsgHead.pObjType, pxGetMsgTrav->sObjName, nGetMode);
		if (pxNewGetObj == NULL) {
			nRet = UGW_FAILURE;
			LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
			goto end;
		}
		if (bPostCalSet == false) {
			FOR_EACH_PARAM(pxObj, pxParam) {
				if (strlen(pxParam->sParamValue) > 0) {
					HELP_PARAM_SET(pxNewGetObj, pxParam->sParamName, pxParam->sParamValue, nGetMode);
				} else {
					HELP_PARAM_GET(pxNewGetObj, pxParam->sParamName, nGetMode);
				}
			}
		}
		pxGetMsgTrav->nCalGetStatus = UGW_FAILURE;
		pxGetMsgTrav->pxGetMsgNext = NULL;
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_executeConcatenatedCalGet
Description: This function calls CALGET in a loop for various objects.
***********************************************************************************************************/
int cli_executeConcatenatedCalGet(GetMsgsList_t * pxGetMsgList)
{
	GetMsgsList_t *pxGetMsgTrav = pxGetMsgList;
	int nRet = UGW_SUCCESS;
	int nTempRet = UGW_SUCCESS;
	int nSuccessCount = 0;
	while (pxGetMsgTrav) {
		nTempRet = cal_getValue(&(pxGetMsgTrav->xGetObjMsgHead));
		if (nTempRet <= UGW_FAILURE) {
			LOGF_LOG_INFO("Failure of cal_get while getting %s Object\n", pxGetMsgTrav->sObjName);
			pxGetMsgTrav->nCalGetStatus = UGW_FAILURE;
		} else {
			LOGF_LOG_DEBUG("Success of cal_get while getting %s Object\n", pxGetMsgTrav->sObjName);
			pxGetMsgTrav->nCalGetStatus = UGW_SUCCESS;
			if (gnDebugMode == DEBUG_ON_DEBUG_LEVEL) {
				HELP_PRINT_OBJ(pxGetMsgTrav->xGetObjMsgHead.pObjType, SOPT_OBJVALUE);
			}
			nSuccessCount++;
		}
		pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext;
	}
	if (nSuccessCount == 0) {
		LOGF_LOG_INFO("None of the requested Objects available in Database\n");
		nRet = UGW_FAILURE;
	}
	return nRet;
}

/***********************************************************************************************************
Function: cli_freeConcatenatedGetObjList
Description: This function frees various Object lists obtained in CALGET.
***********************************************************************************************************/
int cli_freeConcatenatedGetObjList(GetMsgsList_t * pxGetMsgList)
{

	GetMsgsList_t *pxGetMsgTrav = pxGetMsgList;
	GetMsgsList_t *pxGetMsgTemp = NULL;
	int nRet = UGW_SUCCESS;
	while (pxGetMsgTrav) {
		if (pxGetMsgTrav->xGetObjMsgHead.pObjType != NULL) {
			HELP_DELETE_OBJ(pxGetMsgTrav->xGetObjMsgHead.pObjType, SOPT_OBJVALUE, FREE_OBJLIST);
		}
		pxGetMsgTemp = pxGetMsgTrav->pxGetMsgNext;
		HELP_FREE(pxGetMsgTrav);
		pxGetMsgTrav = pxGetMsgTemp;
	}

	return nRet;
}

/***********************************************************************************************************
Function: cli_specificActionsonDisplayData
Description: This function is used to perform specific action based on subsystem/command type. At the 
				moment this function filter unwanted entries of "show wan interface all". This function is 
				a not clean way to achieve this. The better way would be to make use of "hidden" parameter.
***********************************************************************************************************/
int cli_specificActionsonDisplayData(ArgParameter_t ** pxOutList, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	ArgParameter_t *pxOutRowTrav = *pxOutList;
	ArgParameter_t *pxOutRowTemp = NULL;
	ArgParameter_t *pxOutTrav = NULL;
	ArgParameter_t *pxOutTemp = NULL;
	bool bRowDeleted = false;
	char sStringVal[MAX_DEPTH_COMMAND][MAX_LEN_COMMAND] = { 0 };
	int nPos = 0, nStrPos = 0, nStrNum = 0;

	/* LOGF_LOG_DEBUG("Perform Specific Action on display data for \"%s\" length %d\n", pxInData->sCommandName, strlen(pxInData->sCommandName)); */
	for (nPos = 0; nPos < strlen(pxInData->sCommandName); nPos++) {
		if (pxInData->sCommandName[nPos] == CHAR_SPACE) {
			nStrNum++;
			if (nStrNum >= MAX_DEPTH_COMMAND) {
				LOGF_LOG_ERROR("Number of fields in command %s is more than %d\n", pxInData->sCommandName, MAX_DEPTH_COMMAND);
				nRet = UGW_FAILURE;
				goto end;
			}
			nStrPos = 0;
		} else {
			if (nStrNum >= MAX_DEPTH_COMMAND) {
				LOGF_LOG_ERROR("Number of fields in command %s is more than %d\n", pxInData->sCommandName, MAX_DEPTH_COMMAND);
				nRet = UGW_FAILURE;
				goto end;
			}
			sStringVal[nStrNum][nStrPos] = pxInData->sCommandName[nPos];
			nStrPos++;
		}
	}
	nStrNum++;
	for (nPos = DISPLAY_TAG_OFFSET; nPos < nStrNum; nPos++) {
		if (strlen(sStringVal[nPos]) > 0) {
			char sTempTag[MAX_LEN_COMMAND] = { 0 };
			if (strcmp(sStringVal[nPos], "all") == 0) {
				continue;
			}
			strncpy(sTempTag, pxInData->sDisplayTag, MAX_LEN_COMMAND);
			snprintf(pxInData->sDisplayTag, MAX_LEN_COMMAND, "%s %s", sTempTag, sStringVal[nPos]);
		}
	}
	LOGF_LOG_DEBUG("Display Tag is \"%s\"\n", pxInData->sDisplayTag);
	nPos = 0;
	if ((strcmp(sStringVal[OPERATION_TYPE_OFFSET], "show") == 0) && (strcmp(sStringVal[SERVICE_NAME_OFFSET], "wan") == 0) && (strcmp(sStringVal[DISPLAY_TAG_OFFSET], "interface") == 0)
	    && (strcmp(sStringVal[DISPLAY_TAG_OFFSET + 1], "all") == 0)) {
		int nRow = 0;
		/* LOGF_LOG_DEBUG("Inside WAN command match\n");       */
		while (pxOutRowTrav != NULL) {
			nRow++;
			pxOutTrav = pxOutRowTrav;
			pxOutTemp = NULL;
			while (pxOutTrav != NULL) {
				/* LOGF_LOG_DEBUG("Row %d: Comparing Param Name %s - %s\n", nRow, pxOutTrav->sParameter, "X_LANTIQ_COM_Description"); */
				if (strstr(pxOutTrav->sParameter, "X_LANTIQ_COM_Description") != NULL) {
					char sArgumentVal[MAX_LEN_PARAM_VALUE] = { 0 };
					char sStringValue[MAX_DEPTH_COMMAND][MAX_LEN_COMMAND];
					strncpy(sArgumentVal, pxOutTrav->sArgValue, MAX_LEN_PARAM_VALUE - 1);
					LOGF_LOG_DEBUG("Row %d: Parameter Matched\n", nRow);

					nPos = 0;
					nStrNum = 0;
					nStrPos = 0;
					for (nPos = 0; nPos < strlen(sArgumentVal); nPos++) {
						/* LOGF_LOG_DEBUG("%d-%c, ", pxInData->sCommandName[nPos], pxInData->sCommandName[nPos]); */
						if (sArgumentVal[nPos] == CHAR_SPACE) {
							nStrNum++;
							if (nStrNum >= MAX_DEPTH_COMMAND) {
								LOGF_LOG_ERROR("Number of fields in argument value %s is more than %d\n", pxInData->sCommandName, MAX_DEPTH_COMMAND);
								nRet = UGW_FAILURE;
								goto end;
							}
							nStrPos = 0;
						} else {
							if (nStrNum >= MAX_DEPTH_COMMAND) {
								LOGF_LOG_ERROR("Number of fields in argument value %s is more than %d\n", pxInData->sCommandName, MAX_DEPTH_COMMAND);
								nRet = UGW_FAILURE;
								goto end;
							}
							sStringValue[nStrNum][nStrPos] = sArgumentVal[nPos];
							nStrPos++;
						}
					}
					nStrNum++;
					/* LOGF_LOG_DEBUG("The 2nd string is %s\n", sStringValue[1]); */
					if ((strstr(sStringValue[1], "WAN") == NULL)) {

						/* LOGF_LOG_DEBUG("The entry DOESN'T corresponds to WAN. Filter it out\n"); */
						if (pxOutRowTrav == *pxOutList) {
							*pxOutList = pxOutRowTrav->pxArgNextRow;
							pxOutTrav = pxOutRowTrav;
							while (pxOutTrav != NULL) {
								pxOutTemp = pxOutTrav->pxArgNext;
								HELP_FREE(pxOutTrav);
								pxOutTrav = pxOutTemp;
							}
							/* LOGF_LOG_DEBUG("Head Row case\n"); */
							pxOutRowTrav = *pxOutList;
						} else {
							pxOutRowTemp->pxArgNextRow = pxOutRowTrav->pxArgNextRow;
							pxOutTrav = pxOutRowTrav;
							while (pxOutTrav != NULL) {
								pxOutTemp = pxOutTrav->pxArgNext;
								HELP_FREE(pxOutTrav);
								pxOutTrav = pxOutTemp;
							}
							/* LOGF_LOG_DEBUG("Intermediate Row case\n"); */
							pxOutRowTrav = pxOutRowTemp->pxArgNextRow;
						}
						bRowDeleted = true;
						break;
					} else {
						LOGF_LOG_DEBUG("The entry corresponds to WAN. No need to skip\n");
					}

				} else {
					/* LOGF_LOG_DEBUG("Row %d: Parameters not matched %s - %s\n", nRow, pxOutTrav->sParameter, "X_LANTIQ_COM_Description");  */
				}
				pxOutTemp = pxOutTrav;
				pxOutTrav = pxOutTrav->pxArgNext;
			}
			if (bRowDeleted == true) {
				bRowDeleted = false;
				continue;
			}
			pxOutRowTemp = pxOutRowTrav;
			pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
		}
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_getBackwardLinkedObj
Description: This function searches the received Object from CALGET to find a linked object instance. 
				It traverses in the lists in backward direction (bottom-to-top direction) starting from 
				Initial Reference(filter criterion) Object based on "links".
***********************************************************************************************************/
int cli_getBackwardLinkedObj(char *pcInitRefObj, char *pcExpectObj, GetMsgsList_t * pxGetMsgList, ObjList ** pxRetObj, InputData_t * pxInData)
{
	ObjList *pxObj = NULL;
	LinkObject_t *pxLinkTrav = NULL;
	char sRefObj[MAX_LEN_OBJNAME] = { 0 };
	/* char sExpectedObj[MAX_LEN_OBJNAME] = { 0 }; */
	bool bInitRefFound = false;
	ParamList *pxParam = NULL;
	GetMsgsList_t *pxGetMsgTrav = NULL;
	int nRet = UGW_SUCCESS;

	strncpy(sRefObj, pcInitRefObj, MAX_LEN_OBJNAME);
	LOGF_LOG_DEBUG("Backward Traversing in Links, Use %s to find %s\n", pcInitRefObj, pcExpectObj);
	pxLinkTrav = pxInData->pxLinkObjListTail;
	while (pxLinkTrav != NULL) {
		LOGF_LOG_DEBUG("The Outer loop Linking %s.%s points to Linked %s\n", pxLinkTrav->sLinkingObj, pxLinkTrav->sLinkingObjParam, pxLinkTrav->sLinkedObj);
		if (cli_matchInstancedStrings(pxLinkTrav->sLinkedObj, pcInitRefObj)) {
			LinkObject_t *pxBackTrav = pxLinkTrav;
			ObjList *pxGetObjList = NULL;

			bInitRefFound = true;
			while (pxBackTrav != NULL) {
				LOGF_LOG_DEBUG("The Inner Loop: Linking %s.%s points to Linked %s\n", pxBackTrav->sLinkingObj, pxBackTrav->sLinkingObjParam, pxBackTrav->sLinkedObj);
				pxGetObjList = NULL;
				for (pxGetMsgTrav = pxGetMsgList; pxGetMsgTrav != NULL; pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext) {
					if (pxGetMsgTrav->nCalGetStatus <= UGW_FAILURE) {
						continue;
					}
					/* LOGF_LOG_DEBUG("Check %s against %s\n", pxGetMsgTrav->sObjName, pxBackTrav->sLinkingObjPreAsterisk); */
					/* if(cli_matchInstancedStrings(pxGetMsgTrav->sObjName, pxBackTrav->sLinkingObjPreAsterisk) == 0) { */
					if (cli_matchInstancedStrings(pxBackTrav->sLinkingObjPreAsterisk, pxGetMsgTrav->sObjName) == 0) {
						continue;
					}
					pxGetObjList = pxGetMsgTrav->xGetObjMsgHead.pObjType;
					break;
				}
				if (pxGetObjList == NULL) {
					LOGF_LOG_DEBUG("Unable to get pointer to object or Object not available in DB of type %s\n", pxBackTrav->sLinkingObjPreAsterisk);
				}
				/* LOGF_LOG_DEBUG("Check expected %s against %s\n", pcExpectObj, pxBackTrav->sLinkingObjPreAsterisk); */
				if (cli_matchInstancedStrings(pxBackTrav->sLinkingObjPreAsterisk, pcExpectObj) && (pxGetObjList != NULL)) {
					FOR_EACH_OBJ(pxGetObjList, pxObj) {
						FOR_EACH_PARAM(pxObj, pxParam) {
							/* LOGF_LOG_DEBUG("\t\t\t\tFetching parameter if %s - %s matches\n",pxParam->sParamName,pxBackTrav->sLinkingObjParam); */
							if (strncmp(pxParam->sParamName, pxBackTrav->sLinkingObjParam, MAX_LEN_PARAM_NAME) == 0) {
								char sLinkedObjDotted[MAX_LEN_OBJNAME] = { 0 };
								if (pxParam->sParamValue[(strlen(pxParam->sParamValue) - 1)] == CHAR_DOT) {
									strncpy(sLinkedObjDotted, pxParam->sParamValue, MAX_LEN_OBJNAME);
								} else {
									snprintf(sLinkedObjDotted, MAX_LEN_OBJNAME, "%s.", pxParam->sParamValue);
								}

								LOGF_LOG_DEBUG("\t\t\t\tPARAM Name Matched; Now Check Values %s - %s\n", sLinkedObjDotted, sRefObj);
								if (strncmp(sLinkedObjDotted, sRefObj, MAX_LEN_PARAM_VALUE) == 0) {
									ObjList *pxTempObj = NULL;
									LOGF_LOG_DEBUG("\t\t\t\t%s.%s with Value %s Matched %s\n", pxObj->sObjName, pxParam->sParamName, sLinkedObjDotted, sRefObj);
									if (*pxRetObj == NULL) {
										*pxRetObj = HELP_CREATE_OBJ(SOPT_OBJVALUE);
										if (*pxRetObj == NULL) {
											LOGF_LOG_CRITICAL("malloc failed to allocate Object List\n");
											/* nRet = ERR_MEMORY_ALLOC_FAILED; */
											nRet = UGW_FAILURE;
											goto end;
										}
									}
									pxTempObj = HELP_OBJECT_SET(*pxRetObj, pxObj->sObjName, NO_ARG_VALUE, OBJOPT_MODIFY, SOPT_OBJVALUE);
									if (pxTempObj == NULL) {
										nRet = UGW_FAILURE;
										LOGF_LOG_CRITICAL("Unable to append an Object to RetrievedObjList in Backward Links scenario\n");
										goto end;
									}
								}
								break;
							}
						}
					}

				} else {
					/* Might be in a hierarchical link :-)
					   TODO Check if these objects have Reference Value, then change RefValue & Call this function recursively */
				}
				pxBackTrav = pxBackTrav->pxLinkPrev;
			}
			break;
		}
		pxLinkTrav = pxLinkTrav->pxLinkPrev;
	}
	if (bInitRefFound == false) {
		LOGF_LOG_INFO("The Initial Reference Obj itself is not found in (Backward) Links List\"%s\"\n", pcInitRefObj);
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_getForwardLinkedObj
Description: This function searches the received Object from CALGET to find a linked object instance. 
				It traverses in the lists in forward direction (top-to-bottom direction) starting from 
				Initial Reference(filter criterion) Object based on "links".
***********************************************************************************************************/
int cli_getForwardLinkedObj(char *pcInitRefObj, char *pcExpectObj, GetMsgsList_t * pxGetMsgList, ObjList ** pxRetObj, InputData_t * pxInData)
{
	LinkObject_t *pxLinkTrav = pxInData->pxLinkObjList;
	bool bLinkFound = false;
	char sDbObjToCheck[MAX_LEN_OBJNAME] = { 0 };
	bool bObjFound = false;
	int nRet = UGW_SUCCESS;

	strncpy(sDbObjToCheck, pcInitRefObj, MAX_LEN_OBJNAME);
	while (pxLinkTrav != NULL) {
		LOGF_LOG_DEBUG("\t\tLinkingObjName %s being compared in FilteredObj %s\n", pxLinkTrav->sLinkingObj, sDbObjToCheck);
		if (cli_matchInstancedStrings(pxLinkTrav->sLinkingObj, sDbObjToCheck)) {
			ObjList *pxTempObj = NULL;
			char sTempObjName[MAX_LEN_OBJNAME] = { 0 };
			ObjList *pxGetObjList = NULL;
			GetMsgsList_t *pxGetMsgTrav = pxGetMsgList;
			LOGF_LOG_DEBUG("\t\tLinkingObjName %s MATCHED with FilteredObj %s\n", pxLinkTrav->sLinkingObj, sDbObjToCheck);
			/* bParamFound = false; */
			for (pxGetMsgTrav = pxGetMsgList; pxGetMsgTrav != NULL; pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext) {
				if (pxGetMsgTrav->nCalGetStatus <= UGW_FAILURE) {
					continue;
				}
				memset(sTempObjName, 0, MAX_LEN_OBJNAME);
				snprintf(sTempObjName, MAX_LEN_OBJNAME, "%s*.", pxGetMsgTrav->sObjName);
				LOGF_LOG_DEBUG("Check %s against %s\n", sTempObjName, sDbObjToCheck);
				if (cli_matchInstancedStrings(sTempObjName, sDbObjToCheck) == 0) {
					memset(sTempObjName, 0, MAX_LEN_OBJNAME);
					strncpy(sTempObjName, pxGetMsgTrav->sObjName, MAX_LEN_OBJNAME);
					/* LOGF_LOG_DEBUG("Check %s against %s\n", sTempObjName, sDbObjToCheck); */
					if (cli_matchInstancedStrings(sTempObjName, sDbObjToCheck) == 0) {
						continue;
					}
				}
				pxGetObjList = pxGetMsgTrav->xGetObjMsgHead.pObjType;

				bLinkFound = false;
				FOR_EACH_OBJ(pxGetObjList, pxTempObj) {
					memset(sTempObjName, 0, MAX_LEN_OBJNAME);
					if (pxTempObj->sObjName[(strlen(pxTempObj->sObjName) - 1)] == CHAR_DOT) {
						strncpy(sTempObjName, pxTempObj->sObjName, MAX_LEN_OBJNAME);
					} else {
						snprintf(sTempObjName, MAX_LEN_OBJNAME, "%s.", pxTempObj->sObjName);
					}
					/* LOGF_LOG_DEBUG("TempObj %s - DbObj2Chk %s\n", sTempObjName, sDbObjToCheck); */
					if (strncmp(sTempObjName, sDbObjToCheck, MAX_LEN_OBJNAME) == 0) {
						ParamList *pxLinkParam = NULL;
						LOGF_LOG_DEBUG("Got an Object with %s, Now compare if %s matches\n", sDbObjToCheck, pcExpectObj);
						if (cli_matchInstancedStrings(pcExpectObj, sDbObjToCheck)) {
							ObjList *pxNewObj = NULL;
							if (*pxRetObj == NULL) {
								*pxRetObj = HELP_CREATE_OBJ(SOPT_OBJVALUE);
								if (*pxRetObj == NULL) {
									LOGF_LOG_CRITICAL("malloc failed to allocate Object List\n");
									/* nRet = ERR_MEMORY_ALLOC_FAILED; */
									nRet = UGW_FAILURE;
									goto end;
								}
							}
							pxNewObj = HELP_OBJECT_SET(*pxRetObj, pxTempObj->sObjName, NO_ARG_VALUE, OBJOPT_MODIFY, SOPT_OBJVALUE);
							if (pxNewObj == NULL) {
								nRet = UGW_FAILURE;
								LOGF_LOG_CRITICAL("Unable to append an Object to RetrievedObjList in Forward Links scenario\n");
								goto end;
							}
							/* LOGF_LOG_DEBUG("Got a Link Object which matches with Display parameter \n"); */
							/* pxRefObj = pxTempObj; */
							bObjFound = true;
							break;
						}
						FOR_EACH_PARAM(pxTempObj, pxLinkParam) {
							/* LOGF_LOG_DEBUG("\t\t\t\tFetching parameter if %s(%s) - %s matches\n",pxLinkParam->sParamName,pxLinkParam->sParamValue,pxLinkTrav->sLinkingObjParam); */
							if (strncmp(pxLinkParam->sParamName, pxLinkTrav->sLinkingObjParam, MAX_LEN_PARAM_NAME) == 0) {
								LOGF_LOG_DEBUG("\t\tGot link %s%s > %s ;  Traverse further \n", sDbObjToCheck, pxLinkTrav->sLinkingObjParam, pxLinkParam->sParamValue);
								strncpy(sDbObjToCheck, pxLinkParam->sParamValue, MAX_LEN_PARAM_VALUE);
								bLinkFound = true;
								break;
							}
						}
					}
					if (bLinkFound == true) {
						pxGetMsgTrav = pxGetMsgList;
						break;
					}
				}
				if (bObjFound == true) {
					LOGF_LOG_DEBUG("Object Found; No need to traverse further in Links loop \n");
					break;
				}
			}
		}
		if (bObjFound == true) {
			LOGF_LOG_DEBUG("Object Found; No need to traverse further in Links loop \n");
			break;
		}
		pxLinkTrav = pxLinkTrav->pxLinkNext;
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_getInitRefObjectBasedOnFilter
Description: This function searches received object from CALGET & extracts the object Instance based 
			on the filter.
***********************************************************************************************************/
int cli_getInitRefObjectBasedOnFilter(char *pcInitRefObj, GetMsgsList_t * pxGetMsgList, char *pcPreAsterisk, char *pcObjName)
{
	int nRet = UGW_SUCCESS;
	GetMsgsList_t *pxGetMsgTrav = pxGetMsgList;
	char sDBObject[MAX_LEN_OBJNAME] = { 0 };
	ObjList *pxObj = NULL;
	ObjList *pxGetObjList = NULL;

	for (pxGetMsgTrav = pxGetMsgList; pxGetMsgTrav != NULL; pxGetMsgTrav = pxGetMsgTrav->pxGetMsgNext) {
		if (pxGetMsgTrav->nCalGetStatus <= UGW_FAILURE) {
			continue;
		}
		/* LOGF_LOG_DEBUG("Check %s against %s\n", pxGetMsgTrav->sObjName, pcPreAsterisk); */
		if (cli_matchInstancedStrings(pxGetMsgTrav->sObjName, pcPreAsterisk) == 0) {
			continue;
		}
		pxGetObjList = pxGetMsgTrav->xGetObjMsgHead.pObjType;
		break;
	}
	if (pxGetObjList == NULL) {
		LOGF_LOG_ERROR("No Initial Reference Object %s Found in DB\n", pcPreAsterisk);
		nRet = UGW_FAILURE;
		goto end;
	}
	FOR_EACH_OBJ(pxGetObjList, pxObj) {
		if (pxObj->sObjName[(strlen(pxObj->sObjName) - 1)] == CHAR_DOT) {
			strncpy(sDBObject, pxObj->sObjName, MAX_LEN_OBJNAME);
		} else {
			snprintf(sDBObject, MAX_LEN_OBJNAME, "%s.", pxObj->sObjName);
		}
		/* snprintf(sDBObject, MAX_LEN_OBJNAME, "%s.", pxObj->sObjName); */
		LOGF_LOG_DEBUG("ObjName came in GetList is : %s\n", sDBObject);
		if (cli_matchInstancedStrings(pcObjName, sDBObject)) {
			strncpy(pcInitRefObj, sDBObject, MAX_LEN_OBJNAME);
			break;
		}
	}
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_updateSetObjListBasedOnSpecificArg
Description: This function sets the ObjList for CAL-SET based on a specific Argument.
***********************************************************************************************************/
int cli_updateSetObjListBasedOnSpecificArg(ObjList * pxSetObjList, ArgParameter_t * pxArgTrav, char *pcRefObjName, GetMsgsList_t * pxGetMsgList)
{
	GetMsgsList_t *pxGetMsgTempTrav = pxGetMsgList;
	ObjList *pxGetObjListTemp = NULL;
	ObjList *pxTempObj = NULL;
	int nRet = UGW_SUCCESS;
	int nDup = 0;
	bool bObjFound = false;
	ObjList *pxModObj = NULL;
	char sTempObjName[MAX_LEN_OBJNAME] = { 0 };

	LOGF_LOG_DEBUG("One of the found Object is %s\n", pcRefObjName);
	if (strlen(pxArgTrav->sPostAsterisk) > 0) {
		for (pxGetMsgTempTrav = pxGetMsgList; pxGetMsgTempTrav != NULL; pxGetMsgTempTrav = pxGetMsgTempTrav->pxGetMsgNext) {
			if (pxGetMsgTempTrav->nCalGetStatus <= UGW_FAILURE) {
				continue;
			}
			if (cli_matchInstancedStrings(pxGetMsgTempTrav->sObjName, pxArgTrav->sPreAsterisk) == 0) {
				continue;
			}
			pxGetObjListTemp = pxGetMsgTempTrav->xGetObjMsgHead.pObjType;
			break;
		}
		if (pxGetObjListTemp == NULL) {
			LOGF_LOG_ERROR("No Corresponding Objects %s Found in DB\n", pxArgTrav->sPreAsterisk);
			nRet = UGW_FAILURE;
			goto end;
		}
		/* LOGF_LOG_DEBUG("Before FOR loop to get ObjPtr matching to %s\n", pcRefObjName); */
		FOR_EACH_OBJ(pxGetObjListTemp, pxTempObj) {
			memset(sTempObjName, 0, MAX_LEN_OBJNAME);
			if (pxTempObj->sObjName[(strlen(pxTempObj->sObjName) - 1)] == CHAR_DOT) {
				strncpy(sTempObjName, pxTempObj->sObjName, MAX_LEN_OBJNAME);
			} else {
				snprintf(sTempObjName, MAX_LEN_OBJNAME, "%s.", pxTempObj->sObjName);
			}
			/* LOGF_LOG_DEBUG("TempObj %s - DbObj2Chk %s\n", sTempObjName, pxArgTrav->sObjName); */
			if (cli_matchInstancedStrings(pxArgTrav->sObjName, sTempObjName) && strstr(sTempObjName, pcRefObjName)) {
				LOGF_LOG_DEBUG("Old RefObj %s, New RefObj %s\n", pcRefObjName, sTempObjName);
				/* pxRefObj = pxTempObj; */
				bObjFound = true;
				break;
			}
		}
	} else {
		strncpy(sTempObjName, pcRefObjName, MAX_LEN_OBJNAME - 1);
		bObjFound = true;
	}
	if (bObjFound == true) {
		nDup = cli_checkObjDuplicationInObjList(pxSetObjList, sTempObjName, &pxModObj);
		/* LOGF_LOG_DEBUG("\t\tnDup is %d\n", nDup); */
		if (nDup == 0) {
			pxModObj = HELP_OBJECT_SET(pxSetObjList, sTempObjName, NO_ARG_VALUE, OBJOPT_MODIFY, SOPT_OBJVALUE);
			if (pxModObj == NULL) {
				nRet = UGW_FAILURE;
				LOGF_LOG_CRITICAL("Unable to append an Object to SetObjList in MODIFY scenario\n");
				goto end;
			}
		}
		HELP_PARAM_SET(pxModObj, pxArgTrav->sParameter, pxArgTrav->sArgValue, SOPT_LEAFNODE);
	}

 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_getSpecificObjectPointer
Description: This function searches an ObjectName within the CAL-GET received lists and return the  
			corresponding ObjList pointer.
***********************************************************************************************************/
ObjList *cli_getSpecificObjectPointer(char *pcObjName, char *pcObjPreAsterisk, GetMsgsList_t * pxGetMsgList)
{

	GetMsgsList_t *pxGetMsgTempTrav = NULL;
	ObjList *pxGetObjListTemp = NULL;
	ObjList *pxTempObj = NULL;
	ObjList *pxRefObj = NULL;
	char sTempObjName[MAX_LEN_OBJNAME] = { 0 };

	for (pxGetMsgTempTrav = pxGetMsgList; pxGetMsgTempTrav != NULL; pxGetMsgTempTrav = pxGetMsgTempTrav->pxGetMsgNext) {
		if (pxGetMsgTempTrav->nCalGetStatus <= UGW_FAILURE) {
			continue;
		}

		if (cli_matchInstancedStrings(pxGetMsgTempTrav->sObjName, pcObjPreAsterisk) == 0) {
			continue;
		}
		pxGetObjListTemp = pxGetMsgTempTrav->xGetObjMsgHead.pObjType;
		break;
	}
	if (pxGetObjListTemp == NULL) {
		LOGF_LOG_ERROR("No Corresponding Objects %s Found in DB\n", pcObjPreAsterisk);
		goto end;
	}

	FOR_EACH_OBJ(pxGetObjListTemp, pxTempObj) {
		memset(sTempObjName, 0, MAX_LEN_OBJNAME);
		if (pxTempObj->sObjName[(strlen(pxTempObj->sObjName) - 1)] == CHAR_DOT) {
			strncpy(sTempObjName, pxTempObj->sObjName, MAX_LEN_OBJNAME);
		} else {
			snprintf(sTempObjName, MAX_LEN_OBJNAME, "%s.", pxTempObj->sObjName);
		}
		/* LOGF_LOG_DEBUG("TempObj %s - DbObj2Chk %s\n", sTempObjName, pcObjName); */
		if (strncmp(sTempObjName, pcObjName, MAX_LEN_OBJNAME) == 0) {
			pxRefObj = pxTempObj;
			break;
		}
	}
 end:
	return pxRefObj;
}

/***********************************************************************************************************
Function: cli_appendCommandToCommitAwaitingList
Description: This function appends the current command to the list of Commands awaiting "Commit" in 
			explicit transaction mode.
***********************************************************************************************************/
int cli_appendCommandToCommitAwaitingList(InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	Command_t *pxCmdTrav = gpxCmdListToCommit;
	Command_t *pxCmdTemp = NULL;

	while (pxCmdTrav != NULL) {
		pxCmdTemp = pxCmdTrav;
		pxCmdTrav = pxCmdTrav->pxCmdNext;
	}
	if (gpxCmdListToCommit == NULL) {
		gpxCmdListToCommit = HELP_MALLOC(sizeof(Command_t));
		pxCmdTrav = gpxCmdListToCommit;
	} else {
		pxCmdTemp->pxCmdNext = HELP_MALLOC(sizeof(Command_t));
		pxCmdTrav = pxCmdTemp->pxCmdNext;
	}
	if (pxCmdTrav != NULL)
		snprintf(pxCmdTrav->sCommandWithArgs, (MAX_LEN_COMMAND + MAX_LEN_ARG_LIST), "%s %s", pxInData->sCommandName, pxInData->sCmdLineArgs);
	else
		nRet = UGW_FAILURE;

	return nRet;
}

/***********************************************************************************************************
Function: cli_freeCommitAwaitingList
Description: This function frees the list of Commands awaiting "Commit" in explicit transaction mode.
***********************************************************************************************************/
int cli_freeCommitAwaitingList(void)
{
	int nRet = UGW_SUCCESS;
	Command_t *pxCmdTrav = gpxCmdListToCommit;
	Command_t *pxCmdTemp = NULL;

	while (pxCmdTrav != NULL) {
		pxCmdTemp = pxCmdTrav->pxCmdNext;
		HELP_FREE(pxCmdTrav);
		pxCmdTrav = pxCmdTemp;
	}
	gpxCmdListToCommit = NULL;

	return nRet;
}

/***********************************************************************************************************
Function: cli_ignoreSpacesInStrings
Description: This function ignores spaces/tabs/slashes/CRs in Strings and returns a resultant string
***********************************************************************************************************/
int cli_ignoreSpacesInStrings(char **pcResult, char *pcInput)
{
	int nRet = UGW_SUCCESS;
	int nCnt = 0;
	int nCopyCnt = 0;
	char *pcCopier = NULL;

	*pcResult = HELP_MALLOC(strlen(pcInput));
	if (*pcResult == NULL) {
		nRet = UGW_FAILURE;
		goto end;
	}
	pcCopier = *pcResult;
	memset(pcCopier, 0, strlen(pcInput));
	/* LOGF_LOG_DEBUG("Input String \"%s\" of length %d\n", pcInput, strlen(pcInput) ); */
	for (nCnt = 0; nCnt < strlen(pcInput); nCnt++) {
		if ((pcInput[nCnt] == CHAR_SPACE) || (pcInput[nCnt] == CHAR_TAB) || (pcInput[nCnt] == CHAR_BACKSLASH) || (pcInput[nCnt] == CHAR_ENTER)) {
			continue;
		}
		pcCopier[nCopyCnt] = pcInput[nCnt];
		nCopyCnt++;
	}
	/* LOGF_LOG_DEBUG("Output String \"%s\" of length %d\n", *pcResult, strlen(*pcResult) ); */
 end:
	return nRet;
}

/***********************************************************************************************************
Function: cli_printInTabularForm
Description: This function displays parameters in tabular form. Upto 6 parameters, each of 20 char length are 
			shown cleanly.
***********************************************************************************************************/
int cli_printInTabularForm(ArgParameter_t * pxOutList, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	ArgParameter_t *pxOutRowTrav = pxOutList;
	ArgParameter_t *pxOutTrav = NULL;
	char sTitleName[MAX_LEN_PARAM_NAME] = { 0 };

	PRINT_TO_CONSOLE("\n========================================================================================================================\n");
	pxOutTrav = pxOutList;
	while (pxOutTrav != NULL) {
		nRet = cli_getUserDefinedParamName(pxInData->pxCurrentService, pxOutTrav->sParameter, sTitleName);
		PRINT_TO_CONSOLE("%-20.19s", sTitleName);
		pxOutTrav = pxOutTrav->pxArgNext;
	}
	PRINT_TO_CONSOLE("\n========================================================================================================================\n");

	pxOutRowTrav = pxOutList;
	while (pxOutRowTrav != NULL) {
		pxOutTrav = pxOutRowTrav;
		while (pxOutTrav != NULL) {
			PRINT_TO_CONSOLE("%-20.19s", pxOutTrav->sArgValue);
			pxOutTrav = pxOutTrav->pxArgNext;
		}
		PRINT_TO_CONSOLE("\n");
		pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
	}
	PRINT_TO_CONSOLE("\n========================================================================================================================\n");

	return nRet;
}

/***********************************************************************************************************
Function: cli_handleErrorResponseCode
Description: This function handles error response codes and displays error information to user
***********************************************************************************************************/
int cli_handleErrorResponseCode(ObjList * pxObjList)
{
	int nRet = UGW_SUCCESS;
	ObjList *pxErrObj = NULL;
	ParamList *pxParam = NULL;
	char sErrString[MAX_LEN_PARAM_VALUE] = { 0 };
	int nErrCode = 0;
	FOR_EACH_OBJ(pxObjList, pxErrObj) {
		FOR_EACH_PARAM(pxErrObj, pxParam) {
			PRINT_TO_CONSOLE("Object: \"%s\", Parameter: \"%s\" \n", pxErrObj->sObjName, GET_PARAM_NAME(pxParam));
			nErrCode = GET_PARAM_ID(pxParam);
			memset(sErrString, 0, MAX_LEN_PARAM_VALUE);
			strncpy(sErrString, GET_PARAM_VALUE(pxParam), MAX_LEN_PARAM_VALUE - 1);
			if (strlen(sErrString) == 0) {
				int nLoop = 0;
				for (; nLoop < MAX_ERROR_CODES; nLoop++) {
					if (ErrorCodeArray[nLoop].nErrorCode == nErrCode) {
						strncpy(sErrString, ErrorCodeArray[nLoop].sDefaultErrorString, MAX_LEN_PARAM_VALUE);
						break;
					}
				}
			}
			if (strlen(sErrString) > 0) {
				PRINT_TO_CONSOLE("Error Code %d: %s\n", nErrCode, sErrString);
			} else {
				PRINT_TO_CONSOLE("Error Code %d: Unable to retrieve more information about this error; Enable debug mode\n", nErrCode);
			}
		}
	}

	return nRet;
}

/***********************************************************************************************************
Function: cli_printInJsonForm
Description: This function displays parameters in JSON form.
***********************************************************************************************************/
int cli_printInJsonForm(ArgParameter_t * pxOutList, InputData_t * pxInData)
{
	int nRet = UGW_SUCCESS;
	ArgParameter_t *pxOutRowTrav = pxOutList;
	ArgParameter_t *pxOutTrav = NULL;
	int nCnt = 0;
	int nNumberOfRows = 0;
	ObjList *pxMainObjList = NULL;
	ObjList *pxObj = NULL;
	char sObjectName[MAX_LEN_OBJNAME] = { 0 };
	char sParamName[MAX_LEN_PARAM_NAME] = { 0 };
	char sObjectPreAst[MAX_LEN_OBJNAME] = { 0 };
	char sObjectPostAst[MAX_LEN_OBJNAME] = { 0 };
	int nDup = 0;

	while (pxOutRowTrav != NULL) {
		nNumberOfRows++;
		pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
	}
	pxOutRowTrav = pxOutList;
	while (pxOutRowTrav != NULL) {
		nCnt++;
		if (nNumberOfRows > 1) {
			PRINT_TO_CONSOLE("%s %d:\n", pxInData->sDisplayTag, nCnt);
		} else {
			PRINT_TO_CONSOLE("%s :\n", pxInData->sDisplayTag);
		}
		pxOutTrav = pxOutRowTrav;
		while (pxOutTrav != NULL) {
			memset(sObjectName, 0, MAX_LEN_OBJNAME);
			memset(sParamName, 0, MAX_LEN_PARAM_NAME);
			memset(sObjectPreAst, 0, MAX_LEN_OBJNAME);
			memset(sObjectPostAst, 0, MAX_LEN_OBJNAME);
			nRet = cli_splitTR181String(pxOutTrav->sParameter, sParamName, sObjectName, sObjectPreAst, sObjectPostAst);
			if (pxMainObjList == NULL) {
				pxMainObjList = HELP_CREATE_OBJ(SOPT_OBJVALUE);
				if (pxMainObjList == NULL) {
					LOGF_LOG_CRITICAL("malloc failed to allocate Object List\n");
					nRet = UGW_FAILURE;
					goto end;
				}
			}
			pxObj = NULL;
			nDup = cli_checkObjDuplicationInObjList(pxMainObjList, sObjectName, &pxObj);
			if (nDup == 0) {
				/* LOGF_LOG_DEBUG("Adding %s Object to JSON Print List\n", sObjectName); */
				pxObj = HELP_OBJECT_GET(pxMainObjList, sObjectName, SOPT_LEAFNODE);
				if (pxObj == NULL) {
					nRet = UGW_FAILURE;
					LOGF_LOG_CRITICAL("Unable to append an Object to GetObjList \n");
					goto end;
				}
			}
			/* LOGF_LOG_DEBUG("Adding %s Parameter to JSON Print List\n", sParamName); */
			HELP_PARAM_SET(pxObj, sParamName, pxOutTrav->sArgValue, SOPT_OBJVALUE);
			pxOutTrav = pxOutTrav->pxArgNext;
		}
		HELP_PRINT_OBJ(pxMainObjList, SOPT_OBJVALUE);
		HELP_DELETE_OBJ(pxMainObjList, SOPT_OBJVALUE, EMPTY_OBJLIST);
		pxOutRowTrav = pxOutRowTrav->pxArgNextRow;
	}
 end:
	if (pxMainObjList != NULL) {
		HELP_DELETE_OBJ(pxMainObjList, SOPT_OBJVALUE, FREE_OBJLIST);
	}
	return nRet;
}
