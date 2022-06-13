#ifndef HA_CLOSURE_ANALYSIS_TIDE_H
#define HA_CLOSURE_ANALYSIS_TIDE_H

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <log.h>

#include "WillyWeather/authenticate.h"
#include "WillyWeather/location.h"
#include "WillyWeather/forecast.h"
#include "utils.h"

/// Below definitions extracted from
/// https://www.willyweather.com.au/api/docs/v2.html#weather

/// Tide data from Willy Weather request
typedef struct{
    time_t low_tide_timestamps[WW_FORECAST_RESPONSE_BUF];
    double low_tide_values[WW_FORECAST_RESPONSE_BUF];
    time_t high_tide_timestamps[WW_FORECAST_RESPONSE_BUF];
    double high_tide_values[WW_FORECAST_RESPONSE_BUF];
    time_t daily_max_tide_timestamps[WW_FORECAST_RESPONSE_BUF];
    double daily_max_tide_values[WW_FORECAST_RESPONSE_BUF];
} WW_TideDataset_TypeDef;

/// Get the tides data from Willy Weather.
CURLcode WillyWeather_GetTides(uint16_t location_id,
                               const char *start_date,
                               uint16_t n_days,
                               WW_TideDataset_TypeDef *tides);

/// Write tide dataset to csv files in specific directory.
uint8_t WillyWeather_TidesToCSV(WW_Location_TypeDef *location_info,
                                WW_TideDataset_TypeDef *dataset);

#endif // HA_CLOSURE_ANALYSIS_TIDE_H
