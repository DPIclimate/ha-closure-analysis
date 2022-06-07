#include "WillyWeather/authenticate.h"

/**
 * Obtain and populate Willy Weather access token from environment variable.
 *
 * @param env_var_name Name of environment variable where token is held.
 * @return Integer representing
 */
uint8_t WillyWeather_GetToken(const char* env_var_name){
    strncpy(WW_TOKEN, getenv(env_var_name), WW_TOKEN_SIZE);
    if(strlen(WW_TOKEN) == 0) {
        fprintf(stderr, "[Error]: Willy Weather token not found as an "
                        "environment variable with name: '%s'.\n",
                env_var_name);
        return 1;
    }
    fprintf(stdout, "[Info]: Willy Weather access token found and "
                    "initialised.\n");
    return 0;
}

/**
 * Check Willy Weather API token has been initialised.
 *
 * @return Integer representing access state (0 = success)
 */
uint8_t WillyWeather_CheckAccess(){
    if(strlen(WW_TOKEN) == 0){
        fprintf(stderr, "[Error]: Willy Weather access token was "
                        "uninitialised. A valid access token is required to "
                        "access Willy Weather. See: "
                        "WillyWeather_GetToken(<token>)\n");
        return 1;
    }
    return 0;
}

