#ifndef HA_CLOSURE_ANALYSIS_TIMESERIES_H
#define HA_CLOSURE_ANALYSIS_TIMESERIES_H

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <math.h>
#include <time.h>

#include "utils.h"

/// Max number of values in response.
#define IBM_MAX_RESPONSE_LENGTH 2000

/// Max characters in formulated URL.
#define IBM_URL_SIZE 250

/// IBM URL
static const char* IBM_REQUEST_URL = "https://pairs.res.ibm.com";

/// Alternative IBM URL
static const char* IBM_ALT_REQUEST_URL = "https://ibmpairs-mvp2-api.mybluemix.net";

/// Request structure for IBM EMS timeseries data.
typedef struct {
    uint16_t layer_id; /// The layer ID of interest (obtained from IBM query)
    float latitude; /// The latitude of interest
    float longitude; /// The longitude of interest
    time_t start; /// The start time as UNIX epoch time
    time_t end; /// The end time as UNIX epoch time
}TimeseriesReq_TypeDef;

/// Request response data containing IBM EMS timeseries data.
typedef struct {
    time_t start; /// Data start time (unix timestamp in seconds).
    time_t end; /// Data end time (unix timestamp in seconds).
    time_t timestamp[IBM_MAX_RESPONSE_LENGTH]; /// Holds timestamps.
    double values[IBM_MAX_RESPONSE_LENGTH]; /// Holds corresponding values.
    int32_t count; /// Number of returned values.
} TimeseriesDataset_TypeDef;

/// Get timeseries data from IBM EMS
CURLcode IBM_GetTimeseries(const char* access_token,
                           TimeseriesReq_TypeDef *ts_req,
                           TimeseriesDataset_TypeDef *dataset,
                           uint8_t alt_flag);


#endif //HA_CLOSURE_ANALYSIS_TIMESERIES_H
