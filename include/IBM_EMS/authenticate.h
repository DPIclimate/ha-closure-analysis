#ifndef HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
#define HA_CLOSURE_ANALYSIS_AUTHENTICATE_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

/// Authenticate with IBM's Environmental Monitoring Suite (EMS).
CURLcode IBM_Authenticate(const char* token,
                          char* refresh_token,
                          char* access_token);

/// Obtain a new access token from IBM's EMS.
CURLcode IBM_Refresh(const char* refresh_token,
                     char* access_token);

#endif //HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
