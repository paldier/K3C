/********************************************************************************
 
        Copyright (c) 2012, 2014, 2015
        Lantiq Beteiligungs-GmbH & Co. KG
        For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/
/** \addtogroup FAPI_LTE */
/* @{ */
#ifndef WWANFAPI_MBIM_INCLUDES
#define WWANFAPI_MBIM_INCLUDES
/*! \file wwanfapimbim.h
 \brief File contains the structure and function definitions for MBIM service
*/

typedef enum {
    WWANFAPI_MBIM_SUCCESS = 0,   /*!< Macro signifying operation success */
    WWANFAPI_MBIM_FAILURE = 1    /*!< Macro signifying operation failure */
} wwanfapi_mbim_status_code_t;


typedef enum {
    WWANFAPI_MBIM_SERVICE_STATE_NONE = 0,                 /*!< MBIM Service no request state */
    WWANFAPI_MBIM_SERVICE_STATE_REQUESTING = 1,           /*!< MBIM Service request generating state */
    WWANFAPI_MBIM_SERVICE_STATE_AWAITING_REPONSE = 2,     /*!< MBIM Service response awaiting state */
    WWANFAPI_MBIM_SERVICE_STATE_RESPONDED = 3,            /*!< MBIM Service response received state */
    WWANFAPI_MBIM_SERVICE_STATE_FAILED = 4                /*!< MBIM Service operation failed state */
} wwanfapi_mbim_service_request_state_t;

#define WWANFAPI_MBIM_SERVICE_TYPE_QUERY_DEVICE_CAPS     "caps"           /*!< Service query for device caps */
#define WWANFAPI_MBIM_SERVICE_TYPE_QUERY_PIN             "pinstate"       /*!< Service query for PIN state */
#define WWANFAPI_MBIM_SERVICE_TYPE_UNLOCK                "unlock"         /*!< Service request for unlock operation */
#define WWANFAPI_MBIM_SERVICE_TYPE_REGISTRATION          "registration"   /*!< Service request for registration operation */
#define WWANFAPI_MBIM_SERVICE_TYPE_SUBSCRIBER            "subscriber"     /*!< Service query for subscriber status */
#define WWANFAPI_MBIM_SERVICE_TYPE_ATTACH                "attach"         /*!< Service request for attach */
#define WWANFAPI_MBIM_SERVICE_TYPE_DETACH                "detach"         /*!< Service request for dettach */
#define WWANFAPI_MBIM_SERVICE_TYPE_CONNECT               "connect"        /*!< Service request for connect */
#define WWANFAPI_MBIM_SERVICE_TYPE_DISCONNECT            "disconnect"     /*!< Service request for disconnect */
#define WWANFAPI_MBIM_SERVICE_TYPE_CONFIG                "config"         /*!< Service request for config */




/**
    Structure describing mbim service
*/
typedef struct wwanfapi_mbim_service {
    char                                  cmd[32];         /*!< Service description */
    wwanfapi_mbim_service_request_state_t state;           /*!< Request state */ 
} wwanfapi_mbim_service_t;




/**
    Structure describing MBIM request
*/
typedef struct wwanfapi_mbim_request {
    char                        device_str[32];           /*!< Device path */
    char                        param[128];               /*!< Params */
    uint32_t                    nargs;                    /*!< Number of args */
    wwanfapi_mbim_status_code_t status;                   /*!< Overall status of operation */
    wwanfapi_mbim_service_t     service;                  /*!< Service Details */
    void                        *response;                /*!< Response pointer, containg results of operation */
} wwanfapi_mbim_request_t;




/** Device capabilities */
typedef struct wwanfapi_mbim_device_caps_resp {
    char     devicetype[32];
    char     cellularclass[32];
    char     voiceclass[32];
    uint32_t simclass;
    uint32_t dataclass;
    uint32_t smscaps;
    uint32_t controlcaps;
    uint32_t maxsessions;
    char     deviceid[64];
    char     firmwareinfo[64];
    char     hardwareinfo[64];
} wwanfapi_mbim_device_caps_resp_t;

/** Pin state */
typedef struct wwanfapi_mbim_pin_state_resp {
    char     pintype[32];
    char     pinstate[32];
    uint32_t remaining_attempts;
} wwanfapi_mbim_pin_state_resp_t;

/** Registration response */
typedef struct wwanfapi_mbim_registration_resp {
    char nwerror[256];
    char registered_state[32];
    char registered_mode[32];
    char data_classes[32];
    char cur_cellular_class[32];
    char provider_id[256];
    char provider_name[256];
    char roamingtext[256];
} wwanfapi_mbim_registration_resp_t;

/** Subscriber response */
typedef struct wwanfapi_mbim_subscriber_resp {
    char readystate[32];
    char simiccid[64];
    char subscriberid[64];
} wwanfapi_mbim_subscriber_resp_t;

/** Attach response */
typedef struct wwanfapi_mbim_attach_resp {
    char     nwerror[256];
    char     packetservicestate[32];
    uint64_t uplinkspeed;
    uint64_t downlinkspeed;
} wwanfapi_mbim_attach_resp_t;

/** Connect response */
typedef struct wwanfapi_mbim_connect_resp {
    uint32_t sessionid;
    char     activationstate[32];
    char     nwerror[256];
    char     iptype[32];
} wwanfapi_mbim_connect_resp_t;


/**
    \brief This function delegates the task to MBIM handling layer

    \param pmreq
        Mbim request structure containing details of requested operation

    \return
*/
void wwanfapi_mbim_delegate_command(wwanfapi_mbim_request_t *pmreq);
#endif /** #ifndef WWANFAPI_MBIM_INCLUDES */
/* @} */
