#include "IBM_EIS/timeseries.h"

/// Parse timeseries response
static void IBM_ParseTimeseries(cJSON *response,
                                IBM_TimeseriesDataset_TypeDef *dataset);

/// Parse timeseries response from alternative endpoint
static void IBM_ParseTimeseriesAlt(cJSON *response,
                                   IBM_TimeseriesDataset_TypeDef *dataset);

/// Build IBM EIS request URL
static void IBM_BuildURL(IBM_TimeseriesReq_TypeDef *req, char *url);

/// Build IBM EIS request URL for alternative endpoint
static void IBM_BuildURLAlt(IBM_TimeseriesReq_TypeDef *req, char *url);

/**
 * IBM EIS get timeseries data as a JSON response.
 *
 * IBM EIS provides two endpoints to get data from:
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
                           uint8_t alt_flag) {

    if (IBM_HandleAuth(auth_handle) != 0) {
        return CURLE_AUTH_ERROR;
    }

    char url[IBM_URL_SIZE];
    if (alt_flag == 1) {
        IBM_BuildURLAlt(request, url);
    } else {
        IBM_BuildURL(request, url);
    }

    log_info("Getting IBM EIS layer (ID: %d) : %s\n",
             request->layer_id, url);

    // Authorization builder
    const char *BASE_HEADER = "Authorization: Bearer ";
    char *auth_header = (char*)malloc(strlen(BASE_HEADER) +
                               strlen(auth_handle->access_token) + 1);
    strcpy(auth_header, BASE_HEADER);
    strcat(auth_header, auth_handle->access_token);

    // Add headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "accept: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    if (response != NULL) {
        if (alt_flag == 1) {
            IBM_ParseTimeseriesAlt(response, dataset);
        } else {
            IBM_ParseTimeseries(response, dataset);
        }
    } else {
        log_error("Unable to getting JSON response from "
                  "IBM EIS timeseries dataset.\n");
    }

    free(auth_header);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    if (result == CURLE_OK) {
        log_info("IBM EIS timeseries request was successful.\n");
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
static void IBM_ParseTimeseries(cJSON *response,
                                IBM_TimeseriesDataset_TypeDef *dataset) {

    log_info("Parsing IBM EIS request results.\n");

    // For query information
    cJSON *start = NULL;
    cJSON *end = NULL;
    cJSON *count = NULL;

    // For array data
    cJSON *point = NULL;
    cJSON *data = NULL;

    // Get the start time as time_t
    start = cJSON_GetObjectItemCaseSensitive(response, "start");
    if (cJSON_IsNumber(start)) {
        dataset->start = (time_t)start->valuedouble;
    }

    // Get the end time as time_t
    end = cJSON_GetObjectItemCaseSensitive(response, "end");
    if (cJSON_IsNumber(end)) {
        dataset->end = (time_t)end->valuedouble;
    }

    // Number of values returned (count)
    count = cJSON_GetObjectItemCaseSensitive(response, "count");
    if (cJSON_IsNumber(count)) {
        dataset->count = (int32_t)count->valuedouble;
    }

    memset(dataset->timestamps, 0, IBM_MAX_RESPONSE_LENGTH);
    memset(dataset->values, 0, IBM_MAX_RESPONSE_LENGTH);

    data = cJSON_GetObjectItemCaseSensitive(response, "data");
    uint16_t index = 0;
    if (cJSON_IsArray(data)) {
        cJSON_ArrayForEach(point, data) {
            if (index > IBM_MAX_RESPONSE_LENGTH) {
                log_error(
                        "Allocated memory exceeded. "
                        "Increase the size of IBM_MAX_RESPONSE_LENGTH to "
                        "account for the number of results in this query to "
                        "IMB EIS.\n");
                return;
            }
            cJSON *ts = NULL;
            cJSON *value = NULL;
            ts = cJSON_GetObjectItemCaseSensitive(point, "timestamp");
            if (cJSON_IsNumber(ts)) {
                dataset->timestamps[index] = (time_t)ts->valuedouble;
            }

            value = cJSON_GetObjectItemCaseSensitive(point, "value");
            if (cJSON_IsString(value) && value->valuestring != NULL) {
                dataset->values[index] = strtod(value->valuestring, NULL);
            } else {
                log_error("Parse error for value: %s\n",
                          value->valuestring);
            }
            index++;
        }
    }

    log_info("Finished parsing response from IBM EIS.\n");
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
static void IBM_ParseTimeseriesAlt(cJSON *response,
                                   IBM_TimeseriesDataset_TypeDef *dataset) {

    log_info("Parsing IBM EIS alternative request results.\n");

    // For query information
    cJSON *count = NULL;

    // For array data
    cJSON *point = NULL;
    cJSON *data = NULL;

    // Number of values returned (count)
    count = cJSON_GetObjectItemCaseSensitive(response, "count");
    if (cJSON_IsNumber(count)) {
        dataset->count = (uint16_t)count->valuedouble;
    }

    memset(dataset->timestamps, 0, IBM_MAX_RESPONSE_LENGTH);
    memset(dataset->values, 0, IBM_MAX_RESPONSE_LENGTH);

    data = cJSON_GetObjectItemCaseSensitive(response, "timeSeries");
    uint16_t index = 0;
    if (cJSON_IsArray(data)) {
        cJSON_ArrayForEach(point, data) {

            cJSON *ts = NULL;
            cJSON *value = NULL;

            ts = cJSON_GetObjectItemCaseSensitive(point, "dateTime");
            if (cJSON_IsString(ts) && ts->valuestring != NULL) {
                struct tm v_time;
                memset(&v_time, 0, sizeof(struct tm));
                strptime(ts->valuestring, "%FT%T%z", &v_time);
                time_t unix_time = mktime(&v_time);
                if (unix_time != -1) {
                    dataset->timestamps[index] = unix_time;
                    if (index == 0) {
                        dataset->start = unix_time;
                    }
                    if (index == dataset->count - 1) {
                        dataset->end = unix_time;
                    }
                } else {
                    log_error("Unable to parse datetime: %s\n",
                              ts->valuestring);
                }
            }

            value = cJSON_GetObjectItemCaseSensitive(point, "value");
            if (cJSON_IsNumber(value)) {
                dataset->values[index] = value->valuedouble;
            }
            index++;
        }
    } else {
        log_error("IBM response received. However, the response "
                  "was in an unrecognized format and "
                  "could not be parased.\n");
    }

    log_info("Finished parsing response from IBM EIS.\n");
}

/**
 * Build a URL for IBM EIS at endpoint https://pairs.res.ibm.com
 *
 * Takes a request structure and formulates the appropriate URL.
 *
 * @param req Request information to populate URL with.
 * @param url URL to modify.
 */
