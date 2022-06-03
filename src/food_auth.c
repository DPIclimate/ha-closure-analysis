#include "food_auth.h"

static void FoodAuth_FindHTMLValue(char* data, const char* search_term,
                                   char* value);

static void FoodAuth_ParseResponse(char* data,
                                   HarvestAreaStatus_TypeDef *ha_status);

static CJSON_PUBLIC(void) cJSON_Minify_Mod(char *json);

/**
 * Gets harvest area status information from NSW Food Authority.
 *
 * NSW Food Authority provides a JSON response with XML data as values.
 * The XML data basically contains generic cards that are used on the
 * NSW FA website. This function parses out the relevent XML data and used
 * helper functions to pull out a struct containing relevent data.
 *
 * @code
 *      HarvestAreaStatus_TypeDef ha_status;
 *      FoodAuth_GetHarvestAreaStatus("Moonlight", &ha_status);
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
CURLcode FoodAuth_GetHarvestAreaStatus(const char* harvest_name,
                                       HarvestAreaStatus_TypeDef *ha_status){

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
                FoodAuth_ParseResponse(data->valuestring, ha_status);
            } else {
                printf("Request succeded but data was not found.\n");
            }
        }
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
static void FoodAuth_ParseResponse(char* data,
                                   HarvestAreaStatus_TypeDef *ha_status){

    // Items to extract from request. The order of these items matters for the
    // below switch statement.
    const char* search_terms[] = {
            "-classification",
            "-status",
            "datetime=",
            "Reasons/Conditions</div><div",
            "Previous reasons/conditions</div><div"
    };

    char value[MAX_RESPONSE_BUFFER] = {0};
    for(int i = 0; i < sizeof(search_terms) / sizeof(*search_terms); i++){
        FoodAuth_FindHTMLValue(data, search_terms[i], value);
        if(strlen(value) == 0){
            printf("Value not found for: %s\n", search_terms[i]);
        } else{
            switch (i){
                case 0:
                    strcpy(ha_status->classification, value);
                    break;
                case 1:
                    strcpy(ha_status->status, value);
                    break;
                case 2:
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
static CJSON_PUBLIC(void) cJSON_Minify_Mod(char *json){
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
static void FoodAuth_FindHTMLValue(char* data, const char* search_term,
                                   char* value){

    char* clsf = strstr(data, search_term);
    if(clsf == NULL) {
        return; // Sub-string not found
    }

    int start_substr = 0;
    int value_index = 0;
    for(unsigned long i = strlen(search_term); i < strlen(clsf) &&
    i < MAX_RESPONSE_BUFFER; i++){
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
