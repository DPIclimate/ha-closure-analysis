#include "FoodAuthority/harvest_area.h"

/// Find value with HTML tags.
static void FA_FindHTMLValue(char* data, const char* search_term,
                                   char* value);

/// Parse HTML data to populate status information data.
static void FA_ParseResponse(char* data,
                                   FA_HarvestAreaStatus_TypeDef *ha_status);

/// Modified minify function from cJSON
static void cJSON_Minify_Mod(char *json);

/**
 * Gets harvest area status information from NSW Food Authority.
 *
 * NSW Food Authority provides a JSON response with XML data as values.
 * The XML data basically contains generic cards that are used on the
 * NSW FA website. This function parses out the relevent XML data and used
 * helper functions to pull out a struct containing relevent data.
 *
 * @code
 *      FA_HarvestAreaStatus_TypeDef ha_status;
 *      FA_GetHarvestAreaStatus(FA_HA_CLYDE_MOONLIGHT, &ha_status);
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
 * @param ha_status Harvest area struct to populate with status information.
 * @return Code representing CURL response status.
 */
CURLcode FA_GetHarvestAreaStatus(const char* harvest_name,
                                       FA_HarvestAreaStatus_TypeDef *ha_status){

    strcpy(ha_status->location, harvest_name);

    const char* URL = "https://www.foodauthority.nsw.gov.au/views/ajax";

    // Build body for POST request
    const char* BASE_FILTER = "filter=";
    const char* BASE_BODY = "&view_name=sqap_waterways&"
                            "view_display_id=page_1";
    char* req_body = malloc(strlen(BASE_FILTER) +
                            strlen(harvest_name) +
                            strlen(BASE_BODY) + 1);
    strcpy(req_body, BASE_FILTER);
    strcat(req_body, harvest_name);
    strcat(req_body, BASE_BODY);

    log_info("Requesting harvest area information for site %s "
            "from: %s\n", harvest_name, URL);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "authority: "
                                         "www.foodauthority.nsw.gov.au");
    headers = curl_slist_append(headers, "x-requested-with: XMLHttpRequest");
    headers = curl_slist_append(headers, "accept: application/json");

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, URL, headers, 1, req_body);

    if(response != NULL){
        cJSON* res = NULL;
        if(cJSON_IsArray(response)){
            res = cJSON_GetArrayItem(response, 3);
            cJSON* data = NULL;
            data = cJSON_GetObjectItemCaseSensitive(res, "data");
            if(cJSON_IsString(data) && data->valuestring != NULL){
                cJSON_Minify_Mod(data->valuestring); // Remove unessessary chars
                FA_ParseResponse(data->valuestring, ha_status);
            } else {
                log_error("NSW Food Authority request was successful. But"
                          " no data was found.\n");
            }
        }
    }

    if(result == CURLE_OK){
        log_info("NSW Food authority request was successful.\n"
                 "\tLocation:\t\t%s\n"
                 "\tClassification:\t\t%s\n"
                 "\tStatus:\t\t\t%s\n"
                 "\tTime Updated:\t\t%s\n"
                 "\tReason:\t\t\t%s\n"
                 "\tPrevious Reason:\t%s\n",
                 ha_status->location, ha_status->classification,
                 ha_status->status, ha_status->time, ha_status->reason,
                 ha_status->previous_reason);
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
 * @param ha_status The struct to populate with values.
 */
static void FA_ParseResponse(char* data,
                                   FA_HarvestAreaStatus_TypeDef *ha_status){

    log_info("Parsing JSON response from NSW Food Authority.\n");

    // Items to extract from request. The order of these items matters for the
    // below switch statement.
    const char* search_terms[] = {
            "-classification",
            "-status",
            "datetime=",
            "Reasons/Conditions</div><div",
            "Previous reasons/conditions</div><div"
    };

    // Setup to hold time struct
    struct tm v_time; // Value time
    memset(&v_time, 0, sizeof(struct tm));

    char value[FA_MAX_BUFFER] = {0};
    for(int i = 0; i < sizeof(search_terms) / sizeof(*search_terms); i++){
        FA_FindHTMLValue(data, search_terms[i], value);

        if(strlen(value) == 0){
            strcpy(value, "N/A");
        }

        if(strlen(value) > FA_MAX_BUFFER){
            log_error("Value: %s exceeds the set maxiumu  buffer"
                      " size. Skipping.\n", value);
            memset(value, 0, sizeof(value));
            continue;
        }

        switch (i){
            case 0:
                strcpy(ha_status->classification, value);
                break;
            case 1:
                strcpy(ha_status->status, value);
                break;
            case 2:
                /// Convert time to tm struct
                strptime(value, "%02d/%02m/%Y - %02H:%02M%p", &v_time);
                ha_status->u_time = v_time;
                strcpy(ha_status->time, value);
                break;
            case 3:
                strcpy(ha_status->reason, value);
                break;
            case 4:
                strcpy(ha_status->previous_reason, value);
                break;
            default:
                printf("Unknown harvest area option\n");
                break;
        }
        memset(value, 0, sizeof(value));
    }
}

/**
 * HTML minify function modified from cJSON library.
 *
 * The main cJSON_Minify() function gets rid of all white space and '/'
 * characters in a string. This however, causes sentances to appear without
 * the nessessary spaces between words. This modified function keeps only
 * one space between each character (if more than one space occurs) and
 * maintains the spaces between words in a sentance. Additionally '/'
 * characters are maintained as they are used as datetime delimiters.
 *
 * @see https://github.com/DaveGamble/cJSON/blob/master/cJSON.c#L2838
 *
 * @param json The string to minify (parse).
 */
static void cJSON_Minify_Mod(char *json){
    char *into = json;

    if (json == NULL){
        return;
    }

    char prev_char;
    while (json[0] != '\0'){
        switch (json[0]){
            case ' ':
                if (prev_char != ' ' && prev_char != '\n'){
                    into[0] = json[0];
                    into++;
                } else {
                    json++;
                }
                prev_char = json[0];
                break;
            case '\t':
            case '\r':
            case '\n':
                prev_char = json[0];
                json++;
                break;
            default:
                prev_char = json[0];
                into[0] = json[0];
                json++;
                into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
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
static void FA_FindHTMLValue(char* data, const char* search_term,
                                   char* value){

    char* clsf = strstr(data, search_term);
    if(clsf == NULL) {
        log_error("No value found for: %s\n", search_term);
        return; // Sub-string not found
    }

    int start_substr = 0;
    int value_index = 0;
    for(unsigned long i = strlen(search_term); i < strlen(clsf) &&
    i < FA_MAX_BUFFER; i++){
        if(clsf[i] == '>'){
            start_substr = 1;
            continue;
        }
        if(start_substr == 1){
            if(clsf[i] == '<') {
                return;
            }
            value[value_index++] = clsf[i];
        }
    }
}
