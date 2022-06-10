#ifndef HA_CLOSURE_ANALYSIS_UTILS_H
#define HA_CLOSURE_ANALYSIS_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

/// Holds HTTP response data before converting these data into cJSON objects.
typedef struct {
	char *memory; ///< The (response) data
	size_t size; ///< Size of the (response) data
} MemoryStruct_TypeDef;

/// Make directories (with error handling)
int8_t MakeDirectory(const char* directory);

/// HTTP GET & POST request using cURL.
CURLcode HttpRequest(cJSON **response, const char *URL,
                     struct curl_slist *headers, int8_t post, const char* body);

/// Write timeseries data into a csv file
void WriteTimeseriesToFile(const char* filename, time_t* dates, double* values,
                           uint16_t max_n_values);

#endif // HA_CLOSURE_ANALYSIS_UTILS_H
