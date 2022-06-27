#include "BOM/historical_weather.h"

static int8_t BOM_ParseWeather(Utils_ReqData_TypeDef *stream,
                               BOM_WeatherDataset_TypeDef *dataset,
                               BOM_WeatherStation_TypeDef *station);

/**
 * Get a file from the Bureau of Meterology FTP server.
 *
 * The BOM provides formatted .csv files for certain locations in Australia.
 * These .csv files can be downloaded from the BOM's FTP server. Currently,
 * it is unknown when the data is updated (sometime daily).
 *
 * @see http://www.bom.gov.au/catalogue/anon-ftp.shtml
 *
 * @param dataset Timeseries dataset to populate with data.
 * @param filename Station filename obtained from BOM_Stations_TypeDef.
 * @param year_month Year and month of interest e.g. 202206 for 2022 06 (June)
 * @return Returns cURL status code.
 */
CURLcode BOM_GetWeather(BOM_WeatherDataset_TypeDef *dataset,
                        BOM_WeatherStation_TypeDef *station,
                        const char *year_month) {

    // Build URL from filename and year_month
    const uint8_t URL_SIZE = 250;
    char URL[URL_SIZE];
    snprintf(URL, URL_SIZE, "ftp://ftp.bom.gov.au/anon/gen/clim_data/"
                            "IDCKWCDEA0/tables/nsw/%s/%s-%s.csv",
             station->filename,
             station->filename,
             year_month);

    log_info("Getting location data from BOM FTP server in directory: %s\n",
             URL);

    Utils_ReqData_TypeDef stream;
    CURLcode result = FTPRequest(URL, &stream);

    if (result == CURLE_OK) {
        BOM_ParseWeather(&stream, dataset, station);
    } else {
        log_error("Unable to get dataset from "
                  "Bureau of Meterology FTP Server.\n");
    }

    free(stream.memory);
    return result;
}

/**
 * Saves a downloaded .csv dataset to a file. Also parses the data into a BOM
 * dataset struct.
 *
 * @param stream Contains raw string with .csv file.
 * @param dataset Dataset to populate.
 * @return Status code OK = 0 ... ERROR = -1
 */
static int8_t BOM_ParseWeather(Utils_ReqData_TypeDef *stream,
                               BOM_WeatherDataset_TypeDef *dataset,
                               BOM_WeatherStation_TypeDef *station) {

    // Construct tempory file from raw stream
    MakeDirectory("datasets");
    MakeDirectory("datasets/bom");
    MakeDirectory("datasets/bom/historical");

    size_t full_filename_size = (size_t)(BOM_STATION_FILENAME_SIZE * 2);
    char filename[full_filename_size];
    snprintf(filename, full_filename_size,
             "datasets/bom/historical/%s.csv",
             station->filename);
    FILE *write_file = fopen(filename, "w");
    if (write_file != NULL) {
        fputs(stream->memory, write_file);
        fclose(write_file);
    } else {
        log_error("Unable to write BOM weather data to file.\n");
        return -1;
    }

    BOM_LoadWeatherFromCSV(filename, dataset, station);

    return 0;
}

/**
 * Load a BOM dataset from a cached .csv file.
 *
 * @param filename File to load (includes path).
 * @param dataset Dataset to populate.
 * @return Status code.
 */
