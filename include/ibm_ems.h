#ifndef HA_CLOSURE_ANALYSIS_IBM_EMS_H
#define HA_CLOSURE_ANALYSIS_IBM_EMS_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

__attribute__((unused)) CURLcode IBM_Authenticate(const char* token,
                                                  char* refresh_token,
                                                  char* access_token);

__attribute__((unused)) CURLcode IBM_Refresh(const char* refresh_token,
                                             char* access_token);

#endif //HA_CLOSURE_ANALYSIS_IBM_EMS_H
