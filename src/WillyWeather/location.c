#include "WillyWeather/location.h"

/**
 * Get location ID from Willy Weather.
 *
 * Willy Weather forecast requires a location ID. This is obtained by querying
 * by name (e.g. Batemans) -> no spaces are allowed. The response contains
 * cJSON data of which multiple sites can be obatined (limited through q_limit).
 *
 * @code
 * WW_Location_TypeDef *location_info;
 * WillyWeather_GetLocationByName("Batemans", location_info);
 * @endcode
 *
 * @see
 * https://www.willyweather.com.au/api/docs/v2.html#location-get-search-by-query
 *
 * @warning Must not contain spaces in supplied query name.
 * @info Response length is limited to one as to ensure only one location is
 * returned per query.
 *
 * @param name Name to query against (must not contain spaces!).
 * @param location_info Location structure to populate.
 * @return The result status code provided by CURL.
 */
CURLcode WillyWeather_GetLocationByName(const char *name,
                                        WW_Location_TypeDef *location_info){

    if(WillyWeather_CheckAccess() == 1) return CURLE_AUTH_ERROR;

    char url[WW_LOCATION_URL_BUF];
    snprintf(url, WW_LOCATION_URL_BUF, "https://api.willyweather.com.au/v2/%s/"
                                       "search.json?query=%s&limit=%d", WW_TOKEN,
             name, 1);

    log_info("Getting Willy Weather location information for %s\n", name);

    struct curl_slist *headers= NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, url, headers, 0, NULL);

    // Response is limited to 1 (makes it easier to parse automatically)
    if(response != NULL && cJSON_IsArray(response)){
        cJSON* id = NULL;
        cJSON* location = NULL;
        cJSON* region = NULL;
        cJSON* state = NULL;
        cJSON* postcode = NULL;
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
                strncpy(location_info->location, location->valuestring,
                        WW_LOCATION_BUF);
            } else strncpy(location_info->location, "N/a", WW_LOCATION_BUF);

            // Location region
            region = cJSON_GetObjectItemCaseSensitive(item, "region");
            if(cJSON_IsString(region) && region->valuestring != NULL){
                strncpy(location_info->region, region->valuestring,
                        WW_LOCATION_BUF);
            } else strncpy(location_info->region, "N/a", WW_LOCATION_BUF);

            // Location
            state = cJSON_GetObjectItemCaseSensitive(item, "state");
            if(cJSON_IsString(state) && state->valuestring != NULL){
                strncpy(location_info->state, state->valuestring,
                        WW_LOCATION_BUF);
            } else strncpy(location_info->state, "N/a", WW_LOCATION_BUF);

            // Location postcode
            postcode = cJSON_GetObjectItemCaseSensitive(item, "postcode");
            if(cJSON_IsString(postcode) && postcode->valuestring != NULL){
                strncpy(location_info->postcode, postcode->valuestring,
                        WW_LOCATION_BUF);
            } else strncpy(location_info->postcode, "N/a", WW_LOCATION_BUF);

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
        log_info("Location request to Willy Weather was successful.\n"
                        "Location ID:\t\t%hu\n"
                        "Name:\t\t\t%s\n"
                        "Region:\t\t\t%s\n"
                        "State:\t\t\t%s\n"
                        "Postcode:\t\t%s\n"
                        "Coordinates:\n"
                        "\tLatitude:\t%lf\n"
                        "\tLongitude:\t%lf\n",
                location_info->id, location_info->location,
                location_info->region, location_info->state,
                location_info->postcode, location_info->latitude,
                location_info->longitude);
    } else {
        log_error("Unable to process Willy Weather location"
                  "request. Error status: %d\n", result);
    }

    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}
