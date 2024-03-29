#include "WillyWeather/precipitation.h"

/**
 * Get forecast precipitation data from Willy Weather.
 *
 * Willy Weather provides forecast rainfall probability and amount as a range
 * e.g. 10-15 mm 90 %. These data are provided for the past 2 days, the current
 * day and the next 6 days (9 days in total). A location needs to be provided
 * along with a daily rainfall struct to hold the responses.
 *
 * @code
 * // Get location by name
 * const char* search_location = "Batemans";
 * WW_Location_TypeDef location_info = {0};
 * WillyWeather_GetLocationByName(search_location, &location_info);
 *
 * // Get rainfall for a location
 * WW_RainfallForecast_TypeDef rainfall_forecast = {0};
 * WillyWeather_GetRainfallForecast(&location_info, &rainfall_forecast);
 * @endcode
 *
 * @see https://www.willyweather.com.au/api/docs/v2.html#forecast-get-rainfall
 *
 * @param location Location struct (see location.h).
 * @param daily_rainfall Daily rainfall forecast struct to hold responses.
 * @return Curl code representing HTTP errors that may have occured.
 */
CURLcode WillyWeather_GetRainfallForecast(WW_Location_TypeDef *location,
                                          WW_RainfallForecast_TypeDef *rainfall_forecast) {

    if (WillyWeather_CheckAccess() == 1) return CURLE_AUTH_ERROR;

    // Build start time as current day
    struct tm stm = *localtime(&(time_t) {time(NULL)}); // Current time
    time_t unix_time = mktime(&stm) - 172800; // Two days ago
    struct tm ptm = *localtime(&unix_time);
    char start_date[11] = {0};
    strftime(start_date, 10, "%Y-%m-%d", &ptm); // Todays date minus two days

    // Max 9 day forcast (7 days inc today plus 2 days prior)
    char url[WW_FORECAST_URL_BUF];
    snprintf(url, WW_FORECAST_URL_BUF,
             "https://api.willyweather.com.au/v2/%s/"
             "locations/%hu/weather.json?"
             "forecasts=rainfall&"
             "startDate=%s&days=9", WW_TOKEN, location->id, start_date);

    log_info("Requesting precipitation from Willy Weather. URL: %s\n", url);

    // Add relevent headers -> remember to free
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    int16_t index = 0;
    if (response != NULL) {

        cJSON *forcasts = NULL;
        cJSON *rainfall = NULL;

        forcasts = cJSON_GetObjectItemCaseSensitive(response, "forecasts");
        if (forcasts != NULL) {
            rainfall = cJSON_GetObjectItemCaseSensitive(forcasts, "rainfall");
            if (rainfall != NULL) {
                cJSON *days = NULL;
                cJSON *day = NULL;
                cJSON *entries = NULL;
                cJSON *entry = NULL;

                cJSON *datetime = NULL;
                cJSON *start_range = NULL;
                cJSON *end_range = NULL;
                cJSON *range_divide = NULL;
                cJSON *range_code = NULL;
                cJSON *probability = NULL;
                days = cJSON_GetObjectItemCaseSensitive(rainfall, "days");

                if (cJSON_IsArray(days)) {
                    cJSON_ArrayForEach(day, days) {
                        if (index > WW_MAX_DAILY_RAINFALL_RESULTS) break;

                        datetime = cJSON_GetObjectItemCaseSensitive(day, "dateTime");
                        if (datetime != NULL && datetime->valuestring != NULL) {
                            struct tm dt = {0};
                            if (strptime(datetime->valuestring, "%Y-%m-%d %H:%M:%S", &dt) != NULL) {
                                // Convert to UNIX time and append
                                char ts_tz[WW_MAX_TS_SIZE];
                                strftime(ts_tz, sizeof(ts_tz), "%Y-%m-%d %H:%M:%S%z", &dt);
                                strncpy(rainfall_forecast->forecast[index].ts, ts_tz, WW_MAX_TS_SIZE);
                                rainfall_forecast->forecast[index].date = mktime(&dt);
                            } else
                                log_error("Datetime conversion failed: %s\n", datetime->valuestring);
                        }
                        entries = cJSON_GetObjectItemCaseSensitive(day, "entries");
                        if (cJSON_IsArray(entries)) {
                            // Should only be one entry for each day
                            cJSON_ArrayForEach(entry, entries) {
                                // Start range (1, 5, 10, 15, 25, 50, null)
                                start_range = cJSON_GetObjectItemCaseSensitive(entry, "startRange");
                                if (cJSON_IsNumber(start_range)) {
                                    rainfall_forecast->forecast[index].start_range = (int8_t)start_range->valueint;
                                } else {
                                    rainfall_forecast->forecast[index].start_range = 0; // for value "null" return 0
                                }

                                // End range (5, 10, 15, 25, 50, 100)
                                end_range = cJSON_GetObjectItemCaseSensitive(entry, "endRange");
                                if (cJSON_IsNumber(end_range)) {
                                    rainfall_forecast->forecast[index].end_range = (int8_t) end_range->valueint;
                                } else
                                    log_error("Unable to parse end rainfall range.\n");

                                // Range divider ('>' or '=')
                                range_divide = cJSON_GetObjectItemCaseSensitive(entry, "rangeDivide");
                                if (range_divide != NULL && range_divide->valuestring != NULL) {
                                    rainfall_forecast->forecast[index].range_divider = range_divide->valuestring[0];
                                } else
                                    log_error("Error parsing rainfall range divider.\n");

                                // Range code (0, 1-5, 5-10, 10-15, 15-25, 25-50, 50-100, 100) combines start & end range
                                range_code = cJSON_GetObjectItemCaseSensitive(entry, "rangeCode");
                                if (range_code != NULL && range_code->valuestring != NULL) {
                                    strncpy(rainfall_forecast->forecast[index].range_code, range_code->valuestring,
                                            WW_RANGE_CODE_SIZE);
                                } else
                                    log_error("Error parsing rainfall range code.\n");

                                // Probability (value between 0 and 100 (%))
                                probability = cJSON_GetObjectItemCaseSensitive(entry, "probability");
                                if (cJSON_IsNumber(probability)) {
                                    rainfall_forecast->forecast[index].probability = (int8_t) probability->valueint;
                                } else
                                    log_error("Error parsing rainfall probability.\n");

                                log_debug("Date: %s,\tRainfall: %s,\tProbability: %d\n", datetime->valuestring,
                                          range_code->valuestring, probability->valueint);
                            }
                        } else
                            log_error("Entries not found in JSON response.\n");
                        index++;
                    }
                } else
                    log_error("No days found in rainfall JSON response.\n");
            } else
                log_error("Rainfall JSON object not found in response.\n");
        } else
            log_error("Unable to get rainfall forecast. Malformed JSON.\n");
    } else
        log_error("Willy Weather rainfall response was empty.\n");

    rainfall_forecast->n_days = index;

    if (result == CURLE_OK) {
        if (index != 0) {
            log_info("Willy Weather daily rainfall request was successful.\n");
        } else {
            log_error("Willy Weather daily rainfall request was successful. "
                      "But no results were returned.\n");
        }
    }

    curl_slist_free_all(headers);
    cJSON_Delete(response);
    return result;
}

