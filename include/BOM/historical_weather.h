#ifndef PROGRAM_HISTORICAL_WEATHER_H
#define PROGRAM_HISTORICAL_WEATHER_H

#include <curl/curl.h>
#include <ctype.h>
#include <stdint.h>

#include "BOM/stations.h"
#include "ftp.h"

#define BOM_RESPONSE_BUFFER_SIZE        200
#define BOM_MAX_RESPONSE_SIZE           10000

typedef struct {
    uint16_t count;
    time_t timestamps[BOM_RESPONSE_BUFFER_SIZE];
    float precipitation[BOM_RESPONSE_BUFFER_SIZE];
    float max_temperature[BOM_RESPONSE_BUFFER_SIZE];
    float min_temperature[BOM_RESPONSE_BUFFER_SIZE];
}BOM_WeatherDataset_TypeDef;

CURLcode BOM_GetWeather(BOM_WeatherDataset_TypeDef* dataset,
                        BOM_WeatherStation_TypeDef* station,
                        const char* year_month);

int8_t BOM_LoadWeatherFromCSV(const char* filename,
                              BOM_WeatherDataset_TypeDef* dataset,
                              BOM_WeatherStation_TypeDef* station);

#endif //PROGRAM_HISTORICAL_WEATHER_H
