#ifndef PROGRAM_HARVEST_AREAS_H
#define PROGRAM_HARVEST_AREAS_H

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <libpq-fe.h>
#include <log.h>

#include "FoodAuthority/harvest_area.h"
#include "http.h"
#include "utils.h"
#include "WillyWeather/location.h"
#include "BOM/stations.h"

/// Harvest areas directory
#define FA_DEFAULT_DIRECTORY "datasets/nsw_food_authority"
/// Harvest area statuses filename
#define FA_DEFAULT_FILENAME "datasets/nsw_food_authority/statuses.csv"

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

/// Push harvest areas list into a .csv file
int8_t FA_HarvestAreasToCSV(FA_HarvestAreas_TypeDef* harvest_areas);

void FA_HarvestAreasToDB(FA_HarvestAreas_TypeDef* harvest_areas,
                           PGconn* psql_conn);

void FA_CreateLocationsLookupDB(WW_Locations_TypeDef* locations,
                                PGconn* psql_conn);

#endif //PROGRAM_HARVEST_AREAS_H
