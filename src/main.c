#include "main.h"

int main(void) {

    curl_global_init(CURL_GLOBAL_ALL);

    // Connect to postgres
    PGconn* psql_conn;
    const char* psql_conn_info = "host=localhost dbname=oyster_db port=5432 "
                                 "user=postgres password=admin";
    psql_conn = PQconnectdb(psql_conn_info);
    if(PQstatus(psql_conn) != CONNECTION_OK){
        log_fatal("Unable to connect to PostgreSQL database. "
                  "Error: %s\n", PQerrorMessage(psql_conn));
        return 1;
    }

    //FA_HarvestAreas_TypeDef harvest_areas = {0};
    //FA_GetHarvestAreas(&harvest_areas);
    //FA_HarvestAreasToDB(&harvest_areas, psql_conn);
    //FA_CreateLocationsLookupDB(psql_conn);

    //T_LocationsLookup_TypeDef locations;
    //FA_UniqueLocationsFromDB(&locations, psql_conn);

    //// BUILD BOM TIMESERIES DATASET
    //const char* start_dt = "2022-08-01";
    //BOM_TimeseriesToDB(start_dt, psql_conn);

    ////// BUILD IBM TIMESERIES DATASET
    //const char* start_time = "2022-08-01";
    //const char* end_time = "2022-09-01";
    //IBM_BuildTSDatabase(&locations, start_time, end_time, psql_conn);

    //// BUILD COMBINED WEATHER INFORMATION
    //T_BuildWeatherDB(&locations, psql_conn);

    //// BUILD HARVEST AREA OUTLOOK
    //T_ForecastToZScore(psql_conn);
    //T_WindowDataset(psql_conn, 5);
    T_NormaliseWindowedPrecipitation(psql_conn, 5);

    PQfinish(psql_conn);
    curl_global_cleanup();

    return 0;
}
