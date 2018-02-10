/********************************************************************************
 
        Copyright (c) 2012, 2014, 2015
        Lantiq Beteiligungs-GmbH & Co. KG
        For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/
/** \addtogroup FAPI_LTE */
/* @{ */



#ifndef WWANFAPI_AT_INCLUDES
#define WWANFAPI_AT_INCLUDES

#define WWANFAPI_FAILURE 1                                       /*!< Macro signifying operation failure */
#define WWANFAPI_SUCCESS 0                                       /*!< Macro signifying operation success */


#define WWANFAPIAT_BYTEINTERVAL 20000

#define WWANFAPIAT_READBACK_TIMEOUTUSEC	200000
#define WWANFAPIAT_READ_TIMEOUTUSEC 	1000000
/*! \file wwanfapiat.h
 \brief File contains the default AT commands to be used for AT-only vendors who
 *  fail to provide the requisite command in their conf files
*/


/** Default AT commands to be used for AT-only vendors who
 *  fail to provide the requisite command in their conf 
 *  files. No guarantee is provided for the success of 
 *  these operations.
 */
#define WWANFAPI_ATCMD_DEFAULT_INIT               "at+cgatt=0;read;at+cgact=0;read;at+cops=2;at+cgdcont=1,\"IP\",\"internet\";at+xsystrace=0;at+xsystrace=1,\"bb_sw=0\",\"bb_sw=lte_stk:0x01,0x00000000, 0x00000000, 0x00000000, 0x00000000\";at+xsystrace=1,,\"bb_sw=egdci:0x00000000\";at@xl1:xllt_set_template(0,{basic});at@xl1:xllt_set_template(0,{EPHY_ALL});at@sysmon:log_stop(0);at@xptm:set_ip_trace_info(0,0);at+xsystrace=1,,\"bb_sw=period: status(0)\";at+xgendata;at+xact=2,2;at+cmee=2;at+xdatachannel=1,1,\"/USBCDC/0\",\"/USBHS/NCM/0\",2;at+cereg=2;at+cgerep=2;at+creg=2;at+cgreg=2;at+cemode=3"
#define WWANFAPI_ATCMD_DEFAULT_CONNECT            "at+cops=0;read;read;read;at+cgatt=1;at+cgact=1,20;read;read;read;read;at+cgdata=\"M-RAW_IP\",20;read"
#define WWANFAPI_ATCMD_DEFAULT_DISCONNECT         "at+cops=2"
#define WWANFAPI_ATCMD_DEFAULT_LOCK               "at+clck="
#define WWANFAPI_ATCMD_DEFAULT_UNLOCK             "at+clck="
#define WWANFAPI_ATCMD_DEFAULT_SETPIN             "at+cpin="
#define WWANFAPI_ATCMD_DEFAULT_CHANGEPIN          "at+cpwd="
#define WWANFAPI_ATCMD_GET_IP                     "at+cgcontrdp="
#define WWANFAPI_ATCMD_DEFAULT_DETACH             "at+cgatt=0;read;at+cgact=0;read;"

#define WWANFAPI_ATCMD_DEFAULT_IFSTATUS           "at+cgact?"
#define WWANFAPI_ATCMD_DEFAULT_SIGNAL             "at+csq"
#define WWANFAPI_ATCMD_DEFAULT_IMEI               "at+cgsn"
#define WWANFAPI_ATCMD_DEFAULT_USIMSTATUS         "at+cpin?"
#define WWANFAPI_ATCMD_DEFAULT_IMSI               "at+cimi"
#define WWANFAPI_ATCMD_DEFAULT_ICCID              "at+ccid"
#define WWANFAPI_ATCMD_DEFAULT_MSISDN             "at+cnum"



/**
    Structure used for setup of terminal properties
*/
typedef struct wwanfapiat_comparams {
    int32_t     wwanfapi_combaudrate;		/*!< Terminal Baud Rate */
    int32_t     wwanfapi_comparity;			/*!< Terminal Parity */
    int32_t     wwanfapi_csizebitsperbyte;	/*!< Number of bits to use per character  */
    int32_t     wwanfapi_usetwostopbits;	/*!< Whether to use two stop bits  */
    int32_t     wwanfapi_clocal;			/*!< Terminal CLOCAL */
    int32_t     wwanfapi_terminalfd;		/*!< Terminal FD */
    struct      termio wwanfapi_console;
    struct      termio wwanfapi_terminalsettings;
    struct      termio wwanfapi_terminalsettingsorig;
} wwanfapiat_comparams_t;

/**
    Function prototypes
*/

/**
    \brief This function extracts the first sub-string containing a number from given input string

    \param buf
        String buffer to extract the number from

    \return
        String containing number if successful
        NULL in case of error
*/
static char* wwanfapi_extract_num(char *buf);


