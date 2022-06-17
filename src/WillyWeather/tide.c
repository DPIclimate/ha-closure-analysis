#include "WillyWeather/tide.h"

/**
 * Get calculated high and low tides from Willy Weather.
 *
 * Willy Weather provides tide data for coastal regions. These data consist
 * of high and low tides (and timestamps) grouped by day. This function extracts
 * relevent high, low and maximum daily tides and appends them to a tides
 * struct (WW_TideDataset_TypeDef).
 *
 *                ,-'''-.─────── High Tide (HT)
 *              ,-'   ▲   `-.
 *            ,'      │      `.
 *          ,'        │        `.
 *         /          │          \
 *        /           │           \
 *   ----+------------│------------\--------------------------
 *          Tide Diff │             \                       /
 *                    │              \                     /
 * MAX(TD = HT - LT)  │               `.                 ,'
 *                    │                 `.             ,'
 *                    │                   `-.       ,-'
 *                    ▼     Low Tide (LT) ───`-,,,-'
 *
 * @code
 * const char *start_date = "2022-05-01";
 * WillyWeather_GetForecast(ww_token, 1215, WW_FORECAST_TIDE, start_date, 10);
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#forecast-get-tides
 *
 * @note Requires valid access token. See authenticate.h for method to
 * authenticate with Willy Weather.
 *
 * @param location_id Location index provided by Willy Weather.
 * @param start_date Start date (e.g. YYYY-MM-DD HH:MM:SS).
 * @param n_days Number of days from the start date (basically end date).
 * @return CURLcode result integer.
 */
CURLcode WillyWeather_GetTides(uint16_t location_id,
                               const char *start_date,
                               uint16_t n_days,
                               WW_TideDataset_TypeDef *tides){

    if(WillyWeather_CheckAccess() == 1) return CURLE_AUTH_ERROR;

    char url[WW_FORECAST_URL_BUF];
    snprintf(url, WW_FORECAST_URL_BUF, "https://api.willyweather.com.au/v2/%s/"
                                   "locations/%hu/weather.json?forecasts=tides&"
                                   "startDate=%s&days=%hu", WW_TOKEN,
                                   location_id, start_date, n_days);

    log_info("Requesting tides from Willy Weather. URL: %s\n", url);

    // Add relevent headers -> remember to free
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    uint16_t h_index = 0;
    uint16_t l_index = 0;
    uint16_t day_index = 0;
    if(response != NULL){
        cJSON* forcasts = NULL;
        cJSON* j_tides = NULL;
        cJSON* days = NULL;
        cJSON* day = NULL;

        forcasts = cJSON_GetObjectItemCaseSensitive(response, "forecasts");
        if(forcasts != NULL){
            j_tides = cJSON_GetObjectItemCaseSensitive(forcasts, "tides");
            if(j_tides != NULL){
               days = cJSON_GetObjectItemCaseSensitive(j_tides, "days");

               cJSON* entries = NULL;
               cJSON* entry = NULL;
               cJSON* datetime = NULL;
               cJSON* height = NULL;
               cJSON* type = NULL;
               if(cJSON_IsArray(days)){
                   cJSON_ArrayForEach(day, days) {
                       entries = cJSON_GetObjectItemCaseSensitive(day,
                                                                  "entries");

                       if (cJSON_IsArray(entries)) {
                           double daily_max_tide = 0;
                           double daily_min_tide = 0;
                           double daily_tide_diff = 0;
                           cJSON *diff_day = NULL;
                           cJSON_ArrayForEach(entry, entries) {
                               // Get high or low tide
                               char high_low[5] = {0};
                               type = cJSON_GetObjectItemCaseSensitive(
                                       entry, "type");
                               if (cJSON_IsString(type) &&
                                   type->valuestring != NULL) {
                                   strncpy(high_low, type->valuestring, 5);
                               }

                               // Get datetime of value
                               datetime = cJSON_GetObjectItemCaseSensitive(
                                       entry, "dateTime");
                               if (cJSON_IsString(datetime) &&
                                   datetime->valuestring != NULL) {
                                   struct tm dt;
                                   memset(&dt, 0, sizeof(struct tm));
                                   strptime(datetime->valuestring,
                                            "%Y-%02m-%02d %02H:%02M:%02S",
                                            &dt);
                                   time_t unix_time = mktime(&dt);
                                   if (strcmp(high_low, "high") == 0) {
                                       tides->high_tide_timestamps[h_index] =
                                               unix_time;
                                   } else {
                                       tides->low_tide_timestamps[l_index] =
                                               unix_time;
                                   }
                               }

                               // Get height of tide
                               height = cJSON_GetObjectItemCaseSensitive(entry,
                                                                         "height");
                               if (cJSON_IsNumber(height)) {
                                   if (strcmp(high_low, "high") == 0) {
                                       tides->high_tide_values[h_index++] =
                                               height->valuedouble;
                                       if (height->valuedouble > daily_max_tide) {
                                           daily_max_tide = height->valuedouble;
                                       }
                                   } else {
                                       tides->low_tide_values[l_index++] =
                                               height->valuedouble;
                                       if (daily_min_tide == 0 ||
                                           height->valuedouble < daily_min_tide) {
                                           daily_min_tide = height->valuedouble;
                                       }
                                   }
                               }
                           }
                           diff_day = cJSON_GetObjectItemCaseSensitive(
                                   day, "dateTime");
                           if (cJSON_IsString(diff_day) &&
                               diff_day->valuestring != NULL) {

                               // UNIX daily timestamp
                               struct tm dt;
                               memset(&dt, 0, sizeof(struct tm));
                               strptime(diff_day->valuestring,
                                        "%Y-%02m-%02d %02H:%02M:%02S",
                                        &dt);
                               time_t unix_time = mktime(&dt);
                               tides->daily_max_tide_timestamps[day_index] =
                                       unix_time;

                               // Daily max tides
                               daily_tide_diff = daily_max_tide -
                                                 daily_min_tide;
                               tides->daily_max_tide_values[day_index++] =
                                       daily_tide_diff;
                           }
                       }
                   }
               }
            }
        }
    }

    if(result == CURLE_OK) {
        tides->n_days = day_index;
        tides->n_high_tide = h_index;
        tides->n_low_tide = l_index;

        log_info("Tides were successfully received and parsed "
                 "from Willy Weather.\n");
        log_debug("Number of day's parsed: %hu, Number of high tides: %hu, "
                 "Number of low tides: %hu\n", day_index, h_index, l_index);
    } else {
        log_error("Error parsing tide dataset. "
                  "Check response for malformed JSON.\n");
    }

    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}

