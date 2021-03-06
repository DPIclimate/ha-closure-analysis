#include "IBM_EIS/authenticate.h"

/**
 * Handles authentication, checking and re-authentication of IBM EIS access
 * tokens.
 *
 * @code
 * IBM_AuthHandle_TypeDef auth_handle = {0};
 * if(IBM_HandleAuth(&auth_handle) != 0){
 *     return 1;
 * }
 * @endcode
 *
 * @note Authentication handler must be initialised to zero.
 *
 * @param auth_handle Authentication hander with tokens and expiry information.
 * @return Error code. 0 = OK 1 = ERROR
 */
int8_t IBM_HandleAuth(IBM_AuthHandle_TypeDef *auth_handle) {
    /// Check if initialised already
    if (auth_handle->token_expiry != 0) {
        time_t t_now = time(NULL);
        if (t_now < auth_handle->token_expiry) {
            log_info("IBM EIS authentication token is still "
                     "valid. Skipping re-authentication.\n");
            return 0;
        } else {
            log_info("IBM EIS authentication expired. "
                     "Refreshing.\n");
            if (IBM_Refresh(auth_handle) == CURLE_OK) {
                return 0;
            }
            log_error("IBM EIS refresh failed. Trying to "
                      "authenticate again.\n");
        }
    }

    const char *api_key = getenv(IBM_DEFAULT_TOKEN_NAME);
    if (api_key != NULL) {
        if (IBM_Authenticate(api_key, auth_handle) == CURLE_OK) {
            return 0;
        }
    } else {
        log_error("IBM EIS API key not found. "
                  "Unable to authenicate with IBM. Exiting.\n");
    }
    return 1;
}

/**
 * Authenticate with IBM's Enviornmental Monitoring Suite.
 *
 * IBM used OAuth2 for authentication. This function takes care of this
 * process. The lengths of the tokens are:
 * - Access token = 1861 bytes
 * - Refresh token = 43 bytes
 *
 * @param token IBM API key to use.
 * @param auth_handle Authenication handler for IBM tokens
 * @return CURLcode error message
 */
CURLcode IBM_Authenticate(const char *token,
                          IBM_AuthHandle_TypeDef *auth_handle) {

    log_info("Authenicating with IBM EIS.\n");

    // Build request body
    const char *BASE_BODY = "client_id=ibm-pairs"
                            "&grant_type=apikey"
                            "&apikey=";
    char *req_body = (char*)malloc(strlen(BASE_BODY) + strlen(token) + 1);
    strcpy(req_body, BASE_BODY);
    strcat(req_body, token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: "
                                         "application/x-www-form-urlencoded");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, IBM_TOKEN_URL, headers, 1, req_body);

    // Assign access token
    cJSON *j_refresh_token = NULL;
    cJSON *j_access_token = NULL;
    if (response != NULL) {
        j_access_token = cJSON_GetObjectItemCaseSensitive(response,
                                                          "access_token");
        if (cJSON_IsString(j_access_token) &&
            j_access_token->valuestring != NULL) {
            strncpy(auth_handle->access_token, j_access_token->valuestring,
                    IBM_ACCESS_TOKEN_SIZE);
        } else {
            result = CURLE_RECV_ERROR;
        }

        j_refresh_token = cJSON_GetObjectItemCaseSensitive(response,
                                                           "refresh_token");
        if (cJSON_IsString(j_refresh_token) &&
            j_refresh_token->valuestring != NULL) {
            strncpy(auth_handle->refresh_token, j_refresh_token->valuestring,
                    IBM_REFRESH_TOKEN_SIZE);
        }
    } else {
        result = CURLE_RECV_ERROR;
    }

    if (result == CURLE_OK) {
        // Set UNIX time expiry in just under 1 hour
        auth_handle->token_expiry = time(NULL) + 3500;
        log_info("Authenicated with IBM EIS. Token expires at: "
                 "%ld\n", auth_handle->token_expiry);
    }

    free(req_body);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}

/**
 * Re-authenticate with IBM's Enviornmental Monitoring Suite.
 *
 * IBM used OAuth2 for re-authentication. This function takes care of this
 * process. The access_token length is at least 1861 characters.
 *
 * @code
 * char* access_token[IBM_ACCESS_TOKEN_SIZE]; // Access token to populate
 * IBM_Refresh(refresh_token, access_token);
 * @endcode
 *
 * @note Ensure malloc size if large enough to hold the returned access
 * token (at least 1861 bytes).
 *
 * @param auth_handle IBM EIS authentication handler.
 * @return CURLcode error message
 */
CURLcode IBM_Refresh(IBM_AuthHandle_TypeDef *auth_handle) {

    log_info("Re-authenicating with IBM EIS.\n");

    const char *BASE_BODY = "client_id=ibm-pairs"
                            "&grant_type=refresh_token"
                            "&refresh_token=";

    char *req_body = (char*)malloc(
            strlen(BASE_BODY) +
            strlen(auth_handle->refresh_token) + 1);
    strcpy(req_body, BASE_BODY);
    strcat(req_body, auth_handle->refresh_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: "
                                         "application/x-www-form-urlencoded");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, IBM_TOKEN_URL, headers, 1, req_body);

    // Assign access token
    cJSON *j_access_token = NULL;
    if (response != NULL) {
        j_access_token = cJSON_GetObjectItemCaseSensitive(response,
                                                          "access_token");
        if (cJSON_IsString(j_access_token) &&
            j_access_token->valuestring != NULL) {
            strcpy(auth_handle->access_token, j_access_token->valuestring);
        } else {
            result = CURLE_RECV_ERROR;
        }
    } else {
        result = CURLE_RECV_ERROR;
    }

    if (response == CURLE_OK) {
        // Set UNIX time expiry in just under 1 hour
        auth_handle->token_expiry = time(NULL) + 3500;
        log_info("Re-authenicated with IBM EIS. Token expires "
                 "at: %ld\n", auth_handle->token_expiry);
    }

    free(req_body);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;

}