int8_t BOM_LoadWeatherFromCSV(const char *filename,
                              BOM_WeatherDataset_TypeDef *dataset,
                              BOM_WeatherStation_TypeDef *station) {

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        log_error("Unable to open file: %s\n", filename);
        return -1;
    }

    // Convert filename to uppercase location (could be done better)
    // E.g. moruya_airport -> MORUYA AIRPORT
    char location[BOM_STATION_FILENAME_SIZE];
    for (int16_t i = 0; i < BOM_STATION_FILENAME_SIZE; i++) {
        if (station->filename[i] == '\0') break;

        // Conver to uppercase
        if (isalpha(station->filename[i])) {
            location[i] = (char) toupper(station->filename[i]);
        } else {
            if (station->filename[i] == '_') {
                location[i] = ' ';
            } else {
                location[i] = station->filename[i];
            }
        }
    }

    // Extract location, timestamp, rainfall, max temp, min temp
    char buffer[500];
    char loc[BOM_STATION_NAME_SIZE]; // Location name
    char ts[11]; // Timestamp of values
    char precip_buf[8]; // Daily precipitaion
    char max_t_buf[8]; // Daily max temperature
    char min_t_buf[8]; // Daily min temperature
    char *ptr;

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        sscanf(buffer, "%99[^,],%10[^,],%*[^,],%7[^,], "
                       ",%7[^,],%7[^,],%*[^\n]\n",
               loc, ts, precip_buf, max_t_buf, min_t_buf);

        // Check if timestamp is correct (then assuming rest is correct...)
        if (strncmp(loc, location, BOM_STATION_FILENAME_SIZE) == 0) {
            if (dataset->count < BOM_MAX_RESPONSE_SIZE) {
                dataset->precipitation[dataset->count] =
                        strtod(precip_buf, &ptr);
                dataset->max_temperature[dataset->count] =
                        strtod(max_t_buf, &ptr);
                dataset->min_temperature[dataset->count] =
                        strtod(min_t_buf, &ptr);

                // Timestamp
                struct tm dt = {0};
                strptime(ts, "%d/%m/%Y", &dt);
                // As UNIX time
                time_t unix_time = mktime(&dt);
                dataset->timestamps[dataset->count] = unix_time;

                // As timestamp with timezone of psql
                char time_str[BOM_TIME_STR_BUFFER_SIZE];
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S%z",
                         &dt);
                strncpy(dataset->timestr[dataset->count], time_str,
                        sizeof(time_str));

                // Debug output
                log_debug("Location: %s, "
                          "Date: %s, "
                          "Rainfall: %0.2f mm, "
                          "Max Temp: %0.2f C, "
                          "Min Temp: %0.2f C\n",
                          loc, ts, dataset->precipitation[dataset->count],
                          dataset->max_temperature[dataset->count],
                          dataset->min_temperature[dataset->count]);
            }
            dataset->count++;
        }
    }

    log_info("BOM Dataset successfully parsed and loaded.\n");

    fclose(file);

    return 0;
}

void BOM_HistoricalWeatherToDB(BOM_WeatherStation_TypeDef* weather_station,
                               BOM_WeatherDataset_TypeDef* dataset,
                               PGconn* psql_conn){

    log_info("Writing BOM weather data to PostgreSQL.\n");

    int16_t index = 0;
    while(index < dataset->count){
        char query[3000];
        snprintf(query, sizeof(query), "INSERT INTO weather_bom (last_updated, "
                                       "location, ts, precipitation, "
                                       "max_temperature, min_temperature) "
                                       "VALUES "
                                       "(NOW(), "   // Last updated timestamp
                                       "'%s', "     // Location (name)
                                       "'%s', "     // Timestamp (tz)
                                       "%lf, "      // Precipitation
                                       "%lf, "      // Max temperature
                                       "%lf) "      // Min temperature
                                       "ON CONFLICT (ts, location) DO UPDATE "
                                       "SET last_updated = NOW()",
                                       weather_station->name,
                                       dataset->timestr[index],
                                       dataset->precipitation[index],
                                       dataset->max_temperature[index],
                                       dataset->min_temperature[index]);

        PGresult* res = PQexec(psql_conn, query);
        if(PQresultStatus(res) != PGRES_COMMAND_OK){
            log_error("PSQL command failed when entering BOM weather data for "
                      "%s. Error: %s\n", weather_station->name,
                      PQerrorMessage(psql_conn));
        }
        PQclear(res);
        index++;
    }

    log_info("BOM weather data written to PostgreSQL.\n");
}
