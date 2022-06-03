#include "WillyWeather/w_weather.h"

/**
 * Get weather forecast data from Willy Weather.
 *
 * Willy Weather can provide numerous weather forecasts. For example,
 * precis (text weather summary), rainfall, rainfallprobability,
 * sunrisesunset, swell, temperature, tides, uv, weather and wind.
 *
 * @code
 *     const char *location = "1215";
 *     const char *forecast_type = "tides";
 *     const char *start_date = "2022-05-01";
 *     const char *n_days = "1";
 *     WillyWeather_GetForecast(ww_token, location, forecast_type,
 *                              start_date, n_days);
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
                                  const char *location,
                                  const char *forecast_type,
                                  const char *start_date,
                                  const char *n_days){

    const char *BASE_URL = "https://api.willyweather.com.au/v2/";
    const char *BASE_LOCATION = "/locations/";
    const char *BASE_FORECAST = "/weather.json?forecasts=";
    const char *BASE_DAYS = "&days=";
    const char *BASE_START = "&startDate=";

    // Construct URL -> remember to free
    char *url = malloc(strlen(BASE_URL) +
                       strlen(token) +
                       strlen(BASE_LOCATION) +
                       strlen(location) +
                       strlen(BASE_FORECAST) +
                       strlen(forecast_type) +
                       strlen(BASE_DAYS) +
                       strlen(n_days) +
                       strlen(BASE_START) +
                       strlen(start_date) + 1);
    strcpy(url, BASE_URL);
    strcat(url, token);
    strcat(url, BASE_LOCATION);
    strcat(url, location);
    strcat(url, BASE_FORECAST);
    strcat(url, forecast_type);
    strcat(url, BASE_DAYS);
    strcat(url, n_days);
    strcat(url, BASE_START);
    strcat(url, start_date);

    // Add relevent headers -> remember to free
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    // Do something with response here

    free(url);
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
 *      WillyWeather_GetLocationByName(ww_token, search_location, "2");
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#location-get-search-by-query
 *
 * @warning Must not contain spaces in supplied name.
 *
 * @param token Willy Weather token.
 * @param name Name to query against (must not contain spaces!).
 * @param q_limit Number of returned sites to limit to.
 * @return The result status code provided by CURL.
 */
CURLcode WillyWeather_GetLocationByName(
                                        const char *token,
                                        const char *name,
                                        const char *q_limit) {

    const char *BASE_URL = "https://api.willyweather.com.au/v2/";
    const char *BASE_SEARCH = "/search.json?query=";
    const char *BASE_LIMIT = "&limit=";

    char *url = malloc(strlen(BASE_URL) +
                       strlen(token) +
                       strlen(BASE_SEARCH) +
                       strlen(name) +
                       strlen(BASE_LIMIT) +
                       strlen(q_limit) + 1);
    strcpy(url, BASE_URL);
    strcat(url, token);
    strcat(url, BASE_SEARCH);
    strcat(url, name);
    strcat(url, BASE_LIMIT);
    strcat(url, q_limit);

    struct curl_slist *headers= NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    char *res = cJSON_Print(response);
    printf("%s\n", res);
    free(res);

    // Do something with the response here

    free(url);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
