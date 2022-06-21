#include "FoodAuthority/harvest_area.h"

/// Find value with HTML tags.
static void FA_FindHTMLValue(char *data, const char *search_term,
                             char *value);

/**
 * Gets harvest area status information from NSW Food Authority.
 *
 * NSW Food Authority provides a JSON response with XML data as values.
 * The XML data basically contains generic cards that are used on the
 * NSW FA website. This function parses out the relevent XML data and used
 * helper functions to pull out a struct containing relevent data.
 *
 * @code
 *      FA_HarvestAreaStatus_TypeDef harvest_area;
 *      FA_GetHarvestAreaStatus(FA_HA_CLYDE_MOONLIGHT, &harvest_area);
 * @endcode
 *
 * The havest area names for Batemans Bay are as follow:
 * - Waterfall
 * - Moonlight
 * - Rocky (representing Rocky Point)
 *
 * Other harvest areas can be checked using this function (just need to know
 * their name and use it as the harvest_name variable.
 *
 * @param harvest_name Name of harvest area.
 * @param harvest_area Harvest area struct to populate with status information.
 * @return Code representing CURL response status.
 */
CURLcode FA_GetHarvestAreaStatus(const char *harvest_name,
                                 FA_HarvestArea_TypeDef *harvest_area) {

    const char *URL = "https://www.foodauthority.nsw.gov.au/views/ajax";

    // Build body for POST request
    const char *BASE_FILTER = "filter=";
    const char *BASE_BODY = "&view_name=sqap_waterways&"
                            "view_display_id=page_1";
    char *req_body = malloc(strlen(BASE_FILTER) +
                            strlen(harvest_name) +
                            strlen(BASE_BODY) + 1);
    strcpy(req_body, BASE_FILTER);
    strcat(req_body, harvest_name);
    strcat(req_body, BASE_BODY);

    log_info("Requesting harvest area information for site %s "
             "from: %s\n", harvest_name, URL);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "authority: NSW DPI");
    headers = curl_slist_append(headers, "x-requested-with: XMLHttpRequest");
    headers = curl_slist_append(headers, "accept: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, URL, headers, 1, req_body);

    if (response != NULL) {
        cJSON *res = NULL;
        if (cJSON_IsArray(response)) {
            res = cJSON_GetArrayItem(response, 3);
            cJSON *data = NULL;
            data = cJSON_GetObjectItemCaseSensitive(res, "data");
            if (cJSON_IsString(data) && data->valuestring != NULL) {
                cJSON_Minify_Mod(data->valuestring); // Remove unessessary chars
                FA_ParseResponse(data->valuestring, harvest_area);
            } else {
                log_error("NSW Food Authority request was successful. But"
                          " no data was found.\n");
            }
        }
    }

    if (result == CURLE_OK) {
        log_info("NSW Food Authority request was successful.\n");
        log_debug("Location: %s, Status: %s, Time Updated: %s\n",
                  harvest_area->location, harvest_area->status, harvest_area->time);
    }

    free(req_body);
    curl_slist_free_all(headers);
    cJSON_Delete(response);
    return result;
}

/**
 * Takes a character array containing HTML data and formats a struct containing
 * relevent harvest area status information.
 *
 * Search terms are taken from the HTML input and the corresponding values
 * (i.e. the text between relevent tags) are added to the corresponding struct.
 *
 * @param data The string to parse.
 * @param harvest_area The struct to populate with values.
 */
void FA_ParseResponse(char *data,
                      FA_HarvestArea_TypeDef *harvest_area) {

    // Items to extract from request. The order of these items matters for the
    // below switch statement.
    const char *search_terms[] = {
            "sqap-program", // Program name
            "sqap-harvest-area", // Location (may include zone as well)
            "sqap-card__title", // Harvest area name
            "-classification", // Classification (Depuration, Water Drawing)
            "-status", // Open Close
            "datetime=", // Time updated
            "Reasons/Conditions</div><div", // Reason for status
            "Previous reasons/conditions</div><div" // Previous reason (if not NULL)
    };

    // Setup to hold time struct
    struct tm v_time; // Value time
    memset(&v_time, 0, sizeof(struct tm));

    char value[FA_MAX_BUFFER] = {0};
    for (size_t i = 0; i < sizeof(search_terms) / sizeof(*search_terms); i++) {
        FA_FindHTMLValue(data, search_terms[i], value);

        if (strlen(value) == 0) {
            strcpy(value, "N/A");
        }

        if (strlen(value) > FA_MAX_BUFFER) {
            log_error("Value: %s exceeds the set maximum buffer "
                      "size. Skipping.\n", value);
            memset(value, 0, sizeof(value));
            continue;
        }

        switch (i) {
            case 0:
                // Remove " SP" from last part of program name
                for (size_t x = 0; x < strlen(value) - 1; x++) {
                    if (value[x] == 'S' && value[x + 1] == 'P') {
                        if (x > 0) {
                            value[x - 1] = '\0';
                            break;
                        }
                    }
                }
                strncpy(harvest_area->program_name, value, FA_MAX_BUFFER);
                break;
            case 1:
                strncpy(harvest_area->location, value, FA_MAX_BUFFER);
                break;
            case 2:
                strncpy(harvest_area->name, value, FA_MAX_BUFFER);
                break;
            case 3:
                strncpy(harvest_area->classification, value, FA_MAX_BUFFER);
                break;
            case 4:
                strncpy(harvest_area->status, value, FA_MAX_BUFFER);
                break;
            case 5:
                /// Convert time to tm struct
                strptime(value, "%02d/%02m/%Y - %02H:%02M%p", &v_time);
                harvest_area->u_time = v_time;
                strncpy(harvest_area->time, value, FA_MAX_BUFFER);
                break;
            case 6:
                strncpy(harvest_area->reason, value, FA_MAX_BUFFER);
                break;
            case 7:
                strncpy(harvest_area->previous_reason, value, FA_MAX_BUFFER);
                break;
            default:
                log_error("Unknown harvest area option.\n");
                break;
        }
        memset(value, 0, sizeof(value));
    }
}

/**
 * Extracts the value (item between HTML tags) from a HTML tag.
 *
 * This function takes a search term that is unique to a particular HTML tag
 * and searchers for the tags value. The value is then added to a provided
 * char* (value) and returns.
 *
 * @note The search term can contain items from the previous HTML tag if the
 * required tag doesn't have unique information.
 *
 * @param data
 * @param search_term
 * @param value
 */
static void FA_FindHTMLValue(char *data, const char *search_term,
                             char *value) {

    char *clsf = strstr(data, search_term);
    if (clsf == NULL) {
        return; // Sub-string not found
    }

    int start_substr = 0;
    int value_index = 0;
    for (unsigned long i = strlen(search_term); i < strlen(clsf) &&
                                                i < FA_MAX_BUFFER; i++) {
        if (clsf[i] == '>') {
            start_substr = 1;
            continue;
        }
        if (start_substr == 1) {
            if (clsf[i] == '<') {
                return;
            }
            value[value_index++] = clsf[i];
        }
    }
}
