#ifndef HA_CLOSURE_ANALYSIS_FOOD_AUTH_H
#define HA_CLOSURE_ANALYSIS_FOOD_AUTH_H

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "utils.h"

#define MAX_RESPONSE_BUFFER 150 ///< Buffer size for each HTML value

/**
 * Harvest area status information.
 */
typedef struct {
    char location[MAX_RESPONSE_BUFFER]; ///< Relevent location
    /**< NSW Food Authority quality classification */
    char classification[MAX_RESPONSE_BUFFER];
    char status[MAX_RESPONSE_BUFFER]; ///< HA status (open / closed)
    char time[MAX_RESPONSE_BUFFER]; ///< Time updated
    char reason[MAX_RESPONSE_BUFFER]; ///< Reason for status
    char previous_reason[MAX_RESPONSE_BUFFER]; ///< Previous status reason
} HarvestAreaStatus_TypeDef;

CURLcode FoodAuth_GetHarvestAreaStatus(const char* harvest_name,
                                       HarvestAreaStatus_TypeDef *ha_status);

#endif //HA_CLOSURE_ANALYSIS_FOOD_AUTH_H
