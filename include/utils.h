#ifndef HA_CLOSURE_ANALYSIS_UTILS_H
#define HA_CLOSURE_ANALYSIS_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

typedef struct {
	char *memory;
	size_t size;
} MemoryStruct_TypeDef;

CURLcode HttpRequest(cJSON **response, const char *URL,
                     struct curl_slist *headers, int post, const char* body);

#endif // HA_CLOSURE_ANALYSIS_UTILS_H
