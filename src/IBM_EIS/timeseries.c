#include "IBM_EIS/timeseries.h"

/// Parse timeseries response
static void IBM_ParseTimeseries(cJSON* response,
                                IBM_TimeseriesDataset_TypeDef *dataset);

/// Parse timeseries response from alternative endpoint
static void IBM_ParseTimeseriesAlt(cJSON* response,
                                   IBM_TimeseriesDataset_TypeDef *dataset);

/// Build IBM EMS request URL
static void IBM_BuildURL(IBM_TimeseriesReq_TypeDef *req, char* url);

/// Build IBM EMS request URL for alternative endpoint
static void IBM_BuildURLAlt(IBM_TimeseriesReq_TypeDef *req, char* url);

/**
 * IBM EMS get timeseries data as a JSON response.
 *
 * IBM EMS provides two endpoints to get data from:
 * - https://pairs.res.ibm.com (IBM_REQUEST_URL)
 * - https://ibmpairs-mvp2-api.mybluemix.net (IBM_ALT_REQUEST_URL)
 *
 * The responses from these requests have different formats therefore
 * they need to be treated seperatly. Luckly similar data is used for
 * each request making the use of TimeseriesReq_TypeDef possible for both
 * requests.
 *
 * @code
 *  TimeseriesReq_TypeDef ts = {
 *          .layer_id = 16700, // 16700 (alt_flag = 0) or 49097 (alt_flag = 1)
 *          .latitude = -35.69701049568654,
 *          .longitude = 150.1546566614602,
 *          .start = 1654005600,
 *          .end = 1654783200
 *  };
 *
 *  TimeseriesDataset_TypeDef dataset;
 *  IBM_GetTimeseries(auth_handle, &ts, &dataset, 0); // or 1 (for alt URL)
 * @endcode
 *
 * @note Start and end times are represented as UNIX timestamps (in seconds).
 *
 * @param auth_handle IBM authentication handler
 * @param request Request struct with corresponding data.
 * @param dataset The dataset to populate.
 * @param alt_flag A flag representing if the alt enpoint should be used.
 * @return Curl success code.
 */
CURLcode IBM_GetTimeseries(IBM_AuthHandle_TypeDef *auth_handle,
                           IBM_TimeseriesReq_TypeDef *request,
                           IBM_TimeseriesDataset_TypeDef *dataset,
                           uint8_t alt_flag){

    if(IBM_HandleAuth(auth_handle) != 0){
        return CURLE_AUTH_ERROR;
    }

    char url[IBM_URL_SIZE];
    if(alt_flag == 1){
        IBM_BuildURLAlt(request, url);
    } else {
        IBM_BuildURL(request, url);
    }

    fprintf(stdout, "[Info]: GET from IBM EMS (layer ID: %d) : %s\n",
            request->layer_id, url);

    // Authorization builder
    const char* BASE_HEADER = "Authorization: Bearer ";
    char* auth_header = malloc(strlen(BASE_HEADER) +
            strlen(auth_handle->access_token) + 1);
    strcpy(auth_header, BASE_HEADER);
    strcat(auth_header, auth_handle->access_token);

    // Add headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "accept: application/json");

    cJSON* response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    if(response != NULL){
        if(alt_flag == 1){
            IBM_ParseTimeseriesAlt(response, dataset);
        } else {
            IBM_ParseTimeseries(response, dataset);
        }
    } else {
        fprintf(stderr, "[Error]: Unable to getting JSON response from "
                        "IBM EMS timeseries dataset.\n");
    }

    free(auth_header);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    if(result == CURLE_OK){
        fprintf(stdout, "[Info]: IBM EMS timeseries request was successful.\n");
    }

    return result;
}

/**
 * Parse timeseries data from IBM URL response.
 *
 * This function takes JSON data received from the endpoint
 * https://pairs.res.ibm.com (IBM_REQUEST_URL) to formulate
 * a dataset (TimeseriesDataset_TypeDef).
 *
 * @param response The JSON to parse.
 * @param dataset The dataset to populate.
 */
