#ifndef PROGRAM_HTTP_H
#define PROGRAM_HTTP_H

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "utils.h"

/// HTTP GET & POST request using cURL.
CURLcode HttpRequest(cJSON **response, const char *URL,
                     struct curl_slist *headers, int8_t post, const char* body);

#endif //PROGRAM_HTTP_H
