#include "transform.h"

static void T_CloseHarvestArea(T_Outlook_Typedef* outlook,
                               const char* reason,
                               const char* timestamp);

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

    const char* ibm_select = "SELECT ts AT TIME ZONE 'AEST', precipitation "
                             "FROM weather_ibm_eis WHERE "
                             "bom_location_id = '%s' "
                             "ORDER BY (ts) DESC;";
    char ibm_query[200];

    // Prepare weather table insert statement (using IBM (forecast) data)
    const char* forecast_stmt_name = "InsertIBMForecast";
    PGresult* forecast_info = PQdescribePrepared(psql_conn, forecast_stmt_name);
    if(PQresultStatus(forecast_info) != PGRES_COMMAND_OK){
        const char* forecast_stmt ="INSERT INTO weather (last_updated, latitude, "
                                   "longitude, ts, program_name, program_id, "
                                   "bom_location_id, data_type, precipitation, "
                                   "forecast_precipitation) "
                                   "VALUES (NOW(), $1::float, $2::float, "
                                   "$3::timestamptz, $4::text, $5::int, "
                                   "$6::text, $7::text, $8::float, $9::float) "
                                   "ON CONFLICT (ts, program_name) DO UPDATE "
                                   "SET last_updated = NOW(), "
                                   "data_type = 'forecast', "
                                   "precipitation = $10::float, "
                                   "forecast_precipitation = $11::float;";
        PGresult* forecast_prep = PQprepare(psql_conn, forecast_stmt_name,
                                            forecast_stmt, 1, NULL);
        if(PQresultStatus(forecast_prep) != PGRES_COMMAND_OK){
            log_warn("PostgreSQL prepare error: %s\n",
                     PQerrorMessage(psql_conn));
        }
        PQclear(forecast_prep);
    }
    PQclear(forecast_info);

    const char* forecast_paramValues[11];

    const char* bom_select ="SELECT ts AT TIME ZONE 'AEST', precipitation "
                            "FROM weather_bom WHERE location_id = '%s' "
                            "ORDER BY (ts) DESC;";
    char bom_query[200];

    // Prepare weather table insert statement (using BOM (historical) data)
    const char* historical_stmt_name = "InsertBOMHistorical";
    PGresult* historical_info = PQdescribePrepared(psql_conn,
                                                   historical_stmt_name);
    if(PQresultStatus(historical_info) != PGRES_COMMAND_OK){
        const char* historical_stmt = "INSERT INTO weather (last_updated, "
                                      "latitude, longitude, ts, program_name, "
                                      "program_id, bom_location_id, data_type, "
                                      "precipitation, observed_precipitation) "
                                      "VALUES (NOW(), $1::float, $2::float, "
                                      "$3::timestamptz, $4::text, $5::int, "
                                      "$6::text, $7::text, $8::float, "
                                      "$9::float) ON CONFLICT "
                                      "(ts, program_name) DO UPDATE SET "
                                      "last_updated = NOW(), "
                                      "data_type = 'observed', "
                                      "precipitation = $10::float, "
                                      "observed_precipitation = $11::float;";
        PGresult* historical_prep = PQprepare(psql_conn, historical_stmt_name,
                                              historical_stmt, 1, NULL);
        if(PQresultStatus(historical_prep) != PGRES_COMMAND_OK){
            log_warn("PostgreSQL prepare error: %s\n",
                     PQerrorMessage(psql_conn));
        }
        PQclear(historical_prep);
    }
    PQclear(historical_info);

    const char* historical_paramValues[11];

    char lat_buf[10];
    char lng_buf[10];
    char precip_buf[10];

    uint16_t index = 0;
    while(index < locations->count){
        T_LocationLookup_TypeDef loc = locations->locations[index];

        log_info("Parsing location %d of %d (%s)\n", index+1, locations->count,
                 loc.fa_program_name);

        memset(ibm_query, 0, sizeof(ibm_query));
        snprintf(ibm_query, sizeof(ibm_query), ibm_select, loc.bom_location_id);
        PGresult* ibm_res = PQexec(psql_conn, ibm_query);
        float ibm_precipitation = 0;
        char ibm_timestamp[50] = {0};
        if(PQresultStatus(ibm_res) == PGRES_TUPLES_OK){
            int num_fields = PQnfields(ibm_res);
            for(int i = 0; i < PQntuples(ibm_res); i++){
                for(int j = 0; j < num_fields; j++){
                    char* ptr;
                    char* val = PQgetvalue(ibm_res, i, j);
                    switch(j){
                        case 0:
                            strncpy(ibm_timestamp, val, sizeof(ibm_timestamp));
                            break;
                        case 1:
                            ibm_precipitation = strtof(val, &ptr);
                            break;
                        default:
                            log_error("Unknown value in IBM query.\n");
                            break;
                    }
                }

                // Prepare values to be inserted into weather table
                snprintf(lat_buf, sizeof(lat_buf), "%f", loc.ww_latitude);
                forecast_paramValues[0] = lat_buf;
                snprintf(lng_buf, sizeof(lng_buf), "%f", loc.ww_longitude);
                forecast_paramValues[1] = lng_buf;
                forecast_paramValues[2] = ibm_timestamp;
                forecast_paramValues[3] = loc.fa_program_name;
                forecast_paramValues[4] = loc.fa_program_id;
                forecast_paramValues[5] = loc.bom_location_id;
                forecast_paramValues[6] = "forecast";
                snprintf(precip_buf, sizeof(precip_buf), "%f",
                         ibm_precipitation);
                forecast_paramValues[7] = precip_buf;
                forecast_paramValues[8] = precip_buf;
                forecast_paramValues[9] = precip_buf;
                forecast_paramValues[10] = precip_buf;

                // Insert weather forecast data from IBM
                PGresult* ibm_insert_query =
                        PQexecPrepared(psql_conn, forecast_stmt_name, 11,
                                       forecast_paramValues, NULL, NULL, 1);
                if(PQresultStatus(ibm_insert_query) != PGRES_COMMAND_OK){
                    log_error("PSQL command failed: %s\n",
                              PQerrorMessage(psql_conn));
                }
                PQclear(ibm_insert_query);
            }
        } else {
            log_error("PSQL command failed: %s ", PQerrorMessage(psql_conn));
        }

        // BOM historical data select
        memset(bom_query, 0, sizeof(bom_query));
        snprintf(bom_query, sizeof(bom_query), bom_select, loc.bom_location_id);
        PGresult* bom_res = PQexec(psql_conn, bom_query);
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
                            bom_precipitation = strtof(PQgetvalue(bom_res, i,
                                                                  j), &ptr);
                            break;
                        default:
                            log_error("Unknown value in IBM query.\n");
                            break;
                    }
                }

                // Populate BOM values for inserting into weather db
                snprintf(lat_buf, sizeof(lat_buf), "%f", loc.ww_latitude);
                historical_paramValues[0] = lat_buf;
                snprintf(lng_buf, sizeof(lng_buf), "%f", loc.ww_longitude);
                historical_paramValues[1] = lng_buf;
                historical_paramValues[2] = bom_timestamp;
                historical_paramValues[3] = loc.fa_program_name;
                historical_paramValues[4] = loc.fa_program_id;
                historical_paramValues[5] = loc.bom_location_id;
                historical_paramValues[6] = "observed";
                snprintf(precip_buf, sizeof(precip_buf), "%f",
                         bom_precipitation);
                historical_paramValues[7] = precip_buf;
                historical_paramValues[8] = precip_buf;
                historical_paramValues[9] = precip_buf;
                historical_paramValues[10] = precip_buf;

                PGresult* bom_insert_query =
                        PQexecPrepared(psql_conn, historical_stmt_name, 11,
                                       historical_paramValues, NULL, NULL, 1);
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
    // TODO Build outlook method here
}


/**
 * Predicted harvest area closure.
 *
 * @param outlook
 * @param reason
 * @param timestamp
 */
void T_CloseHarvestArea(T_Outlook_Typedef* outlook,
                        const char* reason,
                        const char* timestamp){
    if(strcmp(outlook->status, "Closed") != 0) {
        outlook->closed = false;
        outlook->to_close = true;
        strncpy(outlook->closure_type, "Rainfall",
                sizeof(outlook->closure_type));
        strncpy(outlook->closure_reason, reason,
                sizeof(outlook->closure_reason));
        strncpy(outlook->closure_date, timestamp,
                sizeof(outlook->closure_date));
    }

}
