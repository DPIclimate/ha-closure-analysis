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

    //FA_HarvestAreas_TypeDef harvest_areas = {0};
    //FA_GetHarvestAreas(&harvest_areas);
    //FA_HarvestAreasToDB(&harvest_areas, psql_conn);
    //FA_CreateLocationsLookupDB(psql_conn);

    T_LocationsLookup_TypeDef locations;
    FA_UniqueLocationsFromDB(&locations, psql_conn);

    // BUILD BOM TIMESERIES DATASET
    const char* start_dt = "2022-07-01";
    BOM_TimeseriesToDB(&locations, start_dt, psql_conn);

    //// BUILD IBM TIMESERIES DATASET BELOW
    //const char* start_time = "2018-01-01";
    //const char* end_time = "2022-07-15";
    //IBM_BuildTSDatabase(&locations, start_time, end_time, psql_conn);

    //// BUILD COMBINED WEATHER INFORMATION
    //T_BuildWeatherDB(&locations, psql_conn);

    //// BUILD HARVEST AREA OUTLOOK
    //T_BuildOutlook(psql_conn);

    PQfinish(psql_conn);
    curl_global_cleanup();

    return 0;
}
