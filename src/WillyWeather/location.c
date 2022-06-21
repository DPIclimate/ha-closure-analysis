#include "WillyWeather/location.h"

/**
 * Get location ID from Willy Weather.
 *
 * Willy Weather forecast requires a location ID. This is obtained by querying
 * by name (e.g. Batemans Bay). The response contains cJSON data of which
 * multiple sites can be obatined (limited through q_limit).
 *
 * @code
 * WW_Location_TypeDef *location_info;
 * WillyWeather_GetLocationByName("Batemans Bay", location_info);
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#location-get-search-by-query
 *
 * @info Name can contain spaces. These are encoded as "%20".
 * @info Response length is limited to one as to ensure only one location is
 * returned per query.
 *
 * @param name Name to query against.
 * @param location_info Location structure to populate.
 * @return The result status code provided by CURL.
 */
CURLcode WillyWeather_GetLocationByName(char *name,
                                        WW_Location_TypeDef *location_info) {

    if (WillyWeather_CheckAccess() == 1) return CURLE_AUTH_ERROR;

    // Method to handle spaces in location name (e.g. conver ' ' to '%20')
    int16_t new_name_size = 0;
    for (char *c = name; *c != '\0'; c++) {
        if (*c == ' ') {
            new_name_size += 2;
        }
        new_name_size++;
    }
    char *encoded_name = (char*)malloc((size_t)(new_name_size + 1));
    int16_t n = 0;
    for (int16_t en = 0; en < new_name_size; en++) {
        if (name[n] == ' ') {
            encoded_name[en] = '%';
            encoded_name[en + 1] = '2';
            encoded_name[en + 2] = '0';
            en += 2;
        } else {
            encoded_name[en] = name[n];
        }
        n++;
    }

    char url[WW_LOCATION_URL_BUF];
    snprintf(url, WW_LOCATION_URL_BUF,
             "https://api.willyweather.com.au/v2/%s/"
             "search.json?query=%s&limit=%d", WW_TOKEN, encoded_name, 1);

    log_info("Getting Willy Weather location information for %s\n", name);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    // Response is limited to 1 (makes it easier to parse automatically)
    if (response != NULL && cJSON_IsArray(response)) {
        cJSON *id = NULL;
        cJSON *location = NULL;
        cJSON *region = NULL;
        cJSON *state = NULL;
        cJSON *postcode = NULL;
        cJSON *latitude = NULL;
        cJSON *longitude = NULL;

        cJSON *item = NULL;
        cJSON_ArrayForEach(item, response) {

            // Location ID for queries
            id = cJSON_GetObjectItemCaseSensitive(item, "id");
            if (cJSON_IsNumber(id)) {
                location_info->id = (uint16_t)id->valueint;
            }

            // Location name
            location = cJSON_GetObjectItemCaseSensitive(item, "name");
            if (cJSON_IsString(location) && location->valuestring != NULL) {
                strncpy(location_info->location, location->valuestring,
                        WW_LOCATION_BUF);
            } else{
                strncpy(location_info->location, "N/a", WW_LOCATION_BUF);
            }

            // Location region
            region = cJSON_GetObjectItemCaseSensitive(item, "region");
            if (cJSON_IsString(region) && region->valuestring != NULL) {
                strncpy(location_info->region, region->valuestring,
                        WW_LOCATION_BUF);
            } else{
                strncpy(location_info->region, "N/a", WW_LOCATION_BUF);
            }

            // Location
            state = cJSON_GetObjectItemCaseSensitive(item, "state");
            if (cJSON_IsString(state) && state->valuestring != NULL) {
                strncpy(location_info->state, state->valuestring,
                        WW_LOCATION_BUF);
            } else{
                strncpy(location_info->state, "N/a", WW_LOCATION_BUF);
            }

            // Location postcode
            postcode = cJSON_GetObjectItemCaseSensitive(item, "postcode");
            if (cJSON_IsString(postcode) && postcode->valuestring != NULL) {
                strncpy(location_info->postcode, postcode->valuestring,
                        WW_LOCATION_BUF);
            } else {
                strncpy(location_info->postcode, "N/a", WW_LOCATION_BUF);
            }

            // Location latitude
            latitude = cJSON_GetObjectItemCaseSensitive(item, "lat");
            if (cJSON_IsNumber(latitude)) {
                location_info->latitude = latitude->valuedouble;
            }

            // Location longitude
            longitude = cJSON_GetObjectItemCaseSensitive(item, "lng");
            if (cJSON_IsNumber(longitude)) {
                location_info->longitude = longitude->valuedouble;
            }
        }
    }

    if (result == CURLE_OK) {
        log_info("Location request to Willy Weather was successful.\n");
        log_debug("Location ID: %hu, Name: %s, State: %s, Latitude: %lf, "
                  "Longitude: %lf\n",
                  location_info->id, location_info->location,
                  location_info->state,
                  location_info->latitude,
                  location_info->longitude);
    } else {
        log_error("Unable to process Willy Weather location"
                  "request. Error status: %d\n", result);
    }

    free(encoded_name);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
