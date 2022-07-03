#include "main.h"

int main(void) {

    curl_global_init(CURL_GLOBAL_ALL);

    // Connect to postgres
    PGconn* psql_conn;
    const char* psql_conn_info = "host=localhost dbname=oyster_db port=5432 "
                                 "user=postgres password=admin";
    psql_conn = PQconnectdb(psql_conn_info);
    if(PQstatus(psql_conn) != CONNECTION_OK){
        log_error("Unable to connect to PostgreSQL database. "
                  "Error: %s\n", PQerrorMessage(psql_conn));
    }

    // BUILD BOM TIMESERIES DATASET BELOW
    //FA_HarvestAreas_TypeDef harvest_areas = {0};
    //FA_GetHarvestAreas(&harvest_areas);
    //FA_HarvestAreasToDB(&harvest_areas, psql_conn);
    //FA_CreateLocationsLookupDB(psql_conn);

    T_LocationsLookup_TypeDef locations;
    FA_UniqueLocationsFromDB(&locations, psql_conn);
    T_BuildWeatherDB(&locations, psql_conn);

    //BOM_WeatherStations_TypeDef stations;
    //BOM_LoadStationsFromTxt("tmp/bom_weather_stations.txt", &stations);

    //uint16_t index = 0;
    //while(index < locations.count){
    //    int cws = BOM_ClosestStationIndex((double)locations.locations[index].ww_latitude,
    //                                      (double)locations.locations[index].ww_longitude,
    //                                      &stations);

    //    // Get historical weather from BOM
    //    BOM_WeatherDataset_TypeDef bom_dataset = {0};
    //    BOM_GetWeather(&bom_dataset, &stations.stations[cws], "202206");
    //    BOM_HistoricalWeatherToDB(&stations.stations[cws], &bom_dataset, psql_conn);
    //    index++;
    //}

    // BUILD IBM TIMESERIES DATASET BELOW
//    T_LocationsLookup_TypeDef locations;
//    FA_UniqueLocationsFromDB(&locations, psql_conn);
//
//    const char* start_time = "2022-06-01";
//    struct tm start_dt = {0};
//    strptime(start_time, "%Y-%02m-%02d", &start_dt);
//    const time_t unix_st = mktime(&start_dt);
//
//    const char* end_time = "2022-07-10";
//    struct tm end_dt = {0};
//    strptime(end_time, "%Y-%02m-%02d", &end_dt);
//    const time_t unix_et = mktime(&end_dt);
//
//    IBM_AuthHandle_TypeDef ibm_auth_handle = {0};
//    if(IBM_HandleAuth(&ibm_auth_handle) != 0){
//        return 1;
//    }
//
//    uint16_t index = 0;
//    while(index < locations.count){
//        IBM_TimeseriesReq_TypeDef ibm_req = {
//                .layer_id = IBM_PRECIPITATION_ID,
//                .latitude = locations.locations[index].ww_latitude,
//                .longitude = locations.locations[index].ww_longitude,
//                .start = unix_st,
//                .end = unix_et
//        };
//
//        IBM_TimeseriesDataset_TypeDef ibm_dataset;
//        IBM_GetTimeseries(&ibm_auth_handle, &ibm_req, &ibm_dataset, 1);
//        IBM_TimeseriesToDB(&ibm_req, &ibm_dataset,
//                           &locations.locations[index], psql_conn);
//        index++;
//    }

    PQfinish(psql_conn);
    curl_global_cleanup();

    return 0;
}