static void IBM_ParseTimeseries(cJSON* response,
                                IBM_TimeseriesDataset_TypeDef *dataset){

    fprintf(stdout, "[Info]: Parsing IBM EIS request results.\n");

    // For query information
    cJSON* start = NULL;
    cJSON* end = NULL;
    cJSON* count = NULL;

    // For array data
    cJSON* point = NULL;
    cJSON* data = NULL;

    // Get the start time as time_t
    start = cJSON_GetObjectItemCaseSensitive(response, "start");
    if(cJSON_IsNumber(start)){
        dataset->start = floor(start->valuedouble);
    }

    // Get the end time as time_t
    end = cJSON_GetObjectItemCaseSensitive(response, "end");
    if(cJSON_IsNumber(end)){
        dataset->end = floor(end->valuedouble);
    }

    // Number of values returned (count)
    count = cJSON_GetObjectItemCaseSensitive(response, "count");
    if(cJSON_IsNumber(count)){
        dataset->count = floor(count->valuedouble);
    }

    memset(dataset->timestamp, 0, IBM_MAX_RESPONSE_LENGTH);
    memset(dataset->values, 0, IBM_MAX_RESPONSE_LENGTH);

    data = cJSON_GetObjectItemCaseSensitive(response, "data");
    uint16_t index = 0;
    if(cJSON_IsArray(data)){
        cJSON_ArrayForEach(point, data){
            if(index > IBM_MAX_RESPONSE_LENGTH){
                fprintf(stderr,
                        "[Error]: Allocated memory exceeded. "
                        "Increase the size of IBM_MAX_RESPONSE_LENGTH to "
                        "account for the number of results in this query to "
                        "IMB EMS.\n");
                return;
            }
            cJSON* ts = NULL;
            cJSON* value = NULL;
            ts = cJSON_GetObjectItemCaseSensitive(point, "timestamp");
            if(cJSON_IsNumber(ts)){
                dataset->timestamp[index] = floor(ts->valuedouble);
            }

            value = cJSON_GetObjectItemCaseSensitive(point, "value");
            if(cJSON_IsString(value) && value->valuestring != NULL){
                dataset->values[index] = strtod(value->valuestring, NULL);
            } else {
                fprintf(stderr, "[Error]: Parse error for value: %s\n",
                        value->valuestring);
            }
            index++;
        }
    }

    fprintf(stdout, "[Info]: Finished parsing response from IBM EMS.\n");
}

/**
 * Parse timeseries data from alternative IBM URL response.
 *
 * This function takes JSON data received from the endpoint
 * https://ibmpairs-mvp2-api.mybluemix.net (IBM_ALT_REQUEST_URL) to formulate
 * a dataset (TimeseriesDataset_TypeDef).
 *
 * @param response The JSON to parse.
 * @param dataset The dataset to populate.
 */
