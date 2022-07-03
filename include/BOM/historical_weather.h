#ifndef PROGRAM_HISTORICAL_WEATHER_H
#define PROGRAM_HISTORICAL_WEATHER_H

#include <time.h>
#include <curl/curl.h>
#include <ctype.h>
#include <libpq-fe.h>
#include <stdint.h>

#include "BOM/stations.h"
#include "ftp.h"

/// Max number of items in weather dataset
#define BOM_RESPONSE_BUFFER_SIZE        200

/// Max number of chars in timestamp buffer
#define BOM_TIME_STR_BUFFER_SIZE        30

/// Max buffer size to read into memory (size of .csv file from BOM FTP server)
#define BOM_MAX_RESPONSE_SIZE           10000

typedef struct {
    uint16_t count; ///< Number of values in dataset
    time_t timestamps[BOM_RESPONSE_BUFFER_SIZE]; ///< Timestamps of each datapoint
    /// Timestamps of each datapoint as needed for PostgreSQL
    char timestr[BOM_RESPONSE_BUFFER_SIZE][BOM_TIME_STR_BUFFER_SIZE];
    double precipitation[BOM_RESPONSE_BUFFER_SIZE]; ///< Precipitation data
    double max_temperature[BOM_RESPONSE_BUFFER_SIZE]; ///< Max temperature data
    double min_temperature[BOM_RESPONSE_BUFFER_SIZE]; ///< Min temperature data
}BOM_WeatherDataset_TypeDef;

/// Get historical weather for a particular BOM weather station
CURLcode BOM_GetWeather(BOM_WeatherDataset_TypeDef* dataset,
                        BOM_WeatherStation_TypeDef* station,
                        const char* year_month);

/// Load BOM weather station dataset from .csv file
int8_t BOM_LoadWeatherFromCSV(const char* filename,
                              BOM_WeatherDataset_TypeDef* dataset,
                              BOM_WeatherStation_TypeDef* station);

/// Write observed weather to PostgreSQL table (weather_bom)
void BOM_HistoricalWeatherToDB(BOM_WeatherStation_TypeDef* weather_station,
                               BOM_WeatherDataset_TypeDef* dataset,
                               PGconn* psql_conn);


#endif //PROGRAM_HISTORICAL_WEATHER_H
