#include "transform.h"

/**
 * Main weather table for each location.
 *
 * This table contains both forecasted and historical weather information
 * for each harvest location. One column contains precipitation data where
 * forecasted weather is added first, then overwritten using observed values.
 * Both forecasted and observed data are stored in seperate columns.
 *
 * @note This function requires weather_ibm_eis and weather_bom tables
 * to be populated.
 *
 * @param locations Unique locations information.
 * @param psql_conn PostgreSQL connection handler.
 */
void T_BuildWeatherDB(T_LocationsLookup_TypeDef* locations,
                      PGconn* psql_conn){

    uint16_t index = 0;
    while(index < locations->count){
        T_LocationLookup_TypeDef loc = locations->locations[index];

        log_info("Parsing location %d of %d (%s)\n", index+1, locations->count,
                 loc.fa_program_name);

        // Get weather from IBM EIS for location ID
        char ibm_query[1000];
        snprintf(ibm_query, sizeof(ibm_query),
                 "SELECT ts AT TIME ZONE 'AEST', precipitation "
                 "FROM weather_ibm_eis "
                 "WHERE bom_location_id = '%s' "
                 "ORDER BY (ts) DESC;",
                 loc.bom_location_id);

        PGresult* ibm_res = PQexec(psql_conn, ibm_query);
        float ibm_precipitation = 0;
        char ibm_timestamp[50] = {0};
        if(PQresultStatus(ibm_res) == PGRES_TUPLES_OK){
            int num_fields = PQnfields(ibm_res);
            for(int i = 0; i < PQntuples(ibm_res); i++){
                for(int j = 0; j < num_fields; j++){
                    char* ptr;
                    switch(j){
                        case 0:
                            strncpy(ibm_timestamp, PQgetvalue(ibm_res, i, j),
                                    sizeof(ibm_timestamp));
                            break;
                        case 1:
                            ibm_precipitation = strtof(PQgetvalue(ibm_res, i, j),
                                                       &ptr);
                            break;
                        default:
                            log_error("Unknown value in IBM query.\n");
                            break;
                    }
                }

                char ibm_insert[1000];
                snprintf(ibm_insert, sizeof(ibm_insert), "INSERT INTO weather "
                                                         "(last_updated, "
                                                         "latitude, "
                                                         "longitude, "
                                                         "ts, "
                                                         "program_name, "
                                                         "bom_location_id, "
                                                         "data_type, "
                                                         "precipitation, "
                                                         "forecast_precipitation) "
                                                         "VALUES (NOW(), %f, "
                                                         "%f, '%s', '%s', '%s', "
                                                         "'forecast', %f, %f) ON "
                                                         "CONFLICT (ts, "
                                                         "program_name) DO UPDATE "
                                                         "SET last_updated = "
                                                         "NOW(), "
                                                         "data_type = 'forecast', "
                                                         "precipitation = %f, "
                                                         "forecast_precipitation = "
                                                         "%f;",
                         loc.ww_latitude,
                         loc.ww_longitude,
                         ibm_timestamp,
                         loc.fa_program_name,
                         loc.bom_location_id,
                         ibm_precipitation,
                         ibm_precipitation,
                         ibm_precipitation,
                         ibm_precipitation);

                PGresult* ibm_insert_query = PQexec(psql_conn, ibm_insert);
                if(PQresultStatus(ibm_insert_query) != PGRES_COMMAND_OK){
                    log_error("PSQL command failed: %s\n",
                              PQerrorMessage(psql_conn));
                }
                PQclear(ibm_insert_query);
            }
        } else {
            log_error("PSQL command failed: %s ", PQerrorMessage(psql_conn));
        }

        // Get the latest (at max) 20 values from BOM dataset
        char bom_query[1000];
        snprintf(bom_query, sizeof(bom_query),
                 "SELECT ts AT TIME ZONE 'AEST', precipitation "
                 "FROM weather_bom WHERE location_id = '%s' "
                 "ORDER BY (ts) DESC;", loc.bom_location_id);

        PGresult* bom_res =  PQexec(psql_conn, bom_query);
        float bom_precipitation = 0;
        char bom_timestamp[50] = {0};
        if(PQresultStatus(bom_res) == PGRES_TUPLES_OK){
            int num_fields = PQnfields(bom_res);
            for(int i = 0; i < PQntuples(bom_res); i++){
                for(int j = 0; j < num_fields; j++){
                    char* ptr;
                    switch(j){
                        case 0:
                            strncpy(bom_timestamp, PQgetvalue(bom_res, i, j),
                                    sizeof(bom_timestamp));
                            break;
                        case 1:
                            bom_precipitation = strtof(PQgetvalue(bom_res, i, j),
                                                       &ptr);
                            break;
                        default:
                            log_error("Unknown value in IBM query.\n");
                            break;
                    }
                }

                char bom_insert[1000];
                snprintf(bom_insert, sizeof(bom_insert), "INSERT INTO weather "
                                                         "(last_updated, "
                                                         "latitude, "
                                                         "longitude, "
                                                         "ts, "
                                                         "program_name, "
                                                         "bom_location_id, "
                                                         "data_type, "
                                                         "precipitation, "
                                                         "observed_precipitation) "
                                                         "VALUES (NOW(), %f, "
                                                         "%f, '%s', '%s', '%s', "
                                                         "'observed', %f, %f) ON "
                                                         "CONFLICT (ts, "
                                                         "program_name) DO UPDATE "
                                                         "SET last_updated = "
                                                         "NOW(), "
                                                         "data_type = 'observed', "
                                                         "precipitation = %f, "
                                                         "observed_precipitation = "
                                                         "%f;",
                         loc.ww_latitude,
                         loc.ww_longitude,
                         bom_timestamp,
                         loc.fa_program_name,
                         loc.bom_location_id,
                         bom_precipitation,
                         bom_precipitation,
                         bom_precipitation,
                         bom_precipitation);

                PGresult* bom_insert_query = PQexec(psql_conn, bom_insert);
                if(PQresultStatus(bom_insert_query) != PGRES_COMMAND_OK){
                    log_error("PSQL command failed: %s\n",
                              PQerrorMessage(psql_conn));
                }
                PQclear(bom_insert_query);
            }
        } else {
            log_error("PSQL command failed: %s ", PQerrorMessage(psql_conn));
        }

        PQclear(ibm_res);
        PQclear(bom_res);
        index++;
    }

}


void T_BuildOutlook(PGconn* psql_conn){




}