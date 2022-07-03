#ifndef PROGRAM_PRECIPITATION_H
#define PROGRAM_PRECIPITATION_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

#include "http.h"
#include "WillyWeather/forecast.h"
#include "WillyWeather/location.h"
#include "utils.h"

/// Willy Weather range code (e.g. 0-15 or 5-10 or <0) max size
#define WW_RANGE_CODE_SIZE                      6
/// Max number of daily rainfall forecast results from Willy Weather
#define WW_MAX_DAILY_RAINFALL_RESULTS           9
/// Max timestamp size
#define WW_MAX_TS_SIZE                          30

/// Rainfall forecast for a day
typedef struct{
    time_t date; ///< UNIX timestamp date
    char ts[WW_MAX_TS_SIZE]; ///< Formatted timestamp
    /// Min expected rainfall (1, 5, 10, 15, 25, 50, null)
    int8_t start_range;
    /// Max expected rainfall (5, 10, 15, 25, 50, 100)
    int8_t end_range;
    /// Divider between min and max rainfall ('>' or '=')
    char range_divider;
    /// Range codes: 0, 1-5, 5-10, 10-15, 15-25, 25-50, 50-100, 100
    /// Combines min, divider and max
    char range_code[WW_RANGE_CODE_SIZE];
    int8_t probability; ///< Chance of 0.2 mm of rain (0 to 100 %)
}WW_Rainfall_TypeDef;

/// Holds daily forecast rainfall information from Willy Weather
typedef struct{
    uint16_t location_id; ///< Location identifier
    int16_t n_days; ///< Number of days parsed from HTTP request (length of list)
    /// Willy Weather forecast list
    WW_Rainfall_TypeDef forecast[WW_MAX_DAILY_RAINFALL_RESULTS];
}WW_RainfallForecast_TypeDef;

/// Gets the rainfall forecast (next 7 days inc today) for a location
CURLcode WillyWeather_GetRainfallForecast(WW_Location_TypeDef* location,
                                          WW_RainfallForecast_TypeDef* daily_rainfall);

/// Rainfall to PostgreSQL table method
void WillyWeather_RainfallToDB(WW_Location_TypeDef* location,
                               WW_RainfallForecast_TypeDef* forecast,
                               PGconn* psql_conn);

#endif //PROGRAM_PRECIPITATION_H
