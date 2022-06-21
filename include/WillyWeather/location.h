#ifndef HA_CLOSURE_ANALYSIS_LOCATION_H
#define HA_CLOSURE_ANALYSIS_LOCATION_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <log.h>

#include "WillyWeather/authenticate.h"
#include "http.h"
#include "utils.h"

/// Buffer for location information
#define WW_LOCATION_BUF                 150
/// Buffer for request location URL
#define WW_LOCATION_URL_BUF             200

/// Location data from Willy Weather request
typedef struct{
    uint16_t id; /// Location ID for Willy Weather requests
    char location[WW_LOCATION_BUF]; /// Location (name)
    char region[WW_LOCATION_BUF]; /// Region location is in
    char state[WW_LOCATION_BUF]; /// Australian state name (e.g. NSW)
    char postcode[WW_LOCATION_BUF]; /// Australian post code (e.g. 3960)
    double latitude; /// Latitude of interest
    double longitude; /// Longitude of interest
} WW_Location_TypeDef;

/// Get an ID describing a location from Willy Weather.
CURLcode WillyWeather_GetLocationByName(char *name,
                                        WW_Location_TypeDef *location_info);


#endif //HA_CLOSURE_ANALYSIS_LOCATION_H
