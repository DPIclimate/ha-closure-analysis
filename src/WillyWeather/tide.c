#include "WillyWeather/tide.h"

/**
 * Get weather forecast data from Willy Weather.
 *
 * Willy Weather can provide numerous weather forecasts. For example,
 * precis (text weather summary), rainfall, rainfallprobability,
 * sunrisesunset, swell, temperature, tides, uv, weather and wind.
 *
 * @code
 *     const char *start_date = "2022-05-01";
 *     WillyWeather_GetForecast(ww_token, 1215, WW_FORECAST_TIDE,
 *                              start_date, 10);
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#forecast-get-tides
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

    fprintf(stdout, "[Info]: Requesting tides from Willy Weather."
                    " URL: %s\n", url);

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
                               char high_low[5];
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

    if(result == CURLE_OK && day_index){
        fprintf(stdout, "[Info]: Tides were successfully received and parsed "
                        "from Willy Weather.\n"
                        "\tNumber of day's parsed:\t\t%hu\n"
                        "\tNumber of high tides:\t\t%hu\n"
                        "\tNumber of low tides:\t\t%hu\n", day_index,
                        h_index, l_index);
    } else{
        fprintf(stderr, "[Error]: Error parsing tide dataset.\n"
                        "Number of day's parsed:\t\t%hu\n", day_index);
    }

    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
