#include "ftp.h"

/**
 * FTP request for data.
 *
 * This FTP request is required to communicate with the Buerau of Meterology
 * FTP server. A URL is provided (denoting the base ftp server name and the
 * path to the file of interest). A stream is also give to hold the raw data
 * provided from this FTP request.
 *
 * @param url URL to file of interest on FTP server.
 * @param stream Stream to hold data from response (see utils.h).
 * @return Curl status code representing FTP errors.
 */
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

