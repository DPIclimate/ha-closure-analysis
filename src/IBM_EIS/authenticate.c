#include "IBM_EIS/authenticate.h"

/**
 * Authenticate with IBM's Enviornmental Monitoring Suite.
 *
 * IBM used OAuth2 for authentication. This function takes care of this
 * process. The lengths of the tokens are:
 * - Access token = 1861 bytes
 * - Refresh token = 43 bytes
 *
 * @code
 * // Allocate memory to hold IBM EMS tokens
 * char* access_token[IBM_ACCESS_TOKEN_SIZE]; // Access token to populate
 * char* refresh_token[IBM_REFRESH_TOKEN_SIZE]; // Refresh token to populate
 * IBM_Authenticate(ibm_token, refresh_token, access_token);
 *
 * // Check for errors
 * if(access_token == NULL && refresh_token == NULL){
 *      return ERROR;
 * }
 * @endcode
 *
 * @param token IBM API key to use.
 * @param refresh_token Refresh token to populate.
 * @param access_token Access token to populate.
 * @return CURLcode error message
 */
CURLcode IBM_Authenticate(const char* token,
                          char* refresh_token,
                          char* access_token) {

    // Build request body
    const char* BASE_BODY = "client_id=ibm-pairs"
                            "&grant_type=apikey"
                            "&apikey=";
    char* req_body = malloc(strlen(BASE_BODY) + strlen(token) + 1);
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
    if(response != NULL){
        j_access_token = cJSON_GetObjectItemCaseSensitive(response,
                                                          "access_token");
        if(cJSON_IsString(j_access_token) &&
        j_access_token->valuestring != NULL){
            strncpy(access_token, j_access_token->valuestring,
                    IBM_ACCESS_TOKEN_SIZE);
        }
        else {
            result = CURLE_RECV_ERROR;
        }

        j_refresh_token = cJSON_GetObjectItemCaseSensitive(response,
                                                           "refresh_token");
        if(cJSON_IsString(j_refresh_token) &&
        j_refresh_token->valuestring != NULL){
            strncpy(refresh_token, j_refresh_token->valuestring,
                    IBM_REFRESH_TOKEN_SIZE);
        }
    } else {
        result = CURLE_RECV_ERROR;
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
 * @param refresh_token IBM EMS refresh token.
 * @param access_token Access token to populate.
 * @return CURLcode error message
 */
CURLcode IBM_Refresh(char* refresh_token,
                     char* access_token) {
    const char* BASE_BODY = "client_id=ibm-pairs"
                            "&grant_type=refresh_token"
                            "&refresh_token=";

    char* req_body = malloc(strlen(BASE_BODY) + strlen(refresh_token) + 1);
    strcpy(req_body, BASE_BODY);
    strcat(req_body, refresh_token);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: "
                                         "application/x-www-form-urlencoded");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, IBM_TOKEN_URL, headers, 1, req_body);

    // Assign access token
    cJSON *j_access_token = NULL;
    if(response != NULL){
        j_access_token = cJSON_GetObjectItemCaseSensitive(response,
                                                          "access_token");
        if(cJSON_IsString(j_access_token) &&
           j_access_token->valuestring != NULL){
            strcpy(access_token, j_access_token->valuestring);
        }
        else {
            result = CURLE_RECV_ERROR;
        }
    } else {
        result = CURLE_RECV_ERROR;
    }

    free(req_body);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;

}