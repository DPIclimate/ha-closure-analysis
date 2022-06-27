#include "WillyWeather/authenticate.h"

/**
 * Obtain and populate Willy Weather access token from environment variable.
 *
 * @param env_var_name Name of environment variable where token is held.
 * @return Integer representing
 */
uint8_t WillyWeather_GetToken(const char *env_var_name) {
    char *token = getenv(env_var_name);
    if (token == NULL) return 1; // Not found error
    strncpy(WW_TOKEN, token, WW_TOKEN_SIZE);
    log_info("Willy Weather access token found and "
             "initialised.\n");
    return 0;
}

/**
 * Check Willy Weather API token has been initialised.
 *
 * @return Integer representing access state (0 = success)
 */
uint8_t WillyWeather_CheckAccess(void) {
    if (strlen(WW_TOKEN) == 0) {
        log_warn("Willy Weather access token not found. "
                  "Trying again with the default environmental variable "
                  "name: %s\n", WW_DEFAULT_ENV_NAME);
        if (WillyWeather_GetToken(WW_DEFAULT_ENV_NAME) == 0) {
            return 0;
        }
        log_error("Willy Weather access token not found with "
                  "default token name: %s\n", WW_DEFAULT_ENV_NAME);
        return 1;
    }
    return 0;
}