static void IBM_BuildURL(IBM_TimeseriesReq_TypeDef *req, char *url) {

    log_info("Building URL for IBM EIS endpoint: %s\n",
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
             (req->start - 86400) * 1000,
             (req->end - 86400) * 1000);
}

/**
 * Build a URL for IBM EIS at endpoint https://ibmpairs-mvp2-api.mybluemix.net
 *
 * Takes a request structure and formulates the appropriate URL.
 *
 * @param req Request information to populate URL with.
 * @param url URL to modify.
 */
static void IBM_BuildURLAlt(IBM_TimeseriesReq_TypeDef *req, char *url) {

    log_info("Building URL for alternative IBM EIS endpoint: "
             "%s\n", IBM_ALT_REQUEST_URL);

    // Create start time char* e.g. 2022-06-06T12:25:55.000Z
    const uint8_t START_SIZE = 25;
    char start_time[START_SIZE];
    time_t start_offset = req->start - 86400;
    struct tm *start_tm = localtime(&start_offset);
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
    time_t end_offset = req->end - 86400;
    struct tm *end_tm = localtime(&end_offset);
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

/**
 * IBM timeseires dataset to csv.
 *
 * Creates required directories and then builds a csv files with the downloaded
 * dataset.
 *
 * @param request Timeseries request information (layer ID is needed).
 * @param dataset Dataset to write to file. Count is provided here too.
 * @return Error code. 0 = OK ... 1 = ERROR
 */
int8_t IBM_TimeseriesToCSV(IBM_TimeseriesReq_TypeDef *request,
                           IBM_TimeseriesDataset_TypeDef *dataset) {

    log_info("Writing IBM timeseries dataset to a .csv file.\n");

    if (MakeDirectory("datasets") != 0) return 1;
    if (MakeDirectory("datasets/ibm") != 0) return 1;
    if (MakeDirectory("datasets/ibm/timeseries") != 0) return 1;

    int8_t FILENAME_SIZE = 100;
    char filename[FILENAME_SIZE];
    switch (request->layer_id) {
        case IBM_PRECIPITATION_ID:
            strncpy(filename, "datasets/ibm/timeseries/precipitation.csv",
                    (size_t)FILENAME_SIZE);
            break;
        default:
            strncpy(filename, "datasets/ibm/timeseries/unknown.csv",
                    (size_t)FILENAME_SIZE);
            break;
    }

    WriteTimeseriesToFile(filename, dataset->timestamps, dataset->values,
                          (int16_t)dataset->count);

    log_info("IBM timeseries dataset witten to: %s\n", filename);

    return 0;
}

/**
 * Load timeseries dataset from a cached .csv for .txt file.
 *
 * Requires the file to be formatted as Unix;Date;Value
 *
 * @param filename Filename of file to open, including path.
 * @return Dataset containing timeseries data
 */
IBM_TimeseriesDataset_TypeDef IBM_TimeseriesFromCSV(const char *filename) {
    IBM_TimeseriesDataset_TypeDef dataset = {0};

    log_info("Loading timeseries dataset from: %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        log_error("Unable to open file: %s. Check filename and path.\n",
                  filename);
        return dataset;
    }

    char *ptr;
    time_t unix_time;
    char unix_buf[13], ts[26], precip_buf[21];
    double precip;
    int res;
    uint16_t iters = 0;
    do {
        res = fscanf(file, "%12[^;];%25[^;];%20[^\n]\n", unix_buf, ts,
                     precip_buf);
        unix_time = strtol(unix_buf, &ptr, 10);
        precip = strtod(precip_buf, &ptr);
        if (unix_time != 0) {
            dataset.values[iters] = precip;
            dataset.timestamps[iters] = unix_time;
            iters++;
        }
    } while (res != EOF && iters < IBM_MAX_RESPONSE_LENGTH);

    dataset.count = iters;
    fclose(file);

    log_info("%d timeseries datapoints loaded from %s.\n", iters, filename);

    return dataset;
}

