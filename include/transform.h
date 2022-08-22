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

/// Outlook buffer size
#define T_OUTLOOK_BUF_SIZE          200

/// Outlook closue time code
#define T_EST_CLOSURE_BUF_SIZE      20

/// Data type size
#define T_DATA_TYPE_SIZE            30

#define T_DATA_QUERY_LENGTH         15
#define T_DATA_WINDOW_SIZE          7

/// Structure of a location (i.e. program location)
typedef struct {
    char last_updated[T_TIMESTAMP_SIZE]; ///< Last time data was updated
    char fa_program_name[FA_MAX_BUFFER]; ///< NSW Food Authority program name
    char fa_program_id[FA_MAX_BUFFER]; ///< Unique ID of each program area
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

/// Harvest area outlook / prediction information
typedef struct {
    char program_name[FA_MAX_BUFFER]; ///< Food Auth location program name
    int32_t ha_id; ///< Harvest area unique ID
    char classification[FA_MAX_BUFFER]; ///< Harvest area classification
    char status[FA_MAX_BUFFER]; ///< Harvest areas status (open, closed etc.)
    char time_processed[T_TIMESTAMP_SIZE]; ///< Time status was updated
    char status_reason[FA_MAX_BUFFER]; ///< Reason for change in status
    bool closed; ///< Is harvest area currently closed?
    bool to_close; ///< Is the harvest area predicted to close
    char closure_type[T_OUTLOOK_BUF_SIZE]; ///< What type of closue? Rainfall
    char closure_reason[T_OUTLOOK_BUF_SIZE]; ///< Extended reason
    char closure_date[T_TIMESTAMP_SIZE]; ///< Expected closure data
    float closure_severity_index; ///< Expected closure severity index
    char est_closure_time[T_EST_CLOSURE_BUF_SIZE]; ///< How long closed for?
}T_Outlook_Typedef;

typedef struct{
    char time_processed[T_TIMESTAMP_SIZE];
    char data_type[T_DATA_TYPE_SIZE];
    double precipitaiton;
} T_OutlookData_TypeDef;

typedef struct{
    int32_t count;
    T_OutlookData_TypeDef data[T_DATA_QUERY_LENGTH];
} T_OutlookDataset_TypeDef;

/// Build weather table in PostgreSQL database
void T_BuildWeatherDB(T_LocationsLookup_TypeDef* locations,
                      PGconn* psql_conn);

/// Build outlook for each harvest area
void T_BuildOutlook(PGconn* psql_conn);

#endif //PROGRAM_TRANSFORM_H
