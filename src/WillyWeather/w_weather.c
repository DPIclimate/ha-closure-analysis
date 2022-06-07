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

    char url[200];
    sprintf(url, "https://api.willyweather.com.au/v2/%s/locations/%hu/"
                 "weather.json?forecasts=%s&startDate=%s&days=%hu", token,
                 location, forecast_type, start_date, n_days);

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
 *      const char* search_location = "Batemans";
 *      WillyWeather_GetLocationByName(ww_token, search_location, 2);
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#location-get-search-by-query
 *
 * @warning Must not contain spaces in supplied query name.
 *
 * @param token Willy Weather token.
 * @param name Name to query against (must not contain spaces!).
 * @param q_limit Number of returned sites to limit to.
 * @return The result status code provided by CURL.
 */
CURLcode WillyWeather_GetLocationByName(const char *token,
                                        const char *name,
                                        uint8_t q_limit){

    char url[200];
    sprintf(url, "https://api.willyweather.com.au/v2/%s/search.json?"
                 "query=%s&limit=%hu", token, name, q_limit);

    struct curl_slist *headers= NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    char *res = cJSON_Print(response);
    printf("%s\n", res);
    free(res);

    // Do something with the response here

    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
