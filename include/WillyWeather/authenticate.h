#ifndef HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
#define HA_CLOSURE_ANALYSIS_AUTHENTICATE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <log.h>

#include "http.h"

/// Willy weather access token size
#define WW_TOKEN_SIZE                          100
char WW_TOKEN[WW_TOKEN_SIZE];

/// Willy weather default environmental variable name
#define WW_DEFAULT_ENV_NAME                    "WW_TOKEN"

/// Get Willy Weather API token from environment variable
uint8_t WillyWeather_GetToken(const char* env_var_name);

/// Check API token is initialised and available for use
uint8_t WillyWeather_CheckAccess();

#endif //HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
