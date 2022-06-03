#ifndef HA_CLOSURE_ANALYSIS_UBIDOTS_H
#define HA_CLOSURE_ANALYSIS_UBIDOTS_H

#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

CURLcode Ubidots_ListDevices(const char *token);

#endif // HA_CLOSURE_ANALYSIS_UBIDOTS_H

