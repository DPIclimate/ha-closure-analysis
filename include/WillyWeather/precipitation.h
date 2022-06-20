#ifndef PROGRAM_PRECIPITATION_H
#define PROGRAM_PRECIPITATION_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "http.h"
#include "WillyWeather/forecast.h"
#include "WillyWeather/location.h"
#include "utils.h"

/// Willy Weather range code (e.g. 0-15 or 5-10 or <0) max size
#define WW_RANGE_CODE_SIZE                      6
/// Max number of daily rainfall forecast results from Willy Weather
#define WW_MAX_DAILY_RAINFALL_RESULTS           9

/// Rainfall forecast for a day
typedef struct{
    time_t date; /// UNIX timestamp date
    int8_t start_range; /// Min expected rainfall (1, 5, 10, 15, 25, 50, null)
    int8_t end_range; /// Max expected rainfall (5, 10, 15, 25, 50, 100)
    char range_divider; /// Divider between min and max rainfall ('>' or '=')
    char range_code[WW_RANGE_CODE_SIZE]; /// Combines min divider and max e.g. 10-15 or <0
    int8_t probability; /// Chance of rain (0 to 100 %)
}WW_Rainfall_TypeDef;

/// Holds daily forecast rainfall information from Willy Weather
typedef struct{
    uint16_t location_id; /// Location identifier
    int16_t n_days; /// Number of days parsed from HTTP request
    WW_Rainfall_TypeDef daily_rainfall[WW_MAX_DAILY_RAINFALL_RESULTS]; /// Query results
}WW_DailyRainfall_TypeDef;

/// Gets the rainfall forecast (next 7 days inc today) for a location
CURLcode WillyWeather_GetRainfallForecast(WW_Location_TypeDef* location,
                                          WW_DailyRainfall_TypeDef* daily_rainfall);

#endif //PROGRAM_PRECIPITATION_H