/**
 * Create output CSV files with tide data from a particular location.
 *
 * Build tide directories and then for each of low, high and daily tide values
 * build .csv files. Spaces in filenames are handled by this function and
 * replaced with '-'.
 *
 * @param location_info Location struct for filename.
 * @param dataset Dataset to write to .csv files.
 * @return Error code. OK = 0 ... ERROR = 1
 */
uint8_t WillyWeather_TidesToCSV(WW_Location_TypeDef *location_info,
                                WW_TideDataset_TypeDef *dataset){

    log_info("Pushing tides dataset to .csv\n");

    // Make directories if not exists
    if(MakeDirectory("datasets") != 0) return 1;
    if(MakeDirectory("datasets/tides") != 0) return 1;

    // Remove spaces from filename
    char* location = malloc(strlen(location_info->location) + 1);
    for(uint8_t i = 0; i < strlen(location_info->location); i++){
        if(location_info->location[i] == ' '){
            location[i] = '-';
            continue;
        }
        location[i] = location_info->location[i];
    }

    // Name and create sub-directory
    char directory[50];
    memset(directory, 0, sizeof(directory));
    sprintf(directory, "datasets/tides/%s", location);
    if(MakeDirectory(directory) != 0) {
        free(location);
        return 1;
    }

    // Build low tide file
    char filename[100];
    sprintf(filename, "%s/low.csv", directory);
    WriteTimeseriesToFile(filename,
                          dataset->low_tide_timestamps,
                          dataset->low_tide_values,
                          WW_FORECAST_RESPONSE_BUF);

    // Build high tide file
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s/high.csv", directory);
    WriteTimeseriesToFile(filename,
                          dataset->high_tide_timestamps,
                          dataset->high_tide_values,
                          WW_FORECAST_RESPONSE_BUF);

    // Build daily maximum file
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s/daily-max.csv", directory);
    WriteTimeseriesToFile(filename,
                          dataset->daily_max_tide_timestamps,
                          dataset->daily_max_tide_values,
                          WW_FORECAST_RESPONSE_BUF);

    free(location);
    return 0;
}

/**
 * Build tide dataset from cache (.csv or .txt)
 *
 * Requires the following format Unix;Date;Values
 *
 * @param filename Filename (and path) of .csv or .txt file to open.
 * @return Tide dataset with values, timestamps and count
 */
WW_TideDataset_TypeDef WW_TidesFromCSV(const char* filename){
    WW_TideDataset_TypeDef dataset = {0};

    log_info("Loading timeseries dataset from: %s\n", filename);

    FILE* file = fopen(filename, "r");
    if(file == NULL) {
        log_error("Unable to open file: %s. Check filename and path.\n",
                  filename);
        return dataset;
    }

    char* ptr;
    time_t unix_time;
    char unix_buf[13], ts[26], tide_buf[21];
    double tide;
    int res;
    uint16_t iters = 0;
    do {
        res = fscanf(file, "%12[^;];%25[^;];%20[^\n]\n", unix_buf, ts,
                     tide_buf);
        unix_time = strtol(unix_buf, &ptr, 10);
        tide = strtof(tide_buf, &ptr);
        if(unix_time != 0){
            dataset.daily_max_tide_timestamps[iters] = unix_time;
            dataset.daily_max_tide_values[iters] = tide;
            iters++;
        }
    } while(res != EOF && iters < WW_FORECAST_RESPONSE_BUF);

    dataset.n_days = iters;

    fclose(file);

    log_info("%d timeseries datapoints loaded from %s.\n", iters, filename);

    return dataset;
}