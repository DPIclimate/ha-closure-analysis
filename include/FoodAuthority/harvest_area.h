#ifndef HA_CLOSURE_ANALYSIS_HARVEST_AREA_H
#define HA_CLOSURE_ANALYSIS_HARVEST_AREA_H

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <log.h>

#include "http.h"
#include "utils.h"

/// Buffer size for each HTML value from NSW Food Authority
#define FA_MAX_BUFFER 150

/// Moonlight harvest area
#define FA_HA_CLYDE_MOONLIGHT "Moonlight"
/// Rocky point harvest area
#define FA_HA_CLYDE_ROCKY_PT "Rocky"
/// Waterfall harvest area
#define FA_HA_CLYDE_WATERFALL "Waterfall"

/// Harvest area status information.
typedef struct {
    char program_name[FA_MAX_BUFFER]; ///< Location of harvest area program
    char location[FA_MAX_BUFFER]; ///< Location of harvest area
    char name[FA_MAX_BUFFER]; ///< Name of harvest area
    int32_t id; ///< Unique id from Food Authority
    char classification[FA_MAX_BUFFER]; ///< NSW FA quality classification
    char status[FA_MAX_BUFFER]; ///< HA status (open / closed)
    char time[FA_MAX_BUFFER]; ///< Time updated
    struct tm u_time; ///< System formatted time
    char reason[FA_MAX_BUFFER]; ///< Reason for status
    char previous_reason[FA_MAX_BUFFER]; ///< Previous status reason
} FA_HarvestArea_TypeDef;

/// Harvest area of interest status information.
CURLcode FA_GetHarvestAreaStatus(const char* harvest_name,
                                 FA_HarvestArea_TypeDef *ha_status);

/// Parse HTML data to populate status information data.
void FA_ParseResponse(char* data,
                      FA_HarvestArea_TypeDef *harvest_area);

#endif //HA_CLOSURE_ANALYSIS_HARVEST_AREA_H
