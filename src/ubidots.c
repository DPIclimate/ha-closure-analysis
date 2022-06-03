#include "../include/ubidots.h"

/**
 * Method to get a list of all devices on Ubidots.
 *
 * Page size should be set high enough to get all devices (default 500).
 *
 * @code
 *      // Load ubidots token
 *      const char *ubi_token = getenv("UBI_TOKEN");
 *      if(!ubi_token) {
 *      	printf("Ubidots token not found. Exiting.\n");
 *      	return EXIT_FAILURE;
 *      }
 *      Ubidots_ListDevices(ubi_token);
 * @endcode
 *
 * @see https://docs.ubidots.com/reference/get-all-devices
 *
 * @param token Ubidots API token to use.
 * @return Status code respresenting response status.
 */
CURLcode Ubidots_ListDevices(const char *token){
    const char *URL = "https://industrial.api.ubidots.com.au"
                      "/api/v2.0/devices/?page_size=500";

    // Add headers, remember to free these later
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    const char* auth_key = "X-Auth-Token: ";
    char *auth = malloc(strlen(auth_key) + strlen(token) + 1);
    strcpy(auth, auth_key);
    strcat(auth, token);

    headers = curl_slist_append(headers, auth);

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, URL, headers, 0, NULL);

    // Do things with the response here

    free(auth);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
