#ifndef HA_CLOSURE_ANALYSIS_UTILS_H
#define HA_CLOSURE_ANALYSIS_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

/// Holds HTTP response data before converting these data into cJSON objects.
typedef struct {
	char *memory; ///< The (response) data
	size_t size; ///< Size of the (response) data
} MemoryStruct_TypeDef;

/// HTTP GET & POST request using cURL.
CURLcode HttpRequest(cJSON **response, const char *URL,
                     struct curl_slist *headers, int post, const char* body);

#endif // HA_CLOSURE_ANALYSIS_UTILS_H
