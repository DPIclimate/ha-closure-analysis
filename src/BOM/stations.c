#include "BOM/stations.h"

/// Parse .txt file into a BOM_Stations struct
static int8_t BOM_ParseStations(ReqData_TypeDef *stream,
                                BOM_WeatherStations_TypeDef *stations);

/// Calcualte the distance between a point and the closes BOM weather station
static double StationDistance(double latitude,
                              double longitude,
                              double station_latitude,
                              double station_longitude);

CURLcode BOM_GetWeatherStations(BOM_WeatherStations_TypeDef *stations) {

    log_info("Getting weather station information from "
             "Bureau of Meterology FTP Server.\n");

    ReqData_TypeDef stream;
    CURLcode result = FTPRequest(BOM_FTP_STATIONS_URL, &stream);

    if (result == CURLE_OK) {
        BOM_ParseStations(&stream, stations);
    } else {
        log_error("Unable to get locations from "
                  "Bureau of Meterology FTP Server.\n");
    }

    free(stream.memory);
    return result;
}

static int8_t BOM_ParseStations(ReqData_TypeDef *stream,
                                BOM_WeatherStations_TypeDef *stations) {
    MakeDirectory("tmp");
    const char *filename = "tmp/bom_weather_stations.txt";
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        fputs(stream->memory, file);
        fclose(file);
    } else {
        log_error("Unable to write BOM locations data to file.\n");
        return -1;
    }

    BOM_LoadStationsFromTxt(filename, stations);

    return 0;
}

int8_t BOM_LoadStationsFromTxt(const char *filename,
                               BOM_WeatherStations_TypeDef *stations) {

    log_info("Loading BOM weather stations from file: %s\n", filename);

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        log_error("Unable to open BOM weather stations file: %s\n", filename);
        return -1;
    }

    char buffer[200];
    char lat_buf[10], lng_buf[10];
    char id_buf[BOM_STATION_ID_SIZE];
    char state_buf[BOM_STATION_STATE_SIZE];
    char name_buf[BOM_STATION_NAME_SIZE];
    char *ptr;

    int16_t index = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL
           && index < BOM_STATION_MAX_RESPONSES) {
        sscanf(buffer, "%6[^ ] %3[^ ] %*[^ ] %99[a-z A-Z()] %*[^ ] %9[^ ] %9[^ ] \n",
               id_buf, state_buf, name_buf, lat_buf, lng_buf);

        // Only parse stations in NSW
        if (strstr("NSW", state_buf)) {
            // Build a station
            BOM_WeatherStation_TypeDef station = {0};

            // Remove trailing whitespace from station name
            for (int16_t i = 0; i < BOM_STATION_NAME_SIZE - 2; i++) {
                if (name_buf[i] == ' ' && (name_buf[i + 1] == '\0' ||
                                           name_buf[i + 1] == ' ')) {
                    name_buf[i] = '\0';
                    break;
                }
            }

            // Append information to weather station
            strcpy(station.id, id_buf);
            strcpy(station.state, state_buf);
            strcpy(station.name, name_buf);
            station.latitude = strtod(lat_buf, &ptr);
            station.longitude = strtod(lng_buf, &ptr);

            int8_t opening_bracket = 0;
            // Format name as a filename (lowercase, '_' replaces ' ', no "AWS")
            for (int16_t i = 0; i < BOM_STATION_NAME_SIZE - 3; i++) {
                // Check if opening bracket occurs before AWS instance
                if (name_buf[i] == '(') {
                    opening_bracket = 1;
                }

                // Remove AWS or AW
                if (i > 0) {
                    if (name_buf[i] == 'A' && name_buf[i + 1] == 'W') {
                        // AWS
                        if (name_buf[i + 2] == 'S') {
                            if (opening_bracket != 0) {
                                name_buf[i - 1] = ')';
                                name_buf[i] = '\0';
                            } else {
                                name_buf[i - 1] = '\0';
                            }
                            break;
                        } else {
                            // Handles "PORT MAQUARIE (PORT MACQUARIE AIRPORT AW"
                            // Converts to "PORT MAQUARIE (PORT MACQUARIE AIRPORT AW)"
                            // Which is expected by the BOM...
                            if (opening_bracket != 0) {
                                name_buf[i + 2] = ')';
                                name_buf[i + 3] = '\0';
                            }
                        }
                    }
                }

                // Convert to lowercase
                if (isalpha(name_buf[i])) {
                    if (name_buf[i] >= 65 && name_buf[i] <= 90) {
                        name_buf[i] = (char) (name_buf[i] + 32);
                    }
                }

                // Convert space to underscore
                if (name_buf[i] == ' ') {
                    name_buf[i] = '_';
                }
            }
            // Add formatted filename to weather station
            strcpy(station.filename, name_buf);

            // Add station to list of stations
            stations->stations[index] = station;

            index++;
        }
    }
    fclose(file);

    // Number of weather stations populated
    stations->count = --index;

    if (stations->count == 0) {
        log_error("BOM weather stations could not be loaded.\n");
        return -1;
    }

    log_info("%d BOM weather stations loaded.\n", stations->count);

    return 0;
}


int16_t BOM_ClosestStationIndex(double latitude,
                                double longitude,
                                BOM_WeatherStations_TypeDef *stations) {

    int16_t closest_satation_index = 0;
    double min_distance = 0;
    for (int16_t i = 0; i < stations->count; i++) {
        double distance = StationDistance(latitude, longitude,
                                          stations->stations[i].latitude,
                                          stations->stations[i].longitude);
        if (i == 0 || distance < min_distance) {
            closest_satation_index = i;
            min_distance = distance;
        }
    }
    log_debug("Closest BOM weather station: %s (%0.2lf km)\n",
              stations->stations[closest_satation_index].name,
              min_distance);

    return closest_satation_index;
}

/**
 * Uses haversine formula to calculate the distance in km between two
 * points on earth.
 *
 * Used to calculate the distance between an AOI and the closest Bureau of
 * Meterology station.
 *
 * @param latitude Latitude of point of interest.
 * @param longitude Longitude of point of interest.
 * @param station_latitude Latitude of station.
 * @param station_longitude Longitude of station.
 * @return Distance between two points on earth in km.
 */
static double StationDistance(double latitude,
                              double longitude,
                              double station_latitude,
                              double station_longitude) {
    double p = M_PI / 180;
    return 12742.0 * asin(sqrt(
            (0.5 - (cos((latitude - station_latitude) * p) / 2.0) +
             (cos(station_latitude * p) * cos(latitude * p)) *
             ((1 - cos((longitude - station_longitude) * p)) / 2.0))));
}