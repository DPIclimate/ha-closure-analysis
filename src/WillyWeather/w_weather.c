#include "WillyWeather/w_weather.h"

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
 * @param token API Token for Willy Weather.
 * @param location Location index provided by Willy Weather.
 * @param forecast_type Environmental forecast is required.
 * @param start_date Start date (e.g. YYYY-MM-DD HH:MM:SS).
 * @param n_days Number of days from the start date (basically end date).
 * @return CURLcode result integer.
 */
CURLcode WillyWeather_GetForecast(const char *token,
                                  uint16_t location,
                                  const char *forecast_type,
                                  const char *start_date,
                                  uint16_t n_days){

    char url[WW_MAX_URL_SIZE];
    snprintf(url, WW_MAX_URL_SIZE, "https://api.willyweather.com.au/v2/%s/"
                                   "locations/%hu/weather.json?forecasts=%s&"
                                   "startDate=%s&days=%hu", token, location,
                                   forecast_type, start_date, n_days);

    // Add relevent headers -> remember to free
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    // Do something with response here

    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}


/**
 * Get location ID from Willy Weather.
 *
 * Willy Weather forecast requires a location ID. This is obtained by querying
 * by name (e.g. Batemans) -> no spaces are allowed. The response contains
 * cJSON data of which multiple sites can be obatined (limited through q_limit).
 *
 * @code
 * const char* search_location = "Batemans";
 * WW_Location_TypeDef *location_info;
 * WillyWeather_GetLocationByName(ww_token, search_location, location_info);
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#location-get-search-by-query
 *
 * @warning Must not contain spaces in supplied query name.
 * @info Response length is limited to one as to ensure only one location is
 * returned per query.
 *
 * @param token Willy Weather token.
 * @param name Name to query against (must not contain spaces!).
 * @param location_info Location structure to populate.
 * @return The result status code provided by CURL.
 */
CURLcode WillyWeather_GetLocationByName(const char *token,
                                        const char *name,
                                        WW_Location_TypeDef *location_info){

    char url[WW_MAX_URL_SIZE];
    snprintf(url, WW_MAX_URL_SIZE, "https://api.willyweather.com.au/v2/%s/"
                                   "search.json?query=%s&limit=%d", token,
                                   name, 1);

    struct curl_slist *headers= NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    char *res = cJSON_Print(response);
    free(res);

    // Response is limited to 1 (makes it easier to parse automatically)
    if(response != NULL && cJSON_IsArray(response)){
        cJSON* id = NULL;
        cJSON* location = NULL;
        cJSON* latitude = NULL;
        cJSON* longitude = NULL;

        cJSON* item = NULL;
        cJSON_ArrayForEach(item, response){

            // Location ID for queries
            id = cJSON_GetObjectItemCaseSensitive(item, "id");
            if(cJSON_IsNumber(id)){
                location_info->id = floor(id->valuedouble);
            }

            // Location name
            location = cJSON_GetObjectItemCaseSensitive(item, "name");
            if(cJSON_IsString(location) && location->valuestring != NULL){
                memset(&location_info->location, 0, WW_MAX_BUFFER);
                strncpy(location_info->location, location->valuestring,
                        WW_MAX_BUFFER);
            }

            // Location latitude
            latitude = cJSON_GetObjectItemCaseSensitive(item, "lat");
            if(cJSON_IsNumber(latitude)){
                location_info->latitude = latitude->valuedouble;
            }

            // Location longitude
            longitude = cJSON_GetObjectItemCaseSensitive(item, "lng");
            if(cJSON_IsNumber(longitude)){
                location_info->longitude = longitude->valuedouble;
            }
        }
    }

    if(result == CURLE_OK){
        fprintf(stdout, "[Info]: Location request to Willy Weather was "
                        "successful.\n"
                        "Location ID:\t\t%hu\n"
                        "Location Name:\t\t%s\n"
                        "Coordinates:\n"
                        "\tLatitude:\t%lf\n"
                        "\tLongitude:\t%lf\n",
                        location_info->id, location_info->location,
                        location_info->latitude, location_info->longitude);
    } else {
        fprintf(stderr, "[Error]: Unable to process Willy Weather location"
                        "request. Error status: %d\n", result);
    }

    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
