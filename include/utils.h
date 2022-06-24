#ifndef HA_CLOSURE_ANALYSIS_UTILS_H
#define HA_CLOSURE_ANALYSIS_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <stdint.h>
#include <cjson/cJSON.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>
#include <log.h>

#define USER_AGENT "EnvMonitoring/0.1 (NSW Department of Primary Industries)"

/// Holds HTTP response data before converting these data into cJSON objects.
typedef struct {
	char *memory; ///< The (response) data
	size_t size; ///< Size of the (response) data
} Utils_ReqData_TypeDef;

/// Helper function to handle a CURL request response which normally gets
/// written to stdout.
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                           void *userp);

/// Make directories (with error handling)
int8_t MakeDirectory(const char* directory);

/// Write timeseries data into a csv file
void WriteTimeseriesToFile(const char* filename, time_t* dates, double* values,
                           int16_t max_n_values);

/// Modified minify function from cJSON
void cJSON_Minify_Mod(char *json);

/// Distance between two points on earth
double Utils_PointsDistance(double latitude,
                            double longitude,
                            double station_latitude,
                            double station_longitude);

#endif // HA_CLOSURE_ANALYSIS_UTILS_H