/**
 * Write Willy Weather forecast for a particular location to PostgreSQL table.
 *
 * The target table is called weather_ww.
 *
 * @param location Location information for Willy Weather dataset.
 * @param forecast Willy Weather weather forecast.
 * @param psql_conn PostgreSQL connection handler.
 */
void WillyWeather_RainfallToDB(WW_Location_TypeDef* location,
                               WW_RainfallForecast_TypeDef* forecast,
                               PGconn* psql_conn){

    const char* stmt_name = "InsertWillyWeather";
    Utils_PrepareStatement(psql_conn, stmt_name,
                           "INSERT INTO weather_ww (last_updated, location, "
                           "location_id, ts, rainfall_start_range, "
                           "rainfall_end_range, rainfall_range_divider, "
                           "rainfall_range_code, rainfall_probability_of_any) "
                           "VALUES (NOW(), $1::text, $2::int, $3::timestamptz, "
                           "$4::int, $5::int, $6::char, $7::text, $8::int) "
                           "ON CONFLICT (location_id, ts) DO "
                           "UPDATE SET last_updated = NOW(), "
                           "rainfall_start_range = $9::int, "
                           "rainfall_end_range = $10::int, "
                           "rainfall_range_divider = $11::char, "
                           "rainfall_range_code = $12::text, "
                           "rainfall_probability_of_any = $13::int;", 13);
    const char* paramValues[13];

    char locid_buf[10]; // Location ID buffer
    char srf_buf[10]; // Start rainfall buffer
    char erf_buf[10]; // End rainfall buffer
    char probrf_buf[10]; // Probability rainfall buffer

    int16_t index = 0;
    while(index < forecast->n_days){
        WW_Rainfall_TypeDef daily_rf = forecast->forecast[index];

        paramValues[0] = location->location;
        snprintf(locid_buf, sizeof(locid_buf), "%d", location->id);
        paramValues[1] = locid_buf;
        paramValues[2] = daily_rf.ts;
        snprintf(srf_buf, sizeof(srf_buf), "%d", daily_rf.start_range);
        paramValues[3] = srf_buf;

        snprintf(erf_buf, sizeof(erf_buf), "%d", daily_rf.end_range);
        paramValues[4] = erf_buf;

        paramValues[5] = &daily_rf.range_divider;
        paramValues[6] = daily_rf.range_code;

        snprintf(probrf_buf, sizeof(probrf_buf), "%d", daily_rf.probability);
        paramValues[7] = probrf_buf;

        paramValues[8] = srf_buf;
        paramValues[9] = erf_buf;
        paramValues[10] = &daily_rf.range_divider;

        paramValues[11] = daily_rf.range_code;
        paramValues[12] = probrf_buf;

        PGresult* res = PQexecPrepared(psql_conn, stmt_name, 13, paramValues,
                                       NULL, NULL, 1);

        if(PQresultStatus(res) != PGRES_COMMAND_OK){
            log_error("PSQL command failed when entering %s "
                      "information. Error: %s\n", location->location,
                      PQerrorMessage(psql_conn));
        }
        PQclear(res);
        index++;
    }

}
