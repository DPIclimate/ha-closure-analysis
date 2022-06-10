#include "Ubidots/devices.h"

/**
 * Method to get a list of all devices on Ubidots.
 *
 * Page size should be set high enough to get all devices (default 500).
 *
 * @code
 * Ubidots_Devices_TypeDef devices = {0};
 * Ubidots_ListDevices(&devices);
 * @endcode
 *
 * @see https://docs.ubidots.com/reference/get-all-devices
 * @info Device names need to be less than 100 characters.
 *
 * @param token Ubidots API token to use.
 * @return Status code respresenting response status.
 */
CURLcode Ubidots_ListDevices(Ubidots_Devices_TypeDef *devices){

    // Check authentication / authenticates if not already done
    if(Ubidots_CheckAccess() != 0) return CURLE_AUTH_ERROR;

    const char *URL = "https://industrial.api.ubidots.com.au"
                      "/api/v2.0/devices/?page_size=500";

    fprintf(stdout, "[Info]: Creating ubidots device list. URL: %s\n", URL);

    // Add headers, remember to free these later
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    const char* auth_key = "X-Auth-Token: ";
    char *auth = malloc(strlen(auth_key) + strlen(UBIDOTS_TOKEN) + 1);
    strcpy(auth, auth_key);
    strcat(auth, UBIDOTS_TOKEN);

    headers = curl_slist_append(headers, auth);

    cJSON *response = NULL;
    CURLcode result = HttpRequest(&response, URL, headers, 0, NULL);

    if(response != NULL){
        cJSON* count = NULL;
        cJSON* results = NULL;
        cJSON* device = NULL;
        cJSON* name = NULL;
        cJSON* latitude = NULL;
        cJSON* longitude = NULL;
        cJSON* id = NULL;

        int16_t index = 0;
        count = cJSON_GetObjectItemCaseSensitive(response, "count");
        if(count != NULL){
            if(cJSON_IsNumber(count)){
                devices->count = floor(count->valuedouble);
            }
        }

        // Get a list of devices
        results = cJSON_GetObjectItemCaseSensitive(response, "results");
        if(cJSON_IsArray(results)){
            cJSON_ArrayForEach(device, results){

                // Extract name
                name = cJSON_GetObjectItemCaseSensitive(device, "name");
                if(cJSON_IsString(name) && name->valuestring != NULL
                && strlen(name->valuestring) < UBIDOTS_MAX_INFO_LENGTH){
                    strncpy(devices->names[index], name->valuestring,
                            UBIDOTS_MAX_INFO_LENGTH);
                } else {
                    strncpy(devices->names[index], "unknown",
                            UBIDOTS_MAX_INFO_LENGTH);
                }

                // Extract device latitude and longitude
                cJSON* props = NULL;
                props = cJSON_GetObjectItemCaseSensitive(device, "properties");
                if(props != NULL){
                    cJSON* loc = NULL;
                    loc = cJSON_GetObjectItemCaseSensitive(props, "_location_fixed");
                    if(loc != NULL){
                        latitude = cJSON_GetObjectItemCaseSensitive(loc, "lat");
                        if(cJSON_IsNumber(latitude)){
                            devices->latitudes[index] = latitude->valuedouble;
                        } else {
                            devices->latitudes[index] = 0;
                        }
                        longitude = cJSON_GetObjectItemCaseSensitive(loc, "lng");
                        if(cJSON_IsNumber(longitude)){
                            devices->longitudes[index] = longitude->valuedouble;
                        } else {
                            devices->longitudes[index] = 0;
                        }
                    } else {
                        devices->longitudes[index] = 0;
                        devices->latitudes[index] = 0;
                    }
                }

                // Device identitifier
                id = cJSON_GetObjectItemCaseSensitive(device, "id");
                if(cJSON_IsString(id) && id->valuestring != NULL){
                    strncpy(devices->ids[index], id->valuestring,
                            UBIDOTS_MAX_INFO_LENGTH);
                } else {
                    strncpy(devices->ids[index], "unknown",
                            UBIDOTS_MAX_INFO_LENGTH);
                }

                index++;
            }
        }
    }

    if(result == CURLE_OK){
        fprintf(stdout, "[Info]: Ubidots devices list succesfully created.\n"
                        "\tTotal number of devices: %d\n", devices->count);
    } else {
        fprintf(stderr, "[Error]: Ubidots devices list could not be "
                        "created.\n");
    }

    free(auth);
    curl_slist_free_all(headers);
    cJSON_Delete(response);

    return result;
}

/**
 * Writes out device list from ubidots to a .csv file.
 *
 * Device list containing names, ids and coordinates (if available) are written
 * to a .csv file. The devices count is used to terminate this loop. Errors (1)
 * are returned if directories cannot be created.
 *
 * @param devices Devices struct to write to file.
 * @return Error codes. 0 = OK ... 1 = ERROR
 */
int8_t Ubidots_DevicesToCSV(Ubidots_Devices_TypeDef *devices){
    if(MakeDirectory("datasets") != 0) return 1;
    if(MakeDirectory("datasets/ubidots") != 0) return 1;
    char* filename ="datasets/ubidots/devices.csv";

    FILE* file = fopen(filename, "w+");
    fprintf(file, "Device Name;Device ID;Latitude;Longitude\n");

    int16_t index = 0;
    while(index < devices->count){
        fprintf(file, "%s;%s;%lf;%lf\n",
                devices->names[index],
                devices->ids[index],
                devices->latitudes[index],
                devices->longitudes[index]);
        index++;
    }
    fclose(file);

    return 0;
}
