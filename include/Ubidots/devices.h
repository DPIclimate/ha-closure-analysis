#ifndef HA_CLOSURE_ANALYSIS_DEVICES_H
#define HA_CLOSURE_ANALYSIS_DEVICES_H

#include <string.h>
#include <math.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <log.h>

#include "Ubidots/authenticate.h"
#include "http.h"
#include "utils.h"

/// Maximum number of devices in buffer
#define UBIDOTS_MAX_NUMBER_DEVICES          300

/// Maximum name length for devices
#define UBIDOTS_MAX_INFO_LENGTH             100

/// TODO turn this into a single device then have a list of devices struct
typedef struct{
    int16_t count;
    char names[UBIDOTS_MAX_NUMBER_DEVICES][UBIDOTS_MAX_INFO_LENGTH];
    char ids[UBIDOTS_MAX_NUMBER_DEVICES][UBIDOTS_MAX_INFO_LENGTH];
    double latitudes[UBIDOTS_MAX_NUMBER_DEVICES];
    double longitudes[UBIDOTS_MAX_NUMBER_DEVICES];
} Ubidots_Devices_TypeDef;

/// List all ubidots devices
CURLcode Ubidots_ListDevices(Ubidots_Devices_TypeDef *devices);

/// Write devices to csv file for caching
int8_t Ubidots_DevicesToCSV(Ubidots_Devices_TypeDef *devices);

#endif // HA_CLOSURE_ANALYSIS_DEVICES_H

