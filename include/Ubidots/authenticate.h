#ifndef HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
#define HA_CLOSURE_ANALYSIS_AUTHENTICATE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <log.h>

#include "http.h"

/// Maximum number of characters in access token
#define UBIDOTS_TOKEN_SIZE                      100
static char UBIDOTS_TOKEN[UBIDOTS_TOKEN_SIZE];

/// Default environmental variable for ubidots
#define UBIDOTS_DEFAULT_ENV_NAME "UBI_TOKEN"

/// Get / set access token
int8_t Ubidots_GetToken(const char* env_var_name);

/// Check access token
int8_t Ubidots_CheckAccess(void);

#endif //HA_CLOSURE_ANALYSIS_AUTHENTICATE_H
