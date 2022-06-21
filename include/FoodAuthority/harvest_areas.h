#ifndef PROGRAM_HARVEST_AREAS_H
#define PROGRAM_HARVEST_AREAS_H

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <log.h>

#include "FoodAuthority/harvest_area.h"
#include "http.h"
#include "utils.h"

/// Maxiumum number of oyster harvest areas in response
#define FA_MAX_NUMBER_HARVEST_AREAS             120

/// Holds a list of oyster harvest areas (and their status) in NSW
typedef struct{
    uint16_t count; /// Number of resultsj
    /// List of oyster harvest areas and their status
    FA_HarvestArea_TypeDef harvest_area[FA_MAX_NUMBER_HARVEST_AREAS];
}FA_HarvestAreas_TypeDef;

/// Get a list of oyster harvest areas in NSW Australia
CURLcode FA_GetHarvestAreas(FA_HarvestAreas_TypeDef* harvest_areas);

#endif //PROGRAM_HARVEST_AREAS_H
