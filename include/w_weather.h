#ifndef HA_CLOSURE_ANALYSIS_W_WEATHER_H
#define HA_CLOSURE_ANALYSIS_W_WEATHER_H

#include <string.h>
#include <curl/curl.h>

#include "utils.h"

/// Get the weather forcast from Willy Weather.
CURLcode WillyWeather_GetForecast(const char *token,
                                  const char *location,
                                  const char *forecast_type,
                                  const char *start_date,
                                  const char *n_days);

/// Get an ID describing a location from Willy Weather.
CURLcode WillyWeather_GetLocationByName(const char *token,
                                        const char *name,
                                        const char *q_limit);

#endif // HA_CLOSURE_ANALYSIS_W_WEATHER_H
