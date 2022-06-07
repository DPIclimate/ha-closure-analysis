#ifndef HA_CLOSURE_ANALYSIS_W_WEATHER_H
#define HA_CLOSURE_ANALYSIS_W_WEATHER_H

#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

#define WW_FORECAST_PRECIS              "precis"
#define WW_FORECAST_PRECIP              "rainfall"
#define WW_FORECAST_PRECIP_PROB         "rainfallprobability"
#define WW_FORECAST_TIDE                "tides"
#define WW_FORECAST_SUNRISE_SUNSET      "sunrisesunset"
#define WW_FORECAST_SWELL               "swell"
#define WW_FORECAST_TEMPERATURE         "temperature"
#define WW_FORECAST_UV                  "UV"
#define WW_FORECAST_WEATHER             "weather"
#define WW_FORECAST_WIND                "wind"

#define WW_MAX_BUFFER 150
#define WW_MAX_RESPONSE_SIZE 2000

/// Location data from Willy Weather request
typedef struct{
    uint16_t id;
    char location[WW_MAX_BUFFER];
    char region[WW_MAX_BUFFER];
    char state[WW_MAX_BUFFER];
    float latitude;
    float longitude;
} WW_Location_TypeDef;

/// Tide data from Willy Weather request
typedef struct{
    char location[WW_MAX_BUFFER];
    time_t low_tide_timestamps[WW_MAX_RESPONSE_SIZE];
    double low_tide_values[WW_MAX_RESPONSE_SIZE];
    time_t high_tide_timestamps[WW_MAX_RESPONSE_SIZE];
    double high_tide_values[WW_MAX_RESPONSE_SIZE];
    time_t daily_max_tide_timestamps[WW_MAX_RESPONSE_SIZE];
    double daily_max_tide_values[WW_MAX_RESPONSE_SIZE];
} WW_TideDataset_TypeDef;

/// Get the weather forcast from Willy Weather.
CURLcode WillyWeather_GetForecast(const char *token,
                                  uint16_t location,
                                  const char *forecast_type,
                                  const char *start_date,
                                  uint16_t n_days);

/// Get an ID describing a location from Willy Weather.
CURLcode WillyWeather_GetLocationByName(const char *token,
                                        const char *name,
                                        uint8_t q_limit);

#endif // HA_CLOSURE_ANALYSIS_W_WEATHER_H