void IBM_TimeseriesToDB(IBM_TimeseriesReq_TypeDef* req_info,
                        IBM_TimeseriesDataset_TypeDef* dataset,
                        T_LocationLookup_TypeDef* location,
                        PGconn* psql_conn){

    log_info("Inserting IBM query results into PostgreSQL database.\n");

    int32_t index = 0;
    while(index < dataset->count){
        char ts[30];
        // Add one day as this is UTC time and we want it to line up with
        // AEST time
        time_t unix_time = dataset->timestamps[index];
        struct tm ctm = *localtime(&unix_time);
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S%z", &ctm);
        char query[3000];
        switch(req_info->layer_id){
            case IBM_PRECIPITATION_ID:
                snprintf(query, sizeof(query), "INSERT INTO weather_ibm_eis ("
                                               "last_updated, location, "
                                               "ww_location_id, "
                                               "bom_location_id, latitude, "
                                               "longitude, "
                                               "ts, "
                                               "precipitation) "
                                               "VALUES ("
                                               "NOW(), " // Last updated
                                               "'%s', " // Location name
                                               "'%s', " // WW location id
                                               "'%s', " // BOM location id
                                               "%f, " // Latitude
                                               "%f, " // Longitude
                                               "'%s', " // Timestamp
                                               "%f) " // Precipitation
                                               "ON CONFLICT (ts, "
                                               "ww_location_id, "
                                               "bom_location_id) DO "
                                               "UPDATE SET "
                                               "last_updated = NOW();",
                         location->ww_location,
                         location->ww_location_id,
                         location->bom_location_id,
                         req_info->latitude,
                         req_info->longitude,
                         ts,
                         dataset->values[index]);
                break;

            case IBM_MIN_TEMPERATURE_ID:
                snprintf(query, sizeof(query), "INSERT INTO weather_ibm_eis ("
                                               "last_updated, location, "
                                               "ww_location_id, "
                                               "bom_location_id, latitude, "
                                               "longitude, ts, precipitation) "
                                               "VALUES ("
                                               "NOW(), " // Last updated
                                               "'%s', " // Location name
                                               "'%s', " // WW location id
                                               "'%s', " // BOM location id
                                               "%f, " // Latitude
                                               "%f, " // Longitude
                                               "'%s', " // Timestamp
                                               "%f) " // Precipitation
                                               "ON CONFLICT (ts, "
                                               "ww_location_id, "
                                               "bom_location_id) DO "
                                               "UPDATE SET "
                                               "last_updated = NOW(), "
                                               "min_temperature = %f;",
                         location->ww_location,
                         location->ww_location_id,
                         location->bom_location_id,
                         req_info->latitude,
                         req_info->longitude,
                         ts,
                         dataset->values[index] - 272.15,
                         dataset->values[index] - 272.15);
                break;

            case IBM_MAX_TEMPERATURE_ID:
                snprintf(query, sizeof(query), "INSERT INTO weather_ibm_eis ("
                                               "last_updated, location, "
                                               "ww_location_id, "
                                               "bom_location_id, latitude, "
                                               "longitude, ts, precipitation) "
                                               "VALUES ("
                                               "NOW(), " // Last updated
                                               "'%s', " // Location name
                                               "'%s', " // WW location id
                                               "'%s', " // BOM location id
                                               "%f, " // Latitude
                                               "%f, " // Longitude
                                               "'%s', " // Timestamp
                                               "%f) " // Precipitation
                                               "ON CONFLICT (ts, "
                                               "ww_location_id, "
                                               "bom_location_id) DO "
                                               "UPDATE SET "
                                               "last_updated = NOW(), "
                                               "max_temperature = %f;",
                         location->ww_location,
                         location->ww_location_id,
                         location->bom_location_id,
                         req_info->latitude,
                         req_info->longitude,
                         ts,
                         dataset->values[index] - 272.15,
                         dataset->values[index] - 272.15);
                break;


            default:
                log_error("Unknown IBM dataset query results. Unable to insert"
                          "into PostgreSQL database.\n");
                break;
        }

        PGresult* res = PQexec(psql_conn, query);
        if(PQresultStatus(res) != PGRES_COMMAND_OK){
            log_error("PSQL command failed when parsing IBM query. Error: %s\n",
                      PQerrorMessage(psql_conn));
        }
        PQclear(res);
        index++;
    }
    log_info("Done inserting timeseries data from IBM EIS into "
             "PostgreSQL database.\n");
}

