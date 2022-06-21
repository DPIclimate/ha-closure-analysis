#include "http.h"

/**
 * Basic HTTP request using CURL.
 *
 * Most initialisation is done outside of this function. This function just
 * handles the HTTP request and puts the data inside a provided cJSON
 * object. Note this function has a memory leak on a Mac M1 ->
 * (curl_easy_perform()) has 13 leaks, totalling 496 bytes per call.
 *
 * @code
 *      const char *URL = "https://www.example.com";
 *
 *      // Create some headers (more may be needed depending on application)
 *      struct curl_slist *headers = NULL;
 *      headers = curl_slist_append(headers, "Content-Type: application/json");
 *
 *      // Create a cJSON object to hold the response
 *      cJSON *response = NULL;
 *      CURLcode result = HttpRequest(&response, URL, headers, 0, NULL);
 *
 *      // Free memory
 *      curl_slist_free_all(headers);
 *      cJSON_Delete(response);
 * @endcode
 *
 * @param response The cJSON response to populate with JSON.
 * @param URL The request URL.
 * @param headers Headers to include in request.
 * @param post Interger flag to request POST request (set to 1).
 * @param body POST request body.
 * @return Status code representing the response status.
 */
CURLcode HttpRequest(cJSON **response, const char *URL,
                     struct curl_slist *headers, int8_t post, const char *body) {

    CURL *curl = curl_easy_init();
    if (!curl) {
        log_error("Curl not found. Exiting.\n");
        return CURLE_FUNCTION_NOT_FOUND;
    }

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT); // Needed sometimes
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on 400 - 500 errors

    // Post request requested
    if (post == 1 && body != NULL) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    }

    ReqData_TypeDef chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);

    CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        log_error("Curl request failed: %s\n",
                  curl_easy_strerror(result));
    } else {
        *response = cJSON_Parse(chunk.memory);
    }

    free(chunk.memory);
    curl_easy_cleanup(curl);

    return result;
}
