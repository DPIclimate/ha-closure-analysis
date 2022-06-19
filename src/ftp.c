#include "ftp.h"

CURLcode FTPRequest(const char* url, ReqData_TypeDef* stream) {
    CURL *curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);

    stream->memory = malloc(1);
    stream->size = 0;

    curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream);

    CURLcode result = curl_easy_perform(curl);

    curl_easy_cleanup(curl);

    return result;
}

