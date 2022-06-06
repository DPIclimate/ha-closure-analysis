#ifndef HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
#define HA_CLOSURE_ANALYSIS_AUTHENTICATE_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

/// Buffer size to hold access token
#define IBM_ACCESS_TOKEN_SIZE 2000

/// Buffer size of hold refresh token
#define IBM_REFRESH_TOKEN_SIZE 50

/// IBM authenticate (token) URL
static const char* IBM_TOKEN_URL = "https://auth-b2b-twc.ibm.com/connect/token";

/// Authenticate with IBM's Environmental Monitoring Suite (EMS).
CURLcode IBM_Authenticate(const char* token,
                          char* refresh_token,
                          char* access_token);

/// Obtain a new access token from IBM's EMS.
CURLcode IBM_Refresh(char* refresh_token,
                     char* access_token);

#endif //HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
