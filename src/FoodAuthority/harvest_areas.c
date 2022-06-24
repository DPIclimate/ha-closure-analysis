#include "FoodAuthority/harvest_areas.h"

static int8_t FA_ParseListResponse(char *data,
                                   FA_HarvestAreas_TypeDef *harvest_areas);

/**
 * Request list of harvest areas (and status) from NSW Food Authority.
 *
 * End product of this function is a list of harvest areas with their status
 * (open, closed etc.).
 *
 * @param harvest_areas List of harvest areas to populate.
 * @return CURL error code.
 */
CURLcode FA_GetHarvestAreas(FA_HarvestAreas_TypeDef *harvest_areas) {

    log_info("Requesting list of harvest areas.\n");

    const char *URL = "https://www.foodauthority.nsw.gov.au/views/ajax";
    const char *req_body = "view_name=sqap_waterways&view_display_id=page_1";

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
                FA_ParseListResponse(data->valuestring, harvest_areas);
            } else {
                log_error("NSW Food Authority request was successful. But"
                          " no data was found.\n");
            }
        }
    }

    if (result == CURLE_OK) {
        log_info("NSW Food Authority request was successful.\n");
    } else {
        log_error("NSW Food Authority request failed. Check request.\n");
    }

    curl_slist_free_all(headers);
    cJSON_Delete(response);
    return result;
}

/**
 * Takes a raw text XML input of all harvest areas across NSW from
 * NSW Food Authority and converts the text into a list of harvest
 * area objects.
 *
 * NSW Food Authority provides data regarding harvest area status as a raw
 * XML endpoint. First the string "-row" is identified as this divides the raw
 * text into each individual harvest areas. Then the '-' in "-row" is replaced
 * with a newline character. This results in each harvest area being put on
 * seperate lines. These data are saved to a file and then read from the same
 * file using fgets() to get results line by line. These lines are then passed
 * individually to FA_ParseResponse() to construct FA_HarvestArea_TypeDef's.
 *
 * @param data Raw XML text from NSW FA JSON request.
 * @param harvest_areas List of harvest areas.
 * @return Integer representing error or ok.
 */
static int8_t FA_ParseListResponse(char *data,
                                   FA_HarvestAreas_TypeDef *harvest_areas) {

    log_info("Parsing JSON response from NSW Food Authority.\n");

    // Match "-row" to divide harvest areas into lines (with '\n')
    for (size_t i = 0; i < strlen(data) - 5; i++) {
        if (i > 0) {
            if (data[i] == '-' && data[i + 1] == 'r' && data[i + 2] == 'o' &&
                data[i + 3] == 'w') {
                data[i] = '\n';
            }
        }
    }

    // Save raw response to a file
    MakeDirectory("tmp");
    FILE *file = fopen("tmp/harvest_areas.txt", "w");
    if (file == NULL) {
        log_error("Write error, unable to open file.");
        return -1;
    }
    fputs(data, file);
    fclose(file);

    // Read raw response from file
    FILE *read_file = fopen("tmp/harvest_areas.txt", "r");
    if (read_file == NULL) {
        log_error("Read error, unable to open file.");
        return -1;
    }

    // Construct list of harvest areas
    char buffer[2000];
    uint16_t index = 0;
    while (fgets(buffer, sizeof(buffer), read_file) != NULL &&
           index < FA_MAX_NUMBER_HARVEST_AREAS) {

        FA_HarvestArea_TypeDef harvest_area = {0};
        FA_ParseResponse(buffer, &harvest_area);
        if (strcmp(harvest_area.name, "N/A") != 0) {
            harvest_areas->harvest_area[index] = harvest_area;
            index++;
        };

    }

    // Set count
    harvest_areas->count = index;

    if (harvest_areas->count != 0) {
        log_info("%d harvest areas identified.\n", harvest_areas->count);
    } else {
        log_error("No harvest areas found. Check request.\n");
    }

    fclose(read_file);
    return 0;
}


int8_t FA_HarvestAreasToCSV(FA_HarvestAreas_TypeDef* harvest_areas){

    MakeDirectory("datasets");
    MakeDirectory(FA_DEFAULT_DIRECTORY);

    FILE* file = fopen(FA_DEFAULT_FILENAME, "w");
    if(file == NULL){
        return -1;
    }

    fprintf(file, "Program Name;Location;Name;Classification;Status;Time;Unix;"
                  "Reason;Previous Reason\n");

    uint16_t i = 0;
    while(i < harvest_areas->count){
        FA_HarvestArea_TypeDef ha = harvest_areas->harvest_area[i];
        fprintf(file, "%s;%s;%s;%s;%s;%s;%ld;%s;%s\n",
                ha.program_name, ha.location, ha.name, ha.classification,
                ha.status, ha.time, mktime(&ha.u_time), ha.reason,
                ha.previous_reason);
        i++;
    }
    fclose(file);

    return 0;
}

/**
 * Takes a list of harvest area statuses and passess them into a PSQL table.
 *
 * The current time is taken to reference all values against. Ideally when
 * querying these data the program should sort by the "time_processed" and this
 * will provide a historical look at harvest area status at a particular site.
 *
 * @param harvest_areas Basically a list of harvest areas.
 * @param psql_conn A connection handler for PostrgreSQL.
 */
void FA_HarvestAreasToDB(FA_HarvestAreas_TypeDef* harvest_areas,
                           PGconn* psql_conn){

    log_info("Inserting harvest areas status into PostgreSQL database.\n");

    // Get current time
    struct tm ctm = *localtime(&(time_t){time(NULL)});
    char current_time[30];
    strftime(current_time, sizeof(current_time),
             "%Y-%m-%d %H:%M:%S%z", &ctm);

    char query[3000];
    int16_t index = 0;
    while(index < harvest_areas->count){
        FA_HarvestArea_TypeDef ha = harvest_areas->harvest_area[index];
        snprintf(query, sizeof(query), "INSERT INTO harvest_area ("
                                       "time_processed, program_name, location, "
                                       "name, classification, status, "
                                       "last_updated, status_reason, "
                                       "status_prev_reason) "
                                       "VALUES "
                                       "('%s'," // Current time
                                       "'%s'," // Program name
                                       "'%s'," // Location
                                       "'%s'," // Harvest area name
                                       "'%s'," // Classification
                                       "'%s'," // Status (open, closed etc.)
                                       "'%s'," // Timestamptz
                                       "'%s'," // Reason
                                       "'%s')", // Previous reason
                 current_time, ha.program_name, ha.location, ha.name,
                 ha.classification, ha.status, ha.time, ha.reason,
                 ha.previous_reason);

        PGresult* res = PQexec(psql_conn, query);
        if(PQresultStatus(res) != PGRES_COMMAND_OK){
            log_error("PSQL command failed when entering %s harvest area "
                      "information. Error: %s\n", ha.name,
                      PQerrorMessage(psql_conn));
        }
        PQclear(res);
        index++;
    }

    log_info("Done inserting harvest area statuses into "
             "PostgreSQL database.\n");
}