/**
    \brief This function detach the modem from the registered LTE network

    \param cmd
        Command for the operation

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_do_detach(char *cmd, char *device);


/**
    \brief This function reads from console with the timeout provided

    \param pcparams
        Structure containing device FD

    \param readback_timeout
        Timeout for the operation

    \param buf
        If not NULL, the read data is dumped here

    \return
        WWANFAPI_SUCCESS if successful
        WWANFAPI_FAILURE in case of error
*/
int32_t wwanfapi_readfromconsole(wwanfapiat_comparams_t *pcparams, uint64_t readback_timeout, char *buf);




/**
    \brief This function writes to console the data provided in write_text

    \param pcparams
        Structure containing FD for the device

    \param write_text
        Text to write

    \return
        WWANFAPI_SUCCESS if successful
        WWANFAPI_FAILURE in case of error
*/
int32_t wwanfapi_writetoconsole(wwanfapiat_comparams_t *pcparams, char *write_text);




/**
    \brief This function resets the terminal

    \param pcparams
        Structure containing details for the terminal settings

    \return
*/
void wwanfapi_resetterminal(wwanfapiat_comparams_t *pcparams);




/**
    \brief This function opens device based on terminal properties

    \param device
        Device absolute path name

    \param pcparams
        Terminal properties

    \return
        Terminal FD if successful
        -1 in case of error
*/
int32_t wwanfapi_opendevice(char *device, wwanfapiat_comparams_t *pcparams);




/**
    \brief This function queries device for a device property mentioned in cmd

    \param cmd
        Command for the query

    \param buf
        Buffer to contain the result

    \param device
        Device to query

    \return
*/
void wwanfapi_at_read(char *cmd, char *buf, char *device);




/**
    \brief This function sets a device property mentioned in cmd

    \param cmd
        Command for the operation

    \param buf
        Buffer to contain the result

    \param device
        Device to query

    \return
*/
void wwanfapi_at_set(char *cmd, char *buf, char *device);




/**
    \brief This function initialises the modem

    \param cmd
        Command for the operation

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_do_init(char *cmd, char *device);




/**
    \brief This function connects the modem to a network

    \param cmd
        Command for the operation

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_do_connect(char *cmd, char *device);




/**
    \brief This function disconnects modem from network

    \param cmd
        Command for the operation

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_do_disconnect(char *cmd, char *device);




/**
    \brief This function provides PIN if needed

    \param cmd
        Command for the operation

    \param p
        Pin

    \param device
        Device path

    \return
        WWANFAPI_SUCCESS if successful
        WWANFAPI_FAILURE in case of error
*/
int wwanfapi_do_setpin(char *cmd, char *p, char* device);




/**
    \brief This function is used to lock the modem

    \param cmd
        Command for the operation

    \param p
        Pin for the operation

    \param device
        Device path

    \return
        WWANFAPI_SUCCESS if successful
        WWANFAPI_FAILURE in case of error
*/
int wwanfapi_do_lock(char *cmd, char *p, char* device);




/**
    \brief This function is used to unlock the modem

    \param cmd
        Command for the operation

    \param p
        Pin for the operation

    \param device
        Device path

    \return
        WWANFAPI_SUCCESS if successful
        WWANFAPI_FAILURE in case of error
*/
int wwanfapi_do_unlock(char *cmd, char *p, char* device);




/**
    \brief This function is used to change modem pin

    \param cmd
        Command for the operation

    \param op
        Old pin

    \param np
        New pin

    \param device
        Device path

    \return
        WWANFAPI_SUCCESS if successful
        WWANFAPI_FAILURE in case of error
*/
int wwanfapi_do_changepin(char *cmd, int op, char *np, char* device);




/**
    \brief This function is used to get the interface status

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_interfacestatus(char *cmd, char *ret, char *device);




/**
    \brief This function is used to get the signal strength

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_signalstrength(char *cmd, char *ret, char *device);




/**
    \brief This function is used to get IMEI

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_imei(char *cmd, char *ret, char *device);




/**
    \brief This function is used to get USIM Status

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_usimstatus(char *cmd, char *ret, char *device);




/**
    \brief This function is used to get IMSI

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_imsi(char *cmd, char *ret, char *device);




/**
    \brief This function is used to get ICCID

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_iccid(char *cmd, char *ret, char *device);




/**
    \brief This function is used to get MSISDN

    \param cmd
        Command for the operation

    \param ret
        Buffer to hold the results

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_msisdn(char *cmd, char *ret, char *device);

/**
    \brief This function is used to get IP assigned for PDN connection

    \param cmd
        Command for the operation

    \param buf
        String buffer to extract the ip from

    \param device
        Device absolute path in device tree

    \return
*/
void wwanfapi_get_ip(char *cmd, char *buf, char *device);
#endif /** #ifndef WWANFAPI_AT_INCLUDES */
/* @} */
