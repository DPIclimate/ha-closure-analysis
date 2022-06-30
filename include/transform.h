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

#define T_MAX_N_LOCATIONS           50
#define T_TIMESTAMP_SIZE            50

typedef struct {
    char last_updated[T_TIMESTAMP_SIZE];
    char fa_program_name[FA_MAX_BUFFER];
    char ww_location[WW_LOCATION_BUF];
    char ww_location_id[WW_LOCATION_BUF];
    float ww_latitude;
    float ww_longitude;
    char bom_location[BOM_STATION_NAME_SIZE];
    char bom_location_id[BOM_STATION_ID_SIZE];
    float bom_latitude;
    float bom_longitude;
    float bom_distance;
}T_LocationLookup_TypeDef;

typedef struct {
    uint16_t count;
    T_LocationLookup_TypeDef locations[T_MAX_N_LOCATIONS];
}T_LocationsLookup_TypeDef;

void T_BuildWeatherDB(T_LocationsLookup_TypeDef* locations,
                      PGconn* psql_conn);

#endif //PROGRAM_TRANSFORM_H
