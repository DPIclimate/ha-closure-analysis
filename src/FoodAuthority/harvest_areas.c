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

    int16_t index = 0;
    while(index < harvest_areas->count){
        FA_HarvestArea_TypeDef ha = harvest_areas->harvest_area[index];
        char query[3000];
        snprintf(query, sizeof(query), "INSERT INTO harvest_area ("
                                       "last_updated, program_name, location, "
                                       "name, classification, status, "
                                       "time_processed, status_reason, "
                                       "status_prev_reason) "
                                       "VALUES "
                                       "(NOW(), "   // Current time
                                       "'%s',"      // Program name
                                       "'%s',"      // Location
                                       "'%s',"      // Harvest area name
                                       "'%s',"      // Classification
                                       "'%s',"      // Status (open, closed etc.)
                                       "'%s',"      // Timestamptz
                                       "'%s',"      // Reason
                                       "'%s')"      // Previous reason
                                       "ON CONFLICT (time_processed, name, "
                                       "status) DO "
                                       "UPDATE SET last_updated = NOW();",
                 ha.program_name, ha.location, ha.name,
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

void FA_CreateLocationsLookupDB(PGconn* psql_conn){

    log_info("Writing locations lookup to PostgreSQL database\n");

    BOM_WeatherStations_TypeDef stations;
    BOM_LoadStationsFromTxt("tmp/bom_weather_stations.txt", &stations);

    const char* query = "SELECT DISTINCT program_name FROM harvest_area;";

    PGresult* res = PQexec(psql_conn, query);
    uint16_t index = 0;
    if(PQresultStatus(res) == PGRES_TUPLES_OK){
        int num_fields = PQnfields(res);
        for(int i = 0; i < PQntuples(res); i++){
            for(int j = 0; j < num_fields; j++){
                //FA_HarvestLookup_TypeDef ha_lookup = {0};
                char* location_name = PQgetvalue(res, i , j);
                WW_Location_TypeDef location_info = {0};
                if(strcmp(location_name, "Wapengo Lake") == 0){
                    char new_name[8] = "Wapengo";
                    WillyWeather_GetLocationByName(new_name, &location_info);
                }
                else if(strcmp(location_name, "Bellinger and Kalang "
                                              "Rivers") == 0){
                    char new_name[10] = "Bellinger";
                    WillyWeather_GetLocationByName(new_name, &location_info);
                }
                else if(strcmp(location_name, "Shoalhaven - "
                                              "Crookhaven Rivers") == 0){
                    char new_name[20] = "Crookhaven River";
                    WillyWeather_GetLocationByName(new_name, &location_info);
                }
                else{
                    WillyWeather_GetLocationByName(location_name, &location_info);
                }

                int cws = BOM_ClosestStationIndex(location_info.latitude,
                                                  location_info.longitude,
                                                  &stations);

                double distance = Utils_PointsDistance(location_info.latitude,
                                                       location_info.longitude,
                                                       stations.stations[cws].latitude,
                                                       stations.stations[cws].longitude);

                log_debug("%s\t Willy Weather: %s\t BOM: %s\t Distance: %0.2lf\n", location_name,
                          location_info.location, stations.stations[cws].name, distance);

                char i_query[3000];
                snprintf(i_query,
                         sizeof(i_query),
                         "INSERT INTO harvest_lookup (last_updated, "
                         "fa_program_name, ww_location, ww_location_id, "
                         "ww_latitude, ww_longitude, bom_location, "
                         "bom_location_id, bom_latitude, bom_longitude, "
                         "bom_distance) VALUES ("
                         "NOW(), " // Last updated
                         "'%s', " // program name
                         "'%s'," // ww location
                         "%d," // ww location id
                         "%f," // ww latitude
                         "%f, " // ww longitude
                         "'%s', " // bom location
                         "'%s'," // bom location id
                         "%f, " // bom latitude
                         "%f, " // bom longitude
                         "%f) "  // bom distance
                         "ON CONFLICT (fa_program_name) DO UPDATE SET "
                         "last_updated = NOW();",
                         location_name,
                         location_info.location,
                         location_info.id,
                         location_info.latitude,
                         location_info.longitude,
                         stations.stations[cws].name,
                         stations.stations[cws].id,
                         stations.stations[cws].latitude,
                         stations.stations[cws].longitude,
                         distance);
                PGresult* i_res = PQexec(psql_conn, i_query);
                if(PQresultStatus(i_res) != PGRES_COMMAND_OK){
                    log_error("PSQL command failed when entering station "
                              "information for %s. Error: %s\n", location_name,
                              PQerrorMessage(psql_conn));
                }

                index++;

                PQclear(i_res);

                if(index > WW_MAX_NUM_LOCATONS){
                    break;
                }
            }
        }
    }

    log_info("Harvest area location lookups written to PostgreSQL database\n");

    PQclear(res);
}

void FA_UniqueLocationsFromDB(T_LocationsLookup_TypeDef* locations,
                              PGconn* psql_conn){

    log_info("Getting unique oyster farming regions from PostgreSQL DB.\n",
             locations->count);

    const char* query = "SELECT * FROM harvest_lookup;";
    PGresult* res = PQexec(psql_conn, query);

    if(PQresultStatus(res) == PGRES_TUPLES_OK){
        int num_fields = PQnfields(res);
        int i = 0;
        for(; i < PQntuples(res); i++){
            if(i > T_MAX_N_LOCATIONS){
                log_error("Max locations allocation exceeded. Exiting.\n");
                break;
            }
            for(int j = 0; j < num_fields; j++){
                char* ptr;
                switch(j){
                    case 0:
                        strncpy(locations->locations[i].last_updated,
                                PQgetvalue(res, i, j), T_TIMESTAMP_SIZE);
                        break;
                    case 1:
                        strncpy(locations->locations[i].fa_program_name,
                                PQgetvalue(res, i, j), FA_MAX_BUFFER);
                        break;
                    case 2:
                        strncpy(locations->locations[i].ww_location,
                                PQgetvalue(res, i, j), WW_LOCATION_BUF);
                        break;
                    case 3:
                        strncpy(locations->locations[i].ww_location_id,
                                PQgetvalue(res, i, j), WW_LOCATION_BUF);
                        break;
                    case 4:
                        locations->locations[i].ww_latitude =
                                strtof(PQgetvalue(res, i, j), &ptr);
                        break;
                    case 5:
                        locations->locations[i].ww_longitude =
                                strtof(PQgetvalue(res, i, j), &ptr);
                        break;
                    case 6:
                        strncpy(locations->locations[i].bom_location,
                                PQgetvalue(res, i, j), BOM_STATION_NAME_SIZE);
                        break;
                    case 7:
                        strncpy(locations->locations[i].bom_location_id,
                                PQgetvalue(res, i, j), BOM_STATION_ID_SIZE);
                        break;
                    case 8:
                        locations->locations[i].bom_latitude =
                                strtof(PQgetvalue(res, i, j), &ptr);
                        break;
                    case 9:
                        locations->locations[i].bom_longitude =
                                strtof(PQgetvalue(res, i, j), &ptr);
                        break;
                    case 10:
                        locations->locations[i].bom_distance =
                                strtof(PQgetvalue(res, i, j), &ptr);
                        break;
                    default:
                        log_error("Unknown data field received.\n");
                        break;
                }
            }
        }
        locations->count = (uint16_t)i;
    }

    if(locations->count == 0){
        log_error("No locations found in harvest_lookup table.\n");
    } else {
        log_info("Got %d unique oyster farming regions from PostgreSQL DB.\n",
                 locations->count);
    }

    PQclear(res);
}
