#include "main.h"

// TODO Data transformation to calculate closure severity index

// TODO Download method for Ubidots salinity & weather readings.
// TODO Data transformation to calculate heat severity index

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

    FA_HarvestAreas_TypeDef harvest_areas = {0};
    FA_GetHarvestAreas(&harvest_areas);
    FA_HarvestAreasToDB(&harvest_areas, psql_conn);
    FA_CreateLocationsLookupDB(psql_conn);

 // //  log_info("Getting weather for: %s\n", harvest_areas.harvest_area[0].
 // //          program_name);

    //WW_Location_TypeDef location_info = {0};
    //WillyWeather_GetLocationByName(harvest_areas.harvest_area[0].program_name,
    //                               &location_info);

   //// WW_RainfallForecast_TypeDef rainfall_forecast = {0};
   //// WillyWeather_GetRainfallForecast(&location_info, &rainfall_forecast);
   //// WillyWeather_RainfallToDB(&location_info, &rainfall_forecast, psql_conn);

   //// BOM_WeatherStations_TypeDef stations;
   //// BOM_LoadStationsFromTxt("tmp/bom_weather_stations.txt", &stations);
   //// int cws = BOM_ClosestStationIndex(location_info.latitude,
   ////                                   location_info.longitude,
   ////                                   &stations);
//
//  //  // Get historical weather from BOM
  ////  BOM_WeatherDataset_TypeDef bom_dataset = {0};
  ////  BOM_GetWeather(&bom_dataset, &stations.stations[cws], "202206");
  ////  BOM_HistoricalWeatherToDB(&stations.stations[cws], &bom_dataset, psql_conn);


    //const char* start_time = "2022-06-01";
    //struct tm start_dt = {0};
    //strptime(start_time, "%Y-%02m-%02d", &start_dt);
    //const time_t unix_st = mktime(&start_dt);

    //const char* end_time = "2022-07-10";
    //struct tm end_dt = {0};
    //strptime(end_time, "%Y-%02m-%02d", &end_dt);
    //const time_t unix_et = mktime(&end_dt);

    //IBM_AuthHandle_TypeDef ibm_auth_handle = {0};
    //if(IBM_HandleAuth(&ibm_auth_handle) != 0){
    //    return 1;
    //}

 // //  IBM_TimeseriesReq_TypeDef ibm_req_precip = {
 // //          .layer_id = IBM_PRECIPITATION_ID,
 // //          .latitude = location_info.latitude,
 // //          .longitude = location_info.longitude,
 // //          .start = unix_st,
 // //          .end = unix_et
 // //  };

 // //  IBM_TimeseriesDataset_TypeDef dataset;
 // //  IBM_GetTimeseries(&ibm_auth_handle, &ibm_req_precip, &dataset, 1);
 // //  IBM_TimeseriesToDB(&ibm_req_precip, &dataset, psql_conn);

    //const int ibm_ids[3] = {IBM_PRECIPITATION_ID,
    //                        IBM_MAX_TEMPERATURE_ID,
    //                        IBM_MIN_TEMPERATURE_ID};

    //IBM_TimeseriesReq_TypeDef ibm_req = {
    //        .layer_id = IBM_MIN_TEMPERATURE_ID,
    //        .latitude = location_info.latitude,
    //        .longitude = location_info.longitude,
    //        .start = unix_st,
    //        .end = unix_et
    //};

    //IBM_TimeseriesDataset_TypeDef max_temp_dataset;
    //IBM_GetTimeseries(&ibm_auth_handle, &ibm_req, &max_temp_dataset, 1);
    //IBM_TimeseriesToDB(&ibm_req, &max_temp_dataset, psql_conn);

    PQfinish(psql_conn);
    curl_global_cleanup();

    return 0;
}
