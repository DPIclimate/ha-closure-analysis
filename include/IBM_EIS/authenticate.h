#ifndef HA_CLOSURE_ANALYSIS_AUTHENTICATE_H_
#define HA_CLOSURE_ANALYSIS_AUTHENTICATE_H_

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <log.h>

#include "http.h"
#include "utils.h"

/// Default ENV variable name of API key
#define IBM_DEFAULT_TOKEN_NAME "IBM_TOKEN"

/// Buffer size to hold access token
#define IBM_ACCESS_TOKEN_SIZE 2000

/// Buffer size of hold refresh token
#define IBM_REFRESH_TOKEN_SIZE 50

/// IBM authenticate (token) URL
static const char* IBM_TOKEN_URL = "https://auth-b2b-twc.ibm.com/connect/token";

/// IBM token handler
typedef struct {
    time_t token_expiry;
    char access_token[IBM_ACCESS_TOKEN_SIZE];
    char refresh_token[IBM_REFRESH_TOKEN_SIZE];
} IBM_AuthHandle_TypeDef;

/// Authentication handler for IBM EIS
int8_t IBM_HandleAuth(IBM_AuthHandle_TypeDef *auth_handle);

/// Authenticate with IBM's Environmental Intelligence Suite (EIS).
CURLcode IBM_Authenticate(const char* token,
                          IBM_AuthHandle_TypeDef* auth_handle);

/// Obtain a new access token from IBM's EMS.
CURLcode IBM_Refresh(IBM_AuthHandle_TypeDef *auth_handle);

#endif //HA_CLOSURE_ANALYSIS_AUTHENTICATE_H_
