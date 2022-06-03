#ifndef HA_CLOSURE_ANALYSIS_PRECIPITATION_H
#define HA_CLOSURE_ANALYSIS_PRECIPITATION_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

/// Request structure for IBM EMS timeseries data.
typedef struct {
    const char* layer_id; /// The layer ID of interest (obtained from IBM query)
    float latitude; /// The latitude of interest
    float longitude; /// The longitude of interest
    unsigned long start; /// The start time as UNIX epoch time
    unsigned long end; /// The end time as UNIX epoch time
}TimeseriesReq_TypeDef;

#endif //HA_CLOSURE_ANALYSIS_PRECIPITATION_H
