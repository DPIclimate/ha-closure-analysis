#ifndef PROGRAM_TRANSFORM_H
#define PROGRAM_TRANSFORM_H

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <log.h>

#include "BOM/stations.h"
#include "FoodAuthority/harvest_area.h"
#include "WillyWeather/location.h"
#include "utils.h"

/// Maximum number of locations
#define T_MAX_N_LOCATIONS           50

/// Maximum size of timestamp (num chars)
#define T_TIMESTAMP_SIZE            50

/// Structure of a location (i.e. program location)
typedef struct {
    char last_updated[T_TIMESTAMP_SIZE]; ///< Last time data was updated
    char fa_program_name[FA_MAX_BUFFER]; ///< NSW Food Authority program name
    char ww_location[WW_LOCATION_BUF]; ///< Willy Weather location name
    char ww_location_id[WW_LOCATION_BUF]; ///< Willy Weather location ID
    float ww_latitude; ///< Willy Weather location latitude
    float ww_longitude; ///< Willy Weather location longitude
    char bom_location[BOM_STATION_NAME_SIZE]; ///< BOM weather station name
    char bom_location_id[BOM_STATION_ID_SIZE]; ///< BOM weather station ID
    float bom_latitude; ///< BOM weather station latitude
    float bom_longitude; ///< BOM weather station longitude
    float bom_distance; ///< Distance from BOM weather station to harvest area
}T_LocationLookup_TypeDef;

/// Locations lookup table structure
typedef struct {
    uint16_t count; ///< Number of locations
    T_LocationLookup_TypeDef locations[T_MAX_N_LOCATIONS]; ///< List of locations
}T_LocationsLookup_TypeDef;

/// Build weather table in PostgreSQL database
void T_BuildWeatherDB(T_LocationsLookup_TypeDef* locations,
                      PGconn* psql_conn);

#endif //PROGRAM_TRANSFORM_H