static void IBM_ParseTimeseriesAlt(cJSON* response,
                                   IBM_TimeseriesDataset_TypeDef *dataset){

    fprintf(stdout, "[Info]: Parsing IBM EIS alternative request results.\n");

    // For query information
    cJSON* count = NULL;

    // For array data
    cJSON* point = NULL;
    cJSON* data = NULL;

    // Number of values returned (count)
    count = cJSON_GetObjectItemCaseSensitive(response, "count");
    if(cJSON_IsNumber(count)){
        dataset->count = floor(count->valuedouble);
    }

    memset(dataset->timestamp, 0, IBM_MAX_RESPONSE_LENGTH);
    memset(dataset->values, 0, IBM_MAX_RESPONSE_LENGTH);

    data = cJSON_GetObjectItemCaseSensitive(response, "timeSeries");
    uint16_t index = 0;
    if(cJSON_IsArray(data)){
        cJSON_ArrayForEach(point, data){

            cJSON* ts = NULL;
            cJSON* value = NULL;

            ts = cJSON_GetObjectItemCaseSensitive(point, "dateTime");
            if(cJSON_IsString(ts) && ts->valuestring != NULL){
                struct tm v_time;
                memset(&v_time, 0, sizeof(struct tm));
                strptime(ts->valuestring, "%FT%T%z", &v_time);
                time_t unix_time = mktime(&v_time);
                if(unix_time != -1){
                    dataset->timestamp[index] = unix_time;
                    if(index == 0){
                        dataset->start = unix_time;
                    }
                    if(index == dataset->count - 1){
                        dataset->end = unix_time;
                    }
                } else {
                    fprintf(stderr, "Error parsing datetime: %s\n",
                            ts->valuestring);
                }
            }

            value = cJSON_GetObjectItemCaseSensitive(point, "value");
            if(cJSON_IsNumber(value)){
                memcpy(&dataset->values[index], &value->valuedouble,
                       sizeof(double));
            }
            index++;
        }
    } else {
        fprintf(stderr, "[Error]: IBM response received. However, the response "
                        "was in an unrecognized format and "
                        "could not be parased.\n");
    }

    fprintf(stdout, "[Info]: Finished parsing response from IBM EMS.\n");
}

/**
 * Build a URL for IBM EMS at endpoint https://pairs.res.ibm.com
 *
 * Takes a request structure and formulates the appropriate URL.
 *
 * @param req Request information to populate URL with.
 * @param url URL to modify.
 */
static void IBM_BuildURL(IBM_TimeseriesReq_TypeDef *req, char* url){

    fprintf(stdout, "[Info]: Building URL for IBM EMS endpoint: %s\n",
            IBM_REQUEST_URL);

    snprintf(url, IBM_URL_SIZE, "%s/v2/timeseries?layer="
                                "%d"
                                "&lat=%f"
                                "&lon=%f"
                                "&start=%ld"
                                "&end=%ld",
             IBM_REQUEST_URL,
             req->layer_id,
             req->latitude,
             req->longitude,
             req->start * 1000,
             req->end * 1000);
}

/**
 * Build a URL for IBM EMS at endpoint https://ibmpairs-mvp2-api.mybluemix.net
 *
 * Takes a request structure and formulates the appropriate URL.
 *
 * @param req Request information to populate URL with.
 * @param url URL to modify.
 */
static void IBM_BuildURLAlt(IBM_TimeseriesReq_TypeDef *req, char* url){

    fprintf(stdout, "[Info]: Building URL for alternative IBM EMS endpoint: "
                    "%s\n", IBM_ALT_REQUEST_URL);

    // Create start time char* e.g. 2022-06-06T12:25:55.000Z
    const uint8_t START_SIZE = 25;
    char start_time[START_SIZE];
    struct tm *start_tm = localtime(&req->start);
    snprintf(start_time, START_SIZE, "%d-%02d-%02dT%02d:%02d:%02d.000Z",
            start_tm->tm_year + 1900,
            start_tm->tm_mon + 1,
            start_tm->tm_mday,
            start_tm->tm_hour,
            start_tm->tm_min,
            start_tm->tm_sec);

    // Create end time char*
    const uint8_t END_SIZE = 25;
    char end_time[END_SIZE];
    struct tm *end_tm = localtime(&req->end);
    snprintf(end_time, END_SIZE, "%d-%02d-%02dT%02d:%02d:%02d.000Z",
            end_tm->tm_year + 1900,
            end_tm->tm_mon + 1,
            end_tm->tm_mday,
            end_tm->tm_hour,
            end_tm->tm_min,
            end_tm->tm_sec);

    snprintf(url, IBM_URL_SIZE, "%s/timeseries/"
                 "%d"
                 "?latitude=%f"
                 "&longitude=%f"
                 "&startingDateTime=%s"
                 "&endingDateTime=%s",
            IBM_ALT_REQUEST_URL,
            req->layer_id,
            req->latitude,
            req->longitude,
            start_time,
            end_time);
}
