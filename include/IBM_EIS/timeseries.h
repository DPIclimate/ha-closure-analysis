#ifndef HA_CLOSURE_ANALYSIS_TIMESERIES_H
#define HA_CLOSURE_ANALYSIS_TIMESERIES_H

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <math.h>
#include <time.h>
#include <libpq-fe.h>
#include <log.h>

#include "IBM_EIS/authenticate.h"
#include "transform.h"
#include "http.h"
#include "utils.h"

/// Max characters in formulated URL.
#define IBM_URL_SIZE 250

/// Define layer identifier (daily precip, max temp, min temp)
#define IBM_PRECIPITATION_ID        49097
#define IBM_MIN_TEMPERATURE_ID      26019
#define IBM_MAX_TEMPERATURE_ID      26018

/// Max number of values in response.
#define IBM_MAX_RESPONSE_LENGTH 2000

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
} IBM_TimeseriesReq_TypeDef;

/// Request response data containing IBM EMS timeseries data.
typedef struct {
    time_t start; /// Data start time (unix timestamp in seconds).
    time_t end; /// Data end time (unix timestamp in seconds).
    time_t timestamps[IBM_MAX_RESPONSE_LENGTH]; /// Holds timestamps.
    double values[IBM_MAX_RESPONSE_LENGTH]; /// Holds corresponding values.
    int32_t count; /// Number of returned values.
} IBM_TimeseriesDataset_TypeDef;

/// Get timeseries data from IBM EMS
CURLcode IBM_GetTimeseries(IBM_AuthHandle_TypeDef *auth_handle,
                           IBM_TimeseriesReq_TypeDef *request,
                           IBM_TimeseriesDataset_TypeDef *dataset,
                           uint8_t alt_flag);

/// Write timeseries dataset to .csv file
int8_t IBM_TimeseriesToCSV(IBM_TimeseriesReq_TypeDef *request,
                           IBM_TimeseriesDataset_TypeDef *dataset);

IBM_TimeseriesDataset_TypeDef IBM_TimeseriesFromCSV(const char* filename);

void IBM_TimeseriesToDB(IBM_TimeseriesReq_TypeDef* req_info,
                        IBM_TimeseriesDataset_TypeDef* dataset,
                        T_LocationLookup_TypeDef* location,
                        PGconn* psql_conn);

#endif //HA_CLOSURE_ANALYSIS_TIMESERIES_H
