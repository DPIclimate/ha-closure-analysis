#ifndef PROGRAM_STATIONS_H
#define PROGRAM_STATIONS_H

#include <math.h>
#include <ctype.h>

#include "ftp.h"
#include "utils.h"

/// Max number of BOM sites to hold
#define BOM_STATION_MAX_RESPONSES           500
/// BOM site id size (length)
#define BOM_STATION_ID_SIZE                 7
/// BOM state identifier size (length)
#define BOM_STATION_STATE_SIZE              4
/// BOM site name size (length)
#define BOM_STATION_NAME_SIZE               150
/// BOM filename max size
#define BOM_STATION_FILENAME_SIZE           150

/// Static location of BOM sites .txt file on BOM FTP server
#define BOM_FTP_STATIONS_URL "ftp://ftp.bom.gov.au/anon/gen/clim_data/IDCKWCDEA0/tables/stations_db.txt"

/// BOM weather station (id, state, name, latitude, longitude)
typedef struct {
    char id[BOM_STATION_ID_SIZE]; ///< Station identifier
    char state[BOM_STATION_STATE_SIZE]; ///< Australian state should only be NSW
    char name[BOM_STATION_NAME_SIZE]; ///< Name of station
    char filename[BOM_STATION_FILENAME_SIZE]; ///< Filename formatted name
    double latitude; ///< Latitude of station
    double longitude; ///< Longitude of station
}BOM_WeatherStation_TypeDef;

/// BOM weather stations list and count
typedef struct {
    int16_t count; /// Number of stations in record
    BOM_WeatherStation_TypeDef stations[BOM_STATION_MAX_RESPONSES]; /// List of stations
}BOM_WeatherStations_TypeDef;

/// Get a list of NSW BOM weather stations
CURLcode BOM_GetWeatherStations(BOM_WeatherStations_TypeDef* stations);

/// Load BOM station list from cached .txt file (obtained from BOM FTP server)
int8_t BOM_LoadStationsFromTxt(const char* filename,
                                BOM_WeatherStations_TypeDef* stations);

/// Index of the closest BOM station to a define latitude and longitude
int16_t BOM_ClosestStationIndex(double latitude,
                                double longitude,
                                BOM_WeatherStations_TypeDef* stations);

#endif //PROGRAM_STATIONS_H
